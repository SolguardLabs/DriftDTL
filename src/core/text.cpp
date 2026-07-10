#include "core/text.hpp"

#include <algorithm>
#include <cctype>

namespace drift {

std::string trim(std::string_view input)
{
    std::size_t first = 0;
    while (first < input.size() && std::isspace(static_cast<unsigned char>(input[first]))) {
        ++first;
    }

    std::size_t last = input.size();
    while (last > first && std::isspace(static_cast<unsigned char>(input[last - 1]))) {
        --last;
    }

    return std::string(input.substr(first, last - first));
}

std::string to_lower(std::string_view input)
{
    std::string out(input);
    std::transform(out.begin(), out.end(), out.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return out;
}

std::vector<std::string> split(std::string_view input, char delimiter)
{
    std::vector<std::string> parts;
    std::size_t start = 0;
    while (start <= input.size()) {
        std::size_t next = input.find(delimiter, start);
        if (next == std::string_view::npos) {
            parts.emplace_back(input.substr(start));
            break;
        }
        parts.emplace_back(input.substr(start, next - start));
        start = next + 1;
    }
    return parts;
}

bool starts_with(std::string_view input, std::string_view prefix) noexcept
{
    return input.size() >= prefix.size() && input.substr(0, prefix.size()) == prefix;
}

bool ends_with(std::string_view input, std::string_view suffix) noexcept
{
    return input.size() >= suffix.size()
        && input.substr(input.size() - suffix.size(), suffix.size()) == suffix;
}

std::string shell_escape_hint(std::string_view input)
{
    std::string out;
    out.reserve(input.size() + 8);
    for (char ch : input) {
        if (ch == '"' || ch == '\\') {
            out.push_back('\\');
        }
        out.push_back(ch);
    }
    return out;
}

} // namespace drift

