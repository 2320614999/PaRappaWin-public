#pragma once

#include "pr_scn1.h"
#include "pr_stage1_scorer_direct.h"
#include "pr_stage_payload_bank_direct.h"
#include "pr_stage_runner_direct.h"

#include <array>
#include <cstddef>
#include <cstdint>

struct PrGameContext;
class PrStageRunner;
struct PrStageEventDirectStage1FrameInput;
struct PrStage1RuntimeSlotsDirectFollowUpFacts801CBFDC;

namespace PrScn1 {

// Stage1 COMOD1 seeds `dword_800943C0` to the runtime overlay table at
// `801D2F04`, whose first three entries are {0,28,29}. The historical
// bucket30/bucket31 names refer to owner roles, not literal
// `tick96 % 384 / 12` bucket indices.
inline constexpr std::array<uint8_t, 3> kStage1GameplayCadenceBuckets = {0u, 28u, 29u};
inline constexpr uint8_t kStage1HistoricalBucket31ConsumerCadenceBucket =
    kStage1GameplayCadenceBuckets[2];
inline constexpr uint8_t kStage1LiteralPageClear14BDCBucket = 31u;
inline constexpr size_t kStage1TimingTemplateSlotCount = 8u;
inline constexpr size_t kStage1AcceptedProducerSourceGroupCursorEntryCount = 128u;
inline constexpr size_t kStage1AcceptedProducerReplayBufferCapacity = 600u;
inline constexpr uint8_t kStage1InitialRightRankActiveRow = 1u;
inline constexpr uint32_t kStage1FormalLifecycleAwfulFailHoldFrames = 0u;

struct Stage1RunnerTimingRoots30 {
    int runnerFrame = -1;
    uint32_t queryFrame = 0u;
    int tick96 = -1;
};

struct Stage1NumericRuntimeTimecodeInput801C7560 {
    int32_t sceneEntryField348TicksPerMinute = 0;
    int32_t sceneEntryField352FallbackTickAdvance = 0;
    int32_t sceneEntryField356TickOffset = 0;
    int32_t sceneEntryField360HalfSource = 0;
    bool sceneEntryField196TimeBaseKnown = false;
    int32_t sceneEntryField196TimeBaseA7A4 = 0;
};

struct Stage1RunnerTailGateFrameInputs {
    bool bucket0Ctx118WritePulse = false;
    uint32_t bucket0Ctx118WritePulseQueryFrame = 0u;
    uint8_t bucket0Ctx118WritePulseRightRankRow = 0u;
    bool bucket0Ctx118WritePulseCallWindowOpen = false;
    uint16_t bucket0Ctx118WritePulseDescriptorFlags = 0u;
    bool bucket0Ctx118WritePulseBusyGateActive = false;
    bool bucket0Ctx118WritePulseDescriptorBit8ConsumeGate = false;
    bool bucket0Ctx118WritePulseDescriptorBit10ShortCircuitGate = false;
    bool bucket0Ctx118WritePulseShortCircuitedByBit10 = false;
    bool bucket0Ctx118WritePulseConsumeGateBit8 = false;
    bool bucket0Ctx118WritePulseCtx54PermitInput = false;
    bool bucket0Ctx118WritePulseCtx54PermitOutput = false;
    bool bucket0Ctx118WritePulseReturnGate144B8Called = false;
    bool bucket0Ctx118WritePulseReturnGate144B8Result = false;
    bool bucket0Ctx118WritePulseReturnGate144B8Row3NoInputBranch = false;
    bool bucket0Ctx118WritePulseReturnGate144B8Row3TieCarryBranch = false;
    bool bucket0Ctx118WritePulseReturnGate144B8Row0TieCarryBranch = false;
    bool bucket0Ctx118WritePulseReturnGate144B8NonZeroRowBlocked = false;
    uint16_t bucket0Ctx118WritePulseReturnGate144B8NoInputCounterInput = 0u;
    uint16_t bucket0Ctx118WritePulseReturnGate144B8TieCarryLatchInput = 0u;
    uint16_t bucket0Ctx118WritePulseReturnGate144B8Ctx6AConsumerGateInput = 0u;
    uint16_t bucket0Ctx118WritePulseReturnGate144B8NoInputCounterOutput = 0u;
    uint16_t bucket0Ctx118WritePulseReturnGate144B8TieCarryLatchOutput = 0u;
    bool frameUpdate801C9094CtxKnown = false;
    uint32_t frameUpdate801C9094QueryFrame = 0u;
    uint16_t frameUpdate801C9094Ctx76 = 0u;
    uint16_t frameUpdate801C9094Ctx78 = 0u;
    bool frameUpdate801C9094CtxFlags00InputKnown = false;
    uint32_t frameUpdate801C9094CtxFlags00Input = 0u;
    bool frameUpdate801C9094CtxFlag40Input = false;
    bool frameUpdate801C9094CtxFlag40SourceBucket0 = false;
    uint32_t frameUpdate801C9094CtxFlag40SourceQueryFrame = 0u;
    uint8_t frameUpdate801C9094CtxFlag40SourceRightRankRow = 0u;
    bool frameUpdate801C9094CtxFlag40SourceCallWindowOpen = false;
    uint16_t frameUpdate801C9094CtxFlag40SourceDescriptorFlags = 0u;
    bool frameUpdate801C9094LastCtxFlag40InputKnown = false;
    uint32_t frameUpdate801C9094LastCtxFlag40InputQueryFrame = 0u;
    uint32_t frameUpdate801C9094LastCtxFlag40InputFlags00 = 0u;
    bool frameUpdate801C9094LastCtxFlag40SourceBucket0 = false;
    uint32_t frameUpdate801C9094LastCtxFlag40SourceQueryFrame = 0u;
    uint8_t frameUpdate801C9094LastCtxFlag40SourceRightRankRow = 0u;
    bool frameUpdate801C9094LastCtxFlag40SourceCallWindowOpen = false;
    uint16_t frameUpdate801C9094LastCtxFlag40SourceDescriptorFlags = 0u;
    bool frameUpdate801C9094Ed1CProducerKnown = false;
    uint16_t frameUpdate801C9094Ed1CEventStreamFlag = 0u;
    bool frameUpdate801C9094Ed1CFlagDescriptorValid = false;
    bool frameUpdate801C9094Ed1CActiveFlagStreamValid = false;
    uint32_t frameUpdate801C9094Ed1CActiveFlagStreamIndex = 0u;
    uint32_t frameUpdate801C9094Ed1CActiveFlagStreamCount = 0u;
    bool frameUpdate801C9094Ed1CActiveFlagStreamDueKnown = false;
    int32_t frameUpdate801C9094Ed1CActiveFlagStreamDueFrame = 0;
    int32_t frameUpdate801C9094Ed1CActiveFlagStreamDueDelta = 0;
    uint32_t frameUpdate801C9094Ed1CActiveFlagStreamFlags04 = 0u;
    bool frameUpdate801C9094Ed1CActiveFlagStreamFlag80 = false;
    bool frameUpdate801C9094Ed1CConsumedFlagStreamEvent = false;
    bool frameUpdate801C9094Ed1CProduced = false;
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
    bool eventStreamFlagLastRunnerClearKnown = false;
    uint8_t eventStreamFlagLastRunnerClearReason = 0u;
    uint32_t eventStreamFlagLastRunnerClearQueryFrame = 0u;
    uint32_t eventStreamFlagLastRunnerClearScriptFrame = 0u;
    uint16_t eventStreamFlagLastRunnerClearInputFlag = 0u;
    uint16_t eventStreamFlagLastRunnerClearOutputFlag = 0u;
    uint32_t eventStreamFlagLastRunnerClearInputCtxFlags00 = 0u;
    uint32_t eventStreamFlagLastRunnerClearOutputCtxFlags00 = 0u;
    bool eventStreamFlagLastRunnerClearInputEd1C = false;
    bool eventStreamFlagLastRunnerClearOutputEd1C = false;
    uint16_t eventStreamFlagLastRunnerClearWord4E = 0u;
    bool lateBranchTailDispatchFamilyActive = false;
    uint8_t lateBranchTailDispatchFamilyStream = 0u;
    uint8_t lateBranchActiveDispatchStream = 0u;
};

struct Stage1FormalLifecycleFrameInputs {
    bool introTransitionActive = false;
    bool lateBranchFlag40Active = false;
    uint8_t lateBranchSelectedStream = 0u;
    uint8_t lateBranchActiveDispatchStream = 0u;
    bool lateBranchFlag100BlocksWaitActive = false;
    uint8_t lateBranchFlag100SourceStream = 0u;
    bool lateBranchFirstFlag100PulseKnown = false;
    uint32_t lateBranchFirstFlag100PulseQueryFrame = 0u;
    uint32_t lateBranchFirstFlag100PulseScriptFrame = 0u;
    uint8_t lateBranchFirstFlag100PulseSourceStream = 0u;
    uint8_t lateBranchFirstFlag100PulseReason = 0u;
    uint32_t lateBranchScriptFrame = 0u;
    bool lateBranchClearTerminalTailPulseInput = false;
    bool lateBranchClearTerminalTailPulseArmed = false;
    bool lateBranchClearTerminalTailPulseBlockedAlreadyArmed = false;
    bool lateBranchClearTerminalTailPulseBlockedActiveDispatch = false;
    bool lateBranchClearTerminalTailPulseBlockedPendingMismatch = false;
    uint8_t lateBranchClearTerminalTailPulseStream = 0u;
    bool lateBranchClearTerminalTailLatchSetKnown = false;
    uint32_t lateBranchClearTerminalTailLatchSetQueryFrame = 0u;
    uint32_t lateBranchClearTerminalTailLatchSetScriptFrame = 0u;
    uint8_t lateBranchClearTerminalTailLatchSetRightRankRow = 0u;
    uint8_t lateBranchClearTerminalTailLatchSetCurrentMode = 0u;
    uint8_t lateBranchClearTerminalTailLatchSetStream = 0u;
    bool lateBranchClearTerminalBranchTriggerAttempted = false;
    bool lateBranchClearTerminalBranchTriggerAccepted = false;
    uint32_t lateBranchClearTerminalBranchTriggerScriptFrame = 0u;
    uint8_t lateBranchClearTerminalBranchTriggerRightRankRow = 0u;
    uint8_t lateBranchClearTerminalBranchTriggerCurrentMode = 0u;
    bool lateBranchClearTerminalBranchTriggerBlockedConsumed = false;
    bool lateBranchClearTerminalBranchTriggerBlockedArmed = false;
    bool lateBranchClearTerminalBranchTriggerBlockedFlagNotOne = false;
    bool lateBranchClearTerminalBranchTriggerBlockedRow = false;
    bool lateBranchClearTerminalBranchTriggerBlockedStreamMissing = false;
    bool lateBranchClearTerminalBranchTriggerBlockedCursorDone = false;
    bool lateBranchClearTerminalBranchTriggerBlockedEventNotDue = false;
    bool lateBranchClearTerminalBranchTriggerBlockedMissingFlag80 = false;
    uint16_t lateBranchClearTerminalBranchTriggerStreamFlag = 0u;
    uint32_t lateBranchClearTerminalBranchTriggerAttemptCount = 0u;
    uint32_t lateBranchClearTerminalBranchTriggerAcceptedCount = 0u;
    uint32_t lateBranchClearTerminalBranchTriggerEligibleCount = 0u;
    bool lateBranchClearTerminalBranchTriggerAcceptedKnown = false;
    uint32_t lateBranchClearTerminalBranchTriggerAcceptedQueryFrame = 0u;
    uint32_t lateBranchClearTerminalBranchTriggerAcceptedScriptFrame = 0u;
    uint8_t lateBranchClearTerminalBranchTriggerAcceptedRightRankRow = 0u;
    uint8_t lateBranchClearTerminalBranchTriggerAcceptedCurrentMode = 0u;
    uint16_t lateBranchClearTerminalBranchTriggerAcceptedStreamFlag = 0u;
    uint32_t lateBranchClearTerminalBranchTriggerAcceptedStream1Cursor = 0u;
    uint32_t lateBranchClearTerminalBranchTriggerAcceptedStream1Count = 0u;
    bool lateBranchClearTerminalBranchTriggerAcceptedStream1DueKnown = false;
    uint32_t lateBranchClearTerminalBranchTriggerAcceptedStream1DueFrame = 0u;
    int32_t lateBranchClearTerminalBranchTriggerAcceptedStream1DueDelta = 0;
    uint32_t lateBranchClearTerminalBranchTriggerAcceptedStream1BaseFrame = 0u;
    uint32_t lateBranchClearTerminalBranchTriggerAcceptedStream1AbsDueFrame = 0u;
    int32_t lateBranchClearTerminalBranchTriggerAcceptedStream1AbsDueDelta = 0;
    uint32_t lateBranchClearTerminalBranchTriggerAcceptedStream1PsxAddr = 0u;
    uint32_t lateBranchClearTerminalBranchTriggerAcceptedStream1Flags04 = 0u;
    uint8_t lateBranchClearTerminalBranchTriggerAcceptedStream1Byte29 = 0u;
    uint8_t lateBranchClearTerminalBranchTriggerAcceptedStream1Byte30 = 0u;
    uint32_t lateBranchClearTerminalBranchTriggerBlockedFlagNotOneCount = 0u;
    uint32_t lateBranchClearTerminalBranchTriggerBlockedRowCount = 0u;
    uint32_t lateBranchClearTerminalBranchTriggerBlockedFlagAndRowCount = 0u;
    uint32_t lateBranchClearTerminalBranchTriggerBlockedConsumedCount = 0u;
    uint32_t lateBranchClearTerminalBranchTriggerBlockedArmedCount = 0u;
    uint32_t lateBranchClearTerminalBranchTriggerBlockedStreamMissingCount = 0u;
    uint32_t lateBranchClearTerminalBranchTriggerBlockedCursorDoneCount = 0u;
    uint32_t lateBranchClearTerminalBranchTriggerBlockedEventNotDueCount = 0u;
    uint32_t lateBranchClearTerminalBranchTriggerBlockedMissingFlag80Count = 0u;
    bool lateBranchFirstClearTerminalBranchTriggerKnown = false;
    uint32_t lateBranchFirstClearTerminalBranchTriggerScriptFrame = 0u;
    uint8_t lateBranchFirstClearTerminalBranchTriggerRightRankRow = 0u;
    uint8_t lateBranchFirstClearTerminalBranchTriggerCurrentMode = 0u;
    uint16_t lateBranchFirstClearTerminalBranchTriggerStreamFlag = 0u;
    bool lateBranchFirstClearTerminalBranchTriggerEligibleKnown = false;
    uint32_t lateBranchFirstClearTerminalBranchTriggerEligibleScriptFrame = 0u;
    uint8_t lateBranchFirstClearTerminalBranchTriggerEligibleRightRankRow = 0u;
    uint8_t lateBranchFirstClearTerminalBranchTriggerEligibleCurrentMode = 0u;
    bool lateBranchFirstClearTerminalBranchTriggerEligibleStream1Known = false;
    uint32_t lateBranchFirstClearTerminalBranchTriggerEligibleStream1Cursor = 0u;
    uint32_t lateBranchFirstClearTerminalBranchTriggerEligibleStream1Count = 0u;
    bool lateBranchFirstClearTerminalBranchTriggerEligibleStream1DueKnown = false;
    uint32_t lateBranchFirstClearTerminalBranchTriggerEligibleStream1DueFrame = 0u;
    int32_t lateBranchFirstClearTerminalBranchTriggerEligibleStream1DueDelta = 0;
    uint32_t lateBranchFirstClearTerminalBranchTriggerEligibleStream1BaseFrame = 0u;
    uint32_t lateBranchFirstClearTerminalBranchTriggerEligibleStream1AbsDueFrame = 0u;
    int32_t lateBranchFirstClearTerminalBranchTriggerEligibleStream1AbsDueDelta = 0;
    uint32_t lateBranchFirstClearTerminalBranchTriggerEligibleStream1Flags04 = 0u;
    uint8_t lateBranchFirstClearTerminalBranchTriggerEligibleStream1Byte29 = 0u;
    uint8_t lateBranchFirstClearTerminalBranchTriggerEligibleStream1Byte30 = 0u;
    bool lateBranchFirstClearTerminalBranchTriggerEligibleStream1NextFlag80SearchKnown = false;
    bool lateBranchFirstClearTerminalBranchTriggerEligibleStream1NextFlag80Known = false;
    uint32_t lateBranchFirstClearTerminalBranchTriggerEligibleStream1NextFlag80Cursor = 0u;
    uint32_t lateBranchFirstClearTerminalBranchTriggerEligibleStream1NextFlag80Frame = 0u;
    int32_t lateBranchFirstClearTerminalBranchTriggerEligibleStream1NextFlag80Delta = 0;
    uint32_t lateBranchFirstClearTerminalBranchTriggerEligibleStream1NextFlag80BaseFrame = 0u;
    uint32_t lateBranchFirstClearTerminalBranchTriggerEligibleStream1NextFlag80AbsFrame = 0u;
    int32_t lateBranchFirstClearTerminalBranchTriggerEligibleStream1NextFlag80AbsDelta = 0;
    uint32_t lateBranchFirstClearTerminalBranchTriggerEligibleStream1NextFlag80Flags04 = 0u;
    uint8_t lateBranchFirstClearTerminalBranchTriggerEligibleStream1NextFlag80Byte29 = 0u;
    uint8_t lateBranchFirstClearTerminalBranchTriggerEligibleStream1NextFlag80Byte30 = 0u;
    uint32_t lateBranchClearTerminalBranchTriggerStream1Cursor = 0u;
    uint32_t lateBranchClearTerminalBranchTriggerStream1Count = 0u;
    bool lateBranchClearTerminalBranchTriggerStream1DueKnown = false;
    uint32_t lateBranchClearTerminalBranchTriggerStream1DueFrame = 0u;
    int32_t lateBranchClearTerminalBranchTriggerStream1DueDelta = 0;
    uint32_t lateBranchClearTerminalBranchTriggerStream1BaseFrame = 0u;
    uint32_t lateBranchClearTerminalBranchTriggerStream1AbsDueFrame = 0u;
    int32_t lateBranchClearTerminalBranchTriggerStream1AbsDueDelta = 0;
    uint32_t lateBranchClearTerminalBranchTriggerStream1Flags04 = 0u;
    uint8_t lateBranchClearTerminalBranchTriggerStream1Byte29 = 0u;
    uint8_t lateBranchClearTerminalBranchTriggerStream1Byte30 = 0u;
    bool lateBranchFirstClearTerminalBranchTriggerEventNotDueKnown = false;
    uint32_t lateBranchFirstClearTerminalBranchTriggerEventNotDueScriptFrame = 0u;
    uint32_t lateBranchFirstClearTerminalBranchTriggerEventNotDueCursor = 0u;
    uint32_t lateBranchFirstClearTerminalBranchTriggerEventNotDueFrame = 0u;
    int32_t lateBranchFirstClearTerminalBranchTriggerEventNotDueDelta = 0;
    uint32_t lateBranchFirstClearTerminalBranchTriggerEventNotDueBaseFrame = 0u;
    uint32_t lateBranchFirstClearTerminalBranchTriggerEventNotDueAbsFrame = 0u;
    int32_t lateBranchFirstClearTerminalBranchTriggerEventNotDueAbsDelta = 0;
    uint32_t lateBranchFirstClearTerminalBranchTriggerEventNotDueFlags04 = 0u;
    bool lateBranchFirstClearTerminalBranchTriggerMissingFlag80Known = false;
    uint32_t lateBranchFirstClearTerminalBranchTriggerMissingFlag80ScriptFrame = 0u;
    uint32_t lateBranchFirstClearTerminalBranchTriggerMissingFlag80Cursor = 0u;
    uint32_t lateBranchFirstClearTerminalBranchTriggerMissingFlag80Frame = 0u;
    int32_t lateBranchFirstClearTerminalBranchTriggerMissingFlag80Delta = 0;
    uint32_t lateBranchFirstClearTerminalBranchTriggerMissingFlag80BaseFrame = 0u;
    uint32_t lateBranchFirstClearTerminalBranchTriggerMissingFlag80AbsFrame = 0u;
    int32_t lateBranchFirstClearTerminalBranchTriggerMissingFlag80AbsDelta = 0;
    uint32_t lateBranchFirstClearTerminalBranchTriggerMissingFlag80Flags04 = 0u;
    bool lateBranchClearTerminalTailServiceLatch = false;
    uint32_t lateBranchClearTerminalTailServiceScriptFrame = 0u;
    uint8_t lateBranchClearTerminalTailServiceRightRankRow = 0u;
    uint8_t lateBranchClearTerminalTailServiceCurrentMode = 0u;
    uint8_t lateBranchClearTerminalTailServiceStream = 0u;
    uint32_t lateBranchPendingRatingBranchSeq = 0u;
    uint32_t lateBranchConsumedRatingBranchSeq = 0u;
    bool lateBranchClearTerminalTailArmed = false;
    bool lateBranchClearTerminalTailDispatchActive = false;
    uint8_t lateBranchClearTerminalTailStream = 0u;
    uint32_t lateBranchActiveDispatchStartScriptFrame = 0u;
    uint32_t lateBranchActiveDispatchTerminalEndLocalFrame = 0u;
    bool lateBranchActiveDispatchTerminalPulseEmitted = false;
    bool recordsModeDAActive = false;
    bool recordsModeDAEqualsOne = false;
    bool stageStatus166ACKnown = false;
    uint8_t stageStatus166AC = 0u;
    bool numericActive = false;
    uint8_t numericRightRankActiveRow = kStage1InitialRightRankActiveRow;
    bool runnerTailDirectKnown = false;
    bool runnerTailCleanupRequiredKnown = false;
    bool runnerTailCleanupRequired = false;
    uint8_t runnerTailCleanupDrainFrameCount = 0u;
    bool runnerTailFrameExit1A3B8Known = false;
    bool runnerTailFrameExit1A3B8Taken = false;
};

struct Stage1AcceptedProducerScriptedWriterRuntime {
    bool active = false;
    // Debug shell for the replay/sample-side materialized slot. The
    // authoritative `901C0/901BC` owner lane now lives in the shared replay
    // buffer runtime; this sidecar only keeps the current sampled payload
    // visible to observer/handoff surfaces.
    uint32_t dueTick96EEF8 = 0;
    uint32_t classMaskEEFC = 0;
};

struct Stage1AcceptedProducerReplayBackupRuntime {
    bool valid = false;
    uint32_t publishedCount901BC = 0;
    bool replayMirrorFullBackingKnown8008EEF8 = false;
    bool prevGrade92F40Valid = false;
    uint32_t prevGrade92F40 = 1;
    std::array<uint32_t, kStage1AcceptedProducerReplayBufferCapacity> tick96EEF8{};
    std::array<uint32_t, kStage1AcceptedProducerReplayBufferCapacity> classMaskEEFC{};
};

struct Stage1AcceptedReplayPayloadRestoreGateRuntime {
    bool observed = false;
    bool livePayloadKnown = false;
    bool helperGap = false;
    bool statusBankKnown80092F1D = false;
    uint32_t byteCount = 0u;
    bool byteCountExact = false;
    uint32_t recordsRequiredBytes = 0u;
    bool recordsCovered = false;
    bool writerKnown = false;
    bool replayPayloadBackingProvenanceKnown80092F5C = false;
    PrStagePayloadBankDirect::ReplayPayloadBackingProvenance80092F5C
        replayPayloadBackingProvenance80092F5C =
            PrStagePayloadBankDirect::
                ReplayPayloadBackingProvenance80092F5C::Unknown;
    uint32_t lastWriterFunction = 0u;
    bool wrote800164B4 = false;
    bool wrote8001635C = false;
    bool publishedCountKnown = false;
    uint32_t publishedCount = 0u;
    bool publishedCountInRange = false;
    bool backupValid = false;
    uint32_t lastFaultAddress = 0u;
};

struct Stage1FormulaAccumulatorWritebackState {
    int32_t writebackValue = 0;
    bool clampActive = false;
};

enum class Stage1SteadySfxDispatchCueId : uint8_t {
    None = 0u,
    Bucket30Verdict = 1u,
    Bucket30RowCommit = 2u,
    Bucket31Completion = 3u,
    // Keep literal `LABEL_103` direct-94400 dispatches distinct from the
    // committed row-write cue so runtime probes can accept no-commit fallthroughs.
    Bucket30DirectCue94400 = 4u,
};

struct Stage1SteadySfxDispatchRuntime {
    bool known = false;
    Stage1SteadySfxDispatchCueId cueId = Stage1SteadySfxDispatchCueId::None;
    uint8_t tableSlot = 0xFFu;
    bool firedBucket30 = false;
    bool firedBucket31Completion = false;
    bool callbackSiteReached = false;
};

struct Stage1FormalScorerBranchState {
    bool countKnown = false;
    int32_t countValue = 0;
    bool pairBonusKnown = false;
    int32_t pairBonusValue = 0;
    bool spillPenaltyKnown = false;
    int32_t spillPenaltyValue = 0;
    bool descriptorSubdeltaKnown = false;
    int32_t descriptorSubdeltaValue = 0;
    bool readerPageOrdinalKnown = false;
    int32_t readerPageOrdinalValue = 0;
    bool requiredMaskKnown = false;
    int32_t requiredMaskValue = 0;
    bool unionMaskKnown = false;
    int32_t unionMaskValue = 0;
    bool anchorSlotIndexKnown = false;
    int32_t anchorSlotIndexValue = 0;
    bool requiredClassTokenKnown = false;
    int32_t requiredClassTokenValue = 0;
    bool anchorSlotClassTokenKnown = false;
    int32_t anchorSlotClassTokenValue = 0;
    bool anchorSlotOccupiedKnown = false;
    int32_t anchorSlotOccupiedValue = 0;
    bool anchorSlotAcceptedMaskKnown = false;
    uint32_t anchorSlotAcceptedMaskValue = 0;
    bool anchorSlotPayloadKnown = false;
    uint32_t anchorSlotPayloadValue = 0;
    bool anchorPageOccupiedSlotBitsKnown = false;
    uint32_t anchorPageOccupiedSlotBitsValue = 0;
    bool anchorPageRequiredMaskSlotBitsKnown = false;
    uint32_t anchorPageRequiredMaskSlotBitsValue = 0;
    bool anchorPageRequiredOccupiedSlotBitsKnown = false;
    uint32_t anchorPageRequiredOccupiedSlotBitsValue = 0;
    bool anchorClassMatchKnown = false;
    int32_t anchorClassMatchValue = 0;
};

struct Stage1FormalScorerRuntimeState {
    bool currentBranchKnown = false;
    bool currentBranchSpecial = false;
    bool additiveTermKnown = false;
    int32_t additiveTermValue = 0;
    bool acceptedCountKnown = false;
    int32_t acceptedCountValue = 0;
    bool currentLookbackPageCountKnown = false;
    int32_t currentLookbackPageCountValue = 0;
    bool overflowActiveKnown = false;
    bool overflowActive = false;
    Stage1FormalScorerBranchState defaultBranch{};
    Stage1FormalScorerBranchState specialBranch{};
};

struct Stage1FormalScorerCommitRuntimeState {
    bool currentBranchKnown = false;
    bool currentBranchSpecial = false;
    bool additiveTermKnown = false;
    int32_t additiveTermValue = 0;
    bool acceptedCountKnown = false;
    int32_t acceptedCountValue = 0;
    bool currentLookbackPageCountKnown = false;
    int32_t currentLookbackPageCountValue = 0;
    bool overflowActiveKnown = false;
    bool overflowActive = false;
    Stage1FormalScorerBranchState activeBranch{};
    bool commitTermKnown = false;
    int32_t commitTermValue = 0;
    bool scoreWritebackKnown = false;
    int32_t scoreWritebackValue = 0;
    bool clampActiveKnown = false;
    bool clampActive = false;
};

struct Stage1BucketCadenceRuntime {
    // Stage1 steady gameplay updates on the PSX 12-tick cadence triplet.
    uint8_t currentBucket = 0xFF;
    uint8_t previousBucket = 0xFF;
    uint32_t bucketAdvanceCount = 0;
    bool bucketChanged = false;
    bool bucket0Advanced = false;
    bool bucket30Advanced = false;
    bool bucket31Advanced = false;
    bool ctxFlagTickAdvanceKnown801C9094 = false;
    bool ctxFlagTickAdvance801C9094 = false;
};

struct Stage1NumericRuntimeState {
    struct DescriptorRowView {
        bool available = false;
        uint8_t lessonId = 0;
        uint8_t anchorSlotIndex = 0;
        uint8_t requiredClassToken = 0;
        uint8_t penaltyWeight = 0;
        int16_t finalResolutionThreshold = 0;
        uint32_t requiredMask = 0;
        uint8_t defaultSelectorByte0 = 0;
        uint8_t defaultSelectorByte1 = 0;
        int16_t defaultLookbackPageCount = 0;
        uint16_t defaultFlagWord = 0;
        uint8_t substate1SelectorByte0 = 0;
        uint8_t substate1SelectorByte1 = 0;
        int16_t substate1LookbackPageCount = 0;
        uint16_t substate1FlagWord = 0;
    };

