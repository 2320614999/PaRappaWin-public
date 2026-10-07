#pragma once

#include "pr_ss0_title_packet_work_direct.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <array>
#include <memory>
#include <string>

class ResourceManager;
class D3D11Renderer;
struct DatData;
struct ID3D11ShaderResourceView;
struct TmdObject;
struct TodData;
struct TodCoordMatrix;
struct VdfData;
struct PrStage1LoaderMemoryDirectState;
class PsxVramAtlas;

namespace PrSS0Scene0IntLoadDirect {
struct Transaction8001AC18;
}

namespace PrSS0Scene0IntRendererDirect {
struct Transaction8001AE7C;
}

namespace PrSS0Scene0IntSideEffectDirect {
struct Transaction8001A8F0;
}

namespace PrSS0Scene0IntGpuDirect {
struct Transaction8001AE7C;
}

namespace PrSS0TitleTransformDirect {
struct Mode25BaseTriangleCarrier;
struct Mode25DeformedTriangleCarrier;
struct Mode25Rtpt3Input;
}

namespace PrSS0TitleHudEventsDirect {
struct TitleEventEffect801C5190;
struct TitleHudTimRequest801C5094;
}

namespace PrPsxGraphOwnerDirect {
struct PsxGraphState;
}

namespace PrSS0TransitionDirect {
struct FastTransitionVisualFrame800201AC;
struct SlowTransitionVisualFrame80020110;
}

namespace PrPsxGteDirect {
struct Matrix3x4;
struct Mode25TriangleGeometryTrace;
struct Rtpt3ExactOutput280030;
}

namespace PrSS0TitlePacketWorkDirect {
struct RuntimeState801C609C;
}

namespace PrSS0TitleDrawDescDirect {
struct RuntimeState8001AF1C;
}

namespace PrSS0TitlePacketPlanDirect {
struct BuildResult800428B0;
}

namespace PrSS0TitlePacketCommitDirect {
struct CommitResult800428B0;
}

namespace PrSS0TitlePrimitiveGroupDirect {
struct ExecuteResult8004274C;
}

