#pragma once

#include "domain/model.hpp"
#include "settlement/state.hpp"

namespace drift {

class PolicyEvaluator {
public:
    static void validate_lane(const LaneConfig& lane);
    static void validate_packet(const LaneConfig& lane, const PacketTemplate& packet);
    static SettlementAmounts quote(const LaneConfig& lane, Amount amount);
    static bool can_retry(const LaneConfig& lane, std::int64_t current_attempt);
    static Epoch timeout_for(const LaneConfig& lane, Epoch observed_epoch);
};

} // namespace drift
