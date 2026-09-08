#pragma once

#include <cstddef>
#include <cstdint>

namespace PrSS0MdecOutputDirect {

struct Mdec15bppResult {
    bool known = false;
    bool executed = false;
    uint32_t commandWord = 0;
    uint32_t inputWordCount = 0;
    uint32_t inputHalfwordsConsumed = 0;
    uint16_t width = 0;
    uint16_t height = 0;
    uint32_t macroblockColumns = 0;
    uint32_t macroblockRows = 0;
    uint32_t macroblocksDecoded = 0;
    uint32_t outputBytesWritten = 0;
    uint64_t outputFnv1a64 = 0;
    bool output15bpp = false;
    bool outputUnsigned = false;
    bool outputBit15Set = false;
    bool dma1VerticalMacroblockLayout = false;
    bool currentScusQuantTableAuthority = false;
    bool currentScusScaleTableAuthority = false;
    bool currentIdaCommandAuthority = false;
    bool documentedPsxMdecAlgorithmAuthority = false;
    bool mdecHardwareBitExactAuthority = false;
    bool dmaCompletionCallbackTimingAuthority = false;
    bool psxHardwareMmioAuthority = false;
    bool hostProjection = false;
    bool replayValueAuthority = false;
    bool oldWinS0Authority = false;
    bool stage2PlusAuthority = false;
    bool comod2Authority = false;
};

// Executes the current-SCUS MDEC(1) 15bpp command against host buffers. The
// quant/scale tables are copied from the current SCUS IDA database. The
// decompression path follows the documented PSX MDEC algorithm independently;
// it does not use the old Windows S0 decoder or dynamic replay values.
Mdec15bppResult ExecuteMdec15bppCurrentScus(
    const uint8_t* commandAndRle,
    size_t inputBytes,
    uint16_t width,
    uint16_t height,
    uint8_t* output,
    size_t outputBytes);

} // namespace PrSS0MdecOutputDirect
