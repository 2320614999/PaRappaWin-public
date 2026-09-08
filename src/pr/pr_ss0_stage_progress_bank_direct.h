#pragma once

#include <cstdint>

namespace PrSS0StageProgressBankDirect {

static constexpr uint32_t kFn80015CC4 = 0x80015CC4u;
static constexpr uint32_t kFn80015700 = 0x80015700u;
static constexpr uint32_t kFn80015744 = 0x80015744u;
static constexpr uint32_t kFn80014344 = 0x80014344u;
static constexpr uint32_t kFn800143F0 = 0x800143F0u;
static constexpr uint32_t kFn80014D58 = 0x80014D58u;
static constexpr uint32_t kFn8001615C = 0x8001615Cu;
static constexpr uint32_t kFn800161A8 = 0x800161A8u;
static constexpr uint32_t kFn800161F4 = 0x800161F4u;
static constexpr uint32_t kFn8001628C = 0x8001628Cu;
static constexpr uint32_t kFn8001635C = 0x8001635Cu;
static constexpr uint32_t kFn800164B4 = 0x800164B4u;
static constexpr uint32_t kFn800166AC = 0x800166ACu;
static constexpr uint32_t kFn8001670C = 0x8001670Cu;
static constexpr uint32_t kFn80016758 = 0x80016758u;
static constexpr uint32_t kFn800167A8 = 0x800167A8u;
static constexpr uint32_t kFn8001681C = 0x8001681Cu;
static constexpr uint32_t kFn800169E0 = 0x800169E0u;
static constexpr uint32_t kFn80015D18 = 0x80015D18u;
static constexpr uint32_t kFn80014614 = 0x80014614u;
static constexpr uint32_t kFn80024E54 = 0x80024E54u;
static constexpr uint32_t kFn800259C0 = 0x800259C0u;
static constexpr uint32_t kFn800267F8 = 0x800267F8u;
static constexpr uint32_t kFn801C4988 = 0x801C4988u;
static constexpr uint32_t kFn801C4AB8 = 0x801C4AB8u;
static constexpr uint32_t kFn801C4FC8 = 0x801C4FC8u;
static constexpr uint32_t kFn801C7A60 = 0x801C7A60u;
static constexpr uint32_t kFn801C81EC = 0x801C81ECu;
static constexpr uint32_t kFn801C8660 = 0x801C8660u;

static constexpr uint32_t kProgressBank80092F10 = 0x80092F10u;
static constexpr uint32_t kProgressBankByteCount = 4876u;
static constexpr uint32_t kStatusBank80092F1D =
    kProgressBank80092F10 + 0x0Du;
static constexpr uint32_t kStatusBankStageSelectByteCount = 7u;
static constexpr uint32_t kStatusBankAllClearByteCount = 6u;
static constexpr uint32_t kScoreBank80092F24 =
    kProgressBank80092F10 + 0x14u;
static constexpr uint32_t kScoreBankSlotCount = 6u;
static constexpr uint32_t kLastSavedSlot80092F3C =
    kProgressBank80092F10 + 0x2Cu;
static constexpr uint32_t kPrevGrade80092F40 =
    kProgressBank80092F10 + 0x30u;
static constexpr uint32_t kAllClearFlag80092F44 =
    kProgressBank80092F10 + 0x34u;
static constexpr uint32_t kReplaySourcePtr80092F48 =
    kProgressBank80092F10 + 0x38u;
static constexpr uint32_t kReplayMirrorDst80092F5C =
    kProgressBank80092F10 + 0x4Cu;
static constexpr uint32_t kReplayMirrorSrc8008EEF8 = 0x8008EEF8u;
static constexpr uint32_t kReplayMaskMirror8008EEFC = 0x8008EEFCu;
static constexpr uint32_t kReplaySourceCarrier800901BC = 0x800901BCu;
static constexpr uint32_t kReplayRuntimeCursor800901C0 = 0x800901C0u;
static constexpr uint32_t kReplayMirrorByteCount = 4800u;
static constexpr uint32_t kReplayMirrorEntryStride = 8u;
static constexpr uint32_t kReplayMirrorMaxEntryCount = 600u;
static constexpr uint32_t kScene0ReplayTableCount801C6F84 = 0x801C6F84u;
static constexpr uint32_t kStage1ReplayTableSource801D2E2C = 0x801D2E2Cu;
static constexpr uint32_t kStage1ReplayTableEntryCount = 53u;
static constexpr uint32_t kBackupBank80079008 = 0x80079008u;
static constexpr uint32_t kMapSaveStageTable80048DB8 = 0x80048DB8u;
static constexpr uint32_t kMapSceneTable80048DD8 = 0x80048DD8u;
static constexpr uint32_t kWord800916DA = 0x800916DAu;
static constexpr uint32_t kWord800916D0 = 0x800916D0u;
static constexpr uint32_t kWord800916E2 = 0x800916E2u;
static constexpr uint32_t kWord800916F0 = 0x800916F0u;
static constexpr uint32_t kWord8008ED34 = 0x8008ED34u;
static constexpr uint32_t kWord80091816 = 0x80091816u;
static constexpr uint32_t kWord8009182A = 0x8009182Au;
static constexpr uint32_t kSavedScoreCtxOffset800169E0 = 0x30u;
static constexpr uint32_t kWord800916F0S0ReplayFrameCount = 14104u;
static constexpr uint32_t kWord800916F0S0ReplaySampleCount = 13u;
static constexpr uint32_t kWord800916F0StaticAccessReadCount = 8u;
static constexpr uint32_t kWord800916F0StaticAccessWriteCount = 0u;
static constexpr uint32_t kWord800916F0StaticImmediateStoreCount = 0u;
static constexpr uint32_t kWord800916F0StaticPointerConstructedAccessCount = 0u;
static constexpr uint32_t kWord800916F0StaticPointerConstructedWriteCount = 0u;

enum class StageProgressStatus : uint8_t {
    Locked = 0,
    Unlocked = 1,
    Clear = 2,
    HighClear = 3,
    Unknown = 0xFFu,
};

enum class ProgressBankFieldKind : uint8_t {
    Prefix = 0,
    SaveNameSuffix,
    StageStatusBytes,
    StageScoreDwords,
    LastSavedSlot,
    PreviousGrade,
    AllClearFlag,
    ReplaySourcePointer,
    ReplayMirror,
    BackupPrefix,
};

enum class ProgressBankActionKind : uint8_t {
    None = 0,
    ClearBytes80015CC4,
    CopyBytes80015700,
    GateBackupSource80015700,
    CopyBytes80015744,
    GateRestoreTarget80015744,
    CopyBytes800164B4,
    GateLoadPayloadSource800164B4,
    MapStage8001615C,
    MapScene800161A8,
    QueryAllClear800161F4,
    QueryStatus800166AC,
    GateStageStatusSource8001670C,
    ReadStageStatusGate8001670C,
    ConditionalUnlock8001628C,
    MaxPromoteStatus8001635C,
    WriteScore8001635C,
    WriteLastSlot8001635C,
    WritePrevGrade8001635C,
    WriteReplaySource8001635C,
    GateReplaySourceCarrier8001635C,
    CopyReplayMirror8001635C,
    GateReplayMirrorSource8001635C,
    WriteAllClear8001635C,
    GatePreviousGradeSource80016758,
    ReadPreviousGradeGate80016758,
    GateSetupBranchWord800916D0801C7A60,
    GateSetupLatchSource801C7A60,
    WriteWord8009182A800143F0,
    WriteWord8008ED34800259C0,
    GateSetupTailRecordsMode801C7A60,
    ClearWord8009182A800143F0,
    ClearWord8008ED34800259C0,
    WriteStatus800167A8,
    ReadScore800169E0,
    GateSavedScoreSource800169E0,
    GateSavedScoreCtxTarget800169E0,
    WriteWord80091816,
    GateWord800916E2Source80015D18,
    WriteWord800916E2,
    GateReplayProducerFamily8001635C,
    GateReplaySnapshotMirrorSource8001635C,
    GateReplayPublishedCount8001635C,
    GateReplayCursorSource8001635C,
    GateReplayRestorePayload8001681C,
    WriteReplayPublishedCount8001681C,
    CopyReplayMirrorRestore8001681C,
    GateReplaySetupTransitionState80024E54,
    PreserveReplayCursorAfterRestore8001681C,
    GateReplayAppendAccepted80014614,
    GateReplayAppendBounds80014614,
    WriteReplayAppendMask80014614,
    WriteReplayAppendTick80014614,
    WriteReplayCursorIncrement80014614,
    WriteReplayPublishedCountFromCursor80014614,
    GateReplayEventTableZeroCount801C4FC8,
    WriteReplayPublishedCountZero801C4FC8,
    PreserveReplayMirror801C4FC8,
    PreserveReplayCursor801C4FC8,
    GateReplayStage1EventTableSource801C8660,
    WriteReplayStage1EventTableTicks801C8660,
    WriteReplayStage1EventTableMasks801C8660,
    WriteReplayPublishedCount801C8660,
    PreserveReplayCursor801C8660,
    GateReplayScriptedCursorBounds801C7A60,
    GateReplayScriptedNextTick801C7A60,
    ReadReplayScriptedMask801C7A60,
    WriteReplayScriptedCursorAdvance801C7A60,
    PreserveReplayScriptedCursor801C7A60,
    PreserveReplayPublishedCount801C7A60,
    GateReplaySetupArg80024E54,
    WriteReplayCursorZero80024E54,
    WriteReplayPublishedCountZero80024E54,
    PreserveReplayMirror80024E54,
    GateWord800916F0Source800267F8,
    GateStageSelectStatusSource800267F8,
    ReadStageSelectStatus800267F8,
    ForceEnableAll800916F0,
    WriteStageSelectForceTail800267F8,
    RecordsMode800916DA,
    Gap,
};

enum class ProgressBankCopyProvenanceKind : uint8_t {
    Backup80015700 = 0,
    Restore80015744,
    LoadPayload800164B4,
};

enum class ProgressBankPayloadSourceKind : uint8_t {
    Unknown = 0,
    RuntimeLowerCardProducer,
    DebugSyntheticFixture,
    HostFilesystem,
};

enum class ReplayMirrorProducerKind : uint8_t {
    Stage1EventTableBuild = 0,
    RuntimeAcceptedAppend,
    PayloadRestore8001681C,
};

enum class ReplayMirrorResetKind : uint8_t {
    EventTableClear801C4AB8 = 0,
    EventTableClear801C4988,
    SetupReset80024E54,
};

enum class ScoreAccumulatorWriterKind : uint8_t {
    Reset80014344 = 0,
    GameplayCommit80014D58,
    SavedScoreSync800169E0,
};

enum class SavedScoreSyncSourceKind : uint8_t {
    ScoreBankSlot800169E0 = 0,
};

enum class SetupLatchSameValueCallsiteKind : uint8_t {
    StageRunner801C7A60 = 0,
};

enum class Word800916F0ConsumerKind : uint8_t {
    StageSelectForceEnable800267F8 = 0,
    ComodPostClearSaveGateFamily,
};

struct ProgressBankFieldSpec {
    ProgressBankFieldKind kind = ProgressBankFieldKind::Prefix;
    const char* name = nullptr;
    uint32_t address = 0;
    uint32_t offset = 0;
    uint32_t byteCount = 0;
    const char* producers = nullptr;
    const char* consumers = nullptr;
};

struct ProgressBankAction {
    ProgressBankActionKind kind = ProgressBankActionKind::None;
    uint32_t psxFunction = 0;
    uint32_t srcAddress = 0;
    uint32_t dstAddress = 0;
    uint32_t byteCount = 0;
    int32_t stageOrSelector = 0;
    int32_t slot = -1;
    int32_t value = 0;
    bool valueKnown = false;
    bool conditional = false;
    ProgressBankPayloadSourceKind payloadSource =
        ProgressBankPayloadSourceKind::Unknown;
};

struct ProgressBankPlan {
    const char* name = nullptr;
    bool runtimeCutoverAllowed = false;
    bool blockedByP0Gap = true;
    ProgressBankAction actions[64]{};
    uint32_t count = 0;
    bool truncated = false;
};

struct ProgressBankCopyProvenanceSpec {
    ProgressBankCopyProvenanceKind kind =
        ProgressBankCopyProvenanceKind::Backup80015700;
    uint32_t psxFunction = 0;
    uint32_t srcAddress = 0;
    uint32_t dstAddress = 0;
    uint32_t byteCount = 0;
    bool sourceAddressFixed = false;
    bool destinationAddressFixed = false;
    bool requiresKnownSource = true;
    bool requiresKnownBackup = false;
    bool lowerCardPayloadRequired = false;
    bool runtimePayloadAuthority = false;
    const char* description = nullptr;
    const char* authorityNote = nullptr;
};

struct Word800916F0S0ReplaySampleSpec {
    uint32_t frame = 0;
    uint16_t value = 0;
    const char* label = nullptr;
};

struct Word800916F0S0ReplayBaselineSpec {
    uint32_t address = 0;
    uint32_t byteCount = 0;
    uint32_t frameCount = 0;
    uint32_t expectedChangeCount = 0;
    const char* scope = nullptr;
    const char* nonAuthorityNote = nullptr;
};

struct Word800916F0StaticAccessSpec {
    const char* module = nullptr;
    uint32_t psxAddress = 0;
    uint32_t fileOffset = 0;
    const char* op = nullptr;
    bool isWrite = false;
    const char* note = nullptr;
};

struct Word800916F0StaticAccessAuditSpec {
    uint32_t targetAddress = 0;
    uint32_t directReadCount = 0;
    uint32_t directWriteCount = 0;
    uint32_t immediateStoreCount = 0;
    uint32_t pointerConstructedAccessCount = 0;
    uint32_t pointerConstructedWriteCount = 0;
    bool scusCovered = false;
    bool comodOverlaysCovered = false;
    bool pointerOrHiddenWriterStillGap = true;
    const char* scanScope = nullptr;
    const char* authorityNote = nullptr;
};

struct ReplayMirrorProducerSpec {
    ReplayMirrorProducerKind kind =
        ReplayMirrorProducerKind::Stage1EventTableBuild;
    uint32_t psxFunction = 0;
    uint32_t countSourceAddress = 0;
    uint32_t tickMirrorAddress = 0;
    uint32_t maskMirrorAddress = 0;
    uint32_t countCarrierAddress = 0;
    uint32_t cursorAddress = 0;
    uint32_t entryStrideBytes = 0;
    uint32_t staticEntryCount = 0;
    uint32_t maxEntryCount = 0;
    bool staticEntryCountKnown = false;
    bool runtimeValueAuthority = false;
    const char* description = nullptr;
    const char* authorityNote = nullptr;
};

struct ReplayMirrorResetSpec {
    ReplayMirrorResetKind kind = ReplayMirrorResetKind::EventTableClear801C4AB8;
    uint32_t psxFunction = 0;
    uint32_t countCarrierAddress = 0;
    uint32_t resetValue = 0;
    bool clearsTickMirror = false;
    bool clearsMaskMirror = false;
    bool clearsCursor = false;
    bool runtimeValueAuthority = false;
    const char* description = nullptr;
    const char* authorityNote = nullptr;
};

struct ScoreAccumulatorWriterSpec {
    ScoreAccumulatorWriterKind kind =
        ScoreAccumulatorWriterKind::Reset80014344;
    uint32_t psxFunction = 0;
    uint32_t srcAddress = 0;
    uint32_t dstAddress = 0;
    uint32_t ctxOffset = 0;
    bool writesCtxMirror = false;
    bool runtimeValueAuthority = false;
    const char* description = nullptr;
    const char* authorityNote = nullptr;
};

struct SavedScoreSyncSourceSpec {
    SavedScoreSyncSourceKind kind =
        SavedScoreSyncSourceKind::ScoreBankSlot800169E0;
    uint32_t psxFunction = 0;
    uint32_t branchWordAddress = 0;
    int32_t requiredBranchValue = 0;
    uint32_t sceneWordAddress = 0;
    uint32_t stageMapFunction = 0;
    uint32_t stageMapTableAddress = 0;
    uint32_t scoreBankAddress = 0;
    uint32_t scoreSlotCount = 0;
    uint32_t scoreSlotStride = 0;
    uint32_t ctxScoreOffset = 0;
    uint32_t ctxWriteByteCount = 0;
    uint32_t wordMirrorAddress = 0;
    uint32_t wordMirrorByteCount = 0;
    bool requiresKnownBranch = true;
    bool requiresKnownSceneWord = true;
    bool requiresKnownScoreDword = true;
    bool writesCtxScoreTarget = true;
    bool mirrorsWordLow16 = true;
    bool runtimeValueAuthority = false;
    const char* description = nullptr;
    const char* authorityNote = nullptr;
};

struct SetupLatchSameValueCallsiteSpec {
    SetupLatchSameValueCallsiteKind kind =
        SetupLatchSameValueCallsiteKind::StageRunner801C7A60;
    uint32_t overlayFunction = 0;
    uint32_t branchWordAddress = 0;
    uint32_t defaultGateFunction = 0;
    uint32_t restoreGateFunction = 0;
    uint32_t firstSetterFunction = 0;
    uint32_t secondSetterFunction = 0;
    uint32_t firstDstAddress = 0;
    uint32_t secondDstAddress = 0;
    uint32_t recordsModeWordAddress = 0;
    bool sameValueArgumentProven = false;
    bool recordsTailClearsBoth = false;
    bool globalAbsenceProof = false;
    bool runtimeValueAuthority = false;
    const char* description = nullptr;
    const char* authorityNote = nullptr;
};

struct Word800916F0ConsumerSpec {
    Word800916F0ConsumerKind kind =
        Word800916F0ConsumerKind::StageSelectForceEnable800267F8;
    uint32_t psxFunction = 0;
    uint32_t wordAddress = 0;
    int32_t specialValue = 1;
    bool valueSourceKnown = false;
    bool writerKnown = false;
    bool stageSelectForceEnable = false;
    bool postClearSaveGate = false;
    bool runtimeValueAuthority = false;
    const char* description = nullptr;
    const char* authorityNote = nullptr;
};

bool RuntimeCutoverAllowed();
bool TryMapSaveStage8001615C(int32_t stage, int32_t* slot);
bool TryMapReplayScene800161A8(int32_t selector, int32_t* scene);
bool IsKnownReplayMirrorProducerFunction(uint32_t psxFunction);

uint32_t KnownProgressBankFieldCount();
const ProgressBankFieldSpec& KnownProgressBankFieldAt(uint32_t index);
uint32_t KnownProgressBankCopyProvenanceCount();
const ProgressBankCopyProvenanceSpec& KnownProgressBankCopyProvenanceAt(
    uint32_t index);
const Word800916F0S0ReplayBaselineSpec& KnownWord800916F0S0ReplayBaseline();
uint32_t KnownWord800916F0S0ReplaySampleCount();
const Word800916F0S0ReplaySampleSpec& KnownWord800916F0S0ReplaySampleAt(
    uint32_t index);
const Word800916F0StaticAccessAuditSpec&
KnownWord800916F0StaticAccessAudit();
uint32_t KnownWord800916F0StaticAccessCount();
const Word800916F0StaticAccessSpec& KnownWord800916F0StaticAccessAt(
    uint32_t index);
uint32_t KnownReplayMirrorProducerCount();
const ReplayMirrorProducerSpec& KnownReplayMirrorProducerAt(uint32_t index);
uint32_t KnownReplayMirrorResetCount();
const ReplayMirrorResetSpec& KnownReplayMirrorResetAt(uint32_t index);
uint32_t KnownScoreAccumulatorWriterCount();
const ScoreAccumulatorWriterSpec& KnownScoreAccumulatorWriterAt(
    uint32_t index);
uint32_t KnownSavedScoreSyncSourceCount();
const SavedScoreSyncSourceSpec& KnownSavedScoreSyncSourceAt(uint32_t index);
uint32_t KnownSetupLatchSameValueCallsiteCount();
const SetupLatchSameValueCallsiteSpec& KnownSetupLatchSameValueCallsiteAt(
    uint32_t index);
uint32_t KnownWord800916F0ConsumerCount();
const Word800916F0ConsumerSpec& KnownWord800916F0ConsumerAt(uint32_t index);

ProgressBankPlan BuildClearAndSeed80015CC4Plan();
ProgressBankPlan BuildBackup80015700Plan(
    uint32_t srcAddress = kProgressBank80092F10);
ProgressBankPlan BuildRestore80015744Plan(
    bool backupKnown,
    uint32_t dstAddress = kProgressBank80092F10);
ProgressBankPlan BuildLoadPayload800164B4Plan(bool sourceKnown,
                                               uint32_t sourceAddress,
                                               ProgressBankPayloadSourceKind
                                                   payloadSource =
                                                       ProgressBankPayloadSourceKind::Unknown,
                                               bool lowerCardPayloadKnown =
                                                   false);
ProgressBankPlan BuildAllClear800161F4Plan(bool statusBankKnown);
ProgressBankPlan BuildQueryStatus800166ACPlan(int32_t stage,
                                               bool statusBankKnown);
ProgressBankPlan BuildStageStatusGate8001670CPlan(int32_t stage,
                                                  bool statusKnown,
                                                  uint8_t status);
ProgressBankPlan BuildUnlock8001628CPlan(int32_t stage,
                                          bool currentStatusKnown,
                                          int32_t currentStatus);
ProgressBankPlan BuildUpdate8001635CPlan(int32_t stage,
                                          int32_t targetStatus,
                                          int32_t previousGrade,
                                          int32_t score,
                                          bool statusBankKnown,
                                          bool replayMirrorSourceKnown,
                                          bool carrierSourceKnown,
                                          uint32_t carrierSource);
ProgressBankPlan BuildStatusWrite800167A8Plan(int32_t stage, int32_t mode);
ProgressBankPlan BuildPreviousGradeGate80016758Plan(bool prevGradeKnown,
                                                    uint32_t prevGrade);
ProgressBankPlan BuildSetupGateLatchPair801C7A60Plan(
    bool gateValueKnown,
    int32_t gateValue,
    bool recordsModeKnown,
    int32_t word800916DA);
ProgressBankPlan BuildSetupGateSource801C7A60Plan(
    bool transitionStateKnown,
    int32_t word800916D0,
    int32_t stage,
    bool statusKnown,
    uint8_t status,
    bool prevGradeKnown,
    uint32_t prevGrade,
    bool recordsModeKnown,
    int32_t word800916DA,
    bool restorePayloadKnown,
    bool restorePublishedCountKnown,
    bool restoreMirrorPayloadKnown,
    uint32_t restorePublishedCount);
ProgressBankPlan BuildCurrentSceneWrite80015D18Plan(int32_t sceneIndex,
                                                    bool sceneIndexKnown);
ProgressBankPlan BuildReplaySnapshotAuthority8001635CPlan(
    bool tickMaskMirrorKnown,
    bool fullBackingKnown,
    bool publishedCountKnown,
    bool cursorKnown,
    uint32_t publishedCount,
    uint32_t cursor,
    uint32_t producerFunction,
    bool cursorRequired = true);
ProgressBankPlan BuildReplayRestore8001681CPlan(bool payloadKnown,
                                                bool publishedCountKnown,
                                                bool mirrorPayloadKnown,
                                                uint32_t publishedCount);
ProgressBankPlan BuildReplaySetupRestore80024E54_8001681CPlan(
    bool transitionStateKnown,
    uint32_t transitionState,
    bool payloadKnown,
    bool publishedCountKnown,
    bool mirrorPayloadKnown,
    uint32_t publishedCount);
ProgressBankPlan BuildReplayAcceptedAppend80014614Plan(bool appendAccepted,
                                                       bool cursorKnown,
                                                       uint32_t cursor,
                                                       bool tickKnown,
                                                       uint32_t tick,
                                                       bool maskKnown,
                                                       uint32_t mask);
ProgressBankPlan BuildReplayScene0ZeroEventTableSeed801C4FC8Plan(
    bool countKnown,
    uint32_t count);
ProgressBankPlan BuildReplayStage1EventTableSeed801C8660Plan(
    bool sourceTableKnown);
ProgressBankPlan BuildReplayScriptedCursorConsume801C7A60Plan(
    bool cursorKnown,
    uint32_t cursor,
    bool publishedCountKnown,
    uint32_t publishedCount,
    bool currentTickKnown,
    uint32_t currentTick,
    bool nextTickKnown,
    uint32_t nextTick,
    bool maskKnown,
    uint32_t mask);
ProgressBankPlan BuildReplaySetupReset80024E54Plan(bool argumentKnown,
                                                   int32_t argument);
ProgressBankPlan BuildSavedScoreSync800169E0Plan(int32_t word800916D0,
                                                  int32_t word800916E2,
                                                  bool scoreDwordKnown);
ProgressBankPlan BuildStageSelectStatusSource800267F8Plan(
    int32_t word800916DA,
    int32_t word800916F0,
    bool statusBankKnown,
    bool word800916F0Known = false);

const char* StageProgressStatusName(StageProgressStatus status);
const char* ProgressBankFieldKindName(ProgressBankFieldKind kind);
const char* ProgressBankActionKindName(ProgressBankActionKind kind);
const char* ProgressBankCopyProvenanceKindName(
    ProgressBankCopyProvenanceKind kind);
const char* ProgressBankPayloadSourceKindName(
    ProgressBankPayloadSourceKind kind);
const char* ReplayMirrorProducerKindName(ReplayMirrorProducerKind kind);
const char* ReplayMirrorResetKindName(ReplayMirrorResetKind kind);
const char* ScoreAccumulatorWriterKindName(ScoreAccumulatorWriterKind kind);
const char* SavedScoreSyncSourceKindName(SavedScoreSyncSourceKind kind);
const char* SetupLatchSameValueCallsiteKindName(
    SetupLatchSameValueCallsiteKind kind);
const char* Word800916F0ConsumerKindName(Word800916F0ConsumerKind kind);

} // namespace PrSS0StageProgressBankDirect