    struct PageRecordSlotProvenanceRuntime {
        bool writerKnown = false;
        uint32_t producerFunction = 0u;
        uint32_t writeQueryFrame = 0u;
        int32_t writePageOrdinal38 = -1;
        uint8_t recordSlot24 = 0u;
        uint8_t recordRemainder24 = 0u;
        bool writePageOwnerKnown = false;
        uint16_t writePageOwnerOrdinal1Based = 0u;
        bool writePageOwnerLastSetKnown = false;
        uint32_t writePageOwnerLastSetQueryFrame = 0u;
        uint16_t writePageOwnerLastSetTargetOrdinal1Based = 0u;
        bool writePageOwnerLastSetTick96Known = false;
        int32_t writePageOwnerLastSetTick96 = 0;
        uint16_t writePageOwnerLastSetPhase384 = 0u;
        uint8_t writePageOwnerLastSetBucket31 = 0u;
        bool sameTickPageClearApplied14BDC = false;
        bool sameTickPageClearTargetKnown14BDC = false;
        uint16_t sameTickPageClearTargetOrdinal1Based14BDC = 0u;
        bool acceptedTick96Known = false;
        int32_t acceptedTick96 = 0;
        uint16_t phase384 = 0u;
        uint8_t timingTemplateSlot48 = 0u;
        uint16_t sourceCellCursor = 0u;
        uint32_t acceptedMask = 0u;
        uint16_t pageCompanion = 0u;
        uint16_t occupiedCount = 0u;
        uint32_t sourceCellPtr = 0u;
    };

