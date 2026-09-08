#include "pr/pr_ss0_scene0_shared_event_predispatch_direct.h"

#include <cstdio>
#include <limits>

namespace {

using namespace PrSS0Scene0SharedEventPredispatchDirect;

int g_failed = 0;

#define CHECK(expr)                                                           \
    do {                                                                      \
        if (!(expr)) {                                                        \
            std::printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr);       \
            ++g_failed;                                                       \
        }                                                                     \
    } while (0)

ContextState80024FD0 MakeResetState()
{
    ContextState80024FD0 state{};
    ResetContextState80024E98(state);
    return state;
}

Input80024FD0 MakeInput(uint32_t tick96,
                        uint16_t word800916D0,
                        uint32_t flags = 0u,
                        uint32_t callsite = kCallsite801C4C9C)
{
    Input80024FD0 input{};
    input.callsiteKnown = true;
    input.callsite = callsite;
    input.ctxTick96Known = true;
    input.ctxTick96 = tick96;
    input.word800916D0Known = true;
    input.word800916D0 = word800916D0;
    input.prefixAppliedKnown = true;
    input.prefixApplied = true;
    input.bucketChangedKnown = true;
    input.bucketChanged = true;
    input.bucketKnown = true;
    input.bucket = (tick96 % 384u) / 12u;
    input.ctxFlagsKnown = true;
    input.ctxFlags = flags;
    input.globalBindings =
        PrSS0Scene0GlobalBindingsDirect::BuildState801C5B14();
    return input;
}

void CheckRejectedWithoutMutation(const ContextState80024FD0& state,
                                  const Input80024FD0& input,
                                  Status80024FD0 expected)
{
    const Result80024FD0 result =
        ApplyPredispatch800250E4To800251D4(state, input);
    CHECK(result.status == expected);
    CHECK(!result.accepted);
    CHECK(!result.applied);
    CHECK(!result.stoppedBeforeDispatch);
    CHECK(!result.tableRowDereferenceAttempted);
    CHECK(!result.ctxDword56Written);
    CHECK(!result.ctxDword64Written);
    CHECK(!result.ctxDword68Written);
    CHECK(!result.dword8008ED08Written);
    CHECK(result.nextContext.resetKnown == state.resetKnown);
    CHECK(result.nextContext.ctxDword56Known == state.ctxDword56Known);
    CHECK(result.nextContext.ctxDword56 == state.ctxDword56);
    CHECK(result.nextContext.ctxDword64PsxAddress ==
          state.ctxDword64PsxAddress);
    CHECK(result.nextContext.ctxDword68PsxAddress ==
          state.ctxDword68PsxAddress);
    CHECK(result.nextCtxFlagsKnown == input.ctxFlagsKnown);
    CHECK(result.nextCtxFlags == input.ctxFlags);
}

void CheckApplied(const Result80024FD0& result,
                  uint32_t expectedV6,
                  uint32_t expectedV7,
                  uint32_t expectedFlags)
{
    CHECK(result.status == Status80024FD0::Applied);
    CHECK(result.accepted);
    CHECK(result.applied);
    CHECK(result.stoppedBeforeDispatch);
    CHECK(!result.tableRowDereferenceRequired);
    CHECK(!result.tableRowDereferenceAttempted);
    CHECK(result.ctxDword56Written);
    CHECK(!result.ctxDword64Written);
    CHECK(!result.ctxDword68Written);
    CHECK(!result.dword8008ED08Written);
    CHECK(result.ctxDword56V6 == expectedV6);
    CHECK(result.rowIndexV7 == expectedV7);
    CHECK(result.nextContext.ctxDword56Known);
    CHECK(result.nextContext.ctxDword56 == expectedV6);
    CHECK(result.nextContext.ctxDword64PsxAddressKnown);
    CHECK(result.nextContext.ctxDword64PsxAddress == 0u);
    CHECK(result.nextContext.ctxDword68PsxAddressKnown);
    CHECK(result.nextContext.ctxDword68PsxAddress == 0u);
    CHECK(result.nextCtxFlagsKnown);
    CHECK(result.nextCtxFlags == expectedFlags);
    CHECK(!result.psxMemoryBackingAuthority);
    CHECK(!result.oldWinS0Authority);
    CHECK(!result.stage1Authority);
    CHECK(!result.stage2PlusAuthority);
    CHECK(!result.replayValueAuthority);
    CHECK(!result.hostFilesystemAuthority);
}

