#include "pr/pr_stage1_scorer_direct.h"

#include <cstdio>

namespace {

int g_failed = 0;

#define CHECK(expr)                                                           \
    do {                                                                      \
        if (!(expr)) {                                                        \
            std::printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr);       \
            ++g_failed;                                                       \
        }                                                                     \
    } while (0)

struct FakeSourceRuntime {
    PrStage1ScorerDirectSourceCellHeader header{};
    PrStage1ScorerDirectSourceCell cell{};
};

bool ReadTimingTemplateState(void*, uint8_t selectorByte1, uint8_t slot48,
                             uint8_t& outState) {
    if (selectorByte1 != 6u) {
        return false;
    }
    outState = (slot48 == 3u || slot48 == 6u) ? 2u : 0u;
    return outState != 0u;
}

bool ReadSourceCellHeader(void* userData, uint8_t selectorByte0,
                          uint8_t classToken20,
                          PrStage1ScorerDirectSourceCellHeader& outHeader) {
    FakeSourceRuntime& runtime = *static_cast<FakeSourceRuntime*>(userData);
    if (selectorByte0 != 3u || classToken20 != 3u) {
        return false;
    }
    outHeader = runtime.header;
    return outHeader.valid;
}

bool ReadSourceCell(void* userData, uint32_t sourceCellPtr,
                    PrStage1ScorerDirectSourceCell& outCell) {
    FakeSourceRuntime& runtime = *static_cast<FakeSourceRuntime*>(userData);
    if (sourceCellPtr != runtime.header.dword04BasePtr) {
        return false;
    }
    outCell = runtime.cell;
    return outCell.valid;
}

PrStage1ScorerDirectAcceptedProducerAccessors MakeAccessors(
    FakeSourceRuntime& runtime) {
    PrStage1ScorerDirectAcceptedProducerAccessors accessors{};
    accessors.userData = &runtime;
    accessors.readTimingTemplateState = ReadTimingTemplateState;
    accessors.readSourceCellHeader = ReadSourceCellHeader;
    accessors.readSourceCell = ReadSourceCell;
    return accessors;
}

PrStage1ScorerDirectDescriptorRow MakePage9Class3Descriptor() {
    PrStage1ScorerDirectDescriptorRow row{};
    row.valid = true;
    row.byte12DefaultSelector0 = 3u;
    row.byte13DefaultSelector1 = 6u;
    return row;
}

PrStage1ScorerDirectAcceptedProducerCoreInput MakeAcceptedInput(
    const PrStage1ScorerDirectDescriptorRow& row,
    int32_t acceptedTick96) {
    PrStage1ScorerDirectAcceptedProducerCoreInput in{};
    in.eventStreamFlagActive = true;
    in.writerControlSample18 = 0x0040u;
    in.classToken20Known = true;
    in.classToken20 = 3u;
    in.acceptedTick96Known = true;
    in.acceptedTick96 = acceptedTick96;
    in.halfWindow34 = 8u;
    in.descriptorSubstate50 = 0u;
    in.writePageOrdinal38 = 9;
    in.lookaheadDescriptorRow = &row;
    return in;
}

