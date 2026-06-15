#pragma once

#include "pr_stage_event_direct.h"

#include <array>
#include <cstddef>
#include <cstdint>

constexpr size_t kPrStage1ScorerDirectRingPageCount = 4u;
constexpr size_t kPrStage1ScorerDirectSlotsPerPage = 32u;
constexpr size_t kPrStage1ScorerDirectSourceGroupCursorEntryCount = 128u;
constexpr size_t kPrStage1ScorerDirectReplayBufferCapacity = 600u;
constexpr uint32_t kPrStage1ScorerDirectReplayMirrorByteCount =
    static_cast<uint32_t>(kPrStage1ScorerDirectReplayBufferCapacity * 2u *
                          sizeof(uint32_t));
constexpr uint32_t kPrStage1ScorerDirectFn801C4FC8 = 0x801C4FC8u;
constexpr uint32_t kPrStage1ScorerDirectFn801C8660 = 0x801C8660u;
constexpr uint32_t kPrStage1ScorerDirectFn80014614 = 0x80014614u;
constexpr uint32_t kPrStage1ScorerDirectFn8001681C = 0x8001681Cu;

struct PrStage1ScorerDirectRawRecord {
    uint32_t dword00AcceptedMask = 0;
    uint16_t word04Companion = 0;
    uint16_t word06Occupied = 0;
    uint32_t dword08Payload = 0;
};

struct PrStage1ScorerDirectPage {
    std::array<PrStage1ScorerDirectRawRecord, kPrStage1ScorerDirectSlotsPerPage>
        records{};
};

struct PrStage1ScorerDirectDescriptorBranch {
    uint8_t byte01AnchorSlotIndex = 0;
    uint8_t byte02RequiredClassToken = 0;
    uint8_t byte03PenaltyWeight = 0;
    uint32_t dword08RequiredMask = 0;
    uint16_t word0ELookbackPageCount = 0;
    uint16_t word10FlagWord = 0;
};

struct PrStage1ScorerDirectDescriptorRow {
    bool valid = false;
    uint8_t byte00LessonId = 0;
    PrStage1ScorerDirectDescriptorBranch defaultBranch{};
    PrStage1ScorerDirectDescriptorBranch substate1Branch{};
    int16_t word10FinalResolutionThreshold = 0;
    uint8_t byte12DefaultSelector0 = 0;
    uint8_t byte13DefaultSelector1 = 0;
    uint8_t byte18Substate1Selector0 = 0;
    uint8_t byte19Substate1Selector1 = 0;
};

struct PrStage1ScorerDirectGlobals {
    // PSX main-exe global `word_800916E2` current_scene_index.
    // Stage1 direct-port scorer currently runs only under Scene1, but keep the
    // field explicit because `14D58/14A80` coverage shrink gates on
    // `word_800916E2 == 1`.
    uint16_t word916E2CurrentSceneIndex = 1u;
    uint32_t dword91808AcceptedMask = 0;
    uint32_t dword9180CLastClearedAcceptedMask = 0;
    uint16_t word91810AcceptedCount = 0;
    uint16_t word91812RecordWriteShadowCount = 0;
    uint16_t word91814PenaltySplitCount = 0;
    int16_t word91816Accumulator = 0;
    int16_t word91818SharedBaseline = 0;
    int16_t word9181ASnapshot = 0;
    int16_t word9181CCompareBaseline = 0;
    uint16_t word9181ETwitterCarryLatch = 0;
    uint16_t word91820NoInputCounter = 0;
    int16_t word91822AdditiveTerm = 0;
    uint16_t word91824RecordCompanion = 0;
    uint16_t word91826CoolGainLatch = 0;
    uint16_t word91828CoolGainStreak = 0;
    uint16_t word9182APairBonusGate = 0;

