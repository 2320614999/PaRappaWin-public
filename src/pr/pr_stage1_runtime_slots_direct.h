#pragma once

#include "pr_stage_event_direct.h"
#include "pr_stage1_overlay_parser.h"
#include "pr_stage_scene_submit_constants.h"

#include <array>
#include <cstdint>

struct PrStage1RailCursorEventDirectResult;
struct PrStage1RuntimeSlotsSnapshot;

struct PrStage1RuntimeSlotsDirectPairSlot {
    bool valid = false;
    uint8_t pairIndex = 0;
    uint16_t datHandle = 0;
    uint16_t vdfHandle = 0;
    uint32_t startFrame = 0;
    uint32_t sourceEventPsxAddr = 0;
    bool directResourceHandleOverride = false;
};

struct PrStage1RuntimeSlotsDirectCameraSlot {
    bool valid = false;
    int16_t bezId = 0;
    uint32_t startFrame = 0;
    uint32_t sourceEventPsxAddr = 0;
};

struct PrStage1RuntimeSlotsDirectPerformanceTodSlot {
    bool valid = false;
    uint8_t handle = 0;
    uint8_t sourceStreamId = 0;
    uint32_t startFrame = 0;
    uint32_t sourceEventPsxAddr = 0;
};

struct PrStage1RuntimeSlotsDirectSceneSlot {
    bool valid = false;
    uint8_t selectedRow = 0;
    uint8_t selectedStream = 0;
    bool sharedTodListEnabled = false;
    uint32_t streamStartFrame = 0;
    uint32_t streamWindowFrames = 0;
    std::array<uint8_t, PrStageSceneSubmitConstants::kDynamicRowSlotCount>
        rowHandles{};
};

struct PrStage1RuntimeSlotsDirectDescBankSelectorInput {
    bool selectorValid = false;
    uint8_t selector = 0;
    uint32_t sourceFrame = 0;
    uint32_t ctxFlagsPulse801CBFDC = 0;
    bool uiRenderBlock4CC4Valid = false;
    int16_t uiRenderBlock4CC4 = 0;
    uint32_t uiRenderBlock4CC4Frame = 0;
    bool cameraValid9443C = false;
    uint16_t cameraBezHandle9443C = 0;
    uint32_t cameraSourceFrame9443C = 0;
    bool acceptedTailTimerValid9443C = false;
    int32_t acceptedTailTimerFrames9443C = 0;
    uint32_t acceptedTailTimerSourceFrame9443C = 0;
    bool hudOverlayConfigure801CA0E0Valid = false;
    uint8_t hudOverlayConfigure801CA0E0Arg = 0;
    uint32_t hudOverlayConfigure801CA0E0SourceFrame = 0;
    uint32_t hudOverlayConfigure801CA0E0SourceScriptFrame = 0;
    bool compactPainterGate7AValid = false;
    uint16_t compactPainterGate7A = 0;
    bool compactRailRowCount8AClear80024FD0 = false;
};

struct PrStage1RuntimeSlotsDirectRatingPresentationCarrier801CBFDC {
    bool flag0200FrameUpdateBranchTaken = false;
    bool flag0200FrameUpdateResourcePairKnown = false;
    bool flag0200FrameUpdateCtxFlag20000Set = false;
    uint16_t flag0200FrameUpdateCtxE0ResourceIndex = 0;
    uint16_t flag0200FrameUpdateCtxECResourceIndex = 0;
    bool helper801CBFDCPseudoCKnown = false;
    uint32_t helper801CBFDCPsxAddr = 0x801CBFDCu;
};

