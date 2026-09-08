#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace PrSS0CardInfoRenderDirect {

constexpr uint32_t kFn80020BE4_CardInfoRender = 0x80020BE4u;
constexpr uint32_t kFn8001B744_CardInfoTextGlyph = 0x8001B744u;
constexpr uint32_t kCardInfoArgAddress80049244 = 0x80049244u;

constexpr std::size_t kCardInfoLanguageCount80020BE4 = 5u;
constexpr std::size_t kCardInfoMarkerCount = 57u;
constexpr std::size_t kCardInfoMarkerCount80020BE4 = kCardInfoMarkerCount;
constexpr std::size_t kCardInfoLowRamMarkerIndex80020BE4 = 56u;
constexpr std::size_t kCardInfoPreviewStorageCapacity80020BE4 = 96u;
constexpr std::size_t kCardInfoPreviewMaxByteCount80020BE4 = 95u;
constexpr std::size_t kCardInfoStaticSourceCallCount80020BE4 = 64u;
constexpr std::size_t kCardInfoStaticSpriteCommandCountWithoutLowRam80020BE4 =
    63u;
constexpr std::size_t kCardInfoStaticSpriteCommandCapacity80020BE4 = 64u;
constexpr std::size_t kCardInfoSpriteCommandCapacity80020BE4 =
    kCardInfoPreviewMaxByteCount80020BE4 +
    kCardInfoStaticSpriteCommandCapacity80020BE4;

constexpr uint32_t kCardInfoRawTextureAttr80020BE4 = 0x50000040u;
constexpr uint16_t kCardInfoSpritePriority80020BE4 = 0u;
constexpr int16_t kCardInfoPreviewX80020BE4 = 117;
constexpr int16_t kCardInfoPreviewY80020BE4 = 64;
constexpr uint16_t kCardInfoPreviewClutX80020BE4 = 0x0100u;
constexpr uint16_t kCardInfoPreviewClutY80020BE4 = 0x01E2u;

constexpr uint32_t kCardInfoTopIconOffGpSlotAddress8006EB18 = 0x8006EB18u;
constexpr uint32_t kCardInfoTopIconOnGpSlotAddress8006EB1C = 0x8006EB1Cu;

static_assert(kCardInfoPreviewMaxByteCount80020BE4 + 1u ==
                  kCardInfoPreviewStorageCapacity80020BE4,
              "80020BE4 preview storage must retain one NUL byte");
static_assert(kCardInfoSpriteCommandCapacity80020BE4 == 159u,
              "95 preview glyph packets plus up to 64 static packets");

struct CardInfoSpriteDescriptor8001B25C {
    bool known = false;
    uint32_t attr = 0u;
    uint16_t texX = 0u;
    uint16_t texY = 0u;
    uint16_t width = 0u;
    uint16_t height = 0u;
    uint16_t clutX = 0u;
    uint16_t clutY = 0u;
};

enum class CardInfoSpriteRole80020BE4 : uint8_t {
    Unknown = 0u,
    PreviewGlyph,
    LanguageTitle,
    FixedTitle,
    Marker,
    LowerTopIcon,
    LowerTopLanguageText,
    LowerBottomLanguageText,
    LowerMiddleFixed,
    LowerFinalFixed,
};

struct CardInfoSpriteCommand80020BE4 {
    bool known = false;
    uint32_t sourceTemplateAddress = 0u;
    int16_t x = 0;
    int16_t y = 0;
    uint16_t priority = 0u;
    CardInfoSpriteDescriptor8001B25C descriptor{};

    bool rawTexture = false;
    bool textureCoordinatesResolved = false;
    uint32_t callOrder = 0u;
    uint32_t sourceCallOrder = 0u;
    uint16_t tpage = 0u;
    uint8_t u = 0u;
    uint8_t v = 0u;
    uint8_t glyphCode = 0u;

    CardInfoSpriteRole80020BE4 role =
        CardInfoSpriteRole80020BE4::Unknown;
    int16_t markerIndex = -1;
    bool markerSelected = false;
    bool markerPaletteAdjusted = false;
};

struct CardInfoSpriteInput80020BE4 {
    bool requestBound = false;
    uint32_t argAddress = 0u;
    bool languageKnown = false;
    int32_t languageIndex = -1;
    bool topFlagKnown = false;
    uint32_t topFlag = 0u;
    bool selectedMarkerKnown = false;
    int32_t selectedMarker = -1;
    bool lowerModeKnown = false;
    int32_t lowerMode = 0;
    bool topIconTemplateSlotsKnown = false;
    uint32_t topIconOffTemplate = 0u;
    uint32_t topIconOnTemplate = 0u;

    bool previewBytesKnown = false;
    const uint8_t* previewBytes = nullptr;
    std::size_t previewByteCount = 0u;

    // The marker-table entry at 8005346C contains address 0. On PSX this is
    // the readable KUSEG alias of physical RAM 0, not a C++ null pointer.
    bool lowRamDescriptorKnown = false;
    CardInfoSpriteDescriptor8001B25C lowRamDescriptorAtPhysicalZero{};
};

struct CardInfoSpriteDrawList80020BE4 {
    bool accepted = false;
    bool complete = false;
    bool rawTextureOnly = false;
    std::size_t count = 0u;
    std::size_t capacity = kCardInfoSpriteCommandCapacity80020BE4;
    bool truncated = false;
    bool failure = false;
    bool inputFailure = false;
    bool previewFailure = false;
    bool descriptorFailure = false;
    bool rawTextureFailure = false;
    bool callOrderFailure = false;
    bool capacityFailure = false;
    std::array<CardInfoSpriteCommand80020BE4,
               kCardInfoSpriteCommandCapacity80020BE4>
        commands{};

    bool sourceKnown = false;
    std::size_t sourceCallCount = 0u;
    std::size_t sourceSpriteCallCount = 0u;
    std::size_t skippedSourceCallCount = 0u;
    std::size_t previewByteCount = 0u;
    std::size_t previewGlyphSpriteCount = 0u;
    std::size_t glyphSpriteCount = 0u;
    std::size_t previewNoPacketGlyphCount = 0u;
    std::size_t markerSourceCallCount = 0u;
    std::size_t markerCommandCount = 0u;
    bool lowRamMarkerPointerZeroKnown = false;
    bool lowRamMarkerDescriptorKnown = false;
    bool lowRamMarkerPacketOutcomeKnown = false;
    bool lowRamMarkerPacketEmitted = false;
    bool lowRamMarkerSelected = false;
    std::size_t lowRamMarkerIndex = kCardInfoLowRamMarkerIndex80020BE4;
    uint32_t lowRamMarkerSourceCallOrder = 0u;
};

CardInfoSpriteDrawList80020BE4 BuildCardInfoSpriteDrawList80020BE4(
    const CardInfoSpriteInput80020BE4& input);

}  // namespace PrSS0CardInfoRenderDirect
