#include "pr/pr_stage1_scorer_direct.h"
#include "pr/pr_stage1_scorer_host.h"

#include <cstdio>
#include <cstdint>
#include <vector>

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

void WriteU32LE(std::vector<uint8_t>& bytes, size_t offset, uint32_t value) {
    bytes[offset] = static_cast<uint8_t>(value & 0xFFu);
    bytes[offset + 1u] = static_cast<uint8_t>((value >> 8u) & 0xFFu);
    bytes[offset + 2u] = static_cast<uint8_t>((value >> 16u) & 0xFFu);
    bytes[offset + 3u] = static_cast<uint8_t>((value >> 24u) & 0xFFu);
}

std::vector<uint8_t> MakeReplayMirrorBytes() {
    std::vector<uint8_t> bytes(kPrStage1ScorerDirectReplayMirrorByteCount);
    for (size_t i = 0; i < kPrStage1ScorerDirectReplayBufferCapacity; ++i) {
        const size_t offset = i * 2u * sizeof(uint32_t);
        WriteU32LE(bytes, offset, 0x10000000u + static_cast<uint32_t>(i));
        WriteU32LE(
            bytes,
            offset + sizeof(uint32_t),
            0x20000000u + static_cast<uint32_t>(i));
    }
    return bytes;
}

PrStage1ScorerDirectReplayMirrorObservation ValidObservation(
    const std::vector<uint8_t>& bytes) {
    PrStage1ScorerDirectReplayMirrorObservation observation{};
    observation.source =
        PrStage1ScorerDirectReplayMirrorObservationSource::
            RuntimePsxMemoryObservation;
    observation.psxAddress = 0x8008EEF8u;
    observation.byteSize = kPrStage1ScorerDirectReplayMirrorByteCount;
    observation.valueKnown = true;
    observation.bytes = bytes.data();
    observation.bytesSize = bytes.size();
    observation.publishedCountKnown901BC = true;
    observation.publishedCount901BC = 1u;
    observation.writeCountKnown901C0 = true;
    observation.writeCount901C0 = 1u;
    observation.frameKnown = true;
    observation.frame = 607u;
    observation.pcKnown = true;
    observation.pc = kPrStage1ScorerDirectFn80014614;
    return observation;
}

struct FakeAcceptedProducerRuntime {
    PrStage1ScorerDirectSourceCellHeader header{};
    PrStage1ScorerDirectSourceCell cell{};
};

bool ReadTimingTemplateState(void*, uint8_t selectorByte1, uint8_t,
                             uint8_t& outState) {
    if (selectorByte1 != 6u) {
        return false;
    }
    outState = 2u;
    return true;
}

bool ReadSourceCellHeader(void* userData,
                          uint8_t selectorByte0,
                          uint8_t classToken20,
                          PrStage1ScorerDirectSourceCellHeader& outHeader) {
    FakeAcceptedProducerRuntime& runtime =
        *static_cast<FakeAcceptedProducerRuntime*>(userData);
    if (selectorByte0 != 3u || classToken20 != 3u) {
        return false;
    }
    outHeader = runtime.header;
    return outHeader.valid;
}

bool ReadSourceCell(void* userData,
                    uint32_t sourceCellPtr,
                    PrStage1ScorerDirectSourceCell& outCell) {
    FakeAcceptedProducerRuntime& runtime =
        *static_cast<FakeAcceptedProducerRuntime*>(userData);
    if (sourceCellPtr != runtime.header.dword04BasePtr) {
        return false;
    }
    outCell = runtime.cell;
    return outCell.valid;
}

PrStage1ScorerDirectAcceptedProducerAccessors MakeAcceptedProducerAccessors(
    FakeAcceptedProducerRuntime& runtime) {
    PrStage1ScorerDirectAcceptedProducerAccessors accessors{};
    accessors.userData = &runtime;
    accessors.readTimingTemplateState = ReadTimingTemplateState;
    accessors.readSourceCellHeader = ReadSourceCellHeader;
    accessors.readSourceCell = ReadSourceCell;
    return accessors;
}

FakeAcceptedProducerRuntime MakeAcceptedProducerRuntime() {
    FakeAcceptedProducerRuntime runtime{};
    runtime.header.valid = true;
    runtime.header.dword00HeaderAddr = 0x801CD000u;
    runtime.header.dword04BasePtr = 0x801CE000u;
    runtime.header.word08Count = 1u;
    runtime.header.word0ACursor = 0u;
    runtime.cell.valid = true;
    runtime.cell.dword00SourceCellPtr = 0x801CCCF4u;
    runtime.cell.word06RecordCompanion = 3u;
    runtime.cell.dword08CallbackArgPresent = true;
    runtime.cell.dword08PayloadOpaque = 0x34u;
    return runtime;
}

PrStage1ScorerDirectDescriptorRow MakeQ607DescriptorRow() {
    PrStage1ScorerDirectDescriptorRow row{};
    row.valid = true;
    row.byte12DefaultSelector0 = 3u;
    row.byte13DefaultSelector1 = 6u;
    return row;
}

PrStage1ScorerDirectAcceptedProducerCoreInput MakeQ607AcceptedInput(
    const PrStage1ScorerDirectDescriptorRow& row) {
    PrStage1ScorerDirectAcceptedProducerCoreInput in{};
    in.eventStreamFlagActive = true;
    in.writerControlSample18 = 0x0040u;
    in.classToken20Known = true;
    in.classToken20 = 3u;
    in.acceptedTick96Known = true;
    in.acceptedTick96 = 3647;
    in.halfWindow34 = 8u;
    in.writePageOrdinal38 = 9;
    in.lookaheadDescriptorRow = &row;
    return in;
}

