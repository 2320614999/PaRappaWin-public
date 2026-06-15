#include "pr_stage1_hud_presentation_direct.h"

#include "pr_game_context.h"
#include "pr_psx_fast_sprite_submit_direct.h"

#include <algorithm>
#include <cstdio>

namespace {

using PrPsxSpriteTemplateRender::DrawPsxSpriteTemplate;
using PrPsxSpriteTemplateRender::DrawPsxSpriteTemplateSubrect;
using PrPsxSpriteTemplateRender::PsxSpriteTemplate;

static uint32_t s_sub8001DF24FrameCounter = 0;
static int32_t s_sub8001DE08Lifetime = 0;
static int32_t s_sub8001DE08SideFlag = 0;

static constexpr uint32_t kSub8001DF24 = 0x8001DF24u;
static constexpr uint32_t kSub8001DB9C = 0x8001DB9Cu;
static constexpr uint32_t kStage1HudTplTransitionPromptRandom = 0x8004E6B0u;
static constexpr uint32_t kStage1HudTplTransitionPromptManual = 0x8004E6C0u;
static constexpr uint32_t kStage1HudTplRightRankHighlight = 0x8004E6D0u;
static constexpr uint32_t kStage1HudTplRightRankRow0 = 0x8004E6E0u;
static constexpr uint32_t kStage1HudTplRightRankRow1 = 0x8004E6F0u;
static constexpr uint32_t kStage1HudTplRightRankRow2 = 0x8004E700u;
static constexpr uint32_t kStage1HudTplRightRankRow3 = 0x8004E710u;
static constexpr uint32_t kStage1HudTplShortPromptLeft = 0x8004E720u;
static constexpr uint32_t kStage1HudTplShortPromptRight = 0x8004E730u;
static constexpr uint32_t kStage1HudTplScoreLabel = 0x8004E740u;
static constexpr uint32_t kStage1HudTplScoreDigit = 0x8004E750u;
static constexpr uint32_t kStage1HudTplTopLessonLabel = 0x8004E760u;
static constexpr std::array<uint32_t, 6> kStage1HudTopLessonNumberTplAddrs = {{
    0x8004E770u,
    0x8004E780u,
    0x8004E790u,
    0x8004E7A0u,
    0x8004E7B0u,
    0x8004E7C0u,
}};
static constexpr std::array<uint32_t, 4> kStage1HudRightRankRowTplAddrs = {{
    kStage1HudTplRightRankRow0,
    kStage1HudTplRightRankRow1,
    kStage1HudTplRightRankRow2,
    kStage1HudTplRightRankRow3,
}};

static constexpr std::array<float, 4> kStage1HudRightRankX = {{
    256.0f,
    255.0f,
    260.0f,
    251.0f,
}};
static constexpr std::array<float, 4> kStage1HudRightRankY = {{
    137.0f,
    150.0f,
    163.0f,
    176.0f,
}};

static bool Stage1RightRankRowUsesDimClut(const PrStage1HudPresentationDirectState& state,
                                          size_t rowIndex) {
    const uint8_t activeRow =
        (std::min<uint8_t>)(state.rightRankActiveRow, 3u);
    const uint8_t blinkTarget =
        (std::min<uint8_t>)(state.rightRankBlinkTargetRow, 3u);

    bool dim = true;
    if (rowIndex == activeRow) {
        dim = false;
    }
    if (state.rightRankBlinkEnabled && rowIndex == blinkTarget) {
        dim = (s_sub8001DF24FrameCounter & 1u) == 0u;
    }
    return dim;
}

static void DrawStage1ScoreDigitsSub8001B590(
    PrGameContext& ctx,
    float vx,
    float vy,
    float vs,
    const PrStage1HudPresentationDirectTemplates& templates,
    int32_t scoreDisplayValue,
    float x,
    float y) {
    char scoreDigits[32] = {};
    std::snprintf(scoreDigits, sizeof(scoreDigits), "%d", scoreDisplayValue);
    for (size_t i = 0; scoreDigits[i] != '\0'; i++) {
        const char ch = scoreDigits[i];
        if (ch < '0' || ch > '9') {
            continue;
        }

        const int digit = ch - '0';
        (void)DrawPsxSpriteTemplateSubrect(ctx,
                                           vx,
                                           vy,
                                           vs,
                                           x + 9.0f * (float)i,
                                           y,
                                           templates.scoreDigit,
                                           digit * (int)templates.scoreDigit.w,
                                           (int)templates.scoreDigit.w,
                                           0,
                                           1.0f,
                                           1.0f,
                                           1.0f,
                                           1.0f,
                                           930);
    }
}

static void Stage1ResolveSub8001DE08SideFlag(uint16_t transitionAnim18E) {
    switch (transitionAnim18E) {
    case 1:
    case 2:
    case 6:
        s_sub8001DE08SideFlag = 0;
        break;
    case 3:
    case 4:
    case 5:
        s_sub8001DE08SideFlag = 1;
        break;
    default:
        break;
    }
}

static void PrStage1HudPresentationDirectDrawSub8001DE08(
    PrGameContext& ctx,
    float vx,
    float vy,
    float vs,
    const PrStage1HudPresentationDirectTemplates& templates,
    bool resetTimer,
    float layoutOffset) {
    const bool advanceCadence = !ctx.renderOnlyFrame;
    if (resetTimer && advanceCadence) {
        s_sub8001DE08Lifetime = 20;
    }

    const int32_t lifetime = s_sub8001DE08Lifetime;
    if (lifetime <= 0) {
        return;
    }

    const bool multipleOf5 = (lifetime % 5) == 0;
    const bool rightSide = s_sub8001DE08SideFlag != 0;
    const PsxSpriteTemplate& tpl =
        rightSide ? templates.shortPromptRight : templates.shortPromptLeft;
    const float y =
        layoutOffset +
        (rightSide
             ? (multipleOf5 ? 144.0f : 150.0f)
             : (multipleOf5 ? 152.0f : 146.0f));

    (void)DrawPsxSpriteTemplate(ctx,
                                vx,
                                vy,
                                vs,
                                20.0f,
                                y,
                                tpl,
                                1.0f,
                                1.0f,
                                1.0f,
                                1.0f,
                                930);
    if (advanceCadence) {
        --s_sub8001DE08Lifetime;
    }
}

static void DrawStage1TransitionPromptSub8001DF24(
    PrGameContext& ctx,
    float vx,
    float vy,
    float vs,
    const PrStage1HudPresentationDirectTemplates& templates,
    uint16_t transitionState916D0) {
    if (transitionState916D0 == 2u) {
        (void)DrawPsxSpriteTemplate(ctx,
                                    vx,
                                    vy,
                                    vs,
                                    20.0f,
                                    53.0f,
                                    templates.transitionPromptManual,
                                    1.0f,
                                    1.0f,
                                    1.0f,
                                    1.0f,
                                    930);
    } else if (transitionState916D0 == 1u) {
        (void)DrawPsxSpriteTemplate(ctx,
                                    vx,
                                    vy,
                                    vs,
                                    28.0f,
                                    53.0f,
                                    templates.transitionPromptRandom,
                                    1.0f,
                                    1.0f,
                                    1.0f,
                                    1.0f,
                                    930);
    }
}

static int16_t Stage1HudToPsxScreenCoord(float value) {
    return static_cast<int16_t>(value);
}

static PrPsxFastSpriteSubmitDirect::FastSpriteStackScratchTail8003FA20
BuildStage1HudBee4RgbGapStackTail() {
    return PrPsxFastSpriteSubmitDirect::BuildStackScratchTail8001DF24To8001BEE4(
        0u,
        false,
        "Stage1 HUD 8001DF24 BEE4 producer: RGB stack tail is not owned "
        "by 8001BEE4/8001BE34");
}

static void AppendStage1HudProducerGap(
    PrStage1HudPresentationDirectBee4ActionCarrier& out,
    const PrStage1HudPresentationDirectProducerGap& gap) {
    if (out.gapCount >= kPrStage1HudPresentationDirectMaxProducerGaps) {
        ++out.droppedGapCount;
        return;
    }
    out.gaps[out.gapCount++] = gap;
}

static void AppendStage1HudBee4Action(
    PrStage1HudPresentationDirectBee4ActionCarrier& out,
    PrStage1HudPresentationDirectBee4ActionKind kind,
    const char* label,
    uint32_t templateAddr,
    float x,
    float y,
    int32_t enabled,
    uint8_t rowIndex = 0,
    bool dimClut = false) {
    if (out.actionCount >= kPrStage1HudPresentationDirectMaxBee4Actions) {
        ++out.droppedActionCount;
        AppendStage1HudProducerGap(
            out,
            {PrStage1HudPresentationDirectProducerGapKind::CarrierFull,
             label,
             templateAddr,
             Stage1HudToPsxScreenCoord(x),
             Stage1HudToPsxScreenCoord(y),
             rowIndex,
             0u});
        return;
    }

    const uint32_t index = out.actionCount;
    const int16_t screenX = Stage1HudToPsxScreenCoord(x);
    const int16_t screenY = Stage1HudToPsxScreenCoord(y);
    PrStage1Scene1Movie1Direct::Movie1PsxRawDrawAction action =
        PrStage1Scene1Movie1Direct::BuildRawDrawSubmitActionSub8001BEE4(
            kSub8001DF24,
            screenX,
            screenY,
            templateAddr,
            enabled,
            0u,
            index,
            0u,
            BuildStage1HudBee4RgbGapStackTail());
    action.psxCallOrder = index;
    action.resolutionStatus =
        action.be34.valid
            ? PrStage1Scene1Movie1Direct::Movie1PsxRawDrawResolutionStatus::
                  GapRawDrawFastSpriteStackRgb
            : PrStage1Scene1Movie1Direct::Movie1PsxRawDrawResolutionStatus::
                  GapTemplateDesc;

    out.actions[index] = action;
    PrStage1HudPresentationDirectBee4ActionMetadata& metadata =
        out.metadata[index];
    metadata.kind = kind;
    metadata.label = label;
    metadata.templateAddr = templateAddr;
    metadata.screenX = screenX;
    metadata.screenY = screenY;
    metadata.enabled = enabled;
    metadata.rowIndex = rowIndex;
    metadata.dimClut = dimClut;
    metadata.templateDescResolved = action.be34.valid;
    metadata.fastSpriteRgbKnown =
        action.bee4.gsSortFastSprite.localSpriteRgbKnown;
    ++out.actionCount;

    if (!metadata.templateDescResolved) {
        AppendStage1HudProducerGap(
            out,
            {PrStage1HudPresentationDirectProducerGapKind::Bee4TemplateDesc,
             label,
             templateAddr,
             screenX,
             screenY,
             rowIndex,
             0u});
    }
    if (!metadata.fastSpriteRgbKnown) {
        AppendStage1HudProducerGap(
            out,
            {PrStage1HudPresentationDirectProducerGapKind::
                 Bee4FastSpriteStackRgb,
             label,
             templateAddr,
             screenX,
             screenY,
             rowIndex,
             0u});
    }
}

static void AppendStage1HudB590Action(
    PrStage1HudPresentationDirectBee4ActionCarrier& out,
    PrStage1HudPresentationDirectBee4ActionKind kind,
    const char* label,
    uint32_t templateAddr,
    float x,
    float y,
    uint16_t glyphIndex,
    uint8_t rowIndex = 0,
    uint32_t sourceFunction = kSub8001DF24,
    uint16_t priority = 3u) {
    if (out.actionCount >= kPrStage1HudPresentationDirectMaxBee4Actions) {
        ++out.droppedActionCount;
        AppendStage1HudProducerGap(
            out,
            {PrStage1HudPresentationDirectProducerGapKind::CarrierFull,
             label,
             templateAddr,
             Stage1HudToPsxScreenCoord(x),
             Stage1HudToPsxScreenCoord(y),
             rowIndex,
             static_cast<uint8_t>(glyphIndex)});
        return;
    }

    const uint32_t index = out.actionCount;
    const int16_t screenX = Stage1HudToPsxScreenCoord(x);
    const int16_t screenY = Stage1HudToPsxScreenCoord(y);
    PrStage1Scene1Movie1Direct::Movie1PsxRawDrawAction action =
        PrStage1Scene1Movie1Direct::BuildRawDrawSubmitActionSub8001B590(
            sourceFunction,
            screenX,
            screenY,
            templateAddr,
            glyphIndex,
            false,
            priority,
            0u,
            index,
            0u);
    action.psxCallOrder = index;
    action.resolutionStatus =
        action.b25c.valid
            ? PrStage1Scene1Movie1Direct::Movie1PsxRawDrawResolutionStatus::
                  GapRawDrawFastSpriteStackRgb
            : PrStage1Scene1Movie1Direct::Movie1PsxRawDrawResolutionStatus::
                  GapTemplateDesc;

    out.actions[index] = action;
    PrStage1HudPresentationDirectBee4ActionMetadata& metadata =
        out.metadata[index];
    metadata.kind = kind;
    metadata.label = label;
    metadata.templateAddr = templateAddr;
    metadata.screenX = screenX;
    metadata.screenY = screenY;
    metadata.enabled = 0;
    metadata.rowIndex = rowIndex;
    metadata.dimClut = false;
    metadata.templateDescResolved = action.b25c.valid;
    metadata.fastSpriteRgbKnown =
        action.b590.gsSortFastSprite.localSpriteRgbKnown;
    ++out.actionCount;

    if (!metadata.templateDescResolved) {
        AppendStage1HudProducerGap(
            out,
            {PrStage1HudPresentationDirectProducerGapKind::B590TemplateDesc,
             label,
             templateAddr,
             screenX,
             screenY,
             rowIndex,
             static_cast<uint8_t>(glyphIndex)});
    }
    if (!metadata.fastSpriteRgbKnown) {
        AppendStage1HudProducerGap(
            out,
            {PrStage1HudPresentationDirectProducerGapKind::
                 B590FastSpriteStackRgb,
             label,
             templateAddr,
             screenX,
             screenY,
             rowIndex,
             static_cast<uint8_t>(glyphIndex)});
    }
}

static void AppendStage1HudScoreDigitB590Actions(
    PrStage1HudPresentationDirectBee4ActionCarrier& out,
    int32_t scoreDisplayValue,
    float x,
    float y) {
    char scoreDigits[32] = {};
    std::snprintf(scoreDigits, sizeof(scoreDigits), "%d", scoreDisplayValue);
    for (size_t i = 0; scoreDigits[i] != '\0'; i++) {
        const char ch = scoreDigits[i];
        if (ch < '0' || ch > '9') {
            continue;
        }
        AppendStage1HudB590Action(
            out,
            PrStage1HudPresentationDirectBee4ActionKind::ScoreDigit,
            "scoreDigit",
            kStage1HudTplScoreDigit,
            x + 9.0f * (float)i,
            y,
            static_cast<uint16_t>(ch - '0'),
            static_cast<uint8_t>(i));
    }
}

static void AppendStage1HudTopLessonPairB590Actions(
    PrStage1HudPresentationDirectBee4ActionCarrier& out,
    const PrStage1HudPresentationDirectState& state) {
    if (!state.topLessonPairChangeVisible ||
        state.topLessonPairLessonId < 1u ||
        state.topLessonPairLessonId > kStage1HudTopLessonNumberTplAddrs.size()) {
        return;
    }

    AppendStage1HudB590Action(
        out,
        PrStage1HudPresentationDirectBee4ActionKind::TopLessonLabel,
        "topLessonLabel",
        kStage1HudTplTopLessonLabel,
        101.0f,
        107.0f,
        0u,
        0u,
        kSub8001DB9C,
        2u);
    AppendStage1HudB590Action(
        out,
        PrStage1HudPresentationDirectBee4ActionKind::TopLessonNumber,
        "topLessonNumber",
        kStage1HudTopLessonNumberTplAddrs[
            static_cast<size_t>(state.topLessonPairLessonId - 1u)],
        207.0f,
        107.0f,
        0u,
        state.topLessonPairLessonId,
        kSub8001DB9C,
        2u);
}

} // namespace

