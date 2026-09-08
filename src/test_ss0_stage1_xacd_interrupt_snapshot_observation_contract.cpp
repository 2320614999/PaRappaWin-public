#include "pr/pr_stage1_loader_memory_direct.h"
#include "pr/pr_stage1_xa_cd_direct.h"

#include <algorithm>
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

void WriteU16LE(uint8_t* p, uint16_t value) {
    p[0] = static_cast<uint8_t>(value & 0x00FFu);
    p[1] = static_cast<uint8_t>((value >> 8) & 0x00FFu);
}

void WriteU32LE(uint8_t* p, uint32_t value) {
    p[0] = static_cast<uint8_t>(value & 0x000000FFu);
    p[1] = static_cast<uint8_t>((value >> 8) & 0x000000FFu);
    p[2] = static_cast<uint8_t>((value >> 16) & 0x000000FFu);
    p[3] = static_cast<uint8_t>((value >> 24) & 0x000000FFu);
}

struct SnapshotBytes {
    std::array<uint8_t, 0x34> callbackState{};
    std::array<uint8_t, 8> initialRegs{};
    std::array<uint8_t, 8> terminalRegs{};
    std::array<uint8_t, 4> watchdog{};
};

struct FakeRuntimePsxMemoryProvider800359B8 {
    SnapshotBytes bytes{};
    bool callbackReadable = true;
    bool initialReadable = true;
    bool terminalReadable = true;
    bool watchdogReadable = true;
    uint32_t callbackReadCount = 0u;
    uint32_t interruptRegsReadCount = 0u;
    uint32_t watchdogReadCount = 0u;
};

bool FakeReadInterruptSnapshot800359B8(void* userData,
                                       uint32_t psxAddress,
                                       uint32_t byteSize,
                                       uint8_t* outBytes,
                                       size_t outSize) {
    auto& fake =
        *static_cast<FakeRuntimePsxMemoryProvider800359B8*>(userData);
    if (outBytes == nullptr || outSize < byteSize) {
        return false;
    }
    if (psxAddress == 0x80055F78u && byteSize == 0x34u) {
        ++fake.callbackReadCount;
        if (!fake.callbackReadable) {
            return false;
        }
        std::copy(fake.bytes.callbackState.begin(),
                  fake.bytes.callbackState.end(),
                  outBytes);
        return true;
    }
    if (psxAddress == 0x1F801070u && byteSize == 8u) {
        ++fake.interruptRegsReadCount;
        const bool initialRead = fake.interruptRegsReadCount == 1u;
        const auto& bytes =
            initialRead ? fake.bytes.initialRegs : fake.bytes.terminalRegs;
        if ((initialRead && !fake.initialReadable) ||
            (!initialRead && !fake.terminalReadable)) {
            return false;
        }
        std::copy(bytes.begin(), bytes.end(), outBytes);
        return true;
    }
    if (psxAddress == 0x80057010u && byteSize == 4u) {
        ++fake.watchdogReadCount;
        if (!fake.watchdogReadable) {
            return false;
        }
        std::copy(fake.bytes.watchdog.begin(),
                  fake.bytes.watchdog.end(),
                  outBytes);
        return true;
    }
    return false;
}

PrStage1XaCdDirectInterruptSnapshotObservation800359B8 Observation(
    uint32_t psxAddress,
    uint32_t byteSize,
    const uint8_t* bytes,
    size_t bytesSize) {
    PrStage1XaCdDirectInterruptSnapshotObservation800359B8 observation{};
    observation.source =
        PrStage1XaCdDirectInterruptSnapshotObservationSource800359B8::
            RuntimePsxMemoryObservation;
    observation.psxAddress = psxAddress;
    observation.byteSize = byteSize;
    observation.valueKnown = true;
    observation.bytes = bytes;
    observation.bytesSize = bytesSize;
    observation.frameKnown = true;
    observation.frame = 4438u;
    observation.pcKnown = true;
    observation.pc = 0x800359B8u;
    return observation;
}

