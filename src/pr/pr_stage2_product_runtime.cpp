#include "pr_stage2_product_runtime.h"
#include "pr_stage2_modern_presentation.h"

#include "int_loader.h"
#include "pr_pad.h"
#include "pr_psx_pad_direct.h"
#include "pr_stage2_tim_direct.h"
#include "pr_stage2_retry_direct.h"
#include "pr_stage2_shared_state.h"
#include "pr_stage1_save_ui_direct.h"
#include "pr_memcard_backend.h"
#include "pr_game_context.h"
#include "pr_stage2_vlc_session.h"
#include "pr_stage2_lifecycle_direct.h"
#include "pr_psx_graph_owner_direct.h"
#include "pr_ss0_scene0_runtime_direct.h"
#include "pr_ss0_scene0_int_load_direct.h"
#include "pr_vram_atlas.h"
#include "wasapi_sink.h"
#include "d3d11_renderer.h"
#include "logger.h"
#include <windows.h>

// Product-session diagnostics keep retained device waits inspectable.

#include <algorithm>
#include <array>
#include <chrono>
#include <fstream>
#include <iterator>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>
#include <unordered_map>

namespace PrStage2ProductRuntime {
namespace {

namespace F = PrStage2FrameTask;
namespace D = PrStage2LoadingDeviceSession;

class ProductSession final : public PrStage2VlcSession::Session {
public:
    ProductSession(const std::filesystem::path& nativeData,
                   const std::filesystem::path& overlay,
                   const std::filesystem::path& disc,
                   D3D11Renderer& renderer,
                   IAudioSink& sink, PrGameContext& context)
        : PrStage2VlcSession::Session(nativeData, overlay, disc, renderer, sink) {
        Cd().EnableMechanicalTiming();
        modern_=std::make_unique<PrStage2ModernPresentation>(context,*this);
        Gpu().SetPresentation(modern_.get());
    }

    ~ProductSession() { Gpu().SetPresentation(nullptr); }
    bool WantsTmdPresentation() const override { return modern_!=nullptr; }
    void ObservePresentationFrame(uint32_t work,int32_t kind) override { if(modern_)modern_->BeginFrame(work,kind); }
    void ObserveTmdPacket(uint32_t p,uint32_t d,uint32_t o,uint32_t v) override { if(modern_)modern_->Tmd(p,d,o,v); }
    void ObserveRail(uint32_t w,bool b) override { if(modern_)modern_->Rail(w,b); }
    void ObserveRailNote(uint32_t p,uint16_t x,uint16_t y,uint16_t t) override { if(modern_)modern_->Note(p,x,y,t); }
    void ObservePortrait(uint32_t p,uint32_t t,int32_t x,int32_t y) override { if(modern_)modern_->Portrait(p,t,x,y); }
    void ObserveCaption(uint32_t t,uint32_t first,uint32_t end) override { if(modern_)modern_->Caption(t,first,end); }
    void ObserveStatus(uint32_t w,int32_t layout,uint32_t first,uint32_t end) override { if(modern_)modern_->Status(w,layout,first,end); }
    void ObserveScore(uint32_t w,int32_t f,int32_t r,int32_t d,int32_t h,int32_t total) override { if(modern_)modern_->Score(w,f,r,d,h,total); }

    void SetColdPrerequisites(bool value) noexcept { coldPrerequisites_ = value; }
    void SetDebugPadSource(std::function<uint32_t()> value) {
        debugPadSource_ = std::move(value);
    }
    uint64_t GameUpdates() const noexcept { return inputReads_; }

