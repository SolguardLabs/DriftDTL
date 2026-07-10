#pragma once

#include "settlement/engine.hpp"

#include <string>

namespace drift {

class PlainReport {
public:
    [[nodiscard]] static std::string build(const SettlementEngine& engine);

private:
    static void append_summary(std::string& out, const SettlementEngine& engine);
    static void append_accounts(std::string& out, const SettlementEngine& engine);
    static void append_settlements(std::string& out, const SettlementEngine& engine);
};

} // namespace drift

