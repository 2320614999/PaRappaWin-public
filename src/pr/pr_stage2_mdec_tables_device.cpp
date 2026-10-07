#include "pr_stage2_mdec_tables_device.h"
#include <stdexcept>

namespace PrStage2MdecTablesDevice {
void Device::Owner()const{if(owner_!=std::this_thread::get_id())throw std::logic_error("MDEC device used by another owner");}
uint32_t Device::Register(uint32_t a,uint32_t width){
    const uint32_t segment=a&0xE0000000u;if(segment!=0u&&segment!=0x80000000u&&segment!=0xA0000000u)return 0;
    a&=0x1FFFFFFFu;
    if(!((a>=0x1F801080u&&a<0x1F80108Cu)||(a>=0x1F801090u&&a<0x1F80109Cu)||(a>=0x1F801820u&&a<0x1F801828u)))return 0;
    if(width!=4u||(a&3u))throw std::invalid_argument("MDEC bounded adapter requires aligned word accesses");
    return a;
}
uint32_t Device::Status()const{
    uint32_t v=0x80040000u|((command_>>2u)&0x07800000u)|(statusLow_&65535u);
    if(active_)v|=0x20000000u;
    if((requestEnable_&0x40000000u)&&!faulted_)v|=0x10000000u;
    return v;
}
bool Device::TryRead(uint32_t a,uint32_t width,uint32_t& value)const{
    a=Register(a,width);if(!a)return false;Owner();
    if(a==0x1F801820u)throw std::logic_error("MDEC picture output has no decoder producer");
    if(a==0x1F801824u){value=Status();return true;}
    const uint32_t channel=(a-0x1F801080u)/16u,word=((a-0x1F801080u)%16u)/4u;
    value=channels_.at(channel).at(word);return true;
}
void Device::Word(uint32_t value){
    if(!active_){
        const uint32_t opcode=value>>29u;
        if(opcode!=2u&&opcode!=3u)throw std::logic_error("MDEC picture command is not bound; table adapter cannot report decode success");
        command_=value;received_=0;expected_=opcode==2u?((value&1u)?32u:16u):32u;
        statusLow_=expected_-1u;active_=true;return;
    }
    for(uint32_t i=0;i<4u;++i)parameters_.at(received_*4u+i)=uint8_t(value>>(8u*i));
    ++received_;statusLow_=expected_-received_-1u;
    if(received_==expected_){
        if((command_>>29u)==2u){
            for(uint32_t i=0;i<expected_*4u;++i)quant_[i]=parameters_[i];
            // A luma-only upload does not claim chroma has a known producer.
            quantKnown_=quantKnown_||expected_==32u;
        }else{scale_=parameters_;scaleKnown_=true;}
        active_=false;
    }
}
bool Device::TryWrite(uint32_t a,uint32_t width,uint32_t value){
    a=Register(a,width);if(!a)return false;Owner();
    if(a==0x1F801824u){
        requestEnable_=value&0x60000000u;
        if(value&0x80000000u){active_=false;command_=expected_=received_=statusLow_=0;faulted_=false;++serial_;}
        // Hardware reset aborts command state but preserves uploaded tables.
        return true;
    }
    if(a==0x1F801820u){
        if(Pending())throw std::logic_error("CPU command overwrites an owned MDEC DMA");
        if(faulted_)throw std::logic_error("Faulted MDEC consumer requires explicit reset");
        Word(value);return true;
    }
    const uint32_t c=(a-0x1F801080u)/16u,w=((a-0x1F801080u)%16u)/4u;
    if(w!=2u){if(channels_[c][2]&0x01000000u)throw std::logic_error("Active MDEC DMA register overwritten");channels_[c][w]=w==0u?(value&0x00FFFFFFu):value;return true;}
    if(!(value&0x01000000u)){channels_[c][2]=value;++serial_;return true;}
    if(c!=0u)throw std::logic_error("MDEC output DMA has no decoded image producer");
    if(Pending()||faulted_)throw std::logic_error("MDEC DMA reentry/fault");
    if(value!=0x01000201u)throw std::invalid_argument("Only forward request-mode MDEC table DMA is bound");
    const auto& channel=channels_[0];
    if((channel[0]&3u)||channel[0]>0x1FFFFCu)throw std::out_of_range("MDEC source outside aligned RAM");
    const uint64_t words=uint64_t(channel[1]&65535u)*(channel[1]>>16u);
    if(!active_||words==0u||words!=expected_-received_||words>32u||uint64_t(channel[0])+4u*words>0x200000u)
        throw std::invalid_argument("MDEC DMA length does not match the owned table parameters");
    channels_[0][2]=value;++starts_;++serial_;return true;
}
bool Device::Service(){
    Owner();if(!Pending())return false;if(faulted_)throw std::logic_error("Faulted MDEC transfer cannot complete");
    uint32_t dpcr=0;irq_.TryRead(0x1F8010F0u,4u,dpcr);
    if(!(dpcr&8u)||!(requestEnable_&0x40000000u))return false;
    try{
        auto& c=channels_[0];const uint32_t count=(c[1]&65535u)*(c[1]>>16u);
        std::array<uint32_t,32> input{};
        // Complete validation/read before changing the table. No zero fill for
        // an uninitialized source, and no event if any required byte is absent.
        for(uint32_t i=0;i<count;++i)input.at(i)=memory_.Read32(c[0]+i*4u);
        for(uint32_t i=0;i<count;++i)Word(input[i]);
        if(active_)throw std::logic_error("MDEC table still lacks parameter data");
        c[0]=(c[0]+count*4u)&0xFFFFFFu;c[1]&=65535u;c[2]&=~0x01000000u;
        words_+=count;++completions_;++serial_;
        irq_.NotifyDma(0u,PrStage2InterruptDevice::DmaBoundary::TransferComplete);
        return true;
    }catch(...){faulted_=true;throw;}
}
}
