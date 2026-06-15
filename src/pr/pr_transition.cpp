#include "pr_transition.h"
#include "pr_game_context.h"
#include "pr_sfx.h"
#include "../d3d11_renderer.h"
#include "../logger.h"

#include "../resource_manager.h"

#include <algorithm>
#include <cstdio>
#include <cmath>
#include <vector>
#include <array>
#include <limits>

namespace PrTransition {

struct HoldOverlayRuntime {
    int bitStep = 0;
    int frameIdx = 0;
    int frameCounter = 0;
    std::array<uint8_t, 192> overlay{};
};

struct LoadingCurtain15408Runtime {
    bool active = false;
    int16_t sceneExitReason = 0;
    bool callbackFired = false;
    uint32_t callbackCount = 0;
    bool packetSetupObserved = false;
    bool packetSetupReady = false;
    uint32_t packetSetupCount = 0;
    int totalFrameCount = 0;
    int phaseFrameCount = 0;
    HoldOverlayRuntime hold{};
};

static TransitionPhase s_phase = TransitionPhase::Idle;
static TransitionConfig s_config;
static int s_targetScene = -1;
static TransitionSource s_source = TransitionSource::Legacy;
static int s_totalFrameCount = 0;
static int s_phaseFrameCount = 0;
static bool s_switchTriggered = false;

static bool s_tileOrderInit = false;
static std::array<uint8_t, 192> s_tileOrderRow{};
static std::array<uint8_t, 192> s_tileOrderCol{};
static std::array<uint8_t, 192> s_tileOrderF180Row{};
static std::array<uint8_t, 192> s_tileOrderF180Col{};

static HoldOverlayRuntime s_transitionHoldRuntime{};
static LoadingCurtain15408Runtime s_loadingCurtain15408Runtime{};
static bool s_loggedTextureFallback = false;

static void InitTileOrderPsxTables();

static const uint32_t kHoldMask50730[12][2] = {
    {0x00000000u, 0x00000000u},
    {0x00000000u, 0x00000000u},
    {0x90210000u, 0x00000000u},
    {0x90210000u, 0x00000000u},
    {0x93270CE0u, 0x00000000u},
    {0xF4A91290u, 0x00000000u},
    {0x94A91290u, 0x00000000u},
    {0x94A91290u, 0x00000000u},
    {0x93270C95u, 0x55555554u},
    {0x00000000u, 0x00000000u},
    {0x00000000u, 0x00000000u},
    {0x00000000u, 0x00000000u},
};

static const uint32_t kHoldMask50790[12][2] = {
    {0x00000000u, 0x00000000u},
    {0x00000000u, 0x00000000u},
    {0x90000000u, 0x00000000u},
    {0x93000000u, 0x00000000u},
    {0x90B8E19Cu, 0x00000000u},
    {0xF3A52252u, 0x00000000u},
    {0x94A52252u, 0x00000000u},
    {0x94A52252u, 0x00000000u},
    {0x93A4E192u, 0xAAAAAAAAu},
    {0x00002000u, 0x00000000u},
    {0x0000C000u, 0x00000000u},
    {0x00000000u, 0x00000000u},
};

static const uint32_t kHoldMask507F0[12][2] = {
    {0x00000000u, 0x00000000u},
    {0x00000000u, 0x00000000u},
    {0x88008000u, 0x00000000u},
    {0x89888060u, 0x00000000u},
    {0x8841C010u, 0x1CC60000u},
    {0x89C88070u, 0x21290000u},
    {0xAA488090u, 0x19E80000u},
    {0xDA488090u, 0x05090000u},
    {0x89C84070u, 0x38C65554u},
    {0x00000000u, 0x00000000u},
    {0x00000000u, 0x00000000u},
    {0x00000000u, 0x00000000u},
};

static const uint32_t kHoldMask50850[12][2] = {
    {0x00000000u, 0x00000000u},
    {0x00000000u, 0x00000000u},
    {0x48090000u, 0x00000000u},
    {0x48090000u, 0x00000000u},
    {0x49891800u, 0x00000000u},
    {0x7A492400u, 0x00000000u},
    {0x4BC92400u, 0x00000000u},
    {0x4A092400u, 0x00000000u},
    {0x498918AAu, 0xAAAAAAAAu},
    {0x00000000u, 0x00000000u},
    {0x00000000u, 0x00000000u},
    {0x00000000u, 0x00000000u},
};

static const uint32_t kHoldColors[4] = {0x401C0B5Au, 0x40003F1Eu, 0x40AE2A02u, 0x4053005Du};
static const int kHoldLimit[4] = {2, 2, 2, 2};

static const uint8_t kTileOrderEB80Row[192] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 11, 11, 11, 11, 11,
    11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 10, 9, 8, 7, 6, 5,
    4, 3, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 10, 10, 10, 10, 10,
    10, 10, 10, 10, 10, 10, 10, 10, 9, 8, 7, 6, 5, 4, 3, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 4, 5, 6,
    7, 8, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 8, 7,
    6, 5, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 5,
    6, 7, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 7, 6, 5, 4,
    4, 4, 4, 4, 4, 4, 4, 4, 5, 6, 7, 7, 7, 7, 7, 7,
    7, 7, 6, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6
};
static const uint8_t kTileOrderEB80Col[192] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 14, 13, 12, 11, 10,
    9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
    13, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 13, 12, 11, 10, 9,
    8, 7, 6, 5, 4, 3, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 13, 13, 13, 13,
    13, 13, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 2, 2,
    2, 2, 2, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 12, 12,
    12, 12, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 3, 3, 3, 3,
    4, 5, 6, 7, 8, 9, 10, 11, 11, 11, 11, 10, 9, 8, 7, 6,
    5, 4, 4, 4, 5, 6, 7, 8, 9, 10, 10, 9, 8, 7, 6, 5
};

