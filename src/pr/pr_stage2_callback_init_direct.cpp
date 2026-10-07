#include "pr_stage2_callback_init_direct.h"
#include "pr_stage2_irq_direct.h"

namespace PrStage2CallbackInitDirect {
namespace {
int32_t S(uint32_t v) {
    return v <= 0x7FFFFFFFu ? static_cast<int32_t>(v)
        : static_cast<int32_t>(static_cast<int64_t>(v) - 0x100000000LL);
}
int32_t Clear(Services& s, uint32_t destination, uint32_t count) {
    while (count != 0u) {
        s.Write32(destination, 0u);
        --count;
        destination += 4u;
    }
    return -1;
}
uint32_t Pending(Services& s, uint32_t statusAddress) {
    const uint16_t registered = s.Read16(0x80055FA8u);
    const uint32_t maskAddress = s.Read32(0x80057008u);
    const uint16_t status = s.Read16(statusAddress);
    const uint16_t mask = s.Read16(maskAddress);
    return status & registered & mask;
}
}

int32_t ClearCallbackState80035E28(Services& s, uint32_t destination, uint32_t count) {
    return Clear(s, destination, count);
}
int32_t ClearVBlankWords80035F50(Services& s, uint32_t destination, uint32_t count) {
    return Clear(s, destination, count);
}
int32_t ResetCallback80035744(Services& s) {
    const uint32_t table = s.Read32(0x80057000u);
    const uint32_t target = s.Read32(table + 12u);
    if (target == 0x800358DCu) return InitCallbacks800358DC(s);
    return s.Call(target, {});
}
int32_t VSyncCallback800357D4(Services& s, uint32_t callback) {
    const uint32_t table = s.Read32(0x80057000u);
    const uint32_t target = s.Read32(table + 20u);
    if (target == 0x80035F24u) return SetVBlankCallback80035F24(s, 0u, callback);
    return s.Call(target, {0u, callback});
}
int32_t InitCallbacks800358DC(Services& s) {
    if (s.Read16(0x80055F78u) != 0u) return 0;
    const uint32_t statusAddress = s.Read32(0x80057004u);
    const uint32_t maskAddress = s.Read32(0x80057008u);
    s.Write16(maskAddress, 0u);
    const uint16_t observedMask = s.Read16(maskAddress);
    s.Write16(statusAddress, observedMask);
    s.Write32(s.Read32(0x8005700Cu), 0x33333333u);
    ClearCallbackState80035E28(s, 0x80055F78u, 0x41Au);
    // The global context address is real SCUS data, not a host stack pointer.
    // A platform binding must implement/unwind the continuation boundary;
    // this translation neither saves native CPU registers nor invents success.
    if (s.Call(0x80047F5Cu, {0x80055FB0u}) != 0) DispatchInterrupts800359B8(s);
    s.Write32(0x80055FB4u, 0x80056F90u); // Original HookEntryInt delay-slot write.
    s.Call(0x80048A30u, {0x80055FB0u});
    s.Write16(0x80055F78u, 1u);
    const uint32_t vblank = static_cast<uint32_t>(InitVBlankCallbacks80035E54(s));
    s.Write32(s.Read32(0x80057000u) + 20u, vblank);
    const uint32_t dma = static_cast<uint32_t>(PrStage2IrqDirect::InitDmaCallbacks80035F7C(s));
    s.Write32(s.Read32(0x80057000u) + 4u, dma);
    s.Call(0x80048960u, {});
    s.Call(0x80048A50u, {});
    return S(0x80055F78u);
}
int32_t InitVBlankCallbacks80035E54(Services& s) {
    s.Write32(s.Read32(0x80057038u), 0x107u);
    s.Write32(0x80057034u, 0u);
    ClearVBlankWords80035F50(s, 0x80057014u, 8u);
    PrStage2IrqDirect::InterruptCallback80035774(s, 0u, 0x80035EACu);
    return S(0x80035F24u);
}
int32_t DispatchVBlank80035EAC(Services& s) {
    const uint32_t count = s.Read32(0x80057034u);
    s.Write32(0x80057034u, count + 1u);
    (void)s.Read32(0x80057034u); // Preserve the original post-write read.
    for (uint32_t channel = 0u; channel < 8u; ++channel) {
        const uint32_t callback = s.Read32(0x80057014u + channel * 4u);
        if (callback != 0u) s.Call(callback, {});
    }
    return 0;
}

int32_t SetVBlankCallback80035F24(Services& s, uint32_t channel, uint32_t callback) {
    const uint32_t slot = 0x80057014u + (channel << 2u);
    const uint32_t previous = s.Read32(slot);
    if (previous != callback) s.Write32(slot, callback);
    return S(previous);
}
int32_t DispatchInterrupts800359B8(Services& s) {
    if (s.Read16(0x80055F78u) == 0u) {
        const uint16_t status = s.Read16(s.Read32(0x80057004u));
        s.Call(0x80047FFCu, {0x80011B68u, status});
        s.Call(0x80048A10u, {});
        // Normal BIOS exception-return does not return here. Keep the original
        // continuation for explicit alternate bindings; no host longjmp.
    }
    const uint32_t firstStatusAddress = s.Read32(0x80057004u);
    s.Write16(0x80055F7Au, 1u);
    uint32_t pending = Pending(s, firstStatusAddress);
    while (pending != 0u) {
        for (uint32_t channel = 0u; channel < 11u && pending != 0u; ++channel) {
            if ((pending & 1u) != 0u) {
                const uint32_t statusAddress = s.Read32(0x80057004u);
                s.Write16(statusAddress, static_cast<uint16_t>(~(1u << channel)));
                const uint32_t callback = s.Read32(0x80055F7Cu + 4u * channel);
                if (callback != 0u) s.Call(callback, {});
            }
            pending >>= 1u;
        }
        // Reload pointers, enabled callbacks and status after every pass.
        pending = Pending(s, s.Read32(0x80057004u));
    }
    const uint32_t statusAddress = s.Read32(0x80057004u);
    const uint32_t maskAddress = s.Read32(0x80057008u);
    const uint16_t status = s.Read16(statusAddress);
    const uint16_t mask = s.Read16(maskAddress);
    if ((status & mask) != 0u) {
        const uint32_t count = s.Read32(0x80057010u);
        s.Write32(0x80057010u, count + 1u);
        if (S(count) >= 0x801) {
            const uint16_t freshStatus = s.Read16(statusAddress);
            const uint16_t freshMask = s.Read16(maskAddress);
            s.Call(0x80047FFCu, {0x80011B84u, freshStatus, freshMask});
            const uint32_t freshAddress = s.Read32(0x80057004u);
            s.Write32(0x80057010u, 0u);
            s.Write16(freshAddress, 0u);
        }
    } else s.Write32(0x80057010u, 0u);
    s.Write16(0x80055F7Au, 0u);
    return s.Call(0x80048A10u, {});
}

bool TryDispatch(Services& s, uint32_t function,
                 std::initializer_list<uint32_t> arguments, int32_t& result) {
    size_t arity;
    switch (function) {
    case 0x80035744u: case 0x800358DCu: case 0x800359B8u:
    case 0x80035E54u: case 0x80035EACu: arity = 0u; break;
    case 0x800357D4u: arity = 1u; break;
    case 0x80035E28u: case 0x80035F24u: case 0x80035F50u: arity = 2u; break;
    default: return false;
    }
    if (arguments.size() != arity) throw std::invalid_argument("SCUS callback argument count mismatch");
    const auto a = arguments.begin();
    switch (function) {
    case 0x80035744u: result = ResetCallback80035744(s); break;
    case 0x800357D4u: result = VSyncCallback800357D4(s, a[0]); break;
    case 0x800358DCu: result = InitCallbacks800358DC(s); break;
    case 0x800359B8u: result = DispatchInterrupts800359B8(s); break;
    case 0x80035E28u: result = ClearCallbackState80035E28(s, a[0], a[1]); break;
    case 0x80035E54u: result = InitVBlankCallbacks80035E54(s); break;
    case 0x80035EACu: result = DispatchVBlank80035EAC(s); break;
    case 0x80035F24u: result = SetVBlankCallback80035F24(s, a[0], a[1]); break;
    case 0x80035F50u: result = ClearVBlankWords80035F50(s, a[0], a[1]); break;
    }
    return true;
}
}
