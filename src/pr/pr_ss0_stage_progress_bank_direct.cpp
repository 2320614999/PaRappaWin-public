#include "pr_ss0_stage_progress_bank_direct.h"

namespace PrSS0StageProgressBankDirect {
namespace {

constexpr int32_t kSaveStageMap80048DB8[] = {
    0, 0, 1, 2, 3, 4, 5, 6,
};

constexpr int32_t kReplaySceneMap80048DD8[] = {
    1, 2, 3, 4, 5, 6,
};

constexpr ProgressBankFieldSpec kFields[] = {
    {ProgressBankFieldKind::Prefix,
     "byte_80092F10 save/status prefix",
     kProgressBank80092F10,
     0x0000u,
     kProgressBankByteCount,
     "80015CC4, 800164B4, 8001635C, 8001628C, 800167A8, 80015744, 800185D0 pointer writes",
     "80019414/80019284, 80019148/80019458, 80015700"},
    {ProgressBankFieldKind::SaveNameSuffix,
     "save suffix/name field",
     kProgressBank80092F10 + 0x01u,
     0x0001u,
     0,
     "800185D0 writes through a3+1 when a3 is 80092F10",
     "80019458 state 15 consumes a3 payload"},
    {ProgressBankFieldKind::StageStatusBytes,
     "byte_80092F1D..80092F23 stage status bytes",
     kStatusBank80092F1D,
     0x000Du,
     kStatusBankStageSelectByteCount,
     "8001628C, 8001635C, 800167A8, 800164B4, 80015744",
     "800267F8 stage select, 800166AC, 800161F4"},
    {ProgressBankFieldKind::StageScoreDwords,
     "dword_80092F24[6] score bank",
     kScoreBank80092F24,
     0x0014u,
     kScoreBankSlotCount * 4u,
     "8001635C",
     "800169E0 when word_800916D0 == 2"},
    {ProgressBankFieldKind::LastSavedSlot,
     "dword_80092F3C last slot",
     kLastSavedSlot80092F3C,
     0x002Cu,
     4u,
     "8001635C",
     "save/status consumers"},
    {ProgressBankFieldKind::PreviousGrade,
     "dword_80092F40 previous grade carrier",
     kPrevGrade80092F40,
     0x0030u,
     4u,
     "8001635C writes previousGrade or 1",
     "save/status consumers"},
    {ProgressBankFieldKind::AllClearFlag,
     "dword_80092F44 all-clear latch",
     kAllClearFlag80092F44,
     0x0034u,
     4u,
     "8001635C writes 800161F4(byte_80092F1D)",
     "stage-select/bonus consumers"},
    {ProgressBankFieldKind::ReplaySourcePointer,
     "dword_80092F48 replay source pointer",
     kReplaySourcePtr80092F48,
     0x0038u,
     4u,
     "8001635C writes dword_800901BC",
     "8001681C restore path"},
    {ProgressBankFieldKind::ReplayMirror,
     "byte_80092F5C replay mirror",
     kReplayMirrorDst80092F5C,
     0x004Cu,
     kReplayMirrorByteCount,
     "8001635C copies byte_8008EEF8",
     "8001681C restore path"},
    {ProgressBankFieldKind::BackupPrefix,
     "unk_80079008 backup prefix",
     kBackupBank80079008,
     0x0000u,
     kProgressBankByteCount,
     "80015700 copies current prefix out",
     "80015744 restores prefix when a1 is 80092F10"},
};

constexpr ProgressBankCopyProvenanceSpec kProgressBankCopyProvenances[] = {
    {ProgressBankCopyProvenanceKind::Backup80015700,
     kFn80015700,
     kProgressBank80092F10,
     kBackupBank80079008,
     kProgressBankByteCount,
     true,
     true,
     true,
     false,
     false,
     false,
     "80015700 copies the primary progress bank into the backup bank.",
     "Exact source/destination provenance only; this does not prove payload values inside 80092F10."},
    {ProgressBankCopyProvenanceKind::Restore80015744,
     kFn80015744,
     kBackupBank80079008,
     kProgressBank80092F10,
     kProgressBankByteCount,
     true,
     true,
     true,
     true,
     false,
     false,
     "80015744 restores the backup bank into the primary progress bank.",
     "Requires a known backup snapshot; this does not prove that the backup payload is current or runtime-authoritative."},
    {ProgressBankCopyProvenanceKind::LoadPayload800164B4,
     kFn800164B4,
     0,
     kProgressBank80092F10,
     kProgressBankByteCount,
     false,
     true,
     true,
     false,
     true,
     false,
     "800164B4 copies a caller-provided lower-card payload into the primary progress bank.",
     "The source pointer is dynamic and must be typed as lower-card payload before this path can authorize 80092F10 contents."},
};

constexpr Word800916F0S0ReplayBaselineSpec kWord800916F0S0ReplayBaseline = {
    kWord800916F0,
    2u,
    kWord800916F0S0ReplayFrameCount,
    0u,
    "Captured S0 all-directory recording negative baseline for word_800916F0 value changes.",
    "This baseline does not identify a writer/source for word_800916F0.",
};

constexpr Word800916F0S0ReplaySampleSpec kWord800916F0S0ReplaySamples[] = {
    {0u, 0x0000u, "recording_start"},
    {7000u, 0x0000u, "title_loop_window"},
    {7811u, 0x0000u, "first_input"},
    {8230u, 0x0000u, "main_directory_active"},
    {8291u, 0x0000u, "stage_select_cursor_init"},
    {8339u, 0x0000u, "stage_select_cursor8_move"},
    {8468u, 0x0000u, "stage_select_cancel_confirm"},
    {8839u, 0x0000u, "card_driver_route_9"},
    {9357u, 0x0000u, "card_driver_route_8"},
    {11318u, 0x0000u, "normal_easy_window"},
    {12683u, 0x0000u, "language_window"},
    {13408u, 0x0000u, "exit_highlight"},
    {14103u, 0x0000u, "recording_end"},
};

constexpr Word800916F0StaticAccessAuditSpec
    kWord800916F0StaticAccessAudit = {
        kWord800916F0,
        kWord800916F0StaticAccessReadCount,
        kWord800916F0StaticAccessWriteCount,
        kWord800916F0StaticImmediateStoreCount,
        kWord800916F0StaticPointerConstructedAccessCount,
        kWord800916F0StaticPointerConstructedWriteCount,
        true,
        true,
        true,
        "SCUS_941.83 plus S*/COMOD*.BIN direct paired-access scan for lui 0x8009 + offset 0x16F0 and pointer-constructed 0x800916F0 accesses.",
        "No paired direct store, aligned immediate store, or pointer-constructed store to word_800916F0 was found, but this does not close hidden writer families or runtime source authority; SS0-GAP-009 remains open.",
};

constexpr Word800916F0StaticAccessSpec kWord800916F0StaticAccesses[] = {
    {"SCUS_941.83",
     0x80026818u,
     0x17018u,
     "lh",
     false,
     "800267F8 stage-select force-enable/status consumer."},
    {"S1/COMOD1.BIN",
     0x801C84B4u,
     0x4C44u,
     "lh",
     false,
     "COMOD post-clear save/bootstrap gate consumer."},
    {"S2/COMOD2.BIN",
     0x801C77ACu,
     0x3F3Cu,
     "lh",
     false,
     "COMOD post-clear save/bootstrap gate consumer."},
    {"S3/COMOD3.BIN",
     0x801C7B48u,
     0x42D8u,
     "lh",
     false,
     "COMOD post-clear save/bootstrap gate consumer."},
    {"S5/COMOD5.BIN",
     0x801C8F50u,
     0x56E0u,
     "lh",
     false,
     "COMOD post-clear save/bootstrap gate consumer."},
    {"S6/COMOD6.BIN",
     0x801C730Cu,
     0x3A9Cu,
     "lh",
     false,
     "COMOD post-clear save/bootstrap gate consumer."},
    {"S7/COMOD7.BIN",
     0x801C7F08u,
     0x4698u,
     "lh",
     false,
     "COMOD post-clear save/bootstrap gate consumer."},
    {"S8/COMOD8.BIN",
     0x801C49F0u,
     0x1180u,
     "lh",
     false,
     "COMOD post-clear save/bootstrap gate consumer."},
};

constexpr ReplayMirrorProducerSpec kReplayMirrorProducers[] = {
    {ReplayMirrorProducerKind::Stage1EventTableBuild,
     kFn801C8660,
     kStage1ReplayTableSource801D2E2C,
     kReplayMirrorSrc8008EEF8,
     kReplayMaskMirror8008EEFC,
     kReplaySourceCarrier800901BC,
     0,
     kReplayMirrorEntryStride,
     kStage1ReplayTableEntryCount,
     kReplayMirrorMaxEntryCount,
     true,
     false,
     "Stage1 event-table build writes 53 authored tick/mask entries and 800901BC.",
     "Catalog fact only; it does not authorize the current runtime replay snapshot."},
    {ReplayMirrorProducerKind::RuntimeAcceptedAppend,
     kFn80014614,
     0,
     kReplayMirrorSrc8008EEF8,
     kReplayMaskMirror8008EEFC,
     kReplaySourceCarrier800901BC,
     kReplayRuntimeCursor800901C0,
     kReplayMirrorEntryStride,
     0,
     kReplayMirrorMaxEntryCount,
     false,
     false,
     "Main EXE accepted-input append writes one tick/mask entry and mirrors cursor to 800901BC.",
     "Catalog fact only; accepted input timing and appended values remain runtime producer authority."},
    {ReplayMirrorProducerKind::PayloadRestore8001681C,
     kFn8001681C,
     kReplaySourcePtr80092F48,
     kReplayMirrorSrc8008EEF8,
     kReplayMaskMirror8008EEFC,
     kReplaySourceCarrier800901BC,
     0,
     kReplayMirrorEntryStride,
     0,
     kReplayMirrorMaxEntryCount,
     false,
     false,
     "Main EXE replay restore copies 80092F48/80092F5C back to 800901BC and tick/mask mirror.",
     "Catalog fact only; payload/count/mirror must be known by the restore builder before current snapshot authority."},
};

constexpr ReplayMirrorResetSpec kReplayMirrorResets[] = {
    {ReplayMirrorResetKind::EventTableClear801C4AB8,
     kFn801C4AB8,
     kReplaySourceCarrier800901BC,
     0,
     false,
     false,
     false,
     false,
     "Overlay event-table clear writes 800901BC=0.",
     "Catalog fact only; this does not clear tick/mask mirror or prove 800901C0 state."},
    {ReplayMirrorResetKind::EventTableClear801C4988,
     kFn801C4988,
     kReplaySourceCarrier800901BC,
     0,
     false,
     false,
     false,
     false,
     "Overlay event-table clear writes 800901BC=0.",
     "Catalog fact only; this does not clear tick/mask mirror or prove 800901C0 state."},
    {ReplayMirrorResetKind::SetupReset80024E54,
     kFn80024E54,
     kReplaySourceCarrier800901BC,
     0,
     false,
     false,
     true,
     false,
     "Main EXE replay setup reset 80024E54(0) clears 800901BC and 800901C0.",
     "Catalog fact only; this preserves tick/mask mirror and requires the setup-reset builder for argument authority."},
};

constexpr ScoreAccumulatorWriterSpec kScoreAccumulatorWriters[] = {
    {ScoreAccumulatorWriterKind::Reset80014344,
     kFn80014344,
     0,
     kWord80091816,
     0,
     false,
     false,
     "Main EXE scorer reset clears word_80091816 together with sibling baseline words.",
     "Catalog fact only; reset does not prove any saved-score or gameplay accumulator value."},
    {ScoreAccumulatorWriterKind::GameplayCommit80014D58,
     kFn80014D58,
     0,
     kWord80091816,
     0,
     false,
     false,
     "Main EXE gameplay scorer commit updates and clamps word_80091816.",
     "Catalog fact only; runtime scorer inputs remain Stage1 scorer authority, not stage-progress payload authority."},
    {ScoreAccumulatorWriterKind::SavedScoreSync800169E0,
     kFn800169E0,
     kScoreBank80092F24,
     kWord80091816,
     kSavedScoreCtxOffset800169E0,
     true,
     false,
     "Main EXE saved-score sync reads dword_80092F24[slot], writes ctx+0x30, then mirrors low16 into word_80091816.",
     "Catalog fact only; use BuildSavedScoreSync800169E0Plan with known D0/E2/score source before treating the value as authorized."},
};

constexpr SavedScoreSyncSourceSpec kSavedScoreSyncSources[] = {
    {SavedScoreSyncSourceKind::ScoreBankSlot800169E0,
     kFn800169E0,
     kWord800916D0,
     2,
     kWord800916E2,
     kFn8001615C,
     kMapSaveStageTable80048DB8,
     kScoreBank80092F24,
     kScoreBankSlotCount,
     sizeof(uint32_t),
     kSavedScoreCtxOffset800169E0,
     sizeof(uint32_t),
     kWord80091816,
     sizeof(uint16_t),
     true,
     true,
     true,
     true,
     true,
     false,
     "800169E0 state==2 saved-score path maps word_800916E2 to a score slot, writes ctx+0x30, and mirrors low16 into word_80091816.",
     "Requires known D0/E2/score-bank input, including authoritative E2 scene word and known dword_80092F24[slot]; this is not runtime scorer or replay authority."},
};

constexpr SetupLatchSameValueCallsiteSpec kSetupLatchSameValueCallsites[] = {
    {SetupLatchSameValueCallsiteKind::StageRunner801C7A60,
     kFn801C7A60,
     kWord800916D0,
     kFn8001670C,
     kFn80016758,
     kFn800143F0,
     kFn800259C0,
     kWord8009182A,
     kWord8008ED34,
     kWord800916DA,
     true,
     true,
     false,
     false,
     "Overlay 801C7A60 passes the same narrow setup gate value to 800143F0 and 800259C0, with records-mode tail clearing both setters.",
     "Caller-side proof only; this does not prove global absence of other same-value writes or authorize unknown status/previous-grade inputs."},
};

constexpr Word800916F0ConsumerSpec kWord800916F0Consumers[] = {
    {Word800916F0ConsumerKind::StageSelectForceEnable800267F8,
     kFn800267F8,
     kWord800916F0,
     1,
     false,
     false,
     true,
     false,
     false,
     "800267F8 reads word_800916F0 before records/status handling; value 1 force-enables stage-select entries and keeps the branch as a hard gap.",
     "Consumer catalog only; current evidence does not identify a writer/source for word_800916F0."},
    {Word800916F0ConsumerKind::ComodPostClearSaveGateFamily,
     kFn801C81EC,
     kWord800916F0,
     1,
     false,
     false,
     false,
     true,
     false,
     "COMOD post-clear family reads word_800916F0 after 8001635C/optional 8001628C; value 1 skips 80015590(stage) and 80019148(&80092F10).",
     "Primary COMOD1 export address is recorded; sibling COMOD overlays share the consumer shape. This still proves no writer/source."},
};

bool Append(ProgressBankPlan& plan, const ProgressBankAction& action)
{
    if (plan.count >= sizeof(plan.actions) / sizeof(plan.actions[0])) {
        plan.truncated = true;
        return false;
    }
    plan.actions[plan.count++] = action;
    return true;
}

void AppendAction(ProgressBankPlan& plan,
                  ProgressBankActionKind kind,
                  uint32_t psxFunction,
                  uint32_t srcAddress = 0,
                  uint32_t dstAddress = 0,
                  uint32_t byteCount = 0,
                  int32_t stageOrSelector = 0,
                  int32_t slot = -1,
                  int32_t value = 0,
                  bool valueKnown = false,
                  bool conditional = false,
                  ProgressBankPayloadSourceKind payloadSource =
                      ProgressBankPayloadSourceKind::Unknown)
{
    ProgressBankAction action{};
    action.kind = kind;
    action.psxFunction = psxFunction;
    action.srcAddress = srcAddress;
    action.dstAddress = dstAddress;
    action.byteCount = byteCount;
    action.stageOrSelector = stageOrSelector;
    action.slot = slot;
    action.value = value;
    action.valueKnown = valueKnown;
    action.conditional = conditional;
    action.payloadSource = payloadSource;
    (void)Append(plan, action);
}

void AppendPlan(ProgressBankPlan& dst, const ProgressBankPlan& src)
{
    for (uint32_t i = 0; i < src.count; ++i) {
        (void)Append(dst, src.actions[i]);
    }
    dst.truncated = dst.truncated || src.truncated;
    dst.blockedByP0Gap = dst.blockedByP0Gap || src.blockedByP0Gap;
}

ProgressBankPlan MakePlan(const char* name)
{
    ProgressBankPlan plan{};
    plan.name = name;
    plan.runtimeCutoverAllowed = RuntimeCutoverAllowed();
    return plan;
}

const ProgressBankCopyProvenanceSpec& CopyProvenanceBackup80015700()
{
    return kProgressBankCopyProvenances[0];
}

const ProgressBankCopyProvenanceSpec& CopyProvenanceRestore80015744()
{
    return kProgressBankCopyProvenances[1];
}

const ProgressBankCopyProvenanceSpec& CopyProvenanceLoadPayload800164B4()
{
    return kProgressBankCopyProvenances[2];
}

uint32_t StatusAddressForSlot(int32_t slot)
{
    return kStatusBank80092F1D + static_cast<uint32_t>(slot);
}

const SavedScoreSyncSourceSpec& SavedScoreSync800169E0()
{
    return kSavedScoreSyncSources[0];
}

const SetupLatchSameValueCallsiteSpec& SetupLatchSameValue801C7A60()
{
    return kSetupLatchSameValueCallsites[0];
}

const Word800916F0ConsumerSpec& Word800916F0StageSelectConsumer800267F8()
{
    return kWord800916F0Consumers[0];
}

uint32_t ScoreAddressForSlot(int32_t slot)
{
    return kScoreBank80092F24 +
           static_cast<uint32_t>(slot) * sizeof(uint32_t);
}

uint32_t SavedScoreAddressForSlot(int32_t slot)
{
    const SavedScoreSyncSourceSpec& spec = SavedScoreSync800169E0();
    return spec.scoreBankAddress +
           static_cast<uint32_t>(slot) * spec.scoreSlotStride;
}

} // namespace

bool RuntimeCutoverAllowed()
{
    return false;
}

bool TryMapSaveStage8001615C(int32_t stage, int32_t* slot)
{
    if (stage < 0 ||
        static_cast<uint32_t>(stage) >=
            sizeof(kSaveStageMap80048DB8) / sizeof(kSaveStageMap80048DB8[0])) {
        return false;
    }
    if (slot != nullptr) {
        *slot = kSaveStageMap80048DB8[stage];
    }
    return true;
}

bool TryMapReplayScene800161A8(int32_t selector, int32_t* scene)
{
    if (selector < 0 ||
        static_cast<uint32_t>(selector) >=
            sizeof(kReplaySceneMap80048DD8) / sizeof(kReplaySceneMap80048DD8[0])) {
        return false;
    }
    if (scene != nullptr) {
        *scene = kReplaySceneMap80048DD8[selector];
    }
    return true;
}

uint32_t KnownProgressBankFieldCount()
{
    return sizeof(kFields) / sizeof(kFields[0]);
}

const ProgressBankFieldSpec& KnownProgressBankFieldAt(uint32_t index)
{
    if (index >= KnownProgressBankFieldCount()) {
        index = KnownProgressBankFieldCount() - 1u;
    }
    return kFields[index];
}

uint32_t KnownProgressBankCopyProvenanceCount()
{
    return sizeof(kProgressBankCopyProvenances) /
           sizeof(kProgressBankCopyProvenances[0]);
}

const ProgressBankCopyProvenanceSpec& KnownProgressBankCopyProvenanceAt(
    uint32_t index)
{
    if (index >= KnownProgressBankCopyProvenanceCount()) {
        index = KnownProgressBankCopyProvenanceCount() - 1u;
    }
    return kProgressBankCopyProvenances[index];
}

const Word800916F0S0ReplayBaselineSpec& KnownWord800916F0S0ReplayBaseline()
{
    return kWord800916F0S0ReplayBaseline;
}

uint32_t KnownWord800916F0S0ReplaySampleCount()
{
    return sizeof(kWord800916F0S0ReplaySamples) /
           sizeof(kWord800916F0S0ReplaySamples[0]);
}

const Word800916F0S0ReplaySampleSpec& KnownWord800916F0S0ReplaySampleAt(
    uint32_t index)
{
    if (index >= KnownWord800916F0S0ReplaySampleCount()) {
        index = KnownWord800916F0S0ReplaySampleCount() - 1u;
    }
    return kWord800916F0S0ReplaySamples[index];
}

const Word800916F0StaticAccessAuditSpec&
KnownWord800916F0StaticAccessAudit()
{
    return kWord800916F0StaticAccessAudit;
}

uint32_t KnownWord800916F0StaticAccessCount()
{
    return sizeof(kWord800916F0StaticAccesses) /
           sizeof(kWord800916F0StaticAccesses[0]);
}

const Word800916F0StaticAccessSpec& KnownWord800916F0StaticAccessAt(
    uint32_t index)
{
    if (index >= KnownWord800916F0StaticAccessCount()) {
        index = KnownWord800916F0StaticAccessCount() - 1u;
    }
    return kWord800916F0StaticAccesses[index];
}

uint32_t KnownReplayMirrorProducerCount()
{
    return sizeof(kReplayMirrorProducers) / sizeof(kReplayMirrorProducers[0]);
}

const ReplayMirrorProducerSpec& KnownReplayMirrorProducerAt(uint32_t index)
{
    if (index >= KnownReplayMirrorProducerCount()) {
        index = KnownReplayMirrorProducerCount() - 1u;
    }
    return kReplayMirrorProducers[index];
}

uint32_t KnownReplayMirrorResetCount()
{
    return sizeof(kReplayMirrorResets) / sizeof(kReplayMirrorResets[0]);
}

const ReplayMirrorResetSpec& KnownReplayMirrorResetAt(uint32_t index)
{
    if (index >= KnownReplayMirrorResetCount()) {
        index = KnownReplayMirrorResetCount() - 1u;
    }
    return kReplayMirrorResets[index];
}

uint32_t KnownScoreAccumulatorWriterCount()
{
    return sizeof(kScoreAccumulatorWriters) /
           sizeof(kScoreAccumulatorWriters[0]);
}

const ScoreAccumulatorWriterSpec& KnownScoreAccumulatorWriterAt(
    uint32_t index)
{
    if (index >= KnownScoreAccumulatorWriterCount()) {
        index = KnownScoreAccumulatorWriterCount() - 1u;
    }
    return kScoreAccumulatorWriters[index];
}

uint32_t KnownSavedScoreSyncSourceCount()
{
    return sizeof(kSavedScoreSyncSources) / sizeof(kSavedScoreSyncSources[0]);
}

const SavedScoreSyncSourceSpec& KnownSavedScoreSyncSourceAt(uint32_t index)
{
    if (index >= KnownSavedScoreSyncSourceCount()) {
        index = KnownSavedScoreSyncSourceCount() - 1u;
    }
    return kSavedScoreSyncSources[index];
}

uint32_t KnownSetupLatchSameValueCallsiteCount()
{
    return sizeof(kSetupLatchSameValueCallsites) /
           sizeof(kSetupLatchSameValueCallsites[0]);
}

const SetupLatchSameValueCallsiteSpec& KnownSetupLatchSameValueCallsiteAt(
    uint32_t index)
{
    if (index >= KnownSetupLatchSameValueCallsiteCount()) {
        index = KnownSetupLatchSameValueCallsiteCount() - 1u;
    }
    return kSetupLatchSameValueCallsites[index];
}

uint32_t KnownWord800916F0ConsumerCount()
{
    return sizeof(kWord800916F0Consumers) / sizeof(kWord800916F0Consumers[0]);
}

const Word800916F0ConsumerSpec& KnownWord800916F0ConsumerAt(uint32_t index)
{
    if (index >= KnownWord800916F0ConsumerCount()) {
        index = KnownWord800916F0ConsumerCount() - 1u;
    }
    return kWord800916F0Consumers[index];
}

bool IsKnownReplayMirrorProducerFunction(uint32_t psxFunction)
{
    for (uint32_t i = 0; i < KnownReplayMirrorProducerCount(); ++i) {
        if (kReplayMirrorProducers[i].psxFunction == psxFunction) {
            return true;
        }
    }
    return false;
}

ProgressBankPlan BuildClearAndSeed80015CC4Plan()
{
    ProgressBankPlan plan = MakePlan("ClearAndSeed80015CC4");
    plan.blockedByP0Gap = true;

    AppendAction(plan,
                 ProgressBankActionKind::ClearBytes80015CC4,
                 kFn80015CC4,
                 0,
                 kProgressBank80092F10,
                 kProgressBankByteCount);
    AppendPlan(plan,
               BuildUpdate8001635CPlan(1,
                                        1,
                                        1,
                                        0,
                                        true,
                                        false,
                                        false,
                                        0));
    return plan;
}

ProgressBankPlan BuildBackup80015700Plan(uint32_t srcAddress)
{
    const ProgressBankCopyProvenanceSpec& spec =
        CopyProvenanceBackup80015700();
    ProgressBankPlan plan = MakePlan("Backup80015700");
    plan.blockedByP0Gap = srcAddress != spec.srcAddress;

    AppendAction(plan,
                 ProgressBankActionKind::CopyBytes80015700,
                 spec.psxFunction,
                 srcAddress,
                 spec.dstAddress,
                 spec.byteCount);
    AppendAction(plan,
                 ProgressBankActionKind::GateBackupSource80015700,
                 spec.psxFunction,
                 srcAddress,
                 spec.dstAddress,
                 spec.byteCount,
                 0,
                 -1,
                 srcAddress == spec.srcAddress ? 1 : 0,
                 srcAddress == spec.srcAddress);
    if (srcAddress != spec.srcAddress) {
        AppendAction(plan,
                     ProgressBankActionKind::Gap,
                     spec.psxFunction,
                     srcAddress,
                     spec.dstAddress,
                     spec.byteCount);
    }
    return plan;
}

ProgressBankPlan BuildRestore80015744Plan(bool backupKnown,
                                           uint32_t dstAddress)
{
    const ProgressBankCopyProvenanceSpec& spec =
        CopyProvenanceRestore80015744();
    ProgressBankPlan plan = MakePlan("Restore80015744");
    plan.blockedByP0Gap = !backupKnown || dstAddress != spec.dstAddress;

    AppendAction(plan,
                 ProgressBankActionKind::CopyBytes80015744,
                 spec.psxFunction,
                 spec.srcAddress,
                 dstAddress,
                 spec.byteCount);
    AppendAction(plan,
                 ProgressBankActionKind::GateRestoreTarget80015744,
                 spec.psxFunction,
                 spec.srcAddress,
                 dstAddress,
                 spec.byteCount,
                 0,
                 -1,
                 backupKnown && dstAddress == spec.dstAddress ? 1 : 0,
                 backupKnown);
    if (!backupKnown || dstAddress != spec.dstAddress) {
        AppendAction(plan,
                     ProgressBankActionKind::Gap,
                     spec.psxFunction,
                     spec.srcAddress,
                     dstAddress,
                     spec.byteCount);
    }
    return plan;
}

ProgressBankPlan BuildLoadPayload800164B4Plan(
    bool sourceKnown,
    uint32_t sourceAddress,
    ProgressBankPayloadSourceKind payloadSource,
    bool lowerCardPayloadKnown)
{
    const ProgressBankCopyProvenanceSpec& spec =
        CopyProvenanceLoadPayload800164B4();
    const bool sourceIsRuntimeLowerCardProducer =
        payloadSource == ProgressBankPayloadSourceKind::RuntimeLowerCardProducer;
    const bool sourceAuthorized =
        sourceKnown && (!spec.lowerCardPayloadRequired ||
                        (lowerCardPayloadKnown &&
                         sourceIsRuntimeLowerCardProducer));
    ProgressBankPlan plan = MakePlan("LoadPayload800164B4");
    plan.blockedByP0Gap = !sourceAuthorized;

    AppendAction(plan,
                 ProgressBankActionKind::CopyBytes800164B4,
                 spec.psxFunction,
                 sourceAddress,
                 spec.dstAddress,
                 spec.byteCount);
    AppendAction(plan,
                 ProgressBankActionKind::GateLoadPayloadSource800164B4,
                 spec.psxFunction,
                 sourceAddress,
                 spec.dstAddress,
                 spec.byteCount,
                 0,
                 -1,
                 sourceAuthorized ? 1 : 0,
                 sourceAuthorized,
                 true,
                 payloadSource);
    if (!sourceAuthorized) {
        AppendAction(plan,
                     ProgressBankActionKind::Gap,
                     spec.psxFunction,
                     sourceAddress,
                     spec.dstAddress,
                     spec.byteCount);
    }
    return plan;
}

ProgressBankPlan BuildAllClear800161F4Plan(bool statusBankKnown)
{
    ProgressBankPlan plan = MakePlan("AllClear800161F4");
    plan.blockedByP0Gap = !statusBankKnown;

    AppendAction(plan,
                 ProgressBankActionKind::QueryAllClear800161F4,
                 kFn800161F4,
                 kStatusBank80092F1D,
                 0,
                 kStatusBankAllClearByteCount);
    if (!statusBankKnown) {
        AppendAction(plan,
                     ProgressBankActionKind::Gap,
                     kFn800161F4,
                     kStatusBank80092F1D,
                     0,
                     kStatusBankAllClearByteCount);
    }
    return plan;
}

ProgressBankPlan BuildQueryStatus800166ACPlan(int32_t stage,
                                               bool statusBankKnown)
{
    ProgressBankPlan plan = MakePlan("QueryStatus800166AC");

    int32_t slot = -1;
    const bool mapped = TryMapSaveStage8001615C(stage, &slot);
    plan.blockedByP0Gap = !mapped || !statusBankKnown;

    AppendAction(plan,
                 ProgressBankActionKind::MapStage8001615C,
                 kFn8001615C,
                 kMapSaveStageTable80048DB8,
                 0,
                 0,
                 stage,
                 slot,
                 slot,
                 mapped);
    if (mapped) {
        AppendAction(plan,
                     ProgressBankActionKind::QueryStatus800166AC,
                     kFn800166AC,
                     StatusAddressForSlot(slot),
                     0,
                     1,
                     stage,
                     slot);
    }
    if (!mapped || !statusBankKnown) {
        AppendAction(plan,
                     ProgressBankActionKind::Gap,
                     kFn800166AC,
                     mapped ? StatusAddressForSlot(slot) : 0,
                     0,
                     1,
                     stage,
                     slot);
    }
    return plan;
}

ProgressBankPlan BuildStageStatusGate8001670CPlan(int32_t stage,
                                                  bool statusKnown,
                                                  uint8_t status)
{
    ProgressBankPlan plan = MakePlan("StageStatusGate8001670C");

    int32_t slot = -1;
    const bool mapped = TryMapSaveStage8001615C(stage, &slot);
    const int32_t result = status >= 2u ? 1 : 0;
    plan.blockedByP0Gap = !mapped || !statusKnown;

    AppendAction(plan,
                 ProgressBankActionKind::MapStage8001615C,
                 kFn8001615C,
                 kMapSaveStageTable80048DB8,
                 0,
                 0,
                 stage,
                 slot,
                 slot,
                 mapped);
    if (mapped) {
        AppendAction(plan,
                     ProgressBankActionKind::GateStageStatusSource8001670C,
                     kFn8001670C,
                     StatusAddressForSlot(slot),
                     0,
                     1,
                     stage,
                     slot,
                     status,
                     statusKnown,
                     true);
        AppendAction(plan,
                     ProgressBankActionKind::ReadStageStatusGate8001670C,
                     kFn8001670C,
                     StatusAddressForSlot(slot),
                     0,
                     1,
                     stage,
                     slot,
                     result,
                     statusKnown,
                     true);
    }
    if (plan.blockedByP0Gap) {
        AppendAction(plan,
                     ProgressBankActionKind::Gap,
                     kFn8001670C,
                     mapped ? StatusAddressForSlot(slot) : 0,
                     0,
                     1,
                     stage,
                     slot);
    }
    return plan;
}

ProgressBankPlan BuildUnlock8001628CPlan(int32_t stage,
                                          bool currentStatusKnown,
                                          int32_t currentStatus)
{
    ProgressBankPlan plan = MakePlan("Unlock8001628C");

    int32_t slot = -1;
    const bool mapped = TryMapSaveStage8001615C(stage, &slot);
    plan.blockedByP0Gap = !mapped || !currentStatusKnown;

    AppendAction(plan,
                 ProgressBankActionKind::MapStage8001615C,
                 kFn8001615C,
                 kMapSaveStageTable80048DB8,
                 0,
                 0,
                 stage,
                 slot,
                 slot,
                 mapped);
    if (mapped) {
        AppendAction(plan,
                     ProgressBankActionKind::ConditionalUnlock8001628C,
                     kFn8001628C,
                     StatusAddressForSlot(slot),
                     StatusAddressForSlot(slot),
                     1,
                     stage,
                     slot,
                     1,
                     currentStatusKnown && currentStatus == 0,
                     true);
    }
    if (!mapped || !currentStatusKnown) {
        AppendAction(plan,
                     ProgressBankActionKind::Gap,
                     kFn8001628C,
                     mapped ? StatusAddressForSlot(slot) : 0,
                     0,
                     1,
                     stage,
                     slot);
    }
    return plan;
}

ProgressBankPlan BuildUpdate8001635CPlan(int32_t stage,
                                          int32_t targetStatus,
                                          int32_t previousGrade,
                                          int32_t score,
                                          bool statusBankKnown,
                                          bool replayMirrorSourceKnown,
                                          bool carrierSourceKnown,
                                          uint32_t carrierSource)
{
    ProgressBankPlan plan = MakePlan("Update8001635C");

    int32_t slot = -1;
    const bool mapped = TryMapSaveStage8001615C(stage, &slot);
    plan.blockedByP0Gap =
        !mapped || !statusBankKnown || !replayMirrorSourceKnown ||
        !carrierSourceKnown;

    AppendAction(plan,
                 ProgressBankActionKind::MapStage8001615C,
                 kFn8001615C,
                 kMapSaveStageTable80048DB8,
                 0,
                 0,
                 stage,
                 slot,
                 slot,
                 mapped);
    AppendAction(plan,
                 ProgressBankActionKind::WritePrevGrade8001635C,
                 kFn8001635C,
                 0,
                 kPrevGrade80092F40,
                 4,
                 stage,
                 slot,
                 previousGrade != 0 ? previousGrade : 1,
                 true);
    if (mapped) {
        AppendAction(plan,
                     ProgressBankActionKind::MaxPromoteStatus8001635C,
                     kFn8001635C,
                     StatusAddressForSlot(slot),
                     StatusAddressForSlot(slot),
                     1,
                     stage,
                     slot,
                     targetStatus,
                     true,
                     true);
        AppendAction(plan,
                     ProgressBankActionKind::WriteScore8001635C,
                     kFn8001635C,
                     0,
                     ScoreAddressForSlot(slot),
                     4,
                     stage,
                     slot,
                     score,
                     true);
        AppendAction(plan,
                     ProgressBankActionKind::WriteLastSlot8001635C,
                     kFn8001635C,
                     0,
                     kLastSavedSlot80092F3C,
                     4,
                     stage,
                     slot,
                     slot,
                     true);
    }
    AppendAction(plan,
                 ProgressBankActionKind::WriteReplaySource8001635C,
                 kFn8001635C,
                 carrierSourceKnown ? carrierSource : 0,
                 kReplaySourcePtr80092F48,
                 4,
                 stage,
                 slot,
                 static_cast<int32_t>(carrierSource),
                 carrierSourceKnown);
    AppendAction(plan,
                 ProgressBankActionKind::GateReplaySourceCarrier8001635C,
                 kFn8001635C,
                 kReplaySourceCarrier800901BC,
                 kReplaySourcePtr80092F48,
                 4,
                 stage,
                 slot,
                 static_cast<int32_t>(carrierSource),
                 carrierSourceKnown);
    AppendAction(plan,
                 ProgressBankActionKind::CopyReplayMirror8001635C,
                 kFn8001635C,
                 kReplayMirrorSrc8008EEF8,
                 kReplayMirrorDst80092F5C,
                 kReplayMirrorByteCount,
                 stage,
                 slot,
                 0,
                 replayMirrorSourceKnown);
    AppendAction(plan,
                 ProgressBankActionKind::GateReplayMirrorSource8001635C,
                 kFn8001635C,
                 kReplayMirrorSrc8008EEF8,
                 kReplayMirrorDst80092F5C,
                 kReplayMirrorByteCount,
                 stage,
                 slot,
                 0,
                 replayMirrorSourceKnown);
    AppendAction(plan,
                 ProgressBankActionKind::QueryAllClear800161F4,
                 kFn800161F4,
                 kStatusBank80092F1D,
                 0,
                 kStatusBankAllClearByteCount,
                 stage,
                 slot,
                 0,
                 statusBankKnown);
    AppendAction(plan,
                 ProgressBankActionKind::WriteAllClear8001635C,
                 kFn8001635C,
                 0,
                 kAllClearFlag80092F44,
                 4,
                 stage,
                 slot,
                 0,
                 statusBankKnown);
    if (plan.blockedByP0Gap) {
        AppendAction(plan,
                     ProgressBankActionKind::Gap,
                     kFn8001635C,
                     mapped ? StatusAddressForSlot(slot) : 0,
                     kProgressBank80092F10,
                     kProgressBankByteCount,
                     stage,
                     slot);
    }
    return plan;
}

ProgressBankPlan BuildStatusWrite800167A8Plan(int32_t stage, int32_t mode)
{
    ProgressBankPlan plan = MakePlan("StatusWrite800167A8");

    int32_t slot = -1;
    const bool mapped = TryMapSaveStage8001615C(stage, &slot);
    plan.blockedByP0Gap = !mapped;

    AppendAction(plan,
                 ProgressBankActionKind::MapStage8001615C,
                 kFn8001615C,
                 kMapSaveStageTable80048DB8,
                 0,
                 0,
                 stage,
                 slot,
                 slot,
                 mapped);
    if (mapped) {
        AppendAction(plan,
                     ProgressBankActionKind::WriteStatus800167A8,
                     kFn800167A8,
                     0,
                     StatusAddressForSlot(slot),
                     1,
                     stage,
                     slot,
                     mode == 1 ? 2 : 1,
                     true);
    } else {
        AppendAction(plan,
                     ProgressBankActionKind::Gap,
                     kFn800167A8,
                     0,
                     0,
                     1,
                     stage,
                     slot);
    }
    return plan;
}

ProgressBankPlan BuildPreviousGradeGate80016758Plan(bool prevGradeKnown,
                                                    uint32_t prevGrade)
{
    ProgressBankPlan plan = MakePlan("PreviousGradeGate80016758");
    const int32_t result = prevGrade >= 2u ? 1 : 0;
    plan.blockedByP0Gap = !prevGradeKnown;

    AppendAction(plan,
                 ProgressBankActionKind::GatePreviousGradeSource80016758,
                 kFn80016758,
                 kPrevGrade80092F40,
                 0,
                 4,
                 0,
                 -1,
                 static_cast<int32_t>(prevGrade),
                 prevGradeKnown,
                 true);
    AppendAction(plan,
                 ProgressBankActionKind::ReadPreviousGradeGate80016758,
                 kFn80016758,
                 kPrevGrade80092F40,
                 0,
                 4,
                 0,
                 -1,
                 result,
                 prevGradeKnown,
                 true);
    if (plan.blockedByP0Gap) {
        AppendAction(plan,
                     ProgressBankActionKind::Gap,
                     kFn80016758,
                     kPrevGrade80092F40,
                     0,
                     4);
    }
    return plan;
}

ProgressBankPlan BuildSetupGateLatchPair801C7A60Plan(
    bool gateValueKnown,
    int32_t gateValue,
    bool recordsModeKnown,
    int32_t word800916DA)
{
    ProgressBankPlan plan = MakePlan("SetupGateLatchPair801C7A60");
    const SetupLatchSameValueCallsiteSpec& spec =
        SetupLatchSameValue801C7A60();
    const bool gateValueNarrow = gateValue == 0 || gateValue == 1;
    const bool writeValueKnown = gateValueKnown && gateValueNarrow;
    const int32_t writeValue = gateValueNarrow ? gateValue : 0;
    const bool recordsModeTail = recordsModeKnown && word800916DA == 1;
    plan.blockedByP0Gap = !writeValueKnown || !recordsModeKnown;

    AppendAction(plan,
                 ProgressBankActionKind::GateSetupLatchSource801C7A60,
                 spec.overlayFunction,
                 0,
                 0,
                 0,
                 0,
                 -1,
                 writeValue,
                 writeValueKnown,
                 true);
    AppendAction(plan,
                 ProgressBankActionKind::WriteWord8009182A800143F0,
                 spec.firstSetterFunction,
                 0,
                 spec.firstDstAddress,
                 2,
                 0,
                 -1,
                 writeValue,
                 writeValueKnown);
    AppendAction(plan,
                 ProgressBankActionKind::WriteWord8008ED34800259C0,
                 spec.secondSetterFunction,
                 0,
                 spec.secondDstAddress,
                 2,
                 0,
                 -1,
                 writeValue,
                 writeValueKnown);
    AppendAction(plan,
                 ProgressBankActionKind::GateSetupTailRecordsMode801C7A60,
                 spec.overlayFunction,
                 spec.recordsModeWordAddress,
                 0,
                 2,
                 0,
                 -1,
                 word800916DA,
                 recordsModeKnown,
                 true);
    if (recordsModeTail) {
        AppendAction(plan,
                     ProgressBankActionKind::ClearWord8009182A800143F0,
                     spec.firstSetterFunction,
                     0,
                     spec.firstDstAddress,
                     2,
                     0,
                     -1,
                     0,
                     true,
                     true);
        AppendAction(plan,
                     ProgressBankActionKind::ClearWord8008ED34800259C0,
                     spec.secondSetterFunction,
                     0,
                     spec.secondDstAddress,
                     2,
                     0,
                     -1,
                     0,
                     true,
                     true);
    }
    if (plan.blockedByP0Gap) {
        AppendAction(plan,
                     ProgressBankActionKind::Gap,
                     spec.overlayFunction,
                     recordsModeKnown ? 0 : spec.recordsModeWordAddress,
                     0,
                     2);
    }
    return plan;
}

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
    uint32_t restorePublishedCount)
{
    ProgressBankPlan plan = MakePlan("SetupGateSource801C7A60");
    plan.blockedByP0Gap = !transitionStateKnown;

    AppendAction(plan,
                 ProgressBankActionKind::GateSetupBranchWord800916D0801C7A60,
                 kFn801C7A60,
                 kWord800916D0,
                 0,
                 2,
                 word800916D0,
                 -1,
                 word800916D0,
                 transitionStateKnown,
                 true);
    if (!transitionStateKnown) {
        AppendAction(plan,
                     ProgressBankActionKind::Gap,
                     kFn801C7A60,
                     kWord800916D0,
                     0,
                     2,
                     word800916D0);
        return plan;
    }

    if (word800916D0 == 1) {
        AppendPlan(plan, BuildReplaySetupReset80024E54Plan(true, 0));
        AppendPlan(plan, BuildSetupGateLatchPair801C7A60Plan(true, 0, true, 0));
        return plan;
    }

    if (word800916D0 == 2) {
        const ProgressBankPlan restorePlan =
            BuildReplaySetupRestore80024E54_8001681CPlan(
                true,
                2,
                restorePayloadKnown,
                restorePublishedCountKnown,
                restoreMirrorPayloadKnown,
                restorePublishedCount);
        AppendPlan(plan, restorePlan);

        const ProgressBankPlan previousGradePlan =
            BuildPreviousGradeGate80016758Plan(prevGradeKnown, prevGrade);
        AppendPlan(plan, previousGradePlan);

        const int32_t gateValue = prevGrade >= 2u ? 1 : 0;
        AppendPlan(plan,
                   BuildSetupGateLatchPair801C7A60Plan(prevGradeKnown,
                                                       gateValue,
                                                       true,
                                                       0));
        return plan;
    }

    AppendPlan(plan, BuildReplaySetupReset80024E54Plan(true, 0));
    const ProgressBankPlan statusPlan =
        BuildStageStatusGate8001670CPlan(stage, statusKnown, status);
    AppendPlan(plan, statusPlan);

    const int32_t gateValue = status >= 2u ? 1 : 0;
    AppendPlan(plan,
               BuildSetupGateLatchPair801C7A60Plan(statusKnown,
                                                   gateValue,
                                                   recordsModeKnown,
                                                   word800916DA));
    return plan;
}

