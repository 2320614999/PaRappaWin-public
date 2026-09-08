#pragma once

// UI sound effects – original PSX VAB samples from MINIMUM.VH/VB
// Fallback to procedural tones if VAB not available

#include "vab_player.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace PrSfx {
    static constexpr uint32_t
        kScene0OptionsLanguageCuePointerSlot80094400 = 0x80094400u;
    static constexpr uint32_t
        kScene0OptionsLanguageCueSource801C6E7C = 0x801C6E7Cu;

    struct Scene0IntVabCandidate8001A8F0 {
        VabPlayer player{};
        bool prepared = false;
        size_t vhSize = 0u;
        size_t vbSize = 0u;
        uint64_t vhHash = 0u;
        uint64_t vbHash = 0u;
        int toneCount = 0;
        int vagCount = 0;
    };

    struct Scene0UiCueCommand80025C8C {
        bool accepted = false;
        uint16_t inputMask = 0u;
        uint8_t program = 0u;
        uint8_t note = 0u;
        uint8_t key = 0u;
        uint8_t volume = 0u;
    };

    struct Scene0OptionsLanguageCueCommand80025DBC {
        bool accepted = false;
        uint32_t pointerSlotAddress = 0u;
        uint32_t sourceAddress = 0u;
        uint8_t program = 0u;
        uint8_t note = 0u;
        uint8_t key = 0u;
        uint8_t volume = 0u;
    };

    struct SharedAudioResetBarrierResult26FA4 {
        bool known = false;
        bool translated = false;
        bool committed = false;
        bool zeroVoiceCountNoOp = false;
        bool hostVoiceResetProjectionReady = false;
        uint8_t returnedVoiceCount = 0u;
        uint32_t resetVoiceMask = 0u;
        uint16_t keyOffLowAfter = 0u;
        uint16_t keyOffHighAfter = 0u;
        uint16_t keyOnLowAfter = 0u;
        uint16_t keyOnHighAfter = 0u;
        uint32_t wrapperFunction = 0x80026FA4u;
        uint32_t resetFunction = 0x800351B8u;
        int32_t resetArg = 0;
        bool psxSpuRegisterAuthority = false;
        bool psxInterruptTimingAuthority = false;
    };

    // Initialize SFX system
    void Init();

    // Load original VAB samples (call after Init)
    bool InitVab(const std::string& vhPath, const std::string& vbPath);

    bool InitVabFromMemory(const uint8_t* vhData, size_t vhSize,
                           const uint8_t* vbData, size_t vbSize);

    // Native shared INT loader boundaries. Results are software driver/slot
    // results; the existing Win mixer, volume and replacement banks remain.
    bool InitializeIntVabDriver80026E4C();
    bool OpenIntVab80027078(const uint8_t* vh, size_t bytes, int16_t& result);
    bool TransferIntVab800270D4(const uint8_t* vb, size_t bytes, int32_t& result);
    bool CompleteIntVab800270FC(int32_t& result);
    bool PlayIntLoaderCue94410(int16_t& result);

    Scene0IntVabCandidate8001A8F0
    BuildScene0IntVabCandidate8001A8F0(
        const uint8_t* vhData,
        size_t vhSize,
        const uint8_t* vbData,
        size_t vbSize);

    bool IsExactScene0IntVabCandidate8001A8F0(
        const Scene0IntVabCandidate8001A8F0& candidate,
        size_t vhSize,
        uint64_t vhHash,
        size_t vbSize,
        uint64_t vbHash,
        uint32_t toneCount,
        uint32_t vagCount);

    void CommitScene0IntVabCandidate8001A8F0(
        Scene0IntVabCandidate8001A8F0&& candidate);

    // SCUS 8001A8F0 calls 80027120 before opening the next VAB.  This result
    // records the translated software close/reset boundary without claiming
    // physical SPU register or timing authority.
    struct Scene0VabCloseResult80027120 {
        bool known = false;
        bool committed = false;
        bool closeRequested = true;
        bool priorHostVabActive = false;
        bool hostProjectionCleared = false;
        bool psxSpuRegisterAuthority = false;
        bool psxInterruptTimingAuthority = false;
        uint32_t function = 0x80027120u;
    };

    Scene0VabCloseResult80027120 ApplyScene0VabClose80027120();

    struct Scene0SsMidiNoteProjectionResult8002BEEC {
        bool known = false;
        bool hostVabReady = false;
        bool noteOn = false;
        bool noteOff = false;
        uint32_t matchedToneLayerCount = 0u;
        uint32_t directAllocatedVoiceCount = 0u;
        uint32_t directAllocatedVoiceMask = 0u;
        uint32_t directReplacedVoiceMask = 0u;
        uint32_t directSampleStartKnownVoiceMask = 0u;
        uint32_t startedVoiceCount = 0u;
        uint32_t stoppedVoiceCount = 0u;
        bool directDriverStateTranslated = false;
        bool directVabBodyTransferTranslated = false;
        bool fixedVoiceSlotHostProjection = false;
        bool sourceVabIdOwnershipKeyOnly = false;
        bool psxSampleStartAddressAuthority = false;
        bool psxVabIdAuthority = false;
        bool psxVoiceIdentityAuthority = false;
        bool psxSpuTimingAuthority = false;
    };

    bool ResolveScene0SsMidiNoteToneLayers80032EAC(
        uint8_t program,
        uint8_t note,
        std::vector<int>& outToneIndices);

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
        bool noteOn);

    void ResetScene0SsMidiNoteProjection8003349C();

    Scene0UiCueCommand80025C8C ResolveScene0UiCue80025C8C(
        uint16_t code);
    bool GetScene0IntVabCuePcmInfo80025C8C(uint16_t code,
                                           size_t& outSamples,
                                           uint32_t& outPlayRate);
    bool PlayScene0UiCue80025C8CRaw(uint16_t code);
    // SS0-only wrapper for the complete original 80025C8C input-code map.
    // Accepted masks execute 80026EF8/80034240 and the function-owned
    // 80026ECC tail; the generic helpers below remain for non-SS0 callers.
    bool PlayScene0NavigateCue80025C8C();
    bool PlayScene0ConfirmCue80025C8C();
    bool PlayScene0CancelCue80025C8C();
    // SS0 title transition cue owner: execute the translated
    // 80026EF8/80034240 command without falling through the generic
    // host-side compact-cue helper.  The caller owns the following 26ECC
    // flush, matching the original call order.
    bool PlayScene0TransitionCue94410();
    // SCUS 800271E4 reads one of the two six-byte commands behind
    // Scene0's 8009441C -> 801C6EC4 binding, then executes the translated
    // 80026EF8 -> 80026ECC sequence.  This is independent of the legacy
    // Movie1 shell cue helper below.
    bool PlayScene0MovieTransitionCue800271E4(uint8_t cueIndex);
    Scene0OptionsLanguageCueCommand80025DBC
    ResolveScene0OptionsLanguageCue80025DBC();
    bool GetScene0IntVabOptionsLanguageCuePcmInfo80025DBC(
        size_t& outSamples,
        uint32_t& outPlayRate);
    // Complete original 80025DBC owner, including its 80026ECC tail.
    bool PlayScene0OptionsLanguageCue80025DBCRaw();

    bool InitStage1VabFromInt(const std::string& stage1CompoIntPath);

    bool InitPracticeVabFromInt(const std::string& ycompoIntPath);

    // Play predefined sounds
    void PlayNavigate();   // cursor move
    void PlayConfirm();    // button press / select
    void PlayCancel();     // cancel / back

    void PlayPracticeBeatCue();

    void PlayPracticeBeat();

    void PlayPracticeLoopStart();

    void PlayPracticeHiTick();

    void PlayPracticeRoundPromptCue(int round);

    void PlayPracticePrompt(int round);
    void PlayPracticeResultVoiceCue(int judgeKind);
    void PlayPracticeRoundIntroVoiceCue(int round);
    void PlayPracticeCompletePromptVoiceCue();
    void PlayPracticeExitPromptVoiceCue();
    void PlayPracticeVoiceKind(int kind);

    // Stage1 steady-gameplay bucket30 direct cue tables.
    void PlayStage1SteadyVerdictCue(uint8_t tableSlot);
    void PlayStage1SteadyRowCommitCue(uint8_t tableSlot);
    void PlayStage1Bucket30DirectCue94400(uint8_t tableSlot);
    void PlayStage1DelayedFollowUpArmCue();
    void PlayStage1DelayedFollowUpPrepareCue(bool secondOrLater);
    void PlayStage1SteadyDelayedCompletionCue();
    void PlayStage1FailTailCue943EC();
    void PlayStage1SourceCellVoiceCue(uint8_t program,
                                      uint8_t note,
                                      uint8_t key,
                                      uint8_t volume,
                                      double startOffsetSeconds = 0.0);
    void ResetStage1SourceCellVoiceCueLane();
    SharedAudioResetBarrierResult26FA4 ApplySharedAudioResetBarrier26FA4();
    void ApplySharedAudioDriverFlushBarrier26ECC();
    int32_t ApplySharedAudioDriverFlushBarrier26ECCResult();
    bool IsStage1VabReadyForHostProjection800943A8();
    int16_t PlayStage1Cue80034240HostProjection(
        uint16_t hostDriverToken,
        uint8_t program,
        uint8_t note,
        uint8_t key,
        uint16_t arg4,
        uint8_t volumeLeft,
        uint8_t volumeRight);
    void PlaySceneTransitionCue94410();
    void PlayMovieTransitionCue8006EC18();
    bool ApplyMovieTransitionCueCadence80027194();
    void PlayStage1UiCue80025C8CRaw(uint16_t code);
    void PlayMovie1ShellCue9441CRaw(uint8_t cueIndex);
    void PlayMovie1ShellCue9441C(uint8_t cueIndex);

    // Scene0: Movie0 UI/transition sounds
    void PlayScn0GridIn();    // tiles fill in
    void PlayScn0GridOut();   // tiles retract
    void PlayScn0WaveIn();    // wave border shows
    void PlayScn0WaveOut();   // wave border hides
    void PlayScn0SubIn();     // subtitle box shows
    void PlayScn0SubOut();    // subtitle box hides

    // BGM loop (PSX tone_007 - title/menu shared BGM)
    void PlayBgm();           // start BGM loop (no-op if already playing)
    void StopBgm();           // stop BGM
    bool IsBgmPlaying();

    bool GetBgmPcmInfo(size_t& outSamples, uint32_t& outPlayRate);

    // Master volume (0.0 - 1.0)
    void SetVolume(float vol);
    float GetVolume();

    bool DumpS0Wav(const std::string& outDir);
}
