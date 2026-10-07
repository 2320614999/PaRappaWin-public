#pragma once

class StrPlayer;
struct PrGameContext;

namespace PrPsxFastSpriteSubmitDirect {
struct RuntimeState8003FA20;
}

namespace PrStage1Scene1Movie1Direct {
struct Movie1DrawPlan;
}

namespace PrSS0TransitionDirect {
struct LoadingPatternFrame8001EF40;
}

namespace PrStage1Scene1DrawBackend {

void ResetGameplaySubmitRuntime();
bool BeginTerminalPresentation801C7A60(PrGameContext& ctx);
void DrawTerminalPresentation801C7A60(PrGameContext& ctx);
bool CompleteTerminalDisplayMove8001B120(PrGameContext& ctx);

bool DrawLoadingPattern8001EF40(
    PrGameContext& ctx,
    const PrSS0TransitionDirect::LoadingPatternFrame8001EF40& frame);

void ExecuteMovie1DrawPlan(
    PrGameContext& ctx,
    const PrStage1Scene1Movie1Direct::Movie1DrawPlan& plan);

void DrawMovie1FastSpriteRuntimeAndRouteHud(
    PrGameContext& ctx,
    const PrPsxFastSpriteSubmitDirect::RuntimeState8003FA20* runtime,
    bool drawRouteHud = true,
    bool drawTextRouteOnly = false);

void DrawIntroTransition(PrGameContext& ctx, int frameIndex);

void DrawGameplaySubmitBaseOnly(PrGameContext& ctx,
                                bool allowTransitionFreeze = false);

void DrawGameplaySubmitFrozenRuntimeBaseOnly(PrGameContext& ctx);

bool DrawGameplaySubmitAndHud(PrGameContext& ctx);

} // namespace PrStage1Scene1DrawBackend