ProgressBankPlan BuildCurrentSceneWrite80015D18Plan(int32_t sceneIndex,
                                                    bool sceneIndexKnown)
{
    ProgressBankPlan plan = MakePlan("CurrentSceneWrite80015D18");
    plan.blockedByP0Gap = !sceneIndexKnown;

    AppendAction(plan,
                 ProgressBankActionKind::GateWord800916E2Source80015D18,
                 kFn80015D18,
                 0,
                 kWord800916E2,
                 2,
                 sceneIndex,
                 sceneIndex,
                 sceneIndex,
                 sceneIndexKnown);
    if (sceneIndexKnown) {
        AppendAction(plan,
                     ProgressBankActionKind::WriteWord800916E2,
                     kFn80015D18,
                     0,
                     kWord800916E2,
                     2,
                     sceneIndex,
                     sceneIndex,
                     sceneIndex,
                     true);
    } else {
        AppendAction(plan,
                     ProgressBankActionKind::Gap,
                     kFn80015D18,
                     0,
                     kWord800916E2,
                     2,
                     sceneIndex,
                     sceneIndex);
    }
    return plan;
}

ProgressBankPlan BuildReplaySnapshotAuthority8001635CPlan(
    bool tickMaskMirrorKnown,
    bool fullBackingKnown,
    bool publishedCountKnown,
    bool cursorKnown,
    uint32_t publishedCount,
    uint32_t cursor,
    uint32_t producerFunction,
    bool cursorRequired)
{
    ProgressBankPlan plan = MakePlan("ReplaySnapshotAuthority8001635C");
    const bool producerKnown =
        IsKnownReplayMirrorProducerFunction(producerFunction);
    plan.blockedByP0Gap =
        !producerKnown || !tickMaskMirrorKnown || !fullBackingKnown ||
        !publishedCountKnown ||
        (cursorRequired && !cursorKnown);

    AppendAction(plan,
                 ProgressBankActionKind::GateReplayProducerFamily8001635C,
                 kFn8001635C,
                 producerFunction,
                 kFn8001635C,
                 0,
                 0,
                 -1,
                 static_cast<int32_t>(producerFunction),
                 producerKnown,
                 true);
    AppendAction(plan,
                 ProgressBankActionKind::GateReplaySnapshotMirrorSource8001635C,
                 kFn8001635C,
                 kReplayMirrorSrc8008EEF8,
                 kReplayMirrorDst80092F5C,
                 kReplayMirrorByteCount,
                 0,
                 -1,
                 0,
                 tickMaskMirrorKnown && fullBackingKnown,
                 true);
    AppendAction(plan,
                 ProgressBankActionKind::GateReplayPublishedCount8001635C,
                 kFn8001635C,
                 kReplaySourceCarrier800901BC,
                 kReplaySourcePtr80092F48,
                 4,
                 0,
                 -1,
                 static_cast<int32_t>(publishedCount),
                 publishedCountKnown,
                 true);
    AppendAction(plan,
                 ProgressBankActionKind::GateReplayCursorSource8001635C,
                 kFn8001635C,
                 kReplayRuntimeCursor800901C0,
                 0,
                 4,
                 0,
                 -1,
                 static_cast<int32_t>(cursor),
                 cursorKnown || !cursorRequired,
                 cursorRequired);
    if (plan.blockedByP0Gap) {
        AppendAction(plan,
                     ProgressBankActionKind::Gap,
                     kFn8001635C,
                     kReplayMirrorSrc8008EEF8,
                     kReplayMirrorDst80092F5C,
                     kReplayMirrorByteCount);
    }
    return plan;
}

