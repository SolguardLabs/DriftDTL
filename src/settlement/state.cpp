#include "settlement/state.hpp"

namespace drift {

bool SettlementRecord::observed_open() const noexcept
{
    return observed_status == ObservedStatus::Open;
}

bool SettlementRecord::confirmed_open() const noexcept
{
    return confirmed_status == ConfirmedStatus::Prepared;
}

bool SettlementRecord::terminal() const noexcept
{
    return confirmed_status == ConfirmedStatus::Settled || confirmed_status == ConfirmedStatus::Released;
}

bool SettlementRecord::expired_at(Epoch epoch) const noexcept
{
    return epoch >= timeout_epoch;
}

std::string to_string(ObservedStatus status)
{
    switch (status) {
    case ObservedStatus::Missing:
        return "missing";
    case ObservedStatus::Open:
        return "open";
    case ObservedStatus::Cancelled:
        return "cancelled";
    case ObservedStatus::Consumed:
        return "consumed";
    }
    return "unknown";
}

std::string to_string(ConfirmedStatus status)
{
    switch (status) {
    case ConfirmedStatus::Missing:
        return "missing";
    case ConfirmedStatus::Prepared:
        return "prepared";
    case ConfirmedStatus::Settled:
        return "settled";
    case ConfirmedStatus::Released:
        return "released";
    }
    return "unknown";
}

std::string to_string(EventKind kind)
{
    switch (kind) {
    case EventKind::AccountSeeded:
        return "account_seeded";
    case EventKind::LaneRegistered:
        return "lane_registered";
    case EventKind::PacketSubmitted:
        return "packet_submitted";
    case EventKind::PacketRetried:
        return "packet_retried";
    case EventKind::PacketConfirmed:
        return "packet_confirmed";
    case EventKind::PacketCancelled:
        return "packet_cancelled";
    case EventKind::DuplicateConfirmation:
        return "duplicate_confirmation";
    case EventKind::EpochAdvanced:
        return "epoch_advanced";
    case EventKind::Snapshot:
        return "snapshot";
    case EventKind::Warning:
        return "warning";
    }
    return "unknown";
}

} // namespace drift

