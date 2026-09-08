#pragma once

#include "pr_ss0_transition_direct.h"

namespace PrStage1SaveEntryDirect {

// 80019148 -> 80020110(0,3,2,1): 24 body + 4 tail presentations,
// VSync(2) per presentation. A Win render-only redraw never advances it.
struct State {
    bool pending = false;
    bool tickKnown = false;
    uint32_t lastLogicTick = 0;
    uint32_t presentations = 0;
    PrSS0TransitionDirect::SlowTransitionRuntime80020110 runtime{};
    PrSS0TransitionDirect::SlowTransitionTickResult80020110 frame{};
};

inline bool Begin(State& state) {
    state = {};
    state.pending = PrSS0TransitionDirect::BeginSlowTransitionRuntime80020110(
        state.runtime, 0u, 3, 2, 1);
    return state.pending;
}

inline bool ReadyForDispatcher(const State& state, uint32_t logicTick) {
    // Keep the fourth tail frame drawable for its own presentation interval.
    return state.pending && state.frame.complete && state.tickKnown &&
           logicTick != state.lastLogicTick;
}

inline bool Advance(State& state, uint32_t logicTick) {
    if (!state.pending || state.frame.complete ||
        (state.tickKnown && state.lastLogicTick == logicTick)) return false;
    State next = state;
    const auto first = PrSS0TransitionDirect::TickSlowTransitionRuntime80020110(next.runtime);
    const auto second = PrSS0TransitionDirect::TickSlowTransitionRuntime80020110(next.runtime);
    if (!first.accepted || first.presentRequired || !first.sub80027194CallRequired ||
        !second.accepted || !second.presentRequired || second.sub80027194CallRequired)
        return false;
    next.frame = second;
    next.tickKnown = true;
    next.lastLogicTick = logicTick;
    ++next.presentations;
    state = next;
    return true;
}

} // namespace PrStage1SaveEntryDirect