void PrStage1HudPresentationDirectResetSub8001DF24FrameCounter() {
    s_sub8001DF24FrameCounter = 0;
    s_sub8001DE08Lifetime = 0;
    s_sub8001DE08SideFlag = 0;
}

PrStage1HudPresentationDirectBee4ActionCarrier
PrStage1HudPresentationDirectBuildSub8001DF24Bee4RawActions(
    const PrStage1HudPresentationDirectState& state) {
    PrStage1HudPresentationDirectBee4ActionCarrier out{};

    AppendStage1HudTopLessonPairB590Actions(out, state);

    const float yOffset = state.highLayoutMode ? 0.0f : 30.0f;
    const uint8_t activeRow =
        (std::min<uint8_t>)(state.rightRankActiveRow, 3u);

    bool shortPromptVisible = s_sub8001DE08Lifetime > 0;
    int32_t promptLifetime = s_sub8001DE08Lifetime;
    int32_t promptSideFlag = s_sub8001DE08SideFlag;
    if (state.rightRankFlag0200Pulse) {
        switch (state.rightRankTransitionAnim18E) {
        case 1:
        case 2:
        case 6:
            promptSideFlag = 0;
            break;
        case 3:
        case 4:
        case 5:
            promptSideFlag = 1;
            break;
        default:
            break;
        }
        shortPromptVisible = true;
        promptLifetime = 20;
    }
    if (shortPromptVisible && promptLifetime > 0) {
        const bool multipleOf5 = (promptLifetime % 5) == 0;
        const bool rightSide = promptSideFlag != 0;
        AppendStage1HudBee4Action(
            out,
            rightSide
                ? PrStage1HudPresentationDirectBee4ActionKind::ShortPromptRight
                : PrStage1HudPresentationDirectBee4ActionKind::ShortPromptLeft,
            rightSide ? "shortPromptRight" : "shortPromptLeft",
            rightSide ? kStage1HudTplShortPromptRight
                      : kStage1HudTplShortPromptLeft,
            20.0f,
            yOffset +
                (rightSide
                     ? (multipleOf5 ? 144.0f : 150.0f)
                     : (multipleOf5 ? 152.0f : 146.0f)),
            0);
    }

    const float scoreOffsetX = state.scorePanelOffsetX;
    const float rightRankOffsetX = state.rightRankPanelOffsetX;

    AppendStage1HudBee4Action(
        out,
        PrStage1HudPresentationDirectBee4ActionKind::ScoreLabel,
        "scoreLabel",
        kStage1HudTplScoreLabel,
        20.0f + scoreOffsetX,
        176.0f + yOffset,
        0);
    AppendStage1HudScoreDigitB590Actions(
        out,
        state.scoreDisplayValue,
        58.0f + scoreOffsetX,
        176.0f + yOffset);

    AppendStage1HudBee4Action(
        out,
        PrStage1HudPresentationDirectBee4ActionKind::RightRankHighlight,
        "rightRankHighlight",
        kStage1HudTplRightRankHighlight,
        192.0f + rightRankOffsetX,
        136.0f + 13.0f * (float)activeRow + yOffset,
        0);

    for (size_t rowIndex = 0; rowIndex < kStage1HudRightRankRowTplAddrs.size();
         rowIndex++) {
        const bool dimClut = Stage1RightRankRowUsesDimClut(state, rowIndex);
        AppendStage1HudBee4Action(
            out,
            PrStage1HudPresentationDirectBee4ActionKind::RightRankRow,
            "rightRankRow",
            kStage1HudRightRankRowTplAddrs[rowIndex],
            kStage1HudRightRankX[rowIndex] + rightRankOffsetX,
            kStage1HudRightRankY[rowIndex] + yOffset,
            dimClut ? 1 : 0,
            static_cast<uint8_t>(rowIndex),
            dimClut);
    }

    if (state.transitionState916D0 == 2u) {
        AppendStage1HudB590Action(
            out,
            PrStage1HudPresentationDirectBee4ActionKind::
                TransitionPromptManual,
            "transitionPromptManual",
            kStage1HudTplTransitionPromptManual,
            20.0f,
            53.0f,
            0u);
    } else if (state.transitionState916D0 == 1u) {
        AppendStage1HudB590Action(
            out,
            PrStage1HudPresentationDirectBee4ActionKind::
                TransitionPromptRandom,
            "transitionPromptRandom",
            kStage1HudTplTransitionPromptRandom,
            28.0f,
            53.0f,
            0u);
    }

    return out;
}