struct PrStage1RuntimeSlotsDirectDescBankCarrier801CBFDC {
    uint32_t queryFrame = 0;
    bool selectorValid = false;
    uint8_t selector = 0;
    uint32_t selectorQueryFrame = 0;
    uint32_t ctxFlagsPulse801CBFDC = 0;
    uint8_t rightRankActiveRow = 0;
    bool cameraPulse9443C = false;
    uint16_t cameraBezHandle9443C = 0;
    uint32_t cameraQueryFrame9443C = 0;
    bool acceptedTailTimer9443CPulse = false;
    int32_t acceptedTailTimer9443CFrames = 0;
    uint32_t acceptedTailTimer9443CQueryFrame = 0;
    bool hudOverlayConfigure801CA0E0Pulse = false;
    uint8_t hudOverlayConfigure801CA0E0Arg = 0;
    uint32_t hudOverlayConfigure801CA0E0QueryFrame = 0;
    uint32_t hudOverlayConfigure801CA0E0ScriptFrame = 0;
    uint16_t compactPainterGate7A = 0;
    bool compactRailRowCount8AClear80024FD0 = false;
    bool helper801CBFDCPseudoCKnown = false;
    uint32_t helper801CBFDCPsxAddr = 0x801CBFDCu;
};

struct PrStage1RuntimeSlotsDirectFollowUpFacts801CBFDC {
    bool flag0200FrameUpdateBranchTaken = false;
    bool flag0200FrameUpdateResourcePairKnown = false;
    bool flag0200FrameUpdateCtxFlag20000Set = false;
    uint16_t flag0200FrameUpdateCtxE0ResourceIndex = 0;
    uint16_t flag0200FrameUpdateCtxECResourceIndex = 0;
    bool descBankSelectorPulse = false;
    uint8_t descBankSelector = 0;
    uint32_t descBankSelectorQueryFrame = 0;
    uint32_t descBankCtxFlagsPulse801CBFDC = 0;
    uint8_t rightRankActiveRow = 0;
    bool cameraPulse9443C = false;
    uint16_t cameraBezHandle9443C = 0;
    uint32_t cameraQueryFrame9443C = 0;
    bool acceptedTailTimer9443CPulse = false;
    int32_t acceptedTailTimer9443CFrames = 0;
    uint32_t acceptedTailTimer9443CQueryFrame = 0;
    bool hudOverlayConfigure801CA0E0Pulse = false;
    uint8_t hudOverlayConfigure801CA0E0Arg = 0;
    uint32_t hudOverlayConfigure801CA0E0QueryFrame = 0;
    uint32_t hudOverlayConfigure801CA0E0ScriptFrame = 0;
    uint16_t compactPainterGate7A = 0;
    bool compactRailRowCount8AClear80024FD0 = false;
    bool ctxFlagTickAdvanceKnown801C9094 = false;
    bool ctxFlagTickAdvance801C9094 = false;
};

struct PrStage1RuntimeSlotsDirectDescBankRuntime {
    bool active = false;
    uint8_t activeSelector = 0;
    uint32_t startFrame = 0;
    std::array<bool, PrStageSceneSubmitConstants::kDescBankGroupCount>
        activeGroups{};
    std::array<uint32_t, PrStageSceneSubmitConstants::kDescBankGroupCount>
        groupStartFrames{};
    int32_t dword801D3058AcceptedTailGateFrames = 0;
    uint32_t dword801D3058AcceptedTailStartFrame = 0;
    uint32_t dword801D3058AcceptedTailSourceEventPsxAddr = 0;
};

struct PrStage1RuntimeSlotsDirectTamaSelectorRuntime {
    bool valid = false;
    uint8_t selector = 0;
    uint32_t startFrame = 0;
    uint32_t sourceEventPsxAddr = 0;
};

struct PrStage1RuntimeSlotsDirectCompactRailRow80024744 {
    int16_t teacherRaw8C = -1;
    int16_t studentRaw9E = -1;
    uint32_t bodyStreamPtr94 = 0;
    std::array<int8_t, 18> bodyStreamBytes94{};
};

struct PrStage1RuntimeSlotsDirectCompactRailRuntime80024744 {
    bool valid = false;
    int16_t painterGate7A = 0;
    int16_t rowCount8A = 0;
    std::array<PrStage1RuntimeSlotsDirectCompactRailRow80024744, 2> rows{};
};

