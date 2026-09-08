#include "pr_psx_vsync_direct.h"

#include <algorithm>
#include <cstdint>
#include <limits>

namespace PrPsxVSyncDirect {
namespace {

PsxVSyncState80035560 g_processVSyncState80035560{};

bool ClockTargetReached(uint32_t current, uint32_t target) {
    return static_cast<int32_t>(current) >= static_cast<int32_t>(target);
}

uint32_t FramesUntilTarget(uint32_t current, uint32_t target) {
    if (ClockTargetReached(current, target)) {
        return 0u;
    }
    return target - current;
}

int32_t ClampPendingVblanks(uint64_t pending) {
    return static_cast<int32_t>(std::min<uint64_t>(
        pending,
        static_cast<uint64_t>(std::numeric_limits<int32_t>::max())));
}

void RefreshResult(PsxVSyncState80035560& state) {
    PsxVSyncResult80035560& out = state.lastResult;
    out.phase = state.waitPhase;
    out.vblankCounterAfter = state.vblankCounter80057034;
    out.firstWaitTarget = state.firstWaitTarget;
    out.nextVblankTarget = state.nextVblankTarget;
    out.rootCounterDelta = state.capturedRootCounterDelta;
    out.returnValue = state.capturedReturnValue;
    out.consumedHostVblanks = state.consumedHostVblankCount;
    out.waitPending = state.hostVblankHalPending;
}

void EnterNextVblankPhase(PsxVSyncState80035560& state) {
    PsxVSyncResult80035560& out = state.lastResult;
    state.waitPhase = PsxVSyncWaitPhase80035560::NextVblank;
    state.capturedGpuStatus = state.gpuStatus1F801814;
    state.nextVblankTarget = state.vblankCounter80057034 + 1u;
    out.firstTargetReached = true;
    out.fieldParityCheckModeled = true;
}

void CompleteWait(PsxVSyncState80035560& state) {
    PsxVSyncResult80035560& out = state.lastResult;
    state.waitActive = false;
    state.waitPhase = PsxVSyncWaitPhase80035560::Complete;
    state.hostVblankHalPending = false;
    state.pendingVblanks = 0;
    state.lastVblank80055F74 = state.vblankCounter80057034;
    state.lastRootCounter80055F70 = state.rootCounter1F801110;
    out.nextVblankReached = true;
    out.softwareWaitComplete = true;
    out.softwareStateCommitted = true;
    out.fieldParityWaitRequired =
        state.gpuStatusKnown &&
        (state.capturedGpuStatus & kGpuInterlaceBit80035560) != 0u &&
        ((state.capturedGpuStatus ^ state.gpuStatus1F801814) &
         kGpuFieldBit80035560) == 0u;
    // The host tick closes the translated VBlank wait. It does not claim to
    // have sampled or spun on the PSX GPU field-status MMIO bit.
    out.fieldParityWaitObserved = false;
}

void RecomputeLegacyPending(PsxVSyncState80035560& state) {
    if (!state.waitActive) {
        state.pendingVblanks = 0;
        state.hostVblankHalPending = false;
        return;
    }
    uint64_t pending = 0u;
    if (state.waitPhase == PsxVSyncWaitPhase80035560::FirstTarget) {
        pending = FramesUntilTarget(
            state.vblankCounter80057034,
            state.firstWaitTarget);
        ++pending;
    } else if (state.waitPhase ==
               PsxVSyncWaitPhase80035560::NextVblank) {
        pending = FramesUntilTarget(
            state.vblankCounter80057034,
            state.nextVblankTarget);
    }
    state.pendingVblanks = ClampPendingVblanks(pending);
    state.hostVblankHalPending = state.pendingVblanks > 0;
}

void AdvanceVblankInterrupts(PsxVSyncState80035560& state,
                             uint32_t count) {
    if (count == 0u) {
        return;
    }
    state.vblankCounter80057034 += count;
    state.vblankInterruptCount += count;
    state.callbackFanoutPassCount += count;
    state.consumedHostVblankCount += count;
    state.lastResult.vblankInterruptAdvanced = true;
    state.lastResult.callbackFanoutRecorded = true;
    state.lastResult.callbackSlotCount = kVblankCallbackCount80035EAC;
    state.lastResult.callbackFanoutPassCount =
        state.callbackFanoutPassCount;
}

}  // namespace

PsxVSyncState80035560& ProcessVSyncState80035560() {
    return g_processVSyncState80035560;
}

void ResetProcessVSyncState80035E54() {
    g_processVSyncState80035560 = {};
}

PsxVSyncResult80035560 BeginVSync80035560(
    PsxVSyncState80035560& state,
    int32_t argument) {
    ++state.requestCount;
    state.lastArg = argument;
    state.waitActive = false;
    state.waitPhase = PsxVSyncWaitPhase80035560::None;
    state.pendingVblanks = 0;
    state.hostVblankHalPending = false;
    state.firstWaitTarget = 0u;
    state.nextVblankTarget = 0u;
    state.consumedHostVblankCount = 0u;
    state.callbackFanoutPassCount = 0u;
    state.capturedRootCounterDelta = static_cast<uint16_t>(
        state.rootCounter1F801110 - state.lastRootCounter80055F70);
    state.capturedReturnValue =
        static_cast<int32_t>(state.capturedRootCounterDelta);

    PsxVSyncResult80035560 out{};
    out.attempted = true;
    out.sourceKnown = true;
    out.argument = argument;
    out.rootCounterDelta = state.capturedRootCounterDelta;
    out.returnValue = state.capturedReturnValue;
    out.vblankCounterAtEntry = state.vblankCounter80057034;
    out.vblankCounterAfter = state.vblankCounter80057034;

    if (argument < 0) {
        out.mode = PsxVSyncMode80035560::QueryVblankCounter;
        out.returnValue = static_cast<int32_t>(state.vblankCounter80057034);
        out.clockQueryComplete = true;
        out.softwareStateCommitted = true;
        state.capturedReturnValue = out.returnValue;
        state.lastResult = out;
        return state.lastResult;
    }

    if (argument == 1) {
        out.mode = PsxVSyncMode80035560::QueryRootCounterDelta;
        out.rootCounterQueryComplete = true;
        out.softwareStateCommitted = true;
        state.lastResult = out;
        return state.lastResult;
    }

    out.mode = PsxVSyncMode80035560::WaitForVblanks;
    out.requestedWaitVblanks =
        argument > 0 ? static_cast<uint32_t>(argument) : 1u;
    const uint32_t firstWaitBudget =
        argument > 0 ? static_cast<uint32_t>(argument - 1) : 0u;
    out.firstWaitTimeoutSpinLimit =
        (firstWaitBudget << kTimeoutSpinShift800356A8) + 1u;
    out.nextVblankTimeoutSpinLimit =
        kNextVblankTimeoutSpinLimit800356A8;
    state.firstWaitTarget = state.lastVblank80055F74 + firstWaitBudget;
    state.waitActive = true;
    state.waitPhase = PsxVSyncWaitPhase80035560::FirstTarget;
    state.lastResult = out;

    if (ClockTargetReached(
            state.vblankCounter80057034,
            state.firstWaitTarget)) {
        EnterNextVblankPhase(state);
    }
    RecomputeLegacyPending(state);
    RefreshResult(state);
    return state.lastResult;
}

PsxVSyncResult80035560 ConsumeHostVblanks80035560(
    PsxVSyncState80035560& state,
    int32_t consumedVblanks) {
    if (!state.waitActive) {
        RefreshResult(state);
        return state.lastResult;
    }
    uint32_t remaining = consumedVblanks > 0
        ? static_cast<uint32_t>(consumedVblanks)
        : 1u;

    if (state.waitPhase == PsxVSyncWaitPhase80035560::FirstTarget) {
        const uint32_t needed = FramesUntilTarget(
            state.vblankCounter80057034,
            state.firstWaitTarget);
        const uint32_t advance = std::min(remaining, needed);
        AdvanceVblankInterrupts(state, advance);
        remaining -= advance;
        if (ClockTargetReached(
                state.vblankCounter80057034,
                state.firstWaitTarget)) {
            EnterNextVblankPhase(state);
        }
    }

    if (remaining > 0u &&
        state.waitPhase == PsxVSyncWaitPhase80035560::NextVblank) {
        const uint32_t needed = FramesUntilTarget(
            state.vblankCounter80057034,
            state.nextVblankTarget);
        const uint32_t advance = std::min(remaining, needed);
        AdvanceVblankInterrupts(state, advance);
        remaining -= advance;
        if (ClockTargetReached(
                state.vblankCounter80057034,
                state.nextVblankTarget)) {
            CompleteWait(state);
        }
    }

    RecomputeLegacyPending(state);
    RefreshResult(state);
    return state.lastResult;
}

}  // namespace PrPsxVSyncDirect
