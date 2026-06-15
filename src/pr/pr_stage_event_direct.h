#pragma once

#include "pr_stage1_overlay_parser.h"
#include "pr_stage_runner_direct.h"

#include <array>
#include <cstdint>

struct PrStageEventDirectStage1DescCursor801C9094 {
    uint32_t cursor = 0;
};

struct PrStageEventDirectStage1ConsumedScriptEvent801C9094 {
    bool valid = false;
    uint8_t streamId = 0;
    uint32_t eventIndex = 0;
    uint32_t eventFrame = 0;
    uint32_t dueFrame = 0;
    uint32_t flags04 = 0;
    uint8_t byte29 = 0;
    uint8_t byte30 = 0;
    uint8_t textId = 0;
    uint32_t psxAddr = 0;
};

struct PrStageEventDirectStage1Ctx0FinalRequirements801C9094 {
    bool finalKnown = false;
    uint32_t finalFlags = 0u;
    bool requiresSameFrameCtx = true;
    bool requiresSameFrameGlobals = true;
    bool requiresFlagStreamFlags04 = true;
    bool requiresIdStreamDescriptorState = true;
    bool requiresCompactRailRecord = true;
};

enum class PrStageEventDirectStage1ConsumedEventSource801C9094 : uint8_t {
    None = 0,
    FlagStream,
    IdStream,
};

struct PrStageEventDirectStage1FrameResult801C9094 {
    bool valid = false;
    uint32_t queryFrame = 0;
    uint32_t scriptFrame = 0;
    PrStageEventDirectStage1ConsumedScriptEvent801C9094 flagStreamEvent{};
    PrStageEventDirectStage1ConsumedScriptEvent801C9094 idStreamEvent{};
    PrStageEventDirectStage1ConsumedEventSource801C9094 consumedEventSource =
        PrStageEventDirectStage1ConsumedEventSource801C9094::None;
    PrStageEventDirectStage1ConsumedScriptEvent801C9094 consumedEvent{};
    PrStageEventDirectStage1ConsumedScriptEvent801C9094 setTextEvent801C8604{};
    PrStageEventDirectStage1ConsumedScriptEvent801C9094 compactEvent801C9094{};
    PrStageEventDirectStage1Ctx0FinalRequirements801C9094
        ctx0FinalRequirements801C9094{};
    uint8_t selectedStream = 0;
    uint8_t activeDispatchStream = 0;
    uint32_t activeDispatchStartFrame = 0;
    uint32_t activeDispatchStartScriptFrame = 0;
    bool flag2000SceneFamilyActive = false;
    uint8_t flag2000SceneFamilyStream = 0;
    uint32_t flag2000SceneFamilyStartFrame = 0;
    uint32_t flag2000SceneFamilyStartScriptFrame = 0;
    bool flag40EarlyReturnReset801C9094 = false;
};

struct PrStageEventDirectStage1CompactRailInput801C9094 {
    bool known = false;
    uint8_t recordIndex = 0;
    uint32_t psxAddr = 0;
    int16_t signedMode = 0;
    int16_t startIndex = 0;
    uint32_t primaryStreamPtr = 0;
    uint32_t secondaryStreamPtr = 0;
};

struct PrStageEventDirectStage1RunnerInputBridge801C9094 {
    bool flagStreamDescriptorValid = false;
    bool idStreamDescriptorValid = false;
    PrStageRunnerDirectEventStreamCursor801C9094 activeFlagStream{};
    PrStageRunnerDirectEventStreamCursor801C9094 activeIdStream{};
    PrStageEventDirectStage1CompactRailInput801C9094 compactRail{};
};

struct PrStageEventDirectStage1RunnerGlobalsCore801C9094 {
    bool known = false;
    uint16_t unk8008ED00 = 0;
    bool unk8008ED1C = false;
    uint16_t unk8008ED20 = 0;
    uint16_t eventStreamFlag = 1;
    uint16_t eventStreamId = 0;
    bool eventStreamDone = false;
    int32_t dword801D3048 = 0;
};

struct PrStageEventDirectResetAction80024F8C {
    bool valid = false;
    uint32_t resetPsxAddr80024F8C = 0x80024F8Cu;
    bool eventStreamResetKnown = false;
    uint16_t word8008ED36 = 0;
    uint16_t word8008ED38 = 0;
    bool runtimeSlotResetKnown = false;
    uint16_t word5A_801C369A = 0;
    bool scorerBaselineRefresh1448CRequested = false;
    uint32_t scorerBaselineRefreshPsxAddr8001448C = 0x8001448Cu;
};

