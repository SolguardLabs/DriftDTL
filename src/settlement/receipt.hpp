#pragma once

#include "settlement/state.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace drift {

struct ReceiptView {
    PacketId packet;
    LaneId lane;
    AssetId asset;
    AccountId source;
    AccountId recipient;
    Amount gross = Amount::zero();
    Amount net = Amount::zero();
    Amount fee = Amount::zero();
    Epoch observed_epoch = 0;
    Epoch timeout_epoch = 0;
    std::int64_t attempt = 0;
    std::int64_t ack_count = 0;
    ObservedStatus observed_status = ObservedStatus::Missing;
    ConfirmedStatus confirmed_status = ConfirmedStatus::Missing;
};

class ReceiptCodec {
public:
    [[nodiscard]] static ReceiptView view(const SettlementRecord& record);
    [[nodiscard]] static std::string canonical(const ReceiptView& receipt);
    [[nodiscard]] static std::string canonical(const SettlementRecord& record);
    [[nodiscard]] static std::uint64_t fingerprint64(const ReceiptView& receipt);
    [[nodiscard]] static std::uint64_t fingerprint64(const SettlementRecord& record);
    [[nodiscard]] static std::string fingerprint_hex(const ReceiptView& receipt);
    [[nodiscard]] static std::string fingerprint_hex(const SettlementRecord& record);
    [[nodiscard]] static std::string compact_label(const SettlementRecord& record);
    [[nodiscard]] static std::string batch_fingerprint(const std::vector<SettlementRecord>& records);

private:
    static void append_field(std::string& out, const std::string& key, const std::string& value);
    static void append_field(std::string& out, const std::string& key, Amount value);
    static void append_field(std::string& out, const std::string& key, std::int64_t value);
};

} // namespace drift