    void ProjectDirectory(PrPsxGraphOwnerDirect::PsxGraphState& graph,
                          PsxVramAtlas& atlas) {
        // Read the completed source session. Never initialize another stage's
        // graph, replay its entry callbacks or borrow its stale packet state.
        if (State() != F::State::Completed || Gpu().Pending() || Gpu().Faulted())
            throw std::runtime_error("S2 directory snapshot requires a completed GPU owner");
        const auto half = [this](uint32_t address) {
            return static_cast<int16_t>(Read16(address));
        };
        graph.word_80096590 = Read16(0x80096590u);
        graph.dword_8009658C = Read32(0x8009658Cu);
        graph.word_800965A0 = Read16(0x800965A0u);
        if (graph.word_80096590 > 1u)
            throw std::runtime_error("S2 directory snapshot has an invalid draw buffer");
        for (uint32_t slot = 0; slot < 2; ++slot) {
            graph.word_8008ECA8[slot] = half(0x8008ECA8u + 2u * slot);
            graph.word_8008ECAC[slot] = half(0x8008ECACu + 2u * slot);
            graph.word_8008EEF0[slot] = half(0x8008EEF0u + 2u * slot);
            graph.word_8008EEF4[slot] = half(0x8008EEF4u + 2u * slot);
            graph.dword_8006ED50[slot] = Read32(0x8006ED50u + 4u * slot);
            auto& page = graph.mainPageWorkLists80087288[slot];
            page.workAddr = 0x80087288u + 20u * slot;
            page.otHeadAddr = Read32(page.workAddr + 4u);
            auto& work = page.work;
            work.order_00 = Read32(page.workAddr);
            work.headAddr_04 = page.otHeadAddr;
            work.x_08 = Read32(page.workAddr + 8u);
            work.y_0C = Read32(page.workAddr + 12u);
            work.lastAddr_10 = Read32(page.workAddr + 16u);
            if (work.order_00 != 4u || page.otHeadAddr != 0x800872B0u + 64u * slot ||
                work.lastAddr_10 != page.otHeadAddr + 60u)
                throw std::runtime_error("S2 directory snapshot lacks initialized native work lists");
            // Packet mirrors start unknown. The directory's real per-frame
            // ClearWorkList rebuilds them before it submits any new packet.
        }
        graph.mainPageWorkLists80087288Initialized = true;
        graph.word_800901C4 = half(0x800901C4u);
        graph.word_800901C6 = half(0x800901C6u);
        graph.word_800928D0 = half(0x800928D0u);
        graph.word_800928D2 = half(0x800928D2u);
        graph.word_800928D4 = half(0x800928D4u);
        graph.word_800928D6 = half(0x800928D6u);
        graph.word_80091790 = half(0x80091790u);
        graph.word_80091792 = half(0x80091792u);
        graph.dword_800901C8 = Read32(0x800901C8u);
        auto& draw = graph.drawOffset;
        draw.word_800917AA = half(0x800917AAu);
        draw.word_800917AC = half(0x800917ACu);
        draw.word_80091730 = half(0x80091730u);
        draw.word_80091732 = half(0x80091732u);
        draw.word_80091734 = half(0x80091734u);
        draw.word_80091736 = half(0x80091736u);
        draw.word_80091738 = half(0x80091738u);
        draw.word_8009173A = half(0x8009173Au);
        const auto gpu = Gpu().DrawState();
        draw.setDrawEnvCalled = gpu.offsetX == draw.word_80091738 &&
            gpu.offsetY == draw.word_8009173A &&
            gpu.clipLeft == draw.word_80091730 && gpu.clipTop == draw.word_80091732 &&
            gpu.clipRight == draw.word_80091730 + draw.word_80091734 - 1 &&
            gpu.clipBottom == draw.word_80091732 + draw.word_80091736 - 1;
        if (!draw.setDrawEnvCalled)
            throw std::runtime_error("S2 directory draw environment was not consumed by the GPU");
        const auto& gte = MatrixGte();
        draw.setGeomOffsetCalled = gte.ofx == int32_t(draw.word_800917AA) * 65536 &&
            gte.ofy == int32_t(draw.word_800917AC) * 65536;
        graph.gte = {true, gte.h, true, gte.ofx, gte.ofy,
                     true, gte.dqa, gte.dqb, true, gte.zsf3, gte.zsf4};
        for (uint32_t i = 0; i < graph.tmdFastHandlerTable8001C1E8.size(); ++i) {
            const uint32_t fn = Read32(0x8008EDD8u + 4u * i);
            graph.tmdFastHandlerTable8001C1E8[i] = fn;
            if (fn) ++graph.tmdFastHandlerTableNonZeroCount8001C1E8;
        }
        graph.tmdFastHandlerTableKnown8001C1E8 = true;
        if (!PrPsxGraphOwnerDirect::IsExactTmdFastHandlerTable8001C1E8(graph))
            throw std::runtime_error("S2 directory primitive table differs from native initialization");

        // COMMON survives scene changes in the original VRAM. Retain the
        // actual startup transaction, then the S2 uploads, then the latest
        // native partial writes. TIM metadata only registers page/palette
        // shapes; current native VRAM wins for every word it has written.
        const auto* common = PrSS0Scene0RuntimeDirect::GetSharedStartupCommonIntLoad80016B84();
        if (!common) throw std::runtime_error("S2 directory has no resident COMMON transaction");
        for (const auto& entry : common->entries) {
            if (entry.type != PrSS0Scene0IntLoadDirect::BlockType8001A8F0::Tim) continue;
            if (entry.dataOffset > common->archiveBytes.size() ||
                entry.size > common->archiveBytes.size() - entry.dataOffset)
                throw std::runtime_error("S2 directory COMMON record exceeds archive");
            const auto end = std::find(entry.name.begin(), entry.name.end(), uint8_t(0));
            if (!atlas.LoadTim(common->archiveBytes.data() + entry.dataOffset,
                    entry.size, std::string(entry.name.begin(), end), true, false))
                throw std::runtime_error("S2 directory COMMON TIM projection failed");
        }
        const auto* zcompo =
            PrSS0Scene0RuntimeDirect::GetSharedStartupZCompoIntLoad80015590();
        if (!zcompo)
            throw std::runtime_error("S2 directory has no resident ZCOMPO transaction");
        for (const auto& entry : zcompo->entries) {
            if (entry.type != PrSS0Scene0IntLoadDirect::BlockType8001A8F0::Tim) continue;
            if (entry.dataOffset > zcompo->archiveBytes.size() ||
                entry.size > zcompo->archiveBytes.size() - entry.dataOffset)
                throw std::runtime_error("S2 directory ZCOMPO record exceeds archive");
            const auto end = std::find(entry.name.begin(), entry.name.end(), uint8_t(0));
            if (!atlas.LoadTim(zcompo->archiveBytes.data() + entry.dataOffset,
                    entry.size, std::string(entry.name.begin(), end), true, false))
                throw std::runtime_error("S2 directory ZCOMPO TIM projection failed");
        }
        for (const auto& entry : residentTimUploads_) {
            if (!atlas.LoadTim(entry.data.data(), entry.data.size(), entry.name, true, false))
                throw std::runtime_error("S2 directory TIM projection failed: " + entry.name);
        }
        if (!atlas.ApplyPartialVramWords(Vram().Words().data(), Vram().Known().data(),
                Vram().Words().size()))
            throw std::runtime_error("S2 directory native VRAM projection failed");
        Log::Printf("S2 directory snapshot: slot=%u frame=%u nativeWords=%u tims=%d tpages=%d exit=%d",
            graph.word_80096590, graph.dword_8009658C, Vram().KnownWords(),
            atlas.GetLoadedCount(), atlas.GetTpageCount(), half(0x800916E0u));
    }

    uint32_t ReadPad80035510() override {
        const uint32_t local = ReadHostPadSnapshot();
        const uint32_t debug = debugPadSource_ ? debugPadSource_() : 0u;
        return local | debug;
    }

