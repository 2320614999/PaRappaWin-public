#include "pr_stage2_vram_device.h"
#include "tim_decoder.h"
#include <stdexcept>

namespace PrStage2VramDevice {
Device::Device() : words_(WordCount), known_(WordCount) {}
bool Device::SeedTim(const uint8_t* raw, std::size_t size) {
    TimImage image;
    if (!raw || !TimDecoder::Decode(raw, size, image) ||
        (image.bpp != 4u && image.bpp != 8u) || image.palette.empty()) {
        return false;
    }
    const uint32_t wordsPerRow = image.bpp == 4u ? image.width / 4u : image.width / 2u;
    if (!wordsPerRow || image.orgX < 0 || image.orgY < 0 ||
        static_cast<uint32_t>(image.orgX) + wordsPerRow > 1024u ||
        static_cast<uint32_t>(image.orgY) + image.height > 512u ||
        static_cast<uint32_t>(image.clutX) + image.clutW > 1024u ||
        static_cast<uint32_t>(image.clutY) + image.clutH > 512u ||
        image.pixels.size() != static_cast<std::size_t>(wordsPerRow) * image.height * 2u) {
        return false;
    }
    auto write = [&](uint32_t x, uint32_t y, uint16_t value) {
        const std::size_t index = static_cast<std::size_t>(y) * 1024u + x;
        words_[index] = value;
        if (!known_[index]) { known_[index] = 1u; ++knownWords_; }
        ++writtenWords_;
    };
    for (uint32_t row = 0; row < image.clutH; ++row)
        for (uint32_t col = 0; col < image.clutW; ++col)
            write(static_cast<uint32_t>(image.clutX) + col,
                  static_cast<uint32_t>(image.clutY) + row,
                  image.palette[static_cast<std::size_t>(row) * image.clutW + col]);
    for (uint32_t row = 0; row < image.height; ++row) {
        for (uint32_t col = 0; col < wordsPerRow; ++col) {
            const std::size_t offset =
                (static_cast<std::size_t>(row) * wordsPerRow + col) * 2u;
            const uint16_t value = static_cast<uint16_t>(image.pixels[offset]) |
                static_cast<uint16_t>(image.pixels[offset + 1u]) << 8u;
            write(static_cast<uint32_t>(image.orgX) + col,
                  static_cast<uint32_t>(image.orgY) + row, value);
        }
    }
    return true;
}
int32_t Device::UploadImageWords(PrStage2LifecycleDirect::Services& memory,
                          PrStage2LifecycleDirect::ImageRect r,uint32_t source) {
    // This host backend supports in-bounds image transfers. Out-of-range
    // GPU commands need their actual hardware semantics; never clip them or
    // return success here. The upper original TIM functions remain unchecked.
    if (r.width == 0u || r.height == 0u || r.x >= 1024u || r.y >= 512u ||
        r.width > 1024u-r.x || r.height > 512u-r.y)
        throw std::runtime_error("S2 host VRAM upload requires GPU rectangle handling");
    const uint32_t area = uint32_t(r.width)*r.height;
    for (uint32_t i = 0; i < area; i += 2u) {
        // The original CPU/DMA transfer consumes ceil(area/2) source words.
        const uint32_t packed = memory.Read32(source+2u*i);
        for (uint32_t half = 0; half < 2u && i+half < area; ++half) {
            const uint32_t position = i+half;
            const uint32_t index = (r.y+position/r.width)*1024u+r.x+position%r.width;
            words_[index] = static_cast<uint16_t>(packed>>(16u*half));
            if (!known_[index]) { known_[index] = 1u; ++knownWords_; }
        }
    }
    ++uploads_; writtenWords_ += area;
    return 0;
}
int32_t Device::DrawSync(uint32_t mode) {
    if (mode != 0u) throw std::runtime_error("S2 host VRAM requires an explicit nonblocking queue adapter");
    // All preceding copies have completed in this synchronous host backend.
    ++syncs_;
    return 0;
}
}
