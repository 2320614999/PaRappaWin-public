#include "pr_stage2_irq_direct.h"
#include "pr_stage2_gpu_direct.h"
#include <stdexcept>

namespace PrStage2IrqDirect {
namespace {
int32_t S(uint32_t v) {
    return v <= 0x7FFFFFFFu ? static_cast<int32_t>(v)
        : static_cast<int32_t>(static_cast<int64_t>(v) - 0x100000000LL);
}
uint32_t Bit(uint32_t shift) { return 1u << (shift & 31u); }
}

int32_t SetInterruptMask800358C0(Services& s, uint32_t mask) {
    const uint32_t address = s.Read32(0x80057008u);
    const uint16_t previous = s.Read16(address);
    s.Write16(address, static_cast<uint16_t>(mask));
    return previous; // LHU returns zero-extended, not a signed mask or BOOL.
}

int32_t ClearCallbackWords800361F8(Services& s, uint32_t destination, uint32_t count) {
    while (count != 0u) {
        s.Write32(destination, 0u);
        --count;
        destination += 4u;
    }
    return -1;
}

int32_t SetInterruptCallback80035BA0(Services& s, uint32_t channel, uint32_t callback) {
    const uint32_t slot = 0x80055F7Cu + (channel << 2u);
    const uint32_t previous = s.Read32(slot);
    if (previous == callback || s.Read16(0x80055F78u) == 0u) return S(previous);
    const uint32_t maskAddress = s.Read32(0x80057008u);
    uint32_t mask = s.Read16(maskAddress);
    s.Write16(maskAddress, 0u);
    const uint32_t bit = Bit(channel);
    s.Write32(slot, callback);
    const uint16_t registered = s.Read16(0x80055FA8u);
    if (callback != 0u) {
        mask |= bit;
        s.Write16(0x80055FA8u, static_cast<uint16_t>(registered | bit));
    } else {
        mask &= ~bit;
        s.Write16(0x80055FA8u, static_cast<uint16_t>(registered & ~bit));
    }
    const uint32_t clear = callback == 0u ? 1u : 0u;
    if (channel == 0u) {
        s.Call(0x80048AE0u, {clear});
        s.Call(0x80048AF0u, {3u, clear});
    }
    if (channel == 4u) s.Call(0x80048AF0u, {0u, clear});
    if (channel == 5u) s.Call(0x80048AF0u, {1u, clear});
    if (channel == 6u) s.Call(0x80048AF0u, {2u, clear});
    // A BIOS callback may replace the register pointer. The original reloads it.
    s.Write16(s.Read32(0x80057008u), static_cast<uint16_t>(mask));
    return S(previous);
}

int32_t InterruptCallback80035774(Services& s, uint32_t channel, uint32_t callback) {
    const uint32_t table = s.Read32(0x80057000u);
    const uint32_t target = s.Read32(table + 8u);
    if (target == 0x80035BA0u) return SetInterruptCallback80035BA0(s, channel, callback);
    return s.Call(target, {channel, callback});
}

int32_t SetDmaCallback80036150(Services& s, uint32_t channel, uint32_t callback) {
    const uint32_t slot = 0x80057040u + (channel << 2u);
    const uint32_t previous = s.Read32(slot);
    if (previous == callback) return S(previous);
    const uint32_t control = s.Read32(0x8005703Cu);
    s.Write32(slot, callback);
    const uint32_t oldControl = s.Read32(control);
    const uint32_t bit = Bit(channel + 16u);
    const uint32_t enabled = (oldControl & 0x00FFFFFFu) | 0x00800000u;
    s.Write32(control, callback != 0u ? enabled | bit : enabled & ~bit);
    return S(previous);
}

int32_t DmaCallback800357A4(Services& s, uint32_t channel, uint32_t callback) {
    const uint32_t table = s.Read32(0x80057000u);
    const uint32_t target = s.Read32(table + 4u);
    if (target == 0x80036150u) return SetDmaCallback80036150(s, channel, callback);
    return s.Call(target, {channel, callback});
}

int32_t InitDmaCallbacks80035F7C(Services& s) {
    ClearCallbackWords800361F8(s, 0x80057040u, 8u);
    s.Write32(s.Read32(0x8005703Cu), 0u);
    InterruptCallback80035774(s, 3u, 0x80035FCCu);
    return S(0x80036150u);
}

int32_t DispatchDmaInterrupt80035FCC(Services& s) {
    uint32_t pending = (s.Read32(s.Read32(0x8005703Cu)) >> 24u) & 0x7Fu;
    while (pending != 0u) {
        uint32_t channel = 0u;
        do {
            if (channel >= 7u) break;
            if ((pending & 1u) != 0u) {
                const uint32_t control = s.Read32(0x8005703Cu);
                const uint32_t status = s.Read32(control);
                s.Write32(control, status & (Bit(channel + 24u) | 0x00FFFFFFu));
                const uint32_t callback = s.Read32(0x80057040u + 4u * channel);
                if (callback == 0x80046BC4u) PrStage2GpuDirect::DrainQueue80046BC4(s);
                else if (callback != 0u) s.CallVoid(callback, {});
            }
            pending >>= 1u;
            ++channel;
        } while (pending != 0u);
        // Re-read after callbacks; newly pending channels require another pass.
        pending = (s.Read32(s.Read32(0x8005703Cu)) >> 24u) & 0x7Fu;
    }
    const uint32_t control = s.Read32(0x8005703Cu);
    bool busError = (s.Read32(control) & 0xFF000000u) == 0x80000000u;
    if (!busError) busError = (s.Read32(control) & 0x8000u) != 0u;
    if (busError) {
        s.Call(0x80047FFCu, {0x80011BA0u, s.Read32(control)});
        for (uint32_t channel = 0u; channel < 7u; ++channel) {
            const uint32_t base = s.Read32(0x80057060u);
            s.Call(0x80047FFCu, {0x80011BBCu, channel, s.Read32(base + 16u * channel)});
        }
    }
    return 0;
}

bool TryDispatch(Services& s, uint32_t function,
                 std::initializer_list<uint32_t> arguments, int32_t& result) {
    size_t arity;
    switch (function) {
    case 0x800358C0u: arity = 1; break;
    case 0x800361F8u: case 0x80035BA0u: case 0x80035774u:
    case 0x80036150u: case 0x800357A4u: arity = 2; break;
    case 0x80035F7Cu: case 0x80035FCCu: arity = 0; break;
    default: return false;
    }
    if (arguments.size() != arity) throw std::invalid_argument("SCUS IRQ argument count mismatch");
    const auto a = arguments.begin();
    switch (function) {
    case 0x800358C0u: result = SetInterruptMask800358C0(s, a[0]); break;
    case 0x800361F8u: result = ClearCallbackWords800361F8(s, a[0], a[1]); break;
    case 0x80035BA0u: result = SetInterruptCallback80035BA0(s, a[0], a[1]); break;
    case 0x80035774u: result = InterruptCallback80035774(s, a[0], a[1]); break;
    case 0x80036150u: result = SetDmaCallback80036150(s, a[0], a[1]); break;
    case 0x800357A4u: result = DmaCallback800357A4(s, a[0], a[1]); break;
    case 0x80035F7Cu: result = InitDmaCallbacks80035F7C(s); break;
    case 0x80035FCCu: result = DispatchDmaInterrupt80035FCC(s); break;
    }
    return true;
}
}