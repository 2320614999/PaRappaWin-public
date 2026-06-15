#include "pr_stage1_runtime_slots_direct.h"

#include "pr_scn1.h"
#include "pr_stage_event_direct.h"
#include "pr_stage1_rail_cursor_event_direct.h"
#include "pr_stage1_script_event_runtime_direct.h"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <string>
#include <vector>

namespace {

constexpr uint32_t kStage1SceneBaseRowFlag = 0x00000020u;
constexpr uint32_t kStage1PairFlagPaByCoolness = 0x00010000u;
constexpr uint32_t kStage1PairFlagOnByMode = 0x00020000u;
constexpr uint32_t kStage1PerformanceTodFlag = 0x00040000u;
constexpr uint32_t kStage1RatingPresentationFrameUpdatePsxAddr = 0x801C9094u;
constexpr uint32_t kStage1HudSetRes10000PsxAddr = 0x801C88C8u;
constexpr uint32_t kStage1HudSetRes20000PsxAddr = 0x801C8828u;
constexpr uint32_t kStage1HudOverlayConfigure801CA0E0PsxAddr = 0x801CA0E0u;
constexpr uint32_t kStage1AcceptedTail9443CPsxAddr = 0x801CA264u;
constexpr uint32_t kStage1EventStreamDonePsxAddr = 0x801C9094u;
constexpr uint32_t kStage1UiListSeedFlag801CBFDC = 0x00800000u;
constexpr uint32_t kStage1CameraBezFlag801CBFDC = 0x00000400u;
constexpr uint32_t kStage1LateBranchCtxFlags40_2000_4000 =
    0x00000040u | 0x00002000u | 0x00004000u;
constexpr uint8_t kStage1DefaultOnPairIndex = 14u;
constexpr uint8_t kStage1DefaultPaPairIndex = 35u;
constexpr uint8_t kStage1DefaultRuntimeEventStreamId = 1u;
constexpr uint8_t kStage1HudSetRes10000TimedListSlot = 3u;
constexpr int32_t kStage1Word8008ECFESetResLoopFrames = 33;
constexpr int32_t kStage1Dword801D3054RatingGateFrames = 41;
constexpr int32_t kStage1Word8008ECFECoolRecoveryFrames = 132;
constexpr uint16_t kStage1PaWt0DatHandle = 150u;
constexpr uint16_t kStage1PaWt0VdfHandle = 98u;
constexpr uint16_t kStage1OnWt1DatHandle = 131u;
constexpr uint16_t kStage1OnWt1VdfHandle = 79u;
constexpr int16_t kStage1CompactStudentCursor9EInit801C9094 = 0;
constexpr int16_t kStage1CompactSecondCursorInit801C9094 = -1;

struct Stage1RuntimeDueScriptEvent {
    const PrStage1EventStream* stream = nullptr;
    uint8_t streamId = 0;
    const PrStage1ScriptEvent* event = nullptr;
    uint32_t eventIndex = 0;
    uint32_t dueFrame = 0;
    uint32_t applyScriptFrame = 0;
    uint32_t applyFrame = 0;
};

uint32_t ResolveStage1RuntimeStreamWindowFrames(const PrStage1EventStream* stream) {
    if (!stream) {
        return 0u;
    }

    uint32_t endFrame = 0u;
    for (const auto& ev : stream->events) {
        endFrame = (std::max)(endFrame, ev.frame);
    }
    return endFrame;
}

std::string ToLowerAsciiCopy(std::string s) {
    for (char& ch : s) {
        ch = (char)std::tolower((unsigned char)ch);
    }
    return s;
}

bool EndsWithAscii(const std::string& s, const char* suffix) {
    if (!suffix) {
        return false;
    }
    const size_t suffixLen = std::strlen(suffix);
    return s.size() >= suffixLen &&
           s.compare(s.size() - suffixLen, suffixLen, suffix) == 0;
}

bool ResolveStage1RuntimePairHandles(const PrStage1OverlayData& data,
                                     uint8_t pairIndex,
                                     uint16_t& outDatHandle,
                                     uint16_t& outVdfHandle) {
    outDatHandle = 0;
    outVdfHandle = 0;

    const PrStage1PairEntry* entry = data.FindPairEntry(pairIndex);
    if (!entry) {
        return false;
    }

    auto assignHandle = [&](int16_t handle) {
        if (handle <= 0) {
            return;
        }

        const std::string* memName = data.FindMemHandleName((uint16_t)handle);
        if (!memName || memName->empty()) {
            return;
        }

        const std::string lowerName = ToLowerAsciiCopy(*memName);
        if (EndsWithAscii(lowerName, ".dat")) {
            outDatHandle = (uint16_t)handle;
        } else if (EndsWithAscii(lowerName, ".vdf")) {
            outVdfHandle = (uint16_t)handle;
        }
    };

    assignHandle(entry->idA);
    assignHandle(entry->idB);
    return outDatHandle != 0u && outVdfHandle != 0u;
}

bool IsStage1RuntimeBranchStream(uint8_t streamId) {
    return streamId != 0u && streamId != kStage1DefaultRuntimeEventStreamId;
}

void ClearStage1RuntimePrepassEvent(
    PrStage1RuntimeSlotsDirectRuntime& runtime) {
    runtime.prepassEventAvailable = false;
    runtime.prepassEventStreamId = 0u;
    runtime.prepassEventIndex = 0u;
    runtime.prepassEventPsxAddr = 0u;
    runtime.prepassEventDueFrame = 0u;
}

void SetStage1RuntimePrepassEventFrom801C9094(
    PrStage1RuntimeSlotsDirectRuntime& runtime,
    const PrStageEventDirectStage1ConsumedScriptEvent801C9094& consumed) {
    runtime.prepassEventAvailable = consumed.valid;
    runtime.prepassEventStreamId = consumed.valid ? consumed.streamId : 0u;
    runtime.prepassEventIndex = consumed.valid ? consumed.eventIndex : 0u;
    runtime.prepassEventPsxAddr = consumed.valid ? consumed.psxAddr : 0u;
    runtime.prepassEventDueFrame = consumed.valid ? consumed.dueFrame : 0u;
}

bool Stage1ConsumedEvent801C9094Equals(
    const PrStageEventDirectStage1ConsumedScriptEvent801C9094& lhs,
    const PrStageEventDirectStage1ConsumedScriptEvent801C9094& rhs) {
    return lhs.valid == rhs.valid &&
           lhs.streamId == rhs.streamId &&
           lhs.eventIndex == rhs.eventIndex &&
           lhs.dueFrame == rhs.dueFrame &&
           lhs.psxAddr == rhs.psxAddr;
}

bool HasNewStage1RuntimeConsumedEvent801C9094(
    const PrStage1RuntimeSlotsDirectRuntime& runtime,
    const PrStage1RuntimeSlotsDirectLateBranchInput& input) {
    if (input.consumedEventSource ==
            PrStageEventDirectStage1ConsumedEventSource801C9094::None ||
        !input.consumedEvent801C9094.valid) {
        return false;
    }
    return runtime.lastConsumedEventSource801C9094 !=
               input.consumedEventSource ||
           !Stage1ConsumedEvent801C9094Equals(
               runtime.lastConsumedEvent801C9094,
               input.consumedEvent801C9094);
}

bool HasNewStage1RuntimeCompactEvent801C9094(
    const PrStage1RuntimeSlotsDirectRuntime& runtime,
    const PrStage1RuntimeSlotsDirectLateBranchInput& input) {
    if (!input.compactEvent801C9094.valid) {
        return false;
    }
    return !Stage1ConsumedEvent801C9094Equals(
        runtime.lastCompactEvent801C9094,
        input.compactEvent801C9094);
}

bool ShouldApplyStage1FlagCompactPrepass801C9094(
    const PrStage1RuntimeSlotsDirectLateBranchInput& input) {
    if (!input.compactEvent801C9094.valid) {
        return false;
    }
    if (input.consumedEventSource ==
            PrStageEventDirectStage1ConsumedEventSource801C9094::FlagStream) {
        return !Stage1ConsumedEvent801C9094Equals(
            input.compactEvent801C9094,
            input.consumedEvent801C9094);
    }
    return !Stage1ConsumedEvent801C9094Equals(
        input.compactEvent801C9094,
        input.consumedEvent801C9094);
}

bool AppendStage1RuntimeDueEventFrom801C9094(
    PrStage1RuntimeSlotsDirectRuntime& runtime,
    const PrStage1OverlayData& data,
    const PrStageEventDirectStage1ConsumedScriptEvent801C9094& consumed,
    uint32_t applyScriptFrame,
    uint32_t applyFrame,
    std::vector<Stage1RuntimeDueScriptEvent>& outDueEvents) {
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

    Stage1RuntimeDueScriptEvent due{};
    due.stream = stream;
    due.streamId = consumed.streamId;
    due.event = &ev;
    due.eventIndex = consumed.eventIndex;
    due.dueFrame = consumed.dueFrame;
    due.applyScriptFrame = applyScriptFrame;
    due.applyFrame = applyFrame;
    outDueEvents.push_back(due);

    runtime.activeEventAvailable = true;
    runtime.activeEventStreamId = consumed.streamId;
    runtime.activeEventIndex = consumed.eventIndex;
    runtime.activeEventPsxAddr = consumed.psxAddr;
    runtime.activeEventDueFrame = consumed.dueFrame;
    return true;
}

void AppendStage1RuntimeDueEventsFromFrameResult801C9094(
    PrStage1RuntimeSlotsDirectRuntime& runtime,
    const PrStage1OverlayData& data,
    const PrStage1RuntimeSlotsDirectLateBranchInput& input,
    uint32_t applyScriptFrame,
    uint32_t applyFrame,
    std::vector<Stage1RuntimeDueScriptEvent>& outDueEvents) {
    switch (input.consumedEventSource) {
        case PrStageEventDirectStage1ConsumedEventSource801C9094::IdStream:
            if (input.flagStreamEvent801C9094.valid) {
                SetStage1RuntimePrepassEventFrom801C9094(
                    runtime,
                    input.flagStreamEvent801C9094);
            }
            AppendStage1RuntimeDueEventFrom801C9094(
                runtime,
                data,
                input.consumedEvent801C9094,
                applyScriptFrame,
                applyFrame,
                outDueEvents);
            break;
        case PrStageEventDirectStage1ConsumedEventSource801C9094::FlagStream:
            AppendStage1RuntimeDueEventFrom801C9094(
                runtime,
                data,
                input.consumedEvent801C9094,
                applyScriptFrame,
                applyFrame,
                outDueEvents);
            break;
        case PrStageEventDirectStage1ConsumedEventSource801C9094::None:
        default:
            break;
    }
}

void ClearStage1RuntimeActiveEvent801C9094(
    PrStage1RuntimeSlotsDirectRuntime& runtime) {
    runtime.activeEventAvailable = false;
    runtime.activeEventStreamId = 0u;
    runtime.activeEventIndex = 0u;
    runtime.activeEventPsxAddr = 0u;
    runtime.activeEventDueFrame = 0u;
}

void RefreshStage1RuntimePairSlotHandles(const PrStage1OverlayData* data,
                                         PrStage1RuntimeSlotsDirectPairSlot& slot) {
    if (slot.directResourceHandleOverride) {
        return;
    }

    slot.datHandle = 0;
    slot.vdfHandle = 0;
    if (!data || !slot.valid || slot.pairIndex == 0u) {
        return;
    }

    ResolveStage1RuntimePairHandles(*data,
                                    slot.pairIndex,
                                    slot.datHandle,
                                    slot.vdfHandle);
}

void AssignStage1RuntimePairSlotFromEvent(
    PrStage1RuntimeSlotsDirectPairSlot& slot,
    const PrStage1OverlayData& data,
    uint8_t pairIndex,
    uint32_t dueFrame,
    uint32_t sourceEventPsxAddr) {
    slot.valid = true;
    slot.pairIndex = pairIndex;
    slot.directResourceHandleOverride = false;
    slot.startFrame = dueFrame;
    slot.sourceEventPsxAddr = sourceEventPsxAddr;
    RefreshStage1RuntimePairSlotHandles(&data, slot);
}

void AssignStage1RuntimePairSlotFromScriptEventSlot(
    PrStage1RuntimeSlotsDirectPairSlot& slot,
    const PrStage1ScriptEventRuntimeDirect::PairResourceSlot& source,
    uint32_t dueFrame,
    uint32_t sourceEventPsxAddr) {
    if (!source.valid || source.datHandle == 0u || source.vdfHandle == 0u) {
        return;
    }

    slot.valid = true;
    slot.pairIndex = source.pairIndex;
    slot.datHandle = source.datHandle;
    slot.vdfHandle = source.vdfHandle;
    slot.directResourceHandleOverride = true;
    slot.startFrame = dueFrame;
    slot.sourceEventPsxAddr = sourceEventPsxAddr;
}

void AssignStage1RuntimeDirectResourcePairSlot(
    PrStage1RuntimeSlotsDirectPairSlot& slot,
    uint16_t datHandle,
    uint16_t vdfHandle,
    uint32_t startFrame,
    uint32_t sourcePsxAddr) {
    slot.valid = true;
    slot.pairIndex = 0u;
    slot.datHandle = datHandle;
    slot.vdfHandle = vdfHandle;
    slot.startFrame = startFrame;
    slot.sourceEventPsxAddr = sourcePsxAddr;
    slot.directResourceHandleOverride = true;
}

bool ResolveStage1RuntimePerformanceTodHandle(const PrStage1OverlayData& data,
                                              uint8_t streamId,
                                              uint8_t currentMode,
                                              uint8_t& outHandle) {
    outHandle = 0u;
    if (streamId == 0u || streamId >= data.streamDescRows.size()) {
        return false;
    }

    const auto& row = data.streamDescRows[streamId].bytes;
    const size_t handleIndex = (currentMode == 0u) ? 1u : 2u;
    outHandle = row[handleIndex];
    return outHandle != 0u;
}

bool CopyStage1RuntimeSceneRow(const PrStage1OverlayData& data,
                               uint8_t rowIndex,
                               PrStage1RuntimeSlotsDirectSceneSlot& out) {
    out = PrStage1RuntimeSlotsDirectSceneSlot{};
    if (rowIndex == 0u || rowIndex >= data.streamDescRows.size()) {
        return false;
    }

    out.valid = true;
    out.selectedRow = rowIndex;
    const auto& row = data.streamDescRows[rowIndex].bytes;
    for (size_t i = 0; i < out.rowHandles.size(); ++i) {
        out.rowHandles[i] = row[3u + i];
    }
    return true;
}

bool Stage1SceneMapHandlesEqual(
    const std::array<uint8_t, PrStageSceneSubmitConstants::kDynamicRowSlotCount>& lhs,
    const std::array<uint8_t, PrStageSceneSubmitConstants::kDynamicRowSlotCount>& rhs) {
    for (size_t i = 0; i < lhs.size(); ++i) {
        if (lhs[i] != rhs[i]) {
            return false;
        }
    }
    return true;
}

uint8_t ResolveStage1ApplyScriptEventSceneRow(
    const PrStage1ScriptEvent& ev,
    const PrStage1RuntimeSlotsDirectLateBranchInput& lateBranch) {
    uint32_t ctxFlags = lateBranch.ctxFlags40_2000_4000;
    if ((ev.flags04 & kStage1SceneBaseRowFlag) != 0u) {
        ctxFlags |= kStage1SceneBaseRowFlag;
    }

    // 801C895C picks the scene row from ctx flags after applying
    // event.flags04&0x20. This order intentionally lets row1 dominate
    // GOOD->COOL branch rows for the authored 0xF1020 events.
    if ((ctxFlags & kStage1SceneBaseRowFlag) != 0u) {
        return 1u;
    }
    if ((ctxFlags & 0x2000u) != 0u) {
        return lateBranch.ctx72 < 2u ? 6u : 8u;
    }
    if ((ctxFlags & 0x0040u) != 0u) {
        return lateBranch.ed24 == 1 ? 5u : 4u;
    }
    if ((ctxFlags & 0x4000u) != 0u) {
        return 7u;
    }
    return 0u;
}

void ApplyStage1RuntimeSceneRowFromScriptEvent(
    PrStage1RuntimeSlotsDirectRuntime& runtime,
    const PrStage1OverlayData& data,
    const PrStage1EventStream* stream,
    uint8_t streamId,
    const PrStage1ScriptEvent& ev,
    uint32_t applyFrame,
    const PrStage1RuntimeSlotsDirectLateBranchInput& lateBranch) {
    const uint8_t rowIndex =
        ResolveStage1ApplyScriptEventSceneRow(ev, lateBranch);
    if (rowIndex == 0u) {
        return;
    }

    PrStage1RuntimeSlotsDirectSceneSlot scene{};
    if (!CopyStage1RuntimeSceneRow(data, rowIndex, scene)) {
        return;
    }

    scene.selectedStream = streamId;
    scene.streamStartFrame = applyFrame;
    scene.streamWindowFrames = ResolveStage1RuntimeStreamWindowFrames(stream);
    scene.sharedTodListEnabled =
        (lateBranch.ctxFlags40_2000_4000 & 0x2000u) != 0u ||
        lateBranch.flag2000SceneFamilyActive;
    const uint32_t flag2000SceneFamilyStartFrame =
        lateBranch.flag2000SceneFamilyStartScriptFrame != 0u
            ? lateBranch.flag2000SceneFamilyStartScriptFrame
            : lateBranch.flag2000SceneFamilyStartFrame;
    const uint8_t nextFlag2000SceneFamilyStream =
        lateBranch.flag2000SceneFamilyStream != 0u
            ? lateBranch.flag2000SceneFamilyStream
            : streamId;
    const bool restartFlag2000SceneFamily =
        scene.sharedTodListEnabled &&
        flag2000SceneFamilyStartFrame != 0u &&
        (flag2000SceneFamilyStartFrame > runtime.scene.streamStartFrame ||
         nextFlag2000SceneFamilyStream != runtime.scene.selectedStream);
    if (restartFlag2000SceneFamily) {
        // After dropping out of COOL, the previous scene may be a non-0x2000
        // GOOD row with the same dojo map tail; do not require its shared flag.
        const bool sameSceneMapSubmitFamily =
            runtime.scene.valid &&
            Stage1SceneMapHandlesEqual(scene.rowHandles,
                                       runtime.scene.rowHandles);
        const bool flag2000RowTodOnlyReload =
            (nextFlag2000SceneFamilyStream == 6u ||
             nextFlag2000SceneFamilyStream == 8u) &&
            sameSceneMapSubmitFamily;
        runtime.sceneSubmit.flag2000DynamicRowReload801CBFDC = true;
        if (!flag2000RowTodOnlyReload) {
            runtime.sceneSubmit.sceneInitPulse801CB6BC = true;
        }
        scene.streamStartFrame = flag2000SceneFamilyStartFrame;
        scene.selectedStream = nextFlag2000SceneFamilyStream;
    } else if (scene.sharedTodListEnabled &&
        runtime.scene.valid &&
        Stage1SceneMapHandlesEqual(scene.rowHandles,
                                   runtime.scene.rowHandles)) {
        scene.streamStartFrame = runtime.scene.streamStartFrame;
        scene.streamWindowFrames = runtime.scene.streamWindowFrames;
        scene.selectedStream = runtime.scene.selectedStream;
    } else if (scene.sharedTodListEnabled &&
               flag2000SceneFamilyStartFrame != 0u) {
        scene.streamStartFrame = flag2000SceneFamilyStartFrame;
        scene.selectedStream = lateBranch.flag2000SceneFamilyStream != 0u
                                   ? lateBranch.flag2000SceneFamilyStream
                                   : streamId;
    }
    runtime.scene = scene;
}

void ActivateStage1RuntimeHudTimedList(
    PrStage1RuntimeSlotsDirectRuntime& runtime,
    const PrStage1ScriptEventRuntimeDirect::HudTimedListSlot& source,
    uint32_t baseFrame,
    uint32_t sourceEventPsxAddr) {
    if (!source.valid || source.channel >= runtime.hudTimedLists.size() ||
        source.slotId == 0u) {
        return;
    }

    PrStage1RuntimeSlotsDirectHudTimedListChannel& channel =
        runtime.hudTimedLists[source.channel];
    channel.active = true;
    channel.slotId = source.slotId;
    channel.baseFrame = baseFrame;
    channel.sourceEventPsxAddr = sourceEventPsxAddr;
    if (source.slotId < runtime.hudTimedListDescCursors.size()) {
        runtime.hudTimedListDescCursors[source.slotId] = 0u;
    }
}

void ClearStage1RuntimeHudTimUploads(
    PrStage1RuntimeSlotsDirectRuntime& runtime) {
    for (auto& upload : runtime.hudTimUploads) {
        upload = PrStage1RuntimeSlotsDirectHudTimUpload{};
    }
}

void ResetStage1RuntimeCompactProducer801C9094(
    PrStage1RuntimeSlotsDirectRuntime& runtime) {
    auto& producer = runtime.sceneSubmit.compactProducer801C9094;
    producer.active = false;
    producer.teacherMode8A = 0;
    producer.teacherCursor8C = -1;
    producer.teacherCursor8E = -1;
    producer.teacherState90 = 0;
    producer.teacherBodyPtr94 =
        PrStage1OverlayData::kCompactRailRec44BaseAddr + 4u;
    producer.teacherBodyPtr98 =
        PrStage1OverlayData::kCompactRailRec44BaseAddr + 24u;
    producer.teacherBodyStream94 = {};
    producer.teacherBodyStream98 = {};
    producer.studentCursor9E = -1;
    producer.studentCursorA0 = -1;
    producer.studentStateA2 = 0;
    producer.studentBodyPtrA4 =
        PrStage1OverlayData::kCompactRailRec44BaseAddr + 4u;
    producer.studentBodyPtrA8 =
        PrStage1OverlayData::kCompactRailRec44BaseAddr + 24u;
    producer.studentBodyStreamA4 = {};
    producer.studentBodyStreamA8 = {};
    runtime.sceneSubmit.compactRail80024744 =
        PrStage1RuntimeSlotsDirectCompactRailRuntime80024744{};
    runtime.sceneSubmit.compactWobbleResetPulse80024744 = true;
}

void ClearStage1RuntimeCompactRailRowCount8A80024FD0(
    PrStage1RuntimeSlotsDirectRuntime& runtime) {
    // PSX 80024FD0 GOOD->COOL writes only ctx+0x8A here; keep the wider
    // 801C9094 compact cursor/body state intact.
    runtime.sceneSubmit.compactProducer801C9094.teacherMode8A = 0;
    runtime.sceneSubmit.compactRail80024744.rowCount8A = 0;
    runtime.sceneSubmit.compactRail80024744.valid = false;
    runtime.sceneSubmit.renderCompactRail80024744.rowCount8A = 0;
    runtime.sceneSubmit.renderCompactRail80024744.valid = false;
}

void CopyStage1RuntimeCompactBodyStream80024744(
    const std::array<int8_t, 20>& source,
    std::array<int8_t, 18>& out) {
    out = {};
    for (size_t i = 0; i < out.size() && i < source.size(); ++i) {
        out[i] = source[i];
    }
}

void CopyStage1RuntimeCompactBodyStream801C9094(
    const std::array<int8_t, 20>& source,
    std::array<int8_t, 20>& out) {
    out = source;
}

void PublishStage1RuntimeCompactProducer801C9094(
    PrStage1RuntimeSlotsDirectRuntime& runtime);

void CaptureStage1RuntimeCompactPainterInput801CBFDC(
    PrStage1RuntimeSlotsDirectRuntime& runtime) {
    runtime.sceneSubmit.renderCompactRail80024744 =
        runtime.sceneSubmit.compactRail80024744;
    runtime.sceneSubmit.renderCompactWobbleResetPulse80024744 =
        runtime.sceneSubmit.compactWobbleResetPulse80024744;
    runtime.sceneSubmit.renderCompactTeacherResetPulse80024308 =
        runtime.sceneSubmit.compactTeacherResetPulse80024308;
    runtime.sceneSubmit.renderCompactStudentResetPulse80024390 =
        runtime.sceneSubmit.compactStudentResetPulse80024390;
}

void WriteStage1RuntimeCompactTeacher8E90FFFF801C9094(
    PrStage1RuntimeSlotsDirectCompactProducer801C9094& producer) {
    producer.teacherCursor8E = -1;
    producer.teacherState90 = 0;
}

void WriteStage1RuntimeCompactStudentA0A2FFFF801C9094(
    PrStage1RuntimeSlotsDirectCompactProducer801C9094& producer) {
    producer.studentCursorA0 = -1;
    producer.studentStateA2 = 0;
}

PrStage1RuntimeSlotsDirectCompactRailRuntime80024744
BuildStage1RuntimeCompactRailProjection80024744(
    const PrStage1RuntimeSlotsDirectCompactProducer801C9094& producer) {
    PrStage1RuntimeSlotsDirectCompactRailRuntime80024744 out{};
    out.valid = false;
    out.painterGate7A = 0;
    out.rowCount8A = producer.teacherMode8A;
    if (out.rowCount8A <= 0) {
        return out;
    }

    out.rows[0].teacherRaw8C = producer.teacherCursor8C;
    out.rows[0].studentRaw9E = producer.studentCursor9E;
    out.rows[0].bodyStreamPtr94 = producer.teacherBodyPtr94;
    CopyStage1RuntimeCompactBodyStream80024744(
        producer.teacherBodyStream94,
        out.rows[0].bodyStreamBytes94);

    out.rows[1].teacherRaw8C = producer.teacherCursor8E;
    out.rows[1].studentRaw9E = producer.studentCursorA0;
    out.rows[1].bodyStreamPtr94 = producer.teacherBodyPtr98;
    CopyStage1RuntimeCompactBodyStream80024744(
        producer.teacherBodyStream98,
        out.rows[1].bodyStreamBytes94);
    return out;
}

void PublishStage1RuntimeCompactProducer801C9094(
    PrStage1RuntimeSlotsDirectRuntime& runtime) {
    runtime.sceneSubmit.compactRail80024744 =
        BuildStage1RuntimeCompactRailProjection80024744(
            runtime.sceneSubmit.compactProducer801C9094);
}

void Psx801C9094RunCompactProducer(
    PrStage1RuntimeSlotsDirectRuntime& runtime,
    const PrStage1OverlayData& data,
    const PrStage1RailCursorEventDirectResult* railEvent,
    uint32_t sourceScriptFrame,
    bool ctxFlagTickAdvanceKnown,
    bool ctxFlagTickAdvance,
    bool runPreEventSteadyTick) {
    auto& producer = runtime.sceneSubmit.compactProducer801C9094;
    runtime.sceneSubmit.ctxFlagTickAdvanceKnown801C9094 =
        ctxFlagTickAdvanceKnown;
    runtime.sceneSubmit.ctxFlagTickAdvance801C9094 =
        ctxFlagTickAdvance;
    const bool advanceCompactTick =
        ctxFlagTickAdvanceKnown && ctxFlagTickAdvance;

    if (railEvent != nullptr && railEvent->compactResetRequested801C9094) {
        ResetStage1RuntimeCompactProducer801C9094(runtime);
    }

    if (railEvent != nullptr &&
        railEvent->compactTeacherBootstrapRequested801C9094) {
        const PrStage1CompactRailRec44* primary =
            data.FindCompactRailRec44(railEvent->activeEventByte1D);
        if (primary != nullptr) {
            const int16_t v38CompactMode = primary->head0;
            producer.active = true;
            producer.sourceEventPsxAddr = railEvent->activeEventPsxAddr;
            producer.sourceFrame = sourceScriptFrame;
            producer.teacherRecordIndex = railEvent->activeEventByte1D;
            producer.studentRecordIndex = railEvent->activeEventByte1E;
            producer.cachedTeacherHead0Negative801C9094 =
                v38CompactMode < 0;

            if (v38CompactMode <= 0) {
                producer.teacherMode8A =
                    static_cast<int16_t>(-v38CompactMode);
                producer.teacherBodyPtr94 = primary->psxAddr + 4u;
                producer.teacherBodyStream94 = primary->streamA;
                producer.cachedTeacherBodyPtr801D304C =
                    producer.teacherBodyPtr94;
                producer.teacherCursor8C = -1;
                WriteStage1RuntimeCompactTeacher8E90FFFF801C9094(producer);
            } else {
                producer.teacherMode8A = v38CompactMode;
                producer.teacherCursor8E = -1;
                producer.teacherState90 = 1;
                producer.teacherCursor8C =
                    static_cast<int16_t>(primary->head1);
                producer.teacherBodyPtr94 = primary->psxAddr + 4u;
                producer.teacherBodyStream94 = primary->streamA;
                producer.cachedTeacherBodyPtr801D304C =
                    producer.teacherBodyPtr94;
            }

            const PrStage1CompactRailRec44* secondary =
                data.FindCompactRailRec44(railEvent->activeEventByte1E);
            if (secondary != nullptr) {
                producer.teacherBodyPtr98 = secondary->psxAddr + 24u;
                producer.teacherBodyStream98 = secondary->streamB;
                producer.cachedTeacherBodyPtr801D3050 = producer.teacherBodyPtr98;
            } else {
                producer.teacherBodyPtr98 = 0u;
                producer.teacherBodyStream98 = {};
                producer.cachedTeacherBodyPtr801D3050 = 0u;
            }

            if (producer.teacherBodyStream94[0] == -2) {
                producer.studentCursor9E = -1;
                WriteStage1RuntimeCompactStudentA0A2FFFF801C9094(producer);
            }
            if (producer.teacherMode8A == 0) {
                producer.teacherCursor8C = -1;
                WriteStage1RuntimeCompactTeacher8E90FFFF801C9094(producer);
                producer.studentCursor9E = -1;
                WriteStage1RuntimeCompactStudentA0A2FFFF801C9094(producer);
            }
        }
    }

    const int16_t v8TeacherState90 = producer.teacherState90;
    const bool tickTeacherLane =
        runPreEventSteadyTick ||
        (railEvent != nullptr &&
         railEvent->compactTeacherBootstrapRequested801C9094);
    if (tickTeacherLane &&
        advanceCompactTick &&
        v8TeacherState90 != 0) {
        const int16_t v9TeacherMode8A = producer.teacherMode8A;
        if (v9TeacherMode8A == 1) {
            if (v8TeacherState90 == 1) {
                const int16_t v10TeacherCursor8C = producer.teacherCursor8C;
                if (v10TeacherCursor8C >= 0 &&
                    v10TeacherCursor8C < 19 &&
                    producer.teacherBodyStream94[
                            static_cast<size_t>(v10TeacherCursor8C)] != -1) {
                    ++producer.teacherCursor8C;
                    goto label_143;
                }
                producer.teacherState90 = 0;
                producer.teacherCursor8C = -1;
                runtime.sceneSubmit.compactTeacherResetPulse80024308 = true;
            }
        } else if (v9TeacherMode8A == 2) {
            if (v8TeacherState90 == 1) {
                const int16_t v11TeacherCursor8C = producer.teacherCursor8C;
                if (v11TeacherCursor8C < 0 ||
                    v11TeacherCursor8C >= 19 ||
                    producer.teacherBodyStream94[
                            static_cast<size_t>(v11TeacherCursor8C)] == -1) {
                    producer.teacherCursor8C = -1;
                    producer.teacherCursor8E = 2;
                    producer.teacherState90 = 2;
                } else {
                    ++producer.teacherCursor8C;
                }
            }
            if (producer.teacherState90 == 2) {
                const int16_t v12TeacherCursor8E = producer.teacherCursor8E;
                if (v12TeacherCursor8E >= 0 &&
                    v12TeacherCursor8E < 19 &&
                    producer.teacherBodyStream98[
                            static_cast<size_t>(v12TeacherCursor8E)] != -1) {
                    ++producer.teacherCursor8E;
                    goto label_143;
                }
                producer.teacherCursor8C = -1;
                producer.teacherCursor8E = -1;
                runtime.sceneSubmit.compactTeacherResetPulse80024308 = true;
            }
        }
    }

label_143:
    if (railEvent != nullptr &&
        railEvent->compactStudentCatchupRequested801C9094) {
        producer.sourceEventPsxAddr = railEvent->activeEventPsxAddr;
        producer.sourceFrame = sourceScriptFrame;
        producer.studentCursor9E = kStage1CompactStudentCursor9EInit801C9094;
        producer.studentCursorA0 = kStage1CompactSecondCursorInit801C9094;
        producer.studentStateA2 = 1;
        producer.studentBodyPtrA4 = producer.cachedTeacherBodyPtr801D304C;
        producer.studentBodyPtrA8 = producer.cachedTeacherBodyPtr801D3050;
        CopyStage1RuntimeCompactBodyStream801C9094(
            producer.teacherBodyStream94,
            producer.studentBodyStreamA4);
        CopyStage1RuntimeCompactBodyStream801C9094(
            producer.teacherBodyStream98,
            producer.studentBodyStreamA8);
        const PrStage1CompactRailRec44* catchupRecord =
            data.FindCompactRailRec44(railEvent->activeEventByte1D);
        if (catchupRecord != nullptr && catchupRecord->head0 < 0) {
            runtime.sceneSubmit.compactStudentResetPulse80024390 = true;
        }
    }

    const int16_t v13StudentStateA2 = producer.studentStateA2;
    const bool tickStudentLane =
        runPreEventSteadyTick ||
        (railEvent != nullptr &&
         railEvent->compactStudentCatchupRequested801C9094);
    if (tickStudentLane &&
        advanceCompactTick &&
        v13StudentStateA2 != 0) {
        const int16_t v15TeacherMode8A = producer.teacherMode8A;
        if (v15TeacherMode8A == 1) {
            if (v13StudentStateA2 == 1) {
                const int16_t v16StudentCursor9E =
                    producer.studentCursor9E;
                if (v16StudentCursor9E >= 0 &&
                    v16StudentCursor9E < 19 &&
                    producer.studentBodyStreamA4[
                            static_cast<size_t>(v16StudentCursor9E)] != -1) {
                    ++producer.studentCursor9E;
                    goto label_166;
                }
                producer.studentStateA2 = 0;
                producer.studentCursor9E = -1;
                runtime.sceneSubmit.compactStudentResetPulse80024390 = true;
            }
        } else if (v15TeacherMode8A == 2) {
            if (v13StudentStateA2 == 1) {
                const int16_t v17StudentCursor9E =
                    producer.studentCursor9E;
                if (v17StudentCursor9E < 0 ||
                    v17StudentCursor9E >= 19 ||
                    producer.studentBodyStreamA4[
                            static_cast<size_t>(v17StudentCursor9E)] == -1) {
                    producer.studentCursor9E = -1;
                    producer.studentCursorA0 = 2;
                    producer.studentStateA2 = 2;
                } else {
                    ++producer.studentCursor9E;
                }
            }
            if (producer.studentStateA2 == 2) {
                const int16_t v18StudentCursorA0 =
                    producer.studentCursorA0;
                if (v18StudentCursorA0 >= 0 &&
                    v18StudentCursorA0 < 19 &&
                    producer.studentBodyStreamA8[
                            static_cast<size_t>(v18StudentCursorA0)] != -1) {
                    ++producer.studentCursorA0;
                    goto label_166;
                }
                WriteStage1RuntimeCompactStudentA0A2FFFF801C9094(producer);
                runtime.sceneSubmit.compactStudentResetPulse80024390 = true;
            }
        }
    }

label_166:
    PublishStage1RuntimeCompactProducer801C9094(runtime);
}

bool Psx801C9094ApplyCompactProducerFromEvent(
    PrStage1RuntimeSlotsDirectRuntime& runtime,
    const PrStage1OverlayData& data,
    uint8_t streamId,
    const PrStage1ScriptEvent& ev,
    uint32_t scriptDueFrame,
    uint32_t sourceScriptFrame,
    const PrStage1RailCursorEventDirectResult* railEvent,
    bool ctxFlagTickAdvanceKnown,
    bool ctxFlagTickAdvance) {
    if (railEvent == nullptr || !railEvent->activeEventAvailable ||
        !railEvent->compactEventBearing801C9094 ||
        railEvent->activeEventPsxAddr != ev.psxAddr ||
        (railEvent->activeStreamId != 0u &&
         railEvent->activeStreamId != streamId) ||
        (railEvent->activeDueFrame != 0u &&
         railEvent->activeDueFrame != scriptDueFrame)) {
        return false;
    }

    Psx801C9094RunCompactProducer(runtime,
                                  data,
                                  railEvent,
                                  sourceScriptFrame,
                                  ctxFlagTickAdvanceKnown,
                                  ctxFlagTickAdvance,
                                  false);
    return true;
}

void TickStage1RuntimeHudTimedListChannel(
    PrStage1RuntimeSlotsDirectRuntime& runtime,
    const PrStage1OverlayData& data,
    uint8_t channelIndex,
    uint32_t scriptFrame) {
    if (channelIndex >= runtime.hudTimedLists.size() ||
        channelIndex >= runtime.hudTimUploads.size()) {
        return;
    }

    PrStage1RuntimeSlotsDirectHudTimedListChannel& channel =
        runtime.hudTimedLists[channelIndex];
    if (!channel.active || channel.slotId == 0u) {
        return;
    }

    if (channel.slotId >= runtime.hudTimedListDescCursors.size()) {
        channel = PrStage1RuntimeSlotsDirectHudTimedListChannel{};
        return;
    }

    uint32_t& descCursor = runtime.hudTimedListDescCursors[channel.slotId];
    const PrStage1HudSlotDesc* desc = data.FindHudSlot(channel.slotId);
    if (!desc || desc->events.empty()) {
        descCursor = 0u;
        channel = PrStage1RuntimeSlotsDirectHudTimedListChannel{};
        return;
    }

    const size_t eventCount = desc->eventCount != 0u
        ? (std::min<size_t>)(desc->events.size(), (size_t)desc->eventCount)
        : desc->events.size();
    if (descCursor >= eventCount) {
        descCursor = 0u;
        channel = PrStage1RuntimeSlotsDirectHudTimedListChannel{};
        return;
    }

    const uint32_t eventIndex = descCursor;
    const PrStage1HudAnimEvent& event = desc->events[(size_t)eventIndex];
    const uint64_t dueFrame64 =
        (uint64_t)channel.baseFrame + (uint64_t)event.deltaFrames;
    if ((uint64_t)scriptFrame < dueFrame64) {
        return;
    }

    PrStage1RuntimeSlotsDirectHudTimUpload upload{};
    upload.valid = true;
    upload.channel = channelIndex;
    upload.slotId = channel.slotId;
    upload.eventIndex = eventIndex;
    upload.dueFrame = (uint32_t)(std::min<uint64_t>)(
        dueFrame64,
        (uint64_t)UINT32_MAX);
    upload.sourceEventPsxAddr = channel.sourceEventPsxAddr;
    upload.timIds = event.timIds;
    runtime.hudTimUploads[channelIndex] = upload;
    descCursor++;
}

void TickStage1RuntimeHudTimedLists(
    PrStage1RuntimeSlotsDirectRuntime& runtime,
    const PrStage1OverlayData& data,
    uint32_t scriptFrame) {
    for (uint8_t channel = 0u; channel < runtime.hudTimedLists.size(); ++channel) {
        TickStage1RuntimeHudTimedListChannel(runtime, data, channel, scriptFrame);
    }
}

void ApplyStage1RatingPresentationResourcePairSlot(
    PrStage1RuntimeSlotsDirectRuntime& runtime,
    const PrStage1RuntimeSlotsDirectRatingPresentationInput& rating,
    uint32_t queryFrame) {
    if (!rating.flag0200FrameUpdateBranchTaken ||
        !rating.flag0200FrameUpdateResourcePairKnown ||
        !rating.flag0200FrameUpdateCtxFlag20000Set ||
        rating.flag0200FrameUpdateCtxE0ResourceIndex == 0u ||
        rating.flag0200FrameUpdateCtxECResourceIndex == 0u) {
        return;
    }

    AssignStage1RuntimeDirectResourcePairSlot(
        runtime.onMotion,
        rating.flag0200FrameUpdateCtxE0ResourceIndex,
        rating.flag0200FrameUpdateCtxECResourceIndex,
        queryFrame,
        kStage1RatingPresentationFrameUpdatePsxAddr);
    runtime.sceneSubmit.ctxFlagsFull |= 0x00020000u;
    runtime.sceneSubmit.ctxFlagsPulse801CBFDC |= 0x00020000u;
    runtime.word8008ECFEResource20000TimerFrames =
        kStage1Word8008ECFESetResLoopFrames;
    runtime.dword801D3054ScriptEventOnByModeGateFrames =
        kStage1Dword801D3054RatingGateFrames;
}

void ApplyStage1RuntimeHudOverlayConfigure801CA0E0(
    PrStage1RuntimeSlotsDirectRuntime& runtime,
    const PrStage1OverlayData& data,
    const PrStage1RuntimeSlotsDirectDescBankSelectorInput& input,
    uint32_t queryFrame,
    uint32_t scriptFrame) {
    if (!input.hudOverlayConfigure801CA0E0Valid) {
        return;
    }

    const uint8_t arg = input.hudOverlayConfigure801CA0E0Arg;
    const PrStage1HudOverlayConfigure801CA0E0Row* row =
        data.FindHudOverlayConfigure801CA0E0Row(arg);
    if (row == nullptr) {
        return;
    }

    const uint32_t sourceFrame =
        input.hudOverlayConfigure801CA0E0SourceFrame != 0u
            ? input.hudOverlayConfigure801CA0E0SourceFrame
            : queryFrame;
    runtime.sceneSubmit.ctxFlagsFull |= 0x00000010u;
    runtime.sceneSubmit.ctxFlagsPulse801CBFDC |= 0x00000010u;
    runtime.sceneSubmit.ctx11C = static_cast<uint16_t>(row->ctx11CValue);
    runtime.sceneSubmit.ctx120 = static_cast<uint16_t>(row->timerFrames);
    runtime.word8008ECFCResource10000TimerFrames =
        static_cast<int32_t>(row->timerFrames);
    if (row->paDatResourceIndex > 0 && row->paVdfResourceIndex > 0) {
        AssignStage1RuntimeDirectResourcePairSlot(
            runtime.paMotion,
            static_cast<uint16_t>(row->paDatResourceIndex),
            static_cast<uint16_t>(row->paVdfResourceIndex),
            sourceFrame,
            kStage1HudOverlayConfigure801CA0E0PsxAddr);
    }

    if (row->hudSlotId > 0) {
        PrStage1ScriptEventRuntimeDirect::HudTimedListSlot slot{};
        slot.valid = true;
        slot.channel = 0u;
        slot.slotId = static_cast<uint8_t>(row->hudSlotId);
        const uint32_t sourceScriptFrame =
            input.hudOverlayConfigure801CA0E0SourceScriptFrame != 0u
                ? input.hudOverlayConfigure801CA0E0SourceScriptFrame
                : scriptFrame;
        ActivateStage1RuntimeHudTimedList(
            runtime,
            slot,
            sourceScriptFrame,
            kStage1HudOverlayConfigure801CA0E0PsxAddr);
    }
}

void TickStage1RuntimeWord8008ECFCResource10000(
    PrStage1RuntimeSlotsDirectRuntime& runtime,
    uint32_t queryFrame,
    uint32_t scriptFrame,
    bool mainEventStreamArmed) {
    if (runtime.word8008ECFCResource10000TimerFrames <= 0) {
        return;
    }

    --runtime.word8008ECFCResource10000TimerFrames;
    if (runtime.word8008ECFCResource10000TimerFrames != 0 ||
        !mainEventStreamArmed) {
        return;
    }

    AssignStage1RuntimeDirectResourcePairSlot(runtime.paMotion,
                                             kStage1PaWt0DatHandle,
                                             kStage1PaWt0VdfHandle,
                                             queryFrame,
                                             kStage1HudSetRes10000PsxAddr);
    runtime.sceneSubmit.ctxFlagsPulse801CBFDC |= kStage1PairFlagPaByCoolness;

    PrStage1ScriptEventRuntimeDirect::HudTimedListSlot slot{};
    slot.valid = true;
    slot.channel = 0u;
    slot.slotId = kStage1HudSetRes10000TimedListSlot;
    ActivateStage1RuntimeHudTimedList(
        runtime,
        slot,
        scriptFrame,
        kStage1HudSetRes10000PsxAddr);
}

void TickStage1RuntimeWord8008ECFEResource20000(
    PrStage1RuntimeSlotsDirectRuntime& runtime,
    uint32_t queryFrame,
    bool unk8008ED00Is5) {
    if (!unk8008ED00Is5 ||
        runtime.word8008ECFEResource20000TimerFrames <= 0) {
        return;
    }

    --runtime.word8008ECFEResource20000TimerFrames;
    if (runtime.word8008ECFEResource20000TimerFrames != 0) {
        return;
    }

    AssignStage1RuntimeDirectResourcePairSlot(runtime.onMotion,
                                             kStage1OnWt1DatHandle,
                                             kStage1OnWt1VdfHandle,
                                             queryFrame,
                                             kStage1HudSetRes20000PsxAddr);
    runtime.sceneSubmit.ctxFlagsPulse801CBFDC |= kStage1PairFlagOnByMode;
    runtime.word8008ECFEResource20000TimerFrames =
        kStage1Word8008ECFESetResLoopFrames;
}

void TickStage1RuntimeDword801D3054ScriptEventOnByModeGate(
    PrStage1RuntimeSlotsDirectRuntime& runtime) {
    if (runtime.dword801D3054ScriptEventOnByModeGateFrames > 0) {
        --runtime.dword801D3054ScriptEventOnByModeGateFrames;
    }
}

void ApplyStage1RuntimeSlotDefaults(PrStage1RuntimeSlotsDirectRuntime& runtime,
                                    const PrStage1OverlayData* data,
                                    uint32_t queryFrame) {
    runtime.active = true;
    if (data && !runtime.scene.valid) {
        if (CopyStage1RuntimeSceneRow(*data, 1u, runtime.scene)) {
            runtime.scene.streamStartFrame = queryFrame;
        }
    }
    if (!runtime.tamaSelector.valid) {
        runtime.tamaSelector.valid = true;
        runtime.tamaSelector.selector = 0u;
        runtime.tamaSelector.startFrame = queryFrame;
        runtime.tamaSelector.sourceEventPsxAddr = 0u;
    }
    if (!runtime.onMotion.valid) {
        runtime.onMotion.valid = true;
        runtime.onMotion.pairIndex = kStage1DefaultOnPairIndex;
        runtime.onMotion.startFrame = queryFrame;
        runtime.onMotion.sourceEventPsxAddr = 0u;
    }
    RefreshStage1RuntimePairSlotHandles(data, runtime.onMotion);
    if (!runtime.paMotion.valid) {
        runtime.paMotion.valid = true;
        runtime.paMotion.pairIndex = kStage1DefaultPaPairIndex;
        runtime.paMotion.startFrame = queryFrame;
        runtime.paMotion.sourceEventPsxAddr = 0u;
    }
    RefreshStage1RuntimePairSlotHandles(data, runtime.paMotion);
}

void ApplyStage1RuntimeSlotResetSub80024F8C(
    PrStage1RuntimeSlotsDirectRuntime& runtime) {
    runtime.word5A_801C369A = runtime.ctxInit80024E98.word5A_801C369A;
    runtime.resetAction80024F8C = PrStageEventDirectBuildResetAction80024F8C(
        false,
        0u,
        0u,
        true,
        runtime.word5A_801C369A);
}

void ApplyStage1RuntimeSlotResetSub80024E98(
    PrStage1RuntimeSlotsDirectRuntime& runtime,
    const PrStage1RuntimeSlotsDirectResetInput80024E98& resetInput) {
    runtime.ctxInit80024E98 = PrStage1RuntimeSlotsDirectCtxInit80024E98{};
    runtime.ctxInit80024E98.dword800943D0 = resetInput.dword800943D0;
    runtime.ctxInit80024E98.dword94_801C36D4 =
        resetInput.dword800943D0 + 4u;
    runtime.ctxInit80024E98.dword98_801C36D8 =
        resetInput.dword800943D0 + 24u;
    runtime.ctxInit80024E98.ptr800943D0SeedGap = false;
    runtime.ctxInit80024E98.word800916DC = resetInput.word800916DC;
    runtime.ctxInit80024E98.word68_801C36A8 = resetInput.word800916DC;
    runtime.ctxInit80024E98.word54_801C3694 = resetInput.word800916DC;
    runtime.ctxInit80024E98.word800916DCSeedGap =
        !resetInput.word800916DCKnown;
    ApplyStage1RuntimeSlotResetSub80024F8C(runtime);
}

void ApplyStage1RuntimeSlotsSteadyEntry801C7A60(
    PrStage1RuntimeSlotsDirectRuntime& runtime,
    const PrStage1RuntimeSlotsDirectAdvanceInput& input) {
    if (!input.word800916DCKnown801C7A60) {
        return;
    }

    runtime.steadyCtx801C7A60.valid = true;
    runtime.steadyCtx801C7A60.word800916DCKnown = true;
    runtime.steadyCtx801C7A60.word800916DC =
        input.word800916DC801C7A60;
    runtime.steadyCtx801C7A60.word68Known = true;
    runtime.steadyCtx801C7A60.word68_801C36A8 =
        input.word800916DC801C7A60;
    runtime.steadyCtx801C7A60.word54Known = true;
    runtime.steadyCtx801C7A60.word54_801C3694 =
        input.word800916DC801C7A60;
    runtime.steadyCtx801C7A60.word54ClearedOnExit = false;
}

void ApplyStage1RuntimeSlotsFromEvent(PrStage1RuntimeSlotsDirectRuntime& runtime,
                                      const PrStage1OverlayData& data,
                                      const PrStage1EventStream* stream,
                                      uint8_t streamId,
                                      const PrStage1ScriptEvent& ev,
                                      uint8_t currentMode,
                                      uint32_t scriptDueFrame,
                                      uint32_t applyScriptFrame,
                                      uint32_t applyFrame,
                                      const PrStage1RailCursorEventDirectResult*
                                          inputRailEvent801C9094,
                                      const PrStage1RuntimeSlotsDirectLateBranchInput&
                                          lateBranch,
                                      bool ctxFlagTickAdvanceKnown801C9094,
                                      bool ctxFlagTickAdvance801C9094) {
    const bool compactProducerApplied =
        Psx801C9094ApplyCompactProducerFromEvent(runtime,
                                                 data,
                                                 streamId,
                                                 ev,
                                                 scriptDueFrame,
                                                 applyScriptFrame,
                                                 inputRailEvent801C9094,
                                                 ctxFlagTickAdvanceKnown801C9094,
                                                 ctxFlagTickAdvance801C9094);

    PrStage1ScriptEventRuntimeDirect::ScriptEventSlots slots{};
    if (!PrStage1ScriptEventRuntimeDirect::MaterializeScriptEventSlots(
            data,
            stream,
            streamId,
            ev,
            currentMode,
            scriptDueFrame,
            slots)) {
        return;
    }

    const bool idStreamApplyClearsPaTimer =
        IsStage1RuntimeBranchStream(streamId);
    const bool allowScriptEventPaByCoolness =
        idStreamApplyClearsPaTimer ||
        runtime.word8008ECFCResource10000TimerFrames <= 0;
    const bool allowScriptEventOnByMode =
        runtime.dword801D3054ScriptEventOnByModeGateFrames <= 0;
    uint32_t ctxFlagsSet = slots.ctxFlagsSet;
    if (!allowScriptEventPaByCoolness) {
        ctxFlagsSet &= ~kStage1PairFlagPaByCoolness;
    }
    if (!allowScriptEventOnByMode) {
        ctxFlagsSet &= ~kStage1PairFlagOnByMode;
    }

    runtime.scriptCtxMirror.valid = true;
    runtime.scriptCtxMirror.sourceEventPsxAddr = slots.sourceEventPsxAddr;
    runtime.scriptCtxMirror.dueFrame = slots.dueFrame;
    runtime.scriptCtxMirror.ctxFlagsSet = ctxFlagsSet;
    runtime.sceneSubmit.ctxFlagsFull |= ctxFlagsSet;
    runtime.sceneSubmit.ctxFlagsPulse801CBFDC |= ctxFlagsSet;
    runtime.scriptCtxMirror.paDatCtxOffset =
        allowScriptEventPaByCoolness ? slots.paByCoolness.datCtxOffset : 0u;
    runtime.scriptCtxMirror.paVdfCtxOffset =
        allowScriptEventPaByCoolness ? slots.paByCoolness.vdfCtxOffset : 0u;
    runtime.scriptCtxMirror.onDatCtxOffset =
        allowScriptEventOnByMode ? slots.onByMode.datCtxOffset : 0u;
    runtime.scriptCtxMirror.onVdfCtxOffset =
        allowScriptEventOnByMode ? slots.onByMode.vdfCtxOffset : 0u;
    runtime.scriptCtxMirror.performanceTodCtxOffset =
        slots.performanceTod.handleCtxOffset;
    runtime.scriptCtxMirror.row0TodCtxOffset = slots.row0Tod.handleCtxOffset;
    runtime.scriptCtxMirror.tamaSelectorCtxOffset =
        slots.tama.selectorCtxOffset;
    runtime.scriptCtxMirror.cameraBezCtxOffset = slots.camera.bezCtxOffset;

    if (!compactProducerApplied) {
        Psx801C9094ApplyCompactProducerFromEvent(runtime,
                                                 data,
                                                 streamId,
                                                 ev,
                                                 scriptDueFrame,
                                                 applyScriptFrame,
                                                 inputRailEvent801C9094,
                                                 ctxFlagTickAdvanceKnown801C9094,
                                                 ctxFlagTickAdvance801C9094);
    }

    runtime.tamaSelector.valid = true;
    runtime.tamaSelector.selector = slots.tama.selector;
    runtime.tamaSelector.startFrame = applyFrame;
    runtime.tamaSelector.sourceEventPsxAddr = slots.sourceEventPsxAddr;

    for (const auto& hudSlot : slots.hudTimedLists) {
        ActivateStage1RuntimeHudTimedList(runtime,
                                          hudSlot,
                                          applyScriptFrame,
                                          slots.sourceEventPsxAddr);
    }

    if (allowScriptEventOnByMode) {
        AssignStage1RuntimePairSlotFromScriptEventSlot(runtime.onMotion,
                                                       slots.onByMode,
                                                       applyFrame,
                                                       slots.sourceEventPsxAddr);
    }
    if (allowScriptEventOnByMode && slots.onByMode.valid) {
        runtime.word8008ECFEResource20000TimerFrames =
            slots.mode == 0u ? kStage1Word8008ECFECoolRecoveryFrames : 0;
    }
    if (idStreamApplyClearsPaTimer) {
        runtime.word8008ECFCResource10000TimerFrames = 0;
    }
    if (allowScriptEventPaByCoolness) {
        AssignStage1RuntimePairSlotFromScriptEventSlot(runtime.paMotion,
                                                       slots.paByCoolness,
                                                       applyFrame,
                                                       slots.sourceEventPsxAddr);
    }

    if (slots.camera.valid) {
        runtime.camera.valid = true;
        runtime.camera.bezId = slots.camera.bezId;
        runtime.camera.startFrame = applyFrame;
        runtime.camera.sourceEventPsxAddr = slots.sourceEventPsxAddr;
    }

    if (slots.performanceTod.valid) {
        runtime.performanceTod.valid = true;
        runtime.performanceTod.handle = slots.performanceTod.handle;
        runtime.performanceTod.sourceStreamId = slots.streamId;
        runtime.performanceTod.startFrame = applyFrame;
        runtime.performanceTod.sourceEventPsxAddr = slots.sourceEventPsxAddr;
    }

    if (slots.row0Tod.valid) {
        runtime.row0Tod.valid = true;
        runtime.row0Tod.handle = slots.row0Tod.handle;
        runtime.row0Tod.sourceStreamId = slots.streamId;
        runtime.row0Tod.startFrame = applyFrame;
        runtime.row0Tod.sourceEventPsxAddr = slots.sourceEventPsxAddr;
    }

    ApplyStage1RuntimeSceneRowFromScriptEvent(runtime,
                                              data,
                                              stream,
                                              streamId,
                                              ev,
                                              applyFrame,
                                              lateBranch);

    if (slots.baseSceneRowFlag) {
        runtime.cameraPlaybackEnabled3034 = true;
    }
}

void ApplyStage1RuntimeEventStreamDoneRecovery(
    PrStage1RuntimeSlotsDirectRuntime& runtime,
    const PrStage1OverlayData& data,
    uint32_t applyFrame,
    const PrStage1RuntimeSlotsDirectLateBranchInput& lateBranch) {
    if (!lateBranch.eventStreamDonePulse ||
        lateBranch.eventStreamFlagStream >= data.streamDescRows.size()) {
        return;
    }
    const auto& row =
        data.streamDescRows[lateBranch.eventStreamFlagStream].bytes;
    if (runtime.mode == 0u) {
        const uint8_t handle = row[1u];
        if (handle != 0u) {
            runtime.performanceTod.valid = true;
            runtime.performanceTod.handle = handle;
            runtime.performanceTod.sourceStreamId =
                lateBranch.eventStreamFlagStream;
            runtime.performanceTod.startFrame = applyFrame;
            runtime.performanceTod.sourceEventPsxAddr =
                kStage1EventStreamDonePsxAddr;
        }
        return;
    }

    const uint8_t performanceHandle = row[2u];
    if (performanceHandle != 0u) {
        runtime.performanceTod.valid = true;
        runtime.performanceTod.handle = performanceHandle;
        runtime.performanceTod.sourceStreamId =
            lateBranch.eventStreamFlagStream;
        runtime.performanceTod.startFrame = applyFrame;
        runtime.performanceTod.sourceEventPsxAddr =
            kStage1EventStreamDonePsxAddr;
    }
    const uint8_t row0Handle = row[0u];
    if (row0Handle != 0u) {
        runtime.row0Tod.valid = true;
        runtime.row0Tod.handle = row0Handle;
        runtime.row0Tod.sourceStreamId = lateBranch.eventStreamFlagStream;
        runtime.row0Tod.startFrame = applyFrame;
        runtime.row0Tod.sourceEventPsxAddr = kStage1EventStreamDonePsxAddr;
    }
}

}  // namespace

