#include "pr_stage2_save_ui_direct.h"

#include <string>
#include <cstdio>
#include <unordered_set>

namespace PrStage2SaveUiDirect {
namespace {
constexpr uint32_t Gp = 0x8006EA40u;
constexpr uint32_t Name = 0x80049244u;
constexpr uint32_t List = 0x80048E50u;
constexpr uint32_t Rows = 0x8007A590u;
constexpr uint32_t Filename = 0x8007CBE8u;
constexpr uint32_t SuffixScratch = 0x8007A570u;
std::unordered_set<Services*> g_cardCommunicationOwners;
int32_t S(uint32_t v) {
    return v < 0x80000000u ? int32_t(v) : int32_t(int64_t(v)-0x100000000LL);
}
int32_t H(uint16_t v) { return v < 0x8000u ? int32_t(v) : int32_t(v)-0x10000; }

// BIOS string operations use the live RAM bytes. Do not pad/truncate strcpy:
// a save name is followed immediately by the progress/options payload.
uint32_t Length(Services& s, uint32_t a) {
    uint32_t n=0;
    while (s.Read8(a+n)) ++n;
    return n;
}
void CopyString(Services& s,uint32_t dst,uint32_t src) {
    uint8_t c;
    do { c=s.Read8(src++); s.Write8(dst++,c); } while(c);
}
int32_t CompareString(Services& s,uint32_t a,uint32_t b,uint32_t n=0xFFFFFFFFu) {
    while(n--) {
        const uint8_t x=s.Read8(a++),y=s.Read8(b++);
        if(x!=y) return int32_t(x)-int32_t(y);
        if(!x) return 0;
    }
    return 0;
}
std::string PrivateString(Services& s,uint32_t a) {
    std::string text;
    for(uint8_t c;(c=s.Read8(a++))!=0;) text.push_back(char(c));
    return text;
}
void WriteString(Services& s,uint32_t a,const std::string& text) {
    for(unsigned char c:text) s.Write8(a++,c);
    s.Write8(a,0);
}
uint32_t FilenameSuffix(Services& s,uint32_t filename) {
    const uint32_t prefix=Length(s,s.Read32(Gp+136u));
    for(uint32_t i=0;i<20u;++i) s.Write8(SuffixScratch+i,s.Read8(filename+i));
    s.Write8(SuffixScratch+20u,0u);
    return S(Length(s,SuffixScratch)) < S(prefix) ? 0u : SuffixScratch+prefix;
}

struct DirectoryScan { uint32_t rows,free,matched; };
DirectoryScan ScanDirectory(Services& s,uint32_t suffix) {
    // 17B18's two output words and the match word are private stack locals.
    // Its card-selection/enumeration calls still execute before scanning RAM.
    const uint32_t selected=uint32_t(s.Call(0x800178C8u,{}));
    s.Call(0x80017354u,{selected});
    s.CallVoid(0x80025C44u,{Rows,1624u});
    uint32_t used=0,count=0,matched=0,row=Rows;
    for(uint32_t entry=0x8007A318u;entry<0x8007A570u;entry+=40u) {
        s.Write16(row+104u,0u);
        s.Write8(row+92u,0u);
        s.Write8(row,0u);
        if(!s.Read8(entry)) continue;
        // ADDIU wraps before signed division toward zero in the original.
        const uint32_t bytes=s.Read32(entry+24u);
        used+=uint32_t(S(bytes+8191u)/8192);
        const uint32_t length=Length(s,s.Read32(Gp+136u));
        if(CompareString(s,entry,s.Read32(Gp+136u),length)) continue;
        const uint32_t tail=FilenameSuffix(s,entry);
        if(!CompareString(s,tail,suffix)) matched=1u;
        s.Write16(row+104u,1u);
        CopyString(s,row,entry);
        CopyString(s,row+92u,tail);
        row+=108u;
        ++count;
    }
    const uint32_t free=15u-used;
    for(int32_t i=0;i<S(free);++i) {
        s.Write16(Rows+108u*count+104u,1u);
        ++count;
    }
    return {count,free,matched};
}
int32_t Remainder(Services& s,int32_t a,int32_t b,uint32_t zeroPc,uint32_t overflowPc) {
    if(!b) s.Break(zeroPc,7u);
    if(a==(-2147483647-1)&&b==-1) s.Break(overflowPc,6u);
    return a%b;
}

uint32_t PreviewString(Services& s,uint32_t dst,uint32_t src) {
    uint32_t count=0;
    // The source recomputes strlen on each iteration; preserve overlap cases.
    if(src) while(S(count)<S(Length(s,src))) {
        const uint8_t c=s.Read8(src+count);
        s.Write8(dst+count,s.Read8(0x800491C4u+c));
        ++count;
    }
    s.Write8(dst+count,0u);
    return dst+count;
}
int32_t ResetNameInput(Services& s) {
    InitializeEvent80017E58(s,Name);
    s.Write16(Name+24u,0u);
    s.Write16(Name+26u,0u);
    s.Write16(Name+22u,0u);
    for(uint32_t i=0;i<6u;++i) {
        s.Write8(Name+40u+i,0u);
        s.Write8(Name+28u+i,0u);
    }
    return S(PreviewString(s,Name+40u,0x8006EAF0u));
}
int32_t HandleListInput(Services& s,uint32_t input) {
    int32_t selected=H(s.Read16(List+20u));
    const auto enabled=[&](int32_t i){return H(s.Read16(List+22u+2u*uint32_t(i)));};
    if(input==64u) {
        const int32_t last=H(s.Read16(List+16u))-1;
        if(selected==last) { s.CallVoid(0x80025C8Cu,{32u}); return 2; }
        if(selected<last) {
            s.CallVoid(0x80025C8Cu,{32u});
            s.Write32(Gp+716u,1u);
            CopyString(s,Filename,Rows+108u*uint32_t(selected));
            return 1;
        }
    }
    if(input==0x1000u) {
        s.CallVoid(0x80025C8Cu,{0x1000u});
        const int32_t last=H(s.Read16(List+16u))-1;
        if(selected==last) {
            int32_t candidate=last-1;
            while(candidate>0&&!enabled(candidate)) --candidate;
            if(enabled(candidate)==1) selected=candidate;
        } else {
            selected-=H(s.Read16(List+14u));
            if(selected<0) selected=last;
        }
        while(enabled(selected)!=1) {
            --selected;
            if(selected<0) selected=H(s.Read16(List+16u))-1;
        }
    } else if(input==0x4000u) {
        s.CallVoid(0x80025C8Cu,{0x1000u});
        const int32_t last=H(s.Read16(List+16u))-1;
        if(selected==last) {
            int32_t candidate=0;
            while(candidate<selected&&!enabled(candidate)) ++candidate;
            if(enabled(candidate)==1) selected=candidate;
        } else {
            selected+=H(s.Read16(List+14u));
            if(selected>=last||enabled(selected)!=1) selected=last;
        }
    } else if(input==0x2000u) {
        s.CallVoid(0x80025C8Cu,{0x1000u});
        const int32_t size=H(s.Read16(List+16u));
        selected=Remainder(s,selected+1,size,0x80018490u,0x800184A8u);
        while(enabled(selected)!=1)
            selected=Remainder(s,selected+1,size,0x800184E0u,0x800184F8u);
    } else if(input==0x8000u) {
        s.CallVoid(0x80025C8Cu,{0x1000u});
        do {
            --selected;
            if(selected<0) selected=H(s.Read16(List+16u))-1;
        } while(enabled(selected)!=1);
    }
    s.Write16(List+20u,uint16_t(selected));
    return 0;
}
void DeleteNameCharacter(Services& s) {
    const int32_t count=H(s.Read16(Name+24u));
    if(count>0) {
        s.Write16(Name+24u,uint16_t(count-1));
        s.Write8(Name+28u+uint32_t(count-1),0u);
    }
}
int32_t HandleInput800185D0(Services& s,uint32_t input,int32_t state,uint32_t payload) {
    // Read the GP context only at the original call site, after sound callbacks.
    const auto prompt=[&](uint32_t event,int32_t choice,uint32_t busy) {
        Feedback80017E6C(s,event,s.Read32(Gp+132u),choice,busy);
    };
    switch(state) {
    case 2:
        if(input==64u||input==32u) {
            s.CallVoid(0x80025C8Cu,{32u});
            prompt(11u,input==64u?1:2,0u);
            return input==64u?4:23;
        }
        break;
    case 5: case 7:
        if(input==64u) {
            s.CallVoid(0x80025C8Cu,{32u});
            prompt(state==5?12u:14u,2,0u);
            return 2;
        }
        break;
    case 8:
        if(input==64u||input==32u) {
            s.CallVoid(0x80025C8Cu,{32u});
            prompt(13u,input==64u?1:2,input==64u?1u:0u);
            return input==64u?14:2;
        }
        break;
    case 10: {
        const uint32_t previous=uint32_t(H(s.Read16(Name+22u)));
        uint32_t cursor=previous;
        s.Write16(Name+26u,0u);
        if(input==0x1000u) {
            s.CallVoid(0x80025C8Cu,{0x1000u});
            cursor=previous==0u?56u:previous-1u<13u?0u:
                previous-14u<35u?previous-14u:previous-49u<6u?previous-7u:previous-1u;
        }
        if(input==0x4000u) {
            s.CallVoid(0x80025C8Cu,{0x1000u});
            cursor=previous<35u?previous+14u:previous-35u<7u?48u:
                previous-42u<6u?previous+7u:previous-48u<7u?55u:previous==55u?56u:0u;
        }
        if(input==0x2000u) {
            s.CallVoid(0x80025C8Cu,{0x1000u});
            cursor=uint32_t(Remainder(s,S(previous+1u),H(s.Read16(Name+20u)),0x80018834u,0x8001884Cu));
        }
        if(input==0x8000u) {
            s.CallVoid(0x80025C8Cu,{0x1000u});
            cursor=previous-1u;
            if(S(cursor)<0) cursor=uint32_t(H(s.Read16(Name+20u))-1);
        }
        s.Write16(Name+22u,uint16_t(cursor));
        uint32_t action=0;
        if(input&0xE0u) {
            const uint8_t c=s.Read8(s.Read32(Name+12u)+cursor);
            if(c==8u) {
                s.CallVoid(0x80025C8Cu,{64u});
                DeleteNameCharacter(s);
            } else if(c==10u&&(input==64u||input==32u)) {
                s.CallVoid(0x80025C8Cu,{32u});
                if(input==64u) {
                    const uint32_t src=s.Read8(Name+28u)?Name+28u:s.Read32(Gp+148u);
                    CopyString(s,payload+1u,src);
                }
                action=input==64u?1u:2u;
                s.Write16(Name+26u,uint16_t(action));
                s.Write32(Name+4u,action);
            } else if(c>=32u&&H(s.Read16(Name+24u))<6) {
                s.CallVoid(0x80025C8Cu,{256u});
                s.Write8(Name+28u+uint32_t(H(s.Read16(Name+24u))),c);
                const uint16_t next=uint16_t(s.Read16(Name+24u)+1u);
                s.Write16(Name+24u,next);
                s.Write8(Name+28u+uint32_t(H(next)),0u);
            }
        }
        if(!action) {
            if(input==16u) { s.CallVoid(0x80025C8Cu,{64u}); DeleteNameCharacter(s); }
            PreviewString(s,Name+40u,Name+28u);
        }
        if(action==1u) {
            Feedback80017E6C(s,5u,Name,1,1u);
            const auto scanned=ScanDirectory(s,payload+1u);
            s.Write16(0x8007ABE6u,uint16_t(scanned.free));
            s.Write16(0x8007ABE4u,uint16_t(scanned.rows));
            if(scanned.matched==1u) return 18;
            if(H(s.Read16(0x8007ABE6u))<=0) return 7;
            s.Write32(Gp+712u,1u);
            Feedback80017E6C(s,5u,Name,1,1u);
            return 15;
        }
        if(action==2u) { Feedback80017E6C(s,5u,Name,2,0u); return 2; }
        break;
    }
    case 11: {
        const int32_t result=HandleListInput(s,input);
        if(result==1) {
            if(s.Read8(Filename)) return 22;
            ResetNameInput(s); return 10;
        }
        if(result==2) { Feedback80017E6C(s,7u,List,1,0u); return 2; }
        break;
    }
    case 18:
        if(input==64u) {
            s.CallVoid(0x80025C8Cu,{32u}); prompt(15u,2,0u);
            ResetNameInput(s); return 10;
        }
        break;
    case 22:
        if(input==64u) {
            s.CallVoid(0x80025C8Cu,{32u}); prompt(19u,1,1u);
            const uint32_t suffix=FilenameSuffix(s,Filename);
            CopyString(s,payload+1u,suffix); return 15;
        }
        if(input==32u) {
            s.CallVoid(0x80025C8Cu,{32u}); prompt(19u,2,0u); return 11;
        }
        break;
    default: break;
    }
    return state;
}

int32_t TickCard80019458(Services& s,int32_t state,int32_t io,uint32_t payload) {
    switch(state) {
    case 4:
        if(io==1||io==4) return 6;
        if(io==3) return 5;
        if(io==5) return s.Read32(Gp+732u)?19:8;
        return 4;
    case 6: {
        const auto scan=ScanDirectory(s,payload+1u);
        s.Write16(0x8007ABE4u,uint16_t(scan.rows));
        s.Write16(0x8007ABE6u,uint16_t(scan.free));
        return S(scan.rows)>0?9:7;
    }
    case 8: return io==3?5:state;
    case 9: {
        bool preferFree=true;
        if(s.Read32(Gp+712u)) {
            preferFree=false;
            for(uint32_t i=0;i<600u;++i) {
                const uint8_t a=s.Read8(0x8007A318u+i),b=s.Read8(0x8007CC74u+i);
                if(a!=b) { preferFree=true; break; }
            }
        }
        const auto scan=ScanDirectory(s,payload+1u);
        s.Write16(0x8007ABE4u,uint16_t(scan.rows));
        s.Write16(0x8007ABE6u,uint16_t(scan.free));
        InitializeEvent80017E58(s,List);
        s.Write8(Filename,0u);
        for(uint32_t i=0;i<15u;++i) {
            const uint32_t row=Rows+108u*i;
            if(int32_t(i)<H(s.Read16(0x8007ABE4u))) {
                s.Write16(List+22u+2u*i,s.Read16(row+104u));
                const uint32_t name=H(s.Read16(row+104u))==1?row+92u:0u;
                s.Write32(List+56u+4u*i,name);
                s.Write16(List+632u+2u*i,s.Read16(row+106u));
            } else {
                s.Write16(List+22u+2u*i,0u);
                s.Write32(List+56u+4u*i,0u);
            }
            PreviewString(s,List+120u+32u*i,s.Read32(List+56u+4u*i));
        }
        if(!s.Read8(payload+1u)) preferFree=true;
        int32_t selected=-1;
        for(int32_t i=0;i<H(s.Read16(0x8007ABE4u));++i) {
            if(!s.Read16(List+22u+2u*uint32_t(i))) continue;
            const uint32_t name=s.Read32(List+56u+4u*uint32_t(i));
            if(preferFree?!s.Read8(name):(name&&!CompareString(s,name,payload+1u))) {
                selected=i; break;
            }
        }
        s.Write16(List+18u,s.Read16(0x8007ABE4u));
        if(selected<0) selected=s.Read16(List+12u)*s.Read16(List+14u);
        s.Write16(List+20u,uint16_t(selected));
        s.Write16(List+52u,1u);
        return 11;
    }
    case 10: case 11: return io==3?4:state;
    case 14:
        if(io==3) return 2;
        s.Call(0x80017B60u,{}); return 4;
    case 15: {
        if(io==3) return 2;
        uint32_t i=0;
        while(i<15u&&H(s.Read16(Rows+108u*i+104u))!=1) ++i;
        if(i==15u) s.Exit(0x80047F3Cu,{1u});
        // 4800C's two local formatting buffers stay on the native stack.
        // Only the filename and the 32 big-endian title glyphs are public RAM.
        const std::string prefix=PrivateString(s,s.Read32(Gp+136u));
        WriteString(s,Filename,prefix+PrivateString(s,payload+1u));
        const int32_t stage=s.Call(0x80016314u,{});
        std::string filtered;
        for(uint32_t j=0;s.Read8(payload+1u+j)&&j<12u;++j) {
            const uint8_t c=s.Read8(payload+1u+j);
            filtered.push_back(s.Read8(0x8006E999u+c)&7u?char(c):'?');
        }
        std::string title=PrivateString(s,s.Read32(Gp+160u));
        if(!filtered.empty()) title+=" \""+filtered+"\" ST"+std::to_string(stage);
        for(uint32_t j=0;j<32u;++j) {
            const uint16_t glyph=j<title.size()?s.Read16(0x800490E4u+2u*uint8_t(title[j])):0u;
            s.Write8(0x8007CC08u+2u*j,uint8_t(glyph>>8u));
            s.Write8(0x8007CC09u+2u*j,uint8_t(glyph));
        }
        s.CallVoid(0x80025C44u,{0x8007ABE8u,8192u});
        s.Call(0x80017C08u,{0x8007ABE8u,0x8007CC08u});
        s.Call(0x80025C64u,{payload,0x8007ADE8u,4876u});
        if(s.Call(0x80017A10u,{Filename,0x8007ABE8u,1u})<0) return 2;
        s.Write32(Gp+716u,1u); s.Write32(Gp+720u,1u);
        return 23;
    }
    case 21: InitializeEvent80017E58(s,s.Read32(Gp+132u)); return 2;
    default: return state;
    }
}

// 18FB0 owns its event on the native C++ stack. A public 180D8 caller instead
// supplies RAM storage, which can alias the context or GP fields. Keep both
// forms without assigning a fabricated PSX address to the private local.
template<class ReadEvent, class WriteEvent>
uint32_t SelectEvent(Services& s, uint32_t state, uint32_t context,
                     ReadEvent read, WriteEvent write) {
    switch (state) {
    case 2u: case 21u: write(11u); break;
    case 5u: write(12u); break;
    case 7u: write(14u); break;
    case 8u: write(13u); break;
    case 10u: context=0x80049244u; write(5u); break;
    case 11u: context=0x80048E50u; write(7u); break;
    case 12u: context=0x80048E50u; write(8u); break;
    case 13u: context=0x80048E50u; write(9u); break;
    case 18u: write(15u); break;
    case 19u: write(18u); break;
    case 22u: write(19u); break;
    default: break;
    }
    const uint32_t previous=s.Read32(Gp+704u);
    uint32_t event=read();
    if (previous!=event) {
        InitializeEvent80017E58(s,context);
        event=read();
    }
    s.Write32(Gp+704u,event);
    return context;
}
}

int32_t InitializeEvent80017E58(Services& s,uint32_t context) {
    s.Write32(context,1u);
    s.Write32(context+4u,0u);
    s.Write32(context+8u,0u);
    return 1;
}

int32_t Feedback80017E6C(Services& s,uint32_t event,uint32_t context,
                        int32_t choice,uint32_t busy) {
    if (choice>=0) s.Write32(context+4u,uint32_t(choice));
    s.Write32(context+8u,busy);
    s.Write32(context,1u);
    int32_t result=0;
    for (unsigned frame=0;frame<20u;++frame) {
        s.Call(0x8001E750u,{event,context});
        s.Call(0x80035560u,{0u});
        result=s.Call(0x8001EA00u,{0u});
    }
    return result;
}

uint32_t SelectEvent800180D8(Services& s,uint32_t state,uint32_t eventAddress,
                            uint32_t context) {
    return SelectEvent(s,state,context,[&]{return s.Read32(eventAddress);},
                       [&](uint32_t event){s.Write32(eventAddress,event);});
}

int32_t BeginCardCommunication80017524(Services& s) {
    g_cardCommunicationOwners.insert(&s);
    s.Call(0x80035744u,{});
    s.Call(0x800354C0u,{0u});
    const int32_t result=s.Call(0x800170C4u,{1u});
    s.Write32(0x800917E8u,0u);
    s.Write32(0x800917F0u,0u);
    s.Write32(0x800917ECu,0u);
    s.Write32(0x800917F4u,0u);
    return result;
}

int32_t EndCardCommunication80017574(Services& s) {
    const int32_t result=s.Call(0x8001724Cu,{});
    g_cardCommunicationOwners.erase(&s);
    return result;
}

int32_t ClearSoftwareCardEvents80016FC0(Services& s) {
    int32_t result=0;
    for (uint32_t i=0; i<4u; ++i)
        result=s.Call(0x800489C0u,{s.Read32(Gp+664u+4u*i)});
    return result;
}

int32_t PollSoftwareCardEvent80016E18(Services& s) {
    int32_t result=0;
    for (uint32_t i=0; i<4u; ++i) {
        if (s.Call(0x800489C0u,{s.Read32(Gp+664u+4u*i)})==1) result=int32_t(i+1u);
    }
    const int32_t remaining=S(s.Read32(Gp+700u))-1;
    s.Write32(Gp+700u,uint32_t(remaining));
    return remaining<0 ? 2 : result;
}

int32_t WaitSoftwareCardEvent80016EB8(Services& s) {
    for (int32_t remaining=300; remaining>0; --remaining) {
        for (uint32_t i=0; i<4u; ++i)
            if (s.Call(0x800489C0u,{s.Read32(Gp+664u+4u*i)})==1)
                return int32_t(i+1u);
        s.Call(0x80035560u,{0u});
    }
    return 2;
}

int32_t ClearHardwareCardEvents8001707C(Services& s) {
    int32_t result=0;
    for (uint32_t i=0; i<4u; ++i)
        result=s.Call(0x800489C0u,{s.Read32(Gp+680u+4u*i)});
    return result;
}

int32_t WaitHardwareCardEvent80017008(Services& s) {
    for (;;) {
        for (uint32_t i=0; i<4u; ++i)
            if (s.Call(0x800489C0u,{s.Read32(Gp+680u+4u*i)})==1)
                return int32_t(i+1u);
        s.AwaitDeviceProgress(0x800489C0u);
    }
}

int32_t DirectoryNameExists80017900(Services& s, uint32_t name) {
    for (uint32_t row=0; row<15u; ++row) {
        const uint32_t entry=0x8007A318u+40u*row;
        if (!s.Read8(entry)) continue;
        if (CompareString(s,entry,name,21u)==0) return 1;
    }
    return 0;
}

int32_t FreeCardBlocks80017354(Services& s, int32_t selected) {
    int32_t used=0;
    for (int32_t i=0; i<selected; ++i) {
        const uint32_t entry=0x8007A318u+40u*uint32_t(i);
        const uint32_t bytes=s.Read32(entry+24u);
        used += (S(bytes)+8191)/8192;
    }
    return 15-used;
}

int32_t SubmitCardWrite80017454(Services& s, uint32_t card, uint32_t slot,
                                uint32_t name, uint32_t buffer,
                                uint32_t blocks, uint32_t check) {
    char path[64]{};
    const std::string suffix=PrivateString(s,name);
    const int n=std::snprintf(path,sizeof(path),"bu%1d%1d:%s",
                              S(card),S(slot),suffix.c_str());
    if (n<0 || static_cast<std::size_t>(n)>=sizeof(path)) return -1;
    constexpr uint32_t kScratch=0x800B1400u;
    for (int i=0;i<=n;++i) s.Write8(kScratch+uint32_t(i),uint8_t(path[i]));
    if (check==1u) {
        const int32_t probe=s.Call(0x80048A60u,{kScratch,(blocks<<16u)|0x200u});
        if (probe==-1) return -1;
        s.Call(0x80048A90u,{uint32_t(probe)});
    }
    const int32_t fd=s.Call(0x80048A60u,{kScratch,32770u});
    if (fd==-1) return -1;
    s.Write32(Gp+696u,uint32_t(fd));
    s.Call(0x80016FC0u,{});
    s.Call(0x80048A80u,{uint32_t(fd),buffer,blocks<<13u});
    return 0;
}

int32_t WriteCard80017A10(Services& s,uint32_t name,uint32_t buffer,uint32_t blocks) {
    for (int32_t attempt=4; attempt>0; --attempt) {
        const int32_t scan=DirectoryNameExists80017900(s,name);
        (void)SubmitCardWrite80017454(s,s.Read32(Gp+128u),s.Read32(Gp+124u),
                                      name,buffer,blocks,scan!=1 ? 1u : 0u);
        s.Call(0x80035560u,{4u});
        const int32_t poll=s.Call(0x80016EB8u,{});
        s.Call(0x80048A90u,{s.Read32(Gp+696u)});
        if (poll==1) return 0;
    }
    return -1;
}

int32_t FormatCard80017B60(Services& s) {
    constexpr uint32_t kScratch=0x800B1400u;
    const int n=std::snprintf(reinterpret_cast<char*>(nullptr),0,"bu%1d%1d:",
                              S(s.Read32(Gp+128u)),S(s.Read32(Gp+124u)));
    if (n<0 || n>=63) return 3;
    char path[64]{};
    std::snprintf(path,sizeof(path),"bu%1d%1d:",S(s.Read32(Gp+128u)),S(s.Read32(Gp+124u)));
    for (int i=0;i<=n;++i) s.Write8(kScratch+uint32_t(i),uint8_t(path[i]));
    int32_t result=0;
    for (int32_t attempts=3; attempts>0; --attempts) {
        s.Call(0x8001707Cu,{});
        s.Call(0x80048AB0u,{kScratch});
        const int32_t poll=s.Call(0x80017008u,{});
        if (poll==1) return 1;
        result=(poll==3)?3:result;
        if (poll==3) break;
    }
    return result;
}

int32_t PollCard80017594(Services& s) {
    switch (s.Read32(0x800917E8u)) {
    case 0u:
        s.Call(0x80047EA4u,{0u});
        s.Write32(0x800917E8u,1u);
        s.Write32(0x800917ECu,0u);
        s.Write32(Gp+700u,300u);
        break;
    case 1u: {
        const int32_t result=s.Call(0x80016E18u,{});
        if (!result) break;
        if (result==1) {
            const uint32_t loaded=s.Read32(0x800917F4u);
            s.Write32(0x800917F0u,1u);
            s.Write32(0x800917E8u,loaded==1u?4u:2u);
        } else if (result==4) {
            s.Write32(0x800917F0u,4u);
            s.Call(0x8001707Cu,{});
            s.Call(0x80047EE4u,{0u});
            s.Call(0x80017008u,{});
            s.Write32(0x800917E8u,2u);
            s.Write32(0x800917F4u,0u);
        } else {
            s.Write32(0x800917F0u,result==3?3u:uint32_t(-3));
            s.Write32(0x800917E8u,4u);
            s.Write32(0x800917F4u,0u);
        }
        break;
    }
    case 2u:
        s.Call(0x80016FC0u,{});
        s.Call(0x80047EB4u,{0u});
        s.Write32(0x800917E8u,3u);
        s.Write32(Gp+700u,300u);
        break;
    case 3u: {
        const int32_t result=s.Call(0x80016E18u,{});
        if (!result) break;
        s.Write32(0x800917E8u,4u);
        s.Write32(0x800917F4u,0u);
        if (result==1) s.Write32(0x800917F4u,1u);
        else s.Write32(0x800917F0u,result==3?3u:result==4?5u:2u);
        break;
    }
    case 4u: {
        const uint32_t status=s.Read32(0x800917F0u);
        s.Write32(0x800917E8u,0u);
        s.Write32(0x800917ECu,status);
        break;
    }
    default: break;
    }
    return S(s.Read32(0x800917ECu));
}

int32_t SnapshotCardDirectory80017B18(Services& s,uint32_t channel,uint32_t count) {
    const uint32_t selected=uint32_t(s.Call(0x800178C8u,{}));
    s.Write32(channel,selected);
    s.Write32(count,uint32_t(s.Call(0x80017354u,{selected})));
    return 1;
}

int32_t SnapshotDirectory80018F70(Services& s) {
    // 17B18's two outputs are private, unused stack locals in this caller.
    // Execute its side-effecting callees without fabricating RAM for them.
    const uint32_t selected=uint32_t(s.Call(0x800178C8u,{}));
    s.Call(0x80017354u,{selected});
    return s.Call(0x80025C64u,{0x8007A318u,0x8007CC74u,600u});
}

int32_t RunDispatcher80018FB0(Services& s,uint32_t payload,uint32_t input,
                             uint32_t card,uint32_t state,uint32_t event) {
    uint32_t context=s.Read32(Gp+132u);
    s.Write32(Gp+720u,0u);
    s.Write32(Gp+728u,0u);
    InitializeEvent80017E58(s,context);
    while (state!=23u) {
        uint32_t pad=uint32_t(s.Call(0x80035510u,{1u}));
        if (pad==s.Read32(Gp+708u)) pad=0u;
        else s.Write32(Gp+708u,pad);
        if (pad) state=uint32_t(s.Call(input,{pad,state,payload}));
        const uint32_t io=uint32_t(PollCard80017594(s));
        s.Write32(Gp+724u,io);
        if (io) {
            state=uint32_t(s.Call(card,{state,io,payload}));
            context=SelectEvent(s,state,context,[&]{return event;},
                                [&](uint32_t value){event=value;});
        }
        if (state-15u<2u) {
            s.Write32(context+8u,1u);
            s.Write32(context,1u);
        } else {
            const uint32_t blink=s.Read32(Gp+728u);
            if (blink) s.Write32(Gp+728u,S(blink)<19?blink+1u:0u);
            else {
                const uint32_t visible=s.Read32(context);
                s.Write32(Gp+728u,1u);
                s.Write32(context,visible!=1u?1u:0u);
            }
        }
        s.Call(0x8001E750u,{event,context});
        s.Call(0x80035560u,{0u});
        s.Call(0x8001EA00u,{0u});
    }
    Feedback80017E6C(s,event,context,-1,0u);
    return S(s.Read32(Gp+720u));
}

int32_t RunSave80019148(Services& s,uint32_t payload) {
    s.Call(0x80020110u,{0u,3u,2u,1u});
    s.Call(0x80025C64u,{0x800544F8u,0x8007CC50u,36u});
    s.Write32(Gp+716u,0u);
    s.Write32(Gp+732u,0u);
    BeginCardCommunication80017524(s);
    RunDispatcher80018FB0(s,payload,0x800185D0u,0x80019458u,21u,11u);
    SnapshotDirectory80018F70(s);
    EndCardCommunication80017574(s);
    return S(s.Read32(Gp+716u));
}

int32_t BuildSaveBlockHeader80017C08(Services& s, uint32_t destination,
                                     uint32_t encodedTitle) {
    constexpr uint32_t kDestination = 0x8007ABE8u;
    constexpr uint32_t kTitle = 0x8007CC08u;
    constexpr uint32_t kHeaderBytes = 512u;
    constexpr uint32_t kTitleOffset = 4u;
    constexpr uint32_t kTitleBytes = 64u;
    constexpr uint32_t kIcon0Source = 0x80010004u + 8u * 4u;
    constexpr uint32_t kIcon1Source = 0x80010004u + 40u * 4u;
    constexpr uint32_t kIcon2Source = 0x800100C4u + 24u * 4u;
    constexpr uint32_t kIcon0Destination = 128u;
    constexpr uint32_t kIcon1Destination = 256u;
    constexpr uint32_t kIcon2Destination = 384u;
    constexpr uint32_t kIconBytes = 128u;
    // These are the eight fixed little-endian words copied by the original
    // routine before its three 128-byte icon chunks.
    constexpr uint32_t kFixed[] = {
        0x00DF0000u, 0x00BB00DDu, 0x00D90199u, 0x00B200B9u,
        0x008F0092u, 0x008C008Du, 0x0069006Bu, 0x1A660047u};
    if (destination != kDestination || encodedTitle != kTitle)
        throw std::logic_error("S2 80017C08 received an unexpected buffer");
    for (uint32_t i=0; i<kHeaderBytes; ++i) s.Write8(destination+i,0u);
    s.Write8(destination+0u,'S');
    s.Write8(destination+1u,'C');
    s.Write8(destination+2u,0x13u);
    s.Write8(destination+3u,1u);
    // strcpy in the source is bounded only by the 64-byte temporary. Preserve
    // the source's terminator and reject an unterminated title rather than
    // writing into the fixed header fields that follow it.
    bool terminated=false;
    for (uint32_t i=0; i<kTitleBytes; ++i) {
        const uint8_t value=s.Read8(encodedTitle+i);
        s.Write8(destination+kTitleOffset+i,value);
        if (value==0u) { terminated=true; break; }
    }
    if (!terminated) throw std::runtime_error("S2 80017C08 title is not terminated");
    for (uint32_t i=0; i<sizeof(kFixed)/sizeof(kFixed[0]); ++i)
        s.Write32(destination+96u+4u*i,kFixed[i]);
    const uint32_t sources[] = {kIcon0Source,kIcon1Source,kIcon2Source};
    const uint32_t targets[] = {kIcon0Destination,kIcon1Destination,kIcon2Destination};
    for (uint32_t chunk=0; chunk<3u; ++chunk)
        for (uint32_t i=0; i<kIconBytes; ++i)
            s.Write8(destination+targets[chunk]+i,s.Read8(sources[chunk]+i));
    // The final qword load is the value returned by the MIPS routine. Its
    // caller ignores it, but retaining the low word keeps the direct ABI.
    return S(s.Read32(kIcon2Source+120u));
}

bool TryCall(Services& s,uint32_t fn,std::initializer_list<uint32_t> args,int32_t& result) {
    const bool cardActive=g_cardCommunicationOwners.find(&s)!=g_cardCommunicationOwners.end();
    size_t count;
    switch (fn) {
    case 0x80026784u: case 0x80017B08u: case 0x80018F70u:
    case 0x80017524u: case 0x80017574u: case 0x80017594u: count=0u; break;
    case 0x80018060u: count=0u; break;
    case 0x80016314u: count=0u; break;
    case 0x800161A8u: count=1u; break;
    case 0x80019148u: case 0x80017E58u: count=1u; break;
    case 0x80017C08u: count=2u; break;
    case 0x80017900u: count=1u; break;
    case 0x80017354u: count=1u; break;
    case 0x80017454u: count=6u; break;
    case 0x80017A10u: count=3u; break;
    case 0x80016FC0u: case 0x80016E18u: case 0x80016EB8u:
    case 0x8001707Cu: case 0x80017008u: case 0x80017B60u: count=0u; break;
    case 0x80017FC4u: count=2u; break;
    case 0x800181D0u: count=1u; break;
    case 0x80017B18u: count=2u; break;
    case 0x800180D8u: count=3u; break;
    case 0x80017E6Cu: count=4u; break;
    case 0x80018FB0u: count=5u; break;
    case 0x800185D0u: case 0x80019458u: count=3u; break;
    default: return false;
    }
    if (args.size()!=count) throw std::invalid_argument("S2 save UI argument count mismatch");
    const auto a=args.begin();
    switch (fn) {
    case 0x80026784u: result=S(0x800544F8u); break;
    case 0x80017B08u: result=S(0x8007A318u); break;
    case 0x800161A8u: result=S(s.Read32(0x80048DD8u+(a[0]<<2u))); break;
    case 0x80016314u: result=S(s.Read32(0x80048DD8u+(s.Read32(0x80092F3Cu)<<2u))); break;
    case 0x80018060u:
        result=ResetNameInput(s); break;
    case 0x80017FC4u: {
        result=S(PreviewString(s,a[0],a[1]));
        break;
    }
    case 0x800181D0u:
        result=HandleListInput(s,a[0]); break;
    case 0x80017E58u: result=InitializeEvent80017E58(s,a[0]); break;
    case 0x80017E6Cu: result=Feedback80017E6C(s,a[0],a[1],S(a[2]),a[3]); break;
    case 0x800180D8u: result=S(SelectEvent800180D8(s,a[0],a[1],a[2])); break;
    case 0x80017524u: result=BeginCardCommunication80017524(s); break;
    case 0x80017574u: result=EndCardCommunication80017574(s); break;
    case 0x80017594u: result=PollCard80017594(s); break;
    case 0x80017B18u: result=SnapshotCardDirectory80017B18(s,a[0],a[1]); break;
    case 0x80018F70u: result=SnapshotDirectory80018F70(s); break;
    case 0x80018FB0u: result=RunDispatcher80018FB0(s,a[0],a[1],a[2],a[3],a[4]); break;
    case 0x80019148u: result=RunSave80019148(s,a[0]); break;
    case 0x80017C08u: result=BuildSaveBlockHeader80017C08(s,a[0],a[1]); break;
    case 0x80017900u: if(!cardActive)return false; result=DirectoryNameExists80017900(s,a[0]); break;
    case 0x80017354u: if(!cardActive)return false; result=FreeCardBlocks80017354(s,S(a[0])); break;
    case 0x80017454u: if(!cardActive)return false; result=SubmitCardWrite80017454(s,a[0],a[1],a[2],a[3],a[4],a[5]); break;
    case 0x80017A10u: if(!cardActive)return false; result=WriteCard80017A10(s,a[0],a[1],a[2]); break;
    case 0x80016FC0u: if(!cardActive)return false; result=ClearSoftwareCardEvents80016FC0(s); break;
    case 0x80016E18u: if(!cardActive)return false; result=PollSoftwareCardEvent80016E18(s); break;
    case 0x80016EB8u: if(!cardActive)return false; result=WaitSoftwareCardEvent80016EB8(s); break;
    case 0x8001707Cu: if(!cardActive)return false; result=ClearHardwareCardEvents8001707C(s); break;
    case 0x80017008u: if(!cardActive)return false; result=WaitHardwareCardEvent80017008(s); break;
    case 0x80017B60u: if(!cardActive)return false; result=FormatCard80017B60(s); break;
    case 0x800185D0u: result=HandleInput800185D0(s,a[0],S(a[1]),a[2]); break;
    case 0x80019458u: result=TickCard80019458(s,S(a[0]),S(a[1]),a[2]); break;
    }
    return true;
}
}
