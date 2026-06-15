#include "pr_stage1_p2_scorer_hud.h"

#include "d3d11_renderer.h"
#include "pr_game_context.h"
#include "pr_psx_sprite_template_render.h"
#include "pr_stage1_hud_presentation_direct.h"
#include "pr_stage1_scorer_host.h"
#include "pr_stage1_scorer_host_deps.h"

#include <wincodec.h>
#include <wrl/client.h>

#include <algorithm>
#include <array>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <string>
#include <vector>

using Microsoft::WRL::ComPtr;

namespace PrStage1P2ScorerHud {
namespace {

static constexpr int kHudLayerUnderRankHands = 929;
static constexpr int kCreativePromptLayerNearRail = 783;

struct Stage1P2ScorerHudSnapshot {
    bool valid = false;
    uint32_t updateFrame = 0;
    int32_t flow = 0;
    int32_t rhyme = 0;
    int32_t drop = 0;
    int32_t hype = 0;
    int32_t mix = 0;
};

struct Stage1P2ScorerHudRuntime {
    Stage1P2ScorerHudSnapshot snapshot{};
    uint32_t pulseStartFrame[4] = {};
    bool pulseActive[4] = {};
    uint32_t totalPulseStartFrame = 0;
    bool totalPulseActive = false;
    bool lastQueryFrameValid = false;
    uint32_t lastQueryFrame = 0;
    bool creativePromptVisible = false;
    bool creativePromptLeaving = false;
    bool creativePromptSettledValid = false;
    uint32_t creativePromptKey = 0;
    uint32_t creativePromptSettledKey = 0;
    uint32_t creativePromptStartFrame = 0;
    uint32_t creativePromptLeaveFrame = 0;
};

struct Stage1P2CreativePromptTexture {
    ID3D11ShaderResourceView* srv = nullptr;
    D3D11Renderer* renderer = nullptr;
    std::string language;
    int width = 0;
    int height = 0;
    bool tried = false;
};

struct Stage1P2SubscoreLabelTexture {
    ID3D11ShaderResourceView* srv = nullptr;
    D3D11Renderer* renderer = nullptr;
    std::string language;
    std::string name;
    int width = 0;
    int height = 0;
    int cropX = 0;
    int cropY = 0;
    int cropW = 0;
    int cropH = 0;
    bool tried = false;
};

static constexpr size_t kSubscoreLabelTextureCount = 4u;

Stage1P2ScorerHudRuntime& GetRuntime() {
    static Stage1P2ScorerHudRuntime s_runtime;
    return s_runtime;
}

void ResetRuntime() {
    GetRuntime() = Stage1P2ScorerHudRuntime{};
}

Stage1P2CreativePromptTexture& GetCreativePromptTexture() {
    static Stage1P2CreativePromptTexture s_texture;
    return s_texture;
}

void ReleaseCreativePromptTexture() {
    Stage1P2CreativePromptTexture& texture = GetCreativePromptTexture();
    if (texture.srv != nullptr && texture.renderer != nullptr) {
        texture.renderer->DestroyTexture(texture.srv);
    }
    texture = Stage1P2CreativePromptTexture{};
}

std::array<Stage1P2SubscoreLabelTexture, kSubscoreLabelTextureCount>&
GetSubscoreLabelTextures() {
    static std::array<Stage1P2SubscoreLabelTexture, kSubscoreLabelTextureCount>
        s_textures;
    return s_textures;
}

void ReleaseSubscoreLabelTexture(size_t index) {
    auto& textures = GetSubscoreLabelTextures();
    if (index >= textures.size()) {
        return;
    }
    Stage1P2SubscoreLabelTexture& texture = textures[index];
    if (texture.srv != nullptr && texture.renderer != nullptr) {
        texture.renderer->DestroyTexture(texture.srv);
    }
    texture = Stage1P2SubscoreLabelTexture{};
}

bool CreateWicFactory(ComPtr<IWICImagingFactory>& factory) {
    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (FAILED(hr) && hr != RPC_E_CHANGED_MODE) {
        return false;
    }
    hr = CoCreateInstance(CLSID_WICImagingFactory,
                          nullptr,
                          CLSCTX_INPROC_SERVER,
                          IID_PPV_ARGS(&factory));
    return SUCCEEDED(hr);
}

bool LoadImageRgba(const std::filesystem::path& path,
                   std::vector<uint32_t>& outRgba,
                   int& outWidth,
                   int& outHeight) {
    outRgba.clear();
    outWidth = 0;
    outHeight = 0;

    ComPtr<IWICImagingFactory> factory;
    if (!CreateWicFactory(factory)) {
        return false;
    }

    ComPtr<IWICBitmapDecoder> decoder;
    HRESULT hr = factory->CreateDecoderFromFilename(path.wstring().c_str(),
                                                    nullptr,
                                                    GENERIC_READ,
                                                    WICDecodeMetadataCacheOnDemand,
                                                    &decoder);
    if (FAILED(hr)) {
        return false;
    }

    ComPtr<IWICBitmapFrameDecode> frame;
    hr = decoder->GetFrame(0, &frame);
    if (FAILED(hr)) {
        return false;
    }

    UINT width = 0u;
    UINT height = 0u;
    hr = frame->GetSize(&width, &height);
    if (FAILED(hr) || width == 0u || height == 0u) {
        return false;
    }

    ComPtr<IWICFormatConverter> converter;
    hr = factory->CreateFormatConverter(&converter);
    if (FAILED(hr)) {
        return false;
    }
    hr = converter->Initialize(frame.Get(),
                               GUID_WICPixelFormat32bppRGBA,
                               WICBitmapDitherTypeNone,
                               nullptr,
                               0.0,
                               WICBitmapPaletteTypeCustom);
    if (FAILED(hr)) {
        return false;
    }

    outRgba.resize(static_cast<size_t>(width) * static_cast<size_t>(height));
    hr = converter->CopyPixels(nullptr,
                               width * 4u,
                               static_cast<UINT>(outRgba.size() * 4u),
                               reinterpret_cast<BYTE*>(outRgba.data()));
    if (FAILED(hr)) {
        outRgba.clear();
        return false;
    }

    outWidth = static_cast<int>(width);
    outHeight = static_cast<int>(height);
    return true;
}

void ResolveAlphaBounds(const std::vector<uint32_t>& rgba,
                        int width,
                        int height,
                        int& outX,
                        int& outY,
                        int& outW,
                        int& outH) {
    int minX = width;
    int minY = height;
    int maxX = -1;
    int maxY = -1;
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const uint32_t pixel =
                rgba[static_cast<size_t>(y) * static_cast<size_t>(width) +
                     static_cast<size_t>(x)];
            const uint8_t alpha = static_cast<uint8_t>((pixel >> 24) & 0xFFu);
            if (alpha <= 8u) {
                continue;
            }
            minX = (std::min)(minX, x);
            minY = (std::min)(minY, y);
            maxX = (std::max)(maxX, x);
            maxY = (std::max)(maxY, y);
        }
    }

