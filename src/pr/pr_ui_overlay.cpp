#include "pr_ui_overlay.h"
#include "pr_psx_sprite_template_render.h"

#include <array>
#include <cstdint>
 #include <cstdio>
 #include <filesystem>
 #include <fstream>
 #include <sstream>
#include <string>
#include <vector>

#include "d3d11_renderer.h"
#include "logger.h"
#include "pr_event.h"
#include "pr_game_context.h"
#include "pr_stage1_scene1_draw_backend.h"
#include "pr_stage1_lifecycle_host_adapter_801c81ec.h"
#include "pr_stage1_save_ui_host_bridge_direct.h"
#include "pr_stage_scene_submit_backend.h"
#include "pr_ss0_event4_prompt_render_direct.h"
#include "pr_ss0_scene0_runtime_direct.h"
#include "pr_vtext.h"

#include "resource_manager.h"

#include <algorithm>
#include <cmath>
#include <cstring>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif

namespace PrUiOverlay {

using PrPsxSpriteTemplateRender::DrawPsxSpriteTemplate;
using PrPsxSpriteTemplateRender::DrawPsxSpriteTemplateSubrect;
using PrPsxSpriteTemplateRender::DrawPsxSpriteTemplateViaUiAtlas;
using PrPsxSpriteTemplateRender::DrawPsxSpriteTemplateScaled;
using PrPsxSpriteTemplateRender::FindLoadedTimTextureByTemplate;
using PrPsxSpriteTemplateRender::FindPracticeTimKeyByTemplate;
using PrPsxSpriteTemplateRender::MakePsxAttrForBpp;
using PrPsxSpriteTemplateRender::PsxBppFromAttr;
using PrPsxSpriteTemplateRender::PsxSpriteTemplate;

static D3D11Renderer* s_renderer = nullptr;

struct SystemTextCacheEntry {
    std::string key;
    std::wstring line1;
    std::wstring line2;
    ID3D11ShaderResourceView* srv = nullptr;
    int w = 0;
    int h = 0;
    D3D11Renderer* renderer = nullptr;
    int scale100 = 0;
    int baseW1000 = 0;
};

static std::vector<SystemTextCacheEntry> s_systemTextCache;

static HFONT CreateSmallUiFont(int heightPx, int widthPx, const wchar_t* face) {
    const BYTE quality = ANTIALIASED_QUALITY;
    return CreateFontW(-heightPx, widthPx, 0, 0,
                       FW_NORMAL, FALSE, FALSE, FALSE,
                       DEFAULT_CHARSET,
                       OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                       quality,
                       DEFAULT_PITCH | FF_DONTCARE,
                       face);
}

static int MeasureTextWidthPx(HDC hdc, const wchar_t* text) {
    if (!hdc || !text) return 0;
    SIZE sz = {};
    const int len = (int)wcslen(text);
    if (len <= 0) return 0;
    if (!GetTextExtentPoint32W(hdc, text, len, &sz)) return 0;
    return (int)sz.cx;
}

static void DestroySystemTextEntry(SystemTextCacheEntry& e) {
    if (e.srv && e.renderer) {
        e.renderer->DestroyTexture(e.srv);
    }
    e.srv = nullptr;
    e.w = 0;
    e.h = 0;
    e.renderer = nullptr;
    e.scale100 = 0;
    e.baseW1000 = 0;
}

static SystemTextCacheEntry* FindSystemTextEntry(const char* cacheKey) {
    if (!cacheKey) return nullptr;
    for (auto& e : s_systemTextCache) {
        if (e.key == cacheKey) {
            return &e;
        }
    }
    return nullptr;
}

static SystemTextCacheEntry* GetOrCreateSystemTextEntry(const char* cacheKey) {
    if (!cacheKey) return nullptr;
    if (SystemTextCacheEntry* e = FindSystemTextEntry(cacheKey)) {
        return e;
    }
    s_systemTextCache.emplace_back();
    s_systemTextCache.back().key = cacheKey;
    return &s_systemTextCache.back();
}

static bool EnsureSystemTextTexture(SystemTextCacheEntry& e,
                                   D3D11Renderer* renderer,
                                   float pixelScale,
                                   float baseW,
                                   const wchar_t* line1,
                                   const wchar_t* line2) {
    if (!renderer) return false;
    if (!line1) line1 = L"";
    if (!line2) line2 = L"";
    if (pixelScale <= 0.0f) pixelScale = 1.0f;

    const int scale100 = (int)(pixelScale * 100.0f + 0.5f);
    const int baseW1000 = (int)(baseW * 1000.0f + 0.5f);
    const bool textChanged = (e.line1 != line1) || (e.line2 != line2);
    const bool sizeChanged = (e.scale100 != scale100) || (e.baseW1000 != baseW1000);
    const bool rendererChanged = (e.renderer != renderer);

    if (e.srv && !textChanged && !sizeChanged && !rendererChanged) {
        return true;
    }
    if (e.srv) {
        DestroySystemTextEntry(e);
    }
    e.renderer = renderer;
    e.scale100 = scale100;
    e.baseW1000 = baseW1000;
    e.line1 = line1;
    e.line2 = line2;

    const int texW = (int)(baseW * pixelScale + 0.5f);
    if (texW <= 0) return false;

    HDC hdc = CreateCompatibleDC(nullptr);
    if (!hdc) return false;

    int chosenHeight = 0;
    const wchar_t* chosenFace = nullptr;
    int lineStep = 0;
    float bestScale = 0.0f;

    const wchar_t* faces[] = { L"Arial Narrow", L"Microsoft YaHei UI", L"SimSun", L"Segoe UI", L"Arial" };
    const int baseHeights[] = { 9, 8, 7, 6, 5, 4 };
    for (int hi = 0; hi < (int)(sizeof(baseHeights) / sizeof(baseHeights[0])); ++hi) {
        const int heightPx = (int)((float)baseHeights[hi] * pixelScale + 0.5f);
        if (heightPx <= 0) continue;
        for (int fi = 0; fi < (int)(sizeof(faces) / sizeof(faces[0])); ++fi) {
            const wchar_t* face = faces[fi];
            HFONT font = CreateSmallUiFont(heightPx, 0, face);
            if (!font) continue;
            HGDIOBJ oldFont = SelectObject(hdc, font);

            const int w1 = MeasureTextWidthPx(hdc, line1);
            const int w2 = MeasureTextWidthPx(hdc, line2);
            const int wMax = (w1 > w2) ? w1 : w2;
            float scale = 1.0f;
            if (wMax > texW && wMax > 0) {
                scale = (float)texW / (float)wMax;
            }
            if (scale < 0.05f) scale = 0.05f;
            if (scale > 1.0f) scale = 1.0f;

            TEXTMETRICW tm = {};
            GetTextMetricsW(hdc, &tm);

            SelectObject(hdc, oldFont);
            DeleteObject(font);

            if (tm.tmHeight <= 0) {
                continue;
            }

            if (scale > bestScale + 1e-6f || (fabs(scale - bestScale) <= 1e-6f && heightPx > chosenHeight)) {
                bestScale = scale;
                chosenHeight = heightPx;
                chosenFace = face;
                lineStep = (int)tm.tmHeight + 1;
            }
        }
    }

    if (chosenHeight == 0) {
        chosenHeight = (int)(6.0f * pixelScale + 0.5f);
        chosenFace = faces[0];
        lineStep = chosenHeight + 2;
    }
    if (lineStep <= 0) {
        lineStep = chosenHeight + 2;
    }

    const int minH = (int)(12.0f * pixelScale + 0.5f);
    const int texH = (lineStep * 2 < minH) ? minH : (lineStep * 2);

    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = texW;
    bmi.bmiHeader.biHeight = -texH;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    void* bits = nullptr;
    HBITMAP bmp = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &bits, nullptr, 0);
    if (!bmp || !bits) {
        if (bmp) DeleteObject(bmp);
        DeleteDC(hdc);
        return false;
    }
    HGDIOBJ oldBmp = SelectObject(hdc, bmp);

    RECT rc = { 0, 0, texW, texH };
    FillRect(hdc, &rc, (HBRUSH)GetStockObject(WHITE_BRUSH));
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, RGB(0, 0, 0));

    HFONT font = CreateSmallUiFont(chosenHeight, 0, chosenFace ? chosenFace : L"Microsoft YaHei UI");
    HGDIOBJ oldFont = font ? SelectObject(hdc, font) : nullptr;

    const int len1 = (int)wcslen(line1);
    const int len2 = (int)wcslen(line2);

    const int w1 = MeasureTextWidthPx(hdc, line1);
    const int w2 = MeasureTextWidthPx(hdc, line2);
    const int wMax = (w1 > w2) ? w1 : w2;
    float scale = 1.0f;
    if (wMax > texW && wMax > 0) {
        scale = (float)texW / (float)wMax;
        if (scale < 0.05f) scale = 0.05f;
        if (scale > 1.0f) scale = 1.0f;
    }

    const int virtW = (scale > 0.0f) ? (int)((float)texW / scale) : texW;
    int x1 = (virtW - w1) / 2;
    int x2 = (virtW - w2) / 2;
    if (x1 < 0) x1 = 0;
    if (x2 < 0) x2 = 0;

    const int oldGM = SetGraphicsMode(hdc, GM_ADVANCED);
    XFORM oldXf = {};
    GetWorldTransform(hdc, &oldXf);
    XFORM xf = {};
    xf.eM11 = scale;
    xf.eM22 = 1.0f;
    SetWorldTransform(hdc, &xf);

    if (len1 > 0) TextOutW(hdc, x1, 0, line1, len1);
    if (len2 > 0) TextOutW(hdc, x2, lineStep, line2, len2);

    SetWorldTransform(hdc, &oldXf);
    SetGraphicsMode(hdc, oldGM);

    if (font) {
        SelectObject(hdc, oldFont);
        DeleteObject(font);
    }

    std::vector<uint32_t> rgba;
    rgba.resize((size_t)texW * (size_t)texH);
    const uint8_t* src = (const uint8_t*)bits;
    const size_t n = (size_t)texW * (size_t)texH;
    for (size_t i = 0; i < n; i++) {
        const uint8_t bb = src[i * 4 + 0];
        const uint8_t gg = src[i * 4 + 1];
        const uint8_t rr = src[i * 4 + 2];
        const uint8_t lum = (uint8_t)((77u * (uint32_t)rr + 150u * (uint32_t)gg + 29u * (uint32_t)bb) >> 8);
        uint8_t aa = (uint8_t)(255u - lum);
        if (aa < 8u) {
            aa = 0u;
        }
        rgba[i] = (aa == 0u) ? 0u : (((uint32_t)aa << 24) | (255u << 16) | (255u << 8) | 255u);
    }

    SelectObject(hdc, oldBmp);
    DeleteObject(bmp);
    DeleteDC(hdc);

    e.srv = renderer->CreateTexture(rgba.data(), texW, texH);
    e.w = texW;
    e.h = texH;
    return e.srv != nullptr;
}

static void CalcPs1Viewport(const D3D11Renderer* r, float& outX, float& outY, float& outScale) {
    outX = 0.0f;
    outY = 0.0f;
    outScale = 1.0f;
    if (!r) return;
    const float winW = (float)r->GetWidth();
    const float winH = (float)r->GetHeight();
    const float baseW = 320.0f;
    const float baseH = 240.0f;
    const float fitX = (baseW > 0.0f) ? (winW / baseW) : 1.0f;
    const float fitY = (baseH > 0.0f) ? (winH / baseH) : 1.0f;
    const float scale = (std::min)(fitX, fitY);
    outScale = scale;
    outX = (winW - baseW * scale) * 0.5f;
    outY = (winH - baseH * scale) * 0.5f;
}

static float ToScreenX(float vx, float vs, float x) {
    return vx + x * vs;
}

static float ToScreenY(float vy, float vs, float y) {
    return vy + y * vs;
}

static void DrawRectUI(D3D11Renderer* r, float vx, float vy, float vs,
                       float x, float y, float w, float h,
                       float cr, float cg, float cb, float ca) {
    if (!r) return;
    r->DrawRect(ToScreenX(vx, vs, x), ToScreenY(vy, vs, y), w * vs, h * vs, cr, cg, cb, ca);
}

static void DrawTriangleUI(D3D11Renderer* r, float vx, float vy, float vs,
                           float x0, float y0,
                           float x1, float y1,
                           float x2, float y2,
                           float cr, float cg, float cb, float ca) {
    if (!r) return;
    r->DrawTriangle(ToScreenX(vx, vs, x0),
                    ToScreenY(vy, vs, y0),
                    ToScreenX(vx, vs, x1),
                    ToScreenY(vy, vs, y1),
                    ToScreenX(vx, vs, x2),
                    ToScreenY(vy, vs, y2),
                    cr,
                    cg,
                    cb,
                    ca);
}

static void SubmitSpriteUI(PrGameContext& ctx, float vx, float vy, float vs,
                           const char* texName,
                           float x, float y, float w, float h,
                           float r, float g, float b, float a,
                           int layer) {
    // WARNING(Non-unified): 当前 UI 主要通过 ResourceManager::GetTextureView(texName) 按“纹理名”
    // 直接绘制独立 TIM（以及部分系统字体/GDI 生成贴图），不走 PSX 原生的 VRAM/tpage/clut/spriteTemplate 管线。
    // 当前保留用于不破坏现状；后续新增/扩展请优先按“Win 主程序扮演 PSX 主程序”的统一路线实现。
    if (!ctx.renderer || !ctx.resources || !texName) return;
    ID3D11ShaderResourceView* srv = ctx.resources->GetTextureView(texName);
    if (!srv) return;

    D3D11Renderer::SpriteCmd cmd;
    cmd.texture = srv;
    cmd.x = ToScreenX(vx, vs, x);
    cmd.y = ToScreenY(vy, vs, y);
    cmd.w = w * vs;
    cmd.h = h * vs;
    cmd.u0 = 0.0f;
    cmd.v0 = 0.0f;
    cmd.u1 = 1.0f;
    cmd.v1 = 1.0f;
    cmd.r = r;
    cmd.g = g;
    cmd.b = b;
    cmd.a = a;
    cmd.blend = D3D11Renderer::BlendMode::Alpha;
    cmd.layer = layer;
    cmd.order = 0;
    ctx.renderer->SubmitSprite(cmd);
}

struct PracticeSymbolFlipState {
    bool init = false;
    int lastRound = -9999;
    struct Inner {
        int counter[36] = {};
        int angle[36] = {};
        int val[36] = {};
        int delta[36] = {};
    } st23f20, st24114;
    int16_t sx[36] = {};
    int16_t sy[36] = {};
};

struct PracticeStripSymbolState {
    int type = 0;
    float scaleX = 1.0f;
    float scaleY = 1.0f;
};

struct PracticeStripPortraitState {
    bool draw = false;
    float x = 0.0f;
};

struct PracticeStripRenderState {
    PracticeStripSymbolState symbols[18] = {};
    PracticeStripPortraitState teacher;
    PracticeStripPortraitState student;
};

static int16_t PrRsin4096(int angle) {
    const float twoPi = 6.283185307179586f;
    const float s = std::sin((float)angle * (twoPi / 4096.0f));
    const int v = (int)std::lround(s * 4096.0f);
    if (v < -32768) return (int16_t)-32768;
    if (v > 32767) return (int16_t)32767;
    return (int16_t)v;
}

static void ResetPracticeSymbolFlip(PracticeSymbolFlipState& st) {
    st.init = true;
    for (int i = 0; i < 36; i++) {
        st.st23f20.counter[i] = 0;
        st.st23f20.angle[i] = 0;
        st.st23f20.val[i] = 2048;
        st.st23f20.delta[i] = 2048;

        st.st24114.counter[i] = 0;
        st.st24114.angle[i] = 0;
        st.st24114.val[i] = 2048;
        st.st24114.delta[i] = 2048;

        st.sx[i] = 4096;
        st.sy[i] = 4096;
    }
}

static void UpdatePracticeSymbolFlipSet_23F20(int count,
                                              PracticeSymbolFlipState::Inner& st,
                                              int16_t* outSx, int16_t* outSy) {
    if (count <= 0) return;
    if (count > 36) count = 36;
    for (int i = 0; i < count; i++) {
        if (st.counter[i] >= 24) {
            continue;
        }
        if (st.counter[i] >= 6) {
            if (st.counter[i] >= 22) {
                outSx[i] = 4096;
                outSy[i] = 4096;
            } else {
                if (st.angle[i] >= 8193) st.angle[i] = 0;
                outSx[i] = PrRsin4096(st.angle[i]);
                outSy[i] = 4096;
                st.angle[i] += 256;
            }
        } else {
            const int s = st.val[i] + 4096;
            outSx[i] = (int16_t)s;
            outSy[i] = (int16_t)s;
            st.val[i] += st.delta[i];
            if (st.val[i] >= 4096) {
                st.delta[i] = -1024;
            }
        }
        st.counter[i] += 1;
    }
}

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

static bool ParseQuotedHexU32(const std::string& s, size_t& p, uint32_t& outVal) {
    p = SkipWs(s, p);
    if (p >= s.size() || s[p] != '"') return false;
    p++;
    p = SkipWs(s, p);
    if (p + 2 >= s.size()) return false;
    if (!(s[p] == '0' && (s[p + 1] == 'x' || s[p + 1] == 'X'))) return false;
    p += 2;

    uint32_t v = 0;
    int digits = 0;
    while (p < s.size()) {
        const char c = s[p];
        int d = -1;
        if (c >= '0' && c <= '9') d = c - '0';
        else if (c >= 'a' && c <= 'f') d = 10 + (c - 'a');
        else if (c >= 'A' && c <= 'F') d = 10 + (c - 'A');
        else break;
        v = (v << 4) | (uint32_t)d;
        digits++;
        p++;
    }
    while (p < s.size() && s[p] != '"') p++;
    if (p < s.size() && s[p] == '"') p++;
    if (digits <= 0) return false;
    outVal = v;
    return true;
}

struct Ev3PosTable {
    bool valid = false;
    float x[5] = {};
    float y[5] = {};
    uint32_t tplAddr[5][3] = {};
    PsxSpriteTemplate tpl[5][3] = {};
    bool tplValid[5][3] = {};
};

static bool ParseEv3PosTable(const std::string& json, const char* tableName, Ev3PosTable& out) {
    out = Ev3PosTable{};

    const std::string needle = std::string("\"name\": \"") + tableName + "\"";
    size_t pos = json.find(needle);
    if (pos == std::string::npos) return false;

    size_t p = FindKey(json, pos, "entries");
    if (p == std::string::npos) return false;
    p = json.find('[', p);
    if (p == std::string::npos) return false;
    p++;

    bool seen[5] = {false, false, false, false, false};
    int seenCount = 0;

    while (p < json.size()) {
        p = SkipWs(json, p);
        if (p >= json.size()) break;
        if (json[p] == ']') break;
        if (json[p] == ',') {
            p++;
            continue;
        }
        if (json[p] != '{') {
            p++;
            continue;
        }
        const size_t endBrace = json.find('}', p);
        if (endBrace == std::string::npos) break;
        const std::string obj = json.substr(p, endBrace - p + 1);

        size_t pl = FindKey(obj, 0, "lang");
        size_t px = FindKey(obj, 0, "x");
        size_t py = FindKey(obj, 0, "y");
        size_t pt0 = FindKey(obj, 0, "tpl0");
        size_t pt1 = FindKey(obj, 0, "tpl1");
        size_t pt2 = FindKey(obj, 0, "tpl2");
        if (pl != std::string::npos && px != std::string::npos && py != std::string::npos) {
            const int lang = ParseInt(obj, pl);
            const int x = ParseInt(obj, px);
            const int y = ParseInt(obj, py);
            if (lang >= 0 && lang < 5) {
                if (!seen[lang]) {
                    seen[lang] = true;
                    seenCount++;
                }
                out.x[lang] = (float)x;
                out.y[lang] = (float)y;

                uint32_t a0 = 0, a1 = 0, a2 = 0;
                if (pt0 != std::string::npos && ParseQuotedHexU32(obj, pt0, a0)) out.tplAddr[lang][0] = a0;
                if (pt1 != std::string::npos && ParseQuotedHexU32(obj, pt1, a1)) out.tplAddr[lang][1] = a1;
                if (pt2 != std::string::npos && ParseQuotedHexU32(obj, pt2, a2)) out.tplAddr[lang][2] = a2;
            }
        }

        p = endBrace + 1;
    }

    out.valid = (seenCount == 5);
    return out.valid;
}

static bool ParseSpriteTemplateByAddr(const std::string& json, uint32_t addr, PsxSpriteTemplate& outTpl) {
    char needle[64];
    std::snprintf(needle, sizeof(needle), "\"addr\": \"0x%08X\"", (unsigned)addr);
    const std::string needleStr(needle);
    size_t search = 0;
    while (true) {
        const size_t pos = json.find(needleStr, search);
        if (pos == std::string::npos) return false;
        search = pos + needleStr.size();

        const size_t brace = json.rfind('{', pos);
        const size_t endBrace = json.find('}', pos);
        if (brace == std::string::npos || endBrace == std::string::npos || endBrace <= brace) {
            continue;
        }
        const std::string obj = json.substr(brace, endBrace - brace + 1);

        size_t pa = FindKey(obj, 0, "attr");
        size_t ptx = FindKey(obj, 0, "texX_hw");
        size_t pty = FindKey(obj, 0, "texY_px");
        size_t pw = FindKey(obj, 0, "w");
        size_t ph = FindKey(obj, 0, "h");
        size_t pcx = FindKey(obj, 0, "clutX_px");
        size_t pcy = FindKey(obj, 0, "clutY_px");
        if (pa == std::string::npos || ptx == std::string::npos || pty == std::string::npos ||
            pw == std::string::npos || ph == std::string::npos || pcx == std::string::npos ||
            pcy == std::string::npos) {
            continue;
        }

        uint32_t attr = 0;
        if (!ParseQuotedHexU32(obj, pa, attr)) continue;

        const int texX_hw = ParseInt(obj, ptx);
        const int texY_px = ParseInt(obj, pty);
        const int w = ParseInt(obj, pw);
        const int h = ParseInt(obj, ph);
        const int clutX_px = ParseInt(obj, pcx);
        const int clutY_px = ParseInt(obj, pcy);
        if (texX_hw < 0 || texY_px < 0 || w < 0 || h < 0 || clutX_px < 0 || clutY_px < 0) {
            continue;
        }

        outTpl.attr = attr;
        outTpl.texX_hw = (uint16_t)texX_hw;
        outTpl.texY_px = (uint16_t)texY_px;
        outTpl.w = (uint16_t)w;
        outTpl.h = (uint16_t)h;
        outTpl.clutX_px = (uint16_t)clutX_px;
        outTpl.clutY_px = (uint16_t)clutY_px;
        return true;
    }
}

static void ResolveEv3PosTableTemplates(const std::string& json, Ev3PosTable& t) {
    if (!t.valid) return;
    for (int lang = 0; lang < 5; lang++) {
        for (int st = 0; st < 3; st++) {
            t.tplValid[lang][st] = false;
            const uint32_t addr = t.tplAddr[lang][st];
            if (addr == 0) continue;
            PsxSpriteTemplate tpl = {};
            if (ParseSpriteTemplateByAddr(json, addr, tpl)) {
                t.tpl[lang][st] = tpl;
                t.tplValid[lang][st] = true;
            }
        }
    }
}

struct ActiveEventFrameOverlay {
    const PrPsxEventFrameDirect::EventFrameState8001E750* state = nullptr;
};

static bool IsSS0DirectOwnedScene(const PrGameContext& ctx) {
    return (ctx.currentScene == PrSceneId::Scene0 ||
            ctx.currentScene == PrSceneId::Scene1) &&
           PrSS0Scene0RuntimeDirect::RuntimeEnabled();
}

static bool IsSS0DirectScene1(const PrGameContext& ctx) {
    return ctx.currentScene == PrSceneId::Scene1 &&
           PrSS0Scene0RuntimeDirect::RuntimeEnabled();
}

static ActiveEventFrameOverlay ResolveActiveEventFrameOverlay(
    PrGameContext& ctx) {
    ActiveEventFrameOverlay out{};

    if (IsSS0DirectScene1(ctx)) {
        out.state =
            PrStage1SaveUiHostBridgeDirect::
                GetActiveSaveUiEventFrameState8001E750();
        if (out.state != nullptr) {
            return out;
        }

        const PrStage1LifecycleHostAdapter801C81EC::
            AbortPollEvent4OverlayState801C81EC abortPoll =
                PrStage1LifecycleHostAdapter801C81EC::
                    GetActiveAbortPollEvent4OverlayState801C81EC(ctx.frame);
        out.state = abortPoll.frameState;
        return out;
    }

    if (IsSS0DirectOwnedScene(ctx)) {
        return out;
    }

    out.state = PrEvent::GetActiveEventFrameState8001E750();
    return out;
}

static void RenderEventFrameDirectPackets(
    PrGameContext& ctx,
    const PrPsxEventFrameDirect::EventFrameState8001E750* state) {
    if (state == nullptr) {
        return;
    }
    if (ctx.currentScene == PrSceneId::Scene1 && ctx.renderer &&
        PrStage1SaveUiHostBridgeDirect::IsSaveEntryTransitionActive19148() &&
        state->lastClearImage80040420.softwareStateCommitted) {
        float vx, vy, vs;
        CalcPs1Viewport(ctx.renderer, vx, vy, vs);
        const auto& clear = state->lastClearImage80040420;
        D3D11Renderer::SolidRectCmd rect{};
        rect.x = vx; rect.y = vy; rect.w = 320.0f * vs; rect.h = 240.0f * vs;
        rect.r = float(clear.r) / 255.0f; rect.g = float(clear.g) / 255.0f;
        rect.b = float(clear.b) / 255.0f; rect.a = 1.0f;
        ctx.renderer->SubmitSolidRect(rect);
        ctx.renderer->FlushSprites();
    }
    if (ctx.currentScene == PrSceneId::Scene1 &&
        (state->stage1Event4PrepareUnderlayValid8001B120 ||
         state->stage1Event4MoveImageUnderlayValid8001B120)) {
        PrStage1Scene1DrawBackend::DrawGameplaySubmitFrozenRuntimeBaseOnly(ctx);
        if (ctx.renderer) {
            ctx.renderer->FlushSprites();
        }
        if (state->stage1Event4MoveImageUnderlayValid8001B120) {
            PrStageSceneSubmitBackend::DrawEventFrameMoveImageBoxFill8001B120(
                ctx,
                *state);
            if (ctx.renderer) {
                ctx.renderer->FlushSprites();
            }
        }
    }
    PrStageSceneSubmitBackend::DrawEventFrameFastSpritePackets8003FA20(ctx,
                                                                        *state);
    PrStageSceneSubmitBackend::DrawEventFrameBoxFillPackets8003EE84(ctx,
                                                                     *state);
}

struct PracticeUiLangTable1 {
    bool valid = false;
    float x[5] = {};
    float y[5] = {};
    uint32_t tplAddr[5] = {};
    PsxSpriteTemplate tpl[5] = {};
    bool tplValid[5] = {};
};

struct PracticeUiLangTable3 {
    bool valid = false;
    float x[5] = {};
    float y[5] = {};
    uint32_t tplAddr[5][3] = {};
    PsxSpriteTemplate tpl[5][3] = {};
    bool tplValid[5][3] = {};
};

struct PracticeUiFixedTemplate {
    bool valid = false;
    uint32_t tplAddr = 0;
    PsxSpriteTemplate tpl = {};
};

struct PracticeUiJsonCache {
    bool loaded = false;
    bool ok = false;
    std::filesystem::path loadedRoot;

    PracticeUiLangTable1 topTitle;
    PracticeUiLangTable1 topMessage;
    PracticeUiLangTable1 smallMark;

    PracticeUiLangTable1 kind0;
    PracticeUiLangTable1 kind1;
    PracticeUiLangTable1 kind1Hint;
    PracticeUiLangTable1 kind2;
    PracticeUiLangTable1 kind4;
    PracticeUiLangTable1 kind5;
    PracticeUiLangTable1 kind6;
    PracticeUiLangTable1 kind7;
    PracticeUiLangTable1 kind8;
    PracticeUiLangTable1 kind8Hint;

    PracticeUiLangTable3 exitText;
    PracticeUiLangTable3 exitSelect;

    PracticeUiFixedTemplate fixedFrame0;
    PracticeUiFixedTemplate fixedFrame1;
    PracticeUiFixedTemplate fixedCat;
    PracticeUiFixedTemplate fixedParappa;
    PracticeUiFixedTemplate slotStar;
    PracticeUiFixedTemplate slotDot;
    PracticeUiFixedTemplate exitButtonOff;
    PracticeUiFixedTemplate exitButtonOn;
    PracticeUiFixedTemplate exitSelectIdle;
    PracticeUiFixedTemplate exitSelectOff;
    PracticeUiFixedTemplate exitSelectOn;

    PracticeUiFixedTemplate ovKind0Deco;
    PracticeUiFixedTemplate ovKind1Frame;
    PracticeUiFixedTemplate ovKind1Left;
    PracticeUiFixedTemplate ovKind1Right;
    PracticeUiFixedTemplate ovKind1Bottom;
    PracticeUiFixedTemplate ovKind1Top;
    PracticeUiFixedTemplate ovKindCommonMsg;
    PracticeUiFixedTemplate ovKind4Frame;
    PracticeUiFixedTemplate ovKind4Bottom;
    PracticeUiFixedTemplate trackDigits;
    PracticeUiFixedTemplate promptTriangle;
    PracticeUiFixedTemplate promptCircle;

    PracticeUiFixedTemplate portraitTeacher;
    PracticeUiFixedTemplate portraitStudent;
};

static bool ParsePracticeUiLangTable1(const std::string& json, const char* tableName, PracticeUiLangTable1& out) {
    out = PracticeUiLangTable1{};
    const std::string needle = std::string("\"name\": \"") + tableName + "\"";
    size_t pos = json.find(needle);
    if (pos == std::string::npos) return false;

    size_t p = FindKey(json, pos, "entries");
    if (p == std::string::npos) return false;
    p = json.find('[', p);
    if (p == std::string::npos) return false;
    p++;

    bool seen[5] = {false, false, false, false, false};
    int seenCount = 0;
    while (p < json.size()) {
        p = SkipWs(json, p);
        if (p >= json.size() || json[p] == ']') break;
        if (json[p] == ',') {
            p++;
            continue;
        }
        if (json[p] != '{') {
            p++;
            continue;
        }

        const size_t endBrace = json.find('}', p);
        if (endBrace == std::string::npos) break;
        const std::string obj = json.substr(p, endBrace - p + 1);

        size_t pl = FindKey(obj, 0, "lang");
        size_t pt = FindKey(obj, 0, "tpl");
        size_t px = FindKey(obj, 0, "x");
        size_t py = FindKey(obj, 0, "y");
        if (pl != std::string::npos && pt != std::string::npos &&
            px != std::string::npos && py != std::string::npos) {
            const int lang = ParseInt(obj, pl);
            uint32_t addr = 0;
            if (lang >= 0 && lang < 5 && ParseQuotedHexU32(obj, pt, addr)) {
                if (!seen[lang]) {
                    seen[lang] = true;
                    seenCount++;
                }
                out.tplAddr[lang] = addr;
                out.x[lang] = (float)ParseInt(obj, px);
                out.y[lang] = (float)ParseInt(obj, py);
            }
        }

        p = endBrace + 1;
    }

    if (seenCount != 5) return false;
    out.valid = true;

    bool ok = true;
    for (int i = 0; i < 5; i++) {
        out.tplValid[i] = ParseSpriteTemplateByAddr(json, out.tplAddr[i], out.tpl[i]);
        ok &= out.tplValid[i];
    }
    return ok;
}

static bool ParsePracticeUiLangTable3(const std::string& json, const char* tableName, PracticeUiLangTable3& out) {
    out = PracticeUiLangTable3{};
    Ev3PosTable ev3 = {};
    if (!ParseEv3PosTable(json, tableName, ev3)) return false;
    ResolveEv3PosTableTemplates(json, ev3);

    out.valid = ev3.valid;
    bool ok = true;
    for (int i = 0; i < 5; i++) {
        out.x[i] = ev3.x[i];
        out.y[i] = ev3.y[i];
        for (int st = 0; st < 3; st++) {
            out.tplAddr[i][st] = ev3.tplAddr[i][st];
            out.tpl[i][st] = ev3.tpl[i][st];
            out.tplValid[i][st] = ev3.tplValid[i][st];
            ok &= out.tplValid[i][st];
        }
    }
    return ok;
}

