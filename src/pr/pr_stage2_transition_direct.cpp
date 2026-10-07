#include "pr_stage2_transition_direct.h"
#include <array>

namespace PrStage2TransitionDirect {
namespace {
constexpr uint32_t Phase=0x8006EB04u; // Original GP+C4, shared with the Loading cursor.
constexpr uint32_t WorkBuffer=0x8006EDA8u;
int32_t Signed(uint32_t v){return v<0x80000000u?int32_t(v):int32_t(int64_t(v)-0x100000000LL);}
uint32_t Half(uint32_t v){return (v&0x8000u)?(v|0xFFFF0000u):(v&0xFFFFu);}
int32_t Sprite(Services& s,uint32_t x,uint32_t y,uint32_t source,uint32_t priority){
    // Each source call reloads GP+368. Do not cache an OT across a callee.
    return s.Call(0x8001B590u,{Half(x),Half(y),source,0u,0u,priority,
                              0x80087288u+20u*s.Read32(WorkBuffer)});
}
int32_t Tile(Services& s,uint32_t x,uint32_t y,uint32_t source,uint32_t priority){
    return s.Call(0x8001C550u,{Half(x),Half(y),source,priority});
}
}
int32_t Reset8001FFD4(Services& s,uint32_t mode){
    s.Write32(Phase,0u);
    return s.Call(0x8001EEACu,{mode==1u?0u:1u});
}
int32_t Pending8001F518(Services& s){return Signed(s.Read32(Phase))<191?1:0;}
int32_t FourFrames80020090(Services& s,uint32_t work,uint32_t mode,uint32_t prepare,uint32_t present){
    for(uint32_t i=0;i<4u;++i){
        s.Call(prepare,{work,mode});s.Call(0x80035560u,{2u});s.Call(present,{work});
    }
    return 0; // Final SLTI, not the last callback result.
}
int32_t FourFramesWithCue80020008(Services& s,uint32_t work,uint32_t mode,uint32_t prepare,uint32_t present){
    for(uint32_t i=0;i<4u;++i){
        s.Call(prepare,{work,mode});s.Call(0x80027194u,{30u});
        s.Call(0x80035560u,{2u});s.Call(present,{work});
    }
    return 0;
}
int32_t Transition800201AC(Services& s,uint32_t work,uint32_t mode,uint32_t before,uint32_t after){
    s.ObserveTransitionBoundary(0x800201ACu,0u,mode,0u);
    s.Call(0x8001FFD4u,{before});
    do{
        s.Call(0x8001EA74u,{work,mode});s.Call(0x80035560u,{2u});s.Call(0x8001EBF4u,{work});
    }while(s.Call(0x8001F518u,{})!=0);
    s.Call(0x8001FFD4u,{after});
    s.Write32(Phase,190u);
    const int32_t result = s.Call(0x80020090u,{work,mode,0x8001EA74u,0x8001EBF4u});
    s.ObserveTransitionBoundary(0x800201ACu,1u,mode,190u);
    s.AwaitTransitionPresentation();
    s.ObserveTransitionBoundary(0x800201ACu,2u,mode,190u);
    s.TransitionPresentationCompleted();
    return result;
}
int32_t TransitionWithCue80020110(Services& s,uint32_t work,uint32_t mode,uint32_t before,uint32_t after){
    s.ObserveTransitionBoundary(0x80020110u,0u,mode,0u);
    s.Call(0x8001FFD4u,{before});
    do{
        s.Call(0x8001EA74u,{work,mode});s.Call(0x80027194u,{30u});
        s.Call(0x80035560u,{2u});s.Call(0x8001EBF4u,{work});
    }while(s.Call(0x8001F518u,{})!=0);
    s.Call(0x8001FFD4u,{after});
    const int32_t result = s.Call(0x80020008u,{work,mode,0x8001EA74u,0x8001EBF4u});
    s.ObserveTransitionBoundary(0x80020110u,1u,mode,190u);
    s.AwaitTransitionPresentation();
    s.ObserveTransitionBoundary(0x80020110u,2u,mode,190u);
    s.TransitionPresentationCompleted();
    return result;
}
int32_t TransitionCue800271E4(Services& s,uint32_t index){
    const uint32_t row=s.Read32(0x8009441Cu)+6u*index;
    s.Call(0x80026EF8u,{row});return s.Call(0x80026ECCu,{});
}
int32_t PeriodicCue80027194(Services& s,uint32_t /*unused*/){
    if(Signed(s.Read32(0x8006EC20u))>=2){
        s.Call(0x80026EF8u,{0x8006EC18u});s.Call(0x80026ECCu,{});
        s.Write32(0x8006EC20u,0u);
    }
    const uint32_t next=s.Read32(0x8006EC20u)+1u;s.Write32(0x8006EC20u,next);return Signed(next);
}
int32_t MovieEnter80020248(Services& s,uint32_t subtitles){
    if(s.Read32(Phase)==0u)s.Call(0x800271E4u,{1u});
    uint32_t limit;
    if(subtitles){
        if(s.Read32(Phase)==15u)s.Call(0x800271E4u,{0u});
        if(Signed(s.Read32(Phase))<15)s.Call(0x8001F230u,{5u});
        else s.Call(0x8001C864u,{5u});
        limit=31u;
    }else{s.Call(0x8001CE30u,{5u});limit=16u;}
    const uint32_t next=s.Read32(Phase)+1u;s.Write32(Phase,next);
    if(Signed(next)>=int32_t(limit))s.Write32(Phase,192u);
    return 192; // LI in the conditional branch's delay slot, on both paths.
}
int32_t MovieLeave80020308(Services& s,uint32_t subtitles){
    if(s.Read32(Phase)==0u)s.Call(0x800271E4u,{0u});
    uint32_t limit;int32_t result=0;
    if(subtitles){
        if(s.Read32(Phase)==15u)s.Call(0x800271E4u,{1u});
        if(Signed(s.Read32(Phase))<15)s.Call(0x8001F230u,{5u});
        else{result=1;s.Call(0x8001FEB4u,{0u});}
        limit=31u;
    }else{s.Call(0x8001FEB4u,{0u});limit=16u;}
    const uint32_t next=s.Read32(Phase)+1u;s.Write32(Phase,next);
    if(Signed(next)>=int32_t(limit))s.Write32(Phase,192u);
    return result;
}
int32_t Border8001C864(Services& s,uint32_t priority){
    Sprite(s,280,200,0x8004E940u,priority);Sprite(s,20,200,0x8004E950u,priority);
    Sprite(s,280,180,0x8004E960u,priority);Sprite(s,20,180,0x8004E970u,priority);
    for(uint32_t i=0;i<7u;++i){
        const uint32_t x=40u+40u*i,left=20u+40u*i;
        Sprite(s,x,0,0x8004E900u,priority);Sprite(s,left,0,0x8004E910u,priority);
        Sprite(s,x,220,0x8004E920u,priority);Sprite(s,left,220,0x8004E930u,priority);
    }
    for(uint32_t i=0;i<6u;++i){
        const uint32_t y=20u+40u*i,top=40u*i,x=40u+40u*i,right=60u+40u*i;
        Sprite(s,0,y,0x8004E920u,priority);Sprite(s,0,top,0x8004E900u,priority);
        Sprite(s,300,y,0x8004E930u,priority);Sprite(s,300,top,0x8004E910u,priority);
        Sprite(s,x,200,0x8004E860u,priority);Sprite(s,right,200,0x8004E870u,priority);
        Sprite(s,x,180,0x8004E880u,priority);Sprite(s,right,180,0x8004E890u,priority);
    }
    Sprite(s,40,160,0x8004E8E0u,priority);Sprite(s,160,160,0x8004E8F0u,priority);
    Sprite(s,40,20,0x8004E8C0u,priority);Sprite(s,160,20,0x8004E8D0u,priority);
    Sprite(s,20,20,0x8004E8A0u,priority);return Sprite(s,280,20,0x8004E8B0u,priority);
}
int32_t CaptionBorder8001CE30(Services& s,uint32_t priority){
    for(uint32_t i=0;i<7u;++i){
        const uint32_t x=40u+40u*i,left=20u+40u*i;
        Sprite(s,x,0,0x8004E900u,priority);Sprite(s,left,0,0x8004E910u,priority);
        Sprite(s,x,220,0x8004E920u,priority);Sprite(s,left,220,0x8004E930u,priority);
        Sprite(s,x,20,0x8004E920u,priority);Sprite(s,left,20,0x8004E930u,priority);
        Sprite(s,x,200,0x8004E900u,priority);Sprite(s,left,200,0x8004E910u,priority);
    }
    for(uint32_t i=0;i<6u;++i){
        Sprite(s,0,20u+40u*i,0x8004E920u,priority);Sprite(s,0,40u*i,0x8004E900u,priority);
        Sprite(s,300,20u+40u*i,0x8004E930u,priority);Sprite(s,300,40u*i,0x8004E910u,priority);
    }
    for(uint32_t i=0;i<12u;++i)Sprite(s,40u+20u*i,180u,0x8004EA60u+16u*i,priority);
    // Original nonsequential title-tile order, not sorted by x or address.
    constexpr std::array<std::array<uint32_t,2>,14> tiles={{{280u,0x8004E980u},{20u,0x8004EA50u},
        {160u,0x8004E9F0u},{140u,0x8004E9E0u},{180u,0x8004EA00u},{120u,0x8004E9D0u},
        {200u,0x8004EA10u},{100u,0x8004E9C0u},{240u,0x8004EA30u},{80u,0x8004E9B0u},
        {220u,0x8004EA20u},{60u,0x8004E9A0u},{260u,0x8004EA40u},{40u,0x8004E990u}}};
    int32_t result=0;for(const auto& tile:tiles)result=Sprite(s,tile[0],40u,tile[1],priority);
    return result;
}
int32_t CommonBorder8001F230(Services& s,uint32_t priority){
    Tile(s,20,20,0x80050380u,priority);Tile(s,280,20,0x80050390u,priority);
    Tile(s,40,20,0x800503A0u,priority);Tile(s,160,20,0x800503B0u,priority);
    Tile(s,40,160,0x800503C0u,priority);Tile(s,160,160,0x800503D0u,priority);
    for(uint32_t i=0;i<7u;++i){
        const uint32_t x=40u+40u*i,left=20u+40u*i;
        Tile(s,x,0,0x800503E0u,priority);Tile(s,left,0,0x800503F0u,priority);
        Tile(s,x,220,0x80050400u,priority);Tile(s,left,220,0x80050410u,priority);
    }
    for(uint32_t i=0;i<6u;++i){
        const uint32_t y=20u+40u*i,top=40u*i,x=40u+40u*i,right=60u+40u*i;
        Tile(s,0,y,0x80050400u,priority);Tile(s,0,top,0x800503E0u,priority);
        Tile(s,300,y,0x80050410u,priority);Tile(s,300,top,0x800503F0u,priority);
        Tile(s,x,200,0x800503E0u,priority);Tile(s,right,200,0x800503F0u,priority);
        Tile(s,x,180,0x80050400u,priority);Tile(s,right,180,0x80050410u,priority);
    }
    Tile(s,280,200,0x800503E0u,priority);Tile(s,20,200,0x800503F0u,priority);
    Tile(s,280,180,0x80050400u,priority);return Tile(s,20,180,0x80050410u,priority);
}
int32_t CommonTiles8001FEB4(Services& s,uint32_t priority){
    for(uint32_t x=0;x<8u;++x)for(uint32_t y=0;y<6u;++y){
        Tile(s,40u*x,40u*y,0x800503E0u,priority);Tile(s,20u+40u*x,40u*y,0x800503F0u,priority);
        Tile(s,40u*x,20u+40u*y,0x80050400u,priority);Tile(s,20u+40u*x,20u+40u*y,0x80050410u,priority);
    }
    return 0;
}
void SetPatternTile8001F698(Services& s,uint32_t pattern,uint32_t value){
    // 原 switch 先作无符号范围检查；非法图案不读阶段值，也不写遮罩。
    if(pattern>=18u)return;
    const uint32_t phase=s.Read32(Phase);
    const int32_t p=Signed(phase);
    uint32_t row=0,column=0;
    switch(pattern){
    case 0:case 4:case 9:case 10:case 12:case 16:{
        // 坐标对来自原 RAM 表；反向索引和地址计算保留 32 位回绕。
        const uint32_t table=(pattern==0u||pattern==4u)?0x8004F180u:
            (pattern==9u||pattern==10u)?0x8004EB80u:pattern==12u?0x8004F780u:0x8004FD80u;
        const uint32_t index=(pattern==4u||pattern==10u)?191u-phase:phase;
        row=s.Read32(table+8u*index);column=s.Read32(table+8u*index+4u);break;
    }
    case 1:case 6:{
        uint32_t adjusted=phase;
        if(p>=48)adjusted+=phase-48u<48u?1u:phase-96u<48u?2u:3u;
        row=uint32_t(Signed(adjusted)%12);
        // case 1 仅在 >=48 分支重读；case 6 始终重读，不能缓存第一次读取。
        const uint32_t next=(pattern==6u||p>=48)?s.Read32(Phase):phase;
        column=uint32_t(Signed(next)%16);
        if(pattern==6u)column=15u-column;
        break;
    }
    case 2:row=uint32_t(p/16);column=uint32_t(p%16);break;
    case 3:row=uint32_t(p%12);column=uint32_t((p/12)%16);break;
    case 5:row=uint32_t(p%12);column=uint32_t(Signed(15u-uint32_t(p/12))%16);break;
    case 7:row=11u-uint32_t(p/16);column=15u-uint32_t(p%16);break;
    case 8:row=11u-uint32_t(p%12);column=15u-uint32_t(Signed(15u-uint32_t(p/12))%16);break;
    case 11:
        row=uint32_t(p%12);
        column=(row&1u)?uint32_t((p/12)%16):uint32_t(Signed(15u-uint32_t(p/12))%16);
        break;
    case 13:
        row=uint32_t(p/8);
        column=2u*uint32_t(p%8);
        if(Signed(row)<12){if((row&1u)==0u)++column;}
        else{row-=12u;if(row&1u)++column;}
        break;
    case 14:
        row=uint32_t(p/16);column=uint32_t(p%16);
        if((column&1u)==0u)row=11u-row;
        break;
    case 15:
        column=uint32_t(p/12);row=uint32_t(p%12);
        if((column&1u)==0u)row=11u-row;
        break;
    case 17:row=11u-uint32_t(p/16);column=uint32_t(p%16);break;
    }
    s.Write32(0x80087330u+64u*row+4u*column,value);
}
int32_t RevealTiles8001FC40(Services& s,uint32_t pattern,uint32_t count){
    // 每步判定后重新读取阶段值，保留原函数负计数跳过和阶段溢出行为。
    for(uint32_t i=0;Signed(i)<Signed(count);++i){
        if(Signed(s.Read32(Phase))<192)SetPatternTile8001F698(s,pattern,0u);
        s.Write32(Phase,s.Read32(Phase)+1u);
    }
    return 1;
}
int32_t CoverTiles8001FCBC(Services& s,uint32_t count,uint32_t pattern){
    // 原覆盖入口的两个参数顺序与揭开入口相反。
    for(uint32_t i=0;Signed(i)<Signed(count);++i){
        if(Signed(s.Read32(Phase))<192)SetPatternTile8001F698(s,pattern,1u);
        s.Write32(Phase,s.Read32(Phase)+1u);
    }
    return 1;
}
int32_t DrawMaskedTiles8001FDC0(Services& s,uint32_t priority){
    // 逐行读取遮罩；仅非零格位读取索引和精灵，priority 保留完整原寄存器。
    for(uint32_t y=0;y<12u;++y)for(uint32_t x=0;x<16u;++x){
        const uint32_t offset=4u*(16u*y+x);
        if(s.Read32(0x80087330u+offset)!=0u){
            const uint32_t index=s.Read32(0x80050420u+offset);
            Tile(s,20u*x,20u*y,s.Read32(0x80050720u+4u*index),priority);
        }
    }
    return 0;
}
bool TryCallVoid(Services& s,uint32_t fn,std::initializer_list<uint32_t> args){
    // 原图案函数没有标量返回值，独立保留 void 调用约定。
    if(fn!=0x8001F698u)return false;
    if(args.size()!=2u)throw std::invalid_argument("S2 pattern argument count mismatch");
    SetPatternTile8001F698(s,args.begin()[0],args.begin()[1]);return true;
}
bool TryCall(Services& s,uint32_t fn,std::initializer_list<uint32_t> args,int32_t& result){
    size_t count;
    switch(fn){
    case 0x8001F518u:count=0;break;
    case 0x8001FFD4u:case 0x80020248u:case 0x80020308u:case 0x8001C864u:case 0x8001CE30u:
    case 0x8001F230u:case 0x8001FEB4u:case 0x800271E4u:case 0x80027194u:case 0x8001FDC0u:count=1;break;
    case 0x8001FC40u:case 0x8001FCBCu:count=2;break;
    case 0x8001F698u:throw std::invalid_argument("S2 pattern requires CallVoid");
    case 0x80020090u:case 0x80020008u:case 0x800201ACu:case 0x80020110u:count=4;break;
    default:return false;
    }
    if(args.size()!=count)throw std::invalid_argument("S2 source transition argument count mismatch");
    const auto a=args.begin();
    switch(fn){
    case 0x8001FFD4u:result=Reset8001FFD4(s,a[0]);break;
    case 0x8001F518u:result=Pending8001F518(s);break;
    case 0x80020090u:result=FourFrames80020090(s,a[0],a[1],a[2],a[3]);break;
    case 0x80020008u:result=FourFramesWithCue80020008(s,a[0],a[1],a[2],a[3]);break;
    case 0x800201ACu:result=Transition800201AC(s,a[0],a[1],a[2],a[3]);break;
    case 0x80020110u:result=TransitionWithCue80020110(s,a[0],a[1],a[2],a[3]);break;
    case 0x80020248u:result=MovieEnter80020248(s,a[0]);break;
    case 0x80020308u:result=MovieLeave80020308(s,a[0]);break;
    case 0x8001C864u:result=Border8001C864(s,a[0]);break;
    case 0x8001CE30u:result=CaptionBorder8001CE30(s,a[0]);break;
    case 0x8001F230u:result=CommonBorder8001F230(s,a[0]);break;
    case 0x8001FEB4u:result=CommonTiles8001FEB4(s,a[0]);break;
    case 0x8001FC40u:result=RevealTiles8001FC40(s,a[0],a[1]);break;
    case 0x8001FCBCu:result=CoverTiles8001FCBC(s,a[0],a[1]);break;
    case 0x8001FDC0u:result=DrawMaskedTiles8001FDC0(s,a[0]);break;
    case 0x800271E4u:result=TransitionCue800271E4(s,a[0]);break;
    case 0x80027194u:result=PeriodicCue80027194(s,a[0]);break;
    }
    return true;
}
}
