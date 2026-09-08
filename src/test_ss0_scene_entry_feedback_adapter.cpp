#include "pr/pr_scene_entry_feedback_adapter_direct.h"
#include "pr/pr_scene_entry_card_feedback_direct.h"

#include <cstdint>
#include <cstdio>
#include <cstring>

namespace {

int g_failed = 0;

#define CHECK(expr)                                                           \
    do {                                                                      \
        if (!(expr)) {                                                        \
            std::printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr);       \
            ++g_failed;                                                       \
        }                                                                     \
    } while (0)

PrStage1SaveStatusPrefix80092F10 MakeStatusPrefix(bool statusBankKnown) {
    PrStage1SaveStatusPrefix80092F10 prefix{};
    prefix.known = true;
    prefix.statusBankKnown80092F1D = statusBankKnown;
    prefix.helperGap = false;
    prefix.psxAddress = PrStage1SaveStatusPrefix80092F10::kPsxAddress;
    prefix.byteCount = PrStage1SaveStatusPrefix80092F10::kByteCount;
    for (uint32_t i = 0; i < prefix.byteCount; ++i) {
        prefix.bytes[i] = static_cast<uint8_t>(i & 0xffu);
    }
    return prefix;
}

PrSceneEntryDirect::Case17Result80019D7C MakeCase17Result() {
    PrSceneEntryDirect::Case17Result80019D7C result{};
    result.resultKnown = true;
    result.result = 23;
    result.gp720Written = true;
    result.gp720 = 1;
    result.bank = PrSceneEntryDirect::PsxCall800168DC_ClearHiScoreBank80019D7C();
    return result;
}

void TestUnknownStatusBankBlocksAdapterBeforeInputMemory() {
    const PrStage1SaveStatusPrefix80092F10 prefix = MakeStatusPrefix(false);
    const PrSceneEntryDirect::Case17Result80019D7C case17 =
        MakeCase17Result();

    PrSceneEntryFeedbackAdapterDirect::FeedbackAdapterResult80019414 out{};
    PrSceneEntryFeedbackAdapterDirect::
        BuildFeedback80019414FromStatusPrefixAndCase17Bank(
            prefix,
            case17,
            &out);

    CHECK(!out.completed);
    CHECK(out.gap);
    CHECK(out.gapReason ==
          PrSceneEntryFeedbackAdapterDirect::FeedbackAdapterGap80019414::
              MissingStatusBank80092F1D);
    CHECK(!out.inputMemory.inputMemoryKnown);
    CHECK(!out.call80019284.resultKnown);
    CHECK(out.tablePsxAddress == 0);
    CHECK(out.tableByteCount == 0);
}

void TestKnownStatusBankCompletesAdapter() {
    const PrStage1SaveStatusPrefix80092F10 prefix = MakeStatusPrefix(true);
    const PrSceneEntryDirect::Case17Result80019D7C case17 =
        MakeCase17Result();

    PrSceneEntryFeedbackAdapterDirect::FeedbackAdapterResult80019414 out{};
    PrSceneEntryFeedbackAdapterDirect::
        BuildFeedback80019414FromStatusPrefixAndCase17Bank(
            prefix,
            case17,
            &out);

    CHECK(out.completed);
    CHECK(!out.gap);
    CHECK(out.gapReason ==
          PrSceneEntryFeedbackAdapterDirect::FeedbackAdapterGap80019414::None);
    CHECK(out.inputMemory.inputMemoryKnown);
    CHECK(out.call80019284.resultKnown);
    CHECK(out.feedback.gp720Known);
    CHECK(out.feedback.sub80026784ResultKnown);
    CHECK(static_cast<uint32_t>(out.feedback.sub80026784Result) ==
          PrSceneEntryDirect::kMemcardStateTable800544F8);
    CHECK(out.call800191E4ReturnUnresolvedCurrentIda);
    CHECK(!out.feedback.gp716AfterStateMachineKnown);
    CHECK(!out.feedback.wordA1Plus44Known);
    CHECK(out.feedback.call80019284ResultKnown);
    CHECK(out.tablePsxAddress == 0x80049278u);
    CHECK(out.tableByteCount ==
          PrSceneEntryDirect::kHiScoreTableSize80019284);

    const PrSceneEntryDirect::Call80019414Result80015788 call19414 =
        PrSceneEntryDirect::PsxCall80019414_HiScoreEntry80015788(
            static_cast<int32_t>(prefix.psxAddress),
            true,
            out.feedback);
    const PrSceneEntryDirect::Call800191E4Result80015788& call191E4 =
        call19414.call800191E4;
    CHECK(call191E4.called80026784);
    CHECK(call191E4.copied36BytesTo8007CC50);
    CHECK(call191E4.wroteGp716Zero);
    CHECK(call191E4.wroteGp732Mode);
    CHECK(call191E4.modeKnown);
    CHECK(call191E4.mode == 3);
    CHECK(call191E4.called80017524);
    CHECK(call191E4.called80018FB0);
    CHECK(call191E4.callback80018E10 == 0x80018E10u);
    CHECK(call191E4.callback80019D7C == 0x80019D7Cu);
    CHECK(call191E4.stateMachineArg3 == 20);
    CHECK(call191E4.stateMachineArg4 == 3);
    CHECK(call191E4.called80017574);
    CHECK(call191E4.missingGp716AfterStateMachine);
    CHECK(!call191E4.gp716AfterStateMachineKnown);
    CHECK(!call191E4.wordA1Plus44Known);
    CHECK(!call191E4.resultKnown);
    CHECK(call19414.resultKnown);
    CHECK(call19414.called80019284);
    CHECK(static_cast<uint32_t>(call19414.result) == 0x80049278u);
    CHECK(call19414.event6HostArgPtr == out.tableStorage);
}

