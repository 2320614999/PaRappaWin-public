#pragma once
#include "pr_stage2_loading_device_session.h"
#include "pr_stage2_spu_transfer_device.h"
#include "pr_stage2_spu_events.h"

namespace PrStage2VabSession {
// Extends the same retained Loading owner with source VAB/SPU initialization,
// actual FIFO/DMA4 transport and its explicit software-event adapter. The base
// owner remains available for the already-verified missing-VAB negative test.
class Session : public PrStage2LoadingDeviceSession::Session {
public:
    Session(const std::filesystem::path&,const std::filesystem::path&,
            const std::filesystem::path&,D3D11Renderer&,IAudioSink&);
    PrStage2SpuTransferDevice::Device& SpuTransfer(){return transfer_;}
    PrStage2SpuEvents::Controller& SpuEvents(){return events_;}
protected:
    virtual int32_t RemainingDependency(uint32_t,std::initializer_list<uint32_t>)=0;
    virtual void RemainingDependencyVoid(uint32_t function,std::initializer_list<uint32_t> args){(void)RemainingDependency(function,args);}
private:
    using Base=PrStage2LoadingDeviceSession::Session;
    using Args=std::initializer_list<uint32_t>;
    PrStage2SpuTransferDevice::Device transfer_;
    PrStage2SpuEvents::Controller events_{true};
    bool eventsActive_=false;
protected:
    int32_t PlatformDependency(uint32_t,Args)final;
    void CallLoadingDeviceVoid(uint32_t,Args)override;
    bool ServiceAdditionalDevices()override;
    bool AdditionalInterruptsEnabled()const final;
    uint8_t ReadDevice8(uint32_t)override;
    uint16_t ReadDevice16(uint32_t)final;
    uint32_t ReadDevice32(uint32_t)override;
    void WriteDevice8(uint32_t,uint8_t)override;
    void WriteDevice16(uint32_t,uint16_t)final;
    void WriteDevice32(uint32_t,uint32_t)override;
    void AwaitDeviceProgress(uint32_t)override;
};
}