    int32_t dword8ED00FollowUpState = 0;
    int32_t dword8ED08DescriptorFlags = 0;
    int32_t dword8ED0CFollowUpDeadlineTick = 0;
    int32_t dword8ED14FollowUpActive = 0;
    int32_t dword8ED24FollowUpWriteback = 0;
    uint16_t word8ED34ShortWindowLatch = 0;
    uint16_t word8ED36Phase1Cache = 0;
    uint16_t word8ED38PhaseCounter = 0;

    bool gPrStageEventStreamFlagKnown = false;
    bool gPrStageEventStreamFlag = true;
    bool gPrStageEventStreamIdKnown = false;
    uint8_t gPrStageEventStreamId = 0;
    bool gPrStageEventStreamDone = false;

    uint8_t currentPageOrdinal1Based = 0;
    bool currentWritePageOrdinalKnown = false;
    uint8_t currentWritePageOrdinal1Based = 0;
    bool pageClearPending14BDC = false;
    uint8_t pageClearOrdinal1Based14BDC = 0;
    std::array<PrStage1ScorerDirectPage, kPrStage1ScorerDirectRingPageCount>
        ringPages{};
};

struct PrStage1ScorerDirectBucketContext {
    uint32_t dword0CTick96 = 0;
    uint16_t word54Permit = 0;
    uint16_t word4ERightRankActiveRow = 0;
    uint16_t word50DescriptorSubstate = 0;
    uint16_t word56WritePageOrdinal1Based = 0;
    uint16_t word6ABucket0ConsumerGate = 0;
    uint16_t word72FollowUpCounter = 0;
    uint16_t word74HelperCounter = 0;
    uint16_t word7AFollowUpState = 0;
    uint16_t word18ETransitionAnim = 0;
    int32_t dword30ScoreDisplayValue = 0;
    uint16_t word58BlinkTargetRow = 0;
    uint16_t word5ABlinkEnabled = 0;
    bool descriptorPointerAvailable = false;
};

struct PrStage1ScorerDirectAcceptedPacket {
    bool valid = false;
    bool materializeClass2 = false;
    uint32_t dword08RouteMask = 0;
    uint32_t dword08SourceCellPtr = 0;
    uint32_t dword08CallbackArgOpaque = 0;
    uint16_t word06RecordCompanion = 0;
    bool dword08PayloadNonZero = false;
};

struct PrStage1ScorerDirectSourceGroupCursorEntry {
    bool occupied = false;
    uint32_t dword00HeaderAddr = 0;
    uint32_t dword04BasePtr = 0;
    uint16_t word08Count = 0;
    uint16_t word0ACursor = 0;
};

struct PrStage1ScorerDirectAcceptedProducerOwnerState {
    uint32_t dword91800LastClassToken = 0;
    std::array<PrStage1ScorerDirectSourceGroupCursorEntry,
               kPrStage1ScorerDirectSourceGroupCursorEntryCount>
        cursorEntries{};
};

struct PrStage1ScorerDirectSourceCellHeader {
    bool valid = false;
    uint32_t dword00HeaderAddr = 0;
    uint32_t dword04BasePtr = 0;
    uint16_t word08Count = 0;
    uint16_t word0ACursor = 0;
};

struct PrStage1ScorerDirectSourceCell {
    bool valid = false;
    uint32_t dword00SourceCellPtr = 0;
    uint8_t byte00Program = 0;
    uint8_t byte01Note = 0;
    uint8_t byte02Key = 0;
    uint8_t byte03Volume = 0;
    uint16_t word06RecordCompanion = 0;
    bool dword08CallbackArgPresent = false;
    uint32_t dword08PayloadOpaque = 0;
};

struct PrStage1ScorerDirectAcceptedProducerAccessors {
    void* userData = nullptr;
    bool (*readTimingTemplateState)(void* userData,
                                    uint8_t selectorByte1,
                                    uint8_t slot48,
                                    uint8_t& outState) = nullptr;
    bool (*readSourceCellHeader)(void* userData,
                                 uint8_t selectorByte0,
                                 uint8_t classToken20,
                                 PrStage1ScorerDirectSourceCellHeader& outHeader) = nullptr;
    bool (*readSourceCell)(void* userData,
                           uint32_t sourceCellPtr,
                           PrStage1ScorerDirectSourceCell& outCell) = nullptr;
};

