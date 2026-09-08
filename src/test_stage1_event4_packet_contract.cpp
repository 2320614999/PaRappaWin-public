#include "pr/pr_psx_event_frame_direct.h"
#include "pr/pr_ss0_event4_prompt_render_direct.h"
#include <cstdio>

namespace Frame = PrPsxEventFrameDirect;
namespace Prompt = PrSS0Event4PromptRenderDirect;
namespace Sprite = PrPsxFastSpriteSubmitDirect;

int main() {
    int failures = 0;
    const auto check = [&](bool value, const char* label) {
        if (!value) { std::printf("FAIL: %s\n", label); ++failures; }
    };
    // Exercise the production page encoder across every PSX page boundary.
    for (unsigned y : {0u, 256u, 512u, 768u}) {
        for (unsigned x = 0; x < 1024; ++x) {
            check(Frame::TexturePage80043DF4(x, y) ==
                      (0x20u | ((y / 256u) % 2u) * 16u | x / 64u |
                       (y / 512u) * 2048u), "native page encoding");
        }
    }
    // Bind all five original palettes to the actual GsSortFastSprite packet
    // encoder. Full frame/atlas ownership is covered by live capture.
    for (int choice : {-1, 0, 1}) {
        Prompt::Event4PromptInput800203D4 prompt{};
        prompt.requestBound = prompt.choiceSourceKnown = true;
        prompt.ctx0 = choice;
        const auto list = Prompt::BuildEvent4PromptDrawList800203D4(prompt);
        check(list.complete && list.count == 3u, "three original prompt sprites");
        for (std::size_t i = 0; i < list.count; ++i) {
            const auto& desc = list.sprites[i];
            Sprite::GsSortFastSpriteInput8003FA20 input{};
            auto& local = input.sprite;
            local.attr_00 = desc.attr;
            local.x_04 = desc.localX;
            local.y_06 = static_cast<uint16_t>(desc.localY);
            local.width_08 = desc.width;
            local.height_0A = desc.height;
            local.tpage_0C = Frame::TexturePage80043DF4(
                static_cast<uint16_t>(((4u * desc.texX) & 0xFF00u) >> 2u),
                static_cast<uint16_t>(desc.texY & 0xFF00u));
            local.u_0E = desc.u;
            local.v_0F = desc.v;
            local.clutX_10 = desc.clutX;
            local.clutY_12 = desc.clutY;
            input.drawOffsets.word_800917AA = 160;
            input.drawOffsets.word_800917AC = 120;
            const auto result = Sprite::PredictGsSortFastSpritePartial8003FA20(input, false);
            const auto& packet = result.packet;
            check(packet.wouldWrite && !result.skipped, "native sprite submitted");
            check((packet.word1_drawMode & 0x1FFu) == 0x003Fu, "prompt tpage 003F");
            check(packet.word2CommandKnown && packet.word2CommandCode == 0x67u &&
                  !packet.word2ColorKnown, "raw-texture opcode needs no fabricated RGB");
            check(static_cast<int16_t>(packet.word3_xy) == desc.screenX &&
                  static_cast<int16_t>(packet.word3_xy >> 16) == desc.screenY,
                  "centered graph restores original prompt position");
            check((packet.word4_uvClut >> 16) ==
                  ((static_cast<unsigned>(desc.clutY) << 6) | (desc.clutX >> 4)),
                  "native prompt CLUT");
        }
    }
    std::printf("Stage1 Event4 packet contract: %s (%d failures)\n",
                failures ? "FAIL" : "PASS", failures);
    return failures ? 1 : 0;
}
