#include "pr_overlay_loader.h"

#include <fstream>
#include <filesystem>
#include <memory>
#include <vector>

#include "logger.h"
#include "pr_event.h"
#include "pr_game_context.h"
#include "pr_scene_def.h"
#include "pr_scene1_entry_original_disc_direct.h"
#include "pr_ss0_scene0_runtime_direct.h"
#include "int_loader.h"
#include "resource_manager.h"
#include "xa1_player.h"
#include "pr_sqevs1.h"
#include "pr_stage1_overlay_parser.h"
#include "pr_stage1_texture_replacements.h"
#include "pr_stage_scene_submit_backend.h"
#include "pr_ss0_title_tmd_backend.h"
#include "pr_tmd_renderer.h"

static void LogPathExists(const char* label, const std::filesystem::path& p) {
    if (p.empty()) {
        Log::Printf("  %s: (empty)", label);
        return;
    }

    std::error_code ec;
    const bool exists = std::filesystem::exists(p, ec);
    Log::Printf("  %s: %s exists=%d", label, p.u8string().c_str(), exists ? 1 : 0);
}

static bool ReadFileBytes(const std::filesystem::path& p, std::vector<uint8_t>& out) {
    out.clear();
    std::ifstream f(p, std::ios::binary | std::ios::ate);
    if (!f.is_open()) {
        Log::Printf("  ReadFileBytes: open failed: %s", p.u8string().c_str());
        return false;
    }
    const std::streamsize sz = f.tellg();
    if (sz <= 0) {
        Log::Printf("  ReadFileBytes: empty file: %s", p.u8string().c_str());
        return false;
    }
    f.seekg(0);
    out.resize((size_t)sz);
    f.read(reinterpret_cast<char*>(out.data()), sz);
    if (!f.good()) {
        Log::Printf("  ReadFileBytes: read failed: %s", p.u8string().c_str());
        out.clear();
        return false;
    }
    return true;
}

static void ParseStage1OverlayDataFromComod(
    PrGameContext& ctx,
    const IntArchive& stageCompoArchive,
    bool haveStageCompoArchive) {
    if (ctx.currentComodBytes.empty() || !haveStageCompoArchive) {
        return;
    }

    auto parsed = std::make_shared<PrStage1OverlayData>();
    const bool ok = PrStage1OverlayParser::ParseFromBytes(
        ctx.currentComodBytes.data(),
        ctx.currentComodBytes.size(),
        stageCompoArchive,
        *parsed);
    if (ok && parsed->valid) {
        ctx.stage1OverlayData = parsed;
        Log::Printf(
            "  Stage1OverlayParser ok=1 textTables=%llu pairs=%llu streamRows=%llu hudSlots=%llu streams=%llu",
            (unsigned long long)parsed->textTables.size(),
            (unsigned long long)parsed->pairTable.size(),
            (unsigned long long)parsed->streamDescRows.size(),
            (unsigned long long)parsed->hudSlotDescs.size(),
            (unsigned long long)parsed->streams.size());
        if (const PrStage1EventStream* stream1 = parsed->FindStream(1)) {
            Log::Printf("  Stage1OverlayParser stream1 events=%u ptr=0x%08X",
                        (unsigned)stream1->count,
                        (unsigned)stream1->eventsPtr);
        }
        if (const PrStage1EventStream* stream8 = parsed->FindStream(8)) {
            Log::Printf("  Stage1OverlayParser stream8 events=%u ptr=0x%08X",
                        (unsigned)stream8->count,
                        (unsigned)stream8->eventsPtr);
        }
    } else {
        Log::Printf("  Stage1OverlayParser ok=0");
    }
}

