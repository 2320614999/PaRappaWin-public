#pragma once

#include "pr_stage1_loader_cd_hal.h"
#include "pr_stage1_loader_bootstrap_plan_direct.h"
#include "pr_stage1_loader_gpu_hal.h"
#include "pr_stage1_loader_memory_direct.h"
#include "pr_stage1_loader_spu_hal.h"
#include "pr_movie_segment_direct.h"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace PrStage1LoaderDirect {

using Bootstrap15590Plan =
    PrStage1LoaderBootstrapPlanDirect::Bootstrap15590DirectPlan801C81EC;

enum class RunnerStatus : uint8_t {
    Idle,
    Ready,
    Running,
    Completed,
    Failed,
};

enum class LoaderRecordType : int8_t {
    End = -1,
    Unknown = 0,
    Type1Tim = 1,
    Type2Vab = 2,
    Type3Split = 3,
};

enum class ActionBoundary : uint8_t {
    DirectControl,
    LoaderMemoryDirect,
    CdHalBoundary,
    GpuHalBoundary,
    SpuHalBoundary,
    PadComHalBoundary,
    CallbackHalBoundary,
    Completion,
};

enum class CallbackFeedbackKind : uint8_t {
    None,
    Callback1537C,
    CallbackDraw1EA74,
    CallbackFlush26ECC,
};

enum class ActionKind : uint8_t {
    None,
    BeginBootstrap15590,
    StartCurtain15408,
    Callback1537C,
    CallbackDraw1EA74,
    CallbackFlush26ECC,
    BeginLoader1AC18,
    Reset25A34,
    OpenFile1A324,
    CopyDescriptorWords1AC18,
    BeginParse1A8F0,
    Seek1A89C,
    ReadHeader1A818,
    DispatchRecord1A8F0,
    TempAlloc25B28,
    StackAlloc25A70,
    ReadPayload1A818,
    TimUpload1AE7C,
    TimGetInfo40EAC,
    TimLoadImage44D64,
    TimDrawSync44B3C,
    VabClose27120,
    PadStartCom26E4C,
    VabOpen27078,
    VabTransfer270D4,
    VabEnable270FC,
    RetrySfxReset26FA4,
    RetrySfxCue26EF8,
    RetrySfxFlush26ECC,
    StackFind25BFC,
    StackSplit25BBC,
    StackFree25AF8,
    RecordEnd1A8F0,
    UnknownRecord1A8F0,
    ParserSuccess1A8F0,
    ParserFailure1A8F0,
    RetryNext1AC18,
    StopCurtain1545C,
    Complete15590,
};

struct RecordData {
    bool valid = false;
    bool recordTypeKnown = false;
    LoaderRecordType recordType = LoaderRecordType::Unknown;
    uint16_t recordIndex = 0;
    uint16_t recordCount = 0;
    bool recordCountKnown = false;

    uint8_t recordCountWord = 0;
    uint8_t sectorCountWord = 0;
    uint8_t payloadFirstWord = 0;
    uint8_t payloadBytesWord = 0;
    uint8_t spuBytesWord = 0;
    uint8_t recordStrideWords = 0;

    uint32_t payloadPsxAddress = 0;
    uint32_t payloadBytes = 0;
    uint32_t sectorCount = 0;
    uint32_t startSector = 0;
    uint32_t spuBytes = 0;

    bool payloadPsxAddressKnown = false;
    bool payloadBytesKnown = false;
    bool sectorCountKnown = false;
    bool startSectorKnown = false;
    bool spuBytesKnown = false;
    bool livePayloadBytesKnown = false;
};

struct ResolvedPayload {
    bool valid = false;
    uint16_t recordIndex = 0;
    LoaderRecordType recordType = LoaderRecordType::Unknown;
    uint32_t psxAddress = 0;
    uint32_t sizeBytes = 0;
    uint32_t sectorCount = 0;
    const uint8_t* liveBytesData = nullptr;
    std::size_t liveBytesSize = 0;
    bool psxAddressKnown = false;
    bool sizeBytesKnown = false;
    bool sectorCountKnown = false;
    bool liveBytesPresent = false;
    bool liveBytesSizeKnown = false;
};