static bool ParsePracticeUiFixed(const std::string& json, const char* fixedName, PracticeUiFixedTemplate& out) {
    out = PracticeUiFixedTemplate{};
    const std::string needle = std::string("\"name\": \"") + fixedName + "\"";
    const size_t pos = json.find(needle);
    if (pos == std::string::npos) return false;

    const size_t brace = json.rfind('{', pos);
    const size_t endBrace = json.find('}', pos);
    if (brace == std::string::npos || endBrace == std::string::npos || endBrace <= brace) return false;
    const std::string obj = json.substr(brace, endBrace - brace + 1);

    size_t pa = FindKey(obj, 0, "addr");
    if (pa == std::string::npos) return false;
    if (!ParseQuotedHexU32(obj, pa, out.tplAddr)) return false;

    out.valid = ParseSpriteTemplateByAddr(json, out.tplAddr, out.tpl);
    return out.valid;
}

static bool ArePracticeUiFixedTemplatesValid(const PracticeUiJsonCache& cache) {
    const PracticeUiFixedTemplate* fixed[] = {
        &cache.fixedFrame0,
        &cache.fixedFrame1,
        &cache.fixedCat,
        &cache.fixedParappa,
        &cache.slotStar,
        &cache.slotDot,
        &cache.exitButtonOff,
        &cache.exitButtonOn,
        &cache.exitSelectIdle,
        &cache.exitSelectOff,
        &cache.exitSelectOn,
        &cache.ovKind0Deco,
        &cache.ovKind1Frame,
        &cache.ovKind1Left,
        &cache.ovKind1Right,
        &cache.ovKind1Bottom,
        &cache.ovKind1Top,
        &cache.ovKindCommonMsg,
        &cache.ovKind4Frame,
        &cache.ovKind4Bottom,
        &cache.trackDigits,
        &cache.promptTriangle,
        &cache.promptCircle,
        &cache.portraitTeacher,
        &cache.portraitStudent,
    };
    for (const PracticeUiFixedTemplate* tpl : fixed) {
        if (!tpl || !tpl->valid) return false;
    }
    return true;
}

static const PracticeUiJsonCache& GetPracticeUiJsonCache(const std::filesystem::path& dataRoot) {
    static PracticeUiJsonCache s_cache;
    if (!s_cache.loaded || s_cache.loadedRoot != dataRoot) {
        s_cache = PracticeUiJsonCache{};
        s_cache.loaded = true;
        s_cache.loadedRoot = dataRoot;

        const std::filesystem::path p = dataRoot / "win" / "ex" / "json" / "psx_practice_ui_constants.json";
        const std::string json = ReadFileToString(p);
        if (json.empty()) {
            Log::Printf("PracticeUiJson: missing or empty: %s", p.string().c_str());
            return s_cache;
        }

        bool ok = true;
        ok &= ParsePracticeUiLangTable1(json, "TOP_TITLE", s_cache.topTitle);
        ok &= ParsePracticeUiLangTable1(json, "TOP_MESSAGE", s_cache.topMessage);
        ok &= ParsePracticeUiLangTable1(json, "SMALL_MARK", s_cache.smallMark);
        ok &= ParsePracticeUiLangTable1(json, "KIND0", s_cache.kind0);
        ok &= ParsePracticeUiLangTable1(json, "KIND1", s_cache.kind1);
        ok &= ParsePracticeUiLangTable1(json, "KIND1_HINT", s_cache.kind1Hint);
        ok &= ParsePracticeUiLangTable1(json, "KIND2", s_cache.kind2);
        ok &= ParsePracticeUiLangTable1(json, "KIND4", s_cache.kind4);
        ok &= ParsePracticeUiLangTable1(json, "KIND5", s_cache.kind5);
        ok &= ParsePracticeUiLangTable1(json, "KIND6", s_cache.kind6);
        ok &= ParsePracticeUiLangTable1(json, "KIND7", s_cache.kind7);
        ok &= ParsePracticeUiLangTable1(json, "KIND8", s_cache.kind8);
        ok &= ParsePracticeUiLangTable1(json, "KIND8_HINT", s_cache.kind8Hint);
        ok &= ParsePracticeUiLangTable3(json, "EXIT_TEXT", s_cache.exitText);
        ok &= ParsePracticeUiLangTable3(json, "EXIT_SELECT", s_cache.exitSelect);

        ok &= ParsePracticeUiFixed(json, "fixedFrame0", s_cache.fixedFrame0);
        ok &= ParsePracticeUiFixed(json, "fixedFrame1", s_cache.fixedFrame1);
        ok &= ParsePracticeUiFixed(json, "fixedCat", s_cache.fixedCat);
        ok &= ParsePracticeUiFixed(json, "fixedParappa", s_cache.fixedParappa);
        ok &= ParsePracticeUiFixed(json, "slotStar", s_cache.slotStar);
        ok &= ParsePracticeUiFixed(json, "slotDot", s_cache.slotDot);
        ok &= ParsePracticeUiFixed(json, "exitButtonOff", s_cache.exitButtonOff);
        ok &= ParsePracticeUiFixed(json, "exitButtonOn", s_cache.exitButtonOn);
        ok &= ParsePracticeUiFixed(json, "exitSelectIdle", s_cache.exitSelectIdle);
        ok &= ParsePracticeUiFixed(json, "exitSelectOff", s_cache.exitSelectOff);
        ok &= ParsePracticeUiFixed(json, "exitSelectOn", s_cache.exitSelectOn);
        ok &= ParsePracticeUiFixed(json, "ovKind0Deco", s_cache.ovKind0Deco);
        ok &= ParsePracticeUiFixed(json, "ovKind1Frame", s_cache.ovKind1Frame);
        ok &= ParsePracticeUiFixed(json, "ovKind1Left", s_cache.ovKind1Left);
        ok &= ParsePracticeUiFixed(json, "ovKind1Right", s_cache.ovKind1Right);
        ok &= ParsePracticeUiFixed(json, "ovKind1Bottom", s_cache.ovKind1Bottom);
        ok &= ParsePracticeUiFixed(json, "ovKind1Top", s_cache.ovKind1Top);
        ok &= ParsePracticeUiFixed(json, "ovKindCommonMsg", s_cache.ovKindCommonMsg);
        ok &= ParsePracticeUiFixed(json, "ovKind4Frame", s_cache.ovKind4Frame);
        ok &= ParsePracticeUiFixed(json, "ovKind4Bottom", s_cache.ovKind4Bottom);
        ok &= ParsePracticeUiFixed(json, "trackDigits", s_cache.trackDigits);
        ok &= ParsePracticeUiFixed(json, "promptTriangle", s_cache.promptTriangle);
        ok &= ParsePracticeUiFixed(json, "promptCircle", s_cache.promptCircle);
        ok &= ParsePracticeUiFixed(json, "portraitTeacher", s_cache.portraitTeacher);
        ok &= ParsePracticeUiFixed(json, "portraitStudent", s_cache.portraitStudent);

        ok &= ArePracticeUiFixedTemplatesValid(s_cache);
        s_cache.ok = ok;
        Log::Printf("PracticeUiJson: loaded=%d ok=%d", 1, ok ? 1 : 0);
    }
    return s_cache;
}

static bool TryGetPracticeUiTable1(const PracticeUiLangTable1& table, int langIndex,
                                   float& outX, float& outY, PsxSpriteTemplate& outTpl) {
    if (!table.valid) return false;
    int li = langIndex;
    if (li < 0) li = 0;
    if (li > 4) li = 4;
    if (!table.tplValid[li]) return false;
    outX = table.x[li];
    outY = table.y[li];
    outTpl = table.tpl[li];
    return true;
}

static bool TryGetPracticeUiTable3(const PracticeUiLangTable3& table, int langIndex, int stateIndex,
                                   float& outX, float& outY, PsxSpriteTemplate& outTpl) {
    if (!table.valid) return false;
    int li = langIndex;
    if (li < 0) li = 0;
    if (li > 4) li = 4;
    int si = stateIndex;
    if (si < 0) si = 0;
    if (si > 2) si = 2;
    if (!table.tplValid[li][si]) return false;
    outX = table.x[li];
    outY = table.y[li];
    outTpl = table.tpl[li][si];
    return true;
}

static void RenderVerticalMenu(PrGameContext& ctx, float vx, float vy, float vs,
                               const PrEventDispatcherContext& dispCtx, int eventId);
static char LangSuffix(int langIndex);
static void DrawTimAutoSize(PrGameContext& ctx, float vx, float vy, float vs,
                            const char* texName,
                            float x, float y,
                            float r, float g, float b, float a,
                            int layer);
static bool TryGetEv3ExitText(int langIndex, int state, const std::filesystem::path& dataRoot,
                              float& outX, float& outY, PsxSpriteTemplate& outTpl);

static bool FindPracticePromptIconSlot(const TextureResource& tr, float iconW, float iconH,
                                       float& outX, float& outY) {
    const uint32_t w = tr.tim.width;
    const uint32_t h = tr.tim.height;
    if (w < 16 || h < 16 || tr.tim.rgba.size() < (size_t)w * (size_t)h) return false;

    auto isDark = [&](uint32_t px) -> bool {
        const uint8_t a = (uint8_t)((px >> 24) & 0xFF);
        const uint8_t r = (uint8_t)((px >> 16) & 0xFF);
        const uint8_t g = (uint8_t)((px >> 8) & 0xFF);
        const uint8_t b = (uint8_t)(px & 0xFF);
        return a > 0x40 && ((int)r + (int)g + (int)b) <= 96;
    };

    struct Band {
        int y0 = 0;
        int y1 = 0;
        int score = 0;
    };

    std::vector<Band> bands;
    bool inBand = false;
    Band cur = {};
    for (uint32_t y = 0; y < h; ++y) {
        int darkCount = 0;
        for (uint32_t x = 0; x < w; ++x) {
            if (isDark(tr.tim.rgba[(size_t)y * w + x])) darkCount++;
        }
        if (darkCount > 0) {
            if (!inBand) {
                inBand = true;
                cur = Band{};
                cur.y0 = (int)y;
            }
            cur.y1 = (int)y;
            cur.score += darkCount;
        } else if (inBand) {
            bands.push_back(cur);
            inBand = false;
        }
    }
    if (inBand) bands.push_back(cur);
    if (bands.size() < 2) return false;

    const Band& line2 = bands[1];
    std::vector<std::pair<int, int>> darkRuns;
    bool inRun = false;
    int runStart = 0;
    for (uint32_t x = 0; x < w; ++x) {
        int darkCount = 0;
        for (int y = line2.y0; y <= line2.y1; ++y) {
            if (isDark(tr.tim.rgba[(size_t)y * w + x])) darkCount++;
        }
        if (darkCount > 0) {
            if (!inRun) {
                inRun = true;
                runStart = (int)x;
            }
        } else if (inRun) {
            darkRuns.emplace_back(runStart, (int)x - 1);
            inRun = false;
        }
    }
    if (inRun) darkRuns.emplace_back(runStart, (int)w - 1);
    if (darkRuns.size() < 2) return false;

    int bestGapStart = -1;
    int bestGapEnd = -1;
    int bestGapWidth = -1;
    for (size_t i = 0; i + 1 < darkRuns.size(); ++i) {
        const int gapStart = darkRuns[i].second + 1;
        const int gapEnd = darkRuns[i + 1].first - 1;
        const int gapWidth = gapEnd - gapStart + 1;
        if (gapWidth >= 10 && gapWidth > bestGapWidth) {
            bestGapWidth = gapWidth;
            bestGapStart = gapStart;
            bestGapEnd = gapEnd;
        }
    }
    if (bestGapWidth < 10) return false;

    outX = std::round((float)bestGapStart + std::max(0.0f, ((float)bestGapWidth - iconW) * 0.5f));
    outY = std::round((float)line2.y0 + ((float)(line2.y1 - line2.y0 + 1) - iconH) * 0.5f);
    if (outY < 0.0f) outY = 0.0f;
    return true;
}

static bool PickPracticeStripSymbolTemplate(int type, PsxSpriteTemplate& out) {
    out = PsxSpriteTemplate{};
    out.attr = 0x50000040;
    out.w = 16;
    out.h = 16;
    out.clutX_px = 0x0120;
    switch (type) {
        case 1: // GUI_SANK
            out.texX_hw = 0x03F8;
            out.texY_px = 0x01DD;
            out.clutY_px = 0x01EC;
            return true;
        case 2: // GUI_MARU
            out.texX_hw = 0x03F8;
            out.texY_px = 0x01CD;
            out.clutY_px = 0x01E7;
            return true;
        case 3: // GUI_PEKE
            out.texX_hw = 0x03FC;
            out.texY_px = 0x01CD;
            out.clutY_px = 0x01E8;
            return true;
        case 4: // GUI_SIKA
            out.texX_hw = 0x03FC;
            out.texY_px = 0x01DD;
            out.clutY_px = 0x01ED;
            return true;
        case 5:
        case 6: // GUI_L
            out.texX_hw = 0x03F4;
            out.texY_px = 0x01CD;
            out.clutY_px = 0x01E6;
            return true;
        case 7:
        case 8: // GUI_R
            out.texX_hw = 0x03F4;
            out.texY_px = 0x01DD;
            out.clutY_px = 0x01E9;
            return true;
        default:
            break;
    }
    return false;
}

static bool DrawPracticeFixedShared(PrGameContext& ctx, float vx, float vy, float vs,
                                    const PracticeUiFixedTemplate& tpl,
                                    float x, float y, int layer) {
    if (!tpl.valid) return false;
    if (DrawPsxSpriteTemplate(ctx, vx, vy, vs, x, y, tpl.tpl, 1, 1, 1, 1, layer)) return true;
    if (const char* key = nullptr; FindLoadedTimTextureByTemplate(ctx, tpl.tpl, &key) && key) {
        DrawTimAutoSize(ctx, vx, vy, vs, key, x, y, 1, 1, 1, 1, layer);
        return true;
    }
    return false;
}

static bool DrawPracticeFixedCenteredShared(PrGameContext& ctx, float vx, float vy, float vs,
                                            const PracticeUiFixedTemplate& tpl,
                                            float centerX, float centerY, int layer) {
    if (!tpl.valid) {
        return false;
    }
    return DrawPracticeFixedShared(ctx,
                                   vx,
                                   vy,
                                   vs,
                                   tpl,
                                   centerX - (float)tpl.tpl.w * 0.5f,
                                   centerY - (float)tpl.tpl.h * 0.5f,
                                   layer);
}

static void RenderPracticeStrip(PrGameContext& ctx, float vx, float vy, float vs,
                                const PracticeUiJsonCache& practiceUi,
                                const PracticeStripRenderState& stripState) {
    if (!practiceUi.ok) {
        return;
    }

    for (int i = 0; i < 18; i++) {
        const PracticeStripSymbolState& symbol = stripState.symbols[i];
        if (symbol.type < 1 || symbol.type > 8) {
            continue;
        }

        PsxSpriteTemplate tpl = {};
        if (!PickPracticeStripSymbolTemplate(symbol.type, tpl)) {
            continue;
        }

        const float cx = 41.0f + 14.0f * (float)i;
        const float cy = 99.0f;
        const float x = cx - (float)tpl.w * 0.5f;
        const float y = cy - (float)tpl.h * 0.5f;
        (void)DrawPsxSpriteTemplateScaled(ctx,
                                          vx,
                                          vy,
                                          vs,
                                          x,
                                          y,
                                          tpl,
                                          symbol.scaleX,
                                          symbol.scaleY,
                                          1,
                                          1,
                                          1,
                                          1,
                                          663);
    }

    if (practiceUi.slotStar.valid && practiceUi.slotDot.valid) {
        static const int kStarX[4] = {0x3F, 0x77, 0xAF, 0xE7};
        for (int i = 0; i < 4; i++) {
            DrawPracticeFixedShared(ctx, vx, vy, vs, practiceUi.slotStar, (float)kStarX[i], 93.0f, 662);
        }

        static const int kDotX[14] = {0x27, 0x35, 0x51, 0x5F, 0x6D, 0x89, 0x97, 0xA5, 0xC1, 0xCF, 0xDD, 0xF9, 0x107, 0x115};
        for (int i = 0; i < 14; i++) {
            DrawPracticeFixedShared(ctx, vx, vy, vs, practiceUi.slotDot, (float)kDotX[i], 97.0f, 661);
        }
    }

    if (practiceUi.portraitTeacher.valid && stripState.teacher.draw) {
        DrawPracticeFixedShared(ctx, vx, vy, vs, practiceUi.portraitTeacher, stripState.teacher.x, 93.0f, 664);
    }
    if (practiceUi.portraitStudent.valid && stripState.student.draw) {
        DrawPracticeFixedShared(ctx, vx, vy, vs, practiceUi.portraitStudent, stripState.student.x, 93.0f, 664);
    }
}

static void RenderPracticeRun(PrGameContext& ctx, float vx, float vy, float vs,
                              const PrEventDispatcherContext& dispCtx) {
    vx += ctx.scn0PanelOffsetX * vs;
    vy += ctx.scn0PanelOffsetY * vs;

    {
        static uint32_t s_lastDbgFrame = 0;
        if (ctx.frame - s_lastDbgFrame >= 60) {
            TextureResource* tr = ctx.resources ? ctx.resources->GetTexture("ycompo:prac_1b0") : nullptr;
            ID3D11ShaderResourceView* srv = ctx.resources ? ctx.resources->GetTextureView("ycompo:prac_1b0") : nullptr;
            const uint32_t gen = ctx.resources ? ctx.resources->GetGeneration() : 0u;
            const size_t texCount = ctx.resources ? ctx.resources->GetTextureCount() : 0u;
            const uint32_t w = tr ? (uint32_t)tr->tim.width : 0u;
            const uint32_t h = tr ? (uint32_t)tr->tim.height : 0u;
            const size_t rgba = tr ? tr->tim.rgba.size() : 0u;
            const int hasTrSrv = (tr && tr->srv) ? 1 : 0;
            Log::Printf(
                "RenderPracticeRun: frame=%u scn0Off=(%.2f,%.2f) v=(%.2f,%.2f) vs=%.3f gen=%u texCount=%llu tr=%d srv=%d trSrv=%d rgba=%llu wh=%ux%u ctxR=%d",
                (unsigned)ctx.frame, ctx.scn0PanelOffsetX, ctx.scn0PanelOffsetY, vx, vy, vs,
                (unsigned)gen, (unsigned long long)texCount,
                tr ? 1 : 0, srv ? 1 : 0, hasTrSrv,
                (unsigned long long)rgba, (unsigned)w, (unsigned)h,
                ctx.renderer ? 1 : 0);
            s_lastDbgFrame = ctx.frame;
        }
    }

    if (!ctx.resources) {
        RenderVerticalMenu(ctx, vx, vy, vs, dispCtx, 16);
        return;
    }

    const int lang = (int)ctx.languageIndex;
    const char lc = LangSuffix(lang);
    const char lcl = (char)std::tolower((unsigned char)lc);
    const PracticeUiJsonCache& practiceUi = GetPracticeUiJsonCache(ctx.dataRoot);

    auto drawPracticeTable1 = [&](const PracticeUiLangTable1& table, int layer) -> bool {
        float x = 0.0f;
        float y = 0.0f;
        PsxSpriteTemplate tpl = {};
        if (!TryGetPracticeUiTable1(table, lang, x, y, tpl)) return false;
        if (DrawPsxSpriteTemplateViaUiAtlas(ctx, vx, vy, vs, x, y, tpl, 1, 1, 1, 1, layer)) return true;
        if (DrawPsxSpriteTemplate(ctx, vx, vy, vs, x, y, tpl, 1, 1, 1, 1, layer)) return true;
        if (const char* key = nullptr; FindLoadedTimTextureByTemplate(ctx, tpl, &key) && key) {
            DrawTimAutoSize(ctx, vx, vy, vs, key, x, y, 1, 1, 1, 1, layer);
            return true;
        }
        return false;
    };

    auto drawPracticeTable3 = [&](const PracticeUiLangTable3& table, int stateIndex, int layer) -> bool {
        float x = 0.0f;
        float y = 0.0f;
        PsxSpriteTemplate tpl = {};
        if (!TryGetPracticeUiTable3(table, lang, stateIndex, x, y, tpl)) return false;
        if (DrawPsxSpriteTemplateViaUiAtlas(ctx, vx, vy, vs, x, y, tpl, 1, 1, 1, 1, layer)) return true;
        if (DrawPsxSpriteTemplate(ctx, vx, vy, vs, x, y, tpl, 1, 1, 1, 1, layer)) return true;
        if (const char* key = nullptr; FindLoadedTimTextureByTemplate(ctx, tpl, &key) && key) {
            DrawTimAutoSize(ctx, vx, vy, vs, key, x, y, 1, 1, 1, 1, layer);
            return true;
        }
        return false;
    };

    auto drawPracticeTable3LoadedFirst = [&](const PracticeUiLangTable3& table, int stateIndex, int layer) -> bool {
        float x = 0.0f;
        float y = 0.0f;
        PsxSpriteTemplate tpl = {};
        if (!TryGetPracticeUiTable3(table, lang, stateIndex, x, y, tpl)) return false;
        if (const char* key = nullptr; FindLoadedTimTextureByTemplate(ctx, tpl, &key) && key) {
            DrawTimAutoSize(ctx, vx, vy, vs, key, x, y, 1, 1, 1, 1, layer);
            return true;
        }
        if (const char* key = FindPracticeTimKeyByTemplate(ctx.dataRoot, tpl)) {
            if (ctx.resources->GetTexture(key)) {
                DrawTimAutoSize(ctx, vx, vy, vs, key, x, y, 1, 1, 1, 1, layer);
                return true;
            }
        }
        return drawPracticeTable3(table, stateIndex, layer);
    };

    auto drawPracticeFixed = [&](const PracticeUiFixedTemplate& tpl, float x, float y, int layer) -> bool {
        if (!tpl.valid) return false;
        if (DrawPsxSpriteTemplate(ctx, vx, vy, vs, x, y, tpl.tpl, 1, 1, 1, 1, layer)) return true;
        if (const char* key = nullptr; FindLoadedTimTextureByTemplate(ctx, tpl.tpl, &key) && key) {
            DrawTimAutoSize(ctx, vx, vy, vs, key, x, y, 1, 1, 1, 1, layer);
            return true;
        }
        return false;
    };

    auto drawPracticePromptIcon = [&](const PracticeUiLangTable1& table,
                                      const PracticeUiFixedTemplate& icon,
                                      int layer) -> bool {
        if (!icon.valid) return false;
        float msgX = 0.0f;
        float msgY = 0.0f;
        PsxSpriteTemplate msgTpl = {};
        if (!TryGetPracticeUiTable1(table, lang, msgX, msgY, msgTpl)) return false;

        TextureResource* msgTr = FindLoadedTimTextureByTemplate(ctx, msgTpl);
        if (!msgTr) {
            if (const char* key = FindPracticeTimKeyByTemplate(ctx.dataRoot, msgTpl)) {
                msgTr = ctx.resources->GetTexture(key);
            }
        }

        float slotX = 0.0f;
        float slotY = 0.0f;
        if (!msgTr || !FindPracticePromptIconSlot(*msgTr, (float)icon.tpl.w, (float)icon.tpl.h, slotX, slotY)) return false;
        return DrawPsxSpriteTemplate(ctx, vx, vy, vs,
                                     msgX + slotX, msgY + slotY,
                                     icon.tpl,
                                     1, 1, 1, 1, layer);
    };

    {
        PrEv16Ctx* ev16 = PrEvent::GetEv16CtxPtr();
        const int kind = ev16 ? (int)ev16->state1C : -1;
        const bool overlay = ev16 && ((ev16->flags00 & 0x00400000) != 0);
        if (overlay && practiceUi.ok) {
            switch (kind) {
                case 0:
                    drawPracticeFixed(practiceUi.ovKind0Deco, 54.0f, 196.0f, 666);
                    drawPracticeFixed(practiceUi.ovKindCommonMsg, 101.0f, 114.0f, 667);
                    drawPracticeTable1(practiceUi.kind0, 670);
                    break;
                case 1:
                    drawPracticeFixed(practiceUi.ovKind1Frame, 240.0f, 161.0f, 666);
                    drawPracticeFixed(practiceUi.ovKind1Left, 225.0f, 134.0f, 666);
                    drawPracticeFixed(practiceUi.ovKind1Right, 266.0f, 134.0f, 666);
                    drawPracticeFixed(practiceUi.ovKind1Bottom, 54.0f, 196.0f, 667);
                    drawPracticeFixed(practiceUi.ovKind1Top, 42.0f, 180.0f, 667);
                    drawPracticeFixed(practiceUi.ovKindCommonMsg, 101.0f, 114.0f, 668);
                    drawPracticeTable1(practiceUi.kind1, 670);
                    drawPracticeTable1(practiceUi.kind1Hint, 671);
                    break;
                case 2:
                    drawPracticeFixed(practiceUi.ovKind1Frame, 240.0f, 161.0f, 666);
                    drawPracticeFixed(practiceUi.ovKind1Left, 225.0f, 134.0f, 666);
                    drawPracticeFixed(practiceUi.ovKind1Right, 266.0f, 134.0f, 666);
                    drawPracticeFixed(practiceUi.ovKind1Bottom, 54.0f, 196.0f, 667);
                    drawPracticeFixed(practiceUi.ovKind1Top, 42.0f, 180.0f, 667);
                    drawPracticeFixed(practiceUi.ovKindCommonMsg, 101.0f, 114.0f, 668);
                    drawPracticeTable1(practiceUi.kind2, 670);
                    drawPracticeTable1(practiceUi.kind1Hint, 671);
                    break;
                case 4:
                    drawPracticeFixed(practiceUi.ovKind4Frame, 240.0f, 161.0f, 666);
                    drawPracticeFixed(practiceUi.ovKind4Bottom, 54.0f, 196.0f, 667);
                    drawPracticeFixed(practiceUi.ovKindCommonMsg, 101.0f, 114.0f, 668);
                    drawPracticeTable1(practiceUi.kind4, 670);
                    break;
                case 5:
                    drawPracticeFixed(practiceUi.ovKind4Frame, 240.0f, 161.0f, 666);
                    drawPracticeFixed(practiceUi.ovKind4Bottom, 54.0f, 196.0f, 667);
                    drawPracticeFixed(practiceUi.ovKindCommonMsg, 101.0f, 114.0f, 668);
                    drawPracticeTable1(practiceUi.kind5, 670);
                    break;
                case 6:
                    drawPracticeFixed(practiceUi.ovKind4Frame, 240.0f, 161.0f, 666);
                    drawPracticeFixed(practiceUi.ovKind4Bottom, 54.0f, 196.0f, 667);
                    drawPracticeFixed(practiceUi.ovKindCommonMsg, 101.0f, 114.0f, 668);
                    drawPracticeTable1(practiceUi.kind6, 670);
                    break;
                case 7:
                    drawPracticeFixed(practiceUi.ovKindCommonMsg, 101.0f, 114.0f, 668);
                    drawPracticeTable1(practiceUi.kind7, 670);
                    break;
                case 8:
                case 9:
                    drawPracticeTable1(practiceUi.kind8, 668);
                    drawPracticeTable1(practiceUi.kind8Hint, 669);
                    drawPracticeFixed(practiceUi.ovKind4Frame, 240.0f, 161.0f, 670);
                    drawPracticeFixed(practiceUi.ovKind4Bottom, 54.0f, 196.0f, 671);
                    drawPracticePromptIcon(practiceUi.kind8Hint, practiceUi.promptCircle, 672);
                    break;
                default:
                    break;
            }
        }
    }

    {
        if (!drawPracticeTable1(practiceUi.topTitle, 650)) {
            char b[64];
            char t[64];
            std::snprintf(b, sizeof(b), "ycompo:prac_1b0");
            std::snprintf(t, sizeof(t), "ycompo:prac_1%c0", lcl);
            DrawTimAutoSize(ctx, vx, vy, vs, b, 36.0f, 28.0f, 1, 1, 1, 1, 650);

            TextureResource* tb = ctx.resources->GetTexture(b);
            TextureResource* tt = ctx.resources->GetTexture(t);
            if (tb && tt) {
                const float x = 36.0f + ((float)tb->tim.width - (float)tt->tim.width) * 0.5f;
                const float y = 28.0f + ((float)tb->tim.height - (float)tt->tim.height) * 0.5f;
                DrawTimAutoSize(ctx, vx, vy, vs, t, x, y, 1, 1, 1, 1, 651);
            } else {
                DrawTimAutoSize(ctx, vx, vy, vs, t, 36.0f, 28.0f, 1, 1, 1, 1, 651);
            }
        }
        drawPracticeTable1(practiceUi.topMessage, 651);
        drawPracticePromptIcon(practiceUi.topMessage, practiceUi.promptTriangle, 652);
    }

    {
        if (practiceUi.trackDigits.valid) {
            static const int kDigitX[4] = {65, 122, 178, 234};
            static const int kDigitU[4] = {0, 7, 14, 21};
            for (int i = 0; i < 4; ++i) {
                DrawPsxSpriteTemplateSubrect(ctx, vx, vy, vs,
                                             (float)kDigitX[i], 81.0f,
                                             practiceUi.trackDigits.tpl,
                                             kDigitU[i], 7, 0,
                                             1, 1, 1, 1, 653);
            }
        }
    }

    {
        drawPracticeTable1(practiceUi.smallMark, 652);
        drawPracticeFixed(practiceUi.fixedFrame0, 32.0f, 34.0f, 648);
        drawPracticeFixed(practiceUi.fixedFrame1, 112.0f, 31.0f, 648);
        drawPracticeFixed(practiceUi.fixedCat, 211.0f, 113.0f, 648);
        drawPracticeFixed(practiceUi.fixedParappa, 26.0f, 119.0f, 648);
    }

    {
        PracticeStripRenderState stripState{};
        PrEv16Ctx* ev16 = PrEvent::GetEv16CtxPtr();
        const int8_t* stream = (ev16 && ev16->streamA94) ? ev16->streamA94 : nullptr;

        static PracticeSymbolFlipState s_flipTeacher;
        static PracticeSymbolFlipState s_flipStudent;
        static bool s_seqInit = false;
        static int s_lastSeqA = -9999;
        static int s_lastSeqB = -9999;
        if (ev16) {
            const int round = (int)ev16->round;
            const bool hardResetSeqAnim = (ev16->frame == -1 && ev16->seqCurA8C < 0 && ev16->seqCurB9E < 0);
            if (!s_flipTeacher.init || hardResetSeqAnim || round != s_flipTeacher.lastRound) {
                s_flipTeacher.lastRound = round;
                ResetPracticeSymbolFlip(s_flipTeacher);
            }
            if (!s_flipStudent.init || hardResetSeqAnim || round != s_flipStudent.lastRound) {
                s_flipStudent.lastRound = round;
                ResetPracticeSymbolFlip(s_flipStudent);
            }

            if (!s_seqInit || hardResetSeqAnim) {
                s_seqInit = true;
                s_lastSeqA = -9999;
                s_lastSeqB = -9999;
            }

            if (!ctx.renderOnlyFrame) {
                const int curA = (int)ev16->seqCurA8C;
                const int curB = (int)ev16->seqCurB9E;
                if (curA >= 0 && (s_lastSeqA < 0 || curA < s_lastSeqA)) {
                    ResetPracticeSymbolFlip(s_flipTeacher);
                }
                if (curB >= 0 && (s_lastSeqB < 0 || curB < s_lastSeqB)) {
                    ResetPracticeSymbolFlip(s_flipStudent);
                }
                s_lastSeqA = curA;
                s_lastSeqB = curB;
            }
            // Skip animation state advance on render-only frames (60fps mode)
            if (!ctx.renderOnlyFrame) {
                if (ev16->seqCurA8C >= 0) {
                    UpdatePracticeSymbolFlipSet_23F20(ev16->seqCurA8C, s_flipTeacher.st23f20, s_flipTeacher.sx, s_flipTeacher.sy);
                }
                if (ev16->seqCurB9E >= 0) {
                    UpdatePracticeSymbolFlipSet_23F20(ev16->seqCurB9E, s_flipStudent.st23f20, s_flipStudent.sx, s_flipStudent.sy);
                }
            }
        }

        if (stream) {
            PracticeSymbolFlipState::Inner teacherNext = s_flipTeacher.st23f20;
            PracticeSymbolFlipState::Inner studentNext = s_flipStudent.st23f20;
            int16_t teacherNextSx[36] = {};
            int16_t teacherNextSy[36] = {};
            int16_t studentNextSx[36] = {};
            int16_t studentNextSy[36] = {};
            bool haveTeacherNext = false;
            bool haveStudentNext = false;
            float renderFrac = 0.0f;
            if (ctx.renderOnlyFrame) {
                renderFrac = (float)ctx.renderSubFrame8 / 255.0f;
                if (renderFrac < 0.0f) renderFrac = 0.0f;
                if (renderFrac > 1.0f) renderFrac = 1.0f;
                if (ev16 && ev16->seqCurA8C >= 0) {
                    for (int i = 0; i < 36; ++i) {
                        teacherNextSx[i] = s_flipTeacher.sx[i];
                        teacherNextSy[i] = s_flipTeacher.sy[i];
                    }
                    UpdatePracticeSymbolFlipSet_23F20(ev16->seqCurA8C, teacherNext, teacherNextSx, teacherNextSy);
                    haveTeacherNext = true;
                }
                if (ev16 && ev16->seqCurB9E >= 0) {
                    for (int i = 0; i < 36; ++i) {
                        studentNextSx[i] = s_flipStudent.sx[i];
                        studentNextSy[i] = s_flipStudent.sy[i];
                    }
                    UpdatePracticeSymbolFlipSet_23F20(ev16->seqCurB9E, studentNext, studentNextSx, studentNextSy);
                    haveStudentNext = true;
                }
            }

            for (int i = 0; i < 18; i++) {
                const int type = (int)stream[i];
                if (type == -1) {
                    break;
                }
                if (type < 1 || type > 8) {
                    continue;
                }
                float sx = 1.0f;
                float sy = 1.0f;
                const int stCnt = s_flipStudent.st23f20.counter[i];
                const int tcCnt = s_flipTeacher.st23f20.counter[i];
                const bool stActive = (stCnt > 0 && stCnt < 24);
                const bool tcActive = (tcCnt > 0 && tcCnt < 24);
                const bool useStudent = stActive && (!tcActive || stCnt <= tcCnt);
                const bool useTeacher = tcActive && (!stActive || tcCnt < stCnt);
                if (useStudent) {
                    sx = (float)s_flipStudent.sx[i] / 4096.0f;
                    sy = (float)s_flipStudent.sy[i] / 4096.0f;
                    if (ctx.renderOnlyFrame && haveStudentNext && ev16 && i < ev16->seqCurB9E && stCnt > 0 && stCnt < 24) {
                        const float nextSx = (float)studentNextSx[i] / 4096.0f;
                        const float nextSy = (float)studentNextSy[i] / 4096.0f;
                        sx = sx + (nextSx - sx) * renderFrac;
                        sy = sy + (nextSy - sy) * renderFrac;
                    }
                } else if (useTeacher) {
                    sx = (float)s_flipTeacher.sx[i] / 4096.0f;
                    sy = (float)s_flipTeacher.sy[i] / 4096.0f;
                    if (ctx.renderOnlyFrame && haveTeacherNext && ev16 && i < ev16->seqCurA8C && tcCnt > 0 && tcCnt < 24) {
                        const float nextSx = (float)teacherNextSx[i] / 4096.0f;
                        const float nextSy = (float)teacherNextSy[i] / 4096.0f;
                        sx = sx + (nextSx - sx) * renderFrac;
                        sy = sy + (nextSy - sy) * renderFrac;
                    }
                }
                stripState.symbols[i].type = type;
                stripState.symbols[i].scaleX = sx;
                stripState.symbols[i].scaleY = sy;
            }
        }

        if (ev16 && ev16->mode != 0) {
            struct PracticePortraitCounterState {
                int lastValue = -9999;
                int frame = 0;
            };
            struct PracticePortraitDrawState {
                int idx = -1;
                bool draw = false;
                float x = 0.0f;
            };
            static bool s_init = false;
            static PracticePortraitCounterState s_shared;
            static PracticePortraitDrawState s_teacher;
            static PracticePortraitDrawState s_student;
            const bool hardResetPortraitAnim = (ev16->frame == -1 && ev16->seqCurA8C < 0 && ev16->seqCurB9E < 0);

            if (!s_init || hardResetPortraitAnim) {
                s_init = true;
                s_shared = PracticePortraitCounterState{};
                s_teacher = PracticePortraitDrawState{};
                s_student = PracticePortraitDrawState{};
            }

            auto advancePortrait24600 = [&](PracticePortraitDrawState& st, int idx) {
                if (idx == s_shared.lastValue) {
                    s_shared.frame += 1;
                } else {
                    s_shared.lastValue = idx;
                    s_shared.frame = 0;
                }
                if (s_shared.frame > 3) {
                    s_shared.frame = 3;
                }
                st.idx = idx;
                st.draw = idx > 0;
                st.x = 16.0f + 15.0f * (float)idx + 4.0f * (float)s_shared.frame;
            };

            if (!ctx.renderOnlyFrame) {
                s_teacher.draw = false;
                s_student.draw = false;
                s_teacher.idx = -1;
                s_student.idx = -1;

                if (practiceUi.portraitTeacher.valid && ev16->seqCurA8C >= 0) {
                    advancePortrait24600(s_teacher, (int)ev16->seqCurA8C);
                }
                if (practiceUi.portraitStudent.valid && ev16->seqCurB9E >= 0) {
                    advancePortrait24600(s_student, (int)ev16->seqCurB9E);
                }
            }

            if (practiceUi.portraitTeacher.valid && s_teacher.draw) {
                stripState.teacher.draw = true;
                stripState.teacher.x = s_teacher.x;
            }
            if (practiceUi.portraitStudent.valid && s_student.draw) {
                stripState.student.draw = true;
                stripState.student.x = s_student.x;
            }
        }

        RenderPracticeStrip(ctx, vx, vy, vs, practiceUi, stripState);
    }

    {
        PrEv16Ctx* ev16 = PrEvent::GetEv16CtxPtr();
        const int blink = (ev16 && ev16->blink4C != 0) ? 1 : 0;
        const bool exitConfirmFlow =
            ev16 && (ev16->state1C == 8) && ((ev16->flags00 & 0x00400000) != 0);
        const bool exitSelected = exitConfirmFlow && (ev16->exit48 != 0);
        const int exitTextState = !exitConfirmFlow ? 0 : (exitSelected ? 2 : 1);
        drawPracticeFixed(blink ? practiceUi.exitButtonOn : practiceUi.exitButtonOff, 231.0f, 179.0f, 692);
        drawPracticeTable3LoadedFirst(practiceUi.exitText, exitTextState, 691);
        if (practiceUi.exitSelect.valid) {
            int li = lang;
            if (li < 0) li = 0;
            if (li > 4) li = 4;
            const PracticeUiFixedTemplate& exitSelectTpl =
                !exitConfirmFlow ? practiceUi.exitSelectIdle
                                 : (exitSelected ? practiceUi.exitSelectOn : practiceUi.exitSelectOff);
            drawPracticeFixed(exitSelectTpl, practiceUi.exitSelect.x[li], practiceUi.exitSelect.y[li], 690);
        }
    }

}

} // namespace PrUiOverlay