static const uint8_t kTileOrderF180Row[192] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 11, 11, 11, 11,
    11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 10, 9, 8, 7, 6,
    5, 4, 3, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 10, 10,
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 9, 8, 7, 6, 5,
    4, 3, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    2, 3, 4, 5, 6, 7, 8, 9, 9, 9, 9, 9, 9, 9, 9, 9,
    9, 9, 9, 8, 7, 6, 5, 4, 3, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 3, 4, 5, 6, 7, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 7, 6, 5, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    4, 5, 6, 7, 7, 7, 7, 7, 7, 7, 7, 6, 5, 4, 4, 4,
    4, 4, 4, 4, 5, 6, 6, 6, 6, 6, 6, 5, 5, 5, 5, 5
};
static const uint8_t kTileOrderF180Col[192] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4,
    5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5,
    4, 3, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 3,
    4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 14, 14, 14, 14, 14,
    14, 14, 14, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 3, 4, 5, 6, 7, 8, 9, 10,
    11, 12, 13, 13, 13, 13, 13, 13, 13, 13, 12, 11, 10, 9, 8, 7,
    6, 5, 4, 3, 3, 3, 3, 3, 3, 3, 4, 5, 6, 7, 8, 9,
    10, 11, 12, 12, 12, 12, 12, 12, 11, 10, 9, 8, 7, 6, 5, 4,
    4, 4, 4, 4, 5, 6, 7, 8, 9, 10, 11, 11, 11, 11, 10, 9,
    8, 7, 6, 5, 5, 5, 6, 7, 8, 9, 10, 10, 9, 8, 7, 6
};

static int MapHoldStyle(int16_t sceneExitReason) {
    if (sceneExitReason == 2) {
        return 1;
    }
    if (sceneExitReason == 3) {
        return 2;
    }
    if (sceneExitReason == 4) {
        return 3;
    }
    return 0;
}

static void ResetHoldOverlayRuntime(HoldOverlayRuntime& runtime) {
    runtime.bitStep = 0;
    runtime.frameIdx = 0;
    runtime.frameCounter = 0;
    runtime.overlay.fill(0);
}

