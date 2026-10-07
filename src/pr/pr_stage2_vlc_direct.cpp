#include "pr_stage2_vlc_direct.h"
#include <stdexcept>

namespace PrStage2VlcDirect {
namespace {
int32_t S(uint32_t v){return v<0x80000000u?int32_t(v):int32_t(int64_t(v)-0x100000000LL);}
uint32_t Add(uint32_t a,uint32_t b){const int64_t v=int64_t(S(a))+S(b);if(v<INT32_MIN||v>INT32_MAX)throw std::overflow_error("Original VLC signed ADD/ADDI overflow");return uint32_t(v);}
uint32_t Sub(uint32_t a,uint32_t b){const int64_t v=int64_t(S(a))-S(b);if(v<INT32_MIN||v>INT32_MAX)throw std::overflow_error("Original VLC signed SUB overflow");return uint32_t(v);}
uint32_t Asr(uint32_t a,uint32_t n){n&=31u;return !n?a:(a>>n)|((a&0x80000000u)?(~0u<<(32u-n)):0u);}
struct State {
    Services& s;uint32_t input,output,bits=0,offset=0,quant=0,block=0,cr=0,cb=0,y=0,limit=0;
    uint32_t Read(){const uint16_t v=s.Read16(input);input=Add(input,2u);return v;}
    void Refill(uint32_t length){offset=Add(offset,length);const bool refill=(offset&16u)!=0u;offset&=15u;if(refill)bits|=Read()<<offset;}
    void Consume(uint32_t length){bits<<=(length&31u);Refill(length);}
    void Store(){
        s.Write32(0x80047E70u,input);s.Write32(0x80047E74u,output);
        s.Write32(0x80047E78u,bits);s.Write32(0x80047E7Cu,offset);
        s.Write32(0x80047E80u,quant);s.Write32(0x80047E84u,block);
        s.Write32(0x80047E8Cu,cr);s.Write32(0x80047E90u,cb);s.Write32(0x80047E94u,y);
    }
};
}
int32_t Decode80047B30(Services& s,CacheControl& cache,uint32_t source,uint32_t destination){
    const uint32_t size=s.Read32(0x80047AFCu);
    State v{s,source,destination};bool ac=source==0u;
    if(ac){
        v.input=s.Read32(0x80047E70u);v.output=s.Read32(0x80047E74u);
        v.bits=s.Read32(0x80047E78u);v.offset=s.Read32(0x80047E7Cu);
        v.quant=s.Read32(0x80047E80u);v.block=s.Read32(0x80047E84u);
        v.cr=s.Read32(0x80047E8Cu);v.cb=s.Read32(0x80047E90u);v.y=s.Read32(0x80047E94u);
        v.limit=Add(v.output,Add(size,size));
    }else{
        v.limit=Add(destination,Add(size,size));
        const uint32_t first=s.Read16(source),second=s.Read16(source+2u),quant=s.Read16(source+4u),version=s.Read16(source+6u);
        const uint32_t high=s.Read16(source+8u),low=s.Read16(source+10u);
        v.quant=quant<<10u;v.block=version>=3u?1u:0u;v.input=Add(source,12u);v.bits=(high<<16u)|low;
        s.Write16(destination,uint16_t(first));s.Write16(destination+2u,uint16_t(second));v.output=Add(destination,2u);
    }
    // Valid streams terminate at the original DC sentinel. This host safety
    // bound rejects corrupt/infinite source tables without a success result.
    for(uint32_t guard=0;guard<0x200000u;++guard){
        if(!ac){
            const uint32_t code=v.bits>>22u;v.output=Add(v.output,2u);
            if(code==(v.block?0x3FFu:0x1FFu)){
                for(uint32_t i=0;i<65u;++i){s.Write16(v.output,0xFE00u);v.output=Add(v.output,2u);}
                cache.SetSwapCacheBit80047E30();return 0;
            }
            uint32_t dc;
            if(v.block){
                const uint32_t table=(v.block<3u?0x8005DD98u:0x8005D998u)+(v.bits>>24u)*4u;
                const uint32_t prefix=s.Read16(table),length=s.Read16(table+2u);
                v.bits<<=(prefix&31u);uint32_t delta=0u;
                if(length){
                    const uint32_t shift=Sub(32u,length);delta=v.bits>>(shift&31u);
                    const bool positive=S(v.bits)<0;v.bits<<=(length&31u);
                    if(!positive)delta=Sub(delta,0xFFFFFFFFu>>(shift&31u));
                    v.offset=Add(v.offset,length);
                }
                v.Refill(prefix);
                if(v.block==1u)v.cr=dc=Add(v.cr,delta);
                else if(v.block==2u)v.cb=dc=Add(v.cb,delta);
                else v.y=dc=Add(v.y,delta);
                dc=((dc<<2u)&1023u)|v.quant;
                v.block=Add(v.block,1u);s.Write16(v.output,uint16_t(dc));
                if(v.block==7u)v.block=Add(v.block,0xFFFFFFFAu);
            }else{
                v.Consume(10u);dc=v.quant|code;s.Write16(v.output,uint16_t(dc));
            }
            const bool full=S(v.output-v.limit)>=0;v.output=Add(v.output,2u);
            if(full){v.Store();return 1;}
            ac=true;
        }
        const uint32_t fast=0x8005E198u+(v.bits>>19u)*8u;
        uint32_t packed=s.Read32(fast),extra;
        if(!packed){v.Consume(8u);packed=s.Read32(0x8006E198u+(v.bits>>23u)*4u);extra=0u;}
        else extra=s.Read32(fast+4u);
        v.Consume(packed&255u);
        const uint32_t words[3]={packed>>16u,extra&65535u,extra>>16u};
        const uint32_t count=!extra?1u:words[2]?3u:2u;
        for(uint32_t i=0;i<count;++i){
            const uint32_t word=words[i];
            if(word==0x7C1Fu){
                s.Write16(v.output,uint16_t(v.bits>>16u));v.output=Add(v.output,2u);
                const uint32_t next=v.Read();v.bits=(v.bits<<16u)|(next<<(v.offset&31u));break;
            }
            s.Write16(v.output,uint16_t(word));
            if(word==0xFE00u){ac=false;break;}
            v.output=Add(v.output,2u);
        }
    }
    throw std::runtime_error("Original VLC failed to reach a bounded decode boundary");
}
int32_t ReleaseFrame80039490(Services& s,uint32_t payload){
    const uint32_t slots=s.Read32(0x801C3868u),ring=s.Read32(0x800965ACu);
    const uint32_t delta=payload-(ring+(slots<<5u)),quarter=Asr(delta,2u);
    const int64_t product=int64_t(S(quarter))*int64_t(S(0x82082083u));
    const uint32_t high=uint32_t(uint64_t(product)>>32u);
    const uint32_t index=Asr(high+quarter,8u)-Asr(delta,31u),entry=ring+(index<<5u);
    const uint16_t state=s.Read16(entry),rawCount=s.Read16(entry+6u);
    if(state!=4u)return 1;
    const int32_t count=rawCount<32768u?int32_t(rawCount):int32_t(rawCount)-65536;
    uint32_t done=0;
    while(S(done)<count){const uint32_t at=index+done;++done;s.Write16(s.Read32(0x800965ACu)+(at<<5u),0u);}
    s.Write32(0x80095C58u,index+done);return 0;
}
}
