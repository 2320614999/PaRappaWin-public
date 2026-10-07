// External-only routing and combined GPU/IRQ integration. No game/save backend.
#define PR_STAGE2_BOOTSTRAP_GPU_FIXTURE_ONLY
#include "test_stage2_graph_bootstrap_gpu.cpp"
#undef PR_STAGE2_BOOTSTRAP_GPU_FIXTURE_ONLY
#include "pr/pr_stage2_native_dispatch.h"
#include "pr/pr_stage2_irq_direct.h"
#include "pr/pr_stage2_tim_direct.h"
#include <sstream>
#include <type_traits>
#include <utility>

namespace {
namespace Dispatch = PrStage2NativeDispatch;
namespace G = PrStage2GpuDirect;
namespace I = PrStage2GraphInitDirect;
namespace B = PrStage2GraphBootstrapDirect;
namespace T = PrStage2TimDirect;
namespace Q = PrStage2IrqDirect;
using Base = PrStage2LifecycleDirect::Services;
using Args = std::initializer_list<uint32_t>;
struct PrefixEnd {};
// Deterministic synthetic transport compares a routed call with a direct C++
// call. Stop at 128 effects or a lower external call; do not claim full original
// function verification from these deliberately bounded traces.
struct Probe : Dispatch::Services {
    std::map<uint32_t,uint8_t> memory;
    std::vector<std::vector<uint32_t>> events;
    void Event(uint32_t kind, uint32_t address, uint32_t value=0) {
        events.push_back({kind,address,value});
        if(events.size()>=128u) throw PrefixEnd{};
    }
    uint32_t Read(uint32_t a,uint32_t bytes) {
        Event(bytes,a);uint32_t v=0;
        for(uint32_t i=0;i<bytes;++i) {
            const auto found=memory.find(a+i);
            const auto b=found==memory.end()?uint8_t((a+i)*37u+13u):found->second;
            v|=uint32_t(b)<<(8u*i);
        }
        return v;
    }
    void Write(uint32_t a,uint32_t bytes,uint32_t v) {
        Event(16u+bytes,a,v);
        for(uint32_t i=0;i<bytes;++i)memory[a+i]=uint8_t(v>>(8u*i));
    }
    uint8_t ReadImageMemory8(uint32_t a) override {return uint8_t(Read(a,1));}
    uint16_t ReadImageMemory16(uint32_t a) override {return uint16_t(Read(a,2));}
    uint32_t ReadImageMemory32(uint32_t a) override {return Read(a,4);}
    void WriteImageMemory8(uint32_t a,uint8_t v) override {Write(a,1,v);}
    void WriteImageMemory16(uint32_t a,uint16_t v) override {Write(a,2,v);}
    void WriteImageMemory32(uint32_t a,uint32_t v) override {Write(a,4,v);}
    int32_t CallExternal(uint32_t f,Args a) override {
        events.push_back({64u,f});events.back().insert(events.back().end(),a.begin(),a.end());throw PrefixEnd{};
    }
    void CallExternalVoid(uint32_t f,Args a) override {(void)CallExternal(f,a);}
    PrStage2LifecycleDirect::Words64 Call64(uint32_t f,Args a) override {(void)CallExternal(f,a);throw PrefixEnd{};}
    PrStage2LifecycleDirect::Vector32 NormalizeVector8003A3DC(PrStage2LifecycleDirect::Vector32 v) override {
        (void)CallExternal(0x8003A3DCu,{v.x,v.y,v.z});throw PrefixEnd{};
    }
    int32_t SetCdLocation800367A4(uint32_t v) override {return CallExternal(0x800367A4u,{v});}
    int32_t LoadImage80044D64(PrStage2LifecycleDirect::ImageRect r,uint32_t source) override {
        return CallExternal(0x80044D64u,{r.x,r.y,r.width,r.height,source});
    }
    PrPsxGteDirect::MatrixRegisters& MatrixGte() override {Event(65,0);throw PrefixEnd{};}
    [[noreturn]] void Exit(uint32_t f,Args a) override {(void)CallExternal(f,a);throw PrefixEnd{};}
    [[noreturn]] void Break(uint32_t pc,uint32_t code) override {Event(66,pc,code);throw PrefixEnd{};}
};
struct ExternalProbe : Probe {
    uint32_t calls=0,voidCalls=0;
    bool reenter=false,fail=false;
    int32_t CallExternal(uint32_t f,Args a) override {
        ++calls;events.push_back({64u,f});events.back().insert(events.back().end(),a.begin(),a.end());
        if(fail)throw std::runtime_error("external failure preserved");
        if(reenter)return Call(0x80043EBCu,{0xFFFFu,0xFFFFFFFFu});
        return -73;
    }
    void CallExternalVoid(uint32_t f,Args a) override {++voidCalls;(void)CallExternal(f,a);}
};
template<class F> void WithArgs(size_t n,const std::array<uint32_t,7>& a,F f) {
    switch(n) {
    case 0:f({});break;case 1:f({a[0]});break;case 2:f({a[0],a[1]});break;
    case 3:f({a[0],a[1],a[2]});break;case 4:f({a[0],a[1],a[2],a[3]});break;
    case 5:f({a[0],a[1],a[2],a[3],a[4]});break;
    case 6:f({a[0],a[1],a[2],a[3],a[4],a[5]});break;
    default:throw std::logic_error("test argument count");
    }
}
struct Outcome {int32_t result=0;std::string stopped;};
template<class F> Outcome Capture(F f) {
    Outcome o;
    try{o.result=f();}catch(const PrefixEnd&){o.stopped="prefix";}
    catch(const std::exception& e){o.stopped=e.what();}
    return o;
}
template<class R,class... A,size_t... N>
int32_t Direct(Probe& p,R(*f)(Base&,A...),const std::array<uint32_t,7>& a,std::index_sequence<N...>) {
    if constexpr(std::is_void_v<R>){f(p,a[N]...);return 0;}
    else return f(p,a[N]...);
}
uint32_t routes=0,comparisons=0,arityRejects=0,voidRejects=0;
template<class R,class... A> void Route(uint32_t address,R(*fn)(Base&,A...)) {
    constexpr size_t count=sizeof...(A);
    const std::array<uint32_t,7> words{0x800B7000u,0x800B7020u,3u,1u,0u,7u,9u};
    for(size_t n=0;n<=6;++n)if(n!=count) {
        Probe p;bool rejected=false;
        WithArgs(n,words,[&](Args a){try {
            if constexpr(std::is_void_v<R>)p.CallVoid(address,a);else (void)p.Call(address,a);
        }catch(const std::invalid_argument&){rejected=true;}});
        Check(rejected&&p.events.empty(),"known route arity rejection had effects or delegated");++arityRejects;
    }
    if constexpr(std::is_void_v<R>) {
        Probe p;bool rejected=false;
        WithArgs(count,words,[&](Args a){try{(void)p.Call(address,a);}catch(const std::invalid_argument&){rejected=true;}});
        Check(rejected&&p.events.empty(),"void call manufactured a scalar result");++voidRejects;
    }
    for(uint32_t seed:{0u,1u,0x7FFFFFFFu,0xFFFFFFFFu}) {
        auto args=words;for(size_t i=0;i<count;++i)args[i]^=seed;
        Probe routed,direct;Outcome actual;
        WithArgs(count,args,[&](Args a){actual=Capture([&]()->int32_t {
            if constexpr(std::is_void_v<R>){routed.CallVoid(address,a);return 0;}
            else return routed.Call(address,a);
        });});
        const auto expected=Capture([&]{return Direct(direct,fn,args,std::index_sequence_for<A...>{});});
        if(actual.result!=expected.result||actual.stopped!=expected.stopped||routed.events!=direct.events||routed.memory!=direct.memory)
            throw std::runtime_error("route/direct prefix mismatch "+std::to_string(address));
        ++comparisons;
    }
    ++routes;
}
int32_t DirectClut(Base&,uint32_t x,uint32_t y){return T::GetClut80043EBC(x,y);}
void CheckRoutes() {
    Route(0x80047144u,G::ResetTimeout80047144);
    Route(0x80047178u,G::CheckTimeout80047178);
    Route(0x80046840u,G::StartLinkedDma80046840);
    Route(0x80046BC4u,G::DrainQueue80046BC4);
    Route(0x800468E0u,G::Enqueue800468E0);
    Route(0x800450A0u,G::DrawOrderingTable800450A0);
    Route(0x80044B3Cu,G::DrawSync80044B3C);
    Route(0x80046FFCu,G::SyncQueue80046FFC);
    Route(0x80044FA8u,G::ClearOrderingTable80044FA8);
    Route(0x80045FC4u,G::StartOtcDma80045FC4);
    Route(0x80044BA8u,G::CheckImageRect80044BA8);
    Route(0x80044CD0u,G::ClearImage80044CD0);
    Route(0x80044E2Cu,G::MoveImage80044E2C);
    Route(0x8001B120u,G::MoveFramebuffer8001B120);
    Route(0x800460ACu,G::ClearImage800460AC);
    Route(0x8001B1B0u,G::ClearFramebuffer8001B1B0);
    Route(0x8004688Cu,G::ReadGpuInfo8004688C);
    Route(0x8004019Cu,G::CurrentDrawBuffer8004019C);
    Route(0x80040F90u,G::SetWorkBase80040F90);
    Route(0x80040C94u,G::SetGeomScreen80040C94);
    Route(0x80040C74u,G::SetProjection80040C74);
    Route(0x800402C0u,G::SetGeomOffset800402C0);
    Route(0x80040D20u,I::InitGte80040D20);
    Route(0x8003623Cu,G::VideoMode8003623C);
    Route(0x800467B4u,G::WriteGpuControl800467B4);
    Route(0x800473C0u,G::FillBytes800473C0);
    Route(0x80045EF0u,G::DisplayX80045EF0);
    Route(0x80044AA0u,G::SetDisplayMask80044AA0);
    Route(0x800452ECu,G::PutDisplayEnvironment800452EC);
    Route(0x80045C8Cu,G::PackDrawAreaStart80045C8C);
    Route(0x80045D58u,G::PackDrawAreaEnd80045D58);
    Route(0x80045E24u,G::PackDrawOffset80045E24);
    Route(0x80045C30u,G::PackDrawMode80045C30);
    Route(0x80045E6Cu,G::PackTextureWindow80045E6C);
    Route(0x8004598Cu,G::BuildDrawEnvironment8004598C);
    Route(0x80045114u,G::PutDrawEnvironment80045114);
    Route(0x800402E0u,G::ApplyDrawClip800402E0);
    Route(0x800401ACu,G::ApplyFrameOffset800401AC);
    Route(0x80040370u,G::SwapBuffers80040370);
    Route(0x800472E4u,I::DetectGpu800472E4);
    Route(0x80046EC0u,I::ResetGpu80046EC0);
    Route(0x800446A0u,I::ResetGraph800446A0);
    Route(0x8003FC14u,I::InitGraphEnvironment8003FC14);
    Route(0x800442F4u,I::InitClearPacket800442F4);
    Route(0x8003FDE4u,I::InitGeometry8003FDE4);
    Route(0x8003FB9Cu,I::InitGraph8003FB9C);
    Route(0x80040AE4u,I::SetDoubleBufferOffsets80040AE4);
    Route(0x80040B84u,I::SetScreenCenter80040B84);
    Route(0x8001C1E8u,B::InitPrimitiveDispatch8001C1E8);
    Route(0x800354C0u,B::InitPad800354C0);
    Route(0x8001C470u,B::InitGraphics8001C470);
    Route(0x80040EACu,T::GetTimInfo80040EAC);
    Route(0x8001AE7Cu,T::UploadTim8001AE7C);
    Route(0x80043EBCu,DirectClut);
    Route(0x800431E0u,T::UploadClut800431E0);
    Route(0x8001ADECu,T::UploadRuntimeTim8001ADEC);
    Route(0x800358C0u,Q::SetInterruptMask800358C0);
    Route(0x800361F8u,Q::ClearCallbackWords800361F8);
    Route(0x80035BA0u,Q::SetInterruptCallback80035BA0);
    Route(0x80035774u,Q::InterruptCallback80035774);
    Route(0x80036150u,Q::SetDmaCallback80036150);
    Route(0x800357A4u,Q::DmaCallback800357A4);
    Route(0x80035F7Cu,Q::InitDmaCallbacks80035F7C);
    Route(0x80035FCCu,Q::DispatchDmaInterrupt80035FCC);
    Check(routes==64u&&comparisons==256u&&arityRejects==384u&&voidRejects==5u,"route inventory incomplete");
    for(uint32_t address:{0u,0x800FFFFCu,0x8001C471u,0xA001C470u,0x80044D64u}) {
        ExternalProbe p;int32_t sentinel=123;
        Check(!Dispatch::TryCall(p,address,{},sentinel)&&sentinel==123&&p.events.empty(),"unknown route touched output/device");
        Check(!Dispatch::TryCallVoid(p,address,{})&&p.events.empty(),"unknown void route had effects");
        Check(p.Call(address,{1,2,3,4,5})==-73&&p.calls==1,"external return/arity lost");
        Check(p.events.back()==std::vector<uint32_t>({64,address,1,2,3,4,5}),"external arguments changed");
        p.CallVoid(address,{6});Check(p.calls==2&&p.voidCalls==1,"void external route lost");
        p.reenter=true;Check(p.Call(address,{})==T::GetClut80043EBC(0xFFFF,0xFFFFFFFF),"native reentry failed");
        p.fail=true;bool caught=false;try{(void)p.Call(address,{});}catch(const std::runtime_error& e){caught=std::string(e.what())=="external failure preserved";}
        Check(caught,"external failure swallowed");
    }
    ExternalProbe a,b;
    a.CallVoid(0x800473C0u,{0x800B0000u,0xA5u,3u});
    Check(a.Read8(0x800B0000u)==0xA5u&&a.calls==0u&&b.events.empty(),"scalar discard or service isolation failed");
    std::cout<<"native-dispatch-routes "<<routes<<' '<<comparisons<<' '<<arityRejects<<' '<<voidRejects<<" 5\n";
}

struct NativeGpu : Dispatch::Services {
    BootstrapGpuFixture host;
    uint16_t mask=0;
    uint32_t control=0,pendingChannels=0,delivered=0;
    bool delivering=false;
    std::vector<uint32_t> external;
    NativeGpu(const char* scus,const char* disc):host(scus,disc){}
    static uint32_t Canonical(uint32_t a) {
        if(a<0x00800000u||(a>=0x80000000u&&a<0x80800000u)||(a>=0xA0000000u&&a<0xA0800000u))
            return 0x80000000u|(a&0x1FFFFFu);
        return a;
    }
    uint8_t ReadImageMemory8(uint32_t a) override {return host.Read8(Canonical(a));}
    uint16_t ReadImageMemory16(uint32_t a) override {return a==0x1F801074u?mask:host.Read16(Canonical(a));}
    uint32_t ReadImageMemory32(uint32_t a) override {
        if(a==0x1F8010F4u)return control|(pendingChannels<<24u)|
            (((control&0x00800000u)&&((control>>16u)&pendingChannels))?0x80000000u:0u);
        if(a==0x1F8010A8u&&host.pending&&host.pumpOnStatus&&!host.pumping&&!delivering)Complete();
        const bool old=host.pumping;host.pumping=true;
        try {const auto v=host.Read32(Canonical(a));host.pumping=old;return v;}
        catch(...){host.pumping=old;throw;}
    }
    void WriteImageMemory8(uint32_t a,uint8_t v) override {host.Write8(Canonical(a),v);}
    void WriteImageMemory16(uint32_t a,uint16_t v) override {if(a==0x1F801074u)mask=v;else host.Write16(Canonical(a),v);}
    void WriteImageMemory32(uint32_t a,uint32_t v) override {
        if(a==0x1F8010F4u){pendingChannels&=~(v>>24u);control=v&0x00FFFFFFu;return;}
        host.Write32(Canonical(a),v);
    }
    int32_t CallExternal(uint32_t f,Args args) override {
        external.push_back(f);
        if(f==0x80047FBCu) {
            Check(args.size()==3u&&args.begin()[2]<=92u,"BIOS copy contract");
            const auto a=args.begin();std::vector<uint8_t> bytes(a[2]);
            for(uint32_t i=0;i<a[2];++i)bytes[i]=Read8(a[1]+i);
            for(uint32_t i=0;i<a[2];++i)Write8(a[0]+i,bytes[i]);
            return a[0]<=0x7FFFFFFFu?int32_t(a[0]):int32_t(int64_t(a[0])-0x100000000LL);
        }
        // Only these lower BIOS/GTE/clock/output inputs are supplied by the
        // old device fixture. None of the new native routes may delegate here.
        Check(f==0x80047FFCu||f==0x80035744u||f==0x80048950u||f==0x800489F0u||
              f==0x80048AE0u||f==0x80040D20u||f==0x80035560u,"translated call escaped native ownership");
        return host.Call(f,args);
    }
    void CallExternalVoid(uint32_t,Args) override {throw std::runtime_error("unexpected external void call");}
    PrStage2LifecycleDirect::Words64 Call64(uint32_t,Args) override {throw std::runtime_error("unexpected Call64");}
    PrStage2LifecycleDirect::Vector32 NormalizeVector8003A3DC(PrStage2LifecycleDirect::Vector32) override {throw std::runtime_error("unexpected normalize");}
    int32_t SetCdLocation800367A4(uint32_t v) override {return host.SetCdLocation800367A4(v);}
    int32_t LoadImage80044D64(PrStage2LifecycleDirect::ImageRect r,uint32_t a) override {return host.LoadImage80044D64(r,a);}
    PrPsxGteDirect::MatrixRegisters& MatrixGte() override {return host.gte;}
    [[noreturn]] void Exit(uint32_t,Args) override {throw std::runtime_error("unexpected Exit");}
    [[noreturn]] void Break(uint32_t,uint32_t) override {throw std::runtime_error("unexpected BREAK");}
    void InitIrq() {
        // Explicit wider-BIOS initialization precondition; not a fabricated
        // implementation of ResetCallback80035744 or a production event clock.
        Write16(0x80055F78u,1);Write16(0x80055FA8u,0);
        Write32(0x80057000u,0x80056FE0u);Write32(0x80056FE8u,0x80035BA0u);
        const auto target=uint32_t(Call(0x80035F7Cu,{}));Check(target==0x80036150u,"DMA initializer result");
        Write32(0x80056FE4u,target);
        Check(mask==8u&&Read32(0x80055F88u)==0x80035FCCu,"native IRQ vector missing");
    }
    bool Deliver() {
        if(delivering||!(mask&8u)||!(Read32(0x1F8010F4u)&0x80000000u))return false;
        delivering=true;
        try {Check(Call(Read32(0x80055F88u),{})==0,"registered native IRQ result");++delivered;}
        catch(...){delivering=false;throw;}
        delivering=false;return true;
    }
    void Complete() {
        Check(host.pending&&!host.pumping&&host.dmaCallback==0u,"legacy completion shortcut or invalid DMA");
        host.pumping=true;
        try {
            const auto head=host.io.at(0x1F8010A0u);
            const auto batch=Ot::DecodeLinkedList(*this,head,host.state);
            Ot::Render(batch,*host.projection,*host.renderer,host.view);
            host.state=batch.finalState;host.consumedHeads.push_back(head);++host.completions;
            host.pending=false;host.io[0x1F8010A8u]&=~0x01000000u;
            if(control&0x00040000u)pendingChannels|=4u;
            Deliver();
        }catch(...){host.pumping=false;throw;}
        host.pumping=false;
    }
};
void CheckGpu(NativeGpu& s,int scale,bool masked) {
    Window window;Check(window.hwnd!=nullptr,"native dispatcher window");
    D3D11Renderer renderer;Check(renderer.Initialize(window.hwnd,320*scale,480*scale),"native dispatcher renderer");
    PrStage2VramAtlas::Projection atlas(s.host.vram);renderer.BeginFrame(1,1,1);
    s.host.Bind(renderer,atlas,{},{0,0,320,480,0,0,float(320*scale),float(480*scale)});
    s.InitIrq();s.host.pumpOnStatus=true;s.Write32(0x80057064u,0);
    Check(s.Call(0x8001C470u,{})==0&&s.Call(0x80044B3Cu,{0})==0,"native bootstrap failed");
    Check(s.host.completions==7u&&s.host.resets==2u&&!s.host.pending,"bootstrap did not consume seven actual packets");
    Check(s.host.gte.h==440u&&s.Read32(0x800917FCu)==320u&&s.Read32(0x8009182Cu)==240u,"bootstrap geometry");
    Check(s.host.gte.zsf3==341&&s.host.gte.zsf4==256&&s.host.gte.dqa==-4194&&
          s.host.gte.dqb==20971520,"native bootstrap skipped depth initialization");
    for(auto pixel:ReadFrame(renderer))Near(pixel,0,0,0,"bootstrap clear missing");
    s.CallVoid(0x80040C94u,{600});Check(s.host.gte.h==600u,"native void GTE write missing");
    s.CallVoid(0x80040C94u,{440});
    s.host.pumpOnStatus=false;
    const uint32_t irqBefore=s.delivered;
    Check(s.Call(0x8001B1B0u,{200,0,0})==0,"direct private clear return");
    const auto source=s.ReadGpuCommandSource();Check(bool(source.local),"private identity missing");
    Check(s.Call(0x8001B1B0u,{0,180,0})==1&&s.Call(0x8001B1B0u,{0,0,160})==2,"private queue lengths");
    Check(s.ReadGpuCommandSource().local==source.local&&s.host.completions==7u&&s.host.dmaCallback==0u,"submission consumed or replaced identity early");
    if(masked)Check(s.Call(0x800358C0u,{0})==8,"mask old value");
    volatile uint8_t churn[1024];for(auto& x:churn)x=0xA5u;
    for(uint32_t i=0;i<3u;++i) {
        s.Complete();
        if(masked&&i==0u) {
            Check(s.delivered==irqBefore&&!s.host.pending&&s.pendingChannels==4u&&
                  s.ReadGpuCommandSource().local==source.local,"masked IRQ consumed the next private command");
            Check(!s.Deliver()&&s.Call(0x800358C0u,{8})==0&&s.Deliver(),"pending IRQ not delivered after unmask");
        }
        for(auto pixel:ReadFrame(renderer))Near(pixel,i==0u?200:0,i==1u?180:0,i==2u?160:0,"queued native IRQ pixels");
        Check(!s.ReadGpuCommandSource().local,"queue source did not replace local identity");
    }
    Check(s.host.completions==10u&&s.delivered-irqBefore==2u&&!s.host.pending&&s.pendingChannels==0u&&
          s.Read32(0x80057048u)==0u&&s.Read32(0x8005D838u)==s.Read32(0x8005D83Cu),"native drain final state");
    Check(s.mask==8u&&s.host.dmaCallback==0u,"legacy callback or changed mask");
    std::cout<<"native-dispatch-gpu "<<scale<<' '<<int(masked)<<" 10 2 "<<614400u*scale*scale<<'\n';
}
}
int main(int argc,char** argv) {
    try {
        CheckRoutes();
        if(argc==2&&std::string(argv[1])=="--routes-only")return 0;
        if(argc!=3)return 1;
        for(int scale:{1,4})for(bool masked:{false,true}) {
            NativeGpu s(argv[1],argv[2]);CheckGpu(s,scale,masked);
        }
        std::cout<<"native-dispatch-pass\n";return 0;
    }catch(const PrefixEnd&){std::cerr<<"known call escaped ABI validation into an external boundary\n";return 2;}
    catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 2;}
}
