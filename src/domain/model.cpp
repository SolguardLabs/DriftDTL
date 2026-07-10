#include "domain/model.hpp"

#include "core/error.hpp"
#include "core/text.hpp"

namespace drift {

std::string to_string(ActionType type)
{
    switch (type) {
    case ActionType::Submit:
        return "submit";
    case ActionType::Confirm:
        return "confirm";
    case ActionType::Cancel:
        return "cancel";
    case ActionType::Retry:
        return "retry";
    case ActionType::Advance:
        return "advance";
    case ActionType::Snapshot:
        return "snapshot";
    }
    return "unknown";
}

std::string to_string(PacketPriority priority)
{
    switch (priority) {
    case PacketPriority::Low:
        return "low";
    case PacketPriority::Normal:
        return "normal";
    case PacketPriority::High:
        return "high";
    case PacketPriority::Critical:
        return "critical";
    }
    return "normal";
}

std::string to_string(SettlementSide side)
{
    switch (side) {
    case SettlementSide::Observed:
        return "observed";
    case SettlementSide::Confirmed:
        return "confirmed";
    }
    return "unknown";
}

ActionType parse_action_type(const std::string& value)
{
    std::string normalized = to_lower(value);
    if (normalized == "submit") {
        return ActionType::Submit;
    }
    if (normalized == "confirm") {
        return ActionType::Confirm;
    }
    if (normalized == "cancel") {
        return ActionType::Cancel;
    }
    if (normalized == "retry") {
        return ActionType::Retry;
    }
    if (normalized == "advance") {
        return ActionType::Advance;
    }
    if (normalized == "snapshot") {
        return ActionType::Snapshot;
    }
    throw DriftError(ErrorCode::InvalidArgument, "unknown action type: " + value);
}

PacketPriority parse_packet_priority(const std::string& value)
{
    std::string normalized = to_lower(value);
    if (normalized == "low") {
        return PacketPriority::Low;
    }
    if (normalized == "normal") {
        return PacketPriority::Normal;
    }
    if (normalized == "high") {
        return PacketPriority::High;
    }
    if (normalized == "critical") {
        return PacketPriority::Critical;
    }
    throw DriftError(ErrorCode::InvalidArgument, "unknown packet priority: " + value);
}

} // namespace drift

