#pragma once

#include <cstdint>

namespace PrPsxVSyncDirect {

constexpr uint32_t kFn80035560_VSync = 0x80035560u;
constexpr uint32_t kFn800356A8_WaitForVblankTarget = 0x800356A8u;
constexpr uint32_t kFn80035E54_InstallVblankInterrupt = 0x80035E54u;
constexpr uint32_t kFn80035EAC_VblankInterrupt = 0x80035EACu;
constexpr uint32_t kVblankCounter80057034 = 0x80057034u;
constexpr uint32_t kLastVblank80055F74 = 0x80055F74u;
constexpr uint32_t kRootCounterAddr1F801110 = 0x1F801110u;
constexpr uint32_t kGpuStatusAddr1F801814 = 0x1F801814u;
constexpr uint32_t kGpuInterlaceBit80035560 = 0x00080000u;
constexpr uint32_t kGpuFieldBit80035560 = 0x80000000u;
constexpr uint32_t kTimeoutSpinShift800356A8 = 15u;
constexpr uint32_t kVblankCallbackCount80035EAC = 8u;
constexpr uint32_t kNextVblankTimeoutSpinLimit800356A8 =
    (1u << kTimeoutSpinShift800356A8) + 1u;

enum class PsxVSyncMode80035560 : uint8_t {
    NotRun = 0,
    QueryVblankCounter,
    QueryRootCounterDelta,
    WaitForVblanks,
};

enum class PsxVSyncWaitPhase80035560 : uint8_t {
    None = 0,
    FirstTarget,
    NextVblank,
    Complete,
};

struct PsxVSyncResult80035560 {
    bool attempted = false;
    bool sourceKnown = false;
    bool softwareStateCommitted = false;
    bool clockQueryComplete = false;
    bool rootCounterQueryComplete = false;
    bool firstTargetReached = false;
    bool nextVblankReached = false;
    bool softwareWaitComplete = false;
    bool waitPending = false;
    bool vblankInterruptAdvanced = false;
    bool callbackFanoutRecorded = false;
    bool timeoutRecoveryInvoked = false;
    bool fieldParityCheckModeled = false;
    bool fieldParityWaitRequired = false;
    bool fieldParityWaitObserved = false;
    bool hostMmioRead = false;
    bool exactPsxHalParity = false;
    PsxVSyncMode80035560 mode = PsxVSyncMode80035560::NotRun;
    PsxVSyncWaitPhase80035560 phase = PsxVSyncWaitPhase80035560::None;
    int32_t argument = 0;
    int32_t returnValue = 0;
    uint16_t rootCounterDelta = 0;
    uint32_t vblankCounterAtEntry = 0;
    uint32_t vblankCounterAfter = 0;
    uint32_t firstWaitTarget = 0;
    uint32_t nextVblankTarget = 0;
    uint32_t requestedWaitVblanks = 0;
    uint32_t consumedHostVblanks = 0;
    uint32_t firstWaitTimeoutSpinLimit = 0;
    uint32_t nextVblankTimeoutSpinLimit = 0;
    uint32_t callbackSlotCount = 0;
    uint32_t callbackFanoutPassCount = 0;
};

struct PsxVSyncState80035560 {
    // These four fields retain the existing EventFrame host ABI.
    uint32_t requestCount = 0;
    int32_t lastArg = 0;
    int32_t pendingVblanks = 0;
    bool hostVblankHalPending = false;

    uint32_t vblankCounter80057034 = 0;
    uint32_t lastVblank80055F74 = 0;
    uint16_t rootCounter1F801110 = 0;
    uint16_t lastRootCounter80055F70 = 0;
    uint32_t gpuStatus1F801814 = 0;
    bool gpuStatusKnown = false;
    bool waitActive = false;
    PsxVSyncWaitPhase80035560 waitPhase =
        PsxVSyncWaitPhase80035560::None;
    uint32_t firstWaitTarget = 0;
    uint32_t nextVblankTarget = 0;
    uint32_t capturedGpuStatus = 0;
    uint16_t capturedRootCounterDelta = 0;
    int32_t capturedReturnValue = 0;
    uint32_t consumedHostVblankCount = 0;
    uint32_t vblankInterruptCount = 0;
    uint32_t callbackFanoutPassCount = 0;
    PsxVSyncResult80035560 lastResult{};
};

// 80035E54/80035EAC install and advance one process-wide VBlank clock.  All
// translated SS0 callers, including Scene0 pages, the Scene1 Event4
// abort-poll owner, and SaveUi19148, must use this state instead of page-local
// counters.
PsxVSyncState80035560& ProcessVSyncState80035560();
void ResetProcessVSyncState80035E54();

PsxVSyncResult80035560 BeginVSync80035560(
    PsxVSyncState80035560& state,
    int32_t argument);

PsxVSyncResult80035560 ConsumeHostVblanks80035560(
    PsxVSyncState80035560& state,
    int32_t consumedVblanks);

}  // namespace PrPsxVSyncDirect
