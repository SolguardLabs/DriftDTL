#include "scenario/runner.hpp"

#include "core/error.hpp"

namespace drift {

SettlementEngine ScenarioRunner::run(const Scenario& scenario)
{
    SettlementEngine engine;
    engine.set_epoch(scenario.start_epoch);
    for (const auto& account : scenario.accounts) {
        engine.register_account(account);
    }
    for (const auto& lane : scenario.lanes) {
        engine.register_lane(lane);
    }
    for (const auto& action : scenario.actions) {
        apply(engine, action);
    }
    return engine;
}

void ScenarioRunner::apply(SettlementEngine& engine, const Action& action)
{
    switch (action.type) {
    case ActionType::Submit:
        if (!action.packet_template) {
            throw DriftError(ErrorCode::InvalidArgument, "submit action requires packet template");
        }
        engine.submit(*action.packet_template, action.epoch);
        break;
    case ActionType::Confirm:
        engine.confirm(action.packet, action.ack, action.epoch);
        break;
    case ActionType::Cancel:
        engine.cancel(action.packet, action.epoch, action.force);
        break;
    case ActionType::Retry:
        if (!action.packet_template) {
            throw DriftError(ErrorCode::InvalidArgument, "retry action requires packet template");
        }
        engine.retry(*action.packet_template, action.epoch);
        break;
    case ActionType::Advance:
        engine.advance(action.target_epoch.value_or(action.epoch));
        break;
    case ActionType::Snapshot:
        engine.snapshot(action.epoch, action.note.value_or("snapshot"));
        break;
    }
}

} // namespace drift

