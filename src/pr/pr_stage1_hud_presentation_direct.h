#pragma once

#include "pr_stage1_scene1_movie1_direct.h"
#include "pr_psx_sprite_template_render.h"

#include <array>
#include <cstdint>

struct PrGameContext;

struct PrStage1HudPresentationDirectTemplates {
    uint16_t rightRankDimClutY = 0x01E4;
    PrPsxSpriteTemplateRender::PsxSpriteTemplate scoreLabel{};
    PrPsxSpriteTemplateRender::PsxSpriteTemplate scoreDigit{};
    PrPsxSpriteTemplateRender::PsxSpriteTemplate transitionPromptRandom{};
    PrPsxSpriteTemplateRender::PsxSpriteTemplate transitionPromptManual{};
    PrPsxSpriteTemplateRender::PsxSpriteTemplate rightRankHighlight{};
    std::array<PrPsxSpriteTemplateRender::PsxSpriteTemplate, 4> rightRankRows{};
    PrPsxSpriteTemplateRender::PsxSpriteTemplate shortPromptLeft{};
    PrPsxSpriteTemplateRender::PsxSpriteTemplate shortPromptRight{};
    PrPsxSpriteTemplateRender::PsxSpriteTemplate topLessonLabel{};
    std::array<PrPsxSpriteTemplateRender::PsxSpriteTemplate, 6> topLessonNumbers{};
};

struct PrStage1HudPresentationDirectState {
    int32_t scoreDisplayValue = 0;
    uint8_t rightRankActiveRow = 0;
    uint8_t rightRankBlinkTargetRow = 0;
    bool rightRankBlinkEnabled = false;
    bool rightRankFlag0200Pulse = false;
    uint16_t rightRankTransitionAnim18E = 0;
    bool topLessonPairChangeVisible = false;
    uint8_t topLessonPairLessonId = 0;
    uint16_t transitionState916D0 = 0;
    bool highLayoutMode = true;
    float scorePanelOffsetX = 0.0f;
    float rightRankPanelOffsetX = 0.0f;
};

enum class PrStage1HudPresentationDirectBee4ActionKind : uint8_t {
    None = 0,
    TopLessonLabel,
    TopLessonNumber,
    ShortPromptLeft,
    ShortPromptRight,
    ScoreLabel,
    ScoreDigit,
    RightRankHighlight,
    RightRankRow,
    TransitionPromptRandom,
    TransitionPromptManual,
};

enum class PrStage1HudPresentationDirectProducerGapKind : uint8_t {
    None = 0,
    Bee4TemplateDesc,
    Bee4FastSpriteStackRgb,
    B590TemplateDesc,
    B590FastSpriteStackRgb,
    CarrierFull,
};

struct PrStage1HudPresentationDirectBee4ActionMetadata {
    PrStage1HudPresentationDirectBee4ActionKind kind =
        PrStage1HudPresentationDirectBee4ActionKind::None;
    const char* label = nullptr;
    uint32_t templateAddr = 0;
    int16_t screenX = 0;
    int16_t screenY = 0;
    int32_t enabled = 0;
    uint8_t rowIndex = 0;
    bool dimClut = false;
    bool templateDescResolved = false;
    bool fastSpriteRgbKnown = false;
};

struct PrStage1HudPresentationDirectProducerGap {
    PrStage1HudPresentationDirectProducerGapKind kind =
        PrStage1HudPresentationDirectProducerGapKind::None;
    const char* label = nullptr;
    uint32_t templateAddr = 0;
    int16_t screenX = 0;
    int16_t screenY = 0;
    uint8_t index = 0;
    uint8_t value = 0;
};

static constexpr uint32_t kPrStage1HudPresentationDirectMaxBee4Actions = 16;
static constexpr uint32_t kPrStage1HudPresentationDirectMaxProducerGaps = 16;

struct PrStage1HudPresentationDirectBee4ActionCarrier {
    PrStage1Scene1Movie1Direct::Movie1PsxRawDrawAction
        actions[kPrStage1HudPresentationDirectMaxBee4Actions]{};
    PrStage1HudPresentationDirectBee4ActionMetadata
        metadata[kPrStage1HudPresentationDirectMaxBee4Actions]{};
    uint32_t actionCount = 0;
    PrStage1HudPresentationDirectProducerGap
        gaps[kPrStage1HudPresentationDirectMaxProducerGaps]{};
    uint32_t gapCount = 0;
    uint32_t droppedActionCount = 0;
    uint32_t droppedGapCount = 0;
    uint32_t sourceFunction = 0x8001DF24u;
};

struct PrStage1HudPresentationDirectBee4ConsumeResult {
    bool valid = false;
    bool applied = false;
    bool runtimeOwnerProvided = false;
    uint32_t sourceFunction = 0x8001DF24u;
    uint32_t actionCount = 0;
    uint32_t consumedCount = 0;
    uint32_t submittedCount = 0;
    uint32_t partialCount = 0;
    uint32_t skippedCount = 0;
    uint32_t gapCount = 0;
    uint32_t droppedActionCount = 0;
    uint32_t droppedGapCount = 0;
    PrStage1Scene1Movie1Direct::Movie1RawDrawTypedSubmitResult
        submits[kPrStage1HudPresentationDirectMaxBee4Actions]{};
    PrPsxFastSpriteSubmitDirect::RuntimeState8003FA20 runtimeBefore{};
    PrPsxFastSpriteSubmitDirect::RuntimeState8003FA20 runtimeAfter{};
};

void PrStage1HudPresentationDirectResetSub8001DF24FrameCounter();

PrStage1HudPresentationDirectBee4ActionCarrier
PrStage1HudPresentationDirectBuildSub8001DF24Bee4RawActions(
    const PrStage1HudPresentationDirectState& state);

PrStage1HudPresentationDirectBee4ConsumeResult
PrStage1HudPresentationDirectConsumeSub8001DF24Bee4RawActions(
    const PrStage1HudPresentationDirectBee4ActionCarrier& carrier,
    PrPsxFastSpriteSubmitDirect::RuntimeState8003FA20& runtime,
    bool apply);

void PrStage1HudPresentationDirectDrawSub8001DB9C(
    PrGameContext& ctx,
    float vx,
    float vy,
    float vs,
    const PrStage1HudPresentationDirectTemplates& templates,
    bool visible,
    uint8_t lessonId);

void PrStage1HudPresentationDirectDrawSub8001DF24(
    PrGameContext& ctx,
    float vx,
    float vy,
    float vs,
    const PrStage1HudPresentationDirectTemplates& templates,
    const PrStage1HudPresentationDirectState& state);

void PrStage1HudPresentationDirectDrawNumericStatus(
    PrGameContext& ctx,
    float vx,
    float vy,
    float vs,
    const PrStage1HudPresentationDirectTemplates& templates,
    const PrStage1HudPresentationDirectState& state);