struct PrStageEventDirectEventStreamReset80024E98 {
    // PSX sub_80024E98 clears dword_8008ECE0 for 244 bytes before seeding
    // these event-stream fields. Offsets are relative to 0x8008ECE0.
    uint32_t base8008ECE0 = 0x8008ECE0u;
    uint32_t clearSizeBytes80024E98 = 244u;
    int32_t dword8008ED00 = 0;       // +0x20
    int32_t dword8008ED20 = 0;       // +0x40
    uint16_t word8008ED36 = 2;       // +0x56, sub_80024F8C
    uint16_t word8008ED38 = 0;       // +0x58, sub_80024F8C
    uint16_t gPrStageEventStreamFlag = 1;
    uint16_t gPrStageEventStreamId = 0;
    uint32_t gpPlus800 = 0;          // *(_DWORD *)(gp+800)=0
};

struct PrStageEventDirectStage1FrameInput {
    uint32_t queryFrame = 0;
    uint32_t scriptFrame = 0;
    int32_t tick96 = -1;
    bool allowSameQueryRefresh = false;
    uint8_t currentMode = 0;
    uint8_t rightRankActiveRow = 0;
    bool bucket0Ctx118WritePulse = false;
    bool flag2000Pulse = false;
    bool flag4000Pulse = false;
    bool clearTerminalTailPulse = false;
    bool forcedGoodEventStreamDone = false;
    uint16_t ctx72 = 0;
    int32_t ed24 = 0;
    int32_t ed00 = 0;
    int32_t ed14 = 0;
    int32_t ed0c = 0;
    bool steadySfxDelayedCompletionPending = false;
    uint8_t followUpPhase = 0;
};

struct PrStageEventDirectStage1FrameCarrier801C9094 {
    uint32_t queryFrame = 0;
    int32_t tick96 = -1;
    bool allowSameQueryRefresh = false;
    uint8_t currentMode = 0;
    uint8_t rightRankActiveRow = 0;
    bool bucket0Ctx118WritePulse = false;
    bool flag2000Pulse = false;
    bool flag4000Pulse = false;
    bool clearTerminalTailPulse = false;
    bool forcedGoodEventStreamDone = false;
    uint16_t ctx72 = 0;
    int32_t ed24 = 0;
    int32_t ed00 = 0;
    int32_t ed14 = 0;
    int32_t ed0c = 0;
    bool steadySfxDelayedCompletionPending = false;
    uint8_t followUpPhase = 0;
    bool helper801C9094PseudoCKnown = false;
    uint32_t helper801C9094PsxAddr = 0x801C9094u;
};

