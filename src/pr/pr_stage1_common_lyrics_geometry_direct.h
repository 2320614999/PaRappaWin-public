#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

struct PrStage1CommonLyricsLineGeometrySub8001B954 {
    bool valid = false;
    uint16_t textStartIndex = 0;
    uint16_t textByteCount = 0;
    uint16_t widthPx = 0;
    int16_t centeredX = 0;
    int16_t y = 0;
};

struct PrStage1CommonLyricsTextGeometrySub8001B954 {
    bool valid = false;
    uint8_t lineCount = 0;
    std::array<PrStage1CommonLyricsLineGeometrySub8001B954, 2> lines{};
};

inline constexpr size_t kPrStage1CommonLyricsFrameSubmitCapacitySub8001C864 = 96u;

struct PrStage1CommonLyricsFrameSubmitSub8001C864 {
    bool valid = false;
    uint16_t callOrder = 0;
    int16_t screenX = 0;
    int16_t screenY = 0;
    uint32_t templateAddr = 0;
    uint16_t priority = 0;
    uint16_t layer = 480;
};

struct PrStage1CommonLyricsFrameSub8001C864 {
    bool valid = false;
    uint16_t arg0Priority = 0;
    uint8_t submitCount = 0;
    std::array<PrStage1CommonLyricsFrameSubmitSub8001C864,
               kPrStage1CommonLyricsFrameSubmitCapacitySub8001C864>
        submits{};
};