    if (maxX < minX || maxY < minY) {
        outX = 0;
        outY = 0;
        outW = width;
        outH = height;
        return;
    }

    outX = minX;
    outY = minY;
    outW = maxX - minX + 1;
    outH = maxY - minY + 1;
}

std::string NormalizeCreativePromptLanguage(const std::string& language) {
    std::string upper = language;
    std::transform(upper.begin(), upper.end(), upper.begin(), [](char c) {
        return static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    });
    return upper == "CN" || upper == "ZH" || upper == "ZH-CN" ? "CN" : "EN";
}

std::filesystem::path ResolvePr2RailAssetPath(const PrGameContext& ctx,
                                               const std::string& filename) {
    std::vector<std::filesystem::path> bases;
    if (!ctx.dataRoot.empty()) {
        bases.push_back(ctx.dataRoot / "win" / "ex" / "image" / "pr2_rail");
        bases.push_back(ctx.dataRoot / "ex" / "image" / "pr2_rail");
        if (ctx.dataRoot.has_parent_path()) {
            bases.push_back(ctx.dataRoot.parent_path() / "win" / "ex" /
                            "image" / "pr2_rail");
            bases.push_back(ctx.dataRoot.parent_path() / "ex" / "image" /
                            "pr2_rail");
        }
    }
    bases.push_back(std::filesystem::current_path() / "ex" / "image" /
                    "pr2_rail");

    for (const auto& base : bases) {
        const std::filesystem::path candidate = base / filename;
        if (std::filesystem::exists(candidate)) {
            return candidate;
        }
    }
    return {};
}

std::filesystem::path ResolveCreativePromptAssetPath(
    const PrGameContext& ctx,
    const std::string& language) {
    const std::string filename =
        language == "CN" ? "chuangyishijian_CN.png"
                         : "chuangyishijian_EN.png";
    return ResolvePr2RailAssetPath(ctx, filename);
}

ID3D11ShaderResourceView* EnsureCreativePromptTexture(PrGameContext& ctx,
                                                      int& width,
                                                      int& height) {
    width = 0;
    height = 0;
    if (ctx.renderer == nullptr) {
        return nullptr;
    }

    const std::string language =
        NormalizeCreativePromptLanguage(
            ctx.stage1RailParappa2CreativePromptLanguage);
    Stage1P2CreativePromptTexture& texture = GetCreativePromptTexture();
    if (texture.srv != nullptr &&
        texture.renderer == ctx.renderer &&
        texture.language == language) {
        width = texture.width;
        height = texture.height;
        return texture.srv;
    }

    ReleaseCreativePromptTexture();
    Stage1P2CreativePromptTexture& fresh = GetCreativePromptTexture();
    fresh.renderer = ctx.renderer;
    fresh.language = language;
    fresh.tried = true;

    const std::filesystem::path path =
        ResolveCreativePromptAssetPath(ctx, language);
    if (path.empty()) {
        return nullptr;
    }

    std::vector<uint32_t> rgba;
    int imageWidth = 0;
    int imageHeight = 0;
    if (!LoadImageRgba(path, rgba, imageWidth, imageHeight)) {
        return nullptr;
    }

    fresh.srv = ctx.renderer->CreateTexture(rgba.data(),
                                            imageWidth,
                                            imageHeight);
    if (fresh.srv == nullptr) {
        return nullptr;
    }
    fresh.width = imageWidth;
    fresh.height = imageHeight;
    width = imageWidth;
    height = imageHeight;
    return fresh.srv;
}

const char* SubscoreTextureName(size_t index) {
    static constexpr const char* kNames[kSubscoreLabelTextureCount] = {
        "FLOW",
        "RHYME",
        "DROP",
        "HYPE",
    };
    return index < kSubscoreLabelTextureCount ? kNames[index] : "";
}

