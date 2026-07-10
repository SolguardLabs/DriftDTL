#pragma once

#include "core/amount.hpp"
#include "settlement/engine.hpp"

#include <string>
#include <vector>

namespace drift {

enum class CheckSeverity {
    Info,
    Warning,
    Error,
};

struct CheckFinding {
    CheckSeverity severity = CheckSeverity::Info;
    std::string code;
    std::string message;
    PacketId packet;
    AccountId account;
    AssetId asset;
    Amount amount = Amount::zero();
};

struct AuditSnapshot {
    Amount available = Amount::zero();
    Amount observed_locked = Amount::zero();
    Amount confirmed_locked = Amount::zero();
    Amount received = Amount::zero();
    Amount fees = Amount::zero();
    std::int64_t open_observed = 0;
    std::int64_t open_confirmed = 0;
    std::int64_t closed_records = 0;
    std::int64_t plane_divergences = 0;
    std::vector<CheckFinding> findings;
};

class InvariantAuditor {
public:
    [[nodiscard]] static AuditSnapshot inspect(const SettlementEngine& engine);

private:
    static void inspect_balances(const SettlementEngine& engine, AuditSnapshot& snapshot);
    static void inspect_records(const SettlementEngine& engine, AuditSnapshot& snapshot);
    static void add_finding(
        AuditSnapshot& snapshot,
        CheckSeverity severity,
        std::string code,
        std::string message,
        PacketId packet = PacketId("system"),
        AccountId account = AccountId("system"),
        AssetId asset = AssetId("unit"),
        Amount amount = Amount::zero());
};

std::string to_string(CheckSeverity severity);

} // namespace drift

