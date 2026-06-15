#pragma once

#include "pr_stage1_movie_segment_direct.h"

#include <array>
#include <cstdint>
#include <vector>

namespace PrStage1LifecycleDirect {

struct Globals801CA3BC {
    uint32_t sourceFunction = 0x801CA3BCu;
    bool returnValueKnown = false;
    int32_t returnValue = 0;
    uint32_t ptr800943C0 = 0;
    uint32_t ptr800943C4 = 0;
    uint32_t value800943C8 = 0;
    uint32_t ptr800943CC = 0;
    uint32_t ptr800943D0 = 0;
    uint32_t ptr800943D4 = 0;
    uint32_t ptr800943D8 = 0;
    uint32_t ptr800943DC = 0;
    uint32_t ptr800943E0 = 0;
    uint32_t ptr800943E4 = 0;
    uint32_t ptr800943E8 = 0;
    std::array<uint32_t, 4> ptr800943EC_F8{};
    uint32_t ptr800943FC = 0;
    uint32_t ptr80094400 = 0;
    std::array<uint32_t, 4> ptr80094404_10{};
    std::array<uint32_t, 7> ptr80094414_2C{};
    uint32_t fn80094430 = 0;
    uint32_t fn80094434 = 0;
    uint32_t fn80094438 = 0;
    uint32_t fn8009443C = 0;
    uint32_t fn80094440 = 0;
};

struct Scene1LoaderRecord801C7284 {
    bool present = false;
    uint8_t slotIndex = 0;
    uint32_t psxOffsetFromSceneEntry = 0;
    bool psxAddrKnown = false;
    uint32_t psxAddr = 0;
    PrStage1MovieSegmentDirect::MovieSegmentRecord48 movieSegmentRecord{};
    PrStage1MovieSegmentDirect::SegmentInitResult8001A324 rowInit8001A324{};
};

struct SceneEntryCompoLoadFeedback801C7284 {
    bool known = false;
    bool pending = true;
    bool completed = false;
    bool success = false;
    int32_t result = 0;

    uint32_t loaderFunction = 0x8001AC18u;
    uint32_t loaderOffsetFromSceneEntry = 0x3Cu;
    bool loaderPsxAddrKnown = false;
    uint32_t loaderPsxAddr = 0;
    uint8_t rowIndex = 1;
    int32_t mode = 0;

    uint8_t maxAttempts = 4;
    int32_t firstAttemptParserFlag = 1;
    int32_t retryParserFlag = 0;

    uint8_t descriptorCopyFirstWord = 4;
    uint8_t descriptorCopyWordCount = 6;
    bool descriptorCopyWordsKnown = false;
    std::array<bool, 6> descriptorCopyWordKnown{};
    std::array<uint32_t, 6> descriptorCopyWords{};

    bool intHeaderRecordProducerPending = true;
    bool cdPayloadFeedbackPending = true;
    bool memoryRegistrationFeedbackPending = true;
    bool gpuTimUploadFeedbackPending = true;
    bool spuVabFeedbackPending = true;
};

struct InitSceneInput801C7284 {
    uint8_t sceneIndex = 1;
    uint32_t rawArgA3 = 0;
    uint32_t rawArgA4 = 0;
    int16_t sceneEntryField06_Bpm100 = 11000;
    int16_t sceneEntryField08_TickOffset = 96;
    int16_t sceneEntryField0A_ExtraTickOffset = 0;
    std::array<bool, 7> loaderPresent{};
    bool compoLoadTableValue801CCBBCKnown = false;
    uint16_t compoLoadTableValue801CCBBC = 0;
    bool movieSegmentCdSeamProbeFeedbackKnown = false;
    PrStage1MovieSegmentDirect::MovieSegmentCdSeamProbeFeedback801C4780
        movieSegmentCdSeamProbeFeedback{};
    bool movieSegmentCdLookupBatchFeedbackKnown = false;
    PrStage1MovieSegmentDirect::MovieSegmentCdLookupBatchFeedback801C4780
        movieSegmentCdLookupBatchFeedback{};
    bool row1CompoLoadFeedbackKnown = false;
    SceneEntryCompoLoadFeedback801C7284 row1CompoLoadFeedback{};
};

enum class ActionKind801C7284 : uint8_t {
    None,
    ResetResourcePool25A34,
    StageRunnerResetState,
    ResetEventRuntime24E98,
    ResetScorer14344,
    InitSegmentRecord1A324,
    ResetResourceCursor25A00,
    ResetHoldTiles1EF14,
    LoadCompo15660,
};

enum class MovieSegmentFeedbackSource801C7284 : uint8_t {
    None = 0,
    SingleCdSeamProbe,
    CdLookupBatch,
};

struct Action801C7284 {
    ActionKind801C7284 kind = ActionKind801C7284::None;
    uint32_t psxOrder = 0;
    uint32_t psxFunction = 0;
    bool psxFunctionKnown = true;