    void LoadCommonResident(const std::filesystem::path& path) {
        if (commonLoaded_) return;
        std::ifstream file(path, std::ios::binary | std::ios::ate);
        if (!file) throw std::runtime_error("S2 COMMON resident archive is missing: " + path.u8string());
        const auto length = file.tellg();
        if (length <= 0) throw std::runtime_error("S2 COMMON resident archive is empty: " + path.u8string());
        std::vector<uint8_t> bytes(static_cast<std::size_t>(length));
        file.seekg(0, std::ios::beg);
        file.read(reinterpret_cast<char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
        if (!file) throw std::runtime_error("S2 COMMON resident archive read failed: " + path.u8string());
        constexpr uint32_t archive = 0x800A0000u;
        constexpr uint32_t info = 0x801B0000u;
        if (bytes.size() >= info - archive)
            throw std::runtime_error("S2 COMMON resident archive exceeds its native RAM window");
        for (uint32_t i = 0; i < bytes.size(); ++i) Write8(archive + i, bytes[i]);

        IntArchive parsed;
        if (!IntLoader::Load(path.u8string(), parsed))
            throw std::runtime_error("S2 COMMON resident archive parse failed: " + path.u8string());
        uint32_t timCount = 0;
        for (const auto& entry : parsed.entries) {
            if (entry.type != IntBlockType::Tim || entry.size == 0u) continue;
            const uint32_t entryAddress = archive + entry.offset;
            PrStage2TimDirect::GetTimInfo80040EAC(*this, entryAddress + 4u, info);
            const auto upload = [&](uint32_t rect, uint32_t source) {
                const PrStage2LifecycleDirect::ImageRect image{
                    Read16(rect), Read16(rect + 2u), Read16(rect + 4u), Read16(rect + 6u)};
                if (LoadImage80044D64(image, Read32(source)) < 0)
                    throw std::runtime_error("S2 COMMON resident TIM upload failed");
            };
            upload(info + 4u, info + 12u);
            if ((Read32(info) & 8u) != 0u) upload(info + 16u, info + 24u);
            residentTimUploads_.push_back(entry);
            ++timCount;
        }
        if (timCount == 0u) throw std::runtime_error("S2 COMMON resident archive has no TIM records");
        commonLoaded_ = true;
    }

    void LoadStageResidentTims(const std::filesystem::path& path) {
        IntArchive parsed;
        if (!IntLoader::Load(path.u8string(), parsed))
            throw std::runtime_error("S2 stage TIM archive parse failed: " + path.u8string());
        uint32_t timCount = 0;
        uint32_t skippedCount = 0;
        for (const auto& entry : parsed.entries) {
            if (entry.type != IntBlockType::Tim || entry.size == 0u) continue;
            // COMPO archives also carry placeholder/metadata entries labelled
            // as TIM (for example HANDLE.TIM) that are not valid standalone
            // GPU uploads.  The original loader skips those records and keeps
            // going; one malformed entry must not prevent the product session
            // from reaching the movie clock.
            if (Vram().SeedTim(entry.data.data(), entry.data.size())) {
                residentTimUploads_.push_back(entry);
                ++timCount;
            } else {
                ++skippedCount;
            }
        }
        if (timCount == 0u)
            throw std::runtime_error("S2 stage TIM archive has no TIM records: " + path.u8string());
        Log::Printf("S2 stage TIM resident uploads=%u skipped=%u archive=%s",
                    static_cast<unsigned>(timCount), static_cast<unsigned>(skippedCount),
                    path.u8string().c_str());
    }

protected:
    void BeginGameStreamStartup() override {
        if(modern_)modern_->SetGameplay(false);
        Cd().HoldNextStreamStart();
        gameStreamHeld_ = true;
        gameplayFrameAllowed_ = false;
        clearRevealFrames_ = 0u;
        firstGameInput_ = true;
    }

    bool AllowGameplayFrame() const override { return gameplayFrameAllowed_; }

    bool AllowStartupSceneAnimation() const override {
        // The tile transition reveals the already prepared destination page.
        // Keep that page on one scene frame until the source pre-loop returns;
        // only the tile mask and presentation page are allowed to advance.
        return false;
    }

    // The original pre-loop keeps the prepared destination scene underneath
    // the tile mask. Keep that page visible while the gameplay/input boundary
    // remains gated; the scene below the mask stays on its prepared frame.
    bool AllowStartupSceneDraw() const override { return true; }

    void GameStartupPreLoopEnded() override {
        gameplayFrameAllowed_ = true;
        Log::PrintfNoFlush("S2 startup pre-loop ended; dynamic gameplay frame path enabled phase=%u",
            Read32(0x8006EB04u));
    }

    void AwaitGameEntryPresentation() override {
        if (!transitionPresentationCompleted_) return;
        transitionPresentationCompleted_ = false;
        // 800201AC/80020110 already waited for the submitted page and one
        // sampled scanout edge. Keep one explicit handoff here as well: the
        // next source call is 8001A4D0, and it must not race the compositor
        // with the final transition page.
        AwaitTransitionPresentation();
        Log::PrintfNoFlush("S2 game-entry presentation fence passed vblank=%llu presents=%llu",
            static_cast<unsigned long long>(WallClockVBlankEvents()),
            static_cast<unsigned long long>(Gpu().DisplayPresents()));
    }

    void TransitionPresentationCompleted() override {
        transitionPresentationCompleted_ = true;
    }

    void GameStartupFramePresented() override {
        if (!gameStreamHeld_) return;
        bool clear = Read32(0x8006EB04u) >= 192u;
        for (uint32_t i = 0; clear && i < 192u; ++i)
            clear = Read32(0x80087330u + 4u * i) == 0u;
        clearRevealFrames_ = clear ? clearRevealFrames_ + 1u : 0u;
        // Swap precedes drawing in 801CB170. Two clear submissions populate
        // both native pages; otherwise scanout can still show the preceding
        // page with three covered tiles even though RAM's mask is already zero.
        if (clearRevealFrames_ < 2u) return;
        AwaitTransitionPresentation();
        Log::PrintfNoFlush("S2 startup reveal presented phase=%u maskNonZero=0 presents=%llu sectors=%llu updates=%llu",
            Read32(0x8006EB04u), Gpu().DisplayPresents(), Cd().SectorReads(), inputReads_);
        Cd().ReleaseStreamStart();
        gameStreamHeld_ = false;
    }

    bool MovieCallbackAllowed() override {
        if (!movieCallbackVblank_.has_value()) {
            if (!MovieOutput().Pending()) return true;
            // Anchor publication between the source VSync(2) iterations.
            // Publishing on the decode iteration's own edge races its blit:
            // an early decode then appears one whole iteration sooner than
            // a decode queued just after that edge. One startup edge gives
            // the asynchronous output a stable phase before the next blit.
            movieCallbackVblank_ = WallClockVBlankEvents() + 1u;
        }
        return WallClockVBlankEvents() >= *movieCallbackVblank_;
    }

    void OnMovieFrameDelivered() override {
        constexpr uint64_t period = 4u; // Four native NTSC VBlanks per STR frame.
        const uint64_t now = WallClockVBlankEvents();
        const uint64_t frame = MovieOutput().CallbackCalls();
        const uint64_t previous = lastMovieCallbackVblank_;
        movieCallbackVblank_ = now + period;
        Log::PrintfNoFlush("S2 movie callback frame=%llu vblank=%llu next=%llu",
                          static_cast<unsigned long long>(frame),
                          static_cast<unsigned long long>(now),
                          static_cast<unsigned long long>(*movieCallbackVblank_));
        if (previous != 0u && now - previous != period) {
            Log::PrintfNoFlush(
                "S2 movie delay frame=%llu delta=%llu cdReads=%llu lastLba=%u nextLba=%u videoPending=%u videoDeferrals=%llu ringIndex=%u ringEnd=%u ringSlots=%u ringEntry=%u",
                static_cast<unsigned long long>(frame),
                static_cast<unsigned long long>(now - previous),
                static_cast<unsigned long long>(Cd().SectorReads()),
                static_cast<unsigned>(Cd().LastSector()),
                static_cast<unsigned>(Cd().Location()),
                Cd().VideoDeliveryPending() ? 1u : 0u,
                static_cast<unsigned long long>(Cd().VideoDeferrals()),
                static_cast<unsigned>(Read32(0x80095C50u)),
                static_cast<unsigned>(Read32(0x8009659Cu)),
                static_cast<unsigned>(Read32(0x801C3868u)),
                static_cast<unsigned>(Read16(Read32(0x800965ACu) +
                                             (Read32(0x80095C50u) << 5u))));
        }
        lastMovieCallbackVblank_ = now;
    }

    void OnMovieScanoutEnded() override {
        Log::Printf("S2 movie scanout ended frames=%llu vblank=%llu",
                    MovieOutput().PublishedFrames(), WallClockVBlankEvents());
        movieCallbackVblank_.reset();
        lastMovieCallbackVblank_ = 0;
    }

    void ObserveInput80035510(uint32_t rawPad, uint32_t mappedPad,
                              uint32_t work) override {
        ++inputReads_;
        if (firstGameInput_) {
            uint32_t covered = 0u;
            for (uint32_t i = 0; i < 192u; ++i)
                if (Read32(0x80087330u + 4u * i) != 0u) ++covered;
            Log::PrintfNoFlush("S2 startup first input phase=%u maskNonZero=%u tick=%d presents=%llu sectors=%llu updates=%llu",
                Read32(0x8006EB04u), covered, static_cast<int32_t>(Read32(work + 12u)),
                Gpu().DisplayPresents(), Cd().SectorReads(), inputReads_);
            firstGameInput_ = false;
        }
        if (inputReads_ == 120u || inputReads_ == 480u || inputReads_ == 840u)
            CaptureDrawOrder(work);
        if (rawPad != 0u) ++inputNonzeroReads_;
        if (rawPad != 0u || (inputReads_ % 120u) == 0u) {
            Log::Printf(
                "S2 product 80035510 read count=%llu nonzero=%llu raw=0x%04X mapped=0x%04X work=%08X tick=%d score=%d rating=%u",
                static_cast<unsigned long long>(inputReads_),
                static_cast<unsigned long long>(inputNonzeroReads_),
                static_cast<unsigned>(rawPad & 0xFFFFu),
                static_cast<unsigned>(mappedPad & 0xFFFFu), work,
                static_cast<int32_t>(Read32(work+12u)),static_cast<int32_t>(Read32(work+48u)),
                static_cast<unsigned>(Read16(work+78u)));
            // Read-only cue visibility for product acceptance diagnostics. The
            // translated judge owns these tables; this trace only reports the
            // live event slot and which decoded inputs have a native descriptor.
            // It never writes RAM or changes the input path.
            if (rawPad != 0u && (inputReads_ <= 80u || (inputReads_ % 30u) == 0u)) {
                const uint32_t eventBlock = Read32(work + 68u);
                const uint32_t eventIndex = Read16(work + 80u);
                const uint32_t event = eventBlock + 6u * eventIndex;
                const uint32_t cue = eventBlock != 0u ? Read8(event + 12u) : 0u;
                const uint32_t rhythm = eventBlock != 0u ? Read8(event + 13u) : 0u;
                const uint32_t table = Read32(0x800943D8u);
                const uint32_t row = table + 36u * cue;
                const uint32_t d1 = table != 0u ? Read32(row + 4u) : 0u;
                const uint32_t d2 = table != 0u ? Read32(row + 8u) : 0u;
                const uint32_t d3 = table != 0u ? Read32(row + 12u) : 0u;
                const uint32_t d4 = table != 0u ? Read32(row + 16u) : 0u;
                const uint32_t d5 = table != 0u ? Read32(row + 20u) : 0u;
                const uint32_t d6 = table != 0u ? Read32(row + 24u) : 0u;
                const uint32_t d7 = table != 0u ? Read32(row + 28u) : 0u;
                Log::PrintfNoFlush(
                    "S2 product cue count=%llu eventBlock=%08X eventIndex=%u cue=%u rhythm=%u tick=%d input=%u descriptors=%08X,%08X,%08X,%08X,%08X,%08X,%08X",
                    static_cast<unsigned long long>(inputReads_), eventBlock,
                    static_cast<unsigned>(eventIndex), static_cast<unsigned>(cue),
                    static_cast<unsigned>(rhythm), static_cast<int32_t>(Read32(work + 16u)),
                    static_cast<unsigned>(Read32(work + 32u)), d1, d2, d3, d4, d5, d6, d7);
            }
            if ((inputReads_ % 120u) == 0u) {
                Log::Printf("S2 product HUD updates=%llu banner=%u bannerType=%u railVisible=%u railRows=%u railCalls=%llu noteSubmits=%llu",
                            inputReads_, unsigned(Read16(work+92u)), unsigned(Read16(work+94u)),
                            unsigned(Read16(work+122u)), unsigned(Read16(work+138u)),
                            CompactRailCalls80024744(), RailNotesSubmitted80024418());
            }
        }
    }

    void ObserveTransitionBoundary(uint32_t function, uint32_t stage,
                                   uint32_t mode, uint32_t phase) override {
        if(modern_)modern_->SetTransition(stage!=2u);
        Log::PrintfNoFlush(
            "S2 boundary transition fn=%08X stage=%u mode=%u phase=%u "
            "gpuSerial=%llu display=%08X drawBuffer=%u presents=%llu commands=%llu "
            "vblank=%llu",
            function, stage, mode, Read32(0x8006EB04u),
            static_cast<unsigned long long>(Gpu().Serial()),
            Gpu().DisplayAddress(),
            static_cast<unsigned>(Read32(0x8006EDA8u)),
            static_cast<unsigned long long>(Gpu().DisplayPresents()),
            static_cast<unsigned long long>(Gpu().CommandsRendered()),
            static_cast<unsigned long long>(WallClockVBlankEvents()));
    }

    void ObserveGameBoundary(uint32_t stage, uint32_t work) override {
        if(modern_)modern_->SetGameplay(stage==3u);
        Log::PrintfNoFlush(
            "S2 boundary game stage=%u work=%08X phase=%u gpuSerial=%llu "
            "display=%08X drawBuffer=%u presents=%llu commands=%llu vblank=%llu "
            "animation=%u updates=%llu",
            stage, work, Read32(0x8006EB04u),
            static_cast<unsigned long long>(Gpu().Serial()),
            Gpu().DisplayAddress(),
            static_cast<unsigned>(Read32(0x8006EDA8u)),
            static_cast<unsigned long long>(Gpu().DisplayPresents()),
            static_cast<unsigned long long>(Gpu().CommandsRendered()),
            static_cast<unsigned long long>(WallClockVBlankEvents()),
            static_cast<unsigned>(Read32(0x8006EA54u)),
            static_cast<unsigned long long>(inputReads_));
    }

    void ObserveGamePreLoop(uint32_t iteration, uint32_t work, uint32_t ready) override {
        if (iteration > 3u && iteration % 16u != 0u && ready != 1u && iteration != 1800u)
            return;
        uint32_t maskNonZero = 0u;
        for (uint32_t i = 0; i < 192u; ++i)
            if (Read32(0x80087330u + 4u * i) != 0u) ++maskNonZero;
        Log::PrintfNoFlush(
            "S2 boundary preloop iteration=%u ready=%u phase=%u maskNonZero=%u "
            "animation=%u tick=%d flags=%08X banner=%u railVisible=%u railRows=%u "
            "updates=%llu",
            iteration, ready, Read32(0x8006EB04u), maskNonZero, Read32(0x8006EA54u),
            static_cast<int32_t>(Read32(work + 12u)), Read32(work),
            unsigned(Read16(work + 92u)), unsigned(Read16(work + 122u)),
            unsigned(Read16(work + 138u)),
            static_cast<unsigned long long>(inputReads_));
    }

    void ObserveJudgeInput80014614(uint32_t pad, uint32_t mappedInput,
                                   int32_t result, uint32_t work) override {
        if(modern_)modern_->Input(pad,mappedInput,result,work);
        ++judgeCalls_;
        if (result == 0) ++judgeAccepted_;
        if (pad != 0u || result == 0 || (judgeCalls_ % 120u) == 0u) {
            Log::Printf(
                "S2 product 80014614 judge count=%llu accepted=%llu pad=0x%04X input=%u result=%d work=%08X",
                static_cast<unsigned long long>(judgeCalls_),
                static_cast<unsigned long long>(judgeAccepted_),
                static_cast<unsigned>(pad & 0xFFFFu),
                static_cast<unsigned>(mappedInput), result, work);
        }
    }

    int32_t VideoDependency(uint32_t function,
                            std::initializer_list<uint32_t> args) override {
        // These are the explicit non-PSX platform return values used by the
        // original startup ABI. They do not acknowledge graphics, CD, audio,
        // file or game completion and are only reached for the BIOS/PAD/GTE
        // calls that the native host owns.
        const std::vector<uint32_t> a(args);
        auto cstring = [this](uint32_t address) {
            std::string value;
            for (uint32_t i=0; i<256u; ++i) {
                const char c=static_cast<char>(Read8(address+i));
                if (!c) break;
                value.push_back(c);
            }
            return value;
        };
        // The translated S2 card routines retain the original BIOS seams.
        // Host files only complete the operation; the translated event polls
        // below still decide success/failure.
        if (function == 0x800170C4u && a == std::vector<uint32_t>{1u}) {
            static constexpr uint32_t kSwClass=0xF4000001u;
            static constexpr uint32_t kHwClass=0xF0000011u;
            static constexpr uint32_t kSpecs[4]={4u,0x8000u,0x100u,0x2000u};
            (void)SpuEvents().EnterCritical();
            for (uint32_t i=0;i<4u;++i) {
                Write32(0x8006EA40u+664u+4u*i,SpuEvents().Open(kSwClass,kSpecs[i],0x2000u,0u));
                Write32(0x8006EA40u+680u+4u*i,SpuEvents().Open(kHwClass,kSpecs[i],0x2000u,0u));
            }
            SpuEvents().ExitCritical();
            for (uint32_t i=0;i<8u;++i) {
                const uint32_t handle=Read32(0x8006EA40u+664u+4u*(i%4u)+(i>=4u?16u:0u));
                (void)SpuEvents().Enable(handle);
            }
            return 1;
        }
        if (function == 0x8001724Cu && a.empty()) {
            (void)SpuEvents().EnterCritical();
            for (uint32_t i=0;i<8u;++i)
                (void)SpuEvents().Close(Read32(0x8006EA40u+664u+4u*(i%4u)+(i>=4u?16u:0u)));
            SpuEvents().ExitCritical();
            return 1;
        }
        if (function == 0x80047EA4u && a == std::vector<uint32_t>{0u}) {
            PrMemCardBackend::SlotInfo slots[15]{};
            const bool present=PrMemCardBackend::EnumerateSlots(slots);
            SpuEvents().Deliver(0xF4000001u,present?4u:0x100u);
            return present?0:-1;
        }
        if (function == 0x80047EB4u && a == std::vector<uint32_t>{0u}) {
            PrMemCardBackend::SlotInfo slots[15]{};
            const bool present=PrMemCardBackend::EnumerateSlots(slots);
            SpuEvents().Deliver(0xF4000001u,present?4u:0x100u);
            return present?0:-1;
        }
        if (function == 0x80048A60u && a.size()==2u) {
            const std::string path=cstring(a[0]);
            PrMemCardBackend::SlotInfo slots[15]{};
            if (!PrMemCardBackend::EnumerateSlots(slots)) return -1;
            const int32_t fd=nextCardFd_++;
            cardFiles_[fd]=path;
            return fd;
        }
        if (function == 0x80048A80u && a.size()==3u) {
            const auto it=cardFiles_.find(static_cast<int32_t>(a[0]));
            if (it==cardFiles_.end()) return -1;
            const uint32_t bytes=a[2];
            if (bytes!=0x2000u) return -1;
            std::vector<uint8_t> block(bytes);
            for (uint32_t i=0; i<bytes; ++i) block[i]=Read8(a[1]+i);
            const std::size_t colon=it->second.find(':');
            const std::string name=colon==std::string::npos ? it->second : it->second.substr(colon+1);
            const auto saved=PrMemCardBackend::SaveEntryBlockByName(name.c_str(),block.data(),block.size());
            SpuEvents().Deliver(0xF4000001u,saved.saved?4u:0x100u);
            return saved.saved?static_cast<int32_t>(bytes):-1;
        }
        if (function == 0x80048A90u && a.size()==1u) {
            cardFiles_.erase(static_cast<int32_t>(a[0]));
            return 1;
        }
        if (function == 0x80048AB0u && a.size()==1u) {
            const bool ok=PrMemCardBackend::FormatPrimaryCard();
            SpuEvents().Deliver(0xF0000011u,ok?4u:0x100u);
            return ok?0:-1;
        }
        if (function == 0x800178C8u && a.empty()) {
            PrMemCardBackend::SlotInfo slots[15]{};
            for (uint32_t i=0; i<600u; ++i) Write8(0x8007A318u+i,0u);
            if (!PrMemCardBackend::EnumerateSlots(slots)) return 0;
            uint32_t count=0;
            for (uint32_t i=0; i<15u; ++i) if (slots[i].occupied) {
                const uint32_t entry=0x8007A318u+40u*i;
                for (uint32_t j=0;j<40u;++j) Write8(entry+j,0u);
                for (uint32_t j=0;j<slots[i].internalName.size() && j<20u;++j)
                    Write8(entry+j,static_cast<uint8_t>(slots[i].internalName[j]));
                Write32(entry+24u,8192u);
                ++count;
            }
            return static_cast<int32_t>(count);
        }
        if (function == 0x80026B94u &&
            a == std::vector<uint32_t>{4u, 0u}) {
            Log::Printf("S2 product retry begin updates=%llu score=%d rating=%u",
                        inputReads_,static_cast<int32_t>(Read32(0x801C3670u)),
                        unsigned(Read16(0x801C368Eu)));
            const int32_t result=PrStage2RetryDirect::RunRetry80026B94(*this,a[1]);
            Log::Printf("S2 product retry end result=%d choice=%d updates=%llu",
                        result,static_cast<int32_t>(Read32(0x8006ED74u)),inputReads_);
            return result;
        }
        // 80017524 re-enters InitPad800354C0 when opening the save menu.
        // The Windows PAD source remains bound for the entire session, so
        // these same platform seams must remain available after startup.
        if (function == 0x800489F0u && a == std::vector<uint32_t>{0x20000001u, 0x800882F0u}) return -31;
        if (function == 0x80048AE0u && a == std::vector<uint32_t>{0u}) return -77;
        if (coldPrerequisites_) {
            if (function == 0x80047F5Cu && a == std::vector<uint32_t>{0x80055FB0u}) return 0;
            if (function == 0x80048A30u && a == std::vector<uint32_t>{0x80055FB0u}) return -73;
            if (function == 0x80048AF0u && a == std::vector<uint32_t>{3u, 0u}) return -11;
            if (function == 0x80048960u && a.empty()) return -99;
            if (function == 0x80048A50u && a.empty()) return -17;
            if (function == 0x80048950u && a.size() == 1u) return -47;
            if (function == 0x80047FFCu) return -19;
        }
        throw D::Unbound(function);
    }

private:
    void CaptureDrawOrder(uint32_t work) {
        wchar_t directory[32768]{};
        const DWORD size = GetEnvironmentVariableW(L"PARAPPA_S2_DRAW_TRACE", directory, 32768);
        if (!size || size >= 32768) return;
        const auto root = std::filesystem::path(directory);
        std::filesystem::create_directories(root);
        const auto stem = "draw_" + std::to_string(inputReads_);
        // Optional read-only replay evidence: unknown RAM bytes retain a
        // separate provenance mask and are never consumed as numeric data.
        std::vector<uint32_t> ram(0x200000u/4u);
        std::vector<uint8_t> known(ram.size());
        for(uint32_t i=0;i<ram.size();++i){
            // A private native RECT has no numeric PSX pointer; omit that
            // identity slot from this optional RAM dump rather than read it.
            if(4u*i==0x5D82Cu)continue;
            const auto word=ReadSourceWord32(0x80000000u+4u*i);
            ram[i]=word.value;known[i]=word.known;
        }
        const auto binary=[&](const char* suffix,const void* bytes,size_t count){
            std::ofstream out(root/(stem+suffix),std::ios::binary);
            out.write(static_cast<const char*>(bytes),static_cast<std::streamsize>(count));
            if(!out)throw std::runtime_error("S2 draw snapshot cannot be written");
        };
        binary("_ram.bin",ram.data(),ram.size()*sizeof(uint32_t));
        binary("_known.bin",known.data(),known.size());
        binary("_vram.bin",Vram().Words().data(),Vram().Words().size()*sizeof(uint16_t));
        std::ofstream file(root / ("draw_" + std::to_string(inputReads_) + ".json"));
        if (!file) throw std::runtime_error("S2 draw trace cannot be written");
        const uint32_t slot = Read32(0x8006EDA8u);
        const uint32_t ot = 0x801D29A8u + 20u * slot;
        const auto batch = PrStage2OtDrawBackend::DecodeLinkedList(
            *this, Read32(ot + 16u), Gpu().DrawState());
        const auto& tmdTrace = PrStage2LifecycleDirect::GetTmdPacketTrace();
        const auto findTmdTrace = [&](uint32_t packet)
            -> const PrStage2LifecycleDirect::TmdPacketTrace* {
            for (const auto& trace : tmdTrace)
                if (trace.packet == packet) return &trace;
            return nullptr;
        };
        file << "{\"updates\":" << inputReads_ << ",\"tick\":" << Read32(work+12u)
             << ",\"slot\":" << slot << ",\"commands\":[";
        bool first = true;
        for (const auto& command : batch.commands) {
            if (!first) file << ',';
            first = false;
            const auto* tmd = findTmdTrace(command.packet);
            file << "{\"packet\":" << command.packet << ",\"op\":" << unsigned(command.opcode)
                 << ",\"tpage\":" << command.state.tpage << ",\"clut\":" << command.clut
                 << ",\"offset\":[" << command.state.offsetX << ',' << command.state.offsetY << ']'
                 << ",\"tmd\":";
            if (!tmd) {
                file << "null";
            } else {
                file << "{\"descriptor\":" << tmd->descriptor
                     << ",\"object\":" << tmd->object
                     << ",\"primitive\":" << tmd->primitive
                     << ",\"mode\":" << tmd->primitiveMode
                     << ",\"handler\":" << tmd->handler
                     << ",\"ot\":" << tmd->ot
                     << ",\"bucket\":" << tmd->bucket
                     << ",\"otz\":" << tmd->otz
                     << ",\"bias\":" << tmd->bias
                     << ",\"shift\":" << tmd->shift
                     << ",\"length\":" << tmd->length << '}';
            }
            file << ",\"xyuv\":[";
            for (unsigned i=0; i<command.vertices; ++i) {
                if (i) file << ',';
                const auto& v=command.vertex[i];
                file << '[' << v.x << ',' << v.y << ',' << unsigned(v.u) << ',' << unsigned(v.v)
                     << ',' << unsigned(v.r) << ',' << unsigned(v.g) << ',' << unsigned(v.b) << ']';
            }
            file << "]}";
        }
        file << "]}";
        Log::Printf("S2 draw trace updates=%llu commands=%zu", inputReads_, batch.commands.size());
    }
    std::unique_ptr<PrStage2ModernPresentation> modern_;
    std::vector<IntFileEntry> residentTimUploads_;
    bool coldPrerequisites_ = true;
    std::function<uint32_t()> debugPadSource_;
    bool commonLoaded_ = false;
    uint64_t inputReads_ = 0;
    uint64_t inputNonzeroReads_ = 0;
    uint64_t judgeCalls_ = 0;
    uint64_t judgeAccepted_ = 0;
    bool gameStreamHeld_ = false;
    bool gameplayFrameAllowed_ = true;
    bool firstGameInput_ = false;
    uint32_t clearRevealFrames_ = 0u;
    bool transitionPresentationCompleted_ = false;
    std::optional<uint64_t> movieCallbackVblank_;
    uint64_t lastMovieCallbackVblank_ = 0;
    std::unordered_map<int32_t,std::string> cardFiles_;
    int32_t nextCardFd_ = 0x200;
};

std::filesystem::path ExistingDisc(const std::filesystem::path& dataRoot) {
    const std::array<std::filesystem::path, 3> candidates{{
        dataRoot.parent_path() / "PaRappa the Rapper.bin",
        dataRoot / "PaRappa the Rapper.bin",
        std::filesystem::current_path() / "PaRappa the Rapper.bin"}};
    for (const auto& candidate : candidates)
        if (std::filesystem::is_regular_file(candidate)) return candidate;
    // Keep the diagnostic path deterministic; the CD device will report the
    // missing source when the first real lookup is attempted.
    return candidates[0];
}

} // namespace

struct Runtime::Impl {
    enum class Phase { Created, Graphics, ReadyForInit, Initializing, ReadyForScene,
                       Running, DrainingGraphics, Returned };

