#include "pr/pr_game_context.h"
#include "pr/pr_pad.h"
#include "pr/pr_stage1_scorer_direct.h"
#include "pr/pr_stage1_save_ui_direct.h"
#include "test_stage_payload_authority_fixture.h"

#include <array>
#include <cstring>
#include <cstdio>

PrPadState PrPad::GetState(int port) {
    (void)port;
    return {};
}

namespace {

int g_failedChecks = 0;

#define CHECK(expr)                                                         \
    do {                                                                    \
        if (!(expr)) {                                                      \
            std::printf("CHECK failed %s:%d: %s\n", __FILE__, __LINE__,    \
                        #expr);                                             \
            ++g_failedChecks;                                               \
        }                                                                   \
    } while (0)

PrStage1SaveStatusPrefix80092F10 MakeSeed80092F10(bool statusBankKnown) {
    PrStage1SaveStatusPrefix80092F10 seed{};
    seed.known = true;
    seed.statusBankKnown80092F1D = statusBankKnown;
    seed.helperGap = false;
    seed.psxAddress = PrStage1SaveStatusPrefix80092F10::kPsxAddress;
    seed.byteCount = PrStage1SaveStatusPrefix80092F10::kByteCount;
    seed.lastWriterFunction = PrStagePayloadBankDirect::kFn800164B4;
    seed.wrote800164B4 = statusBankKnown;
    for (uint32_t i = 0; i < seed.byteCount; ++i) {
        seed.bytes[i] = static_cast<uint8_t>(i & 0xffu);
    }
    return seed;
}

PrStagePayloadBankDirect::LoadSavePayloadAuthority800164B4
MakeRuntimeLowerCardPayloadAuthority800164B4(const uint8_t* payload,
                                             size_t payloadBytes) {
    PrStagePayloadBankDirect::LoadSavePayloadAuthority800164B4 authority{};
    CHECK(TestStagePayloadAuthorityFixture::
              MintRuntimeLowerCardPayloadAuthority800164B4(
                  payload, payloadBytes, &authority));
    return authority;
}

uint32_t ReadLe32Prefix(const PrStage1SaveStatusPrefix80092F10& prefix,
                        uint32_t address) {
    const size_t offset =
        static_cast<size_t>(address - PrStage1SaveStatusPrefix80092F10::kPsxAddress);
    return static_cast<uint32_t>(prefix.bytes[offset]) |
           (static_cast<uint32_t>(prefix.bytes[offset + 1u]) << 8u) |
           (static_cast<uint32_t>(prefix.bytes[offset + 2u]) << 16u) |
           (static_cast<uint32_t>(prefix.bytes[offset + 3u]) << 24u);
}

PrStage1ScorerDirectReplayBufferState MakeFullReplayRestoreProducer() {
    PrStage1ScorerDirectReplayBufferState replay{};
    CHECK(PrStage1ScorerDirectInitializeReplayMirrorFromStartupZero80028590(
        replay));
    const PrStage1ScorerDirectEventTableBuild801C8660Result producer =
        PrStage1ScorerDirectBuildStage1EventTable801C8660(replay);
    CHECK(producer.applied);
    CHECK(producer.count800901BC == 53u);
    CHECK(PrStage1ScorerDirectReplayMirrorAuthorityMatchesState(replay));
    CHECK(replay.replayMirrorKnown8008EEF8);
    CHECK(replay.replayMirrorProducerKnown8008EEF8);
    CHECK(replay.replayMirrorProducerFunction == kPrStage1ScorerDirectFn801C8660);
    CHECK(replay.replayMirrorByteCountKnown8008EEF8);
    CHECK(replay.replayMirrorKnownByteCount8008EEF8 ==
          kPrStage1ScorerDirectReplayMirrorByteCount);
    CHECK(replay.replayMirrorFullBackingKnown8008EEF8);
    return replay;
}

bool HasReplayMirrorCopyGap(const PrStage1SavePayloadProducerResult& result) {
    for (uint32_t i = 0; i < result.actions.count; ++i) {
        const PrStage1SaveUi19148Action& action = result.actions.actions[i];
        if (action.kind == PrStage1SaveUi19148ActionKind::HelperGap &&
            action.psxFunction == 0x80025C64u &&
            static_cast<uint32_t>(action.arg0) ==
                PrStagePayloadBankDirect::kMirrorSrcAddress8008EEF8 &&
            static_cast<uint32_t>(action.arg1) ==
                PrStagePayloadBankDirect::kMirrorDstAddress80092F5C &&
            action.arg2 == PrStagePayloadBankDirect::kMirrorBytes8001635C) {
            return true;
        }
    }
    return false;
}

bool ReadAppendTimingTemplateState(void* userData,
                                   uint8_t selectorByte1,
                                   uint8_t slot48,
                                   uint8_t& outState) {
    (void)userData;
    (void)selectorByte1;
    (void)slot48;
    outState = 2u;
    return true;
}

bool ReadAppendSourceCellHeader(void* userData,
                                uint8_t selectorByte0,
                                uint8_t classToken20,
                                PrStage1ScorerDirectSourceCellHeader& outHeader) {
    (void)userData;
    (void)selectorByte0;
    (void)classToken20;
    outHeader.valid = true;
    outHeader.dword00HeaderAddr = 0x801D0000u;
    outHeader.dword04BasePtr = 0x801D1000u;
    outHeader.word08Count = 1u;
    outHeader.word0ACursor = 0u;
    return true;
}

bool ReadAppendSourceCell(void* userData,
                          uint32_t sourceCellPtr,
                          PrStage1ScorerDirectSourceCell& outCell) {
    (void)userData;
    outCell.valid = true;
    outCell.dword00SourceCellPtr = sourceCellPtr;
    outCell.byte00Program = 1u;
    outCell.byte01Note = 2u;
    outCell.byte02Key = 3u;
    outCell.byte03Volume = 4u;
    outCell.word06RecordCompanion = 5u;
    outCell.dword08CallbackArgPresent = true;
    outCell.dword08PayloadOpaque = 0x11223344u;
    return true;
}

PrStage1ScorerDirectDescriptorRow MakeAppendDescriptorRow() {
    PrStage1ScorerDirectDescriptorRow row{};
    row.valid = true;
    row.byte00LessonId = 1u;
    row.byte12DefaultSelector0 = 1u;
    row.byte13DefaultSelector1 = 1u;
    row.defaultBranch.byte02RequiredClassToken = 1u;
    row.defaultBranch.dword08RequiredMask = 0x10u;
    return row;
}

PrStage1ScorerDirectAcceptedProducerCoreInput MakeAppendInput(
    const PrStage1ScorerDirectDescriptorRow& row,
    int32_t tick96) {
    PrStage1ScorerDirectAcceptedProducerCoreInput in{};
    in.eventStreamFlagActive = true;
    in.writerControlSample18 = 0x10u;
    in.classToken20Known = true;
    in.classToken20 = 1u;
    in.acceptedTick96Known = true;
    in.acceptedTick96 = tick96;
    in.halfWindow34 = 12u;
    in.writePageOrdinal38 = tick96 / 24;
    in.lookaheadDescriptorRow = &row;
    return in;
}

void TestColdBootStatusSeedPublishesFromIdaStartupZero() {
    PrStage1SaveUiDirect::Reset19148();
    PrStage1ColdBootStatusSeedContext800154F4 bootContext{};

    const PrStage1SavePayloadProducerResult result =
        PrStage1SaveUiDirect::SeedColdBootStatusPrefix800154F4(bootContext);
    CHECK(result.ok);
    CHECK(result.payloadKnown);
    CHECK(!result.helperGap);
    CHECK(!HasReplayMirrorCopyGap(result));

    const PrStage1SavePayloadBankRuntimeSnapshot runtime =
        PrStage1SaveUiDirect::GetSavePayloadBankRuntimeSnapshot();
    CHECK(runtime.payloadKnown);
    CHECK(runtime.statusBankKnown80092F1D);
    CHECK(!runtime.helperGap);
    CHECK(runtime.savePayloadSourceKnown);
    CHECK(runtime.seedColdBootAttempted);
    CHECK(runtime.seedColdBootOk);
    CHECK(runtime.seedColdBootStartupZeroAccepted);
    CHECK(runtime.sub8001635CAttempted);
    CHECK(runtime.sub8001635COk);
    CHECK(runtime.sub8001635CPreflightPayloadKnown);
    CHECK(runtime.sub8001635CPreflightStatusBankKnown);
    CHECK(runtime.sub8001635CPreflightMapped);
    CHECK(runtime.sub8001635CPreflightCarrierSourceKnown);
    CHECK(runtime.sub8001635CPreflightMirrorSourceKnown);
    CHECK(!runtime.sub8001635CPreflightReplayMirrorProducerSourceKnown);
    CHECK(runtime.sub8001635CPreflightStartupZeroSourceKnown);
    CHECK(runtime.sub8001635CReplayMirrorSourceKnownAtEntry);
    CHECK(runtime.sub8001635CReplayMirrorSourceShapeKnownAtEntry);
    CHECK(runtime.replayMirrorAuthorityKnown8001635C);
    CHECK(runtime.sub8001635CScratchAuthorityKnown);
    CHECK(runtime.sub8001635CMirrorCopied);
    CHECK(runtime.sub8001635CAllClearQueried);
    CHECK(runtime.sub8001635CAllClearWritten);
    CHECK(runtime.lastWriterFunction == PrStagePayloadBankDirect::kFn8001635C);
    CHECK(runtime.wrote8001635C);

    const PrStage1SaveStatusPrefix80092F10 snapshot =
        PrStage1SaveUiDirect::GetSaveStatusPrefix80092F10();
    CHECK(snapshot.known);
    CHECK(snapshot.statusBankKnown80092F1D);
    CHECK(!snapshot.helperGap);
    CHECK(snapshot.lastWriterFunction == PrStagePayloadBankDirect::kFn8001635C);
    CHECK(!snapshot.wrote80015CC4);
    CHECK(snapshot.wrote8001635C);
    CHECK(snapshot.bytes[PrStagePayloadBankDirect::kStatusBaseAddress80092F1D -
                         PrStage1SaveStatusPrefix80092F10::kPsxAddress] == 1u);
    CHECK(ReadLe32Prefix(snapshot,
                         PrStagePayloadBankDirect::kMirrorDstAddress80092F5C) ==
          0u);
    CHECK(ReadLe32Prefix(snapshot,
                         PrStagePayloadBankDirect::kMirrorDstAddress80092F5C +
                             sizeof(uint32_t)) == 0u);

    const PrStage1SavePayloadProducerResult repeated =
        PrStage1SaveUiDirect::SeedColdBootStatusPrefix800154F4(bootContext);
    CHECK(repeated.ok);
    CHECK(repeated.payloadKnown);
    CHECK(!repeated.helperGap);
    CHECK(!HasReplayMirrorCopyGap(repeated));
    const PrStage1SavePayloadBankRuntimeSnapshot repeatedRuntime =
        PrStage1SaveUiDirect::GetSavePayloadBankRuntimeSnapshot();
    CHECK(repeatedRuntime.seedColdBootOk);
    CHECK(!repeatedRuntime.seedColdBootStartupZeroAccepted);
    CHECK(repeatedRuntime.sub8001635CPreflightMirrorSourceKnown);
    CHECK(repeatedRuntime.sub8001635CPreflightStartupZeroSourceKnown);
    CHECK(repeatedRuntime.wrote8001635C);
}

void TestColdBootStatusSeedPublishesWithFullReplayMirrorProducer() {
    PrStage1SaveUiDirect::Reset19148();
    PrStage1ColdBootStatusSeedContext800154F4 bootContext{};
    PrStage1ScorerDirectReplayBufferState replay =
        MakeFullReplayRestoreProducer();
    PrStage1SaveUiDirect::PublishAuthoritativeReplayMirrorSourceFromStage1(replay);

    const PrStage1SavePayloadProducerResult result =
        PrStage1SaveUiDirect::SeedColdBootStatusPrefix800154F4(bootContext);
    CHECK(result.ok);
    CHECK(result.payloadKnown);
    CHECK(!result.helperGap);

    const PrStage1SavePayloadBankRuntimeSnapshot runtime =
        PrStage1SaveUiDirect::GetSavePayloadBankRuntimeSnapshot();
    CHECK(runtime.payloadKnown);
    CHECK(runtime.statusBankKnown80092F1D);
    CHECK(!runtime.helperGap);
    CHECK(runtime.savePayloadSourceKnown);
    CHECK(runtime.seedColdBootAttempted);
    CHECK(runtime.seedColdBootOk);
    CHECK(runtime.seedColdBootResult == result.result);
    CHECK(runtime.sub8001635CAttempted);
    CHECK(runtime.sub8001635COk);
    CHECK(runtime.sub8001635CResult == result.result);
    CHECK(runtime.sub8001635CPreflightPayloadKnown);
    CHECK(runtime.sub8001635CPreflightStatusBankKnown);
    CHECK(runtime.sub8001635CPreflightMapped);
    CHECK(runtime.sub8001635CPreflightCarrierSourceKnown);
    CHECK(runtime.sub8001635CPreflightMirrorSourceKnown);
    CHECK(runtime.sub8001635CPreflightReplayMirrorProducerSourceKnown);
    CHECK(!runtime.sub8001635CPreflightStartupZeroSourceKnown);
    CHECK(!runtime.seedColdBootStartupZeroAccepted);
    CHECK(runtime.replayMirrorSourceKnown8008EEF8);
    CHECK(runtime.replayMirrorSourceShapeKnown8008EEF8);
    CHECK(runtime.replayMirrorSourceProducerKnown8008EEF8);
    CHECK(runtime.replayMirrorSourceFullBackingKnown8008EEF8);
    CHECK(runtime.replayMirrorSourceHydrateCount >= 1u);
    CHECK(runtime.sub8001635CReplayMirrorSourceKnownAtEntry);
    CHECK(runtime.sub8001635CReplayMirrorSourceShapeKnownAtEntry);
    CHECK(runtime.sub8001635CReplayMirrorSourceSetCountAtEntry ==
          runtime.replayMirrorSourceSetCount);
    CHECK(runtime.sub8001635CReplayMirrorSourceHydrateCountAtEntry ==
          runtime.replayMirrorSourceHydrateCount);
    CHECK(runtime.sub8001635CScratchAuthorityKnown);
    CHECK(runtime.sub8001635CMirrorCopied);
    CHECK(runtime.sub8001635CAllClearQueried);
    CHECK(runtime.sub8001635CAllClearWritten);
    CHECK(runtime.lastWriterFunction == PrStagePayloadBankDirect::kFn8001635C);
    CHECK(runtime.wrote8001635C);

    const PrStage1SaveStatusPrefix80092F10 snapshot =
        PrStage1SaveUiDirect::GetSaveStatusPrefix80092F10();
    CHECK(snapshot.known);
    CHECK(snapshot.statusBankKnown80092F1D);
    CHECK(snapshot.lastWriterFunction == PrStagePayloadBankDirect::kFn8001635C);
    CHECK(snapshot.wrote8001635C);
    CHECK(ReadLe32Prefix(snapshot,
                         PrStagePayloadBankDirect::kMirrorDstAddress80092F5C) ==
          24u * 120u);
    CHECK(ReadLe32Prefix(snapshot,
                         PrStagePayloadBankDirect::kMirrorDstAddress80092F5C +
                             sizeof(uint32_t)) == 0x10u);
}

void TestColdBootStartupZeroPersistsThroughFirstSub80015CC4() {
    PrStage1SaveUiDirect::Reset19148();
    PrStage1SaveUiDirect::PublishAuthoritativeReplayMirrorSourceFromStage1(
        PrStage1ScorerDirectReplayBufferState{});
    PrStage1ColdBootStatusSeedContext800154F4 bootContext{};
    CHECK(PrStage1SaveUiDirect::SeedColdBootStatusPrefix800154F4(bootContext)
              .ok);

    const PrStage1SavePayloadProducerResult initialized =
        PrStage1SaveUiDirect::Sub80015CC4();
    CHECK(initialized.ok);
    CHECK(initialized.payloadKnown);
    CHECK(!initialized.helperGap);
    const PrStage1SavePayloadBankRuntimeSnapshot runtime =
        PrStage1SaveUiDirect::GetSavePayloadBankRuntimeSnapshot();
    CHECK(runtime.sub80015CC4Attempted);
    CHECK(runtime.sub80015CC4Ok);
    CHECK(runtime.sub8001635COk);
    CHECK(runtime.sub8001635CPreflightStartupZeroSourceKnown);
    CHECK(runtime.replayMirrorSourceStartupZeroAuthorityKnown80028590);
    const PrStage1SaveStatusPrefix80092F10 snapshot =
        PrStage1SaveUiDirect::GetSaveStatusPrefix80092F10();
    CHECK(snapshot.replayPayloadBackingProvenance80092F5C ==
          PrStagePayloadBankDirect::ReplayPayloadBackingProvenance80092F5C::
              ReplayMirrorCopy8001635C);
    CHECK(ReadLe32Prefix(
              snapshot,
              PrStagePayloadBankDirect::kCarrierSourceAddress80092F48) == 0u);
    CHECK(ReadLe32Prefix(
              snapshot,
              PrStagePayloadBankDirect::kMirrorDstAddress80092F5C) == 0u);
}

void TestColdBootNaturalProducerConsumesStartupZeroOpportunity() {
    PrStage1SaveUiDirect::Reset19148();
    PrStage1ColdBootStatusSeedContext800154F4 bootContext{};
    PrStage1ScorerDirectReplayBufferState replay =
        MakeFullReplayRestoreProducer();
    PrStage1SaveUiDirect::PublishAuthoritativeReplayMirrorSourceFromStage1(replay);

    const PrStage1SavePayloadProducerResult natural =
        PrStage1SaveUiDirect::SeedColdBootStatusPrefix800154F4(bootContext);
    CHECK(natural.ok);
    const PrStage1SavePayloadBankRuntimeSnapshot naturalRuntime =
        PrStage1SaveUiDirect::GetSavePayloadBankRuntimeSnapshot();
    CHECK(naturalRuntime.sub8001635CPreflightReplayMirrorProducerSourceKnown);
    CHECK(!naturalRuntime.sub8001635CPreflightStartupZeroSourceKnown);
    CHECK(!naturalRuntime.seedColdBootStartupZeroAccepted);

    PrStage1SaveUiDirect::PublishAuthoritativeReplayMirrorSourceFromStage1(
        PrStage1ScorerDirectReplayBufferState{});
    const PrStage1SavePayloadProducerResult repeated =
        PrStage1SaveUiDirect::SeedColdBootStatusPrefix800154F4(bootContext);
    CHECK(!repeated.ok);
    CHECK(repeated.helperGap);
    const PrStage1SavePayloadBankRuntimeSnapshot repeatedRuntime =
        PrStage1SaveUiDirect::GetSavePayloadBankRuntimeSnapshot();
    CHECK(!repeatedRuntime.sub8001635CPreflightReplayMirrorProducerSourceKnown);
    CHECK(!repeatedRuntime.sub8001635CPreflightStartupZeroSourceKnown);
    CHECK(!repeatedRuntime.seedColdBootStartupZeroAccepted);
    CHECK(!repeatedRuntime.wrote8001635C);
}

PrStagePayloadBankDirect::MemoryState80092F10
MakeStartupZeroAuthorityState80028590() {
    PrStage1ScorerDirectReplayBufferState startupZero{};
    CHECK(PrStage1ScorerDirectInitializeReplayMirrorFromStartupZero80028590(
        startupZero));
    PrStagePayloadBankDirect::MemoryState80092F10 state{};
    CHECK(PrStagePayloadBankDirect::ImportAuthoritativeReplayMirror8008EEF8(
        state, startupZero));
    state.savePayloadBankKnown = true;
    state.statusBankKnown80092F1D = true;
    return state;
}

void TestStartupZeroAuthorityRemainsBoundedToStaticShape() {
    PrStagePayloadBankDirect::MemoryState80092F10 forgedShape{};
    forgedShape.replayMirror.fill(0u);
    forgedShape.replayMirrorKnown8008EEF8 = true;
    forgedShape.replayMirrorStartupZeroAuthorityKnown80028590 = true;
    forgedShape.replayMirrorByteCountKnown8008EEF8 = true;
    forgedShape.replayMirrorKnownByteCount8008EEF8 =
        PrStagePayloadBankDirect::kMirrorBytes8001635C;
    forgedShape.replayMirrorFullBackingKnown8008EEF8 = true;
    forgedShape.savePayloadBankKnown = true;
    forgedShape.statusBankKnown80092F1D = true;
    const auto blockedForgedShape =
        PrStagePayloadBankDirect::UpdateSavePayload8001635C(
            forgedShape, 1, 1, 1, 0, true, 0u);
    CHECK(!blockedForgedShape.ok);
    CHECK(!blockedForgedShape.preflightStartupZeroSourceKnown);

    PrStagePayloadBankDirect::MemoryState80092F10 nonZeroMirror =
        MakeStartupZeroAuthorityState80028590();
    nonZeroMirror.replayMirror[0] = 1u;
    const auto blockedNonZeroMirror =
        PrStagePayloadBankDirect::UpdateSavePayload8001635C(
            nonZeroMirror, 1, 1, 1, 0, true, 0u);
    CHECK(!blockedNonZeroMirror.ok);
    CHECK(!blockedNonZeroMirror.preflightStartupZeroSourceKnown);

    PrStagePayloadBankDirect::MemoryState80092F10 missingStaticProvenance =
        MakeStartupZeroAuthorityState80028590();
    missingStaticProvenance.replayMirrorStartupZeroAuthorityKnown80028590 =
        false;
    const auto blockedMissingStaticProvenance =
        PrStagePayloadBankDirect::UpdateSavePayload8001635C(
            missingStaticProvenance, 1, 1, 1, 0, true, 0u);
    CHECK(!blockedMissingStaticProvenance.ok);
    CHECK(!blockedMissingStaticProvenance.preflightStartupZeroSourceKnown);

    PrStagePayloadBankDirect::MemoryState80092F10 nonZeroCarrier =
        MakeStartupZeroAuthorityState80028590();
    const auto blockedNonZeroCarrier =
        PrStagePayloadBankDirect::UpdateSavePayload8001635C(
            nonZeroCarrier, 1, 1, 1, 0, true, 1u);
    CHECK(!blockedNonZeroCarrier.ok);
    CHECK(!blockedNonZeroCarrier.preflightStartupZeroSourceKnown);

    PrStagePayloadBankDirect::MemoryState80092F10 payloadUnknown =
        MakeStartupZeroAuthorityState80028590();
    payloadUnknown.savePayloadBankKnown = false;
    const auto blockedPayload =
        PrStagePayloadBankDirect::UpdateSavePayload8001635C(
            payloadUnknown, 1, 1, 1, 0, true, 0u);
    CHECK(!blockedPayload.ok);
    CHECK(blockedPayload.preflightStartupZeroSourceKnown);
    payloadUnknown.savePayloadBankKnown = true;
    const auto acceptedPayloadRetry =
        PrStagePayloadBankDirect::UpdateSavePayload8001635C(
            payloadUnknown, 1, 1, 1, 0, true, 0u);
    CHECK(acceptedPayloadRetry.ok);
    CHECK(acceptedPayloadRetry.preflightStartupZeroSourceKnown);

    PrStagePayloadBankDirect::MemoryState80092F10 statusUnknown =
        MakeStartupZeroAuthorityState80028590();
    statusUnknown.statusBankKnown80092F1D = false;
    const auto blockedStatus =
        PrStagePayloadBankDirect::UpdateSavePayload8001635C(
            statusUnknown, 1, 1, 1, 0, true, 0u);
    CHECK(!blockedStatus.ok);
    CHECK(blockedStatus.preflightStartupZeroSourceKnown);
    statusUnknown.statusBankKnown80092F1D = true;
    const auto acceptedStatusRetry =
        PrStagePayloadBankDirect::UpdateSavePayload8001635C(
            statusUnknown, 1, 1, 1, 0, true, 0u);
    CHECK(acceptedStatusRetry.ok);
    CHECK(acceptedStatusRetry.preflightStartupZeroSourceKnown);

    PrStagePayloadBankDirect::MemoryState80092F10 carrierUnknown =
        MakeStartupZeroAuthorityState80028590();
    const auto blockedCarrier =
        PrStagePayloadBankDirect::UpdateSavePayload8001635C(
            carrierUnknown, 1, 1, 1, 0, false, 0u);
    CHECK(!blockedCarrier.ok);
    CHECK(!blockedCarrier.preflightStartupZeroSourceKnown);
    const auto acceptedCarrierRetry =
        PrStagePayloadBankDirect::UpdateSavePayload8001635C(
            carrierUnknown, 1, 1, 1, 0, true, 0u);
    CHECK(acceptedCarrierRetry.ok);
    CHECK(acceptedCarrierRetry.preflightStartupZeroSourceKnown);

    PrStagePayloadBankDirect::MemoryState80092F10 exact =
        MakeStartupZeroAuthorityState80028590();
    const auto accepted =
        PrStagePayloadBankDirect::UpdateSavePayload8001635C(
            exact, 1, 1, 1, 0, true, 0u);
    CHECK(accepted.ok);
    CHECK(!accepted.preflightReplayMirrorProducerSourceKnown);
    CHECK(accepted.preflightStartupZeroSourceKnown);
    CHECK(accepted.preflightMirrorSourceKnown);

    const auto acceptedReuse =
        PrStagePayloadBankDirect::UpdateSavePayload8001635C(
            exact, 2, 2, 2, 3, true, 0u);
    CHECK(acceptedReuse.ok);
    CHECK(!acceptedReuse.preflightReplayMirrorProducerSourceKnown);
    CHECK(acceptedReuse.preflightStartupZeroSourceKnown);
    CHECK(acceptedReuse.preflightMirrorSourceKnown);
}

void TestUnknownStatusBankSeedFailsClosedBeforeStart() {
    PrGameContext ctx{};
    PrStage1SaveUiDirect::Reset19148();
    const PrStage1SaveStatusPrefix80092F10 seed = MakeSeed80092F10(false);

    CHECK(!PrStage1SaveUiDirect::Start19148(ctx, &seed));
    CHECK(!PrStage1SaveUiDirect::IsActive19148());

    const PrStage1SaveStatusPrefix80092F10 snapshot =
        PrStage1SaveUiDirect::GetSaveStatusPrefix80092F10();
    CHECK(!snapshot.known);
    CHECK(!snapshot.statusBankKnown80092F1D);
}

void TestKnownStatusBankSeedStartsAndPublishesPrefix() {
    PrGameContext ctx{};
    PrStage1SaveUiDirect::Reset19148();
    const PrStage1SaveStatusPrefix80092F10 seed = MakeSeed80092F10(true);

    CHECK(PrStage1SaveUiDirect::Start19148(ctx, &seed));
    CHECK(PrStage1SaveUiDirect::IsActive19148());

    const PrStage1SaveStatusPrefix80092F10 snapshot =
        PrStage1SaveUiDirect::GetSaveStatusPrefix80092F10();
    CHECK(snapshot.known);
    CHECK(snapshot.statusBankKnown80092F1D);
    CHECK(snapshot.psxAddress == PrStage1SaveStatusPrefix80092F10::kPsxAddress);
    CHECK(snapshot.byteCount == PrStage1SaveStatusPrefix80092F10::kByteCount);
    CHECK(snapshot.lastWriterFunction == PrStagePayloadBankDirect::kFn800164B4);
    CHECK(snapshot.wrote800164B4);
    CHECK(snapshot.bytes[0] == seed.bytes[0]);
    CHECK(snapshot.bytes[0x0Du] == seed.bytes[0x0Du]);
}

void TestSaveStatusThenUnlockSeedPreservesChainAuthority() {
    PrGameContext ctx{};
    PrStage1SaveUiDirect::Reset19148();
    PrStage1SaveStatusPrefix80092F10 seed = MakeSeed80092F10(true);
    seed.lastWriterFunction = PrStagePayloadBankDirect::kFn8001628C;
    seed.wrote8001635C = true;
    seed.wrote8001628C = true;

    CHECK(PrStage1SaveUiDirect::Start19148(ctx, &seed));
    CHECK(PrStage1SaveUiDirect::IsActive19148());

    const PrStage1SaveStatusPrefix80092F10 activeSnapshot =
        PrStage1SaveUiDirect::GetSaveStatusPrefix80092F10();
    CHECK(activeSnapshot.known);
    CHECK(activeSnapshot.statusBankKnown80092F1D);
    CHECK(activeSnapshot.lastWriterFunction == PrStagePayloadBankDirect::kFn8001628C);
    CHECK(activeSnapshot.seedAuthorityFunction ==
          PrStagePayloadBankDirect::kFn8001628C);
    CHECK(activeSnapshot.wrote8001635C);
    CHECK(activeSnapshot.wrote8001628C);
}

void TestNameInputSuffixPreservesSaveStatusSeedAuthority() {
    PrStagePayloadBankDirect::MemoryState80092F10 state{};
    CHECK(PrStagePayloadBankDirect::InitSavePayload80015CC4(state).ok);
    PrStage1ScorerDirectReplayBufferState replay =
        MakeFullReplayRestoreProducer();
    CHECK(PrStagePayloadBankDirect::ImportAuthoritativeReplayMirror8008EEF8(
        state, replay));
    CHECK(PrStagePayloadBankDirect::UpdateSavePayload8001635C(
              state,
              1,
              1,
              1,
              0,
              true,
              replay.dword901BCPublishedCount)
              .ok);
    CHECK(PrStagePayloadBankDirect::EnsureProgress8001628C(state, 1).ok);

    PrStagePayloadBankDirect::MarkPayloadWriter80092F10(state, 0x800185D0u);

    const PrStagePayloadBankDirect::SaveStatusPrefixSnapshot80092F10
        activeSnapshot =
            PrStagePayloadBankDirect::SnapshotSaveStatusPrefix80092F10(state);
    CHECK(activeSnapshot.known);
    CHECK(activeSnapshot.statusBankKnown80092F1D);
    CHECK(activeSnapshot.lastWriterFunction == 0x800185D0u);
    CHECK(activeSnapshot.seedAuthorityFunction ==
          PrStagePayloadBankDirect::kFn8001628C);
    CHECK(activeSnapshot.wrote8001635C);
    CHECK(activeSnapshot.wrote8001628C);
    CHECK(activeSnapshot.replayPayloadBackingProvenance80092F5C ==
          PrStagePayloadBankDirect::ReplayPayloadBackingProvenance80092F5C::
              ReplayMirrorCopy8001635C);
}

void TestGenericImportCannotForgeReplayRestorePayloadProvenance() {
    PrStage1SaveUiDirect::Reset19148();
    PrStage1SaveStatusPrefix80092F10 seed = MakeSeed80092F10(true);
    seed.replayPayloadBackingProvenance80092F5C =
        PrStagePayloadBankDirect::ReplayPayloadBackingProvenance80092F5C::
            ReplayMirrorCopy8001635C;
    seed.lastWriterFunction = PrStagePayloadBankDirect::kFn8001635C;
    seed.wrote8001635C = true;
    CHECK(PrStage1SaveUiDirect::ImportSaveStatusPrefix80092F10(seed));

    const PrStage1SaveStatusPrefix80092F10 snapshot =
        PrStage1SaveUiDirect::GetSaveStatusPrefix80092F10();
    CHECK(snapshot.known);
    CHECK(snapshot.wrote8001635C);
    CHECK(snapshot.replayPayloadBackingProvenance80092F5C ==
          PrStagePayloadBankDirect::ReplayPayloadBackingProvenance80092F5C::
              Unknown);
}

void TestSaveStatusBackupProvenancePublishesAtomically() {
    PrStage1SaveUiDirect::Reset19148();
    std::array<uint8_t, PrStage1SaveStatusPrefix80092F10::kByteCount> payload{};
    for (size_t i = 0; i < payload.size(); ++i) {
        payload[i] = static_cast<uint8_t>(0x31u + ((i * 13u) & 0xffu));
    }
    CHECK(PrStage1SaveUiDirect::CommitTypedPayload800164B4(
              PrStagePayloadBankDirect::kTypedPayloadSourceAddress8007ADE8,
              payload.data(),
              payload.size(),
              MakeRuntimeLowerCardPayloadAuthority800164B4(
                  payload.data(), payload.size()))
              .ok);
    const PrStage1SaveStatusPrefix80092F10 restored =
        PrStage1SaveUiDirect::GetSaveStatusPrefix80092F10();
    CHECK(restored.replayPayloadBackingProvenance80092F5C ==
          PrStagePayloadBankDirect::ReplayPayloadBackingProvenance80092F5C::
              TypedCardCopy800164B4);
    CHECK(std::memcmp(restored.bytes, payload.data(), payload.size()) == 0);

    const PrStage1SaveStatusBackupResult80015700 backup =
        PrStage1SaveUiDirect::Sub80015700(
            PrStage1SaveStatusPrefix80092F10::kPsxAddress);
    CHECK(backup.ok);
    CHECK(backup.backupKnown);
    CHECK(backup.backupStatusBankKnown80092F1D);

    CHECK(PrStage1SaveUiDirect::ImportSaveStatusPrefix80092F10(
        MakeSeed80092F10(true)));
    CHECK(PrStage1SaveUiDirect::GetSaveStatusPrefix80092F10()
              .replayPayloadBackingProvenance80092F5C ==
          PrStagePayloadBankDirect::ReplayPayloadBackingProvenance80092F5C::
              Unknown);
    CHECK(PrStage1SaveUiDirect::Sub80015744(
              PrStage1SaveStatusPrefix80092F10::kPsxAddress)
              .ok);
    CHECK(PrStage1SaveUiDirect::GetSaveStatusPrefix80092F10()
              .replayPayloadBackingProvenance80092F5C ==
          PrStagePayloadBankDirect::ReplayPayloadBackingProvenance80092F5C::
              TypedCardCopy800164B4);

    CHECK(PrStage1SaveUiDirect::Sub80015700(
              PrStage1SaveStatusPrefix80092F10::kPsxAddress)
              .ok);
    const PrStage1SaveStatusBackupResult80015700 failedBackup =
        PrStage1SaveUiDirect::Sub80015700(
            PrStage1SaveStatusPrefix80092F10::kPsxAddress + 1u);
    CHECK(!failedBackup.ok);
    CHECK(!failedBackup.backupKnown);
    CHECK(!failedBackup.backupStatusBankKnown80092F1D);
    CHECK(!PrStage1SaveUiDirect::Sub80015744(
               PrStage1SaveStatusPrefix80092F10::kPsxAddress)
               .ok);
}

void TestSaveStatusBackupRestoresUnknownProvenanceAndAllBytes() {
    PrStage1SaveUiDirect::Reset19148();
    const PrStage1SaveStatusPrefix80092F10 original = MakeSeed80092F10(true);
    CHECK(PrStage1SaveUiDirect::ImportSaveStatusPrefix80092F10(original));
    CHECK(PrStage1SaveUiDirect::GetSaveStatusPrefix80092F10()
              .replayPayloadBackingProvenance80092F5C ==
          PrStagePayloadBankDirect::ReplayPayloadBackingProvenance80092F5C::
              Unknown);
    CHECK(PrStage1SaveUiDirect::Sub80015700(
              PrStage1SaveStatusPrefix80092F10::kPsxAddress)
              .ok);

    std::array<uint8_t, PrStage1SaveStatusPrefix80092F10::kByteCount>
        replacement{};
    for (size_t i = 0; i < replacement.size(); ++i) {
        replacement[i] = static_cast<uint8_t>(0xA7u + ((i * 19u) & 0xffu));
    }
    CHECK(PrStage1SaveUiDirect::CommitTypedPayload800164B4(
              PrStagePayloadBankDirect::kTypedPayloadSourceAddress8007ADE8,
              replacement.data(),
              replacement.size(),
              MakeRuntimeLowerCardPayloadAuthority800164B4(
                  replacement.data(), replacement.size()))
              .ok);
    CHECK(PrStage1SaveUiDirect::Sub80015744(
              PrStage1SaveStatusPrefix80092F10::kPsxAddress)
              .ok);

    const PrStage1SaveStatusPrefix80092F10 restored =
        PrStage1SaveUiDirect::GetSaveStatusPrefix80092F10();
    CHECK(restored.known);
    CHECK(restored.statusBankKnown80092F1D);
    CHECK(restored.replayPayloadBackingProvenance80092F5C ==
          PrStagePayloadBankDirect::ReplayPayloadBackingProvenance80092F5C::
              Unknown);
    CHECK(std::memcmp(restored.bytes, original.bytes, original.byteCount) == 0);
}

void TestRollbackFailureInvalidatesPayloadAndBackupAuthority() {
    PrStage1SaveUiDirect::Reset19148();
    const PrStage1SaveStatusPrefix80092F10 original = MakeSeed80092F10(true);
    CHECK(PrStage1SaveUiDirect::ImportSaveStatusPrefix80092F10(original));
    CHECK(PrStage1SaveUiDirect::Sub80015700(
              PrStage1SaveStatusPrefix80092F10::kPsxAddress)
              .ok);

    PrStage1SaveUiDirect::InvalidateSaveStatusPrefixAuthority80092F10(
        PrStage1SaveStatusPrefix80092F10::kPsxAddress);
    const PrStage1SaveStatusPrefix80092F10 invalidated =
        PrStage1SaveUiDirect::GetSaveStatusPrefix80092F10();
    CHECK(!invalidated.known);
    CHECK(!invalidated.statusBankKnown80092F1D);
    CHECK(invalidated.helperGap);
    CHECK(invalidated.replayPayloadBackingProvenance80092F5C ==
          PrStagePayloadBankDirect::ReplayPayloadBackingProvenance80092F5C::
              Unknown);
    CHECK(!PrStage1SaveUiDirect::Sub80015744(
               PrStage1SaveStatusPrefix80092F10::kPsxAddress)
               .ok);
}

void TestHelperGapSeedImportFailsClosed() {
    PrStage1SaveUiDirect::Reset19148();
    PrStage1SaveStatusPrefix80092F10 seed = MakeSeed80092F10(true);
    seed.helperGap = true;
    seed.lastFaultAddress = 0x80092F10u;

    CHECK(!PrStage1SaveUiDirect::ImportSaveStatusPrefix80092F10(seed));

    const PrStage1SaveStatusPrefix80092F10 snapshot =
        PrStage1SaveUiDirect::GetSaveStatusPrefix80092F10();
    CHECK(!snapshot.known);
    CHECK(!snapshot.statusBankKnown80092F1D);
    CHECK(!snapshot.wrote800164B4);
}

void TestUnknownStatusBankSeedImportFailsClosed() {
    PrStage1SaveUiDirect::Reset19148();
    const PrStage1SaveStatusPrefix80092F10 seed = MakeSeed80092F10(false);

    CHECK(!PrStage1SaveUiDirect::ImportSaveStatusPrefix80092F10(seed));

    const PrStage1SaveStatusPrefix80092F10 snapshot =
        PrStage1SaveUiDirect::GetSaveStatusPrefix80092F10();
    CHECK(!snapshot.known);
    CHECK(!snapshot.statusBankKnown80092F1D);
    CHECK(!snapshot.wrote800164B4);
}

void TestKnownStatusBankSeedImportPublishesPrefix() {
    PrStage1SaveUiDirect::Reset19148();
    const PrStage1SaveStatusPrefix80092F10 seed = MakeSeed80092F10(true);

    CHECK(PrStage1SaveUiDirect::ImportSaveStatusPrefix80092F10(seed));

    const PrStage1SaveStatusPrefix80092F10 snapshot =
        PrStage1SaveUiDirect::GetSaveStatusPrefix80092F10();
    CHECK(snapshot.known);
    CHECK(snapshot.statusBankKnown80092F1D);
    CHECK(!snapshot.helperGap);
    CHECK(snapshot.lastWriterFunction == PrStagePayloadBankDirect::kFn800164B4);
    CHECK(snapshot.wrote800164B4);
    CHECK(snapshot.bytes[0] == seed.bytes[0]);
    CHECK(snapshot.bytes[0x0Du] == seed.bytes[0x0Du]);
}

void TestSub8001635CRequiresReplayMirrorSource() {
    PrStage1SaveUiDirect::Reset19148();
    const PrStage1SaveStatusPrefix80092F10 seed = MakeSeed80092F10(true);
    CHECK(PrStage1SaveUiDirect::ImportSaveStatusPrefix80092F10(seed));
    PrStage1ScorerDirectReplayBufferState unknownReplay{};
    unknownReplay.dword901BCPublishedCount = 2u;
    unknownReplay.dwordEEF8Tick96[0] = 0x01020304u;
    unknownReplay.dwordEEFCClassMask[0] = 0x05060708u;
    PrStage1SaveUiDirect::PublishAuthoritativeReplayMirrorSourceFromStage1(unknownReplay);

    const PrStage1SavePayloadProducerResult blocked =
        PrStage1SaveUiDirect::Sub8001635C(3, 3, 1, 0x11223344);
    CHECK(!blocked.ok);
    CHECK(blocked.payloadKnown);
    CHECK(blocked.helperGap);

    const PrStage1SavePayloadBankRuntimeSnapshot runtime =
        PrStage1SaveUiDirect::GetSavePayloadBankRuntimeSnapshot();
    CHECK(!runtime.replayMirrorSourceKnown8008EEF8);
    CHECK(!runtime.replayMirrorSourceShapeKnown8008EEF8);
    CHECK(runtime.replayMirrorSourceInvalidSetCount >= 1u);
    CHECK(runtime.sub8001635CAttempted);
    CHECK(!runtime.sub8001635CReplayMirrorSourceKnownAtEntry);
    CHECK(!runtime.sub8001635CReplayMirrorSourceShapeKnownAtEntry);
    CHECK(runtime.sub8001635CReplayMirrorSourceSetCountAtEntry ==
          runtime.replayMirrorSourceSetCount);
    CHECK(runtime.sub8001635CReplayMirrorSourceInvalidSetCountAtEntry ==
          runtime.replayMirrorSourceInvalidSetCount);

    const PrStage1SaveStatusPrefix80092F10 blockedSnapshot =
        PrStage1SaveUiDirect::GetSaveStatusPrefix80092F10();
    CHECK(blockedSnapshot.known);
    CHECK(blockedSnapshot.statusBankKnown80092F1D);
    CHECK(blockedSnapshot.helperGap);
    CHECK(blockedSnapshot.wrote800164B4);
    CHECK(!blockedSnapshot.wrote8001635C);
}

void TestSub80015CC4RetriesAfterReplayMirrorHydrate() {
    PrStage1SaveUiDirect::Reset19148();
    PrStage1ScorerDirectReplayBufferState unknownReplay{};
    unknownReplay.dword901BCPublishedCount = 2u;
    unknownReplay.dwordEEF8Tick96[0] = 0x01020304u;
    unknownReplay.dwordEEFCClassMask[0] = 0x05060708u;
    PrStage1SaveUiDirect::PublishAuthoritativeReplayMirrorSourceFromStage1(unknownReplay);

    const PrStage1SavePayloadProducerResult blocked =
        PrStage1SaveUiDirect::Sub80015CC4();
    CHECK(blocked.ok);
    CHECK(blocked.payloadKnown);
    CHECK(blocked.helperGap);
    CHECK(HasReplayMirrorCopyGap(blocked));

    const PrStage1SavePayloadBankRuntimeSnapshot beforeRetry =
        PrStage1SaveUiDirect::GetSavePayloadBankRuntimeSnapshot();
    CHECK(beforeRetry.sub80015CC4Attempted);
    CHECK(beforeRetry.sub80015CC4Ok);
    CHECK(beforeRetry.sub8001635CAttempted);
    CHECK(!beforeRetry.sub8001635COk);
    CHECK(beforeRetry.sub8001635CPreflightPayloadKnown);
    CHECK(beforeRetry.sub8001635CPreflightStatusBankKnown);
    CHECK(beforeRetry.sub8001635CPreflightMapped);
    CHECK(beforeRetry.sub8001635CPreflightCarrierSourceKnown);
    CHECK(!beforeRetry.sub8001635CPreflightMirrorSourceKnown);
    CHECK(!beforeRetry.sub8001635CReplayMirrorSourceKnownAtEntry);
    CHECK(!beforeRetry.wrote8001635C);

    PrStage1ScorerDirectReplayBufferState replay =
        MakeFullReplayRestoreProducer();
    PrStage1SaveUiDirect::PublishAuthoritativeReplayMirrorSourceFromStage1(replay);

    const PrStage1SavePayloadBankRuntimeSnapshot afterRetry =
        PrStage1SaveUiDirect::GetSavePayloadBankRuntimeSnapshot();
    CHECK(afterRetry.payloadKnown);
    CHECK(afterRetry.statusBankKnown80092F1D);
    CHECK(!afterRetry.helperGap);
    CHECK(afterRetry.savePayloadSourceKnown);
    CHECK(afterRetry.sub80015CC4Attempted);
    CHECK(afterRetry.sub8001635CAttempted);
    CHECK(afterRetry.sub8001635COk);
    CHECK(afterRetry.sub8001635CPreflightPayloadKnown);
    CHECK(afterRetry.sub8001635CPreflightStatusBankKnown);
    CHECK(afterRetry.sub8001635CPreflightMapped);
    CHECK(afterRetry.sub8001635CPreflightCarrierSourceKnown);
    CHECK(afterRetry.sub8001635CPreflightMirrorSourceKnown);
    CHECK(afterRetry.sub8001635CReplayMirrorSourceKnownAtEntry);
    CHECK(afterRetry.sub8001635CReplayMirrorSourceShapeKnownAtEntry);
    CHECK(afterRetry.sub8001635CReplayMirrorSourceSetCountAtEntry ==
          afterRetry.replayMirrorSourceSetCount);
    CHECK(afterRetry.sub8001635CReplayMirrorSourceHydrateCountAtEntry ==
          afterRetry.replayMirrorSourceHydrateCount);
    CHECK(afterRetry.sub8001635CScratchAuthorityKnown);
    CHECK(afterRetry.sub8001635CMirrorCopied);
    CHECK(afterRetry.sub8001635CAllClearQueried);
    CHECK(afterRetry.sub8001635CAllClearWritten);
    CHECK(afterRetry.lastWriterFunction == PrStagePayloadBankDirect::kFn8001635C);
    CHECK(afterRetry.wrote8001635C);

    const PrStage1SaveStatusPrefix80092F10 snapshot =
        PrStage1SaveUiDirect::GetSaveStatusPrefix80092F10();
    CHECK(snapshot.known);
    CHECK(snapshot.statusBankKnown80092F1D);
    CHECK(!snapshot.helperGap);
    CHECK(snapshot.lastWriterFunction == PrStagePayloadBankDirect::kFn8001635C);
    CHECK(snapshot.wrote8001635C);
}

void TestReplayMirrorHydrateDoesNotRetryNonDefault8001635CArgs() {
    PrStage1SaveUiDirect::Reset19148();
    const PrStage1SaveStatusPrefix80092F10 seed = MakeSeed80092F10(true);
    CHECK(PrStage1SaveUiDirect::ImportSaveStatusPrefix80092F10(seed));
    PrStage1ScorerDirectReplayBufferState unknownReplay{};
    unknownReplay.dword901BCPublishedCount = 2u;
    unknownReplay.dwordEEF8Tick96[0] = 0x01020304u;
    unknownReplay.dwordEEFCClassMask[0] = 0x05060708u;
    PrStage1SaveUiDirect::PublishAuthoritativeReplayMirrorSourceFromStage1(unknownReplay);

    const PrStage1SavePayloadProducerResult blocked =
        PrStage1SaveUiDirect::Sub8001635C(3, 3, 1, 0x11223344);
    CHECK(!blocked.ok);
    CHECK(blocked.payloadKnown);
    CHECK(blocked.helperGap);

    PrStage1ScorerDirectReplayBufferState replay =
        MakeFullReplayRestoreProducer();
    PrStage1SaveUiDirect::PublishAuthoritativeReplayMirrorSourceFromStage1(replay);

    const PrStage1SavePayloadBankRuntimeSnapshot afterHydrate =
        PrStage1SaveUiDirect::GetSavePayloadBankRuntimeSnapshot();
    CHECK(afterHydrate.replayMirrorSourceKnown8008EEF8);
    CHECK(afterHydrate.replayMirrorSourceShapeKnown8008EEF8);
    CHECK(afterHydrate.sub8001635CAttempted);
    CHECK(!afterHydrate.sub8001635COk);
    CHECK(!afterHydrate.sub8001635CReplayMirrorSourceKnownAtEntry);
    CHECK(afterHydrate.lastWriterFunction == PrStagePayloadBankDirect::kFn800164B4);
    CHECK(!afterHydrate.wrote8001635C);
}

void TestSub8001635CRejectsOversizeReplayMirrorShape() {
    PrStage1SaveUiDirect::Reset19148();
    const PrStage1SaveStatusPrefix80092F10 seed = MakeSeed80092F10(true);
    CHECK(PrStage1SaveUiDirect::ImportSaveStatusPrefix80092F10(seed));

    PrStage1ScorerDirectReplayBufferState replay{};
    replay.replayMirrorKnown8008EEF8 = true;
    replay.replayMirrorProducerKnown8008EEF8 = true;
    replay.replayMirrorProducerFunction = kPrStage1ScorerDirectFn801C8660;
    replay.dword901BCPublishedCount =
        static_cast<uint32_t>(kPrStage1ScorerDirectReplayBufferCapacity + 1u);
    replay.dword901C0WriteCount =
        static_cast<uint32_t>(kPrStage1ScorerDirectReplayBufferCapacity + 1u);
    replay.dwordEEF8Tick96[0] = 0x01020304u;
    replay.dwordEEFCClassMask[0] = 0x05060708u;
    PrStage1SaveUiDirect::PublishAuthoritativeReplayMirrorSourceFromStage1(replay);

    const PrStage1SavePayloadProducerResult blocked =
        PrStage1SaveUiDirect::Sub8001635C(3, 3, 1, 0x11223344);
    CHECK(!blocked.ok);
    CHECK(blocked.payloadKnown);
    CHECK(blocked.helperGap);

    const PrStage1SaveStatusPrefix80092F10 blockedSnapshot =
        PrStage1SaveUiDirect::GetSaveStatusPrefix80092F10();
    CHECK(blockedSnapshot.known);
    CHECK(blockedSnapshot.statusBankKnown80092F1D);
    CHECK(blockedSnapshot.helperGap);
    CHECK(blockedSnapshot.wrote800164B4);
    CHECK(!blockedSnapshot.wrote8001635C);
}

void TestSub8001635CRejectsReplayMirrorWithoutProducerFamily() {
    PrStage1SaveUiDirect::Reset19148();
    const PrStage1SaveStatusPrefix80092F10 seed = MakeSeed80092F10(true);
    CHECK(PrStage1SaveUiDirect::ImportSaveStatusPrefix80092F10(seed));

    PrStage1ScorerDirectReplayBufferState replay{};
    replay.replayMirrorKnown8008EEF8 = true;
    replay.dword901BCPublishedCount = 2u;
    replay.dwordEEF8Tick96[0] = 0x01020304u;
    replay.dwordEEFCClassMask[0] = 0x05060708u;
    replay.dwordEEF8Tick96[1] = 0x11121314u;
    replay.dwordEEFCClassMask[1] = 0x15161718u;
    PrStage1SaveUiDirect::PublishAuthoritativeReplayMirrorSourceFromStage1(replay);

    const PrStage1SavePayloadProducerResult blocked =
        PrStage1SaveUiDirect::Sub8001635C(3, 3, 1, 0x11223344);
    CHECK(!blocked.ok);
    CHECK(blocked.payloadKnown);
    CHECK(blocked.helperGap);

    const PrStage1SaveStatusPrefix80092F10 blockedSnapshot =
        PrStage1SaveUiDirect::GetSaveStatusPrefix80092F10();
    CHECK(blockedSnapshot.known);
    CHECK(blockedSnapshot.statusBankKnown80092F1D);
    CHECK(blockedSnapshot.helperGap);
    CHECK(blockedSnapshot.wrote800164B4);
    CHECK(!blockedSnapshot.wrote8001635C);
}

void TestAcceptedAppend80014614FullCapacityPublishesFullBacking() {
    PrStage1ScorerDirectGlobals globals{};
    globals.word916E2CurrentSceneIndex = 1u;
    PrStage1ScorerDirectAcceptedProducerOwnerState ownerState{};
    PrStage1ScorerDirectReplayBufferState replay{};
    const PrStage1ScorerDirectDescriptorRow row = MakeAppendDescriptorRow();
    const PrStage1ScorerDirectAcceptedProducerAccessors accessors{
        nullptr,
        ReadAppendTimingTemplateState,
        ReadAppendSourceCellHeader,
        ReadAppendSourceCell,
    };

    for (size_t i = 0; i < kPrStage1ScorerDirectReplayBufferCapacity; ++i) {
        const PrStage1ScorerDirectAcceptedProducerCoreInput in =
            MakeAppendInput(row, static_cast<int32_t>(i * 24u));
        const PrStage1ScorerDirectAcceptedProducerRunResult run =
            PrStage1ScorerDirectRunAcceptedProducer14614(
                globals,
                ownerState,
                replay,
                in,
                accessors);
        CHECK(run.resultCode == 0);
        CHECK(run.replayAppendRan);
    }

    CHECK(replay.replayMirrorKnown8008EEF8);
    CHECK(replay.replayMirrorProducerKnown8008EEF8);
    CHECK(replay.replayMirrorProducerFunction == kPrStage1ScorerDirectFn80014614);
    CHECK(replay.replayMirrorByteCountKnown8008EEF8);
    CHECK(replay.replayMirrorKnownByteCount8008EEF8 ==
          kPrStage1ScorerDirectReplayMirrorByteCount);
    CHECK(replay.replayMirrorFullBackingKnown8008EEF8);
    CHECK(replay.dword901BCPublishedCount ==
          kPrStage1ScorerDirectReplayBufferCapacity);
    CHECK(replay.dword901C0WriteCount ==
          kPrStage1ScorerDirectReplayBufferCapacity);

    PrStage1SaveUiDirect::Reset19148();
    const PrStage1SaveStatusPrefix80092F10 seed = MakeSeed80092F10(true);
    CHECK(PrStage1SaveUiDirect::ImportSaveStatusPrefix80092F10(seed));
    PrStage1SaveUiDirect::PublishAuthoritativeReplayMirrorSourceFromStage1(replay);
    const PrStage1SavePayloadProducerResult result =
        PrStage1SaveUiDirect::Sub8001635C(3, 3, 1, 0x11223344);
    CHECK(result.ok);
    CHECK(result.payloadKnown);
    CHECK(!result.helperGap);
    const PrStage1SavePayloadBankRuntimeSnapshot runtime =
        PrStage1SaveUiDirect::GetSavePayloadBankRuntimeSnapshot();
    CHECK(runtime.replayMirrorCandidateProducerFunction ==
          kPrStage1ScorerDirectFn80014614);
    CHECK(runtime.replayMirrorCandidateKnownByteCount8008EEF8 ==
          kPrStage1ScorerDirectReplayMirrorByteCount);
    CHECK(runtime.replayMirrorCandidateFullBackingKnown8008EEF8);
    CHECK(runtime.replayMirrorAuthorityKnown8001635C);
    CHECK(runtime.wrote8001635C);
}

void TestNatural801C8660PartialProducerBlocksSaveUiStart() {
    PrStage1SaveUiDirect::Reset19148();
    const PrStage1SaveStatusPrefix80092F10 seed = MakeSeed80092F10(true);
    CHECK(PrStage1SaveUiDirect::ImportSaveStatusPrefix80092F10(seed));

    PrStage1ScorerDirectReplayBufferState replay{};
    const PrStage1ScorerDirectResolvedReplayBackup1681C noBackup{};
    const PrStage1ScorerDirectAcceptedSpecialSetupResult setup =
        PrStage1ScorerDirectRunAcceptedSpecialSetupCore24E54_1681C(
            1u,
            noBackup,
            replay);
    CHECK(!setup.setup.restoreReplayBuffer1681CRequested);
    CHECK(!setup.restore.restoreApplied);
    CHECK(setup.restoreSource ==
          PrStage1ScorerDirectReplayRestoreSource1681C::None);
    CHECK(replay.dword901BCPublishedCount == 0u);
    CHECK(replay.dword901C0WriteCount == 0u);
    const PrStage1ScorerDirectEventTableBuild801C8660Result producer =
        PrStage1ScorerDirectBuildStage1EventTable801C8660(replay);
    CHECK(producer.applied);
    CHECK(producer.count800901BC == 53u);
    CHECK(replay.replayMirrorKnown8008EEF8);
    CHECK(replay.replayMirrorProducerKnown8008EEF8);
    CHECK(replay.replayMirrorProducerFunction == kPrStage1ScorerDirectFn801C8660);
    CHECK(replay.replayMirrorByteCountKnown8008EEF8);
    CHECK(replay.replayMirrorKnownByteCount8008EEF8 ==
          producer.count800901BC * 2u * sizeof(uint32_t));
    CHECK(replay.replayMirrorKnownByteCount8008EEF8 == 424u);
    CHECK(replay.replayMirrorKnownByteCount8008EEF8 <
          kPrStage1ScorerDirectReplayMirrorByteCount);
    CHECK(!replay.replayMirrorFullBackingKnown8008EEF8);
    CHECK(replay.dword901BCPublishedCount == producer.count800901BC);
    CHECK(replay.dword901C0WriteCount == 0u);
    PrStage1SaveUiDirect::PublishAuthoritativeReplayMirrorSourceFromStage1(replay);

    const PrStage1SavePayloadProducerResult blocked =
        PrStage1SaveUiDirect::Sub8001635C(3, 3, 1, 0x11223344);
    CHECK(!blocked.ok);
    CHECK(blocked.payloadKnown);
    CHECK(blocked.helperGap);

    const PrStage1SaveStatusPrefix80092F10 blockedSnapshot =
        PrStage1SaveUiDirect::GetSaveStatusPrefix80092F10();
    CHECK(blockedSnapshot.known);
    CHECK(blockedSnapshot.statusBankKnown80092F1D);
    CHECK(blockedSnapshot.helperGap);
    CHECK(blockedSnapshot.wrote800164B4);
    CHECK(!blockedSnapshot.wrote8001635C);
    const PrStage1SavePayloadBankRuntimeSnapshot runtime =
        PrStage1SaveUiDirect::GetSavePayloadBankRuntimeSnapshot();
    CHECK(runtime.replayMirrorCandidateKnown8008EEF8);
    CHECK(runtime.replayMirrorCandidateProducerKnown8008EEF8);
    CHECK(runtime.replayMirrorCandidateProducerFunction ==
          kPrStage1ScorerDirectFn801C8660);
    CHECK(runtime.replayMirrorCandidateByteCountKnown8008EEF8);
    CHECK(runtime.replayMirrorCandidateKnownByteCount8008EEF8 == 424u);
    CHECK(!runtime.replayMirrorCandidateFullBackingKnown8008EEF8);
    CHECK(runtime.replayMirrorCandidatePublishedCount901BC == 53u);
    CHECK(runtime.replayMirrorCandidateWriteCount901C0 == 0u);
    CHECK(!runtime.replayMirrorAuthorityKnown8001635C);

    PrGameContext ctx{};
    CHECK(!PrStage1SaveUiDirect::Start19148(ctx, &blockedSnapshot));
    CHECK(!PrStage1SaveUiDirect::IsActive19148());
}

void Test801C8660WithExplicitFullBackingAuthorityStartsSaveUi() {
    PrStage1SaveUiDirect::Reset19148();
    const PrStage1SaveStatusPrefix80092F10 seed = MakeSeed80092F10(true);
    CHECK(PrStage1SaveUiDirect::ImportSaveStatusPrefix80092F10(seed));

    PrStage1ScorerDirectReplayBufferState replay{};
    CHECK(PrStage1ScorerDirectInitializeReplayMirrorFromStartupZero80028590(
        replay));
    const PrStage1ScorerDirectEventTableBuild801C8660Result producer =
        PrStage1ScorerDirectBuildStage1EventTable801C8660(replay);
    CHECK(producer.applied);
    CHECK(producer.count800901BC == 53u);
    CHECK(replay.replayMirrorProducerFunction == kPrStage1ScorerDirectFn801C8660);
    CHECK(replay.replayMirrorKnownByteCount8008EEF8 ==
          kPrStage1ScorerDirectReplayMirrorByteCount);
    CHECK(replay.replayMirrorFullBackingKnown8008EEF8);
    CHECK(replay.dword901BCPublishedCount == producer.count800901BC);
    PrStage1SaveUiDirect::PublishAuthoritativeReplayMirrorSourceFromStage1(replay);

    const PrStage1SavePayloadProducerResult result =
        PrStage1SaveUiDirect::Sub8001635C(3, 3, 1, 0x11223344);
    CHECK(result.ok);
    CHECK(result.payloadKnown);
    CHECK(!result.helperGap);

    const PrStage1SavePayloadBankRuntimeSnapshot runtime =
        PrStage1SaveUiDirect::GetSavePayloadBankRuntimeSnapshot();
    CHECK(runtime.payloadKnown);
    CHECK(runtime.statusBankKnown80092F1D);
    CHECK(!runtime.helperGap);
    CHECK(runtime.savePayloadSourceKnown);
    CHECK(runtime.import80092F10Attempted);
    CHECK(runtime.import80092F10Ok);
    CHECK(runtime.sub8001635CAttempted);
    CHECK(runtime.sub8001635COk);
    CHECK(runtime.sub8001635CResult == result.result);
    CHECK(runtime.sub8001635CPreflightPayloadKnown);
    CHECK(runtime.sub8001635CPreflightStatusBankKnown);
    CHECK(runtime.sub8001635CPreflightMapped);
    CHECK(runtime.sub8001635CPreflightCarrierSourceKnown);
    CHECK(runtime.sub8001635CPreflightMirrorSourceKnown);
    CHECK(runtime.sub8001635CScratchAuthorityKnown);
    CHECK(runtime.sub8001635CMirrorCopied);
    CHECK(runtime.sub8001635CAllClearQueried);
    CHECK(runtime.sub8001635CAllClearWritten);
    CHECK(runtime.replayMirrorCandidateKnown8008EEF8);
    CHECK(runtime.replayMirrorCandidateProducerKnown8008EEF8);
    CHECK(runtime.replayMirrorCandidateProducerFunction ==
          kPrStage1ScorerDirectFn801C8660);
    CHECK(runtime.replayMirrorCandidateByteCountKnown8008EEF8);
    CHECK(runtime.replayMirrorCandidateKnownByteCount8008EEF8 ==
          kPrStage1ScorerDirectReplayMirrorByteCount);
    CHECK(runtime.replayMirrorCandidateFullBackingKnown8008EEF8);
    CHECK(runtime.replayMirrorSourceKnown8008EEF8);
    CHECK(runtime.replayMirrorSourceShapeKnown8008EEF8);
    CHECK(runtime.replayMirrorSourceProducerKnown8008EEF8);
    CHECK(runtime.replayMirrorSourceProducerFunction ==
          kPrStage1ScorerDirectFn801C8660);
    CHECK(runtime.replayMirrorSourceByteCountKnown8008EEF8);
    CHECK(runtime.replayMirrorSourceKnownByteCount8008EEF8 ==
          kPrStage1ScorerDirectReplayMirrorByteCount);
    CHECK(runtime.replayMirrorSourceFullBackingKnown8008EEF8);
    CHECK(runtime.replayMirrorSourceSetCount >= 1u);
    CHECK(runtime.replayMirrorSourceHydrateCount >= 1u);
    CHECK(runtime.replayMirrorAuthorityKnown8001635C);
    CHECK(runtime.sub8001635CReplayMirrorSourceKnownAtEntry);
    CHECK(runtime.sub8001635CReplayMirrorSourceShapeKnownAtEntry);
    CHECK(runtime.sub8001635CReplayMirrorSourceSetCountAtEntry ==
          runtime.replayMirrorSourceSetCount);
    CHECK(runtime.sub8001635CReplayMirrorSourceInvalidSetCountAtEntry ==
          runtime.replayMirrorSourceInvalidSetCount);
    CHECK(runtime.sub8001635CReplayMirrorSourceHydrateCountAtEntry ==
          runtime.replayMirrorSourceHydrateCount);
    CHECK(runtime.lastWriterFunction == PrStagePayloadBankDirect::kFn8001635C);
    CHECK(runtime.wrote8001635C);

    const PrStage1SaveStatusPrefix80092F10 snapshot =
        PrStage1SaveUiDirect::GetSaveStatusPrefix80092F10();
    CHECK(snapshot.known);
    CHECK(snapshot.statusBankKnown80092F1D);
    CHECK(snapshot.lastWriterFunction == PrStagePayloadBankDirect::kFn8001635C);
    CHECK(snapshot.wrote8001635C);
    CHECK(ReadLe32Prefix(snapshot, PrStagePayloadBankDirect::kCarrierSourceAddress80092F48) ==
          producer.count800901BC);

    PrGameContext ctx{};
    CHECK(PrStage1SaveUiDirect::Start19148(ctx, &snapshot));
    CHECK(PrStage1SaveUiDirect::IsActive19148());
}

void TestReplayRestoreKeepsFullBackingWithPartialPublishedCount() {
    PrStage1ScorerDirectReplayBackupState backup{};
    backup.valid = true;
    backup.dword92F48PublishedCount = 53u;
    backup.replayMirrorFullBackingKnown8008EEF8 = true;
    for (size_t i = 0; i < kPrStage1ScorerDirectReplayBufferCapacity; ++i) {
        backup.dwordEEF8Tick96[i] = 0x40000000u + static_cast<uint32_t>(i);
        backup.dwordEEFCClassMask[i] = 0x50000000u + static_cast<uint32_t>(i);
    }

    const PrStage1ScorerDirectResolvedReplayBackup1681C resolved =
        PrStage1ScorerDirectResolveReplayRestoreSource1681C(
            true,
            backup,
            false,
            PrStage1ScorerDirectReplayBackupState{});
    PrStage1ScorerDirectReplayBufferState replay{};
    CHECK(PrStage1ScorerDirectInitializeReplayMirrorFromStartupZero80028590(
        replay));
    const PrStage1ScorerDirectAcceptedSpecialSetupResult restore =
        PrStage1ScorerDirectRunAcceptedSpecialSetupCore24E54_1681C(
            2u,
            resolved,
            replay);

    CHECK(restore.restore.restoreApplied);
    CHECK(replay.replayMirrorProducerFunction == kPrStage1ScorerDirectFn8001681C);
    CHECK(replay.dword901BCPublishedCount == 53u);
    CHECK(replay.dword901C0WriteCount == 0u);
    CHECK(replay.replayMirrorKnownByteCount8008EEF8 ==
          kPrStage1ScorerDirectReplayMirrorByteCount);
    CHECK(replay.replayMirrorFullBackingKnown8008EEF8);
    CHECK(replay.dwordEEF8Tick96[0] == 0x40000000u);
    CHECK(replay.dwordEEFCClassMask[0] == 0x50000000u);
    CHECK(replay.dwordEEF8Tick96[kPrStage1ScorerDirectReplayBufferCapacity - 1u] ==
          0u);
    CHECK(replay.dwordEEFCClassMask[kPrStage1ScorerDirectReplayBufferCapacity - 1u] ==
          0u);
}

void TestReplayRestoreCannotMintUnknownDestinationTailFromFullPayload() {
    PrStage1ScorerDirectReplayBackupState backup{};
    backup.valid = true;
    backup.dword92F48PublishedCount = 53u;
    backup.replayMirrorFullBackingKnown8008EEF8 = true;
    for (size_t i = 0; i < kPrStage1ScorerDirectReplayBufferCapacity; ++i) {
        backup.dwordEEF8Tick96[i] = 0x60000000u + static_cast<uint32_t>(i);
        backup.dwordEEFCClassMask[i] = 0x70000000u + static_cast<uint32_t>(i);
    }

    const PrStage1ScorerDirectResolvedReplayBackup1681C resolved =
        PrStage1ScorerDirectResolveReplayRestoreSource1681C(
            true,
            backup,
            false,
            PrStage1ScorerDirectReplayBackupState{});
    PrStage1ScorerDirectReplayBufferState replay{};
    const PrStage1ScorerDirectAcceptedSpecialSetupResult restore =
        PrStage1ScorerDirectRunAcceptedSpecialSetupCore24E54_1681C(
            2u,
            resolved,
            replay);

    CHECK(restore.restore.restoreApplied);
    CHECK(replay.replayMirrorProducerFunction == kPrStage1ScorerDirectFn8001681C);
    CHECK(replay.dword901BCPublishedCount == 53u);
    CHECK(replay.replayMirrorKnownByteCount8008EEF8 == 53u * 8u);
    CHECK(!replay.replayMirrorFullBackingKnown8008EEF8);
    CHECK(replay.dwordEEF8Tick96[52] == 0x60000034u);
    CHECK(replay.dwordEEFCClassMask[52] == 0x70000034u);
    CHECK(replay.dwordEEF8Tick96[53] == 0u);
    CHECK(replay.dwordEEFCClassMask[53] == 0u);
    CHECK(replay.dwordEEF8Tick96[kPrStage1ScorerDirectReplayBufferCapacity - 1u] ==
          0u);
    CHECK(replay.dwordEEFCClassMask[kPrStage1ScorerDirectReplayBufferCapacity - 1u] ==
          0u);
}

void TestSub8001635CRejectsForgedPartialAppendMetadata() {
    const uint32_t appendCounts[] = {1u, 2u};
    for (uint32_t appendCount : appendCounts) {
        PrStage1SaveUiDirect::Reset19148();
        const PrStage1SaveStatusPrefix80092F10 seed = MakeSeed80092F10(true);
        CHECK(PrStage1SaveUiDirect::ImportSaveStatusPrefix80092F10(seed));

        PrStage1ScorerDirectReplayBufferState replay{};
        replay.replayMirrorKnown8008EEF8 = true;
        replay.replayMirrorProducerKnown8008EEF8 = true;
        replay.replayMirrorProducerFunction = kPrStage1ScorerDirectFn80014614;
        replay.replayMirrorByteCountKnown8008EEF8 = true;
        replay.replayMirrorKnownByteCount8008EEF8 =
            appendCount * 2u * sizeof(uint32_t);
        replay.dword901BCPublishedCount = appendCount;
        replay.dword901C0WriteCount = appendCount;
        for (uint32_t i = 0; i < appendCount; ++i) {
            replay.dwordEEF8Tick96[i] = 0x01020304u + i;
            replay.dwordEEFCClassMask[i] = 0x05060708u + i;
        }
        CHECK(replay.replayMirrorKnownByteCount8008EEF8 == appendCount * 8u);
        CHECK(replay.replayMirrorKnownByteCount8008EEF8 <
              kPrStage1ScorerDirectReplayMirrorByteCount);
        CHECK(!PrStage1ScorerDirectReplayMirrorAuthorityMatchesState(replay));
        PrStage1SaveUiDirect::PublishAuthoritativeReplayMirrorSourceFromStage1(replay);

        const PrStage1SavePayloadProducerResult blocked =
            PrStage1SaveUiDirect::Sub8001635C(3, 3, 1, 0x11223344);
        CHECK(!blocked.ok);
        CHECK(blocked.payloadKnown);
        CHECK(blocked.helperGap);

        const PrStage1SaveStatusPrefix80092F10 blockedSnapshot =
            PrStage1SaveUiDirect::GetSaveStatusPrefix80092F10();
        CHECK(blockedSnapshot.known);
        CHECK(blockedSnapshot.statusBankKnown80092F1D);
        CHECK(blockedSnapshot.helperGap);
        CHECK(blockedSnapshot.wrote800164B4);
        CHECK(!blockedSnapshot.wrote8001635C);
    }
}

void TestSub8001635CAcceptsFullReplayMirrorRestoreProducer() {
    PrStage1SaveUiDirect::Reset19148();
    const PrStage1SaveStatusPrefix80092F10 seed = MakeSeed80092F10(true);
    CHECK(PrStage1SaveUiDirect::ImportSaveStatusPrefix80092F10(seed));

    PrStage1ScorerDirectReplayBufferState replay =
        MakeFullReplayRestoreProducer();
    const PrStage1ScorerDirectReplayBackupCaptureResult callbackCapture =
        PrStage1ScorerDirectRunReplayBackupCaptureCore1635C(replay);
    CHECK(callbackCapture.captureApplied);
    CHECK(callbackCapture.backup.valid);
    CHECK(callbackCapture.backup.dword92F48PublishedCount == 53u);
    CHECK(callbackCapture.backup.replayMirrorFullBackingKnown8008EEF8);
    CHECK(callbackCapture.backup.dwordEEF8Tick96[0] == 24u * 120u);
    CHECK(callbackCapture.backup.dwordEEFCClassMask[0] == 0x10u);
    PrStage1SaveUiDirect::PublishAuthoritativeReplayMirrorSourceFromStage1(replay);

    const PrStage1SavePayloadProducerResult result =
        PrStage1SaveUiDirect::Sub8001635C(3, 3, 1, 0x11223344);
    CHECK(result.ok);
    CHECK(result.payloadKnown);
    CHECK(!result.helperGap);

    const PrStage1SaveStatusPrefix80092F10 snapshot =
        PrStage1SaveUiDirect::GetSaveStatusPrefix80092F10();
    CHECK(snapshot.known);
    CHECK(snapshot.statusBankKnown80092F1D);
    CHECK(snapshot.lastWriterFunction == PrStagePayloadBankDirect::kFn8001635C);
    CHECK(snapshot.wrote8001635C);
    CHECK(ReadLe32Prefix(snapshot, PrStagePayloadBankDirect::kCarrierSourceAddress80092F48) ==
          53u);
    CHECK(ReadLe32Prefix(snapshot, PrStagePayloadBankDirect::kMirrorDstAddress80092F5C) ==
          24u * 120u);
    CHECK(ReadLe32Prefix(snapshot,
                         PrStagePayloadBankDirect::kMirrorDstAddress80092F5C +
                             sizeof(uint32_t)) == 0x10u);

    PrGameContext ctx{};
    CHECK(PrStage1SaveUiDirect::Start19148(ctx, &snapshot));
    CHECK(PrStage1SaveUiDirect::IsActive19148());
    const PrStage1SaveStatusPrefix80092F10 activeSnapshot =
        PrStage1SaveUiDirect::GetSaveStatusPrefix80092F10();
    CHECK(activeSnapshot.known);
    CHECK(activeSnapshot.statusBankKnown80092F1D);
    CHECK(activeSnapshot.lastWriterFunction == PrStagePayloadBankDirect::kFn8001635C);
    CHECK(activeSnapshot.wrote8001635C);
}

} // namespace

int main() {
    TestColdBootStatusSeedPublishesFromIdaStartupZero();
    TestColdBootStatusSeedPublishesWithFullReplayMirrorProducer();
    TestColdBootStartupZeroPersistsThroughFirstSub80015CC4();
    TestColdBootNaturalProducerConsumesStartupZeroOpportunity();
    TestStartupZeroAuthorityRemainsBoundedToStaticShape();
    TestUnknownStatusBankSeedFailsClosedBeforeStart();
    TestKnownStatusBankSeedStartsAndPublishesPrefix();
    TestSaveStatusThenUnlockSeedPreservesChainAuthority();
    TestNameInputSuffixPreservesSaveStatusSeedAuthority();
    TestGenericImportCannotForgeReplayRestorePayloadProvenance();
    TestSaveStatusBackupProvenancePublishesAtomically();
    TestSaveStatusBackupRestoresUnknownProvenanceAndAllBytes();
    TestRollbackFailureInvalidatesPayloadAndBackupAuthority();
    TestHelperGapSeedImportFailsClosed();
    TestUnknownStatusBankSeedImportFailsClosed();
    TestKnownStatusBankSeedImportPublishesPrefix();
    TestSub8001635CRequiresReplayMirrorSource();
    TestSub80015CC4RetriesAfterReplayMirrorHydrate();
    TestReplayMirrorHydrateDoesNotRetryNonDefault8001635CArgs();
    TestSub8001635CRejectsOversizeReplayMirrorShape();
    TestSub8001635CRejectsReplayMirrorWithoutProducerFamily();
    TestAcceptedAppend80014614FullCapacityPublishesFullBacking();
    TestNatural801C8660PartialProducerBlocksSaveUiStart();
    Test801C8660WithExplicitFullBackingAuthorityStartsSaveUi();
    TestReplayRestoreKeepsFullBackingWithPartialPublishedCount();
    TestReplayRestoreCannotMintUnknownDestinationTailFromFullPayload();
    TestSub8001635CRejectsForgedPartialAppendMetadata();
    TestSub8001635CAcceptsFullReplayMirrorRestoreProducer();

    if (g_failedChecks != 0) {
        std::printf("test_ss0_save_ui19148_status_bank_seed: failed checks=%d\n",
                    g_failedChecks);
        return 1;
    }
    std::printf("test_ss0_save_ui19148_status_bank_seed: ok\n");
    return 0;
}
