#include "pr_stage2_ring_dma_device.h"
#include <stdexcept>

namespace PrStage2RingDmaDevice {
namespace {[[noreturn]]void Fail(const char* text){throw std::runtime_error(text);}}
void Device::Owner()const{if(owner_!=std::this_thread::get_id())throw std::logic_error("DMA3 owner thread mismatch");}
uint32_t Device::Physical(uint32_t a){const uint32_t segment=a&0xE0000000u;return segment==0u||segment==0x80000000u||segment==0xA0000000u?a&0x1FFFFFFFu:0u;}
uint32_t Device::Register(uint32_t a,uint32_t width){
    const uint32_t p=Physical(a);
    if(p!=0x1F801018u&&(p<0x1F8010B0u||p>=0x1F8010BCu))return 0u;
    if(width!=4u||(p&3u))throw std::invalid_argument("DMA3 requires aligned word register transactions");return p;
}
bool Device::TryRead(uint32_t a,uint32_t width,uint32_t& value)const{
    const uint32_t p=Register(a,width);if(!p)return false;Owner();
    if(p==0x1F801018u)value=busDelay_;else if(p==0x1F8010B0u)value=address_;
    else if(p==0x1F8010B4u)value=count_;else value=control_;return true;
}
bool Device::TryWrite(uint32_t a,uint32_t width,uint32_t value){
    const uint32_t p=Register(a,width);if(!p)return false;Owner();
    if(p==0x1F801018u){busDelay_=value;return true;}
    if(Pending())throw std::logic_error("An active DMA3 register cannot be overwritten");
    if(p==0x1F8010B0u)address_=value&0xFFFFFFu;
    else if(p==0x1F8010B4u)count_=value;
    else if(!(value&0x01000000u)){control_=value;faulted_=false;progress_=0u;++serial_;}
    else{
        if(value!=0x11000000u&&value!=0x11400100u)throw std::invalid_argument("Unbound DMA3 direction or mode");
        const uint32_t words=(count_&65535u)?count_&65535u:65536u;
        if((address_&3u)||address_>=0x800000u||uint64_t(address_)+uint64_t(words)*4u>0x800000u)
            throw std::out_of_range("DMA3 destination is outside original main-RAM mirrors");
        control_=value;progress_=0u;faulted_=false;++starts_;++serial_;
    }
    return true;
}
uint8_t Device::LoadOnDieByte(uint32_t a)const{
    Owner();const uint32_t p=Physical(a);if(p!=0x1F8010F6u)throw std::invalid_argument("Unbound on-die byte read");
    uint32_t value=0u;irq_.TryRead(p&~3u,4u,value);return uint8_t(value>>16u);
}
void Device::StoreOnDieByte(uint32_t a,uint32_t value){
    Owner();const uint32_t p=Physical(a);if(p!=0x1F8010F6u)throw std::invalid_argument("Unbound on-die SB bus store");
    // CPU-package byte enables do not mask this 32-bit latch. Preserve source
    // bits 8..15 as well: they can acknowledge DMA flags after shifting by 16.
    irq_.TryWrite(p&~3u,4u,value<<16u);
}
bool Device::Service(){
    Owner();if(faulted_)Fail("DMA3 failed; no completion can be manufactured");if(!Pending())return false;
    uint32_t priority=0;irq_.TryRead(0x1F8010F0u,4u,priority);
    if(!(priority&0x8000u)||cd_.DmaBytesAvailable()==0u)return false;
    try{
        const uint32_t words=(count_&65535u)?count_&65535u:65536u;
        if(uint64_t(words)*4u>cd_.DmaBytesAvailable())Fail("DMA3 request exceeds the actual available sector payload");
        const uint32_t initial=address_,originalControl=control_,lba=cd_.LastSector();
        // Reserve evidence storage before touching the FIFO. It cannot fail
        // after a transfer and turn a completed write into an artificial error.
        transfers_.reserve(transfers_.size()+1u);
        control_&=~0x10000000u;
        for(uint32_t i=0;i<words;++i){
            const uint32_t word=cd_.ReadDmaWord();
            memory_.Write32(0x80000000u|((initial+4u*i)&0x1FFFFFu),word);
            ++progress_;++written_;
        }
        if(!cd_.DmaBytesAvailable())cd_.ReleaseConsumedSector();
        if(originalControl&0x100u){address_=(initial+4u*words)&0xFFFFFFu;count_&=0xFFFF0000u;}
        control_&=~0x01000000u;++completed_;++serial_;
        transfers_.push_back({lba,initial,words,originalControl});
        irq_.NotifyDma(3u,PrStage2InterruptDevice::DmaBoundary::TransferComplete);
        return true;
    }catch(...){faulted_=true;throw;}
}
}
