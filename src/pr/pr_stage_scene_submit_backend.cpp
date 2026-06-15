#include "pr_stage_scene_submit_backend.h"

#include "d3d11_renderer.h"
#include "logger.h"
#include "pr_game_context.h"
#include "pr_mime.h"
#include "pr_pad.h"
#include "pr_psx_event_frame_direct.h"
#include "pr_psx_fast_sprite_submit_direct.h"
#include "pr_psx_sprite_template_render.h"
#include "pr_stage1_scorer_host.h"
#include "pr_stage1_scorer_host_deps.h"
#include "pr_stage1_overlay_parser.h"
#include "pr_stage1_scene1_movie1_direct.h"
#include "pr_stage1_hd_subtitles.h"
#include "pr_stage1_texture_replacements.h"
#include "pr_stage1_movie_text_direct.h"
#include "pr_stage1_script_box_direct.h"
#include "pr_stage_runner.h"
#include "pr_scn1.h"
#include "pr_stage_scene_submit_debug.h"
#include "pr_stage_scene_submit_draw_private.h"
#include "pr_tmd.h"
#include "pr_vram_atlas.h"
#include "resource_manager.h"
#include "str_player.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <cstddef>
#include <cstdio>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace PrStageSceneSubmitBackend {
namespace {

static const char* kStage1TmdNames[] = {
    "tatami.tmd",
    "kage.tmd",
    "tama.tmd",
    "on.tmd",
    "pa.tmd",
};
static const int kStage1TmdCount = 5;
static constexpr int kStage1OnIndex = 3;
static constexpr int kStage1PaIndex = 4;

static TmdModel s_stage1Models[kStage1TmdCount];
static bool s_stage1Loaded[kStage1TmdCount] = {};
static PrStageSceneSubmitDirect::Stage1SceneSubmitRuntime
    s_stage1RenderOnlyRuntime801CBFDC190;
static bool s_stage1UseRenderOnlyRuntime801CBFDC190 = false;
static uint32_t s_stage1TmdRenderOnlyBlendPauseFrames801CBFDC190 = 0;
static bool s_stage1TmdBlendLastRightRankKnown801CBFDC190 = false;
static uint8_t s_stage1TmdBlendLastRightRankRow801CBFDC190 = 0u;
static uint32_t s_stage1TmdBlendLastRankTriggerMask801CBFDC190 = 0u;

static const char* kStage1SceneMapTmdNames[] = {
    "yuka.tmd",
    "enngawa.tmd",
    "isdn.tmd",
    "mado_l.tmd",
    "mado_r.tmd",
    "kamon_l.tmd",
    "kamon_r.tmd",
    "tourou.tmd",
    "tennjou.tmd",
    "kabe_r.tmd",
    "kabe_l.tmd",
    "kabe_b.tmd",
    "nin_ten.tmd",
    "nin_ttm.tmd",
    "nin_shj.tmd",
    "waku_l.tmd",
    "waku_r.tmd",
    "donde_l.tmd",
    "donde_r.tmd",
    "nin_l.tmd",
    "nin_r.tmd",
};
static const int kStage1SceneMapTmdCount = 21;

static TmdModel s_stage1SceneMapModels[kStage1SceneMapTmdCount];
static bool s_stage1SceneMapLoaded[kStage1SceneMapTmdCount] = {};

static PsxVramAtlas s_stage1VramAtlas;
static bool s_stage1AtlasReady = false;
static ID3D11ShaderResourceView* s_stage1Parappa2RailGlowTexture = nullptr;
static D3D11Renderer* s_stage1Parappa2RailGlowRenderer = nullptr;

template <size_t N>
static void ResetModelSet(TmdModel (&models)[N], bool (&loaded)[N]) {
    for (size_t i = 0; i < N; ++i) {
        models[i] = TmdModel{};
        loaded[i] = false;
    }
}

template <size_t N>
static bool LoadTmdSet(ResourceManager* resources,
                       const char* const (&names)[N],
                       const char* logPrefix,
                       TmdModel (&models)[N],
                       bool (&loaded)[N]) {
    bool anyLoaded = false;
    for (size_t i = 0; i < N; ++i) {
        loaded[i] = false;
        const std::vector<uint8_t>* mem = resources->GetMem(names[i]);
        if (!mem || mem->empty()) {
            continue;
        }

        models[i] = TmdModel{};
        if (!TmdParser::Parse(mem->data(), mem->size(), models[i])) {
            continue;
        }

        loaded[i] = true;
        anyLoaded = true;
        int totalVerts = 0;
        int totalPrims = 0;
        int16_t minX = 32767;
        int16_t maxX = -32768;
        int16_t minY = 32767;
        int16_t maxY = -32768;
        int16_t minZ = 32767;
        int16_t maxZ = -32768;
        for (const auto& obj : models[i].objects) {
            totalVerts += static_cast<int>(obj.vertices.size());
            totalPrims += static_cast<int>(obj.primitives.size());
            for (const auto& v : obj.vertices) {
                if (v.x < minX) minX = v.x;
                if (v.x > maxX) maxX = v.x;
                if (v.y < minY) minY = v.y;
                if (v.y > maxY) maxY = v.y;
                if (v.z < minZ) minZ = v.z;
                if (v.z > maxZ) maxZ = v.z;
            }
        }
        Log::Printf("%s loaded: %s objs=%d verts=%d prims=%d bbox=[%d..%d, %d..%d, %d..%d]",
                    logPrefix,
                    names[i],
                    static_cast<int>(models[i].objects.size()),
                    totalVerts,
                    totalPrims,
                    minX,
                    maxX,
                    minY,
                    maxY,
                    minZ,
                    maxZ);
    }
    return anyLoaded;
}

template <size_t N>
static void RegisterModelSetTpages(const TmdModel (&models)[N],
                                   const bool (&loaded)[N]) {
    for (size_t i = 0; i < N; ++i) {
        if (!loaded[i]) {
            continue;
        }
        for (const auto& obj : models[i].objects) {
            for (const auto& prim : obj.primitives) {
                if (prim.textured) {
                    s_stage1VramAtlas.RegisterTpage(prim.tpage);
                    s_stage1VramAtlas.RegisterClut(prim.clut);
                }
            }
        }
    }
}

static uint16_t PackPsxClutCba(uint16_t clutX, uint16_t clutY) {
    return static_cast<uint16_t>(
        (((static_cast<uint32_t>(clutY) & 0x01FFu) << 6u) |
         ((static_cast<uint32_t>(clutX) >> 4u) & 0x3Fu)));
}

static void RegisterStage1TextGlyphTpages8001B954() {
    // 8001B954 glyph packets build this tpage via 80043DF4(0,1,a3,a4).
    constexpr uint16_t kGlyphTpageSub8001B954 = 0x003Eu;
    constexpr uint16_t kGlyphClutYSub8001B954 = 480u;

    s_stage1VramAtlas.RegisterTpage(kGlyphTpageSub8001B954);
    s_stage1VramAtlas.RegisterClut(
        PackPsxClutCba(
            PrStage1MovieTextDirect::kMovie1TextGlyphFastSpriteClutXSub8001B954,
            kGlyphClutYSub8001B954));
}

static uint16_t ResolveFastSpritePacketTpage8003FA20(uint32_t attr,
                                                     uint16_t localTpage) {
    return static_cast<uint16_t>((static_cast<uint32_t>(localTpage) & 0x1Fu) |
                                 ((attr >> 17) & 0x180u) |
                                 ((attr >> 23) & 0x60u));
}

static void RegisterStage1ScriptBoxCornerTpages8001C550() {
    constexpr std::array<uint32_t, 4> kCornerTemplates = {{
        0x80050900u,
        0x800508F0u,
        0x800508E0u,
        0x800508D0u,
    }};
    for (uint32_t templateAddr : kCornerTemplates) {
        const PrStage1Scene1Movie1Direct::Movie1PsxTemplateDesc desc =
            PrStage1Scene1Movie1Direct::ResolveMovie1PsxTemplateDesc(
                templateAddr);
        if (!desc.valid) {
            continue;
        }
        const PrStage1Scene1Movie1Direct::Movie1PsxRawDrawB25CMetadata b25c =
            PrStage1Scene1Movie1Direct::BuildRawDrawTileRectFieldsSub8001B25C(
                templateAddr,
                desc,
                0u,
                false);
        if (!b25c.valid) {
            continue;
        }
        s_stage1VramAtlas.RegisterTpage(
            ResolveFastSpritePacketTpage8003FA20(b25c.copiedAttr,
                                                 b25c.computedClutWord));
        s_stage1VramAtlas.RegisterClut(
            PackPsxClutCba(b25c.sourceWord12,
                           b25c.finalWord));
    }
}

static D3D11Renderer::BlendMode ResolveBlendMode(uint8_t blend) {
    switch (blend) {
        case 1u:
            return D3D11Renderer::BlendMode::Additive;
        case 2u:
            return D3D11Renderer::BlendMode::Subtractive;
        case 0u:
        default:
            return D3D11Renderer::BlendMode::Alpha;
    }
}

static const char* Stage1BackdropTextureName801CAC34(
    PrStageSceneSubmitDirect::PsxBackdropSpriteCommand801CAC34::Texture texture) {
    using Texture =
        PrStageSceneSubmitDirect::PsxBackdropSpriteCommand801CAC34::Texture;
    switch (texture) {
        case Texture::YuKumo:
            return "yu_kumo";
        case Texture::YuYama:
            return "yu_yama";
        case Texture::YuHi:
            return "yu_hi";
        case Texture::OnKao:
            return "on_kao";
        case Texture::YuJhan:
            return "yu_jhan";
    }
    return "";
}

struct Stage1BackdropAtlasSample801CAC34 {
    ID3D11ShaderResourceView* srv = nullptr;
    float width = 0.0f;
    float height = 0.0f;
    float u0 = 0.0f;
    float v0 = 0.0f;
    float u1 = 1.0f;
    float v1 = 1.0f;
};

static uint16_t NormalizeStage1BackdropTpageKey801CAC34(uint16_t tpage) {
    return static_cast<uint16_t>((tpage & 0x01FFu) & ~0x0060u);
}

static bool ResolveStage1BackdropTimAtlasPlacement801CAC34(
    const TimImage& tim,
    uint16_t& outTpage,
    uint16_t& outClut,
    float& outU0,
    float& outV0,
    float& outU1,
    float& outV1) {
    if ((tim.bpp != 4u && tim.bpp != 8u) ||
        tim.width == 0u ||
        tim.height == 0u ||
        tim.clutW == 0u ||
        tim.clutH == 0u ||
        tim.orgX < 0 ||
        tim.orgY < 0) {
        return false;
    }

    const int colorMode = (tim.bpp == 4u) ? 0 : 1;
    const int pixelsPerHalfword = (tim.bpp == 4u) ? 4 : 2;
    const int tpageIdx = static_cast<int>(tim.orgX) / 64;
    const int tpageY = (tim.orgY >= 256) ? 1 : 0;
    if (tpageIdx < 0 || tpageIdx > 15) {
        return false;
    }

    const uint16_t tpage = NormalizeStage1BackdropTpageKey801CAC34(
        static_cast<uint16_t>(tpageIdx |
                              (tpageY << 4) |
                              (colorMode << 7)));
    const int baseHW = (tpage & 0x0F) * 64;
    const int baseY = ((tpage >> 4) & 0x01) * 256;
    const int pixelX = (static_cast<int>(tim.orgX) - baseHW) *
                       pixelsPerHalfword;
    const int pixelY = static_cast<int>(tim.orgY) - baseY;
    if (pixelX < 0 ||
        pixelY < 0 ||
        pixelX + static_cast<int>(tim.width) > PsxVramAtlas::TPAGE_W ||
        pixelY + static_cast<int>(tim.height) > PsxVramAtlas::TPAGE_H) {
        return false;
    }

    outTpage = tpage;
    outClut = PackPsxClutCba(static_cast<uint16_t>(tim.clutX),
                             static_cast<uint16_t>(tim.clutY));
    outU0 = static_cast<float>(pixelX) /
            static_cast<float>(PsxVramAtlas::TPAGE_W);
    outV0 = static_cast<float>(pixelY) /
            static_cast<float>(PsxVramAtlas::TPAGE_H);
    outU1 = static_cast<float>(pixelX + static_cast<int>(tim.width)) /
            static_cast<float>(PsxVramAtlas::TPAGE_W);
    outV1 = static_cast<float>(pixelY + static_cast<int>(tim.height)) /
            static_cast<float>(PsxVramAtlas::TPAGE_H);
    return true;
}

static bool ResolveStage1BackdropAtlasSample801CAC34(
    D3D11Renderer* renderer,
    const TextureResource& texture,
    Stage1BackdropAtlasSample801CAC34& outSample) {
    uint16_t tpage = 0u;
    uint16_t clut = 0u;
    float u0 = 0.0f;
    float v0 = 0.0f;
    float u1 = 1.0f;
    float v1 = 1.0f;
    if (!ResolveStage1BackdropTimAtlasPlacement801CAC34(
            texture.tim,
            tpage,
            clut,
            u0,
            v0,
            u1,
            v1)) {
        return false;
    }

    ID3D11ShaderResourceView* srv =
        s_stage1VramAtlas.GetTpageSRV(tpage, clut, renderer);
    if (srv == nullptr) {
        return false;
    }

    outSample.srv = srv;
    outSample.width = static_cast<float>(texture.tim.width);
    outSample.height = static_cast<float>(texture.tim.height);
    outSample.u0 = u0;
    outSample.v0 = v0;
    outSample.u1 = u1;
    outSample.v1 = v1;
    return true;
}

static void RegisterStage1BackdropTextureTpages801CAC34(
    ResourceManager* resources) {
    if (resources == nullptr) {
        return;
    }

    using Texture =
        PrStageSceneSubmitDirect::PsxBackdropSpriteCommand801CAC34::Texture;
    constexpr Texture kBackdropTextures[] = {
        Texture::YuKumo,
        Texture::YuYama,
        Texture::YuHi,
        Texture::OnKao,
        Texture::YuJhan,
    };

    for (Texture texture : kBackdropTextures) {
        const char* textureName = Stage1BackdropTextureName801CAC34(texture);
        TextureResource* resource = resources->GetTexture(textureName);
        if (resource == nullptr) {
            continue;
        }

        uint16_t tpage = 0u;
        uint16_t clut = 0u;
        float u0 = 0.0f;
        float v0 = 0.0f;
        float u1 = 1.0f;
        float v1 = 1.0f;
        if (!ResolveStage1BackdropTimAtlasPlacement801CAC34(
                resource->tim,
                tpage,
                clut,
                u0,
                v0,
                u1,
                v1)) {
            continue;
        }

        s_stage1VramAtlas.RegisterTpage(tpage);
        s_stage1VramAtlas.RegisterClut(clut);
    }
}

static std::string Stage1SceneToLowerAsciiCopy(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return s;
}

static bool Stage1SceneStartsWithAscii(const std::string& s,
                                       const char* prefix) {
    if (prefix == nullptr) {
        return false;
    }
    const size_t prefixLen = std::strlen(prefix);
    return s.size() >= prefixLen &&
           s.compare(0, prefixLen, prefix) == 0;
}

static std::string Stage1SceneStripAsciiExtension(const std::string& s) {
    const size_t dot = s.rfind('.');
    return (dot == std::string::npos) ? s : s.substr(0, dot);
}

static float PsxColorByteToFloat(uint32_t attr, uint32_t shift) {
    return static_cast<float>((attr >> shift) & 0xFFu) / 255.0f;
}

constexpr std::size_t kStage1PacketTriangleVertexCount428B0 = 3;
constexpr std::size_t kStage1PacketQuadTriangleCount428B0 = 2;
constexpr std::array<
    std::array<uint8_t, kStage1PacketTriangleVertexCount428B0>,
    kStage1PacketQuadTriangleCount428B0>
    kStage1PacketTriangleSlots428B0 = {{
        {0u, 1u, 2u},
        {1u, 3u, 2u},
    }};
constexpr uint8_t kStage1PacketBlendAlpha428B0 = 0;
constexpr uint8_t kStage1PacketBlendAdditive428B0 = 1;
constexpr uint8_t kStage1PacketBlendSubtractive428B0 = 2;

struct Stage1SceneSubmit428B0DebugState {
    bool drawCalled = false;
    bool rendererAvailable = false;
    uint32_t commandCount = 0;
    uint32_t validCount = 0;
    uint32_t gapCount = 0;
    uint32_t specialTableCount = 0;
    bool renderOnlyBlendAttempted = false;
    bool renderOnlyBlendPaused = false;
    uint32_t renderOnlyBlendPauseFrames = 0;
    uint32_t blendCurrentCount = 0;
    uint32_t blendNextCount = 0;
    uint32_t blendMatchedCount = 0;
    uint32_t blendAppliedCount = 0;
    bool blendFirstDeltaKnown = false;
    float blendFirstDeltaX = 0.0f;
    float blendFirstDeltaY = 0.0f;
    bool lastKnown = false;
    PrStageSceneSubmitDirect::PsxGpuPacketCommand428B0 last{};
    bool lastSpecialKnown = false;
    PrStageSceneSubmitDirect::PsxGpuPacketCommand428B0 lastSpecial{};
};

static Stage1SceneSubmit428B0DebugState s_stage1SceneSubmit428B0Debug{};

static std::string Hex8(uint32_t value) {
    char buf[16] = {};
    std::snprintf(buf, sizeof(buf), "0x%08X", value);
    return std::string(buf);
}

static std::string Hex2(uint32_t value) {
    char buf[8] = {};
    std::snprintf(buf, sizeof(buf), "0x%02X", value & 0xFFu);
    return std::string(buf);
}

static const char* Stage1GpuPacketHandlerName428B0(
    PrStageSceneSubmitDirect::PsxGpuPacketHandler428B0 handler) {
    using Handler = PrStageSceneSubmitDirect::PsxGpuPacketHandler428B0;
    switch (handler) {
        case Handler::Unsupported:
            return "Unsupported";
        case Handler::F3NL:
            return "F3NL";
        case Handler::NF3:
            return "NF3";
        case Handler::TF3NL:
            return "TF3NL";
        case Handler::TNF3:
            return "TNF3";
        case Handler::F4NL:
            return "F4NL";
        case Handler::NF4:
            return "NF4";
        case Handler::TF4NL:
            return "TF4NL";
        case Handler::TNF4:
            return "TNF4";
        case Handler::G3NL:
            return "G3NL";
        case Handler::NG3:
            return "NG3";
        case Handler::TG3NL:
            return "TG3NL";
        case Handler::TNG3:
            return "TNG3";
        case Handler::G4NL:
            return "G4NL";
        case Handler::NG4:
            return "NG4";
        case Handler::TG4NL:
            return "TG4NL";
        case Handler::TNG4:
            return "TNG4";
    }
    return "Unknown";
}

static const char* Stage1GpuPacketDispatchGapName428B0(
    PrStageSceneSubmitDirect::PsxGpuPacketDispatchGap428B0 gap) {
    using Gap = PrStageSceneSubmitDirect::PsxGpuPacketDispatchGap428B0;
    switch (gap) {
        case Gap::None:
            return "None";
        case Gap::Flag4SpecialTableUnsupported:
            return "Flag4SpecialTableUnsupported";
    }
    return "Unknown";
}

static bool Stage1GpuPacketIsSpecialTableGap428B0(
    const PrStageSceneSubmitDirect::PsxGpuPacketCommand428B0& command) {
    return command.specialTablePath800428B0 ||
           command.dispatchGap800428B0 ==
               PrStageSceneSubmitDirect::PsxGpuPacketDispatchGap428B0::
                   Flag4SpecialTableUnsupported;
}

static void BeginStage1SceneSubmit428B0Debug(bool rendererAvailable) {
    s_stage1SceneSubmit428B0Debug = Stage1SceneSubmit428B0DebugState{};
    s_stage1SceneSubmit428B0Debug.drawCalled = true;
    s_stage1SceneSubmit428B0Debug.rendererAvailable = rendererAvailable;
}

static void ObserveStage1SceneSubmit428B0Debug(
    const PrStageSceneSubmitDirect::PsxGpuPacketCommand428B0& command) {
    ++s_stage1SceneSubmit428B0Debug.commandCount;
    if (command.valid) {
        ++s_stage1SceneSubmit428B0Debug.validCount;
    }
    if (command.dispatchGap800428B0 !=
        PrStageSceneSubmitDirect::PsxGpuPacketDispatchGap428B0::None) {
        ++s_stage1SceneSubmit428B0Debug.gapCount;
    }
    s_stage1SceneSubmit428B0Debug.lastKnown = true;
    s_stage1SceneSubmit428B0Debug.last = command;

    if (Stage1GpuPacketIsSpecialTableGap428B0(command)) {
        ++s_stage1SceneSubmit428B0Debug.specialTableCount;
        s_stage1SceneSubmit428B0Debug.lastSpecialKnown = true;
        s_stage1SceneSubmit428B0Debug.lastSpecial = command;
    }
}

static void AppendStage1SceneSubmit428B0CommandDebug(
    std::ostringstream& out,
    const char* label,
    const PrStageSceneSubmitDirect::PsxGpuPacketCommand428B0& command) {
    out << label
        << " valid=" << (command.valid ? 1 : 0)
        << " handler=" << Stage1GpuPacketHandlerName428B0(command.handler)
        << " dispatchKnown=" << (command.dispatchKnown800428B0 ? 1 : 0)
        << " gap="
        << Stage1GpuPacketDispatchGapName428B0(command.dispatchGap800428B0)
        << " primitiveCode=" << Hex2(command.primitiveCode)
        << " primitiveFlag=" << Hex2(command.primitiveFlag)
        << " flag4=" << (command.primitiveFlag4Path ? 1 : 0)
        << " rawKnown=" << (command.rawPacketKnown800428B0 ? 1 : 0)
        << " rawIndex=" << command.rawPrimitiveIndex800428B0
        << " rawOffset=" << command.rawPacketOffset800428B0
        << " rawSize=" << command.rawPacketByteSize800428B0
        << " handlerSlot="
        << static_cast<uint32_t>(command.handlerTableSlot8001C1E8)
        << " handlerEntry="
        << Hex8(command.handlerTableEntryAddress8001C1E8)
        << " specialPath=" << (command.specialTablePath800428B0 ? 1 : 0)
        << " specialBase=" << Hex8(command.specialTableBase800428B0)
        << " specialWriterKnown="
        << (command.specialTableWriterKnown800428B0 ? 1 : 0)
        << " handlerTargetKnown="
        << (command.handlerTargetKnown800428B0 ? 1 : 0)
        << " handlerTarget=" << Hex8(command.handlerTarget800428B0)
        << " cursorAdvanceHalfwords="
        << command.primitiveCursorAdvanceHalfwords800428B0
        << "\n";
}

static bool Stage1GpuPacketIsQuad428B0(
    PrStageSceneSubmitDirect::PsxGpuPacketHandler428B0 handler) {
    using Handler = PrStageSceneSubmitDirect::PsxGpuPacketHandler428B0;
    return handler == Handler::F4NL || handler == Handler::NF4 ||
           handler == Handler::TF4NL || handler == Handler::TNF4 ||
           handler == Handler::G4NL || handler == Handler::NG4 ||
           handler == Handler::TG4NL || handler == Handler::TNG4;
}

static bool Stage1GpuPacketIsTextured428B0(
    PrStageSceneSubmitDirect::PsxGpuPacketHandler428B0 handler) {
    using Handler = PrStageSceneSubmitDirect::PsxGpuPacketHandler428B0;
    return !(handler == Handler::F3NL || handler == Handler::NF3 ||
             handler == Handler::F4NL || handler == Handler::NF4 ||
             handler == Handler::G3NL || handler == Handler::NG3 ||
             handler == Handler::G4NL || handler == Handler::NG4);
}

static uint8_t Stage1GpuPacketSxyWordIndex428B0(
    PrStageSceneSubmitDirect::PsxGpuPacketHandler428B0 handler,
    uint8_t vertexSlot) {
    using Handler = PrStageSceneSubmitDirect::PsxGpuPacketHandler428B0;
    if (handler == Handler::F3NL || handler == Handler::NF3 ||
        handler == Handler::F4NL || handler == Handler::NF4) {
        return static_cast<uint8_t>(2u + vertexSlot);
    }
    if (handler == Handler::G3NL || handler == Handler::NG3 ||
        handler == Handler::G4NL || handler == Handler::NG4) {
        return static_cast<uint8_t>(2u + vertexSlot * 2u);
    }
    if (handler == Handler::TNG3 || handler == Handler::TNG4) {
        return static_cast<uint8_t>(2u + vertexSlot * 3u);
    }
    return static_cast<uint8_t>(2u + vertexSlot * 2u);
}

static uint8_t Stage1GpuPacketUvWordIndex428B0(
    PrStageSceneSubmitDirect::PsxGpuPacketHandler428B0 handler,
    uint8_t vertexSlot) {
    using Handler = PrStageSceneSubmitDirect::PsxGpuPacketHandler428B0;
    if (handler == Handler::F3NL || handler == Handler::NF3 ||
        handler == Handler::F4NL || handler == Handler::NF4 ||
        handler == Handler::G3NL || handler == Handler::NG3 ||
        handler == Handler::G4NL || handler == Handler::NG4) {
        return 0u;
    }
    if (handler == Handler::TNG3 || handler == Handler::TNG4) {
        return static_cast<uint8_t>(3u + vertexSlot * 3u);
    }
    return static_cast<uint8_t>(3u + vertexSlot * 2u);
}

static uint8_t Stage1GpuPacketColorWordIndex428B0(
    PrStageSceneSubmitDirect::PsxGpuPacketHandler428B0 handler,
    uint8_t vertexSlot) {
    using Handler = PrStageSceneSubmitDirect::PsxGpuPacketHandler428B0;
    if (handler == Handler::G3NL || handler == Handler::NG3 ||
        handler == Handler::G4NL || handler == Handler::NG4) {
        return static_cast<uint8_t>(1u + vertexSlot * 2u);
    }
    if (handler == Handler::TNG3 || handler == Handler::TNG4) {
        return static_cast<uint8_t>(1u + vertexSlot * 3u);
    }
    return 1u;
}

static int16_t Stage1GpuPacketUnpackSxyX428B0(uint32_t word) {
    return static_cast<int16_t>(word & 0xFFFFu);
}

static int16_t Stage1GpuPacketUnpackSxyY428B0(uint32_t word) {
    return static_cast<int16_t>((word >> 16) & 0xFFFFu);
}

static uint8_t Stage1GpuPacketUnpackUvU428B0(uint32_t word) {
    return static_cast<uint8_t>(word & 0xFFu);
}

static uint8_t Stage1GpuPacketUnpackUvV428B0(uint32_t word) {
    return static_cast<uint8_t>((word >> 8) & 0xFFu);
}

static uint32_t Stage1TmdVertexPositionCacheKey428B0(int16_t x, int16_t y) {
    return (static_cast<uint32_t>(static_cast<uint16_t>(x)) << 16) |
           static_cast<uint16_t>(y);
}

static void ResolveStage1TmdVertexPosition428B0(
    std::unordered_map<uint32_t, std::array<float, 2>>& cache,
    int16_t packetX,
    int16_t packetY,
    float candidateX,
    float candidateY,
    float& outX,
    float& outY) {
    const uint32_t key = Stage1TmdVertexPositionCacheKey428B0(packetX,
                                                              packetY);
    const auto found = cache.find(key);
    if (found != cache.end()) {
        outX = found->second[0];
        outY = found->second[1];
        return;
    }

    cache.emplace(key, std::array<float, 2>{candidateX, candidateY});
    outX = candidateX;
    outY = candidateY;
}

static float Stage1GpuPacketColorByteToFloat428B0(uint32_t word,
                                                  uint32_t shift,
                                                  bool textured) {
    const float value = static_cast<float>((word >> shift) & 0xFFu);
    return value / (textured ? 128.0f : 255.0f);
}

static bool Stage1GpuPacketSemiTransparencyApplies428B0(
    bool textured,
    uint16_t clut) {
    if (!textured) {
        return true;
    }

    bool clutHasStpBits = false;
    const bool clutStpKnown =
        s_stage1VramAtlas.TryGetClutHasStpBits(clut, clutHasStpBits);
    return !clutStpKnown || clutHasStpBits;
}

static bool ResolveStage1FaceTimPayload(
    PrGameContext& ctx,
    uint16_t timId,
    std::string& outAtlasKey,
    const uint8_t*& outBytes,
    std::size_t& outByteSize) {
    outAtlasKey.clear();
    outBytes = nullptr;
    outByteSize = 0u;
    if (timId == 0u ||
        ctx.resources == nullptr ||
        !ctx.stage1OverlayData ||
        !ctx.stage1OverlayData->valid) {
        return false;
    }

    const std::string* memName =
        ctx.stage1OverlayData->FindMemHandleName(timId);
    if (memName == nullptr || memName->empty()) {
        return false;
    }

    const std::string lowerKey = Stage1SceneStripAsciiExtension(
        Stage1SceneToLowerAsciiCopy(*memName));
    if (!Stage1SceneStartsWithAscii(lowerKey, "f_")) {
        return false;
    }

    const std::vector<uint8_t>* raw = ctx.resources->GetMem(*memName);
    if ((!raw || raw->empty()) && !lowerKey.empty()) {
        raw = ctx.resources->GetTimRaw(lowerKey);
    }
    if (raw == nullptr || raw->empty()) {
        return false;
    }

    outAtlasKey = lowerKey;
    outBytes = raw->data();
    outByteSize = raw->size();
    return true;
}

static bool UploadStage1FaceTimPayload(const std::string& atlasKey,
                                       const uint8_t* bytes,
                                       std::size_t byteSize,
                                       bool uploadClut,
                                       bool& atlasDirty) {
    if (atlasKey.empty() || bytes == nullptr || byteSize == 0u) {
        return false;
    }

    if (!s_stage1VramAtlas.LoadTim(
            bytes, byteSize, atlasKey, uploadClut, false)) {
        return false;
    }

    atlasDirty = true;
    return true;
}

struct Stage1Viewport {
    float x = 0.0f;
    float y = 0.0f;
    float scale = 1.0f;
};

constexpr float kStage1PsxScreenWidth = 320.0f;
constexpr float kStage1PsxScreenHeight = 240.0f;

static Stage1Viewport CalcPs1Viewport(const D3D11Renderer* renderer) {
    Stage1Viewport viewport{};
    if (renderer == nullptr) {
        return viewport;
    }

    const float winW = static_cast<float>(renderer->GetWidth());
    const float winH = static_cast<float>(renderer->GetHeight());
    const float fitX = winW / kStage1PsxScreenWidth;
    const float fitY = winH / kStage1PsxScreenHeight;
    viewport.scale = (std::min)(fitX, fitY);
    viewport.x = (winW - kStage1PsxScreenWidth * viewport.scale) * 0.5f;
    viewport.y = (winH - kStage1PsxScreenHeight * viewport.scale) * 0.5f;
    return viewport;
}

static float SnapStage1ScreenCoord(float value) {
    return value;
}

static float SnapStage1ScreenLength(float value) {
    if (value <= 0.0f) {
        return 0.0f;
    }
    return (std::max)(1.0f, value);
}

static float Stage1ScreenX(const Stage1Viewport& viewport, float x) {
    return SnapStage1ScreenCoord(viewport.x + x * viewport.scale);
}

static float Stage1ScreenY(const Stage1Viewport& viewport, float y) {
    return SnapStage1ScreenCoord(viewport.y + y * viewport.scale);
}

static float Stage1ScreenLength(const Stage1Viewport& viewport, float length) {
    return SnapStage1ScreenLength(length * viewport.scale);
}

static float Stage1TmdScaleAwareCoverageBiasPsx(
    const Stage1Viewport& viewport) {
    if (viewport.scale <= 1.0f) {
        return 0.0f;
    }

    return (std::min)(0.5f, 0.5f * (1.0f - 1.0f / viewport.scale));
}

static void ApplyStage1TmdScaleAwareCoverageBias(
    const Stage1Viewport& viewport,
    std::array<float, 3>& x,
    std::array<float, 3>& y) {
    const float biasPsx = Stage1TmdScaleAwareCoverageBiasPsx(viewport);
    if (biasPsx <= 0.0f) {
        return;
    }

    const float cx = (x[0] + x[1] + x[2]) / 3.0f;
    const float cy = (y[0] + y[1] + y[2]) / 3.0f;
    for (size_t i = 0; i < 3u; ++i) {
        const float dx = x[i] - cx;
        const float dy = y[i] - cy;
        const float len = std::sqrt(dx * dx + dy * dy);
        if (len <= 0.0001f) {
            continue;
        }
        const float scale = (len + biasPsx) / len;
        x[i] = cx + dx * scale;
        y[i] = cy + dy * scale;
    }
}

static float Stage1TriangleAreaPsx(
    const std::array<float, 3>& x,
    const std::array<float, 3>& y) {
    return std::abs((x[1] - x[0]) * (y[2] - y[0]) -
                    (y[1] - y[0]) * (x[2] - x[0])) *
           0.5f;
}

static void Stage1TriangleBoundsPsx(
    const std::array<float, 3>& x,
    const std::array<float, 3>& y,
    float& outW,
    float& outH) {
    const auto [minX, maxX] = std::minmax({x[0], x[1], x[2]});
    const auto [minY, maxY] = std::minmax({y[0], y[1], y[2]});
    outW = maxX - minX;
    outH = maxY - minY;
}

static bool Stage1HdCleanupAllowsSceneArchitectureModel(
    const PrStageSceneSubmitDirect::PsxGpuPacketCommand428B0& packetCommand) {
    if (!packetCommand.sourceModelRefKnown800428B0 ||
        packetCommand.sourceModelSet800428B0 !=
            static_cast<uint8_t>(
                PrStageSceneSubmitDirect::PsxDrawModelRef::Set::
                    Stage1SceneMap)) {
        return false;
    }

    // TENNJOU is a large ceiling surface; at opening camera angles it can
    // briefly project as thin triangles, so aggressive cleanup creates holes.
    if (packetCommand.sourceModelIndex800428B0 == 8u) {
        return false;
    }
    return true;
}

static bool ShouldCullStage1HdSceneNeedleTriangle(
    const PrGameContext& ctx,
    const PrStageSceneSubmitDirect::PsxGpuPacketCommand428B0& packetCommand,
    const std::array<float, 3>& packetX,
    const std::array<float, 3>& packetY,
    const std::array<float, 3>& drawX,
    const std::array<float, 3>& drawY) {
    if (!ctx.stage1HdGeometryCleanup ||
        !Stage1HdCleanupAllowsSceneArchitectureModel(packetCommand)) {
        return false;
    }

    float packetW = 0.0f;
    float packetH = 0.0f;
    Stage1TriangleBoundsPsx(packetX, packetY, packetW, packetH);
    const float packetMinDim = (std::min)(packetW, packetH);
    const float packetMaxDim = (std::max)(packetW, packetH);
    if (packetMaxDim < 8.0f) {
        return false;
    }

    const float packetArea = Stage1TriangleAreaPsx(packetX, packetY);
    if (packetMinDim <= 1.0f && packetArea <= 64.0f) {
        return true;
    }
    if (packetMinDim <= 2.0f && packetArea <= 32.0f) {
        return true;
    }

    const float drawArea = Stage1TriangleAreaPsx(drawX, drawY);
    if (drawArea <= 0.125f) {
        return true;
    }
    return false;
}

static bool IsStage1FullWidthPsxRect(float x, float w) {
    constexpr float kEpsilon = 0.01f;
    return std::abs(x) <= kEpsilon &&
           std::abs(w - kStage1PsxScreenWidth) <= kEpsilon;
}

static void Stage1FullscreenBackgroundXRange(
    const D3D11Renderer* renderer,
    const Stage1Viewport& viewport,
    float psxX,
    float psxW,
    float& outX,
    float& outW) {
    outX = Stage1ScreenX(viewport, psxX);
    outW = Stage1ScreenLength(viewport, psxW);
    if (renderer == nullptr || !IsStage1FullWidthPsxRect(psxX, psxW)) {
        return;
    }

    const float winW = static_cast<float>(renderer->GetWidth());
    const float right = (std::max)(winW, outX + outW);
    outX = (std::min)(0.0f, outX);
    outW = SnapStage1ScreenLength(right - outX);
}

static void Stage1SpriteUvCenterEndpoints(uint8_t u,
                                          uint8_t v,
                                          uint16_t w,
                                          uint16_t h,
                                          float& u0,
                                          float& v0,
                                          float& u1,
                                          float& v1) {
    const float fu = static_cast<float>(u);
    const float fv = static_cast<float>(v);
    const float fw = static_cast<float>(w);
    const float fh = static_cast<float>(h);
    if (fw <= 1.0f) {
        u0 = u1 = (fu + 0.5f) / 256.0f;
    } else {
        u0 = (fu + 0.5f) / 256.0f;
        u1 = (fu + fw - 0.5f) / 256.0f;
    }
    if (fh <= 1.0f) {
        v0 = v1 = (fv + 0.5f) / 256.0f;
    } else {
        v0 = (fv + 0.5f) / 256.0f;
        v1 = (fv + fh - 0.5f) / 256.0f;
    }
}

static uint64_t Stage1TextureReplacementSourceKey(uint16_t tpage,
                                                  uint16_t clut,
                                                  uint8_t u,
                                                  uint8_t v,
                                                  uint16_t w,
                                                  uint16_t h,
                                                  uint64_t salt) {
    uint64_t key = 1469598103934665603ULL ^ salt;
    const auto mix = [&key](uint64_t value) {
        key ^= value;
        key *= 1099511628211ULL;
    };
    mix(tpage);
    mix(clut);
    mix(u);
    mix(v);
    mix(w);
    mix(h);
    return key;
}

static void ApplyStage1ReplacementTint(
    const PrStage1TextureReplacements::ReplacementTexture& replacement,
    D3D11Renderer::SpriteCmd& cmd) {
    if (!replacement.hasTint) {
        return;
    }
    cmd.r *= replacement.tintR;
    cmd.g *= replacement.tintG;
    cmd.b *= replacement.tintB;
}

static void ApplyStage1ReplacementTint(
    const PrStage1TextureReplacements::ReplacementTexture& replacement,
    D3D11Renderer::TexturedTriCmd& cmd) {
    if (!replacement.hasTint) {
        return;
    }
    for (int i = 0; i < cmd.vertexCount && i < 6; ++i) {
        cmd.vertices[i].r *= replacement.tintR;
        cmd.vertices[i].g *= replacement.tintG;
        cmd.vertices[i].b *= replacement.tintB;
    }
}

static bool ApplyStage1SpriteTextureReplacement(
    PrGameContext& ctx,
    const char* kind,
    uint16_t tpage,
    uint16_t clut,
    uint8_t u,
    uint8_t v,
    uint16_t w,
    uint16_t h,
    uint64_t salt,
    D3D11Renderer::SpriteCmd& cmd) {
    if (!ctx.stage1TextureReplacements || w == 0u || h == 0u) {
        return false;
    }

    const uint64_t key =
        Stage1TextureReplacementSourceKey(tpage, clut, u, v, w, h, salt);
    PrStage1TextureReplacements::ReplacementTexture replacement{};
    const bool hit = PrStage1TextureReplacements::TryFindReplacementBySourceKey(
        ctx,
        key,
        replacement);
    char detail[256];
    std::snprintf(detail,
                  sizeof(detail),
                  "tpage=0x%04X clut=0x%04X rect=%u,%u,%u,%u salt=0x%016llX",
                  tpage,
                  clut,
                  u,
                  v,
                  w,
                  h,
                  static_cast<unsigned long long>(salt));
    PrStage1TextureReplacements::DebugLogSourceKeyLookup(
        ctx,
        key,
        kind,
        detail,
        hit);
    if (!hit) {
        return false;
    }

    cmd.texture = replacement.srv;
    cmd.u0 = 0.0f;
    cmd.v0 = 0.0f;
    cmd.u1 = 1.0f;
    cmd.v1 = 1.0f;
    ApplyStage1ReplacementTint(replacement, cmd);
    return true;
}

static bool ApplyStage1TexturedTriTextureReplacement(
    PrGameContext& ctx,
    const char* kind,
    uint16_t tpage,
    uint16_t clut,
    const uint8_t* vertexU,
    const uint8_t* vertexV,
    int vertexCount,
    uint64_t salt,
    D3D11Renderer::TexturedTriCmd& cmd) {
    if (!ctx.stage1TextureReplacements ||
        vertexU == nullptr ||
        vertexV == nullptr ||
        vertexCount <= 0 ||
        vertexCount > 6) {
        return false;
    }

    int minU = 255;
    int minV = 255;
    int maxU = 0;
    int maxV = 0;
    for (int i = 0; i < vertexCount; ++i) {
        minU = (std::min)(minU, static_cast<int>(vertexU[i]));
        minV = (std::min)(minV, static_cast<int>(vertexV[i]));
        maxU = (std::max)(maxU, static_cast<int>(vertexU[i]));
        maxV = (std::max)(maxV, static_cast<int>(vertexV[i]));
    }

    const int w = maxU - minU + 1;
    const int h = maxV - minV + 1;
    if (w <= 0 || h <= 0) {
        return false;
    }

    const uint64_t key = Stage1TextureReplacementSourceKey(
        tpage,
        clut,
        static_cast<uint8_t>(minU),
        static_cast<uint8_t>(minV),
        static_cast<uint16_t>(w),
        static_cast<uint16_t>(h),
        salt);
    PrStage1TextureReplacements::ReplacementTexture replacement{};
    const bool hit = PrStage1TextureReplacements::TryFindReplacementBySourceKey(
        ctx,
        key,
        replacement);
    char detail[320];
    std::snprintf(detail,
                  sizeof(detail),
                  "tpage=0x%04X clut=0x%04X rect=%d,%d,%d,%d verts=%d salt=0x%016llX",
                  tpage,
                  clut,
                  minU,
                  minV,
                  w,
                  h,
                  vertexCount,
                  static_cast<unsigned long long>(salt));
    PrStage1TextureReplacements::DebugLogSourceKeyLookup(
        ctx,
        key,
        kind,
        detail,
        hit);
    if (!hit) {
        int sourceX = 0;
        int sourceY = 0;
        int sourceW = 0;
        int sourceH = 0;
        PrStage1TextureReplacements::ReplacementTexture atlasReplacement{};
        const bool atlasHit =
            PrStage1TextureReplacements::TryFindReplacementByAtlasRect(
                ctx,
                tpage,
                clut,
                minU,
                minV,
                maxU,
                maxV,
                atlasReplacement,
                sourceX,
                sourceY,
                sourceW,
                sourceH);
        if (!atlasHit || sourceW <= 0 || sourceH <= 0) {
            return false;
        }

        char atlasDetail[384];
        std::snprintf(atlasDetail,
                      sizeof(atlasDetail),
                      "tpage=0x%04X clut=0x%04X rect=%d,%d,%d,%d sourceRect=%d,%d,%d,%d verts=%d salt=0x%016llX",
                      tpage,
                      clut,
                      minU,
                      minV,
                      w,
                      h,
                      sourceX,
                      sourceY,
                      sourceW,
                      sourceH,
                      vertexCount,
                      static_cast<unsigned long long>(salt));
        PrStage1TextureReplacements::DebugLogSourceKeyLookup(
            ctx,
            key,
            "tmd_texdump_rect",
            atlasDetail,
            true);

        cmd.texture = atlasReplacement.srv;
        ApplyStage1ReplacementTint(atlasReplacement, cmd);
        for (int i = 0; i < vertexCount; ++i) {
            const float remappedU =
                (static_cast<float>(static_cast<int>(vertexU[i]) - sourceX) + 0.5f) /
                static_cast<float>(sourceW);
            const float remappedV =
                (static_cast<float>(static_cast<int>(vertexV[i]) - sourceY) + 0.5f) /
                static_cast<float>(sourceH);
            cmd.vertices[i].u = (std::clamp)(remappedU, 0.0f, 1.0f);
            cmd.vertices[i].v = (std::clamp)(remappedV, 0.0f, 1.0f);
        }
        return true;
    }

    cmd.texture = replacement.srv;
    ApplyStage1ReplacementTint(replacement, cmd);
    for (int i = 0; i < vertexCount; ++i) {
        cmd.vertices[i].u =
            (static_cast<float>(vertexU[i] - minU) + 0.5f) /
            static_cast<float>(w);
        cmd.vertices[i].v =
            (static_cast<float>(vertexV[i] - minV) + 0.5f) /
            static_cast<float>(h);
    }
    return true;
}

struct Stage1TmdDrawContext428B0 {
    PrGameContext* ctx = nullptr;
    Stage1Viewport viewport{};
    std::unordered_map<uint32_t, std::array<float, 2>> vertexPositionCache;
};

struct Stage1GpuPacketDrawEntry428B0 {
    PrStageSceneSubmitDirect::PsxGpuPacketCommand428B0 command{};
    std::size_t serial = 0;
};

struct Stage1GpuPacketCollectContext428B0 {
    std::vector<Stage1GpuPacketDrawEntry428B0> entries;
};

struct Stage1GpuPacketBlendStats428B0 {
    uint32_t matchedCount = 0;
    uint32_t appliedCount = 0;
    bool firstDeltaKnown = false;
    float firstDeltaX = 0.0f;
    float firstDeltaY = 0.0f;
};

struct Stage1GpuPacketBlendKey428B0 {
    uint8_t sourceModelSet = 0;
    uint16_t sourceModelIndex = 0;
    uint8_t workListFlushOrder = 0;
    uint32_t rawPrimitiveIndex = 0;
    uint32_t rawPacketOffset = 0;
    uint32_t rawPacketByteSize = 0;
    uint8_t handler = 0;
    uint8_t primitiveCode = 0;

