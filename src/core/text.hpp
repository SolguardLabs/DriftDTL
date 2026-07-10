#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace drift {

std::string trim(std::string_view input);
std::string to_lower(std::string_view input);
std::vector<std::string> split(std::string_view input, char delimiter);
bool starts_with(std::string_view input, std::string_view prefix) noexcept;
bool ends_with(std::string_view input, std::string_view suffix) noexcept;
std::string shell_escape_hint(std::string_view input);

} // namespace drift

