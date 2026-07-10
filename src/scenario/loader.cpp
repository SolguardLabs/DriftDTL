#include "scenario/loader.hpp"

#include "core/amount.hpp"
#include "core/error.hpp"
#include "scenario/schema.hpp"

namespace drift {
namespace {

Amount amount_from_i64(std::int64_t value, std::string_view field)
{
    if (value < 0) {
        throw DriftError(ErrorCode::InvalidAmount, std::string(field) + " cannot be negative");
    }
    return Amount::from_units(value);
}

PacketId packet_id_from_action(const json::Value& value)
{
    const json::Value& packet = value.at("packet");
    if (packet.is_string()) {
        return PacketId(packet.as_string());
    }
    if (packet.is_object()) {
        return PacketId(json::require_string(packet, "id"));
    }
    throw DriftError(ErrorCode::InvalidJson, "action packet must be string or object");
}

} // namespace

Scenario ScenarioLoader::from_file(const std::string& path)
{
    return from_json(json::parse_file(path));
}

Scenario ScenarioLoader::from_json(const json::Value& value)
{
    Scenario scenario;
    scenario.name = json::optional_string(value, "name", "drift-scenario");
    scenario.start_epoch = json::optional_i64(value, "startEpoch", 0);

    for (const auto& account : json::require_array(value, "accounts")) {
        scenario.accounts.push_back(parse_account(account));
    }
    for (const auto& lane : json::require_array(value, "lanes")) {
        scenario.lanes.push_back(parse_lane(lane));
    }
    for (const auto& action : json::require_array(value, "actions")) {
        scenario.actions.push_back(parse_action(action));
    }

    ScenarioSchema::require_valid(scenario);
    return scenario;
}

AccountConfig ScenarioLoader::parse_account(const json::Value& value)
{
    AccountConfig account;
    account.id = AccountId(json::require_string(value, "id"));

    const json::Value* balances = value.find("balances");
    if (!balances) {
        BalanceConfig balance;
        balance.asset = AssetId(json::require_string(value, "asset"));
        balance.available = amount_from_i64(json::require_i64(value, "available"), "available");
        account.balances.push_back(balance);
        return account;
    }

    for (const auto& item : balances->as_array()) {
        account.balances.push_back(parse_balance(item));
    }
    return account;
}

BalanceConfig ScenarioLoader::parse_balance(const json::Value& value)
{
    BalanceConfig balance;
    balance.asset = AssetId(json::require_string(value, "asset"));
    balance.available = amount_from_i64(json::require_i64(value, "available"), "available");
    return balance;
}

LaneConfig ScenarioLoader::parse_lane(const json::Value& value)
{
    LaneConfig lane;
    lane.id = LaneId(json::require_string(value, "id"));
    lane.asset = AssetId(json::require_string(value, "asset"));
    lane.operator_account = AccountId(json::optional_string(value, "operator", "operator"));
    const json::Value* policy = value.find("policy");
    if (policy) {
        lane.policy = parse_policy(*policy);
    }
    return lane;
}

LanePolicy ScenarioLoader::parse_policy(const json::Value& value)
{
    LanePolicy policy;
    policy.timeout_epochs = json::optional_i64(value, "timeoutEpochs", policy.timeout_epochs);
    policy.fee_bps = json::optional_i64(value, "feeBps", policy.fee_bps);
    policy.max_retries = json::optional_i64(value, "maxRetries", policy.max_retries);
    policy.max_packet_amount = amount_from_i64(json::optional_i64(value, "maxPacketAmount", policy.max_packet_amount.units()), "maxPacketAmount");
    policy.min_packet_amount = amount_from_i64(json::optional_i64(value, "minPacketAmount", policy.min_packet_amount.units()), "minPacketAmount");
    policy.allow_manual_cancel = json::optional_bool(value, "allowManualCancel", policy.allow_manual_cancel);
    policy.require_unique_ack = json::optional_bool(value, "requireUniqueAck", policy.require_unique_ack);
    policy.close_on_first_confirmation = json::optional_bool(value, "closeOnFirstConfirmation", policy.close_on_first_confirmation);
    return policy;
}

Action ScenarioLoader::parse_action(const json::Value& value)
{
    Action action;
    action.type = parse_action_type(json::require_string(value, "type"));
    action.epoch = json::optional_i64(value, "epoch", 0);
    action.force = json::optional_bool(value, "force", false);
    action.note = json::optional_string(value, "note", "");
    if (action.note->empty()) {
        action.note = std::nullopt;
    }

    switch (action.type) {
    case ActionType::Submit:
    case ActionType::Retry:
        action.packet_template = parse_packet_template(value.at("packet"));
        action.packet = action.packet_template->id;
        break;
    case ActionType::Confirm:
        action.packet = packet_id_from_action(value);
        action.ack = AckId(json::require_string(value, "ack"));
        break;
    case ActionType::Cancel:
        action.packet = packet_id_from_action(value);
        break;
    case ActionType::Advance:
        action.target_epoch = json::optional_i64(value, "targetEpoch", action.epoch);
        break;
    case ActionType::Snapshot:
        break;
    }

    return action;
}

PacketTemplate ScenarioLoader::parse_packet_template(const json::Value& value)
{
    PacketTemplate packet;
    packet.id = PacketId(json::require_string(value, "id"));
    packet.lane = LaneId(json::require_string(value, "lane"));
    packet.source = AccountId(json::require_string(value, "source"));
    packet.recipient = AccountId(json::require_string(value, "recipient"));
    packet.amount = amount_from_i64(json::require_i64(value, "amount"), "amount");
    packet.priority = parse_packet_priority(json::optional_string(value, "priority", "normal"));
    packet.memo = json::optional_string(value, "memo", "");
    return packet;
}

} // namespace drift
