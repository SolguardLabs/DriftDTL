#include "audit/invariants.hpp"

namespace drift {

AuditSnapshot InvariantAuditor::inspect(const SettlementEngine& engine)
{
    AuditSnapshot snapshot;
    inspect_balances(engine, snapshot);
    inspect_records(engine, snapshot);
    return snapshot;
}

void InvariantAuditor::inspect_balances(const SettlementEngine& engine, AuditSnapshot& snapshot)
{
    for (const AccountId& account : engine.ledger().accounts()) {
        for (const Balance& balance : engine.ledger().balances_for(account)) {
            snapshot.available += balance.available;
            snapshot.observed_locked += balance.observed_locked;
            snapshot.confirmed_locked += balance.confirmed_locked;
            snapshot.received += balance.received;
            snapshot.fees += balance.fees_earned;

            if (balance.available.is_negative()) {
                add_finding(
                    snapshot,
                    CheckSeverity::Error,
                    "negative_available",
                    "available balance is negative",
                    PacketId("system"),
                    account,
                    balance.asset,
                    balance.available);
            }
            if (balance.observed_locked.is_negative()) {
                add_finding(
                    snapshot,
                    CheckSeverity::Error,
                    "negative_observed_lock",
                    "observed lock is negative",
                    PacketId("system"),
                    account,
                    balance.asset,
                    balance.observed_locked);
            }
            if (balance.confirmed_locked.is_negative()) {
                add_finding(
                    snapshot,
                    CheckSeverity::Error,
                    "negative_confirmed_lock",
                    "confirmed lock is negative",
                    PacketId("system"),
                    account,
                    balance.asset,
                    balance.confirmed_locked);
            }
        }
    }
}

void InvariantAuditor::inspect_records(const SettlementEngine& engine, AuditSnapshot& snapshot)
{
    for (const auto& entry : engine.records()) {
        const SettlementRecord& record = entry.second;
        if (record.observed_status == ObservedStatus::Open) {
            snapshot.open_observed += 1;
        }
        if (record.confirmed_status == ConfirmedStatus::Prepared) {
            snapshot.open_confirmed += 1;
        }
        if (record.confirmed_status == ConfirmedStatus::Settled) {
            snapshot.closed_records += 1;
        }
        if (to_string(record.observed_status) != to_string(record.confirmed_status)) {
            snapshot.plane_divergences += 1;
        }
        if (record.confirmed_status == ConfirmedStatus::Settled && record.amounts.net.is_zero()) {
            add_finding(
                snapshot,
                CheckSeverity::Warning,
                "zero_net_settlement",
                "settled record has zero net amount",
                record.packet,
                record.source,
                record.asset);
        }
        if (record.observed_status == ObservedStatus::Open && record.confirmed_status == ConfirmedStatus::Settled) {
            add_finding(
                snapshot,
                CheckSeverity::Warning,
                "open_plane_after_close",
                "record has an open visible plane after confirmed close",
                record.packet,
                record.source,
                record.asset,
                record.amounts.gross);
        }
    }
}

void InvariantAuditor::add_finding(
    AuditSnapshot& snapshot,
    CheckSeverity severity,
    std::string code,
    std::string message,
    PacketId packet,
    AccountId account,
    AssetId asset,
    Amount amount)
{
    snapshot.findings.push_back(CheckFinding{
        severity,
        std::move(code),
        std::move(message),
        std::move(packet),
        std::move(account),
        std::move(asset),
        amount,
    });
}

std::string to_string(CheckSeverity severity)
{
    switch (severity) {
    case CheckSeverity::Info:
        return "info";
    case CheckSeverity::Warning:
        return "warning";
    case CheckSeverity::Error:
        return "error";
    }
    return "info";
}

} // namespace drift
