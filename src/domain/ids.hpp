#pragma once

#include <functional>
#include <iosfwd>
#include <string>
#include <string_view>

namespace drift {

class Identifier {
public:
    Identifier() = default;
    explicit Identifier(std::string value);

    [[nodiscard]] const std::string& str() const noexcept;
    [[nodiscard]] bool empty() const noexcept;
    [[nodiscard]] bool valid() const noexcept;

    friend bool operator==(const Identifier& left, const Identifier& right) noexcept
    {
        return left.value_ == right.value_;
    }

    friend bool operator!=(const Identifier& left, const Identifier& right) noexcept
    {
        return !(left == right);
    }

    friend bool operator<(const Identifier& left, const Identifier& right) noexcept
    {
        return left.value_ < right.value_;
    }

private:
    std::string value_;
};

struct AssetId final : Identifier {
    using Identifier::Identifier;
};

struct AccountId final : Identifier {
    using Identifier::Identifier;
};

struct LaneId final : Identifier {
    using Identifier::Identifier;
};

struct PacketId final : Identifier {
    using Identifier::Identifier;
};

struct AckId final : Identifier {
    using Identifier::Identifier;
};

std::ostream& operator<<(std::ostream& os, const Identifier& id);
void validate_identifier(std::string_view value, std::string_view field);

} // namespace drift

namespace std {

template <>
struct hash<drift::Identifier> {
    size_t operator()(const drift::Identifier& id) const noexcept
    {
        return hash<string>{}(id.str());
    }
};

} // namespace std

