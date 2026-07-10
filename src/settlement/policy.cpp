#include "settlement/policy.hpp"

#include "core/error.hpp"
#include "settlement/state.hpp"

namespace drift {

void PolicyEvaluator::validate_lane(const LaneConfig& lane)
{
    if (lane.id.empty()) {
        throw DriftError(ErrorCode::InvalidArgument, "lane id is required");
    }
    if (lane.asset.empty()) {
        throw DriftError(ErrorCode::InvalidArgument, "lane asset is required");
    }
    if (lane.operator_account.empty()) {
        throw DriftError(ErrorCode::InvalidArgument, "lane operator account is required");
    }
    if (lane.policy.timeout_epochs <= 0) {
        throw DriftError(ErrorCode::PolicyViolation, "lane timeout must be positive");
    }
    if (lane.policy.fee_bps < 0 || lane.policy.fee_bps > 2'000) {
        throw DriftError(ErrorCode::PolicyViolation, "lane fee is outside supported bounds");
    }
    if (lane.policy.max_retries < 0 || lane.policy.max_retries > 20) {
        throw DriftError(ErrorCode::PolicyViolation, "lane retry limit is outside supported bounds");
    }
    if (lane.policy.min_packet_amount > lane.policy.max_packet_amount) {
        throw DriftError(ErrorCode::PolicyViolation, "lane packet amount bounds are inverted");
    }
}

void PolicyEvaluator::validate_packet(const LaneConfig& lane, const PacketTemplate& packet)
{
    if (packet.id.empty()) {
        throw DriftError(ErrorCode::InvalidArgument, "packet id is required");
    }
    if (packet.lane != lane.id) {
        throw DriftError(ErrorCode::UnknownLane, "packet lane does not match registered lane");
    }
    if (packet.source.empty() || packet.recipient.empty()) {
        throw DriftError(ErrorCode::InvalidArgument, "packet accounts are required");
    }
    if (packet.amount < lane.policy.min_packet_amount) {
        throw DriftError(ErrorCode::PolicyViolation, "packet amount below lane minimum");
    }
    if (packet.amount > lane.policy.max_packet_amount) {
        throw DriftError(ErrorCode::PolicyViolation, "packet amount above lane maximum");
    }
}

SettlementAmounts PolicyEvaluator::quote(const LaneConfig& lane, Amount amount)
{
    AmountBreakdown fee = split_fee(amount, lane.policy.fee_bps);
    return SettlementAmounts{fee.gross, fee.net, fee.fee};
}

bool PolicyEvaluator::can_retry(const LaneConfig& lane, std::int64_t current_attempt)
{
    return current_attempt < lane.policy.max_retries;
}

Epoch PolicyEvaluator::timeout_for(const LaneConfig& lane, Epoch observed_epoch)
{
    return observed_epoch + lane.policy.timeout_epochs;
}

} // namespace drift
