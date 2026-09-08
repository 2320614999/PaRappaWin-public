#include "tim_decoder.h"
#include "tim_data.h"
#include <cstddef>
#include <cstdio>

static bool ExpectColor(const char* name, uint32_t actual, uint32_t expected) {
    if (actual == expected) {
        return true;
    }
    std::fprintf(stderr, "%s: got 0x%08X, expected 0x%08X\n",
                 name, static_cast<unsigned int>(actual), static_cast<unsigned int>(expected));
    return false;
}

static bool ExpectSize(const char* name, std::size_t actual, std::size_t expected) {
    if (actual == expected) {
        return true;
    }
    std::fprintf(stderr, "%s: got %zu, expected %zu\n", name, actual, expected);
    return false;
}

static bool ExpectPixel(const char* name,
                        const std::vector<uint32_t>& pixels,
                        std::size_t index,
                        uint32_t expected) {
    if (index >= pixels.size()) {
        std::fprintf(stderr, "%s: pixel %zu missing from size %zu\n", name, index, pixels.size());
        return false;
    }
    return ExpectColor(name, pixels[index], expected);
}

int main() {
    constexpr uint16_t kZeroStp0 = 0x0000u;
    constexpr uint16_t kZeroStp1 = 0x8000u;
    constexpr uint16_t kColoredStp0 = 0x0C41u;  // RGB5 = (1, 2, 3)
    constexpr uint16_t kColoredStp1 = 0x8C41u;
    constexpr uint32_t kColoredOpaque = 0xFF181008u;
    constexpr uint32_t kColoredAbr0Stp = 0x80181008u;
    constexpr uint32_t kColoredAbr1Stp = 0x00181008u;

    bool ok = true;
    TimImage sonyTim{};
    ok &= TimDecoder::Decode(tim_sony, sizeof(tim_sony), sonyTim);
    ok &= ExpectSize("Sony TIM width", sonyTim.width, 216u);
    ok &= ExpectSize("Sony TIM height", sonyTim.height, 28u);
    ok &= ExpectSize("Sony TIM palette", sonyTim.palette.size(), 16u);

    {
        const std::vector<uint16_t> fadeSource = {0x0000u, 0x7FFFu};
        std::vector<uint32_t> fadeNormal;
        std::vector<uint32_t> fadeAbr1;
        TimDecoder::GenerateFadePalette(
            fadeSource, 29, 30, fadeNormal, false);
        TimDecoder::GenerateFadePalette(
            fadeSource, 29, 30, fadeAbr1, true);
        ok &= ExpectPixel("fade normal transparent zero",
                          fadeNormal, 0u, 0x00000000u);
        ok &= ExpectPixel("fade normal 8001BF38 step29",
                          fadeNormal, 1u, 0xFFEFEFEFu);
        ok &= ExpectPixel("fade ABR1 transparent zero",
                          fadeAbr1, 0u, 0x00000000u);
        ok &= ExpectPixel("fade ABR1 forced STP step29",
                          fadeAbr1, 1u, 0x00EFEFEFu);
    }

    ok &= ExpectColor("zero/STP0",
                      TimDecoder::ConvertABGR1555toPsxAbr1StpRGBA8888(kZeroStp0),
                      0x00000000u);
    ok &= ExpectColor("zero/STP1",
                      TimDecoder::ConvertABGR1555toPsxAbr1StpRGBA8888(kZeroStp1),
                      0x00000000u);
    ok &= ExpectColor("colored/STP0",
                      TimDecoder::ConvertABGR1555toPsxAbr1StpRGBA8888(kColoredStp0),
                      kColoredOpaque);
    ok &= ExpectColor("colored/STP1",
                      TimDecoder::ConvertABGR1555toPsxAbr1StpRGBA8888(kColoredStp1),
                      kColoredAbr1Stp);
    ok &= ExpectColor("ABR0 zero/STP0",
                      TimDecoder::ConvertABGR1555toPsxAbr0StpRGBA8888(kZeroStp0),
                      0x00000000u);
    ok &= ExpectColor("ABR0 zero/STP1",
                      TimDecoder::ConvertABGR1555toPsxAbr0StpRGBA8888(kZeroStp1),
                      0x80000000u);
    ok &= ExpectColor("ABR0 colored/STP0",
                      TimDecoder::ConvertABGR1555toPsxAbr0StpRGBA8888(kColoredStp0),
                      kColoredOpaque);
    ok &= ExpectColor("ABR0 colored/STP1",
                      TimDecoder::ConvertABGR1555toPsxAbr0StpRGBA8888(kColoredStp1),
                      kColoredAbr0Stp);

    TimImage indexed4{};
    indexed4.width = 4;
    indexed4.height = 1;
    indexed4.bpp = 4;
    indexed4.palette.assign(16u, 0u);
    indexed4.palette[0] = kZeroStp0;
    indexed4.palette[1] = kZeroStp1;
    indexed4.palette[2] = kColoredStp0;
    indexed4.palette[3] = kColoredStp1;
    indexed4.pixels = {0x10u, 0x32u};
    TimDecoder::ApplyPalette(indexed4);

    ok &= ExpectSize("4bpp rgba size", indexed4.rgba.size(), 4u);
    ok &= ExpectSize("4bpp ABR1/STP size", indexed4.rgbaPsxAbr1Stp.size(), 4u);
    ok &= ExpectPixel("4bpp normal zero/STP0", indexed4.rgba, 0u, 0x00000000u);
    ok &= ExpectPixel("4bpp normal zero/STP1", indexed4.rgba, 1u, 0xFF000000u);
    ok &= ExpectPixel("4bpp normal colored/STP0", indexed4.rgba, 2u, kColoredOpaque);
    ok &= ExpectPixel("4bpp normal colored/STP1", indexed4.rgba, 3u, kColoredOpaque);
    ok &= ExpectPixel("4bpp ABR1 zero/STP0", indexed4.rgbaPsxAbr1Stp, 0u, 0x00000000u);
    ok &= ExpectPixel("4bpp ABR1 zero/STP1", indexed4.rgbaPsxAbr1Stp, 1u, 0x00000000u);
    ok &= ExpectPixel("4bpp ABR1 colored/STP0", indexed4.rgbaPsxAbr1Stp, 2u, kColoredOpaque);
    ok &= ExpectPixel("4bpp ABR1 colored/STP1", indexed4.rgbaPsxAbr1Stp, 3u, kColoredAbr1Stp);

    TimImage indexed8{};
    indexed8.width = 1;
    indexed8.height = 1;
    indexed8.bpp = 8;
    indexed8.palette.assign(256u, 0u);
    indexed8.palette[7] = kColoredStp1;
    indexed8.pixels = {7u};
    TimDecoder::ApplyPalette(indexed8);
    ok &= ExpectPixel("8bpp normal colored/STP1", indexed8.rgba, 0u, kColoredOpaque);
    ok &= ExpectPixel("8bpp ABR1 colored/STP1", indexed8.rgbaPsxAbr1Stp, 0u, kColoredAbr1Stp);

    TimImage direct16{};
    direct16.width = 1;
    direct16.height = 1;
    direct16.bpp = 16;
    direct16.pixels = {0x41u, 0x8Cu};
    TimDecoder::ApplyPalette(direct16);
    ok &= ExpectPixel("16bpp normal colored/STP1", direct16.rgba, 0u, kColoredOpaque);
    ok &= ExpectPixel("16bpp ABR1 colored/STP1", direct16.rgbaPsxAbr1Stp, 0u, kColoredAbr1Stp);

    TimImage direct24{};
    direct24.width = 1;
    direct24.height = 1;
    direct24.bpp = 24;
    direct24.pixels = {0u, 0u, 0u};
    direct24.rgbaPsxAbr1Stp = {0xFFFFFFFFu};
    TimDecoder::ApplyPalette(direct24);
    ok &= ExpectSize("24bpp ABR1/STP authority", direct24.rgbaPsxAbr1Stp.size(), 0u);

    if (!ok) {
        return 1;
    }

    std::printf("test_tim: ok\n");
    return 0;
}