void TestConstantsAndReset()
{
    CHECK(kRangeBegin800250E4 == 0x800250E4u);
    CHECK(kRangeEnd800251D4 == 0x800251D4u);
    CHECK(kNextUnclaimed800251D8 == 0x800251D8u);
    CHECK(kFirstDispatchBranch800251F0 == 0x800251F0u);
    CHECK(kCallsite801C4C9C == 0x801C4C9Cu);
    CHECK(kCallsite801C4D14 == 0x801C4D14u);
    const ContextState80024FD0 state = MakeResetState();
    CHECK(state.resetKnown);
    CHECK(state.ctxDword56Known);
    CHECK(state.ctxDword56 == 0u);
    CHECK(state.ctxDword64PsxAddressKnown);
    CHECK(state.ctxDword64PsxAddress == 0u);
    CHECK(state.ctxDword68PsxAddressKnown);
    CHECK(state.ctxDword68PsxAddress == 0u);
}

void TestFlagsAndCallsites()
{
    const ContextState80024FD0 state = MakeResetState();
    CheckApplied(ApplyPredispatch800250E4To800251D4(
                     state, MakeInput(0u, 0u)),
                 1u, 1u, 0xEu);
    CheckApplied(ApplyPredispatch800250E4To800251D4(
                     state, MakeInput(12u, 0u)),
                 1u, 1u, 0u);
    CheckApplied(ApplyPredispatch800250E4To800251D4(
                     state, MakeInput(24u, 0u)),
                 1u, 1u, 8u);
    CheckApplied(ApplyPredispatch800250E4To800251D4(
                     state, MakeInput(96u, 0u)),
                 1u, 1u, 0xCu);
    CheckApplied(ApplyPredispatch800250E4To800251D4(
                     state,
                     MakeInput(24u, 0u, 0u, kCallsite801C4D14)),
                 1u, 1u, 8u);
}

void TestTransitionLookaheadThresholds()
{
    const ContextState80024FD0 state = MakeResetState();
    CheckApplied(ApplyPredispatch800250E4To800251D4(
                     state, MakeInput(367u, 0u)),
                 1u, 1u, 8u);
    CheckApplied(ApplyPredispatch800250E4To800251D4(
                     state, MakeInput(368u, 0u)),
                 1u, 2u, 8u);
    CheckApplied(ApplyPredispatch800250E4To800251D4(
                     state, MakeInput(371u, 1u)),
                 1u, 1u, 8u);
    CheckApplied(ApplyPredispatch800250E4To800251D4(
                     state, MakeInput(372u, 1u)),
                 1u, 2u, 0u);
    CheckApplied(ApplyPredispatch800250E4To800251D4(
                     state, MakeInput(371u, 2u)),
                 1u, 1u, 8u);
    CheckApplied(ApplyPredispatch800250E4To800251D4(
                     state, MakeInput(372u, 2u)),
                 1u, 2u, 0u);
    CheckApplied(ApplyPredispatch800250E4To800251D4(
                     state, MakeInput(368u, 99u)),
                 1u, 2u, 8u);
}

