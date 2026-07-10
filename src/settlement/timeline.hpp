#pragma once

#include "domain/model.hpp"
#include "settlement/state.hpp"

#include <string>
#include <vector>

namespace drift {

enum class DeliveryClass {
    Early,
    OnTime,
    Grace,
    Late,
    Expired,
};

struct TimelineProfile {
    Epoch observed_epoch = 0;
    Epoch timeout_epoch = 0;
    Epoch current_epoch = 0;
    Epoch grace_epochs = 0;
    Epoch lag = 0;
    DeliveryClass delivery = DeliveryClass::OnTime;

    [[nodiscard]] bool before_timeout() const noexcept;
    [[nodiscard]] bool after_timeout() const noexcept;
    [[nodiscard]] bool inside_grace() const noexcept;
};

struct TimelineCheckpoint {
    std::string label;
    Epoch epoch = 0;
    std::string status;
};

class Timeline {
public:
    [[nodiscard]] static TimelineProfile profile(const SettlementRecord& record, Epoch current_epoch, Epoch grace_epochs);
    [[nodiscard]] static DeliveryClass classify(Epoch observed_epoch, Epoch timeout_epoch, Epoch current_epoch, Epoch grace_epochs);
    [[nodiscard]] static std::vector<TimelineCheckpoint> checkpoints(const SettlementRecord& record);
    [[nodiscard]] static bool confirmation_window_open(const SettlementRecord& record, Epoch current_epoch, Epoch grace_epochs);
    [[nodiscard]] static bool cancellation_window_open(const SettlementRecord& record, Epoch current_epoch);
    [[nodiscard]] static Epoch remaining(Epoch current_epoch, Epoch target_epoch);
    [[nodiscard]] static std::string describe(const TimelineProfile& profile);
};

std::string to_string(DeliveryClass delivery);

} // namespace drift