    std::filesystem::path dataRoot;
    PrGameContext& context;
    PrStage2SharedState::Entry entry;
    std::function<uint32_t()> debugPadSource;
    WasapiSink sink;
    ProductSession session;
    std::unique_ptr<PrPsxGraphOwnerDirect::PsxGraphState> directoryGraph;
    PsxVramAtlas directoryAtlas;
    Phase phase = Phase::Created;
    int32_t sceneResult = 2;
    std::chrono::steady_clock::time_point lastPacingLog{};
    uint64_t pollCount = 0;
    int previousThreadPriority = THREAD_PRIORITY_ERROR_RETURN;
    bool threadPriorityRaised = false;

    Impl(const std::filesystem::path& root, D3D11Renderer& renderer, PrGameContext& ctx,
         std::function<uint32_t()> debugPad)
        : dataRoot(root), context(ctx),
          debugPadSource(std::move(debugPad)),
          session(root / "S2" / "S2_NATIVE_DATA.BIN",
                  root / "S2" / "COMOD2.BIN",
                  ExistingDisc(root), renderer, sink, ctx) {}

    ~Impl() {
        if (threadPriorityRaised) {
            SetThreadPriority(GetCurrentThread(), previousThreadPriority);
        }
    }

    void Begin() {
        if (phase != Phase::Created) return;
        // The native S2 VBlank, CD producer and retained source continuation
        // share this host thread. Keep ordinary desktop scheduling from
        // withholding two refresh edges at a time, then restore the caller's
        // priority when the product session ends.
        previousThreadPriority = GetThreadPriority(GetCurrentThread());
        if (previousThreadPriority == THREAD_PRIORITY_NORMAL ||
            previousThreadPriority == THREAD_PRIORITY_BELOW_NORMAL) {
            threadPriorityRaised =
                SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL) != 0;
        }
        PrPad::Init();
        Log::Printf("S2 product Begin before movie backend state=%u initStarts=%u",
                    static_cast<unsigned>(session.State()),
                    static_cast<unsigned>(session.InitializationStarts()));
        session.EnableSharedMovieOutput();
        session.SetDebugPadSource(debugPadSource);
        Log::Printf("S2 product shared movie backend selected");
        session.EnableMovieForeground([this] {
            const uint32_t first = PrPsxPadDirect::NormalizeLocalPrPadMaskToReturnedMask80035510(
                PrPad::GetState(0).held);
            const uint32_t second = PrPsxPadDirect::NormalizeLocalPrPadMaskToReturnedMask80035510(
                PrPad::GetState(1).held);
            const uint32_t result = first | (second << 16u);
            static uint32_t last = 0u;
            if (result != last && result != 0u) {
                Log::Printf("S2 product input mask=0x%08X local0=0x%04X local1=0x%04X debug=0x%04X",
                            result, first, second,
                            0u);
            }
            last = result;
            return result;
        });
        Log::Printf("S2 product movie foreground selected");
        // Product scanout must wait for the current native D3D fence. The
        // isolated GPU contract intentionally retains its submitted-page path.
        session.SetProductPresentationRequiresIdleGpu(true);
        session.StartGraphics();
        Log::Printf("S2 product graphics started state=%u",
                    static_cast<unsigned>(session.State()));
        phase = Phase::Graphics;
    }

