#pragma once
#include <cstddef>
#include <cstdint>
#include <vector>
#include "pr_ss0_mdec_output_direct.h"

namespace PrNativeMovieFrame {
// Pure shared-codec adapter. Owns no scene, clock, device register, DMA or IRQ.
// The caller supplies the actual source-formatted 15-bpp command/RLE stream.
// Existing SCUS quantization/IDCT tables are the codec's explicit contract.
struct Frame {
    uint16_t width = 0, height = 0;
    PrSS0MdecOutputDirect::Mdec15bppResult decode{};
    std::vector<uint8_t> strips15;
    std::vector<uint32_t> rgba;
};
// A failure throws before publishing a Frame. The input is never modified.
Frame Decode15bppRle(const uint8_t* commandAndRle, size_t bytes,
                    uint16_t width, uint16_t height);
// Same vertical 16-pixel-strip projection used by existing Stage1 playback.
std::vector<uint32_t> Project15bpp(const uint8_t* strips, size_t bytes,
                                 uint16_t width, uint16_t height);
}