    struct PageRecordMirrorPage {
        std::array<uint8_t, 384> raw{};
        std::array<PageRecordSlotProvenanceRuntime,
                   kPrStage1ScorerDirectSlotsPerPage>
            slotProvenance{};
    };

    struct PageRecordMirrorRuntime {
        bool initialized = false;
        int currentPageOrdinal = -1;
        std::array<PageRecordMirrorPage, 4> pages{};
    };

    struct SharedAcceptedProducerRuntime {
        uint32_t acceptedContributionCount = 0;
        uint32_t recordedSplitCount = 0;
        uint32_t penaltySideSplitCount = 0;
        uint32_t aggregateAcceptedMask = 0;
    };

    struct SharedAcceptedProducerBoundaryProbeRuntime {
        uint16_t rawPressedMask = 0;
        uint16_t rawAcceptedMask = 0;
        bool initialPostKnown = false;
        uint32_t initialPostCtxInput18 = 0u;
        uint32_t initialPostPreviousInputMask801CCBB8 = 0u;
        uint32_t initialPostAcceptedMask9FF = 0u;
        bool initialPostAcceptedGateOpen = false;
        bool initialPostAcceptedMaskChanged = false;
        bool initialPostBackupCtx10FromCtx0C = false;
        bool initialPostCallAcceptedProducer14614 = false;
        bool frontDoorCurrentTickProbeDisabled = false;
        bool frontDoorCurrentTickSeedAllowed = false;
        bool directAcceptedRunCaptured = false;
        int32_t directAcceptedRunResultCode = -1;
        bool directAcceptedRunReplayAppendRan = false;
        bool directAcceptedRunWriteRan = false;
        int32_t directAcceptedRunWriteResultCode = -1;
        bool directAcceptedRunSelectorResolved = false;
        bool directAcceptedRunTimingTemplateKnown = false;
        uint32_t directAcceptedRunTimingTemplateAddress = 0;
        uint8_t directAcceptedRunTimingTemplateSlot48 = 0;
        uint8_t directAcceptedRunTimingTemplateState = 0;
        bool directAcceptedRunSourceCellValid = false;
        bool directAcceptedRunAcceptedTick96Known = false;
        int32_t directAcceptedRunAcceptedTick96 = 0;
        uint16_t directAcceptedRunPhase384 = 0;
        uint8_t directAcceptedRunRecordSlot24 = 0;
        uint8_t directAcceptedRunRecordRemainder24 = 0;
        uint16_t directAcceptedRunSourceCellCursor = 0;
        bool directAcceptedRunPageWriteApplied = false;
        int32_t directAcceptedRunWritePageOrdinal = -1;
        uint8_t directAcceptedRunPageRecordSlot = 0;
        uint16_t directInputDescriptorSubstate50 = 0;
        bool directInputLookaheadRowValid = false;
        uint8_t directInputLookaheadLessonId = 0;
        uint8_t directInputDefaultSelectorByte0 = 0;
        uint8_t directInputDefaultSelectorByte1 = 0;
        uint8_t directInputSubstate1SelectorByte0 = 0;
        uint8_t directInputSubstate1SelectorByte1 = 0;
        uint8_t directInputBranchSelectorByte0 = 0;
        uint8_t directInputBranchSelectorByte1 = 0;
        uint8_t classToken = 0;
        uint8_t selectorByte0 = 0;
        uint8_t selectorByte1 = 0;
        bool selectorAvailable = false;
        uint8_t timingTemplateState = 0;
        bool sourceCellGateActive = false;
        uint16_t sourceCellCursor = 0;
        bool materialized = false;
        bool carryReplayed = false;
        uint8_t split = 0;
    };

    struct AcceptedProducerCarrierRuntime {
        bool available = false;
        uint8_t controlWriterSourceKind = 0;
        uint16_t rawControlSample18 = 0;
        uint16_t controlMask18 = 0;
        uint8_t classToken20 = 0;
        uint8_t halfWindow34 = 0;
        uint8_t substate50 = 0;
        bool eventStreamFlagKnown = false;
        bool eventStreamFlagActive = false;
        bool eventStreamIdRawKnown = false;
        uint8_t eventStreamIdRaw = 0;
        bool busyGate24BF4Known = false;
        bool busyGate24BF4Active = false;
        bool acceptedGateKnown = false;
        bool acceptedGateActive = false;
        bool acceptedTick96Known = false;
        int32_t acceptedTick96 = 0;
        uint32_t acceptedTick96LastUpdateQueryFrame = 0u;
        uint8_t acceptedTick96LastUpdateSourceKind = 0u;
        uint16_t acceptedTick96LastUpdateControlMask18 = 0u;
        uint8_t acceptedTick96LastUpdateClassToken20 = 0u;
        bool acceptedTick96LastUpdateViaPostCtx10 = false;
        uint32_t acceptedTick96LastUpdateCtxInput18 = 0u;
        uint32_t acceptedTick96LastUpdatePreviousInputMask801CCBB8 = 0u;
        bool acceptedTick96LastUpdateAcceptedMaskChanged = false;
        bool carrierCurrentTickProbeDisabled = false;
        bool carrierCurrentTickSeedAllowed = false;
        uint16_t phase384 = 0;
        int writePageOrdinal38 = 0;
        uint8_t recordSlot24 = 0;
        uint8_t recordRemainder24 = 0;
        uint8_t timingTemplateSlot48 = 0;
        bool recordedSplit = false;
    };

    struct AcceptedProducerReplayBufferRuntime {
        // Process-start `80028590` executes once. This epoch flag survives
        // transient Stage1 host resets so re-entry cannot mint startup
        // provenance over an already-running replay mirror.
        bool startupZeroInitializationConsumed80028590 = false;
        PrStage1ScorerDirectReplayMirrorAuthority replayMirrorAuthority{};
        bool replayMirrorKnown8008EEF8 = false;
        bool replayMirrorStartupZeroAuthorityKnown80028590 = false;
        bool replayMirrorProducerKnown8008EEF8 = false;
        uint32_t replayMirrorProducerFunction = 0;
        bool replayMirrorByteCountKnown8008EEF8 = false;
        uint32_t replayMirrorKnownByteCount8008EEF8 = 0;
        bool replayMirrorFullBackingKnown8008EEF8 = false;
        uint32_t writeCount901C0 = 0;
        uint32_t publishedCount901BC = 0;
        std::array<uint32_t, kStage1AcceptedProducerReplayBufferCapacity> tick96EEF8{};
        std::array<uint32_t, kStage1AcceptedProducerReplayBufferCapacity> classMaskEEFC{};
    };

    struct RunnerCtx0Owner801C3640Runtime {
        bool snapshotAvailable = false;
        uint32_t queryFrame = 0u;
        int32_t tick96 = 0;
        bool baseKnown = false;
        uint32_t baseFlags = 0u;
        bool finalKnown = false;
        uint32_t finalFlags = 0u;
        bool missingFinalDirectOwner = true;
    };

    struct RunnerSameFrameCtxOwner801C9094Runtime {
        bool snapshotAvailable = false;
        uint32_t queryFrame = 0u;
        int32_t tick96 = 0;
        bool sameFrameCtxKnown = false;
        // `projected*` is a carrier for the direct helper's current output;
        // it is not authoritative until the missing owner flags below close.
        PrStageRunnerDirectContext801C9094 inputCtx{};
        PrStageRunnerDirectContext801C9094 projectedCtx{};
        bool ctxFlags00InputKnown = false;
        uint32_t ctxFlags00Input = 0u;
        bool ctxFlags00InputFlag40 = false;
        bool ctxFlags00InputFlag40SourceBucket0 = false;
        uint32_t ctxFlags00InputFlag40SourceQueryFrame = 0u;
        uint8_t ctxFlags00InputFlag40SourceRightRankRow = 0u;
        bool ctxFlags00InputFlag40SourceCallWindowOpen = false;
        uint16_t ctxFlags00InputFlag40SourceDescriptorFlags = 0u;
        bool ctxFlags00ProjectedKnown = false;
        uint32_t ctxFlags00Projected = 0u;
        bool ctxFlags00FinalKnown = false;
        uint32_t ctxFlags00Final = 0u;
        bool ctx54InputKnown = false;
        uint16_t ctx54Input = 0u;
        bool ctx54ProjectedKnown = false;
        uint16_t ctx54Projected = 0u;
        bool ctxFlags00MissingSameFrameOwner = true;
        bool ctxFlags00MissingFlagStreamFlags04 = true;
        bool ctxFlags00MissingIdStreamDescriptorState = true;
        bool ctxFlags00MissingCompactRailRecord = true;
        bool ed1CProducerKnown = false;
        uint16_t ed1CProducerEventStreamFlag = 0u;
        bool ed1CProducerFlagDescriptorValid = false;
        bool ed1CProducerActiveFlagStreamValid = false;
        uint32_t ed1CProducerActiveFlagStreamIndex = 0u;
        uint32_t ed1CProducerActiveFlagStreamCount = 0u;
        bool ed1CProducerActiveFlagStreamDueKnown = false;
        int32_t ed1CProducerActiveFlagStreamDueFrame = 0;
        int32_t ed1CProducerActiveFlagStreamDueDelta = 0;
        uint32_t ed1CProducerActiveFlagStreamFlags04 = 0u;
        bool ed1CProducerActiveFlagStreamFlag80 = false;
        bool ed1CProducerConsumedFlagStreamEvent = false;
        bool ed1CProducerProduced = false;
    };

    struct RunnerSameFrameGlobalsOwner801C9094Runtime {
        bool snapshotAvailable = false;
        uint32_t queryFrame = 0u;
        int32_t tick96 = 0;
        bool sameFrameGlobalsKnown = false;
        // Values are kept even when `*Known` is false so probes can see the
        // gap instead of silently treating zero/defaults as PSX truth.
        PrStageRunnerDirectGlobals801C9094 inputGlobals{};
        PrStageRunnerDirectGlobals801C9094 projectedGlobals{};
        bool timersECF8_ECFA_ECFC_ECFEKnown = false;
        int16_t word8008ECF8 = 0;
        int16_t word8008ECFA = 0;
        int16_t word8008ECFC = 0;
        int16_t word8008ECFE = 0;
        bool ed1CKnown = false;
        bool ed1C = false;
        bool d3054D3058Known = false;
        int32_t dword801D3054 = 0;
        int32_t dword801D3058 = 0;
        bool missingDirectOwner = true;
    };

