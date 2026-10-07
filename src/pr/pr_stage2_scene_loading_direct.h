#pragma once
#include "pr_stage2_lifecycle_direct.h"

namespace PrStage2SceneLoadingDirect {
using Services = PrStage2LifecycleDirect::Services;
// Loading-screen ownership from the original SCUS callers of S2 InitScene.
// The platform still supplies real VBlank events; these functions never skip
// waits, acknowledge an unread file, or fabricate a successful asset load.
int32_t LoadingFrame8001537C(Services& s);
int32_t BeginLoading80015408(Services& s, uint32_t mode);
int32_t EndLoading8001545C(Services& s);
int32_t LoadSaveResources80015590(Services& s,uint32_t scene);
int32_t LoadSceneResources80015660(Services& s, uint32_t scene,
                                uint32_t mode, uint32_t loadingSound);
bool TryCall(Services& s, uint32_t function,
             std::initializer_list<uint32_t> args, int32_t& result);
}