struct PrStage1RuntimeSlotsDirectCompactProducer801C9094 {
    bool active = false;
    uint32_t sourceEventPsxAddr = 0;
    uint32_t sourceFrame = 0;
    uint8_t teacherRecordIndex = 0;
    uint8_t studentRecordIndex = 0;
    // PSX 801C9094 ctx+0x8A/0x8C/0x8E/0x90, teacher rail cursor state.
    int16_t teacherMode8A = 0;
    int16_t teacherCursor8C = -1;
    int16_t teacherCursor8E = -1;
    int16_t teacherState90 = 0;
    // PSX 801C9094 ctx+0x94/0x98, rec44 stream A/B pointers.
    uint32_t teacherBodyPtr94 = 0;
    uint32_t teacherBodyPtr98 = 0;
    // PSX 801C9094 caches dword_801D304C/801D3050 after teacher bootstrap;
    // the student catchup branch copies these into ctx+0xA4/0xA8.
    uint32_t cachedTeacherBodyPtr801D304C = 0;
    uint32_t cachedTeacherBodyPtr801D3050 = 0;
    bool cachedTeacherHead0Negative801C9094 = false;
    // PSX 801C9094 ctx+0x9E/0xA0/0xA2, student rail cursor state.
    int16_t studentCursor9E = -1;
    int16_t studentCursorA0 = -1;
    int16_t studentStateA2 = 0;
    // PSX 801C9094 ctx+0xA4/0xA8, student stream pointers.
    uint32_t studentBodyPtrA4 = 0;
    uint32_t studentBodyPtrA8 = 0;
    std::array<int8_t, 20> teacherBodyStream94{};
    std::array<int8_t, 20> teacherBodyStream98{};
    std::array<int8_t, 20> studentBodyStreamA4{};
    std::array<int8_t, 20> studentBodyStreamA8{};
};

struct PrStage1RuntimeSlotsDirectSceneSubmitRuntime {
    bool uiRenderBlock4CC4Valid = false;
    int16_t uiRenderBlock4CC4 = 0;
    uint32_t uiRenderBlock4CC4Frame = 0;
    uint32_t ctxFlagsFull = 0;
    bool ctxFlagTickAdvanceKnown801C9094 = false;
    bool ctxFlagTickAdvance801C9094 = false;
    uint32_t ctxFlagsPulse801CBFDC = 0;
    uint16_t ctxFlags40_2000_4000 = 0;
    uint16_t ctx72 = 0;
    uint16_t ctx11C = 0;
    uint16_t ctx120 = 0;
    bool flag2000DynamicRowReload801CBFDC = false;
    bool sceneInitPulse801CB6BC = false;
    bool compactWobbleResetPulse80024744 = false;
    bool compactTeacherResetPulse80024308 = false;
    bool compactStudentResetPulse80024390 = false;
    bool renderCompactWobbleResetPulse80024744 = false;
    bool renderCompactTeacherResetPulse80024308 = false;
    bool renderCompactStudentResetPulse80024390 = false;
    PrStage1RuntimeSlotsDirectCompactProducer801C9094
        compactProducer801C9094{};
    PrStage1RuntimeSlotsDirectCompactRailRuntime80024744
        compactRail80024744{};
    PrStage1RuntimeSlotsDirectCompactRailRuntime80024744
        renderCompactRail80024744{};
};

struct PrStage1RuntimeSlotsDirectScriptCtxMirrorRuntime {
    bool valid = false;
    uint32_t sourceEventPsxAddr = 0;
    uint32_t dueFrame = 0;
    uint32_t ctxFlagsSet = 0;
    uint16_t paDatCtxOffset = 0;
    uint16_t paVdfCtxOffset = 0;
    uint16_t onDatCtxOffset = 0;
    uint16_t onVdfCtxOffset = 0;
    uint16_t performanceTodCtxOffset = 0;
    uint16_t row0TodCtxOffset = 0;
    uint16_t tamaSelectorCtxOffset = 0;
    uint16_t cameraBezCtxOffset = 0;
};