ProgressBankPlan BuildReplayRestore8001681CPlan(bool payloadKnown,
                                                bool publishedCountKnown,
                                                bool mirrorPayloadKnown,
                                                uint32_t publishedCount)
{
    ProgressBankPlan plan = MakePlan("ReplayRestore8001681C");
    const bool publishedCountInRange =
        publishedCountKnown && publishedCount <= kReplayMirrorMaxEntryCount;
    const uint32_t restoredMirrorByteCount =
        publishedCountInRange ? publishedCount * kReplayMirrorEntryStride : 0u;
    const bool mirrorPayloadRequired =
        publishedCountInRange && publishedCount != 0u;
    const bool mirrorPayloadSufficient =
        !mirrorPayloadRequired || mirrorPayloadKnown;
    plan.blockedByP0Gap =
        !payloadKnown || !publishedCountInRange || !mirrorPayloadSufficient;

    AppendAction(plan,
                 ProgressBankActionKind::GateReplayRestorePayload8001681C,
                 kFn8001681C,
                 kReplaySourcePtr80092F48,
                 kReplaySourceCarrier800901BC,
                 4,
                 0,
                 -1,
                 static_cast<int32_t>(publishedCount),
                 payloadKnown && publishedCountInRange,
                 true);
    AppendAction(plan,
                 ProgressBankActionKind::WriteReplayPublishedCount8001681C,
                 kFn8001681C,
                 kReplaySourcePtr80092F48,
                 kReplaySourceCarrier800901BC,
                 4,
                 0,
                 -1,
                 static_cast<int32_t>(publishedCount),
                 payloadKnown && publishedCountInRange);
    AppendAction(plan,
                 ProgressBankActionKind::CopyReplayMirrorRestore8001681C,
                 kFn8001681C,
                 kReplayMirrorDst80092F5C,
                 kReplayMirrorSrc8008EEF8,
                 restoredMirrorByteCount,
                 0,
                 -1,
                 0,
                 payloadKnown && publishedCountInRange &&
                     mirrorPayloadSufficient,
                 true);
    if (plan.blockedByP0Gap) {
        AppendAction(plan,
                     ProgressBankActionKind::Gap,
                     kFn8001681C,
                     kReplaySourcePtr80092F48,
                     kReplaySourceCarrier800901BC,
                     4);
    }
    return plan;
}

