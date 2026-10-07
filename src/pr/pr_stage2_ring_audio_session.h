#pragma once
#include "pr_stage2_ring_session.h"
#include "pr_stage2_ring_acquire_direct.h"
#include "pr_stage2_xa_device.h"

namespace PrStage2RingAudioSession {
// Same retained native owner; actual video FIFO/DMA3 and XA/SPU sample queues
// have different consumers. Neither one supplies a fake VLC/MDEC completion.
class Session:public PrStage2RingSession::Session {
public:
    Session(const std::filesystem::path& scus,const std::filesystem::path& overlay,
            const std::filesystem::path& disc,D3D11Renderer& renderer,IAudioSink& sink)
        :Base(scus,overlay,disc,renderer,sink),xa_(Spu()){Cd().BindXaConsumer(xa_);}
    ~Session(){Cancel();Output().Stop();}
    PrStage2XaDevice::Device& Xa()noexcept{return xa_;}
protected:
    virtual int32_t DecodeDependency(uint32_t,std::initializer_list<uint32_t>)=0;
    void CallLoadingDeviceVoid(uint32_t fn,std::initializer_list<uint32_t> args)override{
        if(PrStage2RingAcquireDirect::TryCallVoid(*this,fn,args))return;
        Base::CallLoadingDeviceVoid(fn,args);
    }
private:
    using Base=PrStage2RingSession::Session;
    PrStage2XaDevice::Device xa_;
    int32_t RingDependency(uint32_t fn,std::initializer_list<uint32_t> args)override{
        int32_t result;if(PrStage2RingAcquireDirect::TryCall(*this,fn,args,result))return result;
        return DecodeDependency(fn,args);
    }
};
}
