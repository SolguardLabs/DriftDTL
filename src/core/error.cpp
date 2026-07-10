#include "core/error.hpp"

namespace drift {

std::string_view error_code_name(ErrorCode code) noexcept
{
    switch (code) {
    case ErrorCode::InvalidArgument:
        return "invalid_argument";
    case ErrorCode::InvalidAmount:
        return "invalid_amount";
    case ErrorCode::InvalidJson:
        return "invalid_json";
    case ErrorCode::MissingField:
        return "missing_field";
    case ErrorCode::UnknownAccount:
        return "unknown_account";
    case ErrorCode::UnknownLane:
        return "unknown_lane";
    case ErrorCode::UnknownPacket:
        return "unknown_packet";
    case ErrorCode::DuplicatePacket:
        return "duplicate_packet";
    case ErrorCode::DuplicateConfirmation:
        return "duplicate_confirmation";
    case ErrorCode::PacketAlreadyClosed:
        return "packet_already_closed";
    case ErrorCode::PacketNotReady:
        return "packet_not_ready";
    case ErrorCode::PacketExpired:
        return "packet_expired";
    case ErrorCode::PacketNotExpired:
        return "packet_not_expired";
    case ErrorCode::InsufficientLiquidity:
        return "insufficient_liquidity";
    case ErrorCode::RetryLimitExceeded:
        return "retry_limit_exceeded";
    case ErrorCode::PolicyViolation:
        return "policy_violation";
    case ErrorCode::ArithmeticOverflow:
        return "arithmetic_overflow";
    case ErrorCode::InvariantViolation:
        return "invariant_violation";
    }
    return "unknown_error";
}

DriftError::DriftError(ErrorCode code, std::string message)
    : code_(code)
    , message_(std::move(message))
{
}

ErrorCode DriftError::code() const noexcept
{
    return code_;
}

const char* DriftError::what() const noexcept
{
    return message_.c_str();
}

const std::string& DriftError::message() const noexcept
{
    return message_;
}

void fail(ErrorCode code, std::string message)
{
    throw DriftError(code, std::move(message));
}

} // namespace drift

