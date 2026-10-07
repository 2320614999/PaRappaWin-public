#pragma once
#include "pr_stage2_vab_session.h"
#include "pr_stage2_transition_direct.h"

namespace PrStage2TransitionSession {
// Continue the already completed resource initializer on the same RAM/device
// owner. Unbound movie/input/platform calls still fail at the caller's boundary.
class Session : public PrStage2VabSession::Session {
public:
    using PrStage2VabSession::Session::Session;
protected:
    virtual int32_t SceneDependency(uint32_t,std::initializer_list<uint32_t>)=0;
private:
    void RemainingDependencyVoid(uint32_t fn,std::initializer_list<uint32_t> args) final {
        // 遮罩图案保留 void ABI；其余入口继续交给既有依赖链。
        if(PrStage2TransitionDirect::TryCallVoid(*this,fn,args))return;
        PrStage2VabSession::Session::RemainingDependencyVoid(fn,args);
    }
    int32_t RemainingDependency(uint32_t fn,std::initializer_list<uint32_t> args) final {
        int32_t result;
        if(PrStage2TransitionDirect::TryCall(*this,fn,args,result))return result;
        if(fn==0x8001EEACu||fn==0x80026EF8u){
            if(args.size()!=1u)throw std::invalid_argument("S2 transition shared-callee argument count mismatch");
            return fn==0x8001EEACu?PrStage2LifecycleDirect::FillDrawFlags8001EEAC(*this,*args.begin())
                :PrStage2LifecycleDirect::PlayCue80026EF8(*this,*args.begin());
        }
        return SceneDependency(fn,args);
    }
};
}
