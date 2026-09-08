#pragma once

#include "pr_ss0_transition_direct.h"

namespace PrStage1LoadingDirect {

// 80015590 -> 80015408(3) -> VSync callback 8001537C. The renderer
// consumes this translated 8001EF40 page; it never advances its clock.
struct State {
    PrSS0TransitionDirect::LoadingPatternRuntime8001EF40 pattern{};
    PrSS0TransitionDirect::LoadingPatternFrame8001EF40 frame{};
    bool tickKnown = false;
    uint32_t lastTick = 0;
    uint32_t callbacks = 0;
    bool presented = false;
    uint32_t firstPresentedTick = 0;
    uint32_t lastPresentedTick = 0;
    uint32_t presentedTicks = 0;
};

inline bool Begin(State& state, int16_t mode, const uint8_t* nativeGrid,
                  std::size_t count) {
    if (!PrSS0TransitionDirect::BeginLoadingPatternRuntimeAfter8001FFD4(
            state.pattern, mode, nativeGrid, count)) return false;
    state.frame = {};
    state.tickKnown = state.presented = false;
    state.callbacks = state.presentedTicks = 0;
    return true;
}

inline bool Tick(State& state, uint32_t logicTick) {
    if (!state.pattern.active ||
        (state.tickKnown && state.lastTick == logicTick)) return false;
    state.frame = PrSS0TransitionDirect::TickLoadingPatternRuntime8001EF40(
        state.pattern);
    if (!state.frame.known) return false;
    state.tickKnown = true;
    state.lastTick = logicTick;
    ++state.callbacks;
    return true;
}

inline bool BeginAfterReset8001EF14(State& state, int16_t mode) {
    return state.pattern.countersKnown &&
        Begin(state, mode, state.pattern.liveGrid.data(), state.pattern.liveGrid.size());
}

inline void RecordPresentation(State& state, uint32_t logicTick) {
    if (!state.pattern.active || !state.frame.known) return;
    if (!state.presented) {
        state.presented = true;
        state.firstPresentedTick = logicTick;
    } else if (state.lastPresentedTick == logicTick) {
        return; // Win 60 Hz/high-resolution redraw is not another PSX tick.
    }
    state.lastPresentedTick = logicTick;
    ++state.presentedTicks;
}

inline bool MinimumPresentationComplete(const State& state, uint32_t logicTick) {
    // User-requested Win extension, not a fabricated CD/SPU completion:
    // at least one second of the visible page at the 30 Hz scene cadence.
    return state.pattern.active && state.presented && state.presentedTicks >= 30u &&
           uint32_t(logicTick - state.firstPresentedTick) >= 30u;
}

inline void Stop(State& state) {
    PrSS0TransitionDirect::StopLoadingPatternRuntime8001EF40(state.pattern);
    state.frame = {};
}

} // namespace PrStage1LoadingDirect