PrStage1ScorerDirectReplayBufferState MakeSealedStage1EventTableReplay() {
    PrStage1ScorerDirectReplayBufferState replay{};
    CHECK(PrStage1ScorerDirectInitializeReplayMirrorFromStartupZero80028590(
        replay));
    const PrStage1ScorerDirectEventTableBuild801C8660Result seed =
        PrStage1ScorerDirectBuildStage1EventTable801C8660(replay);
    CHECK(seed.applied);
    CHECK(seed.count800901BC == 53u);
    CHECK(PrStage1ScorerDirectReplayMirrorAuthorityMatchesState(replay));
    CHECK(replay.replayMirrorProducerKnown8008EEF8);
    CHECK(replay.replayMirrorProducerFunction ==
          kPrStage1ScorerDirectFn801C8660);
    CHECK(replay.replayMirrorFullBackingKnown8008EEF8);
    return replay;
}

void CheckTamperedReplayStaysInvalidAfter24E54(
    PrStage1ScorerDirectReplayBufferState replay) {
    CHECK(!PrStage1ScorerDirectReplayMirrorAuthorityMatchesState(replay));
    CHECK(replay.replayMirrorAuthority.Kind() !=
          PrStage1ScorerDirectReplayMirrorAuthorityKind::Unknown);
    const PrStage1ScorerDirectReplayBufferState before = replay;

    const PrStage1ScorerDirectResolvedReplayBackup1681C noBackup{};
    const PrStage1ScorerDirectAcceptedSpecialSetupResult setup =
        PrStage1ScorerDirectRunAcceptedSpecialSetupCore24E54_1681C(
            0u,
            noBackup,
            replay);

    CHECK(!setup.setup.restoreReplayBuffer1681CRequested);
    CHECK(!setup.setup.seedStage1EventTable801C8660Requested);
    CHECK(!setup.restore.restoreApplied);
    CHECK(!PrStage1ScorerDirectReplayMirrorAuthorityMatchesState(replay));
    CHECK(replay.replayMirrorAuthority.Kind() ==
          PrStage1ScorerDirectReplayMirrorAuthorityKind::Unknown);
    CHECK(replay.dwordEEF8Tick96 == before.dwordEEF8Tick96);
    CHECK(replay.dwordEEFCClassMask == before.dwordEEFCClassMask);
    CHECK(replay.replayMirrorKnown8008EEF8 ==
          before.replayMirrorKnown8008EEF8);
    CHECK(replay.replayMirrorProducerKnown8008EEF8 ==
          before.replayMirrorProducerKnown8008EEF8);
    CHECK(replay.replayMirrorProducerFunction ==
          before.replayMirrorProducerFunction);
    CHECK(replay.replayMirrorByteCountKnown8008EEF8 ==
          before.replayMirrorByteCountKnown8008EEF8);
    CHECK(replay.replayMirrorKnownByteCount8008EEF8 ==
          before.replayMirrorKnownByteCount8008EEF8);
    CHECK(replay.replayMirrorFullBackingKnown8008EEF8 ==
          before.replayMirrorFullBackingKnown8008EEF8);
    CHECK(replay.dword901BCPublishedCount == 0u);
    CHECK(replay.dword901C0WriteCount == 0u);

    const PrStage1ScorerDirectReplayBackupCaptureResult capture =
        PrStage1ScorerDirectRunReplayBackupCaptureCore1635C(replay);
    CHECK(!capture.captureApplied);
    CHECK(!capture.backup.valid);
}

void CheckRejectLeavesReplayClosed(
    PrStage1ScorerDirectReplayMirrorObservation observation,
    PrStage1ScorerDirectReplayMirrorObservationRejectReason expectedReason) {
    PrStage1ScorerDirectReplayBufferState replay{};
    replay.dwordEEF8Tick96[0] = 0xAAAAAAAAu;
    replay.dwordEEFCClassMask[0] = 0xBBBBBBBBu;

    const PrStage1ScorerDirectReplayMirrorObservationResult result =
        PrStage1ScorerDirectApplyReplayMirrorRuntimeObservation8008EEF8(
            observation,
            replay);

    CHECK(!result.accepted);
    CHECK(result.rejectReason == expectedReason);
    CHECK(!replay.replayMirrorKnown8008EEF8);
    CHECK(!replay.replayMirrorProducerKnown8008EEF8);
    CHECK(replay.replayMirrorProducerFunction == 0u);
    CHECK(!replay.replayMirrorByteCountKnown8008EEF8);
    CHECK(replay.replayMirrorKnownByteCount8008EEF8 == 0u);
    CHECK(!replay.replayMirrorFullBackingKnown8008EEF8);
    CHECK(replay.dword901BCPublishedCount == 0u);
    CHECK(replay.dword901C0WriteCount == 0u);
    CHECK(replay.dwordEEF8Tick96[0] == 0xAAAAAAAAu);
    CHECK(replay.dwordEEFCClassMask[0] == 0xBBBBBBBBu);
}