ID3D11ShaderResourceView* EnsureSubscoreLabelTexture(PrGameContext& ctx,
                                                     size_t index,
                                                     int& width,
                                                     int& height,
                                                     int& cropX,
                                                     int& cropY,
                                                     int& cropW,
                                                     int& cropH) {
    width = 0;
    height = 0;
    cropX = 0;
    cropY = 0;
    cropW = 0;
    cropH = 0;
    if (ctx.renderer == nullptr || index >= kSubscoreLabelTextureCount) {
        return nullptr;
    }

    const std::string language =
        NormalizeCreativePromptLanguage(
            ctx.stage1RailParappa2CreativePromptLanguage);
    const std::string name = SubscoreTextureName(index);
    auto& textures = GetSubscoreLabelTextures();
    Stage1P2SubscoreLabelTexture& texture = textures[index];
    if (texture.srv != nullptr &&
        texture.renderer == ctx.renderer &&
        texture.language == language &&
        texture.name == name) {
        width = texture.width;
        height = texture.height;
        cropX = texture.cropX;
        cropY = texture.cropY;
        cropW = texture.cropW;
        cropH = texture.cropH;
        return texture.srv;
    }

    ReleaseSubscoreLabelTexture(index);
    Stage1P2SubscoreLabelTexture& fresh = GetSubscoreLabelTextures()[index];
    fresh.renderer = ctx.renderer;
    fresh.language = language;
    fresh.name = name;
    fresh.tried = true;

    std::filesystem::path path = ResolvePr2RailAssetPath(
        ctx,
        name + "_transparent_" + language + ".png");
    if (path.empty() && language != "EN") {
        path = ResolvePr2RailAssetPath(ctx, name + "_transparent_EN.png");
    }
    if (path.empty()) {
        return nullptr;
    }

    std::vector<uint32_t> rgba;
    int imageWidth = 0;
    int imageHeight = 0;
    if (!LoadImageRgba(path, rgba, imageWidth, imageHeight)) {
        return nullptr;
    }

    int imageCropX = 0;
    int imageCropY = 0;
    int imageCropW = imageWidth;
    int imageCropH = imageHeight;
    ResolveAlphaBounds(rgba,
                       imageWidth,
                       imageHeight,
                       imageCropX,
                       imageCropY,
                       imageCropW,
                       imageCropH);

    fresh.srv = ctx.renderer->CreateTexture(rgba.data(),
                                            imageWidth,
                                            imageHeight);
    if (fresh.srv == nullptr) {
        return nullptr;
    }
    fresh.width = imageWidth;
    fresh.height = imageHeight;
    fresh.cropX = imageCropX;
    fresh.cropY = imageCropY;
    fresh.cropW = imageCropW;
    fresh.cropH = imageCropH;
    width = imageWidth;
    height = imageHeight;
    cropX = imageCropX;
    cropY = imageCropY;
    cropW = imageCropW;
    cropH = imageCropH;
    return fresh.srv;
}

void SubmitRectPsx(PrGameContext& ctx,
                   float vx,
                   float vy,
                   float vs,
                   float x,
                   float y,
                   float w,
                   float h,
                   float r,
                   float g,
                   float b,
                   float a,
                   int layer,
                   int& order) {
    if (!ctx.renderer || w <= 0.0f || h <= 0.0f || a <= 0.0f) {
        return;
    }

    D3D11Renderer::SolidRectCmd cmd{};
    cmd.x = vx + x * vs;
    cmd.y = vy + y * vs;
    cmd.w = w * vs;
    cmd.h = h * vs;
    cmd.r = r;
    cmd.g = g;
    cmd.b = b;
    cmd.a = a;
    cmd.layer = layer;
    cmd.order = static_cast<uint64_t>(order++);
    ctx.renderer->SubmitSolidRect(cmd);
}

const char* GlyphRow(char c, int row) {
    static constexpr const char* kBlank[7] = {
        ".....", ".....", ".....", ".....", ".....", ".....", ".....",
    };
    static constexpr const char* kD[7] = {
        "####.", "#...#", "#...#", "#...#", "#...#", "#...#", "####.",
    };
    static constexpr const char* kE[7] = {
        "#####", "#....", "#....", "####.", "#....", "#....", "#####",
    };
    static constexpr const char* kF[7] = {
        "#####", "#....", "#....", "####.", "#....", "#....", "#....",
    };
    static constexpr const char* kH[7] = {
        "#...#", "#...#", "#...#", "#####", "#...#", "#...#", "#...#",
    };
    static constexpr const char* kL[7] = {
        "#....", "#....", "#....", "#....", "#....", "#....", "#####",
    };
    static constexpr const char* kM[7] = {
        "#...#", "##.##", "#.#.#", "#.#.#", "#...#", "#...#", "#...#",
    };
    static constexpr const char* kO[7] = {
        ".###.", "#...#", "#...#", "#...#", "#...#", "#...#", ".###.",
    };
    static constexpr const char* kP[7] = {
        "####.", "#...#", "#...#", "####.", "#....", "#....", "#....",
    };
    static constexpr const char* kR[7] = {
        "####.", "#...#", "#...#", "####.", "#.#..", "#..#.", "#...#",
    };
    static constexpr const char* kW[7] = {
        "#...#", "#...#", "#...#", "#.#.#", "#.#.#", "##.##", "#...#",
    };
    static constexpr const char* kY[7] = {
        "#...#", "#...#", ".#.#.", "..#..", "..#..", "..#..", "..#..",
    };
    static constexpr const char* kPlus[7] = {
        ".....", "..#..", "..#..", "#####", "..#..", "..#..", ".....",
    };
    static constexpr const char* kMinus[7] = {
        ".....", ".....", ".....", "#####", ".....", ".....", ".....",
    };

    const char* const* glyph = kBlank;
    switch (c) {
    case 'D': glyph = kD; break;
    case 'E': glyph = kE; break;
    case 'F': glyph = kF; break;
    case 'H': glyph = kH; break;
    case 'L': glyph = kL; break;
    case 'M': glyph = kM; break;
    case 'O': glyph = kO; break;
    case 'P': glyph = kP; break;
    case 'R': glyph = kR; break;
    case 'W': glyph = kW; break;
    case 'Y': glyph = kY; break;
    case '+': glyph = kPlus; break;
    case '-': glyph = kMinus; break;
    default: glyph = kBlank; break;
    }
    return glyph[(std::max)(0, (std::min)(row, 6))];
}

