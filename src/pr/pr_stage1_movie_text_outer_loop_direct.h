#pragma once

#include "pr_stage1_movie_text_direct.h"
#include "pr_stage1_movie_segment_direct.h"
#include "pr_stage1_xa_cd_direct.h"

#include <array>
#include <cstdint>
#include <filesystem>

namespace PrStage1LifecycleExecutorDirect {
struct State801C81EC;
}

namespace PrStage1MovieTextOuterLoopDirect {

inline constexpr uint32_t kSub801C455C = 0x801C455Cu;
inline constexpr uint32_t kSub801C4350 = 0x801C4350u;
inline constexpr uint32_t kSub801C448C = 0x801C448Cu;
inline constexpr uint32_t kByte800493F4StreamClock = 0x800493F4u;
inline constexpr uint32_t kMovieTextOuterLoopWarmupFramesSub801C455C = 1800u;
inline constexpr uint32_t kMovieTextOuterLoopInputConfirmMaskSub801C455C = 0x0100u;
inline constexpr uint32_t kMovieTextOuterLoopInputAbortMaskSub801C455C = 0x0840u;
inline constexpr uint32_t kMovieTextOuterLoopInputAbortExactSub801C455C = 0x0800u;

enum class MovieTextOuterLoopPhaseSub801C455C : uint8_t {
    Idle = 0,
    Warmup,
    InitMain,
    Main,
    Cleanup,
    Complete,
};

enum class MovieTextOuterLoopExitReasonSub801C455C : uint8_t {
    None = 0,
    InputConfirm0100,
    InputAbort0800Or0840,
    GsWorkBaseOne,
    MovieStepReturnedFalse,
};

enum class MovieTextOuterLoopActionKindSub801C455C : uint8_t {
    None = 0,
    SelectDescSub80024C84,
    ClearDword801C954C,
    PollSub8001A750,
    DisplaySub8001EC54Ctx7,
    WaitGpuSub80035560Arg2,
    FlushSub8001ED3C,
    InitSub8001A280,
    CopyMemory164ToCtxC,
    CopyWord800916D8ToCtx66,
    CopyWord800916DCToCtx68And54,
    ClearCtxWord0,
    PollInputSub80035510,
    ExitInput0100ReturnOne,
    ExitInput0800Or0840,
    AdvanceTextSub80024CF8,
    WorkbaseStepSub80027528,
    GapMovieStepSub801C448C,
    WaitGpuSub801C448CArg2,
    SnapshotWorkBaseSub8001A3C8,
    InitSub801C448CSub8001A280,
    QueryMovieCounterSub8001A7A4,
    ResolveMsfToLbaSub80036A78,
    GapByte800493F4ClockProducer,
    WriteWatchdogDword80049404Sub8001A7A4,
    UpdateCtxTimecodeSub801C4350,
    QueryMovieEndedSub8001A7F8,
    GapHostDerivedSegmentWindowFields,
    ReturnMovieStepSub801C448C,
    EndTextFrameSub8001ED74,
    DrawTextWorkSub8002756C,
    GapDword801C9554Writer,
    CheckGsGetWorkBase,
    StopCallbacksSub8001A694StopCallback,
    CleanupSub80027664,
    CleanupSub8001A4A4Arg1,
    CleanupSub8001A694,
    CleanupNullsub4Ctx,
    CleanupSub8001B120Arg1,
};

struct MovieTextOuterLoopActionSub801C455C {
    MovieTextOuterLoopActionKindSub801C455C kind =
        MovieTextOuterLoopActionKindSub801C455C::None;
    uint32_t psxFunctionAddr = 0;
    bool gap = false;
    bool appliedToDirectRuntime = false;
    bool valueKnown = false;
    uint32_t arg0 = 0;
    uint32_t arg1 = 0;
    uint32_t value = 0;
};

struct MovieTextOuterLoopInputSub801C455C {
    uint8_t languageIndex = 0;
    uint16_t word800916D8 = 0;
    uint16_t word800916DC = 0;
    bool memory164Known = false;
    uint32_t memory164 = 0;
    bool memory164FromSceneEntrySource = false;
    uint32_t movieFrame30 = 0;
    uint32_t queryFrame60 = 0;
    bool inputMaskSub80035510Known = false;
    uint32_t inputMaskSub80035510 = 0;
    bool sub8001A750Known = false;
    uint32_t sub8001A750Result = 0;
    bool dword801C9554Known = false;
    bool dword801C9554EqualsOne = false;
    bool gsGetWorkBaseKnown = false;
    uint32_t gsGetWorkBaseResult = 0;
    bool byte800493F4Known = false;
    uint8_t byte800493F4_00 = 0;
    uint8_t byte800493F4_01 = 0;
    uint8_t byte800493F4_02 = 0;
    bool segmentTimeBaseA1Plus40Known = false;
    int32_t segmentTimeBaseA1Plus40 = 0;
    bool sub801C448CKnown = false;
    bool sub801C448CResult = false;
    bool sub8001A7A4Known = false;
    int32_t sub8001A7A4Result = -1;
    bool dword80049404Known = false;
    int32_t dword80049404 = 0;
    bool segmentEndA1Plus44Known = false;
    int32_t segmentEndA1Plus44 = 0;
    bool segmentEndBiasA1Plus8Known = false;
    int32_t segmentEndBiasA1Plus8 = 0;
    bool segmentWindowFieldsFromDirectCarrier = false;
    bool outerLoopInputAuthorityFromScanCarrier = false;
    bool sub8001A7F8Known = false;
    uint32_t sub8001A7F8Result = 0;
    bool currentGp872SlotKnown = false;
    uint8_t currentGp872Slot = 0;
    bool cdLookupFeedbackCarrierKnown = false;
    bool cdLookupFeedbackCarrierGap = false;
    bool streamClockCarrierKnown = false;
    bool streamClockProducerGap = false;
};

struct MovieTextOuterLoopRuntimeSub801C455C {
    MovieTextOuterLoopPhaseSub801C455C phase =
        MovieTextOuterLoopPhaseSub801C455C::Idle;
    MovieTextOuterLoopExitReasonSub801C455C exitReason =
        MovieTextOuterLoopExitReasonSub801C455C::None;
    uint8_t mode = 0;
    uint8_t languageIndex = 0;
    uint32_t sourceFunction = kSub801C455C;
    uint32_t warmupFramesRemaining =
        kMovieTextOuterLoopWarmupFramesSub801C455C;
    uint32_t mainIteration = 0;
    uint32_t selectedDescAddr = 0;
    bool prologueRecorded = false;
    bool descSelectedSub80024C84 = false;
    bool modeDescGap = false;
    bool sub801C448CGap = false;
    int32_t dword801C954C = 0;
    bool dword80049404Known = false;
    int32_t dword80049404 = 0;
    uint16_t ctxWord04 = 0;
    uint8_t ctxByte06 = 0;
    uint8_t ctxByte07 = 0;
    bool cleanupEntered = false;
    bool complete = false;
    int32_t psxReturnValue = 0;
};

struct MovieTextOuterLoopStepResultSub801C455C {
    MovieTextOuterLoopPhaseSub801C455C phaseBefore =
        MovieTextOuterLoopPhaseSub801C455C::Idle;
    MovieTextOuterLoopPhaseSub801C455C phaseAfter =
        MovieTextOuterLoopPhaseSub801C455C::Idle;
    MovieTextOuterLoopExitReasonSub801C455C exitReason =
        MovieTextOuterLoopExitReasonSub801C455C::None;
    std::array<MovieTextOuterLoopActionSub801C455C, 32> actions{};
    uint32_t actionCount = 0;
    bool movieStepSub801C448CKnown = false;
    bool movieStepSub801C448CResult = true;
    bool sub8001A7A4Computed = false;
    int32_t sub8001A7A4Result = -1;
    bool sub80036A78Computed = false;
    int32_t sub80036A78Result = 0;
    bool gapMissingMemory164SceneEntrySource = false;
    bool gapMissingSub8001A750StatusProducer = false;
    bool gapMissingInputMaskSub80035510Producer = false;
    bool gapMissingDword801C9554Writer = false;
    bool gapMissingGsGetWorkBaseProducer = false;
    bool gapMissingByte800493F4ClockProducer = false;
    bool sub8001A7F8Computed = false;
    bool sub8001A7F8Known = false;
    uint32_t sub8001A7F8Result = 0;
    bool cleanupComplete = false;
    int32_t psxReturnValue = 0;
};

struct MovieTextOuterLoopCdCallbackActionApplyResultSub801C455C {
    bool sawActionSub8001A694 = false;
    uint32_t appliedClearCallbackCount = 0;
    PrStage1XaCdDirectClearCallbackResult8001A694 lastClearCallback{};
    bool sawActionSub8001A3C8 = false;
    uint32_t appliedStreamClockPollCount = 0;
    PrMovieSegmentDirect::StreamClockPollResult8001A3C8
        lastStreamClockPoll{};
};

struct MovieTextOuterLoopScanInputCarrierSub801C455C {
    bool called = false;
    PrStage1MovieSegmentDirect::MovieSegmentScanResult801C4780 scan{};
    PrStage1MovieSegmentDirect::SceneEntrySegmentSelection801C4DC4 selection{};
    MovieTextOuterLoopInputSub801C455C input{};
    bool selectedMovieRowKnown = false;
    bool selectedMovieRowNeedsCdLookupFeedback = false;
    bool selectedMovieRowCdLookupReady = false;
    bool selectedMovieRowMissingCdLookupFeedback = false;
    bool selectedMovieRowCdlFileNameReady = false;
    bool selectedMovieRowMissingCdlFileNameFeedback = false;
    bool byte800493F4ClockProducerKnown = false;
    bool streamClockCarrierKnown = false;
    bool streamClockProducerGap = false;
    bool liveCdLookupFeedbackCarrierKnown = false;
    bool liveCdLookupProducerKnown = false;
    bool liveCdLookupProducerGap = false;
    bool directSegmentWindowFieldsKnown = false;
    bool directSegmentWindowFieldsComplete = false;
    bool memory164Known = false;
    bool memory164FromSceneEntrySource = false;
    bool gapMissingMemory164SceneEntrySource = false;
    bool sub8001A750Known = false;
    bool gapMissingSub8001A750StatusProducer = false;
    bool inputMaskSub80035510Known = false;
    bool gapMissingInputMaskSub80035510Producer = false;
    bool dword801C9554Known = false;
    bool gapMissingDword801C9554Writer = false;
    bool gsGetWorkBaseKnown = false;
    bool gapMissingGsGetWorkBaseProducer = false;
    uint32_t scanRowsNeedingCdLookupFeedback = 0;
    uint32_t scanRowsMissingCdLookupFeedback = 0;
    uint32_t scanRowsMissingCdlFileNameFeedback = 0;
    bool gapMissingCdLookupFeedback = false;
    bool gapMissingCdlFileNameFeedback = false;
    bool gapMissingByte800493F4ClockProducer = false;
    bool gapMissingScanResult = false;
    bool gapMissingSelectedMovieRow = false;
    bool gapMissingSegmentWindowFields = false;
};

struct Stage1MovieTextSceneFrameHostFacts801C455C {
    PrStage1MovieTextDirect::Movie1TextGlobalOptionsCarrier globalOptions{};
    int32_t movieFrame30 = 0;
    bool stageRunning = false;
    int32_t runnerFrame30 = 0;
    bool renderHalfStep = false;
    bool currentGp872SlotKnown = false;
    uint8_t currentGp872Slot = 0;
    bool inputMaskSub80035510Known = false;
    uint32_t inputMaskSub80035510 = 0;
    bool sub8001A750Known = false;
    uint32_t sub8001A750Result = 0;
    bool sub801C448CKnown = false;
    bool sub801C448CResult = false;
    uint8_t languageIndex = 0;
    uint32_t sceneIndex = 0;
    bool sceneEntryMemory164Known = false;
    uint32_t sceneEntryMemory164 = 0;
};

void BeginMovieTextOuterLoopSub801C455C(
    MovieTextOuterLoopRuntimeSub801C455C& outer,
    PrStage1MovieTextDirect::Movie1TextRuntime& textRuntime,
    uint8_t mode,
    uint8_t languageIndex);
void ResetMovieTextOuterLoopSub801C455C(
    MovieTextOuterLoopRuntimeSub801C455C& outer);
void EndMovieTextOuterLoopSub801C455C(
    MovieTextOuterLoopRuntimeSub801C455C& outer,
    PrStage1MovieTextDirect::Movie1TextRuntime& textRuntime);
void CompleteMovieTextOuterLoopFromHostMovieEndSub801C455C(
    MovieTextOuterLoopRuntimeSub801C455C& outer,
    PrStage1MovieTextDirect::Movie1TextRuntime& textRuntime);

MovieTextOuterLoopStepResultSub801C455C StepMovieTextOuterLoopSub801C455C(
    MovieTextOuterLoopRuntimeSub801C455C& outer,
    PrStage1MovieTextDirect::Movie1TextRuntime& textRuntime,
    const MovieTextOuterLoopInputSub801C455C& input,
    PrStage1XaCdDirectState* xaCdState = nullptr);

void TickStage1MovieTextOuterLoopFromSceneFrame801C455C(
    MovieTextOuterLoopRuntimeSub801C455C& outer,
    PrStage1MovieTextDirect::Movie1TextRuntime& textRuntime,
    const PrStage1MovieTextDirect::Movie1TextScene1FrameInputSub801C77C0&
        frameInput,
    uint8_t languageIndex,
    uint32_t sceneIndex,
    PrStage1LifecycleExecutorDirect::State801C81EC& lifecycleExecutor,
    const std::filesystem::path* binPath,
    PrStage1XaCdDirectState* xaCdState,
    PrStage1MovieTextDirect::Movie1TextWindowTickResult& outWindowTick,
    MovieTextOuterLoopStepResultSub801C455C* outLastStep = nullptr);
void TickStage1MovieTextOuterLoopFromHostFacts801C455C(
    MovieTextOuterLoopRuntimeSub801C455C& outer,
    PrStage1MovieTextDirect::Movie1TextRuntime& textRuntime,
    const Stage1MovieTextSceneFrameHostFacts801C455C& facts,
    PrStage1LifecycleExecutorDirect::State801C81EC& lifecycleExecutor,
    const std::filesystem::path& dataRoot,
    PrStage1XaCdDirectState* xaCdState,
    PrStage1MovieTextDirect::Movie1TextWindowTickResult& outWindowTick,
    MovieTextOuterLoopStepResultSub801C455C* outLastStep = nullptr);

}  // namespace PrStage1MovieTextOuterLoopDirect
