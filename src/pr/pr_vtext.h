#pragma once

#include <cstdint>

struct PrGameContext;

struct PrVTextTimecode {
    uint16_t minute = 0;
    uint8_t second = 0;
    uint8_t frame = 0;

    constexpr uint32_t Key() const {
        return (static_cast<uint32_t>(minute) << 16) | (static_cast<uint32_t>(second) << 8) | frame;
    }
};

static_assert(sizeof(PrVTextTimecode) == 4);

struct PrVTextEventEntry {
    PrVTextTimecode at{};
    uint16_t duration = 0;
    int16_t textIndex[5]{};

    constexpr int16_t GetTextIndex(int lang) const {
        return (lang >= 0 && lang < 5) ? textIndex[lang] : 0;
    }
};

static_assert(sizeof(PrVTextEventEntry) == 16);

struct PrVTextEventTableDesc {
    uint32_t textTables[5]{};
    uint32_t entries = 0;
    uint32_t entryCount = 0;

    constexpr uint32_t GetTextTable(int lang) const {
        return (lang >= 0 && lang < 5) ? textTables[lang] : 0;
    }
};

static_assert(sizeof(PrVTextEventTableDesc) == 28);

struct PrVText {
    static void Init(PrGameContext& ctx);
    static void Shutdown(PrGameContext& ctx);

    static void DrawString(PrGameContext& ctx, int x, int y, const char* text, uint32_t color, float scale = 1.0f, bool allowSpriteGlyphs = true);
    static float MeasureString(PrGameContext& ctx, const char* text, float scale = 1.0f, bool allowSpriteGlyphs = true);
};
