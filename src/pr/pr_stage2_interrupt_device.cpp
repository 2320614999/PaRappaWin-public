#include "pr_stage2_interrupt_device.h"
#include <stdexcept>

namespace PrStage2InterruptDevice {
namespace {
constexpr uint32_t IStat = 0x1F801070u;
constexpr uint32_t IMask = 0x1F801074u;
constexpr uint32_t Dpcr = 0x1F8010F0u;
constexpr uint32_t Dicr = 0x1F8010F4u;
constexpr uint32_t IrqBits = 0x7FFu;
constexpr uint32_t DmaWritable = 0x00FF807Fu;
}

uint32_t Controller::Register(uint32_t address) {
    // Only real KUSEG/KSEG0/KSEG1 aliases, not arbitrary top-bit truncation.
    const uint32_t segment = address & 0xE0000000u;
    if (segment != 0u && segment != 0x80000000u && segment != 0xA0000000u) return 0u;
    const uint32_t physical = address & 0x1FFFFFFFu;
    const uint32_t reg = physical & ~3u;
    return reg == IStat || reg == IMask || reg == Dpcr || reg == Dicr ? reg : 0u;
}
void Controller::Validate(uint32_t address, uint32_t reg, uint32_t width, bool writing) {
    const bool irq = reg == IStat || reg == IMask;
    const bool supported = irq ? (width == 2u || (writing && width == 4u)) : width == 4u;
    if ((address & 3u) != 0u || !supported)
        throw std::invalid_argument("Unsupported native IRQ/DMA register transaction");
}
bool Controller::DmaMaster() const noexcept {
    // Channel enables gate NEW flags, not already-latched flags. Disabling a
    // channel must not silently acknowledge an outstanding DMA interrupt.
    return (dmaControl_ & 0x8000u) != 0u ||
           ((dmaControl_ & 0x00800000u) != 0u && dmaFlags_ != 0u);
}
void Controller::Drive(uint32_t source, bool asserted) noexcept {
    const auto bit = static_cast<uint16_t>(1u << source);
    if (asserted) {
        if ((lines_ & bit) == 0u) status_ |= bit;
        lines_ |= bit;
    } else lines_ &= static_cast<uint16_t>(~bit);
}
void Controller::RefreshDmaLine() noexcept { Drive(3u, DmaMaster()); }
uint16_t Controller::Pending() const noexcept { return status_ & mask_; }

bool Controller::TryRead(uint32_t address, uint32_t width, uint32_t& result) const {
    const uint32_t reg = Register(address);
    if (reg == 0u) return false;
    Validate(address, reg, width, false);
    switch (reg) {
    case IStat: result = status_; break;
    case IMask: result = mask_; break;
    case Dpcr: result = dpcr_; break;
    case Dicr: result = dmaControl_ | (dmaFlags_ << 24u) | (DmaMaster() ? 0x80000000u : 0u); break;
    }
    return true;
}
bool Controller::TryWrite(uint32_t address, uint32_t width, uint32_t value) {
    const uint32_t reg = Register(address);
    if (reg == 0u) return false;
    Validate(address, reg, width, true);
    switch (reg) {
    case IStat: status_ &= static_cast<uint16_t>(value & IrqBits); break;
    case IMask: mask_ = static_cast<uint16_t>(value & IrqBits); break;
    case Dpcr: dpcr_ = value; break;
    case Dicr:
        dmaControl_ = value & DmaWritable;
        dmaFlags_ &= ~((value >> 24u) & 0x7Fu);
        RefreshDmaLine();
        break;
    }
    return true;
}
void Controller::SetLine(Source source, bool asserted) {
    const auto index = static_cast<uint32_t>(source);
    if (index > 10u || index == 3u)
        throw std::invalid_argument("Invalid or internally owned native IRQ source");
    Drive(index, asserted);
}
void Controller::NotifyDma(uint32_t channel, DmaBoundary boundary) {
    if (channel >= 7u || (boundary != DmaBoundary::TransferComplete && boundary != DmaBoundary::BlockComplete))
        throw std::invalid_argument("Invalid native DMA completion notification");
    const uint32_t bit = 1u << channel;
    const bool enabled = (dmaControl_ & 0x00800000u) != 0u && (dmaControl_ & (bit << 16u)) != 0u;
    const bool eligible = boundary == DmaBoundary::TransferComplete || (dmaControl_ & bit) != 0u;
    if (enabled && eligible) dmaFlags_ |= bit;
    RefreshDmaLine();
}
void Controller::NotifyDmaBusError() {
    dmaControl_ |= 0x8000u;
    RefreshDmaLine();
}
}
