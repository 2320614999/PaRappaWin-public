#include "pr_timer.h"

#include <windows.h>

namespace {

static LARGE_INTEGER g_freq{};
static LARGE_INTEGER g_baseQpc{};
static PrTicks g_baseTicks = 0;
static bool g_paused = false;
static PrTicks g_pausedTicks = 0;

static LARGE_INTEGER NowQpc() {
    LARGE_INTEGER v{};
    QueryPerformanceCounter(&v);
    return v;
}

static PrTicks QpcDeltaToTicks(LONGLONG delta) {
    if (g_freq.QuadPart <= 0) {
        return 0;
    }
    long double us = (long double)delta * 1000000.0L / (long double)g_freq.QuadPart;
    if (us < 0.0L) us = 0.0L;
    return (PrTicks)us;
}

} // namespace

void PrTimer::Init() {
    QueryPerformanceFrequency(&g_freq);
    g_baseQpc = NowQpc();
    g_baseTicks = 0;
    g_paused = false;
    g_pausedTicks = 0;
}

PrTicks PrTimer::Tell() {
    if (g_paused) {
        return g_pausedTicks;
    }

    const LARGE_INTEGER now = NowQpc();
    const LONGLONG delta = now.QuadPart - g_baseQpc.QuadPart;
    return g_baseTicks + QpcDeltaToTicks(delta);
}

void PrTimer::Seek(PrTicks ticks) {
    if (g_paused) {
        g_pausedTicks = ticks;
        return;
    }

    g_baseTicks = ticks;
    g_baseQpc = NowQpc();
}

void PrTimer::Pause() {
    if (g_paused) {
        return;
    }
    g_pausedTicks = Tell();
    g_paused = true;
}

void PrTimer::Resume() {
    if (!g_paused) {
        return;
    }
    g_paused = false;
    g_baseTicks = g_pausedTicks;
    g_baseQpc = NowQpc();
}
