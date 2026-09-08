#include "pr/pr_stage1_save_card_hal_direct.h"

#include <array>
#include <cstdint>
#include <cstdio>
#include <cstring>

using namespace PrStage1SaveCardHalDirect;

namespace {

int g_failed = 0;

#define CHECK(expr)                                                           \
    do {                                                                      \
        if (!(expr)) {                                                        \
            std::printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr);       \
            ++g_failed;                                                       \
        }                                                                     \
    } while (0)

using ReadBlock = std::array<uint8_t, kCardReadBlockBytes800179B4>;
using DirectCardImage = std::array<uint8_t, 128u * 1024u>;

DirectCardImage g_directCardImage{};
DirectCardImage g_badDirectCardImage{};

ReadBlock MakeReadBlock(uint8_t seed) {
    ReadBlock block{};
    for (std::size_t i = 0; i < block.size(); ++i) {
        block[i] = static_cast<uint8_t>(seed + (i & 0xffu));
    }
    return block;
}

void PutDirectCardImageEntry(
    DirectCardImage& image,
    int32_t blockIndex,
    const char* title,
    const ReadBlock& block) {
    const std::size_t dirOffset =
        static_cast<std::size_t>(blockIndex + 1) * 128u;
    image[dirOffset + 0x00u] = 0x51u;
    image[dirOffset + 0x04u] = 0x00u;
    image[dirOffset + 0x05u] = 0x20u;
    for (std::size_t i = 0; i < 19u && title[i] != '\0'; ++i) {
        image[dirOffset + 0x0Au + i] =
            static_cast<uint8_t>(title[i]);
    }
    const std::size_t blockOffset =
        static_cast<std::size_t>(blockIndex + 1) *
        kCardReadBlockBytes800179B4;
    for (std::size_t i = 0; i < block.size(); ++i) {
        image[blockOffset + i] = block[i];
    }
}

void FillDirectCardImage(
    DirectCardImage& image,
    int32_t blockIndex,
    const char* title,
    const ReadBlock& block) {
    image.fill(0);
    PutDirectCardImageEntry(image, blockIndex, title, block);
}

PrStage1SaveUiCardImagePersistenceView8007A318 MakeDirectCardImageView(
    const DirectCardImage& image,
    int32_t blockIndex) {
    PrStage1SaveUiCardImagePersistenceView8007A318 view{};
    view.known = true;
    view.slotPolicyKnown = true;
    view.blockIndex = blockIndex;
    view.durablePolicyKnown = false;
    view.durableCommitted = false;
    view.byteSize = static_cast<uint32_t>(image.size());
    view.bytes = image.data();
    view.byteCount = image.size();
    return view;
}

void SetRowName(char (&rowName)[32], const char* text) {
    std::size_t i = 0;
    for (; i + 1u < sizeof(rowName) && text[i] != '\0'; ++i) {
        rowName[i] = text[i];
    }
    for (; i < sizeof(rowName); ++i) {
        rowName[i] = '\0';
    }
}

CardReadFeedbackRequest800179B4 MakeReadRequest() {
    return MakeCase17HiScoreReadRequest800179B4(17);
}

void FillClearEvents(CardClearEventsFeedback80016FC0* out) {
    out->called = true;
    out->eventHandlesKnown = true;
    for (int32_t i = 0; i < 4; ++i) {
        out->eventHandles[i] = static_cast<uint32_t>(0x1000 + i);
        out->testEventResultsKnown[i] = true;
        out->testEventResults[i] = 0;
    }
}

void FillPoll(CardPollFeedback80016EB8* out, int32_t eventResult) {
    out->called = true;
    out->eventHandlesKnown = true;
    for (int32_t i = 0; i < 4; ++i) {
        out->eventHandles[i] = static_cast<uint32_t>(0x2000 + i);
    }
    out->resultKnown = true;
    out->psxReturn = eventResult;
    out->timedOutKnown = true;
    out->timedOut = false;
    out->hitEventIndexKnown = true;
    out->hitEventIndex = eventResult - 1;
    out->pollIterationCountKnown = true;
    out->pollIterationCount = eventResult;
    out->waitCallCountKnown80035560 = true;
    out->waitCallCount80035560 = eventResult - 1;
}

void MarkUnusedRowsSkipped(CardReadFeedback800179B4* feedback) {
    for (int32_t i = 1; i < kReadAttemptCount800179B4; ++i) {
        feedback->attempts[i].rowEnabledKnown = true;
        feedback->attempts[i].rowEnabled = false;
    }
}

