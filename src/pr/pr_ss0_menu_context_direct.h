#pragma once

#include "pr_ss0_directory_dispatcher_direct.h"
#include <array>
#include <cstdint>

// Mutable main-program storage, not an immutable template: 80026784 returns
// its address, while 800264AC/80026794/80026720 update its contents.
namespace PrSS0MenuContextDirect {
inline constexpr std::array<uint8_t, 36> kInitial800544F8 = {
    0,0,0,0, 0,0,0,0, 0,0,0,0, 3,0,5,0,
    0,0,2,0, 0,0,1,0, 0,0,2,0, 0,0,4,0, 0,0,1,0};
struct Context800544F8 {
    bool known = true;
    uint64_t generation = 0;
    std::array<uint8_t, 36> bytes = kInitial800544F8;
};
inline Context800544F8 g_context800544F8{};

inline void ResetColdBoot800544F8() { g_context800544F8 = {}; }
inline const Context800544F8& Get80026784() { return g_context800544F8; }

inline bool Publish800264AC(
    const PrSS0DirectoryDispatcherDirect::MainMenuState800264AC& state,
    int32_t blink) {
    auto next = g_context800544F8;
    auto put16 = [&](std::size_t offset, int32_t value) {
        if (value < -32768 || value > 32767) return false;
        const auto word = static_cast<uint16_t>(value);
        next.bytes[offset] = static_cast<uint8_t>(word);
        next.bytes[offset + 1] = static_cast<uint8_t>(word >> 8);
        return true;
    };
    if ((blink != 0 && blink != 1) || !put16(12, state.cursor) || !put16(14, state.count)) {
        g_context800544F8.known = false;
        return false;
    }
    for (std::size_t i = 0; i < 5; ++i) {
        if (!put16(16 + 4 * i, state.itemValue[i])) {
            g_context800544F8.known = false;
            return false;
        }
    }
    // 80026720/80025D70 writes ctx+0; EXIT sets ctx+4. The ctx+8 word
    // and all per-item count words remain the original live storage bytes.
    next.bytes[0] = static_cast<uint8_t>(blink);
    next.bytes[4] = state.doneFlag ? 1u : 0u;
    next.known = true;
    ++next.generation;
    g_context800544F8 = next;
    return true;
}
} // namespace PrSS0MenuContextDirect
