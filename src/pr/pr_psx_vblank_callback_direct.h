#pragma once

#include <array>
#include <cstdint>

namespace PrPsxVblankCallbackDirect {

// SCUS 80035E54 clears eight words at80057014;800357D4 delegates to
// 80035F24(0,newCallback). Host bindings project code addresses only, not
// another callback table or another VBlank clock.
constexpr uint32_t kLoadingCallback8001537C = 0x8001537Cu;
struct Slot {
    uint32_t address = 0;
    void (*invoke)(void*) = nullptr;
    void* user = nullptr;
};
struct State {
    std::array<Slot, 8> slots{};
    uint64_t exchanges = 0;
    uint64_t invoked = 0;
    uint64_t missingBindings = 0;
};
inline State& ProcessSlots80057014() {
    static State state{};
    return state;
}
struct ExchangeResult {
    bool known = false;
    uint32_t previous = 0;
};
inline ExchangeResult Exchange80035F24(State& state, uint32_t index, Slot replacement) {
    if (index >= state.slots.size()) return {};
    const uint32_t previous = state.slots[index].address;
    if (!replacement.address) replacement = {};
    // A same-address host binding can change when a different translated
    // owner projects the same native global Loading data; the PSX word does
    // not change. Never return the new value in place of the old callback.
    state.slots[index] = replacement;
    ++state.exchanges;
    return {true, previous};
}
inline ExchangeResult VSyncCallback800357D4(Slot replacement) {
    return Exchange80035F24(ProcessSlots80057014(), 0, replacement);
}
inline bool InvokeSlot(State& state, uint32_t index) {
    if (index >= state.slots.size()) return false;
    const Slot slot = state.slots[index];
    if (!slot.address) return false;
    if (!slot.invoke) {
        ++state.missingBindings;
        return false;
    }
    ++state.invoked;
    slot.invoke(slot.user);
    return true;
}
inline void Dispatch80035EAC(State& state) {
    // The native loop loads each slot immediately before jalr. An earlier
    // callback can replace/clear a later slot in this very same interrupt.
    for (uint32_t i = 0; i < state.slots.size(); ++i) InvokeSlot(state, i);
}
inline bool InvokeLoadingOwner(void* owner) {
    auto& state = ProcessSlots80057014();
    if (state.slots[0].address != kLoadingCallback8001537C ||
        state.slots[0].user != owner) return false;
    return InvokeSlot(state, 0);
}
inline void ReleaseLoadingOwner(void* owner) {
    auto& state = ProcessSlots80057014();
    // Disposing a host mirror is not permission to erase another live owner.
    if (state.slots[0].address == kLoadingCallback8001537C &&
        state.slots[0].user == owner) VSyncCallback800357D4({});
}

} // namespace PrPsxVblankCallbackDirect