void TestRejectsNonExactSources() {
    CHECK(!PrStage1ScorerDirectIsKnownReplayMirrorProducerFunction(
        0x801C4FC8u));
    const std::vector<uint8_t> bytes = MakeReplayMirrorBytes();
    PrStage1ScorerDirectReplayMirrorObservation observation =
        ValidObservation(bytes);
    observation.source =
        PrStage1ScorerDirectReplayMirrorObservationSource::Unknown;
    CheckRejectLeavesReplayClosed(
        observation,
        PrStage1ScorerDirectReplayMirrorObservationRejectReason::
            NonRuntimePsxMemoryObservation);

    observation = ValidObservation(bytes);
    observation.source =
        PrStage1ScorerDirectReplayMirrorObservationSource::
            RuntimePsxConsumerReadObservation;
    CheckRejectLeavesReplayClosed(
        observation,
        PrStage1ScorerDirectReplayMirrorObservationRejectReason::
            NonRuntimePsxMemoryObservation);
}

void TestRejectsWrongShape() {
    const std::vector<uint8_t> bytes = MakeReplayMirrorBytes();
    PrStage1ScorerDirectReplayMirrorObservation observation =
        ValidObservation(bytes);
    observation.psxAddress = 0x8008EEFCu;
    CheckRejectLeavesReplayClosed(
        observation,
        PrStage1ScorerDirectReplayMirrorObservationRejectReason::
            WrongPsxAddress);

    observation = ValidObservation(bytes);
    observation.byteSize = kPrStage1ScorerDirectReplayMirrorByteCount - 4u;
    CheckRejectLeavesReplayClosed(
        observation,
        PrStage1ScorerDirectReplayMirrorObservationRejectReason::
            WrongByteSize);

    observation = ValidObservation(bytes);
    observation.valueKnown = false;
    CheckRejectLeavesReplayClosed(
        observation,
        PrStage1ScorerDirectReplayMirrorObservationRejectReason::UnknownValue);

    observation = ValidObservation(bytes);
    observation.bytes = nullptr;
    observation.bytesSize = 0u;
    CheckRejectLeavesReplayClosed(
        observation,
        PrStage1ScorerDirectReplayMirrorObservationRejectReason::MissingBytes);
}

void TestRejectsCountGaps() {
    const std::vector<uint8_t> bytes = MakeReplayMirrorBytes();
    PrStage1ScorerDirectReplayMirrorObservation observation =
        ValidObservation(bytes);
    observation.publishedCountKnown901BC = false;
    CheckRejectLeavesReplayClosed(
        observation,
        PrStage1ScorerDirectReplayMirrorObservationRejectReason::CountUnknown);

    observation = ValidObservation(bytes);
    observation.writeCountKnown901C0 = false;
    CheckRejectLeavesReplayClosed(
        observation,
        PrStage1ScorerDirectReplayMirrorObservationRejectReason::CountUnknown);

    observation = ValidObservation(bytes);
    observation.publishedCount901BC =
        static_cast<uint32_t>(kPrStage1ScorerDirectReplayBufferCapacity + 1u);
    CheckRejectLeavesReplayClosed(
        observation,
        PrStage1ScorerDirectReplayMirrorObservationRejectReason::
            CountOutOfRange);

    observation = ValidObservation(bytes);
    observation.writeCount901C0 =
        static_cast<uint32_t>(kPrStage1ScorerDirectReplayBufferCapacity + 1u);
    CheckRejectLeavesReplayClosed(
        observation,
        PrStage1ScorerDirectReplayMirrorObservationRejectReason::
            CountOutOfRange);
}

void TestAcceptedObservationRemainsDiagnosticOnly() {
    const std::vector<uint8_t> bytes = MakeReplayMirrorBytes();
    PrStage1ScorerDirectReplayBufferState replay{};
    CHECK(PrStage1ScorerDirectInitializeReplayMirrorFromStartupZero80028590(
        replay));
    const PrStage1ScorerDirectReplayBufferState before = replay;

    const PrStage1ScorerDirectReplayMirrorObservationResult result =
        PrStage1ScorerDirectApplyReplayMirrorRuntimeObservation8008EEF8(
            ValidObservation(bytes),
            replay);

    CHECK(result.accepted);
    CHECK(result.rejectReason ==
          PrStage1ScorerDirectReplayMirrorObservationRejectReason::None);
    CHECK(replay.replayMirrorKnown8008EEF8 ==
          before.replayMirrorKnown8008EEF8);
    CHECK(replay.replayMirrorStartupZeroAuthorityKnown80028590 ==
          before.replayMirrorStartupZeroAuthorityKnown80028590);
    CHECK(replay.replayMirrorProducerKnown8008EEF8 ==
          before.replayMirrorProducerKnown8008EEF8);
    CHECK(replay.replayMirrorProducerFunction ==
          before.replayMirrorProducerFunction);
    CHECK(replay.replayMirrorByteCountKnown8008EEF8 ==
          before.replayMirrorByteCountKnown8008EEF8);
    CHECK(replay.replayMirrorKnownByteCount8008EEF8 ==
          before.replayMirrorKnownByteCount8008EEF8);
    CHECK(replay.replayMirrorFullBackingKnown8008EEF8 ==
          before.replayMirrorFullBackingKnown8008EEF8);
    CHECK(replay.dword901BCPublishedCount ==
          before.dword901BCPublishedCount);
    CHECK(replay.dword901C0WriteCount == before.dword901C0WriteCount);
    CHECK(replay.dwordEEF8Tick96 == before.dwordEEF8Tick96);
    CHECK(replay.dwordEEFCClassMask == before.dwordEEFCClassMask);

    const PrStage1ScorerDirectReplayBackupCaptureResult beforeProducer =
        PrStage1ScorerDirectRunReplayBackupCaptureCore1635C(replay);
    CHECK(!beforeProducer.backup.valid);
    CHECK(!beforeProducer.captureApplied);
}

