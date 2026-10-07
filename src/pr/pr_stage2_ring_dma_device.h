#pragma once
#include "pr_stage2_cd_command_device.h"
#include "pr_stage2_lifecycle_direct.h"
#include <thread>
#include <vector>

namespace PrStage2RingDmaDevice {
struct Transfer {uint32_t sector,destination,words,control;};
class Device final {
public:
    Device(PrStage2LifecycleDirect::Services& memory,PrStage2CdCommandDevice::Device& cd,
           PrStage2InterruptDevice::Controller& irq):memory_(memory),cd_(cd),irq_(irq){}
    bool TryRead(uint32_t address,uint32_t width,uint32_t& value)const;
    bool TryWrite(uint32_t address,uint32_t width,uint32_t value);
    bool Service();
    // Source register bits are retained through the original SB bus shift.
    // This is deliberately NOT a byte-mask update of DICR.
    uint8_t LoadOnDieByte(uint32_t address)const;
    void StoreOnDieByte(uint32_t address,uint32_t sourceRegister);
    bool Pending()const noexcept{return (control_&0x01000000u)!=0u;}
    bool Faulted()const noexcept{return faulted_;}
    uint64_t Serial()const noexcept{return serial_;}
    uint64_t Starts()const noexcept{return starts_;}
    uint64_t Completions()const noexcept{return completed_;}
    uint64_t WordsWritten()const noexcept{return written_;}
    uint32_t ProgressWords()const noexcept{return progress_;}
    const std::vector<Transfer>& Transfers()const noexcept{return transfers_;}
private:
    PrStage2LifecycleDirect::Services& memory_;
    PrStage2CdCommandDevice::Device& cd_;
    PrStage2InterruptDevice::Controller& irq_;
    std::thread::id owner_=std::this_thread::get_id();
    uint32_t address_=0,count_=0,control_=0,busDelay_=0,progress_=0;
    bool faulted_=false;
    uint64_t serial_=0,starts_=0,completed_=0,written_=0;
    std::vector<Transfer> transfers_;
    void Owner()const;
    static uint32_t Physical(uint32_t address);
    static uint32_t Register(uint32_t address,uint32_t width);
};
}
