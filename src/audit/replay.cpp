#include "audit/replay.hpp"

namespace drift {

ReplayStats ReplayInspector::summarize(const std::vector<EngineEvent>& events)
{
    ReplayStats stats;
    if (events.empty()) {
        return stats;
    }

    stats.first_epoch = events.front().epoch;
    stats.last_epoch = events.front().epoch;
    std::map<std::string, std::int64_t> counters;
    for (const EngineEvent& event : events) {
        stats.total_events += 1;
        if (event.packet.str() == "system") {
            stats.system_events += 1;
        } else {
            stats.packet_events += 1;
        }
        if (event.epoch < stats.first_epoch) {
            stats.first_epoch = event.epoch;
        }
        if (event.epoch > stats.last_epoch) {
            stats.last_epoch = event.epoch;
        }
        count_event(counters, event);
    }
    finalize_counters(stats, counters);
    return stats;
}

std::vector<EngineEvent> ReplayInspector::for_packet(const std::vector<EngineEvent>& events, const PacketId& packet)
{
    std::vector<EngineEvent> out;
    for (const EngineEvent& event : events) {
        if (event.packet == packet) {
            out.push_back(event);
        }
    }
    return out;
}

std::vector<EngineEvent> ReplayInspector::for_lane(const std::vector<EngineEvent>& events, const LaneId& lane)
{
    std::vector<EngineEvent> out;
    for (const EngineEvent& event : events) {
        if (event.lane == lane) {
            out.push_back(event);
        }
    }
    return out;
}

void ReplayInspector::count_event(std::map<std::string, std::int64_t>& counters, const EngineEvent& event)
{
    counters[to_string(event.kind)] += 1;
}

void ReplayInspector::finalize_counters(ReplayStats& stats, const std::map<std::string, std::int64_t>& counters)
{
    stats.counters.reserve(counters.size());
    for (const auto& entry : counters) {
        stats.counters.push_back(EventCounter{entry.first, entry.second});
    }
}

} // namespace drift

