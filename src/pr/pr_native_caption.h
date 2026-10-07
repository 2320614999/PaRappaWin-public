#pragma once
#include "pr_stage2_loading_packets_direct.h"
#include <stdexcept>

// Source caption orchestration over caller-owned memory and the existing
// native sprite writer. No font renderer, device, clock or scene singleton.
// Current SCUS B730/B954 are the authority, not Stage1's private scene state.
namespace PrNativeCaption {
using Services=PrStage2LifecycleDirect::Services;
inline int32_t Signed(uint32_t value){
    return value<0x80000000u?int32_t(value):int32_t(int64_t(value)-0x100000000LL);
}
inline void SetOrigin8001B730(Services& s,uint32_t x,uint32_t y,uint32_t z){
    s.Write32(0x8006ED2Cu,z);
    s.Write16(0x8006ED30u,uint16_t(x));
    s.Write16(0x8006ED34u,uint16_t(y));
}
inline int32_t StringLength80047FAC(Services& s,uint32_t text){
    // SCUS 80047FAC is BIOS A0(1Bh), whose documented null contract returns
    // zero without dereferencing RAM. This is not host libc strlen(nullptr).
    // https://psx-spx.consoledev.net/kernelbios/#a1bh-strlensrc
    if(text==0u)return 0;
    // Real bytes, not a precomputed fixture return. Malformed unterminated
    // non-null input fails before an unbounded host loop.
    for(uint32_t length=0;length<0x200000u;++length)
        if(s.Read8(text+length)==0u)return int32_t(length);
    throw std::runtime_error("Native caption has no terminator in addressable RAM");
}
inline int32_t DrawText8001B954(Services& s,uint32_t text,uint32_t clutY,uint32_t table){
    const uint32_t firstPacket=s.WantsTmdPresentation()?s.Read32(0x800901C8u):0u;
    const int32_t length=s.Call(0x80047FACu,{text});
    PrStage2LoadingPacketsDirect::PrivateSprite local;
    local.attributes=s.Read32(0x8004E690u);
    uint32_t firstWidth=0,secondWidth=0,advance=0;
    bool secondLine=false;
    for(int32_t i=0;i<length;++i){
        const uint8_t ch=s.Read8(text+uint32_t(i));
        if(ch==10u)secondLine=true;
        else {
            const uint32_t width=s.Read8(0x80049460u+8u*ch);
            if(secondLine)secondWidth+=width;else firstWidth+=width;
        }
    }
    uint32_t centered=uint32_t(Signed(264u-firstWidth)/2);
    if(secondLine)secondWidth=uint32_t(Signed(264u-secondWidth)/2);
    for(int32_t i=0;i<length;++i){
        const uint32_t ch=s.Read8(text+uint32_t(i));
        if(ch==10u){
            advance=0;
            const uint32_t z=s.Read32(0x8006ED2Cu);
            s.Write32(0x8006ED2Cu,z+15u);centered=secondWidth;
            continue;
        }
        const uint32_t metric=0x8004945Cu+8u*ch;
        const uint32_t originX=s.Read16(0x8006ED30u);
        const uint32_t width=s.Read8(metric+4u);
        local.width=uint16_t(width);local.height=s.Read8(metric+5u);
        const uint32_t bearing=s.Read16(metric+6u);
        local.x=uint16_t(originX+centered+advance+bearing-155u);
        advance+=width;
        const uint32_t z=s.Read32(0x8006ED2Cu),originY=s.Read16(0x8006ED34u);
        local.y=uint16_t(originY+z-(((ch-192u)<30u&&ch!=199u)?123u:120u));
        const uint16_t glyphY=s.Read16(metric+2u);
        if(glyphY&0x8000u)continue;
        local.clutY=uint16_t(clutY);local.clutX=s.Read16(0x8004E69Cu);
        const uint16_t rawX=s.Read16(metric);
        const int32_t glyphX=rawX<0x8000u?int32_t(rawX):int32_t(rawX)-65536;
        const uint32_t baseX=s.Read16(0x8004E694u);
        const uint32_t u=uint32_t(glyphX)+(baseX<<2u)-(glyphX>0?1u:0u);
        const uint32_t baseY=s.Read16(0x8004E696u),v=baseY+s.Read16(metric+2u);
        const uint32_t page=uint32_t(PrStage2LoadingPacketsDirect::TexturePage80043DF4(s,0u,1u,(u&0xFF00u)>>2u,v&0xFF00u));
        local.page=uint16_t(page);local.u=uint8_t(u);local.v=uint8_t(v);
        (void)PrStage2LoadingPacketsDirect::SortPrivateSprite8003FA20(s,local,table,0u,page);
    }
    if(s.WantsTmdPresentation()) s.ObserveCaption(text,firstPacket,s.Read32(0x800901C8u));
    return Signed(advance);
}
// Save-name font: left aligned, selected/unselected palettes, and the original
// 192..222 accent range. Read the mutable descriptor rather than IDA constants.
inline int32_t DrawName8001B744(Services& s,uint32_t text,uint32_t style,uint32_t table){
    const int32_t length=s.Call(0x80047FACu,{text});
    PrStage2LoadingPacketsDirect::PrivateSprite local;
    local.attributes=s.Read32(0x8004E6A0u);
    if(length<=0)return length;
    uint32_t advance=0u,cursor=text;
    do {
        const uint32_t ch=s.Read8(cursor),metric=0x8004945Cu+8u*ch;
        const uint32_t originX=s.Read16(0x8006ED30u),width=s.Read8(metric+4u);
        local.width=uint16_t(width);local.height=s.Read8(metric+5u);
        const uint32_t bearing=s.Read16(metric+6u);
        local.x=uint16_t(originX+advance+bearing-160u);advance+=width;
        const uint32_t z=s.Read32(0x8006ED2Cu),originY=s.Read16(0x8006ED34u);
        local.y=uint16_t(originY+z-((ch-192u)<31u?123u:120u));
        if(!(s.Read16(metric+2u)&0x8000u)) {
            const uint32_t row=s.Read16(0x8004E6AEu);
            local.clutX=s.Read16(0x8004E6ACu);local.clutY=uint16_t(row+style);
            const uint16_t rawX=s.Read16(metric);
            const int32_t glyphX=rawX<0x8000u?int32_t(rawX):int32_t(rawX)-65536;
            const uint32_t baseX=s.Read16(0x8004E6A4u);
            const uint32_t u=uint32_t(glyphX)+(baseX<<2u)-(glyphX>0?1u:0u);
            const uint32_t baseY=s.Read16(0x8004E6A6u),v=baseY+s.Read16(metric+2u);
            const uint32_t page=uint32_t(PrStage2LoadingPacketsDirect::TexturePage80043DF4(s,0u,1u,(u&0xFF00u)>>2u,v&0xFF00u));
            local.page=uint16_t(page);local.u=uint8_t(u);local.v=uint8_t(v);
            (void)PrStage2LoadingPacketsDirect::SortPrivateSprite8003FA20(s,local,table,0u,page);
        }
        ++cursor;
    } while(Signed(cursor)<Signed(text+uint32_t(length)));
    return 0; // Final SLT result, not text width or the sprite packet tag.
}
// 原 8001C6A0：使用当前双缓冲工作表，调色板行按有符号半字传递。
inline int32_t WorkText8001C6A0(Services& s,uint32_t text,uint32_t clutY){
    const uint32_t table=0x80087288u+20u*s.Read32(0x8006EDA8u);
    const uint32_t row=uint32_t(int32_t(int16_t(clutY&0xFFFFu)));
    return s.Call(0x8001B954u,{text,row,table});
}
// 原 80023E10：空文本仍绘制框体；填充、圆角、文字保持原调用顺序和 OT 优先级。
inline int32_t ScriptBox80023E10(Services& s,uint32_t text){
    s.Call(0x8001C4ECu,{10u,199u,8u,18u,0x400F0F0Fu,2u});
    s.Call(0x8001C4ECu,{18u,191u,284u,34u,0x400F0F0Fu,2u});
    s.Call(0x8001C4ECu,{302u,199u,8u,18u,0x400F0F0Fu,2u});
    s.Call(0x8001C550u,{10u,191u,0x80050900u,2u});
    s.Call(0x8001C550u,{10u,217u,0x800508F0u,2u});
    s.Call(0x8001C550u,{302u,191u,0x800508E0u,2u});
    s.Call(0x8001C550u,{302u,217u,0x800508D0u,2u});
    s.CallVoid(0x8001B730u,{28u,194u,0u});
    return s.Call(0x8001C6A0u,{text,480u});
}
// 按原函数参数个数分派，拒绝在参数不完整时修改绘制状态。
inline bool TryCall(Services& s,uint32_t fn,std::initializer_list<uint32_t> args,int32_t& result){
    if(fn==0x8001B730u)throw std::invalid_argument("Caption origin is void; no scalar may be fabricated");
    size_t count;
    switch(fn){
    case 0x8001B954u:case 0x8001B744u:count=3u;break;
    case 0x8001C6A0u:count=2u;break;
    case 0x80023E10u:case 0x80047FACu:count=1u;break;
    default:return false;
    }
    if(args.size()!=count)throw std::invalid_argument("Caption argument count");
    const auto a=args.begin();
    switch(fn){
    case 0x8001B954u:result=DrawText8001B954(s,a[0],a[1],a[2]);break;
    case 0x8001B744u:result=DrawName8001B744(s,a[0],a[1],a[2]);break;
    case 0x8001C6A0u:result=WorkText8001C6A0(s,a[0],a[1]);break;
    case 0x80023E10u:result=ScriptBox80023E10(s,a[0]);break;
    case 0x80047FACu:result=StringLength80047FAC(s,a[0]);break;
    }
    return true;
}
inline bool TryCallVoid(Services& s,uint32_t fn,std::initializer_list<uint32_t> args){
    if(fn==0x8001B730u){
        if(args.size()!=3u)throw std::invalid_argument("Caption origin argument count");
        const auto a=args.begin();SetOrigin8001B730(s,a[0],a[1],a[2]);return true;
    }
    int32_t discarded;return TryCall(s,fn,args,discarded);
}
}
