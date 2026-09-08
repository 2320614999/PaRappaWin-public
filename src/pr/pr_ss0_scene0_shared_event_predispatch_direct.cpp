#include "pr_ss0_scene0_shared_event_predispatch_direct.h"

#include <algorithm>
#include <limits>

namespace PrSS0Scene0SharedEventPredispatchDirect {

void ResetContextState80024E98(ContextState80024FD0& state)
{
    state = ContextState80024FD0{};
    state.resetKnown = true;
    state.ctxDword56Known = true;
    state.ctxDword64PsxAddressKnown = true;
    state.ctxDword68PsxAddressKnown = true;
}

Result80024FD0 ApplyPredispatch800250E4To800251D4(
    const ContextState80024FD0& state,
    const Input80024FD0& input)
{
    Result80024FD0 result{};
    result.nextContext = state;
    result.nextCtxFlagsKnown = input.ctxFlagsKnown;
    result.nextCtxFlags = input.ctxFlags;

    if (!state.resetKnown || !state.ctxDword56Known ||
        !state.ctxDword64PsxAddressKnown ||
        !state.ctxDword68PsxAddressKnown || !input.callsiteKnown ||
        !input.ctxTick96Known || !input.word800916D0Known ||
        !input.prefixAppliedKnown || !input.bucketChangedKnown ||
        !input.bucketKnown || !input.ctxFlagsKnown) {
        return result;
    }

    if (input.callsite != kCallsite801C4C9C &&
        input.callsite != kCallsite801C4D14) {
        result.status = Status80024FD0::UnsupportedCallsite;
        return result;
    }

    constexpr uint32_t kMaxSafeTick =
        static_cast<uint32_t>(std::numeric_limits<int32_t>::max()) - 16u;
    if (input.ctxTick96 > kMaxSafeTick) {
        result.status = Status80024FD0::TickOutOfRange;
        return result;
    }

    const uint32_t tickInBar = input.ctxTick96 % 384u;
    const uint32_t expectedBucket = tickInBar / 12u;
    if (!input.prefixApplied || !input.bucketChanged ||
        input.bucket != expectedBucket) {
        result.status = Status80024FD0::PrefixMismatch;
        return result;
    }

    if (!PrSS0Scene0GlobalBindingsDirect::IsExactState801C5B14(
            input.globalBindings)) {
        result.status = Status80024FD0::GlobalBindingMismatch;
        return result;
    }

    if (state.ctxDword64PsxAddress != 0u ||
        state.ctxDword68PsxAddress != 0u || input.ctxFlags != 0u) {
        result.status = Status80024FD0::ContextStateMismatch;
        return result;
    }

    const uint32_t bar = input.ctxTick96 / 384u;
    const bool specialTransition =
        input.word800916D0 == 1u || input.word800916D0 == 2u;
    const uint32_t lookahead = specialTransition ? 12u : 16u;
    const uint32_t threshold = specialTransition ? 372u : 368u;
    const uint32_t ctxDword56V6 = bar + 1u;
    uint32_t rowIndexV7 = bar + 1u;
    if (tickInBar >= threshold) {
        rowIndexV7 = (input.ctxTick96 + lookahead) / 384u + 1u;
    }

    result.ctxDword56V6 = ctxDword56V6;
    result.rowIndexV7 = rowIndexV7;
    const uint32_t rowCount = input.globalBindings.slot800943C8;
    if (rowIndexV7 < rowCount || ctxDword56V6 < rowCount) {
        result.status = Status80024FD0::TableRowAuthorityRequired;
        result.tableRowDereferenceRequired = true;
        return result;
    }

    ContextState80024FD0 next = state;
    next.ctxDword56 = ctxDword56V6;
    uint32_t nextFlags = input.ctxFlags;
    if (input.bucket == 0u) {
        nextFlags |= 2u;
    }
    if ((input.bucket & 1u) == 0u) {
        const uint32_t flagsBeforeEvenBucket = nextFlags;
        nextFlags = flagsBeforeEvenBucket | 8u;
        if ((input.bucket & 7u) == 0u) {
            nextFlags = flagsBeforeEvenBucket | 0xCu;
        }
    }

    result.status = Status80024FD0::Applied;
    result.accepted = true;
    result.applied = true;
    result.stoppedBeforeDispatch = true;
    result.ctxDword56Written = true;
    result.ctxFlagsWritten = nextFlags != input.ctxFlags;
    result.nextCtxFlagsKnown = true;
    result.nextCtxFlags = nextFlags;
    result.nextContext = next;
    return result;
}

void ResetStaticDispatchState80014344To80024E98(
    StaticDispatchState80024FD0& state)
{
    state = StaticDispatchState80024FD0{};
    state.resetKnown = true;
    state.dword8008ED00Known = true;
    state.dword8008ED08Known = true;
    state.dword8008ED14Known = true;
    state.stageEventStreamIdKnown = true;
    state.word80091816Known = true;
    state.ctxDword48Known = true;
    state.ctxWord92Known = true;
    state.ctxWord94Known = true;
    state.currentRingPagePsxAddressKnown = true;
    state.currentRingPagePsxAddress = kRingBase80092910;
    state.ringPageKnownZero.fill(true);
}

bool ApplyStaticDispatchReset80024E98(
    StaticDispatchState80024FD0& state)
{
    if (!state.resetKnown || !state.word80091816Known ||
        state.word80091816 != 0u ||
        !state.currentRingPagePsxAddressKnown) {
        return false;
    }
    for (bool knownZero : state.ringPageKnownZero) {
        if (!knownZero) {
            return false;
        }
    }

    if (state.currentRingPagePsxAddress < kRingBase80092910) {
        return false;
    }
    const uint32_t ringOffset =
        state.currentRingPagePsxAddress - kRingBase80092910;
    if (ringOffset % kRingPageBytes80014BDC != 0u ||
        ringOffset / kRingPageBytes80014BDC >=
            kRingPageCount80014BDC) {
        return false;
    }

    StaticDispatchState80024FD0 next = state;
    next.dword8008ED00Known = true;
    next.dword8008ED00 = 0u;
    next.dword8008ED08Known = true;
    next.dword8008ED08 = 0u;
    next.dword8008ED14Known = true;
    next.dword8008ED14 = 0u;
    next.stageEventStreamIdKnown = true;
    next.stageEventStreamId = 0u;
    next.ctxDword48Known = true;
    next.ctxDword48 = 0u;
    next.ctxWord92Known = true;
    next.ctxWord92 = 0u;
    next.ctxWord94Known = true;
    next.ctxWord94 = 0u;
    state = next;
    return true;
}

StaticDispatchResult80024FD0 ApplyStaticScene0Dispatch800251D8To800259B8(
    const StaticDispatchState80024FD0& state,
    const StaticDispatchInput80024FD0& input)
{
    StaticDispatchResult80024FD0 result{};
    result.nextState = state;

    if (!state.resetKnown || !state.dword8008ED00Known ||
        !state.dword8008ED08Known || !state.dword8008ED14Known ||
        !state.stageEventStreamIdKnown || !state.word80091816Known ||
        !state.ctxDword48Known || !state.ctxWord92Known ||
        !state.ctxWord94Known || !state.currentRingPagePsxAddressKnown ||
        !input.callsiteKnown || !input.ctxTick96Known ||
        !input.bucketKnown || !input.predispatchAppliedKnown ||
        !input.predispatchStoppedBeforeDispatchKnown ||
        !input.noRowDereferenceKnown || !input.ctxDword56Known) {
        return result;
    }

    if (input.callsite != kCallsite801C4C9C &&
        input.callsite != kCallsite801C4D14) {
        result.status = StaticDispatchStatus80024FD0::UnsupportedCallsite;
        return result;
    }
    if (input.bucket > kDispatchKey31) {
        result.status = StaticDispatchStatus80024FD0::BucketOutOfRange;
        return result;
    }
    const uint32_t expectedBucket = (input.ctxTick96 % 384u) / 12u;
    const uint32_t expectedCtxDword56 = input.ctxTick96 / 384u + 1u;
    if (!input.predispatchApplied ||
        !input.predispatchStoppedBeforeDispatch ||
        !input.noRowDereference || input.bucket != expectedBucket ||
        input.ctxDword56 != expectedCtxDword56) {
        result.status = StaticDispatchStatus80024FD0::PredispatchMismatch;
        return result;
    }
    if (!PrSS0Scene0GlobalBindingsDirect::IsExactState801C5B14(
            input.globalBindings)) {
        result.status = StaticDispatchStatus80024FD0::GlobalBindingMismatch;
        return result;
    }

    const uint32_t ringEnd =
        kRingBase80092910 + kRingPageBytes80014BDC * kRingPageCount80014BDC;
    const bool currentRingPageAligned =
        state.currentRingPagePsxAddress >= kRingBase80092910 &&
        state.currentRingPagePsxAddress < ringEnd &&
        (state.currentRingPagePsxAddress - kRingBase80092910) %
                kRingPageBytes80014BDC ==
            0u;
    if (state.dword8008ED00 != 0u || state.dword8008ED08 != 0u ||
        state.dword8008ED14 != 0u || state.stageEventStreamId != 0u ||
        state.word80091816 != 0u || state.ctxDword48 != 0u ||
        state.ctxWord92 != 0u || state.ctxWord94 != 0u ||
        !currentRingPageAligned ||
        !std::all_of(state.ringPageKnownZero.begin(),
                     state.ringPageKnownZero.end(),
                     [](bool knownZero) { return knownZero; })) {
        result.status =
            StaticDispatchStatus80024FD0::StateOutsideScene0ZeroRowPath;
        return result;
    }

    StaticDispatchState80024FD0 next = state;
    result.ringPageIndex = static_cast<uint8_t>(
        (state.currentRingPagePsxAddress - kRingBase80092910) /
        kRingPageBytes80014BDC);
    result.ringPagePsxAddress = state.currentRingPagePsxAddress;
    if (input.bucket == kDispatchKey0) {
        result.key = StaticDispatchKey80024FD0::Key0;
        result.helper80024BF4Called = true;
        result.helper80024BF4ResultKnown = true;
        result.helper80024BF4Result = false;
        next.ctxWord92 = 0u;
        result.ctxWord92Written = true;
    } else if (input.bucket == kDispatchKey30) {
        result.key = StaticDispatchKey80024FD0::Key30;
        result.helper80024BF4Called = true;
        result.helper80024BF4ResultKnown = true;
        result.helper80024BF4Result = false;
        result.helper80024FC0Called = true;
        next.ctxDword48 = state.word80091816;
        result.ctxDword48Written = true;
    } else if (input.bucket == kDispatchKey31) {
        result.key = StaticDispatchKey80024FD0::Key31;
        result.helper80014BDCCalled = true;
        result.ringPageIndex = static_cast<uint8_t>(input.ctxDword56 & 3u);
        result.ringPagePsxAddress =
            kRingBase80092910 +
            kRingPageBytes80014BDC * result.ringPageIndex;
        next.currentRingPagePsxAddress = result.ringPagePsxAddress;
        next.ringPageKnownZero[result.ringPageIndex] = true;
        result.currentRingPageWritten = true;
        result.ringPageCleared = true;
    }

    result.status = StaticDispatchStatus80024FD0::Applied;
    result.accepted = true;
    result.applied = true;
    result.completedThrough800259B8 = true;
    result.nextState = next;
    return result;
}

} // namespace PrSS0Scene0SharedEventPredispatchDirect