static void TickHoldOverlayRuntime(HoldOverlayRuntime& runtime, int style) {
    runtime.frameCounter++;
    if ((runtime.frameCounter % 3) != 0) {
        return;
    }

    if (runtime.bitStep >= 32) {
        runtime.bitStep = 0;
        runtime.frameIdx++;
    }
    if (runtime.frameIdx >= kHoldLimit[style]) {
        runtime.frameIdx = 0;
        runtime.bitStep = 0;
    }

    const int bit = 31 - runtime.bitStep;
    for (int row = 0; row < 12; row++) {
        const size_t base = (size_t)row * 16;
        for (int col = 0; col < 15; col++) {
            runtime.overlay[base + (size_t)col] =
                runtime.overlay[base + (size_t)col + 1];
        }

        uint32_t m = 0;
        if (style == 1) {
            m = kHoldMask50790[row][runtime.frameIdx];
        } else if (style == 2) {
            m = kHoldMask507F0[row][runtime.frameIdx];
        } else if (style == 3) {
            m = kHoldMask50850[row][runtime.frameIdx];
        } else {
            m = kHoldMask50730[row][runtime.frameIdx];
        }

        runtime.overlay[base + 15] = (m & (1u << bit)) != 0;
    }

    runtime.bitStep++;
}

static void TickLoadingCurtainVisual1EF40() {
    const int style = MapHoldStyle(s_loadingCurtain15408Runtime.sceneExitReason);
    TickHoldOverlayRuntime(s_loadingCurtain15408Runtime.hold, style);
}

static void ApplyLoadingCurtainPacketSetup1EA74() {
    // Narrow direct-port carrier for the `sub_8001EA74(1,0)` packet/setup
    // wrapper. The host main loop still owns the actual D3D frame begin/end,
    // but this runtime now explicitly arms one callback-produced curtain
    // packet for the render side to consume.
    s_loadingCurtain15408Runtime.packetSetupObserved = true;
    s_loadingCurtain15408Runtime.packetSetupReady = true;
    ++s_loadingCurtain15408Runtime.packetSetupCount;
}

static bool ResolveTransitionTileResources(
    PrGameContext& ctx,
    D3D11Renderer*& outRenderer,
    std::array<ID3D11ShaderResourceView*, 4>& outTiles,
    std::array<TextureResource*, 4>& outTrTiles,
    float& outVx,
    float& outVy,
    float& outTileW,
    float& outTileH) {
    outRenderer = ctx.renderer;
    if (!outRenderer) {
        return false;
    }

    if (!s_tileOrderInit) {
        InitTileOrderPsxTables();
    }

    const float winW = (float)outRenderer->GetWidth();
    const float winH = (float)outRenderer->GetHeight();
    const float baseW = 320.0f;
    const float baseH = 240.0f;
    const float fitX = (baseW > 0.0f) ? (winW / baseW) : 1.0f;
    const float fitY = (baseH > 0.0f) ? (winH / baseH) : 1.0f;
    const float vs = (std::min)(fitX, fitY);
    outVx = (winW - baseW * vs) * 0.5f;
    outVy = (winH - baseH * vs) * 0.5f;
    outTileW = 20.0f * vs;
    outTileH = 20.0f * vs;

    if (!ctx.resources) {
        return false;
    }

    static const char* kTileNames[4] = {"S_FRM11", "S_FRM12", "S_FRM13", "S_FRM14"};
    for (int i = 0; i < 4; i++) {
        outTiles[(size_t)i] = ctx.resources->GetTextureView(kTileNames[i]);
        outTrTiles[(size_t)i] = ctx.resources->GetTexture(kTileNames[i]);
    }
    if (!outTiles[0] || !outTiles[1] || !outTiles[2] || !outTiles[3] ||
        !outTrTiles[0] || outTrTiles[0]->tim.width <= 0 ||
        outTrTiles[0]->tim.height <= 0) {
        return false;
    }

    outTileW = (float)outTrTiles[0]->tim.width * vs;
    outTileH = (float)outTrTiles[0]->tim.height * vs;
    return true;
}

