#pragma once

#include "pr_psx_fast_sprite_submit_direct.h"
#include "pr_psx_graph_owner_direct.h"
#include "pr_movie_subtitles.h"

#include <cstdint>
#include <vector>

enum class StrPlayerResult;

namespace PrStage1MovieTextDirect {
struct Movie1TextWindowTickResult;
}

namespace PrStage1Scene1Movie1Direct {

struct SubtitleFrameWindow {
    bool valid = false;
    uint32_t startFrame = 0;
    uint32_t firstTextFrame = 0;
    uint32_t endFrame = 0;
    uint32_t descOffset = 0;
};

enum class Movie1PsxEntryPoint : uint32_t {
    Sub801C8270 = 0x801C8270u,
    Sub801C77C0 = 0x801C77C0u,
    Sub801C7A60 = 0x801C7A60u,
};

enum class Movie1PsxPhase : uint8_t {
    Idle = 0,
    MovieStrSub801C77C0 = 1,
    MovieOutroSub80020308 = 2,
    StageLoopSub801C7A60 = 3,
};

enum class Movie1PsxDrawHelper : uint8_t {
    Sub8001CE30_NoSubtitleFrame = 0,
    Sub8001C864_SubtitleFrame = 1,
    Sub8001F230_OutroNoSubboxFrame = 2,
    Sub8001FEB4_FinalNoVideoFrame = 3,
    Sub800201AC_TransitionFrame = 4,
};

enum class Movie1PsxTemplateSubmitHelper : uint8_t {
    Sub8001B590 = 0,
    Sub8001C550 = 1,
};

enum class TransitionSub800201ACPhase : uint8_t {
    Idle = 0,
    InitialSub8001FFD4,
    LoopSub8001EA74,
    EndSub8001FFD4,
    TailSub80020090,
    Complete,
};

enum class TransitionSub8001EA74Mode : uint8_t {
    Default = 0,
    Mode1_Sub8001F524 = 1,
    Mode2_Sub8001FCBC_Sub8001FDC0 = 2,
    Mode3_Sub80022CBC_Sub8001D74C_Sub8001FC40_Sub8001FDC0 = 3,
    Mode4_Sub80021E60_Sub8001D74C_Sub8001FC40_Sub8001FDC0 = 4,
    Mode5_Sub80020308 = 5,
    Mode6_Sub80020248 = 6,
};

enum class TransitionSub800201ACAction : uint8_t {
    None = 0,
    Sub8001FFD4_ResetGp196AndInitMask,
    Sub8001EA74_SetGp872AndGp792,
    Sub80040F90_SetDrawBuffer,
    Sub80040CC8_BindBufferState,
    Sub80027194_Cue8006EC18AndFlush,
    Sub80035560_WaitGpu2,
    Sub8001EBF4_Sub80040370,
    Sub80040420_ClearWhenGp792NonZero,
    Sub80040CA4_PresentGp872Buffer,
    Sub8001F518_CheckGp196Lt191,
    Sub8001F524_Mode1Clear8AndDrawMask,
    Sub8001FCBC_Mode2Advance8,
    Sub80022CBC_Mode3DirectDraw,
    Sub80021E60_Mode4DirectDraw,
    Sub8001D74C_Mode3Or4DirectDraw,
    Sub8001FC40_Mode3Or4Advance8,
    Sub8001FDC0_DrawMask,
    Sub80020308_Mode5OutroStep,
    Sub80020248_Mode6OutroStep,
    Sub800271E4_PlayCue0,
    Sub800271E4_PlayCue1,
    Sub8001EF40_DefaultScrollMask,
    SetGp196To190,
    Sub80020090_TailFrame,
    Sub80020008_TailFrame,
    Sub8001F524_ClearMaskOrderDword8004EB80,
    Sub8001F524_DrawActiveMask,
    Sub8001F698_Pattern4SetMask,
    Sub8001F698_Pattern4ClearMask,
    Sub8001C864_Mode6SubtitleFrame,
    Sub8001CE30_Mode6NoSubtitleFrame,
    Sub8001F230_OutroNoSubboxFrame,
    Sub8001FEB4_FinalNoVideoFrame,
};

enum class TransitionTileMaskMutationKind : uint8_t {
    None = 0,
    InitAllClearBySub8001FFD4,
    InitAllSetBySub8001FFD4,
    ClearByDword8004EB80Order,
    SetBySub8001F698Pattern4,
    ClearBySub8001F698Pattern4,
    DrawActiveMask,
};

struct TransitionTileMaskMutation {
    TransitionTileMaskMutationKind kind = TransitionTileMaskMutationKind::None;
    uint32_t startGp196 = 0;
    uint32_t count = 0;
    uint32_t pattern = 0;
    uint32_t value = 0;
    uint32_t layer = 0;
};

struct Movie1TransitionCtxWords801C3640 {
    bool known = false;
    uint32_t base = 0x801C3640u;
    uint32_t word0_flags801C3640 = 0;
    uint32_t word1_timecode801C3644 = 0;
};

static constexpr uint32_t kMovie1TransitionTileMaskRows = 12u;
static constexpr uint32_t kMovie1TransitionTileMaskCols = 16u;
static constexpr uint32_t kMovie1TransitionTileMaskCellCount =
    kMovie1TransitionTileMaskRows * kMovie1TransitionTileMaskCols;

struct Movie1TransitionTileMaskRuntimeState {
    bool ownerValid = false;
    bool cellsKnown = false;
    uint32_t sourceFunction = 0x8001FDC0u;
    uint32_t mutationSerial = 0;
    uint8_t cells[kMovie1TransitionTileMaskCellCount]{};
    TransitionTileMaskMutation lastMutations[8]{};
    uint32_t lastMutationCount = 0;
};

enum class Movie1PsxRawDrawHelper : uint8_t {
    None = 0,
    Call8001C550,
    Call8001C5A8,
    Call8001B590,
    Call8001BEE4,
};

enum class Movie1PsxRawDrawCondition : uint8_t {
    Always = 0,
    ContextWordEquals,
    ContextWordNotEquals,
    ContextWordDefault,
    ActiveTileMaskCell,
};

enum class Movie1PsxRawDrawResolutionStatus : uint8_t {
    RawOnly = 0,
    ResolvedTemplateCommand,
    ResolvedTypedSubmit,
    GapCondition,
    GapLanguageOrPointerTable,
    GapTemplatePointer,
    GapPositionPair,
    GapTemplateDesc,
    GapActiveTileMask,
    GapOtMapping,
    GapUnknownHelper,
    GapSub8001B590Submit,
    GapFastSpriteStackRgb,
    GapRawDrawFastSpriteStackRgb,
    GapTransitionRawActionFastSpriteStackRgb,
    GapTextGlyphFastSpriteRgb,
    GapGsSortFastSpriteSubmit,
    GapPlanFull,
};

enum class Movie1PsxFastSpriteLocalRgbGapReason : uint8_t {
    None = 0,
    StackLocalTailUnresolved8001B590,
    RawDrawStackLocalTailUnresolved8001B590,
    TransitionRawActionStackLocalTailUnresolved8001B590,
    TextGlyphLocalRgbUnresolved8001B954,
    StackLocalTailUnresolved8001BEE4,
};

static constexpr uint32_t kMovie1DrawPlanMaxTemplates = 512;
static constexpr uint32_t kMovie1DrawPlanMaxRawActions = 512;
inline constexpr uint16_t kMovie1PsxDefaultLayer = 480u;

struct Movie1PsxRawDrawC5A8Metadata {
    bool valid = false;
    uint32_t sourceFunction = 0x8001C5A8u;
    Movie1PsxRawDrawHelper submittedHelper = Movie1PsxRawDrawHelper::Call8001B590;
    uint32_t positionPairAddr = 0;
    uint32_t resolvedPositionPairAddr = 0;
    int16_t resolvedX = 0;
    int16_t resolvedY = 0;
    uint32_t xOffset = 0;
    uint32_t yOffset = 2;
    uint32_t pairBytes = 4;
    uint32_t templatePtrAddr = 0;
    uint32_t resolvedTemplateAddr = 0;
    uint8_t languageIndex = 0;
    bool languageIndexKnown = false;
    bool positionPairResolved = false;
    bool templatePointerResolved = false;
    uint32_t workBaseAddr = 0;
    uint32_t workStride = 20;
    uint32_t bufferIndex = 0;
    uint32_t workEntryAddr = 0;
    uint16_t ot = 0;
    bool needsSub8001B590Direct = true;
};

struct Movie1PsxRawDrawB25CMetadata {
    bool valid = false;
    uint32_t sourceFunction = 0x8001B25Cu;
    uint32_t templateAddr = 0;
    uint32_t copiedAttr = 0;
    uint16_t sourceWord4 = 0;
    uint16_t sourceWord6 = 0;
    uint16_t width = 0;
    uint16_t height = 0;
    uint16_t sourceWord12 = 0;
    uint16_t sourceWord14 = 0;
    uint16_t glyphIndex = 0;
    bool addGlyphIndexToFinalWord = false;
    uint16_t computedUvWord = 0;
    uint8_t computedUByte = 0;
    uint8_t computedVByte = 0;
    uint16_t computedClutWord = 0;
    uint16_t finalWord = 0;
};

struct Movie1PsxRawDrawBE34Metadata {
    bool valid = false;
    uint32_t sourceFunction = 0x8001BE34u;
    uint32_t templateAddr = 0;
    uint32_t copiedAttr = 0;
    uint16_t sourceWord4 = 0;
    uint16_t sourceWord6 = 0;
    uint16_t width = 0;
    uint16_t height = 0;
    uint16_t sourceWord12 = 0;
    uint16_t sourceWord14 = 0;
    int32_t enabled = 0;
    uint16_t computedUvWord = 0;
    uint8_t computedUByte = 0;
    uint8_t computedVByte = 0;
    uint16_t computedClutWord = 0;
    uint16_t finalWord = 0;
    bool forcesEnabledClutY = false;
};

struct Movie1PsxGsSortFastSpriteSubmitMetadata {
    bool valid = false;
    uint32_t sourceFunction = 0x8003FA20u;
    uint32_t callerFunction = 0x8001B590u;
    uint32_t localSpritePacketStackOffset = 0x10u;
    uint32_t localSpritePacketBytes = 24u;
    uint32_t emittedPacketWords = 6u;
    uint32_t packetAllocatorGlobal = 0x800901C8u;
    uint32_t drawOffsetXGlobal = 0x800917AAu;
    uint32_t drawOffsetYGlobal = 0x800917ACu;
    uint32_t otStructAddr = 0;
    uint16_t priority = 0;
    uint32_t otHeadOffset = 4u;
    uint32_t otLengthOffset = 8u;
    int32_t otSlotScale = 4;
    int32_t otLengthScale = -4;
    uint32_t otSlotExpressionSource = 0x8003FB58u;
    uint32_t packetLinkTagAddend = 0x05000000u;
    uint32_t packetAddrMask = 0x00FFFFFFu;
    bool rejectsNegativeAttr = true;
    bool rejectsZeroWidth = true;
    bool rejectsZeroHeight = true;
    bool writesPacketToOt = true;
    bool advancesPacketAllocator = true;
    bool localSpritePrefixKnown = false;
    bool localSpriteRgbKnown = false;
    Movie1PsxFastSpriteLocalRgbGapReason localSpriteRgbGapReason =
        Movie1PsxFastSpriteLocalRgbGapReason::None;
    bool localSpriteKnown = false;
    PrPsxFastSpriteSubmitDirect::FastSpriteLocalFields8003FA20 localSprite{};
    PrPsxFastSpriteSubmitDirect::FastSpriteRawLocalBytes8003FA20
        rawLocalBytes{};
};

struct Movie1PsxRawDrawB590Metadata {
    bool valid = false;
    uint32_t sourceFunction = 0x8001B590u;
    int16_t screenX = 0;
    int16_t screenY = 0;
    int16_t spriteX = 0;
    int16_t spriteY = 0;
    uint32_t templateAddr = 0;
    uint16_t glyphIndex = 0;
    bool addGlyphIndexToFinalWord = false;
    uint16_t priority = 0;
    uint32_t otPacketAddr = 0;
    uint32_t workBaseAddr = 0;
    uint32_t workStride = 20;
    uint32_t bufferIndex = 0;
    Movie1PsxRawDrawB25CMetadata b25c{};
    Movie1PsxGsSortFastSpriteSubmitMetadata gsSortFastSprite{};
    bool callsGsSortFastSprite = true;
    bool submitImplemented = false;
};

struct Movie1Sub8001DF24StackResidueCarrier {
    bool valid = false;
    uint32_t sourceFunction = 0x8001DF24u;
    uint32_t producerFunction = 0x8004800Cu;
    uint32_t producerPc = 0x80048040u;
    uint32_t consumerFunction = 0x8001BEE4u;
    uint32_t consumerCallsite = 0x8001BF1Cu;
    bool savedRegisterKnown = false;
    uint32_t savedRegisterValue = 0;
    PrPsxFastSpriteSubmitDirect::FastSpriteStackScratchTail8003FA20
        stackTail{};
};

struct Movie1PsxRawDrawBEE4Metadata {
    bool valid = false;
    uint32_t sourceFunction = 0x8001BEE4u;
    int16_t screenX = 0;
    int16_t screenY = 0;
    int16_t spriteX = 0;
    int16_t spriteY = 0;
    uint32_t templateAddr = 0;
    int32_t enabled = 0;
    uint16_t priority = 3;
    uint32_t otPacketAddr = 0;
    uint32_t workBaseAddr = 0;
    uint32_t workStride = 20;
    uint32_t bufferIndex = 0;
    Movie1PsxRawDrawBE34Metadata be34{};
    Movie1Sub8001DF24StackResidueCarrier stackResidue{};
    Movie1PsxGsSortFastSpriteSubmitMetadata gsSortFastSprite{};
    bool callsGsSortFastSprite = true;
    bool submitImplemented = false;
};

struct Movie1PsxRawDrawAction {
    Movie1PsxRawDrawHelper helper = Movie1PsxRawDrawHelper::None;
    Movie1PsxRawDrawCondition condition = Movie1PsxRawDrawCondition::Always;
    Movie1PsxRawDrawResolutionStatus resolutionStatus =
        Movie1PsxRawDrawResolutionStatus::RawOnly;
    uint32_t sourceFunction = 0;
    uint32_t psxCallOrder = 0;
    int16_t x = 0;
    int16_t y = 0;
    uint32_t positionAddr = 0;
    uint32_t positionLanguageStride = 0;
    uint32_t templateAddr = 0;
    uint32_t templatePtrTableAddr = 0;
    uint32_t templateLanguageStride = 0;
    uint32_t templateGpBaseAddr = 0;
    uint32_t templateGpOffset = 0;
    uint32_t ot = 0;
    uint32_t bufferIndex = 0;
    uint32_t workBaseAddr = 0;
    uint8_t languageIndex = 0;
    bool languageIndexKnown = false;
    uint32_t contextPtr = 0;
    uint32_t contextOffset = 0;
    int32_t contextValue = 0;
    uint32_t languageSourceAddr = 0;
    uint32_t tableIndex = 0;
    uint32_t activeTileMaskIndex = 0;
    uint16_t activeTileMaskRow = 0;
    uint16_t activeTileMaskCol = 0;
    bool activeTileMaskKnown = false;
    bool activeTileMaskActive = false;
    Movie1PsxRawDrawC5A8Metadata c5a8{};
    Movie1PsxRawDrawB25CMetadata b25c{};
    Movie1PsxRawDrawB590Metadata b590{};
    Movie1PsxRawDrawBE34Metadata be34{};
    Movie1PsxRawDrawBEE4Metadata bee4{};
};

struct TransitionSub800201ACStep {
    bool drawFrame = false;
    bool tailFrame = false;
    bool complete = false;
    TransitionSub800201ACPhase phase = TransitionSub800201ACPhase::Idle;
    TransitionSub8001EA74Mode mode = TransitionSub8001EA74Mode::Default;
    TransitionSub800201ACAction actions[16]{};
    uint32_t actionCount = 0;
    uint32_t a1 = 0;
    uint32_t a2 = 0;
    uint32_t a3 = 0;
    uint32_t a4 = 0;
    uint32_t gp196 = 0;
    uint32_t nextGp196 = 0;
    uint32_t gp792 = 0;
    uint32_t nextGp792 = 0;
    uint32_t gp872 = 0;
    uint32_t nextGp872 = 0;
    uint32_t tailFramesRemaining = 0;
    uint32_t tailIteration = 0;
    uint32_t psxResult = 0;
    Movie1PsxDrawHelper drawHelper =
        Movie1PsxDrawHelper::Sub8001CE30_NoSubtitleFrame;
    TransitionTileMaskMutation tileMutations[8]{};
    uint32_t tileMutationCount = 0;
    Movie1TransitionTileMaskRuntimeState activeTileMask{};
    std::vector<Movie1PsxRawDrawAction> psxDrawActions{};
    uint32_t psxDrawActionCount = 0;
    Movie1TransitionCtxWords801C3640 ctxWords801C3640{};
};

struct Movie1FrameState {
    float vx = 0.0f;
    float vy = 0.0f;
    float vs = 1.0f;
    float videoX = 0.0f;
    float videoY = 0.0f;
    float videoW = 0.0f;
    float videoH = 0.0f;
    float subBoxX = 0.0f;
    float subBoxY = 0.0f;
    float subBoxW = 0.0f;
    float subBoxH = 0.0f;
    bool drawSubBox = false;
    bool useSubtitleLayout = false;
    bool useEndingNoSubBoxLayout = false;
    bool useFinalNoVideoLayout = false;
    Movie1PsxDrawHelper psxDrawHelper = Movie1PsxDrawHelper::Sub8001CE30_NoSubtitleFrame;
};

struct Movie1PsxTemplateDesc {
    uint32_t attr = 0;
    uint16_t texX = 0;
    uint16_t texY = 0;
    uint16_t w = 0;
    uint16_t h = 0;
    uint16_t clutX = 0;
    uint16_t clutY = 0;
    bool valid = false;
};

Movie1PsxTemplateDesc ResolveMovie1PsxTemplateDesc(uint32_t tplAddr);

Movie1PsxRawDrawB25CMetadata BuildRawDrawTileRectFieldsSub8001B25C(
    uint32_t templateAddr,
    const Movie1PsxTemplateDesc& desc,
    uint16_t glyphIndex,
    bool addGlyphIndexToFinalWord);
Movie1PsxRawDrawBE34Metadata BuildRawDrawTileRectFieldsSub8001BE34(
    uint32_t templateAddr,
    const Movie1PsxTemplateDesc& desc,
    int32_t enabled);

Movie1PsxRawDrawAction BuildRawDrawWrapperActionSub8001C5A8(
    uint32_t sourceFunction,
    uint32_t positionAddr,
    uint32_t templateAddr,
    uint32_t ot,
    uint32_t bufferIndex,
    uint32_t workBaseAddr);
Movie1PsxRawDrawAction BuildRawDrawSubmitActionSub8001B590(
    uint32_t sourceFunction,
    int16_t screenX,
    int16_t screenY,
    uint32_t templateAddr,
    uint16_t glyphIndex,
    bool addGlyphIndexToFinalWord,
    uint16_t priority,
    uint32_t otPacketAddr,
    uint32_t bufferIndex,
    uint32_t workBaseAddr);
Movie1PsxRawDrawAction BuildRawDrawSubmitActionSub8001BEE4(
    uint32_t sourceFunction,
    int16_t screenX,
    int16_t screenY,
    uint32_t templateAddr,
    int32_t enabled,
    uint32_t otPacketAddr,
    uint32_t bufferIndex,
    uint32_t workBaseAddr,
    const PrPsxFastSpriteSubmitDirect::FastSpriteStackScratchTail8003FA20&
        stackTail);
PrPsxFastSpriteSubmitDirect::FastSpriteLocalFields8003FA20
BuildRawDrawFastSpriteLocalSub8001B590(
    const Movie1PsxRawDrawB590Metadata& b590);
PrPsxFastSpriteSubmitDirect::FastSpriteLocalFields8003FA20
BuildRawDrawFastSpriteLocalSub8001BEE4(
    const Movie1PsxRawDrawBEE4Metadata& bee4);

struct Movie1RawDrawFastSpriteRuntimeBridgeInput {
    bool valid = false;
    Movie1PsxRawDrawB590Metadata b590{};
    Movie1PsxRawDrawBEE4Metadata bee4{};
    PrPsxFastSpriteSubmitDirect::GsSortFastSpriteSubmitProvenance8003FA20
        provenance{};
    PrPsxFastSpriteSubmitDirect::RuntimeState8003FA20 runtime{};
};

struct Movie1RawDrawFastSpriteRuntimeBridgeResult {
    bool valid = false;
    bool applied = false;
    bool partial = false;
    Movie1RawDrawFastSpriteRuntimeBridgeInput bridgeInput{};
    PrPsxFastSpriteSubmitDirect::GsSortFastSpriteInput8003FA20 submitInput{};
    PrPsxFastSpriteSubmitDirect::GsSortFastSpriteResult8003FA20 submitResult{};
    PrPsxFastSpriteSubmitDirect::GsSortFastSpritePartialResult8003FA20
        partialSubmitResult{};
    PrPsxFastSpriteSubmitDirect::RuntimeUpdate8003FA20 runtimeUpdate{};
    PrPsxFastSpriteSubmitDirect::RuntimeState8003FA20 runtimeAfter{};
};

Movie1RawDrawFastSpriteRuntimeBridgeInput
BuildRawDrawFastSpriteRuntimeBridgeInputSub8001B590(
    const Movie1PsxRawDrawB590Metadata& b590,
    const PrPsxFastSpriteSubmitDirect::RuntimeState8003FA20& runtime,
    const Movie1PsxRawDrawAction* action = nullptr);
Movie1RawDrawFastSpriteRuntimeBridgeResult
PredictRawDrawFastSpriteRuntimeBridgeSub8001B590(
    const Movie1RawDrawFastSpriteRuntimeBridgeInput& input);
Movie1RawDrawFastSpriteRuntimeBridgeResult
ApplyRawDrawFastSpriteRuntimeBridgeSub8001B590(
    const Movie1RawDrawFastSpriteRuntimeBridgeInput& input);
Movie1RawDrawFastSpriteRuntimeBridgeInput
BuildRawDrawFastSpriteRuntimeBridgeInputSub8001BEE4(
    const Movie1PsxRawDrawBEE4Metadata& bee4,
    const PrPsxFastSpriteSubmitDirect::RuntimeState8003FA20& runtime,
    const Movie1PsxRawDrawAction* action = nullptr);
Movie1RawDrawFastSpriteRuntimeBridgeResult
PredictRawDrawFastSpriteRuntimeBridgeSub8001BEE4(
    const Movie1RawDrawFastSpriteRuntimeBridgeInput& input);
Movie1RawDrawFastSpriteRuntimeBridgeResult
ApplyRawDrawFastSpriteRuntimeBridgeSub8001BEE4(
    const Movie1RawDrawFastSpriteRuntimeBridgeInput& input);

struct Movie1RawDrawTypedSubmitResult {
    bool valid = false;
    bool applied = false;
    bool submitted = false;
    bool skipped = false;
    bool partial = false;
    Movie1PsxRawDrawResolutionStatus status =
        Movie1PsxRawDrawResolutionStatus::RawOnly;
    Movie1PsxRawDrawAction action{};
    Movie1PsxRawDrawB590Metadata b590{};
    Movie1PsxRawDrawBEE4Metadata bee4{};
    Movie1RawDrawFastSpriteRuntimeBridgeResult fastSprite{};
};

Movie1RawDrawTypedSubmitResult
PredictRawDrawTypedSubmit(
    const Movie1PsxRawDrawAction& action,
    const PrPsxFastSpriteSubmitDirect::RuntimeState8003FA20& runtime,
    uint8_t languageIndex = 0);
Movie1RawDrawTypedSubmitResult
ApplyRawDrawTypedSubmit(
    const Movie1PsxRawDrawAction& action,
    PrPsxFastSpriteSubmitDirect::RuntimeState8003FA20& runtime,
    uint8_t languageIndex = 0);

struct Movie1TemplateDrawCommand {
    float x = 0.0f;
    float y = 0.0f;
    uint32_t tplAddr = 0;
    Movie1PsxTemplateDesc desc{};
    Movie1PsxTemplateSubmitHelper submitHelper = Movie1PsxTemplateSubmitHelper::Sub8001B590;
    float alpha = 1.0f;
    int layer = 0;
    int order = 0;
};

struct Movie1VideoRectDrawCommand {
    float x = 0.0f;
    float y = 0.0f;
    float w = 0.0f;
    float h = 0.0f;
};

enum class Movie1DrawPlanBuildFailureReason : uint8_t {
    None = 0,
    RuntimeInactive,
    StrPlayerNotReady,
    StrVideoFinishedBeforeOutro,
    HelperPlanFull,
};

struct Movie1DrawPlan {
    bool buildFailureKnown = false;
    Movie1DrawPlanBuildFailureReason buildFailureReason =
        Movie1DrawPlanBuildFailureReason::None;
    Movie1FrameState frame{};
    Movie1VideoRectDrawCommand video{};
    bool drawVideo = false;
    std::vector<Movie1TemplateDrawCommand> templates{};
    uint32_t templateCount = 0;
    std::vector<Movie1PsxRawDrawAction> rawDrawActions{};
    uint32_t rawDrawActionCount = 0;
    uint32_t rawResolvedTemplateCount = 0;
    uint32_t rawResolvedTypedSubmitCount = 0;
    uint32_t rawAppliedTypedSubmitCount = 0;
    uint32_t rawTypedSubmitOwnerGapCount = 0;
    uint32_t rawTypedSubmitRgbGapCount = 0;
    uint32_t rawDrawFastSpriteRgbGapCount = 0;
    uint32_t transitionRawActionFastSpriteRgbGapCount = 0;
    uint32_t textGlyphFastSpriteRgbGapCount = 0;
    uint32_t rawOnlyActionCount = 0;
};

struct OutroSub80020308Step {
    bool keepVideoFrame = false;
    bool noVideoFrame = false;
    bool complete = false;
    bool playCue0 = false;
    bool playCue1 = false;
    int psxResult = 0;
    uint32_t nextGp196 = 0;
    Movie1PsxDrawHelper drawHelper =
        Movie1PsxDrawHelper::Sub8001CE30_NoSubtitleFrame;
};

struct Movie1RuntimeState {
    Movie1RuntimeState();