struct CdSeamPayload {
    PrStage1LoaderCdHal::ActionKind actionKind =
        PrStage1LoaderCdHal::ActionKind::None;
    uint32_t callerFunction = 0;
    uint32_t directHelperFunction = 0;
    uint32_t lowerFunction = 0;
    uint32_t finalFunction = 0;

    uint32_t cdlFilePtr = 0;
    uint32_t pathPtr = 0;
    uint32_t msfTargetPtr = 0;
    uint32_t dstPtr = 0;

    int32_t lba = 0;
    int32_t sectorCount = 0;
    int32_t modeFlag = 0;
    int32_t arg0 = 0;
    int32_t arg1 = 0;
    int32_t arg2 = 0;

    uint8_t descriptorPresentWord = 0;
    uint8_t descriptorPayloadFirstWord = 0;
    uint8_t sizeBytesWord = 0;
    uint8_t startSectorWord = 0;
    uint8_t endSectorWord = 0;
    uint8_t sectorShift = 0;
    uint8_t sectorCountWord = 0;
    uint8_t dstPtrWord = 0;
    bool modeFlagFromParserFlag = false;
    bool recordDataRequired = false;
};

struct CdOverlayTransferAttemptAttribution {
    bool known = false;
    uint32_t sourceFunction = 0;
    uint32_t transferFunction = 0;
    bool attemptIndexKnown = false;
    uint32_t attemptIndex = 0;
    bool rowAddrKnown = false;
    uint32_t rowAddr = 0;
    bool dstKnown = false;
    uint32_t dst = 0;
    bool sectorCountKnown = false;
    uint32_t sectorCount = 0;
};

struct CdSeamResult {
    bool present = false;
    PrStage1LoaderCdHal::Feedback feedback{};
    PrStage1LoaderCdHal::LowerActionRequestMetadata lowerRequest{};
    CdOverlayTransferAttemptAttribution overlayTransferAttempt{};
    PrStage1LoaderCdHal::LookupFeedback800381F8 lookup800381F8{};
    bool lookup800381F8Known = false;
    PrStage1LoaderCdHal::ProbeCompletionFeedback8001A2B0 probe8001A2B0{};
    bool probe8001A2B0Known = false;
    uint32_t descriptorPtr = 0;
    bool descriptorPtrKnown = false;
    uint32_t dstPtr = 0;
    bool dstPtrKnown = false;
    int32_t sectorCount = 0;
    bool sectorCountKnown = false;
    const uint8_t* livePayloadData = nullptr;
    std::size_t livePayloadSize = 0;
    bool livePayloadBytesKnown = false;
    PrMovieSegmentDirect::StreamClockProducerCarrier800493F4
        streamClock800493F4{};
    bool streamClock800493F4Known = false;
    bool gapMissingStreamClock800493F4Producer = false;
};

struct MemorySeamPayload {
    uint32_t directFunction = 0;
    int32_t sizeBytes = 0;
    uint8_t sizeBytesWord = 0;
    uint8_t sectorCountWord = 0;
    uint8_t sectorToBytesShift = 0;
    uint32_t psxAddress = 0;
    uint8_t psxAddressWord = 0;
    int32_t stackIndex = 0;
    uint8_t freeCount = 0;
    bool sizeFromSectorCountWord = false;
    bool addressFromRecordPayloadWord = false;
    bool stackIndexFromFindFeedback = false;
    bool recordDataRequired = false;
};

struct MemorySeamResult {
    bool present = false;
    uint32_t directFunction = 0;
    uint32_t tailFunction = 0;
    PrStage1LoaderMemoryDirectAllocResult alloc{};
    PrStage1LoaderMemoryDirectFreeResult freeResult{};
    PrStage1LoaderMemoryDirectSplitResult split{};
    int32_t findStackIndex = 0;
    bool resetKnown = false;
    bool resetApplied = false;
    bool allocKnown = false;
    bool freeKnown = false;
    bool splitKnown = false;
    bool findKnown = false;
};