static void DrawTransitionTileRange(
    D3D11Renderer* renderer,
    const std::array<ID3D11ShaderResourceView*, 4>& tiles,
    float vx,
    float vy,
    float tileW,
    float tileH,
    const uint8_t* orderRow,
    const uint8_t* orderCol,
    int keepFrom,
    int keepTo) {
    if (!renderer || keepTo < keepFrom) {
        return;
    }

    auto DrawChunked = [&](ID3D11ShaderResourceView* tex,
                           const std::vector<TexturedVertex>& verts) {
        if (!tex || verts.empty()) return;
        constexpr size_t kMaxVertsPerDraw = 4096;
        size_t i = 0;
        while (i < verts.size()) {
            size_t n = verts.size() - i;
            if (n > kMaxVertsPerDraw) n = kMaxVertsPerDraw;
            n -= (n % 6);
            if (!n) break;
            renderer->DrawTexturedTriangleBatch(tex, verts.data() + i, (int)n);
            i += n;
        }
    };

    auto AppendQuad = [](std::vector<TexturedVertex>& out,
                         float x,
                         float y,
                         float w,
                         float h) {
        const TexturedVertex v[6] = {
            { x,     y,     0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f },
            { x + w, y,     1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f },
            { x + w, y + h, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f },
            { x,     y,     0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f },
            { x + w, y + h, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f },
            { x,     y + h, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f },
        };
        out.insert(out.end(), &v[0], &v[6]);
    };

    std::array<std::vector<TexturedVertex>, 4> batch{};
    for (int i = 0; i < 4; i++) {
        batch[(size_t)i].reserve(192 * 6);
    }

    for (int oi = keepFrom; oi <= keepTo; oi++) {
        const int row = (int)orderRow[(size_t)oi];
        const int col = (int)orderCol[(size_t)oi];
        const int idx = ((row & 1) == 0) ? (col & 1) : (2 + (col & 1));
        if (!tiles[(size_t)idx]) {
            continue;
        }
        const float x = vx + (float)col * tileW;
        const float y = vy + (float)row * tileH;
        AppendQuad(batch[(size_t)idx], x, y, tileW, tileH);
    }

    for (int i = 0; i < 4; i++) {
        DrawChunked(tiles[(size_t)i], batch[(size_t)i]);
    }
}

static void DrawTransitionFallbackTileRange(
    D3D11Renderer* renderer,
    float vx,
    float vy,
    float tileW,
    float tileH,
    const uint8_t* orderRow,
    const uint8_t* orderCol,
    int keepFrom,
    int keepTo) {
    if (!renderer || keepTo < keepFrom) {
        return;
    }
    const int from = (std::max)(0, keepFrom);
    const int to = (std::min)(191, keepTo);
    for (int i = from; i <= to; ++i) {
        const int row = orderRow ? orderRow[i] : (i / 16);
        const int col = orderCol ? orderCol[i] : (i % 16);
        const float x = vx + (float)col * tileW;
        const float y = vy + (float)row * tileH;
        renderer->DrawRect(x, y, tileW + 0.75f, tileH + 0.75f,
                           0.02f, 0.025f, 0.03f, 0.96f);
    }
}

static void RenderHoldOverlayRects(
    D3D11Renderer* renderer,
    float vx,
    float vy,
    float tileW,
    float tileH,
    const std::array<uint8_t, 192>& overlay,
    int style) {
    if (!renderer) {
        return;
    }

    const uint32_t c = kHoldColors[style];
    const float a = (float)((c >> 24) & 0xFF) / 255.0f;
    const float r = (float)((c >> 16) & 0xFF) / 255.0f;
    const float g = (float)((c >> 8) & 0xFF) / 255.0f;
    const float b = (float)((c >> 0) & 0xFF) / 255.0f;

    for (int row = 0; row < 12; row++) {
        for (int col = 0; col < 16; col++) {
            if (!overlay[(size_t)row * 16 + (size_t)col]) {
                continue;
            }
            const float x = vx + (float)col * tileW;
            const float y = vy + (float)row * tileH;
            renderer->DrawRect(x, y, tileW, tileH, r, g, b, a);
        }
    }
}

