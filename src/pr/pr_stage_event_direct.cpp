#include "pr_stage_event_direct.h"

#include "pr_stage1_overlay_script_text_direct.h"

#include <algorithm>
#include <array>

namespace {

struct Stage1RouteTable {
    std::array<uint8_t, 4> tailStreamByMode{3u, 2u, 4u, 4u};
    std::array<uint8_t, 4> flag40StreamByMode{5u, 4u, 4u, 4u};
    std::array<uint8_t, 2> flag2000StreamByCtx72{6u, 8u};
    std::array<uint8_t, 2> flag40RowByEd24{4u, 5u};
    std::array<uint8_t, 2> flag2000RowByCtx72{6u, 8u};
    uint8_t flag4000Stream = 7u;
    uint8_t flag4000Row = 7u;
};

constexpr Stage1RouteTable kStage1Routes{};

uint8_t ClampMode(uint8_t mode) {
    return (std::min<uint8_t>)(mode, 3u);
}

bool HasFlag(uint16_t flags, uint16_t mask) {
    return (flags & mask) != 0u;
}

bool IsStage1EventStreamId801C9094(uint16_t streamId) {
    return streamId >= 1u && streamId <= 8u;
}

const PrStageEventDirectStage1ConsumedScriptEvent801C9094*
FindStage1ConsumedEventForStream801C9094(
    const PrStageEventDirectStage1Runtime& runtime,
    uint8_t streamId) {
    if (runtime.idStreamEvent801C9094.valid &&
        runtime.idStreamEvent801C9094.streamId == streamId) {
        return &runtime.idStreamEvent801C9094;
    }
    if (runtime.flagStreamEvent801C9094.valid &&
        runtime.flagStreamEvent801C9094.streamId == streamId) {
        return &runtime.flagStreamEvent801C9094;
    }
    return nullptr;
}

bool IsStage1Flag2000SceneFamilySetTextEvent801C8604(
    const PrStageEventDirectStage1Runtime& runtime,
    const PrStageEventDirectStage1ConsumedScriptEvent801C9094& event) {
    return event.valid && event.textId != 0u &&
           PrStageEventDirectStage1IsFlag2000Stream(event.streamId) &&
           runtime.flag2000SceneFamilyActive &&
           runtime.flag2000SceneFamilyStream == event.streamId &&
           runtime.activeDispatchStream == event.streamId;
}

PrStageEventDirectStage1ConsumedScriptEvent801C9094
ResolveStage1SetTextEvent801C8604(
    const PrStageEventDirectStage1Runtime& runtime,
    const PrStageRunnerDirectActionTrace& trace) {
    PrStageEventDirectStage1ConsumedScriptEvent801C9094 out{};
    for (size_t i = 0; i < trace.count && i < trace.actions.size(); ++i) {
        const PrStageRunnerDirectAction& action = trace.actions[i];
        if (action.kind !=
                PrStageRunnerDirectActionKind::
                    Call801C8604ApplyScriptEventSetText ||
            !action.arg0Known ||
            action.arg0 < 0 ||
            action.arg0 > 0xFF) {
            continue;
        }
        const auto* event = FindStage1ConsumedEventForStream801C9094(
            runtime,
            static_cast<uint8_t>(action.arg0));
        if (event != nullptr) {
            out = *event;
            break;
        }
    }
    const auto& idEvent = runtime.idStreamEvent801C9094;
    if (IsStage1Flag2000SceneFamilySetTextEvent801C8604(runtime, idEvent) &&
        (!out.valid || out.streamId != idEvent.streamId)) {
        return idEvent;
    }
    return out;
}

}  // namespace

uint8_t PrStageEventDirectStage1TailStreamForMode(uint8_t currentMode) {
    return kStage1Routes.tailStreamByMode[ClampMode(currentMode)];
}

uint8_t PrStageEventDirectStage1Flag40StreamForMode(uint8_t currentMode) {
    return kStage1Routes.flag40StreamByMode[ClampMode(currentMode)];
}

uint8_t PrStageEventDirectStage1Flag2000StreamForCtx72(uint16_t ctx72) {
    return kStage1Routes.flag2000StreamByCtx72[(ctx72 < 2u) ? 0u : 1u];
}

uint8_t PrStageEventDirectStage1AuthoritativeRow(uint16_t flags40_2000_4000,
                                                 uint16_t ctx72,
                                                 int32_t ed24) {
    // Matches COMOD1 801C9094/801C895C routing priority: 0x2000 scene
    // dominates 0x40, then 0x4000 tail.
    if (HasFlag(flags40_2000_4000, 0x2000u)) {
        return kStage1Routes.flag2000RowByCtx72[(ctx72 < 2u) ? 0u : 1u];
    }
    if (HasFlag(flags40_2000_4000, 0x0040u)) {
        return kStage1Routes.flag40RowByEd24[(ed24 == 1) ? 1u : 0u];
    }
    if (HasFlag(flags40_2000_4000, 0x4000u)) {
        return kStage1Routes.flag4000Row;
    }
    return 0u;
}

bool PrStageEventDirectStage1IsFlag40Stream(uint8_t streamId) {
    return streamId == kStage1Routes.flag40StreamByMode[0] ||
           streamId == kStage1Routes.flag40StreamByMode[1];
}

bool PrStageEventDirectStage1IsFlag2000Stream(uint8_t streamId) {
    return streamId == kStage1Routes.flag2000StreamByCtx72[0] ||
           streamId == kStage1Routes.flag2000StreamByCtx72[1];
}

bool PrStageEventDirectStage1IsFlag4000Stream(uint8_t streamId) {
    return streamId == kStage1Routes.flag4000Stream;
}

bool PrStageEventDirectStage1IsTerminalStream(uint8_t streamId) {
    return streamId >= 2u && streamId <= 5u;
}

PrStageEventDirectResetAction80024F8C PrStageEventDirectBuildResetAction80024F8C(
    bool eventStreamResetKnown,
    uint16_t word8008ED36,
    uint16_t word8008ED38,
    bool runtimeSlotResetKnown,
    uint16_t word5A_801C369A) {
    PrStageEventDirectResetAction80024F8C out{};
    out.valid = true;
    out.eventStreamResetKnown = eventStreamResetKnown;
    out.word8008ED36 = word8008ED36;
    out.word8008ED38 = word8008ED38;
    out.runtimeSlotResetKnown = runtimeSlotResetKnown;
    out.word5A_801C369A = word5A_801C369A;
    out.scorerBaselineRefresh1448CRequested = true;
    return out;
}

PrStageEventDirectResetAction80024F8C PrStageEventDirectMergeResetActions80024F8C(
    const PrStageEventDirectResetAction80024F8C& eventAction,
    const PrStageEventDirectResetAction80024F8C& slotAction) {
    PrStageEventDirectResetAction80024F8C out{};
    out.valid = eventAction.valid || slotAction.valid;
    out.resetPsxAddr80024F8C = eventAction.valid
        ? eventAction.resetPsxAddr80024F8C
        : slotAction.resetPsxAddr80024F8C;
    out.eventStreamResetKnown = eventAction.eventStreamResetKnown;
    out.word8008ED36 = eventAction.word8008ED36;
    out.word8008ED38 = eventAction.word8008ED38;
    out.runtimeSlotResetKnown = slotAction.runtimeSlotResetKnown;
    out.word5A_801C369A = slotAction.word5A_801C369A;
    out.scorerBaselineRefresh1448CRequested =
        eventAction.scorerBaselineRefresh1448CRequested ||
        slotAction.scorerBaselineRefresh1448CRequested;
    out.scorerBaselineRefreshPsxAddr8001448C =
        eventAction.scorerBaselineRefresh1448CRequested
            ? eventAction.scorerBaselineRefreshPsxAddr8001448C
            : slotAction.scorerBaselineRefreshPsxAddr8001448C;
    return out;
}

