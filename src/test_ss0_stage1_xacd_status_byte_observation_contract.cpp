#include "pr/pr_stage1_xa_cd_direct.h"

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

PrStage1XaCdDirectStatusByteObservation800573D4 RuntimeObservation(
    uint8_t value) {
    PrStage1XaCdDirectStatusByteObservation800573D4 observation{};
    observation.source =
        PrStage1XaCdDirectStatusByteObservationSource::
            RuntimePsxMemoryObservation;
    observation.psxAddress = 0x800573D4u;
    observation.byteSize = 1u;
    observation.valueKnown = true;
    observation.value = value;
    observation.frameKnown = true;
    observation.frame = 4096u;
    observation.pcKnown = true;
    observation.pc = 0x80038FC0u;
    return observation;
}

void SeedSentinelState(PrStage1XaCdDirectState& state) {
    state.byte_800573D4Known = true;
    state.byte_800573D4 = 0x11u;
    state.statusRead80036384Known = true;
    state.statusRead80036384 = 0x22u;
    state.statusByte800573D4ObservationAcceptedCount = 3u;
    state.statusByte800573D4ObservationRejectedCount = 4u;
    state.statusByte800573D4LastReject = 0u;
    state.byte_80057119Known = true;
    state.byte_80057119 = 0x10u;
    state.byte80057119ProducerFunction = 0x800375BCu;
}

void CheckRejectLeavesStateUnchanged(
    PrStage1XaCdDirectStatusByteObservation800573D4 observation,
    PrStage1XaCdDirectStatusByteObservationRejectReason expectedReason) {
    PrStage1XaCdDirectState state{};
    SeedSentinelState(state);

    const PrStage1XaCdDirectStatusByteObservationResult result =
        PrStage1XaCdDirectApplyStatusByteRuntimeObservation800573D4(
            observation,
            state);

    CHECK(!result.accepted);
    CHECK(result.rejectReason == expectedReason);
    CHECK(state.byte_800573D4Known);
    CHECK(state.byte_800573D4 == 0x11u);
    CHECK(state.statusRead80036384Known);
    CHECK(state.statusRead80036384 == 0x22u);
    CHECK(state.statusByte800573D4ObservationAcceptedCount == 3u);
    CHECK(state.statusByte800573D4ObservationRejectedCount == 5u);
    CHECK(state.statusByte800573D4LastReject ==
          static_cast<uint8_t>(expectedReason));
    CHECK(state.byte_80057119Known);
    CHECK(state.byte_80057119 == 0x10u);
    CHECK(state.byte80057119ProducerFunction == 0x800375BCu);
}

void TestRejectsNonExactObservationShape() {
    PrStage1XaCdDirectStatusByteObservation800573D4 observation =
        RuntimeObservation(0x00u);
    observation.source = PrStage1XaCdDirectStatusByteObservationSource::Unknown;
    CheckRejectLeavesStateUnchanged(
        observation,
        PrStage1XaCdDirectStatusByteObservationRejectReason::
            NonRuntimePsxMemoryObservation);

    observation = RuntimeObservation(0x00u);
    observation.source =
        PrStage1XaCdDirectStatusByteObservationSource::
            RuntimePsxConsumerReadObservation;
    CheckRejectLeavesStateUnchanged(
        observation,
        PrStage1XaCdDirectStatusByteObservationRejectReason::
            NonRuntimePsxMemoryObservation);

    observation = RuntimeObservation(0x00u);
    observation.psxAddress = 0x800573D5u;
    CheckRejectLeavesStateUnchanged(
        observation,
        PrStage1XaCdDirectStatusByteObservationRejectReason::WrongPsxAddress);

    observation = RuntimeObservation(0x00u);
    observation.byteSize = 4u;
    CheckRejectLeavesStateUnchanged(
        observation,
        PrStage1XaCdDirectStatusByteObservationRejectReason::WrongByteSize);

    observation = RuntimeObservation(0x00u);
    observation.valueKnown = false;
    CheckRejectLeavesStateUnchanged(
        observation,
        PrStage1XaCdDirectStatusByteObservationRejectReason::UnknownValue);
}

void TestAcceptedObservationPublishesOnlyStatusAuthority() {
    PrStage1XaCdDirectState state{};

    const PrStage1XaCdDirectStatusByteObservationResult result =
        PrStage1XaCdDirectApplyStatusByteRuntimeObservation800573D4(
            RuntimeObservation(0x05u),
            state);

    CHECK(result.accepted);
    CHECK(result.rejectReason ==
          PrStage1XaCdDirectStatusByteObservationRejectReason::None);
    CHECK(state.byte_800573D4Known);
    CHECK(state.byte_800573D4 == 0x05u);
    CHECK(state.statusRead80036384Known);
    CHECK(state.statusRead80036384 == 0x05u);
    CHECK(state.statusByte800573D4ObservationAcceptedCount == 1u);
    CHECK(state.statusByte800573D4ObservationRejectedCount == 0u);
    CHECK(state.statusByte800573D4LastReject == 0u);
    CHECK(!state.byte_80057119Known);
    CHECK(state.byte80057119ProducerFunction == 0u);
    CHECK(!state.cdSyncExplicitStatusKnown);
    CHECK(!state.cdLowerFeedback80036AF8Known);
}

void TestRejectedObservationDoesNotOverwriteAcceptedStatus() {
    PrStage1XaCdDirectState state{};
    CHECK(PrStage1XaCdDirectApplyStatusByteRuntimeObservation800573D4(
              RuntimeObservation(0x02u),
              state)
              .accepted);

    PrStage1XaCdDirectStatusByteObservation800573D4 rejected =
        RuntimeObservation(0x05u);
    rejected.valueKnown = false;
    CHECK(!PrStage1XaCdDirectApplyStatusByteRuntimeObservation800573D4(
               rejected,
               state)
               .accepted);

    CHECK(state.byte_800573D4Known);
    CHECK(state.byte_800573D4 == 0x02u);
    CHECK(state.statusRead80036384Known);
    CHECK(state.statusRead80036384 == 0x02u);
    CHECK(state.statusByte800573D4ObservationAcceptedCount == 1u);
    CHECK(state.statusByte800573D4ObservationRejectedCount == 1u);
}

} // namespace

int main() {
    TestRejectsNonExactObservationShape();
    TestAcceptedObservationPublishesOnlyStatusAuthority();
    TestRejectedObservationDoesNotOverwriteAcceptedStatus();
    if (g_failedChecks != 0) {
        std::printf(
            "test_ss0_stage1_xacd_status_byte_observation_contract: failed checks=%d\n",
            g_failedChecks);
        return 1;
    }
    std::printf(
        "test_ss0_stage1_xacd_status_byte_observation_contract: ok\n");
    return 0;
}
