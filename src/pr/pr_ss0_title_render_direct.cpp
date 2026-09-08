#include "pr_ss0_title_render_direct.h"

namespace PrSS0TitleRenderDirect {
namespace {

static const Compo00HandleSpec kCompo00HandleSpecs[] = {
    {TitleResourceKind::VabMinimum, 1, 1, 0, false, "MINIMUM.VH"},
    {TitleResourceKind::PaLoc2Tod, 4, 4, 2, true, "PA_LOC2.TOD"},
    {TitleResourceKind::HpTmd, 5, 5, 3, true, "HP.TMD"},
    {TitleResourceKind::LoTmd, 6, 6, 4, true, "LO.TMD"},
    {TitleResourceKind::PaTmd, 7, 7, 5, true, "PA.TMD"},
    {TitleResourceKind::PaKageTmd, 8, 8, 6, true, "PA_KAGE.TMD"},
    {TitleResourceKind::VdfRange, 9, 17, 7, true, "VDF[0..8]"},
    {TitleResourceKind::DatRange, 18, 26, 16, true, "DAT[0..8]"},
    {TitleResourceKind::CmOpBez, 27, 27, 25, true, "CM_OP.BEZ"},
    {TitleResourceKind::TitleTimRange, 28, 60, 26, true, "F_*.TIM"},
};

static const TitleRawTableSpec kRawTableSpecs[] = {
    {TitleRawTableKind::ResourcePairs, kTableResourcePairs801C6C14,
     78, 4, "{s16 idA,s16 idB}"},
    {TitleRawTableKind::HudTimTimelines, kTableHudTimTimelines801C6D4C,
     7, 12, "{u32 eventsPtr,u32 count,u32 cursor}"},
    {TitleRawTableKind::EventStream, kTableEventStream801C6E50,
     7, 16, "{u32 frame,u32 flags,u8 controls[8]}"},
    {TitleRawTableKind::Selector, kTableSelector801C6F94,
     4, 8, "{s16 idA,s16 idB,u16 ticks,s16 hudSlotId}"},
};

static bool Append(TitleRenderPlan& plan, const TitleRenderAction& action)
{
    if (plan.count >= sizeof(plan.actions) / sizeof(plan.actions[0])) {
        plan.truncated = true;
        return false;
    }
    plan.actions[plan.count++] = action;
    return true;
}

static void AppendAction(TitleRenderPlan& plan,
                         TitleRenderActionKind kind,
                         uint32_t psxFunction,
                         TitleResourceKind resource =
                             TitleResourceKind::Unknown,
                         uint32_t arg0 = 0,
                         uint32_t arg1 = 0,
                         uint32_t arg2 = 0,
                         uint32_t arg3 = 0,
                         bool conditional = false)
{
    TitleRenderAction action{};
    action.kind = kind;
    action.psxFunction = psxFunction;
    action.resource = resource;
    action.arg0 = arg0;
    action.arg1 = arg1;
    action.arg2 = arg2;
    action.arg3 = arg3;
    action.conditional = conditional;
    (void)Append(plan, action);
}

static void AppendPlan(TitleRenderPlan& dst, const TitleRenderPlan& src)
{
    for (uint32_t i = 0; i < src.count; ++i) {
        (void)Append(dst, src.actions[i]);
    }
    dst.truncated = dst.truncated || src.truncated;
    dst.hasOpenP0Gap = dst.hasOpenP0Gap || src.hasOpenP0Gap;
}

static TitleRenderPlan MakePlan()
{
    TitleRenderPlan plan{};
    plan.runtimeCutoverAllowed = RuntimeCutoverAllowed();
    return plan;
}

} // namespace

bool RuntimeCutoverAllowed()
{
    return false;
}

uint32_t KnownCompo00HandleSpecCount()
{
    return sizeof(kCompo00HandleSpecs) / sizeof(kCompo00HandleSpecs[0]);
}

Compo00HandleSpec KnownCompo00HandleSpecAt(uint32_t index)
{
    if (index >= KnownCompo00HandleSpecCount()) {
        return Compo00HandleSpec{};
    }
    return kCompo00HandleSpecs[index];
}

uint32_t KnownTitleRawTableSpecCount()
{
    return sizeof(kRawTableSpecs) / sizeof(kRawTableSpecs[0]);
}

TitleRawTableSpec KnownTitleRawTableSpecAt(uint32_t index)
{
    if (index >= KnownTitleRawTableSpecCount()) {
        return TitleRawTableSpec{};
    }
    return kRawTableSpecs[index];
}

TitleRenderPlan BuildTitleResourceMapPlan()
{
    TitleRenderPlan plan = MakePlan();
    plan.hasOpenP0Gap = true;

    for (uint32_t i = 0; i < KnownCompo00HandleSpecCount(); ++i) {
        const Compo00HandleSpec spec = KnownCompo00HandleSpecAt(i);
        AppendAction(plan,
                     TitleRenderActionKind::BindCompo00Handle,
                     kFn801C609C,
                     spec.kind,
                     spec.firstHandle,
                     spec.lastHandle,
                     spec.memIndexBase,
                     spec.fromMemRecord ? 1u : 0u);
    }

    AppendAction(plan,
                 TitleRenderActionKind::Gap,
                 kFn801C609C,
                 TitleResourceKind::Unknown,
                 kCompo00HandleTable80091858);
    return plan;
}

TitleRenderPlan BuildTitleRenderInit801C609CPlan()
{
    TitleRenderPlan plan = MakePlan();
    plan.hasOpenP0Gap = true;

    AppendAction(plan, TitleRenderActionKind::InitWorkBuffers, kFn801C609C);
    AppendAction(plan, TitleRenderActionKind::InitOrderingTables, kFn801C609C);
    AppendAction(plan, TitleRenderActionKind::InitRenderGlobals, kFn801C609C);
    AppendAction(plan, TitleRenderActionKind::InitCoord, kFn801C609C);
    AppendAction(plan,
                 TitleRenderActionKind::SetProjection440,
                 kFn801C609C,
                 TitleResourceKind::Unknown,
                 kProjectionDepth);
    AppendPlan(plan, BuildTitleResourceMapPlan());
    AppendAction(plan, TitleRenderActionKind::InitDrawDescriptors,
                 kFn801C609C);
    AppendAction(plan, TitleRenderActionKind::InitTodCursor, kFn801C609C);
    AppendAction(plan, TitleRenderActionKind::InitMimeTodResources,
                 kFn801C609C);
    return plan;
}

TitleRenderPlan BuildTitleDraw801C5EF0Plan(bool dword801CB600Nonzero)
{
    TitleRenderPlan plan = MakePlan();
    plan.hasOpenP0Gap = true;

    AppendAction(plan,
                 TitleRenderActionKind::GateLoHpDrawSource801C5EF0,
                 kFn801C5EF0,
                 TitleResourceKind::Unknown,
                 kLoHpDrawGate801CB600,
                 dword801CB600Nonzero ? 1u : 0u,
                 0,
                 1,
                 true);
    AppendAction(plan, TitleRenderActionKind::Call801C5EF0, kFn801C5EF0);
    AppendAction(plan, TitleRenderActionKind::Call800141D8, kFn800141D8);
    AppendAction(plan,
                 TitleRenderActionKind::IncrementTodCursor801CB604,
                 kFn801C5EF0,
                 TitleResourceKind::Unknown,
                 kTodCursor801CB604,
                 1);
    AppendAction(plan,
                 TitleRenderActionKind::Call8001B000TodBuild,
                 kFn8001B000,
                 TitleResourceKind::PaLoc2Tod,
                 kTodCursor801CB604,
                 kTodAnim801CB658,
                 kTodBase801C9534,
                 kTodSubmitSource801CB65C);
    AppendAction(plan,
                 TitleRenderActionKind::Call8001B084SubmitPaLoc2Tod,
                 kFn8001B084,
                 TitleResourceKind::PaLoc2Tod,
                 kTodSubmitSource801CB65C,
                 1,
                 kPresentEnv801C9574,
                 kTodOrderingTableDepth);
    AppendAction(plan,
                 TitleRenderActionKind::IncrementTitleTodCursor801C9540,
                 kFn801C5EF0,
                 TitleResourceKind::Unknown,
                 kTitleTodCursor801C9540,
                 1);
    AppendAction(plan,
                 TitleRenderActionKind::Call80013EA8TitleTod,
                 kFn80013EA8,
                 TitleResourceKind::DatRange,
                 kTitleTodState801CF81C,
                 kTitleTodCursor801C9540,
                 2);
    AppendAction(plan,
                 TitleRenderActionKind::Call801C5E60TmdSubmit,
                 kFn801C5E60,
                 TitleResourceKind::PaKageTmd,
                 kTodSubmitSource801CB65C,
                 kTitleTmdDrawDesc801CD80C,
                 kPresentEnv801C9574,
                 kTitleTmdSubmitDepth);
    AppendAction(plan,
                 TitleRenderActionKind::Call800417A4CopyMatrix,
                 kFn800417A4,
                 TitleResourceKind::Unknown,
                 kTodSubmitSource801CB65C,
                 4);
    AppendAction(plan,
                 TitleRenderActionKind::Call80041A68BuildMatrix,
                 kFn80041A68,
                 TitleResourceKind::Unknown,
                 kMatrixBase801CD7BC);
    AppendAction(plan, TitleRenderActionKind::Call800406D8ConcatMatrix,
                 kFn800406D8);
    AppendAction(plan, TitleRenderActionKind::Call80040544ApplyMatrix,
                 kFn80040544);
    AppendAction(plan,
                 TitleRenderActionKind::Call800428B0PacketDispatch,
                 kFn800428B0,
                 TitleResourceKind::PaKageTmd,
                 kTitleTmdDrawDesc801CD80C,
                 kPresentEnv801C9574,
                 kTitleTmdPacketOtShift,
                 kScratchPacketBuffer1F800000);
    AppendAction(plan,
                 TitleRenderActionKind::Gate8001C1E8TmdFastTableInit,
                 kFn8001C1E8,
                 TitleResourceKind::Unknown,
                 kGsTmdFastHandlerTable8008EDD8,
                 kGsTmdFastFlag4F3Table8008EED8,
                 kGsTmdFastFlag4F4Table8008EEE4,
                 kGsTmdFastTnf3Slot8008EE30);
    AppendAction(plan,
                 TitleRenderActionKind::Gate8004274CPrimitiveGrouping,
                 kFn8004274C,
                 TitleResourceKind::Unknown,
                 kCompo00TitleTmdPrimitiveMode,
                 kCompo00TitleTmdPrimitiveBytes,
                 kCompo00TitleTmdPrimitiveDwordStride,
                 kTitleTmdDrawDesc801CD80C);
    AppendAction(plan,
                 TitleRenderActionKind::Gate800428B0Compo00PrimitiveCoverage,
                 kFn800428B0,
                 TitleResourceKind::Unknown,
                 kCompo00TitleTmdPrimitiveMode,
                 kCompo00TitleTmdFirstHandle,
                 kCompo00TitleTmdLastHandle,
                 kGsTmdFastTnf3Slot8008EE30);
    AppendAction(plan,
                 TitleRenderActionKind::Gate800428B0PacketHalBoundary,
                 kFn800428B0,
                 TitleResourceKind::Unknown,
                 kTitleTmdDrawDesc801CD80C,
                 kPresentEnv801C9574,
                 kTitleTmdPacketOtShift,
                 kScratchPacketBuffer1F800000);
    AppendAction(plan,
                 TitleRenderActionKind::OptionalLoHpDraw,
                 kFn801C5EF0,
                 TitleResourceKind::Unknown,
                 kLoHpDrawGate801CB600,
                 dword801CB600Nonzero ? 1u : 0u,
                 0,
                 0,
                 true);

    if (dword801CB600Nonzero) {
        AppendAction(plan,
                     TitleRenderActionKind::SetLoCursor801C9538,
                     kFn801C5EF0,
                     TitleResourceKind::Unknown,
                     kLoCursor801C9538,
                     1,
                     0,
                     0,
                     true);
        AppendAction(plan,
                     TitleRenderActionKind::Call80013EA8LoAnim,
                     kFn80013EA8,
                     TitleResourceKind::DatRange,
                     kLoAnim801CC66C,
                     0,
                     1,
                     0,
                     true);
        AppendAction(plan,
                     TitleRenderActionKind::Call8001B084SubmitLo,
                     kFn8001B084,
                     TitleResourceKind::LoTmd,
                     kLoSubmit801CB5F0,
                     1,
                     kPresentEnv801C9574,
                     kTodOrderingTableDepth,
                     true);
        AppendAction(plan,
                     TitleRenderActionKind::IncrementHpCursor801C953C,
                     kFn801C5EF0,
                     TitleResourceKind::Unknown,
                     kHpCursor801C953C,
                     1,
                     0,
                     0,
                     true);
        AppendAction(plan,
                     TitleRenderActionKind::Call80013EA8HpAnim,
                     kFn80013EA8,
                     TitleResourceKind::DatRange,
                     kHpAnim801CD71C,
                     kHpCursor801C953C,
                     3,
                     0,
                     true);
        AppendAction(plan,
                     TitleRenderActionKind::Call8001B084SubmitHp,
                     kFn8001B084,
                     TitleResourceKind::HpTmd,
                     kHpSubmit801CC70C,
                     1,
                     kPresentEnv801C9574,
                     kTodOrderingTableDepth,
                     true);
    }

    AppendAction(plan,
                 TitleRenderActionKind::Gap,
                 kFn800428B0,
                 TitleResourceKind::Unknown,
                 kTitleTmdPacketOtShift,
                 kScratchPacketBuffer1F800000);
    return plan;
}

TitleRenderPlan BuildTitleRenderFrame801C6410Plan(
    uint32_t ctxFlags,
    bool dword801CB600Nonzero)
{
    TitleRenderPlan plan = MakePlan();
    plan.hasOpenP0Gap = true;

    AppendAction(plan,
                 TitleRenderActionKind::GateTitleFrameSource801C6410,
                 kFn801C6410,
                 TitleResourceKind::Unknown,
                 ctxFlags,
                 kRuntimeTimFlag,
                 kRuntimeTimListCtxOffsetACh,
                 kRuntimeTimListLaneCount801C6410,
                 true);
    AppendAction(plan,
                 TitleRenderActionKind::Call801C6410Frame,
                 kFn801C6410,
                 TitleResourceKind::Unknown,
                 ctxFlags);

    if ((ctxFlags & kCtxFlagTitleRenderActive) != 0) {
        AppendAction(plan,
                     TitleRenderActionKind::SetTitleRenderActiveFromCtxFlag,
                     kFn801C6410,
                     TitleResourceKind::Unknown,
                     kTitleRenderActive801C9548,
                     1,
                     kCtxFlagTitleRenderActive,
                     ctxFlags,
                     true);
    }

    if ((ctxFlags & kCtxFlagShortcutStart) != 0) {
        AppendAction(plan,
                     TitleRenderActionKind::SetPresentExtraFromCtxFlag,
                     kFn801C6410,
                     TitleResourceKind::Unknown,
                     kPresentExtraGate801CFAEC,
                     1,
                     kCtxFlagShortcutStart,
                     ctxFlags,
                     true);
        AppendAction(plan,
                     TitleRenderActionKind::SetTitleRenderActiveFromCtxFlag,
                     kFn801C6410,
                     TitleResourceKind::Unknown,
                     kTitleRenderActive801C9548,
                     0,
                     kCtxFlagShortcutStart,
                     ctxFlags,
                     true);
    }

    if ((ctxFlags & kCtxFlagShortcutReady) != 0) {
        AppendAction(plan,
                     TitleRenderActionKind::ClearPresentExtraFromCtxFlag,
                     kFn801C6410,
                     TitleResourceKind::Unknown,
                     kPresentExtraGate801CFAEC,
                     0,
                     kCtxFlagShortcutReady,
                     ctxFlags,
                     true);
        AppendAction(plan,
                     TitleRenderActionKind::SetLoHpDrawGateFromCtxFlag,
                     kFn801C6410,
                     TitleResourceKind::Unknown,
                     kLoHpDrawGate801CB600,
                     1,
                     kCtxFlagShortcutReady,
                     ctxFlags,
                     true);
        AppendAction(plan,
                     TitleRenderActionKind::SetTitleRenderActiveFromCtxFlag,
                     kFn801C6410,
                     TitleResourceKind::Unknown,
                     kTitleRenderActive801C9548,
                     1,
                     kCtxFlagShortcutReady,
                     ctxFlags,
                     true);
    }

    if ((ctxFlags & kRuntimeTimFlag) != 0) {
        AppendAction(plan,
                     TitleRenderActionKind::GateRuntimeTimListSource801C6410,
                     kFn801C6410,
                     TitleResourceKind::TitleTimRange,
                     kRuntimeTimListCtxOffsetACh,
                     kRuntimeTimListLaneCount801C6410,
                     kCompo00HandleTable80091858,
                     kRuntimeTimUploadFlag801C6410,
                     true);
        AppendAction(plan,
                     TitleRenderActionKind::UploadRuntimeTimList,
                     kFn801C6410,
                     TitleResourceKind::TitleTimRange,
                     kRuntimeTimListCtxOffsetACh,
                     kRuntimeTimListLaneCount801C6410);
        AppendAction(plan,
                     TitleRenderActionKind::Call8001ADECUploadTim,
                     kFn8001ADEC,
                     TitleResourceKind::TitleTimRange,
                     kCompo00HandleTable80091858,
                     kRuntimeTimUploadFlag801C6410);
        AppendAction(plan,
                     TitleRenderActionKind::GateRuntimeTimUploadHalBoundary8001ADEC,
                     kFn8001ADEC,
                     TitleResourceKind::TitleTimRange,
                     kRuntimeTimPayloadHeaderBytes,
                     kFn80044D64,
                     kFn800431E0,
                     kRuntimeTimHasClutMask,
                     true);
    }

    AppendPlan(plan, BuildTitleDraw801C5EF0Plan(dword801CB600Nonzero));
    AppendAction(plan,
                 TitleRenderActionKind::Gap,
                 kFn801C6410,
                 TitleResourceKind::Unknown,
                 ctxFlags,
                 kRuntimeTimListCtxOffsetACh,
                 kRuntimeTimListLaneCount801C6410);
    return plan;
}

TitleRenderPlan BuildTitlePresent801C689CPlan(bool dword801CFAECNonzero)
{
    TitleRenderPlan plan = MakePlan();
    plan.hasOpenP0Gap = true;

    AppendAction(plan, TitleRenderActionKind::Call801C689C, kFn801C689C);
    AppendAction(plan, TitleRenderActionKind::Call80040370Flip, kFn80040370);
    AppendAction(plan,
                 TitleRenderActionKind::Call80040060SetEnv,
                 kFn80040060,
                 TitleResourceKind::Unknown,
                 kPresentColorChannelMax,
                 kPresentColorChannelMax,
                 kPresentColorChannelMax,
                 kPresentEnv801C9574);
    AppendAction(plan, TitleRenderActionKind::Call80040CA4DrawOtag,
                 kFn80040CA4);
    AppendAction(plan, TitleRenderActionKind::Call8001E3B0Clear,
                 kFn8001E3B0, TitleResourceKind::Unknown,
                 kPresentClearEnvIndex,
                 kPresentClearEnvTable80087288,
                 kPresentEnvRecordBytes);
    AppendAction(plan,
                 TitleRenderActionKind::GatePresentMainHalBoundary801C689C,
                 kFn801C689C,
                 TitleResourceKind::Unknown,
                 kPresentEnv801C9574,
                 kPresentEnvOtagOffset10h,
                 kFn800450A0,
                 kPresentClearEnvTable80087288);
    AppendAction(plan,
                 TitleRenderActionKind::GatePresentExtraSource801C689C,
                 kFn801C689C,
                 TitleResourceKind::Unknown,
                 kPresentExtraGate801CFAEC,
                 dword801CFAECNonzero ? 1u : 0u,
                 0,
                 1,
                 true);
    AppendAction(plan,
                 TitleRenderActionKind::Call80040370OptionalExtraFlip,
                 kFn80040370,
                 TitleResourceKind::Unknown,
                 kPresentExtraGate801CFAEC,
                 dword801CFAECNonzero ? 1u : 0u,
                 0,
                 0,
                 true);
    AppendAction(plan,
                 TitleRenderActionKind::Call8001B1B0Optional,
                 kFn8001B1B0,
                 TitleResourceKind::Unknown,
                 kPresentColorChannelMax,
                 kPresentColorChannelMax,
                 kPresentColorChannelMax,
                 kPresentExtraGate801CFAEC,
                 true);
    AppendAction(plan,
                 TitleRenderActionKind::GatePresentExtraFillHalBoundary80044CD0,
                 kFn80044CD0,
                 TitleResourceKind::Unknown,
                 kPresentExtraFillX,
                 kPresentExtraFillY,
                 kPresentExtraFillWidth,
                 kPresentExtraFillHeight,
                 true);
    AppendAction(plan,
                 TitleRenderActionKind::Gap,
                 kFn801C689C,
                 TitleResourceKind::Unknown,
                 kPresentExtraGate801CFAEC);
    return plan;
}

const char* TitleResourceKindName(TitleResourceKind kind)
{
    switch (kind) {
    case TitleResourceKind::Unknown:
        return "Unknown";
    case TitleResourceKind::VabMinimum:
        return "VabMinimum";
    case TitleResourceKind::PaLoc2Tod:
        return "PaLoc2Tod";
    case TitleResourceKind::HpTmd:
        return "HpTmd";
    case TitleResourceKind::LoTmd:
        return "LoTmd";
    case TitleResourceKind::PaTmd:
        return "PaTmd";
    case TitleResourceKind::PaKageTmd:
        return "PaKageTmd";
    case TitleResourceKind::VdfRange:
        return "VdfRange";
    case TitleResourceKind::DatRange:
        return "DatRange";
    case TitleResourceKind::CmOpBez:
        return "CmOpBez";
    case TitleResourceKind::TitleTimRange:
        return "TitleTimRange";
    }
    return "Unknown";
}

const char* TitleRawTableKindName(TitleRawTableKind kind)
{
    switch (kind) {
    case TitleRawTableKind::Unknown:
        return "Unknown";
    case TitleRawTableKind::ResourcePairs:
        return "ResourcePairs";
    case TitleRawTableKind::HudTimTimelines:
        return "HudTimTimelines";
    case TitleRawTableKind::EventStream:
        return "EventStream";
    case TitleRawTableKind::Selector:
        return "Selector";
    }
    return "Unknown";
}

const char* TitleRenderActionKindName(TitleRenderActionKind kind)
{
    switch (kind) {
    case TitleRenderActionKind::None:
        return "None";
    case TitleRenderActionKind::InitWorkBuffers:
        return "InitWorkBuffers";
    case TitleRenderActionKind::InitOrderingTables:
        return "InitOrderingTables";
    case TitleRenderActionKind::InitRenderGlobals:
        return "InitRenderGlobals";
    case TitleRenderActionKind::InitCoord:
        return "InitCoord";
    case TitleRenderActionKind::SetProjection440:
        return "SetProjection440";
    case TitleRenderActionKind::BindCompo00Handle:
        return "BindCompo00Handle";
    case TitleRenderActionKind::InitDrawDescriptors:
        return "InitDrawDescriptors";
    case TitleRenderActionKind::InitTodCursor:
        return "InitTodCursor";
    case TitleRenderActionKind::InitMimeTodResources:
        return "InitMimeTodResources";
    case TitleRenderActionKind::GateTitleFrameSource801C6410:
        return "GateTitleFrameSource801C6410";
    case TitleRenderActionKind::Call801C6410Frame:
        return "Call801C6410Frame";
    case TitleRenderActionKind::SetTitleRenderActiveFromCtxFlag:
        return "SetTitleRenderActiveFromCtxFlag";
    case TitleRenderActionKind::SetPresentExtraFromCtxFlag:
        return "SetPresentExtraFromCtxFlag";
    case TitleRenderActionKind::ClearPresentExtraFromCtxFlag:
        return "ClearPresentExtraFromCtxFlag";
    case TitleRenderActionKind::SetLoHpDrawGateFromCtxFlag:
        return "SetLoHpDrawGateFromCtxFlag";
    case TitleRenderActionKind::GateRuntimeTimListSource801C6410:
        return "GateRuntimeTimListSource801C6410";
    case TitleRenderActionKind::UploadRuntimeTimList:
        return "UploadRuntimeTimList";
    case TitleRenderActionKind::Call8001ADECUploadTim:
        return "Call8001ADECUploadTim";
    case TitleRenderActionKind::GateRuntimeTimUploadHalBoundary8001ADEC:
        return "GateRuntimeTimUploadHalBoundary8001ADEC";
    case TitleRenderActionKind::Call801C5EF0:
        return "Call801C5EF0";
    case TitleRenderActionKind::Call800141D8:
        return "Call800141D8";
    case TitleRenderActionKind::Call8001B000TodTick:
        return "Call8001B000TodTick";
    case TitleRenderActionKind::Call8001B084SubmitTod:
        return "Call8001B084SubmitTod";
    case TitleRenderActionKind::Call80013EA8:
        return "Call80013EA8";
    case TitleRenderActionKind::Call801C5E60:
        return "Call801C5E60";
    case TitleRenderActionKind::Call800428B0TmdSubmit:
        return "Call800428B0TmdSubmit";
    case TitleRenderActionKind::IncrementTodCursor801CB604:
        return "IncrementTodCursor801CB604";
    case TitleRenderActionKind::Call8001B000TodBuild:
        return "Call8001B000TodBuild";
    case TitleRenderActionKind::Call8001B084SubmitPaLoc2Tod:
        return "Call8001B084SubmitPaLoc2Tod";
    case TitleRenderActionKind::IncrementTitleTodCursor801C9540:
        return "IncrementTitleTodCursor801C9540";
    case TitleRenderActionKind::Call80013EA8TitleTod:
        return "Call80013EA8TitleTod";
    case TitleRenderActionKind::Call801C5E60TmdSubmit:
        return "Call801C5E60TmdSubmit";
    case TitleRenderActionKind::Call800417A4CopyMatrix:
        return "Call800417A4CopyMatrix";
    case TitleRenderActionKind::Call80041A68BuildMatrix:
        return "Call80041A68BuildMatrix";
    case TitleRenderActionKind::Call800406D8ConcatMatrix:
        return "Call800406D8ConcatMatrix";
    case TitleRenderActionKind::Call80040544ApplyMatrix:
        return "Call80040544ApplyMatrix";
    case TitleRenderActionKind::Call800428B0PacketDispatch:
        return "Call800428B0PacketDispatch";
    case TitleRenderActionKind::Gate8001C1E8TmdFastTableInit:
        return "Gate8001C1E8TmdFastTableInit";
    case TitleRenderActionKind::Gate8004274CPrimitiveGrouping:
        return "Gate8004274CPrimitiveGrouping";
    case TitleRenderActionKind::Gate800428B0Compo00PrimitiveCoverage:
        return "Gate800428B0Compo00PrimitiveCoverage";
    case TitleRenderActionKind::Gate800428B0PacketHalBoundary:
        return "Gate800428B0PacketHalBoundary";
    case TitleRenderActionKind::SetLoCursor801C9538:
        return "SetLoCursor801C9538";
    case TitleRenderActionKind::Call80013EA8LoAnim:
        return "Call80013EA8LoAnim";
    case TitleRenderActionKind::Call8001B084SubmitLo:
        return "Call8001B084SubmitLo";
    case TitleRenderActionKind::IncrementHpCursor801C953C:
        return "IncrementHpCursor801C953C";
    case TitleRenderActionKind::Call80013EA8HpAnim:
        return "Call80013EA8HpAnim";
    case TitleRenderActionKind::Call8001B084SubmitHp:
        return "Call8001B084SubmitHp";
    case TitleRenderActionKind::GateLoHpDrawSource801C5EF0:
        return "GateLoHpDrawSource801C5EF0";
    case TitleRenderActionKind::OptionalLoHpDraw:
        return "OptionalLoHpDraw";
    case TitleRenderActionKind::Call801C689C:
        return "Call801C689C";
    case TitleRenderActionKind::Call80040370Flip:
        return "Call80040370Flip";
    case TitleRenderActionKind::Call80040060SetEnv:
        return "Call80040060SetEnv";
    case TitleRenderActionKind::Call80040CA4DrawOtag:
        return "Call80040CA4DrawOtag";
    case TitleRenderActionKind::Call8001E3B0Clear:
        return "Call8001E3B0Clear";
    case TitleRenderActionKind::GatePresentMainHalBoundary801C689C:
        return "GatePresentMainHalBoundary801C689C";
    case TitleRenderActionKind::GatePresentExtraSource801C689C:
        return "GatePresentExtraSource801C689C";
    case TitleRenderActionKind::Call80040370OptionalExtraFlip:
        return "Call80040370OptionalExtraFlip";
    case TitleRenderActionKind::Call8001B1B0Optional:
        return "Call8001B1B0Optional";
    case TitleRenderActionKind::GatePresentExtraFillHalBoundary80044CD0:
        return "GatePresentExtraFillHalBoundary80044CD0";
    case TitleRenderActionKind::Gap:
        return "Gap";
    }
    return "Unknown";
}

} // namespace PrSS0TitleRenderDirect
