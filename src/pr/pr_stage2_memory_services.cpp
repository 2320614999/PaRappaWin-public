#include "pr_stage2_memory_services.h"
#include <limits>
#include <string>

namespace PrStage2MemoryServices {
Services::Services() : bytes_(RamBytes), initialized_(RamBytes) {}

PrStage2SourceWord::Word Services::ReadSourceWord32(uint32_t a) {
    if(!RamAccess(a,4u))throw std::invalid_argument("Indeterminate copy requires main RAM");
    PrStage2SourceWord::Word word{};
    for(uint32_t i=0;i<4u;++i){
        const uint32_t p=(a+i)&(RamBytes-1u);
        if(p>=0x5D82Cu&&p<0x5D830u)return {Read32(a),15u}; // retain private-image identity guard
        if(initialized_[p]){word.known|=uint8_t(1u<<i);word.value|=uint32_t(bytes_[p])<<(8u*i);}
    }
    return word;
}
void Services::WriteSourceWord32(uint32_t a,PrStage2SourceWord::Word word) {
    if(word.known&0xF0u)throw std::invalid_argument("Unknown word lane mask");
    if(word.known==15u){Write32(a,word.value);return;}
    if(!RamAccess(a,4u))throw std::invalid_argument("Indeterminate copy requires main RAM");
    for(uint32_t i=0;i<4u;++i){
        const uint32_t p=(a+i)&(RamBytes-1u);
        if(p>=0x5D82Cu&&p<0x5D830u)throw std::logic_error("Unknown word overlaps private GPU identity");
    }
    for(uint32_t i=0;i<4u;++i){
        const uint32_t p=(a+i)&(RamBytes-1u);
        initialized_[p]=(word.known>>i)&1u;
        if(initialized_[p])bytes_[p]=uint8_t(word.value>>(8u*i));
    }
}

bool Services::RamAddress(uint32_t a) {
    return a < 0x00800000u || (a >= 0x80000000u && a < 0x80800000u) ||
           (a >= 0xA0000000u && a < 0xA0800000u);
}

bool Services::RamAccess(uint32_t a, uint32_t width) {
    // Validate all lanes before any read/write or external device call. The
    // native data ABI permits unaligned RAM words; it does not emulate CPU
    // alignment traps. An access may wrap a physical 2 MiB mirror, but may not
    // cross an 8 MiB mapped window or wrap the 32-bit address space.
    const uint64_t last = uint64_t(a) + width - 1u;
    if (last > std::numeric_limits<uint32_t>::max())
        throw std::out_of_range("Native memory address overflow");
    const bool ram = RamAddress(a);
    if (ram != RamAddress(static_cast<uint32_t>(last)))
        throw std::out_of_range("Native memory access crosses a RAM window");
    return ram;
}

uint32_t Services::ReadRam(uint32_t a, uint32_t width) const {
    uint32_t result = 0u;
    for (uint32_t i = 0; i < width; ++i) {
        const uint32_t p = (a + i) & (RamBytes - 1u);
        if (initialized_[p] == 0u)
            throw std::runtime_error("Native RAM read before initialization at " + std::to_string(a + i));
        result |= uint32_t(bytes_[p]) << (8u * i);
    }
    return result;
}

void Services::WriteRam(uint32_t a, uint32_t width, uint32_t value) {
    // Bounds were checked before entry. Fixed-size backing storage means no
    // allocation/callback can fail halfway through a native RAM transaction.
    for (uint32_t i = 0; i < width; ++i) {
        const uint32_t p = (a + i) & (RamBytes - 1u);
        bytes_[p] = static_cast<uint8_t>(value >> (8u * i));
        initialized_[p] = 1u;
    }
}

uint8_t Services::ReadImageMemory8(uint32_t a) {
    return RamAccess(a,1u) ? static_cast<uint8_t>(ReadRam(a,1u)) : ReadDevice8(a);
}
uint16_t Services::ReadImageMemory16(uint32_t a) {
    return RamAccess(a,2u) ? static_cast<uint16_t>(ReadRam(a,2u)) : ReadDevice16(a);
}
uint32_t Services::ReadImageMemory32(uint32_t a) {
    return RamAccess(a,4u) ? ReadRam(a,4u) : ReadDevice32(a);
}
void Services::WriteImageMemory8(uint32_t a,uint8_t v) {
    if (RamAccess(a,1u)) WriteRam(a,1u,v); else WriteDevice8(a,v);
}
void Services::WriteImageMemory16(uint32_t a,uint16_t v) {
    if (RamAccess(a,2u)) WriteRam(a,2u,v); else WriteDevice16(a,v);
}
void Services::WriteImageMemory32(uint32_t a,uint32_t v) {
    if (RamAccess(a,4u)) WriteRam(a,4u,v); else WriteDevice32(a,v);
}
}