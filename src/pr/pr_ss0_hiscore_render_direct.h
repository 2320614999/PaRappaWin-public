#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace PrSS0HiScoreRenderDirect {

constexpr uint32_t kFn80021594_HiScoreRender = 0x80021594u;
constexpr uint32_t kFn80019284_HiScoreTableProducer = 0x80019284u;
constexpr uint32_t kHiScoreTableAddress80049278 = 0x80049278u;

constexpr std::size_t kHiScoreTableSize80019284 = 0x134u;
constexpr std::size_t kHiScoreExitIconStateOffset80021594 = 0x00u;
constexpr std::size_t kHiScoreExitLabelStateOffset80021594 = 0x04u;
constexpr std::size_t kHiScoreRowsOffset80021594 = 0x0Cu;
constexpr std::size_t kHiScoreColsOffset80021594 = 0x0Eu;
constexpr std::size_t kHiScoreCellBaseOffset80021594 = 0x14u;
constexpr std::size_t kHiScoreCellStride80021594 = 0x10u;
constexpr std::size_t kHiScoreRowCount80021594 = 6u;
constexpr std::size_t kHiScoreColumnCount80021594 = 3u;
constexpr std::size_t kHiScoreCellCount80021594 =
    kHiScoreRowCount80021594 * kHiScoreColumnCount80021594;
constexpr std::size_t kHiScoreLanguageCount80021594 = 5u;
constexpr std::size_t kHiScoreSpriteCommandCount80021594 = 32u;
constexpr std::size_t kHiScoreCellTextMaxBytes80021594 = 15u;
constexpr std::size_t kHiScoreTableStaticSpriteCommandCount80021594 = 14u;
constexpr std::size_t kHiScoreSpriteCommandCapacity80021594 =
    kHiScoreCellCount80021594 *
        (kHiScoreCellTextMaxBytes80021594 + 1u) +
    kHiScoreTableStaticSpriteCommandCount80021594;

constexpr uint32_t kHiScoreRawTextureAttr80021594 = 0x50000040u;
constexpr uint16_t kHiScoreSpritePriority80021594 = 0u;

constexpr int16_t kHiScoreCellStartX80021594 = 55;
constexpr int16_t kHiScoreCellStartY80021594 = 68;
constexpr int16_t kHiScoreCellStepX80021594 = 79;
constexpr int16_t kHiScoreCellStepY80021594 = 18;

constexpr uint32_t kHiScoreEmptyCellTemplate80052F00 = 0x80052F00u;
constexpr uint32_t kHiScoreNonEmptyCellTemplate80052EE0 = 0x80052EE0u;
constexpr uint32_t kFn8001B744_HiScoreTextGlyph = 0x8001B744u;
constexpr uint32_t kHiScoreTitlePanelTemplate80052E50 = 0x80052E50u;
constexpr uint32_t kHiScoreExitIconOffTemplate80050AC0 = 0x80050AC0u;
constexpr uint32_t kHiScoreExitIconOnTemplate80050AD0 = 0x80050AD0u;
constexpr uint32_t kHiScoreBarOffTemplate800509B0 = 0x800509B0u;
constexpr uint32_t kHiScoreBarOnTemplate800509C0 = 0x800509C0u;

static_assert(kHiScoreCellBaseOffset80021594 +
                      kHiScoreCellCount80021594 *
                          kHiScoreCellStride80021594 ==
                  kHiScoreTableSize80019284,
              "80019284 table shape must be exactly 0x134 bytes");
static_assert(kHiScoreCellCount80021594 + 1u + 1u + 3u + 6u + 1u +
                      1u + 1u ==
                  kHiScoreSpriteCommandCount80021594,
              "80021594 empty-table path must publish exactly 32 sprites");
static_assert(kHiScoreTableStaticSpriteCommandCount80021594 ==
                  1u + 1u + 3u + 6u + 1u + 1u + 1u,
              "80021594 page tail must publish exactly 14 static sprites");
static_assert(kHiScoreSpriteCommandCapacity80021594 == 302u,
              "80021594 maximum 18x(15 glyphs + icon) + 14 tail is 302");
static_assert(kHiScoreCellStartX80021594 +
                      kHiScoreCellStepX80021594 * 2 ==
                  213,
              "last empty-cell x must match 80021594");
static_assert(kHiScoreCellStartY80021594 +
                      kHiScoreCellStepY80021594 * 5 ==
                  158,
              "last empty-cell y must match 80021594");

struct HiScoreEmptyTableInput80021594 {
    bool requestBound80019284 = false;
    const uint8_t* tableBytes80019284 = nullptr;
    std::size_t tableByteCount80019284 = 0u;

    bool languageKnown = false;
    int32_t language = -1;

    bool translatedLiveExitIconStateKnown = false;
    int32_t translatedLiveExitIconState = -1;
    bool requestBoundExitLabelStateKnown = false;
    int32_t requestBoundExitLabelState = -1;
};

struct HiScoreRawTextureTemplate8001B25C {
    uint32_t attr = 0u;
    uint16_t texX = 0u;
    uint16_t texY = 0u;
    uint16_t width = 0u;
    uint16_t height = 0u;
    uint16_t clutX = 0u;
    uint16_t clutY = 0u;
};

struct HiScoreSpriteCommand80021594 {
    bool known = false;
    bool rawTexture = false;
    bool textureCoordinatesResolved = false;
    uint32_t templateAddress = 0u;
    HiScoreRawTextureTemplate8001B25C descriptor{};
    int16_t x = 0;
    int16_t y = 0;
    uint16_t priority = 0u;
    uint32_t callOrder = 0u;
    uint16_t tpage = 0u;
    uint8_t u = 0u;
    uint8_t v = 0u;
    uint8_t glyphCode = 0u;
};

struct HiScoreTableDrawList80021594 {
    bool accepted = false;
    bool complete = false;
    bool rawTextureOnly = false;
    bool allCellsEmpty = false;
    bool blockedByMalformedCell = false;
    std::size_t nonEmptyCellCount = 0u;
    std::size_t glyphSpriteCount = 0u;
    std::array<HiScoreSpriteCommand80021594,
               kHiScoreSpriteCommandCapacity80021594>
        commands{};
    std::size_t count = 0u;
    bool sourceKnown = false;
    bool requestBound80019284 = false;
    bool truncated = false;
};

struct HiScoreEmptyTableDrawList80021594 {
    bool accepted = false;
    bool complete = false;
    bool rawTextureOnly = false;
    bool blockedByNonEmptyCell = false;
    std::array<HiScoreSpriteCommand80021594,
               kHiScoreSpriteCommandCount80021594>
        commands{};
    std::size_t count = 0u;
    bool sourceKnown = false;
    bool requestBound80019284 = false;
    bool truncated = false;
};

HiScoreEmptyTableDrawList80021594
BuildHiScoreEmptyTableDrawList80021594(
    const HiScoreEmptyTableInput80021594& input);

HiScoreTableDrawList80021594 BuildHiScoreTableDrawList80021594(
    const HiScoreEmptyTableInput80021594& input);

}  // namespace PrSS0HiScoreRenderDirect
