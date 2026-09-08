#include "pr_ss0_hiscore_render_direct.h"

#include "pr_psx_text_glyph_metrics_direct.h"

namespace PrSS0HiScoreRenderDirect {
namespace {

struct PositionedTemplate80021594 {
    uint32_t address;
    HiScoreRawTextureTemplate8001B25C descriptor;
    int16_t x;
    int16_t y;
};

constexpr HiScoreRawTextureTemplate8001B25C kEmptyCellDescriptor80052F00 = {
    0x50000040u, 0x0280u, 0x003Cu, 0x0050u,
    0x0014u, 0x03F0u, 0x0041u,
};

constexpr HiScoreRawTextureTemplate8001B25C kNonEmptyCellDescriptor80052EE0 = {
    0x50000040u, 0x0280u, 0x003Cu, 0x0050u,
    0x0014u, 0x03F0u, 0x003Fu,
};

constexpr PositionedTemplate80021594 kLanguageTitles80053F84[] = {
    {0x80052E60u, {0x50000040u, 0x0280u, 0x0068u, 0x0064u, 0x0011u, 0x03F0u, 0x0032u}, 52, 34},
    {0x80052E80u, {0x50000040u, 0x0299u, 0x0068u, 0x006Cu, 0x0011u, 0x03F0u, 0x0032u}, 49, 34},
    {0x80052E70u, {0x50000040u, 0x0280u, 0x0079u, 0x0078u, 0x0011u, 0x03F0u, 0x0032u}, 42, 34},
    {0x80052E90u, {0x50000040u, 0x029Eu, 0x0079u, 0x0050u, 0x0011u, 0x03F0u, 0x0032u}, 61, 34},
    {0x80052EA0u, {0x50000040u, 0x0280u, 0x008Au, 0x007Cu, 0x000Eu, 0x03F0u, 0x0032u}, 40, 34},
};

constexpr PositionedTemplate80021594 kFixedSprites80021594[] = {
    {0x80052E50u, {0x50000040u, 0x0280u, 0x0050u, 0x0084u, 0x0017u, 0x03F0u, 0x0048u}, 37, 31},
    {0x80052EB0u, {0x50000040u, 0x0294u, 0x003Cu, 0x0014u, 0x000Cu, 0x03F0u, 0x003Cu}, 84, 56},
    {0x80052EC0u, {0x50000040u, 0x0299u, 0x003Cu, 0x0014u, 0x000Cu, 0x03F0u, 0x003Du}, 163, 56},
    {0x80052ED0u, {0x50000040u, 0x029Eu, 0x003Cu, 0x0014u, 0x000Cu, 0x03F0u, 0x003Eu}, 242, 56},
    {0x80052F10u, {0x50000040u, 0x02A3u, 0x003Cu, 0x0010u, 0x0010u, 0x03F0u, 0x0042u}, 37, 69},
    {0x80052F20u, {0x50000040u, 0x02A7u, 0x003Cu, 0x0010u, 0x0010u, 0x03F0u, 0x0043u}, 37, 87},
    {0x80052F30u, {0x50000040u, 0x02ABu, 0x003Cu, 0x0010u, 0x0010u, 0x03F0u, 0x0044u}, 37, 105},
    {0x80052F40u, {0x50000040u, 0x02AFu, 0x003Cu, 0x0010u, 0x0010u, 0x03F0u, 0x0045u}, 37, 123},
    {0x80052F50u, {0x50000040u, 0x02B3u, 0x003Cu, 0x0010u, 0x0010u, 0x03F0u, 0x0046u}, 37, 141},
    {0x80052F60u, {0x50000040u, 0x02B7u, 0x003Cu, 0x0010u, 0x0010u, 0x03F0u, 0x0047u}, 37, 159},
};

constexpr PositionedTemplate80021594 kExitIcons80021594[] = {
    {0x80050AC0u, {0x50000040u, 0x0180u, 0x0000u, 0x0040u, 0x0025u, 0x03C0u, 0x0000u}, 231, 179},
    {0x80050AD0u, {0x50000040u, 0x0180u, 0x0000u, 0x0040u, 0x0025u, 0x03C0u, 0x0001u}, 231, 179},
};

constexpr PositionedTemplate80021594 kExitLabelsOff80053004[] = {
    {0x800509E0u, {0x50000040u, 0x0140u, 0x0100u, 0x001Cu, 0x000Bu, 0x03C0u, 0x0006u}, 242, 191},
    {0x80050A40u, {0x50000040u, 0x014Fu, 0x0100u, 0x0020u, 0x0008u, 0x03C0u, 0x0006u}, 238, 193},
    {0x80050A10u, {0x50000040u, 0x0147u, 0x0100u, 0x0020u, 0x0008u, 0x03C0u, 0x0006u}, 238, 193},
    {0x80050A70u, {0x50000040u, 0x0157u, 0x0100u, 0x001Cu, 0x000Cu, 0x03C0u, 0x0006u}, 241, 190},
    {0x80050AA0u, {0x50000040u, 0x015Eu, 0x0100u, 0x0020u, 0x000Au, 0x03C0u, 0x0006u}, 239, 191},
};

constexpr PositionedTemplate80021594 kExitLabelsOn80053008[] = {
    {0x800509F0u, {0x50000040u, 0x0140u, 0x0100u, 0x001Cu, 0x000Bu, 0x03C0u, 0x0007u}, 242, 191},
    {0x80050A50u, {0x50000040u, 0x014Fu, 0x0100u, 0x0020u, 0x0008u, 0x03C0u, 0x0007u}, 238, 193},
    {0x80050A20u, {0x50000040u, 0x0147u, 0x0100u, 0x0020u, 0x0008u, 0x03C0u, 0x0007u}, 238, 193},
    {0x80050A80u, {0x50000040u, 0x0157u, 0x0100u, 0x001Cu, 0x000Cu, 0x03C0u, 0x0007u}, 241, 190},
    {0x80050AB0u, {0x50000040u, 0x015Eu, 0x0100u, 0x0020u, 0x000Au, 0x03C0u, 0x0007u}, 239, 191},
};

constexpr PositionedTemplate80021594 kBars80021594[] = {
    {0x800509B0u, {0x50000040u, 0x0140u, 0x0000u, 0x0030u, 0x0011u, 0x03C0u, 0x0003u}, 238, 188},
    {0x800509C0u, {0x50000040u, 0x0140u, 0x0000u, 0x0030u, 0x0011u, 0x03C0u, 0x0004u}, 238, 188},
};

static_assert(sizeof(kLanguageTitles80053F84) /
                      sizeof(kLanguageTitles80053F84[0]) ==
                  kHiScoreLanguageCount80021594,
              "five language titles are required");
static_assert(sizeof(kFixedSprites80021594) /
                      sizeof(kFixedSprites80021594[0]) ==
                  10u,
              "title panel, columns, and row bands require ten sprites");
static_assert(sizeof(kExitLabelsOff80053004) /
                      sizeof(kExitLabelsOff80053004[0]) ==
                  kHiScoreLanguageCount80021594,
              "five off labels are required");
static_assert(sizeof(kExitLabelsOn80053008) /
                      sizeof(kExitLabelsOn80053008[0]) ==
                  kHiScoreLanguageCount80021594,
              "five on labels are required");

int16_t ReadS16LE80021594(const uint8_t* bytes) {
    const uint16_t value = static_cast<uint16_t>(bytes[0]) |
                           static_cast<uint16_t>(bytes[1]) << 8u;
    return static_cast<int16_t>(value);
}

uint32_t ReadU32LE80021594(const uint8_t* bytes) {
    return static_cast<uint32_t>(bytes[0]) |
           static_cast<uint32_t>(bytes[1]) << 8u |
           static_cast<uint32_t>(bytes[2]) << 16u |
           static_cast<uint32_t>(bytes[3]) << 24u;
}

bool AppendSprite80021594(
    HiScoreTableDrawList80021594& out,
    uint32_t address,
    const HiScoreRawTextureTemplate8001B25C& descriptor,
    int16_t x,
    int16_t y) {
    if (out.count >= out.commands.size() || address == 0u ||
        descriptor.attr != kHiScoreRawTextureAttr80021594 ||
        descriptor.width == 0u || descriptor.height == 0u) {
        out.truncated = out.count >= out.commands.size();
        return false;
    }

    HiScoreSpriteCommand80021594 command{};
    command.known = true;
    command.rawTexture = true;
    command.templateAddress = address;
    command.descriptor = descriptor;
    command.x = x;
    command.y = y;
    command.priority = kHiScoreSpritePriority80021594;
    command.callOrder = static_cast<uint32_t>(out.count);
    out.commands[out.count++] = command;
    return true;
}

bool AppendPositionedSprite80021594(
    HiScoreTableDrawList80021594& out,
    const PositionedTemplate80021594& sprite) {
    return AppendSprite80021594(
        out, sprite.address, sprite.descriptor, sprite.x, sprite.y);
}

bool AppendGlyph8001B744(
    HiScoreTableDrawList80021594& out,
    uint8_t glyphCode,
    int16_t textX,
    int16_t textY,
    int32_t& penX) {
    PrPsxTextGlyphMetricsDirect::GlyphMetricRaw8004945C raw{};
    if (!PrPsxTextGlyphMetricsDirect::TryCopyGlyphMetricRaw8004945C(
            glyphCode, raw)) {
        return false;
    }

    const int16_t textureX = ReadS16LE80021594(raw.data());
    const int16_t textureY = ReadS16LE80021594(raw.data() + 2u);
    const uint16_t width = raw[4u];
    const uint16_t height = raw[5u];
    const int16_t xOffset = ReadS16LE80021594(raw.data() + 6u);
    const int32_t penBefore = penX;
    penX += static_cast<int32_t>(width);

    // 8001B744 advances the pen even when the metric has no texture record.
    if (textureY < 0) {
        return true;
    }
    if (width == 0u || height == 0u || out.count >= out.commands.size()) {
        out.truncated = out.count >= out.commands.size();
        return false;
    }

    const int32_t roundedUBase =
        textureX <= 0 ? static_cast<int32_t>(textureX) + 3584
                      : static_cast<int32_t>(textureX) + 3583;
    const uint16_t uvWord = static_cast<uint16_t>(roundedUBase);
    const uint16_t helperA3 = static_cast<uint16_t>(
        (uvWord & 0xFF00u) >> 2u);
    const uint16_t helperA4 = static_cast<uint16_t>(
        (static_cast<uint16_t>(static_cast<int32_t>(textureY) + 256) &
         0xFF00u));

    HiScoreSpriteCommand80021594 command{};
    command.known = true;
    command.rawTexture = true;
    command.textureCoordinatesResolved = true;
    command.templateAddress = kFn8001B744_HiScoreTextGlyph;
    command.descriptor.attr = kHiScoreRawTextureAttr80021594;
    command.descriptor.width = width;
    command.descriptor.height = height;
    command.descriptor.clutX = 0x0100u;
    command.descriptor.clutY = 0x01E3u;
    command.x = static_cast<int16_t>(
        static_cast<int32_t>(textX) + penBefore + xOffset);
    command.y = static_cast<int16_t>(
        static_cast<int32_t>(textY) +
        (glyphCode >= 192u && glyphCode <= 222u ? -3 : 0));
    command.priority = kHiScoreSpritePriority80021594;
    command.callOrder = static_cast<uint32_t>(out.count);
    command.tpage = static_cast<uint16_t>(
        0x20u |
        ((helperA4 & 0x0100u) >> 4u) |
        ((helperA3 & 0x03FFu) >> 6u) |
        (4u * (helperA4 & 0x0200u)));
    command.u = static_cast<uint8_t>(uvWord);
    command.v = static_cast<uint8_t>(textureY);
    command.glyphCode = glyphCode;
    out.commands[out.count++] = command;
    ++out.glyphSpriteCount;
    return true;
}

}  // namespace

HiScoreTableDrawList80021594 BuildHiScoreTableDrawList80021594(
    const HiScoreEmptyTableInput80021594& input) {
    HiScoreTableDrawList80021594 rejected{};

    if (!input.requestBound80019284 ||
        input.tableBytes80019284 == nullptr ||
        input.tableByteCount80019284 != kHiScoreTableSize80019284 ||
        !input.languageKnown || input.language < 0 ||
        input.language >= static_cast<int32_t>(kHiScoreLanguageCount80021594) ||
        !input.translatedLiveExitIconStateKnown ||
        !input.requestBoundExitLabelStateKnown) {
        return rejected;
    }

    const uint8_t* table = input.tableBytes80019284;
    if (ReadS16LE80021594(table + kHiScoreRowsOffset80021594) !=
            static_cast<int16_t>(kHiScoreRowCount80021594) ||
        ReadS16LE80021594(table + kHiScoreColsOffset80021594) !=
            static_cast<int16_t>(kHiScoreColumnCount80021594) ||
        ReadU32LE80021594(table + kHiScoreExitLabelStateOffset80021594) !=
            static_cast<uint32_t>(input.requestBoundExitLabelState)) {
        return rejected;
    }

    std::array<std::size_t, kHiScoreCellCount80021594> cellTextLength{};
    std::size_t nonEmptyCellCount = 0u;
    for (std::size_t cell = 0u; cell < kHiScoreCellCount80021594; ++cell) {
        const std::size_t offset = kHiScoreCellBaseOffset80021594 +
                                   cell * kHiScoreCellStride80021594;
        std::size_t length = 0u;
        while (length < kHiScoreCellStride80021594 &&
               table[offset + length] != 0u) {
            ++length;
        }
        if (length == kHiScoreCellStride80021594) {
            rejected.blockedByMalformedCell = true;
            return rejected;
        }
        cellTextLength[cell] = length;
        if (length != 0u) {
            ++nonEmptyCellCount;
        }
    }

    HiScoreTableDrawList80021594 candidate{};
    candidate.sourceKnown = true;
    candidate.requestBound80019284 = true;
    candidate.rawTextureOnly = true;
    candidate.allCellsEmpty = nonEmptyCellCount == 0u;
    candidate.nonEmptyCellCount = nonEmptyCellCount;
    bool ok = true;

    for (std::size_t row = 0u; row < kHiScoreRowCount80021594; ++row) {
        for (std::size_t col = 0u; col < kHiScoreColumnCount80021594; ++col) {
            const std::size_t cell =
                row * kHiScoreColumnCount80021594 + col;
            const std::size_t offset = kHiScoreCellBaseOffset80021594 +
                                       cell * kHiScoreCellStride80021594;
            const int16_t x = static_cast<int16_t>(
                kHiScoreCellStartX80021594 +
                static_cast<int16_t>(col) * kHiScoreCellStepX80021594);
            const int16_t y = static_cast<int16_t>(
                kHiScoreCellStartY80021594 +
                static_cast<int16_t>(row) * kHiScoreCellStepY80021594);
            if (cellTextLength[cell] == 0u) {
                ok = AppendSprite80021594(candidate,
                                          kHiScoreEmptyCellTemplate80052F00,
                                          kEmptyCellDescriptor80052F00,
                                          x,
                                          y) &&
                     ok;
                continue;
            }

            int32_t penX = 0;
            for (std::size_t glyph = 0u;
                 glyph < cellTextLength[cell]; ++glyph) {
                ok = AppendGlyph8001B744(candidate,
                                         table[offset + glyph],
                                         static_cast<int16_t>(x + 2),
                                         static_cast<int16_t>(y + 4),
                                         penX) &&
                     ok;
            }
            ok = AppendSprite80021594(candidate,
                                      kHiScoreNonEmptyCellTemplate80052EE0,
                                      kNonEmptyCellDescriptor80052EE0,
                                      x,
                                      y) &&
                 ok;
        }
    }

    const std::size_t language = static_cast<std::size_t>(input.language);
    ok = AppendPositionedSprite80021594(
             candidate, kLanguageTitles80053F84[language]) &&
         ok;
    for (const auto& sprite : kFixedSprites80021594) {
        ok = AppendPositionedSprite80021594(candidate, sprite) && ok;
    }

    const std::size_t iconState =
        input.translatedLiveExitIconState == 1 ? 1u : 0u;
    // 80021594 tests the full ctx+4 dword only for zero versus nonzero.
    // Keep the raw request/table binding above, then normalize solely for the
    // two fixed label and bar template arrays.
    const std::size_t labelState =
        input.requestBoundExitLabelState != 0 ? 1u : 0u;
    ok = AppendPositionedSprite80021594(
             candidate, kExitIcons80021594[iconState]) &&
         ok;
    ok = AppendPositionedSprite80021594(
             candidate,
             labelState == 0u ? kExitLabelsOff80053004[language]
                              : kExitLabelsOn80053008[language]) &&
         ok;
    ok = AppendPositionedSprite80021594(
             candidate, kBars80021594[labelState]) &&
         ok;

    const std::size_t expectedCount =
        kHiScoreCellCount80021594 + candidate.glyphSpriteCount +
        kHiScoreTableStaticSpriteCommandCount80021594;
    if (!ok || candidate.truncated || !candidate.rawTextureOnly ||
        candidate.count != expectedCount ||
        candidate.count > kHiScoreSpriteCommandCapacity80021594) {
        return {};
    }

    candidate.accepted = true;
    candidate.complete = true;
    return candidate;
}

HiScoreEmptyTableDrawList80021594
BuildHiScoreEmptyTableDrawList80021594(
    const HiScoreEmptyTableInput80021594& input) {
    HiScoreEmptyTableDrawList80021594 rejected{};
    const HiScoreTableDrawList80021594 table =
        BuildHiScoreTableDrawList80021594(input);
    if (!table.accepted || !table.complete) {
        rejected.blockedByNonEmptyCell = table.blockedByMalformedCell;
        return rejected;
    }
    if (!table.allCellsEmpty) {
        rejected.blockedByNonEmptyCell = true;
        return rejected;
    }
    if (!table.rawTextureOnly || table.glyphSpriteCount != 0u ||
        table.count != kHiScoreSpriteCommandCount80021594) {
        return rejected;
    }

    HiScoreEmptyTableDrawList80021594 out{};
    for (std::size_t i = 0u; i < table.count; ++i) {
        out.commands[i] = table.commands[i];
    }
    out.accepted = true;
    out.complete = true;
    out.rawTextureOnly = true;
    out.count = table.count;
    out.sourceKnown = table.sourceKnown;
    out.requestBound80019284 = table.requestBound80019284;
    return out;
}

}  // namespace PrSS0HiScoreRenderDirect