namespace {

void SetStage1EventStreamFlag(PrStageEventDirectStage1Runtime& runtime,
                              uint16_t mask,
                              bool enabled) {
    if (enabled) {
        runtime.flags40_2000_4000 |= mask;
    } else {
        runtime.flags40_2000_4000 &= (uint16_t)~mask;
    }
}

constexpr uint8_t kStage1EventStreamFlagUpdateReasonReset80024E98 = 1u;
constexpr uint8_t kStage1EventStreamFlagUpdateReasonFlag40EarlyReset = 2u;
constexpr uint8_t kStage1EventStreamFlagUpdateReasonRunnerGlobals = 3u;
constexpr uint8_t kStage1EventStreamFlagUpdateReasonClearTailService = 4u;

void SetStage1PrimaryEventStreamFlag801C9094(
    PrStageEventDirectStage1Runtime& runtime,
    uint16_t value,
    uint8_t reason,
    uint32_t queryFrame,
    uint32_t scriptFrame) {
    const uint16_t previous = runtime.gPrStageEventStreamFlag;
    runtime.gPrStageEventStreamFlag = value;
    runtime.eventStreamFlagLastUpdateKnown = true;
    runtime.eventStreamFlagLastUpdateReason = reason;
    runtime.eventStreamFlagLastUpdateQueryFrame = queryFrame;
    runtime.eventStreamFlagLastUpdateScriptFrame = scriptFrame;
    runtime.eventStreamFlagLastUpdatePrevious = previous;
    runtime.eventStreamFlagLastUpdateCurrent = value;
    if (previous != value) {
        runtime.eventStreamFlagLastChangeKnown = true;
        runtime.eventStreamFlagLastChangeReason = reason;
        runtime.eventStreamFlagLastChangeQueryFrame = queryFrame;
        runtime.eventStreamFlagLastChangeScriptFrame = scriptFrame;
        runtime.eventStreamFlagLastChangePrevious = previous;
        runtime.eventStreamFlagLastChangeCurrent = value;
    }
}

uint8_t ResolveStage1EventStreamTailStreamForMode(uint8_t currentMode) {
    return PrStageEventDirectStage1TailStreamForMode(currentMode);
}

uint8_t ResolveStage1EventStreamFollowUpStreamForFlag40(uint8_t currentMode) {
    return PrStageEventDirectStage1Flag40StreamForMode(currentMode);
}

uint8_t ResolveStage1EventStreamFollowUpStreamForFlag2000(uint16_t ctx72) {
    return PrStageEventDirectStage1Flag2000StreamForCtx72(ctx72);
}

uint32_t ResolveStage1EventStreamDispatchEndLocalFrame(
    const PrStage1EventStream& stream) {
    uint32_t endFrame = 0u;
    for (const auto& ev : stream.events) {
        endFrame = (std::max)(endFrame, ev.frame);
    }
    return endFrame;
}

uint32_t ResolveStage1EventStreamDispatchTerminalEndLocalFrame(
    const PrStage1EventStream& stream) {
    uint32_t endFrame = 0u;
    for (const auto& ev : stream.events) {
        endFrame = (std::max)(endFrame, ev.frame);
    }
    return endFrame;
}

bool ResolveStage1EventStreamFlag40BootstrapPulse(
    const PrStageEventDirectStage1FrameInput& input) {
    return input.bucket0Ctx118WritePulse;
}

bool IsStage1EventStreamTerminalFlag100Source(uint8_t streamId) {
    return streamId >= 2u && streamId <= 5u;
}

void ApplyStage1RunnerTerminalFlag100Result801C9094(
    PrStageEventDirectStage1Runtime& runtime,
    const PrStageRunnerDirectFrameUpdate9094Result& result) {
    if (!result.setCtxFlag0100 ||
        !IsStage1EventStreamTerminalFlag100Source(result.resolvedEventStreamId)) {
        return;
    }

    runtime.flag100BlocksWaitPulse = true;
    runtime.flag100SourceStream = result.resolvedEventStreamId;
    runtime.activeDispatchTerminalPulseEmitted = true;
    if (result.resolvedEventStreamId < runtime.unk801D2D64.size()) {
        runtime.unk801D2D64[result.resolvedEventStreamId].cursor = 0u;
    }
    if (runtime.tailDispatchFamilyActive &&
        runtime.tailDispatchFamilyStream == result.resolvedEventStreamId) {
        runtime.tailDispatchFamilyActive = false;
        runtime.tailDispatchFamilyStream = 0u;
    }
    if (runtime.clearTerminalTailDispatchActive &&
        runtime.clearTerminalTailStream == result.resolvedEventStreamId) {
        runtime.clearTerminalTailDispatchActive = false;
        runtime.clearTerminalTailStream = 0u;
    }
    if (runtime.activeDispatchStream == result.resolvedEventStreamId) {
        runtime.activeDispatchStream = 0u;
        runtime.activeDispatchStartFrame = 0u;
        runtime.activeDispatchStartScriptFrame = 0u;
        runtime.activeDispatchEndLocalFrame = 0u;
        runtime.activeDispatchTerminalEndLocalFrame = 0u;
        runtime.activeDispatchTerminalPulseEmitted = false;
        runtime.activeDispatchDonePulseEmitted = false;
    }
}

PrStageEventDirectStage1FrameResult801C9094
BuildStage1FrameResult801C9094(
    const PrStageEventDirectStage1Runtime& runtime,
    uint32_t queryFrame,
    uint32_t scriptFrame) {
    PrStageEventDirectStage1FrameResult801C9094 out{};
    out.valid = true;
    out.queryFrame = queryFrame;
    out.scriptFrame = scriptFrame;
    out.flagStreamEvent = runtime.flagStreamEvent801C9094;
    out.idStreamEvent = runtime.idStreamEvent801C9094;
    // 801C9094 runs the flag-stream compact prepass before the id-stream
    // apply/text branch, so stream7 must not discard the producer event.
    out.compactEvent801C9094 = out.flagStreamEvent;
    out.ctx0FinalRequirements801C9094 =
        runtime.ctx0FinalRequirements801C9094;
    out.selectedStream = runtime.selectedStream;
    out.activeDispatchStream = runtime.activeDispatchStream;
    out.activeDispatchStartFrame = runtime.activeDispatchStartFrame;
    out.activeDispatchStartScriptFrame =
        runtime.activeDispatchStartScriptFrame;
    out.flag2000SceneFamilyActive = runtime.flag2000SceneFamilyActive;
    out.flag2000SceneFamilyStream = runtime.flag2000SceneFamilyStream;
    out.flag2000SceneFamilyStartFrame =
        runtime.flag2000SceneFamilyStartFrame;
    out.flag2000SceneFamilyStartScriptFrame =
        runtime.flag2000SceneFamilyStartScriptFrame;

    // 801C9094 enters the g_PrStageEventStreamId branch whenever an id stream
    // is active. If no id event is due, that branch returns before applying the
    // flag/prepass event.
    if (out.idStreamEvent.valid) {
        out.consumedEventSource =
            PrStageEventDirectStage1ConsumedEventSource801C9094::IdStream;
        out.consumedEvent = out.idStreamEvent;
    } else if (runtime.activeDispatchStream == 0u && out.flagStreamEvent.valid) {
        out.consumedEventSource =
            PrStageEventDirectStage1ConsumedEventSource801C9094::FlagStream;
        out.consumedEvent = out.flagStreamEvent;
    }
    return out;
}

void ProjectStage1EventStreamGlobals801C9094(
    PrStageEventDirectStage1Runtime& runtime) {
    runtime.eventStreamFlagActive = runtime.gPrStageEventStreamFlag != 0u;
    runtime.eventStreamFlagStream =
        runtime.eventStreamFlagActive
            ? (uint8_t)runtime.gPrStageEventStreamFlag
            : 0u;
    runtime.runnerGlobalsCore801C9094.known = true;
    runtime.runnerGlobalsCore801C9094.unk8008ED00 =
        static_cast<uint16_t>(runtime.ed00);
    runtime.runnerGlobalsCore801C9094.unk8008ED20 =
        static_cast<uint16_t>(runtime.ed20);
    runtime.runnerGlobalsCore801C9094.eventStreamFlag =
        runtime.gPrStageEventStreamFlag;
    runtime.runnerGlobalsCore801C9094.eventStreamId =
        runtime.gPrStageEventStreamId;
    runtime.runnerGlobalsCore801C9094.eventStreamDone =
        runtime.gPrStageEventStreamDone;
    runtime.runnerGlobalsCore801C9094.dword801D3048 =
        static_cast<int32_t>(runtime.dword801D3048);
}

void ApplyStage1EventStreamResetSub80024F8C(
    PrStageEventDirectStage1Runtime& runtime) {
    runtime.word8008ED36 = runtime.eventStreamReset80024E98.word8008ED36;
    runtime.word8008ED38 = runtime.eventStreamReset80024E98.word8008ED38;
    runtime.resetAction80024F8C = PrStageEventDirectBuildResetAction80024F8C(
        true,
        runtime.word8008ED36,
        runtime.word8008ED38,
        false,
        0u);
}

void ApplyStage1EventStreamResetSub80024E98(
    PrStageEventDirectStage1Runtime& runtime) {
    runtime.ed20 = runtime.eventStreamReset80024E98.dword8008ED20;
    runtime.ed00 = runtime.eventStreamReset80024E98.dword8008ED00;
    SetStage1PrimaryEventStreamFlag801C9094(
        runtime,
        runtime.eventStreamReset80024E98.gPrStageEventStreamFlag,
        kStage1EventStreamFlagUpdateReasonReset80024E98,
        0u,
        0u);
    runtime.gPrStageEventStreamId =
        runtime.eventStreamReset80024E98.gPrStageEventStreamId;
    runtime.gpPlus800 = runtime.eventStreamReset80024E98.gpPlus800;
    ApplyStage1EventStreamResetSub80024F8C(runtime);
}

bool ShouldBootstrapStage1EventStreamFlag40Dispatch(
    const PrStageEventDirectStage1Runtime& runtime,
    uint16_t bootstrapMask) {
    if ((bootstrapMask & 0x0040u) == 0u) {
        return false;
    }
    return runtime.activeDispatchStream == 0u &&
           runtime.pendingRatingBranchSeq == runtime.consumedRatingBranchSeq &&
           !PrStageEventDirectStage1HasFlag(runtime, 0x0040u) &&
           !PrStageEventDirectStage1HasFlag(runtime, 0x2000u) &&
           !PrStageEventDirectStage1HasFlag(runtime, 0x4000u);
}

void PrStageEventDirectStage1EventStreamReset801C858C(
    PrStageEventDirectStage1Runtime& runtime,
    uint8_t streamId,
    uint32_t ctxTick0C) {
    // COMOD1 801C858C:
    // if streamId is 1..8, clear `unk_801D2D64[streamId].cursor`;
    // always copy `ctx+0x0C` to `dword_801D3048`.
    if (streamId >= 1u && streamId <= 8u) {
        runtime.unk801D2D64[streamId].cursor = 0u;
    }
    runtime.dword801D3048 = ctxTick0C;
}

bool PrStageEventDirectStage1DescDone801C9094(
    const PrStageEventDirectStage1Runtime& runtime,
    const PrStage1OverlayData& data,
    uint16_t streamId) {
    if (!IsStage1EventStreamId801C9094(streamId) ||
        streamId >= runtime.unk801D2D64.size()) {
        return false;
    }

    const PrStage1EventStream* stream = data.FindStream((uint8_t)streamId);
    return stream && runtime.unk801D2D64[streamId].cursor >=
                         stream->events.size();
}

const PrStage1ScriptEvent* PrStageEventDirectStage1ConsumeNextEvent801C9094(
    PrStageEventDirectStage1Runtime& runtime,
    const PrStage1OverlayData& data,
    uint16_t streamId,
    uint32_t ctxTick0C,
    uint32_t baseFrame,
    PrStageEventDirectStage1ConsumedScriptEvent801C9094& outConsumed) {
    outConsumed = PrStageEventDirectStage1ConsumedScriptEvent801C9094{};
    if (!IsStage1EventStreamId801C9094(streamId) ||
        streamId >= runtime.unk801D2D64.size()) {
        return nullptr;
    }

    const PrStage1EventStream* stream = data.FindStream((uint8_t)streamId);
    if (!stream) {
        return nullptr;
    }

    PrStageEventDirectStage1DescCursor801C9094& desc =
        runtime.unk801D2D64[streamId];
    if (desc.cursor >= stream->events.size()) {
        return nullptr;
    }

    const uint32_t eventIndex = desc.cursor;
    const PrStage1ScriptEvent& ev = stream->events[eventIndex];
    const uint32_t dueFrame = baseFrame + ev.frame;
    if (ctxTick0C < dueFrame) {
        return nullptr;
    }

    desc.cursor++;
    outConsumed.valid = true;
    outConsumed.streamId = (uint8_t)streamId;
    outConsumed.eventIndex = eventIndex;
    outConsumed.eventFrame = ev.frame;
    outConsumed.dueFrame = dueFrame;
    outConsumed.flags04 = ev.flags04;
    outConsumed.byte29 = ev.byte1D;
    outConsumed.byte30 = ev.byte1E;
    outConsumed.textId = ev.textId;
    outConsumed.psxAddr = ev.psxAddr;
    return &ev;
}

void PopulateRunnerCursorFromConsumedEvent801C9094(
    const PrStageEventDirectStage1ConsumedScriptEvent801C9094& consumed,
    PrStageRunnerDirectEventStreamCursor801C9094& outCursor,
    bool& outDescriptorValid) {
    if (!consumed.valid) {
        return;
    }
    outDescriptorValid = true;
    outCursor = PrStageRunnerDirectEventStreamCursor801C9094{};
    outCursor.valid = true;
    outCursor.count = consumed.eventIndex + 1u;
    outCursor.index = consumed.eventIndex;
    outCursor.dueEventKnown = true;
    outCursor.dueFrame = static_cast<int32_t>(consumed.eventFrame);
    outCursor.flags04 = consumed.flags04;
    outCursor.byte29 = consumed.byte29;
    outCursor.byte30 = consumed.byte30;
}

void StartStage1EventStreamDispatch(PrStageEventDirectStage1Runtime& runtime,
                                    const PrStage1EventStream& stream,
                                    uint8_t streamId,
                                    uint32_t queryFrame,
                                    uint32_t scriptFrame) {
    runtime.activeDispatchStream = streamId;
    runtime.activeDispatchStartFrame = queryFrame;
    runtime.activeDispatchStartScriptFrame = scriptFrame;
    runtime.activeDispatchEndLocalFrame =
        ResolveStage1EventStreamDispatchEndLocalFrame(stream);
    runtime.activeDispatchTerminalEndLocalFrame =
        ResolveStage1EventStreamDispatchTerminalEndLocalFrame(stream);
    runtime.activeDispatchTerminalPulseEmitted = false;
    runtime.activeDispatchDonePulseEmitted = false;
    runtime.lastStartedStream = streamId;
    runtime.lastStartedQueryFrame = queryFrame;
    runtime.gPrStageEventStreamId = streamId;
    PrStageEventDirectStage1EventStreamReset801C858C(
        runtime,
        streamId,
        scriptFrame);
    ProjectStage1EventStreamGlobals801C9094(runtime);
}

void ConsumeStartedStage1EventStreamDispatch801C9094(
    PrStageEventDirectStage1Runtime& runtime,
    const PrStage1OverlayData& data,
    uint8_t streamId,
    uint32_t scriptFrame) {
    if (runtime.idStreamEvent801C9094.valid) {
        return;
    }
    PrStageEventDirectStage1ConsumeNextEvent801C9094(
        runtime,
        data,
        streamId,
        scriptFrame,
        runtime.dword801D3048,
        runtime.idStreamEvent801C9094);
}

void RefreshStage1EventStreamDispatchEdges(
    PrStageEventDirectStage1Runtime& runtime) {
    const bool flag40 = PrStageEventDirectStage1HasFlag(runtime, 0x0040u);
    const bool flag2000 = PrStageEventDirectStage1HasFlag(runtime, 0x2000u);
    const bool flag4000 = PrStageEventDirectStage1HasFlag(runtime, 0x4000u);

    if (!flag40 || !runtime.prevFlag40) {
        runtime.flags40LastDispatchedStream = 0u;
    }
    if (!flag2000 || !runtime.prevFlag2000) {
        runtime.flags2000LastDispatchedStream = 0u;
    }
    if (!flag4000 || !runtime.prevFlag4000) {
        runtime.flags4000LastDispatchedStream = 0u;
    }

    runtime.prevFlag40 = flag40;
    runtime.prevFlag2000 = flag2000;
    runtime.prevFlag4000 = flag4000;
}

void ClearStage1ActiveEventStreamDispatch801C9094(
    PrStageEventDirectStage1Runtime& runtime) {
    runtime.activeDispatchStream = 0u;
    runtime.activeDispatchStartFrame = 0u;
    runtime.activeDispatchStartScriptFrame = 0u;
    runtime.activeDispatchEndLocalFrame = 0u;
    runtime.activeDispatchTerminalEndLocalFrame = 0u;
    runtime.activeDispatchTerminalPulseEmitted = false;
    runtime.activeDispatchDonePulseEmitted = false;
    runtime.tailDispatchFamilyActive = false;
    runtime.tailDispatchFamilyStream = 0u;
    runtime.clearTerminalTailDispatchActive = false;
}

void ApplyStage1Flag40EarlyReturnReset801C9094(
    PrStageEventDirectStage1Runtime& runtime,
    const PrStage1OverlayData& data,
    const PrStageEventDirectStage1FrameInput& input,
    const PrStageRunnerDirectFrameUpdate9094Result& frameUpdate) {
    runtime.flagStreamEvent801C9094 =
        PrStageEventDirectStage1ConsumedScriptEvent801C9094{};
    runtime.idStreamEvent801C9094 =
        PrStageEventDirectStage1ConsumedScriptEvent801C9094{};
    SetStage1PrimaryEventStreamFlag801C9094(
        runtime,
        0u,
        kStage1EventStreamFlagUpdateReasonFlag40EarlyReset,
        input.queryFrame,
        input.scriptFrame);
    runtime.eventStreamFlagActive = false;
    runtime.eventStreamFlagStream = 0u;
    runtime.ed20 = static_cast<int32_t>(frameUpdate.globals.unk8008ED20);
    runtime.flags40_2000_4000 |= 0x0040u;

    const uint8_t streamId =
        static_cast<uint8_t>(frameUpdate.globals.eventStreamId & 0xFFu);
    ClearStage1ActiveEventStreamDispatch801C9094(runtime);
    if (IsStage1EventStreamId801C9094(streamId)) {
        if (const PrStage1EventStream* stream = data.FindStream(streamId)) {
            StartStage1EventStreamDispatch(runtime,
                                           *stream,
                                           streamId,
                                           input.queryFrame,
                                           input.scriptFrame);
            if (PrStageEventDirectStage1IsFlag40Stream(streamId)) {
                runtime.flags40LastDispatchedStream = streamId;
                runtime.consumedRatingBranchSeq =
                    runtime.pendingRatingBranchSeq;
            }
        } else {
            runtime.gPrStageEventStreamId = streamId;
            ProjectStage1EventStreamGlobals801C9094(runtime);
        }
    }

    runtime.frameResult801C9094 =
        BuildStage1FrameResult801C9094(
            runtime,
            input.queryFrame,
            input.scriptFrame);
    runtime.frameResult801C9094.flag40EarlyReturnReset801C9094 = true;
}

}  // namespace

