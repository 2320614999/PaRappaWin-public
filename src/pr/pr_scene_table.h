#pragma once

#include <array>
#include <cstddef>

#include "pr_scene_def.h"

struct PrSceneTable {
    std::array<PrSceneDef, kPrSceneCount> scenes{};

    const PrSceneDef& Get(PrSceneId id) const { return scenes[static_cast<size_t>(id)]; }
    PrSceneDef& Get(PrSceneId id) { return scenes[static_cast<size_t>(id)]; }
};