ProgressBankPlan BuildReplaySetupRestore80024E54_8001681CPlan(
    bool transitionStateKnown,
    uint32_t transitionState,
    bool payloadKnown,
    bool publishedCountKnown,
    bool mirrorPayloadKnown,
    uint32_t publishedCount)
{
    ProgressBankPlan plan = MakePlan("ReplaySetupRestore80024E54_8001681C");
    const bool restorePath = transitionStateKnown && transitionState == 2u;
    plan.blockedByP0Gap = !transitionStateKnown;

    AppendAction(
        plan,
        ProgressBankActionKind::GateReplaySetupTransitionState80024E54,
        kFn80024E54,
        0,
        kFn8001681C,
        4,
        static_cast<int32_t>(transitionState),
        -1,
        static_cast<int32_t>(transitionState),
        transitionStateKnown,
        true);

    AppendPlan(plan, BuildReplaySetupReset80024E54Plan(true, 0));
    if (restorePath) {
        const ProgressBankPlan restorePlan =
            BuildReplayRestore8001681CPlan(payloadKnown,
                                           publishedCountKnown,
                                           mirrorPayloadKnown,
                                           publishedCount);
        AppendPlan(plan, restorePlan);
        AppendAction(plan,
                     ProgressBankActionKind::PreserveReplayCursorAfterRestore8001681C,
                     kFn8001681C,
                     kReplayRuntimeCursor800901C0,
                     kReplayRuntimeCursor800901C0,
                     4,
                     static_cast<int32_t>(transitionState),
                     -1,
                     0,
                     true,
                     true);
        plan.blockedByP0Gap = plan.blockedByP0Gap || restorePlan.blockedByP0Gap;
    }
    if (!transitionStateKnown) {
        AppendAction(plan,
                     ProgressBankActionKind::Gap,
                     kFn80024E54,
                     0,
                     kFn8001681C,
                     4,
                     static_cast<int32_t>(transitionState));
    }
    return plan;
}

