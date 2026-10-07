#include "pr_stage2_cd_stream_direct.h"
#include "pr_stage2_movie_cd_direct.h"
#include "pr_stage2_source_word.h"
#include <stdexcept>
namespace PrStage2CdStreamDirect {
namespace {int32_t S(uint32_t v){return v<0x80000000u?int32_t(v):int32_t(int64_t(v)-0x100000000LL);}}
int32_t Start800391AC(Services& s,uint32_t mode){
    // The source reserves eight stack bytes but initializes only byte zero.
    // Its lifetime spans the synchronous command and any retained waits.
    const uint8_t parameter=uint8_t(mode);
    const PrStage2CdParameters::View params{s,0u,&parameter,1u};
    PrStage2MovieCdDirect::ControlPrivate80036540(s,14u,params,0u);
    if(mode&0x100u){
        s.Write32(0x8008ECDCu,(mode&0x20u)?0u:1u);
        s.Call(0x80036930u,{0x80039318u});s.Call(0x80036528u,{0x80039240u});
    }
    return s.Call(0x80036540u,{27u,0u,0u});
}
int32_t Ready80039240(Services& s){return s.Call(0x80039670u,{});}
void Stop800392C0(Services& s){
    s.Call(0x80048A40u,{});
    s.Call(0x80036930u,{0u});s.Call(0x80036528u,{0u});
    // Reload each original register pointer after the callback setters.
    s.Write8(s.Read32(0x8005744Cu),0u);
    s.Write8(s.Read32(0x80057458u),0u);
    // ExitCriticalSection has no portable scalar result. Its caller discards
    // V0, so keep this operation void instead of inventing a BIOS return.
    s.CallVoid(0x80048A50u,{});
}
int32_t Dma80039318(Services& s){
    const uint32_t index=s.Read32(0x80095C54u),base=s.Read32(0x800965ACu),entry=base+(index<<5u);
    s.Write16(entry,2u);
    const auto location=PrStage2SourceWord::LoadUnaligned(s,entry+28u);
    PrStage2SourceWord::Write(s,0x8008A720u,location);
    PrStage2SourceWord::Write(s,0x8008A720u,location);
    uint32_t result=s.Read32(entry+8u);const uint32_t next=s.Read32(0x80095C50u),callback=s.Read32(0x800901D0u);
    s.Write32(0x8008A724u,result);s.Write32(0x80095C54u,next);
    if(callback)result=uint32_t(s.Call(callback,{}));
    s.Write32(0x80092858u,0u);return S(result);
}
bool TryCall(Services& s,uint32_t fn,std::initializer_list<uint32_t> args,int32_t& result){
    size_t n;
    switch(fn){case 0x800391ACu:n=1;break;case 0x80039318u:n=0;break;
    case 0x800392C0u:throw std::invalid_argument("CD stream stop requires CallVoid");
    case 0x80039240u:
        // The actual CD-ready callback ABI supplies two ignored arguments.
        if(args.size()!=0u&&args.size()!=2u)throw std::invalid_argument("CD ready callback argument count");
        result=Ready80039240(s);return true;
    default:return false;}
    if(args.size()!=n)throw std::invalid_argument("CD stream original argument count");
    if(fn==0x800391ACu)result=Start800391AC(s,*args.begin());
    else result=Dma80039318(s);
    return true;
}
bool TryCallVoid(Services& s,uint32_t fn,std::initializer_list<uint32_t> args){
    if(fn!=0x800392C0u)return false;
    if(args.size())throw std::invalid_argument("CD stream stop argument count");
    Stop800392C0(s);return true;
}
}
