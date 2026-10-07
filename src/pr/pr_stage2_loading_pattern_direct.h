#pragma once
#include "pr_stage2_lifecycle_direct.h"

namespace PrStage2LoadingPatternDirect {
using Memory = PrStage2LifecycleDirect::Services;
// Explicit output boundary. These calls discard the original drawing return
// values; a host sink need not invent PSX packet pointers or scalar receipts.
struct DrawSink {
    virtual ~DrawSink() = default;
    virtual void Box(uint32_t x,uint32_t y,uint32_t width,uint32_t height,
                     uint32_t color,uint32_t priority) = 0;
    virtual void Sprite(uint32_t x,uint32_t y,uint32_t source,uint32_t priority) = 0;
};
// Full 8001EF40 state/control translation, not a copy of Stage1's runtime.
// The overload without a sink executes the two original lower call identities.
int32_t DrawPattern8001EF40(Memory& memory,uint32_t mode,uint32_t priority);
int32_t DrawPattern8001EF40(Memory& memory,DrawSink& sink,
                            uint32_t mode,uint32_t priority);
}
