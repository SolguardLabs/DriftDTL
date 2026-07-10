#include "report/json_report.hpp"

#include "audit/replay.hpp"
#include "domain/model.hpp"
#include "ledger/reconcile.hpp"
#include "settlement/queue.hpp"
#include "settlement/receipt.hpp"

namespace drift {

json::Value JsonReport::build(const SettlementEngine& engine, bool include_events)
{
    json::Value root(json::Value::object_type{});
    root.set("ok", true);
    root.set("summary", summary(engine));
    root.set("accounts", accounts(engine));
    root.set("lanes", lanes(engine));
    root.set("settlements", records(engine));
    root.set("audit", audit(InvariantAuditor::inspect(engine)));
    root.set("reconciliation", reconciliation(engine));
    root.set("replay", replay(engine));
    root.set("queue", queue(engine));
    if (include_events) {
        root.set("events", events(engine));
    }
    return root;
}

std::string JsonReport::stringify(const SettlementEngine& engine, bool include_events, bool pretty)
{
    return json::stringify(build(engine, include_events), pretty);
}

json::Value JsonReport::summary(const SettlementEngine& engine)
{
    EngineSummary data = engine.summary();
    json::Value out(json::Value::object_type{});
    out.set("epoch", data.epoch);
    out.set("totalSeeded", amount(data.total_seeded));
    out.set("totalSettled", amount(data.total_settled));
    out.set("totalFees", amount(data.total_fees));
    out.set("observedLocked", amount(data.observed_locked));
    out.set("confirmedLocked", amount(data.confirmed_locked));
    out.set("packetsOpen", data.packets_open);
    out.set("packetsCancelled", data.packets_cancelled);
    out.set("packetsSettled", data.packets_settled);
    out.set("duplicateConfirmations", data.duplicate_confirmations);
    out.set("warnings", data.warnings);
    return out;
}

json::Value JsonReport::accounts(const SettlementEngine& engine)
{
    json::Value out(json::Value::array_type{});
    for (const AccountId& account : engine.ledger().accounts()) {
        json::Value item(json::Value::object_type{});
        item.set("id", account.str());
        item.set("balances", balances(engine, account));
        out.push(std::move(item));
    }
    return out;
}

json::Value JsonReport::balances(const SettlementEngine& engine, const AccountId& account)
{
    json::Value out(json::Value::array_type{});
    for (const Balance& balance : engine.ledger().balances_for(account)) {
        json::Value item(json::Value::object_type{});
        item.set("asset", balance.asset.str());
        item.set("available", amount(balance.available));
        item.set("observedLocked", amount(balance.observed_locked));
        item.set("confirmedLocked", amount(balance.confirmed_locked));
        item.set("received", amount(balance.received));
        item.set("feesPaid", amount(balance.fees_paid));
        item.set("feesEarned", amount(balance.fees_earned));
        out.push(std::move(item));
    }
    return out;
}

json::Value JsonReport::lanes(const SettlementEngine& engine)
{
    json::Value out(json::Value::array_type{});
    for (const auto& entry : engine.lanes()) {
        const LaneConfig& lane = entry.second;
        json::Value item(json::Value::object_type{});
        item.set("id", lane.id.str());
        item.set("asset", lane.asset.str());
        item.set("operator", lane.operator_account.str());
        item.set("timeoutEpochs", lane.policy.timeout_epochs);
        item.set("feeBps", lane.policy.fee_bps);
        item.set("maxRetries", lane.policy.max_retries);
        item.set("minPacketAmount", amount(lane.policy.min_packet_amount));
        item.set("maxPacketAmount", amount(lane.policy.max_packet_amount));
        out.push(std::move(item));
    }
    return out;
}

json::Value JsonReport::records(const SettlementEngine& engine)
{
    json::Value out(json::Value::array_type{});
    for (const auto& entry : engine.records()) {
        const SettlementRecord& record = entry.second;
        json::Value item(json::Value::object_type{});
        item.set("packet", record.packet.str());
        item.set("lane", record.lane.str());
        item.set("asset", record.asset.str());
        item.set("source", record.source.str());
        item.set("recipient", record.recipient.str());
        item.set("operator", record.operator_account.str());
        item.set("priority", to_string(record.priority));
        item.set("gross", amount(record.amounts.gross));
        item.set("net", amount(record.amounts.net));
        item.set("fee", amount(record.amounts.fee));
        item.set("observedStatus", to_string(record.observed_status));
        item.set("confirmedStatus", to_string(record.confirmed_status));
        item.set("observedEpoch", record.observed_epoch);
        item.set("timeoutEpoch", record.timeout_epoch);
        item.set("lastUpdateEpoch", record.last_update_epoch);
        item.set("attempt", record.attempt);
        item.set("ackCount", static_cast<std::int64_t>(record.acknowledgements.size()));
        item.set("receipt", ReceiptCodec::compact_label(record));
        item.set("fingerprint", ReceiptCodec::fingerprint_hex(record));
        item.set("memo", record.memo);
        out.push(std::move(item));
    }
    return out;
}

json::Value JsonReport::events(const SettlementEngine& engine)
{
    json::Value out(json::Value::array_type{});
    for (const EngineEvent& event : engine.events()) {
        json::Value item(json::Value::object_type{});
        item.set("kind", to_string(event.kind));
        item.set("epoch", event.epoch);
        item.set("packet", event.packet.str());
        item.set("lane", event.lane.str());
        item.set("message", event.message);
        item.set("amount", amount(event.amount));
        out.push(std::move(item));
    }
    return out;
}

json::Value JsonReport::audit(const AuditSnapshot& snapshot)
{
    json::Value out(json::Value::object_type{});
    out.set("available", amount(snapshot.available));
    out.set("observedLocked", amount(snapshot.observed_locked));
    out.set("confirmedLocked", amount(snapshot.confirmed_locked));
    out.set("received", amount(snapshot.received));
    out.set("fees", amount(snapshot.fees));
    out.set("openObserved", snapshot.open_observed);
    out.set("openConfirmed", snapshot.open_confirmed);
    out.set("closedRecords", snapshot.closed_records);
    out.set("planeDivergences", snapshot.plane_divergences);

    json::Value findings(json::Value::array_type{});
    for (const CheckFinding& finding : snapshot.findings) {
        json::Value item(json::Value::object_type{});
        item.set("severity", to_string(finding.severity));
        item.set("code", finding.code);
        item.set("message", finding.message);
        item.set("packet", finding.packet.str());
        item.set("account", finding.account.str());
        item.set("asset", finding.asset.str());
        item.set("amount", amount(finding.amount));
        findings.push(std::move(item));
    }
    out.set("findings", std::move(findings));
    return out;
}

json::Value JsonReport::reconciliation(const SettlementEngine& engine)
{
    json::Value out(json::Value::array_type{});
    for (const AssetReconciliation& item : LedgerReconciler::by_asset(engine.ledger())) {
        json::Value entry(json::Value::object_type{});
        entry.set("asset", item.asset.str());
        entry.set("accounts", item.accounts);
        entry.set("available", amount(item.available));
        entry.set("observedLocked", amount(item.observed_locked));
        entry.set("confirmedLocked", amount(item.confirmed_locked));
        entry.set("received", amount(item.received));
        entry.set("feesPaid", amount(item.fees_paid));
        entry.set("feesEarned", amount(item.fees_earned));
        entry.set("visibleTotal", amount(item.visible_total()));
        entry.set("reservedTotal", amount(item.reserved_total()));
        out.push(std::move(entry));
    }
    return out;
}

json::Value JsonReport::replay(const SettlementEngine& engine)
{
    ReplayStats stats = ReplayInspector::summarize(engine.events());
    json::Value out(json::Value::object_type{});
    out.set("firstEpoch", stats.first_epoch);
    out.set("lastEpoch", stats.last_epoch);
    out.set("totalEvents", stats.total_events);
    out.set("packetEvents", stats.packet_events);
    out.set("systemEvents", stats.system_events);

    json::Value counters(json::Value::array_type{});
    for (const EventCounter& counter : stats.counters) {
        json::Value item(json::Value::object_type{});
        item.set("kind", counter.kind);
        item.set("count", counter.count);
        counters.push(std::move(item));
    }
    out.set("counters", std::move(counters));
    return out;
}

json::Value JsonReport::queue(const SettlementEngine& engine)
{
    SettlementQueue settlement_queue = SettlementQueue::from_records(engine.records(), engine.epoch());
    QueueStats stats = settlement_queue.stats();
    json::Value out(json::Value::object_type{});
    out.set("ready", stats.ready);
    out.set("waiting", stats.waiting);
    out.set("timedOut", stats.timed_out);
    out.set("closed", stats.closed);
    out.set("readyGross", amount(stats.ready_gross));
    out.set("waitingGross", amount(stats.waiting_gross));
    out.set("timedOutGross", amount(stats.timed_out_gross));

    json::Value entries(json::Value::array_type{});
    for (const QueueEntry& entry : settlement_queue.ordered()) {
        json::Value item(json::Value::object_type{});
        item.set("packet", entry.packet.str());
        item.set("lane", entry.lane.str());
        item.set("asset", entry.asset.str());
        item.set("bucket", to_string(entry.bucket));
        item.set("priority", to_string(entry.priority));
        item.set("score", entry.score.total());
        item.set("gross", amount(entry.gross));
        item.set("observedEpoch", entry.observed_epoch);
        item.set("timeoutEpoch", entry.timeout_epoch);
        entries.push(std::move(item));
    }
    out.set("entries", std::move(entries));
    return out;
}

json::Value JsonReport::amount(Amount amount_value)
{
    return json::Value(amount_value.units());
}

} // namespace drift