PrStage1HudPresentationDirectBee4ConsumeResult
PrStage1HudPresentationDirectConsumeSub8001DF24Bee4RawActions(
    const PrStage1HudPresentationDirectBee4ActionCarrier& carrier,
    PrPsxFastSpriteSubmitDirect::RuntimeState8003FA20& runtime,
    bool apply) {
    PrStage1HudPresentationDirectBee4ConsumeResult out{};
    out.valid = true;
    out.applied = apply;
    out.runtimeOwnerProvided = true;
    out.sourceFunction = carrier.sourceFunction;
    out.actionCount = carrier.actionCount;
    out.droppedActionCount = carrier.droppedActionCount;
    out.droppedGapCount = carrier.droppedGapCount;
    out.runtimeBefore = runtime;

    const uint32_t count =
        (std::min)(carrier.actionCount,
                   kPrStage1HudPresentationDirectMaxBee4Actions);
    for (uint32_t i = 0; i < count; ++i) {
        PrStage1Scene1Movie1Direct::Movie1RawDrawTypedSubmitResult submit =
            apply
                ? PrStage1Scene1Movie1Direct::ApplyRawDrawTypedSubmit(
                      carrier.actions[i],
                      runtime,
                      0u)
                : PrStage1Scene1Movie1Direct::PredictRawDrawTypedSubmit(
                      carrier.actions[i],
                      runtime,
                      0u);
        out.submits[i] = submit;
        if (!submit.valid) {
            ++out.gapCount;
            continue;
        }
        ++out.consumedCount;
        if (submit.submitted) {
            ++out.submittedCount;
        }
        if (submit.partial) {
            ++out.partialCount;
        }
        if (submit.skipped) {
            ++out.skippedCount;
        }
        if (submit.status !=
            PrStage1Scene1Movie1Direct::
                Movie1PsxRawDrawResolutionStatus::ResolvedTypedSubmit) {
            ++out.gapCount;
        }
    }
    out.gapCount += carrier.gapCount + carrier.droppedGapCount;
    out.runtimeAfter = runtime;
    return out;
}

