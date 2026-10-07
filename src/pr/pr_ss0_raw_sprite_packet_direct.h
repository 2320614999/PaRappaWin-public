#pragma once

#include "pr_psx_fast_sprite_submit_direct.h"
#include "pr_psx_graph_owner_direct.h"

namespace PrSS0RawSpritePacketDirect {

// 8003FA20 writes command and RGB into the same word. For raw-texture
// commands (65/67), RGB does not modulate the texture. Keep unknown RGB
// unknown: the native partial packet already carries command authority.
inline bool ResolveCommand8003FA20(
    const PrPsxFastSpriteSubmitDirect::RuntimePacketWrite8003FA20& packet,
    uint8_t& command) {
    command = 0;
    if (!packet.valid || packet.wordCount != 6u) return false;
    for (uint32_t i = 0; i < 6u; ++i) {
        if (i != 2u && !packet.wordKnown[i]) return false;
    }
    if (!packet.word2CommandKnown) return false;
    const uint8_t nativeCommand = packet.word2CommandCode;
    if (nativeCommand != 0x65u && nativeCommand != 0x67u) return false;
    if (packet.wordKnown[2] &&
        static_cast<uint8_t>(packet.words[2] >> 24u) != nativeCommand) return false;
    command = nativeCommand;
    return true;
}

inline bool ResolvePagePosition8003FA20(
    const PrPsxFastSpriteSubmitDirect::RuntimePacketWrite8003FA20& packet,
    const PrPsxGraphOwnerDirect::PsxGraphState& graph,
    int32_t& x, int32_t& y) {
    if (!packet.wordKnown[3] || !graph.drawOffset.setDrawEnvCalled) return false;
    // GPU draw offset is in VRAM coordinates. The host presents just the
    // current draw-area rectangle, not both PSX framebuffers stacked together.
    x = static_cast<int16_t>(static_cast<int16_t>(packet.words[3] & 0xffffu) +
                            graph.drawOffset.word_80091738) - graph.drawOffset.word_80091730;
    y = static_cast<int16_t>(static_cast<int16_t>(packet.words[3] >> 16u) +
                            graph.drawOffset.word_8009173A) - graph.drawOffset.word_80091732;
    return true;
}

struct TextureOrigin {
    uint16_t x = 0, y = 0, clutX = 0, clutY = 0;
};

inline bool ResolveTextureOrigin8003FA20(
    const PrPsxFastSpriteSubmitDirect::RuntimePacketWrite8003FA20& packet,
    TextureOrigin& out) {
    out = {};
    if (!packet.wordKnown[1] || !packet.wordKnown[4]) return false;
    const uint16_t page = static_cast<uint16_t>(packet.words[1] & 0x1ffu);
    const unsigned mode = (page >> 7u) & 3u;
    if (mode > 2u) return false;
    const unsigned pixelsPerWord = 4u >> mode;
    const unsigned u = packet.words[4] & 0xffu;
    if (u % pixelsPerWord) return false;
    out.x = static_cast<uint16_t>((page & 15u) * 64u + u / pixelsPerWord);
    out.y = static_cast<uint16_t>(((page & 16u) ? 256u : 0u) + ((packet.words[4] >> 8u) & 0xffu));
    const uint16_t clut = static_cast<uint16_t>(packet.words[4] >> 16u);
    out.clutX = static_cast<uint16_t>((clut & 63u) * 16u);
    out.clutY = clut >> 6u;
    return true;
}

} // namespace PrSS0RawSpritePacketDirect