float DrawPixelGlyph(PrGameContext& ctx,
                     float vx,
                     float vy,
                     float vs,
                     float x,
                     float y,
                     char c,
                     float px,
                     float r,
                     float g,
                     float b,
                     float a,
                     int layer,
                     int& order) {
    if (c == ' ') {
        return px * 3.0f;
    }

    const float shadow = px * 0.75f;
    for (int row = 0; row < 7; ++row) {
        const char* bits = GlyphRow(c, row);
        for (int col = 0; col < 5; ++col) {
            if (bits[col] == '.') {
                continue;
            }
            SubmitRectPsx(ctx,
                          vx,
                          vy,
                          vs,
                          x + shadow + static_cast<float>(col) * px,
                          y + shadow + static_cast<float>(row) * px,
                          px,
                          px,
                          0.0f,
                          0.0f,
                          0.0f,
                          a * 0.70f,
                          layer,
                          order);
        }
    }
    for (int row = 0; row < 7; ++row) {
        const char* bits = GlyphRow(c, row);
        for (int col = 0; col < 5; ++col) {
            if (bits[col] == '.') {
                continue;
            }
            SubmitRectPsx(ctx,
                          vx,
                          vy,
                          vs,
                          x + static_cast<float>(col) * px,
                          y + static_cast<float>(row) * px,
                          px,
                          px,
                          r,
                          g,
                          b,
                          a,
                          layer,
                          order);
        }
    }
    return px * 6.0f;
}

float DrawPixelText(PrGameContext& ctx,
                    float vx,
                    float vy,
                    float vs,
                    float x,
                    float y,
                    const char* text,
                    float targetHeight,
                    float r,
                    float g,
                    float b,
                    float a,
                    int layer,
                    int& order) {
    const float px = targetHeight / 7.0f;
    float cx = x;
    for (const char* p = text; p && *p; ++p) {
        cx += DrawPixelGlyph(
            ctx, vx, vy, vs, cx, y, *p, px, r, g, b, a, layer, order);
    }
    return cx - x;
}

float DrawScoreDigit(PrGameContext& ctx,
                     float vx,
                     float vy,
                     float vs,
                     const PrStage1HudPresentationDirectTemplates& templates,
                     float x,
                     float y,
                     int digit,
                     float scale,
                     float r,
                     float g,
                     float b,
                     float a,
                     int layer,
                     int& order) {
    if (digit < 0 || digit > 9 || templates.scoreDigit.w == 0u ||
        templates.scoreDigit.h == 0u) {
        return 0.0f;
    }

    (void)PrPsxSpriteTemplateRender::DrawPsxSpriteTemplateSubrectScaled(
        ctx,
        vx,
        vy,
        vs,
        x,
        y,
        templates.scoreDigit,
        digit * static_cast<int>(templates.scoreDigit.w),
        static_cast<int>(templates.scoreDigit.w),
        0,
        scale,
        scale,
        r,
        g,
        b,
        a,
        layer,
        order++);
    return static_cast<float>(templates.scoreDigit.w) * scale;
}

float DrawSignedScoreNumber(
    PrGameContext& ctx,
    float vx,
    float vy,
    float vs,
    const PrStage1HudPresentationDirectTemplates& templates,
    float x,
    float y,
    int32_t value,
    bool showPositiveSign,
    float scoreScale,
    float r,
    float g,
    float b,
    float a,
    int layer,
    int& order) {
    float cx = x;
    const float glyphHeight =
        static_cast<float>(templates.scoreDigit.h) * scoreScale;
    if (value < 0 || (showPositiveSign && value > 0)) {
        cx += DrawPixelGlyph(ctx,
                             vx,
                             vy,
                             vs,
                             cx,
                             y,
                             value < 0 ? '-' : '+',
                             glyphHeight / 7.0f,
                             r,
                             g,
                             b,
                             a,
                             layer,
                             order) +
              0.5f;
    }

    const uint32_t absValue =
        value < 0 ? static_cast<uint32_t>(-static_cast<int64_t>(value))
                  : static_cast<uint32_t>(value);
    char digits[16] = {};
    std::snprintf(digits, sizeof(digits), "%u", absValue);
    for (const char* p = digits; *p; ++p) {
        if (*p < '0' || *p > '9') {
            continue;
        }
        cx += DrawScoreDigit(ctx,
                             vx,
                             vy,
                             vs,
                             templates,
                             cx,
                             y,
                             *p - '0',
                             scoreScale,
                             r,
                             g,
                             b,
                             a,
                             layer,
                             order) +
              0.5f;
    }
    return cx - x;
}