void TestFailClosedInputs()
{
    const ContextState80024FD0 state = MakeResetState();
    Input80024FD0 input = MakeInput(24u, 0u, 0x1234u);

    input.callsiteKnown = false;
    CheckRejectedWithoutMutation(state, input, Status80024FD0::SourceUnknown);

    input = MakeInput(24u, 0u, 0x1234u);
    input.callsite = 0x801C4D18u;
    CheckRejectedWithoutMutation(
        state, input, Status80024FD0::UnsupportedCallsite);

    input = MakeInput(24u, 0u, 0x1234u);
    input.ctxTick96 =
        static_cast<uint32_t>(std::numeric_limits<int32_t>::max()) - 15u;
    input.bucket = (input.ctxTick96 % 384u) / 12u;
    CheckRejectedWithoutMutation(
        state, input, Status80024FD0::TickOutOfRange);

    input = MakeInput(24u, 0u, 0x1234u);
    input.prefixApplied = false;
    CheckRejectedWithoutMutation(
        state, input, Status80024FD0::PrefixMismatch);

    input = MakeInput(24u, 0u, 0x1234u);
    input.bucketChanged = false;
    CheckRejectedWithoutMutation(
        state, input, Status80024FD0::PrefixMismatch);

    input = MakeInput(24u, 0u, 0x1234u);
    input.bucket = 3u;
    CheckRejectedWithoutMutation(
        state, input, Status80024FD0::PrefixMismatch);

    input = MakeInput(24u, 0u, 0x1234u);
    input.globalBindings.slot800943C8 = 2u;
    CheckRejectedWithoutMutation(
        state, input, Status80024FD0::GlobalBindingMismatch);

    ContextState80024FD0 badState = state;
    badState.ctxDword64PsxAddress = 0x801C6F68u;
    input = MakeInput(24u, 0u, 0x1234u);
    CheckRejectedWithoutMutation(
        badState, input, Status80024FD0::ContextStateMismatch);

    badState = state;
    badState.resetKnown = false;
    CheckRejectedWithoutMutation(
        badState, input, Status80024FD0::SourceUnknown);

    input = MakeInput(24u, 0u, 0x10u);
    CheckRejectedWithoutMutation(
        state, input, Status80024FD0::ContextStateMismatch);
}

StaticDispatchState80024FD0 MakeStaticDispatchResetState()
{
    StaticDispatchState80024FD0 state{};
    ResetStaticDispatchState80014344To80024E98(state);
    return state;
}

StaticDispatchInput80024FD0 MakeStaticDispatchInput(
    uint32_t tick96,
    uint32_t callsite = kCallsite801C4C9C)
{
    StaticDispatchInput80024FD0 input{};
    input.callsiteKnown = true;
    input.callsite = callsite;
    input.ctxTick96Known = true;
    input.ctxTick96 = tick96;
    input.bucketKnown = true;
    input.bucket = (tick96 % 384u) / 12u;
    input.predispatchAppliedKnown = true;
    input.predispatchApplied = true;
    input.predispatchStoppedBeforeDispatchKnown = true;
    input.predispatchStoppedBeforeDispatch = true;
    input.noRowDereferenceKnown = true;
    input.noRowDereference = true;
    input.ctxDword56Known = true;
    input.ctxDword56 = tick96 / 384u + 1u;
    input.globalBindings =
        PrSS0Scene0GlobalBindingsDirect::BuildState801C5B14();
    return input;
}

void CheckStaticDispatchRejectedWithoutMutation(
    const StaticDispatchState80024FD0& state,
    const StaticDispatchInput80024FD0& input,
    StaticDispatchStatus80024FD0 expected)
{
    const StaticDispatchResult80024FD0 result =
        ApplyStaticScene0Dispatch800251D8To800259B8(state, input);
    CHECK(result.status == expected);
    CHECK(!result.accepted);
    CHECK(!result.applied);
    CHECK(!result.completedThrough800259B8);
    CHECK(!result.ctxDword48Written);
    CHECK(!result.ctxWord92Written);
    CHECK(!result.currentRingPageWritten);
    CHECK(!result.ringPageCleared);
    CHECK(result.nextState.dword8008ED00 == state.dword8008ED00);
    CHECK(result.nextState.dword8008ED08 == state.dword8008ED08);
    CHECK(result.nextState.dword8008ED14 == state.dword8008ED14);
    CHECK(result.nextState.ctxDword48 == state.ctxDword48);
    CHECK(result.nextState.ctxWord92 == state.ctxWord92);
    CHECK(result.nextState.ctxWord94 == state.ctxWord94);
    CHECK(result.nextState.currentRingPagePsxAddress ==
          state.currentRingPagePsxAddress);
    CHECK(result.nextState.ringPageKnownZero == state.ringPageKnownZero);
}