PrStage1RuntimeSlotsDirectCompactRailRuntime80024744
PrStage1RuntimeSlotsDirectBuildCompactRailProjection80024744(
    const PrStage1RuntimeSlotsDirectCompactProducer801C9094& producer) {
    return BuildStage1RuntimeCompactRailProjection80024744(producer);
}

PrStage1RuntimeSlotsDirectRunnerGlobalsCarrier801C9094
PrStage1RuntimeSlotsDirectBuildRunnerGlobalsCarrier801C9094(
    const PrStage1RuntimeSlotsDirectRuntime& runtime) {
    PrStage1RuntimeSlotsDirectRunnerGlobalsCarrier801C9094 out{};
    if (!runtime.active) {
        return out;
    }
    out.resourceTimersECFC_ECFEKnown = true;
    out.word8008ECFC =
        static_cast<int16_t>(runtime.word8008ECFCResource10000TimerFrames);
    out.word8008ECFE =
        static_cast<int16_t>(runtime.word8008ECFEResource20000TimerFrames);
    out.gateTimersD3054_D3058Known = true;
    out.dword801D3054 = runtime.dword801D3054ScriptEventOnByModeGateFrames;
    out.dword801D3058 =
        runtime.descBank.dword801D3058AcceptedTailGateFrames;
    const auto& compactProducer =
        runtime.sceneSubmit.compactProducer801C9094;
    if (compactProducer.cachedTeacherBodyPtr801D304C != 0u ||
        compactProducer.cachedTeacherBodyPtr801D3050 != 0u) {
        out.compactRailCacheD304C_D3050Known = true;
        out.dword801D304C = compactProducer.cachedTeacherBodyPtr801D304C;
        out.dword801D3050 = compactProducer.cachedTeacherBodyPtr801D3050;
    }
    return out;
}