bool DrawSubscoreLabelTexture(PrGameContext& ctx,
                              float vx,
                              float vy,
                              float vs,
                              size_t labelTextureIndex,
                              float x,
                              float y,
                              float targetHeight,
                              float maxWidth,
                              float r,
                              float g,
                              float b,
                              float a,
                              int& order) {
    int imageWidth = 0;
    int imageHeight = 0;
    int cropX = 0;
    int cropY = 0;
    int cropW = 0;
    int cropH = 0;
    ID3D11ShaderResourceView* texture =
        EnsureSubscoreLabelTexture(ctx,
                                   labelTextureIndex,
                                   imageWidth,
                                   imageHeight,
                                   cropX,
                                   cropY,
                                   cropW,
                                   cropH);
    if (texture == nullptr || imageWidth <= 0 || imageHeight <= 0 ||
        cropW <= 0 || cropH <= 0 || targetHeight <= 0.0f) {
        return false;
    }

    float drawH = targetHeight;
    float drawW = drawH * static_cast<float>(cropW) /
                  static_cast<float>(cropH);
    if (maxWidth > 1.0f && drawW > maxWidth) {
        const float s = maxWidth / drawW;
        drawW *= s;
        drawH *= s;
    }

    D3D11Renderer::SpriteCmd cmd{};
    cmd.texture = texture;
    cmd.x = vx + x * vs;
    cmd.y = vy + y * vs;
    cmd.w = drawW * vs;
    cmd.h = drawH * vs;
    cmd.u0 = static_cast<float>(cropX) / static_cast<float>(imageWidth);
    cmd.v0 = static_cast<float>(cropY) / static_cast<float>(imageHeight);
    cmd.u1 = static_cast<float>(cropX + cropW) /
             static_cast<float>(imageWidth);
    cmd.v1 = static_cast<float>(cropY + cropH) /
             static_cast<float>(imageHeight);
    cmd.r = r;
    cmd.g = g;
    cmd.b = b;
    cmd.a = a;
    cmd.layer = kHudLayerUnderRankHands;
    cmd.order = static_cast<uint64_t>(order++);
    ctx.renderer->SubmitSprite(cmd);
    return true;
}

void DrawSubscoreItem(
    PrGameContext& ctx,
    float vx,
    float vy,
    float vs,
    const PrStage1HudPresentationDirectTemplates& templates,
    float labelX,
    float valueX,
    float y,
    size_t labelTextureIndex,
    const char* label,
    int32_t value,
    float pulseT,
    int& order) {
    constexpr float kBaseGrey = 0.70f;
    const float bright = std::clamp(pulseT, 0.0f, 1.0f);
    const float r = kBaseGrey + (1.0f - kBaseGrey) * bright;
    const float g = kBaseGrey + (0.95f - kBaseGrey) * bright;
    const float b = kBaseGrey + (0.35f - kBaseGrey) * bright;
    const float scoreScale = 0.5f;
    const float glyphHeight =
        static_cast<float>(templates.scoreDigit.h) * scoreScale;

    const float maxLabelWidth = (std::max)(4.0f, valueX - labelX - 3.0f);
    if (!DrawSubscoreLabelTexture(ctx,
                                  vx,
                                  vy,
                                  vs,
                                  labelTextureIndex,
                                  labelX,
                                  y,
                                  glyphHeight,
                                  maxLabelWidth,
                                  r,
                                  g,
                                  b,
                                  1.0f,
                                  order)) {
        (void)DrawPixelText(ctx,
                            vx,
                            vy,
                            vs,
                            labelX,
                            y,
                            label,
                            glyphHeight,
                            r,
                            g,
                            b,
                            1.0f,
                            kHudLayerUnderRankHands,
                            order);
    }
    (void)DrawSignedScoreNumber(ctx,
                                vx,
                                vy,
                                vs,
                                templates,
                                valueX,
                                y,
                                value,
                                true,
                                scoreScale,
                                r,
                                g,
                                b,
                                1.0f,
                                kHudLayerUnderRankHands,
                                order);
}

void EnsureInitialSnapshot(PrGameContext& ctx) {
    Stage1P2ScorerHudRuntime& runtime = GetRuntime();
    if (runtime.snapshot.valid) {
        return;
    }

    const PrScn1::Stage1NumericRuntimeState& numeric =
        PrScn1::s_stage1NumericRuntime;
    runtime.snapshot = Stage1P2ScorerHudSnapshot{};
    runtime.snapshot.valid = true;
    runtime.snapshot.updateFrame = numeric.active ? numeric.queryFrame : ctx.frame;
}

bool UpdateRuntimeLifecycle(PrGameContext& ctx) {
    const PrScn1::Stage1NumericRuntimeState& numeric =
        PrScn1::s_stage1NumericRuntime;
    if ((!ctx.stage1RailParappa2ScorerHud &&
         !ctx.stage1RailParappa2CreativePrompt) ||
        ctx.stage1RailMode != 1 ||
        ctx.currentScene != PrSceneId::Scene1 ||
        !ctx.stageRunning ||
        !numeric.active) {
        ResetRuntime();
        return false;
    }

    Stage1P2ScorerHudRuntime& runtime = GetRuntime();
    if (runtime.lastQueryFrameValid &&
        numeric.queryFrame < runtime.lastQueryFrame) {
        ResetRuntime();
    } else if (runtime.snapshot.valid &&
               numeric.queryFrame < runtime.snapshot.updateFrame) {
        ResetRuntime();
    }

    Stage1P2ScorerHudRuntime& refreshed = GetRuntime();
    refreshed.lastQueryFrameValid = true;
    refreshed.lastQueryFrame = numeric.queryFrame;
    return true;
}

void UpdateSnapshot(PrGameContext& ctx) {
    if (!ctx.stage1RailParappa2ScorerHud ||
        ctx.stage1RailMode != 1 ||
        ctx.currentScene != PrSceneId::Scene1 ||
        !ctx.stageRunning) {
        return;
    }

    const PrScn1::Stage1NumericRuntimeState& numeric =
        PrScn1::s_stage1NumericRuntime;
    if (!numeric.active || !numeric.bucketCadence.bucket30Advanced) {
        return;
    }

    const auto& owner = numeric.bucket30OwnerObserver;
    if (!owner.scorerCommitWindowOpen ||
        !owner.gameplayCommitTermKnown ||
        !owner.gameplayDescriptorSubdeltaKnown) {
        return;
    }

    Stage1P2ScorerHudRuntime& runtime = GetRuntime();
    const int32_t rhyme =
        owner.gameplayPairBonusKnown ? owner.gameplayPairBonus : 0;
    const int32_t drop =
        owner.gameplaySpillPenaltyKnown ? owner.gameplaySpillPenalty : 0;
    const int32_t flow = owner.gameplayDescriptorSubdelta - rhyme - drop;
    const int32_t hype =
        owner.gameplayAdditiveTermKnown ? owner.gameplayAdditiveTerm : 0;

    Stage1P2ScorerHudSnapshot next{};
    next.valid = true;
    next.updateFrame = numeric.queryFrame;
    next.flow = flow;
    next.rhyme = rhyme;
    next.drop = drop;
    next.hype = hype;
    next.mix = owner.gameplayCommitTerm;
    runtime.snapshot = next;

    const int32_t values[4] = {
        next.flow,
        next.rhyme,
        next.drop,
        next.hype,
    };
    for (size_t i = 0; i < 4u; ++i) {
        runtime.pulseActive[i] = values[i] > 0;
        runtime.pulseStartFrame[i] = numeric.queryFrame;
    }
    runtime.totalPulseActive = next.mix != 0;
    runtime.totalPulseStartFrame = numeric.queryFrame;
}

