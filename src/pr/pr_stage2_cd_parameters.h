#pragma once
#include "pr_stage2_lifecycle_direct.h"
#include <cstddef>
#include <stdexcept>

namespace PrStage2CdParameters {
// An original private byte array is a native reference, never a made-up RAM
// address. Only bytes actually initialized by the source may be read.
struct View {
    PrStage2LifecycleDirect::Services& memory;
    uint32_t address=0;
    const uint8_t* local=nullptr;
    size_t validBytes=0;
    bool Present()const noexcept{return address!=0u||local!=nullptr;}
    uint8_t Read(uint32_t offset)const{
        if(local){if(offset>=validBytes)throw std::out_of_range("Uninitialized private CD parameter read");return local[offset];}
        return memory.Read8(address+offset);
    }
};
struct Engine {
    virtual ~Engine()=default;
    virtual int32_t CallPrivateCdCommand(uint32_t command,const View& parameters,uint32_t output,uint32_t nonblocking)=0;
};
}
