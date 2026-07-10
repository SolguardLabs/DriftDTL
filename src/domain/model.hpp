#pragma once

#include "core/amount.hpp"
#include "domain/ids.hpp"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace drift {

using Epoch = std::int64_t;
using Sequence = std::int64_t;

enum class ActionType {
    Submit,
    Confirm,
    Cancel,
    Retry,
    Advance,
    Snapshot,
};

enum class PacketPriority {
    Low,
    Normal,
    High,
    Critical,
};

enum class SettlementSide {
    Observed,
    Confirmed,
};

struct BalanceConfig {
    AssetId asset;
    Amount available;
};

struct AccountConfig {
    AccountId id;
    std::vector<BalanceConfig> balances;
};

struct LanePolicy {
    std::int64_t timeout_epochs = 3;
    std::int64_t fee_bps = 0;
    std::int64_t max_retries = 1;
    Amount max_packet_amount = Amount::from_units(9'000'000'000);
    Amount min_packet_amount = Amount::zero();
    bool allow_manual_cancel = true;
    bool require_unique_ack = true;
    bool close_on_first_confirmation = true;
};

struct LaneConfig {
    LaneId id;
    AssetId asset;
    AccountId operator_account;
    LanePolicy policy;
};

struct PacketTemplate {
    PacketId id;
    LaneId lane;
    AccountId source;
    AccountId recipient;
    Amount amount;
    PacketPriority priority = PacketPriority::Normal;
    std::string memo;
};

struct Action {
    ActionType type = ActionType::Snapshot;
    Epoch epoch = 0;
    PacketId packet;
    AckId ack;
    std::optional<PacketTemplate> packet_template;
    std::optional<Epoch> target_epoch;
    std::optional<std::string> note;
    bool force = false;
};

struct Scenario {
    std::string name;
    Epoch start_epoch = 0;
    std::vector<AccountConfig> accounts;
    std::vector<LaneConfig> lanes;
    std::vector<Action> actions;
};

std::string to_string(ActionType type);
std::string to_string(PacketPriority priority);
std::string to_string(SettlementSide side);
ActionType parse_action_type(const std::string& value);
PacketPriority parse_packet_priority(const std::string& value);

} // namespace drift

