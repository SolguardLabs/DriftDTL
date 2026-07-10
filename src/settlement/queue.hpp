#pragma once

#include "domain/priority.hpp"
#include "settlement/state.hpp"

#include <map>
#include <optional>
#include <vector>

namespace drift {

enum class QueueBucket {
    Ready,
    Waiting,
    TimedOut,
    Closed,
};

struct QueueEntry {
    PacketId packet;
    LaneId lane;
    AccountId source;
    AssetId asset;
    Amount gross = Amount::zero();
    PacketPriority priority = PacketPriority::Normal;
    Epoch observed_epoch = 0;
    Epoch timeout_epoch = 0;
    ObservedStatus observed_status = ObservedStatus::Missing;
    ConfirmedStatus confirmed_status = ConfirmedStatus::Missing;
    PriorityScore score;
    QueueBucket bucket = QueueBucket::Waiting;
};

struct QueueStats {
    std::int64_t ready = 0;
    std::int64_t waiting = 0;
    std::int64_t timed_out = 0;
    std::int64_t closed = 0;
    Amount ready_gross = Amount::zero();
    Amount waiting_gross = Amount::zero();
    Amount timed_out_gross = Amount::zero();
};

class SettlementQueue {
public:
    static SettlementQueue from_records(const std::map<PacketId, SettlementRecord>& records, Epoch current_epoch);

    [[nodiscard]] const std::vector<QueueEntry>& entries() const noexcept;
    [[nodiscard]] std::vector<QueueEntry> ordered() const;
    [[nodiscard]] std::vector<QueueEntry> bucket(QueueBucket bucket) const;
    [[nodiscard]] std::optional<QueueEntry> next_ready() const;
    [[nodiscard]] QueueStats stats() const;

private:
    static QueueEntry convert(const SettlementRecord& record, Epoch current_epoch);
    static QueueBucket classify(const SettlementRecord& record, Epoch current_epoch);

    std::vector<QueueEntry> entries_;
};

std::string to_string(QueueBucket bucket);

} // namespace drift
