#pragma once

#include "pr_stage1_save_ui_direct.h"

#include <cstdint>

namespace PrStage1SavePresentationDirect {

// Continuation of the already translated action stream, not a second input
// dispatcher. 80017E6C executes 20 draw/VSync(0)/end transactions. The caller
// supplies the Windows 60Hz presentation transport independently of gameplay.
struct State {
    bool pending = false;
    bool blocked = false;
    uint32_t actionIndex = 0;
    uint32_t flashFrames = 0;
    uint32_t presentations = 0;
    uint64_t nextVblank = 0;
};

inline bool IsFlash(const PrStage1SaveUi19148Action& action) {
    return action.kind == PrStage1SaveUi19148ActionKind::Call80017E6CSetEventResult &&
        action.psxFunction == 0x80017E6Cu;
}

inline bool ContainsFlash(const PrStage1SaveUi19148ActionList& actions) {
    for (uint32_t i = 0; i < actions.count; ++i) {
        if (IsFlash(actions.actions[i])) return true;
    }
    return false;
}

inline void Begin(State& state, uint64_t vblank) {
    state = {};
    state.pending = true;
    state.nextVblank = vblank;
}

// Each successful consume commits one native action (or one flash frame).
// Even the LAST flash frame owns a complete interval before return. A normal
// frame between input-feedback and exit-feedback owns its own interval too.
// Catch-up at 30Hz emits both native transactions without slowing the flash.
template<class Consume>
bool Advance(State& state, const PrStage1SaveUi19148ActionList& actions,
             uint64_t vblank, Consume&& consume) {
    if (!state.pending || state.blocked) return !state.pending;
    if (actions.truncated || actions.count > 96u) {
        state.blocked = true;
        return false;
    }
    while (vblank >= state.nextVblank) {
        if (state.actionIndex == actions.count) {
            state.pending = false;
            return true;
        }
        const auto& action = actions.actions[state.actionIndex];
        if (IsFlash(action) &&
            action.arg3 != kSaveUiPromptFlashFrameCount80017E6C) {
            state.blocked = true;
            return false;
        }
        if (!consume(action, state.nextVblank)) {
            state.blocked = true;
            return false;
        }
        if (IsFlash(action)) {
            ++state.presentations;
            ++state.nextVblank;
            if (++state.flashFrames == kSaveUiPromptFlashFrameCount80017E6C) {
                state.flashFrames = 0;
                ++state.actionIndex;
            }
        } else {
            ++state.actionIndex;
            if (action.kind == PrStage1SaveUi19148ActionKind::Call8001EA00EndFrame) {
                ++state.presentations;
                ++state.nextVblank;
            }
        }
    }
    return false;
}

} // namespace PrStage1SavePresentationDirect
