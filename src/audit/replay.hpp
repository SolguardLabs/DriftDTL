#pragma once

#include "settlement/engine.hpp"

#include <map>
#include <string>
#include <vector>

namespace drift {

struct EventCounter {
    std::string kind;
    std::int64_t count = 0;
};

struct ReplayStats {
    Epoch first_epoch = 0;
    Epoch last_epoch = 0;
    std::int64_t total_events = 0;
    std::int64_t packet_events = 0;
    std::int64_t system_events = 0;
    std::vector<EventCounter> counters;
};

class ReplayInspector {
public:
    [[nodiscard]] static ReplayStats summarize(const std::vector<EngineEvent>& events);
    [[nodiscard]] static std::vector<EngineEvent> for_packet(const std::vector<EngineEvent>& events, const PacketId& packet);
    [[nodiscard]] static std::vector<EngineEvent> for_lane(const std::vector<EngineEvent>& events, const LaneId& lane);

private:
    static void count_event(std::map<std::string, std::int64_t>& counters, const EngineEvent& event);
    static void finalize_counters(ReplayStats& stats, const std::map<std::string, std::int64_t>& counters);
};

} // namespace drift