    struct RunnerCompactLaneOwner801C9094Runtime {
        bool snapshotAvailable = false;
        uint32_t queryFrame = 0u;
        int32_t tick96 = 0;
        bool currentKnown = false;
        uint16_t word8A = 0u;
        int16_t word8C = -1;
        int16_t word8E = -1;
        uint16_t word90 = 0u;
        int16_t word9E = -1;
        int16_t wordA0 = -1;
        uint16_t wordA2 = 0u;
        bool missingCurrentDirectOwner = true;
        bool missingCompactRailRecord = true;
    };

    struct RunnerPostFrame7A60Runtime {
        bool known = false;
        uint32_t ctxFlags00 = 0u;
        bool word0Flags801C3640Known = false;
        uint32_t word0Flags801C3640 = 0u;
        bool word0Flags801C3640BaseKnown = false;
        uint32_t word0Flags801C3640Base = 0u;
        bool word0Flags801C3640FinalKnown = false;
        uint32_t word0Flags801C3640Final = 0u;
        bool word0Flags801C9094FinalKnown = false;
        uint32_t word0Flags801C9094Final = 0u;
        bool word0Flags801C9094RequiresSameFrameCtx = true;
        bool word0Flags801C9094RequiresSameFrameGlobals = true;
        bool word0Flags801C9094RequiresFlagStreamFlags04 = true;
        bool word0Flags801C9094RequiresIdStreamDescriptorState = true;
        bool word0Flags801C9094RequiresCompactRailRecord = true;
        bool frameUpdate9094ReturnEarlyAfterFlag40Reset = false;
        bool lastCtxFlag40InputKnown801C9094 = false;
        uint32_t lastCtxFlag40InputQueryFrame801C9094 = 0u;
        uint32_t lastCtxFlag40InputFlags00801C9094 = 0u;
        bool lastCtxFlag40SourceBucket0801C9094 = false;
        uint32_t lastCtxFlag40SourceQueryFrame801C9094 = 0u;
        uint8_t lastCtxFlag40SourceRightRankRow801C9094 = 0u;
        bool lastCtxFlag40SourceCallWindowOpen801C9094 = false;
        uint16_t lastCtxFlag40SourceDescriptorFlags801C9094 = 0u;
        RunnerCtx0Owner801C3640Runtime ctx0Owner801C3640{};
        RunnerSameFrameCtxOwner801C9094Runtime sameFrameCtx801C9094{};
        RunnerSameFrameGlobalsOwner801C9094Runtime sameFrameGlobals801C9094{};
        RunnerCompactLaneOwner801C9094Runtime compactLane801C9094{};
        uint32_t ctxInput18 = 0u;
        uint32_t previousInputMask801CCBB8 = 0u;
        uint32_t acceptedMask9FF = 0u;
        bool acceptedGateOpen = false;
        bool acceptedMaskChanged = false;
        bool backupCtx08FromCtx04 = false;
        bool backupCtx10FromCtx0C = false;
        bool callAcceptedProducer14614 = false;
        bool callAcceptedProducer94434 = false;
        bool callAcceptedProducer9443C = false;
        bool fixedBonus250Requested = false;
        bool continueMainLoop = true;
        bool writeCtx3C = false;
        int32_t ctx3CValue = 0;
        bool writeDword801CCBB8 = false;
        uint32_t dword801CCBB8Value = 0u;
    };

    struct RunnerMainLoopTail7A60Runtime {
        bool known = false;
        bool postFrameContinues = true;
        int32_t sceneEntryField352FallbackTickAdvance = 0;
        bool callWait15350 = false;
        int32_t wait15350Arg = 0;
        bool callWait35560 = false;
        int32_t wait35560Arg = 0;
        bool callPresentFrame = false;
        bool callContinuationGate1A7F8 = false;
        bool continuationGateKnown = false;
        bool continuationGateAllowsNextFrame = false;
        bool callFrameExit1A3B8 = false;
        bool frameExitKnown = false;
        bool frameExitTaken = false;
        bool callEarlyExit1A694 = false;
        bool callEarlyExit35838 = false;
        bool setWord59OnFrameExit = false;
        bool cleanupRequiredKnown = false;
        bool cleanupRequired = false;
        bool clearWord50 = false;
        bool clearWord42 = false;
        bool clearWord61 = false;
        uint8_t cleanupDrainFrameCount = 0u;
        bool finalCall1B120 = false;
        bool finalCall26FA4 = false;
        bool finalCall357D4Zero = false;
        bool finalCall1A694 = false;
    };

    struct RunnerTailHost7A60Runtime {
        bool known = false;
        uint32_t queryFrame = 0u;
        int32_t tick96 = 0;
        PrStageRunnerDirectActionTrace postFrameTrace{};
        PrStageRunnerDirectActionTrace mainLoopTailTrace{};
        bool requestXaSetFilter13 = false;
        uint8_t xaSetFilter13Arg = 0u;
        uint32_t xaSetFilter13RequestCount = 0u;
        bool xaSetFilter13LastRequestKnown = false;
        uint32_t xaSetFilter13LastRequestQueryFrame = 0u;
        int32_t xaSetFilter13LastRequestTick96 = 0;
        bool xaSetFilter13LastRequestFlag0200Pulse = false;
        uint8_t xaSetFilter13LastRequestRow = 0u;
        uint8_t xaSetFilter13LastRequestArg = 0u;
        bool requestFailCue943EC = false;
        bool requestAudioA4A4Arg1 = false;
    };

    struct SteadyInput7A60Runtime {
        bool known = false;
        bool replayMode52 = false;
        uint32_t heldMask = 0u;
        bool writeCtx18 = false;
        bool ctx18Known = false;
        uint32_t ctx18Value = 0u;
        bool writeCtx20 = false;
        bool ctx20Known = false;
        int32_t ctx20Value = 0;
        bool incrementReplayReadIndex901C0 = false;
        uint32_t replayReadIndex901C0After = 0u;
        bool writeCtx10CurrentTick = false;
        bool writeCtx10ReplayTick = false;
        int32_t ctx10ReplayTick = 0;
    };

    struct AcceptedInputSourceEventRuntime {
        bool available = false;
        uint32_t queryFrame = 0u;
        int32_t tick96 = 0;
        uint32_t heldMask = 0u;
        uint32_t ctx18Value = 0u;
        bool writeCtx10CurrentTick = false;
        bool postAcceptedGateOpen = false;
        bool postAcceptedMaskChanged = false;
        bool postBackupCtx10 = false;
    };

    struct RunnerTimecode801C7560Runtime {
        bool known = false;
        bool xaSectorBaselineKnown = false;
        int32_t xaSectorBaseline75 = 0;
        bool xaReadValueA7A4Known = false;
        bool gapMissingSceneEntryField196TimeBase = false;
        bool gapMissingStreamClock800493F4 = false;
        bool clockPoll8001A3C8Called = false;
        int32_t clockPoll8001A3C8Return = 0;
        bool clockPoll8001A3C8AcceptedByte = false;
        bool clockPoll8001A3C8Gap364D0 = false;
        bool clockPoll8001A3C8Gap363A4 = false;
        bool command8001A280Called = false;
        bool command8001A280Issued = false;
        bool command8001A280SkippedNonZeroWorkBase = false;
        bool command8001A280Gap49428 = false;
        int32_t command8001A280WorkBase = 0;
        uint32_t queryFrame = 0u;
        PrStageRunnerDirectTimecodeState801C7560 state{};
        PrStageRunnerDirectTimecodeResult801C7560 lastResult{};
        PrStageRunnerDirectTimecodeSnapshot801C7560 snapshot{};
    };

    struct AcceptedProducerLastRecordedPageWriteRuntime {
        bool available = false;
        int writePageOrdinal38 = -1;
        uint8_t recordSlot24 = 0;
        uint32_t acceptedMask = 0;
        uint16_t pageCompanion = 0;
    };

    struct FirstAcceptedCountClearRequiredOccupiedSlotRuntime {
        bool slotKnown = false;
        uint8_t slotIndex = 0u;
        bool writerKnown = false;
        uint32_t producerFunction = 0u;
        uint32_t writeQueryFrame = 0u;
        int32_t writePageOrdinal38 = -1;
        uint8_t recordSlot24 = 0u;
        uint8_t recordRemainder24 = 0u;
        bool writePageOwnerKnown = false;
        uint16_t writePageOwnerOrdinal1Based = 0u;
        bool writePageOwnerLastSetKnown = false;
        uint32_t writePageOwnerLastSetQueryFrame = 0u;
        uint16_t writePageOwnerLastSetTargetOrdinal1Based = 0u;
        bool writePageOwnerLastSetTick96Known = false;
        int32_t writePageOwnerLastSetTick96 = 0;
        uint16_t writePageOwnerLastSetPhase384 = 0u;
        uint8_t writePageOwnerLastSetBucket31 = 0u;
        bool sameTickPageClearApplied14BDC = false;
        bool sameTickPageClearTargetKnown14BDC = false;
        uint16_t sameTickPageClearTargetOrdinal1Based14BDC = 0u;
        bool acceptedTick96Known = false;
        int32_t acceptedTick96 = 0;
        uint16_t phase384 = 0u;
        uint8_t timingTemplateSlot48 = 0u;
        uint16_t sourceCellCursor = 0u;
        uint32_t acceptedMask = 0u;
        uint16_t pageCompanion = 0u;
        uint16_t occupiedCount = 0u;
        uint32_t sourceCellPtr = 0u;
    };

    struct AdditiveLaneRuntime {
        int32_t value = 0;
        bool clearPendingBucket31 = false;
        bool bucket30CommitVisible = false;
        int32_t bucket30CommitValue = 0;
    };

    struct DirectPortScorerRuntime {
        uint16_t acceptedCount91810 = 0;
        uint16_t recordedHitCount91812 = 0;
        uint16_t penaltySideCount91814 = 0;
        int32_t accumulator91816 = 0;
        int16_t additiveTerm91822 = 0;
        uint16_t recordCompanion91824 = 0;
        uint32_t aggregateAcceptedMask91808 = 0;
        uint32_t lastClearedAcceptedMask9180C = 0;
    };

    struct RightRankPhase1OwnerRuntime {
        int32_t baselineValue18 = 0;
        bool shortWindowLatch34Known = false;
        uint16_t shortWindowLatch34 = 0u;
        uint8_t cachedPhase1Classifier36 = 2u;
        bool phase1LatchArmed38 = false;
    };

    struct RightRank24F8CObserverRuntime {
        int32_t formalWritebackValue24 = 0;
        int32_t lastBaselineReseedQueryFrame = -1;
        int32_t snapshot18 = 0;
    };

    struct RightRankTieBreakerObserverRuntime {
        int32_t snapshot1A = 0;
    };

    struct RightRankBucketContextRuntime {
        bool ctx54Permit = false;
        bool ctx6AConsumerGate = false;
        bool ctx6AConsumerGateInitKnown = false;
        int32_t ctx6AConsumerGateInitQueryFrame = -1;
        uint16_t ctx6AConsumerGateInitValue = 0u;
        uint16_t ctx7A = 0u;
        uint16_t ctx74 = 0u;
        uint16_t transitionAnim18E = 0u;
    };

    struct RightRankHelperShadowRuntime {
        int32_t snapshot1C = 0;
        bool bucket0CallWindowOpen = false;
        bool bucket0Ctx118WritePulse = false;
        uint32_t bucket0Ctx118WritePulseQueryFrame = 0u;
        uint8_t bucket0Ctx118WritePulseRightRankRow = 0u;
        bool bucket0Ctx118WritePulseCallWindowOpen = false;
        uint16_t bucket0Ctx118WritePulseDescriptorFlags = 0u;
        bool bucket0Ctx118WritePulseBusyGateActive = false;
        bool bucket0Ctx118WritePulseDescriptorBit8ConsumeGate = false;
        bool bucket0Ctx118WritePulseDescriptorBit10ShortCircuitGate = false;
        bool bucket0Ctx118WritePulseShortCircuitedByBit10 = false;
        bool bucket0Ctx118WritePulseConsumeGateBit8 = false;
        bool bucket0Ctx118WritePulseCtx54PermitInput = false;
        bool bucket0Ctx118WritePulseCtx54PermitOutput = false;
        bool bucket0Ctx118WritePulseReturnGate144B8Called = false;
        bool bucket0Ctx118WritePulseReturnGate144B8Result = false;
        bool bucket0Ctx118WritePulseReturnGate144B8Row3NoInputBranch = false;
        bool bucket0Ctx118WritePulseReturnGate144B8Row3TieCarryBranch = false;
        bool bucket0Ctx118WritePulseReturnGate144B8Row0TieCarryBranch = false;
        bool bucket0Ctx118WritePulseReturnGate144B8NonZeroRowBlocked = false;
        uint16_t bucket0Ctx118WritePulseReturnGate144B8NoInputCounterInput = 0u;
        uint16_t bucket0Ctx118WritePulseReturnGate144B8TieCarryLatchInput = 0u;
        uint16_t bucket0Ctx118WritePulseReturnGate144B8Ctx6AConsumerGateInput = 0u;
        uint16_t bucket0Ctx118WritePulseReturnGate144B8NoInputCounterOutput = 0u;
        uint16_t bucket0Ctx118WritePulseReturnGate144B8TieCarryLatchOutput = 0u;
        uint16_t tieCarryLatch1E = 0;
        uint16_t noInputCounter20 = 0;
        uint16_t coolGainLatch26 = 0;
        uint16_t coolGainStreak28 = 0;
    };