namespace PrUiOverlay {

struct Ev3FixedSprite {
    bool valid = false;
    float x = 0.0f;
    float y = 0.0f;
    uint32_t tplAddr = 0;
    PsxSpriteTemplate tpl = {};
    bool tplValid = false;
};

struct Ev3Tpl3Layer {
    bool valid = false;
    float x = 0.0f;
    float y = 0.0f;
    uint32_t tplAddr[3] = {0, 0, 0};
    PsxSpriteTemplate tpl[3] = {};
    bool tplValid[3] = {false, false, false};
};

struct Ev3BottomFrame {
    bool valid = false;
    float x = 0.0f;
    float y = 0.0f;
    uint32_t tplAddr[2] = {0, 0};
    PsxSpriteTemplate tpl[2] = {};
    bool tplValid[2] = {false, false};
};

struct Ev3TopFrame {
    bool valid = false;
    float x = 0.0f;
    float y = 0.0f;
    uint32_t tplAddr[2] = {0, 0};
    PsxSpriteTemplate tpl[2] = {};
    bool tplValid[2] = {false, false};
};

struct Ev3Tpl2Frame {
    bool valid = false;
    float x = 0.0f;
    float y = 0.0f;
    uint32_t tplAddr[2] = {0, 0};
    PsxSpriteTemplate tpl[2] = {};
    bool tplValid[2] = {false, false};
};

struct Ev3Pos2 {
    bool valid = false;
    float x = 0.0f;
    float y = 0.0f;
};

static bool ParseEv3FixedSprites(const std::string& json, Ev3FixedSprite outSprites[4]) {
    if (!outSprites) return false;
    for (int i = 0; i < 4; i++) outSprites[i] = Ev3FixedSprite{};

    size_t p = FindKey(json, 0, "fixed_sprites");
    if (p == std::string::npos) return false;
    p = json.find('[', p);
    if (p == std::string::npos) return false;
    p++;

    int outCount = 0;
    while (p < json.size() && outCount < 4) {
        p = SkipWs(json, p);
        if (p >= json.size()) break;
        if (json[p] == ']') break;
        if (json[p] == ',') {
            p++;
            continue;
        }
        if (json[p] != '{') {
            p++;
            continue;
        }
        const size_t endBrace = json.find('}', p);
        if (endBrace == std::string::npos) break;
        const std::string obj = json.substr(p, endBrace - p + 1);

        size_t px = FindKey(obj, 0, "x");
        size_t py = FindKey(obj, 0, "y");
        size_t pt = FindKey(obj, 0, "tpl");
        if (px != std::string::npos && py != std::string::npos && pt != std::string::npos) {
            const int x = ParseInt(obj, px);
            const int y = ParseInt(obj, py);
            uint32_t tplAddr = 0;
            if (ParseQuotedHexU32(obj, pt, tplAddr)) {
                outSprites[outCount].x = (float)x;
                outSprites[outCount].y = (float)y;
                outSprites[outCount].tplAddr = tplAddr;
                outSprites[outCount].valid = true;
                outCount++;
            }
        }
        p = endBrace + 1;
    }

    return outCount == 4;
}

static bool ParseEv3ExitFrame(const std::string& json, Ev3Tpl2Frame& outFrame) {
    outFrame = Ev3Tpl2Frame{};
    size_t p = FindKey(json, 0, "exit_frame");
    if (p == std::string::npos) return false;
    p = json.find('{', p);
    if (p == std::string::npos) return false;
    const size_t endBrace = json.find('}', p);
    if (endBrace == std::string::npos) return false;
    const std::string obj = json.substr(p, endBrace - p + 1);

    size_t px = FindKey(obj, 0, "x");
    size_t py = FindKey(obj, 0, "y");
    size_t pt0 = FindKey(obj, 0, "tpl0");
    size_t pt1 = FindKey(obj, 0, "tpl1");
    if (px == std::string::npos || py == std::string::npos || pt0 == std::string::npos || pt1 == std::string::npos) return false;

    const int x = ParseInt(obj, px);
    const int y = ParseInt(obj, py);
    uint32_t tpl0 = 0;
    uint32_t tpl1 = 0;
    if (!ParseQuotedHexU32(obj, pt0, tpl0) || !ParseQuotedHexU32(obj, pt1, tpl1)) return false;
    outFrame.x = (float)x;
    outFrame.y = (float)y;
    outFrame.tplAddr[0] = tpl0;
    outFrame.tplAddr[1] = tpl1;
    outFrame.valid = true;
    return true;
}

static bool ParseEv3ExitBg(const std::string& json, Ev3Pos2& outPos) {
    outPos = Ev3Pos2{};
    size_t p = FindKey(json, 0, "exit_bg");
    if (p == std::string::npos) return false;
    p = json.find('{', p);
    if (p == std::string::npos) return false;
    const size_t endBrace = json.find('}', p);
    if (endBrace == std::string::npos) return false;
    const std::string obj = json.substr(p, endBrace - p + 1);

    size_t px = FindKey(obj, 0, "x");
    size_t py = FindKey(obj, 0, "y");
    if (px == std::string::npos || py == std::string::npos) return false;

    const int x = ParseInt(obj, px);
    const int y = ParseInt(obj, py);
    outPos.x = (float)x;
    outPos.y = (float)y;
    outPos.valid = true;
    return true;
}

static bool ParseEv3Tpl3LayerObj(const std::string& json, const char* key, Ev3Tpl3Layer& out) {
    out = Ev3Tpl3Layer{};
    size_t p = FindKey(json, 0, key);
    if (p == std::string::npos) return false;
    p = json.find('{', p);
    if (p == std::string::npos) return false;
    const size_t endBrace = json.find('}', p);
    if (endBrace == std::string::npos) return false;
    const std::string obj = json.substr(p, endBrace - p + 1);

    size_t px = FindKey(obj, 0, "x");
    size_t py = FindKey(obj, 0, "y");
    size_t pt0 = FindKey(obj, 0, "tpl0");
    size_t pt1 = FindKey(obj, 0, "tpl1");
    size_t pt2 = FindKey(obj, 0, "tpl2");
    if (px == std::string::npos || py == std::string::npos || pt0 == std::string::npos || pt1 == std::string::npos || pt2 == std::string::npos) {
        return false;
    }

    const int x = ParseInt(obj, px);
    const int y = ParseInt(obj, py);
    uint32_t tpl0 = 0;
    uint32_t tpl1 = 0;
    uint32_t tpl2 = 0;
    if (!ParseQuotedHexU32(obj, pt0, tpl0) || !ParseQuotedHexU32(obj, pt1, tpl1) || !ParseQuotedHexU32(obj, pt2, tpl2)) {
        return false;
    }

    out.x = (float)x;
    out.y = (float)y;
    out.tplAddr[0] = tpl0;
    out.tplAddr[1] = tpl1;
    out.tplAddr[2] = tpl2;
    out.valid = true;
    return true;
}

static bool ParseEv3Tpl3LayerArray2(const std::string& json, const char* key, Ev3Tpl3Layer outLayers[2]) {
    if (!outLayers) return false;
    outLayers[0] = Ev3Tpl3Layer{};
    outLayers[1] = Ev3Tpl3Layer{};

    size_t p = FindKey(json, 0, key);
    if (p == std::string::npos) return false;
    p = json.find('[', p);
    if (p == std::string::npos) return false;
    p++;

    int outCount = 0;
    while (p < json.size() && outCount < 2) {
        p = SkipWs(json, p);
        if (p >= json.size()) break;
        if (json[p] == ']') break;
        if (json[p] == ',') {
            p++;
            continue;
        }
        if (json[p] != '{') {
            p++;
            continue;
        }

        const size_t endBrace = json.find('}', p);
        if (endBrace == std::string::npos) break;
        const std::string obj = json.substr(p, endBrace - p + 1);

        size_t px = FindKey(obj, 0, "x");
        size_t py = FindKey(obj, 0, "y");
        size_t pt0 = FindKey(obj, 0, "tpl0");
        size_t pt1 = FindKey(obj, 0, "tpl1");
        size_t pt2 = FindKey(obj, 0, "tpl2");
        if (px != std::string::npos && py != std::string::npos && pt0 != std::string::npos && pt1 != std::string::npos && pt2 != std::string::npos) {
            const int x = ParseInt(obj, px);
            const int y = ParseInt(obj, py);
            uint32_t tpl0 = 0;
            uint32_t tpl1 = 0;
            uint32_t tpl2 = 0;
            if (ParseQuotedHexU32(obj, pt0, tpl0) && ParseQuotedHexU32(obj, pt1, tpl1) && ParseQuotedHexU32(obj, pt2, tpl2)) {
                outLayers[outCount].x = (float)x;
                outLayers[outCount].y = (float)y;
                outLayers[outCount].tplAddr[0] = tpl0;
                outLayers[outCount].tplAddr[1] = tpl1;
                outLayers[outCount].tplAddr[2] = tpl2;
                outLayers[outCount].valid = true;
                outCount++;
            }
        }

        p = endBrace + 1;
    }

    return outCount == 2;
}

static bool ParseEv3Tpl3LayerArray4(const std::string& json, const char* key, Ev3Tpl3Layer outLayers[4]) {
    if (!outLayers) return false;
    for (int i = 0; i < 4; i++) outLayers[i] = Ev3Tpl3Layer{};

    size_t p = FindKey(json, 0, key);
    if (p == std::string::npos) return false;
    p = json.find('[', p);
    if (p == std::string::npos) return false;
    p++;

    int outCount = 0;
    while (p < json.size() && outCount < 4) {
        p = SkipWs(json, p);
        if (p >= json.size()) break;
        if (json[p] == ']') break;
        if (json[p] == ',') {
            p++;
            continue;
        }
        if (json[p] != '{') {
            p++;
            continue;
        }

        const size_t endBrace = json.find('}', p);
        if (endBrace == std::string::npos) break;
        const std::string obj = json.substr(p, endBrace - p + 1);

        size_t px = FindKey(obj, 0, "x");
        size_t py = FindKey(obj, 0, "y");
        size_t pt0 = FindKey(obj, 0, "tpl0");
        size_t pt1 = FindKey(obj, 0, "tpl1");
        size_t pt2 = FindKey(obj, 0, "tpl2");
        if (px != std::string::npos && py != std::string::npos && pt0 != std::string::npos && pt1 != std::string::npos && pt2 != std::string::npos) {
            const int x = ParseInt(obj, px);
            const int y = ParseInt(obj, py);
            uint32_t tpl0 = 0;
            uint32_t tpl1 = 0;
            uint32_t tpl2 = 0;
            if (ParseQuotedHexU32(obj, pt0, tpl0) && ParseQuotedHexU32(obj, pt1, tpl1) && ParseQuotedHexU32(obj, pt2, tpl2)) {
                outLayers[outCount].x = (float)x;
                outLayers[outCount].y = (float)y;
                outLayers[outCount].tplAddr[0] = tpl0;
                outLayers[outCount].tplAddr[1] = tpl1;
                outLayers[outCount].tplAddr[2] = tpl2;
                outLayers[outCount].valid = true;
                outCount++;
            }
        }

        p = endBrace + 1;
    }

    return outCount == 4;
}

static bool ParseEv3TopFrames(const std::string& json, Ev3TopFrame outFrames[3]) {
    if (!outFrames) return false;
    for (int i = 0; i < 3; i++) outFrames[i] = Ev3TopFrame{};

    size_t p = FindKey(json, 0, "top_frames");
    if (p == std::string::npos) return false;
    p = json.find('[', p);
    if (p == std::string::npos) return false;
    p++;

    int outCount = 0;
    while (p < json.size() && outCount < 3) {
        p = SkipWs(json, p);
        if (p >= json.size()) break;
        if (json[p] == ']') break;
        if (json[p] == ',') {
            p++;
            continue;
        }
        if (json[p] != '{') {
            p++;
            continue;
        }
        const size_t endBrace = json.find('}', p);
        if (endBrace == std::string::npos) break;
        const std::string obj = json.substr(p, endBrace - p + 1);

        size_t px = FindKey(obj, 0, "x");
        size_t py = FindKey(obj, 0, "y");
        size_t pt0 = FindKey(obj, 0, "tpl0");
        size_t pt1 = FindKey(obj, 0, "tpl1");
        if (px != std::string::npos && py != std::string::npos && pt0 != std::string::npos && pt1 != std::string::npos) {
            const int x = ParseInt(obj, px);
            const int y = ParseInt(obj, py);
            uint32_t tpl0 = 0;
            uint32_t tpl1 = 0;
            if (ParseQuotedHexU32(obj, pt0, tpl0) && ParseQuotedHexU32(obj, pt1, tpl1)) {
                outFrames[outCount].x = (float)x;
                outFrames[outCount].y = (float)y;
                outFrames[outCount].tplAddr[0] = tpl0;
                outFrames[outCount].tplAddr[1] = tpl1;
                outFrames[outCount].valid = true;
                outCount++;
            }
        }
        p = endBrace + 1;
    }

    return outCount == 3;
}

static bool ParseEv3BottomFrames(const std::string& json, Ev3BottomFrame outFrames[4]) {
    if (!outFrames) return false;
    for (int i = 0; i < 4; i++) outFrames[i] = Ev3BottomFrame{};

    size_t p = FindKey(json, 0, "bottom_frames");
    if (p == std::string::npos) return false;
    p = json.find('[', p);
    if (p == std::string::npos) return false;
    p++;

    int outCount = 0;
    while (p < json.size() && outCount < 4) {
        p = SkipWs(json, p);
        if (p >= json.size()) break;
        if (json[p] == ']') break;
        if (json[p] == ',') {
            p++;
            continue;
        }
        if (json[p] != '{') {
            p++;
            continue;
        }
        const size_t endBrace = json.find('}', p);
        if (endBrace == std::string::npos) break;
        const std::string obj = json.substr(p, endBrace - p + 1);

        size_t px = FindKey(obj, 0, "x");
        size_t py = FindKey(obj, 0, "y");
        size_t pt0 = FindKey(obj, 0, "tpl0");
        size_t pt1 = FindKey(obj, 0, "tpl1");
        if (px != std::string::npos && py != std::string::npos && pt0 != std::string::npos && pt1 != std::string::npos) {
            const int x = ParseInt(obj, px);
            const int y = ParseInt(obj, py);
            uint32_t tpl0 = 0;
            uint32_t tpl1 = 0;
            if (ParseQuotedHexU32(obj, pt0, tpl0) && ParseQuotedHexU32(obj, pt1, tpl1)) {
                outFrames[outCount].x = (float)x;
                outFrames[outCount].y = (float)y;
                outFrames[outCount].tplAddr[0] = tpl0;
                outFrames[outCount].tplAddr[1] = tpl1;
                outFrames[outCount].valid = true;
                outCount++;
            }
        }
        p = endBrace + 1;
    }

    return outCount == 4;
}

struct Ev3MenuJsonCache {
    bool loaded = false;
    bool ok = false;
    std::filesystem::path loadedRoot;
    Ev3PosTable tbl_LANGUAGE;
    Ev3PosTable tbl_HI_SCORE;
    Ev3PosTable tbl_NORMAL;
    Ev3PosTable tbl_EASY;
    Ev3PosTable tbl_PRACTICE;
    Ev3PosTable tbl_STAGE_SELECT;
    Ev3PosTable tbl_REPLAY;
    Ev3PosTable tbl_LOAD;
    Ev3PosTable tbl_EXIT;
    Ev3Tpl3Layer btn0Bg;
    Ev3Tpl3Layer btn1Hint;
    Ev3Tpl3Layer btn2Deco[2];
    Ev3Tpl3Layer btn3Icons[4];
    Ev3FixedSprite fixedSprites[4];
    Ev3TopFrame topFrames[3];
    Ev3Pos2 exitBg;
    Ev3Tpl2Frame exitFrame;
    Ev3BottomFrame bottomFrames[4];
};

static const Ev3MenuJsonCache& GetEv3MenuJsonCache(const std::filesystem::path& dataRoot) {
    static Ev3MenuJsonCache s_cache;
    if (!s_cache.loaded || s_cache.loadedRoot != dataRoot) {
        s_cache = Ev3MenuJsonCache{};
        s_cache.loaded = true;
        s_cache.loadedRoot = dataRoot;

        const std::filesystem::path p = dataRoot / "win" / "ex" / "json" / "psx_menu_ev3_constants.json";
        const std::string json = ReadFileToString(p);
        if (json.empty()) {
            Log::Printf("Ev3MenuJson: missing or empty: %s", p.string().c_str());
            return s_cache;
        }

        bool okTables = true;
        okTables &= ParseEv3PosTable(json, "LANGUAGE", s_cache.tbl_LANGUAGE);
        okTables &= ParseEv3PosTable(json, "HI_SCORE", s_cache.tbl_HI_SCORE);
        okTables &= ParseEv3PosTable(json, "NORMAL", s_cache.tbl_NORMAL);
        okTables &= ParseEv3PosTable(json, "EASY", s_cache.tbl_EASY);
        okTables &= ParseEv3PosTable(json, "PRACTICE", s_cache.tbl_PRACTICE);
        okTables &= ParseEv3PosTable(json, "STAGE_SELECT", s_cache.tbl_STAGE_SELECT);
        okTables &= ParseEv3PosTable(json, "REPLAY", s_cache.tbl_REPLAY);
        okTables &= ParseEv3PosTable(json, "LOAD", s_cache.tbl_LOAD);
        okTables &= ParseEv3PosTable(json, "EXIT", s_cache.tbl_EXIT);

        const bool okFixed = ParseEv3FixedSprites(json, s_cache.fixedSprites);
        ParseEv3Tpl3LayerObj(json, "btn0_bg", s_cache.btn0Bg);
        ParseEv3Tpl3LayerObj(json, "btn1_hint", s_cache.btn1Hint);
        ParseEv3Tpl3LayerArray2(json, "btn2_deco", s_cache.btn2Deco);
        ParseEv3Tpl3LayerArray4(json, "btn3_icons", s_cache.btn3Icons);
        ParseEv3TopFrames(json, s_cache.topFrames);
        ParseEv3ExitBg(json, s_cache.exitBg);
        ParseEv3ExitFrame(json, s_cache.exitFrame);
        ParseEv3BottomFrames(json, s_cache.bottomFrames);
        s_cache.ok = okTables && okFixed;

        for (int i = 0; i < 4; i++) {
            s_cache.fixedSprites[i].tplValid = false;
            if (!s_cache.fixedSprites[i].valid) continue;
            const uint32_t addr = s_cache.fixedSprites[i].tplAddr;
            if (addr == 0) continue;
            PsxSpriteTemplate tpl = {};
            if (ParseSpriteTemplateByAddr(json, addr, tpl)) {
                s_cache.fixedSprites[i].tpl = tpl;
                s_cache.fixedSprites[i].tplValid = true;
            }
        }

        {
            Ev3Tpl3Layer* layers[] = {
                &s_cache.btn0Bg,
                &s_cache.btn1Hint,
                &s_cache.btn2Deco[0],
                &s_cache.btn2Deco[1],
                &s_cache.btn3Icons[0],
                &s_cache.btn3Icons[1],
                &s_cache.btn3Icons[2],
                &s_cache.btn3Icons[3],
            };
            for (Ev3Tpl3Layer* l : layers) {
                if (!l || !l->valid) continue;
                for (int st = 0; st < 3; st++) {
                    l->tplValid[st] = false;
                    const uint32_t addr = l->tplAddr[st];
                    if (addr == 0) continue;
                    PsxSpriteTemplate tpl = {};
                    if (ParseSpriteTemplateByAddr(json, addr, tpl)) {
                        l->tpl[st] = tpl;
                        l->tplValid[st] = true;
                    }
                }
            }
        }

        for (int i = 0; i < 3; i++) {
            for (int st = 0; st < 2; st++) {
                s_cache.topFrames[i].tplValid[st] = false;
            }
            if (!s_cache.topFrames[i].valid) continue;
            for (int st = 0; st < 2; st++) {
                const uint32_t addr = s_cache.topFrames[i].tplAddr[st];
                if (addr == 0) continue;
                PsxSpriteTemplate tpl = {};
                if (ParseSpriteTemplateByAddr(json, addr, tpl)) {
                    s_cache.topFrames[i].tpl[st] = tpl;
                    s_cache.topFrames[i].tplValid[st] = true;
                }
            }
        }

        for (int i = 0; i < 4; i++) {
            for (int st = 0; st < 2; st++) {
                s_cache.bottomFrames[i].tplValid[st] = false;
            }
            if (!s_cache.bottomFrames[i].valid) continue;
            for (int st = 0; st < 2; st++) {
                const uint32_t addr = s_cache.bottomFrames[i].tplAddr[st];
                if (addr == 0) continue;
                PsxSpriteTemplate tpl = {};
                if (ParseSpriteTemplateByAddr(json, addr, tpl)) {
                    s_cache.bottomFrames[i].tpl[st] = tpl;
                    s_cache.bottomFrames[i].tplValid[st] = true;
                }
            }
        }

        for (int st = 0; st < 2; st++) s_cache.exitFrame.tplValid[st] = false;
        if (s_cache.exitFrame.valid) {
            for (int st = 0; st < 2; st++) {
                const uint32_t addr = s_cache.exitFrame.tplAddr[st];
                if (addr == 0) continue;
                PsxSpriteTemplate tpl = {};
                if (ParseSpriteTemplateByAddr(json, addr, tpl)) {
                    s_cache.exitFrame.tpl[st] = tpl;
                    s_cache.exitFrame.tplValid[st] = true;
                }
            }
        }

        ResolveEv3PosTableTemplates(json, s_cache.tbl_LANGUAGE);
        ResolveEv3PosTableTemplates(json, s_cache.tbl_HI_SCORE);
        ResolveEv3PosTableTemplates(json, s_cache.tbl_NORMAL);
        ResolveEv3PosTableTemplates(json, s_cache.tbl_EASY);
        ResolveEv3PosTableTemplates(json, s_cache.tbl_PRACTICE);
        ResolveEv3PosTableTemplates(json, s_cache.tbl_STAGE_SELECT);
        ResolveEv3PosTableTemplates(json, s_cache.tbl_REPLAY);
        ResolveEv3PosTableTemplates(json, s_cache.tbl_LOAD);
        ResolveEv3PosTableTemplates(json, s_cache.tbl_EXIT);

        Log::Printf("Ev3MenuJson: loaded=%d ok=%d", 1, s_cache.ok ? 1 : 0);
    }
    return s_cache;
}

static bool TryGetEv3ExitText(int langIndex, int state, const std::filesystem::path& dataRoot,
                              float& outX, float& outY, PsxSpriteTemplate& outTpl) {
    const Ev3MenuJsonCache& ev3 = GetEv3MenuJsonCache(dataRoot);
    if (!ev3.tbl_EXIT.valid) return false;
    int li = langIndex;
    if (li < 0) li = 0;
    if (li > 4) li = 4;
    int st = state;
    if (st < 1) st = 1;
    if (st > 3) st = 3;
    if (!ev3.tbl_EXIT.tplValid[li][st - 1]) return false;
    outX = ev3.tbl_EXIT.x[li];
    outY = ev3.tbl_EXIT.y[li];
    outTpl = ev3.tbl_EXIT.tpl[li][st - 1];
    return true;
}

static bool TryGetEv3PosTableEntry(const Ev3PosTable& table, int index, int state,
                                   float& outX, float& outY, PsxSpriteTemplate& outTpl) {
    if (!table.valid) return false;
    int idx = index;
    if (idx < 0) idx = 0;
    if (idx > 4) idx = 4;
    int st = state;
    if (st < 1) st = 1;
    if (st > 3) st = 3;
    if (!table.tplValid[idx][st - 1]) return false;
    outX = table.x[idx];
    outY = table.y[idx];
    outTpl = table.tpl[idx][st - 1];
    return true;
}

static const PracticeUiFixedTemplate kLanguageTitleBg = {
    true, 0x80050AE0, {0x50000040, 0x01E2, 0x0000, 0x0048, 0x0031, 0x03D0, 0x0000}};
static const PracticeUiFixedTemplate kLanguageRowLeft[2] = {
    {true, 0x80050D30, {0x50000040, 0x01C0, 0x0000, 0x0088, 0x0052, 0x03C0, 0x0000}},
    {true, 0x80050D40, {0x50000040, 0x01C0, 0x0000, 0x0088, 0x0052, 0x03C0, 0x0001}},
};
static const PracticeUiFixedTemplate kLanguageRowRight[2] = {
    {true, 0x80050E10, {0x50000040, 0x01C0, 0x0052, 0x0088, 0x0052, 0x03C0, 0x0000}},
    {true, 0x80050E20, {0x50000040, 0x01C0, 0x0052, 0x0088, 0x0052, 0x03C0, 0x0001}},
};
static const PracticeUiFixedTemplate kLanguageExitBar[3] = {
    {true, 0x800509A0, {0x50000040, 0x0140, 0x0000, 0x0030, 0x0011, 0x03C0, 0x0002}},
    {true, 0x800509B0, {0x50000040, 0x0140, 0x0000, 0x0030, 0x0011, 0x03C0, 0x0003}},
    {true, 0x800509C0, {0x50000040, 0x0140, 0x0000, 0x0030, 0x0011, 0x03C0, 0x0004}},
};
static const float kLanguageSubtitleOnX[5] = {220.0f, 219.0f, 218.0f, 218.0f, 220.0f};
static const float kLanguageSubtitleOnY[5] = {49.0f, 50.0f, 50.0f, 50.0f, 48.0f};
static const float kLanguageSubtitleOffX[5] = {220.0f, 219.0f, 218.0f, 218.0f, 220.0f};
static const float kLanguageSubtitleOffY[5] = {72.0f, 73.0f, 73.0f, 73.0f, 71.0f};
static const float kLanguageTextX[5] = {42.0f, 89.0f, 138.0f, 187.0f, 235.0f};
static const float kLanguageTextY[5] = {127.0f, 146.0f, 153.0f, 150.0f, 127.0f};
static const float kLanguageBgX[5] = {33.0f, 81.0f, 134.0f, 183.0f, 228.0f};
static const float kLanguageBgY[5] = {115.0f, 133.0f, 142.0f, 133.0f, 115.0f};

struct StageSelectJsonCache {
    bool loaded = false;
    bool ok = false;
    std::filesystem::path loadedRoot;
    PracticeUiLangTable1 titleText;
    PracticeUiFixedTemplate titleBg;
    PracticeUiFixedTemplate stageDotTpl;
    uint32_t stageBoxTplAddr[5][3] = {};
    PsxSpriteTemplate stageBoxTpl[5][3] = {};
    bool stageBoxTplValid[5][3] = {};
    float stageBoxX[5][6] = {};
    float stageBoxY[5][6] = {};
    float stageDotX[5][6] = {};
    float stageDotY[5][6] = {};
    uint16_t stageDotArgA[6] = {};
    uint16_t stageDotArgB[6] = {};
    float stageTopX[6] = {};
    float stageTopY[6] = {};
    float stageNameX[6] = {};
    float stageNameY[6] = {};
    float stageBadgeX[6] = {};
    float stageBadgeY[6] = {};
    uint32_t stageTopTplAddr[3] = {};
    PsxSpriteTemplate stageTopTpl[3] = {};
    bool stageTopTplValid[3] = {};
    uint32_t stageNameTplAddr[6][2] = {};
    PsxSpriteTemplate stageNameTpl[6][2] = {};
    bool stageNameTplValid[6][2] = {};
    uint32_t stageBadgeATplAddr[8] = {};
    PsxSpriteTemplate stageBadgeATpl[8] = {};
    bool stageBadgeATplValid[8] = {};
    uint32_t stageBadgeBTplAddr[3] = {};
    PsxSpriteTemplate stageBadgeBTpl[3] = {};
    bool stageBadgeBTplValid[3] = {};
    PracticeUiFixedTemplate bonusBaseOff;
    PracticeUiFixedTemplate bonusBaseOn;
    PracticeUiFixedTemplate bonusBaseEnabled;
    PracticeUiFixedTemplate bonusLabel;
    PracticeUiFixedTemplate exitFrameOff;
    PracticeUiFixedTemplate exitFrameOn;
};

static size_t FindNamedJsonEntries(const std::string& json, const char* tableName) {
    const std::string needle = std::string("\"name\": \"") + tableName + "\"";
    const size_t pos = json.find(needle);
    if (pos == std::string::npos) return std::string::npos;
    size_t p = FindKey(json, pos, "entries");
    if (p == std::string::npos) return std::string::npos;
    p = json.find('[', p);
    if (p == std::string::npos) return std::string::npos;
    return p + 1;
}

static bool ParseStageSelectLangTpl3(const std::string& json, const char* tableName,
                                     uint32_t outAddr[5][3],
                                     PsxSpriteTemplate outTpl[5][3],
                                     bool outValid[5][3]) {
    for (int lang = 0; lang < 5; lang++) {
        for (int state = 0; state < 3; state++) {
            outAddr[lang][state] = 0;
            outTpl[lang][state] = {};
            outValid[lang][state] = false;
        }
    }

    size_t p = FindNamedJsonEntries(json, tableName);
    if (p == std::string::npos) return false;

    bool seen[5][3] = {};
    int seenCount = 0;
    while (p < json.size()) {
        p = SkipWs(json, p);
        if (p >= json.size() || json[p] == ']') break;
        if (json[p] == ',') {
            p++;
            continue;
        }
        if (json[p] != '{') {
            p++;
            continue;
        }

        const size_t endBrace = json.find('}', p);
        if (endBrace == std::string::npos) break;
        const std::string obj = json.substr(p, endBrace - p + 1);

        size_t pl = FindKey(obj, 0, "lang");
        size_t ps = FindKey(obj, 0, "state");
        size_t pt = FindKey(obj, 0, "tpl");
        if (pl != std::string::npos && ps != std::string::npos && pt != std::string::npos) {
            const int lang = ParseInt(obj, pl);
            const int state = ParseInt(obj, ps);
            uint32_t addr = 0;
            if (lang >= 0 && lang < 5 && state >= 0 && state < 3 && ParseQuotedHexU32(obj, pt, addr)) {
                if (!seen[lang][state]) {
                    seen[lang][state] = true;
                    seenCount++;
                }
                outAddr[lang][state] = addr;
            }
        }

        p = endBrace + 1;
    }

    if (seenCount != 15) return false;
    bool ok = true;
    for (int lang = 0; lang < 5; lang++) {
        for (int state = 0; state < 3; state++) {
            outValid[lang][state] = ParseSpriteTemplateByAddr(json, outAddr[lang][state], outTpl[lang][state]);
            ok &= outValid[lang][state];
        }
    }
    return ok;
}

static bool ParseStageSelectLangPos6(const std::string& json, const char* tableName,
                                     float outX[5][6], float outY[5][6]) {
    for (int lang = 0; lang < 5; lang++) {
        for (int index = 0; index < 6; index++) {
            outX[lang][index] = 0.0f;
            outY[lang][index] = 0.0f;
        }
    }

    size_t p = FindNamedJsonEntries(json, tableName);
    if (p == std::string::npos) return false;

    bool seen[5][6] = {};
    int seenCount = 0;
    while (p < json.size()) {
        p = SkipWs(json, p);
        if (p >= json.size() || json[p] == ']') break;
        if (json[p] == ',') {
            p++;
            continue;
        }
        if (json[p] != '{') {
            p++;
            continue;
        }

        const size_t endBrace = json.find('}', p);
        if (endBrace == std::string::npos) break;
        const std::string obj = json.substr(p, endBrace - p + 1);

        size_t pl = FindKey(obj, 0, "lang");
        size_t pi = FindKey(obj, 0, "index");
        size_t px = FindKey(obj, 0, "x");
        size_t py = FindKey(obj, 0, "y");
        if (pl != std::string::npos && pi != std::string::npos &&
            px != std::string::npos && py != std::string::npos) {
            const int lang = ParseInt(obj, pl);
            const int index = ParseInt(obj, pi);
            if (lang >= 0 && lang < 5 && index >= 0 && index < 6) {
                if (!seen[lang][index]) {
                    seen[lang][index] = true;
                    seenCount++;
                }
                outX[lang][index] = (float)ParseInt(obj, px);
                outY[lang][index] = (float)ParseInt(obj, py);
            }
        }

        p = endBrace + 1;
    }

    return seenCount == 30;
}

static bool ParseStageSelectPos6(const std::string& json, const char* tableName, float outX[6], float outY[6]) {
    for (int index = 0; index < 6; index++) {
        outX[index] = 0.0f;
        outY[index] = 0.0f;
    }

    size_t p = FindNamedJsonEntries(json, tableName);
    if (p == std::string::npos) return false;

    bool seen[6] = {};
    int seenCount = 0;
    while (p < json.size()) {
        p = SkipWs(json, p);
        if (p >= json.size() || json[p] == ']') break;
        if (json[p] == ',') {
            p++;
            continue;
        }
        if (json[p] != '{') {
            p++;
            continue;
        }

        const size_t endBrace = json.find('}', p);
        if (endBrace == std::string::npos) break;
        const std::string obj = json.substr(p, endBrace - p + 1);

        size_t pi = FindKey(obj, 0, "index");
        size_t px = FindKey(obj, 0, "x");
        size_t py = FindKey(obj, 0, "y");
        if (pi != std::string::npos && px != std::string::npos && py != std::string::npos) {
            const int index = ParseInt(obj, pi);
            if (index >= 0 && index < 6) {
                if (!seen[index]) {
                    seen[index] = true;
                    seenCount++;
                }
                outX[index] = (float)ParseInt(obj, px);
                outY[index] = (float)ParseInt(obj, py);
            }
        }

        p = endBrace + 1;
    }

    return seenCount == 6;
}

static bool ParseStageSelectU16Pairs6(const std::string& json, const char* tableName,
                                      uint16_t outA[6], uint16_t outB[6]) {
    for (int index = 0; index < 6; index++) {
        outA[index] = 0;
        outB[index] = 0;
    }

    size_t p = FindNamedJsonEntries(json, tableName);
    if (p == std::string::npos) return false;

    bool seen[6] = {};
    int seenCount = 0;
    while (p < json.size()) {
        p = SkipWs(json, p);
        if (p >= json.size() || json[p] == ']') break;
        if (json[p] == ',') {
            p++;
            continue;
        }
        if (json[p] != '{') {
            p++;
            continue;
        }

        const size_t endBrace = json.find('}', p);
        if (endBrace == std::string::npos) break;
        const std::string obj = json.substr(p, endBrace - p + 1);

        size_t pi = FindKey(obj, 0, "index");
        size_t pa = FindKey(obj, 0, "a");
        size_t pb = FindKey(obj, 0, "b");
        if (pi != std::string::npos && pa != std::string::npos && pb != std::string::npos) {
            const int index = ParseInt(obj, pi);
            const int a = ParseInt(obj, pa);
            const int b = ParseInt(obj, pb);
            if (index >= 0 && index < 6 && a >= 0 && a <= 0xFFFF && b >= 0 && b <= 0xFFFF) {
                if (!seen[index]) {
                    seen[index] = true;
                    seenCount++;
                }
                outA[index] = (uint16_t)a;
                outB[index] = (uint16_t)b;
            }
        }

        p = endBrace + 1;
    }

    return seenCount == 6;
}

static bool ParseStageSelectTplArray(const std::string& json, const char* tableName, int count,
                                     uint32_t* outAddr, PsxSpriteTemplate* outTpl, bool* outValid) {
    if (!outAddr || !outTpl || !outValid || count <= 0) return false;
    for (int index = 0; index < count; index++) {
        outAddr[index] = 0;
        outTpl[index] = {};
        outValid[index] = false;
    }

    size_t p = FindNamedJsonEntries(json, tableName);
    if (p == std::string::npos) return false;

    std::vector<bool> seen((size_t)count, false);
    int seenCount = 0;
    while (p < json.size()) {
        p = SkipWs(json, p);
        if (p >= json.size() || json[p] == ']') break;
        if (json[p] == ',') {
            p++;
            continue;
        }
        if (json[p] != '{') {
            p++;
            continue;
        }

        const size_t endBrace = json.find('}', p);
        if (endBrace == std::string::npos) break;
        const std::string obj = json.substr(p, endBrace - p + 1);

        size_t pi = FindKey(obj, 0, "index");
        size_t pt = FindKey(obj, 0, "tpl");
        if (pi != std::string::npos && pt != std::string::npos) {
            const int index = ParseInt(obj, pi);
            uint32_t addr = 0;
            if (index >= 0 && index < count && ParseQuotedHexU32(obj, pt, addr)) {
                if (!seen[(size_t)index]) {
                    seen[(size_t)index] = true;
                    seenCount++;
                }
                outAddr[index] = addr;
            }
        }

        p = endBrace + 1;
    }

    if (seenCount != count) return false;
    bool ok = true;
    for (int index = 0; index < count; index++) {
        outValid[index] = ParseSpriteTemplateByAddr(json, outAddr[index], outTpl[index]);
        ok &= outValid[index];
    }
    return ok;
}

static bool ParseStageSelectStageNameTpl(const std::string& json,
                                         uint32_t outAddr[6][2],
                                         PsxSpriteTemplate outTpl[6][2],
                                         bool outValid[6][2]) {
    for (int stage = 0; stage < 6; stage++) {
        for (int state = 0; state < 2; state++) {
            outAddr[stage][state] = 0;
            outTpl[stage][state] = {};
            outValid[stage][state] = false;
        }
    }

    size_t p = FindNamedJsonEntries(json, "STAGE_NAME_TPL");
    if (p == std::string::npos) return false;

    bool seen[6][2] = {};
    int seenCount = 0;
    while (p < json.size()) {
        p = SkipWs(json, p);
        if (p >= json.size() || json[p] == ']') break;
        if (json[p] == ',') {
            p++;
            continue;
        }
        if (json[p] != '{') {
            p++;
            continue;
        }

        const size_t endBrace = json.find('}', p);
        if (endBrace == std::string::npos) break;
        const std::string obj = json.substr(p, endBrace - p + 1);

        size_t pst = FindKey(obj, 0, "stage");
        size_t ps = FindKey(obj, 0, "state");
        size_t pt = FindKey(obj, 0, "tpl");
        if (pst != std::string::npos && ps != std::string::npos && pt != std::string::npos) {
            const int stage = ParseInt(obj, pst);
            const int state = ParseInt(obj, ps);
            uint32_t addr = 0;
            if (stage >= 0 && stage < 6 && state >= 0 && state < 2 && ParseQuotedHexU32(obj, pt, addr)) {
                if (!seen[stage][state]) {
                    seen[stage][state] = true;
                    seenCount++;
                }
                outAddr[stage][state] = addr;
            }
        }

        p = endBrace + 1;
    }

    if (seenCount != 12) return false;
    bool ok = true;
    for (int stage = 0; stage < 6; stage++) {
        for (int state = 0; state < 2; state++) {
            outValid[stage][state] = ParseSpriteTemplateByAddr(json, outAddr[stage][state], outTpl[stage][state]);
            ok &= outValid[stage][state];
        }
    }
    return ok;
}

static bool AreStageSelectFixedTemplatesValid(const StageSelectJsonCache& cache) {
    const PracticeUiFixedTemplate* fixed[] = {
        &cache.titleBg,
        &cache.stageDotTpl,
        &cache.bonusBaseOff,
        &cache.bonusBaseOn,
        &cache.bonusBaseEnabled,
        &cache.bonusLabel,
        &cache.exitFrameOff,
        &cache.exitFrameOn,
    };
    for (const PracticeUiFixedTemplate* tpl : fixed) {
        if (!tpl || !tpl->valid) return false;
    }
    return true;
}

static const StageSelectJsonCache& GetStageSelectJsonCache(const std::filesystem::path& dataRoot) {
    static StageSelectJsonCache s_cache;
    if (!s_cache.loaded || s_cache.loadedRoot != dataRoot) {
        s_cache = StageSelectJsonCache{};
        s_cache.loaded = true;
        s_cache.loadedRoot = dataRoot;

        const std::filesystem::path p = dataRoot / "win" / "ex" / "json" / "psx_stage_select_constants.json";
        const std::string json = ReadFileToString(p);
        if (json.empty()) {
            Log::Printf("StageSelectJson: missing or empty: %s", p.string().c_str());
            return s_cache;
        }

        bool ok = true;
        ok &= ParsePracticeUiLangTable1(json, "TITLE_TEXT", s_cache.titleText);
        ok &= ParsePracticeUiFixed(json, "titleBg", s_cache.titleBg);
        ok &= ParsePracticeUiFixed(json, "stageDotTpl", s_cache.stageDotTpl);
        ok &= ParseStageSelectLangTpl3(json, "STAGE_BOX_TPL",
                                       s_cache.stageBoxTplAddr, s_cache.stageBoxTpl, s_cache.stageBoxTplValid);
        ok &= ParseStageSelectLangPos6(json, "STAGE_BOX_POS", s_cache.stageBoxX, s_cache.stageBoxY);
        ok &= ParseStageSelectLangPos6(json, "STAGE_DOT_POS", s_cache.stageDotX, s_cache.stageDotY);
        ok &= ParseStageSelectU16Pairs6(json, "STAGE_DOT_ARGS", s_cache.stageDotArgA, s_cache.stageDotArgB);
        ok &= ParseStageSelectPos6(json, "STAGE_TOP_POS", s_cache.stageTopX, s_cache.stageTopY);
        ok &= ParseStageSelectPos6(json, "STAGE_NAME_POS", s_cache.stageNameX, s_cache.stageNameY);
        ok &= ParseStageSelectPos6(json, "STAGE_BADGE_POS", s_cache.stageBadgeX, s_cache.stageBadgeY);
        ok &= ParseStageSelectTplArray(json, "STAGE_TOP_TPL", 3,
                                       s_cache.stageTopTplAddr, s_cache.stageTopTpl, s_cache.stageTopTplValid);
        ok &= ParseStageSelectTplArray(json, "STAGE_BADGE_A_TPL", 8,
                                       s_cache.stageBadgeATplAddr, s_cache.stageBadgeATpl, s_cache.stageBadgeATplValid);
        ok &= ParseStageSelectTplArray(json, "STAGE_BADGE_B_TPL", 3,
                                       s_cache.stageBadgeBTplAddr, s_cache.stageBadgeBTpl, s_cache.stageBadgeBTplValid);
        ok &= ParseStageSelectStageNameTpl(json, s_cache.stageNameTplAddr,
                                           s_cache.stageNameTpl, s_cache.stageNameTplValid);
        ok &= ParsePracticeUiFixed(json, "bonusBaseOff", s_cache.bonusBaseOff);
        ok &= ParsePracticeUiFixed(json, "bonusBaseOn", s_cache.bonusBaseOn);
        ok &= ParsePracticeUiFixed(json, "bonusBaseEnabled", s_cache.bonusBaseEnabled);
        ok &= ParsePracticeUiFixed(json, "bonusLabel", s_cache.bonusLabel);
        ok &= ParsePracticeUiFixed(json, "exitFrameOff", s_cache.exitFrameOff);
        ok &= ParsePracticeUiFixed(json, "exitFrameOn", s_cache.exitFrameOn);

        ok &= AreStageSelectFixedTemplatesValid(s_cache);
        s_cache.ok = ok;
        Log::Printf("StageSelectJson: loaded=%d ok=%d", 1, ok ? 1 : 0);
    }
    return s_cache;
}

static PsxSpriteTemplate PsxTplWithState(const PsxSpriteTemplate& base, int state) {
    if (state < 1) state = 1;
    if (state > 3) state = 3;
    PsxSpriteTemplate t = base;
    t.clutY_px = (uint16_t)(t.clutY_px + (uint16_t)(state - 1));
    return t;
}

static bool DrawMenuCursorSprite(PrGameContext& ctx, float vx, float vy, float vs,
                                 float x, float y, float w, float h,
                                 float t, float r, float g, float b, float a) {
    if (!ctx.resources) return false;
    if (!ctx.resources->GetTextureView("ji_lu")) return false;
    if (!ctx.resources->GetTextureView("ji_ld")) return false;
    if (!ctx.resources->GetTextureView("ji_ru")) return false;
    if (!ctx.resources->GetTextureView("ji_rd")) return false;

    const float scale = 1.35f + 0.25f * t;
    const float alpha = a * (0.65f + 0.35f * t);

    const float s = 8.0f * scale;

    const float inset = 1.0f;
    const float x0 = x + inset;
    const float y0 = y + inset;
    const float x1 = x + w - inset - s;
    const float y1 = y + h - inset - s;

    SubmitSpriteUI(ctx, vx, vy, vs, "ji_lu", x0, y0, s, s, r, g, b, alpha, 720);
    SubmitSpriteUI(ctx, vx, vy, vs, "ji_ld", x0, y1, s, s, r, g, b, alpha, 720);
    SubmitSpriteUI(ctx, vx, vy, vs, "ji_ru", x1, y0, s, s, r, g, b, alpha, 720);
    SubmitSpriteUI(ctx, vx, vy, vs, "ji_rd", x1, y1, s, s, r, g, b, alpha, 720);
    return true;
}

static void DrawMenuCursor(D3D11Renderer* r, float vx, float vy, float vs,
                           float x, float y,
                           int frame, float cr, float cg, float cb, float ca) {
    if (!r) return;
    const int f = frame & 3;
    float block = 8.0f;
    float ox = 0.0f;
    float oy = 0.0f;
    float lineW = 8.0f;
    switch (f) {
        case 0: block = 8.0f;  ox = 0.0f; oy = 0.0f; lineW = 8.0f;  break;
        case 1: block = 10.0f; ox = 1.0f; oy = 1.0f; lineW = 10.0f; break;
        case 2: block = 14.0f; ox = 3.0f; oy = 3.0f; lineW = 14.0f; break;
        case 3: block = 10.0f; ox = 1.0f; oy = 1.0f; lineW = 10.0f; break;
    }

    // 小方块 + 指向线（两帧尺寸/偏移变化，做大差异便于截图验证）
    DrawRectUI(r, vx, vy, vs, x + ox, y + oy, block, block, cr, cg, cb, ca);
    DrawRectUI(r, vx, vy, vs, x + ox + block, y + oy + block * 0.5f - 1.5f, lineW, 3.0f, cr, cg, cb, ca);
}

static float UiTimeSeconds(const PrGameContext& ctx) {
    return (float)ctx.frame / 30.0f;
}

static bool BlinkOnOff(float tSeconds, float hz) {
    if (hz <= 0.0f) return true;
    const float phase = std::fmod(tSeconds * hz, 1.0f);
    return phase < 0.5f;
}

static float Lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

static float BlinkFade(float tSeconds, float hz) {
    if (hz <= 0.0f) return 1.0f;
    const float twoPi = 6.283185307179586f;
    const float s = std::sin(tSeconds * hz * twoPi);
    return 0.5f + 0.5f * s;
}

void Init(D3D11Renderer* renderer) {
    s_renderer = renderer;
}

void Shutdown() {
    ClearSystemTextCache();
    s_renderer = nullptr;
}

} // namespace PrUiOverlay