ProgressBankPlan BuildReplayAcceptedAppend80014614Plan(bool appendAccepted,
                                                       bool cursorKnown,
                                                       uint32_t cursor,
                                                       bool tickKnown,
                                                       uint32_t tick,
                                                       bool maskKnown,
                                                       uint32_t mask)
{
    ProgressBankPlan plan = MakePlan("ReplayAcceptedAppend80014614");
    const bool cursorInBounds =
        cursorKnown && cursor < kReplayMirrorMaxEntryCount;
    const bool valuesKnown = tickKnown && maskKnown;
    const bool canAppend = appendAccepted && cursorInBounds && valuesKnown;
    const uint32_t nextCursor = cursor + 1u;
    plan.blockedByP0Gap = !canAppend;

    AppendAction(plan,
                 ProgressBankActionKind::GateReplayAppendAccepted80014614,
                 kFn80014614,
                 0,
                 0,
                 0,
                 appendAccepted ? 1 : 0,
                 -1,
                 appendAccepted ? 1 : 0,
                 appendAccepted,
                 true);
    AppendAction(plan,
                 ProgressBankActionKind::GateReplayAppendBounds80014614,
                 kFn80014614,
                 kReplayRuntimeCursor800901C0,
                 kReplayMirrorSrc8008EEF8,
                 kReplayMirrorEntryStride,
                 static_cast<int32_t>(cursor),
                 static_cast<int32_t>(cursor),
                 static_cast<int32_t>(cursor),
                 cursorInBounds,
                 true);
    AppendAction(plan,
                 ProgressBankActionKind::WriteReplayAppendMask80014614,
                 kFn80014614,
                 0,
                 kReplayMaskMirror8008EEFC,
                 4,
                 static_cast<int32_t>(cursor),
                 static_cast<int32_t>(cursor),
                 static_cast<int32_t>(mask),
                 canAppend);
    AppendAction(plan,
                 ProgressBankActionKind::WriteReplayAppendTick80014614,
                 kFn80014614,
                 0,
                 kReplayMirrorSrc8008EEF8,
                 4,
                 static_cast<int32_t>(cursor),
                 static_cast<int32_t>(cursor),
                 static_cast<int32_t>(tick),
                 canAppend);
    AppendAction(plan,
                 ProgressBankActionKind::WriteReplayCursorIncrement80014614,
                 kFn80014614,
                 kReplayRuntimeCursor800901C0,
                 kReplayRuntimeCursor800901C0,
                 4,
                 static_cast<int32_t>(cursor),
                 static_cast<int32_t>(cursor),
                 static_cast<int32_t>(nextCursor),
                 canAppend);
    AppendAction(
        plan,
        ProgressBankActionKind::WriteReplayPublishedCountFromCursor80014614,
        kFn80014614,
        kReplayRuntimeCursor800901C0,
        kReplaySourceCarrier800901BC,
        4,
        static_cast<int32_t>(cursor),
        static_cast<int32_t>(cursor),
        static_cast<int32_t>(nextCursor),
        canAppend);
    if (plan.blockedByP0Gap) {
        AppendAction(plan,
                     ProgressBankActionKind::Gap,
                     kFn80014614,
                     kReplayRuntimeCursor800901C0,
                     kReplaySourceCarrier800901BC,
                     4,
                     static_cast<int32_t>(cursor),
                     static_cast<int32_t>(cursor));
    }
    return plan;
}

