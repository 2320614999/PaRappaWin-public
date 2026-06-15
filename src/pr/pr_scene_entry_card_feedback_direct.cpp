#include "pr_scene_entry_card_feedback_direct.h"

#include <cstring>

namespace PrSceneEntryCardFeedbackDirect {

namespace {

uint32_t ReadU32LE(const uint8_t* p) {
    return static_cast<uint32_t>(p[0]) |
           (static_cast<uint32_t>(p[1]) << 8) |
           (static_cast<uint32_t>(p[2]) << 16) |
           (static_cast<uint32_t>(p[3]) << 24);
}

bool HasSuccessfulEvent80016EB8(const CardReadAttempt800179B4& row) {
    return row.eventResultKnown80016EB8 && row.eventResult80016EB8 == 1;
}

bool HasLiveCase17PayloadAuthority800179B4(
    const CardReadAttempt800179B4& row) {
    return row.liveCase17PayloadViewKnown &&
           row.successAuthorityKnown800179B4 &&
           row.success800179B4 &&
           row.targetBufferKnown &&
           row.targetBufferAddress == kCardBlockBufferAddr8007ABE8 &&
           row.readLengthKnown &&
           row.readLength == kCardReadBlockBytes800179B4 &&
           row.payloadPointerKnown &&
           row.payloadPointer == kCardSavePayloadAddr8007ADE8 &&
           row.payloadPassedTo800164F8 &&
           row.blockBytesKnown;
}

CardReadGap800179B4 BuildRowGap(const CardReadAttempt800179B4& row) {
    CardReadGap800179B4 gap{};
    gap.missingRowStatus8007A590 = !row.rowEnabledKnown;
    if (!row.rowEnabledKnown || !row.rowEnabled) {
        return gap;
    }

    gap.missingLiveCase17PayloadView = !row.liveCase17PayloadViewKnown;
    if (HasLiveCase17PayloadAuthority800179B4(row)) {
        return gap;
    }

    gap.missingRowName8007CBE8 =
        !row.rowNameKnown && !row.rowNameBuffer8007CBE8Known;
    gap.missingRowNameBuffer8007CBE8 = !row.rowNameBuffer8007CBE8Known;
    gap.missingCardSelectorGp128Gp124 = !row.cardSelectorKnown;
    gap.missingPathBuild800173A8 = !row.pathBuilt800173A8;
    gap.missingOpenAttempt800173A8 = !row.openAttempted800173A8;
    gap.missingOpenFd800173A8 =
        !row.openFailed800173A8 &&
        (!row.fdKnown800173A8 || row.fd800173A8 < 0);
    gap.missingTargetBuffer8007ABE8 =
        !row.targetBufferKnown ||
        row.targetBufferAddress != kCardBlockBufferAddr8007ABE8;
    gap.missingReadLength =
        !row.readLengthKnown ||
        row.readLength != kCardReadBlockBytes800179B4;
    gap.missingPayloadPointer =
        !row.payloadPointerKnown ||
        row.payloadPointer != kCardSavePayloadAddr8007ADE8;
    gap.missingPayloadPassedTo800164F8 = !row.payloadPassedTo800164F8;
    gap.missingBlockCount = !row.blockCountKnown || row.blockCount != 1;
    gap.missingByteCount =
        !row.byteCountKnown ||
        row.byteCount != kCardReadBlockBytes800179B4;
    gap.missingClearSwEvents80016FC0 = !row.clearSwEventsKnown80016FC0;
    gap.missingReadSubmission =
        !row.openFailed800173A8 &&
        (!row.readSubmittedKnown || !row.readSubmitted);
    gap.missingEventResult80016EB8 = !row.eventResultKnown80016EB8;
    gap.missingClose =
        !row.closeKnown ||
        !row.closeFdKnown ||
        (!row.openFailed800173A8 && row.closeFd != row.fd800173A8);
    gap.missingBlockBytes =
        (HasSuccessfulEvent80016EB8(row) ||
         (row.liveCase17PayloadViewKnown &&
          row.successAuthorityKnown800179B4 &&
          row.success800179B4)) &&
        !row.blockBytesKnown;
    return gap;
}

bool HasAnyGap(const CardReadGap800179B4& gap) {
    return gap.missingRowStatus8007A590 ||
           gap.missingWord8007ABE4Status ||
           gap.missingLiveCase17PayloadView ||
           gap.missingRowName8007CBE8 ||
           gap.missingRowNameBuffer8007CBE8 ||
           gap.missingCardSelectorGp128Gp124 ||
           gap.missingPathBuild800173A8 ||
           gap.missingOpenAttempt800173A8 ||
           gap.missingOpenFd800173A8 ||
           gap.missingTargetBuffer8007ABE8 ||
           gap.missingReadLength ||
           gap.missingPayloadPointer ||
           gap.missingPayloadPassedTo800164F8 ||
           gap.missingBlockCount ||
           gap.missingByteCount ||
           gap.missingClearSwEvents80016FC0 ||
           gap.missingReadSubmission ||
           gap.missingEventResult80016EB8 ||
           gap.missingClose ||
           gap.missingBlockBytes;
}

bool CanExposePayloadToCase17(const CardReadAttempt800179B4& row,
                              const CardReadGap800179B4& gap) {
    return HasLiveCase17PayloadAuthority800179B4(row) &&
           !gap.missingRowStatus8007A590 &&
           !gap.missingWord8007ABE4Status &&
           !gap.missingLiveCase17PayloadView &&
           !gap.missingTargetBuffer8007ABE8 &&
           !gap.missingReadLength &&
           !gap.missingPayloadPointer &&
           !gap.missingPayloadPassedTo800164F8;
}

bool HasProducedFailure800179B4(const CardReadAttempt800179B4& row,
                                const CardReadGap800179B4& gap) {
    return !HasAnyGap(gap) &&
           row.eventResultKnown80016EB8 &&
           row.eventResult80016EB8 != 1;
}

}  // namespace

void BuildCase17Feedback80019D7C(
    const Case17CardReadHalFeedback80019D7C& input,
    Case17CardFeedbackBuildResult80019D7C* out) {
    if (!out) {
        return;
    }
    *out = {};
    out->feedbackKnown = input.word8007ABE4Known;
    out->feedback.word8007ABE4Known = input.word8007ABE4Known;
    out->feedback.word8007ABE4 = input.word8007ABE4;
    if (!input.word8007ABE4Known) {
        out->anyMissingRequiredFact = true;
        for (CardReadGap800179B4& gap : out->rowGaps) {
            gap.missingWord8007ABE4Status = true;
        }
        return;
    }
    if (input.word8007ABE4 <= 0) {
        out->feedbackKnown = true;
        return;
    }

    for (size_t i = 0; i < PrSceneEntryDirect::kHiScoreCardRowCount80019D7C;
         ++i) {
        const CardReadAttempt800179B4& src = input.rows[i];
        PrSceneEntryDirect::Case17CardRow80019D7C& dst =
            out->feedback.cardRows[i];
        CardReadGap800179B4& gap = out->rowGaps[i];
        gap = BuildRowGap(src);
        dst.rowEnabledKnown = src.rowEnabledKnown;
        dst.rowEnabled = src.rowEnabled;
        dst.rowNameBuffer8007CBE8Known = src.rowNameBuffer8007CBE8Known;
        dst.liveCase17PayloadViewKnown = src.liveCase17PayloadViewKnown;
        dst.successAuthorityKnown800179B4 =
            src.successAuthorityKnown800179B4;
        dst.success800179B4 = src.success800179B4;
        dst.readBufferKnown = src.targetBufferKnown;
        dst.readBuffer = src.targetBufferAddress;
        dst.readLengthKnown = src.readLengthKnown;
        dst.readLength = src.readLength;
        dst.payloadPointerKnown = src.payloadPointerKnown;
        dst.payloadPointer = src.payloadPointer;
        dst.payloadPassedTo800164F8 = src.payloadPassedTo800164F8;
        if (HasAnyGap(gap)) {
            out->anyMissingRequiredFact = true;
        }
        if (!src.rowEnabledKnown || !src.rowEnabled) {
            continue;
        }

        if (HasProducedFailure800179B4(src, gap)) {
            dst.eventResult80016EB8Known = true;
            dst.eventResult80016EB8 = src.eventResult80016EB8;
            dst.readResultKnown = true;
            dst.readSucceeded = false;
            continue;
        }

        const bool exposePayload = CanExposePayloadToCase17(src, gap);
        dst.eventResult80016EB8Known = exposePayload;
        dst.eventResult80016EB8 = exposePayload ? 1 : 0;
        dst.readResultKnown = exposePayload;
        dst.readSucceeded = exposePayload;
        if (!exposePayload) {
            continue;
        }
        std::memcpy(out->blockStorage[i].data(),
                    src.blockBytes,
                    kCardReadBlockBytes800179B4);
        const uint8_t* stableBlock = out->blockStorage[i].data();
        dst.rowMetadata8007AE14Known = true;
        dst.rowMetadata8007AE14 =
            ReadU32LE(stableBlock + kCardRowMetadataOffset8007AE14);
        dst.readBufferKnown = true;
        dst.readBuffer = kCardBlockBufferAddr8007ABE8;
        dst.readLengthKnown = true;
        dst.readLength = kCardReadBlockBytes800179B4;
        dst.payloadPointerKnown = true;
        dst.payloadPointer = kCardSavePayloadAddr8007ADE8;
        dst.payloadPassedTo800164F8 = true;
        dst.payload = stableBlock + kCardSavePayloadOffset8007ADE8;
        dst.payloadSize =
            kCardReadBlockBytes800179B4 - kCardSavePayloadOffset8007ADE8;
        dst.eventResult80016EB8Known = true;
        dst.eventResult80016EB8 = 1;
        dst.readResultKnown = true;
        dst.readSucceeded = true;
    }
}

void BuildCase17CardReadHalFeedbackFromSaveCardHal800179B4(
    const PrStage1SaveCardHalDirect::CardReadFeedback800179B4& input,
    const PrStage1SaveCardHalDirect::CardReadHalBuildResult800179B4& result,
    Case17CardReadHalFeedback80019D7C* out) {
    if (!out) {
        return;
    }
    *out = {};
    if (!input.feedbackKnown || !result.produced || result.incomplete) {
        return;
    }

    out->word8007ABE4Known = input.word8007ABE4Known;
    out->word8007ABE4 = input.word8007ABE4;
    if (input.word8007ABE4 <= 0) {
        return;
    }

    for (size_t i = 0;
         i < PrSceneEntryDirect::kHiScoreCardRowCount80019D7C &&
         i < static_cast<size_t>(
                 PrStage1SaveCardHalDirect::kReadAttemptCount800179B4);
         ++i) {
        const PrStage1SaveCardHalDirect::CardReadAttemptFeedback800179B4&
            src = input.attempts[i];
        const PrStage1SaveCardHalDirect::CardReadAttemptResult800179B4&
            auth = result.attempts[i];
        CardReadAttempt800179B4& dst = out->rows[i];
        dst.rowEnabledKnown = src.rowEnabledKnown;
        dst.rowEnabled = src.rowEnabled;
        dst.rowNameKnown = src.rowNameKnown;
        if (src.rowNameKnown) {
            std::memcpy(dst.rowName, src.rowName, sizeof(dst.rowName));
        }
        dst.cardSelectorKnown = src.cardSelectorKnown;
        dst.cardSelectorGp128 = src.cardSelectorGp128;
        dst.cardSelectorGp124 = src.cardSelectorGp124;
        dst.rowNameBuffer8007CBE8Known = src.rowNameBuffer8007CBE8Known;
        const bool rowSuccessAuthority =
            src.successAuthorityKnown800179B4 && src.success800179B4;
        const bool rowPayloadAuthority =
            rowSuccessAuthority &&
            auth.produced &&
            !auth.incomplete &&
            auth.readSucceeded &&
            auth.payloadBytesAvailable &&
            auth.blockBytes != nullptr &&
            auth.blockByteCount >= kCardReadBlockBytes800179B4;
        dst.liveCase17PayloadViewKnown = src.liveCase17PayloadViewKnown;
        dst.successAuthorityKnown800179B4 =
            src.successAuthorityKnown800179B4;
        dst.success800179B4 = src.success800179B4;
        dst.pathBuilt800173A8 = src.pathBuilt800173A8;
        dst.openAttempted800173A8 = src.openAttempted800173A8;
        dst.openFailed800173A8 = auth.openFailed800173A8;
        dst.fdKnown800173A8 = src.fdKnown800173A8;
        dst.fd800173A8 = src.fd800173A8;
        dst.targetBufferKnown = src.targetBufferKnown;
        dst.targetBufferAddress = src.targetBufferAddress;
        dst.readLengthKnown = src.readLengthKnown;
        dst.readLength = src.readLength;
        dst.payloadPointerKnown = src.payloadPointerKnown;
        dst.payloadPointer = src.payloadPointer;
        dst.payloadPassedTo800164F8 = src.payloadPassedTo800164F8;
        dst.blockCountKnown = src.blockCountKnown;
        dst.blockCount = src.blockCount;
        dst.byteCountKnown = src.readSubmission.byteCountKnown;
        dst.byteCount = src.readSubmission.byteCount;
        dst.clearSwEventsKnown80016FC0 =
            src.clearEvents.called && src.clearEvents.eventHandlesKnown;
        dst.readSubmittedKnown = auth.produced && !auth.incomplete;
        dst.readSubmitted = auth.readSubmitted800173A8;
        dst.eventResultKnown80016EB8 = auth.produced && !auth.incomplete;
        dst.eventResult80016EB8 = auth.eventResult80016EB8;
        dst.closeKnown = src.closeKnown800179B4;
        dst.closeFdKnown = src.closeFdKnown800179B4;
        dst.closeFd = src.closeFd800179B4;
        dst.blockBytesKnown = rowPayloadAuthority;
        if (dst.blockBytesKnown) {
            std::memcpy(dst.blockBytes,
                        auth.blockBytes,
                        kCardReadBlockBytes800179B4);
        }
    }
}

void BuildFeedback80019414FromCase17CardReadFacts(
    const PrStage1SaveStatusPrefix80092F10& statusPrefix80092F10,
    bool arg2Known,
    int32_t arg2,
    const Case17CardReadHalFeedback80019D7C& input,
    Case17To19414FeedbackBuildResult80015788* out) {
    if (!out) {
        return;
    }
    *out = {};
    BuildCase17Feedback80019D7C(input, &out->case17Feedback);
    out->missingHalFacts800179B4 =
        out->case17Feedback.anyMissingRequiredFact;
    out->case17 =
        PrSceneEntryDirect::PsxCall80019D7C_Case17HiScoreBankCarrier(
            arg2,
            arg2Known,
            out->case17Feedback.feedback);
    PrSceneEntryFeedbackAdapterDirect::
        BuildFeedback80019414FromStatusPrefixAndCase17Bank(
            statusPrefix80092F10,
            out->case17,
            &out->adapter);
    out->completed =
        !out->missingHalFacts800179B4 && out->adapter.completed;
    out->gap = out->missingHalFacts800179B4 || out->adapter.gap;
}

}  // namespace PrSceneEntryCardFeedbackDirect
