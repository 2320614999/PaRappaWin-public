#include "pr_stage2_retry_direct.h"

namespace PrStage2RetryDirect {
namespace {
constexpr uint32_t Gp = 0x8006EA40u;
int32_t S(uint32_t x) { return x<0x80000000u?int32_t(x):int32_t(int64_t(x)-0x100000000LL); }
uint32_t WorkTable(Services& s) { return 0x80087288u+20u*s.Read32(Gp+872u); }
}
int32_t Initialize800267C8(Services& s,uint32_t table) {
    const uint32_t context=s.Read32(table+16u);
    s.Write32(Gp+812u,2u);
    s.Write32(Gp+808u,0u);
    s.Write32(context,0xFFFFFFFFu);
    return -1;
}
int32_t ChangedPad80026744(Services& s) {
    const uint32_t pad=uint32_t(s.Call(0x80035510u,{1u}));
    if(pad==s.Read32(Gp+816u)) return 0;
    s.Write32(Gp+816u,pad);
    return S(pad);
}
void InputSound80025C8C(Services& s,uint32_t mask) {
    uint32_t pointer;
    switch(mask) {
    case 0x1000u:case 0x2000u:case 0x4000u:case 0x8000u:pointer=0x80094420u;break;
    case 0x40u:pointer=0x80094424u;break;
    case 0x20u:pointer=0x80094428u;break;
    case 0x100u:pointer=0x8009442Cu;break;
    default:return; // Caller discards v0, including the unchanged zero-input v0.
    }
    s.Call(0x80026EF8u,{s.Read32(pointer)});
    s.Call(0x80026ECCu,{});
}
int32_t Choose80025F0C(Services& s,uint32_t pad,uint32_t context) {
    if(pad==0x40u) {
        InputSound80025C8C(s,0x20u);
        s.Write32(context,0u);
        return 1;
    }
    if(pad==0x20u) {
        InputSound80025C8C(s,0x40u);
        s.Write32(context,1u);
        return 2;
    }
    return 0;
}
int32_t TickCue80025E6C(Services& s) {
    const int32_t remaining=S(s.Read32(Gp+812u));
    if(remaining<=0) return remaining;
    const uint32_t frame=s.Read32(Gp+808u);
    if(frame==0u || frame==36u) {
        s.Call(0x80026EF8u,{s.Read32(0x8009441Cu)+(frame==36u?6u:0u)});
        s.Call(0x80026ECCu,{});
        if(frame==36u) s.Write32(Gp+812u,s.Read32(Gp+812u)-1u);
    }
    const uint32_t current=s.Read32(Gp+808u),next=current+1u;
    s.Write32(Gp+808u,current==72u?0u:next);
    return S(next);
}
int32_t DrawPrompt800203D4(Services& s,int32_t choice) {
    s.Call(0x8001C550u,{56u,57u,0x80050950u,0u});
    s.Call(0x8001C550u,{70u,149u,choice==0?0x80050980u:0x80050960u,0u});
    return s.Call(0x8001C550u,{178u,152u,choice==1?0x80050990u:0x80050970u,0u});
}
int32_t DrawRetryFrame8001E750(Services& s,uint32_t context) {
    const uint32_t buffer=uint32_t(s.Call(0x8004019Cu,{}));
    const uint32_t packet=s.Read32(0x8006ED50u+4u*buffer);
    s.Write32(Gp+872u,buffer);
    s.CallVoid(0x80040F90u,{packet});
    s.Call(0x80040CC8u,{0u,0u,WorkTable(s)});
    const uint32_t state=s.Read32(Gp+908u);
    if(state==0u) return DrawPrompt800203D4(s,S(s.Read32(context)));
    if(state==1u) {
        s.Call(0x8001B6C4u,{0u,0u,320u,240u,0x400F0F0Fu,0u,WorkTable(s)});
        const uint32_t next=s.Read32(Gp+908u)+1u;
        s.Write32(Gp+908u,next);
        return S(next);
    }
    const int32_t result=s.Call(0x8001B120u,{0u});
    s.Write32(Gp+908u,0u);
    return result;
}
int32_t EndFrame8001EA00(Services& s,uint32_t event) {
    s.Call(0x80040370u,{});
    if(event!=4u && event!=1u && event!=10u) s.Call(0x80040420u,{0u,0u,70u});
    return s.Call(0x80040CA4u,{WorkTable(s)});
}
int32_t PadVsync80026E2C(Services& s) { return s.Call(0x80026ECCu,{}); }
int32_t FlushDebugText800436F0() {
    // This SCUS entry really is `jr ra; move v0,zero`. No font owner, packet,
    // rendering or completion is being substituted by a platform success.
    return 0;
}
int32_t RunRetry80026B94(Services& s,uint32_t argument) {
    constexpr uint32_t table=0x80054564u;
    const uint32_t init=s.Read32(table);
    if(init==0x800267C8u) Initialize800267C8(s,table);
    else s.Call(init,{table,4u,argument});
    s.Call(0x800357D4u,{0x80026E2Cu});
    while(s.Call(0x80035510u,{1u})!=0) s.AwaitPadRelease80035510();
    uint32_t accepting=1u,untouched=1u,timeout=s.Read32(table+12u),tail=60u;
    int32_t result=0;
    s.Write32(Gp+816u,0u);
    do {
        if(accepting) {
            const uint32_t pad=uint32_t(ChangedPad80026744(s));
            if(pad) {
                untouched=0u;
                const uint32_t handler=s.Read32(table+4u),context=s.Read32(table+16u);
                result=handler==0x80025F0Cu?Choose80025F0C(s,pad,context)
                    :s.Call(handler,{pad,context,argument});
                if(result) accepting=0u;
            }
        } else --tail;
        if(S(s.Read32(table+12u))>0 && untouched) {
            --timeout;
            if(S(timeout)<=0) { result=3;accepting=0u; }
        }
        const uint32_t tick=s.Read32(table+8u);
        if(tick==0x80025E6Cu) TickCue80025E6C(s);
        else if(tick) s.Call(tick,{table});
        DrawRetryFrame8001E750(s,s.Read32(table+16u));
        s.Call(0x80035560u,{0u});
        EndFrame8001EA00(s,4u);
        FlushDebugText800436F0();
    } while(S(tail)>0);
    s.Call(0x800357D4u,{0u});
    s.Call(0x800357D4u,{0u});
    return result;
}
}
