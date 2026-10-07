#include "pr_stage2_ring_direct.h"
#include <stdexcept>

namespace PrStage2RingDirect {
namespace {
int32_t S(uint32_t v){return v<0x80000000u?int32_t(v):int32_t(int64_t(v)-0x100000000LL);}
int32_t H(uint16_t v){return v<0x8000u?int32_t(v):int32_t(v)-65536;}
struct Output { Services& s;uint32_t address;Reply* local;
    bool Present()const{return address||local;}
    void Copy(uint32_t source){
        if(!Present())return;
        for(uint32_t i=0;i<8u;++i){const uint8_t b=s.Read8(source+i);if(local)local->bytes[i]=b;else s.Write8(address+i,b);}
        if(local)local->initialized=true;
    }
};
int32_t Ready(Services& s,uint32_t nonblocking,Output out){
    const uint32_t now=uint32_t(s.Call(0x80035560u,{0xFFFFFFFFu}));
    s.Write32(0x80088310u,now+960u);s.Write32(0x80088314u,0u);s.Write32(0x80088318u,0x80011DB8u);
    for(;;){
        const int32_t time=s.Call(0x80035560u,{0xFFFFFFFFu});
        bool timeout=S(s.Read32(0x80088310u))<time;
        if(!timeout){const uint32_t old=s.Read32(0x80088314u);s.Write32(0x80088314u,old+1u);timeout=S(old)>0x3C0000;}
        if(timeout){
            s.Call(0x80047F4Cu,{0x80011D28u});
            const uint32_t sync=s.Read8(0x800573D4u),ready=s.Read8(0x800573D5u),label=s.Read32(0x80088318u);
            const uint32_t rn=s.Read32(0x8005719Cu+4u*ready),cmd=s.Read8(0x80057119u);
            const uint32_t cn=s.Read32(0x8005711Cu+4u*cmd),sn=s.Read32(0x8005719Cu+4u*sync);
            s.Call(0x80047FFCu,{0x80011D38u,label,cn,sn,rn});s.Call(0x80037A8Cu,{});return -1;
        }
        if(s.Call(0x80035898u,{})!=0){
            const uint8_t bank=s.Read8(s.Read32(0x800573BCu))&3u;
            for(;;){
                const uint32_t events=uint32_t(s.Call(0x80036AF8u,{}));if(!events)break;
                if(events&4u){const uint32_t cb=s.Read32(0x800570FCu);if(cb)s.Call(cb,{s.Read8(0x800573D5u),0x80088300u});}
                if(events&2u){const uint32_t cb=s.Read32(0x800570F8u);if(cb)s.Call(cb,{s.Read8(0x800573D4u),0x800882F8u});}
            }
            s.Write8(s.Read32(0x800573BCu),bank);
        }
        uint32_t result=s.Read8(0x800573D6u);
        if(result){s.Write8(0x800573D6u,0u);out.Copy(0x80088308u);return int32_t(result);}
        result=s.Read8(0x800573D5u);
        if(result){s.Write8(0x800573D5u,0u);out.Copy(0x80088300u);return int32_t(result);}
        if(nonblocking)return 0;
        s.AwaitDeviceProgress(0x1F801800u);
    }
}
uint32_t AdvanceInput(Services& s){const uint32_t n=s.Read32(0x80092908u)+1u;s.Write32(0x80092908u,n);return n;}
void ResetPartial(Services& s){
    const uint32_t first=s.Read32(0x80095C54u),end=s.Read32(0x80095C50u);
    s.Write32(0x8008ECA4u,0u);s.Write16(0x8008ECD4u,0u);
    ClearSlots8003954C(s,first,end-first);
    const uint32_t old=s.Read32(0x80095C54u),entry=s.Read32(0x8008A728u);
    s.Write32(0x80095C50u,old);s.Write16(entry,0u);
}
}
Reply ReadyPrivate800372F0(Services& s,uint32_t mode){Reply reply;reply.result=Ready(s,mode,{s,0u,&reply});return reply;}
int32_t Ready800372F0(Services& s,uint32_t mode,uint32_t output){return Ready(s,mode,{s,output,nullptr});}
int32_t Ready800364F0(Services& s,uint32_t mode,uint32_t output){return Ready800372F0(s,mode,output);}
void CopyWords80039FE0(Services& s,uint32_t destination,uint32_t source,uint32_t words){
    for(uint32_t i=0;i<words;++i){const auto word=PrStage2SourceWord::Read(s,source+4u*i);PrStage2SourceWord::Write(s,destination+4u*i,word);}
}
void ClearSlots8003954C(Services& s,uint32_t first,uint32_t count){
    for(uint32_t i=0;i<count;++i){const uint32_t base=s.Read32(0x800965ACu);s.Write32(base+((first+i)<<5u),0u);}
}
int32_t StartDma8003A014(Services& s,uint32_t channel,uint32_t destination,uint32_t blocks,uint32_t words,uint32_t control,uint32_t interrupt){
    const uint32_t port=0x1F801088u+(channel<<4u);
    if(s.Read32(port)&0x01000000u){
        uint32_t n=0;
        for(;;){
            if(n==0x10000u){s.Call(0x80047FFCu,{0x8001206Cu,s.Read32(port)});break;}
            const uint32_t busy=s.Read32(port);++n;
            if(!(busy&0x01000000u))break;
            s.AwaitDeviceProgress(port);
        }
    }
    const uint32_t bit=1u<<(channel&31u),dicr=s.Read32(0x800574D8u);
    auto* bus=dynamic_cast<Bus*>(&s);
    if(!bus)throw std::logic_error("Original on-die SB bus transport is not bound");
    const uint8_t old=bus->LoadOnDieByte(dicr+2u);
    bus->StoreOnDieByte(dicr+2u,uint8_t(interrupt)==1u?old|bit:old&~bit);
    (void)s.Read32(s.Read32(0x800574D8u)); // original readback, not completion
    const uint32_t dpcr=s.Read32(0x800574D4u),priority=s.Read32(dpcr);
    s.Write32(dpcr,priority|(1u<<((channel*4u+3u)&31u)));
    s.Write32(port-8u,destination);s.Write32(port-4u,(blocks<<16u)|words);
    const uint32_t status=s.Read32(0x800574BCu);
    while(!(s.Read8(status)&64u))s.AwaitDeviceProgress(0x1F801800u);
    s.Write32(port,control);return S(s.Read32(port));
}
int32_t Receive80039670(Services& s,PrStage2SourceWord::Word location){
    uint32_t result=s.Read32(0x80092858u);if(result==1u)return 1;
    if(s.Read32(0x8008ECD8u)!=0u&&(s.Read32(s.Read32(0x800574DCu))&0x01000000u)){
        result=s.Read32(0x80096594u);s.Write32(0x80091640u,1u);
        if(result)result=AdvanceInput(s);s.Write32(0x80057504u,1u);return S(result);
    }
    const Reply ready=ReadyPrivate800372F0(s,1u);
    if(ready.result==5)return 5;
    if(!ready.initialized)throw std::logic_error("Original CD ready output was not initialized");
    if(ready.bytes[0]&4u){s.Write32(0x80057504u,3u);return 3;}
    const uint32_t index=s.Read32(0x80095C50u),base=s.Read32(0x800965ACu);
    s.Write32(0x8008A728u,base+(index<<5u));
    if(s.Read16(base+(index<<5u))){if(s.Read32(0x80096594u))AdvanceInput(s);s.Write32(0x80057504u,4u);return 4;}
    s.Write8(s.Read32(0x800574BCu),0u);s.Write8(s.Read32(0x800574C8u),0u);
    s.Write8(s.Read32(0x800574BCu),0u);s.Write8(s.Read32(0x800574C8u),128u);
    s.Write32(s.Read32(0x800574CCu),0x20943u);s.Write32(s.Read32(0x800574D0u),0x1323u);
    if(!s.Read32(0x8008ECDCu)){
        location={0u,15u};
        for(uint32_t i=0;i<4u;++i)location.value|=uint32_t(s.Read8(s.Read32(0x800574C4u)))<<(8u*i);
        const uint32_t data=s.Read32(0x800574C4u);for(uint32_t i=0;i<8u;++i)(void)s.Read8(data);
    }
    uint32_t memory=s.Read32(0x80096594u);
    if(memory){const uint32_t n=s.Read32(0x80092908u),entry=s.Read32(0x8008A728u);CopyWords80039FE0(s,entry,memory+(n<<11u),8u);}
    else StartDma8003A014(s,3u,s.Read32(0x8008A728u),0u,8u,0x11000000u,0u);
    const uint32_t dma=s.Read32(0x800574ECu);
    while(s.Read32(dma)&0x01000000u)s.AwaitDeviceProgress(dma);
    PrStage2SourceWord::StoreUnaligned(s,s.Read32(0x8008A728u)+28u,location);
    s.Write32(s.Read32(0x800574CCu),0x20843u);s.Write32(s.Read32(0x800574D0u),0x1325u);
    if(s.Read32(0x800965A4u)==1u){
        const uint32_t target=s.Read32(0x80091724u);
        if(target){const uint32_t entry=s.Read32(0x8008A728u);
            if(target!=s.Read16(entry+8u)){s.Write16(entry,0u);result=s.Read32(0x80096594u);return S(result?AdvanceInput(s):result);}
            s.Write32(0x800965A4u,0u);
        }
    }
    uint32_t entry=s.Read32(0x8008A728u);
    bool valid=s.Read16(entry)==0x160u;
    if(valid){const uint16_t type=s.Read16(entry+2u);valid=((type>>10u)&31u)==s.Read32(0x800917E0u);}
    if(!valid){if(s.Read32(0x80096594u))s.Write32(0x80092908u,0u);else(void)s.Read16(entry);
        entry=s.Read32(0x8008A728u);s.Write32(0x80057504u,5u);s.Write16(entry,0u);return 5;}
    const int32_t expected=H(s.Read16(0x8008ECD4u));const uint16_t actual=s.Read16(entry+4u);
    bool mismatch=expected!=int32_t(actual);
    if(!mismatch){const uint32_t frame=s.Read32(0x8008ECA4u);mismatch=frame&&frame!=s.Read16(entry+8u);}
    if(mismatch){ResetPartial(s);if(s.Read32(0x80096594u))AdvanceInput(s);s.Write32(0x80057504u,6u);return 6;}
    entry=s.Read32(0x8008A728u);
    if(!s.Read16(entry+4u)){
        const uint16_t frame=s.Read16(entry+8u);const uint32_t limit=s.Read32(0x8009659Cu);
        s.Write16(0x8008ECD4u,0u);s.Write32(0x8008ECA4u,frame);
        if(limit&&frame>=limit){
            ResetPartial(s);const uint32_t cb=s.Read32(0x800901D4u);s.Write32(0x800965A4u,1u);if(cb)s.Call(cb,{});
            if(s.Read32(0x80096594u))AdvanceInput(s);s.Write32(0x80057504u,7u);return 7;
        }
        const uint32_t size=s.Read32(0x801C3868u),tail=s.Read32(0x80095C50u);entry=s.Read32(0x8008A728u);
        if(size-tail-1u<s.Read16(entry+6u)){
            if(!s.Read32(0x8009659Cu)){
                s.Write16(entry,1u);const uint32_t cb=s.Read32(0x800901D4u);s.Write32(0x800965A4u,1u);if(cb)s.Call(cb,{});
                if(s.Read32(0x80096594u))AdvanceInput(s);s.Write32(0x80057504u,8u);return 8;
            }
            if(s.Read16(s.Read32(0x800965ACu))){s.Write16(entry,0u);if(s.Read32(0x80096594u))AdvanceInput(s);s.Write32(0x80057504u,9u);return 9;}
            s.Write16(entry,1u);
            const uint32_t to=s.Read32(0x800965ACu),from=s.Read32(0x8008A728u);
            s.Write32(0x80095C50u,0u);CopyWords80039FE0(s,to,from,8u);
            s.Write32(0x8008A728u,s.Read32(0x800965ACu));
        }
        s.Write32(0x80095C54u,s.Read32(0x80095C50u));
    }
    s.Write32(0x80057504u,10u);
    const uint16_t part=s.Read16(0x8008ECD4u);const uint32_t slots=s.Read32(0x801C3868u),slot=s.Read32(0x80095C50u);
    s.Write16(0x8008ECD4u,uint16_t(part+1u));const uint32_t ring=s.Read32(0x800965ACu);
    const uint32_t synchronous=s.Read32(0x8008ECD8u);
    s.Write32(0x800965A8u,ring+(slots<<5u)+slot*2016u);
    uint32_t control=0x11000000u;
    if(synchronous){s.Write32(s.Read32(0x800574CCu),0x20943u);s.Write32(s.Read32(0x800574D0u),0x1323u);}
    else{control=0x11400100u;s.Write32(s.Read32(0x800574CCu),0x21020843u);}
    entry=s.Read32(0x8008A728u);const uint32_t count=s.Read16(entry+6u),number=s.Read16(entry+4u);
    const bool last=count-1u==number;
    memory=s.Read32(0x80096594u);
    if(last)s.Write32(0x80092858u,1u);
    if(memory){const uint32_t n=s.Read32(0x80092908u),to=s.Read32(0x800965A8u);CopyWords80039FE0(s,to,memory+(n<<11u)+32u,504u);AdvanceInput(s);}
    else StartDma8003A014(s,3u,s.Read32(0x800965A8u),0u,504u,control,last?1u:0u);
    if(last){const uint32_t next=s.Read32(0x80091720u);s.Write16(0x8008ECD4u,0u);s.Write32(0x8008ECA4u,0u);s.Write32(0x800917E0u,next);}
    s.Write32(s.Read32(0x800574D0u),0x1325u);s.Write16(s.Read32(0x8008A728u),3u);
    result=s.Read32(0x80095C50u);memory=s.Read32(0x80096594u);s.Write32(0x80095C50u,++result);
    if(memory){result=s.Read32(0x80092858u);if(result)return s.Call(0x80039318u,{});}
    return S(result);
}
bool TryCall(Services& s,uint32_t fn,std::initializer_list<uint32_t> args,int32_t& result){
    size_t n;switch(fn){case 0x80039670u:n=0;break;case 0x800364F0u:case 0x800372F0u:n=2;break;case 0x8003A014u:n=7;break;default:return false;}
    if(args.size()!=n)throw std::invalid_argument("Ring source argument count");const auto a=args.begin();
    if(fn==0x80039670u)result=Receive80039670(s);
    else if(fn==0x8003A014u)result=StartDma8003A014(s,a[0],a[1],a[2],a[3],a[4],a[5]);
    else result=Ready800372F0(s,a[0],a[1]);return true;
}
bool TryCallVoid(Services& s,uint32_t fn,std::initializer_list<uint32_t> args){
    const auto a=args.begin();
    if(fn==0x80039FE0u){if(args.size()!=3u&&args.size()!=4u)throw std::invalid_argument("Copy word count");CopyWords80039FE0(s,a[0],a[1],a[2]);return true;}
    if(fn==0x8003954Cu){if(args.size()!=2u)throw std::invalid_argument("Clear slot count");ClearSlots8003954C(s,a[0],a[1]);return true;}
    int32_t discarded;return TryCall(s,fn,args,discarded);
}
}
