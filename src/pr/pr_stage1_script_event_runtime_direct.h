#pragma once

#include "pr_stage1_overlay_parser.h"

#include <array>
#include <cstdint>

namespace PrStage1ScriptEventRuntimeDirect {

struct PairResourceSlot {
    bool valid = false;
    uint8_t pairIndex = 0;
    uint16_t datHandle = 0;
    uint16_t vdfHandle = 0;
    uint16_t datCtxOffset = 0;
    uint16_t vdfCtxOffset = 0;
    uint32_t ctxFlag = 0;
};

struct TodResourceSlot {
    bool valid = false;
    uint8_t handle = 0;
    uint16_t handleCtxOffset = 0;
    uint32_t ctxFlag = 0;
};

struct CameraBezSlot {
    bool valid = false;
    int16_t bezId = 0;
    uint16_t bezCtxOffset = 0;
    uint32_t ctxFlag = 0;
};

struct TamaSelectorSlot {
    bool valid = false;
    uint8_t selector = 0;
    uint16_t selectorCtxOffset = 0;
};

struct HudTimedListSlot {
    bool valid = false;
    uint8_t slotId = 0;
    uint8_t channel = 0;
};

struct ScriptEventSlots {
    bool valid = false;
    uint32_t sourceEventPsxAddr = 0;
    uint32_t dueFrame = 0;
    uint8_t streamId = 0;
    uint8_t mode = 0;
    uint32_t ctxFlagsSet = 0;
    PairResourceSlot paByCoolness{};
    PairResourceSlot onByMode{};
    TodResourceSlot performanceTod{};
    TodResourceSlot row0Tod{};
    CameraBezSlot camera{};
    TamaSelectorSlot tama{};
    std::array<HudTimedListSlot, 3> hudTimedLists{};
    bool baseSceneRowFlag = false;
};

bool MaterializeScriptEventSlots(const PrStage1OverlayData& data,
                                 const PrStage1EventStream* stream,
                                 uint8_t streamId,
                                 const PrStage1ScriptEvent& ev,
                                 uint8_t currentMode,
                                 uint32_t dueFrame,
                                 ScriptEventSlots& out);

}  // namespace PrStage1ScriptEventRuntimeDirect
