#pragma once

// UI sound effects – original PSX VAB samples from MINIMUM.VH/VB
// Fallback to procedural tones if VAB not available

#include <cstddef>
#include <cstdint>
#include <string>

namespace PrSfx {
    // Initialize SFX system
    void Init();

    // Load original VAB samples (call after Init)
    bool InitVab(const std::string& vhPath, const std::string& vbPath);

    bool InitVabFromMemory(const uint8_t* vhData, size_t vhSize,
                           const uint8_t* vbData, size_t vbSize);

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
    void ApplySharedAudioResetBarrier26FA4();
    void ApplySharedAudioDriverFlushBarrier26ECC();
    void PlaySceneTransitionCue94410();
    void PlayMovieTransitionCue8006EC18();
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
