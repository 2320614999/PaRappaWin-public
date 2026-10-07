#pragma once

#include <cstdint>
#include <filesystem>
#include <d3d11.h>
#include <wrl/client.h>

struct ID3D11ShaderResourceView;
struct PrGameContext;
struct TextureResource;
class PsxVramAtlas;

namespace PrPsxSpriteTemplateRender {

// Scoped resident-menu projection; replacement-texture fast paths remain intact.
void BindResidentDirectoryAtlasProjection80015788(PsxVramAtlas* atlas);

struct PsxSpriteTemplate {
    uint32_t attr = 0;
    uint16_t texX_hw = 0;
    uint16_t texY_px = 0;
    uint16_t w = 0;
    uint16_t h = 0;
    uint16_t clutX_px = 0;
    uint16_t clutY_px = 0;
};

// Resolve without drawing. A prepared view owns its SRV through later atlas
// rebuilds; submission copies that lease into the renderer's queued command.
struct PreparedSpriteTexture {
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> texture;
    float u0 = 0.0f, v0 = 0.0f, u1 = 1.0f, v1 = 1.0f;
    float width = 0.0f, height = 0.0f;
    bool abr1Stp = false;
};

bool ResolvePsxSpriteTemplateTexture(PrGameContext& ctx,
                                    const PsxSpriteTemplate& tpl,
                                    bool abr1Stp,
                                    PreparedSpriteTexture& out);
bool SubmitPreparedPsxSpriteOrdered(PrGameContext& ctx,
                                    float vx, float vy, float vs, float x, float y,
                                    const PreparedSpriteTexture& texture,
                                    float r, float g, float b, float a,
                                    int layer, int order);

int PsxBppFromAttr(uint32_t attr);
uint32_t MakePsxAttrForBpp(int bpp);

const char* FindPracticeTimKeyByTemplate(const std::filesystem::path& dataRoot,
                                         const PsxSpriteTemplate& tpl);
TextureResource* FindLoadedTimTextureByTemplate(PrGameContext& ctx,
                                                const PsxSpriteTemplate& tpl,
                                                const char** outKey = nullptr);

bool DrawPsxSpriteTemplateViaUiAtlas(PrGameContext& ctx,
                                     float vx,
                                     float vy,
                                     float vs,
                                     float x,
                                     float y,
                                     const PsxSpriteTemplate& tpl,
                                     float r,
                                     float g,
                                     float b,
                                     float a,
                                     int layer,
                                     int order = 0);
bool DrawPsxSpriteTemplate(PrGameContext& ctx,
                           float vx,
                           float vy,
                           float vs,
                           float x,
                           float y,
                           const PsxSpriteTemplate& tpl,
                           float r,
                           float g,
                           float b,
                           float a,
                           int layer);
bool DrawPsxSpriteTemplateOrdered(PrGameContext& ctx,
                                  float vx,
                                  float vy,
                                  float vs,
                                  float x,
                                  float y,
                                  const PsxSpriteTemplate& tpl,
                                  float r,
                                  float g,
                                  float b,
                                  float a,
                                  int layer,
                                  int order);
bool DrawPsxSpriteTemplateAbr1StpOrdered(PrGameContext& ctx,
                                         float vx,
                                         float vy,
                                         float vs,
                                         float x,
                                         float y,
                                         const PsxSpriteTemplate& tpl,
                                         float r,
                                         float g,
                                         float b,
                                         float a,
                                         int layer,
                                         int order);
bool DrawPsxSpriteTemplateScaled(PrGameContext& ctx,
                                 float vx,
                                 float vy,
                                 float vs,
                                 float x,
                                 float y,
                                 const PsxSpriteTemplate& tpl,
                                 float scaleX,
                                 float scaleY,
                                 float r,
                                 float g,
                                 float b,
                                 float a,
                                 int layer,
                                 int order = 0);
bool DrawPsxSpriteTemplateSubrect(PrGameContext& ctx,
                                  float vx,
                                  float vy,
                                  float vs,
                                  float x,
                                  float y,
                                  const PsxSpriteTemplate& baseTpl,
                                  int uOffsetPx,
                                  int drawWidthPx,
                                  int clutYOffset,
                                  float r,
                                  float g,
                                  float b,
                                  float a,
                                  int layer);
bool DrawPsxSpriteTemplateSubrectScaled(PrGameContext& ctx,
                                        float vx,
                                        float vy,
                                        float vs,
                                        float x,
                                        float y,
                                        const PsxSpriteTemplate& baseTpl,
                                        int uOffsetPx,
                                        int drawWidthPx,
                                        int clutYOffset,
                                        float scaleX,
                                        float scaleY,
                                        float r,
                                        float g,
                                        float b,
                                        float a,
                                        int layer,
                                        int order = 0);

}  // namespace PrPsxSpriteTemplateRender
