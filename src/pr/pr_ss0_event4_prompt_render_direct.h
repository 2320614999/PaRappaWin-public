#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace PrSS0Event4PromptRenderDirect {

constexpr uint32_t kFn800203D4_Event4Prompt = 0x800203D4u;
constexpr uint32_t kFn8001C550_FastSpriteChain = 0x8001C550u;
constexpr uint32_t kFn8001B590_LocalSprite = 0x8001B590u;
constexpr uint32_t kFn8001B25C_TemplateCopy = 0x8001B25Cu;
constexpr uint32_t kFn8003FA20_FastSpriteSubmit = 0x8003FA20u;
constexpr uint32_t kWorkListBase80087288 = 0x80087288u;
constexpr uint32_t kWorkListStride80087288 = 20u;
constexpr std::size_t kEvent4PromptSpriteCount800203D4 = 3u;

struct Event4PromptRgbSource800203D4 {
    bool known = false;
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
    uint32_t sourceFunction = 0;
    uint32_t sourceAddress = 0;
};

struct Event4PromptInput800203D4 {
    bool requestBound = false;
    bool choiceSourceKnown = false;
    int32_t ctx0 = -1;
    std::array<Event4PromptRgbSource800203D4,
               kEvent4PromptSpriteCount800203D4>
        rgb{};
};

struct Event4PromptSprite800203D4 {
    bool known = false;
    bool selected = false;
    uint32_t templateAddress = 0;
    int16_t screenX = 0;
    int16_t screenY = 0;
    int16_t localX = 0;
    int16_t localY = 0;
    uint32_t attr = 0;
    uint16_t texX = 0;
    uint16_t texY = 0;
    uint16_t width = 0;
    uint16_t height = 0;
    uint16_t clutX = 0;
    uint16_t clutY = 0;
    uint16_t tpage = 0;
    uint8_t u = 0;
    uint8_t v = 0;
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
    bool rawTextureKnown = false;
    bool rawTexture = false;
    bool rgbKnown = false;
    bool word2CommandKnown = false;
    uint8_t word2CommandCode = 0;
    bool packetWord2ColorKnown = false;
    uint32_t packetWord2ColorCode = 0;
    uint32_t callOrder = 0;
    std::array<uint32_t, 4> psxCallChain{{
        kFn8001C550_FastSpriteChain,
        kFn8001B590_LocalSprite,
        kFn8001B25C_TemplateCopy,
        kFn8003FA20_FastSpriteSubmit,
    }};
};

struct Event4PromptDrawList800203D4 {
    bool accepted = false;
    bool complete = false;
    bool requestBound = false;
    bool choiceSourceKnown = false;
    bool rgbSourceKnown = false;
    bool rgbTailUnresolvedWithoutInput = true;
    bool rawTextureOnly = false;
    bool visibleColorIndependentOfRgb = false;
    int32_t ctx0 = -1;
    std::array<Event4PromptSprite800203D4,
               kEvent4PromptSpriteCount800203D4>
        sprites{};
    std::size_t count = 0u;
};

Event4PromptDrawList800203D4 BuildEvent4PromptDrawList800203D4(
    const Event4PromptInput800203D4& input);

}  // namespace PrSS0Event4PromptRenderDirect
