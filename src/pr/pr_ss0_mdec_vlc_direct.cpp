#include "pr_ss0_mdec_vlc_direct.h"

#include <array>
#include <limits>

namespace PrSS0MdecVlcDirect {
namespace {

// 0x8005D998..0x8006E998, exported from the current SCUS IDA database.
#include "pr_ss0_mdec_vlc_tables_current_ida.inc"

static_assert(kCurrentIdaMdecVlcTableWords.size() * sizeof(uint32_t) ==
              kVlcTableBytes);

static constexpr size_t kDcLumaOffset = 0x0000u;
static constexpr size_t kDcChromaOffset = 0x0400u;
static constexpr size_t kAcFastOffset = 0x0800u;
static constexpr size_t kAcSlowOffset = 0x10800u;

static uint16_t ReadLe16(const uint8_t* bytes)
{
    return static_cast<uint16_t>(bytes[0]) |
           static_cast<uint16_t>(static_cast<uint16_t>(bytes[1]) << 8u);
}

static void WriteLe16(uint8_t* bytes, uint16_t value)
{
    bytes[0] = static_cast<uint8_t>(value & 0xFFu);
    bytes[1] = static_cast<uint8_t>((value >> 8u) & 0xFFu);
}

static uint32_t TableWord(size_t byteOffset)
{
    return kCurrentIdaMdecVlcTableWords[byteOffset / sizeof(uint32_t)];
}

class DirectVlcState80047B30 {
public:
    DirectVlcState80047B30(const uint8_t* input,
                          size_t inputBytes,
                          uint8_t* output,
                          size_t outputBytes)
        : input_(input),
          inputBytes_(inputBytes),
          output_(output),
          outputBytes_(outputBytes)
    {
    }

    bool ReadInputWord(uint16_t& value)
    {
        if (inputOffset_ + sizeof(uint16_t) > inputBytes_) {
            return false;
        }
        value = ReadLe16(input_ + inputOffset_);
        inputOffset_ += sizeof(uint16_t);
        return true;
    }

    bool Emit(uint16_t value)
    {
        if (outputOffset_ + sizeof(uint16_t) > outputBytes_) {
            return false;
        }
        WriteLe16(output_ + outputOffset_, value);
        outputOffset_ += sizeof(uint16_t);
        return true;
    }

    bool RefillAfterShift(uint32_t consumedBits)
    {
        bitOffset_ += consumedBits;
        if ((bitOffset_ & 0x10u) == 0u) {
            bitOffset_ &= 0x0Fu;
            return true;
        }
        bitOffset_ &= 0x0Fu;
        uint16_t next = 0;
        if (!ReadInputWord(next)) {
            return false;
        }
        reservoir_ |= static_cast<uint32_t>(next) << bitOffset_;
        return true;
    }

    bool Consume(uint32_t bits)
    {
        reservoir_ <<= bits;
        return RefillAfterShift(bits);
    }