void PrStageEventDirectStage1Reset(PrStageEventDirectStage1Runtime& runtime) {
    runtime = PrStageEventDirectStage1Runtime{};
    runtime.eventStreamReset80024E98 =
        PrStageEventDirectEventStreamReset80024E98{};
    ApplyStage1EventStreamResetSub80024E98(runtime);
    runtime.gPrStageEventStreamDone = false;
    runtime.eventStreamFlagActive = true;
    runtime.eventStreamFlagStream = 1u;
    ProjectStage1EventStreamGlobals801C9094(runtime);
}

const PrStageEventDirectResetAction80024F8C&
PrStageEventDirectStage1GetResetAction80024F8C(
    const PrStageEventDirectStage1Runtime& runtime) {
    return runtime.resetAction80024F8C;
}

const PrStageEventDirectStage1FrameResult801C9094&
PrStageEventDirectStage1GetFrameResult801C9094(
    const PrStageEventDirectStage1Runtime& runtime) {
    return runtime.frameResult801C9094;
}

bool PrStageEventDirectStage1ConsumeClearTerminalBranchTrigger801C9094(
    PrStageEventDirectStage1Runtime& runtime,
    const PrStage1OverlayData& data,
    uint32_t scriptFrame,
    uint8_t rightRankActiveRow) {
    const uint8_t currentMode =
        PrStageEventDirectStage1ResolveCurrentMode801C9094(
            rightRankActiveRow);
    const uint16_t streamFlag = runtime.gPrStageEventStreamFlag;
    runtime.lastClearTerminalBranchTriggerAttempted = true;
    runtime.lastClearTerminalBranchTriggerAccepted = false;
    runtime.lastClearTerminalBranchTriggerScriptFrame = scriptFrame;
    runtime.lastClearTerminalBranchTriggerRightRankRow = rightRankActiveRow;
    runtime.lastClearTerminalBranchTriggerCurrentMode = currentMode;
    runtime.lastClearTerminalBranchTriggerBlockedConsumed =
        runtime.clearTerminalBranchTriggerConsumed801C9094;
    runtime.lastClearTerminalBranchTriggerBlockedArmed =
        runtime.clearTerminalTailArmed;
    runtime.lastClearTerminalBranchTriggerBlockedFlagNotOne =
        streamFlag != 1u;
    runtime.lastClearTerminalBranchTriggerBlockedRow =
        rightRankActiveRow > 1u;
    runtime.lastClearTerminalBranchTriggerBlockedStreamMissing = false;
    runtime.lastClearTerminalBranchTriggerBlockedCursorDone = false;
    runtime.lastClearTerminalBranchTriggerBlockedEventNotDue = false;
    runtime.lastClearTerminalBranchTriggerBlockedMissingFlag80 = false;
    runtime.lastClearTerminalBranchTriggerStreamFlag = streamFlag;
    runtime.clearTerminalBranchTriggerAttemptCount++;
    if (!runtime.firstClearTerminalBranchTriggerKnown) {
        runtime.firstClearTerminalBranchTriggerKnown = true;
        runtime.firstClearTerminalBranchTriggerScriptFrame = scriptFrame;
        runtime.firstClearTerminalBranchTriggerRightRankRow =
            rightRankActiveRow;
        runtime.firstClearTerminalBranchTriggerCurrentMode = currentMode;
        runtime.firstClearTerminalBranchTriggerStreamFlag = streamFlag;
    }
    if (runtime.lastClearTerminalBranchTriggerBlockedFlagNotOne) {
        runtime.clearTerminalBranchTriggerBlockedFlagNotOneCount++;
    }
    if (runtime.lastClearTerminalBranchTriggerBlockedRow) {
        runtime.clearTerminalBranchTriggerBlockedRowCount++;
    }
    if (runtime.lastClearTerminalBranchTriggerBlockedFlagNotOne &&
        runtime.lastClearTerminalBranchTriggerBlockedRow) {
        runtime.clearTerminalBranchTriggerBlockedFlagAndRowCount++;
    }
    if (runtime.lastClearTerminalBranchTriggerBlockedConsumed) {
        runtime.clearTerminalBranchTriggerBlockedConsumedCount++;
    }
    if (runtime.lastClearTerminalBranchTriggerBlockedArmed) {
        runtime.clearTerminalBranchTriggerBlockedArmedCount++;
    }
    if (!runtime.lastClearTerminalBranchTriggerBlockedConsumed &&
        !runtime.lastClearTerminalBranchTriggerBlockedArmed &&
        !runtime.lastClearTerminalBranchTriggerBlockedFlagNotOne &&
        !runtime.lastClearTerminalBranchTriggerBlockedRow) {
        runtime.clearTerminalBranchTriggerEligibleCount++;
        if (!runtime.firstClearTerminalBranchTriggerEligibleKnown) {
            runtime.firstClearTerminalBranchTriggerEligibleKnown = true;
            runtime.firstClearTerminalBranchTriggerEligibleScriptFrame =
                scriptFrame;
            runtime.firstClearTerminalBranchTriggerEligibleRightRankRow =
                rightRankActiveRow;
            runtime.firstClearTerminalBranchTriggerEligibleCurrentMode =
                currentMode;
        }
    }

    if (runtime.lastClearTerminalBranchTriggerBlockedConsumed ||
        runtime.lastClearTerminalBranchTriggerBlockedArmed ||
        runtime.lastClearTerminalBranchTriggerBlockedFlagNotOne ||
        runtime.lastClearTerminalBranchTriggerBlockedRow) {
        return false;
    }

    const PrStage1EventStream* stream = data.FindStream(1u);
    if (!stream) {
        runtime.lastClearTerminalBranchTriggerBlockedStreamMissing = true;
        runtime.clearTerminalBranchTriggerBlockedStreamMissingCount++;
        return false;
    }

    const PrStageEventDirectStage1DescCursor801C9094& desc =
        runtime.unk801D2D64[1u];
    runtime.lastClearTerminalBranchTriggerStream1Cursor = desc.cursor;
    runtime.lastClearTerminalBranchTriggerStream1Count =
        static_cast<uint32_t>(stream->events.size());
    runtime.lastClearTerminalBranchTriggerStream1DueKnown = false;
    runtime.lastClearTerminalBranchTriggerStream1DueFrame = 0u;
    runtime.lastClearTerminalBranchTriggerStream1DueDelta = 0;
    runtime.lastClearTerminalBranchTriggerStream1BaseFrame =
        runtime.dword801D3048;
    runtime.lastClearTerminalBranchTriggerStream1AbsDueFrame = 0u;
    runtime.lastClearTerminalBranchTriggerStream1AbsDueDelta = 0;
    runtime.lastClearTerminalBranchTriggerStream1Flags04 = 0u;
    runtime.lastClearTerminalBranchTriggerStream1Byte29 = 0u;
    runtime.lastClearTerminalBranchTriggerStream1Byte30 = 0u;
    if (desc.cursor >= stream->events.size()) {
        runtime.lastClearTerminalBranchTriggerBlockedCursorDone = true;
        runtime.clearTerminalBranchTriggerBlockedCursorDoneCount++;
        return false;
    }

    const PrStage1ScriptEvent& ev = stream->events[desc.cursor];
    const uint32_t absDueFrame = runtime.dword801D3048 + ev.frame;
    runtime.lastClearTerminalBranchTriggerStream1DueKnown = true;
    runtime.lastClearTerminalBranchTriggerStream1DueFrame = ev.frame;
    runtime.lastClearTerminalBranchTriggerStream1DueDelta =
        static_cast<int32_t>(ev.frame) - static_cast<int32_t>(scriptFrame);
    runtime.lastClearTerminalBranchTriggerStream1BaseFrame =
        runtime.dword801D3048;
    runtime.lastClearTerminalBranchTriggerStream1AbsDueFrame = absDueFrame;
    runtime.lastClearTerminalBranchTriggerStream1AbsDueDelta =
        static_cast<int32_t>(absDueFrame) - static_cast<int32_t>(scriptFrame);
    runtime.lastClearTerminalBranchTriggerStream1Flags04 = ev.flags04;
    runtime.lastClearTerminalBranchTriggerStream1Byte29 = ev.byte1D;
    runtime.lastClearTerminalBranchTriggerStream1Byte30 = ev.byte1E;

    bool nextFlag80Known = false;
    uint32_t nextFlag80Cursor = 0u;
    uint32_t nextFlag80Frame = 0u;
    uint32_t nextFlag80Flags04 = 0u;
    uint8_t nextFlag80Byte29 = 0u;
    uint8_t nextFlag80Byte30 = 0u;
    for (size_t eventIndex = desc.cursor; eventIndex < stream->events.size();
         ++eventIndex) {
        const PrStage1ScriptEvent& candidate = stream->events[eventIndex];
        if ((candidate.flags04 & 0x00000080u) == 0u) {
            continue;
        }
        nextFlag80Known = true;
        nextFlag80Cursor = static_cast<uint32_t>(eventIndex);
        nextFlag80Frame = candidate.frame;
        nextFlag80Flags04 = candidate.flags04;
        nextFlag80Byte29 = candidate.byte1D;
        nextFlag80Byte30 = candidate.byte1E;
        break;
    }
    if (runtime.firstClearTerminalBranchTriggerEligibleKnown &&
        !runtime.firstClearTerminalBranchTriggerEligibleStream1Known) {
        runtime.firstClearTerminalBranchTriggerEligibleStream1Known = true;
        runtime.firstClearTerminalBranchTriggerEligibleStream1Cursor =
            desc.cursor;
        runtime.firstClearTerminalBranchTriggerEligibleStream1Count =
            static_cast<uint32_t>(stream->events.size());
        runtime.firstClearTerminalBranchTriggerEligibleStream1DueKnown = true;
        runtime.firstClearTerminalBranchTriggerEligibleStream1DueFrame =
            ev.frame;
        runtime.firstClearTerminalBranchTriggerEligibleStream1DueDelta =
            runtime.lastClearTerminalBranchTriggerStream1DueDelta;
        runtime.firstClearTerminalBranchTriggerEligibleStream1BaseFrame =
            runtime.lastClearTerminalBranchTriggerStream1BaseFrame;
        runtime.firstClearTerminalBranchTriggerEligibleStream1AbsDueFrame =
            runtime.lastClearTerminalBranchTriggerStream1AbsDueFrame;
        runtime.firstClearTerminalBranchTriggerEligibleStream1AbsDueDelta =
            runtime.lastClearTerminalBranchTriggerStream1AbsDueDelta;
        runtime.firstClearTerminalBranchTriggerEligibleStream1Flags04 =
            ev.flags04;
        runtime.firstClearTerminalBranchTriggerEligibleStream1Byte29 =
            ev.byte1D;
        runtime.firstClearTerminalBranchTriggerEligibleStream1Byte30 =
            ev.byte1E;
        runtime.firstClearTerminalBranchTriggerEligibleStream1NextFlag80SearchKnown =
            true;
        runtime.firstClearTerminalBranchTriggerEligibleStream1NextFlag80Known =
            nextFlag80Known;
        runtime.firstClearTerminalBranchTriggerEligibleStream1NextFlag80Cursor =
            nextFlag80Cursor;
        runtime.firstClearTerminalBranchTriggerEligibleStream1NextFlag80Frame =
            nextFlag80Frame;
        runtime.firstClearTerminalBranchTriggerEligibleStream1NextFlag80Delta =
            static_cast<int32_t>(nextFlag80Frame) -
            static_cast<int32_t>(scriptFrame);
        runtime.firstClearTerminalBranchTriggerEligibleStream1NextFlag80BaseFrame =
            runtime.lastClearTerminalBranchTriggerStream1BaseFrame;
        runtime.firstClearTerminalBranchTriggerEligibleStream1NextFlag80AbsFrame =
            runtime.lastClearTerminalBranchTriggerStream1BaseFrame + nextFlag80Frame;
        runtime.firstClearTerminalBranchTriggerEligibleStream1NextFlag80AbsDelta =
            static_cast<int32_t>(
                runtime.firstClearTerminalBranchTriggerEligibleStream1NextFlag80AbsFrame) -
            static_cast<int32_t>(scriptFrame);
        runtime.firstClearTerminalBranchTriggerEligibleStream1NextFlag80Flags04 =
            nextFlag80Flags04;
        runtime.firstClearTerminalBranchTriggerEligibleStream1NextFlag80Byte29 =
            nextFlag80Byte29;
        runtime.firstClearTerminalBranchTriggerEligibleStream1NextFlag80Byte30 =
            nextFlag80Byte30;
    }
    // The 801C9094 flag-stream branch compares ctx tick against the raw event
    // frame; only the id-stream branch adds dword_801D3048.
    if (ev.frame > scriptFrame) {
        runtime.lastClearTerminalBranchTriggerBlockedEventNotDue = true;
        runtime.clearTerminalBranchTriggerBlockedEventNotDueCount++;
        if (!runtime.firstClearTerminalBranchTriggerEventNotDueKnown) {
            runtime.firstClearTerminalBranchTriggerEventNotDueKnown = true;
            runtime.firstClearTerminalBranchTriggerEventNotDueScriptFrame =
                scriptFrame;
            runtime.firstClearTerminalBranchTriggerEventNotDueCursor =
                desc.cursor;
            runtime.firstClearTerminalBranchTriggerEventNotDueFrame =
                ev.frame;
            runtime.firstClearTerminalBranchTriggerEventNotDueDelta =
                runtime.lastClearTerminalBranchTriggerStream1DueDelta;
            runtime.firstClearTerminalBranchTriggerEventNotDueBaseFrame =
                runtime.lastClearTerminalBranchTriggerStream1BaseFrame;
            runtime.firstClearTerminalBranchTriggerEventNotDueAbsFrame =
                runtime.lastClearTerminalBranchTriggerStream1AbsDueFrame;
            runtime.firstClearTerminalBranchTriggerEventNotDueAbsDelta =
                runtime.lastClearTerminalBranchTriggerStream1AbsDueDelta;
            runtime.firstClearTerminalBranchTriggerEventNotDueFlags04 =
                ev.flags04;
        }
        return false;
    }
    if ((ev.flags04 & 0x00000080u) == 0u) {
        runtime.lastClearTerminalBranchTriggerBlockedMissingFlag80 = true;
        runtime.clearTerminalBranchTriggerBlockedMissingFlag80Count++;
        if (!runtime.firstClearTerminalBranchTriggerMissingFlag80Known) {
            runtime.firstClearTerminalBranchTriggerMissingFlag80Known = true;
            runtime.firstClearTerminalBranchTriggerMissingFlag80ScriptFrame =
                scriptFrame;
            runtime.firstClearTerminalBranchTriggerMissingFlag80Cursor =
                desc.cursor;
            runtime.firstClearTerminalBranchTriggerMissingFlag80Frame =
                ev.frame;
            runtime.firstClearTerminalBranchTriggerMissingFlag80Delta =
                runtime.lastClearTerminalBranchTriggerStream1DueDelta;
            runtime.firstClearTerminalBranchTriggerMissingFlag80BaseFrame =
                runtime.lastClearTerminalBranchTriggerStream1BaseFrame;
            runtime.firstClearTerminalBranchTriggerMissingFlag80AbsFrame =
                runtime.lastClearTerminalBranchTriggerStream1AbsDueFrame;
            runtime.firstClearTerminalBranchTriggerMissingFlag80AbsDelta =
                runtime.lastClearTerminalBranchTriggerStream1AbsDueDelta;
            runtime.firstClearTerminalBranchTriggerMissingFlag80Flags04 =
                ev.flags04;
        }
        return false;
    }

    runtime.clearTerminalBranchTriggerConsumed801C9094 = true;
    runtime.lastClearTerminalBranchTriggerAccepted = true;
    runtime.clearTerminalBranchTriggerAcceptedCount++;
    return true;
}