namespace PrSS0TitleTmdBackend {

enum class ModelKind {
    Hp,
    Lo,
    Pa,
    PaKage,
    Count,
};

enum class MimeKind {
    Hiphop,
    Logo,
    LogoNew,
    PaDance,
    PaLeft,
    PaOki,
    PaRight,
    PaTurnLeft,
    PaTurnRight,
    Count,
};

enum class TitleTodKind {
    PaDance,
    PaLoc,
    PaLoc2,
    Count,
};

constexpr uint32_t kFn8001385C_TitleMimeChannelInit = 0x8001385Cu;
constexpr uint32_t kFn800139F8_TitleMimeBaseRestore = 0x800139F8u;
constexpr uint32_t kFn80013E40_TitleMimeScratchClear = 0x80013E40u;
constexpr uint32_t kFn8001EEE8_TitleTransitionGridInit = 0x8001EEE8u;
constexpr uint32_t kTitleMimeChannelStateBase80074B08 = 0x80074B08u;
constexpr uint32_t kTitleMimeChannelStateStride8001385C = 12u;
constexpr uint32_t kTitleMimeBaseSnapshotBank8006F6A8 = 0x8006F6A8u;
constexpr uint32_t kTitleMimeBaseSnapshotStride8001385C = 112u;
constexpr uint32_t kTitleMimeVdfStateBase80074B80 = 0x80074B80u;
constexpr uint32_t kTitleMimeVdfWorkspaceBase8006FB08 = 0x8006FB08u;
constexpr uint32_t kTitleMimeVdfWorkspaceStride8001385C = 2048u;
constexpr uint32_t kTitleMimeScratchBase80090240 = 0x80090240u;
constexpr uint32_t kTitleMimeScratchStride80013E40 = 512u;
constexpr std::size_t kTitleMimeScratchSlotCount80013E40 = 10u;
constexpr std::size_t kTitleMimeScratchBytes80013E40 =
    kTitleMimeScratchSlotCount80013E40 *
    kTitleMimeScratchStride80013E40;
constexpr std::size_t kTitleTransitionGridCells8001EEE8 = 192u;

struct TitleMimeChannelInit8001385C {
    bool known = false;
    uint8_t channel = 0u;
    ModelKind modelKind = ModelKind::Count;
    MimeKind mimeKind = MimeKind::Count;
    uint32_t channelStateAddress80074B08 = 0u;
    uint32_t baseSnapshotAddress8006F6A8 = 0u;
    uint32_t vdfStateAddress80074B80 = 0u;
    uint32_t vdfWorkspaceAddress8006FB08 = 0u;
    uint32_t scratchAddress80090240 = 0u;
    uint32_t destinationVertexAddressA4 = 0u;
    uint32_t modelObjectCount = 0u;
    uint32_t modelVertexCount = 0u;
    uint32_t vdfKeyCount = 0u;
    uint32_t datKeyCount = 0u;
    bool tmdBound80013650 = false;
    bool baseSnapshotCommitted8001371C = false;
    bool vdfBound800137BC = false;
};

struct TitleMimeInitState801C609C {
    bool known = false;
    bool exactCallOrder = false;
    bool baseRestoreCalled800139F8 = false;
    uint32_t baseRestoreCallCount800139F8 = 0u;
    std::array<bool, 4> baseChannelRestored800139F8{};
    uint32_t channelInitCallCount8001385C = 0u;
    std::array<TitleMimeChannelInit8001385C, 4> channels{};
    bool scratchClearCalled80013E40 = false;
    uint32_t scratchZeroByteCount80013E40 = 0u;
    std::array<uint8_t, kTitleMimeScratchBytes80013E40> scratch80090240{};
    bool transitionGridInitCalled8001EEE8 = false;
    uint32_t gp49 = 0u;
    uint32_t gp50 = 0u;
    uint32_t gp51 = 0u;
    std::array<uint8_t, kTitleTransitionGridCells8001EEE8>
        transitionGrid8001EEAC{};
};

inline TitleTodKind TitleTodKindFromHandle801C5190(uint8_t handle)
{
    switch (handle) {
    case 2u:
        return TitleTodKind::PaDance;
    case 3u:
        return TitleTodKind::PaLoc;
    case 4u:
        return TitleTodKind::PaLoc2;
    default:
        return TitleTodKind::Count;
    }
}

inline MimeKind MimeKindFromResourcePairIndex801C6C14(
    uint8_t pairIndex)
{
    switch (pairIndex) {
    case 1u:
        return MimeKind::Hiphop;
    case 2u:
        return MimeKind::LogoNew;
    case 3u:
        return MimeKind::PaDance;
    case 4u:
        return MimeKind::PaLeft;
    case 5u:
        return MimeKind::PaOki;
    case 6u:
        return MimeKind::PaRight;
    case 7u:
        return MimeKind::PaTurnLeft;
    case 8u:
        return MimeKind::PaTurnRight;
    default:
        return MimeKind::Count;
    }
}

struct ModelResourceView {
    bool loaded = false;
    uint32_t objectCount = 0;
    uint32_t primitiveCount = 0;
    uint32_t texturedPrimitiveCount = 0;
};

struct MimeResourceView {
    bool pairLoaded = false;
    uint32_t vdfKeyCount = 0;
    uint32_t datKeyCount = 0;
    uint16_t datMaxFrames = 0;
};

struct TitleTodResourceView {
    bool loaded = false;
    uint8_t handle = 0;
    uint32_t blockCount = 0;
    uint32_t rawByteCount = 0;
};

struct RuntimePrimaryPaMimeBinding80014164 {
    bool known = false;
    bool initialBinding801C609C = false;
    bool earlyInputShortcutSetup801C64BC = false;
    uint32_t sourceEventIndex = 0;
    uint32_t appliedTick96 = 0;
    uint8_t resourcePairIndex = 0;
    MimeKind mimeKind = MimeKind::Count;
    bool cursorKnown = false;
    uint32_t cursor = 0;
    bool endKnown = false;
    uint32_t end = 0;
    bool loopFlagKnown = false;
    uint16_t loopFlag = 0;
};

struct RuntimePrimaryPaMimeAdvanceResult800141D8 {
    bool accepted = false;
    bool sourceExhausted = false;
    uint32_t rawCursor = 0;
    uint16_t sampleFrame = 0;
};

struct RuntimeChannel2MimeBinding801C6410 {
    bool known = false;
    bool earlyInputShortcutSetup801C64BC = false;
    uint32_t sourceEventIndex = 0;
    uint32_t appliedTick96 = 0;
    uint8_t resourcePairIndex = 0;
    MimeKind mimeKind = MimeKind::Count;
    bool cursorKnown = false;
    uint32_t cursor = 0;
};

struct PreparedRuntimeChannel2MimeBinding801C6410 {
    bool accepted = false;
    // 801C6410 ctxFlags&0x00010000 executes 800140E0 for visible PA and
    // 80014050(group=2) for PA_KAGE from the same DAT/VDF pair.  Keep the
    // two bindings in one prepared transaction so neither model can retain
    // the previous action when the other switches.
    RuntimePrimaryPaMimeBinding80014164 primaryBinding{};
    RuntimeChannel2MimeBinding801C6410 binding{};
};

struct RuntimeChannel1MimeBinding801C6410 {
    bool known = false;
    bool earlyInputShortcutSetup801C6530 = false;
    uint32_t sourceEventIndex = 0;
    uint32_t thresholdTick96 = 0;
    uint32_t appliedTick96 = 0;
    uint8_t resourcePairIndex = 0;
    MimeKind mimeKind = MimeKind::Count;
    bool cursorKnown = false;
    uint32_t cursor = 0;
};

struct RuntimeChannel3MimeBinding801C6410 {
    bool known = false;
    bool earlyInputShortcutSetup801C6578 = false;
    uint32_t sourceEventIndex = 0;
    uint32_t thresholdTick96 = 0;
    uint32_t appliedTick96 = 0;
    uint8_t resourcePairIndex = 0;
    MimeKind mimeKind = MimeKind::Count;
    bool cursorKnown = false;
    uint32_t cursor = 0;
};

struct RuntimeChannel2MimeAdvanceResult80013EA8 {
    bool accepted = false;
    uint32_t rawCursor = 0;
    uint16_t sampleFrame = 0;
};

struct RuntimeChannel1MimeAdvanceResult80013EA8 {
    bool accepted = false;
    uint32_t cursorAfter = 0;
    uint16_t sampleFrame = 0;
};

struct RuntimeChannel3MimeAdvanceResult80013EA8 {
    bool accepted = false;
    uint32_t rawCursor = 0;
    uint16_t sampleFrame = 0;
};

struct RuntimeTitleTodBinding801C6410 {
    bool known = false;
    bool initialBinding801C609C = false;
    bool earlyInputShortcutReady801C5AB4 = false;
    uint32_t sourceEventIndex = 0;
    uint32_t appliedTick96 = 0;
    uint8_t resourceHandle = 0;
    TitleTodKind todKind = TitleTodKind::Count;
    bool cursorKnown = false;
    uint32_t sampleSeq = 0;
    uint32_t blockIndex = 0;
    bool drawCoordWorldKnown800417A4 = false;
};

struct RuntimeTitleTodAdvanceResult8001B000 {
    bool accepted = false;
    bool blockAdvanced = false;
    bool sourceExhausted = false;
    uint32_t sampleSeq = 0;
    uint32_t blockIndex = 0;
    bool drawCoordWorldKnown800417A4 = false;
};

struct ImmutableScene0ComodView {
    const uint8_t* data = nullptr;
    std::size_t size = 0;
};

struct RuntimeTitleCameraBinding801C6410 {
    bool known = false;
    bool initialBinding801C609C = false;
    bool objectResource104Known = false;
    uint8_t objectResource104Index = 0;
    uint32_t sourceEventIndex = 0;
    uint32_t appliedTick96 = 0;
    bool cursorKnown = false;
    uint32_t cursor = 0;
};

struct RuntimeTitleCameraAdvanceResult80041D3C {
    bool accepted = false;
    bool sourceExhausted = false;
    bool terminalStateHeld = false;
    uint32_t sampledIndex = 0;
    uint32_t nextCursor = 0;
    bool cameraMatrixKnown80092880 = false;
};

static constexpr std::size_t kTitleHudTimRequestCapacity801C5094 = 4u;

enum class TitleHudTimApplyStatus801C5094 : uint8_t {
    InvalidRequest = 0,
    BackendNotReady,
    ResourceManagerMissing,
    ResourceManagerMismatch,
    ResourceGenerationMismatch,
    MissingZeroTerminator,
    EmptyRequest,
    TimIdCountMismatch,
    NonZeroAfterTerminator,
    TimHandleOutOfRange,
    MemHandleOutOfRange,
    MemNameMismatch,
    MemNameAmbiguous,
    MemBytesUnavailable,
    TimMetadataMismatch,
    AtlasPreflightRejected,
    CounterOverflow,
    CommitFailedResourcesInvalidated,
    Accepted,
};

struct TitleHudTimResolution801C5094 {
    int16_t handle = 0;
    bool memRecordResolved = false;
    bool expectedNameMatched = false;
    bool rawBytesResolved = false;
    bool metadataAccepted = false;
    bool preflightAccepted = false;
    bool applied = false;
    std::string expectedName{};
    std::string resolvedMemName{};
    std::size_t rawByteCount = 0u;
};

struct TitleHudTimApplyResult801C5094 {
    TitleHudTimApplyStatus801C5094 status =
        TitleHudTimApplyStatus801C5094::InvalidRequest;
    bool accepted = false;
    bool zeroTerminated = false;
    uint32_t resourceGeneration = 0u;
    uint32_t requestedCount = 0u;
    uint32_t appliedCount = 0u;
    int16_t requestedIds[kTitleHudTimRequestCapacity801C5094]{};
    TitleHudTimResolution801C5094
        resolutions[kTitleHudTimRequestCapacity801C5094]{};
};

enum class TitlePresentModelStatus801C689C : uint8_t {
    NotReady = 0,
    CounterOverflow,
    FrameRejected,
    ModelApplied,
};

struct TitlePresentModelResult801C689C {
    TitlePresentModelStatus801C689C status =
        TitlePresentModelStatus801C689C::NotReady;
    bool modelApplied = false;
    uint8_t cachedDrawLane8006EDA8 = 0;
    uint8_t nextDrawLane80096590 = 0;
    uint32_t titleWorkAddress801C9574 = 0;
    uint32_t mainPageWorkAddress80087288 = 0;
    bool whiteDrawEnv80040060Requested = false;
    bool titleWork80040CA4SubmitRequested = false;
    bool mainPageWork8001E3B0SubmitRequested = false;
    bool presentExtraSource801CFAEC = false;
    bool extraFlipGraphStateAdvanced80040370 = false;
    bool fullHeightWhiteFill8001B1B0Requested = false;
    int16_t fullHeightWhiteFillX8001B1B0 = 0;
    int16_t fullHeightWhiteFillY8001B1B0 = 0;
    int16_t fullHeightWhiteFillWidth8001B1B0 = 0;
    int16_t fullHeightWhiteFillHeight8001B1B0 = 0;
    bool titleWorkSubmit80040CA4Executed = false;
    bool mainPageWorkSubmit8001E3B0Executed = false;
    uint32_t titleWorkSubmitCallCount80040CA4 = 0u;
    uint32_t mainPageWorkSubmitCallCount8001E3B0 = 0u;
    PrSS0TitlePacketWorkDirect::TitleDrawEnvState80040060
        drawEnv80040060{};
};

struct TitleMovie0TState0GraphFlipResult801C4B8C {
    bool committed = false;
    uint16_t drawSlotBefore = 0;
    uint16_t drawSlotAfter = 0;
    uint32_t frameCounterBefore8009658C = 0;
    uint32_t frameCounterAfter8009658C = 0;
};

struct OpeningMovie0WorkListFlushResult8001ED3C {
    bool committed = false;
    uint32_t sourceFunction = 0x8001ED3Cu;
    bool cachedWorkSlotKnown8006EDA8 = false;
    uint16_t cachedWorkSlot8006EDA8 = 0u;
    uint32_t mainPageWorkAddress80087288 = 0u;
    uint32_t mainPageOtHeadAddress80088288 = 0u;
    bool submitCalled80040CA4 = false;
    bool softwareStateCommitted = false;
    uint32_t callCount80040CA4 = 0u;
    bool currentScusSemanticAuthority = false;
    bool currentComod0CallerAuthority = false;
    bool hostMmioWritten = false;
    bool hostGpuSubmitted = false;
    bool exactPsxHalParity = false;
    bool replayValueAuthority = false;
    bool oldWinS0Authority = false;
    bool stage2PlusAuthority = false;
    bool comod2Authority = false;
};

struct FastTransitionPresentResult8001EBF4 {
    bool committed = false;
    uint16_t drawSlotBefore = 0;
    uint16_t drawSlotAfter = 0;
    uint32_t packetAllocator8006ED50 = 0;
    uint32_t mainPageWorkAddress80087288 = 0;
    bool clearColorRequested80040420 = false;
    bool mainPageWorkSubmitRequested80040CA4 = false;
};

enum class Scene0IntRendererAtlasProfile8001AE7C : uint8_t {
    Unknown = 0,
    Scene0Compo00,
    PracticeYCompo,
    Scene0StartupComposite,
};

constexpr uint32_t kScene0StartupCompositeTimCount80016B84 = 721u;

struct ResourceState {
    bool resourceReady = false;
    bool scene0ComodAccepted = false;
    bool titleCameraTableKnown801C6FB4 = false;
    bool cmOpBezResource27Loaded = false;
    bool runtimeTitleCameraBindingKnown = false;
    bool runtimeTitleCameraCursorKnown = false;
    bool titleCameraMatrixKnown80092880 = false;
    bool titlePanelWithCameraKnown80041A68 = false;
    bool paLoc2TodLoaded = false;
    bool paLoc2CoordKnown = false;
    bool runtimeTitleTodBindingKnown = false;
    bool runtimeTitleTodCursorKnown = false;
    bool runtimeChannel1MimeBindingKnown = false;
    bool runtimeChannel1MimeCursorKnown = false;
    bool runtimeChannel3MimeBindingKnown = false;
    bool runtimeChannel3MimeCursorKnown = false;
    bool titleDrawCoordWorldKnown800417A4 = false;
    bool titlePanelCoordKnown801C5D8C = false;
    bool titleGraphControlKnown = false;
    bool firstPaDrawDescKnown8001AF1C = false;
    bool firstPaPrimitiveGroupKnown8004274C = false;
    bool firstPaMode25BaseTriangleKnown = false;
    bool primaryPaMimeBindingKnown80014164 = false;
    bool primaryPaMimeCursorKnown800141D8 = false;
    bool firstPaRuntimeVertexDeformationKnown = false;
    bool firstPaRuntimeMatrixKnown80041A68 = false;
    bool paMode25PrimitiveGroupResultKnown8004274C = false;
    bool firstPaKageDrawDescKnown8001AF1C = false;
    bool firstPaKagePrimitiveGroupKnown8004274C = false;
    bool firstPaKageMode25BaseTriangleKnown = false;
    bool firstPaKageRuntimeMimeBindingKnown = false;
    bool firstPaKageRuntimeMimeCursorKnown = false;
    bool firstPaKageRuntimeVertexDeformationKnown = false;
    bool firstPaKageRuntimeMatrixKnown = false;
    bool firstPaKageMode25Rtpt3InputKnown = false;
    bool firstPaKageMode25Rtpt3OutputKnown = false;
    bool firstPaKageMode25GeometryKnown = false;
    bool firstPaKageMode25PacketPlanKnown = false;
    bool firstPaKageMode25PacketCommitKnown = false;
    bool paKageMode25PrimitiveGroupResultKnown8004274C = false;
    bool firstLoDrawDescKnown8001AF1C = false;
    bool firstLoPrimitiveGroupKnown8004274C = false;
    bool firstLoMode25BaseTriangleKnown = false;
    bool firstLoRuntimeVertexDeformationKnown = false;
    bool firstLoRuntimeMatrixKnown80041A68 = false;
    bool loMode25PrimitiveGroupResultKnown8004274C = false;
    bool firstHpDrawDescKnown8001AF1C = false;
    bool firstHpPrimitiveGroupKnown8004274C = false;
    bool firstHpMode25BaseTriangleKnown = false;
    bool firstHpRuntimeVertexDeformationKnown = false;
    bool firstHpRuntimeMatrixKnown80041A68 = false;
    bool hpMode25PrimitiveGroupResultKnown8004274C = false;
    bool loCoordKnown8004049C = false;
    bool hpCoordKnown8004049C = false;
    bool titlePacketWorkKnown801C609C = false;
    bool titlePacketFrameKnown801C6410 = false;
    bool titleFrameFlagsApplied801C6410 = false;
    bool titleRenderActiveKnown801C9548 = false;
    bool loHpGateKnown801CB600 = false;
    bool presentExtraKnown801CFAEC = false;
    bool titleEarlyInputShortcutSetupKnown801C6410 = false;
    bool titleEarlyInputShortcutReadyKnown801C6410 = false;
    bool titlePresentModelApplied801C689C = false;
    bool titleHudTimResourceBindingKnown801C5094 = false;
    bool titleMimeInitKnown8001385C = false;
    bool titleMimeBaseRestoreKnown800139F8 = false;
    bool titleMimeScratchZeroKnown80013E40 = false;
    bool titleTransitionGridInitKnown8001EEE8 = false;
    bool titleHudTablesLoadedFromComod0801C6Dxx = false;
    // dword_80091858 is the live COMPO00 resource-handle table.  Resource
    // loading establishes the candidate order, but the table is not owned by
    // SS0 until the 801C4260 loader-memory transaction has committed it.
    bool compo00HandleTableKnown80091858 = false;
    bool compo00HandleTableCommitted80091858 = false;
    bool runtimeTitleHudTimRequestKnown801C5094 = false;
    Scene0IntRendererAtlasProfile8001AE7C
        scene0IntRendererAtlasProfile8001AE7C =
            Scene0IntRendererAtlasProfile8001AE7C::Unknown;
    bool scene0IntRendererProjectionKnown8001AE7C = false;
    bool scene0IntRendererSourcePartialVramAuthority8001AE7C = false;
    bool scene0IntRendererCpuAtlasCommitted8001AE7C = false;
    bool scene0IntRendererD3DUploadAttempted8001AE7C = false;
    bool scene0IntRendererD3DUploadCommitted8001AE7C = false;
    bool scene0IntRendererFullVramBackingAuthority8001AE7C = false;
    uint32_t loadedModelCount = 0;
    uint32_t loadedMimePairCount = 0;
    uint32_t loadedTitleTodCount = 0;
    uint32_t registeredTpageCount = 0;
    uint32_t loadedTimCount = 0;
    uint32_t titleRenderActive801C9548 = 0u;
    uint32_t loHpGate801CB600 = 0u;
    uint32_t presentExtra801CFAEC = 0u;
    uint16_t titleEarlyInputShortcutLoopCount801CC676 = 0u;
    uint32_t titlePresentModelApplyCount801C689C = 0u;
    uint8_t titlePresentModelLastCachedLane8006EDA8 = 0u;
    uint8_t titlePresentModelNextDrawLane80096590 = 0u;
    uint32_t titleHudTimResourceGeneration801C5094 = 0u;
    uint32_t compo00HandleTableGeneration80091858 = 0u;
    uint32_t compo00HandleTableNonZeroCount80091858 = 0u;
    uint32_t runtimeTitleHudTimBatchCount801C5094 = 0u;
    uint32_t runtimeTitleHudTimLoadedCount801C5094 = 0u;
    uint32_t runtimeTitleHudTimLastRequestedCount801C5094 = 0u;
    uint32_t runtimeTitleHudTimLastAppliedCount801C5094 = 0u;
    uint32_t scene0IntRendererProjectedTimCount8001AE7C = 0u;
    uint32_t scene0IntRendererProjectedTpageCount8001AE7C = 0u;
    uint32_t scene0IntRendererResolvedTextureInputCount8001AE7C = 0u;
    uint32_t scene0IntRendererD3DUploadAttemptCount8001AE7C = 0u;
    uint32_t scene0IntRendererD3DReadyTpageCount8001AE7C = 0u;
    uint32_t scene0IntRendererD3DFailedTpageCount8001AE7C = 0u;
    int16_t runtimeTitleHudTimLastIds801C5094[
        kTitleHudTimRequestCapacity801C5094]{};
};

struct Scene0IntRendererAtlasCandidate8001AE7C {
    Scene0IntRendererAtlasProfile8001AE7C profile =
        Scene0IntRendererAtlasProfile8001AE7C::Unknown;
    bool prepared = false;
    uint32_t projectedTimCount = 0u;
    uint32_t projectedTpageCount = 0u;
    uint32_t resolvedTextureInputCount = 0u;
    bool partialVramProjected = false;
    uint32_t partialVramKnownWordCount = 0u;
    bool d3dUploadAttempted = false;
    bool d3dUploadComplete = false;
    uint32_t d3dTpageCount = 0u;
    uint32_t d3dDirtyTpageCount = 0u;
    uint32_t d3dCreatedTpageCount = 0u;
    uint32_t d3dUpdatedTpageCount = 0u;
    uint32_t d3dFailedTpageCount = 0u;
    uint32_t d3dReadyTpageCount = 0u;
    std::unique_ptr<PsxVramAtlas> atlas{};