namespace PrUiOverlay {

bool SubmitSystemTextBlockW(PrGameContext& ctx,
                            float vx, float vy, float vs,
                            const char* cacheKey,
                            float x, float y, float baseW,
                            const wchar_t* line1, const wchar_t* line2,
                            float r, float g, float b, float a,
                            int layer) {
    if (!ctx.renderer || !cacheKey) return false;
    SystemTextCacheEntry* e = GetOrCreateSystemTextEntry(cacheKey);
    if (!e) return false;
    if (!EnsureSystemTextTexture(*e, ctx.renderer, vs, baseW, line1, line2)) return false;
    if (!e->srv || e->w <= 0 || e->h <= 0) return false;

    D3D11Renderer::SpriteCmd cmd;
    cmd.texture = e->srv;
    cmd.x = vx + x * vs;
    cmd.y = vy + y * vs;
    cmd.w = (float)e->w;
    cmd.h = (float)e->h;
    cmd.u0 = 0.0f;
    cmd.v0 = 0.0f;
    cmd.u1 = 1.0f;
    cmd.v1 = 1.0f;
    cmd.r = r;
    cmd.g = g;
    cmd.b = b;
    cmd.a = a;
    cmd.blend = D3D11Renderer::BlendMode::Alpha;
    cmd.layer = layer;
    cmd.order = 0;
    ctx.renderer->SubmitSprite(cmd);
    return true;
}

void ClearSystemTextCache() {
    for (auto& e : s_systemTextCache) {
        DestroySystemTextEntry(e);
    }
    s_systemTextCache.clear();
}

} // namespace PrUiOverlay