void TestNaturalProducerCannotPromoteDiagnosticObservationBacking() {
    const std::vector<uint8_t> bytes = MakeReplayMirrorBytes();
    PrStage1ScorerDirectReplayBufferState replay{};
    PrStage1ScorerDirectReplayMirrorObservation observation =
        ValidObservation(bytes);
    observation.publishedCount901BC = 599u;
    observation.writeCount901C0 = 599u;
    CHECK(PrStage1ScorerDirectApplyReplayMirrorRuntimeObservation8008EEF8(
              observation,
              replay)
              .accepted);

    PrStage1ScorerDirectGlobals globals{};
    PrStage1ScorerDirectAcceptedProducerOwnerState ownerState{};
    FakeAcceptedProducerRuntime runtime = MakeAcceptedProducerRuntime();
    const PrStage1ScorerDirectAcceptedProducerAccessors accessors =
        MakeAcceptedProducerAccessors(runtime);
    const PrStage1ScorerDirectDescriptorRow row = MakeQ607DescriptorRow();
    const PrStage1ScorerDirectAcceptedProducerRunResult run =
        PrStage1ScorerDirectRunAcceptedProducer14614(
            globals,
            ownerState,
            replay,
            MakeQ607AcceptedInput(row),
            accessors);

    CHECK(run.resultCode == 0);
    CHECK(run.replayAppendRan);
    CHECK(replay.replayMirrorKnown8008EEF8);
    CHECK(replay.replayMirrorProducerKnown8008EEF8);
    CHECK(replay.replayMirrorProducerFunction ==
          kPrStage1ScorerDirectFn80014614);
    CHECK(replay.replayMirrorByteCountKnown8008EEF8);
    CHECK(replay.replayMirrorKnownByteCount8008EEF8 ==
          2u * sizeof(uint32_t));
    CHECK(!replay.replayMirrorStartupZeroAuthorityKnown80028590);
    CHECK(!replay.replayMirrorFullBackingKnown8008EEF8);
    CHECK(replay.dword901BCPublishedCount == 1u);
    CHECK(replay.dword901C0WriteCount == 1u);
    CHECK(replay.dwordEEF8Tick96[0] == 3647u);
    CHECK(replay.dwordEEFCClassMask[0] == 0x0040u);
    CHECK(replay.dwordEEF8Tick96[599] == 0u);
    CHECK(replay.dwordEEFCClassMask[599] == 0u);

    const PrStage1ScorerDirectReplayBackupCaptureResult afterProducer =
        PrStage1ScorerDirectRunReplayBackupCaptureCore1635C(replay);
    CHECK(!afterProducer.captureApplied);
    CHECK(!afterProducer.backup.valid);
}

void TestDiscFullbootPublisherRemainsDiagnosticOnly() {
    PrStage1ScorerDirectReplayBufferState replay{};
    const auto observation =
        PrStage1ScorerDirectPublishReplayMirrorDiscFullbootStartupObservation8008EEF8(
            replay);
    CHECK(observation.accepted);
    CHECK(!replay.replayMirrorKnown8008EEF8);
    CHECK(!replay.replayMirrorStartupZeroAuthorityKnown80028590);
    CHECK(!replay.replayMirrorProducerKnown8008EEF8);
    CHECK(!replay.replayMirrorByteCountKnown8008EEF8);
    CHECK(replay.replayMirrorKnownByteCount8008EEF8 == 0u);
    CHECK(!replay.replayMirrorFullBackingKnown8008EEF8);
    CHECK(replay.dword901BCPublishedCount == 0u);
    CHECK(replay.dword901C0WriteCount == 0u);
}

