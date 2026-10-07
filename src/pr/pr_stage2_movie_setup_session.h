#pragma once
#include "pr_stage2_transition_session.h"
#include "pr_stage2_movie_setup_direct.h"
#include "pr_stage2_movie_cd_direct.h"
#include "pr_stage2_mdec_tables_device.h"

namespace PrStage2MovieSetupSession {
class Session:public PrStage2TransitionSession::Session {
public:
    Session(const std::filesystem::path& scus,const std::filesystem::path& overlay,
            const std::filesystem::path& disc,D3D11Renderer& renderer,IAudioSink& sink)
        :PrStage2TransitionSession::Session(scus,overlay,disc,renderer,sink),mdec_(*this,Interrupts()){}
    PrStage2MdecTablesDevice::Device& Mdec()noexcept{return mdec_;}
protected:
    virtual int32_t MovieDependency(uint32_t,std::initializer_list<uint32_t>)=0;
private:
    using Args=std::initializer_list<uint32_t>;
    PrStage2MdecTablesDevice::Device mdec_;
protected:
    int32_t SceneDependency(uint32_t fn,Args args)override{
        int32_t value;if(PrStage2MovieSetupDirect::TryCall(*this,fn,args,value))return value;
        if(PrStage2MovieCdDirect::TryCall(*this,fn,args,value))return value;
        return MovieDependency(fn,args);
    }
    void CallLoadingDeviceVoid(uint32_t fn,Args args)override{
        if(PrStage2MovieSetupDirect::TryCallVoid(*this,fn,args))return;
        PrStage2VabSession::Session::CallLoadingDeviceVoid(fn,args);
    }
    uint32_t ReadDevice32(uint32_t a)override{
        uint32_t value;if(mdec_.TryRead(a,4u,value))return value;
        return PrStage2VabSession::Session::ReadDevice32(a);
    }
    void WriteDevice32(uint32_t a,uint32_t v)override{
        if(!mdec_.TryWrite(a,4u,v))PrStage2VabSession::Session::WriteDevice32(a,v);
    }
    bool ServiceAdditionalDevices()override{
        const bool spu=PrStage2VabSession::Session::ServiceAdditionalDevices();
        const bool mdec=mdec_.Service();return spu||mdec;
    }
    void AwaitDeviceProgress(uint32_t a)override{
        const uint32_t p=a&0x1FFFFFFFu;
        if(p==0x1F801824u||p==0x1F801088u){
            if(NativeIrqActive()){mdec_.Service();return;}
            WaitDevice(a,uint32_t(mdec_.Serial()));
        }else PrStage2VabSession::Session::AwaitDeviceProgress(a);
    }
};
}