void CheckStaticDispatchCommon(const StaticDispatchResult80024FD0& result,
                               StaticDispatchKey80024FD0 expectedKey)
{
    CHECK(result.status == StaticDispatchStatus80024FD0::Applied);
    CHECK(result.key == expectedKey);
    CHECK(result.accepted);
    CHECK(result.applied);
    CHECK(result.completedThrough800259B8);
    CHECK(!result.psxMemoryBackingAuthority);
    CHECK(!result.oldWinS0Authority);
    CHECK(!result.stage1Authority);
    CHECK(!result.stage2PlusAuthority);
    CHECK(!result.replayValueAuthority);
    CHECK(!result.hostFilesystemAuthority);
}

void TestStaticDispatchResetAndNoMatch()
{
    const StaticDispatchState80024FD0 state =
        MakeStaticDispatchResetState();
    CHECK(kDispatchRangeBegin800251D8 == 0x800251D8u);
    CHECK(kDispatchRangeEnd800259B8 == 0x800259B8u);
    CHECK(kDispatchKey0 == 0u);
    CHECK(kDispatchKey30 == 30u);
    CHECK(kDispatchKey31 == 31u);
    CHECK(state.resetKnown);
    CHECK(state.dword8008ED00Known && state.dword8008ED00 == 0u);
    CHECK(state.dword8008ED08Known && state.dword8008ED08 == 0u);
    CHECK(state.dword8008ED14Known && state.dword8008ED14 == 0u);
    CHECK(state.stageEventStreamIdKnown && state.stageEventStreamId == 0u);
    CHECK(state.word80091816Known && state.word80091816 == 0u);
    CHECK(state.ctxDword48Known && state.ctxDword48 == 0u);
    CHECK(state.ctxWord92Known && state.ctxWord92 == 0u);
    CHECK(state.ctxWord94Known && state.ctxWord94 == 0u);
    CHECK(state.currentRingPagePsxAddressKnown);
    CHECK(state.currentRingPagePsxAddress == kRingBase80092910);
    for (bool knownZero : state.ringPageKnownZero) {
        CHECK(knownZero);
    }

    const StaticDispatchResult80024FD0 result =
        ApplyStaticScene0Dispatch800251D8To800259B8(
            state, MakeStaticDispatchInput(12u));
    CheckStaticDispatchCommon(result, StaticDispatchKey80024FD0::NoMatch);
    CHECK(!result.helper80024BF4Called);
    CHECK(!result.helper80024FC0Called);
    CHECK(!result.helper80014BDCCalled);
    CHECK(!result.ctxDword48Written);
    CHECK(!result.ctxWord92Written);
    CHECK(!result.currentRingPageWritten);
    CHECK(!result.ringPageCleared);
    CHECK(result.ringPageIndex == 0u);
    CHECK(result.ringPagePsxAddress == kRingBase80092910);
}