struct PrStage1RuntimeSlotsDirectHudTimedListChannel {
    bool active = false;
    uint8_t slotId = 0;
    uint32_t baseFrame = 0;
    uint32_t sourceEventPsxAddr = 0;
};

struct PrStage1RuntimeSlotsDirectHudTimUpload {
    bool valid = false;
    uint8_t channel = 0;
    uint8_t slotId = 0;
    uint32_t eventIndex = 0;
    uint32_t dueFrame = 0;
    uint32_t sourceEventPsxAddr = 0;
    std::array<uint16_t, 4> timIds{};
};

struct PrStage1RuntimeSlotsDirectCtxInit80024E98 {
    // PSX sub_80024E98 clears unk_801C3640 for 488 bytes, then seeds these
    // confirmed fields. Offsets are relative to ctx base 0x801C3640.
    uint32_t base801C3640 = 0x801C3640u;
    uint32_t clearSizeBytes80024E98 = 488u;
    uint32_t dword800943D0 = 0x801CFA54u;
    uint16_t word4E_801C368E = 1;
    uint16_t word50_801C3690 = 0;
    uint16_t word52_801C3692 = 0;
    uint16_t word5A_801C369A = 0; // sub_80024F8C: *(a1+90)=0.
    uint16_t word6A_801C36AA = 1;
    uint16_t word7A_801C36BA = 1;
    uint16_t word8A_801C36CA = 0;
    uint16_t word90_801C36D0 = 0;
    uint16_t wordA2_801C36E2 = 0;
    uint32_t dword94_801C36D4 = 0x801CFA58u;
    uint32_t dword98_801C36D8 = 0x801CFA6Cu;
    uint16_t word800916DC = 0;
    uint16_t word68_801C36A8 = 0;
    uint16_t word54_801C3694 = 0;
    bool ptr800943D0SeedGap = false;  // dword_800943D0 + 4 / +24.
    bool word800916DCSeedGap = true;  // false only when caller supplies seed.
};

struct PrStage1RuntimeSlotsDirectSteadyCtx801C7A60 {
    bool valid = false;
    bool word800916DCKnown = false;
    uint16_t word800916DC = 0;
    bool word68Known = false;
    uint16_t word68_801C36A8 = 0;
    bool word54Known = false;
    uint16_t word54_801C3694 = 0;
    bool word54ClearedOnExit = false;
};

struct PrStage1RuntimeSlotsDirectResetInput80024E98 {
    uint32_t dword800943D0 = 0x801CFA54u;
    uint16_t word800916DC = 0;
    bool word800916DCKnown = false;
};

struct PrStage1RuntimeSlotsDirectGlobalOptionsCarrier {
    bool subtitleEnabled = false;
    bool known = false;
    uint16_t word800916DC = 0;
    bool word800916DCKnown = false;
};