    bool operator==(const Stage1GpuPacketBlendKey428B0& other) const {
        return sourceModelSet == other.sourceModelSet &&
               sourceModelIndex == other.sourceModelIndex &&
               workListFlushOrder == other.workListFlushOrder &&
               rawPrimitiveIndex == other.rawPrimitiveIndex &&
               rawPacketOffset == other.rawPacketOffset &&
               rawPacketByteSize == other.rawPacketByteSize &&
               handler == other.handler &&
               primitiveCode == other.primitiveCode;
    }
};

struct Stage1GpuPacketBlendKeyHash428B0 {
    std::size_t operator()(const Stage1GpuPacketBlendKey428B0& key) const {
        uint64_t hash = 1469598103934665603ULL;
        const auto mix = [&hash](uint64_t value) {
            hash ^= value;
            hash *= 1099511628211ULL;
        };
        mix(key.sourceModelSet);
        mix(key.sourceModelIndex);
        mix(key.workListFlushOrder);
        mix(key.rawPrimitiveIndex);
        mix(key.rawPacketOffset);
        mix(key.rawPacketByteSize);
        mix(key.handler);
        mix(key.primitiveCode);
        return static_cast<std::size_t>(hash);
    }
};

struct Stage1GpuPacketBlendKeyUse428B0 {
    std::size_t index = 0;
    uint32_t count = 0;
};

struct Stage1BackdropDrawContext801CAC34 {
    PrGameContext* ctx = nullptr;
    Stage1Viewport viewport{};
};

} // namespace

static const TmdModel* ResolveStage1DrawModelByDescAddr801CB190(
    uint32_t descAddr,
    const PrStageSceneSubmitDirect::PsxDrawModelRef& modelRef);
static void UploadStage1VramAtlas(PrGameContext& ctx);
static void EnsureStage1VramAtlas(PrGameContext& ctx);
static void DrawStage1TriangleCommand(
    PrGameContext& ctx,
    const PrStageSceneSubmitDirect::PsxDrawTriangleCommand428B0& command,
    const Stage1Viewport& viewport,
    int layer,
    uint64_t order,
    bool coverageBiasAllowed);
static void DrawStage1GpuPacketCommand428B0(
    PrGameContext& ctx,
    const PrStageSceneSubmitDirect::PsxGpuPacketCommand428B0& command,
    const Stage1Viewport& viewport,
    std::unordered_map<uint32_t, std::array<float, 2>>& vertexPositionCache);
static void DrawStage1BackdropCommandBatch(
    PrGameContext& ctx,
    const PrStageSceneSubmitDirect::PsxBackdropGradientCommand801CAC34& gradient,
    const PrStageSceneSubmitDirect::PsxBackdropSpriteCommand801CAC34* sprites,
    std::size_t spriteCount,
    const Stage1Viewport& viewport);
static void DrawStage1DrawEnvClearColor80040060(
    PrGameContext& ctx,
    const PrStageSceneSubmitDirect::Stage1SceneSubmitRuntime& runtime,
    const Stage1Viewport& viewport);
static void ResetStage1FaceTimUploads801CBFDC();

static void DestroyStage1Parappa2RailGlowTexture() {
    if (s_stage1Parappa2RailGlowTexture != nullptr &&
        s_stage1Parappa2RailGlowRenderer != nullptr) {
        s_stage1Parappa2RailGlowRenderer->DestroyTexture(
            s_stage1Parappa2RailGlowTexture);
    }
    s_stage1Parappa2RailGlowTexture = nullptr;
    s_stage1Parappa2RailGlowRenderer = nullptr;
}