    bool sceneEntryPtrKnown = false;
    uint32_t sceneEntryPtr8006EDB8 = 0;
    bool loaderOffsetFromSceneEntryKnown = false;
    uint32_t loaderOffsetFromSceneEntry = 0;
    bool sceneLoaderSlotKnown = false;
    bool sceneLoaderSlotPresent = false;
    uint8_t sceneLoaderSlot = 0;

    uint8_t sceneIndex = 0;
    bool compoLoadTableValue801CCBBCKnown = false;
    uint16_t compoLoadTableValue801CCBBC = 0;
    uint32_t rawArg0 = 0;
    uint32_t rawArg1 = 0;
    uint32_t rawArg2 = 0;
    uint32_t rawArg3 = 0;
};

struct SceneEntry801C7284 {
    uint8_t sceneIndex = 1;
    bool sceneEntryPtr8006EDB8Known = false;
    uint32_t sceneEntryPtr8006EDB8 = 0;
    int32_t field348_TicksPerMinute = 0;
    int32_t field352_TicksPerFrame60 = 0;
    int32_t field356_BaseTick = 0;
    int32_t field360 = 0;
    PrStage1MovieSegmentDirect::SceneEntryMovieSegmentTable801C4780
        movieSegmentTable801C4780{};
    PrStage1MovieSegmentDirect::MovieSegmentScanResult801C4780
        movieSegmentScan801C4780{};
    MovieSegmentFeedbackSource801C7284 movieSegmentFeedbackSource =
        MovieSegmentFeedbackSource801C7284::None;
    std::array<Scene1LoaderRecord801C7284, 7> loaders{};
    bool compoLoadTableValue801CCBBCKnown = false;
    uint16_t compoLoadTableValue801CCBBC = 0;
    SceneEntryCompoLoadFeedback801C7284 row1CompoLoadFeedback{};
    std::vector<Action801C7284> initActions801C7284;
};

enum class Phase801C81EC : uint8_t {
    Entry,
    InitialMovie1Requested,
    StageLoop,
    StageRunRequested,
    AbortPollRequested,
    ClearTailPreludeRequested,
    ClearTailStatusRequested,
    ClearTailMovieRequested,
    Uncovered,
};

struct Runtime801C81EC {
    Phase801C81EC phase = Phase801C81EC::Entry;
    bool entryPrologueRequested = false;
    bool clearTailStageStatusKnown = false;
    int32_t clearTailStageStatus166AC = 0;
    uint32_t nextPsxActionOrder = 0;
};

struct FrameInput801C81EC {
    uint8_t sceneId = 1;
    uint16_t word800916D0 = 0;
    uint16_t word800916DA = 0;
    bool word800916F0Known = false;
    uint16_t word800916F0 = 0;
    uint16_t word80091816 = 0;
    uint32_t dword801D3040 = 0;
    bool byte801C368E = false;

    bool movie1PlayAndWaitResultKnown = false;
    int32_t movie1PlayAndWaitResult = 0;

    bool stageResultKnown = false;
    int32_t stageResult801C7A60 = 0;