PrStageRunnerDirectGlobals801C9094
PrStage1RuntimeSlotsDirectApplyRunnerGlobalsCarrier801C9094(
    const PrStageRunnerDirectGlobals801C9094& fallback,
    const PrStage1RuntimeSlotsDirectRunnerGlobalsCarrier801C9094& carrier) {
    PrStageRunnerDirectGlobals801C9094 out = fallback;
    if (carrier.resourceTimersECFC_ECFEKnown) {
        out.word8008ECFC = carrier.word8008ECFC;
        out.word8008ECFE = carrier.word8008ECFE;
    }
    if (carrier.gateTimersD3054_D3058Known) {
        out.dword801D3054 = carrier.dword801D3054;
        out.dword801D3058 = carrier.dword801D3058;
    }
    if (carrier.compactRailCacheD304C_D3050Known) {
        out.dword801D304C = carrier.dword801D304C;
        out.dword801D3050 = carrier.dword801D3050;
    }
    return out;
}

PrStage1RuntimeSlotsDirectLateBranchInput
PrStage1RuntimeSlotsDirectBuildEventStreamInput9094(
    const PrStageEventDirectStage1Runtime& runtime) {
    PrStage1RuntimeSlotsDirectLateBranchInput input{};
    input.selectedStream = runtime.selectedStream;
    input.ctxFlags40_2000_4000 = runtime.flags40_2000_4000;
    input.ctx72 = runtime.ctx72;
    input.ed00 = runtime.ed00;
    input.ed24 = runtime.ed24;
    input.eventStreamFlagActive = runtime.eventStreamFlagActive;
    input.eventStreamFlagStream = runtime.eventStreamFlagStream;
    input.activeDispatchStream = runtime.activeDispatchStream;
    input.activeDispatchStartFrame = runtime.activeDispatchStartFrame;
    input.activeDispatchStartScriptFrame =
        runtime.activeDispatchStartScriptFrame;
    input.activeDispatchEndLocalFrame = runtime.activeDispatchEndLocalFrame;
    input.flag2000SceneFamilyActive = runtime.flag2000SceneFamilyActive;
    input.flag2000SceneFamilyStream = runtime.flag2000SceneFamilyStream;
    input.flag2000SceneFamilyStartFrame =
        runtime.flag2000SceneFamilyStartFrame;
    input.flag2000SceneFamilyStartScriptFrame =
        runtime.flag2000SceneFamilyStartScriptFrame;
    input.eventStreamDonePulse = runtime.eventStreamDonePulse;
    input.eventStreamDoneSourceStream = runtime.eventStreamDoneSourceStream;
    input.flagStreamEvent801C9094 = runtime.flagStreamEvent801C9094;
    input.idStreamEvent801C9094 = runtime.idStreamEvent801C9094;
    input.consumedEventSource = runtime.frameResult801C9094.consumedEventSource;
    input.consumedEvent801C9094 = runtime.frameResult801C9094.consumedEvent;
    input.compactEvent801C9094 =
        runtime.frameResult801C9094.compactEvent801C9094;
    input.flag40EarlyReturnReset801C9094 =
        runtime.frameResult801C9094.flag40EarlyReturnReset801C9094;
    return input;
}

