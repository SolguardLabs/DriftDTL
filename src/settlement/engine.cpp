#include "settlement/engine.hpp"

#include "core/error.hpp"
#include "settlement/policy.hpp"
#include "settlement/timeline.hpp"

namespace drift {

void SettlementEngine::set_epoch(Epoch epoch)
{
    if (epoch < 0) {
        throw DriftError(ErrorCode::InvalidArgument, "epoch cannot be negative");
    }
    epoch_ = epoch;
}

void SettlementEngine::register_account(const AccountConfig& account)
{
    ledger_.create_account(account.id);
    for (const auto& balance : account.balances) {
        ledger_.seed(account.id, balance.asset, balance.available);
        total_seeded_ += balance.available;
        append_event(
            EventKind::AccountSeeded,
            epoch_,
            PacketId("system"),
            LaneId("system"),
            "seeded " + account.id.str() + " with " + balance.available.str() + " " + balance.asset.str(),
            balance.available);
    }
}

void SettlementEngine::register_lane(const LaneConfig& lane_config)
{
    PolicyEvaluator::validate_lane(lane_config);
    auto insert_result = lanes_.emplace(lane_config.id, lane_config);
    if (!insert_result.second) {
        throw DriftError(ErrorCode::PolicyViolation, "duplicate lane: " + lane_config.id.str());
    }
    ledger_.create_account(lane_config.operator_account);
    append_event(
        EventKind::LaneRegistered,
        epoch_,
        PacketId("system"),
        lane_config.id,
        "registered settlement lane " + lane_config.id.str());
}

void SettlementEngine::submit(const PacketTemplate& packet, Epoch epoch)
{
    ensure_epoch(epoch);
    if (records_.find(packet.id) != records_.end()) {
        throw DriftError(ErrorCode::DuplicatePacket, "packet already exists: " + packet.id.str());
    }

    const LaneConfig& lane_config = lane(packet.lane);
    PolicyEvaluator::validate_packet(lane_config, packet);
    SettlementRecord next = build_record(packet, lane_config, epoch, 0);
    apply_observed_reserve(next);
    apply_confirmed_prepare(next);
    records_.emplace(next.packet, next);
    append_event(
        EventKind::PacketSubmitted,
        epoch,
        packet.id,
        packet.lane,
        "packet submitted and reserved",
        next.amounts.gross);
}

void SettlementEngine::retry(const PacketTemplate& packet, Epoch epoch)
{
    ensure_epoch(epoch);
    const LaneConfig& lane_config = lane(packet.lane);
    PolicyEvaluator::validate_packet(lane_config, packet);

    auto found = records_.find(packet.id);
    if (found == records_.end()) {
        SettlementRecord first = build_record(packet, lane_config, epoch, 0);
        apply_observed_reserve(first);
        apply_confirmed_prepare(first);
        records_.emplace(first.packet, first);
        append_event(EventKind::PacketRetried, epoch, packet.id, packet.lane, "retry opened as first attempt", first.amounts.gross);
        return;
    }

    SettlementRecord& current = found->second;
    if (current.confirmed_status == ConfirmedStatus::Settled) {
        throw DriftError(ErrorCode::PacketAlreadyClosed, "settled packet cannot be retried");
    }
    if (current.observed_status == ObservedStatus::Open) {
        throw DriftError(ErrorCode::PacketNotReady, "open packet cannot be retried");
    }
    if (!PolicyEvaluator::can_retry(lane_config, current.attempt)) {
        throw DriftError(ErrorCode::RetryLimitExceeded, "retry limit exceeded for packet: " + packet.id.str());
    }

    SettlementAmounts amounts = PolicyEvaluator::quote(lane_config, packet.amount);
    if (amounts.gross != current.amounts.gross) {
        throw DriftError(ErrorCode::PolicyViolation, "retry amount does not match original packet");
    }

    current.attempt += 1;
    current.observed_epoch = epoch;
    current.last_update_epoch = epoch;
    current.timeout_epoch = PolicyEvaluator::timeout_for(lane_config, epoch);
    current.observed_status = ObservedStatus::Open;
    current.memo = packet.memo.empty() ? current.memo : packet.memo;
    apply_observed_reserve(current);
    if (current.confirmed_status == ConfirmedStatus::Missing || current.confirmed_status == ConfirmedStatus::Released) {
        current.confirmed_status = ConfirmedStatus::Prepared;
        apply_confirmed_prepare(current);
    }
    append_event(EventKind::PacketRetried, epoch, packet.id, packet.lane, "packet retry opened", current.amounts.gross);
}

void SettlementEngine::confirm(const PacketId& packet, const AckId& ack, Epoch epoch)
{
    ensure_epoch(epoch);
    SettlementRecord& current = record_mut(packet);
    const LaneConfig& lane_config = lane(current.lane);
    if (ack.empty()) {
        throw DriftError(ErrorCode::InvalidArgument, "ack id is required");
    }
    if (lane_config.policy.require_unique_ack && current.acknowledgements.count(ack.str()) != 0) {
        duplicate_confirmations_ += 1;
        append_event(EventKind::DuplicateConfirmation, epoch, packet, current.lane, "duplicate confirmation ignored");
        return;
    }
    current.acknowledgements.insert(ack.str());

    if (current.confirmed_status == ConfirmedStatus::Settled) {
        duplicate_confirmations_ += 1;
        append_event(EventKind::DuplicateConfirmation, epoch, packet, current.lane, "confirmation for closed packet ignored");
        return;
    }
    if (current.confirmed_status != ConfirmedStatus::Prepared) {
        throw DriftError(ErrorCode::PacketNotReady, "packet is not prepared for confirmation");
    }

    TimelineProfile timing = Timeline::profile(current, epoch, 1);
    current.notes.push_back(Timeline::describe(timing));
    consume_observed_if_open(current);
    settle_confirmed(current);
    current.confirmed_sequence += 1;
    current.last_update_epoch = epoch;
    append_event(EventKind::PacketConfirmed, epoch, packet, current.lane, "packet confirmed", current.amounts.net);
}

void SettlementEngine::cancel(const PacketId& packet, Epoch epoch, bool force)
{
    ensure_epoch(epoch);
    SettlementRecord& current = record_mut(packet);
    const LaneConfig& lane_config = lane(current.lane);
    if (!lane_config.policy.allow_manual_cancel && !force) {
        throw DriftError(ErrorCode::PolicyViolation, "manual cancellation disabled for lane");
    }
    if (!current.observed_open()) {
        throw DriftError(ErrorCode::PacketAlreadyClosed, "observed packet is not open");
    }
    if (!force && !current.expired_at(epoch)) {
        throw DriftError(ErrorCode::PacketNotExpired, "packet has not reached timeout");
    }

    ledger_.release_observed(current.source, current.asset, current.amounts.gross);
    current.observed_status = ObservedStatus::Cancelled;
    current.last_update_epoch = epoch;
    current.notes.push_back("cancelled from observed plane");
    append_event(EventKind::PacketCancelled, epoch, packet, current.lane, "packet cancelled", current.amounts.gross);
}

void SettlementEngine::advance(Epoch epoch)
{
    ensure_epoch(epoch);
    append_event(EventKind::EpochAdvanced, epoch, PacketId("system"), LaneId("system"), "epoch advanced");
}

void SettlementEngine::snapshot(Epoch epoch, std::string note)
{
    ensure_epoch(epoch);
    append_event(EventKind::Snapshot, epoch, PacketId("system"), LaneId("system"), std::move(note));
}

Epoch SettlementEngine::epoch() const noexcept
{
    return epoch_;
}

const AccountLedger& SettlementEngine::ledger() const noexcept
{
    return ledger_;
}

const std::map<LaneId, LaneConfig>& SettlementEngine::lanes() const noexcept
{
    return lanes_;
}

const std::map<PacketId, SettlementRecord>& SettlementEngine::records() const noexcept
{
    return records_;
}

const std::vector<EngineEvent>& SettlementEngine::events() const noexcept
{
    return events_;
}

EngineSummary SettlementEngine::summary() const
{
    EngineSummary out;
    out.epoch = epoch_;
    out.total_seeded = total_seeded_;
    out.total_settled = total_settled_;
    out.total_fees = total_fees_;
    out.duplicate_confirmations = duplicate_confirmations_;
    out.warnings = warnings_;

    for (const auto& account : ledger_.accounts()) {
        for (const auto& balance : ledger_.balances_for(account)) {
            out.observed_locked += balance.observed_locked;
            out.confirmed_locked += balance.confirmed_locked;
        }
    }

    for (const auto& entry : records_) {
        const SettlementRecord& record = entry.second;
        if (record.observed_status == ObservedStatus::Open) {
            out.packets_open += 1;
        }
        if (record.observed_status == ObservedStatus::Cancelled) {
            out.packets_cancelled += 1;
        }
        if (record.confirmed_status == ConfirmedStatus::Settled) {
            out.packets_settled += 1;
        }
    }
    return out;
}

const SettlementRecord* SettlementEngine::find_record(const PacketId& packet) const noexcept
{
    auto found = records_.find(packet);
    if (found == records_.end()) {
        return nullptr;
    }
    return &found->second;
}

const LaneConfig& SettlementEngine::lane(const LaneId& id) const
{
    auto found = lanes_.find(id);
    if (found == lanes_.end()) {
        throw DriftError(ErrorCode::UnknownLane, "unknown lane: " + id.str());
    }
    return found->second;
}

SettlementRecord& SettlementEngine::record_mut(const PacketId& packet)
{
    auto found = records_.find(packet);
    if (found == records_.end()) {
        throw DriftError(ErrorCode::UnknownPacket, "unknown packet: " + packet.str());
    }
    return found->second;
}

const SettlementRecord& SettlementEngine::record(const PacketId& packet) const
{
    auto found = records_.find(packet);
    if (found == records_.end()) {
        throw DriftError(ErrorCode::UnknownPacket, "unknown packet: " + packet.str());
    }
    return found->second;
}

SettlementRecord SettlementEngine::build_record(const PacketTemplate& packet, const LaneConfig& lane_config, Epoch epoch, std::int64_t attempt) const
{
    SettlementAmounts amounts = PolicyEvaluator::quote(lane_config, packet.amount);
    SettlementRecord record;
    record.packet = packet.id;
    record.lane = packet.lane;
    record.asset = lane_config.asset;
    record.source = packet.source;
    record.recipient = packet.recipient;
    record.operator_account = lane_config.operator_account;
    record.priority = packet.priority;
    record.memo = packet.memo;
    record.amounts = amounts;
    record.observed_epoch = epoch;
    record.last_update_epoch = epoch;
    record.timeout_epoch = PolicyEvaluator::timeout_for(lane_config, epoch);
    record.attempt = attempt;
    record.observed_status = ObservedStatus::Open;
    record.confirmed_status = ConfirmedStatus::Prepared;
    return record;
}

void SettlementEngine::apply_observed_reserve(const SettlementRecord& record)
{
    ledger_.reserve_observed(record.source, record.asset, record.amounts.gross);
}

void SettlementEngine::apply_confirmed_prepare(const SettlementRecord& record)
{
    ledger_.attach_confirmed(record.source, record.asset, record.amounts.gross);
}

void SettlementEngine::consume_observed_if_open(SettlementRecord& record)
{
    if (record.observed_status == ObservedStatus::Open) {
        ledger_.consume_observed(record.source, record.asset, record.amounts.gross);
        record.observed_status = ObservedStatus::Consumed;
        record.observed_sequence += 1;
    }
}

void SettlementEngine::settle_confirmed(SettlementRecord& record)
{
    ledger_.settle_confirmed(record.source, record.recipient, record.asset, record.amounts.net, record.amounts.fee);
    if (!record.amounts.fee.is_zero()) {
        ledger_.credit_fee(record.operator_account, record.asset, record.amounts.fee);
    }
    total_settled_ += record.amounts.net;
    total_fees_ += record.amounts.fee;
    record.confirmed_status = ConfirmedStatus::Settled;
}

void SettlementEngine::append_event(EventKind kind, Epoch epoch, const PacketId& packet, const LaneId& lane, std::string message, Amount amount)
{
    if (kind == EventKind::Warning) {
        warnings_ += 1;
    }
    events_.push_back(EngineEvent{kind, epoch, packet, lane, std::move(message), amount});
}

void SettlementEngine::ensure_epoch(Epoch epoch)
{
    if (epoch < epoch_) {
        throw DriftError(ErrorCode::InvalidArgument, "actions cannot move backwards in epoch");
    }
    epoch_ = epoch;
}

} // namespace drift
