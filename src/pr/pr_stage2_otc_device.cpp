#include "pr_stage2_otc_device.h"
#include <limits>
#include <stdexcept>

namespace PrStage2OtcDevice {
uint32_t Device::Register(uint32_t address, uint32_t width) {
    // Accept physical, KSEG0 and KSEG1 aliases; no accidental KSEG2 mapping.
    if (address >= 0x20000000u && (address < 0x80000000u || address >= 0xC0000000u)) return 0u;
    const uint32_t physical = address & 0x1FFFFFFFu;
    if (physical < 0x1F8010E0u || physical >= 0x1F8010ECu) return 0u;
    if (width != 4u || (physical & 3u))
        throw std::invalid_argument("S2 OTC requires an aligned 32-bit bus transaction");
    return physical;
}
bool Device::TryRead(uint32_t address, uint32_t width, uint32_t& value) const {
    const uint32_t reg = Register(address, width);
    if (!reg) return false;
    if (reg == 0x1F8010E0u) value = address_;
    else if (reg == 0x1F8010E4u) value = count_;
    else value = control_;
    return true;
}
bool Device::TryWrite(uint32_t address, uint32_t width, uint32_t value) {
    const uint32_t reg = Register(address, width);
    if (!reg) return false;
    if (consuming_) throw std::logic_error("Reentrant OTC register write during transfer");
    if (reg != 0x1F8010E8u) {
        if (control_ & 0x01000000u)
            throw std::logic_error("Changing an active OTC descriptor is not supported");
        if (reg == 0x1F8010E0u) address_ = value & 0x00FFFFFFu;
        else count_ = value;
        return true;
    }
    const uint32_t next = (value & 0x51000000u) | 2u;
    if ((control_ & 0x01000000u) && (next & 0x01000000u))
        throw std::logic_error("Overwriting an active OTC request is not supported");
    if ((next & 0x11000000u) == 0x11000000u) {
        if (serial_ == (std::numeric_limits<uint64_t>::max)())
            throw std::overflow_error("OTC request serial exhausted");
        ++serial_;
    }
    control_ = next;
    // Writing CHCR with busy cleared is an explicit source abort/reset.
    if (!(next & 0x01000000u)) faulted_ = false;
    return true;
}
bool Device::Complete(uint64_t serial, PrStage2LifecycleDirect::Services& memory,
                      PrStage2InterruptDevice::Controller& interrupts) {
    if (consuming_ || faulted_ || !Pending() || serial != serial_)
        throw std::logic_error("Invalid, failed, reentrant or stale OTC completion");
    uint32_t priority = 0u;
    if (!interrupts.TryRead(0x1F8010F0u, 4u, priority))
        throw std::logic_error("Missing DMA priority register");
    if (!(priority & 0x08000000u)) return false;
    const uint32_t words = (count_ & 0xFFFFu) ? (count_ & 0xFFFFu) : 0x10000u;
    // Only the source's bounded non-wrapping OT buffers are supported here.
    // Do not approximate out-of-RAM or word-misaligned hardware transactions.
    if ((address_ & 3u) || address_ > 0x001FFFFCu || 4u * (words - 1u) > address_)
        throw std::out_of_range("Unsupported non-contiguous OTC RAM transfer");
    consuming_ = true;
    control_ &= ~0x10000000u; // Trigger clears on BEGIN, busy only on completion.
    try {
        for (uint32_t i = 0u; i < words; ++i) {
            const uint32_t at = address_ - 4u * i;
            const uint32_t link = i + 1u == words ? 0x00FFFFFFu : at - 4u;
            memory.Write32(0x80000000u | at, link);
            ++writtenWords_;
        }
        // SyncMode 0 without chopping leaves MADR and BCR unchanged.
        control_ &= ~0x01000000u;
        interrupts.NotifyDma(6u, PrStage2InterruptDevice::DmaBoundary::TransferComplete);
        ++completions_;
        consuming_ = false;
    } catch (...) {
        faulted_ = true;
        consuming_ = false;
        throw; // Partial RAM writes are real; do not acknowledge or retry them.
    }
    return true;
}
}
