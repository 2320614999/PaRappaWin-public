#pragma once

#include "pr_psx_sprite_template_render.h"

#include <cstdint>

namespace PrStage1FailPromptDirect {

struct SpriteCommand {
    float x = 0.0f;
    float y = 0.0f;
    PrPsxSpriteTemplateRender::PsxSpriteTemplate tpl{};
    int priority = 0;
};

struct DrawCommands {
    SpriteCommand commands[3]{};
    uint32_t count = 0;
};

DrawCommands BuildDrawCommands_800203D4(int32_t ctx0);

}  // namespace PrStage1FailPromptDirect