namespace PrUiOverlay {

// 菜单项标签配置（根据事件类型）
struct MenuLabelConfig {
    int eventId;
    int menuCount;
    const char* labels[9];
};

static const MenuLabelConfig kMenuLabels[] = {
    // ev=3: 主菜单 (5项)
    { 3, 5, { "OPTION", "PLAY!", "RECORDS", "STAGE", "EXIT" } },
    // ev=2: 选关菜单 (9项: 8关+取消)
    { 2, 9, { "---", "STAGE1", "STAGE2", "STAGE3", "STAGE5", "STAGE6", "STAGE7", "BONUS", "CANCEL" } },
    // ev=5: Practice 选择 (1项 stub)
    { 5, 1, { "PRACTICE" } },
    // ev=6: 確認对话框 (2项)
    { 6, 2, { "YES", "NO" } },
    // ev=11: SAVE 入口确认 (`sub_80019148` 初始二选一)
    { 11, 2, { "YES", "NO" } },
    // ev=16: Practice Run (2项)
    { 16, 2, { "LISTEN", "EXIT" } },
    // ev=17: 设置菜单 (3项)
    { 17, 3, { "SUBTITLE", "LANGUAGE", "OK" } },
    // ev=4: 暂停菜单 (2项)
    { 4, 2, { "CONTINUE", "EXIT" } },
};

static const char* GetMenuLabel(int eventId, int index) {
    for (const auto& cfg : kMenuLabels) {
        if (cfg.eventId == eventId && index < cfg.menuCount) {
            return cfg.labels[index];
        }
    }
    return nullptr;
}

// 简易字符绘制（使用矩形块组合）
static void DrawChar(D3D11Renderer* r, float x, float y, float s, char c, float cr, float cg, float cb, float ca) {
    if (!r) return;
    const float t = 2.0f * s;
    const float w = 6.0f * s;
    const float h = 8.0f * s;
    
    // 简化的5x7点阵字符（只实现常用字母）
    switch (c) {
        case 'A': r->DrawRect(x+t, y, w-2*t, t, cr, cg, cb, ca);
                  r->DrawRect(x, y+t, t, h-t, cr, cg, cb, ca);
                  r->DrawRect(x+w-t, y+t, t, h-t, cr, cg, cb, ca);
                  r->DrawRect(x+t, y+h/2, w-2*t, t, cr, cg, cb, ca); break;
        case 'B': r->DrawRect(x, y, t, h, cr, cg, cb, ca);
                  r->DrawRect(x+t, y, w-2*t, t, cr, cg, cb, ca);
                  r->DrawRect(x+t, y+h/2-t/2, w-2*t, t, cr, cg, cb, ca);
                  r->DrawRect(x+t, y+h-t, w-2*t, t, cr, cg, cb, ca);
                  r->DrawRect(x+w-t, y+t, t, h/2-t, cr, cg, cb, ca);
                  r->DrawRect(x+w-t, y+h/2+t/2, t, h/2-t, cr, cg, cb, ca); break;
        case 'C': r->DrawRect(x+t, y, w-t, t, cr, cg, cb, ca);
                  r->DrawRect(x, y+t, t, h-2*t, cr, cg, cb, ca);
                  r->DrawRect(x+t, y+h-t, w-t, t, cr, cg, cb, ca); break;
        case 'D': r->DrawRect(x, y, t, h, cr, cg, cb, ca);
                  r->DrawRect(x+t, y, w-2*t, t, cr, cg, cb, ca);
                  r->DrawRect(x+t, y+h-t, w-2*t, t, cr, cg, cb, ca);
                  r->DrawRect(x+w-t, y+t, t, h-2*t, cr, cg, cb, ca); break;
        case 'E': r->DrawRect(x, y, t, h, cr, cg, cb, ca);
                  r->DrawRect(x+t, y, w-t, t, cr, cg, cb, ca);
                  r->DrawRect(x+t, y+h/2-t/2, w-2*t, t, cr, cg, cb, ca);
                  r->DrawRect(x+t, y+h-t, w-t, t, cr, cg, cb, ca); break;
        case 'G': r->DrawRect(x+t, y, w-t, t, cr, cg, cb, ca);
                  r->DrawRect(x, y+t, t, h-2*t, cr, cg, cb, ca);
                  r->DrawRect(x+t, y+h-t, w-t, t, cr, cg, cb, ca);
                  r->DrawRect(x+w-t, y+h/2, t, h/2-t, cr, cg, cb, ca);
                  r->DrawRect(x+w/2, y+h/2, w/2-t, t, cr, cg, cb, ca); break;
        case 'I': r->DrawRect(x+w/2-t/2, y, t, h, cr, cg, cb, ca); break;
        case 'K': r->DrawRect(x, y, t, h, cr, cg, cb, ca);
                  r->DrawRect(x+w-t, y, t, h/2, cr, cg, cb, ca);
                  r->DrawRect(x+w-t, y+h/2, t, h/2, cr, cg, cb, ca);
                  r->DrawRect(x+t, y+h/2-t/2, w-2*t, t, cr, cg, cb, ca); break;
        case 'L': r->DrawRect(x, y, t, h, cr, cg, cb, ca);
                  r->DrawRect(x+t, y+h-t, w-t, t, cr, cg, cb, ca); break;
        case 'N': r->DrawRect(x, y, t, h, cr, cg, cb, ca);
                  r->DrawRect(x+w-t, y, t, h, cr, cg, cb, ca);
                  r->DrawRect(x+t, y+t, w-2*t, t, cr, cg, cb, ca); break;
        case 'O': r->DrawRect(x+t, y, w-2*t, t, cr, cg, cb, ca);
                  r->DrawRect(x, y+t, t, h-2*t, cr, cg, cb, ca);
                  r->DrawRect(x+w-t, y+t, t, h-2*t, cr, cg, cb, ca);
                  r->DrawRect(x+t, y+h-t, w-2*t, t, cr, cg, cb, ca); break;
        case 'P': r->DrawRect(x, y, t, h, cr, cg, cb, ca);
                  r->DrawRect(x+t, y, w-2*t, t, cr, cg, cb, ca);
                  r->DrawRect(x+w-t, y+t, t, h/2-t, cr, cg, cb, ca);
                  r->DrawRect(x+t, y+h/2, w-2*t, t, cr, cg, cb, ca); break;
        case 'R': r->DrawRect(x, y, t, h, cr, cg, cb, ca);
                  r->DrawRect(x+t, y, w-2*t, t, cr, cg, cb, ca);
                  r->DrawRect(x+w-t, y+t, t, h/2-t, cr, cg, cb, ca);
                  r->DrawRect(x+t, y+h/2, w-2*t, t, cr, cg, cb, ca);
                  r->DrawRect(x+w-t, y+h/2+t, t, h/2-t, cr, cg, cb, ca); break;
        case 'S': r->DrawRect(x+t, y, w-t, t, cr, cg, cb, ca);
                  r->DrawRect(x, y+t, t, h/2-t, cr, cg, cb, ca);
                  r->DrawRect(x+t, y+h/2-t/2, w-2*t, t, cr, cg, cb, ca);
                  r->DrawRect(x+w-t, y+h/2+t/2, t, h/2-t, cr, cg, cb, ca);
                  r->DrawRect(x, y+h-t, w-t, t, cr, cg, cb, ca); break;
        case 'T': r->DrawRect(x, y, w, t, cr, cg, cb, ca);
                  r->DrawRect(x+w/2-t/2, y+t, t, h-t, cr, cg, cb, ca); break;
        case 'U': r->DrawRect(x, y, t, h-t, cr, cg, cb, ca);
                  r->DrawRect(x+w-t, y, t, h-t, cr, cg, cb, ca);
                  r->DrawRect(x+t, y+h-t, w-2*t, t, cr, cg, cb, ca); break;
        case 'X': r->DrawRect(x, y, t, h/2, cr, cg, cb, ca);
                  r->DrawRect(x+w-t, y, t, h/2, cr, cg, cb, ca);
                  r->DrawRect(x+w/2-t/2, y+h/2-t/2, t, t, cr, cg, cb, ca);
                  r->DrawRect(x, y+h/2, t, h/2, cr, cg, cb, ca);
                  r->DrawRect(x+w-t, y+h/2, t, h/2, cr, cg, cb, ca); break;
        case 'Y': r->DrawRect(x, y, t, h/2, cr, cg, cb, ca);
                  r->DrawRect(x+w-t, y, t, h/2, cr, cg, cb, ca);
                  r->DrawRect(x+w/2-t/2, y+h/2, t, h/2, cr, cg, cb, ca); break;
        case '!': r->DrawRect(x+w/2-t/2, y, t, h-2*t, cr, cg, cb, ca);
                  r->DrawRect(x+w/2-t/2, y+h-t, t, t, cr, cg, cb, ca); break;
        case '.': r->DrawRect(x+w/2-t/2, y+h-t, t, t, cr, cg, cb, ca); break;
        case ',': r->DrawRect(x+w/2-t/2, y+h-t, t, t, cr, cg, cb, ca);
                  r->DrawRect(x+w/2-t/2, y+h-2*t, t, t, cr, cg, cb, ca); break;
        case '\'': r->DrawRect(x+w/2-t/2, y+t, t, t, cr, cg, cb, ca); break;
        case ':': r->DrawRect(x+w/2-t/2, y+h/3, t, t, cr, cg, cb, ca);
                  r->DrawRect(x+w/2-t/2, y+h*2/3, t, t, cr, cg, cb, ca); break;
        case '-': r->DrawRect(x+t, y+h/2-t/2, w-2*t, t, cr, cg, cb, ca); break;
        case '1': r->DrawRect(x+w/2-t/2, y, t, h, cr, cg, cb, ca); break;
        case '2': r->DrawRect(x, y, w, t, cr, cg, cb, ca);
                  r->DrawRect(x+w-t, y+t, t, h/2-t, cr, cg, cb, ca);
                  r->DrawRect(x, y+h/2, w, t, cr, cg, cb, ca);
                  r->DrawRect(x, y+h/2+t, t, h/2-t, cr, cg, cb, ca);
                  r->DrawRect(x, y+h-t, w, t, cr, cg, cb, ca); break;
        case '3': r->DrawRect(x, y, w, t, cr, cg, cb, ca);
                  r->DrawRect(x+w-t, y+t, t, h-2*t, cr, cg, cb, ca);
                  r->DrawRect(x+t, y+h/2-t/2, w-t, t, cr, cg, cb, ca);
                  r->DrawRect(x, y+h-t, w, t, cr, cg, cb, ca); break;
        case '5': r->DrawRect(x, y, w, t, cr, cg, cb, ca);
                  r->DrawRect(x, y+t, t, h/2-t, cr, cg, cb, ca);
                  r->DrawRect(x, y+h/2, w, t, cr, cg, cb, ca);
                  r->DrawRect(x+w-t, y+h/2+t, t, h/2-t, cr, cg, cb, ca);
                  r->DrawRect(x, y+h-t, w-t, t, cr, cg, cb, ca); break;
        case '6': r->DrawRect(x+t, y, w-t, t, cr, cg, cb, ca);
                  r->DrawRect(x, y+t, t, h-2*t, cr, cg, cb, ca);
                  r->DrawRect(x+t, y+h/2, w-2*t, t, cr, cg, cb, ca);
                  r->DrawRect(x+w-t, y+h/2+t, t, h/2-2*t, cr, cg, cb, ca);
                  r->DrawRect(x+t, y+h-t, w-2*t, t, cr, cg, cb, ca); break;
        case '7': r->DrawRect(x, y, w, t, cr, cg, cb, ca);
                  r->DrawRect(x+w-t, y+t, t, h-t, cr, cg, cb, ca); break;
        default: break;
    }
}

static void DrawString(D3D11Renderer* r, float x, float y, float s, const char* str, float cr, float cg, float cb, float ca) {
    if (!r || !str) return;
    const float charW = 7.0f * s;
    float curX = x;
    while (*str) {
        char c = *str;
        if (c >= 'a' && c <= 'z') {
            c = (char)(c - 'a' + 'A');
        }
        DrawChar(r, curX, y, s, c, cr, cg, cb, ca);
        curX += charW;
        str++;
    }
}

void DrawTextUi(float x, float y, float s, const char* text, float r, float g, float b, float a) {
    if (!s_renderer || !text || s <= 0.0f) {
        return;
    }

    DrawString(s_renderer, x + 1.0f, y + 1.0f, s, text, 0.0f, 0.0f, 0.0f, a * 0.65f);
    DrawString(s_renderer, x, y, s, text, r, g, b, a);
}

static const char* MapGlyphTextureName(char c) {
    if (c >= 'a' && c <= 'z') c = (char)(c - 'a' + 'A');

    if (c >= '0' && c <= '9') {
        switch (c) {
            case '0': return "g_nam_01";
            case '1': return "g_nam_11";
            case '2': return "g_nam_21";
            case '3': return "g_nam_31";
            case '4': return "g_nam_41";
            case '5': return "g_nam_51";
            case '6': return "g_nam_61";
            case '7': return "g_nam_71";
            case '8': return "g_nam_81";
            case '9': return "g_nam_91";
        }
    }

    if (c >= 'A' && c <= 'Z') {
        switch (c) {
            case 'A': return "g_nam_a1";
            case 'B': return "g_nam_b1";
            case 'C': return "g_nam_c1";
            case 'D': return "g_nam_d1";
            case 'E': return "g_nam_e1";
            case 'F': return "g_nam_f1";
            case 'G': return "g_nam_g1";
            case 'H': return "g_nam_h1";
            case 'I': return "g_nam_i1";
            case 'J': return "g_nam_j1";
            case 'K': return "g_nam_k1";
            case 'L': return "g_nam_l1";
            case 'M': return "g_nam_m1";
            case 'N': return "g_nam_n1";
            case 'O': return "g_nam_o1";
            case 'P': return "g_nam_p1";
            case 'Q': return "g_nam_q1";
            case 'R': return "g_nam_r1";
            case 'S': return "g_nam_s1";
            case 'T': return "g_nam_t1";
            case 'U': return "g_nam_u1";
            case 'V': return "g_nam_v1";
            case 'W': return "g_nam_w1";
            case 'X': return "g_nam_x1";
            case 'Y': return "g_nam_y1";
            case 'Z': return "g_nam_z1";
        }
    }

    if (c == '_') return "g_nam__1";
    if (c == '!') return "g_nam_ex";
    if (c == '-') return "g_nam_mi";
    return nullptr;
}

static void DrawSpriteText(PrGameContext& ctx, float vx, float vy, float vs,
                            float x, float y, float scale, const char* text,
                            float r, float g, float b, float a) {
    if (!ctx.renderer || !ctx.resources || !text) return;

    float curX = x;
    const float adv = 24.0f * scale;
    while (*text) {
        const char c = *text++;
        if (c == ' ') {
            curX += adv;
            continue;
        }

        const char* name = MapGlyphTextureName(c);
        if (!name) {
            curX += adv;
            continue;
        }

        ID3D11ShaderResourceView* srv = ctx.resources->GetTextureView(name);
        if (!srv) {
            curX += adv;
            continue;
        }

        D3D11Renderer::SpriteCmd cmd;
        cmd.texture = srv;
        cmd.x = ToScreenX(vx, vs, curX);
        cmd.y = ToScreenY(vy, vs, y);
        cmd.w = 24.0f * scale * vs;
        cmd.h = 28.0f * scale * vs;
        cmd.u0 = 0.0f;
        cmd.v0 = 0.0f;
        cmd.u1 = 1.0f;
        cmd.v1 = 1.0f;
        cmd.r = r;
        cmd.g = g;
        cmd.b = b;
        cmd.a = a;
        cmd.blend = D3D11Renderer::BlendMode::Alpha;
        cmd.layer = 700;
        cmd.order = 0;
        ctx.renderer->SubmitSprite(cmd);

        curX += adv;
    }
}

static const char* PickSubtitleFontAtlasName(const PrGameContext& ctx) {
    int lang = (int)ctx.languageIndex;
    if (lang < 0 || lang >= 5) {
        lang = 0;
    }

    switch (lang) {
        case 0: return "EU1_256";
        case 1: return "EU2_256";
        case 2: return "EU3_256";
        case 3: return "EU4_256";
        case 4: return "EU4_256";
        default: return "EU1_256";
    }
}

struct PsxSubtitleGlyph {
    int16_t u;
    int16_t v;
    uint8_t w;
    uint8_t h;
    int16_t xOff;
};

static const uint64_t kPsxSubtitleGlyphPacked[256] = {
    0x00000000FFFFFFFFull, 0x00000000FFFFFFFFull,
    0x00000000FFFFFFFFull, 0x00000000FFFFFFFFull,
    0x00000000FFFFFFFFull, 0x00000000FFFFFFFFull,
    0x00000000FFFFFFFFull, 0x00000000FFFFFFFFull,
    0x00000000FFFFFFFFull, 0x00000000FFFFFFFFull,
    0x00000000FFFFFFFFull, 0x00000000FFFFFFFFull,
    0x00000000FFFFFFFFull, 0x00000000FFFFFFFFull,
    0x00000000FFFFFFFFull, 0x00000000FFFFFFFFull,
    0x00000000FFFFFFFFull, 0x00000000FFFFFFFFull,
    0x00000000FFFFFFFFull, 0x00000000FFFFFFFFull,
    0x00000000FFFFFFFFull, 0x00000000FFFFFFFFull,
    0x00000000FFFFFFFFull, 0x00000000FFFFFFFFull,
    0x00000000FFFFFFFFull, 0x00000000FFFFFFFFull,
    0x00000000FFFFFFFFull, 0x00000000FFFFFFFFull,
    0x00000000FFFFFFFFull, 0x00000000FFFFFFFFull,
    0x00000F0C003C0059ull, 0x00000F0C003C0065ull,
    0x00000F0500000000ull, 0x00000F0500000023ull,
    0x00000F0700000043ull, 0x00000F0B00000065ull,
    0x00000F0800000051ull, 0x00000F0C00000059ull,
    0x00000F0A00000070ull, 0xFFFF0F050000003Dull,
    0x00000F0700000030ull, 0x00000F0700000037ull,
    0x00000F0A0000007Aull, 0x00000F0C003C004Dull,
    0xFFFF0F0700000008ull, 0x00000F070000004Aull,
    0x00000F060000000Full, 0x00000000FFFFFFFFull,
    0x00000F0800000084ull, 0x00000F070000008Cull,
    0x00000F0700000093ull, 0x00000F070000009Aull,
    0x00000F08000000A1ull, 0x00000F08000000A9ull,
    0x00000F08000000B1ull, 0x00000F08000000B9ull,
    0x00000F09000000C1ull, 0x00000F07000000CAull,
    0x00000F04003C0005ull, 0x00000F04003C0000ull,
    0xFFFF0F0C003C002Eull, 0x00000F0C003C000Aull,
    0x00000F0C003C0039ull, 0x00000F080000001Bull,
    0x00000F0B002D00EFull, 0x00000F08000000D1ull,
    0x00000F09000000D9ull, 0x00000F07000000E2ull,
    0x00000F07000000E9ull, 0x00010F06000000F1ull,
    0x00000F08000000F6ull, 0x00000F08000F0000ull,
    0x00000F07000F0008ull, 0x00010F07000F000Full,
    0x00000F07000F0016ull, 0x00000F07000F001Dull,
    0x00000F08000F0024ull, 0x00000F0B000F002Cull,
    0x00000F08000F0037ull, 0x00000F08000F003Full,
    0x00000F08000F0047ull, 0x00000F09000F004Full,
    0x00000F08000F0058ull, 0x00000F07000F0060ull,
    0x00000F07000F0067ull, 0x00010F07000F006Full,
    0x00000F09000F0075ull, 0x00010F0B000F007Full,
    0x00000F08000F0089ull, 0x00000F09000F0091ull,
    0x00000F08000F009Aull, 0x00000F0C003C007Dull,
    0x00000F08001E0063ull, 0x00000F0C003C0089ull,
    0x00000F0800000028ull, 0x00000F08003C0045ull,
    0x00000F0600000015ull, 0x00000F08000F00A2ull,
    0x00000F07000F00AAull, 0x00000F07000F00B2ull,
    0x00000F08000F00B9ull, 0x00000F07000F00C1ull,
    0x00000F07000F00C8ull, 0x00000F07000F00CFull,
    0x00000F08000F00D6ull, 0x00010F05000F00DFull,
    0x00000F06000F00E3ull, 0x00000F06000F00E9ull,
    0x00000F04000F00EFull, 0x00000F08000F00F3ull,
    0x00000F07001E0000ull, 0x00000F07001E0007ull,
    0x00000F07001E000Full, 0x00000F07001E0015ull,
    0x00000F07001E001Cull, 0x00000F08001E0023ull,
    0x00000F07001E002Bull, 0x00000F08001E0032ull,
    0x00000F08001E003Aull, 0x00000F09001E0041ull,
    0x00000F07001E004Bull, 0x00000F08001E0052ull,
    0x00000F09001E005Aull, 0x00000F0A0000007Aull,
    0x00000F0C003C0017ull, 0xFFFF0F0C003C0071ull,
    0x00000F0C003C0022ull, 0x00000000FFFFFFFFull,
    0x00000000FFFFFFFFull, 0x00000000FFFFFFFFull,
    0x00000000FFFFFFFFull, 0x00000000FFFFFFFFull,
    0x00000000FFFFFFFFull, 0x00000000FFFFFFFFull,
    0x00000000FFFFFFFFull, 0x00000000FFFFFFFFull,
    0x00000000FFFFFFFFull, 0x00000000FFFFFFFFull,
    0x00000000FFFFFFFFull, 0x00000000FFFFFFFFull,
    0x00000F0B002D0016ull, 0x00000000FFFFFFFFull,
    0x00000000FFFFFFFFull, 0x00000000FFFFFFFFull,
    0x00000000FFFFFFFFull, 0x00000000FFFFFFFFull,
    0x00000000FFFFFFFFull, 0x00000000FFFFFFFFull,
    0x00000000FFFFFFFFull, 0x00000000FFFFFFFFull,
    0x00000000FFFFFFFFull, 0x00000000FFFFFFFFull,
    0x00000000FFFFFFFFull, 0x00000000FFFFFFFFull,
    0x00000000FFFFFFFFull, 0x00000000FFFFFFFFull,
    0x00000F0B002D002Aull, 0x00000000FFFFFFFFull,
    0x00000000FFFFFFFFull, 0x00000000FFFFFFFFull,
    0x00000000FFFFFFFFull, 0x00000F05002D003Dull,
    0x00000000FFFFFFFFull, 0x00000000FFFFFFFFull,
    0x00000000FFFFFFFFull, 0x00000000FFFFFFFFull,
    0x00000000FFFFFFFFull, 0x00000000FFFFFFFFull,
    0x00000000FFFFFFFFull, 0x00000000FFFFFFFFull,
    0x00000000FFFFFFFFull, 0x00000F08002D0042ull,
    0x00000000FFFFFFFFull, 0x00000000FFFFFFFFull,
    0x00000000FFFFFFFFull, 0x00000000FFFFFFFFull,
    0x00000000FFFFFFFFull, 0x00000000FFFFFFFFull,
    0x00000000FFFFFFFFull, 0x00000000FFFFFFFFull,
    0x00000000FFFFFFFFull, 0x00000000FFFFFFFFull,
    0x00000000FFFFFFFFull, 0x00000000FFFFFFFFull,
    0x00000000FFFFFFFFull, 0x00000000FFFFFFFFull,
    0x00010F05002D00F9ull, 0x00000F08002D004Aull,
    0x00000000FFFFFFFFull, 0x00000000FFFFFFFFull,
    0x00000000FFFFFFFFull, 0x00000F08002D0035ull,
    0x00000F08002D0052ull, 0x00010F08002D005Bull,
    0x00000F08002D0062ull, 0x00000000FFFFFFFFull,
    0x00010F08002D006Bull, 0x00000000FFFFFFFFull,
    0x00000000FFFFFFFFull, 0x00000F07002D000Full,
    0x00000F06002D0072ull, 0x00000F07002D0078ull,
    0x00000F07002D007Full, 0x00000F07002D0086ull,
    0x00010F07002D008Dull, 0x00010F07002D0094ull,
    0x00010F07002D009Bull, 0x00010F07002D00A2ull,
    0x00000000FFFFFFFFull, 0x00000F08002D00E6ull,
    0x00000F08002D00A9ull, 0x00000F08002D00B1ull,
    0x00000F08002D00B9ull, 0x00000000FFFFFFFFull,
    0x00000F08002D00C1ull, 0x00000000FFFFFFFFull,
    0x00000000FFFFFFFFull, 0x00000F07002D00C9ull,
    0x00010F07002D00D1ull, 0x00000F08002D00D7ull,
    0x00000F07002D00DFull, 0x00000000FFFFFFFFull,
    0x00000000FFFFFFFFull, 0x00000F09002D0021ull,
    0x00000F08001E006Bull, 0x00000F08001E0073ull,
    0x00000F08001E007Bull, 0x00000000FFFFFFFFull,
    0x00000F08001E0083ull, 0x00000000FFFFFFFFull,
    0x00000000FFFFFFFFull, 0x00000F07002D0008ull,
    0x00000F08001E008Bull, 0x00000F07001E0092ull,
    0x00000F08001E0099ull, 0x00000F07001E00A1ull,
    0x00000F05001E00A8ull, 0x00000F05001E00ADull,
    0x00000F06001E00B2ull, 0x00000F06001E00B8ull,
    0x00000000FFFFFFFFull, 0x00000F08002D0000ull,
    0x00000F07001E00BEull, 0x00000F07001E00C5ull,
    0x00000F07001E00CDull, 0x00000000FFFFFFFFull,
    0x00000F07001E00D3ull, 0x00000000FFFFFFFFull,
    0x00000000FFFFFFFFull, 0x00000F08001E00DBull,
    0x00000F08001E00E2ull, 0x00000F08001E00EBull,
    0x00000F08001E00F2ull, 0x00000000FFFFFFFFull,
    0x00000000FFFFFFFFull, 0x00000000FFFFFFFFull,
};

static PsxSubtitleGlyph DecodePsxSubtitleGlyph(unsigned char c) {
    const uint64_t p = kPsxSubtitleGlyphPacked[c];
    PsxSubtitleGlyph g;
    g.u = (int16_t)(p & 0xFFFFull);
    g.v = (int16_t)((p >> 16) & 0xFFFFull);
    g.w = (uint8_t)((p >> 32) & 0xFFull);
    g.h = (uint8_t)((p >> 40) & 0xFFull);
    g.xOff = (int16_t)((p >> 48) & 0xFFFFull);
    return g;
}

static bool IsValidPsxSubtitleGlyph(const PsxSubtitleGlyph& g) {
    if (g.u == (int16_t)-1 || g.v == (int16_t)-1) return false;
    if (g.w == 0 || g.h == 0) return false;
    return true;
}

float MeasureSubtitleTextNative(float s, const char* text) {
    if (!text || s <= 0.0f) {
        return 0.0f;
    }

    const PsxSubtitleGlyph gSpace = DecodePsxSubtitleGlyph((unsigned char)' ');
    const float spaceAdv = (float)(IsValidPsxSubtitleGlyph(gSpace) ? gSpace.w : 8) * s;
    float w = 0.0f;
    for (const char* p = text; *p; ++p) {
        const unsigned char uc = (unsigned char)*p;
        if (uc == '\n') {
            break;
        }
        if (uc == '\r') {
            continue;
        }
        if (uc == '\t') {
            w += spaceAdv * 4.0f;
            continue;
        }

        const PsxSubtitleGlyph g = DecodePsxSubtitleGlyph(uc);
        const float adv = (float)(IsValidPsxSubtitleGlyph(g) ? g.w : 8) * s;
        w += adv;
    }
    return w;
}

bool DrawSubtitleTextNative(PrGameContext& ctx, float x, float y, float s, const char* text,
                            float r, float g, float b, float a) {
    if (!ctx.renderer || !ctx.resources || !text || s <= 0.0f) {
        return false;
    }

    const char* atlasName = PickSubtitleFontAtlasName(ctx);
    if (!atlasName) {
        return false;
    }

    TextureResource* tr = ctx.resources->GetTexture(atlasName);
    ID3D11ShaderResourceView* srv = ctx.resources->GetTextureView(atlasName);
    if (!tr || !srv) {
        return false;
    }

    const float texW = (float)tr->tim.width;
    const float texH = (float)tr->tim.height;
    if (texW <= 0.0f || texH <= 0.0f) {
        return false;
    }

    const PsxSubtitleGlyph gSpace = DecodePsxSubtitleGlyph((unsigned char)' ');
    const float spaceAdv = (float)(IsValidPsxSubtitleGlyph(gSpace) ? gSpace.w : 8) * s;
    float cx = x;
    bool didWork = false;
    for (const char* p = text; *p; ++p) {
        const unsigned char uc = (unsigned char)*p;
        if (uc == '\n') {
            break;
        }
        if (uc == '\r') {
            continue;
        }
        if (uc == '\t') {
            cx += spaceAdv * 4.0f;
            didWork = true;
            continue;
        }
        if (uc == ' ') {
            cx += spaceAdv;
            didWork = true;
            continue;
        }

        const PsxSubtitleGlyph glyph = DecodePsxSubtitleGlyph(uc);
        if (!IsValidPsxSubtitleGlyph(glyph)) {
            cx += spaceAdv;
            didWork = true;
            continue;
        }

        const int uPix = (int)glyph.u;
        const int vPix = (int)(glyph.v & 0xFF);
        const float gw = (float)glyph.w;
        const float gh = (float)glyph.h;
        const float drawX = cx + (float)glyph.xOff * s;

        const float u0 = (float)uPix / texW;
        const float v0 = (float)vPix / texH;
        const float u1 = ((float)uPix + gw) / texW;
        const float v1 = ((float)vPix + gh) / texH;

        D3D11Renderer::SpriteCmd cmd;
        cmd.texture = srv;
        cmd.x = drawX;
        cmd.y = y;
        cmd.w = gw * s;
        cmd.h = gh * s;
        cmd.u0 = u0;
        cmd.v0 = v0;
        cmd.u1 = u1;
        cmd.v1 = v1;
        cmd.r = r;
        cmd.g = g;
        cmd.b = b;
        cmd.a = a;
        cmd.blend = D3D11Renderer::BlendMode::Alpha;
        cmd.layer = 950;
        cmd.order = 0;
        ctx.renderer->SubmitSprite(cmd);

        cx += (float)glyph.w * s;
        didWork = true;
    }

    return didWork;
}

static char LangSuffix(int langIndex) {
    switch (langIndex) {
        case 0: return 'E';
        case 1: return 'G';
        case 2: return 'F';
        case 3: return 'I';
        case 4: return 'S';
        default: return 'E';
    }
}

static void DrawTimAutoSize(PrGameContext& ctx, float vx, float vy, float vs,
                            const char* texName,
                            float x, float y,
                            float r, float g, float b, float a,
                            int layer) {
    if (!ctx.resources || !texName) return;
    TextureResource* tr = ctx.resources->GetTexture(texName);
    if (!tr) return;
    const float w = (float)tr->tim.width;
    const float h = (float)tr->tim.height;
    if (w <= 0.0f || h <= 0.0f) return;
    SubmitSpriteUI(ctx, vx, vy, vs, texName, x, y, w, h, r, g, b, a, layer);
}

// ======== ev=2 Stage Select: PSX TIM rendering ========
// memory.md §ev=2: sub_80020568
// Layout: title (32,33) | 6 stage entries per-language | EXIT (231,179)
// S0_OLD_DELETE_AFTER_SS0: legacy ev=2 stage-select UI renderer.
static void RenderStageSelect(PrGameContext& ctx, float vx, float vy, float vs,
                              const PrEventDispatcherContext& dispCtx) {
    vx += ctx.scn0PanelOffsetX * vs;
    vy += ctx.scn0PanelOffsetY * vs;
    const int cursor = (int)dispCtx.menuIndex;
    const int lang = std::clamp((int)ctx.languageIndex, 0, 4);
    const bool blinkOn = (PrEvent::GetEv2BlinkPtr() && *PrEvent::GetEv2BlinkPtr() != 0);

    const StageSelectJsonCache& stg = GetStageSelectJsonCache(ctx.dataRoot);
    const Ev3MenuJsonCache& ev3 = GetEv3MenuJsonCache(ctx.dataRoot);

    auto drawTable1 = [&](const PracticeUiLangTable1& table, int idx, int layer) -> bool {
        if (!table.valid) return false;
        int li = std::clamp(idx, 0, 4);
        if (!table.tplValid[li]) return false;
        return DrawPsxSpriteTemplate(ctx, vx, vy, vs, table.x[li], table.y[li], table.tpl[li], 1, 1, 1, 1, layer);
    };
    auto drawPosTpl = [&](float x, float y, const PsxSpriteTemplate& tpl, bool ok, int layer) -> bool {
        if (!ok) return false;
        return DrawPsxSpriteTemplate(ctx, vx, vy, vs, x, y, tpl, 1, 1, 1, 1, layer);
    };
    auto drawFixed = [&](const PracticeUiFixedTemplate& tpl, float x, float y, int layer) -> bool {
        if (!tpl.valid) return false;
        return DrawPsxSpriteTemplate(ctx, vx, vy, vs, x, y, tpl.tpl, 1, 1, 1, 1, layer);
    };
    auto drawEv3TableEntry = [&](const Ev3PosTable& table, int idx, int st, int layer) -> bool {
        float x = 0.0f;
        float y = 0.0f;
        PsxSpriteTemplate tpl = {};
        if (!TryGetEv3PosTableEntry(table, idx, st, x, y, tpl)) {
            return false;
        }
        return DrawPsxSpriteTemplate(ctx, vx, vy, vs, x, y, tpl, 1, 1, 1, 1, layer);
    };

    // Title.
    drawFixed(stg.titleBg, 32.0f, 33.0f, 700);
    drawTable1(stg.titleText, lang, 710);

    // Local stage state arrays mirror sub_80020568.
    int localSelect[6] = {};
    int localDotState[6] = {};
    int localNameState[6] = {};
    int localBadgeState[6] = {};
    for (int i = 0; i < 6; i++) {
        const int rawStatus = std::clamp((int)dispCtx.selections[i + 1], 0, 3);
        const bool enabled = (rawStatus != 0);
        localBadgeState[i] = enabled ? 1 : 0;
        localDotState[i] = enabled ? 0 : 2;
        localNameState[i] = enabled ? 0 : 1;
        localSelect[i] = 0;
    }
    if (cursor >= 1 && cursor <= 6) {
        const int idx = cursor - 1;
        localDotState[idx] = 1;
        localBadgeState[idx] = 2;
        localSelect[idx] = 1;
    }

    // 6 stage entries.
    for (int i = 0; i < 6; i++) {
        const int boxState = std::clamp(localBadgeState[i], 0, 2);
        const int topState = std::clamp(localBadgeState[i], 0, 2);
        const int nameState = std::clamp(localNameState[i], 0, 1);
        const int rawStatus = std::clamp((int)dispCtx.selections[i + 1], 0, 3);
        const int badgeAState = std::clamp(2 * rawStatus + localSelect[i], 0, 7);
        const int badgeBState = std::clamp(localBadgeState[i], 0, 2);

        drawPosTpl(stg.stageTopX[i], stg.stageTopY[i],
                   stg.stageTopTpl[topState], stg.stageTopTplValid[topState], 700);
        drawPosTpl(stg.stageNameX[i], stg.stageNameY[i],
                   stg.stageNameTpl[i][nameState], stg.stageNameTplValid[i][nameState], 705);
        drawPosTpl(stg.stageBoxX[lang][i], stg.stageBoxY[lang][i],
                   stg.stageBoxTpl[lang][boxState], stg.stageBoxTplValid[lang][boxState], 710);
        if (stg.stageDotTpl.valid) {
            DrawPsxSpriteTemplateSubrect(ctx, vx, vy, vs,
                                         stg.stageDotX[lang][i], stg.stageDotY[lang][i],
                                         stg.stageDotTpl.tpl,
                                         (int)stg.stageDotArgA[i], (int)stg.stageDotArgB[i], localDotState[i],
                                         1, 1, 1, 1, 711);
        }
        drawPosTpl(stg.stageBadgeX[i], stg.stageBadgeY[i],
                   stg.stageBadgeBTpl[badgeBState], stg.stageBadgeBTplValid[badgeBState], 712);
        drawPosTpl(stg.stageBadgeX[i], stg.stageBadgeY[i],
                   stg.stageBadgeATpl[badgeAState], stg.stageBadgeATplValid[badgeAState], 713);
    }

    // BONUS special block.
    if (cursor == 7) {
        drawFixed(dispCtx.confirmFlag ? stg.bonusBaseOn : stg.bonusBaseOff,
                  dispCtx.confirmFlag ? 64.0f : 63.0f,
                  dispCtx.confirmFlag ? 100.0f : 98.0f,
                  700);
        drawFixed(stg.bonusLabel, 66.0f, 103.0f, 710);
    } else if (dispCtx.selections[7] != 0) {
        drawFixed(stg.bonusBaseEnabled, 63.0f, 98.0f, 700);
        drawFixed(stg.bonusLabel, 66.0f, 103.0f, 710);
    }

    // EXIT/CANCEL.
    {
        const int textState = (cursor == 8) ? (dispCtx.confirmFlag ? 3 : 2) : 1;
        const int barState = textState - 1;
        drawFixed((cursor == 8 && blinkOn) ? stg.exitFrameOn : stg.exitFrameOff, 231.0f, 179.0f, 700);
        drawEv3TableEntry(ev3.tbl_EXIT, lang, textState, 710);
        if (barState >= 0 && barState <= 2) {
            drawFixed(kLanguageExitBar[barState], 238.0f, 188.0f, 705);
        }
    }
}

// S0_OLD_DELETE_AFTER_SS0: legacy ev=3 main-menu UI renderer.
static void RenderScn0MainMenu(PrGameContext& ctx, float vx, float vy, float vs,
                               const PrEventDispatcherContext& dispCtx) {
    // Apply panel slide-in animation offset (PSX: dx=-163, dy=192, frames=204)
    vx += ctx.scn0PanelOffsetX * vs;
    vy += ctx.scn0PanelOffsetY * vs;

    const int cursor = (int)dispCtx.menuIndex;
    const int lang = (int)ctx.languageIndex;
    const char lc = LangSuffix(lang);
    const bool blinkOn = (PrEvent::GetEv3BlinkPtr() && *PrEvent::GetEv3BlinkPtr() != 0);

    const Ev3MenuJsonCache& ev3 = GetEv3MenuJsonCache(ctx.dataRoot);
    if (ev3.ok) {
        (void)ev3;
    }

    // ===== PSX-accurate sprite positions (extracted from sub_80021E60 + sprite struct data) =====
    // PSX draws two layers per button:
    //   Layer 0 (BG):   sub_8001C5A8(sprite_struct, tim) → coords from struct [x:s16][y:s16]
    //   Layer 1 (Text): sub_8001C550(x, y, tim) → hardcoded coords in code
    // BG positions vary by language for some buttons.

    // --- BG positions (from PSX sprite struct data, per-language) ---
    // Per-language BG offsets: [E=0, G=1, F=2, I=3, S=4]
    // Button 0 (PLAY!): BG fixed at (41, 64)
    static const float kBtn0BgX = 41.0f, kBtn0BgY = 64.0f;
    // Button 1 (HI-SCORE): BG per-lang
    static const float kBtn1BgX[] = {124.0f, 123.0f, 121.0f, 126.0f, 124.0f};
    static const float kBtn1BgY[] = { 81.0f,  81.0f,  82.0f,  80.0f,  81.0f};
    // Button 2 NORMAL BG: fixed (218, 52); EASY BG: (218,76)/(220,76)/(218,76)/(218,76)
    static const float kNormBgX = 218.0f, kNormBgY = 52.0f;
    static const float kEasyBgX[] = {218.0f, 220.0f, 218.0f, 218.0f, 218.0f};
    static const float kEasyBgY[] = { 76.0f,  76.0f,  76.0f,  76.0f,  76.0f};
    // Shared decorators for btn2 area
    static const float kBtn2DecAX = 216.0f, kBtn2DecAY = 47.0f;
    static const float kBtn2DecBX = 216.0f, kBtn2DecBY = 71.0f;
    // Button 3 sub-buttons: SAVE/LOAD/REPLAY/OPTION BG per-lang
    static const float kSaveBgX[]   = {41.0f, 42.0f, 43.0f, 42.0f, 41.0f};
    static const float kSaveBgY[]   = {127.0f, 126.0f, 123.0f, 127.0f, 127.0f};
    static const float kLoadBgX[]   = {99.0f, 100.0f, 101.0f, 101.0f, 99.0f};
    static const float kLoadBgY[]   = {144.0f, 144.0f, 145.0f, 145.0f, 144.0f};
    static const float kReplayBgX[] = {161.0f, 161.0f, 162.0f, 162.0f, 161.0f};
    static const float kReplayBgY[] = {152.0f, 146.0f, 149.0f, 152.0f, 152.0f};
    static const float kOptBgX[]    = {226.0f, 224.0f, 225.0f, 224.0f, 226.0f};
    static const float kOptBgY[]    = {136.0f, 134.0f, 132.0f, 134.0f, 136.0f};
    // EXIT BG
    static const float kExitBgX = 238.0f, kExitBgY = 188.0f;
    static const float kExitLblBgX[] = {242.0f, 238.0f, 238.0f, 241.0f, 242.0f};
    static const float kExitLblBgY[] = {191.0f, 193.0f, 193.0f, 190.0f, 191.0f};

    // --- Text overlay positions (hardcoded in PSX sub_80021E60) ---
    // Button 0: text at (37,47) when selected; overlay at (28,36)
    const float txt0x = 37.0f, txt0y = 47.0f;
    const float ovl0x = 28.0f, ovl0y = 36.0f;
    // Button 1: text at (110,56); hint at (118,65)
    const float txt1x = 110.0f, txt1y = 56.0f;
    const float hnt1x = 118.0f, hnt1y = 65.0f;
    // Button 2: text at (207,34)
    const float txt2x = 207.0f, txt2y = 34.0f;
    // Button 3 text (bottom text overlays): SAVE(32,115), LOAD(95,136), REPLAY(160,136), OPTION(216,115)
    const float txt5x = 32.0f,  txt5y = 115.0f;
    const float txt6x = 95.0f,  txt6y = 136.0f;
    const float txt7x = 160.0f, txt7y = 136.0f;
    const float txt8x = 216.0f, txt8y = 115.0f;
    // Button 3 bottom sub-sprite positions: (26,106), (94,122), (162,122), (226,106)
    const float bot5x = 26.0f,  bot5y = 106.0f;
    const float bot6x = 94.0f,  bot6y = 122.0f;
    const float bot7x = 162.0f, bot7y = 122.0f;
    const float bot8x = 226.0f, bot8y = 106.0f;
    // EXIT text at (231,179)
    const float txtEx = 231.0f, txtEy = 179.0f;

    const int li = (lang < 5) ? lang : 0; // clamp language index

    auto pickState = [&](int group) -> int {
        if (cursor != group) return 1;
        bool pressed = false;
        switch (group) {
            case 0:
                // PSX: state word for idx=0 becomes 1 when Cross pressed.
                pressed = (dispCtx.selections[0] != 0);
                break;
            case 1:
                // PSX: state word for idx=1 starts at -1; becomes 0 when Cross pressed.
                pressed = (dispCtx.selections[1] != -1);
                break;
            case 4:
                // PSX: EXIT uses a separate confirm/pressed flag.
                pressed = (dispCtx.confirmFlag != 0);
                break;
            default:
                pressed = false;
                break;
        }
        // PSX rule: focused (not pressed) uses tpl1, pressed uses tpl2.
        // blinkOnOff drives the top frame overlay, not the tpl selection.
        return pressed ? 3 : 2;
    };

    // Fallback button helper: draw colored rect + label when TIM unavailable
    auto fallbackBtn = [&](float fx, float fy, float fw, float fh,
                           const char* label, bool selected, bool active) {
        float cr = 0.12f, cg = 0.12f, cb = 0.22f;
        if (selected && active) { cr = 0.55f; cg = 0.40f; cb = 0.08f; }
        else if (selected)      { cr = 0.35f; cg = 0.25f; cb = 0.06f; }
        DrawRectUI(s_renderer, vx, vy, vs, fx, fy, fw, fh, cr, cg, cb, 0.85f);
        if (selected) {
            DrawString(s_renderer, ToScreenX(vx,vs,fx-6), ToScreenY(vy,vs,fy+3),
                      1.0f*vs, ">", 1.0f, 0.85f, 0.2f, 1.0f);
        }
        DrawString(s_renderer, ToScreenX(vx,vs,fx+4), ToScreenY(vy,vs,fy+3),
                  1.0f*vs, label, 0.9f, 0.9f, 0.9f, active ? 1.0f : 0.5f);
    };

    static const PsxSpriteTemplate kEv3TplBg_MAIN_1 = {0x50000040, 0x014C, 0x0000, 0x0044, 0x002D, 0x03C0, 0x0008};
    static const PsxSpriteTemplate kEv3TplBg_MAIN_2 = {0x50000040, 0x015D, 0x0000, 0x0054, 0x002A, 0x03C0, 0x000E};
    static const PsxSpriteTemplate kEv3TplBg_MAIN_3 = {0x50000040, 0x0140, 0x002D, 0x0044, 0x0016, 0x03C0, 0x0014};
    static const PsxSpriteTemplate kEv3TplBg_MAIN_4 = {0x50000040, 0x0140, 0x0043, 0x0044, 0x0016, 0x03C0, 0x001A};
    static const PsxSpriteTemplate kEv3TplBg_MAIN_5 = {0x50000040, 0x0151, 0x002D, 0x0048, 0x0037, 0x03C0, 0x0020};
    static const PsxSpriteTemplate kEv3TplBg_MAIN_6 = {0x50000040, 0x0163, 0x002D, 0x0040, 0x002B, 0x03C0, 0x0026};
    static const PsxSpriteTemplate kEv3TplBg_MAIN_7 = {0x50000040, 0x0140, 0x0064, 0x0044, 0x002B, 0x03C0, 0x002C};
    static const PsxSpriteTemplate kEv3TplBg_MAIN_8 = {0x50000040, 0x0151, 0x0064, 0x0048, 0x0037, 0x03C0, 0x0032};
    static const PsxSpriteTemplate kEv3TplBg_EXIT_0 = {0x50000040, 0x0140, 0x0000, 0x0030, 0x0011, 0x03C0, 0x0002};
    static const PsxSpriteTemplate kEv3TplExitFrame_0 = {0x50000040, 0x0180, 0x0000, 0x0040, 0x0025, 0x03C0, 0x0000};
    static const PsxSpriteTemplate kEv3TplExitFrame_1 = {0x50000040, 0x0180, 0x0000, 0x0040, 0x0025, 0x03C0, 0x0001};

    static const PsxSpriteTemplate kEv3TplTop_1_0 = {0x50000040, 0x0190, 0x0000, 0x0058, 0x0044, 0x03C0, 0x0000};
    static const PsxSpriteTemplate kEv3TplTop_1_1 = {0x50000040, 0x0190, 0x0000, 0x0058, 0x0044, 0x03C0, 0x0001};
    static const PsxSpriteTemplate kEv3TplTop_2_0 = {0x50000040, 0x01A6, 0x0000, 0x0064, 0x003D, 0x03C0, 0x0000};
    static const PsxSpriteTemplate kEv3TplTop_2_1 = {0x50000040, 0x01A6, 0x0000, 0x0064, 0x003D, 0x03C0, 0x0001};
    static const PsxSpriteTemplate kEv3TplTop_3_0 = {0x50000040, 0x0180, 0x0044, 0x0058, 0x0043, 0x03C0, 0x0000};
    static const PsxSpriteTemplate kEv3TplTop_3_1 = {0x50000040, 0x0180, 0x0044, 0x0058, 0x0043, 0x03C0, 0x0001};

    {
        const bool hi0 = (cursor == 0) && blinkOn;
        const bool hi1 = (cursor == 1) && blinkOn;
        const bool hi2 = (cursor == 2) && blinkOn;
        const int st0 = hi0 ? 1 : 0;
        const int st1 = hi1 ? 1 : 0;
        const int st2 = hi2 ? 1 : 0;

        const float x0 = ev3.topFrames[0].valid ? ev3.topFrames[0].x : ovl0x;
        const float y0 = ev3.topFrames[0].valid ? ev3.topFrames[0].y : ovl0y;
        const float x1 = ev3.topFrames[1].valid ? ev3.topFrames[1].x : txt1x;
        const float y1 = ev3.topFrames[1].valid ? ev3.topFrames[1].y : txt1y;
        const float x2 = ev3.topFrames[2].valid ? ev3.topFrames[2].x : txt2x;
        const float y2 = ev3.topFrames[2].valid ? ev3.topFrames[2].y : txt2y;

        const PsxSpriteTemplate t0 = (ev3.topFrames[0].valid && ev3.topFrames[0].tplValid[st0]) ? ev3.topFrames[0].tpl[st0] : (hi0 ? kEv3TplTop_1_1 : kEv3TplTop_1_0);
        const PsxSpriteTemplate t1 = (ev3.topFrames[1].valid && ev3.topFrames[1].tplValid[st1]) ? ev3.topFrames[1].tpl[st1] : (hi1 ? kEv3TplTop_2_1 : kEv3TplTop_2_0);
        const PsxSpriteTemplate t2 = (ev3.topFrames[2].valid && ev3.topFrames[2].tplValid[st2]) ? ev3.topFrames[2].tpl[st2] : (hi2 ? kEv3TplTop_3_1 : kEv3TplTop_3_0);

        DrawPsxSpriteTemplate(ctx, vx, vy, vs, x0, y0, t0, 1, 1, 1, 1, 680);
        DrawPsxSpriteTemplate(ctx, vx, vy, vs, x1, y1, t1, 1, 1, 1, 1, 680);
        DrawPsxSpriteTemplate(ctx, vx, vy, vs, x2, y2, t2, 1, 1, 1, 1, 680);
    }

    static const PsxSpriteTemplate kEv3TplText_MAIN_1[5] = {
        {0x50000040, 0x0140, 0x010B, 0x003C, 0x000C, 0x03C0, 0x000B},
        {0x50000040, 0x014F, 0x0117, 0x003C, 0x000B, 0x03C0, 0x000B},
        {0x50000040, 0x0140, 0x0117, 0x003C, 0x000C, 0x03C0, 0x000B},
        {0x50000040, 0x015E, 0x0117, 0x0038, 0x000C, 0x03C0, 0x000B},
        {0x50000040, 0x016C, 0x0117, 0x003C, 0x000B, 0x03C0, 0x000B},
    };
    static const PsxSpriteTemplate kEv3TplText_MAIN_2[5] = {
        {0x50000040, 0x0140, 0x0123, 0x003C, 0x000B, 0x03C0, 0x0011},
        {0x50000040, 0x015E, 0x0123, 0x003C, 0x000B, 0x03C0, 0x0011},
        {0x50000040, 0x0161, 0x01C6, 0x0040, 0x000A, 0x03C0, 0x0011},
        {0x50000040, 0x016D, 0x0123, 0x0038, 0x000C, 0x03C0, 0x0011},
        {0x50000040, 0x0171, 0x01C5, 0x0038, 0x001A, 0x03C0, 0x0011},
    };
    static const PsxSpriteTemplate kEv3TplText_MAIN_3[5] = {
        {0x50000040, 0x0140, 0x0139, 0x0034, 0x000D, 0x03C0, 0x0017},
        {0x50000040, 0x015A, 0x0139, 0x0034, 0x000D, 0x03C0, 0x0017},
        {0x50000040, 0x014D, 0x0139, 0x0034, 0x000D, 0x03C0, 0x0017},
        {0x50000040, 0x0167, 0x0139, 0x0034, 0x000D, 0x03C0, 0x0017},
        {0x50000040, 0x0140, 0x0146, 0x0034, 0x000D, 0x03C0, 0x0017},
    };
    static const PsxSpriteTemplate kEv3TplText_MAIN_4[5] = {
        {0x50000040, 0x0140, 0x0153, 0x0030, 0x000C, 0x03C0, 0x001D},
        {0x50000040, 0x0158, 0x0153, 0x0030, 0x000C, 0x03C0, 0x001D},
        {0x50000040, 0x014C, 0x0153, 0x0030, 0x000C, 0x03C0, 0x001D},
        {0x50000040, 0x0164, 0x0153, 0x0030, 0x000C, 0x03C0, 0x001D},
        {0x50000040, 0x0170, 0x0153, 0x0030, 0x000F, 0x03C0, 0x001D},
    };
    static const PsxSpriteTemplate kEv3TplText_MAIN_5[5] = {
        {0x50000040, 0x0140, 0x015F, 0x002C, 0x0019, 0x03C0, 0x0023},
        {0x50000040, 0x0156, 0x015F, 0x0028, 0x001A, 0x03C0, 0x0023},
        {0x50000040, 0x014B, 0x015F, 0x002C, 0x0022, 0x03C0, 0x0023},
        {0x50000040, 0x0160, 0x015F, 0x0028, 0x001A, 0x03C0, 0x0023},
        {0x50000040, 0x0170, 0x0162, 0x002C, 0x001A, 0x03C0, 0x0023},
    };
    static const PsxSpriteTemplate kEv3TplText_MAIN_6[5] = {
        {0x50000040, 0x0140, 0x0178, 0x002C, 0x001C, 0x03C0, 0x0029},
        {0x50000040, 0x0156, 0x0179, 0x002C, 0x001C, 0x03C0, 0x0029},
        {0x50000040, 0x014B, 0x0181, 0x002C, 0x001B, 0x03C0, 0x0029},
        {0x50000040, 0x0161, 0x0179, 0x002C, 0x001C, 0x03C0, 0x0029},
        {0x50000040, 0x0170, 0x017C, 0x002C, 0x001A, 0x03C0, 0x0029},
    };
    static const PsxSpriteTemplate kEv3TplText_MAIN_7[5] = {
        {0x50000040, 0x0140, 0x0194, 0x002C, 0x000D, 0x03C0, 0x002F},
        {0x50000040, 0x0157, 0x0195, 0x0030, 0x001B, 0x03C0, 0x002F},
        {0x50000040, 0x014B, 0x019C, 0x0030, 0x0018, 0x03C0, 0x002F},
        {0x50000040, 0x0163, 0x0195, 0x002C, 0x000E, 0x03C0, 0x002F},
        {0x50000040, 0x0170, 0x0196, 0x0028, 0x0018, 0x03C0, 0x002F},
    };
    static const PsxSpriteTemplate kEv3TplText_MAIN_8[5] = {
        {0x50000040, 0x0140, 0x01A1, 0x0024, 0x0013, 0x03C0, 0x0035},
        {0x50000040, 0x0157, 0x01B0, 0x0028, 0x0016, 0x03C0, 0x0035},
        {0x50000040, 0x014B, 0x01B4, 0x0028, 0x0017, 0x03C0, 0x0035},
        {0x50000040, 0x0163, 0x01A3, 0x0028, 0x0015, 0x03C0, 0x0035},
        {0x50000040, 0x0170, 0x01AE, 0x0028, 0x0017, 0x03C0, 0x0035},
    };
    static const PsxSpriteTemplate kEv3TplText_EXIT_0[5] = {
        {0x50000040, 0x0140, 0x0100, 0x001C, 0x000B, 0x03C0, 0x0005},
        {0x50000040, 0x014F, 0x0100, 0x0020, 0x0008, 0x03C0, 0x0005},
        {0x50000040, 0x0147, 0x0100, 0x0020, 0x0008, 0x03C0, 0x0005},
        {0x50000040, 0x0157, 0x0100, 0x001C, 0x000C, 0x03C0, 0x0005},
        {0x50000040, 0x015E, 0x0100, 0x0020, 0x000A, 0x03C0, 0x0005},
    };

    // ===== Button 0: PLAY! =====
    {
        const int st = pickState(0);
        const float b0BgX = ev3.btn0Bg.valid ? ev3.btn0Bg.x : txt0x;
        const float b0BgY = ev3.btn0Bg.valid ? ev3.btn0Bg.y : txt0y;
        const float b0TxX = ev3.tbl_LANGUAGE.valid ? ev3.tbl_LANGUAGE.x[li] : kBtn0BgX;
        const float b0TxY = ev3.tbl_LANGUAGE.valid ? ev3.tbl_LANGUAGE.y[li] : kBtn0BgY;
        const PsxSpriteTemplate b0TextTpl =
            (ev3.tbl_LANGUAGE.valid && st >= 1 && st <= 3 && ev3.tbl_LANGUAGE.tplValid[li][st - 1])
                ? ev3.tbl_LANGUAGE.tpl[li][st - 1]
                : PsxTplWithState(kEv3TplText_MAIN_1[li], st);
        const PsxSpriteTemplate b0BgTpl = (ev3.btn0Bg.valid && st >= 1 && st <= 3 && ev3.btn0Bg.tplValid[st - 1])
                                              ? ev3.btn0Bg.tpl[st - 1]
                                              : PsxTplWithState(kEv3TplBg_MAIN_1, st);
        const bool bgOk = DrawPsxSpriteTemplate(ctx, vx, vy, vs,
                                                b0BgX,
                                                b0BgY,
                                                b0BgTpl,
                                                1, 1, 1, 1, 700);
        DrawPsxSpriteTemplate(ctx, vx, vy, vs, b0TxX, b0TxY,
                              b0TextTpl,
                              1, 1, 1, 1, 710);
        if (!bgOk) {
            fallbackBtn(b0BgX, b0BgY, (float)kEv3TplBg_MAIN_1.w, (float)kEv3TplBg_MAIN_1.h,
                        "PLAY", cursor == 0, blinkOn || cursor != 0);
        }
    }

    // ===== Button 1: HI-SCORE =====
    {
        const int st = pickState(1);
        const float b1HintX = ev3.btn1Hint.valid ? ev3.btn1Hint.x : hnt1x;
        const float b1HintY = ev3.btn1Hint.valid ? ev3.btn1Hint.y : hnt1y;
        const float b1TxX = ev3.tbl_HI_SCORE.valid ? ev3.tbl_HI_SCORE.x[li] : kBtn1BgX[li];
        const float b1TxY = ev3.tbl_HI_SCORE.valid ? ev3.tbl_HI_SCORE.y[li] : kBtn1BgY[li];
        const PsxSpriteTemplate b1HintTpl = (ev3.btn1Hint.valid && st >= 1 && st <= 3 && ev3.btn1Hint.tplValid[st - 1])
                                                ? ev3.btn1Hint.tpl[st - 1]
                                                : PsxTplWithState(kEv3TplBg_MAIN_2, st);
        const bool bgOk = DrawPsxSpriteTemplate(ctx, vx, vy, vs,
                                                b1HintX,
                                                b1HintY,
                                                b1HintTpl,
                                                1, 1, 1, 1, 700);
        const PsxSpriteTemplate b1TextTpl =
            (ev3.tbl_HI_SCORE.valid && st >= 1 && st <= 3 && ev3.tbl_HI_SCORE.tplValid[li][st - 1])
                ? ev3.tbl_HI_SCORE.tpl[li][st - 1]
                : PsxTplWithState(kEv3TplText_MAIN_2[li], st);
        DrawPsxSpriteTemplate(ctx, vx, vy, vs, b1TxX, b1TxY,
                              b1TextTpl,
                              1, 1, 1, 1, 710);
        if (!bgOk) {
            fallbackBtn(b1HintX, b1HintY, (float)kEv3TplBg_MAIN_2.w, (float)kEv3TplBg_MAIN_2.h,
                        "HI-SCORE", cursor == 1, blinkOn || cursor != 1);
        }
    }

    // ===== Button 2: NORMAL / EASY =====
    {
        const int sel = (int)dispCtx.selections[2];
        int stN = 1;
        int stE = 1;
        if (cursor == 2) {
            // PSX rule:
            //   if selected difficulty == NORMAL: NORMAL uses tpl2, EASY uses tpl1
            //   if selected difficulty == EASY:   NORMAL uses tpl1, EASY uses tpl2
            stN = (sel == 0) ? 3 : 2;
            stE = (sel == 1) ? 3 : 2;
        } else {
            // Not focused: show which difficulty is selected (tpl1), other is tpl0.
            stN = (sel == 0) ? 2 : 1;
            stE = (sel == 1) ? 2 : 1;
        }

        const float b2nBgX = kBtn2DecAX;
        const float b2nBgY = kBtn2DecAY;
        const float b2eBgX = kBtn2DecBX;
        const float b2eBgY = kBtn2DecBY;

        const float b2nDecX = ev3.btn2Deco[0].valid ? ev3.btn2Deco[0].x : b2nBgX;
        const float b2nDecY = ev3.btn2Deco[0].valid ? ev3.btn2Deco[0].y : b2nBgY;
        const float b2eDecX = ev3.btn2Deco[1].valid ? ev3.btn2Deco[1].x : b2eBgX;
        const float b2eDecY = ev3.btn2Deco[1].valid ? ev3.btn2Deco[1].y : b2eBgY;

        const PsxSpriteTemplate b2nDecTpl = (ev3.btn2Deco[0].valid && stN >= 1 && stN <= 3 && ev3.btn2Deco[0].tplValid[stN - 1])
                                                ? ev3.btn2Deco[0].tpl[stN - 1]
                                                : PsxTplWithState(kEv3TplBg_MAIN_3, stN);
        const PsxSpriteTemplate b2eDecTpl = (ev3.btn2Deco[1].valid && stE >= 1 && stE <= 3 && ev3.btn2Deco[1].tplValid[stE - 1])
                                                ? ev3.btn2Deco[1].tpl[stE - 1]
                                                : PsxTplWithState(kEv3TplBg_MAIN_4, stE);

        const bool bgOkN = DrawPsxSpriteTemplate(ctx, vx, vy, vs,
                                                 b2nDecX,
                                                 b2nDecY,
                                                 b2nDecTpl,
                                                 1, 1, 1, 1, 700);
        const bool bgOkE = DrawPsxSpriteTemplate(ctx, vx, vy, vs,
                                                 b2eDecX,
                                                 b2eDecY,
                                                 b2eDecTpl,
                                                 1, 1, 1, 1, 700);

        const float b2nTxX = ev3.tbl_NORMAL.valid ? ev3.tbl_NORMAL.x[li] : kNormBgX;
        const float b2nTxY = ev3.tbl_NORMAL.valid ? ev3.tbl_NORMAL.y[li] : kNormBgY;
        const float b2eTxX = ev3.tbl_EASY.valid ? ev3.tbl_EASY.x[li] : kEasyBgX[li];
        const float b2eTxY = ev3.tbl_EASY.valid ? ev3.tbl_EASY.y[li] : kEasyBgY[li];

        const PsxSpriteTemplate b2nTextTpl =
            (ev3.tbl_NORMAL.valid && stN >= 1 && stN <= 3 && ev3.tbl_NORMAL.tplValid[li][stN - 1])
                ? ev3.tbl_NORMAL.tpl[li][stN - 1]
                : PsxTplWithState(kEv3TplText_MAIN_3[li], stN);
        const bool textOkN = DrawPsxSpriteTemplate(ctx, vx, vy, vs, b2nTxX, b2nTxY,
                                                   b2nTextTpl,
                                                   1, 1, 1, 1, 710);
        if (!textOkN && !bgOkN) {
            fallbackBtn(b2nBgX, b2nBgY, (float)kEv3TplBg_MAIN_3.w, (float)kEv3TplBg_MAIN_3.h, "NORMAL",
                       cursor == 2, (sel == 0) || (cursor == 2 && blinkOn));
        }

        const PsxSpriteTemplate b2eTextTpl =
            (ev3.tbl_EASY.valid && stE >= 1 && stE <= 3 && ev3.tbl_EASY.tplValid[li][stE - 1])
                ? ev3.tbl_EASY.tpl[li][stE - 1]
                : PsxTplWithState(kEv3TplText_MAIN_4[li], stE);
        const bool textOkE = DrawPsxSpriteTemplate(ctx, vx, vy, vs, b2eTxX, b2eTxY,
                                                   b2eTextTpl,
                                                   1, 1, 1, 1, 710);
        if (!textOkE && !bgOkE) {
            fallbackBtn(b2eBgX, b2eBgY, (float)kEv3TplBg_MAIN_4.w, (float)kEv3TplBg_MAIN_4.h, "EASY",
                       cursor == 2, (sel == 1) || (cursor == 2 && blinkOn));
        }
    }

    // ===== Button 3: SAVE / LOAD / REPLAY / OPTION =====
    {
        const int stageChoice = (int)dispCtx.selections[3];
        const int st5 = (cursor != 3) ? 1 : (stageChoice == 0 ? 3 : 2);
        const int st6 = (cursor != 3) ? 1 : (stageChoice == 1 ? 3 : 2);
        const int st7 = (cursor != 3) ? 1 : (stageChoice == 2 ? 3 : 2);
        const int st8 = (cursor != 3) ? 1 : (stageChoice == 3 ? 3 : 2);

        static const PsxSpriteTemplate kEv3TplFrame_5_0 = {0x50000040, 0x0196, 0x0044, 0x0044, 0x0049, 0x03C0, 0x0000};
        static const PsxSpriteTemplate kEv3TplFrame_5_1 = {0x50000040, 0x0196, 0x0044, 0x0044, 0x0049, 0x03C0, 0x0001};
        static const PsxSpriteTemplate kEv3TplFrame_6_0 = {0x50000040, 0x01A7, 0x0044, 0x0044, 0x0042, 0x03C0, 0x0000};
        static const PsxSpriteTemplate kEv3TplFrame_6_1 = {0x50000040, 0x01A7, 0x0044, 0x0044, 0x0042, 0x03C0, 0x0001};
        static const PsxSpriteTemplate kEv3TplFrame_7_0 = {0x50000040, 0x0180, 0x008D, 0x0040, 0x0042, 0x03C0, 0x0000};
        static const PsxSpriteTemplate kEv3TplFrame_7_1 = {0x50000040, 0x0180, 0x008D, 0x0040, 0x0042, 0x03C0, 0x0001};
        static const PsxSpriteTemplate kEv3TplFrame_8_0 = {0x50000040, 0x0190, 0x008D, 0x0048, 0x0048, 0x03C0, 0x0000};
        static const PsxSpriteTemplate kEv3TplFrame_8_1 = {0x50000040, 0x0190, 0x008D, 0x0048, 0x0048, 0x03C0, 0x0001};

        const bool frameHi = (cursor == 3) && blinkOn;
        const int frameSt = frameHi ? 1 : 0;

        const PsxSpriteTemplate b3_5FrameTpl = (ev3.bottomFrames[0].valid && ev3.bottomFrames[0].tplValid[frameSt]) ? ev3.bottomFrames[0].tpl[frameSt]
                                         : (frameHi ? kEv3TplFrame_5_1 : kEv3TplFrame_5_0);
        const PsxSpriteTemplate b3_6FrameTpl = (ev3.bottomFrames[1].valid && ev3.bottomFrames[1].tplValid[frameSt]) ? ev3.bottomFrames[1].tpl[frameSt]
                                         : (frameHi ? kEv3TplFrame_6_1 : kEv3TplFrame_6_0);
        const PsxSpriteTemplate b3_7FrameTpl = (ev3.bottomFrames[2].valid && ev3.bottomFrames[2].tplValid[frameSt]) ? ev3.bottomFrames[2].tpl[frameSt]
                                         : (frameHi ? kEv3TplFrame_7_1 : kEv3TplFrame_7_0);
        const PsxSpriteTemplate b3_8FrameTpl = (ev3.bottomFrames[3].valid && ev3.bottomFrames[3].tplValid[frameSt]) ? ev3.bottomFrames[3].tpl[frameSt]
                                         : (frameHi ? kEv3TplFrame_8_1 : kEv3TplFrame_8_0);

        const float b3_5FrameX = ev3.bottomFrames[0].valid ? ev3.bottomFrames[0].x : bot5x;
        const float b3_5FrameY = ev3.bottomFrames[0].valid ? ev3.bottomFrames[0].y : bot5y;
        const float b3_6FrameX = ev3.bottomFrames[1].valid ? ev3.bottomFrames[1].x : bot6x;
        const float b3_6FrameY = ev3.bottomFrames[1].valid ? ev3.bottomFrames[1].y : bot6y;
        const float b3_7FrameX = ev3.bottomFrames[2].valid ? ev3.bottomFrames[2].x : bot7x;
        const float b3_7FrameY = ev3.bottomFrames[2].valid ? ev3.bottomFrames[2].y : bot7y;
        const float b3_8FrameX = ev3.bottomFrames[3].valid ? ev3.bottomFrames[3].x : bot8x;
        const float b3_8FrameY = ev3.bottomFrames[3].valid ? ev3.bottomFrames[3].y : bot8y;

        DrawPsxSpriteTemplate(ctx, vx, vy, vs, b3_5FrameX, b3_5FrameY, b3_5FrameTpl, 1, 1, 1, 1, 690);
        DrawPsxSpriteTemplate(ctx, vx, vy, vs, b3_6FrameX, b3_6FrameY, b3_6FrameTpl, 1, 1, 1, 1, 690);
        DrawPsxSpriteTemplate(ctx, vx, vy, vs, b3_7FrameX, b3_7FrameY, b3_7FrameTpl, 1, 1, 1, 1, 690);
        DrawPsxSpriteTemplate(ctx, vx, vy, vs, b3_8FrameX, b3_8FrameY, b3_8FrameTpl, 1, 1, 1, 1, 690);

        auto drawBtn3Icon = [&](int i, int st, const PsxSpriteTemplate& fallbackTpl, float fx, float fy) -> bool {
            if (st < 1) st = 1;
            if (st > 3) st = 3;
            PsxSpriteTemplate tpl = PsxTplWithState(fallbackTpl, st);
            float x = fx;
            float y = fy;
            if (i >= 0 && i < 4) {
                const Ev3Tpl3Layer& icon = ev3.btn3Icons[i];
                if (icon.valid) {
                    x = icon.x;
                    y = icon.y;
                    if (icon.tplValid[st - 1]) {
                        tpl = icon.tpl[st - 1];
                    }
                }
            }
            return DrawPsxSpriteTemplate(ctx, vx, vy, vs, x, y, tpl, 1, 1, 1, 1, 700);
        };

        const PsxSpriteTemplate b3_5BgTpl = (ev3.fixedSprites[0].valid && ev3.fixedSprites[0].tplValid) ? ev3.fixedSprites[0].tpl : kEv3TplBg_MAIN_5;
        const PsxSpriteTemplate b3_6BgTpl = (ev3.fixedSprites[1].valid && ev3.fixedSprites[1].tplValid) ? ev3.fixedSprites[1].tpl : kEv3TplBg_MAIN_6;
        const PsxSpriteTemplate b3_7BgTpl = (ev3.fixedSprites[2].valid && ev3.fixedSprites[2].tplValid) ? ev3.fixedSprites[2].tpl : kEv3TplBg_MAIN_7;
        const PsxSpriteTemplate b3_8BgTpl = (ev3.fixedSprites[3].valid && ev3.fixedSprites[3].tplValid) ? ev3.fixedSprites[3].tpl : kEv3TplBg_MAIN_8;

        const float b3_5BgX = ev3.fixedSprites[0].valid ? ev3.fixedSprites[0].x : txt5x;
        const float b3_5BgY = ev3.fixedSprites[0].valid ? ev3.fixedSprites[0].y : txt5y;
        const float b3_6BgX = ev3.fixedSprites[1].valid ? ev3.fixedSprites[1].x : txt6x;
        const float b3_6BgY = ev3.fixedSprites[1].valid ? ev3.fixedSprites[1].y : txt6y;
        const float b3_7BgX = ev3.fixedSprites[2].valid ? ev3.fixedSprites[2].x : txt7x;
        const float b3_7BgY = ev3.fixedSprites[2].valid ? ev3.fixedSprites[2].y : txt7y;
        const float b3_8BgX = ev3.fixedSprites[3].valid ? ev3.fixedSprites[3].x : txt8x;
        const float b3_8BgY = ev3.fixedSprites[3].valid ? ev3.fixedSprites[3].y : txt8y;

        const bool bgOk5 = drawBtn3Icon(0, st5, b3_5BgTpl, b3_5BgX, b3_5BgY);
        const bool bgOk6 = drawBtn3Icon(1, st6, b3_6BgTpl, b3_6BgX, b3_6BgY);
        const bool bgOk7 = drawBtn3Icon(2, st7, b3_7BgTpl, b3_7BgX, b3_7BgY);
        const bool bgOk8 = drawBtn3Icon(3, st8, b3_8BgTpl, b3_8BgX, b3_8BgY);

        const float b3_5TxX = ev3.tbl_PRACTICE.valid ? ev3.tbl_PRACTICE.x[li] : kSaveBgX[li];
        const float b3_5TxY = ev3.tbl_PRACTICE.valid ? ev3.tbl_PRACTICE.y[li] : kSaveBgY[li];
        const float b3_6TxX = ev3.tbl_STAGE_SELECT.valid ? ev3.tbl_STAGE_SELECT.x[li] : kLoadBgX[li];
        const float b3_6TxY = ev3.tbl_STAGE_SELECT.valid ? ev3.tbl_STAGE_SELECT.y[li] : kLoadBgY[li];
        const float b3_7TxX = ev3.tbl_REPLAY.valid ? ev3.tbl_REPLAY.x[li] : kReplayBgX[li];
        const float b3_7TxY = ev3.tbl_REPLAY.valid ? ev3.tbl_REPLAY.y[li] : kReplayBgY[li];
        const float b3_8TxX = ev3.tbl_LOAD.valid ? ev3.tbl_LOAD.x[li] : kOptBgX[li];
        const float b3_8TxY = ev3.tbl_LOAD.valid ? ev3.tbl_LOAD.y[li] : kOptBgY[li];

        const PsxSpriteTemplate b3_5TextTpl =
            (ev3.tbl_PRACTICE.valid && st5 >= 1 && st5 <= 3 && ev3.tbl_PRACTICE.tplValid[li][st5 - 1])
                ? ev3.tbl_PRACTICE.tpl[li][st5 - 1]
                : PsxTplWithState(kEv3TplText_MAIN_5[li], st5);
        const PsxSpriteTemplate b3_6TextTpl =
            (ev3.tbl_STAGE_SELECT.valid && st6 >= 1 && st6 <= 3 && ev3.tbl_STAGE_SELECT.tplValid[li][st6 - 1])
                ? ev3.tbl_STAGE_SELECT.tpl[li][st6 - 1]
                : PsxTplWithState(kEv3TplText_MAIN_6[li], st6);
        const PsxSpriteTemplate b3_7TextTpl =
            (ev3.tbl_REPLAY.valid && st7 >= 1 && st7 <= 3 && ev3.tbl_REPLAY.tplValid[li][st7 - 1])
                ? ev3.tbl_REPLAY.tpl[li][st7 - 1]
                : PsxTplWithState(kEv3TplText_MAIN_7[li], st7);
        const PsxSpriteTemplate b3_8TextTpl =
            (ev3.tbl_LOAD.valid && st8 >= 1 && st8 <= 3 && ev3.tbl_LOAD.tplValid[li][st8 - 1])
                ? ev3.tbl_LOAD.tpl[li][st8 - 1]
                : PsxTplWithState(kEv3TplText_MAIN_8[li], st8);

        DrawPsxSpriteTemplate(ctx, vx, vy, vs, b3_5TxX, b3_5TxY,
                              b3_5TextTpl,
                              1, 1, 1, 1, 710);
        DrawPsxSpriteTemplate(ctx, vx, vy, vs, b3_6TxX, b3_6TxY,
                              b3_6TextTpl,
                              1, 1, 1, 1, 710);
        DrawPsxSpriteTemplate(ctx, vx, vy, vs, b3_7TxX, b3_7TxY,
                              b3_7TextTpl,
                              1, 1, 1, 1, 710);
        DrawPsxSpriteTemplate(ctx, vx, vy, vs, b3_8TxX, b3_8TxY,
                              b3_8TextTpl,
                              1, 1, 1, 1, 710);

        if (!bgOk5 && !bgOk6 && !bgOk7 && !bgOk8) {
            const bool sel3 = (cursor == 3);
            const bool act = blinkOn || !sel3;
            fallbackBtn(b3_5BgX, b3_5BgY, (float)b3_5BgTpl.w, (float)b3_5BgTpl.h, "SAVE",   sel3, act);
            fallbackBtn(b3_6BgX, b3_6BgY, (float)b3_6BgTpl.w, (float)b3_6BgTpl.h, "LOAD",   sel3, act);
            fallbackBtn(b3_7BgX, b3_7BgY, (float)b3_7BgTpl.w, (float)b3_7BgTpl.h, "REPLAY", sel3, act);
            fallbackBtn(b3_8BgX, b3_8BgY, (float)b3_8BgTpl.w, (float)b3_8BgTpl.h, "OPTION", sel3, act);
        }
    }

    // ===== Button 4: EXIT =====
    {
        const int st = pickState(4);
        const float exitTxX = ev3.tbl_EXIT.valid ? ev3.tbl_EXIT.x[li] : txtEx;
        const float exitTxY = ev3.tbl_EXIT.valid ? ev3.tbl_EXIT.y[li] : txtEy;
        const PsxSpriteTemplate exitTextTpl =
            (ev3.tbl_EXIT.valid && st >= 1 && st <= 3 && ev3.tbl_EXIT.tplValid[li][st - 1])
                ? ev3.tbl_EXIT.tpl[li][st - 1]
                : kEv3TplText_EXIT_0[li];
        const float exitFrameX = ev3.exitFrame.valid ? ev3.exitFrame.x : 231.0f;
        const float exitFrameY = ev3.exitFrame.valid ? ev3.exitFrame.y : 179.0f;
        const int frameSt = (cursor == 4 && blinkOn) ? 1 : 0;
        const PsxSpriteTemplate exitFrameTpl = (ev3.exitFrame.valid && ev3.exitFrame.tplValid[frameSt]) ? ev3.exitFrame.tpl[frameSt]
                                                                                                         : (frameSt ? kEv3TplExitFrame_1 : kEv3TplExitFrame_0);

        // PSX: at cursor==EXIT it draws a frame sprite at (231,179) with a1=1/0 (blink), then draws bg by clutY state.
        DrawPsxSpriteTemplate(ctx, vx, vy, vs, exitFrameX, exitFrameY, exitFrameTpl, 1, 1, 1, 1, 690);

        const float exitBgX = ev3.exitBg.valid ? ev3.exitBg.x : kExitBgX;
        const float exitBgY = ev3.exitBg.valid ? ev3.exitBg.y : kExitBgY;
        const bool bgOk = DrawPsxSpriteTemplate(ctx, vx, vy, vs, exitBgX, exitBgY,
                                                PsxTplWithState(kEv3TplBg_EXIT_0, st),
                                                1, 1, 1, 1, 700);
        DrawPsxSpriteTemplate(ctx, vx, vy, vs,
                              exitTxX,
                              exitTxY,
                              exitTextTpl,
                              1, 1, 1, 1, 710);
        if (!bgOk) {
            fallbackBtn(exitTxX, exitTxY, 56.0f, 18.0f, "EXIT", cursor == 4, blinkOn || cursor != 4);
        }
    }
}

// ======== ev=17 LANGUAGE page: PSX-accurate TIM rendering ========
// memory.md §ev=17: sub_80021910
// Layout: LANGUAGE title (28,36) | SUBTITLE ON/OFF (207,34) | 5-lang arc | EXIT (231,179)
// S0_OLD_DELETE_AFTER_SS0: legacy ev=17 language/subtitle UI renderer.
static void RenderScn0Language(PrGameContext& ctx, float vx, float vy, float vs,
                               const PrEventDispatcherContext& dispCtx) {
    vx += ctx.scn0PanelOffsetX * vs;
    vy += ctx.scn0PanelOffsetY * vs;
    const int cursor = (int)dispCtx.menuIndex;
    const int lang = std::clamp((int)ctx.languageIndex, 0, 4);
    const int selectedLang = std::clamp((int)dispCtx.selections[1], 0, 4);
    const bool subtitleOn = (dispCtx.selections[0] == 0);
    const bool blinkOn = (PrEvent::GetEv17BlinkPtr() && *PrEvent::GetEv17BlinkPtr() != 0);

    const Ev3MenuJsonCache& ev3 = GetEv3MenuJsonCache(ctx.dataRoot);
    const char lc = LangSuffix(lang);

    auto drawEv3Table = [&](const Ev3PosTable& table, int idx, int st, int layer) -> bool {
        float x = 0.0f;
        float y = 0.0f;
        PsxSpriteTemplate tpl = {};
        if (!TryGetEv3PosTableEntry(table, idx, st, x, y, tpl)) return false;
        return DrawPsxSpriteTemplate(ctx, vx, vy, vs, x, y, tpl, 1, 1, 1, 1, layer);
    };

    auto drawFixedTpl = [&](const PracticeUiFixedTemplate& tpl, float x, float y, int layer) -> bool {
        if (!tpl.valid) return false;
        return DrawPsxSpriteTemplate(ctx, vx, vy, vs, x, y, tpl.tpl, 1, 1, 1, 1, layer);
    };
    auto drawTim = [&](const char* name, float x, float y, int layer) {
        DrawTimAutoSize(ctx, vx, vy, vs, name, x, y, 1, 1, 1, 1, layer);
    };

    auto drawEv3Layer3 = [&](const Ev3Tpl3Layer& layer3, int st, int layer) -> bool {
        if (!layer3.valid || st < 1 || st > 3 || !layer3.tplValid[st - 1]) return false;
        return DrawPsxSpriteTemplate(ctx, vx, vy, vs, layer3.x, layer3.y, layer3.tpl[st - 1], 1, 1, 1, 1, layer);
    };

    auto drawEv3Frame2 = [&](const Ev3TopFrame& frame, int st, int layer) -> bool {
        if (!frame.valid || st < 0 || st > 1 || !frame.tplValid[st]) return false;
        return DrawPsxSpriteTemplate(ctx, vx, vy, vs, frame.x, frame.y, frame.tpl[st], 1, 1, 1, 1, layer);
    };

    auto drawEv3ExitFrame = [&](int st, int layer) -> bool {
        if (!ev3.exitFrame.valid || st < 0 || st > 1 || !ev3.exitFrame.tplValid[st]) return false;
        return DrawPsxSpriteTemplate(ctx, vx, vy, vs, ev3.exitFrame.x, ev3.exitFrame.y, ev3.exitFrame.tpl[st], 1, 1, 1, 1, layer);
    };

    // Title uses its own PSX background sprite, not the main-menu topFrames[0] panel.
    drawFixedTpl(kLanguageTitleBg, 36.0f, 45.0f, 700);
    drawEv3Table(ev3.tbl_LANGUAGE, lang, 3, 710);

    // Row 0: SUBTITLE.
    {
        const int topFrameState = (cursor == 0 && blinkOn) ? 1 : 0;
        drawEv3Frame2(ev3.topFrames[2], topFrameState, 700);

        const int stOn = (cursor == 0) ? (subtitleOn ? 3 : 2) : 1;
        const int stOff = (cursor == 0) ? (subtitleOn ? 2 : 3) : 1;
        char texName[32];
        std::snprintf(texName, sizeof(texName), "LANG_2%c%d", lc, stOn);
        drawTim(texName, kLanguageSubtitleOnX[lang], kLanguageSubtitleOnY[lang], 710);
        std::snprintf(texName, sizeof(texName), "LANG_3%c%d", lc, stOff);
        drawTim(texName, kLanguageSubtitleOffX[lang], kLanguageSubtitleOffY[lang], 710);
        drawEv3Layer3(ev3.btn2Deco[0], stOn, 705);
        drawEv3Layer3(ev3.btn2Deco[1], stOff, 705);
    }

    // Row 1: LANGUAGE list.
    {
        const int capState = (cursor == 1 && blinkOn) ? 1 : 0;
        drawFixedTpl(kLanguageRowLeft[capState], 26.0f, 106.0f, 700);
        drawFixedTpl(kLanguageRowRight[capState], 162.0f, 106.0f, 700);

        for (int i = 0; i < 5; i++) {
            const int st = (cursor == 1) ? ((i == selectedLang) ? 3 : 2) : 1;
            char texName[32];
            std::snprintf(texName, sizeof(texName), "LANG_%dB%d", i + 4, st);
            drawTim(texName, kLanguageBgX[i], kLanguageBgY[i], 700);
            std::snprintf(texName, sizeof(texName), "LANG_%dE%d", i + 4, st);
            drawTim(texName, kLanguageTextX[i], kLanguageTextY[i], 710);
        }
    }

    // Row 2: EXIT.
    {
        const int frameState = (cursor == 2 && blinkOn) ? 1 : 0;
        const int textState = (cursor == 2) ? (dispCtx.confirmFlag ? 3 : 2) : 1;
        const int barState = textState - 1;
        drawEv3ExitFrame(frameState, 700);
        drawEv3Table(ev3.tbl_EXIT, lang, textState, 710);
        if (barState >= 0 && barState <= 2) {
            drawFixedTpl(kLanguageExitBar[barState], 238.0f, 188.0f, 705);
        }
    }
}

// ======== ev=5 PRACTICE page: PSX TIM rendering ========
// memory.md §ev=5: sub_80020BE4
// Layout: title (36,28) | hint (117,64) | 57-element dot strip | EXIT (231,179)
static void RenderScn0Practice(PrGameContext& ctx, float vx, float vy, float vs,
                                const PrEventDispatcherContext& dispCtx) {
    vx += ctx.scn0PanelOffsetX * vs;
    vy += ctx.scn0PanelOffsetY * vs;
    const int lang = (int)ctx.languageIndex;
    const char lc = LangSuffix(lang);
    const bool blinkOn = (((int)ctx.frame / 20) & 1) == 0;
    char b[32], t[32];

    // === 1. PRACTICE title at (36, 28) — reuses MAIN_5 button ===
    {
        const int st = 2; // always show focused state for page title
        std::snprintf(b, sizeof(b), "MAIN_5B%d", st);
        std::snprintf(t, sizeof(t), "MAIN_5%c%d", lc, st);
        DrawTimAutoSize(ctx, vx, vy, vs, b, 36.0f, 28.0f, 1, 1, 1, 1, 700);
        TextureResource* tb = ctx.resources ? ctx.resources->GetTexture(b) : nullptr;
        TextureResource* tt = ctx.resources ? ctx.resources->GetTexture(t) : nullptr;
        if (tb && tt) {
            const float x = 36.0f + ((float)tb->tim.width - (float)tt->tim.width) * 0.5f;
            const float y = 28.0f + ((float)tb->tim.height - (float)tt->tim.height) * 0.5f;
            DrawTimAutoSize(ctx, vx, vy, vs, t, x, y, 1, 1, 1, 1, 710);
        } else if (!tb) {
            DrawRectUI(s_renderer, vx, vy, vs, 34.0f, 26.0f, 76.0f, 20.0f,
                      0.85f, 0.75f, 0.15f, 0.9f);
            DrawString(s_renderer, ToScreenX(vx,vs,40), ToScreenY(vy,vs,30), 1.2f*vs,
                      "PRACTICE", 0.1f, 0.1f, 0.1f, 1.0f);
        } else {
            DrawTimAutoSize(ctx, vx, vy, vs, t, 36.0f, 28.0f, 1, 1, 1, 1, 710);
        }
    }

    // === 2. Hint banner at (117, 64) — MAIN_5W{state} (wide hint) ===
    {
        std::snprintf(b, sizeof(b), "MAIN_5W2");
        DrawTimAutoSize(ctx, vx, vy, vs, b, 117.0f, 64.0f, 1, 1, 1, 1, 690);

        // Fallback hint rectangle
        if (!ctx.resources || !ctx.resources->GetTexture(b)) {
            DrawRectUI(s_renderer, vx, vy, vs, 115.0f, 62.0f, 180.0f, 36.0f,
                      0.95f, 0.90f, 0.20f, 0.9f);
            DrawString(s_renderer, ToScreenX(vx,vs,122), ToScreenY(vy,vs,70), 0.9f*vs,
                      "Look at the bar,", 0.1f, 0.1f, 0.1f, 1.0f);
            DrawString(s_renderer, ToScreenX(vx,vs,122), ToScreenY(vy,vs,82), 0.9f*vs,
                      "press the button.", 0.1f, 0.1f, 0.1f, 1.0f);
        }
    }

    // === 3. Dot strip placeholder — 57 elements across (dword_800532B0) ===
    {
        const float stripY = 72.0f;
        const float stripStartX = 30.0f;
        const float dotSpacing = 4.5f;
        for (int i = 0; i < 57; i++) {
            const float dx = stripStartX + i * dotSpacing;
            DrawRectUI(s_renderer, vx, vy, vs, dx, stripY, 3.0f, 3.0f,
                      0.45f, 0.40f, 0.35f, 0.6f);
        }
    }

    // === 4. EXIT at bottom-right ===
    {
        const int st = blinkOn ? 3 : 2;

        std::snprintf(b, sizeof(b), "EXIT_0B%d", st);
        DrawTimAutoSize(ctx, vx, vy, vs, b, 231.0f, 179.0f, 1, 1, 1, 1, 700);

        static const float kExitTxX[] = {242.0f, 238.0f, 238.0f, 241.0f, 239.0f};
        static const float kExitTxY[] = {191.0f, 193.0f, 193.0f, 190.0f, 191.0f};
        std::snprintf(t, sizeof(t), "EXIT_0%c%d", lc, st);
        DrawTimAutoSize(ctx, vx, vy, vs, t, kExitTxX[lang], kExitTxY[lang], 1, 1, 1, 1, 710);

        if (!ctx.resources || !ctx.resources->GetTexture(b)) {
            DrawRectUI(s_renderer, vx, vy, vs, 229.0f, 177.0f, 56.0f, 16.0f,
                      0.5f, 0.4f, 0.05f, 0.8f);
            DrawString(s_renderer, ToScreenX(vx,vs,233), ToScreenY(vy,vs,181), 1.0f*vs,
                      "EXIT", 0.8f, 0.8f, 0.8f, 1.0f);
        }
    }
}

struct MemCardUiTextTable {
    bool valid = false;
    float x[5] = {};
    float y[5] = {};
    uint32_t tplAddr[5] = {};
    PsxSpriteTemplate tpl[5] = {};
    bool tplValid[5] = {};
};

struct MemCardGlyphMetric {
    int16_t u = -1;
    int16_t v = -1;
    uint8_t w = 0;
    uint8_t h = 0;
    int16_t xOff = 0;
    bool valid = false;
};

struct MemCardUiJsonPage {
    MemCardUiTextTable titleText;
    MemCardUiTextTable hintText;
    MemCardUiTextTable auxText0;
    MemCardUiTextTable auxText1;
    MemCardUiTextTable auxText2;
};

struct MemCardUiJsonCache {
    bool loaded = false;
    bool ok = false;
    std::filesystem::path loadedRoot;

