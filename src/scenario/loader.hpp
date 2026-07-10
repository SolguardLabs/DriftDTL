#pragma once

#include "core/json.hpp"
#include "domain/model.hpp"

#include <string>

namespace drift {

class ScenarioLoader {
public:
    [[nodiscard]] static Scenario from_file(const std::string& path);
    [[nodiscard]] static Scenario from_json(const json::Value& value);

private:
    [[nodiscard]] static AccountConfig parse_account(const json::Value& value);
    [[nodiscard]] static BalanceConfig parse_balance(const json::Value& value);
    [[nodiscard]] static LaneConfig parse_lane(const json::Value& value);
    [[nodiscard]] static LanePolicy parse_policy(const json::Value& value);
    [[nodiscard]] static Action parse_action(const json::Value& value);
    [[nodiscard]] static PacketTemplate parse_packet_template(const json::Value& value);
};

} // namespace drift

