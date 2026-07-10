#pragma once

#include "domain/model.hpp"
#include "settlement/engine.hpp"

namespace drift {

class ScenarioRunner {
public:
    [[nodiscard]] static SettlementEngine run(const Scenario& scenario);
    static void apply(SettlementEngine& engine, const Action& action);
};

} // namespace drift