    MemCardUiJsonPage save;
    MemCardUiJsonPage load;
    MemCardUiJsonPage replay;

    PracticeUiFixedTemplate titleBg;
    PracticeUiFixedTemplate hintBg;
    PracticeUiFixedTemplate gridMarker[3];
    PracticeUiFixedTemplate exitFrameOff;
    PracticeUiFixedTemplate exitFrameOn;
    PracticeUiFixedTemplate exitBar[3];
    PracticeUiFixedTemplate ioCornerLT;
    PracticeUiFixedTemplate ioCornerLB;
    PracticeUiFixedTemplate ioCornerRT;
    PracticeUiFixedTemplate ioCornerRB;
    PracticeUiLangTable3 exitText;
    std::string ioText0[5];
    std::string ioText1[5];
    std::string ioText2[5];
    float exitBarX = 238.0f;
    float exitBarY = 188.0f;
    MemCardGlyphMetric slotGlyph[256];
};

struct MemCardFontAtlasVariant {
    ID3D11ShaderResourceView* srv = nullptr;
    uint32_t width = 0;
    uint32_t height = 0;
    int clutY = -1;
    bool valid = false;
};

struct MemCardFontAtlasCache {
    bool loaded = false;
    std::filesystem::path loadedRoot;
    uint32_t resourceGeneration = 0;
    MemCardFontAtlasVariant variants[5][3];
};

static void ResetMemCardFontAtlasVariant(MemCardFontAtlasVariant& v) {
    v.srv = nullptr;
    v.width = 0;
    v.height = 0;
    v.clutY = -1;
    v.valid = false;
}

static MemCardFontAtlasCache& GetMemCardFontAtlasCache(PrGameContext& ctx) {
    static MemCardFontAtlasCache s_cache;
    const uint32_t gen = ctx.resources ? ctx.resources->GetGeneration() : 0u;
    if (!s_cache.loaded || s_cache.loadedRoot != ctx.dataRoot || s_cache.resourceGeneration != gen) {
        for (int li = 0; li < 5; ++li) {
            for (int vi = 0; vi < 3; ++vi) {
                ResetMemCardFontAtlasVariant(s_cache.variants[li][vi]);
            }
        }
        s_cache = MemCardFontAtlasCache{};
        s_cache.loaded = true;
        s_cache.loadedRoot = ctx.dataRoot;
        s_cache.resourceGeneration = gen;
    }
    return s_cache;
}

static const MemCardFontAtlasVariant* GetMemCardFontAtlasVariant(PrGameContext& ctx, int clutY) {
    if (!ctx.renderer || !ctx.resources) {
        return nullptr;
    }

    int variantIdx = 0;
    if (clutY == 480) variantIdx = 0;
    else if (clutY == 482) variantIdx = 1;
    else if (clutY == 483) variantIdx = 2;
    else return nullptr;
    const int wantedClutY = clutY;
    const char* atlasName = nullptr;
    switch (wantedClutY) {
        case 480: atlasName = "EU1_256"; break;
        case 481: atlasName = "EU2_256"; break;
        case 482: atlasName = "EU3_256"; break;
        case 483: atlasName = "EU4_256"; break;
        default: return nullptr;
    }

    MemCardFontAtlasCache& cache = GetMemCardFontAtlasCache(ctx);
    MemCardFontAtlasVariant& variant = cache.variants[0][variantIdx];
    if (variant.valid && variant.srv && variant.clutY == wantedClutY) {
        return &variant;
    }

    ResetMemCardFontAtlasVariant(variant);

    TextureResource* base = ctx.resources->GetTexture(atlasName);
    if (!base) {
        return nullptr;
    }
    TextureResource* tr = ctx.resources->FindTextureByTimHeader(
        (int)base->tim.bpp,
        base->tim.orgX,
        base->tim.orgY,
        base->tim.width,
        base->tim.height,
        base->tim.clutX,
        (int16_t)wantedClutY);
    if (!tr) {
        return nullptr;
    }
    if (!tr->srv && !tr->tim.rgba.empty()) {
        tr->srv = ctx.renderer->CreateTexture(tr->tim.rgba.data(), tr->tim.width, tr->tim.height);
    }
    if (!tr->srv || tr->tim.width == 0 || tr->tim.height == 0) {
        return nullptr;
    }
    variant.srv = tr->srv;
    variant.width = tr->tim.width;
    variant.height = tr->tim.height;
    variant.clutY = wantedClutY;
    variant.valid = true;
    Log::Printf("MemCardFontAtlas: atlas=%s clutY=%d wh=%ux%u",
                atlasName, wantedClutY,
                (unsigned)variant.width, (unsigned)variant.height);
    return &variant;
}

static bool ParseMemCardUiTextTable(const std::string& json, const char* tableName, MemCardUiTextTable& out) {
    out = MemCardUiTextTable{};
    size_t p = FindNamedJsonEntries(json, tableName);
    if (p == std::string::npos) return false;

    bool seen[5] = {false, false, false, false, false};
    int seenCount = 0;
    while (p < json.size()) {
        p = SkipWs(json, p);
        if (p >= json.size() || json[p] == ']') break;
        if (json[p] == ',') {
            p++;
            continue;
        }
        if (json[p] != '{') {
            p++;
            continue;
        }

        const size_t endBrace = json.find('}', p);
        if (endBrace == std::string::npos) break;
        const std::string obj = json.substr(p, endBrace - p + 1);

        size_t pl = FindKey(obj, 0, "lang");
        size_t pt = FindKey(obj, 0, "tpl");
        size_t px = FindKey(obj, 0, "x");
        size_t py = FindKey(obj, 0, "y");
        if (pl != std::string::npos && pt != std::string::npos &&
            px != std::string::npos && py != std::string::npos) {
            const int lang = ParseInt(obj, pl);
            uint32_t addr = 0;
            if (lang >= 0 && lang < 5 && ParseQuotedHexU32(obj, pt, addr)) {
                if (!seen[lang]) {
                    seen[lang] = true;
                    seenCount++;
                }
                out.tplAddr[lang] = addr;
                out.x[lang] = (float)ParseInt(obj, px);
                out.y[lang] = (float)ParseInt(obj, py);
            }
        }

        p = endBrace + 1;
    }

    if (seenCount != 5) return false;
    out.valid = true;
    bool ok = true;
    for (int lang = 0; lang < 5; lang++) {
        out.tplValid[lang] = ParseSpriteTemplateByAddr(json, out.tplAddr[lang], out.tpl[lang]);
        ok &= out.tplValid[lang];
    }
    return ok;
}

static bool ParseMemCardIoTextTable(const std::string& json, const char* tableName, std::string out[5]) {
    for (int lang = 0; lang < 5; lang++) {
        out[lang].clear();
    }

    size_t p = FindKey(json, 0, tableName);
    if (p == std::string::npos) return false;
    p = json.find('[', p);
    if (p == std::string::npos) return false;
    p++;

    bool seen[5] = {false, false, false, false, false};
    int seenCount = 0;
    while (p < json.size()) {
        p = SkipWs(json, p);
        if (p >= json.size() || json[p] == ']') break;
        if (json[p] == ',') {
            p++;
            continue;
        }
        if (json[p] != '{') {
            p++;
            continue;
        }

        const size_t endBrace = json.find('}', p);
        if (endBrace == std::string::npos) break;
        const std::string obj = json.substr(p, endBrace - p + 1);

        size_t pl = FindKey(obj, 0, "lang");
        size_t pb = FindKey(obj, 0, "bytes");
        if (pl != std::string::npos && pb != std::string::npos) {
            const int lang = ParseInt(obj, pl);
            size_t q = obj.find('[', pb);
            if (lang >= 0 && lang < 5 && q != std::string::npos) {
                q++;
                std::string bytes;
                while (q < obj.size()) {
                    q = SkipWs(obj, q);
                    if (q >= obj.size() || obj[q] == ']') break;
                    if (obj[q] == ',') {
                        q++;
                        continue;
                    }
                    const int b = ParseInt(obj, q);
                    if (b >= 0 && b <= 255) {
                        bytes.push_back((char)(uint8_t)b);
                    }
                }
                if (!seen[lang]) {
                    seen[lang] = true;
                    seenCount++;
                }
                out[lang] = bytes;
            }
        }

        p = endBrace + 1;
    }

    return seenCount == 5;
}

static bool ParseMemCardExitBarPos(const std::string& json, float& outX, float& outY) {
    size_t p = FindKey(json, 0, "exit_bar_pos");
    if (p == std::string::npos) return false;
    const size_t endBrace = json.find('}', p);
    if (endBrace == std::string::npos || endBrace <= p) return false;
    const std::string obj = json.substr(p, endBrace - p + 1);

    size_t px = FindKey(obj, 0, "x");
    size_t py = FindKey(obj, 0, "y");
    if (px == std::string::npos || py == std::string::npos) return false;
    outX = (float)ParseInt(obj, px);
    outY = (float)ParseInt(obj, py);
    return true;
}

static bool ParseMemCardGlyphMetrics(const std::string& json, MemCardGlyphMetric out[256]) {
    for (int i = 0; i < 256; i++) {
        out[i] = MemCardGlyphMetric{};
    }

    size_t p = FindKey(json, 0, "glyph_metrics");
    if (p == std::string::npos) return false;
    p = FindKey(json, p, "entries");
    if (p == std::string::npos) return false;
    p = json.find('[', p);
    if (p == std::string::npos) return false;
    p++;

    bool seen[256] = {};
    int seenCount = 0;
    while (p < json.size()) {
        p = SkipWs(json, p);
        if (p >= json.size() || json[p] == ']') break;
        if (json[p] == ',') {
            p++;
            continue;
        }
        if (json[p] != '{') {
            p++;
            continue;
        }

        const size_t endBrace = json.find('}', p);
        if (endBrace == std::string::npos) break;
        const std::string obj = json.substr(p, endBrace - p + 1);

        size_t pi = FindKey(obj, 0, "index");
        size_t pu = FindKey(obj, 0, "u");
        size_t pv = FindKey(obj, 0, "v");
        size_t pw = FindKey(obj, 0, "w");
        size_t ph = FindKey(obj, 0, "h");
        size_t px = FindKey(obj, 0, "xOff");
        if (pi != std::string::npos && pu != std::string::npos && pv != std::string::npos &&
            pw != std::string::npos && ph != std::string::npos && px != std::string::npos) {
            const int index = ParseInt(obj, pi);
            const int u = ParseInt(obj, pu);
            const int v = ParseInt(obj, pv);
            const int w = ParseInt(obj, pw);
            const int h = ParseInt(obj, ph);
            const int xOff = ParseInt(obj, px);
            if (index >= 0 && index < 256 && u >= -32768 && u <= 32767 &&
                v >= -32768 && v <= 32767 && w >= 0 && w <= 255 && h >= 0 && h <= 255 &&
                xOff >= -32768 && xOff <= 32767) {
                if (!seen[index]) {
                    seen[index] = true;
                    seenCount++;
                }
                MemCardGlyphMetric gm{};
                gm.u = (int16_t)u;
                gm.v = (int16_t)v;
                gm.w = (uint8_t)w;
                gm.h = (uint8_t)h;
                gm.xOff = (int16_t)xOff;
                gm.valid = (gm.u >= 0 && gm.v >= 0 && gm.w > 0 && gm.h > 0);
                out[index] = gm;
            }
        }

        p = endBrace + 1;
    }

    return seenCount == 256;
}

static bool AreMemCardFixedTemplatesValid(const MemCardUiJsonCache& cache) {
    const PracticeUiFixedTemplate* fixed[] = {
        &cache.titleBg,
        &cache.hintBg,
        &cache.gridMarker[0],
        &cache.gridMarker[1],
        &cache.gridMarker[2],
        &cache.exitFrameOff,
        &cache.exitFrameOn,
        &cache.exitBar[0],
        &cache.exitBar[1],
        &cache.exitBar[2],
        &cache.ioCornerLT,
        &cache.ioCornerLB,
        &cache.ioCornerRT,
        &cache.ioCornerRB,
    };
    for (const PracticeUiFixedTemplate* tpl : fixed) {
        if (!tpl || !tpl->valid) return false;
    }
    return true;
}

static const MemCardUiJsonCache& GetMemCardUiJsonCache(const std::filesystem::path& dataRoot) {
    static MemCardUiJsonCache s_cache;
    if (!s_cache.loaded || s_cache.loadedRoot != dataRoot) {
        s_cache = MemCardUiJsonCache{};
        s_cache.loaded = true;
        s_cache.loadedRoot = dataRoot;

        const std::filesystem::path p = dataRoot / "win" / "ex" / "json" / "psx_memcard_ui_constants.json";
        const std::string json = ReadFileToString(p);
        if (json.empty()) {
            Log::Printf("MemCardUiJson: missing or empty: %s", p.string().c_str());
            return s_cache;
        }

        bool ok = true;
        ok &= ParseMemCardUiTextTable(json, "SAVE_TITLETEXT", s_cache.save.titleText);
        ok &= ParseMemCardUiTextTable(json, "SAVE_HINTTEXT", s_cache.save.hintText);
        ok &= ParseMemCardUiTextTable(json, "SAVE_AUXTEXT0", s_cache.save.auxText0);
        ok &= ParseMemCardUiTextTable(json, "SAVE_AUXTEXT1", s_cache.save.auxText1);
        ok &= ParseMemCardUiTextTable(json, "SAVE_AUXTEXT2", s_cache.save.auxText2);
        ok &= ParseMemCardUiTextTable(json, "LOAD_TITLETEXT", s_cache.load.titleText);
        ok &= ParseMemCardUiTextTable(json, "LOAD_HINTTEXT", s_cache.load.hintText);
        ok &= ParseMemCardUiTextTable(json, "LOAD_AUXTEXT0", s_cache.load.auxText0);
        ok &= ParseMemCardUiTextTable(json, "LOAD_AUXTEXT1", s_cache.load.auxText1);
        ok &= ParseMemCardUiTextTable(json, "LOAD_AUXTEXT2", s_cache.load.auxText2);
        ok &= ParseMemCardUiTextTable(json, "REPLAY_TITLETEXT", s_cache.replay.titleText);
        ok &= ParseMemCardUiTextTable(json, "REPLAY_HINTTEXT", s_cache.replay.hintText);
        ok &= ParseMemCardUiTextTable(json, "REPLAY_AUXTEXT0", s_cache.replay.auxText0);
        ok &= ParseMemCardUiTextTable(json, "REPLAY_AUXTEXT1", s_cache.replay.auxText1);
        ok &= ParseMemCardUiTextTable(json, "REPLAY_AUXTEXT2", s_cache.replay.auxText2);

        ok &= ParsePracticeUiFixed(json, "titleBg", s_cache.titleBg);
        ok &= ParsePracticeUiFixed(json, "hintBg", s_cache.hintBg);
        ok &= ParsePracticeUiFixed(json, "gridMarker0", s_cache.gridMarker[0]);
        ok &= ParsePracticeUiFixed(json, "gridMarker1", s_cache.gridMarker[1]);
        ok &= ParsePracticeUiFixed(json, "gridMarker2", s_cache.gridMarker[2]);
        ok &= ParsePracticeUiFixed(json, "exitFrameOff", s_cache.exitFrameOff);
        ok &= ParsePracticeUiFixed(json, "exitFrameOn", s_cache.exitFrameOn);
        ok &= ParsePracticeUiFixed(json, "exitBar0", s_cache.exitBar[0]);
        ok &= ParsePracticeUiFixed(json, "exitBar1", s_cache.exitBar[1]);
        ok &= ParsePracticeUiFixed(json, "exitBar2", s_cache.exitBar[2]);
        ok &= ParsePracticeUiFixed(json, "ioCornerLT", s_cache.ioCornerLT);
        ok &= ParsePracticeUiFixed(json, "ioCornerLB", s_cache.ioCornerLB);
        ok &= ParsePracticeUiFixed(json, "ioCornerRT", s_cache.ioCornerRT);
        ok &= ParsePracticeUiFixed(json, "ioCornerRB", s_cache.ioCornerRB);
        ok &= ParsePracticeUiLangTable3(json, "EXIT_TEXT", s_cache.exitText);
        ok &= ParseMemCardIoTextTable(json, "ioText0", s_cache.ioText0);
        ok &= ParseMemCardIoTextTable(json, "ioText1", s_cache.ioText1);
        ok &= ParseMemCardIoTextTable(json, "ioText2", s_cache.ioText2);
        ok &= ParseMemCardExitBarPos(json, s_cache.exitBarX, s_cache.exitBarY);
        ok &= ParseMemCardGlyphMetrics(json, s_cache.slotGlyph);
        ok &= AreMemCardFixedTemplatesValid(s_cache);

        s_cache.ok = ok;
        Log::Printf("MemCardUiJson: loaded=%d ok=%d", 1, ok ? 1 : 0);
    }
    return s_cache;
}

static bool DrawMemCardTextNative(PrGameContext& ctx, const MemCardUiJsonCache& ui,
                                      float x, float y, float scale, const char* text, int clutY,
                                      float r, float g, float b, float a) {
    if (!ctx.renderer || !ctx.resources || !text) {
        return false;
    }

    const MemCardFontAtlasVariant* atlas = GetMemCardFontAtlasVariant(ctx, clutY);
    if (!atlas || !atlas->srv || atlas->width == 0 || atlas->height == 0) {
        return false;
    }

    const float texW = (float)atlas->width;
    const float texH = (float)atlas->height;
    float cx = x;
    bool drew = false;
    for (const char* p = text; *p; ++p) {
        const unsigned char uc = (unsigned char)*p;
        const MemCardGlyphMetric& gm = ui.slotGlyph[uc];
        const float advance = std::round(((gm.w > 0) ? (float)gm.w : 5.0f) * scale);
        if (!gm.valid) {
            cx += advance;
            drew = true;
            continue;
        }
        const int srcU = (gm.u > 0) ? (gm.u - 1) : gm.u;

        D3D11Renderer::SpriteCmd cmd;
        cmd.texture = atlas->srv;
        cmd.x = std::round(cx + (float)gm.xOff * scale);
        cmd.y = std::round(y - 3.0f * scale);
        cmd.w = std::round((float)gm.w * scale);
        cmd.h = std::round((float)gm.h * scale);
        cmd.u0 = (float)srcU / texW;
        cmd.v0 = (float)(gm.v & 0xFF) / texH;
        cmd.u1 = ((float)srcU + (float)gm.w) / texW;
        cmd.v1 = ((float)(gm.v & 0xFF) + (float)gm.h) / texH;
        cmd.r = r;
        cmd.g = g;
        cmd.b = b;
        cmd.a = a;
        cmd.blend = D3D11Renderer::BlendMode::Alpha;
        cmd.layer = 708;
        cmd.order = 0;
        ctx.renderer->SubmitSprite(cmd);

        cx += advance;
        drew = true;
    }
    return drew;
}

static float MeasureMemCardTextNative(const MemCardUiJsonCache& ui, const char* text) {
    if (!text) {
        return 0.0f;
    }
    float w = 0.0f;
    for (const char* p = text; *p; ++p) {
        const unsigned char uc = (unsigned char)*p;
        const MemCardGlyphMetric& gm = ui.slotGlyph[uc];
        w += std::round((gm.w > 0) ? (float)gm.w : 5.0f);
    }
    return w;
}

struct SaveConfirmStaticText {
    float x = 0.0f;
    float y = 0.0f;
    PsxSpriteTemplate tpl = {};
};

struct SaveConfirmStaticChoice {
    float x = 0.0f;
    float y = 0.0f;
    PsxSpriteTemplate tpl[2] = {};
};

static bool RenderScn0SaveConfirmType4(PrGameContext& ctx, float vx, float vy, float vs,
                                       const PrEventDispatcherContext& dispCtx) {
    vx += ctx.scn0PanelOffsetX * vs;
    vy += ctx.scn0PanelOffsetY * vs;

    static const PracticeUiFixedTemplate kTitleIcon = {
        true, 0x80052320, {0x50000040, 0x02C0, 0x0088, 0x00AC, 0x0054, 0x03F0, 0x00D6}};
    static const PracticeUiFixedTemplate kPromptPanel = {
        true, 0x80052350, {0x51000040, 0x0340, 0x0000, 0x0060, 0x0086, 0x0300, 0x00FF}};
    static const PracticeUiFixedTemplate kConfirmFrame[2] = {
        {true, 0x800526A0, {0x50000040, 0x02EE, 0x0000, 0x0044, 0x003F, 0x03F0, 0x00CE}},
        {true, 0x800526B0, {0x50000040, 0x02EE, 0x0000, 0x0044, 0x003F, 0x03F0, 0x00CF}},
    };
    static const PracticeUiFixedTemplate kYesMarker[2] = {
        {true, 0x80052590, {0x50000040, 0x02EE, 0x003F, 0x0030, 0x0015, 0x03F0, 0x00C9}},
        {true, 0x800525A0, {0x50000040, 0x02EE, 0x003F, 0x0030, 0x0015, 0x03F0, 0x00CA}},
    };
    static const PracticeUiFixedTemplate kNoMarker[2] = {
        {true, 0x800526C0, {0x50000040, 0x02EE, 0x0054, 0x0030, 0x0015, 0x03F0, 0x00D1}},
        {true, 0x800526D0, {0x50000040, 0x02EE, 0x0054, 0x0030, 0x0015, 0x03F0, 0x00D2}},
    };
    static const SaveConfirmStaticText kPromptText[5] = {
        {185.0f, 71.0f, {0x50000040, 0x0221, 0x01B7, 0x0028, 0x000E, 0x03F0, 0x0032}},
        {170.0f, 69.0f, {0x50000040, 0x0225, 0x0175, 0x0048, 0x0012, 0x03F0, 0x0032}},
        {160.0f, 69.0f, {0x50000040, 0x0225, 0x0146, 0x005C, 0x0012, 0x03F0, 0x0032}},
        {162.0f, 71.0f, {0x50000040, 0x0225, 0x0158, 0x005C, 0x000D, 0x03F0, 0x0032}},
        {167.0f, 71.0f, {0x50000040, 0x0225, 0x0165, 0x004C, 0x0010, 0x03F0, 0x0032}},
    };
    static const SaveConfirmStaticChoice kYesText[5] = {
        {237.0f, 162.0f, {{0x50000040, 0x0330, 0x0000, 0x001C, 0x000E, 0x03F0, 0x00CC},
                           {0x50000040, 0x0330, 0x0000, 0x001C, 0x000E, 0x03F0, 0x00CD}}},
        {242.0f, 163.0f, {{0x50000040, 0x0330, 0x000E, 0x0014, 0x000D, 0x03F0, 0x00CC},
                           {0x50000040, 0x0330, 0x000E, 0x0014, 0x000D, 0x03F0, 0x00CD}}},
        {237.0f, 163.0f, {{0x50000040, 0x0330, 0x001B, 0x001C, 0x000D, 0x03F0, 0x00CC},
                           {0x50000040, 0x0330, 0x001B, 0x001C, 0x000D, 0x03F0, 0x00CD}}},
        {241.0f, 160.0f, {{0x50000040, 0x0330, 0x0028, 0x0014, 0x0012, 0x03F0, 0x00CC},
                           {0x50000040, 0x0330, 0x0028, 0x0014, 0x0012, 0x03F0, 0x00CD}}},
        {241.0f, 160.0f, {{0x50000040, 0x0330, 0x003A, 0x0014, 0x0012, 0x03F0, 0x00CC},
                           {0x50000040, 0x0330, 0x003A, 0x0014, 0x0012, 0x03F0, 0x00CD}}},
    };
    static const SaveConfirmStaticChoice kNoText[5] = {
        {241.0f, 185.0f, {{0x50000040, 0x0370, 0x0000, 0x0014, 0x000E, 0x03F0, 0x00D4},
                           {0x50000040, 0x0370, 0x0000, 0x0014, 0x000E, 0x03F0, 0x00D5}}},
        {234.0f, 185.0f, {{0x50000040, 0x0370, 0x000E, 0x0020, 0x000E, 0x03F0, 0x00D4},
                           {0x50000040, 0x0370, 0x000E, 0x0020, 0x000E, 0x03F0, 0x00D5}}},
        {236.0f, 185.0f, {{0x50000040, 0x0370, 0x001C, 0x001C, 0x000E, 0x03F0, 0x00D4},
                           {0x50000040, 0x0370, 0x001C, 0x001C, 0x000E, 0x03F0, 0x00D5}}},
        {241.0f, 185.0f, {{0x50000040, 0x0370, 0x0000, 0x0014, 0x000E, 0x03F0, 0x00D4},
                           {0x50000040, 0x0370, 0x0000, 0x0014, 0x000E, 0x03F0, 0x00D5}}},
        {241.0f, 185.0f, {{0x50000040, 0x0370, 0x0000, 0x0014, 0x000E, 0x03F0, 0x00D4},
                           {0x50000040, 0x0370, 0x0000, 0x0014, 0x000E, 0x03F0, 0x00D5}}},
    };

    const int lang = std::clamp((int)ctx.languageIndex, 0, 4);
    int selected = (int)dispCtx.selections[0];
    if (selected != 1 && selected != 2) {
        selected = ((int)dispCtx.menuIndex == 1) ? 2 : 1;
    }
    const int yesState = (selected == 1) ? 1 : 0;
    const int noState = (selected == 2) ? 1 : 0;
    const int frameState = (dispCtx.confirmFlag != 0) ? 0 : 1;

    auto drawFixed = [&](const PracticeUiFixedTemplate& tpl, float x, float y, int layer) -> bool {
        if (!tpl.valid) return false;
        return DrawPsxSpriteTemplate(ctx, vx, vy, vs, x, y, tpl.tpl, 1, 1, 1, 1, layer);
    };
    auto drawText = [&](const SaveConfirmStaticText& item, int layer) -> bool {
        return DrawPsxSpriteTemplate(ctx, vx, vy, vs, item.x, item.y, item.tpl, 1, 1, 1, 1, layer);
    };
    auto drawChoice = [&](const SaveConfirmStaticChoice& item, int state, int layer) -> bool {
        const int st = std::clamp(state, 0, 1);
        return DrawPsxSpriteTemplate(ctx, vx, vy, vs, item.x, item.y, item.tpl[st], 1, 1, 1, 1, layer);
    };

    bool drew = false;
    drew |= drawFixed(kPromptPanel, 28.0f, 56.0f, 700);
    drew |= drawFixed(kConfirmFrame[frameState], 224.0f, 149.0f, 701);
    drew |= drawFixed(kYesMarker[yesState], 234.0f, 159.0f, 702);
    drew |= drawFixed(kNoMarker[noState], 234.0f, 182.0f, 702);
    drew |= drawText(kPromptText[lang], 710);
    drew |= drawChoice(kYesText[lang], yesState, 711);
    drew |= drawChoice(kNoText[lang], noState, 711);
    drew |= drawFixed(kTitleIcon, 121.0f, 36.0f, 720);
    return drew;
}

bool RenderEvent4PromptNative(PrGameContext& ctx, int selectionState) {
    if (!ctx.renderer) {
        return false;
    }

    float vx = 0.0f;
    float vy = 0.0f;
    float vs = 1.0f;
    CalcPs1Viewport(ctx.renderer, vx, vy, vs);

    PrSS0Event4PromptRenderDirect::Event4PromptInput800203D4 input{};
    input.requestBound = true;
    input.choiceSourceKnown = true;
    input.ctx0 = selectionState;
    const PrSS0Event4PromptRenderDirect::Event4PromptDrawList800203D4
        drawList =
            PrSS0Event4PromptRenderDirect::
                BuildEvent4PromptDrawList800203D4(input);
    if (!drawList.accepted || !drawList.complete ||
        !drawList.visibleColorIndependentOfRgb) {
        return false;
    }
    bool drew = false;
    for (std::size_t i = 0u; i < drawList.count; ++i) {
        const PrSS0Event4PromptRenderDirect::Event4PromptSprite800203D4&
            sprite = drawList.sprites[i];
        if (!sprite.known) {
            return false;
        }
        const PsxSpriteTemplate tpl = {
            sprite.attr,
            sprite.texX,
            sprite.texY,
            sprite.width,
            sprite.height,
            sprite.clutX,
            sprite.clutY};
        const int hostPriority = i == 0u ? 730 : 731;
        drew |= DrawPsxSpriteTemplate(
            ctx,
            vx,
            vy,
            vs,
            static_cast<float>(sprite.screenX),
            static_cast<float>(sprite.screenY),
            tpl,
            1,
            1,
            1,
            1,
            hostPriority);
    }
    return drew;
}

// S0_OLD_DELETE_AFTER_SS0: legacy ev=7/8/9 memcard UI renderer.
static void RenderScn0MemCard(PrGameContext& ctx, float vx, float vy, float vs,
                                const PrEventDispatcherContext& dispCtx, int eventId) {
    vx += ctx.scn0PanelOffsetX * vs;
    vy += ctx.scn0PanelOffsetY * vs;

    const MemCardUiJsonCache& ui = GetMemCardUiJsonCache(ctx.dataRoot);
    const PrEvMemCardCtx* mc = PrEvent::GetEvMemCardCtxPtr();
    const MemCardUiJsonPage* page = nullptr;
    if (eventId == 7) page = &ui.save;
    else if (eventId == 8) page = &ui.load;
    else page = &ui.replay;

    const int lang = std::clamp((int)ctx.languageIndex, 0, 4);
    const int rows = mc ? std::max(0, (int)mc->rows) : 5;
    const int cols = mc ? std::max(0, (int)mc->cols) : 3;
    const int itemCount = mc ? std::max(0, (int)mc->itemCount) : 16;
    const int selected = mc ? (int)mc->selected : (int)dispCtx.menuIndex;
    const int exitIndex = rows * cols;
    const int slotCount = std::min(exitIndex, std::max(0, itemCount));
    const int exitTextOn = mc ? mc->exitTextOn : 0;
    const int exitSelected = mc ? mc->exitSelected : 0;
    const int exitFrameOn = mc ? mc->exitFrameOn : 0;
    const bool argBacked = PrEvent::IsEvMemCardArgBacked();
    const int ioMessage = mc ? mc->ioMessage : -1;

    static int s_lastEv = -1;
    static int s_lastSel = -1;
    static int s_lastIo = -1;
    if (eventId != s_lastEv || selected != s_lastSel || ioMessage != s_lastIo) {
        s_lastEv = eventId;
        s_lastSel = selected;
        s_lastIo = ioMessage;
        Log::Printf("MemCardUi ev=%d selected=%d rows=%d cols=%d items=%d io=%d",
                    eventId, selected, rows, cols, itemCount, ioMessage);
    }

    auto drawFixed = [&](const PracticeUiFixedTemplate& tpl, float x, float y, int layer) -> bool {
        if (!tpl.valid) return false;
        return DrawPsxSpriteTemplate(ctx, vx, vy, vs, x, y, tpl.tpl, 1, 1, 1, 1, layer);
    };
    auto drawLangTable = [&](const MemCardUiTextTable& table, int layer) -> bool {
        if (!table.valid || !table.tplValid[lang]) return false;
        return DrawPsxSpriteTemplate(ctx, vx, vy, vs, table.x[lang], table.y[lang],
                                     table.tpl[lang], 1, 1, 1, 1, layer);
    };
    auto drawExitText = [&](int stateIndex, int layer) -> bool {
        if (!ui.exitText.valid) return false;
        const int st = std::clamp(stateIndex, 0, 2);
        if (!ui.exitText.tplValid[lang][st]) return false;
        return DrawPsxSpriteTemplate(ctx, vx, vy, vs,
                                     ui.exitText.x[lang], ui.exitText.y[lang],
                                     ui.exitText.tpl[lang][st], 1, 1, 1, 1, layer);
    };
    auto drawIoBanner = [&](int msgType, int layerBase) {
        const std::string* msgSet = nullptr;
        if (msgType == 1) msgSet = ui.ioText1;
        else if (msgType == 2) msgSet = ui.ioText2;
        else msgSet = ui.ioText0;
        const std::string& msg = msgSet[lang];
        if (msg.empty()) return;

        const float textW = MeasureMemCardTextNative(ui, msg.c_str()) * vs;
        const float boxW = textW + 8.0f * vs;
        const float boxX = vx + ((320.0f * vs) - boxW) * 0.5f;
        const float boxY = vy + 115.0f * vs;
        DrawRectUI(s_renderer, 0.0f, 0.0f, 1.0f, boxX + 8.0f * vs, boxY, boxW - 8.0f * vs, 25.0f * vs,
                   0.06f, 0.06f, 0.06f, 0.85f);
        DrawRectUI(s_renderer, 0.0f, 0.0f, 1.0f, boxX, boxY + 8.0f * vs, 8.0f * vs, 9.0f * vs,
                   0.06f, 0.06f, 0.06f, 0.85f);
        DrawRectUI(s_renderer, 0.0f, 0.0f, 1.0f, boxX + boxW, boxY + 8.0f * vs, 8.0f * vs, 9.0f * vs,
                   0.06f, 0.06f, 0.06f, 0.85f);
        drawFixed(ui.ioCornerLT, (boxX - vx) / vs, (boxY - vy) / vs, layerBase + 0);
        drawFixed(ui.ioCornerLB, (boxX - vx) / vs, (boxY + 17.0f * vs - vy) / vs, layerBase + 1);
        drawFixed(ui.ioCornerRT, (boxX + boxW - vx) / vs, (boxY - vy) / vs, layerBase + 2);
        drawFixed(ui.ioCornerRB, (boxX + boxW - vx) / vs, (boxY + 17.0f * vs - vy) / vs, layerBase + 3);
        const float tx = ToScreenX(vx, vs, ((boxX - vx) / vs) + 4.0f);
        const float ty = ToScreenY(vy, vs, 121.0f);
        if (ctx.renderer) {
            ctx.renderer->FlushSprites();
        }
        if (!DrawMemCardTextNative(ctx, ui, tx, ty, vs, msg.c_str(), 480, 1.0f, 1.0f, 1.0f, 1.0f)) {
            DrawString(s_renderer, tx, ty, 0.85f * vs, msg.c_str(), 0.10f, 0.10f, 0.10f, 1.0f);
        }
    };

    drawFixed(ui.titleBg, 37.0f, 36.0f, 700);
    drawFixed(ui.hintBg, 123.0f, 30.0f, 700);
    if (page) {
        drawLangTable(page->titleText, 710);
        drawLangTable(page->hintText, 710);
        drawLangTable(page->auxText0, 710);
        drawLangTable(page->auxText1, 710);
        drawLangTable(page->auxText2, 710);
    }

    const float markerStartX = 51.0f;
    const float markerStepX = 74.0f;
    const float markerStartY = 73.0f;
    const float markerStepY = 21.0f;
    const float textStartX = 52.0f;
    const float textStartY = 78.0f;
    struct SlotTextDraw {
        std::string text;
        float x = 0.0f;
        float y = 0.0f;
        float scale = 1.0f;
        float alpha = 1.0f;
        bool dim = false;
    };
    std::vector<SlotTextDraw> slotTextDraws;
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            const int idx = r * cols + c;
            if (idx >= slotCount || idx >= 15) {
                continue;
            }
            int state = 2;
            if (mc && idx == selected) {
                state = 1;
            } else if (!mc || mc->enabled[idx]) {
                state = 0;
            }
            const float markerX = markerStartX + markerStepX * (float)c;
            const float markerY = markerStartY + markerStepY * (float)r;
            drawFixed(ui.gridMarker[state], markerX, markerY, 705);

            if (mc && mc->slotText[idx][0] != '\0') {
                SlotTextDraw td{};
                td.text = mc->slotText[idx];
                const size_t maxChars = 8;
                if (td.text.size() > maxChars) {
                    td.text.resize(maxChars);
                }
                td.dim = (idx != selected);
                td.scale = 1.0f;
                td.x = ToScreenX(vx, vs, textStartX + markerStepX * (float)c);
                td.y = ToScreenY(vy, vs, textStartY + markerStepY * (float)r);
                td.alpha = td.dim ? 1.0f : 1.0f;
                slotTextDraws.push_back(std::move(td));
            }
        }
    }
    if (!slotTextDraws.empty() && ctx.renderer) {
        ctx.renderer->FlushSprites();
        for (const SlotTextDraw& td : slotTextDraws) {
            if (!DrawMemCardTextNative(ctx, ui, td.x, td.y, vs, td.text.c_str(),
                                       td.dim ? 483 : 482,
                                       1.0f, 1.0f, 1.0f, td.alpha)) {
                PrVText::DrawString(ctx, (int)td.x, (int)td.y, td.text.c_str(), 0xFF101010u, 0.60f, false);
            }
        }
    }

    const bool exitFocus = (selected == exitIndex);
    const int exitTextState = exitFocus ? (exitTextOn ? 2 : 1) : 0;
    const bool fallbackBlinkOn = ((((int)ctx.frame / 20) & 1) == 0);
    const bool exitFrameLit = exitFocus && (argBacked ? (exitFrameOn != 0) : fallbackBlinkOn);
    drawFixed(exitFrameLit ? ui.exitFrameOn : ui.exitFrameOff, 231.0f, 179.0f, 700);
    drawFixed(ui.exitBar[exitTextState], ui.exitBarX, ui.exitBarY, 705);
    drawExitText(exitTextState, 710);
    if (ioMessage >= 0) {
        drawIoBanner(ioMessage, 720);
    }
}

