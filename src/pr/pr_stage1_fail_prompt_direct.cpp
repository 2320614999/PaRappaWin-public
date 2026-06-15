#include "pr_stage1_fail_prompt_direct.h"

namespace PrStage1FailPromptDirect {
namespace {

void AppendSprite(DrawCommands& out,
                  float x,
                  float y,
                  const PrPsxSpriteTemplateRender::PsxSpriteTemplate& tpl,
                  int priority) {
    if (out.count >= 3u) {
        return;
    }
    SpriteCommand& command = out.commands[out.count++];
    command.x = x;
    command.y = y;
    command.tpl = tpl;
    command.priority = priority;
}

}  // namespace

DrawCommands BuildDrawCommands_800203D4(int32_t ctx0) {
    static constexpr PrPsxSpriteTemplateRender::PsxSpriteTemplate kPromptTitle =
        {0x50000040, 0x03C0, 0x01CB, 0x00D0, 0x0024, 0x0130, 0x01EE};
    static constexpr PrPsxSpriteTemplateRender::PsxSpriteTemplate kLeftDefault =
        {0x50000040, 0x03C0, 0x01A9, 0x0054, 0x0022, 0x0130, 0x01EF};
    static constexpr PrPsxSpriteTemplateRender::PsxSpriteTemplate kRightDefault =
        {0x50000040, 0x03D5, 0x01A9, 0x004C, 0x0022, 0x0130, 0x01F0};
    static constexpr PrPsxSpriteTemplateRender::PsxSpriteTemplate kLeftSelected =
        {0x50000040, 0x03C0, 0x01A9, 0x0054, 0x0022, 0x0130, 0x01F1};
    static constexpr PrPsxSpriteTemplateRender::PsxSpriteTemplate kRightSelected =
        {0x50000040, 0x03D5, 0x01A9, 0x004C, 0x0022, 0x0130, 0x01F2};

    DrawCommands out{};
    AppendSprite(out, 56.0f, 57.0f, kPromptTitle, 730);
    AppendSprite(out,
                 70.0f,
                 149.0f,
                 ctx0 == 0 ? kLeftSelected : kLeftDefault,
                 731);
    AppendSprite(out,
                 178.0f,
                 152.0f,
                 ctx0 == 1 ? kRightSelected : kRightDefault,
                 731);
    return out;
}

}  // namespace PrStage1FailPromptDirect
