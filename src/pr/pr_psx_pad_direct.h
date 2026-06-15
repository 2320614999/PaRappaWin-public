#pragma once

#include <cstdint>

namespace PrPsxPadDirect {

struct PadReadResult80035510 {
    bool called = false;
    uint32_t psxFunction = 0x80035510u;
    bool padDrCalled = false;
    uint16_t dword800882F0 = 0xFFFFu;
    uint16_t psxReturnMask = 0u;
};

PadReadResult80035510 PsxReadPadMask80035510(uint16_t padReturnedMask);
uint16_t NormalizeDebugServerPsxPadMaskToReturnedMask80035510(
    uint16_t psxPadMask);
uint16_t NormalizeLocalPrPadMaskToReturnedMask80035510(
    uint16_t localPrPadMask);
uint16_t BuildReturnedMask80035510FromLocalAndDebugPad(
    uint16_t localPrPadMask,
    uint16_t debugServerPsxPadMask);

}  // namespace PrPsxPadDirect