void PopulateStage1RuntimeEvent801C9094Snapshot(
    const PrStageEventDirectStage1ConsumedScriptEvent801C9094& src,
    PrStage1RuntimeEvent801C9094EventSnapshot& dst) {
    dst.valid = src.valid;
    dst.streamId = src.streamId;
    dst.eventIndex = src.eventIndex;
    dst.dueFrame = src.dueFrame;
    dst.psxAddr = src.psxAddr;
}

PrStage1RuntimeSlotsDirectRatingPresentationInput
PrStage1RuntimeSlotsDirectBuildRatingPresentationInput801CBFDC(
    const PrStage1RuntimeSlotsDirectRatingPresentationCarrier801CBFDC&
        carrier) {
    PrStage1RuntimeSlotsDirectRatingPresentationInput input{};
    input.flag0200FrameUpdateBranchTaken =
        carrier.flag0200FrameUpdateBranchTaken;
    input.flag0200FrameUpdateResourcePairKnown =
        carrier.flag0200FrameUpdateResourcePairKnown;
    input.flag0200FrameUpdateCtxFlag20000Set =
        carrier.flag0200FrameUpdateCtxFlag20000Set;
    input.flag0200FrameUpdateCtxE0ResourceIndex =
        carrier.flag0200FrameUpdateCtxE0ResourceIndex;
    input.flag0200FrameUpdateCtxECResourceIndex =
        carrier.flag0200FrameUpdateCtxECResourceIndex;
    return input;
}

