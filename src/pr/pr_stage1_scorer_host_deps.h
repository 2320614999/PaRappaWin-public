#pragma once

#include "pr_game_context.h"
#include "pr_stage_runner.h"
#include "pr_stage_runner_direct.h"
#include "pr_stage1_overlay_parser.h"
#include "pr_stage1_scorer_host.h"

namespace PrScn1 {

Stage1RunnerTimingRoots30 ResolveStage1RunnerTimingRoots30(
    const PrStageRunner& runner);
bool TryResolveStage1RuntimeDirectTick96(
    const Stage1NumericRuntimeState& state,
    int& outTick96);
bool IsStage1NumericRuntimeGateActive(PrGameContext& ctx, const PrStageRunner& runner);
void ResetStage1FormalLifecycleRuntime();
void ResetStage1TerminalFormalLifecycleSnapshot();
bool CopyStage1FormalLifecycleSnapshot(PrStage1FormalLifecycleSnapshot& out);
bool CopyStage1TerminalFormalLifecycleSnapshot(PrStage1FormalLifecycleSnapshot& out);
struct Stage1EventStreamFrameUpdate9094Result {
    bool ran = false;
    PrStageRunnerDirectFrameUpdate9094Input runnerInput{};
    PrStageRunnerDirectFrameUpdate9094Result frameUpdate{};
    PrStageRunnerDirectCtx0FinalFlagsSnapshot801C9094 ctx0FinalFlags{};
};
bool PrimeStage1FormalLifecycleRuntime(
    PrGameContext& ctx,
    const PrStageRunner& runner,
    const Stage1FormalLifecycleFrameInputs& inputs);
void LatchStage1TerminalFormalLifecycleSnapshot();
Stage1EventStreamFrameUpdate9094Result RunStage1EventStreamFrameUpdate9094(
    const PrStage1OverlayData& data,
    int tick96,
    uint32_t queryFrame,
    bool sameFrameCtxKnown,
    const PrStageRunnerDirectContext801C9094& sameFrameCtx,
    bool sameFrameGlobalsKnown,
    const PrStageRunnerDirectGlobals801C9094& sameFrameGlobals,
    bool allowSameQueryRefresh = false,
    bool clearTerminalTailPulse = false);
bool ConsumeStage1DirectFrameClearTerminalTailPulse(const PrStage1OverlayData& data,
                                                    int tick96,
                                                    uint32_t queryFrame);
bool ResolveStage1DirectBusyGate24BF4(
    const Stage1NumericRuntimeState& state,
    int tick96);
uint16_t ResolveStage1AcceptedProducerControlMask18(uint16_t rawMask);
uint8_t ResolveStage1AcceptedProducerClassToken20(uint16_t controlMask18);
uint32_t ResolveStage1GameplayRailAcceptedSourceCellId(size_t rowIndex,
                                                       uint8_t classToken,
                                                       uint16_t sourceCellCursor);
struct Stage1RunnerTailGateLateBranchSnapshot {
    bool tailDispatchFamilyActive = false;
    uint8_t tailDispatchFamilyStream = 0u;
    uint8_t activeDispatchStream = 0u;
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
Stage1RunnerTailGateLateBranchSnapshot
ResolveStage1RunnerTailGateLateBranchSnapshot();
void UpdateStage1AcceptedProducerGateCarrierRuntime(
    const PrGameContext& ctx,
    Stage1NumericRuntimeState& state,
    int tick96);
uint16_t ResolveStage1RightRankDescriptorFlagWord(
    const Stage1NumericRuntimeState& state);
bool IsStage1LateBranchQualifiedBridgeProbe(
    const Stage1NumericRuntimeState::SharedAcceptedProducerBoundaryProbeRuntime& probe);
}  // namespace PrScn1
