#include "pr/pr_ss0_state16_runtime_one_shot_direct.h"

#include "pr/pr_pad.h"
#include "pr/pr_stage1_save_card_hal_direct.h"
#include "pr/pr_stage1_save_ui_direct.h"
#include "test_stage_payload_authority_fixture.h"

#include <algorithm>
#include <array>
#include <cstdio>

PrPadState PrPad::GetState(int port) {
    (void)port;
    return {};
}

namespace {

using namespace PrStage1SaveCardHalDirect;
using PrSS0State16RuntimeOneShotDirect::State16RuntimeTypedFactsOneShot;

int g_failedChecks = 0;

#define CHECK(expr)                                                        \
    do {                                                                   \
        if (!(expr)) {                                                     \
            std::printf("CHECK failed %s:%d: %s\n", __FILE__, __LINE__,    \
                        #expr);                                            \
            ++g_failedChecks;                                              \
        }                                                                  \
    } while (0)

using ReadBlock = std::array<uint8_t, kCardReadBlockBytes800179B4>;

ReadBlock MakeReadBlock(uint8_t seed) {
    ReadBlock block{};
    for (std::size_t i = 0; i < block.size(); ++i) {
        block[i] = static_cast<uint8_t>(seed + static_cast<uint8_t>(i));
    }
    return block;
}

void FillClearEvents(CardClearEventsFeedback80016FC0* out) {
    out->called = true;
    out->eventHandlesKnown = true;
    for (int i = 0; i < 4; ++i) {
        out->eventHandles[i] = static_cast<uint32_t>(i + 1);
        out->testEventResultsKnown[i] = true;
        out->testEventResults[i] = 0;
    }
}

void FillPoll(CardPollFeedback80016EB8* out) {
    out->called = true;
    out->eventHandlesKnown = true;
    for (int i = 0; i < 4; ++i) {
        out->eventHandles[i] = static_cast<uint32_t>(0x100 + i);
    }
    out->resultKnown = true;
    out->psxReturn = 1;
    out->timedOutKnown = true;
    out->timedOut = false;
    out->hitEventIndexKnown = true;
    out->hitEventIndex = 0;
    out->pollIterationCountKnown = true;
    out->pollIterationCount = 1;
    out->waitCallCountKnown80035560 = true;
    out->waitCallCount80035560 = 0;
}

CardReadFeedback800179B4 MakeCompleteFeedback(const ReadBlock& block) {
    CardReadFeedback800179B4 feedback{};
    feedback.feedbackKnown = true;
    feedback.word8007ABE4Known = true;
    feedback.word8007ABE4 = 5;
    for (int i = 0; i < kReadAttemptCount800179B4; ++i) {
        feedback.attempts[i].rowEnabledKnown = true;
        feedback.attempts[i].rowEnabled = false;
    }

    CardReadAttemptFeedback800179B4& attempt = feedback.attempts[4];
    attempt.rowEnabled = true;
    attempt.rowNameKnown = true;
    attempt.rowName[0] = 'G';
    attempt.rowName[1] = 'D';
    attempt.rowName[2] = 'B';
    attempt.rowNameBuffer8007CBE8Known = true;
    attempt.cardSelectorKnown = true;
    attempt.cardSelectorGp128 = 0;
    attempt.cardSelectorGp124 = 0;
    attempt.pathBuilt800173A8 = true;
    attempt.pathOpenFlagsKnown800173A8 = true;
    attempt.pathOpenFlags800173A8 = kCardPathOpenFlags800173A8;
    attempt.openAttempted800173A8 = true;
    attempt.fdKnown800173A8 = true;
    attempt.fd800173A8 = 5;
    attempt.gp696FdWriteKnown800173A8 = true;
    attempt.gp696Fd800173A8 = 5;
    attempt.targetBufferKnown = true;
    attempt.targetBufferAddress = kCardReadBlockBufferAddr800179B4;
    attempt.readLengthKnown = true;
    attempt.readLength = kCardReadBlockBytes800179B4;
    attempt.payloadPointerKnown = true;
    attempt.payloadPointer = kCardReadPayloadAddr8007ADE8;
    attempt.payloadPassedTo800164F8 = true;
    attempt.blockCountKnown = true;
    attempt.blockCount = kCardReadBlockCount800179B4;
    FillClearEvents(&attempt.clearEvents);
    attempt.readSubmission.called = true;
    attempt.readSubmission.fdKnown = true;
    attempt.readSubmission.fd = 5;
    attempt.readSubmission.bufferAddressKnown = true;
    attempt.readSubmission.bufferAddress = kCardReadBlockBufferAddr800179B4;
    attempt.readSubmission.byteCountKnown = true;
    attempt.readSubmission.byteCount = kCardReadBlockBytes800179B4;
    FillPoll(&attempt.poll);
    attempt.closeKnown800179B4 = true;
    attempt.closeFdKnown800179B4 = true;
    attempt.closeFd800179B4 = 5;
    attempt.blockBytesKnown = true;
    attempt.blockBytes = block.data();
    attempt.blockByteCount = block.size();
    return feedback;
}

State16CardReadRuntimeTypedFacts800179B4 MakeRuntimeTypedFacts(
    const ReadBlock& block,
    int32_t selectedBlock) {
    State16CardReadRuntimeTypedFacts800179B4 facts{};
    facts.factsKnown = true;
    facts.state16CallKnown = true;
    facts.selectedBlockKnown = true;
    facts.selectedBlockIndex = selectedBlock;
    facts.selectedTitleKnown = true;
    std::snprintf(facts.selectedTitle,
                  sizeof(facts.selectedTitle),
                  "BISLPS-00000GDB");
    facts.rowCountKnown = true;
    facts.rowCount = selectedBlock + 1;
    facts.arg2Known = true;
    facts.arg2 = 16;
    facts.nameAddressKnown = true;
    facts.nameAddress = kCardReadNameBufferAddr8007CBE8;
    facts.targetBufferAddressKnown = true;
    facts.targetBufferAddress = kCardReadBlockBufferAddr800179B4;
    facts.payloadAddressKnown = true;
    facts.payloadAddress = kCardReadPayloadAddr8007ADE8;
    facts.blockCountKnown = true;
    facts.blockCount = kCardReadBlockCount800179B4;
    facts.pathCallKnown = true;
    facts.cardSelectorKnown = true;
    facts.cardPortGp128 = 0;
    facts.cardSlotGp124 = 0;
    facts.gp696FdWriteKnown = true;
    facts.gp696Fd = 5;
    facts.clearEventsCallKnown = true;
    facts.readSubmissionKnown = true;
    facts.readFdKnown = true;
    facts.readFd = 5;
    facts.readBufferAddressKnown = true;
    facts.readBufferAddress = kCardReadBlockBufferAddr800179B4;
    facts.readByteCountKnown = true;
    facts.readByteCount = kCardReadBlockBytes800179B4;
    facts.pollCallKnown = true;
    facts.pollEventHandlesKnown80016EB8 = true;
    facts.pollEventHandle0_80016EB8 = 1;
    facts.pollEventHandle1_80016EB8 = 2;
    facts.pollEventHandle2_80016EB8 = 3;
    facts.pollEventHandle3_80016EB8 = 4;
    facts.pollResultKnown = true;
    facts.pollResult80016EB8 = 1;
    facts.pollTimedOutKnown = true;
    facts.pollTimedOut = false;
    facts.pollIterationCountKnown = true;
    facts.pollIterationCount = 1;
    facts.waitCallCountKnown80035560 = true;
    facts.waitCallCount80035560 = 0;
    facts.closeKnown = true;
    facts.closeFdKnown = true;
    facts.closeFd = 5;
    facts.returnKnown = true;
    facts.psxReturn800179B4 = 0;
    facts.payloadLoadCallKnown = true;
    facts.payloadArgumentKnown = true;
    facts.payloadArgument = kCardReadPayloadAddr8007ADE8;
    facts.fullPayloadBytesKnown = true;
    facts.fullPayloadBytes = block.data();
    facts.fullPayloadByteCount = block.size();
    return facts;
}

void TestQueueRejectsMissingOrShortPayload() {
    const ReadBlock block = MakeReadBlock(0x31u);
    State16RuntimeTypedFactsOneShot queue{};

    State16CardReadRuntimeTypedFacts800179B4 facts =
        MakeRuntimeTypedFacts(block, 4);
    facts.fullPayloadBytesKnown = false;
    CHECK(!PrSS0State16RuntimeOneShotDirect::
              QueueRuntimeState16CardReadTypedFactsOneShot(queue, facts));
    CHECK(!queue.pending);

    facts = MakeRuntimeTypedFacts(block, 4);
    facts.fullPayloadByteCount = block.size() - 1u;
    CHECK(!PrSS0State16RuntimeOneShotDirect::
              QueueRuntimeState16CardReadTypedFactsOneShot(queue, facts));
    CHECK(!queue.pending);
}

void TestRejectedRefreshClearsPendingRuntimeFacts() {
    const ReadBlock validBlock = MakeReadBlock(0x52u);
    const ReadBlock rejectedBlock = MakeReadBlock(0x68u);
    State16RuntimeTypedFactsOneShot queue{};

    CHECK(PrSS0State16RuntimeOneShotDirect::
              QueueRuntimeState16CardReadTypedFactsOneShot(
                  queue,
                  MakeRuntimeTypedFacts(validBlock, 4)));
    CHECK(queue.pending);
    CHECK(queue.fullPayloadBytes[0] == validBlock[0]);

    State16CardReadRuntimeTypedFacts800179B4 rejectedFacts =
        MakeRuntimeTypedFacts(rejectedBlock, 4);
    rejectedFacts.fullPayloadBytesKnown = false;
    CHECK(!PrSS0State16RuntimeOneShotDirect::
              QueueRuntimeState16CardReadTypedFactsOneShot(
                  queue,
                  rejectedFacts));
    CHECK(!queue.pending);

    ClearState16CardReadTypedCarrier800179B4();
    State16CardReadTypedCarrier800179B4 carrier{};
    CHECK(!PrSS0State16RuntimeOneShotDirect::
              ImportPendingState16TypedFactsBeforeConfirm(queue, 4));
    CHECK(!GetState16CardReadTypedCarrier800179B4(&carrier));
}

void TestQueueDeepCopiesAndMismatchConsumesOnce() {
    ReadBlock block = MakeReadBlock(0x42u);
    State16RuntimeTypedFactsOneShot queue{};
    const uint8_t originalFirstByte = block[0];
    CHECK(PrSS0State16RuntimeOneShotDirect::
              QueueRuntimeState16CardReadTypedFactsOneShot(
                  queue,
                  MakeRuntimeTypedFacts(block, 4)));
    block[0] = 0xFFu;
    CHECK(queue.pending);
    CHECK(queue.facts.fullPayloadBytes == queue.fullPayloadBytes.data());
    CHECK(queue.fullPayloadBytes[0] == originalFirstByte);

    State16CardReadTypedCarrier800179B4 carrier{};
    CHECK(!PrSS0State16RuntimeOneShotDirect::
              ImportPendingState16TypedFactsBeforeConfirm(queue, 5));
    CHECK(!queue.pending);
    CHECK(!GetState16CardReadTypedCarrier800179B4(&carrier));
}

void TestImportPublishesRuntimeCarrierAndConsumesOnce() {
    const ReadBlock block = MakeReadBlock(0xA7u);
    State16RuntimeTypedFactsOneShot queue{};
    CHECK(PrSS0State16RuntimeOneShotDirect::
              QueueRuntimeState16CardReadTypedFactsOneShot(
                  queue,
                  MakeRuntimeTypedFacts(block, 4)));
    CHECK(PrSS0State16RuntimeOneShotDirect::
              ImportPendingState16TypedFactsBeforeConfirm(queue, 4));
    CHECK(!queue.pending);

    State16CardReadTypedCarrier800179B4 carrier{};
    CHECK(GetState16CardReadTypedCarrier800179B4(&carrier));
    CHECK(carrier.source ==
          CardReadTypedCarrierSource800179B4::RuntimeLowerCardProducer);
    CHECK(carrier.producerWired800173A8_80016EB8_800179B4);
    CHECK(carrier.state16LoadPayloadLaneKnown);
    CHECK(carrier.selectedBlockKnown);
    CHECK(carrier.selectedBlockIndex == 4);
    CHECK(carrier.feedback.attempts[4].rowNameKnown);
    CHECK(carrier.feedback.attempts[4].rowName[0] == 'B');
    CHECK(carrier.feedback.attempts[4].rowName[1] == 'I');
    CHECK(carrier.typedReadSuccessKnown800179B4);
    CHECK(carrier.payloadBytesKnown8007ADE8);
    CHECK(!carrier.incomplete);
    CHECK(carrier.blockStorage[4][0] == block[0]);

    ClearState16CardReadTypedCarrier800179B4();
    CHECK(!PrSS0State16RuntimeOneShotDirect::
              ImportPendingState16TypedFactsBeforeConfirm(queue, 4));
    CHECK(!GetState16CardReadTypedCarrier800179B4(&carrier));
}

void TestDebugSyntheticFixtureIsNotRuntimeProducer() {
    const ReadBlock block = MakeReadBlock(0xC1u);
    const CardReadFeedback800179B4 feedback = MakeCompleteFeedback(block);
    PrStage1SaveUiDirect::Reset19148();
    CHECK(!PublishDebugState16CardReadTypedCarrier800179B4ForBlock(
        feedback,
        4));

    State16CardReadTypedCarrier800179B4 carrier{};
    CHECK(GetState16CardReadTypedCarrier800179B4(&carrier));
    CHECK(carrier.source ==
          CardReadTypedCarrierSource800179B4::DebugSyntheticFixture);
    CHECK(!carrier.producerWired800173A8_80016EB8_800179B4);
    CHECK(carrier.state16LoadPayloadLaneKnown);
    CHECK(carrier.selectedBlockKnown);
    CHECK(carrier.selectedBlockIndex == 4);
    CHECK(carrier.typedReadSuccessKnown800179B4);
    CHECK(carrier.payloadBytesKnown8007ADE8);

    const PrStage1SavePayloadProducerResult commit =
        PrStage1SaveUiDirect::CommitTypedPayload800164B4(
            kCardReadPayloadAddr8007ADE8,
            carrier.blockStorage[4].data(),
            PrStagePayloadBankDirect::kByteCount80092F10,
            carrier.payloadAuthority800164B4);
    CHECK(!commit.ok);
    const PrStage1SaveStatusPrefix80092F10 prefix =
        PrStage1SaveUiDirect::GetSaveStatusPrefix80092F10();
    CHECK(prefix.replayPayloadBackingProvenance80092F5C ==
          PrStagePayloadBankDirect::
              ReplayPayloadBackingProvenance80092F5C::Unknown);

    PrStage1SaveUiDirect::Reset19148();
    ClearState16CardReadTypedCarrier800179B4();
}

void TestRuntimeCarrierCommits800164B4Prefix() {
    ReadBlock block = MakeReadBlock(0xD4u);
    std::fill_n(block.begin(), kCardReadPayloadOffset8007ADE8, 0xA5u);
    State16RuntimeTypedFactsOneShot queue{};
    PrStage1SaveUiDirect::Reset19148();
    ClearState16CardReadTypedCarrier800179B4();

    CHECK(PrSS0State16RuntimeOneShotDirect::
              QueueRuntimeState16CardReadTypedFactsOneShot(
                  queue,
                  MakeRuntimeTypedFacts(block, 4)));
    CHECK(PrSS0State16RuntimeOneShotDirect::
              ImportPendingState16TypedFactsBeforeConfirm(queue, 4));

    State16CardReadTypedCarrier800179B4 carrier{};
    CHECK(GetState16CardReadTypedCarrier800179B4(&carrier));
    CHECK(carrier.source ==
          CardReadTypedCarrierSource800179B4::RuntimeLowerCardProducer);
    CHECK(carrier.producerWired800173A8_80016EB8_800179B4);
    CHECK(carrier.typedReadSuccessKnown800179B4);
    CHECK(carrier.payloadBytesKnown8007ADE8);

    const uint8_t* const payload =
        carrier.blockStorage[4].data() + kCardReadPayloadOffset8007ADE8;
    CHECK(carrier.payloadAuthority800164B4.Matches(
        kCardReadPayloadAddr8007ADE8,
        payload,
        PrStagePayloadBankDirect::kByteCount80092F10));
    CHECK(!carrier.payloadAuthority800164B4.Matches(
        kCardReadPayloadAddr8007ADE8,
        carrier.blockStorage[4].data(),
        PrStagePayloadBankDirect::kByteCount80092F10));

    const PrStage1SavePayloadProducerResult commit =
        PrStage1SaveUiDirect::CommitTypedPayload800164B4(
            kCardReadPayloadAddr8007ADE8,
            payload,
            PrStagePayloadBankDirect::kByteCount80092F10,
            carrier.payloadAuthority800164B4);
    CHECK(commit.ok);
    CHECK(commit.payloadKnown);
    CHECK(!commit.helperGap);

    const PrStage1SaveStatusPrefix80092F10 prefix =
        PrStage1SaveUiDirect::GetSaveStatusPrefix80092F10();
    CHECK(prefix.known);
    CHECK(prefix.statusBankKnown80092F1D);
    CHECK(prefix.psxAddress == PrStage1SaveStatusPrefix80092F10::kPsxAddress);
    CHECK(prefix.byteCount == PrStagePayloadBankDirect::kByteCount80092F10);
    CHECK(prefix.lastWriterFunction == PrStagePayloadBankDirect::kFn800164B4);
    CHECK(prefix.wrote800164B4);
    CHECK(!prefix.wrote8001635C);
    CHECK(prefix.replayPayloadBackingProvenance80092F5C ==
          PrStagePayloadBankDirect::
              ReplayPayloadBackingProvenance80092F5C::TypedCardCopy800164B4);
    CHECK(prefix.bytes[0] == block[kCardReadPayloadOffset8007ADE8]);
    CHECK(prefix.bytes[0x0Du] ==
          block[kCardReadPayloadOffset8007ADE8 + 0x0Du]);
    CHECK(prefix.bytes[PrStagePayloadBankDirect::kByteCount80092F10 - 1u] ==
          block[kCardReadPayloadOffset8007ADE8 +
                PrStagePayloadBankDirect::kByteCount80092F10 - 1u]);

    PrStage1SaveUiDirect::Reset19148();
    ClearState16CardReadTypedCarrier800179B4();
}

void TestTypedPayloadRollback15744PreservesProvenance() {
    const ReadBlock original = MakeReadBlock(0x24u);
    const ReadBlock replacement = MakeReadBlock(0x91u);
    PrStage1SaveUiDirect::Reset19148();

    PrStagePayloadBankDirect::LoadSavePayloadAuthority800164B4
        originalAuthority{};
    PrStagePayloadBankDirect::LoadSavePayloadAuthority800164B4
        replacementAuthority{};
    CHECK(TestStagePayloadAuthorityFixture::
              MintRuntimeLowerCardPayloadAuthority800164B4(
                  original.data(),
                  PrStagePayloadBankDirect::kByteCount80092F10,
                  &originalAuthority));
    CHECK(TestStagePayloadAuthorityFixture::
              MintRuntimeLowerCardPayloadAuthority800164B4(
                  replacement.data(),
                  PrStagePayloadBankDirect::kByteCount80092F10,
                  &replacementAuthority));
    CHECK(PrStage1SaveUiDirect::CommitTypedPayload800164B4(
              kCardReadPayloadAddr8007ADE8,
              original.data(),
              PrStagePayloadBankDirect::kByteCount80092F10,
              originalAuthority)
              .ok);
    CHECK(PrStage1SaveUiDirect::Sub80015700(
              PrStage1SaveStatusPrefix80092F10::kPsxAddress)
              .ok);
    CHECK(PrStage1SaveUiDirect::CommitTypedPayload800164B4(
              kCardReadPayloadAddr8007ADE8,
              replacement.data(),
              PrStagePayloadBankDirect::kByteCount80092F10,
              replacementAuthority)
              .ok);

    const PrStage1SaveStatusBackupResult80015700 restored =
        PrStage1SaveUiDirect::Sub80015744(
            PrStage1SaveStatusPrefix80092F10::kPsxAddress);
    CHECK(restored.ok);
    CHECK(restored.restoreKnown);
    const PrStage1SaveStatusPrefix80092F10 prefix =
        PrStage1SaveUiDirect::GetSaveStatusPrefix80092F10();
    CHECK(prefix.known);
    CHECK(prefix.bytes[0] == original[0]);
    CHECK(prefix.bytes[PrStagePayloadBankDirect::kByteCount80092F10 - 1u] ==
          original[PrStagePayloadBankDirect::kByteCount80092F10 - 1u]);
    for (size_t i = 0;
         i < PrStagePayloadBankDirect::kByteCount80092F10;
         ++i) {
        CHECK(prefix.bytes[i] == original[i]);
    }
    CHECK(prefix.replayPayloadBackingProvenance80092F5C ==
          PrStagePayloadBankDirect::
              ReplayPayloadBackingProvenance80092F5C::TypedCardCopy800164B4);
    CHECK(PrStagePayloadBankDirect::
              IsKnownReplayRestorePayloadProvenance80092F48_80092F5C(
                  prefix.replayPayloadBackingProvenance80092F5C));

    PrStage1SaveUiDirect::Reset19148();
}

}  // namespace

int main() {
    TestQueueRejectsMissingOrShortPayload();
    TestRejectedRefreshClearsPendingRuntimeFacts();
    TestQueueDeepCopiesAndMismatchConsumesOnce();
    TestImportPublishesRuntimeCarrierAndConsumesOnce();
    TestDebugSyntheticFixtureIsNotRuntimeProducer();
    TestRuntimeCarrierCommits800164B4Prefix();
    TestTypedPayloadRollback15744PreservesProvenance();
    if (g_failedChecks != 0) {
        std::printf("test_ss0_state16_runtime_one_shot: %d failed checks\n",
                    g_failedChecks);
        return 1;
    }
    std::puts("test_ss0_state16_runtime_one_shot: ok");
    return 0;
}
