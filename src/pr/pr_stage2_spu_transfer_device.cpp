#include "pr_stage2_spu_transfer_device.h"
#include <stdexcept>
#include <vector>

namespace PrStage2SpuTransferDevice {
Device::Device(PrStage2LifecycleDirect::Services& m,PrStage2SpuDevice::Device& s,
               PrStage2InterruptDevice::Controller& i):memory_(m),spu_(s),interrupts_(i),owner_(std::this_thread::get_id()){}
void Device::Owner()const{if(owner_!=std::this_thread::get_id())throw std::logic_error("SPU transfer owner thread mismatch");}
uint32_t Device::Register(uint32_t address,uint32_t width){
    const uint32_t segment=address&0xE0000000u;
    if(segment!=0u&&segment!=0x80000000u&&segment!=0xA0000000u)return 0u;
    const uint32_t physical=address&0x1FFFFFFFu;
    const bool dma=(physical>=0x1F8010C0u&&physical<0x1F8010CCu)||(physical>=0x1F801014u&&physical<0x1F801018u);
    const bool port=(physical>=0x1F801DA6u&&physical<0x1F801DB0u)||(physical>=0x1F801D9Cu&&physical<0x1F801DA0u);
    if(!dma&&!port)return 0u;
    const uint32_t expected=dma?4u:2u;
    if(width!=expected||(physical&(expected-1u)))throw std::invalid_argument("SPU DMA/FIFO transaction width/alignment");
    return physical;
}
uint32_t Device::SpuControl()const{uint32_t result=0;if(!spu_.TryRead(0x1F801DAAu,2u,result))throw std::logic_error("Missing SPU control owner");return result;}
bool Device::TryRead(uint32_t address,uint32_t width,uint32_t& value)const{
    Owner();const uint32_t reg=Register(address,width);if(!reg)return false;
    if(reg==0x1F8010C0u)value=address_;
    else if(reg==0x1F8010C4u)value=blocks_;
    else if(reg==0x1F8010C8u)value=control_;
    else if(reg==0x1F801014u)value=delay_;
    else if(reg==0x1F801DAEu){
        const uint32_t ctl=SpuControl(),mode=ctl&0x30u;
        value=(ctl&0x3Fu)|(fifoPending_?0x400u:0u);
        if(mode==0x20u&&!fifoPending_)value|=0x180u;
        if(mode==0x30u&&!fifoPending_)value|=0x280u;
    }else if(reg==0x1F801DA8u)throw std::logic_error("SPU FIFO readback mode is not bound");
    else return spu_.TryRead(address,width,value);
    return true;
}
bool Device::TryWrite(uint32_t address,uint32_t width,uint32_t value){
    Owner();const uint32_t reg=Register(address,width);if(!reg)return false;
    if(width==2u&&value>65535u)throw std::invalid_argument("SPU halfword exceeds width");
    if(servicing_)throw std::logic_error("SPU transfer descriptor reentrancy");
    if(reg==0x1F801014u){
        if(control_&0x01000000u)throw std::logic_error("Memory control changed during SPU DMA");
        if((value&0xF0FFFFFFu)!=0x200931E1u||(value&0x0D000000u))throw std::logic_error("Unimplemented SPU bus-delay mode");
        delay_=value;return true;
    }
    if(reg==0x1F8010C0u||reg==0x1F8010C4u){
        if(control_&0x01000000u)throw std::logic_error("Active SPU DMA descriptor overwritten");
        if(reg==0x1F8010C0u)address_=value&0xFFFFFFu;else blocks_=value;return true;
    }
    if(reg==0x1F8010C8u){
        if(control_&0x01000000u)throw std::logic_error("Active SPU DMA cannot be silently cancelled");
        if(value!=0u&&value!=0x01000201u&&value!=0x01000200u)throw std::logic_error("Only forward slice-mode SPU DMA is bound");
        if(value){
            if(faulted_||fifoSize_||fifoPending_)throw std::logic_error("SPU transfer fault or FIFO collision");
            const uint32_t expected=(value&1u)?0x20u:0x30u;
            if((SpuControl()&0x30u)!=expected)throw std::logic_error("SPU/CPU DMA direction disagreement");
            const uint64_t blockWords=(blocks_&65535u)?(blocks_&65535u):65536u;
            const uint64_t count=(blocks_>>16u)?(blocks_>>16u):65536u;
            const uint64_t length=blockWords*count*4u;
            if(blockWords>16u||!length||(address_&3u)||address_>=0x200000u||length>0x200000u-address_||cursor_>PrStage2SpuDevice::Device::RamBytes||length>PrStage2SpuDevice::Device::RamBytes-cursor_)
                throw std::out_of_range("Unsupported SPU DMA span, wrap, or buffer size");
            ++starts_;
        }
        control_=value;return true;
    }
    if(reg==0x1F801DA6u){
        if(Pending()||fifoSize_)throw std::logic_error("SPU address changed with active FIFO/DMA");
        if(!spu_.TryWrite(address,width,value))throw std::logic_error("SPU transfer address owner missing");
        cursor_=value*8u;return true;
    }
    if(reg==0x1F801DA8u){
        if(Pending()||faulted_||fifoSize_>62u)throw std::logic_error("SPU FIFO busy, full or faulted");
        fifo_[fifoSize_++]=uint8_t(value);fifo_[fifoSize_++]=uint8_t(value>>8u);return true;
    }
    if(reg==0x1F801DAAu){
        const uint32_t mode=value&0x30u;
        if(fifoPending_||(control_&0x01000000u))throw std::logic_error("SPU control changed before actual transfer completion");
        if(fifoSize_&&mode!=0x10u)throw std::logic_error("Unconsumed SPU FIFO cannot be discarded by a mode write");
        spu_.ApplyTransferControl(uint16_t(value));
        if(mode==0x10u&&fifoSize_)fifoPending_=true;
        return true;
    }
    if(reg==0x1F801D9Cu||reg==0x1F801D9Eu){
        if(value!=0u)throw std::logic_error("Only the source SDK's zero ENDX write is bound");
        spu_.WriteSdkEndStatusZero();return true;
    }
    if(reg==0x1F801DAEu)throw std::logic_error("SPUSTAT is device owned");
    return spu_.TryWrite(address,width,value);
}
bool Device::Service(){
    Owner();if(faulted_)throw std::logic_error("SPU transfer previously failed");
    if(servicing_)throw std::logic_error("SPU transfer service reentrancy");
    if(!Pending())return false;
    if(control_&0x01000000u){uint32_t priority=0;interrupts_.TryRead(0x1F8010F0u,4u,priority);if(!(priority&0x80000u))return false;}
    servicing_=true;
    try{
        if(fifoPending_){
            const std::vector<uint8_t> data(fifo_.begin(),fifo_.begin()+fifoSize_);
            spu_.Upload(cursor_,data);cursor_+=fifoSize_;fifoBytes_+=fifoSize_;
            fifoSize_=0u;fifoPending_=false;++fifoCompletions_;
        }else{
            const uint32_t count=(blocks_>>16u)?(blocks_>>16u):65536u;
            const uint32_t length=(blocks_&65535u)*count*4u;
            if(control_&1u){
                std::vector<uint8_t> bytes;bytes.reserve(length);
                for(uint32_t i=0;i<length;i+=4u){const uint32_t word=memory_.Read32(0x80000000u|((address_+i)&0x1FFFFFu));for(uint32_t j=0;j<4u;++j)bytes.push_back(uint8_t(word>>(8u*j)));}
                spu_.Upload(cursor_,bytes);
            }else{
                if(!(delay_&0x02000000u))throw std::logic_error("SPU bus is not configured for DMA reads");
                const auto bytes=spu_.ReadRam(cursor_,length);
                for(uint32_t i=0;i<length;i+=4u){uint32_t word=0;for(uint32_t j=0;j<4u;++j)word|=uint32_t(bytes[i+j])<<(8u*j);memory_.Write32(0x80000000u|(address_+i),word);}
            }
            cursor_+=length;address_=(address_+length)&0xFFFFFFu;blocks_&=65535u;control_&=~0x01000000u;
            bytes_+=length;++completions_;
            interrupts_.NotifyDma(4u,PrStage2InterruptDevice::DmaBoundary::TransferComplete);
        }
        servicing_=false;return true;
    }catch(...){servicing_=false;faulted_=true;throw;}
}
}
