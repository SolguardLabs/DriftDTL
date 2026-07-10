#pragma once

#include "core/amount.hpp"
#include "domain/model.hpp"

#include <set>
#include <string>
#include <vector>

namespace drift {

enum class ObservedStatus {
    Missing,
    Open,
    Cancelled,
    Consumed,
};

enum class ConfirmedStatus {
    Missing,
    Prepared,
    Settled,
    Released,
};

enum class EventKind {
    AccountSeeded,
    LaneRegistered,
    PacketSubmitted,
    PacketRetried,
    PacketConfirmed,
    PacketCancelled,
    DuplicateConfirmation,
    EpochAdvanced,
    Snapshot,
    Warning,
};

struct SettlementAmounts {
    Amount gross;
    Amount net;
    Amount fee;
};

struct SettlementRecord {
    PacketId packet;
    LaneId lane;
    AssetId asset;
    AccountId source;
    AccountId recipient;
    AccountId operator_account;
    PacketPriority priority = PacketPriority::Normal;
    std::string memo;
    SettlementAmounts amounts;
    Epoch observed_epoch = 0;
    Epoch last_update_epoch = 0;
    Epoch timeout_epoch = 0;
    Sequence observed_sequence = 0;
    Sequence confirmed_sequence = 0;
    std::int64_t attempt = 0;
    ObservedStatus observed_status = ObservedStatus::Missing;
    ConfirmedStatus confirmed_status = ConfirmedStatus::Missing;
    std::set<std::string> acknowledgements;
    std::vector<std::string> notes;

    [[nodiscard]] bool observed_open() const noexcept;
    [[nodiscard]] bool confirmed_open() const noexcept;
    [[nodiscard]] bool terminal() const noexcept;
    [[nodiscard]] bool expired_at(Epoch epoch) const noexcept;
};

struct EngineEvent {
    EventKind kind = EventKind::Snapshot;
    Epoch epoch = 0;
    PacketId packet;
    LaneId lane;
    std::string message;
    Amount amount = Amount::zero();
};

std::string to_string(ObservedStatus status);
std::string to_string(ConfirmedStatus status);
std::string to_string(EventKind kind);

} // namespace drift