void TestStaticDispatchKeys()
{
    const StaticDispatchState80024FD0 state =
        MakeStaticDispatchResetState();

    const StaticDispatchResult80024FD0 key0 =
        ApplyStaticScene0Dispatch800251D8To800259B8(
            state, MakeStaticDispatchInput(384u));
    CheckStaticDispatchCommon(key0, StaticDispatchKey80024FD0::Key0);
    CHECK(key0.helper80024BF4Called);
    CHECK(key0.helper80024BF4ResultKnown);
    CHECK(!key0.helper80024BF4Result);
    CHECK(key0.ctxWord92Written);
    CHECK(key0.nextState.ctxWord92 == 0u);

    const StaticDispatchResult80024FD0 key30 =
        ApplyStaticScene0Dispatch800251D8To800259B8(
            state, MakeStaticDispatchInput(360u));
    CheckStaticDispatchCommon(key30, StaticDispatchKey80024FD0::Key30);
    CHECK(key30.helper80024BF4Called);
    CHECK(key30.helper80024BF4ResultKnown);
    CHECK(!key30.helper80024BF4Result);
    CHECK(key30.helper80024FC0Called);
    CHECK(key30.ctxDword48Written);
    CHECK(key30.nextState.ctxDword48 == 0u);

    const StaticDispatchResult80024FD0 key31 =
        ApplyStaticScene0Dispatch800251D8To800259B8(
            state, MakeStaticDispatchInput(372u));
    CheckStaticDispatchCommon(key31, StaticDispatchKey80024FD0::Key31);
    CHECK(key31.helper80014BDCCalled);
    CHECK(key31.currentRingPageWritten);
    CHECK(key31.ringPageCleared);
    CHECK(key31.ringPageIndex == 1u);
    CHECK(key31.ringPagePsxAddress ==
          kRingBase80092910 + kRingPageBytes80014BDC);
    CHECK(key31.nextState.currentRingPagePsxAddress ==
          key31.ringPagePsxAddress);
    CHECK(key31.nextState.ringPageKnownZero[1]);

    const StaticDispatchResult80024FD0 wrappedPage =
        ApplyStaticScene0Dispatch800251D8To800259B8(
            key31.nextState, MakeStaticDispatchInput(1524u));
    CheckStaticDispatchCommon(
        wrappedPage, StaticDispatchKey80024FD0::Key31);
    CHECK(wrappedPage.ringPageIndex == 0u);
    CHECK(wrappedPage.ringPagePsxAddress == kRingBase80092910);
}

void TestStaticDispatchReset80024E98PreservesPriorOwners()
{
    const StaticDispatchState80024FD0 initial =
        MakeStaticDispatchResetState();
    const StaticDispatchResult80024FD0 key31 =
        ApplyStaticScene0Dispatch800251D8To800259B8(
            initial, MakeStaticDispatchInput(372u));
    CheckStaticDispatchCommon(key31, StaticDispatchKey80024FD0::Key31);

    StaticDispatchState80024FD0 reset = key31.nextState;
    reset.dword8008ED00 = 7u;
    reset.dword8008ED08 = 8u;
    reset.dword8008ED14 = 9u;
    reset.stageEventStreamId = 3u;
    reset.ctxDword48 = 4u;
    reset.ctxWord92 = 5u;
    reset.ctxWord94 = 6u;
    CHECK(ApplyStaticDispatchReset80024E98(reset));
    CHECK(reset.dword8008ED00Known && reset.dword8008ED00 == 0u);
    CHECK(reset.dword8008ED08Known && reset.dword8008ED08 == 0u);
    CHECK(reset.dword8008ED14Known && reset.dword8008ED14 == 0u);
    CHECK(reset.stageEventStreamIdKnown && reset.stageEventStreamId == 0u);
    CHECK(reset.ctxDword48Known && reset.ctxDword48 == 0u);
    CHECK(reset.ctxWord92Known && reset.ctxWord92 == 0u);
    CHECK(reset.ctxWord94Known && reset.ctxWord94 == 0u);
    CHECK(reset.word80091816Known && reset.word80091816 == 0u);
    CHECK(reset.currentRingPagePsxAddress ==
          key31.nextState.currentRingPagePsxAddress);
    CHECK(reset.ringPageKnownZero == key31.nextState.ringPageKnownZero);

    StaticDispatchState80024FD0 rejected = key31.nextState;
    rejected.word80091816 = 1u;
    const StaticDispatchState80024FD0 beforeWordReject = rejected;
    CHECK(!ApplyStaticDispatchReset80024E98(rejected));
    CHECK(rejected.word80091816 == beforeWordReject.word80091816);
    CHECK(rejected.currentRingPagePsxAddress ==
          beforeWordReject.currentRingPagePsxAddress);

    rejected = key31.nextState;
    rejected.currentRingPagePsxAddressKnown = false;
    CHECK(!ApplyStaticDispatchReset80024E98(rejected));

    rejected = key31.nextState;
    rejected.ringPageKnownZero[2] = false;
    CHECK(!ApplyStaticDispatchReset80024E98(rejected));
}

