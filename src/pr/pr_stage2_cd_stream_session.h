#pragma once
#include "pr_stage2_cd_command_session.h"
#include "pr_stage2_cd_stream_direct.h"

namespace PrStage2CdStreamSession {
class Session:public PrStage2CdCommandSession::Session,public PrStage2CdParameters::Engine {
public:
    using PrStage2CdCommandSession::Session::Session;
    int32_t CallPrivateCdCommand(uint32_t command,const PrStage2CdParameters::View& parameters,uint32_t output,uint32_t nonblocking)override{
        if(&parameters.memory!=this||!parameters.local||!parameters.validBytes)
            throw std::invalid_argument("Private CD command has no matching retained owner");
        return PrStage2CdCommandDirect::CommandParameters800375BC(*this,command,parameters,output,nonblocking);
    }
protected:
    void CallLoadingDeviceVoid(uint32_t fn,std::initializer_list<uint32_t> args)override{
        if(PrStage2CdStreamDirect::TryCallVoid(*this,fn,args))return;
        PrStage2CdCommandSession::Session::CallLoadingDeviceVoid(fn,args);
    }
    virtual int32_t StreamDependency(uint32_t,std::initializer_list<uint32_t>)=0;
private:
    int32_t CdDependency(uint32_t fn,std::initializer_list<uint32_t> args)override{
        int32_t result;if(PrStage2CdStreamDirect::TryCall(*this,fn,args,result))return result;
        return StreamDependency(fn,args);
    }
};
}
