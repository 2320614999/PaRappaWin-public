#include "pr_stage1_script_event_runtime_direct.h"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <string>

namespace PrStage1ScriptEventRuntimeDirect {
namespace {

constexpr uint32_t kStage1SceneBaseRowFlag = 0x00000020u;
constexpr uint32_t kStage1PairFlagPaByCoolness = 0x00010000u;
constexpr uint32_t kStage1PairFlagOnByMode = 0x00020000u;
constexpr uint32_t kStage1PerformanceTodFlag = 0x00040000u;
constexpr uint32_t kStage1Row0TodFlag = 0x00080000u;
constexpr uint32_t kStage1CameraBezFlag = 0x00000400u;

constexpr uint16_t kStage1CtxOffsetPaDatDC = 0x00DCu;
constexpr uint16_t kStage1CtxOffsetOnDatE0 = 0x00E0u;
constexpr uint16_t kStage1CtxOffsetPaVdfE8 = 0x00E8u;
constexpr uint16_t kStage1CtxOffsetOnVdfEC = 0x00ECu;
constexpr uint16_t kStage1CtxOffsetPerformanceTodF4 = 0x00F4u;
constexpr uint16_t kStage1CtxOffsetRow0TodF8 = 0x00F8u;
constexpr uint16_t kStage1CtxOffsetTamaSelectorFC = 0x00FCu;
constexpr uint16_t kStage1CtxOffsetCameraBez104 = 0x0104u;

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

bool ResolvePairResourceSlot(const PrStage1OverlayData& data,
                             uint8_t pairIndex,
                             PairResourceSlot& out) {
    out = PairResourceSlot{};
    if (pairIndex == 0u) {
        return false;
    }

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
            out.datHandle = (uint16_t)handle;
        } else if (EndsWithAscii(lowerName, ".vdf")) {
            out.vdfHandle = (uint16_t)handle;
        }
    };

    out.pairIndex = pairIndex;
    assignHandle(entry->idA);
    assignHandle(entry->idB);
    out.valid = out.datHandle != 0u && out.vdfHandle != 0u;
    return out.valid;
}

bool ResolveStreamDescTod(const PrStage1OverlayData& data,
                          uint8_t streamId,
                          uint8_t handleIndex,
                          TodResourceSlot& out) {
    out = TodResourceSlot{};
    if (streamId == 0u || streamId >= data.streamDescRows.size()) {
        return false;
    }

    const uint8_t handle = data.streamDescRows[streamId].bytes[handleIndex];
    if (handle == 0u) {
        return false;
    }

    out.valid = true;
    out.handle = handle;
    return true;
}

int16_t ResolveModeCameraBezId(const PrStage1ScriptEvent& ev, uint8_t modeIndex) {
    if (modeIndex < ev.bezIdByMode.size()) {
        return ev.bezIdByMode[modeIndex];
    }
    return 0;
}

}  // namespace

bool MaterializeScriptEventSlots(const PrStage1OverlayData& data,
                                 const PrStage1EventStream* stream,
                                 uint8_t streamId,
                                 const PrStage1ScriptEvent& ev,
                                 uint8_t currentMode,
                                 uint32_t dueFrame,
                                 ScriptEventSlots& out) {
    (void)stream;

    out = ScriptEventSlots{};
    out.valid = true;
    out.sourceEventPsxAddr = ev.psxAddr;
    out.dueFrame = dueFrame;
    out.streamId = streamId;
    out.mode = (std::min<uint8_t>)(currentMode, 3u);
    out.tama.valid = true;
    out.tama.selector = ev.byte16;
    out.tama.selectorCtxOffset = kStage1CtxOffsetTamaSelectorFC;

    auto assignHudTimedList = [&](uint8_t channel, uint8_t slotId) {
        if (channel >= out.hudTimedLists.size() || slotId == 0u) {
            return;
        }
        out.hudTimedLists[channel].valid = true;
        out.hudTimedLists[channel].slotId = slotId;
        out.hudTimedLists[channel].channel = channel;
    };
    assignHudTimedList(0u, ev.hudSlot0);
    assignHudTimedList(1u, ev.hudSlotByMode[out.mode]);
    assignHudTimedList(2u, ev.hudSlot2);

    if ((ev.flags04 & kStage1SceneBaseRowFlag) != 0u) {
        out.ctxFlagsSet |= kStage1SceneBaseRowFlag;
        out.baseSceneRowFlag = true;
    }

    if ((ev.flags04 & kStage1PerformanceTodFlag) != 0u) {
        const uint8_t handleIndex = (currentMode == 0u) ? 1u : 2u;
        if (ResolveStreamDescTod(data, streamId, handleIndex, out.performanceTod)) {
            out.ctxFlagsSet |= kStage1PerformanceTodFlag;
            out.performanceTod.handleCtxOffset = kStage1CtxOffsetPerformanceTodF4;
            out.performanceTod.ctxFlag = kStage1PerformanceTodFlag;
        }
    }

    if ((ev.flags04 & kStage1Row0TodFlag) != 0u) {
        if (ResolveStreamDescTod(data, streamId, 0u, out.row0Tod)) {
            out.ctxFlagsSet |= kStage1Row0TodFlag;
            out.row0Tod.handleCtxOffset = kStage1CtxOffsetRow0TodF8;
            out.row0Tod.ctxFlag = kStage1Row0TodFlag;
        }
    }

    if ((ev.flags04 & kStage1PairFlagPaByCoolness) != 0u) {
        const uint8_t pairIndex = (currentMode == 0u) ? ev.coolPair : ev.noncoolPair;
        if (ResolvePairResourceSlot(data, pairIndex, out.paByCoolness)) {
            out.ctxFlagsSet |= kStage1PairFlagPaByCoolness;
            out.paByCoolness.datCtxOffset = kStage1CtxOffsetPaDatDC;
            out.paByCoolness.vdfCtxOffset = kStage1CtxOffsetPaVdfE8;
            out.paByCoolness.ctxFlag = kStage1PairFlagPaByCoolness;
        }
    }

    if ((ev.flags04 & kStage1PairFlagOnByMode) != 0u) {
        const uint8_t pairIndex = ev.pairByMode[out.mode];
        if (ResolvePairResourceSlot(data, pairIndex, out.onByMode)) {
            out.ctxFlagsSet |= kStage1PairFlagOnByMode;
            out.onByMode.datCtxOffset = kStage1CtxOffsetOnDatE0;
            out.onByMode.vdfCtxOffset = kStage1CtxOffsetOnVdfEC;
            out.onByMode.ctxFlag = kStage1PairFlagOnByMode;
        }
    }

    const int16_t bezId = ResolveModeCameraBezId(ev, out.mode);
    if (bezId > 0) {
        out.camera.valid = true;
        out.camera.bezId = bezId;
        out.camera.bezCtxOffset = kStage1CtxOffsetCameraBez104;
        out.camera.ctxFlag = kStage1CameraBezFlag;
        out.ctxFlagsSet |= kStage1CameraBezFlag;
    }

    return true;
}

}  // namespace PrStage1ScriptEventRuntimeDirect
