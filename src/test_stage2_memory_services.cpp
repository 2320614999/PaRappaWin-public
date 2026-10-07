// GPU scenarios are adapted from the source-bound native-dispatch test.
// Only the explicit lower device fixture remains; all RAM uses product storage.
#define PR_STAGE2_BOOTSTRAP_GPU_FIXTURE_ONLY
#include "test_stage2_graph_bootstrap_gpu.cpp"
#undef PR_STAGE2_BOOTSTRAP_GPU_FIXTURE_ONLY
#include "test_stage2_memory_services_contract.h"
#include "pr/pr_stage2_memory_services.h"
namespace {
using Args=std::initializer_list<uint32_t>;
struct NativeGpu : PrStage2MemoryServices::Services {
    BootstrapGpuFixture host;
    uint16_t mask=0;
    uint32_t control=0,pendingChannels=0,delivered=0;
    bool delivering=false;
    std::vector<uint32_t> external;
    NativeGpu(const char* scus,const char* disc):host(scus,disc){
        // Explicit test startup RAM: SCUS data plus the fixture's initial
        // bytes. This is NOT a production BIOS/startup initialization claim.
        for(uint32_t i=0;i<host.ram.size();++i)Write8(0x80000000u+i,host.ram[i]);
        std::fill(host.ram.begin(),host.ram.end(),0xBDu);
    }
    uint8_t ReadDevice8(uint32_t) override {throw std::runtime_error("unexpected byte MMIO");}
    uint16_t ReadDevice16(uint32_t a) override {Check(a==0x1F801074u,"unexpected halfword MMIO");return mask;}
    uint32_t ReadDevice32(uint32_t a) override {
        if(a==0x1F8010F4u)return control|(pendingChannels<<24u)|
            (((control&0x00800000u)&&((control>>16u)&pendingChannels))?0x80000000u:0u);
        if(a==0x1F8010A8u&&host.pending&&host.pumpOnStatus&&!host.pumping&&!delivering)Complete();
        Check(a==0x1F801814u||a==0x1F801810u||host.io.count(a),"unexpected word MMIO");
        Check(a!=0x1F8010E8u,"OTC not supplied by this GPU-only fixture");
        const bool old=host.pumping;host.pumping=true;
        try {const auto v=host.Read32(a);host.pumping=old;return v;}
        catch(...){host.pumping=old;throw;}
    }
    void WriteDevice8(uint32_t,uint8_t) override {throw std::runtime_error("unexpected byte MMIO");}
    void WriteDevice16(uint32_t a,uint16_t v) override {Check(a==0x1F801074u,"unexpected halfword MMIO");mask=v;}
    void WriteDevice32(uint32_t a,uint32_t v) override {
        if(a==0x1F8010F4u){pendingChannels&=~(v>>24u);control=v&0x00FFFFFFu;return;}
        Check(a==0x1F801814u||host.io.count(a),"unexpected word MMIO write");
        Check(a!=0x1F8010E8u,"OTC not supplied by this GPU-only fixture");
        host.Write32(a,v);
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
        const std::vector<uint32_t> a(args);
        if(f==0x80047FFCu&&a==std::vector<uint32_t>{0x8001243Cu,0x8005D6ECu,0x8005D734u})return 0;
        if(f==0x80035744u&&a.empty())return 0;
        if(f==0x80048950u&&a==std::vector<uint32_t>{0x5D6ECu})return 0;
        if(f==0x800489F0u&&a==std::vector<uint32_t>{0x20000001u,0x800882F0u}) {
            Check(Read32(0x800882F0u)==0xFFFFFFFFu&&Read32(0x80091830u)==0u,"native PAD data missing");return 0;
        }
        if(f==0x80048AE0u&&a==std::vector<uint32_t>{0u})return -77;
        if(f==0x80040D20u&&a.empty())return 0;
        if(f==0x80035560u&&a==std::vector<uint32_t>{0xFFFFFFFFu})return 0;
        throw std::runtime_error("unexpected lower test ABI");
    }
    void CallExternalVoid(uint32_t,Args) override {throw std::runtime_error("unexpected external void call");}
    PrStage2LifecycleDirect::Words64 Call64(uint32_t,Args) override {throw std::runtime_error("unexpected Call64");}
    PrStage2LifecycleDirect::Vector32 NormalizeVector8003A3DC(PrStage2LifecycleDirect::Vector32) override {throw std::runtime_error("unexpected normalize");}
    int32_t SetCdLocation800367A4(uint32_t v) override {return host.SetCdLocation800367A4(v);}
    int32_t LoadImage80044D64(PrStage2LifecycleDirect::ImageRect r,uint32_t a) override {return host.vram.UploadImageWords(*this,r,a);}
    PrPsxGteDirect::MatrixRegisters& MatrixGte() override {return host.gte;}
    [[noreturn]] void Exit(uint32_t,Args) override {throw std::runtime_error("unexpected Exit");}
    [[noreturn]] void Break(uint32_t,uint32_t) override {throw std::runtime_error("unexpected BREAK");}
    void Bind(D3D11Renderer& target,PrStage2VramAtlas::Projection& atlas,Ot::DrawState draw,Ot::Viewport view) {
        Check(!host.pending,"unfinished test DMA");
        host.renderer=&target;host.projection=&atlas;host.state=draw;host.view=view;
        host.starts=host.completions=host.dmaCallback=0;host.consumedHeads.clear();
        host.interruptMask=8;host.pumpOnStatus=false;host.pumping=false;
        host.infoCommands.clear();host.environmentCommands.clear();host.io[0x1F8010A8u]=0;
        Write32(0x8005D838u,0);Write32(0x8005D83Cu,0);Write32(0x8005D740u,0);Write8(0x8005D735u,1);
    }
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
    s.Bind(renderer,atlas,{},{0,0,320,480,0,0,float(320*scale),float(480*scale)});
    s.InitIrq();s.host.pumpOnStatus=true;s.Write32(0x80057064u,0);
    Check(s.Call(0x8001C470u,{})==0&&s.Call(0x80044B3Cu,{0})==0,"native bootstrap failed");
    Check(s.host.completions==7u&&s.host.resets==2u&&!s.host.pending,"bootstrap did not consume seven actual packets");
    Check(s.host.gte.h==440u&&s.Read32(0x800917FCu)==320u&&s.Read32(0x8009182Cu)==240u,"bootstrap geometry");
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
    Check(std::all_of(s.host.ram.begin(),s.host.ram.end(),[](uint8_t b){return b==0xBDu;}),"old fixture RAM was modified");
    std::cout<<"owned-memory-gpu "<<scale<<' '<<int(masked)<<" 10 2 "<<614400u*scale*scale<<'\n';
}
}
int main(int argc,char** argv) {
    try {
        MemoryContract::Run();
        if(argc==2&&std::string(argv[1])=="--contract-only")return 0;
        if(argc!=3)return 1;
        for(int scale:{1,4})for(bool masked:{false,true}) {
            NativeGpu s(argv[1],argv[2]);CheckGpu(s,scale,masked);
        }
        std::cout<<"memory-services-pass\n";return 0;
    }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 2;}
}