static void RenderLoadingCurtainVisual1EF40(PrGameContext& ctx) {
    if (!s_loadingCurtain15408Runtime.packetSetupReady) {
        return;
    }

    D3D11Renderer* renderer = nullptr;
    std::array<ID3D11ShaderResourceView*, 4> tiles{};
    std::array<TextureResource*, 4> trTiles{};
    float vx = 0.0f;
    float vy = 0.0f;
    float tileW = 0.0f;
    float tileH = 0.0f;
    const bool haveTiles = ResolveTransitionTileResources(
        ctx, renderer, tiles, trTiles, vx, vy, tileW, tileH);
    if (!haveTiles) {
        if (!renderer) {
            return;
        }
        if (!s_loggedTextureFallback) {
            Log::Printf("PrTransition: using direct rectangle fallback");
            s_loggedTextureFallback = true;
        }
        DrawTransitionFallbackTileRange(
            renderer,
            vx,
            vy,
            tileW,
            tileH,
            s_tileOrderRow.data(),
            s_tileOrderCol.data(),
            0,
            191);
    } else {
        DrawTransitionTileRange(
            renderer,
            tiles,
            vx,
            vy,
            tileW,
            tileH,
            s_tileOrderRow.data(),
            s_tileOrderCol.data(),
            0,
            191);
    }
    const int style = MapHoldStyle(s_loadingCurtain15408Runtime.sceneExitReason);
    RenderHoldOverlayRects(
        renderer,
        vx,
        vy,
        tileW,
        tileH,
        s_loadingCurtain15408Runtime.hold.overlay,
        style);
    s_loadingCurtain15408Runtime.packetSetupReady = false;
}

void Init() {
    s_phase = TransitionPhase::Idle;
    s_targetScene = -1;
    s_source = TransitionSource::Legacy;
    s_totalFrameCount = 0;
    s_phaseFrameCount = 0;
    s_switchTriggered = false;
    s_tileOrderInit = false;
    ResetHoldOverlayRuntime(s_transitionHoldRuntime);
    s_loadingCurtain15408Runtime = LoadingCurtain15408Runtime{};
}

static void InitTileOrderPsxTables() {
    for (int i = 0; i < 192; i++) {
        s_tileOrderRow[(size_t)i] = kTileOrderEB80Row[(size_t)i];
        s_tileOrderCol[(size_t)i] = kTileOrderEB80Col[(size_t)i];

        s_tileOrderF180Row[(size_t)i] = kTileOrderF180Row[(size_t)i];
        s_tileOrderF180Col[(size_t)i] = kTileOrderF180Col[(size_t)i];
    }
    s_tileOrderInit = true;
}

static const uint8_t* SelectOrderRow(TransitionTileOrder order) {
    return (order == TransitionTileOrder::F180) ? s_tileOrderF180Row.data() : s_tileOrderRow.data();
}

static const uint8_t* SelectOrderCol(TransitionTileOrder order) {
    return (order == TransitionTileOrder::F180) ? s_tileOrderF180Col.data() : s_tileOrderCol.data();
}

bool Start(int targetScene,
           const TransitionConfig& config,
           TransitionSource source) {
    if (s_phase != TransitionPhase::Idle) {
        Log::Printf("PrTransition: already active, ignoring Start");
        return false;
    }

    s_targetScene = targetScene;
    s_source = source;
    s_config = config;
    s_totalFrameCount = 0;
    s_phaseFrameCount = 0;
    s_switchTriggered = false;
    s_phase = TransitionPhase::FadeOut;
    ResetHoldOverlayRuntime(s_transitionHoldRuntime);
    s_loadingCurtain15408Runtime = LoadingCurtain15408Runtime{};

    Log::Printf("PrTransition: Start fadeOut=%d hold=%d fadeIn=%d switchAt=%d target=%d source=%d",
                config.fadeOutFrames, config.holdFrames, config.fadeInFrames,
                config.switchAtFrame, targetScene, static_cast<int>(source));
    return true;
}

