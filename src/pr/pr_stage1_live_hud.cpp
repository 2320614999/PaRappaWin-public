#include "pr_stage1_live_hud.h"

#include "pr_game_context.h"
#include "pr_stage1_hud_presentation_direct.h"
#include "pr_stage1_scene1_movie1_direct.h"
#include "pr_stage1_p2_scorer_hud.h"
#include "pr_psx_sprite_template_render.h"
#include "pr_scn1.h"
#include "d3d11_renderer.h"

#include <algorithm>
#include <array>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

namespace PrStage1LiveHud {
namespace {

using PrPsxSpriteTemplateRender::PsxSpriteTemplate;

static std::string Stage1HudReadFileToString(const std::filesystem::path& path) {
    std::ifstream fs(path, std::ios::binary);
    if (!fs.is_open()) {
        return {};
    }
    std::ostringstream ss;
    ss << fs.rdbuf();
    return ss.str();
}

static size_t Stage1HudSkipWs(const std::string& s, size_t p) {
    while (p < s.size() && (s[p] == ' ' || s[p] == '\t' || s[p] == '\n' || s[p] == '\r')) {
        p++;
    }
    return p;
}

static size_t Stage1HudFindKey(const std::string& s, size_t start, const std::string& key) {
    const std::string needle = "\"" + key + "\"";
    size_t pos = s.find(needle, start);
    if (pos == std::string::npos) {
        return std::string::npos;
    }
    pos += needle.size();
    pos = Stage1HudSkipWs(s, pos);
    if (pos < s.size() && s[pos] == ':') {
        pos++;
    }
    return Stage1HudSkipWs(s, pos);
}

static int Stage1HudParseInt(const std::string& s, size_t& p) {
    p = Stage1HudSkipWs(s, p);
    int sign = 1;
    if (p < s.size() && s[p] == '-') {
        sign = -1;
        p++;
    }
    int val = 0;
    while (p < s.size() && s[p] >= '0' && s[p] <= '9') {
        val = val * 10 + (s[p] - '0');
        p++;
    }
    return sign * val;
}

static bool Stage1HudParseQuotedHexU32(const std::string& s, size_t& p, uint32_t& outVal) {
    p = Stage1HudSkipWs(s, p);
    if (p >= s.size() || s[p] != '"') {
        return false;
    }
    p++;
    p = Stage1HudSkipWs(s, p);
    if (p + 2 >= s.size() || s[p] != '0' || (s[p + 1] != 'x' && s[p + 1] != 'X')) {
        return false;
    }
    p += 2;

    uint32_t v = 0;
    int digits = 0;
    while (p < s.size()) {
        const char c = s[p];
        int d = -1;
        if (c >= '0' && c <= '9') {
            d = c - '0';
        } else if (c >= 'a' && c <= 'f') {
            d = 10 + (c - 'a');
        } else if (c >= 'A' && c <= 'F') {
            d = 10 + (c - 'A');
        } else {
            break;
        }
        v = (v << 4) | (uint32_t)d;
        digits++;
        p++;
    }
    while (p < s.size() && s[p] != '"') {
        p++;
    }
    if (p < s.size() && s[p] == '"') {
        p++;
    }
    if (digits <= 0) {
        return false;
    }
    outVal = v;
    return true;
}

static bool Stage1HudParseSpriteTemplateByAddr(const std::string& json,
                                               uint32_t addr,
                                               PsxSpriteTemplate& outTpl) {
    char needle[64];
    std::snprintf(needle, sizeof(needle), "\"addr\": \"0x%08X\"", (unsigned)addr);
    const std::string needleStr(needle);
    size_t search = 0;
    while (true) {
        const size_t pos = json.find(needleStr, search);
        if (pos == std::string::npos) {
            return false;
        }
        search = pos + needleStr.size();

        const size_t brace = json.rfind('{', pos);
        const size_t endBrace = json.find('}', pos);
        if (brace == std::string::npos || endBrace == std::string::npos || endBrace <= brace) {
            continue;
        }
        const std::string obj = json.substr(brace, endBrace - brace + 1);

        size_t pa = Stage1HudFindKey(obj, 0, "attr");
        size_t ptx = Stage1HudFindKey(obj, 0, "texX_hw");
        size_t pty = Stage1HudFindKey(obj, 0, "texY_px");
        size_t pw = Stage1HudFindKey(obj, 0, "w");
        size_t ph = Stage1HudFindKey(obj, 0, "h");
        size_t pcx = Stage1HudFindKey(obj, 0, "clutX_px");
        size_t pcy = Stage1HudFindKey(obj, 0, "clutY_px");
        if (pa == std::string::npos || ptx == std::string::npos || pty == std::string::npos ||
            pw == std::string::npos || ph == std::string::npos || pcx == std::string::npos ||
            pcy == std::string::npos) {
            continue;
        }

        uint32_t attr = 0;
        if (!Stage1HudParseQuotedHexU32(obj, pa, attr)) {
            continue;
        }

        const int texXHw = Stage1HudParseInt(obj, ptx);
        const int texYPx = Stage1HudParseInt(obj, pty);
        const int w = Stage1HudParseInt(obj, pw);
        const int h = Stage1HudParseInt(obj, ph);
        const int clutXPx = Stage1HudParseInt(obj, pcx);
        const int clutYPx = Stage1HudParseInt(obj, pcy);
        if (texXHw < 0 || texYPx < 0 || w < 0 || h < 0 || clutXPx < 0 || clutYPx < 0) {
            continue;
        }

        outTpl.attr = attr;
        outTpl.texX_hw = (uint16_t)texXHw;
        outTpl.texY_px = (uint16_t)texYPx;
        outTpl.w = (uint16_t)w;
        outTpl.h = (uint16_t)h;
        outTpl.clutX_px = (uint16_t)clutXPx;
        outTpl.clutY_px = (uint16_t)clutYPx;
        return true;
    }
}

struct Stage1HudPresentationJsonCache {
    bool loaded = false;
    bool ok = false;
    std::filesystem::path loadedRoot;
    PrStage1HudPresentationDirectTemplates numericTemplates{};
};

static const Stage1HudPresentationJsonCache& GetStage1HudPresentationJsonCache(
    const std::filesystem::path& dataRoot) {
    static Stage1HudPresentationJsonCache s_cache;
    if (!s_cache.loaded || s_cache.loadedRoot != dataRoot) {
        s_cache = Stage1HudPresentationJsonCache{};
        s_cache.loaded = true;
        s_cache.loadedRoot = dataRoot;

        const std::filesystem::path path =
            dataRoot / "win" / "ex" / "json" / "psx_stage1_hud_presentation_constants.json";
        const std::string json = Stage1HudReadFileToString(path);
        if (json.empty()) {
            return s_cache;
        }

        size_t dimPos = Stage1HudFindKey(json, 0, "right_rank_dim_clut_y");
        if (dimPos != std::string::npos) {
            s_cache.numericTemplates.rightRankDimClutY =
                (uint16_t)Stage1HudParseInt(json, dimPos);
        }

        bool ok = true;
        ok &= Stage1HudParseSpriteTemplateByAddr(json, 0x8004E740u, s_cache.numericTemplates.scoreLabel);
        ok &= Stage1HudParseSpriteTemplateByAddr(json, 0x8004E750u, s_cache.numericTemplates.scoreDigit);
        ok &= Stage1HudParseSpriteTemplateByAddr(
            json,
            0x8004E6B0u,
            s_cache.numericTemplates.transitionPromptRandom);
        ok &= Stage1HudParseSpriteTemplateByAddr(
            json,
            0x8004E6C0u,
            s_cache.numericTemplates.transitionPromptManual);
        ok &= Stage1HudParseSpriteTemplateByAddr(json, 0x8004E6D0u, s_cache.numericTemplates.rightRankHighlight);
        ok &= Stage1HudParseSpriteTemplateByAddr(json, 0x8004E6E0u, s_cache.numericTemplates.rightRankRows[0]);
        ok &= Stage1HudParseSpriteTemplateByAddr(json, 0x8004E6F0u, s_cache.numericTemplates.rightRankRows[1]);
        ok &= Stage1HudParseSpriteTemplateByAddr(json, 0x8004E700u, s_cache.numericTemplates.rightRankRows[2]);
        ok &= Stage1HudParseSpriteTemplateByAddr(json, 0x8004E710u, s_cache.numericTemplates.rightRankRows[3]);
        ok &= Stage1HudParseSpriteTemplateByAddr(
            json,
            0x8004E720u,
            s_cache.numericTemplates.shortPromptLeft);
        ok &= Stage1HudParseSpriteTemplateByAddr(
            json,
            0x8004E730u,
            s_cache.numericTemplates.shortPromptRight);
        ok &= Stage1HudParseSpriteTemplateByAddr(json, 0x8004E760u, s_cache.numericTemplates.topLessonLabel);
        for (size_t i = 0; i < s_cache.numericTemplates.topLessonNumbers.size(); i++) {
            ok &= Stage1HudParseSpriteTemplateByAddr(
                json,
                0x8004E770u + 0x10u * (uint32_t)i,
                s_cache.numericTemplates.topLessonNumbers[i]);
        }
        s_cache.ok = ok;
    }
    return s_cache;
}

static constexpr float kStage1WidescreenHudOuterOffsetPsx = 40.0f;

static bool IsStage1WidescreenViewport(const D3D11Renderer* renderer) {
    if (!renderer || renderer->GetHeight() <= 0) {
        return false;
    }
    const float winW = (float)renderer->GetWidth();
    const float winH = (float)renderer->GetHeight();
    return winW > winH * (4.0f / 3.0f) + 1.0f;
}

static PrStage1HudPresentationDirectState BuildStage1HudPresentationDirectState(
    const Stage1ResolvedHudNumericStatusRouteState& numericRoute,
    float scorePanelOffsetX,
    float rightRankPanelOffsetX) {
    PrStage1HudPresentationDirectState out{};
    out.scoreDisplayValue = numericRoute.scoreDisplayState.scoreDisplayValue;
    out.rightRankActiveRow = numericRoute.rightRankState.rightRankActiveRow;
    out.rightRankBlinkTargetRow = numericRoute.rightRankState.rightRankBlinkTargetRow;
    out.rightRankBlinkEnabled = numericRoute.rightRankState.rightRankBlinkEnabled;
    out.rightRankFlag0200Pulse = numericRoute.rightRankState.rightRankFlag0200Pulse;
    out.rightRankTransitionAnim18E =
        numericRoute.rightRankState.rightRankTransitionAnim18E;
    out.topLessonPairChangeVisible =
        numericRoute.topLessonPairState.topLessonPairChangeVisible;
    out.topLessonPairLessonId =
        numericRoute.topLessonPairState.topLessonPairLessonId;
    out.transitionState916D0 = numericRoute.transitionState916D0;
    out.highLayoutMode = numericRoute.highLayoutMode;
    out.scorePanelOffsetX = scorePanelOffsetX;
    out.rightRankPanelOffsetX = rightRankPanelOffsetX;
    return out;
}

static Stage1HudBee4RawActionRouteState BuildStage1HudBee4RawActionsFromNumericRoute(
    PrGameContext& ctx,
    const Stage1ResolvedHudNumericStatusRouteState& numericRoute,
    PrStage1HudPresentationDirectBee4ActionCarrier& outCarrier) {
    outCarrier = PrStage1HudPresentationDirectBee4ActionCarrier{};

    Stage1HudBee4RawActionRouteState routeState{};
    routeState.numericRouteAvailable = numericRoute.available;
    routeState.steadyGameplayGateActive = numericRoute.steadyGameplayGateActive;
    if (!numericRoute.available || !numericRoute.steadyGameplayGateActive) {
        return routeState;
    }

    const bool widescreen = IsStage1WidescreenViewport(ctx.renderer);
    const float scorePanelOffsetX =
        widescreen ? -kStage1WidescreenHudOuterOffsetPsx : 0.0f;
    const float rightRankPanelOffsetX =
        widescreen ? kStage1WidescreenHudOuterOffsetPsx : 0.0f;

    outCarrier = PrStage1HudPresentationDirectBuildSub8001DF24Bee4RawActions(
        BuildStage1HudPresentationDirectState(
            numericRoute,
            scorePanelOffsetX,
            rightRankPanelOffsetX));
    routeState.available = outCarrier.actionCount > 0 || outCarrier.gapCount > 0;
    return routeState;
}

static void DrawStage1LiveHudNumericStatusRoute(const Stage1HudPaintContext& hudCtx,
                                                const Stage1ResolvedHudNumericStatusRouteState& numericRoute) {
    if (!hudCtx.game.renderer || !numericRoute.available || !numericRoute.steadyGameplayGateActive) {
        return;
    }

    const Stage1HudPresentationJsonCache& psx =
        GetStage1HudPresentationJsonCache(hudCtx.game.dataRoot);
    if (!psx.ok) {
        return;
    }

    const bool widescreen = IsStage1WidescreenViewport(hudCtx.game.renderer);
    const float scorePanelOffsetX =
        widescreen ? -kStage1WidescreenHudOuterOffsetPsx : 0.0f;
    const float rightRankPanelOffsetX =
        widescreen ? kStage1WidescreenHudOuterOffsetPsx : 0.0f;

    PrStage1HudPresentationDirectDrawNumericStatus(
        hudCtx.game,
        hudCtx.vx,
        hudCtx.vy,
        hudCtx.vs,
        psx.numericTemplates,
        BuildStage1HudPresentationDirectState(
            numericRoute,
            scorePanelOffsetX,
            rightRankPanelOffsetX));
    PrStage1P2ScorerHud::Draw(hudCtx.game,
                              hudCtx.vx,
                              hudCtx.vy,
                              hudCtx.vs,
                              numericRoute.highLayoutMode,
                              scorePanelOffsetX,
                              numericRoute.scoreDisplayState.scoreDisplayValue,
                              psx.numericTemplates);
}

static void DrawStage1CommonLyricsFrameDirect(
    const Stage1HudPaintContext& hudCtx,
    const PrStage1CommonLyricsFrameSub8001C864& frame) {
    if (!hudCtx.game.renderer) {
        return;
    }

    if (!frame.valid) {
        return;
    }

    const size_t count =
        (std::min)((size_t)frame.submitCount, frame.submits.size());
    for (size_t i = 0; i < count; ++i) {
        const PrStage1CommonLyricsFrameSubmitSub8001C864& submit =
            frame.submits[i];
        if (!submit.valid || submit.templateAddr == 0u) {
            continue;
        }

        const PrStage1Scene1Movie1Direct::Movie1PsxTemplateDesc desc =
            PrStage1Scene1Movie1Direct::ResolveMovie1PsxTemplateDesc(
                submit.templateAddr);
        if (!desc.valid) {
            continue;
        }
        const PsxSpriteTemplate tpl{
            desc.attr,
            desc.texX,
            desc.texY,
            desc.w,
            desc.h,
            desc.clutX,
            desc.clutY,
        };
        const int rendererOrder = static_cast<int>(submit.callOrder) + 1;

        const bool drewFromVram =
            PrPsxSpriteTemplateRender::DrawPsxSpriteTemplateViaUiAtlas(
                hudCtx.game,
                hudCtx.vx,
                hudCtx.vy,
                hudCtx.vs,
                (float)submit.screenX,
                (float)submit.screenY,
                tpl,
                1.0f,
                1.0f,
                1.0f,
                1.0f,
                submit.layer,
                rendererOrder);
        if (drewFromVram) {
            continue;
        }

        (void)PrPsxSpriteTemplateRender::DrawPsxSpriteTemplateOrdered(
            hudCtx.game,
            hudCtx.vx,
            hudCtx.vy,
            hudCtx.vs,
            (float)submit.screenX,
            (float)submit.screenY,
            tpl,
            1.0f,
            1.0f,
            1.0f,
            1.0f,
            submit.layer,
            rendererOrder);
    }
}

static void DrawStage1LiveHudTextPath(const Stage1HudPaintContext& hudCtx,
                                      const Stage1ResolvedHudTextRouteState& textRoute) {
    const Stage1ResolvedHudTextVisual* textVisual =
        textRoute.active.available ? &textRoute.active : nullptr;
    if (!hudCtx.game.renderer || !textVisual) {
        return;
    }

    const bool hasText = textVisual->hasText && !textVisual->rawText.empty();
    const bool steadyScriptFrame =
        textVisual->pathKind ==
        Stage1ResolvedHudTextPathKind::SteadyGameplayScriptText;
    if (steadyScriptFrame) {
        return;
    }

    if (!hasText && !textVisual->commonLyricsFrameSub8001C864.valid) {
        return;
    }

    DrawStage1CommonLyricsFrameDirect(
        hudCtx,
        textVisual->commonLyricsFrameSub8001C864);
}

} // namespace

bool BuildStage1RuntimeHudBee4RawActions(
    PrGameContext& ctx,
    PrStage1HudPresentationDirectBee4ActionCarrier& outCarrier,
    Stage1HudBee4RawActionRouteState* outRouteState) {
    Stage1ResolvedHudTextRouteState textRoute{};
    Stage1ResolvedHudNumericStatusRouteState numericRoute{};
    (void)PrScn1::GetStage1HudRouteSnapshot(ctx, textRoute, numericRoute);

    const Stage1HudBee4RawActionRouteState routeState =
        BuildStage1HudBee4RawActionsFromNumericRoute(
            ctx,
            numericRoute,
            outCarrier);
    if (outRouteState) {
        *outRouteState = routeState;
    }
    return routeState.available;
}

void DrawStage1LiveSnapshotHud(const Stage1HudPaintContext& hudCtx,
                               const Stage1ResolvedHudTextRouteState& textRoute,
                               const Stage1ResolvedHudNumericStatusRouteState& numericRoute) {
    if (!textRoute.anyAvailable && !numericRoute.available) {
        return;
    }

    DrawStage1LiveHudNumericStatusRoute(hudCtx, numericRoute);
    DrawStage1LiveHudTextPath(hudCtx, textRoute);
}

static void CalcStage1PsxViewport(const D3D11Renderer* renderer, float& outX, float& outY, float& outScale) {
    outX = 0.0f;
    outY = 0.0f;
    outScale = 1.0f;
    if (!renderer) {
        return;
    }

    const float winW = (float)renderer->GetWidth();
    const float winH = (float)renderer->GetHeight();
    const float baseW = 320.0f;
    const float baseH = 240.0f;
    const float fitX = (baseW > 0.0f) ? (winW / baseW) : 1.0f;
    const float fitY = (baseH > 0.0f) ? (winH / baseH) : 1.0f;
    outScale = (std::min)(fitX, fitY);
    outX = (winW - baseW * outScale) * 0.5f;
    outY = (winH - baseH * outScale) * 0.5f;
}

static void RenderStage1ResolvedSnapshotPanels(PrGameContext& ctx,
                                               float vx,
                                               float vy,
                                               float vs) {
    const Stage1HudPaintContext liveHudCtx{ ctx, vx, vy, vs };
    Stage1ResolvedHudTextRouteState textRoute{};
    Stage1ResolvedHudNumericStatusRouteState numericRoute{};
    (void)PrScn1::GetStage1HudRouteSnapshot(ctx, textRoute, numericRoute);
    DrawStage1LiveSnapshotHud(
        liveHudCtx,
        textRoute,
        numericRoute);
}

void DrawStage1RuntimeHud(PrGameContext& ctx) {
    if (!ctx.renderer) {
        return;
    }

    float psxVx = 0.0f;
    float psxVy = 0.0f;
    float psxVs = 1.0f;
    CalcStage1PsxViewport(ctx.renderer, psxVx, psxVy, psxVs);
    RenderStage1ResolvedSnapshotPanels(ctx, psxVx, psxVy, psxVs);
}

void DrawStage1RuntimeHudRouteOnly(PrGameContext& ctx) {
    if (!ctx.renderer) {
        return;
    }

    float psxVx = 0.0f;
    float psxVy = 0.0f;
    float psxVs = 1.0f;
    CalcStage1PsxViewport(ctx.renderer, psxVx, psxVy, psxVs);

    const Stage1HudPaintContext liveHudCtx{ ctx, psxVx, psxVy, psxVs };
    Stage1ResolvedHudTextRouteState textRoute{};
    Stage1ResolvedHudNumericStatusRouteState numericRoute{};
    (void)PrScn1::GetStage1HudRouteSnapshot(ctx, textRoute, numericRoute);
    DrawStage1LiveSnapshotHud(
        liveHudCtx,
        textRoute,
        numericRoute);
}

void DrawStage1RuntimeTextRouteOnly(PrGameContext& ctx) {
    if (!ctx.renderer) {
        return;
    }

    float psxVx = 0.0f;
    float psxVy = 0.0f;
    float psxVs = 1.0f;
    CalcStage1PsxViewport(ctx.renderer, psxVx, psxVy, psxVs);

    const Stage1HudPaintContext liveHudCtx{ ctx, psxVx, psxVy, psxVs };
    Stage1ResolvedHudTextRouteState textRoute{};
    Stage1ResolvedHudNumericStatusRouteState numericRoute{};
    (void)PrScn1::GetStage1CommonLyricsHudRouteSnapshot(ctx, textRoute);
    DrawStage1LiveSnapshotHud(
        liveHudCtx,
        textRoute,
        numericRoute);
}

} // namespace PrStage1LiveHud