FakeSourceRuntime MakeSourceRuntime() {
    FakeSourceRuntime runtime{};
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

void CheckPage9SlotWrite(const PrStage1ScorerDirectAcceptedProducerRunResult& run,
                         uint8_t expectedSlot,
                         uint8_t expectedRemainder,
                         uint8_t expectedTemplateSlot48) {
    CHECK(run.resultCode == 0);
    CHECK(run.replayAppendRan);
    CHECK(run.writeRan);
    CHECK(run.writeResult.recordWritten);
    CHECK(run.pageStorageApply.recordedWriteApplied);
    CHECK(run.resolved.acceptedTick96Known);
    CHECK(run.resolved.remappedWriterControl18 == 0x0040u);
    CHECK(run.resolved.classToken20 == 3u);
    CHECK(run.resolved.phase384 ==
          static_cast<uint16_t>(expectedSlot * 24u + expectedRemainder));
    CHECK(run.resolved.recordSlot24 == expectedSlot);
    CHECK(run.resolved.recordRemainder24 == expectedRemainder);
    CHECK(run.resolved.timingTemplateSlot48 == expectedTemplateSlot48);
    CHECK(run.resolved.writeback.dword38WritePageOrdinal == 9);
    CHECK(run.resolved.writeback.byte24RecordSlot == expectedSlot);
    CHECK(run.resolved.writeback.dword18AcceptedClassMask == 0x0040u);
    CHECK(run.resolved.packet.dword08SourceCellPtr == 0x801CCCF4u);
    CHECK(run.pageStorageApply.writePageOrdinal == 9);
    CHECK(run.pageStorageApply.recordSlot == expectedSlot);
    CHECK(run.pageStorageApply.acceptedMask == 0x0040u);
    CHECK(run.pageStorageApply.pageCompanion == 3u);
    CHECK(run.pageStorageApply.occupiedCount == 1u);
    CHECK(run.pageStorageApply.sourceCellPtr == 0x801CCCF4u);
}

void TestQ604AndQ628ShapeWritesPage9Slots7And13() {
    PrStage1ScorerDirectGlobals globals{};
    PrStage1ScorerDirectAcceptedProducerOwnerState ownerState{};
    PrStage1ScorerDirectReplayBufferState replay{};
    FakeSourceRuntime runtime = MakeSourceRuntime();
    const PrStage1ScorerDirectAcceptedProducerAccessors accessors =
        MakeAccessors(runtime);
    const PrStage1ScorerDirectDescriptorRow row =
        MakePage9Class3Descriptor();

    PrStage1ScorerDirectAcceptedProducerRunResult slot7Run =
        PrStage1ScorerDirectRunAcceptedProducer14614(
            globals,
            ownerState,
            replay,
            MakeAcceptedInput(row, 174),
            accessors);
    CheckPage9SlotWrite(slot7Run, 7u, 14u, 3u);
    CHECK(replay.dword901C0WriteCount == 1u);
    CHECK(replay.dword901BCPublishedCount == 1u);
    CHECK(replay.dwordEEF8Tick96[0] == 174u);
    CHECK(replay.dwordEEFCClassMask[0] == 0x0040u);
    CHECK(globals.ringPages[1].records[7].dword00AcceptedMask == 0x0040u);
    CHECK(globals.ringPages[1].records[7].word04Companion == 3u);
    CHECK(globals.ringPages[1].records[7].word06Occupied == 1u);
    CHECK(globals.ringPages[1].records[7].dword08Payload == 0x801CCCF4u);

    PrStage1ScorerDirectAcceptedProducerRunResult slot13Run =
        PrStage1ScorerDirectRunAcceptedProducer14614(
            globals,
            ownerState,
            replay,
            MakeAcceptedInput(row, 315),
            accessors);
    CheckPage9SlotWrite(slot13Run, 13u, 11u, 6u);
    CHECK(replay.dword901C0WriteCount == 2u);
    CHECK(replay.dword901BCPublishedCount == 2u);
    CHECK(replay.dwordEEF8Tick96[1] == 315u);
    CHECK(replay.dwordEEFCClassMask[1] == 0x0040u);
    CHECK(globals.ringPages[1].records[13].dword00AcceptedMask == 0x0040u);
    CHECK(globals.ringPages[1].records[13].word04Companion == 3u);
    CHECK(globals.ringPages[1].records[13].word06Occupied == 1u);
    CHECK(globals.ringPages[1].records[13].dword08Payload == 0x801CCCF4u);
}

void TestKnownWriterPageOwnerOverridesTickDerivedPage() {
    PrStage1ScorerDirectGlobals globals{};
    PrStage1ScorerDirectAcceptedProducerOwnerState ownerState{};
    PrStage1ScorerDirectReplayBufferState replay{};
    FakeSourceRuntime runtime = MakeSourceRuntime();
    const PrStage1ScorerDirectAcceptedProducerAccessors accessors =
        MakeAccessors(runtime);
    const PrStage1ScorerDirectDescriptorRow row =
        MakePage9Class3Descriptor();
    PrStage1ScorerDirectAcceptedProducerCoreInput in =
        MakeAcceptedInput(row, 174);
    in.writePageOrdinal38 = 4;
    globals.currentWritePageOrdinalKnown = true;
    globals.currentWritePageOrdinal1Based = 9u;

    const PrStage1ScorerDirectAcceptedProducerRunResult run =
        PrStage1ScorerDirectRunAcceptedProducer14614(
            globals,
            ownerState,
            replay,
            in,
            accessors);

    CheckPage9SlotWrite(run, 7u, 14u, 3u);
    CHECK(globals.ringPages[1].records[7].word06Occupied == 1u);
    CHECK(globals.ringPages[0].records[7].word06Occupied == 0u);
}

} // namespace

int main() {
    TestQ604AndQ628ShapeWritesPage9Slots7And13();
    TestKnownWriterPageOwnerOverridesTickDerivedPage();

    if (g_failed != 0) {
        std::printf(
            "test_ss0_stage1_accepted_page9_cadence_contract: failed checks=%d\n",
            g_failed);
        return 1;
    }
    std::printf("test_ss0_stage1_accepted_page9_cadence_contract: ok\n");
    return 0;
}
