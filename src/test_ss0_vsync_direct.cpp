#include "pr/pr_psx_vsync_direct.h"
#include "pr/pr_psx_pad_direct.h"

#include <cstdio>

namespace {

using namespace PrPsxVSyncDirect;

bool CheckQueryModes() {
    PsxVSyncState80035560 state{};
    state.vblankCounter80057034 = 42u;
    state.rootCounter1F801110 = 0x0012u;
    state.lastRootCounter80055F70 = 0xFFFEu;

    const auto clock = BeginVSync80035560(state, -1);
    if (clock.mode != PsxVSyncMode80035560::QueryVblankCounter ||
        !clock.clockQueryComplete || !clock.softwareStateCommitted ||
        clock.returnValue != 42 || clock.waitPending ||
        state.hostVblankHalPending || state.pendingVblanks != 0) {
        return false;
    }

    const auto root = BeginVSync80035560(state, 1);
    return root.mode ==
               PsxVSyncMode80035560::QueryRootCounterDelta &&
           root.rootCounterQueryComplete && root.rootCounterDelta == 20u &&
           root.returnValue == 20 && root.softwareStateCommitted &&
           !root.waitPending && state.lastRootCounter80055F70 == 0xFFFEu;
}

bool CheckZeroModeWait() {
    PsxVSyncState80035560 state{};
    state.vblankCounter80057034 = 7u;
    state.lastVblank80055F74 = 7u;
    state.rootCounter1F801110 = 123u;
    state.lastRootCounter80055F70 = 100u;

    const auto begin = BeginVSync80035560(state, 0);
    if (begin.mode != PsxVSyncMode80035560::WaitForVblanks ||
        !begin.firstTargetReached || begin.softwareWaitComplete ||
        !begin.waitPending || begin.firstWaitTarget != 7u ||
        begin.nextVblankTarget != 8u || begin.returnValue != 23 ||
        state.pendingVblanks != 1 || !state.hostVblankHalPending) {
        return false;
    }

    const auto done = ConsumeHostVblanks80035560(state, 1);
    return done.softwareWaitComplete && done.softwareStateCommitted &&
           done.nextVblankReached && done.vblankInterruptAdvanced &&
           done.callbackFanoutRecorded &&
           done.callbackSlotCount == kVblankCallbackCount80035EAC &&
           done.callbackFanoutPassCount == 1u &&
           done.vblankCounterAfter == 8u && done.consumedHostVblanks == 1u &&
           state.lastVblank80055F74 == 8u &&
           state.lastRootCounter80055F70 == 123u &&
           state.vblankInterruptCount == 1u && state.pendingVblanks == 0 &&
           !state.hostVblankHalPending && !done.timeoutRecoveryInvoked &&
           !done.hostMmioRead && !done.exactPsxHalParity;
}

bool CheckTwoVblankWait() {
    PsxVSyncState80035560 state{};
    state.vblankCounter80057034 = 10u;
    state.lastVblank80055F74 = 10u;

    const auto begin = BeginVSync80035560(state, 2);
    if (begin.firstWaitTarget != 11u || begin.firstTargetReached ||
        begin.phase != PsxVSyncWaitPhase80035560::FirstTarget ||
        begin.requestedWaitVblanks != 2u || state.pendingVblanks != 2) {
        return false;
    }

    const auto first = ConsumeHostVblanks80035560(state, 1);
    if (!first.firstTargetReached || first.softwareWaitComplete ||
        first.phase != PsxVSyncWaitPhase80035560::NextVblank ||
        first.nextVblankTarget != 12u || state.pendingVblanks != 1) {
        return false;
    }

    const auto second = ConsumeHostVblanks80035560(state, 1);
    if (!second.softwareWaitComplete || second.vblankCounterAfter != 12u ||
        second.consumedHostVblanks != 2u ||
        state.vblankInterruptCount != 2u ||
        second.callbackFanoutPassCount != 2u ||
        begin.firstWaitTimeoutSpinLimit != 32769u ||
        begin.nextVblankTimeoutSpinLimit != 32769u) {
        return false;
    }

    const auto query = BeginVSync80035560(state, -1);
    return query.returnValue == 12 && query.clockQueryComplete;
}

bool CheckCollapsedHostDeliveryAndCaughtUpTarget() {
    PsxVSyncState80035560 state{};
    state.vblankCounter80057034 = 20u;
    state.lastVblank80055F74 = 10u;

    const auto begin = BeginVSync80035560(state, 2);
    if (!begin.firstTargetReached || begin.nextVblankTarget != 21u ||
        state.pendingVblanks != 1) {
        return false;
    }
    const auto done = ConsumeHostVblanks80035560(state, 2);
    return done.softwareWaitComplete && done.vblankCounterAfter == 21u &&
           done.consumedHostVblanks == 1u;
}

bool CheckInterlaceParityIsBounded() {
    PsxVSyncState80035560 state{};
    state.gpuStatusKnown = true;
    state.gpuStatus1F801814 = kGpuInterlaceBit80035560;

    const auto begin = BeginVSync80035560(state, 0);
    if (!begin.fieldParityCheckModeled ||
        begin.fieldParityWaitRequired) {
        return false;
    }
    const auto done = ConsumeHostVblanks80035560(state, 1);
    if (!done.softwareWaitComplete || !done.fieldParityWaitRequired ||
        done.fieldParityWaitObserved || done.exactPsxHalParity) {
        return false;
    }

    PsxVSyncState80035560 toggled{};
    toggled.gpuStatusKnown = true;
    toggled.gpuStatus1F801814 = kGpuInterlaceBit80035560;
    (void)BeginVSync80035560(toggled, 0);
    toggled.gpuStatus1F801814 |= kGpuFieldBit80035560;
    const auto fieldChanged = ConsumeHostVblanks80035560(toggled, 1);
    return fieldChanged.softwareWaitComplete &&
           !fieldChanged.fieldParityWaitRequired &&
           !fieldChanged.exactPsxHalParity;
}

bool CheckSharedClockAcrossEventOwners() {
    PsxVSyncState80035560 shared{};
    const auto event3Begin = BeginVSync80035560(shared, 0);
    if (!event3Begin.waitPending ||
        !ConsumeHostVblanks80035560(shared, 1).softwareWaitComplete) {
        return false;
    }
    const uint32_t event3Clock = shared.vblankCounter80057034;

    const auto event17Begin = BeginVSync80035560(shared, 0);
    if (!event17Begin.waitPending || event17Begin.firstWaitTarget != 1u ||
        !ConsumeHostVblanks80035560(shared, 1).softwareWaitComplete) {
        return false;
    }
    return event3Clock == 1u && shared.vblankCounter80057034 == 2u &&
           shared.lastVblank80055F74 == 2u && shared.requestCount == 2u &&
           shared.vblankInterruptCount == 2u;
}

bool CheckProcessWideOwner80035E54() {
    ResetProcessVSyncState80035E54();
    auto& scene0Owner = ProcessVSyncState80035560();
    auto& event4Owner = ProcessVSyncState80035560();
    auto& saveUi19148Owner = ProcessVSyncState80035560();
    if (&scene0Owner != &event4Owner ||
        &event4Owner != &saveUi19148Owner) {
        return false;
    }
    const auto begin = BeginVSync80035560(scene0Owner, 0);
    if (!begin.waitPending || begin.softwareWaitComplete) {
        return false;
    }
    const auto done = ConsumeHostVblanks80035560(scene0Owner, 1);
    if (!done.softwareWaitComplete || done.vblankCounterAfter != 1u) {
        return false;
    }
    const auto observed = BeginVSync80035560(event4Owner, -1);
    if (!observed.clockQueryComplete || observed.returnValue != 1) {
        return false;
    }
    const auto saveUiBegin = BeginVSync80035560(saveUi19148Owner, 0);
    if (!saveUiBegin.waitPending || saveUiBegin.firstWaitTarget != 1u) {
        return false;
    }
    const auto saveUiDone =
        ConsumeHostVblanks80035560(saveUi19148Owner, 1);
    if (!saveUiDone.softwareWaitComplete ||
        saveUiDone.vblankCounterAfter != 2u) {
        return false;
    }
    ResetProcessVSyncState80035E54();
    return ProcessVSyncState80035560().vblankCounter80057034 == 0u;
}

bool CheckStartupPadWait80016AB4() {
    using namespace PrPsxPadDirect;

    PsxVSyncState80035560 noInputClock{};
    const auto noInput = ExecuteStartupPadWait80016AB4(
        0u, noInputClock);
    if (!noInput.known || !noInput.committed ||
        noInput.wrapperFunction != kFn80016AB4_StartupPadWait ||
        noInput.padReadFunction != kFn80035510_PadRead ||
        noInput.waitFunction != kFn80035560_StartupWait ||
        noInput.waitArgument != 0 ||
        noInput.pollCount != kStartupPadWaitPollLimit80016AB4 ||
        noInput.waitCallCount != kStartupPadWaitPollLimit80016AB4 ||
        noInput.consumedVblankCount !=
            kStartupPadWaitPollLimit80016AB4 ||
        !noInput.pollLimitReached || noInput.returnedPadMask != 0u ||
        noInput.resultBeforeSpecialMap != 0u || noInput.returnValue != 0u ||
        noInput.specialMaskMatched || noInput.word800916FAWritten ||
        noInput.hardwarePadHalAuthority ||
        noInput.physicalVblankTimingAuthority ||
        !noInput.softwareStateCommitted ||
        noInputClock.vblankCounter80057034 !=
            kStartupPadWaitPollLimit80016AB4) {
        return false;
    }

    PsxVSyncState80035560 specialClock{};
    const auto special = ExecuteStartupPadWait80016AB4(
        static_cast<uint16_t>(kStartupPadWaitSpecialMask80016AB4),
        specialClock);
    if (!special.committed || special.pollCount != 1u ||
        special.waitCallCount != 0u || special.returnedPadMask != 287u ||
        special.resultBeforeSpecialMap != 287u ||
        special.returnValue != 1u || !special.specialMaskMatched ||
        !special.word800916FAWritten || special.word800916FAValue != 1u ||
        specialClock.vblankCounter80057034 != 0u) {
        return false;
    }

    const auto ordinary = ExecuteStartupPadWait80016AB4(
        1u, specialClock);
    return ordinary.committed && ordinary.pollCount == 1u &&
           ordinary.waitCallCount == 0u && ordinary.returnValue == 1u &&
           !ordinary.specialMaskMatched && !ordinary.word800916FAWritten;
}

}  // namespace

int main() {
    if (!CheckQueryModes()) {
        std::printf("test_ss0_vsync_direct: FAIL query modes\n");
        return 1;
    }
    if (!CheckZeroModeWait()) {
        std::printf("test_ss0_vsync_direct: FAIL zero mode\n");
        return 1;
    }
    if (!CheckTwoVblankWait()) {
        std::printf("test_ss0_vsync_direct: FAIL two-vblank mode\n");
        return 1;
    }
    if (!CheckCollapsedHostDeliveryAndCaughtUpTarget()) {
        std::printf("test_ss0_vsync_direct: FAIL caught-up target\n");
        return 1;
    }
    if (!CheckInterlaceParityIsBounded()) {
        std::printf("test_ss0_vsync_direct: FAIL parity boundary\n");
        return 1;
    }
    if (!CheckSharedClockAcrossEventOwners() ||
        !CheckProcessWideOwner80035E54()) {
        std::printf("test_ss0_vsync_direct: FAIL shared clock\n");
        return 1;
    }
    if (!CheckStartupPadWait80016AB4()) {
        std::printf("test_ss0_vsync_direct: FAIL startup pad wait\n");
        return 1;
    }
    std::printf("test_ss0_vsync_direct: PASS\n");
    return 0;
}