float PulseT(uint32_t nowFrame,
             uint32_t startFrame,
             bool active,
             uint32_t pulseFrames) {
    if (!active || nowFrame < startFrame) {
        return 0.0f;
    }
    const uint32_t age = nowFrame - startFrame;
    if (age >= pulseFrames) {
        return 0.0f;
    }
    return 1.0f - static_cast<float>(age) / static_cast<float>(pulseFrames);
}

int CountScoreDigits(int32_t value) {
    if (value == 0) {
        return 1;
    }
    int64_t v = value < 0 ? -static_cast<int64_t>(value)
                          : static_cast<int64_t>(value);
    int count = 0;
    while (v > 0) {
        v /= 10;
        ++count;
    }
    return (std::max)(1, count);
}

void DrawTotalDelta(PrGameContext& ctx,
                    float vx,
                    float vy,
                    float vs,
                    const PrStage1HudPresentationDirectTemplates& templates,
                    bool highLayoutMode,
                    float scorePanelOffsetX,
                    int32_t scoreDisplayValue,
                    int32_t totalDelta,
                    float pulseT,
                    int& order) {
    if (totalDelta == 0 || pulseT <= 0.0f) {
        return;
    }

    const float yOffset = highLayoutMode ? 0.0f : 30.0f;
    const int digitCount = CountScoreDigits(scoreDisplayValue);
    const float mainScoreDigitsX = 58.0f + scorePanelOffsetX;
    const float mainScoreY = 176.0f + yOffset;
    const float deltaX = mainScoreDigitsX + 9.0f * static_cast<float>(digitCount) + 3.0f;
    const float deltaY = mainScoreY + 2.0f;
    const float r = totalDelta >= 0 ? 1.0f : 1.0f;
    const float g = totalDelta >= 0 ? 0.95f : 0.42f;
    const float b = totalDelta >= 0 ? 0.35f : 0.32f;

    (void)DrawSignedScoreNumber(ctx,
                                vx,
                                vy,
                                vs,
                                templates,
                                deltaX,
                                deltaY,
                                totalDelta,
                                true,
                                0.5f,
                                r,
                                g,
                                b,
                                pulseT,
                                kHudLayerUnderRankHands,
                                order);
}

bool IsCreativePromptDescriptor(
    const PrScn1::Stage1NumericRuntimeState::DescriptorRowView& row) {
    if (!row.available) {
        return false;
    }

    constexpr uint32_t kGameplayMask =
        0x0010u | 0x0020u | 0x0040u | 0x0080u | 0x0004u | 0x0008u;
    uint32_t v = row.requiredMask & kGameplayMask;
    int count = 0;
    while (v != 0u) {
        v &= v - 1u;
        ++count;
    }
    return count >= 2;
}

bool IsStage1CreativePromptRankSuppressed(
    const PrScn1::Stage1NumericRuntimeState& numeric) {
    const uint8_t row = numeric.rightRankState.rightRankActiveRow;
    return row == 0u || row >= 3u;
}

bool IsStage1TerminalEventForCreativePrompt(
    const PrScn1::Stage1NumericRuntimeState& numeric) {
    if (numeric.psxFlag100BlocksWaitPulse &&
        numeric.psxFlag100BlocksWaitSourceStream >= 2u &&
        numeric.psxFlag100BlocksWaitSourceStream <= 5u) {
        return true;
    }
    if (numeric.runnerTailHost7A60.requestFailCue943EC) {
        return true;
    }
    return false;
}

bool IsStage1TerminalLifecycleForCreativePrompt() {
    PrStage1FormalLifecycleSnapshot snapshot{};
    if (PrScn1::CopyStage1FormalLifecycleSnapshot(snapshot) &&
        (snapshot.clearGate || snapshot.clearTerminalTailGate ||
         snapshot.failGate)) {
        return true;
    }
    if (PrScn1::CopyStage1TerminalFormalLifecycleSnapshot(snapshot) &&
        (snapshot.clearGate || snapshot.clearTerminalTailGate ||
         snapshot.failGate)) {
        return true;
    }
    return false;
}

void ClearCreativePromptRuntime(Stage1P2ScorerHudRuntime& runtime) {
    runtime.creativePromptVisible = false;
    runtime.creativePromptLeaving = false;
    runtime.creativePromptSettledValid = false;
    runtime.creativePromptKey = 0u;
    runtime.creativePromptSettledKey = 0u;
    runtime.creativePromptStartFrame = 0u;
    runtime.creativePromptLeaveFrame = 0u;
}