PrStage1XaCdDirectInterruptSnapshotObservationBundle800359B8 Bundle(
    const SnapshotBytes& bytes) {
    PrStage1XaCdDirectInterruptSnapshotObservationBundle800359B8 bundle{};
    bundle.callbackState80055F78 =
        Observation(0x80055F78u,
                    static_cast<uint32_t>(bytes.callbackState.size()),
                    bytes.callbackState.data(),
                    bytes.callbackState.size());
    bundle.initialInterruptRegs1F801070 =
        Observation(0x1F801070u,
                    static_cast<uint32_t>(bytes.initialRegs.size()),
                    bytes.initialRegs.data(),
                    bytes.initialRegs.size());
    bundle.terminalInterruptRegs1F801070 =
        Observation(0x1F801070u,
                    static_cast<uint32_t>(bytes.terminalRegs.size()),
                    bytes.terminalRegs.data(),
                    bytes.terminalRegs.size());
    bundle.watchdog80057010 =
        Observation(0x80057010u,
                    static_cast<uint32_t>(bytes.watchdog.size()),
                    bytes.watchdog.data(),
                    bytes.watchdog.size());
    return bundle;
}

SnapshotBytes ValidBytes() {
    SnapshotBytes bytes{};
    WriteU16LE(bytes.callbackState.data(), 1u);
    WriteU16LE(bytes.callbackState.data() + 2, 0u);
    WriteU32LE(bytes.callbackState.data() + 4u + 2u * sizeof(uint32_t),
               0x80036AF8u);
    WriteU16LE(bytes.callbackState.data() + 0x30u, 0xFFFFu);
    WriteU16LE(bytes.initialRegs.data(), 0x0004u);
    WriteU16LE(bytes.initialRegs.data() + sizeof(uint32_t), 0xFFFFu);
    WriteU16LE(bytes.terminalRegs.data(), 0x0000u);
    WriteU16LE(bytes.terminalRegs.data() + sizeof(uint32_t), 0xFFFFu);
    WriteU32LE(bytes.watchdog.data(), 0u);
    return bytes;
}

void CheckStateStillClosed(const PrStage1XaCdDirectState& state) {
    CHECK(!state.word_80055F78Known);
    CHECK(!state.word_80055F7AKnown);
    CHECK(!state.word_80055FA8Known);
    CHECK(!state.dword_80057010Known);
    CHECK(state.cdCallbackPending800359B8WriteCount == 0u);
    CHECK(state.cdCallbackPending800359B8AckCount == 0u);
    CHECK(state.cdCallbackPending800359B8CallbackDispatchCount == 0u);
    CHECK(state.interruptSnapshot800359B8ObservationAcceptedCount == 0u);
    CHECK(state.lowerCdSnapshotPendingProducerCount == 0u);
    CHECK(state.cdLowerEvent80036AF8Serial == 0u);
}

void CheckLastWindowReadiness(
    const PrStage1XaCdDirectState& state,
    bool callbackStateKnown,
    bool initialRegsKnown,
    bool terminalRegsKnown,
    bool watchdogKnown,
    bool bundleKnown) {
    CHECK(state.interruptSnapshot800359B8LastCallbackStateKnown ==
          callbackStateKnown);
    CHECK(state.interruptSnapshot800359B8LastInitialRegsKnown ==
          initialRegsKnown);
    CHECK(state.interruptSnapshot800359B8LastTerminalRegsKnown ==
          terminalRegsKnown);
    CHECK(state.interruptSnapshot800359B8LastWatchdogKnown == watchdogKnown);
    CHECK(state.interruptSnapshot800359B8LastBundleKnown == bundleKnown);
}

void SeedCompleteCallbackStateWindow800359B8(
    PrStage1XaCdDirectState& state) {
    state.word_80055F78Known = true;
    state.word_80055F78 = 1u;
    state.word_80055F7AKnown = true;
    state.word_80055F7A = 0u;
    state.word_80055FA8Known = true;
    state.word_80055FA8 = 0xFFFFu;
    state.callbackStateTable80055F7CKnown = true;
    for (uint32_t i = 0u;
         i < PrMovieSegmentDirect::
                 kCdCallbackPendingProducerCallbackCount800359B8;
         ++i) {
        state.callbackStateTable80055F7CAddressKnown[i] = true;
        state.callbackStateTable80055F7CAddress[i] = 0u;
    }
    state.callbackStateTable80055F7CAddress[2] = 0x80036AF8u;
}

