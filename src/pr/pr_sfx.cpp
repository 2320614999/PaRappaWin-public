#include "pr_sfx.h"
#include "../vab_player.h"
#include "../audio_engine.h"
#include "../int_loader.h"
#include "../logger.h"

#include <array>
#include <vector>
#include <cmath>
#include <cstring>
#include <algorithm>
#include <string>

namespace PrSfx {

static float s_volume = 0.6f;
static VabPlayer s_vab;
static bool s_vabLoaded = false;

static VabPlayer s_stage1Vab;
static bool s_stage1VabLoaded = false;
static std::string s_stage1VabPath;

static VabPlayer s_practiceVab;
static bool s_practiceVabLoaded = false;

struct CompactSfxCmd {
    uint8_t program;
    uint8_t note;
    uint8_t key;
    uint8_t volume;
};

static constexpr std::array<CompactSfxCmd, 2> kStage1SteadyVerdictCueTable = {{
    { 0u, 3u, 0x1Bu, 0x5Au },
    { 0u, 2u, 0x1Au, 0x5Au },
}};

static constexpr std::array<CompactSfxCmd, 2> kStage1SteadyRowCommitCueTable = {{
    { 0u, 1u, 0x19u, 0x5Au },
    { 0u, 0u, 0x18u, 0x5Au },
}};

static constexpr CompactSfxCmd kStage1DelayedFollowUpArmCue943F0 = {
    1u, 1u, 0x19u, 0x5Au
};

static constexpr CompactSfxCmd kStage1FailTailCue943EC = {
    1u, 0u, 0x18u, 0x5Au
};

static constexpr std::array<CompactSfxCmd, 2> kStage1DelayedFollowUpPrepareCueTable = {{
    { 1u, 2u, 0x1Au, 0x78u },
    { 1u, 3u, 0x1Bu, 0x78u },
}};

static constexpr CompactSfxCmd kSceneTransitionCue94410 = {
    0u, 7u, 0x1Fu, 0x5Au
};

static constexpr CompactSfxCmd kMovieTransitionCue8006EC18 = {
    0u, 12u, 0x24u, 0x50u
};

static constexpr std::array<CompactSfxCmd, 2> kMovie1ShellCue9441C = {{
    { 0u, 10u, 0x22u, 0x5Au },
    { 0u, 11u, 0x23u, 0x5Au },
}};

static constexpr CompactSfxCmd kStage1UiCue80025C8C_94420 = {
    0u, 12u, 0x24u, 0x7Fu
};

static constexpr CompactSfxCmd kStage1UiCue80025C8C_94424 = {
    0u, 13u, 0x25u, 0x7Fu
};

static constexpr CompactSfxCmd kStage1UiCue80025C8C_94428 = {
    0u, 14u, 0x26u, 0x7Fu
};

static constexpr double kStage1SourceCellVoiceMaxStartOffsetSeconds = 0.050;

static constexpr CompactSfxCmd kStage1UiCue80025C8C_9442C = {
    0u, 15u, 0x27u, 0x7Fu
};

// ---- Fallback: procedural tone generation (used when VAB not available) ----

static std::vector<int16_t> GenTone(float freqHz, float durationSec, float amplitude,
                                      float freqEnd = 0.0f, float fadeOut = 0.0f) {
    const int rate = (int)AudioEngine::Get().GetSampleRate();
    if (rate <= 0) return {};
    int samples = (int)(durationSec * rate);
    if (samples < 1) samples = 1;
    std::vector<int16_t> pcm(samples);
    bool sweep = (freqEnd > 0.0f && freqEnd != freqHz);
    for (int i = 0; i < samples; i++) {
        float t = (float)i / (float)rate;
        float progress = (float)i / (float)(samples - 1);
        float freq = sweep ? (freqHz + (freqEnd - freqHz) * progress) : freqHz;
        float env = amplitude;
        if (fadeOut > 0.0f) {
            float fadeStart = 1.0f - fadeOut;
            if (progress > fadeStart) {
                env *= 1.0f - (progress - fadeStart) / fadeOut;
            }
        }
        if (t < 0.002f) env *= t / 0.002f;
        float val = std::sin(2.0f * 3.14159265f * freq * t) * env;
        pcm[i] = (int16_t)std::clamp((int)(val * 32767.0f), -32768, 32767);
    }
    return pcm;
}

// Fallback PCM buffers
static std::vector<int16_t> s_navPcm;
static std::vector<int16_t> s_confirmPcm;
static std::vector<int16_t> s_cancelPcm;
static std::vector<int16_t> s_scn0GridInPcm;
static std::vector<int16_t> s_practiceBeatPcm;
static std::vector<int16_t> s_practiceHiTickPcm;
static std::vector<int16_t> s_stage1FailTailCue943ECPcm;
static std::vector<int16_t> s_stage1DelayedFollowUpArmCuePcm;
static std::array<std::vector<int16_t>, 2> s_stage1DelayedFollowUpPrepareCuePcm;
static std::vector<int16_t> s_sceneTransitionCue94410Pcm;
static std::vector<int16_t> s_movieTransitionCue8006EC18Pcm;
static std::array<std::vector<int16_t>, 2> s_movie1ShellCue9441CPcm;
static std::array<std::vector<int16_t>, 4> s_stage1UiCue80025C8CPcm;
static std::array<std::vector<int16_t>, 2> s_stage1SteadyVerdictCuePcm;
static std::array<std::vector<int16_t>, 2> s_stage1SteadyRowCommitCuePcm;
static int s_stage1SourceCellVoiceLaneVoice = -1;
static int s_bgmVoice = -1;
static constexpr double kStage1SourceCellVoiceRestartWindowSec = 0.50;

static int PlayBufEx(const std::vector<int16_t>& pcm) {
    if (pcm.empty()) return -1;
    auto& engine = AudioEngine::Get();
    if (!engine.IsRunning()) return -1;
    int voice = engine.AllocVoice(1, engine.GetSampleRate(), 1.0f, true);
    if (voice >= 0) {
        engine.QueueSamples(voice, pcm.data(), pcm.size());
    }
    return voice;
}

static void PlayBuf(const std::vector<int16_t>& pcm) {
    (void)PlayBufEx(pcm);
}

static float MidiKeyToHz(uint8_t key) {
    return 440.0f * std::pow(2.0f, ((float)key - 69.0f) / 12.0f);
}

static std::vector<int16_t> GenCompactCuePcm(const CompactSfxCmd& cmd, float sustainSec) {
    const float amp = s_volume * ((float)cmd.volume / 127.0f) * 0.65f;
    return GenTone(MidiKeyToHz(cmd.key), sustainSec, amp, 0.0f, 0.45f);
}

static bool TryPlayCompactCueFromVab(VabPlayer& vab, const CompactSfxCmd& cmd) {
    return vab.PlaySfxCmdEx(cmd.program, cmd.note, cmd.key, cmd.volume) >= 0;
}

static void PlayCompactCue(const CompactSfxCmd& cmd, const std::vector<int16_t>& fallbackPcm) {
    if (s_vabLoaded && TryPlayCompactCueFromVab(s_vab, cmd)) {
        return;
    }
    PlayBuf(fallbackPcm);
}

static void PlayStage1CompactCue(const CompactSfxCmd& cmd, const std::vector<int16_t>& fallbackPcm) {
    if (s_stage1VabLoaded && TryPlayCompactCueFromVab(s_stage1Vab, cmd)) {
        return;
    }
    PlayBuf(fallbackPcm);
}

static void ResetStage1SourceCellVoiceLaneInternal() {
    if (s_stage1SourceCellVoiceLaneVoice < 0) {
        return;
    }

    auto& engine = AudioEngine::Get();
    if (engine.IsVoiceActive(s_stage1SourceCellVoiceLaneVoice) &&
        engine.GetVoicePlayedSeconds(s_stage1SourceCellVoiceLaneVoice) <=
            kStage1SourceCellVoiceRestartWindowSec) {
        engine.FreeVoice(s_stage1SourceCellVoiceLaneVoice);
    }
    s_stage1SourceCellVoiceLaneVoice = -1;
}

// ---- PSX SE command block parameters (from IDA reverse-engineering) ----
// Format: {program, note, volume}
// These map to specific tones in MINIMUM.VH -> VAG samples in MINIMUM.VB
//
// Scene transition SE (0x8006EA84..94):
//   note=7  -> tone[7]  -> vag[7]  (15744 bytes) - transition/grid
//   note=8  -> tone[8]  -> vag[10] (3072 bytes)  - wave border
//   note=9  -> tone[9]  -> vag[11] (3920 bytes)  - subtitle
//
// UI button SE (0x8006EA9C..B4):
//   note=12 -> tone[12] -> vag[14] (912 bytes)   - navigate
//   note=13 -> tone[13] -> vag[15] (288 bytes)   - confirm
//   note=14 -> tone[14] -> vag[16] (704 bytes)   - cancel

void Init() {
    float vol = s_volume;
    // Generate fallback tones (used if VAB not loaded)
    s_navPcm = GenTone(1200.0f, 0.030f, vol * 0.5f, 0.0f, 0.5f);
    s_confirmPcm = GenTone(800.0f, 0.080f, vol * 0.6f, 1400.0f, 0.3f);
    s_cancelPcm = GenTone(600.0f, 0.100f, vol * 0.5f, 300.0f, 0.4f);
    s_scn0GridInPcm = GenTone(160.0f, 0.070f, vol * 0.70f, 120.0f, 0.35f);
    s_practiceBeatPcm = GenTone(660.0f, 0.030f, vol * 0.55f, 0.0f, 0.5f);
    s_practiceHiTickPcm = GenTone(1400.0f, 0.018f, vol * 0.45f, 0.0f, 0.6f);
    s_stage1FailTailCue943ECPcm =
        GenCompactCuePcm(kStage1FailTailCue943EC, 0.080f);
    s_stage1DelayedFollowUpArmCuePcm =
        GenCompactCuePcm(kStage1DelayedFollowUpArmCue943F0, 0.055f);
    for (size_t i = 0; i < kStage1DelayedFollowUpPrepareCueTable.size(); i++) {
        s_stage1DelayedFollowUpPrepareCuePcm[i] =
            GenCompactCuePcm(kStage1DelayedFollowUpPrepareCueTable[i], 0.070f);
    }
    s_sceneTransitionCue94410Pcm =
        GenCompactCuePcm(kSceneTransitionCue94410, 0.080f);
    s_movieTransitionCue8006EC18Pcm =
        GenCompactCuePcm(kMovieTransitionCue8006EC18, 0.070f);
    for (size_t i = 0; i < kMovie1ShellCue9441C.size(); i++) {
        s_movie1ShellCue9441CPcm[i] = GenCompactCuePcm(kMovie1ShellCue9441C[i], 0.070f);
    }
    s_stage1UiCue80025C8CPcm[0] =
        GenCompactCuePcm(kStage1UiCue80025C8C_94420, 0.070f);
    s_stage1UiCue80025C8CPcm[1] =
        GenCompactCuePcm(kStage1UiCue80025C8C_94424, 0.070f);
    s_stage1UiCue80025C8CPcm[2] =
        GenCompactCuePcm(kStage1UiCue80025C8C_94428, 0.070f);
    s_stage1UiCue80025C8CPcm[3] =
        GenCompactCuePcm(kStage1UiCue80025C8C_9442C, 0.070f);
    for (size_t i = 0; i < kStage1SteadyVerdictCueTable.size(); i++) {
        s_stage1SteadyVerdictCuePcm[i] = GenCompactCuePcm(kStage1SteadyVerdictCueTable[i], 0.050f);
        s_stage1SteadyRowCommitCuePcm[i] = GenCompactCuePcm(kStage1SteadyRowCommitCueTable[i], 0.045f);
    }
    Log::Printf("PrSfx: initialized fallback tones (vol=%.2f)", vol);
}

bool InitVab(const std::string& vhPath, const std::string& vbPath) {
    s_vabLoaded = s_vab.Load(vhPath, vbPath);
    if (s_vabLoaded) {
        Log::Printf("PrSfx: VAB loaded successfully (%d tones, %d vags)",
                    s_vab.GetToneCount(), s_vab.GetVagCount());
    } else {
        Log::Printf("PrSfx: VAB load failed, using fallback procedural tones");
    }
    return s_vabLoaded;
}

bool InitVabFromMemory(const uint8_t* vhData, size_t vhSize, const uint8_t* vbData, size_t vbSize) {
    s_vabLoaded = s_vab.LoadFromMemory(vhData, vhSize, vbData, vbSize);
    if (s_vabLoaded) {
        Log::Printf("PrSfx: VAB loaded successfully (%d tones, %d vags)",
                    s_vab.GetToneCount(), s_vab.GetVagCount());
    } else {
        Log::Printf("PrSfx: VAB load failed, using fallback procedural tones");
    }
    return s_vabLoaded;
}

bool InitStage1VabFromInt(const std::string& stage1CompoIntPath) {
    if (s_stage1VabLoaded && s_stage1VabPath == stage1CompoIntPath) {
        return true;
    }
    if (s_stage1VabLoaded) {
        Log::Printf("PrSfx: Stage1 VAB reload requested: %s -> %s",
                    s_stage1VabPath.c_str(), stage1CompoIntPath.c_str());
    }
    s_stage1VabLoaded = false;
    s_stage1VabPath.clear();

    IntArchive archive;
    if (!IntLoader::Load(stage1CompoIntPath, archive)) {
        Log::Printf("PrSfx: Stage1 VAB load failed: cannot load %s",
                    stage1CompoIntPath.c_str());
        return false;
    }

    const IntFileEntry* vh = archive.Find("STAGE1M.VH");
    const IntFileEntry* vb = archive.Find("STAGE1M.VB");
    if (!vh || !vb) {
        for (const auto& e : archive.entries) {
            if (e.type != IntBlockType::Vab) continue;
            if (!vh && e.name.find("STAGE1M") != std::string::npos &&
                e.name.find(".VH") != std::string::npos) {
                vh = &e;
            }
            if (!vb && e.name.find("STAGE1M") != std::string::npos &&
                e.name.find(".VB") != std::string::npos) {
                vb = &e;
            }
        }
    }

    if (!vh || !vb || vh->type != IntBlockType::Vab ||
        vb->type != IntBlockType::Vab) {
        Log::Printf("PrSfx: Stage1 VAB load failed: STAGE1M.VH/VB missing in %s",
                    stage1CompoIntPath.c_str());
        return false;
    }

    s_stage1VabLoaded =
        s_stage1Vab.LoadFromMemory(vh->data.data(), vh->data.size(),
                                   vb->data.data(), vb->data.size());
    if (s_stage1VabLoaded) {
        s_stage1VabPath = stage1CompoIntPath;
        Log::Printf("PrSfx: Stage1 VAB loaded successfully (%d tones, %d vags)",
                    s_stage1Vab.GetToneCount(), s_stage1Vab.GetVagCount());
    } else {
        Log::Printf("PrSfx: Stage1 VAB load failed from %s",
                    stage1CompoIntPath.c_str());
    }
    return s_stage1VabLoaded;
}

bool InitPracticeVabFromInt(const std::string& ycompoIntPath) {
    if (s_practiceVabLoaded) return true;

    IntArchive archive;
    if (!IntLoader::Load(ycompoIntPath, archive)) {
        return false;
    }

    const IntFileEntry* vh = archive.Find("PRACTICE.VH");
    const IntFileEntry* vb = archive.Find("PRACTICE.VB");

    if (!vh || !vb) {
        for (const auto& e : archive.entries) {
            if (e.type != IntBlockType::Vab) continue;
            if (!vh && e.name.find("PRACTICE") != std::string::npos && e.name.find(".VH") != std::string::npos) {
                vh = &e;
            }
            if (!vb && e.name.find("PRACTICE") != std::string::npos && e.name.find(".VB") != std::string::npos) {
                vb = &e;
            }
        }
    }

    if (!vh || !vb) {
        return false;
    }

    s_practiceVabLoaded = s_practiceVab.LoadFromMemory(vh->data.data(), vh->data.size(), vb->data.data(), vb->data.size());
    if (s_practiceVabLoaded) {
        Log::Printf("PrSfx: Practice VAB loaded successfully (%d tones, %d vags)",
                    s_practiceVab.GetToneCount(), s_practiceVab.GetVagCount());
    } else {
        Log::Printf("PrSfx: Practice VAB load failed");
    }
    return s_practiceVabLoaded;
}

// ---- Play functions: VAB if available, else fallback ----

void PlayNavigate() {
    if (s_vabLoaded) {
        s_vab.PlaySfxCmd(0, 12, 0x24, 0x7F); // original: {00 0C 24 7F}
    } else {
        PlayBuf(s_navPcm);
    }
}

void PlayConfirm() {
    if (s_vabLoaded) {
        s_vab.PlaySfxCmd(0, 14, 0x26, 0x7F); // original: {00 0E 26 7F}
    } else {
        PlayBuf(s_confirmPcm);
    }
}

void PlayCancel() {
    if (s_vabLoaded) {
        s_vab.PlaySfxCmd(0, 13, 0x25, 0x7F); // original: {00 0D 25 7F}
    } else {
        PlayBuf(s_cancelPcm);
    }
}

void PlayPracticeBeatCue() {
    if (s_practiceVabLoaded) {
        const int voice = s_practiceVab.PlaySfxCmdEx(1, 1, 0x19, 0x5A);
        if (voice >= 0) return;
    }

    if (s_vabLoaded) {
        const int voice = s_vab.PlaySfxCmdEx(1, 1, 0x19, 0x5A);
        if (voice >= 0) return;
    }

    PlayBuf(s_practiceBeatPcm);
}

void PlayPracticeBeat() {
    PlayPracticeBeatCue();
}

void PlayPracticeLoopStart() {
    if (s_practiceVabLoaded) {
        const int voice = s_practiceVab.PlaySfxCmdEx(1, 0, 0x18, 0x5A);
        if (voice >= 0) return;
    }

    if (s_vabLoaded) {
        const int voice = s_vab.PlaySfxCmdEx(1, 0, 0x18, 0x5A);
        if (voice >= 0) return;
    }

    PlayBuf(s_practiceHiTickPcm);
}

void PlayPracticeHiTick() {
    PlayPracticeLoopStart();
}

void PlayPracticeRoundPromptCue(int round) {
    int r = round;
    if (r < 0) r = 0;
    r &= 3;

    const uint8_t note = (uint8_t)r;
    const uint8_t key = (uint8_t)(note + 24);
    if (s_practiceVabLoaded) {
        const int voice = s_practiceVab.PlaySfxCmdEx(2, note, key, 0x5A);
        if (voice >= 0) return;
    }
    if (s_vabLoaded) {
        const int voice = s_vab.PlaySfxCmdEx(2, note, key, 0x5A);
        if (voice >= 0) return;
    }

    const float base = 720.0f;
    const float freq = base + (float)r * 60.0f;
    const auto pcm = GenTone(freq, 0.045f, s_volume * 0.50f, 0.0f, 0.6f);
    PlayBuf(pcm);
}

void PlayPracticePrompt(int round) {
    PlayPracticeRoundPromptCue(round);
}

static void PlayPracticeVoiceKindRaw(int kind) {
    int note = -1;
    uint8_t vol = 0x6E;
    switch (kind) {
        case 0: note = 1; break;
        case 1: note = 7; break;
        case 2: note = 8; break;
        case 3: note = 3; break;
        case 4: note = 6; break;
        case 5: note = 2; break;
        case 6: note = 5; break;
        case 7: note = 4; break;
        case 8: note = 0; break;
        default: return;
    }
    const uint8_t uNote = (uint8_t)note;
    const uint8_t key = (uint8_t)(uNote + 24);
    if (s_practiceVabLoaded) {
        const int voice = s_practiceVab.PlaySfxCmdEx(3, uNote, key, vol);
        if (voice >= 0) return;
    }
    if (s_vabLoaded) {
        const int voice = s_vab.PlaySfxCmdEx(3, uNote, key, vol);
        if (voice >= 0) return;
    }

    const float freq = (kind == 0) ? 520.0f : (kind == 1) ? 380.0f : 260.0f;
    const auto pcm = GenTone(freq, 0.12f, s_volume * 0.55f, 0.0f, 0.7f);
    PlayBuf(pcm);
}

void PlayPracticeResultVoiceCue(int judgeKind) {
    int kind = judgeKind;
    if (kind < 0) kind = 0;
    if (kind > 2) kind = 2;
    PlayPracticeVoiceKindRaw(kind);
}

void PlayPracticeRoundIntroVoiceCue(int round) {
    int r = round;
    if (r < 0) r = 0;
    r &= 3;
    static const int kIntroKinds[4] = {3, 4, 5, 6};
    PlayPracticeVoiceKindRaw(kIntroKinds[r]);
}

void PlayPracticeCompletePromptVoiceCue() {
    PlayPracticeVoiceKindRaw(7);
}

void PlayPracticeExitPromptVoiceCue() {
    PlayPracticeVoiceKindRaw(8);
}

void PlayPracticeVoiceKind(int kind) {
    PlayPracticeVoiceKindRaw(kind);
}

void PlayStage1SteadyVerdictCue(uint8_t tableSlot) {
    if (tableSlot >= kStage1SteadyVerdictCueTable.size()) {
        return;
    }
    PlayStage1CompactCue(kStage1SteadyVerdictCueTable[tableSlot], s_stage1SteadyVerdictCuePcm[tableSlot]);
}

void PlayStage1SteadyRowCommitCue(uint8_t tableSlot) {
    if (tableSlot >= kStage1SteadyRowCommitCueTable.size()) {
        return;
    }
    PlayStage1CompactCue(kStage1SteadyRowCommitCueTable[tableSlot], s_stage1SteadyRowCommitCuePcm[tableSlot]);
}

void PlayStage1Bucket30DirectCue94400(uint8_t tableSlot) {
    if (tableSlot >= kStage1SteadyRowCommitCueTable.size()) {
        return;
    }
    // PSX LABEL_103 dispatches `dword_80094400 + 6*v21`. Stage1's current
    // 94400 table bytes match the compact row-commit table, but keep the API
    // explicit so direct-port does not route this family through a row-write
    // semantic name.
    PlayStage1CompactCue(
        kStage1SteadyRowCommitCueTable[tableSlot],
        s_stage1SteadyRowCommitCuePcm[tableSlot]);
}

void PlayStage1DelayedFollowUpArmCue() {
    PlayStage1CompactCue(
        kStage1DelayedFollowUpArmCue943F0,
        s_stage1DelayedFollowUpArmCuePcm);
}

void PlayStage1DelayedFollowUpPrepareCue(bool secondOrLater) {
    const size_t tableSlot = secondOrLater ? 1u : 0u;
    PlayStage1CompactCue(
        kStage1DelayedFollowUpPrepareCueTable[tableSlot],
        s_stage1DelayedFollowUpPrepareCuePcm[tableSlot]);
}

void PlayStage1SteadyDelayedCompletionCue() {
    PlayStage1SteadyVerdictCue(0u);
}

void PlayStage1FailTailCue943EC() {
    PlayStage1CompactCue(kStage1FailTailCue943EC, s_stage1FailTailCue943ECPcm);
}

void PlayStage1SourceCellVoiceCue(uint8_t program,
                                  uint8_t note,
                                  uint8_t key,
                                  uint8_t volume,
                                  double startOffsetSeconds) {
    ResetStage1SourceCellVoiceLaneInternal();
    if (!std::isfinite(startOffsetSeconds) || startOffsetSeconds < 0.0) {
        startOffsetSeconds = 0.0;
    }
    startOffsetSeconds =
        std::min(startOffsetSeconds,
                 kStage1SourceCellVoiceMaxStartOffsetSeconds);

    if (s_stage1VabLoaded) {
        s_stage1SourceCellVoiceLaneVoice =
            s_stage1Vab.PlaySfxCmdExWithStartOffset(
                program,
                note,
                key,
                volume,
                startOffsetSeconds);
        if (s_stage1SourceCellVoiceLaneVoice >= 0) {
            return;
        }
    }

    const CompactSfxCmd cue{ program, note, key, volume };
    const std::vector<int16_t> fallbackPcm = GenCompactCuePcm(cue, 0.080f);
    s_stage1SourceCellVoiceLaneVoice = PlayBufEx(fallbackPcm);
}

void ResetStage1SourceCellVoiceCueLane() {
    ResetStage1SourceCellVoiceLaneInternal();
}

void ApplySharedAudioResetBarrier26FA4() {
    auto& engine = AudioEngine::Get();
    if (engine.IsRunning()) {
        engine.ResetAllVoices();
    }
    s_stage1SourceCellVoiceLaneVoice = -1;
    s_bgmVoice = -1;
    Log::Printf("PrSfx: ApplySharedAudioResetBarrier26FA4");
}

void ApplySharedAudioDriverFlushBarrier26ECC() {
    auto& engine = AudioEngine::Get();
    if (!engine.IsRunning()) {
        (void)engine.Initialize(44100, 2);
    }
    Log::Printf("PrSfx: ApplySharedAudioDriverFlushBarrier26ECC running=%d",
                engine.IsRunning() ? 1 : 0);
}

void PlaySceneTransitionCue94410() {
    PlayCompactCue(kSceneTransitionCue94410, s_sceneTransitionCue94410Pcm);
}

void PlayMovieTransitionCue8006EC18() {
    PlayCompactCue(kMovieTransitionCue8006EC18, s_movieTransitionCue8006EC18Pcm);
    ApplySharedAudioDriverFlushBarrier26ECC();
}

void PlayStage1UiCue80025C8CRaw(uint16_t code) {
    switch (code) {
    case 0x20u:
        PlayStage1CompactCue(kStage1UiCue80025C8C_94428,
                             s_stage1UiCue80025C8CPcm[2]);
        return;
    case 0x40u:
        PlayStage1CompactCue(kStage1UiCue80025C8C_94424,
                             s_stage1UiCue80025C8CPcm[1]);
        return;
    case 0x100u:
        PlayStage1CompactCue(kStage1UiCue80025C8C_9442C,
                             s_stage1UiCue80025C8CPcm[3]);
        return;
    case 0x1000u:
    case 0x2000u:
    case 0x4000u:
    case 0x8000u:
        PlayStage1CompactCue(kStage1UiCue80025C8C_94420,
                             s_stage1UiCue80025C8CPcm[0]);
        return;
    default:
        return;
    }
}

void PlayMovie1ShellCue9441CRaw(uint8_t cueIndex) {
    if (cueIndex >= kMovie1ShellCue9441C.size()) {
        return;
    }
    PlayStage1CompactCue(kMovie1ShellCue9441C[cueIndex],
                         s_movie1ShellCue9441CPcm[cueIndex]);
}

void PlayMovie1ShellCue9441C(uint8_t cueIndex) {
    PlayMovie1ShellCue9441CRaw(cueIndex);
    ApplySharedAudioDriverFlushBarrier26ECC();
}

// Scene0 transition SE: grid panels rotating in/out
// dword_80094410 → {00 07 1F 5A}: prog=0, note=7, vol=0x5A
void PlayScn0GridIn() {
    if (s_vabLoaded) {
        s_vab.PlaySfxCmd(0, 12, 0x24, 0x5A); // original: {00 0C 24 5A}
    } else {
        PlayBuf(s_scn0GridInPcm);
    }
}

void PlayScn0GridOut() {
    if (s_vabLoaded) {
        s_vab.PlaySfxCmd(0, 12, 0x24, 0x5A);
    } else {
        PlayBuf(s_scn0GridInPcm);
    }
}

// Scene0 wave border show/hide SE
// dword_80094414 → {00 08 20 5A}: prog=0, note=8, vol=0x5A
void PlayScn0WaveIn() {
    if (s_vabLoaded) {
        s_vab.PlaySfxCmd(0, 11, 0x23, 0x5A); // original: {00 0B 23 5A}
    } else {
        PlayBuf(s_scn0GridInPcm); // fallback
    }
}

void PlayScn0WaveOut() {
    if (s_vabLoaded) {
        s_vab.PlaySfxCmd(0, 11, 0x23, 0x5A);
    } else {
        PlayBuf(s_scn0GridInPcm);
    }
}

// Scene0 subtitle border show/hide SE
// dword_80094418 → {00 09 21 5A}: prog=0, note=9, vol=0x5A
void PlayScn0SubIn() {
    if (s_vabLoaded) {
        s_vab.PlaySfxCmd(0, 10, 0x22, 0x5A); // original: {00 0A 22 5A}
    } else {
        PlayBuf(s_scn0GridInPcm);
    }
}

void PlayScn0SubOut() {
    if (s_vabLoaded) {
        s_vab.PlaySfxCmd(0, 10, 0x22, 0x5A);
    } else {
        PlayBuf(s_scn0GridInPcm);
    }
}

void SetVolume(float vol) {
    s_volume = std::clamp(vol, 0.0f, 1.0f);
    Init(); // regenerate fallback buffers
}

float GetVolume() {
    return s_volume;
}

// ---- BGM loop (PSX tone_007 – title/menu shared BGM) ----
// PSX: sub_80026EF8(dword_80094410) → SPU infinite loop via byte_800928DD=64
// WIN: allocate a looping voice, queue decoded VAG samples

void PlayBgm() {
    if (s_bgmVoice >= 0) {
        auto& engine = AudioEngine::Get();
        if (engine.IsVoiceActive(s_bgmVoice)) {
            return; // already playing
        }
        // voice went stale, clear
        s_bgmVoice = -1;
    }

    if (!s_vabLoaded) {
        Log::Printf("PrSfx: PlayBgm – VAB not loaded, skipping");
        return;
    }

    // tone_007: program=0, note=7 → toneIdx = 0*16+7 = 7
    // PSX SE cmd: {00 07 1F 5A} → prog=0, note=7, key=0x1F, vol=0x5A
    const int toneIdx = 7;
    if (toneIdx >= s_vab.GetToneCount()) {
        Log::Printf("PrSfx: PlayBgm – tone_007 not found in VAB");
        return;
    }

    // Use PlaySfxCmd path to calculate pitch, but we need looping.
    // Instead, manually get VAG data and create a looping voice.
    // Access tone via the public interface – we'll use PlayTone-like logic but with looping.

    auto& engine = AudioEngine::Get();
    if (!engine.IsRunning()) {
        Log::Printf("PrSfx: PlayBgm – AudioEngine not running");
        return;
    }

    // We need the decoded PCM from tone 7.
    // VabPlayer doesn't expose raw PCM directly, but we can use PlaySfxCmd
    // as a template. For looping we need AllocVoice with looping=true.
    // Since VabPlayer::PlaySfxCmd uses oneShot=true, we'll replicate the logic here.

    size_t loopStart = 0;
    size_t loopEnd = 0;
    size_t dummySamples = 0;
    uint32_t dummyRate = 0;
    (void)s_vab.GetTonePcmInfo(0, 7, 0x1F, dummySamples, dummyRate, &loopStart, &loopEnd);

    // Use PlaySfxCmdEx to get the exact voice ID — avoids searching and
    // accidentally finding STR player's voice instead of BGM voice.
    s_bgmVoice = s_vab.PlaySfxCmdEx(0, 7, 0x1F, 0x5A);

    if (s_bgmVoice >= 0) {
        engine.SetVoiceLooping(s_bgmVoice, true);
        engine.SetVoiceLoopRegion(s_bgmVoice, loopStart, loopEnd);
        Log::Printf("PrSfx: PlayBgm – started BGM loop on voice %d", s_bgmVoice);
    } else {
        Log::Printf("PrSfx: PlayBgm – failed to allocate voice");
    }
}

void StopBgm() {
    if (s_bgmVoice >= 0) {
        auto& engine = AudioEngine::Get();
        engine.FreeVoice(s_bgmVoice);
        Log::Printf("PrSfx: StopBgm – freed voice %d", s_bgmVoice);
        s_bgmVoice = -1;
    }
}

bool IsBgmPlaying() {
    if (s_bgmVoice < 0) return false;
    return AudioEngine::Get().IsVoiceActive(s_bgmVoice);
}

bool GetBgmPcmInfo(size_t& outSamples, uint32_t& outPlayRate) {
    outSamples = 0;
    outPlayRate = 0;
    if (!s_vabLoaded) return false;
    return s_vab.GetTonePcmInfo(0, 7, 0x1F, outSamples, outPlayRate);
}

bool DumpS0Wav(const std::string& outDir) {
    if (!s_vabLoaded) return false;

    bool ok = true;
    ok = s_vab.DumpAllWav(outDir) && ok;

    std::vector<VabPlayer::SfxCmd> cmds;
    cmds.push_back({0, 12, 0x24, 0x7F});
    cmds.push_back({0, 13, 0x25, 0x7F});
    cmds.push_back({0, 14, 0x26, 0x7F});
    cmds.push_back({0, 7,  0x1F, 0x5A});
    cmds.push_back({0, 8,  0x20, 0x5A});
    cmds.push_back({0, 9,  0x21, 0x5A});
    cmds.push_back({0, 10, 0x22, 0x5A});
    cmds.push_back({0, 11, 0x23, 0x5A});
    ok = s_vab.DumpCmdWav(outDir, cmds) && ok;
    return ok;
}

} // namespace PrSfx
