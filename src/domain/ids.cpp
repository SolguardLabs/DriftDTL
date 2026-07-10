#include "domain/ids.hpp"

#include "core/error.hpp"

#include <cctype>
#include <ostream>

namespace drift {
namespace {

bool allowed_identifier_char(char ch)
{
    unsigned char value = static_cast<unsigned char>(ch);
    return std::isalnum(value) || ch == '-' || ch == '_' || ch == '.' || ch == ':';
}

} // namespace

Identifier::Identifier(std::string value)
    : value_(std::move(value))
{
    validate_identifier(value_, "identifier");
}

const std::string& Identifier::str() const noexcept
{
    return value_;
}

bool Identifier::empty() const noexcept
{
    return value_.empty();
}

bool Identifier::valid() const noexcept
{
    if (value_.empty() || value_.size() > 96) {
        return false;
    }
    for (char ch : value_) {
        if (!allowed_identifier_char(ch)) {
            return false;
        }
    }
    return true;
}

std::ostream& operator<<(std::ostream& os, const Identifier& id)
{
    os << id.str();
    return os;
}

void validate_identifier(std::string_view value, std::string_view field)
{
    if (value.empty()) {
        throw DriftError(ErrorCode::InvalidArgument, std::string(field) + " cannot be empty");
    }
    if (value.size() > 96) {
        throw DriftError(ErrorCode::InvalidArgument, std::string(field) + " is too long");
    }
    for (char ch : value) {
        if (!allowed_identifier_char(ch)) {
            throw DriftError(ErrorCode::InvalidArgument, std::string(field) + " contains invalid characters");
        }
    }
}

} // namespace drift