// ======== ev=6 HI-SCORE page: PSX TIM rendering ========
// memory.md §ev=6: sub_80021594
// Layout: title (37,31) | 3 column headers (84/163/242, 56) | 6 rows @18px | EXIT (231,179)
static void RenderScn0HiScore(PrGameContext& ctx, float vx, float vy, float vs,
                               const PrEventDispatcherContext& dispCtx) {
    vx += ctx.scn0PanelOffsetX * vs;
    vy += ctx.scn0PanelOffsetY * vs;
    const int lang = (int)ctx.languageIndex;
    const char lc = LangSuffix(lang);
    const bool blinkOn = (((int)ctx.frame / 20) & 1) == 0;
    char b[32], t[32];

    // === 1. HI-SCORE title at (37, 31) ===
    // SCOR_1B0 (background), SCOR_1{lang}0 (text) — single state (suffix 0)
    {
        std::snprintf(b, sizeof(b), "SCOR_1B0");
        std::snprintf(t, sizeof(t), "SCOR_1%c0", lc);
        DrawTimAutoSize(ctx, vx, vy, vs, b, 37.0f, 31.0f, 1, 1, 1, 1, 700);
        TextureResource* tb = ctx.resources ? ctx.resources->GetTexture(b) : nullptr;
        TextureResource* tt = ctx.resources ? ctx.resources->GetTexture(t) : nullptr;
        if (tb && tt) {
            const float x = 37.0f + ((float)tb->tim.width - (float)tt->tim.width) * 0.5f;
            const float y = 31.0f + ((float)tb->tim.height - (float)tt->tim.height) * 0.5f;
            DrawTimAutoSize(ctx, vx, vy, vs, t, x, y, 1, 1, 1, 1, 710);
        } else if (!tb) {
            DrawRectUI(s_renderer, vx, vy, vs, 35.0f, 29.0f, 80.0f, 20.0f,
                      0.85f, 0.75f, 0.15f, 0.9f);
            DrawString(s_renderer, ToScreenX(vx,vs,40), ToScreenY(vy,vs,33), 1.2f*vs,
                      "HI-SCORE", 0.1f, 0.1f, 0.1f, 1.0f);
        } else {
            DrawTimAutoSize(ctx, vx, vy, vs, t, 37.0f, 31.0f, 1, 1, 1, 1, 710);
        }
    }

    // === 2. Column headers at (84,56), (163,56), (242,56) ===
    // PSX: crown/trophy rank icons for COOL/GOOD/BAD columns
    {
        static const float kColX[] = {84.0f, 163.0f, 242.0f};
        static const float kColY = 56.0f;
        static const float kColColors[][3] = {
            {0.85f, 0.75f, 0.15f},  // gold (COOL)
            {0.70f, 0.70f, 0.75f},  // silver (GOOD)
            {0.80f, 0.30f, 0.25f},  // bronze (BAD)
        };
        static const char* kColLabel[] = {"COOL", "GOOD", "BAD"};
        for (int c = 0; c < 3; c++) {
            DrawRectUI(s_renderer, vx, vy, vs, kColX[c], kColY, 12.0f, 12.0f,
                      kColColors[c][0], kColColors[c][1], kColColors[c][2], 0.9f);
            DrawString(s_renderer, ToScreenX(vx,vs,kColX[c]+14), ToScreenY(vy,vs,kColY+1),
                      0.8f*vs, kColLabel[c],
                      kColColors[c][0], kColColors[c][1], kColColors[c][2], 0.8f);
        }
    }

    // === 3. Score table: 6 rows (stages), 3 columns (ranks) per row ===
    // memory.md: rowStartY=69, rowHeight=18, rowX=37
    {
        const float rowX = 37.0f;
        const float rowStartY = 69.0f;
        const float rowH = 18.0f;
        const float cellW = 72.0f;
        const float cellGap = 6.0f;
        static const char* kStageName[] = {
            "S1", "S2", "S3", "S4", "S5", "S6"
        };

        for (int row = 0; row < 6; row++) {
            const float ry = rowStartY + row * rowH;
            const bool altRow = (row & 1) != 0;

            // Stage label at left
            DrawRectUI(s_renderer, vx, vy, vs, rowX - 14.0f, ry + 1.0f, 12.0f, 14.0f,
                      0.25f, 0.45f, 0.25f, 0.7f);
            DrawString(s_renderer, ToScreenX(vx,vs,rowX-13), ToScreenY(vy,vs,ry+3),
                      0.7f*vs, kStageName[row], 0.9f, 0.9f, 0.9f, 0.8f);

            // 3 score cells with alternating row tint
            for (int col = 0; col < 3; col++) {
                const float cx = rowX + col * (cellW + cellGap);
                const float bgR = altRow ? 0.30f : 0.35f;
                const float bgG = altRow ? 0.22f : 0.25f;
                const float bgB = altRow ? 0.06f : 0.08f;
                DrawRectUI(s_renderer, vx, vy, vs, cx, ry, cellW, rowH - 2.0f,
                          bgR, bgG, bgB, 0.85f);
                // Placeholder score text
                DrawString(s_renderer, ToScreenX(vx,vs,cx+cellW*0.5f-12), ToScreenY(vy,vs,ry+3),
                          0.8f*vs, "---", 0.6f, 0.55f, 0.45f, 0.6f);
            }
        }
    }

    // === 4. EXIT at (231, 179) — blinking; Cross=confirm, Circle=cancel ===
    {
        const int st = blinkOn ? 3 : 2;

        std::snprintf(b, sizeof(b), "EXIT_0B%d", st);
        DrawTimAutoSize(ctx, vx, vy, vs, b, 231.0f, 179.0f, 1, 1, 1, 1, 700);

        static const float kExitTxX[] = {242.0f, 238.0f, 238.0f, 241.0f, 239.0f};
        static const float kExitTxY[] = {191.0f, 193.0f, 193.0f, 190.0f, 191.0f};
        std::snprintf(t, sizeof(t), "EXIT_0%c%d", lc, st);
        DrawTimAutoSize(ctx, vx, vy, vs, t, kExitTxX[lang], kExitTxY[lang], 1, 1, 1, 1, 710);

        // Fallback if no EXIT TIM
        if (!ctx.resources || !ctx.resources->GetTexture(b)) {
            DrawRectUI(s_renderer, vx, vy, vs, 229.0f, 177.0f, 56.0f, 16.0f,
                      0.5f, 0.4f, 0.05f, 0.8f);
            DrawString(s_renderer, ToScreenX(vx,vs,233), ToScreenY(vy,vs,181), 1.0f*vs,
                      "EXIT", 0.9f, 0.9f, 0.9f, 1.0f);
        }
    }

    // === 5. Confirm/Cancel hint (fallback only) ===
    {
        char tb[32];
        std::snprintf(tb, sizeof(tb), "SCOR_1B0");
        if (!ctx.resources || !ctx.resources->GetTexture(tb)) {
            const float hintAlpha = blinkOn ? 0.9f : 0.6f;
            DrawString(s_renderer, ToScreenX(vx,vs,37), ToScreenY(vy,vs,200), 0.8f*vs,
                      "X:Play  O:Back", 0.7f, 0.7f, 0.65f, hintAlpha);
        }
    }
}

