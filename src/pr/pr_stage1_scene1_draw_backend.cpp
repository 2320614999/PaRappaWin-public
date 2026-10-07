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
#include "pr_ss0_transition_direct.h"
#include "str_player.h"
#include "logger.h"
#include "pr_psx_graph_owner_direct.h"
#include "pr_stage1_terminal_presentation_direct.h"

#include <algorithm>
#include <array>
#include <cstdint>

namespace PrStage1Scene1DrawBackend {
namespace {

PrStage1TerminalPresentationDirect::State s_terminalPresentation{};
std::array<ID3D11ShaderResourceView*, 2> s_terminalPages{};
ID3D11ShaderResourceView* s_terminalFront = nullptr;
bool s_terminalPageConsumed = false;

void ResetTerminalPages() {
    if (s_terminalFront) s_terminalFront->Release();
    s_terminalFront = nullptr;
    for (auto*& page : s_terminalPages) {
        if (page) page->Release();
        page = nullptr;
    }
    s_terminalPresentation = {};
    s_terminalPageConsumed = false;
}

void AcknowledgeTerminalFront(PrGameContext& ctx) {
    auto& state = s_terminalPresentation;
    if (!ctx.renderer || !PrStage1TerminalPresentationDirect::Poll(
            state, ctx.renderer->GetSuccessfulPresentCount())) return;
    auto* front = s_terminalPages[state.renderedPage];
    front->AddRef();
    s_terminalFront->Release();
    s_terminalFront = front;
    Log::Printf("Scene1 terminal801C7A60: presented cleanup frame=%u page=%u",
        state.frames, state.renderedPage);
}

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

bool SubmitAndDrawStage1Scene801CBFDC190(PrGameContext& ctx,
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
        return false;
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
    return true;
}

} // namespace

void ResetGameplaySubmitRuntime() {
    ResetTerminalPages();
    PrStageSceneSubmitBackend::ResetStage1SceneSubmitRuntimeForRender801CBFDC190();
}

bool BeginTerminalPresentation801C7A60(PrGameContext& ctx) {
    ResetTerminalPages();
    s_terminalPresentation.lastHostVblank = ctx.hostPresentationVblank60;
    // Called from the logic owner, before BeginFrame clears the previous RTV.
    // Keep its actual high-resolution contents while the first back page waits.
    return ctx.renderer && ctx.renderer->CaptureFrameTexture(s_terminalFront);
}

void DrawTerminalPresentation801C7A60(PrGameContext& ctx) {
    auto& state = s_terminalPresentation;
    if (!ctx.renderer || !s_terminalFront) return;
    auto& clock = PrPsxVSyncDirect::ProcessVSyncState80035560();
    AcknowledgeTerminalFront(ctx);
    // The native cleanup loop is suspended inside the runner, not a fresh
    // 30Hz gameplay update. Service its clock and next iteration on either
    // host render phase; do not insert a second 30Hz delay after VSync returns.
    PrStage1TerminalPresentationDirect::AdvanceWait(state, clock, ctx.hostPresentationVblank60,
        PrScn1::WasStage1TimecodeHostClockDelivered(ctx));
    if (state.pending) {
        if (PrStage1TerminalPresentationDirect::Publish(state, ctx.renderer->GetSuccessfulPresentCount())) {
            Log::Printf("Scene1 terminal801C7A60: VSync(2) complete frame=%u page=%u hostVblank=%llu->%llu consumed=%u",
                state.frames + 1u, state.renderedPage,
                static_cast<unsigned long long>(state.hostVblankAtSubmit),
                static_cast<unsigned long long>(state.lastHostVblank), clock.consumedHostVblankCount);
        }
    } else if (!state.copied && state.frames < 4) {
        // No prediction or owned-graph advance while a native frame is waiting
        // for VSync/Present, or while a later tail side effect remains pending.
        const bool submitted = SubmitAndDrawStage1Scene801CBFDC190(ctx, true);
        PrStage1LiveHud::DrawStage1RuntimeHud(ctx);
        if (submitted) {
            // 801C9178 already flipped the authoritative graph. This existing
            // host projection prepares it with RenderFrame; its new display
            // page remains hidden until the original wait has completed.
            const auto& graph = PrStageSceneSubmitDirect::GetOwnedStage1GraphOwner801CBFDC();
            const uint32_t active = PrPsxGraphOwnerDirect::PsxCall8004019C_GetDrawBuffer(graph);
            if (active < 2) {
                const auto page = static_cast<uint8_t>(PrStage1TerminalPresentationDirect::SourcePage(active));
                const bool queued = ctx.renderer->CaptureFrameTexture(s_terminalPages[page]);
                if (PrStage1TerminalPresentationDirect::Submit(state, page,
                        ctx.hostPresentationVblank60, queued, false, clock)) {
                    Log::Printf("Scene1 terminal801C7A60: rendered cleanup frame=%u page=%u VSync(2) pending hostVblank=%llu",
                        state.frames + 1u, page,
                        static_cast<unsigned long long>(ctx.hostPresentationVblank60));
                }
            }
        }
    }
    // Native SetDispEnv is after VSync(2), not after RenderFrame. A host redraw
    // during that wait displays the last acknowledged front and cannot count
    // as the pending page's Present. Full-window copies retain Win HD output.
    auto* displayed = state.pending && state.published
        ? s_terminalPages[state.renderedPage] : s_terminalFront;
    if (!displayed) return;
    ctx.renderer->DrawSprite(displayed, 0, 0,
        static_cast<float>(ctx.renderer->GetWidth()),
        static_cast<float>(ctx.renderer->GetHeight()));
}

bool CompleteTerminalDisplayMove8001B120(PrGameContext& ctx) {
    if (!ctx.renderer) return false;
    auto& state = s_terminalPresentation;
    if (state.copied) return true;
    AcknowledgeTerminalFront(ctx);
    const auto& graph = PrStageSceneSubmitDirect::GetOwnedStage1GraphOwner801CBFDC();
    const uint32_t active = PrPsxGraphOwnerDirect::PsxCall8004019C_GetDrawBuffer(graph);
    if (!PrStage1TerminalPresentationDirect::CanMove(state, active)) return false;
    const uint32_t source = PrStage1TerminalPresentationDirect::SourcePage(active);
    if (!ctx.renderer->CopyFrameTexture(s_terminalPages[active], s_terminalPages[source]))
        return false;
    state.copied = true;
    state.copiedPage = static_cast<uint8_t>(active);
    Log::Printf("Scene1 terminal801C7A60: 8001B120(1) queued page=%u->%u size=%dx%d hostGpuProjection=1 gpuFenceWait=0",
        source, active, ctx.renderer->GetWidth(), ctx.renderer->GetHeight());
    return true;
}

bool DrawLoadingPattern8001EF40(
    PrGameContext& ctx,
    const PrSS0TransitionDirect::LoadingPatternFrame8001EF40& frame) {
    if (!ctx.renderer || !frame.known ||
        frame.sourceFunction != PrSS0TransitionDirect::kFn8001EF40 ||
        frame.drawHighlightFunction != PrSS0TransitionDirect::kFn8001C4EC ||
        frame.drawTileFunction != PrSS0TransitionDirect::kFn8001C550) return false;
    const auto grid = PrSS0TransitionDirect::ResolveSlowTransitionVisualFrame80020110(
        PrSS0TransitionDirect::kScene0WorkAddress, 2, 1, 2, false, 23u);
    const auto plan = PrSS0TransitionDirect::BuildSlowTransitionFramePlan8001FDC0(grid);
    if (!plan.known || plan.truncated || plan.commandCount != 192u) return false;
    float vx, vy, vs;
    CalcPs1Viewport(ctx.renderer, vx, vy, vs);
    for (std::size_t i = 0; i < plan.commandCount; ++i) {
        const auto& command = plan.commands[i];
        const auto& source = command.spriteTemplate;
        const PrPsxSpriteTemplateRender::PsxSpriteTemplate tpl{
            source.attr, source.texX, source.texY, source.width, source.height,
            source.clutX, source.clutY};
        if (!PrPsxSpriteTemplateRender::DrawPsxSpriteTemplateViaUiAtlas(
                ctx, vx, vy, vs, float(command.x), float(command.y), tpl,
                1.0f, 1.0f, 1.0f, 1.0f, 784, 0) &&
            !PrPsxSpriteTemplateRender::DrawPsxSpriteTemplateOrdered(
                ctx, vx, vy, vs, float(command.x), float(command.y), tpl,
                1.0f, 1.0f, 1.0f, 1.0f, 784, 0)) return false;
    }
    // 8001C4EC's GsBOXF attr is converted by GsSortBoxFill, not an RGBA
    // literal. The native same-priority AddPrim order places it over tiles.
    const uint32_t color = frame.boxFillGpuColorCode8003EE84;
    uint32_t submitted = 0;
    for (std::size_t i = 0; i < frame.liveGrid.size(); ++i) {
        if (!frame.liveGrid[i]) continue;
        D3D11Renderer::SolidRectCmd rect{};
        rect.x = vx + float(i % 16u) * 20.0f * vs;
        rect.y = vy + float(i / 16u) * 20.0f * vs;
        rect.w = rect.h = 20.0f * vs;
        rect.r = float(color & 0xFFu) / 255.0f;
        rect.g = float((color >> 8u) & 0xFFu) / 255.0f;
        rect.b = float((color >> 16u) & 0xFFu) / 255.0f;
        rect.a = ((color >> 24u) & 2u) ? 0.5f : 1.0f;
        rect.layer = 784;
        rect.order = (uint64_t{1} << 48u) | uint64_t(i + 1u);
        ctx.renderer->SubmitSolidRect(rect);
        ++submitted;
    }
    return submitted == frame.highlightCount;
}

void ExecuteMovie1DrawPlan(
    PrGameContext& ctx,
    const PrStage1Scene1Movie1Direct::Movie1DrawPlan& plan) {
    static bool directVideoSubmitLogged = false;
    if (plan.drawVideo) {
        if (plan.drawDirectVideo && plan.directVideoTexture && ctx.renderer) {
            // The original MOVIE1 path submits the decoded MDEC image to the
            // same 256x144 rectangle as 8001CE30/8001C864.  Keep it in the
            // ordered sprite queue so native frame templates remain layered
            // above it.  There is deliberately no Host StrPlayer fallback:
            // SS0 owns presentation, and a host projection here would create
            // the old Win-S0/SS0 mixed-shell frame while direct MDEC is
            // waiting for its first valid decoded picture.
            D3D11Renderer::SpriteCmd movie{};
            movie.texture = plan.directVideoTexture;
            movie.x = plan.video.x;
            movie.y = plan.video.y;
            movie.w = plan.video.w;
            movie.h = plan.video.h;
            // Submission order alone is insufficient: FlushSprites sorts by
            // layer first. 700 put the MDEC rectangle over templates at 480,
            // cutting away their nonrectangular inner edge. Reproduce the
            // original LoadImage -> DrawOT order without cropping the movie.
            movie.layer = PrStage1Scene1Movie1Direct::kMovie1MdecBackgroundLayer;
            movie.order = 1;
            ctx.renderer->SubmitSprite(movie);
            if (!directVideoSubmitLogged) {
                directVideoSubmitLogged = true;
                Log::Printf(
                    "Scene1::MOVIE1 direct MDEC texture submitted rect=%.1f,%.1f %.1fx%.1f layer=%d",
                    movie.x, movie.y, movie.w, movie.h, movie.layer);
            }
        }
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
    if (ctx.renderer && s_terminalPresentation.copied &&
        s_terminalPages[s_terminalPresentation.copiedPage]) {
        // Native clear-tail transition starts over the retained display page.
        // Do not rerun the frozen scene graph and lose its final cleanup state.
        ctx.renderer->DrawSprite(s_terminalPages[s_terminalPresentation.copiedPage],
            0, 0, static_cast<float>(ctx.renderer->GetWidth()),
            static_cast<float>(ctx.renderer->GetHeight()));
        if (!s_terminalPageConsumed) {
            s_terminalPageConsumed = true;
            Log::Printf("Scene1 terminal801C7A60: retained copied page=%u consumed by clear-tail transition",
                s_terminalPresentation.copiedPage);
        }
        return;
    }
    SubmitAndDrawStage1Scene801CBFDC190(ctx, true, false, true, false);
}

bool DrawGameplaySubmitAndHud(PrGameContext& ctx) {
    const bool submitted = SubmitAndDrawStage1Scene801CBFDC190(ctx, false);
    PrStage1LiveHud::DrawStage1RuntimeHud(ctx);
    return submitted;
}

} // namespace PrStage1Scene1DrawBackend
