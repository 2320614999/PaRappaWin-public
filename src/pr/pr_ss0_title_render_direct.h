#pragma once

#include <cstdint>

namespace PrSS0TitleRenderDirect {

static constexpr uint32_t kFn801C609C = 0x801C609Cu;
static constexpr uint32_t kFn801C6410 = 0x801C6410u;
static constexpr uint32_t kFn801C5EF0 = 0x801C5EF0u;
static constexpr uint32_t kFn801C5E60 = 0x801C5E60u;
static constexpr uint32_t kFn801C689C = 0x801C689Cu;
static constexpr uint32_t kFn800141D8 = 0x800141D8u;
static constexpr uint32_t kFn8001B000 = 0x8001B000u;
static constexpr uint32_t kFn8001B084 = 0x8001B084u;
static constexpr uint32_t kFn80013EA8 = 0x80013EA8u;
static constexpr uint32_t kFn800417A4 = 0x800417A4u;
static constexpr uint32_t kFn80041A68 = 0x80041A68u;
static constexpr uint32_t kFn800406D8 = 0x800406D8u;
static constexpr uint32_t kFn80040544 = 0x80040544u;
static constexpr uint32_t kFn8001ADEC = 0x8001ADECu;
static constexpr uint32_t kFn800428B0 = 0x800428B0u;
static constexpr uint32_t kFn8004274C = 0x8004274Cu;
static constexpr uint32_t kFn8001C1E8 = 0x8001C1E8u;
static constexpr uint32_t kFn80040370 = 0x80040370u;
static constexpr uint32_t kFn80040060 = 0x80040060u;
static constexpr uint32_t kFn80040CA4 = 0x80040CA4u;
static constexpr uint32_t kFn8001E3B0 = 0x8001E3B0u;
static constexpr uint32_t kFn8001B1B0 = 0x8001B1B0u;
static constexpr uint32_t kFn80044D64 = 0x80044D64u;
static constexpr uint32_t kFn800431E0 = 0x800431E0u;
static constexpr uint32_t kFn800450A0 = 0x800450A0u;
static constexpr uint32_t kFn80044CD0 = 0x80044CD0u;

static constexpr uint32_t kCompo00HandleTable80091858 = 0x80091858u;
static constexpr uint32_t kRuntimeTimFlag = 0x8000u;
static constexpr uint32_t kRuntimeTimListCtxOffsetACh = 0xACu;
static constexpr uint32_t kRuntimeTimListLaneCount801C6410 = 1u;
static constexpr uint32_t kRuntimeTimUploadFlag801C6410 = 1u;
static constexpr uint32_t kRuntimeTimPayloadHeaderBytes = 4u;
static constexpr uint32_t kRuntimeTimHasClutMask = 0x8u;
static constexpr uint32_t kCtxFlagTitleRenderActive = 0x20u;
static constexpr uint32_t kCtxFlagShortcutStart = 0x4000000u;
static constexpr uint32_t kCtxFlagShortcutReady = 0x8000000u;
static constexpr uint32_t kTitleRenderActive801C9548 = 0x801C9548u;
static constexpr uint32_t kTodCursor801CB604 = 0x801CB604u;
static constexpr uint32_t kTodAnim801CB658 = 0x801CB658u;
static constexpr uint32_t kTodBase801C9534 = 0x801C9534u;
static constexpr uint32_t kTodSubmitSource801CB65C = 0x801CB65Cu;
static constexpr uint32_t kTitleTodCursor801C9540 = 0x801C9540u;
static constexpr uint32_t kTitleTodState801CF81C = 0x801CF81Cu;
static constexpr uint32_t kTitleTmdDrawDesc801CD80C = 0x801CD80Cu;
static constexpr uint32_t kLoCursor801C9538 = 0x801C9538u;
static constexpr uint32_t kLoAnim801CC66C = 0x801CC66Cu;
static constexpr uint32_t kLoSubmit801CB5F0 = 0x801CB5F0u;
static constexpr uint32_t kHpCursor801C953C = 0x801C953Cu;
static constexpr uint32_t kHpAnim801CD71C = 0x801CD71Cu;
static constexpr uint32_t kHpSubmit801CC70C = 0x801CC70Cu;
static constexpr uint32_t kMatrixBase801CD7BC = 0x801CD7BCu;
static constexpr uint32_t kLoHpDrawGate801CB600 = 0x801CB600u;
static constexpr uint32_t kPresentExtraGate801CFAEC = 0x801CFAECu;
static constexpr uint32_t kPresentEnv801C9574 = 0x801C9574u;
static constexpr uint32_t kPresentColorChannelMax = 255u;
static constexpr uint32_t kPresentClearEnvIndex = 0u;
static constexpr uint32_t kPresentEnvOtagOffset10h = 0x10u;
static constexpr uint32_t kPresentClearEnvTable80087288 = 0x80087288u;
static constexpr uint32_t kPresentEnvRecordBytes = 20u;
static constexpr uint32_t kPresentExtraFillX = 0u;
static constexpr uint32_t kPresentExtraFillY = 0u;
static constexpr uint32_t kPresentExtraFillWidth = 320u;
static constexpr uint32_t kPresentExtraFillHeight = 480u;
static constexpr uint32_t kScratchPacketBuffer1F800000 = 0x1F800000u;
static constexpr uint32_t kGsTmdFastHandlerTable8008EDD8 = 0x8008EDD8u;
static constexpr uint32_t kGsTmdFastFlag4F3Table8008EED8 = 0x8008EED8u;
static constexpr uint32_t kGsTmdFastFlag4F4Table8008EEE4 = 0x8008EEE4u;
static constexpr uint32_t kGsTmdFastTnf3Slot8008EE30 = 0x8008EE30u;
static constexpr uint32_t kCompo00TitleTmdFirstHandle = 5u;
static constexpr uint32_t kCompo00TitleTmdLastHandle = 8u;
static constexpr uint32_t kCompo00TitleTmdPrimitiveMode = 0x25u;
static constexpr uint32_t kCompo00TitleTmdPrimitiveBytes = 28u;
static constexpr uint32_t kCompo00TitleTmdPrimitiveDwordStride = 7u;
static constexpr uint32_t kProjectionDepth = 440u;
static constexpr uint32_t kTmdOrderingTableDepth = 14u;
static constexpr uint32_t kTodOrderingTableDepth = 10u;
static constexpr uint32_t kTitleTmdSubmitDepth = 10u;
static constexpr uint32_t kTitleTmdPacketOtShift =
    kTmdOrderingTableDepth - kTitleTmdSubmitDepth;

static constexpr uint32_t kTableResourcePairs801C6C14 = 0x801C6C14u;
static constexpr uint32_t kTableHudTimTimelines801C6D4C = 0x801C6D4Cu;
static constexpr uint32_t kTableEventStream801C6E50 = 0x801C6E50u;
static constexpr uint32_t kTableSelector801C6F94 = 0x801C6F94u;

enum class TitleResourceKind : uint8_t {
    Unknown = 0,
    VabMinimum,
    PaLoc2Tod,
    HpTmd,
    LoTmd,
    PaTmd,
    PaKageTmd,
    VdfRange,
    DatRange,
    CmOpBez,
    TitleTimRange,
};

enum class TitleRawTableKind : uint8_t {
    Unknown = 0,
    ResourcePairs,
    HudTimTimelines,
    EventStream,
    Selector,
};

enum class TitleRenderActionKind : uint8_t {
    None = 0,
    InitWorkBuffers,
    InitOrderingTables,
    InitRenderGlobals,
    InitCoord,
    SetProjection440,
    BindCompo00Handle,
    InitDrawDescriptors,
    InitTodCursor,
    InitMimeTodResources,
    GateTitleFrameSource801C6410,
    Call801C6410Frame,
    SetTitleRenderActiveFromCtxFlag,
    SetPresentExtraFromCtxFlag,
    ClearPresentExtraFromCtxFlag,
    SetLoHpDrawGateFromCtxFlag,
    GateRuntimeTimListSource801C6410,
    UploadRuntimeTimList,
    Call8001ADECUploadTim,
    GateRuntimeTimUploadHalBoundary8001ADEC,
    Call801C5EF0,
    Call800141D8,
    Call8001B000TodTick,
    Call8001B084SubmitTod,
    Call80013EA8,
    Call801C5E60,
    Call800428B0TmdSubmit,
    IncrementTodCursor801CB604,
    Call8001B000TodBuild,
    Call8001B084SubmitPaLoc2Tod,
    IncrementTitleTodCursor801C9540,
    Call80013EA8TitleTod,
    Call801C5E60TmdSubmit,
    Call800417A4CopyMatrix,
    Call80041A68BuildMatrix,
    Call800406D8ConcatMatrix,
    Call80040544ApplyMatrix,
    Call800428B0PacketDispatch,
    Gate8001C1E8TmdFastTableInit,
    Gate8004274CPrimitiveGrouping,
    Gate800428B0Compo00PrimitiveCoverage,
    Gate800428B0PacketHalBoundary,
    SetLoCursor801C9538,
    Call80013EA8LoAnim,
    Call8001B084SubmitLo,
    IncrementHpCursor801C953C,
    Call80013EA8HpAnim,
    Call8001B084SubmitHp,
    GateLoHpDrawSource801C5EF0,
    OptionalLoHpDraw,
    Call801C689C,
    Call80040370Flip,
    Call80040060SetEnv,
    Call80040CA4DrawOtag,
    Call8001E3B0Clear,
    GatePresentMainHalBoundary801C689C,
    GatePresentExtraSource801C689C,
    Call80040370OptionalExtraFlip,
    Call8001B1B0Optional,
    GatePresentExtraFillHalBoundary80044CD0,
    Gap,
};

struct Compo00HandleSpec {
    TitleResourceKind kind = TitleResourceKind::Unknown;
    uint32_t firstHandle = 0;
    uint32_t lastHandle = 0;
    uint32_t memIndexBase = 0;
    bool fromMemRecord = false;
    const char* psxName = nullptr;
};

struct TitleRawTableSpec {
    TitleRawTableKind kind = TitleRawTableKind::Unknown;
    uint32_t address = 0;
    uint32_t rowCount = 0;
    uint32_t rowBytes = 0;
    const char* shape = nullptr;
};

struct TitleRenderAction {
    TitleRenderActionKind kind = TitleRenderActionKind::None;
    uint32_t psxFunction = 0;
    TitleResourceKind resource = TitleResourceKind::Unknown;
    uint32_t arg0 = 0;
    uint32_t arg1 = 0;
    uint32_t arg2 = 0;
    uint32_t arg3 = 0;
    bool conditional = false;
};

struct TitleRenderPlan {
    bool runtimeCutoverAllowed = false;
    bool hasOpenP0Gap = true;
    TitleRenderAction actions[96]{};
    uint32_t count = 0;
    bool truncated = false;
};

bool RuntimeCutoverAllowed();

uint32_t KnownCompo00HandleSpecCount();
Compo00HandleSpec KnownCompo00HandleSpecAt(uint32_t index);
uint32_t KnownTitleRawTableSpecCount();
TitleRawTableSpec KnownTitleRawTableSpecAt(uint32_t index);

TitleRenderPlan BuildTitleResourceMapPlan();
TitleRenderPlan BuildTitleRenderInit801C609CPlan();
TitleRenderPlan BuildTitleRenderFrame801C6410Plan(
    uint32_t ctxFlags,
    bool dword801CB600Nonzero);
TitleRenderPlan BuildTitleDraw801C5EF0Plan(bool dword801CB600Nonzero);
TitleRenderPlan BuildTitlePresent801C689CPlan(bool dword801CFAECNonzero);

const char* TitleResourceKindName(TitleResourceKind kind);
const char* TitleRawTableKindName(TitleRawTableKind kind);
const char* TitleRenderActionKindName(TitleRenderActionKind kind);

} // namespace PrSS0TitleRenderDirect
