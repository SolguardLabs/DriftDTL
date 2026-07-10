#include "settlement/queue.hpp"

#include <algorithm>

namespace drift {

SettlementQueue SettlementQueue::from_records(const std::map<PacketId, SettlementRecord>& records, Epoch current_epoch)
{
    SettlementQueue queue;
    queue.entries_.reserve(records.size());
    for (const auto& entry : records) {
        queue.entries_.push_back(convert(entry.second, current_epoch));
    }
    return queue;
}

const std::vector<QueueEntry>& SettlementQueue::entries() const noexcept
{
    return entries_;
}

std::vector<QueueEntry> SettlementQueue::ordered() const
{
    std::vector<QueueEntry> out = entries_;
    std::stable_sort(out.begin(), out.end(), [](const QueueEntry& left, const QueueEntry& right) {
        if (left.bucket != right.bucket) {
            return static_cast<int>(left.bucket) < static_cast<int>(right.bucket);
        }
        if (left.score.total() != right.score.total()) {
            return left.score.total() > right.score.total();
        }
        if (left.timeout_epoch != right.timeout_epoch) {
            return left.timeout_epoch < right.timeout_epoch;
        }
        return left.packet.str() < right.packet.str();
    });
    return out;
}

std::vector<QueueEntry> SettlementQueue::bucket(QueueBucket requested) const
{
    std::vector<QueueEntry> out;
    for (const QueueEntry& entry : entries_) {
        if (entry.bucket == requested) {
            out.push_back(entry);
        }
    }
    return out;
}

std::optional<QueueEntry> SettlementQueue::next_ready() const
{
    std::vector<QueueEntry> ready = bucket(QueueBucket::Ready);
    if (ready.empty()) {
        return std::nullopt;
    }
    std::stable_sort(ready.begin(), ready.end(), [](const QueueEntry& left, const QueueEntry& right) {
        if (left.score.total() != right.score.total()) {
            return left.score.total() > right.score.total();
        }
        return left.observed_epoch < right.observed_epoch;
    });
    return ready.front();
}

QueueStats SettlementQueue::stats() const
{
    QueueStats out;
    for (const QueueEntry& entry : entries_) {
        switch (entry.bucket) {
        case QueueBucket::Ready:
            out.ready += 1;
            out.ready_gross += entry.gross;
            break;
        case QueueBucket::Waiting:
            out.waiting += 1;
            out.waiting_gross += entry.gross;
            break;
        case QueueBucket::TimedOut:
            out.timed_out += 1;
            out.timed_out_gross += entry.gross;
            break;
        case QueueBucket::Closed:
            out.closed += 1;
            break;
        }
    }
    return out;
}

QueueEntry SettlementQueue::convert(const SettlementRecord& record, Epoch current_epoch)
{
    QueueEntry out;
    out.packet = record.packet;
    out.lane = record.lane;
    out.source = record.source;
    out.asset = record.asset;
    out.gross = record.amounts.gross;
    out.priority = record.priority;
    out.observed_epoch = record.observed_epoch;
    out.timeout_epoch = record.timeout_epoch;
    out.observed_status = record.observed_status;
    out.confirmed_status = record.confirmed_status;
    out.score = score_priority(record.priority, record.observed_epoch, current_epoch, record.timeout_epoch);
    out.bucket = classify(record, current_epoch);
    return out;
}

QueueBucket SettlementQueue::classify(const SettlementRecord& record, Epoch current_epoch)
{
    if (record.confirmed_status == ConfirmedStatus::Settled || record.confirmed_status == ConfirmedStatus::Released) {
        return QueueBucket::Closed;
    }
    if (record.observed_status == ObservedStatus::Cancelled || current_epoch >= record.timeout_epoch) {
        return QueueBucket::TimedOut;
    }
    if (record.observed_status == ObservedStatus::Open && record.confirmed_status == ConfirmedStatus::Prepared) {
        return QueueBucket::Ready;
    }
    return QueueBucket::Waiting;
}

std::string to_string(QueueBucket bucket)
{
    switch (bucket) {
    case QueueBucket::Ready:
        return "ready";
    case QueueBucket::Waiting:
        return "waiting";
    case QueueBucket::TimedOut:
        return "timed_out";
    case QueueBucket::Closed:
        return "closed";
    }
    return "unknown";
}

} // namespace drift

