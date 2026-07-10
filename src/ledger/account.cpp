#include "ledger/account.hpp"

#include "core/error.hpp"

namespace drift {

Amount Balance::total_visible() const
{
    return available.checked_add(received);
}

Amount Balance::total_reserved() const
{
    return observed_locked.checked_add(confirmed_locked);
}

void AccountLedger::create_account(const AccountId& account)
{
    accounts_.try_emplace(account, BalanceMap{});
}

void AccountLedger::seed(const AccountId& account, const AssetId& asset, Amount amount)
{
    if (!amount.is_positive() && !amount.is_zero()) {
        throw DriftError(ErrorCode::InvalidAmount, "seed amount must be non-negative");
    }
    Balance& entry = ensure_balance(account, asset);
    entry.available += amount;
}

bool AccountLedger::has_account(const AccountId& account) const
{
    return accounts_.find(account) != accounts_.end();
}

bool AccountLedger::has_balance(const AccountId& account, const AssetId& asset) const
{
    auto account_it = accounts_.find(account);
    if (account_it == accounts_.end()) {
        return false;
    }
    return account_it->second.find(asset) != account_it->second.end();
}

const Balance& AccountLedger::balance(const AccountId& account, const AssetId& asset) const
{
    auto account_it = accounts_.find(account);
    if (account_it == accounts_.end()) {
        throw DriftError(ErrorCode::UnknownAccount, "unknown account: " + account.str());
    }
    auto balance_it = account_it->second.find(asset);
    if (balance_it == account_it->second.end()) {
        throw DriftError(ErrorCode::InvalidArgument, "missing asset balance for account: " + account.str());
    }
    return balance_it->second;
}

Balance& AccountLedger::balance_mut(const AccountId& account, const AssetId& asset)
{
    return ensure_balance(account, asset);
}

std::vector<AccountId> AccountLedger::accounts() const
{
    std::vector<AccountId> out;
    out.reserve(accounts_.size());
    for (const auto& entry : accounts_) {
        out.push_back(entry.first);
    }
    return out;
}

std::vector<Balance> AccountLedger::balances_for(const AccountId& account) const
{
    std::vector<Balance> out;
    const auto& balances = account_balances(account);
    out.reserve(balances.size());
    for (const auto& entry : balances) {
        out.push_back(entry.second);
    }
    return out;
}

void AccountLedger::reserve_observed(const AccountId& account, const AssetId& asset, Amount amount)
{
    Balance& entry = ensure_balance(account, asset);
    if (entry.available < amount) {
        throw DriftError(ErrorCode::InsufficientLiquidity, "insufficient available liquidity for observed reserve");
    }
    entry.available -= amount;
    entry.observed_locked += amount;
}

void AccountLedger::release_observed(const AccountId& account, const AssetId& asset, Amount amount)
{
    Balance& entry = ensure_balance(account, asset);
    if (entry.observed_locked < amount) {
        throw DriftError(ErrorCode::InvariantViolation, "observed reserve release exceeds locked amount");
    }
    entry.observed_locked -= amount;
    entry.available += amount;
}

void AccountLedger::consume_observed(const AccountId& account, const AssetId& asset, Amount amount)
{
    Balance& entry = ensure_balance(account, asset);
    if (entry.observed_locked < amount) {
        throw DriftError(ErrorCode::InvariantViolation, "observed reserve consumption exceeds locked amount");
    }
    entry.observed_locked -= amount;
}

void AccountLedger::attach_confirmed(const AccountId& account, const AssetId& asset, Amount amount)
{
    Balance& entry = ensure_balance(account, asset);
    entry.confirmed_locked += amount;
}

void AccountLedger::release_confirmed(const AccountId& account, const AssetId& asset, Amount amount)
{
    Balance& entry = ensure_balance(account, asset);
    if (entry.confirmed_locked < amount) {
        throw DriftError(ErrorCode::InvariantViolation, "confirmed release exceeds locked amount");
    }
    entry.confirmed_locked -= amount;
}

void AccountLedger::settle_confirmed(const AccountId& source, const AccountId& recipient, const AssetId& asset, Amount net, Amount fee)
{
    Balance& source_entry = ensure_balance(source, asset);
    Amount gross = net.checked_add(fee);
    if (source_entry.confirmed_locked < gross) {
        throw DriftError(ErrorCode::InvariantViolation, "confirmed settlement exceeds confirmed reserve");
    }
    source_entry.confirmed_locked -= gross;
    source_entry.fees_paid += fee;
    credit_received(recipient, asset, net);
}

void AccountLedger::credit_received(const AccountId& account, const AssetId& asset, Amount amount)
{
    Balance& entry = ensure_balance(account, asset);
    entry.received += amount;
    entry.available += amount;
}

void AccountLedger::credit_fee(const AccountId& account, const AssetId& asset, Amount amount)
{
    Balance& entry = ensure_balance(account, asset);
    entry.fees_earned += amount;
    entry.available += amount;
}

Balance& AccountLedger::ensure_balance(const AccountId& account, const AssetId& asset)
{
    create_account(account);
    BalanceMap& balances = accounts_[account];
    auto insert_result = balances.try_emplace(
        asset,
        Balance{
            asset,
            Amount::zero(),
            Amount::zero(),
            Amount::zero(),
            Amount::zero(),
            Amount::zero(),
            Amount::zero(),
        });
    return insert_result.first->second;
}

const AccountLedger::BalanceMap& AccountLedger::account_balances(const AccountId& account) const
{
    auto account_it = accounts_.find(account);
    if (account_it == accounts_.end()) {
        throw DriftError(ErrorCode::UnknownAccount, "unknown account: " + account.str());
    }
    return account_it->second;
}

} // namespace drift
