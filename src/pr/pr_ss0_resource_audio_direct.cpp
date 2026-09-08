#include "pr_ss0_resource_audio_direct.h"

namespace PrSS0ResourceAudioDirect {
namespace {

struct TimBlockReplayBaseline {
    KnownArchive archive = KnownArchive::Unknown;
    uint32_t blockIndex = 0;
    uint32_t timRecordCount = 0;
    uint32_t firstFrame = 0;
    uint32_t lastFrame = 0;
    uint32_t gp0CopyPacketCount = 0;
    uint32_t changedBytes = 0;
};

struct VabReplayBaseline {
    KnownArchive archive = KnownArchive::Unknown;
    uint32_t frame = 0;
    uint32_t changedBytesToNextAnchor = 0;
    uint32_t closeCallCount = 0;
    uint32_t openCallCount = 0;
    uint32_t transferCallCount = 0;
    uint32_t enableCallCount = 0;
};

struct VabCloseReplayBaseline {
    KnownArchive archive = KnownArchive::Unknown;
    uint32_t frame = 0;
    uint32_t changedBytesToNextAnchor = 0;
    uint32_t closeCallCount = 0;
};

struct MemReplayBaseline {
    KnownArchive archive = KnownArchive::Unknown;
    uint32_t frame = 0;
    uint32_t memRecordCount = 0;
    uint32_t handleNonzeroCount = 0;
};

struct SfxCallTotalReplayBaseline {
    uint32_t psxFunction = 0;
    uint32_t frameCount = 0;
    uint32_t callCount = 0;
    uint32_t topCallsite = 0;
    uint32_t topCallsiteCallCount = 0;
};

struct SfxCallAnchorReplayBaseline {
    uint32_t frame = 0;
    uint32_t inputCallCount = 0;
    uint32_t cueCallCount = 0;
    uint32_t flushCallCount = 0;
    uint32_t replaceCallCount = 0;
    uint32_t spuCallCount = 0;
    uint32_t inputCallsite = 0;
    uint32_t cueCallsite = 0;
    uint32_t primaryFlushCallsite = 0;
    uint32_t secondaryFlushCallsite = 0;
    uint32_t spuCallsite = 0;
};

constexpr TimBlockReplayBaseline kTimBlockReplayBaselines[] = {
    {KnownArchive::Common, 0, 53, 174, 174, 106, 26945},
    {KnownArchive::ZCompo, 0, 408, 302, 304, 816, 241106},
    {KnownArchive::ZCompo, 1, 173, 330, 331, 346, 44673},
    {KnownArchive::Compo00, 0, 87, 1371, 1372, 174, 152175},
    {KnownArchive::YCompo, 0, 91, 9881, 9882, 182, 79379},
};

constexpr VabReplayBaseline kVabReplayBaselines[] = {
    {KnownArchive::Common, 195, 0, 1, 1, 1, 1},
    {KnownArchive::ZCompo, 351, 0, 1, 1, 1, 1},
    {KnownArchive::Compo00, 1392, 0, 1, 1, 1, 1},
    {KnownArchive::YCompo, 9964, 0, 1, 1, 1, 1},
};

constexpr VabCloseReplayBaseline kVabCloseReplayBaselines[] = {
    {KnownArchive::Unknown, 202, 0, 1},
};

constexpr MemReplayBaseline kMemReplayBaselines[] = {
    {KnownArchive::Compo00, 1445, 59, 60},
};

constexpr SfxCallTotalReplayBaseline kSfxCallTotalReplayBaselines[] = {
    {kFn80025C8C, 51, 51, 0x800266D0u, 6},
    {kFn80026EF8, 217, 219, 0x800271B0u, 139},
    {kFn80026ECC, 3619, 3668, 0x80026E34u, 2200},
    {kFn80026FC4, 0, 0, 0, 0},
    {kFn80034240, 217, 219, 0x80026F34u, 219},
};

constexpr SfxCallAnchorReplayBaseline kSfxCallAnchorReplayBaselines[] = {
    {7745, 0, 1, 1, 0, 1, 0, 0x801C4A70u, 0x801C4A78u, 0, 0x80026F34u},
    {7812, 1, 1, 1, 0, 1, 0x801C4824u, 0x80025D50u, 0x80025D58u, 0, 0x80026F34u},
    {7822, 1, 1, 1, 0, 1, 0x801C4824u, 0x80025D50u, 0x80025D58u, 0, 0x80026F34u},
    {7854, 0, 1, 1, 0, 1, 0, 0x800271B0u, 0x800271B8u, 0, 0x80026F34u},
    {7910, 0, 1, 1, 0, 1, 0, 0x800157ACu, 0x800157B4u, 0, 0x80026F34u},
    {8230, 1, 1, 2, 0, 1, 0x800265C8u, 0x80025D50u, 0x80025D58u, 0x80026E34u, 0x80026F34u},
    {8339, 1, 1, 2, 0, 1, 0x80025FA0u, 0x80025D50u, 0x80025D58u, 0x80026E34u, 0x80026F34u},
    {8375, 1, 1, 2, 0, 1, 0x80025FA0u, 0x80025D50u, 0x80025D58u, 0x80026E34u, 0x80026F34u},
    {8468, 1, 1, 2, 0, 1, 0x80026140u, 0x80025D50u, 0x80025D58u, 0x80026E34u, 0x80026F34u},
    {8619, 1, 1, 2, 0, 1, 0x800265E8u, 0x80025D50u, 0x80025D58u, 0x80026E34u, 0x80026F34u},
    {8912, 1, 1, 1, 0, 1, 0x8001827Cu, 0x80025D50u, 0x80025D58u, 0, 0x80026F34u},
    {13408, 1, 1, 2, 0, 1, 0x80026678u, 0x80025D50u, 0x80025D58u, 0x80026E34u, 0x80026F34u},
    {13663, 1, 1, 2, 0, 1, 0x80026640u, 0x80025D50u, 0x80025D58u, 0x80026E34u, 0x80026F34u},
    {13874, 0, 1, 1, 0, 1, 0, 0x80027200u, 0x80027208u, 0, 0x80026F34u},
    {13903, 0, 1, 1, 0, 1, 0, 0x80027200u, 0x80027208u, 0, 0x80026F34u},
};

static bool Append(ResourceAudioPlan& plan, const ResourceAudioAction& action)
{
    if (plan.count >= sizeof(plan.actions) / sizeof(plan.actions[0])) {
        plan.truncated = true;
        return false;
    }
    plan.actions[plan.count++] = action;
    return true;
}

static void AppendAction(ResourceAudioPlan& plan,
                         ResourceAudioActionKind kind,
                         uint32_t psxFunction,
                         KnownArchive archive = KnownArchive::Unknown,
                         CueKind cue = CueKind::Unknown,
                         uint32_t arg0 = 0,
                         uint32_t arg1 = 0,
                         uint32_t arg2 = 0,
                         uint32_t arg3 = 0)
{
    ResourceAudioAction action{};
    action.kind = kind;
    action.psxFunction = psxFunction;
    action.archive = archive;
    action.cue = cue;
    action.arg0 = arg0;
    action.arg1 = arg1;
    action.arg2 = arg2;
    action.arg3 = arg3;
    (void)Append(plan, action);
}

static void AppendPlan(ResourceAudioPlan& dst, const ResourceAudioPlan& src)
{
    for (uint32_t i = 0; i < src.count; ++i) {
        (void)Append(dst, src.actions[i]);
    }
    dst.truncated = dst.truncated || src.truncated;
    dst.hasOpenP0Gap = dst.hasOpenP0Gap || src.hasOpenP0Gap;
}

static constexpr uint32_t PackVabReplayLoadCounts(uint32_t openCallCount,
                                                  uint32_t transferCallCount,
                                                  uint32_t enableCallCount)
{
    return (openCallCount & 0xFFu) |
           ((transferCallCount & 0xFFu) << 8) |
           ((enableCallCount & 0xFFu) << 16);
}

static constexpr uint32_t PackSfxAnchorCounts(uint32_t inputCallCount,
                                              uint32_t cueCallCount,
                                              uint32_t flushCallCount,
                                              uint32_t replaceCallCount,
                                              uint32_t spuCallCount)
{
    return (inputCallCount & 0xFu) |
           ((cueCallCount & 0xFu) << 4) |
           ((flushCallCount & 0xFu) << 8) |
           ((replaceCallCount & 0xFu) << 12) |
           ((spuCallCount & 0xFu) << 16);
}

static void AppendVabRecord(ResourceAudioPlan& plan, KnownArchive archive)
{
    AppendAction(plan, ResourceAudioActionKind::Call8001A8F0ParseVab,
                 kFn8001A8F0, archive);
    AppendAction(plan, ResourceAudioActionKind::Call80027120VabClose,
                 kFn80027120, archive);
    AppendAction(plan, ResourceAudioActionKind::PadStartCom, 0, archive);
    AppendAction(plan, ResourceAudioActionKind::Call80027078VabOpen,
                 kFn80027078, archive);
    AppendAction(plan, ResourceAudioActionKind::Call800270D4VabTransfer,
                 kFn800270D4, archive);
    AppendAction(plan, ResourceAudioActionKind::Call800270FCVabEnable,
                 kFn800270FC, archive, CueKind::Unknown, 1);
}

static void AppendVabReplayBaseline(ResourceAudioPlan& plan,
                                    KnownArchive archive)
{
    for (const VabReplayBaseline& baseline : kVabReplayBaselines) {
        if (baseline.archive != archive) {
            continue;
        }
        AppendAction(plan,
                     ResourceAudioActionKind::VabLoadReplayBaseline,
                     kFn80027120,
                     archive,
                     CueKind::Unknown,
                     baseline.frame,
                     baseline.changedBytesToNextAnchor,
                     baseline.closeCallCount,
                     PackVabReplayLoadCounts(baseline.openCallCount,
                                             baseline.transferCallCount,
                                             baseline.enableCallCount));
        return;
    }
}

static void AppendVabCloseReplayBaseline(ResourceAudioPlan& plan,
                                         KnownArchive archive)
{
    for (const VabCloseReplayBaseline& baseline : kVabCloseReplayBaselines) {
        if (baseline.archive != archive) {
            continue;
        }
        AppendAction(plan,
                     ResourceAudioActionKind::VabCloseReplayBaseline,
                     kFn80027120,
                     archive,
                     CueKind::Unknown,
                     baseline.frame,
                     baseline.changedBytesToNextAnchor,
                     baseline.closeCallCount);
        return;
    }
}

static void AppendIntLoad(ResourceAudioPlan& plan,
                          KnownArchive archive,
                          uint32_t descriptor,
                          uint32_t mode)
{
    AppendAction(plan,
                 ResourceAudioActionKind::GateIntLoadRetryAndReset8001AC18,
                 kFn8001AC18,
                 archive,
                 CueKind::Unknown,
                 descriptor,
                 kIntLoadRetryCount8001AC18,
                 kFn80025A34,
                 mode);
    AppendAction(plan, ResourceAudioActionKind::Call8001AC18LoadInt,
                 kFn8001AC18, archive, CueKind::Unknown, descriptor, mode);
}

static ResourceAudioPlan MakePlan()
{
    ResourceAudioPlan plan{};
    plan.runtimeCutoverAllowed = RuntimeCutoverAllowed();
    return plan;
}

static const TimBlockReplayBaseline* FindTimBlockReplayBaseline(
    KnownArchive archive,
    uint32_t blockIndex)
{
    for (const TimBlockReplayBaseline& baseline : kTimBlockReplayBaselines) {
        if (baseline.archive == archive && baseline.blockIndex == blockIndex) {
            return &baseline;
        }
    }
    return nullptr;
}

static void AppendTimBlock(ResourceAudioPlan& plan,
                           const TimBlockReplayBaseline& baseline,
                           uint32_t blockCount)
{
    AppendAction(plan,
                 ResourceAudioActionKind::Call8001A8F0ParseTim,
                 kFn8001A8F0,
                 baseline.archive,
                 CueKind::Unknown,
                 baseline.timRecordCount,
                 baseline.blockIndex,
                 blockCount);
    AppendAction(plan,
                 ResourceAudioActionKind::TimUpload,
                 kFn8001A8F0,
                 baseline.archive,
                 CueKind::Unknown,
                 baseline.timRecordCount,
                 baseline.blockIndex,
                 blockCount);
    AppendAction(plan,
                 ResourceAudioActionKind::TimUploadReplayBaseline,
                 kFn8001A8F0,
                 baseline.archive,
                 CueKind::Unknown,
                 baseline.firstFrame,
                 baseline.lastFrame,
                 baseline.gp0CopyPacketCount,
                 baseline.changedBytes);
}

} // namespace

bool RuntimeCutoverAllowed()
{
    return false;
}

StartupAudioBindings80016C8C BuildStartupAudioBindings80016C8C()
{
    StartupAudioBindings80016C8C out{};
    out.known = true;
    out.complete = true;
    out.sourceFunction = kFn80016C8C;
    out.slot800943FC = 0x80048DA0u;
    out.slot80094400 = 0x80048D94u;
    out.slot80094404 = 0x8006EA6Cu;
    out.slot80094408 = 0x8006EA74u;
    out.slot8009440C = 0x8006EA7Cu;
    out.slot80094410 = 0x8006EA84u;
    out.slot80094414 = 0x8006EA8Cu;
    out.slot80094418 = 0x8006EA94u;
    out.slot8009441C = 0x80048DACu;
    out.slot80094420 = 0x8006EA9Cu;
    out.slot80094424 = 0x8006EAA4u;
    out.slot80094428 = 0x8006EAACu;
    out.slot8009442C = 0x8006EAB4u;
    out.returnValueKnown = true;
    out.returnValue = 0x8006EA84u;
    out.directSoftwareStateAuthority = true;
    out.psxMemoryBackingAuthority = false;
    out.physicalSpuAuthority = false;
    out.hostProjection = false;
    out.replayValueAuthority = false;
    out.oldWinS0Authority = false;
    out.stage2PlusAuthority = false;
    return out;
}

bool IsExactStartupAudioBindings80016C8C(
    const StartupAudioBindings80016C8C& bindings)
{
    return bindings.known && bindings.complete &&
           bindings.sourceFunction == kFn80016C8C &&
           bindings.slot800943FC == 0x80048DA0u &&
           bindings.slot80094400 == 0x80048D94u &&
           bindings.slot80094404 == 0x8006EA6Cu &&
           bindings.slot80094408 == 0x8006EA74u &&
           bindings.slot8009440C == 0x8006EA7Cu &&
           bindings.slot80094410 == 0x8006EA84u &&
           bindings.slot80094414 == 0x8006EA8Cu &&
           bindings.slot80094418 == 0x8006EA94u &&
           bindings.slot8009441C == 0x80048DACu &&
           bindings.slot80094420 == 0x8006EA9Cu &&
           bindings.slot80094424 == 0x8006EAA4u &&
           bindings.slot80094428 == 0x8006EAACu &&
           bindings.slot8009442C == 0x8006EAB4u &&
           bindings.returnValueKnown &&
           bindings.returnValue == 0x8006EA84u &&
           bindings.directSoftwareStateAuthority &&
           !bindings.psxMemoryBackingAuthority &&
           !bindings.physicalSpuAuthority && !bindings.hostProjection &&
           !bindings.replayValueAuthority && !bindings.oldWinS0Authority &&
           !bindings.stage2PlusAuthority;
}

uint32_t KnownTimRecordCount(KnownArchive archive)
{
    uint32_t total = 0;
    for (const TimBlockReplayBaseline& baseline : kTimBlockReplayBaselines) {
        if (baseline.archive == archive) {
            total += baseline.timRecordCount;
        }
    }
    return total;
}

uint32_t KnownTimBlockCount(KnownArchive archive)
{
    uint32_t total = 0;
    for (const TimBlockReplayBaseline& baseline : kTimBlockReplayBaselines) {
        if (baseline.archive == archive) {
            ++total;
        }
    }
    return total;
}

uint32_t KnownTimBlockRecordCount(KnownArchive archive, uint32_t blockIndex)
{
    const TimBlockReplayBaseline* baseline =
        FindTimBlockReplayBaseline(archive, blockIndex);
    return baseline == nullptr ? 0u : baseline->timRecordCount;
}

uint32_t KnownVabRecordCount(KnownArchive archive)
{
    switch (archive) {
    case KnownArchive::Common:
    case KnownArchive::ZCompo:
    case KnownArchive::Compo00:
    case KnownArchive::YCompo:
        return 1;
    case KnownArchive::Unknown:
        return 0;
    }
    return 0;
}

uint32_t KnownMemRecordCount(KnownArchive archive)
{
    switch (archive) {
    case KnownArchive::Compo00:
        return 59;
    case KnownArchive::Common:
    case KnownArchive::ZCompo:
    case KnownArchive::YCompo:
    case KnownArchive::Unknown:
        return 0;
    }
    return 0;
}

static void AppendMemReplayBaseline(ResourceAudioPlan& plan,
                                    KnownArchive archive)
{
    for (const MemReplayBaseline& baseline : kMemReplayBaselines) {
        if (baseline.archive != archive) {
            continue;
        }
        AppendAction(plan,
                     ResourceAudioActionKind::MemMapReplayBaseline,
                     kFn80025BBC,
                     archive,
                     CueKind::Unknown,
                     baseline.frame,
                     baseline.memRecordCount,
                     baseline.handleNonzeroCount);
        return;
    }
}

CueSpec KnownCueSpec(CueKind cue)
{
    CueSpec spec{};
    spec.kind = cue;
    switch (cue) {
    case CueKind::SceneGlobal94410:
        spec.globalPointerAddress = kCueGlobal94410;
        spec.commandAddress = kCueData8006EA84;
        spec.bytes[0] = 0x00;
        spec.bytes[1] = 0x07;
        spec.bytes[2] = 0x1F;
        spec.bytes[3] = 0x5A;
        break;
    case CueKind::Direction94420:
        spec.globalPointerAddress = kCueGlobal94420;
        spec.commandAddress = kCueData8006EA9C;
        spec.bytes[0] = 0x00;
        spec.bytes[1] = 0x0C;
        spec.bytes[2] = 0x24;
        spec.bytes[3] = 0x7F;
        break;
    case CueKind::Cross94424:
        spec.globalPointerAddress = kCueGlobal94424;
        spec.commandAddress = kCueData8006EAA4;
        spec.bytes[0] = 0x00;
        spec.bytes[1] = 0x0D;
        spec.bytes[2] = 0x25;
        spec.bytes[3] = 0x7F;
        break;
    case CueKind::Circle94428:
        spec.globalPointerAddress = kCueGlobal94428;
        spec.commandAddress = kCueData8006EAAC;
        spec.bytes[0] = 0x00;
        spec.bytes[1] = 0x0E;
        spec.bytes[2] = 0x26;
        spec.bytes[3] = 0x7F;
        break;
    case CueKind::Select9442C:
        spec.globalPointerAddress = kCueGlobal9442C;
        spec.commandAddress = kCueData8006EAB4;
        spec.bytes[0] = 0x00;
        spec.bytes[1] = 0x0F;
        spec.bytes[2] = 0x27;
        spec.bytes[3] = 0x7F;
        break;
    default:
        break;
    }
    return spec;
}

CueSpec KnownStage1Event4CueSpec(CueKind cue)
{
    CueSpec spec{};
    spec.kind = cue;
    switch (cue) {
    case CueKind::Stage1Event4Primary9441C:
        spec.globalPointerAddress = kCueGlobal9441C;
        spec.commandAddress = kStage1Event4CueData801CCC68;
        spec.bytes[0] = 0x00u;
        spec.bytes[1] = 0x0Au;
        spec.bytes[2] = 0x22u;
        spec.bytes[3] = 0x5Au;
        break;
    case CueKind::Stage1Event4Secondary9441CPlus6:
        spec.globalPointerAddress = kCueGlobal9441C;
        spec.commandAddress = kStage1Event4CueData801CCC68 + 6u;
        spec.bytes[0] = 0x00u;
        spec.bytes[1] = 0x0Bu;
        spec.bytes[2] = 0x23u;
        spec.bytes[3] = 0x5Au;
        break;
    case CueKind::Direction94420:
        spec = KnownCueSpec(cue);
        spec.commandAddress = kStage1Event4DirectionData801CCC74;
        break;
    case CueKind::Cross94424:
        spec = KnownCueSpec(cue);
        spec.commandAddress = kStage1Event4CrossData801CCC7C;
        break;
    case CueKind::Circle94428:
        spec = KnownCueSpec(cue);
        spec.commandAddress = kStage1Event4CircleData801CCC84;
        break;
    case CueKind::Select9442C:
        spec = KnownCueSpec(cue);
        spec.commandAddress = kStage1Event4SelectData801CCC8C;
        break;
    case CueKind::SceneGlobal94410:
    case CueKind::Unknown:
        break;
    }
    return spec;
}

CueKind ResolveInputCue80025C8C(uint32_t inputMask)
{
    switch (inputMask) {
    case 0x20u:
        return CueKind::Circle94428;
    case 0x40u:
        return CueKind::Cross94424;
    case 0x100u:
        return CueKind::Select9442C;
    case 0x1000u:
    case 0x2000u:
    case 0x4000u:
    case 0x8000u:
        return CueKind::Direction94420;
    default:
        return CueKind::Unknown;
    }
}

bool ExecuteCueSoftware80026EF8(
    const CueSpec& sourceBinding,
    const CueDriverBinding800943A8& driverBinding,
    CuePlaySink80034240 play80034240,
    void* userData,
    CueSoftwareTransaction80026EF8& out)
{
    out = {};
    out.cue = sourceBinding.kind;
    out.driverState800943A8 = driverBinding.value;
    out.driverBindingKind = driverBinding.kind;
    out.exactPsxDriverHandleKnown =
        driverBinding.kind == CueDriverBindingKind800943A8::ExactPsxHandle;
    out.hostStage1VabProjection =
        driverBinding.kind ==
        CueDriverBindingKind800943A8::HostStage1VabProjection;

    const bool driverBindingAccepted =
        out.exactPsxDriverHandleKnown ||
        (out.hostStage1VabProjection && driverBinding.hostStage1VabReady);
    if (sourceBinding.kind == CueKind::Unknown ||
        sourceBinding.globalPointerAddress == 0u ||
        sourceBinding.commandAddress == 0u || !driverBindingAccepted ||
        play80034240 == nullptr) {
        return false;
    }

    out.sourceGlobalPointerAddress = sourceBinding.globalPointerAddress;
    out.sourceCommandAddress = sourceBinding.commandAddress;
    out.command[0] = sourceBinding.bytes[0];
    out.command[1] = sourceBinding.bytes[1];
    out.command[2] = sourceBinding.bytes[2];
    out.command[3] = sourceBinding.bytes[3];
    out.sourceAccepted = true;

    // 80026EF8 reads bytes 0/1/3, writes byte 2 = byte 1 + 0x18,
    // then invokes 80034240 and stores its return in 800943AC.
    out.command[2] = static_cast<uint8_t>(out.command[1] + 0x18u);
    out.sourceMutated = true;
    out.playResult80034240 = play80034240(
        out.driverState800943A8,
        out.command[0],
        out.command[1],
        out.command[2],
        0u,
        out.command[3],
        out.command[3],
        userData);
    out.voiceStored800943AC = true;
    out.exactPsxHalParity = false;
    return true;
}

bool ExecuteInputSfxSoftware80025C8C(
    uint32_t inputMask,
    const CueSpec& sourceBinding,
    const CueDriverBinding800943A8& driverBinding,
    const CueSoftwareSinks80025C8C& sinks,
    CueSoftwareTransaction80025C8C& out)
{
    out = {};
    out.inputMask = inputMask;

    const CueKind cue = ResolveInputCue80025C8C(inputMask);
    const CueSpec expectedSlot = KnownCueSpec(cue);
    if (cue == CueKind::Unknown || sourceBinding.kind != cue ||
        sourceBinding.globalPointerAddress !=
            expectedSlot.globalPointerAddress ||
        sinks.flush80026ECC == nullptr) {
        return false;
    }

    CueSoftwareTransaction80026EF8 cueTransaction{};
    if (!ExecuteCueSoftware80026EF8(
            sourceBinding,
            driverBinding,
            sinks.play80034240,
            sinks.userData,
            cueTransaction)) {
        return false;
    }
    out.cue = cueTransaction.cue;
    out.sourceGlobalPointerAddress =
        cueTransaction.sourceGlobalPointerAddress;
    out.sourceCommandAddress = cueTransaction.sourceCommandAddress;
    out.command[0] = cueTransaction.command[0];
    out.command[1] = cueTransaction.command[1];
    out.command[2] = cueTransaction.command[2];
    out.command[3] = cueTransaction.command[3];
    out.driverState800943A8 = cueTransaction.driverState800943A8;
    out.playResult80034240 = cueTransaction.playResult80034240;
    out.sourceAccepted = cueTransaction.sourceAccepted;
    out.sourceMutated = cueTransaction.sourceMutated;
    out.voiceStored800943AC = cueTransaction.voiceStored800943AC;
    out.driverBindingKind = cueTransaction.driverBindingKind;
    out.exactPsxDriverHandleKnown =
        cueTransaction.exactPsxDriverHandleKnown;
    out.hostStage1VabProjection =
        cueTransaction.hostStage1VabProjection;
    out.exactPsxHalParity = false;
    // 80025C8C calls 80026ECC unconditionally after 80026EF8.
    out.flushResult80026ECC = sinks.flush80026ECC(sinks.userData);
    out.flushExecuted80026ECC = true;
    return true;
}

bool ExecuteInputSfxSoftware80025C8C(
    uint32_t inputMask,
    const CueSpec& sourceBinding,
    uint16_t driverState800943A8,
    const CueSoftwareSinks80025C8C& sinks,
    CueSoftwareTransaction80025C8C& out)
{
    CueDriverBinding800943A8 driverBinding{};
    driverBinding.kind = CueDriverBindingKind800943A8::ExactPsxHandle;
    driverBinding.value = driverState800943A8;
    return ExecuteInputSfxSoftware80025C8C(
        inputMask, sourceBinding, driverBinding, sinks, out);
}

ResourceAudioPlan BuildKnownIntManifestPlan(KnownArchive archive)
{
    ResourceAudioPlan plan = MakePlan();
    plan.hasOpenP0Gap = false;

    const uint32_t timBlockCount = KnownTimBlockCount(archive);
    for (uint32_t i = 0; i < timBlockCount; ++i) {
        const TimBlockReplayBaseline* baseline =
            FindTimBlockReplayBaseline(archive, i);
        if (baseline != nullptr) {
            AppendTimBlock(plan, *baseline, timBlockCount);
        }
    }

    const uint32_t vabCount = KnownVabRecordCount(archive);
    for (uint32_t i = 0; i < vabCount; ++i) {
        AppendVabRecord(plan, archive);
    }
    if (vabCount != 0) {
        AppendVabReplayBaseline(plan, archive);
    }

    const uint32_t memCount = KnownMemRecordCount(archive);
    if (memCount != 0) {
        AppendAction(plan, ResourceAudioActionKind::Call8001A8F0ParseMem,
                     kFn8001A8F0, archive, CueKind::Unknown, memCount);
        AppendAction(plan,
                     ResourceAudioActionKind::GateMemBlockHandleBase80025BFC,
                     kFn80025BFC,
                     archive,
                     CueKind::Unknown,
                     kResourceHandleTable80091858,
                     kCompo00MemHandleBase,
                     memCount,
                     kIntMemRecordType);
        AppendAction(plan,
                     ResourceAudioActionKind::GateMemBlockExplicitMap80025BBC,
                     kFn80025BBC,
                     archive,
                     CueKind::Unknown,
                     kResourceHandleTable80091858,
                     kCompo00MemHandleBase,
                     memCount,
                     kFn8001A8F0);
        AppendAction(plan, ResourceAudioActionKind::MemMapHandle,
                     kFn8001A8F0, archive, CueKind::Unknown, memCount,
                     kCompo00MemHandleBase);
        AppendMemReplayBaseline(plan, archive);
    }
    return plan;
}

ResourceAudioPlan BuildStartupPreload80016B84Plan()
{
    ResourceAudioPlan plan = MakePlan();
    plan.hasOpenP0Gap = true;

    AppendIntLoad(plan, KnownArchive::Common, kCommonIntDescriptor8005468C,
                  0);
    AppendPlan(plan, BuildKnownIntManifestPlan(KnownArchive::Common));
    AppendAction(plan, ResourceAudioActionKind::Call80027120VabClose,
                 kFn80027120, KnownArchive::Unknown);
    AppendVabCloseReplayBaseline(plan, KnownArchive::Unknown);
    AppendIntLoad(plan, KnownArchive::ZCompo, kZCompoIntDescriptor80054878,
                  0);
    AppendPlan(plan, BuildKnownIntManifestPlan(KnownArchive::ZCompo));
    AppendAction(plan, ResourceAudioActionKind::Call80026FA4InputAudioReset,
                 kFn80026FA4);
    AppendAction(plan,
                 ResourceAudioActionKind::InputAudioTailReplayBaseline,
                 kFn80026FA4,
                 KnownArchive::ZCompo,
                 CueKind::Unknown,
                 359,
                 2920);
    AppendAction(plan,
                 ResourceAudioActionKind::Call80015A4CStartupFadeLoop,
                 kFn80015A4C,
                 KnownArchive::Unknown,
                 CueKind::Unknown,
                 kFn8001E408,
                 kFn8001E54C,
                 kFn8001C0A0);
    AppendAction(plan,
                 ResourceAudioActionKind::Call80015B00StartupInputWait,
                 kFn80015B00,
                 KnownArchive::Unknown,
                 CueKind::Unknown,
                 150,
                 60);
    AppendAction(plan,
                 ResourceAudioActionKind::Call80015C20StartupFadeLoop,
                 kFn80015C20,
                 KnownArchive::Unknown,
                 CueKind::Unknown,
                 kFn8001E408,
                 kFn8001E54C);
    AppendAction(plan,
                 ResourceAudioActionKind::Call80015A4CStartupFadeLoop,
                 kFn80015A4C,
                 KnownArchive::Unknown,
                 CueKind::Unknown,
                 kFn8001E5A4,
                 kFn8001E6B0,
                 kFn8001C1B8);
    AppendAction(plan,
                 ResourceAudioActionKind::Call80015B00StartupInputWait,
                 kFn80015B00,
                 KnownArchive::Unknown,
                 CueKind::Unknown,
                 150,
                 60);
    AppendAction(plan,
                 ResourceAudioActionKind::Call80015C20StartupFadeLoop,
                 kFn80015C20,
                 KnownArchive::Unknown,
                 CueKind::Unknown,
                 kFn8001E5A4,
                 kFn8001E6B0);
    return plan;
}

ResourceAudioPlan BuildScene0Compo801C4780Plan()
{
    ResourceAudioPlan plan = MakePlan();
    plan.hasOpenP0Gap = false;

    AppendAction(plan, ResourceAudioActionKind::Call801C4780PreopenSegments,
                 kFn801C4780, KnownArchive::Compo00, CueKind::Unknown, 7);
    AppendAction(plan, ResourceAudioActionKind::Call80025A00LoaderPrepare,
                 kFn80025A00, KnownArchive::Compo00);
    AppendAction(plan,
                 ResourceAudioActionKind::GateResourceHandleTableReset80025A00,
                 kFn80025A00,
                 KnownArchive::Compo00,
                 CueKind::Unknown,
                 kResourceHandleTable80091858,
                 kResourceHandleTableSlots,
                 kResourcePoolBase800965B0,
                 kResourcePoolEnd801C35B0);
    AppendIntLoad(plan, KnownArchive::Compo00, 1, 0);
    AppendPlan(plan, BuildKnownIntManifestPlan(KnownArchive::Compo00));
    return plan;
}

ResourceAudioPlan BuildPracticeYCompo80015618Plan()
{
    ResourceAudioPlan plan = MakePlan();
    plan.hasOpenP0Gap = true;

    AppendIntLoad(plan, KnownArchive::YCompo, kYCompoIntDescriptor800546EC,
                  1);
    AppendPlan(plan, BuildKnownIntManifestPlan(KnownArchive::YCompo));
    AppendAction(plan,
                 ResourceAudioActionKind::InputAudioTailReplayBaseline,
                 kFn80026FA4,
                 KnownArchive::YCompo,
                 CueKind::Unknown,
                 9969,
                 0);
    AppendAction(plan,
                 ResourceAudioActionKind::InputAudioTailReplayBaseline,
                 kFn80026FA4,
                 KnownArchive::YCompo,
                 CueKind::Unknown,
                 9975,
                 0);
    AppendAction(plan, ResourceAudioActionKind::Gap, kFn8001AC18,
                 KnownArchive::YCompo, CueKind::Unknown,
                 kYCompoIntDescriptor800546EC);
    return plan;
}

ResourceAudioPlan BuildCue80026EF8Plan(CueKind cue)
{
    ResourceAudioPlan plan = MakePlan();
    plan.hasOpenP0Gap = cue == CueKind::Unknown;

    const CueSpec spec = KnownCueSpec(cue);
    if (cue == CueKind::Unknown) {
        AppendAction(plan, ResourceAudioActionKind::Gap, kFn80026EF8);
        return plan;
    }

    AppendAction(plan, ResourceAudioActionKind::GateCueSource80026EF8,
                 kFn80026EF8, KnownArchive::Unknown, cue,
                 spec.globalPointerAddress, spec.commandAddress,
                 spec.bytes[1], 1);
    AppendAction(plan, ResourceAudioActionKind::MutateCue80026EF8,
                 kFn80026EF8, KnownArchive::Unknown, cue,
                 spec.commandAddress, spec.bytes[1],
                 static_cast<uint32_t>(spec.bytes[1] + 0x18u));
    AppendAction(plan, ResourceAudioActionKind::PlayCue80034240,
                 kFn80034240, KnownArchive::Unknown, cue,
                 spec.bytes[0], spec.bytes[1],
                 static_cast<uint32_t>(spec.bytes[1] + 0x18u),
                 spec.bytes[3]);
    AppendAction(plan, ResourceAudioActionKind::StoreVoice800943AC,
                 kFn80026EF8, KnownArchive::Unknown, cue);
    return plan;
}

ResourceAudioPlan BuildInputSfx80025C8CPlan(uint32_t inputMask)
{
    ResourceAudioPlan plan = MakePlan();
    const CueKind cue = ResolveInputCue80025C8C(inputMask);
    plan.hasOpenP0Gap = cue == CueKind::Unknown;
    const CueSpec spec = KnownCueSpec(cue);

    AppendAction(plan, ResourceAudioActionKind::GateInputCueSource80025C8C,
                 kFn80025C8C, KnownArchive::Unknown, cue, inputMask,
                 spec.globalPointerAddress, spec.commandAddress,
                 cue == CueKind::Unknown ? 0u : 1u);
    if (cue == CueKind::Unknown) {
        AppendAction(plan, ResourceAudioActionKind::Gap, kFn80025C8C,
                     KnownArchive::Unknown, CueKind::Unknown, inputMask);
        return plan;
    }
    AppendAction(plan, ResourceAudioActionKind::ResolveCue80025C8C,
                 kFn80025C8C, KnownArchive::Unknown, cue, inputMask);
    AppendPlan(plan, BuildCue80026EF8Plan(cue));
    AppendAction(plan, ResourceAudioActionKind::Flush80026ECC,
                 kFn80026ECC, KnownArchive::Unknown, cue);
    return plan;
}

ResourceAudioPlan BuildSfxCallReplayBaselinePlan()
{
    ResourceAudioPlan plan = MakePlan();
    plan.hasOpenP0Gap = true;

    for (const SfxCallTotalReplayBaseline& baseline :
         kSfxCallTotalReplayBaselines) {
        AppendAction(plan,
                     ResourceAudioActionKind::SfxCallTotalReplayBaseline,
                     baseline.psxFunction,
                     KnownArchive::Unknown,
                     CueKind::Unknown,
                     baseline.frameCount,
                     baseline.callCount,
                     baseline.topCallsite,
                     baseline.topCallsiteCallCount);
    }

    for (const SfxCallAnchorReplayBaseline& baseline :
         kSfxCallAnchorReplayBaselines) {
        AppendAction(plan,
                     ResourceAudioActionKind::SfxCallAnchorReplayBaseline,
                     kFn80025C8C,
                     KnownArchive::Unknown,
                     CueKind::Unknown,
                     baseline.frame,
                     PackSfxAnchorCounts(baseline.inputCallCount,
                                         baseline.cueCallCount,
                                         baseline.flushCallCount,
                                         baseline.replaceCallCount,
                                         baseline.spuCallCount),
                     baseline.inputCallsite,
                     baseline.cueCallsite);
        AppendAction(plan,
                     ResourceAudioActionKind::SfxCallAnchorTailReplayBaseline,
                     kFn80026ECC,
                     KnownArchive::Unknown,
                     CueKind::Unknown,
                     baseline.frame,
                     baseline.primaryFlushCallsite,
                     baseline.secondaryFlushCallsite,
                     baseline.spuCallsite);
    }

    AppendAction(plan, ResourceAudioActionKind::Gap, kFn80034240,
                 KnownArchive::Unknown, CueKind::Unknown, kFn80026FC4);
    return plan;
}

ResourceAudioPlan BuildReplaceCue80026FC4Plan(CueKind cue)
{
    ResourceAudioPlan plan = MakePlan();
    plan.hasOpenP0Gap = true;

    const CueSpec spec = KnownCueSpec(cue);
    AppendAction(plan, ResourceAudioActionKind::GateReplaceCueSource80026FC4,
                 kFn80026FC4, KnownArchive::Unknown, cue,
                 spec.globalPointerAddress, spec.commandAddress,
                 cue == CueKind::Unknown ? 0u : 1u);
    if (cue == CueKind::Unknown) {
        AppendAction(plan, ResourceAudioActionKind::Gap, kFn80026FC4,
                     KnownArchive::Unknown, CueKind::Unknown);
        return plan;
    }
    AppendAction(plan, ResourceAudioActionKind::ReplaceCue80026FC4,
                 kFn80026FC4, KnownArchive::Unknown, cue,
                 spec.commandAddress);
    AppendAction(plan, ResourceAudioActionKind::StoreVoice800943AA,
                 kFn80026FC4, KnownArchive::Unknown, cue);
    AppendAction(plan, ResourceAudioActionKind::CopyCueState800943AE,
                 kFn80026FC4, KnownArchive::Unknown, cue, 6);
    AppendAction(plan, ResourceAudioActionKind::Gap, kFn80026FC4,
                 KnownArchive::Unknown, cue);
    return plan;
}

const char* KnownArchiveName(KnownArchive archive)
{
    switch (archive) {
    case KnownArchive::Unknown:
        return "Unknown";
    case KnownArchive::Common:
        return "Common";
    case KnownArchive::ZCompo:
        return "ZCompo";
    case KnownArchive::Compo00:
        return "Compo00";
    case KnownArchive::YCompo:
        return "YCompo";
    }
    return "Unknown";
}

const char* CueKindName(CueKind cue)
{
    switch (cue) {
    case CueKind::Unknown:
        return "Unknown";
    case CueKind::SceneGlobal94410:
        return "SceneGlobal94410";
    case CueKind::Direction94420:
        return "Direction94420";
    case CueKind::Cross94424:
        return "Cross94424";
    case CueKind::Circle94428:
        return "Circle94428";
    case CueKind::Select9442C:
        return "Select9442C";
    case CueKind::Stage1Event4Primary9441C:
        return "Stage1Event4Primary9441C";
    case CueKind::Stage1Event4Secondary9441CPlus6:
        return "Stage1Event4Secondary9441CPlus6";
    }
    return "Unknown";
}

const char* ResourceAudioActionKindName(ResourceAudioActionKind kind)
{
    switch (kind) {
    case ResourceAudioActionKind::None:
        return "None";
    case ResourceAudioActionKind::GateIntLoadRetryAndReset8001AC18:
        return "GateIntLoadRetryAndReset8001AC18";
    case ResourceAudioActionKind::Call8001AC18LoadInt:
        return "Call8001AC18LoadInt";
    case ResourceAudioActionKind::GateResourceHandleTableReset80025A00:
        return "GateResourceHandleTableReset80025A00";
    case ResourceAudioActionKind::Call8001A8F0ParseTim:
        return "Call8001A8F0ParseTim";
    case ResourceAudioActionKind::TimUpload:
        return "TimUpload";
    case ResourceAudioActionKind::Call8001A8F0ParseVab:
        return "Call8001A8F0ParseVab";
    case ResourceAudioActionKind::Call80027120VabClose:
        return "Call80027120VabClose";
    case ResourceAudioActionKind::PadStartCom:
        return "PadStartCom";
    case ResourceAudioActionKind::Call80027078VabOpen:
        return "Call80027078VabOpen";
    case ResourceAudioActionKind::Call800270D4VabTransfer:
        return "Call800270D4VabTransfer";
    case ResourceAudioActionKind::Call800270FCVabEnable:
        return "Call800270FCVabEnable";
    case ResourceAudioActionKind::Call8001A8F0ParseMem:
        return "Call8001A8F0ParseMem";
    case ResourceAudioActionKind::GateMemBlockHandleBase80025BFC:
        return "GateMemBlockHandleBase80025BFC";
    case ResourceAudioActionKind::GateMemBlockExplicitMap80025BBC:
        return "GateMemBlockExplicitMap80025BBC";
    case ResourceAudioActionKind::MemMapHandle:
        return "MemMapHandle";
    case ResourceAudioActionKind::Call80026FA4InputAudioReset:
        return "Call80026FA4InputAudioReset";
    case ResourceAudioActionKind::Call801C4780PreopenSegments:
        return "Call801C4780PreopenSegments";
    case ResourceAudioActionKind::Call80025A00LoaderPrepare:
        return "Call80025A00LoaderPrepare";
    case ResourceAudioActionKind::TimUploadReplayBaseline:
        return "TimUploadReplayBaseline";
    case ResourceAudioActionKind::VabLoadReplayBaseline:
        return "VabLoadReplayBaseline";
    case ResourceAudioActionKind::VabCloseReplayBaseline:
        return "VabCloseReplayBaseline";
    case ResourceAudioActionKind::MemMapReplayBaseline:
        return "MemMapReplayBaseline";
    case ResourceAudioActionKind::InputAudioTailReplayBaseline:
        return "InputAudioTailReplayBaseline";
    case ResourceAudioActionKind::GateInputCueSource80025C8C:
        return "GateInputCueSource80025C8C";
    case ResourceAudioActionKind::ResolveCue80025C8C:
        return "ResolveCue80025C8C";
    case ResourceAudioActionKind::GateCueSource80026EF8:
        return "GateCueSource80026EF8";
    case ResourceAudioActionKind::MutateCue80026EF8:
        return "MutateCue80026EF8";
    case ResourceAudioActionKind::PlayCue80034240:
        return "PlayCue80034240";
    case ResourceAudioActionKind::StoreVoice800943AC:
        return "StoreVoice800943AC";
    case ResourceAudioActionKind::Flush80026ECC:
        return "Flush80026ECC";
    case ResourceAudioActionKind::SfxCallTotalReplayBaseline:
        return "SfxCallTotalReplayBaseline";
    case ResourceAudioActionKind::SfxCallAnchorReplayBaseline:
        return "SfxCallAnchorReplayBaseline";
    case ResourceAudioActionKind::SfxCallAnchorTailReplayBaseline:
        return "SfxCallAnchorTailReplayBaseline";
    case ResourceAudioActionKind::GateReplaceCueSource80026FC4:
        return "GateReplaceCueSource80026FC4";
    case ResourceAudioActionKind::ReplaceCue80026FC4:
        return "ReplaceCue80026FC4";
    case ResourceAudioActionKind::StoreVoice800943AA:
        return "StoreVoice800943AA";
    case ResourceAudioActionKind::CopyCueState800943AE:
        return "CopyCueState800943AE";
    case ResourceAudioActionKind::Gap:
        return "Gap";
    }
    return "Unknown";
}

} // namespace PrSS0ResourceAudioDirect
