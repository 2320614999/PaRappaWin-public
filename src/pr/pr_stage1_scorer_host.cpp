#include "pr_stage1_scorer_host_deps.h"
#include "pr_stage1_scorer_host_direct.h"
#include "pr_pad.h"
#include "pr_psx_pad_direct.h"
#include "pr_stage_event_direct.h"
#include "pr_stage1_rating_presentation_direct.h"
#include "pr_stage1_runtime_slots_direct.h"
#include "pr_stage1_save_ui_direct.h"
#include "pr_stage_runner_direct.h"
#include "pr_sfx.h"

#include <algorithm>

namespace PrScn1 {

// Host-only declarations needed by anonymous-namespace helpers below.
size_t ResolveStage1PageRecordRawOffsetForSlotIndex(size_t slotIndex);
uint16_t ReadStage1PageRecordRawU16(
    const Stage1NumericRuntimeState::PageRecordMirrorPage& page,
    size_t byteOffset);
uint32_t ReadStage1PageRecordRawU32(
    const Stage1NumericRuntimeState::PageRecordMirrorPage& page,
    size_t byteOffset);
void WriteStage1PageRecordRawU16(
    Stage1NumericRuntimeState::PageRecordMirrorPage& page,
    size_t byteOffset,
    uint16_t value);
void WriteStage1PageRecordRawU32(
    Stage1NumericRuntimeState::PageRecordMirrorPage& page,
    size_t byteOffset,
    uint32_t value);
uint16_t ResolveStage1PageRecordSlotOccupiedCount(
    const Stage1NumericRuntimeState::PageRecordMirrorPage& page,
    size_t slotIndex);
uint32_t ResolveStage1PageRecordSlotMask(
    const Stage1NumericRuntimeState::PageRecordMirrorPage& page,
    size_t slotIndex);
uint16_t ResolveStage1PageRecordSlotClassToken(
    const Stage1NumericRuntimeState::PageRecordMirrorPage& page,
    size_t slotIndex);
uint32_t ResolveStage1PageRecordSlotSourceCellPtr(
    const Stage1NumericRuntimeState::PageRecordMirrorPage& page,
    size_t slotIndex);
uint8_t ResolveStage1AcceptedProducerSubstate50(
    const Stage1NumericRuntimeState& state);
uint8_t ResolveStage1AcceptedProducerHalfWindow34(
    const PrGameContext& ctx,
    const Stage1NumericRuntimeState& state);
int ResolveStage1AcceptedProducerWritePageOrdinal38(
    const Stage1NumericRuntimeState& state,
    int tick96);
uint16_t ResolveStage1AcceptedProducerObservedRawSample18(
    const PrGameContext& ctx,
    const Stage1NumericRuntimeState::AcceptedProducerCarrierRuntime& carrier,
    uint16_t heldMask);
PrStage1ScorerDirectDescriptorRow BuildStage1DirectPortDescriptorRowAdapter(
    const Stage1NumericRuntimeState::DescriptorRowView& rowView);
PrStage1ScorerDirectGlobals BuildStage1DirectPortGlobalsAdapter(
    const Stage1NumericRuntimeState& state);
void StoreStage1DirectPortAcceptedProducerNonPageGlobals14614(
    Stage1NumericRuntimeState& state,
    const PrStage1ScorerDirectGlobals& globals);
const Stage1NumericRuntimeState::DescriptorRowView*
ResolveStage1FormalScorerCommittedDescriptorRow(
    const Stage1NumericRuntimeState& runtime);
Stage1FormalScorerRuntimeState ConvertStage1DirectPortRuntimeStateAdapter(
    const PrStage1ScorerDirectCommitState& direct);
Stage1FormalScorerCommitRuntimeState ConvertStage1DirectPortCommitStateAdapter(
    const PrStage1ScorerDirectCommitState& direct);
Stage1FormulaAccumulatorWritebackState ResolveStage1FormulaAccumulatorWriteback(
    int32_t formulaAccumulatorBaselineValue,
    int32_t commitTermValue);
void AdvanceStage1FormulaAccumulatorBaseline(Stage1NumericRuntimeState& state,
                                             int32_t nextBaselineValue);

namespace {

constexpr size_t kStage1ReplayPayloadPrevGrade92F40Offset = 0x30u;
constexpr size_t kStage1ReplayPayloadStatus92F1DOffset = 0x0Du;
constexpr size_t kStage1ReplayPayloadStatusSlotCount = 6u;
constexpr uint32_t kStage1SourceCellSelectorTableAddr = 0x801CCF34u;
constexpr size_t kStage1SourceCellSelectorStride = 36u;
constexpr uint32_t kStage1TimingTemplateTableAddr = 0x801CD2BCu;
constexpr size_t kStage1TimingTemplateSelectorStride = 16u;
constexpr uint32_t kStage1DescriptorMainTableAddr = 0x801CD38Cu;
constexpr size_t kStage1DescriptorMainTableRowSize = 24u;
constexpr size_t kStage1DescriptorMainTableRowCount = 0x42u;
constexpr int32_t kStage1LateBranchGoodToCoolFirstDelayTick96 = 0x610;
constexpr int32_t kStage1LateBranchGoodToCoolRepeatDelayTick96 = 0x310;
constexpr int kStage1LateBranchDynamicCarryProbePersistFrames = 24;
constexpr uint32_t kStage1EventStreamFlag80Ed1C = 0x00000080u;
constexpr size_t kStage1SourceCellHeaderSize = 8u;
constexpr size_t kStage1SourceCellSize = 12u;
constexpr size_t kStage1TopLessonPairInitialLookaheadRowIndex = 2u;
constexpr uint32_t kStage1RunnerCtxFlag0200Dispatch = 0x0200u;
constexpr size_t kStage1PageRecordRawRecordSizeBytes = 12u;
constexpr size_t kStage1PageRecordRawSlotsPerPage =
    384u / kStage1PageRecordRawRecordSizeBytes;
constexpr uint16_t kStage1GameplayAcceptedInputMask =
    (uint16_t)PrPadButton::Left |
    (uint16_t)PrPadButton::Right |
    (uint16_t)PrPadButton::Triangle |
    (uint16_t)PrPadButton::Circle |
    (uint16_t)PrPadButton::Cross |
    (uint16_t)PrPadButton::Square;
constexpr uint32_t kStage1RunnerCtxFlagTickAdvance = 0x00000008u;
constexpr uint32_t kStage1RunnerCtxFlag0040Branch = 0x00000040u;
constexpr uint32_t kStage1RunnerCtxFlagTerminalStreamDone = 0x00000100u;
constexpr uint32_t kStage1RunnerCtxFlag2000SceneFamily = 0x00002000u;

uint32_t BuildStage1RunnerCtxFlagsAfterFrameUpdate7A60(
    const Stage1NumericRuntimeState& state,
    bool ctxAcceptedInputFlagSet) {
    const PrStageRunnerDirectCtx0FlagsSnapshot801C3640 ctx0Base =
        PrStageRunnerDirectMakeCtx0BaseFlagsSnapshot801C7A60(
            ctxAcceptedInputFlagSet);
    uint32_t flags = ctx0Base.baseKnown ? ctx0Base.baseFlags : 0u;
    if (state.bucketCadence.ctxFlagTickAdvanceKnown801C9094 &&
        state.bucketCadence.ctxFlagTickAdvance801C9094) {
        flags |= kStage1RunnerCtxFlagTickAdvance;
    }
    if (state.rightRankHelperShadow.bucket0Ctx118WritePulse) {
        flags |= kStage1RunnerCtxFlag0040Branch;
    }
    if (state.rightRankDirectFollowUp.flag0200Pulse) {
        flags |= kStage1RunnerCtxFlag0200Dispatch;
    }
    if (state.rightRankDirectFollowUp.flag2000Pulse) {
        flags |= kStage1RunnerCtxFlag2000SceneFamily;
    }
    return flags;
}

uint32_t ResolveStage1RunnerCtxFlagsForPostFrame7A60(
    const Stage1NumericRuntimeState& state,
    bool ctxAcceptedInputFlagSet) {
    const Stage1NumericRuntimeState::RunnerPostFrame7A60Runtime& runtime =
        state.runnerPostFrame7A60;
    const bool terminalFlag100Pulse =
        state.psxFlag100BlocksWaitPulse &&
        state.psxFlag100BlocksWaitSourceStream >= 2u &&
        state.psxFlag100BlocksWaitSourceStream <= 5u;
    if (runtime.word0Flags801C9094FinalKnown) {
        uint32_t flags = runtime.word0Flags801C9094Final;
        if (terminalFlag100Pulse) {
            flags |= kStage1RunnerCtxFlagTerminalStreamDone;
        }
        return flags;
    }
    uint32_t flags = BuildStage1RunnerCtxFlagsAfterFrameUpdate7A60(
        state,
        ctxAcceptedInputFlagSet);
    const Stage1NumericRuntimeState::RunnerSameFrameCtxOwner801C9094Runtime&
        frameUpdateCtx = runtime.sameFrameCtx801C9094;
    if (frameUpdateCtx.snapshotAvailable &&
        frameUpdateCtx.sameFrameCtxKnown &&
        (frameUpdateCtx.projectedCtx.flags00 &
         kStage1RunnerCtxFlagTerminalStreamDone) != 0u) {
        flags |= kStage1RunnerCtxFlagTerminalStreamDone;
    }
    if (terminalFlag100Pulse) {
        flags |= kStage1RunnerCtxFlagTerminalStreamDone;
    }
    return flags;
}

PrStageRunnerDirectContext801C9094 BuildStage1RunnerSameFrameCtx801C9094(
    const Stage1NumericRuntimeState& state,
    const Stage1RunnerTimingRoots30& timing,
    bool ctxAcceptedInputFlagSet) {
    const Stage1NumericRuntimeState::RunnerSameFrameCtxOwner801C9094Runtime&
        previousCtx = state.runnerPostFrame7A60.sameFrameCtx801C9094;
    PrStageRunnerDirectContext801C9094 out =
        previousCtx.snapshotAvailable ? previousCtx.projectedCtx
                                      : PrStageRunnerDirectContext801C9094{};
    out.flags00 = BuildStage1RunnerCtxFlagsAfterFrameUpdate7A60(
        state,
        ctxAcceptedInputFlagSet);
    out.tick0C = timing.tick96;
    out.word4E =
        static_cast<uint16_t>((std::min<uint8_t>)(
            state.rightRankState.rightRankActiveRow,
            3u));
    out.word54 = state.rightRankBucketContext.ctx54Permit ? 1u : 0u;
    out.word72 = state.rightRankDirectFollowUp.ctx72;
    out.word72SceneFamily = state.rightRankDirectFollowUp.ctx72;
    out.word72RightRank = out.word4E;
    out.word72Rating = state.rightRankBucketContext.transitionAnim18E;
    out.wordE4 =
        static_cast<uint16_t>(ResolveStage1RightRankDescriptorFlagWord(state));
    return out;
}

PrStageRunnerDirectGlobals801C9094 BuildStage1RunnerSameFrameGlobals801C9094(
    const Stage1NumericRuntimeState& state) {
    const Stage1NumericRuntimeState::RunnerSameFrameGlobalsOwner801C9094Runtime&
        previousGlobals = state.runnerPostFrame7A60.sameFrameGlobals801C9094;
    PrStageRunnerDirectGlobals801C9094 out =
        previousGlobals.snapshotAvailable
            ? previousGlobals.projectedGlobals
            : PrStageRunnerDirectGlobals801C9094{};
    out.unk8008ED00 =
        static_cast<uint16_t>(state.rightRankDirectFollowUp.ed00);
    out.eventStreamFlag =
        state.psxEventStreamFlagKnown && state.psxEventStreamFlagActive
            ? 1u
            : 0u;
    out.eventStreamId =
        state.psxEventStreamIdKnown ? state.psxEventStreamId : 0u;
    out.eventStreamDone = state.rightRankForcedGoodEventStreamDone28;
    out.dword801CCBB8 = state.runnerPostFramePreviousInputMask801CCBB8;
    return out;
}

void CaptureStage1RunnerSameFrame801C9094InputSnapshot(
    Stage1NumericRuntimeState& state,
    const Stage1RunnerTimingRoots30& timing,
    bool sameFrameCtxKnown,
    const PrStageRunnerDirectContext801C9094& sameFrameCtx,
    bool sameFrameGlobalsKnown,
    const PrStageRunnerDirectGlobals801C9094& sameFrameGlobals) {
    Stage1NumericRuntimeState::RunnerPostFrame7A60Runtime& runtime =
        state.runnerPostFrame7A60;

    Stage1NumericRuntimeState::RunnerSameFrameCtxOwner801C9094Runtime&
        ctxOwner = runtime.sameFrameCtx801C9094;
    ctxOwner = Stage1NumericRuntimeState::
        RunnerSameFrameCtxOwner801C9094Runtime{};
    ctxOwner.snapshotAvailable = true;
    ctxOwner.queryFrame = timing.queryFrame;
    ctxOwner.tick96 = timing.tick96;
    ctxOwner.sameFrameCtxKnown = sameFrameCtxKnown;
    ctxOwner.inputCtx = sameFrameCtx;
    ctxOwner.ctxFlags00InputKnown = sameFrameCtxKnown;
    ctxOwner.ctxFlags00Input = sameFrameCtx.flags00;
    ctxOwner.ctx54InputKnown = sameFrameCtxKnown;
    ctxOwner.ctx54Input = sameFrameCtx.word54;
    ctxOwner.ctxFlags00MissingSameFrameOwner = !sameFrameCtxKnown;

    Stage1NumericRuntimeState::RunnerSameFrameGlobalsOwner801C9094Runtime&
        globalsOwner = runtime.sameFrameGlobals801C9094;
    globalsOwner = Stage1NumericRuntimeState::
        RunnerSameFrameGlobalsOwner801C9094Runtime{};
    globalsOwner.snapshotAvailable = true;
    globalsOwner.queryFrame = timing.queryFrame;
    globalsOwner.tick96 = timing.tick96;
    globalsOwner.sameFrameGlobalsKnown = sameFrameGlobalsKnown;
    globalsOwner.inputGlobals = sameFrameGlobals;
    globalsOwner.timersECF8_ECFA_ECFC_ECFEKnown = sameFrameGlobalsKnown;
    globalsOwner.word8008ECF8 = sameFrameGlobals.word8008ECF8;
    globalsOwner.word8008ECFA = sameFrameGlobals.word8008ECFA;
    globalsOwner.word8008ECFC = sameFrameGlobals.word8008ECFC;
    globalsOwner.word8008ECFE = sameFrameGlobals.word8008ECFE;
    globalsOwner.ed1CKnown = sameFrameGlobalsKnown;
    globalsOwner.ed1C = sameFrameGlobals.unk8008ED1C;
    globalsOwner.d3054D3058Known = sameFrameGlobalsKnown;
    globalsOwner.dword801D3054 = sameFrameGlobals.dword801D3054;
    globalsOwner.dword801D3058 = sameFrameGlobals.dword801D3058;
    globalsOwner.missingDirectOwner = !sameFrameGlobalsKnown;

    Stage1NumericRuntimeState::RunnerCompactLaneOwner801C9094Runtime&
        compactLane = runtime.compactLane801C9094;
    compactLane = Stage1NumericRuntimeState::
        RunnerCompactLaneOwner801C9094Runtime{};
    compactLane.snapshotAvailable = true;
    compactLane.queryFrame = timing.queryFrame;
    compactLane.tick96 = timing.tick96;
    compactLane.currentKnown = sameFrameCtxKnown;
    compactLane.word8A = sameFrameCtx.word8A;
    compactLane.word8C = sameFrameCtx.word8C;
    compactLane.word8E = sameFrameCtx.word8E;
    compactLane.word90 = sameFrameCtx.word90;
    compactLane.word9E = sameFrameCtx.word9E;
    compactLane.wordA0 = sameFrameCtx.wordA0;
    compactLane.wordA2 = sameFrameCtx.wordA2;
    compactLane.missingCurrentDirectOwner = !sameFrameCtxKnown;
}

void StoreStage1RunnerFrameUpdate9094Snapshot(
    Stage1NumericRuntimeState& state,
    const Stage1EventStreamFrameUpdate9094Result& result) {
    Stage1NumericRuntimeState::RunnerPostFrame7A60Runtime& runtime =
        state.runnerPostFrame7A60;
    runtime.word0Flags801C9094FinalKnown = result.ctx0FinalFlags.finalKnown;
    runtime.word0Flags801C9094Final = result.ctx0FinalFlags.finalFlags;
    runtime.word0Flags801C9094RequiresSameFrameCtx =
        !result.frameUpdate.ctx0FinalInputsComplete801C9094;
    runtime.word0Flags801C9094RequiresSameFrameGlobals =
        !result.frameUpdate.ctx0FinalInputsComplete801C9094;
    runtime.word0Flags801C9094RequiresFlagStreamFlags04 =
        !result.frameUpdate.consumedFlagStreamEvent;
    runtime.word0Flags801C9094RequiresIdStreamDescriptorState =
        !result.frameUpdate.consumedIdStreamEvent;
    runtime.word0Flags801C9094RequiresCompactRailRecord =
        !result.frameUpdate.ctx0FinalInputsComplete801C9094;
    runtime.frameUpdate9094ReturnEarlyAfterFlag40Reset =
        result.frameUpdate.returnEarlyAfterFlag40Reset;

    Stage1NumericRuntimeState::RunnerSameFrameCtxOwner801C9094Runtime&
        ctxOwner = runtime.sameFrameCtx801C9094;
    if (ctxOwner.snapshotAvailable) {
        ctxOwner.projectedCtx = result.frameUpdate.ctx;
        ctxOwner.ctxFlags00ProjectedKnown =
            result.frameUpdate.ctx0FinalInputsComplete801C9094;
        ctxOwner.ctxFlags00Projected = result.frameUpdate.ctx.flags00;
        ctxOwner.ctxFlags00FinalKnown = result.ctx0FinalFlags.finalKnown;
        ctxOwner.ctxFlags00Final = result.ctx0FinalFlags.finalFlags;
        ctxOwner.ctx54ProjectedKnown = true;
        ctxOwner.ctx54Projected = result.frameUpdate.ctx.word54;
        ctxOwner.ctxFlags00MissingSameFrameOwner =
            !ctxOwner.sameFrameCtxKnown;
        ctxOwner.ctxFlags00MissingFlagStreamFlags04 =
            !result.frameUpdate.consumedFlagStreamEvent;
        ctxOwner.ctxFlags00MissingIdStreamDescriptorState =
            !result.frameUpdate.consumedIdStreamEvent;
        ctxOwner.ctxFlags00MissingCompactRailRecord =
            !result.frameUpdate.ctx0FinalInputsComplete801C9094;
        ctxOwner.ed1CProducerKnown = result.ran;
        ctxOwner.ed1CProducerEventStreamFlag =
            result.runnerInput.globals.eventStreamFlag;
        ctxOwner.ed1CProducerFlagDescriptorValid =
            result.runnerInput.eventStreamFlagDescriptorValid;
        ctxOwner.ed1CProducerActiveFlagStreamValid =
            result.runnerInput.activeFlagStream.valid;
        ctxOwner.ed1CProducerActiveFlagStreamIndex =
            result.runnerInput.activeFlagStream.index;
        ctxOwner.ed1CProducerActiveFlagStreamCount =
            result.runnerInput.activeFlagStream.count;
        ctxOwner.ed1CProducerActiveFlagStreamDueKnown =
            result.runnerInput.activeFlagStream.dueEventKnown;
        ctxOwner.ed1CProducerActiveFlagStreamDueFrame =
            result.runnerInput.activeFlagStream.dueFrame;
        ctxOwner.ed1CProducerActiveFlagStreamDueDelta =
            result.runnerInput.activeFlagStream.dueEventKnown
                ? result.runnerInput.ctx.tick0C -
                      result.runnerInput.activeFlagStream.dueFrame
                : 0;
        ctxOwner.ed1CProducerActiveFlagStreamFlags04 =
            result.runnerInput.activeFlagStream.flags04;
        ctxOwner.ed1CProducerActiveFlagStreamFlag80 =
            (result.runnerInput.activeFlagStream.flags04 &
             kStage1EventStreamFlag80Ed1C) != 0u;
        ctxOwner.ed1CProducerConsumedFlagStreamEvent =
            result.frameUpdate.consumedFlagStreamEvent;
        ctxOwner.ed1CProducerProduced =
            result.frameUpdate.consumedFlagStreamEvent &&
            (result.frameUpdate.consumedEventFlags04 &
             kStage1EventStreamFlag80Ed1C) != 0u;
        state.rightRankBucketContext.ctx54Permit =
            result.frameUpdate.ctx.word54 != 0u;
    }

    Stage1NumericRuntimeState::RunnerSameFrameGlobalsOwner801C9094Runtime&
        globalsOwner = runtime.sameFrameGlobals801C9094;
    if (globalsOwner.snapshotAvailable) {
        globalsOwner.projectedGlobals = result.frameUpdate.globals;
        globalsOwner.timersECF8_ECFA_ECFC_ECFEKnown =
            result.frameUpdate.ctx0FinalInputsComplete801C9094;
        globalsOwner.word8008ECF8 = result.frameUpdate.globals.word8008ECF8;
        globalsOwner.word8008ECFA = result.frameUpdate.globals.word8008ECFA;
        globalsOwner.word8008ECFC = result.frameUpdate.globals.word8008ECFC;
        globalsOwner.word8008ECFE = result.frameUpdate.globals.word8008ECFE;
        globalsOwner.ed1CKnown =
            result.frameUpdate.ctx0FinalInputsComplete801C9094;
        globalsOwner.ed1C = result.frameUpdate.globals.unk8008ED1C;
        globalsOwner.d3054D3058Known =
            result.frameUpdate.ctx0FinalInputsComplete801C9094;
        globalsOwner.dword801D3054 =
            result.frameUpdate.globals.dword801D3054;
        globalsOwner.dword801D3058 =
            result.frameUpdate.globals.dword801D3058;
        globalsOwner.missingDirectOwner =
            !globalsOwner.sameFrameGlobalsKnown;
    }

    Stage1NumericRuntimeState::RunnerCompactLaneOwner801C9094Runtime&
        compactLane = runtime.compactLane801C9094;
    if (compactLane.snapshotAvailable) {
        compactLane.currentKnown =
            result.frameUpdate.ctx0FinalInputsComplete801C9094;
        compactLane.word8A = result.frameUpdate.ctx.word8A;
        compactLane.word8C = result.frameUpdate.ctx.word8C;
        compactLane.word8E = result.frameUpdate.ctx.word8E;
        compactLane.word90 = result.frameUpdate.ctx.word90;
        compactLane.word9E = result.frameUpdate.ctx.word9E;
        compactLane.wordA0 = result.frameUpdate.ctx.wordA0;
        compactLane.wordA2 = result.frameUpdate.ctx.wordA2;
        compactLane.missingCurrentDirectOwner =
            !ctxOwner.sameFrameCtxKnown;
        compactLane.missingCompactRailRecord =
            !result.frameUpdate.ctx0FinalInputsComplete801C9094;
    }
}

void ApplyStage1RunnerFrameUpdate9094ActionTrace(
    const PrGameContext& ctx,
    Stage1NumericRuntimeState& state,
    const PrStageRunnerDirectActionTrace& trace) {
    for (size_t i = 0; i < trace.count; ++i) {
        if (trace.actions[i].kind != PrStageRunnerDirectActionKind::Call800169E0) {
            continue;
        }
        const PrSavedScoreSync169E0Result savedScore =
            PrStage1SaveUiDirect::Sub800169E0(
                static_cast<int32_t>(ctx.transitionState),
                1);
        if (!savedScore.ok || !savedScore.applied) {
            continue;
        }
        state.scoreDisplayValue =
            static_cast<int32_t>(savedScore.ctxScoreDword);
        state.scoreDisplayFormulaBacked = false;
        state.scorerPort.accumulator91816 =
            static_cast<int32_t>(savedScore.word80091816);
        state.formulaAccumulatorValue =
            static_cast<int32_t>(savedScore.word80091816);
        state.formulaAccumulatorKnown = true;
        state.formulaAccumulatorAuthoritative = true;
    }
}

bool ResolveStage1TimecodeXaSector801C7560(
    const PrGameContext& ctx,
    const Stage1NumericRuntimeTimecodeInput801C7560& input,
    int32_t& outReadValueA7A4) {
    outReadValueA7A4 = 0;
    if (!input.sceneEntryField196TimeBaseKnown) {
        return false;
    }

    const PrStage1XaCdDirectStreamClockProbe800493F4 clockProbe =
        PrStage1XaCdDirectProbeStreamClockProducer800493F4(
            ctx.stage1XaCdDirect);
    if (!clockProbe.carrier.clockKnown ||
        clockProbe.carrier.gapMissingByte800493F4ClockProducer) {
        return false;
    }

    outReadValueA7A4 =
        clockProbe.carrier.clockLba - input.sceneEntryField196TimeBaseA7A4;
    return true;
}

void AdvanceStage1RunnerTimecode801C7560(
    Stage1NumericRuntimeState& state,
    const Stage1NumericRuntimeTimecodeInput801C7560& input,
    bool xaSectorKnown,
    int32_t xaReadValueA7A4) {
    Stage1NumericRuntimeState::RunnerTimecode801C7560Runtime& runtime =
        state.runnerTimecode801C7560;
    if (!runtime.known) {
        runtime.state.tick801C364C = input.sceneEntryField356TickOffset;
    }

    int32_t xaSectorReadValueA7A4 =
        runtime.state.dword801D303CPreviousXaReadValue;
    runtime.xaReadValueA7A4Known = xaSectorKnown;
    runtime.gapMissingSceneEntryField196TimeBase =
        !input.sceneEntryField196TimeBaseKnown;
    runtime.gapMissingStreamClock800493F4 =
        input.sceneEntryField196TimeBaseKnown && !xaSectorKnown;
    if (xaSectorKnown) {
        runtime.xaSectorBaselineKnown = true;
        runtime.xaSectorBaseline75 = input.sceneEntryField196TimeBaseA7A4;
        xaSectorReadValueA7A4 = xaReadValueA7A4;
    }

    PrStageRunnerDirectTimecodeInput801C7560 directInput{};
    directInput.state = runtime.state;
    directInput.sceneEntryField348 =
        input.sceneEntryField348TicksPerMinute;
    directInput.sceneEntryField352FallbackTickAdvance =
        input.sceneEntryField352FallbackTickAdvance;
    directInput.sceneEntryField356TickOffset =
        input.sceneEntryField356TickOffset;
    directInput.xaSectorReadValueA7A4 = xaSectorReadValueA7A4;

    runtime.lastResult =
        PrStageRunnerDirectUpdateTimecode801C7560(directInput);
    runtime.state = runtime.lastResult.state;
    runtime.snapshot =
        PrStageRunnerDirectMakeTimecodeSnapshot801C7560(runtime.state);
    runtime.queryFrame = state.queryFrame;
    runtime.known = true;
}

static int32_t ResolveStage1GoodToCoolDelayTick96(uint16_t nextCtx72) {
    // 801CA3BC wires 943F4/943F8 to COMOD1 compact cues. 80024FD0 reads
    // the 16-bit delay at +4: 943F4=0x0610, 943F8=0x0310.
    return nextCtx72 >= 2u ? kStage1LateBranchGoodToCoolRepeatDelayTick96
                           : kStage1LateBranchGoodToCoolFirstDelayTick96;
}

struct Stage1SourceCellHeaderState {
    bool headerPresent = false;
    uint32_t headerAddr = 0;
    uint32_t basePtr = 0;
    uint16_t count = 0;
    uint16_t headerCursor = 0;
};

struct Stage1SelectedSourceCellState {
    bool headerPresent = false;
    uint32_t headerBasePtr = 0;
    uint16_t headerCount = 0;
    uint16_t headerCursor = 0;
    bool sourceCellPresent = false;
    uint32_t sourceCellPtr = 0;
    uint8_t program = 0;
    uint8_t note = 0;
    uint8_t key = 0;
    uint8_t volume = 0;
    uint16_t recordCompanionWord = 0;
    bool callbackArgPresent = false;
};

struct Stage1DirectPortAcceptedProducerAccessorContext {
    const PrGameContext* ctx = nullptr;
};

struct Stage1AcceptedFrontDoorPacket7A60 {
    PrStage1ScorerDirectAcceptedProducerCoreInput directInput{};
    PrStage1ScorerDirectDescriptorRow directRow{};
};

enum class Stage1AcceptedProducerSplit : uint8_t {
    None = 0,
    Recorded,
    PenaltySide,
};

struct Stage1PageClearProjection14BDC {
    bool valid = false;
    uint8_t targetOrdinal1Based = 0u;
    size_t pageIndex = 0u;
};

struct Stage1PageRecordSlotProjection14614 {
    bool valid = false;
    uint8_t targetOrdinal1Based = 0u;
    size_t pageIndex = 0u;
    size_t rawOffset = 0u;
    PrStage1ScorerDirectRawRecord rawRecord{};
};

struct Stage1PageMaintenanceProjection14BDC_14614 {
    bool valid = false;
    bool clearValid = false;
    Stage1PageClearProjection14BDC clear{};
    bool slotValid = false;
    Stage1PageRecordSlotProjection14614 slot{};
};

struct Stage1FormalLifecycleRuntime {
    bool active = false;
    int lastRunnerFrame = -1;
    uint32_t queryFrame = 0;
    uint8_t lastKnownRightRankRow = kStage1InitialRightRankActiveRow;
    uint32_t awfulHoldFrames = 0;
    bool clearGate = false;
    bool clearTerminalTailGate = false;
    bool failGate = false;
    bool runnerExitLatch76Known = false;
    bool runnerExitLatch76Active = false;
    bool runnerExitLatch76SourceBucket0 = false;
    bool runnerExitLatch76SourceTailStream4 = false;
    bool runnerExitLatch76SourceLowLevelAbort = false;
    bool runnerExitLatch76Bucket0PulseDetailKnown = false;
    uint32_t runnerExitLatch76Bucket0PulseQueryFrame = 0u;
    uint8_t runnerExitLatch76Bucket0PulseRightRankRow = 0u;
    bool runnerExitLatch76Bucket0PulseCallWindowOpen = false;
    uint16_t runnerExitLatch76Bucket0PulseDescriptorFlags = 0u;
    bool runnerExitGate78Known = false;
    bool runnerExitGate78Active = false;
    bool runnerExitGate78SourceEd1CHandoff = false;
    bool runnerExitGate78FrameUpdate9094Known = false;
    uint32_t runnerExitGate78FrameUpdate9094QueryFrame = 0u;
    uint16_t runnerExitGate78FrameUpdate9094Ctx76 = 0u;
    uint16_t runnerExitGate78FrameUpdate9094Ctx78 = 0u;
    bool runnerExitGate78FrameUpdate9094TailFamilyActive = false;
    uint8_t runnerExitGate78FrameUpdate9094TailStream = 0u;
    uint8_t runnerExitGate78FrameUpdate9094ActiveStream = 0u;
    bool runnerExitGate78Ed1CProducerKnown = false;
    uint16_t runnerExitGate78Ed1CProducerEventStreamFlag = 0u;
    bool runnerExitGate78Ed1CProducerFlagDescriptorValid = false;
    bool runnerExitGate78Ed1CProducerActiveFlagStreamValid = false;
    uint32_t runnerExitGate78Ed1CProducerActiveFlagStreamIndex = 0u;
    uint32_t runnerExitGate78Ed1CProducerActiveFlagStreamCount = 0u;
    bool runnerExitGate78Ed1CProducerDueKnown = false;
    int32_t runnerExitGate78Ed1CProducerDueFrame = 0;
    int32_t runnerExitGate78Ed1CProducerDueDelta = 0;
    uint32_t runnerExitGate78Ed1CProducerFlags04 = 0u;
    bool runnerExitGate78Ed1CProducerFlag80 = false;
    bool runnerExitGate78Ed1CProducerConsumedFlagStreamEvent = false;
    bool runnerExitGate78Ed1CProducerProduced = false;
    bool runnerExitEventStreamFlagLastUpdateKnown = false;
    uint8_t runnerExitEventStreamFlagLastUpdateReason = 0u;
    uint32_t runnerExitEventStreamFlagLastUpdateQueryFrame = 0u;
    uint32_t runnerExitEventStreamFlagLastUpdateScriptFrame = 0u;
    uint16_t runnerExitEventStreamFlagLastUpdatePrevious = 0u;
    uint16_t runnerExitEventStreamFlagLastUpdateCurrent = 0u;
    bool runnerExitEventStreamFlagLastChangeKnown = false;
    uint8_t runnerExitEventStreamFlagLastChangeReason = 0u;
    uint32_t runnerExitEventStreamFlagLastChangeQueryFrame = 0u;
    uint32_t runnerExitEventStreamFlagLastChangeScriptFrame = 0u;
    uint16_t runnerExitEventStreamFlagLastChangePrevious = 0u;
    uint16_t runnerExitEventStreamFlagLastChangeCurrent = 0u;
    bool runnerExitFlag40Active = false;
    uint8_t runnerExitLateBranchSelectedStream = 0u;
    uint8_t runnerExitLateBranchActiveDispatchStream = 0u;
    bool runnerExitFlag100BlocksWaitActive = false;
    uint8_t runnerExitFlag100SourceStream = 0u;
    uint32_t runnerExitLateBranchScriptFrame = 0u;
    bool runnerExitClearTerminalTailPulseInput = false;
    bool runnerExitClearTerminalTailPulseArmed = false;
    bool runnerExitClearTerminalTailPulseBlockedAlreadyArmed = false;
    bool runnerExitClearTerminalTailPulseBlockedActiveDispatch = false;
    bool runnerExitClearTerminalTailPulseBlockedPendingMismatch = false;
    uint8_t runnerExitClearTerminalTailPulseStream = 0u;
    bool runnerExitClearTerminalBranchTriggerAttempted = false;
    bool runnerExitClearTerminalBranchTriggerAccepted = false;
    uint32_t runnerExitClearTerminalBranchTriggerScriptFrame = 0u;
    uint8_t runnerExitClearTerminalBranchTriggerRightRankRow = 0u;
    uint8_t runnerExitClearTerminalBranchTriggerCurrentMode = 0u;
    bool runnerExitClearTerminalBranchTriggerBlockedConsumed = false;
    bool runnerExitClearTerminalBranchTriggerBlockedArmed = false;
    bool runnerExitClearTerminalBranchTriggerBlockedFlagNotOne = false;
    bool runnerExitClearTerminalBranchTriggerBlockedRow = false;
    bool runnerExitClearTerminalBranchTriggerBlockedStreamMissing = false;
    bool runnerExitClearTerminalBranchTriggerBlockedCursorDone = false;
    bool runnerExitClearTerminalBranchTriggerBlockedEventNotDue = false;
    bool runnerExitClearTerminalBranchTriggerBlockedMissingFlag80 = false;
    uint16_t runnerExitClearTerminalBranchTriggerStreamFlag = 0u;
    uint32_t runnerExitClearTerminalBranchTriggerAttemptCount = 0u;
    uint32_t runnerExitClearTerminalBranchTriggerAcceptedCount = 0u;
    uint32_t runnerExitClearTerminalBranchTriggerEligibleCount = 0u;
    uint32_t runnerExitClearTerminalBranchTriggerBlockedFlagNotOneCount = 0u;
    uint32_t runnerExitClearTerminalBranchTriggerBlockedRowCount = 0u;
    uint32_t runnerExitClearTerminalBranchTriggerBlockedFlagAndRowCount = 0u;
    uint32_t runnerExitClearTerminalBranchTriggerBlockedConsumedCount = 0u;
    uint32_t runnerExitClearTerminalBranchTriggerBlockedArmedCount = 0u;
    uint32_t runnerExitClearTerminalBranchTriggerBlockedStreamMissingCount = 0u;
    uint32_t runnerExitClearTerminalBranchTriggerBlockedCursorDoneCount = 0u;
    uint32_t runnerExitClearTerminalBranchTriggerBlockedEventNotDueCount = 0u;
    uint32_t runnerExitClearTerminalBranchTriggerBlockedMissingFlag80Count = 0u;
    bool runnerExitFirstClearTerminalBranchTriggerKnown = false;
    uint32_t runnerExitFirstClearTerminalBranchTriggerScriptFrame = 0u;
    uint8_t runnerExitFirstClearTerminalBranchTriggerRightRankRow = 0u;
    uint8_t runnerExitFirstClearTerminalBranchTriggerCurrentMode = 0u;
    uint16_t runnerExitFirstClearTerminalBranchTriggerStreamFlag = 0u;
    bool runnerExitFirstClearTerminalBranchTriggerEligibleKnown = false;
    uint32_t runnerExitFirstClearTerminalBranchTriggerEligibleScriptFrame = 0u;
    uint8_t runnerExitFirstClearTerminalBranchTriggerEligibleRightRankRow = 0u;
    uint8_t runnerExitFirstClearTerminalBranchTriggerEligibleCurrentMode = 0u;
    bool runnerExitFirstClearTerminalBranchTriggerEligibleStream1Known = false;
    uint32_t runnerExitFirstClearTerminalBranchTriggerEligibleStream1Cursor = 0u;
    uint32_t runnerExitFirstClearTerminalBranchTriggerEligibleStream1Count = 0u;
    bool runnerExitFirstClearTerminalBranchTriggerEligibleStream1DueKnown = false;
    uint32_t runnerExitFirstClearTerminalBranchTriggerEligibleStream1DueFrame = 0u;
    int32_t runnerExitFirstClearTerminalBranchTriggerEligibleStream1DueDelta = 0;
    uint32_t runnerExitFirstClearTerminalBranchTriggerEligibleStream1BaseFrame = 0u;
    uint32_t runnerExitFirstClearTerminalBranchTriggerEligibleStream1AbsDueFrame = 0u;
    int32_t runnerExitFirstClearTerminalBranchTriggerEligibleStream1AbsDueDelta = 0;
    uint32_t runnerExitFirstClearTerminalBranchTriggerEligibleStream1Flags04 = 0u;
    uint8_t runnerExitFirstClearTerminalBranchTriggerEligibleStream1Byte29 = 0u;
    uint8_t runnerExitFirstClearTerminalBranchTriggerEligibleStream1Byte30 = 0u;
    bool runnerExitFirstClearTerminalBranchTriggerEligibleStream1NextFlag80SearchKnown = false;
    bool runnerExitFirstClearTerminalBranchTriggerEligibleStream1NextFlag80Known = false;
    uint32_t runnerExitFirstClearTerminalBranchTriggerEligibleStream1NextFlag80Cursor = 0u;
    uint32_t runnerExitFirstClearTerminalBranchTriggerEligibleStream1NextFlag80Frame = 0u;
    int32_t runnerExitFirstClearTerminalBranchTriggerEligibleStream1NextFlag80Delta = 0;
    uint32_t runnerExitFirstClearTerminalBranchTriggerEligibleStream1NextFlag80BaseFrame = 0u;
    uint32_t runnerExitFirstClearTerminalBranchTriggerEligibleStream1NextFlag80AbsFrame = 0u;
    int32_t runnerExitFirstClearTerminalBranchTriggerEligibleStream1NextFlag80AbsDelta = 0;
    uint32_t runnerExitFirstClearTerminalBranchTriggerEligibleStream1NextFlag80Flags04 = 0u;
    uint8_t runnerExitFirstClearTerminalBranchTriggerEligibleStream1NextFlag80Byte29 = 0u;
    uint8_t runnerExitFirstClearTerminalBranchTriggerEligibleStream1NextFlag80Byte30 = 0u;
    uint32_t runnerExitClearTerminalBranchTriggerStream1Cursor = 0u;
    uint32_t runnerExitClearTerminalBranchTriggerStream1Count = 0u;
    bool runnerExitClearTerminalBranchTriggerStream1DueKnown = false;
    uint32_t runnerExitClearTerminalBranchTriggerStream1DueFrame = 0u;
    int32_t runnerExitClearTerminalBranchTriggerStream1DueDelta = 0;
    uint32_t runnerExitClearTerminalBranchTriggerStream1BaseFrame = 0u;
    uint32_t runnerExitClearTerminalBranchTriggerStream1AbsDueFrame = 0u;
    int32_t runnerExitClearTerminalBranchTriggerStream1AbsDueDelta = 0;
    uint32_t runnerExitClearTerminalBranchTriggerStream1Flags04 = 0u;
    uint8_t runnerExitClearTerminalBranchTriggerStream1Byte29 = 0u;
    uint8_t runnerExitClearTerminalBranchTriggerStream1Byte30 = 0u;
    bool runnerExitFirstClearTerminalBranchTriggerEventNotDueKnown = false;
    uint32_t runnerExitFirstClearTerminalBranchTriggerEventNotDueScriptFrame = 0u;
    uint32_t runnerExitFirstClearTerminalBranchTriggerEventNotDueCursor = 0u;
    uint32_t runnerExitFirstClearTerminalBranchTriggerEventNotDueFrame = 0u;
    int32_t runnerExitFirstClearTerminalBranchTriggerEventNotDueDelta = 0;
    uint32_t runnerExitFirstClearTerminalBranchTriggerEventNotDueBaseFrame = 0u;
    uint32_t runnerExitFirstClearTerminalBranchTriggerEventNotDueAbsFrame = 0u;
    int32_t runnerExitFirstClearTerminalBranchTriggerEventNotDueAbsDelta = 0;
    uint32_t runnerExitFirstClearTerminalBranchTriggerEventNotDueFlags04 = 0u;
    bool runnerExitFirstClearTerminalBranchTriggerMissingFlag80Known = false;
    uint32_t runnerExitFirstClearTerminalBranchTriggerMissingFlag80ScriptFrame = 0u;
    uint32_t runnerExitFirstClearTerminalBranchTriggerMissingFlag80Cursor = 0u;
    uint32_t runnerExitFirstClearTerminalBranchTriggerMissingFlag80Frame = 0u;
    int32_t runnerExitFirstClearTerminalBranchTriggerMissingFlag80Delta = 0;
    uint32_t runnerExitFirstClearTerminalBranchTriggerMissingFlag80BaseFrame = 0u;
    uint32_t runnerExitFirstClearTerminalBranchTriggerMissingFlag80AbsFrame = 0u;
    int32_t runnerExitFirstClearTerminalBranchTriggerMissingFlag80AbsDelta = 0;
    uint32_t runnerExitFirstClearTerminalBranchTriggerMissingFlag80Flags04 = 0u;
    bool runnerExitClearTerminalTailServiceLatch = false;
    uint32_t runnerExitClearTerminalTailServiceScriptFrame = 0u;
    uint8_t runnerExitClearTerminalTailServiceRightRankRow = 0u;
    uint8_t runnerExitClearTerminalTailServiceCurrentMode = 0u;
    uint8_t runnerExitClearTerminalTailServiceStream = 0u;
    uint32_t runnerExitPendingRatingBranchSeq = 0u;
    uint32_t runnerExitConsumedRatingBranchSeq = 0u;
    bool runnerExitClearTerminalTailArmed = false;
    bool runnerExitClearTerminalTailDispatchActive = false;
    uint8_t runnerExitClearTerminalTailStream = 0u;
    uint32_t runnerExitActiveDispatchStartScriptFrame = 0u;
    uint32_t runnerExitActiveDispatchTerminalEndLocalFrame = 0u;
    bool runnerExitActiveDispatchTerminalPulseEmitted = false;
    bool runnerTailRecordsModeDAActive = false;
    bool runnerTailRecordsModeDAEqualsOne = false;
    bool runnerTailStageStatusKnown = false;
    uint8_t runnerTailStageStatus166AC = 0u;
    uint8_t runnerTailCleanupRenderPassBudget = 0u;
    bool runnerTailFinalReturnKnown = false;
    int32_t runnerTailFinalReturn = -1;
    bool runnerTailFinalReturnWord59Known = false;
    bool runnerTailFinalReturnWord59IsOne = false;
    bool runnerTailFinalReturnWord60Known = false;
    bool runnerTailFinalReturnWord60IsOne = false;
    bool runnerTailFinalReturnCalls166AC = false;
    bool runnerTailFinalReturnRecordsModeReturnsOne = false;
    bool runnerTailFinalReturnStageStatusReturnsOne = false;
};

struct Stage1RunnerTailGateRuntime {
    bool ctx76Known = false;
    bool ctx76Active = false;
    bool ctx76Bucket0Latched = false;
    bool ctx76Bucket0PulseDetailKnown = false;
    uint32_t ctx76Bucket0PulseQueryFrame = 0u;
    uint8_t ctx76Bucket0PulseRightRankRow = 0u;
    bool ctx76Bucket0PulseCallWindowOpen = false;
    uint16_t ctx76Bucket0PulseDescriptorFlags = 0u;
    bool ctx76TailStream4Latched = false;
    bool ctx76LowLevelAbortLatched = false;
    bool ctx78Known = false;
    bool ctx78Active = false;
    bool ctx78Ed1CHandoffLatched = false;
    bool ctx78FrameUpdate9094Known = false;
    uint32_t ctx78FrameUpdate9094QueryFrame = 0u;
    uint16_t ctx78FrameUpdate9094Ctx76 = 0u;
    uint16_t ctx78FrameUpdate9094Ctx78 = 0u;
    bool ctx78FrameUpdate9094TailFamilyActive = false;
    uint8_t ctx78FrameUpdate9094TailStream = 0u;
    uint8_t ctx78FrameUpdate9094ActiveStream = 0u;
    bool ctx78Ed1CProducerKnown = false;
    uint16_t ctx78Ed1CProducerEventStreamFlag = 0u;
    bool ctx78Ed1CProducerFlagDescriptorValid = false;
    bool ctx78Ed1CProducerActiveFlagStreamValid = false;
    uint32_t ctx78Ed1CProducerActiveFlagStreamIndex = 0u;
    uint32_t ctx78Ed1CProducerActiveFlagStreamCount = 0u;
    bool ctx78Ed1CProducerDueKnown = false;
    int32_t ctx78Ed1CProducerDueFrame = 0;
    int32_t ctx78Ed1CProducerDueDelta = 0;
    uint32_t ctx78Ed1CProducerFlags04 = 0u;
    bool ctx78Ed1CProducerFlag80 = false;
    bool ctx78Ed1CProducerConsumedFlagStreamEvent = false;
    bool ctx78Ed1CProducerProduced = false;
    bool eventStreamFlagLastUpdateKnown = false;
    uint8_t eventStreamFlagLastUpdateReason = 0u;
    uint32_t eventStreamFlagLastUpdateQueryFrame = 0u;
    uint32_t eventStreamFlagLastUpdateScriptFrame = 0u;
    uint16_t eventStreamFlagLastUpdatePrevious = 0u;
    uint16_t eventStreamFlagLastUpdateCurrent = 0u;
    bool eventStreamFlagLastChangeKnown = false;
    uint8_t eventStreamFlagLastChangeReason = 0u;
    uint32_t eventStreamFlagLastChangeQueryFrame = 0u;
    uint32_t eventStreamFlagLastChangeScriptFrame = 0u;
    uint16_t eventStreamFlagLastChangePrevious = 0u;
    uint16_t eventStreamFlagLastChangeCurrent = 0u;
};

Stage1FormalLifecycleRuntime s_stage1FormalLifecycleRuntime{};
PrStage1FormalLifecycleSnapshot s_stage1TerminalFormalLifecycleSnapshot{};
Stage1RunnerTailGateRuntime s_stage1RunnerTailGateRuntime{};

bool IsStage1SpecialFixedBonusAdditiveGateActive(const PrStageRunner& runner) {
    return runner.GetStage1SpecialFixedBonusLatch() &&
           runner.GetStage1NormalizedSelectControlBit();
}

uint32_t ReadStage1ReplayPayloadU32(const uint8_t* payload,
                                    size_t payloadSize,
                                    size_t offset) {
    if (!payload || offset + sizeof(uint32_t) > payloadSize) {
        return 0u;
    }
    return (uint32_t)payload[offset + 0] |
           ((uint32_t)payload[offset + 1] << 8) |
           ((uint32_t)payload[offset + 2] << 16) |
           ((uint32_t)payload[offset + 3] << 24);
}

bool TryReadStage1SetupPrevGrade92F40FromPayloadBytes(
    const uint8_t* payload,
    size_t payloadSize,
    uint32_t& outPrevGrade92F40) {
    if (!payload ||
        payloadSize <
            kStage1ReplayPayloadPrevGrade92F40Offset + sizeof(uint32_t)) {
        return false;
    }
    const uint32_t prevGrade =
        (uint32_t)payload[kStage1ReplayPayloadPrevGrade92F40Offset + 0] |
        ((uint32_t)payload[kStage1ReplayPayloadPrevGrade92F40Offset + 1]
         << 8) |
        ((uint32_t)payload[kStage1ReplayPayloadPrevGrade92F40Offset + 2]
         << 16) |
        ((uint32_t)payload[kStage1ReplayPayloadPrevGrade92F40Offset + 3]
         << 24);
    if (prevGrade < 1u || prevGrade > 3u) {
        return false;
    }
    outPrevGrade92F40 = prevGrade;
    return true;
}

bool TryReadStage1SetupRestartReplayPayloadPrevGrade92F40(
    uint32_t& outPrevGrade92F40) {
    const PrStage1SaveStatusPrefix80092F10 livePayload =
        PrStage1SaveUiDirect::GetSaveStatusPrefix80092F10();
    if (livePayload.known &&
        !livePayload.helperGap &&
        livePayload.statusBankKnown80092F1D &&
        TryReadStage1SetupPrevGrade92F40FromPayloadBytes(
            livePayload.bytes,
            livePayload.byteCount,
            outPrevGrade92F40)) {
        return true;
    }
    return false;
}

bool TryReadStage1SetupStatus166ACFromPayloadBytes(const uint8_t* payload,
                                                   size_t payloadSize,
                                                   int32_t sceneId,
                                                   uint8_t& outStatus) {
    if (sceneId < 1 ||
        sceneId > static_cast<int32_t>(kStage1ReplayPayloadStatusSlotCount)) {
        return false;
    }
    const size_t slotIndex = static_cast<size_t>(sceneId - 1);
    const size_t statusOffset =
        kStage1ReplayPayloadStatus92F1DOffset + slotIndex;
    if (!payload || payloadSize <= statusOffset) {
        return false;
    }
    const uint8_t status = payload[statusOffset];
    if (status > 3u) {
        return false;
    }
    outStatus = status;
    return true;
}

bool TryReadStage1SetupStatus166ACFromPayloadFallback(int32_t sceneId,
                                                      uint8_t& outStatus) {
    const PrStage1SaveStatusPrefix80092F10 livePayload =
        PrStage1SaveUiDirect::GetSaveStatusPrefix80092F10();
    if (livePayload.known &&
        !livePayload.helperGap &&
        livePayload.statusBankKnown80092F1D &&
        TryReadStage1SetupStatus166ACFromPayloadBytes(livePayload.bytes,
                                                      livePayload.byteCount,
                                                      sceneId,
                                                      outStatus)) {
        return true;
    }
    return false;
}

uint16_t ReadStage1DescriptorWordLEHost(const uint8_t* bytes) {
    return (uint16_t)((uint16_t)bytes[0] | ((uint16_t)bytes[1] << 8));
}

uint32_t ReadStage1DescriptorDwordLEHost(const uint8_t* bytes) {
    return (uint32_t)bytes[0] |
           ((uint32_t)bytes[1] << 8) |
           ((uint32_t)bytes[2] << 16) |
           ((uint32_t)bytes[3] << 24);
}

bool TryReadStage1OverlayBytesAtHost(const PrGameContext& ctx,
                                     uint32_t psxAddr,
                                     size_t byteCount,
                                     const uint8_t*& outBytes) {
    outBytes = nullptr;
    if (ctx.currentComodBytes.empty() ||
        psxAddr < PrStage1OverlayData::kPsxBase) {
        return false;
    }

    const size_t byteOffset =
        (size_t)(psxAddr - PrStage1OverlayData::kPsxBase);
    if (byteOffset + byteCount > ctx.currentComodBytes.size()) {
        return false;
    }

    outBytes = ctx.currentComodBytes.data() + byteOffset;
    return true;
}

bool TryReadStage1OverlayByteAtHost(const PrGameContext& ctx,
                                    uint32_t psxAddr,
                                    uint8_t& outValue) {
    outValue = 0u;
    const uint8_t* bytes = nullptr;
    if (!TryReadStage1OverlayBytesAtHost(
            ctx,
            psxAddr,
            sizeof(uint8_t),
            bytes) ||
        !bytes) {
        return false;
    }
    outValue = bytes[0];
    return true;
}

bool TryReadStage1OverlayDwordAtHost(const PrGameContext& ctx,
                                     uint32_t psxAddr,
                                     uint32_t& outValue) {
    outValue = 0u;
    const uint8_t* bytes = nullptr;
    if (!TryReadStage1OverlayBytesAtHost(
            ctx,
            psxAddr,
            sizeof(uint32_t),
            bytes) ||
        !bytes) {
        return false;
    }
    outValue = ReadStage1DescriptorDwordLEHost(bytes);
    return true;
}

bool TryReadStage1DescriptorRowHost(
    const PrGameContext& ctx,
    size_t rowIndex,
    Stage1NumericRuntimeState::DescriptorRowView& out) {
    out = Stage1NumericRuntimeState::DescriptorRowView{};
    if (rowIndex >= kStage1DescriptorMainTableRowCount ||
        ctx.currentComodBytes.empty()) {
        return false;
    }

    const size_t tableOffset =
        (size_t)(kStage1DescriptorMainTableAddr - PrStage1OverlayData::kPsxBase);
    const size_t rowOffset = tableOffset + rowIndex * kStage1DescriptorMainTableRowSize;
    if (rowOffset + kStage1DescriptorMainTableRowSize >
        ctx.currentComodBytes.size()) {
        return false;
    }

    const uint8_t* rowBytes = ctx.currentComodBytes.data() + rowOffset;
    out.available = true;
    out.lessonId = rowBytes[0];
    out.anchorSlotIndex = rowBytes[1];
    out.requiredClassToken = rowBytes[2];
    out.penaltyWeight = rowBytes[3];
    out.finalResolutionThreshold =
        (int16_t)ReadStage1DescriptorWordLEHost(rowBytes + 4);
    out.requiredMask = ReadStage1DescriptorDwordLEHost(rowBytes + 8);
    out.defaultSelectorByte0 = rowBytes[12];
    out.defaultSelectorByte1 = rowBytes[13];
    out.defaultLookbackPageCount =
        (int16_t)ReadStage1DescriptorWordLEHost(rowBytes + 14);
    out.defaultFlagWord = ReadStage1DescriptorWordLEHost(rowBytes + 16);
    out.substate1SelectorByte0 = rowBytes[18];
    out.substate1SelectorByte1 = rowBytes[19];
    out.substate1LookbackPageCount =
        (int16_t)ReadStage1DescriptorWordLEHost(rowBytes + 20);
    out.substate1FlagWord = ReadStage1DescriptorWordLEHost(rowBytes + 22);
    return true;
}

bool TryResolveStage1TimingTemplateStateAtSlotIndexHost(
    const PrGameContext& ctx,
    uint8_t selectorByte1,
    uint8_t slotIndex,
    uint8_t& outState) {
    outState = 0u;
    const uint8_t clampedSlotIndex = (std::min<uint8_t>)(
        slotIndex,
        (uint8_t)(kStage1TimingTemplateSlotCount - 1u));
    uint8_t rawState = 0u;
    if (!TryReadStage1OverlayByteAtHost(
            ctx,
            kStage1TimingTemplateTableAddr +
                (uint32_t)selectorByte1 *
                    (uint32_t)kStage1TimingTemplateSelectorStride +
                (uint32_t)clampedSlotIndex,
            rawState)) {
        return false;
    }

    outState = rawState;
    return true;
}

bool TryResolveStage1SourceCellHeaderStateHost(
    const PrGameContext& ctx,
    uint8_t selectorByte0,
    uint8_t classToken,
    Stage1SourceCellHeaderState& out) {
    out = Stage1SourceCellHeaderState{};
    if (classToken == 0u) {
        return false;
    }

    uint32_t headerAddr = 0u;
    if (!TryReadStage1OverlayDwordAtHost(
            ctx,
            kStage1SourceCellSelectorTableAddr +
                (uint32_t)selectorByte0 *
                    (uint32_t)kStage1SourceCellSelectorStride +
                (uint32_t)classToken * sizeof(uint32_t),
            headerAddr) ||
        headerAddr == 0u) {
        return false;
    }

    const uint8_t* headerBytes = nullptr;
    if (!TryReadStage1OverlayBytesAtHost(
            ctx,
            headerAddr,
            kStage1SourceCellHeaderSize,
            headerBytes) ||
        !headerBytes) {
        return false;
    }

    out.headerPresent = true;
    out.headerAddr = headerAddr;
    out.basePtr = ReadStage1DescriptorDwordLEHost(headerBytes);
    out.count = ReadStage1DescriptorWordLEHost(headerBytes + 4);
    out.headerCursor = ReadStage1DescriptorWordLEHost(headerBytes + 6);
    return true;
}

bool TryResolveStage1AcceptedProducerSourceGroupCursorOverride(
    const Stage1NumericRuntimeState::AcceptedProducerSourceGroupRuntime&
        sourceGroup,
    uint32_t headerAddr,
    uint16_t headerCount,
    uint16_t& outCursor) {
    outCursor = 0u;
    if (headerAddr == 0u || headerCount == 0u) {
        return false;
    }

    for (const auto& entry : sourceGroup.cursorEntries) {
        if (!entry.occupied ||
            entry.headerAddr != headerAddr ||
            entry.count != headerCount) {
            continue;
        }

        outCursor = (uint16_t)(entry.cursor % headerCount);
        return true;
    }
    return false;
}

bool ReadStage1DirectPortTimingTemplateStateAdapter(
    void* userData,
    uint8_t selectorByte1,
    uint8_t slot48,
    uint8_t& outState) {
    const Stage1DirectPortAcceptedProducerAccessorContext* adapter =
        static_cast<const Stage1DirectPortAcceptedProducerAccessorContext*>(
            userData);
    if (!adapter || !adapter->ctx) {
        outState = 0u;
        return false;
    }
    return TryResolveStage1TimingTemplateStateAtSlotIndexHost(
        *adapter->ctx,
        selectorByte1,
        slot48,
        outState);
}

bool ReadStage1DirectPortSourceCellHeaderAdapter(
    void* userData,
    uint8_t selectorByte0,
    uint8_t classToken20,
    PrStage1ScorerDirectSourceCellHeader& outHeader) {
    outHeader = PrStage1ScorerDirectSourceCellHeader{};
    const Stage1DirectPortAcceptedProducerAccessorContext* adapter =
        static_cast<const Stage1DirectPortAcceptedProducerAccessorContext*>(
            userData);
    if (!adapter || !adapter->ctx) {
        return false;
    }

    Stage1SourceCellHeaderState header{};
    if (!TryResolveStage1SourceCellHeaderStateHost(
            *adapter->ctx,
            selectorByte0,
            classToken20,
            header)) {
        return false;
    }

    outHeader.valid = true;
    outHeader.dword00HeaderAddr = header.headerAddr;
    outHeader.dword04BasePtr = header.basePtr;
    outHeader.word08Count = header.count;
    outHeader.word0ACursor = header.headerCursor;
    return true;
}

bool ReadStage1DirectPortSourceCellAdapter(
    void* userData,
    uint32_t sourceCellPtr,
    PrStage1ScorerDirectSourceCell& outCell) {
    outCell = PrStage1ScorerDirectSourceCell{};
    const Stage1DirectPortAcceptedProducerAccessorContext* adapter =
        static_cast<const Stage1DirectPortAcceptedProducerAccessorContext*>(
            userData);
    if (!adapter || !adapter->ctx) {
        return false;
    }

    const uint8_t* sourceCellBytes = nullptr;
    if (!TryReadStage1OverlayBytesAtHost(
            *adapter->ctx,
            sourceCellPtr,
            kStage1SourceCellSize,
            sourceCellBytes) ||
        !sourceCellBytes) {
        return false;
    }

    outCell.valid = true;
    outCell.dword00SourceCellPtr = sourceCellPtr;
    outCell.byte00Program = sourceCellBytes[0];
    outCell.byte01Note = sourceCellBytes[1];
    outCell.byte02Key = (uint8_t)(outCell.byte01Note + 24u);
    outCell.byte03Volume = sourceCellBytes[3];
    outCell.word06RecordCompanion =
        ReadStage1DescriptorWordLEHost(sourceCellBytes + 6);
    outCell.dword08PayloadOpaque =
        ReadStage1DescriptorDwordLEHost(sourceCellBytes + 8);
    outCell.dword08CallbackArgPresent = outCell.dword08PayloadOpaque != 0u;
    return true;
}

uint8_t ResolveStage1NextDescriptorConsumerSubstateIndex(
    const Stage1NumericRuntimeState& state) {
    return ResolveStage1AcceptedProducerSubstate50(state);
}

bool TryResolveStage1DescriptorSelectorBytes(
    const Stage1NumericRuntimeState::DescriptorRowView& row,
    uint8_t routeSubstateIndex,
    uint8_t& outSelectorByte0,
    uint8_t& outSelectorByte1) {
    outSelectorByte0 = 0u;
    outSelectorByte1 = 0u;
    if (!row.available) {
        return false;
    }

    if (routeSubstateIndex != 0u) {
        outSelectorByte0 = row.substate1SelectorByte0;
        outSelectorByte1 = row.substate1SelectorByte1;
    } else {
        outSelectorByte0 = row.defaultSelectorByte0;
        outSelectorByte1 = row.defaultSelectorByte1;
    }
    return true;
}

bool TryResolveStage1AcceptedProducerSelectors(
    const Stage1NumericRuntimeState& state,
    uint8_t& outSelectorByte0,
    uint8_t& outSelectorByte1) {
    outSelectorByte0 = 0u;
    outSelectorByte1 = 0u;

    const Stage1NumericRuntimeState::DescriptorCadenceRuntime&
        descriptorRuntime = state.descriptorCadence;
    if (!descriptorRuntime.lookaheadDescriptor44Available ||
        !descriptorRuntime.lookaheadDescriptor44Row.available) {
        return false;
    }

    return TryResolveStage1DescriptorSelectorBytes(
        descriptorRuntime.lookaheadDescriptor44Row,
        ResolveStage1NextDescriptorConsumerSubstateIndex(state),
        outSelectorByte0,
        outSelectorByte1);
}

bool IsStage1NextDescriptorCallbackHookArmed() {
    // Keep the callback hook state internal-only and conservative until there
    // is a stable runtime mirror for dword_80094430. The current confirmed
    // steady Stage1 path leaves the hook dormant.
    return false;
}

uint8_t ResolveStage1NextDescriptorTimingTemplateSlotIndex(int tick96) {
    if (tick96 < 0) {
        return 0u;
    }
    return (uint8_t)((uint32_t)tick96 % 384u / 48u);
}

void ResolveStage1SelectedSourceCellState(const PrGameContext& ctx,
                                          const Stage1NumericRuntimeState::
                                              AcceptedProducerSourceGroupRuntime*
                                              sourceGroup,
                                          uint8_t selectorByte0,
                                          uint8_t classToken,
                                          Stage1SelectedSourceCellState& out) {
    out = Stage1SelectedSourceCellState{};
    Stage1SourceCellHeaderState header{};
    if (!TryResolveStage1SourceCellHeaderStateHost(
            ctx,
            selectorByte0,
            classToken,
            header)) {
        return;
    }
    out.headerPresent = true;
    out.headerBasePtr = header.basePtr;
    out.headerCount = header.count;
    out.headerCursor = header.headerCursor;
    uint16_t cursorOverride = 0u;
    if (sourceGroup &&
        TryResolveStage1AcceptedProducerSourceGroupCursorOverride(
            *sourceGroup,
            header.headerAddr,
            header.count,
            cursorOverride)) {
        out.headerCursor = cursorOverride;
    }
    if (header.basePtr == 0u || header.count == 0u) {
        return;
    }

    uint16_t cursor = (uint16_t)(out.headerCursor % header.count);
    out.headerCursor = cursor;
    const uint32_t sourceCellPtr =
        header.basePtr + (uint32_t)cursor * (uint32_t)kStage1SourceCellSize;
    const uint8_t* sourceCellBytes = nullptr;
    if (!TryReadStage1OverlayBytesAtHost(
            ctx,
            sourceCellPtr,
            kStage1SourceCellSize,
            sourceCellBytes) ||
        !sourceCellBytes) {
        return;
    }

    out.sourceCellPresent = true;
    out.sourceCellPtr = sourceCellPtr;
    out.program = sourceCellBytes[0];
    out.note = sourceCellBytes[1];
    out.key = (uint8_t)(out.note + 24u);
    out.volume = sourceCellBytes[3];
    out.recordCompanionWord = ReadStage1DescriptorWordLEHost(sourceCellBytes + 6);
    out.callbackArgPresent = ReadStage1DescriptorDwordLEHost(sourceCellBytes + 8) != 0u;
}

void BuildStage1SelectedSourceGroupRailWindow(
    const PrGameContext& ctx,
    const Stage1NumericRuntimeState::AcceptedProducerSourceGroupRuntime&
        sourceGroup,
    uint8_t selectorByte0,
    uint8_t selectorByte1,
    uint8_t classToken,
    int tick96,
    Stage1NumericRuntimeState::SelectedSourceGroupRailWindowRuntime& out) {
    // Keep the selector/source-cell chain internal-only, but retain one
    // bounded timing+header window for the facts currently consumed by rail.
    out = Stage1NumericRuntimeState::SelectedSourceGroupRailWindowRuntime{};
    const uint8_t timingTemplateSlotIndex =
        ResolveStage1NextDescriptorTimingTemplateSlotIndex(tick96);
    out.timingTemplateStateKnown = TryResolveStage1TimingTemplateStateAtSlotIndexHost(
        ctx,
        selectorByte1,
        timingTemplateSlotIndex,
        out.timingTemplateState);

    Stage1SelectedSourceCellState sourceCell{};
    ResolveStage1SelectedSourceCellState(
        ctx,
        &sourceGroup,
        selectorByte0,
        classToken,
        sourceCell);
    out.sourceCellHeaderPresent = sourceCell.headerPresent;
    out.sourceCellHeaderCount = sourceCell.headerCount;
    out.sourceCellHeaderCursor = sourceCell.headerCursor;
    out.sourceCellPresent = sourceCell.sourceCellPresent;
    out.sourceCellCallbackArgPresent = sourceCell.callbackArgPresent;
}

uint8_t ResolveStage1GameplayRailSymbolTypeFromClassToken(uint8_t classToken) {
    switch (classToken) {
    case 1u:
        return 1u;
    case 2u:
        return 2u;
    case 3u:
        return 3u;
    case 4u:
        return 4u;
    case 5u:
        return 5u;
    case 7u:
        return 7u;
    default:
        break;
    }
    return 0u;
}

uint8_t ResolveStage1GameplayRailSymbolTypeFromAcceptedMaskImpl(
    uint16_t acceptedMask) {
    const uint16_t gameplayMask =
        acceptedMask &
        ((uint16_t)PrPadButton::Triangle |
         (uint16_t)PrPadButton::Circle |
         (uint16_t)PrPadButton::Cross |
         (uint16_t)PrPadButton::Square |
         (uint16_t)PrPadButton::Left |
         (uint16_t)PrPadButton::Right);
    if (gameplayMask == 0u || (gameplayMask & (gameplayMask - 1u)) != 0u) {
        return 0u;
    }

    switch (gameplayMask) {
    case (uint16_t)PrPadButton::Triangle:
        return 1u;
    case (uint16_t)PrPadButton::Circle:
        return 2u;
    case (uint16_t)PrPadButton::Cross:
        return 3u;
    case (uint16_t)PrPadButton::Square:
        return 4u;
    case (uint16_t)PrPadButton::Left:
        return 5u;
    case (uint16_t)PrPadButton::Right:
        return 7u;
    default:
        break;
    }
    return 0u;
}

uint8_t ResolveStage1GameplayRailSymbolTypeFromDescriptorRow(
    const Stage1NumericRuntimeState::DescriptorRowView& row) {
    if (!row.available) {
        return 0u;
    }

    const uint8_t symbolType =
        ResolveStage1GameplayRailSymbolTypeFromClassToken(row.requiredClassToken);
    if (symbolType != 0u) {
        return symbolType;
    }

    return ResolveStage1GameplayRailSymbolTypeFromAcceptedMaskImpl(
        (uint16_t)row.requiredMask);
}

void BuildStage1GameplayRailDescriptorProducerRuntime(
    const PrGameContext& ctx,
    const Stage1NumericRuntimeState& state,
    const Stage1NumericRuntimeState::DescriptorRowView& row,
    size_t rowIndex,
    uint8_t routeSubstateIndex,
    int tick96,
    Stage1NumericRuntimeState::GameplayRailDescriptorProducerRuntime& out) {
    out = Stage1NumericRuntimeState::GameplayRailDescriptorProducerRuntime{};
    if (!row.available) {
        return;
    }

    out.available = true;
    out.rowIndex = rowIndex;
    out.row = row;
    out.cueClassToken = row.requiredClassToken;
    out.selectorAvailable = TryResolveStage1DescriptorSelectorBytes(
        row,
        routeSubstateIndex,
        out.selectorByte0,
        out.selectorByte1);

    if (out.selectorAvailable) {
        BuildStage1SelectedSourceGroupRailWindow(
            ctx,
            state.acceptedProducerSourceGroup,
            out.selectorByte0,
            out.selectorByte1,
            out.cueClassToken,
            tick96,
            out.selectedSourceGroupWindow);
    }

    // Keep strip topology descriptor-driven, but only light a renderable icon
    // when the current producer-selected source-cell path actually resolves.
    out.symbolType =
        out.selectedSourceGroupWindow.sourceCellPresent
            ? ResolveStage1GameplayRailSymbolTypeFromDescriptorRow(row)
            : 0u;
}

bool IsStage1FormalLifecycleRuntimeGateActiveImpl(PrGameContext& ctx,
                                                  const PrStageRunner& runner) {
    return ctx.currentScene == PrSceneId::Scene1 &&
           ctx.stageRunning &&
           runner.GetSceneId() == 1;
}

void PopulateStage1FormalLifecycleSnapshotFromRuntime(
    const Stage1FormalLifecycleRuntime& runtime,
    PrStage1FormalLifecycleSnapshot& out) {
    out = PrStage1FormalLifecycleSnapshot{};
    out.valid = runtime.active;
    out.queryFrame = runtime.queryFrame;
    out.clearGate = runtime.clearGate;
    out.clearTerminalTailGate = runtime.clearTerminalTailGate;
    out.failGate = runtime.failGate;
    out.rightRankActiveRow = runtime.lastKnownRightRankRow;
    out.awfulHoldFrames = runtime.awfulHoldFrames;
    out.awfulHoldFramesRequired = kStage1FormalLifecycleAwfulFailHoldFrames;
    out.runnerExitLatch76Known = runtime.runnerExitLatch76Known;
    out.runnerExitLatch76Active = runtime.runnerExitLatch76Active;
    out.runnerExitLatch76SourceBucket0 = runtime.runnerExitLatch76SourceBucket0;
    out.runnerExitLatch76SourceTailStream4 =
        runtime.runnerExitLatch76SourceTailStream4;
    out.runnerExitLatch76SourceLowLevelAbort =
        runtime.runnerExitLatch76SourceLowLevelAbort;
    out.runnerExitLatch76Bucket0PulseDetailKnown =
        runtime.runnerExitLatch76Bucket0PulseDetailKnown;
    out.runnerExitLatch76Bucket0PulseQueryFrame =
        runtime.runnerExitLatch76Bucket0PulseQueryFrame;
    out.runnerExitLatch76Bucket0PulseRightRankRow =
        runtime.runnerExitLatch76Bucket0PulseRightRankRow;
    out.runnerExitLatch76Bucket0PulseCallWindowOpen =
        runtime.runnerExitLatch76Bucket0PulseCallWindowOpen;
    out.runnerExitLatch76Bucket0PulseDescriptorFlags =
        runtime.runnerExitLatch76Bucket0PulseDescriptorFlags;
    out.runnerExitGate78Known = runtime.runnerExitGate78Known;
    out.runnerExitGate78Active = runtime.runnerExitGate78Active;
    out.runnerExitGate78SourceEd1CHandoff =
        runtime.runnerExitGate78SourceEd1CHandoff;
    out.runnerExitGate78FrameUpdate9094Known =
        runtime.runnerExitGate78FrameUpdate9094Known;
    out.runnerExitGate78FrameUpdate9094QueryFrame =
        runtime.runnerExitGate78FrameUpdate9094QueryFrame;
    out.runnerExitGate78FrameUpdate9094Ctx76 =
        runtime.runnerExitGate78FrameUpdate9094Ctx76;
    out.runnerExitGate78FrameUpdate9094Ctx78 =
        runtime.runnerExitGate78FrameUpdate9094Ctx78;
    out.runnerExitGate78FrameUpdate9094TailFamilyActive =
        runtime.runnerExitGate78FrameUpdate9094TailFamilyActive;
    out.runnerExitGate78FrameUpdate9094TailStream =
        runtime.runnerExitGate78FrameUpdate9094TailStream;
    out.runnerExitGate78FrameUpdate9094ActiveStream =
        runtime.runnerExitGate78FrameUpdate9094ActiveStream;
    out.runnerExitGate78Ed1CProducerKnown =
        runtime.runnerExitGate78Ed1CProducerKnown;
    out.runnerExitGate78Ed1CProducerEventStreamFlag =
        runtime.runnerExitGate78Ed1CProducerEventStreamFlag;
    out.runnerExitGate78Ed1CProducerFlagDescriptorValid =
        runtime.runnerExitGate78Ed1CProducerFlagDescriptorValid;
    out.runnerExitGate78Ed1CProducerActiveFlagStreamValid =
        runtime.runnerExitGate78Ed1CProducerActiveFlagStreamValid;
    out.runnerExitGate78Ed1CProducerActiveFlagStreamIndex =
        runtime.runnerExitGate78Ed1CProducerActiveFlagStreamIndex;
    out.runnerExitGate78Ed1CProducerActiveFlagStreamCount =
        runtime.runnerExitGate78Ed1CProducerActiveFlagStreamCount;
    out.runnerExitGate78Ed1CProducerDueKnown =
        runtime.runnerExitGate78Ed1CProducerDueKnown;
    out.runnerExitGate78Ed1CProducerDueFrame =
        runtime.runnerExitGate78Ed1CProducerDueFrame;
    out.runnerExitGate78Ed1CProducerDueDelta =
        runtime.runnerExitGate78Ed1CProducerDueDelta;
    out.runnerExitGate78Ed1CProducerFlags04 =
        runtime.runnerExitGate78Ed1CProducerFlags04;
    out.runnerExitGate78Ed1CProducerFlag80 =
        runtime.runnerExitGate78Ed1CProducerFlag80;
    out.runnerExitGate78Ed1CProducerConsumedFlagStreamEvent =
        runtime.runnerExitGate78Ed1CProducerConsumedFlagStreamEvent;
    out.runnerExitGate78Ed1CProducerProduced =
        runtime.runnerExitGate78Ed1CProducerProduced;
    out.runnerExitEventStreamFlagLastUpdateKnown =
        runtime.runnerExitEventStreamFlagLastUpdateKnown;
    out.runnerExitEventStreamFlagLastUpdateReason =
        runtime.runnerExitEventStreamFlagLastUpdateReason;
    out.runnerExitEventStreamFlagLastUpdateQueryFrame =
        runtime.runnerExitEventStreamFlagLastUpdateQueryFrame;
    out.runnerExitEventStreamFlagLastUpdateScriptFrame =
        runtime.runnerExitEventStreamFlagLastUpdateScriptFrame;
    out.runnerExitEventStreamFlagLastUpdatePrevious =
        runtime.runnerExitEventStreamFlagLastUpdatePrevious;
    out.runnerExitEventStreamFlagLastUpdateCurrent =
        runtime.runnerExitEventStreamFlagLastUpdateCurrent;
    out.runnerExitEventStreamFlagLastChangeKnown =
        runtime.runnerExitEventStreamFlagLastChangeKnown;
    out.runnerExitEventStreamFlagLastChangeReason =
        runtime.runnerExitEventStreamFlagLastChangeReason;
    out.runnerExitEventStreamFlagLastChangeQueryFrame =
        runtime.runnerExitEventStreamFlagLastChangeQueryFrame;
    out.runnerExitEventStreamFlagLastChangeScriptFrame =
        runtime.runnerExitEventStreamFlagLastChangeScriptFrame;
    out.runnerExitEventStreamFlagLastChangePrevious =
        runtime.runnerExitEventStreamFlagLastChangePrevious;
    out.runnerExitEventStreamFlagLastChangeCurrent =
        runtime.runnerExitEventStreamFlagLastChangeCurrent;
    out.runnerExitFlag40Active = runtime.runnerExitFlag40Active;
    out.runnerExitLateBranchSelectedStream =
        runtime.runnerExitLateBranchSelectedStream;
    out.runnerExitLateBranchActiveDispatchStream =
        runtime.runnerExitLateBranchActiveDispatchStream;
    out.runnerExitFlag100BlocksWaitActive =
        runtime.runnerExitFlag100BlocksWaitActive;
    out.runnerExitFlag100SourceStream =
        runtime.runnerExitFlag100SourceStream;
    out.runnerExitLateBranchScriptFrame =
        runtime.runnerExitLateBranchScriptFrame;
    out.runnerExitClearTerminalTailPulseInput =
        runtime.runnerExitClearTerminalTailPulseInput;
    out.runnerExitClearTerminalTailPulseArmed =
        runtime.runnerExitClearTerminalTailPulseArmed;
    out.runnerExitClearTerminalTailPulseBlockedAlreadyArmed =
        runtime.runnerExitClearTerminalTailPulseBlockedAlreadyArmed;
    out.runnerExitClearTerminalTailPulseBlockedActiveDispatch =
        runtime.runnerExitClearTerminalTailPulseBlockedActiveDispatch;
    out.runnerExitClearTerminalTailPulseBlockedPendingMismatch =
        runtime.runnerExitClearTerminalTailPulseBlockedPendingMismatch;
    out.runnerExitClearTerminalTailPulseStream =
        runtime.runnerExitClearTerminalTailPulseStream;
    out.runnerExitClearTerminalBranchTriggerAttempted =
        runtime.runnerExitClearTerminalBranchTriggerAttempted;
    out.runnerExitClearTerminalBranchTriggerAccepted =
        runtime.runnerExitClearTerminalBranchTriggerAccepted;
    out.runnerExitClearTerminalBranchTriggerScriptFrame =
        runtime.runnerExitClearTerminalBranchTriggerScriptFrame;
    out.runnerExitClearTerminalBranchTriggerRightRankRow =
        runtime.runnerExitClearTerminalBranchTriggerRightRankRow;
    out.runnerExitClearTerminalBranchTriggerCurrentMode =
        runtime.runnerExitClearTerminalBranchTriggerCurrentMode;
    out.runnerExitClearTerminalBranchTriggerBlockedConsumed =
        runtime.runnerExitClearTerminalBranchTriggerBlockedConsumed;
    out.runnerExitClearTerminalBranchTriggerBlockedArmed =
        runtime.runnerExitClearTerminalBranchTriggerBlockedArmed;
    out.runnerExitClearTerminalBranchTriggerBlockedFlagNotOne =
        runtime.runnerExitClearTerminalBranchTriggerBlockedFlagNotOne;
    out.runnerExitClearTerminalBranchTriggerBlockedRow =
        runtime.runnerExitClearTerminalBranchTriggerBlockedRow;
    out.runnerExitClearTerminalBranchTriggerBlockedStreamMissing =
        runtime.runnerExitClearTerminalBranchTriggerBlockedStreamMissing;
    out.runnerExitClearTerminalBranchTriggerBlockedCursorDone =
        runtime.runnerExitClearTerminalBranchTriggerBlockedCursorDone;
    out.runnerExitClearTerminalBranchTriggerBlockedEventNotDue =
        runtime.runnerExitClearTerminalBranchTriggerBlockedEventNotDue;
    out.runnerExitClearTerminalBranchTriggerBlockedMissingFlag80 =
        runtime.runnerExitClearTerminalBranchTriggerBlockedMissingFlag80;
    out.runnerExitClearTerminalBranchTriggerStreamFlag =
        runtime.runnerExitClearTerminalBranchTriggerStreamFlag;
    out.runnerExitClearTerminalBranchTriggerAttemptCount =
        runtime.runnerExitClearTerminalBranchTriggerAttemptCount;
    out.runnerExitClearTerminalBranchTriggerAcceptedCount =
        runtime.runnerExitClearTerminalBranchTriggerAcceptedCount;
    out.runnerExitClearTerminalBranchTriggerEligibleCount =
        runtime.runnerExitClearTerminalBranchTriggerEligibleCount;
    out.runnerExitClearTerminalBranchTriggerBlockedFlagNotOneCount =
        runtime.runnerExitClearTerminalBranchTriggerBlockedFlagNotOneCount;
    out.runnerExitClearTerminalBranchTriggerBlockedRowCount =
        runtime.runnerExitClearTerminalBranchTriggerBlockedRowCount;
    out.runnerExitClearTerminalBranchTriggerBlockedFlagAndRowCount =
        runtime.runnerExitClearTerminalBranchTriggerBlockedFlagAndRowCount;
    out.runnerExitClearTerminalBranchTriggerBlockedConsumedCount =
        runtime.runnerExitClearTerminalBranchTriggerBlockedConsumedCount;
    out.runnerExitClearTerminalBranchTriggerBlockedArmedCount =
        runtime.runnerExitClearTerminalBranchTriggerBlockedArmedCount;
    out.runnerExitClearTerminalBranchTriggerBlockedStreamMissingCount =
        runtime.runnerExitClearTerminalBranchTriggerBlockedStreamMissingCount;
    out.runnerExitClearTerminalBranchTriggerBlockedCursorDoneCount =
        runtime.runnerExitClearTerminalBranchTriggerBlockedCursorDoneCount;
    out.runnerExitClearTerminalBranchTriggerBlockedEventNotDueCount =
        runtime.runnerExitClearTerminalBranchTriggerBlockedEventNotDueCount;
    out.runnerExitClearTerminalBranchTriggerBlockedMissingFlag80Count =
        runtime.runnerExitClearTerminalBranchTriggerBlockedMissingFlag80Count;
    out.runnerExitFirstClearTerminalBranchTriggerKnown =
        runtime.runnerExitFirstClearTerminalBranchTriggerKnown;
    out.runnerExitFirstClearTerminalBranchTriggerScriptFrame =
        runtime.runnerExitFirstClearTerminalBranchTriggerScriptFrame;
    out.runnerExitFirstClearTerminalBranchTriggerRightRankRow =
        runtime.runnerExitFirstClearTerminalBranchTriggerRightRankRow;
    out.runnerExitFirstClearTerminalBranchTriggerCurrentMode =
        runtime.runnerExitFirstClearTerminalBranchTriggerCurrentMode;
    out.runnerExitFirstClearTerminalBranchTriggerStreamFlag =
        runtime.runnerExitFirstClearTerminalBranchTriggerStreamFlag;
    out.runnerExitFirstClearTerminalBranchTriggerEligibleKnown =
        runtime.runnerExitFirstClearTerminalBranchTriggerEligibleKnown;
    out.runnerExitFirstClearTerminalBranchTriggerEligibleScriptFrame =
        runtime.runnerExitFirstClearTerminalBranchTriggerEligibleScriptFrame;
    out.runnerExitFirstClearTerminalBranchTriggerEligibleRightRankRow =
        runtime.runnerExitFirstClearTerminalBranchTriggerEligibleRightRankRow;
    out.runnerExitFirstClearTerminalBranchTriggerEligibleCurrentMode =
        runtime.runnerExitFirstClearTerminalBranchTriggerEligibleCurrentMode;
    out.runnerExitFirstClearTerminalBranchTriggerEligibleStream1Known =
        runtime.runnerExitFirstClearTerminalBranchTriggerEligibleStream1Known;
    out.runnerExitFirstClearTerminalBranchTriggerEligibleStream1Cursor =
        runtime.runnerExitFirstClearTerminalBranchTriggerEligibleStream1Cursor;
    out.runnerExitFirstClearTerminalBranchTriggerEligibleStream1Count =
        runtime.runnerExitFirstClearTerminalBranchTriggerEligibleStream1Count;
    out.runnerExitFirstClearTerminalBranchTriggerEligibleStream1DueKnown =
        runtime.runnerExitFirstClearTerminalBranchTriggerEligibleStream1DueKnown;
    out.runnerExitFirstClearTerminalBranchTriggerEligibleStream1DueFrame =
        runtime.runnerExitFirstClearTerminalBranchTriggerEligibleStream1DueFrame;
    out.runnerExitFirstClearTerminalBranchTriggerEligibleStream1DueDelta =
        runtime.runnerExitFirstClearTerminalBranchTriggerEligibleStream1DueDelta;
    out.runnerExitFirstClearTerminalBranchTriggerEligibleStream1BaseFrame =
        runtime.runnerExitFirstClearTerminalBranchTriggerEligibleStream1BaseFrame;
    out.runnerExitFirstClearTerminalBranchTriggerEligibleStream1AbsDueFrame =
        runtime.runnerExitFirstClearTerminalBranchTriggerEligibleStream1AbsDueFrame;
    out.runnerExitFirstClearTerminalBranchTriggerEligibleStream1AbsDueDelta =
        runtime.runnerExitFirstClearTerminalBranchTriggerEligibleStream1AbsDueDelta;
    out.runnerExitFirstClearTerminalBranchTriggerEligibleStream1Flags04 =
        runtime.runnerExitFirstClearTerminalBranchTriggerEligibleStream1Flags04;
    out.runnerExitFirstClearTerminalBranchTriggerEligibleStream1Byte29 =
        runtime.runnerExitFirstClearTerminalBranchTriggerEligibleStream1Byte29;
    out.runnerExitFirstClearTerminalBranchTriggerEligibleStream1Byte30 =
        runtime.runnerExitFirstClearTerminalBranchTriggerEligibleStream1Byte30;
    out.runnerExitFirstClearTerminalBranchTriggerEligibleStream1NextFlag80SearchKnown =
        runtime.runnerExitFirstClearTerminalBranchTriggerEligibleStream1NextFlag80SearchKnown;
    out.runnerExitFirstClearTerminalBranchTriggerEligibleStream1NextFlag80Known =
        runtime.runnerExitFirstClearTerminalBranchTriggerEligibleStream1NextFlag80Known;
    out.runnerExitFirstClearTerminalBranchTriggerEligibleStream1NextFlag80Cursor =
        runtime.runnerExitFirstClearTerminalBranchTriggerEligibleStream1NextFlag80Cursor;
    out.runnerExitFirstClearTerminalBranchTriggerEligibleStream1NextFlag80Frame =
        runtime.runnerExitFirstClearTerminalBranchTriggerEligibleStream1NextFlag80Frame;
    out.runnerExitFirstClearTerminalBranchTriggerEligibleStream1NextFlag80Delta =
        runtime.runnerExitFirstClearTerminalBranchTriggerEligibleStream1NextFlag80Delta;
    out.runnerExitFirstClearTerminalBranchTriggerEligibleStream1NextFlag80BaseFrame =
        runtime.runnerExitFirstClearTerminalBranchTriggerEligibleStream1NextFlag80BaseFrame;
    out.runnerExitFirstClearTerminalBranchTriggerEligibleStream1NextFlag80AbsFrame =
        runtime.runnerExitFirstClearTerminalBranchTriggerEligibleStream1NextFlag80AbsFrame;
    out.runnerExitFirstClearTerminalBranchTriggerEligibleStream1NextFlag80AbsDelta =
        runtime.runnerExitFirstClearTerminalBranchTriggerEligibleStream1NextFlag80AbsDelta;
    out.runnerExitFirstClearTerminalBranchTriggerEligibleStream1NextFlag80Flags04 =
        runtime.runnerExitFirstClearTerminalBranchTriggerEligibleStream1NextFlag80Flags04;
    out.runnerExitFirstClearTerminalBranchTriggerEligibleStream1NextFlag80Byte29 =
        runtime.runnerExitFirstClearTerminalBranchTriggerEligibleStream1NextFlag80Byte29;
    out.runnerExitFirstClearTerminalBranchTriggerEligibleStream1NextFlag80Byte30 =
        runtime.runnerExitFirstClearTerminalBranchTriggerEligibleStream1NextFlag80Byte30;
    out.runnerExitClearTerminalBranchTriggerStream1Cursor =
        runtime.runnerExitClearTerminalBranchTriggerStream1Cursor;
    out.runnerExitClearTerminalBranchTriggerStream1Count =
        runtime.runnerExitClearTerminalBranchTriggerStream1Count;
    out.runnerExitClearTerminalBranchTriggerStream1DueKnown =
        runtime.runnerExitClearTerminalBranchTriggerStream1DueKnown;
    out.runnerExitClearTerminalBranchTriggerStream1DueFrame =
        runtime.runnerExitClearTerminalBranchTriggerStream1DueFrame;
    out.runnerExitClearTerminalBranchTriggerStream1DueDelta =
        runtime.runnerExitClearTerminalBranchTriggerStream1DueDelta;
    out.runnerExitClearTerminalBranchTriggerStream1BaseFrame =
        runtime.runnerExitClearTerminalBranchTriggerStream1BaseFrame;
    out.runnerExitClearTerminalBranchTriggerStream1AbsDueFrame =
        runtime.runnerExitClearTerminalBranchTriggerStream1AbsDueFrame;
    out.runnerExitClearTerminalBranchTriggerStream1AbsDueDelta =
        runtime.runnerExitClearTerminalBranchTriggerStream1AbsDueDelta;
    out.runnerExitClearTerminalBranchTriggerStream1Flags04 =
        runtime.runnerExitClearTerminalBranchTriggerStream1Flags04;
    out.runnerExitClearTerminalBranchTriggerStream1Byte29 =
        runtime.runnerExitClearTerminalBranchTriggerStream1Byte29;
    out.runnerExitClearTerminalBranchTriggerStream1Byte30 =
        runtime.runnerExitClearTerminalBranchTriggerStream1Byte30;
    out.runnerExitFirstClearTerminalBranchTriggerEventNotDueKnown =
        runtime.runnerExitFirstClearTerminalBranchTriggerEventNotDueKnown;
    out.runnerExitFirstClearTerminalBranchTriggerEventNotDueScriptFrame =
        runtime.runnerExitFirstClearTerminalBranchTriggerEventNotDueScriptFrame;
    out.runnerExitFirstClearTerminalBranchTriggerEventNotDueCursor =
        runtime.runnerExitFirstClearTerminalBranchTriggerEventNotDueCursor;
    out.runnerExitFirstClearTerminalBranchTriggerEventNotDueFrame =
        runtime.runnerExitFirstClearTerminalBranchTriggerEventNotDueFrame;
    out.runnerExitFirstClearTerminalBranchTriggerEventNotDueDelta =
        runtime.runnerExitFirstClearTerminalBranchTriggerEventNotDueDelta;
    out.runnerExitFirstClearTerminalBranchTriggerEventNotDueBaseFrame =
        runtime.runnerExitFirstClearTerminalBranchTriggerEventNotDueBaseFrame;
    out.runnerExitFirstClearTerminalBranchTriggerEventNotDueAbsFrame =
        runtime.runnerExitFirstClearTerminalBranchTriggerEventNotDueAbsFrame;
    out.runnerExitFirstClearTerminalBranchTriggerEventNotDueAbsDelta =
        runtime.runnerExitFirstClearTerminalBranchTriggerEventNotDueAbsDelta;
    out.runnerExitFirstClearTerminalBranchTriggerEventNotDueFlags04 =
        runtime.runnerExitFirstClearTerminalBranchTriggerEventNotDueFlags04;
    out.runnerExitFirstClearTerminalBranchTriggerMissingFlag80Known =
        runtime.runnerExitFirstClearTerminalBranchTriggerMissingFlag80Known;
    out.runnerExitFirstClearTerminalBranchTriggerMissingFlag80ScriptFrame =
        runtime.runnerExitFirstClearTerminalBranchTriggerMissingFlag80ScriptFrame;
    out.runnerExitFirstClearTerminalBranchTriggerMissingFlag80Cursor =
        runtime.runnerExitFirstClearTerminalBranchTriggerMissingFlag80Cursor;
    out.runnerExitFirstClearTerminalBranchTriggerMissingFlag80Frame =
        runtime.runnerExitFirstClearTerminalBranchTriggerMissingFlag80Frame;
    out.runnerExitFirstClearTerminalBranchTriggerMissingFlag80Delta =
        runtime.runnerExitFirstClearTerminalBranchTriggerMissingFlag80Delta;
    out.runnerExitFirstClearTerminalBranchTriggerMissingFlag80BaseFrame =
        runtime.runnerExitFirstClearTerminalBranchTriggerMissingFlag80BaseFrame;
    out.runnerExitFirstClearTerminalBranchTriggerMissingFlag80AbsFrame =
        runtime.runnerExitFirstClearTerminalBranchTriggerMissingFlag80AbsFrame;
    out.runnerExitFirstClearTerminalBranchTriggerMissingFlag80AbsDelta =
        runtime.runnerExitFirstClearTerminalBranchTriggerMissingFlag80AbsDelta;
    out.runnerExitFirstClearTerminalBranchTriggerMissingFlag80Flags04 =
        runtime.runnerExitFirstClearTerminalBranchTriggerMissingFlag80Flags04;
    out.runnerExitClearTerminalTailServiceLatch =
        runtime.runnerExitClearTerminalTailServiceLatch;
    out.runnerExitClearTerminalTailServiceScriptFrame =
        runtime.runnerExitClearTerminalTailServiceScriptFrame;
    out.runnerExitClearTerminalTailServiceRightRankRow =
        runtime.runnerExitClearTerminalTailServiceRightRankRow;
    out.runnerExitClearTerminalTailServiceCurrentMode =
        runtime.runnerExitClearTerminalTailServiceCurrentMode;
    out.runnerExitClearTerminalTailServiceStream =
        runtime.runnerExitClearTerminalTailServiceStream;
    out.runnerExitPendingRatingBranchSeq =
        runtime.runnerExitPendingRatingBranchSeq;
    out.runnerExitConsumedRatingBranchSeq =
        runtime.runnerExitConsumedRatingBranchSeq;
    out.runnerExitClearTerminalTailArmed =
        runtime.runnerExitClearTerminalTailArmed;
    out.runnerExitClearTerminalTailDispatchActive =
        runtime.runnerExitClearTerminalTailDispatchActive;
    out.runnerExitClearTerminalTailStream =
        runtime.runnerExitClearTerminalTailStream;
    out.runnerExitActiveDispatchStartScriptFrame =
        runtime.runnerExitActiveDispatchStartScriptFrame;
    out.runnerExitActiveDispatchTerminalEndLocalFrame =
        runtime.runnerExitActiveDispatchTerminalEndLocalFrame;
    out.runnerExitActiveDispatchTerminalPulseEmitted =
        runtime.runnerExitActiveDispatchTerminalPulseEmitted;
    out.runnerTailRecordsModeDAActive = runtime.runnerTailRecordsModeDAActive;
    out.runnerTailRecordsModeDAEqualsOne =
        runtime.runnerTailRecordsModeDAEqualsOne;
    out.runnerTailStageStatusKnown = runtime.runnerTailStageStatusKnown;
    out.runnerTailStageStatus166AC = runtime.runnerTailStageStatus166AC;
    out.runnerTailCleanupRenderPassBudget =
        runtime.runnerTailCleanupRenderPassBudget;
    out.runnerTailFinalReturnKnown = runtime.runnerTailFinalReturnKnown;
    out.runnerTailFinalReturn = runtime.runnerTailFinalReturn;
    out.runnerTailFinalReturnWord59Known =
        runtime.runnerTailFinalReturnWord59Known;
    out.runnerTailFinalReturnWord59IsOne =
        runtime.runnerTailFinalReturnWord59IsOne;
    out.runnerTailFinalReturnWord60Known =
        runtime.runnerTailFinalReturnWord60Known;
    out.runnerTailFinalReturnWord60IsOne =
        runtime.runnerTailFinalReturnWord60IsOne;
    out.runnerTailFinalReturnCalls166AC =
        runtime.runnerTailFinalReturnCalls166AC;
    out.runnerTailFinalReturnRecordsModeReturnsOne =
        runtime.runnerTailFinalReturnRecordsModeReturnsOne;
    out.runnerTailFinalReturnStageStatusReturnsOne =
        runtime.runnerTailFinalReturnStageStatusReturnsOne;
}

void RefreshStage1FormalLifecycleExitResultObserverRuntimeImpl(
    const Stage1FormalLifecycleFrameInputs& inputs,
    Stage1FormalLifecycleRuntime& runtime) {
    runtime.runnerExitLatch76SourceBucket0 =
        s_stage1RunnerTailGateRuntime.ctx76Bucket0Latched;
    runtime.runnerExitLatch76SourceTailStream4 =
        s_stage1RunnerTailGateRuntime.ctx76TailStream4Latched;
    runtime.runnerExitLatch76SourceLowLevelAbort =
        s_stage1RunnerTailGateRuntime.ctx76LowLevelAbortLatched;
    runtime.runnerExitLatch76Bucket0PulseDetailKnown =
        s_stage1RunnerTailGateRuntime.ctx76Bucket0PulseDetailKnown;
    runtime.runnerExitLatch76Bucket0PulseQueryFrame =
        s_stage1RunnerTailGateRuntime.ctx76Bucket0PulseQueryFrame;
    runtime.runnerExitLatch76Bucket0PulseRightRankRow =
        s_stage1RunnerTailGateRuntime.ctx76Bucket0PulseRightRankRow;
    runtime.runnerExitLatch76Bucket0PulseCallWindowOpen =
        s_stage1RunnerTailGateRuntime.ctx76Bucket0PulseCallWindowOpen;
    runtime.runnerExitLatch76Bucket0PulseDescriptorFlags =
        s_stage1RunnerTailGateRuntime.ctx76Bucket0PulseDescriptorFlags;
    runtime.runnerExitLatch76Known =
        s_stage1RunnerTailGateRuntime.ctx76Known ||
        runtime.runnerExitLatch76SourceBucket0 ||
        runtime.runnerExitLatch76SourceTailStream4 ||
        runtime.runnerExitLatch76SourceLowLevelAbort;
    runtime.runnerExitLatch76Active =
        runtime.runnerExitLatch76SourceBucket0 ||
        runtime.runnerExitLatch76SourceTailStream4 ||
        runtime.runnerExitLatch76SourceLowLevelAbort ||
        (s_stage1RunnerTailGateRuntime.ctx76Known &&
         s_stage1RunnerTailGateRuntime.ctx76Active);
    runtime.runnerExitGate78SourceEd1CHandoff =
        s_stage1RunnerTailGateRuntime.ctx78Ed1CHandoffLatched;
    runtime.runnerExitGate78FrameUpdate9094Known =
        s_stage1RunnerTailGateRuntime.ctx78FrameUpdate9094Known;
    runtime.runnerExitGate78FrameUpdate9094QueryFrame =
        s_stage1RunnerTailGateRuntime.ctx78FrameUpdate9094QueryFrame;
    runtime.runnerExitGate78FrameUpdate9094Ctx76 =
        s_stage1RunnerTailGateRuntime.ctx78FrameUpdate9094Ctx76;
    runtime.runnerExitGate78FrameUpdate9094Ctx78 =
        s_stage1RunnerTailGateRuntime.ctx78FrameUpdate9094Ctx78;
    runtime.runnerExitGate78FrameUpdate9094TailFamilyActive =
        s_stage1RunnerTailGateRuntime.ctx78FrameUpdate9094TailFamilyActive;
    runtime.runnerExitGate78FrameUpdate9094TailStream =
        s_stage1RunnerTailGateRuntime.ctx78FrameUpdate9094TailStream;
    runtime.runnerExitGate78FrameUpdate9094ActiveStream =
        s_stage1RunnerTailGateRuntime.ctx78FrameUpdate9094ActiveStream;
    runtime.runnerExitGate78Ed1CProducerKnown =
        s_stage1RunnerTailGateRuntime.ctx78Ed1CProducerKnown;
    runtime.runnerExitGate78Ed1CProducerEventStreamFlag =
        s_stage1RunnerTailGateRuntime.ctx78Ed1CProducerEventStreamFlag;
    runtime.runnerExitGate78Ed1CProducerFlagDescriptorValid =
        s_stage1RunnerTailGateRuntime.ctx78Ed1CProducerFlagDescriptorValid;
    runtime.runnerExitGate78Ed1CProducerActiveFlagStreamValid =
        s_stage1RunnerTailGateRuntime.ctx78Ed1CProducerActiveFlagStreamValid;
    runtime.runnerExitGate78Ed1CProducerActiveFlagStreamIndex =
        s_stage1RunnerTailGateRuntime.ctx78Ed1CProducerActiveFlagStreamIndex;
    runtime.runnerExitGate78Ed1CProducerActiveFlagStreamCount =
        s_stage1RunnerTailGateRuntime.ctx78Ed1CProducerActiveFlagStreamCount;
    runtime.runnerExitGate78Ed1CProducerDueKnown =
        s_stage1RunnerTailGateRuntime.ctx78Ed1CProducerDueKnown;
    runtime.runnerExitGate78Ed1CProducerDueFrame =
        s_stage1RunnerTailGateRuntime.ctx78Ed1CProducerDueFrame;
    runtime.runnerExitGate78Ed1CProducerDueDelta =
        s_stage1RunnerTailGateRuntime.ctx78Ed1CProducerDueDelta;
    runtime.runnerExitGate78Ed1CProducerFlags04 =
        s_stage1RunnerTailGateRuntime.ctx78Ed1CProducerFlags04;
    runtime.runnerExitGate78Ed1CProducerFlag80 =
        s_stage1RunnerTailGateRuntime.ctx78Ed1CProducerFlag80;
    runtime.runnerExitGate78Ed1CProducerConsumedFlagStreamEvent =
        s_stage1RunnerTailGateRuntime.ctx78Ed1CProducerConsumedFlagStreamEvent;
    runtime.runnerExitGate78Ed1CProducerProduced =
        s_stage1RunnerTailGateRuntime.ctx78Ed1CProducerProduced;
    runtime.runnerExitEventStreamFlagLastUpdateKnown =
        s_stage1RunnerTailGateRuntime.eventStreamFlagLastUpdateKnown;
    runtime.runnerExitEventStreamFlagLastUpdateReason =
        s_stage1RunnerTailGateRuntime.eventStreamFlagLastUpdateReason;
    runtime.runnerExitEventStreamFlagLastUpdateQueryFrame =
        s_stage1RunnerTailGateRuntime.eventStreamFlagLastUpdateQueryFrame;
    runtime.runnerExitEventStreamFlagLastUpdateScriptFrame =
        s_stage1RunnerTailGateRuntime.eventStreamFlagLastUpdateScriptFrame;
    runtime.runnerExitEventStreamFlagLastUpdatePrevious =
        s_stage1RunnerTailGateRuntime.eventStreamFlagLastUpdatePrevious;
    runtime.runnerExitEventStreamFlagLastUpdateCurrent =
        s_stage1RunnerTailGateRuntime.eventStreamFlagLastUpdateCurrent;
    runtime.runnerExitEventStreamFlagLastChangeKnown =
        s_stage1RunnerTailGateRuntime.eventStreamFlagLastChangeKnown;
    runtime.runnerExitEventStreamFlagLastChangeReason =
        s_stage1RunnerTailGateRuntime.eventStreamFlagLastChangeReason;
    runtime.runnerExitEventStreamFlagLastChangeQueryFrame =
        s_stage1RunnerTailGateRuntime.eventStreamFlagLastChangeQueryFrame;
    runtime.runnerExitEventStreamFlagLastChangeScriptFrame =
        s_stage1RunnerTailGateRuntime.eventStreamFlagLastChangeScriptFrame;
    runtime.runnerExitEventStreamFlagLastChangePrevious =
        s_stage1RunnerTailGateRuntime.eventStreamFlagLastChangePrevious;
    runtime.runnerExitEventStreamFlagLastChangeCurrent =
        s_stage1RunnerTailGateRuntime.eventStreamFlagLastChangeCurrent;
    runtime.runnerExitGate78Known =
        s_stage1RunnerTailGateRuntime.ctx78Known ||
        runtime.runnerExitGate78SourceEd1CHandoff;
    runtime.runnerExitGate78Active =
        runtime.runnerExitGate78SourceEd1CHandoff ||
        (s_stage1RunnerTailGateRuntime.ctx78Known &&
         s_stage1RunnerTailGateRuntime.ctx78Active);
    runtime.runnerExitFlag40Active = inputs.lateBranchFlag40Active;
    runtime.runnerExitLateBranchSelectedStream = inputs.lateBranchSelectedStream;
    runtime.runnerExitLateBranchActiveDispatchStream =
        inputs.lateBranchActiveDispatchStream;
    runtime.runnerExitFlag100BlocksWaitActive =
        inputs.lateBranchFlag100BlocksWaitActive;
    runtime.runnerExitFlag100SourceStream =
        inputs.lateBranchFlag100SourceStream;
    runtime.runnerExitLateBranchScriptFrame =
        inputs.lateBranchScriptFrame;
    runtime.runnerExitClearTerminalTailPulseInput =
        inputs.lateBranchClearTerminalTailPulseInput;
    runtime.runnerExitClearTerminalTailPulseArmed =
        inputs.lateBranchClearTerminalTailPulseArmed;
    runtime.runnerExitClearTerminalTailPulseBlockedAlreadyArmed =
        inputs.lateBranchClearTerminalTailPulseBlockedAlreadyArmed;
    runtime.runnerExitClearTerminalTailPulseBlockedActiveDispatch =
        inputs.lateBranchClearTerminalTailPulseBlockedActiveDispatch;
    runtime.runnerExitClearTerminalTailPulseBlockedPendingMismatch =
        inputs.lateBranchClearTerminalTailPulseBlockedPendingMismatch;
    runtime.runnerExitClearTerminalTailPulseStream =
        inputs.lateBranchClearTerminalTailPulseStream;
    runtime.runnerExitClearTerminalBranchTriggerAttempted =
        inputs.lateBranchClearTerminalBranchTriggerAttempted;
    runtime.runnerExitClearTerminalBranchTriggerAccepted =
        inputs.lateBranchClearTerminalBranchTriggerAccepted;
    runtime.runnerExitClearTerminalBranchTriggerScriptFrame =
        inputs.lateBranchClearTerminalBranchTriggerScriptFrame;
    runtime.runnerExitClearTerminalBranchTriggerRightRankRow =
        inputs.lateBranchClearTerminalBranchTriggerRightRankRow;
    runtime.runnerExitClearTerminalBranchTriggerCurrentMode =
        inputs.lateBranchClearTerminalBranchTriggerCurrentMode;
    runtime.runnerExitClearTerminalBranchTriggerBlockedConsumed =
        inputs.lateBranchClearTerminalBranchTriggerBlockedConsumed;
    runtime.runnerExitClearTerminalBranchTriggerBlockedArmed =
        inputs.lateBranchClearTerminalBranchTriggerBlockedArmed;
    runtime.runnerExitClearTerminalBranchTriggerBlockedFlagNotOne =
        inputs.lateBranchClearTerminalBranchTriggerBlockedFlagNotOne;
    runtime.runnerExitClearTerminalBranchTriggerBlockedRow =
        inputs.lateBranchClearTerminalBranchTriggerBlockedRow;
    runtime.runnerExitClearTerminalBranchTriggerBlockedStreamMissing =
        inputs.lateBranchClearTerminalBranchTriggerBlockedStreamMissing;
    runtime.runnerExitClearTerminalBranchTriggerBlockedCursorDone =
        inputs.lateBranchClearTerminalBranchTriggerBlockedCursorDone;
    runtime.runnerExitClearTerminalBranchTriggerBlockedEventNotDue =
        inputs.lateBranchClearTerminalBranchTriggerBlockedEventNotDue;
    runtime.runnerExitClearTerminalBranchTriggerBlockedMissingFlag80 =
        inputs.lateBranchClearTerminalBranchTriggerBlockedMissingFlag80;
    runtime.runnerExitClearTerminalBranchTriggerStreamFlag =
        inputs.lateBranchClearTerminalBranchTriggerStreamFlag;
    runtime.runnerExitClearTerminalBranchTriggerAttemptCount =
        inputs.lateBranchClearTerminalBranchTriggerAttemptCount;
    runtime.runnerExitClearTerminalBranchTriggerAcceptedCount =
        inputs.lateBranchClearTerminalBranchTriggerAcceptedCount;
    runtime.runnerExitClearTerminalBranchTriggerEligibleCount =
        inputs.lateBranchClearTerminalBranchTriggerEligibleCount;
    runtime.runnerExitClearTerminalBranchTriggerBlockedFlagNotOneCount =
        inputs.lateBranchClearTerminalBranchTriggerBlockedFlagNotOneCount;
    runtime.runnerExitClearTerminalBranchTriggerBlockedRowCount =
        inputs.lateBranchClearTerminalBranchTriggerBlockedRowCount;
    runtime.runnerExitClearTerminalBranchTriggerBlockedFlagAndRowCount =
        inputs.lateBranchClearTerminalBranchTriggerBlockedFlagAndRowCount;
    runtime.runnerExitClearTerminalBranchTriggerBlockedConsumedCount =
        inputs.lateBranchClearTerminalBranchTriggerBlockedConsumedCount;
    runtime.runnerExitClearTerminalBranchTriggerBlockedArmedCount =
        inputs.lateBranchClearTerminalBranchTriggerBlockedArmedCount;
    runtime.runnerExitClearTerminalBranchTriggerBlockedStreamMissingCount =
        inputs.lateBranchClearTerminalBranchTriggerBlockedStreamMissingCount;
    runtime.runnerExitClearTerminalBranchTriggerBlockedCursorDoneCount =
        inputs.lateBranchClearTerminalBranchTriggerBlockedCursorDoneCount;
    runtime.runnerExitClearTerminalBranchTriggerBlockedEventNotDueCount =
        inputs.lateBranchClearTerminalBranchTriggerBlockedEventNotDueCount;
    runtime.runnerExitClearTerminalBranchTriggerBlockedMissingFlag80Count =
        inputs.lateBranchClearTerminalBranchTriggerBlockedMissingFlag80Count;
    runtime.runnerExitFirstClearTerminalBranchTriggerKnown =
        inputs.lateBranchFirstClearTerminalBranchTriggerKnown;
    runtime.runnerExitFirstClearTerminalBranchTriggerScriptFrame =
        inputs.lateBranchFirstClearTerminalBranchTriggerScriptFrame;
    runtime.runnerExitFirstClearTerminalBranchTriggerRightRankRow =
        inputs.lateBranchFirstClearTerminalBranchTriggerRightRankRow;
    runtime.runnerExitFirstClearTerminalBranchTriggerCurrentMode =
        inputs.lateBranchFirstClearTerminalBranchTriggerCurrentMode;
    runtime.runnerExitFirstClearTerminalBranchTriggerStreamFlag =
        inputs.lateBranchFirstClearTerminalBranchTriggerStreamFlag;
    runtime.runnerExitFirstClearTerminalBranchTriggerEligibleKnown =
        inputs.lateBranchFirstClearTerminalBranchTriggerEligibleKnown;
    runtime.runnerExitFirstClearTerminalBranchTriggerEligibleScriptFrame =
        inputs.lateBranchFirstClearTerminalBranchTriggerEligibleScriptFrame;
    runtime.runnerExitFirstClearTerminalBranchTriggerEligibleRightRankRow =
        inputs.lateBranchFirstClearTerminalBranchTriggerEligibleRightRankRow;
    runtime.runnerExitFirstClearTerminalBranchTriggerEligibleCurrentMode =
        inputs.lateBranchFirstClearTerminalBranchTriggerEligibleCurrentMode;
    runtime.runnerExitFirstClearTerminalBranchTriggerEligibleStream1Known =
        inputs.lateBranchFirstClearTerminalBranchTriggerEligibleStream1Known;
    runtime.runnerExitFirstClearTerminalBranchTriggerEligibleStream1Cursor =
        inputs.lateBranchFirstClearTerminalBranchTriggerEligibleStream1Cursor;
    runtime.runnerExitFirstClearTerminalBranchTriggerEligibleStream1Count =
        inputs.lateBranchFirstClearTerminalBranchTriggerEligibleStream1Count;
    runtime.runnerExitFirstClearTerminalBranchTriggerEligibleStream1DueKnown =
        inputs.lateBranchFirstClearTerminalBranchTriggerEligibleStream1DueKnown;
    runtime.runnerExitFirstClearTerminalBranchTriggerEligibleStream1DueFrame =
        inputs.lateBranchFirstClearTerminalBranchTriggerEligibleStream1DueFrame;
    runtime.runnerExitFirstClearTerminalBranchTriggerEligibleStream1DueDelta =
        inputs.lateBranchFirstClearTerminalBranchTriggerEligibleStream1DueDelta;
    runtime.runnerExitFirstClearTerminalBranchTriggerEligibleStream1BaseFrame =
        inputs.lateBranchFirstClearTerminalBranchTriggerEligibleStream1BaseFrame;
    runtime.runnerExitFirstClearTerminalBranchTriggerEligibleStream1AbsDueFrame =
        inputs.lateBranchFirstClearTerminalBranchTriggerEligibleStream1AbsDueFrame;
    runtime.runnerExitFirstClearTerminalBranchTriggerEligibleStream1AbsDueDelta =
        inputs.lateBranchFirstClearTerminalBranchTriggerEligibleStream1AbsDueDelta;
    runtime.runnerExitFirstClearTerminalBranchTriggerEligibleStream1Flags04 =
        inputs.lateBranchFirstClearTerminalBranchTriggerEligibleStream1Flags04;
    runtime.runnerExitFirstClearTerminalBranchTriggerEligibleStream1Byte29 =
        inputs.lateBranchFirstClearTerminalBranchTriggerEligibleStream1Byte29;
    runtime.runnerExitFirstClearTerminalBranchTriggerEligibleStream1Byte30 =
        inputs.lateBranchFirstClearTerminalBranchTriggerEligibleStream1Byte30;
    runtime.runnerExitFirstClearTerminalBranchTriggerEligibleStream1NextFlag80SearchKnown =
        inputs.lateBranchFirstClearTerminalBranchTriggerEligibleStream1NextFlag80SearchKnown;
    runtime.runnerExitFirstClearTerminalBranchTriggerEligibleStream1NextFlag80Known =
        inputs.lateBranchFirstClearTerminalBranchTriggerEligibleStream1NextFlag80Known;
    runtime.runnerExitFirstClearTerminalBranchTriggerEligibleStream1NextFlag80Cursor =
        inputs.lateBranchFirstClearTerminalBranchTriggerEligibleStream1NextFlag80Cursor;
    runtime.runnerExitFirstClearTerminalBranchTriggerEligibleStream1NextFlag80Frame =
        inputs.lateBranchFirstClearTerminalBranchTriggerEligibleStream1NextFlag80Frame;
    runtime.runnerExitFirstClearTerminalBranchTriggerEligibleStream1NextFlag80Delta =
        inputs.lateBranchFirstClearTerminalBranchTriggerEligibleStream1NextFlag80Delta;
    runtime.runnerExitFirstClearTerminalBranchTriggerEligibleStream1NextFlag80BaseFrame =
        inputs.lateBranchFirstClearTerminalBranchTriggerEligibleStream1NextFlag80BaseFrame;
    runtime.runnerExitFirstClearTerminalBranchTriggerEligibleStream1NextFlag80AbsFrame =
        inputs.lateBranchFirstClearTerminalBranchTriggerEligibleStream1NextFlag80AbsFrame;
    runtime.runnerExitFirstClearTerminalBranchTriggerEligibleStream1NextFlag80AbsDelta =
        inputs.lateBranchFirstClearTerminalBranchTriggerEligibleStream1NextFlag80AbsDelta;
    runtime.runnerExitFirstClearTerminalBranchTriggerEligibleStream1NextFlag80Flags04 =
        inputs.lateBranchFirstClearTerminalBranchTriggerEligibleStream1NextFlag80Flags04;
    runtime.runnerExitFirstClearTerminalBranchTriggerEligibleStream1NextFlag80Byte29 =
        inputs.lateBranchFirstClearTerminalBranchTriggerEligibleStream1NextFlag80Byte29;
    runtime.runnerExitFirstClearTerminalBranchTriggerEligibleStream1NextFlag80Byte30 =
        inputs.lateBranchFirstClearTerminalBranchTriggerEligibleStream1NextFlag80Byte30;
    runtime.runnerExitClearTerminalBranchTriggerStream1Cursor =
        inputs.lateBranchClearTerminalBranchTriggerStream1Cursor;
    runtime.runnerExitClearTerminalBranchTriggerStream1Count =
        inputs.lateBranchClearTerminalBranchTriggerStream1Count;
    runtime.runnerExitClearTerminalBranchTriggerStream1DueKnown =
        inputs.lateBranchClearTerminalBranchTriggerStream1DueKnown;
    runtime.runnerExitClearTerminalBranchTriggerStream1DueFrame =
        inputs.lateBranchClearTerminalBranchTriggerStream1DueFrame;
    runtime.runnerExitClearTerminalBranchTriggerStream1DueDelta =
        inputs.lateBranchClearTerminalBranchTriggerStream1DueDelta;
    runtime.runnerExitClearTerminalBranchTriggerStream1BaseFrame =
        inputs.lateBranchClearTerminalBranchTriggerStream1BaseFrame;
    runtime.runnerExitClearTerminalBranchTriggerStream1AbsDueFrame =
        inputs.lateBranchClearTerminalBranchTriggerStream1AbsDueFrame;
    runtime.runnerExitClearTerminalBranchTriggerStream1AbsDueDelta =
        inputs.lateBranchClearTerminalBranchTriggerStream1AbsDueDelta;
    runtime.runnerExitClearTerminalBranchTriggerStream1Flags04 =
        inputs.lateBranchClearTerminalBranchTriggerStream1Flags04;
    runtime.runnerExitClearTerminalBranchTriggerStream1Byte29 =
        inputs.lateBranchClearTerminalBranchTriggerStream1Byte29;
    runtime.runnerExitClearTerminalBranchTriggerStream1Byte30 =
        inputs.lateBranchClearTerminalBranchTriggerStream1Byte30;
    runtime.runnerExitFirstClearTerminalBranchTriggerEventNotDueKnown =
        inputs.lateBranchFirstClearTerminalBranchTriggerEventNotDueKnown;
    runtime.runnerExitFirstClearTerminalBranchTriggerEventNotDueScriptFrame =
        inputs.lateBranchFirstClearTerminalBranchTriggerEventNotDueScriptFrame;
    runtime.runnerExitFirstClearTerminalBranchTriggerEventNotDueCursor =
        inputs.lateBranchFirstClearTerminalBranchTriggerEventNotDueCursor;
    runtime.runnerExitFirstClearTerminalBranchTriggerEventNotDueFrame =
        inputs.lateBranchFirstClearTerminalBranchTriggerEventNotDueFrame;
    runtime.runnerExitFirstClearTerminalBranchTriggerEventNotDueDelta =
        inputs.lateBranchFirstClearTerminalBranchTriggerEventNotDueDelta;
    runtime.runnerExitFirstClearTerminalBranchTriggerEventNotDueBaseFrame =
        inputs.lateBranchFirstClearTerminalBranchTriggerEventNotDueBaseFrame;
    runtime.runnerExitFirstClearTerminalBranchTriggerEventNotDueAbsFrame =
        inputs.lateBranchFirstClearTerminalBranchTriggerEventNotDueAbsFrame;
    runtime.runnerExitFirstClearTerminalBranchTriggerEventNotDueAbsDelta =
        inputs.lateBranchFirstClearTerminalBranchTriggerEventNotDueAbsDelta;
    runtime.runnerExitFirstClearTerminalBranchTriggerEventNotDueFlags04 =
        inputs.lateBranchFirstClearTerminalBranchTriggerEventNotDueFlags04;
    runtime.runnerExitFirstClearTerminalBranchTriggerMissingFlag80Known =
        inputs.lateBranchFirstClearTerminalBranchTriggerMissingFlag80Known;
    runtime.runnerExitFirstClearTerminalBranchTriggerMissingFlag80ScriptFrame =
        inputs.lateBranchFirstClearTerminalBranchTriggerMissingFlag80ScriptFrame;
    runtime.runnerExitFirstClearTerminalBranchTriggerMissingFlag80Cursor =
        inputs.lateBranchFirstClearTerminalBranchTriggerMissingFlag80Cursor;
    runtime.runnerExitFirstClearTerminalBranchTriggerMissingFlag80Frame =
        inputs.lateBranchFirstClearTerminalBranchTriggerMissingFlag80Frame;
    runtime.runnerExitFirstClearTerminalBranchTriggerMissingFlag80Delta =
        inputs.lateBranchFirstClearTerminalBranchTriggerMissingFlag80Delta;
    runtime.runnerExitFirstClearTerminalBranchTriggerMissingFlag80BaseFrame =
        inputs.lateBranchFirstClearTerminalBranchTriggerMissingFlag80BaseFrame;
    runtime.runnerExitFirstClearTerminalBranchTriggerMissingFlag80AbsFrame =
        inputs.lateBranchFirstClearTerminalBranchTriggerMissingFlag80AbsFrame;
    runtime.runnerExitFirstClearTerminalBranchTriggerMissingFlag80AbsDelta =
        inputs.lateBranchFirstClearTerminalBranchTriggerMissingFlag80AbsDelta;
    runtime.runnerExitFirstClearTerminalBranchTriggerMissingFlag80Flags04 =
        inputs.lateBranchFirstClearTerminalBranchTriggerMissingFlag80Flags04;
    runtime.runnerExitClearTerminalTailServiceLatch =
        inputs.lateBranchClearTerminalTailServiceLatch;
    runtime.runnerExitClearTerminalTailServiceScriptFrame =
        inputs.lateBranchClearTerminalTailServiceScriptFrame;
    runtime.runnerExitClearTerminalTailServiceRightRankRow =
        inputs.lateBranchClearTerminalTailServiceRightRankRow;
    runtime.runnerExitClearTerminalTailServiceCurrentMode =
        inputs.lateBranchClearTerminalTailServiceCurrentMode;
    runtime.runnerExitClearTerminalTailServiceStream =
        inputs.lateBranchClearTerminalTailServiceStream;
    runtime.runnerExitPendingRatingBranchSeq =
        inputs.lateBranchPendingRatingBranchSeq;
    runtime.runnerExitConsumedRatingBranchSeq =
        inputs.lateBranchConsumedRatingBranchSeq;
    runtime.runnerExitClearTerminalTailArmed =
        inputs.lateBranchClearTerminalTailArmed;
    runtime.runnerExitClearTerminalTailDispatchActive =
        inputs.lateBranchClearTerminalTailDispatchActive;
    runtime.runnerExitClearTerminalTailStream =
        inputs.lateBranchClearTerminalTailStream;
    runtime.runnerExitActiveDispatchStartScriptFrame =
        inputs.lateBranchActiveDispatchStartScriptFrame;
    runtime.runnerExitActiveDispatchTerminalEndLocalFrame =
        inputs.lateBranchActiveDispatchTerminalEndLocalFrame;
    runtime.runnerExitActiveDispatchTerminalPulseEmitted =
        inputs.lateBranchActiveDispatchTerminalPulseEmitted;
    runtime.runnerTailRecordsModeDAActive = inputs.recordsModeDAActive;
    runtime.runnerTailRecordsModeDAEqualsOne = inputs.recordsModeDAEqualsOne;
    runtime.runnerTailStageStatusKnown = inputs.stageStatus166ACKnown;
    runtime.runnerTailStageStatus166AC = inputs.stageStatus166AC;
    runtime.runnerTailCleanupRenderPassBudget =
        inputs.runnerTailDirectKnown &&
                inputs.runnerTailCleanupRequiredKnown &&
                inputs.runnerTailCleanupRequired
            ? inputs.runnerTailCleanupDrainFrameCount
            : 0u;
    if (inputs.runnerTailDirectKnown && inputs.runnerTailCleanupRequiredKnown &&
        inputs.runnerTailCleanupRequired &&
        inputs.runnerTailFrameExit1A3B8Known) {
        runtime.runnerExitLatch76Known = true;
        runtime.runnerExitLatch76Active =
            runtime.runnerExitLatch76Active ||
            inputs.runnerTailFrameExit1A3B8Taken;
    }
    const PrStageRunnerDirectFinalReturn7A60Result finalReturn =
        PrStageRunnerDirectResolveFinalReturn7A60(
            PrStageRunnerDirectFinalReturn7A60Input{
                runtime.runnerExitLatch76Known,
                runtime.runnerExitLatch76Active,
                runtime.runnerExitGate78Known,
                runtime.runnerExitGate78Active,
                runtime.runnerTailRecordsModeDAEqualsOne,
                runtime.runnerTailStageStatusKnown,
                runtime.runnerTailStageStatus166AC,
            });
    runtime.runnerTailFinalReturnKnown = finalReturn.resultKnown;
    runtime.runnerTailFinalReturn = finalReturn.result;
    runtime.runnerTailFinalReturnWord59Known = finalReturn.word59Known;
    runtime.runnerTailFinalReturnWord59IsOne = finalReturn.word59IsOne;
    runtime.runnerTailFinalReturnWord60Known = finalReturn.word60Known;
    runtime.runnerTailFinalReturnWord60IsOne = finalReturn.word60IsOne;
    runtime.runnerTailFinalReturnCalls166AC = finalReturn.calls166AC;
    runtime.runnerTailFinalReturnRecordsModeReturnsOne =
        finalReturn.recordsModeReturnsOne;
    runtime.runnerTailFinalReturnStageStatusReturnsOne =
        finalReturn.stageStatusReturnsOne;
}

PrStage1ScorerDirectReplayBackupState BuildStage1DirectPortReplayBackupAdapter(
    const Stage1AcceptedProducerReplayBackupRuntime& backup) {
    PrStage1ScorerDirectReplayBackupState directBackup{};
    directBackup.valid = backup.valid;
    directBackup.dword92F48PublishedCount = backup.publishedCount901BC;
    for (size_t i = 0; i < kStage1AcceptedProducerReplayBufferCapacity; ++i) {
        directBackup.dwordEEF8Tick96[i] = backup.tick96EEF8[i];
        directBackup.dwordEEFCClassMask[i] = backup.classMaskEEFC[i];
    }
    return directBackup;
}

PrStage1ScorerDirectReplayBufferState BuildStage1DirectPortReplayBufferAdapter(
    const Stage1NumericRuntimeState::AcceptedProducerReplayBufferRuntime& replay) {
    PrStage1ScorerDirectReplayBufferState directReplay{};
    directReplay.replayMirrorKnown8008EEF8 = replay.replayMirrorKnown8008EEF8;
    directReplay.replayMirrorProducerKnown8008EEF8 =
        replay.replayMirrorProducerKnown8008EEF8;
    directReplay.replayMirrorProducerFunction =
        replay.replayMirrorProducerFunction;
    directReplay.replayMirrorByteCountKnown8008EEF8 =
        replay.replayMirrorByteCountKnown8008EEF8;
    directReplay.replayMirrorKnownByteCount8008EEF8 =
        replay.replayMirrorKnownByteCount8008EEF8;
    directReplay.dword901C0WriteCount = replay.writeCount901C0;
    directReplay.dword901BCPublishedCount = replay.publishedCount901BC;
    for (size_t i = 0; i < kStage1AcceptedProducerReplayBufferCapacity; ++i) {
        directReplay.dwordEEF8Tick96[i] = replay.tick96EEF8[i];
        directReplay.dwordEEFCClassMask[i] = replay.classMaskEEFC[i];
    }
    return directReplay;
}

void StoreStage1DirectPortReplayBufferRestoreResult(
    const PrStage1ScorerDirectReplayBufferState& directReplay,
    Stage1NumericRuntimeState::AcceptedProducerReplayBufferRuntime& replay) {
    replay.replayMirrorKnown8008EEF8 = directReplay.replayMirrorKnown8008EEF8;
    replay.replayMirrorProducerKnown8008EEF8 =
        directReplay.replayMirrorProducerKnown8008EEF8;
    replay.replayMirrorProducerFunction =
        directReplay.replayMirrorProducerFunction;
    replay.replayMirrorByteCountKnown8008EEF8 =
        directReplay.replayMirrorByteCountKnown8008EEF8;
    replay.replayMirrorKnownByteCount8008EEF8 =
        directReplay.replayMirrorKnownByteCount8008EEF8;
    replay.writeCount901C0 = directReplay.dword901C0WriteCount;
    replay.publishedCount901BC = directReplay.dword901BCPublishedCount;
    for (size_t i = 0; i < kStage1AcceptedProducerReplayBufferCapacity; ++i) {
        replay.tick96EEF8[i] = directReplay.dwordEEF8Tick96[i];
        replay.classMaskEEFC[i] = directReplay.dwordEEFCClassMask[i];
    }
    PrStage1SaveUiDirect::SetReplayMirrorSource(directReplay);
}

PrStage1ScorerDirectResolvedReplayBackup1681C ResolveStage1AcceptedReplayBackupSource1681C(
    const Stage1AcceptedProducerReplayBackupRuntime& backup) {
    return PrStage1ScorerDirectResolveReplayRestoreSource1681C(
        false,
        PrStage1ScorerDirectReplayBackupState{},
        backup.valid,
        BuildStage1DirectPortReplayBackupAdapter(backup));
}

PrStage1ScorerDirectAcceptedProducerOwnerState
BuildStage1DirectPortAcceptedOwnerStateAdapter(
    const Stage1NumericRuntimeState& state) {
    PrStage1ScorerDirectAcceptedProducerOwnerState out{};
    out.dword91800LastClassToken =
        state.acceptedProducerSourceGroup.lastClassToken91800;
    for (size_t i = 0u; i < kPrStage1ScorerDirectSourceGroupCursorEntryCount;
         ++i) {
        const auto& src = state.acceptedProducerSourceGroup.cursorEntries[i];
        auto& dst = out.cursorEntries[i];
        dst.occupied = src.occupied;
        dst.dword00HeaderAddr = src.headerAddr;
        dst.word08Count = src.count;
        dst.word0ACursor = src.cursor;
    }
    return out;
}

void StoreStage1DirectPortAcceptedOwnerStateAdapter(
    Stage1NumericRuntimeState& state,
    const PrStage1ScorerDirectAcceptedProducerOwnerState& ownerState) {
    state.acceptedProducerSourceGroup.lastClassToken91800 =
        ownerState.dword91800LastClassToken;
    for (size_t i = 0u; i < kStage1AcceptedProducerSourceGroupCursorEntryCount;
         ++i) {
        const auto& src = ownerState.cursorEntries[i];
        auto& dst = state.acceptedProducerSourceGroup.cursorEntries[i];
        dst.occupied = src.occupied;
        dst.headerAddr = src.dword00HeaderAddr;
        dst.count = src.word08Count;
        dst.cursor = src.word0ACursor;
    }
}

PrStage1ScorerDirectReplayBufferState BuildStage1DirectPortReplayBufferAdapter(
    const Stage1NumericRuntimeState& state) {
    PrStage1ScorerDirectReplayBufferState out{};
    out.replayMirrorKnown8008EEF8 =
        state.acceptedProducerReplayBuffer.replayMirrorKnown8008EEF8;
    out.replayMirrorProducerKnown8008EEF8 =
        state.acceptedProducerReplayBuffer.replayMirrorProducerKnown8008EEF8;
    out.replayMirrorProducerFunction =
        state.acceptedProducerReplayBuffer.replayMirrorProducerFunction;
    out.replayMirrorByteCountKnown8008EEF8 =
        state.acceptedProducerReplayBuffer.replayMirrorByteCountKnown8008EEF8;
    out.replayMirrorKnownByteCount8008EEF8 =
        state.acceptedProducerReplayBuffer.replayMirrorKnownByteCount8008EEF8;
    out.dword901C0WriteCount = state.acceptedProducerReplayBuffer.writeCount901C0;
    out.dword901BCPublishedCount =
        state.acceptedProducerReplayBuffer.publishedCount901BC;
    for (size_t i = 0u; i < kPrStage1ScorerDirectReplayBufferCapacity; ++i) {
        out.dwordEEF8Tick96[i] = state.acceptedProducerReplayBuffer.tick96EEF8[i];
        out.dwordEEFCClassMask[i] =
            state.acceptedProducerReplayBuffer.classMaskEEFC[i];
    }
    return out;
}

void StoreStage1DirectPortReplayBufferAdapter(
    Stage1NumericRuntimeState& state,
    const PrStage1ScorerDirectReplayBufferState& replay) {
    state.acceptedProducerReplayBuffer.replayMirrorKnown8008EEF8 =
        replay.replayMirrorKnown8008EEF8;
    state.acceptedProducerReplayBuffer.replayMirrorProducerKnown8008EEF8 =
        replay.replayMirrorProducerKnown8008EEF8;
    state.acceptedProducerReplayBuffer.replayMirrorProducerFunction =
        replay.replayMirrorProducerFunction;
    state.acceptedProducerReplayBuffer.replayMirrorByteCountKnown8008EEF8 =
        replay.replayMirrorByteCountKnown8008EEF8;
    state.acceptedProducerReplayBuffer.replayMirrorKnownByteCount8008EEF8 =
        replay.replayMirrorKnownByteCount8008EEF8;
    state.acceptedProducerReplayBuffer.writeCount901C0 =
        replay.dword901C0WriteCount;
    state.acceptedProducerReplayBuffer.publishedCount901BC =
        replay.dword901BCPublishedCount;
    for (size_t i = 0u; i < kPrStage1ScorerDirectReplayBufferCapacity; ++i) {
        state.acceptedProducerReplayBuffer.tick96EEF8[i] =
            replay.dwordEEF8Tick96[i];
        state.acceptedProducerReplayBuffer.classMaskEEFC[i] =
            replay.dwordEEFCClassMask[i];
    }
    PrStage1SaveUiDirect::SetReplayMirrorSource(replay);
}

void MaterializeStage1SourceCellVoiceLaneMirrorFromDirectPort(
    Stage1NumericRuntimeState& state,
    const PrStage1ScorerDirectAcceptedProducerSideEffectResult& sideEffect,
    uint16_t rawAcceptedMask) {
    if (!sideEffect.playSourceCellCue) {
        return;
    }

    Stage1NumericRuntimeState::SourceCellVoiceLaneRuntime& voiceLane =
        state.sourceCellVoice;
    voiceLane.active = true;
    voiceLane.program = sideEffect.cueProgram;
    voiceLane.note = sideEffect.cueNote;
    voiceLane.key = sideEffect.cueKey;
    voiceLane.volume = sideEffect.cueVolume;
    voiceLane.replaceRestartPulse = true;
    const double inputAgeSeconds =
        PrPad::GetPressedAgeSeconds(0, rawAcceptedMask);
    PrSfx::PlayStage1SourceCellVoiceCue(
        sideEffect.cueProgram,
        sideEffect.cueNote,
        sideEffect.cueKey,
        sideEffect.cueVolume,
        inputAgeSeconds);
}

void ClearStage1PageRecordMirrorPageHost(
    Stage1NumericRuntimeState::PageRecordMirrorPage& page) {
    page = Stage1NumericRuntimeState::PageRecordMirrorPage{};
}

int ResolveStage1PageRecordMirrorPageOrdinalForTick96Host(int tick96) {
    return (tick96 < 0) ? 0 : (tick96 / 384);
}

void PrepareStage1PageRecordMirrorForPageOrdinalHost(
    Stage1NumericRuntimeState::PageRecordMirrorRuntime& runtime,
    int pageOrdinal) {
    if (!runtime.initialized) {
        runtime = Stage1NumericRuntimeState::PageRecordMirrorRuntime{};
        runtime.initialized = true;
        runtime.currentPageOrdinal = pageOrdinal;
        ClearStage1PageRecordMirrorPageHost(
            runtime.pages[(size_t)(pageOrdinal & 3)]);
        return;
    }

    if (pageOrdinal < runtime.currentPageOrdinal) {
        runtime = Stage1NumericRuntimeState::PageRecordMirrorRuntime{};
        runtime.initialized = true;
        runtime.currentPageOrdinal = pageOrdinal;
        ClearStage1PageRecordMirrorPageHost(
            runtime.pages[(size_t)(pageOrdinal & 3)]);
        return;
    }

    if (pageOrdinal == runtime.currentPageOrdinal) {
        return;
    }

    if (pageOrdinal == runtime.currentPageOrdinal + 1) {
        runtime.currentPageOrdinal = pageOrdinal;
        ClearStage1PageRecordMirrorPageHost(
            runtime.pages[(size_t)(pageOrdinal & 3)]);
        return;
    }

    for (int nextOrdinal = runtime.currentPageOrdinal + 1;
         nextOrdinal <= pageOrdinal;
         nextOrdinal++) {
        ClearStage1PageRecordMirrorPageHost(
            runtime.pages[(size_t)(nextOrdinal & 3)]);
    }
    runtime.currentPageOrdinal = pageOrdinal;
}

bool TryBuildStage1PageMaintenanceProjectionFromFinalResult14BDC_14614Host(
    const Stage1NumericRuntimeState& state,
    const PrStage1ScorerDirectFinalPageMaintenance14BDC_14614Result&
        finalResult,
    Stage1PageMaintenanceProjection14BDC_14614& out) {
    out = Stage1PageMaintenanceProjection14BDC_14614{};
    if (!finalResult.valid || !finalResult.pageKnown) {
        return false;
    }

    if (finalResult.clearApplied && finalResult.clearPageKnown &&
        finalResult.clearPageOrdinal1Based <= 0xFFu) {
        const int pageOrdinal =
            static_cast<int>(finalResult.clearPageOrdinal1Based);
        if (pageOrdinal >= 0 && state.pageRecordMirror.initialized) {
            out.clearValid = true;
            out.clear.valid = true;
            out.clear.targetOrdinal1Based =
                finalResult.clearPageOrdinal1Based;
            out.clear.pageIndex = static_cast<size_t>(pageOrdinal & 3);
        }
    }

    if (finalResult.slotWriteApplied && state.pageRecordMirror.initialized) {
        out.slotValid = true;
        out.slot.valid = true;
        out.slot.targetOrdinal1Based =
            static_cast<uint8_t>(finalResult.pageOrdinal1Based);
        out.slot.pageIndex = static_cast<size_t>(
            static_cast<int>(finalResult.pageOrdinal1Based) & 3);
        out.slot.rawOffset = ResolveStage1PageRecordRawOffsetForSlotIndex(
            (std::min<size_t>)(static_cast<size_t>(finalResult.recordSlot),
                               kPrStage1ScorerDirectSlotsPerPage - 1u));
        out.slot.rawRecord = finalResult.rawRecord;
    }

    out.valid = out.clearValid || out.slotValid;
    return out.valid;
}

bool ApplyStage1PageMaintenanceProjection14BDC_14614Host(
    Stage1NumericRuntimeState& state,
    const Stage1PageMaintenanceProjection14BDC_14614& projection) {
    if (!projection.valid) {
        return false;
    }

    bool applied = false;
    if (projection.clearValid) {
        state.pageOwner.currentWritePageOrdinalKnown = true;
        state.pageOwner.currentWritePageOrdinal1Based =
            projection.clear.targetOrdinal1Based;
        ClearStage1PageRecordMirrorPageHost(
            state.pageRecordMirror.pages[projection.clear.pageIndex]);
        applied = true;
    }
    if (projection.slotValid) {
        Stage1NumericRuntimeState::PageRecordMirrorPage& page =
            state.pageRecordMirror.pages[projection.slot.pageIndex];
        WriteStage1PageRecordRawU32(
            page,
            projection.slot.rawOffset,
            projection.slot.rawRecord.dword00AcceptedMask);
        WriteStage1PageRecordRawU16(
            page,
            projection.slot.rawOffset + 4u,
            projection.slot.rawRecord.word04Companion);
        WriteStage1PageRecordRawU16(
            page,
            projection.slot.rawOffset + 6u,
            projection.slot.rawRecord.word06Occupied);
        WriteStage1PageRecordRawU32(
            page,
            projection.slot.rawOffset + 8u,
            projection.slot.rawRecord.dword08Payload);
        applied = true;
    }
    return applied;
}

bool ResolveStage1Ctx52ReplayMode7A60Impl(
    const Stage1NumericRuntimeState& state) {
    return state.ctx52ReplayMode7A60;
}

bool TryBuildStage1AcceptedFrontDoorPacket7A60(
    const PrGameContext& ctx,
    const Stage1NumericRuntimeState& state,
    int tick96,
    bool sameFrameWriteCtx10CurrentTick,
    uint16_t writerControlSample18,
    uint16_t rawPressedMask,
    uint16_t rawAcceptedMask,
    Stage1AcceptedFrontDoorPacket7A60& out) {
    (void)ctx;
    (void)rawPressedMask;
    (void)rawAcceptedMask;
    out = Stage1AcceptedFrontDoorPacket7A60{};
    const uint16_t acceptedMask9FF =
        static_cast<uint16_t>(writerControlSample18 & 0x09FFu);
    if (acceptedMask9FF == 0u) {
        return false;
    }
    const bool busyGate24BF4Active =
        ResolveStage1DirectBusyGate24BF4(state, tick96);
    if (busyGate24BF4Active) {
        return false;
    }

    const Stage1NumericRuntimeState::AcceptedProducerCarrierRuntime& carrier =
        state.acceptedProducerCarrier;
    if (carrier.available && carrier.acceptedGateKnown &&
        !carrier.acceptedGateActive) {
        return false;
    }

    const Stage1NumericRuntimeState::DescriptorCadenceRuntime&
        descriptorRuntime = state.descriptorCadence;
    if (!descriptorRuntime.lookaheadDescriptor44Available ||
        !descriptorRuntime.lookaheadDescriptor44Row.available) {
        return false;
    }

    out.directInput.busyGate24BF4Active = busyGate24BF4Active;
    out.directInput.eventStreamFlagActive =
        !carrier.available || !carrier.eventStreamFlagKnown ||
        carrier.eventStreamFlagActive;
    out.directInput.writerControlSample18 = writerControlSample18;
    out.directInput.classToken20Known = carrier.available;
    out.directInput.classToken20 = carrier.classToken20;
    const bool replayMode52 = ResolveStage1Ctx52ReplayMode7A60(state);
    if (!replayMode52 && sameFrameWriteCtx10CurrentTick && tick96 >= 0) {
        // PSX local dispatch only authorizes ctx+0x10 from the current
        // ctx+0x0C on the same accepted-input owner beat right before 14614.
        out.directInput.acceptedTick96Known = true;
        out.directInput.acceptedTick96 = tick96;
    } else {
        out.directInput.acceptedTick96Known = carrier.acceptedTick96Known;
        out.directInput.acceptedTick96 =
            carrier.acceptedTick96Known ? carrier.acceptedTick96 : 0;
    }
    out.directInput.halfWindow34 =
        carrier.available ? carrier.halfWindow34
                          : ResolveStage1AcceptedProducerHalfWindow34(ctx, state);
    out.directInput.descriptorSubstate50 =
        ResolveStage1AcceptedProducerSubstate50(state);
    // `sub_80014614` consumes the live `ctx+0x38` write page after the
    // same-tick owner windows have already updated page ownership.
    out.directInput.writePageOrdinal38 =
        ResolveStage1AcceptedProducerWritePageOrdinal38(state, tick96);
    out.directInput.replayLogBlocked82 = replayMode52;
    out.directRow = BuildStage1DirectPortDescriptorRowAdapter(
        descriptorRuntime.lookaheadDescriptor44Row);
    out.directInput.lookaheadDescriptorRow = &out.directRow;
    return true;
}

void RememberStage1AcceptedProducerLastRecordedPageWrite14614(
    Stage1NumericRuntimeState& state,
    const PrStage1ScorerDirectPageStorageApply14614Result& pageStorageApply) {
    if (!pageStorageApply.recordedWriteApplied ||
        pageStorageApply.writePageOrdinal < 0) {
        return;
    }

    Stage1NumericRuntimeState::AcceptedProducerLastRecordedPageWriteRuntime&
        lastWrite = state.acceptedProducerLastRecordedPageWrite;
    lastWrite.available = true;
    lastWrite.writePageOrdinal38 = pageStorageApply.writePageOrdinal;
    lastWrite.recordSlot24 = pageStorageApply.recordSlot;
    lastWrite.acceptedMask = pageStorageApply.acceptedMask;
    lastWrite.pageCompanion = pageStorageApply.pageCompanion;
}

}  // namespace

bool ResolveStage1Ctx52ReplayMode7A60(const Stage1NumericRuntimeState& state) {
    return ResolveStage1Ctx52ReplayMode7A60Impl(state);
}

PrStage1RuntimeSlotsDirectFollowUpFacts801CBFDC
BuildStage1RuntimeSlotsFollowUpFacts801CBFDC(
    const Stage1NumericRuntimeState& state) {
    const Stage1NumericRuntimeState::RightRankDirectFollowUpRuntime& followUp =
        state.rightRankDirectFollowUp;
    PrStage1RuntimeSlotsDirectFollowUpFacts801CBFDC facts{};
    facts.flag0200FrameUpdateBranchTaken =
        followUp.flag0200FrameUpdateBranchTaken;
    facts.flag0200FrameUpdateResourcePairKnown =
        followUp.flag0200FrameUpdateResourcePairKnown;
    facts.flag0200FrameUpdateCtxFlag20000Set =
        followUp.flag0200FrameUpdateCtxFlag20000Set;
    facts.flag0200FrameUpdateCtxE0ResourceIndex =
        followUp.flag0200FrameUpdateCtxE0ResourceIndex;
    facts.flag0200FrameUpdateCtxECResourceIndex =
        followUp.flag0200FrameUpdateCtxECResourceIndex;
    facts.descBankSelectorPulse = followUp.descBankSelectorPulse;
    facts.descBankSelector = followUp.descBankSelector;
    facts.descBankSelectorQueryFrame = followUp.descBankSelectorQueryFrame;
    facts.descBankCtxFlagsPulse801CBFDC =
        followUp.descBankCtxFlagsPulse801CBFDC;
    facts.rightRankActiveRow = state.rightRankState.rightRankActiveRow;
    facts.cameraPulse9443C = followUp.cameraPulse9443C;
    facts.cameraBezHandle9443C = followUp.cameraBezHandle9443C;
    facts.cameraQueryFrame9443C = followUp.cameraQueryFrame9443C;
    facts.acceptedTailTimer9443CPulse =
        followUp.acceptedTailTimer9443CPulse;
    facts.acceptedTailTimer9443CFrames =
        followUp.acceptedTailTimer9443CFrames;
    facts.acceptedTailTimer9443CQueryFrame =
        followUp.acceptedTailTimer9443CQueryFrame;
    facts.hudOverlayConfigure801CA0E0Pulse =
        followUp.hudOverlayConfigure801CA0E0Pulse;
    facts.hudOverlayConfigure801CA0E0Arg =
        followUp.hudOverlayConfigure801CA0E0Arg;
    facts.hudOverlayConfigure801CA0E0QueryFrame =
        followUp.hudOverlayConfigure801CA0E0QueryFrame;
    facts.hudOverlayConfigure801CA0E0ScriptFrame =
        followUp.hudOverlayConfigure801CA0E0ScriptFrame;
    facts.compactPainterGate7A = state.rightRankBucketContext.ctx7A;
    facts.compactRailRowCount8AClear80024FD0 = followUp.flag2000Pulse;
    facts.ctxFlagTickAdvanceKnown801C9094 =
        state.bucketCadence.ctxFlagTickAdvanceKnown801C9094;
    facts.ctxFlagTickAdvance801C9094 =
        state.bucketCadence.ctxFlagTickAdvance801C9094;
    return facts;
}

PrStageEventDirectStage1FrameInput
BuildStage1EventStreamFrameInput801C9094(
    const Stage1NumericRuntimeState& state,
    int tick96,
    uint32_t queryFrame,
    bool allowSameQueryRefresh,
    bool clearTerminalTailPulse) {
    return PrStageEventDirectStage1BuildFrameInputFromPrimitiveFacts801C9094(
        queryFrame,
        tick96,
        allowSameQueryRefresh,
        state.rightRankState.rightRankActiveRow,
        state.rightRankHelperShadow.bucket0Ctx118WritePulse,
        state.rightRankDirectFollowUp.flag2000Pulse,
        state.rightRankDirectFollowUp.flag4000Pulse,
        clearTerminalTailPulse,
        state.rightRankForcedGoodEventStreamDone28,
        state.rightRankDirectFollowUp.ctx72,
        state.rightRankDirectFollowUp.ed24,
        state.rightRankDirectFollowUp.ed00,
        state.rightRankDirectFollowUp.ed14,
        state.rightRankDirectFollowUp.ed0c,
        state.steadySfxDelayedCompletionPending,
        static_cast<uint8_t>(state.rightRankFollowUpPhase));
}

Stage1NumericRuntimeState s_stage1NumericRuntime;
Stage1AcceptedProducerReplayBackupRuntime
    s_stage1AcceptedProducerReplayBackupRuntime;

bool TryResolveStage1RuntimeDirectTick96(
    const Stage1NumericRuntimeState& state,
    int& outTick96) {
    outTick96 = 0;
    if (!state.active || !state.runnerTimecode801C7560.known) {
        return false;
    }
    outTick96 = state.runnerTimecode801C7560.state.tick801C364C;
    return true;
}

Stage1RunnerTimingRoots30 ResolveStage1RunnerTimingRoots30(
    const PrStageRunner& runner) {
    Stage1RunnerTimingRoots30 out{};
    out.runnerFrame = runner.GetFrame();
    out.queryFrame = (uint32_t)(std::max)(runner.GetFrame(), 0);
    out.tick96 = runner.GetTick96();
    int directTick96 = 0;
    if (TryResolveStage1RuntimeDirectTick96(
            s_stage1NumericRuntime,
            directTick96)) {
        out.tick96 = directTick96;
    }
    return out;
}

bool IsStage1NumericRuntimeGateActive(PrGameContext& ctx,
                                      const PrStageRunner& runner) {
    return IsStage1FormalLifecycleRuntimeGateActiveImpl(ctx, runner) &&
           runner.GetState() == StageRunnerState::Playing;
}

// Host-only forward declarations. Keep scene-visible declarations in deps.h.
void ApplyStage1DirectPortFollowUpPhaseAction(
    Stage1NumericRuntimeState& state,
    PrStage1ScorerDirectFollowUpPhaseAction action);
void ApplyStage1DirectPortBucket30Phase1StateAdapter(
    Stage1NumericRuntimeState& state,
    const PrStage1ScorerDirectGlobals& globals,
    uint8_t activeRow);
void ApplyStage1Bucket30CoolPhase1ObserverProjection(
    Stage1NumericRuntimeState& state,
    const PrStage1ScorerDirectGlobals& globals);
void ApplyStage1Bucket0WindowSceneProjection(
    Stage1NumericRuntimeState& state,
    const PrStage1ScorerDirectBucket0WindowSliceResult& directWindow,
    uint16_t descriptorFlagWord);
void ApplyStage1Bucket30ResolvedRowPublishRuntime(
    Stage1NumericRuntimeState& state,
    const PrStage1ScorerDirectBucket30ResolvedPublish& publish);
void ClearStage1Bucket30ResolutionMirror(Stage1NumericRuntimeState& state);
void ClearStage1SteadySfxDispatchMirror(Stage1NumericRuntimeState& state);
void ArmStage1Bucket31DelayedSteadySfxCompletion(Stage1NumericRuntimeState& state);
void ApplyStage1RightRank24F8CObserverProjection(
    Stage1NumericRuntimeState& state);
void ApplyStage1Bucket30DirectClearSceneProjection(
    Stage1NumericRuntimeState& state,
    const PrStage1ScorerDirectBucket30ClearSliceResult& directClearSlice);
void ApplyStage1Bucket30RowWriteSceneProjection(
    Stage1NumericRuntimeState& state,
    const PrStage1ScorerDirectBucket30ResolvedPublish& publish,
    const PrStage1ScorerDirectBucket30RowWriteSliceResult& directRowWriteSlice);
void ApplyStage1Bucket30SteadySfxSceneProjection(
    Stage1NumericRuntimeState& state,
    const PrStage1ScorerDirectSteadySfxAction& directAction);
void ApplyStage1Bucket30Phase1SceneProjection(
    Stage1NumericRuntimeState& state,
    uint8_t activeRow,
    const PrStage1ScorerDirectGlobals& globals,
    const PrStage1ScorerDirectPhase1StepResult& directPhase1Step,
    bool& outDirectConsumerSlotKnown,
    uint8_t& outDirectConsumerSlot);
void ApplyStage1Bucket30DelayedTailSceneProjection(
    Stage1NumericRuntimeState& state,
    const PrStage1ScorerDirectBucket30DelayedTailSliceResult& directDelayedTailSlice);
void ApplyStage1Bucket31DirectSceneProjection(
    Stage1NumericRuntimeState& state,
    const PrStage1ScorerDirectBucket31DispatcherResult& directBucket31Result);
bool ApplyStage1PageMaintenanceProjectionFromLiteralPageClear14BDC_14614(
    Stage1NumericRuntimeState& state,
    const PrStage1ScorerDirectPageClear14BDCResult* sameTickPageClear,
    const PrStage1ScorerDirectAcceptedProducerRunResult* directAcceptedResult);
void ApplyStage1AcceptedSpecialSetupCore24E54_1681C(
    const PrGameContext& ctx,
    Stage1NumericRuntimeState& state);
void RefreshStage1DescriptorDrivenRuntimeStateForTick(
    const PrGameContext& ctx,
    Stage1NumericRuntimeState& state,
    int tick96);
void ApplyStage1FrameUpdateFlag0200Branch9094(
    Stage1NumericRuntimeState& state);
void ApplyStage1PostAcceptedLoop7A60(
    Stage1NumericRuntimeState& state,
    const PrStageRunnerDirectPostFrame7A60Result& postFrame);
void CaptureStage1DescBankSelector7A60(
    Stage1NumericRuntimeState& state,
    uint32_t queryFrame,
    int32_t tick96,
    uint16_t writerControlSample18,
    bool call94434,
    bool call9443C);
PrStageRunnerDirectPostFrame7A60Result ResolveStage1RunnerPostFrame7A60(
    Stage1NumericRuntimeState& state,
    const PrStageRunner& runner,
    bool ctxAcceptedInputFlagSet,
    int32_t tick96,
    uint16_t writerControlSample18,
    bool scorerAccept14614Known,
    bool scorerAccept14614ReturnsNonZero);
void ResolveStage1RunnerMainLoopTail7A60(
    Stage1NumericRuntimeState& state,
    int32_t sceneEntryField352FallbackTickAdvance,
    bool continuationGate1A7F8Known,
    bool continuationGate1A7F8ReturnsOne,
    bool frameExit1A3B8Known,
    bool frameExit1A3B8ReturnsOne,
    const PrStageRunnerDirectPostFrame7A60Result& postFrame);
void SeedStage1AuthoritativeAdditiveLaneRuntime(
    Stage1NumericRuntimeState& state);
void CaptureStage1AuthoritativeAdditiveLaneBucket30Commit(
    Stage1NumericRuntimeState& state);
void ClearStage1AdditiveLaneBookkeepingAfterDirectClear(
    Stage1NumericRuntimeState& state);

bool IsStage1LateBranchQualifiedBridgeProbe(
    const Stage1NumericRuntimeState::SharedAcceptedProducerBoundaryProbeRuntime& probe) {
    return probe.rawAcceptedMask != 0u &&
           probe.selectorAvailable &&
           probe.timingTemplateState == 2u &&
           probe.sourceCellGateActive &&
           probe.materialized;
}

void RefreshStage1LateBranchBridgeProbe(
    Stage1NumericRuntimeState& state,
    uint16_t rawPressedMask,
    uint16_t rawAcceptedMask,
    int runnerFrame,
    bool allowDynamicCarryProbe) {
    state.lateBranchBridgeProbe =
        Stage1NumericRuntimeState::SharedAcceptedProducerBoundaryProbeRuntime{};

    if (IsStage1LateBranchQualifiedBridgeProbe(
            state.acceptedProducerBoundaryProbe)) {
        state.lateBranchBridgeProbe = state.acceptedProducerBoundaryProbe;
    }

    if (!allowDynamicCarryProbe) {
        state.lateBranchDynamicCarryProbe =
            Stage1NumericRuntimeState::SharedAcceptedProducerBoundaryProbeRuntime{};
        state.lateBranchDynamicCarryProbeFrame = -1;
        return;
    }

    const int carryProbeAge =
        (state.lateBranchDynamicCarryProbeFrame >= 0)
            ? (runnerFrame - state.lateBranchDynamicCarryProbeFrame)
            : (kStage1LateBranchDynamicCarryProbePersistFrames + 1);
    if (IsStage1LateBranchQualifiedBridgeProbe(
            state.acceptedProducerBoundaryProbe)) {
        state.lateBranchDynamicCarryProbe = state.acceptedProducerBoundaryProbe;
        state.lateBranchDynamicCarryProbeFrame = runnerFrame;
    } else if (rawAcceptedMask == 0u &&
               rawPressedMask == 0u &&
               carryProbeAge >= 0 &&
               carryProbeAge <=
                   kStage1LateBranchDynamicCarryProbePersistFrames &&
               IsStage1LateBranchQualifiedBridgeProbe(
                   state.lateBranchDynamicCarryProbe)) {
        // Preserve the late-branch bridge as a separate observer/consumer
        // packet. Do not overwrite the current-frame accepted writer sample.
        state.lateBranchBridgeProbe = state.lateBranchDynamicCarryProbe;
        state.lateBranchBridgeProbe.carryReplayed = true;
    }
}

void ResetStage1FormalLifecycleRuntime() {
    s_stage1FormalLifecycleRuntime = Stage1FormalLifecycleRuntime{};
    s_stage1FormalLifecycleRuntime.lastKnownRightRankRow =
        kStage1InitialRightRankActiveRow;
    s_stage1FormalLifecycleRuntime.lastRunnerFrame = -1;
}

void ResetStage1TerminalFormalLifecycleSnapshot() {
    s_stage1TerminalFormalLifecycleSnapshot = PrStage1FormalLifecycleSnapshot{};
}

bool CopyStage1FormalLifecycleSnapshot(PrStage1FormalLifecycleSnapshot& out) {
    PopulateStage1FormalLifecycleSnapshotFromRuntime(
        s_stage1FormalLifecycleRuntime,
        out);
    return out.valid;
}

bool CopyStage1TerminalFormalLifecycleSnapshot(PrStage1FormalLifecycleSnapshot& out) {
    out = s_stage1TerminalFormalLifecycleSnapshot;
    return out.valid;
}

void ResetStage1RunnerTailGateRuntime() {
    s_stage1RunnerTailGateRuntime = Stage1RunnerTailGateRuntime{};
}

void ResetStage1ScorerHostNumericRuntimeState() {
    PrSfx::ResetStage1SourceCellVoiceCueLane();
    s_stage1NumericRuntime = Stage1NumericRuntimeState{};
    ResetStage1RunnerTailGateRuntime();
}

void UpdateStage1RunnerTailGateRuntime(
    const Stage1RunnerTailGateFrameInputs& inputs) {
    if (inputs.bucket0Ctx118WritePulse) {
        s_stage1RunnerTailGateRuntime.ctx76Bucket0Latched = true;
        if (!s_stage1RunnerTailGateRuntime.ctx76Bucket0PulseDetailKnown) {
            s_stage1RunnerTailGateRuntime.ctx76Bucket0PulseDetailKnown = true;
            s_stage1RunnerTailGateRuntime.ctx76Bucket0PulseQueryFrame =
                inputs.bucket0Ctx118WritePulseQueryFrame;
            s_stage1RunnerTailGateRuntime.ctx76Bucket0PulseRightRankRow =
                inputs.bucket0Ctx118WritePulseRightRankRow;
            s_stage1RunnerTailGateRuntime.ctx76Bucket0PulseCallWindowOpen =
                inputs.bucket0Ctx118WritePulseCallWindowOpen;
            s_stage1RunnerTailGateRuntime.ctx76Bucket0PulseDescriptorFlags =
                inputs.bucket0Ctx118WritePulseDescriptorFlags;
        }
    }

    if (!inputs.frameUpdate801C9094CtxKnown) {
        return;
    }
    s_stage1RunnerTailGateRuntime.ctx78FrameUpdate9094Known = true;
    s_stage1RunnerTailGateRuntime.ctx78FrameUpdate9094QueryFrame =
        inputs.frameUpdate801C9094QueryFrame;
    s_stage1RunnerTailGateRuntime.ctx78FrameUpdate9094Ctx76 =
        inputs.frameUpdate801C9094Ctx76;
    s_stage1RunnerTailGateRuntime.ctx78FrameUpdate9094Ctx78 =
        inputs.frameUpdate801C9094Ctx78;
    s_stage1RunnerTailGateRuntime.ctx78FrameUpdate9094TailFamilyActive =
        inputs.lateBranchTailDispatchFamilyActive;
    s_stage1RunnerTailGateRuntime.ctx78FrameUpdate9094TailStream =
        inputs.lateBranchTailDispatchFamilyStream;
    s_stage1RunnerTailGateRuntime.ctx78FrameUpdate9094ActiveStream =
        inputs.lateBranchActiveDispatchStream;
    s_stage1RunnerTailGateRuntime.ctx78Ed1CProducerKnown =
        inputs.frameUpdate801C9094Ed1CProducerKnown;
    s_stage1RunnerTailGateRuntime.ctx78Ed1CProducerEventStreamFlag =
        inputs.frameUpdate801C9094Ed1CEventStreamFlag;
    s_stage1RunnerTailGateRuntime.ctx78Ed1CProducerFlagDescriptorValid =
        inputs.frameUpdate801C9094Ed1CFlagDescriptorValid;
    s_stage1RunnerTailGateRuntime.ctx78Ed1CProducerActiveFlagStreamValid =
        inputs.frameUpdate801C9094Ed1CActiveFlagStreamValid;
    s_stage1RunnerTailGateRuntime.ctx78Ed1CProducerActiveFlagStreamIndex =
        inputs.frameUpdate801C9094Ed1CActiveFlagStreamIndex;
    s_stage1RunnerTailGateRuntime.ctx78Ed1CProducerActiveFlagStreamCount =
        inputs.frameUpdate801C9094Ed1CActiveFlagStreamCount;
    s_stage1RunnerTailGateRuntime.ctx78Ed1CProducerDueKnown =
        inputs.frameUpdate801C9094Ed1CActiveFlagStreamDueKnown;
    s_stage1RunnerTailGateRuntime.ctx78Ed1CProducerDueFrame =
        inputs.frameUpdate801C9094Ed1CActiveFlagStreamDueFrame;
    s_stage1RunnerTailGateRuntime.ctx78Ed1CProducerDueDelta =
        inputs.frameUpdate801C9094Ed1CActiveFlagStreamDueDelta;
    s_stage1RunnerTailGateRuntime.ctx78Ed1CProducerFlags04 =
        inputs.frameUpdate801C9094Ed1CActiveFlagStreamFlags04;
    s_stage1RunnerTailGateRuntime.ctx78Ed1CProducerFlag80 =
        inputs.frameUpdate801C9094Ed1CActiveFlagStreamFlag80;
    s_stage1RunnerTailGateRuntime.ctx78Ed1CProducerConsumedFlagStreamEvent =
        inputs.frameUpdate801C9094Ed1CConsumedFlagStreamEvent;
    s_stage1RunnerTailGateRuntime.ctx78Ed1CProducerProduced =
        inputs.frameUpdate801C9094Ed1CProduced;
    s_stage1RunnerTailGateRuntime.eventStreamFlagLastUpdateKnown =
        inputs.eventStreamFlagLastUpdateKnown;
    s_stage1RunnerTailGateRuntime.eventStreamFlagLastUpdateReason =
        inputs.eventStreamFlagLastUpdateReason;
    s_stage1RunnerTailGateRuntime.eventStreamFlagLastUpdateQueryFrame =
        inputs.eventStreamFlagLastUpdateQueryFrame;
    s_stage1RunnerTailGateRuntime.eventStreamFlagLastUpdateScriptFrame =
        inputs.eventStreamFlagLastUpdateScriptFrame;
    s_stage1RunnerTailGateRuntime.eventStreamFlagLastUpdatePrevious =
        inputs.eventStreamFlagLastUpdatePrevious;
    s_stage1RunnerTailGateRuntime.eventStreamFlagLastUpdateCurrent =
        inputs.eventStreamFlagLastUpdateCurrent;
    s_stage1RunnerTailGateRuntime.eventStreamFlagLastChangeKnown =
        inputs.eventStreamFlagLastChangeKnown;
    s_stage1RunnerTailGateRuntime.eventStreamFlagLastChangeReason =
        inputs.eventStreamFlagLastChangeReason;
    s_stage1RunnerTailGateRuntime.eventStreamFlagLastChangeQueryFrame =
        inputs.eventStreamFlagLastChangeQueryFrame;
    s_stage1RunnerTailGateRuntime.eventStreamFlagLastChangeScriptFrame =
        inputs.eventStreamFlagLastChangeScriptFrame;
    s_stage1RunnerTailGateRuntime.eventStreamFlagLastChangePrevious =
        inputs.eventStreamFlagLastChangePrevious;
    s_stage1RunnerTailGateRuntime.eventStreamFlagLastChangeCurrent =
        inputs.eventStreamFlagLastChangeCurrent;

    if (inputs.frameUpdate801C9094Ctx76 == 1u) {
        s_stage1RunnerTailGateRuntime.ctx76Active = true;
    } else if (!s_stage1RunnerTailGateRuntime.ctx76Known) {
        s_stage1RunnerTailGateRuntime.ctx76Active = false;
    }
    s_stage1RunnerTailGateRuntime.ctx76Known = true;
    if (inputs.frameUpdate801C9094Ctx78 == 1u) {
        s_stage1RunnerTailGateRuntime.ctx78Active = true;
    } else if (!s_stage1RunnerTailGateRuntime.ctx78Known) {
        s_stage1RunnerTailGateRuntime.ctx78Active = false;
    }
    s_stage1RunnerTailGateRuntime.ctx78Known = true;

    const bool tailDispatchFamilyActive =
        inputs.lateBranchTailDispatchFamilyActive &&
        inputs.lateBranchTailDispatchFamilyStream != 0u &&
        inputs.lateBranchActiveDispatchStream ==
            inputs.lateBranchTailDispatchFamilyStream;
    const uint8_t tailDispatchFamilyStream =
        inputs.lateBranchTailDispatchFamilyStream;

    if (inputs.frameUpdate801C9094Ctx76 == 1u &&
        tailDispatchFamilyActive &&
        tailDispatchFamilyStream == 4u) {
        s_stage1RunnerTailGateRuntime.ctx76TailStream4Latched = true;
    }
    if (inputs.frameUpdate801C9094Ctx78 == 1u &&
        tailDispatchFamilyActive &&
        tailDispatchFamilyStream >= 2u &&
        tailDispatchFamilyStream <= 4u) {
        s_stage1RunnerTailGateRuntime.ctx78Ed1CHandoffLatched = true;
    }
}

Stage1RunnerTailGateFrameInputs ResolveStage1RunnerTailGateFrameInputs(
    const Stage1NumericRuntimeState& state) {
    const Stage1RunnerTailGateLateBranchSnapshot lateBranch =
        ResolveStage1RunnerTailGateLateBranchSnapshot();
    Stage1RunnerTailGateFrameInputs out{};
    out.bucket0Ctx118WritePulse =
        state.rightRankHelperShadow.bucket0Ctx118WritePulse;
    out.bucket0Ctx118WritePulseQueryFrame =
        state.rightRankHelperShadow.bucket0Ctx118WritePulseQueryFrame;
    out.bucket0Ctx118WritePulseRightRankRow =
        state.rightRankHelperShadow.bucket0Ctx118WritePulseRightRankRow;
    out.bucket0Ctx118WritePulseCallWindowOpen =
        state.rightRankHelperShadow.bucket0Ctx118WritePulseCallWindowOpen;
    out.bucket0Ctx118WritePulseDescriptorFlags =
        state.rightRankHelperShadow.bucket0Ctx118WritePulseDescriptorFlags;
    const Stage1NumericRuntimeState::RunnerSameFrameCtxOwner801C9094Runtime&
        ctx801C9094 = state.runnerPostFrame7A60.sameFrameCtx801C9094;
    // `ctx+0x76/0x78` are ED1C handoff writes in 801C9094. Do not gate
    // their tail-return consumption on the broader ctx.flags00 final-known
    // bit, which can stay open for unrelated compact/event owner gaps.
    out.frameUpdate801C9094CtxKnown =
        ctx801C9094.snapshotAvailable &&
        ctx801C9094.sameFrameCtxKnown;
    out.frameUpdate801C9094QueryFrame = ctx801C9094.queryFrame;
    out.frameUpdate801C9094Ctx76 = ctx801C9094.projectedCtx.word76;
    out.frameUpdate801C9094Ctx78 = ctx801C9094.projectedCtx.word78;
    out.frameUpdate801C9094Ed1CProducerKnown =
        ctx801C9094.ed1CProducerKnown;
    out.frameUpdate801C9094Ed1CEventStreamFlag =
        ctx801C9094.ed1CProducerEventStreamFlag;
    out.frameUpdate801C9094Ed1CFlagDescriptorValid =
        ctx801C9094.ed1CProducerFlagDescriptorValid;
    out.frameUpdate801C9094Ed1CActiveFlagStreamValid =
        ctx801C9094.ed1CProducerActiveFlagStreamValid;
    out.frameUpdate801C9094Ed1CActiveFlagStreamIndex =
        ctx801C9094.ed1CProducerActiveFlagStreamIndex;
    out.frameUpdate801C9094Ed1CActiveFlagStreamCount =
        ctx801C9094.ed1CProducerActiveFlagStreamCount;
    out.frameUpdate801C9094Ed1CActiveFlagStreamDueKnown =
        ctx801C9094.ed1CProducerActiveFlagStreamDueKnown;
    out.frameUpdate801C9094Ed1CActiveFlagStreamDueFrame =
        ctx801C9094.ed1CProducerActiveFlagStreamDueFrame;
    out.frameUpdate801C9094Ed1CActiveFlagStreamDueDelta =
        ctx801C9094.ed1CProducerActiveFlagStreamDueDelta;
    out.frameUpdate801C9094Ed1CActiveFlagStreamFlags04 =
        ctx801C9094.ed1CProducerActiveFlagStreamFlags04;
    out.frameUpdate801C9094Ed1CActiveFlagStreamFlag80 =
        ctx801C9094.ed1CProducerActiveFlagStreamFlag80;
    out.frameUpdate801C9094Ed1CConsumedFlagStreamEvent =
        ctx801C9094.ed1CProducerConsumedFlagStreamEvent;
    out.frameUpdate801C9094Ed1CProduced =
        ctx801C9094.ed1CProducerProduced;
    out.eventStreamFlagLastUpdateKnown =
        lateBranch.eventStreamFlagLastUpdateKnown;
    out.eventStreamFlagLastUpdateReason =
        lateBranch.eventStreamFlagLastUpdateReason;
    out.eventStreamFlagLastUpdateQueryFrame =
        lateBranch.eventStreamFlagLastUpdateQueryFrame;
    out.eventStreamFlagLastUpdateScriptFrame =
        lateBranch.eventStreamFlagLastUpdateScriptFrame;
    out.eventStreamFlagLastUpdatePrevious =
        lateBranch.eventStreamFlagLastUpdatePrevious;
    out.eventStreamFlagLastUpdateCurrent =
        lateBranch.eventStreamFlagLastUpdateCurrent;
    out.eventStreamFlagLastChangeKnown =
        lateBranch.eventStreamFlagLastChangeKnown;
    out.eventStreamFlagLastChangeReason =
        lateBranch.eventStreamFlagLastChangeReason;
    out.eventStreamFlagLastChangeQueryFrame =
        lateBranch.eventStreamFlagLastChangeQueryFrame;
    out.eventStreamFlagLastChangeScriptFrame =
        lateBranch.eventStreamFlagLastChangeScriptFrame;
    out.eventStreamFlagLastChangePrevious =
        lateBranch.eventStreamFlagLastChangePrevious;
    out.eventStreamFlagLastChangeCurrent =
        lateBranch.eventStreamFlagLastChangeCurrent;
    out.lateBranchTailDispatchFamilyActive =
        lateBranch.tailDispatchFamilyActive;
    out.lateBranchTailDispatchFamilyStream =
        lateBranch.tailDispatchFamilyStream;
    out.lateBranchActiveDispatchStream = lateBranch.activeDispatchStream;
    return out;
}

bool PrimeStage1FormalLifecycleRuntime(
    PrGameContext& ctx,
    const PrStageRunner& runner,
    const Stage1FormalLifecycleFrameInputs& inputs) {
    if (!IsStage1FormalLifecycleRuntimeGateActiveImpl(ctx, runner)) {
        ResetStage1FormalLifecycleRuntime();
        return false;
    }

    const Stage1RunnerTimingRoots30 timing = ResolveStage1RunnerTimingRoots30(runner);
    const int runnerFrame = timing.runnerFrame;
    if (runnerFrame < 0) {
        ResetStage1FormalLifecycleRuntime();
        return false;
    }

    Stage1FormalLifecycleRuntime& runtime = s_stage1FormalLifecycleRuntime;
    if (!runtime.active || runnerFrame < runtime.lastRunnerFrame) {
        ResetStage1FormalLifecycleRuntime();
    }

    runtime.active = true;
    runtime.queryFrame = timing.queryFrame;
    if (inputs.numericActive) {
        runtime.lastKnownRightRankRow =
            (std::min<uint8_t>)(inputs.numericRightRankActiveRow, 3u);
    }
    RefreshStage1FormalLifecycleExitResultObserverRuntimeImpl(inputs, runtime);

    const bool clearTerminalTailCompleted =
        inputs.lateBranchFlag100BlocksWaitActive &&
        (inputs.lateBranchFlag100SourceStream == 2u ||
         inputs.lateBranchFlag100SourceStream == 3u);
    const bool clearGate = clearTerminalTailCompleted;
    const bool rawClearGate = clearGate;
    const bool failSamplingActive = !inputs.introTransitionActive;
    if (runnerFrame != runtime.lastRunnerFrame) {
        if (failSamplingActive &&
            !rawClearGate &&
            runtime.lastKnownRightRankRow == 3u) {
            runtime.awfulHoldFrames++;
        } else {
            runtime.awfulHoldFrames = 0u;
        }
    }

    runtime.clearGate = clearGate;
    runtime.clearTerminalTailGate =
        inputs.lateBranchClearTerminalTailArmed &&
        !clearTerminalTailCompleted;
    const bool failStream100Completed =
        inputs.lateBranchFlag100BlocksWaitActive &&
        (inputs.lateBranchFlag100SourceStream == 4u ||
         inputs.lateBranchFlag100SourceStream == 5u);
    runtime.failGate =
        failSamplingActive &&
        !runtime.clearGate &&
        failStream100Completed;
    runtime.lastRunnerFrame = runnerFrame;
    return runtime.active;
}

void LatchStage1TerminalFormalLifecycleSnapshot() {
    PopulateStage1FormalLifecycleSnapshotFromRuntime(
        s_stage1FormalLifecycleRuntime,
        s_stage1TerminalFormalLifecycleSnapshot);
}

uint32_t ResolveStage1GameplayRailAcceptedSourceCellId(size_t rowIndex,
                                                       uint8_t classToken,
                                                       uint16_t sourceCellCursor) {
    return (((uint32_t)rowIndex & 0xFFu) << 24) |
           (((uint32_t)classToken & 0xFFu) << 16) |
           (uint32_t)sourceCellCursor;
}

void UpdateStage1NextDescriptorConsumerMirror(const PrGameContext& ctx,
                                              Stage1NumericRuntimeState& state,
                                              int tick96,
                                              uint16_t heldMask) {
    (void)heldMask;
    Stage1NumericRuntimeState::NextDescriptorConsumerRuntime& consumer =
        state.nextDescriptorConsumer;
    consumer = Stage1NumericRuntimeState::NextDescriptorConsumerRuntime{};

    uint8_t selectorByte0 = 0;
    uint8_t selectorByte1 = 0;
    if (!TryResolveStage1AcceptedProducerSelectors(
            state,
            selectorByte0,
            selectorByte1)) {
        return;
    }

    if (!state.acceptedProducerCarrier.available) {
        return;
    }
    consumer.cueCallback.available = true;
    const uint8_t classToken = state.acceptedProducerCarrier.classToken20;
    Stage1NumericRuntimeState::SelectedSourceGroupRailWindowRuntime
        selectedSourceGroupWindow{};
    BuildStage1SelectedSourceGroupRailWindow(
        ctx,
        state.acceptedProducerSourceGroup,
        selectorByte0,
        selectorByte1,
        classToken,
        tick96,
        selectedSourceGroupWindow);
    consumer.cueCallback.timingTemplateState =
        selectedSourceGroupWindow.timingTemplateStateKnown
            ? selectedSourceGroupWindow.timingTemplateState
            : 0u;
    consumer.cueCallback.sourceCellCallbackPresent =
        selectedSourceGroupWindow.sourceCellPresent;
    consumer.cueCallback.sourceCellCallbackArgPresent =
        selectedSourceGroupWindow.sourceCellCallbackArgPresent;
    consumer.cueCallback.callbackHookArmed =
        consumer.cueCallback.sourceCellCallbackArgPresent &&
        IsStage1NextDescriptorCallbackHookArmed();
}

void UpdateStage1GameplayRailCadenceProducerMirror(const PrGameContext& ctx,
                                                   Stage1NumericRuntimeState& state,
                                                   int tick96) {
    Stage1NumericRuntimeState::GameplayRailCadenceProducerRuntime& producer =
        state.gameplayRailCadenceProducer;
    producer = Stage1NumericRuntimeState::GameplayRailCadenceProducerRuntime{};

    const Stage1NumericRuntimeState::DescriptorCadenceRuntime& descriptorRuntime =
        state.descriptorCadence;
    const uint8_t routeSubstateIndex =
        ResolveStage1NextDescriptorConsumerSubstateIndex(state);

    if (descriptorRuntime.currentCommittedAvailable &&
        descriptorRuntime.currentCommittedRow.available) {
        BuildStage1GameplayRailDescriptorProducerRuntime(
            ctx,
            state,
            descriptorRuntime.currentCommittedRow,
            descriptorRuntime.currentCommittedRowIndex,
            routeSubstateIndex,
            tick96,
            producer.currentCommitted);
    }

    if (descriptorRuntime.nextLookaheadAvailable &&
        descriptorRuntime.nextLookaheadRow.available) {
        BuildStage1GameplayRailDescriptorProducerRuntime(
            ctx,
            state,
            descriptorRuntime.nextLookaheadRow,
            descriptorRuntime.nextLookaheadRowIndex,
            routeSubstateIndex,
            tick96,
            producer.nextLookahead);
    }
}

bool MirrorStage1SharedAcceptedProducerFromRawInput(
    const PrGameContext& ctx,
    Stage1NumericRuntimeState& state,
    int tick96,
    bool sameFrameWriteCtx10CurrentTick,
    uint16_t writerControlSample18,
    uint16_t rawPressedMask,
    uint16_t rawAcceptedMask,
    PrStage1ScorerDirectAcceptedProducerRunResult* outDirectRun,
    const PrStage1ScorerDirectPageClear14BDCResult* sameTickPageClear14BDC) {
    Stage1NumericRuntimeState::SharedAcceptedProducerBoundaryProbeRuntime& probe =
        state.acceptedProducerBoundaryProbe;
    probe.rawPressedMask = rawPressedMask;
    probe.rawAcceptedMask = rawAcceptedMask;
    Stage1AcceptedFrontDoorPacket7A60 frontDoor{};
    if (!TryBuildStage1AcceptedFrontDoorPacket7A60(
            ctx,
            state,
            tick96,
            sameFrameWriteCtx10CurrentTick,
            writerControlSample18,
            rawPressedMask,
            rawAcceptedMask,
            frontDoor)) {
        return false;
    }
    probe.directInputDescriptorSubstate50 =
        frontDoor.directInput.descriptorSubstate50;
    probe.directInputLookaheadRowValid = frontDoor.directRow.valid;
    probe.directInputLookaheadLessonId = frontDoor.directRow.byte00LessonId;
    probe.directInputDefaultSelectorByte0 =
        frontDoor.directRow.byte12DefaultSelector0;
    probe.directInputDefaultSelectorByte1 =
        frontDoor.directRow.byte13DefaultSelector1;
    probe.directInputSubstate1SelectorByte0 =
        frontDoor.directRow.byte18Substate1Selector0;
    probe.directInputSubstate1SelectorByte1 =
        frontDoor.directRow.byte19Substate1Selector1;
    if (frontDoor.directInput.descriptorSubstate50 != 0u) {
        probe.directInputBranchSelectorByte0 =
            frontDoor.directRow.byte18Substate1Selector0;
        probe.directInputBranchSelectorByte1 =
            frontDoor.directRow.byte19Substate1Selector1;
    } else {
        probe.directInputBranchSelectorByte0 =
            frontDoor.directRow.byte12DefaultSelector0;
        probe.directInputBranchSelectorByte1 =
            frontDoor.directRow.byte13DefaultSelector1;
    }

    Stage1DirectPortAcceptedProducerAccessorContext accessorContext{&ctx};
    const PrStage1ScorerDirectAcceptedProducerAccessors accessors{
        &accessorContext,
        ReadStage1DirectPortTimingTemplateStateAdapter,
        ReadStage1DirectPortSourceCellHeaderAdapter,
        ReadStage1DirectPortSourceCellAdapter,
    };

    PrStage1ScorerDirectAcceptedProducerOwnerState ownerState =
        BuildStage1DirectPortAcceptedOwnerStateAdapter(state);
    PrStage1ScorerDirectGlobals directGlobals =
        BuildStage1DirectPortGlobalsAdapter(state);
    PrStage1ScorerDirectReplayBufferState replayState =
        BuildStage1DirectPortReplayBufferAdapter(state);
    const PrStage1ScorerDirectPageClear14BDCResult* const activePageClear =
        (sameTickPageClear14BDC && sameTickPageClear14BDC->clearApplied)
            ? sameTickPageClear14BDC
            : nullptr;
    const PrStage1ScorerDirectAcceptedProducerRunResult directRun =
        activePageClear
            ? PrStage1ScorerDirectRunAcceptedProducerWithLatentPageClear14614(
                  directGlobals,
                  ownerState,
                  replayState,
                  frontDoor.directInput,
                  accessors,
                  activePageClear)
            : PrStage1ScorerDirectRunAcceptedProducer14614(
                  directGlobals,
                  ownerState,
                  replayState,
                  frontDoor.directInput,
                  accessors);
    if (outDirectRun) {
        *outDirectRun = directRun;
    }
    const PrStage1ScorerDirectAcceptedProducerSideEffectResult sideEffect =
        directRun.sideEffect;
    const uint8_t resolvedSplit =
        directRun.resolved.recordedSplit
            ? (uint8_t)Stage1AcceptedProducerSplit::Recorded
            : (uint8_t)Stage1AcceptedProducerSplit::PenaltySide;
    StoreStage1DirectPortAcceptedOwnerStateAdapter(state, ownerState);
    StoreStage1DirectPortAcceptedProducerNonPageGlobals14614(
        state,
        directGlobals);
    RememberStage1AcceptedProducerLastRecordedPageWrite14614(
        state,
        directRun.pageStorageApply);
    StoreStage1DirectPortReplayBufferAdapter(
        state,
        replayState);

    if (directRun.resolved.selectorResolved) {
        probe.selectorAvailable = true;
        probe.selectorByte0 = directRun.resolved.selectorByte0;
        probe.selectorByte1 = directRun.resolved.selectorByte1;
    }
    probe.classToken = directRun.resolved.classToken20;
    probe.timingTemplateState = directRun.resolved.timingTemplateState;
    if (sideEffect.playSourceCellCue) {
        probe.sourceCellGateActive = true;
        probe.sourceCellCursor = directRun.resolved.sourceCellCursor;
        MaterializeStage1SourceCellVoiceLaneMirrorFromDirectPort(
            state,
            sideEffect,
            rawAcceptedMask);
    }

    if (directRun.resultCode != 0 && directRun.resultCode != -9) {
        return true;
    }
    if (directRun.resultCode == -9) {
        return true;
    }

    if (directRun.writeResult.resultCode != 0) {
        return true;
    }

    Stage1NumericRuntimeState::SharedAcceptedProducerRuntime& producer =
        state.acceptedProducer;
    producer.acceptedContributionCount++;
    producer.aggregateAcceptedMask |=
        (uint32_t)directRun.resolved.writeback.dword18AcceptedClassMask;
    probe.materialized = true;

    const Stage1AcceptedProducerSplit split =
        (resolvedSplit == (uint8_t)Stage1AcceptedProducerSplit::Recorded)
            ? Stage1AcceptedProducerSplit::Recorded
            : Stage1AcceptedProducerSplit::PenaltySide;
    probe.split = resolvedSplit;
    if (split == Stage1AcceptedProducerSplit::Recorded) {
        producer.recordedSplitCount++;
    } else if (split == Stage1AcceptedProducerSplit::PenaltySide) {
        producer.penaltySideSplitCount++;
    }
    return true;
}

uint16_t ResolveStage1LocalAcceptedHoldMask80035510(uint16_t returnedMask) {
    return static_cast<uint16_t>(
        returnedMask &
        (kStage1GameplayAcceptedInputMask |
         (uint16_t)PrPadButton::Up |
         (uint16_t)PrPadButton::Down));
}

uint16_t ResolveStage1RunnerNormalizedMask801C7A60(uint16_t returnedMask) {
    if ((returnedMask & (uint16_t)PrPadButton::Up) != 0u) {
        return static_cast<uint16_t>(
            (returnedMask & 0xFFFAu) | (uint16_t)PrPadButton::Left);
    }
    if ((returnedMask & (uint16_t)PrPadButton::Down) != 0u) {
        return static_cast<uint16_t>(
            (returnedMask & 0xFFF5u) | (uint16_t)PrPadButton::Right);
    }
    return returnedMask;
}

uint16_t ResolveStage1LocalAcceptedConsumedHoldMask80035510(
    uint16_t localHoldMask,
    uint16_t acceptedMask9FF) {
    uint16_t consumed = static_cast<uint16_t>(localHoldMask & acceptedMask9FF);
    const uint16_t normalizedHold =
        ResolveStage1RunnerNormalizedMask801C7A60(localHoldMask);
    if ((localHoldMask & (uint16_t)PrPadButton::Up) != 0u &&
        (normalizedHold & acceptedMask9FF & (uint16_t)PrPadButton::Left) != 0u) {
        consumed = static_cast<uint16_t>(
            consumed | (uint16_t)PrPadButton::Up);
    }
    if ((localHoldMask & (uint16_t)PrPadButton::Down) != 0u &&
        (normalizedHold & acceptedMask9FF & (uint16_t)PrPadButton::Right) != 0u) {
        consumed = static_cast<uint16_t>(
            consumed | (uint16_t)PrPadButton::Down);
    }
    return consumed;
}

uint16_t BuildStage1NumericRuntimeInputMask(Stage1NumericRuntimeState& state,
                                            const PrGameContext& ctx,
                                            uint16_t localPrPadMask) {
    const uint16_t localReturnedMask =
        PrPsxPadDirect::NormalizeLocalPrPadMaskToReturnedMask80035510(
            localPrPadMask);
    const uint16_t debugReturnedMask =
        PrPsxPadDirect::NormalizeDebugServerPsxPadMaskToReturnedMask80035510(
            ctx.debugPadInput);
    const uint16_t localAcceptedHold =
        ResolveStage1LocalAcceptedHoldMask80035510(localReturnedMask);
    const bool bypassDebounce =
        debugReturnedMask != 0u || ResolveStage1Ctx52ReplayMode7A60(state);
    state.localAcceptedProducerHoldMask80035510 = localAcceptedHold;
    state.localAcceptedProducerHoldDebounceBypassed80035510 = bypassDebounce;
    if (bypassDebounce) {
        state.localAcceptedProducerConsumedHoldMask80035510 = 0u;
    } else {
        state.localAcceptedProducerConsumedHoldMask80035510 =
            static_cast<uint16_t>(
                state.localAcceptedProducerConsumedHoldMask80035510 &
                localAcceptedHold);
    }

    const uint16_t localDebouncedMask =
        bypassDebounce
            ? localReturnedMask
            : static_cast<uint16_t>(
                  localReturnedMask &
                  ~state.localAcceptedProducerConsumedHoldMask80035510);
    const uint16_t padReturnedMask =
        static_cast<uint16_t>(localDebouncedMask | debugReturnedMask);
    const PrPsxPadDirect::PadReadResult80035510 pad80035510 =
        PrPsxPadDirect::PsxReadPadMask80035510(padReturnedMask);
    return pad80035510.psxReturnMask;
}

uint16_t GetStage1NumericRuntimeHeldMask(Stage1NumericRuntimeState& state,
                                         const PrGameContext& ctx) {
    return BuildStage1NumericRuntimeInputMask(
        state,
        ctx,
        PrPad::GetState(0).held);
}

void MarkStage1LocalAcceptedProducerHoldMaskConsumed(
    Stage1NumericRuntimeState& state,
    uint16_t acceptedMask9FF) {
    if (state.localAcceptedProducerHoldDebounceBypassed80035510) {
        return;
    }
    const uint16_t consumedNow =
        ResolveStage1LocalAcceptedConsumedHoldMask80035510(
            state.localAcceptedProducerHoldMask80035510,
            acceptedMask9FF);
    state.localAcceptedProducerConsumedHoldMask80035510 =
        static_cast<uint16_t>(
            state.localAcceptedProducerConsumedHoldMask80035510 |
            consumedNow);
}

size_t ResolveStage1PageRecordRawOffsetForSlotIndex(size_t slotIndex) {
    const size_t clampedSlotIndex =
        (std::min)(slotIndex, kStage1PageRecordRawSlotsPerPage - 1u);
    return clampedSlotIndex * kStage1PageRecordRawRecordSizeBytes;
}

uint16_t ReadStage1PageRecordRawU16(
    const Stage1NumericRuntimeState::PageRecordMirrorPage& page,
    size_t byteOffset) {
    return (uint16_t)page.raw[byteOffset] |
           (uint16_t)((uint16_t)page.raw[byteOffset + 1u] << 8);
}

uint32_t ReadStage1PageRecordRawU32(
    const Stage1NumericRuntimeState::PageRecordMirrorPage& page,
    size_t byteOffset) {
    return (uint32_t)page.raw[byteOffset] |
           ((uint32_t)page.raw[byteOffset + 1u] << 8) |
           ((uint32_t)page.raw[byteOffset + 2u] << 16) |
           ((uint32_t)page.raw[byteOffset + 3u] << 24);
}

void WriteStage1PageRecordRawU16(
    Stage1NumericRuntimeState::PageRecordMirrorPage& page,
    size_t byteOffset,
    uint16_t value) {
    page.raw[byteOffset] = (uint8_t)(value & 0xFFu);
    page.raw[byteOffset + 1u] = (uint8_t)((value >> 8) & 0xFFu);
}

void WriteStage1PageRecordRawU32(
    Stage1NumericRuntimeState::PageRecordMirrorPage& page,
    size_t byteOffset,
    uint32_t value) {
    page.raw[byteOffset] = (uint8_t)(value & 0xFFu);
    page.raw[byteOffset + 1u] = (uint8_t)((value >> 8) & 0xFFu);
    page.raw[byteOffset + 2u] = (uint8_t)((value >> 16) & 0xFFu);
    page.raw[byteOffset + 3u] = (uint8_t)((value >> 24) & 0xFFu);
}

uint16_t ResolveStage1PageRecordSlotOccupiedCount(
    const Stage1NumericRuntimeState::PageRecordMirrorPage& page,
    size_t slotIndex) {
    return ReadStage1PageRecordRawU16(
        page,
        ResolveStage1PageRecordRawOffsetForSlotIndex(slotIndex) + 6u);
}

uint32_t ResolveStage1PageRecordSlotMask(
    const Stage1NumericRuntimeState::PageRecordMirrorPage& page,
    size_t slotIndex) {
    return ReadStage1PageRecordRawU32(
        page,
        ResolveStage1PageRecordRawOffsetForSlotIndex(slotIndex));
}

uint16_t ResolveStage1PageRecordSlotClassToken(
    const Stage1NumericRuntimeState::PageRecordMirrorPage& page,
    size_t slotIndex) {
    return ReadStage1PageRecordRawU16(
        page,
        ResolveStage1PageRecordRawOffsetForSlotIndex(slotIndex) + 4u);
}

uint32_t ResolveStage1PageRecordSlotSourceCellPtr(
    const Stage1NumericRuntimeState::PageRecordMirrorPage& page,
    size_t slotIndex) {
    return ReadStage1PageRecordRawU32(
        page,
        ResolveStage1PageRecordRawOffsetForSlotIndex(slotIndex) + 8u);
}

PrStage1ScorerDirectDescriptorRow BuildStage1DirectPortDescriptorRowAdapter(
    const Stage1NumericRuntimeState::DescriptorRowView& rowView) {
    PrStage1ScorerDirectDescriptorRow out{};
    out.valid = rowView.available;
    out.byte00LessonId = rowView.lessonId;
    out.defaultBranch.byte01AnchorSlotIndex = rowView.anchorSlotIndex;
    out.defaultBranch.byte02RequiredClassToken = rowView.requiredClassToken;
    out.defaultBranch.byte03PenaltyWeight = rowView.penaltyWeight;
    out.defaultBranch.dword08RequiredMask = rowView.requiredMask;
    out.defaultBranch.word0ELookbackPageCount =
        (uint16_t)(std::max<int16_t>)(rowView.defaultLookbackPageCount, 0);
    out.defaultBranch.word10FlagWord = rowView.defaultFlagWord;
    out.substate1Branch.byte01AnchorSlotIndex = rowView.anchorSlotIndex;
    out.substate1Branch.byte02RequiredClassToken = rowView.requiredClassToken;
    out.substate1Branch.byte03PenaltyWeight = rowView.penaltyWeight;
    out.substate1Branch.dword08RequiredMask = rowView.requiredMask;
    out.substate1Branch.word0ELookbackPageCount =
        (uint16_t)(std::max<int16_t>)(rowView.substate1LookbackPageCount, 0);
    out.substate1Branch.word10FlagWord = rowView.substate1FlagWord;
    out.word10FinalResolutionThreshold = rowView.finalResolutionThreshold;
    out.byte12DefaultSelector0 = rowView.defaultSelectorByte0;
    out.byte13DefaultSelector1 = rowView.defaultSelectorByte1;
    out.byte18Substate1Selector0 = rowView.substate1SelectorByte0;
    out.byte19Substate1Selector1 = rowView.substate1SelectorByte1;
    return out;
}

static bool TryBuildStage1DirectPortCommittedDescriptorRowAdapter(
    const Stage1NumericRuntimeState& state,
    PrStage1ScorerDirectDescriptorRow& outRow) {
    outRow = PrStage1ScorerDirectDescriptorRow{};
    const Stage1NumericRuntimeState::DescriptorRowView* rowView =
        ResolveStage1FormalScorerCommittedDescriptorRow(state);
    if (!rowView) {
        return false;
    }
    outRow = BuildStage1DirectPortDescriptorRowAdapter(*rowView);
    return true;
}

uint16_t ResolveStage1RightRankDescriptorFlagWord(
    const Stage1NumericRuntimeState& state) {
    return PrStage1ScorerDirectResolveDescriptorFlagWordWithLatch24FD0(
        state.rightRankDescriptorFlagWord08Known,
        state.rightRankDescriptorFlagWord08);
}

PrStage1ScorerDirectGlobals BuildStage1DirectPortGlobalsAdapter(
    const Stage1NumericRuntimeState& state) {
    PrStage1ScorerDirectGlobals out{};
    out.word916E2CurrentSceneIndex = 1u;
    out.dword91808AcceptedMask = state.scorerPort.aggregateAcceptedMask91808;
    out.dword9180CLastClearedAcceptedMask =
        state.scorerPort.lastClearedAcceptedMask9180C;
    out.word91810AcceptedCount = state.scorerPort.acceptedCount91810;
    out.word91812RecordWriteShadowCount = state.scorerPort.recordedHitCount91812;
    out.word91814PenaltySplitCount = state.scorerPort.penaltySideCount91814;
    out.word91816Accumulator = (int16_t)state.scorerPort.accumulator91816;
    out.word91818SharedBaseline =
        (int16_t)state.rightRankPhase1Owner.baselineValue18;
    out.word9181ASnapshot =
        (int16_t)state.rightRankTieBreakerObserver.snapshot1A;
    out.word9181CCompareBaseline =
        (int16_t)state.rightRankHelperShadow.snapshot1C;
    out.word9181ETwitterCarryLatch =
        state.rightRankHelperShadow.tieCarryLatch1E;
    out.word91820NoInputCounter = state.rightRankHelperShadow.noInputCounter20;
    out.word91822AdditiveTerm = state.scorerPort.additiveTerm91822;
    out.word91824RecordCompanion = state.scorerPort.recordCompanion91824;
    out.word91826CoolGainLatch = state.rightRankHelperShadow.coolGainLatch26;
    out.word91828CoolGainStreak = state.rightRankHelperShadow.coolGainStreak28;
    out.word9182APairBonusGate = state.word9182AEnabled ? 1u : 0u;
    out.dword8ED00FollowUpState = state.rightRankDirectFollowUp.ed00;
    out.dword8ED08DescriptorFlags =
        ResolveStage1RightRankDescriptorFlagWord(state);
    out.dword8ED0CFollowUpDeadlineTick = state.rightRankDirectFollowUp.ed0c;
    out.dword8ED14FollowUpActive = state.rightRankDirectFollowUp.ed14;
    out.dword8ED24FollowUpWriteback = state.rightRankDirectFollowUp.ed24;
    out.word8ED34ShortWindowLatch =
        state.rightRankPhase1Owner.shortWindowLatch34Known
            ? state.rightRankPhase1Owner.shortWindowLatch34
            : static_cast<uint16_t>(
                  state.rightRankHelperShadow.bucket0CallWindowOpen ? 1u : 0u);
    out.word8ED36Phase1Cache =
        state.rightRankPhase1Owner.cachedPhase1Classifier36;
    out.word8ED38PhaseCounter =
        state.rightRankPhase1Owner.phase1LatchArmed38 ? 1u : 0u;
    out.gPrStageEventStreamFlagKnown =
        state.psxEventStreamFlagKnown ||
        state.acceptedProducerCarrier.eventStreamFlagKnown;
    out.gPrStageEventStreamFlag =
        state.psxEventStreamFlagKnown
            ? state.psxEventStreamFlagActive
            : (state.acceptedProducerCarrier.eventStreamFlagKnown &&
               state.acceptedProducerCarrier.eventStreamFlagActive);
    out.gPrStageEventStreamIdKnown =
        state.psxEventStreamIdKnown ||
        state.acceptedProducerCarrier.eventStreamIdRawKnown;
    out.gPrStageEventStreamId =
        state.psxEventStreamIdKnown
            ? state.psxEventStreamId
            : state.acceptedProducerCarrier.eventStreamIdRaw;
    out.gPrStageEventStreamDone = state.rightRankForcedGoodEventStreamDone28;
    out.currentPageOrdinal1Based =
        (state.descriptorCadence.pageOrdinal56Available &&
         state.descriptorCadence.pageOrdinal56 > 0u)
            ? static_cast<uint8_t>(
                  (std::min)(state.descriptorCadence.pageOrdinal56,
                             (size_t)0xFFu))
            : (state.pageRecordMirror.initialized
                   ? static_cast<uint8_t>(
                         state.pageRecordMirror.currentPageOrdinal + 1)
                   : 0u);
    out.currentWritePageOrdinal1Based =
        static_cast<uint8_t>(state.pageOwner.currentWritePageOrdinal1Based);
    out.currentWritePageOrdinalKnown =
        state.pageOwner.currentWritePageOrdinalKnown;
    out.pageClearPending14BDC = false;
    out.pageClearOrdinal1Based14BDC = 0u;

    for (size_t pageIndex = 0u; pageIndex < kPrStage1ScorerDirectRingPageCount;
         ++pageIndex) {
        const Stage1NumericRuntimeState::PageRecordMirrorPage& page =
            state.pageRecordMirror.pages[pageIndex];
        for (size_t slotIndex = 0u; slotIndex < kPrStage1ScorerDirectSlotsPerPage;
             ++slotIndex) {
            PrStage1ScorerDirectRawRecord& record =
                out.ringPages[pageIndex].records[slotIndex];
            record.dword00AcceptedMask =
                ResolveStage1PageRecordSlotMask(page, slotIndex);
            record.word04Companion =
                ResolveStage1PageRecordSlotClassToken(page, slotIndex);
            record.word06Occupied =
                ResolveStage1PageRecordSlotOccupiedCount(page, slotIndex);
            record.dword08Payload =
                ResolveStage1PageRecordSlotSourceCellPtr(page, slotIndex);
        }
    }
    return out;
}

bool ResolveStage1DirectBusyGate24BF4(
    const Stage1NumericRuntimeState& state,
    int tick96) {
    const uint8_t streamId =
        state.psxEventStreamIdKnown
            ? state.psxEventStreamId
            : (state.acceptedProducerCarrier.eventStreamIdRawKnown
                   ? state.acceptedProducerCarrier.eventStreamIdRaw
                   : 0u);

    const int32_t ed00 = state.rightRankDirectFollowUp.ed00;
    const int32_t ed14 = state.rightRankDirectFollowUp.ed14;
    const int32_t ed0c = state.rightRankDirectFollowUp.ed0c;

    if (ed00 == 1) {
        if (ed14 == 0) {
            return false;
        }
        return tick96 <= ed0c;
    }

    const bool sameFrameEventStreamStart =
        state.rightRankDirectFollowUp.flag2000Pulse ||
        state.rightRankDirectFollowUp.flag4000Pulse;
    if (streamId == 0u && !sameFrameEventStreamStart) {
        return ed14 != 0 && ed00 == 5;
    }
    return true;
}

void StoreStage1DirectPortAcceptedProducerNonPageGlobals14614(
    Stage1NumericRuntimeState& state,
    const PrStage1ScorerDirectGlobals& globals) {
    state.scorerPort.aggregateAcceptedMask91808 =
        globals.dword91808AcceptedMask;
    state.scorerPort.acceptedCount91810 = globals.word91810AcceptedCount;
    state.scorerPort.recordedHitCount91812 =
        globals.word91812RecordWriteShadowCount;
    state.scorerPort.penaltySideCount91814 =
        globals.word91814PenaltySplitCount;
    state.scorerPort.recordCompanion91824 = globals.word91824RecordCompanion;
}

void StoreStage1DirectPortBucket31ConsumerGlobals24FD0(
    Stage1NumericRuntimeState& state,
    const PrStage1ScorerDirectGlobals& globals,
    const PrStage1ScorerDirectBucketContext& ctx,
    const PrStage1ScorerDirectBucket31ConsumerSliceResult& consumer) {
    state.rightRankDirectFollowUp.ed00 = globals.dword8ED00FollowUpState;
    if (!consumer.bucketLocalClearRan) {
        return;
    }

    state.scorerPort.aggregateAcceptedMask91808 =
        globals.dword91808AcceptedMask;
    state.scorerPort.lastClearedAcceptedMask9180C =
        globals.dword9180CLastClearedAcceptedMask;
    state.scorerPort.acceptedCount91810 = globals.word91810AcceptedCount;
    state.scorerPort.recordedHitCount91812 =
        globals.word91812RecordWriteShadowCount;
    state.scorerPort.penaltySideCount91814 =
        globals.word91814PenaltySplitCount;
    state.scorerPort.additiveTerm91822 = globals.word91822AdditiveTerm;
    state.rightRankHelperShadow.tieCarryLatch1E =
        globals.word9181ETwitterCarryLatch;
    state.rightRankHelperShadow.noInputCounter20 =
        globals.word91820NoInputCounter;
    state.rightRankPhase1Owner.baselineValue18 =
        globals.word91818SharedBaseline;
    state.rightRankTieBreakerObserver.snapshot1A =
        globals.word9181ASnapshot;
    state.rightRankHelperShadow.snapshot1C =
        globals.word9181CCompareBaseline;
    state.rightRankHelperShadow.coolGainLatch26 = globals.word91826CoolGainLatch;
    state.rightRankHelperShadow.coolGainStreak28 =
        globals.word91828CoolGainStreak;
    state.rightRankPhase1Owner.cachedPhase1Classifier36 =
        globals.word8ED36Phase1Cache;
    state.rightRankPhase1Owner.phase1LatchArmed38 =
        globals.word8ED38PhaseCounter != 0u;
    state.rightRankState.rightRankBlinkTargetRow =
        static_cast<uint8_t>(ctx.word58BlinkTargetRow & 0x00FFu);
    state.rightRankState.rightRankBlinkEnabled =
        ctx.word5ABlinkEnabled != 0u;
}

void StoreStage1DirectPortBucket30OwnerGlobals24FD0(
    Stage1NumericRuntimeState& state,
    const PrStage1ScorerDirectGlobals& preGlobals,
    const PrStage1ScorerDirectGlobals& globals,
    const PrStage1ScorerDirectBucketContext& preCtx,
    const PrStage1ScorerDirectBucketContext& ctx) {
    if (globals.dword91808AcceptedMask != preGlobals.dword91808AcceptedMask) {
        state.scorerPort.aggregateAcceptedMask91808 =
            globals.dword91808AcceptedMask;
    }
    if (globals.dword9180CLastClearedAcceptedMask !=
        preGlobals.dword9180CLastClearedAcceptedMask) {
        state.scorerPort.lastClearedAcceptedMask9180C =
            globals.dword9180CLastClearedAcceptedMask;
    }
    if (globals.word91810AcceptedCount != preGlobals.word91810AcceptedCount) {
        state.scorerPort.acceptedCount91810 = globals.word91810AcceptedCount;
    }
    if (globals.word91812RecordWriteShadowCount !=
        preGlobals.word91812RecordWriteShadowCount) {
        state.scorerPort.recordedHitCount91812 =
            globals.word91812RecordWriteShadowCount;
    }
    if (globals.word91814PenaltySplitCount !=
        preGlobals.word91814PenaltySplitCount) {
        state.scorerPort.penaltySideCount91814 =
            globals.word91814PenaltySplitCount;
    }
    if (globals.word91816Accumulator != preGlobals.word91816Accumulator) {
        state.scorerPort.accumulator91816 = globals.word91816Accumulator;
    }
    if (globals.word91818SharedBaseline !=
        preGlobals.word91818SharedBaseline) {
        state.rightRankPhase1Owner.baselineValue18 =
            globals.word91818SharedBaseline;
    }
    if (globals.word9181ASnapshot != preGlobals.word9181ASnapshot) {
        state.rightRankTieBreakerObserver.snapshot1A =
            globals.word9181ASnapshot;
    }
    if (globals.word9181CCompareBaseline !=
        preGlobals.word9181CCompareBaseline) {
        state.rightRankHelperShadow.snapshot1C =
            globals.word9181CCompareBaseline;
    }
    if (globals.word9181ETwitterCarryLatch !=
        preGlobals.word9181ETwitterCarryLatch) {
        state.rightRankHelperShadow.tieCarryLatch1E =
            globals.word9181ETwitterCarryLatch;
    }
    if (globals.word91820NoInputCounter != preGlobals.word91820NoInputCounter) {
        state.rightRankHelperShadow.noInputCounter20 =
            globals.word91820NoInputCounter;
    }
    if (globals.word91822AdditiveTerm != preGlobals.word91822AdditiveTerm) {
        state.scorerPort.additiveTerm91822 = globals.word91822AdditiveTerm;
    }
    if (globals.word91826CoolGainLatch != preGlobals.word91826CoolGainLatch) {
        state.rightRankHelperShadow.coolGainLatch26 =
            globals.word91826CoolGainLatch;
    }
    if (globals.word91828CoolGainStreak != preGlobals.word91828CoolGainStreak) {
        state.rightRankHelperShadow.coolGainStreak28 =
            globals.word91828CoolGainStreak;
    }
    if (globals.dword8ED00FollowUpState !=
        preGlobals.dword8ED00FollowUpState) {
        state.rightRankDirectFollowUp.ed00 = globals.dword8ED00FollowUpState;
    }
    if (globals.dword8ED0CFollowUpDeadlineTick !=
        preGlobals.dword8ED0CFollowUpDeadlineTick) {
        state.rightRankDirectFollowUp.ed0c =
            globals.dword8ED0CFollowUpDeadlineTick;
    }
    if (globals.dword8ED14FollowUpActive !=
        preGlobals.dword8ED14FollowUpActive) {
        state.rightRankDirectFollowUp.ed14 = globals.dword8ED14FollowUpActive;
    }
    if (globals.dword8ED24FollowUpWriteback !=
        preGlobals.dword8ED24FollowUpWriteback) {
        state.rightRankDirectFollowUp.ed24 =
            globals.dword8ED24FollowUpWriteback;
    }
    if (globals.gPrStageEventStreamDone != preGlobals.gPrStageEventStreamDone) {
        state.rightRankForcedGoodEventStreamDone28 =
            globals.gPrStageEventStreamDone;
    }
    if (globals.word8ED36Phase1Cache != preGlobals.word8ED36Phase1Cache) {
        state.rightRankPhase1Owner.cachedPhase1Classifier36 =
            globals.word8ED36Phase1Cache;
    }
    if (globals.word8ED38PhaseCounter != preGlobals.word8ED38PhaseCounter) {
        state.rightRankPhase1Owner.phase1LatchArmed38 =
            globals.word8ED38PhaseCounter != 0u;
    }

    if (ctx.word50DescriptorSubstate != preCtx.word50DescriptorSubstate) {
        state.descriptorSubstate50 = ctx.word50DescriptorSubstate;
    }
    state.scoreDisplayValue = ctx.dword30ScoreDisplayValue;
    if (ctx.word5ABlinkEnabled != preCtx.word5ABlinkEnabled ||
        ctx.word58BlinkTargetRow != preCtx.word58BlinkTargetRow) {
        state.rightRankState.rightRankBlinkTargetRow =
            static_cast<uint8_t>(ctx.word58BlinkTargetRow & 0x00FFu);
        state.rightRankState.rightRankBlinkEnabled =
            ctx.word5ABlinkEnabled != 0u;
    }
    if (ctx.word54Permit != preCtx.word54Permit) {
        state.rightRankBucketContext.ctx54Permit = ctx.word54Permit != 0u;
    }
    if (ctx.word72FollowUpCounter != preCtx.word72FollowUpCounter) {
        state.rightRankDirectFollowUp.ctx72 = ctx.word72FollowUpCounter;
    }
    if (ctx.word74HelperCounter != preCtx.word74HelperCounter) {
        state.rightRankBucketContext.ctx74 = ctx.word74HelperCounter;
    }
    if (ctx.word7AFollowUpState != preCtx.word7AFollowUpState) {
        state.rightRankBucketContext.ctx7A = ctx.word7AFollowUpState;
    }
    if (ctx.word18ETransitionAnim != preCtx.word18ETransitionAnim) {
        state.rightRankBucketContext.transitionAnim18E =
            ctx.word18ETransitionAnim;
    }
}

PrStage1ScorerDirectBucketContext BuildStage1DirectPortBucketContextAdapter(
    const Stage1NumericRuntimeState& state,
    int tick96) {
    PrStage1ScorerDirectBucketContext out{};
    const Stage1NumericRuntimeState::DescriptorCadenceRuntime& cadence =
        state.descriptorCadence;
    out.dword0CTick96 = (uint32_t)(std::max)(tick96, 0);
    out.word54Permit = state.rightRankBucketContext.ctx54Permit ? 1u : 0u;
    out.word4ERightRankActiveRow =
        (uint16_t)(std::min<uint8_t>)(state.rightRankState.rightRankActiveRow, 3u);
    out.word50DescriptorSubstate = state.descriptorSubstate50;
    out.word56WritePageOrdinal1Based =
        (cadence.pageOrdinal56Available && cadence.pageOrdinal56 > 0u)
            ? static_cast<uint16_t>(
                  (std::min)(cadence.pageOrdinal56, (size_t)0xFFFFu))
            : 0u;
    out.word6ABucket0ConsumerGate =
        state.rightRankBucketContext.ctx6AConsumerGate ? 1u : 0u;
    out.word72FollowUpCounter = state.rightRankDirectFollowUp.ctx72;
    out.word74HelperCounter = state.rightRankBucketContext.ctx74;
    out.word7AFollowUpState = state.rightRankBucketContext.ctx7A;
    out.word18ETransitionAnim = state.rightRankBucketContext.transitionAnim18E;
    out.dword30ScoreDisplayValue = state.scoreDisplayValue;
    out.word58BlinkTargetRow = state.rightRankState.rightRankBlinkTargetRow;
    out.word5ABlinkEnabled = state.rightRankState.rightRankBlinkEnabled ? 1u : 0u;
    out.descriptorPointerAvailable =
        ResolveStage1FormalScorerCommittedDescriptorRow(state) != nullptr;
    return out;
}

void ApplyStage1Bucket0DescriptorCommitSceneProjection(
    Stage1NumericRuntimeState& state,
    const Stage1NumericRuntimeState::DescriptorRowView& committedRow,
    const PrStage1ScorerDirectBucket0DescriptorCommitResult& /*commit*/) {
    Stage1NumericRuntimeState::DescriptorCadenceRuntime& descriptorRuntime =
        state.descriptorCadence;
    descriptorRuntime.currentCommittedAvailable = true;
    descriptorRuntime.currentCommittedRowIndex =
        descriptorRuntime.cadenceCursorOrdinal1Based;
    descriptorRuntime.currentCommittedRow = committedRow;
    descriptorRuntime.currentDescriptor40Available = true;
    descriptorRuntime.currentDescriptor40RowIndex =
        descriptorRuntime.cadenceCursorOrdinal1Based;
    descriptorRuntime.currentDescriptor40Row = committedRow;
    state.currentDescriptorRow = committedRow;
    state.rightRankDescriptorFlagWord08Known = committedRow.available;
    state.rightRankDescriptorFlagWord08 =
        committedRow.available
            ? (state.descriptorSubstate50 != 0u
                   ? committedRow.substate1FlagWord
                   : committedRow.defaultFlagWord)
            : 0u;

    const uint8_t previousLessonId =
        state.topLessonPairState.topLessonPairLessonId;
    const uint8_t committedLessonId = committedRow.lessonId;
    if (committedLessonId == 0u) {
        state.topLessonPairState.topLessonPairChangeVisible = false;
        return;
    }
    if (committedLessonId == previousLessonId) {
        state.topLessonPairState.topLessonPairChangeVisible = false;
        return;
    }

    state.topLessonPairState.topLessonPairLessonId = committedLessonId;
    if (state.rightRankState.rightRankActiveRow != 0u) {
        state.topLessonPairState.topLessonPairChangeVisible = true;
    }
    // When the lesson commits while the active row is COOL(0), PSX still
    // commits the selector but does not actively clear the visible gate here.
}

void AdvanceStage1DescriptorRuntimeStateForBucket0(const PrGameContext& ctx,
                                                   Stage1NumericRuntimeState& state,
                                                   int tick96) {
    RefreshStage1DescriptorDrivenRuntimeStateForTick(ctx, state, tick96);

    Stage1NumericRuntimeState::DescriptorCadenceRuntime& descriptorRuntime =
        state.descriptorCadence;
    if (!descriptorRuntime.cadenceCursorAvailable) {
        return;
    }

    ApplyStage1Bucket0DescriptorCommitSceneProjection(
        state,
        descriptorRuntime.cadenceCursorRow,
        PrStage1ScorerDirectBucket0DescriptorCommitResult{});
}

void ProcessStage1Bucket0OwnerWindow(const PrGameContext& ctx,
                                     Stage1NumericRuntimeState& state,
                                     int tick96) {
    AdvanceStage1DescriptorRuntimeStateForBucket0(ctx, state, tick96);
    PrStage1ScorerDirectGlobals directGlobals =
        BuildStage1DirectPortGlobalsAdapter(state);
    PrStage1ScorerDirectBucketContext directBucketCtx =
        BuildStage1DirectPortBucketContextAdapter(state, tick96);
    const uint16_t descriptorFlagWord =
        static_cast<uint16_t>(directGlobals.dword8ED08DescriptorFlags);
    const PrStage1ScorerDirectBucket0WindowSliceResult directWindow =
        PrStage1ScorerDirectRunBucket0WindowSlice24FD0(
            directGlobals,
            directBucketCtx,
            descriptorFlagWord);
    state.rightRankDirectFollowUp.ed14 =
        directGlobals.dword8ED14FollowUpActive;
    state.rightRankHelperShadow.tieCarryLatch1E =
        directGlobals.word9181ETwitterCarryLatch;
    state.rightRankHelperShadow.noInputCounter20 =
        directGlobals.word91820NoInputCounter;
    state.rightRankBucketContext.ctx54Permit =
        directBucketCtx.word54Permit != 0u;
    ApplyStage1Bucket0WindowSceneProjection(
        state,
        directWindow,
        descriptorFlagWord);
}

void ProcessStage1Bucket30OwnerWindow(Stage1NumericRuntimeState& state,
                                      int tick96) {
    const int preBucket30Ed00 = state.rightRankDirectFollowUp.ed00;
    const bool hadBucket30DeferredClear =
        state.acceptedProducerNarrowClearPending &&
        state.acceptedProducerNarrowClearPhase ==
            Stage1NumericRuntimeState::AcceptedProducerNarrowClearPhase::
                Bucket30;
    PrStage1ScorerDirectDescriptorRow directCommittedRow{};
    (void)TryBuildStage1DirectPortCommittedDescriptorRowAdapter(
        state,
        directCommittedRow);
    const uint8_t activeRow =
        (std::min<uint8_t>)(state.rightRankState.rightRankActiveRow, 3u);
    PrStage1ScorerDirectGlobals directGlobals =
        BuildStage1DirectPortGlobalsAdapter(state);
    PrStage1ScorerDirectBucketContext directBucketCtx =
        BuildStage1DirectPortBucketContextAdapter(state, tick96);
    const PrStage1ScorerDirectGlobals preDirectGlobals = directGlobals;
    const PrStage1ScorerDirectBucketContext preDirectBucketCtx =
        directBucketCtx;
    // Match the old row-owner wrapper entry semantics: every bucket30 owner
    // beat clears the previous post-24F8C resolution/commit mirror before the
    // current row-kernel decides whether it will publish a new package.
    ClearStage1Bucket30ResolutionMirror(state);
    const PrStage1ScorerDirectBucket30OwnerSliceInput directOwnerInput{
        preBucket30Ed00,
        hadBucket30DeferredClear,
        state.rightRankFollowUpPhase ==
            Stage1NumericRuntimeState::RightRankFollowUpPhase::None,
        state.word9182AEnabled,
        tick96,
        ResolveStage1GoodToCoolDelayTick96(
            static_cast<uint16_t>(
                state.rightRankDirectFollowUp.ctx72 + 1u)),
    };
    const PrStage1ScorerDirectBucket30OwnerSliceResult directOwnerSlice =
        PrStage1ScorerDirectRunBucket30OwnerSlice24FD0(
            directGlobals,
            directBucketCtx,
            directCommittedRow,
            directOwnerInput);

    StoreStage1DirectPortBucket30OwnerGlobals24FD0(
        state,
        preDirectGlobals,
        directGlobals,
        preDirectBucketCtx,
        directBucketCtx);
    auto& ownerObserver = state.bucket30OwnerObserver;
    ownerObserver = {};
    ownerObserver.busyGateActive = directOwnerSlice.busyGateActive;
    ownerObserver.scorerCommitWindowOpen = directOwnerSlice.scorerWindowOpen;
    ownerObserver.kernelOpen = directOwnerSlice.ownerKernelOpen;
    ownerObserver.kernelEntered = directOwnerSlice.ownerKernelOpen;
    ownerObserver.processDescriptorFlagWord = directOwnerSlice.descriptorFlagWord;
    ownerObserver.descriptorFlagWord = directOwnerSlice.descriptorFlagWord;
    ownerObserver.activeRow = activeRow;
    ownerObserver.prePhase1Classifier36 =
        static_cast<uint8_t>(preDirectGlobals.word8ED36Phase1Cache & 0x00FFu);
    ownerObserver.prePhase1LatchArmed38 =
        preDirectGlobals.word8ED38PhaseCounter != 0u;
    ownerObserver.phase1AdvanceCalled = directOwnerSlice.ownerKernelOpen;
    ownerObserver.phase1AdvanceSampledClassifier =
        directOwnerSlice.phase1.sampledClassifier;
    ownerObserver.phase1AdvanceFirstBeat = directOwnerSlice.phase1.firstBeat;
    ownerObserver.phase1PrevBaseline18 =
        preDirectGlobals.word91818SharedBaseline;
    ownerObserver.phase1LiveAccumulator91816 =
        directGlobals.word91816Accumulator;
    ownerObserver.phase1Delta91816MinusPrev18 =
        directGlobals.word91816Accumulator -
        preDirectGlobals.word91818SharedBaseline;
    ownerObserver.producerGrowthBaseline18 =
        preDirectGlobals.word91818SharedBaseline;
    ownerObserver.blinkBaseline18 = preDirectGlobals.word9181ASnapshot;
    ownerObserver.helperSnapshot18 = preDirectGlobals.word9181CCompareBaseline;
    ownerObserver.afterAdvancePhase1Classifier36 =
        directOwnerSlice.resolution.resolutionCalled
            ? directOwnerSlice.resolution.resolutionInputPhase1Classifier36
            : static_cast<uint8_t>(
                  directGlobals.word8ED36Phase1Cache & 0x00FFu);
    ownerObserver.afterAdvancePhase1LatchArmed38 =
        directGlobals.word8ED38PhaseCounter != 0u;
    ownerObserver.tieBreakerCalled =
        directOwnerSlice.resolution.tieBreakerCalled;
    ownerObserver.tieBreakerResult14548 =
        directOwnerSlice.resolution.tieBreakerResult14548;
    ownerObserver.resolverGateBit4 =
        (directOwnerSlice.descriptorFlagWord & 0x0004u) != 0u;
    ownerObserver.resolverGateEd00Idle = preBucket30Ed00 == 0;
    ownerObserver.resolutionCalled =
        directOwnerSlice.resolution.resolutionCalled;
    ownerObserver.resolutionKnown =
        directOwnerSlice.resolution.resolutionCalled;
    ownerObserver.resolutionInputPhase1Classifier36 =
        directOwnerSlice.resolution.resolutionInputPhase1Classifier36;
    ownerObserver.resolutionV22 = directOwnerSlice.resolution.resolutionV22;
    ownerObserver.goodToCoolCommitted =
        directOwnerSlice.rowWrite.rowWrite.goodToCoolCommitted;
    ownerObserver.afterProducePhase1Classifier36 =
        static_cast<uint8_t>(directGlobals.word8ED36Phase1Cache & 0x00FFu);
    ownerObserver.afterProducePhase1LatchArmed38 =
        directGlobals.word8ED38PhaseCounter != 0u;
    ownerObserver.afterProduceGrowthBaseline18 =
        directGlobals.word91818SharedBaseline;
    ownerObserver.afterProduceBlinkBaseline18 =
        directGlobals.word9181ASnapshot;
    ownerObserver.afterProduceHelperSnapshot18 =
        directGlobals.word9181CCompareBaseline;
    Stage1FormalScorerCommitRuntimeState gameplayCommitState =
        ConvertStage1DirectPortCommitStateAdapter(
            directOwnerSlice.commitSlice.commit);
    ownerObserver.gameplayCurrentBranchKnown =
        gameplayCommitState.currentBranchKnown;
    ownerObserver.gameplayCurrentBranchSpecial =
        gameplayCommitState.currentBranchSpecial;
    ownerObserver.gameplayAcceptedCountKnown =
        gameplayCommitState.acceptedCountKnown;
    ownerObserver.gameplayAcceptedCount =
        gameplayCommitState.acceptedCountValue;
    ownerObserver.gameplayLookbackPageCountKnown =
        gameplayCommitState.currentLookbackPageCountKnown;
    ownerObserver.gameplayLookbackPageCount =
        gameplayCommitState.currentLookbackPageCountValue;
    ownerObserver.gameplayOverflowActiveKnown =
        gameplayCommitState.overflowActiveKnown;
    ownerObserver.gameplayOverflowActive =
        gameplayCommitState.overflowActive;
    ownerObserver.gameplayBranchCountKnown =
        gameplayCommitState.activeBranch.countKnown;
    ownerObserver.gameplayBranchCount =
        gameplayCommitState.activeBranch.countValue;
    ownerObserver.gameplayDescriptorSubdeltaKnown =
        gameplayCommitState.activeBranch.descriptorSubdeltaKnown;
    ownerObserver.gameplayDescriptorSubdelta =
        gameplayCommitState.activeBranch.descriptorSubdeltaValue;
    ownerObserver.gameplayReaderPageOrdinalKnown =
        gameplayCommitState.activeBranch.readerPageOrdinalKnown;
    ownerObserver.gameplayReaderPageOrdinal =
        gameplayCommitState.activeBranch.readerPageOrdinalValue;
    ownerObserver.gameplayRequiredMaskKnown =
        gameplayCommitState.activeBranch.requiredMaskKnown;
    ownerObserver.gameplayRequiredMask =
        gameplayCommitState.activeBranch.requiredMaskValue;
    ownerObserver.gameplayUnionMaskKnown =
        gameplayCommitState.activeBranch.unionMaskKnown;
    ownerObserver.gameplayUnionMask =
        gameplayCommitState.activeBranch.unionMaskValue;
    ownerObserver.gameplayAnchorSlotIndexKnown =
        gameplayCommitState.activeBranch.anchorSlotIndexKnown;
    ownerObserver.gameplayAnchorSlotIndex =
        gameplayCommitState.activeBranch.anchorSlotIndexValue;
    ownerObserver.gameplayRequiredClassTokenKnown =
        gameplayCommitState.activeBranch.requiredClassTokenKnown;
    ownerObserver.gameplayRequiredClassToken =
        gameplayCommitState.activeBranch.requiredClassTokenValue;
    ownerObserver.gameplayAnchorSlotClassTokenKnown =
        gameplayCommitState.activeBranch.anchorSlotClassTokenKnown;
    ownerObserver.gameplayAnchorSlotClassToken =
        gameplayCommitState.activeBranch.anchorSlotClassTokenValue;
    ownerObserver.gameplayAnchorSlotOccupiedKnown =
        gameplayCommitState.activeBranch.anchorSlotOccupiedKnown;
    ownerObserver.gameplayAnchorSlotOccupied =
        gameplayCommitState.activeBranch.anchorSlotOccupiedValue;
    ownerObserver.gameplayAnchorClassMatchKnown =
        gameplayCommitState.activeBranch.anchorClassMatchKnown;
    ownerObserver.gameplayAnchorClassMatch =
        gameplayCommitState.activeBranch.anchorClassMatchValue;
    ownerObserver.gameplayPairBonusKnown =
        gameplayCommitState.activeBranch.pairBonusKnown;
    ownerObserver.gameplayPairBonus =
        gameplayCommitState.activeBranch.pairBonusValue;
    ownerObserver.gameplaySpillPenaltyKnown =
        gameplayCommitState.activeBranch.spillPenaltyKnown;
    ownerObserver.gameplaySpillPenalty =
        gameplayCommitState.activeBranch.spillPenaltyValue;
    ownerObserver.gameplayAdditiveTermKnown =
        gameplayCommitState.additiveTermKnown;
    ownerObserver.gameplayAdditiveTerm =
        gameplayCommitState.additiveTermValue;
    ownerObserver.gameplayCommitTermKnown =
        gameplayCommitState.commitTermKnown;
    ownerObserver.gameplayCommitTerm =
        gameplayCommitState.commitTermValue;
    ownerObserver.gameplayScoreWritebackKnown =
        gameplayCommitState.scoreWritebackKnown;
    ownerObserver.gameplayScoreWriteback =
        gameplayCommitState.scoreWritebackValue;
    ownerObserver.gameplayClampActiveKnown =
        gameplayCommitState.clampActiveKnown;
    ownerObserver.gameplayClampActive =
        gameplayCommitState.clampActive;
    const bool bucket30BusyGateActive = directOwnerSlice.busyGateActive;
    // Mirror the top-level PSX bucket30 owner ordering under `sub_80024FD0`:
    // 24BF4 busy gate -> (ED08&3 ? 14D58/14458 : skip) ->
    // (ED08&2 ? row owner kernel : skip) -> fixed 24FC0 tail publish.
    const bool scorerCommitWindowOpen = directOwnerSlice.scorerWindowOpen;
    const bool bucket30OwnerKernelOpen = directOwnerSlice.ownerKernelOpen;
    if (!bucket30BusyGateActive &&
        !bucket30OwnerKernelOpen &&
        state.rightRankFollowUpPhase ==
            Stage1NumericRuntimeState::RightRankFollowUpPhase::
                AwaitForceGoodWriteback) {
        state.rightRankFollowUpPhase =
            Stage1NumericRuntimeState::RightRankFollowUpPhase::None;
    }
    if (!scorerCommitWindowOpen) {
        gameplayCommitState.scoreWritebackKnown = false;
        gameplayCommitState.clampActiveKnown = false;
        gameplayCommitState.clampActive = false;
    } else {
        if (gameplayCommitState.scoreWritebackKnown) {
            AdvanceStage1FormulaAccumulatorBaseline(
                state,
                gameplayCommitState.scoreWritebackValue);
        }
    }
    bool directConsumerSlotKnown = false;
    uint8_t directConsumerSlot = 0u;
    if (bucket30OwnerKernelOpen) {
        if (activeRow == 0u) {
            ApplyStage1Bucket30Phase1SceneProjection(
                state,
                activeRow,
                directGlobals,
                directOwnerSlice.phase1,
                directConsumerSlotKnown,
                directConsumerSlot);
        } else {
            ApplyStage1Bucket30Phase1SceneProjection(
                state,
                activeRow,
                directGlobals,
                directOwnerSlice.phase1,
                directConsumerSlotKnown,
                directConsumerSlot);
        }

        if (gameplayCommitState.commitTermKnown) {
            const int32_t rightRankFormalWritebackValue =
                ResolveStage1FormulaAccumulatorWriteback(
                    state.scorerPort.accumulator91816,
                    gameplayCommitState.commitTermValue)
                    .writebackValue;
            state.rightRank24F8CObserver.formalWritebackValue24 =
                rightRankFormalWritebackValue;
            ownerObserver.formalWritebackKnown = true;
            ownerObserver.formalWritebackValue = rightRankFormalWritebackValue;
            if (activeRow == 0u) {
                state.rightRank24F8CObserver.lastBaselineReseedQueryFrame =
                    (int32_t)state.queryFrame;
            }
            if (directOwnerSlice.resolution.resolutionCalled) {
                ApplyStage1RightRank24F8CObserverProjection(state);
            }
        }

        ApplyStage1Bucket30RowWriteSceneProjection(
            state,
            directOwnerSlice.resolvedPublish,
            directOwnerSlice.rowWrite);
    }
    ApplyStage1Bucket30SteadySfxSceneProjection(
        state,
        directOwnerSlice.steadySfx);
    ApplyStage1Bucket30DirectClearSceneProjection(
        state,
        directOwnerSlice.clearSlice);
    ApplyStage1Bucket30DelayedTailSceneProjection(
        state,
        directOwnerSlice.delayedTail);
    CaptureStage1AuthoritativeAdditiveLaneBucket30Commit(state);
    // `sub_80024FC0` always runs at bucket30 tail, even when the inner scorer
    // or row-write owner kernels were skipped by the busy gate/descriptor bits.
    state.scoreDisplayFormulaBacked = true;
}

void ApplyStage1Bucket31DirectConsumerPackage(
    Stage1NumericRuntimeState& state,
    int preBucket31Ed00,
    bool bucket31NarrowClearPending,
    int tick96,
    bool runPageClear14BDC,
    PrStage1ScorerDirectBucket31DispatcherResult* outDirectBucket31Result) {
    PrStage1ScorerDirectGlobals directGlobals =
        BuildStage1DirectPortGlobalsAdapter(state);
    PrStage1ScorerDirectBucketContext directBucketCtx =
        BuildStage1DirectPortBucketContextAdapter(state, tick96);
    const PrStage1ScorerDirectBucket31DispatcherInput directBucket31Input{
        preBucket31Ed00,
        state.rightRankFollowUpPhase ==
            Stage1NumericRuntimeState::RightRankFollowUpPhase::
                AwaitBucket31AfterGoodToCool,
        bucket31NarrowClearPending,
        state.additiveLane.clearPendingBucket31,
        runPageClear14BDC,
    };
    const PrStage1ScorerDirectBucket31DispatcherResult directBucket31Result =
        PrStage1ScorerDirectRunBucket31Dispatcher24FD0(
            directGlobals,
            directBucketCtx,
            directBucket31Input);
    Stage1NumericRuntimeState::PageOwnerRuntime& pageOwner = state.pageOwner;
    pageOwner.literalBucket31PageClear14BDCWindowAdvanced = true;
    pageOwner.literalBucket31PageClear14BDCRequested =
        directBucket31Result.pageClear.requested;
    pageOwner.literalBucket31PageClear14BDCTargetKnown =
        directBucket31Result.pageClear.targetKnown;
    pageOwner.literalBucket31PageClear14BDCTargetOrdinal1Based =
        directBucket31Result.pageClear.targetOrdinal1Based;
    pageOwner.literalBucket31PageClear14BDCClearApplied =
        directBucket31Result.pageClear.clearApplied;
    if (outDirectBucket31Result) {
        *outDirectBucket31Result = directBucket31Result;
    }
    const PrStage1ScorerDirectBucket31ConsumerSliceResult& directConsumerSlice =
        directBucket31Result.consumer;
    StoreStage1DirectPortBucket31ConsumerGlobals24FD0(
        state,
        directGlobals,
        directBucketCtx,
        directConsumerSlice);
    ApplyStage1DirectPortFollowUpPhaseAction(
        state,
        directConsumerSlice.followUpPhaseAction);

    if (directConsumerSlice.consumerPackageRan &&
        directConsumerSlice.playCompletionCue) {
        Stage1SteadySfxDispatchRuntime dispatch{};
        dispatch.known = true;
        dispatch.cueId = Stage1SteadySfxDispatchCueId::Bucket31Completion;
        dispatch.tableSlot = 0u;
        dispatch.firedBucket31Completion = true;
        state.steadySfx = dispatch;
        PrSfx::PlayStage1SteadyDelayedCompletionCue();
    }
    ApplyStage1Bucket31DirectSceneProjection(state, directBucket31Result);
}

void ProcessStage1Bucket31OwnerWindow(
    Stage1NumericRuntimeState& state,
    PrStage1ScorerDirectBucket31DispatcherResult* outDirectBucket31Result,
    int tick96,
    bool runPageClear14BDC) {
    const int preBucket31Ed00 = state.rightRankDirectFollowUp.ed00;
    const bool bucket31NarrowClearPending =
        state.acceptedProducerNarrowClearPending &&
        state.acceptedProducerNarrowClearPhase ==
            Stage1NumericRuntimeState::AcceptedProducerNarrowClearPhase::
                Bucket31;

    ApplyStage1Bucket31DirectConsumerPackage(
        state,
        preBucket31Ed00,
        bucket31NarrowClearPending,
        tick96,
        runPageClear14BDC,
        outDirectBucket31Result);
}

PrStage1ScorerDirectPageClear14BDCResult
ProcessStage1LiteralPageClear14BDCWindow(
    Stage1NumericRuntimeState& state,
    int tick96) {
    PrStage1ScorerDirectGlobals directGlobals =
        BuildStage1DirectPortGlobalsAdapter(state);
    PrStage1ScorerDirectBucketContext directBucketCtx =
        BuildStage1DirectPortBucketContextAdapter(state, tick96);
    const PrStage1ScorerDirectPageClear14BDCResult pageClear =
        PrStage1ScorerDirectRunPageClearCore14BDC(
            directGlobals,
            directBucketCtx.word56WritePageOrdinal1Based,
            true);

    Stage1NumericRuntimeState::PageOwnerRuntime& pageOwner = state.pageOwner;
    pageOwner.literalBucket31PageClear14BDCWindowAdvanced = true;
    pageOwner.literalBucket31PageClear14BDCRequested = pageClear.requested;
    pageOwner.literalBucket31PageClear14BDCTargetKnown =
        pageClear.targetKnown;
    pageOwner.literalBucket31PageClear14BDCTargetOrdinal1Based =
        pageClear.targetOrdinal1Based;
    pageOwner.literalBucket31PageClear14BDCClearApplied =
        pageClear.clearApplied;
    return pageClear;
}


void InitializeStage1DescriptorCadenceRuntime(const PrGameContext& ctx,
                                              Stage1NumericRuntimeState& state,
                                              int tick96) {
    Stage1NumericRuntimeState::DescriptorCadenceRuntime& runtime =
        state.descriptorCadence;
    runtime = Stage1NumericRuntimeState::DescriptorCadenceRuntime{};
    runtime.initialized = true;
    runtime.freshEntryLookaheadSeedActive = true;
    runtime.cadenceCursorOrdinal1Based =
        PrStage1ScorerDirectResolvePageOrdinal56_24FD0(tick96);
    runtime.pageOrdinal56Available = runtime.cadenceCursorOrdinal1Based != 0u;
    runtime.pageOrdinal56 = runtime.cadenceCursorOrdinal1Based;
    runtime.cadenceCursorAvailable =
        TryReadStage1DescriptorRowHost(
            ctx,
            runtime.cadenceCursorOrdinal1Based,
            runtime.cadenceCursorRow) &&
        runtime.cadenceCursorRow.available;
    runtime.nextLookaheadRowIndex = (std::min)(
        kStage1TopLessonPairInitialLookaheadRowIndex,
        kStage1DescriptorMainTableRowCount - 1u);
    runtime.nextLookaheadAvailable =
        TryReadStage1DescriptorRowHost(
            ctx,
            runtime.nextLookaheadRowIndex,
            runtime.nextLookaheadRow) &&
        runtime.nextLookaheadRow.available;
    runtime.lookaheadDescriptor44Available = runtime.nextLookaheadAvailable;
    runtime.lookaheadDescriptor44Row = runtime.nextLookaheadRow;
    state.currentDescriptorRow = Stage1NumericRuntimeState::DescriptorRowView{};
}

void RefreshStage1DescriptorDrivenRuntimeStateForTick(const PrGameContext& ctx,
                                                      Stage1NumericRuntimeState& state,
                                                      int tick96) {
    if (!state.descriptorCadence.initialized) {
        InitializeStage1DescriptorCadenceRuntime(ctx, state, tick96);
        return;
    }

    Stage1NumericRuntimeState::DescriptorCadenceRuntime& cadence =
        state.descriptorCadence;
    cadence.cadenceCursorOrdinal1Based =
        PrStage1ScorerDirectResolvePageOrdinal56_24FD0(tick96);
    cadence.pageOrdinal56Available = cadence.cadenceCursorOrdinal1Based != 0u;
    cadence.pageOrdinal56 = cadence.cadenceCursorOrdinal1Based;
    cadence.cadenceCursorAvailable =
        TryReadStage1DescriptorRowHost(
            ctx,
            cadence.cadenceCursorOrdinal1Based,
            cadence.cadenceCursorRow) &&
        cadence.cadenceCursorRow.available;

    if (cadence.freshEntryLookaheadSeedActive) {
        cadence.freshEntryLookaheadSeedActive = false;
    }

    cadence.nextLookaheadRowIndex =
        PrStage1ScorerDirectResolveLookaheadRowIndex44_24FD0(
            tick96,
            static_cast<uint16_t>(ctx.transitionState));
    cadence.nextLookaheadAvailable =
        TryReadStage1DescriptorRowHost(
            ctx,
            cadence.nextLookaheadRowIndex,
            cadence.nextLookaheadRow) &&
        cadence.nextLookaheadRow.available;
    cadence.lookaheadDescriptor44Available = cadence.nextLookaheadAvailable;
    cadence.lookaheadDescriptor44Row = cadence.nextLookaheadRow;
}

void PrepareStage1PageRecordMirrorForTick(
    Stage1NumericRuntimeState::PageRecordMirrorRuntime& runtime,
    int tick96) {
    PrepareStage1PageRecordMirrorForPageOrdinalHost(
        runtime,
        ResolveStage1PageRecordMirrorPageOrdinalForTick96Host(tick96));
}

bool ApplyStage1PageMaintenanceProjectionFromLiteralPageClear14BDC_14614(
    Stage1NumericRuntimeState& state,
    const PrStage1ScorerDirectPageClear14BDCResult* sameTickPageClear,
    const PrStage1ScorerDirectAcceptedProducerRunResult* directAcceptedResult) {
    if (!sameTickPageClear && !directAcceptedResult) {
        return false;
    }

    const PrStage1ScorerDirectAcceptedProducerRunResult defaultAcceptedResult{};
    const PrStage1ScorerDirectAcceptedProducerRunResult& acceptedResult =
        directAcceptedResult ? *directAcceptedResult : defaultAcceptedResult;
    const PrStage1ScorerDirectFinalPageMaintenance14BDC_14614Result finalResult =
        PrStage1ScorerDirectResolveFinalPageMaintenance14BDC_14614(
            sameTickPageClear,
            acceptedResult);

    Stage1PageMaintenanceProjection14BDC_14614 projection{};
    if (!TryBuildStage1PageMaintenanceProjectionFromFinalResult14BDC_14614Host(
            state,
            finalResult,
            projection)) {
        return false;
    }
    return ApplyStage1PageMaintenanceProjection14BDC_14614Host(
        state,
        projection);
}

uint8_t GetStage1GameplayBucketForTick96(int tick96) {
    if (tick96 < 0) {
        return 0xFF;
    }
    return (uint8_t)(((uint32_t)tick96 % 384u) / 12u);
}

uint16_t ResolveStage1AcceptedProducerObservedRawSample18(
    const PrGameContext& ctx,
    const Stage1NumericRuntimeState::AcceptedProducerCarrierRuntime& carrier,
    uint16_t heldMask) {
    (void)ctx;
    return PrStage1ScorerHostDirectResolveObservedRawSample18(
        carrier.available,
        carrier.rawControlSample18,
        heldMask);
}

uint8_t ResolveStage1AcceptedProducerSubstate50(
    const Stage1NumericRuntimeState& state) {
    return state.descriptorSubstate50;
}

uint16_t ResolveStage1AcceptedProducerControlMask18(uint16_t rawMask) {
    const uint16_t filteredMask = rawMask & kStage1GameplayAcceptedInputMask;
    if (filteredMask == 0u || (filteredMask & (filteredMask - 1u)) != 0u) {
        return rawMask;
    }
    return filteredMask;
}

uint8_t ResolveStage1AcceptedProducerClassToken20(uint16_t controlMask18) {
    const uint8_t a1 = static_cast<uint8_t>(controlMask18 & 0xFFu);
    int result = 1;
    if ((a1 & 0x10u) == 0u) {
        result = 2;
        if ((a1 & 0x20u) == 0u) {
            result = 3;
            if ((a1 & 0x40u) == 0u) {
                result = 4;
                if ((a1 & 0x80u) == 0u) {
                    result = 5;
                    if ((a1 & 0x04u) == 0u) {
                        result = 5;
                        if ((a1 & 0x01u) == 0u) {
                            result = 7;
                            if ((a1 & 0x08u) == 0u) {
                                return static_cast<uint8_t>(
                                    (a1 & 0x02u) != 0u ? 7u : 0u);
                            }
                        }
                    }
                }
            }
        }
    }
    return static_cast<uint8_t>(result);
}

uint8_t ResolveStage1AcceptedProducerHalfWindow34(
    const PrGameContext& ctx,
    const Stage1NumericRuntimeState& state) {
    if (state.runnerPreambleHalfWindow34Known) {
        return state.runnerPreambleHalfWindow34;
    }
    return (ctx.transitionStateDA != 0) ? 24u : 8u;
}

int ResolveStage1AcceptedProducerWritePageOrdinal38(
    const Stage1NumericRuntimeState& state,
    int tick96) {
    return PrStage1ScorerHostDirectResolveWritePageOrdinal38(
        state.pageOwner.currentWritePageOrdinalKnown,
        state.pageOwner.currentWritePageOrdinal1Based,
        tick96);
}

static PrStage1ScorerHostDirectAcceptedReplayBufferState
BuildStage1AcceptedHostDirectReplayBufferAdapter(
    const Stage1NumericRuntimeState::AcceptedProducerReplayBufferRuntime& replay) {
    PrStage1ScorerHostDirectAcceptedReplayBufferState out{};
    out.replayMirrorKnown8008EEF8 = replay.replayMirrorKnown8008EEF8;
    out.replayMirrorProducerKnown8008EEF8 =
        replay.replayMirrorProducerKnown8008EEF8;
    out.replayMirrorProducerFunction = replay.replayMirrorProducerFunction;
    out.replayMirrorByteCountKnown8008EEF8 =
        replay.replayMirrorByteCountKnown8008EEF8;
    out.replayMirrorKnownByteCount8008EEF8 =
        replay.replayMirrorKnownByteCount8008EEF8;
    out.writeCount901C0 = replay.writeCount901C0;
    out.publishedCount901BC = replay.publishedCount901BC;
    for (size_t i = 0; i < kPrStage1ScorerHostDirectReplayBufferCapacity; ++i) {
        out.tick96EEF8[i] = replay.tick96EEF8[i];
        out.classMaskEEFC[i] = replay.classMaskEEFC[i];
    }
    return out;
}

static void StoreStage1AcceptedHostDirectReplayBufferAdapter(
    const PrStage1ScorerHostDirectAcceptedReplayBufferState& directReplay,
    Stage1NumericRuntimeState::AcceptedProducerReplayBufferRuntime& replay) {
    replay.replayMirrorKnown8008EEF8 = directReplay.replayMirrorKnown8008EEF8;
    replay.replayMirrorProducerKnown8008EEF8 =
        directReplay.replayMirrorProducerKnown8008EEF8;
    replay.replayMirrorProducerFunction =
        directReplay.replayMirrorProducerFunction;
    replay.replayMirrorByteCountKnown8008EEF8 =
        directReplay.replayMirrorByteCountKnown8008EEF8;
    replay.replayMirrorKnownByteCount8008EEF8 =
        directReplay.replayMirrorKnownByteCount8008EEF8;
    replay.writeCount901C0 = directReplay.writeCount901C0;
    replay.publishedCount901BC = directReplay.publishedCount901BC;
    for (size_t i = 0; i < kPrStage1ScorerHostDirectReplayBufferCapacity; ++i) {
        replay.tick96EEF8[i] = directReplay.tick96EEF8[i];
        replay.classMaskEEFC[i] = directReplay.classMaskEEFC[i];
    }
}

static PrStage1ScorerHostDirectAcceptedScriptedWriterState
BuildStage1AcceptedHostDirectScriptedWriterAdapter(
    const Stage1AcceptedProducerScriptedWriterRuntime& runtime) {
    PrStage1ScorerHostDirectAcceptedScriptedWriterState out{};
    out.active = runtime.active;
    out.dueTick96EEF8 = runtime.dueTick96EEF8;
    out.classMaskEEFC = runtime.classMaskEEFC;
    return out;
}

static void StoreStage1AcceptedHostDirectScriptedWriterAdapter(
    const PrStage1ScorerHostDirectAcceptedScriptedWriterState& directRuntime,
    Stage1AcceptedProducerScriptedWriterRuntime& runtime) {
    runtime.active = directRuntime.active;
    runtime.dueTick96EEF8 = directRuntime.dueTick96EEF8;
    runtime.classMaskEEFC = directRuntime.classMaskEEFC;
}

static PrStage1ScorerHostDirectAcceptedCarrierCore
BuildStage1AcceptedHostDirectCarrierCoreAdapter(
    const Stage1NumericRuntimeState::AcceptedProducerCarrierRuntime& carrier) {
    PrStage1ScorerHostDirectAcceptedCarrierCore out{};
    out.available = carrier.available;
    out.controlWriterSourceKind = carrier.controlWriterSourceKind;
    out.rawControlSample18 = carrier.rawControlSample18;
    out.controlMask18 = carrier.controlMask18;
    out.classToken20 = carrier.classToken20;
    out.halfWindow34 = carrier.halfWindow34;
    out.substate50 = carrier.substate50;
    out.acceptedTick96Known = carrier.acceptedTick96Known;
    out.acceptedTick96 = carrier.acceptedTick96;
    out.acceptedTick96LastUpdateQueryFrame =
        carrier.acceptedTick96LastUpdateQueryFrame;
    out.acceptedTick96LastUpdateSourceKind =
        carrier.acceptedTick96LastUpdateSourceKind;
    out.acceptedTick96LastUpdateControlMask18 =
        carrier.acceptedTick96LastUpdateControlMask18;
    out.acceptedTick96LastUpdateClassToken20 =
        carrier.acceptedTick96LastUpdateClassToken20;
    out.acceptedTick96LastUpdateViaPostCtx10 =
        carrier.acceptedTick96LastUpdateViaPostCtx10;
    out.acceptedTick96LastUpdateCtxInput18 =
        carrier.acceptedTick96LastUpdateCtxInput18;
    out.acceptedTick96LastUpdatePreviousInputMask801CCBB8 =
        carrier.acceptedTick96LastUpdatePreviousInputMask801CCBB8;
    out.acceptedTick96LastUpdateAcceptedMaskChanged =
        carrier.acceptedTick96LastUpdateAcceptedMaskChanged;
    out.phase384 = carrier.phase384;
    out.writePageOrdinal38 = carrier.writePageOrdinal38;
    out.recordSlot24 = carrier.recordSlot24;
    out.recordRemainder24 = carrier.recordRemainder24;
    out.timingTemplateSlot48 = carrier.timingTemplateSlot48;
    out.recordedSplit = carrier.recordedSplit;
    return out;
}

static void StoreStage1AcceptedHostDirectCarrierCoreAdapter(
    const PrStage1ScorerHostDirectAcceptedCarrierCore& directCarrier,
    Stage1NumericRuntimeState::AcceptedProducerCarrierRuntime& carrier) {
    carrier.available = directCarrier.available;
    carrier.controlWriterSourceKind = directCarrier.controlWriterSourceKind;
    carrier.rawControlSample18 = directCarrier.rawControlSample18;
    carrier.controlMask18 = directCarrier.controlMask18;
    carrier.classToken20 = directCarrier.classToken20;
    carrier.halfWindow34 = directCarrier.halfWindow34;
    carrier.substate50 = directCarrier.substate50;
    carrier.acceptedTick96Known = directCarrier.acceptedTick96Known;
    carrier.acceptedTick96 = directCarrier.acceptedTick96;
    carrier.acceptedTick96LastUpdateQueryFrame =
        directCarrier.acceptedTick96LastUpdateQueryFrame;
    carrier.acceptedTick96LastUpdateSourceKind =
        directCarrier.acceptedTick96LastUpdateSourceKind;
    carrier.acceptedTick96LastUpdateControlMask18 =
        directCarrier.acceptedTick96LastUpdateControlMask18;
    carrier.acceptedTick96LastUpdateClassToken20 =
        directCarrier.acceptedTick96LastUpdateClassToken20;
    carrier.acceptedTick96LastUpdateViaPostCtx10 =
        directCarrier.acceptedTick96LastUpdateViaPostCtx10;
    carrier.acceptedTick96LastUpdateCtxInput18 =
        directCarrier.acceptedTick96LastUpdateCtxInput18;
    carrier.acceptedTick96LastUpdatePreviousInputMask801CCBB8 =
        directCarrier.acceptedTick96LastUpdatePreviousInputMask801CCBB8;
    carrier.acceptedTick96LastUpdateAcceptedMaskChanged =
        directCarrier.acceptedTick96LastUpdateAcceptedMaskChanged;
    carrier.phase384 = directCarrier.phase384;
    carrier.writePageOrdinal38 = directCarrier.writePageOrdinal38;
    carrier.recordSlot24 = directCarrier.recordSlot24;
    carrier.recordRemainder24 = directCarrier.recordRemainder24;
    carrier.timingTemplateSlot48 = directCarrier.timingTemplateSlot48;
    carrier.recordedSplit = directCarrier.recordedSplit;
}

void UpdateStage1AcceptedProducerGateCarrierRuntime(
    const PrGameContext& ctx,
    Stage1NumericRuntimeState& state,
    int tick96) {
    Stage1NumericRuntimeState::AcceptedProducerCarrierRuntime& carrier =
        state.acceptedProducerCarrier;
    carrier.eventStreamFlagKnown = false;
    carrier.eventStreamFlagActive = false;
    carrier.eventStreamIdRawKnown = false;
    carrier.eventStreamIdRaw = 0u;
    carrier.busyGate24BF4Known = false;
    carrier.busyGate24BF4Active = false;
    carrier.acceptedGateKnown = false;
    carrier.acceptedGateActive = false;

    if (state.psxEventStreamFlagKnown) {
        carrier.eventStreamFlagKnown = true;
        carrier.eventStreamFlagActive = state.psxEventStreamFlagActive;
    } else if (ctx.stage1OverlayData && ctx.stage1OverlayData->valid) {
        if (const PrStage1EventStream* stream1 =
                ctx.stage1OverlayData->FindStream(1u)) {
            for (auto it = stream1->events.rbegin();
                 it != stream1->events.rend();
                 ++it) {
                if ((it->flags04 & 0x00000080u) == 0u) {
                    continue;
                }
                carrier.eventStreamFlagKnown = true;
                carrier.eventStreamFlagActive =
                    tick96 >= 0 ? ((uint32_t)tick96 < it->frame) : true;
                break;
            }
        }
    }

    if (state.psxEventStreamIdKnown) {
        carrier.eventStreamIdRawKnown = true;
        carrier.eventStreamIdRaw = state.psxEventStreamId;
        carrier.busyGate24BF4Known = true;
        if (state.rightRankDirectFollowUp.ed00 == 1) {
            carrier.busyGate24BF4Active =
                state.rightRankDirectFollowUp.ed14 != 0 &&
                tick96 <= state.rightRankDirectFollowUp.ed0c;
        } else if (carrier.eventStreamIdRaw != 0u) {
            carrier.busyGate24BF4Active = true;
        } else {
            carrier.busyGate24BF4Active =
                state.rightRankDirectFollowUp.ed14 != 0 &&
                state.rightRankDirectFollowUp.ed00 == 5;
        }
    }

    if (carrier.busyGate24BF4Known && carrier.eventStreamFlagKnown) {
        carrier.acceptedGateKnown = true;
        carrier.acceptedGateActive =
            !carrier.busyGate24BF4Active && carrier.eventStreamFlagActive;
    }
}

void ApplyStage1AcceptedProducerCurrentTickLatchFromPostFrame(
    Stage1NumericRuntimeState::AcceptedProducerCarrierRuntime& carrier,
    const PrStageRunnerDirectPostFrame7A60Result& postFrame,
    int32_t tick96,
    uint32_t queryFrame30,
    uint32_t ctxInput18,
    uint32_t previousInputMask801CCBB8) {
    if (!postFrame.backupCtx10FromCtx0C || tick96 < 0) {
        return;
    }

    carrier.acceptedTick96Known = true;
    carrier.acceptedTick96 = tick96;
    carrier.acceptedTick96LastUpdateQueryFrame = queryFrame30;
    carrier.acceptedTick96LastUpdateSourceKind = carrier.controlWriterSourceKind;
    carrier.acceptedTick96LastUpdateControlMask18 = carrier.controlMask18;
    carrier.acceptedTick96LastUpdateClassToken20 = carrier.classToken20;
    carrier.acceptedTick96LastUpdateViaPostCtx10 = true;
    carrier.acceptedTick96LastUpdateCtxInput18 = ctxInput18;
    carrier.acceptedTick96LastUpdatePreviousInputMask801CCBB8 =
        previousInputMask801CCBB8;
    carrier.acceptedTick96LastUpdateAcceptedMaskChanged =
        postFrame.acceptedMaskChanged;

    const uint16_t phase384 = static_cast<uint16_t>(
        (static_cast<uint32_t>(tick96) +
         static_cast<uint32_t>(carrier.halfWindow34)) %
        384u);
    carrier.phase384 = phase384;
    carrier.recordSlot24 = static_cast<uint8_t>(phase384 / 24u);
    carrier.recordRemainder24 = static_cast<uint8_t>(phase384 % 24u);
    carrier.timingTemplateSlot48 = static_cast<uint8_t>(phase384 / 48u);
    carrier.recordedSplit =
        carrier.recordRemainder24 <=
        static_cast<uint8_t>(2u * static_cast<uint32_t>(carrier.halfWindow34));
}

void UpdateStage1AcceptedProducerCarrierRuntime(const PrGameContext& ctx,
                                                Stage1NumericRuntimeState& state,
                                                const PrStage1ScorerHostDirectSteadyInput7A60Result&
                                                    steadyInput,
                                                uint16_t rawControlMask18,
                                                int tick96,
                                                uint32_t queryFrame30) {
    Stage1NumericRuntimeState::AcceptedProducerCarrierRuntime& carrier =
        state.acceptedProducerCarrier;
    PrStage1ScorerHostDirectAcceptedReplayBufferState directReplay =
        BuildStage1AcceptedHostDirectReplayBufferAdapter(
            state.acceptedProducerReplayBuffer);
    PrStage1ScorerHostDirectAcceptedScriptedWriterState directScriptedWriter =
        BuildStage1AcceptedHostDirectScriptedWriterAdapter(
            state.acceptedProducerScriptedWriter);
    PrStage1ScorerHostDirectAcceptedCarrierCore directCarrier =
        BuildStage1AcceptedHostDirectCarrierCoreAdapter(carrier);
    const uint16_t writerControlSample18 =
        (steadyInput.writeCtx18 && steadyInput.ctx18Known)
            ? static_cast<uint16_t>(steadyInput.ctx18Value & 0xFFFFu)
            : rawControlMask18;
    if (steadyInput.incrementReplayReadIndex901C0) {
        directReplay.writeCount901C0 = steadyInput.replayReadIndex901C0After;
    }
    const bool replayMode52 = ResolveStage1Ctx52ReplayMode7A60(state);
    const PrStage1ScorerHostDirectAcceptedCarrierCoreUpdateInput directInput{
        replayMode52,
        tick96,
        queryFrame30,
        writerControlSample18,
        steadyInput.writeCtx20 && steadyInput.ctx20Known,
        static_cast<uint8_t>(steadyInput.ctx20Value & 0xFF),
        steadyInput.incrementReplayReadIndex901C0,
        steadyInput.writeCtx10CurrentTick,
        steadyInput.writeCtx10ReplayTick,
        steadyInput.ctx10ReplayTick,
        ResolveStage1AcceptedProducerSubstate50(state),
        ResolveStage1AcceptedProducerHalfWindow34(ctx, state),
        ResolveStage1AcceptedProducerWritePageOrdinal38(state, tick96),
    };
    PrStage1ScorerHostDirectUpdateAcceptedCarrierCore(
        directInput,
        directReplay,
        directScriptedWriter,
        directCarrier);
    StoreStage1AcceptedHostDirectReplayBufferAdapter(
        directReplay,
        state.acceptedProducerReplayBuffer);
    StoreStage1AcceptedHostDirectScriptedWriterAdapter(
        directScriptedWriter,
        state.acceptedProducerScriptedWriter);
    StoreStage1AcceptedHostDirectCarrierCoreAdapter(directCarrier, carrier);
    UpdateStage1AcceptedProducerGateCarrierRuntime(ctx, state, tick96);
}

bool ShouldAllowStage1LateBranchDynamicCarryAcceptedMask(
    const Stage1NumericRuntimeState& state) {
    const Stage1NumericRuntimeState::DescriptorCadenceRuntime& descriptorRuntime =
        state.descriptorCadence;
    return state.rightRankState.rightRankActiveRow == 1u &&
           !state.bucket30RightRankWritebackCommitted &&
           state.rightRankPhase1Owner.phase1LatchArmed38 &&
           descriptorRuntime.nextLookaheadAvailable &&
           descriptorRuntime.nextLookaheadRowIndex == 9u;
}

void ApplyStage1DirectPortFollowUpPhaseAction(
    Stage1NumericRuntimeState& state,
    PrStage1ScorerDirectFollowUpPhaseAction action) {
    switch (action) {
    case PrStage1ScorerDirectFollowUpPhaseAction::SetAwaitBucket31AfterGoodToCool:
        state.rightRankFollowUpPhase =
            Stage1NumericRuntimeState::RightRankFollowUpPhase::AwaitBucket31AfterGoodToCool;
        break;
    case PrStage1ScorerDirectFollowUpPhaseAction::SetAwaitForceGoodWriteback:
        state.rightRankFollowUpPhase =
            Stage1NumericRuntimeState::RightRankFollowUpPhase::AwaitForceGoodWriteback;
        break;
    case PrStage1ScorerDirectFollowUpPhaseAction::ClearToNone:
        state.rightRankFollowUpPhase =
            Stage1NumericRuntimeState::RightRankFollowUpPhase::None;
        break;
    case PrStage1ScorerDirectFollowUpPhaseAction::None:
    default:
        break;
    }
}

void ApplyStage1DirectPortBucket30Phase1StateAdapter(
    Stage1NumericRuntimeState& state,
    const PrStage1ScorerDirectGlobals& globals,
    uint8_t activeRow) {
    (void)activeRow;
    state.rightRankPhase1Owner.baselineValue18 = globals.word91818SharedBaseline;
    state.rightRankPhase1Owner.cachedPhase1Classifier36 =
        globals.word8ED36Phase1Cache;
    state.rightRankPhase1Owner.phase1LatchArmed38 =
        globals.word8ED38PhaseCounter != 0u;
    state.rightRankHelperShadow.tieCarryLatch1E =
        globals.word9181ETwitterCarryLatch;
}

void ApplyStage1Bucket30CoolPhase1ObserverProjection(
    Stage1NumericRuntimeState& state,
    const PrStage1ScorerDirectGlobals& globals) {
    state.rightRankHelperShadow.coolGainLatch26 =
        globals.word91826CoolGainLatch;
    state.rightRankHelperShadow.coolGainStreak28 =
        globals.word91828CoolGainStreak;
}

static int32_t ReadStage1RightRankLiveAccumulator(
    const Stage1NumericRuntimeState& state) {
    return state.scorerPort.accumulator91816;
}

static void ClearStage1RightRankBlinkEmitGate(
    Stage1NumericRuntimeState& state) {
    state.rightRankState.rightRankBlinkEnabled = false;
}

static void ClearStage1AcceptedProducerMirrorAfterDirect14C1C(
    Stage1NumericRuntimeState& state) {
    state.acceptedProducer = Stage1NumericRuntimeState::SharedAcceptedProducerRuntime{};
    state.acceptedProducerLastRecordedPageWrite =
        Stage1NumericRuntimeState::AcceptedProducerLastRecordedPageWriteRuntime{};
}

void ApplyStage1DirectAcceptedClearBookkeeping(
    Stage1NumericRuntimeState& state,
    bool narrowClearFired,
    Stage1NumericRuntimeState::AcceptedProducerNarrowClearPhase phase,
    bool clearDeferredAdditiveBookkeeping) {
    ClearStage1AcceptedProducerMirrorAfterDirect14C1C(state);
    if (narrowClearFired) {
        state.acceptedProducerNarrowClearPending = false;
        state.acceptedProducerNarrowClearFired = true;
        state.acceptedProducerNarrowClearPhase = phase;
    }
    if (clearDeferredAdditiveBookkeeping) {
        ClearStage1AdditiveLaneBookkeepingAfterDirectClear(state);
    }
}

void ApplyStage1Bucket30DirectClearSceneProjection(
    Stage1NumericRuntimeState& state,
    const PrStage1ScorerDirectBucket30ClearSliceResult& directClearSlice) {
    switch (directClearSlice.action) {
    case PrStage1ScorerDirectAcceptedClearAction::DeferBucket31:
        state.acceptedProducerNarrowClearPending = true;
        state.acceptedProducerNarrowClearPhase =
            Stage1NumericRuntimeState::AcceptedProducerNarrowClearPhase::Bucket31;
        break;
    case PrStage1ScorerDirectAcceptedClearAction::DeferBucket30:
        state.acceptedProducerNarrowClearPending = true;
        state.acceptedProducerNarrowClearPhase =
            Stage1NumericRuntimeState::AcceptedProducerNarrowClearPhase::Bucket30;
        break;
    case PrStage1ScorerDirectAcceptedClearAction::ClearBucket30Now:
        if (directClearSlice.bucketLocalClearRan) {
            ApplyStage1DirectAcceptedClearBookkeeping(
                state,
                true,
                Stage1NumericRuntimeState::AcceptedProducerNarrowClearPhase::Bucket30,
                directClearSlice.clearDeferredAdditiveBookkeeping);
        }
        break;
    case PrStage1ScorerDirectAcceptedClearAction::None:
    default:
        break;
    }

    if (directClearSlice.markBucket31AdditiveClearPending) {
        state.additiveLane.clearPendingBucket31 = true;
    }
}

void ApplyStage1Bucket30ResolvedRowPublishRuntime(
    Stage1NumericRuntimeState& state,
    const PrStage1ScorerDirectBucket30ResolvedPublish& publish) {
    state.bucket30ResolutionKnown = publish.bucket30ResolutionKnown;
    state.bucket30ResolutionV22 = publish.bucket30ResolutionV22;
    state.bucket30RightRankWritebackCommitted =
        publish.rightRankWritebackCommitted;
    if (!publish.resolvedRightRankRowKnown) {
        return;
    }

    if (publish.rowWriteEventKnown) {
        state.rightRankLastRowWriteKnown = true;
        state.rightRankLastRowWriteQueryFrame = (int32_t)state.queryFrame;
        state.rightRankLastRowWritePrevRow = publish.rowWritePreviousRow;
        state.rightRankLastRowWriteResolvedRow = publish.resolvedRightRankRow;
        state.rightRankLastRowWriteResolutionKnown =
            publish.bucket30ResolutionKnown;
        state.rightRankLastRowWriteResolutionV22 =
            publish.bucket30ResolutionV22;
        state.rightRankLastRowWriteCommitted =
            publish.rightRankWritebackCommitted;
        if (publish.resolvedRightRankRow == 3u &&
            !state.rightRankFirstRow3WriteKnown) {
            const auto& ownerObserver = state.bucket30OwnerObserver;
            state.rightRankFirstRow3WriteKnown = true;
            state.rightRankFirstRow3WriteQueryFrame =
                (int32_t)state.queryFrame;
            state.rightRankFirstRow3WritePrevRow =
                publish.rowWritePreviousRow;
            state.rightRankFirstRow3WriteResolvedRow =
                publish.resolvedRightRankRow;
            state.rightRankFirstRow3WriteResolutionKnown =
                publish.bucket30ResolutionKnown;
            state.rightRankFirstRow3WriteResolutionV22 =
                publish.bucket30ResolutionV22;
            state.rightRankFirstRow3WriteCommitted =
                publish.rightRankWritebackCommitted;
            state.rightRankFirstRow3WriteOwnerActiveRow =
                ownerObserver.activeRow;
            state.rightRankFirstRow3WriteResolutionInputPhase1Classifier36 =
                ownerObserver.resolutionInputPhase1Classifier36;
            state.rightRankFirstRow3WritePhase1AdvanceFirstBeat =
                ownerObserver.phase1AdvanceFirstBeat;
            state.rightRankFirstRow3WritePhase1Delta91816MinusPrev18 =
                ownerObserver.phase1Delta91816MinusPrev18;
            state.rightRankFirstRow3WriteFormalWritebackKnown =
                ownerObserver.formalWritebackKnown;
            state.rightRankFirstRow3WriteFormalWritebackValue =
                ownerObserver.formalWritebackValue;
            state.rightRankFirstRow3WriteTieBreakerCalled =
                ownerObserver.tieBreakerCalled;
            state.rightRankFirstRow3WriteTieBreakerResult14548 =
                ownerObserver.tieBreakerResult14548;
            state.rightRankFirstRow3WriteResolverGateBit4 =
                ownerObserver.resolverGateBit4;
            state.rightRankFirstRow3WriteResolverGateEd00Idle =
                ownerObserver.resolverGateEd00Idle;
            state.rightRankFirstRow3WriteGoodToCoolCommitted =
                ownerObserver.goodToCoolCommitted;
        }
    }
    state.rightRankState.rightRankActiveRow = publish.resolvedRightRankRow;
}

void ApplyStage1Bucket30RowWriteSceneProjection(
    Stage1NumericRuntimeState& state,
    const PrStage1ScorerDirectBucket30ResolvedPublish& publish,
    const PrStage1ScorerDirectBucket30RowWriteSliceResult& directRowWriteSlice) {
    ApplyStage1Bucket30ResolvedRowPublishRuntime(state, publish);
    if (directRowWriteSlice.rowWrite.goodToCoolCommitted) {
        if (directRowWriteSlice.playPrepareCue) {
            PrSfx::PlayStage1DelayedFollowUpPrepareCue(
                directRowWriteSlice.prepareCueUseRepeatVariant);
        }
        state.rightRankDirectFollowUp.flag2000Pulse =
            directRowWriteSlice.ctxFlag2000Pulse;
        ApplyStage1DirectPortFollowUpPhaseAction(
            state,
            directRowWriteSlice.followUpPhaseAction);
        return;
    }

    if (directRowWriteSlice.rowWrite.resolutionKnown &&
        directRowWriteSlice.rowWrite.rightRankWritebackCommitted) {
        ApplyStage1DirectPortFollowUpPhaseAction(
            state,
            directRowWriteSlice.followUpPhaseAction);
    }
}

void ApplyStage1Bucket30SteadySfxSceneProjection(
    Stage1NumericRuntimeState& state,
    const PrStage1ScorerDirectSteadySfxAction& directAction) {
    if (directAction.clearMirror) {
        ClearStage1SteadySfxDispatchMirror(state);
    }
    state.steadySfx.callbackSiteReached = directAction.callbackSiteReached;
    if (!directAction.play) {
        return;
    }

    Stage1SteadySfxDispatchRuntime dispatch{};
    dispatch.firedBucket30 = true;
    dispatch.known = true;
    dispatch.tableSlot = directAction.tableSlot;
    dispatch.callbackSiteReached = directAction.callbackSiteReached;
    switch (directAction.cueId) {
    case PrStage1ScorerDirectSteadyCueId::Bucket30Verdict:
        dispatch.cueId = Stage1SteadySfxDispatchCueId::Bucket30Verdict;
        break;
    case PrStage1ScorerDirectSteadyCueId::Bucket30RowCommit:
        dispatch.cueId = Stage1SteadySfxDispatchCueId::Bucket30RowCommit;
        break;
    case PrStage1ScorerDirectSteadyCueId::Bucket30DirectCue94400:
        dispatch.cueId = Stage1SteadySfxDispatchCueId::Bucket30DirectCue94400;
        break;
    case PrStage1ScorerDirectSteadyCueId::Bucket31Completion:
        dispatch.cueId = Stage1SteadySfxDispatchCueId::Bucket31Completion;
        break;
    case PrStage1ScorerDirectSteadyCueId::None:
    default:
        return;
    }
    state.steadySfx = dispatch;
    switch (dispatch.cueId) {
    case Stage1SteadySfxDispatchCueId::Bucket30Verdict:
        PrSfx::PlayStage1SteadyVerdictCue(dispatch.tableSlot);
        break;
    case Stage1SteadySfxDispatchCueId::Bucket30RowCommit:
        PrSfx::PlayStage1SteadyRowCommitCue(dispatch.tableSlot);
        break;
    case Stage1SteadySfxDispatchCueId::Bucket30DirectCue94400:
        PrSfx::PlayStage1Bucket30DirectCue94400(dispatch.tableSlot);
        break;
    case Stage1SteadySfxDispatchCueId::None:
    default:
        break;
    }
}

void ApplyStage1Bucket30Phase1SceneProjection(
    Stage1NumericRuntimeState& state,
    uint8_t activeRow,
    const PrStage1ScorerDirectGlobals& directPhase1Globals,
    const PrStage1ScorerDirectPhase1StepResult& directPhase1Step,
    bool& outDirectConsumerSlotKnown,
    uint8_t& outDirectConsumerSlot) {
    ApplyStage1DirectPortBucket30Phase1StateAdapter(
        state,
        directPhase1Globals,
        activeRow);
    if (activeRow == 0u) {
        ApplyStage1Bucket30CoolPhase1ObserverProjection(
            state,
            directPhase1Globals);
        outDirectConsumerSlotKnown = directPhase1Step.directConsumerSlotKnown;
        outDirectConsumerSlot = (uint8_t)directPhase1Step.directConsumerSlot;
        return;
    }

    outDirectConsumerSlotKnown = true;
    outDirectConsumerSlot = directPhase1Step.sampledClassifier;
}

void ApplyStage1Bucket30DelayedTailSceneProjection(
    Stage1NumericRuntimeState& state,
    const PrStage1ScorerDirectBucket30DelayedTailSliceResult& directTailSlice) {
    if (directTailSlice.playArmCue) {
        PrSfx::PlayStage1DelayedFollowUpArmCue();
        state.rightRankDirectFollowUp.flag4000Pulse =
            directTailSlice.flag4000Set;
    }

    if (!directTailSlice.publishForcedGoodRowWrite) {
        return;
    }

    ApplyStage1Bucket30ResolvedRowPublishRuntime(
        state,
        directTailSlice.forcedGoodPublish);
    ApplyStage1DirectPortFollowUpPhaseAction(
        state,
        directTailSlice.followUpPhaseAction);
    if (directTailSlice.armBucket31DelayedSteadySfxCompletion) {
        ArmStage1Bucket31DelayedSteadySfxCompletion(state);
    }
}

void ApplyStage1Bucket31DirectSceneProjection(
    Stage1NumericRuntimeState& state,
    const PrStage1ScorerDirectBucket31DispatcherResult& directBucket31Result) {
    const PrStage1ScorerDirectBucket31ConsumerSliceResult& directConsumerSlice =
        directBucket31Result.consumer;
    state.rightRankDirectFollowUp.flag0200Pulse =
        directConsumerSlice.ctxFlag0200Pulse;
    if (directConsumerSlice.clearDelayedCompletionPending) {
        state.steadySfxDelayedCompletionPending = false;
    }
    if (!directConsumerSlice.bucketLocalClearRan) {
        return;
    }
    ApplyStage1DirectAcceptedClearBookkeeping(
        state,
        directConsumerSlice.narrowClearFired,
        Stage1NumericRuntimeState::AcceptedProducerNarrowClearPhase::Bucket31,
        directConsumerSlice.clearDeferredAdditiveBookkeeping);
    if (directConsumerSlice.consumerPackageRan) {
        ApplyStage1RightRank24F8CObserverProjection(state);
        ClearStage1RightRankBlinkEmitGate(state);
    }
}

void ClearStage1Bucket30ResolutionMirror(Stage1NumericRuntimeState& state) {
    state.bucket30ResolutionKnown = false;
    state.bucket30ResolutionV22 = 2u;
    state.bucket30RightRankWritebackCommitted = false;
}

void ClearStage1SteadySfxDispatchMirror(Stage1NumericRuntimeState& state) {
    state.steadySfx = Stage1SteadySfxDispatchRuntime{};
}

void ArmStage1Bucket31DelayedSteadySfxCompletion(
    Stage1NumericRuntimeState& state) {
    state.steadySfxDelayedCompletionPending = true;
}

void ApplyStage1Bucket0WindowSceneProjection(
    Stage1NumericRuntimeState& state,
    const PrStage1ScorerDirectBucket0WindowSliceResult& directWindow,
    uint16_t descriptorFlagWord) {
    state.rightRankHelperShadow.bucket0CallWindowOpen = directWindow.callWindowOpen;
    state.rightRankHelperShadow.bucket0Ctx118WritePulse =
        directWindow.ctx118WritePulse;
    if (directWindow.ctx118WritePulse) {
        state.rightRankHelperShadow.bucket0Ctx118WritePulseQueryFrame =
            state.queryFrame;
        state.rightRankHelperShadow.bucket0Ctx118WritePulseRightRankRow =
            state.rightRankState.rightRankActiveRow;
        state.rightRankHelperShadow.bucket0Ctx118WritePulseCallWindowOpen =
            directWindow.callWindowOpen;
        state.rightRankHelperShadow.bucket0Ctx118WritePulseDescriptorFlags =
            descriptorFlagWord;
    }
}

void ApplyStage1RightRank24F8CObserverProjection(
    Stage1NumericRuntimeState& state) {
    const int32_t liveAccumulator = ReadStage1RightRankLiveAccumulator(state);
    state.rightRank24F8CObserver.formalWritebackValue24 = liveAccumulator;
    state.rightRank24F8CObserver.lastBaselineReseedQueryFrame =
        (int32_t)state.queryFrame;
    state.rightRank24F8CObserver.snapshot18 = liveAccumulator;
}

PrStage1ScorerDirectBaselineRefresh1448CResult
ApplyStage1ScorerHostResetAction80024F8C(
    Stage1NumericRuntimeState& state,
    const PrStageEventDirectResetAction80024F8C& action) {
    PrStage1ScorerDirectGlobals globals =
        BuildStage1DirectPortGlobalsAdapter(state);
    const PrStage1ScorerDirectBaselineRefresh1448CResult result =
        PrStage1ScorerDirectApplyResetAction80024F8C(globals, action);
    if (!result.applied) {
        return result;
    }

    state.rightRankPhase1Owner.baselineValue18 =
        globals.word91818SharedBaseline;
    state.rightRankTieBreakerObserver.snapshot1A =
        globals.word9181ASnapshot;
    state.rightRankHelperShadow.snapshot1C =
        globals.word9181CCompareBaseline;
    ApplyStage1RightRank24F8CObserverProjection(state);
    return result;
}

int32_t ApplyStage1ScorerHostScoreMirror80024FC0(
    Stage1NumericRuntimeState& state) {
    PrStage1ScorerDirectGlobals globals =
        BuildStage1DirectPortGlobalsAdapter(state);
    PrStage1ScorerDirectBucketContext ctx =
        BuildStage1DirectPortBucketContextAdapter(state, 0);
    const int32_t result =
        PrStage1ScorerDirectPublishScoreMirror80024FC0(globals, ctx);
    state.scoreDisplayValue = ctx.dword30ScoreDisplayValue;
    state.scoreDisplayFormulaBacked = true;
    return result;
}

const Stage1NumericRuntimeState::DescriptorRowView*
ResolveStage1FormalScorerCommittedDescriptorRow(
    const Stage1NumericRuntimeState& runtime) {
    const Stage1NumericRuntimeState::DescriptorCadenceRuntime& descriptorRuntime =
        runtime.descriptorCadence;
    if (descriptorRuntime.currentDescriptor40Available &&
        descriptorRuntime.currentDescriptor40Row.available) {
        return &descriptorRuntime.currentDescriptor40Row;
    }
    if (runtime.currentDescriptorRow.available) {
        return &runtime.currentDescriptorRow;
    }
    return nullptr;
}

Stage1FormalScorerBranchState ConvertStage1DirectPortBranchStateAdapter(
    const PrStage1ScorerDirectBranchState& direct) {
    Stage1FormalScorerBranchState out{};
    out.countKnown = direct.countKnown;
    out.countValue = direct.countValue;
    out.pairBonusKnown = direct.pairBonusKnown;
    out.pairBonusValue = direct.pairBonusValue;
    out.spillPenaltyKnown = direct.spillPenaltyKnown;
    out.spillPenaltyValue = direct.spillPenaltyValue;
    out.descriptorSubdeltaKnown = direct.descriptorSubdeltaKnown;
    out.descriptorSubdeltaValue = direct.descriptorSubdeltaValue;
    out.readerPageOrdinalKnown = direct.readerPageOrdinalKnown;
    out.readerPageOrdinalValue = direct.readerPageOrdinalValue;
    out.requiredMaskKnown = direct.requiredMaskKnown;
    out.requiredMaskValue = direct.requiredMaskValue;
    out.unionMaskKnown = direct.unionMaskKnown;
    out.unionMaskValue = direct.unionMaskValue;
    out.anchorSlotIndexKnown = direct.anchorSlotIndexKnown;
    out.anchorSlotIndexValue = direct.anchorSlotIndexValue;
    out.requiredClassTokenKnown = direct.requiredClassTokenKnown;
    out.requiredClassTokenValue = direct.requiredClassTokenValue;
    out.anchorSlotClassTokenKnown = direct.anchorSlotClassTokenKnown;
    out.anchorSlotClassTokenValue = direct.anchorSlotClassTokenValue;
    out.anchorSlotOccupiedKnown = direct.anchorSlotOccupiedKnown;
    out.anchorSlotOccupiedValue = direct.anchorSlotOccupiedValue;
    out.anchorClassMatchKnown = direct.anchorClassMatchKnown;
    out.anchorClassMatchValue = direct.anchorClassMatchValue;
    return out;
}

Stage1FormalScorerRuntimeState ConvertStage1DirectPortRuntimeStateAdapter(
    const PrStage1ScorerDirectCommitState& direct) {
    Stage1FormalScorerRuntimeState out{};
    out.currentBranchKnown = direct.currentBranchKnown;
    out.currentBranchSpecial = direct.currentBranchSpecial;
    out.additiveTermKnown = direct.additiveTermKnown;
    out.additiveTermValue = direct.additiveTermValue;
    out.acceptedCountKnown = direct.acceptedCountKnown;
    out.acceptedCountValue = direct.acceptedCountValue;
    out.currentLookbackPageCountKnown = direct.currentLookbackPageCountKnown;
    out.currentLookbackPageCountValue = direct.currentLookbackPageCountValue;
    out.overflowActiveKnown = direct.overflowActiveKnown;
    out.overflowActive = direct.overflowActive;
    out.defaultBranch = ConvertStage1DirectPortBranchStateAdapter(direct.defaultBranch);
    out.specialBranch = ConvertStage1DirectPortBranchStateAdapter(direct.specialBranch);
    return out;
}

Stage1FormalScorerCommitRuntimeState ConvertStage1DirectPortCommitStateAdapter(
    const PrStage1ScorerDirectCommitState& direct) {
    Stage1FormalScorerCommitRuntimeState out{};
    out.currentBranchKnown = direct.currentBranchKnown;
    out.currentBranchSpecial = direct.currentBranchSpecial;
    out.additiveTermKnown = direct.additiveTermKnown;
    out.additiveTermValue = direct.additiveTermValue;
    out.acceptedCountKnown = direct.acceptedCountKnown;
    out.acceptedCountValue = direct.acceptedCountValue;
    out.currentLookbackPageCountKnown = direct.currentLookbackPageCountKnown;
    out.currentLookbackPageCountValue = direct.currentLookbackPageCountValue;
    out.overflowActiveKnown = direct.overflowActiveKnown;
    out.overflowActive = direct.overflowActive;
    out.activeBranch = ConvertStage1DirectPortBranchStateAdapter(direct.activeBranch);
    out.commitTermKnown = direct.commitTermKnown;
    out.commitTermValue = direct.commitTermValue;
    out.scoreWritebackKnown = direct.scoreWritebackKnown;
    out.scoreWritebackValue = direct.scoreWritebackValue;
    out.clampActiveKnown = direct.clampActiveKnown;
    out.clampActive = direct.clampActive;
    return out;
}

void InitializeStage1FormulaAccumulatorBaseline(Stage1NumericRuntimeState& state) {
    state.formulaAccumulatorValue = 0;
    state.formulaAccumulatorKnown = true;
    state.formulaAccumulatorAuthoritative = true;
    state.scorerPort.accumulator91816 = 0;
    state.scoreDisplayValue = state.scorerPort.accumulator91816;
    state.scoreDisplayFormulaBacked = false;
}

void SeedStage1AuthoritativeAdditiveLaneRuntime(Stage1NumericRuntimeState& state) {
    Stage1NumericRuntimeState::AdditiveLaneRuntime& lane = state.additiveLane;
    lane.value = 0;
    state.scorerPort.additiveTerm91822 = (int16_t)lane.value;
    lane.clearPendingBucket31 = false;
    lane.bucket30CommitVisible = false;
    lane.bucket30CommitValue = 0;
}

void CaptureStage1StartupSetup7A60(
    const PrGameContext& ctx,
    Stage1NumericRuntimeState& state,
    const PrStageRunner& runner,
    const Stage1NumericRuntimeTimecodeInput801C7560& timecodeInput) {
    uint32_t payloadPrevGrade92F40 = 0u;
    const bool specialSetupReturn16758Known =
        ctx.transitionState == 2 &&
        TryReadStage1SetupRestartReplayPayloadPrevGrade92F40(
            payloadPrevGrade92F40);
    const int32_t specialSetupReturn16758 =
        (specialSetupReturn16758Known && payloadPrevGrade92F40 >= 2u) ? 1 : 0;
    const PrStageClearStatusQueryResult defaultSetupStatus166AC =
        (ctx.transitionState != 1 && ctx.transitionState != 2)
            ? PrStage1SaveUiDirect::Sub800166AC(runner.GetSceneId())
            : PrStageClearStatusQueryResult{};
    const bool defaultSetupDirectStatus166ACKnown =
        ctx.transitionState != 1 && ctx.transitionState != 2 &&
        defaultSetupStatus166AC.ok &&
        defaultSetupStatus166AC.statusBankKnown &&
        defaultSetupStatus166AC.mapped;
    uint8_t defaultSetupStatus166ACValue = 0u;
    if (defaultSetupDirectStatus166ACKnown) {
        defaultSetupStatus166ACValue = defaultSetupStatus166AC.status;
    }
    const bool defaultSetupReturn1670CKnown =
        defaultSetupDirectStatus166ACKnown;
    const int32_t defaultSetupReturn1670C =
        (defaultSetupReturn1670CKnown &&
         defaultSetupStatus166ACValue >= 2u)
            ? 1
            : 0;
    const PrStageRunnerDirectStartupSetup7A60Result result =
        PrStageRunnerDirectResolveStartupSetup7A60(
            PrStageRunnerDirectStartupSetup7A60Input{
                static_cast<uint16_t>(ctx.transitionState),
                static_cast<uint16_t>(ctx.transitionStateDA),
                defaultSetupReturn1670CKnown,
                defaultSetupReturn1670C,
                specialSetupReturn16758Known,
                specialSetupReturn16758,
            });
    state.ctx52ReplayMode7A60 =
        result.writeWord41 && result.word41Value != 0u;

    const uint16_t recordsMode916DAAfterSetup =
        result.clearRecordsMode916DA
            ? 0u
            : static_cast<uint16_t>(ctx.transitionStateDA);
    const PrStageRunnerDirectPreamble7A60Result preamble =
        PrStageRunnerDirectResolvePreamble7A60(
            PrStageRunnerDirectPreamble7A60Input{
                recordsMode916DAAfterSetup,
                timecodeInput.sceneEntryField360HalfSource,
                0u,
            });
    const int32_t halfWindow34 = preamble.writeDword34;
    state.runnerPreambleHalfWindow34Known = true;
    state.runnerPreambleHalfWindow34 = static_cast<uint8_t>(
        (std::min)(255, (std::max)(0, halfWindow34)));

    if (result.firstCall143F0 && result.firstCall143F0ArgKnown) {
        const bool enabled = result.firstCall143F0Arg != 0;
        state.word9182AEnabled = enabled;
    }
    if (result.tailCall143F0Zero) {
        state.word9182AEnabled = false;
    }

    Stage1NumericRuntimeState::RightRankPhase1OwnerRuntime& phase1Owner =
        state.rightRankPhase1Owner;
    phase1Owner.shortWindowLatch34Known = false;
    phase1Owner.shortWindowLatch34 = 0u;
    if (result.call259C0 && result.call259C0ArgKnown) {
        phase1Owner.shortWindowLatch34Known = true;
        phase1Owner.shortWindowLatch34 =
            static_cast<uint16_t>(result.call259C0Arg);
    }
    if (result.tailCall259C0Zero) {
        phase1Owner.shortWindowLatch34Known = true;
        phase1Owner.shortWindowLatch34 = 0u;
    }
}

PrStage1ScorerHostDirectSteadyInput7A60Result CaptureStage1SteadyInput7A60(
    Stage1NumericRuntimeState& state,
    uint16_t heldMask,
    int32_t tick96) {
    const Stage1NumericRuntimeState::AcceptedProducerReplayBufferRuntime& replay =
        state.acceptedProducerReplayBuffer;
    const bool replayMode52 = ResolveStage1Ctx52ReplayMode7A60(state);
    const bool replaySlotKnown =
        replayMode52 &&
        replay.writeCount901C0 < replay.publishedCount901BC &&
        replay.writeCount901C0 < kStage1AcceptedProducerReplayBufferCapacity;
    const uint32_t replaySlotIndex = replay.writeCount901C0;
    PrStageRunnerDirectInputDispatch7A60Input input{};
    input.replayModeWord41 = replayMode52;
    input.replayReadIndex901C0 = replay.writeCount901C0;
    input.replayPublishedCount901BC = replay.publishedCount901BC;
    input.ctxTick0C = tick96;
    input.replaySlot.available = replaySlotKnown;
    input.replaySlot.tick96 =
        replaySlotKnown ? (int32_t)replay.tick96EEF8[(size_t)replaySlotIndex]
                        : 0;
    input.replaySlot.rawMask =
        replaySlotKnown ? replay.classMaskEEFC[(size_t)replaySlotIndex] : 0u;
    input.livePoll35510Known = true;
    input.livePoll35510 = heldMask;

    const PrStageRunnerDirectInputDispatch7A60Result direct =
        PrStageRunnerDirectResolveInputDispatch7A60(input);
    PrStage1ScorerHostDirectSteadyInput7A60Result out{};
    out.writeCtx18 = direct.writeCtx18;
    out.ctx18Known = direct.writeCtx18;
    out.ctx18Value = direct.ctx18Value;
    out.writeCtx20 = direct.writeCtx20;
    out.ctx20Known = direct.ctx20Known;
    out.ctx20Value = direct.ctx20Value;
    out.incrementReplayReadIndex901C0 = direct.incrementReplayReadIndex901C0;
    out.replayReadIndex901C0After = direct.replayReadIndex901C0After;
    out.writeCtx10CurrentTick = direct.writeCtx10CurrentTick;
    out.writeCtx10ReplayTick = direct.writeCtx10ReplayTick;
    out.ctx10ReplayTick = direct.ctx10ReplayTick;
    state.steadyInput7A60 = Stage1NumericRuntimeState::SteadyInput7A60Runtime{};
    state.steadyInput7A60.known = true;
    state.steadyInput7A60.replayMode52 = replayMode52;
    state.steadyInput7A60.heldMask = heldMask;
    state.steadyInput7A60.writeCtx18 = direct.writeCtx18;
    state.steadyInput7A60.ctx18Known = direct.writeCtx18;
    state.steadyInput7A60.ctx18Value = direct.ctx18Value;
    state.steadyInput7A60.writeCtx20 = direct.writeCtx20;
    state.steadyInput7A60.ctx20Known = direct.ctx20Known;
    state.steadyInput7A60.ctx20Value = direct.ctx20Value;
    state.steadyInput7A60.incrementReplayReadIndex901C0 =
        direct.incrementReplayReadIndex901C0;
    state.steadyInput7A60.replayReadIndex901C0After =
        direct.replayReadIndex901C0After;
    state.steadyInput7A60.writeCtx10CurrentTick = direct.writeCtx10CurrentTick;
    state.steadyInput7A60.writeCtx10ReplayTick = direct.writeCtx10ReplayTick;
    state.steadyInput7A60.ctx10ReplayTick = direct.ctx10ReplayTick;
    return out;
}

void ApplyStage1PostAcceptedLoop7A60(
    Stage1NumericRuntimeState& state,
    const PrStageRunnerDirectPostFrame7A60Result& postFrame) {
    Stage1NumericRuntimeState::AdditiveLaneRuntime& lane = state.additiveLane;

    const PrStage1RatingPresentationDirectPostTailFlag0200Dispatch7A60Result
        flag0200PostTail =
            PrStage1RatingPresentationDirectResolvePostTailFlag0200Dispatch7A60(
                PrStage1RatingPresentationDirectPostTailFlag0200Dispatch7A60Input{
                    state.rightRankDirectFollowUp.flag0200Pulse,
                    static_cast<uint16_t>(
                        state.rightRankState.rightRankActiveRow),
                });
    state.rightRankDirectFollowUp.flag0200PostTail1A654Requested =
        flag0200PostTail.call1A654;
    state.rightRankDirectFollowUp.flag0200PostTail1A654Arg =
        flag0200PostTail.call1A654Arg;
    state.runnerTailHost7A60.requestXaSetFilter13 =
        flag0200PostTail.call1A654;
    state.runnerTailHost7A60.xaSetFilter13Arg =
        flag0200PostTail.call1A654Arg;
    if (flag0200PostTail.call1A654) {
        ++state.runnerTailHost7A60.xaSetFilter13RequestCount;
        state.runnerTailHost7A60.xaSetFilter13LastRequestKnown = true;
        state.runnerTailHost7A60.xaSetFilter13LastRequestQueryFrame =
            state.queryFrame;
        state.runnerTailHost7A60.xaSetFilter13LastRequestTick96 =
            state.runnerTimecode801C7560.state.tick801C364C;
        state.runnerTailHost7A60.xaSetFilter13LastRequestRow =
            state.rightRankState.rightRankActiveRow;
        state.runnerTailHost7A60.xaSetFilter13LastRequestArg =
            flag0200PostTail.call1A654Arg;
    }

    if (postFrame.fixedBonus250Requested) {
        PrStage1ScorerDirectGlobals globals =
            BuildStage1DirectPortGlobalsAdapter(state);
        const uint16_t acceptedCount =
            PrStage1ScorerDirectInjectAdditive14C5C(
                globals,
                static_cast<int16_t>(250));
        state.scorerPort.additiveTerm91822 = globals.word91822AdditiveTerm;
        state.scorerPort.acceptedCount91810 = acceptedCount;
        lane.value = globals.word91822AdditiveTerm;
    }
}

static void CaptureStage1AcceptedInputSourceEventRuntime(
    Stage1NumericRuntimeState& state,
    uint32_t queryFrame,
    int32_t tick96,
    const PrStageRunnerDirectPostFrame7A60Result& postFrame) {
    const Stage1NumericRuntimeState::SteadyInput7A60Runtime& steady =
        state.steadyInput7A60;
    if (!steady.writeCtx10CurrentTick && !postFrame.acceptedGateOpen &&
        !postFrame.backupCtx10FromCtx0C) {
        return;
    }
    state.acceptedInputSourceEvent =
        Stage1NumericRuntimeState::AcceptedInputSourceEventRuntime{};
    state.acceptedInputSourceEvent.available = true;
    state.acceptedInputSourceEvent.queryFrame = queryFrame;
    state.acceptedInputSourceEvent.tick96 = tick96;
    state.acceptedInputSourceEvent.heldMask = steady.heldMask;
    state.acceptedInputSourceEvent.ctx18Value = steady.ctx18Value;
    state.acceptedInputSourceEvent.writeCtx10CurrentTick =
        steady.writeCtx10CurrentTick;
    state.acceptedInputSourceEvent.postAcceptedGateOpen =
        postFrame.acceptedGateOpen;
    state.acceptedInputSourceEvent.postAcceptedMaskChanged =
        postFrame.acceptedMaskChanged;
    state.acceptedInputSourceEvent.postBackupCtx10 =
        postFrame.backupCtx10FromCtx0C;
}

void CaptureStage1RunnerTailHostPostFrame7A60(
    Stage1NumericRuntimeState& state,
    const PrStageRunnerDirectPostFrame7A60Result& postFrame,
    uint32_t queryFrame,
    int32_t tick96) {
    Stage1NumericRuntimeState::RunnerTailHost7A60Runtime& host =
        state.runnerTailHost7A60;
    host.known = true;
    host.queryFrame = queryFrame;
    host.tick96 = tick96;
    host.postFrameTrace = postFrame.trace;
    for (size_t i = 0; i < postFrame.trace.count; ++i) {
        const PrStageRunnerDirectAction& action =
            postFrame.trace.actions[i];
        if (action.kind == PrStageRunnerDirectActionKind::Call80026EF8) {
            host.requestFailCue943EC = true;
        } else if (action.kind ==
                   PrStageRunnerDirectActionKind::Call8001A4A4) {
            host.requestAudioA4A4Arg1 =
                action.arg0Known && action.arg0 == 1;
        }
    }
}

PrStageRunnerDirectPostFrame7A60Result ResolveStage1RunnerPostFrame7A60(
    Stage1NumericRuntimeState& state,
    const PrStageRunner& runner,
    bool ctxAcceptedInputFlagSet,
    int32_t tick96,
    uint16_t writerControlSample18,
    bool scorerAccept14614Known,
    bool scorerAccept14614ReturnsNonZero) {
    const uint32_t ctxFlags =
        ResolveStage1RunnerCtxFlagsForPostFrame7A60(
            state,
            ctxAcceptedInputFlagSet);
    const PrStageRunnerDirectPostFrame7A60Result postFrame =
        PrStageRunnerDirectResolvePostFrame7A60(
            PrStageRunnerDirectPostFrame7A60Input{
                ctxFlags,
                writerControlSample18,
                state.runnerPostFramePreviousInputMask801CCBB8,
                ResolveStage1Ctx52ReplayMode7A60(state),
                scorerAccept14614Known,
                scorerAccept14614ReturnsNonZero,
                IsStage1SpecialFixedBonusAdditiveGateActive(runner),
            });

    Stage1NumericRuntimeState::RunnerPostFrame7A60Runtime& runtime =
        state.runnerPostFrame7A60;
    const bool previous801C9094FinalKnown =
        runtime.word0Flags801C9094FinalKnown;
    const uint32_t previous801C9094Final =
        runtime.word0Flags801C9094Final;
    const bool previousRequiresSameFrameCtx =
        runtime.word0Flags801C9094RequiresSameFrameCtx;
    const bool previousRequiresSameFrameGlobals =
        runtime.word0Flags801C9094RequiresSameFrameGlobals;
    const bool previousRequiresFlagStreamFlags04 =
        runtime.word0Flags801C9094RequiresFlagStreamFlags04;
    const bool previousRequiresIdStreamDescriptorState =
        runtime.word0Flags801C9094RequiresIdStreamDescriptorState;
    const bool previousRequiresCompactRailRecord =
        runtime.word0Flags801C9094RequiresCompactRailRecord;
    const PrStageRunnerDirectCtx0FlagsSnapshot801C3640 ctx0Snapshot =
        PrStageRunnerDirectMakeCtx0BaseFlagsSnapshot801C7A60(
            ctxAcceptedInputFlagSet);
    runtime.known = true;
    runtime.ctxFlags00 = ctxFlags;
    runtime.ctx0Owner801C3640.snapshotAvailable = true;
    runtime.ctx0Owner801C3640.queryFrame = state.queryFrame;
    runtime.ctx0Owner801C3640.tick96 = tick96;
    runtime.ctx0Owner801C3640.baseKnown = ctx0Snapshot.baseKnown;
    runtime.ctx0Owner801C3640.baseFlags = ctx0Snapshot.baseFlags;
    runtime.ctx0Owner801C3640.finalKnown = ctx0Snapshot.finalKnown;
    runtime.ctx0Owner801C3640.finalFlags = ctx0Snapshot.finalFlags;
    runtime.ctx0Owner801C3640.missingFinalDirectOwner =
        !ctx0Snapshot.finalKnown;
    runtime.word0Flags801C3640BaseKnown = ctx0Snapshot.baseKnown;
    runtime.word0Flags801C3640Base = ctx0Snapshot.baseFlags;
    runtime.word0Flags801C3640FinalKnown = ctx0Snapshot.finalKnown;
    runtime.word0Flags801C3640Final = ctx0Snapshot.finalFlags;
    runtime.word0Flags801C9094FinalKnown = previous801C9094FinalKnown;
    runtime.word0Flags801C9094Final = previous801C9094Final;
    runtime.word0Flags801C9094RequiresSameFrameCtx =
        previousRequiresSameFrameCtx;
    runtime.word0Flags801C9094RequiresSameFrameGlobals =
        previousRequiresSameFrameGlobals;
    runtime.word0Flags801C9094RequiresFlagStreamFlags04 =
        previousRequiresFlagStreamFlags04;
    runtime.word0Flags801C9094RequiresIdStreamDescriptorState =
        previousRequiresIdStreamDescriptorState;
    runtime.word0Flags801C9094RequiresCompactRailRecord =
        previousRequiresCompactRailRecord;
    runtime.word0Flags801C3640Known = ctx0Snapshot.finalKnown;
    runtime.word0Flags801C3640 =
        ctx0Snapshot.finalKnown ? ctx0Snapshot.finalFlags
                                : ctx0Snapshot.baseFlags;
    runtime.ctxInput18 = writerControlSample18;
    runtime.previousInputMask801CCBB8 =
        state.runnerPostFramePreviousInputMask801CCBB8;
    runtime.acceptedMask9FF = postFrame.acceptedMask9FF;
    runtime.acceptedGateOpen = postFrame.acceptedGateOpen;
    runtime.acceptedMaskChanged = postFrame.acceptedMaskChanged;
    runtime.backupCtx08FromCtx04 = postFrame.backupCtx08FromCtx04;
    runtime.backupCtx10FromCtx0C = postFrame.backupCtx10FromCtx0C;
    runtime.callAcceptedProducer14614 = postFrame.callAcceptedProducer14614;
    runtime.callAcceptedProducer94434 = postFrame.callAcceptedProducer94434;
    runtime.callAcceptedProducer9443C = postFrame.callAcceptedProducer9443C;
    runtime.fixedBonus250Requested = postFrame.fixedBonus250Requested;
    runtime.continueMainLoop = postFrame.continueMainLoop;
    runtime.writeCtx3C = postFrame.writeCtx3C;
    runtime.ctx3CValue = postFrame.ctx3CValue;
    runtime.writeDword801CCBB8 = postFrame.writeDword801CCBB8;
    runtime.dword801CCBB8Value = postFrame.dword801CCBB8Value;
    if (postFrame.writeDword801CCBB8) {
        state.runnerPostFramePreviousInputMask801CCBB8 =
            postFrame.dword801CCBB8Value;
    }
    return postFrame;
}

void ResolveStage1RunnerMainLoopTail7A60(
    Stage1NumericRuntimeState& state,
    int32_t sceneEntryField352FallbackTickAdvance,
    bool continuationGate1A7F8Known,
    bool continuationGate1A7F8ReturnsOne,
    bool frameExit1A3B8Known,
    bool frameExit1A3B8ReturnsOne,
    const PrStageRunnerDirectPostFrame7A60Result& postFrame) {
    const PrStageRunnerDirectMainLoopTail7A60Result tail =
        PrStageRunnerDirectResolveMainLoopTail7A60(
            PrStageRunnerDirectMainLoopTail7A60Input{
                postFrame.continueMainLoop,
                sceneEntryField352FallbackTickAdvance,
                continuationGate1A7F8Known,
                continuationGate1A7F8ReturnsOne,
                frameExit1A3B8Known,
                frameExit1A3B8ReturnsOne,
            });
    state.runnerTailHost7A60.mainLoopTailTrace = tail.trace;

    Stage1NumericRuntimeState::RunnerMainLoopTail7A60Runtime& runtime =
        state.runnerMainLoopTail7A60;
    runtime.known = true;
    runtime.postFrameContinues = postFrame.continueMainLoop;
    runtime.sceneEntryField352FallbackTickAdvance =
        sceneEntryField352FallbackTickAdvance;
    runtime.callWait15350 = tail.callWait15350;
    runtime.wait15350Arg = tail.wait15350Arg;
    runtime.callWait35560 = tail.callWait35560;
    runtime.wait35560Arg = tail.wait35560Arg;
    runtime.callPresentFrame = tail.callPresentFrame;
    runtime.callContinuationGate1A7F8 = tail.callContinuationGate1A7F8;
    runtime.continuationGateKnown = tail.continuationGateKnown;
    runtime.continuationGateAllowsNextFrame =
        tail.continuationGateAllowsNextFrame;
    runtime.callFrameExit1A3B8 = tail.callFrameExit1A3B8;
    runtime.frameExitKnown = tail.frameExitKnown;
    runtime.frameExitTaken = tail.frameExitTaken;
    runtime.callEarlyExit1A694 = tail.callEarlyExit1A694;
    runtime.callEarlyExit35838 = tail.callEarlyExit35838;
    runtime.setWord59OnFrameExit = tail.setWord59OnFrameExit;
    runtime.cleanupRequiredKnown = tail.cleanupRequiredKnown;
    runtime.cleanupRequired = tail.cleanupRequired;
    runtime.clearWord50 = tail.clearWord50;
    runtime.clearWord42 = tail.clearWord42;
    runtime.clearWord61 = tail.clearWord61;
    runtime.cleanupDrainFrameCount = tail.cleanupDrainFrameCount;
    runtime.finalCall1B120 = tail.finalCall1B120;
    runtime.finalCall26FA4 = tail.finalCall26FA4;
    runtime.finalCall357D4Zero = tail.finalCall357D4Zero;
    runtime.finalCall1A694 = tail.finalCall1A694;
}

void CaptureStage1DescBankSelector7A60(
    Stage1NumericRuntimeState& state,
    uint32_t queryFrame,
    int32_t tick96,
    uint16_t writerControlSample18,
    bool call94434,
    bool call9443C) {
    if (!call94434 && !call9443C) {
        return;
    }

    const Stage1NumericRuntimeState::DescriptorCadenceRuntime& cadence =
        state.descriptorCadence;
    uint8_t lookaheadSelectorByte0 = 0u;
    uint8_t lookaheadSelectorByte1 = 0u;
    if (cadence.lookaheadDescriptor44Available &&
        cadence.lookaheadDescriptor44Row.available) {
        (void)TryResolveStage1DescriptorSelectorBytes(
            cadence.lookaheadDescriptor44Row,
            ResolveStage1NextDescriptorConsumerSubstateIndex(state),
            lookaheadSelectorByte0,
            lookaheadSelectorByte1);
        (void)lookaheadSelectorByte1;
    }

    const uint16_t currentBar1 =
        tick96 >= 0 ? static_cast<uint16_t>(tick96 / 384 + 1) : 0u;
    const uint8_t currentBeat1 =
        tick96 >= 0 ? static_cast<uint8_t>((tick96 % 384) / 96 + 1) : 0u;
    const Stage1NumericRuntimeState::RunnerSameFrameGlobalsOwner801C9094Runtime&
        globals801C9094 = state.runnerPostFrame7A60.sameFrameGlobals801C9094;
    const int32_t acceptedTailGateFramesD3058 =
        globals801C9094.snapshotAvailable
            ? globals801C9094.projectedGlobals.dword801D3058
            : 0;
    const uint16_t activeRow4E =
        static_cast<uint16_t>(state.rightRankState.rightRankActiveRow);

    const PrStage1ScorerHostDirectHudOverlayStart801CA1ECResult start801CA1EC =
        PrStage1ScorerHostDirectHudOverlayStart801CA1EC(
            PrStage1ScorerHostDirectHudOverlayStart801CA1ECInput{
                call94434,
                activeRow4E,
                writerControlSample18,
                cadence.currentDescriptor40Available &&
                    cadence.currentDescriptor40Row.available,
                cadence.currentDescriptor40Row.requiredMask,
                cadence.lookaheadDescriptor44Available &&
                    cadence.lookaheadDescriptor44Row.available,
                cadence.lookaheadDescriptor44Row.requiredMask,
            });
    const PrStage1ScorerHostDirectHudOverlayUpdate801CA264Result update801CA264 =
        PrStage1ScorerHostDirectHudOverlayUpdate801CA264(
            PrStage1ScorerHostDirectHudOverlayUpdate801CA264Input{
                call9443C,
                activeRow4E,
                writerControlSample18,
                acceptedTailGateFramesD3058,
                lookaheadSelectorByte0,
                currentBar1,
                currentBeat1,
            });

    Stage1NumericRuntimeState::RightRankDirectFollowUpRuntime& followUp =
        state.rightRankDirectFollowUp;
    if (start801CA1EC.ctxFlag800000Set) {
        followUp.descBankCtxFlagsPulse801CBFDC |= 0x00800000u;
    }
    if (start801CA1EC.selectorWrite && start801CA1EC.rebuildRequested &&
        start801CA1EC.selector >= 1u && start801CA1EC.selector <= 4u) {
        followUp.descBankSelectorPulse = true;
        followUp.descBankSelector = start801CA1EC.selector;
        followUp.descBankSelectorQueryFrame = queryFrame;
    }
    if (update801CA264.ctxFlag400Set) {
        followUp.descBankCtxFlagsPulse801CBFDC |= 0x00000400u;
    }
    if (update801CA264.ctxFlag800000Set) {
        followUp.descBankCtxFlagsPulse801CBFDC |= 0x00800000u;
    }
    if (update801CA264.selectorWrite) {
        followUp.descBankSelectorPulse = true;
        followUp.descBankSelector = update801CA264.selector;
        followUp.descBankSelectorQueryFrame = queryFrame;
    }
    if (update801CA264.cameraWrite && update801CA264.cameraBezHandle != 0u) {
        followUp.cameraPulse9443C = true;
        followUp.cameraBezHandle9443C = update801CA264.cameraBezHandle;
        followUp.cameraQueryFrame9443C = queryFrame;
    }
    if (update801CA264.followUpTimerFrames > 0) {
        followUp.acceptedTailTimer9443CPulse = true;
        followUp.acceptedTailTimer9443CFrames =
            update801CA264.followUpTimerFrames;
        followUp.acceptedTailTimer9443CQueryFrame = queryFrame;
    }
}

void ApplyStage1FrameUpdateFlag0200Branch9094(
    Stage1NumericRuntimeState& state) {
    if (state.runnerPostFrame7A60.frameUpdate9094ReturnEarlyAfterFlag40Reset) {
        Stage1NumericRuntimeState::RightRankDirectFollowUpRuntime& followUp =
            state.rightRankDirectFollowUp;
        followUp.flag0200FrameUpdateBranchTaken = false;
        followUp.flag0200FrameUpdateResourcePairKnown = false;
        followUp.flag0200FrameUpdateCtxE0ResourceIndex = 0u;
        followUp.flag0200FrameUpdateCtxECResourceIndex = 0u;
        followUp.flag0200FrameUpdateCtxFlag20000Set = false;
        return;
    }

    const PrStage1RatingPresentationDirectFrameUpdateFlag0200Branch9094Result
        result =
            PrStage1RatingPresentationDirectResolveFrameUpdateFlag0200Branch9094(
                PrStage1RatingPresentationDirectFrameUpdateFlag0200Branch9094Input{
                    state.rightRankDirectFollowUp.flag0200Pulse,
                    state.rightRankBucketContext.transitionAnim18E,
                });

    Stage1NumericRuntimeState::RightRankDirectFollowUpRuntime& followUp =
        state.rightRankDirectFollowUp;
    followUp.flag0200FrameUpdateBranchTaken = result.branchTaken;
    followUp.flag0200FrameUpdateResourcePairKnown = result.resourcePairKnown;
    followUp.flag0200FrameUpdateCtxE0ResourceIndex = result.ctxE0ResourceIndex;
    followUp.flag0200FrameUpdateCtxECResourceIndex = result.ctxECResourceIndex;
    followUp.flag0200FrameUpdateCtxFlag20000Set = result.setCtxFlag20000;
}

void CaptureStage1AuthoritativeAdditiveLaneBucket30Commit(
    Stage1NumericRuntimeState& state) {
    Stage1NumericRuntimeState::AdditiveLaneRuntime& lane = state.additiveLane;
    lane.bucket30CommitVisible = true;
    lane.bucket30CommitValue = lane.value;
}

void ClearStage1AdditiveLaneBookkeepingAfterDirectClear(
    Stage1NumericRuntimeState& state) {
    Stage1NumericRuntimeState::AdditiveLaneRuntime& lane = state.additiveLane;
    lane.value = 0;
    lane.clearPendingBucket31 = false;
}

Stage1FormulaAccumulatorWritebackState ResolveStage1FormulaAccumulatorWriteback(
    int32_t formulaAccumulatorBaselineValue,
    int32_t commitTermValue) {
    Stage1FormulaAccumulatorWritebackState out{};
    const int32_t low16Writeback =
        (int32_t)(uint16_t)formulaAccumulatorBaselineValue + commitTermValue;
    out.clampActive = (((uint32_t)low16Writeback) & 0x8000u) != 0u;
    out.writebackValue = out.clampActive ? 0 : (int32_t)(uint16_t)low16Writeback;
    return out;
}

void AdvanceStage1FormulaAccumulatorBaseline(Stage1NumericRuntimeState& state,
                                             int32_t nextBaselineValue) {
    if (!state.formulaAccumulatorKnown || !state.formulaAccumulatorAuthoritative) {
        return;
    }
    state.formulaAccumulatorValue = nextBaselineValue;
}

void CaptureStage1AcceptedProducerReplayBackup1635C(
    uint32_t prevGrade92F40,
    const Stage1NumericRuntimeState::AcceptedProducerReplayBufferRuntime& replay,
    Stage1AcceptedProducerReplayBackupRuntime& backup) {
    backup = Stage1AcceptedProducerReplayBackupRuntime{};
    const PrStage1ScorerDirectReplayBufferState directReplay =
        BuildStage1DirectPortReplayBufferAdapter(replay);
    const PrStage1ScorerDirectReplayBackupCaptureResult directResult =
        PrStage1ScorerDirectRunReplayBackupCaptureCore1635C(directReplay);
    PrStage1SaveUiDirect::SetReplayMirrorSource(directReplay);
    if (!directResult.backup.valid) {
        return;
    }
    backup.valid = directResult.backup.valid;
    backup.publishedCount901BC = directResult.backup.dword92F48PublishedCount;
    backup.prevGrade92F40Valid = true;
    backup.prevGrade92F40 = prevGrade92F40;
    for (size_t i = 0; i < kStage1AcceptedProducerReplayBufferCapacity; ++i) {
        backup.tick96EEF8[i] = directResult.backup.dwordEEF8Tick96[i];
        backup.classMaskEEFC[i] = directResult.backup.dwordEEFCClassMask[i];
    }
}

void ApplyStage1AcceptedSpecialSetupCore24E54_1681C(
    const PrGameContext& ctx,
    Stage1NumericRuntimeState& state) {
    PrStage1ScorerDirectReplayBufferState directReplay =
        BuildStage1DirectPortReplayBufferAdapter(
            state.acceptedProducerReplayBuffer);
    const PrStage1ScorerDirectAcceptedSpecialSetupResult directSpecialSetup =
        PrStage1ScorerDirectRunAcceptedSpecialSetupCore24E54_1681C(
            static_cast<uint16_t>(ctx.transitionState),
            ResolveStage1AcceptedReplayBackupSource1681C(
                s_stage1AcceptedProducerReplayBackupRuntime),
            directReplay);
    if (ctx.transitionState == 1) {
        (void)PrStage1ScorerDirectBuildStage1EventTable801C8660(directReplay);
    }
    StoreStage1DirectPortReplayBufferRestoreResult(
        directReplay,
        state.acceptedProducerReplayBuffer);
    if (directSpecialSetup.restore.scriptedWriterResetRequired) {
        state.acceptedProducerScriptedWriter =
            Stage1AcceptedProducerScriptedWriterRuntime{};
    }
}

#include "pr_stage1_scorer_host.inl"

}  // namespace PrScn1
