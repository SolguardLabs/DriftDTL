#include "core/amount.hpp"

#include "core/error.hpp"

#include <charconv>
#include <ostream>
#include <string_view>

namespace drift {
namespace {

constexpr auto max_amount = std::numeric_limits<Amount::value_type>::max();
constexpr auto min_amount = std::numeric_limits<Amount::value_type>::min();

void require_non_negative(Amount amount, std::string_view field)
{
    if (amount.is_negative()) {
        throw DriftError(ErrorCode::InvalidAmount, std::string(field) + " must be non-negative");
    }
}

} // namespace

Amount Amount::zero() noexcept
{
    return Amount(0);
}

Amount Amount::from_units(value_type value)
{
    Amount amount(value);
    require_non_negative(amount, "amount");
    return amount;
}

Amount Amount::parse(const std::string& input)
{
    if (input.empty()) {
        throw DriftError(ErrorCode::InvalidAmount, "empty amount");
    }

    value_type parsed = 0;
    const char* begin = input.data();
    const char* end = input.data() + input.size();
    auto [ptr, ec] = std::from_chars(begin, end, parsed);
    if (ec != std::errc{} || ptr != end) {
        throw DriftError(ErrorCode::InvalidAmount, "invalid amount: " + input);
    }
    return from_units(parsed);
}

Amount::value_type Amount::units() const noexcept
{
    return value_;
}

bool Amount::is_zero() const noexcept
{
    return value_ == 0;
}

bool Amount::is_positive() const noexcept
{
    return value_ > 0;
}

bool Amount::is_negative() const noexcept
{
    return value_ < 0;
}

std::string Amount::str() const
{
    return std::to_string(value_);
}

Amount Amount::checked_add(Amount other) const
{
    if (other.value_ > 0 && value_ > max_amount - other.value_) {
        throw DriftError(ErrorCode::ArithmeticOverflow, "amount addition overflow");
    }
    if (other.value_ < 0 && value_ < min_amount - other.value_) {
        throw DriftError(ErrorCode::ArithmeticOverflow, "amount addition underflow");
    }
    return Amount(value_ + other.value_);
}

Amount Amount::checked_sub(Amount other) const
{
    if (other.value_ < 0 && value_ > max_amount + other.value_) {
        throw DriftError(ErrorCode::ArithmeticOverflow, "amount subtraction overflow");
    }
    if (other.value_ > 0 && value_ < min_amount + other.value_) {
        throw DriftError(ErrorCode::ArithmeticOverflow, "amount subtraction underflow");
    }
    return Amount(value_ - other.value_);
}

Amount Amount::checked_mul(value_type factor) const
{
    if (factor < 0) {
        throw DriftError(ErrorCode::InvalidAmount, "negative multiplication factor");
    }
    if (value_ != 0 && factor > max_amount / value_) {
        throw DriftError(ErrorCode::ArithmeticOverflow, "amount multiplication overflow");
    }
    return Amount(value_ * factor);
}

Amount Amount::mul_bps_floor(std::int64_t bps) const
{
    if (bps < 0 || bps > 10'000) {
        throw DriftError(ErrorCode::PolicyViolation, "basis points outside allowed range");
    }
    if (value_ > max_amount / bps && bps != 0) {
        throw DriftError(ErrorCode::ArithmeticOverflow, "basis point multiplication overflow");
    }
    return Amount((value_ * bps) / 10'000);
}

Amount Amount::min(Amount other) const noexcept
{
    return *this <= other ? *this : other;
}

Amount Amount::max(Amount other) const noexcept
{
    return *this >= other ? *this : other;
}

Amount& Amount::operator+=(Amount other)
{
    *this = checked_add(other);
    return *this;
}

Amount& Amount::operator-=(Amount other)
{
    *this = checked_sub(other);
    return *this;
}

std::ostream& operator<<(std::ostream& os, Amount amount)
{
    os << amount.units();
    return os;
}

AmountBreakdown split_fee(Amount gross, std::int64_t fee_bps)
{
    require_non_negative(gross, "gross");
    Amount fee = gross.mul_bps_floor(fee_bps);
    Amount net = gross.checked_sub(fee);
    return AmountBreakdown{gross, fee, net};
}

} // namespace drift