void TestNoAttemptHasNoWindowReadiness() {
    PrStage1XaCdDirectState state{};

    CheckLastWindowReadiness(state, false, false, false, false, false);
    CHECK(state.interruptSnapshot800359B8ObservationAcceptedCount == 0u);
    CHECK(state.interruptSnapshot800359B8ObservationRejectedCount == 0u);
    CHECK(state.interruptSnapshot800359B8ObservationLastReject == 0u);
}

void TestRuntimeSourceAdapterFailsClosedWithoutBundle() {
    {
        PrStage1XaCdDirectState state{};
        PrStage1XaCdDirectInterruptSnapshotRuntimeSource800359B8 source{};

        const auto result =
            PrStage1XaCdDirectPublishInterruptSnapshotRuntimeSource800359B8(
                source,
                state);

        CHECK(!result.sourceAvailable);
        CHECK(!result.bundleKnown);
        CHECK(!result.publishAttempted);
        CheckLastWindowReadiness(state, false, false, false, false, false);
        CheckStateStillClosed(state);
        CHECK(state.interruptSnapshot800359B8ObservationRejectedCount == 0u);
    }

    {
        PrStage1XaCdDirectState state{};
        PrStage1XaCdDirectInterruptSnapshotRuntimeSource800359B8 source{};
        source.sourceAvailable = true;

        const auto result =
            PrStage1XaCdDirectPublishInterruptSnapshotRuntimeSource800359B8(
                source,
                state);

        CHECK(result.sourceAvailable);
        CHECK(!result.bundleKnown);
        CHECK(!result.publishAttempted);
        CheckLastWindowReadiness(state, false, false, false, false, false);
        CheckStateStillClosed(state);
        CHECK(state.interruptSnapshot800359B8ObservationRejectedCount == 0u);
    }
}

void TestRejectsPartialOrNonRuntimeWindows() {
    const SnapshotBytes bytes = ValidBytes();

    {
        PrStage1XaCdDirectState state{};
        auto bundle = Bundle(bytes);
        bundle.callbackState80055F78.source =
            PrStage1XaCdDirectInterruptSnapshotObservationSource800359B8::
                RuntimePsxConsumerReadObservation;
        const auto result =
            PrStage1XaCdDirectApplyInterruptSnapshotRuntimeObservation800359B8(
                bundle,
                state);
        CHECK(!result.accepted);
        CHECK(result.rejectReason ==
              PrStage1XaCdDirectInterruptSnapshotObservationRejectReason800359B8::
                  NonRuntimePsxMemoryObservation);
        CHECK(state.interruptSnapshot800359B8ObservationRejectedCount == 1u);
        CHECK(state.interruptSnapshot800359B8ObservationLastReject ==
              static_cast<uint8_t>(
                  PrStage1XaCdDirectInterruptSnapshotObservationRejectReason800359B8::
                      NonRuntimePsxMemoryObservation));
        CheckLastWindowReadiness(state, false, true, true, true, false);
        CheckStateStillClosed(state);
    }

    {
        PrStage1XaCdDirectState state{};
        auto bundle = Bundle(bytes);
        bundle.initialInterruptRegs1F801070.psxAddress = 0x1F801074u;
        const auto result =
            PrStage1XaCdDirectApplyInterruptSnapshotRuntimeObservation800359B8(
                bundle,
                state);
        CHECK(!result.accepted);
        CHECK(result.rejectReason ==
              PrStage1XaCdDirectInterruptSnapshotObservationRejectReason800359B8::
                  WrongPsxAddress);
        CHECK(state.interruptSnapshot800359B8ObservationRejectedCount == 1u);
        CHECK(state.interruptSnapshot800359B8ObservationLastReject ==
              static_cast<uint8_t>(
                  PrStage1XaCdDirectInterruptSnapshotObservationRejectReason800359B8::
                      WrongPsxAddress));
        CheckLastWindowReadiness(state, true, false, true, true, false);
        CheckStateStillClosed(state);
    }

    {
        PrStage1XaCdDirectState state{};
        auto bundle = Bundle(bytes);
        bundle.watchdog80057010.valueKnown = false;
        const auto result =
            PrStage1XaCdDirectApplyInterruptSnapshotRuntimeObservation800359B8(
                bundle,
                state);
        CHECK(!result.accepted);
        CHECK(result.rejectReason ==
              PrStage1XaCdDirectInterruptSnapshotObservationRejectReason800359B8::
                  UnknownValue);
        CHECK(state.interruptSnapshot800359B8ObservationRejectedCount == 1u);
        CHECK(state.interruptSnapshot800359B8ObservationLastReject ==
              static_cast<uint8_t>(
                  PrStage1XaCdDirectInterruptSnapshotObservationRejectReason800359B8::
                      UnknownValue));
        CheckLastWindowReadiness(state, true, true, true, false, false);
        CheckStateStillClosed(state);
    }
}