bool ResolveCreativePromptKey(const PrGameContext& ctx,
                              const PrScn1::Stage1NumericRuntimeState& numeric,
                              uint32_t& outKey) {
    (void)ctx;
    const auto& cadence = numeric.descriptorCadence;
    if (cadence.currentDescriptor40Available &&
        IsCreativePromptDescriptor(cadence.currentDescriptor40Row)) {
        outKey = static_cast<uint32_t>(cadence.currentDescriptor40RowIndex);
        return true;
    }
    if (cadence.lookaheadDescriptor44Available &&
        IsCreativePromptDescriptor(cadence.lookaheadDescriptor44Row)) {
        outKey = static_cast<uint32_t>(cadence.nextLookaheadRowIndex);
        return true;
    }
    return false;
}

bool ResolveCurrentCreativePromptKey(
    const PrGameContext& ctx,
    const PrScn1::Stage1NumericRuntimeState& numeric,
    uint32_t& outKey) {
    (void)ctx;
    const auto& cadence = numeric.descriptorCadence;
    if (!cadence.currentDescriptor40Available ||
        !IsCreativePromptDescriptor(cadence.currentDescriptor40Row)) {
        return false;
    }
    outKey = static_cast<uint32_t>(cadence.currentDescriptor40RowIndex);
    return true;
}

void StartCreativePromptLeave(Stage1P2ScorerHudRuntime& runtime,
                              uint32_t frame) {
    if (!runtime.creativePromptVisible || runtime.creativePromptLeaving) {
        return;
    }
    runtime.creativePromptLeaving = true;
    runtime.creativePromptLeaveFrame = frame;
}

void UpdateCreativePromptState(PrGameContext& ctx) {
    Stage1P2ScorerHudRuntime& runtime = GetRuntime();
    if (!ctx.stage1RailParappa2CreativePrompt) {
        ClearCreativePromptRuntime(runtime);
        return;
    }

    const PrScn1::Stage1NumericRuntimeState& numeric =
        PrScn1::s_stage1NumericRuntime;
    if (IsStage1CreativePromptRankSuppressed(numeric) ||
        IsStage1TerminalEventForCreativePrompt(numeric) ||
        IsStage1TerminalLifecycleForCreativePrompt()) {
        ClearCreativePromptRuntime(runtime);
        return;
    }

    uint32_t key = 0u;
    bool wantsPrompt = ResolveCreativePromptKey(ctx, numeric, key);
    if (wantsPrompt &&
        runtime.creativePromptSettledValid &&
        runtime.creativePromptSettledKey == key) {
        wantsPrompt = false;
    }

    if (wantsPrompt) {
        if (!runtime.creativePromptVisible ||
            runtime.creativePromptLeaving ||
            runtime.creativePromptKey != key) {
            runtime.creativePromptVisible = true;
            runtime.creativePromptLeaving = false;
            runtime.creativePromptKey = key;
            runtime.creativePromptStartFrame = ctx.frame;
            if (runtime.creativePromptSettledKey != key) {
                runtime.creativePromptSettledValid = false;
            }
        }
        if (ctx.frame > runtime.creativePromptStartFrame &&
            ctx.frame - runtime.creativePromptStartFrame >= 240u) {
            runtime.creativePromptSettledValid = true;
            runtime.creativePromptSettledKey = runtime.creativePromptKey;
            StartCreativePromptLeave(runtime, ctx.frame);
        }
        return;
    }

    if (runtime.creativePromptVisible &&
        !runtime.creativePromptLeaving &&
        ctx.frame > runtime.creativePromptStartFrame &&
        ctx.frame - runtime.creativePromptStartFrame >= 240u) {
        runtime.creativePromptSettledValid = true;
        runtime.creativePromptSettledKey = runtime.creativePromptKey;
    }

    StartCreativePromptLeave(runtime, ctx.frame);
    if (runtime.creativePromptLeaving &&
        ctx.frame - runtime.creativePromptLeaveFrame >= 28u) {
        runtime.creativePromptVisible = false;
        runtime.creativePromptLeaving = false;
    }
}

void MarkCreativePromptSettled(PrGameContext& ctx) {
    if (!ctx.stage1RailParappa2CreativePrompt ||
        !PrScn1::s_stage1NumericRuntime.bucketCadence.bucket30Advanced) {
        return;
    }

    Stage1P2ScorerHudRuntime& runtime = GetRuntime();
    uint32_t key = 0u;
    if (!ResolveCurrentCreativePromptKey(ctx,
                                         PrScn1::s_stage1NumericRuntime,
                                         key)) {
        return;
    }
    if (!runtime.creativePromptVisible || runtime.creativePromptKey != key) {
        return;
    }
    if (ctx.frame <= runtime.creativePromptStartFrame) {
        return;
    }

    runtime.creativePromptSettledValid = true;
    runtime.creativePromptSettledKey = key;
    StartCreativePromptLeave(runtime, ctx.frame);
}

void ResolveCreativePromptAnimation(const Stage1P2ScorerHudRuntime& runtime,
                                    uint32_t frame,
                                    float& outScaleX,
                                    float& outScaleY,
                                    float& outAlpha) {
    constexpr float kPi = 3.14159265358979323846f;
    outScaleX = 1.0f;
    outScaleY = 1.0f;
    outAlpha = 1.0f;

    if (runtime.creativePromptLeaving) {
        const float t = std::clamp(
            static_cast<float>(frame - runtime.creativePromptLeaveFrame) /
                28.0f,
            0.0f,
            1.0f);
        const float scale =
            (std::max)(0.12f,
                       1.0f + 0.62f * std::sin(kPi * t) - 0.86f * t * t);
        outScaleX = scale;
        outScaleY = scale;
        outAlpha = 1.0f - t;
        return;
    }

    const float t = std::clamp(
        static_cast<float>(frame - runtime.creativePromptStartFrame) /
            40.0f,
        0.0f,
        1.0f);
    const float bounce = std::exp(-3.2f * t) * std::sin(4.0f * kPi * t);
    const float settle = std::exp(-8.0f * t);
    const float scale = 1.0f - 0.55f * settle + 0.30f * bounce;
    outScaleX = scale * (1.0f + 0.075f * bounce);
    outScaleY = scale * (1.0f - 0.095f * bounce);
}