void TestIdaStartupZeroThenQ607AppendPublishes80014614Backing() {
    PrStage1ScorerDirectReplayBufferState replay{};
    CHECK(PrStage1ScorerDirectInitializeReplayMirrorFromStartupZero80028590(
        replay));
    CHECK(replay.replayMirrorKnown8008EEF8);
    CHECK(replay.replayMirrorStartupZeroAuthorityKnown80028590);
    CHECK(!replay.replayMirrorProducerKnown8008EEF8);
    CHECK(replay.replayMirrorProducerFunction == 0u);
    CHECK(replay.replayMirrorByteCountKnown8008EEF8);
    CHECK(replay.replayMirrorKnownByteCount8008EEF8 ==
          kPrStage1ScorerDirectReplayMirrorByteCount);
    CHECK(replay.replayMirrorFullBackingKnown8008EEF8);
    CHECK(replay.dword901BCPublishedCount == 0u);
    CHECK(replay.dword901C0WriteCount == 0u);
    CHECK(replay.dwordEEF8Tick96[0] == 0u);
    CHECK(replay.dwordEEFCClassMask[0] == 0u);
    CHECK(replay.dwordEEF8Tick96[599] == 0u);
    CHECK(replay.dwordEEFCClassMask[599] == 0u);

    PrStage1ScorerDirectGlobals globals{};
    PrStage1ScorerDirectAcceptedProducerOwnerState ownerState{};
    FakeAcceptedProducerRuntime runtime = MakeAcceptedProducerRuntime();
    const PrStage1ScorerDirectAcceptedProducerAccessors accessors =
        MakeAcceptedProducerAccessors(runtime);
    const PrStage1ScorerDirectDescriptorRow row = MakeQ607DescriptorRow();
    const PrStage1ScorerDirectAcceptedProducerRunResult run =
        PrStage1ScorerDirectRunAcceptedProducer14614(
            globals,
            ownerState,
            replay,
            MakeQ607AcceptedInput(row),
            accessors);

    CHECK(run.resultCode == 0);
    CHECK(run.replayAppendRan);
    CHECK(run.writeRan);
    CHECK(run.writeResult.recordWritten);
    CHECK(replay.replayMirrorKnown8008EEF8);
    CHECK(!replay.replayMirrorStartupZeroAuthorityKnown80028590);
    CHECK(replay.replayMirrorProducerKnown8008EEF8);
    CHECK(replay.replayMirrorProducerFunction ==
          kPrStage1ScorerDirectFn80014614);
    CHECK(replay.replayMirrorByteCountKnown8008EEF8);
    CHECK(replay.replayMirrorKnownByteCount8008EEF8 ==
          kPrStage1ScorerDirectReplayMirrorByteCount);
    CHECK(replay.replayMirrorFullBackingKnown8008EEF8);
    CHECK(replay.dword901BCPublishedCount == 1u);
    CHECK(replay.dword901C0WriteCount == 1u);
    CHECK(replay.dwordEEF8Tick96[0] == 3647u);
    CHECK(replay.dwordEEFCClassMask[0] == 0x0040u);
    CHECK(replay.dwordEEF8Tick96[1] == 0u);
    CHECK(replay.dwordEEFCClassMask[1] == 0u);

    const PrStage1ScorerDirectReplayBackupCaptureResult afterProducer =
        PrStage1ScorerDirectRunReplayBackupCaptureCore1635C(replay);
    CHECK(afterProducer.captureApplied);
    CHECK(afterProducer.backup.valid);
    CHECK(afterProducer.backup.dword92F48PublishedCount == 1u);
    CHECK(afterProducer.backup.replayMirrorFullBackingKnown8008EEF8);
    CHECK(afterProducer.backup.dwordEEF8Tick96[0] == 3647u);
    CHECK(afterProducer.backup.dwordEEFCClassMask[0] == 0x0040u);
    CHECK(afterProducer.backup.dwordEEF8Tick96[1] == 0u);
    CHECK(afterProducer.backup.dwordEEFCClassMask[1] == 0u);
}

void TestTransitionZeroSetupKeepsIdaStartupBackingWithoutProducer() {
    PrStage1ScorerDirectReplayBufferState replay{};
    CHECK(PrStage1ScorerDirectInitializeReplayMirrorFromStartupZero80028590(
        replay));
    CHECK(replay.replayMirrorKnown8008EEF8);
    CHECK(replay.replayMirrorStartupZeroAuthorityKnown80028590);
    CHECK(!replay.replayMirrorProducerKnown8008EEF8);
    CHECK(replay.replayMirrorByteCountKnown8008EEF8);
    CHECK(replay.replayMirrorKnownByteCount8008EEF8 ==
          kPrStage1ScorerDirectReplayMirrorByteCount);
    CHECK(replay.replayMirrorFullBackingKnown8008EEF8);

    PrStage1ScorerDirectResolvedReplayBackup1681C noBackup{};
    const PrStage1ScorerDirectAcceptedSpecialSetupResult setup =
        PrStage1ScorerDirectRunAcceptedSpecialSetupCore24E54_1681C(
            0u,
            noBackup,
            replay);

    CHECK(!setup.setup.restoreReplayBuffer1681CRequested);
    CHECK(!setup.setup.seedStage1EventTable801C8660Requested);
    CHECK(!setup.restore.restoreApplied);
    CHECK(setup.restoreSource ==
          PrStage1ScorerDirectReplayRestoreSource1681C::None);
    CHECK(replay.replayMirrorKnown8008EEF8);
    CHECK(replay.replayMirrorStartupZeroAuthorityKnown80028590);
    CHECK(!replay.replayMirrorProducerKnown8008EEF8);
    CHECK(replay.replayMirrorProducerFunction == 0u);
    CHECK(replay.replayMirrorByteCountKnown8008EEF8);
    CHECK(replay.replayMirrorKnownByteCount8008EEF8 ==
          kPrStage1ScorerDirectReplayMirrorByteCount);
    CHECK(replay.replayMirrorFullBackingKnown8008EEF8);
    CHECK(replay.dword901BCPublishedCount == 0u);
    CHECK(replay.dword901C0WriteCount == 0u);

    const PrStage1ScorerDirectReplayBackupCaptureResult backup =
        PrStage1ScorerDirectRunReplayBackupCaptureCore1635C(replay);
    CHECK(!backup.captureApplied);
    CHECK(!backup.backup.valid);
}

void TestTransitionOneSetupRequestsOnlyStage1EventTableSeed() {
    PrStage1ScorerDirectReplayBufferState replay{};
    CHECK(PrStage1ScorerDirectInitializeReplayMirrorFromStartupZero80028590(
        replay));

    PrStage1ScorerDirectResolvedReplayBackup1681C noBackup{};
    const PrStage1ScorerDirectAcceptedSpecialSetupResult setup =
        PrStage1ScorerDirectRunAcceptedSpecialSetupCore24E54_1681C(
            1u,
            noBackup,
            replay);

    CHECK(!setup.setup.restoreReplayBuffer1681CRequested);
    CHECK(setup.setup.seedStage1EventTable801C8660Requested);
    CHECK(!setup.restore.restoreApplied);
    CHECK(setup.restoreSource ==
          PrStage1ScorerDirectReplayRestoreSource1681C::None);
    CHECK(replay.replayMirrorKnown8008EEF8);
    CHECK(!replay.replayMirrorProducerKnown8008EEF8);

    const PrStage1ScorerDirectEventTableBuild801C8660Result seed =
        PrStage1ScorerDirectBuildStage1EventTable801C8660(replay);
    CHECK(seed.applied);
    CHECK(replay.replayMirrorProducerKnown8008EEF8);
    CHECK(replay.replayMirrorProducerFunction ==
          kPrStage1ScorerDirectFn801C8660);
    CHECK(replay.dword901BCPublishedCount == seed.count800901BC);
}