struct PrStageEventDirectStage1Runtime {
    uint16_t flags40_2000_4000 = 0;
    int32_t ed24 = 0;
    uint16_t ctx72 = 0;
    int32_t ed00 = 0;
    int32_t ed20 = 0;
    int32_t ed14 = 0;
    int32_t ed0c = 0;
    uint16_t word8008ED36 = 2;       // sub_80024F8C reset/invalid sentinel.
    uint16_t word8008ED38 = 0;       // sub_80024F8C two-phase latch clear.
    uint8_t selectedStream = 0;
    uint16_t gPrStageEventStreamFlag = 1;
    uint16_t gPrStageEventStreamId = 0;
    uint32_t gpPlus800 = 0;
    PrStageEventDirectStage1RunnerGlobalsCore801C9094
        runnerGlobalsCore801C9094{};
    PrStageEventDirectEventStreamReset80024E98 eventStreamReset80024E98{};
    PrStageEventDirectResetAction80024F8C resetAction80024F8C{};
    bool gPrStageEventStreamDone = false;
    std::array<PrStageEventDirectStage1DescCursor801C9094, 9>
        unk801D2D64{};
    PrStageEventDirectStage1ConsumedScriptEvent801C9094
        flagStreamEvent801C9094{};
    PrStageEventDirectStage1ConsumedScriptEvent801C9094
        idStreamEvent801C9094{};
    PrStageEventDirectStage1FrameResult801C9094 frameResult801C9094{};
    PrStageEventDirectStage1Ctx0FinalRequirements801C9094
        ctx0FinalRequirements801C9094{};
    bool eventStreamFlagActive = true;
    uint8_t eventStreamFlagStream = 1;
    bool eventStreamFlagLastUpdateKnown = false;
    uint8_t eventStreamFlagLastUpdateReason = 0;
    uint32_t eventStreamFlagLastUpdateQueryFrame = 0;
    uint32_t eventStreamFlagLastUpdateScriptFrame = 0;
    uint16_t eventStreamFlagLastUpdatePrevious = 0;
    uint16_t eventStreamFlagLastUpdateCurrent = 1;
    bool eventStreamFlagLastChangeKnown = false;
    uint8_t eventStreamFlagLastChangeReason = 0;
    uint32_t eventStreamFlagLastChangeQueryFrame = 0;
    uint32_t eventStreamFlagLastChangeScriptFrame = 0;
    uint16_t eventStreamFlagLastChangePrevious = 0;
    uint16_t eventStreamFlagLastChangeCurrent = 1;
    uint8_t activeDispatchStream = 0;
    uint32_t activeDispatchStartFrame = 0;
    uint32_t activeDispatchStartScriptFrame = 0;
    uint32_t activeDispatchEndLocalFrame = 0;
    uint32_t activeDispatchTerminalEndLocalFrame = 0;
    bool activeDispatchTerminalPulseEmitted = false;
    bool activeDispatchDonePulseEmitted = false;
    uint32_t dword801D3048 = 0;
    uint8_t lastStartedStream = 0;
    uint32_t lastStartedQueryFrame = 0;
    bool flag100BlocksWaitPulse = false;
    uint8_t flag100SourceStream = 0;
    bool eventStreamDonePending = false;
    uint8_t eventStreamDonePendingSourceStream = 0;
    bool eventStreamDonePulse = false;
    uint8_t eventStreamDoneSourceStream = 0;
    bool prevForcedGoodEventStreamDone = false;
    uint32_t lastAdvanceScriptFrame = 0;
    bool lastClearTerminalTailPulseInput = false;
    bool lastClearTerminalTailPulseArmed = false;
    bool lastClearTerminalTailPulseBlockedAlreadyArmed = false;
    bool lastClearTerminalTailPulseBlockedActiveDispatch = false;
    bool lastClearTerminalTailPulseBlockedPendingMismatch = false;
    uint8_t lastClearTerminalTailPulseStream = 0;
    bool lastClearTerminalBranchTriggerAttempted = false;
    bool lastClearTerminalBranchTriggerAccepted = false;
    uint32_t lastClearTerminalBranchTriggerScriptFrame = 0;
    uint8_t lastClearTerminalBranchTriggerRightRankRow = 0;
    uint8_t lastClearTerminalBranchTriggerCurrentMode = 0;
    bool lastClearTerminalBranchTriggerBlockedConsumed = false;
    bool lastClearTerminalBranchTriggerBlockedArmed = false;
    bool lastClearTerminalBranchTriggerBlockedFlagNotOne = false;
    bool lastClearTerminalBranchTriggerBlockedRow = false;
    bool lastClearTerminalBranchTriggerBlockedStreamMissing = false;
    bool lastClearTerminalBranchTriggerBlockedCursorDone = false;
    bool lastClearTerminalBranchTriggerBlockedEventNotDue = false;
    bool lastClearTerminalBranchTriggerBlockedMissingFlag80 = false;
    uint16_t lastClearTerminalBranchTriggerStreamFlag = 0;
    uint32_t clearTerminalBranchTriggerAttemptCount = 0;
    uint32_t clearTerminalBranchTriggerAcceptedCount = 0;
    uint32_t clearTerminalBranchTriggerEligibleCount = 0;
    uint32_t clearTerminalBranchTriggerBlockedFlagNotOneCount = 0;
    uint32_t clearTerminalBranchTriggerBlockedRowCount = 0;
    uint32_t clearTerminalBranchTriggerBlockedFlagAndRowCount = 0;
    uint32_t clearTerminalBranchTriggerBlockedConsumedCount = 0;
    uint32_t clearTerminalBranchTriggerBlockedArmedCount = 0;
    uint32_t clearTerminalBranchTriggerBlockedStreamMissingCount = 0;
    uint32_t clearTerminalBranchTriggerBlockedCursorDoneCount = 0;
    uint32_t clearTerminalBranchTriggerBlockedEventNotDueCount = 0;
    uint32_t clearTerminalBranchTriggerBlockedMissingFlag80Count = 0;
    bool firstClearTerminalBranchTriggerKnown = false;
    uint32_t firstClearTerminalBranchTriggerScriptFrame = 0;
    uint8_t firstClearTerminalBranchTriggerRightRankRow = 0;
    uint8_t firstClearTerminalBranchTriggerCurrentMode = 0;
    uint16_t firstClearTerminalBranchTriggerStreamFlag = 0;
    bool firstClearTerminalBranchTriggerEligibleKnown = false;
    uint32_t firstClearTerminalBranchTriggerEligibleScriptFrame = 0;
    uint8_t firstClearTerminalBranchTriggerEligibleRightRankRow = 0;
    uint8_t firstClearTerminalBranchTriggerEligibleCurrentMode = 0;
    bool firstClearTerminalBranchTriggerEligibleStream1Known = false;
    uint32_t firstClearTerminalBranchTriggerEligibleStream1Cursor = 0;
    uint32_t firstClearTerminalBranchTriggerEligibleStream1Count = 0;
    bool firstClearTerminalBranchTriggerEligibleStream1DueKnown = false;
    uint32_t firstClearTerminalBranchTriggerEligibleStream1DueFrame = 0;
    int32_t firstClearTerminalBranchTriggerEligibleStream1DueDelta = 0;
    uint32_t firstClearTerminalBranchTriggerEligibleStream1BaseFrame = 0;
    uint32_t firstClearTerminalBranchTriggerEligibleStream1AbsDueFrame = 0;
    int32_t firstClearTerminalBranchTriggerEligibleStream1AbsDueDelta = 0;
    uint32_t firstClearTerminalBranchTriggerEligibleStream1Flags04 = 0;
    uint8_t firstClearTerminalBranchTriggerEligibleStream1Byte29 = 0;
    uint8_t firstClearTerminalBranchTriggerEligibleStream1Byte30 = 0;
    bool firstClearTerminalBranchTriggerEligibleStream1NextFlag80SearchKnown = false;
    bool firstClearTerminalBranchTriggerEligibleStream1NextFlag80Known = false;
    uint32_t firstClearTerminalBranchTriggerEligibleStream1NextFlag80Cursor = 0;
    uint32_t firstClearTerminalBranchTriggerEligibleStream1NextFlag80Frame = 0;
    int32_t firstClearTerminalBranchTriggerEligibleStream1NextFlag80Delta = 0;
    uint32_t firstClearTerminalBranchTriggerEligibleStream1NextFlag80BaseFrame = 0;
    uint32_t firstClearTerminalBranchTriggerEligibleStream1NextFlag80AbsFrame = 0;
    int32_t firstClearTerminalBranchTriggerEligibleStream1NextFlag80AbsDelta = 0;
    uint32_t firstClearTerminalBranchTriggerEligibleStream1NextFlag80Flags04 = 0;
    uint8_t firstClearTerminalBranchTriggerEligibleStream1NextFlag80Byte29 = 0;
    uint8_t firstClearTerminalBranchTriggerEligibleStream1NextFlag80Byte30 = 0;
    uint32_t lastClearTerminalBranchTriggerStream1Cursor = 0;
    uint32_t lastClearTerminalBranchTriggerStream1Count = 0;
    bool lastClearTerminalBranchTriggerStream1DueKnown = false;
    uint32_t lastClearTerminalBranchTriggerStream1DueFrame = 0;
    int32_t lastClearTerminalBranchTriggerStream1DueDelta = 0;
    uint32_t lastClearTerminalBranchTriggerStream1BaseFrame = 0;
    uint32_t lastClearTerminalBranchTriggerStream1AbsDueFrame = 0;
    int32_t lastClearTerminalBranchTriggerStream1AbsDueDelta = 0;
    uint32_t lastClearTerminalBranchTriggerStream1Flags04 = 0;
    uint8_t lastClearTerminalBranchTriggerStream1Byte29 = 0;
    uint8_t lastClearTerminalBranchTriggerStream1Byte30 = 0;
    bool firstClearTerminalBranchTriggerEventNotDueKnown = false;
    uint32_t firstClearTerminalBranchTriggerEventNotDueScriptFrame = 0;
    uint32_t firstClearTerminalBranchTriggerEventNotDueCursor = 0;
    uint32_t firstClearTerminalBranchTriggerEventNotDueFrame = 0;
    int32_t firstClearTerminalBranchTriggerEventNotDueDelta = 0;
    uint32_t firstClearTerminalBranchTriggerEventNotDueBaseFrame = 0;
    uint32_t firstClearTerminalBranchTriggerEventNotDueAbsFrame = 0;
    int32_t firstClearTerminalBranchTriggerEventNotDueAbsDelta = 0;
    uint32_t firstClearTerminalBranchTriggerEventNotDueFlags04 = 0;
    bool firstClearTerminalBranchTriggerMissingFlag80Known = false;
    uint32_t firstClearTerminalBranchTriggerMissingFlag80ScriptFrame = 0;
    uint32_t firstClearTerminalBranchTriggerMissingFlag80Cursor = 0;
    uint32_t firstClearTerminalBranchTriggerMissingFlag80Frame = 0;
    int32_t firstClearTerminalBranchTriggerMissingFlag80Delta = 0;
    uint32_t firstClearTerminalBranchTriggerMissingFlag80BaseFrame = 0;
    uint32_t firstClearTerminalBranchTriggerMissingFlag80AbsFrame = 0;
    int32_t firstClearTerminalBranchTriggerMissingFlag80AbsDelta = 0;
    uint32_t firstClearTerminalBranchTriggerMissingFlag80Flags04 = 0;
    bool lastClearTerminalTailServiceLatch = false;
    uint32_t lastClearTerminalTailServiceScriptFrame = 0;
    uint8_t lastClearTerminalTailServiceRightRankRow = 0;
    uint8_t lastClearTerminalTailServiceCurrentMode = 0;
    uint8_t lastClearTerminalTailServiceStream = 0;
    bool tailDispatchFamilyActive = false;
    uint8_t tailDispatchFamilyStream = 0;
    bool flag2000SceneFamilyActive = false;
    uint8_t flag2000SceneFamilyStream = 0;
    uint32_t flag2000SceneFamilyStartFrame = 0;
    uint32_t flag2000SceneFamilyStartScriptFrame = 0;
    uint32_t pendingRatingBranchSeq = 0;
    uint32_t consumedRatingBranchSeq = 0;
    bool clearTerminalTailArmed = false;
    bool clearTerminalTailDispatchActive = false;
    uint8_t clearTerminalTailStream = 0;
    bool clearTerminalTailLatchPending801C9094 = false;
    bool clearTerminalBranchTriggerConsumed801C9094 = false;
    uint8_t flags40LastDispatchedStream = 0;
    uint8_t flags2000LastDispatchedStream = 0;
    uint8_t flags4000LastDispatchedStream = 0;
    bool prevFlag40 = false;
    bool prevFlag2000 = false;
    bool prevFlag4000 = false;
    uint8_t prevFollowUpPhase = 0;
    int32_t lastQueryFrame = -1;
};

