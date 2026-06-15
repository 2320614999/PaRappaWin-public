#include "pr_psx_pad_direct.h"

#include "pr_pad.h"

namespace PrPsxPadDirect {

uint16_t NormalizeDebugServerPsxPadMaskToReturnedMask80035510(
    uint16_t psxPadMask) {
    // Debug commands can still supply the PSX high directional quartet; the
    // Stage1 gameplay seam needs the post-PAD_dr returned mask seen by 80035510.
    uint16_t returnedMask = static_cast<uint16_t>(psxPadMask & 0x0FFFu);
    if ((psxPadMask & 0x1000u) != 0u) {
        returnedMask = static_cast<uint16_t>(returnedMask | 0x0010u);
    }
    if ((psxPadMask & 0x2000u) != 0u) {
        returnedMask = static_cast<uint16_t>(returnedMask | 0x0020u);
    }
    if ((psxPadMask & 0x4000u) != 0u) {
        returnedMask = static_cast<uint16_t>(returnedMask | 0x0040u);
    }
    if ((psxPadMask & 0x8000u) != 0u) {
        returnedMask = static_cast<uint16_t>(returnedMask | 0x0080u);
    }
    return returnedMask;
}

uint16_t NormalizeLocalPrPadMaskToReturnedMask80035510(
    uint16_t localPrPadMask) {
    uint16_t returnedMask =
        static_cast<uint16_t>(localPrPadMask &
            ((uint16_t)PrPadButton::Triangle |
             (uint16_t)PrPadButton::Circle |
             (uint16_t)PrPadButton::Cross |
             (uint16_t)PrPadButton::Square));
    if ((localPrPadMask & (uint16_t)PrPadButton::Up) != 0u) {
        returnedMask = static_cast<uint16_t>(returnedMask | 0x0010u);
    }
    if ((localPrPadMask & (uint16_t)PrPadButton::Right) != 0u) {
        returnedMask = static_cast<uint16_t>(returnedMask | 0x0020u);
    }
    if ((localPrPadMask & (uint16_t)PrPadButton::Down) != 0u) {
        returnedMask = static_cast<uint16_t>(returnedMask | 0x0040u);
    }
    if ((localPrPadMask & (uint16_t)PrPadButton::Left) != 0u) {
        returnedMask = static_cast<uint16_t>(returnedMask | 0x0080u);
    }
    if ((localPrPadMask & (uint16_t)PrPadButton::L1) != 0u) {
        returnedMask = static_cast<uint16_t>(returnedMask | 0x0004u);
    }
    if ((localPrPadMask & (uint16_t)PrPadButton::L2) != 0u) {
        returnedMask = static_cast<uint16_t>(returnedMask | 0x0001u);
    }
    if ((localPrPadMask & (uint16_t)PrPadButton::R1) != 0u) {
        returnedMask = static_cast<uint16_t>(returnedMask | 0x0008u);
    }
    if ((localPrPadMask & (uint16_t)PrPadButton::R2) != 0u) {
        returnedMask = static_cast<uint16_t>(returnedMask | 0x0002u);
    }
    if ((localPrPadMask & (uint16_t)PrPadButton::Select) != 0u) {
        returnedMask = static_cast<uint16_t>(returnedMask | 0x0100u);
    }
    if ((localPrPadMask & (uint16_t)PrPadButton::Start) != 0u) {
        returnedMask = static_cast<uint16_t>(returnedMask | 0x0800u);
    }
    return returnedMask;
}

uint16_t BuildReturnedMask80035510FromLocalAndDebugPad(
    uint16_t localPrPadMask,
    uint16_t debugServerPsxPadMask) {
    const uint16_t localReturnedMask =
        NormalizeLocalPrPadMaskToReturnedMask80035510(localPrPadMask);
    const uint16_t debugReturnedMask =
        NormalizeDebugServerPsxPadMaskToReturnedMask80035510(
            debugServerPsxPadMask);
    return static_cast<uint16_t>(localReturnedMask | debugReturnedMask);
}

PadReadResult80035510 PsxReadPadMask80035510(uint16_t padReturnedMask) {
    PadReadResult80035510 out{};
    out.called = true;
    out.padDrCalled = true;
    out.dword800882F0 = static_cast<uint16_t>(~padReturnedMask);
    out.psxReturnMask = static_cast<uint16_t>(~out.dword800882F0);
    return out;
}

}  // namespace PrPsxPadDirect
