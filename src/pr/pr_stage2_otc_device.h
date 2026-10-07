#pragma once
#include "pr_stage2_lifecycle_direct.h"
#include "pr_stage2_interrupt_device.h"
#include <cstdint>

namespace PrStage2OtcDevice {
// Host-side owner of channel 6's bounded, contiguous reverse-OT transfer.
// Reading registers never progresses DMA. The serializing scene owner must
// call Complete on a separate work event while the native caller is retained.
// This is not a CPU/DMA timing emulator or a GPU rendering implementation.
class Device final {
public:
    bool TryRead(uint32_t address, uint32_t width, uint32_t& value) const;
    bool TryWrite(uint32_t address, uint32_t width, uint32_t value);
    bool Pending() const noexcept { return (control_ & 0x11000000u) == 0x11000000u; }
    bool Faulted() const noexcept { return faulted_; }
    uint64_t Serial() const noexcept { return serial_; }
    uint32_t Address() const noexcept { return address_; }
    uint32_t Count() const noexcept { return count_; }
    uint32_t Control() const noexcept { return control_; }
    uint64_t CompletedTransfers() const noexcept { return completions_; }
    uint64_t WrittenWords() const noexcept { return writtenWords_; }
    // False while DPCR disables channel 6. No mutation in that case. A stale
    // serial or unsupported transfer fails before any RAM/interrupt effect.
    bool Complete(uint64_t serial, PrStage2LifecycleDirect::Services& memory,
                  PrStage2InterruptDevice::Controller& interrupts);
private:
    uint32_t address_ = 0u, count_ = 0u, control_ = 2u;
    uint64_t serial_ = 0u, completions_ = 0u, writtenWords_ = 0u;
    bool faulted_ = false, consuming_ = false;
    static uint32_t Register(uint32_t address, uint32_t width);
};
}
