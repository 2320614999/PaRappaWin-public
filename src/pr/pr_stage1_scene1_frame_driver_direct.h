#pragma once

#include "pr_stage1_scene1_movie1_direct.h"

namespace PrStage1Scene1FrameDriverDirect {

static constexpr int kSceneResultContinue = 1;

struct IntroTransitionState {
    bool active = false;
    bool renderActive = false;
    bool consumerResetPending = false;
    int frame = 0;
};

struct LoopFrameWindow {
    bool introTransitionActive = false;
    bool resetIntroConsumer = false;
};

inline void TickIntroTransition(IntroTransitionState& intro, int maxLife) {
    if (!intro.active) {
        return;
    }

    if (intro.frame < 0) {
        intro.frame = 0;
    } else {
        ++intro.frame;
    }
    if (intro.frame >= maxLife) {
        intro.active = false;
        intro.consumerResetPending = true;
        intro.frame = maxLife;
    }
}

}  // namespace PrStage1Scene1FrameDriverDirect
