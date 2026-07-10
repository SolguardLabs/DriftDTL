#pragma once

#include "core/amount.hpp"
#include "domain/ids.hpp"

#include <map>
#include <vector>

namespace drift {

struct Balance {
    AssetId asset;
    Amount available;
    Amount observed_locked;
    Amount confirmed_locked;
    Amount received;
    Amount fees_paid;
    Amount fees_earned;

    [[nodiscard]] Amount total_visible() const;
    [[nodiscard]] Amount total_reserved() const;
};

class AccountLedger {
public:
    void create_account(const AccountId& account);
    void seed(const AccountId& account, const AssetId& asset, Amount amount);

    [[nodiscard]] bool has_account(const AccountId& account) const;
    [[nodiscard]] bool has_balance(const AccountId& account, const AssetId& asset) const;
    [[nodiscard]] const Balance& balance(const AccountId& account, const AssetId& asset) const;
    [[nodiscard]] Balance& balance_mut(const AccountId& account, const AssetId& asset);
    [[nodiscard]] std::vector<AccountId> accounts() const;
    [[nodiscard]] std::vector<Balance> balances_for(const AccountId& account) const;

    void reserve_observed(const AccountId& account, const AssetId& asset, Amount amount);
    void release_observed(const AccountId& account, const AssetId& asset, Amount amount);
    void consume_observed(const AccountId& account, const AssetId& asset, Amount amount);
    void attach_confirmed(const AccountId& account, const AssetId& asset, Amount amount);
    void release_confirmed(const AccountId& account, const AssetId& asset, Amount amount);
    void settle_confirmed(const AccountId& source, const AccountId& recipient, const AssetId& asset, Amount net, Amount fee);
    void credit_received(const AccountId& account, const AssetId& asset, Amount amount);
    void credit_fee(const AccountId& account, const AssetId& asset, Amount amount);

private:
    using BalanceMap = std::map<AssetId, Balance>;

    [[nodiscard]] Balance& ensure_balance(const AccountId& account, const AssetId& asset);
    [[nodiscard]] const BalanceMap& account_balances(const AccountId& account) const;

    std::map<AccountId, BalanceMap> accounts_;
};

} // namespace drift
