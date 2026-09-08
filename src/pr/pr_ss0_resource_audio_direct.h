#pragma once

#include <cstdint>

namespace PrSS0ResourceAudioDirect {

static constexpr uint32_t kFn80016B84 = 0x80016B84u;
static constexpr uint32_t kFn80016C8C = 0x80016C8Cu;
static constexpr uint32_t kFn80015A4C = 0x80015A4Cu;
static constexpr uint32_t kFn80015B00 = 0x80015B00u;
static constexpr uint32_t kFn80015C20 = 0x80015C20u;
static constexpr uint32_t kFn8001E408 = 0x8001E408u;
static constexpr uint32_t kFn8001E54C = 0x8001E54Cu;
static constexpr uint32_t kFn8001C0A0 = 0x8001C0A0u;
static constexpr uint32_t kFn8001E5A4 = 0x8001E5A4u;
static constexpr uint32_t kFn8001E6B0 = 0x8001E6B0u;
static constexpr uint32_t kFn8001C1B8 = 0x8001C1B8u;
static constexpr uint32_t kFn8001AC18 = 0x8001AC18u;
static constexpr uint32_t kFn8001A8F0 = 0x8001A8F0u;
static constexpr uint32_t kFn80027120 = 0x80027120u;
static constexpr uint32_t kFn80027078 = 0x80027078u;
static constexpr uint32_t kFn800270D4 = 0x800270D4u;
static constexpr uint32_t kFn800270FC = 0x800270FCu;
static constexpr uint32_t kFn80026FA4 = 0x80026FA4u;
static constexpr uint32_t kFn801C4780 = 0x801C4780u;
static constexpr uint32_t kFn80025A00 = 0x80025A00u;
static constexpr uint32_t kFn80025A34 = 0x80025A34u;
static constexpr uint32_t kFn80025A70 = 0x80025A70u;
static constexpr uint32_t kFn80025BFC = 0x80025BFCu;
static constexpr uint32_t kFn80025BBC = 0x80025BBCu;
static constexpr uint32_t kFn80025C8C = 0x80025C8Cu;
static constexpr uint32_t kFn80026EF8 = 0x80026EF8u;
static constexpr uint32_t kFn80026ECC = 0x80026ECCu;
static constexpr uint32_t kFn80026FC4 = 0x80026FC4u;
static constexpr uint32_t kFn80034240 = 0x80034240u;

static constexpr uint32_t kResourceHandleTable80091858 = 0x80091858u;
static constexpr uint32_t kResourceHandleTableSlots = 1024u;
static constexpr uint32_t kResourcePoolBase800965B0 = 0x800965B0u;
static constexpr uint32_t kResourcePoolEnd801C35B0 = 0x801C35B0u;
static constexpr uint32_t kResourceAllocAlignBytes = 8u;
static constexpr uint32_t kIntLoadRetryCount8001AC18 = 4u;
static constexpr uint32_t kIntMemRecordType = 3u;
static constexpr uint32_t kCompo00MemHandleBase = 2u;
static constexpr uint32_t kCommonIntDescriptor8005468C = 0x8005468Cu;
static constexpr uint32_t kZCompoIntDescriptor80054878 = 0x80054878u;
static constexpr uint32_t kYCompoIntDescriptor800546EC = 0x800546ECu;
static constexpr uint32_t kCueGlobal94410 = 0x80094410u;
static constexpr uint32_t kCueGlobal9441C = 0x8009441Cu;
static constexpr uint32_t kCueGlobal94420 = 0x80094420u;
static constexpr uint32_t kCueGlobal94424 = 0x80094424u;
static constexpr uint32_t kCueGlobal94428 = 0x80094428u;
static constexpr uint32_t kCueGlobal9442C = 0x8009442Cu;
static constexpr uint32_t kCueData8006EA84 = 0x8006EA84u;
static constexpr uint32_t kCueData8006EA9C = 0x8006EA9Cu;
static constexpr uint32_t kCueData8006EAA4 = 0x8006EAA4u;
static constexpr uint32_t kCueData8006EAAC = 0x8006EAACu;
static constexpr uint32_t kCueData8006EAB4 = 0x8006EAB4u;
static constexpr uint32_t kStage1Event4CueData801CCC68 = 0x801CCC68u;
static constexpr uint32_t kStage1Event4DirectionData801CCC74 = 0x801CCC74u;
static constexpr uint32_t kStage1Event4CrossData801CCC7C = 0x801CCC7Cu;
static constexpr uint32_t kStage1Event4CircleData801CCC84 = 0x801CCC84u;
static constexpr uint32_t kStage1Event4SelectData801CCC8C = 0x801CCC8Cu;

enum class KnownArchive : uint8_t {
    Unknown = 0,
    Common,
    ZCompo,
    Compo00,
    YCompo,
};

enum class CueKind : uint8_t {
    Unknown = 0,
    SceneGlobal94410,
    Direction94420,
    Cross94424,
    Circle94428,
    Select9442C,
    Stage1Event4Primary9441C,
    Stage1Event4Secondary9441CPlus6,
};

enum class CueDriverBindingKind800943A8 : uint8_t {
    Unknown = 0,
    ExactPsxHandle,
    HostStage1VabProjection,
};

struct CueDriverBinding800943A8 {
    CueDriverBindingKind800943A8 kind =
        CueDriverBindingKind800943A8::Unknown;
    uint16_t value = 0u;
    bool hostStage1VabReady = false;
};

enum class ResourceAudioActionKind : uint8_t {
    None = 0,
    GateIntLoadRetryAndReset8001AC18,
    Call8001AC18LoadInt,
    GateResourceHandleTableReset80025A00,
    Call8001A8F0ParseTim,
    TimUpload,
    Call8001A8F0ParseVab,
    Call80027120VabClose,
    PadStartCom,
    Call80027078VabOpen,
    Call800270D4VabTransfer,
    Call800270FCVabEnable,
    Call8001A8F0ParseMem,
    GateMemBlockHandleBase80025BFC,
    GateMemBlockExplicitMap80025BBC,
    MemMapHandle,
    Call80026FA4InputAudioReset,
    Call80015A4CStartupFadeLoop,
    Call80015B00StartupInputWait,
    Call80015C20StartupFadeLoop,
    Call801C4780PreopenSegments,
    Call80025A00LoaderPrepare,
    TimUploadReplayBaseline,
    VabLoadReplayBaseline,
    VabCloseReplayBaseline,
    MemMapReplayBaseline,
    InputAudioTailReplayBaseline,
    GateInputCueSource80025C8C,
    ResolveCue80025C8C,
    GateCueSource80026EF8,
    MutateCue80026EF8,
    PlayCue80034240,
    StoreVoice800943AC,
    Flush80026ECC,
    SfxCallTotalReplayBaseline,
    SfxCallAnchorReplayBaseline,
    SfxCallAnchorTailReplayBaseline,
    GateReplaceCueSource80026FC4,
    ReplaceCue80026FC4,
    StoreVoice800943AA,
    CopyCueState800943AE,
    Gap,
};

struct CueSpec {
    CueKind kind = CueKind::Unknown;
    uint32_t globalPointerAddress = 0;
    uint32_t commandAddress = 0;
    uint8_t bytes[8]{};
};

// Software-only representation of the proven 80025C8C -> 80026EF8
// transaction. The two sinks are explicit HAL boundaries; this type does not
// infer PSX SPU state from replay memory.
struct CueSoftwareTransaction80025C8C {
    uint32_t inputMask = 0;
    CueKind cue = CueKind::Unknown;
    uint32_t sourceGlobalPointerAddress = 0;
    uint32_t sourceCommandAddress = 0;
    uint8_t command[4]{};
    uint16_t driverState800943A8 = 0;
    int16_t playResult80034240 = -1;
    int32_t flushResult80026ECC = 0;
    bool sourceAccepted = false;
    bool sourceMutated = false;
    bool voiceStored800943AC = false;
    bool flushExecuted80026ECC = false;
    CueDriverBindingKind800943A8 driverBindingKind =
        CueDriverBindingKind800943A8::Unknown;
    bool exactPsxDriverHandleKnown = false;
    bool hostStage1VabProjection = false;
    bool exactPsxHalParity = false;
};

struct CueSoftwareTransaction80026EF8 {
    CueKind cue = CueKind::Unknown;
    uint32_t sourceGlobalPointerAddress = 0u;
    uint32_t sourceCommandAddress = 0u;
    uint8_t command[4]{};
    uint16_t driverState800943A8 = 0u;
    int16_t playResult80034240 = -1;
    bool sourceAccepted = false;
    bool sourceMutated = false;
    bool voiceStored800943AC = false;
    CueDriverBindingKind800943A8 driverBindingKind =
        CueDriverBindingKind800943A8::Unknown;
    bool exactPsxDriverHandleKnown = false;
    bool hostStage1VabProjection = false;
    bool exactPsxHalParity = false;
};

using CuePlaySink80034240 = int16_t (*)(
    uint16_t driverState800943A8,
    uint8_t byte0,
    uint8_t byte1,
    uint8_t byte2,
    uint16_t arg4,
    uint8_t byte3Left,
    uint8_t byte3Right,
    void* userData);
using CueFlushSink80026ECC = int32_t (*)(void* userData);

struct CueSoftwareSinks80025C8C {
    CuePlaySink80034240 play80034240 = nullptr;
    CueFlushSink80026ECC flush80026ECC = nullptr;
    void* userData = nullptr;
};

struct ResourceAudioAction {
    ResourceAudioActionKind kind = ResourceAudioActionKind::None;
    uint32_t psxFunction = 0;
    KnownArchive archive = KnownArchive::Unknown;
    CueKind cue = CueKind::Unknown;
    uint32_t arg0 = 0;
    uint32_t arg1 = 0;
    uint32_t arg2 = 0;
    uint32_t arg3 = 0;
};

struct ResourceAudioPlan {
    bool runtimeCutoverAllowed = false;
    bool hasOpenP0Gap = true;
    ResourceAudioAction actions[64]{};
    uint32_t count = 0;
    bool truncated = false;
};

// Exact software pointer-table publication performed by SCUS 80016C8C
// immediately before 80016B84.  These are PSX address carriers, never host
// pointers and never evidence of physical SPU/register state.
struct StartupAudioBindings80016C8C {
    bool known = false;
    bool complete = false;
    uint32_t sourceFunction = 0u;
    uint32_t slot800943FC = 0u;
    uint32_t slot80094400 = 0u;
    uint32_t slot80094404 = 0u;
    uint32_t slot80094408 = 0u;
    uint32_t slot8009440C = 0u;
    uint32_t slot80094410 = 0u;
    uint32_t slot80094414 = 0u;
    uint32_t slot80094418 = 0u;
    uint32_t slot8009441C = 0u;
    uint32_t slot80094420 = 0u;
    uint32_t slot80094424 = 0u;
    uint32_t slot80094428 = 0u;
    uint32_t slot8009442C = 0u;
    bool returnValueKnown = false;
    uint32_t returnValue = 0u;
    bool directSoftwareStateAuthority = false;
    bool psxMemoryBackingAuthority = false;
    bool physicalSpuAuthority = false;
    bool hostProjection = false;
    bool replayValueAuthority = false;
    bool oldWinS0Authority = false;
    bool stage2PlusAuthority = false;
};

bool RuntimeCutoverAllowed();

StartupAudioBindings80016C8C BuildStartupAudioBindings80016C8C();
bool IsExactStartupAudioBindings80016C8C(
    const StartupAudioBindings80016C8C& bindings);

uint32_t KnownTimRecordCount(KnownArchive archive);
uint32_t KnownTimBlockCount(KnownArchive archive);
uint32_t KnownTimBlockRecordCount(KnownArchive archive, uint32_t blockIndex);
uint32_t KnownVabRecordCount(KnownArchive archive);
uint32_t KnownMemRecordCount(KnownArchive archive);
CueSpec KnownCueSpec(CueKind cue);
CueSpec KnownStage1Event4CueSpec(CueKind cue);
CueKind ResolveInputCue80025C8C(uint32_t inputMask);
bool ExecuteCueSoftware80026EF8(
    const CueSpec& sourceBinding,
    const CueDriverBinding800943A8& driverBinding,
    CuePlaySink80034240 play80034240,
    void* userData,
    CueSoftwareTransaction80026EF8& out);
bool ExecuteInputSfxSoftware80025C8C(
    uint32_t inputMask,
    const CueSpec& sourceBinding,
    const CueDriverBinding800943A8& driverBinding,
    const CueSoftwareSinks80025C8C& sinks,
    CueSoftwareTransaction80025C8C& out);
bool ExecuteInputSfxSoftware80025C8C(
    uint32_t inputMask,
    const CueSpec& sourceBinding,
    uint16_t driverState800943A8,
    const CueSoftwareSinks80025C8C& sinks,
    CueSoftwareTransaction80025C8C& out);

ResourceAudioPlan BuildKnownIntManifestPlan(KnownArchive archive);
ResourceAudioPlan BuildStartupPreload80016B84Plan();
ResourceAudioPlan BuildScene0Compo801C4780Plan();
ResourceAudioPlan BuildPracticeYCompo80015618Plan();
ResourceAudioPlan BuildCue80026EF8Plan(CueKind cue);
ResourceAudioPlan BuildInputSfx80025C8CPlan(uint32_t inputMask);
ResourceAudioPlan BuildSfxCallReplayBaselinePlan();
ResourceAudioPlan BuildReplaceCue80026FC4Plan(CueKind cue);

const char* KnownArchiveName(KnownArchive archive);
const char* CueKindName(CueKind cue);
const char* ResourceAudioActionKindName(ResourceAudioActionKind kind);

} // namespace PrSS0ResourceAudioDirect
