#pragma once

#include <cstdint>

using PrTicks = uint64_t;

struct PrTimer {
    static void Init();
    static PrTicks Tell();
    static void Seek(PrTicks ticks);
    static void Pause();
    static void Resume();
};