void LoadStage1Resources(ResourceManager* resources) {
    if (resources == nullptr) {
        return;
    }

    ResetModelSet(s_stage1Models, s_stage1Loaded);
    ResetModelSet(s_stage1SceneMapModels, s_stage1SceneMapLoaded);

    const bool anyStage1ModelLoaded = LoadTmdSet(resources,
                                                kStage1TmdNames,
                                                "Stage1 TMD",
                                                s_stage1Models,
                                                s_stage1Loaded);
    const bool anyStage1SceneMapLoaded = LoadTmdSet(resources,
                                                   kStage1SceneMapTmdNames,
                                                   "Stage1 SceneMap TMD",
                                                   s_stage1SceneMapModels,
                                                   s_stage1SceneMapLoaded);
    if (anyStage1ModelLoaded) {
        Log::Printf("Stage1 TMD resources: models=%d sceneMap=%d floor=%d shadow=%d mascot=%d onion=%d parappa=%d",
                    anyStage1ModelLoaded ? 1 : 0,
                    anyStage1SceneMapLoaded ? 1 : 0,
                    s_stage1Loaded[0] ? 1 : 0,
                    s_stage1Loaded[1] ? 1 : 0,
                    s_stage1Loaded[2] ? 1 : 0,
                    s_stage1Loaded[kStage1OnIndex] ? 1 : 0,
                    s_stage1Loaded[kStage1PaIndex] ? 1 : 0);
    }

    s_stage1VramAtlas.Clear();
    s_stage1AtlasReady = false;
    PrStage1TextureReplacements::Reset();
    ResetStage1FaceTimUploads801CBFDC();
    PrStageSceneSubmitDirect::ResetOwnedStage1SceneSubmitRuntime801CBFDC190();
    s_stage1RenderOnlyRuntime801CBFDC190 =
        PrStageSceneSubmitDirect::Stage1SceneSubmitRuntime{};
    s_stage1UseRenderOnlyRuntime801CBFDC190 = false;
    RegisterModelSetTpages(s_stage1Models, s_stage1Loaded);
    RegisterModelSetTpages(s_stage1SceneMapModels, s_stage1SceneMapLoaded);
    RegisterStage1TextGlyphTpages8001B954();
    RegisterStage1ScriptBoxCornerTpages8001C550();
    RegisterStage1BackdropTextureTpages801CAC34(resources);

    const std::vector<std::string> timNames = resources->GetTimRawNames();
    int timLoaded = 0;
    int timSkipped = 0;
    for (const std::string& tn : timNames) {
        if (tn.size() >= 2 && tn[0] == 'f' && tn[1] == '_') {
            ++timSkipped;
            continue;
        }
        const std::vector<uint8_t>* raw = resources->GetTimRaw(tn);
        if (!raw || raw->empty()) {
            continue;
        }
        if (s_stage1VramAtlas.LoadTim(raw->data(), raw->size(), tn)) {
            ++timLoaded;
        }
    }
    Log::Printf("Stage1 VramAtlas: loaded %d TIM textures into %d tpages (scanned %d, skipped %d face TIMs)",
                timLoaded,
                s_stage1VramAtlas.GetTpageCount(),
                static_cast<int>(timNames.size()),
                timSkipped);
}

void ClearStage1Resources() {
    ResetModelSet(s_stage1Models, s_stage1Loaded);
    ResetModelSet(s_stage1SceneMapModels, s_stage1SceneMapLoaded);
    s_stage1AtlasReady = false;
    s_stage1VramAtlas.Clear();
    PrStage1TextureReplacements::Reset();
    ResetStage1FaceTimUploads801CBFDC();
    DestroyStage1Parappa2RailGlowTexture();
    PrStageSceneSubmitDirect::ResetOwnedStage1SceneSubmitRuntime801CBFDC190();
    s_stage1RenderOnlyRuntime801CBFDC190 =
        PrStageSceneSubmitDirect::Stage1SceneSubmitRuntime{};
    s_stage1UseRenderOnlyRuntime801CBFDC190 = false;
}

static std::string DumpStage1StatsForDebug() {
    std::string out;
    out += "\nStage1 Gameplay Models:\n";
    for (int i = 0; i < kStage1TmdCount; ++i) {
        out += "  [" + std::to_string(i) + "] " + kStage1TmdNames[i] + ": ";
        if (!s_stage1Loaded[i]) {
            out += "not loaded\n";
            continue;
        }
        int tv = 0;
        int tp = 0;
        int16_t minX = 32767;
        int16_t maxX = -32768;
        int16_t minY = 32767;
        int16_t maxY = -32768;
        int16_t minZ = 32767;
        int16_t maxZ = -32768;
        for (const auto& obj : s_stage1Models[i].objects) {
            tv += static_cast<int>(obj.vertices.size());
            tp += static_cast<int>(obj.primitives.size());
            for (const auto& v : obj.vertices) {
                if (v.x < minX) minX = v.x;
                if (v.x > maxX) maxX = v.x;
                if (v.y < minY) minY = v.y;
                if (v.y > maxY) maxY = v.y;
                if (v.z < minZ) minZ = v.z;
                if (v.z > maxZ) maxZ = v.z;
            }
        }
        out += "objs=" + std::to_string(s_stage1Models[i].objects.size()) +
               " v=" + std::to_string(tv) +
               " p=" + std::to_string(tp) +
               " bbox=[" + std::to_string(minX) + ".." +
               std::to_string(maxX) + "," + std::to_string(minY) + ".." +
               std::to_string(maxY) + "," + std::to_string(minZ) + ".." +
               std::to_string(maxZ) + "]\n";
    }
    out += "Stage1 SceneMap Models:\n";
    for (int i = 0; i < kStage1SceneMapTmdCount; ++i) {
        out += "  [" + std::to_string(i) + "] " +
               kStage1SceneMapTmdNames[i] + ": ";
        if (!s_stage1SceneMapLoaded[i]) {
            out += "not loaded\n";
            continue;
        }
        int tv = 0;
        int tp = 0;
        int16_t minX = 32767;
        int16_t maxX = -32768;
        int16_t minY = 32767;
        int16_t maxY = -32768;
        int16_t minZ = 32767;
        int16_t maxZ = -32768;
        for (const auto& obj : s_stage1SceneMapModels[i].objects) {
            tv += static_cast<int>(obj.vertices.size());
            tp += static_cast<int>(obj.primitives.size());
            for (const auto& v : obj.vertices) {
                if (v.x < minX) minX = v.x;
                if (v.x > maxX) maxX = v.x;
                if (v.y < minY) minY = v.y;
                if (v.y > maxY) maxY = v.y;
                if (v.z < minZ) minZ = v.z;
                if (v.z > maxZ) maxZ = v.z;
            }
        }
        out += "objs=" + std::to_string(s_stage1SceneMapModels[i].objects.size()) +
               " v=" + std::to_string(tv) +
               " p=" + std::to_string(tp) +
               " bbox=[" + std::to_string(minX) + ".." +
               std::to_string(maxX) + "," + std::to_string(minY) + ".." +
               std::to_string(maxY) + "," + std::to_string(minZ) + ".." +
               std::to_string(maxZ) + "]\n";
    }
    out += "Stage1 VramAtlas: ready=" +
           std::string(s_stage1AtlasReady ? "Y" : "N") +
           " tpages=" + std::to_string(s_stage1VramAtlas.GetTpageCount()) +
           "\n";
    return out;
}

}  // namespace PrStageSceneSubmitBackend

namespace PrStageSceneSubmitDebug {

std::string DumpStage1Stats() {
    return PrStageSceneSubmitBackend::DumpStage1StatsForDebug();
}

}  // namespace PrStageSceneSubmitDebug

