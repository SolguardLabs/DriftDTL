#include "economics/capital_model.hpp"

#include "ledger/reconcile.hpp"

#include <algorithm>
#include <map>

namespace drift {

CapitalSnapshot CapitalModel::evaluate(const SettlementEngine& engine)
{
    CapitalSnapshot snapshot;
    std::map<AssetId, AssetCapitalMetrics> metrics;

    for (const AssetReconciliation& item : LedgerReconciler::by_asset(engine.ledger())) {
        AssetCapitalMetrics& asset = metrics[item.asset];
        asset.asset = item.asset;
        asset.liquid = item.available;
        asset.observed_commitments = item.observed_locked;
        asset.confirmed_commitments = item.confirmed_locked;
        asset.settled_credits = item.received;
        asset.earned_fees = item.fees_earned;
    }

    for (const auto& entry : engine.records()) {
        const SettlementRecord& record = entry.second;
        AssetCapitalMetrics& asset = metrics[record.asset];
        asset.asset = record.asset;
        if (record.observed_status == ObservedStatus::Open) {
            snapshot.open_packets += 1;
        }
        if (record.observed_status == ObservedStatus::Cancelled ||
            (record.observed_status == ObservedStatus::Open && engine.epoch() >= record.timeout_epoch)) {
            asset.timeout_exposure += record.amounts.gross;
            snapshot.timed_out_packets += 1;
        }
    }

    snapshot.assets.reserve(metrics.size());
    for (auto& entry : metrics) {
        AssetCapitalMetrics& asset = entry.second;
        const Amount operating_base = asset.liquid.checked_add(asset.observed_commitments);
        asset.observed_utilization_bps = ratio_bps(asset.observed_commitments, operating_base);
        asset.confirmation_coverage_bps = asset.confirmed_commitments.is_zero()
            ? 10'000
            : ratio_bps(operating_base, asset.confirmed_commitments);
        const Amount overlap_denominator = asset.observed_commitments.max(asset.confirmed_commitments);
        asset.overlap_bps = ratio_bps(
            asset.observed_commitments.min(asset.confirmed_commitments), overlap_denominator);

        snapshot.aggregate_liquid += asset.liquid;
        snapshot.aggregate_commitments += asset.observed_commitments;
        snapshot.aggregate_commitments += asset.confirmed_commitments;
        snapshot.aggregate_timeout_exposure += asset.timeout_exposure;
        snapshot.assets.push_back(asset);
    }
    return snapshot;
}

std::int64_t CapitalModel::ratio_bps(Amount numerator, Amount denominator)
{
    if (denominator.is_zero() || numerator.is_zero()) {
        return 0;
    }
    const long double scaled = static_cast<long double>(numerator.units()) * 10'000.0L;
    const long double quotient = scaled / static_cast<long double>(denominator.units());
    return static_cast<std::int64_t>(std::min(quotient, 1'000'000.0L));
}

} // namespace drift
