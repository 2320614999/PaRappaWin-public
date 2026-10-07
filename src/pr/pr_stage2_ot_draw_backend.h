#pragma once
#include "pr_stage2_lifecycle_direct.h"
#include "pr_stage2_vram_atlas.h"
#include <array>
#include <vector>

namespace PrStage2OtDrawBackend {
// Host consumption of native GP0 words, not a replacement for DrawOTag's
// original dispatch/IRQ completion. The future device owner submits here
// when the original DMA linked-list transaction is actually consumed.
struct DrawState {
    uint16_t tpage = 0;
    int16_t offsetX = 0, offsetY = 0;
    int16_t clipLeft = 0, clipTop = 0, clipRight = 0, clipBottom = 0;
    uint16_t drawModeFlags = 0; // E1 bits 9/10 retained for concrete GPUSTAT.
};
// Explicit host view of the selected native framebuffer; no guessed 320x240
// or borrowing of Stage1's global camera/atlas. Dimensions must be positive.
struct Viewport {
    int frameX, frameY, frameWidth, frameHeight;
    float x, y, width, height;
};
struct Vertex {
    int16_t x = 0, y = 0;
    uint8_t u = 0, v = 0, r = 0, g = 0, b = 0;
};
struct HostVertex { float x=0,y=0,u=0,v=0,r=1,g=1,b=1; };
struct HostPrimitive {
    std::array<HostVertex,4> vertex{};
    ComPtr<ID3D11ShaderResourceView> texture;
    float alpha=1;
    D3D11Renderer::BlendMode blend=D3D11Renderer::BlendMode::Alpha;
    bool textureOverride=false;
    bool expandedViewport=false; // host-only scene outside the original 4:3 edges
    uint64_t motionIdentity=0; // native descriptor/primitive, never OT position
};
struct Command {
    // packet owns the first command word; wordOffset indexes the concatenated
    // GP0 stream (not the payload offset within a DMA node).
    uint32_t packet = 0, wordOffset = 0;
    std::shared_ptr<HostPrimitive> host;
    uint8_t opcode = 0, vertices = 0;
    uint16_t clut = 0;
    DrawState state{};
    std::array<Vertex,4> vertex{};
    uint16_t fillWidth = 0, fillHeight = 0;
    // GP0 显存复制使用绝对坐标，绕过绘图偏移与裁剪区域。
    uint16_t copySourceX=0,copySourceY=0,copyDestinationX=0,copyDestinationY=0;
};
struct Batch {
    DrawState finalState{};
    uint32_t nodes = 0;
    std::vector<Command> commands;
};
// Fails explicitly for malformed chains or GP0 operations not implemented
// here. Does not sort by material/depth or impose the old title's one OT.
Batch DecodeLinkedList(PrStage2LifecycleDirect::Services& memory,
                       uint32_t head, DrawState state);
// Actual GP0 port words share the decoder, without a fabricated PSX RAM node.
Batch DecodeCommandStream(const std::vector<uint32_t>& words, DrawState state);
struct RenderResult { uint32_t commands = 0, triangles = 0, gpuDraws = 0; };
// Prepares every resource before submitting any draw. Missing native VRAM
// fails the batch, never draws stale textures or silently drops a model.
RenderResult Render(const Batch& batch, PrStage2VramAtlas::Projection& atlas,
                    D3D11Renderer& renderer, const Viewport& viewport);
}