void DrawCreativePrompt(PrGameContext& ctx,
                        float vx,
                        float vy,
                        float vs) {
    Stage1P2ScorerHudRuntime& runtime = GetRuntime();
    if (!runtime.creativePromptVisible) {
        return;
    }

    int imageWidth = 0;
    int imageHeight = 0;
    ID3D11ShaderResourceView* texture =
        EnsureCreativePromptTexture(ctx, imageWidth, imageHeight);
    if (texture == nullptr || imageWidth <= 0 || imageHeight <= 0) {
        return;
    }

    float scaleX = 1.0f;
    float scaleY = 1.0f;
    float alpha = 1.0f;
    ResolveCreativePromptAnimation(runtime,
                                   ctx.frame,
                                   scaleX,
                                   scaleY,
                                   alpha);
    if (alpha <= 0.0f) {
        return;
    }

    constexpr float kPromptBaseWidthPsx = 86.0f;
    constexpr float kPromptCenterXPsx = 160.0f;
    constexpr float kPromptYBelowRailPsx = 42.0f;
    const float aspect =
        static_cast<float>(imageHeight) / static_cast<float>(imageWidth);
    const float baseHeight = kPromptBaseWidthPsx * aspect;
    const float drawW = kPromptBaseWidthPsx * scaleX;
    const float drawH = baseHeight * scaleY;
    const float drawX = kPromptCenterXPsx - drawW * 0.5f;
    const float drawY = kPromptYBelowRailPsx - (drawH - baseHeight) * 0.5f;

    D3D11Renderer::SpriteCmd cmd{};
    cmd.texture = texture;
    cmd.x = vx + drawX * vs;
    cmd.y = vy + drawY * vs;
    cmd.w = drawW * vs;
    cmd.h = drawH * vs;
    cmd.a = std::clamp(alpha, 0.0f, 1.0f);
    cmd.layer = kCreativePromptLayerNearRail;
    cmd.order = 500u;
    ctx.renderer->SubmitSprite(cmd);
}

}  // namespace

void Draw(PrGameContext& ctx,
          float vx,
          float vy,
          float vs,
          bool highLayoutMode,
          float scorePanelOffsetX,
          int32_t scoreDisplayValue,
          const PrStage1HudPresentationDirectTemplates& templates) {
    if (!UpdateRuntimeLifecycle(ctx)) {
        return;
    }

    UpdateSnapshot(ctx);
    EnsureInitialSnapshot(ctx);
    UpdateCreativePromptState(ctx);
    MarkCreativePromptSettled(ctx);

    Stage1P2ScorerHudRuntime& runtime = GetRuntime();
    const Stage1P2ScorerHudSnapshot& snapshot = runtime.snapshot;
    const PrScn1::Stage1NumericRuntimeState& numeric =
        PrScn1::s_stage1NumericRuntime;
    const uint32_t nowFrame = numeric.active ? numeric.queryFrame
                                             : snapshot.updateFrame;
    const float yOffset = highLayoutMode ? 0.0f : 30.0f;
    const float scoreLabelX = 20.0f + scorePanelOffsetX;
    const float groupX = scoreLabelX - 20.0f;
    const float labelX = groupX;
    const float valueX = groupX + 36.0f;
    const float rowY = 142.0f + yOffset;
    const float rowStepY = 8.0f;
    int order = 300;

    DrawCreativePrompt(ctx, vx, vy, vs);
    if (!ctx.stage1RailParappa2ScorerHud) {
        return;
    }

    DrawSubscoreItem(ctx,
                     vx,
                     vy,
                     vs,
                     templates,
                     labelX,
                     valueX,
                     rowY + rowStepY * 0.0f,
                     0u,
                     "FLOW",
                     snapshot.flow,
                     PulseT(nowFrame,
                            runtime.pulseStartFrame[0],
                            runtime.pulseActive[0],
                            24u),
                     order);
    DrawSubscoreItem(ctx,
                     vx,
                     vy,
                     vs,
                     templates,
                     labelX,
                     valueX,
                     rowY + rowStepY * 1.0f,
                     1u,
                     "RHYME",
                     snapshot.rhyme,
                     PulseT(nowFrame,
                            runtime.pulseStartFrame[1],
                            runtime.pulseActive[1],
                            24u),
                     order);
    DrawSubscoreItem(ctx,
                     vx,
                     vy,
                     vs,
                     templates,
                     labelX,
                     valueX,
                     rowY + rowStepY * 2.0f,
                     2u,
                     "DROP",
                     snapshot.drop,
                     PulseT(nowFrame,
                            runtime.pulseStartFrame[2],
                            runtime.pulseActive[2],
                            24u),
                     order);
    DrawSubscoreItem(ctx,
                     vx,
                     vy,
                     vs,
                     templates,
                     labelX,
                     valueX,
                     rowY + rowStepY * 3.0f,
                     3u,
                     "HYPE",
                     snapshot.hype,
                     PulseT(nowFrame,
                            runtime.pulseStartFrame[3],
                            runtime.pulseActive[3],
                            24u),
                     order);

    DrawTotalDelta(ctx,
                   vx,
                   vy,
                   vs,
                   templates,
                   highLayoutMode,
                   scorePanelOffsetX,
                   scoreDisplayValue,
                   snapshot.mix,
                   PulseT(nowFrame,
                          runtime.totalPulseStartFrame,
                          runtime.totalPulseActive,
                          36u),
                   order);
}

}  // namespace PrStage1P2ScorerHud
