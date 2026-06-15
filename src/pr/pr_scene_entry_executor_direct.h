#pragma once

#include <cstddef>
#include <cstdint>
#include <array>

#include "pr_movie_segment_direct.h"
#include "pr_scene_drawbuffer_direct.h"
#include "pr_scene_entry_direct.h"
#include "pr_scene_entry_feedback_adapter_direct.h"
#include "pr_scene_table.h"
#include "pr_stage1_loader_direct.h"

struct PrGameContext;
struct PrStage1LoaderMemoryDirectState;
struct PrStage1XaCdDirectLowerCdProducerResult;

namespace PrSceneEntryExecutorDirect {

static constexpr uint32_t kRow0OverlayTransferMaxCdSeamRequests80015D18 =
    PrMovieSegmentDirect::kOverlayTransferMaxAttempts8001ACF8 * 3u;

struct GenericSwitchRunResult80015788 {
    PrSceneId scene = PrSceneId::Scene0;
    bool decided = false;
};

struct CompletedCall80019414Input80015788 {
    bool completed = false;
    PrSceneEntryDirect::Call80019414Feedback80015788 feedback{};
    uint32_t tablePsxAddress = 0;
    size_t tableByteCount = 0;
    std::array<uint8_t, PrSceneEntryDirect::kHiScoreTableSize80019284>
        tableStorage{};
    const uint8_t* tableBytes = nullptr;
};

struct MainSceneInitGateState80015D18 {
    bool initializedSceneKnown = false;
    PrSceneId initializedScene = PrSceneId::Scene0;
};

CompletedCall80019414Input80015788
BuildCompletedCall80019414InputFromFeedbackAdapterResult80019414(
    const PrSceneEntryFeedbackAdapterDirect::FeedbackAdapterResult80019414&
        adapter);

struct MainSceneTraceExecution80015D18 {
    bool traceOverflow = false;
    bool unsupportedRequest = false;
    bool call8001EF14Executed = false;
    bool call80015CC4Executed = false;
    bool call80015CC4Ok = false;
    bool call8001E34CGap = false;
    bool call8001E34CExecuted = false;
    PrSceneDrawBufferDirect::DrawBufferGlobals8001E33C drawBuffers8001E34C{};
};

enum class Row0OverlayTransferCdSeamRejectReason80015D18 : uint8_t {
    None = 0,
    NotPresent,
    Unhandled,
    MissingAttemptProvenance,
    UnsupportedKind,
    MissingAttemptIndex,
    MissingLowerRequest,
    LowerRequestKindMismatch,
    LowerRequestRouteMismatch,
    LowerRequestIncomplete,
    AttemptRouteMismatch,
    AttemptPayloadMismatch,
};

struct MainSceneCallbackExecution80015D18 {
    bool traceOverflow = false;
    bool unsupportedRequest = false;
    bool invalidCurrentSceneIndex = false;
    bool missingWord800916D0For15CC4Gate = false;
    bool waitingForSceneFn2Result = false;
    bool call80025A34Gap = false;
    bool call80025A34Executed = false;
    bool call80025A34StateApplied = false;
    bool call80025A34StateMissing = false;
    uint32_t call80025A34Count = 0;
    uint32_t call80025A34Function = 0;
    uint32_t call80025A34TailFunction = 0;
    bool call8001A324Gap = false;
    bool call8001A324Executed = false;
    PrMovieSegmentDirect::SegmentInitResult8001A324 row0Init8001A324{};
    bool call800154B0Executed = false;
    bool call800154B0Gap = false;
    PrMovieSegmentDirect::OverlayTransferResult800154B0 overlayTransfer800154B0{};
    uint32_t row0OverlayTransferCdSeamRequestCount = 0;
    bool row0OverlayTransferCdSeamRequestsTruncated = false;
    std::array<PrStage1LoaderDirect::CdSeamResult,
               kRow0OverlayTransferMaxCdSeamRequests80015D18>
        row0OverlayTransferCdSeamRequests{};
    bool row0OverlayTransferCdSeamProducerCalled = false;
    bool row0OverlayTransferCdSeamProducerProduced = false;
    uint32_t row0OverlayTransferCompletedCdSeamRejectedCount = 0;
    uint32_t row0OverlayTransferLastRejectedCdSeamIndex = 0;
    Row0OverlayTransferCdSeamRejectReason80015D18
        row0OverlayTransferLastRejectedCdSeamReason =
            Row0OverlayTransferCdSeamRejectReason80015D18::None;
    PrStage1LoaderDirect::CdSeamResult
        row0OverlayTransferLastRejectedCdSeam{};
    bool row0OverlayTransferCdSeamProducerSkippedForExplicitFeedback = false;
    uint32_t row0OverlayTransferCompletedCdSeamCount = 0;
    bool row0OverlayTransferCompletedCdSeamsTruncated = false;
    std::array<PrStage1LoaderDirect::CdSeamResult,
               kRow0OverlayTransferMaxCdSeamRequests80015D18>
        row0OverlayTransferCompletedCdSeams{};
    bool row0OverlayTransferCompletedFeedbackApplied = false;
    bool fn0Executed = false;
    bool fn0FunctionMatched = false;
    bool fn1Executed = false;
    bool fn1FunctionMatched = false;
    bool fn2Executed = false;
    bool fn2FunctionMatched = false;
    bool fn2ResultKnown = false;
    int32_t fn2Result = 0;
};

struct Row0OverlayTransferCdSeamProducerInput80015D18 {
    PrGameContext* ctx = nullptr;
    bool sceneIndexKnown = false;
    uint32_t sceneIndex = 0;
    PrMovieSegmentDirect::MovieSegmentRecord48 row{};
    PrMovieSegmentDirect::MovieSegmentRowInitFeedback8001A324 rowFeedback{};
    PrMovieSegmentDirect::OverlayTransferResult800154B0 discovery{};
    const PrStage1LoaderDirect::CdSeamResult* requests = nullptr;
    uint32_t requestCount = 0;
    bool requestsTruncated = false;
};

struct Row0OverlayTransferCdSeamProducerOutput80015D18 {
    uint32_t seamCount = 0;
    bool seamsTruncated = false;
    std::array<PrStage1LoaderDirect::CdSeamResult,
               kRow0OverlayTransferMaxCdSeamRequests80015D18>
        seams{};
};

using Row0OverlayTransferCdSeamProducerFn80015D18 = bool (*)(
    const Row0OverlayTransferCdSeamProducerInput80015D18& input,
    Row0OverlayTransferCdSeamProducerOutput80015D18& output,
    void* user);

struct Row0OverlayTransferCdSeamProducer80015D18 {
    Row0OverlayTransferCdSeamProducerFn80015D18 produce = nullptr;
    void* user = nullptr;
};

struct Row0OverlayTransferFeedbackFromCdSeams80015D18 {
    uint32_t seekFeedbackCount = 0;
    std::array<PrMovieSegmentDirect::OverlayTransferSeekFeedback8001A89C,
               PrMovieSegmentDirect::kOverlayTransferMaxAttempts8001ACF8>
        seekFeedback{};
    uint32_t readFeedbackCount = 0;
    std::array<PrMovieSegmentDirect::OverlayTransferReadFeedback8001A818,
               PrMovieSegmentDirect::kOverlayTransferMaxAttempts8001ACF8>
        readFeedback{};
};

bool BuildRow0OverlayTransferFeedbackFromCdSeams80015D18(
    const PrStage1LoaderDirect::CdSeamResult* seams,
    uint32_t seamCount,
    Row0OverlayTransferFeedbackFromCdSeams80015D18& out);

bool BuildScene1Row0OverlayTransferFeedback80015D18(
    PrGameContext& ctx,
    PrSceneId scene,
    PrMovieSegmentDirect::MovieSegmentRowInitFeedback8001A324& out);

bool BuildRow0OverlayTransferCdSeamProducerOutputFromExplicitLowerResult80015D18(
    const PrStage1XaCdDirectLowerCdProducerResult& lower,
    Row0OverlayTransferCdSeamProducerOutput80015D18& out);

MainSceneTraceExecution80015D18
ExecuteMainSceneHostTrace80015D18(
    const PrSceneEntryDirect::MainSceneRequestTrace80015D18& trace,
    bool executeSceneSwitchRequests,
    bool executeSceneCallbackRequests);

MainSceneCallbackExecution80015D18
ExecuteMainSceneCallbacks80015D18(
    const PrSceneEntryDirect::MainSceneStepResult80015D18& step,
    PrGameContext& ctx,
    const PrSceneDef& def,
    bool executeFn0Fn1,
    bool executeFn2,
    const PrMovieSegmentDirect::MovieSegmentRowInitFeedback8001A324*
        row0Feedback8001A324,
    const PrMovieSegmentDirect::OverlayTransferSeekFeedback8001A89C*
        row0SeekFeedback8001A89C,
    uint32_t row0SeekFeedbackCount8001A89C,
    const PrMovieSegmentDirect::OverlayTransferReadFeedback8001A818*
        row0ReadFeedback8001A818,
    uint32_t row0ReadFeedbackCount8001A818,
    PrStage1LoaderMemoryDirectState* loaderMemoryState80025A34 = nullptr,
    const Row0OverlayTransferCdSeamProducer80015D18*
        row0CdSeamProducer8001ACF8 = nullptr);

MainSceneCallbackExecution80015D18
ExecuteMainSceneCallbacks80015D18(
    const PrSceneEntryDirect::MainSceneStepResult80015D18& step,
    PrGameContext& ctx,
    const PrSceneDef& def,
    bool executeFn0Fn1,
    bool executeFn2,
    const PrMovieSegmentDirect::MovieSegmentRowInitFeedback8001A324*
        row0Feedback8001A324,
    const PrStage1LoaderDirect::CdSeamResult* row0CdSeams8001ACF8,
    uint32_t row0CdSeamCount8001ACF8,
    PrStage1LoaderMemoryDirectState* loaderMemoryState80025A34 = nullptr);

MainSceneCallbackExecution80015D18
ExecuteMainSceneCallbacks80015D18(
    const PrSceneEntryDirect::MainSceneStepResult80015D18& step,
    PrGameContext& ctx,
    const PrSceneDef& def,
    bool executeFn0Fn1,
    bool executeFn2,
    const PrMovieSegmentDirect::MovieSegmentRowInitFeedback8001A324*
        row0Feedback8001A324,
    const Row0OverlayTransferCdSeamProducer80015D18*
        row0CdSeamProducer8001ACF8,
    PrStage1LoaderMemoryDirectState* loaderMemoryState80025A34 = nullptr);

void ResetMainSceneInitGate80015D18(
    MainSceneInitGateState80015D18& state);

MainSceneCallbackExecution80015D18
ExecuteMainSceneInitCallbacksOnce80015D18(
    MainSceneInitGateState80015D18& gate,
    PrSceneId scene,
    const PrSceneEntryDirect::MainSceneStepResult80015D18& step,
    PrGameContext& ctx,
    const PrSceneDef& def,
    const PrMovieSegmentDirect::MovieSegmentRowInitFeedback8001A324*
        row0Feedback8001A324,
    PrStage1LoaderMemoryDirectState* loaderMemoryState80025A34 = nullptr);

MainSceneCallbackExecution80015D18
ExecuteMainSceneInitCallbacksOnce80015D18(
    MainSceneInitGateState80015D18& gate,
    PrSceneId scene,
    const PrSceneEntryDirect::MainSceneStepResult80015D18& step,
    PrGameContext& ctx,
    const PrSceneDef& def,
    PrStage1LoaderMemoryDirectState* loaderMemoryState80025A34 = nullptr);

GenericSwitchRunResult80015788
RunGenericSwitch80015788(PrGameContext& ctx,
                         PrSceneTable& table,
                         PrSceneId prevScene);

GenericSwitchRunResult80015788
RunGenericSwitch80015788(PrGameContext& ctx,
                         PrSceneTable& table,
                         PrSceneId prevScene,
                         const CompletedCall80019414Input80015788*
                             completedCall19414);

GenericSwitchRunResult80015788
ExecuteMainSceneSwitchTrace80015D18(
    const PrSceneEntryDirect::MainSceneRequestTrace80015D18& trace,
    PrGameContext& ctx,
    PrSceneTable& table,
    PrSceneId prevScene,
    const CompletedCall80019414Input80015788* completedCall19414 = nullptr);

}  // namespace PrSceneEntryExecutorDirect
