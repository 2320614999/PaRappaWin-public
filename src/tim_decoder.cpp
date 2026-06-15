#include "tim_decoder.h"
#include <cstring>

static uint32_t ReadU32LE(const uint8_t* p) {
    uint32_t v = 0;
    std::memcpy(&v, p, sizeof(v));
    return v;
}

static uint16_t ReadU16LE(const uint8_t* p) {
    uint16_t v = 0;
    std::memcpy(&v, p, sizeof(v));
    return v;
}

bool TimDecoder::Decode(const uint8_t* data, size_t size, TimImage& out) {
    out = TimImage{};
    if (size < 8) return false;

    // TIM header: magic(4) + flags(4)
    uint32_t magic = ReadU32LE(data);
    uint32_t flags = ReadU32LE(data + 4);

    if (magic != 0x10) return false;  // TIM magic

    uint32_t pmode = flags & 0x7;
    bool hasClut = (flags & 0x8) != 0;

    switch (pmode) {
        case 0: out.bpp = 4; break;
        case 1: out.bpp = 8; break;
        case 2: out.bpp = 16; break;
        case 3: out.bpp = 24; break;
        default: return false;
    }

    size_t offset = 8;

    // CLUT block
    if (hasClut) {
        if (offset + 4 > size) return false;
        const size_t clutBlockStart = offset;
        uint32_t clutSize = ReadU32LE(data + offset);
        offset += 4;

        if (clutSize < 12) return false;
        if (clutBlockStart + (size_t)clutSize > size) return false;

        if (offset + 8 > size) return false;
        uint16_t clutX = ReadU16LE(data + offset);
        uint16_t clutY = ReadU16LE(data + offset + 2);
        uint16_t clutW = ReadU16LE(data + offset + 4);
        uint16_t clutH = ReadU16LE(data + offset + 6);
        if (clutW == 0 || clutH == 0) return false;
        if (clutX >= 1024 || clutY >= 512) return false;
        out.clutX = (int16_t)clutX;
        out.clutY = (int16_t)clutY;
        out.clutW = clutW;
        out.clutH = clutH;
        offset += 8;

        uint32_t numColors = clutW * clutH;
        const size_t paletteBytes = (size_t)clutSize - 12u;
        if (paletteBytes > (size - offset)) return false;
        if ((size_t)numColors * 2u != paletteBytes) return false;

        out.palette.resize(numColors);
        memcpy(out.palette.data(), data + offset, numColors * 2);
        offset = clutBlockStart + (size_t)clutSize;
    }

    // Pixel data block
    if (offset + 4 > size) return false;
    const size_t pixelBlockStart = offset;
    uint32_t pixelSize = ReadU32LE(data + offset);
    offset += 4;

    if (pixelSize < 12) return false;
    if (pixelBlockStart + (size_t)pixelSize > size) return false;

    if (offset + 8 > size) return false;
    uint16_t pixX = ReadU16LE(data + offset);
    uint16_t pixY = ReadU16LE(data + offset + 2);
    uint16_t pixW = ReadU16LE(data + offset + 4);
    uint16_t pixH = ReadU16LE(data + offset + 6);
    if (pixW == 0 || pixH == 0) return false;
    if (pixX >= 1024 || pixY >= 512) return false;
    out.orgX = (int16_t)pixX;
    out.orgY = (int16_t)pixY;
    offset += 8;

    // Calculate actual dimensions based on bpp
    switch (out.bpp) {
        case 4:  out.width = pixW * 4; break;
        case 8:  out.width = pixW * 2; break;
        case 16: out.width = pixW; break;
        case 24: out.width = pixW * 2 / 3; break;
    }
    out.height = pixH;

    uint32_t dataSize = pixelSize - 12;
    if ((size_t)dataSize != (size_t)pixW * (size_t)pixH * 2u) return false;
    if (offset + (size_t)dataSize > size) return false;
    out.pixels.resize(dataSize);
    memcpy(out.pixels.data(), data + offset, (size_t)dataSize);

    return true;
}

uint32_t TimDecoder::ConvertABGR1555toRGBA8888(uint16_t color) {
    uint8_t r = (color & 0x1F) << 3;
    uint8_t g = ((color >> 5) & 0x1F) << 3;
    uint8_t b = ((color >> 10) & 0x1F) << 3;
    const uint16_t rgb = (uint16_t)(color & 0x7FFF);

    // PSX STP (Semi-Transparency Processing) bit handling:
    //   rgb==0, STP=0 → fully transparent (color key)
    //   rgb==0, STP=1 → opaque black (drawn as visible black)
    //   rgb!=0, STP=0 → opaque
    //   rgb!=0, STP=1 → opaque (STP only affects blending when primitive enables it)
    // Semi-transparency is a per-primitive render state, not per-pixel in texture.
    uint8_t a;
    if (rgb == 0 && !(color & 0x8000)) {
        a = 0;    // transparent black (color key)
    } else {
        a = 255;  // all other pixels are opaque in the texture
    }

    // Expand 5-bit to 8-bit properly
    r |= (r >> 5);
    g |= (g >> 5);
    b |= (b >> 5);

    return (a << 24) | (b << 16) | (g << 8) | r;
}

void TimDecoder::ApplyPalette(TimImage& img) {
    ApplyPalette(img, 0);
}

