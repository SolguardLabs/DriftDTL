#include "core/diagnostics.hpp"

namespace drift {

BuildInfo Diagnostics::build_info()
{
    BuildInfo info;
    info.name = "DriftDTL";
    info.version = "0.1.0";
    info.standard = standard_name();
    info.compiler = compiler_name();
#if defined(NDEBUG)
    info.debug = false;
#else
    info.debug = true;
#endif
    return info;
}

std::vector<DiagnosticLine> Diagnostics::lines()
{
    BuildInfo info = build_info();
    std::vector<DiagnosticLine> out;
    out.push_back(DiagnosticLine{"name", info.name});
    out.push_back(DiagnosticLine{"version", info.version});
    out.push_back(DiagnosticLine{"standard", info.standard});
    out.push_back(DiagnosticLine{"compiler", info.compiler});
    out.push_back(DiagnosticLine{"profile", info.debug ? "debug" : "release"});
    return out;
}

std::string Diagnostics::banner()
{
    BuildInfo info = build_info();
    std::string out = info.name;
    out += " ";
    out += info.version;
    out += " (";
    out += info.standard;
    out += ", ";
    out += info.compiler;
    out += ")";
    return out;
}

std::string Diagnostics::version_string()
{
    BuildInfo info = build_info();
    return info.name + " " + info.version;
}

std::string Diagnostics::profile_name()
{
    return build_info().debug ? "debug" : "release";
}

std::string Diagnostics::machine_summary()
{
    std::string out = pointer_width();
    out += ", ";
    out += endian_hint();
    out += ", ";
    out += platform_name();
    return out;
}

std::string Diagnostics::platform_name()
{
#if defined(_WIN32)
    return "windows";
#elif defined(__APPLE__)
    return "macos";
#elif defined(__linux__)
    return "linux";
#elif defined(__unix__)
    return "unix";
#else
    return "unknown-os";
#endif
}

std::string Diagnostics::compiler_name()
{
#if defined(__clang__)
    return "clang";
#elif defined(__GNUC__)
    return "gcc";
#elif defined(_MSC_VER)
    return "msvc";
#else
    return "unknown";
#endif
}

std::string Diagnostics::standard_name()
{
#if __cplusplus >= 202302L
    return "c++23";
#elif __cplusplus >= 202002L
    return "c++20";
#elif __cplusplus >= 201703L
    return "c++17";
#else
    return "pre-c++17";
#endif
}

std::string Diagnostics::pointer_width()
{
    return std::to_string(sizeof(void*) * 8) + "-bit";
}

std::string Diagnostics::endian_hint()
{
    const unsigned int probe = 1;
    const auto* bytes = reinterpret_cast<const unsigned char*>(&probe);
    if (bytes[0] == 1) {
        return "little-endian";
    }
    return "big-endian";
}

} // namespace drift
