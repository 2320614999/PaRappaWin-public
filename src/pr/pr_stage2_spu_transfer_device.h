#pragma once
#include "pr_stage2_lifecycle_direct.h"
#include "pr_stage2_spu_device.h"
#include "pr_stage2_interrupt_device.h"
#include <array>
#include <thread>

namespace PrStage2SpuTransferDevice {
// Bounded native FIFO/manual-write and DMA4 transfer owner. Register reads are
// observations. Service performs data transport before publishing DMA IRQs;
// only the translated source ISR may deliver its software completion event.
class Device final {
public:
    Device(PrStage2LifecycleDirect::Services&,PrStage2SpuDevice::Device&,
           PrStage2InterruptDevice::Controller&);
    bool TryRead(uint32_t address,uint32_t width,uint32_t& value) const;
    bool TryWrite(uint32_t address,uint32_t width,uint32_t value);
    bool Service();
    bool Pending()const{return fifoPending_||(control_&0x01000000u);}
    uint64_t DmaStarts()const{return starts_;}
    uint64_t DmaCompletions()const{return completions_;}
    uint64_t FifoCompletions()const{return fifoCompletions_;}
    uint64_t DmaBytes()const{return bytes_;}
    uint64_t FifoBytes()const{return fifoBytes_;}
    uint32_t Cursor()const{return cursor_;}
    bool Faulted()const{return faulted_;}
private:
    PrStage2LifecycleDirect::Services& memory_;
    PrStage2SpuDevice::Device& spu_;
    PrStage2InterruptDevice::Controller& interrupts_;
    std::thread::id owner_;
    uint32_t address_=0,blocks_=0,control_=0,cursor_=0,delay_=0x200931E1u;
    std::array<uint8_t,64> fifo_{};
    uint32_t fifoSize_=0;
    bool fifoPending_=false,faulted_=false,servicing_=false;
    uint64_t starts_=0,completions_=0,bytes_=0,fifoCompletions_=0,fifoBytes_=0;
    void Owner()const;
    uint32_t SpuControl()const;
    static uint32_t Register(uint32_t address,uint32_t width);
};
}