    bool abortPollResultKnown = false;
    int32_t abortPollResult26B94 = 0;

    bool stageStatus166ACKnown = false;
    int32_t stageStatus166AC = 0;

    bool clearTailPlayAndWaitResultKnown = false;
    int32_t clearTailPlayAndWaitResult = 0;
};

enum class ActionKind801C81EC : uint8_t {
    None,
    ConfigMovieViewport,
    AudioReset26FA4,
    Transition201AC,
    Transition20110,
    StrInit,
    StrPlayAndWait,
    StageRecordTick1A4D0,
    StageRunnerRun7A60,
    QueryStageStatus166AC,
    ResetHoldTiles1EF14,
    RestoreTransitionPayload15744,
    QueryAbort26B94,
    SfxCue26EF8,
    AudioFlush26ECC,
    SaveStatus1635C,
    UnlockNextStage1628C,
    Bootstrap15590,
    SaveUi19148,
};

enum class StrBlockKind801C81EC : uint8_t {
    None,
    InitialMovie1,
    ClearTailMovie,
};

struct Action801C81EC {
    ActionKind801C81EC kind = ActionKind801C81EC::None;

    // Monotonic PSX-sequence token. Adapters must apply actions in this order.
    uint32_t psxOrder = 0;

    uint32_t psxFunction = 0;
    bool psxFunctionKnown = true;
    bool blocksUntilResult = false;
    uint32_t transitionWorkPtr = 0;
    uint32_t transitionA1_801C3640 = 0;
    int32_t transitionModeA2 = 0;
    int32_t transitionPreFfd4ArgA3 = 0;
    int32_t transitionPostFfd4ArgA4 = 0;
    bool transitionHasExtraDelay27194 = false;
    int32_t transitionExtraDelayFrames27194 = 0;
    uint32_t transitionFinishFunction = 0;
    uint32_t loaderOffsetFromSceneEntry = 0;
    StrBlockKind801C81EC strBlockKind = StrBlockKind801C81EC::None;
    bool sceneLoaderSlotKnown = false;
    bool sceneLoaderSlotPresent = false;
    uint8_t sceneLoaderSlot = 0;
    bool sceneLoaderRecordKnown = false;
    uint8_t sceneLoaderRowIndex = 0;
    bool sceneLoaderPsxAddrKnown = false;
    uint32_t sceneLoaderPsxAddr = 0;
    PrStage1MovieSegmentDirect::MovieSegmentRecord48
        sceneLoaderMovieSegmentRecord{};
    PrStage1MovieSegmentDirect::SegmentInitResult8001A324
        sceneLoaderRowInit8001A324{};
    bool bootstrapZcompoLoader = false;
    uint8_t sceneId = 0;
    int32_t arg0 = 0;
    int32_t arg1 = 0;
    int32_t arg2 = 0;
    int32_t arg3 = 0;
    uint32_t rawArg0 = 0;
    uint32_t rawArg1 = 0;
    uint32_t rawArg2 = 0;
    uint32_t rawArg3 = 0;
};

struct StepResult801C81EC {
    Phase801C81EC phaseBefore = Phase801C81EC::Entry;
    Phase801C81EC phaseAfter = Phase801C81EC::Entry;
    std::vector<Action801C81EC> actions;

    bool write800916D0 = false;
    uint16_t word800916D0 = 0;
    bool write800916DA = false;
    uint16_t word800916DA = 0;
    bool write800916E0 = false;
    uint16_t word800916E0 = 0;

    bool sceneResultKnown = false;
    int32_t sceneResult = 0;
    bool blockedByUnknownWord800916F0 = false;
};

Globals801CA3BC InitGlobals801CA3BC();
SceneEntry801C7284 InitScene801C7284(const InitSceneInput801C7284& input);
void Reset801C81ECRuntime(Runtime801C81EC& runtime);
StepResult801C81EC Step801C81EC(Runtime801C81EC& runtime,
                                const SceneEntry801C7284& sceneEntry,
                                const FrameInput801C81EC& input);

} // namespace PrStage1LifecycleDirect