    const uint8_t* input_ = nullptr;
    size_t inputBytes_ = 0;
    uint8_t* output_ = nullptr;
    size_t outputBytes_ = 0;
    size_t inputOffset_ = 0;
    size_t outputOffset_ = 0;
    uint32_t reservoir_ = 0;
    uint32_t bitOffset_ = 0;
    uint16_t quantWord_ = 0;
    uint32_t blockIndex_ = 0;
    int32_t previousCr_ = 0;
    int32_t previousCb_ = 0;
    int32_t previousY_ = 0;
};

static bool EmitEscape80047DE4(DirectVlcState80047B30& state)
{
    if (!state.Emit(static_cast<uint16_t>(state.reservoir_ >> 16u))) {
        return false;
    }
    uint16_t next = 0;
    if (!state.ReadInputWord(next)) {
        return false;
    }
    state.reservoir_ =
        (state.reservoir_ << 16u) |
        (static_cast<uint32_t>(next) << state.bitOffset_);
    return true;
}

static bool DecodeAcCodes80047D00(DirectVlcState80047B30& state,
                                  bool& endOfBlock)
{
    endOfBlock = false;
    for (uint32_t guard = 0u; guard < 0x100000u; ++guard) {
        const uint32_t fastIndex = state.reservoir_ >> 19u;
        const size_t fastOffset =
            kAcFastOffset + static_cast<size_t>(fastIndex) * 8u;
        uint32_t packed = TableWord(fastOffset);
        uint32_t extra = 0u;
        uint32_t bitLength = packed & 0xFFu;
        if (packed == 0u) {
            if (!state.Consume(8u)) {
                return false;
            }
            const uint32_t slowIndex = state.reservoir_ >> 23u;
            packed = TableWord(
                kAcSlowOffset + static_cast<size_t>(slowIndex) * 4u);
            bitLength = packed & 0xFFu;
        } else {
            extra = TableWord(fastOffset + 4u);
        }
        if (bitLength == 0u || !state.Consume(bitLength)) {
            return false;
        }

        const uint16_t first = static_cast<uint16_t>(packed >> 16u);
        if (first == 0x7C1Fu) {
            if (!EmitEscape80047DE4(state)) {
                return false;
            }
            continue;
        }
        if (!state.Emit(first)) {
            return false;
        }
        if (first == 0xFE00u) {
            endOfBlock = true;
            return true;
        }
        if (extra == 0u) {
            continue;
        }

        const uint16_t second = static_cast<uint16_t>(extra & 0xFFFFu);
        if (second == 0x7C1Fu) {
            if (!EmitEscape80047DE4(state)) {
                return false;
            }
            continue;
        }
        if (!state.Emit(second)) {
            return false;
        }
        if (second == 0xFE00u) {
            endOfBlock = true;
            return true;
        }

        const uint16_t third = static_cast<uint16_t>(extra >> 16u);
        if (third == 0u) {
            continue;
        }
        if (third == 0x7C1Fu) {
            if (!EmitEscape80047DE4(state)) {
                return false;
            }
            continue;
        }
        if (!state.Emit(third)) {
            return false;
        }
        if (third == 0xFE00u) {
            endOfBlock = true;
            return true;
        }
    }
    return false;
}

static bool DecodeV3Dc80047BE4(DirectVlcState80047B30& state,
                               bool& frameComplete)
{
    frameComplete = false;
    const uint32_t code = state.reservoir_ >> 22u;
    if (code == 0x3FFu) {
        frameComplete = true;
        return true;
    }

    const size_t tableOffset = state.blockIndex_ < 3u
        ? kDcChromaOffset
        : kDcLumaOffset;
    const uint32_t entry = TableWord(
        tableOffset +
        static_cast<size_t>(state.reservoir_ >> 24u) * 4u);
    const uint32_t prefixBits = entry & 0xFFFFu;
    const uint32_t valueBits = entry >> 16u;
    if (prefixBits == 0u || prefixBits > 16u || valueBits > 16u) {
        return false;
    }

    state.reservoir_ <<= prefixBits;
    int32_t difference = 0;
    if (valueBits != 0u) {
        const uint32_t raw = state.reservoir_ >> (32u - valueBits);
        const bool positive =
            (state.reservoir_ & 0x80000000u) != 0u;
        state.reservoir_ <<= valueBits;
        difference = positive
            ? static_cast<int32_t>(raw)
            : static_cast<int32_t>(raw) -
                  static_cast<int32_t>((1u << valueBits) - 1u);
    }
    if (!state.RefillAfterShift(prefixBits + valueBits)) {
        return false;
    }

    int32_t dc = 0;
    if (state.blockIndex_ == 1u) {
        state.previousCr_ += difference;
        dc = state.previousCr_;
    } else if (state.blockIndex_ == 2u) {
        state.previousCb_ += difference;
        dc = state.previousCb_;
    } else {
        state.previousY_ += difference;
        dc = state.previousY_;
    }
    ++state.blockIndex_;
    if (state.blockIndex_ == 7u) {
        state.blockIndex_ = 1u;
    }
    return state.Emit(static_cast<uint16_t>(
        state.quantWord_ |
        (static_cast<uint32_t>(dc * 4) & 0x3FFu)));
}

static bool DecodeV2Dc80047CBC(DirectVlcState80047B30& state,
                               bool& frameComplete)
{
    frameComplete = false;
    const uint32_t code = state.reservoir_ >> 22u;
    if (code == 0x1FFu) {
        frameComplete = true;
        return true;
    }
    if (!state.Consume(10u)) {
        return false;
    }
    return state.Emit(static_cast<uint16_t>(state.quantWord_ | code));
}

} // namespace

DecDctVlc2Result80047B30 ExecuteDecDCTvlc2ResetSize80047B30(
    const uint8_t* input,
    size_t inputBytes,
    uint8_t* output,
    size_t outputBytes)
{
    DecDctVlc2Result80047B30 result{};
    result.decDctVlcSize2ResetAuthority = true;
    result.exactCurrentIdaTableAuthority = true;
    result.currentScusSemanticAuthority = true;
    if (input == nullptr || output == nullptr || inputBytes < 12u ||
        outputBytes < 4u + 65u * sizeof(uint16_t)) {
        return result;
    }

    DirectVlcState80047B30 state(input, inputBytes, output, outputBytes);
    uint16_t first = 0;
    uint16_t second = 0;
    uint16_t qscale = 0;
    uint16_t version = 0;
    uint16_t reservoirHigh = 0;
    uint16_t reservoirLow = 0;
    if (!state.ReadInputWord(first) ||
        !state.ReadInputWord(second) ||
        !state.ReadInputWord(qscale) ||
        !state.ReadInputWord(version) ||
        !state.ReadInputWord(reservoirHigh) ||
        !state.ReadInputWord(reservoirLow) ||
        !state.Emit(first) ||
        !state.Emit(second)) {
        return result;
    }
    state.quantWord_ = static_cast<uint16_t>(qscale << 10u);
    state.blockIndex_ = version >= 3u ? 1u : 0u;
    state.reservoir_ =
        (static_cast<uint32_t>(reservoirHigh) << 16u) |
        static_cast<uint32_t>(reservoirLow);
    result.frameCodeCount = first;
    result.frameMagic = second;
    result.quantScale = qscale;
    result.version = version;

    for (uint32_t guard = 0u; guard < 0x100000u; ++guard) {
        bool frameComplete = false;
        const bool dcDecoded = state.blockIndex_ != 0u
            ? DecodeV3Dc80047BE4(state, frameComplete)
            : DecodeV2Dc80047CBC(state, frameComplete);
        if (!dcDecoded) {
            return result;
        }
        if (frameComplete) {
            for (uint32_t index = 0u; index < 65u; ++index) {
                if (!state.Emit(0xFE00u)) {
                    return result;
                }
            }
            result.known = true;
            result.executed = true;
            result.returnValue = 0;
            result.inputHalfwordsConsumed =
                static_cast<uint32_t>(state.inputOffset_ / 2u);
            result.outputHalfwordsWritten =
                static_cast<uint32_t>(state.outputOffset_ / 2u);
            result.outputBytesWritten =
                static_cast<uint32_t>(state.outputOffset_);
            result.frameComplete = true;
            result.trailingFe00PaddingWritten = true;
            result.trailingFe00Halfwords = 65u;
            return result;
        }

        bool endOfBlock = false;
        if (!DecodeAcCodes80047D00(state, endOfBlock) || !endOfBlock) {
            return result;
        }
    }
    return result;
}

} // namespace PrSS0MdecVlcDirect
