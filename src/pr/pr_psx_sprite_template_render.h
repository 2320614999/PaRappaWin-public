#pragma once

#include <cstdint>
#include <filesystem>

struct ID3D11ShaderResourceView;
struct PrGameContext;
struct TextureResource;

namespace PrPsxSpriteTemplateRender {

struct PsxSpriteTemplate {
    uint32_t attr = 0;
    uint16_t texX_hw = 0;
    uint16_t texY_px = 0;
    uint16_t w = 0;
    uint16_t h = 0;
    uint16_t clutX_px = 0;
    uint16_t clutY_px = 0;
};

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
