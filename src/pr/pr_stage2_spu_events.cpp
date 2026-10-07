#include "pr_stage2_spu_events.h"
#include <stdexcept>

namespace PrStage2SpuEvents {
Controller::Controller(bool enabled):owner_(std::this_thread::get_id()),interrupts_(enabled){}
void Controller::Owner()const{if(owner_!=std::this_thread::get_id())throw std::logic_error("SPU event called from a foreign owner thread");}
bool Controller::InterruptsEnabled()const{Owner();return interrupts_;}
int32_t Controller::EnterCritical(){Owner();const int32_t previous=interrupts_?1:0;interrupts_=false;return previous;}
void Controller::ExitCritical(){Owner();interrupts_=true;}
Controller::Event* Controller::Find(uint32_t handle){
    const uint32_t slot=handle-0xF1000000u;
    return slot<events_.size()&&events_[slot].state!=State::Unused?&events_[slot]:nullptr;
}
bool Controller::Owns(uint32_t handle) const {
    Owner();
    const uint32_t slot=handle-0xF1000000u;
    return slot<events_.size()&&events_[slot].state!=State::Unused;
}
uint32_t Controller::Open(uint32_t eventClass,uint32_t spec,uint32_t mode,uint32_t callback){
    Owner();
    if(interrupts_)throw std::logic_error("Source OpenEvent requires its critical-section owner");
    // The same BIOS event seam is used by SPU, software CARD, and hardware
    // CARD notifications. Keep the event tuple opaque after validating the
    // classes/mode used by the original S2 image.
    if(eventClass!=0xF0000009u&&eventClass!=0xF4000001u&&eventClass!=0xF0000011u)
        throw std::logic_error("Unsupported S2 BIOS event class");
    if(mode!=0x2000u||callback!=0u)
        throw std::logic_error("Unsupported S2 BIOS event mode");
    for(uint32_t i=0;i<events_.size();++i)if(events_[i].state==State::Unused){
        events_[i]={State::Disabled,eventClass,spec};++opened_;return 0xF1000000u+i;
    }
    return 0xFFFFFFFFu;
}
int32_t Controller::Close(uint32_t handle){Owner();if(auto* e=Find(handle))*e={};return 1;}
int32_t Controller::Enable(uint32_t handle){Owner();if(auto* e=Find(handle))e->state=State::Busy;return 1;}
int32_t Controller::Disable(uint32_t handle){Owner();if(auto* e=Find(handle))e->state=State::Disabled;return 1;}
int32_t Controller::Test(uint32_t handle){
    Owner();auto* e=Find(handle);
    if(!e)throw std::logic_error("Unowned SPU event descriptor");
    if(e->state!=State::Ready)return 0;
    e->state=State::Busy;++consumed_;return 1;
}
void Controller::Deliver(uint32_t eventClass,uint32_t spec){
    Owner();++deliveries_;
    for(auto& e:events_)if(e.state==State::Busy&&e.eventClass==eventClass&&e.spec==spec)e.state=State::Ready;
}
bool Controller::TryCall(uint32_t fn,std::initializer_list<uint32_t> a,int32_t& result){
    size_t arity;
    switch(fn){
    case 0x80048A40u:arity=0;break;
    case 0x80048990u:arity=4;break;
    case 0x800489A0u:case 0x800489C0u:case 0x800489D0u:arity=1;break;
    case 0x80048A50u:case 0x80048980u:throw std::logic_error("Void BIOS event operation requires CallVoid");
    default:return false;
    }
    if(a.size()!=arity)throw std::invalid_argument("SPU event source argument count mismatch");
    const auto v=a.begin();
    switch(fn){
    case 0x80048A40u:result=EnterCritical();break;
    case 0x80048990u:{const uint32_t h=Open(v[0],v[1],v[2],v[3]);result=h<0x80000000u?int32_t(h):int32_t(int64_t(h)-0x100000000LL);break;}
    case 0x800489A0u:result=Close(v[0]);break;
    case 0x800489C0u:result=Test(v[0]);break;
    case 0x800489D0u:result=Enable(v[0]);break;
    }
    return true;
}
bool Controller::TryCallVoid(uint32_t fn,std::initializer_list<uint32_t> a){
    if(fn==0x80048A50u){if(a.size())throw std::invalid_argument("ExitCritical argument count");ExitCritical();return true;}
    if(fn==0x80048980u){if(a.size()!=2u)throw std::invalid_argument("DeliverEvent argument count");Deliver(a.begin()[0],a.begin()[1]);return true;}
    int32_t ignored;return TryCall(fn,a,ignored);
}
}
