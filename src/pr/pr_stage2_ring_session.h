#pragma once
#include "pr_stage2_cd_stream_session.h"
#include "pr_stage2_ring_direct.h"
#include "pr_stage2_ring_dma_device.h"

namespace PrStage2RingSession {
class Session:public PrStage2CdStreamSession::Session,public PrStage2RingDirect::Bus {
public:
    Session(const std::filesystem::path& scus,const std::filesystem::path& overlay,
            const std::filesystem::path& disc,D3D11Renderer& renderer,IAudioSink& sink)
        :Base(scus,overlay,disc,renderer,sink),dma_(*this,Cd(),Interrupts()){}
    PrStage2RingDmaDevice::Device& RingDma()noexcept{return dma_;}
    uint8_t LoadOnDieByte(uint32_t a)override{return dma_.LoadOnDieByte(a);}
    void StoreOnDieByte(uint32_t a,uint32_t value)override{dma_.StoreOnDieByte(a,value);}
protected:
    virtual int32_t RingDependency(uint32_t,std::initializer_list<uint32_t>)=0;
private:
    using Base=PrStage2CdStreamSession::Session;
    PrStage2RingDmaDevice::Device dma_;
    int32_t StreamDependency(uint32_t fn,std::initializer_list<uint32_t> args)override{
        int32_t result;if(PrStage2RingDirect::TryCall(*this,fn,args,result))return result;
        return RingDependency(fn,args);
    }
protected:
    uint32_t ReadDevice32(uint32_t a)override{uint32_t v;if(dma_.TryRead(a,4u,v))return v;return Base::ReadDevice32(a);}
    void WriteDevice32(uint32_t a,uint32_t v)override{if(!dma_.TryWrite(a,4u,v))Base::WriteDevice32(a,v);}
    void CallLoadingDeviceVoid(uint32_t fn,std::initializer_list<uint32_t> args)override{
        if(PrStage2RingDirect::TryCallVoid(*this,fn,args))return;Base::CallLoadingDeviceVoid(fn,args);
    }
    bool ServiceAdditionalDevices()override{
        const bool dma=dma_.Service();const bool devices=Base::ServiceAdditionalDevices();return dma||devices;
    }
    void AwaitDeviceProgress(uint32_t a)override{
        if((a&0x1FFFFFFFu)==0x1F8010B8u){
            if(NativeIrqActive()){dma_.Service();return;}
            WaitDevice(a,uint32_t(dma_.Serial()));
        }
        else Base::AwaitDeviceProgress(a);
    }
};
}
