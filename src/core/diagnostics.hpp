#pragma once

#include <string>
#include <vector>

namespace drift {

struct BuildInfo {
    std::string name;
    std::string version;
    std::string standard;
    std::string compiler;
    bool debug = false;
};

struct DiagnosticLine {
    std::string key;
    std::string value;
};

class Diagnostics {
public:
    [[nodiscard]] static BuildInfo build_info();
    [[nodiscard]] static std::vector<DiagnosticLine> lines();
    [[nodiscard]] static std::string banner();
    [[nodiscard]] static std::string version_string();
    [[nodiscard]] static std::string profile_name();
    [[nodiscard]] static std::string machine_summary();
    [[nodiscard]] static std::string platform_name();

private:
    [[nodiscard]] static std::string compiler_name();
    [[nodiscard]] static std::string standard_name();
    [[nodiscard]] static std::string pointer_width();
    [[nodiscard]] static std::string endian_hint();
};

} // namespace drift