bool PrStageEventDirectStage1BuildRunnerCursor801C9094(
    const PrStageEventDirectStage1Runtime& runtime,
    const PrStage1OverlayData& data,
    uint16_t streamId,
    PrStageRunnerDirectEventStreamCursor801C9094& out) {
    out = PrStageRunnerDirectEventStreamCursor801C9094{};
    if (!IsStage1EventStreamId801C9094(streamId) ||
        streamId >= runtime.unk801D2D64.size()) {
        return false;
    }

    const PrStage1EventStream* stream = data.FindStream((uint8_t)streamId);
    if (!stream) {
        return false;
    }

    const uint32_t cursor = runtime.unk801D2D64[streamId].cursor;
    out.valid = true;
    out.count = (uint32_t)stream->events.size();
    out.index = cursor;
    if (cursor < stream->events.size()) {
        const PrStage1ScriptEvent& ev = stream->events[cursor];
        out.dueEventKnown = true;
        out.dueFrame = (int32_t)ev.frame;
        out.flags04 = ev.flags04;
        out.byte29 = ev.byte1D;
        out.byte30 = ev.byte1E;
    }
    return true;
}

PrStageEventDirectStage1RunnerInputBridge801C9094
PrStageEventDirectStage1BuildRunnerInputBridge801C9094(
    const PrStageEventDirectStage1Runtime& runtime,
    const PrStage1OverlayData& data) {
    PrStageEventDirectStage1RunnerInputBridge801C9094 out{};
    if (runtime.gPrStageEventStreamFlag != 0u) {
        out.flagStreamDescriptorValid =
            PrStageEventDirectStage1BuildRunnerCursor801C9094(
                runtime,
                data,
                runtime.gPrStageEventStreamFlag,
                out.activeFlagStream);
    }
    if (runtime.gPrStageEventStreamId != 0u) {
        out.idStreamDescriptorValid =
            PrStageEventDirectStage1BuildRunnerCursor801C9094(
                runtime,
                data,
                runtime.gPrStageEventStreamId,
                out.activeIdStream);
    }

    if (out.activeFlagStream.valid &&
        out.activeFlagStream.dueEventKnown) {
        const PrStage1CompactRailRec44* rec =
            data.FindCompactRailRec44(out.activeFlagStream.byte29);
        if (rec != nullptr) {
            out.compactRail.known = true;
            out.compactRail.recordIndex = out.activeFlagStream.byte29;
            out.compactRail.psxAddr = rec->psxAddr;
            out.compactRail.signedMode = rec->head0;
            out.compactRail.startIndex = (int16_t)rec->head1;
            out.compactRail.primaryStreamPtr = rec->psxAddr + 4u;
            const PrStage1CompactRailRec44* secondary =
                data.FindCompactRailRec44(out.activeFlagStream.byte30);
            out.compactRail.secondaryStreamPtr =
                secondary != nullptr ? secondary->psxAddr + 24u : 0u;
        }
    }
    return out;
}

