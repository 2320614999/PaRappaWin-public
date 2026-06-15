#include "pr_stage1_loader_producer_adapter.h"

namespace PrStage1LoaderProducerAdapter {

namespace {

void MarkTypedFeedbackKind(TypedActionFeedback& out) {
    switch (out.category) {
    case ProducerCategory::Record:
        out.recordFeedback = true;
        break;
    case ProducerCategory::Cd:
        out.cdFeedback = true;
        break;
    case ProducerCategory::Memory:
        out.memoryFeedback = true;
        break;
    case ProducerCategory::Gpu:
        out.gpuFeedback = true;
        break;
    case ProducerCategory::Spu:
        out.spuFeedback = true;
        break;
    case ProducerCategory::PadCom:
        out.padComFeedback = true;
        break;
    case ProducerCategory::Callback:
        out.callbackFeedback = true;
        break;
    case ProducerCategory::Completion:
        out.completionFeedback = true;
        break;
    default:
        break;
    }
}

LiveHalFeedbackBuildResult MakeLiveHalFeedbackBuildResult(
    const Direct::ProducerStep& step,
    ProducerCategory expectedCategory,
    AdapterStatus status,
    const TypedActionFeedback& feedback = TypedActionFeedback{}) {
    LiveHalFeedbackBuildResult out{};
    out.status = status;
    out.waitingStepValid = step.valid;
    out.categoryMatches = step.valid && step.category == expectedCategory;
    out.helperGap = status == AdapterStatus::HelperGap;
    out.incompleteFeedback = status == AdapterStatus::IncompleteFeedback;
    out.feedback = feedback;
    out.produced = status == AdapterStatus::Ready;
    if (out.produced) {
        out.feedbackComplete =
            Direct::IsTypedActionFeedbackCompleteForProducerStep(step,
                                                                 feedback);
    }
    return out;
}

Direct::GpuSeamResult BuildGpuSeamResultFromTimBytes(
    const Direct::ProducerStep& step,
    const GpuTimBytesLiveInput& timBytes) {
    Direct::GpuSeamResult gpu{};
    gpu.present = true;
    gpu.actionKind = step.gpuActionKind;

    PrStage1LoaderGpuHal::TimPayloadView view{};
    view.data = timBytes.data;
    view.size = timBytes.size;
    view.psxAddress = timBytes.psxAddress;
    view.psxAddressKnown = timBytes.psxAddressKnown;

    gpu.actionList =
        PrStage1LoaderGpuHal::BuildGpuActions8001AE7C(view);
    gpu.timParse = gpu.actionList.timParse;
    gpu.actionListKnown = true;
    gpu.timParseKnown = true;
    return gpu;
}

bool PayloadRecordMatchesStep(const Direct::ResolvedPayload& payload,
                              const Direct::ProducerStep& step) {
    return payload.recordIndex == step.recordIndex;
}

bool BuildGpuTimBytesInputFromResolvedPayload(
    const Direct::ResolvedPayload& payload,
    const Direct::ProducerStep& step,
    GpuTimBytesLiveInput& out) {
    if (!payload.valid || !PayloadRecordMatchesStep(payload, step) ||
        !payload.liveBytesPresent || payload.liveBytesData == nullptr ||
        !payload.liveBytesSizeKnown || payload.liveBytesSize == 0u) {
        out = GpuTimBytesLiveInput{};
        return false;
    }

    out = GpuTimBytesLiveInput{};
    out.data = payload.liveBytesData;
    out.size = payload.liveBytesSize;
    out.psxAddress = payload.psxAddress;
    out.psxAddressKnown = payload.psxAddressKnown;
    out.resolvedPayload = payload;
    out.resolvedPayloadKnown = true;
    return true;
}

bool ResolveGpuTimBytesInputFromRunnerPayload(
    const Direct::RunnerState& state,
    const Direct::ProducerStep& step,
    GpuTimBytesLiveInput& out) {
    if (BuildGpuTimBytesInputFromResolvedPayload(
            step.action.resolvedPayload,
            step,
            out)) {
        return true;
    }

    Direct::ResolvedPayload payload{};
    if (Direct::TryGetCurrentResolvedPayload(state, payload) &&
        BuildGpuTimBytesInputFromResolvedPayload(payload, step, out)) {
        return true;
    }
    if (Direct::TryGetResolvedPayloadForRecord(state,
                                               step.recordIndex,
                                               payload) &&
        BuildGpuTimBytesInputFromResolvedPayload(payload, step, out)) {
        return true;
    }

    out = GpuTimBytesLiveInput{};
    return false;
}

Direct::ResolvedPayload BuildResolvedPayloadFromGpuTimBytes(
    const Direct::ProducerStep& step,
    const GpuTimBytesLiveInput& timBytes) {
    Direct::ResolvedPayload payload =
        timBytes.resolvedPayloadKnown ? timBytes.resolvedPayload
                                      : step.action.resolvedPayload;
    if (payload.recordIndex == 0u) {
        payload.recordIndex = step.recordIndex;
    }
    if (payload.recordType == Direct::LoaderRecordType::Unknown &&
        step.recordType != Direct::LoaderRecordType::Unknown) {
        payload.recordType = step.recordType;
    }
    if (timBytes.psxAddressKnown) {
        payload.psxAddress = timBytes.psxAddress;
        payload.psxAddressKnown = true;
    }
    if (timBytes.data != nullptr) {
        payload.liveBytesPresent = true;
        payload.liveBytesData = timBytes.data;
    }
    if (timBytes.size != 0u) {
        payload.liveBytesSize = timBytes.size;
        payload.liveBytesSizeKnown = true;
        if (!payload.sizeBytesKnown &&
            timBytes.size <= static_cast<std::size_t>(0xFFFFFFFFu)) {
            payload.sizeBytes = static_cast<uint32_t>(timBytes.size);
            payload.sizeBytesKnown = true;
        }
    }
    payload.valid = payload.psxAddressKnown || payload.sizeBytesKnown ||
                    payload.sectorCountKnown || payload.liveBytesPresent;
    return payload;
}

Direct::CdSeamResult BuildCdSeamResultFromLookupInput(
    const CdLookupLiveInput& input) {
    Direct::CdSeamResult cd{};
    cd.present = true;
    cd.feedback = input.lookup800381F8Known ? input.lookup800381F8.result
                                            : input.feedback;
    cd.lookup800381F8 = input.lookup800381F8;
    cd.lookup800381F8Known = input.lookup800381F8Known;
    cd.probe8001A2B0 = input.probe8001A2B0;
    cd.probe8001A2B0Known = input.probe8001A2B0Known;
    return cd;
}

bool GpuLiveResultHasRequiredPayload(
    const Direct::ProducerStep& step,
    const GpuTimBytesLiveInput& timBytes) {
    if (step.category != ProducerCategory::Gpu) {
        return false;
    }
    if (step.action.gpu.timPayloadFromRecord && timBytes.data == nullptr) {
        return false;
    }
    if (step.action.gpu.timPayloadFromRecord && timBytes.size == 0u) {
        return false;
    }
    return true;
}

bool SpuLiveResultHasRequiredPayload(const Direct::ProducerStep& step,
                                     const Direct::SpuSeamResult& spu) {
    if (!spu.present || !spu.actionListKnown) {
        return false;
    }
    if (step.action.spu.lowerResultFeedbackRequired &&
        !spu.lowerResultKnown) {
        return false;
    }
    return true;
}

bool BuildSpuSeamResultFromLowerInput(
    const Direct::ProducerStep& step,
    PrStage1LoaderSpuHal::State& spuState,
    const SpuHalResultLiveInput& input,
    Direct::SpuSeamResult& out) {
    if (!step.valid || step.category != ProducerCategory::Spu) {
        out = Direct::SpuSeamResult{};
        return false;
    }

    PrStage1LoaderSpuHal::LiveActionInput live{};
    live.actionKind = step.spuActionKind;
    live.vhPtr = input.vhPtr;
    live.vhPtrKnown = input.vhPtrKnown;
    live.vbPtr = input.vbPtr;
    live.vbPtrKnown = input.vbPtrKnown;
    live.cue = input.cue;
    live.cueKnown = input.cueKnown;
    live.cuePointer = step.action.spu.cuePointer;
    live.lowerResult = input.lowerResult;
    live.lowerResultKnown = input.lowerResultKnown;

    PrStage1LoaderSpuHal::LiveActionResult result{};
    if (!PrStage1LoaderSpuHal::ApplyLiveActionResult(
            spuState,
            live,
            result)) {
        out = Direct::SpuSeamResult{};
        return false;
    }

    out = Direct::SpuSeamResult{};
    out.present = true;
    out.actionKind = result.actionKind;
    out.actionList = result.actionList;
    out.lowerResult = result.lowerResult;
    out.lowerResultKnown = result.lowerResultKnown;
    out.actionListKnown = true;
    return true;
}

} // namespace

void ResetProducerRuntime(ProducerRuntime& runtime) {
    runtime = ProducerRuntime{};
    PrStage1LoaderMemoryDirectReset(runtime.memoryState);
    runtime.initialized = true;
}

void InitializeProducerRuntime(ProducerRuntime& runtime) {
    if (!runtime.initialized) {
        ResetProducerRuntime(runtime);
    }
}

bool RunMemoryProducerForWaitingStep(Direct::RunnerState& state,
                                     ProducerRuntime& runtime,
                                     bool applyFeedback,
                                     MemoryRuntimeStepResult* out) {
    InitializeProducerRuntime(runtime);

    MemoryRuntimeStepResult result{};

    Direct::ProducerStep waitingStep{};
    if (!Direct::DescribeWaitingProducerStep(state, waitingStep)) {
        if (out) {
            *out = result;
        }
        return false;
    }

    const bool memoryCategory =
        waitingStep.category == ProducerCategory::Memory;
    if (!memoryCategory) {
        result.helperGap = waitingStep.helperGap;
        if (out) {
            *out = result;
        }
        return false;
    }

    PrStage1LoaderMemoryDirectState candidateMemoryState =
        runtime.memoryState;
    TypedActionFeedback feedback{};
    MemoryFeedbackProducerResult feedbackResult{};
    if (!BuildMemoryTypedFeedbackForWaitingStep(state,
                                                candidateMemoryState,
                                                feedback,
                                                &feedbackResult)) {
        result.helperGap = feedbackResult.helperGap;
        if (out) {
            *out = result;
        }
        return false;
    }

    result.produced = true;

    if (!applyFeedback) {
        if (out) {
            *out = result;
        }
        return true;
    }

    result.applied =
        PrStage1LoaderProducerAdapter::ApplyTypedFeedback(state, feedback);
    if (result.applied) {
        runtime.memoryState = candidateMemoryState;
    }
    if (out) {
        *out = result;
    }
    return result.applied;
}

static bool BuildTypedFeedbackSkeleton(const Direct::ProducerStep& step,
                                       TypedActionFeedback& out) {
    if (!step.valid) {
        out = TypedActionFeedback{};
        return false;
    }

    out = TypedActionFeedback{};
    out.valid = true;
    out.category = step.category;
    out.stepKind = step.stepKind;
    out.callbackKind = step.callbackKind;
    out.feedback = step.feedbackSkeleton;
    out.feedback.completed = false;
    out.feedback.success = false;
    MarkTypedFeedbackKind(out);
    return true;
}

bool BuildMemoryTypedFeedback(const Direct::RunnerState& state,
                              PrStage1LoaderMemoryDirectState& memoryState,
                              const Direct::ProducerStep& step,
                              const TypedActionFeedback& skeleton,
                              TypedActionFeedback& outFeedback,
                              MemoryFeedbackProducerResult* out) {
    MemoryFeedbackProducerResult result{};
    result.waitingStepValid = step.valid;
    result.memoryCategory = step.category == ProducerCategory::Memory;

    if (!step.valid || !skeleton.valid) {
        result.status = AdapterStatus::IncompleteFeedback;
        result.incompleteFeedback = true;
        if (out) {
            *out = result;
        }
        outFeedback = TypedActionFeedback{};
        return false;
    }
    if (!result.memoryCategory ||
        skeleton.category != ProducerCategory::Memory ||
        skeleton.stepKind != step.stepKind ||
        skeleton.feedback.psxOrder != step.psxOrder ||
        skeleton.feedback.kind != step.actionKind) {
        result.status = result.memoryCategory ? AdapterStatus::IncompleteFeedback
                                              : AdapterStatus::HelperGap;
        result.helperGap = !result.memoryCategory;
        result.incompleteFeedback = result.memoryCategory;
        if (out) {
            *out = result;
        }
        outFeedback = TypedActionFeedback{};
        return false;
    }

    Direct::MemorySeamResult memoryResult{};
    PrStage1LoaderMemoryDirectState candidateMemoryState = memoryState;
    if (!Direct::BuildMemorySeamResultForProducerStep(
            state,
            candidateMemoryState,
            step,
            memoryResult)) {
        result.status = AdapterStatus::HelperGap;
        result.helperGap = true;
        result.incompleteFeedback = true;
        if (out) {
            *out = result;
        }
        outFeedback = skeleton;
        return false;
    }

    outFeedback = skeleton;
    outFeedback.feedback.completed = true;
    outFeedback.feedback.success =
        Direct::MemorySeamResultSucceededForAction(step.action, memoryResult);
    outFeedback.feedback.hasMemoryResult = true;
    outFeedback.feedback.memoryResult = memoryResult;

    result.status = AdapterStatus::Ready;
    result.produced = true;
    result.helperExecuted = true;
    result.memoryResult = memoryResult;
    result.feedback = outFeedback;
    memoryState = candidateMemoryState;
    if (out) {
        *out = result;
    }
    return true;
}

bool BuildMemoryTypedFeedbackForWaitingStep(
    const Direct::RunnerState& state,
    PrStage1LoaderMemoryDirectState& memoryState,
    TypedActionFeedback& outFeedback,
    MemoryFeedbackProducerResult* out) {
    MemoryFeedbackProducerResult result{};
    Direct::ProducerStep step{};
    if (!Direct::DescribeWaitingProducerStep(state, step)) {
        result.status = AdapterStatus::NoWaitingStep;
        if (out) {
            *out = result;
        }
        outFeedback = TypedActionFeedback{};
        return false;
    }

    TypedActionFeedback skeleton{};
    if (!BuildTypedFeedbackSkeleton(step, skeleton)) {
        result.status = AdapterStatus::IncompleteFeedback;
        result.waitingStepValid = step.valid;
        result.incompleteFeedback = true;
        if (out) {
            *out = result;
        }
        outFeedback = TypedActionFeedback{};
        return false;
    }
    return BuildMemoryTypedFeedback(
        state,
        memoryState,
        step,
        skeleton,
        outFeedback,
        out);
}

bool BuildRecordDispatchTypedFeedback(
    const Direct::ProducerStep& step,
    const Direct::RecordData& recordData,
    TypedActionFeedback& outFeedback) {
    if (!step.valid ||
        step.stepKind != Direct::ProducerStepKind::RecordDispatch1A8F0 ||
        !recordData.valid || !recordData.recordTypeKnown) {
        outFeedback = TypedActionFeedback{};
        return false;
    }
    if (!BuildTypedFeedbackSkeleton(step, outFeedback)) {
        return false;
    }

    outFeedback.feedback.completed = true;
    outFeedback.feedback.success = true;
    outFeedback.feedback.hasRecordType = true;
    outFeedback.feedback.recordType = recordData.recordType;
    outFeedback.feedback.hasRecordData = true;
    outFeedback.feedback.recordData = recordData;
    return true;
}

bool BuildRecordDispatchLiveTypedFeedback(
    const Direct::ProducerStep& step,
    const RecordDispatchLiveInput& input,
    TypedActionFeedback& outFeedback,
    LiveHalFeedbackBuildResult* out) {
    if (!step.valid || step.category != ProducerCategory::Record) {
        outFeedback = TypedActionFeedback{};
        if (out) {
            *out = MakeLiveHalFeedbackBuildResult(
                step,
                ProducerCategory::Record,
                AdapterStatus::HelperGap);
        }
        return false;
    }
    if (!input.recordDataKnown || !input.recordData.valid ||
        !input.recordData.recordTypeKnown) {
        outFeedback = TypedActionFeedback{};
        if (out) {
            *out = MakeLiveHalFeedbackBuildResult(
                step,
                ProducerCategory::Record,
                AdapterStatus::IncompleteFeedback);
        }
        return false;
    }

    Direct::RecordData recordData = input.recordData;
    recordData.recordIndex = step.recordIndex;
    if (!BuildRecordDispatchTypedFeedback(step, recordData, outFeedback)) {
        if (out) {
            *out = MakeLiveHalFeedbackBuildResult(
                step,
                ProducerCategory::Record,
                AdapterStatus::IncompleteFeedback,
                outFeedback);
        }
        return false;
    }
    if (!Direct::IsTypedActionFeedbackCompleteForProducerStep(step,
                                                              outFeedback)) {
        if (out) {
            *out = MakeLiveHalFeedbackBuildResult(
                step,
                ProducerCategory::Record,
                AdapterStatus::IncompleteFeedback,
                outFeedback);
        }
        return false;
    }
    if (out) {
        *out = MakeLiveHalFeedbackBuildResult(
            step,
            ProducerCategory::Record,
            AdapterStatus::Ready,
            outFeedback);
    }
    return true;
}

bool BuildCdTypedFeedback(const Direct::ProducerStep& step,
                          const Direct::CdSeamResult& cd,
                          bool success,
                          TypedActionFeedback& outFeedback) {
    if (!step.valid || step.category != ProducerCategory::Cd ||
        !cd.present) {
        outFeedback = TypedActionFeedback{};
        return false;
    }
    if (!BuildTypedFeedbackSkeleton(step, outFeedback)) {
        return false;
    }

    const bool cdSuccess =
        step.action.cd.actionKind ==
                PrStage1LoaderCdHal::ActionKind::Lookup800381F8
            ? success
            : (cd.feedback.handled &&
               Direct::CdFeedbackKindCompletesProducerStep(
                   step,
                   cd.feedback.kind) &&
               cd.feedback.success);

    Direct::ActionFeedback feedback{};
    if (!Direct::BuildCdActionFeedbackForProducerStep(
            step,
            cd,
            cdSuccess,
            feedback)) {
        outFeedback = TypedActionFeedback{};
        return false;
    }
    outFeedback.feedback = feedback;
    return true;
}

bool BuildCdLiveTypedFeedback(const Direct::ProducerStep& step,
                              const Direct::CdSeamResult& cd,
                              bool success,
                              TypedActionFeedback& outFeedback,
                              LiveHalFeedbackBuildResult* out) {
    auto makeCdOut = [&](AdapterStatus status,
                         const TypedActionFeedback& feedback =
                             TypedActionFeedback{}) {
        LiveHalFeedbackBuildResult result =
            MakeLiveHalFeedbackBuildResult(
                step,
                ProducerCategory::Cd,
                status,
                feedback);
        result.cdResultKnown = cd.present;
        result.cdResult = cd;
        return result;
    };

    if (!step.valid || step.category != ProducerCategory::Cd) {
        outFeedback = TypedActionFeedback{};
        if (out) {
            *out = makeCdOut(AdapterStatus::HelperGap);
        }
        return false;
    }
    if (!Direct::CdSeamResultHasRequiredPayloadForProducerStep(step, cd)) {
        outFeedback = TypedActionFeedback{};
        if (out) {
            *out = makeCdOut(AdapterStatus::IncompleteFeedback);
        }
        return false;
    }
    if (!BuildCdTypedFeedback(step, cd, success, outFeedback) ||
        !Direct::IsActionFeedbackCompleteForProducerStep(
            step,
            outFeedback.feedback,
            outFeedback.resultPresent)) {
        if (out) {
            *out = makeCdOut(AdapterStatus::IncompleteFeedback,
                             outFeedback);
        }
        return false;
    }
    if (out) {
        *out = makeCdOut(AdapterStatus::Ready, outFeedback);
    }
    return true;
}

bool BuildCdLookupLiveTypedFeedback(
    const Direct::ProducerStep& step,
    const CdLookupLiveInput& input,
    bool success,
    TypedActionFeedback& outFeedback,
    LiveHalFeedbackBuildResult* out) {
    const Direct::CdSeamResult cd =
        BuildCdSeamResultFromLookupInput(input);
    return BuildCdLiveTypedFeedback(
        step,
        cd,
        success,
        outFeedback,
        out);
}

Direct::CdSeamResult BuildCdLookupLiveSeamResult(
    const CdLookupLiveInput& input) {
    return BuildCdSeamResultFromLookupInput(input);
}

bool BuildGpuTypedFeedback(const Direct::ProducerStep& step,
                           const Direct::GpuSeamResult& gpu,
                           bool success,
                           TypedActionFeedback& outFeedback) {
    if (!step.valid || step.category != ProducerCategory::Gpu ||
        !gpu.present) {
        outFeedback = TypedActionFeedback{};
        return false;
    }
    if (!BuildTypedFeedbackSkeleton(step, outFeedback)) {
        return false;
    }

    outFeedback.feedback.completed = true;
    outFeedback.feedback.success = success;
    outFeedback.feedback.hasGpuResult = true;
    outFeedback.feedback.gpuResult = gpu;
    if (step.requiredFields.resolvedPayload &&
        step.action.resolvedPayload.valid) {
        outFeedback.feedback.hasResolvedPayload = true;
        outFeedback.feedback.resolvedPayload = step.action.resolvedPayload;
    }
    return true;
}

bool BuildGpuTimBytesLiveTypedFeedback(
    const Direct::ProducerStep& step,
    const GpuTimBytesLiveInput& timBytes,
    TypedActionFeedback& outFeedback,
    LiveHalFeedbackBuildResult* out) {
    if (!step.valid || step.category != ProducerCategory::Gpu) {
        outFeedback = TypedActionFeedback{};
        if (out) {
            *out = MakeLiveHalFeedbackBuildResult(
                step,
                ProducerCategory::Gpu,
                AdapterStatus::HelperGap);
        }
        return false;
    }
    if (!GpuLiveResultHasRequiredPayload(step, timBytes)) {
        outFeedback = TypedActionFeedback{};
        if (out) {
            *out = MakeLiveHalFeedbackBuildResult(
                step,
                ProducerCategory::Gpu,
                AdapterStatus::IncompleteFeedback);
        }
        return false;
    }

    Direct::GpuSeamResult gpu =
        BuildGpuSeamResultFromTimBytes(step, timBytes);
    const bool success =
        gpu.timParseKnown &&
        gpu.timParse.status == PrStage1LoaderGpuHal::TimParseStatus::Ok &&
        !gpu.actionList.overflow;
    if (!success) {
        (void)BuildGpuTypedFeedback(step, gpu, false, outFeedback);
        outFeedback.feedback.completed = false;
        if (out) {
            *out = MakeLiveHalFeedbackBuildResult(
                step,
                ProducerCategory::Gpu,
                AdapterStatus::IncompleteFeedback,
                outFeedback);
        }
        return false;
    }
    if (!BuildGpuTypedFeedback(step, gpu, success, outFeedback)) {
        if (out) {
            *out = MakeLiveHalFeedbackBuildResult(
                step,
                ProducerCategory::Gpu,
                AdapterStatus::IncompleteFeedback,
                outFeedback);
        }
        return false;
    }
    if (step.requiredFields.resolvedPayload &&
        !outFeedback.feedback.hasResolvedPayload) {
        const Direct::ResolvedPayload payload =
            BuildResolvedPayloadFromGpuTimBytes(step, timBytes);
        if (payload.valid) {
            outFeedback.feedback.hasResolvedPayload = true;
            outFeedback.feedback.resolvedPayload = payload;
        }
    }
    if (!Direct::IsTypedActionFeedbackCompleteForProducerStep(step,
                                                              outFeedback)) {
        if (out) {
            *out = MakeLiveHalFeedbackBuildResult(
                step,
                ProducerCategory::Gpu,
                AdapterStatus::IncompleteFeedback,
                outFeedback);
        }
        return false;
    }
    if (out) {
        *out = MakeLiveHalFeedbackBuildResult(
            step,
            ProducerCategory::Gpu,
            AdapterStatus::Ready,
            outFeedback);
    }
    return true;
}

bool BuildGpuTimBytesLiveTypedFeedbackFromResolvedPayload(
    const Direct::RunnerState& state,
    const Direct::ProducerStep& step,
    TypedActionFeedback& outFeedback,
    LiveHalFeedbackBuildResult* out) {
    if (!step.valid || step.category != ProducerCategory::Gpu) {
        outFeedback = TypedActionFeedback{};
        if (out) {
            *out = MakeLiveHalFeedbackBuildResult(
                step,
                ProducerCategory::Gpu,
                AdapterStatus::HelperGap);
        }
        return false;
    }

    GpuTimBytesLiveInput timBytes{};
    if (!ResolveGpuTimBytesInputFromRunnerPayload(state, step, timBytes)) {
        outFeedback = TypedActionFeedback{};
        if (out) {
            *out = MakeLiveHalFeedbackBuildResult(
                step,
                ProducerCategory::Gpu,
                AdapterStatus::IncompleteFeedback);
        }
        return false;
    }

    return BuildGpuTimBytesLiveTypedFeedback(
        step,
        timBytes,
        outFeedback,
        out);
}

bool BuildSpuTypedFeedback(const Direct::ProducerStep& step,
                           const Direct::SpuSeamResult& spu,
                           bool success,
                           TypedActionFeedback& outFeedback) {
    if (!step.valid || step.category != ProducerCategory::Spu ||
        !spu.present) {
        outFeedback = TypedActionFeedback{};
        return false;
    }
    if (!BuildTypedFeedbackSkeleton(step, outFeedback)) {
        return false;
    }

    outFeedback.feedback.completed = true;
    outFeedback.feedback.success = success;
    outFeedback.feedback.hasSpuResult = true;
    outFeedback.feedback.spuResult = spu;
    if (step.requiredFields.resolvedPayload &&
        step.action.resolvedPayload.valid) {
        outFeedback.feedback.hasResolvedPayload = true;
        outFeedback.feedback.resolvedPayload = step.action.resolvedPayload;
    }
    return true;
}

bool BuildSpuHalResultLiveTypedFeedback(
    const Direct::ProducerStep& step,
    const Direct::SpuSeamResult& spu,
    bool success,
    TypedActionFeedback& outFeedback,
    LiveHalFeedbackBuildResult* out) {
    if (!step.valid || step.category != ProducerCategory::Spu) {
        outFeedback = TypedActionFeedback{};
        if (out) {
            *out = MakeLiveHalFeedbackBuildResult(
                step,
                ProducerCategory::Spu,
                AdapterStatus::HelperGap);
        }
        return false;
    }
    if (!SpuLiveResultHasRequiredPayload(step, spu)) {
        outFeedback = TypedActionFeedback{};
        if (out) {
            *out = MakeLiveHalFeedbackBuildResult(
                step,
                ProducerCategory::Spu,
                AdapterStatus::IncompleteFeedback);
        }
        return false;
    }
    if (!BuildSpuTypedFeedback(step, spu, success, outFeedback) ||
        !Direct::IsTypedActionFeedbackCompleteForProducerStep(step,
                                                              outFeedback)) {
        if (out) {
            *out = MakeLiveHalFeedbackBuildResult(
                step,
                ProducerCategory::Spu,
                AdapterStatus::IncompleteFeedback,
                outFeedback);
        }
        return false;
    }
    if (out) {
        *out = MakeLiveHalFeedbackBuildResult(
            step,
            ProducerCategory::Spu,
            AdapterStatus::Ready,
            outFeedback);
    }
    return true;
}

bool BuildSpuHalResultLiveTypedFeedbackFromLowerResult(
    const Direct::ProducerStep& step,
    PrStage1LoaderSpuHal::State& spuState,
    const SpuHalResultLiveInput& input,
    bool success,
    TypedActionFeedback& outFeedback,
    LiveHalFeedbackBuildResult* out) {
    if (!step.valid || step.category != ProducerCategory::Spu) {
        outFeedback = TypedActionFeedback{};
        if (out) {
            *out = MakeLiveHalFeedbackBuildResult(
                step,
                ProducerCategory::Spu,
                AdapterStatus::HelperGap);
        }
        return false;
    }

    Direct::SpuSeamResult spu{};
    if (!BuildSpuSeamResultFromLowerInput(step, spuState, input, spu)) {
        outFeedback = TypedActionFeedback{};
        if (out) {
            *out = MakeLiveHalFeedbackBuildResult(
                step,
                ProducerCategory::Spu,
                AdapterStatus::IncompleteFeedback);
        }
        return false;
    }

    return BuildSpuHalResultLiveTypedFeedback(
        step,
        spu,
        success,
        outFeedback,
        out);
}

bool BuildPadComTypedFeedback(const Direct::ProducerStep& step,
                              bool completed,
                              bool success,
                              int32_t result,
                              TypedActionFeedback& outFeedback) {
    if (!step.valid ||
        step.stepKind != Direct::ProducerStepKind::PadComBoundary) {
        outFeedback = TypedActionFeedback{};
        return false;
    }
    if (!BuildTypedFeedbackSkeleton(step, outFeedback)) {
        return false;
    }

    outFeedback.feedback.completed = completed;
    outFeedback.feedback.success = success;
    outFeedback.feedback.result = result;
    outFeedback.resultPresent = completed;
    return completed;
}

bool BuildCallback1537CTypedFeedback(
    const Direct::ProducerStep& step,
    const Direct::Callback1537CFeedback& callback,
    TypedActionFeedback& outFeedback) {
    if (!step.valid ||
        step.stepKind != Direct::ProducerStepKind::Callback1537C ||
        step.callbackKind != Direct::CallbackFeedbackKind::Callback1537C) {
        outFeedback = TypedActionFeedback{};
        return false;
    }
    if (!BuildTypedFeedbackSkeleton(step, outFeedback)) {
        return false;
    }

    Direct::CallbackSeamResult result{};
    result.present = true;
    result.kind = Direct::CallbackFeedbackKind::Callback1537C;
    result.callback1537C = callback;
    result.resultKnown = callback.resultKnown;
    result.result = callback.result;

    outFeedback.feedback.completed = true;
    outFeedback.feedback.success = callback.callbackInvoked;
    outFeedback.feedback.hasCallbackResult = true;
    outFeedback.feedback.callbackResult = result;
    outFeedback.feedback.result = callback.result;
    outFeedback.resultPresent = callback.resultKnown;
    return true;
}

bool BuildCallbackDraw1EA74TypedFeedback(
    const Direct::ProducerStep& step,
    const Direct::CallbackDraw1EA74Feedback& callback,
    TypedActionFeedback& outFeedback) {
    if (!step.valid ||
        step.stepKind != Direct::ProducerStepKind::CallbackDraw1EA74 ||
        step.callbackKind !=
            Direct::CallbackFeedbackKind::CallbackDraw1EA74) {
        outFeedback = TypedActionFeedback{};
        return false;
    }
    if (!BuildTypedFeedbackSkeleton(step, outFeedback)) {
        return false;
    }

    Direct::CallbackSeamResult result{};
    result.present = true;
    result.kind = Direct::CallbackFeedbackKind::CallbackDraw1EA74;
    result.draw1EA74 = callback;
    result.resultKnown = callback.resultKnown;
    result.result = callback.result;

    outFeedback.feedback.completed = true;
    outFeedback.feedback.success = callback.drawSubmitted;
    outFeedback.feedback.hasCallbackResult = true;
    outFeedback.feedback.callbackResult = result;
    outFeedback.feedback.result = callback.result;
    outFeedback.resultPresent = callback.resultKnown;
    return true;
}

bool BuildCallbackFlush26ECCTypedFeedback(
    const Direct::ProducerStep& step,
    const Direct::CallbackFlush26ECCFeedback& callback,
    TypedActionFeedback& outFeedback) {
    if (!step.valid ||
        step.stepKind != Direct::ProducerStepKind::CallbackFlush26ECC ||
        step.callbackKind !=
            Direct::CallbackFeedbackKind::CallbackFlush26ECC) {
        outFeedback = TypedActionFeedback{};
        return false;
    }
    if (!BuildTypedFeedbackSkeleton(step, outFeedback)) {
        return false;
    }

    Direct::CallbackSeamResult result{};
    result.present = true;
    result.kind = Direct::CallbackFeedbackKind::CallbackFlush26ECC;
    result.flush26ECC = callback;
    result.resultKnown = callback.resultKnown;
    result.result = callback.result;

    outFeedback.feedback.completed = true;
    outFeedback.feedback.success = callback.audioFlushSubmitted;
    outFeedback.feedback.hasCallbackResult = true;
    outFeedback.feedback.callbackResult = result;
    outFeedback.feedback.result = callback.result;
    outFeedback.resultPresent = callback.resultKnown;
    return true;
}

bool ApplyTypedFeedback(Direct::RunnerState& state,
                        const TypedActionFeedback& feedback) {
    return Direct::ApplyTypedFeedback(state, feedback, nullptr);
}

} // namespace PrStage1LoaderProducerAdapter