ProgressBankPlan BuildReplayScene0ZeroEventTableSeed801C4FC8Plan(
    bool countKnown,
    uint32_t count)
{
    ProgressBankPlan plan = MakePlan("ReplayScene0ZeroEventTableSeed801C4FC8");
    const bool zeroCount = countKnown && count == 0u;
    plan.blockedByP0Gap = !zeroCount;

    AppendAction(
        plan,
        ProgressBankActionKind::GateReplayEventTableZeroCount801C4FC8,
        kFn801C4FC8,
        kScene0ReplayTableCount801C6F84,
        kReplaySourceCarrier800901BC,
        4,
        static_cast<int32_t>(count),
        -1,
        static_cast<int32_t>(count),
        zeroCount,
        true);
    AppendAction(
        plan,
        ProgressBankActionKind::WriteReplayPublishedCountZero801C4FC8,
        kFn801C4FC8,
        kScene0ReplayTableCount801C6F84,
        kReplaySourceCarrier800901BC,
        4,
        static_cast<int32_t>(count),
        -1,
        0,
        zeroCount);
    AppendAction(plan,
                 ProgressBankActionKind::PreserveReplayMirror801C4FC8,
                 kFn801C4FC8,
                 kReplayMirrorSrc8008EEF8,
                 kReplayMirrorSrc8008EEF8,
                 kReplayMirrorByteCount,
                 static_cast<int32_t>(count),
                 -1,
                 0,
                 zeroCount,
                 true);
    AppendAction(plan,
                 ProgressBankActionKind::PreserveReplayCursor801C4FC8,
                 kFn801C4FC8,
                 kReplayRuntimeCursor800901C0,
                 kReplayRuntimeCursor800901C0,
                 4,
                 static_cast<int32_t>(count),
                 -1,
                 0,
                 zeroCount,
                 true);
    if (plan.blockedByP0Gap) {
        AppendAction(plan,
                     ProgressBankActionKind::Gap,
                     kFn801C4FC8,
                     kScene0ReplayTableCount801C6F84,
                     kReplaySourceCarrier800901BC,
                     4,
                     static_cast<int32_t>(count));
    }
    return plan;
}

ProgressBankPlan BuildReplayStage1EventTableSeed801C8660Plan(
    bool sourceTableKnown)
{
    ProgressBankPlan plan = MakePlan("ReplayStage1EventTableSeed801C8660");
    constexpr uint32_t kStage1ReplayTableBytes =
        kStage1ReplayTableEntryCount * kReplayMirrorEntryStride;
    plan.blockedByP0Gap = !sourceTableKnown;

    AppendAction(
        plan,
        ProgressBankActionKind::GateReplayStage1EventTableSource801C8660,
        kFn801C8660,
        kStage1ReplayTableSource801D2E2C,
        kReplayMirrorSrc8008EEF8,
        kStage1ReplayTableBytes,
        static_cast<int32_t>(kStage1ReplayTableEntryCount),
        -1,
        static_cast<int32_t>(kStage1ReplayTableEntryCount),
        sourceTableKnown,
        true);
    AppendAction(
        plan,
        ProgressBankActionKind::WriteReplayStage1EventTableTicks801C8660,
        kFn801C8660,
        kStage1ReplayTableSource801D2E2C,
        kReplayMirrorSrc8008EEF8,
        kStage1ReplayTableBytes,
        static_cast<int32_t>(kStage1ReplayTableEntryCount),
        -1,
        static_cast<int32_t>(kStage1ReplayTableEntryCount),
        sourceTableKnown);
    AppendAction(
        plan,
        ProgressBankActionKind::WriteReplayStage1EventTableMasks801C8660,
        kFn801C8660,
        kStage1ReplayTableSource801D2E2C,
        kReplayMaskMirror8008EEFC,
        kStage1ReplayTableBytes,
        static_cast<int32_t>(kStage1ReplayTableEntryCount),
        -1,
        static_cast<int32_t>(kStage1ReplayTableEntryCount),
        sourceTableKnown);
    AppendAction(plan,
                 ProgressBankActionKind::WriteReplayPublishedCount801C8660,
                 kFn801C8660,
                 kStage1ReplayTableSource801D2E2C,
                 kReplaySourceCarrier800901BC,
                 4,
                 static_cast<int32_t>(kStage1ReplayTableEntryCount),
                 -1,
                 static_cast<int32_t>(kStage1ReplayTableEntryCount),
                 sourceTableKnown);
    AppendAction(plan,
                 ProgressBankActionKind::PreserveReplayCursor801C8660,
                 kFn801C8660,
                 kReplayRuntimeCursor800901C0,
                 kReplayRuntimeCursor800901C0,
                 4,
                 static_cast<int32_t>(kStage1ReplayTableEntryCount),
                 -1,
                 0,
                 sourceTableKnown,
                 true);
    if (plan.blockedByP0Gap) {
        AppendAction(plan,
                     ProgressBankActionKind::Gap,
                     kFn801C8660,
                     kStage1ReplayTableSource801D2E2C,
                     kReplaySourceCarrier800901BC,
                     4,
                     static_cast<int32_t>(kStage1ReplayTableEntryCount));
    }
    return plan;
}

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
    uint32_t mask)
{
    ProgressBankPlan plan = MakePlan("ReplayScriptedCursorConsume801C7A60");
    const bool boundsKnown = cursorKnown && publishedCountKnown;
    const bool cursorCanRead =
        boundsKnown && cursor < publishedCount &&
        cursor < kReplayMirrorMaxEntryCount;
    const bool tickGateKnown =
        cursorCanRead && currentTickKnown && nextTickKnown;
    const bool consumeReady = tickGateKnown && currentTick >= nextTick;
    const bool noConsumeKnown =
        boundsKnown &&
        (!cursorCanRead || (tickGateKnown && currentTick < nextTick));
    const bool cursorAuthorityKnown =
        noConsumeKnown || (consumeReady && maskKnown);
    const uint32_t nextCursor = cursor + 1u;
    plan.blockedByP0Gap = !cursorAuthorityKnown;

    AppendAction(plan,
                 ProgressBankActionKind::GateReplayScriptedCursorBounds801C7A60,
                 kFn801C7A60,
                 kReplayRuntimeCursor800901C0,
                 kReplaySourceCarrier800901BC,
                 8,
                 static_cast<int32_t>(currentTick),
                 static_cast<int32_t>(cursor),
                 static_cast<int32_t>(publishedCount),
                 boundsKnown,
                 true);
    AppendAction(plan,
                 ProgressBankActionKind::GateReplayScriptedNextTick801C7A60,
                 kFn801C7A60,
                 kReplayMirrorSrc8008EEF8,
                 0,
                 4,
                 static_cast<int32_t>(currentTick),
                 static_cast<int32_t>(cursor),
                 static_cast<int32_t>(nextTick),
                 tickGateKnown,
                 cursorCanRead);
    AppendAction(plan,
                 ProgressBankActionKind::ReadReplayScriptedMask801C7A60,
                 kFn801C7A60,
                 kReplayMaskMirror8008EEFC,
                 0,
                 4,
                 static_cast<int32_t>(currentTick),
                 static_cast<int32_t>(cursor),
                 static_cast<int32_t>(mask),
                 consumeReady && maskKnown,
                 consumeReady);
    AppendAction(plan,
                 ProgressBankActionKind::WriteReplayScriptedCursorAdvance801C7A60,
                 kFn801C7A60,
                 kReplayRuntimeCursor800901C0,
                 kReplayRuntimeCursor800901C0,
                 4,
                 static_cast<int32_t>(currentTick),
                 static_cast<int32_t>(cursor),
                 static_cast<int32_t>(nextCursor),
                 consumeReady && maskKnown,
                 consumeReady);
    AppendAction(plan,
                 ProgressBankActionKind::PreserveReplayScriptedCursor801C7A60,
                 kFn801C7A60,
                 kReplayRuntimeCursor800901C0,
                 kReplayRuntimeCursor800901C0,
                 4,
                 static_cast<int32_t>(currentTick),
                 static_cast<int32_t>(cursor),
                 static_cast<int32_t>(cursor),
                 noConsumeKnown,
                 true);
    AppendAction(plan,
                 ProgressBankActionKind::PreserveReplayPublishedCount801C7A60,
                 kFn801C7A60,
                 kReplaySourceCarrier800901BC,
                 kReplaySourceCarrier800901BC,
                 4,
                 static_cast<int32_t>(currentTick),
                 static_cast<int32_t>(cursor),
                 static_cast<int32_t>(publishedCount),
                 boundsKnown,
                 true);
    if (plan.blockedByP0Gap) {
        AppendAction(plan,
                     ProgressBankActionKind::Gap,
                     kFn801C7A60,
                     kReplayRuntimeCursor800901C0,
                     kReplaySourceCarrier800901BC,
                     8,
                     static_cast<int32_t>(currentTick),
                     static_cast<int32_t>(cursor));
    }
    return plan;
}