PrStageRunnerDirectGlobals801C9094
PrStageEventDirectStage1BuildRunnerGlobalsCore801C9094(
    const PrStageEventDirectStage1Runtime& runtime,
    const PrStageRunnerDirectGlobals801C9094& fallback) {
    PrStageRunnerDirectGlobals801C9094 out = fallback;
    out.unk8008ED00 =
        static_cast<uint16_t>(runtime.runnerGlobalsCore801C9094.unk8008ED00);
    out.unk8008ED1C = runtime.runnerGlobalsCore801C9094.unk8008ED1C;
    out.unk8008ED20 =
        static_cast<uint16_t>(runtime.runnerGlobalsCore801C9094.unk8008ED20);
    out.eventStreamFlag =
        runtime.runnerGlobalsCore801C9094.eventStreamFlag;
    out.eventStreamId = runtime.runnerGlobalsCore801C9094.eventStreamId;
    out.eventStreamDone = runtime.runnerGlobalsCore801C9094.eventStreamDone;
    out.dword801D3048 = runtime.runnerGlobalsCore801C9094.dword801D3048;
    return out;
}

void PrStageEventDirectStage1ApplyRunnerGlobalsCore801C9094(
    PrStageEventDirectStage1Runtime& runtime,
    const PrStageRunnerDirectGlobals801C9094& globals,
    uint32_t queryFrame,
    uint32_t scriptFrame) {
    runtime.runnerGlobalsCore801C9094.known = true;
    runtime.runnerGlobalsCore801C9094.unk8008ED00 = globals.unk8008ED00;
    runtime.runnerGlobalsCore801C9094.unk8008ED1C = globals.unk8008ED1C;
    runtime.runnerGlobalsCore801C9094.unk8008ED20 = globals.unk8008ED20;
    runtime.runnerGlobalsCore801C9094.eventStreamFlag =
        globals.eventStreamFlag;
    runtime.runnerGlobalsCore801C9094.eventStreamId =
        static_cast<uint16_t>(globals.eventStreamId & 0xFFFFu);
    runtime.runnerGlobalsCore801C9094.eventStreamDone =
        globals.eventStreamDone;
    runtime.runnerGlobalsCore801C9094.dword801D3048 =
        globals.dword801D3048;
    runtime.ed00 = globals.unk8008ED00;
    runtime.ed20 = globals.unk8008ED20;
    SetStage1PrimaryEventStreamFlag801C9094(
        runtime,
        globals.eventStreamFlag,
        kStage1EventStreamFlagUpdateReasonRunnerGlobals,
        queryFrame,
        scriptFrame);
    runtime.gPrStageEventStreamId =
        static_cast<uint16_t>(globals.eventStreamId & 0xFFFFu);
    runtime.gPrStageEventStreamDone = globals.eventStreamDone;
    runtime.dword801D3048 = static_cast<uint32_t>(globals.dword801D3048);
    ProjectStage1EventStreamGlobals801C9094(runtime);
}

