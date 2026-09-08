#include "pr_ss0_event4_prompt_render_direct.h"

#include <array>
#include <cstdint>

namespace PrSS0Event4PromptRenderDirect {
namespace {

struct PromptTemplate800203D4 {
    uint32_t address = 0;
    uint32_t attr = 0;
    uint16_t texX = 0;
    uint16_t texY = 0;
    uint16_t width = 0;
    uint16_t height = 0;
    uint16_t clutX = 0;
    uint16_t clutY = 0;
};

constexpr PromptTemplate800203D4 kTitleTemplate80050950 = {
    0x80050950u, 0x50000040u, 0x03C0u, 0x01CBu, 0x00D0u, 0x0024u,
    0x0130u,     0x01EEu};
constexpr PromptTemplate800203D4 kLeftDefaultTemplate80050960 = {
    0x80050960u, 0x50000040u, 0x03C0u, 0x01A9u, 0x0054u, 0x0022u,
    0x0130u,     0x01EFu};
constexpr PromptTemplate800203D4 kRightDefaultTemplate80050970 = {
    0x80050970u, 0x50000040u, 0x03D5u, 0x01A9u, 0x004Cu, 0x0022u,
    0x0130u,     0x01F0u};
constexpr PromptTemplate800203D4 kLeftSelectedTemplate80050980 = {
    0x80050980u, 0x50000040u, 0x03C0u, 0x01A9u, 0x0054u, 0x0022u,
    0x0130u,     0x01F1u};
constexpr PromptTemplate800203D4 kRightSelectedTemplate80050990 = {
    0x80050990u, 0x50000040u, 0x03D5u, 0x01A9u, 0x004Cu, 0x0022u,
    0x0130u,     0x01F2u};

constexpr std::array<int16_t, kEvent4PromptSpriteCount800203D4>
    kScreenX800203D4{{56, 70, 178}};
constexpr std::array<int16_t, kEvent4PromptSpriteCount800203D4>
    kScreenY800203D4{{57, 149, 152}};

uint16_t ResolveTpage8001B25C(uint16_t texX, uint16_t texY) {
    const uint16_t uvWord = static_cast<uint16_t>(4u * texX);
    const uint16_t helperA3 = static_cast<uint16_t>((uvWord & 0xFF00u) >> 2u);
    const uint16_t helperA4 = static_cast<uint16_t>(texY & 0xFF00u);
    return static_cast<uint16_t>(
        0x20u |
        ((helperA4 & 0x0100u) >> 4u) |
        ((helperA3 & 0x03FFu) >> 6u) |
        (4u * (helperA4 & 0x0200u)));
}

uint32_t ResolvePacketWord2Color8003FA20(uint32_t attr,
                                         uint8_t r,
                                         uint8_t g,
                                         uint8_t b) {
    return ((attr >> 5) & 0x02000000u) |
           ((attr << 18) & 0x01000000u) |
           0x64000000u |
           (static_cast<uint32_t>(b) << 16) |
           (static_cast<uint32_t>(g) << 8) |
           static_cast<uint32_t>(r);
}

uint8_t ResolvePacketWord2Command8003FA20(uint32_t attr) {
    return static_cast<uint8_t>(((attr >> 29u) & 0x02u) |
                                ((attr >> 6u) & 0x01u) | 0x64u);
}

PromptTemplate800203D4 ResolveTemplate800203D4(std::size_t index,
                                               int32_t ctx0) {
    if (index == 0u) {
        return kTitleTemplate80050950;
    }
    if (index == 1u) {
        return ctx0 == 0 ? kLeftSelectedTemplate80050980
                         : kLeftDefaultTemplate80050960;
    }
    return ctx0 == 1 ? kRightSelectedTemplate80050990
                     : kRightDefaultTemplate80050970;
}

bool IsSelected800203D4(std::size_t index, int32_t ctx0) {
    return (index == 1u && ctx0 == 0) || (index == 2u && ctx0 == 1);
}

Event4PromptSprite800203D4 BuildSprite800203D4(
    std::size_t index,
    int32_t ctx0,
    const Event4PromptRgbSource800203D4& rgb) {
    const PromptTemplate800203D4 tpl = ResolveTemplate800203D4(index, ctx0);
    Event4PromptSprite800203D4 out{};
    out.known = true;
    out.selected = IsSelected800203D4(index, ctx0);
    out.templateAddress = tpl.address;
    out.screenX = kScreenX800203D4[index];
    out.screenY = kScreenY800203D4[index];
    out.localX = static_cast<int16_t>(out.screenX - 160);
    out.localY = static_cast<int16_t>(out.screenY - 120);
    out.attr = tpl.attr;
    out.texX = tpl.texX;
    out.texY = tpl.texY;
    out.width = tpl.width;
    out.height = tpl.height;
    out.clutX = tpl.clutX;
    out.clutY = tpl.clutY;
    out.tpage = ResolveTpage8001B25C(tpl.texX, tpl.texY);
    out.u = static_cast<uint8_t>(4u * tpl.texX);
    out.v = static_cast<uint8_t>(tpl.texY);
    if (rgb.known) {
        out.r = rgb.r;
        out.g = rgb.g;
        out.b = rgb.b;
    }
    out.rawTextureKnown = true;
    out.rawTexture = (out.attr & 0x40u) != 0u;
    out.rgbKnown = rgb.known;
    out.word2CommandKnown = true;
    out.word2CommandCode = ResolvePacketWord2Command8003FA20(out.attr);
    out.packetWord2ColorKnown = rgb.known;
    if (rgb.known) {
        out.packetWord2ColorCode = ResolvePacketWord2Color8003FA20(
            out.attr, out.r, out.g, out.b);
    }
    out.callOrder = static_cast<uint32_t>(index);
    return out;
}

}  // namespace

Event4PromptDrawList800203D4 BuildEvent4PromptDrawList800203D4(
    const Event4PromptInput800203D4& input) {
    Event4PromptDrawList800203D4 out{};
    out.requestBound = input.requestBound;
    out.choiceSourceKnown = input.choiceSourceKnown;
    out.ctx0 = input.ctx0;
    // 800203D4 compares only with 0 and 1. Every other known value uses
    // both default templates; the producer's ordinary -1/0/1 range is not a
    // renderer input gate.
    if (!input.requestBound || !input.choiceSourceKnown) {
        return out;
    }

    std::array<Event4PromptSprite800203D4,
               kEvent4PromptSpriteCount800203D4>
        candidate{};
    bool allRgbKnown = true;
    bool rawTextureOnly = true;
    for (std::size_t i = 0u; i < candidate.size(); ++i) {
        candidate[i] = BuildSprite800203D4(i, input.ctx0, input.rgb[i]);
        allRgbKnown = allRgbKnown && candidate[i].rgbKnown;
        rawTextureOnly = rawTextureOnly && candidate[i].rawTextureKnown &&
                         candidate[i].rawTexture;
    }

    // 8001B590 leaves +0x14..+0x16 as stack scratch.  All five formal
    // templates set the GPU raw-texture bit, so those bytes do not modulate
    // the visible texture.  Keep the packet color unknown; do not invent a
    // default RGB value or block the translated draw list on dead data.
    if (!rawTextureOnly && !allRgbKnown) {
        return out;
    }

    out.sprites = candidate;
    out.count = candidate.size();
    out.accepted = true;
    out.complete = true;
    out.rgbSourceKnown = allRgbKnown;
    out.rgbTailUnresolvedWithoutInput = !allRgbKnown;
    out.rawTextureOnly = rawTextureOnly;
    out.visibleColorIndependentOfRgb = rawTextureOnly;
    return out;
}

}  // namespace PrSS0Event4PromptRenderDirect
