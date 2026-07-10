#pragma once

#include "ledger/account.hpp"

#include <map>
#include <vector>

namespace drift {

struct AssetReconciliation {
    AssetId asset;
    Amount available = Amount::zero();
    Amount observed_locked = Amount::zero();
    Amount confirmed_locked = Amount::zero();
    Amount received = Amount::zero();
    Amount fees_paid = Amount::zero();
    Amount fees_earned = Amount::zero();
    std::int64_t accounts = 0;

    [[nodiscard]] Amount visible_total() const;
    [[nodiscard]] Amount reserved_total() const;
    [[nodiscard]] Amount activity_total() const;
};

struct AccountExposure {
    AccountId account;
    AssetId asset;
    Amount available = Amount::zero();
    Amount observed_locked = Amount::zero();
    Amount confirmed_locked = Amount::zero();
    Amount received = Amount::zero();
    std::int64_t reserved_bps = 0;
};

class LedgerReconciler {
public:
    [[nodiscard]] static std::vector<AssetReconciliation> by_asset(const AccountLedger& ledger);
    [[nodiscard]] static std::vector<AccountExposure> exposures(const AccountLedger& ledger);
    [[nodiscard]] static Amount total_available(const AccountLedger& ledger);
    [[nodiscard]] static Amount total_reserved(const AccountLedger& ledger);
    [[nodiscard]] static Amount total_received(const AccountLedger& ledger);

private:
    static void add_balance(std::map<AssetId, AssetReconciliation>& out, const Balance& balance);
    [[nodiscard]] static std::int64_t reserved_ratio_bps(const Balance& balance);
};

} // namespace drift