bool PrStageEventDirectStage1RunnerGlobalsCoreKnown801C9094(
    const PrStageEventDirectStage1Runtime& runtime) {
    return runtime.runnerGlobalsCore801C9094.known;
}

PrStageRunnerDirectFrameUpdate9094Input
PrStageEventDirectStage1BuildRunnerFrameUpdateInput801C9094(
    const PrStageEventDirectStage1Runtime& runtime,
    const PrStage1OverlayData& data,
    bool sameFrameCtxKnown,
    const PrStageRunnerDirectContext801C9094& sameFrameCtx,
    bool sameFrameGlobalsKnown,
    const PrStageRunnerDirectGlobals801C9094& sameFrameGlobals) {
    const PrStageEventDirectStage1RunnerInputBridge801C9094 bridge =
        PrStageEventDirectStage1BuildRunnerInputBridge801C9094(runtime, data);

    PrStageRunnerDirectFrameUpdate9094Input out{};
    out.sameFrameCtxKnown = sameFrameCtxKnown;
    out.sameFrameGlobalsKnown = sameFrameGlobalsKnown;
    out.ctx = sameFrameCtx;
    out.globals = sameFrameGlobals;
    out.activeFlagStream = bridge.activeFlagStream;
    out.activeIdStream = bridge.activeIdStream;
    out.eventStreamFlagDescriptorValid = bridge.flagStreamDescriptorValid;
    out.eventStreamIdDescriptorValid = bridge.idStreamDescriptorValid;
    out.compactRailRecKnown = bridge.compactRail.known;
    out.compactRailRecSignedMode = bridge.compactRail.signedMode;
    out.compactRailRecStartIndex = bridge.compactRail.startIndex;
    out.compactRailRecPrimaryStreamPtr = bridge.compactRail.primaryStreamPtr;
    out.compactRailRecSecondaryStreamPtr =
        bridge.compactRail.secondaryStreamPtr;
    return out;
}

PrStageEventDirectStage1FrameUpdateResult801C9094
PrStageEventDirectStage1RunFrameUpdate801C9094(
    PrStageEventDirectStage1Runtime& runtime,
    const PrStage1OverlayData& data,
    const PrStageEventDirectStage1FrameInput& frameInput,
    bool sameFrameCtxKnown,
    const PrStageRunnerDirectContext801C9094& sameFrameCtx,
    bool sameFrameGlobalsKnown,
    const PrStageRunnerDirectGlobals801C9094& sameFrameGlobals) {
    PrStageEventDirectStage1FrameUpdateResult801C9094 out{};
    PrStageEventDirectStage1Advance(runtime, data, frameInput);
    PrStageRunnerDirectFrameUpdate9094Input runnerInput =
        PrStageEventDirectStage1BuildRunnerFrameUpdateInput801C9094(
            runtime,
            data,
            sameFrameCtxKnown,
            sameFrameCtx,
            sameFrameGlobalsKnown,
            PrStageEventDirectStage1BuildRunnerGlobalsCore801C9094(
                runtime,
                sameFrameGlobals));
    PopulateRunnerCursorFromConsumedEvent801C9094(
        runtime.flagStreamEvent801C9094,
        runnerInput.activeFlagStream,
        runnerInput.eventStreamFlagDescriptorValid);
    PopulateRunnerCursorFromConsumedEvent801C9094(
        runtime.idStreamEvent801C9094,
        runnerInput.activeIdStream,
        runnerInput.eventStreamIdDescriptorValid);
    out.runnerInput = runnerInput;
    out.frameUpdate = PrStageRunnerDirectFrameUpdate801C9094(runnerInput);
    out.ctx0FinalFlags =
        PrStageRunnerDirectMakeCtx0FinalFlagsSnapshot801C9094(out.frameUpdate);
    out.ran = true;
    ApplyStage1RunnerTerminalFlag100Result801C9094(
        runtime,
        out.frameUpdate);
    PrStageEventDirectStage1ApplyRunnerGlobalsCore801C9094(
        runtime,
        out.frameUpdate.globals,
        frameInput.queryFrame,
        frameInput.scriptFrame);
    if (out.frameUpdate.returnEarlyAfterFlag40Reset) {
        ApplyStage1Flag40EarlyReturnReset801C9094(
            runtime,
            data,
            frameInput,
            out.frameUpdate);
    } else {
        runtime.frameResult801C9094.setTextEvent801C8604 =
            ResolveStage1SetTextEvent801C8604(runtime, out.frameUpdate.trace);
    }
    return out;
}

bool PrStageEventDirectStage1HasFlag(
    const PrStageEventDirectStage1Runtime& runtime,
    uint16_t mask) {
    return (runtime.flags40_2000_4000 & mask) != 0u;
}

uint8_t PrStageEventDirectStage1ResolveCurrentMode801C9094(
    uint8_t rightRankActiveRow) {
    return rightRankActiveRow > 3u ? 3u : rightRankActiveRow;
}

PrStageEventDirectStage1FrameInput
PrStageEventDirectStage1BuildFrameInputFromCarrier801C9094(
    const PrStageEventDirectStage1FrameCarrier801C9094& carrier) {
    PrStageEventDirectStage1FrameInput input{};
    input.queryFrame = carrier.queryFrame;
    input.scriptFrame = carrier.tick96 >= 0
                            ? static_cast<uint32_t>(carrier.tick96)
                            : carrier.queryFrame;
    input.tick96 = carrier.tick96;
    input.allowSameQueryRefresh = carrier.allowSameQueryRefresh;
    input.rightRankActiveRow = carrier.rightRankActiveRow;
    input.currentMode =
        PrStageEventDirectStage1ResolveCurrentMode801C9094(
            input.rightRankActiveRow);
    input.bucket0Ctx118WritePulse = carrier.bucket0Ctx118WritePulse;
    input.flag2000Pulse = carrier.flag2000Pulse;
    input.flag4000Pulse = carrier.flag4000Pulse;
    input.clearTerminalTailPulse = carrier.clearTerminalTailPulse;
    input.forcedGoodEventStreamDone = carrier.forcedGoodEventStreamDone;
    input.ctx72 = carrier.ctx72;
    input.ed24 = carrier.ed24;
    input.ed00 = carrier.ed00;
    input.ed14 = carrier.ed14;
    input.ed0c = carrier.ed0c;
    input.steadySfxDelayedCompletionPending =
        carrier.steadySfxDelayedCompletionPending;
    input.followUpPhase = carrier.followUpPhase;
    return input;
}

