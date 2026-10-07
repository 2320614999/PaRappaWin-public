#pragma once
#include <cstdint>

namespace PrSS0CardReadCallbackDirect {

constexpr uint32_t kCallback80017F38 = 0x80017F38u;
struct State80017F38 {
    int32_t gp736 = 0;
    int32_t gp740 = 0;
    int32_t returnValue = 0;
    uint32_t invocations = 0;
    uint32_t prepares = 0;
    uint32_t ends = 0;
};

// LiveSCUS80017F38/80017EE8: noVSync call here. On0 toggle the
// gp132-target's first word; on0/1 alternate prepare3/end0. On19 reset
// the counter to0 but RETURN20. Other ticks do not prepare or submit.
template<class Prepare, class End>
bool Tick80017F38(State80017F38& state, int32_t& contextWord0,
                 Prepare prepare3, End end0) {
    ++state.invocations;
    const int32_t before = state.gp740;
    if (before == 0) contextWord0 = contextWord0 != 1;
    if (before == 0 || before == 1) {
        if (state.gp736) {
            if (!end0()) return false;
            state.gp736 = 0;
            ++state.ends;
        } else {
            if (!prepare3()) return false;
            ++state.gp736;
            ++state.prepares;
        }
    }
    state.returnValue = state.gp740 + 1;
    state.gp740 = before == 19 ? 0 : state.returnValue;
    return true;
}

} // namespace PrSS0CardReadCallbackDirect