void TestTransitionTwoSetupRequestsOnlyReplayRestore1681C() {
    PrStage1ScorerDirectReplayBufferState replay{};
    CHECK(PrStage1ScorerDirectInitializeReplayMirrorFromStartupZero80028590(
        replay));

    PrStage1ScorerDirectResolvedReplayBackup1681C backup{};
    backup.payloadBackupValid = true;
    backup.source =
        PrStage1ScorerDirectReplayRestoreSource1681C::Payload92F48_92F5C;
    backup.backup.valid = true;
    backup.backup.dword92F48PublishedCount = 2u;
    backup.backup.replayMirrorFullBackingKnown8008EEF8 = true;
    backup.backup.dwordEEF8Tick96[0] = 111u;
    backup.backup.dwordEEFCClassMask[0] = 0x40u;
    backup.backup.dwordEEF8Tick96[1] = 222u;
    backup.backup.dwordEEFCClassMask[1] = 0x20u;

    const PrStage1ScorerDirectAcceptedSpecialSetupResult setup =
        PrStage1ScorerDirectRunAcceptedSpecialSetupCore24E54_1681C(
            2u,
            backup,
            replay);

    CHECK(setup.setup.restoreReplayBuffer1681CRequested);
    CHECK(!setup.setup.seedStage1EventTable801C8660Requested);
    CHECK(setup.restore.restoreApplied);
    CHECK(setup.restoreSource ==
          PrStage1ScorerDirectReplayRestoreSource1681C::Payload92F48_92F5C);
    CHECK(replay.replayMirrorProducerKnown8008EEF8);
    CHECK(replay.replayMirrorProducerFunction ==
          kPrStage1ScorerDirectFn8001681C);
    CHECK(replay.dword901BCPublishedCount == 2u);
    CHECK(replay.dword901C0WriteCount == 0u);
    CHECK(replay.dwordEEF8Tick96[0] == 111u);
    CHECK(replay.dwordEEFCClassMask[0] == 0x40u);
    CHECK(replay.dwordEEF8Tick96[1] == 222u);
    CHECK(replay.dwordEEFCClassMask[1] == 0x20u);
    CHECK(replay.replayMirrorFullBackingKnown8008EEF8);
    CHECK(replay.replayMirrorKnownByteCount8008EEF8 ==
          kPrStage1ScorerDirectReplayMirrorByteCount);
    CHECK(replay.dwordEEF8Tick96[599] == 0u);
    CHECK(replay.dwordEEFCClassMask[599] == 0u);
}

void TestTamperedSealedReplayCannotBeRemintedBy24E54() {
    const PrStage1ScorerDirectReplayBufferState sealed =
        MakeSealedStage1EventTableReplay();

    PrStage1ScorerDirectReplayBufferState tamperedBytes = sealed;
    tamperedBytes.dwordEEF8Tick96[7] ^= 0x01010101u;
    CheckTamperedReplayStaysInvalidAfter24E54(tamperedBytes);

    PrStage1ScorerDirectReplayBufferState tamperedProducer = sealed;
    tamperedProducer.replayMirrorProducerFunction =
        kPrStage1ScorerDirectFn80014614;
    CheckTamperedReplayStaysInvalidAfter24E54(tamperedProducer);

    PrStage1ScorerDirectReplayBufferState tamperedFullBacking = sealed;
    tamperedFullBacking.replayMirrorFullBackingKnown8008EEF8 = false;
    CheckTamperedReplayStaysInvalidAfter24E54(tamperedFullBacking);

    PrStage1ScorerDirectReplayBufferState tamperedCursor = sealed;
    tamperedCursor.dword901C0WriteCount += 1u;
    CheckTamperedReplayStaysInvalidAfter24E54(tamperedCursor);
}

