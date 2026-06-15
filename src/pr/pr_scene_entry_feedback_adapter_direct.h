#pragma once

#include "pr_scene_entry_direct.h"
#include "pr_stage1_save_ui_direct.h"

#include <cstddef>
#include <cstdint>

namespace PrSceneEntryFeedbackAdapterDirect {

enum class FeedbackAdapterGap80019414 : uint8_t {
    None = 0,
    MissingStatusPrefix80092F10,
    MissingStatusBank80092F1D,
    UnexpectedStatusPrefix80092F10,
    MissingCase17Result80019D7C,
    MissingCase17Gp72080019D7C,
    MissingCase17Bank80019D7C,
    MissingInputMemory80019284,
    MissingRecordTable80019284,
};

struct FeedbackAdapterResult80019414 {
    FeedbackAdapterResult80019414() = default;
    FeedbackAdapterResult80019414(
        const FeedbackAdapterResult80019414& other);
    FeedbackAdapterResult80019414& operator=(
        const FeedbackAdapterResult80019414& other);

    bool completed = false;
    bool gap = false;
    FeedbackAdapterGap80019414 gapReason =
        FeedbackAdapterGap80019414::None;

    PrSceneEntryDirect::Call80019284InputMemory80015788 inputMemory{};
    PrSceneEntryDirect::Call80019284Result80015788 call80019284{};
    PrSceneEntryDirect::Call80019414Feedback80015788 feedback{};

    uint32_t tablePsxAddress = 0;
    size_t tableByteCount = 0;
    uint8_t tableStorage[PrSceneEntryDirect::kHiScoreTableSize80019284]{};
};

const char* FeedbackAdapterGapName80019414(
    FeedbackAdapterGap80019414 gap);

void BuildFeedback80019414FromStatusPrefixAndCase17Bank(
    const PrStage1SaveStatusPrefix80092F10& statusPrefix80092F10,
    const PrSceneEntryDirect::Case17Result80019D7C& case17,
    FeedbackAdapterResult80019414* out);

}  // namespace PrSceneEntryFeedbackAdapterDirect
