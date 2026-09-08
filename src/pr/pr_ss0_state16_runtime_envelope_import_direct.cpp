#include "pr_ss0_state16_runtime_envelope_import_direct.h"

#include <array>
#include <cctype>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <limits>
#include <sstream>
#include <string>
#include <unordered_map>

namespace PrSS0State16RuntimeEnvelopeImportDirect {
namespace {

constexpr char kExpectedEnvelopeSource[] =
    "duck_gdb_state16_card_read_capture.py";
constexpr char kExpectedEnvelopeAuthority[] =
    "validated_live_gdb_state16_runtime_typed_facts";

using ArgMap = std::unordered_map<std::string, std::string>;

uint32_t Rotr(uint32_t value, uint32_t bits) {
    return (value >> bits) | (value << (32u - bits));
}

std::string HexByte(uint8_t value) {
    constexpr char kHex[] = "0123456789abcdef";
    std::string out;
    out.push_back(kHex[(value >> 4) & 0x0F]);
    out.push_back(kHex[value & 0x0F]);
    return out;
}

std::string Sha256Hex(const uint8_t* data, std::size_t size) {
    static constexpr uint32_t kInit[8] = {
        0x6a09e667u, 0xbb67ae85u, 0x3c6ef372u, 0xa54ff53au,
        0x510e527fu, 0x9b05688cu, 0x1f83d9abu, 0x5be0cd19u};
    static constexpr uint32_t kRound[64] = {
        0x428a2f98u, 0x71374491u, 0xb5c0fbcfu, 0xe9b5dba5u,
        0x3956c25bu, 0x59f111f1u, 0x923f82a4u, 0xab1c5ed5u,
        0xd807aa98u, 0x12835b01u, 0x243185beu, 0x550c7dc3u,
        0x72be5d74u, 0x80deb1feu, 0x9bdc06a7u, 0xc19bf174u,
        0xe49b69c1u, 0xefbe4786u, 0x0fc19dc6u, 0x240ca1ccu,
        0x2de92c6fu, 0x4a7484aau, 0x5cb0a9dcu, 0x76f988dau,
        0x983e5152u, 0xa831c66du, 0xb00327c8u, 0xbf597fc7u,
        0xc6e00bf3u, 0xd5a79147u, 0x06ca6351u, 0x14292967u,
        0x27b70a85u, 0x2e1b2138u, 0x4d2c6dfcu, 0x53380d13u,
        0x650a7354u, 0x766a0abbu, 0x81c2c92eu, 0x92722c85u,
        0xa2bfe8a1u, 0xa81a664bu, 0xc24b8b70u, 0xc76c51a3u,
        0xd192e819u, 0xd6990624u, 0xf40e3585u, 0x106aa070u,
        0x19a4c116u, 0x1e376c08u, 0x2748774cu, 0x34b0bcb5u,
        0x391c0cb3u, 0x4ed8aa4au, 0x5b9cca4fu, 0x682e6ff3u,
        0x748f82eeu, 0x78a5636fu, 0x84c87814u, 0x8cc70208u,
        0x90befffau, 0xa4506cebu, 0xbef9a3f7u, 0xc67178f2u};

    std::array<uint32_t, 8> h{};
    std::memcpy(h.data(), kInit, sizeof(kInit));

    const uint64_t bitSize = static_cast<uint64_t>(size) * 8u;
    const std::size_t paddedSize = ((size + 9u + 63u) / 64u) * 64u;
    std::string padded(paddedSize, '\0');
    if (size > 0) {
        std::memcpy(&padded[0], data, size);
    }
    padded[size] = static_cast<char>(0x80);
    for (int i = 0; i < 8; ++i) {
        padded[paddedSize - 1u - static_cast<std::size_t>(i)] =
            static_cast<char>((bitSize >> (i * 8)) & 0xFFu);
    }

    for (std::size_t offset = 0; offset < paddedSize; offset += 64u) {
        uint32_t w[64]{};
        for (int i = 0; i < 16; ++i) {
            const std::size_t base = offset + static_cast<std::size_t>(i) * 4u;
            w[i] = (static_cast<uint32_t>(
                        static_cast<uint8_t>(padded[base]))
                    << 24) |
                   (static_cast<uint32_t>(
                        static_cast<uint8_t>(padded[base + 1u]))
                    << 16) |
                   (static_cast<uint32_t>(
                        static_cast<uint8_t>(padded[base + 2u]))
                    << 8) |
                   static_cast<uint32_t>(
                       static_cast<uint8_t>(padded[base + 3u]));
        }
        for (int i = 16; i < 64; ++i) {
            const uint32_t s0 =
                Rotr(w[i - 15], 7) ^ Rotr(w[i - 15], 18) ^ (w[i - 15] >> 3);
            const uint32_t s1 =
                Rotr(w[i - 2], 17) ^ Rotr(w[i - 2], 19) ^ (w[i - 2] >> 10);
            w[i] = w[i - 16] + s0 + w[i - 7] + s1;
        }

        uint32_t a = h[0];
        uint32_t b = h[1];
        uint32_t c = h[2];
        uint32_t d = h[3];
        uint32_t e = h[4];
        uint32_t f = h[5];
        uint32_t g = h[6];
        uint32_t hh = h[7];

        for (int i = 0; i < 64; ++i) {
            const uint32_t s1 = Rotr(e, 6) ^ Rotr(e, 11) ^ Rotr(e, 25);
            const uint32_t ch = (e & f) ^ ((~e) & g);
            const uint32_t temp1 = hh + s1 + ch + kRound[i] + w[i];
            const uint32_t s0 = Rotr(a, 2) ^ Rotr(a, 13) ^ Rotr(a, 22);
            const uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
            const uint32_t temp2 = s0 + maj;
            hh = g;
            g = f;
            f = e;
            e = d + temp1;
            d = c;
            c = b;
            b = a;
            a = temp1 + temp2;
        }

        h[0] += a;
        h[1] += b;
        h[2] += c;
        h[3] += d;
        h[4] += e;
        h[5] += f;
        h[6] += g;
        h[7] += hh;
    }

    std::string out;
    out.reserve(64);
    for (uint32_t value : h) {
        out += HexByte(static_cast<uint8_t>((value >> 24) & 0xFFu));
        out += HexByte(static_cast<uint8_t>((value >> 16) & 0xFFu));
        out += HexByte(static_cast<uint8_t>((value >> 8) & 0xFFu));
        out += HexByte(static_cast<uint8_t>(value & 0xFFu));
    }
    return out;
}

bool IsHexChar(char c) {
    return (c >= '0' && c <= '9') ||
           (c >= 'a' && c <= 'f') ||
           (c >= 'A' && c <= 'F');
}

int HexValue(char c) {
    if (c >= '0' && c <= '9') {
        return c - '0';
    }
    if (c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
    }
    if (c >= 'A' && c <= 'F') {
        return c - 'A' + 10;
    }
    return -1;
}

bool IsLowerHexDigest(const std::string& text) {
    if (text.size() != 64u) {
        return false;
    }
    for (char c : text) {
        if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'))) {
            return false;
        }
    }
    return true;
}

ArgMap ParseArgs(const std::string& args, std::string& error) {
    ArgMap out;
    std::istringstream stream(args);
    std::string token;
    while (stream >> token) {
        const std::size_t eq = token.find('=');
        if (eq == std::string::npos || eq == 0u || eq + 1u >= token.size()) {
            error = "bad key=value token";
            return {};
        }
        const std::string key = token.substr(0, eq);
        const std::string value = token.substr(eq + 1u);
        if (!out.emplace(key, value).second) {
            error = "duplicate key: " + key;
            return {};
        }
    }
    return out;
}

bool RequireString(
    const ArgMap& args,
    const char* key,
    const char* expected,
    std::string& error) {
    const auto it = args.find(key);
    if (it == args.end()) {
        error = std::string("missing ") + key;
        return false;
    }
    if (it->second != expected) {
        error = std::string("unexpected ") + key;
        return false;
    }
    return true;
}

bool ParseI64(const std::string& text, int64_t& out) {
    if (text.empty()) {
        return false;
    }
    char* end = nullptr;
    const long long value = std::strtoll(text.c_str(), &end, 0);
    if (end == nullptr || *end != '\0') {
        return false;
    }
    out = static_cast<int64_t>(value);
    return true;
}

bool GetI32(
    const ArgMap& args,
    const char* key,
    int32_t& out,
    std::string& error) {
    const auto it = args.find(key);
    if (it == args.end()) {
        error = std::string("missing ") + key;
        return false;
    }
    int64_t value = 0;
    if (!ParseI64(it->second, value) ||
        value < std::numeric_limits<int32_t>::min() ||
        value > std::numeric_limits<int32_t>::max()) {
        error = std::string("bad int ") + key;
        return false;
    }
    out = static_cast<int32_t>(value);
    return true;
}

bool GetU32(
    const ArgMap& args,
    const char* key,
    uint32_t& out,
    std::string& error) {
    int64_t value = 0;
    const auto it = args.find(key);
    if (it == args.end()) {
        error = std::string("missing ") + key;
        return false;
    }
    if (!ParseI64(it->second, value) ||
        value < 0 ||
        value > std::numeric_limits<uint32_t>::max()) {
        error = std::string("bad uint ") + key;
        return false;
    }
    out = static_cast<uint32_t>(value);
    return true;
}

bool GetSize(
    const ArgMap& args,
    const char* key,
    std::size_t& out,
    std::string& error) {
    int64_t value = 0;
    const auto it = args.find(key);
    if (it == args.end()) {
        error = std::string("missing ") + key;
        return false;
    }
    if (!ParseI64(it->second, value) || value < 0) {
        error = std::string("bad size ") + key;
        return false;
    }
    out = static_cast<std::size_t>(value);
    return true;
}

bool GetBoolTrue(
    const ArgMap& args,
    const char* key,
    bool& out,
    std::string& error) {
    const auto it = args.find(key);
    if (it == args.end()) {
        error = std::string("missing ") + key;
        return false;
    }
    if (it->second != "1" && it->second != "true") {
        error = std::string("required true ") + key;
        return false;
    }
    out = true;
    return true;
}

bool GetBoolFalse(
    const ArgMap& args,
    const char* key,
    bool& out,
    std::string& error) {
    const auto it = args.find(key);
    if (it == args.end()) {
        error = std::string("missing ") + key;
        return false;
    }
    if (it->second != "0" && it->second != "false") {
        error = std::string("required false ") + key;
        return false;
    }
    out = false;
    return true;
}

bool DecodePayloadHex(
    const ArgMap& args,
    State16RuntimeTypedFactsEnvelope& out,
    std::string& error) {
    const auto it = args.find("fullPayloadBytesHex");
    if (it == args.end()) {
        error = "missing fullPayloadBytesHex";
        return false;
    }
    const std::string& hex = it->second;
    const std::size_t expectedBytes =
        PrStage1SaveCardHalDirect::kCardReadBlockBytes800179B4;
    if (hex.size() != expectedBytes * 2u) {
        error = "bad fullPayloadBytesHex length";
        return false;
    }
    for (std::size_t i = 0; i < expectedBytes; ++i) {
        const char hi = hex[i * 2u];
        const char lo = hex[i * 2u + 1u];
        if (!IsHexChar(hi) || !IsHexChar(lo)) {
            error = "bad fullPayloadBytesHex";
            return false;
        }
        out.fullPayloadBytes[i] =
            static_cast<uint8_t>((HexValue(hi) << 4) | HexValue(lo));
    }

    const auto sha = args.find("fullPayloadBytesSha256");
    if (sha == args.end() || !IsLowerHexDigest(sha->second)) {
        error = "bad fullPayloadBytesSha256";
        return false;
    }
    out.fullPayloadBytesSha256 = sha->second;
    const std::string actual =
        Sha256Hex(out.fullPayloadBytes.data(), out.fullPayloadBytes.size());
    if (actual != out.fullPayloadBytesSha256) {
        error = "fullPayloadBytesSha256 mismatch";
        return false;
    }
    return true;
}

bool GetString(
    const ArgMap& args,
    const char* key,
    std::string& out,
    std::string& error) {
    const auto it = args.find(key);
    if (it == args.end()) {
        error = std::string("missing ") + key;
        return false;
    }
    out = it->second;
    return true;
}

bool CopyCommandSafeTitle(
    const ArgMap& args,
    const char* key,
    char (&out)[32],
    std::string& error) {
    std::string value;
    if (!GetString(args, key, value, error)) {
        return false;
    }
    if (value.empty() || value.size() >= sizeof(out)) {
        error = std::string("bad title ") + key;
        return false;
    }
    for (char ch : value) {
        const unsigned char byte = static_cast<unsigned char>(ch);
        if (byte < 0x21u || byte > 0x7Eu) {
            error = std::string("bad title ") + key;
            return false;
        }
    }
    std::memset(out, 0, sizeof(out));
    std::memcpy(out, value.data(), value.size());
    return true;
}

}  // namespace