void TestZeroCount8001681CCannotRemintInvalidReplay() {
    PrStage1ScorerDirectReplayBackupState zeroCountBackup{};
    zeroCountBackup.valid = true;
    zeroCountBackup.dword92F48PublishedCount = 0u;
    zeroCountBackup.replayMirrorFullBackingKnown8008EEF8 = true;
    zeroCountBackup.dwordEEF8Tick96[0] = 0xDEADBEEFu;
    zeroCountBackup.dwordEEFCClassMask[0] = 0xCAFEBABEu;
    const PrStage1ScorerDirectResolvedReplayBackup1681C resolved =
        PrStage1ScorerDirectResolveReplayRestoreSource1681C(
            true,
            zeroCountBackup,
            false,
            PrStage1ScorerDirectReplayBackupState{});
    CHECK(resolved.source ==
          PrStage1ScorerDirectReplayRestoreSource1681C::Payload92F48_92F5C);

    const PrStage1ScorerDirectReplayBufferState sealed =
        MakeSealedStage1EventTableReplay();
    PrStage1ScorerDirectReplayBufferState authorizedReplay = sealed;
    const PrStage1ScorerDirectReplayBufferState authorizedBefore =
        authorizedReplay;
    const PrStage1ScorerDirectAcceptedSpecialSetupResult authorizedSetup =
        PrStage1ScorerDirectRunAcceptedSpecialSetupCore24E54_1681C(
            2u,
            resolved,
            authorizedReplay);
    CHECK(!authorizedSetup.restore.restoreApplied);
    CHECK(authorizedSetup.restore.scriptedWriterResetRequired);
    CHECK(authorizedReplay.dwordEEF8Tick96 ==
          authorizedBefore.dwordEEF8Tick96);
    CHECK(authorizedReplay.dwordEEFCClassMask ==
          authorizedBefore.dwordEEFCClassMask);
    CHECK(authorizedReplay.replayMirrorProducerKnown8008EEF8 ==
          authorizedBefore.replayMirrorProducerKnown8008EEF8);
    CHECK(authorizedReplay.replayMirrorProducerFunction ==
          authorizedBefore.replayMirrorProducerFunction);
    CHECK(authorizedReplay.replayMirrorFullBackingKnown8008EEF8 ==
          authorizedBefore.replayMirrorFullBackingKnown8008EEF8);
    CHECK(authorizedReplay.dword901BCPublishedCount == 0u);
    CHECK(authorizedReplay.dword901C0WriteCount == 0u);
    CHECK(PrStage1ScorerDirectReplayMirrorAuthorityMatchesState(
        authorizedReplay));
    CHECK(authorizedReplay.replayMirrorAuthority.Kind() ==
          PrStage1ScorerDirectReplayMirrorAuthorityKind::
              Stage1EventTable801C8660);

    PrStage1ScorerDirectReplayBufferState replay = sealed;
    replay.dwordEEFCClassMask[9] ^= 0x00000001u;
    CHECK(!PrStage1ScorerDirectReplayMirrorAuthorityMatchesState(replay));
    const PrStage1ScorerDirectReplayBufferState invalidBefore = replay;

    const PrStage1ScorerDirectAcceptedSpecialSetupResult setup =
        PrStage1ScorerDirectRunAcceptedSpecialSetupCore24E54_1681C(
            2u,
            resolved,
            replay);

    CHECK(setup.setup.restoreReplayBuffer1681CRequested);
    CHECK(!setup.setup.seedStage1EventTable801C8660Requested);
    CHECK(!setup.restore.restoreApplied);
    CHECK(setup.restore.scriptedWriterResetRequired);
    CHECK(replay.dword901BCPublishedCount == 0u);
    CHECK(replay.dword901C0WriteCount == 0u);
    CHECK(replay.dwordEEF8Tick96 == invalidBefore.dwordEEF8Tick96);
    CHECK(replay.dwordEEFCClassMask == invalidBefore.dwordEEFCClassMask);
    CHECK(replay.replayMirrorProducerKnown8008EEF8 ==
          invalidBefore.replayMirrorProducerKnown8008EEF8);
    CHECK(replay.replayMirrorProducerFunction ==
          invalidBefore.replayMirrorProducerFunction);
    CHECK(replay.replayMirrorFullBackingKnown8008EEF8 ==
          invalidBefore.replayMirrorFullBackingKnown8008EEF8);
    CHECK(!PrStage1ScorerDirectReplayMirrorAuthorityMatchesState(replay));
    CHECK(replay.replayMirrorAuthority.Kind() ==
          PrStage1ScorerDirectReplayMirrorAuthorityKind::Unknown);

    const PrStage1ScorerDirectReplayBackupCaptureResult capture =
        PrStage1ScorerDirectRunReplayBackupCaptureCore1635C(replay);
    CHECK(!capture.captureApplied);
    CHECK(!capture.backup.valid);
}