    Scene0IntRendererAtlasCandidate8001AE7C();
    ~Scene0IntRendererAtlasCandidate8001AE7C();
    Scene0IntRendererAtlasCandidate8001AE7C(
        Scene0IntRendererAtlasCandidate8001AE7C&& other) noexcept;
    Scene0IntRendererAtlasCandidate8001AE7C& operator=(
        Scene0IntRendererAtlasCandidate8001AE7C&& other) noexcept;
    Scene0IntRendererAtlasCandidate8001AE7C(
        const Scene0IntRendererAtlasCandidate8001AE7C&) = delete;
    Scene0IntRendererAtlasCandidate8001AE7C& operator=(
        const Scene0IntRendererAtlasCandidate8001AE7C&) = delete;
};

struct TitleVramAtlasUploadResult801C689C {
    bool attempted = false;
    bool complete = false;
    bool sourceScene0IntCpuAtlasAuthority = false;
    bool sourceResidentDirectoryCpuAtlas = false;
    bool HasNativeCpuAtlasAuthority() const {
        return sourceScene0IntCpuAtlasAuthority || sourceResidentDirectoryCpuAtlas;
    }
    Scene0IntRendererAtlasProfile8001AE7C sourceAtlasProfile8001AE7C =
        Scene0IntRendererAtlasProfile8001AE7C::Unknown;
    bool visiblePresentAuthority = false;
    uint32_t tpageCount = 0u;
    uint32_t dirtyTpageCount = 0u;
    uint32_t createdTpageCount = 0u;
    uint32_t updatedTpageCount = 0u;
    uint32_t failedTpageCount = 0u;
    uint32_t readyTpageCount = 0u;
};

struct TitleTpageExactResolveResult801C689C {
    bool sourceScene0IntCpuAtlasAuthority = false;
    bool sourceResidentDirectoryCpuAtlas = false;
    bool HasNativeCpuAtlasAuthority() const {
        return sourceScene0IntCpuAtlasAuthority || sourceResidentDirectoryCpuAtlas;
    }
    bool tpageClutResolvable = false;
    bool clutStpKnown = false;
    bool clutHasStpBits = false;
    ID3D11ShaderResourceView* srv = nullptr;
    ID3D11ShaderResourceView* psxAbr0StpSrv = nullptr;
    ID3D11ShaderResourceView* psxAbr1StpSrv = nullptr;
};

enum class TitleEarlyInputShortcutStatus801C6410 : uint8_t {
    NotReady = 0,
    ResourceSourceMissing,
    RuntimeOrderMismatch,
    PacketTransactionRejected,
    SetupCommitted,
    ReadyCommitted,
};

struct TitleEarlyInputShortcutResult801C6410 {
    TitleEarlyInputShortcutStatus801C6410 status =
        TitleEarlyInputShortcutStatus801C6410::NotReady;
    bool committed = false;
    uint16_t channel1LoopCount801CC676 = 0u;
    uint32_t channel1CursorAfter = 0u;
    uint32_t channel3CursorAfter = 0u;
    uint32_t cameraCursorAfter801CB59C = 0u;
};

inline bool IsResourceIngressReady(const ResourceState& state)
{
    return state.scene0ComodAccepted &&
           state.titleHudTablesLoadedFromComod0801C6Dxx &&
           state.titleCameraTableKnown801C6FB4 &&
           state.cmOpBezResource27Loaded &&
           state.loadedModelCount ==
               static_cast<uint32_t>(ModelKind::Count) &&
           state.loadedMimePairCount ==
               static_cast<uint32_t>(MimeKind::Count) &&
           state.loadedTitleTodCount ==
               static_cast<uint32_t>(TitleTodKind::Count) &&
           state.paLoc2TodLoaded && state.paLoc2CoordKnown &&
           state.registeredTpageCount > 0 && state.loadedTimCount > 0 &&
           state.titlePacketWorkKnown801C609C;
}

void Clear();
void ResetRuntimeTitleState801C609C();
const TitleMimeInitState801C609C& GetTitleMimeInitState801C609C();
inline bool IsExactTitleMimeInitState801C609C(
    const TitleMimeInitState801C609C& state)
{
    if (!state.known || !state.exactCallOrder ||
        !state.baseRestoreCalled800139F8 ||
        state.baseRestoreCallCount800139F8 != 4u ||
        !std::all_of(state.baseChannelRestored800139F8.begin(),
                     state.baseChannelRestored800139F8.end(),
                     [](bool restored) { return restored; }) ||
        state.channelInitCallCount8001385C != 3u ||
        !state.scratchClearCalled80013E40 ||
        state.scratchZeroByteCount80013E40 !=
            kTitleMimeScratchBytes80013E40 ||
        !state.transitionGridInitCalled8001EEE8 ||
        state.gp49 != 0u || state.gp50 != 0u || state.gp51 != 0u ||
        !std::all_of(state.scratch80090240.begin(),
                     state.scratch80090240.end(),
                     [](uint8_t value) { return value == 0u; }) ||
        !std::all_of(state.transitionGrid8001EEAC.begin(),
                     state.transitionGrid8001EEAC.end(),
                     [](uint8_t value) { return value == 1u; })) {
        return false;
    }

    static constexpr struct {
        uint8_t channel;
        ModelKind modelKind;
        MimeKind mimeKind;
        uint32_t destinationVertexAddressA4;
    } kExpected[] = {
        {1u, ModelKind::Lo, MimeKind::Logo, 0x801CB66Cu},
        {2u, ModelKind::PaKage, MimeKind::PaOki, 0x801CD81Cu},
        {3u, ModelKind::Hp, MimeKind::Hiphop, 0x801CC71Cu},
    };
    for (const auto& expected : kExpected) {
        const auto& channel = state.channels[expected.channel];
        if (!channel.known || channel.channel != expected.channel ||
            channel.modelKind != expected.modelKind ||
            channel.mimeKind != expected.mimeKind ||
            channel.channelStateAddress80074B08 !=
                kTitleMimeChannelStateBase80074B08 +
                    kTitleMimeChannelStateStride8001385C *
                        expected.channel ||
            channel.baseSnapshotAddress8006F6A8 !=
                kTitleMimeBaseSnapshotBank8006F6A8 +
                    kTitleMimeBaseSnapshotStride8001385C *
                        expected.channel ||
            channel.vdfStateAddress80074B80 !=
                kTitleMimeVdfStateBase80074B80 +
                    kTitleMimeChannelStateStride8001385C *
                        expected.channel ||
            channel.vdfWorkspaceAddress8006FB08 !=
                kTitleMimeVdfWorkspaceBase8006FB08 +
                    kTitleMimeVdfWorkspaceStride8001385C *
                        expected.channel ||
            channel.scratchAddress80090240 !=
                kTitleMimeScratchBase80090240 +
                    kTitleMimeScratchStride80013E40 * expected.channel ||
            channel.destinationVertexAddressA4 !=
                expected.destinationVertexAddressA4 ||
            channel.modelObjectCount == 0u ||
            channel.modelVertexCount == 0u ||
            channel.vdfKeyCount == 0u ||
            channel.vdfKeyCount != channel.datKeyCount ||
            !channel.tmdBound80013650 ||
            !channel.baseSnapshotCommitted8001371C ||
            !channel.vdfBound800137BC) {
            return false;
        }
    }
    return true;
}
bool LoadResources(ResourceManager* resources,
                   ImmutableScene0ComodView comod0,
                   PrStage1LoaderMemoryDirectState* loaderMemory);
bool CommitCompo00HandleTable80091858(
    const PrStage1LoaderMemoryDirectState& loaderMemory,
    const ResourceManager& resources);
Scene0IntRendererAtlasCandidate8001AE7C
BuildScene0IntRendererAtlasCandidate8001AE7C(
    const PrSS0Scene0IntRendererDirect::Transaction8001AE7C& transaction,
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& commonIntLoad,
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& zCompoIntLoad,
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& compoIntLoad);
bool IsExactScene0IntRendererAtlasCandidate8001AE7C(
    const Scene0IntRendererAtlasCandidate8001AE7C& candidate,
    const PrSS0Scene0IntRendererDirect::Transaction8001AE7C& transaction,
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& commonIntLoad,
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& zCompoIntLoad,
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& compoIntLoad);
bool CommitScene0IntRendererAtlasCandidate8001AE7C(
    Scene0IntRendererAtlasCandidate8001AE7C&& candidate,
    PrSS0Scene0IntRendererDirect::Transaction8001AE7C& transaction,
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& commonIntLoad,
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& zCompoIntLoad,
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& compoIntLoad);
bool ApplyRendererAtlasPartialVram8001AE7C(
    Scene0IntRendererAtlasCandidate8001AE7C& candidate,
    const PrSS0Scene0IntGpuDirect::Transaction8001AE7C& gpuTransaction);
Scene0IntRendererAtlasCandidate8001AE7C
BuildPracticeYCompoRendererAtlasCandidate8001AE7C(
    const PrSS0Scene0IntRendererDirect::Transaction8001AE7C& transaction,
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& intLoad,
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& startupCommonIntLoad,
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& startupZCompoIntLoad,
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& startupCompoIntLoad);
bool IsExactPracticeYCompoRendererAtlasCandidate8001AE7C(
    const Scene0IntRendererAtlasCandidate8001AE7C& candidate,
    const PrSS0Scene0IntRendererDirect::Transaction8001AE7C& transaction);
bool PreparePracticeYCompoRendererAtlasD3DCandidate8001AE7C(
    Scene0IntRendererAtlasCandidate8001AE7C& candidate,
    const PrSS0Scene0IntRendererDirect::Transaction8001AE7C&
        rendererTransaction,
    D3D11Renderer* renderer);
bool IsExactPracticeYCompoRendererAtlasD3DCandidate8001AE7C(
    const Scene0IntRendererAtlasCandidate8001AE7C& candidate,
    const PrSS0Scene0IntRendererDirect::Transaction8001AE7C& transaction);
bool CommitPracticeYCompoRendererAtlasCandidate8001AE7C(
    Scene0IntRendererAtlasCandidate8001AE7C&& candidate,
    PrSS0Scene0IntRendererDirect::Transaction8001AE7C& rendererTransaction,
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& intLoad,
    const PrSS0Scene0IntSideEffectDirect::Transaction8001A8F0&
        loaderCandidate,
    const PrSS0Scene0IntGpuDirect::Transaction8001AE7C& gpuTransaction);
TitleHudTimApplyResult801C5094 ApplyTitleHudTimRequest801C5094(
    ResourceManager* resources,
    const PrSS0TitleHudEventsDirect::TitleHudTimRequest801C5094& request);
const char* TitleHudTimApplyStatusName801C5094(
    TitleHudTimApplyStatus801C5094 status);
TitleMovie0TState0GraphFlipResult801C4B8C
ApplyTitleMovie0TState0GraphFlip801C4B8C();
TitleMovie0TState0GraphFlipResult801C4B8C
ApplyOpeningMovie0GraphFlip8001ED74();
OpeningMovie0WorkListFlushResult8001ED3C
ApplyOpeningMovie0WorkListFlush8001ED3C(uint16_t gp368WorkSlot,
                                       uint32_t priorCallCount80040CA4);
TitleMovie0TState0GraphFlipResult801C4B8C
ApplyTitleMovie0TEarlyInputGraphFlip801C4B7C();
TitlePresentModelResult801C689C ApplyTitlePresentModel801C689C();
FastTransitionPresentResult8001EBF4 ApplyFastTransitionPresent8001EBF4(
    const PrSS0TransitionDirect::FastTransitionVisualFrame800201AC&
        visualFrame);
FastTransitionPresentResult8001EBF4 ApplySlowTransitionPresent8001EBF4(
    const PrSS0TransitionDirect::SlowTransitionVisualFrame80020110&
        visualFrame);
ResourceState GetResourceState();
bool ApplyScene0LoaderResetPreservingTitlePacketArena80025A34(
    PrStage1LoaderMemoryDirectState& loaderMemory);
// A resident directory is not a Scene0 COMPO/model initialization. Bind its
// independently loaded native VRAM and page graph without claiming S0 authority.
// Caller must unbind before unloading the source scene or destroying its graph.
void BindResidentDirectoryRenderProjection80015788(
    PsxVramAtlas* atlas, PrPsxGraphOwnerDirect::PsxGraphState* graph);

TitleVramAtlasUploadResult801C689C PrepareTitleVramAtlas801C689C(
    D3D11Renderer* renderer);
ID3D11ShaderResourceView* ResolveTitleTpageSRV801C689C(
    D3D11Renderer* renderer,
    uint16_t tpage,
    uint16_t clut);
ID3D11ShaderResourceView* ResolveTitleStandaloneTimSRV801C689C(
    D3D11Renderer* renderer,
    uint16_t orgX,
    uint16_t orgY,
    uint16_t width,
    uint16_t height,
    uint16_t clutX,
    uint16_t clutY,
    int psxAbr = -1);
TitleTpageExactResolveResult801C689C
ResolveTitleTpageSRVExact801C689C(D3D11Renderer* renderer,
                                  uint16_t tpage,
                                  uint16_t clut);
ModelResourceView GetModelResourceView(ModelKind kind);
MimeResourceView GetMimeResourceView(MimeKind kind);
TitleTodResourceView GetTitleTodResourceView(TitleTodKind kind);
RuntimeChannel2MimeBinding801C6410 GetRuntimeChannel2MimeBinding801C6410();
RuntimeChannel1MimeBinding801C6410 GetRuntimeChannel1MimeBinding801C6410();
RuntimeChannel3MimeBinding801C6410 GetRuntimeChannel3MimeBinding801C6410();
RuntimePrimaryPaMimeBinding80014164
GetRuntimePrimaryPaMimeBinding80014164();
RuntimeTitleTodBinding801C6410 GetRuntimeTitleTodBinding801C6410();
RuntimeTitleCameraBinding801C6410 GetRuntimeTitleCameraBinding801C6410();
bool CanCommitRuntimeChannel2MimeBinding801C6410(
    const PrSS0TitleHudEventsDirect::TitleEventEffect801C5190& effect);
PreparedRuntimeChannel2MimeBinding801C6410
PrepareRuntimeChannel2MimeBinding801C6410(
    const PrSS0TitleHudEventsDirect::TitleEventEffect801C5190& effect);
void CommitPreparedRuntimeChannel2MimeBinding801C6410(
    const PreparedRuntimeChannel2MimeBinding801C6410& prepared);
bool CommitRuntimeChannel2MimeBinding801C6410(
    const PrSS0TitleHudEventsDirect::TitleEventEffect801C5190& effect);
bool CommitRuntimeTitleEventResources801C6410(
    const PrSS0TitleHudEventsDirect::TitleEventEffect801C5190& effect);
RuntimeChannel2MimeAdvanceResult80013EA8
AdvanceRuntimeChannel2Mime80013EA8();
RuntimeChannel1MimeAdvanceResult80013EA8
AdvanceRuntimeChannel1Mime80013EA8();
RuntimeChannel3MimeAdvanceResult80013EA8
AdvanceRuntimeChannel3Mime80013EA8();
RuntimePrimaryPaMimeAdvanceResult800141D8
AdvanceRuntimePrimaryPaMime800141D8();
RuntimeTitleTodAdvanceResult8001B000 AdvanceRuntimeTitleTod8001B000();
RuntimeTitleCameraAdvanceResult80041D3C
AdvanceRuntimeTitleCamera80041D3C();
bool ComposeRuntimeTitleMatrix801C5E60();
bool ComposeRuntimePaMatrix8001B084();
bool ComposeRuntimeLoMatrix8001B084();
bool ComposeRuntimeHpMatrix8001B084();
bool BeginTitlePacketFrame801C6410();
bool ApplyTitleFrameFlags801C6410(uint32_t ctxFlags);
TitleEarlyInputShortcutResult801C6410
ApplyTitleEarlyInputShortcutSetupFrame801C6410();
TitleEarlyInputShortcutResult801C6410
ApplyTitleEarlyInputShortcutReadyFrame801C6410();
bool IsTitleEarlyInputShortcutReady801C6410();
bool ExecuteFirstPaKageMode25Rtpt3At800428B0();
bool BuildFirstPaKageMode25PacketPlan800428B0();
bool CommitFirstPaKageMode25PacketPlan800428B0();
bool ExecutePaMode25PrimitiveGroup8004274C();
bool ExecutePaKageMode25PrimitiveGroup8004274C();
bool CanExecuteLoHpPrimitiveGroups801C5EF0();
bool CanExecuteHpPrimitiveGroup801C5EF0();
bool ExecuteLoMode25PrimitiveGroup8004274C();
bool ExecuteHpMode25PrimitiveGroup8004274C();
const TmdObject* GetModelObject(ModelKind kind,
                                std::size_t objectIndex = 0u);
const VdfData* GetMimeVdf(MimeKind kind);
const DatData* GetMimeDat(MimeKind kind);
const TodData* GetTitleTod(TitleTodKind kind);
const TodCoordMatrix* GetPaLoc2Coord();
const PrPsxGteDirect::Matrix3x4* GetTitlePanelCoord801CD7BC();
const PrPsxGteDirect::Matrix3x4*
GetRuntimeTitleDrawCoordWorld800417A4();
const PrPsxGteDirect::Matrix3x4* GetRuntimeTitleCameraMatrix80092880();
const PrPsxGteDirect::Matrix3x4* GetRuntimeTitlePanelWithCamera80041A68();
const PrPsxGteDirect::Matrix3x4* GetFirstPaKageRuntimeMatrix800406D8();
const PrSS0TitleTransformDirect::Mode25Rtpt3Input*
GetFirstPaKageMode25Rtpt3Input801C5E60();
const PrPsxGteDirect::Rtpt3ExactOutput280030*
GetFirstPaKageMode25Rtpt3Output800428B0();
const PrPsxGteDirect::Mode25TriangleGeometryTrace*
GetFirstPaKageMode25Geometry800428B0();
const PrSS0TitlePacketWorkDirect::RuntimeState801C609C*
GetTitlePacketWork801C609C();
const PrSS0TitleDrawDescDirect::RuntimeState8001AF1C*
GetFirstPaKageDrawDesc8001AF1C();
const PrSS0TitleDrawDescDirect::RuntimeState8001AF1C*
GetFirstPaDrawDesc8001AF1C();
const PrSS0TitleDrawDescDirect::RuntimeState8001AF1C*
GetFirstLoDrawDesc8001AF1C();
const PrSS0TitleDrawDescDirect::RuntimeState8001AF1C*
GetFirstHpDrawDesc8001AF1C();
const PrSS0TitlePacketPlanDirect::BuildResult800428B0*
GetFirstPaKageMode25PacketPlan800428B0();
const PrSS0TitlePacketCommitDirect::CommitResult800428B0*
GetFirstPaKageMode25PacketCommit800428B0();
const PrSS0TitlePrimitiveGroupDirect::ExecuteResult8004274C*
GetPaKageMode25PrimitiveGroup8004274C();
const PrSS0TitlePrimitiveGroupDirect::ExecuteResult8004274C*
GetPaMode25PrimitiveGroup8004274C();
const PrSS0TitlePrimitiveGroupDirect::ExecuteResult8004274C*
GetLoMode25PrimitiveGroup8004274C();
const PrSS0TitlePrimitiveGroupDirect::ExecuteResult8004274C*
GetHpMode25PrimitiveGroup8004274C();
const PrPsxGraphOwnerDirect::PsxGraphState* GetTitleGraphState();
// Scene0's 8001EC54/8001B954 path writes the current draw-buffer work list
// before the 8001ED74 flip.  Keep the mutable access behind the title backend
// so callers do not const-cast the graph singleton themselves.
PrPsxGraphOwnerDirect::PsxGraphState* GetTitleGraphStateMutable();
const PrSS0TitleTransformDirect::Mode25BaseTriangleCarrier*
GetFirstPaKageMode25BaseTriangleCarrier();
const PrSS0TitleTransformDirect::Mode25DeformedTriangleCarrier*
GetFirstPaKageMode25DeformedTriangleCarrier();
const char* ModelName(ModelKind kind);
const char* MimeVdfName(MimeKind kind);
const char* MimeDatName(MimeKind kind);
const char* TitleTodName(TitleTodKind kind);

} // namespace PrSS0TitleTmdBackend