uint8_t PrStageEventDirectStage1TailStreamForMode(uint8_t currentMode);
uint8_t PrStageEventDirectStage1Flag40StreamForMode(uint8_t currentMode);
uint8_t PrStageEventDirectStage1Flag2000StreamForCtx72(uint16_t ctx72);
uint8_t PrStageEventDirectStage1AuthoritativeRow(uint16_t flags40_2000_4000,
                                                 uint16_t ctx72,
                                                 int32_t ed24);
bool PrStageEventDirectStage1IsFlag40Stream(uint8_t streamId);
bool PrStageEventDirectStage1IsFlag2000Stream(uint8_t streamId);
bool PrStageEventDirectStage1IsFlag4000Stream(uint8_t streamId);
bool PrStageEventDirectStage1IsTerminalStream(uint8_t streamId);
PrStageEventDirectResetAction80024F8C PrStageEventDirectBuildResetAction80024F8C(
    bool eventStreamResetKnown,
    uint16_t word8008ED36,
    uint16_t word8008ED38,
    bool runtimeSlotResetKnown,
    uint16_t word5A_801C369A);
PrStageEventDirectResetAction80024F8C PrStageEventDirectMergeResetActions80024F8C(
    const PrStageEventDirectResetAction80024F8C& eventAction,
    const PrStageEventDirectResetAction80024F8C& slotAction);
