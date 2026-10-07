#pragma once
#include <cstdint>
#include <limits>
#include "pr_psx_vsync_direct.h"

namespace PrStage1TerminalPresentationDirect {
// COMOD1 801C7A60: four RenderFrame(7)/VSync(2)/PresentFrame iterations,
// then SCUS8001B120(1). Rendering a page does not make it eligible for Present:
// first consume the shared 80035560(2) wait from the absolute host VBlank clock.
// Redrawing the old front while suspended must not acknowledge the new page.
struct State {
    uint8_t frames = 0;
    bool pending = false;
    uint64_t presentBefore = 0;
    uint8_t renderedPage = 0;
    bool pageKnown[2]{};
    bool copied = false;
    uint8_t copiedPage = 0;
    bool waitComplete = false;
    bool published = false;
    uint64_t hostVblankAtSubmit = 0;
    uint64_t lastHostVblank = 0;
    uint32_t waitRequest = 0;
};
inline bool Submit(State& state, uint8_t renderedPage, uint64_t hostVblank,
                   bool copyQueued, bool renderOnly,
                   PrPsxVSyncDirect::PsxVSyncState80035560& clock) {
    if (renderOnly || !copyQueued || renderedPage > 1 || state.pending || state.frames >= 4)
        return false;
    state.pending = true;
    state.renderedPage = renderedPage;
    state.waitComplete = state.published = false;
    state.hostVblankAtSubmit = state.lastHostVblank = hostVblank;
    PrPsxVSyncDirect::BeginVSync80035560(clock, 2);
    state.waitRequest = clock.requestCount;
    return true;
}
inline bool AdvanceWait(State& state,
                        PrPsxVSyncDirect::PsxVSyncState80035560& clock,
                        uint64_t hostVblank,
                        bool clockAlreadyDelivered = false) {
    // Do not consume another caller's wait if ownership was unexpectedly lost.
    if (state.pending && !state.waitComplete &&
        (clock.requestCount != state.waitRequest || clock.lastArg != 2)) return false;
    if (!state.pending && clock.waitActive) return false;
    if (hostVblank <= state.lastHostVblank) return state.pending && state.waitComplete;
    const uint64_t elapsed = hostVblank - state.lastHostVblank;
    state.lastHostVblank = hostVblank;
    const auto ticks = elapsed > static_cast<uint64_t>((std::numeric_limits<int32_t>::max)())
        ? (std::numeric_limits<int32_t>::max)() : static_cast<int32_t>(elapsed);
    const auto wait = clockAlreadyDelivered ? clock.lastResult :
        PrPsxVSyncDirect::AdvanceHostVblankClock80035EAC(clock, ticks);
    if (state.pending)
        state.waitComplete = wait.softwareWaitComplete && wait.softwareStateCommitted;
    return state.pending && state.waitComplete;
}
inline bool Publish(State& state, uint64_t presentBefore) {
    if (!state.pending || !state.waitComplete || state.published) return false;
    state.presentBefore = presentBefore;
    state.published = true;
    return true;
}
inline bool Poll(State& state, uint64_t successfulPresents) {
    if (!state.pending || !state.published || successfulPresents <= state.presentBefore) return false;
    state.pending = false;
    state.pageKnown[state.renderedPage] = true;
    ++state.frames;
    return true;
}
inline bool CanMove(const State& state, uint32_t activeDrawPage) {
    return activeDrawPage < 2 && state.frames == 4 && !state.pending &&
        state.pageKnown[0] && state.pageKnown[1];
}
// 8001B120(1): source is the opposite of the current draw page, not itself.
inline uint32_t SourcePage(uint32_t activeDrawPage) { return activeDrawPage ^ 1u; }
}
