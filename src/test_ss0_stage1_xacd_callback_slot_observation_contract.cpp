#include "pr/pr_stage1_xa_cd_direct.h"

#include <cstdio>
#include <cstdint>

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

constexpr uint32_t kTestReadyCallback80039240 = 0x80039240u;
constexpr uint32_t kTestInterruptCallback80039240 = 0x80039240u;

void WriteU32LE(uint8_t* bytes, uint32_t value) {
    bytes[0] = static_cast<uint8_t>(value & 0xFFu);
    bytes[1] = static_cast<uint8_t>((value >> 8u) & 0xFFu);
    bytes[2] = static_cast<uint8_t>((value >> 16u) & 0xFFu);
    bytes[3] = static_cast<uint8_t>((value >> 24u) & 0xFFu);
}

PrStage1XaCdDirectCallbackSlotsObservation800570F8FC ValidObservation(
    const uint8_t* bytes,
    size_t size) {
    PrStage1XaCdDirectCallbackSlotsObservation800570F8FC observation{};
    observation.source =
        PrStage1XaCdDirectCallbackSlotsObservationSource::
            RuntimePsxMemoryObservation;
    observation.psxAddress = 0x800570F8u;
    observation.byteSize = 8u;
    observation.valueKnown = true;
    observation.bytes = bytes;
    observation.bytesSize = size;
    observation.frameKnown = true;
    observation.frame = 3600u;
    observation.pcKnown = true;
    observation.pc = 0x800356D0u;
    return observation;
}

void SeedSentinelState(PrStage1XaCdDirectState& state) {
    state.dword_800570F8Known = true;
    state.dword_800570F8 = 0x11111111u;
    state.streamClockCallback8001A210Registered = true;
    state.dword_800570FCKnown = true;
    state.dword_800570FC = 0x22222222u;
    state.callback80039240Installed = true;
    state.readyCallbackRegisterWriteCount = 7u;
    state.readyCallbackRegisterClearCount = 3u;
    state.readyCallbackRegisterInstall39240Count = 2u;
}

void CheckRejectLeavesStateUnchanged(
    PrStage1XaCdDirectCallbackSlotsObservation800570F8FC observation,
    PrStage1XaCdDirectCallbackSlotsObservationRejectReason expectedReason) {
    PrStage1XaCdDirectState state{};
    SeedSentinelState(state);

    const PrStage1XaCdDirectCallbackSlotsObservationResult result =
        PrStage1XaCdDirectApplyCallbackSlotsRuntimeObservation800570F8FC(
            observation,
            state);

    CHECK(!result.accepted);
    CHECK(result.rejectReason == expectedReason);
    CHECK(state.dword_800570F8Known);
    CHECK(state.dword_800570F8 == 0x11111111u);
    CHECK(state.streamClockCallback8001A210Registered);
    CHECK(state.dword_800570FCKnown);
    CHECK(state.dword_800570FC == 0x22222222u);
    CHECK(state.callback80039240Installed);
    CHECK(state.readyCallbackRegisterWriteCount == 7u);
    CHECK(state.readyCallbackRegisterClearCount == 3u);
    CHECK(state.readyCallbackRegisterInstall39240Count == 2u);
}

void TestRejectsNonExactObservationShape() {
    uint8_t bytes[8]{};
    PrStage1XaCdDirectCallbackSlotsObservation800570F8FC observation =
        ValidObservation(bytes, sizeof(bytes));
    observation.source =
        PrStage1XaCdDirectCallbackSlotsObservationSource::Unknown;
    CheckRejectLeavesStateUnchanged(
        observation,
        PrStage1XaCdDirectCallbackSlotsObservationRejectReason::
            NonRuntimePsxMemoryObservation);

    observation = ValidObservation(bytes, sizeof(bytes));
    observation.source =
        PrStage1XaCdDirectCallbackSlotsObservationSource::
            RuntimePsxConsumerReadObservation;
    CheckRejectLeavesStateUnchanged(
        observation,
        PrStage1XaCdDirectCallbackSlotsObservationRejectReason::
            NonRuntimePsxMemoryObservation);

    observation = ValidObservation(bytes, sizeof(bytes));
    observation.psxAddress = 0x800570FCu;
    CheckRejectLeavesStateUnchanged(
        observation,
        PrStage1XaCdDirectCallbackSlotsObservationRejectReason::
            WrongPsxAddress);

    observation = ValidObservation(bytes, sizeof(bytes));
    observation.byteSize = 4u;
    CheckRejectLeavesStateUnchanged(
        observation,
        PrStage1XaCdDirectCallbackSlotsObservationRejectReason::
            WrongByteSize);

    observation = ValidObservation(bytes, sizeof(bytes));
    observation.valueKnown = false;
    CheckRejectLeavesStateUnchanged(
        observation,
        PrStage1XaCdDirectCallbackSlotsObservationRejectReason::UnknownValue);

    observation = ValidObservation(bytes, sizeof(bytes));
    observation.bytes = nullptr;
    observation.bytesSize = 0u;
    CheckRejectLeavesStateUnchanged(
        observation,
        PrStage1XaCdDirectCallbackSlotsObservationRejectReason::MissingBytes);
}