bool StartRevealOnly(const TransitionConfig& config,
                     TransitionSource source) {
    if (s_phase != TransitionPhase::Idle) {
        Log::Printf("PrTransition: already active, ignoring StartRevealOnly");
        return false;
    }

    s_targetScene = -1;
    s_source = source;
    s_config = config;
    s_totalFrameCount = 0;
    s_phaseFrameCount = 0;
    s_switchTriggered = true;
    s_phase = TransitionPhase::FadeIn;
    ResetHoldOverlayRuntime(s_transitionHoldRuntime);
    s_loadingCurtain15408Runtime = LoadingCurtain15408Runtime{};

    Log::Printf("PrTransition: StartRevealOnly fadeIn=%d order=%d source=%d",
                config.fadeInFrames,
                (int)config.fadeInOrder,
                static_cast<int>(source));
    return true;
}

bool StartLoadingCurtain15408(int16_t sceneExitReason,
                              TransitionSource source) {
    s_phase = TransitionPhase::Idle;
    s_targetScene = -1;
    s_source = source;
    s_config = TransitionConfig{};
    s_totalFrameCount = 0;
    s_phaseFrameCount = 0;
    s_switchTriggered = false;
    ResetHoldOverlayRuntime(s_transitionHoldRuntime);
    s_loadingCurtain15408Runtime = LoadingCurtain15408Runtime{};
    s_loadingCurtain15408Runtime.active = true;
    s_loadingCurtain15408Runtime.sceneExitReason = sceneExitReason;
    Log::Printf(
        "PrTransition: StartLoadingCurtain15408 sceneExitReason=%d source=%d",
        (int)sceneExitReason,
        static_cast<int>(source));
    return true;
}

void StopLoadingCurtain1545C() {
    if (!s_loadingCurtain15408Runtime.active && s_phase == TransitionPhase::Idle) {
        return;
    }
    s_loadingCurtain15408Runtime = LoadingCurtain15408Runtime{};
    Log::Printf("PrTransition: StopLoadingCurtain1545C");
}

bool HasLoadingCurtainCallbackFired15408() {
    return s_loadingCurtain15408Runtime.callbackFired;
}

static void TickLoadingCurtainCallback1537C() {
    // Narrow direct-port of `sub_8001537C()`:
    // - `ApplyLoadingCurtainPacketSetup1EA74()` stands in for the
    //   `sub_8001EA74(1,0)` packet/setup wrapper
    // - `TickLoadingCurtainVisual1EF40()` stands in for the visual
    //   `sub_8001EF40(...)` callback body
    // - `sub_80026ECC()` is mirrored as a shared audio-driver flush barrier
    // - `dword_8006ECD4 = 1` is carried as the callback-fired latch below
    ApplyLoadingCurtainPacketSetup1EA74();
    TickLoadingCurtainVisual1EF40();
    PrSfx::ApplySharedAudioDriverFlushBarrier26ECC();
    s_loadingCurtain15408Runtime.callbackFired = true;
    ++s_loadingCurtain15408Runtime.callbackCount;
}