struct PrStage1ScorerDirectAcceptedProducerCoreInput {
    bool busyGate24BF4Active = false;
    bool eventStreamFlagActive = false;
    uint16_t writerControlSample18 = 0;
    bool classToken20Known = false;
    uint8_t classToken20 = 0;
    bool acceptedTick96Known = false;
    int32_t acceptedTick96 = 0;
    uint8_t halfWindow34 = 0;
    uint16_t descriptorSubstate50 = 0;
    int32_t writePageOrdinal38 = -1;
    bool replayLogBlocked82 = false;
    const PrStage1ScorerDirectDescriptorRow* lookaheadDescriptorRow = nullptr;
};

struct PrStage1ScorerDirectReplayAppendIntent {
    bool append = false;
    bool blockedByWord82 = false;
    uint32_t dwordEEF8Tick96 = 0;
    uint32_t dwordEEFCClassMask = 0;
};

struct PrStage1ScorerDirectReplayBufferState {
    bool replayMirrorKnown8008EEF8 = false;
    bool replayMirrorProducerKnown8008EEF8 = false;
    uint32_t replayMirrorProducerFunction = 0;
    bool replayMirrorByteCountKnown8008EEF8 = false;
    uint32_t replayMirrorKnownByteCount8008EEF8 = 0;
    uint32_t dword901C0WriteCount = 0;
    uint32_t dword901BCPublishedCount = 0;
    std::array<uint32_t, kPrStage1ScorerDirectReplayBufferCapacity> dwordEEF8Tick96{};
    std::array<uint32_t, kPrStage1ScorerDirectReplayBufferCapacity> dwordEEFCClassMask{};
};

struct PrStage1ScorerDirectReplayBackupState {
    bool valid = false;
    uint32_t dword92F48PublishedCount = 0;
    std::array<uint32_t, kPrStage1ScorerDirectReplayBufferCapacity> dwordEEF8Tick96{};
    std::array<uint32_t, kPrStage1ScorerDirectReplayBufferCapacity> dwordEEFCClassMask{};
};

struct PrStage1ScorerDirectReplayBackupCaptureResult {
    bool captureApplied = false;
    PrStage1ScorerDirectReplayBackupState backup{};
};

struct PrStage1ScorerDirectEventTableBuild801C8660Result {
    bool applied = false;
    uint32_t sourceTableAddr801D2E2C = 0x801D2E2Cu;
    uint32_t outputBase8008EEF8 = 0x8008EEF8u;
    uint32_t count800901BC = 0;
};

struct PrStage1ScorerDirectAcceptedProducerInput {
    bool busyGate24BF4Active = false;
    bool eventStreamFlagActive = false;
    bool descriptorPointerAvailable68 = false;
    bool byte12StreamIdValid = false;
    uint8_t byte12StreamId = 0;
    bool byte13SourceGroupValid = false;
    uint8_t byte13SourceGroup = 0;
    uint32_t dword18AcceptedClassMask = 0;
    bool word52ReplayMode = false;
    bool word82ReplayLogBlocked = false;
    int32_t dword10Tick96 = 0;
    uint8_t byte34HalfWindow = 0;
    uint16_t word48Phase384 = 0;
    int32_t dword38WritePageOrdinal = -1;
    uint8_t byte24RecordSlot = 0;
    uint8_t byte24RecordRemainder = 0;
    bool recordedSplit = false;
    PrStage1ScorerDirectAcceptedPacket packet{};
};

