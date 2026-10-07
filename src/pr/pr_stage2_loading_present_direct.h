#pragma once
#include "pr_stage2_lifecycle_direct.h"

namespace PrStage2LoadingPresentDirect {
using Services = PrStage2LifecycleDirect::Services;

// Original shared Loading prepare/present owners. GPU work, ordering tables,
// pattern state and audio remain in the same Services instance as InitScene.
// Unbound lower devices/draw calls still fail; there is no success fallback.
int32_t Prepare8001EA74(Services& s, uint32_t drawPattern, uint32_t mode);
int32_t Present8001EBF4(Services& s, uint32_t unused);
int32_t ClearCurrentFramebuffer80040420(Services& s, uint32_t red,
                                      uint32_t green, uint32_t blue);
bool TryCall(Services& s, uint32_t function,
             std::initializer_list<uint32_t> args, int32_t& result);
}