PrStageEventDirectStage1FrameInput
PrStageEventDirectStage1BuildFrameInputFromPrimitiveFacts801C9094(
    uint32_t queryFrame,
    int32_t tick96,
    bool allowSameQueryRefresh,
    uint8_t rightRankActiveRow,
    bool bucket0Ctx118WritePulse,
    bool flag2000Pulse,
    bool flag4000Pulse,
    bool clearTerminalTailPulse,
    bool forcedGoodEventStreamDone,
    uint16_t ctx72,
    int32_t ed24,
    int32_t ed00,
    int32_t ed14,
    int32_t ed0c,
    bool steadySfxDelayedCompletionPending,
    uint8_t followUpPhase) {
    PrStageEventDirectStage1FrameCarrier801C9094 carrier{};
    carrier.queryFrame = queryFrame;
    carrier.tick96 = tick96;
    carrier.allowSameQueryRefresh = allowSameQueryRefresh;
    carrier.rightRankActiveRow = rightRankActiveRow;
    carrier.bucket0Ctx118WritePulse = bucket0Ctx118WritePulse;
    carrier.flag2000Pulse = flag2000Pulse;
    carrier.flag4000Pulse = flag4000Pulse;
    carrier.clearTerminalTailPulse = clearTerminalTailPulse;
    carrier.forcedGoodEventStreamDone = forcedGoodEventStreamDone;
    carrier.ctx72 = ctx72;
    carrier.ed24 = ed24;
    carrier.ed00 = ed00;
    carrier.ed14 = ed14;
    carrier.ed0c = ed0c;
    carrier.steadySfxDelayedCompletionPending =
        steadySfxDelayedCompletionPending;
    carrier.followUpPhase = followUpPhase;
    carrier.helper801C9094PseudoCKnown = true;
    return PrStageEventDirectStage1BuildFrameInputFromCarrier801C9094(
        carrier);
}