struct PrStage1ScorerDirectAcceptedProducerResolvedInput {
    int32_t resultCode = -1;
    bool selectorResolved = false;
    uint8_t selectorByte0 = 0;
    uint8_t selectorByte1 = 0;
    uint8_t timingTemplateSlot48 = 0;
    uint8_t timingTemplateState = 0;
    uint16_t remappedWriterControl18 = 0;
    uint8_t classToken20 = 0;
    uint16_t phase384 = 0;
    uint8_t recordSlot24 = 0;
    uint8_t recordRemainder24 = 0;
    bool recordedSplit = false;
    uint16_t sourceCellCursor = 0;
    PrStage1ScorerDirectSourceCell sourceCell{};
    PrStage1ScorerDirectAcceptedPacket packet{};
    PrStage1ScorerDirectReplayAppendIntent replayAppend{};
    PrStage1ScorerDirectAcceptedProducerInput writeback{};
};

struct PrStage1ScorerDirectAcceptedProducerResult {
    int32_t resultCode = -1;
    bool packetConsumed = false;
    bool recordWritten = false;
    bool penaltySideIncremented = false;
    bool aggregateMaskUpdated = false;
    bool recordCompanionWritten = false;
};

struct PrStage1ScorerDirectPageStorageApply14614Result {
    bool recordedWriteApplied = false;
    int32_t writePageOrdinal = -1;
    uint8_t recordSlot = 0u;
    uint32_t acceptedMask = 0u;
    uint16_t pageCompanion = 0u;
    uint16_t occupiedCount = 0u;
    uint32_t sourceCellPtr = 0u;
};

struct PrStage1ScorerDirectBaselineRefresh1448CResult {
    bool applied = false;
    bool sourceReset80024F8C = false;
    uint32_t sourceResetPsxAddr80024F8C = 0;
    uint32_t psxAddr8001448C = 0x8001448Cu;
    int16_t word80091816Accumulator = 0;
    int16_t word80091818SharedBaseline = 0;
    int16_t word8009181ASnapshot = 0;
    int16_t word8009181CCompareBaseline = 0;
    uint16_t returnValue = 0;
};

struct PrStage1ScorerDirectFinalPageMaintenance14BDC_14614Result {
    bool valid = false;
    bool pageKnown = false;
    uint8_t pageOrdinal1Based = 0u;
    bool clearApplied = false;
    bool clearPageKnown = false;
    uint8_t clearPageOrdinal1Based = 0u;
    bool clearBeforeWrite = false;
    bool slotWriteApplied = false;
    uint8_t recordSlot = 0u;
    PrStage1ScorerDirectRawRecord rawRecord{};
};

struct PrStage1ScorerDirectAcceptedProducerSideEffectResult {
    bool playSourceCellCue = false;
    uint8_t cueProgram = 0u;
    uint8_t cueNote = 0u;
    uint8_t cueKey = 0u;
    uint8_t cueVolume = 0u;
    bool callbackArgPresent = false;
    uint32_t callbackArgOpaque = 0u;
};

struct PrStage1ScorerDirectAcceptedProducerRunResult {
    int32_t resultCode = -1;
    bool writeRan = false;
    bool replayAppendRan = false;
    PrStage1ScorerDirectAcceptedProducerResolvedInput resolved{};
    PrStage1ScorerDirectAcceptedProducerSideEffectResult sideEffect{};
    PrStage1ScorerDirectAcceptedProducerResult writeResult{};
    PrStage1ScorerDirectPageStorageApply14614Result pageStorageApply{};
};

struct PrStage1ScorerDirectAcceptedReplaySetupResult {
    bool restoreReplayBuffer1681CRequested = false;
};

struct PrStage1ScorerDirectAcceptedReplayRestoreResult {
    bool restoreApplied = false;
    bool scriptedWriterResetRequired = false;
};

enum class PrStage1ScorerDirectReplayRestoreSource1681C : uint8_t {
    None = 0,
    Payload92F48_92F5C = 1,
    Sidecar = 2,
};