void TestProcessEpochReentryPreservesWriterAndCannotRemintStartupZero() {
    const PrStage1ScorerDirectReplayBufferState writerReplay =
        MakeSealedStage1EventTableReplay();

    PrScn1::Stage1NumericRuntimeState runtime{};
    auto& persistent = runtime.acceptedProducerReplayBuffer;
    persistent.startupZeroInitializationConsumed80028590 = true;
    persistent.replayMirrorAuthority = writerReplay.replayMirrorAuthority;
    persistent.replayMirrorKnown8008EEF8 =
        writerReplay.replayMirrorKnown8008EEF8;
    persistent.replayMirrorStartupZeroAuthorityKnown80028590 =
        writerReplay.replayMirrorStartupZeroAuthorityKnown80028590;
    persistent.replayMirrorProducerKnown8008EEF8 =
        writerReplay.replayMirrorProducerKnown8008EEF8;
    persistent.replayMirrorProducerFunction =
        writerReplay.replayMirrorProducerFunction;
    persistent.replayMirrorByteCountKnown8008EEF8 =
        writerReplay.replayMirrorByteCountKnown8008EEF8;
    persistent.replayMirrorKnownByteCount8008EEF8 =
        writerReplay.replayMirrorKnownByteCount8008EEF8;
    persistent.replayMirrorFullBackingKnown8008EEF8 =
        writerReplay.replayMirrorFullBackingKnown8008EEF8;
    persistent.writeCount901C0 = writerReplay.dword901C0WriteCount;
    persistent.publishedCount901BC = writerReplay.dword901BCPublishedCount;
    persistent.tick96EEF8 = writerReplay.dwordEEF8Tick96;
    persistent.classMaskEEFC = writerReplay.dwordEEFCClassMask;
    const auto before = persistent;

    runtime.psxEventStreamFlagKnown = true;
    runtime.psxEventStreamIdKnown = true;
    runtime.ctx52ReplayMode7A60 = true;
    PrScn1::ResetStage1NumericRuntimeStatePreservingProcessReplay80028590(
        runtime);
    CHECK(!PrScn1::
              Stage1AcceptedProducerReplayStartupZeroInitializationRequired80028590(
                  runtime));
    bool startupZeroBuildAttempted = false;
    bool startupZeroStoreAttempted = false;
    PrScn1::
        InitializeStage1AcceptedProducerReplayMirrorFromStartupZero80028590(
            runtime,
            [&startupZeroBuildAttempted](
                const PrScn1::Stage1NumericRuntimeState&) {
                startupZeroBuildAttempted = true;
                return PrStage1ScorerDirectReplayBufferState{};
            },
            [&startupZeroStoreAttempted](
                PrScn1::Stage1NumericRuntimeState&,
                const PrStage1ScorerDirectReplayBufferState&) {
                startupZeroStoreAttempted = true;
            });
    CHECK(!startupZeroBuildAttempted);
    CHECK(!startupZeroStoreAttempted);

    const auto& after = runtime.acceptedProducerReplayBuffer;
    CHECK(after.startupZeroInitializationConsumed80028590);
    CHECK(after.replayMirrorAuthority.Kind() ==
          before.replayMirrorAuthority.Kind());
    CHECK(after.replayMirrorKnown8008EEF8 ==
          before.replayMirrorKnown8008EEF8);
    CHECK(after.replayMirrorStartupZeroAuthorityKnown80028590 ==
          before.replayMirrorStartupZeroAuthorityKnown80028590);
    CHECK(after.replayMirrorProducerKnown8008EEF8 ==
          before.replayMirrorProducerKnown8008EEF8);
    CHECK(after.replayMirrorProducerFunction ==
          kPrStage1ScorerDirectFn801C8660);
    CHECK(after.replayMirrorByteCountKnown8008EEF8 ==
          before.replayMirrorByteCountKnown8008EEF8);
    CHECK(after.replayMirrorKnownByteCount8008EEF8 ==
          before.replayMirrorKnownByteCount8008EEF8);
    CHECK(after.replayMirrorFullBackingKnown8008EEF8 ==
          before.replayMirrorFullBackingKnown8008EEF8);
    CHECK(after.writeCount901C0 == before.writeCount901C0);
    CHECK(after.publishedCount901BC == before.publishedCount901BC);
    CHECK(after.tick96EEF8 == before.tick96EEF8);
    CHECK(after.classMaskEEFC == before.classMaskEEFC);
    PrStage1ScorerDirectReplayBufferState restored{};
    restored.replayMirrorAuthority = after.replayMirrorAuthority;
    restored.replayMirrorKnown8008EEF8 = after.replayMirrorKnown8008EEF8;
    restored.replayMirrorStartupZeroAuthorityKnown80028590 =
        after.replayMirrorStartupZeroAuthorityKnown80028590;
    restored.replayMirrorProducerKnown8008EEF8 =
        after.replayMirrorProducerKnown8008EEF8;
    restored.replayMirrorProducerFunction = after.replayMirrorProducerFunction;
    restored.replayMirrorByteCountKnown8008EEF8 =
        after.replayMirrorByteCountKnown8008EEF8;
    restored.replayMirrorKnownByteCount8008EEF8 =
        after.replayMirrorKnownByteCount8008EEF8;
    restored.replayMirrorFullBackingKnown8008EEF8 =
        after.replayMirrorFullBackingKnown8008EEF8;
    restored.dword901C0WriteCount = after.writeCount901C0;
    restored.dword901BCPublishedCount = after.publishedCount901BC;
    restored.dwordEEF8Tick96 = after.tick96EEF8;
    restored.dwordEEFCClassMask = after.classMaskEEFC;
    CHECK(PrStage1ScorerDirectReplayMirrorAuthorityMatchesState(restored));
    CHECK(!runtime.psxEventStreamFlagKnown);
    CHECK(!runtime.psxEventStreamIdKnown);
    CHECK(!runtime.ctx52ReplayMode7A60);
    CHECK(!PrScn1::
              Stage1AcceptedProducerReplayStartupZeroInitializationRequired80028590(
                  runtime));
}

} // namespace

int main() {
    TestRejectsNonExactSources();
    TestRejectsWrongShape();
    TestRejectsCountGaps();
    TestAcceptedObservationRemainsDiagnosticOnly();
    TestNaturalProducerCannotPromoteDiagnosticObservationBacking();
    TestDiscFullbootPublisherRemainsDiagnosticOnly();
    TestIdaStartupZeroThenQ607AppendPublishes80014614Backing();
    TestTransitionZeroSetupKeepsIdaStartupBackingWithoutProducer();
    TestTransitionOneSetupRequestsOnlyStage1EventTableSeed();
    TestTransitionTwoSetupRequestsOnlyReplayRestore1681C();
    TestTamperedSealedReplayCannotBeRemintedBy24E54();
    TestZeroCount8001681CCannotRemintInvalidReplay();
    TestProcessEpochReentryPreservesWriterAndCannotRemintStartupZero();

    if (g_failedChecks != 0) {
        std::printf(
            "test_ss0_stage1_replay_mirror_observation_contract: failed checks=%d\n",
            g_failedChecks);
        return 1;
    }
    std::printf("test_ss0_stage1_replay_mirror_observation_contract: ok\n");
    return 0;
}
