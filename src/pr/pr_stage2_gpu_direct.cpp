#include "pr_stage2_gpu_direct.h"

namespace PrStage2GpuDirect {
namespace {
uint32_t U(int32_t v) { return static_cast<uint32_t>(v); }
int32_t S(uint32_t v) {
    return v<=0x7FFFFFFFu ? static_cast<int32_t>(v)
        : static_cast<int32_t>(static_cast<int64_t>(v)-0x100000000LL);
}
uint32_t Slot(uint32_t index) { return 0x80094448u+96u*index; }
using PrStage2LifecycleDirect::ImageRect;
using PrStage2LifecycleDirect::GpuCommandSource;
struct ImageRef {
    Services& s;
    uint32_t address = 0;
    ImageRect* local = nullptr;
    GpuCommandSource identity{};
    uint16_t Half(uint32_t offset) const {
        if (!local) return s.Read16(address+offset);
        switch (offset) {
        case 0: return local->x; case 2: return local->y;
        case 4: return local->width; case 6: return local->height;
        default: throw std::out_of_range("Private image halfword offset");
        }
    }
    void Half(uint32_t offset,uint16_t value) const {
        if (!local) { s.Write16(address+offset,value); return; }
        switch (offset) {
        case 0: local->x=value; break; case 2: local->y=value; break;
        case 4: local->width=value; break; case 6: local->height=value; break;
        default: throw std::out_of_range("Private image halfword offset");
        }
    }
    uint32_t Word(uint32_t offset) const {
        if (!local) return s.Read32(address+offset);
        const uint32_t low=Half(offset),high=Half(offset+2u);
        return low|(high<<16u);
    }
    GpuCommandSource Source() const { return local ? identity : GpuCommandSource{address,{}}; }
};
int32_t ClearImageBody(Services& s,ImageRef rect,uint32_t color);

void RunCommand(Services& s,uint32_t function,uint32_t first,uint32_t second) {
    if (function==0x80046840u) StartLinkedDma80046840(s,first);
    else if (function==0x800460ACu) ClearImage800460AC(s,first,second);
    else s.Call(function,{first,second});
}
void RunImageCommand(Services& s,uint32_t function,ImageRef source,uint32_t argument) {
    if (!source.local) { RunCommand(s,function,source.address,argument); return; }
    if (function==0x800460ACu) ClearImageBody(s,source,argument);
    else s.CallWithPrivateImage(function,*source.local,source.Source(),{},{argument});
}
bool DmaBusy(Services& s) { return (s.Read32(s.Read32(0x8005D814u))&0x01000000u)!=0u; }
void WaitCommandReady(Services& s) {
    const uint32_t status=s.Read32(0x8005D808u);
    while ((s.Read32(status)&0x04000000u)==0u) s.AwaitDeviceProgress(status);
}
}

int32_t ResetTimeout80047144(Services& s) {
    const uint32_t deadline=U(s.Call(0x80035560u,{0xFFFFFFFFu}))+240u;
    s.Write32(0x8005D84Cu,deadline);
    s.Write32(0x8005D850u,0u);
    return S(deadline);
}

int32_t CheckTimeout80047178(Services& s) {
    const int32_t now=s.Call(0x80035560u,{0xFFFFFFFFu});
    bool expired=S(s.Read32(0x8005D84Cu))<now;
    if (!expired) {
        const uint32_t count=s.Read32(0x8005D850u);
        s.Write32(0x8005D850u,count+1u);
        expired=S(count)>0xF0000;
    }
    if (!expired) return 0;
    // Keep even the first discarded status read: MMIO reads can have effects.
    const uint32_t status=s.Read32(0x8005D808u);
    s.Read32(status);
    const uint32_t head=s.Read32(0x8005D838u);
    const uint32_t madr=s.Read32(0x8005D80Cu);
    const uint32_t tail=s.Read32(0x8005D83Cu);
    const uint32_t address=s.Read32(madr);
    const uint32_t chcr=s.Read32(0x8005D814u);
    const uint32_t gpu=s.Read32(status), dma=s.Read32(chcr);
    s.Call(0x80047FFCu,{0x800125F0u,(head-tail)&63u,gpu,dma,address});
    const uint32_t function=s.Read32(0x8005D828u);
    const GpuCommandSource first=s.ReadGpuCommandSource();
    const uint32_t second=s.Read32(0x8005D830u);
    s.CallWithGpuCommandSource(0x80047FFCu,0x80012624u,function,first,second);
    const uint32_t mask=U(s.Call(0x800358C0u,{0u}));
    s.Write32(0x8005D83Cu,0u);
    const uint32_t resetTail=s.Read32(0x8005D83Cu);
    s.Write32(0x8005D848u,mask);
    s.Write32(0x8005D838u,resetTail);
    s.Write32(s.Read32(0x8005D814u),0x401u);
    const uint32_t priority=s.Read32(0x8005D824u);
    s.Write32(priority,s.Read32(priority)|0x800u);
    s.Write32(s.Read32(0x8005D808u),0x02000000u);
    s.Write32(s.Read32(0x8005D808u),0x01000000u);
    s.Call(0x800358C0u,{s.Read32(0x8005D848u)});
    return -1;
}

int32_t StartLinkedDma80046840(Services& s,uint32_t head) {
    s.Write32(s.Read32(0x8005D808u),0x04000002u);
    s.Write32(s.Read32(0x8005D80Cu),head);
    s.Write32(s.Read32(0x8005D810u),0u);
    const uint32_t chcr=s.Read32(0x8005D814u);
    s.Write32(chcr,0x01000401u);
    return S(chcr); // Original V0 is the register address, not a success flag.
}

int32_t DrainQueue80046BC4(Services& s) {
    if (DmaBusy(s)) return 1;
    const uint32_t mask=U(s.Call(0x800358C0u,{0u}));
    const uint32_t head=s.Read32(0x8005D838u), tail=s.Read32(0x8005D83Cu);
    s.Write32(0x8005D844u,mask);
    if (head!=tail && !DmaBusy(s)) {
        do {
            const uint32_t next=(s.Read32(0x8005D83Cu)+1u)&63u;
            if (next==s.Read32(0x8005D838u) && s.Read32(0x8005D740u)==0u)
                s.Call(0x800357A4u,{2u,0u});
            WaitCommandReady(s);
            // The original rereads the tail separately for each field, and
            // rereads it again after callbacks before recording/advancing.
            const uint32_t functionIndex=s.Read32(0x8005D83Cu);
            const uint32_t firstIndex=s.Read32(0x8005D83Cu);
            const uint32_t first=s.Read32(Slot(firstIndex)+4u);
            const uint32_t secondIndex=s.Read32(0x8005D83Cu);
            const uint32_t second=s.Read32(Slot(secondIndex)+8u);
            const uint32_t function=s.Read32(Slot(functionIndex));
            RunCommand(s,function,first,second);
            s.Write32(0x8005D828u,s.Read32(Slot(s.Read32(0x8005D83Cu))));
            s.WriteGpuCommandSource({s.Read32(Slot(s.Read32(0x8005D83Cu))+4u),{}});
            s.Write32(0x8005D830u,s.Read32(Slot(s.Read32(0x8005D83Cu))+8u));
            s.Write32(0x8005D83Cu,(s.Read32(0x8005D83Cu)+1u)&63u);
            const uint32_t h=s.Read32(0x8005D838u), t=s.Read32(0x8005D83Cu);
            if (h==t || DmaBusy(s)) break;
        } while (true);
    }
    s.Call(0x800358C0u,{s.Read32(0x8005D844u)});
    const uint32_t h=s.Read32(0x8005D838u), t=s.Read32(0x8005D83Cu);
    if (h==t && !DmaBusy(s) && s.Read32(0x8005D73Cu)!=0u && s.Read32(0x8005D740u)!=0u) {
        s.Write32(0x8005D73Cu,0u);
        s.Call(s.Read32(0x8005D740u),{});
    }
    const uint32_t finalHead=s.Read32(0x8005D838u), finalTail=s.Read32(0x8005D83Cu);
    return S((finalHead-finalTail)&63u);
}

namespace {
int32_t EnqueueSource(Services& s,uint32_t function,ImageRef source,
                       uint32_t copyBytes,uint32_t argument) {
    ResetTimeout80047144(s);
    while (true) {
        const uint32_t head=s.Read32(0x8005D838u), tail=s.Read32(0x8005D83Cu);
        if (((head+1u)&63u)!=tail) break;
        if (CheckTimeout80047178(s)!=0) return -1;
        DrainQueue80046BC4(s);
        s.AwaitDeviceProgress(0x1F8010A8u);
    }
    const uint32_t mask=U(s.Call(0x800358C0u,{0u}));
    s.Write32(0x8005D73Cu,1u);
    const uint8_t enabled=s.Read8(0x8005D735u);
    s.Write32(0x8005D840u,mask);
    bool direct=enabled==0u;
    if (!direct) {
        const uint32_t head=s.Read32(0x8005D838u), tail=s.Read32(0x8005D83Cu);
        direct=head==tail && !DmaBusy(s) && s.Read32(0x8005D740u)==0u;
    }
    if (direct) {
        WaitCommandReady(s);
        RunImageCommand(s,function,source,argument);
        const uint32_t restore=s.Read32(0x8005D840u);
        s.Write32(0x8005D828u,function);
        s.WriteGpuCommandSource(source.Source());
        s.Write32(0x8005D830u,argument);
        s.Call(0x800358C0u,{restore});
        return 0;
    }
    s.Call(0x800357A4u,{2u,0x80046BC4u});
    if (copyBytes!=0u) {
        const int32_t words=S(copyBytes)/4; // MIPS truncates signed division toward zero.
        for (int32_t i=0;i<words;++i) {
            const uint32_t word=source.Word(4u*U(i));
            s.Write32(Slot(s.Read32(0x8005D838u))+12u+4u*U(i),word);
        }
        const uint32_t target=s.Read32(0x8005D838u), data=s.Read32(0x8005D838u);
        s.Write32(Slot(target)+4u,Slot(data)+12u);
    } else {
        if (source.local) throw std::logic_error("Private RECT requires the original eight-byte queue copy");
        s.Write32(Slot(s.Read32(0x8005D838u))+4u,source.address);
    }
    s.Write32(Slot(s.Read32(0x8005D838u))+8u,argument);
    s.Write32(Slot(s.Read32(0x8005D838u)),function);
    const uint32_t next=(s.Read32(0x8005D838u)+1u)&63u;
    const uint32_t restore=s.Read32(0x8005D840u);
    s.Write32(0x8005D838u,next);
    s.Call(0x800358C0u,{restore});
    DrainQueue80046BC4(s);
    const uint32_t head=s.Read32(0x8005D838u), tail=s.Read32(0x8005D83Cu);
    return S((head-tail)&63u);
}

}
int32_t Enqueue800468E0(Services& s,uint32_t function,uint32_t source,
                       uint32_t copyBytes,uint32_t argument) {
    return EnqueueSource(s,function,{s,source},copyBytes,argument);
}

int32_t DrawOrderingTable800450A0(Services& s,uint32_t head) {
    if (s.Read8(0x8005D736u)>=2u) s.Call(s.Read32(0x8005D730u),{0x80012590u,head});
    const uint32_t driver=s.Read32(0x8005D72Cu);
    const uint32_t draw=s.Read32(driver+24u), enqueue=s.Read32(driver+8u);
    if (enqueue==0x800468E0u) return Enqueue800468E0(s,draw,head,0u,0u);
    return s.Call(enqueue,{draw,head,0u,0u});
}

int32_t SyncQueue80046FFC(Services& s,uint32_t mode) {
    if (mode!=0u) {
        const uint32_t head=s.Read32(0x8005D838u), tail=s.Read32(0x8005D83Cu);
        const uint32_t depth=(head-tail)&63u;
        if (depth!=0u) DrainQueue80046BC4(s);
        const bool busy=DmaBusy(s);
        if (busy || (s.Read32(s.Read32(0x8005D808u))&0x04000000u)==0u)
            return depth!=0u ? S(depth) : 1;
        return S(depth); // Snapshot BEFORE draining; do not recompute it.
    }
    ResetTimeout80047144(s);
    while (true) {
        const uint32_t head=s.Read32(0x8005D838u), tail=s.Read32(0x8005D83Cu);
        if (head==tail) break;
        DrainQueue80046BC4(s);
        if (CheckTimeout80047178(s)!=0) return -1;
        s.AwaitDeviceProgress(0x1F8010A8u);
    }
    // Original does not revisit the ring once it enters this hardware wait.
    while (DmaBusy(s) || (s.Read32(s.Read32(0x8005D808u))&0x04000000u)==0u) {
        if (CheckTimeout80047178(s)!=0) return -1;
        s.AwaitDeviceProgress(0x1F8010A8u);
    }
    return 0;
}

int32_t DrawSync80044B3C(Services& s,uint32_t mode) {
    if (s.Read8(0x8005D736u)>=2u) s.Call(s.Read32(0x8005D730u),{0x800124F8u,mode});
    const uint32_t driver=s.Read32(0x8005D72Cu);
    const uint32_t sync=s.Read32(driver+0x3Cu);
    if (sync==0x80046FFCu) return SyncQueue80046FFC(s,mode);
    return s.Call(sync,{mode});
}

int32_t StartOtcDma80045FC4(Services& s,uint32_t table,uint32_t count) {
    const uint32_t priority=s.Read32(0x8005D824u);
    s.Write32(priority,s.Read32(priority)|0x08000000u);
    s.Write32(s.Read32(0x8005D820u),0u);
    s.Write32(s.Read32(0x8005D818u),table+(count<<2u)-4u);
    s.Write32(s.Read32(0x8005D81Cu),count);
    s.Write32(s.Read32(0x8005D820u),0x11000002u);
    ResetTimeout80047144(s);
    while ((s.Read32(s.Read32(0x8005D820u))&0x01000000u)!=0u) {
        if (CheckTimeout80047178(s)!=0) return -1;
        s.AwaitDeviceProgress(0x1F8010E8u);
    }
    return S(count);
}

int32_t ClearOrderingTable80044FA8(Services& s,uint32_t table,uint32_t count) {
    if (s.Read8(0x8005D736u)>=2u) s.Call(s.Read32(0x8005D730u),{0x80012578u,table,count});
    const uint32_t driver=s.Read32(0x8005D72Cu);
    const uint32_t clear=s.Read32(driver+0x2Cu);
    if (clear==0x80045FC4u) StartOtcDma80045FC4(s,table,count);
    else s.Call(clear,{table,count});
    // 80045018..80045024 links the address of the shared four-NOP packet,
    // even after a DMA timeout.  8005D7F0 holds its tag, not a tail pointer.
    s.Write32(table,0x0005D7F0u);
    return S(table);
}

int32_t ReadGpuInfo8004688C(Services& s,uint32_t command) {
    s.Write32(s.Read32(0x8005D808u),command|0x10000000u);
    return S(s.Read32(s.Read32(0x8005D804u))&0x00FFFFFFu);
}

namespace {
int32_t Half(uint32_t value) { return S((value&0x7FFFu)-(value&0x8000u)); }
int32_t ClampGpuHalf(Services& s,uint32_t value,uint32_t limitAddress) {
    const int32_t input=Half(value);
    if (input<0) return 0;
    const int32_t limit=Half(s.Read16(limitAddress))-1;
    return input>limit ? limit : input;
}
bool WideGpuCoordinates(Services& s) {
    return uint32_t(s.Read8(0x8005D734u)-1u)<2u;
}
int32_t ControlThroughDriver(Services& s,uint32_t command) {
    const uint32_t driver=s.Read32(0x8005D72Cu);
    const uint32_t function=s.Read32(driver+0x10u);
    if (function==0x800467B4u) return WriteGpuControl800467B4(s,command);
    return s.Call(function,{command});
}
int32_t PackDrawArea(Services& s,uint32_t x,uint32_t y,uint32_t opcode) {
    const uint32_t clippedX=U(ClampGpuHalf(s,x,0x8005D738u));
    const uint32_t clippedY=U(ClampGpuHalf(s,y,0x8005D73Au));
    if (WideGpuCoordinates(s)) return S(opcode|(clippedX&0xFFFu)|((clippedY&0xFFFu)<<12u));
    return S(opcode|(clippedX&0x3FFu)|((clippedY&0x3FFu)<<10u));
}
}

int32_t CurrentDrawBuffer8004019C(Services& s) { return Half(s.Read16(0x80096590u)); }
void SetWorkBase80040F90(Services& s,uint32_t packet) { s.Write32(0x800901C8u,packet); }
void SetGeomScreen80040C94(Services& s,uint32_t screen) { s.MatrixGte().h=static_cast<uint16_t>(screen); }
void SetProjection80040C74(Services& s,uint32_t screen) { SetGeomScreen80040C94(s,screen); }
void SetGeomOffset800402C0(Services& s,uint32_t x,uint32_t y) {
    s.MatrixGte().ofx=S(x<<16u); s.MatrixGte().ofy=S(y<<16u);
}
int32_t VideoMode8003623C(Services& s) { return S(s.Read32(0x80057064u)); }
int32_t WriteGpuControl800467B4(Services& s,uint32_t command) {
    s.Write32(s.Read32(0x8005D808u),command);
    const uint32_t index=command>>24u;
    s.Write8(0x8008EB98u+index,static_cast<uint8_t>(command));
    return S(index);
}
int32_t FillBytes800473C0(Services& s,uint32_t destination,uint32_t value,uint32_t count) {
    while (count!=0u) { s.Write8(destination,static_cast<uint8_t>(value)); --count; ++destination; }
    return -1;
}
int32_t DisplayX80045EF0(Services& s,uint32_t environment) {
    const uint8_t version=s.Read8(0x8005D734u);
    if (version==1u) {
        if (s.Read8(0x8005D737u)!=0u) {
            const int32_t width=Half(s.Read16(environment+4u)), x=Half(s.Read16(environment));
            return 1024-width-x;
        }
    } else if (version==2u) {
        if (s.Read8(0x8005D737u)!=0u) {
            const int32_t width=Half(s.Read16(environment+4u)), x=Half(s.Read16(environment));
            return 1024-width/2-x; // Original signed divide truncates toward zero.
        }
        return Half(s.Read16(environment))/2;
    }
    return Half(s.Read16(environment));
}
int32_t SetDisplayMask80044AA0(Services& s,uint32_t enabled) {
    if (s.Read8(0x8005D736u)>=2u) s.Call(s.Read32(0x8005D730u),{0x800124E4u,enabled});
    if (enabled==0u) FillBytes800473C0(s,0x8005D7A0u,0xFFFFFFFFu,20u);
    return ControlThroughDriver(s,enabled==0u ? 0x03000001u : 0x03000000u);
}
int32_t PutDisplayEnvironment800452EC(Services& s,uint32_t environment) {
    uint32_t mode=0x08000000u;
    if (s.Read8(0x8005D736u)>=2u) s.Call(s.Read32(0x8005D730u),{0x800125D8u,environment});
    uint32_t start;
    if (WideGpuCoordinates(s)) {
        const uint32_t x=U(DisplayX80045EF0(s,environment));
        const uint32_t y=s.Read16(environment+2u);
        start=0x05000000u|(x&0xFFFu)|((y&0xFFFu)<<12u);
    } else {
        const uint32_t y=s.Read16(environment+2u), x=s.Read16(environment);
        start=0x05000000u|(x&0x3FFu)|((y&0x3FFu)<<10u);
    }
    ControlThroughDriver(s,start);
    // Compare each field freshly and stop at the first difference, as the
    // original does. GP1 callbacks can mutate both environment and cache.
    bool changed=false;
    for (uint32_t i=8u;i<16u;i+=2u) {
        const uint16_t cached=s.Read16(0x8005D7A0u+i), current=s.Read16(environment+i);
        if (cached!=current) { changed=true; break; }
    }
    if (changed) {
        const uint32_t video=U(VideoMode8003623C(s));
        const int32_t sx=Half(s.Read16(environment+8u));
        s.Write8(environment+18u,static_cast<uint8_t>(video));
        int32_t left=10*sx+608;
        const uint8_t pal=s.Read8(environment+18u);
        const int32_t sy=Half(s.Read16(environment+10u));
        int32_t top=sy+(pal!=0u ? 19 : 16);
        const int32_t sw=Half(s.Read16(environment+12u));
        int32_t right=left+(sw!=0 ? 10*sw : 2560);
        const int32_t sh=Half(s.Read16(environment+14u));
        int32_t bottom=top+(sh!=0 ? sh : 240);
        if (left<500) left=500; else if (left>=3291) left=3290;
        if (right<left+80) right=left+80; else if (right>=3291) right=3290;
        if (top<16) top=16;
        else {
            const int32_t maximum=s.Read8(environment+18u)!=0u ? 310 : 256;
            if (top>maximum) top=s.Read8(environment+18u)!=0u ? 310 : 256;
        }
        if (bottom<top+2) bottom=top+2;
        else {
            const int32_t maximum=s.Read8(environment+18u)!=0u ? 312 : 258;
            if (bottom>maximum) bottom=s.Read8(environment+18u)!=0u ? 312 : 258;
        }
        ControlThroughDriver(s,0x06000000u|(U(left)&0xFFFu)|((U(right)&0xFFFu)<<12u));
        ControlThroughDriver(s,0x07000000u|(U(top)&0x3FFu)|((U(bottom)&0x3FFu)<<10u));
    }
    const uint32_t cachedFlags=s.Read32(0x8005D7B0u), flags=s.Read32(environment+16u);
    changed=cachedFlags!=flags;
    if (!changed) for (uint32_t i=0;i<8u;i+=2u) {
        const uint16_t cached=s.Read16(0x8005D7A0u+i), current=s.Read16(environment+i);
        if (cached!=current) { changed=true; break; }
    }
    if (changed) {
        s.Write8(environment+18u,static_cast<uint8_t>(VideoMode8003623C(s)));
        if (s.Read8(environment+18u)==1u) mode|=8u;
        if (s.Read8(environment+17u)!=0u) mode|=0x10u;
        if (s.Read8(environment+16u)!=0u) mode|=0x20u;
        if (s.Read8(0x8005D737u)!=0u) mode|=0x80u;
        const int32_t width=Half(s.Read16(environment+4u));
        if (width>=561) mode|=3u;
        else if (width>=401) mode|=2u;
        else if (width>=353) mode|=0x40u;
        else if (width>=281) mode|=1u;
        const uint8_t pal=s.Read8(environment+18u);
        const int32_t height=Half(s.Read16(environment+6u));
        if (height>=(pal!=0u ? 289 : 257)) mode|=0x24u;
        ControlThroughDriver(s,mode);
    }
    // BIOS A0/2A is an explicit device dependency, not a success receipt.
    s.Call(0x80047FBCu,{0x8005D7A0u,environment,20u});
    return S(environment);
}
int32_t PackDrawAreaStart80045C8C(Services& s,uint32_t x,uint32_t y) {
    return PackDrawArea(s,x,y,0xE3000000u);
}
int32_t PackDrawAreaEnd80045D58(Services& s,uint32_t x,uint32_t y) {
    return PackDrawArea(s,x,y,0xE4000000u);
}
int32_t PackDrawOffset80045E24(Services& s,uint32_t x,uint32_t y) {
    if (WideGpuCoordinates(s)) return S(0xE5000000u|(x&0xFFFu)|((y&0xFFFu)<<12u));
    return S(0xE5000000u|(x&0x7FFu)|((y&0x7FFu)<<11u));
}
int32_t PackDrawMode80045C30(Services& s,uint32_t drawToDisplay,uint32_t dither,uint32_t tpage) {
    if (WideGpuCoordinates(s))
        return S(0xE1000000u|(tpage&0x27FFu)|(dither!=0u?0x800u:0u)|(drawToDisplay!=0u?0x1000u:0u));
    return S(0xE1000000u|(tpage&0x9FFu)|(dither!=0u?0x200u:0u)|(drawToDisplay!=0u?0x400u:0u));
}
int32_t PackTextureWindow80045E6C(Services& s,uint32_t rectangle) {
    if (rectangle==0u) return 0;
    const uint32_t x=s.Read8(rectangle)>>3u;
    const uint32_t w=(0u-U(Half(s.Read16(rectangle+4u))))&0xFFu;
    const uint32_t y=s.Read8(rectangle+2u)>>3u;
    const uint32_t h=(0u-U(Half(s.Read16(rectangle+6u))))&0xFFu;
    return S(0xE2000000u|(x<<10u)|(y<<15u)|((h>>3u)<<5u)|(w>>3u));
}
int32_t BuildDrawEnvironment8004598C(Services& s,uint32_t packet,uint32_t environment) {
    uint32_t x=s.Read16(environment), y=s.Read16(environment+2u);
    s.Write32(packet+4u,U(PackDrawAreaStart80045C8C(s,x,y)));
    uint32_t w=s.Read16(environment+4u); x=s.Read16(environment); y=s.Read16(environment+2u);
    x=w+x-1u; const uint32_t h=s.Read16(environment+6u); y=y+h-1u;
    s.Write32(packet+8u,U(PackDrawAreaEnd80045D58(s,x,y)));
    x=s.Read16(environment+8u); y=s.Read16(environment+10u);
    s.Write32(packet+12u,U(PackDrawOffset80045E24(s,x,y)));
    const uint32_t dfe=s.Read8(environment+23u), dtd=s.Read8(environment+22u), page=s.Read16(environment+20u);
    s.Write32(packet+16u,U(PackDrawMode80045C30(s,dfe,dtd,page)));
    s.Write32(packet+20u,U(PackTextureWindow80045E6C(s,environment+12u)));
    s.Write32(packet+24u,0xE6000000u);
    uint32_t count=6u;
    if (s.Read8(environment+24u)!=0u) {
        uint32_t rx=s.Read16(environment), ry=s.Read16(environment+2u);
        const uint32_t rw=s.Read16(environment+4u), rh=s.Read16(environment+6u);
        const uint32_t cw=U(ClampGpuHalf(s,rw,0x8005D738u))&0xFFFFu;
        const uint32_t ch=U(ClampGpuHalf(s,rh,0x8005D73Au))&0xFFFFu;
        const bool rectangle=(rx&63u)!=0u || (cw&63u)!=0u;
        if (rectangle) {
            rx=(rx-s.Read16(environment+8u))&0xFFFFu;
            ry=(ry-s.Read16(environment+10u))&0xFFFFu;
        }
        const uint32_t blue=s.Read8(environment+27u), green=s.Read8(environment+26u), red=s.Read8(environment+25u);
        s.Write32(packet+28u,(rectangle ? 0x60000000u : 0x02000000u)|(blue<<16u)|(green<<8u)|red);
        s.Write32(packet+32u,rx|(ry<<16u));
        s.Write32(packet+36u,cw|(ch<<16u));
        if (rectangle) {
            // Original restores private RECT coordinates even though unused.
            // Keep its two late reads (which can be observable for MMIO).
            s.Read16(environment+8u); s.Read16(environment+10u);
        }
        count=9u;
    }
    s.Write8(packet+3u,static_cast<uint8_t>(count));
    return S(count);
}
int32_t PutDrawEnvironment80045114(Services& s,uint32_t environment) {
    if (s.Read8(0x8005D736u)>=2u) s.Call(s.Read32(0x8005D730u),{0x800125A4u,environment});
    const uint32_t packet=environment+28u;
    BuildDrawEnvironment8004598C(s,packet,environment);
    const uint32_t tag=s.Read32(packet), driver=s.Read32(0x8005D72Cu);
    s.Write32(packet,tag|0x00FFFFFFu);
    const uint32_t draw=s.Read32(driver+24u), enqueue=s.Read32(driver+8u);
    if (enqueue==0x800468E0u) Enqueue800468E0(s,draw,packet,64u,0u);
    else s.Call(enqueue,{draw,packet,64u,0u});
    s.Call(0x80047FBCu,{0x8005D744u,environment,92u});
    return S(environment);
}
int32_t ApplyDrawClip800402E0(Services& s) {
    const uint32_t index=U(Half(s.Read16(0x80096590u)))<<1u;
    const uint16_t width=s.Read16(0x800928D4u);
    const uint32_t baseX=U(Half(s.Read16(0x800928D0u)));
    const uint32_t x=U(Half(s.Read16(0x8008ECA8u+index))), y=U(Half(s.Read16(0x8008ECACu+index)));
    const uint16_t height=s.Read16(0x800928D6u);
    s.Write16(0x80091734u,width); s.Write16(0x80091736u,height);
    const uint32_t baseY=U(Half(s.Read16(0x800928D2u)));
    s.Write16(0x80091730u,static_cast<uint16_t>(baseX+x));
    s.Write16(0x80091732u,static_cast<uint16_t>(baseY+y));
    return PutDrawEnvironment80045114(s,0x80091730u);
}
int32_t ApplyFrameOffset800401AC(Services& s) {
    if (s.Read16(0x800965A0u)!=0u) {
        const uint32_t index=U(Half(s.Read16(0x80096590u)))<<1u;
        const uint16_t baseX=s.Read16(0x800901C4u), x=s.Read16(0x8008ECA8u+index);
        s.Write16(0x800917ACu,0u); s.Write16(0x800917AAu,0u);
        s.Write16(0x80091738u,static_cast<uint16_t>(baseX+x));
        const uint16_t baseY=s.Read16(0x800901C6u), y=s.Read16(0x8008ECACu+index);
        s.Write16(0x8009173Au,static_cast<uint16_t>(baseY+y));
        return PutDrawEnvironment80045114(s,0x80091730u);
    }
    const bool nonzero=s.Read16(0x80096590u)!=0u;
    const uint32_t baseX=U(Half(s.Read16(0x800901C4u)));
    const uint32_t x=baseX+U(Half(s.Read16(nonzero ? 0x8008ECA8u : 0x8008ECAAu)));
    const uint32_t baseY=U(Half(s.Read16(0x800901C6u)));
    const uint32_t y=baseY+U(Half(s.Read16(nonzero ? 0x8008ECACu : 0x8008ECAEu)));
    SetGeomOffset800402C0(s,x,y);
    s.Write16(0x800917AAu,static_cast<uint16_t>(x)); s.Write16(0x800917ACu,static_cast<uint16_t>(y));
    return S(baseY); // SetGeomOffset leaves V0 from the prior base-Y load.
}
int32_t SwapBuffers80040370(Services& s) {
    const uint32_t index=U(Half(s.Read16(0x80096590u)))<<1u;
    s.Write16(0x80091790u,s.Read16(0x8008ECA8u+index));
    s.Write16(0x80091792u,s.Read16(0x8008ECACu+index));
    PutDisplayEnvironment800452EC(s,0x80091790u);
    SetDisplayMask80044AA0(s,1u);
    const uint32_t counter=s.Read32(0x8009658Cu)+1u;
    s.Write32(0x8009658Cu,counter);
    const uint16_t buffer=s.Read16(0x80096590u);
    s.Write32(0x8009658Cu,counter);
    s.Write16(0x80096590u,buffer==0u ? 1u : 0u);
    ApplyDrawClip800402E0(s);
    return ApplyFrameOffset800401AC(s);
}

namespace {
int32_t CheckImageRect(Services& s,uint32_t operation,ImageRef rect) {
    const uint8_t level=s.Read8(0x8005D736u);
    if (level!=1u && level!=2u) return 2;
    const auto half=[](uint16_t v) { return v<0x8000u ? int32_t(v) : int32_t(v)-65536; };
    if (level==1u) {
        // Keep the original short-circuit read order, including its y/height
        // check. This is the original advisory path, not a host reject gate.
        const int32_t width=half(rect.Half(4u));
        const int32_t maxWidth=half(s.Read16(0x8005D738u));
        if (width<=maxWidth) {
            const int32_t x=half(rect.Half(0u));
            if (width+x<=maxWidth) {
                const int32_t y=half(rect.Half(2u));
                const int32_t maxHeight=half(s.Read16(0x8005D73Au));
                if (y<=maxHeight) {
                    const int32_t height=half(rect.Half(6u));
                    if (y+height<=maxHeight && width>0 && x>=0 && y>=0 && height>0)
                        return 0;
                }
            }
        }
    }
    s.Call(s.Read32(0x8005D730u),{level==1u ? 0x8001250Cu : 0x8001252Cu,operation});
    // The first callback can change both the RECT and callback pointer.
    const uint32_t x=U(half(rect.Half(0u))), y=U(half(rect.Half(2u)));
    const uint32_t width=U(half(rect.Half(4u))), height=U(half(rect.Half(6u)));
    const uint32_t output=s.Read32(0x8005D730u);
    return s.Call(output,{0x80012518u,x,y,width,height});
}

}
int32_t CheckImageRect80044BA8(Services& s,uint32_t operation,uint32_t rect) {
    return CheckImageRect(s,operation,{s,rect});
}

namespace {
// 逐条保留 80044E2C 的读取/写入次序，允许诊断回调修改 RECT 和驱动。
int32_t MoveImage(Services& s,ImageRef rect,uint32_t x,uint32_t y) {
    CheckImageRect(s,0x80012554u,rect);
    if(rect.Half(4u)==0u || rect.Half(6u)==0u)return -1;
    const uint32_t destination=(y<<16u)|(x&0xFFFFu);
    const uint32_t source=rect.Word(0u),driver=s.Read32(0x8005D72Cu);
    s.Write32(0x8005D7E8u,destination);
    s.Write32(0x8005D7E4u,source);
    s.Write32(0x8005D7ECu,rect.Word(4u));
    // 包头和 GP0 0x80 命令来自原程序数据，不在这里重新填成常量。
    const uint32_t draw=s.Read32(driver+24u),enqueue=s.Read32(driver+8u);
    if(enqueue==0x800468E0u)return Enqueue800468E0(s,draw,0x8005D7DCu,20u,0u);
    return s.Call(enqueue,{draw,0x8005D7DCu,20u,0u});
}
}
// 公共 RECT 入口保持原始三参数 ABI。
int32_t MoveImage80044E2C(Services& s,uint32_t rect,uint32_t x,uint32_t y) {
    return MoveImage(s,{s,rect},x,y);
}
// 原版以零/非零判断 draw buffer，保留任意半字输入的分支语义。
int32_t MoveFramebuffer8001B120(Services& s,uint32_t argument) {
    const bool draw=CurrentDrawBuffer8004019C(s)!=0;
    const bool destination=argument!=0u?draw:!draw;
    ImageRect rect{0u,uint16_t(destination?0u:240u),320u,240u};
    return MoveImage(s,{s,0u,&rect},0u,destination?240u:0u);
}

namespace {
int32_t SubmitImage(Services& s,ImageRef rect,uint32_t red,
                         uint32_t green,uint32_t blue) {
    CheckImageRect(s,0x80012530u,rect);
    const uint32_t color=((blue&255u)<<16u)|((green&255u)<<8u)|(red&255u);
    // The advisory callback can replace the driver. Read its live entries only
    // after diagnostics, and retain the original asynchronous eight-byte copy.
    const uint32_t driver=s.Read32(0x8005D72Cu);
    const uint32_t clear=s.Read32(driver+12u), enqueue=s.Read32(driver+8u);
    if (enqueue==0x800468E0u) return EnqueueSource(s,clear,rect,8u,color);
    if (rect.local) return s.CallWithPrivateImage(enqueue,*rect.local,rect.Source(),{clear},{8u,color});
    return s.Call(enqueue,{clear,rect.address,8u,color});
}

}
int32_t ClearImage80044CD0(Services& s,uint32_t rect,uint32_t red,uint32_t green,uint32_t blue) {
    return SubmitImage(s,{s,rect},red,green,blue);
}
int32_t ClearLocalImage80044CD0(Services& s,ImageRect rect,uint32_t red,uint32_t green,uint32_t blue) {
    // Only the identity survives a direct call; queued data is copied to the
    // original queue slot before return. No pointer to these bytes escapes.
    if (!s.SupportsPrivateGpuSources()) throw std::logic_error("Private GPU source transport is not bound");
    const GpuCommandSource identity{0u,std::make_shared<const uint8_t>(0u)};
    return SubmitImage(s,{s,0u,&rect,identity},red,green,blue);
}
int32_t ClearFramebuffer8001B1B0(Services& s,uint32_t red,uint32_t green,uint32_t blue) {
    ImageRect rect;
    rect.width=320u; rect.height=480u; rect.y=0u; rect.x=0u;
    return ClearLocalImage80044CD0(s,rect,red&255u,green&255u,blue&255u);
}

namespace {
int32_t ClearImageBody(Services& s,ImageRef rect,uint32_t color) {
    const auto half=[](uint16_t v) { return v<0x8000u ? int32_t(v) : int32_t(v)-65536; };
    int32_t width=half(rect.Half(4u));
    if (width<0) width=0;
    else {
        const int32_t maximum=half(s.Read16(0x8005D738u))-1;
        if (maximum<width) width=maximum;
    }
    int32_t height=half(rect.Half(6u));
    rect.Half(4u,static_cast<uint16_t>(width));
    if (height<0) height=0;
    else {
        const int32_t maximum=half(s.Read16(0x8005D73Au))-1;
        if (maximum<height) height=maximum;
    }
    const uint16_t x=rect.Half(0u);
    rect.Half(6u,static_cast<uint16_t>(height));
    if ((x&63u)!=0u || (rect.Half(4u)&63u)!=0u) {
        s.Write32(0x8008EB58u,0x0708EB78u);
        s.Write32(0x8008EB68u,0xE6000000u);
        s.Write32(0x8008EB5Cu,0xE3000000u);
        s.Write32(0x8008EB60u,0xE4FFFFFFu);
        s.Write32(0x8008EB64u,0xE5000000u);
        s.Write32(0x8008EB6Cu,(color&0x00FFFFFFu)|0x60000000u);
        s.Write32(0x8008EB70u,rect.Word(0u));
        const uint32_t size=rect.Word(4u);
        s.Write32(0x8008EB78u,0x03FFFFFFu);
        s.Write32(0x8008EB74u,size);
        s.Write32(0x8008EB7Cu,U(ReadGpuInfo8004688C(s,3u))|0xE3000000u);
        s.Write32(0x8008EB80u,U(ReadGpuInfo8004688C(s,4u))|0xE4000000u);
        s.Write32(0x8008EB84u,U(ReadGpuInfo8004688C(s,5u))|0xE5000000u);
    } else {
        s.Write32(0x8008EB58u,0x04FFFFFFu);
        s.Write32(0x8008EB5Cu,0xE6000000u);
        s.Write32(0x8008EB60u,(color&0x00FFFFFFu)|0x02000000u);
        s.Write32(0x8008EB64u,rect.Word(0u));
        s.Write32(0x8008EB68u,rect.Word(4u));
    }
    StartLinkedDma80046840(s,0x8008EB58u);
    return 0;
}
}
int32_t ClearImage800460AC(Services& s,uint32_t rect,uint32_t color) {
    return ClearImageBody(s,{s,rect},color);
}
}
