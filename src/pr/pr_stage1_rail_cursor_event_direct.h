#pragma once

#include "pr_stage_event_direct.h"
#include "pr_stage1_overlay_parser.h"

#include <cstdint>

struct PrStage1RailCursorEventDirectInput {
    const PrStage1OverlayData* data = nullptr;
    uint16_t lateBranchFlags40_2000_4000 = 0;
    bool runtimeActive = false;
    uint8_t runtimeActiveStreamId = 0;
    uint32_t previousLastAppliedEventPsxAddr = 0;
    uint32_t previousLastAppliedEventDueFrame = 0;
    PrStageEventDirectStage1ConsumedScriptEvent801C9094
        consumedEvent801C9094{};
    PrStageEventDirectStage1ConsumedEventSource801C9094 consumedEventSource =
        PrStageEventDirectStage1ConsumedEventSource801C9094::None;
    PrStageEventDirectStage1ConsumedScriptEvent801C9094
        compactEvent801C9094{};
};

struct PrStage1RailCursorEventDirectResult {
    bool hardReset = false;
    bool returnAfterHardReset = false;
    bool runtimeActive = false;
    uint8_t runtimeActiveStreamId = 0;
    const PrStage1EventStream* activeStream = nullptr;
    uint8_t activeStreamId = 0;
    const PrStage1ScriptEvent* activeEvent = nullptr;
    uint32_t activeDueFrame = 0;
    uint32_t streamBaseDueFrame = 0;
    int32_t activeEventIndex = -1;
    bool activeEventAvailable = false;
    uint32_t activeEventPsxAddr = 0;
    bool activeEventChangedThisFrame = false;
    bool activeEventShouldResetAuthority = false;
    bool activeEventResetAuthorityThisFrame = false;
    uint32_t lastAppliedEventPsxAddr = 0;
    uint32_t lastAppliedEventDueFrame = 0;
    uint32_t activeEventFlags04 = 0;
    uint8_t activeEventByte1D = 0;
    uint8_t activeEventByte1E = 0;
    uint8_t activeEventByte1F = 0;
    uint8_t activeEventTextId = 0;
    bool compactEventBearing801C9094 = false;
    bool compactResetRequested801C9094 = false;
    bool compactTeacherBootstrapRequested801C9094 = false;
    bool compactStudentCatchupRequested801C9094 = false;
    bool terminalTailRequested801C9094 = false;
    uint32_t compactTeacherRowBasePsxAddr801C9094 = 0;
    uint32_t compactTeacherLaneAPsxAddr801C9094 = 0;
    uint32_t compactTeacherLaneBPsxAddr801C9094 = 0;
};

bool PrStage1RailCursorEventDirectShouldResetForFlags(
    uint16_t lateBranchFlags40_2000_4000,
    const PrStage1ScriptEvent& ev);

void PrStage1RailCursorEventDirectPopulateEventFields801C9094(
    uint16_t lateBranchFlags40_2000_4000,
    const PrStage1ScriptEvent& ev,
    PrStage1RailCursorEventDirectResult& out);

void PrStage1RailCursorEventDirectAdvance(
    const PrStage1RailCursorEventDirectInput& input,
    PrStage1RailCursorEventDirectResult& out);