    struct Bucket30OwnerObserverRuntime {
        bool busyGateActive = false;
        bool scorerCommitWindowOpen = false;
        bool noInputCounterAdvanceRan = false;
        uint16_t noInputCounterAcceptedCountInput = 0u;
        uint16_t noInputCounterInput = 0u;
        uint16_t noInputCounterOutput = 0u;
        bool noInputCounterIncremented = false;
        bool kernelOpen = false;
        bool kernelEntered = false;
        uint16_t processDescriptorFlagWord = 0u;
        uint16_t descriptorFlagWord = 0u;
        uint8_t activeRow = 0u;
        uint8_t prePhase1Classifier36 = 2u;
        bool prePhase1LatchArmed38 = false;
        bool phase1AdvanceCalled = false;
        uint8_t phase1AdvanceSampledClassifier = 2u;
        bool phase1AdvanceFirstBeat = false;
        bool gameplayCurrentBranchKnown = false;
        bool gameplayCurrentBranchSpecial = false;
        bool gameplayAcceptedCountKnown = false;
        int32_t gameplayAcceptedCount = 0;
        bool gameplayLookbackPageCountKnown = false;
        int32_t gameplayLookbackPageCount = 0;
        bool gameplayOverflowActiveKnown = false;
        bool gameplayOverflowActive = false;
        bool gameplayBranchCountKnown = false;
        int32_t gameplayBranchCount = 0;
        bool gameplayDescriptorSubdeltaKnown = false;
        int32_t gameplayDescriptorSubdelta = 0;
        bool gameplayReaderPageOrdinalKnown = false;
        int32_t gameplayReaderPageOrdinal = 0;
        bool gameplayRequiredMaskKnown = false;
        int32_t gameplayRequiredMask = 0;
        bool gameplayUnionMaskKnown = false;
        int32_t gameplayUnionMask = 0;
        bool gameplayAnchorSlotIndexKnown = false;
        int32_t gameplayAnchorSlotIndex = 0;
        bool gameplayRequiredClassTokenKnown = false;
        int32_t gameplayRequiredClassToken = 0;
        bool gameplayAnchorSlotClassTokenKnown = false;
        int32_t gameplayAnchorSlotClassToken = 0;
        bool gameplayAnchorSlotOccupiedKnown = false;
        int32_t gameplayAnchorSlotOccupied = 0;
        bool gameplayAnchorSlotAcceptedMaskKnown = false;
        uint32_t gameplayAnchorSlotAcceptedMask = 0;
        bool gameplayAnchorSlotPayloadKnown = false;
        uint32_t gameplayAnchorSlotPayload = 0;
        bool gameplayAnchorPageOccupiedSlotBitsKnown = false;
        uint32_t gameplayAnchorPageOccupiedSlotBits = 0;
        bool gameplayAnchorPageRequiredMaskSlotBitsKnown = false;
        uint32_t gameplayAnchorPageRequiredMaskSlotBits = 0;
        bool gameplayAnchorPageRequiredOccupiedSlotBitsKnown = false;
        uint32_t gameplayAnchorPageRequiredOccupiedSlotBits = 0;
        bool gameplayAnchorClassMatchKnown = false;
        int32_t gameplayAnchorClassMatch = 0;
        bool gameplayPairBonusKnown = false;
        int32_t gameplayPairBonus = 0;
        bool gameplaySpillPenaltyKnown = false;
        int32_t gameplaySpillPenalty = 0;
        bool gameplayAdditiveTermKnown = false;
        int32_t gameplayAdditiveTerm = 0;
        bool gameplayCommitTermKnown = false;
        int32_t gameplayCommitTerm = 0;
        bool gameplayScoreWritebackKnown = false;
        int32_t gameplayScoreWriteback = 0;
        bool gameplayClampActiveKnown = false;
        bool gameplayClampActive = false;
        int32_t phase1PrevBaseline18 = 0;
        int32_t phase1LiveAccumulator91816 = 0;
        int32_t phase1Delta91816MinusPrev18 = 0;
        int32_t producerGrowthBaseline18 = 0;
        int32_t blinkBaseline18 = 0;
        int32_t helperSnapshot18 = 0;
        uint8_t afterAdvancePhase1Classifier36 = 2u;
        bool afterAdvancePhase1LatchArmed38 = false;
        bool formalWritebackKnown = false;
        int32_t formalWritebackValue = 0;
        bool tieBreakerCalled = false;
        int32_t tieBreakerResult14548 = 2;
        bool resolverGateBit4 = false;
        bool resolverGateEd00Idle = false;
        bool resolutionCalled = false;
        bool resolutionKnown = false;
        uint8_t resolutionInputPhase1Classifier36 = 2u;
        uint8_t resolutionV22 = 2u;
        bool goodToCoolCommitted = false;
        uint8_t afterProducePhase1Classifier36 = 2u;
        bool afterProducePhase1LatchArmed38 = false;
        int32_t afterProduceGrowthBaseline18 = 0;
        int32_t afterProduceBlinkBaseline18 = 0;
        int32_t afterProduceHelperSnapshot18 = 0;
    };

    struct RightRankDirectFollowUpRuntime {
        uint16_t ctx72 = 0u;
        int32_t ed24 = 0;
        int32_t ed00 = 0;
        int32_t ed14 = 0;
        int32_t ed0c = 0;
        bool flag0200Pulse = false;
        bool flag0200FrameUpdateBranchTaken = false;
        bool flag0200FrameUpdateResourcePairKnown = false;
        uint16_t flag0200FrameUpdateCtxE0ResourceIndex = 0u;
        uint16_t flag0200FrameUpdateCtxECResourceIndex = 0u;
        bool flag0200FrameUpdateCtxFlag20000Set = false;
        bool flag0200PostTail1A654Requested = false;
        uint8_t flag0200PostTail1A654Arg = 0u;
        bool flag2000Pulse = false;
        bool flag4000Pulse = false;
        bool hudOverlayConfigure801CA0E0Pulse = false;
        uint8_t hudOverlayConfigure801CA0E0Arg = 0u;
        uint32_t hudOverlayConfigure801CA0E0QueryFrame = 0u;
        uint32_t hudOverlayConfigure801CA0E0ScriptFrame = 0u;
        bool descBankSelectorPulse = false;
        uint8_t descBankSelector = 0u;
        uint32_t descBankSelectorQueryFrame = 0u;
        uint32_t descBankCtxFlagsPulse801CBFDC = 0u;
        bool cameraPulse9443C = false;
        uint16_t cameraBezHandle9443C = 0u;
        uint32_t cameraQueryFrame9443C = 0u;
        bool acceptedTailTimer9443CPulse = false;
        int32_t acceptedTailTimer9443CFrames = 0;
        uint32_t acceptedTailTimer9443CQueryFrame = 0u;
    };

    struct Bucket30Flag0200PrecursorRuntime {
        bool known = false;
        uint32_t queryFrame = 0u;
        int32_t tick96 = 0;
        uint8_t activeRow = 0u;
        int32_t preEd00 = 0;
        int32_t postEd00 = 0;
        uint8_t clearAction = 0u;
        bool deferBucket31 = false;
        bool markBucket31AdditiveClearPending = false;
        bool ownerKernelOpen = false;
        bool resolverGateBit4 = false;
        bool resolutionGateEd00Idle = false;
        bool rowWriteResolutionKnown = false;
        uint8_t rowWriteResolutionV22 = 2u;
        bool rowWriteCommitted = false;
        bool rowWriteGoodToCoolCommitted = false;
        bool acceptedTailSurvived = false;
        bool waitSecondBeatInsideBucket30 = false;
    };

    struct Bucket31Flag0200ProducerRuntime {
        bool known = false;
        uint32_t queryFrame = 0u;
        int32_t tick96 = 0;
        int32_t preEd00 = 0;
        bool awaitBucket31AfterGoodToCool = false;
        bool narrowClearPending = false;
        bool additiveClearPending = false;
        bool runPageClear14BDC = false;
        bool consumerPackageRan = false;
        bool ctxFlag0200Pulse = false;
        bool bucketLocalClearRan = false;
        bool narrowClearFired = false;
        bool clearDeferredAdditiveBookkeeping = false;
        uint8_t followUpPhaseAction = 0u;
        bool playCompletionCue = false;
        bool clearDelayedCompletionPending = false;
        bool precursorKnown = false;
        uint32_t precursorQueryFrame = 0u;
        int32_t precursorTick96 = 0;
        uint8_t precursorActiveRow = 0u;
        int32_t precursorPreEd00 = 0;
        int32_t precursorPostEd00 = 0;
        uint8_t precursorClearAction = 0u;
        bool precursorDeferBucket31 = false;
        bool precursorMarkBucket31AdditiveClearPending = false;
        bool precursorOwnerKernelOpen = false;
        bool precursorResolverGateBit4 = false;
        bool precursorResolutionGateEd00Idle = false;
        bool precursorRowWriteResolutionKnown = false;
        uint8_t precursorRowWriteResolutionV22 = 2u;
        bool precursorRowWriteCommitted = false;
        bool precursorRowWriteGoodToCoolCommitted = false;
        bool precursorAcceptedTailSurvived = false;
        bool precursorWaitSecondBeatInsideBucket30 = false;
    };

    struct DescriptorCadenceRuntime {
        bool initialized = false;
        bool freshEntryLookaheadSeedActive = false;
        size_t cadenceCursorOrdinal1Based = 0;
        bool cadenceCursorAvailable = false;
        DescriptorRowView cadenceCursorRow{};
        bool pageOrdinal56Available = false;
        size_t pageOrdinal56 = 0;
        bool currentCommittedAvailable = false;
        size_t currentCommittedRowIndex = 0;
        DescriptorRowView currentCommittedRow{};
        bool currentDescriptor40Available = false;
        size_t currentDescriptor40RowIndex = 0;
        DescriptorRowView currentDescriptor40Row{};
        bool nextLookaheadAvailable = false;
        size_t nextLookaheadRowIndex = 0;
        DescriptorRowView nextLookaheadRow{};
        bool lookaheadDescriptor44Available = false;
        DescriptorRowView lookaheadDescriptor44Row{};
    };

    struct PageOwnerRuntime {
        bool currentWritePageOrdinalKnown = false;
        uint16_t currentWritePageOrdinal1Based = 0u;
        bool currentWritePageOwnerLastSetKnown = false;
        uint32_t currentWritePageOwnerLastSetQueryFrame = 0u;
        uint16_t currentWritePageOwnerLastSetTargetOrdinal1Based = 0u;
        bool currentWritePageOwnerLastSetTick96Known = false;
        int32_t currentWritePageOwnerLastSetTick96 = 0;
        uint16_t currentWritePageOwnerLastSetPhase384 = 0u;
        uint8_t currentWritePageOwnerLastSetBucket31 = 0u;
        bool literalBucket31PageClear14BDCWindowAdvanced = false;
        bool literalBucket31PageClear14BDCRequested = false;
        bool literalBucket31PageClear14BDCTargetKnown = false;
        uint16_t literalBucket31PageClear14BDCTargetOrdinal1Based = 0u;
        bool literalBucket31PageClear14BDCClearApplied = false;
    };

    struct SelectedSourceGroupRailWindowRuntime {
        bool timingTemplateStateKnown = false;
        uint8_t timingTemplateState = 0;
        bool sourceCellHeaderPresent = false;
        uint16_t sourceCellHeaderCount = 0;
        uint16_t sourceCellHeaderCursor = 0;
        bool sourceCellPresent = false;
        bool sourceCellCallbackArgPresent = false;
    };

    struct GameplayRailDescriptorProducerRuntime {
        bool available = false;
        size_t rowIndex = 0;
        DescriptorRowView row{};
        bool selectorAvailable = false;
        uint8_t selectorByte0 = 0;
        uint8_t selectorByte1 = 0;
        uint8_t cueClassToken = 0;
        SelectedSourceGroupRailWindowRuntime selectedSourceGroupWindow{};
        uint8_t symbolType = 0;
    };

    struct GameplayRailCadenceProducerRuntime {
        GameplayRailDescriptorProducerRuntime currentCommitted{};
        GameplayRailDescriptorProducerRuntime nextLookahead{};
    };

    struct SourceCellVoiceLaneRuntime {
        bool active = false;
        uint8_t program = 0;
        uint8_t note = 0;
        uint8_t key = 0;
        uint8_t volume = 0;
        bool replaceRestartPulse = false;
    };

    struct AcceptedProducerSourceGroupCursorEntryRuntime {
        bool occupied = false;
        uint32_t headerAddr = 0;
        uint16_t count = 0;
        uint16_t cursor = 0;
    };

    struct AcceptedProducerSourceGroupRuntime {
        uint32_t lastClassToken91800 = 0;
        std::array<AcceptedProducerSourceGroupCursorEntryRuntime,
                   kStage1AcceptedProducerSourceGroupCursorEntryCount>
            cursorEntries{};
    };

