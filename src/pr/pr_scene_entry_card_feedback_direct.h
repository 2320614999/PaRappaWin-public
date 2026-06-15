#pragma once

#include "pr_scene_entry_direct.h"
#include "pr_scene_entry_feedback_adapter_direct.h"
#include "pr_stage1_save_card_hal_direct.h"
#include "pr_stage1_save_ui_direct.h"

#include <array>
#include <cstddef>
#include <cstdint>

namespace PrSceneEntryCardFeedbackDirect {

static constexpr uint32_t kFn800179B4 = 0x800179B4u;
static constexpr uint32_t kFn800173A8 = 0x800173A8u;
static constexpr uint32_t kFn80016FC0 = 0x80016FC0u;
static constexpr uint32_t kFn80016EB8 = 0x80016EB8u;
static constexpr uint32_t kFnRead = 0xFFFFFFFFu;
static constexpr uint32_t kFnClose = 0xFFFFFFFEu;
static constexpr uint32_t kCardBlockBufferAddr8007ABE8 = 0x8007ABE8u;
static constexpr uint32_t kCardSavePayloadAddr8007ADE8 = 0x8007ADE8u;
static constexpr size_t kCardReadBlockBytes800179B4 = 0x2000u;
static constexpr size_t kCardSavePayloadOffset8007ADE8 = 0x200u;
static constexpr size_t kCardRowMetadataOffset8007AE14 = 0x22Cu;

struct CardReadGap800179B4 {
    bool missingRowStatus8007A590 = false;
    bool missingWord8007ABE4Status = false;
    bool missingLiveCase17PayloadView = false;
    bool missingRowName8007CBE8 = false;
    bool missingRowNameBuffer8007CBE8 = false;
    bool missingCardSelectorGp128Gp124 = false;
    bool missingPathBuild800173A8 = false;
    bool missingOpenAttempt800173A8 = false;
    bool missingOpenFd800173A8 = false;
    bool missingTargetBuffer8007ABE8 = false;
    bool missingReadLength = false;
    bool missingPayloadPointer = false;
    bool missingPayloadPassedTo800164F8 = false;
    bool missingBlockCount = false;
    bool missingByteCount = false;
    bool missingClearSwEvents80016FC0 = false;
    bool missingReadSubmission = false;
    bool missingEventResult80016EB8 = false;
    bool missingClose = false;
    bool missingBlockBytes = false;
};

struct CardReadAttempt800179B4 {
    uint32_t function800179B4 = kFn800179B4;
    uint32_t function800173A8 = kFn800173A8;
    uint32_t function80016FC0 = kFn80016FC0;
    uint32_t function80016EB8 = kFn80016EB8;
    uint32_t functionRead = kFnRead;
    uint32_t functionClose = kFnClose;
    bool rowEnabledKnown = false;
    bool rowEnabled = false;
    bool rowNameKnown = false;
    char rowName[32]{};
    bool rowNameBuffer8007CBE8Known = false;
    bool cardSelectorKnown = false;
    int32_t cardSelectorGp128 = 0;
    int32_t cardSelectorGp124 = 0;
    bool liveCase17PayloadViewKnown = false;
    bool successAuthorityKnown800179B4 = false;
    bool success800179B4 = false;
    bool pathBuilt800173A8 = false;
    bool openAttempted800173A8 = false;
    bool openFailed800173A8 = false;
    bool fdKnown800173A8 = false;
    int32_t fd800173A8 = -1;
    bool targetBufferKnown = false;
    uint32_t targetBufferAddress = kCardBlockBufferAddr8007ABE8;
    bool readLengthKnown = false;
    size_t readLength = kCardReadBlockBytes800179B4;
    bool payloadPointerKnown = false;
    uint32_t payloadPointer = kCardSavePayloadAddr8007ADE8;
    bool payloadPassedTo800164F8 = false;
    bool blockCountKnown = false;
    int32_t blockCount = 1;
    bool byteCountKnown = false;
    size_t byteCount = kCardReadBlockBytes800179B4;
    bool clearSwEventsKnown80016FC0 = false;
    bool readSubmittedKnown = false;
    bool readSubmitted = false;
    bool eventResultKnown80016EB8 = false;
    int32_t eventResult80016EB8 = 0;
    bool closeKnown = false;
    bool closeFdKnown = false;
    int32_t closeFd = -1;
    bool blockBytesKnown = false;
    uint8_t blockBytes[kCardReadBlockBytes800179B4]{};
};

struct Case17CardReadHalFeedback80019D7C {
    bool word8007ABE4Known = false;
    int32_t word8007ABE4 = 0;
    std::array<CardReadAttempt800179B4,
               PrSceneEntryDirect::kHiScoreCardRowCount80019D7C>
        rows{};
};

struct Case17CardFeedbackBuildResult80019D7C {
    bool feedbackKnown = false;
    bool anyMissingRequiredFact = false;
    std::array<CardReadGap800179B4,
               PrSceneEntryDirect::kHiScoreCardRowCount80019D7C>
        rowGaps{};
    std::array<std::array<uint8_t, kCardReadBlockBytes800179B4>,
               PrSceneEntryDirect::kHiScoreCardRowCount80019D7C>
        blockStorage{};
    PrSceneEntryDirect::Case17Feedback80019D7C feedback{};
};

struct Case17To19414FeedbackBuildResult80015788 {
    bool completed = false;
    bool gap = false;
    bool missingHalFacts800179B4 = false;
    Case17CardFeedbackBuildResult80019D7C case17Feedback{};
    PrSceneEntryDirect::Case17Result80019D7C case17{};
    PrSceneEntryFeedbackAdapterDirect::FeedbackAdapterResult80019414
        adapter{};
};

void BuildCase17Feedback80019D7C(
    const Case17CardReadHalFeedback80019D7C& input,
    Case17CardFeedbackBuildResult80019D7C* out);

void BuildCase17CardReadHalFeedbackFromSaveCardHal800179B4(
    const PrStage1SaveCardHalDirect::CardReadFeedback800179B4& input,
    const PrStage1SaveCardHalDirect::CardReadHalBuildResult800179B4& result,
    Case17CardReadHalFeedback80019D7C* out);

void BuildFeedback80019414FromCase17CardReadFacts(
    const PrStage1SaveStatusPrefix80092F10& statusPrefix80092F10,
    bool arg2Known,
    int32_t arg2,
    const Case17CardReadHalFeedback80019D7C& input,
    Case17To19414FeedbackBuildResult80015788* out);

}  // namespace PrSceneEntryCardFeedbackDirect