static bool LoadScene1DirectRuntimeResources(
    const PrScene1EntryOriginalDiscDirect::Transaction80015D18& transaction,
    const std::filesystem::path& comod,
    const std::filesystem::path& compo,
    const std::filesystem::path& zcompo,
    const std::filesystem::path& xa,
    PrGameContext& ctx) {
    ctx.currentComodBytes = transaction.read8001A818.bytes;
    Log::Printf(
        "  Direct Scene1 original-disc ingress accepted=1 psxPath=%s disc=%s lba=%d bytes=%llu",
        transaction.psxPath.c_str(),
        transaction.originalDiscPath.u8string().c_str(),
        static_cast<int>(transaction.read8001A818.lba),
        static_cast<unsigned long long>(ctx.currentComodBytes.size()));

    LogPathExists("COMOD projection (comparison only)", comod);
    LogPathExists("COMPO", compo);
    LogPathExists("ZCOMPO", zcompo);
    LogPathExists("XA", xa);

    IntArchive stageCompoArchive;
    bool haveStageCompoArchive = false;
    if (!compo.empty()) {
        haveStageCompoArchive =
            IntLoader::Load(compo.u8string(), stageCompoArchive);
        Log::Printf("  Direct Stage1 COMPO parse ok=%d entries=%llu",
                    haveStageCompoArchive ? 1 : 0,
                    (unsigned long long)stageCompoArchive.entries.size());
    }

    if (ctx.resources) {
        ctx.resources->Clear();
        PrStageSceneSubmitBackend::ClearStage1Resources();

        if (!compo.empty()) {
            const std::string compoUtf8 = compo.u8string();
            Log::Printf("  Direct LoadIntArchive(COMPO): %s",
                        compoUtf8.c_str());
            const bool ok = ctx.resources->LoadIntArchive(compoUtf8);
            Log::Printf(
                "  Direct LoadIntArchive(COMPO) ok=%d textures=%llu mem=%llu",
                ok ? 1 : 0,
                (unsigned long long)ctx.resources->GetTextureCount(),
                (unsigned long long)ctx.resources->GetMemCount());
        }

        if (!zcompo.empty()) {
            const std::string zcompoUtf8 = zcompo.u8string();
            Log::Printf("  Direct LoadIntArchive(ZCOMPO): %s",
                        zcompoUtf8.c_str());
            const bool ok = ctx.resources->LoadIntArchive(zcompoUtf8);
            Log::Printf(
                "  Direct LoadIntArchive(ZCOMPO) ok=%d textures=%llu mem=%llu",
                ok ? 1 : 0,
                (unsigned long long)ctx.resources->GetTextureCount(),
                (unsigned long long)ctx.resources->GetMemCount());
        }

        const auto* sharedCommon = PrSS0Scene0RuntimeDirect::GetSharedStartupCommonIntLoad80016B84();
        if (sharedCommon == nullptr ||
            !PrStageSceneSubmitBackend::LoadStage1Resources(ctx.resources, sharedCommon)) {
            Log::Printf("Direct Stage1 atlas rejected: native startup COMMON projection unavailable");
            return false;
        }
        PrStage1TextureReplacements::Prime(ctx);
    }

    ParseStage1OverlayDataFromComod(ctx,
                                    stageCompoArchive,
                                    haveStageCompoArchive);
    Log::Printf(
        "OverlayLoader::Load scene=1 direct runtime: skipped legacy TMD/SQEV setup");
    return true;
}

static void ClearLegacyOverlayResidueForDirectCutover(PrGameContext& ctx) {
    PrSS0TitleTmdBackend::Clear();
    PrEvent::ClearDispatcherResidueForDirectCutover();
    PrSqevs1::Shutdown(ctx);
    PrTmdRenderer::Clear();
}