void TestRejectsIncompletePendingSampleSequence() {
    SnapshotBytes bytes = ValidBytes();
    WriteU16LE(bytes.terminalRegs.data(), 0x0004u);
    PrStage1XaCdDirectState state{};

    const auto result =
        PrStage1XaCdDirectApplyInterruptSnapshotRuntimeObservation800359B8(
            Bundle(bytes),
            state);

    CHECK(!result.accepted);
    CHECK(result.rejectReason ==
          PrStage1XaCdDirectInterruptSnapshotObservationRejectReason800359B8::
              IncompleteSnapshot);
    CHECK(state.interruptSnapshot800359B8ObservationRejectedCount == 1u);
    CHECK(state.interruptSnapshot800359B8ObservationLastReject ==
          static_cast<uint8_t>(
              PrStage1XaCdDirectInterruptSnapshotObservationRejectReason800359B8::
                  IncompleteSnapshot));
    CheckLastWindowReadiness(state, true, true, true, true, true);
    CheckStateStillClosed(state);
}

void TestAcceptedBundlePublishesOnlyPendingProducer() {
    const SnapshotBytes bytes = ValidBytes();
    PrStage1XaCdDirectState state{};

    const auto result =
        PrStage1XaCdDirectApplyInterruptSnapshotRuntimeObservation800359B8(
            Bundle(bytes),
            state);

    CHECK(result.accepted);
    CHECK(result.rejectReason ==
          PrStage1XaCdDirectInterruptSnapshotObservationRejectReason800359B8::
              None);
    CHECK(result.bridge.produced);
    CHECK(result.bridge.pendingProducer800359B8Bridged);
    CHECK(state.word_80055F78Known);
    CHECK(state.word_80055F78 == 1u);
    CHECK(state.word_80055F7AKnown);
    CHECK(state.word_80055F7A == 0u);
    CHECK(state.word_80055FA8Known);
    CHECK(state.word_80055FA8 == 0xFFFFu);
    CHECK(state.dword_80057010Known);
    CHECK(state.dword_80057010 == 0u);
    CHECK(state.cdCallbackPending800359B8InterruptStatusKnown);
    CHECK(state.cdCallbackPending800359B8InterruptStatus == 0x0004u);
    CHECK(state.cdCallbackPending800359B8InterruptMaskKnown);
    CHECK(state.cdCallbackPending800359B8InterruptMask == 0xFFFFu);
    CHECK(state.cdCallbackPending800359B8WriteCount == 2u);
    CHECK(state.cdCallbackPending800359B8AckCount == 1u);
    CHECK(state.cdCallbackPending800359B8CallbackDispatchCount == 1u);
    CHECK(state.interruptSnapshot800359B8ObservationAcceptedCount == 1u);
    CHECK(state.interruptSnapshot800359B8ObservationRejectedCount == 0u);
    CHECK(state.interruptSnapshot800359B8ObservationLastReject == 0u);
    CheckLastWindowReadiness(state, true, true, true, true, true);
    CHECK(state.lowerCdSnapshotPendingProducerCount == 1u);
    CHECK(state.lowerCdSnapshotCallbackEventCount == 1u);
    CHECK(state.cdLowerEvent80036AF8Serial == 0u);
    CHECK(!state.dword_80057108Known);
    CHECK(!state.byte_800573D4Known);
}