bool ParseValidatedLiveGdbState16RuntimeEnvelopeArgs(
    const std::string& argsText,
    State16RuntimeTypedFactsEnvelope& out,
    std::string& error) {
    out = {};
    error.clear();
    const ArgMap args = ParseArgs(argsText, error);
    if (!error.empty()) {
        return false;
    }

    if (!RequireString(args, "source", kExpectedEnvelopeSource, error) ||
        !RequireString(args, "authority", kExpectedEnvelopeAuthority, error)) {
        return false;
    }
    out.source = kExpectedEnvelopeSource;
    out.authority = kExpectedEnvelopeAuthority;

    auto& facts = out.facts;
    if (!GetBoolTrue(args, "factsKnown", facts.factsKnown, error) ||
        !GetBoolTrue(args, "state16CallKnown", facts.state16CallKnown, error) ||
        !GetBoolTrue(args,
                     "selectedBlockKnown",
                     facts.selectedBlockKnown,
                     error) ||
        !GetI32(args, "selectedBlockIndex", facts.selectedBlockIndex, error) ||
        !GetBoolTrue(args,
                     "selectedTitleKnown",
                     facts.selectedTitleKnown,
                     error) ||
        !CopyCommandSafeTitle(args,
                              "selectedTitle",
                              facts.selectedTitle,
                              error) ||
        !GetBoolTrue(args, "rowCountKnown", facts.rowCountKnown, error) ||
        !GetI32(args, "rowCount", facts.rowCount, error) ||
        !GetBoolTrue(args, "arg2Known", facts.arg2Known, error) ||
        !GetI32(args, "arg2", facts.arg2, error) ||
        !GetBoolTrue(args, "nameAddressKnown", facts.nameAddressKnown, error) ||
        !GetU32(args, "nameAddress", facts.nameAddress, error) ||
        !GetBoolTrue(args,
                     "targetBufferAddressKnown",
                     facts.targetBufferAddressKnown,
                     error) ||
        !GetU32(args,
                "targetBufferAddress",
                facts.targetBufferAddress,
                error) ||
        !GetBoolTrue(args,
                     "payloadAddressKnown",
                     facts.payloadAddressKnown,
                     error) ||
        !GetU32(args, "payloadAddress", facts.payloadAddress, error) ||
        !GetBoolTrue(args, "blockCountKnown", facts.blockCountKnown, error) ||
        !GetI32(args, "blockCount", facts.blockCount, error) ||
        !GetBoolTrue(args, "pathCallKnown", facts.pathCallKnown, error) ||
        !GetBoolTrue(args,
                     "cardSelectorKnown",
                     facts.cardSelectorKnown,
                     error) ||
        !GetI32(args, "cardPortGp128", facts.cardPortGp128, error) ||
        !GetI32(args, "cardSlotGp124", facts.cardSlotGp124, error) ||
        !GetBoolTrue(args,
                     "gp696FdWriteKnown",
                     facts.gp696FdWriteKnown,
                     error) ||
        !GetI32(args, "gp696Fd", facts.gp696Fd, error) ||
        !GetBoolTrue(args,
                     "clearEventsCallKnown",
                     facts.clearEventsCallKnown,
                     error) ||
        !GetBoolTrue(args,
                     "readSubmissionKnown",
                     facts.readSubmissionKnown,
                     error) ||
        !GetBoolTrue(args, "readFdKnown", facts.readFdKnown, error) ||
        !GetI32(args, "readFd", facts.readFd, error) ||
        !GetBoolTrue(args,
                     "readBufferAddressKnown",
                     facts.readBufferAddressKnown,
                     error) ||
        !GetU32(args, "readBufferAddress", facts.readBufferAddress, error) ||
        !GetBoolTrue(args,
                     "readByteCountKnown",
                     facts.readByteCountKnown,
                     error) ||
        !GetSize(args, "readByteCount", facts.readByteCount, error) ||
        !GetBoolTrue(args, "pollCallKnown", facts.pollCallKnown, error) ||
        !GetBoolTrue(args,
                     "pollEventHandlesKnown80016EB8",
                     facts.pollEventHandlesKnown80016EB8,
                     error) ||
        !GetU32(args,
                "pollEventHandle0_80016EB8",
                facts.pollEventHandle0_80016EB8,
                error) ||
        !GetU32(args,
                "pollEventHandle1_80016EB8",
                facts.pollEventHandle1_80016EB8,
                error) ||
        !GetU32(args,
                "pollEventHandle2_80016EB8",
                facts.pollEventHandle2_80016EB8,
                error) ||
        !GetU32(args,
                "pollEventHandle3_80016EB8",
                facts.pollEventHandle3_80016EB8,
                error) ||
        !GetBoolTrue(args, "pollResultKnown", facts.pollResultKnown, error) ||
        !GetI32(args,
                "pollResult80016EB8",
                facts.pollResult80016EB8,
                error) ||
        !GetBoolTrue(args,
                     "pollTimedOutKnown",
                     facts.pollTimedOutKnown,
                     error) ||
        !GetBoolFalse(args, "pollTimedOut", facts.pollTimedOut, error) ||
        !GetBoolTrue(args,
                     "pollIterationCountKnown",
                     facts.pollIterationCountKnown,
                     error) ||
        !GetI32(args,
                "pollIterationCount",
                facts.pollIterationCount,
                error) ||
        !GetBoolTrue(args,
                     "waitCallCountKnown80035560",
                     facts.waitCallCountKnown80035560,
                     error) ||
        !GetI32(args,
                "waitCallCount80035560",
                facts.waitCallCount80035560,
                error) ||
        !GetBoolTrue(args, "closeKnown", facts.closeKnown, error) ||
        !GetBoolTrue(args, "closeFdKnown", facts.closeFdKnown, error) ||
        !GetI32(args, "closeFd", facts.closeFd, error) ||
        !GetBoolTrue(args, "returnKnown", facts.returnKnown, error) ||
        !GetI32(args,
                "psxReturn800179B4",
                facts.psxReturn800179B4,
                error) ||
        !GetBoolTrue(args,
                     "payloadLoadCallKnown",
                     facts.payloadLoadCallKnown,
                     error) ||
        !GetBoolTrue(args,
                     "payloadArgumentKnown",
                     facts.payloadArgumentKnown,
                     error) ||
        !GetU32(args, "payloadArgument", facts.payloadArgument, error) ||
        !GetBoolTrue(args,
                     "fullPayloadBytesKnown",
                     facts.fullPayloadBytesKnown,
                     error) ||
        !GetSize(args,
                 "fullPayloadByteCount",
                 facts.fullPayloadByteCount,
                 error)) {
        out = {};
        return false;
    }

    if (!DecodePayloadHex(args, out, error)) {
        out = {};
        return false;
    }

    facts.fullPayloadBytes = out.fullPayloadBytes.data();
    facts.fullPayloadByteCount = out.fullPayloadBytes.size();
    if (!PrStage1SaveCardHalDirect::
            IsImportableState16RuntimeTypedFacts800179B4(
                facts,
                facts.selectedBlockIndex)) {
        error = "state16 runtime facts not importable";
        out = {};
        return false;
    }

    return true;
}