void PrStageEventDirectStage1Reset(PrStageEventDirectStage1Runtime& runtime);
const PrStageEventDirectResetAction80024F8C&
PrStageEventDirectStage1GetResetAction80024F8C(
    const PrStageEventDirectStage1Runtime& runtime);
const PrStageEventDirectStage1FrameResult801C9094&
PrStageEventDirectStage1GetFrameResult801C9094(
    const PrStageEventDirectStage1Runtime& runtime);
bool PrStageEventDirectStage1ConsumeClearTerminalBranchTrigger801C9094(
    PrStageEventDirectStage1Runtime& runtime,
    const PrStage1OverlayData& data,
    uint32_t scriptFrame,
    uint8_t rightRankActiveRow);
bool PrStageEventDirectStage1BuildRunnerCursor801C9094(
    const PrStageEventDirectStage1Runtime& runtime,
    const PrStage1OverlayData& data,
    uint16_t streamId,
    PrStageRunnerDirectEventStreamCursor801C9094& out);
PrStageEventDirectStage1RunnerInputBridge801C9094
PrStageEventDirectStage1BuildRunnerInputBridge801C9094(
    const PrStageEventDirectStage1Runtime& runtime,
    const PrStage1OverlayData& data);
PrStageRunnerDirectFrameUpdate9094Input
PrStageEventDirectStage1BuildRunnerFrameUpdateInput801C9094(
    const PrStageEventDirectStage1Runtime& runtime,
    const PrStage1OverlayData& data,
    bool sameFrameCtxKnown,
    const PrStageRunnerDirectContext801C9094& sameFrameCtx,
    bool sameFrameGlobalsKnown,
    const PrStageRunnerDirectGlobals801C9094& sameFrameGlobals);