void TestRuntimeSourceAdapterPublishesOnlyKnownBundle() {
    const SnapshotBytes bytes = ValidBytes();
    PrStage1XaCdDirectState state{};
    PrStage1XaCdDirectInterruptSnapshotRuntimeSource800359B8 source{};
    source.sourceAvailable = true;
    source.bundleKnown = true;
    source.bundle = Bundle(bytes);

    const auto result =
        PrStage1XaCdDirectPublishInterruptSnapshotRuntimeSource800359B8(
            source,
            state);

    CHECK(result.sourceAvailable);
    CHECK(result.bundleKnown);
    CHECK(result.publishAttempted);
    CHECK(result.observation.accepted);
    CHECK(state.interruptSnapshot800359B8ObservationAcceptedCount == 1u);
    CHECK(state.interruptSnapshot800359B8ObservationRejectedCount == 0u);
    CHECK(state.cdCallbackPending800359B8WriteCount == 2u);
    CHECK(state.lowerCdSnapshotPendingProducerCount == 1u);
    CHECK(state.cdLowerEvent80036AF8Serial == 0u);
    CHECK(!state.dword_80057108Known);
    CHECK(!state.byte_800573D4Known);
}

void TestRuntimeProviderReadsOnlyExactBundleWindows() {
    FakeRuntimePsxMemoryProvider800359B8 fake{};
    fake.bytes = ValidBytes();
    PrStage1XaCdDirectRuntimePsxMemoryProvider provider{};
    provider.installed = false;
    provider.read = FakeReadInterruptSnapshot800359B8;
    provider.userData = &fake;
    provider.frameKnown = true;
    provider.frame = 4438u;
    provider.pcKnown = true;
    provider.pc = 0x800359B8u;

    auto window =
        PrStage1XaCdDirectReadInterruptSnapshotRuntimeSourceWindow800359B8(
            provider);
    CHECK(!window.providerInstalled);
    CHECK(!window.callbackStateReadAttempted);
    CHECK(!window.bundleReadable);
    CHECK(fake.callbackReadCount == 0u);

    provider.installed = true;
    provider.read = nullptr;
    window =
        PrStage1XaCdDirectReadInterruptSnapshotRuntimeSourceWindow800359B8(
            provider);
    CHECK(!window.providerInstalled);
    CHECK(!window.callbackStateReadAttempted);
    CHECK(!window.bundleReadable);
    CHECK(fake.callbackReadCount == 0u);

    provider.read = FakeReadInterruptSnapshot800359B8;
    fake.terminalReadable = false;
    window =
        PrStage1XaCdDirectReadInterruptSnapshotRuntimeSourceWindow800359B8(
            provider);
    CHECK(window.providerInstalled);
    CHECK(window.callbackStateReadAttempted);
    CHECK(window.callbackStateReadable);
    CHECK(window.initialRegsReadAttempted);
    CHECK(window.initialRegsReadable);
    CHECK(window.terminalRegsReadAttempted);
    CHECK(!window.terminalRegsReadable);
    CHECK(window.watchdogReadAttempted);
    CHECK(window.watchdogReadable);
    CHECK(!window.bundleReadable);
    auto source =
        PrStage1XaCdDirectBuildInterruptSnapshotRuntimeSource800359B8(window);
    CHECK(source.sourceAvailable);
    CHECK(!source.bundleKnown);
    PrStage1XaCdDirectState state{};
    auto result =
        PrStage1XaCdDirectPublishInterruptSnapshotRuntimeSource800359B8(
            source,
            state);
    CHECK(result.sourceAvailable);
    CHECK(!result.bundleKnown);
    CHECK(!result.publishAttempted);
    CheckStateStillClosed(state);

    fake = FakeRuntimePsxMemoryProvider800359B8{};
    fake.bytes = ValidBytes();
    window =
        PrStage1XaCdDirectReadInterruptSnapshotRuntimeSourceWindow800359B8(
            provider);
    CHECK(window.providerInstalled);
    CHECK(window.callbackStateReadable);
    CHECK(window.initialRegsReadable);
    CHECK(window.terminalRegsReadable);
    CHECK(window.watchdogReadable);
    CHECK(window.bundleReadable);
    CHECK(fake.callbackReadCount == 1u);
    CHECK(fake.interruptRegsReadCount == 2u);
    CHECK(fake.watchdogReadCount == 1u);

    source =
        PrStage1XaCdDirectBuildInterruptSnapshotRuntimeSource800359B8(window);
    CHECK(source.sourceAvailable);
    CHECK(source.bundleKnown);
    CHECK(source.bundle.callbackState80055F78.psxAddress == 0x80055F78u);
    CHECK(source.bundle.callbackState80055F78.byteSize == 0x34u);
    CHECK(source.bundle.initialInterruptRegs1F801070.psxAddress ==
          0x1F801070u);
    CHECK(source.bundle.initialInterruptRegs1F801070.byteSize == 8u);
    CHECK(source.bundle.terminalInterruptRegs1F801070.psxAddress ==
          0x1F801070u);
    CHECK(source.bundle.terminalInterruptRegs1F801070.byteSize == 8u);
    CHECK(source.bundle.watchdog80057010.psxAddress == 0x80057010u);
    CHECK(source.bundle.watchdog80057010.byteSize == 4u);
    CHECK(source.bundle.callbackState80055F78.frameKnown);
    CHECK(source.bundle.callbackState80055F78.frame == 4438u);
    CHECK(source.bundle.callbackState80055F78.pcKnown);
    CHECK(source.bundle.callbackState80055F78.pc == 0x800359B8u);

    state = PrStage1XaCdDirectState{};
    result =
        PrStage1XaCdDirectPublishInterruptSnapshotRuntimeSource800359B8(
            source,
            state);
    CHECK(result.sourceAvailable);
    CHECK(result.bundleKnown);
    CHECK(result.publishAttempted);
    CHECK(result.observation.accepted);
    CHECK(state.interruptSnapshot800359B8ObservationAcceptedCount == 1u);
    CHECK(state.lowerCdSnapshotPendingProducerCount == 1u);
    CHECK(state.cdLowerEvent80036AF8Serial == 0u);
    CHECK(!state.dword_80057108Known);
}

