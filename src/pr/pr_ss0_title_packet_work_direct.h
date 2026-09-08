#pragma once

#include "pr_psx_graph_owner_direct.h"
#include "pr_scene_drawbuffer_direct.h"

#include <array>
#include <cstdint>

struct PrStage1LoaderMemoryDirectState;

namespace PrSS0TitlePacketWorkDirect {

constexpr uint32_t kFn801C609C = 0x801C609Cu;
constexpr uint32_t kCallsite801C60C8 = 0x801C60C8u;
constexpr uint32_t kFn801C6410 = 0x801C6410u;
constexpr uint32_t kFn801C689C = 0x801C689Cu;
constexpr int32_t kPacketArenaBytes801C5D28 = 0x15180;
constexpr uint32_t kPacketArenaLaneBytes801C5D28 = 0xA8C0u;
constexpr uint32_t kWorkOrder801C609C = 10u;
constexpr uint32_t kWork0Address801C9574 = 0x801C9574u;
constexpr uint32_t kWork0OtHead801C959C = 0x801C959Cu;
constexpr uint32_t kWorkLaneStride801C609C = 0x14u;
constexpr uint32_t kWorkOtHeadStride801C609C = 0x1000u;
// 801C6410/801C689C pass the literal address 801C9574.  The adjacent
// 801C9588/801CA59C-looking storage is not referenced by the original title
// call chain; only the packet allocator selected by 8004019C is double-sided.
constexpr uint8_t kTitleWorkFixedLane801C609C = 0u;
constexpr uint32_t kMainPageWork0Address80087288 = 0x80087288u;
constexpr uint32_t kMainPageWorkLaneStride8001E3B0 = 0x14u;
constexpr uint32_t kMainPageOtHead0Address80088288 = 0x80088288u;
constexpr uint32_t kMainPageOtHeadLaneStride8001E3B0 = 0x10000u;
constexpr uint32_t kMainPageWorkOrder8001E3B0 = 14u;
// COMOD0 801C6410/801C689C pass literal zero to 8001E374/8001E3B0.  The
// title work list is double-buffered, but this independent main-page helper
// is not selected from the title draw lane at those call sites.
constexpr uint8_t kMainPageFixedLane8001E3B0 = 0u;
constexpr uint32_t kDrawEnvBase8008A730 = 0x8008A730u;
constexpr uint32_t kDrawEnvStride80040060 = 16u;
constexpr int16_t kPresentExtraFillX8001B1B0 = 0;
constexpr int16_t kPresentExtraFillY8001B1B0 = 0;
constexpr int16_t kPresentExtraFillWidth8001B1B0 = 320;
constexpr int16_t kPresentExtraFillHeight8001B1B0 = 480;
constexpr uint32_t kCtxFlagTitleRenderActive801C6410 = 0x00000020u;
constexpr uint32_t kCtxFlagLoPairWrite801C6410 = 0x00400000u;
constexpr uint32_t kCtxFlagTitleShortcutSetup801C6410 = 0x04000000u;
constexpr uint32_t kCtxFlagTitleImmediateSetup801C6410 = 0x08000000u;

enum class TitleFrameSetupSource801C6410 : uint8_t {
    None = 0,
    EventCtxFlags,
    EarlyInputShortcut,
};

// Software owner for the draw-environment packet written by 80040060. The
// command is linked to the post-80040370 graph lane and carries the fixed
// 320x240 clip rectangle/white clear color observed in COMOD0. It remains
// translated PSX state; no host draw-environment API is implied.
struct TitleDrawEnvState80040060 {
    bool known = false;
    uint16_t graphSlot80096590 = 0u;
    uint32_t sourceWorkListAddress801C9574 = 0u;
    uint32_t sourceOtagHeadAddress = 0u;
    uint32_t commandAddress8008A730 = 0u;
    uint8_t r = 0u;
    uint8_t g = 0u;
    uint8_t b = 0u;
    int16_t clipX = 0;
    int16_t clipY = 0u;
    uint16_t clipWidth = 0u;
    uint16_t clipHeight = 0u;
};

struct RuntimeState801C609C {
    bool initialized = false;
    uint32_t packetArenaBase80025B28 = 0;
    std::array<uint32_t, 2> packetAllocatorBases801C956C{};
    PrSceneDrawBufferDirect::DrawBufferGlobals8001E33C
        packetLaneBinding8001E33C{};
    uint32_t packetLaneBindingCallsite801C60C8 = 0;
    std::array<PrPsxGraphOwnerDirect::PsxGraphWorkList80040CC8, 2>
        workLists801C9574{};
    bool framePrepared801C6410 = false;
    bool frameFlagsApplied801C6410 = false;
    uint8_t currentDrawBuffer8004019C = 0;
    uint32_t currentPacketAllocator800901C8 = 0;
    uint32_t titleRenderActive801C9548 = 0;
    uint32_t loHpGate801CB600 = 0;
    uint32_t presentExtra801CFAEC = 0;
    bool titleShortcutSetupApplied801C6410 = false;
    bool titleShortcutReadyApplied801C6410 = false;
    TitleFrameSetupSource801C6410 frameSetupSource801C6410 =
        TitleFrameSetupSource801C6410::None;
    uint32_t titleSubmitCallCount80040CA4 = 0u;
    uint32_t mainSubmitCallCount8001E3B0 = 0u;
    TitleDrawEnvState80040060 drawEnv80040060{};
};

struct InitializeResult801C609C {
    bool initialized = false;
    uint32_t packetArenaBase80025B28 = 0;
    uint32_t packetArenaBytes = 0;
    PrSceneDrawBufferDirect::DrawBufferGlobals8001E33C
        packetLaneBinding8001E33C{};
    uint32_t packetLaneBindingCallsite801C60C8 = 0;
    bool packetLaneGraphBindingApplied8001E33C = false;
};

struct BeginFrameResult801C6410 {
    bool prepared = false;
    uint8_t drawBuffer8004019C = 0;
    uint32_t packetAllocator800901C8 = 0;
    uint32_t workAddress801C9574 = 0;
    PrPsxGraphOwnerDirect::PsxGraphClearWorkListResult80040CC8
        titleClear80040CC8{};
    PrPsxGraphOwnerDirect::PsxGraphClearWorkListResult80040CC8
        mainPageClear8001E374{};
};

enum class FrameFlagsFailure801C6410 : uint8_t {
    None = 0,
    RuntimeNotReady,
    AlreadyApplied,
    ShortcutSetupUnsupported,
    ImmediateSetupUnsupported,
    ShortcutReadyBeforeSetup,
};

struct FrameFlagsResult801C6410 {
    bool applied = false;
    FrameFlagsFailure801C6410 failure =
        FrameFlagsFailure801C6410::RuntimeNotReady;
    uint32_t ctxFlags = 0;
    uint32_t titleRenderActiveBefore801C9548 = 0;
    uint32_t titleRenderActiveAfter801C9548 = 0;
    uint32_t loHpGateBefore801CB600 = 0;
    uint32_t loHpGateAfter801CB600 = 0;
    uint32_t presentExtraBefore801CFAEC = 0;
    uint32_t presentExtraAfter801CFAEC = 0;
    bool shortcutSetupWriter801C64E0 = false;
    bool shortcutReadyWriter801C663C = false;
    bool loHpWriterFromPairSetup801C66B0 = false;
};

enum class EarlyInputShortcutFailure801C6410 : uint8_t {
    None = 0,
    RuntimeNotReady,
    SetupAlreadyApplied,
    SetupMissing,
    ReadyAlreadyApplied,
    FramePrepareFailed,
    FlagApplyFailed,
};

struct EarlyInputShortcutResult801C6410 {
    bool committed = false;
    EarlyInputShortcutFailure801C6410 failure =
        EarlyInputShortcutFailure801C6410::RuntimeNotReady;
    BeginFrameResult801C6410 frame{};
    FrameFlagsResult801C6410 flags{};
};

enum class PresentFrameFailure801C689C : uint8_t {
    None = 0,
    RuntimeNotReady,
    FrameFlagsNotApplied,
    CachedLaneInvalid,
    GraphSlotMismatch,
    AllocatorMismatch,
    TitleWorkMismatch,
    MainPageWorkMismatch,
    TitleWorkSubmitFailed,
    MainPageWorkSubmitFailed,
    FlipModelMismatch,
};

struct PresentFrameResult801C689C {
    bool modelApplied = false;
    PresentFrameFailure801C689C failure =
        PresentFrameFailure801C689C::RuntimeNotReady;
    uint8_t cachedDrawLane8006EDA8 = 0;
    uint32_t titleWorkAddress801C9574 = 0;
    uint32_t mainPageWorkAddress80087288 = 0;
    uint8_t clearR80040060 = 0;
    uint8_t clearG80040060 = 0;
    uint8_t clearB80040060 = 0;
    bool flipGraphStateAdvanced80040370 = false;
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
    TitleDrawEnvState80040060 drawEnv80040060{};
    PrPsxGraphOwnerDirect::PsxGraphFlipResult80040370 flip{};
    PrPsxGraphOwnerDirect::PsxGraphFlipResult80040370 extraFlip{};
};

enum class LoaderResetFailure801C4260 : uint8_t {
    None = 0,
    RuntimeNotReady,
    PacketArenaMismatch,
    PacketLaneMismatch,
    PacketBindingMismatch,
    GraphPacketLaneMismatch,
    LoaderHeapBaseMismatch,
    LoaderHeapEndMismatch,
    LoaderLowWaterMismatch,
    LoaderHeapCursorMismatch,
};

struct LoaderResetResult801C4260 {
    bool applied = false;
    LoaderResetFailure801C4260 failure =
        LoaderResetFailure801C4260::RuntimeNotReady;
};

void Clear(RuntimeState801C609C& state);

void ResetTitleFrameGates801C609C(RuntimeState801C609C& state);

InitializeResult801C609C Initialize801C609C(
    RuntimeState801C609C& state,
    PrStage1LoaderMemoryDirectState* loaderMemory,
    PrPsxGraphOwnerDirect::PsxGraphState& graph);

LoaderResetResult801C4260
ApplyLoaderReset80025A34PreservingPacketArena801C4260(
    const RuntimeState801C609C& state,
    const PrPsxGraphOwnerDirect::PsxGraphState& graph,
    PrStage1LoaderMemoryDirectState& loaderMemory);

BeginFrameResult801C6410 BeginFrame801C6410(
    RuntimeState801C609C& state,
    PrPsxGraphOwnerDirect::PsxGraphState& graph);

FrameFlagsResult801C6410 ApplyTitleFrameFlags801C6410(
    RuntimeState801C609C& state,
    uint32_t ctxFlags);

EarlyInputShortcutResult801C6410
ApplyEarlyInputShortcutSetupFrame801C6410(
    RuntimeState801C609C& state,
    PrPsxGraphOwnerDirect::PsxGraphState& graph);

EarlyInputShortcutResult801C6410
ApplyEarlyInputShortcutReadyFrame801C6410(
    RuntimeState801C609C& state,
    PrPsxGraphOwnerDirect::PsxGraphState& graph);

PresentFrameResult801C689C ApplyPresentFrameModel801C689C(
    RuntimeState801C609C& state,
    PrPsxGraphOwnerDirect::PsxGraphState& graph);

const PrPsxGraphOwnerDirect::PsxGraphWorkList80040CC8*
GetCurrentWorkList801C6410(const RuntimeState801C609C& state);

} // namespace PrSS0TitlePacketWorkDirect