struct PrStageEventDirectStage1FrameUpdateResult801C9094 {
    bool ran = false;
    PrStageRunnerDirectFrameUpdate9094Input runnerInput{};
    PrStageRunnerDirectFrameUpdate9094Result frameUpdate{};
    PrStageRunnerDirectCtx0FinalFlagsSnapshot801C9094 ctx0FinalFlags{};
};
PrStageEventDirectStage1FrameUpdateResult801C9094
PrStageEventDirectStage1RunFrameUpdate801C9094(
    PrStageEventDirectStage1Runtime& runtime,
    const PrStage1OverlayData& data,
    const PrStageEventDirectStage1FrameInput& frameInput,
    bool sameFrameCtxKnown,
    const PrStageRunnerDirectContext801C9094& sameFrameCtx,
    bool sameFrameGlobalsKnown,
    const PrStageRunnerDirectGlobals801C9094& sameFrameGlobals);
PrStageRunnerDirectGlobals801C9094
PrStageEventDirectStage1BuildRunnerGlobalsCore801C9094(
    const PrStageEventDirectStage1Runtime& runtime,
    const PrStageRunnerDirectGlobals801C9094& fallback);
void PrStageEventDirectStage1ApplyRunnerGlobalsCore801C9094(
    PrStageEventDirectStage1Runtime& runtime,
    const PrStageRunnerDirectGlobals801C9094& globals,
    uint32_t queryFrame,
    uint32_t scriptFrame);
bool PrStageEventDirectStage1RunnerGlobalsCoreKnown801C9094(
    const PrStageEventDirectStage1Runtime& runtime);
bool PrStageEventDirectStage1HasFlag(
    const PrStageEventDirectStage1Runtime& runtime,
    uint16_t mask);
uint8_t PrStageEventDirectStage1ResolveCurrentMode801C9094(
    uint8_t rightRankActiveRow);
PrStageEventDirectStage1FrameInput
PrStageEventDirectStage1BuildFrameInputFromCarrier801C9094(
    const PrStageEventDirectStage1FrameCarrier801C9094& carrier);

PrStageEventDirectStage1FrameInput
PrStageEventDirectStage1BuildFrameInputFromPrimitiveFacts801C9094(
    uint32_t queryFrame,
    int32_t tick96,
    bool allowSameQueryRefresh,
    uint8_t rightRankActiveRow,
    bool bucket0Ctx118WritePulse,
    bool flag2000Pulse,
    bool flag4000Pulse,
    bool clearTerminalTailPulse,
    bool forcedGoodEventStreamDone,
    uint16_t ctx72,
    int32_t ed24,
    int32_t ed00,
    int32_t ed14,
    int32_t ed0c,
    bool steadySfxDelayedCompletionPending,
    uint8_t followUpPhase);
void PrStageEventDirectStage1Advance(
    PrStageEventDirectStage1Runtime& runtime,
    const PrStage1OverlayData& data,
    const PrStageEventDirectStage1FrameInput& input);
