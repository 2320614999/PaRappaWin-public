#include "pr_stage2_loading_packets_direct.h"
#include <array>
#include <stdexcept>

namespace PrStage2LoadingPacketsDirect {
namespace {
constexpr uint32_t Allocator=0x800901C8u;
int32_t Signed(uint32_t v){return v<=0x7FFFFFFFu?static_cast<int32_t>(v):static_cast<int32_t>(int64_t(v)-0x100000000LL);}
uint32_t Half(uint32_t v){v&=65535u;return v<32768u?v:v|0xFFFF0000u;}
struct Ram {
    Services& s; uint32_t base;
    uint8_t R8(uint32_t p){return s.Read8(base+p);}
    uint16_t R16(uint32_t p){return s.Read16(base+p);}
    uint32_t R32(uint32_t p){return s.Read32(base+p);}
    void W8(uint32_t p,uint8_t v){s.Write8(base+p,v);}
    void W16(uint32_t p,uint16_t v){s.Write16(base+p,v);}
    void W32(uint32_t p,uint32_t v){s.Write32(base+p,v);}
};
struct Local {
    std::array<uint8_t,24> bytes{};
    std::array<bool,24> known{};
    uint8_t R8(uint32_t p){
        if(p>=bytes.size())throw std::logic_error("Private Gs local exceeds its type");
        if(!known[p]){
            // 8001B25C does not initialize local+20..22. GP0 raw texture
            // ignores these three bytes. Canonicalize ONLY that ignored field;
            // this is semantic equivalence, not recovered PSX stack residue.
            if(p>=20u&&p<=22u&&known[0]&&(bytes[0]&0x40u))return 0u;
            throw std::runtime_error("Gs modulated sprite RGB producer is unbound");
        }
        return bytes[p];
    }
    uint16_t R16(uint32_t p){const auto a=R8(p),b=R8(p+1u);return uint16_t(a|(uint16_t(b)<<8u));}
    uint32_t R32(uint32_t p){const auto a=R16(p),b=R16(p+2u);return uint32_t(a)|(uint32_t(b)<<16u);}
    void W8(uint32_t p,uint8_t v){if(p>=bytes.size())throw std::logic_error("Gs local write bound");bytes[p]=v;known[p]=true;}
    void W16(uint32_t p,uint16_t v){W8(p,uint8_t(v));W8(p+1u,uint8_t(v>>8u));}
    void W32(uint32_t p,uint32_t v){W16(p,uint16_t(v));W16(p+2u,uint16_t(v>>16u));}
};
template<class View>
int32_t Prefix(Services& s,View& v,uint32_t source,uint32_t frame,uint32_t advance){
    v.W32(0u,s.Read32(source));
    v.W16(8u,s.Read16(source+8u));
    v.W16(10u,s.Read16(source+10u));
    const uint32_t width=Half(s.Read16(source+8u));
    const uint32_t tx=Half(s.Read16(source+4u));
    const uint32_t ty=Half(s.Read16(source+6u));
    const uint32_t u=4u*tx+frame*width;
    const auto page=TexturePage80043DF4(s,0u,1u,(u&0xFF00u)>>2u,ty&0xFF00u);
    v.W16(12u,uint16_t(page));v.W8(14u,uint8_t(u));
    v.W8(15u,uint8_t(s.Read16(source+6u)));
    v.W16(16u,s.Read16(source+12u));
    const uint32_t cy=uint32_t(s.Read16(source+14u))+(advance?frame:0u);
    v.W16(18u,uint16_t(cy));return Signed(cy);
}
template<class View>
int32_t SpritePacket(Services& s,View& v,uint32_t table,uint32_t priority,uint32_t incoming){
    const uint32_t attr=v.R32(0u);
    if(Signed(attr)<0)return Signed(incoming);
    if(v.R16(8u)==0u)return 0;
    if(v.R16(10u)==0u)return 0;
    const uint32_t page=v.R16(12u);
    const uint32_t packet=s.Read32(Allocator);
    const uint32_t y=v.R16(6u),x=v.R16(4u);
    s.Write32(packet+4u,(page&31u)|((attr>>17u)&0x180u)|0xE1000200u|((attr>>23u)&0x60u));
    const uint32_t ox=s.Read16(0x800917AAu),oy=s.Read16(0x800917ACu);
    const uint32_t blue=v.R8(22u),green=v.R8(21u),red=v.R8(20u);
    s.Write32(packet+12u,((x+ox)&65535u)|((y+oy)<<16u));
    s.Write32(packet+8u,((attr>>5u)&0x02000000u)|((attr<<18u)&0x01000000u)|0x64000000u|(blue<<16u)|(green<<8u)|red);
    const uint32_t vv=v.R8(15u),uu=v.R8(14u),cy=Half(v.R16(18u));
    const uint32_t cx=Half(v.R16(16u));
    s.Write32(packet+16u,uu|(vv<<8u)|(cy<<22u)|((cx<<12u)&0x003F0000u));
    const uint32_t height=v.R16(10u),width=v.R16(8u);
    s.Write32(packet+20u,width|(height<<16u));
    const uint32_t head=s.Read32(table+4u),offset=s.Read32(table+8u);
    const uint32_t slot=4u*(priority&65535u)+head-4u*offset;
    const uint32_t tag=s.Read32(slot)+0x05000000u; // ADDU, not a masked OR.
    s.Write32(packet,tag);
    s.Write32(slot,packet&0x00FFFFFFu);
    s.Write32(Allocator,packet+24u);
    return Signed(tag);
}

template<class View>
int32_t SelectionPrefix(Services& s,View& v,uint32_t source,uint32_t selected){
    v.W32(0u,s.Read32(source));
    v.W16(8u,s.Read16(source+8u));v.W16(10u,s.Read16(source+10u));
    const uint32_t x=Half(s.Read16(source+4u)),y=Half(s.Read16(source+6u));
    v.W16(12u,uint16_t(TexturePage80043DF4(s,0u,1u,x&0x3FC0u,y&0xFF00u)));
    v.W8(14u,uint8_t(x*4u));v.W8(15u,uint8_t(s.Read16(source+6u)));
    v.W16(16u,s.Read16(source+12u));
    const uint32_t row=uint32_t(s.Read16(source+14u))+(selected?0u:1u);
    v.W16(18u,uint16_t(row));return Signed(row);
}
template<class View>
int32_t BoxPacket(Services& s,View& v,uint32_t table,uint32_t priority,uint32_t incoming){
    const uint32_t attr=v.R32(0u);if(Signed(attr)<0)return Signed(incoming);
    const uint32_t packet=s.Read32(Allocator);
    s.Write32(packet+4u,((attr>>17u)&0x180u)|((attr>>23u)&0x60u)|0xE1000200u);
    s.Write8(packet+8u,v.R8(12u));s.Write8(packet+9u,v.R8(13u));
    const uint8_t blue=v.R8(14u);
    s.Write8(packet+11u,uint8_t(((attr>>29u)&2u)|0x60u));s.Write8(packet+10u,blue);
    const uint32_t x=v.R16(4u),ox=s.Read16(0x800917AAu);
    s.Write16(packet+12u,uint16_t(x+ox));
    const uint32_t y=v.R16(6u),oy=s.Read16(0x800917ACu);
    s.Write16(packet+14u,uint16_t(y+oy));
    s.Write16(packet+16u,v.R16(8u));s.Write16(packet+18u,v.R16(10u));
    const uint32_t next=uint32_t(Link8003EF5C(s,packet,table,priority&65535u,4u));
    s.Write32(Allocator,next);return Signed(next);
}
}
int32_t GpuType80044A24(Services& s){return s.Read8(0x8005D734u);}
int32_t TexturePage80043DF4(Services& s,uint32_t depth,uint32_t blend,uint32_t x,uint32_t y){
    // Keep the conditional second read: this mutable byte is folded to zero
    // by the original IDA ROM segment attributes.
    if(GpuType80044A24(s)==1||GpuType80044A24(s)==2)
        return int32_t(((depth&3u)<<9u)|((blend&3u)<<7u)|((y&0x300u)>>3u)|((x&0x3FFu)>>6u));
    return int32_t(((depth&3u)<<7u)|((blend&3u)<<5u)|((y&0x100u)>>4u)|((x&0x3FFu)>>6u)|((y&0x200u)<<2u));
}
int32_t SpritePrefix8001B25C(Services& s,uint32_t local,uint32_t source,uint32_t frame,uint32_t advance){Ram v{s,local};return Prefix(s,v,source,frame,advance);}
int32_t SelectionPrefix8001B428(Services& s,uint32_t local,uint32_t source,uint32_t selected){Ram v{s,local};return SelectionPrefix(s,v,source,selected);}
int32_t Link8003EF5C(Services& s,uint32_t packet,uint32_t table,uint32_t priority,uint32_t words){
    const uint32_t offset=(priority&65535u)-s.Read32(table+8u);
    if(Signed(offset)<0)(void)s.Call(0x80047FFCu,{0x80012084u});
    const uint32_t slot=s.Read32(table+4u)+4u*offset;
    const uint32_t next=packet+4u*(words&255u)+4u;
    const uint32_t previous=s.Read32(slot);
    s.Write32(packet,previous);s.Write8(packet+3u,uint8_t(words));
    s.Write32(slot,packet);s.Write8(slot+3u,0u);
    return Signed(next);
}
int32_t SortSprite8003FA20(Services& s,uint32_t local,uint32_t table,uint32_t priority,uint32_t incoming){Ram v{s,local};return SpritePacket(s,v,table,priority,incoming);}
int32_t SortPrivateSprite8003FA20(Services& s,const PrivateSprite& sprite,uint32_t table,uint32_t priority,uint32_t incoming){
    Local v;v.W32(0u,sprite.attributes);
    v.W16(4u,sprite.x);v.W16(6u,sprite.y);v.W16(8u,sprite.width);v.W16(10u,sprite.height);
    v.W16(12u,sprite.page);v.W8(14u,sprite.u);v.W8(15u,sprite.v);
    v.W16(16u,sprite.clutX);v.W16(18u,sprite.clutY);
    // Reuse the exact existing packet/allocator/OT implementation. Local::R8
    // rejects modulated RGB with no producer; raw-texture ignored RGB only is
    // canonicalized by the previously verified private-image rule.
    return SpritePacket(s,v,table,priority,incoming);
}
int32_t SortBox8003EE84(Services& s,uint32_t local,uint32_t table,uint32_t priority,uint32_t incoming){Ram v{s,local};return BoxPacket(s,v,table,priority,incoming);}
int32_t Sprite8001B590(Services& s,uint32_t x,uint32_t y,uint32_t source,uint32_t frame,uint32_t advance,uint32_t priority,uint32_t table){
    Local v;v.W16(4u,uint16_t(x-160u));v.W16(6u,uint16_t(y-120u));
    const uint32_t previous=uint32_t(Prefix(s,v,source,frame,advance));
    return SpritePacket(s,v,table,priority&65535u,previous);
}
int32_t SelectionSprite8001B5F4(Services& s,uint32_t x,uint32_t y,uint32_t source,uint32_t selected,uint32_t priority,uint32_t table){
    Local v;v.W16(4u,uint16_t(x-160u));v.W16(6u,uint16_t(y-120u));
    const uint32_t previous=uint32_t(SelectionPrefix(s,v,source,selected));
    return SpritePacket(s,v,table,priority&65535u,previous);
}
int32_t FastSprite8001BEE4(Services& s,uint32_t x,uint32_t y,uint32_t source,
                           uint32_t alternateClut,uint32_t table){
    // 8001BEE4 subtracts the 160/120 draw origin before calling 8001BE34.
    // The descriptor X is in VRAM words; only the 4-bit texel U is scaled.
    // 8001BE34 selects CLUT row 484 for any nonzero alternate-palette flag.
    PrivateSprite local{};
    local.attributes=s.Read32(source);
    local.x=static_cast<uint16_t>(x-160u);
    local.y=static_cast<uint16_t>(y-120u);
    local.width=s.Read16(source+8u);
    local.height=s.Read16(source+10u);
    const uint32_t tx=s.Read16(source+4u);
    const uint32_t ty=s.Read16(source+6u);
    local.page=static_cast<uint16_t>(TexturePage80043DF4(s,0u,1u,
        tx&0x3FC0u,ty&0xFF00u));
    local.u=static_cast<uint8_t>(tx*4);
    local.v=static_cast<uint8_t>(s.Read16(source+6u));
    local.clutX=s.Read16(source+12u);
    local.clutY=static_cast<uint16_t>(alternateClut!=0u?484u:s.Read16(source+14u));
    // The preceding 8001BE34 call leaves its selected CLUT row in V0. The
    // native GsSortFastSprite skip path preserves that incoming value.
    return SortPrivateSprite8003FA20(s,local,table,3u,local.clutY);
}
int32_t Box8001B6C4(Services& s,uint32_t x,uint32_t y,uint32_t width,uint32_t height,uint32_t color,uint32_t priority,uint32_t table){
    Local v;v.W16(4u,uint16_t(x-160u));v.W16(6u,uint16_t(y-120u));
    v.W16(8u,uint16_t(width));v.W16(10u,uint16_t(height));v.W32(0u,color&0xFF000000u);
    v.W8(12u,uint8_t(color>>16u));v.W8(13u,uint8_t(color>>8u));v.W8(14u,uint8_t(color));
    return BoxPacket(s,v,table,priority&65535u,color>>8u);
}
int32_t WorkSprite8001C550(Services& s,uint32_t x,uint32_t y,uint32_t source,uint32_t priority){
    const uint32_t table=0x80087288u+20u*s.Read32(0x8006EDA8u);
    return Sprite8001B590(s,Half(x),Half(y),source,0u,0u,priority,table);
}
int32_t WorkBox8001C4EC(Services& s,uint32_t x,uint32_t y,uint32_t width,uint32_t height,uint32_t color,uint32_t priority){
    const uint32_t table=0x80087288u+20u*s.Read32(0x8006EDA8u);
    return Box8001B6C4(s,Half(x),Half(y),width&65535u,height&65535u,color,priority,table);
}
bool TryCall(Services& s,uint32_t fn,std::initializer_list<uint32_t> args,int32_t& result){
    size_t n;
    switch(fn){
    case 0x8001C550u:case 0x8001B25Cu:case 0x80043DF4u:case 0x8003EF5Cu:n=4u;break;
    case 0x8001C4ECu:case 0x8001B5F4u:n=6u;break;
    case 0x8001B428u:n=3u;break;
    case 0x8001BEE4u:n=5u;break;
    case 0x8001B590u:case 0x8001B6C4u:n=7u;break;
    case 0x80044A24u:n=0u;break;
    default:return false;
    }
    if(args.size()!=n)throw std::invalid_argument("Loading packet argument count mismatch");
    const auto a=args.begin();
    switch(fn){
    case 0x8001C550u:result=WorkSprite8001C550(s,a[0],a[1],a[2],a[3]);break;
    case 0x8001C4ECu:result=WorkBox8001C4EC(s,a[0],a[1],a[2],a[3],a[4],a[5]);break;
    case 0x8001B590u:result=Sprite8001B590(s,a[0],a[1],a[2],a[3],a[4],a[5],a[6]);break;
    case 0x8001B5F4u:result=SelectionSprite8001B5F4(s,a[0],a[1],a[2],a[3],a[4],a[5]);break;
    case 0x8001B428u:result=SelectionPrefix8001B428(s,a[0],a[1],a[2]);break;
    case 0x8001BEE4u:result=FastSprite8001BEE4(s,a[0],a[1],a[2],a[3],a[4]);break;
    case 0x8001B6C4u:result=Box8001B6C4(s,a[0],a[1],a[2],a[3],a[4],a[5],a[6]);break;
    case 0x8001B25Cu:result=SpritePrefix8001B25C(s,a[0],a[1],a[2],a[3]);break;
    case 0x80043DF4u:result=TexturePage80043DF4(s,a[0],a[1],a[2],a[3]);break;
    case 0x8003EF5Cu:result=Link8003EF5C(s,a[0],a[1],a[2],a[3]);break;
    case 0x80044A24u:result=GpuType80044A24(s);break;
    }
    return true;
}
}
