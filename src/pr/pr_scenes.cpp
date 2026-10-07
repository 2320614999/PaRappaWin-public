#include "pr_scn1.h"
#include "pr_scn2.h"
#include "pr_scn3.h"
#include "pr_scn5.h"
#include "pr_scn6.h"
#include "pr_scn7.h"
#include "pr_scn8.h"
#include "pr_scn9.h"

#include "logger.h"
#include "pr_game_context.h"
#include "pr_main.h"
#include "pr_scene_entry_direct.h"
#include "pr_scene_def.h"
#include "pr_event.h"
#include "pr_stage_runner.h"
#include "pr_transition.h"
#include "pr_sqevs1.h"
#include "pr_stage1_live_hud.h"
#include "pr_stage1_overlay_parser.h"
#include "pr_stage1_scorer_host.h"
#include "pr_stage1_scorer_host_deps.h"
#include "pr_stage1_scorer_host_direct.h"
#include "pr_stage1_scorer_direct.h"
#include "pr_stage1_rating_presentation_direct.h"
#include "pr_stage1_hud_text_bridge_direct.h"
#include "pr_stage1_hd_subtitles.h"
#include "pr_stage1_overlay_script_text_direct.h"
#include "pr_stage1_runtime_slots_direct.h"
#include "pr_stage1_xa_cd_direct.h"
#include "pr_stage1_cd_stop_execution_direct.h"
#include "pr_stage1_lifecycle_direct.h"
#include "pr_stage1_lifecycle_executor_direct.h"
#include "pr_stage1_lifecycle_host_adapter_801c81ec.h"
#include "pr_stage1_save_ui_direct.h"
#include "pr_stage1_save_ui_host_bridge_direct.h"
#include "pr_stage_payload_bank_direct.h"
#include "pr_stage1_scene1_movie1_direct.h"
#include "pr_stage1_scene1_frame_driver_direct.h"
#include "pr_stage1_scene1_render_router_direct.h"
#include "pr_stage1_scene1_draw_backend.h"
#include "pr_stage1_movie_segment_direct.h"
#include "pr_stage1_movie_text_direct.h"
#include "pr_stage1_movie_text_outer_loop_direct.h"
#include "pr_movie_subtitles.h"
#include "pr_stage_event_direct.h"
#include "pr_stage_runner_direct.h"
#include "pr_stage2_product_runtime.h"
#include "pr_ss0_scene0_runtime_direct.h"
#include "pr_ss0_mdec_vlc_direct.h"
#include "pr_ss0_mdec_output_direct.h"
#include "pr_tmd_renderer.h"
#include "pr_mime.h"
#include "pr_sfx.h"
#include "pr_pad.h"
#include "pr_psx_pad_direct.h"
#include "pr_psx_vblank_callback_direct.h"
#include "pr_vtext.h"
#include "pr_ui_overlay.h"
#include "d3d11_renderer.h"
#include "resource_manager.h"
#include "str_player.h"
#include "xa1_player.h"

#include <array>
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <filesystem>
#include <sstream>
#include <string>
#include <system_error>
#include <unordered_set>
#include <vector>

namespace PrScn1 {
static void ApplyStage1RunnerDirectInitLifecycle(PrGameContext& ctx);
}