    void PumpOne() {
        if (phase == Phase::Created || phase == Phase::Returned) return;
        session.RethrowFailure();
        const auto before = session.State();
        if (before == F::State::Waiting) {
            const auto pending = session.Pending();
            const auto now = std::chrono::steady_clock::now();
            // 设备轮询已与刷新率解耦；按墙钟限流日志，避免诊断 I/O 改变节奏。
            if (pollCount < 8 || now - lastPacingLog >= std::chrono::seconds(1)) {
                lastPacingLog = now;
                Log::Printf("S2 product wait phase=%u kind=%u fn=%08X target=%u count=%llu vblank=%llu nativeVblank=%u nativePrev=%u irq=%llu presents=%llu gpuCommands=%llu",
                            static_cast<unsigned>(phase),
                            static_cast<unsigned>(pending.kind), pending.function,
                            pending.target,
                            static_cast<unsigned long long>(pollCount),
                            static_cast<unsigned long long>(session.WallClockVBlankEvents()),
                            static_cast<unsigned>(session.Read32(0x80057034u)),
                            static_cast<unsigned>(session.Read32(0x80055F74u)),
                            static_cast<unsigned long long>(session.NativeIrqEntries()),
                            static_cast<unsigned long long>(session.Gpu().DisplayPresents()),
                            static_cast<unsigned long long>(session.Gpu().CommandsRendered()));
                Log::Printf("S2 pacing sectors=%llu xa=%llu decoded=%llu published=%llu audioQueued=%llu audioConsumed=%llu audioUnderflow=%llu animationFrame=%u gameUpdates=%llu movieCopies=%llu",
                            static_cast<unsigned long long>(session.Cd().SectorReads()),
                            static_cast<unsigned long long>(session.Cd().AudioSectors()),
                            static_cast<unsigned long long>(session.MovieOutput().DecodedFrames()),
                            static_cast<unsigned long long>(session.MovieOutput().PublishedFrames()),
                            static_cast<unsigned long long>(session.Spu().CdFramesQueued()),
                            static_cast<unsigned long long>(session.Spu().CdFramesConsumed()),
                            static_cast<unsigned long long>(session.Spu().CdUnderflowFrames()),
                            static_cast<unsigned>(session.Read32(0x8006EA54u)),
                            session.GameUpdates(),session.Gpu().CpuImageCopies());
            }
            ++pollCount;
            session.Poll();
        }
        session.RethrowFailure();
        if (session.State() != F::State::Completed) return;
        if (phase == Phase::Graphics) {
            phase = Phase::ReadyForInit;
            Log::Printf("S2 product graphics ready");
        } else if (phase == Phase::Initializing) {
            phase = Phase::ReadyForScene;
            Log::Printf("S2 product initializer returned");
        } else if (phase == Phase::Running) {
            sceneResult = session.TaskResult();
            if(entry.mode==2u) {
                // The source has restored the pre-replay bank in this RAM.
                // Publish that same restoration to the shared owner, retaining
                // its original typed backup and replay provenance.
                const auto current=PrStage1SaveUiDirect::GetSharedPayloadForStageEntry();
                if(current.savePayloadBank!=entry.payload.savePayloadBank ||
                   current.saveStatusBackup!=entry.payload.saveStatusBackup)
                    throw std::runtime_error("Shared progress changed during S2 replay");
                for(uint32_t i=0;i<entry.payload.saveStatusBackup.size();++i)
                    if(session.Read8(0x80092F10u+i)!=entry.payload.saveStatusBackup[i])
                        throw std::runtime_error("S2 replay returned without restoring its backup");
                const auto restored=PrStage1SaveUiDirect::Sub80015744(0x80092F10u);
                if(!restored.ok || !restored.restoreKnown || restored.helperGap)
                    throw std::runtime_error("S2 shared replay backup restoration failed");
            }
            context.transitionState=static_cast<int16_t>(session.Read16(0x800916D0u));
            context.transitionStateDA=static_cast<int16_t>(session.Read16(0x800916DAu));
            context.sceneExitReason=static_cast<int16_t>(session.Read16(0x800916E0u));
            phase = Phase::DrainingGraphics;
            session.StartGraphicsHandoff();
        } else if (phase == Phase::DrainingGraphics) {
            phase = Phase::Returned;
            Log::Printf("S2 product scene returned result=%d", sceneResult);
        }
    }