ProgressBankPlan BuildReplaySetupReset80024E54Plan(bool argumentKnown,
                                                   int32_t argument)
{
    ProgressBankPlan plan = MakePlan("ReplaySetupReset80024E54");
    const bool argIsZero = argumentKnown && argument == 0;
    plan.blockedByP0Gap = !argIsZero;

    AppendAction(plan,
                 ProgressBankActionKind::GateReplaySetupArg80024E54,
                 kFn80024E54,
                 0,
                 0,
                 4,
                 argument,
                 -1,
                 argument,
                 argIsZero,
                 true);
    AppendAction(plan,
                 ProgressBankActionKind::WriteReplayCursorZero80024E54,
                 kFn80024E54,
                 0,
                 kReplayRuntimeCursor800901C0,
                 4,
                 argument,
                 -1,
                 0,
                 argIsZero);
    AppendAction(plan,
                 ProgressBankActionKind::WriteReplayPublishedCountZero80024E54,
                 kFn80024E54,
                 0,
                 kReplaySourceCarrier800901BC,
                 4,
                 argument,
                 -1,
                 0,
                 argIsZero);
    AppendAction(plan,
                 ProgressBankActionKind::PreserveReplayMirror80024E54,
                 kFn80024E54,
                 kReplayMirrorSrc8008EEF8,
                 kReplayMirrorSrc8008EEF8,
                 kReplayMirrorByteCount,
                 argument,
                 -1,
                 0,
                 argIsZero,
                 true);
    if (plan.blockedByP0Gap) {
        AppendAction(plan,
                     ProgressBankActionKind::Gap,
                     kFn80024E54,
                     0,
                     kReplaySourceCarrier800901BC,
                     4,
                     argument);
    }
    return plan;
}

ProgressBankPlan BuildSavedScoreSync800169E0Plan(int32_t word800916D0,
                                                  int32_t word800916E2,
                                                  bool scoreDwordKnown)
{
    ProgressBankPlan plan = MakePlan("SavedScoreSync800169E0");
    const SavedScoreSyncSourceSpec& spec = SavedScoreSync800169E0();
    if (word800916D0 != spec.requiredBranchValue) {
        plan.blockedByP0Gap = false;
        return plan;
    }

    int32_t slot = -1;
    const bool mapped = TryMapSaveStage8001615C(word800916E2, &slot);
    const bool scoreSourceKnown =
        !spec.requiresKnownScoreDword || scoreDwordKnown;
    plan.blockedByP0Gap = !mapped || !scoreSourceKnown;

    AppendAction(plan,
                 ProgressBankActionKind::MapStage8001615C,
                 spec.stageMapFunction,
                 spec.stageMapTableAddress,
                 0,
                 0,
                 word800916E2,
                 slot,
                 slot,
                 mapped);
    if (mapped) {
        AppendAction(plan,
                     ProgressBankActionKind::GateSavedScoreSource800169E0,
                     spec.psxFunction,
                     SavedScoreAddressForSlot(slot),
                     spec.scoreBankAddress,
                     spec.scoreSlotStride,
                     word800916E2,
                     slot,
                     word800916D0,
                     scoreSourceKnown,
                     true);
        AppendAction(plan,
                     ProgressBankActionKind::GateSavedScoreCtxTarget800169E0,
                     spec.psxFunction,
                     SavedScoreAddressForSlot(slot),
                     spec.ctxScoreOffset,
                     spec.ctxWriteByteCount,
                     word800916E2,
                     slot,
                     word800916D0,
                     scoreSourceKnown,
                     true);
        AppendAction(plan,
                     ProgressBankActionKind::ReadScore800169E0,
                     spec.psxFunction,
                     SavedScoreAddressForSlot(slot),
                     spec.ctxScoreOffset,
                     spec.ctxWriteByteCount,
                     word800916E2,
                     slot,
                     0,
                     scoreSourceKnown);
        AppendAction(plan,
                     ProgressBankActionKind::WriteWord80091816,
                     spec.psxFunction,
                     spec.ctxScoreOffset,
                     spec.wordMirrorAddress,
                     spec.wordMirrorByteCount,
                     word800916E2,
                     slot,
                     0,
                     scoreSourceKnown);
    }
    if (plan.blockedByP0Gap) {
        AppendAction(plan,
                     ProgressBankActionKind::Gap,
                     spec.psxFunction,
                     mapped ? SavedScoreAddressForSlot(slot) : 0,
                     spec.wordMirrorAddress,
                     spec.wordMirrorByteCount,
                     word800916E2,
                     slot);
    }
    return plan;
}

ProgressBankPlan BuildStageSelectStatusSource800267F8Plan(
    int32_t word800916DA,
    int32_t word800916F0,
    bool statusBankKnown,
    bool word800916F0Known)
{
    ProgressBankPlan plan = MakePlan("StageSelectStatusSource800267F8");
    const Word800916F0ConsumerSpec& f0Consumer =
        Word800916F0StageSelectConsumer800267F8();
    plan.blockedByP0Gap = false;

    AppendAction(plan,
                 ProgressBankActionKind::GateWord800916F0Source800267F8,
                 f0Consumer.psxFunction,
                 f0Consumer.wordAddress,
                 0,
                 2,
                 word800916DA,
                 -1,
                 word800916F0,
                 word800916F0Known,
                 true);
    if (!word800916F0Known) {
        AppendAction(plan,
                     ProgressBankActionKind::Gap,
                     f0Consumer.psxFunction,
                     f0Consumer.wordAddress,
                     0,
                     2);
        plan.blockedByP0Gap = true;
        return plan;
    }

    if (word800916F0 == f0Consumer.specialValue) {
        AppendAction(plan,
                     ProgressBankActionKind::ForceEnableAll800916F0,
                     f0Consumer.psxFunction,
                     f0Consumer.wordAddress,
                     0,
                     18,
                     0,
                     -1,
                     f0Consumer.specialValue,
                     true,
                     true);
        AppendAction(plan,
                     ProgressBankActionKind::WriteStageSelectForceTail800267F8,
                     f0Consumer.psxFunction,
                     f0Consumer.wordAddress,
                     22,
                     2,
                     0,
                     -1,
                     2,
                     true,
                     true);
        AppendAction(plan,
                     ProgressBankActionKind::WriteStageSelectForceTail800267F8,
                     f0Consumer.psxFunction,
                     f0Consumer.wordAddress,
                     24,
                     2,
                     0,
                     -1,
                     3,
                     true,
                     true);
        AppendAction(plan,
                     ProgressBankActionKind::Gap,
                     f0Consumer.psxFunction,
                     f0Consumer.wordAddress,
                     0,
                     2);
        plan.blockedByP0Gap = true;
        return plan;
    }

    if (word800916DA == 1) {
        AppendAction(plan,
                     ProgressBankActionKind::RecordsMode800916DA,
                     kFn800267F8,
                     kWord800916DA,
                     0,
                     2,
                     0,
                     -1,
                     word800916DA,
                     true);
        return plan;
    }

    AppendAction(plan,
                 ProgressBankActionKind::GateStageSelectStatusSource800267F8,
                 kFn800267F8,
                 kStatusBank80092F1D,
                 f0Consumer.wordAddress,
                 kStatusBankStageSelectByteCount,
                 word800916DA,
                 word800916F0,
                 0,
                 statusBankKnown,
                 true);
    AppendAction(plan,
                 ProgressBankActionKind::ReadStageSelectStatus800267F8,
                 kFn800267F8,
                 kStatusBank80092F1D,
                 0,
                 kStatusBankStageSelectByteCount,
                 0,
                 -1,
                 0,
                 statusBankKnown);
    if (!statusBankKnown) {
        AppendAction(plan,
                     ProgressBankActionKind::Gap,
                     kFn800267F8,
                     kStatusBank80092F1D,
                     0,
                     kStatusBankStageSelectByteCount);
        plan.blockedByP0Gap = true;
    }
    return plan;
}

const char* StageProgressStatusName(StageProgressStatus status)
{
    switch (status) {
    case StageProgressStatus::Locked:
        return "Locked";
    case StageProgressStatus::Unlocked:
        return "Unlocked";
    case StageProgressStatus::Clear:
        return "Clear";
    case StageProgressStatus::HighClear:
        return "HighClear";
    case StageProgressStatus::Unknown:
        return "Unknown";
    }
    return "Unknown";
}

const char* ProgressBankFieldKindName(ProgressBankFieldKind kind)
{
    switch (kind) {
    case ProgressBankFieldKind::Prefix:
        return "Prefix";
    case ProgressBankFieldKind::SaveNameSuffix:
        return "SaveNameSuffix";
    case ProgressBankFieldKind::StageStatusBytes:
        return "StageStatusBytes";
    case ProgressBankFieldKind::StageScoreDwords:
        return "StageScoreDwords";
    case ProgressBankFieldKind::LastSavedSlot:
        return "LastSavedSlot";
    case ProgressBankFieldKind::PreviousGrade:
        return "PreviousGrade";
    case ProgressBankFieldKind::AllClearFlag:
        return "AllClearFlag";
    case ProgressBankFieldKind::ReplaySourcePointer:
        return "ReplaySourcePointer";
    case ProgressBankFieldKind::ReplayMirror:
        return "ReplayMirror";
    case ProgressBankFieldKind::BackupPrefix:
        return "BackupPrefix";
    }
    return "Unknown";
}

