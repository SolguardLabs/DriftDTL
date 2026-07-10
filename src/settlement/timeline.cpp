#include "settlement/timeline.hpp"

namespace drift {

bool TimelineProfile::before_timeout() const noexcept
{
    return current_epoch < timeout_epoch;
}

bool TimelineProfile::after_timeout() const noexcept
{
    return current_epoch >= timeout_epoch;
}

bool TimelineProfile::inside_grace() const noexcept
{
    return current_epoch >= timeout_epoch && current_epoch <= timeout_epoch + grace_epochs;
}

TimelineProfile Timeline::profile(const SettlementRecord& record, Epoch current_epoch, Epoch grace_epochs)
{
    TimelineProfile out;
    out.observed_epoch = record.observed_epoch;
    out.timeout_epoch = record.timeout_epoch;
    out.current_epoch = current_epoch;
    out.grace_epochs = grace_epochs;
    out.lag = current_epoch - record.observed_epoch;
    out.delivery = classify(record.observed_epoch, record.timeout_epoch, current_epoch, grace_epochs);
    return out;
}

DeliveryClass Timeline::classify(Epoch observed_epoch, Epoch timeout_epoch, Epoch current_epoch, Epoch grace_epochs)
{
    if (current_epoch < observed_epoch) {
        return DeliveryClass::Early;
    }
    if (current_epoch < timeout_epoch) {
        return DeliveryClass::OnTime;
    }
    if (current_epoch <= timeout_epoch + grace_epochs) {
        return DeliveryClass::Grace;
    }
    if (current_epoch <= timeout_epoch + grace_epochs + 1) {
        return DeliveryClass::Late;
    }
    return DeliveryClass::Expired;
}

std::vector<TimelineCheckpoint> Timeline::checkpoints(const SettlementRecord& record)
{
    std::vector<TimelineCheckpoint> out;
    out.push_back(TimelineCheckpoint{"observed", record.observed_epoch, to_string(record.observed_status)});
    out.push_back(TimelineCheckpoint{"timeout", record.timeout_epoch, "scheduled"});
    out.push_back(TimelineCheckpoint{"updated", record.last_update_epoch, to_string(record.confirmed_status)});
    return out;
}

bool Timeline::confirmation_window_open(const SettlementRecord& record, Epoch current_epoch, Epoch grace_epochs)
{
    DeliveryClass delivery = classify(record.observed_epoch, record.timeout_epoch, current_epoch, grace_epochs);
    return delivery == DeliveryClass::OnTime || delivery == DeliveryClass::Grace || delivery == DeliveryClass::Late;
}

bool Timeline::cancellation_window_open(const SettlementRecord& record, Epoch current_epoch)
{
    return record.observed_status == ObservedStatus::Open && current_epoch >= record.timeout_epoch;
}

Epoch Timeline::remaining(Epoch current_epoch, Epoch target_epoch)
{
    if (current_epoch >= target_epoch) {
        return 0;
    }
    return target_epoch - current_epoch;
}

std::string Timeline::describe(const TimelineProfile& profile)
{
    std::string out = to_string(profile.delivery);
    out += ": observed=" + std::to_string(profile.observed_epoch);
    out += " timeout=" + std::to_string(profile.timeout_epoch);
    out += " current=" + std::to_string(profile.current_epoch);
    out += " lag=" + std::to_string(profile.lag);
    return out;
}

std::string to_string(DeliveryClass delivery)
{
    switch (delivery) {
    case DeliveryClass::Early:
        return "early";
    case DeliveryClass::OnTime:
        return "on_time";
    case DeliveryClass::Grace:
        return "grace";
    case DeliveryClass::Late:
        return "late";
    case DeliveryClass::Expired:
        return "expired";
    }
    return "unknown";
}

} // namespace drift

