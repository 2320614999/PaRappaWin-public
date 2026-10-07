#include "pr_stage2_vab_session.h"
#include "pr_stage2_vab_direct.h"
#include "pr_stage2_spu_boot_direct.h"

namespace PrStage2VabSession {
Session::Session(const std::filesystem::path& scus,const std::filesystem::path& overlay,
                 const std::filesystem::path& disc,D3D11Renderer& renderer,IAudioSink& sink)
    :Base(scus,overlay,disc,renderer,sink),transfer_(*this,Spu(),Interrupts()){}
int32_t Session::PlatformDependency(uint32_t fn,Args args){
    if(fn==0x80026E4Cu)eventsActive_=true;
    int32_t result;
    if(PrStage2VabDirect::TryCall(*this,fn,args,result))return result;
    if(PrStage2SpuBootDirect::TryCall(*this,fn,args,result))return result;
    const bool cardOrSpuEventOpen = fn==0x80048990u && args.size()==4u &&
        (args.begin()[0]==0xF0000009u || args.begin()[0]==0xF4000001u ||
         args.begin()[0]==0xF0000011u);
    if((eventsActive_ || cardOrSpuEventOpen || fn==0x80048A40u || fn==0x80048A50u ||
        (args.size()==1u && events_.Owns(args.begin()[0]))) &&
       (fn==0x80048990u || fn==0x80048A40u || fn==0x80048A50u ||
                         ((fn==0x800489A0u||fn==0x800489C0u||fn==0x800489D0u) &&
                          args.size()==1u && events_.Owns(args.begin()[0])) ) &&
       (fn==0x80048A50u ? (events_.TryCallVoid(fn,args), result=0, true) :
        events_.TryCall(fn,args,result))) return result;
    if(fn==0x8003623Cu){
        if(args.size())throw std::invalid_argument("Original video-mode read takes no argument");
        const uint32_t word=Read32(0x80057064u);
        return word<0x80000000u?int32_t(word):int32_t(int64_t(word)-0x100000000LL);
    }
    return RemainingDependency(fn,args);
}
void Session::CallLoadingDeviceVoid(uint32_t fn,Args args){
    if(PrStage2VabDirect::TryCallVoid(*this,fn,args))return;
    if(PrStage2SpuBootDirect::TryCallVoid(*this,fn,args))return;
    if((eventsActive_ || (fn==0x80048980u && args.size()==2u)) &&
       ((fn==0x80048980u && args.size()==2u) ||
                         (fn==0x80048A50u && args.size()==0u) ||
                         (fn==0x800489A0u && args.size()==1u && events_.Owns(args.begin()[0]))) &&
       events_.TryCallVoid(fn,args)) return;
    Base::CallLoadingDeviceVoid(fn,args);
}
bool Session::ServiceAdditionalDevices(){return transfer_.Service();}
bool Session::AdditionalInterruptsEnabled()const{return !eventsActive_||events_.InterruptsEnabled();}
uint8_t Session::ReadDevice8(uint32_t a){uint32_t v;if(transfer_.TryRead(a,1u,v))return uint8_t(v);return Base::ReadDevice8(a);}
uint16_t Session::ReadDevice16(uint32_t a){uint32_t v;if(transfer_.TryRead(a,2u,v))return uint16_t(v);return Base::ReadDevice16(a);}
uint32_t Session::ReadDevice32(uint32_t a){uint32_t v;if(transfer_.TryRead(a,4u,v))return v;return Base::ReadDevice32(a);}
void Session::WriteDevice8(uint32_t a,uint8_t v){if(!transfer_.TryWrite(a,1u,v))Base::WriteDevice8(a,v);}
void Session::WriteDevice16(uint32_t a,uint16_t v){if(!transfer_.TryWrite(a,2u,v))Base::WriteDevice16(a,v);}
void Session::WriteDevice32(uint32_t a,uint32_t v){if(!transfer_.TryWrite(a,4u,v))Base::WriteDevice32(a,v);}
void Session::AwaitDeviceProgress(uint32_t a){
    const uint32_t p=a&0x1FFFFFFFu;
    if(NativeIrqActive() &&
       ((p>=0x1F8010C0u&&p<0x1F8010CCu)||
        (p>=0x1F801D9Cu&&p<0x1F801DB0u)||p==0x1F801014u)){
        transfer_.Service();return;
    }
    Base::AwaitDeviceProgress(a);
}
}
