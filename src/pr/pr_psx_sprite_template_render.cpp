#include "pr_psx_sprite_template_render.h"

#include "d3d11_renderer.h"
#include "logger.h"
#include "pr_game_context.h"
#include "pr_stage1_texture_replacements.h"
#include "pr_vram_atlas.h"
#include "resource_manager.h"
#include "tim_decoder.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

namespace PrPsxSpriteTemplateRender {
namespace {

static float ToScreenX(float vx, float vs, float x) {
    return vx + x * vs;
}

static float ToScreenY(float vy, float vs, float y) {
    return vy + y * vs;
}

static void SubmitSpriteUI_SRV_UV(PrGameContext& ctx,
                                  float vx,
                                  float vy,
                                  float vs,
                                  ID3D11ShaderResourceView* srv,
                                  float x,
                                  float y,
                                  float w,
                                  float h,
                                  float u0,
                                  float v0,
                                  float u1,
                                  float v1,
                                  float r,
                                  float g,
                                  float b,
                                  float a,
                                  int layer,
                                  int order = 0) {
    if (!ctx.renderer || !srv) {
        return;
    }

    D3D11Renderer::SpriteCmd cmd;
    cmd.texture = srv;
    cmd.x = ToScreenX(vx, vs, x);
    cmd.y = ToScreenY(vy, vs, y);
    cmd.w = w * vs;
    cmd.h = h * vs;
    cmd.u0 = u0;
    cmd.v0 = v0;
    cmd.u1 = u1;
    cmd.v1 = v1;
    cmd.r = r;
    cmd.g = g;
    cmd.b = b;
    cmd.a = a;
    cmd.blend = D3D11Renderer::BlendMode::Alpha;
    cmd.layer = layer;
    cmd.order = order;
    ctx.renderer->SubmitSprite(cmd);
}

static void BuildTextureSubrectUv(float texelX,
                                  float texelY,
                                  float texelW,
                                  float texelH,
                                  float textureW,
                                  float textureH,
                                  float& u0,
                                  float& v0,
                                  float& u1,
                                  float& v1) {
    if (texelW <= 1.0f) {
        u0 = u1 = (texelX + 0.5f) / textureW;
    } else {
        u0 = (texelX + 0.5f) / textureW;
        u1 = (texelX + texelW - 0.5f) / textureW;
    }
    if (texelH <= 1.0f) {
        v0 = v1 = (texelY + 0.5f) / textureH;
    } else {
        v0 = (texelY + 0.5f) / textureH;
        v1 = (texelY + texelH - 0.5f) / textureH;
    }
}

static uint64_t SpriteTemplateReplacementKey(const PsxSpriteTemplate& tpl,
                                             int subX,
                                             int subY,
                                             int subW,
                                             int subH) {
    uint64_t key = static_cast<uint64_t>(tpl.attr);
    key = (key << 16) ^ static_cast<uint64_t>(tpl.texX_hw);
    key = (key << 16) ^ static_cast<uint64_t>(tpl.texY_px);
    key = (key << 16) ^ static_cast<uint64_t>(tpl.clutX_px);
    key = (key << 16) ^ static_cast<uint64_t>(tpl.clutY_px);
    key ^= (static_cast<uint64_t>(static_cast<uint16_t>(subX)) << 48);
    key ^= (static_cast<uint64_t>(static_cast<uint16_t>(subY)) << 32);
    key ^= (static_cast<uint64_t>(static_cast<uint16_t>(subW)) << 16);
    key ^= static_cast<uint64_t>(static_cast<uint16_t>(subH));
    return key;
}

static int LoadedTextureMatchPriority(const std::string& key) {
    if (key.rfind("comod:", 0) == 0) return 0;
    if (key.rfind("ycompo:", 0) == 0) return 1;
    return 2;
}

static ID3D11ShaderResourceView* TryTextureReplacementForTim(
    PrGameContext& ctx,
    const PsxSpriteTemplate& tpl,
    const TimImage& tim) {
    if (ctx.currentScene != PrSceneId::Scene1) {
        return nullptr;
    }
    if (tim.rgba.empty() || tim.width == 0u || tim.height == 0u ||
        tim.width != tpl.w || tim.height != tpl.h) {
        return nullptr;
    }

    const uint64_t key =
        SpriteTemplateReplacementKey(tpl,
                                     0,
                                     0,
                                     static_cast<int>(tim.width),
                                     static_cast<int>(tim.height));
    PrStage1TextureReplacements::ReplacementTexture replacement{};
    const bool hit = PrStage1TextureReplacements::TryFindReplacementBySourceKey(
        ctx,
        key,
        replacement);
    char detail[256];
    std::snprintf(detail,
                  sizeof(detail),
                  "attr=0x%08X texHW=%u texY=%u tpl=%ux%u clut=%u,%u tim=%ux%u",
                  tpl.attr,
                  tpl.texX_hw,
                  tpl.texY_px,
                  tpl.w,
                  tpl.h,
                  tpl.clutX_px,
                  tpl.clutY_px,
                  tim.width,
                  tim.height);
    PrStage1TextureReplacements::DebugLogSourceKeyLookup(
        ctx,
        key,
        "template_tim",
        detail,
        hit);
    if (!hit) {
        return nullptr;
    }
    return replacement.srv;
}

static ID3D11ShaderResourceView* TryTextureReplacementForTimSubrect(
    PrGameContext& ctx,
    const PsxSpriteTemplate& tpl,
    const TimImage& tim,
    int subX,
    int subY,
    int subW,
    int subH) {
    if (ctx.currentScene != PrSceneId::Scene1) {
        return nullptr;
    }
    if (tim.rgba.empty() || tim.width == 0u || tim.height == 0u ||
        subX < 0 || subY < 0 || subW <= 0 || subH <= 0 ||
        subX + subW > static_cast<int>(tim.width) ||
        subY + subH > static_cast<int>(tim.height)) {
        return nullptr;
    }

    const uint64_t key = SpriteTemplateReplacementKey(tpl, subX, subY, subW, subH);
    PrStage1TextureReplacements::ReplacementTexture replacement{};
    const bool hit = PrStage1TextureReplacements::TryFindReplacementBySourceKey(
        ctx,
        key,
        replacement);
    char detail[320];
    std::snprintf(detail,
                  sizeof(detail),
                  "attr=0x%08X texHW=%u texY=%u tpl=%ux%u clut=%u,%u sub=%d,%d,%d,%d tim=%ux%u",
                  tpl.attr,
                  tpl.texX_hw,
                  tpl.texY_px,
                  tpl.w,
                  tpl.h,
                  tpl.clutX_px,
                  tpl.clutY_px,
                  subX,
                  subY,
                  subW,
                  subH,
                  tim.width,
                  tim.height);
    PrStage1TextureReplacements::DebugLogSourceKeyLookup(
        ctx,
        key,
        "template_tim_subrect",
        detail,
        hit);
    if (!hit) {
        return nullptr;
    }
    return replacement.srv;
}

static ID3D11ShaderResourceView* TryTextureReplacementForAtlasRect(
    PrGameContext& ctx,
    const PsxSpriteTemplate& tpl,
    PsxVramAtlas& atlas,
    uint16_t tpage,
    uint16_t clut,
    int texelX,
    int texelY,
    int texelW,
    int texelH,
    int keySubX,
    int keySubY) {
    if (ctx.currentScene != PrSceneId::Scene1) {
        return nullptr;
    }
    if (texelW <= 0 || texelH <= 0) {
        return nullptr;
    }

    (void)atlas;
    (void)tpage;
    (void)clut;
    (void)texelX;
    (void)texelY;

    const uint64_t key = SpriteTemplateReplacementKey(tpl,
                                                      keySubX,
                                                      keySubY,
                                                      texelW,
                                                      texelH);
    PrStage1TextureReplacements::ReplacementTexture replacement{};
    const bool hit = PrStage1TextureReplacements::TryFindReplacementBySourceKey(
        ctx,
        key,
        replacement);
    char detail[384];
    std::snprintf(detail,
                  sizeof(detail),
                  "attr=0x%08X texHW=%u texY=%u tpl=%ux%u clutXY=%u,%u tpage=0x%04X clut=0x%04X rect=%d,%d,%d,%d keySub=%d,%d",
                  tpl.attr,
                  tpl.texX_hw,
                  tpl.texY_px,
                  tpl.w,
                  tpl.h,
                  tpl.clutX_px,
                  tpl.clutY_px,
                  tpage,
                  clut,
                  texelX,
                  texelY,
                  texelW,
                  texelH,
                  keySubX,
                  keySubY);
    PrStage1TextureReplacements::DebugLogSourceKeyLookup(
        ctx,
        key,
        "template_atlas_rect",
        detail,
        hit);
    if (!hit) {
        return nullptr;
    }
    return replacement.srv;
}

struct UiVramAtlasCache {
    bool loaded = false;
    std::filesystem::path loadedRoot;
    uint32_t resourceGeneration = 0;
    PsxVramAtlas atlas;
    std::unordered_set<uint16_t> registeredTpages;
};

static uint16_t UiTplTpageFromTemplate(const PsxSpriteTemplate& tpl) {
    const int bpp = PsxBppFromAttr(tpl.attr);
    const int colorMode = bpp == 8 ? 1 : (bpp == 16 ? 2 : 0);
    const int tpageX = (int)tpl.texX_hw / 64;
    const int tpageY = ((int)tpl.texY_px >= 256) ? 1 : 0;
    return (uint16_t)((tpageX & 0xF) | (tpageY << 4) |
                      (colorMode << 7));
}

static uint16_t PackPsxClutCba(uint16_t clutX, uint16_t clutY) {
    return (uint16_t)((((uint32_t)clutY & 0x01FFu) << 6u) |
                      (((uint32_t)clutX >> 4u) & 0x3Fu));
}

static UiVramAtlasCache& GetUiVramAtlasCache(PrGameContext& ctx) {
    static UiVramAtlasCache s_cache;
    const uint32_t gen = ctx.resources ? ctx.resources->GetGeneration() : 0u;
    if (!s_cache.loaded || s_cache.loadedRoot != ctx.dataRoot ||
        s_cache.resourceGeneration != gen) {
        s_cache = UiVramAtlasCache{};
        s_cache.loaded = true;
        s_cache.loadedRoot = ctx.dataRoot;
        s_cache.resourceGeneration = gen;
    }
    return s_cache;
}

static bool EnsureUiAtlasHasTemplate(PrGameContext& ctx,
                                     const PsxSpriteTemplate& tpl) {
    if (!ctx.resources || !ctx.renderer) {
        return false;
    }
    UiVramAtlasCache& cache = GetUiVramAtlasCache(ctx);
    const uint16_t tpage = UiTplTpageFromTemplate(tpl);
    if (cache.registeredTpages.find(tpage) !=
        cache.registeredTpages.end()) {
        return cache.atlas.GetTpageSRV(tpage) != nullptr;
    }

    cache.atlas.RegisterTpage(tpage);
    cache.registeredTpages.insert(tpage);

    const std::vector<std::string> timNames = ctx.resources->GetTimRawNames();
    for (const std::string& name : timNames) {
        const std::vector<uint8_t>* raw = ctx.resources->GetTimRaw(name);
        if (!raw || raw->empty()) {
            continue;
        }
        cache.atlas.LoadTim(raw->data(), raw->size(), name);
    }
    cache.atlas.UploadAll(ctx.renderer);
    return cache.atlas.GetTpageSRV(tpage) != nullptr;
}

static bool DrawPsxSpriteTemplateScaledFallback(PrGameContext& ctx,
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
                                                int order) {
    if (!ctx.renderer || !ctx.resources) {
        return false;
    }

    const float baseW = (float)tpl.w;
    const float baseH = (float)tpl.h;
    const float cx = x + baseW * 0.5f;
    const float cy = y + baseH * 0.5f;
    const float sx = std::fabs(scaleX);
    const float sy = std::fabs(scaleY);
    const float w = baseW * sx;
    const float h = baseH * sy;
    const float x0 = cx - w * 0.5f;
    const float y0 = cy - h * 0.5f;
    const bool flipX = scaleX < 0.0f;

    const auto drawScaled = [&](ID3D11ShaderResourceView* srv,
                                float u0,
                                float v0,
                                float u1,
                                float v1) -> bool {
        if (!srv) {
            return false;
        }
        SubmitSpriteUI_SRV_UV(ctx,
                              vx,
                              vy,
                              vs,
                              srv,
                              x0,
                              y0,
                              w,
                              h,
                              u0,
                              v0,
                              u1,
                              v1,
                              r,
                              g,
                              b,
                              a,
                              layer,
                              order);
        return true;
    };

    TextureResource* tr = FindLoadedTimTextureByTemplate(ctx, tpl);
    if (tr) {
        ID3D11ShaderResourceView* drawSrv =
            TryTextureReplacementForTim(ctx, tpl, tr->tim);
        if (!drawSrv && !tr->srv && !tr->tim.rgba.empty()) {
            tr->srv = ctx.renderer->CreateTexture(
                tr->tim.rgba.data(), tr->tim.width, tr->tim.height);
        }
        if (!drawSrv) {
            drawSrv = tr->srv;
        }
        if (drawScaled(drawSrv,
                       flipX ? 1.0f : 0.0f,
                       0.0f,
                       flipX ? 0.0f : 1.0f,
                       1.0f)) {
            return true;
        }
    }

    if (!EnsureUiAtlasHasTemplate(ctx, tpl)) {
        return false;
    }

    const uint16_t tpage = UiTplTpageFromTemplate(tpl);
    UiVramAtlasCache& cache = GetUiVramAtlasCache(ctx);
    const int bpp = PsxBppFromAttr(tpl.attr);
    const uint16_t clut =
        (bpp == 4 || bpp == 8) ? PackPsxClutCba(tpl.clutX_px, tpl.clutY_px)
                               : 0u;
    ID3D11ShaderResourceView* srv =
        (bpp == 4 || bpp == 8)
            ? cache.atlas.GetTpageSRV(tpage, clut, ctx.renderer)
            : cache.atlas.GetTpageSRV(tpage);
    if (!srv) {
        return false;
    }

    const int pxFactor = bpp == 8 ? 2 : (bpp == 16 ? 1 : 4);
    const int baseHW = (tpage & 0xF) * 64;
    const int baseY = ((tpage >> 4) & 1) * 256;
    const int texelX = ((int)tpl.texX_hw - baseHW) * pxFactor;
    const int texelY = (int)tpl.texY_px - baseY;
    float u0 = 0.0f;
    float v0 = 0.0f;
    float u1 = 0.0f;
    float v1 = 0.0f;
    BuildTextureSubrectUv((float)texelX,
                          (float)texelY,
                          (float)tpl.w,
                          (float)tpl.h,
                          256.0f,
                          256.0f,
                          u0,
                          v0,
                          u1,
                          v1);
    ID3D11ShaderResourceView* replacementSrv =
        TryTextureReplacementForAtlasRect(ctx,
                                          tpl,
                                          cache.atlas,
                                          tpage,
                                          clut,
                                          texelX,
                                          texelY,
                                          static_cast<int>(tpl.w),
                                          static_cast<int>(tpl.h),
                                          0,
                                          0);
    if (replacementSrv) {
        srv = replacementSrv;
        u0 = 0.0f;
        v0 = 0.0f;
        u1 = 1.0f;
        v1 = 1.0f;
    }
    if (flipX) {
        std::swap(u0, u1);
    }
    return drawScaled(srv, u0, v0, u1, v1);
}

}  // namespace

int PsxBppFromAttr(uint32_t attr) {
    const int depth = (int)((attr >> 24) & 3u);
    switch (depth) {
    case 0:
        return 4;
    case 1:
        return 8;
    case 2:
        return 16;
    default:
        return 4;
    }
}

uint32_t MakePsxAttrForBpp(int bpp) {
    switch (bpp) {
    case 8:
        return 1u << 24;
    case 16:
        return 2u << 24;
    case 24:
        return 3u << 24;
    case 4:
    default:
        break;
    }
    return 0u;
}

const char* FindPracticeTimKeyByTemplate(const std::filesystem::path& dataRoot,
                                         const PsxSpriteTemplate& tpl) {
    (void)dataRoot;
    (void)tpl;
    return nullptr;
}

TextureResource* FindLoadedTimTextureByTemplate(PrGameContext& ctx,
                                                const PsxSpriteTemplate& tpl,
                                                const char** outKey) {
    if (outKey) {
        *outKey = nullptr;
    }
    if (!ctx.resources) {
        return nullptr;
    }
    const int bpp = PsxBppFromAttr(tpl.attr);
    static thread_local std::string s_lastMatchedTextureKey;
    std::string bestKey;
    TextureResource* bestTexture = nullptr;
    int bestPriority = 999;

    const std::vector<std::string> textureNames = ctx.resources->GetTextureNames();
    for (const std::string& key : textureNames) {
        TextureResource* tr = ctx.resources->GetTexture(key);
        if (!tr) {
            continue;
        }
        const TimImage& tim = tr->tim;
        if ((int)tim.bpp != bpp) {
            continue;
        }
        if (tim.orgX != tpl.texX_hw || tim.orgY != tpl.texY_px) {
            continue;
        }
        if (tim.width != tpl.w || tim.height != tpl.h) {
            continue;
        }
        if (tim.clutX != tpl.clutX_px || tim.clutY != tpl.clutY_px) {
            continue;
        }

        const int priority = LoadedTextureMatchPriority(key);
        if (!bestTexture || priority < bestPriority) {
            bestTexture = tr;
            bestKey = key;
            bestPriority = priority;
            if (priority == 0) {
                break;
            }
        }
    }

    if (bestTexture && outKey) {
        s_lastMatchedTextureKey = std::move(bestKey);
        *outKey = s_lastMatchedTextureKey.c_str();
    }
    return bestTexture;
}

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
                                     int order) {
    if (!ctx.renderer || !ctx.resources) {
        return false;
    }
    if (!EnsureUiAtlasHasTemplate(ctx, tpl)) {
        return false;
    }

    const uint16_t tpage = UiTplTpageFromTemplate(tpl);
    UiVramAtlasCache& cache = GetUiVramAtlasCache(ctx);
    const int bpp = PsxBppFromAttr(tpl.attr);
    const uint16_t clut =
        (bpp == 4 || bpp == 8) ? PackPsxClutCba(tpl.clutX_px, tpl.clutY_px)
                               : 0u;
    ID3D11ShaderResourceView* srv =
        (bpp == 4 || bpp == 8)
            ? cache.atlas.GetTpageSRV(tpage, clut, ctx.renderer)
            : cache.atlas.GetTpageSRV(tpage);
    if (!srv) {
        return false;
    }

    const int pxFactor = bpp == 8 ? 2 : (bpp == 16 ? 1 : 4);
    const int baseHW = (tpage & 0xF) * 64;
    const int baseY = ((tpage >> 4) & 1) * 256;
    const int texelX = ((int)tpl.texX_hw - baseHW) * pxFactor;
    const int texelY = (int)tpl.texY_px - baseY;
    float u0 = 0.0f;
    float v0 = 0.0f;
    float u1 = 0.0f;
    float v1 = 0.0f;
    BuildTextureSubrectUv((float)texelX,
                          (float)texelY,
                          (float)tpl.w,
                          (float)tpl.h,
                          256.0f,
                          256.0f,
                          u0,
                          v0,
                          u1,
                          v1);
    ID3D11ShaderResourceView* replacementSrv =
        TryTextureReplacementForAtlasRect(ctx,
                                          tpl,
                                          cache.atlas,
                                          tpage,
                                          clut,
                                          texelX,
                                          texelY,
                                          static_cast<int>(tpl.w),
                                          static_cast<int>(tpl.h),
                                          0,
                                          0);
    if (replacementSrv) {
        srv = replacementSrv;
        u0 = 0.0f;
        v0 = 0.0f;
        u1 = 1.0f;
        v1 = 1.0f;
    }
    SubmitSpriteUI_SRV_UV(ctx,
                          vx,
                          vy,
                          vs,
                          srv,
                          x,
                          y,
                          (float)tpl.w,
                          (float)tpl.h,
                          u0,
                          v0,
                          u1,
                          v1,
                          r,
                          g,
                          b,
                          a,
                          layer,
                          order);
    return true;
}

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
                                  int order) {
    if (!ctx.renderer || !ctx.resources) {
        return false;
    }

    const int bpp = PsxBppFromAttr(tpl.attr);
    TextureResource* tr = ctx.resources->FindTextureByTimHeader(
        bpp,
        (int16_t)tpl.texX_hw,
        (int16_t)tpl.texY_px,
        (uint32_t)tpl.w,
        (uint32_t)tpl.h,
        (int16_t)tpl.clutX_px,
        (int16_t)tpl.clutY_px);
    if (!tr) {
        tr = FindLoadedTimTextureByTemplate(ctx, tpl);
    }
    if (!tr) {
        if (DrawPsxSpriteTemplateViaUiAtlas(
                ctx, vx, vy, vs, x, y, tpl, r, g, b, a, layer, order)) {
            return true;
        }
        static uint32_t s_lastFailFrame = 0;
        static uint64_t s_lastFailKey = 0;
        const uint64_t key =
            ((uint64_t)(uint32_t)tpl.attr << 32) ^
            ((uint64_t)(uint16_t)tpl.texX_hw << 16) ^
            (uint64_t)(uint16_t)tpl.texY_px ^
            ((uint64_t)(uint16_t)tpl.w << 48) ^
            ((uint64_t)(uint16_t)tpl.h << 56) ^
            ((uint64_t)(uint16_t)tpl.clutX_px << 8) ^
            (uint64_t)(uint16_t)tpl.clutY_px;
        if ((ctx.frame - s_lastFailFrame) >= 30 || key != s_lastFailKey) {
            Log::Printf(
                "DrawPsxSpriteTemplate: FindTextureByTimHeader miss frame=%u bpp=%d attr=0x%08X org=(%d,%d) wh=(%u,%u) clut=(%d,%d)",
                (unsigned)ctx.frame,
                bpp,
                (unsigned)tpl.attr,
                (int)(int16_t)tpl.texX_hw,
                (int)(int16_t)tpl.texY_px,
                (unsigned)tpl.w,
                (unsigned)tpl.h,
                (int)(int16_t)tpl.clutX_px,
                (int)(int16_t)tpl.clutY_px);
            s_lastFailFrame = ctx.frame;
            s_lastFailKey = key;
        }
        return false;
    }

    ID3D11ShaderResourceView* drawSrv =
        TryTextureReplacementForTim(ctx, tpl, tr->tim);
    if (!drawSrv && !tr->srv && !tr->tim.rgba.empty()) {
        tr->srv = ctx.renderer->CreateTexture(
            tr->tim.rgba.data(), tr->tim.width, tr->tim.height);
    }
    if (!drawSrv) {
        drawSrv = tr->srv;
    }
    if (!drawSrv) {
        static uint32_t s_lastSrvFailFrame = 0;
        if (ctx.frame - s_lastSrvFailFrame >= 30) {
            Log::Printf(
                "DrawPsxSpriteTemplate: srv missing frame=%u name='%s' rgba=%llu w=%u h=%u",
                (unsigned)ctx.frame,
                tr->name.c_str(),
                (unsigned long long)tr->tim.rgba.size(),
                (unsigned)tr->tim.width,
                (unsigned)tr->tim.height);
            s_lastSrvFailFrame = ctx.frame;
        }
        return false;
    }

    SubmitSpriteUI_SRV_UV(ctx,
                          vx,
                          vy,
                          vs,
                          drawSrv,
                          x,
                          y,
                          (float)tpl.w,
                          (float)tpl.h,
                          0.0f,
                          0.0f,
                          1.0f,
                          1.0f,
                          r,
                          g,
                          b,
                          a,
                          layer,
                          order);
    return true;
}

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
                           int layer) {
    return DrawPsxSpriteTemplateOrdered(
        ctx, vx, vy, vs, x, y, tpl, r, g, b, a, layer, 0);
}

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
                                 int order) {
    if (!ctx.renderer || !ctx.resources) {
        return false;
    }

    const int bpp = PsxBppFromAttr(tpl.attr);
    TextureResource* tr = ctx.resources->FindTextureByTimHeader(
        bpp,
        (int16_t)tpl.texX_hw,
        (int16_t)tpl.texY_px,
        (uint32_t)tpl.w,
        (uint32_t)tpl.h,
        (int16_t)tpl.clutX_px,
        (int16_t)tpl.clutY_px);
    if (!tr) {
        tr = FindLoadedTimTextureByTemplate(ctx, tpl, nullptr);
    }
    if (!tr) {
        return DrawPsxSpriteTemplateScaledFallback(
            ctx,
            vx,
            vy,
            vs,
            x,
            y,
            tpl,
            scaleX,
            scaleY,
            r,
            g,
            b,
            a,
            layer,
            order);
    }
    ID3D11ShaderResourceView* drawSrv =
        TryTextureReplacementForTim(ctx, tpl, tr->tim);
    if (!drawSrv && !tr->srv && !tr->tim.rgba.empty()) {
        tr->srv = ctx.renderer->CreateTexture(
            tr->tim.rgba.data(), tr->tim.width, tr->tim.height);
    }
    if (!drawSrv) {
        drawSrv = tr->srv;
    }
    if (!drawSrv) {
        return DrawPsxSpriteTemplateScaledFallback(
            ctx,
            vx,
            vy,
            vs,
            x,
            y,
            tpl,
            scaleX,
            scaleY,
            r,
            g,
            b,
            a,
            layer,
            order);
    }

    const float baseW = (float)tpl.w;
    const float baseH = (float)tpl.h;

    const float cx = x + baseW * 0.5f;
    const float cy = y + baseH * 0.5f;

    const float sx = std::fabs(scaleX);
    const float sy = std::fabs(scaleY);

    const float w = baseW * sx;
    const float h = baseH * sy;

    const float x0 = cx - w * 0.5f;
    const float y0 = cy - h * 0.5f;

    const bool flipX = scaleX < 0.0f;
    const float u0 = flipX ? 1.0f : 0.0f;
    const float u1 = flipX ? 0.0f : 1.0f;
    SubmitSpriteUI_SRV_UV(ctx,
                          vx,
                          vy,
                          vs,
                          drawSrv,
                          x0,
                          y0,
                          w,
                          h,
                          u0,
                          0.0f,
                          u1,
                          1.0f,
                          r,
                          g,
                          b,
                          a,
                          layer,
                          order);
    return true;
}

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
                                  int layer) {
    if (!ctx.renderer || !ctx.resources) {
        return false;
    }
    if (drawWidthPx <= 0 || baseTpl.w == 0 || baseTpl.h == 0) {
        return false;
    }

    PsxSpriteTemplate tpl = baseTpl;
    tpl.clutY_px =
        (uint16_t)(tpl.clutY_px + (uint16_t)(std::max)(clutYOffset, 0));

    if (EnsureUiAtlasHasTemplate(ctx, tpl)) {
        const uint16_t tpage = UiTplTpageFromTemplate(tpl);
        UiVramAtlasCache& cache = GetUiVramAtlasCache(ctx);
        const int bpp = PsxBppFromAttr(tpl.attr);
        const uint16_t clut =
            (bpp == 4 || bpp == 8)
                ? PackPsxClutCba(tpl.clutX_px, tpl.clutY_px)
                : 0u;
        ID3D11ShaderResourceView* srv =
            (bpp == 4 || bpp == 8)
                ? cache.atlas.GetTpageSRV(tpage, clut, ctx.renderer)
                : cache.atlas.GetTpageSRV(tpage);
        if (srv) {
            const int pxFactor = bpp == 8 ? 2 : (bpp == 16 ? 1 : 4);
            const int baseHW = (tpage & 0xF) * 64;
            const int baseY = ((tpage >> 4) & 1) * 256;
            const int texBasePxX = ((int)tpl.texX_hw - baseHW) * pxFactor;
            const int texBasePxY = (int)tpl.texY_px - baseY;
            float u0 = 0.0f;
            float v0 = 0.0f;
            float u1 = 0.0f;
            float v1 = 0.0f;
            BuildTextureSubrectUv((float)(texBasePxX + uOffsetPx),
                                  (float)texBasePxY,
                                  (float)drawWidthPx,
                                  (float)tpl.h,
                                  256.0f,
                                  256.0f,
                                  u0,
                                  v0,
                                  u1,
                                  v1);
            ID3D11ShaderResourceView* replacementSrv =
                TryTextureReplacementForAtlasRect(ctx,
                                                  tpl,
                                                  cache.atlas,
                                                  tpage,
                                                  clut,
                                                  texBasePxX + uOffsetPx,
                                                  texBasePxY,
                                                  drawWidthPx,
                                                  static_cast<int>(tpl.h),
                                                  uOffsetPx,
                                                  0);
            if (replacementSrv) {
                srv = replacementSrv;
                u0 = 0.0f;
                v0 = 0.0f;
                u1 = 1.0f;
                v1 = 1.0f;
            }
            SubmitSpriteUI_SRV_UV(ctx,
                                  vx,
                                  vy,
                                  vs,
                                  srv,
                                  x,
                                  y,
                                  (float)drawWidthPx,
                                  (float)tpl.h,
                                  u0,
                                  v0,
                                  u1,
                                  v1,
                                  r,
                                  g,
                                  b,
                                  a,
                                  layer);
            return true;
        }
    }

    const int bpp = PsxBppFromAttr(tpl.attr);
    TextureResource* tr = ctx.resources->FindTextureByTimHeader(
        bpp,
        (int16_t)tpl.texX_hw,
        (int16_t)tpl.texY_px,
        (uint32_t)tpl.w,
        (uint32_t)tpl.h,
        (int16_t)tpl.clutX_px,
        (int16_t)tpl.clutY_px);
    if (!tr) {
        tr = FindLoadedTimTextureByTemplate(ctx, tpl);
    }
    if (!tr) {
        return false;
    }
    ID3D11ShaderResourceView* replacementSrv =
        TryTextureReplacementForTimSubrect(ctx,
                                           tpl,
                                           tr->tim,
                                           uOffsetPx,
                                           0,
                                           drawWidthPx,
                                           static_cast<int>(tpl.h));
    if (replacementSrv) {
        SubmitSpriteUI_SRV_UV(ctx,
                              vx,
                              vy,
                              vs,
                              replacementSrv,
                              x,
                              y,
                              (float)drawWidthPx,
                              (float)tpl.h,
                              0.0f,
                              0.0f,
                              1.0f,
                              1.0f,
                              r,
                              g,
                              b,
                              a,
                              layer);
        return true;
    }

    if (!tr->srv && !tr->tim.rgba.empty()) {
        tr->srv = ctx.renderer->CreateTexture(
            tr->tim.rgba.data(), tr->tim.width, tr->tim.height);
    }
    if (!tr->srv) {
        return false;
    }

    const float texW = (float)tpl.w;
    const float texH = (float)tpl.h;
    float u0 = 0.0f;
    float v0 = 0.0f;
    float u1 = 0.0f;
    float v1 = 0.0f;
    BuildTextureSubrectUv((float)uOffsetPx,
                          0.0f,
                          (float)drawWidthPx,
                          texH,
                          texW,
                          texH,
                          u0,
                          v0,
                          u1,
                          v1);
    u0 = std::clamp(u0, 0.0f, 1.0f);
    u1 = std::clamp(u1, 0.0f, 1.0f);
    v0 = std::clamp(v0, 0.0f, 1.0f);
    v1 = std::clamp(v1, 0.0f, 1.0f);
    SubmitSpriteUI_SRV_UV(ctx,
                          vx,
                          vy,
                          vs,
                          tr->srv,
                          x,
                          y,
                          (float)drawWidthPx,
                          (float)tpl.h,
                          u0,
                          v0,
                          u1,
                          v1,
                          r,
                          g,
                          b,
                          a,
                          layer);
    return true;
}

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
                                        int order) {
    if (!ctx.renderer || !ctx.resources) {
        return false;
    }
    if (drawWidthPx <= 0 || baseTpl.w == 0 || baseTpl.h == 0 ||
        scaleX == 0.0f || scaleY == 0.0f) {
        return false;
    }

    PsxSpriteTemplate tpl = baseTpl;
    tpl.clutY_px =
        (uint16_t)(tpl.clutY_px + (uint16_t)(std::max)(clutYOffset, 0));

    const float drawW = static_cast<float>(drawWidthPx) * std::fabs(scaleX);
    const float drawH = static_cast<float>(tpl.h) * std::fabs(scaleY);
    const bool flipX = scaleX < 0.0f;

    if (EnsureUiAtlasHasTemplate(ctx, tpl)) {
        const uint16_t tpage = UiTplTpageFromTemplate(tpl);
        UiVramAtlasCache& cache = GetUiVramAtlasCache(ctx);
        const int bpp = PsxBppFromAttr(tpl.attr);
        const uint16_t clut =
            (bpp == 4 || bpp == 8)
                ? PackPsxClutCba(tpl.clutX_px, tpl.clutY_px)
                : 0u;
        ID3D11ShaderResourceView* srv =
            (bpp == 4 || bpp == 8)
                ? cache.atlas.GetTpageSRV(tpage, clut, ctx.renderer)
                : cache.atlas.GetTpageSRV(tpage);
        if (srv) {
            const int pxFactor = bpp == 8 ? 2 : (bpp == 16 ? 1 : 4);
            const int baseHW = (tpage & 0xF) * 64;
            const int baseY = ((tpage >> 4) & 1) * 256;
            const int texBasePxX = ((int)tpl.texX_hw - baseHW) * pxFactor;
            const int texBasePxY = (int)tpl.texY_px - baseY;
            float u0 = 0.0f;
            float v0 = 0.0f;
            float u1 = 0.0f;
            float v1 = 0.0f;
            BuildTextureSubrectUv((float)(texBasePxX + uOffsetPx),
                                  (float)texBasePxY,
                                  (float)drawWidthPx,
                                  (float)tpl.h,
                                  256.0f,
                                  256.0f,
                                  u0,
                                  v0,
                                  u1,
                                  v1);
            ID3D11ShaderResourceView* replacementSrv =
                TryTextureReplacementForAtlasRect(ctx,
                                                  tpl,
                                                  cache.atlas,
                                                  tpage,
                                                  clut,
                                                  texBasePxX + uOffsetPx,
                                                  texBasePxY,
                                                  drawWidthPx,
                                                  static_cast<int>(tpl.h),
                                                  uOffsetPx,
                                                  0);
            if (replacementSrv) {
                srv = replacementSrv;
                u0 = 0.0f;
                v0 = 0.0f;
                u1 = 1.0f;
                v1 = 1.0f;
            }
            if (flipX) {
                std::swap(u0, u1);
            }
            SubmitSpriteUI_SRV_UV(ctx,
                                  vx,
                                  vy,
                                  vs,
                                  srv,
                                  x,
                                  y,
                                  drawW,
                                  drawH,
                                  u0,
                                  v0,
                                  u1,
                                  v1,
                                  r,
                                  g,
                                  b,
                                  a,
                                  layer,
                                  order);
            return true;
        }
    }

    const int bpp = PsxBppFromAttr(tpl.attr);
    TextureResource* tr = ctx.resources->FindTextureByTimHeader(
        bpp,
        (int16_t)tpl.texX_hw,
        (int16_t)tpl.texY_px,
        (uint32_t)tpl.w,
        (uint32_t)tpl.h,
        (int16_t)tpl.clutX_px,
        (int16_t)tpl.clutY_px);
    if (!tr) {
        tr = FindLoadedTimTextureByTemplate(ctx, tpl);
    }
    if (!tr) {
        return false;
    }
    ID3D11ShaderResourceView* replacementSrv =
        TryTextureReplacementForTimSubrect(ctx,
                                           tpl,
                                           tr->tim,
                                           uOffsetPx,
                                           0,
                                           drawWidthPx,
                                           static_cast<int>(tpl.h));
    if (replacementSrv) {
        SubmitSpriteUI_SRV_UV(ctx,
                              vx,
                              vy,
                              vs,
                              replacementSrv,
                              x,
                              y,
                              drawW,
                              drawH,
                              flipX ? 1.0f : 0.0f,
                              0.0f,
                              flipX ? 0.0f : 1.0f,
                              1.0f,
                              r,
                              g,
                              b,
                              a,
                              layer,
                              order);
        return true;
    }

    if (!tr->srv && !tr->tim.rgba.empty()) {
        tr->srv = ctx.renderer->CreateTexture(
            tr->tim.rgba.data(), tr->tim.width, tr->tim.height);
    }
    if (!tr->srv) {
        return false;
    }

    const float texW = (float)tpl.w;
    const float texH = (float)tpl.h;
    float u0 = 0.0f;
    float v0 = 0.0f;
    float u1 = 0.0f;
    float v1 = 0.0f;
    BuildTextureSubrectUv((float)uOffsetPx,
                          0.0f,
                          (float)drawWidthPx,
                          texH,
                          texW,
                          texH,
                          u0,
                          v0,
                          u1,
                          v1);
    u0 = std::clamp(u0, 0.0f, 1.0f);
    u1 = std::clamp(u1, 0.0f, 1.0f);
    v0 = std::clamp(v0, 0.0f, 1.0f);
    v1 = std::clamp(v1, 0.0f, 1.0f);
    if (flipX) {
        std::swap(u0, u1);
    }
    SubmitSpriteUI_SRV_UV(ctx,
                          vx,
                          vy,
                          vs,
                          tr->srv,
                          x,
                          y,
                          drawW,
                          drawH,
                          u0,
                          v0,
                          u1,
                          v1,
                          r,
                          g,
                          b,
                          a,
                          layer,
                          order);
    return true;
}

}  // namespace PrPsxSpriteTemplateRender