struct GpuSeamPayload {
    PrStage1LoaderGpuHal::GpuActionKind actionKind =
        PrStage1LoaderGpuHal::GpuActionKind::None;
    PrStage1LoaderGpuHal::GpuUploadSection uploadSection =
        PrStage1LoaderGpuHal::GpuUploadSection::None;
    uint32_t directFunction = 0;
    uint32_t psxFunction = 0;
    uint32_t lowerFunction = 0;
    uint8_t timPayloadWord = 0;
    uint32_t timPayloadPtr = 0;
    uint32_t timPayloadBytes = 0;
    std::size_t timInfoInputOffset = 0;
    PrStage1LoaderGpuHal::PsxRect rect{};
    uint32_t dataPtr = 0;
    uint32_t dataBytes = 0;
    int32_t drawSyncMode = 0;
    bool timPayloadFromRecord = false;
    bool parsedTimInfoRequired = false;
    bool recordDataRequired = false;
};

struct GpuSeamResult {
    bool present = false;
    PrStage1LoaderGpuHal::GpuActionKind actionKind =
        PrStage1LoaderGpuHal::GpuActionKind::None;
    PrStage1LoaderGpuHal::GpuActionList8001AE7C actionList{};
    PrStage1LoaderGpuHal::TimParseResult timParse{};
    bool actionListKnown = false;
    bool timParseKnown = false;
};

struct SpuSeamPayload {
    PrStage1LoaderSpuHal::ActionKind actionKind =
        PrStage1LoaderSpuHal::ActionKind::None;
    uint32_t wrapperFunction = 0;
    uint32_t lowerFunction = 0;
    uint32_t vhPtr = 0;
    uint32_t vbPtr = 0;
    uint8_t vhPtrWord = 0;
    uint8_t vbPtrWord = 0;
    uint8_t spuBytesWord = 0;
    uint32_t cuePointer = 0;
    PrStage1LoaderSpuHal::Cue4 cue{};
    bool cueFromRetryTable = false;
    bool lowerResultFeedbackRequired = false;
    bool recordDataRequired = false;
};

struct SpuSeamResult {
    bool present = false;
    PrStage1LoaderSpuHal::ActionKind actionKind =
        PrStage1LoaderSpuHal::ActionKind::None;
    PrStage1LoaderSpuHal::ActionList actionList{};
    int32_t lowerResult = 0;
    bool actionListKnown = false;
    bool lowerResultKnown = false;
};

struct CallbackSeamPayload {
    CallbackFeedbackKind kind = CallbackFeedbackKind::None;
    uint32_t callbackFunction = 0;
    uint32_t drawFunction = 0;
    uint32_t flushFunction = 0;
    int32_t drawArg0 = 0;
    int32_t drawArg1 = 0;
    int32_t flagValue = 0;
    int32_t returnValue = 0;
};

struct Callback1537CFeedback {
    bool callbackInvoked = false;
    bool flagValueKnown = false;
    int32_t flagValue = 0;
    bool resultKnown = false;
    int32_t result = 0;
};

struct CallbackDraw1EA74Feedback {
    bool drawSubmitted = false;
    bool argsKnown = false;
    int32_t arg0 = 0;
    int32_t arg1 = 0;
    bool resultKnown = false;
    int32_t result = 0;
};

struct CallbackFlush26ECCFeedback {
    bool audioFlushSubmitted = false;
    bool resultKnown = false;
    int32_t result = 0;
};

struct CallbackSeamResult {
    bool present = false;
    CallbackFeedbackKind kind = CallbackFeedbackKind::None;
    Callback1537CFeedback callback1537C{};
    CallbackDraw1EA74Feedback draw1EA74{};
    CallbackFlush26ECCFeedback flush26ECC{};
    bool resultKnown = false;
    int32_t result = 0;
};

