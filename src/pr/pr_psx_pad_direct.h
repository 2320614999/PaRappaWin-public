#pragma once

#include <cstdint>

#include "pr_psx_vsync_direct.h"

namespace PrPsxPadDirect {

constexpr uint32_t kFn800354C0_PadInit = 0x800354C0u;
constexpr uint32_t kPadInit2Protocol800354C0 = 0x20000001u;
constexpr uint32_t kPadStatusGlobal800882F0 = 0x800882F0u;
constexpr uint32_t kPadModeGlobal80091830 = 0x80091830u;
constexpr uint32_t kFn80016AB4_StartupPadWait = 0x80016AB4u;
constexpr uint32_t kFn80035510_PadRead = 0x80035510u;
constexpr uint32_t kFn80035560_StartupWait = 0x80035560u;
constexpr uint32_t kStartupPadWaitPollLimit80016AB4 = 30u;
constexpr uint32_t kStartupPadWaitSpecialMask80016AB4 = 287u;
constexpr int32_t kStartupPadWaitArgument80016AB4 = 0;
constexpr uint32_t kWord800916FA_StartupSpecial = 0x800916FAu;

struct PadReadResult80035510 {
    bool called = false;
    uint32_t psxFunction = 0x80035510u;
    bool padDrCalled = false;
    uint16_t dword800882F0 = 0xFFFFu;
    uint16_t psxReturnMask = 0u;
};

// Software-owned result for COMOD0/SCUS 80016AB4.  The original routine is
// called once after 80016B84 and before the 80015D18 main loop: it polls
// 80035510 up to thirty times, waits through 80035560(0) only while the
// returned pad mask is zero, and maps returned mask 287 to result 1 while
// setting word_800916FA.  The host PAD/VBlank HAL remains deliberately
// unclaimed.
struct StartupPadWaitResult80016AB4 {
    bool known = false;
    bool committed = false;
    uint32_t wrapperFunction = kFn80016AB4_StartupPadWait;
    uint32_t padReadFunction = kFn80035510_PadRead;
    uint32_t waitFunction = kFn80035560_StartupWait;
    int32_t waitArgument = kStartupPadWaitArgument80016AB4;
    uint32_t pollLimit = kStartupPadWaitPollLimit80016AB4;
    uint32_t pollCount = 0u;
    uint32_t waitCallCount = 0u;
    uint32_t consumedVblankCount = 0u;
    uint16_t returnedPadMask = 0u;
    uint32_t resultBeforeSpecialMap = 0u;
    uint32_t returnValue = 0u;
    bool pollLimitReached = false;
    bool specialMaskMatched = false;
    bool word800916FAWritten = false;
    uint32_t word800916FAValue = 0u;
    bool softwareStateCommitted = false;
    bool hardwarePadHalAuthority = false;
    bool physicalVblankTimingAuthority = false;
    bool exactPsxHalParity = false;
};

// Software-owned side effects of the PSX PAD bootstrap.  The call order and
// global writes are deterministic from IDA; the actual controller interrupt
// timing remains a hardware/HAL boundary and is intentionally not claimed.
struct PadInitState800354C0 {
    bool sourceKnown = false;
    bool accepted = false;
    int32_t inputArg = 0;
    uint32_t modeGlobalAddress = kPadModeGlobal80091830;
    int32_t modeGlobalValue = 0;
    uint32_t statusGlobalAddress = kPadStatusGlobal800882F0;
    int32_t statusGlobalValue = -1;
    bool resetCallbackCalled = false;
    bool padInit2Called = false;
    uint32_t padInit2Protocol = 0;
    uint32_t padInit2StatusAddress = 0;
    bool changeClearPadCalled = false;
    int32_t changeClearPadArg = 0;
    bool softwareStateCommitted = false;
    bool hardwarePadHalAuthority = false;
    bool exactPsxHalParity = false;
    bool hostProjection = false;
    bool replayValueAuthority = false;
    bool oldWinS0Authority = false;
    bool stage2PlusAuthority = false;
    bool comod2Authority = false;
};

PadInitState800354C0 PsxCall800354C0_InitPadRuntime(int32_t inputArg);

PadReadResult80035510 PsxReadPadMask80035510(uint16_t padReturnedMask);

StartupPadWaitResult80016AB4 ExecuteStartupPadWait80016AB4(
    uint16_t padReturnedMask,
    PrPsxVSyncDirect::PsxVSyncState80035560& sharedVSync);
uint16_t NormalizeDebugServerPsxPadMaskToReturnedMask80035510(
    uint16_t psxPadMask);
uint16_t NormalizeLocalPrPadMaskToReturnedMask80035510(
    uint16_t localPrPadMask);
uint16_t BuildReturnedMask80035510FromLocalAndDebugPad(
    uint16_t localPrPadMask,
    uint16_t debugServerPsxPadMask);

}  // namespace PrPsxPadDirect