int Update(PrGameContext& ctx) {
    if (s_loadingCurtain15408Runtime.active) {
        s_loadingCurtain15408Runtime.totalFrameCount++;
        s_loadingCurtain15408Runtime.phaseFrameCount++;
        TickLoadingCurtainCallback1537C();
        return -1;
    }

    if (s_phase == TransitionPhase::Idle) {
        return -1;
    }

    int result = -1;

    // 检查是否到达切场帧
    if (!s_switchTriggered && s_totalFrameCount >= s_config.switchAtFrame) {
        s_switchTriggered = true;
        if (s_targetScene >= 0) {
            result = s_targetScene;
            Log::Printf("PrTransition: switch triggered at frame %d -> scene %d", s_totalFrameCount, s_targetScene);
        }
    }

    s_totalFrameCount++;

    // 状态机推进
    switch (s_phase) {
        case TransitionPhase::FadeOut:
            s_phaseFrameCount++;
            if (s_phaseFrameCount >= s_config.fadeOutFrames) {
                s_phase = TransitionPhase::Hold;
                s_phaseFrameCount = 0;
                ResetHoldOverlayRuntime(s_transitionHoldRuntime);
                Log::Printf("PrTransition: entering Hold phase");
            }
            break;

        case TransitionPhase::Hold: {
            s_phaseFrameCount++;

            const int style = MapHoldStyle(ctx.sceneExitReason);
            TickHoldOverlayRuntime(s_transitionHoldRuntime, style);

            if (s_phaseFrameCount >= s_config.holdFrames) {
                if (s_config.fadeInFrames > 0) {
                    s_phase = TransitionPhase::FadeIn;
                    s_phaseFrameCount = 0;
                    Log::Printf("PrTransition: entering FadeIn phase");
                } else {
                    if (!s_switchTriggered && s_targetScene >= 0) {
                        s_switchTriggered = true;
                        result = s_targetScene;
                        Log::Printf("PrTransition: switch triggered at hold end -> scene %d", s_targetScene);
                    }
                    s_phase = TransitionPhase::Idle;
                    s_phaseFrameCount = 0;
                    Log::Printf("PrTransition: transition complete (no fade-in)");
                }
            }
            break;
        }

        case TransitionPhase::FadeIn:
            s_phaseFrameCount++;
            if (s_phaseFrameCount >= s_config.fadeInFrames) {
                s_phase = TransitionPhase::Idle;
                s_phaseFrameCount = 0;
                Log::Printf("PrTransition: transition complete");
            }
            break;

        default:
            break;
    }

    return result;
}

void Render(PrGameContext& ctx) {
    // WARNING(Non-unified): 该转场实现通过 ResourceManager::GetTextureView("FRM_UN01"/"FRM_OB01")
    // 直接铺砖绘制独立 TIM 纹理，不走 PSX 原生的 VRAM/tpage/clut/spriteTemplate 管线。
    // 当前保留用于不破坏现状；后续新增/扩展请优先按“Win 主程序扮演 PSX 主程序”的统一路线实现。
    const bool loadingCurtainActive = s_loadingCurtain15408Runtime.active;
    if (loadingCurtainActive) {
        RenderLoadingCurtainVisual1EF40(ctx);
        return;
    }

    const TransitionPhase phaseForRender = s_phase;
    if (phaseForRender == TransitionPhase::Idle) {
        return;
    }

    const float alpha = (std::min)(1.0f, (std::max)(0.0f, GetAlpha()));
    if (alpha <= 0.001f && phaseForRender != TransitionPhase::Hold) {
        return;
    }

    D3D11Renderer* renderer = nullptr;
    std::array<ID3D11ShaderResourceView*, 4> tiles{};
    std::array<TextureResource*, 4> trTiles{};
    float vx = 0.0f;
    float vy = 0.0f;
    float tileW = 0.0f;
    float tileH = 0.0f;
    if (!ResolveTransitionTileResources(
            ctx, renderer, tiles, trTiles, vx, vy, tileW, tileH)) {
        if (!renderer) {
            return;
        }
        if (!s_loggedTextureFallback) {
            Log::Printf("PrTransition: using direct rectangle fallback");
            s_loggedTextureFallback = true;
        }
    }

    int keepFrom = 0;
    int keepTo = 191;
    if (phaseForRender == TransitionPhase::FadeOut) {
        int fill = (int)std::floor(alpha * 192.0f + 0.0001f);
        if (fill < 0) fill = 0;
        if (fill > 192) fill = 192;
        keepFrom = 0;
        keepTo = fill - 1;
    } else if (phaseForRender == TransitionPhase::Hold) {
        keepFrom = 0;
        keepTo = 191;
    } else if (phaseForRender == TransitionPhase::FadeIn) {
        const float prog = 1.0f - alpha;
        int clear = (int)std::floor(prog * 192.0f + 0.0001f);
        if (clear < 0) clear = 0;
        if (clear > 192) clear = 192;
        keepFrom = clear;
        keepTo = 191;
    } else {
        keepFrom = 0;
        keepTo = -1;
    }
    if (keepTo < keepFrom) {
        return;
    }

    const uint8_t* orderRow = s_tileOrderRow.data();
    const uint8_t* orderCol = s_tileOrderCol.data();
    if (phaseForRender == TransitionPhase::FadeOut) {
        orderRow = SelectOrderRow(s_config.fadeOutOrder);
        orderCol = SelectOrderCol(s_config.fadeOutOrder);
    } else if (phaseForRender == TransitionPhase::FadeIn) {
        orderRow = SelectOrderRow(s_config.fadeInOrder);
        orderCol = SelectOrderCol(s_config.fadeInOrder);
    }

    if (tiles[0] && tiles[1] && tiles[2] && tiles[3]) {
        DrawTransitionTileRange(
            renderer, tiles, vx, vy, tileW, tileH, orderRow, orderCol, keepFrom, keepTo);
    } else {
        DrawTransitionFallbackTileRange(
            renderer, vx, vy, tileW, tileH, orderRow, orderCol, keepFrom, keepTo);
    }

    if (phaseForRender == TransitionPhase::Hold) {
        const int style = MapHoldStyle(ctx.sceneExitReason);
        RenderHoldOverlayRects(
            renderer,
            vx,
            vy,
            tileW,
            tileH,
            s_transitionHoldRuntime.overlay,
            style);
    }
}