    bool strPlayed = false;
    bool strStarted = false;
    bool outroActive = false;
    bool outroDrainStrAudio = false;
    bool outroCue0Played = false;
    bool outroCue1Played = false;
    uint32_t outroGp196 = 0;
    uint32_t outroSourceFrame30 = 0;
    uint32_t outroTailFrames = 0;
    bool outroTailCompletePending = false;
    bool playAndWaitCompletionPending = false;
    uint32_t transitionGp792 = 0;
    uint32_t transitionGp872 = 0;
    PrPsxGraphOwnerDirect::PsxGraphState graphOwner8003FB9C{};
    bool transitionSub800201ACActive = false;
    bool transitionSub800201ACCompletesMovie = true;
    bool transitionSub800201ACCompleted = false;
    TransitionSub800201ACPhase transitionSub800201ACPhase =
        TransitionSub800201ACPhase::Idle;
    uint32_t transitionSub800201ACA1 = 0;
    uint32_t transitionSub800201ACA2 = 0;
    uint32_t transitionSub800201ACA3 = 0;
    uint32_t transitionSub800201ACA4 = 0;
    uint32_t transitionSub80020090TailIndex = 0;
    bool transitionSub80020110Variant = false;
    uint32_t transitionSub80027194Counter = 0;
    Movie1TransitionCtxWords801C3640 transitionCtxWords801C3640{};
    Movie1TransitionTileMaskRuntimeState transitionTileMask{};
    bool transitionDrawStepValid = false;
    TransitionSub800201ACStep transitionDrawStep{};
    Movie1TransitionTileMaskRuntimeState transitionDrawMask{};
    bool rawDrawFastSpriteRuntimeOwnerValid = false;
    PrPsxFastSpriteSubmitDirect::RuntimeState8003FA20 rawDrawFastSpriteRuntime{};
    bool lastDrawPlanBuildFailureKnown = false;
    Movie1DrawPlanBuildFailureReason lastDrawPlanBuildFailureReason =
        Movie1DrawPlanBuildFailureReason::None;
    uint32_t skipPreludeFrames = 0;
    uint32_t skipPreludeSourceFrame30 = 0;
    SubtitleFrameWindow subtitleWindow{};
    uint32_t currentMovieFrame30 = 0;
};

struct Movie1DrawableStateQueryResult {
    bool drawableActive = false;
    bool strFrameActive = false;
    bool outroActive = false;
    bool outroTailActive = false;
    bool skipPreludeActive = false;
};

enum class Movie1HostActionKind : uint8_t {
    None = 0,
    StopStr,
    PlayMovie1Str,
    PauseStr,
    PlayMovie1Cue9441C,
    PlayMovieTransitionCue8006EC18,
    LogDebugDirectBootSkip,
};

struct Movie1HostAction {
    Movie1HostActionKind kind = Movie1HostActionKind::None;
    uint32_t subFrame = 0;
    uint8_t cue9441CIndex = 0;
};

struct Movie1HostActionList {
    Movie1HostAction actions[16]{};
    uint32_t count = 0;
};

struct Movie1HostFeedback {
    bool strPlayerReady = false;
    bool movie1StrExists = false;
    bool debugStage1DirectBootRequested = false;
    bool debugF1StrSkipRequested = false;
    bool inputMaskSub80035510Known = false;
    uint32_t inputMaskSub80035510 = 0;
    bool subtitleEnabled = false;
    bool freezeSubbox = false;
    uint8_t languageIndex = 0;
    uint32_t movieFrame30 = 0;
    StrPlayerResult lastStrUpdateResult{};
    bool strVideoFinished = false;
};

struct Movie1HostStrPollPlan {
    bool strPlayerReady = false;
    bool shouldUpdateStr = false;
    bool skipAllowed = false;
    bool preludeActive = false;
    bool tailActive = false;
    bool outroDrainStrAudio = false;
};

struct Movie1AdvanceResult {
    bool handledFrame = false;
    bool completedToStage1 = false;
    bool resetStageRenderRuntime = false;
    bool resetTextRuntimes = false;
    Movie1HostActionList hostActions{};
};

enum class Movie1HostActionFeedbackKind : uint8_t {
    None = 0,
    PlayMovie1Str,
};

struct Movie1HostActionFeedback {
    Movie1HostActionFeedbackKind kind = Movie1HostActionFeedbackKind::None;
    uint32_t actionIndex = 0;
    bool playAttempted = false;
    bool playSucceeded = false;
};

struct Movie1HostActionFeedbackResolution {
    Movie1AdvanceResult advanceResult{};
    Movie1HostActionList followupHostActions{};
};

SubtitleFrameWindow ScanSubtitleFrameWindow(
    const PrMovieSubtitles::MovieSubtitleTrack& track);
SubtitleFrameWindow SelectMovieTextDescSub80024C84(
    const PrMovieSubtitles::MovieSubtitleTrack& track);
const char* ResolveMovieTextSub80024CF8(
    const PrMovieSubtitles::MovieSubtitleTrack& track,
    uint32_t frame30,
    int lang);
bool IsMovieTextFrameVisibleSub80024CF8(
    uint32_t movieFrame30,
    const SubtitleFrameWindow& window,
    bool textVisible);
Movie1PsxPhase ResolveMovie1PsxPhaseSub801C8270(
    const Movie1RuntimeState& state,
    bool stageLoopActive);
TransitionSub800201ACStep ResolveTransitionSub800201ACStep(
    uint32_t gp196,
    uint32_t tailFramesRemaining,
    uint32_t mode);
TransitionSub800201ACStep ResolveTransitionSub8001EA74Step(
    uint32_t a1,
    uint32_t mode,
    uint32_t gp196,
    uint32_t gp792,
    uint32_t gp872,
    bool word800916DC,
    uint32_t word80096590);
TransitionSub800201ACStep ResolveTransitionSub800201ACStep(
    const Movie1RuntimeState& state);
TransitionSub800201ACStep ResolveTransitionSub800201ACStep(
    const Movie1RuntimeState& state,
    bool word800916DC);
void ResetRuntime(Movie1RuntimeState& state);
void ClearCurrentMovieFrame30(Movie1RuntimeState& state);
void SetSubtitleWindow(Movie1RuntimeState& state,
                       const SubtitleFrameWindow& window);
void SetTransitionCtxWords801C3640(
    Movie1RuntimeState& state,
    const Movie1TransitionCtxWords801C3640& ctxWords);
void SetRawDrawFastSpriteRuntime8003FA20(
    Movie1RuntimeState& state,
    const PrPsxFastSpriteSubmitDirect::RuntimeState8003FA20& runtime);
void ClearRawDrawFastSpriteRuntime8003FA20(Movie1RuntimeState& state);
bool SyncRawDrawFastSpriteRuntimeOwner8003FA20(Movie1RuntimeState& state);
bool PrepareMovie1DrawRuntimeSub801C77C0(
    Movie1RuntimeState& state,
    PrStage1MovieTextDirect::Movie1TextWindowTickResult& textWindowTick);
bool TryGetDrawBufferWord80096590(const Movie1RuntimeState& state,
                                  uint8_t& out);
void ApplyTransitionStateMovieBoundaryPrelude(Movie1RuntimeState& state,
                                               uint16_t transitionState916D0);
void BeginOutro(Movie1RuntimeState& state,
                uint32_t movieFrame30,
                bool drainStrAudio);
void BeginSkipPrelude(Movie1RuntimeState& state, uint32_t movieFrame30);
bool TickSkipPrelude(Movie1RuntimeState& state);
bool IsSkipPreludeActive(const Movie1RuntimeState& state);
Movie1DrawableStateQueryResult QueryDrawableState(
    const Movie1RuntimeState& state);
Movie1HostStrPollPlan BuildHostStrPollPlan(
    const Movie1RuntimeState& state,
    const Movie1HostFeedback& host);
Movie1HostActionFeedbackResolution ApplyHostActionFeedback(
    Movie1RuntimeState& state,
    const Movie1AdvanceResult& advance,
    const Movie1HostActionFeedback& feedback);
OutroSub80020308Step ResolveOutroSub80020308Step(
    const Movie1RuntimeState& state,
    bool word800916DC);
OutroSub80020308Step AdvanceOutroSub80020308(Movie1RuntimeState& state,
                                             bool word800916DC);
OutroSub80020308Step ResolveOutroSub80020308Step(bool word800916DC,
                                                 uint32_t gp196);
OutroSub80020308Step AdvanceOutroSub80020308(bool word800916DC,
                                             uint32_t& gp196);
OutroSub80020308Step ResolveOutroSub80020248Step(bool word800916DC,
                                                 uint32_t gp196);
OutroSub80020308Step AdvanceOutroSub80020248(bool word800916DC,
                                             uint32_t& gp196);
void BeginTransitionSub800201ACTail(Movie1RuntimeState& state,
                                    uint32_t sourceFrame30);
void BeginTransitionSub800201AC(Movie1RuntimeState& state,
                                uint32_t a1,
                                uint32_t modeA2,
                                uint32_t preFfd4ArgA3,
                                uint32_t postFfd4ArgA4,
                                uint32_t sourceFrame30,
                                bool completesMovie);
void BeginTransitionSub80020110(Movie1RuntimeState& state,
                                uint32_t a1,
                                uint32_t modeA2,
                                uint32_t preFfd4ArgA3,
                                uint32_t postFfd4ArgA4,
                                uint32_t sourceFrame30,
                                bool completesMovie);
bool ConsumeTransitionSub800201ACCompleted(Movie1RuntimeState& state);
bool IsPlayAndWaitCompletionPending(const Movie1RuntimeState& state);
bool IsTransitionSub80020110Active(const Movie1RuntimeState& state,
                                   uint32_t modeA2,
                                   uint32_t preFfd4ArgA3,
                                   uint32_t postFfd4ArgA4);
bool ApplySub80027194CueCadence(Movie1RuntimeState& state);
void ClearPlayAndWaitCompletionPending(Movie1RuntimeState& state);
Movie1AdvanceResult AdvanceRuntimePure(
    Movie1RuntimeState& state,
    const Movie1HostFeedback& host,
    const PrMovieSubtitles::MovieSubtitleTrack& subtitleTrack);
void CalcMovie1VideoRect(bool subtitleEnabled,
                         float vx,
                         float vy,
                         float vs,
                         float& outX,
                         float& outY,
                         float& outW,
                         float& outH);
void CalcSubtitleBoxRect(float vx,
                         float vy,
                         float vs,
                         float& outX,
                         float& outY,
                         float& outW,
                         float& outH);
Movie1FrameState BuildFrameState(bool subtitleEnabled,
                                  float vx,
                                  float vy,
                                  float vs,
                                  bool subtitleFrameVisible,
                                  bool endingNoSubBoxLayout,
                                  bool finalNoVideoLayout);
bool BuildDrawPlanFromFrameState(const Movie1FrameState& frame,
                                 Movie1DrawPlan& outPlan,
                                 bool suppressFrameTemplates = false);
bool BuildRuntimeDrawPlan(Movie1RuntimeState& state,
                          const Movie1HostFeedback& host,
                          const PrMovieSubtitles::MovieSubtitleTrack& subtitleTrack,
                          float vx,
                          float vy,
                          float vs,
                          Movie1DrawPlan& outPlan,
                          bool suppressFrameTemplates = false);
bool BuildIntroTransitionDrawPlan(int frameIndex,
                                  float vx,
                                  float vy,
                                  float vs,
                                  Movie1DrawPlan& outPlan);
void InitTransitionTileMaskRuntimeSub8001FFD4(
    Movie1TransitionTileMaskRuntimeState& runtime,
    uint32_t arg);
bool AdvanceTransitionTileMaskSub8001FC40(
    Movie1TransitionTileMaskRuntimeState& runtime,
    uint32_t& gp196,
    uint32_t pattern,
    uint32_t count);
bool AdvanceTransitionTileMaskSub8001FCBC(
    Movie1TransitionTileMaskRuntimeState& runtime,
    uint32_t& gp196,
    uint32_t pattern,
    uint32_t count);
bool BuildTransitionTileMaskDrawPlanSub8001FDC0(
    const Movie1TransitionTileMaskRuntimeState& runtime,
    uint16_t priority,
    float vx,
    float vy,
    float vs,
    Movie1DrawPlan& outPlan);
bool AppendResolvedRawDrawActionsToPlan(const TransitionSub800201ACStep& step,
                                        Movie1DrawPlan& plan,
                                        uint8_t languageIndex = 0,
                                        Movie1RuntimeState* rawDrawRuntimeOwner =
                                            nullptr,
                                        const Movie1TransitionTileMaskRuntimeState* activeTileMask =
                                            nullptr);

}  // namespace PrStage1Scene1Movie1Direct