void TimDecoder::ApplyPalette(TimImage& img, int paletteRow) {
    if (img.bpp == 16 || img.bpp == 24) {
        // Direct color - no palette needed
        img.rgba.resize(img.width * img.height);
        if (img.bpp == 16) {
            for (size_t i = 0; i < img.width * img.height; i++) {
                uint16_t color = *(uint16_t*)(img.pixels.data() + i * 2);
                img.rgba[i] = ConvertABGR1555toRGBA8888(color);
            }
        }
        return;
    }

    img.rgba.resize(img.width * img.height);

    const size_t rowSize = (img.bpp == 4) ? 16u : (img.bpp == 8 ? 256u : 0u);
    size_t rowOffset = 0u;
    if (rowSize != 0u && !img.palette.empty()) {
        const size_t rows = img.palette.size() / rowSize;
        int pr = paletteRow;
        if (rows == 0u) {
            pr = 0;
        } else {
            if (pr < 0) pr = 0;
            if ((size_t)pr >= rows) pr = (int)rows - 1;
        }
        rowOffset = (size_t)pr * rowSize;
    }

    if (img.bpp == 4) {
        size_t idx = 0;
        for (size_t i = 0; i < img.pixels.size() && idx < img.rgba.size(); i++) {
            uint8_t byte = img.pixels[i];
            uint8_t lo = byte & 0x0F;
            uint8_t hi = (byte >> 4) & 0x0F;

            if (idx < img.rgba.size() && (rowOffset + (size_t)lo) < img.palette.size())
                img.rgba[idx++] = ConvertABGR1555toRGBA8888(img.palette[rowOffset + (size_t)lo]);
            if (idx < img.rgba.size() && (rowOffset + (size_t)hi) < img.palette.size())
                img.rgba[idx++] = ConvertABGR1555toRGBA8888(img.palette[rowOffset + (size_t)hi]);
        }
    } else if (img.bpp == 8) {
        for (size_t i = 0; i < img.pixels.size() && i < img.rgba.size(); i++) {
            uint8_t idx = img.pixels[i];
            if ((rowOffset + (size_t)idx) < img.palette.size())
                img.rgba[i] = ConvertABGR1555toRGBA8888(img.palette[rowOffset + (size_t)idx]);
        }
    }
}

int TimDecoder::PickBestGrayscalePaletteRow(const TimImage& img) {
    if (img.bpp != 4 && img.bpp != 8) {
        return 0;
    }
    const size_t rowSize = (img.bpp == 4) ? 16u : 256u;
    if (rowSize == 0u) {
        return 0;
    }
    const size_t rows = img.palette.size() / rowSize;
    if (rows <= 1u) {
        return 0;
    }

    int bestRow = 0;
    uint64_t bestScore = UINT64_MAX;

    for (size_t r = 0; r < rows; r++) {
        uint64_t chromaSum = 0;
        uint64_t brightSum = 0;
        uint64_t count = 0;

        const size_t base = r * rowSize;
        for (size_t i = 0; i < rowSize && (base + i) < img.palette.size(); i++) {
            const uint16_t c = img.palette[base + i];
            const uint16_t rgb = (uint16_t)(c & 0x7FFF);
            if (rgb == 0) {
                continue;
            }
            const int rr = (int)(c & 0x1F);
            const int gg = (int)((c >> 5) & 0x1F);
            const int bb = (int)((c >> 10) & 0x1F);
            const int dRG = rr > gg ? rr - gg : gg - rr;
            const int dGB = gg > bb ? gg - bb : bb - gg;
            const int dRB = rr > bb ? rr - bb : bb - rr;
            chromaSum += (uint64_t)(dRG + dGB + dRB);
            brightSum += (uint64_t)(rr + gg + bb);
            count++;
        }

        if (count == 0) {
            continue;
        }

        // Prefer low chroma (grayscale) first; then prefer darker palettes.
        const uint64_t score = chromaSum * 1024ull + brightSum;
        if (score < bestScore) {
            bestScore = score;
            bestRow = (int)r;
        }
    }

    return bestRow;
}

void TimDecoder::GenerateFadePalette(const std::vector<uint16_t>& basePalette,
                                      int fadeStep, int maxSteps,
                                      std::vector<uint32_t>& outRGBA) {
    outRGBA.resize(basePalette.size());

    for (size_t i = 0; i < basePalette.size(); i++) {
        uint16_t color = basePalette[i];

        // Extract RGB components (5-bit each)
        int r = (color & 0x1F);
        int g = ((color >> 5) & 0x1F);
        int b = ((color >> 10) & 0x1F);

        // Apply fade: lerp from black to full color
        r = (r * fadeStep) / maxSteps;
        g = (g * fadeStep) / maxSteps;
        b = (b * fadeStep) / maxSteps;

        // Convert to 8-bit
        uint8_t r8 = (r << 3) | (r >> 2);
        uint8_t g8 = (g << 3) | (g >> 2);
        uint8_t b8 = (b << 3) | (b >> 2);
        uint8_t a8 = ((color & 0x7FFF) == 0) ? 0 : 255;

        outRGBA[i] = (a8 << 24) | (b8 << 16) | (g8 << 8) | r8;
    }
}