void ClearStagedValidatedLiveGdbState16RuntimeEnvelope(
    State16RuntimeEnvelopeStaging& staging) {
    staging = {};
}

bool BeginStagedValidatedLiveGdbState16RuntimeEnvelopeArgs(
    State16RuntimeEnvelopeStaging& staging,
    const std::string& argsText,
    std::string& error) {
    ClearStagedValidatedLiveGdbState16RuntimeEnvelope(staging);
    error.clear();
    ArgMap args = ParseArgs(argsText, error);
    if (!error.empty()) {
        return false;
    }
    if (!RequireString(args, "source", kExpectedEnvelopeSource, error) ||
        !RequireString(args, "authority", kExpectedEnvelopeAuthority, error)) {
        return false;
    }
    if (args.find("fullPayloadBytesHex") != args.end()) {
        error = "begin must not include fullPayloadBytesHex";
        return false;
    }
    std::string digest;
    if (!GetString(args, "fullPayloadBytesSha256", digest, error) ||
        !IsLowerHexDigest(digest)) {
        error = "bad fullPayloadBytesSha256";
        return false;
    }
    int32_t payloadKnown = 0;
    std::size_t payloadByteCount = 0;
    if (!GetI32(args, "fullPayloadBytesKnown", payloadKnown, error) ||
        payloadKnown != 1 ||
        !GetSize(args, "fullPayloadByteCount", payloadByteCount, error) ||
        payloadByteCount !=
            PrStage1SaveCardHalDirect::kCardReadBlockBytes800179B4) {
        error = "bad fullPayloadBytesKnown/fullPayloadByteCount";
        return false;
    }

    staging.active = true;
    staging.headerArgs = argsText;
    staging.payloadHex.assign(
        PrStage1SaveCardHalDirect::kCardReadBlockBytes800179B4 * 2u,
        '0');
    staging.payloadByteKnown.fill(false);
    staging.payloadBytesKnown = 0u;
    return true;
}

