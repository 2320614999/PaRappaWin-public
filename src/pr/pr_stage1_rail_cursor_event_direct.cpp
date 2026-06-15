#include "pr_stage1_rail_cursor_event_direct.h"

namespace {

constexpr uint32_t kStage1CompactRailRec44BaseAddr801C9094 =
    PrStage1OverlayData::kCompactRailRec44BaseAddr;
constexpr uint32_t kStage1CompactRailRec44Size801C9094 =
    PrStage1OverlayData::kCompactRailRec44Size;
constexpr uint32_t kStage1CompactRailLaneAOffset801C9094 = 0x04u;
constexpr uint32_t kStage1CompactRailLaneBOffset801C9094 = 0x18u;
constexpr uint32_t kStage1CompactTeacherBootstrapFlag801C9094 = 0x00000800u;
constexpr uint32_t kStage1CompactResetFlag801C9094 = 0x00001000u;
constexpr uint32_t kStage1CompactStudentCatchupFlag801C9094 = 0x00100000u;
constexpr uint32_t kStage1TerminalTailFlag801C9094 = 0x00000080u;

struct Stage1RailResolvedEvent801C9094 {
    const PrStage1EventStream* stream = nullptr;
    uint8_t streamId = 0;
    const PrStage1ScriptEvent* event = nullptr;
    uint32_t dueFrame = 0;
};

bool ResolveStage1RailCursorEventIndex(const PrStage1EventStream& stream,
                                       const PrStage1ScriptEvent& ev,
                                       int32_t& outIndex) {
    outIndex = -1;
    if (stream.events.empty()) {
        return false;
    }

    const PrStage1ScriptEvent* begin = stream.events.data();
    const PrStage1ScriptEvent* end = begin + stream.events.size();
    if (&ev >= begin && &ev < end) {
        outIndex = (int32_t)(&ev - begin);
        return true;
    }

    for (size_t i = 0; i < stream.events.size(); ++i) {
        if (stream.events[i].psxAddr == ev.psxAddr) {
            outIndex = (int32_t)i;
            return true;
        }
    }
    return false;
}

uint32_t ResolveStage1RailCursorStreamBaseDueFrame(
    const PrStage1ScriptEvent& activeEvent,
    uint32_t activeDueFrame) {
    return (activeDueFrame >= activeEvent.frame)
               ? (activeDueFrame - activeEvent.frame)
               : 0u;
}

bool ResolveStage1RailConsumedEvent801C9094(
    const PrStage1OverlayData& data,
    const PrStageEventDirectStage1ConsumedScriptEvent801C9094& consumed,
    Stage1RailResolvedEvent801C9094& out) {
    out = Stage1RailResolvedEvent801C9094{};
    if (!consumed.valid || consumed.streamId == 0u) {
        return false;
    }

    const PrStage1EventStream* stream = data.FindStream(consumed.streamId);
    if (!stream || consumed.eventIndex >= stream->events.size()) {
        return false;
    }

    const PrStage1ScriptEvent& ev =
        stream->events[static_cast<size_t>(consumed.eventIndex)];
    if (ev.psxAddr != consumed.psxAddr) {
        return false;
    }

    out.stream = stream;
    out.streamId = consumed.streamId;
    out.event = &ev;
    out.dueFrame = consumed.dueFrame;
    return true;
}

}  // namespace

bool PrStage1RailCursorEventDirectShouldResetForFlags(
    uint16_t lateBranchFlags40_2000_4000,
    const PrStage1ScriptEvent& ev) {
    if ((lateBranchFlags40_2000_4000 & 0x40u) != 0u ||
        (lateBranchFlags40_2000_4000 & 0x2000u) != 0u) {
        return true;
    }
    return (ev.flags04 & 0x00001000u) != 0u;
}