struct SeamResultCarrier {
    bool present = false;
    uint32_t psxOrder = 0;
    ActionKind kind = ActionKind::None;
    ActionBoundary boundary = ActionBoundary::DirectControl;
    uint16_t recordIndex = 0;
    LoaderRecordType recordType = LoaderRecordType::Unknown;
    bool resolvedPayloadKnown = false;
    ResolvedPayload resolvedPayload{};
    CdSeamResult cd{};
    MemorySeamResult memory{};
    GpuSeamResult gpu{};
    SpuSeamResult spu{};
    CallbackSeamResult callback{};
};

struct Action {
    ActionKind kind = ActionKind::None;
    ActionBoundary boundary = ActionBoundary::DirectControl;
    uint32_t psxOrder = 0;
    uint32_t psxFunction = 0;
    uint32_t lowerFunction = 0;
    uint8_t attemptIndex = 0;
    uint16_t recordIndex = 0;
    LoaderRecordType recordType = LoaderRecordType::Unknown;
    int32_t parserFlag = 0;
    int32_t loaderMode = 0;
    uint8_t descriptorFirstWord = 0;
    uint8_t descriptorWordCount = 0;
    uint32_t byteCount = 0;
    uint8_t sectorCountWord = 0;
    uint8_t payloadBytesWord = 0;
    uint8_t spuBytesWord = 0;
    uint8_t freeCount = 0;
    bool branchTemplate = false;
    bool requiresRecordData = false;
    bool payloadResolved = false;
    bool recordDataResolved = false;
    bool blocksForFeedback = false;
    bool halBoundary = false;
    bool completesRunner = false;
    RecordData recordData{};
    ResolvedPayload resolvedPayload{};
    CdSeamPayload cd{};
    MemorySeamPayload memory{};
    GpuSeamPayload gpu{};
    SpuSeamPayload spu{};
    CallbackSeamPayload callback{};
};

struct RunnerState {
    RunnerStatus status = RunnerStatus::Idle;
    Bootstrap15590Plan plan;
    bool planKnown = false;
    std::vector<Action> actions;
    std::size_t nextActionIndex = 0;
    bool waitingForFeedback = false;
    Action feedbackAction;
    bool currentRecordTypeKnown = false;
    LoaderRecordType currentRecordType = LoaderRecordType::Unknown;
    bool currentRecordDataKnown = false;
    RecordData currentRecordData{};
    bool currentResolvedPayloadKnown = false;
    ResolvedPayload currentResolvedPayload{};
    std::vector<RecordData> records;
    std::vector<ResolvedPayload> resolvedPayloads;
    std::vector<SeamResultCarrier> seamResults;
    bool recordLoopActive = false;
    bool recordLoopPendingDispatch = false;
    Action recordLoopDispatchAction{};
    std::size_t recordBranchBeginIndex = 0;
    std::size_t recordBranchEndIndex = 0;
    std::size_t recordAfterBranchIndex = 0;
    uint16_t nextRecordIndex = 0;
    bool parserTailSelectionKnown = false;
    ActionKind parserTailSelection = ActionKind::None;
    bool completionKnown = false;
    int32_t completionResult = 0;
};

struct ActionFeedback {
    uint32_t psxOrder = 0;
    ActionKind kind = ActionKind::None;
    bool completed = false;
    bool success = false;
    bool hasRecordType = false;
    LoaderRecordType recordType = LoaderRecordType::Unknown;
    bool hasRecordData = false;
    RecordData recordData{};
    bool hasResolvedPayload = false;
    ResolvedPayload resolvedPayload{};
    bool hasCdResult = false;
    CdSeamResult cdResult{};
    bool hasMemoryResult = false;
    MemorySeamResult memoryResult{};
    bool hasGpuResult = false;
    GpuSeamResult gpuResult{};
    bool hasSpuResult = false;
    SpuSeamResult spuResult{};
    bool hasCallbackResult = false;
    CallbackSeamResult callbackResult{};
    int32_t result = 0;
};

