#include "pr_sfx.h"
#include "pr_ss0_scene0_int_spu_direct.h"
#include "pr_stage1_loader_spu_hal.h"
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
#include <utility>

namespace PrSfx {

static float s_volume = 0.6f;
static VabPlayer s_vab;
static bool s_vabLoaded = false;

struct Scene0HostVoiceLease80032B00 {
    int voice = -1;
    uint64_t generation = 0u;
};

struct Scene0SsMidiVoiceOwner8003349C {
    uint16_t packedSequenceTrack = 0u;
    int16_t sourceVabId = -1;
    uint8_t program = 0u;
    uint8_t note = 0u;
    std::vector<Scene0HostVoiceLease80032B00> voices{};
};

static std::vector<Scene0SsMidiVoiceOwner8003349C>
    s_scene0SsMidiVoiceOwners8003349C;
static PrSS0Scene0IntSpuDirect::SsDriverFlushState80026ECC
    s_sharedAudioDriverFlushState80026ECC;
static PrSS0Scene0IntSpuDirect::SsDriverVoiceCompletionState80032B00
    s_sharedAudioDriverVoiceCompletionState80032B00;
static std::array<
    PrSS0Scene0IntSpuDirect::SsDriverVoiceRegisterState80032B00,
    AudioEngine::kMaxVoices>
    s_sharedAudioDriverVoiceRegisters80032B00{};
static PrSS0Scene0IntSpuDirect::SsDriverVolumeMix80032B00
    s_sharedAudioDriverVolumeMix80032B00{};
static PrSS0Scene0IntSpuDirect::SsDriverVolumeScratch80032B00
    s_sharedAudioDriverVolumeScratch80032B00{};
static PrSS0Scene0IntSpuDirect::SsDriverGlobalMaskState80032B00
    s_sharedAudioDriverGlobalMasks80032B00{};
static std::array<
    PrSS0Scene0IntSpuDirect::SsDriverVoiceOwnerState8003226C,
    AudioEngine::kMaxVoices>
    s_sharedAudioDriverVoiceOwners8003226C{};
static PrSS0Scene0IntSpuDirect::SsDriverInitializeState8003226C
    s_sharedAudioDriverInitializeState8003226C{};
static PrSS0Scene0IntSpuDirect::SsSpuFirstAllocationState8002E87C
    s_sharedSsSpuFirstAllocationState8002E87C{};
static PrSS0Scene0IntSpuDirect::SsVabBodyTransferState8002EB80
    s_sharedSsVabBodyTransferState8002EB80{};
static bool s_scene0VabBodyTransferTranslated8002EB80 = false;
static int16_t s_scene0CompactSfxVoice800943AC = -1;
static PrStage1LoaderSpuHal::VabSlots8002E474 s_intVabSlots;
static std::vector<uint8_t> s_intVabHeader;
static VabPlayer s_intVabCandidate;
static int16_t s_intVabSlot = -1;
static bool s_intVabTransferStarted = false;
static bool s_intVabPayloadCommitted = false;

static int32_t CommitSharedAudioDriverHostProjection80032B00(void*);

static VabPlayer s_stage1Vab;
static bool s_stage1VabLoaded = false;
static std::string s_stage1VabPath;

static VabPlayer s_practiceVab;
static bool s_practiceVabLoaded = false;
static int32_t s_movieTransitionCueCadenceCounter8006EC20 = 0;

static uint64_t HashVabBytes8001A8F0(const uint8_t* data, size_t size) {
    uint64_t hash = 14695981039346656037ull;
    for (size_t index = 0u; index < size; ++index) {
        hash ^= data[index];
        hash *= 1099511628211ull;
    }
    return hash;
}

struct CompactSfxCmd {
    uint8_t program;
    uint8_t note;
    uint8_t key;
    uint8_t volume;
};

// Current original PSX Scene0 binding used by the SS0 direct translation:
//   dword_80094400 -> 801C6E7C -> 00 01 19 5A 00 00.
// 80025DBC copies that six-byte record, forces byte 3 to 0x5A, then calls
// 80026EF8/80026ECC. Keep this Scene0 owner independent from the coincident
// Stage1 row-commit command with the same compact values.
static constexpr CompactSfxCmd kScene0OptionsLanguageCue80025DBC = {
    0u, 1u, 0x19u, 0x5Au
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

// Current COMOD0 binding 8009441C -> 801C6EC4.  SCUS 800271E4 indexes this
// pair in six-byte steps and then owns the 80026EF8 -> 80026ECC call order.
static constexpr std::array<CompactSfxCmd, 2>
kScene0MovieTransitionCue800271E4 = {{
    { 0u, 10u, 0x22u, 0x5Au },
    { 0u, 11u, 0x23u, 0x5Au },
}};

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

struct UiCueSelection80025C8C {
    const CompactSfxCmd* command = nullptr;
    size_t fallbackIndex = 0u;
};

static UiCueSelection80025C8C ResolveUiCueSelection80025C8C(
    uint16_t code) {
    switch (code) {
    case 0x20u:
        return {&kStage1UiCue80025C8C_94428, 2u};
    case 0x40u:
        return {&kStage1UiCue80025C8C_94424, 1u};
    case 0x100u:
        return {&kStage1UiCue80025C8C_9442C, 3u};
    case 0x1000u:
    case 0x2000u:
    case 0x4000u:
    case 0x8000u:
        return {&kStage1UiCue80025C8C_94420, 0u};
    default:
        return {};
    }
}

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
static uint64_t s_bgmVoiceGeneration = 0u;
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

static bool TryPlayScene0CompactCueDirect80026EF8(
    const CompactSfxCmd& sourceCommand, bool* nativeResultKnown = nullptr) {
    using namespace PrSS0Scene0IntSpuDirect;
    VabPlayer::MidiProgramAttributes80032EAC sourceProgram{};
    VabPlayer::MidiToneAttributes80032EAC sourceTone{};
    const bool attributesResolved =
        s_vab.ResolveCompactSfxAttributes80034240(
            sourceCommand.program,
            sourceCommand.note,
            sourceProgram,
            sourceTone);

    SsCompactSfxRequest80034240 request{};
    request.bankReady =
        s_vabLoaded &&
        s_scene0VabBodyTransferTranslated8002EB80 &&
        attributesResolved;
    request.sourceVabId = 0;
    request.command = {
        sourceCommand.program,
        sourceCommand.note,
        sourceCommand.key,
        sourceCommand.volume,
    };
    request.vabMasterVolume = s_vab.GetMasterVolume80032EAC();
    request.programAttributes.valid = sourceProgram.valid;
    request.programAttributes.toneCount = sourceProgram.toneCount;
    request.programAttributes.volume = sourceProgram.volume;
    request.programAttributes.priority = sourceProgram.priority;
    request.programAttributes.mode = sourceProgram.mode;
    request.programAttributes.pan = sourceProgram.pan;
    request.programAttributes.toneTableProgram =
        sourceProgram.toneTableProgram;
    request.tone.valid = sourceTone.valid;
    request.tone.toneSlot = sourceTone.toneSlot;
    request.tone.toneIndex = sourceTone.toneIndex;
    request.tone.priority = sourceTone.priority;
    request.tone.mode = sourceTone.mode;
    request.tone.volume = sourceTone.volume;
    request.tone.pan = sourceTone.pan;
    request.tone.centerNote = sourceTone.centerNote;
    request.tone.centerFine = sourceTone.centerFine;
    request.tone.noteMin = sourceTone.noteMin;
    request.tone.noteMax = sourceTone.noteMax;
    request.tone.adsr1 = sourceTone.adsr1;
    request.tone.adsr2 = sourceTone.adsr2;
    request.tone.sampleId = sourceTone.sampleId;
    request.tone.sampleStartAddressKnown =
        sourceTone.sampleStartAddressKnown;
    request.tone.sampleStartAddress = sourceTone.sampleStartAddress;

    SsCompactSfxDispatch80034240 dispatch{};
    const bool translated = TryExecuteSsCompactSfxCommand80026EF8(
        BuildSsCompactSfxSurface80034240(),
        request,
        s_sharedAudioDriverFlushState80026ECC,
        s_sharedAudioDriverInitializeState8003226C,
        s_sharedAudioDriverVoiceOwners8003226C,
        s_sharedAudioDriverVoiceCompletionState80032B00,
        s_sharedAudioDriverVoiceRegisters80032B00,
        s_sharedAudioDriverVolumeMix80032B00,
        s_sharedAudioDriverVolumeScratch80032B00,
        s_sharedAudioDriverGlobalMasks80032B00,
        dispatch);
    s_scene0CompactSfxVoice800943AC =
        translated ? static_cast<int16_t>(dispatch.result) : -1;
    if (nativeResultKnown) *nativeResultKnown = translated;
    if (!translated || !dispatch.hostPlaybackReady ||
        dispatch.selectedVoice >= AudioEngine::kMaxVoices) {
        Log::Printf(
            "PrSfx: Scene0 compact SFX direct translated=%d "
            "program=%u tone=%u key=%u voice=%d validationReject=%d "
            "allocationFail=%d hostReady=0 psxVoiceAuthority=0",
            translated ? 1 : 0,
            static_cast<unsigned>(dispatch.commandAfter[0]),
            static_cast<unsigned>(dispatch.commandAfter[1]),
            static_cast<unsigned>(dispatch.commandAfter[2]),
            static_cast<int>(s_scene0CompactSfxVoice800943AC),
            dispatch.validationRejected ? 1 : 0,
            dispatch.allocationFailed ? 1 : 0);
        return false;
    }

    const float hostVolume =
        static_cast<float>(std::max(
            dispatch.pendingVolumeLeft,
            dispatch.pendingVolumeRight)) /
        16383.0f;
    const int voice = s_vab.PlayMidiToneLayerAtVoice80032EAC(
        sourceTone,
        dispatch.pitch,
        hostVolume,
        dispatch.selectedVoice);
    auto& engine = AudioEngine::Get();
    const uint64_t generation =
        voice >= 0 ? engine.GetVoiceGeneration(voice) : 0u;
    if (voice < 0 || generation == 0u) {
        if (nativeResultKnown) *nativeResultKnown = false;
        return false;
    }
    s_scene0SsMidiVoiceOwners8003349C.push_back({
        33u,
        0,
        dispatch.commandAfter[0],
        dispatch.commandAfter[2],
        {{voice, generation}},
    });
    if (sourceCommand.program == kSceneTransitionCue94410.program &&
        sourceCommand.note == kSceneTransitionCue94410.note &&
        sourceCommand.key == kSceneTransitionCue94410.key &&
        sourceCommand.volume == kSceneTransitionCue94410.volume) {
        // This is the original title/menu/loading loop, already played by
        // the translated 80026EF8 path.  VabPlayer applied the VAG loop flags;
        // retain its lease so later menu entry cannot start a second copy.
        s_bgmVoice = voice;
        s_bgmVoiceGeneration = generation;
        Log::Printf("PrSfx: 94410 direct BGM loop lease voice=%d generation=%llu",
                    voice, static_cast<unsigned long long>(generation));
    }
    Log::Printf(
        "PrSfx: Scene0 compact SFX direct program=%u tone=%u key=%u "
        "voice=%d pitch=%u volumeL=%u volumeR=%u "
        "sampleStartKnown=%d psxVoiceAuthority=0",
        static_cast<unsigned>(dispatch.commandAfter[0]),
        static_cast<unsigned>(dispatch.commandAfter[1]),
        static_cast<unsigned>(dispatch.commandAfter[2]),
        voice,
        dispatch.pitch,
        dispatch.pendingVolumeLeft,
        dispatch.pendingVolumeRight,
        dispatch.sampleStartAddressKnown ? 1 : 0);
    return true;
}

static void PlayCompactCue(const CompactSfxCmd& cmd, const std::vector<int16_t>& fallbackPcm) {
    if (s_vabLoaded && TryPlayCompactCueFromVab(s_vab, cmd)) {
        return;
    }
    PlayBuf(fallbackPcm);
}

static uint32_t StopScene0SsMidiNoteOwners8003349C(
    uint16_t packedSequenceTrack,
    int16_t sourceVabId,
    uint8_t program,
    uint8_t note,
    bool matchAll) {
    auto& engine = AudioEngine::Get();
    uint32_t stopped = 0u;
    auto owner = s_scene0SsMidiVoiceOwners8003349C.begin();
    while (owner != s_scene0SsMidiVoiceOwners8003349C.end()) {
        const bool matches =
            matchAll ||
            (owner->packedSequenceTrack == packedSequenceTrack &&
             owner->sourceVabId == sourceVabId &&
             owner->program == program && owner->note == note);
        if (!matches) {
            ++owner;
            continue;
        }
        for (const Scene0HostVoiceLease80032B00& lease : owner->voices) {
            if (engine.FreeVoiceIfGeneration(
                    lease.voice, lease.generation)) {
                ++stopped;
            }
        }
        owner = s_scene0SsMidiVoiceOwners8003349C.erase(owner);
    }
    return stopped;
}

static uint32_t ReconcileScene0SsMidiVoiceOwners80032B00(
    uint32_t stableZeroEnvelopeMask) {
    auto& engine = AudioEngine::Get();
    uint32_t retired = 0u;
    auto owner = s_scene0SsMidiVoiceOwners8003349C.begin();
    while (owner != s_scene0SsMidiVoiceOwners8003349C.end()) {
        auto lease = owner->voices.begin();
        while (lease != owner->voices.end()) {
            const bool generationStillCurrent =
                engine.GetVoiceGeneration(lease->voice) ==
                lease->generation;
            const bool stableComplete =
                lease->voice >= 0 &&
                lease->voice < AudioEngine::kMaxVoices &&
                (stableZeroEnvelopeMask &
                 (1u << static_cast<uint32_t>(lease->voice))) != 0u;
            if (generationStillCurrent && !stableComplete) {
                ++lease;
                continue;
            }
            lease = owner->voices.erase(lease);
            ++retired;
        }
        if (owner->voices.empty()) {
            owner = s_scene0SsMidiVoiceOwners8003349C.erase(owner);
        } else {
            ++owner;
        }
    }
    return retired;
}

static uint32_t ApplyScene0HostVoiceRegisterProjection80032B00(
    const PrSS0Scene0IntSpuDirect::
        SsDriverRegisterCommitDispatch80032B00& dispatch) {
    auto& engine = AudioEngine::Get();
    const uint32_t keyOffMask =
        static_cast<uint32_t>(dispatch.keyOffLow) |
        (static_cast<uint32_t>(dispatch.keyOffHigh) << 16u);
    uint32_t projected = 0u;
    auto owner = s_scene0SsMidiVoiceOwners8003349C.begin();
    while (owner != s_scene0SsMidiVoiceOwners8003349C.end()) {
        auto lease = owner->voices.begin();
        while (lease != owner->voices.end()) {
            if (lease->voice < 0 ||
                lease->voice >= AudioEngine::kMaxVoices) {
                lease = owner->voices.erase(lease);
                continue;
            }
            const uint32_t voiceIndex =
                static_cast<uint32_t>(lease->voice);
            const uint32_t voiceBit = 1u << voiceIndex;
            if ((keyOffMask & voiceBit) != 0u) {
                (void)engine.FreeVoiceIfGeneration(
                    lease->voice, lease->generation);
                lease = owner->voices.erase(lease);
                ++projected;
                continue;
            }
            const auto& request =
                dispatch.voiceRequests[voiceIndex];
            if (request.writeVolume &&
                engine.IsVoiceLeaseActive(
                    lease->voice, lease->generation)) {
                const uint16_t hostMagnitude = std::max(
                    request.volumeLeft, request.volumeRight);
                const float hostVolume = std::clamp(
                    static_cast<float>(hostMagnitude) / 16383.0f,
                    0.0f,
                    1.0f);
                engine.SetVoiceVolume(lease->voice, hostVolume);
                ++projected;
            }
            ++lease;
        }
        if (owner->voices.empty()) {
            owner = s_scene0SsMidiVoiceOwners8003349C.erase(owner);
        } else {
            ++owner;
        }
    }
    return projected;
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

bool InitializeIntVabDriver80026E4C() {
    using namespace PrSS0Scene0IntSpuDirect;
    // SsInitHot -> _SsInit -> 8003226C initializes the 24 voices, VAB
    // status slots and the 32-entry SPU allocator before 80027078.
    ResetScene0SsMidiNoteProjection8003349C();
    s_vabLoaded = false;
    s_intVabHeader.clear();
    s_intVabCandidate = VabPlayer{};
    s_intVabSlot = -1;
    s_intVabTransferStarted = s_intVabPayloadCommitted = false;
    s_intVabSlots = {};
    SsDriverInitializeDispatch8003226C driver{};
    SsSpuFirstAllocationDispatch8002E87C allocator{};
    if (!TryInitializeSsDriverState8003226C(
            BuildSsDriverInitializeSurface8003226C(), 24u,
            s_sharedAudioDriverVoiceOwners8003226C,
            s_sharedAudioDriverVoiceCompletionState80032B00,
            s_sharedAudioDriverVoiceRegisters80032B00,
            s_sharedAudioDriverVolumeMix80032B00,
            s_sharedAudioDriverGlobalMasks80032B00,
            s_sharedAudioDriverInitializeState8003226C,
            CommitSharedAudioDriverHostProjection80032B00, nullptr, driver) ||
        !TryInitializeSsSpuAllocator80035394(
            BuildSsSpuFirstAllocationSurface8002E87C(),
            s_sharedSsSpuFirstAllocationState8002E87C, allocator)) return false;
    PrStage1LoaderSpuHal::InitializeVabSlots8003226C(s_intVabSlots);
    return true;
}

static bool AllocateIntVabSpu8002E87C(uint32_t bytes, int32_t& result, void*) {
    using namespace PrSS0Scene0IntSpuDirect;
    // The INT branch performs SsInitHot immediately before every open.
    // Do not treat an unsupported non-first allocator state as a native fail.
    if (!s_sharedSsSpuFirstAllocationState8002E87C.initialized ||
        s_sharedSsSpuFirstAllocationState8002E87C.allocationCount != 0u)
        return false;
    SsSpuFirstAllocationDispatch8002E87C allocation{};
    if (!TryAllocateFirstSsSpuBlock8002E87C(
            BuildSsSpuFirstAllocationSurface8002E87C(), bytes,
            s_sharedSsSpuFirstAllocationState8002E87C, allocation)) return false;
    result = allocation.allocationSucceeded ? int32_t(allocation.allocatedAddress) : -1;
    return allocation.known;
}

bool OpenIntVab80027078(const uint8_t* vh, size_t bytes, int16_t& result) {
    if (!vh || bytes == 0u) return false;
    s_intVabHeader.assign(vh, vh + bytes);
    if (!PrStage1LoaderSpuHal::OpenVab8002E474(s_intVabSlots,
            s_intVabHeader, -1, AllocateIntVabSpu8002E87C, nullptr, result))
        return false;
    s_intVabSlot = result;
    if (result >= 0) {
        const size_t slot = size_t(result);
        s_sharedSsVabBodyTransferState8002EB80.vabStatus800928F8[slot] =
            s_intVabSlots.status800928F8[slot];
        s_sharedSsVabBodyTransferState8002EB80.vabSpuBase801C35F8[slot] =
            s_intVabSlots.base801C35F8[slot];
        s_sharedSsVabBodyTransferState8002EB80.vabTransferBytes801C35B0[slot] =
            s_intVabSlots.bytes801C35B0[slot];
        s_sharedSsVabBodyTransferState8002EB80.transferReady800555F8 = false;
    }
    Log::Printf("PrSfx: INT native VAB open result=%d bytes=%zu", int(result), bytes);
    return true;
}

bool TransferIntVab800270D4(const uint8_t* vb, size_t bytes, int32_t& result) {
    using namespace PrSS0Scene0IntSpuDirect;
    if (!vb || bytes > UINT32_MAX || s_intVabSlot < 0) return false;
    if (!s_intVabTransferStarted) {
        SsVabBodyTransferDispatch8002EB80 transfer{};
        if (!TryBeginSsVabBodyTransfer8002EB80(
                BuildSsVabBodyTransferSurface8002EB80(), uint16_t(s_intVabSlot),
                uint32_t(bytes), s_sharedSsVabBodyTransferState8002EB80, transfer))
            return false;
        result = transfer.result;
        if (!transfer.accepted) return transfer.known;
        if (!transfer.hostPayloadProjectionReady) return false;
        s_intVabTransferStarted = true;
        s_intVabSlots.status800928F8[size_t(s_intVabSlot)] = transfer.statusAfter;
    }
    // PCM decoding is the Win HAL's payload transfer, not the slot allocator.
    // Keep a failed host projection pending instead of fabricating completion.
    if (!s_intVabPayloadCommitted) {
        if (!s_intVabCandidate.LoadFromMemory(s_intVabHeader.data(),
                s_intVabHeader.size(), vb, bytes) ||
            !s_intVabCandidate.ApplySpuAllocationBase8002E474(
                s_intVabSlots.base801C35F8[size_t(s_intVabSlot)])) return false;
        s_intVabPayloadCommitted = true;
    }
    result = s_intVabSlot;
    return true;
}

bool CompleteIntVab800270FC(int32_t& result) {
    using namespace PrSS0Scene0IntSpuDirect;
    SsVabTransferCompletionDispatch8002EF28 completion{};
    if (!TryCompleteSsVabBodyTransfer8002EEFC(
            BuildSsVabBodyTransferSurface8002EB80(), true,
            s_intVabPayloadCommitted,
            s_sharedSsVabBodyTransferState8002EB80, completion) ||
        completion.wouldBlock) return false;
    result = completion.result;
    if (completion.result == 1 && s_intVabPayloadCommitted) {
        s_vab = std::move(s_intVabCandidate);
        s_vabLoaded = s_vab.IsLoaded();
        s_scene0VabBodyTransferTranslated8002EB80 = s_vabLoaded;
        s_intVabSlots.transferReady800555F8 = true;
        Log::Printf("PrSfx: INT native VAB transfer completed slot=%d tones=%d vags=%d",
                    int(s_intVabSlot), s_vab.GetToneCount(), s_vab.GetVagCount());
    }
    return true;
}

bool PlayIntLoaderCue94410(int16_t& result) {
    bool known = false;
    (void)TryPlayScene0CompactCueDirect80026EF8(kSceneTransitionCue94410, &known);
    result = s_scene0CompactSfxVoice800943AC;
    return known;
}

Scene0IntVabCandidate8001A8F0
BuildScene0IntVabCandidate8001A8F0(
    const uint8_t* vhData,
    size_t vhSize,
    const uint8_t* vbData,
    size_t vbSize) {
    Scene0IntVabCandidate8001A8F0 candidate{};
    if (vhData == nullptr || vhSize == 0u || vbData == nullptr ||
        vbSize == 0u) {
        return candidate;
    }
    candidate.vhSize = vhSize;
    candidate.vbSize = vbSize;
    candidate.vhHash = HashVabBytes8001A8F0(vhData, vhSize);
    candidate.vbHash = HashVabBytes8001A8F0(vbData, vbSize);
    if (!candidate.player.LoadFromMemory(
            vhData, vhSize, vbData, vbSize)) {
        return candidate;
    }
    candidate.toneCount = candidate.player.GetToneCount();
    candidate.vagCount = candidate.player.GetVagCount();
    candidate.prepared = true;
    return candidate;
}

bool IsExactScene0IntVabCandidate8001A8F0(
    const Scene0IntVabCandidate8001A8F0& candidate,
    size_t vhSize,
    uint64_t vhHash,
    size_t vbSize,
    uint64_t vbHash,
    uint32_t toneCount,
    uint32_t vagCount) {
    return candidate.prepared && candidate.player.IsLoaded() &&
           candidate.vhSize == vhSize && candidate.vhHash == vhHash &&
           candidate.vbSize == vbSize && candidate.vbHash == vbHash &&
           candidate.toneCount >= 0 && candidate.vagCount >= 0 &&
           static_cast<uint32_t>(candidate.toneCount) == toneCount &&
           static_cast<uint32_t>(candidate.vagCount) == vagCount;
}

void CommitScene0IntVabCandidate8001A8F0(
    Scene0IntVabCandidate8001A8F0&& candidate) {
    ResetScene0SsMidiNoteProjection8003349C();
    s_scene0VabBodyTransferTranslated8002EB80 = false;
    PrSS0Scene0IntSpuDirect::SsDriverInitializeDispatch8003226C
        initializeDispatch{};
    const bool driverInitialized =
        PrSS0Scene0IntSpuDirect::TryInitializeSsDriverState8003226C(
            PrSS0Scene0IntSpuDirect::
                BuildSsDriverInitializeSurface8003226C(),
            24u,
            s_sharedAudioDriverVoiceOwners8003226C,
            s_sharedAudioDriverVoiceCompletionState80032B00,
            s_sharedAudioDriverVoiceRegisters80032B00,
            s_sharedAudioDriverVolumeMix80032B00,
            s_sharedAudioDriverGlobalMasks80032B00,
            s_sharedAudioDriverInitializeState8003226C,
            CommitSharedAudioDriverHostProjection80032B00,
            nullptr,
            initializeDispatch);
    const auto spuAllocationSurface =
        PrSS0Scene0IntSpuDirect::
            BuildSsSpuFirstAllocationSurface8002E87C();
    PrSS0Scene0IntSpuDirect::SsSpuFirstAllocationDispatch8002E87C
        spuInitializeDispatch{};
    PrSS0Scene0IntSpuDirect::SsSpuFirstAllocationDispatch8002E87C
        spuAllocationDispatch{};
    const bool spuAllocatorInitialized =
        driverInitialized &&
        PrSS0Scene0IntSpuDirect::TryInitializeSsSpuAllocator80035394(
            spuAllocationSurface,
            s_sharedSsSpuFirstAllocationState8002E87C,
            spuInitializeDispatch);
    const bool spuAllocationTranslated =
        spuAllocatorInitialized &&
        PrSS0Scene0IntSpuDirect::TryAllocateFirstSsSpuBlock8002E87C(
            spuAllocationSurface,
            candidate.player.GetSpuAllocationBytes8002E474(),
            s_sharedSsSpuFirstAllocationState8002E87C,
            spuAllocationDispatch);
    const bool sampleStartAddressesProjected =
        spuAllocationTranslated &&
        spuAllocationDispatch.allocationSucceeded &&
        candidate.player.ApplySpuAllocationBase8002E474(
            spuAllocationDispatch.allocatedAddress);
    const auto vabTransferSurface =
        PrSS0Scene0IntSpuDirect::
            BuildSsVabBodyTransferSurface8002EB80();
    PrSS0Scene0IntSpuDirect::SsVabBodyTransferDispatch8002EB80
        vabTransferDispatch{};
    PrSS0Scene0IntSpuDirect::SsVabTransferCompletionDispatch8002EF28
        vabTransferCompletion{};
    const bool vabTransferStatePrepared =
        sampleStartAddressesProjected &&
        PrSS0Scene0IntSpuDirect::
            TryPrepareFirstSsVabBodyTransferState8002E474(
                vabTransferSurface,
                spuAllocationDispatch.allocatedAddress,
                spuAllocationDispatch.alignedBytes,
                s_sharedSsVabBodyTransferState8002EB80);
    const bool sourceSizeRepresentable =
        candidate.vbSize <= UINT32_MAX;
    const bool vabTransferStarted =
        vabTransferStatePrepared && sourceSizeRepresentable &&
        PrSS0Scene0IntSpuDirect::TryBeginSsVabBodyTransfer8002EB80(
            vabTransferSurface,
            0u,
            static_cast<uint32_t>(candidate.vbSize),
            s_sharedSsVabBodyTransferState8002EB80,
            vabTransferDispatch) &&
        vabTransferDispatch.accepted &&
        vabTransferDispatch.hostPayloadProjectionReady;
    const bool vabTransferCompleted =
        vabTransferStarted &&
        PrSS0Scene0IntSpuDirect::TryCompleteSsVabBodyTransfer8002EEFC(
            vabTransferSurface,
            true,
            candidate.player.IsLoaded(),
            s_sharedSsVabBodyTransferState8002EB80,
            vabTransferCompletion) &&
        vabTransferCompletion.result == 1 &&
        vabTransferCompletion.completionLatched;
    s_scene0VabBodyTransferTranslated8002EB80 =
        vabTransferCompleted;
    s_vab = std::move(candidate.player);
    s_vabLoaded = s_vab.IsLoaded();
    candidate.prepared = false;
    Log::Printf(
        "PrSfx: Scene0 direct INT VAB bank committed tones=%d vags=%d "
        "hostBank=%d driverInit8003226C=%d voices=%u "
        "driverCommit80032B00=%d spuAllocator80035394=%d "
        "firstSpuAllocation8002E87C=%d base=%05X bytes=%u "
        "sampleStarts8002E474=%d vabBodyTransfer8002EB80=%d "
        "transferBytes=%u transferReady8002EF28=%d "
        "psxSpuAuthority=0 "
        "psxSpuTransferAuthority=0",
        s_vab.GetToneCount(),
        s_vab.GetVagCount(),
        s_vabLoaded ? 1 : 0,
        driverInitialized ? 1 : 0,
        initializeDispatch.configuredVoiceCount,
        initializeDispatch.driverCommitExecuted ? 1 : 0,
        spuAllocatorInitialized ? 1 : 0,
        spuAllocationDispatch.allocationSucceeded ? 1 : 0,
        spuAllocationDispatch.allocatedAddress,
        spuAllocationDispatch.alignedBytes,
        sampleStartAddressesProjected ? 1 : 0,
        s_scene0VabBodyTransferTranslated8002EB80 ? 1 : 0,
        vabTransferDispatch.committedTransferBytes,
        vabTransferCompletion.completionLatched ? 1 : 0);
}

bool ResolveScene0SsMidiNoteToneLayers80032EAC(
    uint8_t program,
    uint8_t note,
    std::vector<int>& outToneIndices) {
    outToneIndices.clear();
    if (!s_vabLoaded) {
        return false;
    }
    outToneIndices =
        s_vab.ResolveMidiNoteToneLayers80032EAC(program, note);
    return true;
}

Scene0SsMidiNoteProjectionResult8002BEEC
ProjectScene0SsMidiNote8002BEEC(
    uint16_t packedSequenceTrack,
    int16_t sourceVabId,
    uint8_t program,
    uint8_t note,
    uint16_t velocity,
    uint16_t channelVolume,
    uint8_t channelPan,
    uint16_t trackVolumeLeft,
    uint16_t trackVolumeRight,
    bool noteOn) {
    using namespace PrSS0Scene0IntSpuDirect;
    Scene0SsMidiNoteProjectionResult8002BEEC result{};
    result.known = true;
    result.hostVabReady = s_vabLoaded;
    result.noteOn = noteOn;
    result.noteOff = !noteOn;
    result.directVabBodyTransferTranslated =
        s_scene0VabBodyTransferTranslated8002EB80;
    result.sourceVabIdOwnershipKeyOnly = sourceVabId >= 0;
    result.psxSampleStartAddressAuthority = false;
    result.psxVabIdAuthority = false;
    result.psxVoiceIdentityAuthority = false;
    result.psxSpuTimingAuthority = false;
    if (!s_vabLoaded || sourceVabId < 0 || sourceVabId >= 16) {
        return result;
    }

    if (!noteOn) {
        SsDriverNoteOffDispatch8003349C driverDispatch{};
        result.directDriverStateTranslated =
            TryExecuteSsDriverNoteOff8003349C(
                BuildSsDriverVoiceAllocationSurface80032EAC(),
                packedSequenceTrack,
                sourceVabId,
                program,
                note,
                s_sharedAudioDriverInitializeState8003226C,
                s_sharedAudioDriverVoiceOwners8003226C,
                s_sharedAudioDriverVoiceCompletionState80032B00,
                s_sharedAudioDriverGlobalMasks80032B00,
                driverDispatch);
        result.stoppedVoiceCount =
            StopScene0SsMidiNoteOwners8003349C(
                packedSequenceTrack,
                sourceVabId,
                program,
                note,
                false);
        return result;
    }

    VabPlayer::MidiProgramAttributes80032EAC sourceProgram{};
    std::vector<VabPlayer::MidiToneAttributes80032EAC> toneLayers;
    const bool attributesResolved =
        s_vab.ResolveMidiNoteAttributes80032EAC(
            program, note, sourceProgram, toneLayers);
    result.matchedToneLayerCount =
        static_cast<uint32_t>(toneLayers.size());
    SsDriverNoteOnRequest80032EAC request{};
    request.bankReady = attributesResolved;
    request.packedSequenceTrack = packedSequenceTrack;
    request.sourceVabId = sourceVabId;
    request.program = program;
    request.note = note;
    request.velocity = velocity;
    request.channelVolume = channelVolume;
    request.channelPan = channelPan;
    request.trackVolumeLeft = trackVolumeLeft;
    request.trackVolumeRight = trackVolumeRight;
    request.vabMasterVolume = s_vab.GetMasterVolume80032EAC();
    request.programAttributes.valid = sourceProgram.valid;
    request.programAttributes.toneCount = sourceProgram.toneCount;
    request.programAttributes.volume = sourceProgram.volume;
    request.programAttributes.priority = sourceProgram.priority;
    request.programAttributes.mode = sourceProgram.mode;
    request.programAttributes.pan = sourceProgram.pan;
    request.programAttributes.toneTableProgram =
        sourceProgram.toneTableProgram;
    request.matchedToneCount = static_cast<uint8_t>(
        std::min<std::size_t>(toneLayers.size(), request.tones.size()));
    for (uint8_t index = 0u;
         index < request.matchedToneCount;
         ++index) {
        const auto& sourceTone = toneLayers[index];
        auto& tone = request.tones[index];
        tone.valid = sourceTone.valid;
        tone.toneSlot = sourceTone.toneSlot;
        tone.toneIndex = sourceTone.toneIndex;
        tone.priority = sourceTone.priority;
        tone.mode = sourceTone.mode;
        tone.volume = sourceTone.volume;
        tone.pan = sourceTone.pan;
        tone.centerNote = sourceTone.centerNote;
        tone.centerFine = sourceTone.centerFine;
        tone.noteMin = sourceTone.noteMin;
        tone.noteMax = sourceTone.noteMax;
        tone.adsr1 = sourceTone.adsr1;
        tone.adsr2 = sourceTone.adsr2;
        tone.sampleId = sourceTone.sampleId;
        tone.sampleStartAddressKnown =
            sourceTone.sampleStartAddressKnown;
        tone.sampleStartAddress = sourceTone.sampleStartAddress;
    }

    SsDriverNoteOnDispatch80032EAC driverDispatch{};
    result.directDriverStateTranslated =
        TryExecuteSsDriverNoteOn80032EAC(
            BuildSsDriverVoiceAllocationSurface80032EAC(),
            request,
            s_sharedAudioDriverInitializeState8003226C,
            s_sharedAudioDriverVoiceOwners8003226C,
            s_sharedAudioDriverVoiceCompletionState80032B00,
            s_sharedAudioDriverVoiceRegisters80032B00,
            s_sharedAudioDriverVolumeMix80032B00,
            s_sharedAudioDriverVolumeScratch80032B00,
            s_sharedAudioDriverGlobalMasks80032B00,
            driverDispatch);
    if (!result.directDriverStateTranslated) {
        return result;
    }
    result.directAllocatedVoiceCount =
        driverDispatch.allocatedToneCount;
    result.directAllocatedVoiceMask =
        driverDispatch.allocatedVoiceMask;
    result.directReplacedVoiceMask =
        driverDispatch.replacedVoiceMask;
    for (uint8_t index = 0u;
         index < request.matchedToneCount;
         ++index) {
        const auto& layer = driverDispatch.layers[index];
        if (layer.sampleStartAddressKnown &&
            layer.selectedVoice < AudioEngine::kMaxVoices) {
            result.directSampleStartKnownVoiceMask |=
                1u << layer.selectedVoice;
        }
    }
    result.fixedVoiceSlotHostProjection = true;

    std::vector<Scene0HostVoiceLease80032B00> leases;
    leases.reserve(driverDispatch.allocatedToneCount);
    auto& engine = AudioEngine::Get();
    for (uint8_t index = 0u;
         index < request.matchedToneCount;
         ++index) {
        const auto& layer = driverDispatch.layers[index];
        if (!layer.hostPlaybackReady ||
            layer.selectedVoice >= AudioEngine::kMaxVoices) {
            continue;
        }
        const float hostVolume =
            static_cast<float>(std::max(
                layer.pendingVolumeLeft,
                layer.pendingVolumeRight)) /
            16383.0f;
        const int voice = s_vab.PlayMidiToneLayerAtVoice80032EAC(
            toneLayers[index],
            layer.pitch,
            hostVolume,
            layer.selectedVoice);
        const uint64_t generation =
            engine.GetVoiceGeneration(voice);
        if (voice < 0 || generation == 0u) {
            continue;
        }
        leases.push_back({voice, generation});
    }
    result.startedVoiceCount =
        static_cast<uint32_t>(leases.size());
    if (!leases.empty()) {
        s_scene0SsMidiVoiceOwners8003349C.push_back({
            packedSequenceTrack,
            sourceVabId,
            program,
            note,
            std::move(leases),
        });
    }
    return result;
}

void ResetScene0SsMidiNoteProjection8003349C() {
    (void)StopScene0SsMidiNoteOwners8003349C(
        0u, -1, 0u, 0u, true);
    s_sharedAudioDriverVoiceCompletionState80032B00 =
        PrSS0Scene0IntSpuDirect::
            SsDriverVoiceCompletionState80032B00{};
    s_sharedAudioDriverVoiceRegisters80032B00 = {};
    s_sharedAudioDriverVolumeMix80032B00 = {};
    s_sharedAudioDriverVolumeScratch80032B00 = {};
    s_sharedAudioDriverGlobalMasks80032B00 = {};
    s_sharedAudioDriverVoiceOwners8003226C = {};
    s_sharedAudioDriverInitializeState8003226C = {};
    s_sharedSsSpuFirstAllocationState8002E87C = {};
    s_sharedSsVabBodyTransferState8002EB80 = {};
    s_scene0VabBodyTransferTranslated8002EB80 = false;
    s_scene0CompactSfxVoice800943AC = -1;
}

Scene0VabCloseResult80027120 ApplyScene0VabClose80027120() {
    Scene0VabCloseResult80027120 result{};
    result.known = true;
    result.priorHostVabActive = s_vabLoaded;

    // The translated 80027120 body reaches SsVabClose only when the current
    // VAB handle is valid.  Reset the SS0 software driver/voice projection in
    // either case so a subsequent 80027078 open cannot inherit stale owners;
    // this is deliberately a host projection, not physical SPU authority.
    if (s_vabLoaded) {
        ResetScene0SsMidiNoteProjection8003349C();
        s_vabLoaded = false;
    }
    result.hostProjectionCleared = true;
    result.committed = true;
    Log::Printf(
        "PrSfx: Scene0 80027120 close barrier committed priorHostVab=%d hostProjectionCleared=%d psxSpuAuthority=0 psxInterruptTimingAuthority=0",
        result.priorHostVabActive ? 1 : 0,
        result.hostProjectionCleared ? 1 : 0);
    return result;
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

SharedAudioResetBarrierResult26FA4 ApplySharedAudioResetBarrier26FA4() {
    using namespace PrSS0Scene0IntSpuDirect;
    SharedAudioResetBarrierResult26FA4 result{};
    SsDriverResetDispatch800351B8 dispatch{};
    const bool translated = TryExecuteSsDriverReset800351B8(
        BuildSsDriverResetSurface800351B8(),
        s_sharedAudioDriverVoiceOwners8003226C,
        s_sharedAudioDriverVoiceCompletionState80032B00,
        s_sharedAudioDriverVoiceRegisters80032B00,
        s_sharedAudioDriverGlobalMasks80032B00,
        s_sharedAudioDriverInitializeState8003226C,
        dispatch);
    result.known = dispatch.known;
    result.translated = translated;
    result.zeroVoiceCountNoOp = dispatch.zeroVoiceCountNoOp;
    result.hostVoiceResetProjectionReady =
        dispatch.hostVoiceResetProjectionReady;
    result.returnedVoiceCount = dispatch.returnedVoiceCount;
    result.resetVoiceMask = dispatch.resetVoiceMask;
    result.keyOffLowAfter = dispatch.keyOffLowAfter;
    result.keyOffHighAfter = dispatch.keyOffHighAfter;
    result.keyOnLowAfter = dispatch.keyOnLowAfter;
    result.keyOnHighAfter = dispatch.keyOnHighAfter;
    result.committed =
        translated && dispatch.known &&
        (dispatch.zeroVoiceCountNoOp ||
         dispatch.hostVoiceResetProjectionReady);

    auto& engine = AudioEngine::Get();
    uint32_t hostResetVoiceMask = 0u;
    if (translated && dispatch.hostVoiceResetProjectionReady) {
        for (uint32_t voice = 0u;
             voice < dispatch.returnedVoiceCount;
             ++voice) {
            engine.FreeVoice(static_cast<int>(voice));
            hostResetVoiceMask |= 1u << voice;
        }
        if (s_stage1SourceCellVoiceLaneVoice >= 0 &&
            s_stage1SourceCellVoiceLaneVoice <
                dispatch.returnedVoiceCount) {
            s_stage1SourceCellVoiceLaneVoice = -1;
        }
        if (s_bgmVoice >= 0 &&
            s_bgmVoice < dispatch.returnedVoiceCount) {
            s_bgmVoice = -1;
            s_bgmVoiceGeneration = 0u;
        }
        s_scene0SsMidiVoiceOwners8003349C.clear();
    }
    Log::Printf(
        "PrSfx: ApplySharedAudioResetBarrier26FA4 "
        "translated=%d voiceCount=%u resetMask=%06X "
        "keyOff=%04X:%04X keyOn=%04X:%04X "
        "hostResetMask=%06X driverCommit=0 "
        "psxRegisterAuthority=0 psxInterruptTimingAuthority=0",
        translated ? 1 : 0,
        static_cast<unsigned>(dispatch.returnedVoiceCount),
        dispatch.resetVoiceMask,
        dispatch.keyOffHighAfter,
        dispatch.keyOffLowAfter,
        dispatch.keyOnHighAfter,
        dispatch.keyOnLowAfter,
         hostResetVoiceMask);
    return result;
}

static int32_t CommitSharedAudioDriverHostProjection80032B00(void*) {
    using namespace PrSS0Scene0IntSpuDirect;
    auto& engine = AudioEngine::Get();
    if (!engine.IsRunning()) {
        (void)engine.Initialize(44100, 2);
    }
    SsDriverVoiceCompletionObservation80032B00 observation{};
    observation.voiceCount =
        s_sharedAudioDriverInitializeState8003226C
            .configuredVoiceCount800928A0;
    observation.completionScanDisabled =
        s_sharedAudioDriverInitializeState8003226C
            .completionScanDisabled8009290C;
    for (uint32_t voice = 0u; voice < observation.voiceCount; ++voice) {
        const bool hostPcmVoiceActive =
            engine.IsVoiceActive(static_cast<int>(voice));
        const bool translatedNoiseVoiceActive =
            s_sharedAudioDriverVoiceCompletionState80032B00
                .voiceStatus[voice] == 2u;
        observation.envelopeObservations[voice] =
            hostPcmVoiceActive || translatedNoiseVoiceActive
                ? 0x7FFFu
                : 0u;
    }
    const SsDriverVoiceCompletionSurface80032B00 completionSurface =
        BuildSsDriverVoiceCompletionSurface80032B00();
    const bool envelopeStateProjected =
        TryProjectSsDriverVoiceEnvelope80032B00(
            completionSurface,
            observation,
            s_sharedAudioDriverVoiceOwners8003226C);
    SsDriverVoiceCompletionDispatch80032B00 dispatch{};
    const bool completionTranslated =
        TryExecuteSsDriverVoiceCompletion80032B00(
            completionSurface,
            observation,
            s_sharedAudioDriverVoiceCompletionState80032B00,
            dispatch);
    const uint32_t retiredOwners =
        completionTranslated
            ? ReconcileScene0SsMidiVoiceOwners80032B00(
                  dispatch.stableZeroEnvelopeMask)
            : 0u;

    const SsDriverVolumeRampSurface80032B00 rampSurface =
        BuildSsDriverVolumeRampSurface80032B00();
    uint32_t rampExecutions = 0u;
    for (auto& voice : s_sharedAudioDriverVoiceRegisters80032B00) {
        SsDriverVolumeRampDispatch80032B00 rampDispatch{};
        if (voice.firstRamp.active != 0 &&
            TryExecuteSsDriverVolumeRamp80032B00(
                rampSurface,
                SsDriverVolumeRampKind80032B00::First80031A28,
                s_sharedAudioDriverVolumeMix80032B00,
                s_sharedAudioDriverVolumeScratch80032B00,
                voice,
                rampDispatch)) {
            ++rampExecutions;
        }
        if (voice.secondRamp.active != 0 &&
            TryExecuteSsDriverVolumeRamp80032B00(
                rampSurface,
                SsDriverVolumeRampKind80032B00::Second80031F28,
                s_sharedAudioDriverVolumeMix80032B00,
                s_sharedAudioDriverVolumeScratch80032B00,
                voice,
                rampDispatch)) {
            ++rampExecutions;
        }
    }

    SsDriverRegisterCommitDispatch80032B00 registerDispatch{};
    const bool registerCommitTranslated =
        TryExecuteSsDriverRegisterCommit80032B00(
            BuildSsDriverRegisterCommitSurface80032B00(),
            s_sharedAudioDriverVoiceRegisters80032B00,
            s_sharedAudioDriverGlobalMasks80032B00,
            registerDispatch);
    const uint32_t hostRegisterProjections =
        registerCommitTranslated
            ? ApplyScene0HostVoiceRegisterProjection80032B00(
                  registerDispatch)
            : 0u;
    Log::Printf(
        "PrSfx: CommitSharedAudioDriverHostProjection80032B00 "
        "envelopeState=%d completion=%d ring=%u stable=%06X statusClear=%06X "
        "noiseClearRequests=%u retiredOwners=%u ramps=%u "
        "registerCommit=%d dirty=%06X hostRegisterProjections=%u "
        "psxEnvelopeAuthority=0 psxNoiseRegisterAuthority=0 "
        "psxSpuRegisterAuthority=0",
        envelopeStateProjected ? 1 : 0,
        completionTranslated ? 1 : 0,
        dispatch.completionRingIndexAfter,
        dispatch.stableZeroEnvelopeMask,
        dispatch.clearedVoiceStatusMask,
        dispatch.noiseMaskClearCallCount,
        retiredOwners,
        rampExecutions,
        registerCommitTranslated ? 1 : 0,
        registerDispatch.dirtyVoiceMask,
        hostRegisterProjections);
    return engine.IsRunning() ? 1 : 0;
}

void ApplySharedAudioDriverFlushBarrier26ECC() {
    (void)ApplySharedAudioDriverFlushBarrier26ECCResult();
}

int32_t ApplySharedAudioDriverFlushBarrier26ECCResult() {
    using namespace PrSS0Scene0IntSpuDirect;
    SsDriverFlushDispatch80026ECC dispatch{};
    const bool translated = TryExecuteSsDriverFlush80026ECC(
        BuildSsDriverFlushSurface80026ECC(),
        s_sharedAudioDriverFlushState80026ECC,
        CommitSharedAudioDriverHostProjection80032B00,
        nullptr,
        dispatch);
    Log::Printf(
        "PrSfx: ApplySharedAudioDriverFlushBarrier26ECC translated=%d "
        "wrapperSkip=%d reentrySkip=%d commit=%d running=%d "
        "psxSpuAuthority=0",
        translated ? 1 : 0,
        dispatch.wrapperSkippedByBusyFlag ? 1 : 0,
        dispatch.lowerSkippedByReentryGuard ? 1 : 0,
        dispatch.driverCommitExecuted ? 1 : 0,
        AudioEngine::Get().IsRunning() ? 1 : 0);
    return translated ? dispatch.result : 0;
}

bool IsStage1VabReadyForHostProjection800943A8() {
    return s_stage1VabLoaded;
}

int16_t PlayStage1Cue80034240HostProjection(
    uint16_t hostDriverToken,
    uint8_t program,
    uint8_t note,
    uint8_t key,
    uint16_t arg4,
    uint8_t volumeLeft,
    uint8_t volumeRight) {
    // word_800943A8 is a dynamic PSX VAB handle. The Windows projection owns
    // one already-validated Stage1 VAB bank and therefore accepts only the
    // explicit host token 0. This does not claim the PSX handle value or SPU
    // voice identity.
    if (hostDriverToken != 0u || arg4 != 0u ||
        volumeLeft != volumeRight || !s_stage1VabLoaded) {
        return -1;
    }
    const int voice = s_stage1Vab.PlaySfxCmdEx(
        program, note, key, volumeLeft);
    if (voice < 0 || voice > 0x7FFF) {
        return -1;
    }
    return static_cast<int16_t>(voice);
}

void PlaySceneTransitionCue94410() {
    PlayCompactCue(kSceneTransitionCue94410, s_sceneTransitionCue94410Pcm);
}

void PlayMovieTransitionCue8006EC18() {
    PlayCompactCue(kMovieTransitionCue8006EC18, s_movieTransitionCue8006EC18Pcm);
    ApplySharedAudioDriverFlushBarrier26ECC();
}

bool ApplyMovieTransitionCueCadence80027194() {
    const bool cueRequired =
        s_movieTransitionCueCadenceCounter8006EC20 >= 2;
    if (cueRequired) {
        s_movieTransitionCueCadenceCounter8006EC20 = 0;
    }
    ++s_movieTransitionCueCadenceCounter8006EC20;
    if (cueRequired) {
        PlayMovieTransitionCue8006EC18();
    }
    return cueRequired;
}

Scene0UiCueCommand80025C8C ResolveScene0UiCue80025C8C(uint16_t code) {
    Scene0UiCueCommand80025C8C out{};
    const UiCueSelection80025C8C selection =
        ResolveUiCueSelection80025C8C(code);
    if (selection.command == nullptr) {
        return out;
    }
    out.accepted = true;
    out.inputMask = code;
    out.program = selection.command->program;
    out.note = selection.command->note;
    out.key = selection.command->key;
    out.volume = selection.command->volume;
    return out;
}

bool GetScene0IntVabCuePcmInfo80025C8C(uint16_t code,
                                      size_t& outSamples,
                                      uint32_t& outPlayRate) {
    outSamples = 0u;
    outPlayRate = 0u;
    const Scene0UiCueCommand80025C8C command =
        ResolveScene0UiCue80025C8C(code);
    return command.accepted && s_vabLoaded &&
           s_vab.GetTonePcmInfo(command.program,
                                command.note,
                                command.key,
                                outSamples,
                                outPlayRate);
}

bool PlayScene0UiCue80025C8CRaw(uint16_t code) {
    const UiCueSelection80025C8C selection =
        ResolveUiCueSelection80025C8C(code);
    if (selection.command == nullptr) {
        return false;
    }
    const bool directScene0VabVoice =
        TryPlayScene0CompactCueDirect80026EF8(
            *selection.command);
    // SCUS 80025C8C owns the complete accepted-input tail: 80026EF8 first,
    // then 80026ECC even when voice allocation/playback did not succeed.
    ApplySharedAudioDriverFlushBarrier26ECC();
    return directScene0VabVoice;
}

bool PlayScene0NavigateCue80025C8C() {
    // All four directional PSX masks select the same 80094420 command. Use
    // the right-arrow mask as the representative code after the direct
    // dispatcher has committed a navigation edge.
    return PlayScene0UiCue80025C8CRaw(0x2000u);
}

bool PlayScene0ConfirmCue80025C8C() {
    return PlayScene0UiCue80025C8CRaw(0x0020u);
}

bool PlayScene0CancelCue80025C8C() {
    return PlayScene0UiCue80025C8CRaw(0x0040u);
}

bool PlayScene0TransitionCue94410() {
    // 801C4894 emits 94410 through the same translated 80026EF8 owner as
    // the Scene0 input cues.  Do not use PlayCompactCue here: that helper is
    // retained for non-SS0 callers and bypasses the translated SPU state.
    return TryPlayScene0CompactCueDirect80026EF8(kSceneTransitionCue94410);
}

bool PlayScene0MovieTransitionCue800271E4(uint8_t cueIndex) {
    if (cueIndex >= kScene0MovieTransitionCue800271E4.size()) {
        return false;
    }
    const CompactSfxCmd& command =
        kScene0MovieTransitionCue800271E4[cueIndex];
    const bool directScene0VabVoice =
        TryPlayScene0CompactCueDirect80026EF8(command);
    // 800271E4 calls 80026ECC unconditionally after 80026EF8 and returns its
    // value.  The transition caller ignores that value, but the driver flush
    // remains an observable Scene0 side effect even if voice allocation fails.
    ApplySharedAudioDriverFlushBarrier26ECC();
    Log::Printf(
        "PrSfx: Scene0 800271E4 cue index=%u source=%08X command=%u/%u/%u/%u translatedVoice=%d",
        static_cast<unsigned>(cueIndex),
        0x8009441Cu + 6u * static_cast<unsigned>(cueIndex),
        static_cast<unsigned>(command.program),
        static_cast<unsigned>(command.note),
        static_cast<unsigned>(command.key),
        static_cast<unsigned>(command.volume),
        directScene0VabVoice ? 1 : 0);
    return directScene0VabVoice;
}

Scene0OptionsLanguageCueCommand80025DBC
ResolveScene0OptionsLanguageCue80025DBC() {
    Scene0OptionsLanguageCueCommand80025DBC out{};
    out.accepted = true;
    out.pointerSlotAddress = kScene0OptionsLanguageCuePointerSlot80094400;
    out.sourceAddress = kScene0OptionsLanguageCueSource801C6E7C;
    out.program = kScene0OptionsLanguageCue80025DBC.program;
    out.note = kScene0OptionsLanguageCue80025DBC.note;
    out.key = kScene0OptionsLanguageCue80025DBC.key;
    out.volume = kScene0OptionsLanguageCue80025DBC.volume;
    return out;
}

bool GetScene0IntVabOptionsLanguageCuePcmInfo80025DBC(
    size_t& outSamples,
    uint32_t& outPlayRate) {
    outSamples = 0u;
    outPlayRate = 0u;
    const Scene0OptionsLanguageCueCommand80025DBC command =
        ResolveScene0OptionsLanguageCue80025DBC();
    return command.accepted && s_vabLoaded &&
           s_vab.GetTonePcmInfo(command.program,
                                command.note,
                                command.key,
                                outSamples,
                                outPlayRate);
}

bool PlayScene0OptionsLanguageCue80025DBCRaw() {
    const bool directScene0VabVoice =
        TryPlayScene0CompactCueDirect80026EF8(
            kScene0OptionsLanguageCue80025DBC);
    // SCUS 80025DBC copies the six-byte command, forces byte 3 to 0x5A,
    // and owns the same unconditional 80026ECC tail as 80025C8C.
    ApplySharedAudioDriverFlushBarrier26ECC();
    return directScene0VabVoice;
}

void PlayStage1UiCue80025C8CRaw(uint16_t code) {
    const UiCueSelection80025C8C selection =
        ResolveUiCueSelection80025C8C(code);
    if (selection.command == nullptr) {
        return;
    }
    PlayStage1CompactCue(*selection.command,
                         s_stage1UiCue80025C8CPcm[
                             selection.fallbackIndex]);
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
        if (engine.GetVoiceGeneration(s_bgmVoice) == s_bgmVoiceGeneration &&
            engine.IsVoiceActive(s_bgmVoice)) {
            return; // already playing
        }
        // voice went stale, clear
        s_bgmVoice = -1;
        s_bgmVoiceGeneration = 0u;
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
    const bool loopInfoKnown = s_vab.GetTonePcmInfo(
        0, 7, 0x1F, dummySamples, dummyRate, &loopStart, &loopEnd);
    if (!loopInfoKnown || dummySamples == 0u || dummyRate == 0u ||
        loopEnd <= loopStart || loopEnd > dummySamples) {
        Log::Printf(
            "PrSfx: PlayBgm - invalid VAG loop metadata known=%d samples=%zu rate=%u loop=%zu..%zu",
            loopInfoKnown ? 1 : 0,
            dummySamples,
            dummyRate,
            loopStart,
            loopEnd);
        return;
    }

    // Use PlaySfxCmdEx to get the exact voice ID — avoids searching and
    // accidentally finding STR player's voice instead of BGM voice.
    s_bgmVoice = s_vab.PlaySfxCmdEx(0, 7, 0x1F, 0x5A);

    if (s_bgmVoice >= 0) {
        s_bgmVoiceGeneration = engine.GetVoiceGeneration(s_bgmVoice);
        engine.SetVoiceLooping(s_bgmVoice, true);
        engine.SetVoiceLoopRegion(s_bgmVoice, loopStart, loopEnd);
        Log::Printf(
            "PrSfx: PlayBgm - started BGM loop on voice %d samples=%zu rate=%u loop=%zu..%zu",
            s_bgmVoice,
            dummySamples,
            dummyRate,
            loopStart,
            loopEnd);
    } else {
        Log::Printf("PrSfx: PlayBgm – failed to allocate voice");
    }
}

void StopBgm() {
    if (s_bgmVoice >= 0) {
        auto& engine = AudioEngine::Get();
        engine.FreeVoiceIfGeneration(s_bgmVoice, s_bgmVoiceGeneration);
        Log::Printf("PrSfx: StopBgm – freed voice %d", s_bgmVoice);
        s_bgmVoice = -1;
        s_bgmVoiceGeneration = 0u;
    }
}

bool IsBgmPlaying() {
    if (s_bgmVoice < 0) return false;
    auto& engine = AudioEngine::Get();
    return engine.GetVoiceGeneration(s_bgmVoice) == s_bgmVoiceGeneration &&
           engine.IsVoiceActive(s_bgmVoice);
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