struct PrStage1RuntimeSlotsDirectRuntime {
    bool active = false;
    int32_t lastQueryFrame = -1;
    int32_t lastScriptFrame = -1;
    uint32_t queryFrame = 0;
    uint32_t scriptFrame = 0;
    uint8_t mode = 0xFF;
    uint16_t word5A_801C369A = 0;    // sub_80024F8C clears ctx+0x5A.
    uint32_t lastAppliedEventPsxAddr = 0;
    uint32_t lastAppliedEventDueFrame = 0;
    bool activeEventAvailable = false;
    uint8_t activeEventStreamId = 0;
    uint32_t activeEventIndex = 0;
    uint32_t activeEventPsxAddr = 0;
    uint32_t activeEventDueFrame = 0;
    bool prepassEventAvailable = false;
    uint8_t prepassEventStreamId = 0;
    uint32_t prepassEventIndex = 0;
    uint32_t prepassEventPsxAddr = 0;
    uint32_t prepassEventDueFrame = 0;
    int32_t word8008ECFCResource10000TimerFrames = 0;
    int32_t word8008ECFEResource20000TimerFrames = 0;
    int32_t dword801D3054ScriptEventOnByModeGateFrames = 0;
    PrStage1RuntimeSlotsDirectCtxInit80024E98 ctxInit80024E98{};
    PrStage1RuntimeSlotsDirectSteadyCtx801C7A60 steadyCtx801C7A60{};
    PrStageEventDirectResetAction80024F8C resetAction80024F8C{};
    PrStage1RuntimeSlotsDirectPairSlot onMotion{};
    PrStage1RuntimeSlotsDirectPairSlot paMotion{};
    PrStage1RuntimeSlotsDirectCameraSlot camera{};
    PrStage1RuntimeSlotsDirectPerformanceTodSlot performanceTod{};
    PrStage1RuntimeSlotsDirectPerformanceTodSlot row0Tod{};
    PrStage1RuntimeSlotsDirectSceneSlot scene{};
    PrStage1RuntimeSlotsDirectDescBankRuntime descBank{};
    PrStage1RuntimeSlotsDirectTamaSelectorRuntime tamaSelector{};
    PrStage1RuntimeSlotsDirectSceneSubmitRuntime sceneSubmit{};
    PrStage1RuntimeSlotsDirectScriptCtxMirrorRuntime scriptCtxMirror{};
    PrStageEventDirectStage1ConsumedEventSource801C9094
        lastConsumedEventSource801C9094 =
            PrStageEventDirectStage1ConsumedEventSource801C9094::None;
    PrStageEventDirectStage1ConsumedScriptEvent801C9094
        lastFlagStreamEvent801C9094{};
    PrStageEventDirectStage1ConsumedScriptEvent801C9094
        lastIdStreamEvent801C9094{};
    PrStageEventDirectStage1ConsumedScriptEvent801C9094
        lastConsumedEvent801C9094{};
    PrStageEventDirectStage1ConsumedScriptEvent801C9094
        lastCompactEvent801C9094{};
    std::array<PrStage1RuntimeSlotsDirectHudTimedListChannel, 3> hudTimedLists{};
    std::array<uint32_t, PrStage1OverlayData::kHudSlotCount>
        hudTimedListDescCursors{};
    std::array<PrStage1RuntimeSlotsDirectHudTimUpload, 3> hudTimUploads{};
    bool cameraPlaybackEnabled3034 = false;
};

struct PrStage1RuntimeSlotsDirectLateBranchInput {
    uint8_t selectedStream = 0;
    uint16_t ctxFlags40_2000_4000 = 0;
    uint16_t ctx72 = 0;
    int32_t ed00 = 0; // PSX unk_8008ED00; kept for caller compatibility.
    int32_t ed24 = 0;
    bool eventStreamFlagActive = false;
    uint8_t eventStreamFlagStream = 0;
    uint8_t activeDispatchStream = 0;
    uint32_t activeDispatchStartFrame = 0;
    uint32_t activeDispatchStartScriptFrame = 0;
    uint32_t activeDispatchEndLocalFrame = 0;
    bool flag2000SceneFamilyActive = false;
    uint8_t flag2000SceneFamilyStream = 0;
    uint32_t flag2000SceneFamilyStartFrame = 0;
    uint32_t flag2000SceneFamilyStartScriptFrame = 0;
    bool eventStreamDonePulse = false;
    uint8_t eventStreamDoneSourceStream = 0;
    PrStageEventDirectStage1ConsumedScriptEvent801C9094
        flagStreamEvent801C9094{};
    PrStageEventDirectStage1ConsumedScriptEvent801C9094
        idStreamEvent801C9094{};
    PrStageEventDirectStage1ConsumedEventSource801C9094 consumedEventSource =
        PrStageEventDirectStage1ConsumedEventSource801C9094::None;
    PrStageEventDirectStage1ConsumedScriptEvent801C9094
        consumedEvent801C9094{};
    PrStageEventDirectStage1ConsumedScriptEvent801C9094
        compactEvent801C9094{};
    bool flag40EarlyReturnReset801C9094 = false;
};

PrStage1RuntimeSlotsDirectLateBranchInput
PrStage1RuntimeSlotsDirectBuildEventStreamInput9094(
    const PrStageEventDirectStage1Runtime& runtime);

