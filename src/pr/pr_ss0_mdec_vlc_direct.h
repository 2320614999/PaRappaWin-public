#pragma once

#include <cstddef>
#include <cstdint>

namespace PrSS0MdecVlcDirect {

static constexpr uint32_t kFnDecDCTvlcSize2 = 0x80047B00u;
static constexpr uint32_t kFnDecDCTvlc2 = 0x80047B30u;
static constexpr uint32_t kFnDecDCTvlc2End = 0x80047E70u;
static constexpr uint32_t kVlcTableBase8005D998 = 0x8005D998u;
static constexpr uint32_t kVlcTableEnd8006E998 = 0x8006E998u;
static constexpr uint32_t kVlcTableBytes =
    kVlcTableEnd8006E998 - kVlcTableBase8005D998;

struct DecDctVlc2Result80047B30 {
    bool known = false;
    bool executed = false;
    int32_t returnValue = -1;
    uint32_t inputHalfwordsConsumed = 0;
    uint32_t outputHalfwordsWritten = 0;
    uint32_t outputBytesWritten = 0;
    uint16_t frameCodeCount = 0;
    uint16_t frameMagic = 0;
    uint16_t quantScale = 0;
    uint16_t version = 0;
    bool frameComplete = false;
    bool trailingFe00PaddingWritten = false;
    uint32_t trailingFe00Halfwords = 0;
    bool decDctVlcSize2ResetAuthority = false;
    bool exactCurrentIdaTableAuthority = false;
    bool currentScusSemanticAuthority = false;
    bool psxPointerAuthority = false;
    bool psxHardwareMmioAuthority = false;
    bool hostProjection = false;
    bool replayValueAuthority = false;
    bool oldWinS0Authority = false;
    bool stage2PlusAuthority = false;
    bool comod2Authority = false;
};

// Direct translation of the current-SCUS DecDCTvlc2 path used after
// DecDCTvlcSize2(0). The input and output are host buffers containing the
// original little-endian PSX halfwords; no old-Windows S0 decoder is used.
DecDctVlc2Result80047B30 ExecuteDecDCTvlc2ResetSize80047B30(
    const uint8_t* input,
    size_t inputBytes,
    uint8_t* output,
    size_t outputBytes);

} // namespace PrSS0MdecVlcDirect
