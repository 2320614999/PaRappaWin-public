#include "pr/pr_ss0_word800916f0_direct.h"

#include <cstdio>

namespace {

using namespace PrSS0Scene0RuntimeDirect;

int g_failedChecks = 0;

#define CHECK(expr)                                                        \
    do {                                                                   \
        if (!(expr)) {                                                     \
            std::printf("CHECK failed %s:%d: %s\n", __FILE__, __LINE__,    \
                        #expr);                                            \
            ++g_failedChecks;                                              \
        }                                                                  \
    } while (0)

Word800916F0Observation RuntimeObservation(uint16_t value) {
    Word800916F0Observation observation{};
    observation.source = Word800916F0ObservationSource::RuntimePsxMemoryObservation;
    observation.psxAddress = 0x800916F0u;
    observation.byteSize = 2u;
    observation.valueKnown = true;
    observation.value = value;
    observation.frameKnown = true;
    observation.frame = 1234u;
    observation.pcKnown = true;
    observation.pc = 0x800267F8u;
    return observation;
}

Word800916F0Observation ConsumerReadObservation(uint16_t value) {
    Word800916F0Observation observation = RuntimeObservation(value);
    observation.source =
        Word800916F0ObservationSource::RuntimePsxConsumerReadObservation;
    observation.pc = 0x801C84B0u;
    return observation;
}

Word800916F0InitialObservationInput ResetInjectStartupSource() {
    Word800916F0InitialObservationInput input{};
    input.source =
        Word800916F0InitialObservationSource::DuckStationResetInjectBootExe;
    input.ramResetZeroKnown = true;
    input.psExeLoadRangeKnown = true;
    input.psExeLoadStart = 0x80010000u;
    input.psExeLoadEnd = 0x8006F000u;
    input.psExeMemfillRangeKnown = true;
    input.psExeMemfillStart = 0u;
    input.psExeMemfillEnd = 0u;
    input.targetAddressKnown = true;
    input.targetAddress = 0x800916F0u;
    return input;
}

Word800916F0InitialObservationInput DiscBootStartupSource() {
    Word800916F0InitialObservationInput input = ResetInjectStartupSource();
    input.source = Word800916F0InitialObservationSource::DuckStationDiscBoot;
    input.discBootBiosNonWriterKnown = true;
    return input;
}

void CheckUnknownPlaceholder() {
    CHECK(!IsWord800916F0Known());
    CHECK(GetWord800916F0() == 0u);
}

void CheckObservationSnapshotWithProvenance(
    int acceptedCount,
    int runtimeObservationAcceptedCount,
    int startupObservationAcceptedCount,
    int rejectedCount,
    Word800916F0ObservationRejectReason reason,
    Word800916F0InitialObservationSource lastStartupSource) {
    const Word800916F0ObservationDebugSnapshot snapshot =
        GetWord800916F0ObservationDebugSnapshot();
    CHECK(snapshot.known == IsWord800916F0Known());
    CHECK(snapshot.acceptedCount == acceptedCount);
    CHECK(snapshot.runtimeObservationAcceptedCount ==
          runtimeObservationAcceptedCount);
    CHECK(snapshot.startupObservationAcceptedCount ==
          startupObservationAcceptedCount);
    CHECK(snapshot.rejectedCount == rejectedCount);
    CHECK(snapshot.lastRejectReason == reason);
    CHECK(snapshot.lastStartupSource == lastStartupSource);
}

void CheckObservationSnapshot(int acceptedCount,
                              int rejectedCount,
                              Word800916F0ObservationRejectReason reason) {
    CheckObservationSnapshotWithProvenance(
        acceptedCount,
        acceptedCount,
        0,
        rejectedCount,
        reason,
        Word800916F0InitialObservationSource::Unknown);
}

void CheckStartupObservationSnapshot(
    int acceptedCount,
    int runtimeObservationAcceptedCount,
    int startupObservationAcceptedCount,
    int rejectedCount,
    Word800916F0ObservationRejectReason reason,
    Word800916F0InitialObservationSource lastStartupSource) {
    CheckObservationSnapshotWithProvenance(
        acceptedCount,
        runtimeObservationAcceptedCount,
        startupObservationAcceptedCount,
        rejectedCount,
        reason,
        lastStartupSource);
}

void TestClearKeepsF0Unknown() {
    ClearWord800916F0RuntimeObservation();
    CheckUnknownPlaceholder();
    CheckObservationSnapshot(
        0,
        0,
        Word800916F0ObservationRejectReason::None);
}

void TestRejectsUnknownSource() {
    ClearWord800916F0RuntimeObservation();
    Word800916F0Observation observation = RuntimeObservation(0x1234u);
    observation.source = Word800916F0ObservationSource::Unknown;
    CHECK(!ApplyWord800916F0RuntimeObservation(observation));
    CheckUnknownPlaceholder();
    CheckObservationSnapshot(
        0,
        1,
        Word800916F0ObservationRejectReason::
            NonRuntimePsxMemoryObservation);
}

void TestRejectsWrongAddress() {
    ClearWord800916F0RuntimeObservation();
    Word800916F0Observation observation = RuntimeObservation(0x2222u);
    observation.psxAddress = 0x800916EEu;
    CHECK(!ApplyWord800916F0RuntimeObservation(observation));
    CheckUnknownPlaceholder();
    CheckObservationSnapshot(
        0,
        1,
        Word800916F0ObservationRejectReason::WrongPsxAddress);
}

void TestRejectsWrongByteSize() {
    ClearWord800916F0RuntimeObservation();
    Word800916F0Observation observation = RuntimeObservation(0x3333u);
    observation.byteSize = 4u;
    CHECK(!ApplyWord800916F0RuntimeObservation(observation));
    CheckUnknownPlaceholder();
    CheckObservationSnapshot(
        0,
        1,
        Word800916F0ObservationRejectReason::WrongByteSize);
}

void TestRejectsUnknownValue() {
    ClearWord800916F0RuntimeObservation();
    Word800916F0Observation observation = RuntimeObservation(0x4444u);
    observation.valueKnown = false;
    CHECK(!ApplyWord800916F0RuntimeObservation(observation));
    CheckUnknownPlaceholder();
    CheckObservationSnapshot(
        0,
        1,
        Word800916F0ObservationRejectReason::UnknownValue);
}

void TestRejectsConsumerReadObservation() {
    ClearWord800916F0RuntimeObservation();
    CHECK(!ApplyWord800916F0RuntimeObservation(
        ConsumerReadObservation(0x0000u)));
    CheckUnknownPlaceholder();
    CheckObservationSnapshot(
        0,
        1,
        Word800916F0ObservationRejectReason::
            NonRuntimePsxMemoryObservation);
}

void TestAcceptedRuntimeObservationSetsKnownValue() {
    ClearWord800916F0RuntimeObservation();
    CHECK(ApplyWord800916F0RuntimeObservation(RuntimeObservation(0x55AAu)));
    CHECK(IsWord800916F0Known());
    CHECK(GetWord800916F0() == 0x55AAu);
    CheckObservationSnapshot(
        1,
        0,
        Word800916F0ObservationRejectReason::None);
}

void TestRejectedObservationDoesNotOverwriteAcceptedValue() {
    ClearWord800916F0RuntimeObservation();
    CHECK(ApplyWord800916F0RuntimeObservation(RuntimeObservation(0x1357u)));
    Word800916F0Observation rejected = RuntimeObservation(0x2468u);
    rejected.valueKnown = false;
    CHECK(!ApplyWord800916F0RuntimeObservation(rejected));
    CHECK(IsWord800916F0Known());
    CHECK(GetWord800916F0() == 0x1357u);
    CheckObservationSnapshot(
        1,
        1,
        Word800916F0ObservationRejectReason::UnknownValue);
}

void TestConsumerReadObservationDoesNotOverwriteAcceptedValue() {
    ClearWord800916F0RuntimeObservation();
    CHECK(ApplyWord800916F0RuntimeObservation(RuntimeObservation(0x1357u)));
    CHECK(!ApplyWord800916F0RuntimeObservation(
        ConsumerReadObservation(0x0000u)));
    CHECK(IsWord800916F0Known());
    CHECK(GetWord800916F0() == 0x1357u);
    CheckObservationSnapshot(
        1,
        1,
        Word800916F0ObservationRejectReason::
            NonRuntimePsxMemoryObservation);
}

void TestResetInjectStartupSourcePublishesKnownZeroWhenAllClear() {
    ClearWord800916F0RuntimeObservation();
    CHECK(PublishInitialWord800916F0FromStartupSource(
        ResetInjectStartupSource()));
    CHECK(IsWord800916F0Known());
    CHECK(GetWord800916F0() == 0u);
    CheckStartupObservationSnapshot(
        1,
        0,
        1,
        0,
        Word800916F0ObservationRejectReason::None,
        Word800916F0InitialObservationSource::DuckStationResetInjectBootExe);
}

void TestResetInjectStartupSourceFailsClosedWithoutResetProof() {
    ClearWord800916F0RuntimeObservation();
    Word800916F0InitialObservationInput input = ResetInjectStartupSource();
    input.ramResetZeroKnown = false;
    CHECK(!PublishInitialWord800916F0FromStartupSource(input));
    CheckUnknownPlaceholder();
    CheckObservationSnapshot(
        0,
        0,
        Word800916F0ObservationRejectReason::None);
}

void TestResetInjectStartupSourceFailsClosedWhenLoadCoversF0() {
    ClearWord800916F0RuntimeObservation();
    Word800916F0InitialObservationInput input = ResetInjectStartupSource();
    input.psExeLoadEnd = 0x800A0000u;
    CHECK(!PublishInitialWord800916F0FromStartupSource(input));
    CheckUnknownPlaceholder();
    CheckObservationSnapshot(
        0,
        0,
        Word800916F0ObservationRejectReason::None);
}

void TestDiscBootStartupSourceFailsClosedWithoutBiosNonWriterProof() {
    ClearWord800916F0RuntimeObservation();
    Word800916F0InitialObservationInput input = ResetInjectStartupSource();
    input.source = Word800916F0InitialObservationSource::DuckStationDiscBoot;
    input.discBootBiosNonWriterKnown = false;
    CHECK(!PublishInitialWord800916F0FromStartupSource(input));
    CheckUnknownPlaceholder();
    CheckObservationSnapshot(
        0,
        0,
        Word800916F0ObservationRejectReason::None);
}

void TestDiscBootStartupSourcePublishesKnownZeroWhenAllClear() {
    ClearWord800916F0RuntimeObservation();
    CHECK(PublishInitialWord800916F0FromStartupSource(
        DiscBootStartupSource()));
    CHECK(IsWord800916F0Known());
    CHECK(GetWord800916F0() == 0u);
    CheckStartupObservationSnapshot(
        1,
        0,
        1,
        0,
        Word800916F0ObservationRejectReason::None,
        Word800916F0InitialObservationSource::DuckStationDiscBoot);
}

void TestDiscFullbootStartupSourcePublishesKnownZeroFromContract() {
    ClearWord800916F0RuntimeObservation();
    CHECK(PublishInitialWord800916F0FromDiscFullbootStartupSource());
    CHECK(IsWord800916F0Known());
    CHECK(GetWord800916F0() == 0u);
    CheckStartupObservationSnapshot(
        1,
        0,
        1,
        0,
        Word800916F0ObservationRejectReason::None,
        Word800916F0InitialObservationSource::DuckStationDiscBoot);
}

void TestPsxrecFrameSnapshotCannotPublishStartupSource() {
    ClearWord800916F0RuntimeObservation();
    Word800916F0InitialObservationInput input = ResetInjectStartupSource();
    input.source = Word800916F0InitialObservationSource::PsxrecFrameSnapshot;
    CHECK(!PublishInitialWord800916F0FromStartupSource(input));
    CheckUnknownPlaceholder();
    CheckObservationSnapshot(
        0,
        0,
        Word800916F0ObservationRejectReason::None);
}

void TestRejectedStartupSourceDoesNotOverwriteAcceptedRuntimeObservation() {
    ClearWord800916F0RuntimeObservation();
    CHECK(ApplyWord800916F0RuntimeObservation(RuntimeObservation(0x2468u)));

    Word800916F0InitialObservationInput consumerRead =
        ResetInjectStartupSource();
    consumerRead.source =
        Word800916F0InitialObservationSource::RuntimePsxConsumerRead;
    CHECK(!PublishInitialWord800916F0FromStartupSource(consumerRead));
    CHECK(IsWord800916F0Known());
    CHECK(GetWord800916F0() == 0x2468u);
    CheckObservationSnapshot(
        1,
        0,
        Word800916F0ObservationRejectReason::None);

    Word800916F0InitialObservationInput psxrec =
        ResetInjectStartupSource();
    psxrec.source = Word800916F0InitialObservationSource::PsxrecFrameSnapshot;
    CHECK(!PublishInitialWord800916F0FromStartupSource(psxrec));
    CHECK(IsWord800916F0Known());
    CHECK(GetWord800916F0() == 0x2468u);
    CheckObservationSnapshot(
        1,
        0,
        Word800916F0ObservationRejectReason::None);
}

void TestStartupKnownZeroDoesNotOverwriteAcceptedRuntimeObservation() {
    ClearWord800916F0RuntimeObservation();
    CHECK(ApplyWord800916F0RuntimeObservation(RuntimeObservation(0x2468u)));

    CHECK(!PublishInitialWord800916F0FromStartupSource(
        ResetInjectStartupSource()));
    CHECK(IsWord800916F0Known());
    CHECK(GetWord800916F0() == 0x2468u);
    CheckObservationSnapshot(
        1,
        0,
        Word800916F0ObservationRejectReason::None);

    CHECK(!PublishInitialWord800916F0FromDiscFullbootStartupSource());
    CHECK(IsWord800916F0Known());
    CHECK(GetWord800916F0() == 0x2468u);
    CheckObservationSnapshot(
        1,
        0,
        Word800916F0ObservationRejectReason::None);
}

} // namespace

int main() {
    TestClearKeepsF0Unknown();
    TestRejectsUnknownSource();
    TestRejectsWrongAddress();
    TestRejectsWrongByteSize();
    TestRejectsUnknownValue();
    TestRejectsConsumerReadObservation();
    TestAcceptedRuntimeObservationSetsKnownValue();
    TestRejectedObservationDoesNotOverwriteAcceptedValue();
    TestConsumerReadObservationDoesNotOverwriteAcceptedValue();
    TestResetInjectStartupSourcePublishesKnownZeroWhenAllClear();
    TestResetInjectStartupSourceFailsClosedWithoutResetProof();
    TestResetInjectStartupSourceFailsClosedWhenLoadCoversF0();
    TestDiscBootStartupSourceFailsClosedWithoutBiosNonWriterProof();
    TestDiscBootStartupSourcePublishesKnownZeroWhenAllClear();
    TestDiscFullbootStartupSourcePublishesKnownZeroFromContract();
    TestPsxrecFrameSnapshotCannotPublishStartupSource();
    TestRejectedStartupSourceDoesNotOverwriteAcceptedRuntimeObservation();
    TestStartupKnownZeroDoesNotOverwriteAcceptedRuntimeObservation();

    if (g_failedChecks != 0) {
        std::printf("test_ss0_word800916f0_observation: failed checks=%d\n",
                    g_failedChecks);
        return 1;
    }
    std::printf("test_ss0_word800916f0_observation: ok\n");
    return 0;
}