void FillTypedReadAttempt(CardReadAttemptFeedback800179B4* out,
                          const ReadBlock& block,
                          bool success) {
    out->rowEnabledKnown = true;
    out->rowEnabled = true;
    out->rowNameKnown = true;
    SetRowName(out->rowName, "BISLPS-00000READ");
    out->rowNameBuffer8007CBE8Known = true;
    out->cardSelectorKnown = true;
    out->cardSelectorGp128 = 0;
    out->cardSelectorGp124 = 0;
    out->pathBuilt800173A8 = true;
    out->pathOpenFlagsKnown800173A8 = true;
    out->pathOpenFlags800173A8 = kCardPathOpenFlags800173A8;
    out->openAttempted800173A8 = true;
    out->fdKnown800173A8 = true;
    out->fd800173A8 = 5;
    out->gp696FdWriteKnown800173A8 = true;
    out->gp696Fd800173A8 = 5;
    out->targetBufferKnown = true;
    out->targetBufferAddress = kCardReadBlockBufferAddr800179B4;
    out->readLengthKnown = true;
    out->readLength = kCardReadBlockBytes800179B4;
    out->payloadPointerKnown = true;
    out->payloadPointer = kCardReadPayloadAddr8007ADE8;
    out->payloadPassedTo800164F8 = success;
    out->blockCountKnown = true;
    out->blockCount = kCardReadBlockCount800179B4;
    FillClearEvents(&out->clearEvents);
    out->readSubmission.called = true;
    out->readSubmission.fdKnown = true;
    out->readSubmission.fd = 5;
    out->readSubmission.bufferAddressKnown = true;
    out->readSubmission.bufferAddress = kCardReadBlockBufferAddr800179B4;
    out->readSubmission.byteCountKnown = true;
    out->readSubmission.byteCount = kCardReadBlockBytes800179B4;
    FillPoll(&out->poll, success ? 1 : 2);
    out->closeKnown800179B4 = true;
    out->closeFdKnown800179B4 = true;
    out->closeFd800179B4 = 5;
    out->blockBytesKnown = true;
    out->blockBytes = block.data();
    out->blockByteCount = block.size();
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

void TestState16ReadRequestShapeDoesNotProduceSuccess() {
    CardReadFeedbackRequest800179B4 request =
        MakeState16LoadPayloadReadRequest800179B4(16);
    CHECK(request.callFunction80019414 == kFn80019414);
    CHECK(request.caseFunction80019D7C == kFn80019D7C);
    CHECK(request.state16LoadPayloadRequest80019D7C);
    CHECK(!request.case17HiScoreRequest80019D7C);
    CHECK(request.arg2Known);
    CHECK(request.arg2 == 16);
    CHECK(request.arg2 != kCase17Arg2EarlyReturn80019D7C);
    CHECK(request.psxFunction == kFn800179B4);
    CHECK(request.retryCount == kReadAttemptCount800179B4);
    CHECK(request.pathFunction800173A8 == kFn800173A8);
    CHECK(request.pathOpenFlags800173A8 == kCardPathOpenFlags800173A8);
    CHECK(request.clearEventsFunction80016FC0 == kFn80016FC0);
    CHECK(request.pollFunction80016EB8 == kFn80016EB8);
    CHECK(request.nameAddress == kCardReadNameBufferAddr8007CBE8);
    CHECK(request.targetBufferAddress == kCardReadBlockBufferAddr800179B4);
    CHECK(request.payloadAddress == kCardReadPayloadAddr8007ADE8);
    CHECK(request.blockCount == kCardReadBlockCount800179B4);
    CHECK(request.blockBytes == kCardReadBlockBytes800179B4);
    CHECK(request.closeGp696FactRequired);
    CHECK(request.closeGp696Address == kReadCloseGp696Address800179B4);
    CHECK(request.closeFdMustMatchGp696);

    CardReadFeedbackProducerInput800179B4 input{};
    input.requestKnown = true;
    input.request = request;

    CardReadFeedbackProducerResult800179B4 producer{};
    BuildCardReadFeedbackFromHostFacts800179B4(input, &producer);
    CHECK(!producer.produced);
    CHECK(producer.incomplete);
    CHECK(producer.requestUsed);
    CHECK(producer.requestMatched);
    CHECK(!producer.hostFactsUsed);
    CHECK(!producer.explicitFeedbackUsed);
    CHECK(producer.triggerChainKnown);
    CHECK(producer.arg2Known);
    CHECK(producer.arg2 == 16);
    CHECK(producer.rowNameBuffer8007CBE8Known);
    CHECK(producer.readLengthKnown);
    CHECK(producer.readLength == kCardReadBlockBytes800179B4);
    CHECK(producer.payloadPointerKnown);
    CHECK(producer.payloadPointer == kCardReadPayloadAddr8007ADE8);
    CHECK(producer.payloadPassedTo800164F8);
    CHECK(!producer.feedback.feedbackKnown);
}

void TestCase17ReadRequestShapeDoesNotEarlyReturn() {
    CardReadFeedbackRequest800179B4 request =
        MakeCase17HiScoreReadRequest800179B4(17);
    CHECK(request.callFunction80019414 == kFn80019414);
    CHECK(request.caseFunction80019D7C == kFn80019D7C);
    CHECK(!request.state16LoadPayloadRequest80019D7C);
    CHECK(request.case17HiScoreRequest80019D7C);
    CHECK(request.arg2Known);
    CHECK(request.arg2 == 17);
    CHECK(request.arg2 != kCase17Arg2EarlyReturn80019D7C);
    CHECK(request.psxFunction == kFn800179B4);
    CHECK(request.retryCount == kReadAttemptCount800179B4);
    CHECK(request.nameAddress == kCardReadNameBufferAddr8007CBE8);
    CHECK(request.targetBufferAddress == kCardReadBlockBufferAddr800179B4);
    CHECK(request.payloadAddress == kCardReadPayloadAddr8007ADE8);
    CHECK(request.closeGp696FactRequired);
}

void TestHostFactsCannotSynthesizeTypedReadSuccess() {
    const ReadBlock block = MakeReadBlock(0x31u);
    CardReadFeedbackProducerInput800179B4 input{};
    input.requestKnown = true;
    input.request = MakeReadRequest();
    input.hostFactsKnown = true;
    input.hostFacts.factsKnown = true;
    input.hostFacts.triggerChainKnown = true;
    input.hostFacts.arg2Known = true;
    input.hostFacts.arg2 = 17;
    input.hostFacts.word8007ABE4Known = true;
    input.hostFacts.word8007ABE4 = 1;
    input.hostFacts.rowNameBuffer8007CBE8Known = true;
    input.hostFacts.readBufferKnown = true;
    input.hostFacts.readBuffer = kCardReadBlockBufferAddr800179B4;
    input.hostFacts.readLengthKnown = true;
    input.hostFacts.readLength = kCardReadBlockBytes800179B4;
    input.hostFacts.payloadPointerKnown = true;
    input.hostFacts.payloadPointer = kCardReadPayloadAddr8007ADE8;
    input.hostFacts.payloadPassedTo800164F8 = true;
    input.hostFacts.attempts[0].rowEnabledKnown = true;
    input.hostFacts.attempts[0].rowEnabled = true;
    input.hostFacts.attempts[0].rowNameKnown = true;
    SetRowName(input.hostFacts.attempts[0].rowName, "BISLPS-00000HOST");
    input.hostFacts.attempts[0].rowNameBuffer8007CBE8Known = true;
    input.hostFacts.attempts[0].cardSelectorKnown = true;
    input.hostFacts.attempts[0].readBufferBytesKnown = true;
    input.hostFacts.attempts[0].readBufferBytes = block.data();
    input.hostFacts.attempts[0].readBufferByteCount = block.size();

    CardReadFeedbackProducerResult800179B4 producer{};
    BuildCardReadFeedbackFromHostFacts800179B4(input, &producer);
    CHECK(producer.produced);
    CHECK(!producer.incomplete);
    CHECK(producer.requestUsed);
    CHECK(producer.requestMatched);
    CHECK(producer.hostFactsUsed);
    CHECK(producer.feedback.feedbackKnown);
    const CardReadAttemptFeedback800179B4& attempt =
        producer.feedback.attempts[0];
    CHECK(attempt.rowEnabledKnown);
    CHECK(attempt.rowEnabled);
    CHECK(attempt.rowNameKnown);
    CHECK(attempt.rowNameBuffer8007CBE8Known);
    CHECK(attempt.cardSelectorKnown);
    CHECK(attempt.targetBufferKnown);
    CHECK(attempt.readLengthKnown);
    CHECK(attempt.payloadPointerKnown);
    CHECK(attempt.payloadPassedTo800164F8);
    CHECK(attempt.blockBytesKnown);
    CHECK(!attempt.liveCase17PayloadViewKnown);
    CHECK(!attempt.successAuthorityKnown800179B4);
    CHECK(!attempt.success800179B4);
    CHECK(!attempt.poll.resultKnown);
    CHECK(!attempt.pathBuilt800173A8);
    CHECK(!attempt.fdKnown800173A8);
    CHECK(!attempt.closeKnown800179B4);

    CardReadHalBuildResult800179B4 hal{};
    BuildCardReadHalResult800179B4(producer.feedback, &hal);
    CHECK(hal.produced);
    CHECK(hal.incomplete);
    CHECK(!hal.anyPayloadBytesAvailable);
    CHECK(!hal.attempts[0].payloadBytesAvailable);
    CHECK(!hal.attempts[0].readSucceeded);
}

void TestExplicitTypedReadSuccessProducesPayloadOnlyAfterPollSuccess() {
    const ReadBlock block = MakeReadBlock(0x51u);
    CardReadFeedback800179B4 feedback{};
    feedback.feedbackKnown = true;
    feedback.word8007ABE4Known = true;
    feedback.word8007ABE4 = 1;
    MarkUnusedRowsSkipped(&feedback);
    FillTypedReadAttempt(&feedback.attempts[0], block, true);

    CardReadHalBuildResult800179B4 hal{};
    BuildCardReadHalResult800179B4(feedback, &hal);
    CHECK(hal.produced);
    CHECK(!hal.incomplete);
    CHECK(hal.anyPayloadBytesAvailable);
    const CardReadAttemptResult800179B4& row = hal.attempts[0];
    CHECK(row.produced);
    CHECK(!row.incomplete);
    CHECK(row.readSubmitted800173A8);
    CHECK(row.readSucceeded);
    CHECK(row.payloadBytesAvailable);
    CHECK(row.psxReturn800179B4 == 0);
    CHECK(row.eventResult80016EB8 == 1);
    CHECK(row.blockBytes == block.data());
    CHECK(row.blockByteCount == block.size());
    CHECK(row.payloadPointerKnown);
    CHECK(row.payloadPointer == kCardReadPayloadAddr8007ADE8);
    CHECK(row.payloadPassedTo800164F8);
}

void TestExplicitTypedReadFailureDoesNotExposePayloadBytes() {
    const ReadBlock block = MakeReadBlock(0x71u);
    CardReadFeedback800179B4 feedback{};
    feedback.feedbackKnown = true;
    feedback.word8007ABE4Known = true;
    feedback.word8007ABE4 = 1;
    MarkUnusedRowsSkipped(&feedback);
    FillTypedReadAttempt(&feedback.attempts[0], block, false);

    CardReadHalBuildResult800179B4 hal{};
    BuildCardReadHalResult800179B4(feedback, &hal);
    CHECK(hal.produced);
    CHECK(!hal.incomplete);
    CHECK(!hal.anyPayloadBytesAvailable);
    const CardReadAttemptResult800179B4& row = hal.attempts[0];
    CHECK(row.produced);
    CHECK(!row.incomplete);
    CHECK(row.readSubmitted800173A8);
    CHECK(!row.readSucceeded);
    CHECK(!row.payloadBytesAvailable);
    CHECK(row.psxReturn800179B4 == -1);
    CHECK(row.eventResult80016EB8 == 2);
    CHECK(!row.payloadPassedTo800164F8);
}

void TestLiveCase17AuthorityRequiresPayloadBytes() {
    CardReadFeedback800179B4 feedback{};
    feedback.feedbackKnown = true;
    feedback.word8007ABE4Known = true;
    feedback.word8007ABE4 = 1;
    MarkUnusedRowsSkipped(&feedback);
    CardReadAttemptFeedback800179B4& attempt = feedback.attempts[0];
    attempt.rowEnabledKnown = true;
    attempt.rowEnabled = true;
    attempt.liveCase17PayloadViewKnown = true;
    attempt.successAuthorityKnown800179B4 = true;
    attempt.success800179B4 = true;
    attempt.targetBufferKnown = true;
    attempt.targetBufferAddress = kCardReadBlockBufferAddr800179B4;
    attempt.readLengthKnown = true;
    attempt.readLength = kCardReadBlockBytes800179B4;
    attempt.payloadPointerKnown = true;
    attempt.payloadPointer = kCardReadPayloadAddr8007ADE8;
    attempt.payloadPassedTo800164F8 = true;
    attempt.blockBytesKnown = false;

    CardReadHalBuildResult800179B4 hal{};
    BuildCardReadHalResult800179B4(feedback, &hal);
    CHECK(hal.produced);
    CHECK(hal.incomplete);
    CHECK(!hal.anyPayloadBytesAvailable);
    CHECK(!hal.attempts[0].produced);
    CHECK(hal.attempts[0].incomplete);
    CHECK(!hal.attempts[0].readSucceeded);
    CHECK(!hal.attempts[0].payloadBytesAvailable);
}

void TestRowCoverageAloneCannotProduceReadAuthority() {
    CardReadFeedback800179B4 feedback{};
    feedback.feedbackKnown = true;
    feedback.word8007ABE4Known = true;
    feedback.word8007ABE4 = 1;
    MarkUnusedRowsSkipped(&feedback);
    feedback.attempts[0].rowEnabledKnown = true;
    feedback.attempts[0].rowEnabled = true;
    feedback.attempts[0].rowNameKnown = true;
    SetRowName(feedback.attempts[0].rowName, "BISLPS-00000ROW");
    feedback.attempts[0].cardSelectorKnown = true;

    CardReadHalBuildResult800179B4 hal{};
    BuildCardReadHalResult800179B4(feedback, &hal);
    CHECK(hal.produced);
    CHECK(hal.incomplete);
    CHECK(!hal.anyPayloadBytesAvailable);
    CHECK(!hal.attempts[0].payloadBytesAvailable);
    CHECK(!hal.attempts[0].readSucceeded);
}

void TestRequestRejectsEarlyReturnArg2() {
    CardReadFeedbackProducerInput800179B4 input{};
    input.requestKnown = true;
    input.request = MakeReadRequest();
    input.request.arg2 = kCase17Arg2EarlyReturn80019D7C;
    input.hostFactsKnown = true;
    input.hostFacts.factsKnown = true;

    CardReadFeedbackProducerResult800179B4 producer{};
    BuildCardReadFeedbackFromHostFacts800179B4(input, &producer);
    CHECK(!producer.produced);
    CHECK(producer.incomplete);
    CHECK(producer.requestUsed);
    CHECK(!producer.requestMatched);
    CHECK(!producer.hostFactsUsed);
}

void TestState16ProducerRejectsMismatchedRequestWithExplicitFeedback() {
    const ReadBlock block = MakeReadBlock(0xD1u);
    CardReadFeedback800179B4 successFeedback{};
    successFeedback.feedbackKnown = true;
    successFeedback.word8007ABE4Known = true;
    successFeedback.word8007ABE4 = 1;
    MarkUnusedRowsSkipped(&successFeedback);
    FillTypedReadAttempt(&successFeedback.attempts[0], block, true);

    CardReadFeedbackProducerInput800179B4 input{};
    input.requestKnown = true;
    input.request = MakeState16LoadPayloadReadRequest800179B4(16);
    input.request.payloadAddress = kCardReadPayloadAddr8007ADE8 + 0x10u;
    input.explicitFeedbackKnown = true;
    input.explicitFeedback = successFeedback;

    CardReadFeedbackProducerResult800179B4 producer{};
    BuildCardReadFeedbackFromHostFacts800179B4(input, &producer);
    CHECK(!producer.produced);
    CHECK(producer.incomplete);
    CHECK(producer.requestUsed);
    CHECK(!producer.requestMatched);
    CHECK(!producer.explicitFeedbackUsed);
    CHECK(!producer.hostFactsUsed);
    CHECK(!producer.feedback.feedbackKnown);
}

void TestState16ProducerRejectsCase17ShapedArg2Request() {
    const ReadBlock block = MakeReadBlock(0xD2u);
    CardReadFeedback800179B4 successFeedback{};
    successFeedback.feedbackKnown = true;
    successFeedback.word8007ABE4Known = true;
    successFeedback.word8007ABE4 = 1;
    MarkUnusedRowsSkipped(&successFeedback);
    FillTypedReadAttempt(&successFeedback.attempts[0], block, true);

    CardReadFeedbackProducerInput800179B4 input{};
    input.requestKnown = true;
    input.request.arg2Known = true;
    input.request.arg2 = 16;
    input.explicitFeedbackKnown = true;
    input.explicitFeedback = successFeedback;

    CardReadFeedbackProducerResult800179B4 producer{};
    BuildCardReadFeedbackFromHostFacts800179B4(input, &producer);
    CHECK(!producer.produced);
    CHECK(producer.incomplete);
    CHECK(producer.requestUsed);
    CHECK(!producer.requestMatched);
    CHECK(!producer.explicitFeedbackUsed);
    CHECK(!producer.hostFactsUsed);
    CHECK(!producer.feedback.feedbackKnown);
}

void TestCase17TypedCarrierRequiresExplicitTypedRead() {
    ClearCase17CardReadTypedCarrier800179B4();
    Case17CardReadTypedCarrier800179B4 carrier{};
    CHECK(!GetCase17CardReadTypedCarrier800179B4(&carrier));

    const ReadBlock successBlock = MakeReadBlock(0x91u);
    CardReadFeedback800179B4 successFeedback{};
    successFeedback.feedbackKnown = true;
    successFeedback.word8007ABE4Known = true;
    successFeedback.word8007ABE4 = 1;
    MarkUnusedRowsSkipped(&successFeedback);
    FillTypedReadAttempt(&successFeedback.attempts[0], successBlock, true);

    CHECK(!PublishCase17CardReadTypedCarrier800179B4(successFeedback));
    CHECK(GetCase17CardReadTypedCarrier800179B4(&carrier));
    CHECK(carrier.known);
    CHECK(carrier.source == CardReadTypedCarrierSource800179B4::Unknown);
    CHECK(!carrier.producerWired800173A8_80016EB8_800179B4);
    CHECK(!carrier.case17LoopCompletionKnown80019D7C);
    CHECK(!carrier.case17HiScorePayloadLaneKnown);
    CHECK(carrier.typedReadSuccessKnown800179B4);
    CHECK(carrier.payloadBytesKnown8007ADE8);
    CHECK(!carrier.incomplete);
    CHECK(carrier.hal.produced);
    CHECK(carrier.hal.anyPayloadBytesAvailable);
    CHECK(carrier.hal.attempts[0].readSucceeded);
    CHECK(carrier.hal.attempts[0].payloadBytesAvailable);
    CHECK(carrier.hal.attempts[0].blockBytes ==
          carrier.blockStorage[0].data());
    CHECK(carrier.feedback.attempts[0].blockBytes ==
          carrier.blockStorage[0].data());
    CHECK(carrier.blockStorage[0][0] == successBlock[0]);

    CHECK(PublishCase17CardReadTypedCarrier800179B4(
        successFeedback,
        CardReadTypedCarrierSource800179B4::RuntimeLowerCardProducer));
    CHECK(GetCase17CardReadTypedCarrier800179B4(&carrier));
    CHECK(carrier.known);
    CHECK(carrier.source ==
          CardReadTypedCarrierSource800179B4::RuntimeLowerCardProducer);
    CHECK(carrier.producerWired800173A8_80016EB8_800179B4);
    CHECK(carrier.case17LoopCompletionKnown80019D7C);
    CHECK(carrier.case17HiScorePayloadLaneKnown);
    CHECK(carrier.typedReadSuccessKnown800179B4);
    CHECK(carrier.payloadBytesKnown8007ADE8);
    CHECK(!carrier.incomplete);
    CHECK(carrier.hal.produced);
    CHECK(carrier.hal.anyPayloadBytesAvailable);
    CHECK(carrier.hal.attempts[0].readSucceeded);
    CHECK(carrier.hal.attempts[0].payloadBytesAvailable);
    CHECK(carrier.hal.attempts[0].blockBytes ==
          carrier.blockStorage[0].data());
    CHECK(carrier.feedback.attempts[0].blockBytes ==
          carrier.blockStorage[0].data());
    CHECK(carrier.blockStorage[0][0] == successBlock[0]);

    const ReadBlock failBlock = MakeReadBlock(0xA1u);
    CardReadFeedback800179B4 failFeedback{};
    failFeedback.feedbackKnown = true;
    failFeedback.word8007ABE4Known = true;
    failFeedback.word8007ABE4 = 1;
    MarkUnusedRowsSkipped(&failFeedback);
    FillTypedReadAttempt(&failFeedback.attempts[0], failBlock, false);

    CHECK(!PublishCase17CardReadTypedCarrier800179B4(failFeedback));
    CHECK(GetCase17CardReadTypedCarrier800179B4(&carrier));
    CHECK(carrier.source == CardReadTypedCarrierSource800179B4::Unknown);
    CHECK(!carrier.producerWired800173A8_80016EB8_800179B4);
    CHECK(!carrier.case17LoopCompletionKnown80019D7C);
    CHECK(!carrier.case17HiScorePayloadLaneKnown);
    CHECK(!carrier.typedReadSuccessKnown800179B4);
    CHECK(!carrier.payloadBytesKnown8007ADE8);
    CHECK(!carrier.incomplete);
    CHECK(!carrier.hal.anyPayloadBytesAvailable);

    CHECK(PublishCase17CardReadTypedCarrier800179B4(
        failFeedback,
        CardReadTypedCarrierSource800179B4::RuntimeLowerCardProducer));
    CHECK(GetCase17CardReadTypedCarrier800179B4(&carrier));
    CHECK(carrier.producerWired800173A8_80016EB8_800179B4);
    CHECK(carrier.case17LoopCompletionKnown80019D7C);
    CHECK(!carrier.case17HiScorePayloadLaneKnown);
    CHECK(!carrier.typedReadSuccessKnown800179B4);
    CHECK(!carrier.payloadBytesKnown8007ADE8);
    CHECK(!carrier.incomplete);

    CardReadFeedback800179B4 emptyFeedback{};
    emptyFeedback.feedbackKnown = true;
    emptyFeedback.word8007ABE4Known = true;
    emptyFeedback.word8007ABE4 = 0;
    CHECK(PublishCase17CardReadTypedCarrier800179B4(
        emptyFeedback,
        CardReadTypedCarrierSource800179B4::RuntimeLowerCardProducer));
    CHECK(GetCase17CardReadTypedCarrier800179B4(&carrier));
    CHECK(carrier.producerWired800173A8_80016EB8_800179B4);
    CHECK(carrier.case17LoopCompletionKnown80019D7C);
    CHECK(!carrier.case17HiScorePayloadLaneKnown);
    CHECK(!carrier.typedReadSuccessKnown800179B4);
    CHECK(!carrier.payloadBytesKnown8007ADE8);
    CHECK(carrier.feedback.word8007ABE4Known);
    CHECK(carrier.feedback.word8007ABE4 == 0);
    CHECK(carrier.hal.produced);
    CHECK(!carrier.incomplete);

    ClearCase17CardReadTypedCarrier800179B4();
    CHECK(!GetCase17CardReadTypedCarrier800179B4(&carrier));
}

void TestState16TypedCarrierRejectsHostFactsOnlyFeedback() {
    ClearState16CardReadTypedCarrier800179B4();
    State16CardReadTypedCarrier800179B4 carrier{};
    CHECK(!GetState16CardReadTypedCarrier800179B4(&carrier));

    const ReadBlock block = MakeReadBlock(0xB1u);
    CardReadFeedbackProducerInput800179B4 input{};
    input.requestKnown = true;
    input.request = MakeState16LoadPayloadReadRequest800179B4(16);
    input.hostFactsKnown = true;
    input.hostFacts.factsKnown = true;
    input.hostFacts.triggerChainKnown = true;
    input.hostFacts.arg2Known = true;
    input.hostFacts.arg2 = 16;
    input.hostFacts.word8007ABE4Known = true;
    input.hostFacts.word8007ABE4 = 1;
    input.hostFacts.rowNameBuffer8007CBE8Known = true;
    input.hostFacts.readBufferKnown = true;
    input.hostFacts.readBuffer = kCardReadBlockBufferAddr800179B4;
    input.hostFacts.readLengthKnown = true;
    input.hostFacts.readLength = kCardReadBlockBytes800179B4;
    input.hostFacts.payloadPointerKnown = true;
    input.hostFacts.payloadPointer = kCardReadPayloadAddr8007ADE8;
    input.hostFacts.payloadPassedTo800164F8 = true;
    input.hostFacts.attempts[0].rowEnabledKnown = true;
    input.hostFacts.attempts[0].rowEnabled = true;
    input.hostFacts.attempts[0].rowNameKnown = true;
    SetRowName(input.hostFacts.attempts[0].rowName, "BISLPS-00000HOST");
    input.hostFacts.attempts[0].rowNameBuffer8007CBE8Known = true;
    input.hostFacts.attempts[0].cardSelectorKnown = true;
    input.hostFacts.attempts[0].readBufferBytesKnown = true;
    input.hostFacts.attempts[0].readBufferBytes = block.data();
    input.hostFacts.attempts[0].readBufferByteCount = block.size();

    CardReadFeedbackProducerResult800179B4 producer{};
    BuildCardReadFeedbackFromHostFacts800179B4(input, &producer);
    CHECK(producer.produced);
    CHECK(!producer.incomplete);
    CHECK(producer.hostFactsUsed);
    CHECK(!producer.feedback.attempts[0].successAuthorityKnown800179B4);
    CHECK(!producer.feedback.attempts[0].success800179B4);

    CHECK(!PublishState16CardReadTypedCarrier800179B4ForBlock(
        producer.feedback,
        4));
    CHECK(GetState16CardReadTypedCarrier800179B4(&carrier));
    CHECK(carrier.known);
    CHECK(carrier.source == CardReadTypedCarrierSource800179B4::Unknown);
    CHECK(!carrier.producerWired800173A8_80016EB8_800179B4);
    CHECK(!carrier.state16LoadPayloadLaneKnown);
    CHECK(carrier.selectedBlockKnown);
    CHECK(carrier.selectedBlockIndex == 4);
    CHECK(!carrier.typedReadSuccessKnown800179B4);
    CHECK(!carrier.payloadBytesKnown8007ADE8);
    CHECK(carrier.incomplete);
    CHECK(carrier.hal.produced);
    CHECK(carrier.hal.incomplete);
    CHECK(!carrier.hal.anyPayloadBytesAvailable);

    ClearState16CardReadTypedCarrier800179B4();
    CHECK(!GetState16CardReadTypedCarrier800179B4(&carrier));
}

void TestState16TypedCarrierRequiresSelectedSuccessfulBlockReadyBits() {
    ClearState16CardReadTypedCarrier800179B4();
    State16CardReadTypedCarrier800179B4 carrier{};

    const ReadBlock failBlock = MakeReadBlock(0xD5u);
    CardReadFeedback800179B4 failFeedback{};
    failFeedback.feedbackKnown = true;
    failFeedback.word8007ABE4Known = true;
    failFeedback.word8007ABE4 = 1;
    MarkUnusedRowsSkipped(&failFeedback);
    FillTypedReadAttempt(&failFeedback.attempts[0], failBlock, false);

    CHECK(!PublishState16CardReadTypedCarrier800179B4ForBlock(
        failFeedback,
        4));
    CHECK(GetState16CardReadTypedCarrier800179B4(&carrier));
    CHECK(carrier.known);
    CHECK(carrier.source == CardReadTypedCarrierSource800179B4::Unknown);
    CHECK(!carrier.producerWired800173A8_80016EB8_800179B4);
    CHECK(!carrier.state16LoadPayloadLaneKnown);
    CHECK(carrier.selectedBlockKnown);
    CHECK(carrier.selectedBlockIndex == 4);
    CHECK(!carrier.typedReadSuccessKnown800179B4);
    CHECK(!carrier.payloadBytesKnown8007ADE8);
    CHECK(!carrier.incomplete);
    CHECK(carrier.hal.produced);
    CHECK(!carrier.hal.incomplete);
    CHECK(!carrier.hal.anyPayloadBytesAvailable);

    const ReadBlock successBlock = MakeReadBlock(0xE5u);
    CardReadFeedback800179B4 successFeedback{};
    successFeedback.feedbackKnown = true;
    successFeedback.word8007ABE4Known = true;
    successFeedback.word8007ABE4 = 1;
    MarkUnusedRowsSkipped(&successFeedback);
    successFeedback.attempts[0].rowEnabledKnown = true;
    successFeedback.attempts[0].rowEnabled = false;
    FillTypedReadAttempt(&successFeedback.attempts[4], successBlock, true);

    CHECK(!PublishState16CardReadTypedCarrier800179B4(successFeedback));
    CHECK(GetState16CardReadTypedCarrier800179B4(&carrier));
    CHECK(carrier.known);
    CHECK(carrier.source == CardReadTypedCarrierSource800179B4::Unknown);
    CHECK(!carrier.producerWired800173A8_80016EB8_800179B4);
    CHECK(!carrier.state16LoadPayloadLaneKnown);
    CHECK(!carrier.selectedBlockKnown);
    CHECK(carrier.selectedBlockIndex == -1);
    CHECK(!carrier.typedReadSuccessKnown800179B4);
    CHECK(!carrier.payloadBytesKnown8007ADE8);
    CHECK(!carrier.incomplete);

    CardReadFeedback800179B4 wrongRowFeedback{};
    wrongRowFeedback.feedbackKnown = true;
    wrongRowFeedback.word8007ABE4Known = true;
    wrongRowFeedback.word8007ABE4 = 1;
    MarkUnusedRowsSkipped(&wrongRowFeedback);
    FillTypedReadAttempt(&wrongRowFeedback.attempts[0], successBlock, true);

    CHECK(!PublishState16CardReadTypedCarrier800179B4ForBlock(
        wrongRowFeedback,
        4));
    CHECK(GetState16CardReadTypedCarrier800179B4(&carrier));
    CHECK(carrier.known);
    CHECK(!carrier.producerWired800173A8_80016EB8_800179B4);
    CHECK(!carrier.state16LoadPayloadLaneKnown);
    CHECK(carrier.selectedBlockKnown);
    CHECK(carrier.selectedBlockIndex == 4);
    CHECK(!carrier.typedReadSuccessKnown800179B4);
    CHECK(!carrier.payloadBytesKnown8007ADE8);
    CHECK(carrier.hal.anyPayloadBytesAvailable);

    ClearState16CardReadTypedCarrier800179B4();
    CHECK(!GetState16CardReadTypedCarrier800179B4(&carrier));
}

void TestState16TypedCarrierAcceptsExplicitTypedReadSuccess() {
    ClearState16CardReadTypedCarrier800179B4();
    State16CardReadTypedCarrier800179B4 carrier{};

    const ReadBlock successBlock = MakeReadBlock(0xC1u);
    CardReadFeedback800179B4 successFeedback{};
    successFeedback.feedbackKnown = true;
    successFeedback.word8007ABE4Known = true;
    successFeedback.word8007ABE4 = 1;
    MarkUnusedRowsSkipped(&successFeedback);
    successFeedback.attempts[0].rowEnabledKnown = true;
    successFeedback.attempts[0].rowEnabled = false;
    FillTypedReadAttempt(&successFeedback.attempts[4], successBlock, true);

    CHECK(!PublishState16CardReadTypedCarrier800179B4ForBlock(
        successFeedback,
        4));
    CHECK(GetState16CardReadTypedCarrier800179B4(&carrier));
    CHECK(carrier.known);
    CHECK(carrier.source == CardReadTypedCarrierSource800179B4::Unknown);
    CHECK(!carrier.producerWired800173A8_80016EB8_800179B4);
    CHECK(!carrier.state16LoadPayloadLaneKnown);
    CHECK(carrier.selectedBlockKnown);
    CHECK(carrier.selectedBlockIndex == 4);
    CHECK(carrier.typedReadSuccessKnown800179B4);
    CHECK(carrier.payloadBytesKnown8007ADE8);
    CHECK(!carrier.incomplete);
    CHECK(carrier.hal.produced);
    CHECK(carrier.hal.anyPayloadBytesAvailable);

    CHECK(!PublishDebugState16CardReadTypedCarrier800179B4ForBlock(
        successFeedback,
        4));
    CHECK(GetState16CardReadTypedCarrier800179B4(&carrier));
    CHECK(carrier.known);
    CHECK(carrier.source ==
          CardReadTypedCarrierSource800179B4::DebugSyntheticFixture);
    CHECK(!carrier.producerWired800173A8_80016EB8_800179B4);
    CHECK(carrier.state16LoadPayloadLaneKnown);
    CHECK(carrier.selectedBlockKnown);
    CHECK(carrier.selectedBlockIndex == 4);
    CHECK(carrier.typedReadSuccessKnown800179B4);
    CHECK(carrier.payloadBytesKnown8007ADE8);
    CHECK(!carrier.incomplete);
    CHECK(carrier.hal.produced);
    CHECK(carrier.hal.anyPayloadBytesAvailable);

    CardReadFeedbackRequest800179B4 mismatchedRequest =
        MakeState16LoadPayloadReadRequest800179B4(16);
    mismatchedRequest.payloadAddress = kCardReadPayloadAddr8007ADE8 + 0x10u;
    CHECK(!PublishRuntimeState16CardReadTypedCarrier800179B4ForBlock(
        mismatchedRequest,
        successFeedback,
        4));
    CHECK(!GetState16CardReadTypedCarrier800179B4(&carrier));

    const CardReadFeedbackRequest800179B4 runtimeRequest =
        MakeState16LoadPayloadReadRequest800179B4(16);
    CHECK(!PublishRuntimeState16CardReadTypedCarrier800179B4ForBlock(
        runtimeRequest,
        successFeedback,
        5));
    CHECK(!GetState16CardReadTypedCarrier800179B4(&carrier));

    CHECK(PublishRuntimeState16CardReadTypedCarrier800179B4ForBlock(
        runtimeRequest,
        successFeedback,
        4));
    CHECK(GetState16CardReadTypedCarrier800179B4(&carrier));
    CHECK(carrier.known);
    CHECK(carrier.source ==
          CardReadTypedCarrierSource800179B4::RuntimeLowerCardProducer);
    CHECK(carrier.producerWired800173A8_80016EB8_800179B4);
    CHECK(carrier.state16LoadPayloadLaneKnown);
    CHECK(carrier.selectedBlockKnown);
    CHECK(carrier.selectedBlockIndex == 4);
    CHECK(carrier.typedReadSuccessKnown800179B4);
    CHECK(carrier.payloadBytesKnown8007ADE8);
    CHECK(!carrier.incomplete);
    CHECK(carrier.hal.produced);
    CHECK(carrier.hal.anyPayloadBytesAvailable);
    CHECK(carrier.hal.attempts[4].readSucceeded);
    CHECK(carrier.hal.attempts[4].payloadBytesAvailable);
    CHECK(carrier.hal.attempts[4].blockBytes ==
          carrier.blockStorage[4].data());
    CHECK(carrier.feedback.attempts[4].blockBytes ==
          carrier.blockStorage[4].data());
    CHECK(carrier.blockStorage[4][0] == successBlock[0]);

    ClearState16CardReadTypedCarrier800179B4();
    CHECK(!GetState16CardReadTypedCarrier800179B4(&carrier));

    CHECK(!PublishState16CardReadTypedCarrier800179B4(successFeedback));
    CHECK(GetState16CardReadTypedCarrier800179B4(&carrier));
    CHECK(!carrier.producerWired800173A8_80016EB8_800179B4);
    CHECK(!carrier.state16LoadPayloadLaneKnown);
    CHECK(!carrier.selectedBlockKnown);
    CHECK(carrier.selectedBlockIndex == -1);
    CHECK(!carrier.typedReadSuccessKnown800179B4);
    CHECK(!carrier.payloadBytesKnown8007ADE8);

    ClearState16CardReadTypedCarrier800179B4();
    CHECK(!GetState16CardReadTypedCarrier800179B4(&carrier));
}

void TestState16RuntimeTypedFactsBridgeRequiresImportableFacts() {
    ClearState16CardReadTypedCarrier800179B4();
    State16CardReadTypedCarrier800179B4 carrier{};
    const ReadBlock block = MakeReadBlock(0xA7u);

    State16CardReadRuntimeTypedFacts800179B4 facts =
        MakeRuntimeTypedFacts(block, 4);
    CHECK(!IsImportableState16RuntimeTypedFacts800179B4(facts, 5));
    CHECK(!PublishRuntimeState16CardReadTypedCarrier800179B4FromTypedFacts(
        facts,
        5));
    CHECK(!GetState16CardReadTypedCarrier800179B4(&carrier));

    facts = MakeRuntimeTypedFacts(block, 4);
    facts.fullPayloadByteCount = block.size() - 1u;
    CHECK(!IsImportableState16RuntimeTypedFacts800179B4(facts, 4));
    CHECK(!PublishRuntimeState16CardReadTypedCarrier800179B4FromTypedFacts(
        facts,
        4));
    CHECK(!GetState16CardReadTypedCarrier800179B4(&carrier));

    facts = MakeRuntimeTypedFacts(block, 4);
    facts.pollResult80016EB8 = 2;
    facts.psxReturn800179B4 = -1;
    CHECK(!IsImportableState16RuntimeTypedFacts800179B4(facts, 4));
    CHECK(!PublishRuntimeState16CardReadTypedCarrier800179B4FromTypedFacts(
        facts,
        4));
    CHECK(!GetState16CardReadTypedCarrier800179B4(&carrier));

    facts = MakeRuntimeTypedFacts(block, 4);
    facts.pollEventHandlesKnown80016EB8 = false;
    CHECK(!IsImportableState16RuntimeTypedFacts800179B4(facts, 4));
    CHECK(!PublishRuntimeState16CardReadTypedCarrier800179B4FromTypedFacts(
        facts,
        4));
    CHECK(!GetState16CardReadTypedCarrier800179B4(&carrier));

    facts = MakeRuntimeTypedFacts(block, 4);
    facts.payloadArgument = kCardReadPayloadAddr8007ADE8 + 4u;
    CHECK(!IsImportableState16RuntimeTypedFacts800179B4(facts, 4));
    CHECK(!PublishRuntimeState16CardReadTypedCarrier800179B4FromTypedFacts(
        facts,
        4));
    CHECK(!GetState16CardReadTypedCarrier800179B4(&carrier));

    facts = MakeRuntimeTypedFacts(block, 4);
    CHECK(IsImportableState16RuntimeTypedFacts800179B4(facts, 4));
    CHECK(PublishRuntimeState16CardReadTypedCarrier800179B4FromTypedFacts(
        facts,
        4));
    CHECK(GetState16CardReadTypedCarrier800179B4(&carrier));
    CHECK(carrier.known);
    CHECK(carrier.source ==
          CardReadTypedCarrierSource800179B4::RuntimeLowerCardProducer);
    CHECK(carrier.producerWired800173A8_80016EB8_800179B4);
    CHECK(carrier.state16LoadPayloadLaneKnown);
    CHECK(carrier.selectedBlockKnown);
    CHECK(carrier.selectedBlockIndex == 4);
    CHECK(carrier.typedReadSuccessKnown800179B4);
    CHECK(carrier.payloadBytesKnown8007ADE8);
    CHECK(!carrier.incomplete);
    CHECK(carrier.hal.produced);
    CHECK(carrier.hal.anyPayloadBytesAvailable);
    CHECK(carrier.hal.attempts[4].readSucceeded);
    CHECK(carrier.hal.attempts[4].eventResult80016EB8 == 1);
    CHECK(carrier.hal.attempts[4].payloadBytesAvailable);
    CHECK(carrier.blockStorage[4][0] == block[0]);

    ClearState16CardReadTypedCarrier800179B4();
    CHECK(!GetState16CardReadTypedCarrier800179B4(&carrier));
}

void TestState16RuntimeReadFailureClearsTypedCarrier() {
    ClearState16CardReadTypedCarrier800179B4();
    State16CardReadTypedCarrier800179B4 carrier{};
    const ReadBlock block = MakeReadBlock(0xB7u);
    const CardReadFeedbackRequest800179B4 runtimeRequest =
        MakeState16LoadPayloadReadRequest800179B4(16);

    State16CardReadRuntimeTypedFacts800179B4 successFacts =
        MakeRuntimeTypedFacts(block, 4);
    CHECK(PublishRuntimeState16CardReadTypedCarrier800179B4FromTypedFacts(
        successFacts,
        4));
    CHECK(GetState16CardReadTypedCarrier800179B4(&carrier));
    CHECK(carrier.producerWired800173A8_80016EB8_800179B4);
    CHECK(carrier.payloadBytesKnown8007ADE8);

    State16CardReadRuntimeTypedFacts800179B4 failedPollFacts =
        successFacts;
    failedPollFacts.pollResult80016EB8 = 2;
    failedPollFacts.psxReturn800179B4 = -1;
    CHECK(!PublishRuntimeState16CardReadTypedCarrier800179B4FromTypedFacts(
        failedPollFacts,
        4));
    CHECK(!GetState16CardReadTypedCarrier800179B4(&carrier));

    CHECK(PublishRuntimeState16CardReadTypedCarrier800179B4FromTypedFacts(
        successFacts,
        4));
    CHECK(!PublishRuntimeState16CardReadTypedCarrier800179B4FromTypedFacts(
        successFacts,
        5));
    CHECK(!GetState16CardReadTypedCarrier800179B4(&carrier));

    CardReadFeedback800179B4 successFeedback{};
    successFeedback.feedbackKnown = true;
    successFeedback.word8007ABE4Known = true;
    successFeedback.word8007ABE4 = 1;
    MarkUnusedRowsSkipped(&successFeedback);
    successFeedback.attempts[0].rowEnabledKnown = true;
    successFeedback.attempts[0].rowEnabled = false;
    FillTypedReadAttempt(&successFeedback.attempts[4], block, true);

    CHECK(PublishRuntimeState16CardReadTypedCarrier800179B4ForBlock(
        runtimeRequest,
        successFeedback,
        4));
    CHECK(GetState16CardReadTypedCarrier800179B4(&carrier));
    CHECK(carrier.producerWired800173A8_80016EB8_800179B4);

    CardReadFeedbackRequest800179B4 wrongRequest = runtimeRequest;
    wrongRequest.payloadAddress = kCardReadPayloadAddr8007ADE8 + 0x10u;
    CHECK(!PublishRuntimeState16CardReadTypedCarrier800179B4ForBlock(
        wrongRequest,
        successFeedback,
        4));
    CHECK(!GetState16CardReadTypedCarrier800179B4(&carrier));

    ClearState16CardReadTypedCarrier800179B4();
    CHECK(!GetState16CardReadTypedCarrier800179B4(&carrier));
}

void TestState16DirectCardImagePersistenceSinkPublishesRuntimeCarrier() {
    ClearState16CardReadTypedCarrier800179B4();
    State16CardReadTypedCarrier800179B4 carrier{};
    const ReadBlock block = MakeReadBlock(0x5Au);
    FillDirectCardImage(g_directCardImage, 4, "BASCUS-94183Z", block);
    const PrStage1SaveUiCardImagePersistenceView8007A318 view =
        MakeDirectCardImageView(g_directCardImage, 4);

    CHECK(PublishRuntimeState16CardReadTypedCarrier800179B4FromDirectCardImagePersistenceSink(
        view,
        4));
    CHECK(GetState16CardReadTypedCarrier800179B4(&carrier));
    CHECK(carrier.known);
    CHECK(carrier.source ==
          CardReadTypedCarrierSource800179B4::RuntimeLowerCardProducer);
    CHECK(carrier.producerWired800173A8_80016EB8_800179B4);
    CHECK(carrier.state16LoadPayloadLaneKnown);
    CHECK(carrier.selectedBlockKnown);
    CHECK(carrier.selectedBlockIndex == 4);
    CHECK(carrier.typedReadSuccessKnown800179B4);
    CHECK(carrier.payloadBytesKnown8007ADE8);
    CHECK(!carrier.incomplete);
    CHECK(carrier.hal.produced);
    CHECK(carrier.hal.anyPayloadBytesAvailable);
    CHECK(carrier.hal.attempts[4].readSucceeded);
    CHECK(carrier.hal.attempts[4].payloadBytesAvailable);
    CHECK(carrier.blockStorage[4][0] == block[0]);
    CHECK(carrier.blockStorage[4][0x1FFu] == block[0x1FFu]);
    CHECK(!view.durablePolicyKnown);
    CHECK(!view.durableCommitted);

    ClearState16CardReadTypedCarrier800179B4();
    CHECK(!GetState16CardReadTypedCarrier800179B4(&carrier));
}

void TestState16DirectCardImagePersistenceSinkFailsClosed() {
    ClearState16CardReadTypedCarrier800179B4();
    State16CardReadTypedCarrier800179B4 carrier{};
    const ReadBlock block = MakeReadBlock(0x6Au);
    FillDirectCardImage(g_directCardImage, 4, "BASCUS-94183Z", block);
    PrStage1SaveUiCardImagePersistenceView8007A318 view =
        MakeDirectCardImageView(g_directCardImage, 4);

    CHECK(!PublishRuntimeState16CardReadTypedCarrier800179B4FromDirectCardImagePersistenceSink(
        view,
        5));
    CHECK(!GetState16CardReadTypedCarrier800179B4(&carrier));

    view = MakeDirectCardImageView(g_directCardImage, 4);
    view.slotPolicyKnown = false;
    CHECK(!PublishRuntimeState16CardReadTypedCarrier800179B4FromDirectCardImagePersistenceSink(
        view,
        4));
    CHECK(!GetState16CardReadTypedCarrier800179B4(&carrier));

    view = MakeDirectCardImageView(g_directCardImage, 4);
    view.bytes = nullptr;
    CHECK(!PublishRuntimeState16CardReadTypedCarrier800179B4FromDirectCardImagePersistenceSink(
        view,
        4));
    CHECK(!GetState16CardReadTypedCarrier800179B4(&carrier));

    g_badDirectCardImage = g_directCardImage;
    const std::size_t dirOffset = 5u * 128u;
    g_badDirectCardImage[dirOffset + 0x00u] = 0x00u;
    view = MakeDirectCardImageView(g_badDirectCardImage, 4);
    CHECK(!PublishRuntimeState16CardReadTypedCarrier800179B4FromDirectCardImagePersistenceSink(
        view,
        4));
    CHECK(!GetState16CardReadTypedCarrier800179B4(&carrier));
}

void TestCase17DirectCardImagePersistenceSinkPublishesRuntimeCarrier() {
    ClearCase17CardReadTypedCarrier800179B4();
    Case17CardReadTypedCarrier800179B4 carrier{};
    const ReadBlock block = MakeReadBlock(0x7Au);
    FillDirectCardImage(g_directCardImage, 2, "BASCUS-94183H", block);
    PrStage1SaveUiCardImagePersistenceView8007A318 view =
        MakeDirectCardImageView(g_directCardImage, 2);

    CHECK(PublishRuntimeCase17CardReadTypedCarrier800179B4FromDirectCardImagePersistenceSink(
        view,
        2));
    CHECK(GetCase17CardReadTypedCarrier800179B4(&carrier));
    CHECK(carrier.known);
    CHECK(carrier.source ==
          CardReadTypedCarrierSource800179B4::RuntimeLowerCardProducer);
    CHECK(carrier.producerWired800173A8_80016EB8_800179B4);
    CHECK(carrier.case17LoopCompletionKnown80019D7C);
    CHECK(carrier.case17HiScorePayloadLaneKnown);
    CHECK(carrier.typedReadSuccessKnown800179B4);
    CHECK(carrier.payloadBytesKnown8007ADE8);
    CHECK(!carrier.incomplete);
    CHECK(carrier.hal.produced);
    CHECK(carrier.hal.anyPayloadBytesAvailable);
    CHECK(carrier.feedback.word8007ABE4Known);
    CHECK(carrier.feedback.word8007ABE4 == 1);
    CHECK(carrier.hal.attempts[0].readSucceeded);
    CHECK(carrier.hal.attempts[0].payloadBytesAvailable);
    CHECK(carrier.hal.attempts[0].blockBytes ==
          carrier.blockStorage[0].data());
    CHECK(carrier.feedback.attempts[0].blockBytes ==
          carrier.blockStorage[0].data());
    CHECK(carrier.blockStorage[0][0] == block[0]);
    CHECK(carrier.blockStorage[0][0x1FFu] == block[0x1FFu]);
    CHECK(carrier.feedback.attempts[0].rowNameKnown);
    CHECK(std::strcmp(carrier.feedback.attempts[0].rowName,
                      "BASCUS-94183H") == 0);
    CHECK(carrier.feedback.attempts[1].rowEnabledKnown);
    CHECK(!carrier.feedback.attempts[1].rowEnabled);
    CHECK(!view.durablePolicyKnown);
    CHECK(!view.durableCommitted);

    ClearCase17CardReadTypedCarrier800179B4();
    CHECK(!GetCase17CardReadTypedCarrier800179B4(&carrier));
}

void TestCase17DirectCardImagePersistenceSinkCompactsAllMatchingRows() {
    ClearCase17CardReadTypedCarrier800179B4();
    Case17CardReadTypedCarrier800179B4 carrier{};
    const ReadBlock lowBlock = MakeReadBlock(0x21u);
    const ReadBlock highBlock = MakeReadBlock(0x61u);
    const ReadBlock unrelatedBlock = MakeReadBlock(0xA1u);
    g_directCardImage.fill(0);
    PutDirectCardImageEntry(
        g_directCardImage, 7, "BASCUS-94183Z", highBlock);
    PutDirectCardImageEntry(
        g_directCardImage, 2, "BASCUS-94183A", lowBlock);
    PutDirectCardImageEntry(
        g_directCardImage, 4, "OTHER-GAME-SAVE", unrelatedBlock);
    const PrStage1SaveUiCardImagePersistenceView8007A318 view =
        MakeDirectCardImageView(g_directCardImage, 7);

    CHECK(PublishRuntimeCase17CardReadTypedCarrier800179B4FromDirectCardImagePersistenceSink(
        view,
        7));
    CHECK(GetCase17CardReadTypedCarrier800179B4(&carrier));
    CHECK(carrier.case17LoopCompletionKnown80019D7C);
    CHECK(carrier.feedback.word8007ABE4Known);
    CHECK(carrier.feedback.word8007ABE4 == 2);
    CHECK(std::strcmp(carrier.feedback.attempts[0].rowName,
                      "BASCUS-94183A") == 0);
    CHECK(std::strcmp(carrier.feedback.attempts[1].rowName,
                      "BASCUS-94183Z") == 0);
    CHECK(carrier.blockStorage[0][0] == lowBlock[0]);
    CHECK(carrier.blockStorage[1][0] == highBlock[0]);
    CHECK(carrier.feedback.attempts[2].rowEnabledKnown);
    CHECK(!carrier.feedback.attempts[2].rowEnabled);

    ClearCase17CardReadTypedCarrier800179B4();
}

void TestCase17DirectCardImagePersistenceSinkKnownEmptyCompletes() {
    ClearCase17CardReadTypedCarrier800179B4();
    Case17CardReadTypedCarrier800179B4 carrier{};
    g_directCardImage.fill(0);
    const PrStage1SaveUiCardImagePersistenceView8007A318 view =
        MakeDirectCardImageView(g_directCardImage, 2);

    CHECK(PublishRuntimeCase17CardReadTypedCarrier800179B4FromDirectCardImagePersistenceSink(
        view,
        2));
    CHECK(GetCase17CardReadTypedCarrier800179B4(&carrier));
    CHECK(carrier.producerWired800173A8_80016EB8_800179B4);
    CHECK(carrier.case17LoopCompletionKnown80019D7C);
    CHECK(!carrier.case17HiScorePayloadLaneKnown);
    CHECK(!carrier.typedReadSuccessKnown800179B4);
    CHECK(!carrier.payloadBytesKnown8007ADE8);
    CHECK(carrier.feedback.word8007ABE4Known);
    CHECK(carrier.feedback.word8007ABE4 == 0);
    CHECK(carrier.hal.produced);
    CHECK(!carrier.hal.anyPayloadBytesAvailable);
    CHECK(!carrier.incomplete);

    ClearCase17CardReadTypedCarrier800179B4();
}

void TestCase17DirectCardImagePersistenceSinkFailsClosed() {
    ClearCase17CardReadTypedCarrier800179B4();
    Case17CardReadTypedCarrier800179B4 carrier{};
    const ReadBlock block = MakeReadBlock(0x8Au);
    FillDirectCardImage(g_directCardImage, 2, "BASCUS-94183I", block);
    PrStage1SaveUiCardImagePersistenceView8007A318 view =
        MakeDirectCardImageView(g_directCardImage, 2);

    CHECK(!PublishRuntimeCase17CardReadTypedCarrier800179B4FromDirectCardImagePersistenceSink(
        view,
        3));
    CHECK(!GetCase17CardReadTypedCarrier800179B4(&carrier));

    view = MakeDirectCardImageView(g_directCardImage, 2);
    view.slotPolicyKnown = false;
    CHECK(!PublishRuntimeCase17CardReadTypedCarrier800179B4FromDirectCardImagePersistenceSink(
        view,
        2));
    CHECK(!GetCase17CardReadTypedCarrier800179B4(&carrier));

    view = MakeDirectCardImageView(g_directCardImage, 2);
    view.bytes = nullptr;
    CHECK(!PublishRuntimeCase17CardReadTypedCarrier800179B4FromDirectCardImagePersistenceSink(
        view,
        2));
    CHECK(!GetCase17CardReadTypedCarrier800179B4(&carrier));

}

} // namespace

