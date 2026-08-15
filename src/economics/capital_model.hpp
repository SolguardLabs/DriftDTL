#pragma once

#include "core/amount.hpp"
#include "domain/ids.hpp"
#include "settlement/engine.hpp"

#include <cstdint>
#include <vector>

namespace drift {

struct AssetCapitalMetrics {
    AssetId asset;
    Amount liquid;
    Amount observed_commitments;
    Amount confirmed_commitments;
    Amount settled_credits;
    Amount earned_fees;
    Amount timeout_exposure;
    std::int64_t observed_utilization_bps = 0;
    std::int64_t confirmation_coverage_bps = 0;
    std::int64_t overlap_bps = 0;
};

struct CapitalSnapshot {
    std::vector<AssetCapitalMetrics> assets;
    Amount aggregate_liquid;
    Amount aggregate_commitments;
    Amount aggregate_timeout_exposure;
    std::int64_t open_packets = 0;
    std::int64_t timed_out_packets = 0;
};

class CapitalModel {
public:
    [[nodiscard]] static CapitalSnapshot evaluate(const SettlementEngine& engine);

private:
    [[nodiscard]] static std::int64_t ratio_bps(Amount numerator, Amount denominator);
};

} // namespace drift
