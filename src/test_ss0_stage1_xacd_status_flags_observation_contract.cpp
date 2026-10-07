#include "pr/pr_movie_segment_direct.h"
#include "pr/pr_stage1_lower_cd_producer_direct.h"
#include "pr/pr_stage1_loader_memory_direct.h"
#include "pr/pr_stage1_xa_cd_direct.h"

#include <array>
#include <cstdio>

namespace {

int g_failedChecks = 0;

#define CHECK(expr)                                                        \
    do {                                                                   \
        if (!(expr)) {                                                     \
            std::printf("CHECK failed %s:%d: %s\n", __FILE__, __LINE__,    \
                        #expr);                                            \
            ++g_failedChecks;                                              \
        }                                                                  \
    } while (0)

PrStage1XaCdDirectStatusFlagsObservation80057108 RuntimeObservation(
    uint32_t value) {
    PrStage1XaCdDirectStatusFlagsObservation80057108 observation{};
    observation.source =
        PrStage1XaCdDirectStatusFlagsObservationSource80057108::
            RuntimePsxMemoryObservation;
    observation.psxAddress = 0x80057108u;
    observation.byteSize = 4u;
    observation.valueKnown = true;
    observation.value = value;
    observation.frameKnown = true;
    observation.frame = 4438u;
    observation.pcKnown = true;
    observation.pc = 0x80036AF8u;
    return observation;
}

PrStage1XaCdDirectStatusCounterObservation80057110 RuntimeCounterObservation(
    uint32_t value) {
    PrStage1XaCdDirectStatusCounterObservation80057110 observation{};
    observation.source =
        PrStage1XaCdDirectStatusCounterObservationSource80057110::
            RuntimePsxMemoryObservation;
    observation.psxAddress = 0x80057110u;
    observation.byteSize = 4u;
    observation.valueKnown = true;
    observation.value = value;
    observation.frameKnown = true;
    observation.frame = 4438u;
    observation.pcKnown = true;
    observation.pc = 0x80036AF8u;
    return observation;
}

PrStage1XaCdDirectCallbackPendingObservation80055F7A
RuntimeCallbackPendingObservation(uint16_t value) {
    PrStage1XaCdDirectCallbackPendingObservation80055F7A observation{};
    observation.source =
        PrStage1XaCdDirectCallbackPendingObservationSource80055F7A::
            RuntimePsxMemoryObservation;
    observation.psxAddress = 0x80055F7Au;
    observation.byteSize = 2u;
    observation.valueKnown = true;
    observation.value = value;
    observation.frameKnown = true;
    observation.frame = 4438u;
    observation.pcKnown = true;
    observation.pc = PrMovieSegmentDirect::kSub80035898CheckCallback;
    return observation;
}

void SeedSentinelState(PrStage1XaCdDirectState& state) {
    state.dword_80057108Known = true;
    state.dword_80057108 = 0xAA55AA55u;
    state.statusFlags80057108ObservationPcKnown = true;
    state.statusFlags80057108ObservationPc = 0x80036AF8u;
    state.statusFlags80057108ObservationAcceptedCount = 7u;
    state.statusFlags80057108ObservationRejectedCount = 8u;
    state.statusFlags80057108LastReject = 0u;
    state.dword_80057110Known = true;
    state.dword_80057110 = 0x55AA55AAu;
    state.statusCounter80057110ObservationPcKnown = true;
    state.statusCounter80057110ObservationPc = 0x80036AF8u;
    state.statusCounter80057110ObservationAcceptedCount = 5u;
    state.statusCounter80057110ObservationRejectedCount = 6u;
    state.statusCounter80057110LastReject = 0u;
    state.byte_80057119Known = true;
    state.byte_80057119 = 0x0Eu;
    state.word_80055F7AKnown = true;
    state.word_80055F7A = 0x1234u;
    state.cdCallbackPending80035898Known = true;
    state.cdCallbackPending80035898 = true;
    state.callbackPendingWord80055F7AObservationPcKnown = true;
    state.callbackPendingWord80055F7AObservationPc =
        PrMovieSegmentDirect::kSub80035898CheckCallback;
    state.callbackPendingWord80055F7AObservationAcceptedCount = 3u;
    state.callbackPendingWord80055F7AObservationRejectedCount = 4u;
    state.callbackPendingWord80055F7ALastReject = 0u;
}

struct FakeRuntimePsxMemoryProvider80057108 {
    bool called = false;
    uint32_t requestedAddress = 0u;
    uint32_t requestedByteSize = 0u;
    size_t requestedOutSize = 0u;
    bool readable = true;
    uint32_t value = 0x00000022u;
};

bool FakeReadStatus57108(void* userData,
                         uint32_t psxAddress,
                         uint32_t byteSize,
                         uint8_t* outBytes,
                         size_t outSize) {
    auto& fake =
        *static_cast<FakeRuntimePsxMemoryProvider80057108*>(userData);
    fake.called = true;
    fake.requestedAddress = psxAddress;
    fake.requestedByteSize = byteSize;
    fake.requestedOutSize = outSize;
    if (!fake.readable || outBytes == nullptr || outSize < 4u) {
        return false;
    }
    outBytes[0] = static_cast<uint8_t>(fake.value & 0xFFu);
    outBytes[1] = static_cast<uint8_t>((fake.value >> 8) & 0xFFu);
    outBytes[2] = static_cast<uint8_t>((fake.value >> 16) & 0xFFu);
    outBytes[3] = static_cast<uint8_t>((fake.value >> 24) & 0xFFu);
    return true;
}

struct FakeRuntimePsxMemoryProvider80055F7A {
    bool called = false;
    uint32_t requestedAddress = 0u;
    uint32_t requestedByteSize = 0u;
    size_t requestedOutSize = 0u;
    bool readable = true;
    uint16_t value = 1u;
};

bool FakeReadCallbackPending80055F7A(void* userData,
                                     uint32_t psxAddress,
                                     uint32_t byteSize,
                                     uint8_t* outBytes,
                                     size_t outSize) {
    auto& fake =
        *static_cast<FakeRuntimePsxMemoryProvider80055F7A*>(userData);
    fake.called = true;
    fake.requestedAddress = psxAddress;
    fake.requestedByteSize = byteSize;
    fake.requestedOutSize = outSize;
    if (!fake.readable || outBytes == nullptr || outSize < 2u) {
        return false;
    }
    outBytes[0] = static_cast<uint8_t>(fake.value & 0xFFu);
    outBytes[1] = static_cast<uint8_t>((fake.value >> 8) & 0xFFu);
    return true;
}

struct FakeRuntimePsxMemoryProvider800375BC {
    uint32_t callCount = 0u;
    uint32_t requestedAddress[4]{};
    uint32_t requestedByteSize[4]{};
    bool deadlineReadable = true;
    uint32_t deadlineClock = 2000u;
    bool spinReadable = true;
    uint32_t spinCount = 0u;
};

bool FakeReadCommandWaitLoop800375BC(void* userData,
                                     uint32_t psxAddress,
                                     uint32_t byteSize,
                                     uint8_t* outBytes,
                                     size_t outSize) {
    auto& fake =
        *static_cast<FakeRuntimePsxMemoryProvider800375BC*>(userData);
    const uint32_t index = fake.callCount < 4u ? fake.callCount : 3u;
    fake.requestedAddress[index] = psxAddress;
    fake.requestedByteSize[index] = byteSize;
    ++fake.callCount;
    if (outBytes == nullptr || outSize < 4u || byteSize != 4u) {
        return false;
    }
    uint32_t value = 0u;
    if (psxAddress == 0x80088310u) {
        if (!fake.deadlineReadable) {
            return false;
        }
        value = fake.deadlineClock;
    } else if (psxAddress == 0x80088314u) {
        if (!fake.spinReadable) {
            return false;
        }
        value = fake.spinCount;
    } else {
        return false;
    }
    outBytes[0] = static_cast<uint8_t>(value & 0xFFu);
    outBytes[1] = static_cast<uint8_t>((value >> 8) & 0xFFu);
    outBytes[2] = static_cast<uint8_t>((value >> 16) & 0xFFu);
    outBytes[3] = static_cast<uint8_t>((value >> 24) & 0xFFu);
    return true;
}

void CheckRejectLeavesStateUnchanged(
    PrStage1XaCdDirectStatusFlagsObservation80057108 observation,
    PrStage1XaCdDirectStatusFlagsObservationRejectReason80057108
        expectedReason) {
    PrStage1XaCdDirectState state{};
    SeedSentinelState(state);

    const PrStage1XaCdDirectStatusFlagsObservationResult80057108 result =
        PrStage1XaCdDirectApplyStatusFlagsRuntimeObservation80057108(
            observation,
            state);

    CHECK(!result.accepted);
    CHECK(result.rejectReason == expectedReason);
    CHECK(state.dword_80057108Known);
    CHECK(state.dword_80057108 == 0xAA55AA55u);
    CHECK(state.statusFlags80057108ObservationPcKnown);
    CHECK(state.statusFlags80057108ObservationPc == 0x80036AF8u);
    CHECK(state.statusFlags80057108ObservationAcceptedCount == 7u);
    CHECK(state.statusFlags80057108ObservationRejectedCount == 9u);
    CHECK(state.statusFlags80057108LastReject ==
          static_cast<uint8_t>(expectedReason));
    CHECK(state.byte_80057119Known);
    CHECK(state.byte_80057119 == 0x0Eu);
}

void TestRejectsNonExactObservationShape() {
    PrStage1XaCdDirectStatusFlagsObservation80057108 observation =
        RuntimeObservation(0x10u);
    observation.source =
        PrStage1XaCdDirectStatusFlagsObservationSource80057108::Unknown;
    CheckRejectLeavesStateUnchanged(
        observation,
        PrStage1XaCdDirectStatusFlagsObservationRejectReason80057108::
            NonRuntimePsxMemoryObservation);

    observation = RuntimeObservation(0x10u);
    observation.source =
        PrStage1XaCdDirectStatusFlagsObservationSource80057108::
            RuntimePsxConsumerReadObservation;
    CheckRejectLeavesStateUnchanged(
        observation,
        PrStage1XaCdDirectStatusFlagsObservationRejectReason80057108::
            NonRuntimePsxMemoryObservation);

    observation = RuntimeObservation(0x10u);
    observation.psxAddress = 0x8005710Cu;
    CheckRejectLeavesStateUnchanged(
        observation,
        PrStage1XaCdDirectStatusFlagsObservationRejectReason80057108::
            WrongPsxAddress);

    observation = RuntimeObservation(0x10u);
    observation.byteSize = 1u;
    CheckRejectLeavesStateUnchanged(
        observation,
        PrStage1XaCdDirectStatusFlagsObservationRejectReason80057108::
            WrongByteSize);

    observation = RuntimeObservation(0x10u);
    observation.valueKnown = false;
    CheckRejectLeavesStateUnchanged(
        observation,
        PrStage1XaCdDirectStatusFlagsObservationRejectReason80057108::
            UnknownValue);
}

void TestAcceptedObservationPublishesOnlyStatusFlagsAuthority() {
    PrStage1XaCdDirectState state{};

    const PrStage1XaCdDirectStatusFlagsObservationResult80057108 result =
        PrStage1XaCdDirectApplyStatusFlagsRuntimeObservation80057108(
            RuntimeObservation(0x00000010u),
            state);

    CHECK(result.accepted);
    CHECK(result.rejectReason ==
          PrStage1XaCdDirectStatusFlagsObservationRejectReason80057108::None);
    CHECK(state.dword_80057108Known);
    CHECK(state.dword_80057108 == 0x00000010u);
    CHECK(state.statusFlags80057108ObservationPcKnown);
    CHECK(state.statusFlags80057108ObservationPc == 0x80036AF8u);
    CHECK(state.statusFlags80057108ObservationAcceptedCount == 1u);
    CHECK(state.statusFlags80057108ObservationRejectedCount == 0u);
    CHECK(state.statusFlags80057108LastReject == 0u);
    CHECK(!state.byte_80057119Known);
    CHECK(!state.statusRead80036384Known);
    CHECK(!state.cdSyncExplicitStatusKnown);
    CHECK(!state.cdLowerFeedback80036AF8Known);
}

void TestRejectedObservationDoesNotOverwriteAcceptedStatusFlags() {
    PrStage1XaCdDirectState state{};
    CHECK(PrStage1XaCdDirectApplyStatusFlagsRuntimeObservation80057108(
              RuntimeObservation(0x00000005u),
              state)
              .accepted);

    PrStage1XaCdDirectStatusFlagsObservation80057108 rejected =
        RuntimeObservation(0x00000010u);
    rejected.valueKnown = false;
    CHECK(!PrStage1XaCdDirectApplyStatusFlagsRuntimeObservation80057108(
               rejected,
               state)
               .accepted);

    CHECK(state.dword_80057108Known);
    CHECK(state.dword_80057108 == 0x00000005u);
    CHECK(state.statusFlags80057108ObservationPcKnown);
    CHECK(state.statusFlags80057108ObservationPc == 0x80036AF8u);
    CHECK(state.statusFlags80057108ObservationAcceptedCount == 1u);
    CHECK(state.statusFlags80057108ObservationRejectedCount == 1u);
}

void TestSourceAdapterFailsClosedBeforeObservationGate() {
    PrStage1XaCdDirectState state{};
    SeedSentinelState(state);

    PrStage1XaCdDirectStatusFlagsRuntimeSource80057108 source{};
    source.sourceAvailable = false;
    source.valueKnown = true;
    source.observation = RuntimeObservation(0x10u);

    PrStage1XaCdDirectStatusFlagsRuntimeSourceResult80057108 result =
        PrStage1XaCdDirectPublishStatusFlagsRuntimeSource80057108(source,
                                                                 state);

    CHECK(!result.sourceAvailable);
    CHECK(result.valueKnown);
    CHECK(!result.publishAttempted);
    CHECK(result.blocker ==
          PrStage1XaCdDirectStatusFlagsRuntimeSourceBlocker80057108::
              SourceUnavailable);
    CHECK(!result.observation.accepted);
    CHECK(state.dword_80057108Known);
    CHECK(state.dword_80057108 == 0xAA55AA55u);
    CHECK(state.statusFlags80057108ObservationAcceptedCount == 7u);
    CHECK(state.statusFlags80057108ObservationRejectedCount == 8u);

    source.sourceAvailable = true;
    source.valueKnown = false;
    result =
        PrStage1XaCdDirectPublishStatusFlagsRuntimeSource80057108(source,
                                                                 state);

    CHECK(result.sourceAvailable);
    CHECK(!result.valueKnown);
    CHECK(!result.publishAttempted);
    CHECK(result.blocker ==
          PrStage1XaCdDirectStatusFlagsRuntimeSourceBlocker80057108::
              ValueUnknown);
    CHECK(!result.observation.accepted);
    CHECK(state.dword_80057108Known);
    CHECK(state.dword_80057108 == 0xAA55AA55u);
    CHECK(state.statusFlags80057108ObservationAcceptedCount == 7u);
    CHECK(state.statusFlags80057108ObservationRejectedCount == 8u);
}

void TestRuntimeSourceBuilderRequiresExactWindow() {
    PrStage1XaCdDirectState state{};
    SeedSentinelState(state);

    PrStage1XaCdDirectStatusFlagsRuntimeSourceWindow80057108 window{};
    window.providerInstalled = false;
    window.windowReadable = true;
    window.psxAddress = 0x80057108u;
    window.byteSize = 4u;
    window.valueKnown = true;
    window.value = 0x00000022u;

    PrStage1XaCdDirectStatusFlagsRuntimeSource80057108 source =
        PrStage1XaCdDirectBuildStatusFlagsRuntimeSource80057108(window);
    CHECK(!source.sourceAvailable);
    CHECK(source.valueKnown);
    PrStage1XaCdDirectStatusFlagsRuntimeSourceResult80057108 result =
        PrStage1XaCdDirectPublishStatusFlagsRuntimeSource80057108(source,
                                                                 state);
    CHECK(!result.publishAttempted);
    CHECK(result.blocker ==
          PrStage1XaCdDirectStatusFlagsRuntimeSourceBlocker80057108::
              SourceUnavailable);
    CHECK(state.dword_80057108 == 0xAA55AA55u);
    CHECK(state.statusFlags80057108ObservationAcceptedCount == 7u);

    window.providerInstalled = true;
    window.psxAddress = 0x8005710Cu;
    source = PrStage1XaCdDirectBuildStatusFlagsRuntimeSource80057108(window);
    CHECK(!source.sourceAvailable);
    result = PrStage1XaCdDirectPublishStatusFlagsRuntimeSource80057108(source,
                                                                      state);
    CHECK(!result.publishAttempted);
    CHECK(result.blocker ==
          PrStage1XaCdDirectStatusFlagsRuntimeSourceBlocker80057108::
              SourceUnavailable);
    CHECK(state.dword_80057108 == 0xAA55AA55u);

    window.psxAddress = 0x80057108u;
    window.byteSize = 1u;
    source = PrStage1XaCdDirectBuildStatusFlagsRuntimeSource80057108(window);
    CHECK(!source.sourceAvailable);
    result = PrStage1XaCdDirectPublishStatusFlagsRuntimeSource80057108(source,
                                                                      state);
    CHECK(!result.publishAttempted);
    CHECK(result.blocker ==
          PrStage1XaCdDirectStatusFlagsRuntimeSourceBlocker80057108::
              SourceUnavailable);
    CHECK(state.dword_80057108 == 0xAA55AA55u);

    window.byteSize = 4u;
    window.valueKnown = false;
    source = PrStage1XaCdDirectBuildStatusFlagsRuntimeSource80057108(window);
    CHECK(source.sourceAvailable);
    CHECK(!source.valueKnown);
    result = PrStage1XaCdDirectPublishStatusFlagsRuntimeSource80057108(source,
                                                                      state);
    CHECK(!result.publishAttempted);
    CHECK(result.blocker ==
          PrStage1XaCdDirectStatusFlagsRuntimeSourceBlocker80057108::
              ValueUnknown);
    CHECK(state.dword_80057108 == 0xAA55AA55u);

    window.valueKnown = true;
    window.value = 0x00000022u;
    window.frameKnown = true;
    window.frame = 4438u;
    window.pcKnown = true;
    window.pc = 0x80036AF8u;
    source = PrStage1XaCdDirectBuildStatusFlagsRuntimeSource80057108(window);
    CHECK(source.sourceAvailable);
    CHECK(source.valueKnown);
    CHECK(source.observation.source ==
          PrStage1XaCdDirectStatusFlagsObservationSource80057108::
              RuntimePsxMemoryObservation);
    CHECK(source.observation.psxAddress == 0x80057108u);
    CHECK(source.observation.byteSize == 4u);
    CHECK(source.observation.valueKnown);
    CHECK(source.observation.value == 0x00000022u);
    CHECK(source.observation.frameKnown);
    CHECK(source.observation.frame == 4438u);
    CHECK(source.observation.pcKnown);
    CHECK(source.observation.pc == 0x80036AF8u);

    result = PrStage1XaCdDirectPublishStatusFlagsRuntimeSource80057108(source,
                                                                      state);
    CHECK(result.publishAttempted);
    CHECK(result.observation.accepted);
    CHECK(state.dword_80057108Known);
    CHECK(state.dword_80057108 == 0x00000022u);
    CHECK(state.statusFlags80057108ObservationPcKnown);
    CHECK(state.statusFlags80057108ObservationPc == 0x80036AF8u);
    CHECK(state.statusFlags80057108ObservationAcceptedCount == 8u);
}

void TestRuntimeProviderReadsExactStatusWindowOnly() {
    FakeRuntimePsxMemoryProvider80057108 fake{};
    PrStage1XaCdDirectRuntimePsxMemoryProvider provider{};
    provider.installed = false;
    provider.read = FakeReadStatus57108;
    provider.userData = &fake;
    provider.frameKnown = true;
    provider.frame = 4438u;
    provider.pcKnown = true;
    provider.pc = 0x80036540u;

    auto window =
        PrStage1XaCdDirectReadStatusFlagsRuntimeSourceWindow80057108(provider);
    CHECK(!window.providerInstalled);
    CHECK(!window.readAttempted);
    CHECK(!window.windowReadable);
    CHECK(!window.valueKnown);
    CHECK(!fake.called);

    provider.installed = true;
    provider.read = nullptr;
    window =
        PrStage1XaCdDirectReadStatusFlagsRuntimeSourceWindow80057108(provider);
    CHECK(!window.providerInstalled);
    CHECK(!window.readAttempted);
    CHECK(!window.windowReadable);
    CHECK(!window.valueKnown);
    CHECK(!fake.called);

    provider.read = FakeReadStatus57108;
    fake.readable = false;
    window =
        PrStage1XaCdDirectReadStatusFlagsRuntimeSourceWindow80057108(provider);
    CHECK(window.providerInstalled);
    CHECK(window.readAttempted);
    CHECK(!window.windowReadable);
    CHECK(!window.valueKnown);
    CHECK(fake.called);
    CHECK(fake.requestedAddress == 0x80057108u);
    CHECK(fake.requestedByteSize == 4u);
    CHECK(fake.requestedOutSize == 4u);

    fake.called = false;
    fake.readable = true;
    fake.value = 0x00000022u;
    window =
        PrStage1XaCdDirectReadStatusFlagsRuntimeSourceWindow80057108(provider);
    CHECK(window.providerInstalled);
    CHECK(window.readAttempted);
    CHECK(window.windowReadable);
    CHECK(window.psxAddress == 0x80057108u);
    CHECK(window.byteSize == 4u);
    CHECK(window.valueKnown);
    CHECK(window.value == 0x00000022u);
    CHECK(window.frameKnown);
    CHECK(window.frame == 4438u);
    CHECK(window.pcKnown);
    CHECK(window.pc == 0x80036540u);

    PrStage1XaCdDirectState state{};
    PrStage1XaCdDirectStatusFlagsRuntimeSource80057108 source =
        PrStage1XaCdDirectBuildStatusFlagsRuntimeSource80057108(window);
    PrStage1XaCdDirectStatusFlagsRuntimeSourceResult80057108 result =
        PrStage1XaCdDirectPublishStatusFlagsRuntimeSource80057108(source,
                                                                 state);
    CHECK(result.publishAttempted);
    CHECK(result.observation.accepted);
    CHECK(state.dword_80057108Known);
    CHECK(state.dword_80057108 == 0x00000022u);
    CHECK(state.statusFlags80057108ObservationPcKnown);
    CHECK(state.statusFlags80057108ObservationPc == 0x80036540u);
    CHECK(state.statusFlags80057108ObservationAcceptedCount == 1u);
    CHECK(state.statusFlags80057108ObservationRejectedCount == 0u);
}

void TestLoaderHeapProviderFailsClosedForStatus57108() {
    static PrStage1LoaderMemoryDirectState loaderMemory{};
    PrStage1XaCdDirectRuntimePsxMemoryProvider provider{};
    provider.installed = true;
    provider.read = PrStage1LoaderMemoryDirectReadRuntimePsxMemory;
    provider.userData = &loaderMemory;
    provider.frameKnown = true;
    provider.frame = 4438u;

    const auto window =
        PrStage1XaCdDirectReadStatusFlagsRuntimeSourceWindow80057108(provider);
    CHECK(window.providerInstalled);
    CHECK(window.readAttempted);
    CHECK(!window.windowReadable);
    CHECK(!window.valueKnown);

    PrStage1XaCdDirectState state{};
    const auto source =
        PrStage1XaCdDirectBuildStatusFlagsRuntimeSource80057108(window);
    CHECK(!source.sourceAvailable);
    CHECK(!source.valueKnown);
    const auto result =
        PrStage1XaCdDirectPublishStatusFlagsRuntimeSource80057108(source,
                                                                 state);
    CHECK(!result.publishAttempted);
    CHECK(!result.observation.accepted);
    CHECK(!state.dword_80057108Known);
    CHECK(state.statusFlags80057108ObservationAcceptedCount == 0u);
}

void TestKnownStateProviderReadsOnlyKnownStatus57108() {
    PrStage1XaCdDirectState sourceState{};
    PrStage1XaCdDirectRuntimePsxMemoryProvider provider{};
    provider.installed = true;
    provider.read = PrStage1XaCdDirectReadKnownStateRuntimePsxMemory;
    provider.userData = &sourceState;
    provider.frameKnown = true;
    provider.frame = 4438u;
    provider.pcKnown = true;
    provider.pc = 0x80036AF8u;

    auto window =
        PrStage1XaCdDirectReadStatusFlagsRuntimeSourceWindow80057108(provider);
    CHECK(window.providerInstalled);
    CHECK(window.readAttempted);
    CHECK(!window.windowReadable);
    CHECK(!window.valueKnown);
    auto source =
        PrStage1XaCdDirectBuildStatusFlagsRuntimeSource80057108(window);
    CHECK(!source.sourceAvailable);

    sourceState.dword_80057108Known = true;
    sourceState.dword_80057108 = 0x00000022u;
    window =
        PrStage1XaCdDirectReadStatusFlagsRuntimeSourceWindow80057108(provider);
    CHECK(window.providerInstalled);
    CHECK(window.readAttempted);
    CHECK(window.windowReadable);
    CHECK(window.valueKnown);
    CHECK(window.value == 0x00000022u);
    CHECK(window.frameKnown);
    CHECK(window.frame == 4438u);
    CHECK(window.pcKnown);
    CHECK(window.pc == 0x80036AF8u);

    PrStage1XaCdDirectState targetState{};
    source = PrStage1XaCdDirectBuildStatusFlagsRuntimeSource80057108(window);
    const auto result =
        PrStage1XaCdDirectPublishStatusFlagsRuntimeSource80057108(
            source,
            targetState);
    CHECK(result.sourceAvailable);
    CHECK(result.valueKnown);
    CHECK(result.publishAttempted);
    CHECK(result.observation.accepted);
    CHECK(targetState.dword_80057108Known);
    CHECK(targetState.dword_80057108 == 0x00000022u);
    CHECK(targetState.statusFlags80057108ObservationPcKnown);
    CHECK(targetState.statusFlags80057108ObservationPc == 0x80036AF8u);
    CHECK(targetState.statusFlags80057108ObservationAcceptedCount == 1u);
    CHECK(targetState.statusFlags80057108ObservationRejectedCount == 0u);
}

void TestRuntimeProviderReadsExactPriorStatusCounter57110Only() {
    PrStage1XaCdDirectState directState{};
    auto directObservation = RuntimeCounterObservation(0x00000002u);
    auto directResult =
        PrStage1XaCdDirectApplyStatusCounterRuntimeObservation80057110(
            directObservation,
            directState);
    CHECK(directResult.accepted);
    CHECK(directState.dword_80057110Known);
    CHECK(directState.dword_80057110 == 0x00000002u);
    CHECK(directState.statusCounter80057110ObservationPcKnown);
    CHECK(directState.statusCounter80057110ObservationPc == 0x80036AF8u);
    CHECK(directState.statusCounter80057110ObservationAcceptedCount == 1u);

    directObservation.valueKnown = false;
    directResult =
        PrStage1XaCdDirectApplyStatusCounterRuntimeObservation80057110(
            directObservation,
            directState);
    CHECK(!directResult.accepted);
    CHECK(directResult.rejectReason ==
          PrStage1XaCdDirectStatusCounterObservationRejectReason80057110::
              UnknownValue);
    CHECK(directState.dword_80057110 == 0x00000002u);
    CHECK(directState.statusCounter80057110ObservationPcKnown);
    CHECK(directState.statusCounter80057110ObservationPc == 0x80036AF8u);
    CHECK(directState.statusCounter80057110ObservationAcceptedCount == 1u);
    CHECK(directState.statusCounter80057110ObservationRejectedCount == 1u);

    FakeRuntimePsxMemoryProvider80057108 fake{};
    PrStage1XaCdDirectRuntimePsxMemoryProvider provider{};
    provider.installed = false;
    provider.read = FakeReadStatus57108;
    provider.userData = &fake;
    provider.frameKnown = true;
    provider.frame = 4438u;
    provider.pcKnown = true;
    provider.pc = 0x80036AF8u;

    auto window =
        PrStage1XaCdDirectReadStatusCounterRuntimeSourceWindow80057110(
            provider);
    CHECK(!window.providerInstalled);
    CHECK(!window.readAttempted);
    CHECK(!window.windowReadable);
    CHECK(!window.valueKnown);
    CHECK(!fake.called);

    provider.installed = true;
    fake.readable = false;
    window =
        PrStage1XaCdDirectReadStatusCounterRuntimeSourceWindow80057110(
            provider);
    CHECK(window.providerInstalled);
    CHECK(window.readAttempted);
    CHECK(!window.windowReadable);
    CHECK(!window.valueKnown);
    CHECK(fake.called);
    CHECK(fake.requestedAddress == 0x80057110u);
    CHECK(fake.requestedByteSize == 4u);
    CHECK(fake.requestedOutSize == 4u);

    fake.called = false;
    fake.readable = true;
    fake.value = 0x00000003u;
    window =
        PrStage1XaCdDirectReadStatusCounterRuntimeSourceWindow80057110(
            provider);
    CHECK(window.providerInstalled);
    CHECK(window.readAttempted);
    CHECK(window.windowReadable);
    CHECK(window.psxAddress == 0x80057110u);
    CHECK(window.byteSize == 4u);
    CHECK(window.valueKnown);
    CHECK(window.value == 0x00000003u);
    CHECK(window.frameKnown);
    CHECK(window.frame == 4438u);
    CHECK(window.pcKnown);
    CHECK(window.pc == 0x80036AF8u);

    PrStage1XaCdDirectState targetState{};
    auto source =
        PrStage1XaCdDirectBuildStatusCounterRuntimeSource80057110(window);
    CHECK(source.sourceAvailable);
    CHECK(source.valueKnown);
    CHECK(source.observation.source ==
          PrStage1XaCdDirectStatusCounterObservationSource80057110::
              RuntimePsxMemoryObservation);
    CHECK(source.observation.psxAddress == 0x80057110u);
    CHECK(source.observation.byteSize == 4u);
    CHECK(source.observation.valueKnown);
    CHECK(source.observation.value == 0x00000003u);
    auto result =
        PrStage1XaCdDirectPublishStatusCounterRuntimeSource80057110(
            source,
            targetState);
    CHECK(result.sourceAvailable);
    CHECK(result.valueKnown);
    CHECK(result.publishAttempted);
    CHECK(result.observation.accepted);
    CHECK(targetState.dword_80057110Known);
    CHECK(targetState.dword_80057110 == 0x00000003u);
    CHECK(targetState.statusCounter80057110ObservationPcKnown);
    CHECK(targetState.statusCounter80057110ObservationPc == 0x80036AF8u);
    CHECK(targetState.statusCounter80057110ObservationAcceptedCount == 1u);
    CHECK(targetState.statusCounter80057110ObservationRejectedCount == 0u);

    PrStage1XaCdDirectStatusCounterRuntimeSource80057110 unavailable{};
    unavailable.sourceAvailable = false;
    unavailable.valueKnown = true;
    unavailable.observation = RuntimeCounterObservation(0x00000007u);
    SeedSentinelState(targetState);
    result = PrStage1XaCdDirectPublishStatusCounterRuntimeSource80057110(
        unavailable,
        targetState);
    CHECK(!result.publishAttempted);
    CHECK(result.blocker ==
          PrStage1XaCdDirectStatusCounterRuntimeSourceBlocker80057110::
              SourceUnavailable);
    CHECK(targetState.dword_80057110Known);
    CHECK(targetState.dword_80057110 == 0x55AA55AAu);
    CHECK(targetState.statusCounter80057110ObservationPcKnown);
    CHECK(targetState.statusCounter80057110ObservationPc == 0x80036AF8u);
    CHECK(targetState.statusCounter80057110ObservationAcceptedCount == 5u);
    CHECK(targetState.statusCounter80057110ObservationRejectedCount == 6u);

    PrStage1XaCdDirectState sourceState{};
    provider.read = PrStage1XaCdDirectReadKnownStateRuntimePsxMemory;
    provider.userData = &sourceState;
    window =
        PrStage1XaCdDirectReadStatusCounterRuntimeSourceWindow80057110(
            provider);
    CHECK(window.providerInstalled);
    CHECK(window.readAttempted);
    CHECK(!window.windowReadable);
    CHECK(!window.valueKnown);

    sourceState.dword_80057110Known = true;
    sourceState.dword_80057110 = 0x00000004u;
    window =
        PrStage1XaCdDirectReadStatusCounterRuntimeSourceWindow80057110(
            provider);
    CHECK(window.providerInstalled);
    CHECK(window.readAttempted);
    CHECK(window.windowReadable);
    CHECK(window.valueKnown);
    CHECK(window.value == 0x00000004u);
    source =
        PrStage1XaCdDirectBuildStatusCounterRuntimeSource80057110(window);
    PrStage1XaCdDirectState knownStateTarget{};
    result = PrStage1XaCdDirectPublishStatusCounterRuntimeSource80057110(
        source,
        knownStateTarget);
    CHECK(result.publishAttempted);
    CHECK(result.observation.accepted);
    CHECK(knownStateTarget.dword_80057110Known);
    CHECK(knownStateTarget.dword_80057110 == 0x00000004u);
    CHECK(knownStateTarget.statusCounter80057110ObservationPcKnown);
    CHECK(knownStateTarget.statusCounter80057110ObservationPc == 0x80036AF8u);
}

void TestSourceAdapterDelegatesOnlyCompleteRuntimeObservation() {
    PrStage1XaCdDirectState state{};

    PrStage1XaCdDirectStatusFlagsRuntimeSource80057108 source{};
    source.sourceAvailable = true;
    source.valueKnown = true;
    source.observation = RuntimeObservation(0x00000010u);

    const PrStage1XaCdDirectStatusFlagsRuntimeSourceResult80057108 result =
        PrStage1XaCdDirectPublishStatusFlagsRuntimeSource80057108(source,
                                                                 state);

    CHECK(result.sourceAvailable);
    CHECK(result.valueKnown);
    CHECK(result.publishAttempted);
    CHECK(result.blocker ==
          PrStage1XaCdDirectStatusFlagsRuntimeSourceBlocker80057108::None);
    CHECK(result.observation.accepted);
    CHECK(state.dword_80057108Known);
    CHECK(state.dword_80057108 == 0x00000010u);
    CHECK(state.statusFlags80057108ObservationPcKnown);
    CHECK(state.statusFlags80057108ObservationPc == 0x80036AF8u);
    CHECK(state.statusFlags80057108ObservationAcceptedCount == 1u);
    CHECK(state.statusFlags80057108ObservationRejectedCount == 0u);

    PrStage1XaCdDirectStatusFlagsRuntimeSource80057108 rejected{};
    rejected.sourceAvailable = true;
    rejected.valueKnown = true;
    rejected.observation = RuntimeObservation(0x00000020u);
    rejected.observation.source =
        PrStage1XaCdDirectStatusFlagsObservationSource80057108::
            RuntimePsxConsumerReadObservation;

    const PrStage1XaCdDirectStatusFlagsRuntimeSourceResult80057108
        rejectedResult =
            PrStage1XaCdDirectPublishStatusFlagsRuntimeSource80057108(
                rejected,
                state);

    CHECK(rejectedResult.publishAttempted);
    CHECK(rejectedResult.blocker ==
          PrStage1XaCdDirectStatusFlagsRuntimeSourceBlocker80057108::
              ObservationRejected);
    CHECK(!rejectedResult.observation.accepted);
    CHECK(rejectedResult.observation.rejectReason ==
          PrStage1XaCdDirectStatusFlagsObservationRejectReason80057108::
              NonRuntimePsxMemoryObservation);
    CHECK(state.dword_80057108Known);
    CHECK(state.dword_80057108 == 0x00000010u);
    CHECK(state.statusFlags80057108ObservationPcKnown);
    CHECK(state.statusFlags80057108ObservationPc == 0x80036AF8u);
    CHECK(state.statusFlags80057108ObservationAcceptedCount == 1u);
    CHECK(state.statusFlags80057108ObservationRejectedCount == 1u);
}

void TestStatusFlagsDoNotCompleteModeCommandByThemselves() {
    PrStage1XaCdDirectState state{};

    PrStage1XaCdDirectStatusFlagsRuntimeSource80057108 source{};
    source.sourceAvailable = true;
    source.valueKnown = true;
    source.observation = RuntimeObservation(0x00000022u);

    const PrStage1XaCdDirectStatusFlagsRuntimeSourceResult80057108 result =
        PrStage1XaCdDirectPublishStatusFlagsRuntimeSource80057108(source,
                                                                 state);
    CHECK(result.publishAttempted);
    CHECK(result.observation.accepted);
    CHECK(state.dword_80057108Known);

    const PrStage1LowerCdProducerDirect::CommandAttr800375BC modeAttr =
        PrStage1LowerCdProducerDirect::ResolveCommandAttr800375BC(0x0Eu);
    CHECK(modeAttr.known);
    CHECK(modeAttr.paramCount == 1u);
    CHECK(modeAttr.needsSetlocKnown);
    CHECK(!modeAttr.needsSetloc);

    PrStage1LowerCdProducerDirect::CommandWrapperInput80036540 modeCmd{};
    modeCmd.command = 0x0Eu;
    modeCmd.argsKnown = true;
    modeCmd.argsPresent = true;
    modeCmd.status57108Known = state.dword_80057108Known;
    modeCmd.status57108 = static_cast<uint8_t>(state.dword_80057108 & 0xFFu);

    PrStage1LowerCdProducerDirect::CommandWrapperResult80036540 modeResult =
        PrStage1LowerCdProducerDirect::BuildCommandWrapperResult80036540(
            modeCmd);
    CHECK(!modeResult.produced);
    CHECK(modeResult.incomplete);

    modeCmd.commandNeedsSetlocKnown = modeAttr.needsSetlocKnown;
    modeCmd.commandNeedsSetloc = modeAttr.needsSetloc;
    modeResult =
        PrStage1LowerCdProducerDirect::BuildCommandWrapperResult80036540(
            modeCmd);
    CHECK(!modeResult.produced);
    CHECK(modeResult.incomplete);

    modeCmd.attempts[0].commandKnown = true;
    modeCmd.attempts[0].command.produced = true;
    modeCmd.attempts[0].command.psxReturn = 0;
    modeResult =
        PrStage1LowerCdProducerDirect::BuildCommandWrapperResult80036540(
            modeCmd);
    CHECK(modeResult.produced);
    CHECK(!modeResult.incomplete);
    CHECK(modeResult.attemptsUsed == 1u);
}

void Test800375BCCommandPreservesStatusFlagsUntilResponse() {
    PrStage1XaCdDirectState state{};
    CHECK(PrStage1XaCdDirectApplyStatusFlagsRuntimeObservation80057108(
        RuntimeObservation(0x22u), state).accepted);

    PrStage1XaCdDirectCommandInput800375BC command{};
    command.command = 0x0Eu;
    command.argsKnown = true;
    command.argCount = 1u;
    command.args[0] = 0xA0u;
    command.skipWait = true;

    const PrStage1XaCdDirectCommandResult800375BC commandResult =
        PrStage1XaCdDirectApplySub800375BCCommand(state, command);
    CHECK(commandResult.called);
    CHECK(commandResult.byte80057119Known);
    CHECK(commandResult.byte80057119 == 0x0Eu);
    CHECK(state.byte_800573D4Known);
    CHECK(state.byte_800573D4 == 0u);
    CHECK(state.byte_80057119Known);
    CHECK(state.byte_80057119 == 0x0Eu);
    CHECK(state.dword_80057108Known);
    CHECK(state.dword_80057108 == 0x22u);
    CHECK(state.statusFlags80057108ObservationPcKnown);
    CHECK(state.statusFlags80057108ObservationPc ==
          PrMovieSegmentDirect::kSub80036AF8CdCallbackEvent);
    CHECK(state.statusFlags80057108ObservationAcceptedCount == 1u);
    CHECK(state.statusFlags80057108ObservationRejectedCount == 0u);

    const PrStage1LowerCdProducerDirect::CommandAttr800375BC modeAttr =
        PrStage1LowerCdProducerDirect::ResolveCommandAttr800375BC(0x0Eu);
    CHECK(modeAttr.known);
    CHECK(modeAttr.needsSetlocKnown);
    CHECK(!modeAttr.needsSetloc);

    PrStage1LowerCdProducerDirect::CommandWrapperInput80036540 modeCmd{};
    modeCmd.command = 0x0Eu;
    modeCmd.argsKnown = true;
    modeCmd.argsPresent = true;
    modeCmd.status57108Known = state.dword_80057108Known;
    modeCmd.status57108 = static_cast<uint8_t>(state.dword_80057108 & 0xFFu);
    modeCmd.commandNeedsSetlocKnown = modeAttr.needsSetlocKnown;
    modeCmd.commandNeedsSetloc = modeAttr.needsSetloc;

    const PrStage1LowerCdProducerDirect::CommandWrapperResult80036540
        missingCommand =
            PrStage1LowerCdProducerDirect::BuildCommandWrapperResult80036540(
                modeCmd);
    CHECK(!missingCommand.produced);
    CHECK(missingCommand.incomplete);
    CHECK(missingCommand.attemptsUsed == 1u);

    modeCmd.attempts[0].commandKnown = true;
    modeCmd.attempts[0].command.produced = true;
    modeCmd.attempts[0].command.psxReturn = 0;
    const PrStage1LowerCdProducerDirect::CommandWrapperResult80036540
        modeProduced =
            PrStage1LowerCdProducerDirect::BuildCommandWrapperResult80036540(
                modeCmd);
    CHECK(modeProduced.produced);
    CHECK(!modeProduced.incomplete);
    CHECK(modeProduced.attemptsUsed == 1u);
}

void Test800375BCDirectApplyRequiresExplicitWaitLoopResult() {
    PrStage1XaCdDirectState state{};

    PrStage1XaCdDirectCommandInput800375BC command{};
    command.command = 0x0Eu;
    command.argsKnown = true;
    command.argCount = 1u;
    command.args[0] = 0xA0u;
    command.skipWait = false;
    command.clockKnown = true;
    command.clockNow = 2000;
    command.waitLoopResultKnown = false;

    PrStage1XaCdDirectCommandResult800375BC commandResult =
        PrStage1XaCdDirectApplySub800375BCCommand(state, command);
    CHECK(commandResult.called);
    CHECK(commandResult.waitLoopRequested);
    CHECK(commandResult.incomplete);
    CHECK(!commandResult.psxReturnKnown);
    CHECK(!commandResult.waitLoopResultKnown);
    CHECK(state.byte_800573D4Known);
    CHECK(state.byte_800573D4 == 0u);
    CHECK(!state.dword_80057108Known); // an issue is not a drive-status response
    CHECK(state.byte_80057119Known);
    CHECK(state.byte_80057119 == 0x0Eu);
    CHECK(state.cdCommandTimeoutDeadline80088310Known);
    CHECK(state.cdCommandTimeoutDeadline80088310 == 2960);
    CHECK(state.cdCommandTimeoutSpin80088314Known);
    CHECK(state.cdCommandTimeoutSpin80088314 == 0u);

    PrStage1XaCdDirectRuntimePsxMemoryProvider provider{};
    provider.installed = true;
    provider.read = PrStage1XaCdDirectReadKnownStateRuntimePsxMemory;
    provider.userData = &state;
    PrStage1XaCdDirectCommandWaitLoopRuntimeSourceWindow800375BC window =
        PrStage1XaCdDirectReadCommandWaitLoopRuntimeSourceWindow800375BC(
            provider,
            true,
            2000);
    CHECK(window.deadlineReadAttempted);
    CHECK(window.spinReadAttempted);
    CHECK(window.deadlineReadable);
    CHECK(window.spinReadable);
    PrStage1XaCdDirectCommandWaitLoopRuntimeSource800375BC waitSource =
        PrStage1XaCdDirectBuildCommandWaitLoopRuntimeSource800375BC(window);
    PrStage1XaCdDirectCommandWaitLoopRuntimeSourceResult800375BC waitResult =
        PrStage1XaCdDirectResolveCommandWaitLoopRuntimeSource800375BC(
            waitSource);
    CHECK(waitResult.sourceAvailable);
    CHECK(!waitResult.resultKnown);
    CHECK(!waitResult.feedsCommand);

    window = PrStage1XaCdDirectReadCommandWaitLoopRuntimeSourceWindow800375BC(
        provider,
        true,
        2961);
    waitSource =
        PrStage1XaCdDirectBuildCommandWaitLoopRuntimeSource800375BC(window);
    waitResult =
        PrStage1XaCdDirectResolveCommandWaitLoopRuntimeSource800375BC(
            waitSource);
    CHECK(waitResult.sourceAvailable);
    CHECK(waitResult.resultKnown);
    CHECK(waitResult.feedsCommand);
    CHECK(waitResult.psxReturn == -1);

    PrStage1XaCdDirectState explicitState{};
    command.waitLoopResultKnown = true;
    command.waitLoopPsxReturn = -1;
    commandResult =
        PrStage1XaCdDirectApplySub800375BCCommand(explicitState, command);
    CHECK(commandResult.called);
    CHECK(commandResult.waitLoopRequested);
    CHECK(!commandResult.incomplete);
    CHECK(commandResult.waitLoopResultKnown);
    CHECK(commandResult.waitLoopPsxReturn == -1);
    CHECK(commandResult.psxReturnKnown);
    CHECK(commandResult.psxReturn == -1);
}

void TestStatusFlagsAndModeCommandStillNeedReadPumpStartFacts() {
    PrStage1XaCdDirectState state{};

    PrStage1XaCdDirectStatusFlagsRuntimeSource80057108 source{};
    source.sourceAvailable = true;
    source.valueKnown = true;
    source.observation = RuntimeObservation(0x00000022u);
    CHECK(PrStage1XaCdDirectPublishStatusFlagsRuntimeSource80057108(source,
                                                                    state)
              .observation.accepted);

    PrStage1LowerCdProducerDirect::CommandWrapperInput80036540 modeCmd{};
    modeCmd.command = 0x0Eu;
    modeCmd.argsKnown = true;
    modeCmd.argsPresent = true;
    modeCmd.status57108Known = state.dword_80057108Known;
    modeCmd.status57108 = static_cast<uint8_t>(state.dword_80057108 & 0xFFu);
    modeCmd.commandNeedsSetlocKnown = true;
    modeCmd.commandNeedsSetloc = false;

    PrStage1LowerCdProducerDirect::CommandWrapperResult80036540 modeMissing =
        PrStage1LowerCdProducerDirect::BuildCommandWrapperResult80036540(
            modeCmd);
    CHECK(!modeMissing.produced);
    CHECK(modeMissing.incomplete);

    modeCmd.attempts[0].commandKnown = true;
    modeCmd.attempts[0].command.produced = true;
    modeCmd.attempts[0].command.psxReturn = 0;
    const PrStage1LowerCdProducerDirect::CommandWrapperResult80036540
        modeProduced =
            PrStage1LowerCdProducerDirect::BuildCommandWrapperResult80036540(
                modeCmd);
    CHECK(modeProduced.produced);
    CHECK(!modeProduced.incomplete);

    PrStage1LowerCdProducerDirect::CallbackSwapInput80036510 syncSwap{};
    syncSwap.oldCallbackKnown = true;
    syncSwap.oldCallback = 0u;
    syncSwap.newCallback = 0u;
    const PrStage1LowerCdProducerDirect::CallbackSwapResult80036510
        syncCleared =
            PrStage1LowerCdProducerDirect::BuildCallbackSwapResult80036510(
                syncSwap);

    PrStage1LowerCdProducerDirect::CallbackSwapInput80036528 readySwap{};
    readySwap.oldCallbackKnown = true;
    readySwap.oldCallback = 0u;
    readySwap.newCallback = 0u;
    const PrStage1LowerCdProducerDirect::CallbackSwapResult80036528
        readyCleared =
            PrStage1LowerCdProducerDirect::BuildCallbackSwapResult80036528(
                readySwap);

    PrStage1LowerCdProducerDirect::StatusReadInput80036384 statusInput{};
    statusInput.statusKnown = true;
    statusInput.status = 0u;
    const PrStage1LowerCdProducerDirect::StatusReadResult80036384 status =
        PrStage1LowerCdProducerDirect::BuildStatusReadResult80036384(
            statusInput);

    PrStage1LowerCdProducerDirect::ReadStartSetupInput80038FC0 setupInput{};
    setupInput.sectorCountKnown = true;
    setupInput.sectorCount = 4;
    setupInput.dstKnown = true;
    setupInput.dst = static_cast<int32_t>(0x801C15B0u);
    setupInput.modeKnown = true;
    setupInput.mode = 0x80u;
    setupInput.savedSyncCallback = syncCleared;
    setupInput.savedReadyCallback = readyCleared;
    setupInput.status = status;
    setupInput.clockKnown = true;
    setupInput.clockNow = 1513;
    const PrStage1LowerCdProducerDirect::ReadStartSetupResult80038FC0 setup =
        PrStage1LowerCdProducerDirect::BuildReadStartSetupResult80038FC0(
            setupInput);
    CHECK(setup.produced);
    CHECK(!setup.incomplete);
    CHECK(setup.modeWord8005741C == 0xA0u);

    PrStage1LowerCdProducerDirect::ReadPumpInput80038DE8 pump{};
    pump.clearSyncCallback = syncCleared;
    pump.clearReadyCallback = readyCleared;
    pump.status = status;
    pump.clockKnown = true;
    pump.clockNow = 1513;
    pump.preReadSetupKnown = true;
    pump.modeKnown = true;
    pump.modeWord8005741C = setup.modeWord8005741C;
    pump.modeCommandResult = modeProduced;
    pump.activeDstKnown = true;
    pump.activeDst80057414 = setup.dst80057414;
    pump.activeSectorCountKnown = true;
    pump.activeSectorCount80057410 = setup.sectorCount80057410;

    const PrStage1LowerCdProducerDirect::ReadPumpResult80038DE8
        pumpMissingStart =
            PrStage1LowerCdProducerDirect::BuildReadPumpResult80038DE8(pump);
    CHECK(!pumpMissingStart.produced);
    CHECK(pumpMissingStart.incomplete);

    pump.locSectorKnown = true;
    pump.locSector80057430 = 138859;
    const PrStage1LowerCdProducerDirect::ReadPumpResult80038DE8
        pumpMissingStartRead =
            PrStage1LowerCdProducerDirect::BuildReadPumpResult80038DE8(pump);
    CHECK(!pumpMissingStartRead.produced);
    CHECK(pumpMissingStartRead.incomplete);

    pump.startReadResultKnown = true;
    pump.startReadResult.produced = true;
    pump.startReadResult.psxReturn = 1;
    const PrStage1LowerCdProducerDirect::ReadPumpResult80038DE8 pumpComplete =
        PrStage1LowerCdProducerDirect::BuildReadPumpResult80038DE8(pump);
    CHECK(pumpComplete.produced);
    CHECK(!pumpComplete.incomplete);
    CHECK(pumpComplete.modeCommandIssued);
    CHECK(pumpComplete.callbackInstalled80038BC4);
    CHECK(pumpComplete.psxReturn == 4);
}

void Test80036AF8PublishesStatusFlagsThroughObservationGate() {
    PrStage1XaCdDirectState state{};
    PrMovieSegmentDirect::CdCallbackEventInput80036AF8 input{};
    input.interruptKnown = true;
    input.interruptCode = 2u;
    input.resultBytesKnown = true;
    input.resultByteCount = 2u;
    input.resultBytes[0] = 0x10u;
    input.resultBytes[1] = 0x02u;
    input.commandKnown = true;
    input.command = 0x0Eu;

    const PrMovieSegmentDirect::CdCallbackEventResult80036AF8 result =
        PrStage1XaCdDirectApplySub80036AF8CdLowerEvent(state, input);

    CHECK(result.called);
    CHECK(result.dword80057108Known);
    CHECK(result.dword80057108 == 0x10u);
    CHECK(state.dword_80057108Known);
    CHECK(state.dword_80057108 == 0x10u);
    CHECK(state.statusFlags80057108ObservationPcKnown);
    CHECK(state.statusFlags80057108ObservationPc ==
          PrMovieSegmentDirect::kSub80036AF8CdCallbackEvent);
    CHECK(state.statusFlags80057108ObservationAcceptedCount == 1u);
    CHECK(state.statusFlags80057108ObservationRejectedCount == 0u);
    CHECK(state.cdLowerEvent80036AF8Serial == 1u);
}

void CheckCallbackPendingRejectLeavesStateUnchanged(
    PrStage1XaCdDirectCallbackPendingObservation80055F7A observation,
    PrStage1XaCdDirectCallbackPendingObservationRejectReason80055F7A
        expectedReason) {
    PrStage1XaCdDirectState state{};
    SeedSentinelState(state);

    const PrStage1XaCdDirectCallbackPendingObservationResult80055F7A result =
        PrStage1XaCdDirectApplyCallbackPendingRuntimeObservation80055F7A(
            observation,
            state);

    CHECK(!result.accepted);
    CHECK(result.rejectReason == expectedReason);
    CHECK(state.word_80055F7AKnown);
    CHECK(state.word_80055F7A == 0x1234u);
    CHECK(state.cdCallbackPending80035898Known);
    CHECK(state.cdCallbackPending80035898);
    CHECK(state.callbackPendingWord80055F7AObservationPcKnown);
    CHECK(state.callbackPendingWord80055F7AObservationPc ==
          PrMovieSegmentDirect::kSub80035898CheckCallback);
    CHECK(state.callbackPendingWord80055F7AObservationAcceptedCount == 3u);
    CHECK(state.callbackPendingWord80055F7AObservationRejectedCount == 5u);
    CHECK(state.callbackPendingWord80055F7ALastReject ==
          static_cast<uint8_t>(expectedReason));
}

void TestCallbackPendingWordRequiresExactRuntimeObservation() {
    PrStage1XaCdDirectCallbackPendingObservation80055F7A observation =
        RuntimeCallbackPendingObservation(1u);
    observation.source =
        PrStage1XaCdDirectCallbackPendingObservationSource80055F7A::Unknown;
    CheckCallbackPendingRejectLeavesStateUnchanged(
        observation,
        PrStage1XaCdDirectCallbackPendingObservationRejectReason80055F7A::
            NonRuntimePsxMemoryObservation);

    observation = RuntimeCallbackPendingObservation(1u);
    observation.source =
        PrStage1XaCdDirectCallbackPendingObservationSource80055F7A::
            RuntimePsxConsumerReadObservation;
    CheckCallbackPendingRejectLeavesStateUnchanged(
        observation,
        PrStage1XaCdDirectCallbackPendingObservationRejectReason80055F7A::
            NonRuntimePsxMemoryObservation);

    observation = RuntimeCallbackPendingObservation(1u);
    observation.psxAddress = 0x80055F78u;
    CheckCallbackPendingRejectLeavesStateUnchanged(
        observation,
        PrStage1XaCdDirectCallbackPendingObservationRejectReason80055F7A::
            WrongPsxAddress);

    observation = RuntimeCallbackPendingObservation(1u);
    observation.byteSize = 4u;
    CheckCallbackPendingRejectLeavesStateUnchanged(
        observation,
        PrStage1XaCdDirectCallbackPendingObservationRejectReason80055F7A::
            WrongByteSize);

    observation = RuntimeCallbackPendingObservation(1u);
    observation.valueKnown = false;
    CheckCallbackPendingRejectLeavesStateUnchanged(
        observation,
        PrStage1XaCdDirectCallbackPendingObservationRejectReason80055F7A::
            UnknownValue);
}

void TestCallbackPendingWordObservationFeedsCheckCallback() {
    PrStage1XaCdDirectState state{};

    const PrStage1XaCdDirectCallbackPendingObservationResult80055F7A one =
        PrStage1XaCdDirectApplyCallbackPendingRuntimeObservation80055F7A(
            RuntimeCallbackPendingObservation(1u),
            state);
    CHECK(one.accepted);
    CHECK(state.word_80055F7AKnown);
    CHECK(state.word_80055F7A == 1u);
    CHECK(state.cdCallbackPending80035898Known);
    CHECK(state.cdCallbackPending80035898);
    CHECK(state.callbackPendingWord80055F7AObservationPcKnown);
    CHECK(state.callbackPendingWord80055F7AObservationPc ==
          PrMovieSegmentDirect::kSub80035898CheckCallback);
    CHECK(state.callbackPendingWord80055F7AObservationAcceptedCount == 1u);
    CHECK(state.callbackPendingWord80055F7AObservationRejectedCount == 0u);
    CHECK(state.callbackPendingWord80055F7ALastReject == 0u);

    PrMovieSegmentDirect::CheckCallbackInput80035898 input{};
    input.word80055F7AKnown = state.word_80055F7AKnown;
    input.word80055F7A = state.word_80055F7A;
    const PrMovieSegmentDirect::CheckCallbackResult80035898 pending =
        PrMovieSegmentDirect::PsxCall80035898_CheckCallback(input);
    CHECK(pending.pendingKnown);
    CHECK(pending.pending);

    const PrStage1XaCdDirectCallbackPendingObservationResult80055F7A zero =
        PrStage1XaCdDirectApplyCallbackPendingRuntimeObservation80055F7A(
            RuntimeCallbackPendingObservation(0u),
            state);
    CHECK(zero.accepted);
    CHECK(state.word_80055F7AKnown);
    CHECK(state.word_80055F7A == 0u);
    CHECK(state.cdCallbackPending80035898Known);
    CHECK(!state.cdCallbackPending80035898);
    CHECK(state.callbackPendingWord80055F7AObservationAcceptedCount == 2u);

    input.word80055F7AKnown = state.word_80055F7AKnown;
    input.word80055F7A = state.word_80055F7A;
    const PrMovieSegmentDirect::CheckCallbackResult80035898 idle =
        PrMovieSegmentDirect::PsxCall80035898_CheckCallback(input);
    CHECK(idle.pendingKnown);
    CHECK(!idle.pending);
}

void TestCallbackPendingRuntimeSourceAdapterFeedsCheckCallbackOnlyWhenExactWindow() {
    PrStage1XaCdDirectState state{};
    SeedSentinelState(state);

    PrStage1XaCdDirectRuntimePsxMemoryProvider provider{};
    PrStage1XaCdDirectCallbackPendingRuntimeSourceWindow80055F7A window =
        PrStage1XaCdDirectReadCallbackPendingRuntimeSourceWindow80055F7A(
            provider);
    CHECK(!window.providerInstalled);
    CHECK(!window.readAttempted);
    PrStage1XaCdDirectCallbackPendingRuntimeSource80055F7A source =
        PrStage1XaCdDirectBuildCallbackPendingRuntimeSource80055F7A(window);
    CHECK(!source.sourceAvailable);
    PrStage1XaCdDirectCallbackPendingRuntimeSourceResult80055F7A result =
        PrStage1XaCdDirectPublishCallbackPendingRuntimeSource80055F7A(source,
                                                                     state);
    CHECK(!result.publishAttempted);
    CHECK(result.blocker ==
          PrStage1XaCdDirectCallbackPendingRuntimeSourceBlocker80055F7A::
              SourceUnavailable);
    CHECK(state.word_80055F7A == 0x1234u);
    CHECK(state.callbackPendingWord80055F7AObservationAcceptedCount == 3u);

    FakeRuntimePsxMemoryProvider80055F7A fake{};
    fake.readable = false;
    provider.installed = true;
    provider.read = FakeReadCallbackPending80055F7A;
    provider.userData = &fake;
    provider.frameKnown = true;
    provider.frame = 4438u;
    provider.pcKnown = true;
    provider.pc = PrMovieSegmentDirect::kSub80035898CheckCallback;
    window =
        PrStage1XaCdDirectReadCallbackPendingRuntimeSourceWindow80055F7A(
            provider);
    CHECK(window.providerInstalled);
    CHECK(window.readAttempted);
    CHECK(!window.windowReadable);
    CHECK(fake.called);
    CHECK(fake.requestedAddress == 0x80055F7Au);
    CHECK(fake.requestedByteSize == 2u);
    source =
        PrStage1XaCdDirectBuildCallbackPendingRuntimeSource80055F7A(window);
    result =
        PrStage1XaCdDirectPublishCallbackPendingRuntimeSource80055F7A(source,
                                                                     state);
    CHECK(!result.publishAttempted);
    CHECK(result.blocker ==
          PrStage1XaCdDirectCallbackPendingRuntimeSourceBlocker80055F7A::
              SourceUnavailable);
    CHECK(state.word_80055F7A == 0x1234u);
    CHECK(state.callbackPendingWord80055F7AObservationAcceptedCount == 3u);

    fake.called = false;
    fake.readable = true;
    fake.value = 0u;
    window =
        PrStage1XaCdDirectReadCallbackPendingRuntimeSourceWindow80055F7A(
            provider);
    CHECK(window.windowReadable);
    CHECK(window.valueKnown);
    CHECK(window.value == 0u);
    source =
        PrStage1XaCdDirectBuildCallbackPendingRuntimeSource80055F7A(window);
    CHECK(source.sourceAvailable);
    CHECK(source.valueKnown);
    CHECK(source.observation.psxAddress == 0x80055F7Au);
    CHECK(source.observation.byteSize == 2u);
    result =
        PrStage1XaCdDirectPublishCallbackPendingRuntimeSource80055F7A(source,
                                                                     state);
    CHECK(result.publishAttempted);
    CHECK(result.observation.accepted);
    CHECK(state.word_80055F7AKnown);
    CHECK(state.word_80055F7A == 0u);
    CHECK(state.callbackPendingWord80055F7AObservationPcKnown);
    CHECK(state.callbackPendingWord80055F7AObservationPc ==
          PrMovieSegmentDirect::kSub80035898CheckCallback);
    CHECK(state.callbackPendingWord80055F7AObservationAcceptedCount == 4u);

    PrMovieSegmentDirect::CheckCallbackInput80035898 input{};
    input.word80055F7AKnown = state.word_80055F7AKnown;
    input.word80055F7A = state.word_80055F7A;
    const PrMovieSegmentDirect::CheckCallbackResult80035898 idle =
        PrMovieSegmentDirect::PsxCall80035898_CheckCallback(input);
    CHECK(idle.pendingKnown);
    CHECK(!idle.pending);
}

void TestCommandWaitLoopRuntimeSourceFeeds800375BCOnlyWhenKnown() {
    PrStage1XaCdDirectCommandWaitLoopRuntimeSource800375BC source{};

    PrStage1XaCdDirectCommandWaitLoopRuntimeSourceResult800375BC result =
        PrStage1XaCdDirectResolveCommandWaitLoopRuntimeSource800375BC(source);
    CHECK(!result.sourceAvailable);
    CHECK(!result.resultKnown);
    CHECK(!result.feedsCommand);
    CHECK(result.blocker ==
          PrStage1XaCdDirectCommandWaitLoopRuntimeSourceBlocker800375BC::
              SourceUnavailable);

    source.resultKnown = true;
    source.psxReturn = 0;
    result =
        PrStage1XaCdDirectResolveCommandWaitLoopRuntimeSource800375BC(source);
    CHECK(!result.sourceAvailable);
    CHECK(result.resultKnown);
    CHECK(!result.feedsCommand);
    CHECK(result.blocker ==
          PrStage1XaCdDirectCommandWaitLoopRuntimeSourceBlocker800375BC::
              SourceUnavailable);

    source.sourceAvailable = true;
    source.resultKnown = false;
    result =
        PrStage1XaCdDirectResolveCommandWaitLoopRuntimeSource800375BC(source);
    CHECK(result.sourceAvailable);
    CHECK(!result.resultKnown);
    CHECK(!result.feedsCommand);
    CHECK(result.blocker ==
          PrStage1XaCdDirectCommandWaitLoopRuntimeSourceBlocker800375BC::
              ResultUnknown);

    source.resultKnown = true;
    source.psxReturn = 0;
    result =
        PrStage1XaCdDirectResolveCommandWaitLoopRuntimeSource800375BC(source);
    CHECK(result.sourceAvailable);
    CHECK(result.resultKnown);
    CHECK(result.feedsCommand);
    CHECK(result.psxReturn == 0);
    CHECK(result.blocker ==
          PrStage1XaCdDirectCommandWaitLoopRuntimeSourceBlocker800375BC::None);

    uint8_t arg = 0xA0u;
    PrStage1LowerCdProducerDirect::CommandInput800375BC command =
        PrStage1LowerCdProducerDirect::BuildCommandInput800375BC(
            0x0Eu,
            true,
            1u,
            &arg,
            true,
            0,
            false,
            true,
            1513,
            true,
            false,
            true,
            false,
            false,
            0u,
            false,
            0u,
            false,
            0,
            false,
            nullptr,
            nullptr,
            false,
            result.feedsCommand,
            result.psxReturn);
    PrStage1LowerCdProducerDirect::CommandResult800375BC commandResult =
        PrStage1LowerCdProducerDirect::BuildCommandResult800375BC(command);
    CHECK(commandResult.produced);
    CHECK(!commandResult.incomplete);
    CHECK(commandResult.psxReturn == 0);

    source.sourceAvailable = false;
    result =
        PrStage1XaCdDirectResolveCommandWaitLoopRuntimeSource800375BC(source);
    command = PrStage1LowerCdProducerDirect::BuildCommandInput800375BC(
        0x0Eu,
        true,
        1u,
        &arg,
        true,
        0,
        false,
        true,
        1513,
        true,
        false,
        true,
        false,
        false,
        0u,
        false,
        0u,
        false,
        0,
        false,
        nullptr,
        nullptr,
        false,
        result.feedsCommand,
        result.psxReturn);
    commandResult =
        PrStage1LowerCdProducerDirect::BuildCommandResult800375BC(command);
    CHECK(!commandResult.produced);
    CHECK(commandResult.incomplete);
    CHECK(!command.waitLoopResultKnown);

    PrStage1XaCdDirectRuntimePsxMemoryProvider provider{};
    PrStage1XaCdDirectCommandWaitLoopRuntimeSourceWindow800375BC window =
        PrStage1XaCdDirectReadCommandWaitLoopRuntimeSourceWindow800375BC(
            provider,
            true,
            2001);
    CHECK(!window.providerInstalled);
    CHECK(!window.deadlineReadAttempted);
    CHECK(!window.spinReadAttempted);
    source =
        PrStage1XaCdDirectBuildCommandWaitLoopRuntimeSource800375BC(window);
    result =
        PrStage1XaCdDirectResolveCommandWaitLoopRuntimeSource800375BC(source);
    CHECK(!result.sourceAvailable);
    CHECK(!result.feedsCommand);

    FakeRuntimePsxMemoryProvider800375BC fake{};
    provider.installed = true;
    provider.read = FakeReadCommandWaitLoop800375BC;
    provider.userData = &fake;
    provider.frameKnown = true;
    provider.frame = 4438u;
    provider.pcKnown = true;
    provider.pc = PrMovieSegmentDirect::kSub800375BCCdCommand;
    window =
        PrStage1XaCdDirectReadCommandWaitLoopRuntimeSourceWindow800375BC(
            provider,
            false,
            0);
    CHECK(window.deadlineReadAttempted);
    CHECK(window.spinReadAttempted);
    CHECK(window.deadlineReadable);
    CHECK(window.spinReadable);
    CHECK(fake.callCount == 2u);
    CHECK(fake.requestedAddress[0] == 0x80088310u);
    CHECK(fake.requestedByteSize[0] == 4u);
    CHECK(fake.requestedAddress[1] == 0x80088314u);
    CHECK(fake.requestedByteSize[1] == 4u);
    source =
        PrStage1XaCdDirectBuildCommandWaitLoopRuntimeSource800375BC(window);
    result =
        PrStage1XaCdDirectResolveCommandWaitLoopRuntimeSource800375BC(source);
    CHECK(result.sourceAvailable);
    CHECK(!result.resultKnown);
    CHECK(!result.feedsCommand);
    CHECK(result.blocker ==
          PrStage1XaCdDirectCommandWaitLoopRuntimeSourceBlocker800375BC::
              ResultUnknown);

    fake = {};
    fake.deadlineClock = 3000u;
    fake.spinCount = 0x003C0000u;
    window =
        PrStage1XaCdDirectReadCommandWaitLoopRuntimeSourceWindow800375BC(
            provider,
            true,
            2000);
    source =
        PrStage1XaCdDirectBuildCommandWaitLoopRuntimeSource800375BC(window);
    result =
        PrStage1XaCdDirectResolveCommandWaitLoopRuntimeSource800375BC(source);
    CHECK(result.sourceAvailable);
    CHECK(!result.resultKnown);
    CHECK(!result.feedsCommand);

    fake = {};
    fake.deadlineClock = 1999u;
    fake.spinCount = 0u;
    window =
        PrStage1XaCdDirectReadCommandWaitLoopRuntimeSourceWindow800375BC(
            provider,
            true,
            2000);
    CHECK(window.resultKnown);
    CHECK(window.psxReturn == -1);
    CHECK(window.frameKnown);
    CHECK(window.frame == 4438u);
    CHECK(window.pcKnown);
    CHECK(window.pc == PrMovieSegmentDirect::kSub800375BCCdCommand);
    source =
        PrStage1XaCdDirectBuildCommandWaitLoopRuntimeSource800375BC(window);
    result =
        PrStage1XaCdDirectResolveCommandWaitLoopRuntimeSource800375BC(source);
    CHECK(result.sourceAvailable);
    CHECK(result.resultKnown);
    CHECK(result.feedsCommand);
    CHECK(result.psxReturn == -1);

    command = PrStage1LowerCdProducerDirect::BuildCommandInput800375BC(
        0x0Eu,
        true,
        1u,
        &arg,
        true,
        0,
        false,
        true,
        2000,
        true,
        false,
        true,
        false,
        false,
        0u,
        false,
        0u,
        false,
        0,
        false,
        nullptr,
        nullptr,
        false,
        result.feedsCommand,
        result.psxReturn);
    commandResult =
        PrStage1LowerCdProducerDirect::BuildCommandResult800375BC(command);
    CHECK(command.waitLoopResultKnown);
    CHECK(commandResult.produced);
    CHECK(!commandResult.incomplete);
    CHECK(commandResult.psxReturn == -1);

    fake = {};
    fake.deadlineClock = 3000u;
    fake.spinCount = 0x003C0001u;
    window =
        PrStage1XaCdDirectReadCommandWaitLoopRuntimeSourceWindow800375BC(
            provider,
            true,
            2000);
    source =
        PrStage1XaCdDirectBuildCommandWaitLoopRuntimeSource800375BC(window);
    result =
        PrStage1XaCdDirectResolveCommandWaitLoopRuntimeSource800375BC(source);
    CHECK(result.sourceAvailable);
    CHECK(result.resultKnown);
    CHECK(result.feedsCommand);
    CHECK(result.psxReturn == -1);

    fake = {};
    fake.spinReadable = false;
    window =
        PrStage1XaCdDirectReadCommandWaitLoopRuntimeSourceWindow800375BC(
            provider,
            true,
            2000);
    source =
        PrStage1XaCdDirectBuildCommandWaitLoopRuntimeSource800375BC(window);
    result =
        PrStage1XaCdDirectResolveCommandWaitLoopRuntimeSource800375BC(source);
    CHECK(!window.spinReadable);
    CHECK(!result.sourceAvailable);
    CHECK(!result.feedsCommand);
}

} // namespace

int main() {
    TestRejectsNonExactObservationShape();
    TestAcceptedObservationPublishesOnlyStatusFlagsAuthority();
    TestRejectedObservationDoesNotOverwriteAcceptedStatusFlags();
    TestSourceAdapterFailsClosedBeforeObservationGate();
    TestRuntimeSourceBuilderRequiresExactWindow();
    TestRuntimeProviderReadsExactStatusWindowOnly();
    TestLoaderHeapProviderFailsClosedForStatus57108();
    TestKnownStateProviderReadsOnlyKnownStatus57108();
    TestRuntimeProviderReadsExactPriorStatusCounter57110Only();
    TestSourceAdapterDelegatesOnlyCompleteRuntimeObservation();
    TestStatusFlagsDoNotCompleteModeCommandByThemselves();
    Test800375BCCommandPreservesStatusFlagsUntilResponse();
    Test800375BCDirectApplyRequiresExplicitWaitLoopResult();
    TestStatusFlagsAndModeCommandStillNeedReadPumpStartFacts();
    Test80036AF8PublishesStatusFlagsThroughObservationGate();
    TestCallbackPendingWordRequiresExactRuntimeObservation();
    TestCallbackPendingWordObservationFeedsCheckCallback();
    TestCallbackPendingRuntimeSourceAdapterFeedsCheckCallbackOnlyWhenExactWindow();
    TestCommandWaitLoopRuntimeSourceFeeds800375BCOnlyWhenKnown();
    if (g_failedChecks != 0) {
        std::printf(
            "test_ss0_stage1_xacd_status_flags_observation_contract: failed checks=%d\n",
            g_failedChecks);
        return 1;
    }
    std::printf(
        "test_ss0_stage1_xacd_status_flags_observation_contract: ok\n");
    return 0;
}