struct ActionProducerRequirements {
    bool waitsForFeedback = false;
    bool needsRecordData = false;
    bool needsResolvedPayload = false;
    bool needsCdFeedback = false;
    bool needsMemoryFeedback = false;
    bool needsGpuFeedback = false;
    bool needsSpuFeedback = false;
    bool needsPadComFeedback = false;
    bool needsCallbackFeedback = false;
    bool completesRunner = false;
};

enum class ProducerStepKind : uint8_t {
    None,
    RecordDispatch1A8F0,
    CdReadPayload1A818,
    CdBoundary,
    TimUpload1AE7C,
    TimGpuBoundary,
    VabSfxBoundary,
    MemoryBoundary,
    PadComBoundary,
    CallbackBoundary,
    Callback1537C,
    CallbackDraw1EA74,
    CallbackFlush26ECC,
    Completion,
    DirectControl,
};

enum class ProducerCategory : uint8_t {
    None,
    Record,
    Cd,
    Memory,
    Gpu,
    Spu,
    PadCom,
    Callback,
    Completion,
    DirectControl,
    Unknown,
};

struct ProducerFeedbackFields {
    bool completed = true;
    bool success = true;
    bool result = false;
    bool recordType = false;
    bool recordData = false;
    bool resolvedPayload = false;
    bool cdResult = false;
    bool memoryResult = false;
    bool gpuResult = false;
    bool spuResult = false;
    bool callbackResult = false;
};

struct ProducerStep {
    bool valid = false;
    ProducerStepKind stepKind = ProducerStepKind::None;
    Action action{};
    ActionProducerRequirements requirements{};
    ProducerFeedbackFields requiredFields{};
    ActionFeedback feedbackSkeleton{};
    ProducerCategory category = ProducerCategory::None;
    bool externalProducerRequired = false;
    bool helperGap = false;
    bool incompleteFeedback = false;
    bool canApplyFeedback = false;

    ActionKind actionKind = ActionKind::None;
    ActionBoundary boundary = ActionBoundary::DirectControl;
    uint32_t psxOrder = 0;
    uint32_t psxFunction = 0;
    uint32_t directFunction = 0;
    uint32_t lowerFunction = 0;
    uint16_t recordIndex = 0;
    LoaderRecordType recordType = LoaderRecordType::Unknown;

    PrStage1LoaderCdHal::ActionKind cdActionKind =
        PrStage1LoaderCdHal::ActionKind::None;
    PrStage1LoaderGpuHal::GpuActionKind gpuActionKind =
        PrStage1LoaderGpuHal::GpuActionKind::None;
    PrStage1LoaderSpuHal::ActionKind spuActionKind =
        PrStage1LoaderSpuHal::ActionKind::None;
    CallbackFeedbackKind callbackKind = CallbackFeedbackKind::None;
};

enum class TypedFeedbackApplyStatus : uint8_t {
    None,
    NoWaitingStep,
    IncompleteFeedback,
    HelperGap,
    ApplyRejected,
    Applied,
};

struct TypedActionFeedback {
    bool valid = false;
    ProducerCategory category = ProducerCategory::None;
    ProducerStepKind stepKind = ProducerStepKind::None;
    CallbackFeedbackKind callbackKind = CallbackFeedbackKind::None;
    ActionFeedback feedback{};

    bool recordFeedback = false;
    bool cdFeedback = false;
    bool memoryFeedback = false;
    bool gpuFeedback = false;
    bool spuFeedback = false;
    bool padComFeedback = false;
    bool callbackFeedback = false;
    bool completionFeedback = false;
    bool resultPresent = false;
};

struct TypedFeedbackApplyResult {
    TypedFeedbackApplyStatus status = TypedFeedbackApplyStatus::None;
    bool applied = false;
    bool waitingStepValid = false;
    bool feedbackComplete = false;
    ProducerStep waitingStep{};
};

