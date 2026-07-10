#pragma once

#include "domain/model.hpp"

#include <string>

namespace drift {

struct PriorityScore {
    PacketPriority priority = PacketPriority::Normal;
    std::int64_t base_weight = 0;
    std::int64_t age_weight = 0;
    std::int64_t timeout_weight = 0;

    [[nodiscard]] std::int64_t total() const noexcept;
};

std::int64_t priority_weight(PacketPriority priority) noexcept;
PacketPriority priority_from_weight(std::int64_t weight) noexcept;
PriorityScore score_priority(PacketPriority priority, Epoch observed_epoch, Epoch current_epoch, Epoch timeout_epoch);
bool priority_before(const PriorityScore& left, const PriorityScore& right) noexcept;
std::string priority_label_with_score(const PriorityScore& score);

} // namespace drift

