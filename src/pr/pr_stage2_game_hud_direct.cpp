#include "pr_stage2_game_hud_direct.h"
#include "pr_stage2_loading_packets_direct.h"
#include <array>
#include <string>

namespace PrStage2GameHudDirect {
namespace {
constexpr uint32_t Gp = 0x8006EA40u;
int32_t S(uint32_t x) { return x < 0x80000000u ? int32_t(x) : int32_t(int64_t(x)-0x100000000LL); }
int32_t H(uint16_t x) { return x < 0x8000u ? int32_t(x) : int32_t(x)-65536; }
uint32_t Table(Services& s) { return 0x80087288u+20u*s.Read32(Gp+872u); }
int32_t Sprite(Services& s, uint32_t x, uint32_t y, uint32_t source,
               uint32_t frame, uint32_t priority) {
    return s.Call(0x8001B590u,{x,y,source,frame,0u,priority,Table(s)});
}
void Fast(Services& s, uint32_t x, uint32_t y, uint32_t source, uint32_t clut) {
    s.Call(0x8001BEE4u,{x,y,source,clut,Table(s)});
}
void Advance(Services& s, int32_t count, uint32_t bank, bool student) {
    for (int32_t i=0;i<count;++i) {
        const uint32_t p=bank+16u*uint32_t(i),sx=0x80087668u+2u*uint32_t(i),sy=0x800876B0u+2u*uint32_t(i);
        const int32_t step=S(s.Read32(p+12u));
        if (step>=24) continue;
        if (step >= (student?5:6)) {
            if (step>=22) { s.Write16(sx,4096u);s.Write16(sy,4096u); }
            else {
                if (S(s.Read32(p))>=8193) s.Write32(p,0u);
                const auto sine=s.Call(0x8003A1D0u,{s.Read32(p)});
                s.Write16(sx,uint16_t(sine));s.Write16(sy,4096u);
                s.Write32(p,s.Read32(p)+256u);
            }
        } else {
            if (student) s.Write32(p+4u,s.Read32(p+4u)+s.Read32(p+8u));
            s.Write16(sx,uint16_t(s.Read32(p+4u)+4096u));
            s.Write16(sy,uint16_t(s.Read32(p+4u)+4096u));
            if (!student) s.Write32(p+4u,s.Read32(p+4u)+s.Read32(p+8u));
            if (S(s.Read32(p+4u))>=4096) s.Write32(p+8u,uint32_t(-1024));
        }
        s.Write32(p+12u,s.Read32(p+12u)+1u);
    }
}
}
void DrawBanner8001DB9C(Services& s,int32_t mode,uint32_t workIndex) {
    if (mode<1 || mode>6) return;
    const uint32_t table=0x80087288u+20u*workIndex;
    s.Call(0x8001B590u,{101u,107u,0x8004E760u,0u,0u,2u,table});
    s.Call(0x8001B590u,{207u,107u,0x8004E770u+16u*uint32_t(mode-1),0u,0u,2u,table});
}
void DrawRatingFeedback8001DE08(Services& s,uint32_t restart,uint32_t direction,int32_t offset) {
    if (restart) s.Write32(Gp+184u,20u);
    const int32_t count=S(s.Read32(Gp+184u));
    if (count<=0) return;
    const bool fifth=count%5==0;
    const uint32_t y=uint32_t(H(uint16_t(uint32_t(offset)+(direction?(fifth?144u:150u):(fifth?152u:146u)))));
    Sprite(s,20u,y,direction?0x8004E730u:0x8004E720u,0u,3u);
    s.Write32(Gp+184u,s.Read32(Gp+184u)-1u);
}
int32_t DrawRating8001DF24(Services& s,uint32_t work,int32_t layout) {
    const int32_t flash=H(s.Read16(work+90u)),rating=H(s.Read16(work+78u));
    const int32_t target=H(s.Read16(work+88u)),score=S(s.Read32(work+48u));
    const int32_t offset=layout==0?30:0;
    if ((s.Read32(work)&0x200u)!=0u) {
        switch (s.Read16(work+398u)) {
        case 1:case 2:case 6:s.Write32(Gp+780u,0u);break;
        case 3:case 4:case 5:s.Write32(Gp+780u,1u);break;
        default:break;
        }
        DrawRatingFeedback8001DE08(s,1u,s.Read32(Gp+780u),offset);
    }
    DrawRatingFeedback8001DE08(s,0u,s.Read32(Gp+780u),offset);
    Fast(s,20u,uint32_t(offset+176),0x8004E740u,0u);
    uint32_t x=58u;
    for (char digit:std::to_string(score)) {
        Sprite(s,x,uint32_t(offset+176),0x8004E750u,uint32_t(int32_t(digit)-48),3u);
        x+=9u;
    }
    Fast(s,192u,uint32_t(H(uint16_t(offset+13*rating+136))),0x8004E6D0u,0u);
    std::array<uint32_t,4> clut{{1u,1u,1u,1u}};
    if (rating<0 || rating>=4 || (flash && (target<0 || target>=4)))
        throw std::out_of_range("S2 rating HUD index exceeds original private array");
    clut[size_t(rating)]=0u;
    if (flash) clut[size_t(target)]=(s.Read32(Gp+188u)&1u)==0u?1u:0u;
    constexpr uint32_t xs[]={256u,255u,260u,251u};
    for (uint32_t i=0;i<4u;++i)
        Fast(s,xs[i],uint32_t(offset+137+13*int32_t(i)),0x8004E6E0u+16u*i,clut[i]);
    const int32_t mode=H(s.Read16(0x800916D0u));
    s.Write32(Gp+188u,s.Read32(Gp+188u)+1u);
    if (mode==2) return Sprite(s,20u,53u,0x8004E6C0u,0u,3u);
    if (mode==1) return Sprite(s,28u,53u,0x8004E6B0u,0u,3u);
    return 1;
}
int32_t DrawStatus8001E2E4(Services& s,uint32_t work,int32_t layout) {
    const uint32_t first=s.WantsTmdPresentation()?s.Read32(0x800901C8u):0u;
    if (s.Read16(work+92u)) DrawBanner8001DB9C(s,H(s.Read16(work+94u)),s.Read32(Gp+872u));
    const int32_t result=DrawRating8001DF24(s,work,layout);
    if(s.WantsTmdPresentation()) s.ObserveStatus(work,layout,first,s.Read32(0x800901C8u));
    return result;
}
void AdvanceTeacher80023F20(Services& s,int32_t count) { Advance(s,count,0x800876F8u,false); }
void AdvanceStudent80024114(Services& s,int32_t count) { Advance(s,count,0x80087938u,true); }
void DrawNote80024418(Services& s,uint16_t x,uint16_t y,uint16_t slot,uint16_t type) {
    // 80024418 -> 8001C804/8001B338 -> GsSortSprite. Its private sprite
    // always uses raw texturing and zero rotation. Ignored RGB is canonicalized
    // just like the native loading sprites; no modulated color is invented.
    namespace Packets=PrStage2LoadingPacketsDirect;
    const uint32_t source=s.Read32(0x800540BCu+4u*uint32_t(H(type)));
    const int32_t sx=H(s.Read16(0x80087668u+2u*uint32_t(H(slot))));
    const int32_t sy=H(s.Read16(0x800876B0u+2u*uint32_t(H(slot))));
    const uint16_t width=s.Read16(source+8u),height=s.Read16(source+10u);
    const uint32_t tx=s.Read16(source+4u),ty=s.Read16(source+6u);
    const uint32_t page=uint32_t(Packets::TexturePage80043DF4(s,0u,1u,tx&0x3FC0u,ty&0xFF00u));
    const uint32_t u=uint8_t(4u*tx),v=uint8_t(s.Read16(source+6u));
    const uint32_t cx=uint32_t(H(s.Read16(source+12u))),cy=uint32_t(H(s.Read16(source+14u)));
    const int32_t mx=H(s.Read16(source+8u))/2,my=H(s.Read16(source+10u))/2;
    if (!width || !height) return;
    const uint32_t table=Table(s),packet=s.Read32(0x800901C8u);
    constexpr uint32_t attr=0x50000040u;
    uint32_t words;
    if (sx==4096 && sy==4096) {
        s.Write32(packet+4u,(page&31u)|((attr>>17u)&0x180u)|0xE1000200u|((attr>>23u)&0x60u));
        s.Write32(packet+8u,((attr>>5u)&0x02000000u)|((attr<<18u)&0x01000000u)|0x64000000u);
        const uint16_t px=uint16_t(uint32_t(x)-160u+s.Read16(0x800917AAu)-uint32_t(mx));
        const uint16_t py=uint16_t(uint32_t(y)-120u+s.Read16(0x800917ACu)-uint32_t(my));
        s.Write32(packet+12u,uint32_t(px)|(uint32_t(py)<<16u));
        s.Write32(packet+16u,u|(v<<8u)|(cy<<22u)|((cx<<12u)&0x003F0000u));
        s.Write32(packet+20u,uint32_t(width)|(uint32_t(height)<<16u));
        words=5u;
    } else {
        auto& g=s.MatrixGte();
        for (uint32_t i=0;i<8u;++i) g.matrix.words[i]=s.Read32(0x80091838u+4u*i);
        const int32_t factors[]={sx,sy,0};
        for (uint32_t i=0;i<9u;++i) {
            const uint32_t shift=16u*(i&1u),old=g.matrix.words[i/2u];
            const int32_t value=H(uint16_t(old>>shift));
            const uint16_t scaled=uint16_t(S(uint32_t(int64_t(value)*factors[i%3u]))>>12);
            g.matrix.words[i/2u]=(old&~(0xFFFFu<<shift))|(uint32_t(scaled)<<shift);
        }
        g.matrix.words[4]=uint32_t(H(uint16_t(g.matrix.words[4])));
        g.matrix.words[5]=uint32_t(H(uint16_t(x-160u)));
        g.matrix.words[6]=uint32_t(H(uint16_t(y-120u)));
        g.matrix.words[7]=g.h;
        const auto xy=[](int32_t a,int32_t b) {return uint32_t(uint16_t(a))|(uint32_t(uint16_t(b))<<16u);};
        g.vectorXY0=xy(-mx,-my);g.vectorZ0=0;
        g.vectorXY12={{xy(int32_t(width)-mx,-my),xy(-mx,int32_t(height)-my)}};
        g.vectorZ12={{0,0}};
        PrPsxGteDirect::ExecutePerspective(g,true);
        const std::array<uint32_t,3> first=g.sxy;
        g.vectorXY0=xy(int32_t(width)-mx,int32_t(height)-my);g.vectorZ0=0;
        PrPsxGteDirect::ExecutePerspective(g,false);
        const uint32_t right=uint8_t(u+width-1u),bottom=uint8_t(v+height-1u);
        s.Write32(packet+8u,first[0]);
        s.Write32(packet+4u,((attr>>5u)&0x02000000u)|((attr<<18u)&0x01000000u)|0x2C000000u);
        s.Write32(packet+16u,first[1]);
        s.Write32(packet+12u,u|(v<<8u)|(cy<<22u)|((cx<<12u)&0x003F0000u));
        s.Write32(packet+24u,first[2]);s.Write32(packet+28u,u|(bottom<<8u));
        s.Write32(packet+32u,g.sxy[2]);s.Write32(packet+36u,right|(bottom<<8u));
        s.Write32(packet+20u,right|(v<<8u)|((page&31u)<<16u)|((attr>>1u)&0x01800000u)|((attr>>7u)&0x00600000u));
        words=9u;
    }
    s.ObserveRailNote(packet,x,y,type);
    const uint32_t next=uint32_t(Packets::Link8003EF5C(s,packet,table,1u,words));
    s.Write32(0x800901C8u,next);
}
int32_t DrawRail80024744(Services& s,uint32_t work) {
    s.ObserveRail(work,true);
    const int32_t count=H(s.Read16(work+138u));
    if (count<=0) { s.ObserveRail(work,false); return count; }
    for (int32_t row=0;row<H(s.Read16(work+138u));++row) {
        const uint32_t p=work+2u*uint32_t(row);
        int32_t teacher=H(s.Read16(p+140u));
        if (teacher>=0) {
            AdvanceTeacher80023F20(s,teacher+18*row);
            if (s.Read16(work+122u)==1u) {
                teacher=H(s.Read16(p+140u));
                if (S(s.Read32(Gp+308u))==teacher) s.Write32(Gp+304u,s.Read32(Gp+304u)+1u);
                else {s.Write32(Gp+308u,uint32_t(teacher));s.Write32(Gp+304u,0u);}
                if (S(s.Read32(Gp+304u))>4) s.Write32(Gp+304u,4u);
                if (teacher>0) {
                    const uint32_t packet=s.WantsTmdPresentation()?s.Read32(0x800901C8u):0u;
                    s.Call(0x8001C550u,{uint32_t(H(uint16_t(15*teacher+26+4*H(s.Read16(Gp+304u))))),uint32_t(20*row+18),s.Read32(Gp+796u),0u});
                    if(s.WantsTmdPresentation()) s.ObservePortrait(packet,s.Read32(Gp+796u),
                        15*teacher+26+4*H(s.Read16(Gp+304u)),20*row+18);
                }
            }
        }
        int32_t student=H(s.Read16(p+158u));
        if (student>=0) {
            AdvanceStudent80024114(s,student+18*row);
            if (s.Read16(work+122u)==1u) {
                student=H(s.Read16(p+158u));
                if (S(s.Read32(Gp+316u))==student) s.Write32(Gp+312u,s.Read32(Gp+312u)+1u);
                else {s.Write32(Gp+316u,uint32_t(student));s.Write32(Gp+312u,1u);}
                if (S(s.Read32(Gp+312u))>=5) s.Write32(Gp+312u,4u);
                if (student>=0) {
                    const uint32_t packet=s.WantsTmdPresentation()?s.Read32(0x800901C8u):0u;
                    s.Call(0x8001C550u,{uint32_t(H(uint16_t(15*student+26+4*H(s.Read16(Gp+312u))))),uint32_t(20*row+16),0x8005400Cu,0u});
                    if(s.WantsTmdPresentation()) s.ObservePortrait(packet,0x8005400Cu,
                        15*student+26+4*H(s.Read16(Gp+312u)),20*row+16);
                }
            }
        }
        if (s.Read16(work+122u)==1u) {
            teacher=H(s.Read16(p+140u));const int32_t threshold=teacher>0?15*teacher+31:0;
            for (uint32_t i=0;i<4u;++i) {
                const uint32_t x=s.Read32(0x800540E8u+4u*i);
                s.Call(0x8001C550u,{x,uint32_t(H(s.Read16(0x800540E0u+4u*uint32_t(row)))),threshold>=S(x)?0x800540ACu:0x8005409Cu,3u});
            }
            for (uint32_t i=row>0?2u:0u;i<14u;++i) {
                const uint32_t x=s.Read32(0x80054100u+4u*i);
                s.Call(0x8001C550u,{x,uint32_t(H(s.Read16(0x800540F8u+4u*uint32_t(row)))),threshold>=S(x)?0x8005408Cu:0x8005407Cu,3u});
            }
        }
        for (uint32_t slot=0;slot<18u;++slot) {
            const int32_t type=int8_t(s.Read8(s.Read32(work+148u+4u*uint32_t(row))+slot));
            if (s.Read16(work+122u)==1u && uint16_t(type-1)<8u)
                s.DrawRailNote80024418(uint16_t(32u+15u*slot),uint16_t(24+20*row),uint16_t(17*row+int32_t(slot)),uint16_t(type));
        }
    }
    s.ObserveRail(work,false);
    return 0;
}
}