void PrStageEventDirectStage1Advance(
    PrStageEventDirectStage1Runtime& runtime,
    const PrStage1OverlayData& data,
    const PrStageEventDirectStage1FrameInput& input) {
    const uint32_t queryFrame = input.queryFrame;
    const uint32_t scriptFrame = input.scriptFrame;
    if (runtime.lastQueryFrame >= 0 &&
        (int32_t)queryFrame < runtime.lastQueryFrame) {
        PrStageEventDirectStage1Reset(runtime);
    }
    const bool forceMutationRefresh =
        input.clearTerminalTailPulse ||
        input.bucket0Ctx118WritePulse ||
        input.flag2000Pulse ||
        input.flag4000Pulse;
    const bool projectionOnlyRefresh =
        runtime.lastQueryFrame == (int32_t)queryFrame &&
        input.allowSameQueryRefresh &&
        !forceMutationRefresh;
    if (runtime.lastQueryFrame == (int32_t)queryFrame &&
        !input.allowSameQueryRefresh &&
        !forceMutationRefresh) {
        return;
    }
    runtime.lastQueryFrame = (int32_t)queryFrame;
    if (!projectionOnlyRefresh) {
        runtime.flagStreamEvent801C9094 =
            PrStageEventDirectStage1ConsumedScriptEvent801C9094{};
        runtime.idStreamEvent801C9094 =
            PrStageEventDirectStage1ConsumedScriptEvent801C9094{};
        runtime.frameResult801C9094 =
            PrStageEventDirectStage1FrameResult801C9094{};
        runtime.flag100BlocksWaitPulse = false;
        runtime.flag100SourceStream = 0u;
        runtime.gPrStageEventStreamDone = runtime.eventStreamDonePending;
        runtime.eventStreamDonePulse = runtime.eventStreamDonePending;
        runtime.eventStreamDoneSourceStream =
            runtime.eventStreamDonePending
                ? runtime.eventStreamDonePendingSourceStream
                : 0u;
        runtime.eventStreamDonePending = false;
        runtime.eventStreamDonePendingSourceStream = 0u;
        runtime.lastAdvanceScriptFrame = scriptFrame;
        runtime.lastClearTerminalTailPulseInput =
            input.clearTerminalTailPulse;
        runtime.lastClearTerminalTailPulseArmed = false;
        runtime.lastClearTerminalTailPulseBlockedAlreadyArmed = false;
        runtime.lastClearTerminalTailPulseBlockedActiveDispatch = false;
        runtime.lastClearTerminalTailPulseBlockedPendingMismatch = false;
        runtime.lastClearTerminalTailPulseStream = 0u;
    }

    const bool flag40BootstrapRequested =
        ResolveStage1EventStreamFlag40BootstrapPulse(input);

    const bool goodToCoolCommitted = input.flag2000Pulse;
    if (goodToCoolCommitted) {
        SetStage1EventStreamFlag(runtime, 0x2000u, true);
        runtime.ctx72 = input.ctx72;
        runtime.ed24 = input.ed24;
        runtime.ed00 = input.ed00;
        runtime.ed14 = input.ed14;
        runtime.ed0c = input.ed0c;
    }

    const bool flag4000Requested = input.flag4000Pulse;
    const uint16_t bootstrapMask =
        (flag40BootstrapRequested ? 0x0040u : 0u) |
        (goodToCoolCommitted ? 0x2000u : 0u) |
        (flag4000Requested ? 0x4000u : 0u);

    if (!projectionOnlyRefresh && runtime.clearTerminalTailLatchPending801C9094) {
        runtime.clearTerminalTailLatchPending801C9094 = false;
        SetStage1PrimaryEventStreamFlag801C9094(
            runtime,
            0u,
            kStage1EventStreamFlagUpdateReasonClearTailService,
            queryFrame,
            scriptFrame);
        ProjectStage1EventStreamGlobals801C9094(runtime);
        runtime.clearTerminalTailArmed = true;
        runtime.clearTerminalTailStream =
            ResolveStage1EventStreamTailStreamForMode(input.currentMode);
        runtime.lastClearTerminalTailServiceLatch = true;
        runtime.lastClearTerminalTailServiceScriptFrame = scriptFrame;
        runtime.lastClearTerminalTailServiceRightRankRow =
            input.rightRankActiveRow;
        runtime.lastClearTerminalTailServiceCurrentMode = input.currentMode;
        runtime.lastClearTerminalTailServiceStream =
            runtime.clearTerminalTailStream;
        runtime.lastClearTerminalTailPulseArmed = true;
        runtime.lastClearTerminalTailPulseStream =
            runtime.clearTerminalTailStream;
        if (const PrStage1EventStream* stream =
                data.FindStream(runtime.clearTerminalTailStream)) {
            StartStage1EventStreamDispatch(
                runtime,
                *stream,
                runtime.clearTerminalTailStream,
                queryFrame,
                scriptFrame);
            runtime.tailDispatchFamilyActive = true;
            runtime.tailDispatchFamilyStream = runtime.clearTerminalTailStream;
            runtime.clearTerminalTailDispatchActive = true;
        }
    }

    if (!projectionOnlyRefresh && input.clearTerminalTailPulse) {
        runtime.clearTerminalTailLatchPending801C9094 = true;
    }

    if (!projectionOnlyRefresh &&
        input.forcedGoodEventStreamDone &&
        !runtime.prevForcedGoodEventStreamDone) {
        runtime.gPrStageEventStreamDone = true;
        runtime.eventStreamDonePending = true;
        runtime.eventStreamDonePendingSourceStream = 1u;
    }

    if (!projectionOnlyRefresh && runtime.gPrStageEventStreamFlag != 0u) {
        PrStageEventDirectStage1ConsumeNextEvent801C9094(
            runtime,
            data,
            runtime.gPrStageEventStreamFlag,
            scriptFrame,
            0u,
            runtime.flagStreamEvent801C9094);
    }

    if (!projectionOnlyRefresh &&
        ShouldBootstrapStage1EventStreamFlag40Dispatch(runtime, bootstrapMask)) {
        runtime.pendingRatingBranchSeq++;
    }
    if (!projectionOnlyRefresh &&
        runtime.activeDispatchStream != 0u) {
        const uint8_t previousActiveDispatchStream = runtime.activeDispatchStream;
        const PrStage1ScriptEvent* consumedEvent801C9094 =
            PrStageEventDirectStage1ConsumeNextEvent801C9094(
                runtime,
                data,
                previousActiveDispatchStream,
                scriptFrame,
                runtime.dword801D3048,
                runtime.idStreamEvent801C9094);
        const bool beforeDispatchStart =
            scriptFrame < runtime.activeDispatchStartScriptFrame;
        const uint32_t localFrame =
            beforeDispatchStart
                ? 0u
                : (scriptFrame - runtime.activeDispatchStartScriptFrame);
        const bool descCursorDone801C9094 =
            PrStageEventDirectStage1DescDone801C9094(
                runtime,
                data,
                previousActiveDispatchStream);
        const bool terminalFlag100ReadyForFrameUpdate =
            !beforeDispatchStart &&
            IsStage1EventStreamTerminalFlag100Source(
                previousActiveDispatchStream) &&
            !consumedEvent801C9094 &&
            descCursorDone801C9094;
        if (!beforeDispatchStart &&
            PrStageEventDirectStage1IsTerminalStream(
                previousActiveDispatchStream) &&
            !runtime.activeDispatchTerminalPulseEmitted &&
            ((!consumedEvent801C9094 && descCursorDone801C9094) ||
             localFrame > runtime.activeDispatchTerminalEndLocalFrame)) {
            runtime.flag100BlocksWaitPulse = true;
            runtime.flag100SourceStream = previousActiveDispatchStream;
            runtime.activeDispatchTerminalPulseEmitted = true;
        }
        if (!beforeDispatchStart &&
            PrStageEventDirectStage1IsFlag2000Stream(
                previousActiveDispatchStream) &&
            !runtime.activeDispatchDonePulseEmitted &&
            ((!consumedEvent801C9094 && descCursorDone801C9094) ||
             localFrame > runtime.activeDispatchTerminalEndLocalFrame)) {
            runtime.gPrStageEventStreamDone = true;
            runtime.eventStreamDonePending = true;
            runtime.eventStreamDonePendingSourceStream =
                previousActiveDispatchStream;
            runtime.activeDispatchDonePulseEmitted = true;
        }
        if (beforeDispatchStart ||
            (descCursorDone801C9094 && !consumedEvent801C9094) ||
            localFrame > runtime.activeDispatchEndLocalFrame) {
            if (!terminalFlag100ReadyForFrameUpdate) {
                if (descCursorDone801C9094 &&
                    !consumedEvent801C9094 &&
                    previousActiveDispatchStream < runtime.unk801D2D64.size()) {
                    runtime.unk801D2D64[previousActiveDispatchStream].cursor = 0u;
                }
                if (runtime.tailDispatchFamilyActive &&
                    runtime.tailDispatchFamilyStream ==
                        previousActiveDispatchStream) {
                    runtime.tailDispatchFamilyActive = false;
                    runtime.tailDispatchFamilyStream = 0u;
                }
                if (runtime.clearTerminalTailDispatchActive &&
                    runtime.clearTerminalTailStream ==
                        previousActiveDispatchStream) {
                    runtime.clearTerminalTailDispatchActive = false;
                    runtime.clearTerminalTailStream = 0u;
                }
                if (PrStageEventDirectStage1IsFlag2000Stream(
                        previousActiveDispatchStream) &&
                    runtime.flag2000SceneFamilyStream ==
                        previousActiveDispatchStream) {
                    runtime.flag2000SceneFamilyActive = false;
                    runtime.flag2000SceneFamilyStream = 0u;
                    runtime.flag2000SceneFamilyStartFrame = 0u;
                    runtime.flag2000SceneFamilyStartScriptFrame = 0u;
                }
                runtime.activeDispatchStream = 0u;
                runtime.activeDispatchStartFrame = 0u;
                runtime.activeDispatchStartScriptFrame = 0u;
                runtime.activeDispatchEndLocalFrame = 0u;
                runtime.activeDispatchTerminalEndLocalFrame = 0u;
                runtime.activeDispatchTerminalPulseEmitted = false;
                runtime.activeDispatchDonePulseEmitted = false;
                runtime.gPrStageEventStreamId = 0u;
            }
        }
    }

    const bool followUpDelayActive =
        input.ed14 != 0 &&
        input.tick96 <= input.ed0c;
    if (!followUpDelayActive) {
        runtime.ed14 = 0;
        runtime.ed0c = 0;
    } else {
        runtime.ed14 = input.ed14;
        runtime.ed0c = input.ed0c;
    }

    const bool flag2000Keep = goodToCoolCommitted;
    SetStage1EventStreamFlag(runtime, 0x2000u, flag2000Keep);

    const bool flag40Pending =
        runtime.pendingRatingBranchSeq > runtime.consumedRatingBranchSeq &&
        !runtime.clearTerminalTailArmed;
    const bool flag40ActiveStream =
        PrStageEventDirectStage1IsFlag40Stream(runtime.activeDispatchStream);
    const bool flag40Latched =
        flag40BootstrapRequested ||
        PrStageEventDirectStage1HasFlag(runtime, 0x0040u);
    const bool flag40AwaitingInitialDispatch =
        flag40Latched &&
        !flag40Pending &&
        !flag40ActiveStream &&
        runtime.flags40LastDispatchedStream == 0u;
    const bool flag40AwaitingCoolFollowUp =
        flag40Latched &&
        !flag40Pending &&
        !flag40ActiveStream &&
        runtime.flags40LastDispatchedStream == 4u &&
        input.currentMode == 0u;
    SetStage1EventStreamFlag(
        runtime,
        0x0040u,
        flag40BootstrapRequested ||
            flag40Pending ||
            flag40ActiveStream ||
            flag40AwaitingInitialDispatch ||
            flag40AwaitingCoolFollowUp);

    const bool flag4000Keep = flag4000Requested;
    SetStage1EventStreamFlag(runtime, 0x4000u, flag4000Keep);

    if (!PrStageEventDirectStage1HasFlag(runtime, 0x0040u) &&
        !PrStageEventDirectStage1HasFlag(runtime, 0x2000u)) {
        runtime.ed24 = 0;
    }

    if (input.ed00 != 0) {
        runtime.ed00 = input.ed00;
    } else if (input.steadySfxDelayedCompletionPending) {
        runtime.ed00 = 8;
    } else {
        runtime.ed00 = 0;
    }

    if (!projectionOnlyRefresh) {
        RefreshStage1EventStreamDispatchEdges(runtime);
    }

    if (!projectionOnlyRefresh &&
        runtime.activeDispatchStream == 0u) {
        if (runtime.pendingRatingBranchSeq > runtime.consumedRatingBranchSeq) {
            const uint8_t streamId =
                runtime.clearTerminalTailArmed &&
                        runtime.clearTerminalTailStream != 0u
                    ? runtime.clearTerminalTailStream
                    : ResolveStage1EventStreamFollowUpStreamForFlag40(
                          input.currentMode);
            if (const PrStage1EventStream* stream = data.FindStream(streamId)) {
                StartStage1EventStreamDispatch(
                    runtime,
                    *stream,
                    streamId,
                    queryFrame,
                    scriptFrame);
                const bool clearTailDispatch =
                    runtime.clearTerminalTailArmed &&
                    runtime.clearTerminalTailStream == streamId;
                if (clearTailDispatch) {
                    ConsumeStartedStage1EventStreamDispatch801C9094(
                        runtime,
                        data,
                        streamId,
                        scriptFrame);
                }
                runtime.tailDispatchFamilyActive = clearTailDispatch;
                runtime.tailDispatchFamilyStream =
                    clearTailDispatch ? streamId : 0u;
                runtime.clearTerminalTailDispatchActive =
                    runtime.clearTerminalTailArmed;
                runtime.consumedRatingBranchSeq = runtime.pendingRatingBranchSeq;
                if (!clearTailDispatch &&
                    PrStageEventDirectStage1IsFlag40Stream(streamId)) {
                    runtime.flags40LastDispatchedStream = streamId;
                }
            }
        } else if (PrStageEventDirectStage1HasFlag(runtime, 0x0040u)) {
            const uint8_t streamId =
                ResolveStage1EventStreamFollowUpStreamForFlag40(input.currentMode);
            if (streamId != runtime.flags40LastDispatchedStream) {
                if (const PrStage1EventStream* stream = data.FindStream(streamId)) {
                    StartStage1EventStreamDispatch(
                        runtime,
                        *stream,
                        streamId,
                        queryFrame,
                        scriptFrame);
                    runtime.tailDispatchFamilyActive = false;
                    runtime.tailDispatchFamilyStream = 0u;
                    runtime.flags40LastDispatchedStream = streamId;
                }
            }
        } else if (PrStageEventDirectStage1HasFlag(runtime, 0x2000u)) {
            const uint8_t streamId =
                ResolveStage1EventStreamFollowUpStreamForFlag2000(runtime.ctx72);
            if (streamId != runtime.flags2000LastDispatchedStream) {
                if (const PrStage1EventStream* stream = data.FindStream(streamId)) {
                    StartStage1EventStreamDispatch(
                        runtime,
                        *stream,
                        streamId,
                        queryFrame,
                        scriptFrame);
                    ConsumeStartedStage1EventStreamDispatch801C9094(
                        runtime,
                        data,
                        streamId,
                        scriptFrame);
                    runtime.tailDispatchFamilyActive = false;
                    runtime.tailDispatchFamilyStream = 0u;
                    runtime.flag2000SceneFamilyActive = true;
                    runtime.flag2000SceneFamilyStream = streamId;
                    runtime.flag2000SceneFamilyStartFrame = scriptFrame;
                    runtime.flag2000SceneFamilyStartScriptFrame = scriptFrame;
                    runtime.flags2000LastDispatchedStream = streamId;
                }
            }
        } else if (PrStageEventDirectStage1HasFlag(runtime, 0x4000u)) {
            constexpr uint8_t kStage1LateBranchTailStreamId = 7u;
            if (kStage1LateBranchTailStreamId !=
                runtime.flags4000LastDispatchedStream) {
                if (const PrStage1EventStream* stream =
                        data.FindStream(kStage1LateBranchTailStreamId)) {
                    StartStage1EventStreamDispatch(
                        runtime,
                        *stream,
                        kStage1LateBranchTailStreamId,
                        queryFrame,
                        scriptFrame);
                    ConsumeStartedStage1EventStreamDispatch801C9094(
                        runtime,
                        data,
                        kStage1LateBranchTailStreamId,
                        scriptFrame);
                    runtime.tailDispatchFamilyActive = false;
                    runtime.tailDispatchFamilyStream = 0u;
                    runtime.flags4000LastDispatchedStream =
                        kStage1LateBranchTailStreamId;
                }
            }
        }
    }

    runtime.selectedStream = runtime.activeDispatchStream;
    ProjectStage1EventStreamGlobals801C9094(runtime);
    runtime.prevFollowUpPhase = input.followUpPhase;
    runtime.prevForcedGoodEventStreamDone =
        input.forcedGoodEventStreamDone;
    runtime.frameResult801C9094 =
        BuildStage1FrameResult801C9094(runtime, queryFrame, scriptFrame);
}