PrSceneEntryCardFeedbackDirect::Case17CardReadHalFeedback80019D7C
MakeTypedCase17ReadFeedback(bool readSucceeded) {
    using namespace PrSceneEntryCardFeedbackDirect;
    Case17CardReadHalFeedback80019D7C input{};
    input.word8007ABE4Known = true;
    input.word8007ABE4 = 1;
    for (CardReadAttempt800179B4& row : input.rows) {
        row.rowEnabledKnown = true;
        row.rowEnabled = false;
    }

    CardReadAttempt800179B4& row = input.rows[0];
    row.rowEnabled = true;
    row.rowNameKnown = true;
    std::memcpy(row.rowName, "BASCUS-94183A", 14u);
    row.rowNameBuffer8007CBE8Known = true;
    row.cardSelectorKnown = true;
    row.pathBuilt800173A8 = true;
    row.openAttempted800173A8 = true;
    row.fdKnown800173A8 = true;
    row.fd800173A8 = 5;
    row.targetBufferKnown = true;
    row.targetBufferAddress = kCardBlockBufferAddr8007ABE8;
    row.readLengthKnown = true;
    row.readLength = kCardReadBlockBytes800179B4;
    row.payloadPointerKnown = true;
    row.payloadPointer = kCardSavePayloadAddr8007ADE8;
    row.payloadPassedTo800164F8 = readSucceeded;
    row.blockCountKnown = true;
    row.blockCount = 1;
    row.byteCountKnown = true;
    row.byteCount = kCardReadBlockBytes800179B4;
    row.clearSwEventsKnown80016FC0 = true;
    row.readSubmittedKnown = true;
    row.readSubmitted = true;
    row.eventResultKnown80016EB8 = true;
    row.eventResult80016EB8 = readSucceeded ? 1 : 2;
    row.closeKnown = true;
    row.closeFdKnown = true;
    row.closeFd = 5;
    if (readSucceeded) {
        row.blockBytesKnown = true;
        row.blockBytes[kCardSavePayloadOffset8007ADE8 + 1u] = 'A';
        row.blockBytes[kCardSavePayloadOffset8007ADE8 + 2u] = 'B';
        row.blockBytes[kCardSavePayloadOffset8007ADE8 + 3u] = 'C';
        row.blockBytes[kCardSavePayloadOffset8007ADE8 + 4u] = 0;
        const size_t scoreOffset =
            kCardSavePayloadOffset8007ADE8 +
            PrSceneEntryDirect::kHiScoreSavePayloadScoreBase800164F8;
        row.blockBytes[scoreOffset + 0u] = 123u;
    }
    return input;
}

