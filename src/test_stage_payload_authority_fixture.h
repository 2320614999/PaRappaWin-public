#pragma once

#include "pr/pr_stage1_save_card_hal_direct.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>

namespace TestStagePayloadAuthorityFixture {

inline bool MintRuntimeLowerCardPayloadAuthority800164B4(
    const uint8_t* payload,
    std::size_t payloadBytes,
    PrStagePayloadBankDirect::LoadSavePayloadAuthority800164B4* out) {
    using namespace PrStage1SaveCardHalDirect;
    if (!out || !payload ||
        payloadBytes != PrStagePayloadBankDirect::kByteCount80092F10) {
        return false;
    }
    *out = {};

    std::array<uint8_t, kCardReadBlockBytes800179B4> block{};
    std::fill_n(block.begin(), kCardReadPayloadOffset8007ADE8, 0xA5u);
    std::copy_n(payload,
                payloadBytes,
                block.begin() + kCardReadPayloadOffset8007ADE8);

    CardReadFeedback800179B4 feedback{};
    feedback.feedbackKnown = true;
    feedback.word8007ABE4Known = true;
    feedback.word8007ABE4 = 1;
    for (int32_t i = 0; i < kReadAttemptCount800179B4; ++i) {
        feedback.attempts[i].rowEnabledKnown = true;
        feedback.attempts[i].rowEnabled = false;
    }

    CardReadAttemptFeedback800179B4& attempt = feedback.attempts[0];
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
    attempt.clearEvents.called = true;
    attempt.clearEvents.eventHandlesKnown = true;
    for (int32_t i = 0; i < 4; ++i) {
        attempt.clearEvents.eventHandles[i] = static_cast<uint32_t>(i + 1);
        attempt.clearEvents.testEventResultsKnown[i] = true;
        attempt.clearEvents.testEventResults[i] = 0;
    }
    attempt.readSubmission.called = true;
    attempt.readSubmission.fdKnown = true;
    attempt.readSubmission.fd = 5;
    attempt.readSubmission.bufferAddressKnown = true;
    attempt.readSubmission.bufferAddress = kCardReadBlockBufferAddr800179B4;
    attempt.readSubmission.byteCountKnown = true;
    attempt.readSubmission.byteCount = kCardReadBlockBytes800179B4;
    attempt.poll.called = true;
    attempt.poll.eventHandlesKnown = true;
    for (int32_t i = 0; i < 4; ++i) {
        attempt.poll.eventHandles[i] = static_cast<uint32_t>(0x100 + i);
    }
    attempt.poll.resultKnown = true;
    attempt.poll.psxReturn = 1;
    attempt.poll.timedOutKnown = true;
    attempt.poll.timedOut = false;
    attempt.poll.hitEventIndexKnown = true;
    attempt.poll.hitEventIndex = 0;
    attempt.poll.pollIterationCountKnown = true;
    attempt.poll.pollIterationCount = 1;
    attempt.poll.waitCallCountKnown80035560 = true;
    attempt.poll.waitCallCount80035560 = 0;
    attempt.closeKnown800179B4 = true;
    attempt.closeFdKnown800179B4 = true;
    attempt.closeFd800179B4 = 5;
    attempt.blockBytesKnown = true;
    attempt.blockBytes = block.data();
    attempt.blockByteCount = block.size();

    ClearState16CardReadTypedCarrier800179B4();
    const CardReadFeedbackRequest800179B4 request =
        MakeState16LoadPayloadReadRequest800179B4(16);
    if (!PublishRuntimeState16CardReadTypedCarrier800179B4ForBlock(
            request, feedback, 0)) {
        ClearState16CardReadTypedCarrier800179B4();
        return false;
    }
    State16CardReadTypedCarrier800179B4 carrier{};
    const uint8_t* const payloadView =
        block.data() + kCardReadPayloadOffset8007ADE8;
    const bool ok = GetState16CardReadTypedCarrier800179B4(&carrier) &&
                    carrier.producerWired800173A8_80016EB8_800179B4 &&
                    carrier.payloadAuthority800164B4.Matches(
                        PrStagePayloadBankDirect::
                            kTypedPayloadSourceAddress8007ADE8,
                        payloadView,
                        payloadBytes);
    if (ok) {
        *out = carrier.payloadAuthority800164B4;
    }
    ClearState16CardReadTypedCarrier800179B4();
    return ok;
}

} // namespace TestStagePayloadAuthorityFixture