bool IsActive() {
    return s_loadingCurtain15408Runtime.active || s_phase != TransitionPhase::Idle;
}

TransitionPhase GetPhase() {
    return s_loadingCurtain15408Runtime.active ? TransitionPhase::Hold : s_phase;
}

float GetAlpha() {
    if (s_loadingCurtain15408Runtime.active) {
        return 1.0f;
    }
    switch (s_phase) {
        case TransitionPhase::FadeOut: {
            if (s_config.fadeOutFrames <= 0) return 1.0f;
            float t = (float)s_phaseFrameCount / (float)s_config.fadeOutFrames;
            return t < 1.0f ? t : 1.0f;
        }
        case TransitionPhase::Hold:
            return 1.0f;
        case TransitionPhase::FadeIn: {
            if (s_config.fadeInFrames <= 0) return 0.0f;
            float t = (float)s_phaseFrameCount / (float)s_config.fadeInFrames;
            return t < 1.0f ? (1.0f - t) : 0.0f;
        }
        default:
            return 0.0f;
    }
}

int GetTotalFrameCount() {
    return s_loadingCurtain15408Runtime.active
               ? s_loadingCurtain15408Runtime.totalFrameCount
               : s_totalFrameCount;
}

int GetPhaseFrameCount() {
    return s_loadingCurtain15408Runtime.active
               ? s_loadingCurtain15408Runtime.phaseFrameCount
               : s_phaseFrameCount;
}

int GetTargetScene() {
    return s_loadingCurtain15408Runtime.active ? -1 : s_targetScene;
}

TransitionSource GetSource() {
    return s_source;
}

void Cancel() {
    s_phase = TransitionPhase::Idle;
    s_targetScene = -1;
    s_source = TransitionSource::Legacy;
    s_totalFrameCount = 0;
    s_phaseFrameCount = 0;
    s_switchTriggered = false;
    ResetHoldOverlayRuntime(s_transitionHoldRuntime);
    s_loadingCurtain15408Runtime = LoadingCurtain15408Runtime{};
    Log::Printf("PrTransition: cancelled");
}

void ResetHoldOverlayState1EF14() {
    // Direct-port the narrow `sub_8001EF14()` effect only:
    //   gp[49]=0; gp[50]=0; gp[51]=0; sub_8001EEAC(0)
    // Do not touch the broader transition phase machine here.
    ResetHoldOverlayRuntime(s_transitionHoldRuntime);
}

}  // namespace PrTransition
