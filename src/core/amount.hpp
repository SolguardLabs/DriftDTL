#pragma once

#include <cstdint>
#include <iosfwd>
#include <limits>
#include <string>

namespace drift {

class DriftError;

class Amount {
public:
    using value_type = std::int64_t;

    constexpr Amount() noexcept : value_(0) {}
    explicit constexpr Amount(value_type value) noexcept : value_(value) {}

    static Amount zero() noexcept;
    static Amount from_units(value_type value);
    static Amount parse(const std::string& input);

    [[nodiscard]] value_type units() const noexcept;
    [[nodiscard]] bool is_zero() const noexcept;
    [[nodiscard]] bool is_positive() const noexcept;
    [[nodiscard]] bool is_negative() const noexcept;
    [[nodiscard]] std::string str() const;

    [[nodiscard]] Amount checked_add(Amount other) const;
    [[nodiscard]] Amount checked_sub(Amount other) const;
    [[nodiscard]] Amount checked_mul(value_type factor) const;
    [[nodiscard]] Amount mul_bps_floor(std::int64_t bps) const;
    [[nodiscard]] Amount min(Amount other) const noexcept;
    [[nodiscard]] Amount max(Amount other) const noexcept;

    Amount& operator+=(Amount other);
    Amount& operator-=(Amount other);

    friend constexpr bool operator==(Amount left, Amount right) noexcept
    {
        return left.value_ == right.value_;
    }

    friend constexpr bool operator!=(Amount left, Amount right) noexcept
    {
        return !(left == right);
    }

    friend constexpr bool operator<(Amount left, Amount right) noexcept
    {
        return left.value_ < right.value_;
    }

    friend constexpr bool operator<=(Amount left, Amount right) noexcept
    {
        return left.value_ <= right.value_;
    }

    friend constexpr bool operator>(Amount left, Amount right) noexcept
    {
        return right < left;
    }

    friend constexpr bool operator>=(Amount left, Amount right) noexcept
    {
        return right <= left;
    }

private:
    value_type value_;
};

std::ostream& operator<<(std::ostream& os, Amount amount);

struct AmountBreakdown {
    Amount gross;
    Amount fee;
    Amount net;
};

AmountBreakdown split_fee(Amount gross, std::int64_t fee_bps);

} // namespace drift

