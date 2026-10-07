#include "pr_stage1_save_card_hal_direct.h"
#include "logger.h"

#include <cstring>
#include <memory>

namespace PrStage1SaveCardHalDirect {

struct LoadSavePayloadAuthorityAccess800164B4 {
    static PrStagePayloadBankDirect::LoadSavePayloadAuthority800164B4 Mint(
        const uint8_t* payload,
        std::size_t payloadBytes) {
        PrStagePayloadBankDirect::LoadSavePayloadAuthority800164B4 authority{};
        authority.Mint(
            PrStagePayloadBankDirect::kTypedPayloadSourceAddress8007ADE8,
            payload,
            payloadBytes);
        return authority;
    }
};

namespace {

State16CardReadTypedCarrier800179B4 s_state16CardReadTypedCarrier800179B4{};
Case17CardReadTypedCarrier800179B4 s_case17CardReadTypedCarrier800179B4{};
SaveUiWriteTypedCarrier80017A10 s_saveUiWriteTypedCarrier80017A10{};
SaveUiFormatTypedCarrier80017B60 s_saveUiFormatTypedCarrier80017B60{};
SaveUiCardIoState3TypedPollCarrier80017594
    s_saveUiCardIoState3TypedPollCarrier80017594{};
CardTranslatedEventBrokerState800170C4
    s_translatedCardEventBroker800170C4{};
CardCommunicationSetupState80017524
    s_cardCommunicationSetup80017524{};
CardCommunicationTeardownState80017574
    s_cardCommunicationTeardown80017574{};

static constexpr std::size_t kDirectCardImageBytes8007A318 =
    128u * 1024u;
static constexpr std::size_t kDirectCardImageBlockBytes8007A318 =
    kCardReadBlockBytes800179B4;
static constexpr std::size_t kDirectCardImageFrameBytes8007A318 = 128u;
static constexpr std::size_t kDirectCardImageDirectoryNameOffset8007A318 =
    0x0Au;
static constexpr std::size_t kDirectCardImageDirectoryNameBytes8007A318 =
    20u;

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

bool IsExpectedFormatRequest80017B60(
    const PrStage1SaveUi19148LowerFeedbackRequest& request) {
    return request.kind ==
               PrStage1SaveUi19148LowerFeedbackRequestKind::Format80017B60 &&
           request.psxFunction == kFn80017B60 &&
           request.retryCount == kFormatAttemptCount80017B60 &&
           request.formatArg0 == kFormatArg0_80017B60 &&
           request.formatArg1 == kFormatArg1_80017B60 &&
           request.action.kind ==
               PrStage1SaveUi19148ActionKind::Call80017B60FormatCard;
}

bool IsExpectedState3CardIoPollRequest80017594(
    const PrStage1SaveUi19148LowerFeedbackRequest& request) {
    return request.kind ==
               PrStage1SaveUi19148LowerFeedbackRequestKind::CardIo80017594 &&
           request.psxFunction == 0x80017594u &&
           request.cardIoState.dword800917E8 == 3;
}

bool CardIoStateEquals80017594(
    const PrStage1SaveUiCardIoState80017594& lhs,
    const PrStage1SaveUiCardIoState80017594& rhs) {
    return lhs.dword800917E8 == rhs.dword800917E8 &&
           lhs.dword800917EC == rhs.dword800917EC &&
           lhs.dword800917F0 == rhs.dword800917F0 &&
           lhs.dword800917F4 == rhs.dword800917F4 &&
           lhs.gp700 == rhs.gp700;
}

PrStage1SaveUiCardIoState80017594 ExpectedState3AfterTypedPoll80016E18(
    const PrStage1SaveUiCardIoState80017594& before,
    int32_t pollResult) {
    PrStage1SaveUiCardIoState80017594 after = before;
    after.gp700 = before.gp700 - 1;
    if (pollResult == 0) {
        return after;
    }

    after.dword800917E8 = 4;
    after.dword800917F4 = 0;
    if (pollResult == 1) {
        after.dword800917F4 = 1;
    } else if (pollResult == 3) {
        after.dword800917F0 = 3;
    } else if (pollResult == 4) {
        after.dword800917F0 = 5;
    } else {
        after.dword800917F0 = 2;
    }
    return after;
}

bool IsImportableState3CardIoTypedPollFacts80017594(
    const PrStage1SaveUi19148LowerFeedbackRequest& request,
    const CardIoHostFacts80017594& facts) {
    if (!IsExpectedState3CardIoPollRequest80017594(request) ||
        !facts.factsKnown ||
        !facts.stateBeforeKnown ||
        !facts.stateAfterKnown ||
        !CardIoStateEquals80017594(facts.stateBefore, request.cardIoState) ||
        !facts.pollSwKnown80016E18 ||
        facts.pollSwResult80016E18 < 0 ||
        facts.pollSwResult80016E18 > 4 ||
        !facts.pollSwGp700BeforeKnown80016E18 ||
        facts.pollSwGp700Before80016E18 != request.cardIoState.gp700 ||
        !facts.pollSwGp700AfterKnown80016E18 ||
        facts.pollSwGp700After80016E18 != request.cardIoState.gp700 - 1 ||
        !facts.pollSwTimedOutKnown80016E18) {
        return false;
    }

    const bool timedOut = facts.pollSwGp700After80016E18 < 0;
    if (facts.pollSwTimedOut80016E18 != timedOut ||
        (timedOut && facts.pollSwResult80016E18 != 2)) {
        return false;
    }

    if (!CardIoStateEquals80017594(
            facts.stateAfter,
            ExpectedState3AfterTypedPoll80016E18(request.cardIoState,
                                                facts.pollSwResult80016E18))) {
        return false;
    }

    return !facts.cardInfoKnown &&
           !facts.cardLoadKnown &&
           !facts.clearSwEventsKnown80016FC0 &&
           !facts.drainHwEventsKnown8001707C &&
           !facts.resetHwEventsKnown80047EE4 &&
           !facts.pollHwKnown80017008;
}

bool IsImportableSaveUiWriteRuntimeFacts80017A10(
    const SaveUiWriteRuntimeFacts80017A10& facts,
    const PrStage1SaveUi19148LowerFeedbackRequest& request) {
    if (!IsExpectedWriteRequest80017A10(request) ||
        request.nameAddress != kCardReadNameBufferAddr8007CBE8 ||
        request.dataAddress != kCardReadBlockBufferAddr800179B4 ||
        request.blockCount != kCardReadBlockCount800179B4 ||
        request.action.kind !=
            PrStage1SaveUi19148ActionKind::Call80017A10WriteSaveBlock ||
        !facts.factsKnown ||
        !facts.scanResultKnown80017900 ||
        facts.scanResult80017900 != 1 ||
        !facts.openWriteKnown80017454 ||
        !facts.openWriteFdKnown80017454 ||
        facts.openWriteFd80017454 < 0 ||
        !facts.openWriteReturnKnown80017454 ||
        facts.openWriteReturn80017454 != facts.openWriteFd80017454 ||
        !facts.gp696FdWriteKnown80017454 ||
        facts.gp696Fd80017454 != facts.openWriteFd80017454 ||
        !facts.clearSwEventsKnown80016FC0 ||
        !facts.writeKnown80017454 ||
        !facts.writeByteCountKnown80017454 ||
        facts.writeByteCount80017454 !=
            static_cast<int32_t>(kCardReadBlockBytes800179B4) ||
        !facts.writeReturnKnown80017454 ||
        !facts.submitReturnKnown80017454 ||
        facts.submitReturn80017454 != 0 ||
        !facts.waitCallKnown80035560 ||
        facts.waitArg80035560 != 4 ||
        !facts.pollResultKnown80016EB8 ||
        facts.pollResult80016EB8 != 1 ||
        !facts.closeResultKnown ||
        !facts.closeFdKnown ||
        !facts.gp696FdCloseKnown80017A10 ||
        facts.closeFd != facts.gp696FdClose80017A10 ||
        facts.gp696FdClose80017A10 != facts.gp696Fd80017454) {
        return false;
    }
    return true;
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

bool IsExpectedCase17HiScoreReadRequest800179B4(
    const CardReadFeedbackRequest800179B4& request) {
    return IsExpectedReadRequest800179B4(request) &&
           !request.state16LoadPayloadRequest80019D7C &&
           request.case17HiScoreRequest80019D7C &&
           request.arg2 != kCase17Arg2EarlyReturn80019D7C;
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

bool CopyDirectCardImageDirectoryTitle800179B4(
    const PrStage1SaveUiCardImagePersistenceView8007A318& view,
    int32_t physicalBlockIndex,
    char (&outTitle)[32]) {
    std::memset(outTitle, 0, sizeof(outTitle));
    if (!view.known ||
        !view.slotPolicyKnown ||
        physicalBlockIndex < 0 ||
        physicalBlockIndex >= kReadAttemptCount800179B4 ||
        view.bytes == nullptr ||
        view.byteCount != kDirectCardImageBytes8007A318 ||
        view.byteSize != kDirectCardImageBytes8007A318) {
        return false;
    }

    const std::size_t dirOffset =
        static_cast<std::size_t>(physicalBlockIndex + 1) *
        kDirectCardImageFrameBytes8007A318;
    if (dirOffset + kDirectCardImageFrameBytes8007A318 >
        view.byteCount) {
        return false;
    }

    const uint8_t* directoryEntry = view.bytes + dirOffset;
    if (directoryEntry[0] != 0x51u ||
        directoryEntry[4] != 0x00u ||
        directoryEntry[5] != 0x20u ||
        directoryEntry[6] != 0x00u ||
        directoryEntry[7] != 0x00u) {
        return false;
    }

    const uint8_t* name =
        directoryEntry + kDirectCardImageDirectoryNameOffset8007A318;
    bool terminated = false;
    for (std::size_t i = 0;
         i < kDirectCardImageDirectoryNameBytes8007A318 && i + 1u < 32u;
         ++i) {
        outTitle[i] = static_cast<char>(name[i]);
        if (name[i] == 0u) {
            terminated = true;
            break;
        }
    }
    if (!terminated) {
        outTitle[kDirectCardImageDirectoryNameBytes8007A318] = '\0';
    }
    return IsCommandSafeState16Title800179B4(outTitle);
}

bool CopySelectedDirectCardImageDirectoryTitle800179B4(
    const PrStage1SaveUiCardImagePersistenceView8007A318& view,
    int32_t selectedBlockIndex,
    char (&outTitle)[32]) {
    if (view.blockIndex != selectedBlockIndex) {
        std::memset(outTitle, 0, sizeof(outTitle));
        return false;
    }
    return CopyDirectCardImageDirectoryTitle800179B4(
        view,
        selectedBlockIndex,
        outTitle);
}

bool IsCase17GameSaveDirectoryTitle80019D7CCase6(const char (&title)[32]) {
    static constexpr char kFilenamePrefix80019D7CCase6[] = "BASCUS-94183";
    return std::strncmp(
               title,
               kFilenamePrefix80019D7CCase6,
               sizeof(kFilenamePrefix80019D7CCase6) - 1u) == 0;
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
            !input.targetBufferKnown ||
            input.targetBufferAddress != kCardReadBlockBufferAddr800179B4 ||
            !input.readLengthKnown ||
            input.readLength != kCardReadBlockBytes800179B4 ||
            !input.payloadPointerKnown ||
            input.payloadPointer != kCardReadPayloadAddr8007ADE8 ||
            (input.success800179B4 && !input.payloadPassedTo800164F8)) {
            out.incomplete = true;
            return out;
        }

        if (!input.success800179B4) {
            // Bounded platform file-read failure, not an invented PSX event/FD.
            if (!input.rowNameKnown || !input.rowNameBuffer8007CBE8Known ||
                input.payloadPassedTo800164F8 || input.blockBytesKnown ||
                input.blockBytes != nullptr || input.blockByteCount != 0) {
                out.incomplete = true;
                return out;
            }
            out.produced = true;
            out.psxReturn800179B4 = -1;
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
    out.payloadPassedTo800164F8 = out.readSucceeded;
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
    if (next.producerWired800173A8_80016EB8_800179B4) {
        const uint8_t* payload =
            next.blockStorage[static_cast<size_t>(selectedBlock)].data() +
            kCardReadPayloadOffset8007ADE8;
        next.payloadAuthority800164B4 =
            LoadSavePayloadAuthorityAccess800164B4::Mint(
                payload,
                PrStagePayloadBankDirect::kByteCount80092F10);
    }

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

static bool PublishDirectState16CardReadAtLocation800173A8(
    const PrStage1SaveUiCardImagePersistenceView8007A318& view,
    int32_t selectedBlockIndex, int32_t sourcePhysicalBlock) {
    char selectedTitle[32]{};
    if (selectedBlockIndex < 0 || selectedBlockIndex >= kReadAttemptCount800179B4 ||
        !CopyDirectCardImageDirectoryTitle800179B4(
            view,
            sourcePhysicalBlock,
            selectedTitle)) {
        s_state16CardReadTypedCarrier800179B4 = {};
        return false;
    }

    const std::size_t blockOffset =
        static_cast<std::size_t>(sourcePhysicalBlock + 1) *
        kDirectCardImageBlockBytes8007A318;
    if (blockOffset + kDirectCardImageBlockBytes8007A318 >
        view.byteCount) {
        s_state16CardReadTypedCarrier800179B4 = {};
        return false;
    }

    State16CardReadRuntimeTypedFacts800179B4 facts{};
    facts.factsKnown = true;
    facts.state16CallKnown = true;
    facts.selectedBlockKnown = true;
    facts.selectedBlockIndex = selectedBlockIndex;
    facts.selectedTitleKnown = true;
    std::memcpy(facts.selectedTitle,
                selectedTitle,
                sizeof(facts.selectedTitle));
    facts.rowCountKnown = true;
    facts.rowCount = selectedBlockIndex + 1;
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
    facts.gp696Fd = 2;
    facts.clearEventsCallKnown = true;
    facts.readSubmissionKnown = true;
    facts.readFdKnown = true;
    facts.readFd = 2;
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
    facts.closeFd = 2;
    facts.returnKnown = true;
    facts.psxReturn800179B4 = 0;
    facts.payloadLoadCallKnown = true;
    facts.payloadArgumentKnown = true;
    facts.payloadArgument = kCardReadPayloadAddr8007ADE8;
    facts.fullPayloadBytesKnown = true;
    facts.fullPayloadBytes = view.bytes + blockOffset;
    facts.fullPayloadByteCount = kDirectCardImageBlockBytes8007A318;
    return PublishRuntimeState16CardReadTypedCarrier800179B4FromTypedFacts(
        facts,
        selectedBlockIndex);
}

bool PublishRuntimeState16CardReadTypedCarrier800179B4FromDirectCardImagePersistenceSink(
    const PrStage1SaveUiCardImagePersistenceView8007A318& view,
    int32_t selectedBlockIndex) {
    char title[32]{};
    if (!CopySelectedDirectCardImageDirectoryTitle800179B4(view, selectedBlockIndex, title)) {
        s_state16CardReadTypedCarrier800179B4 = {};
        return false;
    }
    return PublishDirectState16CardReadAtLocation800173A8(view, selectedBlockIndex, selectedBlockIndex);
}

bool PublishRuntimeState16CardReadByName800173A8(
    const PrStage1SaveUiCardImagePersistenceView8007A318& view,
    const char* requestedName, std::size_t nameCapacity, int32_t requestSlot,
    NamedCardReadLocation800173A8* location) {
    s_state16CardReadTypedCarrier800179B4 = {};
    if (!location) return false;
    *location = {};
    if (!requestedName || requestSlot < 0 || requestSlot >= kReadAttemptCount800179B4) return false;
    char name[32]{};
    std::size_t length = 0;
    while (length < nameCapacity && length <= kDirectCardImageDirectoryNameBytes8007A318 && requestedName[length]) {
        name[length] = requestedName[length];
        ++length;
    }
    if (length == 0 || length >= nameCapacity || length > kDirectCardImageDirectoryNameBytes8007A318 ||
        !IsCommandSafeState16Title800179B4(name)) return false;
    for (int32_t physical = 0; physical < kReadAttemptCount800179B4; ++physical) {
        char candidate[32]{};
        if (!CopyDirectCardImageDirectoryTitle800179B4(view, physical, candidate) ||
            std::strcmp(candidate, name) != 0) continue;
        if (!PublishDirectState16CardReadAtLocation800173A8(view, requestSlot, physical)) return false;
        location->known = true;
        location->requestSlot = requestSlot;
        location->sourcePhysicalBlock = physical;
        return true;
    }
    return false;
}

CardReadFeedbackRequest800179B4
MakeState16LoadPayloadReadRequest800179B4(int32_t arg2) {
    CardReadFeedbackRequest800179B4 request{};
    request.state16LoadPayloadRequest80019D7C = true;
    request.arg2Known = true;
    request.arg2 = arg2;
    return request;
}

CardReadFeedbackRequest800179B4
MakeCase17HiScoreReadRequest800179B4(int32_t arg2) {
    CardReadFeedbackRequest800179B4 request{};
    request.case17HiScoreRequest80019D7C = true;
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
            !openCheckPassed ||
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

bool FinalizeSaveUiDirectWriteAttempt80017A10(
    const SaveUiDirectWriteCompletion80017A10& completion,
    CardWriteHostAttemptFacts80017A10* attempt) {
    if (attempt == nullptr || !completion.backendCompletionKnown ||
        !completion.waitCompleted80035560 ||
        completion.virtualFd80017454 < 0 ||
        !attempt->scanResultKnown80017900 ||
        !attempt->openWriteKnown80017454 ||
        !attempt->openWriteFdKnown80017454 ||
        attempt->openWriteFd80017454 != completion.virtualFd80017454 ||
        !attempt->openWriteReturnKnown80017454 ||
        attempt->openWriteReturn80017454 != completion.virtualFd80017454 ||
        !attempt->gp696FdWriteKnown80017454 ||
        attempt->gp696Fd80017454 != completion.virtualFd80017454 ||
        !attempt->clearSwEventsKnown80016FC0 ||
        !attempt->writeKnown80017454 ||
        !attempt->writeByteCountKnown80017454 ||
        attempt->writeByteCount80017454 !=
            static_cast<int32_t>(kCardReadBlockBytes800179B4) ||
        !attempt->submitReturnKnown80017454 ||
        attempt->submitReturn80017454 != 0 ||
        !attempt->waitCallKnown80035560 ||
        attempt->waitArg80035560 != 4) {
        return false;
    }

    attempt->writeReturnKnown80017454 = true;
    attempt->writeReturn80017454 =
        completion.backendAccepted
            ? static_cast<int32_t>(kCardReadBlockBytes800179B4)
            : -1;
    attempt->pollResultKnown80016EB8 = true;
    attempt->pollResult80016EB8 = completion.backendAccepted ? 1 : 2;
    attempt->closeResultKnown = true;
    attempt->closeResult = 0;
    attempt->closeFdKnown = true;
    attempt->closeFd = completion.virtualFd80017454;
    attempt->gp696FdCloseKnown80017A10 = true;
    attempt->gp696FdClose80017A10 = completion.virtualFd80017454;
    return true;
}

bool BuildRuntimeSaveUiWriteFacts80017A10(
    const SaveUiWriteRuntimeFacts80017A10& facts,
    CardWriteHostFacts80017A10* out) {
    if (!out) {
        return false;
    }
    *out = {};
    PrStage1SaveUi19148LowerFeedbackRequest request{};
    request.kind = PrStage1SaveUi19148LowerFeedbackRequestKind::Write80017A10;
    request.psxFunction = kFn80017A10;
    request.retryCount = kWriteAttemptCount80017A10;
    request.nameAddress = kCardReadNameBufferAddr8007CBE8;
    request.dataAddress = kCardReadBlockBufferAddr800179B4;
    request.blockCount = kCardReadBlockCount800179B4;
    request.writeCloseGp696FactRequired80017A10 = true;
    request.writeCloseGp696Address80017A10 =
        kWriteCloseGp696Address80017A10;
    request.writeFdMustMatchCloseGp69680017A10 = true;
    request.action.kind =
        PrStage1SaveUi19148ActionKind::Call80017A10WriteSaveBlock;
    if (!IsImportableSaveUiWriteRuntimeFacts80017A10(facts, request)) {
        return false;
    }

    out->factsKnown = true;
    CardWriteHostAttemptFacts80017A10& attempt = out->attempts[0];
    attempt.scanResultKnown80017900 = true;
    attempt.scanResult80017900 = facts.scanResult80017900;
    attempt.openWriteKnown80017454 = true;
    attempt.openWriteFdKnown80017454 = true;
    attempt.openWriteFd80017454 = facts.openWriteFd80017454;
    attempt.openWriteReturnKnown80017454 = true;
    attempt.openWriteReturn80017454 = facts.openWriteReturn80017454;
    attempt.gp696FdWriteKnown80017454 = true;
    attempt.gp696Fd80017454 = facts.gp696Fd80017454;
    attempt.clearSwEventsKnown80016FC0 = true;
    attempt.writeKnown80017454 = true;
    attempt.writeByteCountKnown80017454 = true;
    attempt.writeByteCount80017454 = facts.writeByteCount80017454;
    attempt.writeReturnKnown80017454 = true;
    attempt.writeReturn80017454 = facts.writeReturn80017454;
    attempt.submitReturnKnown80017454 = true;
    attempt.submitReturn80017454 = facts.submitReturn80017454;
    attempt.waitCallKnown80035560 = true;
    attempt.waitArg80035560 = facts.waitArg80035560;
    attempt.pollResultKnown80016EB8 = true;
    attempt.pollResult80016EB8 = facts.pollResult80016EB8;
    attempt.closeResultKnown = true;
    attempt.closeResult = facts.closeResult;
    attempt.closeFdKnown = true;
    attempt.closeFd = facts.closeFd;
    attempt.gp696FdCloseKnown80017A10 = true;
    attempt.gp696FdClose80017A10 = facts.gp696FdClose80017A10;
    return true;
}

void BuildSaveUiWriteFactsFromRuntimeProducerInput80017A10(
    const PrStage1SaveUi19148LowerFeedbackRequest& request,
    const SaveUiWriteRuntimeFacts80017A10& facts,
    SaveUiWriteRuntimeProducerResult80017A10* out) {
    if (!out) {
        return;
    }
    *out = {};
    out->requestUsed = true;
    out->requestMatched = IsExpectedWriteRequest80017A10(request) &&
                          request.nameAddress ==
                              kCardReadNameBufferAddr8007CBE8 &&
                          request.dataAddress ==
                              kCardReadBlockBufferAddr800179B4 &&
                          request.blockCount == kCardReadBlockCount800179B4 &&
                          request.action.kind ==
                              PrStage1SaveUi19148ActionKind::
                                  Call80017A10WriteSaveBlock;
    out->runtimeFactsKnown = facts.factsKnown;
    if (!out->requestMatched ||
        !IsImportableSaveUiWriteRuntimeFacts80017A10(facts, request) ||
        !BuildRuntimeSaveUiWriteFacts80017A10(facts, &out->hostFacts)) {
        out->incomplete = true;
        return;
    }
    out->produced = true;
}

bool PublishRuntimeSaveUiWriteTypedCarrier80017A10(
    const PrStage1SaveUi19148LowerFeedbackRequest& request,
    const SaveUiWriteRuntimeFacts80017A10& facts) {
    SaveUiWriteRuntimeProducerResult80017A10 producer{};
    BuildSaveUiWriteFactsFromRuntimeProducerInput80017A10(
        request,
        facts,
        &producer);
    if (!producer.produced || producer.incomplete ||
        !producer.requestMatched || !producer.hostFacts.factsKnown) {
        s_saveUiWriteTypedCarrier80017A10 = {};
        return false;
    }

    CardWriteFeedbackProducerInput80017A10 input{};
    input.requestKnown = true;
    input.request = request;
    input.hostFactsKnown = true;
    input.hostFacts = producer.hostFacts;
    CardWriteLowerFeedbackBuildResult80017A10 lower{};
    BuildSaveUiWriteLowerFeedbackFromProducerInput80017A10(input, &lower);
    if (!lower.lowerFeedbackKnown ||
        !lower.lowerFeedback.writeFeedbackKnown80017A10 ||
        lower.anyMissingRequiredFact) {
        s_saveUiWriteTypedCarrier80017A10 = {};
        return false;
    }

    const PrStage1SaveUiWriteFeedbackInput80017A10& writeFeedback =
        lower.lowerFeedback.writeFeedback80017A10;
    if (!writeFeedback.attempts[0].writeReturnKnown80017454 ||
        !writeFeedback.attempts[0].writeByteCountKnown80017454 ||
        writeFeedback.attempts[0].writeByteCount80017454 !=
            static_cast<int32_t>(kCardReadBlockBytes800179B4) ||
        !writeFeedback.attempts[0].pollResultKnown80016EB8 ||
        writeFeedback.attempts[0].pollResult80016EB8 != 1 ||
        !writeFeedback.attempts[0].closeResultKnown ||
        writeFeedback.attempts[0].closeResult != 0 ||
        !writeFeedback.attempts[0].closeFdKnown ||
        writeFeedback.attempts[0].closeFd != facts.gp696FdClose80017A10 ||
        !writeFeedback.attempts[0].gp696FdCloseKnown80017A10 ||
        writeFeedback.attempts[0].gp696FdClose80017A10 !=
            facts.gp696FdClose80017A10) {
        s_saveUiWriteTypedCarrier80017A10 = {};
        return false;
    }

    SaveUiWriteTypedCarrier80017A10 next{};
    next.known = true;
    next.producerWired80017900_80017454_80016EB8_80017A10 = true;
    next.typedWriteSuccessKnown80017A10 = true;
    next.incomplete = false;
    next.hostFacts = producer.hostFacts;
    next.lower = lower;
    s_saveUiWriteTypedCarrier80017A10 = next;
    return true;
}

bool GetSaveUiWriteTypedCarrier80017A10(
    SaveUiWriteTypedCarrier80017A10* out) {
    if (!out) {
        return false;
    }
    *out = {};
    if (!s_saveUiWriteTypedCarrier80017A10.known) {
        return false;
    }
    *out = s_saveUiWriteTypedCarrier80017A10;
    return true;
}

void ClearSaveUiWriteTypedCarrier80017A10() {
    s_saveUiWriteTypedCarrier80017A10 = {};
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

bool BuildSaveUiCardIoPollFactsFromResult80016E18(
    const PrStage1SaveUi19148LowerFeedbackRequest& request,
    int32_t pollResult80016E18,
    CardIoHostFacts80017594* out) {
    if (!out) {
        return false;
    }
    *out = {};
    if (request.kind !=
            PrStage1SaveUi19148LowerFeedbackRequestKind::CardIo80017594 ||
        request.psxFunction != 0x80017594u ||
        (request.cardIoState.dword800917E8 != 1 &&
         request.cardIoState.dword800917E8 != 3) ||
        pollResult80016E18 < 0 || pollResult80016E18 > 4) {
        return false;
    }

    const PrStage1SaveUiCardIoState80017594& before = request.cardIoState;
    const int32_t gp700After = before.gp700 - 1;
    const bool timedOut = gp700After < 0;
    const int32_t effectivePoll = timedOut ? 2 : pollResult80016E18;

    out->factsKnown = true;
    out->stateBeforeKnown = true;
    out->stateBefore = before;
    out->stateAfterKnown = true;
    out->stateAfter = before;
    out->stateAfter.gp700 = gp700After;
    out->pollSwKnown80016E18 = true;
    out->pollSwResult80016E18 = effectivePoll;
    out->pollSwGp700BeforeKnown80016E18 = true;
    out->pollSwGp700Before80016E18 = before.gp700;
    out->pollSwGp700AfterKnown80016E18 = true;
    out->pollSwGp700After80016E18 = gp700After;
    out->pollSwTimedOutKnown80016E18 = true;
    out->pollSwTimedOut80016E18 = timedOut;

    if (before.dword800917E8 == 3) {
        out->stateAfter =
            ExpectedState3AfterTypedPoll80016E18(before, effectivePoll);
        return true;
    }

    if (effectivePoll == 0) {
        return true;
    }
    if (effectivePoll == 1) {
        out->stateAfter.dword800917F0 = 1;
        out->stateAfter.dword800917E8 =
            before.dword800917F4 == 1 ? 4 : 2;
        return true;
    }
    if (effectivePoll == 3) {
        out->stateAfter.dword800917F0 = 3;
        out->stateAfter.dword800917E8 = 4;
        out->stateAfter.dword800917F4 = 0;
        return true;
    }
    if (effectivePoll == 4) {
        out->stateAfter.dword800917F0 = 4;
        out->stateAfter.dword800917E8 = 2;
        out->stateAfter.dword800917F4 = 0;
        return true;
    }
    out->stateAfter.dword800917F0 = -3;
    out->stateAfter.dword800917E8 = 4;
    out->stateAfter.dword800917F4 = 0;
    return true;
}

bool BuildSaveUiCardIoPollFactsFromNaturalEvent80016E18(
    const PrStage1SaveUi19148LowerFeedbackRequest& request,
    const CardNaturalSwCardEventInput80016E18& input,
    CardIoHostFacts80017594* out) {
    if (!out) {
        return false;
    }
    *out = {};
    if (!input.sourceKnown ||
        (input.source !=
             CardNaturalEventIngressSource::DeviceTestEventProvider &&
         input.source !=
             CardNaturalEventIngressSource::
                 TranslatedDirectCardEventBroker) ||
        !input.gp700BeforeKnown ||
        request.cardIoState.gp700 != input.gp700Before) {
        return false;
    }

    // IDA 80016E18 tests all four SwCARD handles in order.  A later hit
    // overrides an earlier hit, and gp+700 < 0 overrides every event with 2.
    int32_t pollResult80016E18 = 0;
    for (int32_t i = 0; i < 4; ++i) {
        if (!input.testEventResultKnown[i]) {
            return false;
        }
        if (input.testEventResults[i] == 1) {
            pollResult80016E18 = i + 1;
        }
    }
    if (!BuildSaveUiCardIoPollFactsFromResult80016E18(
            request, pollResult80016E18, out)) {
        return false;
    }
    out->pollSwGp700Before80016E18 = input.gp700Before;
    out->pollSwGp700After80016E18 = input.gp700Before - 1;
    out->naturalSwCardEventSourceKnown80016E18 = true;
    return true;
}

bool ApplySaveUiCardIoEvent4ResetProviderFacts80047EE4(
    const CardBiosResetProviderFacts80047EE4& provider,
    CardIoHostFacts80017594* ioFacts) {
    if (!ioFacts || !provider.sourceKnown ||
        !ioFacts->factsKnown || !ioFacts->stateBeforeKnown ||
        ioFacts->stateBefore.dword800917E8 != 1 ||
        !ioFacts->stateAfterKnown ||
        ioFacts->pollSwResult80016E18 != 4) {
        return false;
    }
    ioFacts->drainHwEventsKnown8001707C =
        provider.drainHwEventsKnown8001707C;
    ioFacts->resetHwEventsKnown80047EE4 = provider.sourceKnown;
    ioFacts->resetHwNewCardKnown80047EE4 = provider.newCardKnown80047EE4;
    ioFacts->resetHwCardWriteArgsKnown80047EE4 =
        provider.cardWriteArgsKnown80047EE4;
    ioFacts->resetHwCardWriteArg0_80047EE4 =
        provider.cardWriteArg0_80047EE4;
    ioFacts->resetHwCardWriteArg1_80047EE4 =
        provider.cardWriteArg1_80047EE4;
    ioFacts->resetHwCardWriteArg2_80047EE4 =
        provider.cardWriteArg2_80047EE4;
    ioFacts->resetHwCardWriteResultKnown80047EE4 =
        provider.cardWriteResultKnown80047EE4;
    ioFacts->resetHwCardWriteResult80047EE4 =
        provider.cardWriteResult80047EE4;
    ioFacts->pollHwKnown80017008 = provider.pollHwKnown80017008;
    ioFacts->pollHwResult80017008 = provider.pollHwResult80017008;
    return AreSaveUiCardIoEvent4ResetFactsComplete80047EE4(*ioFacts);
}

bool AreSaveUiCardIoEvent4ResetFactsComplete80047EE4(
    const CardIoHostFacts80017594& input) {
    if (!input.factsKnown || !input.stateBeforeKnown ||
        input.stateBefore.dword800917E8 != 1 ||
        !input.stateAfterKnown || input.stateAfter.dword800917E8 != 2 ||
        input.stateAfter.dword800917F0 != 4 ||
        !input.pollSwKnown80016E18 || input.pollSwResult80016E18 != 4 ||
        !input.drainHwEventsKnown8001707C ||
        !input.resetHwEventsKnown80047EE4 ||
        !input.resetHwNewCardKnown80047EE4 ||
        !input.resetHwCardWriteArgsKnown80047EE4 ||
        input.resetHwCardWriteArg0_80047EE4 != 0 ||
        input.resetHwCardWriteArg1_80047EE4 != 63 ||
        input.resetHwCardWriteArg2_80047EE4 != 0 ||
        !input.resetHwCardWriteResultKnown80047EE4 ||
        !input.pollHwKnown80017008 || input.pollHwResult80017008 < 0 ||
        input.pollHwResult80017008 > 4) {
        return false;
    }
    return true;
}

bool ComputeNaturalHwCardPollResult80017008(
    const CardNaturalHwCardEventInput80017008& input,
    int32_t* outPollResult80017008) {
    if (!outPollResult80017008) {
        return false;
    }
    *outPollResult80017008 = 0;
    if (!input.sourceKnown ||
        (input.source !=
             CardNaturalEventIngressSource::DeviceTestEventProvider &&
         input.source !=
             CardNaturalEventIngressSource::
                 TranslatedDirectCardEventBroker)) {
        return false;
    }

    // IDA 80017008 loops until the first HwCARD TestEvent hit, returning its
    // 1-based handle index.  No hit is not a successful poll result.
    for (int32_t i = 0; i < 4; ++i) {
        if (!input.testEventResultKnown[i]) {
            return false;
        }
        if (input.testEventResults[i] == 1) {
            *outPollResult80017008 = i + 1;
            return true;
        }
    }
    return false;
}

void ResetTranslatedCardEventBroker800170C4() {
    s_translatedCardEventBroker800170C4 = {};
    s_translatedCardEventBroker800170C4.initialized = true;
}

CardCommunicationSetupState80017524
ExecuteCardCommunicationSetup80017524(
    const PrPsxPadDirect::PadInitState800354C0& padInit800354C0) {
    CardCommunicationSetupState80017524 out{};
    out.sourceKnown = true;
    // 80017524 begins with ResetCallback and then calls 800354C0(0).  The
    // translated PAD seam records the PSX writes without claiming ownership
    // of the Win input device or its host VBlank interrupt.
    out.resetCallbackCalled = true;
    out.padInit800354C0 = padInit800354C0;
    out.padInitCalled800354C0 =
        padInit800354C0.sourceKnown && padInit800354C0.accepted &&
        padInit800354C0.inputArg == 0 &&
        padInit800354C0.modeGlobalValue == 0 &&
        padInit800354C0.statusGlobalValue == -1 &&
        padInit800354C0.resetCallbackCalled &&
        padInit800354C0.padInit2Called &&
        padInit800354C0.padInit2Protocol ==
            PrPsxPadDirect::kPadInit2Protocol800354C0 &&
        padInit800354C0.padInit2StatusAddress ==
            PrPsxPadDirect::kPadStatusGlobal800882F0 &&
        padInit800354C0.changeClearPadCalled &&
        padInit800354C0.changeClearPadArg == 0 &&
        padInit800354C0.softwareStateCommitted &&
        !padInit800354C0.hardwarePadHalAuthority &&
        !padInit800354C0.hostProjection;
    if (!out.padInitCalled800354C0) {
        return out;
    }
    // 800170C4 has a fixed InitCARD2/StartCARD2/bu_init/ChangeClearPAD(0)
    // prefix, opens four SwCARD and four HwCARD events, then enables all 8.
    out.initCard2Called = true;
    out.startCard2Called = true;
    out.buInitCalled = true;
    out.changeClearPadCalled = true;
    out.changeClearPadArg = 0;
    out.softwareEventHandlesOpened = 4;
    out.hardwareEventHandlesOpened = 4;
    out.eventsEnabled = 8;
    ResetTranslatedCardEventBroker800170C4();
    out.cardGlobalsZeroed = true;
    out.dword800917E8 = 0;
    out.dword800917EC = 0;
    out.dword800917F0 = 0;
    out.dword800917F4 = 0;
    out.softwareStateCommitted =
        out.padInitCalled800354C0 && out.initCard2Called &&
        out.startCard2Called && out.buInitCalled &&
        out.changeClearPadCalled && out.cardGlobalsZeroed &&
        s_translatedCardEventBroker800170C4.initialized;
    s_cardCommunicationSetup80017524 = out;
    s_cardCommunicationTeardown80017574 = {};
    return out;
}

CardCommunicationTeardownState80017574
ExecuteCardCommunicationTeardown80017574() {
    CardCommunicationTeardownState80017574 out{};
    out.sourceKnown = true;
    out.setupWasActive =
        s_cardCommunicationSetup80017524.softwareStateCommitted;
    // 8001724C enters the critical section, closes the same four SwCARD and
    // four HwCARD handles, and exits.  The translated broker is then made
    // inactive until the next 80017524 setup; no physical device is closed.
    out.enterCriticalSectionCalled = out.setupWasActive;
    out.softwareEventHandlesClosed = out.setupWasActive ? 4u : 0u;
    out.hardwareEventHandlesClosed = out.setupWasActive ? 4u : 0u;
    out.exitCriticalSectionCalled = out.setupWasActive;
    s_translatedCardEventBroker800170C4 = {};
    out.softwareStateCommitted =
        out.setupWasActive &&
        s_translatedCardEventBroker800170C4.initialized == false;
    s_cardCommunicationTeardown80017574 = out;
    s_cardCommunicationSetup80017524 = {};
    return out;
}

CardCommunicationSetupState80017524
GetCardCommunicationSetupState80017524() {
    return s_cardCommunicationSetup80017524;
}

CardCommunicationTeardownState80017574
GetCardCommunicationTeardownState80017574() {
    return s_cardCommunicationTeardown80017574;
}

bool SignalTranslatedSwCardEvent80016E18(
    CardTranslatedEventSignalSource source,
    int32_t eventResult) {
    if (!s_translatedCardEventBroker800170C4.initialized ||
        (source != CardTranslatedEventSignalSource::CardInfo80017594 &&
         source != CardTranslatedEventSignalSource::CardLoad80017594 &&
         source != CardTranslatedEventSignalSource::FileRead800173A8 &&
         source != CardTranslatedEventSignalSource::PhysicalHotplug80017594) ||
        eventResult < 1 || eventResult > 4) {
        return false;
    }
    const int32_t index = eventResult - 1;
    if (s_translatedCardEventBroker800170C4.swPending[index]) {
        return false;
    }
    s_translatedCardEventBroker800170C4.swPending[index] = true;
    s_translatedCardEventBroker800170C4.swSource[index] = source;
    return true;
}

bool PollTranslatedSwCardEvents80016E18(
    int32_t gp700Before,
    CardNaturalSwCardEventInput80016E18* out) {
    if (!out) {
        return false;
    }
    *out = {};
    if (!s_translatedCardEventBroker800170C4.initialized) {
        return false;
    }
    out->sourceKnown = true;
    out->source =
        CardNaturalEventIngressSource::TranslatedDirectCardEventBroker;
    out->gp700BeforeKnown = true;
    out->gp700Before = gp700Before;
    for (int32_t i = 0; i < 4; ++i) {
        out->testEventResultKnown[i] = true;
        out->testEventResults[i] =
            s_translatedCardEventBroker800170C4.swPending[i] ? 1 : 0;
        s_translatedCardEventBroker800170C4.swPending[i] = false;
        s_translatedCardEventBroker800170C4.swSource[i] =
            CardTranslatedEventSignalSource::None;
    }
    return true;
}

void DrainTranslatedSwCardEvents80016FC0() {
    if (!s_translatedCardEventBroker800170C4.initialized) {
        return;
    }
    for (int32_t i = 0; i < 4; ++i) {
        s_translatedCardEventBroker800170C4.swPending[i] = false;
        s_translatedCardEventBroker800170C4.swSource[i] =
            CardTranslatedEventSignalSource::None;
    }
}

bool PollTranslatedReadEvents80016EB8(int32_t* result) {
    if (!result || !s_translatedCardEventBroker800170C4.initialized) return false;
    *result = 0;
    for (int32_t i = 0; i < 4; ++i) {
        if (!s_translatedCardEventBroker800170C4.swPending[i]) continue;
        s_translatedCardEventBroker800170C4.swPending[i] = false;
        s_translatedCardEventBroker800170C4.swSource[i] = CardTranslatedEventSignalSource::None;
        *result = i + 1;
        break;
    }
    return true;
}

bool SignalTranslatedHwCardEvent80017008(
    CardTranslatedEventSignalSource source,
    int32_t eventResult) {
    if (!s_translatedCardEventBroker800170C4.initialized ||
        (source != CardTranslatedEventSignalSource::Format80017B60 &&
         source != CardTranslatedEventSignalSource::ResetHwCard80047EE4) ||
        eventResult < 1 || eventResult > 4) {
        return false;
    }
    const int32_t index = eventResult - 1;
    if (s_translatedCardEventBroker800170C4.hwPending[index]) {
        return false;
    }
    s_translatedCardEventBroker800170C4.hwPending[index] = true;
    s_translatedCardEventBroker800170C4.hwSource[index] = source;
    return true;
}

bool PollTranslatedHwCardEvents80017008(
    CardNaturalHwCardEventInput80017008* out) {
    if (!out) {
        return false;
    }
    *out = {};
    if (!s_translatedCardEventBroker800170C4.initialized) {
        return false;
    }
    out->sourceKnown = true;
    out->source =
        CardNaturalEventIngressSource::TranslatedDirectCardEventBroker;
    for (int32_t i = 0; i < 4; ++i) {
        out->testEventResultKnown[i] = true;
        if (!s_translatedCardEventBroker800170C4.hwPending[i]) {
            out->testEventResults[i] = 0;
            continue;
        }
        out->testEventResults[i] = 1;
        s_translatedCardEventBroker800170C4.hwPending[i] = false;
        s_translatedCardEventBroker800170C4.hwSource[i] =
            CardTranslatedEventSignalSource::None;
        break;
    }
    return true;
}

void DrainTranslatedHwCardEvents8001707C() {
    if (!s_translatedCardEventBroker800170C4.initialized) {
        return;
    }
    for (int32_t i = 0; i < 4; ++i) {
        s_translatedCardEventBroker800170C4.hwPending[i] = false;
        s_translatedCardEventBroker800170C4.hwSource[i] =
            CardTranslatedEventSignalSource::None;
    }
}

CardTranslatedEventBrokerState800170C4
GetTranslatedCardEventBrokerState800170C4() {
    return s_translatedCardEventBroker800170C4;
}

bool PublishRuntimeSaveUiCardIoState3TypedPollCarrier80017594(
    const PrStage1SaveUi19148LowerFeedbackRequest& request,
    const CardIoHostFacts80017594& facts) {
    if (!IsImportableState3CardIoTypedPollFacts80017594(request, facts)) {
        s_saveUiCardIoState3TypedPollCarrier80017594 = {};
        return false;
    }

    CardIoLowerFeedbackBuildResult80017594 lower{};
    BuildSaveUiCardIoLowerFeedbackFromHostFacts80017594(facts, &lower);
    if (!lower.lowerFeedbackKnown ||
        !lower.lowerFeedback.cardIoFeedbackKnown80017594) {
        s_saveUiCardIoState3TypedPollCarrier80017594 = {};
        return false;
    }

    SaveUiCardIoState3TypedPollCarrier80017594 next{};
    next.known = true;
    next.producerWired80016E18_80017594 = true;
    next.typedPollResultKnown80016E18 = true;
    next.pollResult80016E18 = facts.pollSwResult80016E18;
    next.incomplete = false;
    next.hostFacts = facts;
    next.lower = lower;
    s_saveUiCardIoState3TypedPollCarrier80017594 = next;
    return true;
}

bool GetSaveUiCardIoState3TypedPollCarrier80017594(
    SaveUiCardIoState3TypedPollCarrier80017594* out) {
    if (!out) {
        return false;
    }
    *out = {};
    if (!s_saveUiCardIoState3TypedPollCarrier80017594.known) {
        return false;
    }
    *out = s_saveUiCardIoState3TypedPollCarrier80017594;
    return true;
}

void ClearSaveUiCardIoState3TypedPollCarrier80017594() {
    s_saveUiCardIoState3TypedPollCarrier80017594 = {};
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

void BuildSaveUiFormatFactsFromRuntimeProducerInput80017B60(
    const PrStage1SaveUi19148LowerFeedbackRequest& request,
    const SaveUiFormatRuntimeFacts80017B60& facts,
    SaveUiFormatRuntimeProducerResult80017B60* out) {
    if (!out) {
        return;
    }
    *out = {};
    out->requestUsed = true;
    out->requestMatched = IsExpectedFormatRequest80017B60(request);
    out->runtimeFactsKnown = facts.factsKnown;
    if (!out->requestMatched || !facts.factsKnown) {
        out->incomplete = true;
        return;
    }

    CardFormatHostFacts80017B60 hostFacts{};
    hostFacts.factsKnown = true;
    for (int32_t i = 0; i < kFormatAttemptCount80017B60; ++i) {
        const SaveUiFormatRuntimeAttemptFacts80017B60& src =
            facts.attempts[i];
        CardFormatHostAttemptFacts80017B60& dst = hostFacts.attempts[i];
        dst.drainHwEventsKnown8001707C = src.drainHwEventsKnown8001707C;
        dst.formatKnown = src.formatKnown;
        dst.formatArgsKnown = src.formatArgsKnown;
        dst.formatArg0 = src.formatArg0;
        dst.formatArg1 = src.formatArg1;
        dst.pollResultKnown80017008 = src.pollResultKnown80017008;
        dst.pollResult80017008 = src.pollResult80017008;
    }

    int32_t terminalResult = 0;
    bool terminalKnown = false;
    bool callCompleted = false;
    bool retryExhaustedReturnUnknown = false;
    for (int32_t i = 0; i < kFormatAttemptCount80017B60; ++i) {
        const CardFormatHostAttemptFacts80017B60& attempt =
            hostFacts.attempts[i];
        if (!attempt.drainHwEventsKnown8001707C || !attempt.formatKnown ||
            !attempt.formatArgsKnown ||
            attempt.formatArg0 != kFormatArg0_80017B60 ||
            attempt.formatArg1 != kFormatArg1_80017B60 ||
            !attempt.pollResultKnown80017008) {
            out->incomplete = true;
            return;
        }
        if (attempt.pollResult80017008 == 1 ||
            attempt.pollResult80017008 == 3) {
            terminalResult = attempt.pollResult80017008;
            terminalKnown = true;
            callCompleted = true;
            break;
        }
        if (i == kFormatAttemptCount80017B60 - 1) {
            callCompleted = true;
            retryExhaustedReturnUnknown = true;
            break;
        }
    }
    if (!callCompleted) {
        out->incomplete = true;
        return;
    }

    CardFormatLowerFeedbackBuildResult80017B60 lower{};
    BuildSaveUiFormatLowerFeedbackFromHostFacts80017B60(hostFacts, &lower);
    if (!lower.lowerFeedbackKnown ||
        !lower.lowerFeedback.formatFeedbackKnown80017B60) {
        out->incomplete = true;
        return;
    }

    out->produced = true;
    out->callCompleted = true;
    out->resultKnown = terminalKnown;
    out->retryExhaustedReturnUnknown = retryExhaustedReturnUnknown;
    out->result80017B60 = terminalResult;
    out->hostFacts = hostFacts;
    out->lower = lower;
}

bool PublishRuntimeSaveUiFormatTypedCarrier80017B60(
    const PrStage1SaveUi19148LowerFeedbackRequest& request,
    const SaveUiFormatRuntimeFacts80017B60& facts) {
    SaveUiFormatRuntimeProducerResult80017B60 producer{};
    BuildSaveUiFormatFactsFromRuntimeProducerInput80017B60(
        request,
        facts,
        &producer);
    if (!producer.produced || producer.incomplete ||
        !producer.requestMatched ||
        !producer.lower.lowerFeedback.formatFeedbackKnown80017B60) {
        s_saveUiFormatTypedCarrier80017B60 = {};
        return false;
    }

    SaveUiFormatTypedCarrier80017B60 next{};
    next.known = true;
    next.producerWired8001707C_80017008_80017B60 = true;
    next.formatCallCompleted80017B60 = producer.callCompleted;
    next.typedFormatResultKnown80017B60 = producer.resultKnown;
    next.retryExhaustedReturnUnknown80017B60 =
        producer.retryExhaustedReturnUnknown;
    next.result80017B60 = producer.result80017B60;
    next.incomplete = false;
    next.hostFacts = producer.hostFacts;
    next.lower = producer.lower;
    s_saveUiFormatTypedCarrier80017B60 = next;
    return true;
}

bool GetSaveUiFormatTypedCarrier80017B60(
    SaveUiFormatTypedCarrier80017B60* out) {
    if (!out) {
        return false;
    }
    *out = {};
    if (!s_saveUiFormatTypedCarrier80017B60.known) {
        return false;
    }
    *out = s_saveUiFormatTypedCarrier80017B60;
    return true;
}

void ClearSaveUiFormatTypedCarrier80017B60() {
    s_saveUiFormatTypedCarrier80017B60 = {};
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
    const bool sourceCanPublishCase17Completion =
        source ==
        CardReadTypedCarrierSource800179B4::RuntimeLowerCardProducer;
    const bool compactCountKnown =
        next.feedback.word8007ABE4Known &&
        next.feedback.word8007ABE4 >= 0 &&
        next.feedback.word8007ABE4 <= kReadAttemptCount800179B4;
    next.case17LoopCompletionKnown80019D7C =
        sourceCanPublishCase17Completion &&
        compactCountKnown &&
        next.hal.produced &&
        !next.incomplete;
    next.case17HiScorePayloadLaneKnown =
        next.typedReadSuccessKnown800179B4 &&
        next.payloadBytesKnown8007ADE8 &&
        next.case17LoopCompletionKnown80019D7C;
    next.producerWired800173A8_80016EB8_800179B4 =
        next.case17LoopCompletionKnown80019D7C;

    s_case17CardReadTypedCarrier800179B4 = next;
    return s_case17CardReadTypedCarrier800179B4
        .producerWired800173A8_80016EB8_800179B4;
}

bool BuildHiScoreCase6Directory80019D7C(
    const PrStage1SaveUiDirectoryRawBankView8007A318& raw,
    HiScoreCase6Directory80019D7C* out) {
    if (!out) return false;
    *out = {};
    if (!raw.known || !raw.bytes || raw.psxAddress != 0x8007A318u ||
        raw.byteCount != 600u || raw.byteSize != 600u) return false;
    for (int row = 0; row < kReadAttemptCount800179B4; ++row) {
        char name[32]{};
        std::memcpy(name, raw.bytes + row * 40u, 20u);
        if (!name[0] || !IsCase17GameSaveDirectoryTitle80019D7CCase6(name)) continue;
        std::memcpy(out->names[out->entryCount++].data(), name, sizeof(name));
    }
    out->known = true;
    return true;
}

static bool PumpHiScoreNamedReads800179B4(HiScoreNamedReadExecution800179B4& e) {
    using namespace PrSS0CardImageStorageDirect;
    while (e.row < e.directory.entryCount) {
        if (!e.rowOpened) {
            e.blocks[e.row].fill(0); // native memset8007ABE8 before EACH open
            e.pendingRead = e.reader(e.directory.names[e.row].data(),
                e.blocks[e.row].data(), e.blocks[e.row].size(), e.readerOwner);
            e.rowOpened = true;
            e.rowWaits = 0;
            const auto& r = e.pendingRead.receipt;
            if (r.fileFound && e.pendingRead.handle) {
                // Successful open:800173A8 drains old software events, then
                // read signals the translated completion. Failed open does
                // neither;179B4 still enters80016EB8 with the existing events.
                DrainTranslatedSwCardEvents80016FC0();
                if (!SignalTranslatedSwCardEvent80016E18(
                        CardTranslatedEventSignalSource::FileRead800173A8,
                        r.readComplete && r.bytesRead == kCardReadBlockBytes800179B4 ? 1 : 2)) {
                    e.failed = true;
                    return false;
                }
            }
        }
        int32_t poll = 2;
        // The 300th empty pass performs VSync(0), THEN returns2 without a
        // 301st TestEvent. Later pending events belong to the next operation.
        if (e.rowWaits < kCardPollLimit80016EB8 && !PollTranslatedReadEvents80016EB8(&poll)) {
            e.failed = true;
            return false;
        }
        if (poll == 0) {
            e.waitPending = true;
            return true;
        }
        e.lastPollResult = poll;
        Log::Printf("SS0 card named poll: row=%d name=%s result=%d waits=%d retainedHandle=%d beforeClose=1 translatedEventAuthority=1 psxFdAuthority=0",
            e.row, e.directory.names[e.row].data(), poll, e.rowWaits, e.pendingRead.handle ? 1 : 0);
        CloseNamedCardFileAfterPoll800179B4(e.pendingRead);
        auto& attempt = e.feedback.attempts[e.row];
        attempt.rowEnabled = true;
        attempt.rowNameKnown = attempt.rowNameBuffer8007CBE8Known = true;
        std::memcpy(attempt.rowName, e.directory.names[e.row].data(), sizeof(attempt.rowName));
        attempt.cardSelectorKnown = true;
        // Keep the existing platform payload lane: do not forge PSX BIOS
        // fd/event-handle facts. The SUCCESS VALUE now comes from executed
        // translated polling, not from a cached image or close's return.
        attempt.liveCase17PayloadViewKnown = true;
        attempt.successAuthorityKnown800179B4 = true;
        attempt.success800179B4 = poll == 1;
        attempt.targetBufferKnown = attempt.readLengthKnown = attempt.payloadPointerKnown = true;
        attempt.targetBufferAddress = kCardReadBlockBufferAddr800179B4;
        attempt.readLength = kCardReadBlockBytes800179B4;
        attempt.payloadPointer = kCardReadPayloadAddr8007ADE8;
        attempt.payloadPassedTo800164F8 = attempt.blockBytesKnown = poll == 1;
        attempt.blockBytes = poll == 1 ? e.blocks[e.row].data() : nullptr;
        attempt.blockByteCount = poll == 1 ? kCardReadBlockBytes800179B4 : 0;
        if (e.consumeRow && !e.consumeRow(e.row, true, poll,
                e.blocks[e.row].data(), e.blocks[e.row].size(), e.consumerOwner)) {
            e.failed = true;
            return false;
        }
        ++e.row;
        e.rowOpened = false;
    }
    // Native nonempty Case17 scans all15 rows, clearing even disabled rows.
    if (e.directory.entryCount != 0) {
        for (int32_t row = e.row; row < kReadAttemptCount800179B4; ++row) {
            e.blocks[row].fill(0);
            if (e.consumeRow && !e.consumeRow(row, false, 0,
                    e.blocks[row].data(), e.blocks[row].size(), e.consumerOwner)) {
                e.failed = true;
                return false;
            }
        }
    }
    e.complete = PublishCase17CardReadTypedCarrier800179B4(
        e.feedback, CardReadTypedCarrierSource800179B4::RuntimeLowerCardProducer);
    e.failed = !e.complete;
    return e.complete;
}

bool BeginHiScoreNamedReads800179B4(HiScoreNamedReadExecution800179B4& e,
    const HiScoreCase6Directory80019D7C& directory,
    PrSS0CardImageStorageDirect::DeferredNamedCardReader800173A8 reader, void* owner,
    HiScoreNamedReadExecution800179B4::RowConsumer consumeRow, void* consumerOwner) {
    if (e.started || !reader || !directory.known || directory.entryCount < 0 ||
        directory.entryCount > kReadAttemptCount800179B4 ||
        !s_translatedCardEventBroker800170C4.initialized) return false;
    for (int32_t row = 0; row < directory.entryCount; ++row) {
        const auto& name = directory.names[row];
        if (!name[0] || std::find(name.begin(), name.end(), '\0') == name.end()) return false;
    }
    ClearCase17CardReadTypedCarrier800179B4();
    e.started = true;
    e.directory = directory;
    e.reader = reader;
    e.readerOwner = owner;
    e.consumeRow = consumeRow;
    e.consumerOwner = consumerOwner;
    e.feedback.feedbackKnown = e.feedback.word8007ABE4Known = true;
    e.feedback.word8007ABE4 = directory.entryCount;
    for (auto& attempt : e.feedback.attempts) attempt.rowEnabledKnown = true;
    return PumpHiScoreNamedReads800179B4(e);
}

bool ResumeHiScoreNamedReadsAfterVSync80016EB8(HiScoreNamedReadExecution800179B4& e) {
    if (!e.started || e.failed || e.complete || !e.waitPending) return false;
    e.waitPending = false;
    ++e.rowWaits;
    ++e.totalWaits;
    return PumpHiScoreNamedReads800179B4(e);
}

bool PublishRuntimeCase17FromCase6Directory80019D7C(
    const PrStage1SaveUiCardImagePersistenceView8007A318& view,
    const HiScoreCase6Directory80019D7C& directory,
    PrSS0CardImageStorageDirect::NamedCardBlockReader800173A8 reader,
    void* readerOwner) {
    if (!view.known ||
        !view.slotPolicyKnown ||
        !directory.known || directory.entryCount < 0 ||
        directory.entryCount > kReadAttemptCount800179B4 ||
        view.bytes == nullptr ||
        view.byteCount != kDirectCardImageBytes8007A318 ||
        view.byteSize != kDirectCardImageBytes8007A318) {
        s_case17CardReadTypedCarrier800179B4 = {};
        return false;
    }

    const CardReadFeedbackRequest800179B4 request =
        MakeCase17HiScoreReadRequest800179B4(17);
    if (!IsExpectedCase17HiScoreReadRequest800179B4(request)) {
        s_case17CardReadTypedCarrier800179B4 = {};
        return false;
    }

    CardReadFeedback800179B4 feedback{};
    feedback.feedbackKnown = true;
    feedback.word8007ABE4Known = true;
    for (int32_t i = 0; i < kReadAttemptCount800179B4; ++i) {
        feedback.attempts[i].rowEnabledKnown = true;
        feedback.attempts[i].rowEnabled = false;
    }

    using ReadBlocks = std::array<std::array<uint8_t, kCardReadBlockBytes800179B4>,
                                  kReadAttemptCount800179B4>;
    auto readBlocks = reader ? std::make_unique<ReadBlocks>() : nullptr;
    // Production opens each prior Case6 name independently. Retain the pure
    // image-view adapter for explicit value tests/compatibility only. Values
    // must own all fifteen buffers until the carrier has copied them.
    for (int32_t row = 0; row < directory.entryCount; ++row) {
        const auto& title = directory.names[row];
        if (!title[0] || std::find(title.begin(), title.end(), '\0') == title.end()) {
            s_case17CardReadTypedCarrier800179B4 = {};
            return false;
        }
        int32_t physicalBlockIndex = -1;
        for (int32_t physical = 0; !reader && physical < kReadAttemptCount800179B4; ++physical) {
            char candidate[32]{};
            if (CopyDirectCardImageDirectoryTitle800179B4(view, physical, candidate) &&
                std::strcmp(candidate, title.data()) == 0) {
                physicalBlockIndex = physical;
                break;
            }
        }
        PrSS0CardImageStorageDirect::NamedCardBlockRead800173A8 read{};
        if (reader) {
            read = reader(title.data(), (*readBlocks)[row].data(),
                          (*readBlocks)[row].size(), readerOwner);
            physicalBlockIndex = read.blockIndex;
        }
        const bool readSucceeded = reader
            ? read.requestValid && read.imageOpened && read.directoryValid &&
                read.fileFound && read.readComplete && read.bytesRead == kCardReadBlockBytes800179B4 &&
                read.closeAttempted
            : physicalBlockIndex >= 0;
        const std::size_t blockOffset =
            static_cast<std::size_t>(physicalBlockIndex + 1) *
            kDirectCardImageBlockBytes8007A318;
        if (readSucceeded && blockOffset + kDirectCardImageBlockBytes8007A318 >
            view.byteCount) {
            s_case17CardReadTypedCarrier800179B4 = {};
            return false;
        }

        CardReadAttemptFeedback800179B4& attempt =
            feedback.attempts[row];
        attempt.rowEnabled = true;
        attempt.rowNameKnown = true;
        std::memcpy(attempt.rowName, title.data(), sizeof(attempt.rowName));
        attempt.rowNameBuffer8007CBE8Known = true;
        attempt.cardSelectorKnown = true;
        attempt.liveCase17PayloadViewKnown = true;
        attempt.successAuthorityKnown800179B4 = true;
        attempt.success800179B4 = readSucceeded;
        attempt.targetBufferKnown = true;
        attempt.targetBufferAddress = kCardReadBlockBufferAddr800179B4;
        attempt.readLengthKnown = true;
        attempt.readLength = kCardReadBlockBytes800179B4;
        attempt.payloadPointerKnown = true;
        attempt.payloadPointer = kCardReadPayloadAddr8007ADE8;
        attempt.payloadPassedTo800164F8 = readSucceeded;
        attempt.blockBytesKnown = readSucceeded;
        attempt.blockBytes = readSucceeded
            ? (reader ? (*readBlocks)[row].data() : view.bytes + blockOffset) : nullptr;
        attempt.blockByteCount = readSucceeded ? kDirectCardImageBlockBytes8007A318 : 0;
    }
    feedback.word8007ABE4 = directory.entryCount;

    if (!PublishCase17CardReadTypedCarrier800179B4(
            feedback,
            CardReadTypedCarrierSource800179B4::RuntimeLowerCardProducer)) {
        s_case17CardReadTypedCarrier800179B4 = {};
        return false;
    }
    return true;
}

bool PublishRuntimeCase17CardReadTypedCarrier800179B4FromDirectCardImagePersistenceSink(
    const PrStage1SaveUiCardImagePersistenceView8007A318& view,
    int32_t selectedBlockIndex) {
    // Compatibility adapter for explicit callers. Production captures Case6
    // earlier and calls PublishRuntimeCase17FromCase6Directory80019D7C directly.
    if (!view.known || !view.slotPolicyKnown || !view.bytes ||
        view.byteCount != kDirectCardImageBytes8007A318 ||
        view.byteSize != kDirectCardImageBytes8007A318 ||
        view.blockIndex != selectedBlockIndex || selectedBlockIndex < 0 ||
        selectedBlockIndex >= kReadAttemptCount800179B4) {
        s_case17CardReadTypedCarrier800179B4 = {};
        return false;
    }
    std::array<uint8_t, 600> bytes{};
    for (int row = 0; row < kReadAttemptCount800179B4; ++row) {
        char name[32]{};
        if (CopyDirectCardImageDirectoryTitle800179B4(view, row, name))
            std::memcpy(bytes.data() + row * 40u, name, 20u);
    }
    PrStage1SaveUiDirectoryRawBankView8007A318 raw{};
    raw.known = true;
    raw.bytes = bytes.data();
    raw.byteCount = bytes.size();
    HiScoreCase6Directory80019D7C directory{};
    return BuildHiScoreCase6Directory80019D7C(raw, &directory) &&
        PublishRuntimeCase17FromCase6Directory80019D7C(view, directory);
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