void TestAcceptedObservationPublishesOnlyCallbackSlotValues() {
    uint8_t bytes[8]{};
    WriteU32LE(&bytes[0], PrMovieSegmentDirect::kSub8001A210StreamClockCallback);
    WriteU32LE(&bytes[4], kTestReadyCallback80039240);
    PrStage1XaCdDirectState state{};

    const PrStage1XaCdDirectCallbackSlotsObservationResult result =
        PrStage1XaCdDirectApplyCallbackSlotsRuntimeObservation800570F8FC(
            ValidObservation(bytes, sizeof(bytes)),
            state);

    CHECK(result.accepted);
    CHECK(result.rejectReason ==
          PrStage1XaCdDirectCallbackSlotsObservationRejectReason::None);
    CHECK(state.dword_800570F8Known);
    CHECK(state.dword_800570F8 ==
          PrMovieSegmentDirect::kSub8001A210StreamClockCallback);
    CHECK(state.streamClockCallback8001A210Registered);
    CHECK(state.dword_800570FCKnown);
    CHECK(state.dword_800570FC == kTestReadyCallback80039240);
    CHECK(state.callback80039240Installed);
    CHECK(state.readyCallbackRegisterWriteCount == 0u);
    CHECK(state.readyCallbackRegisterClearCount == 0u);
    CHECK(state.readyCallbackRegisterInstall39240Count == 0u);
}

void TestDiscFullbootStartupObservationPublishesZeroSlotsOnly() {
    PrStage1XaCdDirectState state{};
    SeedSentinelState(state);
    state.readyCallbackRegisterWriteCount = 0u;
    state.readyCallbackRegisterClearCount = 0u;
    state.readyCallbackRegisterInstall39240Count = 0u;

    const PrStage1XaCdDirectCallbackSlotsObservationResult result =
        PrStage1XaCdDirectPublishCallbackSlotsDiscFullbootStartupObservation800570F8FC(
            state);

    CHECK(result.accepted);
    CHECK(state.dword_800570F8Known);
    CHECK(state.dword_800570F8 == 0u);
    CHECK(!state.streamClockCallback8001A210Registered);
    CHECK(state.dword_800570FCKnown);
    CHECK(state.dword_800570FC == 0u);
    CHECK(!state.callback80039240Installed);
    CHECK(state.readyCallbackRegisterWriteCount == 0u);
    CHECK(state.readyCallbackRegisterClearCount == 0u);
    CHECK(state.readyCallbackRegisterInstall39240Count == 0u);
}

void TestNaturalRegistersOverwriteStartupObservation() {
    PrStage1XaCdDirectState state{};
    PrStage1XaCdDirectPublishCallbackSlotsDiscFullbootStartupObservation800570F8FC(
        state);

    const PrStage1XaCdDirectCallbackRegisterResult80036510 sync =
        PrStage1XaCdDirectApplySub80036510SetCdCallback(
            state,
            PrMovieSegmentDirect::kSub8001A210StreamClockCallback);
    CHECK(sync.dword800570F8Known);
    CHECK(state.dword_800570F8 ==
          PrMovieSegmentDirect::kSub8001A210StreamClockCallback);
    CHECK(state.streamClockCallback8001A210Registered);

    const PrStage1XaCdDirectReadyCallbackRegisterResult80036528 ready =
        PrStage1XaCdDirectApplySub80036528SetCdReadyCallback(
            state,
            kTestReadyCallback80039240);
    CHECK(ready.dword800570FCKnown);
    CHECK(state.dword_800570FC == kTestReadyCallback80039240);
    CHECK(state.callback80039240Installed);
    CHECK(state.readyCallbackRegisterWriteCount == 1u);
    CHECK(state.readyCallbackRegisterInstall39240Count == 1u);
}

void TestInterruptCallbackTableSetterFailsClosed() {
    PrStage1XaCdDirectState state{};
    state.word_80055FA8Known = true;
    state.word_80055FA8 = 0u;

    PrStage1XaCdDirectCallbackStateTableSetResult80035BA0 result =
        PrStage1XaCdDirectApplySub80035BA0SetCdInterruptCallbackTable(
            state,
            2u,
            true,
            kTestInterruptCallback80039240);
    CHECK(!result.applied);
    CHECK(result.gapMissingWord80055F78);
    CHECK(state.callbackStateTable80035BA0GapCount == 1u);
    CHECK(!state.callbackStateTable80055F7CAddressKnown[2]);
    CHECK(state.word_80055FA8 == 0u);

    state.word_80055F78Known = true;
    state.word_80055F78 = 0u;
    result = PrStage1XaCdDirectApplySub80035BA0SetCdInterruptCallbackTable(
        state,
        2u,
        true,
        kTestInterruptCallback80039240);
    CHECK(!result.applied);
    CHECK(result.gateClosedWord80055F78);
    CHECK(state.callbackStateTable80035BA0GapCount == 2u);
    CHECK(!state.callbackStateTable80055F7CAddressKnown[2]);
    CHECK(state.word_80055FA8 == 0u);

    state.word_80055F78 = 1u;
    result = PrStage1XaCdDirectApplySub80035BA0SetCdInterruptCallbackTable(
        state,
        PrMovieSegmentDirect::kCdCallbackPendingProducerCallbackCount800359B8,
        true,
        kTestInterruptCallback80039240);
    CHECK(!result.applied);
    CHECK(result.gapInvalidCallbackIndex);
    CHECK(state.callbackStateTable80035BA0GapCount == 3u);

    result = PrStage1XaCdDirectApplySub80035BA0SetCdInterruptCallbackTable(
        state,
        2u,
        false,
        kTestInterruptCallback80039240);
    CHECK(!result.applied);
    CHECK(result.gapMissingCallbackAddr);
    CHECK(state.callbackStateTable80035BA0GapCount == 4u);
    CHECK(!state.callbackStateTable80055F7CAddressKnown[2]);
}