bool AppendStagedValidatedLiveGdbState16RuntimeEnvelopeChunk(
    State16RuntimeEnvelopeStaging& staging,
    const std::string& argsText,
    std::string& error) {
    error.clear();
    if (!staging.active) {
        error = "no active state16 envelope staging";
        return false;
    }
    ArgMap args = ParseArgs(argsText, error);
    if (!error.empty()) {
        return false;
    }
    std::size_t offset = 0;
    std::string hex;
    if (!GetSize(args, "offset", offset, error) ||
        !GetString(args, "hex", hex, error)) {
        return false;
    }
    if ((hex.size() % 2u) != 0u) {
        error = "chunk hex length must be even";
        return false;
    }
    const std::size_t chunkBytes = hex.size() / 2u;
    const std::size_t maxBytes =
        PrStage1SaveCardHalDirect::kCardReadBlockBytes800179B4;
    if (chunkBytes == 0u || offset >= maxBytes ||
        chunkBytes > maxBytes - offset) {
        error = "chunk range outside payload";
        return false;
    }
    for (char c : hex) {
        if (!IsHexChar(c)) {
            error = "bad chunk hex";
            return false;
        }
    }
    for (std::size_t i = 0; i < chunkBytes; ++i) {
        if (staging.payloadByteKnown[offset + i]) {
            error = "chunk overlaps existing payload bytes";
            return false;
        }
    }
    for (std::size_t i = 0; i < chunkBytes; ++i) {
        staging.payloadHex[(offset + i) * 2u] = hex[i * 2u];
        staging.payloadHex[(offset + i) * 2u + 1u] = hex[i * 2u + 1u];
        staging.payloadByteKnown[offset + i] = true;
    }
    staging.payloadBytesKnown += chunkBytes;
    return true;
}

bool CommitStagedValidatedLiveGdbState16RuntimeEnvelope(
    State16RuntimeEnvelopeStaging& staging,
    State16RuntimeTypedFactsEnvelope& out,
    std::string& error) {
    out = {};
    error.clear();
    if (!staging.active) {
        error = "no active state16 envelope staging";
        return false;
    }
    if (staging.payloadBytesKnown !=
        PrStage1SaveCardHalDirect::kCardReadBlockBytes800179B4) {
        error = "staged payload incomplete";
        ClearStagedValidatedLiveGdbState16RuntimeEnvelope(staging);
        return false;
    }
    const std::string args =
        staging.headerArgs + " fullPayloadBytesHex=" + staging.payloadHex;
    const bool ok =
        ParseValidatedLiveGdbState16RuntimeEnvelopeArgs(args, out, error);
    ClearStagedValidatedLiveGdbState16RuntimeEnvelope(staging);
    return ok;
}

}  // namespace PrSS0State16RuntimeEnvelopeImportDirect