void PrStage1HudPresentationDirectDrawSub8001DB9C(
    PrGameContext& ctx,
    float vx,
    float vy,
    float vs,
    const PrStage1HudPresentationDirectTemplates& templates,
    bool visible,
    uint8_t lessonId) {
    if (!ctx.renderer || !visible || lessonId < 1u || lessonId > 6u) {
        return;
    }

    (void)DrawPsxSpriteTemplate(ctx,
                                vx,
                                vy,
                                vs,
                                101.0f,
                                107.0f,
                                templates.topLessonLabel,
                                1.0f,
                                1.0f,
                                1.0f,
                                1.0f,
                                920);
    (void)DrawPsxSpriteTemplate(ctx,
                                vx,
                                vy,
                                vs,
                                207.0f,
                                107.0f,
                                templates.topLessonNumbers[(size_t)lessonId - 1u],
                                1.0f,
                                1.0f,
                                1.0f,
                                1.0f,
                                920);
}

void PrStage1HudPresentationDirectDrawSub8001DF24(
    PrGameContext& ctx,
    float vx,
    float vy,
    float vs,
    const PrStage1HudPresentationDirectTemplates& templates,
    const PrStage1HudPresentationDirectState& state) {
    if (!ctx.renderer) {
        return;
    }

    const float yOffset = state.highLayoutMode ? 0.0f : 30.0f;
    const uint8_t activeRow =
        (std::min<uint8_t>)(state.rightRankActiveRow, 3u);

    if (state.rightRankFlag0200Pulse) {
        Stage1ResolveSub8001DE08SideFlag(state.rightRankTransitionAnim18E);
        PrStage1HudPresentationDirectDrawSub8001DE08(
            ctx,
            vx,
            vy,
            vs,
            templates,
            true,
            yOffset);
    }
    PrStage1HudPresentationDirectDrawSub8001DE08(
        ctx,
        vx,
        vy,
        vs,
        templates,
        false,
        yOffset);

    const float scoreOffsetX = state.scorePanelOffsetX;
    const float rightRankOffsetX = state.rightRankPanelOffsetX;

    (void)DrawPsxSpriteTemplate(ctx,
                                vx,
                                vy,
                                vs,
                                20.0f + scoreOffsetX,
                                176.0f + yOffset,
                                templates.scoreLabel,
                                1.0f,
                                1.0f,
                                1.0f,
                                1.0f,
                                930);
    DrawStage1ScoreDigitsSub8001B590(
        ctx,
        vx,
        vy,
        vs,
        templates,
        state.scoreDisplayValue,
        58.0f + scoreOffsetX,
        176.0f + yOffset);

    (void)DrawPsxSpriteTemplate(ctx,
                                vx,
                                vy,
                                vs,
                                192.0f + rightRankOffsetX,
                                136.0f + 13.0f * (float)activeRow + yOffset,
                                templates.rightRankHighlight,
                                1.0f,
                                1.0f,
                                1.0f,
                                1.0f,
                                930);

    for (size_t rowIndex = 0; rowIndex < templates.rightRankRows.size(); rowIndex++) {
        PsxSpriteTemplate tpl = templates.rightRankRows[rowIndex];
        if (Stage1RightRankRowUsesDimClut(state, rowIndex)) {
            tpl.clutY_px = templates.rightRankDimClutY;
        }
        (void)DrawPsxSpriteTemplate(ctx,
                                    vx,
                                    vy,
                                    vs,
                                    kStage1HudRightRankX[rowIndex] + rightRankOffsetX,
                                    kStage1HudRightRankY[rowIndex] + yOffset,
                                    tpl,
                                    1.0f,
                                    1.0f,
                                    1.0f,
                                    1.0f,
                                    930);
    }

    if (!ctx.renderOnlyFrame) {
        ++s_sub8001DF24FrameCounter;
    }
    DrawStage1TransitionPromptSub8001DF24(
        ctx,
        vx,
        vy,
        vs,
        templates,
        state.transitionState916D0);
}

void PrStage1HudPresentationDirectDrawNumericStatus(
    PrGameContext& ctx,
    float vx,
    float vy,
    float vs,
    const PrStage1HudPresentationDirectTemplates& templates,
    const PrStage1HudPresentationDirectState& state) {
    PrStage1HudPresentationDirectDrawSub8001DB9C(
        ctx,
        vx,
        vy,
        vs,
        templates,
        state.topLessonPairChangeVisible,
        state.topLessonPairLessonId);
    PrStage1HudPresentationDirectDrawSub8001DF24(
        ctx,
        vx,
        vy,
        vs,
        templates,
        state);
}