struct PrStage1RuntimeSlotsDirectRatingPresentationInput {
    bool flag0200FrameUpdateBranchTaken = false;
    bool flag0200FrameUpdateResourcePairKnown = false;
    bool flag0200FrameUpdateCtxFlag20000Set = false;
    uint16_t flag0200FrameUpdateCtxE0ResourceIndex = 0;
    uint16_t flag0200FrameUpdateCtxECResourceIndex = 0;
};

PrStage1RuntimeSlotsDirectRatingPresentationInput
PrStage1RuntimeSlotsDirectBuildRatingPresentationInput801CBFDC(
    const PrStage1RuntimeSlotsDirectRatingPresentationCarrier801CBFDC&
        carrier);

PrStage1RuntimeSlotsDirectDescBankSelectorInput
PrStage1RuntimeSlotsDirectBuildDescBankSelectorInput801CBFDC(
    const PrStage1RuntimeSlotsDirectDescBankCarrier801CBFDC& carrier);

struct PrStage1RuntimeSlotsDirectAdvanceInput {
    uint32_t queryFrame = 0;
    uint32_t scriptFrame = 0;
    uint8_t currentMode = 0xFF;
    bool word800916DCKnown801C7A60 = false;
    uint16_t word800916DC801C7A60 = 0;
    bool ctxFlagTickAdvanceKnown801C9094 = false;
    bool ctxFlagTickAdvance801C9094 = false;
    PrStage1RuntimeSlotsDirectLateBranchInput lateBranch{};
    PrStage1RuntimeSlotsDirectRatingPresentationInput ratingPresentation{};
    PrStage1RuntimeSlotsDirectDescBankSelectorInput descBank{};
    const PrStage1RailCursorEventDirectResult* railCursorEvent801C9094 =
        nullptr;
};

struct PrStage1RuntimeSlotsDirectAdvanceCarrier801C9094 {
    uint32_t queryFrame = 0;
    uint32_t scriptFrame = 0;
    uint8_t currentMode = 0xFF;
    bool word800916DCKnown801C7A60 = false;
    uint16_t word800916DC801C7A60 = 0;
    bool ctxFlagTickAdvanceKnown801C9094 = false;
    bool ctxFlagTickAdvance801C9094 = false;
    const PrStageEventDirectStage1Runtime* eventStreamRuntime801C9094 =
        nullptr;
    PrStage1RuntimeSlotsDirectRatingPresentationCarrier801CBFDC
        ratingPresentation{};
    PrStage1RuntimeSlotsDirectDescBankCarrier801CBFDC descBank{};
};

struct PrStage1RuntimeSlotsDirectActiveEventResult {
    const PrStage1EventStream* stream = nullptr;
    uint8_t streamId = 0;
    const PrStage1ScriptEvent* event = nullptr;
    uint32_t dueFrame = 0;
};

struct PrStage1RuntimeSlotsDirectRunnerGlobalsCarrier801C9094 {
    bool resourceTimersECFC_ECFEKnown = false;
    int16_t word8008ECFC = 0;
    int16_t word8008ECFE = 0;
    bool gateTimersD3054_D3058Known = false;
    int32_t dword801D3054 = 0;
    int32_t dword801D3058 = 0;
    bool compactRailCacheD304C_D3050Known = false;
    uint32_t dword801D304C = 0;
    uint32_t dword801D3050 = 0;
};

void PrStage1RuntimeSlotsDirectReset(
    PrStage1RuntimeSlotsDirectRuntime& runtime);
void PrStage1RuntimeSlotsDirectReset(
    PrStage1RuntimeSlotsDirectRuntime& runtime,
    const PrStage1RuntimeSlotsDirectResetInput80024E98& resetInput);
void PrStage1RuntimeSlotsDirectResetFromGlobalOptionsCarrier(
    PrStage1RuntimeSlotsDirectRuntime& runtime,
    const PrStage1RuntimeSlotsDirectGlobalOptionsCarrier& globalOptions);
