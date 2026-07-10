#include "core/diagnostics.hpp"
#include "core/error.hpp"
#include "core/json.hpp"
#include "report/json_report.hpp"
#include "report/plain_report.hpp"
#include "scenario/loader.hpp"
#include "scenario/runner.hpp"

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

namespace {

struct CliOptions {
    std::string command;
    std::string fixture;
    bool json = false;
    bool events = false;
    bool pretty = false;
};

bool has_flag(const std::vector<std::string>& args, const std::string& flag)
{
    for (const std::string& arg : args) {
        if (arg == flag) {
            return true;
        }
    }
    return false;
}

void print_usage(std::ostream& os)
{
    os << "DriftDTL settlement simulator\n"
       << drift::Diagnostics::banner() << "\n"
       << "\n"
       << "Usage:\n"
       << "  driftdtl validate <fixture.json>\n"
       << "  driftdtl run <fixture.json> [--json] [--events] [--pretty]\n"
       << "\n"
       << "Commands:\n"
       << "  validate   Parse and execute a fixture without printing state\n"
       << "  run        Execute a fixture and emit final state\n";
}

CliOptions parse_cli(int argc, char** argv)
{
    std::vector<std::string> args;
    for (int i = 1; i < argc; ++i) {
        args.emplace_back(argv[i]);
    }

    if (args.empty() || args[0] == "--help" || args[0] == "-h") {
        print_usage(std::cout);
        std::exit(0);
    }
    if (args[0] == "--version" || args[0] == "-V") {
        std::cout << drift::Diagnostics::version_string() << " "
                  << drift::Diagnostics::profile_name() << " "
                  << drift::Diagnostics::machine_summary() << "\n";
        std::exit(0);
    }

    CliOptions options;
    options.command = args[0];
    options.json = has_flag(args, "--json");
    options.events = has_flag(args, "--events");
    options.pretty = has_flag(args, "--pretty");

    for (std::size_t i = 1; i < args.size(); ++i) {
        if (!args[i].empty() && args[i][0] == '-') {
            continue;
        }
        options.fixture = args[i];
        break;
    }

    if (options.fixture.empty()) {
        throw drift::DriftError(drift::ErrorCode::InvalidArgument, "fixture path is required");
    }
    return options;
}

} // namespace

int main(int argc, char** argv)
{
    try {
        CliOptions options = parse_cli(argc, argv);
        drift::Scenario scenario = drift::ScenarioLoader::from_file(options.fixture);
        drift::SettlementEngine engine = drift::ScenarioRunner::run(scenario);

        if (options.command == "validate") {
            std::cout << "ok\n";
            return 0;
        }
        if (options.command != "run") {
            throw drift::DriftError(drift::ErrorCode::InvalidArgument, "unknown command: " + options.command);
        }

        if (options.json) {
            std::cout << drift::JsonReport::stringify(engine, options.events, options.pretty);
            if (!options.pretty) {
                std::cout << '\n';
            }
            return 0;
        }
        std::cout << drift::PlainReport::build(engine);
        return 0;
    } catch (const drift::DriftError& error) {
        std::cerr << drift::error_code_name(error.code()) << ": " << error.message() << '\n';
        return 1;
    } catch (const std::exception& error) {
        std::cerr << "runtime_error: " << error.what() << '\n';
        return 1;
    }
    return 0;
}
