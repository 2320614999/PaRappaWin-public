#pragma once

#include <cstdint>

enum class PrSceneId : uint8_t {
    Scene0 = 0,
    Scene1 = 1,
    Scene2 = 2,
    Scene3 = 3,
    Scene5 = 4,
    Scene6 = 5,
    Scene7 = 6,
    Scene8 = 7,
    Scene9 = 8,
};

constexpr uint32_t kPrSceneCount = 9;