void TestStaticDispatchFailClosedInputs()
{
    const StaticDispatchState80024FD0 state =
        MakeStaticDispatchResetState();
    StaticDispatchInput80024FD0 input = MakeStaticDispatchInput(12u);

    input.callsiteKnown = false;
    CheckStaticDispatchRejectedWithoutMutation(
        state, input, StaticDispatchStatus80024FD0::SourceUnknown);

    input = MakeStaticDispatchInput(12u, 0x801C4D18u);
    CheckStaticDispatchRejectedWithoutMutation(
        state, input, StaticDispatchStatus80024FD0::UnsupportedCallsite);

    input = MakeStaticDispatchInput(12u);
    input.bucket = 32u;
    CheckStaticDispatchRejectedWithoutMutation(
        state, input, StaticDispatchStatus80024FD0::BucketOutOfRange);

    input = MakeStaticDispatchInput(12u);
    input.predispatchApplied = false;
    CheckStaticDispatchRejectedWithoutMutation(
        state, input, StaticDispatchStatus80024FD0::PredispatchMismatch);

    input = MakeStaticDispatchInput(12u);
    input.noRowDereference = false;
    CheckStaticDispatchRejectedWithoutMutation(
        state, input, StaticDispatchStatus80024FD0::PredispatchMismatch);

    input = MakeStaticDispatchInput(12u);
    input.ctxDword56 = 2u;
    CheckStaticDispatchRejectedWithoutMutation(
        state, input, StaticDispatchStatus80024FD0::PredispatchMismatch);

    input = MakeStaticDispatchInput(12u);
    input.globalBindings.slot800943C0 = 0u;
    CheckStaticDispatchRejectedWithoutMutation(
        state, input, StaticDispatchStatus80024FD0::GlobalBindingMismatch);

    StaticDispatchState80024FD0 badState = state;
    badState.dword8008ED08 = 1u;
    input = MakeStaticDispatchInput(12u);
    CheckStaticDispatchRejectedWithoutMutation(
        badState,
        input,
        StaticDispatchStatus80024FD0::StateOutsideScene0ZeroRowPath);

    badState = state;
    badState.ringPageKnownZero[2] = false;
    CheckStaticDispatchRejectedWithoutMutation(
        badState,
        input,
        StaticDispatchStatus80024FD0::StateOutsideScene0ZeroRowPath);
}

} // namespace

int main()
{
    TestConstantsAndReset();
    TestFlagsAndCallsites();
    TestTransitionLookaheadThresholds();
    TestFailClosedInputs();
    TestStaticDispatchResetAndNoMatch();
    TestStaticDispatchKeys();
    TestStaticDispatchReset80024E98PreservesPriorOwners();
    TestStaticDispatchFailClosedInputs();
    if (g_failed != 0) {
        std::printf(
            "test_ss0_scene0_shared_event_predispatch_direct: %d failed\n",
            g_failed);
        return 1;
    }
    std::printf(
        "test_ss0_scene0_shared_event_predispatch_direct: ok\n");
    return 0;
}
