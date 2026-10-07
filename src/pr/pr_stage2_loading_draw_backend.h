#pragma once
#include "pr_stage2_loading_pattern_direct.h"
#include "pr_stage2_vram_atlas.h"
#include <vector>

namespace PrStage2LoadingDrawBackend {
struct Command {
    bool box=false;
    uint32_t x=0,y=0,width=0,height=0,color=0,priority=0,source=0;
    uint16_t tpage=0,clut=0;
    uint8_t u=0,v=0;
};
struct Frame { std::vector<Command> sourceOrder; };
// Executes the original pattern against THIS session's RAM. Templates are
// captured at each source call; no global Stage1 state or asset-file reload.
Frame Capture(PrStage2LifecycleDirect::Services& memory,uint32_t mode,uint32_t priority);
// Host projection of the two lower drawing calls. This is not a GPU/DMA
// controller, Gs packet allocator, Present, or complete 8001EA74 replacement.
// Every texture is validated/leased before submitting any part of the frame.
void Render(const Frame& frame,PrStage2VramAtlas::Projection& atlas,
            D3D11Renderer& renderer,float x,float y,float scale);
}
