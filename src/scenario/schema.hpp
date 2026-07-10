#pragma once

#include "domain/model.hpp"

#include <set>
#include <string>
#include <vector>

namespace drift {

enum class SchemaIssueKind {
    DuplicateAccount,
    DuplicateLane,
    DuplicatePacket,
    MissingAccount,
    MissingLane,
    EpochRegression,
    InvalidRetry,
};

struct SchemaIssue {
    SchemaIssueKind kind = SchemaIssueKind::MissingLane;
    std::string message;
};

class ScenarioSchema {
public:
    [[nodiscard]] static std::vector<SchemaIssue> inspect(const Scenario& scenario);
    static void require_valid(const Scenario& scenario);

private:
    struct SeenState {
        std::set<AccountId> accounts;
        std::set<LaneId> lanes;
        std::set<PacketId> packets;
    };

    static void collect_accounts(const Scenario& scenario, SeenState& seen, std::vector<SchemaIssue>& issues);
    static void collect_lanes(const Scenario& scenario, SeenState& seen, std::vector<SchemaIssue>& issues);
    static void inspect_actions(const Scenario& scenario, SeenState& seen, std::vector<SchemaIssue>& issues);
    static void inspect_packet_template(const PacketTemplate& packet, const SeenState& seen, std::vector<SchemaIssue>& issues);
    static void add(std::vector<SchemaIssue>& issues, SchemaIssueKind kind, std::string message);
};

std::string to_string(SchemaIssueKind kind);

} // namespace drift