void TestLoaderHeapProviderFailsClosedForInterruptSnapshotWindows() {
    static PrStage1LoaderMemoryDirectState loaderMemory{};
    PrStage1XaCdDirectRuntimePsxMemoryProvider provider{};
    provider.installed = true;
    provider.read = PrStage1LoaderMemoryDirectReadRuntimePsxMemory;
    provider.userData = &loaderMemory;
    provider.frameKnown = true;
    provider.frame = 4438u;

    const auto window =
        PrStage1XaCdDirectReadInterruptSnapshotRuntimeSourceWindow800359B8(
            provider);
    CHECK(window.providerInstalled);
    CHECK(window.callbackStateReadAttempted);
    CHECK(window.initialRegsReadAttempted);
    CHECK(window.terminalRegsReadAttempted);
    CHECK(window.watchdogReadAttempted);
    CHECK(!window.callbackStateReadable);
    CHECK(!window.initialRegsReadable);
    CHECK(!window.terminalRegsReadable);
    CHECK(!window.watchdogReadable);
    CHECK(!window.bundleReadable);

    PrStage1XaCdDirectState state{};
    const auto source =
        PrStage1XaCdDirectBuildInterruptSnapshotRuntimeSource800359B8(window);
    CHECK(source.sourceAvailable);
    CHECK(!source.bundleKnown);
    const auto result =
        PrStage1XaCdDirectPublishInterruptSnapshotRuntimeSource800359B8(
            source,
            state);
    CHECK(result.sourceAvailable);
    CHECK(!result.bundleKnown);
    CHECK(!result.publishAttempted);
    CHECK(!result.observation.accepted);
    CHECK(state.interruptSnapshot800359B8ObservationAcceptedCount == 0u);
    CHECK(state.lowerCdSnapshotPendingProducerCount == 0u);
}

