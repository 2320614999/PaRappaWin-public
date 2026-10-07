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
    // STP-preserving pixels for PSX ABR1 with Src=ONE/Dst=INV_SRC_ALPHA.
    std::vector<uint32_t> rgbaPsxAbr1Stp;
};

class TimDecoder {
public:
    static bool Decode(const uint8_t* data, size_t size, TimImage& out);
    static uint32_t ConvertABGR1555toRGBA8888(uint16_t color);
    static uint32_t ConvertABGR1555toPsxAbr0StpRGBA8888(uint16_t color);
    static uint32_t ConvertABGR1555toPsxAbr1StpRGBA8888(uint16_t color);
    static void ApplyPalette(TimImage& img);
    // -1: ordinary texture; 0/1: preserve per-pixel STP for the selected ABR.
    static void ApplyPalette(TimImage& img, int paletteRow, int psxAbr = -1);
    static int PickBestGrayscalePaletteRow(const TimImage& img);
    static void GenerateFadePalette(const std::vector<uint16_t>& basePalette, 
                                     int fadeStep, int maxSteps,
                                     std::vector<uint32_t>& outRGBA,
                                     bool psxAbr1Stp = false);
};
