#include "pr_ss0_title_draw_backend.h"

#include "d3d11_renderer.h"
#include "pr_game_context.h"
#include "pr_ss0_title_packet_render_direct.h"
#include "pr_ss0_title_tmd_backend.h"
#include "pr_vram_atlas.h"

#include <algorithm>
#include <array>
#include <cstddef>

namespace PrSS0TitleDrawBackend {
namespace {

float NormalizeColor128(uint8_t component)
{
    return std::clamp(
        static_cast<float>(component) / 128.0f, 0.0f, 1.0f);
}

} // namespace

SubmitResult801C689C SubmitCurrentMode25Packets801C689C(
    PrGameContext& ctx,
    const PrSS0TitlePacketWorkDirect::RuntimeState801C609C& runtime,
    float viewportX,
    float viewportY,
    float viewportScale,
    int layer)
{
    const auto decoded =
        PrSS0TitlePacketRenderDirect::
            BuildCurrentMode25DrawCommands801C689C(runtime);
    return SubmitMode25DrawCommands801C689C(
        ctx, decoded, viewportX, viewportY, viewportScale, layer);
}

SubmitResult801C689C SubmitMode25DrawCommands801C689C(
    PrGameContext& ctx,
    const PrSS0TitlePacketRenderDirect::BuildResult801C689C& decoded,
    float viewportX,
    float viewportY,
    float viewportScale,
    int layer)
{
    SubmitResult801C689C result{};
    result.decodeComplete = decoded.complete;
    result.decodeFailure = decoded.failure;
    result.decodedTriangles =
        static_cast<uint32_t>(decoded.commandCount);
    if (!decoded.complete) {
        return result;
    }

    if (ctx.renderer == nullptr) {
        result.missingTextures = result.decodedTriangles;
        return result;
    }
    const auto atlasUpload =
        PrSS0TitleTmdBackend::PrepareTitleVramAtlas801C689C(ctx.renderer);
    result.textureUploadAttempted = atlasUpload.attempted;
    result.textureUploadComplete = atlasUpload.complete;
    result.directScene0AtlasUploadCommitted =
        atlasUpload.sourceScene0IntCpuAtlasAuthority &&
        atlasUpload.complete;
    result.readyTpages = atlasUpload.readyTpageCount;
    result.failedTpageUploads = atlasUpload.failedTpageCount;
    if (!result.directScene0AtlasUploadCommitted) {
        result.missingTextures = result.decodedTriangles;
        return result;
    }

    std::array<ID3D11ShaderResourceView*,
               PrSS0TitlePacketRenderDirect::
                   kMaxMode25DrawCommands801C689C>
        textures{};
    for (std::size_t commandIndex = 0;
         commandIndex < static_cast<std::size_t>(decoded.commandCount);
         ++commandIndex) {
        const auto& command = decoded.commands[commandIndex];
        textures[commandIndex] =
            PrSS0TitleTmdBackend::ResolveTitleTpageSRV801C689C(
                ctx.renderer, command.tpage, command.clut);
        if (textures[commandIndex] == nullptr) {
            ++result.missingTextures;
        }
    }
    if (result.missingTextures != 0u) {
        return result;
    }
    result.texturePreflightComplete = true;

    for (std::size_t commandIndex = 0;
         commandIndex < static_cast<std::size_t>(decoded.commandCount);
         ++commandIndex) {
        const auto& command = decoded.commands[commandIndex];
        D3D11Renderer::TexturedTriCmd draw{};
        draw.texture = textures[commandIndex];
        draw.vertexCount = 3;
        draw.blend = D3D11Renderer::BlendMode::Alpha;
        draw.layer = layer;
        draw.order = static_cast<uint64_t>(command.traversalOrder) + 1u;

        const float r = NormalizeColor128(command.r);
        const float g = NormalizeColor128(command.g);
        const float b = NormalizeColor128(command.b);
        // COMPO00's title graph is initialized through 80040B84 with a
        // 320x240 draw environment and screen center (160,120), while the
        // title GTE control intentionally keeps OFX/OFY at zero.  RTPT's
        // SXY words therefore remain centered coordinates; the PSX draw
        // offset is applied by the GPU when the packet is consumed.  The
        // host path must apply that same draw offset before mapping the
        // packet into the letterboxed viewport.  Omitting it leaves the
        // native logo/character half off the left/top edge (the old shell
        // symptom seen in the visual audit).
        constexpr float kPsxTitleDrawOffsetX = 160.0f;
        constexpr float kPsxTitleDrawOffsetY = 120.0f;
        for (std::size_t vertexIndex = 0; vertexIndex < 3u;
             ++vertexIndex) {
            TexturedVertex& vertex = draw.vertices[vertexIndex];
            vertex.x = viewportX +
                       (static_cast<float>(command.x[vertexIndex]) +
                        kPsxTitleDrawOffsetX) *
                           viewportScale;
            vertex.y = viewportY +
                       (static_cast<float>(command.y[vertexIndex]) +
                        kPsxTitleDrawOffsetY) *
                           viewportScale;
            // Mode-25 packets carry PSX byte UVs.  Use texel-centre
            // coordinates for the host atlas; integer page boundaries are
            // ambiguous to D3D point sampling and become visible as seams
            // when the title models are magnified.
            PsxVramAtlas::UVtoNormalized(
                command.u[vertexIndex], command.v[vertexIndex],
                vertex.u, vertex.v);
            vertex.r = r;
            vertex.g = g;
            vertex.b = b;
            vertex.a = 1.0f;
            vertex.perspectiveW = 1.0f;
        }

        ctx.renderer->SubmitTexturedTriangles(draw);
        ++result.submittedTriangles;
    }

    return result;
}

} // namespace PrSS0TitleDrawBackend
