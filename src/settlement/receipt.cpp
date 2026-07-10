#include "settlement/receipt.hpp"

#include "core/json.hpp"

#include <algorithm>

namespace drift {
namespace {

constexpr std::uint64_t fnv_offset = 14695981039346656037ull;
constexpr std::uint64_t fnv_prime = 1099511628211ull;

std::uint64_t fnv1a(std::string_view value, std::uint64_t seed = fnv_offset)
{
    std::uint64_t hash = seed;
    for (unsigned char ch : value) {
        hash ^= static_cast<std::uint64_t>(ch);
        hash *= fnv_prime;
    }
    return hash;
}

std::string hex64(std::uint64_t value)
{
    constexpr char alphabet[] = "0123456789abcdef";
    std::string out(16, '0');
    for (int i = 15; i >= 0; --i) {
        out[static_cast<std::size_t>(i)] = alphabet[value & 0x0F];
        value >>= 4;
    }
    return out;
}

std::vector<SettlementRecord> sorted_records(std::vector<SettlementRecord> records)
{
    std::stable_sort(records.begin(), records.end(), [](const SettlementRecord& left, const SettlementRecord& right) {
        return left.packet.str() < right.packet.str();
    });
    return records;
}

} // namespace

ReceiptView ReceiptCodec::view(const SettlementRecord& record)
{
    ReceiptView out;
    out.packet = record.packet;
    out.lane = record.lane;
    out.asset = record.asset;
    out.source = record.source;
    out.recipient = record.recipient;
    out.gross = record.amounts.gross;
    out.net = record.amounts.net;
    out.fee = record.amounts.fee;
    out.observed_epoch = record.observed_epoch;
    out.timeout_epoch = record.timeout_epoch;
    out.attempt = record.attempt;
    out.ack_count = static_cast<std::int64_t>(record.acknowledgements.size());
    out.observed_status = record.observed_status;
    out.confirmed_status = record.confirmed_status;
    return out;
}

std::string ReceiptCodec::canonical(const ReceiptView& receipt)
{
    std::string out;
    out.reserve(256);
    append_field(out, "packet", receipt.packet.str());
    append_field(out, "lane", receipt.lane.str());
    append_field(out, "asset", receipt.asset.str());
    append_field(out, "source", receipt.source.str());
    append_field(out, "recipient", receipt.recipient.str());
    append_field(out, "gross", receipt.gross);
    append_field(out, "net", receipt.net);
    append_field(out, "fee", receipt.fee);
    append_field(out, "observedEpoch", receipt.observed_epoch);
    append_field(out, "timeoutEpoch", receipt.timeout_epoch);
    append_field(out, "attempt", receipt.attempt);
    append_field(out, "ackCount", receipt.ack_count);
    append_field(out, "observedStatus", to_string(receipt.observed_status));
    append_field(out, "confirmedStatus", to_string(receipt.confirmed_status));
    return out;
}

std::string ReceiptCodec::canonical(const SettlementRecord& record)
{
    return canonical(view(record));
}

std::uint64_t ReceiptCodec::fingerprint64(const ReceiptView& receipt)
{
    return fnv1a(canonical(receipt));
}

std::uint64_t ReceiptCodec::fingerprint64(const SettlementRecord& record)
{
    return fingerprint64(view(record));
}

std::string ReceiptCodec::fingerprint_hex(const ReceiptView& receipt)
{
    return hex64(fingerprint64(receipt));
}

std::string ReceiptCodec::fingerprint_hex(const SettlementRecord& record)
{
    return fingerprint_hex(view(record));
}

std::string ReceiptCodec::compact_label(const SettlementRecord& record)
{
    std::string out = record.packet.str();
    out += ":";
    out += record.lane.str();
    out += ":";
    out += fingerprint_hex(record).substr(0, 8);
    return out;
}

std::string ReceiptCodec::batch_fingerprint(const std::vector<SettlementRecord>& records)
{
    std::vector<SettlementRecord> sorted = sorted_records(records);
    std::uint64_t hash = fnv_offset;
    for (const SettlementRecord& record : sorted) {
        hash = fnv1a(canonical(record), hash);
    }
    return hex64(hash);
}

void ReceiptCodec::append_field(std::string& out, const std::string& key, const std::string& value)
{
    if (!out.empty()) {
        out.push_back('|');
    }
    out += key;
    out.push_back('=');
    out += json::escape(value);
}

void ReceiptCodec::append_field(std::string& out, const std::string& key, Amount value)
{
    append_field(out, key, value.str());
}

void ReceiptCodec::append_field(std::string& out, const std::string& key, std::int64_t value)
{
    append_field(out, key, std::to_string(value));
}

} // namespace drift

