#pragma once

#include <cstdint>

struct PrGameContext;

// Transition state machine
enum class TransitionPhase : uint8_t {
    Idle,
    FadeOut,
    Hold,
    FadeIn,
};

enum class TransitionTileOrder : uint8_t {
    EB80 = 0,
    F180 = 1,
};

enum class TransitionSource : uint8_t {
    Legacy = 0,
    SS0Direct = 1,
};

// Transition config
struct TransitionConfig {
    int fadeOutFrames = 16;
    int holdFrames = 4;
    int fadeInFrames = 16;
    int switchAtFrame = 16;
    TransitionTileOrder fadeOutOrder = TransitionTileOrder::EB80;
    TransitionTileOrder fadeInOrder = TransitionTileOrder::EB80;
};

namespace PrTransition {

// Initialize transition state
void Init();

// Start transition to target scene.
bool Start(int targetScene,
           const TransitionConfig& config = {},
           TransitionSource source = TransitionSource::Legacy);

// Start reveal-only animation without switching scenes.
bool StartRevealOnly(const TransitionConfig& config,
                     TransitionSource source = TransitionSource::Legacy);

// Narrow carrier for PSX `sub_80015408(mode)` / `sub_8001545C()` using the
// existing hold-phase curtain renderer.
bool StartLoadingCurtain15408(
    int16_t sceneExitReason,
    TransitionSource source = TransitionSource::Legacy);
void StopLoadingCurtain1545C();
bool HasLoadingCurtainCallbackFired15408();

// Per-frame update.
// -1 = inactive/finished
// >=0 = target scene to switch to
int Update(PrGameContext& ctx);

// Render transition overlay.
void Render(PrGameContext& ctx);

// Query state.
bool IsActive();
TransitionPhase GetPhase();
float GetAlpha();
int GetTotalFrameCount();
int GetPhaseFrameCount();
int GetTargetScene();
TransitionSource GetSource();

// Force stop.
void Cancel();

// PSX `sub_8001EF14`: reset the hold/tile overlay counters plus the
// 12x16 active-mask grid back to all-zero.
void ResetHoldOverlayState1EF14();

}  // namespace PrTransition
