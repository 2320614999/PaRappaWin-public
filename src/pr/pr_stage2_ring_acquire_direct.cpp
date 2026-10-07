#include "pr_stage2_ring_acquire_direct.h"
#include <stdexcept>

namespace PrStage2RingAcquireDirect {
namespace {
int32_t S(uint32_t v){return v<0x80000000u?int32_t(v):int32_t(int64_t(v)-0x100000000LL);}
uint32_t Asr(uint32_t v,unsigned n){return uint32_t(int64_t(S(v))>>n);}
template<class Out>int32_t Acquire(Services& s,Out output){
    const uint32_t index=s.Read32(0x80095C58u),base=s.Read32(0x800965ACu);
    uint32_t entry=base+(index<<5u);
    if(s.Read16(entry)==1u){
        const uint32_t end=s.Read32(0x8009659Cu);s.Write32(0x80095C58u,0u);
        if(end)s.Write16(entry,0u);
        const uint32_t next=s.Read32(0x80095C58u),ring=s.Read32(0x800965ACu);entry=ring+(next<<5u);
    }
    if(s.Read16(entry)!=2u)return 1;
    s.Write16(entry,4u);
    const uint32_t slots=s.Read32(0x801C3868u),ring=s.Read32(0x800965ACu),next=s.Read32(0x80095C58u);
    output(ring+(slots<<5u)+next*2016u,entry);return 0;
}
}
Frame AcquirePrivate8003958C(Services& s){
    Frame out;Acquire(s,[&](uint32_t payload,uint32_t entry){out={payload,entry,true};});return out;
}
int32_t Acquire8003958C(Services& s,uint32_t payloadOut,uint32_t metadataOut){
    return Acquire(s,[&](uint32_t payload,uint32_t entry){s.Write32(payloadOut,payload);s.Write32(metadataOut,entry);});
}
void Locate80039650(Services& s,uint32_t seek,uint32_t first,uint32_t last){
    s.Write32(0x800965A4u,seek);s.Write32(0x80091724u,first);s.Write32(0x8009659Cu,last);
}
void SetStream80039408(Services& s,uint32_t mode,uint32_t first,uint32_t last,uint32_t callback,uint32_t loop){
    Locate80039650(s,1u,first,last);s.Write32(0x80096594u,0u);s.Write32(0x800901D0u,callback);
    s.Write32(0x8008ECD8u,mode&1u);s.Write32(0x800917E0u,0u);s.Write32(0x80091720u,0u);
    s.Write16(0x8008ECD4u,0u);s.Write32(0x8008ECA4u,0u);s.Write32(0x800901D4u,loop);
}
int32_t VlcSize80047B00(Services& s,uint32_t size){
    // Original ADDI, not ADDIU. Do not invoke host signed-overflow UB or
    // silently execute a branch for the source's arithmetic-exception input.
    if(size==0x80000000u)throw std::overflow_error("Original VLC size ADDI overflow");
    const uint32_t old=s.Read32(0x80047AFCu);
    s.Write32(0x80047AFCu,S(size-1u)<=0?0xFFFFFFu:size<<1u);return S(old);
}
int32_t DecodeNext800273A4(Services& s){
    const Frame frame=AcquirePrivate8003958C(s);
    uint32_t payload=0;
    if(frame.available){
        const uint32_t entry=frame.metadata,control=s.Read32(0x8006ED78u);
        const uint32_t width=s.Read16(entry+16u);
        bool changed=s.Read32(control+8u)!=width;
        if(!changed){const uint32_t height=s.Read16(entry+18u);changed=s.Read32(control+12u)!=height;}
        if(changed){s.Write32(control+8u,width);s.Write32(control+12u,s.Read16(entry+18u));}
        const uint32_t count=s.Read32(0x8006ED88u);payload=frame.payload;s.Write32(0x8006ED88u,count+1u);
    }
    if(!payload){const uint32_t control=s.Read32(0x8006ED78u);s.Write32(control+28u,0u);return S(control);}
    uint32_t control=s.Read32(0x8006ED78u);const uint32_t vlc=s.Read32(control);
    s.Write32(control+28u,1u);
    const int32_t decoded=s.Call(0x80047B30u,{payload,vlc});
    control=s.Read32(0x8006ED78u);s.Write32(control+16u,uint32_t(decoded));
    s.CallVoid(0x80039490u,{payload});
    control=s.Read32(0x8006ED78u);const uint32_t status=s.Read32(control+16u);
    if(status)return S(status);
    s.CallVoid(0x80047558u,{s.Read32(control),2u});
    control=s.Read32(0x8006ED78u);const uint32_t height=s.Read32(control+12u),width=s.Read32(control+8u);
    const uint32_t rounded=S(height+15u)<0?height+30u:height+15u;
    const uint32_t product=(width<<4u)*Asr(rounded,4u),image=s.Read32(control+4u);
    s.CallVoid(0x800475D4u,{image,Asr(product,1u)});
    control=s.Read32(0x8006ED78u);s.Write32(control+32u,1u);return S(control);
}
int32_t InitializeDecoder800274D4(Services& s){
    const uint32_t control=s.Read32(0x8006ED78u);
    s.Write32(control+16u,0u);s.Write32(control+28u,0u);s.Write32(control+32u,0u);
    SetStream80039408(s,0u,1u,0xFFFFFFFFu,0u,0u);VlcSize80047B00(s,0u);
    const int32_t result=DecodeNext800273A4(s);s.Write32(0x8006ED88u,0u);return result;
}
bool TryCall(Services& s,uint32_t fn,std::initializer_list<uint32_t> args,int32_t& result){
    size_t n;switch(fn){case 0x8003958Cu:n=2;break;case 0x80047B00u:n=1;break;case 0x800273A4u:case 0x800274D4u:n=0;break;default:return false;}
    if(args.size()!=n)throw std::invalid_argument("Ring consumer argument count");const auto a=args.begin();
    if(fn==0x8003958Cu)result=Acquire8003958C(s,a[0],a[1]);
    else if(fn==0x80047B00u)result=VlcSize80047B00(s,a[0]);
    else if(fn==0x800273A4u)result=DecodeNext800273A4(s);
    else result=InitializeDecoder800274D4(s);return true;
}
bool TryCallVoid(Services& s,uint32_t fn,std::initializer_list<uint32_t> args){
    const auto a=args.begin();
    if(fn==0x80039408u){if(args.size()!=5u)throw std::invalid_argument("SetStream argument count");SetStream80039408(s,a[0],a[1],a[2],a[3],a[4]);return true;}
    if(fn==0x80039650u){if(args.size()!=3u)throw std::invalid_argument("Stream range argument count");Locate80039650(s,a[0],a[1],a[2]);return true;}
    int32_t ignored;return TryCall(s,fn,args,ignored);
}
}
