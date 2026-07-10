#include "ledger/reconcile.hpp"

namespace drift {

Amount AssetReconciliation::visible_total() const
{
    return available.checked_add(received);
}

Amount AssetReconciliation::reserved_total() const
{
    return observed_locked.checked_add(confirmed_locked);
}

Amount AssetReconciliation::activity_total() const
{
    return received.checked_add(fees_earned);
}

std::vector<AssetReconciliation> LedgerReconciler::by_asset(const AccountLedger& ledger)
{
    std::map<AssetId, AssetReconciliation> grouped;
    for (const AccountId& account : ledger.accounts()) {
        for (const Balance& balance : ledger.balances_for(account)) {
            add_balance(grouped, balance);
        }
    }

    std::vector<AssetReconciliation> out;
    out.reserve(grouped.size());
    for (const auto& entry : grouped) {
        out.push_back(entry.second);
    }
    return out;
}

std::vector<AccountExposure> LedgerReconciler::exposures(const AccountLedger& ledger)
{
    std::vector<AccountExposure> out;
    for (const AccountId& account : ledger.accounts()) {
        for (const Balance& balance : ledger.balances_for(account)) {
            AccountExposure exposure;
            exposure.account = account;
            exposure.asset = balance.asset;
            exposure.available = balance.available;
            exposure.observed_locked = balance.observed_locked;
            exposure.confirmed_locked = balance.confirmed_locked;
            exposure.received = balance.received;
            exposure.reserved_bps = reserved_ratio_bps(balance);
            out.push_back(exposure);
        }
    }
    return out;
}

Amount LedgerReconciler::total_available(const AccountLedger& ledger)
{
    Amount total = Amount::zero();
    for (const AccountId& account : ledger.accounts()) {
        for (const Balance& balance : ledger.balances_for(account)) {
            total += balance.available;
        }
    }
    return total;
}

Amount LedgerReconciler::total_reserved(const AccountLedger& ledger)
{
    Amount total = Amount::zero();
    for (const AccountId& account : ledger.accounts()) {
        for (const Balance& balance : ledger.balances_for(account)) {
            total += balance.observed_locked;
            total += balance.confirmed_locked;
        }
    }
    return total;
}

Amount LedgerReconciler::total_received(const AccountLedger& ledger)
{
    Amount total = Amount::zero();
    for (const AccountId& account : ledger.accounts()) {
        for (const Balance& balance : ledger.balances_for(account)) {
            total += balance.received;
        }
    }
    return total;
}

void LedgerReconciler::add_balance(std::map<AssetId, AssetReconciliation>& out, const Balance& balance)
{
    AssetReconciliation& item = out[balance.asset];
    item.asset = balance.asset;
    item.available += balance.available;
    item.observed_locked += balance.observed_locked;
    item.confirmed_locked += balance.confirmed_locked;
    item.received += balance.received;
    item.fees_paid += balance.fees_paid;
    item.fees_earned += balance.fees_earned;
    item.accounts += 1;
}

std::int64_t LedgerReconciler::reserved_ratio_bps(const Balance& balance)
{
    Amount denominator = balance.available.checked_add(balance.observed_locked).checked_add(balance.confirmed_locked);
    if (denominator.is_zero()) {
        return 0;
    }
    Amount numerator = balance.observed_locked.checked_add(balance.confirmed_locked);
    return static_cast<std::int64_t>((numerator.units() * 10'000) / denominator.units());
}

} // namespace drift

