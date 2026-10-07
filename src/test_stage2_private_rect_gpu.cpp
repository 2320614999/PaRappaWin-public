// External-only native private-RECT and actual GPU integration checks.
#define PR_STAGE2_BOOTSTRAP_GPU_FIXTURE_ONLY
#include "test_stage2_graph_bootstrap_gpu.cpp"
#undef PR_STAGE2_BOOTSTRAP_GPU_FIXTURE_ONLY
#include "pr/pr_stage2_image_source_direct.h"

namespace {
using Source=PrStage2LifecycleDirect::GpuCommandSource;
using Rect=PrStage2LifecycleDirect::ImageRect;
struct PrivateGpu : PrStage2GpuDirect::ImageSourceServices {
    BootstrapGpuFixture host;
    bool failNextSourceWrite=false;
    uint32_t privateEntries=0u;
    PrivateGpu(const char* scus,const char* disc):host(scus,disc) {}
    static uint32_t Canonical(uint32_t a) {
        if (a<0x00800000u || (a>=0x80000000u && a<0x80800000u) ||
            (a>=0xA0000000u && a<0xA0800000u)) return 0x80000000u|(a&0x1FFFFFu);
        return a;
    }
    uint8_t ReadImageMemory8(uint32_t a) override { return host.Read8(Canonical(a)); }
    uint16_t ReadImageMemory16(uint32_t a) override { return host.Read16(Canonical(a)); }
    uint32_t ReadImageMemory32(uint32_t a) override {
        if(a==0x1F8010A8u && host.pending && host.pumpOnStatus && !host.pumping) Pump();
        // The pending work must always be consumed with THIS Services object,
        // not the backing fixture, so pointer-field invalidation is preserved.
        const bool pumping=host.pumping; host.pumping=true;
        const auto value=host.Read32(Canonical(a));host.pumping=pumping;return value;
    }
    void WriteImageMemory8(uint32_t a,uint8_t v) override { host.Write8(Canonical(a),v); }
    void WriteImageMemory16(uint32_t a,uint16_t v) override { host.Write16(Canonical(a),v); }
    void WriteImageMemory32(uint32_t a,uint32_t v) override {
        if(failNextSourceWrite && Canonical(a)==0x8005D82Cu) {
            failNextSourceWrite=false;throw std::runtime_error("Explicit memory-device write failure");
        }
        host.Write32(Canonical(a),v);
    }
    int32_t Call(uint32_t f,std::initializer_list<uint32_t> args) override {
        const auto a=args.begin();
        if(f==0x8001B1B0u) {
            Check(args.size()==3u,"private bootstrap argument count");++privateEntries;
            return PrStage2GpuDirect::ClearFramebuffer8001B1B0(*this,a[0],a[1],a[2]);
        }
        if(f==0x80047FBCu) {
            Check(args.size()==3u && a[2]<=92u,"BIOS-copy fixture length");
            std::vector<uint8_t> copy(a[2]);
            for(uint32_t i=0;i<a[2];++i) copy[i]=Read8(a[1]+i);
            for(uint32_t i=0;i<a[2];++i) Write8(a[0]+i,copy[i]);
            return int32_t(int64_t(a[0])-0x100000000LL);
        }
        return host.Call(f,args);
    }
    PrStage2LifecycleDirect::Words64 Call64(uint32_t f,std::initializer_list<uint32_t> a) override {return host.Call64(f,a);}
    PrStage2LifecycleDirect::Vector32 NormalizeVector8003A3DC(PrStage2LifecycleDirect::Vector32 v) override {return host.NormalizeVector8003A3DC(v);}
    int32_t SetCdLocation800367A4(uint32_t v) override {return host.SetCdLocation800367A4(v);}
    int32_t LoadImage80044D64(Rect r,uint32_t a) override {return host.LoadImage80044D64(r,a);}
    PrPsxGteDirect::MatrixRegisters& MatrixGte() override {return host.gte;}
    [[noreturn]] void Exit(uint32_t,std::initializer_list<uint32_t>) override {throw std::runtime_error("Unexpected native Exit");}
    [[noreturn]] void Break(uint32_t,uint32_t) override {throw std::runtime_error("Unexpected native BREAK");}
    void Pump() {
        Check(host.pending && host.renderer && host.projection && !host.pumping,"Invalid private-device DMA pump");
        host.pumping=true;
        const auto head=host.io.at(0x1F8010A0u);
        const auto batch=Ot::DecodeLinkedList(*this,head,host.state);
        Ot::Render(batch,*host.projection,*host.renderer,host.view);
        host.state=batch.finalState;host.consumedHeads.push_back(head);++host.completions;
        host.pending=false;host.io[0x1F8010A8u]&=~0x01000000u;
        if(host.dmaCallback) {
            Check(host.dmaCallback==0x80046BC4u && host.interruptMask!=0u,"Unexpected private DMA callback");
            PrStage2GpuDirect::DrainQueue80046BC4(*this);
        }
        host.pumping=false;
    }
};

template<class F> void Reject(F operation,const char* why) {
    bool failed=false;try {operation();}catch(const std::exception&) {failed=true;}
    Check(failed,why);
}

void CheckSourceTransport(PrivateGpu& s) {
    uint32_t trials=0u;
    for(uint32_t segment:{0u,0x80000000u,0xA0000000u}) for(uint32_t mirror:{0u,0x200000u,0x400000u,0x600000u}) {
        const uint32_t a=segment+mirror+0x5D82Cu;
        const Source native{0u,std::make_shared<const uint8_t>(0u)};
        s.Write32(0x8005D82Cu,0x12345678u);
        s.WriteGpuCommandSource(native);
        Check(s.ReadGpuCommandSource().local==native.local,"Typed identity lost");
        Check(s.host.Read32(0x8005D82Cu)==0x12345678u,"Host pointer or fake address leaked into RAM");
        Reject([&]{s.Read8(a);},"Raw native-source byte read accepted");
        Reject([&]{s.Read16(a+1u);},"Raw native-source halfword read accepted");
        Reject([&]{s.Read32(a);},"Raw native-source word read accepted");
        Reject([&]{s.Read32(a-1u);},"Overlapping native-source read accepted");
        Reject([&]{s.Write8(a,0);},"Partial native-source byte write accepted");
        Reject([&]{s.Write16(a+2u,0);},"Partial native-source halfword write accepted");
        Reject([&]{s.Write32(a-1u,0);},"Overlapping native-source write accepted");
        s.failNextSourceWrite=true;
        Reject([&]{s.Write32(a,0);},"Injected memory failure disappeared");
        Check(s.ReadGpuCommandSource().local==native.local,"Failed write discarded live source");
        s.Write32(a,0x800A1234u);
        Check(!s.ReadGpuCommandSource().local && s.ReadGpuCommandSource().address==0x800A1234u,
              "Successful mirrored full write failed to invalidate native source");
        ++trials;
    }
    const auto before=s.host.ram;
    Reject([&]{PrStage2GpuDirect::ClearFramebuffer8001B1B0(s.host,0,0,0);},"Unbound private transport accepted");
    Check(before==s.host.ram && !s.host.pending,"Unbound transport caused effects before rejection");
    std::cout<<"private-source-transport "<<trials<<" 1\n";
}

void CheckClears(PrivateGpu& s,int scale) {
    Window window;Check(window.hwnd!=nullptr,"Private clear window");
    D3D11Renderer renderer;Check(renderer.Initialize(window.hwnd,320*scale,480*scale),"Private clear renderer");
    PrStage2VramAtlas::Projection atlas(s.host.vram);
    renderer.BeginFrame(1,1,1);
    const auto background=ReadFrame(renderer);
    s.host.Bind(renderer,atlas,{0,7,9,3,4,19,27},{0,0,320,480,0,0,float(320*scale),float(480*scale)});
    s.Write8(0x8005D736u,0u);s.Write16(0x8005D738u,1024u);s.Write16(0x8005D73Au,512u);
    s.Write32(0x8005D82Cu,0x12345678u);
    Check(PrStage2GpuDirect::ClearFramebuffer8001B1B0(s,0x1C8u,0x264u,0x332u)==0,"First private clear return");
    const auto first=s.ReadGpuCommandSource();Check(first.local && first.address==0u,"First private clear identity");
    Check(s.host.pending && s.host.completions==0u && ReadFrame(renderer)==background,"Private clear completed early");
    Check(PrStage2GpuDirect::ClearFramebuffer8001B1B0(s,0,180,0)==1,"Queued private clear return");
    Check(s.Read32(0x80094454u)==0u && s.Read32(0x80094458u)==Xy(320,480),"Private RECT not copied to native queue");
    Check(s.ReadGpuCommandSource().local==first.local,"Queued submission replaced last executed source");
    // Both C++ activation records have returned. Nothing borrows their bytes.
    volatile uint8_t stackChurn[1024];for(auto& x:stackChurn)x=0xA5u;
    s.Pump();
    Check(s.host.pending && s.host.completions==1u,"First completion failed to drain queued private clear");
    Check(!s.ReadGpuCommandSource().local && s.ReadGpuCommandSource().address==0x80094454u,
          "Queue consumption did not replace native identity with real queue RAM source");
    for(auto p:ReadFrame(renderer))Near(p,200,100,50,"First private clear pixels");
    s.Pump();
    Check(!s.host.pending && s.host.completions==2u,"Second private clear unfinished");
    for(auto p:ReadFrame(renderer))Near(p,0,180,0,"Queued private clear pixels after return");
    Check(PrStage2GpuDirect::ClearFramebuffer8001B1B0(s,0,0,200)==0,"Repeated private clear return");
    Check(s.ReadGpuCommandSource().local && s.ReadGpuCommandSource().local!=first.local,"Repeated activation reused live identity");
    s.Pump();for(auto p:ReadFrame(renderer))Near(p,0,0,200,"Repeated private clear pixels");
    std::cout<<"private-clear "<<scale<<" 3 "<<460800u*scale*scale<<'\n';
}

void CheckBootstrap(PrivateGpu& s,int scale) {
    Window window;Check(window.hwnd!=nullptr,"Full private bootstrap window");
    D3D11Renderer renderer;Check(renderer.Initialize(window.hwnd,320*scale,480*scale),"Full private bootstrap renderer");
    PrStage2VramAtlas::Projection atlas(s.host.vram);renderer.BeginFrame(1,1,1);
    s.host.Bind(renderer,atlas,{},{0,0,320,480,0,0,float(320*scale),float(480*scale)});
    s.host.pumpOnStatus=true;s.Write32(0x80057064u,0u);
    const auto result=PrStage2GraphBootstrapDirect::InitGraphics8001C470(s);
    Check(result==0 && s.privateEntries==1u && s.host.resets==2u,"Full bootstrap clear was not executed natively");
    Check(PrStage2GpuDirect::DrawSync80044B3C(s,0u)==0,"Full private bootstrap DMA unfinished");
    Check(s.host.completions==7u && !s.host.pending,"Expected six environments and one real clear");
    Check(s.Read32(0x800917FCu)==320u && s.Read32(0x8009182Cu)==240u && s.host.gte.h==440u,
          "Full private bootstrap geometry regressed");
    Check(s.Read16(0x8008ECAEu)==240u,"Full private bootstrap buffer origin");
    for(auto p:ReadFrame(renderer))Near(p,0,0,0,"Full bootstrap did not clear both framebuffers");
    std::cout<<"private-bootstrap "<<scale<<" 2 7 "<<153600u*scale*scale<<'\n';
}
}

int main(int argc,char** argv) {
    try {
        if(argc!=3)return 1;
        {PrivateGpu device(argv[1],argv[2]);CheckSourceTransport(device);}
        for(int scale:{1,4}) {
            {PrivateGpu device(argv[1],argv[2]);CheckClears(device,scale);}
            {PrivateGpu device(argv[1],argv[2]);CheckBootstrap(device,scale);}
        }
        std::cout<<"private-rect-gpu-pass\n";return 0;
    } catch(const std::exception& error) {std::cerr<<error.what()<<'\n';return 2;}
}