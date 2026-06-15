#pragma once
#include <cstdint>
#include <vector>

struct TimImage {
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t bpp = 0;           // 4, 8, 16, or 24
    int16_t  orgX = 0;      // TIM image origin X (VRAM dest, used for screen positioning)
    int16_t  orgY = 0;      // TIM image origin Y (VRAM dest, used for screen positioning)
    int16_t  clutX = 0;
    int16_t  clutY = 0;
    uint16_t clutW = 0;
    uint16_t clutH = 0;
    std::vector<uint16_t> palette;  // CLUT (16-bit ABGR1555)
    std::vector<uint8_t> pixels;    // Raw pixel indices or direct color
    std::vector<uint32_t> rgba;     // Decoded RGBA8888 pixels
};

class TimDecoder {
public:
    static bool Decode(const uint8_t* data, size_t size, TimImage& out);
    static uint32_t ConvertABGR1555toRGBA8888(uint16_t color);
    static void ApplyPalette(TimImage& img);
    static void ApplyPalette(TimImage& img, int paletteRow);
    static int PickBestGrayscalePaletteRow(const TimImage& img);
    static void GenerateFadePalette(const std::vector<uint16_t>& basePalette,
                                     int fadeStep, int maxSteps,
                                     std::vector<uint32_t>& outRGBA);
};