const char* ProgressBankActionKindName(ProgressBankActionKind kind)
{
    switch (kind) {
    case ProgressBankActionKind::None:
        return "None";
    case ProgressBankActionKind::ClearBytes80015CC4:
        return "ClearBytes80015CC4";
    case ProgressBankActionKind::CopyBytes80015700:
        return "CopyBytes80015700";
    case ProgressBankActionKind::GateBackupSource80015700:
        return "GateBackupSource80015700";
    case ProgressBankActionKind::CopyBytes80015744:
        return "CopyBytes80015744";
    case ProgressBankActionKind::GateRestoreTarget80015744:
        return "GateRestoreTarget80015744";
    case ProgressBankActionKind::CopyBytes800164B4:
        return "CopyBytes800164B4";
    case ProgressBankActionKind::GateLoadPayloadSource800164B4:
        return "GateLoadPayloadSource800164B4";
    case ProgressBankActionKind::MapStage8001615C:
        return "MapStage8001615C";
    case ProgressBankActionKind::MapScene800161A8:
        return "MapScene800161A8";
    case ProgressBankActionKind::QueryAllClear800161F4:
        return "QueryAllClear800161F4";
    case ProgressBankActionKind::QueryStatus800166AC:
        return "QueryStatus800166AC";
    case ProgressBankActionKind::GateStageStatusSource8001670C:
        return "GateStageStatusSource8001670C";
    case ProgressBankActionKind::ReadStageStatusGate8001670C:
        return "ReadStageStatusGate8001670C";
    case ProgressBankActionKind::ConditionalUnlock8001628C:
        return "ConditionalUnlock8001628C";
    case ProgressBankActionKind::MaxPromoteStatus8001635C:
        return "MaxPromoteStatus8001635C";
    case ProgressBankActionKind::WriteScore8001635C:
        return "WriteScore8001635C";
    case ProgressBankActionKind::WriteLastSlot8001635C:
        return "WriteLastSlot8001635C";
    case ProgressBankActionKind::WritePrevGrade8001635C:
        return "WritePrevGrade8001635C";
    case ProgressBankActionKind::WriteReplaySource8001635C:
        return "WriteReplaySource8001635C";
    case ProgressBankActionKind::GateReplaySourceCarrier8001635C:
        return "GateReplaySourceCarrier8001635C";
    case ProgressBankActionKind::CopyReplayMirror8001635C:
        return "CopyReplayMirror8001635C";
    case ProgressBankActionKind::GateReplayMirrorSource8001635C:
        return "GateReplayMirrorSource8001635C";
    case ProgressBankActionKind::WriteAllClear8001635C:
        return "WriteAllClear8001635C";
    case ProgressBankActionKind::GatePreviousGradeSource80016758:
        return "GatePreviousGradeSource80016758";
    case ProgressBankActionKind::ReadPreviousGradeGate80016758:
        return "ReadPreviousGradeGate80016758";
    case ProgressBankActionKind::GateSetupBranchWord800916D0801C7A60:
        return "GateSetupBranchWord800916D0801C7A60";
    case ProgressBankActionKind::GateSetupLatchSource801C7A60:
        return "GateSetupLatchSource801C7A60";
    case ProgressBankActionKind::WriteWord8009182A800143F0:
        return "WriteWord8009182A800143F0";
    case ProgressBankActionKind::WriteWord8008ED34800259C0:
        return "WriteWord8008ED34800259C0";
    case ProgressBankActionKind::GateSetupTailRecordsMode801C7A60:
        return "GateSetupTailRecordsMode801C7A60";
    case ProgressBankActionKind::ClearWord8009182A800143F0:
        return "ClearWord8009182A800143F0";
    case ProgressBankActionKind::ClearWord8008ED34800259C0:
        return "ClearWord8008ED34800259C0";
    case ProgressBankActionKind::WriteStatus800167A8:
        return "WriteStatus800167A8";
    case ProgressBankActionKind::ReadScore800169E0:
        return "ReadScore800169E0";
    case ProgressBankActionKind::GateSavedScoreSource800169E0:
        return "GateSavedScoreSource800169E0";
    case ProgressBankActionKind::GateSavedScoreCtxTarget800169E0:
        return "GateSavedScoreCtxTarget800169E0";
    case ProgressBankActionKind::WriteWord80091816:
        return "WriteWord80091816";
    case ProgressBankActionKind::GateWord800916E2Source80015D18:
        return "GateWord800916E2Source80015D18";
    case ProgressBankActionKind::WriteWord800916E2:
        return "WriteWord800916E2";
    case ProgressBankActionKind::GateReplayProducerFamily8001635C:
        return "GateReplayProducerFamily8001635C";
    case ProgressBankActionKind::GateReplaySnapshotMirrorSource8001635C:
        return "GateReplaySnapshotMirrorSource8001635C";
    case ProgressBankActionKind::GateReplayPublishedCount8001635C:
        return "GateReplayPublishedCount8001635C";
    case ProgressBankActionKind::GateReplayCursorSource8001635C:
        return "GateReplayCursorSource8001635C";
    case ProgressBankActionKind::GateReplayRestorePayload8001681C:
        return "GateReplayRestorePayload8001681C";
    case ProgressBankActionKind::WriteReplayPublishedCount8001681C:
        return "WriteReplayPublishedCount8001681C";
    case ProgressBankActionKind::CopyReplayMirrorRestore8001681C:
        return "CopyReplayMirrorRestore8001681C";
    case ProgressBankActionKind::GateReplaySetupTransitionState80024E54:
        return "GateReplaySetupTransitionState80024E54";
    case ProgressBankActionKind::PreserveReplayCursorAfterRestore8001681C:
        return "PreserveReplayCursorAfterRestore8001681C";
    case ProgressBankActionKind::GateReplayAppendAccepted80014614:
        return "GateReplayAppendAccepted80014614";
    case ProgressBankActionKind::GateReplayAppendBounds80014614:
        return "GateReplayAppendBounds80014614";
    case ProgressBankActionKind::WriteReplayAppendMask80014614:
        return "WriteReplayAppendMask80014614";
    case ProgressBankActionKind::WriteReplayAppendTick80014614:
        return "WriteReplayAppendTick80014614";
    case ProgressBankActionKind::WriteReplayCursorIncrement80014614:
        return "WriteReplayCursorIncrement80014614";
    case ProgressBankActionKind::WriteReplayPublishedCountFromCursor80014614:
        return "WriteReplayPublishedCountFromCursor80014614";
    case ProgressBankActionKind::GateReplayEventTableZeroCount801C4FC8:
        return "GateReplayEventTableZeroCount801C4FC8";
    case ProgressBankActionKind::WriteReplayPublishedCountZero801C4FC8:
        return "WriteReplayPublishedCountZero801C4FC8";
    case ProgressBankActionKind::PreserveReplayMirror801C4FC8:
        return "PreserveReplayMirror801C4FC8";
    case ProgressBankActionKind::PreserveReplayCursor801C4FC8:
        return "PreserveReplayCursor801C4FC8";
    case ProgressBankActionKind::GateReplayStage1EventTableSource801C8660:
        return "GateReplayStage1EventTableSource801C8660";
    case ProgressBankActionKind::WriteReplayStage1EventTableTicks801C8660:
        return "WriteReplayStage1EventTableTicks801C8660";
    case ProgressBankActionKind::WriteReplayStage1EventTableMasks801C8660:
        return "WriteReplayStage1EventTableMasks801C8660";
    case ProgressBankActionKind::WriteReplayPublishedCount801C8660:
        return "WriteReplayPublishedCount801C8660";
    case ProgressBankActionKind::PreserveReplayCursor801C8660:
        return "PreserveReplayCursor801C8660";
    case ProgressBankActionKind::GateReplayScriptedCursorBounds801C7A60:
        return "GateReplayScriptedCursorBounds801C7A60";
    case ProgressBankActionKind::GateReplayScriptedNextTick801C7A60:
        return "GateReplayScriptedNextTick801C7A60";
    case ProgressBankActionKind::ReadReplayScriptedMask801C7A60:
        return "ReadReplayScriptedMask801C7A60";
    case ProgressBankActionKind::WriteReplayScriptedCursorAdvance801C7A60:
        return "WriteReplayScriptedCursorAdvance801C7A60";
    case ProgressBankActionKind::PreserveReplayScriptedCursor801C7A60:
        return "PreserveReplayScriptedCursor801C7A60";
    case ProgressBankActionKind::PreserveReplayPublishedCount801C7A60:
        return "PreserveReplayPublishedCount801C7A60";
    case ProgressBankActionKind::GateReplaySetupArg80024E54:
        return "GateReplaySetupArg80024E54";
    case ProgressBankActionKind::WriteReplayCursorZero80024E54:
        return "WriteReplayCursorZero80024E54";
    case ProgressBankActionKind::WriteReplayPublishedCountZero80024E54:
        return "WriteReplayPublishedCountZero80024E54";
    case ProgressBankActionKind::PreserveReplayMirror80024E54:
        return "PreserveReplayMirror80024E54";
    case ProgressBankActionKind::GateWord800916F0Source800267F8:
        return "GateWord800916F0Source800267F8";
    case ProgressBankActionKind::GateStageSelectStatusSource800267F8:
        return "GateStageSelectStatusSource800267F8";
    case ProgressBankActionKind::ReadStageSelectStatus800267F8:
        return "ReadStageSelectStatus800267F8";
    case ProgressBankActionKind::ForceEnableAll800916F0:
        return "ForceEnableAll800916F0";
    case ProgressBankActionKind::WriteStageSelectForceTail800267F8:
        return "WriteStageSelectForceTail800267F8";
    case ProgressBankActionKind::RecordsMode800916DA:
        return "RecordsMode800916DA";
    case ProgressBankActionKind::Gap:
        return "Gap";
    }
    return "Unknown";
}

const char* ProgressBankCopyProvenanceKindName(
    ProgressBankCopyProvenanceKind kind)
{
    switch (kind) {
    case ProgressBankCopyProvenanceKind::Backup80015700:
        return "Backup80015700";
    case ProgressBankCopyProvenanceKind::Restore80015744:
        return "Restore80015744";
    case ProgressBankCopyProvenanceKind::LoadPayload800164B4:
        return "LoadPayload800164B4";
    }
    return "Unknown";
}

const char* ProgressBankPayloadSourceKindName(
    ProgressBankPayloadSourceKind kind)
{
    switch (kind) {
    case ProgressBankPayloadSourceKind::Unknown:
        return "Unknown";
    case ProgressBankPayloadSourceKind::RuntimeLowerCardProducer:
        return "RuntimeLowerCardProducer";
    case ProgressBankPayloadSourceKind::DebugSyntheticFixture:
        return "DebugSyntheticFixture";
    case ProgressBankPayloadSourceKind::HostFilesystem:
        return "HostFilesystem";
    }
    return "Unknown";
}

const char* ReplayMirrorProducerKindName(ReplayMirrorProducerKind kind)
{
    switch (kind) {
    case ReplayMirrorProducerKind::Stage1EventTableBuild:
        return "Stage1EventTableBuild";
    case ReplayMirrorProducerKind::RuntimeAcceptedAppend:
        return "RuntimeAcceptedAppend";
    case ReplayMirrorProducerKind::PayloadRestore8001681C:
        return "PayloadRestore8001681C";
    }
    return "Unknown";
}

const char* ReplayMirrorResetKindName(ReplayMirrorResetKind kind)
{
    switch (kind) {
    case ReplayMirrorResetKind::EventTableClear801C4AB8:
        return "EventTableClear801C4AB8";
    case ReplayMirrorResetKind::EventTableClear801C4988:
        return "EventTableClear801C4988";
    case ReplayMirrorResetKind::SetupReset80024E54:
        return "SetupReset80024E54";
    }
    return "Unknown";
}

const char* ScoreAccumulatorWriterKindName(ScoreAccumulatorWriterKind kind)
{
    switch (kind) {
    case ScoreAccumulatorWriterKind::Reset80014344:
        return "Reset80014344";
    case ScoreAccumulatorWriterKind::GameplayCommit80014D58:
        return "GameplayCommit80014D58";
    case ScoreAccumulatorWriterKind::SavedScoreSync800169E0:
        return "SavedScoreSync800169E0";
    }
    return "Unknown";
}

const char* SavedScoreSyncSourceKindName(SavedScoreSyncSourceKind kind)
{
    switch (kind) {
    case SavedScoreSyncSourceKind::ScoreBankSlot800169E0:
        return "ScoreBankSlot800169E0";
    }
    return "Unknown";
}

const char* SetupLatchSameValueCallsiteKindName(
    SetupLatchSameValueCallsiteKind kind)
{
    switch (kind) {
    case SetupLatchSameValueCallsiteKind::StageRunner801C7A60:
        return "StageRunner801C7A60";
    }
    return "Unknown";
}

const char* Word800916F0ConsumerKindName(Word800916F0ConsumerKind kind)
{
    switch (kind) {
    case Word800916F0ConsumerKind::StageSelectForceEnable800267F8:
        return "StageSelectForceEnable800267F8";
    case Word800916F0ConsumerKind::ComodPostClearSaveGateFamily:
        return "ComodPostClearSaveGateFamily";
    }
    return "Unknown";
}

} // namespace PrSS0StageProgressBankDirect
