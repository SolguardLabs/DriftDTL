#pragma once

#include <exception>
#include <string>
#include <string_view>

namespace drift {

enum class ErrorCode {
    InvalidArgument,
    InvalidAmount,
    InvalidJson,
    MissingField,
    UnknownAccount,
    UnknownLane,
    UnknownPacket,
    DuplicatePacket,
    DuplicateConfirmation,
    PacketAlreadyClosed,
    PacketNotReady,
    PacketExpired,
    PacketNotExpired,
    InsufficientLiquidity,
    RetryLimitExceeded,
    PolicyViolation,
    ArithmeticOverflow,
    InvariantViolation,
};

std::string_view error_code_name(ErrorCode code) noexcept;

class DriftError final : public std::exception {
public:
    DriftError(ErrorCode code, std::string message);

    [[nodiscard]] ErrorCode code() const noexcept;
    [[nodiscard]] const char* what() const noexcept override;
    [[nodiscard]] const std::string& message() const noexcept;

private:
    ErrorCode code_;
    std::string message_;
};

[[noreturn]] void fail(ErrorCode code, std::string message);

} // namespace drift

