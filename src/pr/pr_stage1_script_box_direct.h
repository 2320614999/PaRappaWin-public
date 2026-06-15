#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace PrStage1ScriptBoxDirect {

inline constexpr size_t kScriptBoxRectCountSub80023E10 = 3u;
inline constexpr size_t kScriptBoxCornerCountSub80023E10 = 4u;

enum class ScriptBoxTextSubmitGapSub80023E10 : uint8_t {
    None,
    MissingTextHandle10C,
    MissingWorkListSlotCarrier,
    MissingGlyphLocalFastSpriteRgb,
    MissingFullFastSpritePacket,
};

struct ScriptBoxRectSub80023E10 {
    bool valid = false;
    int16_t x = 0;
    int16_t y = 0;
    uint16_t w = 0;
    uint16_t h = 0;
    uint32_t attr = 0x400F0F0Fu;
    uint16_t priority = 2;
};

struct ScriptBoxCornerSub80023E10 {
    bool valid = false;
    int16_t x = 0;
    int16_t y = 0;
    uint32_t templateAddr = 0;
    uint16_t priority = 2;
    uint16_t layer = 920;
    uint16_t callOrder = 0;
};

struct ScriptBoxOriginSub8001B730 {
    bool valid = false;
    int16_t x = 28;
    int16_t y = 194;
    int16_t z = 0;
};

struct ScriptBoxTextSub8001C6A0 {
    bool valid = false;
    uint32_t sourceFunction = 0x8001C6A0u;
    uint32_t tailFunction = 0x8001B954u;
    uint32_t submitFunction = 0x8003FA20u;
    uint32_t textHandle10C = 0;
    bool textPtr10CKnown = false;
    uint32_t workListBaseAddr = 0x80087288u;
    bool workListSlotKnown = false;
    uint8_t workListSlot = 0;
    uint32_t workListStrideBytes = 20u;
    bool workListAddrKnown = false;
    uint32_t workListAddr = 0;
    const char* text = nullptr;
    uint16_t scale = 480;
    bool originKnownSub8001B730 = false;
    int16_t originX = 28;
    int16_t originY = 194;
    int16_t originZ = 0;
    bool strlenKnownSub8001B954 = false;
    uint16_t strlenBytesSub8001B954 = 0;
    bool glyphLoopInputKnownSub8001C6A0 = false;
    bool wouldCallGlyphLoopSub8001B954 = false;
    bool glyphLocalFastSpriteRgbKnown = false;
    bool fullFastSpritePacketKnown = false;
    bool glyphPacketGap = true;
    ScriptBoxTextSubmitGapSub80023E10 firstGap =
        ScriptBoxTextSubmitGapSub80023E10::MissingTextHandle10C;
};

struct ScriptBoxSub80023E10 {
    bool valid = false;
    uint32_t sourceFunction = 0x80023E10u;
    std::array<ScriptBoxRectSub80023E10, kScriptBoxRectCountSub80023E10>
        rects{};
    std::array<ScriptBoxCornerSub80023E10, kScriptBoxCornerCountSub80023E10>
        corners{};
    ScriptBoxOriginSub8001B730 origin{};
    ScriptBoxTextSub8001C6A0 text{};
};

inline void PopulateScriptBoxGeometrySub80023E10(
    ScriptBoxSub80023E10& out) {
    out.rects[0] = {true, 10, 199, 8, 0x12u, 0x400F0F0Fu, 2};
    out.rects[1] = {true, 18, 191, 0x11Cu, 0x22u, 0x400F0F0Fu, 2};
    out.rects[2] = {true, 302, 199, 8, 0x12u, 0x400F0F0Fu, 2};

    out.corners[0] = {true, 10, 191, 0x80050900u, 2, 920, 0};
    out.corners[1] = {true, 10, 217, 0x800508F0u, 2, 920, 1};
    out.corners[2] = {true, 302, 191, 0x800508E0u, 2, 920, 2};
    out.corners[3] = {true, 302, 217, 0x800508D0u, 2, 920, 3};

    out.origin.valid = true;
}

inline uint16_t PsxStrLenLimitedSub8001C6A0(const char* text) {
    if (text == nullptr) {
        return 0;
    }
    uint16_t len = 0;
    while (text[len] != '\0' && len < 0xFFFFu) {
        ++len;
    }
    return len;
}

inline ScriptBoxSub80023E10 BuildScriptBoxFrameSub80023E10(
    uint32_t textHandle10C,
    const char* textPtr10C,
    uint16_t maxGlyphs,
    bool workListSlotKnown = false,
    uint8_t workListSlot = 0,
    uint32_t workListBaseAddr = 0x80087288u,
    uint32_t workListStrideBytes = 20u) {
    ScriptBoxSub80023E10 out{};
    out.valid = true;
    PopulateScriptBoxGeometrySub80023E10(out);
    out.text.valid = textPtr10C != nullptr;
    out.text.textHandle10C = textHandle10C;
    out.text.textPtr10CKnown = textPtr10C != nullptr;
    out.text.text = textPtr10C;
    out.text.scale = maxGlyphs;
    out.text.workListBaseAddr = workListBaseAddr;
    out.text.workListSlotKnown = workListSlotKnown;
    out.text.workListSlot = static_cast<uint8_t>(workListSlot & 1u);
    out.text.workListStrideBytes = workListStrideBytes;
    out.text.workListAddrKnown = workListSlotKnown;
    out.text.workListAddr =
        workListBaseAddr +
        static_cast<uint32_t>(out.text.workListSlot) * workListStrideBytes;
    out.text.originKnownSub8001B730 = out.origin.valid;
    out.text.originX = out.origin.x;
    out.text.originY = out.origin.y;
    out.text.originZ = out.origin.z;
    out.text.strlenKnownSub8001B954 = true;
    out.text.strlenBytesSub8001B954 =
        PsxStrLenLimitedSub8001C6A0(textPtr10C);
    out.text.glyphLoopInputKnownSub8001C6A0 =
        out.text.textPtr10CKnown &&
        out.text.workListAddrKnown &&
        out.text.originKnownSub8001B730;
    out.text.wouldCallGlyphLoopSub8001B954 =
        out.text.glyphLoopInputKnownSub8001C6A0 &&
        out.text.strlenBytesSub8001B954 != 0u;
    out.text.glyphLocalFastSpriteRgbKnown = false;
    out.text.fullFastSpritePacketKnown = false;
    out.text.glyphPacketGap = true;
    out.text.firstGap =
        textPtr10C == nullptr
            ? ScriptBoxTextSubmitGapSub80023E10::MissingTextHandle10C
        : workListSlotKnown
            ? ScriptBoxTextSubmitGapSub80023E10::MissingGlyphLocalFastSpriteRgb
            : ScriptBoxTextSubmitGapSub80023E10::MissingWorkListSlotCarrier;
    return out;
}

}  // namespace PrStage1ScriptBoxDirect