int main() {
    TestState16ReadRequestShapeDoesNotProduceSuccess();
    TestCase17ReadRequestShapeDoesNotEarlyReturn();
    TestHostFactsCannotSynthesizeTypedReadSuccess();
    TestExplicitTypedReadSuccessProducesPayloadOnlyAfterPollSuccess();
    TestExplicitTypedReadFailureDoesNotExposePayloadBytes();
    TestLiveCase17AuthorityRequiresPayloadBytes();
    TestRowCoverageAloneCannotProduceReadAuthority();
    TestRequestRejectsEarlyReturnArg2();
    TestState16ProducerRejectsMismatchedRequestWithExplicitFeedback();
    TestState16ProducerRejectsCase17ShapedArg2Request();
    TestCase17TypedCarrierRequiresExplicitTypedRead();
    TestState16TypedCarrierRejectsHostFactsOnlyFeedback();
    TestState16TypedCarrierRequiresSelectedSuccessfulBlockReadyBits();
    TestState16TypedCarrierAcceptsExplicitTypedReadSuccess();
    TestState16RuntimeTypedFactsBridgeRequiresImportableFacts();
    TestState16RuntimeReadFailureClearsTypedCarrier();
    TestState16DirectCardImagePersistenceSinkPublishesRuntimeCarrier();
    TestState16DirectCardImagePersistenceSinkFailsClosed();
    TestCase17DirectCardImagePersistenceSinkPublishesRuntimeCarrier();
    TestCase17DirectCardImagePersistenceSinkCompactsAllMatchingRows();
    TestCase17DirectCardImagePersistenceSinkKnownEmptyCompletes();
    TestCase17DirectCardImagePersistenceSinkFailsClosed();

    if (g_failed != 0) {
        std::printf("test_ss0_card_read_authority: failed checks=%d\n",
                    g_failed);
        return 1;
    }

    std::printf("test_ss0_card_read_authority: ok\n");
    return 0;
}