// ======== Generic vertical menu fallback (ev=4/5 and not-yet-native paths) ========
static void RenderVerticalMenu(PrGameContext& ctx, float vx, float vy, float vs,
                                const PrEventDispatcherContext& dispCtx, int eventId) {
    const float baseW = 320.0f;
    const float baseH = 240.0f;
    const bool isMainMenu = (eventId == 3);
    const bool isConfirm = (eventId == 6 || eventId == 11);

    const float boxW = isMainMenu ? 100.0f : 140.0f;
    const float boxH = 22.0f;
    const float spacing = 3.0f;
    const int menuCount = (int)dispCtx.menuCount;
    const float listH = (float)menuCount * boxH + (menuCount > 1 ? (float)(menuCount - 1) * spacing : 0.0f);

    // ev=3: right-aligned lower area (PSX: menu items beside PaRappa model)
    // ev=6: centered confirm dialog
    // ev=17: centered options
    float startX, startY;
    if (isMainMenu) {
        startX = baseW - boxW - 20.0f;
        startY = baseH - listH - 40.0f;
    } else if (isConfirm) {
        startX = (baseW - boxW) * 0.5f;
        startY = (baseH - listH) * 0.5f;
    } else {
        startX = (baseW - boxW) * 0.5f;
        startY = (baseH - listH) * 0.5f - 10.0f;
    }

    // Panel background
    const float panelPad = 12.0f;
    const float panelTopPad = (eventId == 11) ? 30.0f : panelPad;
    DrawRectUI(s_renderer, vx, vy, vs, startX-panelPad, startY-panelTopPad,
              boxW+panelPad*2, listH+panelTopPad+panelPad, 0.0f, 0.0f, 0.0f, 0.65f);

    const float hiT = BlinkFade(UiTimeSeconds(ctx), 2.0f);
    if (eventId == 11) {
        DrawSpriteText(ctx, vx, vy, vs, startX + 46.0f, startY - 22.0f, 0.85f,
                       "SAVE?", 0.95f, 0.9f, 0.65f, 1.0f);
    }

    for (int i = 0; i < menuCount && i < 9; i++) {
        const float y = startY + i * (boxH + spacing);
        const bool sel = (i == (int)dispCtx.menuIndex);

        float r = 0.12f, g = 0.12f, b = 0.22f, a = 0.85f;
        float tr = 0.65f, tg = 0.65f, tb = 0.65f;
        if (sel) {
            r = Lerp(0.35f, 0.95f, hiT); g = Lerp(0.22f, 0.75f, hiT); b = Lerp(0.0f, 0.12f, hiT);
            tr = 1.0f; tg = 1.0f; tb = 1.0f;
        }

        DrawRectUI(s_renderer, vx, vy, vs, startX, y, boxW, boxH, r, g, b, a);
        if (sel) {
            const float ba = 0.65f + 0.35f * hiT;
            DrawRectUI(s_renderer, vx, vy, vs, startX-1, y-1, boxW+2, 1, 1.0f, 0.8f, 0.2f, ba);
            DrawRectUI(s_renderer, vx, vy, vs, startX-1, y+boxH, boxW+2, 1, 1.0f, 0.8f, 0.2f, ba);
            DrawRectUI(s_renderer, vx, vy, vs, startX-1, y, 1, boxH, 1.0f, 0.8f, 0.2f, ba);
            DrawRectUI(s_renderer, vx, vy, vs, startX+boxW, y, 1, boxH, 1.0f, 0.8f, 0.2f, ba);

            DrawMenuCursorSprite(ctx, vx, vy, vs, startX, y, boxW, boxH, hiT, 1.0f, 0.8f, 0.2f, 1.0f);
        }

        const char* label = GetMenuLabel(eventId, i);
        if (label) {
            DrawSpriteText(ctx, vx, vy, vs, startX+7, y+1, 0.75f, label, 0.0f, 0.0f, 0.0f, 0.45f);
            DrawSpriteText(ctx, vx, vy, vs, startX+6, y+0, 0.75f, label, tr, tg, tb, 1.0f);
        }

        // ev=17: show current value next to SUBTITLE/LANGUAGE rows
        if (eventId == 17 && i < 2) {
            const char* val = nullptr;
            if (i == 0) val = (dispCtx.selections[0] == 0) ? "ON" : "OFF";
            if (i == 1) {
                static const char* langs[] = {"ENGLISH", "FRENCH", "GERMAN", "SPANISH", "ITALIAN"};
                int lang = dispCtx.selections[1];
                if (lang >= 0 && lang < 5) val = langs[lang];
                else val = "???";
            }
            if (val) {
                DrawString(s_renderer, ToScreenX(vx,vs,startX+boxW+6), ToScreenY(vy,vs,y+6),
                          1.0f*vs, val, tr, tg, tb, 0.9f);
            }
        }
    }
}

void Render(PrGameContext& ctx) {
    if (!s_renderer) return;
    const bool ss0DirectOwnedScene = IsSS0DirectOwnedScene(ctx);
    const ActiveEventFrameOverlay activeFrame =
        ResolveActiveEventFrameOverlay(ctx);
    const bool hasDirectEventFrame = activeFrame.state != nullptr;
    if (ss0DirectOwnedScene) {
        if (!hasDirectEventFrame) {
            return;
        }
        // The SS0 Event4 prompt is already represented by the exact
        // 8001E750 packet mirror for the gp+0x38C == 0 branch. Rendering the
        // native prompt here as well would draw it during the formal box-fill
        // and MoveImage states, so the direct path remains packet-only.
        RenderEventFrameDirectPackets(ctx, activeFrame.state);
        return;
    }

    const bool dispatcherRunning = PrEvent::IsDispatcherRunning();
    if (!dispatcherRunning && !hasDirectEventFrame) return;


    float vx = 0.0f, vy = 0.0f, vs = 1.0f;
    CalcPs1Viewport(s_renderer, vx, vy, vs);

    if (!dispatcherRunning) {
        RenderEventFrameDirectPackets(ctx, activeFrame.state);
        return;
    }

    const PrEventDispatcherContext& dispCtx = PrEvent::GetDispatcherContext();
    const int eventId = PrEvent::GetDispEventIdPtr() ? *PrEvent::GetDispEventIdPtr() : 0;

    // Dispatch to event-specific renderer
    if (eventId == 2) {
        RenderEventFrameDirectPackets(ctx, activeFrame.state);
        RenderStageSelect(ctx, vx, vy, vs, dispCtx);
    } else if (eventId == 3) {
        RenderScn0MainMenu(ctx, vx, vy, vs, dispCtx);
    } else if (eventId == 4) {
        RenderEventFrameDirectPackets(ctx, activeFrame.state);
        int selectionState = (int)dispCtx.pauseChoice;
        if (selectionState != 0 && selectionState != 1) {
            selectionState = -1;
        }
        if (!RenderEvent4PromptNative(ctx, selectionState)) {
            RenderVerticalMenu(ctx, vx, vy, vs, dispCtx, eventId);
        }
    } else if (eventId == 6) {
        RenderScn0HiScore(ctx, vx, vy, vs, dispCtx);
    } else if (eventId == 5) {
        RenderScn0Practice(ctx, vx, vy, vs, dispCtx);
    } else if (eventId == 11) {
        RenderEventFrameDirectPackets(ctx, activeFrame.state);
    } else if (eventId == 7 || eventId == 8 || eventId == 9) {
        RenderScn0MemCard(ctx, vx, vy, vs, dispCtx, eventId);
    } else if (eventId == 17) {
        RenderScn0Language(ctx, vx, vy, vs, dispCtx);
    } else if (eventId == 16) {
        RenderPracticeRun(ctx, vx, vy, vs, dispCtx);
    } else {
        RenderVerticalMenu(ctx, vx, vy, vs, dispCtx, eventId);
    }
}

}