void SeedKnownCallbackStateWords(PrStage1XaCdDirectState& state) {
    state.word_80055F78Known = true;
    state.word_80055F78 = 1u;
    state.word_80055F7AKnown = true;
    state.word_80055F7A = 0u;
    state.word_80055FA8Known = true;
    state.word_80055FA8 = 0u;
}

void TestInterruptCallbackTableSetterUpdatesMaskAndTypedAudit() {
    PrStage1XaCdDirectState state{};
    SeedKnownCallbackStateWords(state);

    PrStage1XaCdDirectCallbackStateTableSetResult80035BA0 result =
        PrStage1XaCdDirectApplySub80035BA0SetCdInterruptCallbackTable(
            state,
            2u,
            true,
            kTestInterruptCallback80039240);
    CHECK(result.applied);
    CHECK(!result.callbackStateTableKnown);
    CHECK(state.callbackStateTable80035BA0ApplyCount == 1u);
    CHECK(state.callbackStateTable80055F7CAddressKnown[2]);
    CHECK(state.callbackStateTable80055F7CAddress[2] ==
          kTestInterruptCallback80039240);
    CHECK(state.word_80055FA8Known);
    CHECK((state.word_80055FA8 & (1u << 2u)) != 0u);

    PrStage1XaCdDirectInterruptSnapshotTypedSourceAudit800359B8 audit =
        PrStage1XaCdDirectAuditInterruptSnapshotTypedSource800359B8(state);
    CHECK(audit.callbackStateWordsKnown);
    CHECK(!audit.callbackStateTableKnown);
    CHECK((audit.missingMask &
           kPrStage1XaCdDirectTypedSourceGapCallbackStateTable800359B8) != 0u);
    CHECK(!audit.typedCanReconstructBundle);
    CHECK(!audit.canFeedRuntimeObservation);

    for (uint32_t i = 0u;
         i < PrMovieSegmentDirect::
                 kCdCallbackPendingProducerCallbackCount800359B8;
         ++i) {
        if (i == 2u) {
            continue;
        }
        result =
            PrStage1XaCdDirectApplySub80035BA0SetCdInterruptCallbackTable(
                state,
                i,
                true,
                0u);
        CHECK(result.applied);
    }
    CHECK(state.callbackStateTable80055F7CKnown);

    audit = PrStage1XaCdDirectAuditInterruptSnapshotTypedSource800359B8(state);
    CHECK(audit.callbackStateTableKnown);
    CHECK((audit.missingMask &
           kPrStage1XaCdDirectTypedSourceGapCallbackStateTable800359B8) == 0u);
    CHECK(!audit.initialInterruptRegsKnown);
    CHECK(!audit.terminalInterruptRegsKnown);
    CHECK(!audit.watchdogKnown);
    CHECK(!audit.typedCanReconstructBundle);
    CHECK(!audit.canFeedRuntimeObservation);

    result = PrStage1XaCdDirectApplySub80035BA0SetCdInterruptCallbackTable(
        state,
        2u,
        true,
        0u);
    CHECK(result.applied);
    CHECK(result.oldCallbackKnown);
    CHECK(result.oldCallback == kTestInterruptCallback80039240);
    CHECK((state.word_80055FA8 & (1u << 2u)) == 0u);
}

} // namespace

int main() {
    TestRejectsNonExactObservationShape();
    TestAcceptedObservationPublishesOnlyCallbackSlotValues();
    TestDiscFullbootStartupObservationPublishesZeroSlotsOnly();
    TestNaturalRegistersOverwriteStartupObservation();
    TestInterruptCallbackTableSetterFailsClosed();
    TestInterruptCallbackTableSetterUpdatesMaskAndTypedAudit();
    if (g_failedChecks != 0) {
        std::printf(
            "test_ss0_stage1_xacd_callback_slot_observation_contract: failed checks=%d\n",
            g_failedChecks);
        return 1;
    }
    std::printf(
        "test_ss0_stage1_xacd_callback_slot_observation_contract: ok\n");
    return 0;
}