void Reset(RunnerState& state);

bool Begin(RunnerState& state, const Bootstrap15590Plan& plan);

std::vector<Action> BuildActionSkeleton(const Bootstrap15590Plan& plan);

bool HasPendingAction(const RunnerState& state);
bool PopNextAction(RunnerState& state, Action& out);

bool ApplyFeedback(RunnerState& state,
                   const Action& action,
                   const ActionFeedback& feedback);
bool IsTypedActionFeedbackCompleteForProducerStep(
    const ProducerStep& step,
    const TypedActionFeedback& feedback);
bool ApplyTypedFeedback(RunnerState& state,
                        const TypedActionFeedback& feedback,
                        TypedFeedbackApplyResult* out = nullptr);

bool IsTerminalStatus(RunnerStatus status);
bool IsHalBoundary(ActionBoundary boundary);
bool ActionNeedsFeedback(const Action& action);
bool ActionNeedsRecordData(const Action& action);
bool ActionNeedsResolvedPayload(const Action& action);
bool ActionNeedsCdFeedback(const Action& action);
bool ActionNeedsMemoryFeedback(const Action& action);
bool ActionNeedsGpuFeedback(const Action& action);
bool ActionNeedsSpuFeedback(const Action& action);
ActionProducerRequirements GetActionProducerRequirements(
    const Action& action);
bool IsWaitingForFeedback(const RunnerState& state);
bool PeekWaitingAction(const RunnerState& state, Action& out);
ActionFeedback BuildFeedbackSkeletonForAction(const Action& action);
bool DescribeWaitingProducerStep(const RunnerState& state, ProducerStep& out);
bool IsActionFeedbackCompleteForProducerStep(
    const ProducerStep& step,
    const ActionFeedback& feedback,
    bool resultPresent = false);
bool CdSeamResultHasRequiredPayloadForProducerStep(
    const ProducerStep& step,
    const CdSeamResult& cd);
bool CdFeedbackKindCompletesProducerStep(
    const ProducerStep& step,
    PrStage1LoaderCdHal::ActionKind kind);
bool BuildCdActionFeedbackForProducerStep(
    const ProducerStep& step,
    const CdSeamResult& cd,
    bool success,
    ActionFeedback& out);
bool BuildMemorySeamResultForProducerStep(
    const RunnerState& state,
    PrStage1LoaderMemoryDirectState& memoryState,
    const ProducerStep& step,
    MemorySeamResult& out);
bool MemorySeamResultSucceededForAction(
    const Action& action,
    const MemorySeamResult& result);
bool HasCdSeamPayload(const Action& action);
bool HasMemorySeamPayload(const Action& action);
bool HasGpuSeamPayload(const Action& action);
bool HasSpuSeamPayload(const Action& action);
bool TryGetCurrentRecordData(const RunnerState& state, RecordData& out);
bool TryGetCurrentResolvedPayload(const RunnerState& state,
                                  ResolvedPayload& out);
bool TryGetResolvedPayloadForRecord(const RunnerState& state,
                                    uint16_t recordIndex,
                                    ResolvedPayload& out);
bool TryBuildRecordDataFromDescriptorPayload(const RunnerState& state,
                                             uint16_t recordIndex,
                                             RecordData& out);
bool TryGetLastSeamResult(const RunnerState& state, SeamResultCarrier& out);
bool TryGetLastSeamResultForRecord(const RunnerState& state,
                                   uint16_t recordIndex,
                                   SeamResultCarrier& out);
CdSeamResult NormalizeCdSeamResultStreamClock800493F4(
    const CdSeamResult& result);
CdSeamResult AttachStreamClockProducerCarrierToCdSeamResult800493F4(
    const CdSeamResult& cd,
    const PrMovieSegmentDirect::StreamClockProducerCarrier800493F4& carrier);

} // namespace PrStage1LoaderDirect