PrStageEventDirectResetAction80024F8C
PrStage1RuntimeSlotsDirectResetEventAndSlots80024E98(
    PrStageEventDirectStage1Runtime& eventRuntime,
    PrStage1RuntimeSlotsDirectRuntime& slotsRuntime,
    const PrStage1RuntimeSlotsDirectGlobalOptionsCarrier& globalOptions);
PrStage1RuntimeSlotsDirectGlobalOptionsCarrier
PrStage1RuntimeSlotsDirectBuildGlobalOptionsCarrierFromWord800916DC(
    bool known,
    uint16_t word800916DC);
const PrStageEventDirectResetAction80024F8C&
PrStage1RuntimeSlotsDirectGetResetAction80024F8C(
    const PrStage1RuntimeSlotsDirectRuntime& runtime);

bool PrStage1RuntimeSlotsDirectGetActiveScriptEvent(
    const PrStage1RuntimeSlotsDirectRuntime& runtime,
    const PrStage1OverlayData& data,
    PrStage1RuntimeSlotsDirectActiveEventResult& out);

bool PrStage1RuntimeSlotsDirectGetPrepassScriptEvent(
    const PrStage1RuntimeSlotsDirectRuntime& runtime,
    const PrStage1OverlayData& data,
    PrStage1RuntimeSlotsDirectActiveEventResult& out);

PrStage1RuntimeSlotsDirectRunnerGlobalsCarrier801C9094
PrStage1RuntimeSlotsDirectBuildRunnerGlobalsCarrier801C9094(
    const PrStage1RuntimeSlotsDirectRuntime& runtime);

PrStageRunnerDirectGlobals801C9094
PrStage1RuntimeSlotsDirectApplyRunnerGlobalsCarrier801C9094(
    const PrStageRunnerDirectGlobals801C9094& fallback,
    const PrStage1RuntimeSlotsDirectRunnerGlobalsCarrier801C9094& carrier);

PrStage1RuntimeSlotsDirectCompactRailRuntime80024744
PrStage1RuntimeSlotsDirectBuildCompactRailProjection80024744(
    const PrStage1RuntimeSlotsDirectCompactProducer801C9094& producer);

void PrStage1RuntimeSlotsDirectAdvance(
    PrStage1RuntimeSlotsDirectRuntime& runtime,
    const PrStage1OverlayData& data,
    const PrStage1RuntimeSlotsDirectAdvanceInput& input);

void PrStage1RuntimeSlotsDirectApplyFollowUpFacts801CBFDC(
    PrStage1RuntimeSlotsDirectAdvanceCarrier801C9094& carrier,
    const PrStage1RuntimeSlotsDirectFollowUpFacts801CBFDC& facts,
    uint32_t queryFrame);

PrStage1RuntimeSlotsDirectAdvanceCarrier801C9094
PrStage1RuntimeSlotsDirectBuildAdvanceCarrier801C9094(
    uint32_t queryFrame,
    uint32_t scriptFrame,
    const PrStage1RuntimeSlotsDirectGlobalOptionsCarrier& globalOptions,
    const PrStageEventDirectStage1Runtime* eventStreamRuntime801C9094,
    const PrStage1RuntimeSlotsDirectFollowUpFacts801CBFDC& followUpFacts);

void PrStage1RuntimeSlotsDirectAdvanceFromCarrier801C9094(
    PrStage1RuntimeSlotsDirectRuntime& runtime,
    const PrStage1OverlayData& data,
    const PrStage1RuntimeSlotsDirectAdvanceCarrier801C9094& carrier);

void PrStage1RuntimeSlotsDirectApplyStageLoopExit801C7A60(
    PrStage1RuntimeSlotsDirectRuntime& runtime);

void PrStage1RuntimeSlotsDirectPopulateSnapshot(
    const PrStage1RuntimeSlotsDirectRuntime& runtime,
    uint32_t queryFrame60,
    uint32_t scriptQueryFrame,
    PrStage1RuntimeSlotsSnapshot& out);
