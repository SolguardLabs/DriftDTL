#include "scenario/schema.hpp"

#include "core/error.hpp"

namespace drift {

std::vector<SchemaIssue> ScenarioSchema::inspect(const Scenario& scenario)
{
    SeenState seen;
    std::vector<SchemaIssue> issues;
    collect_accounts(scenario, seen, issues);
    collect_lanes(scenario, seen, issues);
    inspect_actions(scenario, seen, issues);
    return issues;
}

void ScenarioSchema::require_valid(const Scenario& scenario)
{
    std::vector<SchemaIssue> issues = inspect(scenario);
    if (!issues.empty()) {
        const SchemaIssue& first = issues.front();
        throw DriftError(ErrorCode::InvalidArgument, to_string(first.kind) + ": " + first.message);
    }
}

void ScenarioSchema::collect_accounts(const Scenario& scenario, SeenState& seen, std::vector<SchemaIssue>& issues)
{
    for (const AccountConfig& account : scenario.accounts) {
        if (!seen.accounts.insert(account.id).second) {
            add(issues, SchemaIssueKind::DuplicateAccount, "duplicate account " + account.id.str());
        }
        std::set<AssetId> assets;
        for (const BalanceConfig& balance : account.balances) {
            if (!assets.insert(balance.asset).second) {
                add(
                    issues,
                    SchemaIssueKind::DuplicateAccount,
                    "duplicate balance for " + account.id.str() + "/" + balance.asset.str());
            }
        }
    }
}

void ScenarioSchema::collect_lanes(const Scenario& scenario, SeenState& seen, std::vector<SchemaIssue>& issues)
{
    for (const LaneConfig& lane : scenario.lanes) {
        if (!seen.lanes.insert(lane.id).second) {
            add(issues, SchemaIssueKind::DuplicateLane, "duplicate lane " + lane.id.str());
        }
        if (seen.accounts.count(lane.operator_account) == 0) {
            add(issues, SchemaIssueKind::MissingAccount, "lane operator account is not declared: " + lane.operator_account.str());
        }
    }
}

void ScenarioSchema::inspect_actions(const Scenario& scenario, SeenState& seen, std::vector<SchemaIssue>& issues)
{
    Epoch last_epoch = scenario.start_epoch;
    for (const Action& action : scenario.actions) {
        if (action.epoch < last_epoch) {
            add(issues, SchemaIssueKind::EpochRegression, "action epoch moves backwards");
        }
        last_epoch = action.epoch;

        switch (action.type) {
        case ActionType::Submit:
            if (action.packet_template) {
                inspect_packet_template(*action.packet_template, seen, issues);
                if (!seen.packets.insert(action.packet_template->id).second) {
                    add(issues, SchemaIssueKind::DuplicatePacket, "duplicate packet " + action.packet_template->id.str());
                }
            }
            break;
        case ActionType::Retry:
            if (action.packet_template) {
                inspect_packet_template(*action.packet_template, seen, issues);
            }
            break;
        case ActionType::Confirm:
        case ActionType::Cancel:
            if (seen.packets.count(action.packet) == 0) {
                add(issues, SchemaIssueKind::DuplicatePacket, "packet action references unknown packet " + action.packet.str());
            }
            break;
        case ActionType::Advance:
        case ActionType::Snapshot:
            break;
        }
    }
}

void ScenarioSchema::inspect_packet_template(const PacketTemplate& packet, const SeenState& seen, std::vector<SchemaIssue>& issues)
{
    if (seen.lanes.count(packet.lane) == 0) {
        add(issues, SchemaIssueKind::MissingLane, "packet references unknown lane " + packet.lane.str());
    }
    if (seen.accounts.count(packet.source) == 0) {
        add(issues, SchemaIssueKind::MissingAccount, "packet source is not declared: " + packet.source.str());
    }
    if (seen.accounts.count(packet.recipient) == 0) {
        add(issues, SchemaIssueKind::MissingAccount, "packet recipient is not declared: " + packet.recipient.str());
    }
}

void ScenarioSchema::add(std::vector<SchemaIssue>& issues, SchemaIssueKind kind, std::string message)
{
    issues.push_back(SchemaIssue{kind, std::move(message)});
}

std::string to_string(SchemaIssueKind kind)
{
    switch (kind) {
    case SchemaIssueKind::DuplicateAccount:
        return "duplicate_account";
    case SchemaIssueKind::DuplicateLane:
        return "duplicate_lane";
    case SchemaIssueKind::DuplicatePacket:
        return "duplicate_packet";
    case SchemaIssueKind::MissingAccount:
        return "missing_account";
    case SchemaIssueKind::MissingLane:
        return "missing_lane";
    case SchemaIssueKind::EpochRegression:
        return "epoch_regression";
    case SchemaIssueKind::InvalidRetry:
        return "invalid_retry";
    }
    return "schema_issue";
}

} // namespace drift

