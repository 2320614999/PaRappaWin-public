#include "pr_psx_clear_image_direct.h"

namespace PrPsxClearImageDirect {

PsxClearImageResult80040420 PsxCall80040420_ClearImage(
    const PsxClearImageInput80040420& input,
    uint32_t r,
    uint32_t g,
    uint32_t b) {
    PsxClearImageResult80040420 out{};
    out.attempted = true;
    out.callCount = input.priorCallCount;
    if (input.word_80096590 > 1u) {
        return out;
    }

    const uint16_t slot = input.word_80096590;
    out.sourceKnown = true;
    out.slot = slot;
    out.x = input.word_8008ECA8[slot];
    out.y = input.word_8008ECAC[slot];
    out.width = static_cast<uint16_t>(input.dword_800917FC);
    out.height = static_cast<uint16_t>(input.dword_8009182C);
    out.r = static_cast<uint8_t>(r);
    out.g = static_cast<uint8_t>(g);
    out.b = static_cast<uint8_t>(b);
    out.clearImage80044CD0Called = true;
    out.softwareStateCommitted = true;
    out.callCount += 1u;
    return out;
}

} // namespace PrPsxClearImageDirect