PrStage1RuntimeSlotsDirectDescBankSelectorInput
PrStage1RuntimeSlotsDirectBuildDescBankSelectorInput801CBFDC(
    const PrStage1RuntimeSlotsDirectDescBankCarrier801CBFDC& carrier) {
    PrStage1RuntimeSlotsDirectDescBankSelectorInput input{};
    input.selectorValid = carrier.selectorValid;
    input.selector = carrier.selector;
    input.sourceFrame =
        carrier.selectorValid
            ? (carrier.selectorQueryFrame != 0u
                   ? carrier.selectorQueryFrame
                   : carrier.queryFrame)
            : 0u;
    input.ctxFlagsPulse801CBFDC = carrier.ctxFlagsPulse801CBFDC;
    input.uiRenderBlock4CC4Valid = true;
    input.uiRenderBlock4CC4 =
        static_cast<int16_t>(carrier.rightRankActiveRow);
    input.cameraValid9443C = carrier.cameraPulse9443C;
    input.cameraBezHandle9443C = carrier.cameraBezHandle9443C;
    input.cameraSourceFrame9443C =
        carrier.cameraPulse9443C ? carrier.cameraQueryFrame9443C : 0u;
    input.acceptedTailTimerValid9443C =
        carrier.acceptedTailTimer9443CPulse;
    input.acceptedTailTimerFrames9443C =
        carrier.acceptedTailTimer9443CFrames;
    input.acceptedTailTimerSourceFrame9443C =
        carrier.acceptedTailTimer9443CPulse
            ? carrier.acceptedTailTimer9443CQueryFrame
            : 0u;
    input.hudOverlayConfigure801CA0E0Valid =
        carrier.hudOverlayConfigure801CA0E0Pulse;
    input.hudOverlayConfigure801CA0E0Arg =
        carrier.hudOverlayConfigure801CA0E0Arg;
    input.hudOverlayConfigure801CA0E0SourceFrame =
        carrier.hudOverlayConfigure801CA0E0QueryFrame;
    input.hudOverlayConfigure801CA0E0SourceScriptFrame =
        carrier.hudOverlayConfigure801CA0E0ScriptFrame;
    input.compactPainterGate7AValid = true;
    input.compactPainterGate7A = carrier.compactPainterGate7A;
    input.compactRailRowCount8AClear80024FD0 =
        carrier.compactRailRowCount8AClear80024FD0;
    return input;
}

void PrStage1RuntimeSlotsDirectReset(
    PrStage1RuntimeSlotsDirectRuntime& runtime) {
    PrStage1RuntimeSlotsDirectReset(runtime,
                                    PrStage1RuntimeSlotsDirectResetInput80024E98{});
}

void PrStage1RuntimeSlotsDirectReset(
    PrStage1RuntimeSlotsDirectRuntime& runtime,
    const PrStage1RuntimeSlotsDirectResetInput80024E98& resetInput) {
    runtime = PrStage1RuntimeSlotsDirectRuntime{};
    ApplyStage1RuntimeSlotResetSub80024E98(runtime, resetInput);
    runtime.sceneSubmit.compactRail80024744.painterGate7A =
        static_cast<int16_t>(runtime.ctxInit80024E98.word7A_801C36BA);
    runtime.sceneSubmit.compactProducer801C9094.teacherState90 =
        static_cast<int16_t>(runtime.ctxInit80024E98.word90_801C36D0);
    runtime.sceneSubmit.compactProducer801C9094.studentStateA2 =
        static_cast<int16_t>(runtime.ctxInit80024E98.wordA2_801C36E2);
}