namespace {

namespace LifecycleHost801C81EC = PrStage1LifecycleHostAdapter801C81EC;

static void SubmitTitleCopyrightText(PrGameContext& ctx, float vx, float vy, float vs, float py, int layer) {
    const wchar_t* line1 =
        L"Unofficial PC Port | \u963F\u72FC. \u00A91997 Sony Computer Entertainment Inc. \u00A9Rodney A. Greenblat / Interlink";
    const wchar_t* line2 =
        L"All trademarks are the property of Sony Computer Entertainment America Inc. & respective owners.";

    PrUiOverlay::SubmitSystemTextBlockW(ctx, vx, vy, vs,
                                       "title_copyright",
                                       0.0f, py, 320.0f,
                                       line1, line2,
                                       0.0f, 0.0f, 0.0f, 0.5f,
                                       layer);
}

struct TitleOverlayPos {
    float x = 0.0f;
    float y = 0.0f;
    bool valid = false;
};

static std::string ReadFileToString(const std::filesystem::path& path) {
    std::ifstream fs(path, std::ios::binary);
    if (!fs.is_open()) return {};
    std::ostringstream ss;
    ss << fs.rdbuf();
    return ss.str();
}

static size_t SkipWs(const std::string& s, size_t p) {
    while (p < s.size() && (s[p] == ' ' || s[p] == '\t' || s[p] == '\n' || s[p] == '\r')) {
        p++;
    }
    return p;
}

static int ParseInt(const std::string& s, size_t& p) {
    p = SkipWs(s, p);
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

static size_t FindKey(const std::string& s, size_t start, const std::string& key) {
    std::string needle = "\"" + key + "\"";
    size_t pos = s.find(needle, start);
    if (pos == std::string::npos) return std::string::npos;
    pos += needle.size();
    pos = SkipWs(s, pos);
    if (pos < s.size() && s[pos] == ':') pos++;
    return SkipWs(s, pos);
}

static bool ParseXYObject(const std::string& json, size_t objStart, float& outX, float& outY) {
    if (objStart == std::string::npos) return false;
    size_t brace = json.find('{', objStart);
    if (brace == std::string::npos) return false;
    size_t endBrace = json.find('}', brace);
    if (endBrace == std::string::npos) return false;
    std::string obj = json.substr(brace, endBrace - brace + 1);

    size_t px = FindKey(obj, 0, "x");
    size_t py = FindKey(obj, 0, "y");
    if (px == std::string::npos || py == std::string::npos) return false;
    int x = ParseInt(obj, px);
    int y = ParseInt(obj, py);
    outX = (float)x;
    outY = (float)y;
    return true;
}

static void LoadTitleOverlayPositionsOnce(const std::filesystem::path& dataRoot,
                                         TitleOverlayPos& outStart,
                                         TitleOverlayPos& outMenu,
                                         TitleOverlayPos& outTm) {
    static bool s_loaded = false;
    static std::filesystem::path s_loadedRoot;
    static TitleOverlayPos s_cachedStart;
    static TitleOverlayPos s_cachedMenu;
    static TitleOverlayPos s_cachedTm;

    if (!s_loaded || s_loadedRoot != dataRoot) {
        s_loaded = true;
        s_loadedRoot = dataRoot;

        s_cachedStart = TitleOverlayPos{};
        s_cachedMenu = TitleOverlayPos{};
        s_cachedTm = TitleOverlayPos{};

        std::filesystem::path p = dataRoot / "win" / "ex" / "json" / "psx_title_overlays.json";
        std::string json = ReadFileToString(p);
        if (!json.empty()) {
            float x = 0.0f, y = 0.0f;
            if (ParseXYObject(json, FindKey(json, 0, "start"), x, y)) {
                s_cachedStart.x = x;
                s_cachedStart.y = y;
                s_cachedStart.valid = true;
            }
            if (ParseXYObject(json, FindKey(json, 0, "menu"), x, y)) {
                s_cachedMenu.x = x;
                s_cachedMenu.y = y;
                s_cachedMenu.valid = true;
            }
            if (ParseXYObject(json, FindKey(json, 0, "tm"), x, y)) {
                s_cachedTm.x = x;
                s_cachedTm.y = y;
                s_cachedTm.valid = true;
            }
        }
    }

    outStart = s_cachedStart;
    outMenu = s_cachedMenu;
    outTm = s_cachedTm;
}

} // namespace


// ========== Stage Loop Helpers ==========

// Legacy non-Stage1 pre-run blocker. Scene1 now polls 80026B94(4,0) through
// the 801C81EC direct action path; do not route Stage1 back through this shell.
// Check forced scene-loop exits before running the stage runner.
// 1000 = continue, 999 = blocked this frame, -1 = exit to caller.
static int CheckLegacyStageLoopExitConditions(PrGameContext& ctx) {
    constexpr int kStageLoopContinue = 1000;
    constexpr int kStageLoopBlocked = 999;
    static bool s_ev4Requested = false;

    if (PrSS0Scene0RuntimeDirect::RuntimeEnabled() &&
        (ctx.currentScene == PrSceneId::Scene0 ||
         ctx.currentScene == PrSceneId::Scene1)) {
        s_ev4Requested = false;
        ctx.debugEsc_Ev4Exit = false;
        PrEvent::ClearDispatcherResidueForDirectCutover();
        return kStageLoopContinue;
    }

    if (ctx.debugEsc_Ev4Exit) {
        s_ev4Requested = true;
    }

    // Only keep pre-run blockers here. `word_800916D0 in {1,2}` is a PSX
    // post-run scene-loop branch and must not be consumed early here.
    // Condition 1: ev=4 pause/exit menu (Esc request, CROSS=1 continue, CIRCLE=2 exit).
    if (s_ev4Requested) {
        if (!PrEvent::IsDispatcherRunning()) {
            const int r = PrEvent::ConsumeDispatcherResult();
            if (r >= 0) {
                Log::Printf("StageLoop: ev4 done result=%d", r);
                s_ev4Requested = false;
                if (r == 2) {
                    ctx.sceneExitReason = 2;
                    return -1;
                }
                return kStageLoopContinue;
            }

            if (!PrEvent::StartDispatcherEx(4, nullptr, ctx)) {
                Log::Printf("StageLoop: failed to start ev=4 dispatcher");
                s_ev4Requested = false;
                return kStageLoopContinue;
            }
        }

        // Dispatcher is active: block the runner for this frame.
        return kStageLoopBlocked;
    }

    return kStageLoopContinue;
}

// 使用 StageRunner 骨架替代原 stub
// 返回 0=继续, 1=通关/结束
static int PrStageRunner_Run(PrGameContext& ctx, int sceneId) {
    PrStageRunner& runner = GetStageRunner();

    // 首次调用时初始化
    if (runner.GetState() == StageRunnerState::Idle || runner.GetSceneId() != sceneId) {
        runner.Init(sceneId, ctx);
    }

    return runner.Update(ctx);
}

// 场景颜色表 (R, G, B)
static const float kSceneColors[9][3] = {
    {0.10f, 0.12f, 0.18f}, // Scene0: 深蓝灰 (菜单)
    {0.20f, 0.08f, 0.08f}, // Scene1: 深红 (Stage1)
    {0.08f, 0.18f, 0.08f}, // Scene2: 深绿 (Stage2)
    {0.18f, 0.15f, 0.05f}, // Scene3: 深黄 (Stage3)
    {0.15f, 0.08f, 0.18f}, // Scene5: 深紫 (Stage5)
    {0.08f, 0.15f, 0.18f}, // Scene6: 深青 (Stage6)
    {0.18f, 0.10f, 0.05f}, // Scene7: 深橙 (Stage7)
    {0.12f, 0.12f, 0.12f}, // Scene8: 灰色 (Credits)
    {0.15f, 0.15f, 0.08f}, // Scene9: 黄灰 (Bonus)
};

// 纹理名黑名单（排除字库/字体sheet）
static bool IsBlacklistedTextureName(const std::string& name) {
    static const char* kBlacklist[] = {
        "japan", "kanji", "font", "moji", "ascii", "hiragana", "katakana",
        "alpha", "table", "msg", "text", "char", "letter", "kana"
    };
    for (const char* pattern : kBlacklist) {
        if (name.find(pattern) != std::string::npos) return true;
    }
    return false;
}

// 计算纹理的不透明像素占比（用于排除大面积透明的图标/字库sheet）
static float CalcOpaqueRatio(const TimImage& tim) {
    if (tim.rgba.empty() || tim.width <= 0 || tim.height <= 0) return 0.0f;
 
    // 使用采样而非全量扫描：避免纹理很多时切场景卡顿
    const int w = tim.width;
    const int h = tim.height;
    const int kMaxSamples = 64;
    const int samplesX = (w < kMaxSamples) ? w : kMaxSamples;
    const int samplesY = (h < kMaxSamples) ? h : kMaxSamples;
    const int stepX = (std::max)(1, w / (samplesX > 0 ? samplesX : 1));
    const int stepY = (std::max)(1, h / (samplesY > 0 ? samplesY : 1));

    size_t total = 0;
    size_t opaque = 0;
    for (int y = 0; y < h; y += stepY) {
        for (int x = 0; x < w; x += stepX) {
            const size_t idx = ((size_t)y * (size_t)w + (size_t)x) * 4;
            if (tim.rgba[idx + 3] > 16) {
                ++opaque;
            }
            ++total;
        }
    }
    if (!total) return 0.0f;
    return (float)opaque / (float)total;
}

static bool TryRenderResourceBackground(PrGameContext& ctx) {
    // 开关检查：默认关闭避免误选字库
    if (!ctx.debugShowBgTexture) return false;
    if (!ctx.renderer) return false;
    if (!ctx.resources) return false;

    if (ctx.debugBgTexIndex >= 0) {
        static PrSceneId s_lastSceneForce = (PrSceneId)0xFF;
        static uint32_t s_lastGenerationForce = 0;
        static int s_lastIndexForce = -2;
        static std::string s_nameForce;
        static int s_wForce = 0;
        static int s_hForce = 0;

        const uint32_t curGen = ctx.resources->GetGeneration();
        const bool needReselect = (ctx.currentScene != s_lastSceneForce) ||
                                 (curGen != s_lastGenerationForce) ||
                                 (ctx.debugBgTexIndex != s_lastIndexForce);

        if (needReselect) {
            s_lastSceneForce = ctx.currentScene;
            s_lastGenerationForce = curGen;
            s_lastIndexForce = ctx.debugBgTexIndex;
            s_nameForce.clear();
            s_wForce = 0;
            s_hForce = 0;

            const std::vector<std::string> names = ctx.resources->GetTextureNames();
            const int idx = ctx.debugBgTexIndex;
            if (idx >= 0 && idx < (int)names.size()) {
                s_nameForce = names[(size_t)idx];
                if (TextureResource* res = ctx.resources->GetTexture(s_nameForce)) {
                    s_wForce = res->tim.width;
                    s_hForce = res->tim.height;
                }
                Log::Printf("SceneBg forced idx=%d name='%s' (%dx%d)", idx, s_nameForce.c_str(), s_wForce, s_hForce);
            } else {
                Log::Printf("SceneBg forced idx=%d out of range (count=%d)", idx, (int)names.size());
            }
        }

        if (s_nameForce.empty()) return false;
        if (s_wForce <= 0 || s_hForce <= 0) return false;

        ID3D11ShaderResourceView* srv = ctx.resources->GetTextureView(s_nameForce);
        if (!srv) return false;

        const float texW = (float)s_wForce;
        const float texH = (float)s_hForce;
        const float winW = (float)ctx.renderer->GetWidth();
        const float winH = (float)ctx.renderer->GetHeight();
        const float fitX = winW / texW;
        const float fitY = winH / texH;
        const float scale = (std::min)(fitX, fitY);
        const float w = texW * scale;
        const float h = texH * scale;
        const float x = (winW - w) * 0.5f;
        const float y = (winH - h) * 0.5f;
        ctx.renderer->DrawRect(x, y, w, h, 0.55f, 0.55f, 0.55f, 1.0f);
        ctx.renderer->DrawSprite(srv, x, y, w, h);
        return true;
    }

    static PrSceneId s_lastScene = (PrSceneId)0xFF;
    static uint32_t s_lastGeneration = 0;
    static std::string s_bestName;
    static int s_bestW = 0;
    static int s_bestH = 0;

    const uint32_t curGen = ctx.resources->GetGeneration();
    const bool needReselect = (ctx.currentScene != s_lastScene) || (curGen != s_lastGeneration);

    if (needReselect) {
        s_lastScene = ctx.currentScene;
        s_lastGeneration = curGen;
        s_bestName.clear();
        s_bestW = 0;
        s_bestH = 0;

        const float winW = (float)ctx.renderer->GetWidth();
        const float winH = (float)ctx.renderer->GetHeight();
        const float winAspect = (winH > 0.0f) ? (winW / winH) : (4.0f / 3.0f);

        int bestArea = -1;
        float bestAspectDiff = 1e9f;
        float bestOpaqueRatio = 0.0f;

        const std::vector<std::string> names = ctx.resources->GetTextureNames();
        for (const std::string& name : names) {
            TextureResource* res = ctx.resources->GetTexture(name);
            if (!res) continue;
            const int w = res->tim.width;
            const int h = res->tim.height;

            // 黑名单过滤
            if (IsBlacklistedTextureName(name)) continue;

            // 尺寸过滤：至少 160x120
            if (w < 160 || h < 120) continue;

            const float aspect = (float)w / (float)h;
            const float aspectDiff = std::abs(aspect - winAspect);
            // 比例过滤：与窗口比例差距不超过 0.8
            if (aspectDiff > 0.8f) continue;

            // 透明度过滤：不透明占比至少 30%
            const float opaqueRatio = CalcOpaqueRatio(res->tim);
            if (opaqueRatio < 0.3f) continue;

            const int area = w * h;
            // 优先选面积最大、比例最接近、不透明度最高的
            if (area > bestArea ||
                (area == bestArea && aspectDiff < bestAspectDiff) ||
                (area == bestArea && aspectDiff == bestAspectDiff && opaqueRatio > bestOpaqueRatio)) {
                bestArea = area;
                bestAspectDiff = aspectDiff;
                bestOpaqueRatio = opaqueRatio;
                s_bestName = name;
                s_bestW = w;
                s_bestH = h;
            }
        }

        if (!s_bestName.empty()) {
            Log::Printf("SceneBg tex='%s' (%dx%d) opaque=%.1f%%", 
                        s_bestName.c_str(), s_bestW, s_bestH, bestOpaqueRatio * 100.0f);
        } else {
            Log::Printf("SceneBg tex=(none matched filters)");
        }
    }

    if (s_bestName.empty()) return false;
    if (s_bestW <= 0 || s_bestH <= 0) return false;

    ID3D11ShaderResourceView* srv = ctx.resources->GetTextureView(s_bestName);
    if (!srv) return false;

    const float texW = (float)s_bestW;
    const float texH = (float)s_bestH;
    const float winW = (float)ctx.renderer->GetWidth();
    const float winH = (float)ctx.renderer->GetHeight();
    const float fitX = winW / texW;
    const float fitY = winH / texH;
    const float scale = (std::min)(fitX, fitY);
    const float w = texW * scale;
    const float h = texH * scale;
    const float x = (winW - w) * 0.5f;
    const float y = (winH - h) * 0.5f;

    ctx.renderer->DrawRect(x, y, w, h, 0.55f, 0.55f, 0.55f, 1.0f);
    ctx.renderer->DrawSprite(srv, x, y, w, h);
    return true;
}

static void CalcPs1Viewport(const D3D11Renderer* renderer, float& outX, float& outY, float& outScale) {
    outX = 0.0f;
    outY = 0.0f;
    outScale = 1.0f;
    if (!renderer) return;

    const float winW = (float)renderer->GetWidth();
    const float winH = (float)renderer->GetHeight();
    const float baseW = 320.0f;
    const float baseH = 240.0f;
    const float fitX = (baseW > 0.0f) ? (winW / baseW) : 1.0f;
    const float fitY = (baseH > 0.0f) ? (winH / baseH) : 1.0f;
    const float scale = (std::min)(fitX, fitY);
    outScale = scale;
    outX = (winW - baseW * scale) * 0.5f;
    outY = (winH - baseH * scale) * 0.5f;
}

static bool DrawResourceTextureByIndex(PrGameContext& ctx, int index, bool fullViewport) {
    if (!ctx.renderer) return false;
    if (!ctx.resources) return false;
    if (index < 0) return false;

    const std::vector<std::string> names = ctx.resources->GetTextureNames();
    if (index < 0 || index >= (int)names.size()) return false;

    const std::string& name = names[(size_t)index];
    TextureResource* res = ctx.resources->GetTexture(name);
    if (!res) return false;

    ID3D11ShaderResourceView* srv = ctx.resources->GetTextureView(name);
    if (!srv) return false;

    float vx = 0.0f, vy = 0.0f, vs = 1.0f;
    CalcPs1Viewport(ctx.renderer, vx, vy, vs);

    const float baseW = 320.0f;
    const float baseH = 240.0f;
    const float viewW = baseW * vs;
    const float viewH = baseH * vs;

    float x = vx;
    float y = vy;
    float w = viewW;
    float h = viewH;

    if (!fullViewport) {
        const float texW = (float)res->tim.width;
        const float texH = (float)res->tim.height;
        const bool looksFullscreen = (res->tim.width >= 300 && res->tim.height >= 200);
        if (!looksFullscreen) {
            w = texW * vs;
            h = texH * vs;
            x = vx + (viewW - w) * 0.5f;
            y = vy + (viewH - h) * 0.5f;
        }
    }

    ctx.renderer->DrawSprite(srv, x, y, w, h);
    return true;
}

// No scene may draw the former colored debug shell.  During a result/stage
// movie the original STR frame is the only valid presentation; when no
// original TMD/STR is available, fail closed to black instead of inventing a
// replacement UI that could be mistaken for native S0 artwork.
static bool RenderNativeStrFrame(PrGameContext& ctx) {
    if (PrSS0Scene0RuntimeDirect::RuntimeEnabled() &&
        (ctx.currentScene == PrSceneId::Scene0 ||
         ctx.currentScene == PrSceneId::Scene1)) {
        // Direct SS0 supplies its own MDEC surface; the process Host STR
        // projection is never a valid visual fallback for these scenes.
        return false;
    }
    if (!ctx.renderer || !ctx.strPlayer || !ctx.strPlayer->IsPlaying() ||
        ctx.strPlayer->IsVideoFinished()) {
        return false;
    }

    float vx = 0.0f;
    float vy = 0.0f;
    float vs = 1.0f;
    CalcPs1Viewport(ctx.renderer, vx, vy, vs);
    ctx.strPlayer->RenderToRect(vx, vy, 320.0f * vs, 240.0f * vs);
    return true;
}

static void RenderNativeSceneFallback(PrGameContext& ctx, int sceneIndex) {
    if (!ctx.renderer) {
        return;
    }

    const float winW = static_cast<float>(ctx.renderer->GetWidth());
    const float winH = static_cast<float>(ctx.renderer->GetHeight());
    ctx.renderer->DrawRect(0.0f, 0.0f, winW, winH, 0.0f, 0.0f, 0.0f, 1.0f);

    static bool logged[9] = {};
    if (sceneIndex >= 0 && sceneIndex < 9 && !logged[sceneIndex]) {
        logged[sceneIndex] = true;
        Log::Printf(
            "Native scene render blocked: scene=%d has no accepted STR/TMD presentation; debug shell disabled",
            sceneIndex);
    }
}

// Scene0 is owned exclusively by PrSS0Scene0RuntimeDirect via pr_main.cpp.
// The unreachable legacy PrScn0 state machine was retired after direct cutover.
namespace PrScn1 {
static bool s_xaStarted = false;
static bool s_stage1RetryStageRecordRestartPending801C81EC = false;
static PrStage1Scene1Movie1Direct::Movie1RuntimeState s_movie1Runtime;
static PrStage1MovieTextDirect::Movie1TextRuntime s_movie1TextRuntime;
static PrStage1MovieTextDirect::Movie1TextWindowTickResult s_movie1TextWindowTickResult;
static PrStage1MovieTextOuterLoopDirect::MovieTextOuterLoopRuntimeSub801C455C
    s_movie1TextOuterLoopRuntime;
static PrStage1MovieTextOuterLoopDirect::MovieTextOuterLoopStepResultSub801C455C
    s_movie1TextOuterLoopLastStep;
static bool s_stage1IntroTransitionActive = false;
static bool s_stage1IntroTransitionRenderActive = false;
static bool s_stage1IntroTransitionConsumerResetPending = false;
static int s_stage1IntroTransitionFrame = 0;
static bool s_stage1XaCurrentPhysicalGetlocPProbeDisabled = false;

// Stage1 MOVIE1 uses the same PSX MDEC(1) stream contract as Scene0, but its
// CD/XA ring is owned by COMOD1 and therefore cannot reuse Scene0's decoder
// storage.  Keep a small independent presentation owner here so the native
// role-grid/frame templates can sit over the decoded picture without routing
// video through the Host StrPlayer projection.
struct Stage1DirectMovie1MdecRuntime {
    ID3D11ShaderResourceView* texture = nullptr;
    D3D11Renderer* renderer = nullptr;
    bool ready = false;
    bool successLogged = false;
    uint32_t sourceFrameNo = 0xFFFFFFFFu;
    uint32_t frameIndex = 0u;
    uint32_t decodedFrameCount = 0u;
    uint32_t decodeFailureCount = 0u;
    std::vector<uint8_t> vlcOutput{};
    std::vector<uint8_t> imageOutput{};
    std::vector<uint32_t> rgbaOutput{};
};

static Stage1DirectMovie1MdecRuntime s_stage1DirectMovie1Mdec;

static uint16_t ReadLe16Stage1Movie1(const uint8_t* bytes) {
    return static_cast<uint16_t>(bytes[0]) |
           static_cast<uint16_t>(static_cast<uint16_t>(bytes[1]) << 8u);
}

static void WriteLe32Stage1Movie1(uint8_t* bytes, uint32_t value) {
    bytes[0] = static_cast<uint8_t>(value & 0xFFu);
    bytes[1] = static_cast<uint8_t>((value >> 8u) & 0xFFu);
    bytes[2] = static_cast<uint8_t>((value >> 16u) & 0xFFu);
    bytes[3] = static_cast<uint8_t>((value >> 24u) & 0xFFu);
}

static void DestroyStage1DirectMovie1MdecTexture(PrGameContext& ctx) {
    if (s_stage1DirectMovie1Mdec.texture != nullptr &&
        s_stage1DirectMovie1Mdec.renderer != nullptr) {
        s_stage1DirectMovie1Mdec.renderer->DestroyTexture(
            s_stage1DirectMovie1Mdec.texture);
    }
    (void)ctx;
    s_stage1DirectMovie1Mdec.texture = nullptr;
    s_stage1DirectMovie1Mdec.renderer = nullptr;
}

static void ResetStage1DirectMovie1Mdec(PrGameContext& ctx) {
    DestroyStage1DirectMovie1MdecTexture(ctx);
    s_stage1DirectMovie1Mdec.ready = false;
    s_stage1DirectMovie1Mdec.successLogged = false;
    s_stage1DirectMovie1Mdec.sourceFrameNo = 0xFFFFFFFFu;
    s_stage1DirectMovie1Mdec.frameIndex = 0u;
    s_stage1DirectMovie1Mdec.decodedFrameCount = 0u;
    s_stage1DirectMovie1Mdec.decodeFailureCount = 0u;
    s_stage1DirectMovie1Mdec.vlcOutput.clear();
    s_stage1DirectMovie1Mdec.imageOutput.clear();
    s_stage1DirectMovie1Mdec.rgbaOutput.clear();
}

static bool DecodeStage1DirectMovie1Frame(
    PrGameContext& ctx,
    const StrVideoFrame& frame) {
    constexpr uint16_t kMovie1Width = 256u;
    constexpr uint16_t kMovie1Height = 144u;
    constexpr uint32_t kVlcCapacityBytes = 0x24000u;
    if (frame.width != kMovie1Width || frame.height != kMovie1Height ||
        frame.bitstream.empty()) {
        return false;
    }

    // StrParser keeps only the declared frameSize.  The PSX decoder, however,
    // reads the complete 2016-byte Form2 chunk stream and relies on the zero
    // tail of the final chunk while refilling its 32-bit reservoir.  Restore
    // that sector padding before invoking the direct VLC routine.
    const size_t paddedBytes =
        ((frame.bitstream.size() + 2015u) / 2016u) * 2016u;
    std::vector<uint8_t> paddedInput(paddedBytes, 0u);
    std::copy(frame.bitstream.begin(), frame.bitstream.end(),
              paddedInput.begin());

    s_stage1DirectMovie1Mdec.vlcOutput.resize(kVlcCapacityBytes);
    const auto vlc = PrSS0MdecVlcDirect::
        ExecuteDecDCTvlc2ResetSize80047B30(
            paddedInput.data(), paddedInput.size(),
            s_stage1DirectMovie1Mdec.vlcOutput.data(),
            s_stage1DirectMovie1Mdec.vlcOutput.size());
    if (!vlc.known || !vlc.executed || vlc.returnValue != 0 ||
        !vlc.frameComplete || !vlc.trailingFe00PaddingWritten ||
        vlc.trailingFe00Halfwords != 65u) {
        if (s_stage1DirectMovie1Mdec.decodeFailureCount == 0u) {
            Log::Printf(
                "Scene1::MOVIE1 direct MDEC VLC detail known=%u executed=%u ret=%d frameComplete=%u trailing=%u count=%u inHalf=%u outBytes=%u inputBytes=%u",
                vlc.known ? 1u : 0u,
                vlc.executed ? 1u : 0u,
                vlc.returnValue,
                vlc.frameComplete ? 1u : 0u,
                vlc.trailingFe00PaddingWritten ? 1u : 0u,
                vlc.trailingFe00Halfwords,
                vlc.inputHalfwordsConsumed,
                vlc.outputBytesWritten,
                static_cast<unsigned>(frame.bitstream.size()));
        }
        ++s_stage1DirectMovie1Mdec.decodeFailureCount;
        return false;
    }

    // 80047558 rewrites the two VLC header halfwords into the MDEC DMA
    // command before 8004780C consumes the RLE payload at offset four.
    // Preserve the original frame-code/magic pair and set the current SCUS
    // 15bpp command mode bits exactly as the Scene0 direct path does.
    const uint32_t vlcCommandBefore =
        static_cast<uint32_t>(s_stage1DirectMovie1Mdec.vlcOutput[0]) |
        (static_cast<uint32_t>(s_stage1DirectMovie1Mdec.vlcOutput[1]) << 8u) |
        (static_cast<uint32_t>(s_stage1DirectMovie1Mdec.vlcOutput[2]) << 16u) |
        (static_cast<uint32_t>(s_stage1DirectMovie1Mdec.vlcOutput[3]) << 24u);
    WriteLe32Stage1Movie1(
        s_stage1DirectMovie1Mdec.vlcOutput.data(),
        vlcCommandBefore | 0x0A000000u);

    const uint32_t macroblockColumns =
        (static_cast<uint32_t>(kMovie1Width) + 15u) >> 4u;
    const uint32_t macroblockRows =
        (static_cast<uint32_t>(kMovie1Height) + 15u) >> 4u;
    const size_t imageBytes = static_cast<size_t>(macroblockColumns) *
                              macroblockRows * 512u;
    s_stage1DirectMovie1Mdec.imageOutput.resize(imageBytes);
    const auto mdec = PrSS0MdecOutputDirect::ExecuteMdec15bppCurrentScus(
        s_stage1DirectMovie1Mdec.vlcOutput.data(),
        vlc.outputBytesWritten,
        kMovie1Width,
        kMovie1Height,
        s_stage1DirectMovie1Mdec.imageOutput.data(),
        s_stage1DirectMovie1Mdec.imageOutput.size());
    if (!mdec.known || !mdec.executed ||
        mdec.outputBytesWritten != imageBytes ||
        !mdec.dma1VerticalMacroblockLayout) {
        if (s_stage1DirectMovie1Mdec.decodeFailureCount == 0u) {
            Log::Printf(
                "Scene1::MOVIE1 direct MDEC output detail known=%u executed=%u outBytes=%u expected=%u vertical=%u command=%08X words=%u",
                mdec.known ? 1u : 0u,
                mdec.executed ? 1u : 0u,
                mdec.outputBytesWritten,
                static_cast<unsigned>(imageBytes),
                mdec.dma1VerticalMacroblockLayout ? 1u : 0u,
                mdec.commandWord,
                mdec.inputWordCount);
        }
        ++s_stage1DirectMovie1Mdec.decodeFailureCount;
        return false;
    }
    if (ctx.renderer == nullptr) {
        ++s_stage1DirectMovie1Mdec.decodeFailureCount;
        return false;
    }

    s_stage1DirectMovie1Mdec.rgbaOutput.resize(
        static_cast<size_t>(kMovie1Width) * kMovie1Height);
    // 80044D64 consumes the DMA1 result in vertical-strip order: one 16-px
    // strip is contiguous across all macroblock rows, then the next strip.
    // Reading the buffer as mbY-major tiles produces the block-scrambled
    // picture seen when this ordering is lost.
    for (uint32_t strip = 0u; strip < macroblockColumns; ++strip) {
        const size_t stripOffset =
            static_cast<size_t>(strip) * 16u * macroblockRows * 16u * 2u;
        for (uint32_t y = 0u; y < kMovie1Height; ++y) {
            for (uint32_t x = 0u; x < 16u; ++x) {
                const uint32_t dstX = strip * 16u + x;
                if (dstX >= kMovie1Width) {
                    continue;
                }
                const uint16_t pixel = ReadLe16Stage1Movie1(
                    s_stage1DirectMovie1Mdec.imageOutput.data() +
                    stripOffset +
                    (static_cast<size_t>(y) * 16u + x) * 2u);
                const uint32_t red5 = (pixel >> 0u) & 0x1Fu;
                const uint32_t green5 = (pixel >> 5u) & 0x1Fu;
                const uint32_t blue5 = (pixel >> 10u) & 0x1Fu;
                const uint32_t red = (red5 << 3u) | (red5 >> 2u);
                const uint32_t green = (green5 << 3u) | (green5 >> 2u);
                const uint32_t blue = (blue5 << 3u) | (blue5 >> 2u);
                s_stage1DirectMovie1Mdec.rgbaOutput[
                    static_cast<size_t>(y) * kMovie1Width + dstX] =
                    red | (green << 8u) | (blue << 16u) | 0xFF000000u;
            }
        }
    }

    if (s_stage1DirectMovie1Mdec.texture == nullptr ||
        s_stage1DirectMovie1Mdec.renderer != ctx.renderer) {
        DestroyStage1DirectMovie1MdecTexture(ctx);
        s_stage1DirectMovie1Mdec.texture = ctx.renderer->CreateTexture(
            s_stage1DirectMovie1Mdec.rgbaOutput.data(),
            static_cast<int>(kMovie1Width),
            static_cast<int>(kMovie1Height));
        s_stage1DirectMovie1Mdec.renderer = ctx.renderer;
    } else if (!ctx.renderer->TryUpdateTexture(
                   s_stage1DirectMovie1Mdec.texture,
                   s_stage1DirectMovie1Mdec.rgbaOutput.data(),
                   static_cast<int>(kMovie1Width),
                   static_cast<int>(kMovie1Height))) {
        ++s_stage1DirectMovie1Mdec.decodeFailureCount;
        return false;
    }
    if (s_stage1DirectMovie1Mdec.texture == nullptr) {
        ++s_stage1DirectMovie1Mdec.decodeFailureCount;
        return false;
    }
    s_stage1DirectMovie1Mdec.ready = true;
    s_stage1DirectMovie1Mdec.frameIndex = frame.frameNo;
    s_stage1DirectMovie1Mdec.sourceFrameNo = frame.frameNo;
    ++s_stage1DirectMovie1Mdec.decodedFrameCount;
    if (!s_stage1DirectMovie1Mdec.successLogged) {
        s_stage1DirectMovie1Mdec.successLogged = true;
        Log::Printf(
            "Scene1::MOVIE1 direct MDEC frame ready frame=%u bytes=%u decoded=%u texture=1",
            frame.frameNo,
            static_cast<unsigned>(frame.bitstream.size()),
            s_stage1DirectMovie1Mdec.decodedFrameCount);
    }
    return true;
}

static void UpdateStage1DirectMovie1MdecFromStrPlayer(PrGameContext& ctx) {
    if (!ctx.strPlayer || !ctx.strPlayer->IsPlaying() ||
        ctx.strPlayer->IsVideoFinished()) {
        return;
    }
    const StrVideoFrame* frame = ctx.strPlayer->GetCurrentVideoFrame();
    if (frame == nullptr ||
        frame->frameNo == s_stage1DirectMovie1Mdec.sourceFrameNo) {
        return;
    }
    if (!DecodeStage1DirectMovie1Frame(ctx, *frame) &&
        s_stage1DirectMovie1Mdec.decodeFailureCount == 1u) {
        Log::Printf(
            "Scene1::MOVIE1 direct MDEC frame decode blocked frame=%u bytes=%u geometry=%ux%u",
            frame->frameNo,
            static_cast<unsigned>(frame->bitstream.size()),
            static_cast<unsigned>(frame->width),
            static_cast<unsigned>(frame->height));
    }
}

static void ResetStage1NumericRuntimeState() {
    ResetStage1ScorerHostNumericRuntimeState();
}

static void ResetStage1RetryStageRecordRestart801C81EC() {
    s_stage1RetryStageRecordRestartPending801C81EC = false;
}

static void ArmStage1RetryStageRecordRestart801C81EC() {
    s_stage1RetryStageRecordRestartPending801C81EC = true;
}

static PrStage1OverlayScriptTextDirectRuntime s_stage1OverlayScriptTextDirect{};
static PrStage1RuntimeSlotsDirectRuntime s_stage1RuntimeSlotPlayer;
static PrStageEventDirectStage1Runtime s_stage1EventStreamRuntime9094;
static PrStage1LifecycleDirect::Globals801CA3BC s_stage1LifecycleGlobals801CA3BC{};
static PrStage1LifecycleDirect::SceneEntry801C7284 s_stage1LifecycleScene801C7284{};
static PrStage1LifecycleDirect::Runtime801C81EC s_stage1LifecycleRuntime801C81EC{};

static PrStage1LifecycleExecutorDirect::State801C81EC
    s_stage1LifecycleExecutor801C81EC{};
static uint32_t s_stage1LifecycleDword801D3040 = 0u;

static constexpr int16_t kStage1CurrentLiveSubstate1LookbackPageCount = 2;
static constexpr uint16_t kStage1CurrentLiveSubstate1FlagWord = 0x0042;
static constexpr uint32_t kStage1SceneBaseRowFlag = 0x00000020u;
static constexpr uint32_t kStage1PairFlagPaByCoolness = 0x00010000u;
static constexpr uint32_t kStage1PairFlagOnByMode = 0x00020000u;
static constexpr uint32_t kStage1PerformanceTodFlag = 0x00040000u;

Stage1EventStreamFrameUpdate9094Result RunStage1EventStreamFrameUpdate9094(
    const PrStage1OverlayData& data,
    int tick96,
    uint32_t queryFrame,
    bool sameFrameCtxKnown,
    const PrStageRunnerDirectContext801C9094& sameFrameCtx,
    bool sameFrameGlobalsKnown,
    const PrStageRunnerDirectGlobals801C9094& sameFrameGlobals,
    bool allowSameQueryRefresh,
    bool clearTerminalTailPulse);
static void ResetStage1LifecycleActionExecutor801C81EC();
static void ApplyStage1RunnerDirectInitLifecycle(PrGameContext& ctx);
void CaptureStage1AcceptedProducerReplayBackup1635C(
    uint32_t prevGrade92F40,
    const Stage1NumericRuntimeState::AcceptedProducerReplayBufferRuntime& replay,
    Stage1AcceptedProducerReplayBackupRuntime& backup);
static bool PrimeStage1RuntimeSlotPlayer(PrGameContext& ctx);
static bool ResolveStage1RuntimeSlotsSnapshot(PrGameContext& ctx,
                                              PrStage1RuntimeSlotsSnapshot& out,
                                              bool allowTransitionFreeze = false,
                                              bool allowStoppedRuntimeFreeze = false);
static void ApplyStage1TerminalCleanupRenderCtxOverride801C7A60(
    PrStage1RuntimeSlotsSnapshot& snapshot);
static bool IsStage1TerminalCleanupNumericStatusGateCleared801C7A60();
// The short "freeze on skip before subtitle box closes" sits in front of
// sub_80020308; the direct MOVIE1 runtime owns its frame countdown.
static constexpr int kStage1IntroTransitionMaxLife = 76;
static constexpr uint32_t kStage1StageLoopLoaderOffset801C7A60 = 0x9Cu;

static uint8_t GetStage1CommonLyricsLanguageIndex(const PrGameContext& ctx) {
    return PrStage1MovieTextDirect::ResolveLanguageIndex(ctx.languageIndex);
}

static int16_t ReadStage1SceneEntryRecordS16FromOpaque04(uint32_t opaque04,
                                                         bool highWord) {
    const uint32_t value =
        highWord ? ((opaque04 >> 16u) & 0xFFFFu) : (opaque04 & 0xFFFFu);
    return static_cast<int16_t>(value);
}

static PrStageRunnerDirectInitInput801C79E4
BuildStage1RunnerDirectInitInput801C79E4() {
    PrStageRunnerDirectInitInput801C79E4 input{};
    for (const PrStage1LifecycleDirect::Scene1LoaderRecord801C7284& loader :
         s_stage1LifecycleScene801C7284.loaders) {
        if (loader.psxOffsetFromSceneEntry !=
            kStage1StageLoopLoaderOffset801C7A60) {
            continue;
        }

        const auto& record = loader.movieSegmentRecord;
        if (!loader.present || !record.known || !record.opaqueA1Plus04Known) {
            return input;
        }

        input.stageXaChannelSeg04 =
            ReadStage1SceneEntryRecordS16FromOpaque04(record.opaqueA1Plus04,
                                                      false);
        input.stageBgmArgSeg06 =
            ReadStage1SceneEntryRecordS16FromOpaque04(record.opaqueA1Plus04,
                                                      true);
        return input;
    }
    return input;
}

static uint8_t ResolveStage1StageRecordXaChannel8001A4D0() {
    return static_cast<uint8_t>(
        BuildStage1RunnerDirectInitInput801C79E4().stageXaChannelSeg04);
}

static PrStage1RuntimeSlotsDirectGlobalOptionsCarrier
BuildStage1GlobalOptionsCarrier801C7A60(const PrGameContext& ctx) {
    return PrStage1RuntimeSlotsDirectBuildGlobalOptionsCarrierFromWord800916DC(
        true,
        static_cast<uint16_t>(ctx.subtitleFlag != 0 ? 1u : 0u));
}

static void ApplyStage1RunnerDirectInitLifecycle(PrGameContext& ctx) {
    const PrStageRunnerDirectInitResult801C79E4 init =
        PrStageRunnerDirectApplySub801C79E4(
            BuildStage1RunnerDirectInitInput801C79E4());
    if (init.audioResetBarrier26FA4) {
        PrSfx::ApplySharedAudioResetBarrier26FA4();
        if (ctx.xa1Player) {
            ctx.xa1Player->NotifyAudioEngineReset();
        }
    }
    if (init.setBgm8001A478) {
        const PrStage1XaCdDirectBgmVolumeRequest8001A4A4 volume =
            PrStage1XaCdDirectApplySub8001A478(init.bgmArg);
        if (volume.requestSetVolume && ctx.xa1Player) {
            ctx.xa1Player->SetVolume(volume.normalizedVolume);
        }
    }
    if (init.setFilter8001A654) {
        const PrStage1XaCdDirectSetFilter13Request setFilter =
            PrStage1XaCdDirectApplySub8001A654(
                ctx.stage1XaCdDirect,
                init.filterChannel);
        if (ctx.xa1Player && setFilter.requestSetFilter) {
            ctx.xa1Player->SetFilter(setFilter.file, setFilter.channel);
        }
    }
    if (init.resetScorer80014344) {
        ResetStage1NumericRuntimeState();
    }
    if (init.resetEventRuntime80024E98) {
        const PrStage1RuntimeSlotsDirectGlobalOptionsCarrier globalOptions =
            BuildStage1GlobalOptionsCarrier801C7A60(ctx);
        const PrStageEventDirectResetAction80024F8C resetAction =
            PrStage1RuntimeSlotsDirectResetEventAndSlots80024E98(
                s_stage1EventStreamRuntime9094,
                s_stage1RuntimeSlotPlayer,
                globalOptions);
        (void)ApplyStage1ScorerHostResetAction80024F8C(
            s_stage1NumericRuntime,
            resetAction);
    }
    if (init.bindRunnerContext80024FC0) {
        (void)ApplyStage1ScorerHostScoreMirror80024FC0(
            s_stage1NumericRuntime);
    }
}

static uint32_t ResolveStage1RunnerQueryFrame30(const PrStageRunner& runner) {
    return (uint32_t)(std::max)(runner.GetFrame(), 0);
}

static uint32_t ResolveStage1RunnerQueryFrame60(const PrGameContext& ctx,
                                                const PrStageRunner& runner) {
    const uint32_t queryFrame30 = ResolveStage1RunnerQueryFrame30(runner);
    const bool renderHalfStep = ctx.renderOnlyFrame && ctx.renderSubFrame8 > 0u;
    return queryFrame30 * 2u + (renderHalfStep ? 1u : 0u);
}

uint32_t PrScn1::GetStage1QueryFrame30(PrGameContext& ctx) {
    (void)ctx;
    return ResolveStage1RunnerQueryFrame30(GetStageRunner());
}

uint32_t PrScn1::GetStage1QueryFrame60(PrGameContext& ctx) {
    return ResolveStage1RunnerQueryFrame60(ctx, GetStageRunner());
}

static bool TryResolveStage1MovieSegmentIso9660BinPath(
    const std::filesystem::path& dataRoot,
    std::filesystem::path& outPath) {
    outPath.clear();
    std::error_code ec;
    std::filesystem::path p = dataRoot;
    for (int i = 0; i < 8 && !p.empty(); ++i) {
        const std::filesystem::path candidate =
            p / "PaRappa the Rapper.bin";
        if (std::filesystem::exists(candidate, ec)) {
            outPath = candidate;
            return true;
        }
        const std::filesystem::path parent = p.parent_path();
        if (parent == p) {
            break;
        }
        p = parent;
    }

    p = std::filesystem::current_path(ec);
    for (int i = 0; i < 8 && !p.empty(); ++i) {
        const std::filesystem::path candidate =
            p / "PaRappa the Rapper.bin";
        if (std::filesystem::exists(candidate, ec)) {
            outPath = candidate;
            return true;
        }
        const std::filesystem::path parent = p.parent_path();
        if (parent == p) {
            break;
        }
        p = parent;
    }
    return false;
}

bool PrScn1::BuildScene1Row0OverlayTransferFeedback800154B0(
    PrGameContext& ctx,
    PrMovieSegmentDirect::MovieSegmentRowInitFeedback8001A324&
        outRow0Feedback) {
    outRow0Feedback =
        PrMovieSegmentDirect::MovieSegmentRowInitFeedback8001A324{};

    std::filesystem::path binPath;
    const bool binPathKnown =
        TryResolveStage1MovieSegmentIso9660BinPath(ctx.dataRoot, binPath);
    const bool row0Known =
        PrStage1MovieSegmentDirect::
            TryBuildStage1MovieSegmentRowFeedbackFromIso801C4780(
                PrStage1MovieSegmentDirect::
                    kStage1SceneEntryMovieSegmentSceneIndex801C4780,
                0u,
                binPath,
                binPathKnown,
                outRow0Feedback);
    if (!row0Known) {
        return false;
    }
    return true;
}

static constexpr uint16_t kStage1GameplayAcceptedInputMask =
    (uint16_t)PrPadButton::Left |
    (uint16_t)PrPadButton::Right |
    (uint16_t)PrPadButton::Triangle |
    (uint16_t)PrPadButton::Circle |
    (uint16_t)PrPadButton::Cross |
    (uint16_t)PrPadButton::Square;

bool ConsumeStage1DirectFrameClearTerminalTailPulse(const PrStage1OverlayData& data,
                                                    int tick96,
                                                    uint32_t queryFrame) {
    if (!s_stage1NumericRuntime.active) {
        return false;
    }

    const uint32_t scriptFrame =
        tick96 >= 0 ? static_cast<uint32_t>(tick96) : queryFrame;
    return PrStageEventDirectStage1ConsumeClearTerminalBranchTrigger801C9094(
        s_stage1EventStreamRuntime9094,
        data,
        queryFrame,
        scriptFrame,
        s_stage1NumericRuntime.rightRankState.rightRankActiveRow);
}

Stage1RunnerTailGateLateBranchSnapshot
ResolveStage1RunnerTailGateLateBranchSnapshot() {
    const PrStageEventDirectStage1Runtime& lateBranch =
        s_stage1EventStreamRuntime9094;
    Stage1RunnerTailGateLateBranchSnapshot out{};
    out.tailDispatchFamilyActive = lateBranch.tailDispatchFamilyActive;
    out.tailDispatchFamilyStream = lateBranch.tailDispatchFamilyStream;
    out.activeDispatchStream = lateBranch.activeDispatchStream;
    out.eventStreamFlagLastUpdateKnown =
        lateBranch.eventStreamFlagLastUpdateKnown;
    out.eventStreamFlagLastUpdateReason =
        lateBranch.eventStreamFlagLastUpdateReason;
    out.eventStreamFlagLastUpdateQueryFrame =
        lateBranch.eventStreamFlagLastUpdateQueryFrame;
    out.eventStreamFlagLastUpdateScriptFrame =
        lateBranch.eventStreamFlagLastUpdateScriptFrame;
    out.eventStreamFlagLastUpdatePrevious =
        lateBranch.eventStreamFlagLastUpdatePrevious;
    out.eventStreamFlagLastUpdateCurrent =
        lateBranch.eventStreamFlagLastUpdateCurrent;
    out.eventStreamFlagLastChangeKnown =
        lateBranch.eventStreamFlagLastChangeKnown;
    out.eventStreamFlagLastChangeReason =
        lateBranch.eventStreamFlagLastChangeReason;
    out.eventStreamFlagLastChangeQueryFrame =
        lateBranch.eventStreamFlagLastChangeQueryFrame;
    out.eventStreamFlagLastChangeScriptFrame =
        lateBranch.eventStreamFlagLastChangeScriptFrame;
    out.eventStreamFlagLastChangePrevious =
        lateBranch.eventStreamFlagLastChangePrevious;
    out.eventStreamFlagLastChangeCurrent =
        lateBranch.eventStreamFlagLastChangeCurrent;
    out.eventStreamFlagLastRunnerClearKnown =
        lateBranch.eventStreamFlagLastRunnerClearKnown;
    out.eventStreamFlagLastRunnerClearReason =
        lateBranch.eventStreamFlagLastRunnerClearReason;
    out.eventStreamFlagLastRunnerClearQueryFrame =
        lateBranch.eventStreamFlagLastRunnerClearQueryFrame;
    out.eventStreamFlagLastRunnerClearScriptFrame =
        lateBranch.eventStreamFlagLastRunnerClearScriptFrame;
    out.eventStreamFlagLastRunnerClearInputFlag =
        lateBranch.eventStreamFlagLastRunnerClearInputFlag;
    out.eventStreamFlagLastRunnerClearOutputFlag =
        lateBranch.eventStreamFlagLastRunnerClearOutputFlag;
    out.eventStreamFlagLastRunnerClearInputCtxFlags00 =
        lateBranch.eventStreamFlagLastRunnerClearInputCtxFlags00;
    out.eventStreamFlagLastRunnerClearOutputCtxFlags00 =
        lateBranch.eventStreamFlagLastRunnerClearOutputCtxFlags00;
    out.eventStreamFlagLastRunnerClearInputEd1C =
        lateBranch.eventStreamFlagLastRunnerClearInputEd1C;
    out.eventStreamFlagLastRunnerClearOutputEd1C =
        lateBranch.eventStreamFlagLastRunnerClearOutputEd1C;
    out.eventStreamFlagLastRunnerClearWord4E =
        lateBranch.eventStreamFlagLastRunnerClearWord4E;
    return out;
}

static PrScn1::Stage1NumericRuntimeTimecodeInput801C7560
BuildStage1RuntimeTimecodeInput801C7560() {
    PrScn1::Stage1NumericRuntimeTimecodeInput801C7560 input{};
    input.sceneEntryField348TicksPerMinute =
        s_stage1LifecycleScene801C7284.field348_TicksPerMinute;
    input.sceneEntryField352FallbackTickAdvance =
        s_stage1LifecycleScene801C7284.field352_TicksPerFrame60;
    input.sceneEntryField356TickOffset =
        s_stage1LifecycleScene801C7284.field356_BaseTick;
    input.sceneEntryField360HalfSource =
        s_stage1LifecycleScene801C7284.field360;

    constexpr size_t kStage1SceneEntryStageXaRowIndex801C7560 = 3u;
    const auto& loaders = s_stage1LifecycleScene801C7284.loaders;
    if (kStage1SceneEntryStageXaRowIndex801C7560 < loaders.size()) {
        const PrStage1MovieSegmentDirect::MovieSegmentRecord48& row =
            loaders[kStage1SceneEntryStageXaRowIndex801C7560]
                .movieSegmentRecord;
        if (row.timeBaseA1Plus40Known) {
            input.sceneEntryField196TimeBaseKnown = true;
            input.sceneEntryField196TimeBaseA7A4 = row.timeBaseA1Plus40;
        }
    }
    return input;
}

static uint32_t PackStage1XaStartCdlFilePosBcd() {
    constexpr size_t kStage1SceneEntryStageXaRowIndex = 3u;
    const auto& loaders = s_stage1LifecycleScene801C7284.loaders;
    if (kStage1SceneEntryStageXaRowIndex >= loaders.size()) {
        return 0u;
    }

    const auto& row =
        loaders[kStage1SceneEntryStageXaRowIndex].movieSegmentRecord;
    if (!row.startMsfKnown) {
        return 0u;
    }

    return static_cast<uint32_t>(row.startMsf.minute) |
           (static_cast<uint32_t>(row.startMsf.second) << 8u) |
           (static_cast<uint32_t>(row.startMsf.frame) << 16u);
}

static bool PrimeStage1RuntimeSlotPlayer(PrGameContext& ctx) {
    if (ctx.currentScene != PrSceneId::Scene1 ||
        !ctx.stageRunning ||
        s_stage1IntroTransitionActive ||
        !ctx.stage1OverlayData ||
        !ctx.stage1OverlayData->valid) {
        return false;
    }

    PrStageRunner& runner = GetStageRunner();
    if (runner.GetSceneId() != 1 || runner.GetState() != StageRunnerState::Playing) {
        return false;
    }

    UpdateStage1NumericRuntimeState(
        ctx,
        runner,
        BuildStage1RuntimeTimecodeInput801C7560());
    if (!s_stage1NumericRuntime.active) {
        return false;
    }

    const Stage1RunnerTimingRoots30 timing = ResolveStage1RunnerTimingRoots30(runner);
    const uint32_t queryFrame = timing.queryFrame;
    const uint32_t scriptFrame =
        timing.tick96 >= 0 ? (uint32_t)timing.tick96 : queryFrame;
    if (s_stage1RuntimeSlotPlayer.active &&
        s_stage1RuntimeSlotPlayer.lastQueryFrame == (int32_t)queryFrame &&
        s_stage1RuntimeSlotPlayer.lastScriptFrame == (int32_t)scriptFrame) {
        return true;
    }

    const PrStage1RuntimeSlotsDirectAdvanceCarrier801C9094 carrier =
        PrStage1RuntimeSlotsDirectBuildAdvanceCarrier801C9094(
            queryFrame,
            scriptFrame,
            BuildStage1GlobalOptionsCarrier801C7A60(ctx),
            &s_stage1EventStreamRuntime9094,
            BuildStage1RuntimeSlotsFollowUpFacts801CBFDC(
                s_stage1NumericRuntime));
    PrStage1RuntimeSlotsDirectAdvanceFromCarrier801C9094(
        s_stage1RuntimeSlotPlayer,
        *ctx.stage1OverlayData,
        carrier);
    return s_stage1RuntimeSlotPlayer.active;
}

static PrStage1RuntimeSlotsDirectFollowUpFacts801CBFDC
BuildStage1TransitionPreviewFollowUpFacts801CBFDC() {
    PrStage1RuntimeSlotsDirectFollowUpFacts801CBFDC facts{};
    // This preview is a fresh 801CBFDC snapshot: previewRuntime{},
    // queryFrame=0, scriptFrame=0, and no 801C9094 event stream. Keep only
    // reset-compatible initial fields so stale gameplay follow-up pulses do
    // not re-upload failed-frame face/HUD state during retry transition.
    facts.rightRankActiveRow = kStage1InitialRightRankActiveRow;
    facts.compactPainterGate7A = 1u;
    return facts;
}

static bool ResolveStage1RuntimeSlotsTransitionPreviewSnapshot(
    PrGameContext& ctx,
    PrStage1RuntimeSlotsSnapshot& out) {
    out = PrStage1RuntimeSlotsSnapshot{};

    if (ctx.currentScene != PrSceneId::Scene1 ||
        !ctx.stageRunning ||
        !s_stage1IntroTransitionActive ||
        !ctx.stage1OverlayData ||
        !ctx.stage1OverlayData->valid) {
        return false;
    }

    const PrStage1RuntimeSlotsDirectFollowUpFacts801CBFDC facts =
        BuildStage1TransitionPreviewFollowUpFacts801CBFDC();

    PrStage1RuntimeSlotsDirectRuntime previewRuntime{};
    const uint32_t queryFrame = 0u;
    const uint32_t scriptFrame = 0u;
    const PrStage1RuntimeSlotsDirectAdvanceCarrier801C9094 carrier =
        PrStage1RuntimeSlotsDirectBuildAdvanceCarrier801C9094(
            queryFrame,
            scriptFrame,
            BuildStage1GlobalOptionsCarrier801C7A60(ctx),
            nullptr,
            facts);
    PrStage1RuntimeSlotsDirectAdvanceFromCarrier801C9094(
        previewRuntime,
        *ctx.stage1OverlayData,
        carrier);
    PrStage1RuntimeSlotsDirectPopulateSnapshot(
        previewRuntime,
        queryFrame,
        scriptFrame,
        out);
    if (!out.valid) {
        return false;
    }

    out.cameraPlaybackEnabled3034 = false;
    out.sceneSubmit.ctxFlagsFull &= ~kStage1SceneBaseRowFlag;
    out.sceneSubmit.ctxFlagsPulse801CBFDC &= ~kStage1SceneBaseRowFlag;
    return true;
}

static bool ResolveStage1RuntimeSlotsSnapshot(PrGameContext& ctx,
                                              PrStage1RuntimeSlotsSnapshot& out,
                                              bool allowTransitionFreeze,
                                              bool allowStoppedRuntimeFreeze) {
    out = PrStage1RuntimeSlotsSnapshot{};

    if (allowStoppedRuntimeFreeze &&
        ctx.currentScene == PrSceneId::Scene1 &&
        s_stage1RuntimeSlotPlayer.active) {
        PrStage1RuntimeSlotsDirectPopulateSnapshot(
            s_stage1RuntimeSlotPlayer,
            ResolveStage1RunnerQueryFrame60(ctx, GetStageRunner()),
            s_stage1RuntimeSlotPlayer.scriptFrame,
            out);
        ApplyStage1TerminalCleanupRenderCtxOverride801C7A60(out);
        return out.valid;
    }

    if (allowTransitionFreeze &&
        ResolveStage1RuntimeSlotsTransitionPreviewSnapshot(ctx, out)) {
        return true;
    }

    if (ctx.currentScene != PrSceneId::Scene1 ||
        !ctx.stageRunning ||
        !s_stage1RuntimeSlotPlayer.active) {
        return false;
    }

    PrStage1RuntimeSlotsDirectPopulateSnapshot(
        s_stage1RuntimeSlotPlayer,
        ResolveStage1RunnerQueryFrame60(ctx, GetStageRunner()),
        s_stage1RuntimeSlotPlayer.scriptFrame,
        out);
    ApplyStage1TerminalCleanupRenderCtxOverride801C7A60(out);
    return true;
}

static void ApplyStage1PsxXaSetFilter13Request(PrGameContext& ctx) {
    if (!ctx.xa1Player || !ctx.xa1Player->IsPlaying()) {
        return;
    }

    const auto& tailHost = s_stage1NumericRuntime.runnerTailHost7A60;
    if (!tailHost.known || !tailHost.requestXaSetFilter13) {
        return;
    }

    const PrStage1XaCdDirectSetFilter13Request setFilter =
        PrStage1XaCdDirectApplySub8001A654(
            ctx.stage1XaCdDirect,
            tailHost.xaSetFilter13Arg);
    if (setFilter.requestSetFilter) {
        ctx.xa1Player->SetFilter(setFilter.file, setFilter.channel);
    }
}

static void ApplyStage1RunnerTailHostAudioActions7A60(PrGameContext& ctx) {
    const auto& tailHost = s_stage1NumericRuntime.runnerTailHost7A60;
    if (!tailHost.known) {
        return;
    }
    if (tailHost.requestFailCue943EC) {
        PrSfx::PlayStage1FailTailCue943EC();
    }
    if (tailHost.requestAudioA4A4Arg1) {
        const PrStage1XaCdDirectBgmVolumeRequest8001A4A4 volume =
            PrStage1XaCdDirectApplySub8001A4A4(1);
        if (volume.requestSetVolume && ctx.xa1Player) {
            ctx.xa1Player->SetVolume(volume.normalizedVolume);
        }
    }
}

static void PumpStage1XaCdDirectRingPackets(PrGameContext& ctx) {
    if (!ctx.xa1Player || !ctx.xa1Player->IsPlaying()) {
        return;
    }

    for (uint32_t guard = 0u; guard < 64u; ++guard) {
        Xa1RingPacketView packet{};
        if (!ctx.xa1Player->PollAcceptedRingPacket(packet)) {
            break;
        }

        PrStage1XaCdDirectHalGetlocPFactsInput getlocPFacts{};
        getlocPFacts.source =
            PrStage1XaCdDirectHalGetlocPSource::AcceptedRingPacket;
        getlocPFacts.sectorIndexKnown = true;
        getlocPFacts.sectorIndex = packet.sectorIndex;
        getlocPFacts.cdGetlocPResponseKnown =
            packet.cdGetlocPResponseKnown;
        getlocPFacts.cdGetlocPResponse = packet.cdGetlocPResponse;
        getlocPFacts.cdDataReadyInterruptKnown =
            packet.cdDataReadyInterruptKnown;
        getlocPFacts.cdDataReadyInterrupt = packet.cdDataReadyInterrupt;
        (void)PrStage1XaCdDirectApplyHalGetlocPFacts(
            ctx.stage1XaCdDirect,
            getlocPFacts);

        const PrStage1XaCdDirectRingPacketInput directPacket{
            packet.sectorIndex,
            packet.file,
            packet.channel,
            packet.coding,
            packet.cdGetlocPResponseKnown,
            packet.cdGetlocPResponse,
            packet.cdDataReadyInterruptKnown,
            packet.cdDataReadyInterrupt,
            packet.header32,
            packet.headerSize,
            packet.payload2016,
            packet.payloadSize};
        if (PrStage1XaCdDirectIsRingPacketCandidate(directPacket)) {
            (void)PrStage1XaCdDirectApplySub80039670Packet(
                ctx.stage1XaCdDirect,
                directPacket);
        }
    }

    std::array<uint8_t, 8> currentGetlocP{};
    uint32_t currentGetlocPSector = 0u;
    if (!s_stage1XaCurrentPhysicalGetlocPProbeDisabled &&
        ctx.xa1Player->GetLastCdClockGetlocPResponse(currentGetlocP,
                                                     &currentGetlocPSector)) {
        PrStage1XaCdDirectHalGetlocPFactsInput currentGetlocPFacts{};
        currentGetlocPFacts.source =
            PrStage1XaCdDirectHalGetlocPSource::CurrentPhysicalClock;
        currentGetlocPFacts.sectorIndexKnown = true;
        currentGetlocPFacts.sectorIndex = currentGetlocPSector;
        currentGetlocPFacts.cdGetlocPResponseKnown = true;
        currentGetlocPFacts.cdGetlocPResponse = currentGetlocP;
        currentGetlocPFacts.cdDataReadyInterruptKnown = true;
        currentGetlocPFacts.cdDataReadyInterrupt = 2u;
        (void)PrStage1XaCdDirectApplyHalGetlocPFacts(
            ctx.stage1XaCdDirect,
            currentGetlocPFacts);
    }
}

static bool ResolveStage1NumericStatusRuntimeSnapshot(PrGameContext& ctx,
                                                      PrStage1NumericStatusSnapshot& out) {
    out = PrStage1NumericStatusSnapshot{};

    if (ctx.currentScene != PrSceneId::Scene1) {
        return false;
    }
    if (!ctx.stageRunning || s_stage1IntroTransitionActive) {
        return false;
    }

    PrStageRunner& runner = GetStageRunner();
    if (runner.GetSceneId() != 1 || runner.GetState() != StageRunnerState::Playing) {
        return false;
    }

    if (!s_stage1NumericRuntime.active) {
        return false;
    }

    if (IsStage1TerminalCleanupNumericStatusGateCleared801C7A60()) {
        return false;
    }

    const Stage1NumericRuntimeState& runtime = s_stage1NumericRuntime;
    PrStage1HudTextBridgeDirect::NumericStatusRuntimeInput input{};
    input.valid = true;
    input.scoreDisplayValue = runtime.scoreDisplayValue;
    input.rightRankState = runtime.rightRankState;
    input.rightRankFlag0200Pulse = runtime.rightRankDirectFollowUp.flag0200Pulse;
    input.rightRankTransitionAnim18E =
        runtime.rightRankBucketContext.transitionAnim18E;
    input.topLessonPairState = runtime.topLessonPairState;
    input.steadyGameplayGateActive = true;
    input.highLayoutMode = ctx.subtitleFlag != 0;
    out = PrStage1HudTextBridgeDirect::BuildNumericStatusSnapshotFromRuntimeState(
        input);
    return true;
}

bool GetStage1HudRouteSnapshot(
    PrGameContext& ctx,
    PrStage1LiveHud::Stage1ResolvedHudTextRouteState& outTextRoute,
    PrStage1LiveHud::Stage1ResolvedHudNumericStatusRouteState& outNumericRoute) {
    outTextRoute = PrStage1LiveHud::Stage1ResolvedHudTextRouteState{};
    outNumericRoute = PrStage1LiveHud::Stage1ResolvedHudNumericStatusRouteState{};
    if (ctx.currentScene != PrSceneId::Scene1) {
        return false;
    }

    PrStage1OverlayScriptTextSnapshot overlayScript{};
    const bool overlayScriptAvailable =
        PrStage1OverlayScriptTextDirectResolveSnapshot(
            s_stage1OverlayScriptTextDirect,
            overlayScript);

    PrStage1CommonLyricsSnapshot commonLyrics{};
    (void)PrStage1MovieTextDirect::GetCommonLyricsSnapshot(
        s_movie1TextRuntime,
        commonLyrics);
    const PrStage1CommonLyricsSnapshot commonLyricsForHud =
        PrStage1MovieTextDirect::BuildCommonLyricsHudCarrierSub8001EC54(
            s_movie1TextWindowTickResult,
            commonLyrics)
            .commonLyrics;

    PrStage1NumericStatusSnapshot numeric{};
    const bool numericAvailable =
        ResolveStage1NumericStatusRuntimeSnapshot(ctx, numeric);

    PrStage1HudTextBridgeDirect::DisplayGateFromRuntimeSnapshotsInput
        displayInput{};
    displayInput.currentSceneIsStage1 = true;
    displayInput.introTransitionActive = s_stage1IntroTransitionActive;
    displayInput.subtitleEnabled = ctx.subtitleFlag != 0;
    displayInput.overlayScriptSnapshotAvailable = overlayScriptAvailable;
    displayInput.overlayScript = overlayScript;
    displayInput.commonLyrics = commonLyricsForHud;
    displayInput.numericStatusSnapshotAvailable = numericAvailable;
    displayInput.numericStatus = numeric;

    PrStage1DisplayGateSnapshot displayGate{};
    if (!PrStage1HudTextBridgeDirect::BuildDisplayGateSnapshotFromRuntimeSnapshots(
            displayInput,
            displayGate)) {
        return false;
    }

    PrStage1HudTextBridgeDirect::TextRouteFromRuntimeSnapshotsInput textInput{};
    textInput.overlayData = ctx.stage1OverlayData.get();
    textInput.displayGate = displayGate;
    textInput.overlayScriptSnapshotAvailable = overlayScriptAvailable;
    textInput.overlayScript = overlayScript;
    textInput.commonLyrics = commonLyricsForHud;
    outTextRoute =
        PrStage1HudTextBridgeDirect::BuildTextRouteStateFromRuntimeSnapshots(textInput);

    if (numericAvailable && numeric.valid) {
        outNumericRoute =
            PrStage1HudTextBridgeDirect::BuildNumericStatusRouteStateFromRuntimeSnapshot(
                numeric,
                displayGate);
        outNumericRoute.transitionState916D0 = (uint16_t)ctx.transitionState;
    }

    return outTextRoute.anyAvailable || outNumericRoute.available;
}

bool GetStage1CommonLyricsHudRouteSnapshot(
    PrGameContext& ctx,
    PrStage1LiveHud::Stage1ResolvedHudTextRouteState& outTextRoute) {
    outTextRoute = PrStage1LiveHud::Stage1ResolvedHudTextRouteState{};
    if (ctx.currentScene != PrSceneId::Scene1) {
        return false;
    }

    PrStage1CommonLyricsSnapshot commonLyrics{};
    (void)PrStage1MovieTextDirect::GetCommonLyricsSnapshot(
        s_movie1TextRuntime,
        commonLyrics);
    const PrStage1CommonLyricsSnapshot commonLyricsForHud =
        PrStage1MovieTextDirect::BuildCommonLyricsHudCarrierSub8001EC54(
            s_movie1TextWindowTickResult,
            commonLyrics)
            .commonLyrics;

    PrStage1DisplayGateSnapshot displayGate{};
    displayGate.valid = true;
    displayGate.subtitleHighLayout = ctx.subtitleFlag != 0;
    displayGate.commonLyricVisible =
        ctx.subtitleFlag != 0 && commonLyricsForHud.valid;
    displayGate.commonLyricMuteGate = false;

    PrStage1HudTextBridgeDirect::TextRouteFromRuntimeSnapshotsInput textInput{};
    textInput.overlayData = ctx.stage1OverlayData.get();
    textInput.displayGate = displayGate;
    textInput.commonLyrics = commonLyricsForHud;
    outTextRoute =
        PrStage1HudTextBridgeDirect::BuildTextRouteStateFromRuntimeSnapshots(
            textInput);
    return outTextRoute.anyAvailable;
}

struct Stage1ClearTailMovieDebugSnapshot {
    int hostBlockKind = 0;
    int hostBlockActive = 0;
    int hostBlockWaitingForPendingActions = 0;
    int deferredSceneResultKnown = 0;
    int deferredSceneResult = 0;
    int pendingActionCount = 0;
    int pendingActionKind = 0;
    int pendingActionPsxOrder = 0;
    int pendingActionPsxFunctionKnown = 0;
    int pendingActionPsxFunction = 0;
    int pendingSaveStatus1635CKnown = 0;
    int pendingSaveStatus1635CIndex = 0;
    int pendingSaveStatus1635CPsxOrder = 0;
    int pendingSaveStatus1635CPsxFunctionKnown = 0;
    int pendingSaveStatus1635CPsxFunction = 0;
    int clearTailStageStatusKnown = 0;
    int clearTailWaitingForWord800916F0 = 0;
    int clearTailPreWord800916F0ActionsApplied = 0;
    int saveUi19148StartPending = 0;
    int saveUi19148Active = 0;
    int saveUi19148LowerFeedbackRequestCount = 0;
    int saveUi19148Seed80092F10Known = 0;
    int saveUi19148Seed80092F10Address = 0;
    int bootstrap15590LoaderDirectBeginAttempted = 0;
    int bootstrap15590LoaderDirectBegun = 0;
    int bootstrap15590LoaderDirectBeginSucceeded = 0;
    int bootstrap15590LoaderDirectBeginFailed = 0;
    int bootstrap15590LoaderDirectPumpStarted = 0;
    int bootstrap15590LoaderDirectWaitingExternal = 0;
    int bootstrap15590LoaderDirectCompleted = 0;
    int bootstrap15590LoaderDirectFailed = 0;
    int bootstrap15590LoaderDirectActionCount = 0;
    int bootstrap15590LoaderDirectNextActionIndex = 0;
    int bootstrap15590LoaderDirectLastPumpActionCount = 0;
    int bootstrap15590LoaderDirectWaitingForFeedback = 0;
    int bootstrap15590LoaderDirectWaitingStepValid = 0;
    int bootstrap15590LoaderDirectWaitingStepKind = 0;
    int bootstrap15590LoaderDirectWaitingCategory = 0;
    int bootstrap15590LoaderDirectWaitingActionKind = 0;
    int bootstrap15590LoaderDirectWaitingPsxOrder = 0;
    int bootstrap15590LoaderDirectWaitingPsxFunction = 0;
    int bootstrap15590LoaderDirectWaitingDirectFunction = 0;
    int bootstrap15590LoaderDirectWaitingLowerFunction = 0;
    int bootstrap15590LoaderDirectWaitingCdActionKind = 0;
    int bootstrap15590LoaderDirectWaitingRecordIndex = 0;
    int bootstrap15590LoaderDirectWaitingRecordType = 0;
    int bootstrap15590CdLowerKeyKnown = 0;
    int bootstrap15590CdLowerRequestPending = 0;
    int bootstrap15590CdLowerStatus = 0;
    int bootstrap15590CdLowerPsxOrder = 0;
    int bootstrap15590CdLowerStepKind = 0;
    int bootstrap15590CdLowerCdActionKind = 0;
    int bootstrap15590CdLowerRecordIndex = 0;
    int bootstrap15590CdLowerRecordType = 0;
    int bootstrap15590CdLowerAttemptIndex = 0;
    int bootstrap15590CdLowerLowerFunction = 0;
    int bootstrap15590CdLowerFinalFunction = 0;
    int bootstrap15590CdLowerPayloadBytesRequired = 0;
    int bootstrap15590CdLowerSeekLbaKnown = 0;
    int bootstrap15590CdLowerSeekLba = 0;
    int bootstrap15590CdLowerReadDstPtrKnown = 0;
    int bootstrap15590CdLowerReadDstPtr = 0;
    int bootstrap15590CdLowerReadSectorCountKnown = 0;
    int bootstrap15590CdLowerReadSectorCount = 0;
    int bootstrap15590CdLowerReadStartHalProgressAccepted = 0;
    int bootstrap15590CdLowerReadStartHalProgressReadS27Serial = 0;
    int bootstrap15590CdLowerFinalReadyHalFactsRequired = 0;
    int bootstrap15590CdLowerFinalReadyHalFactsReadS27Serial = 0;
    int bootstrap15590CdLowerCdSyncLoopFactsRequired80037070 = 0;
    int bootstrap15590CdLowerCdSyncLoopFunction80037070 = 0;
    int bootstrap15590CdLowerCdSyncLoopA0WaitModeKnown80037070 = 0;
    int bootstrap15590CdLowerCdSyncLoopA0WaitMode80037070 = 0;
    int bootstrap15590CdLowerLastRejectKnown = 0;
    int bootstrap15590CdLowerLastRejectReason = 0;
    int bootstrap15590CdLowerLastRejectStatus = 0;
    int bootstrap15590CdLowerLastFactsAttempted = 0;
    int bootstrap15590CdLowerLastFactsActionKind = 0;
    int bootstrap15590CdLowerLastFactsReadStartHalFactsKnown = 0;
    int bootstrap15590CdLowerLastFactsReadStartSetupProduced = 0;
    int bootstrap15590CdLowerLastFactsReadStartSetupIncomplete = 0;
    int bootstrap15590CdLowerLastFactsReadStartSetupFirstMissing = 0;
    int bootstrap15590CdLowerLastFactsReadPumpProduced = 0;
    int bootstrap15590CdLowerLastFactsReadPumpIncomplete = 0;
    int bootstrap15590CdLowerLastFactsReadPumpFirstMissing = 0;
    int bootstrap15590CdLowerLastFactsPayloadBytesKnown = 0;
    int bootstrap15590CdLowerLastFactsBridgeProduced = 0;
    int bootstrap15590CdLowerLastFactsBridgeIncomplete = 0;
    int bootstrap15590CdLowerLastFactsLowerCdFactsBridged = 0;
    int bootstrap15590CdLowerLastFactsXaCdSeamKnown = 0;
    int bootstrap15590CdLowerLastFactsXaCdAccepted = 0;
    int bootstrap15590CdLowerLastFactsRejectReason = 0;
    int clearTailMovieBlockActive = 0;
    int clearTailMoviePlayAndWaitPending = 0;
    int clearTailMoviePlayAndWaitResultKnown = 0;
    int clearTailMoviePlayAndWaitResult = 0;
    int clearTailMovieVisualActive = 0;
    int moviePlayAndWaitCompletionPending = 0;
    int movieDrawableActive = 0;
    int transition20110Mode2Pending = 0;
    int transitionSub80020110Mode2Active = 0;
    int transitionSub800201ACActive = 0;
    int transitionSub800201ACCompleted = 0;
    int transitionSub800201ACPhase = 0;
    int transitionSub800201ACGp196 = 0;
    int transitionSub800201ACTailFrames = 0;
    int transitionSub800201ACTailIndex = 0;
    int movieStrFrameActive = 0;
    int movieOutroActive = 0;
    int movieOutroTailActive = 0;
    int movieSkipPreludeActive = 0;
    int movieOuterLoopPhase = 0;
    int movieOuterLoopComplete = 0;
    int movieOuterLoopExitReason = 0;
    int movieOuterLoopReturn = 0;
    int movieOuterLoopLastStepKnown = 0;
    int movieOuterLoopLastStepPhaseBefore = 0;
    int movieOuterLoopLastStepPhaseAfter = 0;
    int movieOuterLoopLastStepActionCount = 0;
    int movieOuterLoopLastStepMovieStepKnown = 0;
    int movieOuterLoopLastStepMovieStepResult = 0;
    int movieOuterLoopLastStepA7F8Computed = 0;
    int movieOuterLoopLastStepA7F8Known = 0;
    int movieOuterLoopLastStepA7F8Result = 0;
    int movieOuterLoopLastStepGapInputMask = 0;
    int movieOuterLoopLastStepGapDword9554 = 0;
    int movieOuterLoopLastStepGapGsWorkBase = 0;
    int movieOuterLoopLastStepGapByte493F4 = 0;
    int movieOuterLoopLastStepGapSub8001A750 = 0;
};

Stage1ClearTailMovieDebugSnapshot GetStage1ClearTailMovieDebugSnapshot() {
    Stage1ClearTailMovieDebugSnapshot out{};
    const PrStage1LifecycleExecutorDirect::HostBlockSnapshot801C81EC block =
        PrStage1LifecycleExecutorDirect::GetHostBlockSnapshot801C81EC(
            s_stage1LifecycleExecutor801C81EC);
    out.hostBlockKind = static_cast<int>(block.kind);
    out.hostBlockActive = block.active ? 1 : 0;
    out.hostBlockWaitingForPendingActions =
        block.waitingForPendingActions ? 1 : 0;
    out.deferredSceneResultKnown =
        block.deferredSceneResultKnown ? 1 : 0;
    out.deferredSceneResult = block.deferredSceneResult;
    out.pendingActionCount = static_cast<int>(block.pendingActionCount);
    out.pendingActionKind = static_cast<int>(block.pendingActionKind);
    out.pendingActionPsxOrder = static_cast<int>(block.pendingActionPsxOrder);
    out.pendingActionPsxFunctionKnown =
        block.pendingActionPsxFunctionKnown ? 1 : 0;
    out.pendingActionPsxFunction =
        static_cast<int>(block.pendingActionPsxFunction);
    out.pendingSaveStatus1635CKnown =
        block.pendingSaveStatus1635CKnown ? 1 : 0;
    out.pendingSaveStatus1635CIndex =
        static_cast<int>(block.pendingSaveStatus1635CIndex);
    out.pendingSaveStatus1635CPsxOrder =
        static_cast<int>(block.pendingSaveStatus1635CPsxOrder);
    out.pendingSaveStatus1635CPsxFunctionKnown =
        block.pendingSaveStatus1635CPsxFunctionKnown ? 1 : 0;
    out.pendingSaveStatus1635CPsxFunction =
        static_cast<int>(block.pendingSaveStatus1635CPsxFunction);
    out.clearTailStageStatusKnown =
        s_stage1LifecycleRuntime801C81EC.clearTailStageStatusKnown ? 1 : 0;
    out.clearTailWaitingForWord800916F0 =
        s_stage1LifecycleRuntime801C81EC.clearTailWaitingForWord800916F0 ? 1 : 0;
    out.clearTailPreWord800916F0ActionsApplied =
        s_stage1LifecycleRuntime801C81EC.clearTailPreWord800916F0ActionsApplied ? 1 : 0;
    out.saveUi19148StartPending = block.startPending ? 1 : 0;
    out.saveUi19148Active =
        block.kind ==
                    PrStage1LifecycleExecutorDirect::HostBlockKind801C81EC::
                        SaveUi19148 &&
                block.active
            ? 1
            : 0;
    out.saveUi19148LowerFeedbackRequestCount =
        static_cast<int>(block.saveUi19148LowerFeedbackRequests.count);
    out.saveUi19148Seed80092F10Known =
        block.saveUi19148Seed80092F10Known ? 1 : 0;
    out.saveUi19148Seed80092F10Address =
        static_cast<int>(block.saveUi19148Seed80092F10Address);
    out.bootstrap15590LoaderDirectBeginAttempted =
        block.bootstrap15590LoaderDirectBeginAttempted ? 1 : 0;
    out.bootstrap15590LoaderDirectBegun =
        block.bootstrap15590LoaderDirectBegun ? 1 : 0;
    out.bootstrap15590LoaderDirectBeginSucceeded =
        block.bootstrap15590LoaderDirectBeginSucceeded ? 1 : 0;
    out.bootstrap15590LoaderDirectBeginFailed =
        block.bootstrap15590LoaderDirectBeginFailed ? 1 : 0;
    out.bootstrap15590LoaderDirectPumpStarted =
        block.bootstrap15590LoaderDirectPumpStarted ? 1 : 0;
    out.bootstrap15590LoaderDirectWaitingExternal =
        block.bootstrap15590LoaderDirectWaitingExternal ? 1 : 0;
    out.bootstrap15590LoaderDirectCompleted =
        block.bootstrap15590LoaderDirectCompleted ? 1 : 0;
    out.bootstrap15590LoaderDirectFailed =
        block.bootstrap15590LoaderDirectFailed ? 1 : 0;
    out.bootstrap15590LoaderDirectActionCount =
        static_cast<int>(block.bootstrap15590LoaderDirectActionCount);
    out.bootstrap15590LoaderDirectNextActionIndex =
        static_cast<int>(block.bootstrap15590LoaderDirectNextActionIndex);
    out.bootstrap15590LoaderDirectLastPumpActionCount =
        static_cast<int>(block.bootstrap15590LoaderDirectLastPumpActionCount);
    out.bootstrap15590LoaderDirectWaitingForFeedback =
        block.bootstrap15590LoaderDirectWaitingForFeedback ? 1 : 0;
    out.bootstrap15590LoaderDirectWaitingStepValid =
        block.bootstrap15590LoaderDirectWaitingStepValid ? 1 : 0;
    out.bootstrap15590LoaderDirectWaitingStepKind =
        static_cast<int>(block.bootstrap15590LoaderDirectWaitingStepKind);
    out.bootstrap15590LoaderDirectWaitingCategory =
        static_cast<int>(block.bootstrap15590LoaderDirectWaitingCategory);
    out.bootstrap15590LoaderDirectWaitingActionKind =
        static_cast<int>(block.bootstrap15590LoaderDirectWaitingActionKind);
    out.bootstrap15590LoaderDirectWaitingPsxOrder =
        static_cast<int>(block.bootstrap15590LoaderDirectWaitingPsxOrder);
    out.bootstrap15590LoaderDirectWaitingPsxFunction =
        static_cast<int>(block.bootstrap15590LoaderDirectWaitingPsxFunction);
    out.bootstrap15590LoaderDirectWaitingDirectFunction =
        static_cast<int>(block.bootstrap15590LoaderDirectWaitingDirectFunction);
    out.bootstrap15590LoaderDirectWaitingLowerFunction =
        static_cast<int>(block.bootstrap15590LoaderDirectWaitingLowerFunction);
    out.bootstrap15590LoaderDirectWaitingCdActionKind =
        static_cast<int>(block.bootstrap15590LoaderDirectWaitingCdActionKind);
    out.bootstrap15590LoaderDirectWaitingRecordIndex =
        static_cast<int>(block.bootstrap15590LoaderDirectWaitingRecordIndex);
    out.bootstrap15590LoaderDirectWaitingRecordType =
        static_cast<int>(block.bootstrap15590LoaderDirectWaitingRecordType);
    out.bootstrap15590CdLowerKeyKnown =
        block.bootstrap15590CdLower.keyKnown ? 1 : 0;
    out.bootstrap15590CdLowerRequestPending =
        block.bootstrap15590CdLower.requestPending ? 1 : 0;
    out.bootstrap15590CdLowerStatus =
        static_cast<int>(block.bootstrap15590CdLower.status);
    out.bootstrap15590CdLowerPsxOrder =
        static_cast<int>(block.bootstrap15590CdLower.psxOrder);
    out.bootstrap15590CdLowerStepKind =
        static_cast<int>(block.bootstrap15590CdLower.stepKind);
    out.bootstrap15590CdLowerCdActionKind =
        static_cast<int>(block.bootstrap15590CdLower.cdActionKind);
    out.bootstrap15590CdLowerRecordIndex =
        static_cast<int>(block.bootstrap15590CdLower.recordIndex);
    out.bootstrap15590CdLowerRecordType =
        static_cast<int>(block.bootstrap15590CdLower.recordType);
    out.bootstrap15590CdLowerAttemptIndex =
        static_cast<int>(block.bootstrap15590CdLower.attemptIndex);
    out.bootstrap15590CdLowerLowerFunction =
        static_cast<int>(block.bootstrap15590CdLower.lowerFunction);
    out.bootstrap15590CdLowerFinalFunction =
        static_cast<int>(block.bootstrap15590CdLower.finalFunction);
    out.bootstrap15590CdLowerPayloadBytesRequired =
        block.bootstrap15590CdLower.payloadBytesRequired ? 1 : 0;
    out.bootstrap15590CdLowerSeekLbaKnown =
        block.bootstrap15590CdLower.seekLbaKnown ? 1 : 0;
    out.bootstrap15590CdLowerSeekLba =
        block.bootstrap15590CdLower.seekLba;
    out.bootstrap15590CdLowerReadDstPtrKnown =
        block.bootstrap15590CdLower.readDstPtrKnown ? 1 : 0;
    out.bootstrap15590CdLowerReadDstPtr =
        static_cast<int>(block.bootstrap15590CdLower.readDstPtr);
    out.bootstrap15590CdLowerReadSectorCountKnown =
        block.bootstrap15590CdLower.readSectorCountKnown ? 1 : 0;
    out.bootstrap15590CdLowerReadSectorCount =
        block.bootstrap15590CdLower.readSectorCount;
    out.bootstrap15590CdLowerReadStartHalProgressAccepted =
        block.bootstrap15590CdLower.readStartHalProgressAccepted ? 1 : 0;
    out.bootstrap15590CdLowerReadStartHalProgressReadS27Serial =
        static_cast<int>(
            block.bootstrap15590CdLower.readStartHalProgressReadS27Serial);
    out.bootstrap15590CdLowerFinalReadyHalFactsRequired =
        block.bootstrap15590CdLower.finalReadyHalFactsRequired ? 1 : 0;
    out.bootstrap15590CdLowerFinalReadyHalFactsReadS27Serial =
        static_cast<int>(
            block.bootstrap15590CdLower.finalReadyHalFactsReadS27Serial);
    out.bootstrap15590CdLowerCdSyncLoopFactsRequired80037070 =
        block.bootstrap15590CdLower.cdSyncLoopFactsRequired80037070 ? 1 : 0;
    out.bootstrap15590CdLowerCdSyncLoopFunction80037070 =
        static_cast<int>(
            block.bootstrap15590CdLower.cdSyncLoopFunction80037070);
    out.bootstrap15590CdLowerCdSyncLoopA0WaitModeKnown80037070 =
        block.bootstrap15590CdLower.cdSyncLoopA0WaitModeKnown80037070 ? 1 : 0;
    out.bootstrap15590CdLowerCdSyncLoopA0WaitMode80037070 =
        block.bootstrap15590CdLower.cdSyncLoopA0WaitMode80037070;
    out.bootstrap15590CdLowerLastRejectKnown =
        block.bootstrap15590CdLower.lastRejectKnown ? 1 : 0;
    out.bootstrap15590CdLowerLastRejectReason =
        static_cast<int>(block.bootstrap15590CdLower.lastRejectReason);
    out.bootstrap15590CdLowerLastRejectStatus =
        static_cast<int>(block.bootstrap15590CdLower.lastRejectStatus);
    out.bootstrap15590CdLowerLastFactsAttempted =
        block.bootstrap15590CdLower.lastFactsAttempted ? 1 : 0;
    out.bootstrap15590CdLowerLastFactsActionKind =
        static_cast<int>(block.bootstrap15590CdLower.lastFactsActionKind);
    out.bootstrap15590CdLowerLastFactsReadStartHalFactsKnown =
        block.bootstrap15590CdLower.lastFactsReadStartHalFactsKnown ? 1 : 0;
    out.bootstrap15590CdLowerLastFactsReadStartSetupProduced =
        block.bootstrap15590CdLower.lastFactsReadStartSetupProduced ? 1 : 0;
    out.bootstrap15590CdLowerLastFactsReadStartSetupIncomplete =
        block.bootstrap15590CdLower.lastFactsReadStartSetupIncomplete ? 1 : 0;
    out.bootstrap15590CdLowerLastFactsReadStartSetupFirstMissing =
        static_cast<int>(
            block.bootstrap15590CdLower.lastFactsReadStartSetupFirstMissing);
    out.bootstrap15590CdLowerLastFactsReadPumpProduced =
        block.bootstrap15590CdLower.lastFactsReadPumpProduced ? 1 : 0;
    out.bootstrap15590CdLowerLastFactsReadPumpIncomplete =
        block.bootstrap15590CdLower.lastFactsReadPumpIncomplete ? 1 : 0;
    out.bootstrap15590CdLowerLastFactsReadPumpFirstMissing =
        static_cast<int>(
            block.bootstrap15590CdLower.lastFactsReadPumpFirstMissing);
    out.bootstrap15590CdLowerLastFactsPayloadBytesKnown =
        block.bootstrap15590CdLower.lastFactsPayloadBytesKnown ? 1 : 0;
    out.bootstrap15590CdLowerLastFactsBridgeProduced =
        block.bootstrap15590CdLower.lastFactsBridgeProduced ? 1 : 0;
    out.bootstrap15590CdLowerLastFactsBridgeIncomplete =
        block.bootstrap15590CdLower.lastFactsBridgeIncomplete ? 1 : 0;
    out.bootstrap15590CdLowerLastFactsLowerCdFactsBridged =
        block.bootstrap15590CdLower.lastFactsLowerCdFactsBridged ? 1 : 0;
    out.bootstrap15590CdLowerLastFactsXaCdSeamKnown =
        block.bootstrap15590CdLower.lastFactsXaCdSeamKnown ? 1 : 0;
    out.bootstrap15590CdLowerLastFactsXaCdAccepted =
        block.bootstrap15590CdLower.lastFactsXaCdAccepted ? 1 : 0;
    out.bootstrap15590CdLowerLastFactsRejectReason =
        static_cast<int>(block.bootstrap15590CdLower.lastFactsRejectReason);
    out.clearTailMovieBlockActive =
        s_stage1LifecycleExecutor801C81EC.clearTailMovieBlockActive ? 1 : 0;
    out.clearTailMoviePlayAndWaitPending =
        s_stage1LifecycleExecutor801C81EC.clearTailMoviePlayAndWaitPending ? 1 : 0;
    out.clearTailMoviePlayAndWaitResultKnown =
        s_stage1LifecycleExecutor801C81EC.clearTailMoviePlayAndWaitResultKnown ? 1 : 0;
    out.clearTailMoviePlayAndWaitResult =
        s_stage1LifecycleExecutor801C81EC.clearTailMoviePlayAndWaitResult;
    out.clearTailMovieVisualActive =
        s_stage1LifecycleExecutor801C81EC.clearTailMovieVisualActive ? 1 : 0;
    out.moviePlayAndWaitCompletionPending =
        PrStage1Scene1Movie1Direct::IsPlayAndWaitCompletionPending(
            s_movie1Runtime) ? 1 : 0;
    const PrStage1Scene1Movie1Direct::Movie1DrawableStateQueryResult
        drawable =
            PrStage1Scene1Movie1Direct::QueryDrawableState(s_movie1Runtime);
    out.movieDrawableActive = drawable.drawableActive ? 1 : 0;
    out.transition20110Mode2Pending =
        PrStage1LifecycleExecutorDirect::IsTransition20110Mode2Pending801C81EC(
            s_stage1LifecycleExecutor801C81EC) ? 1 : 0;
    out.transitionSub80020110Mode2Active =
        PrStage1Scene1Movie1Direct::IsTransitionSub80020110Active(
            s_movie1Runtime,
            2u,
            1u,
            2u) ? 1 : 0;
    out.transitionSub800201ACActive =
        s_movie1Runtime.transitionSub800201ACActive ? 1 : 0;
    out.transitionSub800201ACCompleted =
        s_movie1Runtime.transitionSub800201ACCompleted ? 1 : 0;
    out.transitionSub800201ACPhase =
        static_cast<int>(s_movie1Runtime.transitionSub800201ACPhase);
    out.transitionSub800201ACGp196 =
        static_cast<int>(s_movie1Runtime.outroGp196);
    out.transitionSub800201ACTailFrames =
        static_cast<int>(s_movie1Runtime.outroTailFrames);
    out.transitionSub800201ACTailIndex =
        static_cast<int>(s_movie1Runtime.transitionSub80020090TailIndex);
    out.movieStrFrameActive = drawable.strFrameActive ? 1 : 0;
    out.movieOutroActive = drawable.outroActive ? 1 : 0;
    out.movieOutroTailActive = drawable.outroTailActive ? 1 : 0;
    out.movieSkipPreludeActive = drawable.skipPreludeActive ? 1 : 0;
    out.movieOuterLoopPhase =
        static_cast<int>(s_movie1TextOuterLoopRuntime.phase);
    out.movieOuterLoopComplete =
        s_movie1TextOuterLoopRuntime.complete ? 1 : 0;
    out.movieOuterLoopExitReason =
        static_cast<int>(s_movie1TextOuterLoopRuntime.exitReason);
    out.movieOuterLoopReturn = s_movie1TextOuterLoopRuntime.psxReturnValue;
    out.movieOuterLoopLastStepKnown = s_movie1TextOuterLoopLastStep.actionCount != 0u ||
                                      s_movie1TextOuterLoopLastStep.phaseBefore !=
                                          PrStage1MovieTextOuterLoopDirect::
                                              MovieTextOuterLoopPhaseSub801C455C::Idle ||
                                      s_movie1TextOuterLoopLastStep.phaseAfter !=
                                          PrStage1MovieTextOuterLoopDirect::
                                              MovieTextOuterLoopPhaseSub801C455C::Idle
                                          ? 1
                                          : 0;
    out.movieOuterLoopLastStepPhaseBefore =
        static_cast<int>(s_movie1TextOuterLoopLastStep.phaseBefore);
    out.movieOuterLoopLastStepPhaseAfter =
        static_cast<int>(s_movie1TextOuterLoopLastStep.phaseAfter);
    out.movieOuterLoopLastStepActionCount =
        static_cast<int>(s_movie1TextOuterLoopLastStep.actionCount);
    out.movieOuterLoopLastStepMovieStepKnown =
        s_movie1TextOuterLoopLastStep.movieStepSub801C448CKnown ? 1 : 0;
    out.movieOuterLoopLastStepMovieStepResult =
        s_movie1TextOuterLoopLastStep.movieStepSub801C448CResult ? 1 : 0;
    out.movieOuterLoopLastStepA7F8Computed =
        s_movie1TextOuterLoopLastStep.sub8001A7F8Computed ? 1 : 0;
    out.movieOuterLoopLastStepA7F8Known =
        s_movie1TextOuterLoopLastStep.sub8001A7F8Known ? 1 : 0;
    out.movieOuterLoopLastStepA7F8Result =
        static_cast<int>(s_movie1TextOuterLoopLastStep.sub8001A7F8Result);
    out.movieOuterLoopLastStepGapInputMask =
        s_movie1TextOuterLoopLastStep.gapMissingInputMaskSub80035510Producer ? 1 : 0;
    out.movieOuterLoopLastStepGapDword9554 =
        s_movie1TextOuterLoopLastStep.gapMissingDword801C9554Writer ? 1 : 0;
    out.movieOuterLoopLastStepGapGsWorkBase =
        s_movie1TextOuterLoopLastStep.gapMissingGsGetWorkBaseProducer ? 1 : 0;
    out.movieOuterLoopLastStepGapByte493F4 =
        s_movie1TextOuterLoopLastStep.gapMissingByte800493F4ClockProducer ? 1 : 0;
    out.movieOuterLoopLastStepGapSub8001A750 =
        s_movie1TextOuterLoopLastStep.gapMissingSub8001A750StatusProducer ? 1 : 0;
    return out;
}

static void ResetStage1Movie1TextRuntime() {
    PrStage1MovieTextDirect::ResetPlaybackStateSub801C77C0(s_movie1TextRuntime);
    s_movie1TextWindowTickResult =
        PrStage1MovieTextDirect::Movie1TextWindowTickResult{};
    PrStage1MovieTextOuterLoopDirect::ResetMovieTextOuterLoopSub801C455C(
        s_movie1TextOuterLoopRuntime);
    PrStage1OverlayScriptTextDirectResetSub801C8604(
        s_stage1OverlayScriptTextDirect);
    ResetStage1FormalLifecycleRuntime();
}

static void ResetStage1SceneLoopRuntime(PrGameContext& ctx);

static PrStage1Scene1FrameDriverDirect::IntroTransitionState
LoadStage1Scene1IntroTransitionState() {
    PrStage1Scene1FrameDriverDirect::IntroTransitionState intro{};
    intro.active = s_stage1IntroTransitionActive;
    intro.renderActive = s_stage1IntroTransitionRenderActive;
    intro.consumerResetPending = s_stage1IntroTransitionConsumerResetPending;
    intro.frame = s_stage1IntroTransitionFrame;
    return intro;
}

static void StoreStage1Scene1IntroTransitionState(
    const PrStage1Scene1FrameDriverDirect::IntroTransitionState& intro) {
    s_stage1IntroTransitionActive = intro.active;
    s_stage1IntroTransitionRenderActive = intro.renderActive;
    s_stage1IntroTransitionConsumerResetPending = intro.consumerResetPending;
    s_stage1IntroTransitionFrame = intro.frame;
}

static void MarkStage1IntroTransitionPrerollPending801C7A60() {
    s_stage1IntroTransitionActive = false;
    s_stage1IntroTransitionRenderActive = false;
    s_stage1IntroTransitionConsumerResetPending = false;
    s_stage1IntroTransitionFrame = -1;
}

static bool ArmStage1IntroTransitionPreroll801C7A60() {
    if (s_stage1IntroTransitionFrame >= 0 || s_stage1IntroTransitionActive) {
        return false;
    }

    s_stage1IntroTransitionActive = true;
    s_stage1IntroTransitionRenderActive = true;
    s_stage1IntroTransitionConsumerResetPending = false;
    PrStage1Scene1DrawBackend::ResetGameplaySubmitRuntime();
    return true;
}

static void SyncStage1Scene1FrameDriverTextProducers(PrGameContext& ctx) {
    {
        const PrStage1RuntimeSlotsDirectGlobalOptionsCarrier globalOptions =
            BuildStage1GlobalOptionsCarrier801C7A60(ctx);
        const PrStage1MovieTextDirect::Movie1TextGlobalOptionsCarrier
            movieTextGlobalOptions =
            PrStage1MovieTextDirect::BuildGlobalOptionsCarrierFromWord800916DC(
                globalOptions.word800916DCKnown,
                globalOptions.word800916DC);
        uint8_t currentGp872Slot = 0;
        const bool currentGp872SlotKnown =
            PrStage1Scene1Movie1Direct::TryGetDrawBufferWord80096590(
                s_movie1Runtime,
                currentGp872Slot);
        const PrPadState padState = PrPad::GetState(0);
        const uint16_t padReturnedMask =
            PrPsxPadDirect::BuildReturnedMask80035510FromLocalAndDebugPad(
                padState.held,
                ctx.debugPadInput);
        const PrPsxPadDirect::PadReadResult80035510 pad80035510 =
            PrPsxPadDirect::PsxReadPadMask80035510(padReturnedMask);
        PrStage1MovieTextOuterLoopDirect::Stage1MovieTextSceneFrameHostFacts801C455C
            facts{};
        facts.globalOptions = movieTextGlobalOptions;
        const uint32_t movieFrame30FromRuntime =
            s_movie1Runtime.currentMovieFrame30;
        const uint32_t movieFrame30FromStr =
            LifecycleHost801C81EC::ResolveMovie1HostFrame30(ctx);
        facts.movieFrame30 =
            movieFrame30FromStr != 0u ? movieFrame30FromStr
                                      : movieFrame30FromRuntime;
        facts.stageRunning = ctx.stageRunning;
        facts.runnerFrame30 = GetStageRunner().GetFrame();
        facts.renderHalfStep = ctx.renderOnlyFrame && ctx.renderSubFrame8 > 0u;
        facts.sceneEntryMemory164Known =
            s_stage1LifecycleScene801C7284.sceneEntryPtr8006EDB8Known;
        facts.sceneEntryMemory164 =
            static_cast<uint32_t>(
                s_stage1LifecycleScene801C7284.field356_BaseTick);
        facts.currentGp872SlotKnown = currentGp872SlotKnown;
        facts.currentGp872Slot = currentGp872Slot;
        facts.inputMaskSub80035510Known =
            pad80035510.called && pad80035510.padDrCalled;
        facts.inputMaskSub80035510 = pad80035510.psxReturnMask;
        const PrStage1LifecycleExecutorDirect::HostBlockSnapshot801C81EC
            movie1Block =
                PrStage1LifecycleExecutorDirect::GetHostBlockSnapshot801C81EC(
                    s_stage1LifecycleExecutor801C81EC);
        const bool movie1BlockActive =
            movie1Block.kind ==
                PrStage1LifecycleExecutorDirect::HostBlockKind801C81EC::
                    Movie1 &&
            movie1Block.active;
        const bool clearTailMovieBlockActive =
            movie1Block.kind ==
                PrStage1LifecycleExecutorDirect::HostBlockKind801C81EC::
                    ClearTailMovie &&
            movie1Block.active &&
            !movie1Block.waitingForPendingActions;
        const bool movieTextBlockActive =
            movie1BlockActive || clearTailMovieBlockActive;
        facts.sub8001A750Known =
            movieTextBlockActive && ctx.strPlayer != nullptr &&
            ctx.strPlayer->IsPlaying();
        facts.sub8001A750Result = facts.sub8001A750Known ? 1u : 0u;
        facts.sub801C448CKnown =
            clearTailMovieBlockActive && ctx.strPlayer != nullptr;
        facts.sub801C448CResult =
            facts.sub801C448CKnown && ctx.strPlayer->IsPlaying();
        facts.languageIndex = GetStage1CommonLyricsLanguageIndex(ctx);
        facts.sceneIndex =
            PrStage1MovieSegmentDirect::
                kStage1SceneEntryMovieSegmentSceneIndex801C4780;

        PrStage1MovieTextOuterLoopDirect::
            MovieTextOuterLoopStepResultSub801C455C outerLoopStep{};
        PrStage1MovieTextOuterLoopDirect::
            TickStage1MovieTextOuterLoopFromHostFacts801C455C(
                s_movie1TextOuterLoopRuntime,
                s_movie1TextRuntime,
                facts,
                s_stage1LifecycleExecutor801C81EC,
                ctx.dataRoot,
                &ctx.stage1XaCdDirect,
                s_movie1TextWindowTickResult,
                &outerLoopStep);
        ctx.stage1XaCdDirect.sub8001A7F8Known =
            outerLoopStep.sub8001A7F8Known;
        ctx.stage1XaCdDirect.sub8001A7F8Result =
            outerLoopStep.sub8001A7F8Result;
        s_movie1TextOuterLoopLastStep = outerLoopStep;
    }
    PrStageRunner& runner = GetStageRunner();
    const bool eventStreamAuthorityReady =
        !s_stage1IntroTransitionActive &&
        PrimeStage1RuntimeSlotPlayer(ctx);
    PrStage1RuntimeSlotsSnapshot runtimeSlotsSnapshot{};
    bool runtimeSlotsSnapshotKnown = false;
    if (eventStreamAuthorityReady) {
        PrStage1RuntimeSlotsDirectPopulateSnapshot(
            s_stage1RuntimeSlotPlayer,
            ResolveStage1RunnerQueryFrame60(ctx, runner),
            s_stage1RuntimeSlotPlayer.scriptFrame,
            runtimeSlotsSnapshot);
        runtimeSlotsSnapshotKnown = runtimeSlotsSnapshot.valid;
    }
    const bool directScriptBoxPermit4E =
        runtimeSlotsSnapshotKnown &&
        runtimeSlotsSnapshot.sceneSubmit.directScriptBoxGate54Known &&
        runtimeSlotsSnapshot.sceneSubmit.directScriptBoxGate54 != 0u;
    const PrStageEventDirectStage1FrameResult801C9094* frameResult801C9094 =
        eventStreamAuthorityReady
            ? &PrStageEventDirectStage1GetFrameResult801C9094(
                  s_stage1EventStreamRuntime9094)
            : nullptr;
    const PrStage1OverlayScriptTextDirectScene1FrameInputSub801C8604 input =
        PrStage1OverlayScriptTextDirectBuildScene1FrameInputSub801C8604(
            ctx.stage1OverlayData.get(),
            ctx.currentScene == PrSceneId::Scene1,
            ctx.stageRunning,
            runner.GetSceneId() == 1 &&
                runner.GetState() == StageRunnerState::Playing,
            GetStage1CommonLyricsLanguageIndex(ctx),
            ResolveStage1RunnerQueryFrame30(runner),
            s_stage1RuntimeSlotPlayer.scriptFrame,
            directScriptBoxPermit4E,
            frameResult801C9094);

    (void)PrStage1OverlayScriptTextDirectAdvanceScene1FrameSub801C8604(
        s_stage1OverlayScriptTextDirect,
        input);
}

static void SyncStage1Movie1TransitionCtxWordsFromRunnerSnapshot() {
    PrStage1Scene1Movie1Direct::Movie1TransitionCtxWords801C3640 out{};
    // 80022CBC reads the actual dwords at 0x801C3640/0x801C3644.
    // This adapter must not derive them from nearby Win timing state.
    const Stage1NumericRuntimeState::RunnerTimecode801C7560Runtime& timecode =
        s_stage1NumericRuntime.runnerTimecode801C7560;
    if (!s_stage1NumericRuntime.active || !timecode.known) {
        PrStage1Scene1Movie1Direct::SetTransitionCtxWords801C3640(
            s_movie1Runtime,
            out);
        return;
    }
    const Stage1NumericRuntimeState::RunnerPostFrame7A60Runtime& postFrame =
        s_stage1NumericRuntime.runnerPostFrame7A60;
    if (!postFrame.word0Flags801C3640Known) {
        PrStage1Scene1Movie1Direct::SetTransitionCtxWords801C3640(
            s_movie1Runtime,
            out);
        return;
    }

    out.known = true;
    out.word0_flags801C3640 = postFrame.word0Flags801C3640;
    out.word1_timecode801C3644 = timecode.snapshot.word801C3644;
    PrStage1Scene1Movie1Direct::SetTransitionCtxWords801C3640(
        s_movie1Runtime,
        out);
}

static bool IsStage1TerminalAudioResetDone801C7A60();

static bool ServiceStage1PendingCdQuery(PrGameContext& ctx, bool terminal = false) {
    auto& cd = ctx.stage1XaCdDirect;
    if (!ctx.xa1Player || !cd.commandSerial || !cd.byte_800573D4Known ||
        cd.byte_800573D4 != 0u) return false;
    bool accepted = false;
    uint32_t sector = 0;
    if (cd.lastCdCommand == 1u) {
        const uint8_t driveStatus = ctx.xa1Player->IsPlaying() ? 0x20u : 0u;
        accepted = PrStage1CdStopExecutionDirect::ApplyPendingStatusReceipt(
            cd, cd.commandSerial, true, driveStatus);
    } else if (cd.lastCdCommand == 0x10u) {
        std::array<uint8_t, 8> header{};
        // Despite the historical accessor name, Xa1Player copies the actual
        // sector0C..13 bytes here: the native command10/GetlocL response.
        const bool known = ctx.xa1Player->GetLastCdClockGetlocPResponse(header, &sector);
        accepted = PrStage1CdStopExecutionDirect::ApplyPendingLocationReceipt(
            cd, cd.commandSerial, known, header);
    }
    if (terminal && accepted) {
        Log::Printf("Scene1 terminal801C7A60: pending CD query acknowledged command=%02X nativeSerial=%u sector=%u syncStatus=%u driveStatusKnown=%d driveStatus=%02X softwareDevice=1 psxMmioAuthority=0",
            cd.lastCdCommand, cd.commandSerial, sector, cd.byte_800573D4,
            cd.dword_80057108Known ? 1 : 0, cd.dword_80057108);
    }
    return accepted;
}

static bool AdvanceStage1Scene1XaFrameDriverAdapter(
    PrGameContext& ctx,
    const std::filesystem::path& stageRuntimePath,
    const PrStage1Scene1FrameDriverDirect::LoopFrameWindow& window) {
    if (window.introTransitionActive) {
        return false;
    }

    // Do not restart/pump XA or overwrite the pending native CD Stop after
    // the terminal owner has completed the shared audio reset.
    if (IsStage1TerminalAudioResetDone801C7A60()) return true;
    const uint8_t stageRecordXaChannel8001A4D0 =
        ResolveStage1StageRecordXaChannel8001A4D0();
    const auto finishEntry = [&ctx](
        const PrStage1XaCdDirectStageRecordTickResult8001A4D0& tick) {
        if (!tick.resultKnown || tick.psxReturn != 0) return false;
        ServiceStage1PendingCdQuery(ctx);
        GetStageRunner().SyncXaStartBaseline(ctx);
        Log::Printf("Scene1 stage-record8001A4D0 complete: frame=%u readS27Serial=%u postStatusSerial=%u",
            ctx.frame, ctx.stage1XaCdDirect.readS27Serial,
            ctx.stage1XaCdDirect.command1Serial);
        return true;
    };

    if (s_stage1RetryStageRecordRestartPending801C81EC) {
        ResetStage1RetryStageRecordRestart801C81EC();
        if (s_xaStarted || ctx.stage1XaCdDirect.streamStarted) {
            if (ctx.xa1Player) {
                ctx.xa1Player->Stop();
            }
            PrStage1XaCdDirectReset(ctx.stage1XaCdDirect);
            s_xaStarted = false;
        }
    }

    if (!s_xaStarted && ctx.xa1Player && !stageRuntimePath.empty()) {
        const bool ok = ctx.xa1Player->Play(stageRuntimePath);
        Log::Printf("Scene1::Fn2 XA1 start ok=%d path='%s'",
                    ok ? 1 : 0,
                    stageRuntimePath.u8string().c_str());
        s_xaStarted = ok;
        if (ok) {
            PrStage1XaCdDirectStartInput xaStartInput{};
            xaStartInput.segPresent = true;
            xaStartInput.cdlFilePosBcd = PackStage1XaStartCdlFilePosBcd();
            xaStartInput.initialChannel = stageRecordXaChannel8001A4D0;
            xaStartInput.mode1Streaming = false;
            const PrStage1XaCdDirectStageRecordTickResult8001A4D0
                xaStart =
                    PrStage1XaCdDirectApplySub8001A4D0StageRecordTick(
                      ctx.stage1XaCdDirect,
                      xaStartInput);
            bool setFilterAccepted = false;
            if (xaStart.start.started) {
                setFilterAccepted =
                    ctx.xa1Player->SetFilter(xaStart.start.file,
                                             xaStart.start.channel);
            }
            PrStage1XaCdDirectStartInput commandCompleteInput =
                xaStartInput;
            commandCompleteInput.cdCommandCompletionKnown =
                xaStart.start.started && setFilterAccepted;
            commandCompleteInput.cdCommandTimedOut = false;
            commandCompleteInput.cdCommandSyncResultKnown =
                commandCompleteInput.cdCommandCompletionKnown;
            commandCompleteInput.cdCommandSyncResult = 2;
            const PrStage1XaCdDirectStageRecordTickResult8001A4D0
                tickResult =
                    PrStage1XaCdDirectApplySub8001A4D0StageRecordTick(
                        ctx.stage1XaCdDirect,
                        commandCompleteInput);
            return finishEntry(tickResult);
        } else {
            PrStage1XaCdDirectReset(ctx.stage1XaCdDirect);
            return false;
        }
    }

    if (s_xaStarted) {
        // Continue an incomplete entry action only. Steady gameplay no longer
        // returns to this action or reissues the initialization tail.
        PrStage1XaCdDirectStartInput input{};
        input.segPresent = true;
        input.cdlFilePosBcd = PackStage1XaStartCdlFilePosBcd();
        input.initialChannel = stageRecordXaChannel8001A4D0;
        input.mode1Streaming = false;
        input.cdCommandCompletionKnown = true;
        input.cdCommandTimedOut = false;
        input.cdCommandSyncResultKnown = true;
        input.cdCommandSyncResult = 2;
        const PrStage1XaCdDirectStageRecordTickResult8001A4D0 tickResult =
            PrStage1XaCdDirectApplySub8001A4D0StageRecordTick(
                ctx.stage1XaCdDirect,
                input);
        return finishEntry(tickResult);
    }
    return false;
}

static void AdvanceStage1RunningXaFrame(PrGameContext& ctx) {
    if (IsStage1TerminalAudioResetDone801C7A60()) return;
    // Once 801C7560 is registered, the shared VBlank dispatcher owns XA
    // service and 1A3C8/1A280. The 30Hz scorer must not pump that queue again.
    if (IsStage1TimecodeVblankBound()) return;
    if (!IsStage1TimecodeVblankBound() && ctx.xa1Player && ctx.xa1Player->IsPlaying())
        ctx.xa1Player->Update();
    // Finish the previous native query before the scorer can issue another.
    // Publish current sector headers afterwards, keeping status bytes out of
    // the BCD clock. This pumps the device, not the native entry procedure.
    ServiceStage1PendingCdQuery(ctx);
    PumpStage1XaCdDirectRingPackets(ctx);
}

static LifecycleHost801C81EC::StageRunnerHostResult801C81EC
RunStage1Scene1RunnerFrameDriverAdapter(
    PrGameContext& ctx,
    const PrStage1Scene1FrameDriverDirect::LoopFrameWindow& window);
static Stage1FormalLifecycleFrameInputs BuildStage1FormalLifecycleFrameInputs(
    PrGameContext& ctx,
    const PrStageRunner& runner);

static bool s_stage1TerminalCleanupDrainActive801C7A60 = false;
static int32_t s_stage1TerminalCleanupDrainResult801C7A60 = 0;
static bool s_stage1TerminalAudioResetDone801C7A60 = false;
static bool s_stage1TerminalVblankCleared801C7A60 = false;
static PrStage1CdStopRuntime8001A694 s_stage1TerminalCdStop801C7A60{};
static PrStage1CdStopExecutionDirect::State s_stage1TerminalCdExecution801C7A60{};
static PrStage1CdStopExecutionDirect::DeviceReply s_stage1TerminalCdDeviceReply{};
static uint64_t s_stage1TerminalCdDeviceSerial = 0;

static bool IsStage1TerminalAudioResetDone801C7A60() {
    return s_stage1TerminalCleanupDrainActive801C7A60 && s_stage1TerminalAudioResetDone801C7A60;
}

void ServiceStage1TimecodeCdVblank(PrGameContext& ctx) {
    if (IsStage1TerminalAudioResetDone801C7A60()) return;
    if (ctx.xa1Player && ctx.xa1Player->IsPlaying()) ctx.xa1Player->Update(1);
    ServiceStage1PendingCdQuery(ctx);
    PumpStage1XaCdDirectRingPackets(ctx);
}

void AdvanceStage1RunnerVblankClock(PrGameContext& ctx) {
    if (ctx.currentScene != PrSceneId::Scene1) return;
    AdvanceStage1TimecodeHostClock(ctx);
}

static PrStage1CdStopExecutionDirect::DeviceReply SubmitStage1TerminalCdDevice(
    void* user, bool submit, uint8_t command, uint64_t serial) {
    auto& ctx = *static_cast<PrGameContext*>(user);
    if (submit) {
        s_stage1TerminalCdDeviceReply = {};
        s_stage1TerminalCdDeviceSerial = serial;
        if (command == 1u && ctx.xa1Player) {
            // Query the actual file-backed reader, not the destination57108
            // that36AF8 is supposed to populate. No physical motor is modeled.
            const uint8_t status = ctx.xa1Player->IsPlaying() ? 0x20u : 0u;
            s_stage1TerminalCdDeviceReply = {true, status, true, status};
        } else if (command == 8u && ctx.xa1Player) {
            const bool before = ctx.xa1Player->IsPlaying();
            ctx.xa1Player->Stop();
            const bool stopped = !ctx.xa1Player->IsPlaying();
            // The file-backed drive stops synchronously. Its idle status is
            // zero (no reading/motor/error); this is a software-device receipt,
            // NOT a PSX interrupt/MMIO or memory-replay observation.
            s_stage1TerminalCdDeviceReply = {stopped, 0u, stopped, 0u};
            Log::Printf("Scene1 terminal801C7A60: CD8 host stop serial=%llu playing=%d->%d softwareDevice=1 psxMmioAuthority=0",
                static_cast<unsigned long long>(serial), before ? 1 : 0, stopped ? 0 : 1);
        }
    }
    return serial == s_stage1TerminalCdDeviceSerial
        ? s_stage1TerminalCdDeviceReply : PrStage1CdStopExecutionDirect::DeviceReply{};
}

static void ResetStage1TerminalCleanupDrain801C7A60() {
    s_stage1TerminalCleanupDrainActive801C7A60 = false;
    s_stage1TerminalCleanupDrainResult801C7A60 = 0;
    s_stage1TerminalAudioResetDone801C7A60 = false;
    s_stage1TerminalVblankCleared801C7A60 = false;
    s_stage1TerminalCdStop801C7A60 = {};
    s_stage1TerminalCdExecution801C7A60 = {};
    s_stage1TerminalCdDeviceReply = {};
    s_stage1TerminalCdDeviceSerial = 0;
}

static bool IsStage1TerminalCleanupNumericStatusGateCleared801C7A60() {
    if (!s_stage1TerminalCleanupDrainActive801C7A60) {
        return false;
    }

    const auto& tail = s_stage1NumericRuntime.runnerMainLoopTail7A60;
    return tail.known &&
           tail.cleanupRequiredKnown &&
           tail.cleanupRequired &&
           tail.clearWord50;
}

static void ApplyStage1TerminalCleanupRenderCtxOverride801C7A60(
    PrStage1RuntimeSlotsSnapshot& snapshot) {
    if (!snapshot.valid || !s_stage1TerminalCleanupDrainActive801C7A60) {
        return;
    }

    const auto& tail = s_stage1NumericRuntime.runnerMainLoopTail7A60;
    if (!tail.known || !tail.cleanupRequiredKnown || !tail.cleanupRequired) {
        return;
    }

    if (tail.clearWord42) {
        snapshot.sceneSubmit.directScriptBoxGate54Known = true;
        snapshot.sceneSubmit.directScriptBoxGate54 = 0u;
    }
    if (tail.clearWord50) {
        snapshot.sceneSubmit.directNumericStatusGate64Known = true;
        snapshot.sceneSubmit.directNumericStatusGate64 = 0u;
    }
    if (tail.clearWord61 && snapshot.sceneSubmit.compactRail80024744.valid) {
        snapshot.sceneSubmit.compactRail80024744.painterGate7A = 0;
    }
}

static bool TryServiceStage1TerminalCleanupDrain801C7A60(
    PrGameContext& ctx,
    const PrStage1FormalLifecycleSnapshot& lifecycleSnapshot,
    LifecycleHost801C81EC::StageRunnerHostResult801C81EC& out) {
    if (!s_stage1TerminalCleanupDrainActive801C7A60) {
        if (!lifecycleSnapshot.valid ||
            lifecycleSnapshot.runnerTailCleanupRenderPassBudget != 4u ||
            !lifecycleSnapshot.runnerTailFinalReturnKnown) {
            return false;
        }
        if (!PrStage1Scene1DrawBackend::BeginTerminalPresentation801C7A60(ctx)) {
            out.known = false;
            out.result = 0;
            return true;
        }
        s_stage1TerminalCleanupDrainActive801C7A60 = true;
        s_stage1TerminalCleanupDrainResult801C7A60 =
            lifecycleSnapshot.runnerTailFinalReturn;
    } else if (lifecycleSnapshot.valid &&
               lifecycleSnapshot.runnerTailFinalReturnKnown) {
        s_stage1TerminalCleanupDrainResult801C7A60 =
            lifecycleSnapshot.runnerTailFinalReturn;
    }

    if (!PrStage1Scene1DrawBackend::CompleteTerminalDisplayMove8001B120(ctx)) {
        out.known = false;
        out.result = 0;
        return true;
    }

    if (!s_stage1TerminalAudioResetDone801C7A60) {
        const auto audioReset = PrSfx::ApplySharedAudioResetBarrier26FA4();
        if (!audioReset.committed) {
            out.known = false;
            out.result = 0;
            return true;
        }
        // The PSX driver just freed the shared voices. Invalidate the Win XA
        // cache before the subsequent CD Stop can free a recycled voice ID.
        if (ctx.xa1Player) ctx.xa1Player->NotifyAudioEngineReset();
        s_stage1TerminalAudioResetDone801C7A60 = true;
    }
    if (!s_stage1TerminalVblankCleared801C7A60) {
        LogStage1TimecodeVblankRelease();
        const auto cleared = PrPsxVblankCallbackDirect::VSyncCallback800357D4({});
        if (!cleared.known) {
            out.known = false;
            out.result = 0;
            return true;
        }
        s_stage1TerminalVblankCleared801C7A60 = true;
        Log::Printf("Scene1 terminal801C7A60: 800357D4(0) previous=%08X slot0=0 sharedClockReset=0", cleared.previous);
    }

    // The scorer's last1A280 can leave command10 outstanding, not command1.
    // Service the actual query receipt even while the Stop pre-sync waits.
    // This cannot acknowledge command8 or replace its separate completion.
    ServiceStage1PendingCdQuery(ctx, true);
    // Missing lower feedback suspends the native loop, never means success.
    auto stop = PrStage1XaCdDirectAdvanceStop8001A694(
        s_stage1TerminalCdStop801C7A60, ctx.stage1XaCdDirect);
    for (uint32_t budget = 0; !stop.complete && budget < 8u; ++budget) {
        const auto request = s_stage1TerminalCdStop801C7A60.request;
        const auto feedback = PrStage1CdStopExecutionDirect::Execute(
            s_stage1TerminalCdExecution801C7A60, ctx.stage1XaCdDirect, request,
            static_cast<int32_t>(PrPsxVSyncDirect::ProcessVSyncState80035560().vblankCounter80057034),
            SubmitStage1TerminalCdDevice, &ctx);
        if (!feedback.known) break;
        Log::Printf("Scene1 terminal801C7A60: CD lower receipt serial=%llu fn=%08X result=%d response49414Known=%d",
            static_cast<unsigned long long>(request.serial), request.function, feedback.result,
            ctx.stage1XaCdDirect.response_80049414Known ? 1 : 0);
        stop = PrStage1XaCdDirectAdvanceStop8001A694(
            s_stage1TerminalCdStop801C7A60, ctx.stage1XaCdDirect, feedback);
    }
    if (!stop.complete || !s_stage1TerminalCdStop801C7A60.returnKnown) {
        out.known = false;
        out.result = 0;
        return true;
    }
    s_xaStarted = false;
    Log::Printf("Scene1 terminal801C7A60: 8001A694 complete oldCallback=%08X callbackNow=%08X readS27Serial=%u readyCallback=%08X fullCdReset=0",
        static_cast<uint32_t>(s_stage1TerminalCdStop801C7A60.result), ctx.stage1XaCdDirect.dword_800570F8,
        ctx.stage1XaCdDirect.readS27Serial, ctx.stage1XaCdDirect.dword_800570FC);
    out.known = true;
    out.result = s_stage1TerminalCleanupDrainResult801C7A60;
    if (out.result != 0) {
        ArmStage1RetryStageRecordRestart801C81EC();
    }
    ResetStage1TerminalCleanupDrain801C7A60();
    return true;
}

static LifecycleHost801C81EC::StageRunnerHostResult801C81EC
RunStage1Scene1RunnerFrameDriverAdapter(
    PrGameContext& ctx,
    const PrStage1Scene1FrameDriverDirect::LoopFrameWindow& window) {
    LifecycleHost801C81EC::StageRunnerHostResult801C81EC out{};
    if (window.introTransitionActive) {
        return out;
    }

    PrStageRunner& runner = GetStageRunner();
    ctx.stageRunning = true;
    if (runner.GetState() == StageRunnerState::Idle ||
        runner.GetSceneId() != 1) {
        ResetStage1FormalLifecycleRuntime();
        ResetStage1TerminalFormalLifecycleSnapshot();
        ResetStage1TerminalCleanupDrain801C7A60();
        // Stage1's PSX caller seam is the wrapper around runner entry
        // (`sub_801C79E4`), not the generic `PrStageRunner::Init` body itself.
        PrScn1::ApplyStage1RunnerDirectInitLifecycle(ctx);
        runner.Init(1, ctx);
        MarkStage1IntroTransitionPrerollPending801C7A60();
    }
    if (ArmStage1IntroTransitionPreroll801C7A60()) {
        return out;
    }

    if (window.resetIntroConsumer) {
        // StageRecordTick1A4D0 starts XA before this split runner entry. On the
        // first post-preroll frame, re-sample the baseline so gameplay tick 0
        // does not inherit the intro-transition XA advance.
        runner.SyncXaStartBaseline(ctx);
    }

    PrStage1FormalLifecycleSnapshot lifecycleSnapshot{};
    if (CopyStage1FormalLifecycleSnapshot(lifecycleSnapshot) &&
        TryServiceStage1TerminalCleanupDrain801C7A60(ctx, lifecycleSnapshot,
                                                     out)) {
        return out;
    }

    AdvanceStage1RunningXaFrame(ctx);
    (void)runner.Update(ctx);
    // Retain the pending801C7A60 action until its native terminal tail returns.
    // Publishing known/0 here used to route ordinary frames back through1A4D0.
    ApplyStage1PsxXaSetFilter13Request(ctx);
    ApplyStage1RunnerTailHostAudioActions7A60(ctx);
    (void)PrimeStage1RuntimeSlotPlayer(ctx);
    const Stage1FormalLifecycleFrameInputs lifecycleInputs =
        BuildStage1FormalLifecycleFrameInputs(
            ctx,
            runner);
    (void)PrimeStage1FormalLifecycleRuntime(
        ctx,
        runner,
        lifecycleInputs);
    if (CopyStage1FormalLifecycleSnapshot(lifecycleSnapshot) &&
        TryServiceStage1TerminalCleanupDrain801C7A60(ctx, lifecycleSnapshot,
                                                     out)) {
        return out;
    }
    if (lifecycleSnapshot.valid &&
        (lifecycleSnapshot.clearGate || lifecycleSnapshot.failGate)) {
        out.known = false;
        out.result = 0;
        return out;
    }
    // Scene1's host runner can hit its Win duration without a PSX clear
    // decision. Keep the split 801C7A60 action open until the direct
    // final-return rule has closed the PSX result family.
    return out;
}

static Stage1FormalLifecycleFrameInputs BuildStage1FormalLifecycleFrameInputs(
    PrGameContext& ctx,
    const PrStageRunner& runner) {
    Stage1FormalLifecycleFrameInputs inputs{};
    inputs.introTransitionActive = s_stage1IntroTransitionActive;
    inputs.lateBranchFlag40Active =
        (s_stage1EventStreamRuntime9094.flags40_2000_4000 & 0x40u) != 0u;
    inputs.lateBranchSelectedStream =
        s_stage1EventStreamRuntime9094.selectedStream;
    inputs.lateBranchActiveDispatchStream =
        s_stage1EventStreamRuntime9094.activeDispatchStream;
    inputs.lateBranchFlag100BlocksWaitActive =
        s_stage1EventStreamRuntime9094.flag100BlocksWaitPulse;
    inputs.lateBranchFlag100SourceStream =
        s_stage1EventStreamRuntime9094.flag100SourceStream;
    inputs.lateBranchFirstFlag100PulseKnown =
        s_stage1EventStreamRuntime9094.firstTerminalFlag100PulseKnown;
    inputs.lateBranchFirstFlag100PulseQueryFrame =
        s_stage1EventStreamRuntime9094.firstTerminalFlag100PulseQueryFrame;
    inputs.lateBranchFirstFlag100PulseScriptFrame =
        s_stage1EventStreamRuntime9094.firstTerminalFlag100PulseScriptFrame;
    inputs.lateBranchFirstFlag100PulseSourceStream =
        s_stage1EventStreamRuntime9094.firstTerminalFlag100PulseSourceStream;
    inputs.lateBranchFirstFlag100PulseReason =
        s_stage1EventStreamRuntime9094.firstTerminalFlag100PulseReason;
    inputs.lateBranchScriptFrame =
        s_stage1EventStreamRuntime9094.lastAdvanceScriptFrame;
    inputs.lateBranchClearTerminalTailPulseInput =
        s_stage1EventStreamRuntime9094.lastClearTerminalTailPulseInput;
    inputs.lateBranchClearTerminalTailPulseArmed =
        s_stage1EventStreamRuntime9094.lastClearTerminalTailPulseArmed;
    inputs.lateBranchClearTerminalTailPulseBlockedAlreadyArmed =
        s_stage1EventStreamRuntime9094
            .lastClearTerminalTailPulseBlockedAlreadyArmed;
    inputs.lateBranchClearTerminalTailPulseBlockedActiveDispatch =
        s_stage1EventStreamRuntime9094
            .lastClearTerminalTailPulseBlockedActiveDispatch;
    inputs.lateBranchClearTerminalTailPulseBlockedPendingMismatch =
        s_stage1EventStreamRuntime9094
            .lastClearTerminalTailPulseBlockedPendingMismatch;
    inputs.lateBranchClearTerminalTailPulseStream =
        s_stage1EventStreamRuntime9094.lastClearTerminalTailPulseStream;
    inputs.lateBranchClearTerminalTailLatchSetKnown =
        s_stage1EventStreamRuntime9094.lastClearTerminalTailLatchSetKnown;
    inputs.lateBranchClearTerminalTailLatchSetQueryFrame =
        s_stage1EventStreamRuntime9094.lastClearTerminalTailLatchSetQueryFrame;
    inputs.lateBranchClearTerminalTailLatchSetScriptFrame =
        s_stage1EventStreamRuntime9094.lastClearTerminalTailLatchSetScriptFrame;
    inputs.lateBranchClearTerminalTailLatchSetRightRankRow =
        s_stage1EventStreamRuntime9094
            .lastClearTerminalTailLatchSetRightRankRow;
    inputs.lateBranchClearTerminalTailLatchSetCurrentMode =
        s_stage1EventStreamRuntime9094
            .lastClearTerminalTailLatchSetCurrentMode;
    inputs.lateBranchClearTerminalTailLatchSetStream =
        s_stage1EventStreamRuntime9094.lastClearTerminalTailLatchSetStream;
    inputs.lateBranchClearTerminalBranchTriggerAttempted =
        s_stage1EventStreamRuntime9094
            .lastClearTerminalBranchTriggerAttempted;
    inputs.lateBranchClearTerminalBranchTriggerAccepted =
        s_stage1EventStreamRuntime9094
            .lastClearTerminalBranchTriggerAccepted;
    inputs.lateBranchClearTerminalBranchTriggerScriptFrame =
        s_stage1EventStreamRuntime9094
            .lastClearTerminalBranchTriggerScriptFrame;
    inputs.lateBranchClearTerminalBranchTriggerRightRankRow =
        s_stage1EventStreamRuntime9094
            .lastClearTerminalBranchTriggerRightRankRow;
    inputs.lateBranchClearTerminalBranchTriggerCurrentMode =
        s_stage1EventStreamRuntime9094
            .lastClearTerminalBranchTriggerCurrentMode;
    inputs.lateBranchClearTerminalBranchTriggerBlockedConsumed =
        s_stage1EventStreamRuntime9094
            .lastClearTerminalBranchTriggerBlockedConsumed;
    inputs.lateBranchClearTerminalBranchTriggerBlockedArmed =
        s_stage1EventStreamRuntime9094
            .lastClearTerminalBranchTriggerBlockedArmed;
    inputs.lateBranchClearTerminalBranchTriggerBlockedFlagNotOne =
        s_stage1EventStreamRuntime9094
            .lastClearTerminalBranchTriggerBlockedFlagNotOne;
    inputs.lateBranchClearTerminalBranchTriggerBlockedRow =
        s_stage1EventStreamRuntime9094
            .lastClearTerminalBranchTriggerBlockedRow;
    inputs.lateBranchClearTerminalBranchTriggerBlockedStreamMissing =
        s_stage1EventStreamRuntime9094
            .lastClearTerminalBranchTriggerBlockedStreamMissing;
    inputs.lateBranchClearTerminalBranchTriggerBlockedCursorDone =
        s_stage1EventStreamRuntime9094
            .lastClearTerminalBranchTriggerBlockedCursorDone;
    inputs.lateBranchClearTerminalBranchTriggerBlockedEventNotDue =
        s_stage1EventStreamRuntime9094
            .lastClearTerminalBranchTriggerBlockedEventNotDue;
    inputs.lateBranchClearTerminalBranchTriggerBlockedMissingFlag80 =
        s_stage1EventStreamRuntime9094
            .lastClearTerminalBranchTriggerBlockedMissingFlag80;
    inputs.lateBranchClearTerminalBranchTriggerStreamFlag =
        s_stage1EventStreamRuntime9094
            .lastClearTerminalBranchTriggerStreamFlag;
    inputs.lateBranchClearTerminalBranchTriggerAttemptCount =
        s_stage1EventStreamRuntime9094
            .clearTerminalBranchTriggerAttemptCount;
    inputs.lateBranchClearTerminalBranchTriggerAcceptedCount =
        s_stage1EventStreamRuntime9094
            .clearTerminalBranchTriggerAcceptedCount;
    inputs.lateBranchClearTerminalBranchTriggerEligibleCount =
        s_stage1EventStreamRuntime9094
            .clearTerminalBranchTriggerEligibleCount;
    inputs.lateBranchClearTerminalBranchTriggerAcceptedKnown =
        s_stage1EventStreamRuntime9094
            .clearTerminalBranchTriggerAcceptedKnown;
    inputs.lateBranchClearTerminalBranchTriggerAcceptedQueryFrame =
        s_stage1EventStreamRuntime9094
            .clearTerminalBranchTriggerAcceptedQueryFrame;
    inputs.lateBranchClearTerminalBranchTriggerAcceptedScriptFrame =
        s_stage1EventStreamRuntime9094
            .clearTerminalBranchTriggerAcceptedScriptFrame;
    inputs.lateBranchClearTerminalBranchTriggerAcceptedRightRankRow =
        s_stage1EventStreamRuntime9094
            .clearTerminalBranchTriggerAcceptedRightRankRow;
    inputs.lateBranchClearTerminalBranchTriggerAcceptedCurrentMode =
        s_stage1EventStreamRuntime9094
            .clearTerminalBranchTriggerAcceptedCurrentMode;
    inputs.lateBranchClearTerminalBranchTriggerAcceptedStreamFlag =
        s_stage1EventStreamRuntime9094
            .clearTerminalBranchTriggerAcceptedStreamFlag;
    inputs.lateBranchClearTerminalBranchTriggerAcceptedStream1Cursor =
        s_stage1EventStreamRuntime9094
            .clearTerminalBranchTriggerAcceptedStream1Cursor;
    inputs.lateBranchClearTerminalBranchTriggerAcceptedStream1Count =
        s_stage1EventStreamRuntime9094
            .clearTerminalBranchTriggerAcceptedStream1Count;
    inputs.lateBranchClearTerminalBranchTriggerAcceptedStream1DueKnown =
        s_stage1EventStreamRuntime9094
            .clearTerminalBranchTriggerAcceptedStream1DueKnown;
    inputs.lateBranchClearTerminalBranchTriggerAcceptedStream1DueFrame =
        s_stage1EventStreamRuntime9094
            .clearTerminalBranchTriggerAcceptedStream1DueFrame;
    inputs.lateBranchClearTerminalBranchTriggerAcceptedStream1DueDelta =
        s_stage1EventStreamRuntime9094
            .clearTerminalBranchTriggerAcceptedStream1DueDelta;
    inputs.lateBranchClearTerminalBranchTriggerAcceptedStream1BaseFrame =
        s_stage1EventStreamRuntime9094
            .clearTerminalBranchTriggerAcceptedStream1BaseFrame;
    inputs.lateBranchClearTerminalBranchTriggerAcceptedStream1AbsDueFrame =
        s_stage1EventStreamRuntime9094
            .clearTerminalBranchTriggerAcceptedStream1AbsDueFrame;
    inputs.lateBranchClearTerminalBranchTriggerAcceptedStream1AbsDueDelta =
        s_stage1EventStreamRuntime9094
            .clearTerminalBranchTriggerAcceptedStream1AbsDueDelta;
    inputs.lateBranchClearTerminalBranchTriggerAcceptedStream1PsxAddr =
        s_stage1EventStreamRuntime9094
            .clearTerminalBranchTriggerAcceptedStream1PsxAddr;
    inputs.lateBranchClearTerminalBranchTriggerAcceptedStream1Flags04 =
        s_stage1EventStreamRuntime9094
            .clearTerminalBranchTriggerAcceptedStream1Flags04;
    inputs.lateBranchClearTerminalBranchTriggerAcceptedStream1Byte29 =
        s_stage1EventStreamRuntime9094
            .clearTerminalBranchTriggerAcceptedStream1Byte29;
    inputs.lateBranchClearTerminalBranchTriggerAcceptedStream1Byte30 =
        s_stage1EventStreamRuntime9094
            .clearTerminalBranchTriggerAcceptedStream1Byte30;
    inputs.lateBranchClearTerminalBranchTriggerBlockedFlagNotOneCount =
        s_stage1EventStreamRuntime9094
            .clearTerminalBranchTriggerBlockedFlagNotOneCount;
    inputs.lateBranchClearTerminalBranchTriggerBlockedRowCount =
        s_stage1EventStreamRuntime9094
            .clearTerminalBranchTriggerBlockedRowCount;
    inputs.lateBranchClearTerminalBranchTriggerBlockedFlagAndRowCount =
        s_stage1EventStreamRuntime9094
            .clearTerminalBranchTriggerBlockedFlagAndRowCount;
    inputs.lateBranchClearTerminalBranchTriggerBlockedConsumedCount =
        s_stage1EventStreamRuntime9094
            .clearTerminalBranchTriggerBlockedConsumedCount;
    inputs.lateBranchClearTerminalBranchTriggerBlockedArmedCount =
        s_stage1EventStreamRuntime9094
            .clearTerminalBranchTriggerBlockedArmedCount;
    inputs.lateBranchClearTerminalBranchTriggerBlockedStreamMissingCount =
        s_stage1EventStreamRuntime9094
            .clearTerminalBranchTriggerBlockedStreamMissingCount;
    inputs.lateBranchClearTerminalBranchTriggerBlockedCursorDoneCount =
        s_stage1EventStreamRuntime9094
            .clearTerminalBranchTriggerBlockedCursorDoneCount;
    inputs.lateBranchClearTerminalBranchTriggerBlockedEventNotDueCount =
        s_stage1EventStreamRuntime9094
            .clearTerminalBranchTriggerBlockedEventNotDueCount;
    inputs.lateBranchClearTerminalBranchTriggerBlockedMissingFlag80Count =
        s_stage1EventStreamRuntime9094
            .clearTerminalBranchTriggerBlockedMissingFlag80Count;
    inputs.lateBranchFirstClearTerminalBranchTriggerKnown =
        s_stage1EventStreamRuntime9094
            .firstClearTerminalBranchTriggerKnown;
    inputs.lateBranchFirstClearTerminalBranchTriggerScriptFrame =
        s_stage1EventStreamRuntime9094
            .firstClearTerminalBranchTriggerScriptFrame;
    inputs.lateBranchFirstClearTerminalBranchTriggerRightRankRow =
        s_stage1EventStreamRuntime9094
            .firstClearTerminalBranchTriggerRightRankRow;
    inputs.lateBranchFirstClearTerminalBranchTriggerCurrentMode =
        s_stage1EventStreamRuntime9094
            .firstClearTerminalBranchTriggerCurrentMode;
    inputs.lateBranchFirstClearTerminalBranchTriggerStreamFlag =
        s_stage1EventStreamRuntime9094
            .firstClearTerminalBranchTriggerStreamFlag;
    inputs.lateBranchFirstClearTerminalBranchTriggerEligibleKnown =
        s_stage1EventStreamRuntime9094
            .firstClearTerminalBranchTriggerEligibleKnown;
    inputs.lateBranchFirstClearTerminalBranchTriggerEligibleScriptFrame =
        s_stage1EventStreamRuntime9094
            .firstClearTerminalBranchTriggerEligibleScriptFrame;
    inputs.lateBranchFirstClearTerminalBranchTriggerEligibleRightRankRow =
        s_stage1EventStreamRuntime9094
            .firstClearTerminalBranchTriggerEligibleRightRankRow;
    inputs.lateBranchFirstClearTerminalBranchTriggerEligibleCurrentMode =
        s_stage1EventStreamRuntime9094
            .firstClearTerminalBranchTriggerEligibleCurrentMode;
    inputs.lateBranchFirstClearTerminalBranchTriggerEligibleStream1Known =
        s_stage1EventStreamRuntime9094
            .firstClearTerminalBranchTriggerEligibleStream1Known;
    inputs.lateBranchFirstClearTerminalBranchTriggerEligibleStream1Cursor =
        s_stage1EventStreamRuntime9094
            .firstClearTerminalBranchTriggerEligibleStream1Cursor;
    inputs.lateBranchFirstClearTerminalBranchTriggerEligibleStream1Count =
        s_stage1EventStreamRuntime9094
            .firstClearTerminalBranchTriggerEligibleStream1Count;
    inputs.lateBranchFirstClearTerminalBranchTriggerEligibleStream1DueKnown =
        s_stage1EventStreamRuntime9094
            .firstClearTerminalBranchTriggerEligibleStream1DueKnown;
    inputs.lateBranchFirstClearTerminalBranchTriggerEligibleStream1DueFrame =
        s_stage1EventStreamRuntime9094
            .firstClearTerminalBranchTriggerEligibleStream1DueFrame;
    inputs.lateBranchFirstClearTerminalBranchTriggerEligibleStream1DueDelta =
        s_stage1EventStreamRuntime9094
            .firstClearTerminalBranchTriggerEligibleStream1DueDelta;
    inputs.lateBranchFirstClearTerminalBranchTriggerEligibleStream1BaseFrame =
        s_stage1EventStreamRuntime9094
            .firstClearTerminalBranchTriggerEligibleStream1BaseFrame;
    inputs.lateBranchFirstClearTerminalBranchTriggerEligibleStream1AbsDueFrame =
        s_stage1EventStreamRuntime9094
            .firstClearTerminalBranchTriggerEligibleStream1AbsDueFrame;
    inputs.lateBranchFirstClearTerminalBranchTriggerEligibleStream1AbsDueDelta =
        s_stage1EventStreamRuntime9094
            .firstClearTerminalBranchTriggerEligibleStream1AbsDueDelta;
    inputs.lateBranchFirstClearTerminalBranchTriggerEligibleStream1Flags04 =
        s_stage1EventStreamRuntime9094
            .firstClearTerminalBranchTriggerEligibleStream1Flags04;
    inputs.lateBranchFirstClearTerminalBranchTriggerEligibleStream1Byte29 =
        s_stage1EventStreamRuntime9094
            .firstClearTerminalBranchTriggerEligibleStream1Byte29;
    inputs.lateBranchFirstClearTerminalBranchTriggerEligibleStream1Byte30 =
        s_stage1EventStreamRuntime9094
            .firstClearTerminalBranchTriggerEligibleStream1Byte30;
    inputs.lateBranchFirstClearTerminalBranchTriggerEligibleStream1NextFlag80SearchKnown =
        s_stage1EventStreamRuntime9094
            .firstClearTerminalBranchTriggerEligibleStream1NextFlag80SearchKnown;
    inputs.lateBranchFirstClearTerminalBranchTriggerEligibleStream1NextFlag80Known =
        s_stage1EventStreamRuntime9094
            .firstClearTerminalBranchTriggerEligibleStream1NextFlag80Known;
    inputs.lateBranchFirstClearTerminalBranchTriggerEligibleStream1NextFlag80Cursor =
        s_stage1EventStreamRuntime9094
            .firstClearTerminalBranchTriggerEligibleStream1NextFlag80Cursor;
    inputs.lateBranchFirstClearTerminalBranchTriggerEligibleStream1NextFlag80Frame =
        s_stage1EventStreamRuntime9094
            .firstClearTerminalBranchTriggerEligibleStream1NextFlag80Frame;
    inputs.lateBranchFirstClearTerminalBranchTriggerEligibleStream1NextFlag80Delta =
        s_stage1EventStreamRuntime9094
            .firstClearTerminalBranchTriggerEligibleStream1NextFlag80Delta;
    inputs.lateBranchFirstClearTerminalBranchTriggerEligibleStream1NextFlag80BaseFrame =
        s_stage1EventStreamRuntime9094
            .firstClearTerminalBranchTriggerEligibleStream1NextFlag80BaseFrame;
    inputs.lateBranchFirstClearTerminalBranchTriggerEligibleStream1NextFlag80AbsFrame =
        s_stage1EventStreamRuntime9094
            .firstClearTerminalBranchTriggerEligibleStream1NextFlag80AbsFrame;
    inputs.lateBranchFirstClearTerminalBranchTriggerEligibleStream1NextFlag80AbsDelta =
        s_stage1EventStreamRuntime9094
            .firstClearTerminalBranchTriggerEligibleStream1NextFlag80AbsDelta;
    inputs.lateBranchFirstClearTerminalBranchTriggerEligibleStream1NextFlag80Flags04 =
        s_stage1EventStreamRuntime9094
            .firstClearTerminalBranchTriggerEligibleStream1NextFlag80Flags04;
    inputs.lateBranchFirstClearTerminalBranchTriggerEligibleStream1NextFlag80Byte29 =
        s_stage1EventStreamRuntime9094
            .firstClearTerminalBranchTriggerEligibleStream1NextFlag80Byte29;
    inputs.lateBranchFirstClearTerminalBranchTriggerEligibleStream1NextFlag80Byte30 =
        s_stage1EventStreamRuntime9094
            .firstClearTerminalBranchTriggerEligibleStream1NextFlag80Byte30;
    inputs.lateBranchClearTerminalBranchTriggerStream1Cursor =
        s_stage1EventStreamRuntime9094
            .lastClearTerminalBranchTriggerStream1Cursor;
    inputs.lateBranchClearTerminalBranchTriggerStream1Count =
        s_stage1EventStreamRuntime9094
            .lastClearTerminalBranchTriggerStream1Count;
    inputs.lateBranchClearTerminalBranchTriggerStream1DueKnown =
        s_stage1EventStreamRuntime9094
            .lastClearTerminalBranchTriggerStream1DueKnown;
    inputs.lateBranchClearTerminalBranchTriggerStream1DueFrame =
        s_stage1EventStreamRuntime9094
            .lastClearTerminalBranchTriggerStream1DueFrame;
    inputs.lateBranchClearTerminalBranchTriggerStream1DueDelta =
        s_stage1EventStreamRuntime9094
            .lastClearTerminalBranchTriggerStream1DueDelta;
    inputs.lateBranchClearTerminalBranchTriggerStream1BaseFrame =
        s_stage1EventStreamRuntime9094
            .lastClearTerminalBranchTriggerStream1BaseFrame;
    inputs.lateBranchClearTerminalBranchTriggerStream1AbsDueFrame =
        s_stage1EventStreamRuntime9094
            .lastClearTerminalBranchTriggerStream1AbsDueFrame;
    inputs.lateBranchClearTerminalBranchTriggerStream1AbsDueDelta =
        s_stage1EventStreamRuntime9094
            .lastClearTerminalBranchTriggerStream1AbsDueDelta;
    inputs.lateBranchClearTerminalBranchTriggerStream1Flags04 =
        s_stage1EventStreamRuntime9094
            .lastClearTerminalBranchTriggerStream1Flags04;
    inputs.lateBranchClearTerminalBranchTriggerStream1Byte29 =
        s_stage1EventStreamRuntime9094
            .lastClearTerminalBranchTriggerStream1Byte29;
    inputs.lateBranchClearTerminalBranchTriggerStream1Byte30 =
        s_stage1EventStreamRuntime9094
            .lastClearTerminalBranchTriggerStream1Byte30;
    inputs.lateBranchFirstClearTerminalBranchTriggerEventNotDueKnown =
        s_stage1EventStreamRuntime9094
            .firstClearTerminalBranchTriggerEventNotDueKnown;
    inputs.lateBranchFirstClearTerminalBranchTriggerEventNotDueScriptFrame =
        s_stage1EventStreamRuntime9094
            .firstClearTerminalBranchTriggerEventNotDueScriptFrame;
    inputs.lateBranchFirstClearTerminalBranchTriggerEventNotDueCursor =
        s_stage1EventStreamRuntime9094
            .firstClearTerminalBranchTriggerEventNotDueCursor;
    inputs.lateBranchFirstClearTerminalBranchTriggerEventNotDueFrame =
        s_stage1EventStreamRuntime9094
            .firstClearTerminalBranchTriggerEventNotDueFrame;
    inputs.lateBranchFirstClearTerminalBranchTriggerEventNotDueDelta =
        s_stage1EventStreamRuntime9094
            .firstClearTerminalBranchTriggerEventNotDueDelta;
    inputs.lateBranchFirstClearTerminalBranchTriggerEventNotDueBaseFrame =
        s_stage1EventStreamRuntime9094
            .firstClearTerminalBranchTriggerEventNotDueBaseFrame;
    inputs.lateBranchFirstClearTerminalBranchTriggerEventNotDueAbsFrame =
        s_stage1EventStreamRuntime9094
            .firstClearTerminalBranchTriggerEventNotDueAbsFrame;
    inputs.lateBranchFirstClearTerminalBranchTriggerEventNotDueAbsDelta =
        s_stage1EventStreamRuntime9094
            .firstClearTerminalBranchTriggerEventNotDueAbsDelta;
    inputs.lateBranchFirstClearTerminalBranchTriggerEventNotDueFlags04 =
        s_stage1EventStreamRuntime9094
            .firstClearTerminalBranchTriggerEventNotDueFlags04;
    inputs.lateBranchFirstClearTerminalBranchTriggerMissingFlag80Known =
        s_stage1EventStreamRuntime9094
            .firstClearTerminalBranchTriggerMissingFlag80Known;
    inputs.lateBranchFirstClearTerminalBranchTriggerMissingFlag80ScriptFrame =
        s_stage1EventStreamRuntime9094
            .firstClearTerminalBranchTriggerMissingFlag80ScriptFrame;
    inputs.lateBranchFirstClearTerminalBranchTriggerMissingFlag80Cursor =
        s_stage1EventStreamRuntime9094
            .firstClearTerminalBranchTriggerMissingFlag80Cursor;
    inputs.lateBranchFirstClearTerminalBranchTriggerMissingFlag80Frame =
        s_stage1EventStreamRuntime9094
            .firstClearTerminalBranchTriggerMissingFlag80Frame;
    inputs.lateBranchFirstClearTerminalBranchTriggerMissingFlag80Delta =
        s_stage1EventStreamRuntime9094
            .firstClearTerminalBranchTriggerMissingFlag80Delta;
    inputs.lateBranchFirstClearTerminalBranchTriggerMissingFlag80BaseFrame =
        s_stage1EventStreamRuntime9094
            .firstClearTerminalBranchTriggerMissingFlag80BaseFrame;
    inputs.lateBranchFirstClearTerminalBranchTriggerMissingFlag80AbsFrame =
        s_stage1EventStreamRuntime9094
            .firstClearTerminalBranchTriggerMissingFlag80AbsFrame;
    inputs.lateBranchFirstClearTerminalBranchTriggerMissingFlag80AbsDelta =
        s_stage1EventStreamRuntime9094
            .firstClearTerminalBranchTriggerMissingFlag80AbsDelta;
    inputs.lateBranchFirstClearTerminalBranchTriggerMissingFlag80Flags04 =
        s_stage1EventStreamRuntime9094
            .firstClearTerminalBranchTriggerMissingFlag80Flags04;
    inputs.lateBranchClearTerminalTailServiceLatch =
        s_stage1EventStreamRuntime9094.lastClearTerminalTailServiceLatch;
    inputs.lateBranchClearTerminalTailServiceScriptFrame =
        s_stage1EventStreamRuntime9094
            .lastClearTerminalTailServiceScriptFrame;
    inputs.lateBranchClearTerminalTailServiceRightRankRow =
        s_stage1EventStreamRuntime9094
            .lastClearTerminalTailServiceRightRankRow;
    inputs.lateBranchClearTerminalTailServiceCurrentMode =
        s_stage1EventStreamRuntime9094
            .lastClearTerminalTailServiceCurrentMode;
    inputs.lateBranchClearTerminalTailServiceStream =
        s_stage1EventStreamRuntime9094.lastClearTerminalTailServiceStream;
    inputs.lateBranchPendingRatingBranchSeq =
        s_stage1EventStreamRuntime9094.pendingRatingBranchSeq;
    inputs.lateBranchConsumedRatingBranchSeq =
        s_stage1EventStreamRuntime9094.consumedRatingBranchSeq;
    inputs.lateBranchClearTerminalTailArmed =
        s_stage1EventStreamRuntime9094.clearTerminalTailArmed;
    inputs.lateBranchClearTerminalTailDispatchActive =
        s_stage1EventStreamRuntime9094.clearTerminalTailDispatchActive;
    inputs.lateBranchClearTerminalTailStream =
        s_stage1EventStreamRuntime9094.clearTerminalTailStream;
    inputs.lateBranchActiveDispatchStartScriptFrame =
        s_stage1EventStreamRuntime9094.activeDispatchStartScriptFrame;
    inputs.lateBranchActiveDispatchTerminalEndLocalFrame =
        s_stage1EventStreamRuntime9094.activeDispatchTerminalEndLocalFrame;
    inputs.lateBranchActiveDispatchTerminalPulseEmitted =
        s_stage1EventStreamRuntime9094.activeDispatchTerminalPulseEmitted;
    inputs.recordsModeDAActive = ctx.transitionStateDA != 0;
    inputs.recordsModeDAEqualsOne = ctx.transitionStateDA == 1;
    const PrStageClearStatusQueryResult stageStatus =
        PrStage1SaveUiDirect::Sub800166AC(runner.GetSceneId());
    inputs.stageStatus166ACKnown =
        stageStatus.statusBankKnown && stageStatus.ok;
    inputs.stageStatus166AC = static_cast<uint8_t>(
        std::clamp<int32_t>(stageStatus.status, 0, 0xFF));
    inputs.numericActive = s_stage1NumericRuntime.active;
    inputs.numericRightRankActiveRow =
        s_stage1NumericRuntime.rightRankState.rightRankActiveRow;
    const auto& tail = s_stage1NumericRuntime.runnerMainLoopTail7A60;
    inputs.runnerTailDirectKnown = tail.known;
    inputs.runnerTailCleanupRequiredKnown = tail.cleanupRequiredKnown;
    inputs.runnerTailCleanupRequired = tail.cleanupRequired;
    inputs.runnerTailCleanupDrainFrameCount = tail.cleanupDrainFrameCount;
    inputs.runnerTailFrameExit1A3B8Known = tail.frameExitKnown;
    inputs.runnerTailFrameExit1A3B8Taken = tail.frameExitTaken;
    return inputs;
}

static void AdvanceStage1Scene1IntroTransitionFrameDriverAdapter() {
    PrStage1Scene1FrameDriverDirect::IntroTransitionState intro =
        LoadStage1Scene1IntroTransitionState();
    const bool applyPrerollTransitionCueCadence = intro.active;
    PrStage1Scene1FrameDriverDirect::TickIntroTransition(
        intro,
        kStage1IntroTransitionMaxLife);
    if (applyPrerollTransitionCueCadence) {
        (void)PrSfx::ApplyMovieTransitionCueCadence80027194();
    }
    StoreStage1Scene1IntroTransitionState(intro);
}

static void ResetStage1LifecycleActionExecutor801C81EC() {
    PrStage1LifecycleDirect::Reset801C81ECRuntime(
        s_stage1LifecycleRuntime801C81EC);
    PrStage1LifecycleExecutorDirect::Reset801C81EC(
        s_stage1LifecycleExecutor801C81EC);
    s_stage1LifecycleDword801D3040 = 0u;
    LifecycleHost801C81EC::ResetAbortPollEvent4Overlay801C81EC();
}

static void PrimeStage1LifecycleDaStash801D3040(PrGameContext& ctx) {
    s_stage1LifecycleDword801D3040 =
        static_cast<uint32_t>(static_cast<uint16_t>(ctx.transitionStateDA));
    if (ctx.transitionState == 1 || ctx.transitionState == 2) {
        ctx.transitionStateDA = 0;
    }
}

static PrStage1LifecycleExecutorDirect::ActionApplyResult801C81EC
TickStage1InitialMovieBlock801C81EC(PrGameContext& ctx) {
    using HostBlockKind =
        PrStage1LifecycleExecutorDirect::HostBlockKind801C81EC;
    const PrStage1LifecycleExecutorDirect::HostBlockSnapshot801C81EC block =
        PrStage1LifecycleExecutorDirect::GetHostBlockSnapshot801C81EC(
            s_stage1LifecycleExecutor801C81EC);
    if (block.kind != HostBlockKind::Movie1 || !block.active) {
        return {};
    }

    SyncStage1Movie1TransitionCtxWordsFromRunnerSnapshot();
    PrStage1Scene1Movie1Direct::Movie1HostFeedback movie1Host =
        LifecycleHost801C81EC::BuildMovie1HostFeedback(
            ctx,
            block.pathResolved,
            block.path,
            false);
    movie1Host.nativePlayAndWaitComplete801C455C = s_movie1TextOuterLoopRuntime.complete;
    LifecycleHost801C81EC::AdvanceMovie1HostStrPlayer(
        ctx,
        s_movie1Runtime,
        movie1Host);
    PrStage1Scene1Movie1Direct::Movie1AdvanceResult rawMovie1Result =
        PrStage1Scene1Movie1Direct::AdvanceRuntimePure(
            s_movie1Runtime,
            movie1Host,
            PrStage1MovieTextDirect::GetActiveMovieSubtitleTrack(
                s_movie1TextRuntime));
    const PrStage1Scene1Movie1Direct::Movie1HostActionFeedback feedback =
        LifecycleHost801C81EC::ExecuteMovie1HostActions(
            ctx,
            block.pathResolved,
            block.path,
            rawMovie1Result.hostActions);
    const PrStage1Scene1Movie1Direct::Movie1HostActionFeedbackResolution
        feedbackResolution =
            PrStage1Scene1Movie1Direct::ApplyHostActionFeedback(
                s_movie1Runtime,
                rawMovie1Result,
                feedback);
    (void)LifecycleHost801C81EC::ExecuteMovie1HostActions(
        ctx,
        block.pathResolved,
        block.path,
        feedbackResolution.followupHostActions);
    const PrStage1Scene1Movie1Direct::Movie1AdvanceResult movie1Result =
        feedbackResolution.advanceResult;
    if ((movie1Host.lastStrUpdateResult == StrPlayerResult::Skipped ||
         movie1Result.completedToStage1) &&
        !s_movie1TextOuterLoopRuntime.complete) {
        PrStage1MovieTextOuterLoopDirect::
            CompleteMovieTextOuterLoopFromHostMovieEndSub801C455C(
                s_movie1TextOuterLoopRuntime,
                s_movie1TextRuntime);
    }
    if (movie1Result.completedToStage1 ||
        PrStage1Scene1Movie1Direct::IsPlayAndWaitCompletionPending(
            s_movie1Runtime)) {
        if (!s_movie1TextOuterLoopRuntime.complete) {
            return PrStage1LifecycleExecutorDirect::MakeHostBlockResult801C81EC(
                true);
        }
        const int32_t movie1PlayAndWaitResult =
            s_movie1TextOuterLoopRuntime.psxReturnValue;
        PrStage1MovieTextOuterLoopDirect::EndMovieTextOuterLoopSub801C455C(
            s_movie1TextOuterLoopRuntime,
            s_movie1TextRuntime);
        ResetStage1Movie1TextRuntime();
        return PrStage1LifecycleExecutorDirect::ApplyHostBlockFeedback801C81EC(
            s_stage1LifecycleExecutor801C81EC,
            PrStage1LifecycleExecutorDirect::
                BuildMovie1CompletedHostBlockFeedback801C81EC(
                    movie1PlayAndWaitResult));
    }
    if (movie1Result.resetStageRenderRuntime) {
        PrStage1Scene1DrawBackend::ResetGameplaySubmitRuntime();
    }
    if (movie1Result.resetTextRuntimes) {
        ResetStage1Movie1TextRuntime();
    }
    return PrStage1LifecycleExecutorDirect::MakeHostBlockResult801C81EC(true);
}

static PrStage1LifecycleExecutorDirect::ActionApplyResult801C81EC
TickStage1InitialMovie1Transition201AC801C81EC(
    PrGameContext& ctx,
    const PrStage1LifecycleDirect::Action801C81EC& action) {
    const bool preMovieTransition =
        action.transitionModeA2 == 6 &&
        action.transitionPreFfd4ArgA3 == 2 &&
        action.transitionPostFfd4ArgA4 == 1;
    const bool postMovieTransition =
        action.transitionModeA2 == 5 &&
        action.transitionPreFfd4ArgA3 == 1 &&
        action.transitionPostFfd4ArgA4 == 2;
    if (preMovieTransition) {
        PrStage1Scene1Movie1Direct::Movie1HostFeedback movie1Host =
            LifecycleHost801C81EC::BuildMovie1HostFeedback(
                ctx,
                false,
                std::filesystem::path{},
                false);
        if (PrStage1Scene1Movie1Direct::ConsumeTransitionSub800201ACCompleted(
                s_movie1Runtime)) {
            return PrStage1LifecycleExecutorDirect::
                MakeImmediateInputResult801C81EC();
        }
        if (!s_movie1Runtime.transitionSub800201ACActive &&
            s_movie1Runtime.outroTailFrames == 0u &&
            !s_movie1Runtime.outroTailCompletePending) {
            PrStage1Scene1Movie1Direct::BeginTransitionSub800201AC(
                s_movie1Runtime,
                action.transitionA1_801C3640,
                static_cast<uint32_t>(action.transitionModeA2),
                static_cast<uint32_t>(action.transitionPreFfd4ArgA3),
                static_cast<uint32_t>(action.transitionPostFfd4ArgA4),
                0u,
                false);
        }

        for (uint32_t guard = 0; guard < 256u; ++guard) {
            const PrStage1Scene1Movie1Direct::Movie1AdvanceResult
                rawMovie1Result =
                    PrStage1Scene1Movie1Direct::AdvanceRuntimePure(
                        s_movie1Runtime,
                        movie1Host,
                        PrStage1MovieTextDirect::GetActiveMovieSubtitleTrack(
                            s_movie1TextRuntime));
            const PrStage1Scene1Movie1Direct::Movie1HostActionFeedback feedback =
                LifecycleHost801C81EC::ExecuteMovie1HostActions(
                    ctx,
                    false,
                    std::filesystem::path{},
                    rawMovie1Result.hostActions);
            const PrStage1Scene1Movie1Direct::Movie1HostActionFeedbackResolution
                feedbackResolution =
                    PrStage1Scene1Movie1Direct::ApplyHostActionFeedback(
                        s_movie1Runtime,
                        rawMovie1Result,
                        feedback);
            (void)LifecycleHost801C81EC::ExecuteMovie1HostActions(
                ctx,
                false,
                std::filesystem::path{},
                feedbackResolution.followupHostActions);
            if (PrStage1Scene1Movie1Direct::
                    ConsumeTransitionSub800201ACCompleted(s_movie1Runtime)) {
                return PrStage1LifecycleExecutorDirect::
                    MakeImmediateInputResult801C81EC();
            }
            if (!rawMovie1Result.handledFrame) {
                break;
            }
        }
        return PrStage1LifecycleExecutorDirect::
            MakeBlockedActionRetryResult801C81EC();
    }
    if (!postMovieTransition) {
        return PrStage1LifecycleExecutorDirect::MakeBlockedActionRetryResult801C81EC();
    }

    SyncStage1Movie1TransitionCtxWordsFromRunnerSnapshot();
    PrStage1Scene1Movie1Direct::Movie1HostFeedback movie1Host =
        LifecycleHost801C81EC::BuildMovie1HostFeedback(
            ctx,
            false,
            std::filesystem::path{},
            false);
    if (PrStage1Scene1Movie1Direct::ConsumeTransitionSub800201ACCompleted(
            s_movie1Runtime)) {
        if (ctx.strPlayer) {
            ctx.strPlayer->Stop();
        }
        PrStage1Scene1Movie1Direct::ClearCurrentMovieFrame30(s_movie1Runtime);
        PrStage1Scene1DrawBackend::ResetGameplaySubmitRuntime();
        MarkStage1IntroTransitionPrerollPending801C7A60();
        ResetStage1Movie1TextRuntime();
        return PrStage1LifecycleExecutorDirect::MakeImmediateInputResult801C81EC();
    }
    if (!s_movie1Runtime.transitionSub800201ACActive &&
        s_movie1Runtime.outroTailFrames == 0u &&
        !s_movie1Runtime.outroTailCompletePending) {
        uint32_t sourceFrame30 = s_movie1Runtime.currentMovieFrame30;
        if (sourceFrame30 == 0u) {
            sourceFrame30 = s_movie1Runtime.outroSourceFrame30;
        }
        if (sourceFrame30 == 0u) {
            sourceFrame30 = movie1Host.movieFrame30;
        }
        PrStage1Scene1Movie1Direct::BeginTransitionSub800201AC(
            s_movie1Runtime,
            action.transitionA1_801C3640,
            static_cast<uint32_t>(action.transitionModeA2),
            static_cast<uint32_t>(action.transitionPreFfd4ArgA3),
            static_cast<uint32_t>(action.transitionPostFfd4ArgA4),
            sourceFrame30,
            false);
    }

    const PrStage1Scene1Movie1Direct::Movie1AdvanceResult rawMovie1Result =
        PrStage1Scene1Movie1Direct::AdvanceRuntimePure(
            s_movie1Runtime,
            movie1Host,
            PrStage1MovieTextDirect::GetActiveMovieSubtitleTrack(
                s_movie1TextRuntime));
    const PrStage1Scene1Movie1Direct::Movie1HostActionFeedback feedback =
        LifecycleHost801C81EC::ExecuteMovie1HostActions(
            ctx,
            false,
            std::filesystem::path{},
            rawMovie1Result.hostActions);
    const PrStage1Scene1Movie1Direct::Movie1HostActionFeedbackResolution
        feedbackResolution =
            PrStage1Scene1Movie1Direct::ApplyHostActionFeedback(
                s_movie1Runtime,
                rawMovie1Result,
                feedback);
    (void)LifecycleHost801C81EC::ExecuteMovie1HostActions(
        ctx,
        false,
        std::filesystem::path{},
        feedbackResolution.followupHostActions);

    const PrStage1Scene1Movie1Direct::Movie1AdvanceResult movie1Result =
        feedbackResolution.advanceResult;
    if (movie1Result.resetStageRenderRuntime) {
        PrStage1Scene1DrawBackend::ResetGameplaySubmitRuntime();
    }
    if (movie1Result.resetTextRuntimes) {
        ResetStage1Movie1TextRuntime();
    }
    if (PrStage1Scene1Movie1Direct::ConsumeTransitionSub800201ACCompleted(
            s_movie1Runtime)) {
        if (ctx.strPlayer) {
            ctx.strPlayer->Stop();
        }
        PrStage1Scene1Movie1Direct::ClearCurrentMovieFrame30(s_movie1Runtime);
        PrStage1Scene1DrawBackend::ResetGameplaySubmitRuntime();
        MarkStage1IntroTransitionPrerollPending801C7A60();
        ResetStage1Movie1TextRuntime();
        return PrStage1LifecycleExecutorDirect::MakeImmediateInputResult801C81EC();
    }
    return PrStage1LifecycleExecutorDirect::MakeBlockedActionRetryResult801C81EC();
}

static PrStage1LifecycleExecutorDirect::FrameHostInput801C81EC
BuildStage1LifecycleFrameHostInput801C81EC(PrGameContext& ctx) {
    PrStage1LifecycleExecutorDirect::FrameHostInput801C81EC host{};
    host.sceneId = 1u;
    host.word800916D0 = static_cast<uint16_t>(ctx.transitionState);
    host.word800916DA = static_cast<uint16_t>(ctx.transitionStateDA);
    (void)PrMain::TryPublishWord800916F0FromStage1RuntimePsxMemoryProvider(ctx);
    host.word800916F0Known =
        PrSS0Scene0RuntimeDirect::TryReadWord800916F0Software(host.word800916F0);
    static bool s_loggedUnknownF0Source801C81EC = false;
    if (!host.word800916F0Known && !s_loggedUnknownF0Source801C81EC) {
        s_loggedUnknownF0Source801C81EC = true;
        Log::Printf(
            "Stage1 lifecycle direct: word_800916F0 source unknown; "
            "post-clear save/UI gate will block");
    }
    host.word80091816 = static_cast<uint16_t>(
        std::clamp<int32_t>(
            s_stage1NumericRuntime.scorerPort.accumulator91816,
            0,
            0xFFFF));
    host.dword801D3040 = s_stage1LifecycleDword801D3040;
    host.byte801C368E =
        s_stage1NumericRuntime.rightRankState.rightRankActiveRow != 0u;
    return host;
}

static LifecycleHost801C81EC::ActionHostRefs801C81EC
BuildStage1LifecycleActionHostRefs801C81EC(PrGameContext& ctx) {
    return LifecycleHost801C81EC::ActionHostRefs801C81EC{
        s_stage1LifecycleExecutor801C81EC,
        s_stage1RuntimeSlotPlayer,
        s_stage1NumericRuntime,
        s_stage1AcceptedProducerReplayBackupRuntime,
        s_movie1TextRuntime,
        s_movie1TextOuterLoopRuntime,
        &s_movie1Runtime,
        GetStage1CommonLyricsLanguageIndex(ctx),
        TickStage1InitialMovieBlock801C81EC,
        TickStage1InitialMovie1Transition201AC801C81EC,
        RunStage1Scene1RunnerFrameDriverAdapter,
        CaptureStage1AcceptedProducerReplayBackup1635C,
        AdvanceStage1Scene1XaFrameDriverAdapter};
}

static int TickStage1Scene1_801C81EC_ActionExecutor(PrGameContext& ctx) {
    PrStage1Scene1Movie1Direct::ApplyTransitionStateMovieBoundaryPrelude(
        s_movie1Runtime,
        static_cast<uint16_t>(ctx.transitionState));

    (void)TickStage1InitialMovieBlock801C81EC(ctx);
    (void)LifecycleHost801C81EC::TickClearTailMovieBlock(
        ctx,
        s_stage1LifecycleExecutor801C81EC,
        s_movie1TextOuterLoopRuntime,
        s_movie1TextRuntime,
        &s_movie1Runtime);
    LifecycleHost801C81EC::TickAbortPollBlock(
        ctx,
        s_stage1LifecycleExecutor801C81EC);
    LifecycleHost801C81EC::TickBootstrap15590Block(
        ctx,
        s_stage1LifecycleExecutor801C81EC);
    LifecycleHost801C81EC::TickSaveUi19148Block(
        ctx,
        s_stage1LifecycleExecutor801C81EC);

    PrStage1Scene1FrameDriverDirect::IntroTransitionState intro =
        LoadStage1Scene1IntroTransitionState();
    PrStage1Scene1FrameDriverDirect::LoopFrameWindow frameWindow{};
    frameWindow.introTransitionActive = intro.active;
    frameWindow.resetIntroConsumer =
        !frameWindow.introTransitionActive && intro.consumerResetPending;
    intro.renderActive = frameWindow.introTransitionActive;
    StoreStage1Scene1IntroTransitionState(intro);
    if (frameWindow.resetIntroConsumer) {
        PrStage1Scene1FrameDriverDirect::IntroTransitionState introReset =
            LoadStage1Scene1IntroTransitionState();
        introReset.consumerResetPending = false;
        StoreStage1Scene1IntroTransitionState(introReset);
    }
    LifecycleHost801C81EC::ActionHostRefs801C81EC pendingActionHost =
        BuildStage1LifecycleActionHostRefs801C81EC(ctx);
    LifecycleHost801C81EC::DrainPendingActions801C81EC(
        ctx,
        s_stage1LifecycleRuntime801C81EC,
        frameWindow,
        pendingActionHost);
    if (PrStage1LifecycleExecutorDirect::HasPendingActions801C81EC(
            s_stage1LifecycleExecutor801C81EC)) {
        AdvanceStage1Scene1IntroTransitionFrameDriverAdapter();
        SyncStage1Scene1FrameDriverTextProducers(ctx);
        return PrStage1Scene1FrameDriverDirect::kSceneResultContinue;
    }

    int deferredSceneResult = 0;
    if (PrStage1LifecycleExecutorDirect::PopReadyDeferredSceneResult801C81EC(
            s_stage1LifecycleExecutor801C81EC,
            deferredSceneResult)) {
        AdvanceStage1Scene1IntroTransitionFrameDriverAdapter();
        SyncStage1Scene1FrameDriverTextProducers(ctx);
        LatchStage1TerminalFormalLifecycleSnapshot();
        ResetStage1SceneLoopRuntime(ctx);
        return deferredSceneResult;
    }

    if (PrStage1LifecycleExecutorDirect::HasDeferredSceneResult801C81EC(
            s_stage1LifecycleExecutor801C81EC)) {
        AdvanceStage1Scene1IntroTransitionFrameDriverAdapter();
        SyncStage1Scene1FrameDriverTextProducers(ctx);
        return PrStage1Scene1FrameDriverDirect::kSceneResultContinue;
    }

    LifecycleHost801C81EC::ActionHostRefs801C81EC actionHost =
        BuildStage1LifecycleActionHostRefs801C81EC(ctx);
    const LifecycleHost801C81EC::LifecycleFrameTransactionResult801C81EC
        frameTransaction =
            LifecycleHost801C81EC::RunLifecycleFrameTransaction801C81EC(
                ctx,
                s_stage1LifecycleRuntime801C81EC,
                s_stage1LifecycleScene801C7284,
                BuildStage1LifecycleFrameHostInput801C81EC,
                frameWindow,
                actionHost);

    AdvanceStage1Scene1IntroTransitionFrameDriverAdapter();
    SyncStage1Scene1FrameDriverTextProducers(ctx);

    if (frameTransaction.sceneResultKnown) {
        LatchStage1TerminalFormalLifecycleSnapshot();
        ResetStage1SceneLoopRuntime(ctx);
        return frameTransaction.sceneResult;
    }
    return PrStage1Scene1FrameDriverDirect::kSceneResultContinue;
}

void CaptureStage1AcceptedProducerReplayBackup1635C(
    uint32_t prevGrade92F40,
    const Stage1NumericRuntimeState::AcceptedProducerReplayBufferRuntime& replay,
    Stage1AcceptedProducerReplayBackupRuntime& backup);

Stage1EventStreamFrameUpdate9094Result RunStage1EventStreamFrameUpdate9094(
    const PrStage1OverlayData& data,
    int tick96,
    uint32_t queryFrame,
    bool sameFrameCtxKnown,
    const PrStageRunnerDirectContext801C9094& sameFrameCtx,
    bool sameFrameGlobalsKnown,
    const PrStageRunnerDirectGlobals801C9094& sameFrameGlobals,
    bool allowSameQueryRefresh,
    bool clearTerminalTailPulse) {
    Stage1EventStreamFrameUpdate9094Result out{};
    const PrStageRunnerDirectGlobals801C9094 runtimeSlotGlobals =
        PrStage1RuntimeSlotsDirectApplyRunnerGlobalsCarrier801C9094(
            sameFrameGlobals,
            PrStage1RuntimeSlotsDirectBuildRunnerGlobalsCarrier801C9094(
                s_stage1RuntimeSlotPlayer));
    const PrStageEventDirectStage1FrameUpdateResult801C9094 directResult =
        PrStageEventDirectStage1RunFrameUpdate801C9094(
            s_stage1EventStreamRuntime9094,
            data,
            BuildStage1EventStreamFrameInput801C9094(
                s_stage1NumericRuntime,
                tick96,
                queryFrame,
                allowSameQueryRefresh,
                clearTerminalTailPulse),
            sameFrameCtxKnown,
            sameFrameCtx,
            sameFrameGlobalsKnown,
            runtimeSlotGlobals);
    out.ran = directResult.ran;
    out.runnerInput = directResult.runnerInput;
    out.frameUpdate = directResult.frameUpdate;
    out.ctx0FinalFlags = directResult.ctx0FinalFlags;
    s_stage1NumericRuntime.psxEventStreamFlagKnown = true;
    s_stage1NumericRuntime.psxEventStreamFlagActive =
        s_stage1EventStreamRuntime9094.eventStreamFlagActive;
    s_stage1NumericRuntime.psxEventStreamIdKnown = true;
    s_stage1NumericRuntime.psxEventStreamId =
        static_cast<uint8_t>(
            s_stage1EventStreamRuntime9094.gPrStageEventStreamId & 0xFFu);
    s_stage1NumericRuntime.psxCtxFlags40_2000_4000Known = true;
    s_stage1NumericRuntime.psxCtxFlags40_2000_4000 =
        s_stage1EventStreamRuntime9094.flags40_2000_4000;
    s_stage1NumericRuntime.psxFlag100BlocksWaitPulse =
        s_stage1EventStreamRuntime9094.flag100BlocksWaitPulse;
    s_stage1NumericRuntime.psxFlag100BlocksWaitSourceStream =
        s_stage1EventStreamRuntime9094.flag100SourceStream;
    s_stage1NumericRuntime.rightRankForcedGoodEventStreamDone28 =
        s_stage1EventStreamRuntime9094.gPrStageEventStreamDone;
    return out;
}

static void ResetStage1SceneLoopRuntime(PrGameContext& ctx) {
    if (ctx.xa1Player) {
        ctx.xa1Player->Stop();
    }
    // This is 801C81EC returning to resident 80015D18, not a CD cold boot.
    // 801C7A60's native tail stops CD via 8001A694 (command 8, then
    // 80036510(0)); it does not erase the SCUS CD globals. 80015788 may
    // immediately call 80015590, whose read setup saves those callback slots
    // and consumes the retained clock / request state. Keep the host voice
    // stop above, but leave shared software state to its translated owners.
    Log::Printf("Scene1 exit: retain resident CD state sync=%d/%08X ready=%d/%08X readS27Serial=%u",
        ctx.stage1XaCdDirect.dword_800570F8Known ? 1 : 0,
        ctx.stage1XaCdDirect.dword_800570F8,
        ctx.stage1XaCdDirect.dword_800570FCKnown ? 1 : 0,
        ctx.stage1XaCdDirect.dword_800570FC,
        ctx.stage1XaCdDirect.readS27Serial);
    if (ctx.strPlayer) {
        ctx.strPlayer->Stop();
    }
    PrTransition::StopLoadingCurtain1545C();
    ResetStage1NumericRuntimeState();
    ResetStage1RetryStageRecordRestart801C81EC();
    const PrStage1RuntimeSlotsDirectGlobalOptionsCarrier globalOptions =
        BuildStage1GlobalOptionsCarrier801C7A60(ctx);
    const PrStageEventDirectResetAction80024F8C resetAction =
        PrStage1RuntimeSlotsDirectResetEventAndSlots80024E98(
            s_stage1EventStreamRuntime9094,
            s_stage1RuntimeSlotPlayer,
            globalOptions);
    (void)ApplyStage1ScorerHostResetAction80024F8C(
        s_stage1NumericRuntime,
        resetAction);
    ResetStage1LifecycleActionExecutor801C81EC();
    ResetStage1TerminalCleanupDrain801C7A60();
    GetStageRunner().Reset();
    if (!PrSS0Scene0RuntimeDirect::RuntimeEnabled()) {
        PrSqevs1::Shutdown(ctx);
    }
    ctx.stageRunning = false;
    PrStage1Scene1Movie1Direct::ResetRuntime(s_movie1Runtime);
    ResetStage1DirectMovie1Mdec(ctx);
    s_xaStarted = false;
    s_stage1IntroTransitionActive = false;
    s_stage1IntroTransitionRenderActive = false;
    s_stage1IntroTransitionConsumerResetPending = false;
    s_stage1IntroTransitionFrame = 0;
    PrStage1Scene1DrawBackend::ResetGameplaySubmitRuntime();
    ResetStage1Movie1TextRuntime();
    PrStage1SaveUiDirect::Reset19148();
    PrStage1SaveUiHostBridgeDirect::ResetSaveUiPresentation19148();
}

static bool IsStage1ClearResultMovieVisualActive() {
    return PrStage1LifecycleExecutorDirect::
        IsClearTailMovieVisualActive801C81EC(
            s_stage1LifecycleExecutor801C81EC);
}

int Fn0(PrGameContext& ctx) {
    Log::Printf("Scene1::Fn0 init global pointers");
    s_stage1LifecycleGlobals801CA3BC =
        PrStage1LifecycleDirect::InitGlobals801CA3BC();
    Log::Printf(
        "Scene1::Fn0 direct 801CA3BC globals 943C8=%u 943CC=0x%08X 94440=0x%08X",
        (unsigned)s_stage1LifecycleGlobals801CA3BC.value800943C8,
        (unsigned)s_stage1LifecycleGlobals801CA3BC.ptr800943CC,
        (unsigned)s_stage1LifecycleGlobals801CA3BC.fn80094440);
    ctx.stageRunning = false;
    ctx.sceneExitReason = 0;
    ResetStage1NumericRuntimeState();
    GetStageRunner().Reset();
    if (ctx.strPlayer) {
        ctx.strPlayer->Stop();
    }
    if (ctx.xa1Player) {
        ctx.xa1Player->Stop();
    }
    PrStage1XaCdDirectReset(ctx.stage1XaCdDirect);
    s_xaStarted = false;
    ResetStage1RetryStageRecordRestart801C81EC();
    PrStage1MovieTextDirect::ResetSub80024C84(s_movie1TextRuntime);
    s_movie1TextWindowTickResult =
        PrStage1MovieTextDirect::Movie1TextWindowTickResult{};
    PrStage1MovieTextOuterLoopDirect::ResetMovieTextOuterLoopSub801C455C(
        s_movie1TextOuterLoopRuntime);
    PrStage1Scene1Movie1Direct::ResetRuntime(s_movie1Runtime);
    ResetStage1DirectMovie1Mdec(ctx);
    ResetStage1LifecycleActionExecutor801C81EC();
    s_stage1IntroTransitionActive = false;
    s_stage1IntroTransitionRenderActive = false;
    s_stage1IntroTransitionConsumerResetPending = false;
    s_stage1IntroTransitionFrame = 0;
    PrStage1Scene1DrawBackend::ResetGameplaySubmitRuntime();
    PrStage1OverlayScriptTextDirectResetSub801C8604(
        s_stage1OverlayScriptTextDirect);
    ResetStage1FormalLifecycleRuntime();
    ResetStage1TerminalFormalLifecycleSnapshot();
    ResetStage1TerminalCleanupDrain801C7A60();
    return s_stage1LifecycleGlobals801CA3BC.returnValue;
}
void Fn1(PrGameContext& ctx) {
    const std::filesystem::path stage1CompoPath =
        !ctx.currentCompoPath.empty()
            ? ctx.currentCompoPath
            : (ctx.dataRoot / "S1" / "COMPO01.INT");
    Log::Printf("Scene1::Fn1 load Stage1 COMPO path='%s'",
                stage1CompoPath.u8string().c_str());
    s_stage1LifecycleScene801C7284 =
        PrStage1LifecycleDirect::InitScene801C7284(
            PrStage1LifecycleExecutorDirect::
                BuildStage1InitSceneInputWithMovieSegmentFeedback801C7284(
                    s_stage1LifecycleExecutor801C81EC,
                    ctx.dataRoot,
                    &ctx.stage1XaCdDirect));
    Log::Printf(
        "Scene1::Fn1 direct 801C7284 timing tickPerMin=%d tickPerFrame60=%d baseTick=%d field360=%d loader0=%d loader6=%d",
        s_stage1LifecycleScene801C7284.field348_TicksPerMinute,
        s_stage1LifecycleScene801C7284.field352_TicksPerFrame60,
        s_stage1LifecycleScene801C7284.field356_BaseTick,
        s_stage1LifecycleScene801C7284.field360,
        s_stage1LifecycleScene801C7284.loaders[0].present ? 1 : 0,
        s_stage1LifecycleScene801C7284.loaders[6].present ? 1 : 0);
    (void)PrSfx::InitStage1VabFromInt(stage1CompoPath.u8string());
    ctx.stageRunning = false;
    PrStage1MovieTextDirect::ResetSub80024C84(s_movie1TextRuntime);
    s_movie1TextWindowTickResult =
        PrStage1MovieTextDirect::Movie1TextWindowTickResult{};
    PrStage1MovieTextOuterLoopDirect::ResetMovieTextOuterLoopSub801C455C(
        s_movie1TextOuterLoopRuntime);
    PrStage1OverlayScriptTextDirectResetSub801C8604(
        s_stage1OverlayScriptTextDirect);
    ResetStage1FormalLifecycleRuntime();
    ResetStage1LifecycleActionExecutor801C81EC();
    PrimeStage1LifecycleDaStash801D3040(ctx);
    ResetStage1TerminalFormalLifecycleSnapshot();
    ResetStage1TerminalCleanupDrain801C7A60();
    if (!PrSS0Scene0RuntimeDirect::RuntimeEnabled()) {
        PrSqevs1::Shutdown(ctx);
    }
    if (!ctx.currentComodBytes.empty()) {
        (void)PrStage1MovieTextDirect::LoadFromComodSub80024C84(
            s_movie1TextRuntime,
            ctx.currentComodBytes,
            s_stage1LifecycleGlobals801CA3BC.ptr800943CC);
        PrStage1MovieTextDirect::ApplyMovieSubtitleWindow(
            s_movie1TextRuntime,
            s_movie1Runtime);
    } else {
        PrStage1MovieTextDirect::ResetSub80024C84(s_movie1TextRuntime);
        s_movie1TextWindowTickResult =
            PrStage1MovieTextDirect::Movie1TextWindowTickResult{};
        PrStage1MovieTextOuterLoopDirect::ResetMovieTextOuterLoopSub801C455C(
            s_movie1TextOuterLoopRuntime);
        PrStage1Scene1Movie1Direct::SetSubtitleWindow(
            s_movie1Runtime,
            PrStage1Scene1Movie1Direct::SubtitleFrameWindow{});
    }
    PrStage1HdSubtitles::Preload(ctx);
}
int Fn2(PrGameContext& ctx) {
    return TickStage1Scene1_801C81EC_ActionExecutor(ctx);
}
void Main(PrGameContext& ctx) {
    (void)ctx;
}

static void DrawStage1Movie1DrawAdapter(
    PrGameContext& ctx,
    const PrMovieSubtitles::MovieSubtitleTrack& subtitleTrack) {
    float vx = 0.0f, vy = 0.0f, vs = 1.0f;
    CalcPs1Viewport(ctx.renderer, vx, vy, vs);
    SyncStage1Movie1TransitionCtxWordsFromRunnerSnapshot();
    // MOVIE1 video comes from SS/MOVIE1.STR. Decode its original frame
    // bitstream through the direct MDEC path before composing native templates.
    UpdateStage1DirectMovie1MdecFromStrPlayer(ctx);
    PrStage1Scene1Movie1Direct::SetSubtitleWindow(
        s_movie1Runtime,
        PrStage1MovieTextDirect::GetActiveMovieSubtitleFrameWindow(
            s_movie1TextRuntime));
    (void)PrStage1Scene1Movie1Direct::PrepareMovie1DrawRuntimeSub801C77C0(
        s_movie1Runtime,
        s_movie1TextWindowTickResult);
    const PrStage1LifecycleExecutorDirect::HostBlockSnapshot801C81EC block =
        PrStage1LifecycleExecutorDirect::GetHostBlockSnapshot801C81EC(
            s_stage1LifecycleExecutor801C81EC);
    const bool movie1BlockActive =
        block.kind ==
            PrStage1LifecycleExecutorDirect::HostBlockKind801C81EC::Movie1 &&
        block.active;
    const bool clearTailMovieBlockActive =
        block.kind ==
            PrStage1LifecycleExecutorDirect::HostBlockKind801C81EC::
                ClearTailMovie &&
        block.active &&
        !block.waitingForPendingActions;
    const bool movieStrBlockActive =
        movie1BlockActive || clearTailMovieBlockActive;
    const bool initialMovie1PreTransitionPending =
        PrStage1LifecycleExecutorDirect::
            IsInitialMovie1PreTransition201ACPending801C81EC(
                s_stage1LifecycleExecutor801C81EC);
    const bool clearTailPreMovieTransitionPending =
        (block.kind ==
             PrStage1LifecycleExecutorDirect::HostBlockKind801C81EC::
                 ClearTailMovie &&
         block.active &&
         block.waitingForPendingActions &&
         PrStage1LifecycleExecutorDirect::
             IsClearTailPreMovieTransition201ACPending801C81EC(
                 s_stage1LifecycleExecutor801C81EC)) ||
        PrStage1LifecycleExecutorDirect::
            IsTransition20110Mode2Pending801C81EC(
                s_stage1LifecycleExecutor801C81EC);
    const bool clearTailPreMovieTransitionActive =
        clearTailPreMovieTransitionPending ||
        PrStage1Scene1Movie1Direct::IsTransitionSub80020110Active(
            s_movie1Runtime, 2u, 1u, 2u);
    PrStage1CommonLyricsSnapshot commonLyrics{};
    (void)PrStage1MovieTextDirect::GetCommonLyricsSnapshot(
        s_movie1TextRuntime,
        commonLyrics);
    const PrStage1MovieTextDirect::Movie1TextCommonLyricsHudCarrierSub8001EC54
        commonLyricsCarrier =
            PrStage1MovieTextDirect::BuildCommonLyricsHudCarrierSub8001EC54(
                s_movie1TextWindowTickResult,
                commonLyrics);
    static uint32_t s_clearTailTextTraceCount = 0;
    if (clearTailMovieBlockActive && s_clearTailTextTraceCount < 120u) {
        ++s_clearTailTextTraceCount;
        const PrStage1MovieTextDirect::Movie1TextDisplayActionSub8001EC54&
            display = s_movie1TextWindowTickResult.displayActionSub8001EC54Ctx7;
        const PrStage1MovieTextDirect::Movie1TextFastSpriteSequenceSub8001B954&
            sequence = display.textFastSpriteSequenceSub8001B954;
        const uint32_t strFrame =
            ctx.strPlayer != nullptr ? ctx.strPlayer->GetCurrentFrame() : 0u;
        const uint32_t strTotal =
            ctx.strPlayer != nullptr ? ctx.strPlayer->GetTotalFrames() : 0u;
        const size_t modeIndex =
            s_movie1TextRuntime.playAndWaitModeSub801C77C0;
        const bool commonTrackModeInRange =
            modeIndex < s_movie1TextRuntime.commonLyricsTracks.size();
        const PrMovieSubtitles::MovieSubtitleTrack* commonTrack =
            commonTrackModeInRange
                ? &s_movie1TextRuntime.commonLyricsTracks[modeIndex]
                : nullptr;
        Log::Printf(
            "[ClearTailTextState] frame=%u str=%u/%u playing=%d "
            "playActive=%d mode=%u gateKnown=%d gate=%d display=%d "
            "helper=%u word916DC=%u text=0x%08X textPtr=%d "
            "seqValid=%d strlen=%u glyphSubmit=%u firstGap=%u "
            "commonValid=%d producer=%d commonMode=%u query=%u event=%u "
            "idx=%d hasGeom=%d hasFrame=%d carrierValid=%d "
            "descSelected=%d modeGap=%d desc=0x%08X trackLoaded=%d "
            "trackLines=%u trackEntries=%u vSel=0x%08X vEntries=0x%08X "
            "vCount=%u vCursor=%u vText=0x%08X",
            ctx.frame,
            strFrame,
            strTotal,
            ctx.strPlayer != nullptr && ctx.strPlayer->IsPlaying() ? 1 : 0,
            s_movie1TextRuntime.playAndWaitActiveSub801C77C0 ? 1 : 0,
            static_cast<unsigned>(
                s_movie1TextRuntime.playAndWaitModeSub801C77C0),
            s_movie1TextRuntime.movie1DisplayFlushGate801D3044Known ? 1 : 0,
            s_movie1TextRuntime.movie1DisplayFlushGate801D3044 ? 1 : 0,
            display.requested ? 1 : 0,
            static_cast<unsigned>(display.helper),
            static_cast<unsigned>(display.word800916DC),
            sequence.textPsxAddr,
            sequence.textPtr != nullptr ? 1 : 0,
            sequence.valid ? 1 : 0,
            static_cast<unsigned>(sequence.strlenBytes),
            static_cast<unsigned>(sequence.glyphSubmitResultCount),
            static_cast<unsigned>(sequence.firstGap),
            commonLyrics.valid ? 1 : 0,
            commonLyrics.producerActive ? 1 : 0,
            static_cast<unsigned>(commonLyrics.mode),
            commonLyrics.queryFrame,
            commonLyrics.eventFrame,
            static_cast<int>(commonLyrics.textIndex),
            commonLyricsCarrier.hasTextGeometrySub8001B954 ? 1 : 0,
            commonLyricsCarrier.hasFrameSub8001C864 ? 1 : 0,
            commonLyricsCarrier.valid ? 1 : 0,
            s_movie1TextRuntime.playAndWaitDescSelectedSub80024C84 ? 1 : 0,
            s_movie1TextRuntime.playAndWaitModeGapSub801C77C0 ? 1 : 0,
            s_movie1TextRuntime.playAndWaitDescAddrSub801C77C0,
            commonTrack != nullptr && commonTrack->loaded ? 1 : 0,
            commonTrack != nullptr
                ? static_cast<unsigned>(commonTrack->lines.size())
                : 0u,
            commonTrack != nullptr
                ? static_cast<unsigned>(commonTrack->entryCount)
                : 0u,
            s_movie1TextRuntime.commonLyricsVText.selectedDescAddr,
            s_movie1TextRuntime.commonLyricsVText.selectedEntriesAddr,
            s_movie1TextRuntime.commonLyricsVText.selectedEntryCount,
            s_movie1TextRuntime.commonLyricsVText.eventCursor,
            s_movie1TextRuntime.commonLyricsVText.text268MirrorPsxAddr);
    }
    const bool directTextFrameSub8001C864Active =
        commonLyricsCarrier.valid && commonLyricsCarrier.hasFrameSub8001C864;
    PrStage1Scene1Movie1Direct::Movie1DrawPlan plan{};
    const auto submitVideoMatte = [&ctx](
        const PrStage1Scene1Movie1Direct::Movie1DrawPlan& drawPlan) {
        if (!ctx.renderer ||
            drawPlan.video.w <= 0.0f ||
            drawPlan.video.h <= 0.0f) {
            return;
        }
        D3D11Renderer::SolidRectCmd rect{};
        rect.x = drawPlan.video.x;
        rect.y = drawPlan.video.y;
        rect.w = drawPlan.video.w;
        rect.h = drawPlan.video.h;
        rect.r = 0.0f;
        rect.g = 0.0f;
        rect.b = 0.0f;
        rect.a = 1.0f;
        rect.layer = PrStage1Scene1Movie1Direct::kMovie1MdecBackgroundLayer;
        rect.order = 1;
        ctx.renderer->SubmitSolidRect(rect);
    };
    PrStage1Scene1Movie1Direct::Movie1HostFeedback movie1HostFeedback =
        LifecycleHost801C81EC::BuildMovie1HostFeedback(
            ctx,
            movieStrBlockActive && block.pathResolved,
            movieStrBlockActive ? block.path : std::filesystem::path{},
            false);
    movie1HostFeedback.directMdecTextureReady =
        s_stage1DirectMovie1Mdec.ready;
    movie1HostFeedback.directMdecTexture =
        s_stage1DirectMovie1Mdec.texture;
    if (PrStage1Scene1Movie1Direct::BuildRuntimeDrawPlan(
            s_movie1Runtime,
            movie1HostFeedback,
            subtitleTrack,
            vx,
            vy,
            vs,
            plan,
            directTextFrameSub8001C864Active)) {
        if (initialMovie1PreTransitionPending) {
            plan.drawVideo = false;
        }
        if (clearTailPreMovieTransitionActive) {
            PrStage1Scene1DrawBackend::DrawGameplaySubmitFrozenRuntimeBaseOnly(ctx);
            if (ctx.renderer) {
                ctx.renderer->FlushSprites();
            }
            plan.drawVideo = false;
        }
        if (PrStage1Scene1Movie1Direct::ShouldSubmitEmptyVideoMatte(plan)) {
            submitVideoMatte(plan);
        }
        PrStage1Scene1DrawBackend::ExecuteMovie1DrawPlan(ctx, plan);
        PrStage1Scene1DrawBackend::DrawMovie1FastSpriteRuntimeAndRouteHud(
            ctx,
            s_movie1Runtime.rawDrawFastSpriteRuntimeOwnerValid
                ? &s_movie1Runtime.rawDrawFastSpriteRuntime
                : nullptr,
            false,
            movie1BlockActive || clearTailMovieBlockActive);
        return;
    }
    if (clearTailPreMovieTransitionActive) {
        PrStage1Scene1DrawBackend::DrawGameplaySubmitFrozenRuntimeBaseOnly(ctx);
        if (ctx.renderer) {
            ctx.renderer->FlushSprites();
        }
    }
}

void Render(PrGameContext& ctx) {
    PrStage1Scene1RenderRouterDirect::Scene1RenderRouterCarrier carrier{};
    auto& loading = s_stage1LifecycleExecutor801C81EC.bootstrap15590Loading;
    if (s_stage1LifecycleExecutor801C81EC.bootstrap15590Active &&
        loading.pattern.active) {
        if (PrStage1Scene1DrawBackend::DrawLoadingPattern8001EF40(ctx, loading.frame)) {
            PrStage1LoadingDirect::RecordPresentation(loading, ctx.frame);
        }
        return;
    }
    carrier.rendererReady = ctx.renderer != nullptr;
    carrier.clearResultMovieDrawActive = IsStage1ClearResultMovieVisualActive();
    carrier.clearResultMoviePlayerReady = ctx.strPlayer != nullptr;
    const PrStage1Scene1Movie1Direct::Movie1DrawableStateQueryResult
        movie1DrawableState =
            PrStage1Scene1Movie1Direct::QueryDrawableState(s_movie1Runtime);
    const bool initialMovie1PreTransitionPending =
        PrStage1LifecycleExecutorDirect::
            IsInitialMovie1PreTransition201ACPending801C81EC(
                s_stage1LifecycleExecutor801C81EC);
    carrier.movie1DrawableActive =
        movie1DrawableState.drawableActive &&
        !initialMovie1PreTransitionPending;
    carrier.stageRunning = ctx.stageRunning;
    carrier.introTransitionDrawActive = s_stage1IntroTransitionRenderActive;
    PrStage1Scene1RenderRouterDirect::Scene1RenderRouterInput routeInput{};
    routeInput.rendererReady = carrier.rendererReady;
    routeInput.clearResultMovieDrawActive =
        carrier.clearResultMovieDrawActive;
    routeInput.clearResultMoviePlayerReady =
        carrier.clearResultMoviePlayerReady;
    routeInput.movie1DrawableActive = carrier.movie1DrawableActive;
    routeInput.stageRunning = carrier.stageRunning;
    routeInput.introTransitionDrawActive =
        carrier.introTransitionDrawActive;

    for (;;) {
        const PrStage1Scene1RenderRouterDirect::Scene1RenderRoute route =
            PrStage1Scene1RenderRouterDirect::ResolveScene1RenderRoute(
                routeInput);
        if (route ==
                PrStage1Scene1RenderRouterDirect::Scene1RenderRoute::
                    IntroTransitionDraw ||
            route ==
                PrStage1Scene1RenderRouterDirect::Scene1RenderRoute::
                    GameplaySubmitDraw) {
            PrStage1Scene1Movie1Direct::ClearCurrentMovieFrame30(
                s_movie1Runtime);
        }

        switch (route) {
        case PrStage1Scene1RenderRouterDirect::Scene1RenderRoute::None:
            return;

        case PrStage1Scene1RenderRouterDirect::Scene1RenderRoute::ClearResultMovieDraw:
            DrawStage1Movie1DrawAdapter(
                ctx,
                PrStage1MovieTextDirect::GetActiveMovieSubtitleTrack(
                    s_movie1TextRuntime));
            return;

        case PrStage1Scene1RenderRouterDirect::Scene1RenderRoute::Movie1Draw: {
            DrawStage1Movie1DrawAdapter(
                ctx,
                PrStage1MovieTextDirect::GetActiveMovieSubtitleTrack(
                    s_movie1TextRuntime));
            return;
        }

        case PrStage1Scene1RenderRouterDirect::Scene1RenderRoute::IntroTransitionDraw:
            PrStage1Scene1DrawBackend::DrawIntroTransition(
                ctx,
                s_stage1IntroTransitionFrame < 0
                    ? 0
                    : s_stage1IntroTransitionFrame);
            return;

        case PrStage1Scene1RenderRouterDirect::Scene1RenderRoute::GameplaySubmitDraw:
            if (s_stage1TerminalCleanupDrainActive801C7A60) {
                PrStage1Scene1DrawBackend::DrawTerminalPresentation801C7A60(ctx);
            } else {
                PrStage1Scene1DrawBackend::DrawGameplaySubmitAndHud(ctx);
            }
            return;
        }
    }
}

bool GetStage1OverlayScriptTextRuntimeSnapshot(PrGameContext& ctx, PrStage1OverlayScriptTextSnapshot& out) {
    (void)ctx;
    return PrStage1OverlayScriptTextDirectResolveSnapshot(
        s_stage1OverlayScriptTextDirect,
        out);
}

static bool IsStage1HdCommonLyricsMode(uint8_t mode) {
    return mode > 0u &&
           mode < PrStage1MovieTextDirect::kMovie1CommonLyricsModeCount;
}

static bool TryResolveStage1ClearTailHdCommonLyricsMode(uint8_t& outMode) {
    if (s_movie1TextRuntime.playAndWaitActiveSub801C77C0 &&
        IsStage1HdCommonLyricsMode(
            s_movie1TextRuntime.playAndWaitModeSub801C77C0)) {
        outMode = s_movie1TextRuntime.playAndWaitModeSub801C77C0;
        return true;
    }

    using OuterPhase =
        PrStage1MovieTextOuterLoopDirect::MovieTextOuterLoopPhaseSub801C455C;
    if (s_movie1TextOuterLoopRuntime.phase != OuterPhase::Idle &&
        s_movie1TextOuterLoopRuntime.phase != OuterPhase::Complete &&
        IsStage1HdCommonLyricsMode(s_movie1TextOuterLoopRuntime.mode)) {
        outMode = s_movie1TextOuterLoopRuntime.mode;
        return true;
    }

    const PrStage1LifecycleExecutorDirect::HostBlockSnapshot801C81EC block =
        PrStage1LifecycleExecutorDirect::GetHostBlockSnapshot801C81EC(
            s_stage1LifecycleExecutor801C81EC);
    if (block.kind !=
            PrStage1LifecycleExecutorDirect::HostBlockKind801C81EC::
                ClearTailMovie ||
        !block.active) {
        return false;
    }

    outMode =
        s_stage1NumericRuntime.rightRankState.rightRankActiveRow != 0u ? 2u
                                                                       : 1u;
    return true;
}

static bool TryBuildStage1HdTrackSource(
    const PrMovieSubtitles::MovieSubtitleTrack& track,
    const std::vector<PrStage1VTextDirectLineMeta>* lineMetas,
    PrStage1HdSubtitleSourceKind kind,
    uint8_t mode,
    uint32_t queryFrame30,
    PrStage1HdSubtitleRuntimeSource& out) {
    if (!track.loaded || track.lines.empty()) {
        return false;
    }

    constexpr uint8_t kMatchLanguageIndex = 0u;
    for (size_t i = 0; i < track.lines.size(); ++i) {
        const PrMovieSubtitles::MovieSubtitleLine& line = track.lines[i];
        const uint32_t endFrame =
            line.frame30 + static_cast<uint32_t>(line.duration);
        if (queryFrame30 < line.frame30 || queryFrame30 >= endFrame) {
            continue;
        }

        out.valid = true;
        out.kind = kind;
        out.mode = mode;
        out.queryFrame = queryFrame30;
        out.eventFrame = line.frame30;
        out.durationFrames = line.duration;
        out.textIndex = line.textIndex[kMatchLanguageIndex];
        if (lineMetas != nullptr && i < lineMetas->size()) {
            out.psxAddr = (*lineMetas)[i].textAddrs[kMatchLanguageIndex];
        }
        out.originalText = line.texts[kMatchLanguageIndex].empty()
                               ? nullptr
                               : line.texts[kMatchLanguageIndex].c_str();
        return true;
    }

    return false;
}

bool GetStage1HdSubtitleRuntimeSource(PrGameContext& ctx,
                                      PrStage1HdSubtitleRuntimeSource& out) {
    out = PrStage1HdSubtitleRuntimeSource{};
    if (ctx.currentScene != PrSceneId::Scene1 || ctx.subtitleFlag == 0) {
        return false;
    }

    PrStage1CommonLyricsSnapshot commonLyrics{};
    (void)PrStage1MovieTextDirect::GetCommonLyricsSnapshot(
        s_movie1TextRuntime,
        commonLyrics);
    const PrStage1MovieTextDirect::Movie1TextCommonLyricsHudCarrierSub8001EC54
        commonLyricsCarrier =
            PrStage1MovieTextDirect::BuildCommonLyricsHudCarrierSub8001EC54(
                s_movie1TextWindowTickResult,
                commonLyrics);
    const PrStage1CommonLyricsSnapshot commonLyricsForHud =
        commonLyricsCarrier.commonLyrics;
    const bool commonLyricsTextActive =
        commonLyricsForHud.valid &&
        commonLyricsForHud.text != nullptr &&
        commonLyricsForHud.text[0] != '\0';
    uint8_t clearTailCommonLyricsMode = 0u;
    const bool clearTailCommonLyricsWindow =
        ctx.strPlayer && ctx.strPlayer->IsPlaying() &&
        TryResolveStage1ClearTailHdCommonLyricsMode(clearTailCommonLyricsMode);
    if (clearTailCommonLyricsWindow) {
        uint32_t commonLyricsFrame30 = s_movie1Runtime.currentMovieFrame30;
        commonLyricsFrame30 =
            LifecycleHost801C81EC::ResolveMovie1HostFrame30(ctx);
        if (commonLyricsFrame30 == 0u && commonLyricsForHud.queryFrame != 0u) {
            commonLyricsFrame30 = commonLyricsForHud.queryFrame;
        }
        const size_t modeIndex = static_cast<size_t>(clearTailCommonLyricsMode);
        if (modeIndex < s_movie1TextRuntime.commonLyricsTracks.size() &&
            TryBuildStage1HdTrackSource(
                s_movie1TextRuntime.commonLyricsTracks[modeIndex],
                &s_movie1TextRuntime.commonLyricsLineMetas[modeIndex],
                PrStage1HdSubtitleSourceKind::CommonLyrics,
                clearTailCommonLyricsMode,
                commonLyricsFrame30,
                out)) {
            return true;
        }
    }
    if (clearTailCommonLyricsWindow && commonLyricsTextActive) {
        out.valid = true;
        out.kind = PrStage1HdSubtitleSourceKind::CommonLyrics;
        out.mode = commonLyricsForHud.mode;
        out.queryFrame = commonLyricsForHud.queryFrame;
        out.eventFrame = commonLyricsForHud.eventFrame;
        out.durationFrames = commonLyricsForHud.durationFrames;
        out.textIndex = commonLyricsForHud.textIndex;
        out.originalText = commonLyricsForHud.text;
        return true;
    }

    const PrMovieSubtitles::MovieSubtitleTrack& movieTrack =
        PrStage1MovieTextDirect::GetMovieSubtitleTrack(s_movie1TextRuntime);
    uint32_t movieFrame30 = s_movie1Runtime.currentMovieFrame30;
    if (movieFrame30 == 0u && movieTrack.loaded) {
        const PrStage1LifecycleExecutorDirect::HostBlockSnapshot801C81EC
            block =
                PrStage1LifecycleExecutorDirect::GetHostBlockSnapshot801C81EC(
                    s_stage1LifecycleExecutor801C81EC);
        const bool movieBlockActive =
            block.active &&
            !block.waitingForPendingActions &&
            (block.kind ==
                 PrStage1LifecycleExecutorDirect::HostBlockKind801C81EC::
                     Movie1 ||
             block.kind ==
                 PrStage1LifecycleExecutorDirect::HostBlockKind801C81EC::
                     ClearTailMovie);
        if (movieBlockActive && ctx.strPlayer && ctx.strPlayer->IsPlaying()) {
            movieFrame30 = LifecycleHost801C81EC::ResolveMovie1HostFrame30(ctx);
        }
    }
    if (!clearTailCommonLyricsWindow &&
        movieFrame30 != 0u &&
        movieTrack.loaded) {
        if (TryBuildStage1HdTrackSource(
                movieTrack,
                &s_movie1TextRuntime.subtitleLineMeta,
                PrStage1HdSubtitleSourceKind::Movie1,
                0u,
                movieFrame30,
                out)) {
            return true;
        }
    }

    if (commonLyricsTextActive) {
        out.valid = true;
        out.kind = PrStage1HdSubtitleSourceKind::CommonLyrics;
        out.mode = commonLyricsForHud.mode;
        out.queryFrame = commonLyricsForHud.queryFrame;
        out.eventFrame = commonLyricsForHud.eventFrame;
        out.durationFrames = commonLyricsForHud.durationFrames;
        out.textIndex = commonLyricsForHud.textIndex;
        out.originalText = commonLyricsForHud.text;
        return true;
    }

    PrStage1OverlayScriptTextSnapshot overlayScript{};
    if (PrStage1OverlayScriptTextDirectResolveSnapshot(
            s_stage1OverlayScriptTextDirect,
            overlayScript) &&
        overlayScript.directScriptBoxPermit4E &&
        overlayScript.activeTextMirrorPtr != nullptr &&
        overlayScript.activeTextMirrorPtr[0] != '\0' &&
        overlayScript.activeTimeoutFramesRemaining != 0u) {
        out.valid = true;
        out.kind = PrStage1HdSubtitleSourceKind::OverlayScriptText;
        out.mode = 0u;
        out.queryFrame = ResolveStage1RunnerQueryFrame30(GetStageRunner());
        out.durationFrames = overlayScript.activeTimeoutFramesRemaining;
        out.textId = overlayScript.activeTextId;
        out.originalText = overlayScript.activeTextMirrorPtr;
        return true;
    }

    return false;
}

bool GetStage1RuntimeSlotsSnapshot(PrGameContext& ctx,
                                   PrStage1RuntimeSlotsSnapshot& out,
                                   bool allowTransitionFreeze,
                                   bool allowStoppedRuntimeFreeze) {
    return ResolveStage1RuntimeSlotsSnapshot(ctx,
                                             out,
                                             allowTransitionFreeze,
                                             allowStoppedRuntimeFreeze);
}

bool CopyStage1HostBlockSnapshot801C81EC(
    PrStage1LifecycleExecutorDirect::HostBlockSnapshot801C81EC& out) {
    out = PrStage1LifecycleExecutorDirect::GetHostBlockSnapshot801C81EC(
        s_stage1LifecycleExecutor801C81EC);
    return true;
}

void SetStage1XaCurrentPhysicalGetlocPProbeDisabled(bool disabled) {
    s_stage1XaCurrentPhysicalGetlocPProbeDisabled = disabled;
}

bool IsStage1XaCurrentPhysicalGetlocPProbeDisabled() {
    return s_stage1XaCurrentPhysicalGetlocPProbeDisabled;
}

}

// ========== Scene2: Stage2 ==========
namespace PrScn2 {
static std::unique_ptr<PrStage2ProductRuntime::Runtime> s_runtime;

int Fn0(PrGameContext& ctx) {
    Log::Printf("Scene2::Fn0 start retained native S2 session");
    // Stage1's post-clear Save UI can leave its host XA player and the
    // process-wide menu BGM alive until the next resident scene boundary.
    // S2 owns a separate XA/SPU path, so close those Stage1 owners before
    // constructing the retained native S2 session.  Do this at the explicit
    // scene handoff rather than relying on a save-page callback to stop it.
    if (ctx.xa1Player) {
        const bool wasPlaying = ctx.xa1Player->IsPlaying();
        ctx.xa1Player->Stop();
        Log::Printf("Scene2::Fn0 stop retained Stage1 XA playing=%d",
                    wasPlaying ? 1 : 0);
    }
    const bool bgmPlaying = PrSfx::IsBgmPlaying();
    PrSfx::StopBgm();
    Log::Printf("Scene2::Fn0 stop retained Stage1 BGM playing=%d",
                bgmPlaying ? 1 : 0);
    s_runtime.reset();
    if (!ctx.renderer) throw std::runtime_error("Scene2 requires a D3D renderer");
    s_runtime = std::make_unique<PrStage2ProductRuntime::Runtime>(
        ctx.dataRoot, *ctx.renderer, ctx, [&ctx] {
            const uint32_t value = static_cast<uint32_t>(ctx.debugPadInput);
            ctx.debugPadInput = 0;
            return value;
        });
    return 2;
}

void Fn1(PrGameContext& ctx) {
    (void)ctx;
    Log::Printf("Scene2::Fn1 retained native S2 owner ready");
}

int Fn2(PrGameContext& ctx) {
    if (!s_runtime) {
        if (!ctx.renderer) throw std::runtime_error("Scene2 runtime was not initialized");
        s_runtime = std::make_unique<PrStage2ProductRuntime::Runtime>(
            ctx.dataRoot, *ctx.renderer, ctx, [&ctx] {
                const uint32_t value = static_cast<uint32_t>(ctx.debugPadInput);
                ctx.debugPadInput = 0;
                return value;
            });
    }
    const int result = s_runtime->Tick();
    ctx.stageRunning = s_runtime->Running();
    if (result != 2) {
        ctx.stageRunning = false;
        ctx.sceneExitReason = s_runtime->ExitReason();
    }
    return result;
}

void Pump(PrGameContext& ctx) {
    (void)ctx;
    if (s_runtime) s_runtime->Pump();
}

bool OwnsNativePresentation(const PrGameContext& ctx) {
    (void)ctx;
    return s_runtime && s_runtime->OwnsPresentation();
}

bool BeginResidentDirectory(PrGameContext& ctx, int previousScene) {
    return s_runtime && s_runtime->BeginResidentDirectory(ctx, previousScene);
}

void Main(PrGameContext& ctx) {
    (void)ctx;
}

void Render(PrGameContext& ctx) {
    if (OwnsNativePresentation(ctx)) {
        // The retained S2 GPU owner presents its selected framebuffer page
        // from Poll(); the outer Windows renderer must not clear it again.
        return;
    }
    if (ctx.stageRunning) {
        GetStageRunner().Render(ctx);
        return;
    }
    if (RenderNativeStrFrame(ctx)) {
        return;
    }
    if (PrTmdRenderer::RenderStage(ctx, 0.0f, 0.0f, 1.0f)) {
        return;
    }
    RenderNativeSceneFallback(ctx, 2);
}
}

// ========== Scene3: Stage3 ==========
namespace PrScn3 {
static bool s_strPlayed = false;
static bool s_strStarted = false;
static bool s_xaStarted = false;

int Fn0(PrGameContext& ctx) {
    Log::Printf("Scene3::Fn0 init global pointers");
    s_strPlayed = false;
    s_strStarted = false;
    s_xaStarted = false;
    return 3;
}
void Fn1(PrGameContext& ctx) {
    Log::Printf("Scene3::Fn1 load COMPO03.INT");
}
int Fn2(PrGameContext& ctx) {
    if (!s_strPlayed) {
        const std::filesystem::path dataRoot = ctx.dataRoot;
        if (ctx.strPlayer) {
            if (!s_strStarted) {
                ctx.strPlayer->Stop();
                const std::filesystem::path strPath = dataRoot / "SS" / "MOVIE3.STR";
                if (std::filesystem::exists(strPath) && ctx.strPlayer->Play(strPath)) {
                    s_strStarted = true;
                } else {
                    s_strPlayed = true;
                    s_strStarted = false;
                }
            }

            if (s_strStarted) {
                const StrPlayerResult r = ctx.strPlayer->Update(ctx.debugF1_StrSkip);
                if (r == StrPlayerResult::Playing) {
                    return 3;
                }
                ctx.strPlayer->Stop();
                s_strPlayed = true;
                s_strStarted = false;
            }
        } else {
            s_strPlayed = true;
        }
    }

    if (!s_xaStarted) {
        if (ctx.xa1Player && !ctx.currentXaPath.empty()) {
            const bool ok = ctx.xa1Player->Play(ctx.currentXaPath);
            Log::Printf("Scene3::Fn2 XA1 start ok=%d path='%s'", ok ? 1 : 0, ctx.currentXaPath.u8string().c_str());
            ctx.xa1Player->Update();
            s_xaStarted = ok;
        }
    }

    if (ctx.xa1Player && ctx.xa1Player->IsPlaying()) {
        ctx.xa1Player->Update();
    }
    ctx.stageRunning = true;
    int exitCheck = CheckLegacyStageLoopExitConditions(ctx);
    if (exitCheck == 999) {
        return 3;
    }
    if (exitCheck != 1000) {
        if (ctx.xa1Player) {
            ctx.xa1Player->Stop();
        }
        ctx.stageRunning = false;
        s_strPlayed = false;
        s_strStarted = false;
        s_xaStarted = false;
        return exitCheck;
    }
    if (PrStageRunner_Run(ctx, 3) == 1) {
        const StageRunnerState st = GetStageRunner().GetState();
        if (ctx.xa1Player) {
            ctx.xa1Player->Stop();
        }
        if (st == StageRunnerState::Cleared) {
            ctx.sceneExitReason = 3;
            ctx.stageRunning = false;
            s_strPlayed = false;
            s_strStarted = false;
            s_xaStarted = false;
            return 4; // Scene5 (no Scene4)
        }
        if (st == StageRunnerState::Failed) {
            ctx.stageRunning = false;
            s_strPlayed = false;
            s_strStarted = false;
            s_xaStarted = false;
            return 0;
        }
    }
    return 3;
}
void Main(PrGameContext& ctx) {
    (void)ctx;
}
void Render(PrGameContext& ctx) {
    if (ctx.stageRunning) {
        GetStageRunner().Render(ctx);
        return;
    }
    if (RenderNativeStrFrame(ctx)) {
        return;
    }
    if (PrTmdRenderer::RenderStage(ctx, 0.0f, 0.0f, 1.0f)) {
        return;
    }
    RenderNativeSceneFallback(ctx, 3);
}
}

// ========== Scene5 (idx4): Stage5 ==========
namespace PrScn5 {
static bool s_strPlayed = false;
static bool s_strStarted = false;
static bool s_xaStarted = false;

int Fn0(PrGameContext& ctx) {
    Log::Printf("Scene5::Fn0 init global pointers");
    (void)ctx;
    s_strPlayed = false;
    s_strStarted = false;
    s_xaStarted = false;
    return 4;
}

void Fn1(PrGameContext& ctx) {
    (void)ctx;
    Log::Printf("Scene5::Fn1 load COMPO05.INT");
}

int Fn2(PrGameContext& ctx) {
    if (!s_strPlayed) {
        const std::filesystem::path dataRoot = ctx.dataRoot;
        if (ctx.strPlayer) {
            if (!s_strStarted) {
                ctx.strPlayer->Stop();
                const std::filesystem::path strPath = dataRoot / "SS" / "MOVIE5.STR";
                if (std::filesystem::exists(strPath) && ctx.strPlayer->Play(strPath)) {
                    s_strStarted = true;
                } else {
                    s_strPlayed = true;
                    s_strStarted = false;
                }
            }

            if (s_strStarted) {
                const StrPlayerResult r = ctx.strPlayer->Update(ctx.debugF1_StrSkip);
                if (r == StrPlayerResult::Playing) {
                    return 4;
                }
                ctx.strPlayer->Stop();
                s_strPlayed = true;
                s_strStarted = false;
            }
        } else {
            s_strPlayed = true;
        }
    }

    if (!s_xaStarted) {
        if (ctx.xa1Player && !ctx.currentXaPath.empty()) {
            const bool ok = ctx.xa1Player->Play(ctx.currentXaPath);
            Log::Printf("Scene5::Fn2 XA1 start ok=%d path='%s'", ok ? 1 : 0, ctx.currentXaPath.u8string().c_str());
            ctx.xa1Player->Update();
            s_xaStarted = ok;
        }
    }

    if (ctx.xa1Player && ctx.xa1Player->IsPlaying()) {
        ctx.xa1Player->Update();
    }

    ctx.stageRunning = true;
    int exitCheck = CheckLegacyStageLoopExitConditions(ctx);
    if (exitCheck == 999) {
        return 4;
    }
    if (exitCheck != 1000) {
        if (ctx.xa1Player) {
            ctx.xa1Player->Stop();
        }
        ctx.stageRunning = false;
        s_strPlayed = false;
        s_strStarted = false;
        s_xaStarted = false;
        return exitCheck;
    }

    const int runnerResult = PrStageRunner_Run(ctx, 5);
    if (runnerResult == 1) {
        const StageRunnerState st = GetStageRunner().GetState();
        if (ctx.xa1Player) {
            ctx.xa1Player->Stop();
        }
        if (st == StageRunnerState::Cleared) {
            ctx.sceneExitReason = 3;
            ctx.stageRunning = false;
            s_strPlayed = false;
            s_strStarted = false;
            s_xaStarted = false;
            return 5;
        }
        if (st == StageRunnerState::Failed) {
            ctx.stageRunning = false;
            s_strPlayed = false;
            s_strStarted = false;
            s_xaStarted = false;
            return 0;
        }
    }

    return 4;
}

void Main(PrGameContext& ctx) {
    (void)ctx;
}

void Render(PrGameContext& ctx) {
    if (ctx.stageRunning) {
        GetStageRunner().Render(ctx);
        return;
    }
    if (RenderNativeStrFrame(ctx)) {
        return;
    }
    if (PrTmdRenderer::RenderStage(ctx, 0.0f, 0.0f, 1.0f)) {
        return;
    }
    RenderNativeSceneFallback(ctx, 4);
}
}

// ========== Scene6 (idx5): Stage6 ==========
namespace PrScn6 {
static bool s_strPlayed = false;
static bool s_strStarted = false;
static bool s_xaStarted = false;

int Fn0(PrGameContext& ctx) {
    Log::Printf("Scene6::Fn0 init global pointers");
    (void)ctx;
    s_strPlayed = false;
    s_strStarted = false;
    s_xaStarted = false;
    return 5;
}

void Fn1(PrGameContext& ctx) {
    (void)ctx;
    Log::Printf("Scene6::Fn1 load COMPO06.INT");
}

int Fn2(PrGameContext& ctx) {
    if (!s_strPlayed) {
        const std::filesystem::path dataRoot = ctx.dataRoot;
        if (ctx.strPlayer) {
            if (!s_strStarted) {
                ctx.strPlayer->Stop();
                const std::filesystem::path strPath = dataRoot / "SS" / "MOVIE6.STR";
                if (std::filesystem::exists(strPath) && ctx.strPlayer->Play(strPath)) {
                    s_strStarted = true;
                } else {
                    s_strPlayed = true;
                    s_strStarted = false;
                }
            }

            if (s_strStarted) {
                const StrPlayerResult r = ctx.strPlayer->Update(ctx.debugF1_StrSkip);
                if (r == StrPlayerResult::Playing) {
                    return 5;
                }
                ctx.strPlayer->Stop();
                s_strPlayed = true;
                s_strStarted = false;
            }
        } else {
            s_strPlayed = true;
        }
    }

    if (!s_xaStarted) {
        if (ctx.xa1Player && !ctx.currentXaPath.empty()) {
            const bool ok = ctx.xa1Player->Play(ctx.currentXaPath);
            Log::Printf("Scene6::Fn2 XA1 start ok=%d path='%s'", ok ? 1 : 0, ctx.currentXaPath.u8string().c_str());
            ctx.xa1Player->Update();
            s_xaStarted = ok;
        }
    }

    if (ctx.xa1Player && ctx.xa1Player->IsPlaying()) {
        ctx.xa1Player->Update();
    }

    ctx.stageRunning = true;
    int exitCheck = CheckLegacyStageLoopExitConditions(ctx);
    if (exitCheck == 999) {
        return 5;
    }
    if (exitCheck != 1000) {
        if (ctx.xa1Player) {
            ctx.xa1Player->Stop();
        }
        ctx.stageRunning = false;
        s_strPlayed = false;
        s_strStarted = false;
        s_xaStarted = false;
        return exitCheck;
    }

    const int runnerResult = PrStageRunner_Run(ctx, 6);
    if (runnerResult == 1) {
        const StageRunnerState st = GetStageRunner().GetState();
        if (ctx.xa1Player) {
            ctx.xa1Player->Stop();
        }
        if (st == StageRunnerState::Cleared) {
            ctx.sceneExitReason = 3;
            ctx.stageRunning = false;
            s_strPlayed = false;
            s_strStarted = false;
            s_xaStarted = false;
            return 6;
        }
        if (st == StageRunnerState::Failed) {
            ctx.stageRunning = false;
            s_strPlayed = false;
            s_strStarted = false;
            s_xaStarted = false;
            return 0;
        }
    }

    return 5;
}

void Main(PrGameContext& ctx) {
    (void)ctx;
}

void Render(PrGameContext& ctx) {
    if (ctx.stageRunning) {
        GetStageRunner().Render(ctx);
        return;
    }
    if (RenderNativeStrFrame(ctx)) {
        return;
    }
    if (PrTmdRenderer::RenderStage(ctx, 0.0f, 0.0f, 1.0f)) {
        return;
    }
    RenderNativeSceneFallback(ctx, 5);
}
}

// ========== Scene7 (idx6): Stage7 ==========
namespace PrScn7 {
static bool s_strPlayed = false;
static bool s_strStarted = false;
static bool s_xaStarted = false;

int Fn0(PrGameContext& ctx) {
    Log::Printf("Scene7::Fn0 init global pointers");
    s_strPlayed = false;
    s_strStarted = false;
    s_xaStarted = false;
    return 6;
}
void Fn1(PrGameContext& ctx) {
    Log::Printf("Scene7::Fn1 load COMPO07.INT");
}
int Fn2(PrGameContext& ctx) {
    if (!s_strPlayed) {
        const std::filesystem::path dataRoot = ctx.dataRoot;
        if (ctx.strPlayer) {
            if (!s_strStarted) {
                ctx.strPlayer->Stop();
                const std::filesystem::path strPath = dataRoot / "SS" / "MOVIE7.STR";
                if (std::filesystem::exists(strPath) && ctx.strPlayer->Play(strPath)) {
                    s_strStarted = true;
                } else {
                    s_strPlayed = true;
                    s_strStarted = false;
                }
            }

            if (s_strStarted) {
                const StrPlayerResult r = ctx.strPlayer->Update(ctx.debugF1_StrSkip);
                if (r == StrPlayerResult::Playing) {
                    return 6;
                }
                ctx.strPlayer->Stop();
                s_strPlayed = true;
                s_strStarted = false;
            }
        } else {
            s_strPlayed = true;
        }
    }

    if (!s_xaStarted) {
        if (ctx.xa1Player && !ctx.currentXaPath.empty()) {
            const bool ok = ctx.xa1Player->Play(ctx.currentXaPath);
            Log::Printf("Scene7::Fn2 XA1 start ok=%d path='%s'", ok ? 1 : 0, ctx.currentXaPath.u8string().c_str());
            ctx.xa1Player->Update();
            s_xaStarted = ok;
        }
    }

    if (ctx.xa1Player && ctx.xa1Player->IsPlaying()) {
        ctx.xa1Player->Update();
    }
    ctx.stageRunning = true;
    int exitCheck = CheckLegacyStageLoopExitConditions(ctx);
    if (exitCheck == 999) {
        return 6;
    }
    if (exitCheck != 1000) {
        if (ctx.xa1Player) {
            ctx.xa1Player->Stop();
        }
        ctx.stageRunning = false;
        s_strPlayed = false;
        s_strStarted = false;
        s_xaStarted = false;
        return exitCheck;
    }
    if (PrStageRunner_Run(ctx, 7) == 1) {
        const StageRunnerState st = GetStageRunner().GetState();
        if (ctx.xa1Player) {
            ctx.xa1Player->Stop();
        }
        if (st == StageRunnerState::Cleared) {
            ctx.sceneExitReason = 3;
            ctx.stageRunning = false;
            s_strPlayed = false;
            s_strStarted = false;
            s_xaStarted = false;
            return 7; // Credits
        }
        if (st == StageRunnerState::Failed) {
            ctx.stageRunning = false;
            s_strPlayed = false;
            s_strStarted = false;
            s_xaStarted = false;
            return 0;
        }
    }
    return 6;
}
void Main(PrGameContext& ctx) {
    (void)ctx;
}
void Render(PrGameContext& ctx) {
    if (ctx.stageRunning) {
        GetStageRunner().Render(ctx);
        return;
    }
    if (RenderNativeStrFrame(ctx)) {
        return;
    }
    if (PrTmdRenderer::RenderStage(ctx, 0.0f, 0.0f, 1.0f)) {
        return;
    }
    RenderNativeSceneFallback(ctx, 6);
}
}

// ========== Scene8 (idx7): Credits/Ending ==========
namespace PrScn8 {
static bool s_resultMoviePlayed = false;
static bool s_resultMovieStarted = false;
static bool s_postEndingTailApplied = false;
static bool s_postEndingSaveUiStartAttempted = false;
static bool s_postEndingSaveUiActive = false;
static bool s_postEndingSaveUiDone = false;
static bool s_postEndingSaveUiBlocked = false;
static int32_t s_postEndingSaveUiResult = 0;
static PrStage1SaveUi19148LowerFeedbackRequestList
    s_postEndingSaveUiLowerFeedbackRequests{};

static bool SeedWriterIsSaveStatus1635CChain(
    const PrStage1SaveStatusPrefix80092F10& seed) {
    return seed.wrote8001635C &&
           (seed.seedAuthorityFunction == PrStagePayloadBankDirect::kFn8001635C ||
            (seed.seedAuthorityFunction == PrStagePayloadBankDirect::kFn8001628C &&
             seed.wrote8001628C));
}

static bool TryStartPostEndingSaveUi19148(PrGameContext& ctx) {
    if (s_postEndingSaveUiStartAttempted) {
        return s_postEndingSaveUiActive;
    }
    s_postEndingSaveUiStartAttempted = true;

    const PrStage1SaveStatusPrefix80092F10 seed80092F10 =
        PrStage1SaveUiDirect::GetSaveStatusPrefix80092F10();
    const bool seedShapeOk =
        seed80092F10.known &&
        !seed80092F10.helperGap &&
        seed80092F10.statusBankKnown80092F1D &&
        seed80092F10.psxAddress ==
            PrStage1SaveStatusPrefix80092F10::kPsxAddress &&
        seed80092F10.byteCount ==
            PrStage1SaveStatusPrefix80092F10::kByteCount;
    const bool seedWriterOk = SeedWriterIsSaveStatus1635CChain(seed80092F10);
    Log::Printf(
        "Scene8::Fn2 post-ending SaveUi19148 start gate: seedKnown=%d helperGap=%d statusKnown=%d addr=%08X bytes=%u writer=%08X seedAuthority=%08X wrote1635C=%d wrote1628C=%d",
        seed80092F10.known ? 1 : 0,
        seed80092F10.helperGap ? 1 : 0,
        seed80092F10.statusBankKnown80092F1D ? 1 : 0,
        seed80092F10.psxAddress,
        seed80092F10.byteCount,
        seed80092F10.lastWriterFunction,
        seed80092F10.seedAuthorityFunction,
        seed80092F10.wrote8001635C ? 1 : 0,
        seed80092F10.wrote8001628C ? 1 : 0);
    if (!seedShapeOk || !seedWriterOk) {
        s_postEndingSaveUiBlocked = true;
        Log::Printf(
            "Scene8::Fn2 post-ending SaveUi19148 start blocked: seedShapeOk=%d seedWriterOk=%d",
            seedShapeOk ? 1 : 0,
            seedWriterOk ? 1 : 0);
        return false;
    }

    s_postEndingSaveUiActive =
        PrStage1SaveUiDirect::Start19148(ctx, &seed80092F10);
    Log::Printf(
        "Scene8::Fn2 post-ending SaveUi19148 start attempted: started=%d",
        s_postEndingSaveUiActive ? 1 : 0);
    if (!s_postEndingSaveUiActive) {
        s_postEndingSaveUiBlocked = true;
    } else {
        s_postEndingSaveUiLowerFeedbackRequests = {};
    }
    return s_postEndingSaveUiActive;
}

static int TickPostEndingSaveUi19148(PrGameContext& ctx) {
    if (!s_postEndingSaveUiActive || s_postEndingSaveUiDone) {
        return s_postEndingSaveUiDone ? 0 : 7;
    }
    const PrStage1SaveUi19148LowerFeedbackRequestList pendingRequests =
        s_postEndingSaveUiLowerFeedbackRequests;
    const PrStage1SaveUiHostBridgeDirect::SaveUi19148HostTickAttempt tick =
        LifecycleHost801C81EC::TickSaveUi19148Standalone801C81EC(
            ctx,
            pendingRequests,
            &s_postEndingSaveUiLowerFeedbackRequests);
    Log::Printf(
        "Scene8::Fn2 post-ending SaveUi19148 tick: active=%d done=%d result=%d state=%d event=%d lowerRequests=%u pendingRequests=%u inputConsumes=%d inputRaw=%d inputAfterDedup=%d inputHandled=%d inputState=%d/%d io=%d/%d cardIo=%d/%d directory=%d/%d write=%d/%d",
        tick.active ? 1 : 0,
        tick.done ? 1 : 0,
        tick.saveResult,
        tick.psxState,
        tick.psxEventId,
        tick.lowerFeedbackRequests.count,
        pendingRequests.count,
        tick.inputStateConsumes80018FB0 ? 1 : 0,
        tick.inputMaskRaw80035510,
        tick.inputMaskAfterDedup80018FB0,
        tick.inputHandled800185D0 ? 1 : 0,
        tick.inputStateBefore800185D0,
        tick.inputStateAfter800185D0,
        tick.ioResultKnown ? 1 : 0,
        tick.ioResult,
        tick.cardIoStateBeforeKnown80017594 ? 1 : 0,
        tick.cardIoStateAfterKnown80017594 ? 1 : 0,
        pendingRequests.count > 0 &&
                pendingRequests.requests[0].kind ==
                    PrStage1SaveUi19148LowerFeedbackRequestKind::
                        DirectoryRows80019458
            ? 1
            : 0,
        tick.lowerFeedbackRequests.count > 0 &&
                tick.lowerFeedbackRequests.requests[0].kind ==
                    PrStage1SaveUi19148LowerFeedbackRequestKind::
                        DirectoryRows80019458
            ? 1
            : 0,
        pendingRequests.count > 0 &&
                pendingRequests.requests[0].kind ==
                    PrStage1SaveUi19148LowerFeedbackRequestKind::Write80017A10
            ? 1
            : 0,
        tick.lowerFeedbackRequests.count > 0 &&
                tick.lowerFeedbackRequests.requests[0].kind ==
                    PrStage1SaveUi19148LowerFeedbackRequestKind::Write80017A10
            ? 1
            : 0);
    if (tick.done) {
        s_postEndingSaveUiDone = true;
        s_postEndingSaveUiActive = false;
        s_postEndingSaveUiLowerFeedbackRequests = {};
        s_postEndingSaveUiResult = tick.saveResult;
        Log::Printf(
            "Scene8::Fn2 post-ending SaveUi19148 done result=%d success=%d",
            tick.saveResult,
            tick.saveSucceeded ? 1 : 0);
        return 0;
    }
    return 7;
}

int Fn0(PrGameContext& ctx) {
    Log::Printf("Scene8::Fn0 init global pointers");
    s_resultMoviePlayed = false;
    s_resultMovieStarted = false;
    s_postEndingTailApplied = false;
    s_postEndingSaveUiStartAttempted = false;
    s_postEndingSaveUiActive = false;
    s_postEndingSaveUiDone = false;
    s_postEndingSaveUiBlocked = false;
    s_postEndingSaveUiResult = 0;
    s_postEndingSaveUiLowerFeedbackRequests = {};
    return 7;
}
void Fn1(PrGameContext& ctx) {
    Log::Printf("Scene8::Fn1 (no COMPO)");
}
int Fn2(PrGameContext& ctx) {
    if (!s_resultMoviePlayed) {
        const std::filesystem::path moviePath =
            (ctx.currentSceneDef != nullptr)
                ? (ctx.dataRoot / ctx.currentSceneDef->resultMovieA.path)
                : std::filesystem::path{};
        if (ctx.strPlayer && !moviePath.empty()) {
            if (!s_resultMovieStarted) {
                ctx.strPlayer->Stop();
                if (std::filesystem::exists(moviePath) &&
                    ctx.strPlayer->Play(moviePath)) {
                    s_resultMovieStarted = true;
                    Log::Printf("Scene8::Fn2 ending movie start path='%s'",
                                moviePath.u8string().c_str());
                } else {
                    Log::Printf("Scene8::Fn2 ending movie unavailable path='%s'",
                                moviePath.u8string().c_str());
                    s_resultMoviePlayed = true;
                    s_resultMovieStarted = false;
                }
            }

            if (s_resultMovieStarted) {
                const StrPlayerResult r = ctx.strPlayer->Update(ctx.debugF1_StrSkip);
                if (r == StrPlayerResult::Playing) {
                    return 7;
                }
                ctx.strPlayer->Stop();
                s_resultMoviePlayed = true;
                s_resultMovieStarted = false;
                Log::Printf("Scene8::Fn2 ending movie complete result=%d",
                            static_cast<int>(r));
            }
        } else {
            Log::Printf("Scene8::Fn2 ending movie no player/path");
            s_resultMoviePlayed = true;
        }
    }
    if (!s_postEndingTailApplied) {
        s_postEndingTailApplied = true;
        ctx.sceneExitReason = 3;
        const bool word800916F0Known =
            PrSS0Scene0RuntimeDirect::IsWord800916F0RuntimeObservationKnown();
        const uint16_t word800916F0 =
            word800916F0Known ? PrSS0Scene0RuntimeDirect::GetWord800916F0() : 0u;
        Log::Printf(
            "Scene8::Fn2 post-ending tail policy: word_800916E0=3 word_800916F0Known=%d word_800916F0=%u",
            word800916F0Known ? 1 : 0,
            static_cast<unsigned>(word800916F0));
        if (word800916F0Known && word800916F0 != 1u) {
            Log::Printf(
                "Scene8::Fn2 post-ending save policy pending: 80015590/80019148 host-boundary connecting");
            if (TryStartPostEndingSaveUi19148(ctx)) {
                return TickPostEndingSaveUi19148(ctx);
            }
        } else if (!word800916F0Known) {
            Log::Printf(
                "Scene8::Fn2 post-ending save policy blocked: word_800916F0 source unknown");
        }
    }
    if (s_postEndingSaveUiActive) {
        return TickPostEndingSaveUi19148(ctx);
    }
    if (s_postEndingSaveUiBlocked) {
        Log::Printf(
            "Scene8::Fn2 post-ending save policy blocked: SaveUi19148 start gate");
    }
    return 0;
}
void Main(PrGameContext& ctx) {
    (void)ctx;
}
void Render(PrGameContext& ctx) {
    // Ending presentation is the original XMOVIE8.STR.  Do not expose the
    // former debug shell while the movie is decoding or while the post-ending
    // save UI is waiting for its native event data.
    if (RenderNativeStrFrame(ctx)) {
        return;
    }
    RenderNativeSceneFallback(ctx, 7);
}
}

// ========== Scene9 (idx8): Bonus Stage ==========
namespace PrScn9 {
int Fn0(PrGameContext& ctx) {
    Log::Printf("Scene9::Fn0 init global pointers");
    return 8;
}
void Fn1(PrGameContext& ctx) {
    Log::Printf("Scene9::Fn1 load COMPO09.INT + COMMON.INT");
}
int Fn2(PrGameContext& ctx) {
    return 0;
}
void Main(PrGameContext& ctx) {
    (void)ctx;
}
void Render(PrGameContext& ctx) {
    if (ctx.stageRunning) {
        GetStageRunner().Render(ctx);
        return;
    }
    if (RenderNativeStrFrame(ctx)) {
        return;
    }
    if (PrTmdRenderer::RenderStage(ctx, 0.0f, 0.0f, 1.0f)) {
        return;
    }
    RenderNativeSceneFallback(ctx, 8);
}
}
