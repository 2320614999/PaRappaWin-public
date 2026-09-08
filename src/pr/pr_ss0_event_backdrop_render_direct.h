#pragma once

#include <cstdint>

namespace PrSS0EventBackdropRenderDirect {

constexpr uint32_t kFn8001D74C_EventBackdrop = 0x8001D74Cu;
constexpr uint32_t kFn8001B590_FastSpriteSubmit = 0x8001B590u;
constexpr uint32_t kFn8001B25C_FastSpriteLocal = 0x8001B25Cu;
constexpr uint32_t kEventBackdropSpriteCapacity8001D74C = 84u;

struct EventBackdropSpriteTemplate8001B25C {
    bool known = false;
    uint32_t psxAddress = 0;
    uint32_t attr = 0;
    uint16_t texX = 0;
    uint16_t texY = 0;
    uint16_t width = 0;
    uint16_t height = 0;
    uint16_t clutX = 0;
    uint16_t clutY = 0;
};

struct EventBackdropSpriteCommand8001D74C {
    bool known = false;
    int16_t x = 0;
    int16_t y = 0;
    uint16_t priority = 0;
    uint32_t callOrder = 0;
    EventBackdropSpriteTemplate8001B25C sprite{};
};

struct EventBackdropDrawList8001D74C {
    bool sourceKnown = false;
    bool accepted = false;
    bool complete = false;
    bool rawTextureOnly = false;
    EventBackdropSpriteCommand8001D74C
        commands[kEventBackdropSpriteCapacity8001D74C]{};
    uint32_t count = 0;
    bool truncated = false;
};

EventBackdropSpriteTemplate8001B25C
ResolveEventBackdropSpriteTemplate8001B25C(uint32_t psxAddress);

EventBackdropDrawList8001D74C BuildEventBackdropDrawList8001D74C(
    uint16_t priority);

}  // namespace PrSS0EventBackdropRenderDirect