void PrStage1RuntimeSlotsDirectResetFromGlobalOptionsCarrier(
    PrStage1RuntimeSlotsDirectRuntime& runtime,
    const PrStage1RuntimeSlotsDirectGlobalOptionsCarrier& globalOptions) {
    PrStage1RuntimeSlotsDirectResetInput80024E98 resetInput{};
    if (globalOptions.word800916DCKnown) {
        resetInput.word800916DC = globalOptions.word800916DC;
        resetInput.word800916DCKnown = true;
    } else if (globalOptions.known) {
        resetInput.word800916DC = globalOptions.subtitleEnabled ? 1u : 0u;
        resetInput.word800916DCKnown = true;
    }
    PrStage1RuntimeSlotsDirectReset(runtime, resetInput);
}

PrStageEventDirectResetAction80024F8C
PrStage1RuntimeSlotsDirectResetEventAndSlots80024E98(
    PrStageEventDirectStage1Runtime& eventRuntime,
    PrStage1RuntimeSlotsDirectRuntime& slotsRuntime,
    const PrStage1RuntimeSlotsDirectGlobalOptionsCarrier& globalOptions) {
    PrStageEventDirectStage1Reset(eventRuntime);
    PrStage1RuntimeSlotsDirectResetFromGlobalOptionsCarrier(
        slotsRuntime,
        globalOptions);
    return PrStageEventDirectMergeResetActions80024F8C(
        PrStageEventDirectStage1GetResetAction80024F8C(eventRuntime),
        PrStage1RuntimeSlotsDirectGetResetAction80024F8C(slotsRuntime));
}

PrStage1RuntimeSlotsDirectGlobalOptionsCarrier
PrStage1RuntimeSlotsDirectBuildGlobalOptionsCarrierFromWord800916DC(
    bool known,
    uint16_t word800916DC) {
    PrStage1RuntimeSlotsDirectGlobalOptionsCarrier out{};
    out.known = known;
    out.subtitleEnabled = word800916DC != 0u;
    out.word800916DCKnown = known;
    out.word800916DC = word800916DC;
    return out;
}

const PrStageEventDirectResetAction80024F8C&
PrStage1RuntimeSlotsDirectGetResetAction80024F8C(
    const PrStage1RuntimeSlotsDirectRuntime& runtime) {
    return runtime.resetAction80024F8C;
}

static void ApplyStage1RuntimeDescBankSelector(
    PrStage1RuntimeSlotsDirectRuntime& runtime,
    const PrStage1RuntimeSlotsDirectDescBankSelectorInput& input,
    uint32_t queryFrame) {
    if (runtime.descBank.dword801D3058AcceptedTailGateFrames > 0) {
        return;
    }
    if (!input.selectorValid || input.selector > 4u) {
        return;
    }

    const uint32_t sourceFrame = input.sourceFrame != 0u ? input.sourceFrame : queryFrame;
    runtime.descBank.active = true;
    runtime.descBank.activeSelector = input.selector;
    runtime.descBank.startFrame = sourceFrame;
    runtime.sceneSubmit.ctxFlagsPulse801CBFDC |=
        input.ctxFlagsPulse801CBFDC & kStage1UiListSeedFlag801CBFDC;
    if (input.selector == 0u) {
        return;
    }

    const size_t groupIndex = static_cast<size_t>(input.selector - 1u);
    runtime.descBank.activeGroups[groupIndex] = true;
    runtime.descBank.groupStartFrames[groupIndex] = sourceFrame;
}

static void ApplyStage1RuntimeSceneSubmitInputs801CBFDC(
    PrStage1RuntimeSlotsDirectRuntime& runtime,
    const PrStage1RuntimeSlotsDirectLateBranchInput& lateBranch,
    const PrStage1RuntimeSlotsDirectDescBankSelectorInput& input,
    uint32_t queryFrame) {
    runtime.sceneSubmit.ctxFlags40_2000_4000 =
        lateBranch.ctxFlags40_2000_4000;
    runtime.sceneSubmit.ctxFlagsFull =
        (runtime.sceneSubmit.ctxFlagsFull &
         ~kStage1LateBranchCtxFlags40_2000_4000) |
        lateBranch.ctxFlags40_2000_4000;
    runtime.sceneSubmit.ctx72 = lateBranch.ctx72;
    if (input.compactPainterGate7AValid) {
        runtime.sceneSubmit.compactRail80024744.painterGate7A =
            static_cast<int16_t>(input.compactPainterGate7A);
        runtime.sceneSubmit.compactRail80024744.valid =
            runtime.sceneSubmit.compactRail80024744.rowCount8A > 0;
    } else {
        runtime.sceneSubmit.compactRail80024744.painterGate7A = 0;
        runtime.sceneSubmit.compactRail80024744.valid = false;
    }
    runtime.sceneSubmit.uiRenderBlock4CC4Valid =
        input.uiRenderBlock4CC4Valid;
    if (!input.uiRenderBlock4CC4Valid) {
        runtime.sceneSubmit.uiRenderBlock4CC4 = 0;
        runtime.sceneSubmit.uiRenderBlock4CC4Frame = 0;
        return;
    }

    runtime.sceneSubmit.uiRenderBlock4CC4 = input.uiRenderBlock4CC4;
    runtime.sceneSubmit.uiRenderBlock4CC4Frame =
        input.uiRenderBlock4CC4Frame != 0u ? input.uiRenderBlock4CC4Frame
                                           : queryFrame;
}

static void ApplyStage1RuntimeDescBankCameraPulse(
    PrStage1RuntimeSlotsDirectRuntime& runtime,
    const PrStage1RuntimeSlotsDirectDescBankSelectorInput& input,
    uint32_t queryFrame) {
    if (runtime.descBank.dword801D3058AcceptedTailGateFrames > 0) {
        return;
    }
    if (!input.cameraValid9443C || input.cameraBezHandle9443C == 0u) {
        return;
    }

    runtime.camera.valid = true;
    runtime.camera.bezId = static_cast<int16_t>(input.cameraBezHandle9443C);
    runtime.camera.startFrame =
        input.cameraSourceFrame9443C != 0u ? input.cameraSourceFrame9443C
                                           : queryFrame;
    runtime.camera.sourceEventPsxAddr = kStage1AcceptedTail9443CPsxAddr;
    runtime.cameraPlaybackEnabled3034 = true;
    runtime.sceneSubmit.ctxFlagsPulse801CBFDC |=
        input.ctxFlagsPulse801CBFDC & kStage1CameraBezFlag801CBFDC;
}

static void TickStage1RuntimeDword801D3058AcceptedTailGate(
    PrStage1RuntimeSlotsDirectRuntime& runtime) {
    if (runtime.descBank.dword801D3058AcceptedTailGateFrames > 0) {
        --runtime.descBank.dword801D3058AcceptedTailGateFrames;
    }
}

static void ApplyStage1RuntimeDword801D3058AcceptedTailGatePulse(
    PrStage1RuntimeSlotsDirectRuntime& runtime,
    const PrStage1RuntimeSlotsDirectDescBankSelectorInput& input,
    uint32_t queryFrame) {
    if (runtime.descBank.dword801D3058AcceptedTailGateFrames > 0) {
        return;
    }
    if (!input.acceptedTailTimerValid9443C ||
        input.acceptedTailTimerFrames9443C <= 0) {
        return;
    }

    runtime.descBank.dword801D3058AcceptedTailGateFrames =
        input.acceptedTailTimerFrames9443C;
    runtime.descBank.dword801D3058AcceptedTailStartFrame =
        input.acceptedTailTimerSourceFrame9443C != 0u
            ? input.acceptedTailTimerSourceFrame9443C
            : queryFrame;
    runtime.descBank.dword801D3058AcceptedTailSourceEventPsxAddr =
        kStage1AcceptedTail9443CPsxAddr;
}

bool PrStage1RuntimeSlotsDirectGetActiveScriptEvent(
    const PrStage1RuntimeSlotsDirectRuntime& runtime,
    const PrStage1OverlayData& data,
    PrStage1RuntimeSlotsDirectActiveEventResult& out) {
    out = PrStage1RuntimeSlotsDirectActiveEventResult{};
    if (!runtime.activeEventAvailable || runtime.activeEventStreamId == 0u) {
        return false;
    }

    const PrStage1EventStream* stream =
        data.FindStream(runtime.activeEventStreamId);
    if (!stream ||
        runtime.activeEventIndex >= stream->events.size()) {
        return false;
    }

    const PrStage1ScriptEvent& ev =
        stream->events[(size_t)runtime.activeEventIndex];
    if (ev.psxAddr != runtime.activeEventPsxAddr) {
        return false;
    }

    out.stream = stream;
    out.streamId = runtime.activeEventStreamId;
    out.event = &ev;
    out.dueFrame = runtime.activeEventDueFrame;
    return true;
}

bool PrStage1RuntimeSlotsDirectGetPrepassScriptEvent(
    const PrStage1RuntimeSlotsDirectRuntime& runtime,
    const PrStage1OverlayData& data,
    PrStage1RuntimeSlotsDirectActiveEventResult& out) {
    out = PrStage1RuntimeSlotsDirectActiveEventResult{};
    if (!runtime.prepassEventAvailable || runtime.prepassEventStreamId == 0u) {
        return false;
    }

    const PrStage1EventStream* stream =
        data.FindStream(runtime.prepassEventStreamId);
    if (!stream ||
        runtime.prepassEventIndex >= stream->events.size()) {
        return false;
    }

    const PrStage1ScriptEvent& ev =
        stream->events[(size_t)runtime.prepassEventIndex];
    if (ev.psxAddr != runtime.prepassEventPsxAddr) {
        return false;
    }

    out.stream = stream;
    out.streamId = runtime.prepassEventStreamId;
    out.event = &ev;
    out.dueFrame = runtime.prepassEventDueFrame;
    return true;
}