struct PrStage1ScorerDirectResolvedReplayBackup1681C {
    PrStage1ScorerDirectReplayBackupState backup{};
    PrStage1ScorerDirectReplayRestoreSource1681C source =
        PrStage1ScorerDirectReplayRestoreSource1681C::None;
};

struct PrStage1ScorerDirectAcceptedSpecialSetupResult {
    PrStage1ScorerDirectAcceptedReplaySetupResult setup{};
    PrStage1ScorerDirectAcceptedReplayRestoreResult restore{};
    PrStage1ScorerDirectReplayRestoreSource1681C restoreSource =
        PrStage1ScorerDirectReplayRestoreSource1681C::None;
};

struct PrStage1ScorerDirectStageSelectSaveStatusCore1635CResult {
    bool slotValid = false;
    int32_t slotIndex = -1;
    uint32_t dword92F24ScoreValue = 0;
    uint32_t dword92F3CLastSavedSlot = 0;
    uint32_t dword92F44AllClearLatch = 0;
};

struct PrStage1ScorerDirectStageUnlockCore1628CResult {
    bool slotValid = false;
    bool promoteRequired = false;
    int32_t resultingStatus = 0;
};

struct PrStage1ScorerDirectClearTerminalSaveInputs1635C {
    int32_t statusPromotion = 2;
    uint32_t prevGrade92F40 = 1u;
};

struct PrStage1ScorerDirectClearTerminalSaveTailCoreResult {
    PrStage1ScorerDirectClearTerminalSaveInputs1635C clearInputs{};
    PrStage1ScorerDirectStageUnlockCore1628CResult unlockResult{};
};

struct PrStage1ScorerDirectBranchState {
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
    bool anchorClassMatchKnown = false;
    int32_t anchorClassMatchValue = 0;
};

struct PrStage1ScorerDirectCommitState {
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
    PrStage1ScorerDirectBranchState defaultBranch{};
    PrStage1ScorerDirectBranchState specialBranch{};
    PrStage1ScorerDirectBranchState activeBranch{};
    bool commitTermKnown = false;
    int32_t commitTermValue = 0;
    bool scoreWritebackKnown = false;
    int32_t scoreWritebackValue = 0;
    bool clampActiveKnown = false;
    bool clampActive = false;
};

struct PrStage1ScorerDirectCommitSliceResult {
    bool busyGateActive = false;
    bool scorerWindowOpen = false;
    bool scorerRan = false;
    bool noInputCounterRan = false;
    PrStage1ScorerDirectCommitState commit{};
};

struct PrStage1ScorerDirectPhase1StepResult {
    uint8_t sampledClassifier = 2u;
    bool firstBeat = false;
    bool blinkEmit = false;
    uint16_t blinkTargetRow = 0u;
    bool directConsumerSlotKnown = false;
    uint16_t directConsumerSlot = 0u;
};

struct PrStage1ScorerDirectResolutionResult {
    bool resolutionCalled = false;
    uint8_t resolutionV22 = 2u;
    bool tieBreakerCalled = false;
    int32_t tieBreakerResult14548 = 2;
    uint8_t resolutionInputPhase1Classifier36 = 2u;
};

enum class PrStage1ScorerDirectFollowUpPhaseAction : uint8_t {
    None = 0u,
    SetAwaitBucket31AfterGoodToCool,
    SetAwaitForceGoodWriteback,
    ClearToNone,
};

enum class PrStage1ScorerDirectAcceptedClearAction : uint8_t {
    None = 0u,
    DeferBucket30,
    DeferBucket31,
    ClearBucket30Now,
};

enum class PrStage1ScorerDirectSteadyCueId : uint8_t {
    None = 0u,
    Bucket30Verdict,
    Bucket30RowCommit,
    Bucket31Completion,
    Bucket30DirectCue94400,
};

