#include "domain/priority.hpp"

namespace drift {

std::int64_t PriorityScore::total() const noexcept
{
    return base_weight + age_weight + timeout_weight;
}

std::int64_t priority_weight(PacketPriority priority) noexcept
{
    switch (priority) {
    case PacketPriority::Low:
        return 10;
    case PacketPriority::Normal:
        return 20;
    case PacketPriority::High:
        return 40;
    case PacketPriority::Critical:
        return 80;
    }
    return 20;
}

PacketPriority priority_from_weight(std::int64_t weight) noexcept
{
    if (weight >= 80) {
        return PacketPriority::Critical;
    }
    if (weight >= 40) {
        return PacketPriority::High;
    }
    if (weight >= 20) {
        return PacketPriority::Normal;
    }
    return PacketPriority::Low;
}

PriorityScore score_priority(PacketPriority priority, Epoch observed_epoch, Epoch current_epoch, Epoch timeout_epoch)
{
    PriorityScore score;
    score.priority = priority;
    score.base_weight = priority_weight(priority);
    if (current_epoch > observed_epoch) {
        score.age_weight = (current_epoch - observed_epoch) * 2;
    }
    if (current_epoch >= timeout_epoch) {
        score.timeout_weight = 25 + (current_epoch - timeout_epoch);
    } else {
        Epoch remaining = timeout_epoch - current_epoch;
        score.timeout_weight = remaining <= 2 ? 10 : 0;
    }
    return score;
}

bool priority_before(const PriorityScore& left, const PriorityScore& right) noexcept
{
    if (left.total() != right.total()) {
        return left.total() > right.total();
    }
    return priority_weight(left.priority) > priority_weight(right.priority);
}

std::string priority_label_with_score(const PriorityScore& score)
{
    return to_string(score.priority) + ":" + std::to_string(score.total());
}

} // namespace drift