namespace PrStageSceneSubmitBackend {

std::string DescribeStage1SceneSubmit428B0Debug() {
    const Stage1SceneSubmit428B0DebugState& state =
        s_stage1SceneSubmit428B0Debug;
    std::ostringstream out;
    out << "sceneSubmit428B0"
        << " drawCalled=" << (state.drawCalled ? 1 : 0)
        << " renderer=" << (state.rendererAvailable ? 1 : 0)
        << " commands=" << state.commandCount
        << " valid=" << state.validCount
        << " gaps=" << state.gapCount
        << " specialTable=" << state.specialTableCount
        << " renderBlend=" << (state.renderOnlyBlendAttempted ? 1 : 0)
        << " blendPaused=" << (state.renderOnlyBlendPaused ? 1 : 0)
        << " blendPauseFrames=" << state.renderOnlyBlendPauseFrames
        << " blendCurrent=" << state.blendCurrentCount
        << " blendNext=" << state.blendNextCount
        << " blendMatched=" << state.blendMatchedCount
        << " blendApplied=" << state.blendAppliedCount
        << " blendFirstDeltaKnown="
        << (state.blendFirstDeltaKnown ? 1 : 0)
        << " blendFirstDx=" << state.blendFirstDeltaX
        << " blendFirstDy=" << state.blendFirstDeltaY
        << "\n";
    if (state.lastKnown) {
        AppendStage1SceneSubmit428B0CommandDebug(out, "last", state.last);
    } else {
        out << "last <none>\n";
    }
    if (state.lastSpecialKnown) {
        AppendStage1SceneSubmit428B0CommandDebug(
            out,
            "lastSpecial",
            state.lastSpecial);
    } else {
        out << "lastSpecial <none>\n";
    }
    return out.str();
}

static bool UploadStage1FaceTimCommand801CBFDC(PrGameContext& ctx,
                                               uint16_t timId,
                                               bool uploadClut,
                                               bool& atlasDirty) {
    std::string atlasKey;
    const uint8_t* bytes = nullptr;
    std::size_t byteSize = 0u;
    if (!ResolveStage1FaceTimPayload(ctx, timId, atlasKey, bytes, byteSize)) {
        return false;
    }

    return UploadStage1FaceTimPayload(atlasKey,
                                      bytes,
                                      byteSize,
                                      uploadClut,
                                      atlasDirty);
}

static void UploadStage1VramAtlas(PrGameContext& ctx) {
    if (ctx.renderer) {
        s_stage1VramAtlas.UploadAll(ctx.renderer);
        s_stage1AtlasReady = true;
    }
}

static bool ResolveStage1ResourceBytes(PrGameContext& ctx,
                                       uint16_t handle,
                                       const uint8_t*& outBytes,
                                       std::size_t& outByteSize) {
    outBytes = nullptr;
    outByteSize = 0u;
    if (ctx.resources == nullptr ||
        !ctx.stage1OverlayData ||
        !ctx.stage1OverlayData->valid) {
        return false;
    }

    const std::string* name =
        ctx.stage1OverlayData->FindMemHandleName(handle);
    if (name == nullptr || name->empty()) {
        return false;
    }

    const std::vector<uint8_t>* bytes = ctx.resources->GetMem(*name);
    if (bytes == nullptr || bytes->empty()) {
        return false;
    }

    outBytes = bytes->data();
    outByteSize = bytes->size();
    return true;
}

static std::string BuildStage1MimeHandleName801CB190(uint8_t handle,
                                                     const char* ext) {
    char buffer[40]{};
    std::snprintf(buffer,
                  sizeof(buffer),
                  "stage1_801cb190_%02x%s",
                  static_cast<unsigned>(handle),
                  ext);
    return std::string(buffer);
}

static const DatData* ResolveStage1MimeDat801CB190(PrGameContext& ctx,
                                                   uint8_t handle) {
    if (handle == 0u) {
        return nullptr;
    }
    const std::string name = BuildStage1MimeHandleName801CB190(handle, ".dat");
    if (const DatData* cached = MimeEngine::GetDatByName(name)) {
        return cached;
    }

    const uint8_t* bytes = nullptr;
    std::size_t byteSize = 0;
    if (!ResolveStage1ResourceBytes(ctx, handle, bytes, byteSize) ||
        bytes == nullptr ||
        byteSize == 0u ||
        MimeEngine::LoadDat(name, bytes, byteSize) < 0) {
        return nullptr;
    }
    return MimeEngine::GetDatByName(name);
}

static const VdfData* ResolveStage1MimeVdf801CB190(PrGameContext& ctx,
                                                   uint8_t handle) {
    if (handle == 0u) {
        return nullptr;
    }
    const std::string name = BuildStage1MimeHandleName801CB190(handle, ".vdf");
    if (const VdfData* cached = MimeEngine::GetVdfByName(name)) {
        return cached;
    }

    const uint8_t* bytes = nullptr;
    std::size_t byteSize = 0;
    if (!ResolveStage1ResourceBytes(ctx, handle, bytes, byteSize) ||
        bytes == nullptr ||
        byteSize == 0u ||
        MimeEngine::LoadVdf(name, bytes, byteSize) < 0) {
        return nullptr;
    }
    return MimeEngine::GetVdfByName(name);
}

static bool ResolveStage1MimeResources801CB190(
    uint8_t datHandle,
    uint8_t vdfHandle,
    const DatData*& outDat,
    const VdfData*& outVdf,
    void* userData) {
    outDat = nullptr;
    outVdf = nullptr;
    auto* ctx = static_cast<PrGameContext*>(userData);
    if (ctx == nullptr) {
        return false;
    }

    outDat = ResolveStage1MimeDat801CB190(*ctx, datHandle);
    outVdf = ResolveStage1MimeVdf801CB190(*ctx, vdfHandle);
    return outDat != nullptr && outVdf != nullptr;
}

static bool ResolveStage1SceneResourceBytes801CBFDC(
    uint16_t handle,
    const uint8_t*& outBytes,
    std::size_t& outByteSize,
    void* userData) {
    auto* ctx = static_cast<PrGameContext*>(userData);
    if (ctx == nullptr) {
        return false;
    }
    return ResolveStage1ResourceBytes(*ctx, handle, outBytes, outByteSize);
}

static void ResetStage1FaceTimUploads801CBFDC() {}

void ResetStage1SceneSubmitRuntimeForRender801CBFDC190() {
    PrStageSceneSubmitDirect::ResetOwnedStage1SceneSubmitRuntime801CBFDC190();
    s_stage1RenderOnlyRuntime801CBFDC190 =
        PrStageSceneSubmitDirect::Stage1SceneSubmitRuntime{};
    s_stage1UseRenderOnlyRuntime801CBFDC190 = false;
    s_stage1TmdRenderOnlyBlendPauseFrames801CBFDC190 = 0;
    s_stage1TmdBlendLastRightRankKnown801CBFDC190 = false;
    s_stage1TmdBlendLastRightRankRow801CBFDC190 = 0u;
    s_stage1TmdBlendLastRankTriggerMask801CBFDC190 = 0u;
    ResetStage1FaceTimUploads801CBFDC();
}

static void ConsumeStage1FaceTimUploadCommands801CBFDC(
    PrGameContext& ctx,
    const PrStageSceneSubmitDirect::Stage1SceneSubmitRuntime& runtime) {
    struct UploadBatch {
        PrGameContext* ctx = nullptr;
        bool atlasDirty = false;
    } batch{&ctx, false};

    PrStageSceneSubmitDirect::BuildStage1TimUploadCommands801CBFDC(
        runtime,
        +[](const PrStageSceneSubmitDirect::PsxTimUploadCommand801CBFDC& command,
            void* userData) {
            auto* batch = static_cast<UploadBatch*>(userData);
            if (batch == nullptr || batch->ctx == nullptr ||
                command.timId == 0u) {
                return;
            }
            UploadStage1FaceTimCommand801CBFDC(*batch->ctx,
                                                command.timId,
                                                command.uploadClut,
                                                batch->atlasDirty);
        },
        &batch);

    if (batch.atlasDirty) {
        UploadStage1VramAtlas(ctx);
    }
}

static bool IsStage1ClearLifecycleTmdBlendPauseActive801CBFDC190(
    const PrStage1FormalLifecycleSnapshot& snapshot) {
    return snapshot.valid &&
           (snapshot.clearGate ||
            snapshot.clearTerminalTailGate ||
            snapshot.runnerExitClearTerminalTailPulseInput ||
            snapshot.runnerExitClearTerminalTailPulseArmed ||
            snapshot.runnerExitClearTerminalTailArmed ||
            snapshot.runnerExitClearTerminalTailDispatchActive ||
            snapshot.runnerExitActiveDispatchTerminalPulseEmitted);
}

static bool ShouldPauseStage1TmdBlendForClear801CBFDC190() {
    const PrStageRunner& runner = GetStageRunner();
    if (runner.GetSceneId() == 1 &&
        runner.GetState() == StageRunnerState::Cleared) {
        return true;
    }

    PrStage1FormalLifecycleSnapshot snapshot{};
    if (PrScn1::CopyStage1FormalLifecycleSnapshot(snapshot) &&
        IsStage1ClearLifecycleTmdBlendPauseActive801CBFDC190(snapshot)) {
        return true;
    }
    if (PrScn1::CopyStage1TerminalFormalLifecycleSnapshot(snapshot) &&
        IsStage1ClearLifecycleTmdBlendPauseActive801CBFDC190(snapshot)) {
        return true;
    }

    const PrScn1::Stage1NumericRuntimeState& numeric =
        PrScn1::s_stage1NumericRuntime;
    const auto& tail = numeric.runnerMainLoopTail7A60;
    return numeric.active &&
           tail.known &&
           (tail.clearWord42 ||
            tail.clearWord50 ||
            tail.clearWord61 ||
            tail.finalCall1B120 ||
            tail.finalCall26FA4 ||
            tail.finalCall357D4Zero ||
            tail.finalCall1A694 ||
            (tail.cleanupRequiredKnown && tail.cleanupRequired));
}

static bool UpdateStage1TmdBlendRightRankStateChanged801CBFDC190() {
    const PrScn1::Stage1NumericRuntimeState& numeric =
        PrScn1::s_stage1NumericRuntime;
    if (!numeric.active) {
        s_stage1TmdBlendLastRightRankKnown801CBFDC190 = false;
        s_stage1TmdBlendLastRightRankRow801CBFDC190 = 0u;
        s_stage1TmdBlendLastRankTriggerMask801CBFDC190 = 0u;
        return false;
    }

    const uint8_t row = numeric.rightRankState.rightRankActiveRow;
    const bool rankStateChanged =
        s_stage1TmdBlendLastRightRankKnown801CBFDC190 &&
        row != s_stage1TmdBlendLastRightRankRow801CBFDC190;
    s_stage1TmdBlendLastRightRankKnown801CBFDC190 = true;
    s_stage1TmdBlendLastRightRankRow801CBFDC190 = row;
    return rankStateChanged;
}

static uint32_t BuildStage1TmdBlendRankTriggerMask801CBFDC190() {
    uint32_t mask = 0u;
    const PrScn1::Stage1NumericRuntimeState& numeric =
        PrScn1::s_stage1NumericRuntime;
    if (numeric.active) {
        if (numeric.rightRankDirectFollowUp.flag0200Pulse) {
            mask |= 0x00000001u;
        }
        if (numeric.rightRankDirectFollowUp.flag2000Pulse) {
            mask |= 0x00000002u;
        }
        if (numeric.rightRankDirectFollowUp.flag4000Pulse) {
            mask |= 0x00000004u;
        }
    }
    return mask;
}

static bool UpdateStage1TmdBlendRankTriggerEdge801CBFDC190() {
    const uint32_t mask = BuildStage1TmdBlendRankTriggerMask801CBFDC190();
    const bool edge =
        mask != 0u &&
        (mask & ~s_stage1TmdBlendLastRankTriggerMask801CBFDC190) != 0u;
    s_stage1TmdBlendLastRankTriggerMask801CBFDC190 = mask;
    return edge;
}

static void UpdateStage1TmdRenderOnlyBlendPause801CBFDC190(
    bool renderOnlyFrame) {
    if (renderOnlyFrame) {
        return;
    }

    constexpr uint32_t kRankTransitionPauseFrames = 240u;
    constexpr uint32_t kClearAnimationPauseFrames = 300u;
    uint32_t requestedPauseFrames = 0u;
    if (UpdateStage1TmdBlendRightRankStateChanged801CBFDC190() ||
        UpdateStage1TmdBlendRankTriggerEdge801CBFDC190()) {
        requestedPauseFrames =
            (std::max)(requestedPauseFrames, kRankTransitionPauseFrames);
    }
    if (ShouldPauseStage1TmdBlendForClear801CBFDC190()) {
        requestedPauseFrames =
            (std::max)(requestedPauseFrames, kClearAnimationPauseFrames);
    }

    if (requestedPauseFrames != 0u) {
        s_stage1TmdRenderOnlyBlendPauseFrames801CBFDC190 =
            (std::max)(s_stage1TmdRenderOnlyBlendPauseFrames801CBFDC190,
                       requestedPauseFrames);
    } else if (s_stage1TmdRenderOnlyBlendPauseFrames801CBFDC190 != 0u) {
        --s_stage1TmdRenderOnlyBlendPauseFrames801CBFDC190;
    }
}

bool AdvanceStage1SceneSubmitRuntimeForRender801CBFDC190(
    PrGameContext& ctx,
    const PrStage1RuntimeSlotsSnapshot& runtimeSlots,
    uint8_t renderSubFrame8) {
    PrStage1OverlayScriptTextSnapshot scriptText{};
    const bool scriptTextAvailable =
        PrScn1::GetStage1OverlayScriptTextRuntimeSnapshot(ctx, scriptText);
    const char* scriptBoxTextPtr10C =
        scriptTextAvailable ? scriptText.activeTextMirrorPtr : nullptr;
    const bool renderOnlyFrame = renderSubFrame8 != 0u;
    UpdateStage1TmdRenderOnlyBlendPause801CBFDC190(renderOnlyFrame);
    bool advanced = false;
    if (renderOnlyFrame) {
        s_stage1RenderOnlyRuntime801CBFDC190 =
            PrStageSceneSubmitDirect::
                GetOwnedStage1SceneSubmitRuntime801CBFDC190();
        advanced =
            PrStageSceneSubmitDirect::
                AdvanceStage1SceneSubmitRuntime801CBFDC190(
                    s_stage1RenderOnlyRuntime801CBFDC190,
                    runtimeSlots,
                    scriptBoxTextPtr10C,
                    // Keep render-only prediction in the non-logic path so
                    // one-shot action pulses are suppressed by the direct layer.
                    renderSubFrame8,
                    ResolveStage1SceneResourceBytes801CBFDC,
                    &ctx);
        s_stage1UseRenderOnlyRuntime801CBFDC190 = true;
    } else {
        s_stage1UseRenderOnlyRuntime801CBFDC190 = false;
        advanced =
            PrStageSceneSubmitDirect::
                AdvanceOwnedStage1SceneSubmitRuntime801CBFDC190(
                    runtimeSlots,
                    scriptBoxTextPtr10C,
                    renderSubFrame8,
                    ResolveStage1SceneResourceBytes801CBFDC,
                    &ctx);
    }
    if (!renderOnlyFrame) {
        ConsumeStage1FaceTimUploadCommands801CBFDC(
            ctx,
            PrStageSceneSubmitDirect::
                GetOwnedStage1SceneSubmitRuntime801CBFDC190());
    }
    if (!advanced && !renderOnlyFrame) {
        s_stage1UseRenderOnlyRuntime801CBFDC190 = false;
        ResetStage1FaceTimUploads801CBFDC();
    }
    return renderOnlyFrame ? true : advanced;
}

static const TmdModel* ResolveStage1DrawModelByDescAddr801CB190(
    uint32_t descAddr,
    const PrStageSceneSubmitDirect::PsxDrawModelRef& modelRef) {
    (void)descAddr;
    const TmdModel* models = s_stage1Models;
    const bool* loaded = s_stage1Loaded;
    int modelCount = kStage1TmdCount;
    if (modelRef.set ==
        PrStageSceneSubmitDirect::PsxDrawModelRef::Set::Stage1SceneMap) {
        models = s_stage1SceneMapModels;
        loaded = s_stage1SceneMapLoaded;
        modelCount = kStage1SceneMapTmdCount;
    }

    const int mi = static_cast<int>(modelRef.index);
    if (mi < 0 || mi >= modelCount || !loaded[mi]) {
        return nullptr;
    }
    return &models[mi];
}

static void EnsureStage1VramAtlas(PrGameContext& ctx) {
    if (ctx.renderer && !s_stage1AtlasReady) {
        s_stage1VramAtlas.UploadAll(ctx.renderer);
        s_stage1AtlasReady = true;
    }
}

static int Stage1PacketLayerFromOtBucket(int otBucket) {
    switch (otBucket) {
    case 0:
        return 784;
    case 1:
        return 782;
    case 2:
        return 781;
    case 3:
        return 780;
    default:
        return 779;
    }
}

static uint64_t Stage1PacketOrder(uint32_t groupOrder, uint32_t psxCallOrder) {
    return ((static_cast<uint64_t>(groupOrder) + 1u) << 32) |
           static_cast<uint64_t>(psxCallOrder + 1u);
}

static bool IsStage1DenkiCeilingLightTriangle(
    const PrStageSceneSubmitDirect::PsxDrawTriangleCommand428B0& command) {
    return command.textured && command.tpage == 0x0008u &&
           command.clut == 0x7802u;
}

static uint64_t Stage1RestoredCeilingLightOrder(
    const PrStageSceneSubmitDirect::PsxGpuPacketCommand428B0& packetCommand,
    uint8_t triangleIndex) {
    constexpr uint32_t kRestoreWorkListTailOrder = 0x00FFFF00u;
    const uint32_t localPacketOrder =
        kRestoreWorkListTailOrder + static_cast<uint32_t>(triangleIndex);
    return Stage1PacketOrder(
        packetCommand.workListFlushOrder,
        localPacketOrder);
}

static void DrawStage1TriangleCommand(
    PrGameContext& ctx,
    const PrStageSceneSubmitDirect::PsxDrawTriangleCommand428B0& command,
    const Stage1Viewport& viewport,
    int layer,
    uint64_t order,
    bool coverageBiasAllowed) {
    if (!ctx.renderer) {
        return;
    }

    const D3D11Renderer::BlendMode blend = ResolveBlendMode(command.blend);

    ID3D11ShaderResourceView* tpageSrv =
        command.textured
            ? s_stage1VramAtlas.GetTpageSRV(command.tpage,
                                            command.clut,
                                            ctx.renderer)
            : nullptr;
    if (command.textured && tpageSrv == nullptr) {
        return;
    }

    std::array<float, 3> x = command.x;
    std::array<float, 3> y = command.y;
    if (coverageBiasAllowed) {
        ApplyStage1TmdScaleAwareCoverageBias(viewport, x, y);
    }
    const float sx0 = viewport.x + x[0] * viewport.scale;
    const float sy0 = viewport.y + y[0] * viewport.scale;
    const float sx1 = viewport.x + x[1] * viewport.scale;
    const float sy1 = viewport.y + y[1] * viewport.scale;
    const float sx2 = viewport.x + x[2] * viewport.scale;
    const float sy2 = viewport.y + y[2] * viewport.scale;
    if (command.textured) {
        float u0 = 0.0f;
        float v0 = 0.0f;
        float u1 = 0.0f;
        float v1 = 0.0f;
        float u2 = 0.0f;
        float v2 = 0.0f;
        PsxVramAtlas::UVtoNormalized(command.u[0], command.v[0], u0, v0);
        PsxVramAtlas::UVtoNormalized(command.u[1], command.v[1], u1, v1);
        PsxVramAtlas::UVtoNormalized(command.u[2], command.v[2], u2, v2);

        TexturedVertex textured[3] = {
            {sx0, sy0, u0, v0, command.r[0], command.g[0], command.b[0], command.a[0], command.perspectiveW[0]},
            {sx1, sy1, u1, v1, command.r[1], command.g[1], command.b[1], command.a[1], command.perspectiveW[1]},
            {sx2, sy2, u2, v2, command.r[2], command.g[2], command.b[2], command.a[2], command.perspectiveW[2]},
        };
        D3D11Renderer::TexturedTriCmd tri{};
        tri.texture = tpageSrv;
        for (int i = 0; i < 3; ++i) {
            tri.vertices[i] = textured[i];
        }
        tri.vertexCount = 3;
        tri.blend = blend;
        tri.layer = layer;
        tri.order = order;
        const uint8_t replacementU[3] = {
            command.u[0],
            command.u[1],
            command.u[2],
        };
        const uint8_t replacementV[3] = {
            command.v[0],
            command.v[1],
            command.v[2],
        };
        ApplyStage1TexturedTriTextureReplacement(ctx,
                                                 "tmd_tri_428b0",
                                                 command.tpage,
                                                 command.clut,
                                                 replacementU,
                                                 replacementV,
                                                 3,
                                                 0x544D443432384230ULL,
                                                 tri);
        ctx.renderer->SubmitTexturedTriangles(tri);
        return;
    }

    ColorVertex colored[3] = {
        {sx0, sy0, command.r[0], command.g[0], command.b[0], command.a[0]},
        {sx1, sy1, command.r[1], command.g[1], command.b[1], command.a[1]},
        {sx2, sy2, command.r[2], command.g[2], command.b[2], command.a[2]},
    };
    D3D11Renderer::ColorTriCmd tri{};
    for (int i = 0; i < 3; ++i) {
        tri.vertices[i] = colored[i];
    }
    tri.vertexCount = 3;
    tri.blend = blend;
    tri.layer = layer;
    tri.order = order;
    ctx.renderer->SubmitColorTriangles(tri);
}

static void DrawStage1GpuPacketCommand428B0(
    PrGameContext& ctx,
    const PrStageSceneSubmitDirect::PsxGpuPacketCommand428B0& packetCommand,
    const Stage1Viewport& viewport,
    std::unordered_map<uint32_t, std::array<float, 2>>& vertexPositionCache) {
    using Handler = PrStageSceneSubmitDirect::PsxGpuPacketHandler428B0;
    if (!packetCommand.valid ||
        packetCommand.handler == Handler::Unsupported ||
        packetCommand.wordCount == 0u) {
        return;
    }

    const uint8_t triangleCount =
        Stage1GpuPacketIsQuad428B0(packetCommand.handler) ? 2u : 1u;
    for (uint8_t ti = 0; ti < triangleCount; ++ti) {
        const auto& slots = kStage1PacketTriangleSlots428B0[ti];
        PrStageSceneSubmitDirect::PsxDrawTriangleCommand428B0 command{};
        std::array<float, 3> packetXs{};
        std::array<float, 3> packetYs{};
        for (size_t i = 0; i < kStage1PacketTriangleVertexCount428B0; ++i) {
            const uint8_t slot = slots[i];
            const uint8_t sxyWordIndex =
                Stage1GpuPacketSxyWordIndex428B0(packetCommand.handler, slot);
            if (sxyWordIndex >= packetCommand.wordCount) {
                return;
            }
            const uint32_t sxyWord = packetCommand.words[sxyWordIndex];
            const int16_t packetX =
                Stage1GpuPacketUnpackSxyX428B0(sxyWord);
            const int16_t packetY =
                Stage1GpuPacketUnpackSxyY428B0(sxyWord);
            command.x[i] = static_cast<float>(packetX);
            command.y[i] = static_cast<float>(packetY);
            packetXs[i] = command.x[i];
            packetYs[i] = command.y[i];
            if (packetCommand.projectedScreenKnown800428B0 &&
                slot < packetCommand.projectedVertexCount800428B0) {
                command.x[i] =
                    packetCommand.projectedScreenX800428B0[slot];
                command.y[i] =
                    packetCommand.projectedScreenY800428B0[slot];
                if (packetCommand.projectedScreenZ800428B0[slot] > 0.0f) {
                    command.perspectiveW[i] =
                        packetCommand.projectedScreenZ800428B0[slot];
                }
            }
            if (command.perspectiveW[i] <= 1.0f &&
                packetCommand.projectedDepthKnown800428B0 &&
                slot < packetCommand.projectedVertexCount800428B0 &&
                packetCommand.projectedDepth800428B0[slot] > 0) {
                command.perspectiveW[i] = static_cast<float>(
                    packetCommand.projectedDepth800428B0[slot]);
            }
            ResolveStage1TmdVertexPosition428B0(
                vertexPositionCache,
                packetX,
                packetY,
                command.x[i],
                command.y[i],
                command.x[i],
                command.y[i]);

            const uint8_t uvWordIndex =
                Stage1GpuPacketUvWordIndex428B0(packetCommand.handler, slot);
            if (uvWordIndex >= packetCommand.wordCount && uvWordIndex != 0u) {
                return;
            }
            const uint32_t uvWord =
                uvWordIndex != 0u ? packetCommand.words[uvWordIndex] : 0u;
            command.u[i] = Stage1GpuPacketUnpackUvU428B0(uvWord);
            command.v[i] = Stage1GpuPacketUnpackUvV428B0(uvWord);
        }

        command.textured =
            Stage1GpuPacketIsTextured428B0(packetCommand.handler);
        const uint8_t uv0WordIndex =
            Stage1GpuPacketUvWordIndex428B0(packetCommand.handler, 0);
        const uint8_t uv1WordIndex =
            Stage1GpuPacketUvWordIndex428B0(packetCommand.handler, 1);
        command.tpage =
            (command.textured && uv1WordIndex != 0u &&
             uv1WordIndex < packetCommand.wordCount)
                ? static_cast<uint16_t>(packetCommand.words[uv1WordIndex] >>
                                        16)
                : 0u;
        command.clut =
            (command.textured && uv0WordIndex != 0u &&
             uv0WordIndex < packetCommand.wordCount)
                ? static_cast<uint16_t>(packetCommand.words[uv0WordIndex] >>
                                        16)
                : 0u;

        for (size_t i = 0; i < kStage1PacketTriangleVertexCount428B0; ++i) {
            const uint8_t slot = slots[i];
            const uint8_t colorWordIndex =
                Stage1GpuPacketColorWordIndex428B0(packetCommand.handler,
                                                   slot);
            if (colorWordIndex >= packetCommand.wordCount) {
                return;
            }
            const uint32_t colorWord = packetCommand.words[colorWordIndex];
            command.r[i] = Stage1GpuPacketColorByteToFloat428B0(
                colorWord,
                0,
                command.textured);
            command.g[i] = Stage1GpuPacketColorByteToFloat428B0(
                colorWord,
                8,
                command.textured);
            command.b[i] = Stage1GpuPacketColorByteToFloat428B0(
                colorWord,
                16,
                command.textured);
        }

        const bool semiTransparencyApplies =
            packetCommand.semiTransparent &&
            Stage1GpuPacketSemiTransparencyApplies428B0(command.textured,
                                                        command.clut);
        float alphaScale = 1.0f;
        if (semiTransparencyApplies) {
            switch (packetCommand.abr & 0x03u) {
                case 1u:
                case 2u:
                    alphaScale = 1.0f;
                    break;
                case 3u:
                    alphaScale = 0.25f;
                    break;
                case 0u:
                default:
                    alphaScale = 0.5f;
                    break;
            }
        }
        command.a = {alphaScale, alphaScale, alphaScale};
        command.blend = kStage1PacketBlendAlpha428B0;
        if (semiTransparencyApplies) {
            switch (packetCommand.abr & 0x03u) {
                case 1u:
                case 3u:
                    command.blend = kStage1PacketBlendAdditive428B0;
                    break;
                case 2u:
                    command.blend = kStage1PacketBlendSubtractive428B0;
                    break;
                case 0u:
                default:
                    command.blend = kStage1PacketBlendAlpha428B0;
                    break;
            }
        }

        const int layer =
            Stage1PacketLayerFromOtBucket(packetCommand.otBucket);
        const uint64_t order = Stage1PacketOrder(
            packetCommand.workListFlushOrder,
            packetCommand.psxCallOrder);
        const bool triangleCoverageBiasAllowed = false;
        if (ShouldCullStage1HdSceneNeedleTriangle(ctx,
                                                  packetCommand,
                                                  packetXs,
                                                  packetYs,
                                                  command.x,
                                                  command.y)) {
            continue;
        }
        DrawStage1TriangleCommand(
            ctx,
            command,
            viewport,
            layer,
            order,
            triangleCoverageBiasAllowed);
        if (ctx.stage1RestoreCeilingLights &&
            IsStage1DenkiCeilingLightTriangle(command)) {
            DrawStage1TriangleCommand(
                ctx,
                command,
                viewport,
                layer,
                Stage1RestoredCeilingLightOrder(packetCommand, ti),
                triangleCoverageBiasAllowed);
        }
    }
}

static void DrawStage1Backdrop801CAC34(
    PrGameContext& ctx,
    const PrStageSceneSubmitDirect::Stage1SceneSubmitRuntime& runtime,
    const Stage1Viewport& viewport) {
    Stage1BackdropDrawContext801CAC34 drawCtx{&ctx, viewport};
    PrStageSceneSubmitDirect::BuildStage1BackdropCommands801CAC34(
        runtime,
        +[](const PrStageSceneSubmitDirect::PsxBackdropGradientCommand801CAC34& gradient,
            const PrStageSceneSubmitDirect::PsxBackdropSpriteCommand801CAC34* sprites,
            std::size_t spriteCount,
            void* userData) {
            auto* drawCtx =
                static_cast<Stage1BackdropDrawContext801CAC34*>(userData);
            if (drawCtx == nullptr || drawCtx->ctx == nullptr) {
                return;
            }
            DrawStage1BackdropCommandBatch(*drawCtx->ctx,
                                           gradient,
                                           sprites,
                                           spriteCount,
                                           drawCtx->viewport);
        },
        &drawCtx);
}

static bool Stage1RenderOnlyFramePhaseActive428B0(const PrGameContext& ctx) {
    return ctx.renderOnlyFrame && ctx.renderSubFrame8 > 0u;
}

static bool Stage1RenderOnlyBlendActive428B0(const PrGameContext& ctx) {
    return s_stage1UseRenderOnlyRuntime801CBFDC190 &&
           Stage1RenderOnlyFramePhaseActive428B0(ctx);
}

static float Stage1RenderOnlyBlendT428B0(const PrGameContext& ctx) {
    return std::clamp(static_cast<float>(ctx.renderSubFrame8) / 255.0f,
                      0.0f,
                      1.0f);
}

static bool Stage1TmdRenderOnlyBlendPaused428B0() {
    return s_stage1TmdRenderOnlyBlendPauseFrames801CBFDC190 != 0u;
}

static float Stage1Lerp428B0(float a, float b, float t) {
    return a + (b - a) * t;
}

static int32_t Stage1LerpI32Round428B0(int32_t a, int32_t b, float t) {
    return static_cast<int32_t>(
        std::lround(Stage1Lerp428B0(static_cast<float>(a),
                                    static_cast<float>(b),
                                    t)));
}

static uint32_t PackStage1SxyWord428B0(float x, float y) {
    const int32_t sx =
        std::clamp<int32_t>(static_cast<int32_t>(std::lround(x)),
                            INT16_MIN,
                            INT16_MAX);
    const int32_t sy =
        std::clamp<int32_t>(static_cast<int32_t>(std::lround(y)),
                            INT16_MIN,
                            INT16_MAX);
    return static_cast<uint16_t>(sx) |
           (static_cast<uint32_t>(static_cast<uint16_t>(sy)) << 16);
}

static Stage1GpuPacketBlendKey428B0 Stage1GpuPacketBlendKey(
    const PrStageSceneSubmitDirect::PsxGpuPacketCommand428B0& command) {
    Stage1GpuPacketBlendKey428B0 key{};
    key.sourceModelSet = command.sourceModelSet800428B0;
    key.sourceModelIndex = command.sourceModelIndex800428B0;
    key.workListFlushOrder = command.workListFlushOrder;
    key.rawPrimitiveIndex = command.rawPrimitiveIndex800428B0;
    key.rawPacketOffset = command.rawPacketOffset800428B0;
    key.rawPacketByteSize = command.rawPacketByteSize800428B0;
    key.handler = static_cast<uint8_t>(command.handler);
    key.primitiveCode = command.primitiveCode;
    return key;
}

static bool CanBlendStage1GpuPacket428B0(
    const PrStageSceneSubmitDirect::PsxGpuPacketCommand428B0& current,
    const PrStageSceneSubmitDirect::PsxGpuPacketCommand428B0& next) {
    return current.valid &&
           next.valid &&
           current.handler == next.handler &&
           current.wordCount == next.wordCount &&
           current.projectedScreenKnown800428B0 &&
           next.projectedScreenKnown800428B0 &&
           current.projectedVertexCount800428B0 ==
               next.projectedVertexCount800428B0 &&
           current.projectedVertexCount800428B0 > 0u &&
           current.sourceModelRefKnown800428B0 ==
               next.sourceModelRefKnown800428B0 &&
           current.sourceModelSet800428B0 ==
               next.sourceModelSet800428B0 &&
           current.sourceModelIndex800428B0 ==
               next.sourceModelIndex800428B0 &&
           current.workListFlushOrder == next.workListFlushOrder &&
           current.primitiveCode == next.primitiveCode &&
           current.rawPrimitiveIndex800428B0 ==
               next.rawPrimitiveIndex800428B0 &&
           current.rawPacketOffset800428B0 ==
               next.rawPacketOffset800428B0 &&
           current.rawPacketByteSize800428B0 ==
               next.rawPacketByteSize800428B0;
}

static PrStageSceneSubmitDirect::PsxGpuPacketCommand428B0
BlendStage1GpuPacket428B0(
    const PrStageSceneSubmitDirect::PsxGpuPacketCommand428B0& current,
    const PrStageSceneSubmitDirect::PsxGpuPacketCommand428B0& next,
    float t) {
    PrStageSceneSubmitDirect::PsxGpuPacketCommand428B0 out = next;
    out.otBucket = current.otBucket;
    out.workListFlushOrder = current.workListFlushOrder;
    out.psxCallOrder = current.psxCallOrder;
    out.otz = static_cast<uint16_t>(
        std::clamp<int32_t>(Stage1LerpI32Round428B0(current.otz,
                                                    next.otz,
                                                    t),
                            0,
                            0xFFFF));
    const uint8_t count =
        (std::min)(current.projectedVertexCount800428B0,
                   next.projectedVertexCount800428B0);
    out.projectedVertexCount800428B0 = count;
    out.projectedScreenKnown800428B0 = true;
    out.projectedDepthKnown800428B0 =
        current.projectedDepthKnown800428B0 &&
        next.projectedDepthKnown800428B0;
    for (uint8_t i = 0; i < count; ++i) {
        out.projectedScreenX800428B0[i] =
            Stage1Lerp428B0(current.projectedScreenX800428B0[i],
                            next.projectedScreenX800428B0[i],
                            t);
        out.projectedScreenY800428B0[i] =
            Stage1Lerp428B0(current.projectedScreenY800428B0[i],
                            next.projectedScreenY800428B0[i],
                            t);
        out.projectedScreenZ800428B0[i] =
            Stage1Lerp428B0(current.projectedScreenZ800428B0[i],
                            next.projectedScreenZ800428B0[i],
                            t);
        if (out.projectedDepthKnown800428B0) {
            out.projectedDepth800428B0[i] =
                Stage1LerpI32Round428B0(current.projectedDepth800428B0[i],
                                        next.projectedDepth800428B0[i],
                                        t);
        }
        const uint8_t sxyWordIndex =
            Stage1GpuPacketSxyWordIndex428B0(out.handler, i);
        if (sxyWordIndex < out.wordCount && sxyWordIndex < out.words.size()) {
            out.words[sxyWordIndex] =
                PackStage1SxyWord428B0(out.projectedScreenX800428B0[i],
                                       out.projectedScreenY800428B0[i]);
        }
    }
    return out;
}

static std::vector<Stage1GpuPacketDrawEntry428B0>
CollectStage1GpuPacketCommands428B0(
    PrGameContext& ctx,
    const PrStageSceneSubmitDirect::Stage1SceneSubmitRuntime& runtime) {
    Stage1GpuPacketCollectContext428B0 collectCtx{};
    PrStageSceneSubmitDirect::BuildStage1GpuPacketCommands428B0(
        runtime,
        +[](uint32_t descAddr,
            const PrStageSceneSubmitDirect::PsxDrawModelRef& modelRef,
            void*) -> const TmdModel* {
            return ResolveStage1DrawModelByDescAddr801CB190(descAddr, modelRef);
        },
        nullptr,
        ResolveStage1MimeResources801CB190,
        &ctx,
        +[](const PrStageSceneSubmitDirect::PsxGpuPacketCommand428B0& command,
            void* userData) {
            auto* collectCtx =
                static_cast<Stage1GpuPacketCollectContext428B0*>(userData);
            if (collectCtx == nullptr) {
                return;
            }
            Stage1GpuPacketDrawEntry428B0 entry{};
            entry.command = command;
            entry.serial = collectCtx->entries.size();
            collectCtx->entries.push_back(entry);
        },
        &collectCtx);
    return collectCtx.entries;
}

static void DrawStage1GpuPacketEntries428B0(
    PrGameContext& ctx,
    const Stage1Viewport& viewport,
    const std::vector<Stage1GpuPacketDrawEntry428B0>& entries) {
    Stage1TmdDrawContext428B0 drawCtx{&ctx, viewport};
    for (const Stage1GpuPacketDrawEntry428B0& entry : entries) {
        ObserveStage1SceneSubmit428B0Debug(entry.command);
        DrawStage1GpuPacketCommand428B0(ctx,
                                        entry.command,
                                        viewport,
                                        drawCtx.vertexPositionCache);
    }
}

static std::vector<Stage1GpuPacketDrawEntry428B0>
BlendStage1GpuPacketEntries428B0(
    const std::vector<Stage1GpuPacketDrawEntry428B0>& currentEntries,
    const std::vector<Stage1GpuPacketDrawEntry428B0>& nextEntries,
    float t,
    Stage1GpuPacketBlendStats428B0* outStats) {
    if (outStats != nullptr) {
        *outStats = Stage1GpuPacketBlendStats428B0{};
    }
    std::unordered_map<
        Stage1GpuPacketBlendKey428B0,
        Stage1GpuPacketBlendKeyUse428B0,
        Stage1GpuPacketBlendKeyHash428B0>
        currentByKey;
    for (std::size_t i = 0; i < currentEntries.size(); ++i) {
        Stage1GpuPacketBlendKeyUse428B0& use =
            currentByKey[Stage1GpuPacketBlendKey(currentEntries[i].command)];
        if (use.count == 0u) {
            use.index = i;
        }
        ++use.count;
    }

    std::unordered_map<
        Stage1GpuPacketBlendKey428B0,
        Stage1GpuPacketBlendKeyUse428B0,
        Stage1GpuPacketBlendKeyHash428B0>
        nextByKey;
    for (std::size_t i = 0; i < nextEntries.size(); ++i) {
        Stage1GpuPacketBlendKeyUse428B0& use =
            nextByKey[Stage1GpuPacketBlendKey(nextEntries[i].command)];
        if (use.count == 0u) {
            use.index = i;
        }
        ++use.count;
    }

    std::vector<Stage1GpuPacketDrawEntry428B0> blended;
    blended.reserve(currentEntries.size());
    for (const Stage1GpuPacketDrawEntry428B0& currentEntry : currentEntries) {
        Stage1GpuPacketDrawEntry428B0 outEntry = currentEntry;
        const Stage1GpuPacketBlendKey428B0 currentKey =
            Stage1GpuPacketBlendKey(currentEntry.command);
        const auto currentFound = currentByKey.find(currentKey);
        const auto nextFound = nextByKey.find(currentKey);
        const PrStageSceneSubmitDirect::PsxGpuPacketCommand428B0* nextCommand =
            nullptr;
        const bool ambiguousKey =
            currentFound != currentByKey.end() &&
            nextFound != nextByKey.end() &&
            (currentFound->second.count != 1u ||
             nextFound->second.count != 1u);
        if (nextFound != nextByKey.end() && !ambiguousKey) {
            nextCommand = &nextEntries[nextFound->second.index].command;
        }
        if (nextCommand != nullptr && outStats != nullptr) {
            ++outStats->matchedCount;
        }
        if (nextCommand != nullptr &&
            CanBlendStage1GpuPacket428B0(currentEntry.command,
                                         *nextCommand)) {
            if (outStats != nullptr) {
                ++outStats->appliedCount;
                if (!outStats->firstDeltaKnown &&
                    currentEntry.command.projectedVertexCount800428B0 > 0u &&
                    nextCommand->projectedVertexCount800428B0 > 0u) {
                    outStats->firstDeltaKnown = true;
                    outStats->firstDeltaX =
                        nextCommand->projectedScreenX800428B0[0] -
                        currentEntry.command.projectedScreenX800428B0[0];
                    outStats->firstDeltaY =
                        nextCommand->projectedScreenY800428B0[0] -
                        currentEntry.command.projectedScreenY800428B0[0];
                }
            }
            outEntry.command =
                BlendStage1GpuPacket428B0(currentEntry.command,
                                          *nextCommand,
                                          t);
        }
        blended.push_back(outEntry);
    }
    return blended;
}

static void DrawStage1Tmd428B0(
    PrGameContext& ctx,
    const PrStageSceneSubmitDirect::Stage1SceneSubmitRuntime& runtime,
    const Stage1Viewport& viewport) {
    BeginStage1SceneSubmit428B0Debug(ctx.renderer != nullptr);
    if (!ctx.renderer) {
        return;
    }

    EnsureStage1VramAtlas(ctx);

    if (Stage1RenderOnlyBlendActive428B0(ctx) &&
        Stage1TmdRenderOnlyBlendPaused428B0()) {
        const std::vector<Stage1GpuPacketDrawEntry428B0> currentEntries =
            CollectStage1GpuPacketCommands428B0(
                ctx,
                PrStageSceneSubmitDirect::
                    GetOwnedStage1SceneSubmitRuntime801CBFDC190());
        s_stage1SceneSubmit428B0Debug.renderOnlyBlendAttempted = true;
        s_stage1SceneSubmit428B0Debug.renderOnlyBlendPaused = true;
        s_stage1SceneSubmit428B0Debug.renderOnlyBlendPauseFrames =
            s_stage1TmdRenderOnlyBlendPauseFrames801CBFDC190;
        s_stage1SceneSubmit428B0Debug.blendCurrentCount =
            static_cast<uint32_t>(currentEntries.size());
        DrawStage1GpuPacketEntries428B0(ctx, viewport, currentEntries);
        return;
    }

    if (Stage1RenderOnlyBlendActive428B0(ctx)) {
        const std::vector<Stage1GpuPacketDrawEntry428B0> currentEntries =
            CollectStage1GpuPacketCommands428B0(
                ctx,
                PrStageSceneSubmitDirect::
                    GetOwnedStage1SceneSubmitRuntime801CBFDC190());
        const std::vector<Stage1GpuPacketDrawEntry428B0> nextEntries =
            CollectStage1GpuPacketCommands428B0(ctx, runtime);
        Stage1GpuPacketBlendStats428B0 blendStats{};
        const std::vector<Stage1GpuPacketDrawEntry428B0> blendedEntries =
            BlendStage1GpuPacketEntries428B0(currentEntries,
                                             nextEntries,
                                             Stage1RenderOnlyBlendT428B0(ctx),
                                             &blendStats);
        s_stage1SceneSubmit428B0Debug.renderOnlyBlendAttempted = true;
        s_stage1SceneSubmit428B0Debug.blendCurrentCount =
            static_cast<uint32_t>(currentEntries.size());
        s_stage1SceneSubmit428B0Debug.blendNextCount =
            static_cast<uint32_t>(nextEntries.size());
        s_stage1SceneSubmit428B0Debug.blendMatchedCount =
            blendStats.matchedCount;
        s_stage1SceneSubmit428B0Debug.blendAppliedCount =
            blendStats.appliedCount;
        s_stage1SceneSubmit428B0Debug.blendFirstDeltaKnown =
            blendStats.firstDeltaKnown;
        s_stage1SceneSubmit428B0Debug.blendFirstDeltaX =
            blendStats.firstDeltaX;
        s_stage1SceneSubmit428B0Debug.blendFirstDeltaY =
            blendStats.firstDeltaY;
        DrawStage1GpuPacketEntries428B0(ctx, viewport, blendedEntries);
        return;
    }

    DrawStage1GpuPacketEntries428B0(
        ctx,
        viewport,
        CollectStage1GpuPacketCommands428B0(ctx, runtime));
}

static int Stage1FastSpritePacketLayer8003FA20(uint16_t priority) {
    return Stage1PacketLayerFromOtBucket(priority);
}

static float PsxBoxFillAlpha8003EE84(uint8_t commandCode) {
    const bool semiTransparent = (commandCode & 0x02u) != 0u;
    return semiTransparent ? 0.5f : 1.0f;
}

static bool PsxFastSpriteSemiTransparent8003FA20(uint32_t word2ColorCode) {
    const uint8_t commandCode =
        static_cast<uint8_t>((word2ColorCode >> 24) & 0xFFu);
    return (commandCode & 0x02u) != 0u;
}

static bool PsxFastSpriteRawTexture8003FA20(uint32_t word2ColorCode) {
    const uint8_t commandCode =
        static_cast<uint8_t>((word2ColorCode >> 24) & 0xFFu);
    return (commandCode & 0x01u) != 0u;
}

static uint8_t PsxFastSpriteAbr8003FA20(uint32_t word1DrawMode) {
    return static_cast<uint8_t>((word1DrawMode >> 5) & 0x03u);
}

static float PsxFastSpriteAlpha8003FA20(uint32_t word1DrawMode,
                                        uint32_t word2ColorCode,
                                        bool textureSemiTransparencyApplies) {
    if (!PsxFastSpriteSemiTransparent8003FA20(word2ColorCode) ||
        !textureSemiTransparencyApplies) {
        return 1.0f;
    }
    switch (PsxFastSpriteAbr8003FA20(word1DrawMode)) {
        case 1u:
        case 2u:
            return 1.0f;
        case 3u:
            return 0.25f;
        case 0u:
        default:
            return 0.5f;
    }
}

static D3D11Renderer::BlendMode PsxFastSpriteBlend8003FA20(
    uint32_t word1DrawMode,
    uint32_t word2ColorCode,
    bool textureSemiTransparencyApplies) {
    if (!PsxFastSpriteSemiTransparent8003FA20(word2ColorCode) ||
        !textureSemiTransparencyApplies) {
        return D3D11Renderer::BlendMode::Alpha;
    }
    switch (PsxFastSpriteAbr8003FA20(word1DrawMode)) {
        case 1u:
        case 3u:
            return D3D11Renderer::BlendMode::Additive;
        case 2u:
            return D3D11Renderer::BlendMode::Subtractive;
        case 0u:
        default:
            return D3D11Renderer::BlendMode::Alpha;
    }
}

static void DrawStage1BoxFillPacket8003EE84(
    PrGameContext& ctx,
    const PrStageSceneSubmitDirect::PsxBoxFillPacketCommand8003EE84& command,
    const Stage1Viewport& viewport) {
    if (!ctx.renderer || !command.valid || !command.wordsKnown ||
        command.w == 0u || command.h == 0u) {
        return;
    }
    D3D11Renderer::SolidRectCmd rect{};
    rect.x = Stage1ScreenX(viewport, static_cast<float>(command.x));
    rect.y = Stage1ScreenY(viewport, static_cast<float>(command.y));
    rect.w = Stage1ScreenLength(viewport, static_cast<float>(command.w));
    rect.h = Stage1ScreenLength(viewport, static_cast<float>(command.h));
    rect.r = static_cast<float>(command.r) / 255.0f;
    rect.g = static_cast<float>(command.g) / 255.0f;
    rect.b = static_cast<float>(command.b) / 255.0f;
    rect.a = PsxBoxFillAlpha8003EE84(command.commandCode);
    rect.layer = Stage1FastSpritePacketLayer8003FA20(command.priority);
    rect.order = Stage1PacketOrder(0u, command.psxCallOrder);
    ctx.renderer->SubmitSolidRect(rect);
}

static void DrawStage1BoxFillPackets8003EE84(
    PrGameContext& ctx,
    const PrStageSceneSubmitDirect::Stage1SceneSubmitRuntime& runtime,
    const Stage1Viewport& viewport) {
    if (!ctx.renderer) {
        return;
    }

    Stage1TmdDrawContext428B0 drawCtx{&ctx, viewport};
    PrStageSceneSubmitDirect::BuildStage1BoxFillPacketCommands8003EE84(
        runtime,
        +[](const PrStageSceneSubmitDirect::PsxBoxFillPacketCommand8003EE84&
                command,
            void* userData) {
            auto* drawCtx = static_cast<Stage1TmdDrawContext428B0*>(userData);
            if (drawCtx == nullptr || drawCtx->ctx == nullptr) {
                return;
            }
            DrawStage1BoxFillPacket8003EE84(*drawCtx->ctx,
                                            command,
                                            drawCtx->viewport);
        },
        &drawCtx);
}

static void DrawEventFrameBoxFillPacket8003EE84(
    PrGameContext& ctx,
    const PrPsxEventFrameDirect::BoxFillPacketCommand8003EE84& command,
    const Stage1Viewport& viewport) {
    if (!ctx.renderer || !command.valid || !command.wordsKnown ||
        command.w == 0u || command.h == 0u) {
        return;
    }

    D3D11Renderer::SolidRectCmd rect{};
    rect.x = Stage1ScreenX(viewport, static_cast<float>(command.x));
    rect.y = Stage1ScreenY(viewport, static_cast<float>(command.y));
    rect.w = Stage1ScreenLength(viewport, static_cast<float>(command.w));
    rect.h = Stage1ScreenLength(viewport, static_cast<float>(command.h));
    rect.r = static_cast<float>(command.r) / 255.0f;
    rect.g = static_cast<float>(command.g) / 255.0f;
    rect.b = static_cast<float>(command.b) / 255.0f;
    rect.a = PsxBoxFillAlpha8003EE84(command.commandCode);
    rect.layer = Stage1FastSpritePacketLayer8003FA20(command.priority);
    rect.order = Stage1PacketOrder(0u, command.psxCallOrder);
    ctx.renderer->SubmitSolidRect(rect);
}

static bool IsStage1NativeSubtitleTextPacket8003FA20(
    const PrStageSceneSubmitDirect::PsxFastSpritePacketCommand8003FA20&
        command) {
    using SourceKind =
        PrPsxFastSpriteSubmitDirect::FastSpriteSubmitSourceKind8003FA20;
    using RawLocal =
        PrPsxFastSpriteSubmitDirect::FastSpriteRawLocalProvenance8003FA20;
    const SourceKind sourceKind = static_cast<SourceKind>(command.sourceKind);
    const RawLocal rawLocal = static_cast<RawLocal>(command.rawLocalProvenance);
    if (rawLocal != RawLocal::Stage1GlyphStackScratch8001B954) {
        return false;
    }
    return sourceKind == SourceKind::Stage1MovieText ||
           sourceKind == SourceKind::Stage1ScriptBox;
}

static void ObserveStage1NativeSubtitleTextPacket8003FA20(
    PrGameContext& ctx,
    const PrStageSceneSubmitDirect::PsxFastSpritePacketCommand8003FA20&
        command,
    const Stage1Viewport& viewport) {
    if (!IsStage1NativeSubtitleTextPacket8003FA20(command)) {
        return;
    }

    using SourceKind =
        PrPsxFastSpriteSubmitDirect::FastSpriteSubmitSourceKind8003FA20;
    const SourceKind sourceKind = static_cast<SourceKind>(command.sourceKind);
    const PrStage1HdSubtitleSourceKind subtitleKind =
        sourceKind == SourceKind::Stage1ScriptBox
            ? PrStage1HdSubtitleSourceKind::OverlayScriptText
            : PrStage1HdSubtitleSourceKind::Movie1;
    const int16_t screenX = command.drawEnvOffsetKnown800401AC
        ? static_cast<int16_t>(command.x + command.drawEnvOffsetX80091738)
        : command.x;
    const int16_t screenY = command.drawEnvOffsetKnown800401AC
        ? static_cast<int16_t>(command.y + command.drawEnvOffsetY8009173A)
        : command.y;
    PrStage1HdSubtitles::ObserveNativeSubtitleTextRect(
        ctx,
        subtitleKind,
        Stage1ScreenX(viewport, static_cast<float>(screenX)),
        Stage1ScreenY(viewport, static_cast<float>(screenY)),
        Stage1ScreenLength(viewport, static_cast<float>(command.w)),
        Stage1ScreenLength(viewport, static_cast<float>(command.h)));
}

static bool ShouldDrawStage1FastSpritePacket8003FA20(
    PrGameContext& ctx,
    const PrStageSceneSubmitDirect::PsxFastSpritePacketCommand8003FA20&
        command) {
    using SourceKind =
        PrPsxFastSpriteSubmitDirect::FastSpriteSubmitSourceKind8003FA20;
    if (!command.valid || !command.provenanceKnown ||
        !command.word2CommandKnown ||
        (!command.rawTexture && !command.colorAuthoritative) ||
        !command.renderPayloadKnown || command.auditOnlyPartial) {
        return false;
    }

    const SourceKind sourceKind = static_cast<SourceKind>(command.sourceKind);
    if (PrStage1HdSubtitles::ShouldSuppressNativeSubtitleText(ctx) &&
        IsStage1NativeSubtitleTextPacket8003FA20(command)) {
        return false;
    }
    return sourceKind == SourceKind::Movie1RawDraw ||
           sourceKind == SourceKind::Stage1Hud ||
           sourceKind == SourceKind::Stage1ScriptBox ||
           sourceKind == SourceKind::Stage1MovieText;
}

static bool ShouldDrawEventFrameFastSpritePacket8003FA20(
    const PrStageSceneSubmitDirect::PsxFastSpritePacketCommand8003FA20&
        command) {
    using SourceKind =
        PrPsxFastSpriteSubmitDirect::FastSpriteSubmitSourceKind8003FA20;
    if (!command.valid || !command.provenanceKnown ||
        !command.word2CommandKnown ||
        (!command.rawTexture && !command.colorAuthoritative) ||
        !command.renderPayloadKnown || command.auditOnlyPartial) {
        return false;
    }

    const SourceKind sourceKind = static_cast<SourceKind>(command.sourceKind);
    return sourceKind == SourceKind::Stage1EventFramePrompt ||
           sourceKind == SourceKind::Stage1EventFrameStageSelect ||
           sourceKind == SourceKind::Stage1EventFrameBackdrop ||
           sourceKind == SourceKind::Stage1EventFrameSaveUi;
}

static void SubmitFastSpritePacket8003FA20(
    PrGameContext& ctx,
    const PrStageSceneSubmitDirect::PsxFastSpritePacketCommand8003FA20&
        command,
    const Stage1Viewport& viewport) {
    if (!ctx.renderer) {
        return;
    }

    ID3D11ShaderResourceView* srv =
        s_stage1VramAtlas.GetTpageSRV(command.tpage,
                                      command.clut,
                                      ctx.renderer);
    if (srv == nullptr) {
        return;
    }

    D3D11Renderer::SpriteCmd cmd{};
    cmd.texture = srv;
    const int16_t screenX = command.drawEnvOffsetKnown800401AC
        ? static_cast<int16_t>(command.x + command.drawEnvOffsetX80091738)
        : command.x;
    const int16_t screenY = command.drawEnvOffsetKnown800401AC
        ? static_cast<int16_t>(command.y + command.drawEnvOffsetY8009173A)
        : command.y;
    cmd.x = Stage1ScreenX(viewport, static_cast<float>(screenX));
    cmd.y = Stage1ScreenY(viewport, static_cast<float>(screenY));
    cmd.w = Stage1ScreenLength(viewport, static_cast<float>(command.w));
    cmd.h = Stage1ScreenLength(viewport, static_cast<float>(command.h));
    Stage1SpriteUvCenterEndpoints(command.u,
                                  command.v,
                                  command.w,
                                  command.h,
                                  cmd.u0,
                                  cmd.v0,
                                  cmd.u1,
                                  cmd.v1);
    ApplyStage1SpriteTextureReplacement(ctx,
                                        "fast_sprite_8003fa20",
                                        command.tpage,
                                        command.clut,
                                        command.u,
                                        command.v,
                                        command.w,
                                        command.h,
                                        0u,
                                        cmd);
    const uint32_t word2ForRender =
        command.wordKnown[2]
            ? command.words[2]
            : (static_cast<uint32_t>(command.word2CommandCode) << 24);
    const bool rawTexture =
        command.rawTextureKnown
            ? command.rawTexture
            : PsxFastSpriteRawTexture8003FA20(word2ForRender);
    bool clutHasStpBits = false;
    const bool clutStpKnown =
        s_stage1VramAtlas.TryGetClutHasStpBits(command.clut,
                                               clutHasStpBits);
    // Textured primitives only become semi-transparent for CLUT texels whose
    // STP bit is set. Unknown CLUTs preserve the conservative old behavior.
    const bool textureSemiTransparencyApplies =
        !clutStpKnown || clutHasStpBits;
    cmd.r = rawTexture ? 1.0f : static_cast<float>(command.r) / 255.0f;
    cmd.g = rawTexture ? 1.0f : static_cast<float>(command.g) / 255.0f;
    cmd.b = rawTexture ? 1.0f : static_cast<float>(command.b) / 255.0f;
    cmd.a = PsxFastSpriteAlpha8003FA20(command.words[1],
                                       word2ForRender,
                                       textureSemiTransparencyApplies);
    cmd.blend = PsxFastSpriteBlend8003FA20(command.words[1],
                                           word2ForRender,
                                           textureSemiTransparencyApplies);
    cmd.layer = Stage1FastSpritePacketLayer8003FA20(command.priority);
    cmd.order = Stage1PacketOrder(0u, command.psxCallOrder);
    ctx.renderer->SubmitSprite(cmd);
}

static void DrawStage1FastSpritePacket8003FA20(
    PrGameContext& ctx,
    const PrStageSceneSubmitDirect::PsxFastSpritePacketCommand8003FA20&
        command,
    const Stage1Viewport& viewport) {
    ObserveStage1NativeSubtitleTextPacket8003FA20(ctx, command, viewport);
    const bool shouldDraw = ShouldDrawStage1FastSpritePacket8003FA20(ctx,
                                                                     command);
    using SourceKind =
        PrPsxFastSpriteSubmitDirect::FastSpriteSubmitSourceKind8003FA20;
    const SourceKind sourceKind = static_cast<SourceKind>(command.sourceKind);
    static uint32_t s_tailMovieTextPacketTraceCount = 0;
    const bool clearTailStrActive =
        ctx.strPlayer != nullptr && ctx.strPlayer->GetTotalFrames() == 245u;
    if (clearTailStrActive &&
        s_tailMovieTextPacketTraceCount < 96u &&
        IsStage1NativeSubtitleTextPacket8003FA20(command) &&
        sourceKind == SourceKind::Stage1MovieText) {
        ++s_tailMovieTextPacketTraceCount;
        const bool suppressNative =
            PrStage1HdSubtitles::ShouldSuppressNativeSubtitleText(ctx);
        const bool srvReady =
            ctx.renderer != nullptr &&
            s_stage1VramAtlas.GetTpageSRV(command.tpage,
                                          command.clut,
                                          ctx.renderer) != nullptr;
        Log::Printf(
            "[TailNativeTextPacket] frame=%u shouldDraw=%d suppress=%d "
            "valid=%d prov=%d word2=%d rawTex=%d colorAuth=%d "
            "payload=%d audit=%d tpage=%u clut=%u srv=%d "
            "xy=%d,%d wh=%u,%u order=%u",
            ctx.frame,
            shouldDraw ? 1 : 0,
            suppressNative ? 1 : 0,
            command.valid ? 1 : 0,
            command.provenanceKnown ? 1 : 0,
            command.word2CommandKnown ? 1 : 0,
            command.rawTexture ? 1 : 0,
            command.colorAuthoritative ? 1 : 0,
            command.renderPayloadKnown ? 1 : 0,
            command.auditOnlyPartial ? 1 : 0,
            static_cast<unsigned>(command.tpage),
            static_cast<unsigned>(command.clut),
            srvReady ? 1 : 0,
            command.x,
            command.y,
            static_cast<unsigned>(command.w),
            static_cast<unsigned>(command.h),
            command.psxCallOrder);
    }
    if (!shouldDraw) {
        return;
    }
    SubmitFastSpritePacket8003FA20(ctx, command, viewport);
}

static bool ShouldDrawStage1GsSpritePacket8003F1B4(
    const PrStageSceneSubmitDirect::PsxGsSpritePacketCommand8003F1B4&
        command) {
    return command.valid && command.renderPayloadKnown &&
           command.priorityKnown &&
           (command.transformPath || (command.w != 0u && command.h != 0u));
}

static void SubmitFastGsSpritePacket8003F1B4(
    PrGameContext& ctx,
    const PrStageSceneSubmitDirect::PsxGsSpritePacketCommand8003F1B4&
        command,
    const Stage1Viewport& viewport) {
    if (!ctx.renderer) {
        return;
    }

    ID3D11ShaderResourceView* srv =
        s_stage1VramAtlas.GetTpageSRV(command.tpage,
                                      command.clut,
                                      ctx.renderer);
    if (srv == nullptr) {
        return;
    }

    D3D11Renderer::SpriteCmd cmd{};
    cmd.texture = srv;
    cmd.x = Stage1ScreenX(viewport, static_cast<float>(command.x[0]));
    cmd.y = Stage1ScreenY(viewport, static_cast<float>(command.y[0]));
    cmd.w = Stage1ScreenLength(viewport, static_cast<float>(command.w));
    cmd.h = Stage1ScreenLength(viewport, static_cast<float>(command.h));
    Stage1SpriteUvCenterEndpoints(command.u[0],
                                  command.v[0],
                                  command.w,
                                  command.h,
                                  cmd.u0,
                                  cmd.v0,
                                  cmd.u1,
                                  cmd.v1);
    ApplyStage1SpriteTextureReplacement(ctx,
                                        "gs_sprite_8003f1b4",
                                        command.tpage,
                                        command.clut,
                                        command.u[0],
                                        command.v[0],
                                        command.w,
                                        command.h,
                                        0u,
                                        cmd);
    cmd.r = static_cast<float>(command.r) / 255.0f;
    cmd.g = static_cast<float>(command.g) / 255.0f;
    cmd.b = static_cast<float>(command.b) / 255.0f;
    cmd.a = 1.0f;
    cmd.blend = D3D11Renderer::BlendMode::Alpha;
    cmd.layer = Stage1FastSpritePacketLayer8003FA20(command.priority);
    cmd.order = Stage1PacketOrder(0u, command.psxCallOrder);
    ctx.renderer->SubmitSprite(cmd);
}

static void SubmitTransformGsSpritePacket8003F1B4(
    PrGameContext& ctx,
    const PrStageSceneSubmitDirect::PsxGsSpritePacketCommand8003F1B4&
        command,
    const Stage1Viewport& viewport) {
    if (!ctx.renderer) {
        return;
    }

    ID3D11ShaderResourceView* srv =
        s_stage1VramAtlas.GetTpageSRV(command.tpage,
                                      command.clut,
                                      ctx.renderer);
    if (srv == nullptr) {
        return;
    }

    float u[4]{};
    float v[4]{};
    for (std::size_t i = 0; i < 4u; ++i) {
        PsxVramAtlas::UVtoNormalized(command.u[i], command.v[i], u[i], v[i]);
    }
    const float r = static_cast<float>(command.r) / 255.0f;
    const float g = static_cast<float>(command.g) / 255.0f;
    const float b = static_cast<float>(command.b) / 255.0f;
    const auto vertex = [&](std::size_t i) -> TexturedVertex {
        return TexturedVertex{
            Stage1ScreenX(viewport, static_cast<float>(command.x[i])),
            Stage1ScreenY(viewport, static_cast<float>(command.y[i])),
            u[i],
            v[i],
            r,
            g,
            b,
            1.0f};
    };
    TexturedVertex verts[6] = {
        vertex(0), vertex(1), vertex(2),
        vertex(1), vertex(3), vertex(2),
    };
    D3D11Renderer::TexturedTriCmd cmd{};
    cmd.texture = srv;
    for (std::size_t i = 0; i < 6u; ++i) {
        cmd.vertices[i] = verts[i];
    }
    cmd.vertexCount = 6;
    cmd.blend = D3D11Renderer::BlendMode::Alpha;
    cmd.layer = Stage1FastSpritePacketLayer8003FA20(command.priority);
    cmd.order = Stage1PacketOrder(0u, command.psxCallOrder);
    const uint8_t replacementU[6] = {
        command.u[0],
        command.u[1],
        command.u[2],
        command.u[1],
        command.u[3],
        command.u[2],
    };
    const uint8_t replacementV[6] = {
        command.v[0],
        command.v[1],
        command.v[2],
        command.v[1],
        command.v[3],
        command.v[2],
    };
    ApplyStage1TexturedTriTextureReplacement(ctx,
                                             "gs_sprite_transform_8003f1b4",
                                             command.tpage,
                                             command.clut,
                                             replacementU,
                                             replacementV,
                                             6,
                                             0x4753535052545246ULL,
                                             cmd);
    ctx.renderer->SubmitTexturedTriangles(cmd);
}

static void DrawStage1GsSpritePacket8003F1B4(
    PrGameContext& ctx,
    const PrStageSceneSubmitDirect::PsxGsSpritePacketCommand8003F1B4&
        command,
    const Stage1Viewport& viewport) {
    if (!ShouldDrawStage1GsSpritePacket8003F1B4(command)) {
        return;
    }
    if (command.transformPath) {
        SubmitTransformGsSpritePacket8003F1B4(ctx, command, viewport);
    } else {
        SubmitFastGsSpritePacket8003F1B4(ctx, command, viewport);
    }
}

static void DrawEventFrameFastSpritePacket8003FA20(
    PrGameContext& ctx,
    const PrStageSceneSubmitDirect::PsxFastSpritePacketCommand8003FA20&
        command,
    const Stage1Viewport& viewport) {
    if (!ShouldDrawEventFrameFastSpritePacket8003FA20(command)) {
        return;
    }
    SubmitFastSpritePacket8003FA20(ctx, command, viewport);
}

static void DrawStage1FastSpritePackets8003FA20(
    PrGameContext& ctx,
    const PrStageSceneSubmitDirect::Stage1SceneSubmitRuntime& runtime,
    const Stage1Viewport& viewport) {
    if (!ctx.renderer) {
        return;
    }

    EnsureStage1VramAtlas(ctx);
    Stage1TmdDrawContext428B0 drawCtx{&ctx, viewport};
    PrStageSceneSubmitDirect::BuildStage1FastSpritePacketCommands8003FA20(
        runtime,
        +[](const PrStageSceneSubmitDirect::PsxFastSpritePacketCommand8003FA20&
                command,
            void* userData) {
            auto* drawCtx = static_cast<Stage1TmdDrawContext428B0*>(userData);
            if (drawCtx == nullptr || drawCtx->ctx == nullptr) {
                return;
            }
            DrawStage1FastSpritePacket8003FA20(*drawCtx->ctx,
                                               command,
                                               drawCtx->viewport);
        },
        &drawCtx);
}

static PrPsxSpriteTemplateRender::PsxSpriteTemplate
ToSpriteTemplate80024744(
    const PrStageSceneSubmitDirect::PsxSpriteTemplate80024744& tpl) {
    return PrPsxSpriteTemplateRender::PsxSpriteTemplate{
        tpl.attr,
        tpl.texX_hw,
        tpl.texY_px,
        tpl.w,
        tpl.h,
        tpl.clutX_px,
        tpl.clutY_px,
    };
}

static PrStageSceneSubmitDirect::PsxSpriteTemplate80024744
MakeStage1Parappa2RailSpriteTemplate(uint16_t texX,
                                     uint16_t texY,
                                     uint16_t w,
                                     uint16_t h,
                                     uint16_t clutY) {
    PrStageSceneSubmitDirect::PsxSpriteTemplate80024744 out{};
    out.attr = 0x50000040u;
    out.texX_hw = texX;
    out.texY_px = texY;
    out.w = w;
    out.h = h;
    out.clutX_px = 0x0120u;
    out.clutY_px = clutY;
    return out;
}

static void DrawStage1CompactRailSprite80024744(
    PrGameContext& ctx,
    const PrStageSceneSubmitDirect::PsxCompactRailSpriteCommand80024744&
        command,
    const Stage1Viewport& viewport,
    uint32_t psxCallOrder,
    float r = 1.0f,
    float g = 1.0f,
    float b = 1.0f,
    float a = 1.0f) {
    if (!ctx.renderer || command.tpl.w == 0u || command.tpl.h == 0u) {
        return;
    }

    const PrPsxSpriteTemplateRender::PsxSpriteTemplate tpl =
        ToSpriteTemplate80024744(command.tpl);
    float x = command.x;
    float y = command.y;
    if (command.anchor ==
        PrStageSceneSubmitDirect::PsxCompactRailSpriteCommand80024744::
            Anchor::Center) {
        x -= static_cast<float>(command.tpl.w) * 0.5f;
        y -= static_cast<float>(command.tpl.h) * 0.5f;
    }

    (void)PrPsxSpriteTemplateRender::DrawPsxSpriteTemplateScaled(
        ctx,
        viewport.x,
        viewport.y,
        viewport.scale,
        x,
        y,
        tpl,
        command.scaleX,
        command.scaleY,
        r,
        g,
        b,
        a,
        Stage1FastSpritePacketLayer8003FA20(command.otBucket),
        static_cast<int>(psxCallOrder + 1u));
}

struct Stage1Parappa2RailStamp {
    uint8_t row = 0;
    uint8_t symbolType = 0;
    PrStageSceneSubmitDirect::PsxSpriteTemplate80024744 tpl{};
    float centerX = 0.0f;
    float centerY = 0.0f;
    float startFrame = 0.0f;
    uint32_t serial = 0;
};

struct Stage1Parappa2RailRowState {
    uint32_t noteSignature = 0;
    int16_t lastStudentRaw = -1;
    bool wasStudentActive = false;
};

struct Stage1Parappa2RailRuntime {
    uint32_t lastLogicFrame = UINT32_MAX;
    uint32_t nextSerial = 1;
    bool visualCorrectionValid = false;
    float visualCorrectionX = 0.0f;
    uint16_t lastDebugPadMask = 0u;
    uint16_t pendingDebugPressedMask = 0u;
    std::array<Stage1Parappa2RailRowState,
               PrStageSceneSubmitDirect::kCompactRailMaxRows80024744>
        rows{};
    std::vector<Stage1Parappa2RailStamp> stamps;
};

struct Stage1Parappa2RailRowFrame {
    bool studentActive = false;
    int16_t studentRaw = -1;
    float studentCenterX = 0.0f;
    float studentCenterY = 0.0f;
    uint32_t noteSignature = 2166136261u;
    std::array<bool, 9> noteTypePresent{};
    std::array<uint8_t, 9> noteSlots{};
    std::array<float, 9> noteCenterX{};
    std::array<PrStageSceneSubmitDirect::PsxSpriteTemplate80024744, 9>
        noteTemplates{};
};

static Stage1Parappa2RailRuntime s_stage1Parappa2RailRuntime;

static void ResetStage1Parappa2RailRuntime() {
    s_stage1Parappa2RailRuntime = Stage1Parappa2RailRuntime{};
}

static ID3D11ShaderResourceView* EnsureStage1Parappa2RailGlowTexture(
    PrGameContext& ctx) {
    if (ctx.renderer == nullptr) {
        return nullptr;
    }
    if (s_stage1Parappa2RailGlowTexture != nullptr &&
        s_stage1Parappa2RailGlowRenderer == ctx.renderer) {
        return s_stage1Parappa2RailGlowTexture;
    }

    DestroyStage1Parappa2RailGlowTexture();

    constexpr int kGlowSize = 64;
    constexpr float kGlowCenter = static_cast<float>(kGlowSize) * 0.5f;
    constexpr float kGlowRadius = static_cast<float>(kGlowSize) * 0.5f;
    std::vector<uint32_t> pixels(
        static_cast<size_t>(kGlowSize) * static_cast<size_t>(kGlowSize));
    for (int y = 0; y < kGlowSize; ++y) {
        for (int x = 0; x < kGlowSize; ++x) {
            const float dx =
                (static_cast<float>(x) + 0.5f - kGlowCenter) / kGlowRadius;
            const float dy =
                (static_cast<float>(y) + 0.5f - kGlowCenter) / kGlowRadius;
            const float dist = std::sqrt(dx * dx + dy * dy);
            const float edge = std::clamp(1.0f - dist, 0.0f, 1.0f);
            const float smooth = edge * edge * (3.0f - 2.0f * edge);
            const float alpha = std::pow(smooth, 1.35f);
            const auto a = static_cast<uint32_t>(
                std::clamp(alpha * 255.0f, 0.0f, 255.0f));
            pixels[static_cast<size_t>(y) * kGlowSize +
                   static_cast<size_t>(x)] =
                (a << 24) | 0x00FFFFFFu;
        }
    }

    s_stage1Parappa2RailGlowTexture =
        ctx.renderer->CreateTexture(pixels.data(), kGlowSize, kGlowSize);
    if (s_stage1Parappa2RailGlowTexture != nullptr) {
        s_stage1Parappa2RailGlowRenderer = ctx.renderer;
    }
    return s_stage1Parappa2RailGlowTexture;
}

static float Stage1Parappa2RailLeadPx(const PrGameContext& ctx) {
    return -ctx.stage1RailParappa2LeadSlots * 15.0f;
}

static float Stage1Parappa2RailRenderFrameFrac(const PrGameContext& ctx) {
    return Stage1RenderOnlyFramePhaseActive428B0(ctx)
               ? Stage1RenderOnlyBlendT428B0(ctx)
               : 0.0f;
}

static float Stage1Parappa2RailVisualTickOffset(float renderFrameFrac) {
    return std::clamp(renderFrameFrac, 0.0f, 1.0f) *
           static_cast<float>(kPrStage1TicksPerRenderFrame60 * 2);
}

static float Stage1Parappa2RailCompactPortraitDeltaX(float renderFrameFrac) {
    constexpr float kTicksPerJudgeSlot = 24.0f;
    constexpr float kRailSlotWidth = 15.0f;
    return Stage1Parappa2RailVisualTickOffset(renderFrameFrac) *
           (kRailSlotWidth / kTicksPerJudgeSlot);
}

struct Stage1Parappa2RailCursorAlign {
    bool coreAlignEnabled = false;
    bool coreValid = false;
    float coreCenterX = 0.0f;
    float renderDeltaX = 0.0f;
    bool correctionValid = false;
    float correctionX = 0.0f;
    float extraOffsetX = 0.0f;
};

static float Stage1Parappa2RailSlotCenterX(float slot) {
    constexpr float kRailFirstSlotCenterX = 32.0f;
    constexpr float kRailSlotWidth = 15.0f;
    return kRailFirstSlotCenterX + kRailSlotWidth * slot;
}

static float Stage1Parappa2RailCenterSlot(float centerX) {
    constexpr float kRailFirstSlotCenterX = 32.0f;
    constexpr float kRailSlotWidth = 15.0f;
    return (centerX - kRailFirstSlotCenterX) / kRailSlotWidth;
}

static float Stage1Parappa2RailNearestCycleDeltaX(float deltaX) {
    constexpr float kJudgeCycleWidth = 16.0f * 15.0f;
    constexpr float kHalfJudgeCycleWidth = kJudgeCycleWidth * 0.5f;
    while (deltaX > kHalfJudgeCycleWidth) {
        deltaX -= kJudgeCycleWidth;
    }
    while (deltaX < -kHalfJudgeCycleWidth) {
        deltaX += kJudgeCycleWidth;
    }
    return deltaX;
}

static float Stage1Parappa2RailPositiveMod384(int32_t tick96) {
    int32_t phase = tick96 % 384;
    if (phase < 0) {
        phase += 384;
    }
    return static_cast<float>(phase);
}

static float Stage1Parappa2RailPositiveMod384Float(float tick96) {
    float phase = std::fmod(tick96, 384.0f);
    if (phase < 0.0f) {
        phase += 384.0f;
    }
    return phase;
}

static uint8_t Stage1Parappa2RailCurrentSourceGroup() {
    const PrScn1::Stage1NumericRuntimeState& numeric =
        PrScn1::s_stage1NumericRuntime;
    const auto& probe = numeric.acceptedProducerBoundaryProbe;
    if (probe.selectorAvailable && probe.selectorByte1 != 0u) {
        return probe.selectorByte1;
    }

    const auto& current =
        numeric.gameplayRailCadenceProducer.currentCommitted;
    if (current.selectorAvailable && current.selectorByte1 != 0u) {
        return current.selectorByte1;
    }

    const auto& next = numeric.gameplayRailCadenceProducer.nextLookahead;
    if (next.selectorAvailable && next.selectorByte1 != 0u) {
        return next.selectorByte1;
    }

    return 0u;
}

static float Stage1Parappa2RailApplySourceGroupPhaseFold(float phase384) {
    const uint8_t sourceGroup = Stage1Parappa2RailCurrentSourceGroup();
    if (sourceGroup == 6u) {
        phase384 -= 8.0f * 24.0f;
    }
    while (phase384 < 0.0f) {
        phase384 += 384.0f;
    }
    while (phase384 >= 384.0f) {
        phase384 -= 384.0f;
    }
    return phase384;
}

static bool ResolveStage1Parappa2RailCorePhase384(
    uint32_t logicFrame,
    bool preferAcceptedTick,
    float& outPhase384) {
    const PrScn1::Stage1NumericRuntimeState& numeric =
        PrScn1::s_stage1NumericRuntime;
    if (!numeric.active) {
        return false;
    }

    const auto& carrier = numeric.acceptedProducerCarrier;
    const int32_t halfWindow34 =
        carrier.available ? static_cast<int32_t>(carrier.halfWindow34) : 0;
    if (preferAcceptedTick &&
        carrier.acceptedTick96Known &&
        carrier.acceptedTick96LastUpdateQueryFrame == logicFrame) {
        outPhase384 =
            Stage1Parappa2RailPositiveMod384(carrier.acceptedTick96 +
                                             halfWindow34);
        return true;
    }

    if (numeric.runnerTimecode801C7560.known) {
        outPhase384 = Stage1Parappa2RailPositiveMod384(
            numeric.runnerTimecode801C7560.state.tick801C364C +
            halfWindow34);
        return true;
    }

    if (carrier.acceptedTick96Known) {
        outPhase384 =
            Stage1Parappa2RailPositiveMod384(carrier.acceptedTick96 +
                                             halfWindow34);
        return true;
    }
    return false;
}

static bool ResolveStage1Parappa2RailCoreCursorCenterX(
    const PrGameContext& ctx,
    uint32_t logicFrame,
    bool preferAcceptedTick,
    float renderFrameFrac,
    float& outCenterX) {
    if (!ctx.stage1RailParappa2CoreAlign) {
        return false;
    }

    float phase384 = 0.0f;
    if (!ResolveStage1Parappa2RailCorePhase384(logicFrame,
                                               preferAcceptedTick,
                                               phase384)) {
        return false;
    }
    const float visualTickOffset =
        Stage1Parappa2RailVisualTickOffset(renderFrameFrac);
    phase384 = Stage1Parappa2RailPositiveMod384Float(
        phase384 + visualTickOffset);

    constexpr float kTicksPerJudgeSlot = 24.0f;
    const PrScn1::Stage1NumericRuntimeState& numeric =
        PrScn1::s_stage1NumericRuntime;
    const auto& carrier = numeric.acceptedProducerCarrier;
    const float halfWindow =
        carrier.available ? static_cast<float>(carrier.halfWindow34) : 0.0f;
    float centeredPhase = phase384 - halfWindow;
    if (centeredPhase < 0.0f) {
        centeredPhase += 384.0f;
    }
    centeredPhase =
        Stage1Parappa2RailApplySourceGroupPhaseFold(centeredPhase);
    const float slot = centeredPhase / kTicksPerJudgeSlot;
    outCenterX = Stage1Parappa2RailSlotCenterX(slot);
    return true;
}

static Stage1Parappa2RailCursorAlign ResolveStage1Parappa2RailCursorAlign(
    const PrGameContext& ctx,
    uint32_t logicFrame,
    bool preferAcceptedTick,
    float renderFrameFrac) {
    Stage1Parappa2RailCursorAlign align{};
    align.coreAlignEnabled = ctx.stage1RailParappa2CoreAlign;
    align.extraOffsetX = Stage1Parappa2RailLeadPx(ctx);
    align.renderDeltaX = Stage1Parappa2RailCompactPortraitDeltaX(
        renderFrameFrac);
    align.coreValid = ResolveStage1Parappa2RailCoreCursorCenterX(
        ctx,
        logicFrame,
        preferAcceptedTick,
        renderFrameFrac,
        align.coreCenterX);
    if (align.coreValid && renderFrameFrac > 0.0f) {
        float logicCenterX = 0.0f;
        if (ResolveStage1Parappa2RailCoreCursorCenterX(ctx,
                                                       logicFrame,
                                                       preferAcceptedTick,
                                                       0.0f,
                                                       logicCenterX)) {
            align.renderDeltaX = Stage1Parappa2RailNearestCycleDeltaX(
                align.coreCenterX - logicCenterX);
        }
    }
    return align;
}

static bool ResolveStage1Parappa2RailConvertedPortraitCenterX(
    const PrStageSceneSubmitDirect::PsxCompactRailSpriteCommand80024744&
        command,
    const Stage1Parappa2RailCursorAlign& align,
    float& outCenterX) {
    if (command.raw < 0) {
        return false;
    }

    const float baseCenterX =
        command.x + static_cast<float>(command.tpl.w) * 0.5f;
    const float compactSlot =
        static_cast<float>(command.raw);
    const float compactSlotCenterX =
        Stage1Parappa2RailSlotCenterX(compactSlot);
    const float portraitHoldResidualX = baseCenterX - compactSlotCenterX;

    // The core-align experiment deliberately remaps the visual baseline to the
    // scorer cursor. With core-align off, only the movable portraits receive
    // render-only half-frame motion; the static rail body stays on logic frames.
    const float alignedSlot =
        align.coreAlignEnabled ? compactSlot - 1.0f : compactSlot;
    const float judgementSlotCenterX =
        Stage1Parappa2RailSlotCenterX(alignedSlot);
    const float correctionX =
        align.coreAlignEnabled
            ? (align.correctionValid ? align.correctionX
                                     : align.renderDeltaX)
            : align.renderDeltaX;
    outCenterX =
        judgementSlotCenterX + portraitHoldResidualX + correctionX +
        align.extraOffsetX;
    return true;
}

static void ResolveStage1Parappa2RailVisualCorrection(
    const std::vector<
        PrStageSceneSubmitDirect::PsxCompactRailSpriteCommand80024744>&
        commands,
    Stage1Parappa2RailCursorAlign& align,
    bool updateRuntime) {
    if (!align.coreAlignEnabled) {
        align.correctionValid = false;
        align.correctionX = 0.0f;
        return;
    }

    if (!align.coreValid) {
        if (s_stage1Parappa2RailRuntime.visualCorrectionValid) {
            align.correctionValid = true;
            align.correctionX =
                s_stage1Parappa2RailRuntime.visualCorrectionX +
                align.renderDeltaX;
        }
        return;
    }

    if (!updateRuntime) {
        if (s_stage1Parappa2RailRuntime.visualCorrectionValid) {
            align.correctionValid = true;
            align.correctionX =
                s_stage1Parappa2RailRuntime.visualCorrectionX +
                align.renderDeltaX;
        }
        return;
    }

    using Command =
        PrStageSceneSubmitDirect::PsxCompactRailSpriteCommand80024744;

    auto findCandidate = [&](bool teacherOnly,
                             float& outDelta) {
        float bestAbsDelta = 1000000.0f;
        float bestDelta = 0.0f;
        bool found = false;
        Stage1Parappa2RailCursorAlign baseAlign{};
        baseAlign.coreAlignEnabled = align.coreAlignEnabled;
        baseAlign.extraOffsetX = align.extraOffsetX;

        for (const auto& command : commands) {
            if (teacherOnly) {
                if (command.kind != Command::Kind::TeacherPortrait) {
                    continue;
                }
            } else if (command.kind != Command::Kind::TeacherPortrait &&
                       command.kind != Command::Kind::StudentPortrait) {
                continue;
            }

            float baseCenterX = 0.0f;
            if (!ResolveStage1Parappa2RailConvertedPortraitCenterX(
                    command,
                    baseAlign,
                    baseCenterX)) {
                continue;
            }

            const float delta = Stage1Parappa2RailNearestCycleDeltaX(
                align.coreCenterX - baseCenterX);
            const float absDelta = std::fabs(delta);
            if (!found || absDelta < bestAbsDelta) {
                found = true;
                bestAbsDelta = absDelta;
                bestDelta = delta;
            }
        }

        if (found) {
            outDelta = bestDelta;
        }
        return found;
    };

    float candidate = 0.0f;
    bool found = findCandidate(true, candidate);
    if (!found && !s_stage1Parappa2RailRuntime.visualCorrectionValid) {
        found = findCandidate(false, candidate);
    }

    constexpr float kMaxCorrectionX = 45.0f;
    constexpr float kMaxFrameStepX = 18.0f;
    constexpr float kCorrectionLerp = 0.35f;

    if (found) {
        candidate = std::clamp(candidate, -kMaxCorrectionX, kMaxCorrectionX);
        if (s_stage1Parappa2RailRuntime.visualCorrectionValid) {
            const float delta = Stage1Parappa2RailNearestCycleDeltaX(
                candidate - s_stage1Parappa2RailRuntime.visualCorrectionX);
            if (std::fabs(delta) <= kMaxFrameStepX) {
                s_stage1Parappa2RailRuntime.visualCorrectionX +=
                    delta * kCorrectionLerp;
            }
        } else {
            s_stage1Parappa2RailRuntime.visualCorrectionX = candidate;
            s_stage1Parappa2RailRuntime.visualCorrectionValid = true;
        }
    }

    if (s_stage1Parappa2RailRuntime.visualCorrectionValid) {
        align.correctionValid = true;
        align.correctionX =
            s_stage1Parappa2RailRuntime.visualCorrectionX +
            align.renderDeltaX;
    }
}

static void ApplyStage1Parappa2RailPortraitAlign(
    std::vector<
        PrStageSceneSubmitDirect::PsxCompactRailSpriteCommand80024744>&
        commands,
    const Stage1Parappa2RailCursorAlign& align) {
    using Kind =
        PrStageSceneSubmitDirect::PsxCompactRailSpriteCommand80024744::Kind;
    for (auto& command : commands) {
        if (command.kind == Kind::TeacherPortrait ||
            command.kind == Kind::StudentPortrait) {
            const float baseCenterX =
                command.x + static_cast<float>(command.tpl.w) * 0.5f;
            float centerX = baseCenterX;
            if (!ResolveStage1Parappa2RailConvertedPortraitCenterX(
                    command,
                    align,
                    centerX)) {
                continue;
            }
            command.x += centerX - baseCenterX;
        }
    }
}

static void ClearStage1Parappa2RailRowStamps(uint8_t row) {
    auto& stamps = s_stage1Parappa2RailRuntime.stamps;
    stamps.erase(
        std::remove_if(
            stamps.begin(),
            stamps.end(),
            [row](const Stage1Parappa2RailStamp& stamp) {
                return stamp.row == row;
            }),
        stamps.end());
}

static bool Stage1Parappa2RailSymbolMatchesNote(uint8_t symbolType,
                                                uint8_t noteType) {
    if (symbolType == noteType) {
        return true;
    }
    return (symbolType == 5u && noteType == 6u) ||
           (symbolType == 7u && noteType == 8u);
}

static bool ResolveStage1Parappa2RailSymbolTemplate(
    uint8_t symbolType,
    PrStageSceneSubmitDirect::PsxSpriteTemplate80024744& outTpl) {
    switch (symbolType) {
    case 1:
        outTpl = MakeStage1Parappa2RailSpriteTemplate(
            0x03F8u, 0x01DDu, 16u, 16u, 0x01ECu);
        return true;
    case 2:
        outTpl = MakeStage1Parappa2RailSpriteTemplate(
            0x03F8u, 0x01CDu, 16u, 16u, 0x01E7u);
        return true;
    case 3:
        outTpl = MakeStage1Parappa2RailSpriteTemplate(
            0x03FCu, 0x01CDu, 16u, 16u, 0x01E8u);
        return true;
    case 4:
        outTpl = MakeStage1Parappa2RailSpriteTemplate(
            0x03FCu, 0x01DDu, 16u, 16u, 0x01EDu);
        return true;
    case 5:
    case 6:
        outTpl = MakeStage1Parappa2RailSpriteTemplate(
            0x03F4u, 0x01CDu, 16u, 16u, 0x01E6u);
        return true;
    case 7:
    case 8:
        outTpl = MakeStage1Parappa2RailSpriteTemplate(
            0x03F4u, 0x01DDu, 16u, 16u, 0x01E9u);
        return true;
    default:
        break;
    }
    return false;
}

static bool ResolveStage1Parappa2RailStampTemplate(
    const Stage1Parappa2RailRowFrame& row,
    uint8_t symbolType,
    PrStageSceneSubmitDirect::PsxSpriteTemplate80024744& outTpl,
    uint8_t& outNoteSlot,
    float& outNoteCenterX) {
    if (symbolType < row.noteTypePresent.size() &&
        row.noteTypePresent[symbolType]) {
        outTpl = row.noteTemplates[symbolType];
        outNoteSlot = row.noteSlots[symbolType];
        outNoteCenterX = row.noteCenterX[symbolType];
        return true;
    }
    for (uint8_t noteType = 1u; noteType < row.noteTypePresent.size();
         ++noteType) {
        if (row.noteTypePresent[noteType] &&
            Stage1Parappa2RailSymbolMatchesNote(symbolType, noteType)) {
            outTpl = row.noteTemplates[noteType];
            outNoteSlot = row.noteSlots[noteType];
            outNoteCenterX = row.noteCenterX[noteType];
            return true;
        }
    }
    if (ResolveStage1Parappa2RailSymbolTemplate(symbolType, outTpl)) {
        outNoteSlot = UINT8_MAX;
        outNoteCenterX = row.studentCenterX;
        return true;
    }
    return false;
}

static void AppendStage1Parappa2RailSymbolType(std::vector<uint8_t>& out,
                                               uint8_t symbolType) {
    if (symbolType == 0u ||
        std::find(out.begin(), out.end(), symbolType) != out.end()) {
        return;
    }
    out.push_back(symbolType);
}

static void AppendStage1Parappa2RailSymbolTypesFromMask(
    uint16_t mask,
    std::vector<uint8_t>& out) {
    if ((mask & (uint16_t)PrPadButton::Triangle) != 0u) {
        AppendStage1Parappa2RailSymbolType(out, 1u);
    }
    if ((mask & (uint16_t)PrPadButton::Circle) != 0u) {
        AppendStage1Parappa2RailSymbolType(out, 2u);
    }
    if ((mask & (uint16_t)PrPadButton::Cross) != 0u) {
        AppendStage1Parappa2RailSymbolType(out, 3u);
    }
    if ((mask & (uint16_t)PrPadButton::Square) != 0u) {
        AppendStage1Parappa2RailSymbolType(out, 4u);
    }
    if ((mask & (uint16_t)PrPadButton::Left) != 0u) {
        AppendStage1Parappa2RailSymbolType(out, 5u);
    }
    if ((mask & (uint16_t)PrPadButton::Right) != 0u) {
        AppendStage1Parappa2RailSymbolType(out, 7u);
    }
}

static uint16_t NormalizeStage1Parappa2RailLocalTriggerMask(uint16_t mask) {
    uint16_t out = 0u;
    constexpr uint16_t kFaceMask =
        (uint16_t)PrPadButton::Triangle |
        (uint16_t)PrPadButton::Circle |
        (uint16_t)PrPadButton::Cross |
        (uint16_t)PrPadButton::Square;
    out = static_cast<uint16_t>(mask & kFaceMask);
    if ((mask & ((uint16_t)PrPadButton::Left |
                 (uint16_t)PrPadButton::L1)) != 0u) {
        out = static_cast<uint16_t>(out | (uint16_t)PrPadButton::Left);
    }
    if ((mask & ((uint16_t)PrPadButton::Right |
                 (uint16_t)PrPadButton::R1)) != 0u) {
        out = static_cast<uint16_t>(out | (uint16_t)PrPadButton::Right);
    }
    return out;
}

static uint16_t NormalizeStage1Parappa2RailDebugTriggerMask(uint16_t mask) {
    uint16_t out = 0u;
    constexpr uint16_t kFaceMask =
        (uint16_t)PrPadButton::Triangle |
        (uint16_t)PrPadButton::Circle |
        (uint16_t)PrPadButton::Cross |
        (uint16_t)PrPadButton::Square;
    out = static_cast<uint16_t>(mask & kFaceMask);
    constexpr uint16_t kPsxL1Mask = 0x0004u;
    constexpr uint16_t kPsxR1Mask = 0x0008u;
    if ((mask & kPsxL1Mask) != 0u) {
        out = static_cast<uint16_t>(out | (uint16_t)PrPadButton::Left);
    }
    if ((mask & kPsxR1Mask) != 0u) {
        out = static_cast<uint16_t>(out | (uint16_t)PrPadButton::Right);
    }
    return out;
}

struct Stage1Parappa2RailInputSnapshot {
    uint16_t triggerMask = 0;
    uint16_t padPressedMask = 0;
    uint16_t debugPadMask = 0;
    uint16_t debugPressedMask = 0;
    uint16_t postAcceptedMask = 0;
    bool postAcceptedChanged = false;
    bool postAcceptedGateOpen = false;
};

static Stage1Parappa2RailInputSnapshot
ResolveStage1Parappa2RailInputSnapshot(const PrGameContext& ctx) {
    constexpr uint16_t kGameplayMask =
        (uint16_t)PrPadButton::Triangle |
        (uint16_t)PrPadButton::Circle |
        (uint16_t)PrPadButton::Cross |
        (uint16_t)PrPadButton::Square |
        (uint16_t)PrPadButton::Left |
        (uint16_t)PrPadButton::Right;
    constexpr uint16_t kLocalGameplayMask =
        kGameplayMask |
        (uint16_t)PrPadButton::L1 |
        (uint16_t)PrPadButton::R1;

    Stage1Parappa2RailInputSnapshot snapshot{};
    const PrScn1::Stage1NumericRuntimeState& numeric =
        PrScn1::s_stage1NumericRuntime;
    if (numeric.active && numeric.runnerPostFrame7A60.acceptedMaskChanged) {
        snapshot.postAcceptedMask = static_cast<uint16_t>(
            numeric.runnerPostFrame7A60.acceptedMask9FF & kGameplayMask);
        snapshot.postAcceptedChanged = true;
        snapshot.postAcceptedGateOpen =
            numeric.runnerPostFrame7A60.acceptedGateOpen;
    }

    const PrPadState pad = PrPad::GetState(0);
    snapshot.padPressedMask =
        static_cast<uint16_t>(pad.pressed & kLocalGameplayMask);
    snapshot.debugPadMask = ctx.debugPadInput;
    snapshot.debugPressedMask =
        s_stage1Parappa2RailRuntime.pendingDebugPressedMask;
    s_stage1Parappa2RailRuntime.pendingDebugPressedMask = 0u;
    snapshot.triggerMask =
        NormalizeStage1Parappa2RailLocalTriggerMask(snapshot.padPressedMask);
    snapshot.triggerMask = static_cast<uint16_t>(
        snapshot.triggerMask |
        NormalizeStage1Parappa2RailDebugTriggerMask(
            snapshot.debugPressedMask));
    if (snapshot.postAcceptedChanged && snapshot.postAcceptedMask != 0u) {
        snapshot.triggerMask = static_cast<uint16_t>(
            snapshot.triggerMask |
            NormalizeStage1Parappa2RailLocalTriggerMask(
                snapshot.postAcceptedMask));
    }
    return snapshot;
}

static void CaptureStage1Parappa2RailDebugPadEdge(const PrGameContext& ctx) {
    Stage1Parappa2RailRuntime& runtime = s_stage1Parappa2RailRuntime;
    const uint16_t current = ctx.debugPadInput;
    const uint16_t pressed =
        static_cast<uint16_t>(current & ~runtime.lastDebugPadMask);
    runtime.lastDebugPadMask = current;
    runtime.pendingDebugPressedMask =
        static_cast<uint16_t>(runtime.pendingDebugPressedMask | pressed);
}

static uint32_t ResolveStage1Parappa2RailLogicFrame(const PrGameContext& ctx) {
    const PrScn1::Stage1NumericRuntimeState& numeric =
        PrScn1::s_stage1NumericRuntime;
    return numeric.active ? numeric.queryFrame : ctx.frame;
}

static float ResolveStage1Parappa2RailDisplayFrame(
    uint32_t logicFrame,
    float renderFrameFrac) {
    return static_cast<float>(logicFrame) +
           std::clamp(renderFrameFrac, 0.0f, 1.0f);
}

static void TraceStage1Parappa2RailAlign(
    const PrGameContext& ctx,
    uint32_t logicFrame,
    const Stage1Parappa2RailInputSnapshot& inputSnapshot,
    const Stage1Parappa2RailRowFrame& row,
    uint8_t rowIndex,
    uint8_t symbolType,
    uint8_t noteSlot,
    float noteCenterX,
    float stampCenterX) {
    if (!ctx.stage1RailParappa2TraceAlign) {
        return;
    }

    const PrScn1::Stage1NumericRuntimeState& numeric =
        PrScn1::s_stage1NumericRuntime;
    const auto& carrier = numeric.acceptedProducerCarrier;
    const auto& probe = numeric.acceptedProducerBoundaryProbe;
    const auto& lastWrite = numeric.acceptedProducerLastRecordedPageWrite;
    const auto& scorer = numeric.scorerPort;
    const bool tickKnown =
        numeric.active && numeric.runnerTimecode801C7560.known;
    const int32_t tick96 =
        tickKnown ? numeric.runnerTimecode801C7560.state.tick801C364C : 0;
    const bool acceptedSameFrame =
        carrier.acceptedTick96Known &&
        carrier.acceptedTick96LastUpdateQueryFrame == logicFrame;
    const float visualSlot = Stage1Parappa2RailCenterSlot(stampCenterX);
    const float noteVisualSlot = Stage1Parappa2RailCenterSlot(noteCenterX);
    const float noteDeltaSlot = visualSlot - noteVisualSlot;
    const float corePhaseSlot =
        static_cast<float>(carrier.phase384) / 24.0f;
    const uint8_t sourceGroup = Stage1Parappa2RailCurrentSourceGroup();
    const float railPhaseSlot =
        Stage1Parappa2RailApplySourceGroupPhaseFold(
            static_cast<float>(carrier.phase384)) /
        24.0f;
    const float deltaRecordSlot =
        visualSlot - static_cast<float>(carrier.recordSlot24);
    const float deltaPhaseSlot = visualSlot - corePhaseSlot;
    const float deltaRailPhaseSlot = visualSlot - railPhaseSlot;

    Log::Printf(
        "[P2RailAlign] q=%u row=%u raw=%d sym=%u trigger=0x%04X "
        "padPressed=0x%04X debugPad=0x%04X debugPressed=0x%04X "
        "postAccepted=0x%04X postChanged=%d postGate=%d "
        "stampX=%.2f stampSlot=%.3f noteSlot=%u noteX=%.2f "
        "noteVisualSlot=%.3f tickKnown=%d tick96=%d acceptedSameFrame=%d "
        "acceptedTickKnown=%d acceptedTick96=%d phase384=%u phaseSlot=%.3f "
        "railPhaseSlot=%.3f sourceGroup=%u recordSlot24=%u rem24=%u "
        "halfWindow34=%u recordedSplit=%d "
        "noteDeltaSlot=%.3f deltaRecordSlot=%.3f deltaPhaseSlot=%.3f "
        "deltaRailPhaseSlot=%.3f "
        "probePressed=0x%04X probeAccepted=0x%04X probeClass20=%u "
        "selector=%u/%u timingTemplateState=%u sourceCellGate=%d "
        "materialized=%d carryReplayed=%d split=%u sourceCellCursor=%u "
        "lastWrite=%d lastWritePage38=%d lastWriteSlot24=%u "
        "lastWriteMask=0x%08X lastWriteComp=%u scorerCount=%u "
        "scorerRecorded=%u scorerPenalty=%u scorerMask=0x%08X",
        logicFrame,
        static_cast<unsigned>(rowIndex),
        static_cast<int>(row.studentRaw),
        static_cast<unsigned>(symbolType),
        static_cast<unsigned>(inputSnapshot.triggerMask),
        static_cast<unsigned>(inputSnapshot.padPressedMask),
        static_cast<unsigned>(inputSnapshot.debugPadMask),
        static_cast<unsigned>(inputSnapshot.debugPressedMask),
        static_cast<unsigned>(inputSnapshot.postAcceptedMask),
        inputSnapshot.postAcceptedChanged ? 1 : 0,
        inputSnapshot.postAcceptedGateOpen ? 1 : 0,
        stampCenterX,
        visualSlot,
        static_cast<unsigned>(noteSlot),
        noteCenterX,
        noteVisualSlot,
        tickKnown ? 1 : 0,
        tick96,
        acceptedSameFrame ? 1 : 0,
        carrier.acceptedTick96Known ? 1 : 0,
        carrier.acceptedTick96,
        static_cast<unsigned>(carrier.phase384),
        corePhaseSlot,
        railPhaseSlot,
        static_cast<unsigned>(sourceGroup),
        static_cast<unsigned>(carrier.recordSlot24),
        static_cast<unsigned>(carrier.recordRemainder24),
        static_cast<unsigned>(carrier.halfWindow34),
        carrier.recordedSplit ? 1 : 0,
        noteDeltaSlot,
        deltaRecordSlot,
        deltaPhaseSlot,
        deltaRailPhaseSlot,
        static_cast<unsigned>(probe.rawPressedMask),
        static_cast<unsigned>(probe.rawAcceptedMask),
        static_cast<unsigned>(probe.classToken),
        static_cast<unsigned>(probe.selectorByte0),
        static_cast<unsigned>(probe.selectorByte1),
        static_cast<unsigned>(probe.timingTemplateState),
        probe.sourceCellGateActive ? 1 : 0,
        probe.materialized ? 1 : 0,
        probe.carryReplayed ? 1 : 0,
        static_cast<unsigned>(probe.split),
        static_cast<unsigned>(probe.sourceCellCursor),
        lastWrite.available ? 1 : 0,
        lastWrite.writePageOrdinal38,
        static_cast<unsigned>(lastWrite.recordSlot24),
        static_cast<unsigned>(lastWrite.acceptedMask),
        static_cast<unsigned>(lastWrite.pageCompanion),
        static_cast<unsigned>(scorer.acceptedCount91810),
        static_cast<unsigned>(scorer.recordedHitCount91812),
        static_cast<unsigned>(scorer.penaltySideCount91814),
        static_cast<unsigned>(scorer.aggregateAcceptedMask91808));
}

static void TraceStage1Parappa2RailInput(
    const PrGameContext& ctx,
    uint32_t logicFrame,
    const Stage1Parappa2RailInputSnapshot& inputSnapshot) {
    if (!ctx.stage1RailParappa2TraceAlign ||
        inputSnapshot.triggerMask == 0u) {
        return;
    }

    const PrScn1::Stage1NumericRuntimeState& numeric =
        PrScn1::s_stage1NumericRuntime;
    const auto& carrier = numeric.acceptedProducerCarrier;
    const auto& probe = numeric.acceptedProducerBoundaryProbe;
    const auto& lastWrite = numeric.acceptedProducerLastRecordedPageWrite;
    const auto& scorer = numeric.scorerPort;
    const bool tickKnown =
        numeric.active && numeric.runnerTimecode801C7560.known;
    const int32_t tick96 =
        tickKnown ? numeric.runnerTimecode801C7560.state.tick801C364C : 0;
    const bool acceptedSameFrame =
        carrier.acceptedTick96Known &&
        carrier.acceptedTick96LastUpdateQueryFrame == logicFrame;
    const float corePhaseSlot =
        static_cast<float>(carrier.phase384) / 24.0f;

    Log::Printf(
        "[P2RailInput] q=%u trigger=0x%04X padPressed=0x%04X "
        "debugPad=0x%04X debugPressed=0x%04X postAccepted=0x%04X "
        "postChanged=%d postGate=%d tickKnown=%d "
        "tick96=%d acceptedSameFrame=%d acceptedTickKnown=%d "
        "acceptedTick96=%d phase384=%u phaseSlot=%.3f recordSlot24=%u "
        "rem24=%u halfWindow34=%u recordedSplit=%d probePressed=0x%04X "
        "probeAccepted=0x%04X probeClass20=%u selector=%u/%u "
        "timingTemplateState=%u sourceCellGate=%d materialized=%d "
        "carryReplayed=%d split=%u sourceCellCursor=%u lastWrite=%d "
        "lastWritePage38=%d lastWriteSlot24=%u lastWriteMask=0x%08X "
        "lastWriteComp=%u scorerCount=%u scorerRecorded=%u "
        "scorerPenalty=%u scorerMask=0x%08X",
        logicFrame,
        static_cast<unsigned>(inputSnapshot.triggerMask),
        static_cast<unsigned>(inputSnapshot.padPressedMask),
        static_cast<unsigned>(inputSnapshot.debugPadMask),
        static_cast<unsigned>(inputSnapshot.debugPressedMask),
        static_cast<unsigned>(inputSnapshot.postAcceptedMask),
        inputSnapshot.postAcceptedChanged ? 1 : 0,
        inputSnapshot.postAcceptedGateOpen ? 1 : 0,
        tickKnown ? 1 : 0,
        tick96,
        acceptedSameFrame ? 1 : 0,
        carrier.acceptedTick96Known ? 1 : 0,
        carrier.acceptedTick96,
        static_cast<unsigned>(carrier.phase384),
        corePhaseSlot,
        static_cast<unsigned>(carrier.recordSlot24),
        static_cast<unsigned>(carrier.recordRemainder24),
        static_cast<unsigned>(carrier.halfWindow34),
        carrier.recordedSplit ? 1 : 0,
        static_cast<unsigned>(probe.rawPressedMask),
        static_cast<unsigned>(probe.rawAcceptedMask),
        static_cast<unsigned>(probe.classToken),
        static_cast<unsigned>(probe.selectorByte0),
        static_cast<unsigned>(probe.selectorByte1),
        static_cast<unsigned>(probe.timingTemplateState),
        probe.sourceCellGateActive ? 1 : 0,
        probe.materialized ? 1 : 0,
        probe.carryReplayed ? 1 : 0,
        static_cast<unsigned>(probe.split),
        static_cast<unsigned>(probe.sourceCellCursor),
        lastWrite.available ? 1 : 0,
        lastWrite.writePageOrdinal38,
        static_cast<unsigned>(lastWrite.recordSlot24),
        static_cast<unsigned>(lastWrite.acceptedMask),
        static_cast<unsigned>(lastWrite.pageCompanion),
        static_cast<unsigned>(scorer.acceptedCount91810),
        static_cast<unsigned>(scorer.recordedHitCount91812),
        static_cast<unsigned>(scorer.penaltySideCount91814),
        static_cast<unsigned>(scorer.aggregateAcceptedMask91808));
}

static void BuildStage1Parappa2RailRows(
    const std::vector<
        PrStageSceneSubmitDirect::PsxCompactRailSpriteCommand80024744>&
        commands,
    std::array<Stage1Parappa2RailRowFrame,
               PrStageSceneSubmitDirect::kCompactRailMaxRows80024744>& rows) {
    for (const auto& command : commands) {
        if (command.row >= rows.size()) {
            continue;
        }
        Stage1Parappa2RailRowFrame& row = rows[command.row];
        using Kind =
            PrStageSceneSubmitDirect::PsxCompactRailSpriteCommand80024744::Kind;
        if (command.kind == Kind::StudentPortrait && command.raw >= 0) {
            row.studentActive = true;
            row.studentRaw = command.raw;
            row.studentCenterX =
                command.x + static_cast<float>(command.tpl.w) * 0.5f;
            row.studentCenterY =
                command.y + static_cast<float>(command.tpl.h) * 0.5f;
            continue;
        }
        if (command.kind != Kind::Note || command.raw < 1 ||
            command.raw > 8) {
            continue;
        }

        const uint8_t noteType = static_cast<uint8_t>(command.raw);
        row.noteTypePresent[noteType] = true;
        row.noteSlots[noteType] = command.slot;
        row.noteCenterX[noteType] = command.x;
        row.noteTemplates[noteType] = command.tpl;
        row.noteSignature ^= static_cast<uint32_t>(noteType) +
                             (static_cast<uint32_t>(command.slot) << 8);
        row.noteSignature *= 16777619u;
    }
}

static void UpdateStage1Parappa2RailRowLatches(
    const std::array<Stage1Parappa2RailRowFrame,
                     PrStageSceneSubmitDirect::kCompactRailMaxRows80024744>&
        rows) {
    for (uint8_t i = 0; i < rows.size(); ++i) {
        const Stage1Parappa2RailRowFrame& row = rows[i];
        Stage1Parappa2RailRowState& latch =
            s_stage1Parappa2RailRuntime.rows[i];
        const bool restartedStudent =
            row.studentActive &&
            (!latch.wasStudentActive ||
             (row.studentRaw >= 0 && latch.lastStudentRaw >= 0 &&
              row.studentRaw < latch.lastStudentRaw));
        const bool noteStreamChanged =
            row.noteSignature != latch.noteSignature;
        if (!row.studentActive || restartedStudent || noteStreamChanged) {
            ClearStage1Parappa2RailRowStamps(i);
        }

        latch.noteSignature = row.noteSignature;
        latch.lastStudentRaw = row.studentRaw;
        latch.wasStudentActive = row.studentActive;
    }
}

static void AddStage1Parappa2RailStampsForInput(
    PrGameContext& ctx,
    const std::array<Stage1Parappa2RailRowFrame,
                     PrStageSceneSubmitDirect::kCompactRailMaxRows80024744>&
        rows,
    const std::vector<uint8_t>& symbolTypes,
    uint32_t logicFrame,
    const Stage1Parappa2RailInputSnapshot& inputSnapshot) {
    if (symbolTypes.empty()) {
        return;
    }

    for (uint8_t rowIndex = 0; rowIndex < rows.size(); ++rowIndex) {
        const Stage1Parappa2RailRowFrame& row = rows[rowIndex];
        if (!row.studentActive) {
            continue;
        }
        for (uint8_t symbolType : symbolTypes) {
            PrStageSceneSubmitDirect::PsxSpriteTemplate80024744 tpl{};
            uint8_t noteSlot = 0u;
            float noteCenterX = 0.0f;
            if (!ResolveStage1Parappa2RailStampTemplate(row,
                                                        symbolType,
                                                        tpl,
                                                        noteSlot,
                                                        noteCenterX)) {
                continue;
            }

            Stage1Parappa2RailStamp stamp{};
            stamp.row = rowIndex;
            stamp.symbolType = symbolType;
            stamp.tpl = tpl;
            stamp.centerX = row.studentCenterX;
            stamp.centerY = row.studentCenterY;
            stamp.startFrame = static_cast<float>(logicFrame);
            stamp.serial = s_stage1Parappa2RailRuntime.nextSerial++;
            TraceStage1Parappa2RailAlign(ctx,
                                         logicFrame,
                                         inputSnapshot,
                                         row,
                                         rowIndex,
                                         symbolType,
                                         noteSlot,
                                         noteCenterX,
                                         stamp.centerX);
            s_stage1Parappa2RailRuntime.stamps.push_back(stamp);
        }
    }
}

static void TickStage1Parappa2RailRuntime(
    PrGameContext& ctx,
    const std::array<Stage1Parappa2RailRowFrame,
                     PrStageSceneSubmitDirect::kCompactRailMaxRows80024744>&
        rows,
    uint32_t logicFrame) {
    if (!ctx.stageRunning || ctx.currentScene != PrSceneId::Scene1) {
        ResetStage1Parappa2RailRuntime();
        return;
    }

    CaptureStage1Parappa2RailDebugPadEdge(ctx);
    if (logicFrame == s_stage1Parappa2RailRuntime.lastLogicFrame) {
        return;
    }
    s_stage1Parappa2RailRuntime.lastLogicFrame = logicFrame;

    UpdateStage1Parappa2RailRowLatches(rows);

    std::vector<uint8_t> symbolTypes;
    const Stage1Parappa2RailInputSnapshot inputSnapshot =
        ResolveStage1Parappa2RailInputSnapshot(ctx);
    AppendStage1Parappa2RailSymbolTypesFromMask(inputSnapshot.triggerMask,
                                                symbolTypes);
    TraceStage1Parappa2RailInput(ctx, logicFrame, inputSnapshot);
    AddStage1Parappa2RailStampsForInput(ctx,
                                        rows,
                                        symbolTypes,
                                        logicFrame,
                                        inputSnapshot);
}

static void DrawStage1Parappa2RailGlow(
    PrGameContext& ctx,
    const Stage1Viewport& viewport,
    const PrStageSceneSubmitDirect::PsxCompactRailSpriteCommand80024744&
        command,
    float centerX,
    float centerY,
    float flipScaleY,
    float glowT,
    uint32_t& psxCallOrder) {
    ID3D11ShaderResourceView* texture =
        EnsureStage1Parappa2RailGlowTexture(ctx);
    if (texture == nullptr || glowT <= 0.0f) {
        return;
    }

    const float basePsx =
        static_cast<float>((std::max)(command.tpl.w, command.tpl.h));
    if (basePsx <= 0.0f) {
        return;
    }

    const float glowScale = (std::max)(1.0f,
                                       ctx.stage1RailParappa2GlowScale);
    const float pulse = 1.0f + (std::max)(0.0f, flipScaleY - 1.0f) * 0.20f;
    const float alpha =
        std::clamp(ctx.stage1RailParappa2GlowAlpha * glowT, 0.0f, 1.0f);
    const int layer = Stage1FastSpritePacketLayer8003FA20(command.otBucket);

    auto submitGlow = [&](float sizePsx,
                          float r,
                          float g,
                          float b,
                          float alphaScale) {
        D3D11Renderer::SpriteCmd sprite{};
        sprite.texture = texture;
        sprite.x = Stage1ScreenX(viewport, centerX - sizePsx * 0.5f);
        sprite.y = Stage1ScreenY(viewport, centerY - sizePsx * 0.5f);
        sprite.w = Stage1ScreenLength(viewport, sizePsx);
        sprite.h = Stage1ScreenLength(viewport, sizePsx);
        sprite.r = r;
        sprite.g = g;
        sprite.b = b;
        sprite.a = alpha * alphaScale;
        sprite.blend = D3D11Renderer::BlendMode::Additive;
        sprite.layer = layer;
        sprite.order = static_cast<uint64_t>(psxCallOrder++ + 1u);
        ctx.renderer->SubmitSprite(sprite);
    };

    submitGlow(basePsx * glowScale * 1.35f * pulse,
               1.0f,
               0.76f,
               0.18f,
               0.70f);
    submitGlow(basePsx * glowScale * 0.78f * pulse,
               1.0f,
               0.96f,
               0.58f,
               0.55f);
}

static int16_t Stage1Parappa2RailRsinStep256(int32_t phase) {
    static constexpr std::array<int16_t, 32> kRsin = {{
        0, 1567, 2896, 3784, 4096, 3784, 2896, 1567,
        0, -1567, -2896, -3784, -4096, -3784, -2896, -1567,
        0, 1567, 2896, 3784, 4096, 3784, 2896, 1567,
        0, -1567, -2896, -3784, -4096, -3784, -2896, -1567,
    }};
    int32_t wrapped = phase % 8192;
    if (wrapped < 0) {
        wrapped += 8192;
    }
    return kRsin[static_cast<std::size_t>(wrapped / 256) & 31u];
}

static float Stage1Parappa2RailPopScaleSample(uint32_t age,
                                              float popScale) {
    constexpr int32_t kIdentity = 4096;
    constexpr int32_t kInitialAcc = 2048;
    constexpr int32_t kInitialVel = 2048;
    constexpr int32_t kFlipVel = -1024;
    int32_t acc = kInitialAcc;
    int32_t vel = kInitialVel;
    int32_t scaleWord = kIdentity;
    for (uint32_t frame = 0; frame <= age; ++frame) {
        acc += vel;
        scaleWord = acc + kIdentity;
        if (acc >= kIdentity) {
            vel = kFlipVel;
        }
    }
    return (static_cast<float>(scaleWord) /
            static_cast<float>(kIdentity)) *
           (popScale / 2.0f);
}

static void ResolveStage1Parappa2RailStampScale(
    const PrGameContext& ctx,
    float ageFrames,
    float& scaleX,
    float& scaleY,
    float& glowSolidEndFrame) {
    const uint32_t popFrames =
        static_cast<uint32_t>((std::max)(0, ctx.stage1RailParappa2PopFrames));
    const float popScale = (std::max)(1.0f, ctx.stage1RailParappa2PopScale);
    const int flipFrames = (std::max)(1, ctx.stage1RailParappa2FlipFrames);
    const uint32_t effectiveFlipFrames =
        static_cast<uint32_t>((std::max)(14, flipFrames));
    const uint32_t flipEndFrame = popFrames + effectiveFlipFrames;
    glowSolidEndFrame = static_cast<float>(flipEndFrame);

    scaleX = 1.0f;
    scaleY = 1.0f;
    ageFrames = (std::max)(0.0f, ageFrames);
    if (ageFrames < static_cast<float>(popFrames)) {
        const float ageFloorFloat = std::floor(ageFrames);
        const uint32_t ageFloor =
            static_cast<uint32_t>((std::max)(0.0f, ageFloorFloat));
        const uint32_t ageNext = ageFloor + 1u;
        const float frac = ageFrames - ageFloorFloat;
        const float scale0 =
            Stage1Parappa2RailPopScaleSample(ageFloor, popScale);
        const float scale1 =
            ageNext < popFrames
                ? Stage1Parappa2RailPopScaleSample(ageNext, popScale)
                : 1.0f;
        const float scale = Stage1Lerp428B0(scale0, scale1, frac);
        scaleX = scale;
        scaleY = scale;
        return;
    }

    if (ageFrames < static_cast<float>(flipEndFrame)) {
        const float flipAge = ageFrames - static_cast<float>(popFrames);
        float t = 0.0f;
        if (effectiveFlipFrames > 1u) {
            t = flipAge /
                static_cast<float>(effectiveFlipFrames - 1u);
        }

        constexpr int32_t kOneFlipPhaseStart = 2048;
        constexpr int32_t kOneFlipPhaseRange = 4096;
        const int32_t phase =
            kOneFlipPhaseStart +
            static_cast<int32_t>(
                std::clamp(t, 0.0f, 1.0f) *
                    static_cast<float>(kOneFlipPhaseRange) +
                0.5f);
        scaleX = static_cast<float>(Stage1Parappa2RailRsinStep256(phase)) /
                 4096.0f;
        scaleY = 1.0f;
    }
}

static void DrawStage1Parappa2RailStamp(
    PrGameContext& ctx,
    const Stage1Parappa2RailStamp& stamp,
    const Stage1Viewport& viewport,
    float visualFrame,
    uint32_t& psxCallOrder) {
    using Command =
        PrStageSceneSubmitDirect::PsxCompactRailSpriteCommand80024744;
    const float centerX = stamp.centerX;
    const float centerY = stamp.centerY;

    Command command{};
    command.kind = Command::Kind::Note;
    command.anchor = Command::Anchor::Center;
    command.tpl = stamp.tpl;
    command.x = centerX;
    command.y = centerY;
    command.otBucket = 1u;
    command.row = stamp.row;
    command.raw = stamp.symbolType;

    const float ageFrames =
        (std::max)(0.0f, visualFrame - stamp.startFrame);
    const int glowFadeFrames =
        (std::max)(0, ctx.stage1RailParappa2GlowFadeFrames);
    float scaleX = 1.0f;
    float scaleY = 1.0f;
    float glowSolidEndFrame = 0.0f;
    ResolveStage1Parappa2RailStampScale(ctx,
                                        ageFrames,
                                        scaleX,
                                        scaleY,
                                        glowSolidEndFrame);

    float glowT = 0.0f;
    if (ageFrames < glowSolidEndFrame) {
        glowT = 1.0f;
    } else if (glowFadeFrames > 0) {
        const float fadeAge = ageFrames - glowSolidEndFrame;
        if (fadeAge < static_cast<float>(glowFadeFrames)) {
            glowT = 1.0f -
                    fadeAge / static_cast<float>(glowFadeFrames);
        }
    }

    if (glowT > 0.0f) {
        DrawStage1Parappa2RailGlow(ctx,
                                   viewport,
                                   command,
                                   centerX,
                                   centerY,
                                   scaleY,
                                   glowT,
                                   psxCallOrder);
    }

    command.scaleX = scaleX;
    command.scaleY = scaleY;
    DrawStage1CompactRailSprite80024744(ctx,
                                        command,
                                        viewport,
                                        psxCallOrder++,
                                        1.0f,
                                        1.0f,
                                        1.0f,
                                        1.0f);
}

static void DrawStage1CompactRailParappa2(
    PrGameContext& ctx,
    const std::vector<
        PrStageSceneSubmitDirect::PsxCompactRailSpriteCommand80024744>&
        commands,
    const Stage1Viewport& viewport) {
    const uint32_t logicFrame = ResolveStage1Parappa2RailLogicFrame(ctx);
    const float renderFrameFrac = Stage1Parappa2RailRenderFrameFrac(ctx);
    const float displayFrame =
        ResolveStage1Parappa2RailDisplayFrame(logicFrame, renderFrameFrac);
    Stage1Parappa2RailCursorAlign liveAlign =
        ResolveStage1Parappa2RailCursorAlign(ctx,
                                             logicFrame,
                                             false,
                                             renderFrameFrac);
    const bool updateVisualCorrection =
        !ctx.renderOnlyFrame || ctx.renderSubFrame8 == 0u;
    ResolveStage1Parappa2RailVisualCorrection(commands,
                                              liveAlign,
                                              updateVisualCorrection);
    std::vector<
        PrStageSceneSubmitDirect::PsxCompactRailSpriteCommand80024744>
        ledCommands = commands;
    ApplyStage1Parappa2RailPortraitAlign(ledCommands, liveAlign);

    std::array<Stage1Parappa2RailRowFrame,
               PrStageSceneSubmitDirect::kCompactRailMaxRows80024744>
        rows{};
    BuildStage1Parappa2RailRows(ledCommands, rows);
    TickStage1Parappa2RailRuntime(ctx, rows, logicFrame);

    uint32_t psxCallOrder = 1u;
    using Kind =
        PrStageSceneSubmitDirect::PsxCompactRailSpriteCommand80024744::Kind;
    for (const auto& command : ledCommands) {
        if (command.kind == Kind::Note &&
            command.row < rows.size() &&
            rows[command.row].studentActive) {
            auto darkCommand = command;
            darkCommand.scaleX = 1.0f;
            darkCommand.scaleY = 1.0f;
            const float dark = std::clamp(ctx.stage1RailParappa2Darken,
                                          0.0f,
                                          1.0f);
            DrawStage1CompactRailSprite80024744(ctx,
                                                darkCommand,
                                                viewport,
                                                psxCallOrder++,
                                                dark,
                                                dark,
                                                dark,
                                                1.0f);
            continue;
        }

        DrawStage1CompactRailSprite80024744(ctx,
                                            command,
                                            viewport,
                                            psxCallOrder++);
    }

    for (const Stage1Parappa2RailStamp& stamp :
         s_stage1Parappa2RailRuntime.stamps) {
        DrawStage1Parappa2RailStamp(ctx,
                                    stamp,
                                    viewport,
                                    displayFrame,
                                    psxCallOrder);
    }
}

static std::vector<
    PrStageSceneSubmitDirect::PsxCompactRailSpriteCommand80024744>
CollectStage1CompactRailSpriteCommands80024744(
    const PrStageSceneSubmitDirect::Stage1SceneSubmitRuntime& runtime) {
    std::vector<
        PrStageSceneSubmitDirect::PsxCompactRailSpriteCommand80024744>
        commands;
    PrStageSceneSubmitDirect::BuildStage1CompactRailSpriteCommands80024744(
        runtime,
        +[](const PrStageSceneSubmitDirect::PsxCompactRailSpriteCommand80024744&
                command,
            void* userData) {
            auto* commands = static_cast<std::vector<
                PrStageSceneSubmitDirect::PsxCompactRailSpriteCommand80024744>*>(
                userData);
            if (commands != nullptr) {
                commands->push_back(command);
            }
        },
        &commands);
    return commands;
}

static void DrawStage1CompactRail80024744(
    PrGameContext& ctx,
    const PrStageSceneSubmitDirect::Stage1SceneSubmitRuntime& runtime,
    const Stage1Viewport& viewport) {
    if (!ctx.renderer) {
        return;
    }

    std::vector<
        PrStageSceneSubmitDirect::PsxCompactRailSpriteCommand80024744>
        commands = CollectStage1CompactRailSpriteCommands80024744(
            runtime);

    if (ctx.stage1RailMode == 1) {
        DrawStage1CompactRailParappa2(ctx, commands, viewport);
        return;
    }
    ResetStage1Parappa2RailRuntime();

    struct CompactRailDrawContext80024744 {
        PrGameContext* ctx = nullptr;
        Stage1Viewport viewport{};
        uint32_t psxCallOrder = 1u;
    };
    CompactRailDrawContext80024744 drawCtx{&ctx, viewport, 1u};
    for (const auto& command : commands) {
        DrawStage1CompactRailSprite80024744(*drawCtx.ctx,
                                            command,
                                            drawCtx.viewport,
                                            drawCtx.psxCallOrder++);
    }
}

static void DrawStage1GsSpritePackets8003F1B4(
    PrGameContext& ctx,
    const PrStageSceneSubmitDirect::Stage1SceneSubmitRuntime& runtime,
    const Stage1Viewport& viewport) {
    if (!ctx.renderer) {
        return;
    }

    EnsureStage1VramAtlas(ctx);
    Stage1TmdDrawContext428B0 drawCtx{&ctx, viewport};
    PrStageSceneSubmitDirect::BuildStage1GsSpritePacketCommands8003F1B4(
        runtime,
        +[](const PrStageSceneSubmitDirect::PsxGsSpritePacketCommand8003F1B4&
                command,
            void* userData) {
            auto* drawCtx = static_cast<Stage1TmdDrawContext428B0*>(userData);
            if (drawCtx == nullptr || drawCtx->ctx == nullptr) {
                return;
            }
            DrawStage1GsSpritePacket8003F1B4(*drawCtx->ctx,
                                             command,
                                             drawCtx->viewport);
        },
        &drawCtx);
}

void DrawStage1FastSpriteRuntime8003FA20(
    PrGameContext& ctx,
    const PrPsxFastSpriteSubmitDirect::RuntimeState8003FA20& runtime) {
    if (!ctx.renderer) {
        return;
    }

    EnsureStage1VramAtlas(ctx);
    const Stage1Viewport viewport = CalcPs1Viewport(ctx.renderer);
    Stage1TmdDrawContext428B0 drawCtx{&ctx, viewport};
    PrStageSceneSubmitDirect::BuildFastSpritePacketCommands8003FA20(
        runtime,
        +[](const PrStageSceneSubmitDirect::PsxFastSpritePacketCommand8003FA20&
                command,
            void* userData) {
            auto* drawCtx = static_cast<Stage1TmdDrawContext428B0*>(userData);
            if (drawCtx == nullptr || drawCtx->ctx == nullptr) {
                return;
            }
            DrawStage1FastSpritePacket8003FA20(*drawCtx->ctx,
                                               command,
                                               drawCtx->viewport);
        },
        &drawCtx);
}

void DrawEventFrameBoxFillPackets8003EE84(
    PrGameContext& ctx,
    const PrPsxEventFrameDirect::EventFrameState8001E750& state) {
    if (!ctx.renderer) {
        return;
    }

    const Stage1Viewport viewport = CalcPs1Viewport(ctx.renderer);
    Stage1TmdDrawContext428B0 drawCtx{&ctx, viewport};
    PrPsxEventFrameDirect::BuildEventFrameBoxFillPacketCommands8003EE84(
        state,
        +[](const PrPsxEventFrameDirect::BoxFillPacketCommand8003EE84& command,
            void* userData) {
            auto* drawCtx = static_cast<Stage1TmdDrawContext428B0*>(userData);
            if (drawCtx == nullptr || drawCtx->ctx == nullptr) {
                return;
            }
            DrawEventFrameBoxFillPacket8003EE84(*drawCtx->ctx,
                                                command,
                                                drawCtx->viewport);
        },
        &drawCtx);
}

void DrawEventFrameMoveImageBoxFill8001B120(
    PrGameContext& ctx,
    const PrPsxEventFrameDirect::EventFrameState8001E750& state) {
    if (!ctx.renderer ||
        !state.stage1Event4MoveImageUnderlayValid8001B120 ||
        !state.stage1Event4MoveImageBoxFill8001B120.valid) {
        return;
    }

    const PrPsxEventFrameDirect::EventFrameMoveImageBoxFill8001B120& copy =
        state.stage1Event4MoveImageBoxFill8001B120;
    if (copy.w == 0u || copy.h == 0u) {
        return;
    }

    const Stage1Viewport viewport = CalcPs1Viewport(ctx.renderer);
    D3D11Renderer::SolidRectCmd rect{};
    if (copy.x == 0 && copy.y == 0 && copy.w == 320u && copy.h == 240u) {
        rect.x = 0.0f;
        rect.y = 0.0f;
        rect.w = static_cast<float>(ctx.renderer->GetWidth());
        rect.h = static_cast<float>(ctx.renderer->GetHeight());
    } else {
        rect.x = Stage1ScreenX(viewport, static_cast<float>(copy.x));
        rect.y = Stage1ScreenY(viewport, static_cast<float>(copy.y));
        rect.w = Stage1ScreenLength(viewport, static_cast<float>(copy.w));
        rect.h = Stage1ScreenLength(viewport, static_cast<float>(copy.h));
    }
    rect.r = static_cast<float>(copy.r) / 255.0f;
    rect.g = static_cast<float>(copy.g) / 255.0f;
    rect.b = static_cast<float>(copy.b) / 255.0f;
    rect.a = PsxBoxFillAlpha8003EE84(copy.commandCode);
    rect.layer = 0;
    rect.order = 0;
    ctx.renderer->SubmitSolidRect(rect);
}

void DrawEventFrameFastSpritePackets8003FA20(
    PrGameContext& ctx,
    const PrPsxEventFrameDirect::EventFrameState8001E750& state) {
    if (!ctx.renderer || !state.initialized ||
        !state.graph.mainPageWorkLists80087288Initialized) {
        return;
    }

    const uint8_t pageIndex = static_cast<uint8_t>(state.gp368WorkSlot & 1u);
    const PrPsxGraphOwnerDirect::PsxGraphPageWorkList8001E374& pageWork =
        state.graph.mainPageWorkLists80087288[pageIndex];
    if (!pageWork.work.packetWriteMirrorKnown) {
        return;
    }

    EnsureStage1VramAtlas(ctx);
    const Stage1Viewport viewport = CalcPs1Viewport(ctx.renderer);
    Stage1TmdDrawContext428B0 drawCtx{&ctx, viewport};
    PrPsxFastSpriteSubmitDirect::RuntimeState8003FA20 submitRuntime{};
    submitRuntime.packetWrites = pageWork.work.packetWriteMirror;
    PrStageSceneSubmitDirect::BuildFastSpritePacketCommands8003FA20(
        submitRuntime,
        +[](const PrStageSceneSubmitDirect::PsxFastSpritePacketCommand8003FA20&
                command,
            void* userData) {
            auto* drawCtx = static_cast<Stage1TmdDrawContext428B0*>(userData);
            if (drawCtx == nullptr || drawCtx->ctx == nullptr) {
                return;
            }
            DrawEventFrameFastSpritePacket8003FA20(*drawCtx->ctx,
                                                   command,
                                                   drawCtx->viewport);
        },
        &drawCtx);
}

static void DrawStage1PreventTransition801CBFDC(
    PrGameContext& ctx,
    const PrStageSceneSubmitDirect::Stage1SceneSubmitRuntime& runtime,
    const Stage1Viewport& viewport) {
    if (!ctx.renderer) {
        return;
    }

    PrStage1Scene1Movie1Direct::Movie1DrawPlan plan{};
    if (!PrStageSceneSubmitDirect::
            BuildStage1PreventTransitionDrawPlan801CBFDC(
                runtime,
                viewport.x,
                viewport.y,
                viewport.scale,
                plan)) {
        return;
    }

    for (uint32_t i = 0; i < plan.templateCount; ++i) {
        const PrStage1Scene1Movie1Direct::Movie1TemplateDrawCommand& command =
            plan.templates[i];
        if (!command.desc.valid) {
            continue;
        }
        const PrPsxSpriteTemplateRender::PsxSpriteTemplate tpl{
            command.desc.attr,
            command.desc.texX,
            command.desc.texY,
            command.desc.w,
            command.desc.h,
            command.desc.clutX,
            command.desc.clutY,
        };
        const bool drewFromVram =
            PrPsxSpriteTemplateRender::DrawPsxSpriteTemplateViaUiAtlas(
                ctx,
                plan.frame.vx,
                plan.frame.vy,
                plan.frame.vs,
                command.x,
                command.y,
                tpl,
                1.0f,
                1.0f,
                1.0f,
                command.alpha,
                command.layer,
                command.order);
        if (!drewFromVram) {
            (void)PrPsxSpriteTemplateRender::DrawPsxSpriteTemplateOrdered(
                ctx,
                plan.frame.vx,
                plan.frame.vy,
                plan.frame.vs,
                command.x,
                command.y,
                tpl,
                1.0f,
                1.0f,
                1.0f,
                command.alpha,
                command.layer,
                command.order);
        }
    }
}

static void DrawStage1ScriptBoxCommand80023E10(
    PrGameContext& ctx,
    const PrStageSceneSubmitDirect::PsxScriptBoxCommand80023E10& command,
    const Stage1Viewport& viewport) {
    (void)ctx;
    (void)command;
    (void)viewport;
}

static void DrawStage1ScriptBox80023E10(
    PrGameContext& ctx,
    const PrStageSceneSubmitDirect::Stage1SceneSubmitRuntime& runtime,
    const Stage1Viewport& viewport) {
    Stage1TmdDrawContext428B0 drawCtx{&ctx, viewport};
    PrStageSceneSubmitDirect::BuildStage1ScriptBoxCommands80023E10(
        runtime,
        +[](const PrStageSceneSubmitDirect::PsxScriptBoxCommand80023E10& command,
            void* userData) {
            auto* drawCtx = static_cast<Stage1TmdDrawContext428B0*>(userData);
            if (drawCtx == nullptr || drawCtx->ctx == nullptr) {
                return;
            }
            DrawStage1ScriptBoxCommand80023E10(*drawCtx->ctx,
                                               command,
                                               drawCtx->viewport);
        },
        &drawCtx);
}

static void DrawStage1DrawEnvClearColor80040060(
    PrGameContext& ctx,
    const PrStageSceneSubmitDirect::Stage1SceneSubmitRuntime& runtime,
    const Stage1Viewport& viewport) {
    if (!ctx.renderer) {
        return;
    }

    PrStageSceneSubmitDirect::PsxDrawEnvClearColorCommand80040060 command{};
    if (!PrStageSceneSubmitDirect::BuildStage1DrawEnvClearColorCommand80040060(
            runtime,
            command)) {
        return;
    }

    D3D11Renderer::SolidRectCmd rect{};
    Stage1FullscreenBackgroundXRange(ctx.renderer,
                                      viewport,
                                      0.0f,
                                      kStage1PsxScreenWidth,
                                      rect.x,
                                      rect.w);
    rect.y = Stage1ScreenY(viewport, 0.0f);
    rect.h = Stage1ScreenLength(viewport, kStage1PsxScreenHeight);
    rect.r = static_cast<float>(command.r) / 255.0f;
    rect.g = static_cast<float>(command.g) / 255.0f;
    rect.b = static_cast<float>(command.b) / 255.0f;
    rect.a = 1.0f;
    rect.layer = 0;
    rect.order = 1;
    ctx.renderer->SubmitSolidRect(rect);
}

static void DrawStage1Scene801CBFDC190Impl(PrGameContext& ctx,
                                           bool includeGameplayUi) {
    const Stage1Viewport viewport = CalcPs1Viewport(ctx.renderer);
    const PrStageSceneSubmitDirect::Stage1SceneSubmitRuntime& runtime =
        s_stage1UseRenderOnlyRuntime801CBFDC190
            ? s_stage1RenderOnlyRuntime801CBFDC190
            : PrStageSceneSubmitDirect::
                  GetOwnedStage1SceneSubmitRuntime801CBFDC190();
    DrawStage1DrawEnvClearColor80040060(ctx, runtime, viewport);
    DrawStage1Backdrop801CAC34(ctx, runtime, viewport);
    DrawStage1PreventTransition801CBFDC(ctx, runtime, viewport);
    DrawStage1Tmd428B0(ctx, runtime, viewport);
    DrawStage1GsSpritePackets8003F1B4(ctx, runtime, viewport);
    if (includeGameplayUi) {
        DrawStage1CompactRail80024744(ctx, runtime, viewport);
    }
    DrawStage1BoxFillPackets8003EE84(ctx, runtime, viewport);
    if (includeGameplayUi) {
        DrawStage1FastSpritePackets8003FA20(ctx, runtime, viewport);
        DrawStage1ScriptBox80023E10(ctx, runtime, viewport);
    }
}

void DrawStage1Scene801CBFDC190(PrGameContext& ctx) {
    DrawStage1Scene801CBFDC190Impl(ctx, true);
}

void DrawStage1SceneGameplayBase801CBFDC190(PrGameContext& ctx) {
    DrawStage1Scene801CBFDC190Impl(ctx, false);
}

static void DrawStage1BackdropCommandBatch(
    PrGameContext& ctx,
    const PrStageSceneSubmitDirect::PsxBackdropGradientCommand801CAC34& gradient,
    const PrStageSceneSubmitDirect::PsxBackdropSpriteCommand801CAC34* sprites,
    std::size_t spriteCount,
    const Stage1Viewport& viewport) {
    if (!ctx.renderer || !ctx.resources) {
        return;
    }

    if (gradient.valid && gradient.h > 0.0f) {
        const float tr =
            static_cast<float>((gradient.topRgb >> 16) & 0xFFu) / 255.0f;
        const float tg =
            static_cast<float>((gradient.topRgb >> 8) & 0xFFu) / 255.0f;
        const float tb =
            static_cast<float>(gradient.topRgb & 0xFFu) / 255.0f;
        const float br =
            static_cast<float>((gradient.bottomRgb >> 16) & 0xFFu) / 255.0f;
        const float bg =
            static_cast<float>((gradient.bottomRgb >> 8) & 0xFFu) / 255.0f;
        const float bb =
            static_cast<float>(gradient.bottomRgb & 0xFFu) / 255.0f;
        constexpr int kGradientSteps = 18;
        for (int i = 0; i < kGradientSteps; ++i) {
            const float t0 =
                static_cast<float>(i) / static_cast<float>(kGradientSteps);
            const float t1 =
                static_cast<float>(i + 1) / static_cast<float>(kGradientSteps);
            const float y0 =
                Stage1ScreenY(viewport, gradient.y + gradient.h * t0);
            const float y1 =
                Stage1ScreenY(viewport, gradient.y + gradient.h * t1);
            D3D11Renderer::SolidRectCmd rect{};
            Stage1FullscreenBackgroundXRange(ctx.renderer,
                                              viewport,
                                              gradient.x,
                                              gradient.w,
                                              rect.x,
                                              rect.w);
            rect.y = y0;
            rect.h = SnapStage1ScreenLength(y1 - y0);
            rect.r = tr + (br - tr) * t0;
            rect.g = tg + (bg - tg) * t0;
            rect.b = tb + (bb - tb) * t0;
            rect.a = 1.0f;
            rect.layer = 0;
            rect.order = Stage1PacketOrder(0u, static_cast<uint32_t>(i));
            ctx.renderer->SubmitSolidRect(rect);
        }
    }

    EnsureStage1VramAtlas(ctx);

    for (std::size_t i = 0; i < spriteCount; ++i) {
        const auto& sprite = sprites[i];
        const char* textureName = Stage1BackdropTextureName801CAC34(sprite.texture);
        TextureResource* tr = ctx.resources->GetTexture(textureName);
        if (tr == nullptr || tr->tim.width == 0 || tr->tim.height == 0) {
            continue;
        }

        Stage1BackdropAtlasSample801CAC34 sample{};
        if (!ResolveStage1BackdropAtlasSample801CAC34(ctx.renderer,
                                                       *tr,
                                                       sample)) {
            sample.srv = ctx.resources->GetTextureView(textureName);
            sample.width = static_cast<float>(tr->tim.width);
            sample.height = static_cast<float>(tr->tim.height);
            sample.u0 = 0.0f;
            sample.v0 = 0.0f;
            sample.u1 = 1.0f;
            sample.v1 = 1.0f;
        }
        if (sample.srv == nullptr ||
            sample.width <= 0.0f ||
            sample.height <= 0.0f) {
            continue;
        }

        const float x0 = Stage1ScreenX(viewport, sprite.x);
        const float y0 = Stage1ScreenY(viewport, sprite.y);
        const float x1 =
            x0 + Stage1ScreenLength(viewport, sample.width);
        const float y1 =
            y0 + Stage1ScreenLength(viewport, sample.height);
        TexturedVertex verts[6] = {
            {x0, y0, sample.u0, sample.v0, 1.0f, 1.0f, 1.0f, sprite.alpha},
            {x1, y0, sample.u1, sample.v0, 1.0f, 1.0f, 1.0f, sprite.alpha},
            {x1, y1, sample.u1, sample.v1, 1.0f, 1.0f, 1.0f, sprite.alpha},
            {x0, y0, sample.u0, sample.v0, 1.0f, 1.0f, 1.0f, sprite.alpha},
            {x1, y1, sample.u1, sample.v1, 1.0f, 1.0f, 1.0f, sprite.alpha},
            {x0, y1, sample.u0, sample.v1, 1.0f, 1.0f, 1.0f, sprite.alpha},
        };
        D3D11Renderer::TexturedTriCmd cmd{};
        cmd.texture = sample.srv;
        for (int vi = 0; vi < 6; ++vi) {
            cmd.vertices[vi] = verts[vi];
        }
        cmd.vertexCount = 6;
        cmd.blend = D3D11Renderer::BlendMode::Alpha;
        cmd.layer = 1;
        cmd.order = Stage1PacketOrder(0u, static_cast<uint32_t>(i));
        ctx.renderer->SubmitTexturedTriangles(cmd);
    }
}

} // namespace PrStageSceneSubmitBackend
