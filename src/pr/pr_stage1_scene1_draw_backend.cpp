#include "pr_stage1_scene1_draw_backend.h"

#include "d3d11_renderer.h"
#include "pr_game_context.h"
#include "pr_psx_fast_sprite_submit_direct.h"
#include "pr_psx_sprite_template_render.h"
#include "pr_scn1.h"
#include "pr_stage1_hud_presentation_direct.h"
#include "pr_stage1_live_hud.h"
#include "pr_stage1_scene1_movie1_direct.h"
#include "pr_stage_scene_submit_backend.h"
#include "pr_stage_scene_submit_runtime_private.h"
#include "str_player.h"

#include <algorithm>
#include <cstdint>

namespace PrStage1Scene1DrawBackend {
namespace {

void CalcPs1Viewport(const D3D11Renderer* renderer,
                     float& outX,
                     float& outY,
                     float& outScale) {
    outX = 0.0f;
    outY = 0.0f;
    outScale = 1.0f;
    if (!renderer) {
        return;
    }

    const float winW = (float)renderer->GetWidth();
    const float winH = (float)renderer->GetHeight();
    const float baseW = 320.0f;
    const float baseH = 240.0f;
    const float fitX = (baseW > 0.0f) ? (winW / baseW) : 1.0f;
    const float fitY = (baseH > 0.0f) ? (winH / baseH) : 1.0f;
    outScale = (std::min)(fitX, fitY);
    outX = (winW - baseW * outScale) * 0.5f;
    outY = (winH - baseH * outScale) * 0.5f;
}

void SubmitAndDrawStage1Scene801CBFDC190(PrGameContext& ctx,
                                         bool forceLogicFrame,
                                         bool allowTransitionFreeze = false,
                                         bool allowStoppedRuntimeFreeze = false,
                                         bool includeGameplayUi = true) {
    PrStage1RuntimeSlotsSnapshot runtimeSlots{};
    const bool haveRuntimeSlots =
        PrScn1::GetStage1RuntimeSlotsSnapshot(
            ctx,
            runtimeSlots,
            allowTransitionFreeze,
            allowStoppedRuntimeFreeze) &&
        runtimeSlots.valid;
    if (!haveRuntimeSlots) {
        ResetGameplaySubmitRuntime();
        return;
    }

    const uint8_t renderSubFrame8 =
        (!forceLogicFrame && ctx.renderOnlyFrame && ctx.renderSubFrame8 > 0u)
            ? ctx.renderSubFrame8
            : 0u;
    const bool advanced =
        PrStageSceneSubmitBackend::
            AdvanceStage1SceneSubmitRuntimeForRender801CBFDC190(
                ctx,
                runtimeSlots,
                renderSubFrame8);
    if (advanced && renderSubFrame8 == 0u) {
        PrStage1HudPresentationDirectBee4ActionCarrier carrier{};
        PrStage1LiveHud::Stage1HudBee4RawActionRouteState routeState{};
        if (PrStage1LiveHud::BuildStage1RuntimeHudBee4RawActions(
                ctx,
                carrier,
                &routeState) &&
            routeState.available) {
            PrStage1HudPresentationDirectBee4ConsumeResult consumeResult{};
            (void)PrStageSceneSubmitDirect::
                ApplyOwnedStage1SceneSubmitRuntimeHudBee4RawActions801CBFDC190(
                    carrier,
                    &consumeResult);
        }
    }
    if (includeGameplayUi) {
        PrStageSceneSubmitBackend::DrawStage1Scene801CBFDC190(ctx);
    } else {
        PrStageSceneSubmitBackend::DrawStage1SceneGameplayBase801CBFDC190(ctx);
    }
}

} // namespace

void ResetGameplaySubmitRuntime() {
    PrStageSceneSubmitBackend::ResetStage1SceneSubmitRuntimeForRender801CBFDC190();
}

void ExecuteMovie1DrawPlan(
    PrGameContext& ctx,
    const PrStage1Scene1Movie1Direct::Movie1DrawPlan& plan) {
    if (plan.drawVideo && ctx.strPlayer) {
        ctx.strPlayer->RenderToRect(
            plan.video.x,
            plan.video.y,
            plan.video.w,
            plan.video.h);
    }

    for (uint32_t i = 0; i < plan.templateCount; ++i) {
        const PrStage1Scene1Movie1Direct::Movie1TemplateDrawCommand& command =
            plan.templates[i];
        if (!command.desc.valid) {
            continue;
        }
        const PrPsxSpriteTemplateRender::PsxSpriteTemplate tpl{
            command.desc.attr,
            command.desc.texX,
            command.desc.texY,
            command.desc.w,
            command.desc.h,
            command.desc.clutX,
            command.desc.clutY,
        };
        const bool drewFromVram =
            PrPsxSpriteTemplateRender::DrawPsxSpriteTemplateViaUiAtlas(
                ctx,
                plan.frame.vx,
                plan.frame.vy,
                plan.frame.vs,
                command.x,
                command.y,
                tpl,
                1.0f,
                1.0f,
                1.0f,
                command.alpha,
                command.layer,
                command.order);
        if (!drewFromVram) {
            (void)PrPsxSpriteTemplateRender::DrawPsxSpriteTemplateOrdered(
                ctx,
                plan.frame.vx,
                plan.frame.vy,
                plan.frame.vs,
                command.x,
                command.y,
                tpl,
                1.0f,
                1.0f,
                1.0f,
                command.alpha,
                command.layer,
                command.order);
        }
    }

    // Movie1 subtitle text must come from PSX fast-sprite submit packets.
}

void DrawMovie1FastSpriteRuntimeAndRouteHud(
    PrGameContext& ctx,
    const PrPsxFastSpriteSubmitDirect::RuntimeState8003FA20* runtime,
    bool drawRouteHud,
    bool drawTextRouteOnly) {
    if (runtime) {
        PrStageSceneSubmitBackend::DrawStage1FastSpriteRuntime8003FA20(
            ctx,
            *runtime);
    }
    if (drawRouteHud) {
        PrStage1LiveHud::DrawStage1RuntimeHudRouteOnly(ctx);
    } else if (drawTextRouteOnly) {
        PrStage1LiveHud::DrawStage1RuntimeTextRouteOnly(ctx);
    }
}

void DrawIntroTransition(PrGameContext& ctx, int frameIndex) {
    float vx = 0.0f;
    float vy = 0.0f;
    float vs = 1.0f;
    CalcPs1Viewport(ctx.renderer, vx, vy, vs);
    SubmitAndDrawStage1Scene801CBFDC190(ctx, true, true);
    PrStage1Scene1Movie1Direct::Movie1DrawPlan plan{};
    if (PrStage1Scene1Movie1Direct::BuildIntroTransitionDrawPlan(
            frameIndex,
            vx,
            vy,
            vs,
            plan)) {
        ExecuteMovie1DrawPlan(ctx, plan);
    }
    PrStage1LiveHud::DrawStage1RuntimeHudRouteOnly(ctx);
}

void DrawGameplaySubmitBaseOnly(PrGameContext& ctx,
                                bool allowTransitionFreeze) {
    SubmitAndDrawStage1Scene801CBFDC190(ctx, true, allowTransitionFreeze);
}

void DrawGameplaySubmitFrozenRuntimeBaseOnly(PrGameContext& ctx) {
    SubmitAndDrawStage1Scene801CBFDC190(ctx, true, false, true, false);
}

void DrawGameplaySubmitAndHud(PrGameContext& ctx) {
    SubmitAndDrawStage1Scene801CBFDC190(ctx, false);
    PrStage1LiveHud::DrawStage1RuntimeHud(ctx);
}

} // namespace PrStage1Scene1DrawBackend
