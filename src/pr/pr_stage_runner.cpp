#include "pr_stage_runner.h"
#include "pr_game_context.h"
#include "pr_sqevs1.h"
#include "pr_scn1.h"
#include "pr_beat_chart.h"
#include "pr_pad.h"
#include "pr_stage1_hd_subtitles.h"
#include "pr_ss0_scene0_runtime_direct.h"
#include "pr_tmd_renderer.h"
#include "pr_ui_overlay.h"
#include "xa1_player.h"
#include "d3d11_renderer.h"
#include "logger.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <cstring>
#include <cstdio>
#include <string>

// Global singleton.
static PrStageRunner s_stageRunner;

PrStageRunner& GetStageRunner() {
    return s_stageRunner;
}

// Per-stage fallback duration in frames at 30 fps.
// Real gameplay prefers XA audio duration when available.
static const int kStageDuration[10] = {
    0,      // Scene0 (menu, unused)
    2700,   // Stage1: about 90 seconds
    2400,   // Stage2: about 80 seconds
    2100,   // Stage3: about 70 seconds
    0,      // Scene4 (unused)
    1800,   // Stage5: about 60 seconds
    2400,   // Stage6: about 80 seconds
    3000,   // Stage7: about 100 seconds
    1500,   // Scene8 (ending): about 50 seconds
    1800,   // Stage9 (bonus): about 60 seconds
};

// Stage colors for the background gradient.
static const float kStageColors[10][3] = {
    { 0.1f, 0.1f, 0.2f },  // Scene0
    { 0.2f, 0.15f, 0.1f }, // Stage1: warm brown
    { 0.1f, 0.2f, 0.15f }, // Stage2: green tint
    { 0.15f, 0.1f, 0.2f }, // Stage3: purple tint
    { 0.1f, 0.1f, 0.1f },  // Scene4
    { 0.2f, 0.1f, 0.1f },  // Stage5: red tint
    { 0.1f, 0.15f, 0.2f }, // Stage6: blue tint
    { 0.2f, 0.2f, 0.1f },  // Stage7: yellow tint
    { 0.1f, 0.1f, 0.15f }, // Scene8
    { 0.15f, 0.15f, 0.2f }, // Stage9
};

