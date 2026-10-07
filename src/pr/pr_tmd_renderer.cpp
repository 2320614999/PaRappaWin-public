#include "pr_tmd_renderer.h"
#include "pr_tmd.h"
#include "pr_mime.h"
#include "pr_vram_atlas.h"
#include "pr_game_context.h"
#include "pr_stage1_camera_motion_direct.h"
#include "d3d11_renderer.h"
#include "resource_manager.h"
#include "str_player.h"
#include "scene_event_parser.h"
#include "face_event_processor.h"
#include "logger.h"
#include "pr_sfx.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <chrono>
#include <unordered_map>

namespace PrTmdRenderer {

// S0_OLD_DELETE_AFTER_SS0: legacy Scene0/title-only renderer. SS0 may observe
// the visual side effects, but must not depend on this renderer as S0 logic.
// Boundary note: this renderer is Scene0/title-only. Stage1 scene submit state
// and 801CBFDC/801CB190 command semantics live in the direct/backend path.

// Scene0 loads 4 TMDs: HP(handle5), LO(handle6), PA(handle7), PA_KAGE(handle8)
static const char* kTmdNames[] = { "hp.tmd", "lo.tmd", "pa.tmd", "pa_kage.tmd" };
static const int kTmdCount = 4;

static TmdModel s_models[4];
static bool s_loaded[4] = {};
static bool s_anyLoaded = false;

// Stage COMPO archives contain a larger, scene-specific TMD directory.  Keep
// it isolated from the Scene0/title models so loading a stage cannot mutate
// the title animation state or make the legacy shell the visual owner.
static constexpr int kStageModelCapacity = 64;
static TmdModel s_stageModels[kStageModelCapacity];
static bool s_stageLoaded[kStageModelCapacity] = {};
static std::vector<TmdVertex> s_stageAnimVerts[kStageModelCapacity];
static bool s_stageHasAnimVerts[kStageModelCapacity] = {};
static int s_stageModelCount = 0;
static bool s_stageAnyLoaded = false;
static int s_stageSceneId = -1;

// Scene0 VDF/DAT/TOD resource names (from memory.md handle table)
static const char* kVdfNames[] = {
    "hippop.vdf", "logo.vdf", "logonew.vdf",
    "pa_l.vdf", "pa_r.vdf", "pa_t_l.vdf", "pa_t_r.vdf", "pa_oki.vdf", "pa_dance.vdf"
};
static const char* kDatNames[] = {
    "hippop.dat", "logo.dat", "logonew.dat",
    "pa_l.dat", "pa_r.dat", "pa_t_l.dat", "pa_t_r.dat", "pa_oki.dat", "pa_dance.dat"
};
static const char* kTodNames[] = {
    "pa_loc.tod", "pa_loc2.tod", "pa_dance.tod"
};
static bool s_mimeLoaded = false;
static ResourceManager* s_resources = nullptr;  // saved for runtime TIM swaps

// VRAM texture atlas for TMD textured rendering
static PsxVramAtlas s_vramAtlas;
static bool s_atlasReady = false;

// Animated vertex buffers (per-model, object0)
static std::vector<TmdVertex> s_animVerts[4];
static bool s_hasAnimVerts[4] = {};

// Animation phase state machine (PSX Scene0 sequence)
// WaitVideo: freeze animation until STR video finishes
// StandUp:   PA_OKI on chan0 (one-shot, ~32 frames)
// Dance:     PA_DANCE on chan0 (one-shot, ~267 frames)
// DanceEndHold: freeze at the end of PA_DANCE for N ticks
// MenuIdle:  PA_L on chan0 (looping, pointing at cursor)
enum class AnimPhase { WaitVideo, StandUp, Dance, DanceEndHold, MenuIdle };
static AnimPhase s_animPhase = AnimPhase::WaitVideo;
static uint32_t s_phaseStartTick = 0;
static const uint32_t kOkiFrames   = 32;   // PA_OKI stand-up duration
static const uint32_t kDanceFrames = 267;  // PA_DANCE full performance duration
static uint32_t s_okiFrames = kOkiFrames;
static uint32_t s_danceFrames = kDanceFrames;

// PSX事件5: LOGONEW/HIPPOP在v10=318时激活 → 从v10=50起经过268个tick
static const uint32_t kTitleFlyInTick = 268;
static bool s_titleFlyInActivated = false;

// PSX face expression system: parsed from COMOD*.BIN at runtime
static FaceEventProcessor s_faceProcessor;
static bool s_faceProcessorReady = false;
static uint32_t s_scn0SqevFrame = 0;
static bool s_loggedScn0SqevNonZero = false;
static const uint32_t kLegacyScn0FaceMaxFrame = 9792u;
static uint32_t s_faceFrameAbs = 0;
static uint32_t s_scn0FaceMaxFrame = kLegacyScn0FaceMaxFrame;

static TodCamera s_savedTodCam;
static TodViewport s_savedTodVp;
static bool s_hasSavedTodView = false;

struct PsxCamera {
    float pos[3];
    float tgt[3];
    float projDist;
    float screenCX;
    float screenCY;
};

static PsxCamera s_camera = {
    { 78.0f, -1437.0f, -9869.0f },
    { 0.0f, -1400.0f, 0.0f },
    440.0f, 160.0f, 120.0f
};

static bool s_viewMatrixBuilt = false;

// ====================================================================
// ComodStageEventParser — Parse COMOD0.BIN overlay event table
// PSX: off_801C6E50 = {basePtr, count, idx}, entries are 16B each:
//   +0x00 u32 time     — trigger tick (monotonically increasing)
//   +0x04 u32 flags    — action bitflags
//   +0x08 u8  bezHdl   — BEZ camera resource handle (when non-zero → ctx->flags|=0x0400)
//   +0x09 u8  todHdl   — TOD resource handle (when flags & 0x00040000)
//   +0x0A u8  vdfPair  — VDF/DAT pair index (when flags & 0x00010000)
//   +0x0B u8  hudSlot  — HUD overlay slot index
//   +0x0C..0x0F u8     — additional params
// Handle mapping: handle h → GetMemNamesOrdered()[h - 2]
// ====================================================================
struct ComodStageEvent {
    uint32_t time;
    uint32_t flags;
    uint8_t  bezHandle;   // +0x08
    uint8_t  todHandle;   // +0x09
    uint8_t  vdfPairIdx;  // +0x0A
    uint8_t  hudSlot;     // +0x0B
    uint8_t  params[4];   // +0x0C..0x0F
};

struct ComodStageEventTable {
    std::vector<ComodStageEvent> events;
    int camBezEventIdx = -1;   // first event with BEZ handle set
    bool parsed = false;

    // Scan COMOD0.BIN for the event table
    bool Parse(const uint8_t* data, size_t size) {
        events.clear();
        camBezEventIdx = -1;
        parsed = false;
        if (!data || size < 32) return false;

        // Strategy: scan for the stream descriptor pattern
        // Descriptor: {u32 basePtr (0x801xxxxx), u32 count (1..20), u32 idx (0)}
        // Then compute overlay_base = basePtr - event_array_offset
        // Since we don't know the exact overlay_base, we try a different approach:
        // Search for sequences of valid event entries directly.

        int bestStart = -1;
        int bestCount = 0;

        for (size_t off = 0; off + 16 <= size; off += 4) {
            // Try reading this as the start of an event array
            int count = 0;
            uint32_t prevTime = 0;
            bool valid = true;

            for (size_t eoff = off; eoff + 16 <= size && count < 30; eoff += 16) {
                uint32_t time  = *(const uint32_t*)(data + eoff);
                uint32_t flags = *(const uint32_t*)(data + eoff + 4);

                // Time must be reasonable: < 0x10000 ticks (~36 min at 30Hz)
                if (time > 0xFFFF) { valid = false; break; }
                // Time must be monotonically non-decreasing
                if (count > 0 && time < prevTime) break;
                // First entry time should be > 0 (typically 0xC0+ for S0)
                if (count == 0 && time == 0) { valid = false; break; }
                // Flags should have at most known bits set
                // Known flag bits: 0x02050020, 0x01C10000, etc.
                // Reject if flags has bits above 0x0FFFFFFF (unlikely)
                if (flags & 0xF0000000 && flags != 0) { valid = false; break; }

                prevTime = time;
                count++;
            }

            if (valid && count >= 3 && count > bestCount) {
                // Additional validation: check that at least one entry has BEZ handle
                bool hasBez = false;
                for (int i = 0; i < count; i++) {
                    uint8_t bh = *(data + off + i * 16 + 8);
                    if (bh != 0 && bh < 100) { hasBez = true; break; }
                }
                if (hasBez) {
                    bestStart = (int)off;
                    bestCount = count;
                }
            }
        }

        if (bestStart < 0 || bestCount < 1) {
            Log::Printf("ComodEvent: no event table found in COMOD0.BIN (%zu bytes)", size);
            return false;
        }

        // Parse the found event array
        for (int i = 0; i < bestCount; i++) {
            const uint8_t* ep = data + bestStart + i * 16;
            ComodStageEvent ev;
            ev.time       = *(const uint32_t*)(ep + 0);
            ev.flags      = *(const uint32_t*)(ep + 4);
            ev.bezHandle  = ep[8];
            ev.todHandle  = ep[9];
            ev.vdfPairIdx = ep[10];
            ev.hudSlot    = ep[11];
            ev.params[0]  = ep[12];
            ev.params[1]  = ep[13];
            ev.params[2]  = ep[14];
            ev.params[3]  = ep[15];

            if (ev.bezHandle != 0 && camBezEventIdx < 0) {
                camBezEventIdx = i;
            }

            events.push_back(ev);
        }

        parsed = true;
        Log::Printf("ComodEvent: parsed %d events (offset=0x%X), camBezEvent=%d",
                     bestCount, bestStart, camBezEventIdx);
        for (int i = 0; i < bestCount; i++) {
            const auto& e = events[i];
            Log::Printf("  [%d] time=0x%X flags=0x%08X bez=%d tod=%d vdf=%d hud=%d",
                         i, e.time, e.flags, e.bezHandle, e.todHandle, e.vdfPairIdx, e.hudSlot);
        }
        return true;
    }

    // Resolve BEZ handle to resource name using ResourceManager's ordered Mem list
    std::string ResolveBezName(const ResourceManager* rm) const {
        if (camBezEventIdx < 0 || !rm) return "";
        int handle = events[camBezEventIdx].bezHandle;
        if (handle < 2) return "";
        const auto& ordered = rm->GetMemNamesOrdered();
        int idx = handle - 2;
        if (idx < 0 || idx >= (int)ordered.size()) return "";
        return ordered[idx];
    }

