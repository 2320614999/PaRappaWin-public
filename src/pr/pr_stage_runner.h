#pragma once

#include <cmath>
#include <cstdint>

struct PrGameContext;

inline constexpr int kPrStage1Bpm100 = 11000;
inline constexpr int kPrStage1TicksPerBeat = 96;
inline constexpr int kPrStage1BeatsPerBar = 4;
inline constexpr int kPrStage1TicksPerBar =
    kPrStage1TicksPerBeat * kPrStage1BeatsPerBar;
inline constexpr int kPrStage1TicksPerRenderFrame60 =
    ((kPrStage1TicksPerBeat * kPrStage1Bpm100) + (100 * 3600 / 2)) /
    (100 * 3600);

// Judge result
enum class JudgeResult : uint8_t {
    None = 0,
    Perfect,
    Good,
    Bad,
    Miss
};

// Stage runner state
enum class StageRunnerState : uint8_t {
    Idle = 0,
    Playing,
    Cleared,
    Failed,
    Aborted
};

// Minimal stage runner shell
class PrStageRunner {
public:
    PrStageRunner();
    ~PrStageRunner();
    
    // Initialize stage (sceneId: 1-9)
    void Init(int sceneId, PrGameContext& ctx);
    
    // Per-frame update. 0=continue, 1=finished.
    int Update(PrGameContext& ctx);

    // Re-lock audio baseline when XA starts after Init().
    void SyncXaStartBaseline(PrGameContext& ctx);
    
    // Render current frame
    void Render(PrGameContext& ctx);

    // Reset state
    void Reset();
    
    // State queries
    StageRunnerState GetState() const { return m_state; }
    int GetSceneId() const { return m_sceneId; }
    int GetFrame() const { return m_frameCount; }
    int GetFrame60() const { return (m_elapsedSeconds > 0.0) ? (int)std::floor(m_elapsedSeconds * 60.0 + 1e-9) : 0; }
    double GetElapsedSeconds() const { return m_elapsedSeconds; }
    double GetStartAudioSeconds() const { return m_startAudioSeconds; }
    int GetTick96() const { return m_tick96; }
    float GetProgress() const;
    
    bool GetStage1NormalizedSelectControlBit() const { return m_stage1NormalizedSelectControlBit; }
    bool GetStage1SpecialFixedBonusLatch() const { return m_stage1SpecialFixedBonusLatch; }
private:
    StageRunnerState m_state;
    int m_sceneId;
    int m_frameCount;
    int m_beatCount;        // 节拍计数（未来用于节奏判定）
    int m_grade;            // 当前评级 (0-6)

    int m_bpm100;
    int m_tickOffset;
    int m_tickPerFrame60;
    int64_t m_tickAccumNum;
    int m_tick96;
    int m_bar;
    int m_beatInBar;
    int m_tickInBeat;
    
    // Approx stage duration (frames @30fps)
    int m_targetFrames;

    // Audio-driven timeline
    double m_startAudioSeconds = 0.0;
    int m_startAudioSector75 = 0;
    int m_lastAcceptedXaSector75 = 0;
    double m_targetSeconds = 0.0;
    double m_elapsedSeconds = 0.0;

    // Legacy generic judge/score system. Frozen for Stage1; use only as the
    // non-Stage1 fallback lane.
    int m_score = 0;
    int m_meter = 50;
    int m_combo = 0;
    JudgeResult m_lastJudge = JudgeResult::None;
    int m_judgeFlash = 0;
    uint16_t m_lastPadMask = 0;
    int m_nextChartIndex = 0;
    int m_meterZeroFrames = 0;
    
    // 内部辅助
    void UpdateStage1HybridTimecode(PrGameContext& ctx);
    void RefreshStage1BeatFromTick96();
    void UpdateBeat(double relSeconds);
    void RenderBackground(PrGameContext& ctx);
    void RenderHUD(PrGameContext& ctx);
    void ProcessInput(PrGameContext& ctx);
    void UpdateStage1SpecialFixedBonusObserver(PrGameContext& ctx);
    void ApplyJudge(JudgeResult judge);
    int MeterToGrade(int meter) const;

    bool m_stage1SpecialFixedBonusBootLatch = false;
    bool m_stage1NormalizedSelectControlBit = false;
    bool m_stage1SpecialFixedBonusLatch = false;
};

// Global singleton
PrStageRunner& GetStageRunner();
