#pragma once

#include "audit/invariants.hpp"
#include "core/json.hpp"
#include "settlement/engine.hpp"

namespace drift {

class JsonReport {
public:
    [[nodiscard]] static json::Value build(const SettlementEngine& engine, bool include_events);
    [[nodiscard]] static std::string stringify(const SettlementEngine& engine, bool include_events, bool pretty);

private:
    [[nodiscard]] static json::Value summary(const SettlementEngine& engine);
    [[nodiscard]] static json::Value accounts(const SettlementEngine& engine);
    [[nodiscard]] static json::Value balances(const SettlementEngine& engine, const AccountId& account);
    [[nodiscard]] static json::Value lanes(const SettlementEngine& engine);
    [[nodiscard]] static json::Value records(const SettlementEngine& engine);
    [[nodiscard]] static json::Value events(const SettlementEngine& engine);
    [[nodiscard]] static json::Value audit(const AuditSnapshot& snapshot);
    [[nodiscard]] static json::Value reconciliation(const SettlementEngine& engine);
    [[nodiscard]] static json::Value replay(const SettlementEngine& engine);
    [[nodiscard]] static json::Value queue(const SettlementEngine& engine);
    [[nodiscard]] static json::Value amount(Amount amount);
};

} // namespace drift
