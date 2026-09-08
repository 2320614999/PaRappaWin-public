#pragma once

#include <cstddef>
#include <cstdint>

namespace PrSS0CardIoBannerRenderDirect {

constexpr uint32_t kFn80020A3C_CardIoBanner = 0x80020A3Cu;
constexpr uint32_t kCallsite80022314_MainDirectoryCardIoBanner = 0x80022314u;
constexpr int32_t kRemoveCardMessageType80020A3C = 0;
constexpr int32_t kMainDirectoryCardIoMessageType80020A3C = 1;
constexpr int32_t kSavingMessageType80020A3C = 2;
constexpr std::size_t kCardIoBannerCommandCapacity80020A3C = 64u;

enum class CardIoBannerCommandKind80020A3C : uint8_t {
    Sprite = 0,
    SolidRect,
};

struct CardIoBannerSpriteCommand80020A3C {
    bool known = false;
    bool rawTexture = false;
    uint32_t sourceAddress = 0;
    uint8_t charCode = 0;
    int16_t x = 0;
    int16_t y = 0;
    uint16_t width = 0;
    uint16_t height = 0;
    uint16_t priority = 0;
    uint32_t attr = 0;
    uint16_t tpage = 0;
    uint8_t u = 0;
    uint8_t v = 0;
    uint16_t clutX = 0;
    uint16_t clutY = 0;
};

struct CardIoBannerSolidRectCommand80020A3C {
    bool known = false;
    int16_t x = 0;
    int16_t y = 0;
    uint16_t width = 0;
    uint16_t height = 0;
    uint16_t priority = 0;
    uint32_t attr = 0;
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
};

struct CardIoBannerCommand80020A3C {
    bool known = false;
    CardIoBannerCommandKind80020A3C kind =
        CardIoBannerCommandKind80020A3C::Sprite;
    uint32_t callOrder = 0;
    CardIoBannerSpriteCommand80020A3C sprite{};
    CardIoBannerSolidRectCommand80020A3C solidRect{};
};

struct CardIoBannerDrawList80020A3C {
    bool sourceKnown = false;
    bool accepted = false;
    bool complete = false;
    bool rawTextureSpritesOnly = false;
    bool packetRgbKnown = false;
    bool packetRgbRequiredForVisibleOutput = false;
    int32_t messageType = kMainDirectoryCardIoMessageType80020A3C;
    int32_t language = -1;
    const char* text = nullptr;
    uint16_t textWidth = 0;
    uint16_t barWidth = 0;
    int16_t left = 0;
    int16_t right = 0;
    uint32_t glyphSpriteCount = 0;
    uint32_t solidRectCount = 0;
    uint32_t cornerSpriteCount = 0;
    CardIoBannerCommand80020A3C
        commands[kCardIoBannerCommandCapacity80020A3C]{};
    uint32_t count = 0;
    bool truncated = false;
};

CardIoBannerDrawList80020A3C
BuildCardIoBannerDrawList80020A3C(int32_t messageType, int32_t language);

CardIoBannerDrawList80020A3C
BuildMainDirectoryCardIoBannerDrawList80020A3C(int32_t language);

}  // namespace PrSS0CardIoBannerRenderDirect