    int Tick() {
        Begin();
        PumpOne();
        if (phase == Phase::ReadyForInit) {
            entry.mode=static_cast<uint16_t>(context.transitionState);
            entry.easy=static_cast<uint16_t>(context.transitionStateDA);
            entry.language=static_cast<uint16_t>(context.languageIndex);
            entry.subtitles=static_cast<uint16_t>(context.subtitleFlag);
            entry.exitReason=static_cast<uint16_t>(context.sceneExitReason);
            entry.savePolicyKnown=PrSS0Scene0RuntimeDirect::TryReadWord800916F0Software(entry.savePolicy);
            entry.payload=PrStage1SaveUiDirect::GetSharedPayloadForStageEntry();
            PrStage2SharedState::ImportEntry(session,entry);
            Log::Printf("S2 shared entry mode=%u easy=%u language=%u subtitles=%u savePolicy=%u stage1=%u stage2=%u replayCount=%u backup=%u",
                entry.mode,entry.easy,entry.language,entry.subtitles,entry.savePolicy,
                unsigned(session.Read8(0x80092F1Du)),unsigned(session.Read8(0x80092F1Eu)),
                session.Read32(0x80092F48u),unsigned(entry.payload.saveStatusBackupKnown80079008));
            session.LoadCommonResident(dataRoot / "S0" / "COMPO00.INT");
            // The native scene descriptors at 8004E6B0..8004E7C0 point into
            // COMPO02's indexed pages. Keep those real TIM writes in the same
            // S2 VRAM owner before the first translated gameplay frame.
            session.LoadStageResidentTims(dataRoot / "S2" / "COMPO02.INT");
            session.SetColdPrerequisites(false);
            session.StartInitialization();
            Log::Printf("S2 product initialization started state=%u",
                        static_cast<unsigned>(session.State()));
            phase = Phase::Initializing;
            PumpOne();
        }
        if (phase == Phase::ReadyForScene) {
            session.StartScene();
            Log::Printf("S2 product scene started state=%u",
                        static_cast<unsigned>(session.State()));
            phase = Phase::Running;
            PumpOne();
        }
        if (phase == Phase::Returned) {
            return sceneResult;
        }
        return 2;
    }
};

Runtime::Runtime(const std::filesystem::path& dataRoot, D3D11Renderer& renderer,
                 PrGameContext& context,
                 std::function<uint32_t()> debugPadSource)
    : impl_(std::make_unique<Impl>(dataRoot, renderer, context,
                                   std::move(debugPadSource))) {}
Runtime::~Runtime() = default;
int Runtime::Tick() { return impl_->Tick(); }
// 一个宿主空闲步可完成几次设备交接，但不额外推进场景 Tick。
// 同时限制次数和墙钟预算，避免轮询霸占消息循环；原 VSync 仍控制帧速。
void Runtime::Pump() {
    if (impl_->phase == Impl::Phase::Returned) return;
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(2);
    for (unsigned pass = 0; pass < 4; ++pass) {
        impl_->PumpOne();
        // Presentation is driven by the wall-clock VBlank request, not by
        // whether the retained native continuation happened to yield on this
        // pass. Keep the real GPU fence/display checks in PresentIfReady().
        impl_->session.PresentIfReady();
        if (std::chrono::steady_clock::now() >= deadline) break;
    }
}
bool Runtime::OwnsPresentation() const noexcept {
    return impl_->phase != Impl::Phase::Created && impl_->phase != Impl::Phase::Returned;
}
bool Runtime::Running() const noexcept {
    return impl_->phase == Impl::Phase::Running || impl_->phase == Impl::Phase::ReadyForScene;
}
int16_t Runtime::ExitReason() const {
    return static_cast<int16_t>(impl_->session.Read16(0x800916E0u));
}
bool Runtime::BeginResidentDirectory(PrGameContext& ctx, int previousScene) {
    if (impl_->phase != Impl::Phase::Returned || impl_->sceneResult >= 0) return false;
    if (!impl_->directoryGraph) {
        auto graph = std::make_unique<PrPsxGraphOwnerDirect::PsxGraphState>();
        impl_->session.ProjectDirectory(*graph, impl_->directoryAtlas);
        impl_->directoryGraph = std::move(graph);
        impl_->session.Output().Stop();
    }
    return PrSS0Scene0RuntimeDirect::BeginResidentDirectory80015788(
        ctx, previousScene, impl_->directoryGraph.get(), &impl_->directoryAtlas);
}
void Runtime::RethrowFailure() const { impl_->session.RethrowFailure(); }

} // namespace PrStage2ProductRuntime
