#pragma once
#include <cstdint>

namespace PrStage2InterruptDevice {

// Concrete event/register state for the native port. This is not a CPU, DMA
// transfer engine, BIOS, clock, or scheduler. One execution owner serializes
// register access and real device notifications; there are no global instances.
// Authority and supported transaction widths: s2_interrupt_device_spec_20260919.
enum class Source : uint32_t {
    VBlank = 0, Gpu = 1, CdRom = 2, Dma = 3,
    Timer0 = 4, Timer1 = 5, Timer2 = 6, Pad = 7,
    Serial = 8, Spu = 9, Peripheral = 10
};
enum class DmaBoundary { TransferComplete, BlockComplete };

class Controller {
public:
    // Fresh native device: no asserted inputs or latched events. DPCR has the
    // documented reset value; SCUS initialization subsequently writes it.
    Controller() = default;

    // Unknown address: false, with result/state untouched. Recognized register
    // but unsupported width/alignment: throws before any effect. IRQ reads are
    // halfwords only; do not invent the open-bus upper half of a full-word read.
    // Narrow DMA writes are rejected: the source bus word is not transported by
    // the existing typed Write8/Write16 Services ABI.
    bool TryRead(uint32_t address, uint32_t width, uint32_t& result) const;
    bool TryWrite(uint32_t address, uint32_t width, uint32_t value);

    // Device producer drives its actual line. A held-high line does not relatch
    // after I_STAT acknowledgement. DMA's line is owned internally by DICR and
    // cannot also be driven through this interface.
    void SetLine(Source source, bool asserted);
    // Call only after the transfer/block owner has completed that work. This
    // function does not transfer memory, clear CHCR, or run native callbacks.
    void NotifyDma(uint32_t channel, DmaBoundary boundary);
    void NotifyDmaBusError();

    // Raw controller request only. BIOS/CPU interrupt enables, critical-section
    // state, and callback reentrancy remain responsibilities of the caller.
    uint16_t Pending() const noexcept;

private:
    uint16_t status_ = 0;
    uint16_t mask_ = 0;
    uint16_t lines_ = 0;
    uint32_t dpcr_ = 0x07654321u;
    uint32_t dmaControl_ = 0;
    uint32_t dmaFlags_ = 0;
    static uint32_t Register(uint32_t address);
    static void Validate(uint32_t address, uint32_t reg, uint32_t width, bool writing);
    bool DmaMaster() const noexcept;
    void Drive(uint32_t source, bool asserted) noexcept;
    void RefreshDmaLine() noexcept;
};
}
