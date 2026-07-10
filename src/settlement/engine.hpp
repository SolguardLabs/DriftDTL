#pragma once

#include "domain/model.hpp"
#include "ledger/account.hpp"
#include "settlement/state.hpp"

#include <map>
#include <optional>
#include <vector>

namespace drift {

struct EngineSummary {
    Epoch epoch = 0;
    Amount total_seeded = Amount::zero();
    Amount total_settled = Amount::zero();
    Amount total_fees = Amount::zero();
    Amount observed_locked = Amount::zero();
    Amount confirmed_locked = Amount::zero();
    std::int64_t packets_open = 0;
    std::int64_t packets_cancelled = 0;
    std::int64_t packets_settled = 0;
    std::int64_t duplicate_confirmations = 0;
    std::int64_t warnings = 0;
};

class SettlementEngine {
public:
    SettlementEngine() = default;

    void set_epoch(Epoch epoch);
    void register_account(const AccountConfig& account);
    void register_lane(const LaneConfig& lane);
    void submit(const PacketTemplate& packet, Epoch epoch);
    void retry(const PacketTemplate& packet, Epoch epoch);
    void confirm(const PacketId& packet, const AckId& ack, Epoch epoch);
    void cancel(const PacketId& packet, Epoch epoch, bool force);
    void advance(Epoch epoch);
    void snapshot(Epoch epoch, std::string note);

    [[nodiscard]] Epoch epoch() const noexcept;
    [[nodiscard]] const AccountLedger& ledger() const noexcept;
    [[nodiscard]] const std::map<LaneId, LaneConfig>& lanes() const noexcept;
    [[nodiscard]] const std::map<PacketId, SettlementRecord>& records() const noexcept;
    [[nodiscard]] const std::vector<EngineEvent>& events() const noexcept;
    [[nodiscard]] EngineSummary summary() const;
    [[nodiscard]] const SettlementRecord* find_record(const PacketId& packet) const noexcept;

private:
    [[nodiscard]] const LaneConfig& lane(const LaneId& id) const;
    [[nodiscard]] SettlementRecord& record_mut(const PacketId& packet);
    [[nodiscard]] const SettlementRecord& record(const PacketId& packet) const;
    [[nodiscard]] SettlementRecord build_record(const PacketTemplate& packet, const LaneConfig& lane, Epoch epoch, std::int64_t attempt) const;
    void apply_observed_reserve(const SettlementRecord& record);
    void apply_confirmed_prepare(const SettlementRecord& record);
    void consume_observed_if_open(SettlementRecord& record);
    void settle_confirmed(SettlementRecord& record);
    void append_event(EventKind kind, Epoch epoch, const PacketId& packet, const LaneId& lane, std::string message, Amount amount = Amount::zero());
    void ensure_epoch(Epoch epoch);

    Epoch epoch_ = 0;
    AccountLedger ledger_;
    std::map<LaneId, LaneConfig> lanes_;
    std::map<PacketId, SettlementRecord> records_;
    std::vector<EngineEvent> events_;
    Amount total_seeded_ = Amount::zero();
    Amount total_settled_ = Amount::zero();
    Amount total_fees_ = Amount::zero();
    std::int64_t duplicate_confirmations_ = 0;
    std::int64_t warnings_ = 0;
};

} // namespace drift

