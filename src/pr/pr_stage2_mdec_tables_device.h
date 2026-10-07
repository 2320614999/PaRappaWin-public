#pragma once
#include "pr_stage2_lifecycle_direct.h"
#include "pr_stage2_interrupt_device.h"
#include <array>
#include <thread>

namespace PrStage2MdecTablesDevice {
// Concrete bounded MDEC reset/table transport. Parameter words really come
// from session RAM via DMA0. Picture decode/DMA1 output is still unbound and
// rejected, never represented by a successful completion or blank frame.
class Device final {
public:
    Device(PrStage2LifecycleDirect::Services& memory,PrStage2InterruptDevice::Controller& irq):memory_(memory),irq_(irq){}
    bool TryRead(uint32_t address,uint32_t width,uint32_t& value) const;
    bool TryWrite(uint32_t address,uint32_t width,uint32_t value);
    bool Service();
    bool Pending()const noexcept{return (channels_[0][2]&0x01000000u)!=0u;}
    uint64_t Starts()const noexcept{return starts_;}
    uint64_t Completions()const noexcept{return completions_;}
    uint64_t WordsTransferred()const noexcept{return words_;}
    uint64_t Serial()const noexcept{return serial_;}
    bool QuantKnown()const noexcept{return quantKnown_;}
    bool ScaleKnown()const noexcept{return scaleKnown_;}
    const std::array<uint8_t,128>& Quant()const noexcept{return quant_;}
    const std::array<uint8_t,128>& ScaleBytes()const noexcept{return scale_;}
    bool Faulted()const noexcept{return faulted_;}
private:
    PrStage2LifecycleDirect::Services& memory_;
    PrStage2InterruptDevice::Controller& irq_;
    std::array<std::array<uint32_t,3>,2> channels_{};
    std::array<uint8_t,128> quant_{},scale_{},parameters_{};
    uint32_t requestEnable_=0,command_=0,expected_=0,received_=0,statusLow_=0;
    bool active_=false,quantKnown_=false,scaleKnown_=false,faulted_=false;
    uint64_t starts_=0,completions_=0,words_=0,serial_=0;
    std::thread::id owner_=std::this_thread::get_id();
    void Owner()const;
    static uint32_t Register(uint32_t,uint32_t);
    uint32_t Status()const;
    void Word(uint32_t);
};
}