    uint32_t GetBezStartTime() const {
        if (camBezEventIdx < 0) return 0;
        return events[camBezEventIdx].time;
    }
};

static ComodStageEventTable s_comodEvents;

// Reuse the BEZ sampler implementation only; no Stage1 scene state is owned here.
using CameraBezPlayer = PrStage1CameraMotionDirect::CameraBezPlayer;

static CameraBezPlayer s_camBez;
static uint32_t s_camBezStartTick = 0;  // animation tick when BEZ playback started

template <size_t N>
static void ResetModelSet(TmdModel (&models)[N],
                          bool (&loaded)[N],
                          std::vector<TmdVertex> (&animVerts)[N],
                          bool (&hasAnimVerts)[N]) {
    for (size_t i = 0; i < N; i++) {
        models[i] = TmdModel{};
        loaded[i] = false;
        hasAnimVerts[i] = false;
        animVerts[i].clear();
    }
}

template <size_t N>
static bool LoadTmdSet(ResourceManager* resources,
                       const char* const (&names)[N],
                       const char* logPrefix,
                       TmdModel (&models)[N],
                       bool (&loaded)[N],
                       std::vector<TmdVertex> (&animVerts)[N],
                       bool (&hasAnimVerts)[N]) {
    bool anyLoaded = false;
    for (size_t i = 0; i < N; i++) {
        loaded[i] = false;
        hasAnimVerts[i] = false;
        animVerts[i].clear();
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
        int16_t minX = 32767, maxX = -32768;
        int16_t minY = 32767, maxY = -32768;
        int16_t minZ = 32767, maxZ = -32768;
        for (const auto& obj : models[i].objects) {
            totalVerts += (int)obj.vertices.size();
            totalPrims += (int)obj.primitives.size();
            for (const auto& v : obj.vertices) {
                if (v.x < minX) minX = v.x; if (v.x > maxX) maxX = v.x;
                if (v.y < minY) minY = v.y; if (v.y > maxY) maxY = v.y;
                if (v.z < minZ) minZ = v.z; if (v.z > maxZ) maxZ = v.z;
            }
        }
        Log::Printf("%s loaded: %s objs=%d verts=%d prims=%d bbox=[%d..%d, %d..%d, %d..%d]",
                    logPrefix,
                    names[i],
                    (int)models[i].objects.size(),
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
static void RegisterModelSetTpages(const TmdModel (&models)[N], const bool (&loaded)[N]) {
    for (size_t i = 0; i < N; i++) {
        if (!loaded[i]) {
            continue;
        }
        for (const auto& obj : models[i].objects) {
            for (const auto& prim : obj.primitives) {
                if (prim.textured) {
                    s_vramAtlas.RegisterTpage(prim.tpage);
                    s_vramAtlas.RegisterClut(prim.clut);
                }
            }
        }
    }
}

template <size_t N>
static void LoadMimeResourceList(ResourceManager* resources,
                                 const char* const (&names)[N],
                                 bool isTod) {
    for (const char* name : names) {
        const std::vector<uint8_t>* mem = resources->GetMem(name);
        if (!mem || mem->empty()) {
            continue;
        }
        if (isTod) {
            MimeEngine::LoadTod(name, mem->data(), mem->size());
        } else if (std::strstr(name, ".vdf") != nullptr) {
            MimeEngine::LoadVdf(name, mem->data(), mem->size());
        } else {
            MimeEngine::LoadDat(name, mem->data(), mem->size());
        }
    }
}

void LoadModels(ResourceManager* resources) {
    if (!resources) return;
    s_anyLoaded = false;
    s_resources = resources;

    // Initialize MIMe engine
    MimeEngine::Init();
    s_mimeLoaded = false;

    ResetModelSet(s_models, s_loaded, s_animVerts, s_hasAnimVerts);
    s_anyLoaded = LoadTmdSet(resources, kTmdNames, "Scene0 TMD", s_models, s_loaded, s_animVerts, s_hasAnimVerts);

    // Load VDF resources
    LoadMimeResourceList(resources, kVdfNames, false);

    // Load DAT resources
    LoadMimeResourceList(resources, kDatNames, false);

    // Load TOD resources
    LoadMimeResourceList(resources, kTodNames, true);

    // Set up initial MIMe channels for animation sequence:
    // Phase 1: PA_OKI on chan0 (one-shot stand-up, ~32 frames)
    // Phase 2: PA_DANCE on chan0 (looping dance, 267 frames synced with TOD)
    // chan1: LOGONEW (logo fly-in, one-shot)
    // chan2: PA_L (hand gesture, looping, activated on DanceLoop)
    // chan3: HIPPOP (subtitle fly-in, one-shot)
    const VdfData* vdfOki     = MimeEngine::GetVdfByName("pa_oki.vdf");
    const DatData* datOki     = MimeEngine::GetDatByName("pa_oki.dat");
    const VdfData* vdfDance   = MimeEngine::GetVdfByName("pa_dance.vdf");
    const DatData* datDance   = MimeEngine::GetDatByName("pa_dance.dat");
    const VdfData* vdfPaL     = MimeEngine::GetVdfByName("pa_l.vdf");
    const DatData* datPaL     = MimeEngine::GetDatByName("pa_l.dat");
    const VdfData* vdfLogonew = MimeEngine::GetVdfByName("logonew.vdf");
    const DatData* datLogonew = MimeEngine::GetDatByName("logonew.dat");
    const VdfData* vdfHippop  = MimeEngine::GetVdfByName("hippop.vdf");
    const DatData* datHippop  = MimeEngine::GetDatByName("hippop.dat");

    // Start in WaitVideo phase - animation won't begin until STR video finishes
    s_animPhase = AnimPhase::WaitVideo;
    s_phaseStartTick = 0;
    s_titleFlyInActivated = false;
    if (vdfOki && datOki) {
        // PA_OKI found - will be activated when WaitVideo ends
        Log::Printf("MIMe: PA_OKI ready on chan0 (deferred until video ends, ~%d frames)", kOkiFrames);
    } else if (vdfDance && datDance) {
        // Fallback: skip stand-up, go straight to dance when video ends
        Log::Printf("MIMe: PA_OKI not found, will fallback to PA_DANCE after video");
    }
    // PSX事件5: LOGONEW(chan1)和HIPPOP(chan3)延迟到v10=318时激活
    // 不在Init时设置channel，在UpdateDynamicXforms中根据s_animFrame触发
    Log::Printf("MIMe: LOGONEW/HIPPOP deferred until tick %u (PSX event5/v10=318)", kTitleFlyInTick);

    // Set up TOD: first process pa_loc2.tod for camera/viewport setup,
    // then switch to pa_dance.tod for looping animation
    const TodData* todLoc2 = MimeEngine::GetTodByName("pa_loc2.tod");
    if (todLoc2) {
        MimeEngine::SetTod(todLoc2, false);
        MimeEngine::TickTod(); // process setup block immediately
    }

     s_savedTodCam = MimeEngine::GetTodPlayback().camera;
     s_savedTodVp = MimeEngine::GetTodPlayback().viewport;
     s_hasSavedTodView = (s_savedTodCam.valid || s_savedTodVp.valid);

     const TodData* todDance = MimeEngine::GetTodByName("pa_dance.tod");
     if (todDance) {
         Log::Printf("TOD: dance animation ready (%d blocks, start deferred)", (int)todDance->blockCount);
     }

    s_mimeLoaded = (vdfOki != nullptr || vdfDance != nullptr || vdfLogonew != nullptr || vdfHippop != nullptr);
    if (s_mimeLoaded) {
        Log::Printf("MIMe: anim resources loaded, phase=%s",
                    s_animPhase == AnimPhase::StandUp ? "StandUp" : "Dance");
    }

    // Build Scene0 VRAM texture atlas: register tpages from TMD primitives, then load TIMs.
    s_vramAtlas.Clear();
    s_atlasReady = false;
    RegisterModelSetTpages(s_models, s_loaded);
    Log::Printf("Scene0 VramAtlas: registered %d tpages from TMD models", s_vramAtlas.GetTpageCount());

    // Load TIM textures from TIM blocks into atlas.
    // We iterate all TIMs and only decode those that land in a registered tpage.
    // IMPORTANT: Skip Mem-block face TIMs (names starting with "f_") — on PSX these
    // are NOT loaded into VRAM during COMPO loading, only via face event callbacks.
    // Loading them here would overwrite the base face texture from Tim-block FRM_*.TIM.
    const std::vector<std::string> timNames = resources->GetTimRawNames();
    int timLoaded = 0, timSkipped = 0;
    for (const std::string& tn : timNames) {
        // Skip face TIMs (Mem block overlays: f_paku, f_pamel, f_pamer, f_patbl, etc.)
        if (tn.size() >= 2 && tn[0] == 'f' && tn[1] == '_') {
            timSkipped++;
            continue;
        }
        const std::vector<uint8_t>* raw = resources->GetTimRaw(tn);
        if (!raw || raw->empty()) continue;
        if (s_vramAtlas.LoadTim(raw->data(), raw->size(), tn)) {
            timLoaded++;
        }
    }
    Log::Printf("Scene0 VramAtlas: loaded %d TIM textures into %d tpages (scanned %d, skipped %d face TIMs)",
                timLoaded, s_vramAtlas.GetTpageCount(), (int)timNames.size(), timSkipped);

    // MEM TIMs are runtime upload payloads (face/UI/save effects), not static
    // atlas sources. They are loaded on demand by their direct consumers.
}

bool HasModels() { return s_anyLoaded; }

void InitCameraEvents(const uint8_t* comodData, size_t comodSize,
                      ResourceManager* resources,
                      int sceneId) {
    s_camBez = CameraBezPlayer{};
    s_comodEvents = ComodStageEventTable{};
    if (!comodData || comodSize == 0 || !resources) return;

    // Parse COMOD event table (data-driven BEZ resolution)
    s_comodEvents.Parse(comodData, comodSize);
    if (s_comodEvents.parsed) {
        // COMOD stage BEZ handles are indices into the original PSX loader's
        // mixed resource table, not ResourceManager's MEM-only insertion
        // order.  Resolving handle 65 directly through MEM therefore picked
        // BRK_MU.TMD for COMOD2 and produced a bogus camera.  The stage
        // archives carry an unambiguous CM_0N.BEZ camera for each scene; use
        // that semantic name when available, and retain the legacy handle
        // resolver only as a fallback for unknown scene layouts.
        std::string bezName;
        if (sceneId >= 1 && sceneId <= 9) {
            char sceneBez[32];
            std::snprintf(sceneBez, sizeof(sceneBez), "CM_%02d.BEZ", sceneId);
            if (resources->GetMem(sceneBez) != nullptr) {
                bezName = sceneBez;
                Log::Printf(
                    "CameraBez: scene semantic binding scene=%d name='%s'",
                    sceneId,
                    bezName.c_str());
            }
        }
        if (bezName.empty()) {
            bezName = s_comodEvents.ResolveBezName(resources);
        }
        if (!bezName.empty()) {
            const auto* bezData = resources->GetMem(bezName);
            if (bezData && !bezData->empty()) {
                // tick_division: ticks per keyframe segment
                // TODO: extract from overlay data; 10 works for S0
                constexpr int32_t kCamBezPeriod = 10;
                s_camBez.LoadBez(bezData->data(), bezData->size(), kCamBezPeriod, false);
                // Set camera immediately from BEZ entry[0] (before Start)
                s_camera.pos[0] = (float)s_camBez.posX[0];
                s_camera.pos[1] = (float)s_camBez.posY[0];
                s_camera.pos[2] = (float)s_camBez.posZ[0];
                s_camera.tgt[0] = (float)s_camBez.tgtX[0];
                s_camera.tgt[1] = (float)s_camBez.tgtY[0];
                s_camera.tgt[2] = (float)s_camBez.tgtZ[0];
                s_viewMatrixBuilt = false;
                Log::Printf("CameraBez: resolved '%s' from event handle %d",
                             bezName.c_str(),
                             s_comodEvents.events[s_comodEvents.camBezEventIdx].bezHandle);
            } else {
                Log::Printf("CameraBez: '%s' not found in resources", bezName.c_str());
            }
        }
    } else {
        // Fallback: try known BEZ names
        const auto* bezData = resources->GetMem("CM_OP.BEZ");
        if (!bezData) bezData = resources->GetMem("cm_op.bez");
        if (bezData && !bezData->empty()) {
            constexpr int32_t kCamBezPeriod = 10;
            s_camBez.LoadBez(bezData->data(), bezData->size(), kCamBezPeriod, false);
            s_camera.pos[0] = (float)s_camBez.posX[0];
            s_camera.pos[1] = (float)s_camBez.posY[0];
            s_camera.pos[2] = (float)s_camBez.posZ[0];
            s_camera.tgt[0] = (float)s_camBez.tgtX[0];
            s_camera.tgt[1] = (float)s_camBez.tgtY[0];
            s_camera.tgt[2] = (float)s_camBez.tgtZ[0];
            s_viewMatrixBuilt = false;
            Log::Printf("CameraBez: fallback loaded CM_OP.BEZ");
        }
    }
}

void InitFaceEvents(const uint8_t* comodData, size_t comodSize,
                    const std::string& jsonPath, ResourceManager* resources) {
    s_faceProcessorReady = false;
    s_scn0FaceMaxFrame = kLegacyScn0FaceMaxFrame;
    if (!comodData || comodSize == 0 || !resources) return;

    SceneEventData sceneData;
    if (!SceneEventParser::Parse(comodData, comodSize, jsonPath, sceneData)) {
        Log::Printf("InitFaceEvents: SceneEventParser::Parse failed");
        return;
    }

    // Get Mem filenames in INT insertion order (= PSX dword_80091858 index order)
    const auto& memNames = resources->GetMemNamesOrdered();
    if (memNames.empty()) {
        Log::Printf("InitFaceEvents: no Mem file names available");
        return;
    }

    // Diagnostic: print first few and face-TIM-region mem entries
    Log::Printf("InitFaceEvents: memNames count=%zu", memNames.size());
    for (size_t i = 0; i < memNames.size() && i < 5; i++) {
        Log::Printf("  mem[%zu] = %s", i, memNames[i].c_str());
    }
    // Print face TIM region (typically starts ~index 26)
    for (size_t i = 25; i < memNames.size() && i < 35; i++) {
        Log::Printf("  mem[%zu] = %s", i, memNames[i].c_str());
    }

    s_faceProcessor.Init(sceneData, memNames);
    s_faceProcessorReady = s_faceProcessor.IsValid();
    if (s_faceProcessorReady) {
        s_faceProcessor.SetSmileEyeHoldFrames(261u);
        const uint32_t parsedMax = s_faceProcessor.GetMaxFrame();
        if (parsedMax != 0u) {
            s_scn0FaceMaxFrame = parsedMax;
        }
    }
    Log::Printf("InitFaceEvents: ready=%d (events=%zu memFiles=%zu maxFrame=%u)",
                 s_faceProcessorReady ? 1 : 0,
                 sceneData.events.size(), memNames.size(),
                 (unsigned)s_scn0FaceMaxFrame);
}

void SetScn0SqevFrame(uint32_t stageFrame) {
    s_scn0SqevFrame = stageFrame;
    if (!s_loggedScn0SqevNonZero && stageFrame != 0) {
        s_loggedScn0SqevNonZero = true;
        Log::Printf("Scn0SqevFrame: first non-zero=%u", (unsigned)stageFrame);
    }
}

void ResetScn0FaceEvents() {
    s_scn0SqevFrame = 0;
    s_loggedScn0SqevNonZero = false;
    s_faceFrameAbs = 0;
    s_faceProcessor.Reset();
}

struct Vec2f { float x, y; };

struct SortedTri {
    float depth;
    int modelIdx;
    int objIdx;
    int primIdx;
    int triIdx;
};

struct TmdRenderTri {
    uint16_t vi[3] = {};
    uint8_t u[3] = {};
    uint8_t v[3] = {};
    bool textured = false;
    bool semiTransparent = false;
    uint16_t tpage = 0;
    uint16_t clut = 0;
    uint8_t abr = 0;
    uint8_t r = 0x80;
    uint8_t g = 0x80;
    uint8_t b = 0x80;
};

// ============================================================
// PSX-style 3x3 rotation + translation matrix (float version)
// Matches PSX MATRIX layout: m[3][3] rotation + t[3] translation
// ============================================================
struct Mat3x3f {
    float m[3][3];  // rotation (identity = diag 1.0)
    float t[3];     // translation

    void SetIdentity() {
        m[0][0]=1; m[0][1]=0; m[0][2]=0;
        m[1][0]=0; m[1][1]=1; m[1][2]=0;
        m[2][0]=0; m[2][1]=0; m[2][2]=1;
        t[0]=0; t[1]=0; t[2]=0;
    }

    // Build from PSX fixed-point TodCoordMatrix (4.12 format, 4096=1.0)
    void FromPsxCoord(const TodCoordMatrix& c) {
        for (int r = 0; r < 3; r++)
            for (int col = 0; col < 3; col++)
                m[r][col] = (float)c.m[r][col] / 4096.0f;
        t[0] = (float)c.t[0];
        t[1] = (float)c.t[1];
        t[2] = (float)c.t[2];
    }

    // Transform a point: result = m * p + t
    void TransformPoint(float px, float py, float pz, float& ox, float& oy, float& oz) const {
        ox = m[0][0]*px + m[0][1]*py + m[0][2]*pz + t[0];
        oy = m[1][0]*px + m[1][1]*py + m[1][2]*pz + t[1];
        oz = m[2][0]*px + m[2][1]*py + m[2][2]*pz + t[2];
    }
};

// Matrix multiply: out = a * b  (rotation and translation)
static void MatMul(const Mat3x3f& a, const Mat3x3f& b, Mat3x3f& out) {
    // Rotation: out.m = a.m * b.m
    for (int r = 0; r < 3; r++)
        for (int c = 0; c < 3; c++)
            out.m[r][c] = a.m[r][0]*b.m[0][c] + a.m[r][1]*b.m[1][c] + a.m[r][2]*b.m[2][c];
    // Translation: out.t = a.m * b.t + a.t
    for (int r = 0; r < 3; r++)
        out.t[r] = a.m[r][0]*b.t[0] + a.m[r][1]*b.t[1] + a.m[r][2]*b.t[2] + a.t[r];
}

// ============================================================
// PSX Camera: sub_801C609C hardcodes these values
// Camera matrix built by sub_80040FA0 (look-at)
// ============================================================

static Mat3x3f s_viewMatrix;  // camera view matrix (world->view)

// Build look-at view matrix from camera pos/tgt
// PSX convention: Y increases downward, +Z is into screen
static void BuildViewMatrix(const PsxCamera& cam, Mat3x3f& view) {
    float dx = cam.tgt[0] - cam.pos[0];
    float dy = cam.tgt[1] - cam.pos[1];
    float dz = cam.tgt[2] - cam.pos[2];
    float dist = std::sqrt(dx*dx + dy*dy + dz*dz);
    if (dist < 1.0f) { view.SetIdentity(); return; }

    // Forward vector (camera looks along this direction)
    float fwd[3] = { dx/dist, dy/dist, dz/dist };

    // PSX left-handed coordinate system: Y+ = down, -Y = visually up
    // WorldUp = (0,1,0) so that world -Y maps to screen top (small sy)
    float worldUp[3] = { 0.0f, 1.0f, 0.0f };

    // Right = WorldUp x Forward (left-handed convention)
    float right[3] = {
        worldUp[1]*fwd[2] - worldUp[2]*fwd[1],
        worldUp[2]*fwd[0] - worldUp[0]*fwd[2],
        worldUp[0]*fwd[1] - worldUp[1]*fwd[0]
    };
    float rlen = std::sqrt(right[0]*right[0] + right[1]*right[1] + right[2]*right[2]);
    if (rlen > 0.0001f) { right[0]/=rlen; right[1]/=rlen; right[2]/=rlen; }

    // Up = Forward x Right
    float up[3] = {
        fwd[1]*right[2] - fwd[2]*right[1],
        fwd[2]*right[0] - fwd[0]*right[2],
        fwd[0]*right[1] - fwd[1]*right[0]
    };

    // View rotation: rows = right, up, forward
    view.m[0][0]=right[0]; view.m[0][1]=right[1]; view.m[0][2]=right[2];
    view.m[1][0]=up[0];    view.m[1][1]=up[1];    view.m[1][2]=up[2];
    view.m[2][0]=fwd[0];   view.m[2][1]=fwd[1];   view.m[2][2]=fwd[2];

    // Translation = -R * pos
    float px = cam.pos[0], py = cam.pos[1], pz = cam.pos[2];
    view.t[0] = -(right[0]*px + right[1]*py + right[2]*pz);
    view.t[1] = -(up[0]*px + up[1]*py + up[2]*pz);
    view.t[2] = -(fwd[0]*px + fwd[1]*py + fwd[2]*pz);
}

// Per-model state: COORD matrix + rendering params
// ============================================================
struct ModelState {
    Mat3x3f coord;     // object COORD matrix (from TOD or init)
    Mat3x3f combined;  // view * panelCOORD * coord (precomputed)
    float   alpha = 1.0f;
    bool    enabled = false;
    uint32_t objectMask = 0xFFFFFFFFu;
    float   tintR = 1.0f;
    float   tintG = 1.0f;
    float   tintB = 1.0f;
};

static ModelState s_modelState[4];
static ModelState s_stageModelState[kStageModelCapacity];
static Mat3x3f s_panelCoord;  // panel slide COORD (identity in steady state)
static uint32_t s_animFrame = 0;  // 30Hz tick counter (matches PSX sub_80035560(2))
static uint32_t s_dbgTodPrintInterval = 10;
static float s_dbgLastTodTy = 0.0f;
static float s_dbgLastTodTz = 0.0f;
static uint32_t s_dbgLastTodFrame = 0;
static bool s_dbgLastTodValid = false;
static int s_menuPointDir = -1;  // 0=START(left), 1=MENU(right)
static bool s_menuPointTransitionActive = false;
static uint32_t s_menuPointTransitionEndTick = 0;
static int s_menuPointTransitionTarget = 0;

static auto s_lastAnimTime = std::chrono::steady_clock::now();
static double s_animAccum = 0.0;
static bool s_animClockInit = false;

static void ResetModelState(ModelState& state, bool enabled) {
    state.coord.SetIdentity();
    state.alpha = 1.0f;
    state.enabled = enabled;
    state.objectMask = 0xFFFFFFFFu;
    state.tintR = 1.0f;
    state.tintG = 1.0f;
    state.tintB = 1.0f;
}

static void ResetStageModelSet() {
    for (int i = 0; i < kStageModelCapacity; ++i) {
        s_stageModels[i] = TmdModel{};
        s_stageLoaded[i] = false;
        ResetModelState(s_stageModelState[i], false);
        s_stageAnimVerts[i].clear();
        s_stageHasAnimVerts[i] = false;
    }
    s_stageModelCount = 0;
    s_stageAnyLoaded = false;
    s_stageSceneId = -1;
}

void LoadStageModels(ResourceManager* resources, int sceneId) {
    ResetStageModelSet();
    if (resources == nullptr) {
        return;
    }

    const auto& ordered = resources->GetMemNamesOrdered();
    for (const std::string& originalName : ordered) {
        if (s_stageModelCount >= kStageModelCapacity) {
            Log::Printf("Stage TMD: capacity reached (%d), remaining models ignored",
                        kStageModelCapacity);
            break;
        }

        std::string name = originalName;
        std::transform(name.begin(), name.end(), name.begin(),
                       [](unsigned char c) {
                           return static_cast<char>(std::tolower(c));
                       });
        if (name.size() < 4 || name.rfind(".tmd") != name.size() - 4) {
            continue;
        }

        const std::vector<uint8_t>* mem = resources->GetMem(originalName);
        if (mem == nullptr || mem->empty()) {
            continue;
        }

        const int index = s_stageModelCount++;
        if (!TmdParser::Parse(mem->data(), mem->size(), s_stageModels[index])) {
            --s_stageModelCount;
            Log::Printf("Stage TMD: parse failed scene=%d name='%s' bytes=%llu",
                        sceneId, originalName.c_str(),
                        static_cast<unsigned long long>(mem->size()));
            continue;
        }

        s_stageLoaded[index] = true;
        s_stageAnyLoaded = true;
        ResetModelState(s_stageModelState[index], true);

        int vertexCount = 0;
        int primitiveCount = 0;
        for (const TmdObject& object : s_stageModels[index].objects) {
            vertexCount += static_cast<int>(object.vertices.size());
            primitiveCount += static_cast<int>(object.primitives.size());
        }
        Log::Printf("Stage TMD loaded: scene=%d name='%s' objs=%d verts=%d prims=%d",
                    sceneId, originalName.c_str(),
                    static_cast<int>(s_stageModels[index].objects.size()),
                    vertexCount, primitiveCount);
    }

    if (!s_stageAnyLoaded) {
        Log::Printf("Stage TMD: no original stage models found scene=%d",
                    sceneId);
        return;
    }

    s_stageSceneId = sceneId;
    s_vramAtlas.Clear();
    s_atlasReady = false;
    RegisterModelSetTpages(s_stageModels, s_stageLoaded);

    int timLoaded = 0;
    const std::vector<std::string> timNames = resources->GetTimRawNames();
    for (const std::string& timName : timNames) {
        const std::vector<uint8_t>* raw = resources->GetTimRaw(timName);
        if (raw == nullptr || raw->empty()) {
            continue;
        }
        if (s_vramAtlas.LoadTim(raw->data(), raw->size(), timName)) {
            ++timLoaded;
        }
    }
    Log::Printf("Stage VramAtlas: scene=%d registeredTpages=%d loadedTim=%d scannedTim=%llu",
                sceneId, s_vramAtlas.GetTpageCount(), timLoaded,
                static_cast<unsigned long long>(timNames.size()));
}

bool HasStageModels() {
    return s_stageAnyLoaded;
}

void Clear() {
    ResetModelSet(s_models, s_loaded, s_animVerts, s_hasAnimVerts);
    ResetStageModelSet();
    s_anyLoaded = false;
    s_resources = nullptr;
    s_mimeLoaded = false;
    MimeEngine::Reset();

    s_vramAtlas.Clear();
    s_atlasReady = false;

    s_animPhase = AnimPhase::WaitVideo;
    s_phaseStartTick = 0;
    s_okiFrames = kOkiFrames;
    s_danceFrames = kDanceFrames;
    s_titleFlyInActivated = false;
    s_animFrame = 0;
    s_animAccum = 0.0;
    s_animClockInit = false;

    for (ModelState& state : s_modelState) {
        ResetModelState(state, false);
    }
    s_panelCoord.SetIdentity();
    s_menuPointDir = -1;
    s_menuPointTransitionActive = false;
    s_menuPointTransitionEndTick = 0;
    s_menuPointTransitionTarget = 0;

    s_faceProcessor.Reset();
    s_faceProcessorReady = false;
    s_scn0SqevFrame = 0;
    s_loggedScn0SqevNonZero = false;
    s_faceFrameAbs = 0;
    s_scn0FaceMaxFrame = kLegacyScn0FaceMaxFrame;

    s_savedTodCam = TodCamera{};
    s_savedTodVp = TodViewport{};
    s_hasSavedTodView = false;
    s_camera = PsxCamera{{78.0f, -1437.0f, -9869.0f},
                         {0.0f, -1400.0f, 0.0f},
                         440.0f,
                         160.0f,
                         120.0f};
    s_viewMatrixBuilt = false;
    s_comodEvents = ComodStageEventTable{};
    s_camBez = CameraBezPlayer{};
    s_camBezStartTick = 0;
}

static void StartDanceTod() {
     const TodData* todDance = MimeEngine::GetTodByName("pa_dance.tod");
     if (!todDance) return;
     MimeEngine::SetTod(todDance, false);
     if (s_hasSavedTodView) {
         MimeEngine::GetTodPlayback().camera = s_savedTodCam;
         MimeEngine::GetTodPlayback().viewport = s_savedTodVp;
     }
     auto& pb = MimeEngine::GetTodPlayback();
     pb.currentBlock = 0;
     pb.frameCounter = 0xFFFFFFFFu;
     MimeEngine::TickTod();
 }

static uint32_t GetDatAnimFrames(const DatData* dat) {
    if (!dat) return 0;
    uint32_t maxFrames = 0;
    for (const auto& k : dat->keyList) {
        const uint32_t sz = (uint32_t)k.influence.size();
        if (sz > 0) {
            const uint32_t f = sz - 1;
            if (f > maxFrames) maxFrames = f;
        }
    }
    return maxFrames;
}

static uint32_t ComputeMenuIdleLoopSpeed(const DatData* dat) {
    if (!dat) return 0x100u;
    const uint32_t animFrames = GetDatAnimFrames(dat);
    if (animFrames == 0u) return 0x100u;

    size_t bgmSamples = 0;
    uint32_t bgmRate = 0;
    if (!PrSfx::GetBgmPcmInfo(bgmSamples, bgmRate) || bgmSamples == 0u || bgmRate == 0u) {
        return 0x100u;
    }

    const uint64_t num = (uint64_t)animFrames * 256ull * 4ull * (uint64_t)bgmRate;
    const uint64_t den = (uint64_t)bgmSamples * 30ull;
    if (den == 0ull) return 0x100u;

    uint64_t sp = (num + den / 2ull) / den;
    if (sp < 0x40ull) sp = 0x40ull;
    if (sp > 0x400ull) sp = 0x400ull;
    return (uint32_t)sp;
}

static void UpdateDynamicXforms(PrGameContext& ctx) {
    auto now = std::chrono::steady_clock::now();
    // --- 30Hz animation clock (PSX Scene0: sub_80035560(2) = 2 VBlanks = 30fps) ---
    // WaitVideo: activated as initial phase; transitions on first Render call.
    // Render() is only called in TitleLoop+ phases (inMenuPhase), meaning MOVIE0 is done.
    // We do NOT check IsVideoFinished() here because the StrPlayer may already be
    // playing MOVIE0T.STR (background loop), not MOVIE0.STR.
    if (s_animPhase == AnimPhase::WaitVideo) {
        // First Render call after menu phase entered → activate PA_OKI and start StandUp
        const VdfData* vdfOki = MimeEngine::GetVdfByName("pa_oki.vdf");
        const DatData* datOki = MimeEngine::GetDatByName("pa_oki.dat");
        const DatData* datDance = MimeEngine::GetDatByName("pa_dance.dat");
        if (vdfOki && datOki) {
            MimeEngine::SetChannel(0, vdfOki, datOki, 0, 0x100, false);  // one-shot
            s_animPhase = AnimPhase::StandUp;
            {
                const uint32_t fOki = GetDatAnimFrames(datOki);
                const uint32_t fDance = datDance ? GetDatAnimFrames(datDance) : 0u;
                s_okiFrames = (fOki != 0u) ? fOki : kOkiFrames;
                s_danceFrames = (fDance != 0u) ? fDance : kDanceFrames;
                Log::Printf("MIMe: anim lengths from DAT: oki=%u dance=%u", (unsigned)s_okiFrames, (unsigned)s_danceFrames);
            }
            // Start camera BEZ animation (PSX: BEZ + PA_OKI start simultaneously)
            if (s_camBez.loaded) {
                s_camBez.Start();
                s_camBezStartTick = 0;  // BEZ starts at tick 0
            }
            Log::Printf("MIMe: Menu phase entered, PA_OKI activated (StandUp)");
        } else {
            // No PA_OKI, fallback to PA_DANCE
            const VdfData* vdfDance = MimeEngine::GetVdfByName("pa_dance.vdf");
            if (vdfDance && datDance) {
                MimeEngine::SetChannel(0, vdfDance, datDance, 0, 0x100, false);
                s_animPhase = AnimPhase::Dance;
                {
                    const uint32_t fDance = GetDatAnimFrames(datDance);
                    s_okiFrames = kOkiFrames;
                    s_danceFrames = (fDance != 0u) ? fDance : kDanceFrames;
                    Log::Printf("MIMe: anim lengths from DAT: oki=%u dance=%u", (unsigned)s_okiFrames, (unsigned)s_danceFrames);
                }
                StartDanceTod();
                Log::Printf("MIMe: Menu phase entered, no PA_OKI, fallback to PA_DANCE");
            }
        }
        s_animFrame = 0;
        s_phaseStartTick = 0;
        s_animClockInit = false;  // reset clock
        s_menuPointDir = -1;
        s_menuPointTransitionActive = false;
        s_menuPointTransitionEndTick = 0;
        s_menuPointTransitionTarget = 0;
        // Don't tick animations on first frame
        goto skipAnimUpdate;
    }

    const AnimPhase phaseBeforeTransitions = s_animPhase;

    if (!s_animClockInit) {
        s_lastAnimTime = now;
        s_animClockInit = true;
    }
    double dt = 0.0;
    if (s_animClockInit) {
        dt = std::chrono::duration<double>(now - s_lastAnimTime).count();
    }
    s_lastAnimTime = now;
    // Cap dt to avoid huge jumps on breakpoints/stutters
    if (dt > 0.1) dt = 0.1;
    s_animAccum += dt;

    {
        const double kTickInterval = 1.0 / 30.0;  // 30Hz = PSX 2-VBlank tick
        int ticksThisRender = 0;
        while (s_animAccum >= kTickInterval) {
            s_animAccum -= kTickInterval;
            s_animFrame++;
            ticksThisRender++;
            // TOD advances 1 block per 30Hz tick, but STOP in MenuIdle (freeze pose)
            if (s_animPhase == AnimPhase::Dance) {
                MimeEngine::TickTod();
            }

            // Camera BEZ animation: runs across ALL phases (independent of VDF/DAT)
            if (s_camBez.active) {
                uint32_t bezTick = s_animFrame - s_camBezStartTick;
                int result = s_camBez.Tick((int)bezTick);
                if (result >= 0) {
                    s_camera.pos[0] = (float)s_camBez.outPosX;
                    s_camera.pos[1] = (float)s_camBez.outPosY;
                    s_camera.pos[2] = (float)s_camBez.outPosZ;
                    s_camera.tgt[0] = (float)s_camBez.outTgtX;
                    s_camera.tgt[1] = (float)s_camBez.outTgtY;
                    s_camera.tgt[2] = (float)s_camBez.outTgtZ;
                    s_viewMatrixBuilt = false;  // force rebuild
                }
            }

            if (s_animPhase == AnimPhase::Dance) {
                const auto& pb = MimeEngine::GetTodPlayback();
                const auto& ch0 = MimeEngine::GetChannel(0);
                uint32_t ch0Frame = 0;
                uint32_t ch0Delta = 0;
                if (ch0.active) {
                    ch0Delta = s_animFrame - ch0.startFrame;
                    ch0Frame = (ch0Delta * ch0.speed) >> 8;
                }

                bool forceLog = false;
                if (!s_dbgLastTodValid) {
                    forceLog = true;
                } else {
                    if (pb.frameCounter > s_dbgLastTodFrame + 1) forceLog = true;
                    if (std::fabs((double)pb.ty - (double)s_dbgLastTodTy) >= 512.0) forceLog = true;
                    const double absTy = std::fabs((double)pb.ty);
                    const double absLastTy = std::fabs((double)s_dbgLastTodTy);
                    if ((absTy >= 800.0 && absLastTy < 800.0) || (absTy < 800.0 && absLastTy >= 800.0)) {
                        forceLog = true;
                    }
                    if (std::fabs((double)pb.tz - (double)s_dbgLastTodTz) >= 512.0) forceLog = true;
                }

                if ((s_dbgTodPrintInterval != 0 && (s_animFrame % s_dbgTodPrintInterval) == 0) || forceLog) {
                    Log::Printf(
                        "S0 Dance anim=%u tod=%u ch0=%u d0=%u tx=%.0f ty=%.0f tz=%.0f",
                        (unsigned)s_animFrame,
                        (unsigned)pb.frameCounter,
                        (unsigned)ch0Frame,
                        (unsigned)ch0Delta,
                        (double)pb.tx,
                        (double)pb.ty,
                        (double)pb.tz);
                    s_dbgLastTodTy = pb.ty;
                    s_dbgLastTodTz = pb.tz;
                    s_dbgLastTodFrame = pb.frameCounter;
                    s_dbgLastTodValid = true;
                }
            }
        }

        if (ticksThisRender > 1 && s_animPhase == AnimPhase::Dance) {
            Log::Printf("S0 TickBurst anim=%u ticks=%d dt=%.3f", (unsigned)s_animFrame, ticksThisRender, dt);
        }
    }

    // --- Animation phase transitions ---
    // StandUp → Dance: PA_OKI finishes, switch to PA_DANCE (one-shot)
    if (s_animPhase == AnimPhase::StandUp && s_animFrame >= (s_phaseStartTick + s_okiFrames)) {
        const VdfData* vdfDance = MimeEngine::GetVdfByName("pa_dance.vdf");
        const DatData* datDance = MimeEngine::GetDatByName("pa_dance.dat");
        if (vdfDance && datDance) {
            {
                const TodPlayback& pbBefore = MimeEngine::GetTodPlayback();
                const TodObjectTransform* o1 = pbBefore.FindObject(1);
                if (o1 && o1->valid) {
                    Log::Printf("S0 StandUp->Dance preTOD frame=%u obj1.t=(%d,%d,%d)",
                                (unsigned)pbBefore.frameCounter,
                                (int)o1->matrix.t[0], (int)o1->matrix.t[1], (int)o1->matrix.t[2]);
                } else {
                    Log::Printf("S0 StandUp->Dance preTOD frame=%u obj1=(none)", (unsigned)pbBefore.frameCounter);
                }
            }
            MimeEngine::SetChannel(0, vdfDance, datDance, s_animFrame, 0x100, false);  // one-shot!
            StartDanceTod();
            {
                const TodPlayback& pbAfter = MimeEngine::GetTodPlayback();
                const TodObjectTransform* o1 = pbAfter.FindObject(1);
                if (o1 && o1->valid) {
                    Log::Printf("S0 StandUp->Dance postTOD frame=%u obj1.t=(%d,%d,%d)",
                                (unsigned)pbAfter.frameCounter,
                                (int)o1->matrix.t[0], (int)o1->matrix.t[1], (int)o1->matrix.t[2]);
                } else {
                    Log::Printf("S0 StandUp->Dance postTOD frame=%u obj1=(none)", (unsigned)pbAfter.frameCounter);
                }
            }
            Log::Printf("MIMe: StandUp->Dance at tick %d", s_animFrame);
        }
        s_animPhase = AnimPhase::Dance;
        s_phaseStartTick = s_animFrame;
    }

    // Dance → MenuIdle: PA_DANCE finishes, switch to PA_L (looping cursor-pointing)
    else if (s_animPhase == AnimPhase::Dance && s_animFrame >= (s_phaseStartTick + s_danceFrames)) {
        const uint32_t delayTicks = (ctx.scn0DanceEndDelayTicks > 0) ? (uint32_t)ctx.scn0DanceEndDelayTicks : 0u;
        if (delayTicks != 0u) {
            s_animPhase = AnimPhase::DanceEndHold;
            s_phaseStartTick = s_animFrame;
            Log::Printf("MIMe: Dance->DanceEndHold at tick %d (delayTicks=%u)", s_animFrame, (unsigned)delayTicks);
        } else {
            const int cursor = ctx.debugScn0CursorOverride ? ctx.debugScn0Cursor : ctx.scn0HiliteCursor;
            const int wantDir = (cursor != 0) ? 1 : 0;
            const char* vdfName = (wantDir == 0) ? "pa_l.vdf" : "pa_r.vdf";
            const char* datName = (wantDir == 0) ? "pa_l.dat" : "pa_r.dat";
            const VdfData* vdf = MimeEngine::GetVdfByName(vdfName);
            const DatData* dat = MimeEngine::GetDatByName(datName);
            if (vdf && dat) {
                const uint32_t speed = ComputeMenuIdleLoopSpeed(dat);
                MimeEngine::SetChannel(0, vdf, dat, s_animFrame, speed, true);  // looping
                Log::Printf("MIMe: Dance->MenuIdle at tick %d (dir=%d)", s_animFrame, wantDir);
                s_menuPointDir = wantDir;
                s_menuPointTransitionActive = false;
            }
            s_animPhase = AnimPhase::MenuIdle;
            s_phaseStartTick = s_animFrame;
        }
    }
    else if (s_animPhase == AnimPhase::DanceEndHold) {
        const uint32_t delayTicks = (ctx.scn0DanceEndDelayTicks > 0) ? (uint32_t)ctx.scn0DanceEndDelayTicks : 0u;
        if (delayTicks == 0u || s_animFrame >= (s_phaseStartTick + delayTicks)) {
            const int cursor = ctx.debugScn0CursorOverride ? ctx.debugScn0Cursor : ctx.scn0HiliteCursor;
            const int wantDir = (cursor != 0) ? 1 : 0;
            const char* vdfName = (wantDir == 0) ? "pa_l.vdf" : "pa_r.vdf";
            const char* datName = (wantDir == 0) ? "pa_l.dat" : "pa_r.dat";
            const VdfData* vdf = MimeEngine::GetVdfByName(vdfName);
            const DatData* dat = MimeEngine::GetDatByName(datName);
            if (vdf && dat) {
                const uint32_t speed = ComputeMenuIdleLoopSpeed(dat);
                MimeEngine::SetChannel(0, vdf, dat, s_animFrame, speed, true);  // looping
                Log::Printf("MIMe: DanceEndHold->MenuIdle at tick %d (delayTicks=%u dir=%d)", s_animFrame, (unsigned)delayTicks, wantDir);
                s_menuPointDir = wantDir;
                s_menuPointTransitionActive = false;
            }
            s_animPhase = AnimPhase::MenuIdle;
            s_phaseStartTick = s_animFrame;
        }
    }

    const bool enteredMenuIdle = (phaseBeforeTransitions != AnimPhase::MenuIdle && s_animPhase == AnimPhase::MenuIdle);

    if (s_animPhase == AnimPhase::MenuIdle) {
        const int cursor = ctx.debugScn0CursorOverride ? ctx.debugScn0Cursor : ctx.scn0HiliteCursor;
        const int wantDir = (cursor != 0) ? 1 : 0;

        auto setLoop = [&](int dir) {
            const char* vdfName = (dir == 0) ? "pa_l.vdf" : "pa_r.vdf";
            const char* datName = (dir == 0) ? "pa_l.dat" : "pa_r.dat";
            const VdfData* vdf = MimeEngine::GetVdfByName(vdfName);
            const DatData* dat = MimeEngine::GetDatByName(datName);
            if (vdf && dat) {
                const uint32_t speed = ComputeMenuIdleLoopSpeed(dat);
                MimeEngine::SetChannel(0, vdf, dat, s_animFrame, speed, true);
                s_menuPointDir = dir;
                s_menuPointTransitionActive = false;
            }
        };

        auto startTransition = [&](int targetDir) {
            const char* vdfName = (targetDir == 0) ? "pa_t_l.vdf" : "pa_t_r.vdf";
            const char* datName = (targetDir == 0) ? "pa_t_l.dat" : "pa_t_r.dat";
            const VdfData* vdf = MimeEngine::GetVdfByName(vdfName);
            const DatData* dat = MimeEngine::GetDatByName(datName);
            if (!vdf || !dat) {
                setLoop(targetDir);
                return;
            }
            const uint32_t frames = GetDatAnimFrames(dat);
            if (frames == 0u) {
                setLoop(targetDir);
                return;
            }
            MimeEngine::SetChannel(0, vdf, dat, s_animFrame, 0x100, false);
            s_menuPointTransitionActive = true;
            s_menuPointTransitionTarget = targetDir;
            s_menuPointTransitionEndTick = s_animFrame + frames;
            Log::Printf("MIMe: MenuIdle point transition start dir=%d frames=%u (end=%u)", targetDir, (unsigned)frames, (unsigned)s_menuPointTransitionEndTick);
        };

        if (s_menuPointDir < 0) {
            setLoop(wantDir);
        }

        if (s_menuPointTransitionActive) {
            if (wantDir != s_menuPointTransitionTarget) {
                startTransition(wantDir);
            } else if (s_animFrame >= s_menuPointTransitionEndTick) {
                setLoop(s_menuPointTransitionTarget);
                Log::Printf("MIMe: MenuIdle point transition end dir=%d", s_menuPointTransitionTarget);
            }
        } else {
            if (wantDir != s_menuPointDir) {
                startTransition(wantDir);
            }
        }
    }

    // --- PSX事件5: LOGONEW/HIPPOP标题飞入 ---
    // PSX: v10=318时通过0x400000和0x1000000标志激活chan1和chan3
    // 对应Win: s_animFrame >= kTitleFlyInTick (从StandUp起268个tick)
    if (!s_titleFlyInActivated && s_animPhase != AnimPhase::WaitVideo &&
        s_animFrame >= kTitleFlyInTick) {
        const VdfData* vdfLogonew = MimeEngine::GetVdfByName("logonew.vdf");
        const DatData* datLogonew = MimeEngine::GetDatByName("logonew.dat");
        if (vdfLogonew && datLogonew) {
            MimeEngine::SetChannel(1, vdfLogonew, datLogonew, s_animFrame, 0x100, false);
            Log::Printf("MIMe: LOGONEW fly-in activated at tick %d", s_animFrame);
        }
        const VdfData* vdfHippop = MimeEngine::GetVdfByName("hippop.vdf");
        const DatData* datHippop = MimeEngine::GetDatByName("hippop.dat");
        if (vdfHippop && datHippop) {
            MimeEngine::SetChannel(3, vdfHippop, datHippop, s_animFrame, 0x100, false);
            Log::Printf("MIMe: HIPPOP fly-in activated at tick %d", s_animFrame);
        }
        s_titleFlyInActivated = true;
    }

    // --- PSX Face Event System (parsed from COMOD*.BIN) ---
    if (s_faceProcessorReady && s_resources && s_animPhase != AnimPhase::WaitVideo) {
        // Drive face events with a monotonic absolute clock so overlay-slot restore can complete.
        // We still generate PSX-like "frame in cycle" internally from GetMaxFrame(), but we do NOT
        // feed modulo frames to the processor (doing so can prevent restore forever).
        if (s_scn0SqevFrame > s_faceFrameAbs) {
            s_faceFrameAbs = s_scn0SqevFrame;
        }

        const FaceEventMode mode = (s_animPhase == AnimPhase::MenuIdle) ? FaceEventMode::MenuIdle : FaceEventMode::Full;
        if (mode == FaceEventMode::MenuIdle) {
            const int cursor = ctx.debugScn0CursorOverride ? ctx.debugScn0Cursor : ctx.scn0HiliteCursor;
            const int wantDir = (cursor != 0) ? 1 : 0;
            s_faceProcessor.SetMenuIdlePointDir(wantDir);
        } else {
            s_faceProcessor.SetMenuIdlePointDir(-1);
        }
        bool faceChanged = false;
        auto loadTim = [&](const std::string& timName) {
            const auto* data = s_resources->GetTimRaw(timName);
            if (data && !data->empty()) {
                const bool ok = s_vramAtlas.LoadTim(data->data(), data->size(), timName);
                if (ok) {
                    faceChanged = true;
                    Log::Printf("FaceEvent: loaded TIM '%s' (%zu bytes)", timName.c_str(), data->size());
                    if (ctx.debugFaceAutoShots) {
                        ctx.debugFaceAutoShotPending = true;
                        if (ctx.debugFaceAutoShotTag.empty()) {
                            ctx.debugFaceAutoShotTag = timName;
                        } else {
                            if (ctx.debugFaceAutoShotTag.size() < 140) {
                                ctx.debugFaceAutoShotTag += "+";
                                ctx.debugFaceAutoShotTag += timName;
                            }
                        }
                    }
                } else {
                    Log::Printf("FaceEvent: TIM '%s' atlas load failed (%zu bytes)", timName.c_str(), data->size());
                }
            } else {
                Log::Printf("FaceEvent: TIM '%s' NOT FOUND in timRaw!", timName.c_str());
            }
        };

        s_faceProcessor.Update(s_faceFrameAbs, mode, loadTim);
        if (enteredMenuIdle) {
            Log::Printf("FaceEvent: entered MenuIdle, restore base (faceAbs=%u)", (unsigned)s_faceFrameAbs);
            s_faceProcessor.RestoreBase(loadTim);
        }
        if (faceChanged && ctx.renderer) {
            Log::Printf("FaceEvent: faceChanged=true, calling UploadAll");
            s_vramAtlas.UploadAll(ctx.renderer);
        }
    } else if (s_animFrame % 60 == 0) {
        // Periodic debug: why face events are not running
        Log::Printf("FaceEvent DEBUG: ready=%d resources=%d phase=%d frame=%d sqev=%u",
                      s_faceProcessorReady ? 1 : 0, s_resources ? 1 : 0,
                     (int)s_animPhase, s_animFrame, (unsigned)s_scn0SqevFrame);
    }

skipAnimUpdate:

    // Build view matrix once
    if (!s_viewMatrixBuilt) {
        BuildViewMatrix(s_camera, s_viewMatrix);
        s_viewMatrixBuilt = true;
        Log::Printf("Camera: pos=(%.0f,%.0f,%.0f) tgt=(%.0f,%.0f,%.0f) proj=%.0f",
            s_camera.pos[0], s_camera.pos[1], s_camera.pos[2],
            s_camera.tgt[0], s_camera.tgt[1], s_camera.tgt[2], s_camera.projDist);
        Log::Printf("ViewMatrix: [%.4f %.4f %.4f | %.4f %.4f %.4f | %.4f %.4f %.4f] t=(%.1f,%.1f,%.1f)",
            s_viewMatrix.m[0][0], s_viewMatrix.m[0][1], s_viewMatrix.m[0][2],
            s_viewMatrix.m[1][0], s_viewMatrix.m[1][1], s_viewMatrix.m[1][2],
            s_viewMatrix.m[2][0], s_viewMatrix.m[2][1], s_viewMatrix.m[2][2],
            s_viewMatrix.t[0], s_viewMatrix.t[1], s_viewMatrix.t[2]);
    }

    // Update panel COORD from menu slide animation
    // PSX: sub_801C5D8C(-163, 204, 192) slides the panel during menu page transitions
    // Panel offset is in screen-space pixels; convert to world-space translation
    // At projDist=440 and z~9875, world_per_pixel = z/projDist ~ 22.4
    s_panelCoord.SetIdentity();
    if (ctx.scn0PanelAnimActive) {
        float worldPerPixel = 9875.0f / s_camera.projDist;
        s_panelCoord.t[0] = ctx.scn0PanelOffsetX * worldPerPixel;
        s_panelCoord.t[1] = ctx.scn0PanelOffsetY * worldPerPixel;
    }
    const TodPlayback& tod = MimeEngine::GetTodPlayback();

    // --- Initialize model COORDs ---
    // HP (index 0): flat decoration, COORD = identity
    ResetModelState(s_modelState[0], s_titleFlyInActivated);

    // LO (index 1): flat logo, COORD t[1]=-100 from PSX dword_801CC6D8
    // LO is associated with COORD at 801CC6BC via sub_8001AF1C(dword_8009186C,...)
    ResetModelState(s_modelState[1], s_titleFlyInActivated);
    s_modelState[1].coord.t[1] = -100.0f;

    // PA (index 2): character, COORD from TOD type4 (objId=1)
    // pa_loc2.tod sets: identity rotation + t=(0,0,800)
    // pa_dance.tod updates rotation per frame
    ResetModelState(s_modelState[2], true);
    if (tod.active) {
        const TodObjectTransform* obj1 = tod.FindObject(1);
        if (obj1 && obj1->valid) {
            s_modelState[2].coord.FromPsxCoord(obj1->matrix);
        } else {
            s_modelState[2].coord.t[2] = 800.0f;
        }
    } else {
        s_modelState[2].coord.t[2] = 800.0f;
    }
    s_modelState[2].enabled = true;

    // PA_KAGE (index 3): shadow - flatten Y to project onto ground plane
    // PSX shadow technique: zero out Y rotation row, set t[1] to feet level
    // PA feet at model y≈6; shadow projects all verts to this Y plane
    ResetModelState(s_modelState[3], true);
    {
        // PSX (SCUS_941.83): sub_801C5D8C(-163, 204, 192) builds a shear projection:
        //  sx = -a1/a2, sz = -a3/a2, projecting along (a1,a2,a3) onto ground plane.
        constexpr float sx = 163.0f / 204.0f;
        constexpr float sz = -192.0f / 204.0f;
        constexpr float groundY = 6.0f;

        const Mat3x3f& pa = s_modelState[2].coord;
        Mat3x3f sh = pa;

        // Apply world-space projection P * pa, where:
        // x' = x + sx*(y-groundY), z' = z + sz*(y-groundY), y' = groundY
        for (int c = 0; c < 3; c++) {
            sh.m[0][c] = pa.m[0][c] + sx * pa.m[1][c];
            sh.m[2][c] = pa.m[2][c] + sz * pa.m[1][c];
            sh.m[1][c] = 0.0f;
        }
        sh.t[0] = pa.t[0] + sx * (pa.t[1] - groundY);
        sh.t[1] = groundY;
        sh.t[2] = pa.t[2] + sz * (pa.t[1] - groundY);

        s_modelState[3].coord = sh;
    }
    s_modelState[3].alpha = 0.30f;

    // Precompute combined transform: view * panel * coord
    // In PSX: Mfinal = CameraMatrix * PanelCOORD * ObjectCOORD
    // sub_80041A68 multiplies panel by camera; sub_801C5E60 multiplies result by object
    Mat3x3f viewPanel;
    MatMul(s_viewMatrix, s_panelCoord, viewPanel);
    for (int i = 0; i < kTmdCount; i++) {
        MatMul(viewPanel, s_modelState[i].coord, s_modelState[i].combined);
    }

    // One-time position debug log (after panel animation completes)
    static bool s_posLogged = false;
    if (!s_posLogged && s_animFrame > 2 && !ctx.scn0PanelAnimActive) {
        s_posLogged = true;
        const char* names[] = {"HP","LO","PA","PA_KAGE"};
        for (int i = 0; i < kTmdCount; i++) {
            if (!s_loaded[i]) continue;
            // Compute screen center from model bbox center
            const auto& objs = s_models[i].objects;
            if (objs.empty()) continue;
            int16_t mnX=32767,mxX=-32768,mnY=32767,mxY=-32768,mnZ=32767,mxZ=-32768;
            for (auto& v : objs[0].vertices) {
                if(v.x<mnX)mnX=v.x; if(v.x>mxX)mxX=v.x;
                if(v.y<mnY)mnY=v.y; if(v.y>mxY)mxY=v.y;
                if(v.z<mnZ)mnZ=v.z; if(v.z>mxZ)mxZ=v.z;
            }
            float cx=(mnX+mxX)/2.0f, cy=(mnY+mxY)/2.0f, cz=(mnZ+mxZ)/2.0f;
            float vx,vy,vz;
            s_modelState[i].combined.TransformPoint(cx,cy,cz,vx,vy,vz);
            float sx = (vz>10.0f) ? vx*s_camera.projDist/vz + s_camera.screenCX : -1;
            float sy = (vz>10.0f) ? vy*s_camera.projDist/vz + s_camera.screenCY : -1;
            Log::Printf("Model[%d] %s: coord_t=(%.0f,%.0f,%.0f) bbox_center=(%.0f,%.0f,%.0f) view=(%.0f,%.0f,%.0f) screen=(%.1f,%.1f)",
                i, names[i], s_modelState[i].coord.t[0], s_modelState[i].coord.t[1], s_modelState[i].coord.t[2],
                cx, cy, cz, vx, vy, vz, sx, sy);
        }
    }

    // Apply MIMe vertex deformation per model with correct channel masks
    // HP (model 0): chan3 = HIPPOP
    // LO (model 1): chan1 = LOGONEW
    // PA (model 2): chan0 only (PA_OKI / PA_DANCE / PA_L depending on phase)
    //               PA_L is only on chan0 during MenuIdle, never simultaneously with dance
    if (s_mimeLoaded) {
        // Sub-frame interpolation: derive from s_animAccum (time remaining until next 30Hz tick)
        // 30fps: s_animAccum ≈ 0 after each tick → subFrame8 ≈ 0 (PSX-exact behavior)
        // 60fps: intermediate frame has s_animAccum ≈ 16ms → subFrame8 ≈ 128 (half-frame interpolation)
        const double kTickInterval = 1.0 / 30.0;
        double frac = s_animAccum / kTickInterval;
        if (frac < 0.0) frac = 0.0;
        if (frac > 1.0) frac = 1.0;
        uint32_t subFrame8 = (uint32_t)(frac * 255.0);

        if (s_loaded[0] && !s_models[0].objects.empty()) {
            const auto& baseObj = s_models[0].objects[0];
            MimeEngine::ApplyDeformation(baseObj.vertices, s_animVerts[0], s_animFrame, 1u << 3, subFrame8);
            s_hasAnimVerts[0] = true;
        }
        if (s_loaded[1] && !s_models[1].objects.empty()) {
            const auto& baseObj = s_models[1].objects[0];
            MimeEngine::ApplyDeformation(baseObj.vertices, s_animVerts[1], s_animFrame, 1u << 1, subFrame8);
            s_hasAnimVerts[1] = true;
        }
        if (s_loaded[2] && !s_models[2].objects.empty()) {
            const auto& baseObj = s_models[2].objects[0];
            // Only chan0 for PA - all animation phases use chan0 exclusively
            MimeEngine::ApplyDeformation(baseObj.vertices, s_animVerts[2], s_animFrame, 1u << 0, subFrame8);
            s_hasAnimVerts[2] = true;
            if (s_loaded[3] && !s_models[3].objects.empty()) {
                s_animVerts[3] = s_animVerts[2];
                s_hasAnimVerts[3] = true;
            }
        }
    }
}

// PSX-style vertex transform + perspective projection
// Uses precomputed combined matrix (view * panel * coord)
static Vec2f ProjectVertex(const TmdVertex& v,
                           const Mat3x3f& combined,
                           const PsxCamera& camera,
                           float& outZ) {
    float vx = (float)v.x;
    float vy = (float)v.y;
    float vz = (float)v.z;

    // Transform: view_pos = combined.m * vertex + combined.t
    float cx, cy, cz;
    combined.TransformPoint(vx, vy, vz, cx, cy, cz);

    outZ = cz;
    if (cz < 10.0f) cz = 10.0f;

    // PSX GTE perspective: sx = x * h / z + 160, sy = y * h / z + 120
    float scale = camera.projDist / cz;
    return { cx * scale + camera.screenCX, cy * scale + camera.screenCY };
}

// Matches the PSX GTE NCLIP sign used by GsTMDfast*: packets are emitted only
// when MAC0/NCLIP is positive after RTPT.
static float Cross2D(Vec2f a, Vec2f b, Vec2f c) {
    return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
}

static bool PassesGsTmdFastNclip(Vec2f p0, Vec2f p1, Vec2f p2) {
    return Cross2D(p0, p1, p2) > 0.0f;
}

static int GetTmdPrimitiveTriangleCount(const TmdPrimitive& prim) {
    return prim.quad ? 2 : 1;
}

static bool BuildTmdPrimitiveTriangle(const TmdPrimitive& prim,
                                      int triIndex,
                                      TmdRenderTri& out) {
    out = TmdRenderTri{};
    out.textured = prim.textured;
    out.semiTransparent = prim.semiTransparent;
    out.tpage = prim.tpage;
    out.clut = prim.clut;
    out.abr = prim.abr;
    out.r = prim.r;
    out.g = prim.g;
    out.b = prim.b;

    if (!prim.quad) {
        if (triIndex != 0) {
            return false;
        }
        out.vi[0] = prim.v0_idx;
        out.vi[1] = prim.v1_idx;
        out.vi[2] = prim.v2_idx;
        out.u[0] = prim.u0;
        out.v[0] = prim.v0;
        out.u[1] = prim.u1;
        out.v[1] = prim.v1;
        out.u[2] = prim.u2;
        out.v[2] = prim.v2;
        return true;
    }

    if (triIndex == 0) {
        out.vi[0] = prim.v0_idx;
        out.vi[1] = prim.v1_idx;
        out.vi[2] = prim.v2_idx;
        out.u[0] = prim.u0;
        out.v[0] = prim.v0;
        out.u[1] = prim.u1;
        out.v[1] = prim.v1;
        out.u[2] = prim.u2;
        out.v[2] = prim.v2;
        return true;
    }
    if (triIndex == 1) {
        out.vi[0] = prim.v1_idx;
        out.vi[1] = prim.v3_idx;
        out.vi[2] = prim.v2_idx;
        out.u[0] = prim.u1;
        out.v[0] = prim.v1;
        out.u[1] = prim.u3;
        out.v[1] = prim.v3;
        out.u[2] = prim.u2;
        out.v[2] = prim.v2;
        return true;
    }
    return false;
}

static float ResolveTmdPrimitiveAlpha(float modelAlpha, const TmdRenderTri& tri) {
    if (!tri.semiTransparent) {
        return modelAlpha;
    }

    switch (tri.abr & 0x03u) {
        case 1u:
        case 2u:
            return modelAlpha;
        case 3u:
            return modelAlpha * 0.25f;
        case 0u:
        default:
            return modelAlpha * 0.5f;
    }
}

static D3D11Renderer::BlendMode ResolveTmdPrimitiveBlendMode(const TmdRenderTri& tri) {
    if (!tri.semiTransparent) {
        return D3D11Renderer::BlendMode::Alpha;
    }

    switch (tri.abr & 0x03u) {
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

static uint32_t MakeTmdTextureBatchKey(uint16_t tpage,
                                       uint16_t clut,
                                       D3D11Renderer::BlendMode blend) {
    const uint32_t blendBits = static_cast<uint32_t>(blend) & 0x03u;
    return (uint32_t)(tpage & 0x01FFu) |
           ((uint32_t)clut << 9) |
           (blendBits << 25);
}

static uint16_t TmdTextureBatchKeyTpage(uint32_t key) {
    return (uint16_t)(key & 0x01FFu);
}

static uint16_t TmdTextureBatchKeyClut(uint32_t key) {
    return (uint16_t)((key >> 9) & 0xFFFFu);
}

static D3D11Renderer::BlendMode TmdTextureBatchKeyBlend(uint32_t key) {
    const uint32_t blendBits = (key >> 25) & 0x03u;
    if (blendBits == static_cast<uint32_t>(D3D11Renderer::BlendMode::Additive)) {
        return D3D11Renderer::BlendMode::Additive;
    }
    if (blendBits == static_cast<uint32_t>(D3D11Renderer::BlendMode::Subtractive)) {
        return D3D11Renderer::BlendMode::Subtractive;
    }
    return D3D11Renderer::BlendMode::Alpha;
}

static void RenderModelSet(PrGameContext& ctx,
                           float viewX,
                           float viewY,
                           float viewScale,
                           const PsxCamera& camera,
                           const TmdModel* models,
                           const bool* loaded,
                           const ModelState* modelState,
                           const std::vector<TmdVertex>* animVerts,
                           const bool* hasAnimVerts,
                           int modelCount,
                           int shadowModelIndex,
                           bool darkenBrightFallbacks) {
    // Collect triangles with depth for sorting
    std::vector<SortedTri> sortList;
    sortList.reserve(1024);

    for (int mi = 0; mi < modelCount; mi++) {
        if (!loaded[mi] || !modelState[mi].enabled) continue;
        const TmdModel& model = models[mi];
        const Mat3x3f& combined = modelState[mi].combined;
        for (int oi = 0; oi < (int)model.objects.size(); oi++) {
            if (oi < 32 && ((modelState[mi].objectMask & (1u << oi)) == 0u)) {
                continue;
            }
            const TmdObject& obj = model.objects[oi];
            const std::vector<TmdVertex>& verts =
                (hasAnimVerts[mi] && oi == 0 && !animVerts[mi].empty())
                ? animVerts[mi] : obj.vertices;
            for (int pi = 0; pi < (int)obj.primitives.size(); pi++) {
                const TmdPrimitive& prim = obj.primitives[pi];
                const int triCount = GetTmdPrimitiveTriangleCount(prim);
                for (int ti = 0; ti < triCount; ++ti) {
                    TmdRenderTri tri{};
                    if (!BuildTmdPrimitiveTriangle(prim, ti, tri)) {
                        continue;
                    }
                    if (tri.vi[0] >= verts.size() ||
                        tri.vi[1] >= verts.size() ||
                        tri.vi[2] >= verts.size()) {
                        continue;
                    }

                    float z0, z1, z2;
                    ProjectVertex(verts[tri.vi[0]], combined, camera, z0);
                    ProjectVertex(verts[tri.vi[1]], combined, camera, z1);
                    ProjectVertex(verts[tri.vi[2]], combined, camera, z2);
                    sortList.push_back({z0 + z1 + z2, mi, oi, pi, ti});
                }
            }
        }
    }

    // Sort back-to-front (larger z = farther = draw first)
    std::sort(sortList.begin(), sortList.end(),
              [](const SortedTri& a, const SortedTri& b) { return a.depth > b.depth; });

    // Upload VRAM atlas textures to GPU if needed
    if (!s_atlasReady) {
        s_vramAtlas.UploadAll(ctx.renderer);
        s_atlasReady = true;
    }

    // Keep the global back-to-front order while still batching adjacent
    // triangles that use the same pipeline state.  Previously all textured
    // triangles were rendered first and all flat-colour fallbacks afterwards.
    // CAR.TMD contains legitimate black, untextured faces; that regrouping
    // put those faces over the characters and made them appear/disappear as
    // the camera moved.
    struct OrderedItem {
        bool textured = false;
        ID3D11ShaderResourceView* texture = nullptr;
        D3D11Renderer::BlendMode blend = D3D11Renderer::BlendMode::Alpha;
        TexturedVertex texturedVertices[3]{};
        ColorVertex colorVertices[3]{};
    };
    std::vector<OrderedItem> ordered;
    ordered.reserve(sortList.size());

    std::unordered_map<uint32_t, std::vector<TexturedVertex>> shadowTexBatches;
    std::vector<ColorVertex> shadowFallbackBatch;

    for (const auto& st : sortList) {
        const TmdObject& obj = models[st.modelIdx].objects[st.objIdx];
        const TmdPrimitive& prim = obj.primitives[st.primIdx];
        TmdRenderTri tri{};
        if (!BuildTmdPrimitiveTriangle(prim, st.triIdx, tri)) {
            continue;
        }
        const ModelState& ms = modelState[st.modelIdx];
        const std::vector<TmdVertex>& verts =
            (hasAnimVerts[st.modelIdx] && st.objIdx == 0 && !animVerts[st.modelIdx].empty())
            ? animVerts[st.modelIdx] : obj.vertices;

        float z0, z1, z2;
        if (tri.vi[0] >= verts.size() ||
            tri.vi[1] >= verts.size() ||
            tri.vi[2] >= verts.size()) {
            continue;
        }
        Vec2f p0 = ProjectVertex(verts[tri.vi[0]], ms.combined, camera, z0);
        Vec2f p1 = ProjectVertex(verts[tri.vi[1]], ms.combined, camera, z1);
        Vec2f p2 = ProjectVertex(verts[tri.vi[2]], ms.combined, camera, z2);

        if (z0 < 10.0f && z1 < 10.0f && z2 < 10.0f) continue;

        if (!PassesGsTmdFastNclip(p0, p1, p2)) continue;

        float sx0 = viewX + p0.x * viewScale;
        float sy0 = viewY + p0.y * viewScale;
        float sx1 = viewX + p1.x * viewScale;
        float sy1 = viewY + p1.y * viewScale;
        float sx2 = viewX + p2.x * viewScale;
        float sy2 = viewY + p2.y * viewScale;

        // Primitive color modulation (PSX range 0-128 = full brightness)
        float r = (float)tri.r / 128.0f;
        float g = (float)tri.g / 128.0f;
        float b = (float)tri.b / 128.0f;
        if (r > 1.0f) r = 1.0f;
        if (g > 1.0f) g = 1.0f;
        if (b > 1.0f) b = 1.0f;
        r = (std::clamp)(r * ms.tintR, 0.0f, 1.0f);
        g = (std::clamp)(g * ms.tintG, 0.0f, 1.0f);
        b = (std::clamp)(b * ms.tintB, 0.0f, 1.0f);
        const D3D11Renderer::BlendMode blend = ResolveTmdPrimitiveBlendMode(tri);
        float a = ResolveTmdPrimitiveAlpha(ms.alpha, tri);

        // Shadow model darkening
        if (st.modelIdx == shadowModelIndex) {
            r *= 0.15f; g *= 0.15f; b *= 0.15f;
        }

        const bool isShadow = (st.modelIdx == shadowModelIndex);

        ID3D11ShaderResourceView* tpageSrv =
            tri.textured ? s_vramAtlas.GetTpageSRV(tri.tpage, tri.clut, ctx.renderer) : nullptr;
        if (tpageSrv) {
            float u0, v0, u1, v1, u2, v2;
            PsxVramAtlas::UVtoNormalized(tri.u[0], tri.v[0], u0, v0);
            PsxVramAtlas::UVtoNormalized(tri.u[1], tri.v[1], u1, v1);
            PsxVramAtlas::UVtoNormalized(tri.u[2], tri.v[2], u2, v2);

            const uint32_t batchKey = MakeTmdTextureBatchKey(tri.tpage, tri.clut, blend);
            if (isShadow) {
                auto& batch = shadowTexBatches[batchKey];
                batch.push_back({sx0, sy0, u0, v0, r, g, b, a});
                batch.push_back({sx1, sy1, u1, v1, r, g, b, a});
                batch.push_back({sx2, sy2, u2, v2, r, g, b, a});
            } else {
                OrderedItem item;
                item.textured = true;
                item.texture = tpageSrv;
                item.blend = blend;
                item.texturedVertices[0] = {sx0, sy0, u0, v0, r, g, b, a};
                item.texturedVertices[1] = {sx1, sy1, u1, v1, r, g, b, a};
                item.texturedVertices[2] = {sx2, sy2, u2, v2, r, g, b, a};
                ordered.push_back(item);
            }
        } else {
            if (!isShadow && darkenBrightFallbacks && (st.modelIdx == 0 || st.modelIdx == 1)) {
                float lum = r * 0.299f + g * 0.587f + b * 0.114f;
                if (lum > 0.85f) { r *= 0.65f; g *= 0.65f; b *= 0.65f; }
            }
            if (isShadow) {
                shadowFallbackBatch.push_back({sx0, sy0, r, g, b, a});
                shadowFallbackBatch.push_back({sx1, sy1, r, g, b, a});
                shadowFallbackBatch.push_back({sx2, sy2, r, g, b, a});
            } else {
                OrderedItem item;
                item.blend = blend;
                item.colorVertices[0] = {sx0, sy0, r, g, b, a};
                item.colorVertices[1] = {sx1, sy1, r, g, b, a};
                item.colorVertices[2] = {sx2, sy2, r, g, b, a};
                ordered.push_back(item);
            }
        }
    }

    if (!shadowTexBatches.empty() || !shadowFallbackBatch.empty()) {
        ctx.renderer->BeginShadowStencil();
        for (auto& [batchKey, batch] : shadowTexBatches) {
            if (batch.empty()) continue;
            ID3D11ShaderResourceView* srv =
                s_vramAtlas.GetTpageSRV(TmdTextureBatchKeyTpage(batchKey),
                                        TmdTextureBatchKeyClut(batchKey),
                                        ctx.renderer);
            if (srv) {
                ctx.renderer->DrawTexturedTriangleBatch(srv,
                                                        batch.data(),
                                                        (int)batch.size(),
                                                        TmdTextureBatchKeyBlend(batchKey));
            }
        }
        if (!shadowFallbackBatch.empty()) {
            ctx.renderer->DrawTriangleBatch(shadowFallbackBatch.data(), (int)shadowFallbackBatch.size());
        }
        ctx.renderer->EndShadowStencil();
    }

    // Flush only adjacent runs.  Reordering by texture or by fallback type
    // would recreate the priority bug above.
    bool haveRun = false;
    bool runTextured = false;
    ID3D11ShaderResourceView* runTexture = nullptr;
    D3D11Renderer::BlendMode runBlend = D3D11Renderer::BlendMode::Alpha;
    std::vector<TexturedVertex> runTexturedVertices;
    std::vector<ColorVertex> runColorVertices;
    runTexturedVertices.reserve(4096);
    runColorVertices.reserve(4096);
    auto flushRun = [&]() {
        if (!haveRun) return;
        if (runTextured) {
            ctx.renderer->DrawTexturedTriangleBatch(
                runTexture, runTexturedVertices.data(),
                static_cast<int>(runTexturedVertices.size()), runBlend);
            runTexturedVertices.clear();
        } else {
            ctx.renderer->DrawTriangleBatch(
                runColorVertices.data(), static_cast<int>(runColorVertices.size()), runBlend);
            runColorVertices.clear();
        }
        haveRun = false;
        runTexture = nullptr;
    };
    for (const auto& item : ordered) {
        const bool sameRun = haveRun && runTextured == item.textured &&
                             (!item.textured || runTexture == item.texture) &&
                             runBlend == item.blend;
        const size_t currentVertices = item.textured ? runTexturedVertices.size()
                                                      : runColorVertices.size();
        if (!sameRun || currentVertices + 3u > 4096u) flushRun();
        if (!haveRun) {
            haveRun = true;
            runTextured = item.textured;
            runTexture = item.texture;
            runBlend = item.blend;
        }
        if (item.textured) {
            runTexturedVertices.insert(runTexturedVertices.end(),
                                       std::begin(item.texturedVertices),
                                       std::end(item.texturedVertices));
        } else {
            runColorVertices.insert(runColorVertices.end(),
                                    std::begin(item.colorVertices),
                                    std::end(item.colorVertices));
        }
    }
    flushRun();
}

bool RenderStage(PrGameContext& ctx, float viewX, float viewY,
                 float viewScale) {
    if (!ctx.renderer || !s_stageAnyLoaded || s_stageModelCount <= 0) {
        return false;
    }

    // Stage TMDs carry the original world coordinates.  Their COMOD camera
    // (when present) is installed by InitCameraEvents; rebuild the look-at
    // matrix here so a scene transition cannot retain Scene0's view state.
    BuildViewMatrix(s_camera, s_viewMatrix);

    // The PSX camera is expressed in a 320x240 logical viewport.  Scene0
    // callers already pass the window-fit viewport, while the stage runners
    // intentionally pass the logical origin/scale.  Apply the same fit here
    // so native stage geometry is not confined to the upper-left quarter of
    // a 640x480 window.
    if (ctx.renderer) {
        const float winW = static_cast<float>(ctx.renderer->GetWidth());
        const float winH = static_cast<float>(ctx.renderer->GetHeight());
        const float fitX = winW / 320.0f;
        const float fitY = winH / 240.0f;
        const float fit = (std::min)(fitX, fitY);
        viewX += (winW - 320.0f * fit) * 0.5f;
        viewY += (winH - 240.0f * fit) * 0.5f;
        viewScale *= fit;
    }
    s_panelCoord.SetIdentity();
    for (int i = 0; i < s_stageModelCount; ++i) {
        if (!s_stageLoaded[i]) {
            continue;
        }
        MatMul(s_viewMatrix, s_panelCoord,
               s_stageModelState[i].combined);
    }

    RenderModelSet(ctx,
                   viewX,
                   viewY,
                   viewScale,
                   s_camera,
                   s_stageModels,
                   s_stageLoaded,
                   s_stageModelState,
                   s_stageAnimVerts,
                   s_stageHasAnimVerts,
                   s_stageModelCount,
                   -1,
                   false);
    return true;
}

void Render(PrGameContext& ctx, float viewX, float viewY, float viewScale) {
    if (!ctx.renderer || !s_anyLoaded) return;

    // Update dynamic transforms based on cursor position
    UpdateDynamicXforms(ctx);

    // Don't render models during WaitVideo phase (STR video still playing)
    if (s_animPhase == AnimPhase::WaitVideo) return;

    RenderModelSet(ctx,
                   viewX,
                   viewY,
                   viewScale,
                   s_camera,
                   s_models,
                   s_loaded,
                   s_modelState,
                   s_animVerts,
                   s_hasAnimVerts,
                   kTmdCount,
                   3,
                   true);
}

std::string DumpStats() {
    std::string out;
    out += "Scene0 TMD Models:\n";
    for (int i = 0; i < kTmdCount; i++) {
        out += "  [" + std::to_string(i) + "] " + kTmdNames[i] + ": ";
        if (!s_loaded[i]) { out += "not loaded\n"; continue; }
        const auto& ms = s_modelState[i];
        int tv = 0, tp = 0;
        int16_t mnX=32767, mxX=-32768, mnY=32767, mxY=-32768, mnZ=32767, mxZ=-32768;
        for (const auto& obj : s_models[i].objects) {
            tv += (int)obj.vertices.size();
            tp += (int)obj.primitives.size();
            for (const auto& v : obj.vertices) {
                if (v.x<mnX) mnX=v.x; if (v.x>mxX) mxX=v.x;
                if (v.y<mnY) mnY=v.y; if (v.y>mxY) mxY=v.y;
                if (v.z<mnZ) mnZ=v.z; if (v.z>mxZ) mxZ=v.z;
            }
        }
        out += "objs=" + std::to_string(s_models[i].objects.size())
             + " v=" + std::to_string(tv) + " p=" + std::to_string(tp)
             + " bbox=[" + std::to_string(mnX) + ".." + std::to_string(mxX)
             + "," + std::to_string(mnY) + ".." + std::to_string(mxY)
             + "," + std::to_string(mnZ) + ".." + std::to_string(mxZ) + "]"
             + " enabled=" + (ms.enabled ? "Y" : "N")
             + " anim=" + (s_hasAnimVerts[i] ? "Y" : "N")
             + " coord_t=(" + std::to_string((int)ms.coord.t[0]) + "," + std::to_string((int)ms.coord.t[1]) + "," + std::to_string((int)ms.coord.t[2]) + ")\n";
    }
    out += "\n" + MimeEngine::DumpInfo();
    return out;
}

// ============================================================
// Scene0 menu overlay quad rendering.
// Replicates sub_80013D10 (create quad mesh from TIM dimensions)
// + sub_80014050 (render through camera projection)
// ============================================================
void SubmitOverlayQuad(PrGameContext& ctx, const OverlayQuadParams& params,
                       float viewX, float viewY, float viewScale) {
    if (!ctx.resources || !ctx.renderer) return;

    // 1) Get TIM texture SRV and dimensions (data-driven from COMPO00.INT)
    ID3D11ShaderResourceView* srv = ctx.resources->GetTextureView(params.textureName);
    if (!srv) return;
    TextureResource* tr = ctx.resources->GetTexture(params.textureName);
    if (!tr || tr->tim.width == 0 || tr->tim.height == 0) return;

    float texW = (float)tr->tim.width;
    float texH = (float)tr->tim.height;
    float px = params.screenX;
    if (px < 0.0f) {
        px = (320.0f - texW) * 0.5f;
    }
    float py = params.screenY;

    if (!s_viewMatrixBuilt) {
        BuildViewMatrix(s_camera, s_viewMatrix);
        s_viewMatrixBuilt = true;
    }

    Mat3x3f viewPanel;
    MatMul(s_viewMatrix, s_panelCoord, viewPanel);

    auto InverseTransformPoint = [](const Mat3x3f& m, float px, float py, float pz,
                                    float& ox, float& oy, float& oz) {
        float x = px - m.t[0];
        float y = py - m.t[1];
        float z = pz - m.t[2];
        ox = m.m[0][0] * x + m.m[1][0] * y + m.m[2][0] * z;
        oy = m.m[0][1] * x + m.m[1][1] * y + m.m[2][1] * z;
        oz = m.m[0][2] * x + m.m[1][2] * y + m.m[2][2] * z;
    };

    auto ProjectPoint = [&](float lx, float ly, float lz, float& outZ) -> Vec2f {
        float cx, cy, cz;
        viewPanel.TransformPoint(lx, ly, lz, cx, cy, cz);
        outZ = cz;
        if (cz < 10.0f) cz = 10.0f;
        float scale = s_camera.projDist / cz;
        return { cx * scale + s_camera.screenCX, cy * scale + s_camera.screenCY };
    };

    constexpr float kOverlayViewZ = 9875.0f;
    auto MakeLocalFromScreen = [&](float sx, float sy, float& lx, float& ly, float& lz) {
        float vx = (sx - s_camera.screenCX) * kOverlayViewZ / s_camera.projDist;
        float vy = (sy - s_camera.screenCY) * kOverlayViewZ / s_camera.projDist;
        float vz = kOverlayViewZ;
        InverseTransformPoint(viewPanel, vx, vy, vz, lx, ly, lz);
    };

    float lx0, ly0, lz0;
    float lx1, ly1, lz1;
    float lx2, ly2, lz2;
    float lx3, ly3, lz3;
    MakeLocalFromScreen(px,         py,         lx0, ly0, lz0);
    MakeLocalFromScreen(px + texW,  py,         lx1, ly1, lz1);
    MakeLocalFromScreen(px + texW,  py + texH,  lx2, ly2, lz2);
    MakeLocalFromScreen(px,         py + texH,  lx3, ly3, lz3);

    float tmpZ = 0.0f;
    Vec2f p0 = ProjectPoint(lx0, ly0, lz0, tmpZ);
    Vec2f p1 = ProjectPoint(lx1, ly1, lz1, tmpZ);
    Vec2f p2 = ProjectPoint(lx2, ly2, lz2, tmpZ);
    Vec2f p3 = ProjectPoint(lx3, ly3, lz3, tmpZ);

    float sx0 = viewX + p0.x * viewScale;
    float sy0 = viewY + p0.y * viewScale;
    float sx1 = viewX + p1.x * viewScale;
    float sy1 = viewY + p1.y * viewScale;
    float sx2 = viewX + p2.x * viewScale;
    float sy2 = viewY + p2.y * viewScale;
    float sx3 = viewX + p3.x * viewScale;
    float sy3 = viewY + p3.y * viewScale;

    TexturedVertex verts[6] = {
        { sx0, sy0, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f },
        { sx1, sy1, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f },
        { sx3, sy3, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f },
        { sx1, sy1, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f },
        { sx2, sy2, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f },
        { sx3, sy3, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f },
    };

    ctx.renderer->DrawTexturedTriangleBatch(srv, verts, 6);
}

}
