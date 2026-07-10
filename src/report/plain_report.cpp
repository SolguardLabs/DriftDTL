#include "report/plain_report.hpp"

#include "ledger/reconcile.hpp"

namespace drift {

std::string PlainReport::build(const SettlementEngine& engine)
{
    std::string out;
    append_summary(out, engine);
    append_accounts(out, engine);
    append_settlements(out, engine);
    return out;
}

void PlainReport::append_summary(std::string& out, const SettlementEngine& engine)
{
    EngineSummary summary = engine.summary();
    out += "DriftDTL summary\n";
    out += "epoch: " + std::to_string(summary.epoch) + "\n";
    out += "settled: " + summary.total_settled.str() + "\n";
    out += "fees: " + summary.total_fees.str() + "\n";
    out += "observed_locked: " + summary.observed_locked.str() + "\n";
    out += "confirmed_locked: " + summary.confirmed_locked.str() + "\n";
    out += "packets_open: " + std::to_string(summary.packets_open) + "\n";
    out += "packets_cancelled: " + std::to_string(summary.packets_cancelled) + "\n";
    out += "packets_settled: " + std::to_string(summary.packets_settled) + "\n";
}

void PlainReport::append_accounts(std::string& out, const SettlementEngine& engine)
{
    out += "\nAccounts\n";
    for (const AccountExposure& exposure : LedgerReconciler::exposures(engine.ledger())) {
        out += "- " + exposure.account.str() + "/" + exposure.asset.str();
        out += " available=" + exposure.available.str();
        out += " observed=" + exposure.observed_locked.str();
        out += " confirmed=" + exposure.confirmed_locked.str();
        out += " received=" + exposure.received.str();
        out += "\n";
    }
}

void PlainReport::append_settlements(std::string& out, const SettlementEngine& engine)
{
    out += "\nSettlements\n";
    for (const auto& entry : engine.records()) {
        const SettlementRecord& record = entry.second;
        out += "- " + record.packet.str();
        out += " observed=" + to_string(record.observed_status);
        out += " confirmed=" + to_string(record.confirmed_status);
        out += " gross=" + record.amounts.gross.str();
        out += " net=" + record.amounts.net.str();
        out += " fee=" + record.amounts.fee.str();
        out += "\n";
    }
}

} // namespace drift

