#pragma once

#include <cstdint>

namespace PrStage1Scene1RenderRouterDirect {

enum class Scene1RenderRoute : uint8_t {
    None,
    ClearResultMovieDraw,
    Movie1Draw,
    IntroTransitionDraw,
    GameplaySubmitDraw,
};

struct Scene1RenderRouterInput {
    bool rendererReady = false;
    bool clearResultMovieDrawActive = false;
    bool clearResultMoviePlayerReady = false;
    bool movie1DrawableActive = false;
    bool stageRunning = false;
    bool introTransitionDrawActive = false;
};

struct Scene1RenderRouterCarrier {
    bool rendererReady = false;
    bool clearResultMovieDrawActive = false;
    bool clearResultMoviePlayerReady = false;
    bool movie1DrawableActive = false;
    bool stageRunning = false;
    bool introTransitionDrawActive = false;
    bool helperRenderRouteKnown = true;
};

inline Scene1RenderRoute ResolveScene1RenderRoute(
    const Scene1RenderRouterInput& input) {
    // Direct router owns PSX branch priority; the scene adapter owns drawing.
    if (!input.rendererReady) {
        return Scene1RenderRoute::None;
    }
    if (input.clearResultMovieDrawActive && input.clearResultMoviePlayerReady) {
        return Scene1RenderRoute::ClearResultMovieDraw;
    }
    if (input.movie1DrawableActive) {
        return Scene1RenderRoute::Movie1Draw;
    }
    if (input.stageRunning && input.introTransitionDrawActive) {
        return Scene1RenderRoute::IntroTransitionDraw;
    }
    if (input.stageRunning) {
        return Scene1RenderRoute::GameplaySubmitDraw;
    }
    return Scene1RenderRoute::None;
}

}  // namespace PrStage1Scene1RenderRouterDirect