void PrStage1RuntimeSlotsDirectAdvance(
    PrStage1RuntimeSlotsDirectRuntime& runtime,
    const PrStage1OverlayData& data,
    const PrStage1RuntimeSlotsDirectAdvanceInput& input) {
    const uint32_t hostFrame = input.queryFrame;
    const uint32_t scriptFrame = input.scriptFrame;
    if (runtime.lastQueryFrame >= 0 &&
        (int32_t)hostFrame < runtime.lastQueryFrame) {
        PrStage1RuntimeSlotsDirectReset(runtime);
    }
    if (runtime.lastScriptFrame >= 0 &&
        (int32_t)scriptFrame < runtime.lastScriptFrame) {
        PrStage1RuntimeSlotsDirectReset(runtime);
    }

    if (!runtime.active) {
        runtime.active = true;
        ApplyStage1RuntimeSlotDefaults(runtime, &data, hostFrame);
    }
    if (input.descBank.compactRailRowCount8AClear80024FD0) {
        ClearStage1RuntimeCompactRailRowCount8A80024FD0(runtime);
    }
    const bool sameFrameNewEvent801C9094 =
        runtime.lastQueryFrame == (int32_t)hostFrame &&
        runtime.lastScriptFrame == (int32_t)scriptFrame &&
        (HasNewStage1RuntimeConsumedEvent801C9094(runtime, input.lateBranch) ||
         HasNewStage1RuntimeCompactEvent801C9094(runtime, input.lateBranch));
    runtime.lastConsumedEventSource801C9094 =
        input.lateBranch.consumedEventSource;
    runtime.lastFlagStreamEvent801C9094 =
        input.lateBranch.flagStreamEvent801C9094;
    runtime.lastIdStreamEvent801C9094 =
        input.lateBranch.idStreamEvent801C9094;
    runtime.lastConsumedEvent801C9094 =
        input.lateBranch.consumedEvent801C9094;
    runtime.lastCompactEvent801C9094 =
        input.lateBranch.compactEvent801C9094;
    if (runtime.lastQueryFrame == (int32_t)hostFrame &&
        runtime.lastScriptFrame == (int32_t)scriptFrame) {
        ApplyStage1RuntimeSlotsSteadyEntry801C7A60(runtime, input);
        // 801C9094: ctx&0x4000 starts stream7 and resets compact lanes
        // before the flag-stream compact prepass continues.
        if ((input.lateBranch.ctxFlags40_2000_4000 & 0x4000u) != 0u) {
            ResetStage1RuntimeCompactProducer801C9094(runtime);
            runtime.sceneSubmit.compactTeacherResetPulse80024308 = true;
            runtime.sceneSubmit.compactStudentResetPulse80024390 = true;
        }
        if (sameFrameNewEvent801C9094) {
            if (ShouldApplyStage1FlagCompactPrepass801C9094(input.lateBranch) &&
                input.railCursorEvent801C9094 != nullptr &&
                input.railCursorEvent801C9094->compactEventBearing801C9094) {
                Psx801C9094RunCompactProducer(
                    runtime,
                    data,
                    input.railCursorEvent801C9094,
                    scriptFrame,
                    input.ctxFlagTickAdvanceKnown801C9094,
                    input.ctxFlagTickAdvance801C9094,
                    false);
            }
            ClearStage1RuntimeActiveEvent801C9094(runtime);
            ApplyStage1RatingPresentationResourcePairSlot(
                runtime,
                input.ratingPresentation,
                hostFrame);
            std::vector<Stage1RuntimeDueScriptEvent> dueEvents;
            AppendStage1RuntimeDueEventsFromFrameResult801C9094(
                runtime,
                data,
                input.lateBranch,
                scriptFrame,
                hostFrame,
                dueEvents);
            for (const Stage1RuntimeDueScriptEvent& active : dueEvents) {
                if (!active.stream || !active.event) {
                    continue;
                }
                ApplyStage1RuntimeSlotsFromEvent(
                    runtime,
                    data,
                    active.stream,
                    active.streamId,
                    *active.event,
                    runtime.mode,
                    active.dueFrame,
                    active.applyScriptFrame,
                    active.applyFrame,
                    input.railCursorEvent801C9094,
                    input.lateBranch,
                    input.ctxFlagTickAdvanceKnown801C9094,
                    input.ctxFlagTickAdvance801C9094);
                runtime.lastAppliedEventPsxAddr = active.event->psxAddr;
                runtime.lastAppliedEventDueFrame = active.dueFrame;
            }
        }
        ApplyStage1RuntimeSceneSubmitInputs801CBFDC(
            runtime,
            input.lateBranch,
            input.descBank,
            hostFrame);
        ApplyStage1RuntimeDescBankSelector(
            runtime,
            input.descBank,
            hostFrame);
        ApplyStage1RuntimeDescBankCameraPulse(
            runtime,
            input.descBank,
            hostFrame);
        ApplyStage1RuntimeDword801D3058AcceptedTailGatePulse(
            runtime,
            input.descBank,
            hostFrame);
        ApplyStage1RuntimeHudOverlayConfigure801CA0E0(
            runtime,
            data,
            input.descBank,
            hostFrame,
            scriptFrame);
        return;
    }

    runtime.queryFrame = hostFrame;
    runtime.scriptFrame = scriptFrame;
    runtime.mode = input.currentMode;
    CaptureStage1RuntimeCompactPainterInput801CBFDC(runtime);
    ApplyStage1RuntimeSlotsSteadyEntry801C7A60(runtime, input);
    runtime.sceneSubmit.ctxFlagsPulse801CBFDC = 0u;
    runtime.sceneSubmit.flag2000DynamicRowReload801CBFDC = false;
    runtime.sceneSubmit.sceneInitPulse801CB6BC = false;
    runtime.sceneSubmit.compactWobbleResetPulse80024744 = false;
    runtime.sceneSubmit.compactTeacherResetPulse80024308 = false;
    runtime.sceneSubmit.compactStudentResetPulse80024390 = false;
    ClearStage1RuntimePrepassEvent(runtime);
    ApplyStage1RuntimeSlotDefaults(runtime, &data, hostFrame);
    ClearStage1RuntimeHudTimUploads(runtime);

    TickStage1RuntimeWord8008ECFCResource10000(
        runtime,
        hostFrame,
        scriptFrame,
        input.lateBranch.eventStreamFlagActive);
    TickStage1RuntimeWord8008ECFEResource20000(
        runtime,
        hostFrame,
        input.lateBranch.ed00 == 5);
    TickStage1RuntimeDword801D3054ScriptEventOnByModeGate(runtime);
    TickStage1RuntimeDword801D3058AcceptedTailGate(runtime);
    if (input.ctxFlagTickAdvanceKnown801C9094 &&
        input.ctxFlagTickAdvance801C9094) {
        runtime.sceneSubmit.ctxFlagsFull |= 0x00000008u;
    }
    if (input.lateBranch.flag40EarlyReturnReset801C9094) {
        ResetStage1RuntimeCompactProducer801C9094(runtime);
        runtime.sceneSubmit.compactTeacherResetPulse80024308 = true;
        runtime.sceneSubmit.compactStudentResetPulse80024390 = true;
    }
    // 801C9094: ctx&0x4000 starts stream7 and resets compact lanes before the
    // flag-stream compact prepass continues.
    if ((input.lateBranch.ctxFlags40_2000_4000 & 0x4000u) != 0u) {
        ResetStage1RuntimeCompactProducer801C9094(runtime);
        runtime.sceneSubmit.compactTeacherResetPulse80024308 = true;
        runtime.sceneSubmit.compactStudentResetPulse80024390 = true;
    }
    Psx801C9094RunCompactProducer(runtime,
                                  data,
                                  nullptr,
                                  scriptFrame,
                                  input.ctxFlagTickAdvanceKnown801C9094,
                                  input.ctxFlagTickAdvance801C9094,
                                  true);
    if (ShouldApplyStage1FlagCompactPrepass801C9094(input.lateBranch) &&
        input.railCursorEvent801C9094 != nullptr &&
        input.railCursorEvent801C9094->compactEventBearing801C9094) {
        Psx801C9094RunCompactProducer(runtime,
                                      data,
                                      input.railCursorEvent801C9094,
                                      scriptFrame,
                                      input.ctxFlagTickAdvanceKnown801C9094,
                                      input.ctxFlagTickAdvance801C9094,
                                      false);
    }
    TickStage1RuntimeHudTimedLists(runtime, data, scriptFrame);
    ApplyStage1RuntimeEventStreamDoneRecovery(
        runtime,
        data,
        hostFrame,
        input.lateBranch);
    ApplyStage1RatingPresentationResourcePairSlot(
        runtime,
        input.ratingPresentation,
        hostFrame);

    ClearStage1RuntimeActiveEvent801C9094(runtime);
    std::vector<Stage1RuntimeDueScriptEvent> dueEvents;
    AppendStage1RuntimeDueEventsFromFrameResult801C9094(
        runtime,
        data,
        input.lateBranch,
        scriptFrame,
        hostFrame,
        dueEvents);
    for (const Stage1RuntimeDueScriptEvent& active : dueEvents) {
        if (!active.stream || !active.event) {
            continue;
        }
        ApplyStage1RuntimeSlotsFromEvent(runtime,
                                         data,
                                         active.stream,
                                         active.streamId,
                                         *active.event,
                                         runtime.mode,
                                         active.dueFrame,
                                         active.applyScriptFrame,
                                         active.applyFrame,
                                         input.railCursorEvent801C9094,
                                         input.lateBranch,
                                         input.ctxFlagTickAdvanceKnown801C9094,
                                         input.ctxFlagTickAdvance801C9094);
        runtime.lastAppliedEventPsxAddr = active.event->psxAddr;
        runtime.lastAppliedEventDueFrame = active.dueFrame;
    }

    ApplyStage1RuntimeSceneSubmitInputs801CBFDC(
        runtime,
        input.lateBranch,
        input.descBank,
        hostFrame);
    ApplyStage1RuntimeDescBankSelector(
        runtime,
        input.descBank,
        hostFrame);
    ApplyStage1RuntimeDescBankCameraPulse(
        runtime,
        input.descBank,
        hostFrame);
    ApplyStage1RuntimeDword801D3058AcceptedTailGatePulse(
        runtime,
        input.descBank,
        hostFrame);
    ApplyStage1RuntimeHudOverlayConfigure801CA0E0(
        runtime,
        data,
        input.descBank,
        hostFrame,
        scriptFrame);

    runtime.lastQueryFrame = (int32_t)hostFrame;
    runtime.lastScriptFrame = (int32_t)scriptFrame;
}

void PrStage1RuntimeSlotsDirectApplyFollowUpFacts801CBFDC(
    PrStage1RuntimeSlotsDirectAdvanceCarrier801C9094& carrier,
    const PrStage1RuntimeSlotsDirectFollowUpFacts801CBFDC& facts,
    uint32_t queryFrame) {
    carrier.ratingPresentation.flag0200FrameUpdateBranchTaken =
        facts.flag0200FrameUpdateBranchTaken;
    carrier.ratingPresentation.flag0200FrameUpdateResourcePairKnown =
        facts.flag0200FrameUpdateResourcePairKnown;
    carrier.ratingPresentation.flag0200FrameUpdateCtxFlag20000Set =
        facts.flag0200FrameUpdateCtxFlag20000Set;
    carrier.ratingPresentation.flag0200FrameUpdateCtxE0ResourceIndex =
        facts.flag0200FrameUpdateCtxE0ResourceIndex;
    carrier.ratingPresentation.flag0200FrameUpdateCtxECResourceIndex =
        facts.flag0200FrameUpdateCtxECResourceIndex;
    carrier.descBank.queryFrame = queryFrame;
    carrier.descBank.selectorValid = facts.descBankSelectorPulse;
    carrier.descBank.selector = facts.descBankSelector;
    carrier.descBank.selectorQueryFrame = facts.descBankSelectorQueryFrame;
    carrier.descBank.ctxFlagsPulse801CBFDC =
        facts.descBankCtxFlagsPulse801CBFDC;
    carrier.descBank.rightRankActiveRow = facts.rightRankActiveRow;
    carrier.descBank.cameraPulse9443C = facts.cameraPulse9443C;
    carrier.descBank.cameraBezHandle9443C = facts.cameraBezHandle9443C;
    carrier.descBank.cameraQueryFrame9443C = facts.cameraQueryFrame9443C;
    carrier.descBank.acceptedTailTimer9443CPulse =
        facts.acceptedTailTimer9443CPulse;
    carrier.descBank.acceptedTailTimer9443CFrames =
        facts.acceptedTailTimer9443CFrames;
    carrier.descBank.acceptedTailTimer9443CQueryFrame =
        facts.acceptedTailTimer9443CQueryFrame;
    carrier.descBank.hudOverlayConfigure801CA0E0Pulse =
        facts.hudOverlayConfigure801CA0E0Pulse;
    carrier.descBank.hudOverlayConfigure801CA0E0Arg =
        facts.hudOverlayConfigure801CA0E0Arg;
    carrier.descBank.hudOverlayConfigure801CA0E0QueryFrame =
        facts.hudOverlayConfigure801CA0E0QueryFrame;
    carrier.descBank.hudOverlayConfigure801CA0E0ScriptFrame =
        facts.hudOverlayConfigure801CA0E0ScriptFrame;
    carrier.descBank.compactPainterGate7A = facts.compactPainterGate7A;
    carrier.descBank.compactRailRowCount8AClear80024FD0 =
        facts.compactRailRowCount8AClear80024FD0;
    carrier.ctxFlagTickAdvanceKnown801C9094 =
        facts.ctxFlagTickAdvanceKnown801C9094;
    carrier.ctxFlagTickAdvance801C9094 =
        facts.ctxFlagTickAdvance801C9094;
}

PrStage1RuntimeSlotsDirectAdvanceCarrier801C9094
PrStage1RuntimeSlotsDirectBuildAdvanceCarrier801C9094(
    uint32_t queryFrame,
    uint32_t scriptFrame,
    const PrStage1RuntimeSlotsDirectGlobalOptionsCarrier& globalOptions,
    const PrStageEventDirectStage1Runtime* eventStreamRuntime801C9094,
    const PrStage1RuntimeSlotsDirectFollowUpFacts801CBFDC& followUpFacts) {
    PrStage1RuntimeSlotsDirectAdvanceCarrier801C9094 carrier{};
    carrier.queryFrame = queryFrame;
    carrier.scriptFrame = scriptFrame;
    carrier.word800916DCKnown801C7A60 = globalOptions.word800916DCKnown;
    carrier.word800916DC801C7A60 = globalOptions.word800916DC;
    carrier.eventStreamRuntime801C9094 = eventStreamRuntime801C9094;
    PrStage1RuntimeSlotsDirectApplyFollowUpFacts801CBFDC(
        carrier,
        followUpFacts,
        queryFrame);
    return carrier;
}

void PrStage1RuntimeSlotsDirectAdvanceFromCarrier801C9094(
    PrStage1RuntimeSlotsDirectRuntime& runtime,
    const PrStage1OverlayData& data,
    const PrStage1RuntimeSlotsDirectAdvanceCarrier801C9094& carrier) {
    PrStage1RuntimeSlotsDirectAdvanceInput input{};
    input.queryFrame = carrier.queryFrame;
    input.scriptFrame = carrier.scriptFrame;
    input.currentMode =
        PrStageEventDirectStage1ResolveCurrentMode801C9094(
            carrier.descBank.rightRankActiveRow);
    input.word800916DCKnown801C7A60 =
        carrier.word800916DCKnown801C7A60;
    input.word800916DC801C7A60 = carrier.word800916DC801C7A60;
    input.ctxFlagTickAdvanceKnown801C9094 =
        carrier.ctxFlagTickAdvanceKnown801C9094;
    input.ctxFlagTickAdvance801C9094 =
        carrier.ctxFlagTickAdvance801C9094;
    if (carrier.eventStreamRuntime801C9094 != nullptr) {
        input.lateBranch =
            PrStage1RuntimeSlotsDirectBuildEventStreamInput9094(
                *carrier.eventStreamRuntime801C9094);
    }
    input.ratingPresentation =
        PrStage1RuntimeSlotsDirectBuildRatingPresentationInput801CBFDC(
            carrier.ratingPresentation);
    input.descBank =
        PrStage1RuntimeSlotsDirectBuildDescBankSelectorInput801CBFDC(
            carrier.descBank);

    PrStage1RailCursorEventDirectInput railInput{};
    railInput.data = &data;
    railInput.lateBranchFlags40_2000_4000 =
        input.lateBranch.ctxFlags40_2000_4000;
    railInput.runtimeActive = runtime.active;
    railInput.runtimeActiveStreamId = runtime.activeEventStreamId;
    railInput.previousLastAppliedEventPsxAddr =
        runtime.lastAppliedEventPsxAddr;
    railInput.previousLastAppliedEventDueFrame =
        runtime.lastAppliedEventDueFrame;
    railInput.consumedEvent801C9094 = input.lateBranch.consumedEvent801C9094;
    railInput.consumedEventSource = input.lateBranch.consumedEventSource;
    railInput.compactEvent801C9094 = input.lateBranch.compactEvent801C9094;

    PrStage1RailCursorEventDirectResult railResult{};
    PrStage1RailCursorEventDirectAdvance(railInput, railResult);
    input.railCursorEvent801C9094 = &railResult;

    PrStage1RuntimeSlotsDirectAdvance(runtime, data, input);
}

