#include "pr_native_movie_frame.h"
#include <stdexcept>

namespace PrNativeMovieFrame {
namespace {
size_t RequiredBytes(uint16_t width, uint16_t height) {
    // Explicit allocation bound, not a modeled PS1 device. All established
    // source geometries (including 256x144 and 320x240) fit this interface.
    if (!width || !height || width > 1024u || height > 512u)
        throw std::invalid_argument("Movie frame geometry exceeds the bounded codec interface");
    return size_t((uint32_t(width) + 15u) / 16u) *
           ((uint32_t(height) + 15u) / 16u) * 512u;
}
}
std::vector<uint32_t> Project15bpp(const uint8_t* strips, size_t bytes,
                                 uint16_t width, uint16_t height) {
    const size_t required = RequiredBytes(width, height);
    if (!strips || bytes < required)
        throw std::invalid_argument("Truncated vertical movie pixel buffer");
    std::vector<uint32_t> rgba(size_t(width) * height);
    const size_t stripBytes = size_t((uint32_t(height) + 15u) / 16u) * 512u;
    for (uint32_t y = 0; y < height; ++y) {
        for (uint32_t x = 0; x < width; ++x) {
            const size_t source = (x / 16u) * stripBytes +
                                  (size_t(y) * 16u + x % 16u) * 2u;
            const uint32_t p = uint32_t(strips[source]) |
                               (uint32_t(strips[source + 1u]) << 8u);
            const uint32_t r = p & 31u, g = (p >> 5u) & 31u, b = (p >> 10u) & 31u;
            rgba[size_t(y) * width + x] = ((r << 3u) | (r >> 2u)) |
                (((g << 3u) | (g >> 2u)) << 8u) |
                (((b << 3u) | (b >> 2u)) << 16u) | 0xFF000000u;
        }
    }
    return rgba;
}
Frame Decode15bppRle(const uint8_t* commandAndRle, size_t bytes,
                    uint16_t width, uint16_t height) {
    const size_t required = RequiredBytes(width, height);
    if (!commandAndRle || bytes < 4u)
        throw std::invalid_argument("Movie command/RLE is missing");
    Frame frame;
    frame.width = width; frame.height = height;
    frame.strips15.resize(required);
    // Reuse the unchanged S0/Stage1 kernel, not a new decoder implementation.
    frame.decode = PrSS0MdecOutputDirect::ExecuteMdec15bppCurrentScus(
        commandAndRle, bytes, width, height, frame.strips15.data(), frame.strips15.size());
    if (!frame.decode.known || !frame.decode.executed ||
        !frame.decode.dma1VerticalMacroblockLayout ||
        frame.decode.outputBytesWritten != required)
        throw std::runtime_error("Shared movie codec rejected the actual command/RLE");
    frame.rgba = Project15bpp(frame.strips15.data(), frame.strips15.size(), width, height);
    return frame;
}
}