struct PrStage1ScorerDirectSteadySfxAction {
    bool play = false;
    bool clearMirror = false;
    bool callbackSiteReached = false;
    PrStage1ScorerDirectSteadyCueId cueId =
        PrStage1ScorerDirectSteadyCueId::None;
    uint8_t tableSlot = 0u;
};

struct PrStage1ScorerDirectBucket30ResolvedPublish {
    bool bucket30ResolutionKnown = false;
    uint8_t bucket30ResolutionV22 = 2u;
    bool resolvedRightRankRowKnown = false;
    uint8_t resolvedRightRankRow = 0u;
    bool rightRankWritebackCommitted = false;
    bool rowWriteEventKnown = false;
    uint8_t rowWritePreviousRow = 0u;
};

struct PrStage1ScorerDirectBucket30RowWriteDecision {
    bool resolutionKnown = false;
    uint8_t resolutionV22 = 2u;
    bool resolvedRightRankRowKnown = false;
    uint8_t resolvedRightRankRow = 0u;
    bool rightRankWritebackCommitted = false;
    bool goodToCoolCommitted = false;
    bool directConsumerFallback94400 = false;
};

struct PrStage1ScorerDirectBucket30RowWriteSliceResult {
    PrStage1ScorerDirectBucket30RowWriteDecision rowWrite{};
    bool playPrepareCue = false;
    bool prepareCueUseRepeatVariant = false;
    bool ctxFlag2000Pulse = false;
    PrStage1ScorerDirectFollowUpPhaseAction followUpPhaseAction =
        PrStage1ScorerDirectFollowUpPhaseAction::None;
};

struct PrStage1ScorerDirectBucket30DelayedTailSliceResult {
    bool playArmCue = false;
    bool flag4000Set = false;
    bool publishForcedGoodRowWrite = false;
    PrStage1ScorerDirectBucket30ResolvedPublish forcedGoodPublish{};
    PrStage1ScorerDirectFollowUpPhaseAction followUpPhaseAction =
        PrStage1ScorerDirectFollowUpPhaseAction::None;
    bool armBucket31DelayedSteadySfxCompletion = false;
};

struct PrStage1ScorerDirectBucket31ConsumerSliceResult {
    bool consumerPackageRan = false;
    bool ctxFlag0200Pulse = false;
    bool bucketLocalClearRan = false;
    bool narrowClearFired = false;
    bool clearDeferredAdditiveBookkeeping = false;
    PrStage1ScorerDirectFollowUpPhaseAction followUpPhaseAction =
        PrStage1ScorerDirectFollowUpPhaseAction::None;
    bool playCompletionCue = false;
    bool clearDelayedCompletionPending = false;
};

struct PrStage1ScorerDirectBucket30AcceptedClearDecision {
    PrStage1ScorerDirectAcceptedClearAction action =
        PrStage1ScorerDirectAcceptedClearAction::None;
    bool markBucket31AdditiveClearPending = false;
    bool clearAdditiveBookkeepingNow = false;
};

struct PrStage1ScorerDirectBucket30ClearSliceResult {
    PrStage1ScorerDirectAcceptedClearAction action =
        PrStage1ScorerDirectAcceptedClearAction::None;
    bool bucketLocalClearRan = false;
    bool markBucket31AdditiveClearPending = false;
    bool clearDeferredAdditiveBookkeeping = false;
};

struct PrStage1ScorerDirectBucket30OwnerSliceInput {
    int32_t preBucket30Ed00 = 0;
    bool hadBucket30DeferredClear = false;
    bool followUpPhaseIsNone = true;
    bool goodToCoolGateEnabled = false;
    int32_t tick96 = 0;
    int32_t goodToCoolDelayTick96 = 0;
};