void TestKnownStateProviderDoesNotSynthesizeInterruptSnapshotBundle() {
    PrStage1XaCdDirectState sourceState{};
    sourceState.dword_80057010Known = true;
    sourceState.dword_80057010 = 0u;
    sourceState.dword_80057108Known = true;
    sourceState.dword_80057108 = 0x00000022u;
    sourceState.byte_80057119Known = true;
    sourceState.byte_80057119 = 0x0Eu;

    PrStage1XaCdDirectRuntimePsxMemoryProvider provider{};
    provider.installed = true;
    provider.read = PrStage1XaCdDirectReadKnownStateRuntimePsxMemory;
    provider.userData = &sourceState;
    provider.frameKnown = true;
    provider.frame = 4438u;

    const auto window =
        PrStage1XaCdDirectReadInterruptSnapshotRuntimeSourceWindow800359B8(
            provider);
    CHECK(window.providerInstalled);
    CHECK(window.callbackStateReadAttempted);
    CHECK(window.initialRegsReadAttempted);
    CHECK(window.terminalRegsReadAttempted);
    CHECK(window.watchdogReadAttempted);
    CHECK(!window.callbackStateReadable);
    CHECK(!window.initialRegsReadable);
    CHECK(!window.terminalRegsReadable);
    CHECK(window.watchdogReadable);
    CHECK(!window.bundleReadable);

    PrStage1XaCdDirectState targetState{};
    const auto source =
        PrStage1XaCdDirectBuildInterruptSnapshotRuntimeSource800359B8(window);
    CHECK(source.sourceAvailable);
    CHECK(!source.bundleKnown);
    const auto result =
        PrStage1XaCdDirectPublishInterruptSnapshotRuntimeSource800359B8(
            source,
            targetState);
    CHECK(result.sourceAvailable);
    CHECK(!result.bundleKnown);
    CHECK(!result.publishAttempted);
    CHECK(!result.observation.accepted);
    CHECK(targetState.interruptSnapshot800359B8ObservationAcceptedCount == 0u);
    CHECK(targetState.lowerCdSnapshotPendingProducerCount == 0u);
    CHECK(!targetState.dword_80057108Known);
}

void TestKnownStateProviderRelaysOnlyCompleteCallbackStateWindow() {
    PrStage1XaCdDirectState sourceState{};
    sourceState.dword_80057010Known = true;
    sourceState.dword_80057010 = 0u;
    SeedCompleteCallbackStateWindow800359B8(sourceState);
    sourceState.callbackStateTable80055F7CAddressKnown[3] = false;

    PrStage1XaCdDirectRuntimePsxMemoryProvider provider{};
    provider.installed = true;
    provider.read = PrStage1XaCdDirectReadKnownStateRuntimePsxMemory;
    provider.userData = &sourceState;
    provider.frameKnown = true;
    provider.frame = 4438u;

    auto window =
        PrStage1XaCdDirectReadInterruptSnapshotRuntimeSourceWindow800359B8(
            provider);
    CHECK(window.providerInstalled);
    CHECK(window.callbackStateReadAttempted);
    CHECK(!window.callbackStateReadable);
    CHECK(!window.initialRegsReadable);
    CHECK(!window.terminalRegsReadable);
    CHECK(window.watchdogReadable);
    CHECK(!window.bundleReadable);

    sourceState.callbackStateTable80055F7CAddressKnown[3] = true;
    window =
        PrStage1XaCdDirectReadInterruptSnapshotRuntimeSourceWindow800359B8(
            provider);
    CHECK(window.callbackStateReadable);
    CHECK(window.callbackState80055F78Bytes[0] == 1u);
    CHECK(window.callbackState80055F78Bytes[1] == 0u);
    CHECK(window.callbackState80055F78Bytes[2] == 0u);
    CHECK(window.callbackState80055F78Bytes[3] == 0u);
    CHECK(window.callbackState80055F78Bytes[12] == 0xF8u);
    CHECK(window.callbackState80055F78Bytes[13] == 0x6Au);
    CHECK(window.callbackState80055F78Bytes[14] == 0x03u);
    CHECK(window.callbackState80055F78Bytes[15] == 0x80u);
    CHECK(window.callbackState80055F78Bytes[0x30] == 0xFFu);
    CHECK(window.callbackState80055F78Bytes[0x31] == 0xFFu);
    CHECK(!window.initialRegsReadable);
    CHECK(!window.terminalRegsReadable);
    CHECK(window.watchdogReadable);
    CHECK(!window.bundleReadable);

    const auto source =
        PrStage1XaCdDirectBuildInterruptSnapshotRuntimeSource800359B8(window);
    CHECK(source.sourceAvailable);
    CHECK(!source.bundleKnown);
    PrStage1XaCdDirectState targetState{};
    const auto result =
        PrStage1XaCdDirectPublishInterruptSnapshotRuntimeSource800359B8(
            source,
            targetState);
    CHECK(result.sourceAvailable);
    CHECK(!result.bundleKnown);
    CHECK(!result.publishAttempted);
    CHECK(targetState.interruptSnapshot800359B8ObservationAcceptedCount == 0u);
    CHECK(targetState.lowerCdSnapshotPendingProducerCount == 0u);
}

