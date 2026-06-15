#pragma once

#include "pr_stage1_common_lyrics_geometry_direct.h"

#include <cstdint>
#include <array>
#include <string>

struct PrGameContext;
struct PrStage1ResolvedTextEvent;
struct PrStage1HudPresentationDirectBee4ActionCarrier;
namespace PrStage1LiveHud {

struct Stage1HudPaintContext {
    PrGameContext& game;
    float vx = 0.0f;
    float vy = 0.0f;
    float vs = 1.0f;
};

enum class Stage1ResolvedHudTextPathKind : uint8_t {
    None = 0,
    SteadyGameplayScriptText,
    GenericLyricsBox,
};

enum class Stage1ResolvedHudTextSourceKind : uint8_t {
    None = 0,
    OverlayScriptTextRuntimeSnapshot,
    CommonLyricsRuntimeSnapshot,
};

// Sanitized payload for one routed Stage1 text source.
struct Stage1ResolvedHudTextVisual {
    bool available = false;
    bool hasText = false;
    Stage1ResolvedHudTextSourceKind sourceKind = Stage1ResolvedHudTextSourceKind::None;
    Stage1ResolvedHudTextPathKind pathKind = Stage1ResolvedHudTextPathKind::None;
    std::string rawText;
    PrStage1CommonLyricsFrameSub8001C864 commonLyricsFrameSub8001C864{};
};

// Source-routed Stage1 text state. The steady gameplay script box consumes the
// Stage1-local overlay script-text runtime mirror; common lyrics consume the
// Stage1 VText direct runtime and must not borrow overlay text.
struct Stage1ResolvedHudTextRouteState {
    Stage1ResolvedHudTextVisual steadyGameplayScript{};
    Stage1ResolvedHudTextVisual commonLyrics{};
    Stage1ResolvedHudTextVisual active{};
    bool anyAvailable = false;
};

// Read-only Stage1 steady-gameplay numeric/status route state. The confirmed
// runtime origins group into three presentation-facing sub-states:
// scoreDisplayState (ctx+0x30 cumulative display mirror), rightRankState
// (ctx+0x4E active-row selector + ctx+0x58 blink-target selector +
// ctx+0x5A blink enable bit), and topLessonPairState
// (ctx+0x5C change-triggered visible bit + ctx+0x5E lesson selector).
struct Stage1ResolvedHudScoreDisplayState {
    // bucket30 mirror + cumulative display value; keep this below gameplay
    // formula names.
    int32_t scoreDisplayValue = 0;
};

struct Stage1ResolvedHudRightRankState {
    // right-side 4-row output: active-row selector, blink-target
    // selector, and blink enable bit.
    uint8_t rightRankActiveRow = 0;
    uint8_t rightRankBlinkTargetRow = 0;
    bool rightRankBlinkEnabled = false;
    bool rightRankFlag0200Pulse = false;
    uint16_t rightRankTransitionAnim18E = 0;
};

struct Stage1ResolvedHudTopLessonPairState {
    // top LESSON pair change-triggered visible bit plus lesson selector.
    bool topLessonPairChangeVisible = false;
    uint8_t topLessonPairLessonId = 0;
};

struct Stage1ResolvedHudNumericStatusRouteState {
    bool available = false;
    Stage1ResolvedHudScoreDisplayState scoreDisplayState{};
    Stage1ResolvedHudRightRankState rightRankState{};
    Stage1ResolvedHudTopLessonPairState topLessonPairState{};
    uint16_t transitionState916D0 = 0;
    bool steadyGameplayGateActive = false;
    bool highLayoutMode = true;
};

struct Stage1HudBee4RawActionRouteState {
    bool available = false;
    bool numericRouteAvailable = false;
    bool steadyGameplayGateActive = false;
};

// Builds the direct 8001DF24 -> 8001BEE4 raw-action carrier for the
// current Stage1 HUD route. This only forwards PSX raw actions; it does not
// execute renderer draw, infer RGB, or assign a submit owner.
bool BuildStage1RuntimeHudBee4RawActions(
    PrGameContext& ctx,
    PrStage1HudPresentationDirectBee4ActionCarrier& outCarrier,
    Stage1HudBee4RawActionRouteState* outRouteState = nullptr);

// Draws the Stage1 runtime HUD surfaces that still belong in this module:
// source-routed text plus steady numeric/status presentation. Scene rail
// sprites now come from the PSX scene-submit backend, not this HUD path.
void DrawStage1LiveSnapshotHud(const Stage1HudPaintContext& hudCtx,
                               const Stage1ResolvedHudTextRouteState& textRoute,
                               const Stage1ResolvedHudNumericStatusRouteState& numericRoute);

// Stage1 runtime HUD entrypoint for the stage runner. The runner should only
// forward here; route selection is owned by PrScn1 direct snapshots and this
// module only performs viewport adaptation and drawing.
void DrawStage1RuntimeHud(PrGameContext& ctx);

// Draws only the Stage1 HUD route for MOVIE1/outro/transition frames that
// share Scene1 UI state but must not render scene-submit sprites.
void DrawStage1RuntimeHudRouteOnly(PrGameContext& ctx);

// Draws only the direct Stage1 text route. This is used by movie routes that
// need common-lyrics text but must not draw gameplay numeric/status HUD.
void DrawStage1RuntimeTextRouteOnly(PrGameContext& ctx);

}