void PrStage1RuntimeSlotsDirectApplyStageLoopExit801C7A60(
    PrStage1RuntimeSlotsDirectRuntime& runtime) {
    runtime.steadyCtx801C7A60.valid = true;
    runtime.steadyCtx801C7A60.word54Known = true;
    runtime.steadyCtx801C7A60.word54_801C3694 = 0u;
    runtime.steadyCtx801C7A60.word54ClearedOnExit = true;
}

void PrStage1RuntimeSlotsDirectPopulateSnapshot(
    const PrStage1RuntimeSlotsDirectRuntime& runtime,
    uint32_t queryFrame60,
    uint32_t scriptQueryFrame,
    PrStage1RuntimeSlotsSnapshot& out) {
    out = PrStage1RuntimeSlotsSnapshot{};
    if (!runtime.active) {
        return;
    }

    out.valid = true;
    out.queryFrame = runtime.queryFrame;
    out.queryFrame60 = queryFrame60;
    out.scriptQueryFrame = runtime.scriptFrame != 0u ? runtime.scriptFrame
                                                     : scriptQueryFrame;
    out.mode = runtime.mode;
    out.cameraPlaybackEnabled3034 = runtime.cameraPlaybackEnabled3034;

    out.onMotion.valid = runtime.onMotion.valid;
    out.onMotion.pairIndex = runtime.onMotion.pairIndex;
    out.onMotion.datHandle = runtime.onMotion.datHandle;
    out.onMotion.vdfHandle = runtime.onMotion.vdfHandle;
    out.onMotion.startFrame = runtime.onMotion.startFrame;
    out.onMotion.sourceEventPsxAddr = runtime.onMotion.sourceEventPsxAddr;

    out.paMotion.valid = runtime.paMotion.valid;
    out.paMotion.pairIndex = runtime.paMotion.pairIndex;
    out.paMotion.datHandle = runtime.paMotion.datHandle;
    out.paMotion.vdfHandle = runtime.paMotion.vdfHandle;
    out.paMotion.startFrame = runtime.paMotion.startFrame;
    out.paMotion.sourceEventPsxAddr = runtime.paMotion.sourceEventPsxAddr;

    out.camera.valid = runtime.camera.valid;
    out.camera.bezId = runtime.camera.bezId;
    out.camera.startFrame = runtime.camera.startFrame;
    out.camera.sourceEventPsxAddr = runtime.camera.sourceEventPsxAddr;

    out.performanceTod.valid = runtime.performanceTod.valid;
    out.performanceTod.handle = runtime.performanceTod.handle;
    out.performanceTod.sourceStreamId = runtime.performanceTod.sourceStreamId;
    out.performanceTod.startFrame = runtime.performanceTod.startFrame;
    out.performanceTod.sourceEventPsxAddr = runtime.performanceTod.sourceEventPsxAddr;

    out.row0Tod.valid = runtime.row0Tod.valid;
    out.row0Tod.handle = runtime.row0Tod.handle;
    out.row0Tod.sourceStreamId = runtime.row0Tod.sourceStreamId;
    out.row0Tod.startFrame = runtime.row0Tod.startFrame;
    out.row0Tod.sourceEventPsxAddr = runtime.row0Tod.sourceEventPsxAddr;

    out.scene.valid = runtime.scene.valid;
    out.scene.selectedRow = runtime.scene.selectedRow;
    out.scene.selectedStream = runtime.scene.selectedStream;
    out.scene.sharedTodListEnabled = runtime.scene.sharedTodListEnabled;
    out.scene.streamStartFrame = runtime.scene.streamStartFrame;
    out.scene.streamWindowFrames = runtime.scene.streamWindowFrames;
    out.scene.rowHandles = runtime.scene.rowHandles;

    out.event801C9094.valid = true;
    out.event801C9094.consumedEventSource =
        static_cast<uint8_t>(runtime.lastConsumedEventSource801C9094);
    PopulateStage1RuntimeEvent801C9094Snapshot(
        runtime.lastFlagStreamEvent801C9094,
        out.event801C9094.flagStreamEvent);
    PopulateStage1RuntimeEvent801C9094Snapshot(
        runtime.lastIdStreamEvent801C9094,
        out.event801C9094.idStreamEvent);
    PopulateStage1RuntimeEvent801C9094Snapshot(
        runtime.lastConsumedEvent801C9094,
        out.event801C9094.consumedEvent);
    PopulateStage1RuntimeEvent801C9094Snapshot(
        runtime.lastCompactEvent801C9094,
        out.event801C9094.compactEvent);

    out.descBank.valid = runtime.descBank.active;
    out.descBank.selector = runtime.descBank.activeSelector;
    out.descBank.startFrame = runtime.descBank.startFrame;
    out.descBank.activeGroups = runtime.descBank.activeGroups;
    out.descBank.groupStartFrames = runtime.descBank.groupStartFrames;
    out.descBank.acceptedTailTimerFrames9443C =
        runtime.descBank.dword801D3058AcceptedTailGateFrames;
    out.descBank.acceptedTailTimerStartFrame9443C =
        runtime.descBank.dword801D3058AcceptedTailStartFrame;
    out.descBank.acceptedTailTimerSourceEventPsxAddr =
        runtime.descBank.dword801D3058AcceptedTailSourceEventPsxAddr;

    out.tamaSelector.valid = runtime.tamaSelector.valid;
    out.tamaSelector.selector = runtime.tamaSelector.selector;
    out.tamaSelector.startFrame = runtime.tamaSelector.startFrame;
    out.tamaSelector.sourceEventPsxAddr = runtime.tamaSelector.sourceEventPsxAddr;

    for (size_t i = 0; i < runtime.hudTimUploads.size() &&
                       i < out.hudTimUploads.size(); ++i) {
        const auto& src = runtime.hudTimUploads[i];
        auto& dst = out.hudTimUploads[i];
        dst.valid = src.valid;
        dst.channel = src.channel;
        dst.slotId = src.slotId;
        dst.eventIndex = src.eventIndex;
        dst.dueFrame = src.dueFrame;
        dst.sourceEventPsxAddr = src.sourceEventPsxAddr;
        dst.timIds = src.timIds;
    }

    out.sceneSubmit.uiRenderBlock4CC4Valid =
        runtime.sceneSubmit.uiRenderBlock4CC4Valid;
    out.sceneSubmit.uiRenderBlock4CC4 =
        runtime.sceneSubmit.uiRenderBlock4CC4;
    out.sceneSubmit.uiRenderBlock4CC4Frame =
        runtime.sceneSubmit.uiRenderBlock4CC4Frame;
    out.sceneSubmit.directScriptBoxGate54Known =
        runtime.steadyCtx801C7A60.word54Known ||
        !runtime.ctxInit80024E98.word800916DCSeedGap;
    out.sceneSubmit.directScriptBoxGate54 =
        runtime.steadyCtx801C7A60.word54Known
            ? runtime.steadyCtx801C7A60.word54_801C3694
            : runtime.ctxInit80024E98.word54_801C3694;
    out.sceneSubmit.directScriptBoxArg68Known =
        runtime.steadyCtx801C7A60.word68Known ||
        !runtime.ctxInit80024E98.word800916DCSeedGap;
    out.sceneSubmit.directScriptBoxArg68 =
        static_cast<int16_t>(
            runtime.steadyCtx801C7A60.word68Known
                ? runtime.steadyCtx801C7A60.word68_801C36A8
                : runtime.ctxInit80024E98.word68_801C36A8);
    out.sceneSubmit.ctxFlagsFull =
        runtime.sceneSubmit.ctxFlagsFull;
    out.sceneSubmit.ctxFlagTickAdvanceKnown801C9094 =
        runtime.sceneSubmit.ctxFlagTickAdvanceKnown801C9094;
    out.sceneSubmit.ctxFlagTickAdvance801C9094 =
        runtime.sceneSubmit.ctxFlagTickAdvance801C9094;
    out.sceneSubmit.ctxFlagsPulse801CBFDC =
        runtime.sceneSubmit.ctxFlagsPulse801CBFDC;
    out.sceneSubmit.ctxFlags40_2000_4000 =
        runtime.sceneSubmit.ctxFlags40_2000_4000;
    out.sceneSubmit.ctx72 = runtime.sceneSubmit.ctx72;
    out.sceneSubmit.ctx11C = runtime.sceneSubmit.ctx11C;
    out.sceneSubmit.ctx120 = runtime.sceneSubmit.ctx120;
    out.sceneSubmit.flag2000DynamicRowReload801CBFDC =
        runtime.sceneSubmit.flag2000DynamicRowReload801CBFDC;
    out.sceneSubmit.sceneInitPulse801CB6BC =
        runtime.sceneSubmit.sceneInitPulse801CB6BC;
    out.sceneSubmit.compactWobbleResetPulse80024744 =
        runtime.sceneSubmit.renderCompactWobbleResetPulse80024744;
    out.sceneSubmit.compactTeacherResetPulse80024308 =
        runtime.sceneSubmit.renderCompactTeacherResetPulse80024308;
    out.sceneSubmit.compactStudentResetPulse80024390 =
        runtime.sceneSubmit.renderCompactStudentResetPulse80024390;
    out.sceneSubmit.compactProducer801C9094.active =
        runtime.sceneSubmit.compactProducer801C9094.active;
    out.sceneSubmit.compactProducer801C9094.sourceEventPsxAddr =
        runtime.sceneSubmit.compactProducer801C9094.sourceEventPsxAddr;
    out.sceneSubmit.compactProducer801C9094.sourceFrame =
        runtime.sceneSubmit.compactProducer801C9094.sourceFrame;
    out.sceneSubmit.compactProducer801C9094.teacherRecordIndex =
        runtime.sceneSubmit.compactProducer801C9094.teacherRecordIndex;
    out.sceneSubmit.compactProducer801C9094.studentRecordIndex =
        runtime.sceneSubmit.compactProducer801C9094.studentRecordIndex;
    out.sceneSubmit.compactProducer801C9094.teacherMode8A =
        runtime.sceneSubmit.compactProducer801C9094.teacherMode8A;
    out.sceneSubmit.compactProducer801C9094.teacherCursor8C =
        runtime.sceneSubmit.compactProducer801C9094.teacherCursor8C;
    out.sceneSubmit.compactProducer801C9094.teacherCursor8E =
        runtime.sceneSubmit.compactProducer801C9094.teacherCursor8E;
    out.sceneSubmit.compactProducer801C9094.teacherState90 =
        runtime.sceneSubmit.compactProducer801C9094.teacherState90;
    out.sceneSubmit.compactProducer801C9094.studentCursor9E =
        runtime.sceneSubmit.compactProducer801C9094.studentCursor9E;
    out.sceneSubmit.compactProducer801C9094.studentCursorA0 =
        runtime.sceneSubmit.compactProducer801C9094.studentCursorA0;
    out.sceneSubmit.compactProducer801C9094.studentStateA2 =
        runtime.sceneSubmit.compactProducer801C9094.studentStateA2;
    out.sceneSubmit.compactRail80024744.valid =
        runtime.sceneSubmit.renderCompactRail80024744.valid;
    out.sceneSubmit.compactRail80024744.painterGate7A =
        runtime.sceneSubmit.renderCompactRail80024744.painterGate7A;
    out.sceneSubmit.compactRail80024744.rowCount8A =
        runtime.sceneSubmit.renderCompactRail80024744.rowCount8A;
    for (size_t i = 0;
         i < runtime.sceneSubmit.renderCompactRail80024744.rows.size() &&
         i < out.sceneSubmit.compactRail80024744.rows.size();
         ++i) {
        const auto& src =
            runtime.sceneSubmit.renderCompactRail80024744.rows[i];
        auto& dst = out.sceneSubmit.compactRail80024744.rows[i];
        dst.teacherRaw8C = src.teacherRaw8C;
        dst.studentRaw9E = src.studentRaw9E;
        dst.bodyStreamPtr94 = src.bodyStreamPtr94;
        dst.bodyStreamBytes94 = src.bodyStreamBytes94;
    }

    out.scriptCtxMirror.valid = runtime.scriptCtxMirror.valid;
    out.scriptCtxMirror.sourceEventPsxAddr =
        runtime.scriptCtxMirror.sourceEventPsxAddr;
    out.scriptCtxMirror.dueFrame = runtime.scriptCtxMirror.dueFrame;
    out.scriptCtxMirror.ctxFlagsSet = runtime.scriptCtxMirror.ctxFlagsSet;
    out.scriptCtxMirror.paDatCtxOffset =
        runtime.scriptCtxMirror.paDatCtxOffset;
    out.scriptCtxMirror.paVdfCtxOffset =
        runtime.scriptCtxMirror.paVdfCtxOffset;
    out.scriptCtxMirror.onDatCtxOffset =
        runtime.scriptCtxMirror.onDatCtxOffset;
    out.scriptCtxMirror.onVdfCtxOffset =
        runtime.scriptCtxMirror.onVdfCtxOffset;
    out.scriptCtxMirror.performanceTodCtxOffset =
        runtime.scriptCtxMirror.performanceTodCtxOffset;
    out.scriptCtxMirror.row0TodCtxOffset =
        runtime.scriptCtxMirror.row0TodCtxOffset;
    out.scriptCtxMirror.tamaSelectorCtxOffset =
        runtime.scriptCtxMirror.tamaSelectorCtxOffset;
    out.scriptCtxMirror.cameraBezCtxOffset =
        runtime.scriptCtxMirror.cameraBezCtxOffset;
}
