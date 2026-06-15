#pragma once

#include "pr_stage1_loader_direct.h"
#include "pr_stage1_loader_memory_direct.h"

#include <cstddef>
#include <cstdint>

namespace PrStage1LoaderProducerAdapter {

namespace Direct = PrStage1LoaderDirect;

using ProducerCategory = Direct::ProducerCategory;

enum class AdapterStatus : uint8_t {
    None,
    Ready,
    NoWaitingStep,
    NotMemoryStep,
    HelperGap,
    IncompleteFeedback,
};

using TypedActionFeedback = Direct::TypedActionFeedback;

struct MemoryFeedbackProducerResult {
    AdapterStatus status = AdapterStatus::None;
    bool produced = false;
    bool waitingStepValid = false;
    bool memoryCategory = false;
    bool helperGap = false;
    bool incompleteFeedback = false;
    bool helperExecuted = false;
    Direct::MemorySeamResult memoryResult{};
    TypedActionFeedback feedback{};
};

struct ProducerRuntime {
    bool initialized = false;
    PrStage1LoaderMemoryDirectState memoryState{};
};

struct MemoryRuntimeStepResult {
    bool produced = false;
    bool applied = false;
    bool helperGap = false;
};

struct LiveHalFeedbackBuildResult {
    AdapterStatus status = AdapterStatus::None;
    bool produced = false;
    bool waitingStepValid = false;
    bool categoryMatches = false;
    bool helperGap = false;
    bool incompleteFeedback = false;
    bool feedbackComplete = false;
    TypedActionFeedback feedback{};
    bool cdResultKnown = false;
    Direct::CdSeamResult cdResult{};
};

struct GpuTimBytesLiveInput {
    const uint8_t* data = nullptr;
    std::size_t size = 0u;
    uint32_t psxAddress = 0u;
    bool psxAddressKnown = false;
    Direct::ResolvedPayload resolvedPayload{};
    bool resolvedPayloadKnown = false;
};

struct SpuHalResultLiveInput {
    int32_t lowerResult = 0;
    bool lowerResultKnown = false;
    uint32_t vhPtr = 0u;
    bool vhPtrKnown = false;
    uint32_t vbPtr = 0u;
    bool vbPtrKnown = false;
    PrStage1LoaderSpuHal::Cue4 cue{};
    bool cueKnown = false;
};

struct CdLookupLiveInput {
    PrStage1LoaderCdHal::Feedback feedback{};
    PrStage1LoaderCdHal::LookupFeedback800381F8 lookup800381F8{};
    bool lookup800381F8Known = false;
    PrStage1LoaderCdHal::ProbeCompletionFeedback8001A2B0 probe8001A2B0{};
    bool probe8001A2B0Known = false;
};

struct RecordDispatchLiveInput {
    Direct::RecordData recordData{};
    bool recordDataKnown = false;
};

void ResetProducerRuntime(ProducerRuntime& runtime);
void InitializeProducerRuntime(ProducerRuntime& runtime);
bool RunMemoryProducerForWaitingStep(Direct::RunnerState& state,
                                     ProducerRuntime& runtime,
                                     bool applyFeedback,
                                     MemoryRuntimeStepResult* out = nullptr);
bool BuildMemoryTypedFeedback(const Direct::RunnerState& state,
                              PrStage1LoaderMemoryDirectState& memoryState,
                              const Direct::ProducerStep& step,
                              const TypedActionFeedback& skeleton,
                              TypedActionFeedback& outFeedback,
                              MemoryFeedbackProducerResult* out = nullptr);
bool BuildMemoryTypedFeedbackForWaitingStep(
    const Direct::RunnerState& state,
    PrStage1LoaderMemoryDirectState& memoryState,
    TypedActionFeedback& outFeedback,
    MemoryFeedbackProducerResult* out = nullptr);
bool BuildRecordDispatchTypedFeedback(
    const Direct::ProducerStep& step,
    const Direct::RecordData& recordData,
    TypedActionFeedback& outFeedback);
bool BuildRecordDispatchLiveTypedFeedback(
    const Direct::ProducerStep& step,
    const RecordDispatchLiveInput& input,
    TypedActionFeedback& outFeedback,
    LiveHalFeedbackBuildResult* out = nullptr);
bool BuildCdTypedFeedback(const Direct::ProducerStep& step,
                          const Direct::CdSeamResult& cd,
                          bool success,
                          TypedActionFeedback& outFeedback);
bool BuildCdLiveTypedFeedback(const Direct::ProducerStep& step,
                              const Direct::CdSeamResult& cd,
                              bool success,
                              TypedActionFeedback& outFeedback,
                              LiveHalFeedbackBuildResult* out = nullptr);
bool BuildCdLookupLiveTypedFeedback(
    const Direct::ProducerStep& step,
    const CdLookupLiveInput& input,
    bool success,
    TypedActionFeedback& outFeedback,
    LiveHalFeedbackBuildResult* out = nullptr);
Direct::CdSeamResult BuildCdLookupLiveSeamResult(
    const CdLookupLiveInput& input);
bool BuildGpuTypedFeedback(const Direct::ProducerStep& step,
                           const Direct::GpuSeamResult& gpu,
                           bool success,
                           TypedActionFeedback& outFeedback);
bool BuildGpuTimBytesLiveTypedFeedback(
    const Direct::ProducerStep& step,
    const GpuTimBytesLiveInput& timBytes,
    TypedActionFeedback& outFeedback,
    LiveHalFeedbackBuildResult* out = nullptr);
bool BuildGpuTimBytesLiveTypedFeedbackFromResolvedPayload(
    const Direct::RunnerState& state,
    const Direct::ProducerStep& step,
    TypedActionFeedback& outFeedback,
    LiveHalFeedbackBuildResult* out = nullptr);
bool BuildSpuTypedFeedback(const Direct::ProducerStep& step,
                           const Direct::SpuSeamResult& spu,
                           bool success,
                           TypedActionFeedback& outFeedback);
bool BuildSpuHalResultLiveTypedFeedback(
    const Direct::ProducerStep& step,
    const Direct::SpuSeamResult& spu,
    bool success,
    TypedActionFeedback& outFeedback,
    LiveHalFeedbackBuildResult* out = nullptr);
bool BuildSpuHalResultLiveTypedFeedbackFromLowerResult(
    const Direct::ProducerStep& step,
    PrStage1LoaderSpuHal::State& spuState,
    const SpuHalResultLiveInput& input,
    bool success,
    TypedActionFeedback& outFeedback,
    LiveHalFeedbackBuildResult* out = nullptr);
bool BuildPadComTypedFeedback(const Direct::ProducerStep& step,
                              bool completed,
                              bool success,
                              int32_t result,
                              TypedActionFeedback& outFeedback);
bool BuildCallback1537CTypedFeedback(
    const Direct::ProducerStep& step,
    const Direct::Callback1537CFeedback& callback,
    TypedActionFeedback& outFeedback);
bool BuildCallbackDraw1EA74TypedFeedback(
    const Direct::ProducerStep& step,
    const Direct::CallbackDraw1EA74Feedback& callback,
    TypedActionFeedback& outFeedback);
bool BuildCallbackFlush26ECCTypedFeedback(
    const Direct::ProducerStep& step,
    const Direct::CallbackFlush26ECCFeedback& callback,
    TypedActionFeedback& outFeedback);
bool ApplyTypedFeedback(Direct::RunnerState& state,
                        const TypedActionFeedback& feedback);

} // namespace PrStage1LoaderProducerAdapter
