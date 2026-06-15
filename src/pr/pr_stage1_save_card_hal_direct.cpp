#include "pr_stage1_save_card_hal_direct.h"

#include <cstring>

namespace PrStage1SaveCardHalDirect {
namespace {

State16CardReadTypedCarrier800179B4 s_state16CardReadTypedCarrier800179B4{};
Case17CardReadTypedCarrier800179B4 s_case17CardReadTypedCarrier800179B4{};

bool IsCompleteClearEvents80016FC0(
    const CardClearEventsFeedback80016FC0& input) {
    if (!input.called || !input.eventHandlesKnown) {
        return false;
    }
    for (bool known : input.testEventResultsKnown) {
        if (!known) {
            return false;
        }
    }
    return true;
}

bool IsCompletePoll80016EB8(const CardPollFeedback80016EB8& input) {
    if (!input.called ||
        !input.eventHandlesKnown ||
        !input.resultKnown ||
        !input.timedOutKnown ||
        !input.pollIterationCountKnown ||
        !input.waitCallCountKnown80035560 ||
        input.pollIterationCount < 1 ||
        input.pollIterationCount > kCardPollLimit80016EB8 ||
        input.waitCallCount80035560 < 0 ||
        input.waitCallCount80035560 > kCardPollLimit80016EB8) {
        return false;
    }

    if (input.timedOut) {
        return input.psxReturn == 2 &&
               input.pollIterationCount == kCardPollLimit80016EB8 &&
               input.waitCallCount80035560 == kCardPollLimit80016EB8;
    }

    if (!input.hitEventIndexKnown ||
        input.hitEventIndex < 0 ||
        input.hitEventIndex >= 4) {
        return false;
    }
    return input.psxReturn == input.hitEventIndex + 1;
}

bool IsExpectedWriteRequest80017A10(
    const PrStage1SaveUi19148LowerFeedbackRequest& request) {
    return request.kind ==
               PrStage1SaveUi19148LowerFeedbackRequestKind::Write80017A10 &&
           request.psxFunction == kFn80017A10 &&
           request.retryCount == kWriteAttemptCount80017A10 &&
           request.writeCloseGp696FactRequired80017A10 &&
           request.writeCloseGp696Address80017A10 ==
               kWriteCloseGp696Address80017A10 &&
           request.writeFdMustMatchCloseGp69680017A10;
}

bool IsExpectedReadRequest800179B4(
    const CardReadFeedbackRequest800179B4& request) {
    return request.callFunction80019414 == kFn80019414 &&
           request.caseFunction80019D7C == kFn80019D7C &&
           (request.state16LoadPayloadRequest80019D7C ||
            request.case17HiScoreRequest80019D7C) &&
           request.arg2Known &&
           request.arg2 != kCase17Arg2EarlyReturn80019D7C &&
           request.psxFunction == kFn800179B4 &&
           request.retryCount == kReadAttemptCount800179B4 &&
           request.pathFunction800173A8 == kFn800173A8 &&
           request.pathOpenFlags800173A8 == kCardPathOpenFlags800173A8 &&
           request.clearEventsFunction80016FC0 == kFn80016FC0 &&
           request.pollFunction80016EB8 == kFn80016EB8 &&
           request.nameAddress == kCardReadNameBufferAddr8007CBE8 &&
           request.targetBufferAddress ==
               kCardReadBlockBufferAddr800179B4 &&
           request.payloadAddress == kCardReadPayloadAddr8007ADE8 &&
           request.blockCount == kCardReadBlockCount800179B4 &&
           request.blockBytes == kCardReadBlockBytes800179B4 &&
           request.closeGp696FactRequired &&
           request.closeGp696Address == kReadCloseGp696Address800179B4 &&
           request.closeFdMustMatchGp696;
}

bool IsExpectedState16LoadPayloadReadRequest800179B4(
    const CardReadFeedbackRequest800179B4& request) {
    return IsExpectedReadRequest800179B4(request) &&
           request.state16LoadPayloadRequest80019D7C &&
           !request.case17HiScoreRequest80019D7C &&
           request.arg2 == 16;
}

bool IsExpectedReadSubmission800173A8(
    const CardReadSubmissionFeedback800173A8& input,
    int32_t expectedFd) {
    return input.called &&
           input.fdKnown &&
           input.fd == expectedFd &&
           input.bufferAddressKnown &&
           input.bufferAddress == kCardReadBlockBufferAddr800179B4 &&
           input.byteCountKnown &&
           input.byteCount == kCardReadBlockBytes800179B4;
}

bool IsCommandSafeState16Title800179B4(const char (&title)[32]) {
    if (title[0] == '\0') {
        return false;
    }
    for (std::size_t i = 0; i < sizeof(title); ++i) {
        const unsigned char ch = static_cast<unsigned char>(title[i]);
        if (ch == '\0') {
            return true;
        }
        if (ch < 0x21u || ch > 0x7Eu) {
            return false;
        }
    }
    return false;
}

bool IsImportableState16RuntimeTypedFacts800179B4Internal(
    const State16CardReadRuntimeTypedFacts800179B4& facts,
    int32_t selectedBlockIndex) {
    if (!facts.factsKnown ||
        !facts.state16CallKnown ||
        !facts.selectedBlockKnown ||
        facts.selectedBlockIndex != selectedBlockIndex ||
        selectedBlockIndex < 0 ||
        selectedBlockIndex >= kReadAttemptCount800179B4 ||
        !facts.selectedTitleKnown ||
        !IsCommandSafeState16Title800179B4(facts.selectedTitle) ||
        !facts.rowCountKnown ||
        facts.rowCount <= selectedBlockIndex ||
        facts.rowCount > kReadAttemptCount800179B4 ||
        !facts.arg2Known ||
        facts.arg2 != 16 ||
        !facts.nameAddressKnown ||
        facts.nameAddress != kCardReadNameBufferAddr8007CBE8 ||
        !facts.targetBufferAddressKnown ||
        facts.targetBufferAddress != kCardReadBlockBufferAddr800179B4 ||
        !facts.payloadAddressKnown ||
        facts.payloadAddress != kCardReadPayloadAddr8007ADE8 ||
        !facts.blockCountKnown ||
        facts.blockCount != kCardReadBlockCount800179B4 ||
        !facts.pathCallKnown ||
        !facts.cardSelectorKnown ||
        !facts.gp696FdWriteKnown ||
        facts.gp696Fd < 0 ||
        !facts.clearEventsCallKnown ||
        !facts.readSubmissionKnown ||
        !facts.readFdKnown ||
        facts.readFd != facts.gp696Fd ||
        !facts.readBufferAddressKnown ||
        facts.readBufferAddress != kCardReadBlockBufferAddr800179B4 ||
        !facts.readByteCountKnown ||
        facts.readByteCount != kCardReadBlockBytes800179B4 ||
        !facts.pollCallKnown ||
        !facts.pollEventHandlesKnown80016EB8 ||
        facts.pollEventHandle0_80016EB8 == 0 ||
        facts.pollEventHandle1_80016EB8 == 0 ||
        facts.pollEventHandle2_80016EB8 == 0 ||
        facts.pollEventHandle3_80016EB8 == 0 ||
        !facts.pollResultKnown ||
        facts.pollResult80016EB8 != 1 ||
        !facts.pollTimedOutKnown ||
        facts.pollTimedOut ||
        !facts.pollIterationCountKnown ||
        facts.pollIterationCount < 1 ||
        facts.pollIterationCount > kCardPollLimit80016EB8 ||
        !facts.waitCallCountKnown80035560 ||
        facts.waitCallCount80035560 < 0 ||
        facts.waitCallCount80035560 > kCardPollLimit80016EB8 ||
        !facts.closeKnown ||
        !facts.closeFdKnown ||
        facts.closeFd != facts.gp696Fd ||
        !facts.returnKnown ||
        facts.psxReturn800179B4 != 0 ||
        !facts.payloadLoadCallKnown ||
        !facts.payloadArgumentKnown ||
        facts.payloadArgument != kCardReadPayloadAddr8007ADE8 ||
        !facts.fullPayloadBytesKnown ||
        facts.fullPayloadBytes == nullptr ||
        facts.fullPayloadByteCount < kCardReadBlockBytes800179B4) {
        return false;
    }
    return true;
}

void CopyRuntimeTypedFactsRowName800179B4(
    char (&rowName)[32],
    const char (&selectedTitle)[32]) {
    std::size_t i = 0;
    for (; i + 1u < sizeof(rowName) && selectedTitle[i] != '\0'; ++i) {
        rowName[i] = selectedTitle[i];
    }
    for (; i < sizeof(rowName); ++i) {
        rowName[i] = '\0';
    }
}

CardReadFeedback800179B4 BuildRuntimeState16Feedback800179B4(
    const State16CardReadRuntimeTypedFacts800179B4& facts) {
    CardReadFeedback800179B4 feedback{};
    feedback.feedbackKnown = true;
    feedback.word8007ABE4Known = true;
    feedback.word8007ABE4 = facts.rowCount;
    for (int32_t i = 0; i < kReadAttemptCount800179B4; ++i) {
        feedback.attempts[i].rowEnabledKnown = true;
        feedback.attempts[i].rowEnabled = false;
    }

    CardReadAttemptFeedback800179B4& attempt =
        feedback.attempts[facts.selectedBlockIndex];
    attempt.rowEnabled = true;
    attempt.rowNameKnown = true;
    CopyRuntimeTypedFactsRowName800179B4(attempt.rowName, facts.selectedTitle);
    attempt.rowNameBuffer8007CBE8Known = true;
    attempt.cardSelectorKnown = true;
    attempt.cardSelectorGp128 = facts.cardPortGp128;
    attempt.cardSelectorGp124 = facts.cardSlotGp124;
    attempt.pathBuilt800173A8 = true;
    attempt.pathOpenFlagsKnown800173A8 = true;
    attempt.pathOpenFlags800173A8 = kCardPathOpenFlags800173A8;
    attempt.openAttempted800173A8 = true;
    attempt.fdKnown800173A8 = true;
    attempt.fd800173A8 = facts.gp696Fd;
    attempt.gp696FdWriteKnown800173A8 = true;
    attempt.gp696Fd800173A8 = facts.gp696Fd;
    attempt.targetBufferKnown = true;
    attempt.targetBufferAddress = facts.targetBufferAddress;
    attempt.readLengthKnown = true;
    attempt.readLength = facts.readByteCount;
    attempt.payloadPointerKnown = true;
    attempt.payloadPointer = facts.payloadAddress;
    attempt.payloadPassedTo800164F8 = true;
    attempt.blockCountKnown = true;
    attempt.blockCount = facts.blockCount;
    attempt.clearEvents.called = true;
    attempt.clearEvents.eventHandlesKnown = true;
    attempt.clearEvents.eventHandles[0] = facts.pollEventHandle0_80016EB8;
    attempt.clearEvents.eventHandles[1] = facts.pollEventHandle1_80016EB8;
    attempt.clearEvents.eventHandles[2] = facts.pollEventHandle2_80016EB8;
    attempt.clearEvents.eventHandles[3] = facts.pollEventHandle3_80016EB8;
    for (int32_t i = 0; i < 4; ++i) {
        attempt.clearEvents.testEventResultsKnown[i] = true;
        attempt.clearEvents.testEventResults[i] = 0;
    }
    attempt.readSubmission.called = true;
    attempt.readSubmission.fdKnown = true;
    attempt.readSubmission.fd = facts.readFd;
    attempt.readSubmission.bufferAddressKnown = true;
    attempt.readSubmission.bufferAddress = facts.readBufferAddress;
    attempt.readSubmission.byteCountKnown = true;
    attempt.readSubmission.byteCount = facts.readByteCount;
    attempt.poll.called = true;
    attempt.poll.eventHandlesKnown = true;
    attempt.poll.eventHandles[0] = facts.pollEventHandle0_80016EB8;
    attempt.poll.eventHandles[1] = facts.pollEventHandle1_80016EB8;
    attempt.poll.eventHandles[2] = facts.pollEventHandle2_80016EB8;
    attempt.poll.eventHandles[3] = facts.pollEventHandle3_80016EB8;
    attempt.poll.resultKnown = true;
    attempt.poll.psxReturn = facts.pollResult80016EB8;
    attempt.poll.timedOutKnown = true;
    attempt.poll.timedOut = facts.pollTimedOut;
    attempt.poll.hitEventIndexKnown = true;
    attempt.poll.hitEventIndex = facts.pollResult80016EB8 - 1;
    attempt.poll.pollIterationCountKnown = true;
    attempt.poll.pollIterationCount = facts.pollIterationCount;
    attempt.poll.waitCallCountKnown80035560 = true;
    attempt.poll.waitCallCount80035560 = facts.waitCallCount80035560;
    attempt.closeKnown800179B4 = true;
    attempt.closeFdKnown800179B4 = true;
    attempt.closeFd800179B4 = facts.closeFd;
    attempt.blockBytesKnown = true;
    attempt.blockBytes = facts.fullPayloadBytes;
    attempt.blockByteCount = kCardReadBlockBytes800179B4;
    return feedback;
}

CardReadAttemptResult800179B4 BuildCardReadAttemptResult800179B4(
    const CardReadAttemptFeedback800179B4& input) {
    CardReadAttemptResult800179B4 out{};
    if (!input.rowEnabledKnown) {
        out.incomplete = true;
        return out;
    }
    if (!input.rowEnabled) {
        out.produced = true;
        out.rowSkipped = true;
        return out;
    }

    if (input.liveCase17PayloadViewKnown) {
        if (!input.successAuthorityKnown800179B4 ||
            !input.success800179B4 ||
            !input.targetBufferKnown ||
            input.targetBufferAddress != kCardReadBlockBufferAddr800179B4 ||
            !input.readLengthKnown ||
            input.readLength != kCardReadBlockBytes800179B4 ||
            !input.payloadPointerKnown ||
            input.payloadPointer != kCardReadPayloadAddr8007ADE8 ||
            !input.payloadPassedTo800164F8) {
            out.incomplete = true;
            return out;
        }

        if (!input.blockBytesKnown ||
            input.blockBytes == nullptr ||
            input.blockByteCount < kCardReadBlockBytes800179B4) {
            out.incomplete = true;
            return out;
        }
        out.produced = true;
        out.eventResult80016EB8 = 1;
        out.psxReturn800179B4 = 0;
        out.readSubmitted800173A8 = true;
        out.readSucceeded = true;
        out.readLengthKnown = true;
        out.readLength = kCardReadBlockBytes800179B4;
        out.payloadPointerKnown = true;
        out.payloadPointer = kCardReadPayloadAddr8007ADE8;
        out.payloadPassedTo800164F8 = true;
        out.payloadBytesAvailable = true;
        out.blockBytes = input.blockBytes;
        out.blockByteCount = kCardReadBlockBytes800179B4;
        return out;
    }

    if (!input.rowNameKnown ||
        !input.cardSelectorKnown ||
        !input.pathBuilt800173A8 ||
        !input.pathOpenFlagsKnown800173A8 ||
        input.pathOpenFlags800173A8 != kCardPathOpenFlags800173A8 ||
        !input.openAttempted800173A8 ||
        !input.fdKnown800173A8 ||
        !input.targetBufferKnown ||
        input.targetBufferAddress != kCardReadBlockBufferAddr800179B4 ||
        !input.blockCountKnown ||
        input.blockCount != kCardReadBlockCount800179B4 ||
        !IsCompletePoll80016EB8(input.poll) ||
        !input.closeKnown800179B4 ||
        !input.closeFdKnown800179B4) {
        out.incomplete = true;
        return out;
    }

    out.eventResult80016EB8 = input.poll.psxReturn;
    out.psxReturn800179B4 = input.poll.psxReturn == 1 ? 0 : -1;
    out.openFailed800173A8 = input.fd800173A8 < 0;
    if (out.openFailed800173A8) {
        if (!input.openFailCloseKnown800173A8 ||
            input.openFailCloseFd800173A8 != -1) {
            out.incomplete = true;
            return out;
        }
        out.produced = true;
        return out;
    }

    if (!input.gp696FdWriteKnown800173A8 ||
        input.gp696Fd800173A8 != input.fd800173A8 ||
        !IsCompleteClearEvents80016FC0(input.clearEvents) ||
        !IsExpectedReadSubmission800173A8(input.readSubmission,
                                         input.fd800173A8) ||
        input.closeFd800179B4 != input.fd800173A8) {
        out.incomplete = true;
        return out;
    }

    out.readSubmitted800173A8 = true;
    out.readSucceeded = out.psxReturn800179B4 == 0;
    out.readLengthKnown = true;
    out.readLength = kCardReadBlockBytes800179B4;
    out.payloadPointerKnown = true;
    out.payloadPointer = kCardReadPayloadAddr8007ADE8;
    out.payloadPassedTo800164F8 = true;
    if (out.readSucceeded) {
        if (!input.blockBytesKnown ||
            input.blockBytes == nullptr ||
            input.blockByteCount < kCardReadBlockBytes800179B4) {
            out.incomplete = true;
            return out;
        }
        out.payloadBytesAvailable = true;
        out.blockBytes = input.blockBytes;
        out.blockByteCount = kCardReadBlockBytes800179B4;
    }

    out.produced = true;
    return out;
}

}  // namespace

bool IsImportableState16RuntimeTypedFacts800179B4(
    const State16CardReadRuntimeTypedFacts800179B4& facts,
    int32_t selectedBlockIndex) {
    return IsImportableState16RuntimeTypedFacts800179B4Internal(
        facts,
        selectedBlockIndex);
}

static bool PublishState16CardReadTypedCarrier800179B4ForBlockWithSource(
    const CardReadFeedback800179B4& feedback,
    int32_t selectedBlockIndex,
    CardReadTypedCarrierSource800179B4 source) {
    State16CardReadTypedCarrier800179B4 next{};
    next.known = feedback.feedbackKnown;
    if (!next.known) {
        s_state16CardReadTypedCarrier800179B4 = {};
        return false;
    }

    next.source = source;
    next.feedback = feedback;
    BuildCardReadHalResult800179B4(next.feedback, &next.hal);
    next.selectedBlockKnown =
        selectedBlockIndex >= 0 && selectedBlockIndex < 15;
    next.selectedBlockIndex =
        next.selectedBlockKnown ? selectedBlockIndex : -1;
    next.typedReadSuccessKnown800179B4 = false;
    next.payloadBytesKnown8007ADE8 = false;
    next.incomplete = next.hal.incomplete;
    const int32_t selectedBlock = next.selectedBlockIndex;

    if (next.feedback.word8007ABE4 > 0) {
        for (int32_t i = 0; i < kReadAttemptCount800179B4; ++i) {
            CardReadAttemptFeedback800179B4& attempt =
                next.feedback.attempts[i];
            CardReadAttemptResult800179B4& result = next.hal.attempts[i];
            if (!result.produced || result.incomplete ||
                !result.readSucceeded ||
                !result.payloadBytesAvailable ||
                result.blockBytes == nullptr ||
                result.blockByteCount < kCardReadBlockBytes800179B4) {
                continue;
            }

            std::memcpy(next.blockStorage[i].data(),
                        result.blockBytes,
                        next.blockStorage[i].size());
            attempt.blockBytes = next.blockStorage[i].data();
            attempt.blockByteCount = next.blockStorage[i].size();
            result.blockBytes = next.blockStorage[i].data();
            result.blockByteCount = next.blockStorage[i].size();
            if (next.selectedBlockKnown && i == selectedBlock) {
                next.typedReadSuccessKnown800179B4 = true;
            }
        }
    }
    next.payloadBytesKnown8007ADE8 =
        next.selectedBlockKnown &&
        next.typedReadSuccessKnown800179B4;
    const bool sourceCanPublishPayloadLane =
        source ==
            CardReadTypedCarrierSource800179B4::RuntimeLowerCardProducer ||
        source ==
            CardReadTypedCarrierSource800179B4::DebugSyntheticFixture;
    next.state16LoadPayloadLaneKnown =
        next.payloadBytesKnown8007ADE8 &&
        !next.incomplete &&
        sourceCanPublishPayloadLane;
    next.producerWired800173A8_80016EB8_800179B4 =
        next.payloadBytesKnown8007ADE8 &&
        !next.incomplete &&
        source ==
            CardReadTypedCarrierSource800179B4::RuntimeLowerCardProducer;

    s_state16CardReadTypedCarrier800179B4 = next;
    return s_state16CardReadTypedCarrier800179B4
        .producerWired800173A8_80016EB8_800179B4;
}

bool PublishState16CardReadTypedCarrier800179B4(
    const CardReadFeedback800179B4& feedback) {
    return PublishState16CardReadTypedCarrier800179B4ForBlockWithSource(
        feedback,
        -1,
        CardReadTypedCarrierSource800179B4::Unknown);
}

bool PublishState16CardReadTypedCarrier800179B4ForBlock(
    const CardReadFeedback800179B4& feedback,
    int32_t selectedBlockIndex) {
    return PublishState16CardReadTypedCarrier800179B4ForBlockWithSource(
        feedback,
        selectedBlockIndex,
        CardReadTypedCarrierSource800179B4::Unknown);
}

bool PublishDebugState16CardReadTypedCarrier800179B4ForBlock(
    const CardReadFeedback800179B4& feedback,
    int32_t selectedBlockIndex) {
    return PublishState16CardReadTypedCarrier800179B4ForBlockWithSource(
        feedback,
        selectedBlockIndex,
        CardReadTypedCarrierSource800179B4::DebugSyntheticFixture);
}

bool PublishRuntimeState16CardReadTypedCarrier800179B4ForBlock(
    const CardReadFeedbackRequest800179B4& request,
    const CardReadFeedback800179B4& feedback,
    int32_t selectedBlockIndex) {
    if (!IsExpectedState16LoadPayloadReadRequest800179B4(request)) {
        s_state16CardReadTypedCarrier800179B4 = {};
        return false;
    }
    const bool published =
        PublishState16CardReadTypedCarrier800179B4ForBlockWithSource(
        feedback,
        selectedBlockIndex,
        CardReadTypedCarrierSource800179B4::RuntimeLowerCardProducer);
    if (!published) {
        s_state16CardReadTypedCarrier800179B4 = {};
        return false;
    }
    return true;
}

bool PublishRuntimeState16CardReadTypedCarrier800179B4FromTypedFacts(
    const State16CardReadRuntimeTypedFacts800179B4& facts,
    int32_t selectedBlockIndex) {
    if (!IsImportableState16RuntimeTypedFacts800179B4(
            facts,
            selectedBlockIndex)) {
        s_state16CardReadTypedCarrier800179B4 = {};
        return false;
    }

    const CardReadFeedbackRequest800179B4 request =
        MakeState16LoadPayloadReadRequest800179B4(facts.arg2);
    const CardReadFeedback800179B4 feedback =
        BuildRuntimeState16Feedback800179B4(facts);
    return PublishRuntimeState16CardReadTypedCarrier800179B4ForBlock(
        request,
        feedback,
        selectedBlockIndex);
}

CardReadFeedbackRequest800179B4
MakeState16LoadPayloadReadRequest800179B4(int32_t arg2) {
    CardReadFeedbackRequest800179B4 request{};
    request.state16LoadPayloadRequest80019D7C = true;
    request.arg2Known = true;
    request.arg2 = arg2;
    return request;
}

bool GetState16CardReadTypedCarrier800179B4(
    State16CardReadTypedCarrier800179B4* out) {
    if (!out) {
        return false;
    }
    *out = {};
    if (!s_state16CardReadTypedCarrier800179B4.known) {
        return false;
    }

    *out = s_state16CardReadTypedCarrier800179B4;
    for (int32_t i = 0; i < kReadAttemptCount800179B4; ++i) {
        CardReadAttemptFeedback800179B4& attempt =
            out->feedback.attempts[i];
        CardReadAttemptResult800179B4& result = out->hal.attempts[i];
        if (attempt.blockBytesKnown &&
            attempt.blockBytes != nullptr &&
            attempt.blockByteCount >= kCardReadBlockBytes800179B4) {
            attempt.blockBytes = out->blockStorage[i].data();
            attempt.blockByteCount = out->blockStorage[i].size();
        }
        if (result.payloadBytesAvailable &&
            result.blockBytes != nullptr &&
            result.blockByteCount >= kCardReadBlockBytes800179B4) {
            result.blockBytes = out->blockStorage[i].data();
            result.blockByteCount = out->blockStorage[i].size();
        }
    }
    return true;
}

void ClearState16CardReadTypedCarrier800179B4() {
    s_state16CardReadTypedCarrier800179B4 = {};
}

void BuildSaveUiWriteLowerFeedback80017A10(
    const CardWriteFeedback80017A10& input,
    CardWriteLowerFeedbackBuildResult80017A10* out) {
    if (!out) {
        return;
    }
    *out = {};
    if (!input.feedbackKnown) {
        return;
    }

    out->lowerFeedbackKnown = true;
    out->lowerFeedback.writeFeedbackKnown80017A10 = true;
    for (int32_t attempt = 0; attempt < kWriteAttemptCount80017A10;
         ++attempt) {
        const CardWriteAttemptFeedback80017A10& src =
            input.attempts[attempt];
        PrStage1SaveUiWriteAttemptFeedback80017A10& dst =
            out->lowerFeedback.writeFeedback80017A10.attempts[attempt];
        dst.scanResultKnown80017900 = src.scanResultKnown80017900;
        dst.scanResult80017900 = src.scanResult80017900;
        dst.openCheckKnown80017454 = src.openCheckKnown80017454;
        dst.openCheckReturnKnown80017454 =
            src.openCheckReturnKnown80017454;
        dst.openCheckReturn80017454 = src.openCheckReturn80017454;
        dst.openCheckFdKnown80017454 = src.openCheckFdKnown80017454;
        dst.openCheckFd80017454 = src.openCheckFd80017454;
        dst.openCheckCloseKnown80017454 =
            src.openCheckCloseKnown80017454;
        dst.openCheckCloseFd80017454 = src.openCheckCloseFd80017454;
        dst.openWriteKnown80017454 = src.openWriteKnown80017454;
        dst.openWriteFdKnown80017454 = src.openWriteFdKnown80017454;
        dst.openWriteFd80017454 = src.openWriteFd80017454;
        dst.openWriteReturnKnown80017454 =
            src.openWriteReturnKnown80017454;
        dst.openWriteReturn80017454 = src.openWriteReturn80017454;
        dst.gp696FdWriteKnown80017454 =
            src.gp696FdWriteKnown80017454;
        dst.gp696Fd80017454 = src.gp696Fd80017454;
        dst.clearSwEventsKnown80016FC0 = src.clearSwEventsKnown80016FC0;
        dst.writeKnown80017454 = src.writeKnown80017454;
        dst.writeByteCountKnown80017454 =
            src.writeByteCountKnown80017454;
        dst.writeByteCount80017454 = src.writeByteCount80017454;
        dst.writeReturnKnown80017454 = src.writeReturnKnown80017454;
        dst.writeReturn80017454 = src.writeReturn80017454;
        dst.submitReturnKnown80017454 = src.submitReturnKnown80017454;
        dst.submitReturn80017454 = src.submitReturn80017454;
        dst.waitCallKnown80035560 = src.waitCallKnown80035560;
        dst.waitArg80035560 = src.waitArg80035560;
        dst.pollResultKnown80016EB8 = src.pollResultKnown80016EB8;
        dst.pollResult80016EB8 = src.pollResult80016EB8;
        dst.closeResultKnown = src.closeResultKnown;
        dst.closeResult = src.closeResult;
        dst.closeFdKnown = src.closeFdKnown;
        dst.closeFd = src.closeFd;
        dst.gp696FdCloseKnown80017A10 =
            src.gp696FdCloseKnown80017A10;
        dst.gp696FdClose80017A10 = src.gp696FdClose80017A10;
        const bool openCheckRequired =
            src.scanResultKnown80017900 && src.scanResult80017900 != 1;
        const bool openCheckComplete =
            !openCheckRequired ||
            (src.openCheckKnown80017454 &&
             src.openCheckReturnKnown80017454 &&
             src.openCheckFdKnown80017454 &&
             src.openCheckReturn80017454 == src.openCheckFd80017454 &&
             (src.openCheckFd80017454 == -1 ||
              (src.openCheckCloseKnown80017454 &&
               src.openCheckCloseFd80017454 ==
                   src.openCheckFd80017454)));
        const bool openCheckFailed =
            openCheckRequired &&
            openCheckComplete &&
            src.openCheckFd80017454 == -1;
        const bool openCheckPassed =
            !openCheckRequired || (openCheckComplete && !openCheckFailed);
        const bool openWriteComplete =
            openCheckPassed &&
            src.openWriteKnown80017454 &&
            src.openWriteFdKnown80017454 &&
            src.openWriteReturnKnown80017454 &&
            src.openWriteReturn80017454 == src.openWriteFd80017454;
        const bool openWriteFailed =
            openWriteComplete && src.openWriteFd80017454 == -1;
        const bool waitPollCloseComplete =
            src.waitCallKnown80035560 &&
            src.waitArg80035560 == 4 &&
            src.pollResultKnown80016EB8 &&
            src.closeResultKnown &&
            src.closeFdKnown &&
            src.gp696FdCloseKnown80017A10 &&
            src.closeFd == src.gp696FdClose80017A10;
        const bool submitEarlyFailed =
            src.submitReturnKnown80017454 &&
            src.submitReturn80017454 == -1 &&
            (openCheckFailed || openWriteFailed);
        const bool writeSubmitted =
            src.submitReturnKnown80017454 &&
            src.submitReturn80017454 == 0 &&
            openWriteComplete &&
            src.openWriteFd80017454 != -1 &&
            src.gp696FdWriteKnown80017454 &&
            src.gp696Fd80017454 == src.openWriteFd80017454 &&
            src.clearSwEventsKnown80016FC0 &&
            src.writeKnown80017454 &&
            src.writeByteCountKnown80017454 &&
            src.writeReturnKnown80017454 &&
            src.gp696FdClose80017A10 == src.gp696Fd80017454;
        if (!src.scanResultKnown80017900 ||
            !openCheckComplete ||
            !(writeSubmitted || submitEarlyFailed) ||
            !waitPollCloseComplete) {
            out->anyMissingRequiredFact = true;
        }
        if (writeSubmitted && src.pollResult80016EB8 == 1) {
            break;
        }
    }
}

void BuildCardWriteFeedbackFromHostFacts80017A10(
    const CardWriteFeedbackProducerInput80017A10& input,
    CardWriteFeedbackProducerResult80017A10* out) {
    if (!out) {
        return;
    }
    *out = {};
    if (input.requestKnown) {
        out->requestUsed = true;
        out->requestMatched =
            IsExpectedWriteRequest80017A10(input.request);
        if (!out->requestMatched) {
            out->incomplete = true;
            return;
        }
    }
    if (!input.explicitFeedbackKnown && !input.hostFactsKnown) {
        out->incomplete = true;
        return;
    }

    out->feedback = input.explicitFeedbackKnown ? input.explicitFeedback
                                                : CardWriteFeedback80017A10{};
    out->explicitFeedbackUsed = input.explicitFeedbackKnown;
    if (input.hostFactsKnown) {
        if (!input.hostFacts.factsKnown) {
            out->incomplete = true;
            return;
        }
        out->hostFactsUsed = true;
        out->feedback.feedbackKnown = true;
        for (int32_t i = 0; i < kWriteAttemptCount80017A10; ++i) {
            const CardWriteHostAttemptFacts80017A10& src =
                input.hostFacts.attempts[i];
            CardWriteAttemptFeedback80017A10& dst = out->feedback.attempts[i];
            if (src.scanResultKnown80017900) {
                dst.scanResultKnown80017900 = true;
                dst.scanResult80017900 = src.scanResult80017900;
            }
            if (src.openCheckKnown80017454) {
                dst.openCheckKnown80017454 = true;
            }
            if (src.openCheckReturnKnown80017454) {
                dst.openCheckReturnKnown80017454 = true;
                dst.openCheckReturn80017454 = src.openCheckReturn80017454;
            }
            if (src.openCheckFdKnown80017454) {
                dst.openCheckFdKnown80017454 = true;
                dst.openCheckFd80017454 = src.openCheckFd80017454;
            }
            if (src.openCheckCloseKnown80017454) {
                dst.openCheckCloseKnown80017454 = true;
                dst.openCheckCloseFd80017454 =
                    src.openCheckCloseFd80017454;
            }
            if (src.openWriteKnown80017454) {
                dst.openWriteKnown80017454 = true;
            }
            if (src.openWriteFdKnown80017454) {
                dst.openWriteFdKnown80017454 = true;
                dst.openWriteFd80017454 = src.openWriteFd80017454;
            }
            if (src.openWriteReturnKnown80017454) {
                dst.openWriteReturnKnown80017454 = true;
                dst.openWriteReturn80017454 = src.openWriteReturn80017454;
            }
            if (src.gp696FdWriteKnown80017454) {
                dst.gp696FdWriteKnown80017454 = true;
                dst.gp696Fd80017454 = src.gp696Fd80017454;
            }
            if (src.clearSwEventsKnown80016FC0) {
                dst.clearSwEventsKnown80016FC0 = true;
            }
            if (src.writeKnown80017454) {
                dst.writeKnown80017454 = true;
            }
            if (src.writeByteCountKnown80017454) {
                dst.writeByteCountKnown80017454 = true;
                dst.writeByteCount80017454 = src.writeByteCount80017454;
            }
            if (src.writeReturnKnown80017454) {
                dst.writeReturnKnown80017454 = true;
                dst.writeReturn80017454 = src.writeReturn80017454;
            }
            if (src.submitReturnKnown80017454) {
                dst.submitReturnKnown80017454 = true;
                dst.submitReturn80017454 = src.submitReturn80017454;
            }
            if (src.waitCallKnown80035560) {
                dst.waitCallKnown80035560 = true;
                dst.waitArg80035560 = src.waitArg80035560;
            }
            if (src.pollResultKnown80016EB8) {
                dst.pollResultKnown80016EB8 = true;
                dst.pollResult80016EB8 = src.pollResult80016EB8;
            }
            if (src.closeResultKnown) {
                dst.closeResultKnown = true;
                dst.closeResult = src.closeResult;
            }
            if (src.closeFdKnown) {
                dst.closeFdKnown = true;
                dst.closeFd = src.closeFd;
            }
            if (src.gp696FdCloseKnown80017A10) {
                dst.gp696FdCloseKnown80017A10 = true;
                dst.gp696FdClose80017A10 =
                    src.gp696FdClose80017A10;
            }
        }
        out->writeResultsCarried = true;
    }

    out->produced = true;
}

void BuildSaveUiWriteLowerFeedbackFromProducerInput80017A10(
    const CardWriteFeedbackProducerInput80017A10& input,
    CardWriteLowerFeedbackBuildResult80017A10* out) {
    if (!out) {
        return;
    }
    *out = {};

    CardWriteFeedbackProducerResult80017A10 producer{};
    BuildCardWriteFeedbackFromHostFacts80017A10(input, &producer);
    if (!producer.produced || producer.incomplete) {
        return;
    }
    BuildSaveUiWriteLowerFeedback80017A10(producer.feedback, out);
}

bool BuildSaveUiWriteObservedSuccessFacts80017A10(
    const PrStage1SaveUi19148LowerFeedbackRequest& request,
    const CardWriteObservedSuccessEvidence80017A10& evidence,
    CardWriteHostFacts80017A10* out) {
    if (!out) {
        return false;
    }
    *out = {};
    if (!IsExpectedWriteRequest80017A10(request) ||
        request.nameAddress != kCardReadNameBufferAddr8007CBE8 ||
        request.dataAddress != kCardReadBlockBufferAddr800179B4 ||
        request.blockCount != kCardReadBlockCount800179B4 ||
        request.action.kind !=
            PrStage1SaveUi19148ActionKind::Call80017A10WriteSaveBlock) {
        return false;
    }
    if (evidence.source != CardWriteObservedSource80017A10::Recorder20260515 ||
        !evidence.callChainKnown ||
        evidence.frameCall80017A10 != 13386 ||
        evidence.callPc80017A10 != 0x80019D24u ||
        evidence.callRa80017A10 != 0x80019D2Cu ||
        !evidence.writeSubmitKnown ||
        evidence.frameWriteSubmit80017454 != 13394 ||
        evidence.writeBufferAddress != kCardReadBlockBufferAddr800179B4 ||
        evidence.writeByteCount != kCardReadBlockBytes800179B4 ||
        !evidence.pollSuccessKnown ||
        evidence.framePollSuccess80016EB8 != 13524 ||
        !evidence.commitKnown ||
        evidence.frameCommit80019458 != 13524 ||
        evidence.gp696AfterCommit != 2 ||
        evidence.gp716AfterCommit != 1 ||
        evidence.gp720AfterCommit != 1 ||
        evidence.gp724AfterCommit != 1) {
        return false;
    }

    out->factsKnown = true;
    CardWriteHostAttemptFacts80017A10& attempt = out->attempts[0];
    attempt.scanResultKnown80017900 = true;
    attempt.scanResult80017900 = 1;
    attempt.openWriteKnown80017454 = true;
    attempt.openWriteFdKnown80017454 = true;
    attempt.openWriteFd80017454 = 2;
    attempt.openWriteReturnKnown80017454 = true;
    attempt.openWriteReturn80017454 = 2;
    attempt.gp696FdWriteKnown80017454 = true;
    attempt.gp696Fd80017454 = 2;
    attempt.clearSwEventsKnown80016FC0 = true;
    attempt.writeKnown80017454 = true;
    attempt.writeByteCountKnown80017454 = true;
    attempt.writeByteCount80017454 =
        request.blockCount << 13;
    attempt.writeReturnKnown80017454 = true;
    attempt.writeReturn80017454 = 0;
    attempt.submitReturnKnown80017454 = true;
    attempt.submitReturn80017454 = 0;
    attempt.waitCallKnown80035560 = true;
    attempt.waitArg80035560 = 4;
    attempt.pollResultKnown80016EB8 = true;
    attempt.pollResult80016EB8 = 1;
    attempt.closeResultKnown = true;
    attempt.closeResult = 0;
    attempt.closeFdKnown = true;
    attempt.closeFd = 2;
    attempt.gp696FdCloseKnown80017A10 = true;
    attempt.gp696FdClose80017A10 = 2;
    return true;
}

void BuildSaveUiCardIoLowerFeedbackFromHostFacts80017594(
    const CardIoHostFacts80017594& input,
    CardIoLowerFeedbackBuildResult80017594* out) {
    if (!out) {
        return;
    }
    *out = {};
    if (!input.factsKnown) {
        out->incomplete = true;
        return;
    }

    out->lowerFeedbackKnown = true;
    out->lowerFeedback.cardIoFeedbackKnown80017594 = true;
    PrStage1SaveUiCardIoFeedback80017594& dst =
        out->lowerFeedback.cardIoFeedback80017594;
    dst.stateBeforeKnown = input.stateBeforeKnown;
    dst.stateBefore = input.stateBefore;
    dst.stateAfterKnown = input.stateAfterKnown;
    dst.stateAfter = input.stateAfter;
    dst.cardInfoKnown = input.cardInfoKnown;
    dst.cardInfoArgKnown = input.cardInfoArgKnown;
    dst.cardInfoArg = input.cardInfoArg;
    dst.pollSwKnown80016E18 = input.pollSwKnown80016E18;
    dst.pollSwResult80016E18 = input.pollSwResult80016E18;
    dst.pollSwGp700BeforeKnown80016E18 =
        input.pollSwGp700BeforeKnown80016E18;
    dst.pollSwGp700Before80016E18 = input.pollSwGp700Before80016E18;
    dst.pollSwGp700AfterKnown80016E18 =
        input.pollSwGp700AfterKnown80016E18;
    dst.pollSwGp700After80016E18 = input.pollSwGp700After80016E18;
    dst.pollSwTimedOutKnown80016E18 =
        input.pollSwTimedOutKnown80016E18;
    dst.pollSwTimedOut80016E18 = input.pollSwTimedOut80016E18;
    dst.clearSwEventsKnown80016FC0 = input.clearSwEventsKnown80016FC0;
    dst.cardLoadKnown = input.cardLoadKnown;
    dst.cardLoadArgKnown = input.cardLoadArgKnown;
    dst.cardLoadArg = input.cardLoadArg;
    dst.drainHwEventsKnown8001707C = input.drainHwEventsKnown8001707C;
    dst.resetHwEventsKnown80047EE4 = input.resetHwEventsKnown80047EE4;
    dst.resetHwNewCardKnown80047EE4 =
        input.resetHwNewCardKnown80047EE4;
    dst.resetHwCardWriteArgsKnown80047EE4 =
        input.resetHwCardWriteArgsKnown80047EE4;
    dst.resetHwCardWriteArg0_80047EE4 =
        input.resetHwCardWriteArg0_80047EE4;
    dst.resetHwCardWriteArg1_80047EE4 =
        input.resetHwCardWriteArg1_80047EE4;
    dst.resetHwCardWriteArg2_80047EE4 =
        input.resetHwCardWriteArg2_80047EE4;
    dst.resetHwCardWriteResultKnown80047EE4 =
        input.resetHwCardWriteResultKnown80047EE4;
    dst.resetHwCardWriteResult80047EE4 =
        input.resetHwCardWriteResult80047EE4;
    dst.pollHwKnown80017008 = input.pollHwKnown80017008;
    dst.pollHwResult80017008 = input.pollHwResult80017008;
}

bool BuildSaveUiCardIoObservedNormalPathFacts80017594(
    const PrStage1SaveUi19148LowerFeedbackRequest& request,
    CardIoHostFacts80017594* out) {
    if (!out) {
        return false;
    }
    *out = {};
    if (request.kind !=
            PrStage1SaveUi19148LowerFeedbackRequestKind::CardIo80017594 ||
        request.psxFunction != 0x80017594u) {
        return false;
    }

    const PrStage1SaveUiCardIoState80017594& before =
        request.cardIoState;
    out->factsKnown = true;
    out->stateBeforeKnown = true;
    out->stateBefore = before;
    out->stateAfterKnown = true;
    out->stateAfter = before;

    switch (before.dword800917E8) {
    case 0:
        out->cardInfoKnown = true;
        out->cardInfoArgKnown = true;
        out->cardInfoArg = 0;
        out->stateAfter.dword800917E8 = 1;
        out->stateAfter.dword800917EC = 0;
        out->stateAfter.gp700 = 300;
        return true;

    case 1:
        out->factsKnown = false;
        out->stateAfterKnown = false;
        return false;

    case 2:
        out->clearSwEventsKnown80016FC0 = true;
        out->cardLoadKnown = true;
        out->cardLoadArgKnown = true;
        out->cardLoadArg = 0;
        out->stateAfter.dword800917E8 = 3;
        out->stateAfter.gp700 = 300;
        return true;

    case 3: {
        out->factsKnown = false;
        out->stateAfterKnown = false;
        return false;
    }

    case 4:
        out->stateAfter.dword800917E8 = 0;
        out->stateAfter.dword800917EC = before.dword800917F0;
        return true;

    default:
        out->factsKnown = false;
        out->stateAfterKnown = false;
        return false;
    }
}

void BuildSaveUiFormatLowerFeedbackFromHostFacts80017B60(
    const CardFormatHostFacts80017B60& input,
    CardFormatLowerFeedbackBuildResult80017B60* out) {
    if (!out) {
        return;
    }
    *out = {};
    if (!input.factsKnown) {
        out->incomplete = true;
        return;
    }

    out->lowerFeedbackKnown = true;
    out->lowerFeedback.formatFeedbackKnown80017B60 = true;
    for (int32_t i = 0; i < 3; ++i) {
        const CardFormatHostAttemptFacts80017B60& src = input.attempts[i];
        PrStage1SaveUiFormatAttemptFeedback80017B60& dst =
            out->lowerFeedback.formatFeedback80017B60.attempts[i];
        dst.drainHwEventsKnown8001707C = src.drainHwEventsKnown8001707C;
        dst.formatKnown = src.formatKnown;
        dst.formatArgsKnown = src.formatArgsKnown;
        dst.formatArg0 = src.formatArg0;
        dst.formatArg1 = src.formatArg1;
        dst.pollResultKnown80017008 = src.pollResultKnown80017008;
        dst.pollResult80017008 = src.pollResult80017008;
    }
}

void BuildCardReadHalResult800179B4(
    const CardReadFeedback800179B4& input,
    CardReadHalBuildResult800179B4* out) {
    if (!out) {
        return;
    }
    *out = {};
    if (!input.feedbackKnown) {
        return;
    }
    if (!input.word8007ABE4Known) {
        out->incomplete = true;
        return;
    }

    out->produced = true;
    if (input.word8007ABE4 <= 0) {
        return;
    }

    for (int32_t i = 0; i < kReadAttemptCount800179B4; ++i) {
        out->attempts[i] =
            BuildCardReadAttemptResult800179B4(input.attempts[i]);
        if (!out->attempts[i].produced || out->attempts[i].incomplete) {
            out->incomplete = true;
        }
        if (out->attempts[i].payloadBytesAvailable) {
            out->anyPayloadBytesAvailable = true;
        }
    }
}

void BuildCardReadFeedbackFromHostFacts800179B4(
    const CardReadFeedbackProducerInput800179B4& input,
    CardReadFeedbackProducerResult800179B4* out) {
    if (!out) {
        return;
    }
    *out = {};
    if (input.requestKnown) {
        out->requestUsed = true;
        out->requestMatched =
            IsExpectedReadRequest800179B4(input.request);
        if (!out->requestMatched) {
            out->incomplete = true;
            return;
        }
        out->triggerChainKnown = true;
        out->arg2Known = input.request.arg2Known;
        out->arg2 = input.request.arg2;
        out->rowNameBuffer8007CBE8Known = true;
        out->readLengthKnown = true;
        out->readLength = input.request.blockBytes;
        out->payloadPointerKnown = true;
        out->payloadPointer = input.request.payloadAddress;
        out->payloadPassedTo800164F8 = true;
    }
    if (!input.explicitFeedbackKnown && !input.hostFactsKnown) {
        out->incomplete = true;
        return;
    }

    out->feedback =
        input.explicitFeedbackKnown ? input.explicitFeedback
                                    : CardReadFeedback800179B4{};
    out->explicitFeedbackUsed = input.explicitFeedbackKnown;
    if (input.hostFactsKnown) {
        if (!input.hostFacts.factsKnown) {
            out->incomplete = true;
            return;
        }
        out->hostFactsUsed = true;
        out->feedback.feedbackKnown = true;
        if (input.hostFacts.triggerChainKnown) {
            out->triggerChainKnown = true;
        }
        if (input.hostFacts.arg2Known) {
            out->arg2Known = true;
            out->arg2 = input.hostFacts.arg2;
        }
        if (input.hostFacts.word8007ABE4Known) {
            out->feedback.word8007ABE4Known = true;
            out->feedback.word8007ABE4 = input.hostFacts.word8007ABE4;
            out->word8007ABE4Known = true;
            out->word8007ABE4 = input.hostFacts.word8007ABE4;
        }
        if (input.hostFacts.rowNameBuffer8007CBE8Known) {
            out->rowNameBuffer8007CBE8Known = true;
        }
        if (input.hostFacts.readLengthKnown) {
            out->readLengthKnown = true;
            out->readLength = input.hostFacts.readLength;
        }
        if (input.hostFacts.payloadPointerKnown) {
            out->payloadPointerKnown = true;
            out->payloadPointer = input.hostFacts.payloadPointer;
        }
        if (input.hostFacts.payloadPassedTo800164F8) {
            out->payloadPassedTo800164F8 = true;
        }
        for (int32_t i = 0; i < kReadAttemptCount800179B4; ++i) {
            const CardReadHostAttemptFacts800179B4& src =
                input.hostFacts.attempts[i];
            CardReadAttemptFeedback800179B4& dst =
                out->feedback.attempts[i];
            const bool rowCoveredByLiveCount =
                input.hostFacts.word8007ABE4Known &&
                i < input.hostFacts.word8007ABE4;
            if (src.rowEnabledKnown) {
                dst.rowEnabledKnown = true;
                dst.rowEnabled = src.rowEnabled;
            } else if (rowCoveredByLiveCount) {
                dst.rowEnabledKnown = true;
                dst.rowEnabled = true;
            }
            if (src.rowNameKnown) {
                dst.rowNameKnown = true;
                for (std::size_t c = 0; c < sizeof(dst.rowName); ++c) {
                    dst.rowName[c] = src.rowName[c];
                }
            }
            if (src.rowNameBuffer8007CBE8Known ||
                input.hostFacts.rowNameBuffer8007CBE8Known) {
                dst.rowNameBuffer8007CBE8Known = true;
            }
            if (src.cardSelectorKnown) {
                dst.cardSelectorKnown = true;
                dst.cardSelectorGp128 = src.cardSelectorGp128;
                dst.cardSelectorGp124 = src.cardSelectorGp124;
            }
            if (input.hostFacts.readBufferKnown) {
                dst.targetBufferKnown = true;
                dst.targetBufferAddress = input.hostFacts.readBuffer;
            }
            if (input.hostFacts.readLengthKnown) {
                dst.readLengthKnown = true;
                dst.readLength = input.hostFacts.readLength;
            }
            if (input.hostFacts.payloadPointerKnown) {
                dst.payloadPointerKnown = true;
                dst.payloadPointer = input.hostFacts.payloadPointer;
            }
            if (input.hostFacts.payloadPassedTo800164F8) {
                dst.payloadPassedTo800164F8 = true;
            }
            if (input.requestKnown && out->requestMatched) {
                dst.blockCountKnown = true;
                dst.blockCount = input.request.blockCount;
                dst.readSubmission.called = true;
                dst.readSubmission.bufferAddressKnown =
                    input.hostFacts.readBufferKnown;
                dst.readSubmission.bufferAddress = input.hostFacts.readBuffer;
                dst.readSubmission.byteCountKnown =
                    input.hostFacts.readLengthKnown;
                dst.readSubmission.byteCount = input.hostFacts.readLength;
            }
            if (src.readBufferBytesKnown) {
                dst.blockBytesKnown = true;
                dst.blockBytes = src.readBufferBytes;
                dst.blockByteCount = src.readBufferByteCount;
            }
        }
    }

    out->produced = true;
}

bool PublishCase17CardReadTypedCarrier800179B4(
    const CardReadFeedback800179B4& feedback,
    CardReadTypedCarrierSource800179B4 source) {
    Case17CardReadTypedCarrier800179B4 next{};
    next.known = feedback.feedbackKnown;
    if (!next.known) {
        s_case17CardReadTypedCarrier800179B4 = {};
        return false;
    }

    next.source = source;
    next.feedback = feedback;
    BuildCardReadHalResult800179B4(next.feedback, &next.hal);
    next.typedReadSuccessKnown800179B4 = false;
    next.payloadBytesKnown8007ADE8 = false;
    next.incomplete = next.hal.incomplete;

    if (next.feedback.word8007ABE4 > 0) {
        for (int32_t i = 0; i < kReadAttemptCount800179B4; ++i) {
            CardReadAttemptFeedback800179B4& attempt =
                next.feedback.attempts[i];
            CardReadAttemptResult800179B4& result = next.hal.attempts[i];
            if (!result.produced || result.incomplete ||
                !result.readSucceeded ||
                !result.payloadBytesAvailable ||
                result.blockBytes == nullptr ||
                result.blockByteCount < kCardReadBlockBytes800179B4) {
                continue;
            }

            std::memcpy(next.blockStorage[i].data(),
                        result.blockBytes,
                        next.blockStorage[i].size());
            attempt.blockBytes = next.blockStorage[i].data();
            attempt.blockByteCount = next.blockStorage[i].size();
            result.blockBytes = next.blockStorage[i].data();
            result.blockByteCount = next.blockStorage[i].size();
            next.typedReadSuccessKnown800179B4 = true;
        }
    }

    next.payloadBytesKnown8007ADE8 =
        next.hal.anyPayloadBytesAvailable &&
        next.typedReadSuccessKnown800179B4;
    const bool sourceCanPublishPayloadLane =
        source ==
        CardReadTypedCarrierSource800179B4::RuntimeLowerCardProducer;
    next.case17HiScorePayloadLaneKnown =
        next.typedReadSuccessKnown800179B4 &&
        next.payloadBytesKnown8007ADE8 &&
        !next.incomplete &&
        sourceCanPublishPayloadLane;
    next.producerWired800173A8_80016EB8_800179B4 =
        next.case17HiScorePayloadLaneKnown;

    s_case17CardReadTypedCarrier800179B4 = next;
    return s_case17CardReadTypedCarrier800179B4
        .producerWired800173A8_80016EB8_800179B4;
}

bool GetCase17CardReadTypedCarrier800179B4(
    Case17CardReadTypedCarrier800179B4* out) {
    if (!out) {
        return false;
    }
    *out = {};
    if (!s_case17CardReadTypedCarrier800179B4.known) {
        return false;
    }

    *out = s_case17CardReadTypedCarrier800179B4;
    for (int32_t i = 0; i < kReadAttemptCount800179B4; ++i) {
        CardReadAttemptFeedback800179B4& attempt =
            out->feedback.attempts[i];
        CardReadAttemptResult800179B4& result = out->hal.attempts[i];
        if (attempt.blockBytesKnown &&
            attempt.blockBytes != nullptr &&
            attempt.blockByteCount >= kCardReadBlockBytes800179B4) {
            attempt.blockBytes = out->blockStorage[i].data();
            attempt.blockByteCount = out->blockStorage[i].size();
        }
        if (result.payloadBytesAvailable &&
            result.blockBytes != nullptr &&
            result.blockByteCount >= kCardReadBlockBytes800179B4) {
            result.blockBytes = out->blockStorage[i].data();
            result.blockByteCount = out->blockStorage[i].size();
        }
    }
    return true;
}

void ClearCase17CardReadTypedCarrier800179B4() {
    s_case17CardReadTypedCarrier800179B4 = {};
}

}  // namespace PrStage1SaveCardHalDirect
