#pragma once
#include "pr_stage2_native_dispatch.h"
#include "pr_stage2_source_word.h"
#include <vector>

namespace PrStage2MemoryServices {
// Native data storage for the established 2 MiB / twelve-alias RAM contract.
// This class neither fetches instructions nor models a CPU/cache. RAM bytes
// become readable only after an explicit write by a loader or native routine.
// Non-RAM transactions preserve their original address and width; the concrete
// device owner implements MMIO/scratchpad/BIOS access or rejects it explicitly.
class Services : public PrStage2NativeDispatch::Services, public PrStage2SourceWord::Memory {
public:
    static constexpr uint32_t RamBytes = 0x200000u;
    Services();
    Services(const Services&) = delete;
    Services& operator=(const Services&) = delete;
    Services(Services&&) = delete;
    Services& operator=(Services&&) = delete;
    PrStage2SourceWord::Word ReadSourceWord32(uint32_t address) override;
    void WriteSourceWord32(uint32_t address,PrStage2SourceWord::Word word) override;
protected:
    virtual uint8_t ReadDevice8(uint32_t address) = 0;
    virtual uint16_t ReadDevice16(uint32_t address) = 0;
    virtual uint32_t ReadDevice32(uint32_t address) = 0;
    virtual void WriteDevice8(uint32_t address, uint8_t value) = 0;
    virtual void WriteDevice16(uint32_t address, uint16_t value) = 0;
    virtual void WriteDevice32(uint32_t address, uint32_t value) = 0;
private:
    std::vector<uint8_t> bytes_;
    std::vector<uint8_t> initialized_;
    static bool RamAddress(uint32_t address);
    static bool RamAccess(uint32_t address, uint32_t width);
    uint32_t ReadRam(uint32_t address, uint32_t width) const;
    void WriteRam(uint32_t address, uint32_t width, uint32_t value);
    uint8_t ReadImageMemory8(uint32_t address) final;
    uint16_t ReadImageMemory16(uint32_t address) final;
    uint32_t ReadImageMemory32(uint32_t address) final;
    void WriteImageMemory8(uint32_t address, uint8_t value) final;
    void WriteImageMemory16(uint32_t address, uint16_t value) final;
    void WriteImageMemory32(uint32_t address, uint32_t value) final;
};
}