struct PrStage1ScorerDirectBucket30OwnerSliceResult {
    uint16_t descriptorFlagWord = 0u;
    bool busyGateActive = false;
    bool scorerWindowOpen = false;
    bool ownerKernelOpen = false;
    PrStage1ScorerDirectCommitSliceResult commitSlice{};
    PrStage1ScorerDirectPhase1StepResult phase1{};
    PrStage1ScorerDirectResolutionResult resolution{};
    PrStage1ScorerDirectBucket30ResolvedPublish resolvedPublish{};
    PrStage1ScorerDirectBucket30RowWriteSliceResult rowWrite{};
    PrStage1ScorerDirectBucket30AcceptedClearDecision clearDecision{};
    PrStage1ScorerDirectBucket30ClearSliceResult clearSlice{};
    PrStage1ScorerDirectBucket30DelayedTailSliceResult delayedTail{};
    PrStage1ScorerDirectSteadySfxAction steadySfx{};
    int32_t scoreDisplayValue = 0;
};

struct PrStage1ScorerDirectBucket31DispatcherInput {
    int32_t preBucket31Ed00 = 0;
    bool awaitBucket31AfterGoodToCoolActive = false;
    bool bucket31NarrowClearPending = false;
    bool bucket31AdditiveClearPending = false;
    bool runPageClear14BDC = false;
};

struct PrStage1ScorerDirectPageClear14BDCResult {
    bool requested = false;
    bool targetKnown = false;
    uint8_t targetOrdinal1Based = 0u;
    bool clearApplied = false;
};

struct PrStage1ScorerDirectBucket31DispatcherResult {
    PrStage1ScorerDirectPageClear14BDCResult pageClear{};
    PrStage1ScorerDirectBucket31ConsumerSliceResult consumer{};
};

struct PrStage1ScorerDirectBucket0WindowSliceResult {
    bool callWindowOpen = false;
    bool ctx118WritePulse = false;
};

struct PrStage1ScorerDirectBucket0DescriptorCommitResult {
    bool committedDescriptorWrite = false;
    uint16_t committedDescriptorRowIndex = 0u;
    bool descriptorFlagWordKnown = false;
    uint16_t descriptorFlagWord = 0u;
    bool lessonIdWrite = false;
    uint8_t lessonId = 0u;
    bool lessonVisibleWrite = false;
    bool lessonVisible = false;
};

uint16_t PrStage1ScorerDirectInjectAdditive14C5C(
    PrStage1ScorerDirectGlobals& globals,
    int16_t additiveValue);

PrStage1ScorerDirectBaselineRefresh1448CResult
PrStage1ScorerDirectRunBaselineRefresh1448C(
    PrStage1ScorerDirectGlobals& globals);

PrStage1ScorerDirectBaselineRefresh1448CResult
PrStage1ScorerDirectApplyResetAction80024F8C(
    PrStage1ScorerDirectGlobals& globals,
    const PrStageEventDirectResetAction80024F8C& action);

int32_t PrStage1ScorerDirectPublishScoreMirror80024FC0(
    const PrStage1ScorerDirectGlobals& globals,
    PrStage1ScorerDirectBucketContext& ctx);

PrStage1ScorerDirectPageClear14BDCResult
PrStage1ScorerDirectRunPageClearCore14BDC(
    PrStage1ScorerDirectGlobals& globals,
    uint16_t pageOrdinal1Based,
    bool runPageClear14BDC);

PrStage1ScorerDirectReplayBackupCaptureResult
PrStage1ScorerDirectRunReplayBackupCaptureCore1635C(
    const PrStage1ScorerDirectReplayBufferState& replay);

bool PrStage1ScorerDirectIsKnownReplayMirrorProducerFunction(
    uint32_t psxFunction);

PrStage1ScorerDirectEventTableBuild801C8660Result
PrStage1ScorerDirectBuildStage1EventTable801C8660(
    PrStage1ScorerDirectReplayBufferState& replay);

PrStage1ScorerDirectAcceptedSpecialSetupResult
PrStage1ScorerDirectRunAcceptedSpecialSetupCore24E54_1681C(
    uint16_t transitionState,
    const PrStage1ScorerDirectResolvedReplayBackup1681C& resolvedBackup,
    PrStage1ScorerDirectReplayBufferState& replay);