void TestKnownStateProviderRelaysOnlyProducedCheckCallbackWord() {
    PrStage1XaCdDirectState state{};
    PrStage1XaCdDirectRuntimePsxMemoryProvider provider{};
    provider.installed = true;
    provider.read = PrStage1XaCdDirectReadKnownStateRuntimePsxMemory;
    provider.userData = &state;
    provider.frameKnown = true;
    provider.frame = 4438u;
    provider.pcKnown = true;
    provider.pc = 0x80035898u;

    uint8_t wordBytes[2]{0xFFu, 0xFFu};
    CHECK(!provider.read(provider.userData,
                         0x80055F7Au,
                         2u,
                         wordBytes,
                         sizeof(wordBytes)));
    CHECK(wordBytes[0] == 0xFFu);
    CHECK(wordBytes[1] == 0xFFu);

    const SnapshotBytes bytes = ValidBytes();
    PrStage1XaCdDirectInterruptSnapshotRuntimeSource800359B8 source{};
    source.sourceAvailable = true;
    source.bundleKnown = true;
    source.bundle = Bundle(bytes);
    const auto result =
        PrStage1XaCdDirectPublishInterruptSnapshotRuntimeSource800359B8(
            source,
            state);
    CHECK(result.publishAttempted);
    CHECK(result.observation.accepted);
    CHECK(state.word_80055F7AKnown);
    CHECK(state.word_80055F7A == 0u);

    wordBytes[0] = 0xFFu;
    wordBytes[1] = 0xFFu;
    CHECK(provider.read(provider.userData,
                        0x80055F7Au,
                        2u,
                        wordBytes,
                        sizeof(wordBytes)));
    CHECK(wordBytes[0] == 0u);
    CHECK(wordBytes[1] == 0u);
    CHECK(!provider.read(provider.userData,
                         0x80055F7Au,
                         4u,
                         wordBytes,
                         sizeof(wordBytes)));
}

} // namespace

int main() {
    TestNoAttemptHasNoWindowReadiness();
    TestRuntimeSourceAdapterFailsClosedWithoutBundle();
    TestRejectsPartialOrNonRuntimeWindows();
    TestRejectsIncompletePendingSampleSequence();
    TestAcceptedBundlePublishesOnlyPendingProducer();
    TestRuntimeSourceAdapterPublishesOnlyKnownBundle();
    TestRuntimeProviderReadsOnlyExactBundleWindows();
    TestLoaderHeapProviderFailsClosedForInterruptSnapshotWindows();
    TestKnownStateProviderDoesNotSynthesizeInterruptSnapshotBundle();
    TestKnownStateProviderRelaysOnlyCompleteCallbackStateWindow();
    TestKnownStateProviderRelaysOnlyProducedCheckCallbackWord();
    if (g_failedChecks != 0) {
        std::printf(
            "test_ss0_stage1_xacd_interrupt_snapshot_observation_contract: failed checks=%d\n",
            g_failedChecks);
        return 1;
    }
    std::printf(
        "test_ss0_stage1_xacd_interrupt_snapshot_observation_contract: ok\n");
    return 0;
}