bool PrOverlayLoader::Load(
    PrSceneId id,
    const PrSceneDef& def,
    PrGameContext& ctx,
    const PrScene1EntryOriginalDiscDirect::Transaction80015D18*
        scene1OriginalDiscTransaction) {
    if (m_loaded) {
        Unload(ctx);
    }

    m_scene = id;
    m_loaded = true;

    Log::Printf("OverlayLoader::Load scene=%u", (unsigned)id);

    const std::filesystem::path comod = def.comod.path.empty() ? std::filesystem::path{} : (ctx.dataRoot / def.comod.path);
    const std::filesystem::path compo = def.compo.path.empty() ? std::filesystem::path{} : (ctx.dataRoot / def.compo.path);
    const std::filesystem::path zcompo = def.zcompo.path.empty() ? std::filesystem::path{} : (ctx.dataRoot / def.zcompo.path);
    const std::filesystem::path xa = def.xa.path.empty() ? std::filesystem::path{} : (ctx.dataRoot / def.xa.path);

    ctx.currentXaPath = xa;
    ctx.currentComodPath = comod;
    ctx.currentCompoPath = compo;
    ctx.currentZcompoPath = zcompo;
    ctx.currentComodBytes.clear();
    ctx.stage1OverlayData.reset();

    const bool ss0DirectCutover = PrSS0Scene0RuntimeDirect::RuntimeEnabled();
    const bool scene0DirectRuntime =
        id == PrSceneId::Scene0 && ss0DirectCutover;
    const bool scene1DirectRuntime =
        id == PrSceneId::Scene1 && ss0DirectCutover;
    if (scene0DirectRuntime) {
        ctx.currentComodPath = std::filesystem::path{};
        ctx.currentZcompoPath = std::filesystem::path{};
        ClearLegacyOverlayResidueForDirectCutover(ctx);
        PrStageSceneSubmitBackend::ClearStage1Resources();
        if (ctx.resources) {
            ctx.resources->Clear();

            std::vector<uint8_t> scene0ComodBytes;
            const bool scene0ComodLoaded =
                !comod.empty() && ReadFileBytes(comod, scene0ComodBytes);
            bool compoLoaded = false;
            if (!compo.empty()) {
                const std::string compoUtf8 = compo.u8string();
                Log::Printf("  Direct Scene0 LoadIntArchive(COMPO): %s",
                            compoUtf8.c_str());
                compoLoaded = ctx.resources->LoadIntArchive(compoUtf8);
            }
            const bool titleTmdReady =
                compoLoaded && scene0ComodLoaded &&
                PrSS0TitleTmdBackend::LoadResources(
                    ctx.resources,
                    {scene0ComodBytes.data(), scene0ComodBytes.size()},
                    ctx.mainSceneLoaderMemoryDirect);
            const PrSS0TitleTmdBackend::ResourceState titleTmdState =
                PrSS0TitleTmdBackend::GetResourceState();
            Log::Printf(
                "  Direct Scene0 LoadIntArchive(COMPO) ok=%d textures=%llu mem=%llu titleTmd=%d/%u mimePairs=%u tods=%u comod=%d/%d cameraTable=%d cmOp27=%d cameraBinding=%d cameraCursor=%d cameraMatrix=%d panelCamera=%d paLoc2=%d/%d graphControl=%d paKageBase=%d runtimeMimeBinding=%d runtimeMimeCursor=%d runtimeVertexDeform=%d runtimeMatrix=%d rtpt3Input=%d tpages=%u tim=%u",
                compoLoaded ? 1 : 0,
                (unsigned long long)ctx.resources->GetTextureCount(),
                (unsigned long long)ctx.resources->GetMemCount(),
                titleTmdReady ? 1 : 0,
                static_cast<unsigned>(titleTmdState.loadedModelCount),
                static_cast<unsigned>(titleTmdState.loadedMimePairCount),
                static_cast<unsigned>(titleTmdState.loadedTitleTodCount),
                scene0ComodLoaded ? 1 : 0,
                titleTmdState.scene0ComodAccepted ? 1 : 0,
                titleTmdState.titleCameraTableKnown801C6FB4 ? 1 : 0,
                titleTmdState.cmOpBezResource27Loaded ? 1 : 0,
                titleTmdState.runtimeTitleCameraBindingKnown ? 1 : 0,
                titleTmdState.runtimeTitleCameraCursorKnown ? 1 : 0,
                titleTmdState.titleCameraMatrixKnown80092880 ? 1 : 0,
                titleTmdState.titlePanelWithCameraKnown80041A68 ? 1 : 0,
                titleTmdState.paLoc2TodLoaded ? 1 : 0,
                titleTmdState.paLoc2CoordKnown ? 1 : 0,
                titleTmdState.titleGraphControlKnown ? 1 : 0,
                titleTmdState.firstPaKageMode25BaseTriangleKnown ? 1 : 0,
                titleTmdState.firstPaKageRuntimeMimeBindingKnown ? 1 : 0,
                titleTmdState.firstPaKageRuntimeMimeCursorKnown ? 1 : 0,
                titleTmdState.firstPaKageRuntimeVertexDeformationKnown ? 1 : 0,
                titleTmdState.firstPaKageRuntimeMatrixKnown ? 1 : 0,
                titleTmdState.firstPaKageMode25Rtpt3InputKnown ? 1 : 0,
                static_cast<unsigned>(titleTmdState.registeredTpageCount),
                static_cast<unsigned>(titleTmdState.loadedTimCount));
        }
        Log::Printf("OverlayLoader::Load scene=0 direct runtime: skipped legacy overlay setup");
        return true;
    }
    if (scene1DirectRuntime) {
        ClearLegacyOverlayResidueForDirectCutover(ctx);
        if (scene1OriginalDiscTransaction == nullptr) {
            Log::Printf(
                "OverlayLoader::Load scene=1 direct runtime: original-disc ingress rejected (missing transaction)");
            return false;
        }
        if (!PrScene1EntryOriginalDiscDirect::
                IsExactAcceptedTransaction80015D18(
                    *scene1OriginalDiscTransaction)) {
            Log::Printf(
                "OverlayLoader::Load scene=1 direct runtime: original-disc ingress rejected (transaction not exact accepted) status=%u accepted=%d complete=%d",
                static_cast<unsigned>(scene1OriginalDiscTransaction->status),
                scene1OriginalDiscTransaction->accepted ? 1 : 0,
                scene1OriginalDiscTransaction->complete ? 1 : 0);
            return false;
        }
        return LoadScene1DirectRuntimeResources(*scene1OriginalDiscTransaction,
                                                comod,
                                                compo,
                                                zcompo,
                                                xa,
                                                ctx);
    }

    if (!comod.empty()) {
        ReadFileBytes(comod, ctx.currentComodBytes);
    }

    LogPathExists("COMOD", comod);
    LogPathExists("COMPO", compo);
    LogPathExists("ZCOMPO", zcompo);
    LogPathExists("XA", xa);

    IntArchive stageCompoArchive;
    bool haveStageCompoArchive = false;
    if (id == PrSceneId::Scene1 && !compo.empty()) {
        haveStageCompoArchive = IntLoader::Load(compo.u8string(), stageCompoArchive);
        Log::Printf("  IntLoader(Stage1 COMPO) ok=%d entries=%llu",
                    haveStageCompoArchive ? 1 : 0,
                    (unsigned long long)stageCompoArchive.entries.size());
    }

    if (ctx.resources) {
        ctx.resources->Clear();
        PrStageSceneSubmitBackend::ClearStage1Resources();

        if (!compo.empty()) {
            const std::string compoUtf8 = compo.u8string();
            Log::Printf("  LoadIntArchive(COMPO): %s", compoUtf8.c_str());
            const bool ok = ctx.resources->LoadIntArchive(compoUtf8);
            Log::Printf("  LoadIntArchive(COMPO) ok=%d textures=%llu mem=%llu",
                        ok ? 1 : 0,
                        (unsigned long long)ctx.resources->GetTextureCount(),
                        (unsigned long long)ctx.resources->GetMemCount());
        }

        if (!zcompo.empty()) {
            const std::string zcompoUtf8 = zcompo.u8string();
            Log::Printf("  LoadIntArchive(ZCOMPO): %s", zcompoUtf8.c_str());
            const bool ok = ctx.resources->LoadIntArchive(zcompoUtf8);
            Log::Printf("  LoadIntArchive(ZCOMPO) ok=%d textures=%llu mem=%llu",
                        ok ? 1 : 0,
                        (unsigned long long)ctx.resources->GetTextureCount(),
                        (unsigned long long)ctx.resources->GetMemCount());
        }

        // Scene1 direct owns its draw resources through Stage1 submit backends;
        // keep the title/TMD renderer out of that path.
        if (!scene1DirectRuntime) {
            PrTmdRenderer::LoadModels(ctx.resources);
            if (id != PrSceneId::Scene0 && !compo.empty()) {
                PrTmdRenderer::LoadStageModels(ctx.resources,
                                                static_cast<int>(id));
            }
        }
        if (id == PrSceneId::Scene1) {
            PrStageSceneSubmitBackend::LoadStage1Resources(ctx.resources);
            PrStage1TextureReplacements::Prime(ctx);
        }

        // Initialize camera animation from COMOD*.BIN event table
        if (!scene1DirectRuntime && !ctx.currentComodBytes.empty()) {
            PrTmdRenderer::InitCameraEvents(ctx.currentComodBytes.data(),
                                             ctx.currentComodBytes.size(),
                                             ctx.resources,
                                             static_cast<int>(id));
        }

        // Initialize face event system from COMOD*.BIN + PSX tables JSON
        if (!scene1DirectRuntime && !ctx.currentComodBytes.empty()) {
            const std::string faceJsonPath =
                (ctx.dataRoot / "out" / "psx_face_tables.json").u8string();
            PrTmdRenderer::InitFaceEvents(ctx.currentComodBytes.data(),
                                           ctx.currentComodBytes.size(),
                                           faceJsonPath,
                                           ctx.resources);

            const bool okTim = ctx.resources->LoadTimFromBytes(ctx.currentComodBytes.data(),
                                                               ctx.currentComodBytes.size(),
                                                               "comod:");
            Log::Printf("  LoadTimFromBytes(COMOD) ok=%d textures=%llu", okTim ? 1 : 0,
                        (unsigned long long)ctx.resources->GetTextureCount());
        }
    }

    if (id == PrSceneId::Scene1 && !ctx.currentComodBytes.empty() && haveStageCompoArchive) {
        ParseStage1OverlayDataFromComod(ctx,
                                        stageCompoArchive,
                                        haveStageCompoArchive);
    }

    if ((id == PrSceneId::Scene0 && !scene0DirectRuntime) ||
        (id == PrSceneId::Scene1 && !scene1DirectRuntime)) {
        bool loaded = false;

        if (!comod.empty()) {
            if (!ctx.currentComodBytes.empty()) {
                Log::Printf("  Scan COMOD for event table size=%u", (unsigned)ctx.currentComodBytes.size());

                PrSqevs1::SetEventSourceBytes(ctx.currentComodBytes.data(), ctx.currentComodBytes.size());

                constexpr size_t kScanStep = 0x400;
                constexpr size_t kMaxScanOffset = 0x20000;
                const size_t scanLimit = std::min<size_t>(ctx.currentComodBytes.size(), kMaxScanOffset);

                for (size_t off = 0; off < scanLimit && !loaded; off += kScanStep) {
                    const uint8_t* ptr = ctx.currentComodBytes.data() + off;
                    const size_t rem = ctx.currentComodBytes.size() - off;
                    if (PrSqevs1::LoadEventTable(ptr, rem)) {
                        Log::Printf("  Sqevs1::LoadEventTable ok=1 comod='%s' off=0x%X size=%u events=%d",
                                    comod.u8string().c_str(),
                                    (unsigned)off,
                                    (unsigned)ctx.currentComodBytes.size(),
                                    PrSqevs1::GetEventCount());
                        loaded = true;
                        break;
                    }
                }
            }
        }

        if (!loaded && !compo.empty()) {
            const std::string compoUtf8 = compo.u8string();
            IntArchive archive;
            if (IntLoader::Load(compoUtf8, archive)) {
                for (const auto& entry : archive.entries) {
                    if (entry.type != IntBlockType::Mem) {
                        continue;
                    }

                    // 过滤掉明显不是脚本表的资源
                    if (entry.name.size() >= 4) {
                        const std::string ext = entry.name.substr(entry.name.size() - 4);
                        if (ext == ".TMD" || ext == ".TOD") {
                            continue;
                        }
                    }
                    PrSqevs1::SetEventSourceBytes(entry.data.data(), entry.data.size());
                    if (PrSqevs1::LoadEventTable(entry.data.data(), entry.data.size())) {
                        Log::Printf("  Sqevs1::LoadEventTable ok=1 mem='%s' size=%u events=%d",
                                    entry.name.c_str(),
                                    (unsigned)entry.data.size(),
                                    PrSqevs1::GetEventCount());
                        loaded = true;
                        break;
                    }
                }
            }
        }

        if (!loaded) {
            Log::Printf("  Sqevs1::LoadEventTable ok=0 (no block matched)");
        }
    }

    return true;
}