static uint32_t ReadU32LE(const uint8_t* p) {
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

static bool TryGetXa1DataSize(const std::filesystem::path& path, uint32_t& outDataSize) {
    outDataSize = 0;

    std::ifstream f(path, std::ios::binary);
    if (!f.is_open()) {
        return false;
    }

    uint8_t riff[12] = {};
    f.read((char*)riff, sizeof(riff));
    if (f.gcount() != (std::streamsize)sizeof(riff)) {
        return false;
    }

    if (riff[0] != 'R' || riff[1] != 'I' || riff[2] != 'F' || riff[3] != 'F') {
        return false;
    }
    if (riff[8] != 'C' || riff[9] != 'D' || riff[10] != 'X' || riff[11] != 'A') {
        return false;
    }

    while (true) {
        uint8_t h[8] = {};
        f.read((char*)h, sizeof(h));
        if (f.gcount() != (std::streamsize)sizeof(h)) {
            break;
        }

        const uint32_t chunkSize = ReadU32LE(h + 4);
        const bool isData = (h[0] == 'd' && h[1] == 'a' && h[2] == 't' && h[3] == 'a');
        if (isData) {
            outDataSize = chunkSize;
            return outDataSize > 0;
        }

        std::streamoff skip = (std::streamoff)chunkSize;
        if (chunkSize & 1) {
            skip += 1;
        }
        f.seekg(skip, std::ios::cur);
        if (!f.good()) {
            break;
        }
    }

    return false;
}

static int ComputeTargetFramesFromXa1DataSize(uint32_t dataSize) {
    constexpr int kFps = 30;
    constexpr int kSectorBytes = 2352;
    constexpr int kSectorsPerSecond = 75;

    if (dataSize < (uint32_t)kSectorBytes) {
        return 0;
    }

    const uint32_t sectors = dataSize / (uint32_t)kSectorBytes;
    const double seconds = (double)sectors / (double)kSectorsPerSecond;
    const int frames = (int)(seconds * (double)kFps + 0.5);
    return frames;
}

// PSX pad bit definitions, kept consistent with main.cpp.
static constexpr uint16_t KPAD_CROSS    = 0x0040;
static constexpr uint16_t KPAD_CIRCLE   = 0x0020;
static constexpr uint16_t KPAD_SQUARE   = 0x0080;
static constexpr uint16_t KPAD_TRIANGLE = 0x0010;
static constexpr uint16_t KPAD_BUTTONS  = KPAD_CROSS | KPAD_CIRCLE | KPAD_SQUARE | KPAD_TRIANGLE;

static bool GetStage1ObserverSelectControlBit(const PrGameContext& ctx);

static bool UsesRunnerMeterFailAuthority(int sceneId) {
    // Stage1's formal lifecycle is moving onto Scene1-local runtime gating.
    // Keep the generic runner meter lane alive as a debug-only shadow there.
    return sceneId != 1;
}

PrStageRunner::PrStageRunner()
    : m_state(StageRunnerState::Idle)
    , m_sceneId(0)
    , m_frameCount(0)
    , m_beatCount(0)
    , m_grade(4)  // Default GOOD.
    , m_bpm100(0)
    , m_tickOffset(0)
    , m_tickPerFrame60(0)
    , m_tickAccumNum(0)
    , m_tick96(0)
    , m_bar(0)
    , m_beatInBar(0)
    , m_tickInBeat(0)
    , m_targetFrames(0)
    , m_startAudioSeconds(0.0)
    , m_startAudioSector75(0)
    , m_lastAcceptedXaSector75(0)
    , m_targetSeconds(0.0)
    , m_elapsedSeconds(0.0)
    , m_score(0)
    , m_meter(50)
    , m_combo(0)
    , m_lastJudge(JudgeResult::None)
    , m_judgeFlash(0)
    , m_lastPadMask(0)
    , m_nextChartIndex(0)
{
}

PrStageRunner::~PrStageRunner() {
    Reset();
}

void PrStageRunner::Init(int sceneId, PrGameContext& ctx) {
    m_sceneId = sceneId;
    m_state = StageRunnerState::Playing;
    m_frameCount = 0;
    m_beatCount = 0;
    m_grade = 4;
    m_startAudioSeconds = 0.0;
    m_startAudioSector75 = 0;
    m_lastAcceptedXaSector75 = 0;
    m_targetSeconds = 0.0;
    m_elapsedSeconds = 0.0;

    m_bpm100 = 0;
    m_tickOffset = 0;
    m_tickPerFrame60 = 0;
    m_tickAccumNum = 0;
    m_tick96 = 0;
    m_bar = 0;
    m_beatInBar = 0;
    m_tickInBeat = 0;

    // Reset judgement and scoring state.
    m_score = 0;
    m_meter = 50;
    m_combo = 0;
    m_lastJudge = JudgeResult::None;
    m_judgeFlash = 0;
    m_lastPadMask = 0;
    m_nextChartIndex = 0;
    m_meterZeroFrames = 0;
    m_stage1SpecialFixedBonusBootLatch = false;
    m_stage1NormalizedSelectControlBit = false;
    m_stage1SpecialFixedBonusLatch = false;

    if (sceneId == 1) {
        m_bpm100 = kPrStage1Bpm100;
        m_tickOffset = kPrStage1TicksPerBeat;
        m_tickPerFrame60 = kPrStage1TicksPerRenderFrame60;
        if (m_tickPerFrame60 < 1) {
            m_tickPerFrame60 = 1;
        }
        m_tickAccumNum = m_tickOffset;
        m_tick96 = m_tickOffset;
        RefreshStage1BeatFromTick96();
        // Debug-only observer: keep the special fixed bonus gate at a local
        // entry-sampled control-bit/latch layer instead of gameplay authority.
        m_stage1SpecialFixedBonusBootLatch = GetStage1ObserverSelectControlBit(ctx);
        m_stage1NormalizedSelectControlBit = m_stage1SpecialFixedBonusBootLatch;
    }

    // Load the generic beat chart only for non-Stage1 fallback stages. Stage1
    // uses the direct scorer path; leaving the old preset loaded makes stale
    // BeatChart state look authoritative in debug/UI callers.
    if (sceneId == 1) {
        GetBeatChart().Clear();
    } else {
        GetBeatChart().LoadPreset(sceneId);
        GetBeatChart().ResetHitState();
    }
    
    // Set stage duration.
    m_targetFrames = 0;
    if (!ctx.currentXaPath.empty()) {
        uint32_t dataSize = 0;
        if (TryGetXa1DataSize(ctx.currentXaPath, dataSize)) {
            const int frames = ComputeTargetFramesFromXa1DataSize(dataSize);
            if (frames > 0) {
                m_targetFrames = frames;
                Log::Printf("StageRunner::Init sceneId=%d xa='%s' dataSize=%u targetFrames=%d",
                            sceneId,
                            ctx.currentXaPath.u8string().c_str(),
                            (unsigned)dataSize,
                            m_targetFrames);
            }
        }
    }

    if (m_targetFrames <= 0) {
        if (sceneId >= 0 && sceneId < 10) {
            m_targetFrames = kStageDuration[sceneId];
        } else {
            m_targetFrames = 1800;
        }
        Log::Printf("StageRunner::Init sceneId=%d targetFrames=%d", sceneId, m_targetFrames);
    }

    m_targetSeconds = (m_targetFrames > 0) ? ((double)m_targetFrames / 30.0) : 0.0;

    if (ctx.xa1Player && ctx.xa1Player->IsPlaying()) {
        m_startAudioSeconds = ctx.xa1Player->GetPlayedSeconds();
        m_startAudioSector75 = ctx.xa1Player->GetCdClockSectorIndex();
    }

    if (sceneId == 1) {
        PrStage1HdSubtitles::Preload(ctx);
    }
}

int PrStageRunner::Update(PrGameContext& ctx) {
    if (m_state != StageRunnerState::Playing) {
        return 1; // Already finished.
    }

    // F5 remains a non-Stage1 debug shortcut. Stage1 clear authority comes
    // from the direct stream2/3 terminal flag100 path; letting F5 mutate the
    // runner state would freeze the direct Stage1 loop after the host result
    // is correctly ignored by lifecycle.
    if (ctx.debugF5_StageClear) {
        if (m_sceneId == 1) {
            Log::Printf("StageRunner: ignored Stage1 F5 debug clear at frame %d",
                        m_frameCount);
            return 0;
        }
        Log::Printf("StageRunner: F5 debug clear at frame %d", m_frameCount);
        m_state = StageRunnerState::Cleared;
        return 1;
    }

    double relSeconds = (double)m_frameCount / 30.0;
    if (m_sceneId == 1) {
        m_frameCount++;
        relSeconds = (double)m_frameCount / 30.0;
        UpdateStage1HybridTimecode(ctx);
    } else if (ctx.xa1Player && ctx.xa1Player->IsPlaying()) {
        relSeconds = ctx.xa1Player->GetPlayedSeconds() - m_startAudioSeconds;
        if (relSeconds < 0.0) relSeconds = 0.0;

        const int audioFrame = (int)std::floor(relSeconds * 30.0 + 1e-9);
        if (audioFrame > m_frameCount) {
            m_frameCount = audioFrame;
        }
    } else {
        m_frameCount++;
        relSeconds = (double)m_frameCount / 30.0;
    }
    m_elapsedSeconds = relSeconds;

    UpdateBeat(relSeconds);
    UpdateStage1SpecialFixedBonusObserver(ctx);

    if (m_sceneId != 1) {
        // Process input/judgement
        ProcessInput(ctx);

        // Decay judge flash
        if (m_judgeFlash > 0) {
            m_judgeFlash--;
        }

        // Update grade from meter
        m_grade = MeterToGrade(m_meter);

        if (m_meter <= 0) {
            m_meterZeroFrames++;
        } else {
            m_meterZeroFrames = 0;
        }

        if (UsesRunnerMeterFailAuthority(m_sceneId) && m_meterZeroFrames >= 60) {
            Log::Printf("StageRunner: Stage failed at frame %d meter=%d", m_frameCount, m_meter);
            m_state = StageRunnerState::Failed;
            return 1;
        }
    }

    // Check stage completion. Stage1 completion is driven by the direct
    // stream2/3 terminal flag100 path, not by host XA/duration. Keep the host
    // runner advancing so duration cannot leak into runnerResult observers.
    if (m_targetSeconds > 0.0 && relSeconds >= m_targetSeconds) {
        if (m_sceneId == 1) {
            return 0;
        }
        Log::Printf("StageRunner: Stage complete at frame %d", m_frameCount);
        if (m_sceneId != 1) {
            m_state = StageRunnerState::Cleared;
        }
        return 1;
    }
    
    return 0; // Continue.
}

void PrStageRunner::SyncXaStartBaseline(PrGameContext& ctx) {
    if (m_state != StageRunnerState::Playing ||
        !ctx.xa1Player ||
        !ctx.xa1Player->IsPlaying()) {
        return;
    }

    if (m_frameCount == 0 && m_elapsedSeconds <= 0.0) {
        m_startAudioSeconds = ctx.xa1Player->GetPlayedSeconds();
        m_startAudioSector75 = ctx.xa1Player->GetCdClockSectorIndex();
        m_lastAcceptedXaSector75 = 0;
    }
}

void PrStageRunner::Render(PrGameContext& ctx) {
    if (!ctx.renderer) return;

    // Stage COMPO TMD/TIM resources are the visual authority.  The previous
    // gradient/progress widgets are retained only as an explicit diagnostic
    // fallback when the original stage model directory could not be loaded.
    if (m_sceneId != 1 && PrTmdRenderer::RenderStage(ctx, 0.0f, 0.0f, 1.0f)) {
        return;
    }

    if (m_sceneId != 1) {
        RenderBackground(ctx);
    }
    RenderHUD(ctx);
}

static bool GetStage1ObserverSelectControlBit(const PrGameContext& ctx) {
    constexpr uint16_t kPsxSelectMask80035510 = 0x0100u;
    const PrPadState pad = PrPad::GetState(0);
    return (pad.held & (uint16_t)PrPadButton::Select) != 0u ||
           (ctx.debugPadInput & kPsxSelectMask80035510) != 0u;
}

static void RenderStageRunnerTextBox(PrGameContext& ctx,
                                     const char* text,
                                     bool drawText) {
    if (!ctx.renderer || !text || text[0] == '\0') {
        return;
    }

    const float winW = (float)ctx.renderer->GetWidth();
    const float winH = (float)ctx.renderer->GetHeight();

    const float subW = winW * 0.8f;
    const float subX = (winW - subW) / 2;
    const float baseSubY = winH - 100.0f;

    int lineCount = 1;
    for (const char* p = text; *p; ++p) {
        if (*p == '\n') {
            lineCount++;
        }
    }
    if (lineCount < 1) {
        lineCount = 1;
    }

    const float textScale = 1.6f;
    const float lineH = 15.0f * textScale;
    const float padX = 8.0f;
    const float padY = 8.0f;
    const float subH = (lineCount <= 1) ? 36.0f : (padY * 2.0f + (float)lineCount * lineH);
    const float subY = (lineCount <= 1) ? baseSubY : (baseSubY - (subH - 36.0f));

    ctx.renderer->DrawRect(subX, subY, subW, subH, 0.0f, 0.0f, 0.0f, 0.7f);

    float textY = (lineCount <= 1) ? (subY + 10.0f) : (subY + padY);
    const char* lineStart = text;
    for (int li = 0; li < lineCount; li++) {
        const char* lineEnd = std::strchr(lineStart, '\n');
        const size_t len = (lineEnd != nullptr) ? (size_t)(lineEnd - lineStart) : std::strlen(lineStart);
        std::string line(lineStart, len);
        const float lineW = PrUiOverlay::MeasureSubtitleTextNative(textScale, line.c_str());
        float textX = subX + (subW - lineW) * 0.5f;
        if (textX < subX + padX) {
            textX = subX + padX;
        }

        const float drawY = textY + (float)li * lineH;
        PrStage1HdSubtitles::ObserveNativeSubtitleTextRect(
            ctx,
            PrStage1HdSubtitleSourceKind::OverlayScriptText,
            textX,
            drawY,
            lineW,
            lineH);
        if (drawText) {
            if (!PrUiOverlay::DrawSubtitleTextNative(ctx, textX, drawY, textScale, line.c_str(), 1.0f, 1.0f, 1.0f, 1.0f)) {
                PrUiOverlay::DrawTextUi(textX, drawY, textScale, line.c_str(), 1.0f, 1.0f, 1.0f, 1.0f);
            }
        }

        if (lineEnd == nullptr) {
            break;
        }
        lineStart = lineEnd + 1;
    }
}

void PrStageRunner::Reset() {
    m_state = StageRunnerState::Idle;
    m_sceneId = 0;
    m_frameCount = 0;
    m_beatCount = 0;
    m_grade = 4;
    m_bpm100 = 0;
    m_tickOffset = 0;
    m_tickPerFrame60 = 0;
    m_tickAccumNum = 0;
    m_tick96 = 0;
    m_bar = 0;
    m_beatInBar = 0;
    m_tickInBeat = 0;
    m_targetFrames = 0;
    m_startAudioSeconds = 0.0;
    m_startAudioSector75 = 0;
    m_lastAcceptedXaSector75 = 0;
    m_targetSeconds = 0.0;
    m_elapsedSeconds = 0.0;
    m_score = 0;
    m_meter = 50;
    m_combo = 0;
    m_lastJudge = JudgeResult::None;
    m_judgeFlash = 0;
    m_lastPadMask = 0;
    m_nextChartIndex = 0;
    m_meterZeroFrames = 0;
    m_stage1SpecialFixedBonusBootLatch = false;
    m_stage1NormalizedSelectControlBit = false;
    m_stage1SpecialFixedBonusLatch = false;
}

float PrStageRunner::GetProgress() const {
    if (m_targetFrames <= 0) return 0.0f;
    return (float)m_frameCount / (float)m_targetFrames;
}

void PrStageRunner::RefreshStage1BeatFromTick96() {
    m_bar = m_tick96 / kPrStage1TicksPerBar + 1;
    m_beatInBar =
        (m_tick96 % kPrStage1TicksPerBar) / kPrStage1TicksPerBeat + 1;
    m_tickInBeat = (m_tick96 % kPrStage1TicksPerBeat) + 1;
    m_beatCount = m_tick96 / kPrStage1TicksPerBeat;
}

void PrStageRunner::UpdateStage1HybridTimecode(PrGameContext& ctx) {
    const int tickPerMinute = (kPrStage1TicksPerBeat * m_bpm100) / 100;
    const int fallbackTickAdvance = m_tickPerFrame60 * 2;
    int relativeSector75 = -1;

    if (ctx.xa1Player && ctx.xa1Player->IsPlaying()) {
        relativeSector75 =
            ctx.xa1Player->GetCdClockSectorIndex() - m_startAudioSector75;
        if (relativeSector75 < 0) {
            relativeSector75 = 0;
        }
    }

    if (relativeSector75 > m_lastAcceptedXaSector75) {
        const double tickFromXa =
            ((double)relativeSector75 * (double)tickPerMinute) / 4500.0;
        m_tick96 = m_tickOffset + (int)std::floor(tickFromXa + 0.5);
        m_tickAccumNum = (int64_t)m_tick96;
        m_lastAcceptedXaSector75 = relativeSector75;
    } else {
        m_tickAccumNum += (int64_t)fallbackTickAdvance;
        m_tick96 = (int)m_tickAccumNum;
    }

    RefreshStage1BeatFromTick96();
}

void PrStageRunner::UpdateBeat(double relSeconds) {
    if (m_sceneId == 1 && m_bpm100 > 0) {
        RefreshStage1BeatFromTick96();
        return;
    }

    if (m_bpm100 <= 0) {
        m_tick96 = (int)std::floor(relSeconds * 96.0 * 2.0 + 1e-9);
        m_bar = m_tick96 / kPrStage1TicksPerBar + 1;
        m_beatInBar =
            (m_tick96 % kPrStage1TicksPerBar) / kPrStage1TicksPerBeat + 1;
        m_tickInBeat = (m_tick96 % kPrStage1TicksPerBeat) + 1;
        m_beatCount = m_tick96 / kPrStage1TicksPerBeat;
        if ((m_frameCount % 15) == 0 && m_beatCount < 1) {
            m_beatCount = 1;
        }
        return;
    }

    const double tickFromStartD =
        relSeconds * (double)m_bpm100 * (double)kPrStage1TicksPerBeat /
        (60.0 * 100.0);
    const int tickFromStart = (tickFromStartD > 0.0) ? (int)std::floor(tickFromStartD + 1e-9) : 0;
    m_tick96 = m_tickOffset + tickFromStart;

    m_bar = m_tick96 / kPrStage1TicksPerBar + 1;
    m_beatInBar =
        (m_tick96 % kPrStage1TicksPerBar) / kPrStage1TicksPerBeat + 1;
    m_tickInBeat = (m_tick96 % kPrStage1TicksPerBeat) + 1;
    m_beatCount = m_tick96 / kPrStage1TicksPerBeat;
}

void PrStageRunner::RenderBackground(PrGameContext& ctx) {
    if (!ctx.renderer) return;
    
    const float winW = (float)ctx.renderer->GetWidth();
    const float winH = (float)ctx.renderer->GetHeight();
    
    // Stage color
    int colorIdx = (m_sceneId >= 0 && m_sceneId < 10) ? m_sceneId : 0;
    const float r = kStageColors[colorIdx][0];
    const float g = kStageColors[colorIdx][1];
    const float b = kStageColors[colorIdx][2];
    
    // Draw gradient background
    ctx.renderer->DrawRect(0, 0, winW, winH * 0.6f, r * 0.5f, g * 0.5f, b * 0.5f, 1.0f);
    ctx.renderer->DrawRect(0, winH * 0.6f, winW, winH * 0.4f, r * 1.5f, g * 1.5f, b * 1.5f, 1.0f);
    
    // Horizon line
    ctx.renderer->DrawRect(0, winH * 0.6f - 2, winW, 4, r * 2.0f, g * 2.0f, b * 2.0f, 0.8f);
    
    // Beat bar
    const float beatBarH = 8.0f;
    float beatPhase = 0.0f;
    if (m_bpm100 > 0) {
        beatPhase =
            (float)(m_tickInBeat - 1) / (float)kPrStage1TicksPerBeat;
    } else {
        beatPhase = (float)(m_frameCount % 15) / 15.0f;
    }
    const float beatW = winW * 0.6f;
    const float beatX = (winW - beatW) / 2;
    
    // Beat bar background
    ctx.renderer->DrawRect(beatX, winH - 60, beatW, beatBarH, 0.2f, 0.2f, 0.2f, 0.8f);
    // Beat progress
    ctx.renderer->DrawRect(beatX, winH - 60, beatW * beatPhase, beatBarH, 0.8f, 0.6f, 0.2f, 1.0f);
}

void PrStageRunner::RenderHUD(PrGameContext& ctx) {
    if (!ctx.renderer) return;
    if (m_sceneId == 1) return;
    
    const float winW = (float)ctx.renderer->GetWidth();
    const float winH = (float)ctx.renderer->GetHeight();
    
    // Progress bar
    const float progressBarW = winW * 0.8f;
    const float progressBarH = 12.0f;
    const float progressBarX = (winW - progressBarW) / 2;
    const float progressBarY = 20.0f;
    
    // Progress bar background
    ctx.renderer->DrawRect(progressBarX, progressBarY, progressBarW, progressBarH, 0.1f, 0.1f, 0.1f, 0.8f);
    
    // Progress fill
    const float progress = GetProgress();
    ctx.renderer->DrawRect(progressBarX, progressBarY, progressBarW * progress, progressBarH, 0.2f, 0.7f, 0.3f, 1.0f);
    
    // Progress border
    ctx.renderer->DrawRect(progressBarX - 2, progressBarY - 2, progressBarW + 4, 2, 0.4f, 0.4f, 0.4f, 1.0f);
    ctx.renderer->DrawRect(progressBarX - 2, progressBarY + progressBarH, progressBarW + 4, 2, 0.4f, 0.4f, 0.4f, 1.0f);

    if (ctx.lastSqevsEventFrame != 0) {
        const int diff = (int)ctx.frame - (int)ctx.lastSqevsEventFrame;
        const bool hot = (diff >= 0 && diff < 15);
        const float x = winW - 24.0f;
        const float y = progressBarY;
        ctx.renderer->DrawRect(x, y, 14.0f, 14.0f,
                                hot ? 0.9f : 0.2f,
                                hot ? 0.9f : 0.2f,
                                hot ? 0.2f : 0.2f,
                                1.0f);
    }
    
    // Stage label in the top-left corner.
    const float stageBoxW = 80.0f;
    const float stageBoxH = 30.0f;
    ctx.renderer->DrawRect(10, 50, stageBoxW, stageBoxH, 0.1f, 0.1f, 0.2f, 0.9f);
    
    {
        char buf[64] = {};
        std::snprintf(buf, sizeof(buf), "STAGE %d", m_sceneId);
        PrUiOverlay::DrawTextUi(16.0f, 56.0f, 1.2f, buf, 1.0f, 1.0f, 1.0f, 1.0f);
    }

    // Beat counter on the right side.
    const float beatBoxW = 60.0f;
    const float beatBoxH = 30.0f;
    const float beatBoxX = winW - beatBoxW - 10;
    ctx.renderer->DrawRect(beatBoxX, 50, beatBoxW, beatBoxH, 0.1f, 0.2f, 0.1f, 0.9f);
    
    {
        char buf[64] = {};
        std::snprintf(buf, sizeof(buf), "B%03d", m_beatCount);
        PrUiOverlay::DrawTextUi(beatBoxX + 6.0f, 56.0f, 1.2f, buf, 1.0f, 1.0f, 1.0f, 1.0f);
    }
    
    // Grade bar in the center.
    const float gradeY = winH * 0.4f;
    const float gradeW = 200.0f;
    const float gradeH = 20.0f;
    const float gradeX = (winW - gradeW) / 2;
    
    // Grade background.
    ctx.renderer->DrawRect(gradeX, gradeY, gradeW, gradeH, 0.15f, 0.15f, 0.15f, 0.7f);
    
    // Grade slots 0-6, with the active one highlighted.
    const float segW = gradeW / 7.0f;
    for (int i = 0; i < 7; i++) {
        float segR = 0.3f, segG = 0.3f, segB = 0.3f;
        if (i == m_grade) {
            // Highlight the active grade slot.
            if (i <= 1) { segR = 0.8f; segG = 0.2f; segB = 0.2f; }      // BAD/AWFUL
            else if (i <= 3) { segR = 0.8f; segG = 0.6f; segB = 0.2f; } // GOOD
            else { segR = 0.2f; segG = 0.8f; segB = 0.3f; }             // COOL
        }
        ctx.renderer->DrawRect(gradeX + i * segW + 1, gradeY + 2, segW - 2, gradeH - 4, segR, segG, segB, 0.9f);
    }

    // Meter bar below the grade bar.
    const float meterY = gradeY + gradeH + 8.0f;
    const float meterW = gradeW;
    const float meterH = 10.0f;
    const float meterX = gradeX;
    
    // Meter background.
    ctx.renderer->DrawRect(meterX, meterY, meterW, meterH, 0.1f, 0.1f, 0.1f, 0.8f);
    
    // Meter fill, tinted by current value.
    const float meterRatio = (float)m_meter / 100.0f;
    float meterR = 0.3f, meterG = 0.7f, meterB = 0.3f;
    if (m_meter < 30) { meterR = 0.8f; meterG = 0.2f; meterB = 0.2f; }
    else if (m_meter < 60) { meterR = 0.8f; meterG = 0.6f; meterB = 0.2f; }
    ctx.renderer->DrawRect(meterX, meterY, meterW * meterRatio, meterH, meterR, meterG, meterB, 0.9f);

    {
        char buf[96] = {};
        std::snprintf(buf, sizeof(buf), "SCORE %d  METER %d  G%d", m_score, m_meter, m_grade);
        PrUiOverlay::DrawTextUi(meterX, meterY + 14.0f, 1.1f, buf, 1.0f, 1.0f, 1.0f, 0.95f);
    }

    // Flashing judgement feedback.
    if (m_judgeFlash > 0 && m_lastJudge != JudgeResult::None) {
        const float judgeBoxW = 80.0f;
        const float judgeBoxH = 24.0f;
        const float judgeX = (winW - judgeBoxW) / 2;
        const float judgeY = gradeY - judgeBoxH - 10.0f;
        
        float jr = 0.5f, jg = 0.5f, jb = 0.5f;
        switch (m_lastJudge) {
            case JudgeResult::Perfect: jr = 0.2f; jg = 0.9f; jb = 0.4f; break;
            case JudgeResult::Good:    jr = 0.9f; jg = 0.8f; jb = 0.2f; break;
            case JudgeResult::Bad:     jr = 0.9f; jg = 0.5f; jb = 0.2f; break;
            case JudgeResult::Miss:    jr = 0.9f; jg = 0.2f; jb = 0.2f; break;
            default: break;
        }
        
        // Flash fade-out.
        const float alpha = (m_judgeFlash > 6) ? 1.0f : (float)m_judgeFlash / 6.0f;
        ctx.renderer->DrawRect(judgeX, judgeY, judgeBoxW, judgeBoxH, jr, jg, jb, alpha * 0.9f);

        const char* jt = "";
        switch (m_lastJudge) {
            case JudgeResult::Perfect: jt = "PERFECT"; break;
            case JudgeResult::Good:    jt = "GOOD"; break;
            case JudgeResult::Bad:     jt = "BAD"; break;
            case JudgeResult::Miss:    jt = "MISS"; break;
            default: break;
        }
        
        if (jt[0] != 0) {
            PrUiOverlay::DrawTextUi(judgeX + 6.0f, judgeY + 6.0f, 1.0f, jt, 0.0f, 0.0f, 0.0f, alpha);
        }
    }

    // Combo counter on the right side.
    if (m_combo > 1) {
        const float comboBoxW = 50.0f;
        const float comboBoxH = 24.0f;
        const float comboX = winW - comboBoxW - 20.0f;
        const float comboY = gradeY;
        
        // Combo background changes with combo length.
        float cr = 0.2f, cg = 0.5f, cb = 0.8f;
        if (m_combo >= 10) { cr = 0.9f; cg = 0.7f; cb = 0.2f; }
        else if (m_combo >= 5) { cr = 0.5f; cg = 0.8f; cb = 0.3f; }
        ctx.renderer->DrawRect(comboX, comboY, comboBoxW, comboBoxH, cr, cg, cb, 0.85f);

        char buf[32] = {};
        std::snprintf(buf, sizeof(buf), "x%d", m_combo);
        PrUiOverlay::DrawTextUi(comboX + 8.0f, comboY + 6.0f, 1.0f, buf, 0.0f, 0.0f, 0.0f, 0.95f);
    }

    const char* bottomText = nullptr;
    if (!(PrSS0Scene0RuntimeDirect::RuntimeEnabled() &&
          ctx.currentScene == PrSceneId::Scene1)) {
        const SubtitleInfo* subtitle = PrSqevs1::GetActiveSubtitle();
        if (subtitle != nullptr && subtitle->text != nullptr) {
            bottomText = subtitle->text;
        }
    }
    const bool drawNativeSubtitleText =
        !PrStage1HdSubtitles::ShouldSuppressNativeSubtitleText(ctx);
    RenderStageRunnerTextBox(ctx, bottomText, drawNativeSubtitleText);
}

void PrStageRunner::UpdateStage1SpecialFixedBonusObserver(PrGameContext& ctx) {
    if (m_sceneId != 1 || m_state != StageRunnerState::Playing) {
        m_stage1NormalizedSelectControlBit = false;
        return;
    }

    m_stage1NormalizedSelectControlBit = GetStage1ObserverSelectControlBit(ctx);
    if (m_stage1SpecialFixedBonusBootLatch && m_stage1NormalizedSelectControlBit) {
        m_stage1SpecialFixedBonusLatch = true;
    }
}

void PrStageRunner::ProcessInput(PrGameContext& ctx) {
    // Current pad state
    const uint16_t curPad = ctx.debugPadInput;
    const uint16_t pressed = curPad & ~m_lastPadMask;  // Rising edge
    m_lastPadMask = curPad;

    // Only handle gameplay buttons on the rising edge.
    const uint16_t buttons = pressed & KPAD_BUTTONS;
    
    BeatChart& chart = GetBeatChart();
    const int kToleranceTicks = 24;  // +/-24 tick judgement window.

    // Start grace period: avoid immediate auto-miss damage.
    const bool allowAutoMiss = (m_frameCount >= 60);

    // Incremental miss processing; avoid rescanning the full chart each frame.
    if (allowAutoMiss) {
        const int noteCount = chart.GetNoteCount();
        while (m_nextChartIndex < noteCount) {
            const BeatNote* note = chart.GetNote(m_nextChartIndex);
            if (!note) break;

            if (note->tick96 >= m_tick96 - kToleranceTicks) {
                break; // The note is not expired yet.
            }

            if (!chart.IsNoteHit(m_nextChartIndex)) {
                chart.MarkNoteHit(m_nextChartIndex);
                ApplyJudge(JudgeResult::Miss);
            }

            m_nextChartIndex++;
        }
    }

    // Judge only when at least one button was pressed.
    if (buttons != 0) {
        const int noteCount = chart.GetNoteCount();
        int nextIdx = m_nextChartIndex;
        while (nextIdx < noteCount && chart.IsNoteHit(nextIdx)) {
            nextIdx++;
        }
        const BeatNote* nextNote = (nextIdx < noteCount) ? chart.GetNote(nextIdx) : nullptr;
        if (nextNote != nullptr && m_tick96 < nextNote->tick96 - kToleranceTicks) {
            return;
        }

        // Search locally around m_nextChartIndex for the nearest unhit note.
        int searchBegin = m_nextChartIndex - 4;
        if (searchBegin < 0) searchBegin = 0;
        int searchEnd = m_nextChartIndex + 12;
        if (searchEnd > noteCount) searchEnd = noteCount;

        int noteIdx = -1;
        int bestAbsDist = kToleranceTicks + 1;
        for (int i = searchBegin; i < searchEnd; i++) {
            if (chart.IsNoteHit(i)) continue;
            const BeatNote* n = chart.GetNote(i);
            if (!n) continue;
            const int dist = m_tick96 - n->tick96;
            if (dist < -kToleranceTicks || dist > kToleranceTicks) continue;
            const int absDist = (dist < 0) ? -dist : dist;
            if (absDist < bestAbsDist) {
                bestAbsDist = absDist;
                noteIdx = i;
            }
        }
        
        if (noteIdx >= 0) {
            const BeatNote* note = chart.GetNote(noteIdx);
            int dist = m_tick96 - note->tick96;
            int absDist = (dist < 0) ? -dist : dist;
            
            // Check whether the pressed button matches the chart note.
            bool buttonMatch = false;
            uint8_t noteBtn = (uint8_t)note->button;
            if (noteBtn == (uint8_t)BeatButton::Any) {
                buttonMatch = true;
            } else {
                // Convert the PSX pad mask into BeatButton bits.
                uint8_t pressedBtn = 0;
                if (buttons & KPAD_CROSS) pressedBtn |= (uint8_t)BeatButton::Cross;
                if (buttons & KPAD_CIRCLE) pressedBtn |= (uint8_t)BeatButton::Circle;
                if (buttons & KPAD_SQUARE) pressedBtn |= (uint8_t)BeatButton::Square;
                if (buttons & KPAD_TRIANGLE) pressedBtn |= (uint8_t)BeatButton::Triangle;
                buttonMatch = (pressedBtn & noteBtn) != 0;
            }
            
            JudgeResult judge;
            if (!buttonMatch) {
                judge = JudgeResult::Bad;  // Wrong button.
            } else if (absDist <= 6) {
                judge = JudgeResult::Perfect;
            } else if (absDist <= 12) {
                judge = JudgeResult::Good;
            } else {
                judge = JudgeResult::Bad;
            }
            
            chart.MarkNoteHit(noteIdx);
            ApplyJudge(judge);

            // Advance to the next unresolved chart note.
            const int count = chart.GetNoteCount();
            while (m_nextChartIndex < count && chart.IsNoteHit(m_nextChartIndex)) {
                m_nextChartIndex++;
            }
            
            Log::Printf("StageRunner: Note[%d] judge=%d dist=%d tick=%d meter=%d combo=%d",
                        noteIdx, (int)judge, dist, m_tick96, m_meter, m_combo);
        } else {
            // No note nearby: count it as an empty hit.
            Log::Printf("StageRunner: Empty hit at tick=%d", m_tick96);
            ApplyJudge(JudgeResult::Bad);
        }
    }
}

void PrStageRunner::ApplyJudge(JudgeResult judge) {
    m_lastJudge = judge;
    m_judgeFlash = 12;  // Show the flash for 12 frames.

    switch (judge) {
        case JudgeResult::Perfect:
            m_score += 120;
            m_meter += 3;
            m_combo++;
            break;
        case JudgeResult::Good:
            m_score += 60;
            m_meter += 1;
            m_combo++;
            break;
        case JudgeResult::Bad:
            m_meter -= 2;
            m_combo = 0;
            break;
        case JudgeResult::Miss:
            m_meter -= 6;
            m_combo = 0;
            break;
        default:
            break;
    }

    // Clamp meter to 0-100
    if (m_meter < 0) m_meter = 0;
    if (m_meter > 100) m_meter = 100;
}

int PrStageRunner::MeterToGrade(int meter) const {
    // Map meter 0-100 onto grade 0-6.
    if (meter < 10) return 0;
    if (meter < 20) return 1;
    if (meter < 40) return 2;
    if (meter < 60) return 3;
    if (meter < 80) return 4;
    if (meter < 95) return 5;
    return 6;
}
