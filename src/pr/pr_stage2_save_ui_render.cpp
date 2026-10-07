#include "pr_stage2_save_ui_render.h"
#include <array>

namespace PrStage2SaveUiRender {
namespace {
constexpr uint32_t Gp=0x8006EA40u;
int32_t S(uint32_t value) {
    return value<0x80000000u?int32_t(value):int32_t(int64_t(value)-0x100000000LL);
}
uint32_t H(uint32_t value) {
    return (value&0x8000u)?(value|0xFFFF0000u):(value&0xFFFFu);
}
uint32_t WorkTable(Services& s) { return 0x80087288u+20u*s.Read32(Gp+872u); }
int32_t Sprite(Services& s,uint32_t x,uint32_t y,uint32_t source,uint32_t priority) {
    return s.Call(0x8001C550u,{x,y,source,priority});
}
void Positioned(Services& s,uint32_t xy,uint32_t source,uint32_t priority) {
    s.Call(0x8001C5A8u,{xy,source,priority});
}
}

int32_t CoordinateSprite8001C5A8(Services& s,uint32_t xy,uint32_t source,uint32_t priority) {
    const uint32_t x=H(s.Read16(xy)),y=H(s.Read16(xy+2u));
    const uint32_t table=WorkTable(s);
    // The caller stores the full register on the stack; the callee owns any
    // halfword conversion despite the decompiler's uint16_t argument type.
    return s.Call(0x8001B590u,{x,y,source,0u,0u,priority,table});
}

int32_t NameText8001C668(Services& s,uint32_t text,uint32_t style) {
    return s.Call(0x8001B744u,{text,style,WorkTable(s)});
}
int32_t BannerText8001C6E0(Services& s,uint32_t text) {
    return s.Call(0x8001B954u,{text,480u,WorkTable(s)});
}
int32_t CoordinateFrame8001C7A8(Services& s,uint32_t xy,uint32_t source,uint32_t frame,uint32_t priority) {
    const uint32_t x=H(s.Read16(xy)),y=H(s.Read16(xy+2u));
    return s.Call(0x8001B5F4u,{x,y,source,frame,priority,WorkTable(s)});
}

int32_t Background8001D74C(Services& s,uint32_t priority,uint32_t buffer) {
    const uint32_t table=0x80087288u+20u*buffer;
    const auto sprite=[&](uint32_t x,uint32_t y,uint32_t source) {
        s.Call(0x8001B590u,{x,y,source,0u,0u,priority,table});
    };
    sprite(20u,20u,0x8004E7E0u); sprite(20u,120u,0x8004E7F0u);
    sprite(280u,20u,0x8004E800u); sprite(280u,120u,0x8004E810u);
    sprite(40u,20u,0x8004E820u); sprite(160u,20u,0x8004E830u);
    sprite(40u,200u,0x8004E840u); sprite(160u,200u,0x8004E850u);
    for(uint32_t x=40u;x<=240u;x+=40u)
        for(uint32_t y=40u;y<=160u;y+=40u) sprite(x,y,0x8004E7D0u);
    for(uint32_t x=40u;x<=280u;x+=40u) {
        sprite(x,0u,0x8004E900u); sprite(x-20u,0u,0x8004E910u);
        sprite(x,220u,0x8004E920u); sprite(x-20u,220u,0x8004E930u);
    }
    for(uint32_t y=0u;y<=200u;y+=40u) {
        sprite(0u,y,0x8004E900u); sprite(300u,y,0x8004E910u);
        sprite(0u,y+20u,0x8004E920u); sprite(300u,y+20u,0x8004E930u);
    }
    return 0; // Last SLTI in the original loop, not the final sprite result.
}

int32_t CardBanner80020A3C(Services& s,uint32_t kind) {
    const uint32_t language=H(s.Read16(0x800916D8u));
    s.CallVoid(0x8001B730u,{30u,121u,0u});
    const uint32_t text=kind==2u&&language==0u?0x800111CCu:
        s.Read32((kind==1u?0x80053280u:0x8005326Cu)+4u*language);
    const uint32_t width=uint32_t(s.Call(0x8001C6E0u,{text}))+8u;
    const uint32_t center=uint32_t(S(320u-width)/2),left=H(center);
    const uint32_t halfWidth=H(width),right=H(center+halfWidth);
    s.Call(0x8001C4ECu,{left,123u,8u,9u,0x400F0F0Fu,0u});
    s.Call(0x8001C4ECu,{H(center+8u),115u,(halfWidth-8u)&0xFFFFu,25u,0x400F0F0Fu,0u});
    s.Call(0x8001C4ECu,{right,123u,8u,9u,0x400F0F0Fu,0u});
    Sprite(s,left,115u,0x80050900u,0u); Sprite(s,left,132u,0x800508F0u,0u);
    Sprite(s,right,115u,0x800508E0u,0u);
    return Sprite(s,right,132u,0x800508D0u,0u);
}

int32_t Prompt80022CBC(Services& s,uint32_t kind,uint32_t context) {
    const uint32_t language=H(s.Read16(0x800916D8u));
    uint32_t message=0u;
    switch(kind) {
    case 1u: message=0x80053A08u; break;
    case 3u: message=0x800539B8u; break;
    case 5u: message=0x80053A58u; break;
    case 6u: message=0x80053A80u; break;
    default: break;
    }
    if(message) {
        Sprite(s,36u,54u,0x80052330u,1u);
        Positioned(s,message+4u+8u*language,s.Read32(message+8u*language),1u);
        const uint32_t selected=s.Read32(context);
        Sprite(s,231u,179u,s.Read32(Gp+(selected==1u?208u:212u)),1u);
        const bool choice=s.Read32(context+4u)!=0u;
        Positioned(s,0x8005300Cu+16u*language,
                   s.Read32((choice?0x80053008u:0x80053004u)+16u*language),0u);
        Positioned(s,0x80052FFCu,s.Read32(choice?0x80052FF8u:0x80052FF4u),0u);
    } else if(kind==2u||kind==4u||kind==7u) {
        Sprite(s,kind==2u?34u:28u,kind==2u?54u:56u,kind==2u?0x80052340u:0x80052350u,1u);
        message=kind==2u?0x80053990u:kind==4u?0x80053A30u:0x800539E0u;
        Positioned(s,message+4u+8u*language,s.Read32(message+8u*language),1u);
        if(kind==7u&&s.Read32(context+8u)!=0u) s.Call(0x80020A3Cu,{2u});
        Sprite(s,224u,149u,s.Read32(context)==1u?0x800526A0u:0x800526B0u,1u);
        const uint32_t choice=s.Read32(context+4u);
        Positioned(s,0x80053AB4u+16u*language,
                   s.Read32((choice==1u?0x80053AB0u:0x80053AACu)+16u*language),1u);
        Positioned(s,0x80053B04u+16u*language,
                   s.Read32((choice==2u?0x80053B00u:0x80053AFCu)+16u*language),1u);
        Sprite(s,234u,159u,choice==1u?0x800525A0u:0x80052590u,1u);
        Sprite(s,234u,182u,choice==2u?0x800526D0u:0x800526C0u,1u);
    }
    return Sprite(s,121u,36u,0x80052320u,2u);
}

int32_t NameEntry80020BE4(Services& s,uint32_t context) {
    const uint32_t language=H(s.Read16(0x800916D8u));
    std::array<uint32_t,57> selected{};
    const uint32_t cursor=H(s.Read16(context+22u));
    if(cursor>=selected.size()) throw std::out_of_range("S2 name cursor exceeds original private array");
    selected[cursor]=1u;
    if(s.Read32(context+8u)) s.Call(0x80020A3Cu,{2u});
    s.CallVoid(0x8001B730u,{117u,64u,0u});
    s.Call(0x8001C668u,{context+40u,0u});
    Positioned(s,0x80053478u+8u*language,s.Read32(0x80053474u+8u*language),0u);
    Sprite(s,36u,28u,0x80051C00u,0u);
    for(uint32_t i=0;i<selected.size();++i)
        s.Call(0x8001C7A8u,{0x800532B0u+8u*i,s.Read32(0x800532ACu+8u*i),selected[i],0u});
    const uint32_t choice=H(s.Read16(context+26u));
    uint32_t pointer,first,second,icon1,icon2;
    if(choice==1u||choice==2u) {
        pointer=s.Read32(Gp+220u);
        first=choice==1u?0x800534A4u:0x800534A0u;
        second=choice==2u?0x800534F4u:0x800534F0u;
        icon1=choice==1u?0x8005329Cu:0x80053298u;
        icon2=choice==2u?0x800532A8u:0x800532A4u;
    } else if(selected[56]) {
        pointer=s.Read32(Gp+(s.Read32(context)?220u:216u));
        first=0x800534A0u; second=0x800534F0u;
        icon1=0x80053298u; icon2=0x800532A4u;
    } else {
        pointer=s.Read32(Gp+216u);
        first=0x8005349Cu; second=0x800534ECu;
        icon1=0x80053294u; icon2=0x800532A0u;
    }
    Sprite(s,223u,160u,pointer,0u);
    Positioned(s,0x800534A8u+16u*language,s.Read32(first+16u*language),0u);
    Positioned(s,0x800534F8u+16u*language,s.Read32(second+16u*language),0u);
    Sprite(s,232u,169u,s.Read32(icon1),0u);
    return Sprite(s,232u,188u,s.Read32(icon2),0u);
}

int32_t CardList80020F94(Services& s,uint32_t event,uint32_t context) {
    const uint32_t language=H(s.Read16(0x800916D8u));
    const uint32_t column=event==7u?0u:event==8u?4u:8u;
    Positioned(s,0x80053DF8u+16u*language+column,s.Read32(0x80053DF4u+16u*language),0u);
    Positioned(s,0x80053DA8u+16u*language+column,s.Read32(0x80053DA4u+16u*language),0u);
    const uint32_t label=event==7u?0x80053E94u:event==8u?0x80053EE4u:0x80053F34u;
    Positioned(s,label+4u+8u*language,s.Read32(label+8u*language),0u);
    Positioned(s,label+44u+8u*language,s.Read32(label+40u+8u*language),0u);
    Positioned(s,0x80053E48u+16u*language+column,
        event!=7u&&event!=8u&&language==1u?0x80052950u:s.Read32(0x80053E44u+16u*language),0u);
    if(s.Read32(context+8u)) s.Call(0x80020A3Cu,{event==7u?2u:0u});
    Sprite(s,37u,36u,0x80052870u,0u);
    Sprite(s,123u,30u,0x800522C0u,0u);
    uint32_t slot=0u,row=0u;
    if(S(H(s.Read16(context+12u)))>0) do {
        if(S(slot)>=S(H(s.Read16(context+18u)))) break;
        uint32_t col=0u;
        if(S(H(s.Read16(context+14u)))>0) do {
            if(S(slot)>=S(H(s.Read16(context+18u)))) break;
            const uint32_t state=slot==H(s.Read16(context+20u))?1u:
                s.Read16(context+22u+2u*slot)?0u:2u;
            const uint32_t text=context+120u+32u*slot;
            const bool named=s.Read8(text)!=0u;
            s.CallVoid(0x8001B730u,{H(52u+74u*col),H(78u+21u*row),0u});
            if(named) s.Call(0x8001C668u,{text,state!=1u?1u:0u});
            ++slot;
            Sprite(s,H(51u+74u*col),H(73u+21u*row),s.Read32(0x80053D98u+4u*state),0u);
            ++col;
        } while(S(col)<S(H(s.Read16(context+14u))));
        ++row;
    } while(S(row)<S(H(s.Read16(context+12u))));
    const uint32_t rows=H(s.Read16(context+12u)),cols=H(s.Read16(context+14u));
    const uint32_t cursor=H(s.Read16(context+20u));
    uint32_t last;
    if(cursor==rows*cols) {
        Sprite(s,231u,179u,s.Read32(Gp+(s.Read32(context)?208u:212u)),0u);
        const bool choice=s.Read32(context+4u)!=0u;
        Positioned(s,0x8005300Cu+16u*language,s.Read32((choice?0x80053008u:0x80053004u)+16u*language),0u);
        last=s.Read32(choice?0x80052FF8u:0x80052FF4u);
    } else {
        Sprite(s,231u,179u,s.Read32(Gp+212u),0u);
        Positioned(s,0x8005300Cu+16u*language,s.Read32(0x80053000u+16u*language),0u);
        last=s.Read32(0x80052FF0u);
    }
    return s.Call(0x8001C5A8u,{0x80052FFCu,last,0u});
}

int32_t EventFrame8001E750(Services& s,uint32_t event,uint32_t context) {
    const uint32_t buffer=uint32_t(s.Call(0x8004019Cu,{}));
    const uint32_t packet=s.Read32(0x8006ED50u+4u*buffer);
    s.Write32(Gp+872u,buffer);
    s.CallVoid(0x80040F90u,{packet});
    s.Call(0x80040CC8u,{0u,0u,WorkTable(s)});
    if(event==4u) {
        const uint32_t state=s.Read32(Gp+908u);
        if(!state) return s.Call(0x800203D4u,{s.Read32(context)});
        if(state==1u) {
            s.Call(0x8001B6C4u,{0u,0u,320u,240u,0x400F0F0Fu,0u,WorkTable(s)});
            const uint32_t next=s.Read32(Gp+908u)+1u;
            s.Write32(Gp+908u,next); return S(next);
        }
        const int32_t result=s.Call(0x8001B120u,{0u});
        s.Write32(Gp+908u,0u); return result;
    }
    if(event==10u) return s.Call(0x8001EF40u,{H(s.Read16(0x800916E0u)),0u});
    if(event<2u||event>19u) return S((event-2u)<<2u);
    s.Call(0x8001D74Cu,{event==17u?4u:3u,s.Read32(Gp+872u)});
    switch(event) {
    case 2u: return s.Call(0x80020568u,{context});
    case 3u: return s.Call(0x80021E60u,{context});
    case 5u: return s.Call(0x80020BE4u,{context});
    case 6u: return s.Call(0x80021594u,{context});
    case 7u: case 8u: case 9u: return s.Call(0x80020F94u,{event,context});
    case 11u: return s.Call(0x80022CBCu,{4u,context});
    case 12u: return s.Call(0x80022CBCu,{1u,context});
    case 13u: return s.Call(0x80022CBCu,{2u,context});
    case 14u: return s.Call(0x80022CBCu,{3u,context});
    case 15u: return s.Call(0x80022CBCu,{5u,context});
    case 16u: return s.Call(0x80023618u,{context});
    case 17u: return s.Call(0x80021910u,{context});
    case 18u: return s.Call(0x80022CBCu,{6u,context});
    case 19u: return s.Call(0x80022CBCu,{7u,context});
    }
    throw std::logic_error("S2 event draw branch was not dispatched");
}

bool TryCall(Services& s,uint32_t function,std::initializer_list<uint32_t> args,int32_t& result) {
    size_t count;
    switch(function) {
    case 0x8001C5A8u: count=3u; break;
    case 0x8001C7A8u: count=4u; break;
    case 0x8001C668u: case 0x80020F94u: count=2u; break;
    case 0x8001D74Cu: case 0x80022CBCu: case 0x8001E750u: count=2u; break;
    case 0x80020A3Cu: case 0x8001C6E0u: case 0x80020BE4u: count=1u; break;
    default: return false;
    }
    if(args.size()!=count) throw std::invalid_argument("S2 save render argument count mismatch");
    const auto a=args.begin();
    switch(function) {
    case 0x8001C5A8u: result=CoordinateSprite8001C5A8(s,a[0],a[1],a[2]); break;
    case 0x8001C7A8u: result=CoordinateFrame8001C7A8(s,a[0],a[1],a[2],a[3]); break;
    case 0x8001C668u: result=NameText8001C668(s,a[0],a[1]); break;
    case 0x8001C6E0u: result=BannerText8001C6E0(s,a[0]); break;
    case 0x80020BE4u: result=NameEntry80020BE4(s,a[0]); break;
    case 0x80020F94u: result=CardList80020F94(s,a[0],a[1]); break;
    case 0x8001D74Cu: result=Background8001D74C(s,a[0],a[1]); break;
    case 0x80022CBCu: result=Prompt80022CBC(s,a[0],a[1]); break;
    case 0x80020A3Cu: result=CardBanner80020A3C(s,a[0]); break;
    case 0x8001E750u: result=EventFrame8001E750(s,a[0],a[1]); break;
    }
    return true;
}
}