void PrOverlayLoader::Unload(PrGameContext& ctx) {
    if (!m_loaded) {
        return;
    }

    Log::Printf("OverlayLoader::Unload scene=%u", (unsigned)m_scene);

    if (ctx.xa1Player) {
        ctx.xa1Player->Stop();
    }

    const bool scene0DirectRuntime =
        m_scene == PrSceneId::Scene0 &&
        PrSS0Scene0RuntimeDirect::RuntimeEnabled();
    const bool scene1DirectRuntime =
        m_scene == PrSceneId::Scene1 &&
        PrSS0Scene0RuntimeDirect::RuntimeEnabled();
    if ((m_scene == PrSceneId::Scene0 && !scene0DirectRuntime) ||
        (m_scene == PrSceneId::Scene1 && !scene1DirectRuntime)) {
        PrSqevs1::Shutdown(ctx);
    }
    if (scene0DirectRuntime || scene1DirectRuntime) {
        ClearLegacyOverlayResidueForDirectCutover(ctx);
    }
    PrStageSceneSubmitBackend::ClearStage1Resources();

    ctx.currentXaPath = std::filesystem::path{};
    ctx.currentComodPath = std::filesystem::path{};
    ctx.currentCompoPath = std::filesystem::path{};
    ctx.currentZcompoPath = std::filesystem::path{};
    ctx.currentComodBytes.clear();
    ctx.stage1OverlayData.reset();

    m_loaded = false;
    m_scene = PrSceneId::Scene0;
}