void PrStage1RailCursorEventDirectPopulateEventFields801C9094(
    uint16_t lateBranchFlags40_2000_4000,
    const PrStage1ScriptEvent& ev,
    PrStage1RailCursorEventDirectResult& out) {
    out.activeEvent = &ev;
    out.activeEventAvailable = true;
    out.activeEventPsxAddr = ev.psxAddr;
    out.activeEventFlags04 = ev.flags04;
    out.activeEventByte1D = ev.byte1D;
    out.activeEventByte1E = ev.byte1E;
    out.activeEventByte1F = ev.byte1F;
    out.activeEventTextId = ev.textId;
    out.compactTeacherBootstrapRequested801C9094 =
        (ev.flags04 & kStage1CompactTeacherBootstrapFlag801C9094) != 0u;
    out.compactResetRequested801C9094 =
        !out.compactTeacherBootstrapRequested801C9094 &&
        ((ev.flags04 & kStage1CompactResetFlag801C9094) != 0u ||
         (lateBranchFlags40_2000_4000 & 0x2000u) != 0u);
    out.compactStudentCatchupRequested801C9094 =
        (ev.flags04 & kStage1CompactStudentCatchupFlag801C9094) != 0u;
    out.compactEventBearing801C9094 =
        out.compactResetRequested801C9094 ||
        out.compactTeacherBootstrapRequested801C9094 ||
        out.compactStudentCatchupRequested801C9094;
    out.terminalTailRequested801C9094 =
        (ev.flags04 & kStage1TerminalTailFlag801C9094) != 0u;

    const uint32_t teacherRowOffset =
        static_cast<uint32_t>(ev.byte1D) *
        kStage1CompactRailRec44Size801C9094;
    const uint32_t teacherLaneBOffset =
        static_cast<uint32_t>(ev.byte1E) *
        kStage1CompactRailRec44Size801C9094;
    out.compactTeacherRowBasePsxAddr801C9094 =
        kStage1CompactRailRec44BaseAddr801C9094 + teacherRowOffset;
    out.compactTeacherLaneAPsxAddr801C9094 =
        kStage1CompactRailRec44BaseAddr801C9094 +
        teacherRowOffset +
        kStage1CompactRailLaneAOffset801C9094;
    out.compactTeacherLaneBPsxAddr801C9094 =
        kStage1CompactRailRec44BaseAddr801C9094 +
        teacherLaneBOffset +
        kStage1CompactRailLaneBOffset801C9094;
}

void PrStage1RailCursorEventDirectAdvance(
    const PrStage1RailCursorEventDirectInput& input,
    PrStage1RailCursorEventDirectResult& out) {
    out = PrStage1RailCursorEventDirectResult{};
    out.runtimeActive = input.runtimeActive;
    out.runtimeActiveStreamId = input.runtimeActiveStreamId;
    out.lastAppliedEventPsxAddr = input.previousLastAppliedEventPsxAddr;
    out.lastAppliedEventDueFrame = input.previousLastAppliedEventDueFrame;

    if (!input.data) {
        return;
    }

    PrStageEventDirectStage1ConsumedScriptEvent801C9094 compactSource{};
    if (input.compactEvent801C9094.valid) {
        compactSource = input.compactEvent801C9094;
    } else if (input.consumedEventSource ==
               PrStageEventDirectStage1ConsumedEventSource801C9094::FlagStream) {
        compactSource = input.consumedEvent801C9094;
    }

    Stage1RailResolvedEvent801C9094 activeRuntimeEvent{};
    bool haveActiveEvent =
        ResolveStage1RailConsumedEvent801C9094(*input.data,
                                               compactSource,
                                               activeRuntimeEvent);

    if (haveActiveEvent) {
        out.activeStream = activeRuntimeEvent.stream;
        out.activeStreamId = activeRuntimeEvent.streamId;
        out.activeEvent = activeRuntimeEvent.event;
        out.activeDueFrame = activeRuntimeEvent.dueFrame;
        out.activeEventAvailable = true;
        out.activeEventPsxAddr = activeRuntimeEvent.event->psxAddr;
        PrStage1RailCursorEventDirectPopulateEventFields801C9094(
            input.lateBranchFlags40_2000_4000,
            *activeRuntimeEvent.event,
            out);
        out.activeEventChangedThisFrame =
            activeRuntimeEvent.event->psxAddr != input.previousLastAppliedEventPsxAddr ||
            activeRuntimeEvent.dueFrame != input.previousLastAppliedEventDueFrame;

        if (input.runtimeActive && input.runtimeActiveStreamId != 0u &&
            input.runtimeActiveStreamId != activeRuntimeEvent.streamId) {
            out.hardReset = true;
        }

        out.runtimeActive = true;
        out.runtimeActiveStreamId = activeRuntimeEvent.streamId;
        out.streamBaseDueFrame = ResolveStage1RailCursorStreamBaseDueFrame(
            *activeRuntimeEvent.event,
            activeRuntimeEvent.dueFrame);
        if (activeRuntimeEvent.stream != nullptr) {
            ResolveStage1RailCursorEventIndex(*activeRuntimeEvent.stream,
                                              *activeRuntimeEvent.event,
                                              out.activeEventIndex);
        }

        out.activeEventShouldResetAuthority =
            PrStage1RailCursorEventDirectShouldResetForFlags(
                input.lateBranchFlags40_2000_4000,
                *activeRuntimeEvent.event);
        if (out.activeEventChangedThisFrame) {
            out.lastAppliedEventPsxAddr = activeRuntimeEvent.event->psxAddr;
            out.lastAppliedEventDueFrame = activeRuntimeEvent.dueFrame;
            out.activeEventResetAuthorityThisFrame =
                out.activeEventShouldResetAuthority;
        }
        return;
    }

    if (!input.runtimeActive) {
        out.hardReset = true;
        out.returnAfterHardReset = true;
        out.runtimeActive = false;
        out.runtimeActiveStreamId = 0u;
    }
}
