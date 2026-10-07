#pragma once
#include "pr_stage2_lifecycle_direct.h"

namespace PrStage2GpuDirect {
// Host storage for the one original pointer field which may name a private
// RECT. All RAM/MMIO accesses and callbacks must go through this Services
// object. This is data/ABI transport, not an interpreter or a diagnostic log.
class ImageSourceServices : public PrStage2LifecycleDirect::Services {
    using Source = PrStage2LifecycleDirect::GpuCommandSource;
    Source source_{};
    static bool RamWindow(uint32_t a) {
        return a < 0x00800000u || (a >= 0x80000000u && a < 0x80800000u) ||
               (a >= 0xA0000000u && a < 0xA0800000u);
    }
    static bool Overlaps(uint32_t a, uint32_t bytes) {
        if (!RamWindow(a)) return false;
        const uint32_t offset = a & 0x1FFFFFu;
        return offset < 0x5D830u && uint64_t(offset) + bytes > 0x5D82Cu;
    }
    void CheckRawRead(uint32_t a, uint32_t bytes) const {
        if (source_.local && Overlaps(a, bytes))
            throw std::logic_error("A native GPU source has no raw PSX pointer representation");
    }
    void BeforeRawWrite(uint32_t a, uint32_t bytes) {
        if (!Overlaps(a, bytes)) return;
        if (source_.local && (bytes != 4u || (a & 0x1FFFFFu) != 0x5D82Cu))
            throw std::logic_error("Partial overwrite of a native GPU pointer is unsupported");
    }
protected:
    virtual uint8_t ReadImageMemory8(uint32_t address) = 0;
    virtual uint16_t ReadImageMemory16(uint32_t address) = 0;
    virtual uint32_t ReadImageMemory32(uint32_t address) = 0;
    virtual void WriteImageMemory8(uint32_t address, uint8_t value) = 0;
    virtual void WriteImageMemory16(uint32_t address, uint16_t value) = 0;
    virtual void WriteImageMemory32(uint32_t address, uint32_t value) = 0;
public:
    bool SupportsPrivateGpuSources() const final { return true; }
    uint8_t Read8(uint32_t a) final { CheckRawRead(a,1); return ReadImageMemory8(a); }
    uint16_t Read16(uint32_t a) final { CheckRawRead(a,2); return ReadImageMemory16(a); }
    uint32_t Read32(uint32_t a) final { CheckRawRead(a,4); return ReadImageMemory32(a); }
    void Write8(uint32_t a,uint8_t v) final {
        BeforeRawWrite(a,1); WriteImageMemory8(a,v);
        if (Overlaps(a,1)) source_ = {};
    }
    void Write16(uint32_t a,uint16_t v) final {
        BeforeRawWrite(a,2); WriteImageMemory16(a,v);
        if (Overlaps(a,2)) source_ = {};
    }
    void Write32(uint32_t a,uint32_t v) final {
        BeforeRawWrite(a,4); WriteImageMemory32(a,v);
        if (Overlaps(a,4)) source_ = {};
    }
    Source ReadGpuCommandSource() override {
        return source_.local ? source_ : Source{Read32(0x8005D82Cu), {}};
    }
    void WriteGpuCommandSource(const Source& source) override {
        if (source.local) source_ = source;
        else Write32(0x8005D82Cu, source.address);
    }
};
}