PrStage1ScorerDirectResolvedReplayBackup1681C
PrStage1ScorerDirectResolveReplayRestoreSource1681C(
    bool payloadBackupValid,
    const PrStage1ScorerDirectReplayBackupState& payloadBackup,
    bool sidecarBackupValid,
    const PrStage1ScorerDirectReplayBackupState& sidecarBackup);

PrStage1ScorerDirectStageSelectSaveStatusCore1635CResult
PrStage1ScorerDirectRunStageSelectSaveStatusCore1635C(
    int32_t stage,
    int32_t score,
    const int16_t* stageSelectStatus);

PrStage1ScorerDirectClearTerminalSaveTailCoreResult
PrStage1ScorerDirectRunClearTerminalSaveTailCore1635C_1628C(
    uint16_t rightRankActiveRow,
    int32_t currentStageStatus166AC,
    int32_t nextStage,
    int32_t nextStageCurrentStatus);

PrStage1ScorerDirectBucket30OwnerSliceResult
PrStage1ScorerDirectRunBucket30OwnerSlice24FD0(
    PrStage1ScorerDirectGlobals& globals,
    PrStage1ScorerDirectBucketContext& ctx,
    const PrStage1ScorerDirectDescriptorRow& row,
    const PrStage1ScorerDirectBucket30OwnerSliceInput& in);

PrStage1ScorerDirectBucket31DispatcherResult
PrStage1ScorerDirectRunBucket31Dispatcher24FD0(
    PrStage1ScorerDirectGlobals& globals,
    PrStage1ScorerDirectBucketContext& ctx,
    const PrStage1ScorerDirectBucket31DispatcherInput& in);

PrStage1ScorerDirectBucket0WindowSliceResult
PrStage1ScorerDirectRunBucket0WindowSlice24FD0(
    PrStage1ScorerDirectGlobals& globals,
    PrStage1ScorerDirectBucketContext& ctx,
    uint16_t descriptorFlagWord);

size_t PrStage1ScorerDirectResolvePageOrdinal56_24FD0(
    int32_t tick96);

size_t PrStage1ScorerDirectResolveLookaheadRowIndex44_24FD0(
    int32_t tick96,
    uint16_t word916D0);

uint16_t PrStage1ScorerDirectResolveDescriptorFlagWordWithLatch24FD0(
    bool latchedFlagWordKnown,
    uint16_t latchedFlagWord);

PrStage1ScorerDirectAcceptedProducerRunResult
PrStage1ScorerDirectRunAcceptedProducer14614(
    PrStage1ScorerDirectGlobals& globals,
    PrStage1ScorerDirectAcceptedProducerOwnerState& ownerState,
    PrStage1ScorerDirectReplayBufferState& replay,
    const PrStage1ScorerDirectAcceptedProducerCoreInput& in,
    const PrStage1ScorerDirectAcceptedProducerAccessors& accessors);

PrStage1ScorerDirectAcceptedProducerRunResult
PrStage1ScorerDirectRunAcceptedProducerWithLatentPageClear14614(
    PrStage1ScorerDirectGlobals& globals,
    PrStage1ScorerDirectAcceptedProducerOwnerState& ownerState,
    PrStage1ScorerDirectReplayBufferState& replay,
    const PrStage1ScorerDirectAcceptedProducerCoreInput& in,
    const PrStage1ScorerDirectAcceptedProducerAccessors& accessors,
    const PrStage1ScorerDirectPageClear14BDCResult* sameTickPageClear);

PrStage1ScorerDirectFinalPageMaintenance14BDC_14614Result
PrStage1ScorerDirectResolveFinalPageMaintenance14BDC_14614(
    const PrStage1ScorerDirectPageClear14BDCResult* sameTickPageClear,
    const PrStage1ScorerDirectAcceptedProducerRunResult& acceptedRun);