    bool active = false;
    int lastRunnerFrame = -1;
    uint32_t queryFrame = 0;
    int32_t scoreDisplayValue = 0;
    bool scoreDisplayFormulaBacked = false;
    int32_t formulaAccumulatorValue = 0;
    bool formulaAccumulatorKnown = false;
    bool formulaAccumulatorAuthoritative = false;
    bool word9182AEnabled = false;
    uint8_t descriptorSubstate50 = 0u;
    bool rightRankDescriptorFlagWord08Known = false;
    uint16_t rightRankDescriptorFlagWord08 = 0u;
    bool bucket30ResolutionKnown = false;
    uint8_t bucket30ResolutionV22 = 2u;
    bool bucket30RightRankWritebackCommitted = false;
    bool rightRankLastRowWriteKnown = false;
    int32_t rightRankLastRowWriteQueryFrame = -1;
    uint8_t rightRankLastRowWritePrevRow = 0u;
    uint8_t rightRankLastRowWriteResolvedRow = 0u;
    bool rightRankLastRowWriteResolutionKnown = false;
    uint8_t rightRankLastRowWriteResolutionV22 = 2u;
    bool rightRankLastRowWriteCommitted = false;
    bool rightRankFirstRow3WriteKnown = false;
    int32_t rightRankFirstRow3WriteQueryFrame = -1;
    uint8_t rightRankFirstRow3WritePrevRow = 0u;
    uint8_t rightRankFirstRow3WriteResolvedRow = 0u;
    bool rightRankFirstRow3WriteResolutionKnown = false;
    uint8_t rightRankFirstRow3WriteResolutionV22 = 2u;
    bool rightRankFirstRow3WriteCommitted = false;
    uint8_t rightRankFirstRow3WriteOwnerActiveRow = 0u;
    uint8_t rightRankFirstRow3WriteResolutionInputPhase1Classifier36 = 2u;
    bool rightRankFirstRow3WritePhase1AdvanceFirstBeat = false;
    int32_t rightRankFirstRow3WritePhase1Delta91816MinusPrev18 = 0;
    bool rightRankFirstRow3WriteFormalWritebackKnown = false;
    int32_t rightRankFirstRow3WriteFormalWritebackValue = 0;
    bool rightRankFirstRow3WriteTieBreakerCalled = false;
    int32_t rightRankFirstRow3WriteTieBreakerResult14548 = 2;
    bool rightRankFirstRow3WriteResolverGateBit4 = false;
    bool rightRankFirstRow3WriteResolverGateEd00Idle = false;
    bool rightRankFirstRow3WriteGoodToCoolCommitted = false;
    bool rightRankLastAcceptedCountNonZeroKnown = false;
    int32_t rightRankLastAcceptedCountNonZeroQueryFrame = -1;
    uint16_t rightRankLastAcceptedCountNonZeroValue = 0u;
    uint32_t rightRankLastAcceptedCountNonZeroMask = 0u;
    bool rightRankLastReplayAppendKnown = false;
    int32_t rightRankLastReplayAppendQueryFrame = -1;
    uint32_t rightRankLastReplayAppendKnownBytes = 0u;
    uint32_t rightRankLastReplayAppendMissingBytes = 0u;
    uint32_t rightRankLastReplayAppendPublishedCount = 0u;
    uint32_t rightRankLastReplayAppendWriteCount = 0u;
    bool rightRankLastReplayAppendFullBytes = false;
    bool rightRankLastReplayAppendFullBackingKnown8008EEF8 = false;
    bool rightRankAcceptedReplayRestoreObserved = false;
    bool rightRankAcceptedReplayRestorePayloadBackupValid = false;
    bool rightRankAcceptedReplayRestorePayloadFullBackingKnown8008EEF8 = false;
    bool rightRankAcceptedReplayRestoreSidecarBackupValid = false;
    bool rightRankAcceptedReplayRestoreSidecarFullBackingKnown8008EEF8 = false;
    uint8_t rightRankAcceptedReplayRestoreSource1681C = 0u;
    bool rightRankAcceptedReplayRestoreRequested1681C = false;
    bool rightRankAcceptedReplayRestoreApplied1681C = false;
    uint16_t rightRankAcceptedReplaySetupTransitionState = 0u;
    bool rightRankAcceptedReplayEventTableSeed801C8660Applied = false;
    uint32_t rightRankAcceptedReplayRestorePostKnownBytes = 0u;
    bool rightRankAcceptedReplayRestorePostFullBytes = false;
    bool rightRankAcceptedReplayRestorePostFullBackingKnown8008EEF8 = false;
    Stage1AcceptedReplayPayloadRestoreGateRuntime
        rightRankAcceptedReplayRestorePayloadGate{};
    bool rightRankLastAcceptedTailDecisionKnown = false;
    int32_t rightRankLastAcceptedTailDecisionQueryFrame = -1;
    uint32_t rightRankLastAcceptedTailDecisionCtxInput18 = 0u;
    uint32_t rightRankLastAcceptedTailDecisionPreviousInputMask801CCBB8 = 0u;
    uint32_t rightRankLastAcceptedTailDecisionAcceptedMask9FF = 0u;
    bool rightRankLastAcceptedTailDecisionGateOpen = false;
    bool rightRankLastAcceptedTailDecisionMaskChanged = false;
    bool rightRankLastAcceptedTailDecisionCall14614 = false;
    bool rightRankLastAcceptedTailDecisionReplayMode52 = false;
    bool rightRankLastAcceptedTailCallKnown = false;
    int32_t rightRankLastAcceptedTailCallQueryFrame = -1;
    uint32_t rightRankLastAcceptedTailCallCtxInput18 = 0u;
    uint32_t rightRankLastAcceptedTailCallPreviousInputMask801CCBB8 = 0u;
    uint32_t rightRankLastAcceptedTailCallAcceptedMask9FF = 0u;
    bool rightRankLastAcceptedTailCallDirectRunCaptured = false;
    int32_t rightRankLastAcceptedTailCallDirectRunResult = 0;
    bool rightRankLastAcceptedTailCallReplayAppend = false;
    bool rightRankLastAcceptedTailCallSelectorResolved = false;
    uint8_t rightRankLastAcceptedTailCallSelectorByte0 = 0u;
    uint8_t rightRankLastAcceptedTailCallSelectorByte1 = 0u;
    bool rightRankLastAcceptedTailCallTimingTemplateKnown = false;
    uint8_t rightRankLastAcceptedTailCallTimingTemplateSlot48 = 0u;
    uint8_t rightRankLastAcceptedTailCallTimingTemplateState = 0u;
    bool rightRankLastAcceptedTailCallAcceptedTick96Known = false;
    int32_t rightRankLastAcceptedTailCallAcceptedTick96 = 0;
    uint8_t rightRankLastAcceptedTailCallHalfWindow34 = 0u;
    uint16_t rightRankLastAcceptedTailCallPhase384 = 0u;
    uint8_t rightRankLastAcceptedTailCallRecordSlot24 = 0u;
    uint8_t rightRankLastAcceptedTailCallRecordRemainder24 = 0u;
    bool rightRankLastAcceptedTailCallSourceCellHeaderValid = false;
    uint32_t rightRankLastAcceptedTailCallSourceCellHeaderAddr = 0u;
    uint32_t rightRankLastAcceptedTailCallSourceCellHeaderBasePtr = 0u;
    uint16_t rightRankLastAcceptedTailCallSourceCellHeaderCount = 0u;
    uint16_t rightRankLastAcceptedTailCallSourceCellHeaderCursor = 0u;
    bool rightRankLastAcceptedTailCallSourceCellValid = false;
    bool rightRankLastSteadyInputNonZeroKnown = false;
    int32_t rightRankLastSteadyInputNonZeroQueryFrame = -1;
    bool rightRankLastSteadyInputNonZeroReplayMode52 = false;
    uint32_t rightRankLastSteadyInputNonZeroHeldMask = 0u;
    bool rightRankLastSteadyInputNonZeroWriteCtx18 = false;
    uint32_t rightRankLastSteadyInputNonZeroCtx18Value = 0u;
    bool rightRankLastSteadyInputNonZeroWriteCtx20 = false;
    int32_t rightRankLastSteadyInputNonZeroCtx20Value = 0;
    bool rightRankFirstNoInputCounterProducerKnown = false;
    int32_t rightRankFirstNoInputCounterProducerQueryFrame = -1;
    bool rightRankFirstNoInputCounterProducerAfterAcceptedClear = false;
    int32_t rightRankFirstNoInputCounterProducerDeltaAfterAcceptedClear = -1;
    int32_t rightRankFirstNoInputCounterProducerDeltaAfterLastAcceptedCountClear =
        -1;
    bool rightRankFirstNoInputCounterProducerAcceptedSourceAfterLastClearKnown =
        false;
    bool rightRankFirstNoInputCounterProducerAcceptedSourceAfterLastClear =
        false;
    uint8_t rightRankFirstNoInputCounterProducerActiveRow = 0u;
    uint16_t rightRankFirstNoInputCounterProducerDescriptorFlags = 0u;
    uint16_t rightRankFirstNoInputCounterProducerDescriptorLowBits03 = 0u;
    bool rightRankFirstNoInputCounterProducerBusyGateActive = false;
    bool rightRankFirstNoInputCounterProducerScorerWindowOpen = false;
    uint8_t rightRankFirstNoInputCounterProducerDescriptorSubstate50 = 0u;
    bool rightRankFirstNoInputCounterProducerDescriptorCadenceCursorAvailable =
        false;
    size_t rightRankFirstNoInputCounterProducerDescriptorCadenceCursorOrdinal1Based =
        0u;
    bool rightRankFirstNoInputCounterProducerDescriptorPageOrdinal56Available =
        false;
    size_t rightRankFirstNoInputCounterProducerDescriptorPageOrdinal56 = 0u;
    bool rightRankFirstNoInputCounterProducerDescriptorCurrentCommittedAvailable =
        false;
    size_t rightRankFirstNoInputCounterProducerDescriptorCurrentCommittedRowIndex =
        0u;
    bool rightRankFirstNoInputCounterProducerDescriptorCurrentDescriptor40Available =
        false;
    size_t rightRankFirstNoInputCounterProducerDescriptorCurrentDescriptor40RowIndex =
        0u;
    bool rightRankFirstNoInputCounterProducerDescriptorRowAvailable = false;
    uint8_t rightRankFirstNoInputCounterProducerDescriptorLessonId = 0u;
    uint16_t rightRankFirstNoInputCounterProducerDescriptorDefaultFlagWord = 0u;
    uint16_t rightRankFirstNoInputCounterProducerDescriptorSubstateFlagWord = 0u;
    uint32_t rightRankFirstNoInputCounterProducerDescriptorRequiredMask = 0u;
    uint8_t rightRankFirstNoInputCounterProducerDescriptorAnchorSlotIndex = 0u;
    uint8_t rightRankFirstNoInputCounterProducerDescriptorRequiredClassToken =
        0u;
    uint8_t rightRankFirstNoInputCounterProducerDescriptorDefaultSelector0 =
        0u;
    uint8_t rightRankFirstNoInputCounterProducerDescriptorDefaultSelector1 =
        0u;
    uint8_t rightRankFirstNoInputCounterProducerDescriptorSubstateSelector0 =
        0u;
    uint8_t rightRankFirstNoInputCounterProducerDescriptorSubstateSelector1 =
        0u;
    uint8_t rightRankFirstNoInputCounterProducerCurrentBucket = 0xFFu;
    uint8_t rightRankFirstNoInputCounterProducerPreviousBucket = 0xFFu;
    bool rightRankFirstNoInputCounterProducerBucket30Advanced = false;
    uint32_t rightRankFirstNoInputCounterProducerBucketAdvanceCount = 0u;
    uint16_t rightRankFirstNoInputCounterProducerAcceptedCountInput = 0u;
    bool rightRankFirstNoInputCounterProducerSteadyInputKnown = false;
    uint32_t rightRankFirstNoInputCounterProducerSteadyInputHeldMask = 0u;
    bool rightRankFirstNoInputCounterProducerSteadyInputWriteCtx18 = false;
    uint32_t rightRankFirstNoInputCounterProducerSteadyInputCtx18Value = 0u;
    bool rightRankFirstNoInputCounterProducerSteadyInputWriteCtx20 = false;
    int32_t rightRankFirstNoInputCounterProducerSteadyInputCtx20Value = 0;
    uint16_t rightRankFirstNoInputCounterProducerLocalHoldMask80035510 = 0u;
    uint16_t rightRankFirstNoInputCounterProducerLocalConsumedHoldMask80035510 =
        0u;
    bool rightRankFirstNoInputCounterProducerLocalDebounceBypassed80035510 =
        false;
    bool rightRankFirstNoInputCounterProducerLastSteadyInputNonZeroKnown =
        false;
    int32_t rightRankFirstNoInputCounterProducerLastSteadyInputNonZeroQueryFrame =
        -1;
    int32_t rightRankFirstNoInputCounterProducerLastSteadyInputNonZeroDeltaToProducer =
        -1;
    int32_t rightRankFirstNoInputCounterProducerLastSteadyInputNonZeroDeltaAfterLastAcceptedCountClear =
        -1;
    bool rightRankFirstNoInputCounterProducerLastSteadyInputNonZeroAfterLastClearKnown =
        false;
    bool rightRankFirstNoInputCounterProducerLastSteadyInputNonZeroAfterLastClear =
        false;
    bool rightRankFirstNoInputCounterProducerLastSteadyInputNonZeroReplayMode52 =
        false;
    uint32_t rightRankFirstNoInputCounterProducerLastSteadyInputNonZeroHeldMask =
        0u;
    bool rightRankFirstNoInputCounterProducerLastSteadyInputNonZeroWriteCtx18 =
        false;
    uint32_t rightRankFirstNoInputCounterProducerLastSteadyInputNonZeroCtx18Value =
        0u;
    bool rightRankFirstNoInputCounterProducerLastSteadyInputNonZeroWriteCtx20 =
        false;
    int32_t rightRankFirstNoInputCounterProducerLastSteadyInputNonZeroCtx20Value =
        0;
    uint32_t rightRankFirstNoInputCounterProducerLastSteadyInputNonZeroRequiredMaskOverlap =
        0u;
    bool rightRankFirstNoInputCounterProducerReplayMirrorKnown = false;
    bool rightRankFirstNoInputCounterProducerReplayProducerKnown = false;
    bool rightRankFirstNoInputCounterProducerReplayBytesKnown = false;
    uint32_t rightRankFirstNoInputCounterProducerReplayRequiredBytes = 0u;
    uint32_t rightRankFirstNoInputCounterProducerReplayKnownBytes = 0u;
    uint32_t rightRankFirstNoInputCounterProducerReplayMissingBytes = 0u;
    bool rightRankFirstNoInputCounterProducerReplayFullBytes = false;
    bool rightRankFirstNoInputCounterProducerReplayFullBackingKnown8008EEF8 =
        false;
    bool rightRankFirstNoInputCounterProducerLastReplayAppendKnown = false;
    int32_t rightRankFirstNoInputCounterProducerLastReplayAppendQueryFrame =
        -1;
    int32_t rightRankFirstNoInputCounterProducerLastReplayAppendDeltaAfterLastAcceptedCountClear =
        -1;
    bool rightRankFirstNoInputCounterProducerLastReplayAppendAfterLastClearKnown =
        false;
    bool rightRankFirstNoInputCounterProducerLastReplayAppendAfterLastClear =
        false;
    uint32_t rightRankFirstNoInputCounterProducerLastReplayAppendKnownBytes =
        0u;
    uint32_t rightRankFirstNoInputCounterProducerLastReplayAppendMissingBytes =
        0u;
    uint32_t rightRankFirstNoInputCounterProducerLastReplayAppendPublishedCount =
        0u;
    uint32_t rightRankFirstNoInputCounterProducerLastReplayAppendWriteCount =
        0u;
    bool rightRankFirstNoInputCounterProducerLastReplayAppendFullBytes = false;
    bool rightRankFirstNoInputCounterProducerLastReplayAppendFullBackingKnown8008EEF8 =
        false;
    bool rightRankFirstNoInputCounterProducerLastAcceptedTailDecisionKnown =
        false;
    int32_t rightRankFirstNoInputCounterProducerLastAcceptedTailDecisionQueryFrame =
        -1;
    uint32_t rightRankFirstNoInputCounterProducerLastAcceptedTailDecisionCtxInput18 =
        0u;
    uint32_t rightRankFirstNoInputCounterProducerLastAcceptedTailDecisionPreviousInputMask801CCBB8 =
        0u;
    uint32_t rightRankFirstNoInputCounterProducerLastAcceptedTailDecisionAcceptedMask9FF =
        0u;
    bool rightRankFirstNoInputCounterProducerLastAcceptedTailDecisionGateOpen =
        false;
    bool rightRankFirstNoInputCounterProducerLastAcceptedTailDecisionMaskChanged =
        false;
    bool rightRankFirstNoInputCounterProducerLastAcceptedTailDecisionCall14614 =
        false;
    bool rightRankFirstNoInputCounterProducerLastAcceptedTailDecisionReplayMode52 =
        false;
    bool rightRankFirstNoInputCounterProducerLastAcceptedTailCallKnown = false;
    int32_t rightRankFirstNoInputCounterProducerLastAcceptedTailCallQueryFrame =
        -1;
    int32_t rightRankFirstNoInputCounterProducerLastAcceptedTailCallDeltaAfterLastAcceptedCountClear =
        -1;
    bool rightRankFirstNoInputCounterProducerLastAcceptedTailCallAfterLastClearKnown =
        false;
    bool rightRankFirstNoInputCounterProducerLastAcceptedTailCallAfterLastClear =
        false;
    uint32_t rightRankFirstNoInputCounterProducerLastAcceptedTailCallCtxInput18 =
        0u;
    uint32_t rightRankFirstNoInputCounterProducerLastAcceptedTailCallPreviousInputMask801CCBB8 =
        0u;
    uint32_t rightRankFirstNoInputCounterProducerLastAcceptedTailCallAcceptedMask9FF =
        0u;
    bool rightRankFirstNoInputCounterProducerLastAcceptedTailCallDirectRunCaptured =
        false;
    int32_t rightRankFirstNoInputCounterProducerLastAcceptedTailCallDirectRunResult =
        0;
    bool rightRankFirstNoInputCounterProducerLastAcceptedTailCallReplayAppend =
        false;
    bool rightRankFirstNoInputCounterProducerLastAcceptedTailCallSelectorResolved =
        false;
    uint8_t rightRankFirstNoInputCounterProducerLastAcceptedTailCallSelectorByte0 =
        0u;
    uint8_t rightRankFirstNoInputCounterProducerLastAcceptedTailCallSelectorByte1 =
        0u;
    bool rightRankFirstNoInputCounterProducerLastAcceptedTailCallTimingTemplateKnown =
        false;
    uint8_t rightRankFirstNoInputCounterProducerLastAcceptedTailCallTimingTemplateSlot48 =
        0u;
    uint8_t rightRankFirstNoInputCounterProducerLastAcceptedTailCallTimingTemplateState =
        0u;
    bool rightRankFirstNoInputCounterProducerLastAcceptedTailCallAcceptedTick96Known =
        false;
    int32_t rightRankFirstNoInputCounterProducerLastAcceptedTailCallAcceptedTick96 =
        0;
    uint8_t rightRankFirstNoInputCounterProducerLastAcceptedTailCallHalfWindow34 =
        0u;
    uint16_t rightRankFirstNoInputCounterProducerLastAcceptedTailCallPhase384 =
        0u;
    uint8_t rightRankFirstNoInputCounterProducerLastAcceptedTailCallRecordSlot24 =
        0u;
    uint8_t rightRankFirstNoInputCounterProducerLastAcceptedTailCallRecordRemainder24 =
        0u;
    bool rightRankFirstNoInputCounterProducerLastAcceptedTailCallSourceCellHeaderValid =
        false;
    uint32_t rightRankFirstNoInputCounterProducerLastAcceptedTailCallSourceCellHeaderAddr =
        0u;
    uint32_t rightRankFirstNoInputCounterProducerLastAcceptedTailCallSourceCellHeaderBasePtr =
        0u;
    uint16_t rightRankFirstNoInputCounterProducerLastAcceptedTailCallSourceCellHeaderCount =
        0u;
    uint16_t rightRankFirstNoInputCounterProducerLastAcceptedTailCallSourceCellHeaderCursor =
        0u;
    bool rightRankFirstNoInputCounterProducerLastAcceptedTailCallSourceCellValid =
        false;
    bool rightRankFirstNoInputCounterProducerLastAcceptedCountNonZeroKnown =
        false;
    int32_t rightRankFirstNoInputCounterProducerLastAcceptedCountNonZeroQueryFrame =
        -1;
    uint16_t rightRankFirstNoInputCounterProducerLastAcceptedCountNonZeroValue =
        0u;
    uint32_t rightRankFirstNoInputCounterProducerLastAcceptedCountNonZeroMask =
        0u;
    bool rightRankFirstNoInputCounterProducerLastAcceptedCountClearKnown =
        false;
    int32_t rightRankFirstNoInputCounterProducerLastAcceptedCountClearQueryFrame =
        -1;
    uint8_t rightRankFirstNoInputCounterProducerLastAcceptedCountClearSourceBucket =
        0u;
    uint16_t rightRankFirstNoInputCounterProducerLastAcceptedCountClearPreCount =
        0u;
    uint32_t rightRankFirstNoInputCounterProducerLastAcceptedCountClearPreMask =
        0u;
    uint32_t rightRankFirstNoInputCounterProducerLastAcceptedCountClearMask9180C =
        0u;
    uint8_t rightRankFirstNoInputCounterProducerLastAcceptedCountClearAction =
        0u;
    int32_t rightRankFirstNoInputCounterProducerLastAcceptedCountClearPreBucket30Ed00 =
        0;
    bool rightRankFirstNoInputCounterProducerLastAcceptedCountClearDirectConsumer94400 =
        false;
    bool rightRankFirstNoInputCounterProducerLastAcceptedCountClearDirectConsumerImmediateFollowUpClear =
        false;
    bool rightRankFirstNoInputCounterProducerLastAcceptedCountClearDirectConsumerOwnerNoResolution94400 =
        false;
    bool rightRankFirstNoInputCounterProducerLastAcceptedCountClearWaitSecondBeatBucket30 =
        false;
    bool rightRankFirstNoInputCounterProducerLastAcceptedCountClearAcceptedTailSurvived =
        false;
    bool rightRankFirstNoInputCounterProducerLastAcceptedCountClearOwnerKernelOpen =
        false;
    bool rightRankFirstNoInputCounterProducerLastAcceptedCountClearResolverGateBit4 =
        false;
    bool rightRankFirstNoInputCounterProducerLastAcceptedCountClearResolutionGateEd00Idle =
        false;
    bool rightRankFirstNoInputCounterProducerLastAcceptedCountClearPhase1LatchArmed38 =
        false;
    bool rightRankFirstNoInputCounterProducerLastAcceptedCountClearFollowUpPhaseIsNone =
        false;
    bool rightRankFirstNoInputCounterProducerLastAcceptedCountClearRowWriteResolutionKnown =
        false;
    uint8_t rightRankFirstNoInputCounterProducerLastAcceptedCountClearRowWriteResolutionV22 =
        2u;
    bool rightRankFirstNoInputCounterProducerLastAcceptedCountClearRowWriteCommitted =
        false;
    bool rightRankFirstNoInputCounterProducerLastAcceptedCountClearRowWriteGoodToCoolCommitted =
        false;
    bool rightRankFirstNoInputCounterProducerLastAcceptedCountClearRowWriteDirectConsumerFallback94400 =
        false;
    bool rightRankFirstNoInputCounterProducerLastAcceptedCountClearDirectSlotKnown =
        false;
    uint8_t rightRankFirstNoInputCounterProducerLastAcceptedCountClearDirectSlot =
        0u;
    uint16_t rightRankFirstNoInputCounterProducerInput = 0u;
    uint16_t rightRankFirstNoInputCounterProducerOutput = 0u;
    bool rightRankFirstNoInputCounterProducerIncremented = false;
    uint8_t rightRankFirstNoInputCounterProducerSameSliceClearAction = 0u;
    bool rightRankFirstNoInputCounterProducerSameSliceClearRan = false;
    bool rightRankFirstNoInputCounterProducerBeforeSameSliceClear = false;
    bool rightRankFirstNoInputCounterProducerCtx6AInitKnown = false;
    int32_t rightRankFirstNoInputCounterProducerCtx6AInitQueryFrame = -1;
    uint16_t rightRankFirstNoInputCounterProducerCtx6AInitValue = 0u;
    bool rightRankLastAcceptedCountClearKnown = false;
    int32_t rightRankLastAcceptedCountClearQueryFrame = -1;
    uint8_t rightRankLastAcceptedCountClearSourceBucket = 0u;
    uint16_t rightRankLastAcceptedCountClearPreCount = 0u;
    uint16_t rightRankLastAcceptedCountClearPostCount = 0u;
    uint32_t rightRankLastAcceptedCountClearPreMask = 0u;
    uint32_t rightRankLastAcceptedCountClearPostMask = 0u;
    uint32_t rightRankLastAcceptedCountClearMask9180C = 0u;
    uint8_t rightRankLastAcceptedCountClearAction = 0u;
    int32_t rightRankLastAcceptedCountClearPreBucket30Ed00 = 0;
    bool rightRankLastAcceptedCountClearDirectConsumer94400 = false;
    bool rightRankLastAcceptedCountClearDirectConsumerImmediateFollowUpClear =
        false;
    bool rightRankLastAcceptedCountClearDirectConsumerOwnerNoResolution94400 =
        false;
    bool rightRankLastAcceptedCountClearWaitSecondBeatBucket30 = false;
    bool rightRankLastAcceptedCountClearAcceptedTailSurvived = false;
    bool rightRankLastAcceptedCountClearOwnerKernelOpen = false;
    bool rightRankLastAcceptedCountClearResolverGateBit4 = false;
    bool rightRankLastAcceptedCountClearResolutionGateEd00Idle = false;
    bool rightRankLastAcceptedCountClearPhase1LatchArmed38 = false;
    bool rightRankLastAcceptedCountClearFollowUpPhaseIsNone = false;
    bool rightRankLastAcceptedCountClearRowWriteResolutionKnown = false;
    uint8_t rightRankLastAcceptedCountClearRowWriteResolutionV22 = 2u;
    bool rightRankLastAcceptedCountClearRowWriteCommitted = false;
    bool rightRankLastAcceptedCountClearRowWriteGoodToCoolCommitted = false;
    bool rightRankLastAcceptedCountClearRowWriteDirectConsumerFallback94400 =
        false;
    bool rightRankLastAcceptedCountClearDirectSlotKnown = false;
    uint8_t rightRankLastAcceptedCountClearDirectSlot = 0u;
    bool rightRankFirstAcceptedCountClearKnown = false;
    int32_t rightRankFirstAcceptedCountClearQueryFrame = -1;
    uint8_t rightRankFirstAcceptedCountClearSourceBucket = 0u;
    uint8_t rightRankFirstAcceptedCountClearCurrentBucket = 0xFFu;
    uint8_t rightRankFirstAcceptedCountClearPreviousBucket = 0xFFu;
    bool rightRankFirstAcceptedCountClearBucket30Advanced = false;
    uint32_t rightRankFirstAcceptedCountClearBucketAdvanceCount = 0u;
    uint16_t rightRankFirstAcceptedCountClearPreCount = 0u;
    uint16_t rightRankFirstAcceptedCountClearPostCount = 0u;
    uint32_t rightRankFirstAcceptedCountClearPreMask = 0u;
    uint32_t rightRankFirstAcceptedCountClearPostMask = 0u;
    uint32_t rightRankFirstAcceptedCountClearMask9180C = 0u;
    bool rightRankFirstAcceptedCountClearBucketLocalClearRan = false;
    bool rightRankFirstAcceptedCountClearNarrowFired = false;
    uint8_t rightRankFirstAcceptedCountClearAction = 0u;
    int32_t rightRankFirstAcceptedCountClearPreBucket30Ed00 = 0;
    bool rightRankFirstAcceptedCountClearDirectConsumer94400 = false;
    bool rightRankFirstAcceptedCountClearDirectConsumerImmediateFollowUpClear =
        false;
    bool rightRankFirstAcceptedCountClearDirectConsumerOwnerNoResolution94400 =
        false;
    bool rightRankFirstAcceptedCountClearWaitSecondBeatBucket30 = false;
    bool rightRankFirstAcceptedCountClearAcceptedTailSurvived = false;
    bool rightRankFirstAcceptedCountClearOwnerKernelOpen = false;
    bool rightRankFirstAcceptedCountClearResolverGateBit4 = false;
    bool rightRankFirstAcceptedCountClearResolutionGateEd00Idle = false;
    bool rightRankFirstAcceptedCountClearPhase1LatchArmed38 = false;
    bool rightRankFirstAcceptedCountClearFollowUpPhaseIsNone = false;
    bool rightRankFirstAcceptedCountClearRowWriteResolutionKnown = false;
    uint8_t rightRankFirstAcceptedCountClearRowWriteResolutionV22 = 2u;
    bool rightRankFirstAcceptedCountClearRowWriteResolutionSkippedMissingResolverGateBit4 =
        false;
    bool rightRankFirstAcceptedCountClearRowWriteResolutionSkippedFollowUpActive =
        false;
    bool rightRankFirstAcceptedCountClearRowWriteCommitted = false;
    bool rightRankFirstAcceptedCountClearRowWriteGoodToCoolCommitted = false;
    bool rightRankFirstAcceptedCountClearRowWriteDirectConsumerFallback94400 =
        false;
    bool rightRankFirstAcceptedCountClearDirectSlotKnown = false;
    uint8_t rightRankFirstAcceptedCountClearDirectSlot = 0u;
    bool rightRankFirstAcceptedCountClearDescriptorFlagKnown = false;
    uint16_t rightRankFirstAcceptedCountClearDescriptorFlagWord = 0u;
    uint8_t rightRankFirstAcceptedCountClearDescriptorSubstate50 = 0u;
    bool rightRankFirstAcceptedCountClearDescriptorCadenceCursorAvailable =
        false;
    size_t rightRankFirstAcceptedCountClearDescriptorCadenceCursorOrdinal1Based =
        0u;
    bool rightRankFirstAcceptedCountClearDescriptorPageOrdinal56Available =
        false;
    size_t rightRankFirstAcceptedCountClearDescriptorPageOrdinal56 = 0u;
    bool rightRankFirstAcceptedCountClearDescriptorCurrentCommittedAvailable =
        false;
    size_t rightRankFirstAcceptedCountClearDescriptorCurrentCommittedRowIndex =
        0u;
    bool rightRankFirstAcceptedCountClearDescriptorCurrentDescriptor40Available =
        false;
    size_t rightRankFirstAcceptedCountClearDescriptorCurrentDescriptor40RowIndex =
        0u;
    bool rightRankFirstAcceptedCountClearDescriptorRowAvailable = false;
    uint8_t rightRankFirstAcceptedCountClearDescriptorLessonId = 0u;
    uint8_t rightRankFirstAcceptedCountClearDescriptorDefaultSelector0 = 0u;
    uint8_t rightRankFirstAcceptedCountClearDescriptorDefaultSelector1 = 0u;
    uint16_t rightRankFirstAcceptedCountClearDescriptorDefaultFlagWord = 0u;
    uint8_t rightRankFirstAcceptedCountClearDescriptorSubstateSelector0 = 0u;
    uint8_t rightRankFirstAcceptedCountClearDescriptorSubstateSelector1 = 0u;
    uint16_t rightRankFirstAcceptedCountClearDescriptorSubstateFlagWord = 0u;
    bool rightRankFirstAcceptedCountClearGameplayReaderPageOrdinalKnown = false;
    int32_t rightRankFirstAcceptedCountClearGameplayReaderPageOrdinal = 0;
    bool rightRankFirstAcceptedCountClearGameplayRequiredMaskKnown = false;
    int32_t rightRankFirstAcceptedCountClearGameplayRequiredMask = 0;
    bool rightRankFirstAcceptedCountClearGameplayUnionMaskKnown = false;
    int32_t rightRankFirstAcceptedCountClearGameplayUnionMask = 0;
    bool rightRankFirstAcceptedCountClearGameplayAnchorSlotIndexKnown = false;
    int32_t rightRankFirstAcceptedCountClearGameplayAnchorSlotIndex = 0;
    bool rightRankFirstAcceptedCountClearGameplayRequiredClassTokenKnown = false;
    int32_t rightRankFirstAcceptedCountClearGameplayRequiredClassToken = 0;
    bool rightRankFirstAcceptedCountClearGameplayAnchorSlotClassTokenKnown =
        false;
    int32_t rightRankFirstAcceptedCountClearGameplayAnchorSlotClassToken = 0;
    bool rightRankFirstAcceptedCountClearGameplayAnchorSlotOccupiedKnown =
        false;
    int32_t rightRankFirstAcceptedCountClearGameplayAnchorSlotOccupied = 0;
    bool rightRankFirstAcceptedCountClearGameplayAnchorSlotAcceptedMaskKnown =
        false;
    uint32_t rightRankFirstAcceptedCountClearGameplayAnchorSlotAcceptedMask = 0u;
    bool rightRankFirstAcceptedCountClearGameplayAnchorSlotPayloadKnown =
        false;
    uint32_t rightRankFirstAcceptedCountClearGameplayAnchorSlotPayload = 0u;
    bool rightRankFirstAcceptedCountClearGameplayAnchorPageOccupiedSlotBitsKnown =
        false;
    uint32_t rightRankFirstAcceptedCountClearGameplayAnchorPageOccupiedSlotBits =
        0u;
    bool rightRankFirstAcceptedCountClearGameplayAnchorPageRequiredMaskSlotBitsKnown =
        false;
    uint32_t
        rightRankFirstAcceptedCountClearGameplayAnchorPageRequiredMaskSlotBits =
            0u;
    bool rightRankFirstAcceptedCountClearGameplayAnchorPageRequiredOccupiedSlotBitsKnown =
        false;
    uint32_t
        rightRankFirstAcceptedCountClearGameplayAnchorPageRequiredOccupiedSlotBits =
            0u;
    bool rightRankFirstAcceptedCountClearGameplayAnchorClassMatchKnown = false;
    int32_t rightRankFirstAcceptedCountClearGameplayAnchorClassMatch = 0;
    std::array<FirstAcceptedCountClearRequiredOccupiedSlotRuntime, 2>
        rightRankFirstAcceptedCountClearGameplayRequiredOccupiedSlots{};
    DescriptorRowView currentDescriptorRow{};
    PrStage1RightRankSnapshot rightRankState{};
    PrStage1TopLessonPairSnapshot topLessonPairState{};
    Stage1BucketCadenceRuntime bucketCadence{};
    enum class RightRankFollowUpPhase : uint8_t {
        None = 0,
        AwaitBucket31AfterGoodToCool,
        AwaitForceGoodWriteback,
    };
    RightRankFollowUpPhase rightRankFollowUpPhase = RightRankFollowUpPhase::None;
    SharedAcceptedProducerRuntime acceptedProducer{};
    enum class AcceptedProducerNarrowClearPhase : uint8_t {
        None = 0,
        Bucket30,
        Bucket31,
    };
    bool acceptedProducerNarrowClearPending = false;
    bool acceptedProducerNarrowClearFired = false;
    AcceptedProducerNarrowClearPhase acceptedProducerNarrowClearPhase =
        AcceptedProducerNarrowClearPhase::None;
    AcceptedProducerCarrierRuntime acceptedProducerCarrier{};
    SharedAcceptedProducerBoundaryProbeRuntime acceptedProducerBoundaryProbe{};
    AcceptedProducerLastRecordedPageWriteRuntime acceptedProducerLastRecordedPageWrite{};
    AcceptedProducerReplayBufferRuntime acceptedProducerReplayBuffer{};
    Stage1AcceptedProducerScriptedWriterRuntime acceptedProducerScriptedWriter{};
    bool ctx52ReplayMode7A60 = false;
    bool runnerPreambleHalfWindow34Known = false;
    uint8_t runnerPreambleHalfWindow34 = 0u;
    uint16_t localAcceptedProducerHoldMask80035510 = 0u;
    uint16_t localAcceptedProducerConsumedHoldMask80035510 = 0u;
    bool localAcceptedProducerHoldDebounceBypassed80035510 = false;
    uint32_t runnerPostFramePreviousInputMask801CCBB8 = 0u;
    RunnerPostFrame7A60Runtime runnerPostFrame7A60{};
    RunnerMainLoopTail7A60Runtime runnerMainLoopTail7A60{};
    RunnerTailHost7A60Runtime runnerTailHost7A60{};
    SteadyInput7A60Runtime steadyInput7A60{};
    AcceptedInputSourceEventRuntime acceptedInputSourceEvent{};
    RunnerTimecode801C7560Runtime runnerTimecode801C7560{};
    SharedAcceptedProducerBoundaryProbeRuntime lateBranchDynamicCarryProbe{};
    SharedAcceptedProducerBoundaryProbeRuntime lateBranchBridgeProbe{};
    int lateBranchDynamicCarryProbeFrame = -1;
    AdditiveLaneRuntime additiveLane{};
    DirectPortScorerRuntime scorerPort{};
    PageRecordMirrorRuntime pageRecordMirror{};
    RightRankPhase1OwnerRuntime rightRankPhase1Owner{};
    RightRank24F8CObserverRuntime rightRank24F8CObserver{};
    RightRankTieBreakerObserverRuntime rightRankTieBreakerObserver{};
    RightRankBucketContextRuntime rightRankBucketContext{};
    RightRankHelperShadowRuntime rightRankHelperShadow{};
    Bucket30OwnerObserverRuntime bucket30OwnerObserver{};
    Bucket30Flag0200PrecursorRuntime bucket30Flag0200Precursor{};
    Bucket31Flag0200ProducerRuntime bucket31Flag0200Producer{};
    RightRankDirectFollowUpRuntime rightRankDirectFollowUp{};
    bool rightRankForcedGoodEventStreamDone28 = false;
    bool psxEventStreamFlagKnown = false;
    bool psxEventStreamFlagActive = true;
    bool psxEventStreamIdKnown = false;
    uint8_t psxEventStreamId = 0;
    bool psxCtxFlags40_2000_4000Known = false;
    uint16_t psxCtxFlags40_2000_4000 = 0u;
    bool psxFlag100BlocksWaitPulse = false;
    uint8_t psxFlag100BlocksWaitSourceStream = 0u;
    DescriptorCadenceRuntime descriptorCadence{};
    PageOwnerRuntime pageOwner{};
    GameplayRailCadenceProducerRuntime gameplayRailCadenceProducer{};
    SourceCellVoiceLaneRuntime sourceCellVoice{};
    AcceptedProducerSourceGroupRuntime acceptedProducerSourceGroup{};
    Stage1SteadySfxDispatchRuntime steadySfx{};
    bool steadySfxDelayedCompletionPending = false;
};

inline bool
Stage1AcceptedProducerReplayStartupZeroInitializationRequired80028590(
    const Stage1NumericRuntimeState& state) {
    return !state.acceptedProducerReplayBuffer
                .startupZeroInitializationConsumed80028590;
}

inline void ResetStage1NumericRuntimeStatePreservingProcessReplay80028590(
    Stage1NumericRuntimeState& state) {
    const Stage1NumericRuntimeState::AcceptedProducerReplayBufferRuntime
        persistentReplay = state.acceptedProducerReplayBuffer;
    state = Stage1NumericRuntimeState{};
    state.acceptedProducerReplayBuffer = persistentReplay;
}

template <typename BuildReplay, typename StoreReplay>
inline void InitializeStage1AcceptedProducerReplayMirrorFromStartupZero80028590(
    Stage1NumericRuntimeState& state,
    BuildReplay&& buildReplay,
    StoreReplay&& storeReplay) {
    if (!Stage1AcceptedProducerReplayStartupZeroInitializationRequired80028590(
            state)) {
        return;
    }
    PrStage1ScorerDirectReplayBufferState replay = buildReplay(state);
    if (!PrStage1ScorerDirectInitializeReplayMirrorFromStartupZero80028590(
            replay)) {
        return;
    }
    storeReplay(state, replay);
    state.acceptedProducerReplayBuffer
        .startupZeroInitializationConsumed80028590 = true;
}

extern Stage1NumericRuntimeState s_stage1NumericRuntime;
extern Stage1AcceptedProducerReplayBackupRuntime s_stage1AcceptedProducerReplayBackupRuntime;

void SetStage1AcceptedProducerFrontDoorCurrentTickProbeDisabled(bool disabled);
bool IsStage1AcceptedProducerFrontDoorCurrentTickProbeDisabled();
void SetStage1AcceptedProducerCarrierCurrentTickProbeDisabled(bool disabled);
bool IsStage1AcceptedProducerCarrierCurrentTickProbeDisabled();
void ResetStage1ScorerHostNumericRuntimeState();
PrStage1ScorerDirectBaselineRefresh1448CResult
ApplyStage1ScorerHostResetAction80024F8C(
    Stage1NumericRuntimeState& state,
    const PrStageEventDirectResetAction80024F8C& action);
int32_t ApplyStage1ScorerHostScoreMirror80024FC0(
    Stage1NumericRuntimeState& state);
bool ResolveStage1Ctx52ReplayMode7A60(const Stage1NumericRuntimeState& state);
void UpdateStage1NumericRuntimeState(
    PrGameContext& ctx,
    const PrStageRunner& runner,
    const Stage1NumericRuntimeTimecodeInput801C7560& timecodeInput,
    bool allowTransitionFreeze = false);
PrStage1RuntimeSlotsDirectFollowUpFacts801CBFDC
BuildStage1RuntimeSlotsFollowUpFacts801CBFDC(
    const Stage1NumericRuntimeState& state);
PrStageEventDirectStage1FrameInput
BuildStage1EventStreamFrameInput801C9094(
    const Stage1NumericRuntimeState& state,
    int tick96,
    uint32_t queryFrame,
    bool allowSameQueryRefresh,
    bool clearTerminalTailPulse);

}  // namespace PrScn1