void TestCase17EmptyAndTypedFailuresStillReach19414() {
    using namespace PrSceneEntryCardFeedbackDirect;
    const PrStage1SaveStatusPrefix80092F10 prefix = MakeStatusPrefix(true);

    Case17CardReadHalFeedback80019D7C empty{};
    empty.word8007ABE4Known = true;
    empty.word8007ABE4 = 0;
    Case17To19414FeedbackBuildResult80015788 emptyBridge{};
    BuildFeedback80019414FromCase17CardReadFacts(
        prefix,
        true,
        17,
        empty,
        &emptyBridge);
    CHECK(emptyBridge.completed);
    CHECK(!emptyBridge.gap);
    CHECK(emptyBridge.case17.gp720Written);
    CHECK(emptyBridge.case17.gp720 == 1);
    CHECK(emptyBridge.case17.resultKnown);
    CHECK(emptyBridge.case17.result == 23);
    CHECK(emptyBridge.adapter.call80019284.resultKnown);

    const Case17CardReadHalFeedback80019D7C failed =
        MakeTypedCase17ReadFeedback(false);
    Case17To19414FeedbackBuildResult80015788 failedBridge{};
    BuildFeedback80019414FromCase17CardReadFacts(
        prefix,
        true,
        17,
        failed,
        &failedBridge);
    CHECK(failedBridge.completed);
    CHECK(!failedBridge.gap);
    CHECK(!failedBridge.missingHalFacts800179B4);
    CHECK(failedBridge.case17.cardRows[0].readResultKnown);
    CHECK(!failedBridge.case17.cardRows[0].readSucceeded);
    CHECK(failedBridge.case17.gp720 == 1);
    CHECK(failedBridge.case17.result == 23);
    const PrSceneEntryDirect::Call80019414Result80015788 failedCall19414 =
        PrSceneEntryDirect::PsxCall80019414_HiScoreEntry80015788(
            static_cast<int32_t>(prefix.psxAddress),
            true,
            failedBridge.adapter.feedback);
    CHECK(failedBridge.adapter.call800191E4ReturnUnresolvedCurrentIda);
    CHECK(failedCall19414.call800191E4.missingGp716AfterStateMachine);
    CHECK(!failedCall19414.call800191E4.gp716AfterStateMachineKnown);
    CHECK(!failedCall19414.call800191E4.wordA1Plus44Known);
    CHECK(!failedCall19414.call800191E4.resultKnown);
    CHECK(failedCall19414.called80019284);
    CHECK(failedCall19414.resultKnown);
    CHECK(static_cast<uint32_t>(failedCall19414.result) == 0x80049278u);
}

void TestCase17OriginalTypedSuccessExposesPayloadWithoutLiveFastPath() {
    using namespace PrSceneEntryCardFeedbackDirect;
    const PrStage1SaveStatusPrefix80092F10 prefix = MakeStatusPrefix(true);
    const Case17CardReadHalFeedback80019D7C success =
        MakeTypedCase17ReadFeedback(true);
    Case17To19414FeedbackBuildResult80015788 bridge{};
    BuildFeedback80019414FromCase17CardReadFacts(
        prefix,
        true,
        17,
        success,
        &bridge);

    CHECK(bridge.completed);
    CHECK(!bridge.gap);
    CHECK(bridge.case17.cardRows[0].readSucceeded);
    CHECK(bridge.case17.cardRows[0].mergeCalled);
    CHECK(bridge.case17.bank.rows[0].slots[0].scoreKnown);
    CHECK(bridge.case17.bank.rows[0].slots[0].score == 123);
    CHECK(bridge.adapter.completed);
}

void TestPracticeYCompoPathIdentity() {
    const PrSceneEntryDirect::SceneEntryPathIdentity identity =
        PrSceneEntryDirect::IdentifySceneEntryPathPtr(0x800113C8u);

    CHECK(identity.known);
    CHECK(identity.pathPtrKnown);
    CHECK(identity.pathPtr == 0x800113C8u);
    CHECK(identity.role ==
          PrSceneEntryDirect::SceneEntryPathRole::PracticeYCompo);
    CHECK(identity.psxPath != nullptr);
    if (identity.psxPath != nullptr) {
        CHECK(std::strcmp(identity.psxPath, "\\S0\\YCOMPO.INT;1") == 0);
    }
    CHECK(identity.relativeWinPath != nullptr);
    if (identity.relativeWinPath != nullptr) {
        CHECK(std::strcmp(identity.relativeWinPath, "S0/YCOMPO.INT") == 0);
    }
}

}  // namespace

int main() {
    TestUnknownStatusBankBlocksAdapterBeforeInputMemory();
    TestKnownStatusBankCompletesAdapter();
    TestCase17EmptyAndTypedFailuresStillReach19414();
    TestCase17OriginalTypedSuccessExposesPayloadWithoutLiveFastPath();
    TestPracticeYCompoPathIdentity();

    if (g_failed != 0) {
        std::printf(
            "test_ss0_scene_entry_feedback_adapter: failed checks=%d\n",
            g_failed);
        return 1;
    }
    std::printf("test_ss0_scene_entry_feedback_adapter: ok\n");
    return 0;
}
