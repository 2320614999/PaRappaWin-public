// GPU scenarios are adapted from the source-bound native-dispatch test.
// Only the explicit lower device fixture remains; all RAM uses product storage.
#define PR_STAGE2_BOOTSTRAP_GPU_FIXTURE_ONLY
#include "test_stage2_graph_bootstrap_gpu.cpp"
#undef PR_STAGE2_BOOTSTRAP_GPU_FIXTURE_ONLY
#include "test_stage2_data_services_contract.h"
#include "pr/pr_stage2_data_services.h"
#include "pr/pr_stage2_callback_init_direct.h"
#include "test_stage2_memory_services_contract.h"
namespace {
using Args=std::initializer_list<uint32_t>;
struct IrqReturn {};
struct NativeGpu : PrStage2DataServices::Services {
    BootstrapGpuFixture host;
    uint16_t mask=0,status=0;
    uint32_t timerMode=0,savedContexts=0,hooks=0,criticalEnds=0;
    uint32_t failFunction=0;
    bool failVBlank=false;
    std::vector<uint32_t> vblankCalls;
    uint32_t control=0,pendingChannels=0,delivered=0;
    bool delivering=false;
    std::vector<uint32_t> external;
    NativeGpu(const char* scus,const char* disc)
        :Services(std::filesystem::u8path(scus)),host(scus,disc){
        // Actual SCUS data and native startup clearing own initialization.
        // The old fixture supplies devices only; its RAM must remain poisoned.
        DataContract::Reject([&]{Read32(0x800000A0u);});
        DataContract::Reject([&]{Read32(0x801C3870u);});
        std::fill(host.ram.begin(),host.ram.end(),0xBDu);
    }
    uint8_t ReadDevice8(uint32_t) override {throw std::runtime_error("unexpected byte MMIO");}
    uint16_t ReadDevice16(uint32_t a) override {
        if(a==0x1F801074u)return mask;
        Check(a==0x1F801070u,"unexpected halfword MMIO");return status;
    }
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
    void WriteDevice16(uint32_t a,uint16_t v) override {
        if(a==0x1F801074u){mask=v;return;}
        Check(a==0x1F801070u,"unexpected halfword MMIO");status&=v;
    }
    void WriteDevice32(uint32_t a,uint32_t v) override {
        if(a==0x1F801114u){timerMode=v;return;}
        if(a==0x1F8010F4u){
            pendingChannels&=~(v>>24u);control=v&0x00FFFFFFu;
            if((control&0x00800000u)&&((control>>16u)&pendingChannels))status|=8u;
            return;
        }
        Check(a==0x1F801814u||host.io.count(a),"unexpected word MMIO write");
        Check(a!=0x1F8010E8u,"OTC not supplied by this GPU-only fixture");
        host.Write32(a,v);
    }
    int32_t CallExternal(uint32_t f,Args args) override {
        external.push_back(f);
        if(f==failFunction)throw std::runtime_error("injected lower BIOS failure");
        if(f==0x80047F5Cu) {
            Check(std::vector<uint32_t>(args)==std::vector<uint32_t>{0x80055FB0u}&&Read32(0x80055FB4u)==0u,"test context ABI");
            ++savedContexts;return 0; // Explicit cold test boundary, not native setjmp.
        }
        if(f==0x80048A30u) {
            Check(std::vector<uint32_t>(args)==std::vector<uint32_t>{0x80055FB0u}&&Read32(0x80055FB4u)==0x80056F90u,"hook before context publication");
            ++hooks;return -73;
        }
        if(f==0x80048960u) {
            Check(args.size()==0u&&Read32(0x80056FF4u)==0x80035F24u&&Read32(0x80056FE4u)==0x80036150u,"callback slots published too late");return -99;
        }
        if(f==0x80048A50u){Check(args.size()==0u,"critical ABI");++criticalEnds;return -17;}
        if(f==0x80048AF0u){Check(std::vector<uint32_t>(args)==std::vector<uint32_t>{3u,0u},"counter callback ABI");return -11;}
        if(f==0x80048A10u){Check(args.size()==0u,"exception-return ABI");throw IrqReturn{};}
        if(f==0x800F4000u||f==0x800F4004u) {
            Check(args.size()==0u,"VBlank callback arguments");
            Check(Read16(0x80055F7Au)==1u&&(status&1u)==0u,"VBlank callback preceded IRQ acknowledge");
            vblankCalls.push_back(f);
            if(failVBlank)throw std::runtime_error("injected VBlank failure");
            if(f==0x800F4000u)Call(0x80035F24u,{1u,0x800F4004u});
            return 0;
        }
        if(f==0x80047FBCu) {
            Check(args.size()==3u&&args.begin()[2]<=92u,"BIOS copy contract");
            const auto a=args.begin();std::vector<uint8_t> bytes(a[2]);
            for(uint32_t i=0;i<a[2];++i)bytes[i]=Read8(a[1]+i);
            for(uint32_t i=0;i<a[2];++i)Write8(a[0]+i,bytes[i]);
            return a[0]<=0x7FFFFFFFu?int32_t(a[0]):int32_t(int64_t(a[0])-0x100000000LL);
        }
        // Only these lower BIOS/GTE/clock/output inputs are supplied by the
        // old device fixture. None of the new native routes may delegate here.
        Check(f==0x80047FFCu||f==0x80048950u||f==0x800489F0u||
              f==0x80048AE0u||f==0x80040D20u||f==0x80035560u,"translated call escaped native ownership");
        const std::vector<uint32_t> a(args);
        if(f==0x80047FFCu&&a==std::vector<uint32_t>{0x8001243Cu,0x8005D6ECu,0x8005D734u})return 0;

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
        Check(Read16(0x80055F78u)==0u,"test bypassed cold initialization");
        Check(uint32_t(Call(0x80035744u,{}))==0x80055F78u,"native callback initialization result");
        Check(mask==9u&&timerMode==0x107u&&Read16(0x80055FA8u)==9u,"native IRQ setup incomplete");
        Check(Read32(0x80055F7Cu)==0x80035EACu&&Read32(0x80055F88u)==0x80035FCCu,"native IRQ vectors missing");
        const auto before=external.size();
        Check(Call(0x80035744u,{})==0&&external.size()==before,"repeat initialization had lower effects");
        Check(savedContexts==1u&&hooks==1u&&criticalEnds==1u,"test BIOS boundaries missing/duplicated");
    }
    bool Deliver() {
        if(delivering||!(status&mask))return false;
        delivering=true;bool returned=false;
        try {(void)Call(0x800359B8u,{});}
        catch(const IrqReturn&){returned=true;}
        catch(...){delivering=false;throw;}
        delivering=false;
        Check(returned&&Read16(0x80055F7Au)==0u,"outer IRQ did not unwind at exception return");
        ++delivered;return true;
    }
    void VBlank() {status|=1u;}
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
            if((control&0x00800000u)&&((control>>16u)&pendingChannels))status|=8u;
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
    if(masked)Check(s.Call(0x800358C0u,{0})==9,"mask old value");
    volatile uint8_t churn[1024];for(auto& x:churn)x=0xA5u;
    for(uint32_t i=0;i<3u;++i) {
        s.Complete();
        if(masked&&i==0u) {
            Check(s.delivered==irqBefore&&!s.host.pending&&s.pendingChannels==4u&&
                  s.ReadGpuCommandSource().local==source.local,"masked IRQ consumed the next private command");
            Check(!s.Deliver()&&s.Call(0x800358C0u,{9})==0&&s.Deliver(),"pending IRQ not delivered after unmask");
        }
        for(auto pixel:ReadFrame(renderer))Near(pixel,i==0u?200:0,i==1u?180:0,i==2u?160:0,"queued native IRQ pixels");
        Check(!s.ReadGpuCommandSource().local,"queue source did not replace local identity");
    }
    Check(s.host.completions==10u&&s.delivered-irqBefore==2u&&!s.host.pending&&s.pendingChannels==0u&&
          s.Read32(0x80057048u)==0u&&s.Read32(0x8005D838u)==s.Read32(0x8005D83Cu),"native drain final state");
    Check(s.mask==9u&&s.host.dmaCallback==0u,"legacy callback or changed mask");
    Check(std::all_of(s.host.ram.begin(),s.host.ram.end(),[](uint8_t b){return b==0xBDu;}),"old fixture RAM was modified");
    Check(s.savedContexts==1u&&s.hooks==1u&&s.criticalEnds==1u,"graphics reinitialized callback state");
    const uint32_t ticks=s.Read32(0x80057034u), delivered=s.delivered;
    Check(s.Call(0x800357D4u,{0x800F4000u})==0,"VBlank wrapper slot already set");
    s.VBlank();Check(s.Deliver()&&s.Read32(0x80057034u)==ticks+1u,"first VBlank not dispatched");
    Check(s.vblankCalls==std::vector<uint32_t>{0x800F4000u,0x800F4004u},"VBlank dispatcher cached later slots");
    Check(s.Call(0x800358C0u,{8u})==9,"VBlank mask previous value");s.VBlank();
    Check(!s.Deliver()&&s.Read32(0x80057034u)==ticks+1u&&(s.status&1u),"masked VBlank lost or delivered early");
    s.Call(0x800358C0u,{9u});Check(s.Deliver()&&s.Read32(0x80057034u)==ticks+2u,"latched VBlank not dispatched");
    Check(s.delivered==delivered+2u&&s.vblankCalls.size()==4u&&s.status==0u,"VBlank final state");
    std::cout<<"callback-init-gpu "<<scale<<' '<<int(masked)<<" 10 2 "<<614400u*scale*scale<<'\n';
}
void CheckAbiAndFailures(const char* scus,const char* disc) {
    uint32_t arityChecks=0;
    for(uint32_t address:{0x80035744u,0x800357D4u,0x800358DCu,0x800359B8u,0x80035E28u,
                          0x80035E54u,0x80035EACu,0x80035F24u,0x80035F50u}) {
        const bool two=address==0x80035E28u||address==0x80035F24u||address==0x80035F50u;
        const bool one=address==0x800357D4u;
        auto test=[&](Args args) {
            if(args.size()==(two?2u:(one?1u:0u)))return;
            MemoryContract::Probe p;bool rejected=false;
            try{p.Call(address,args);}catch(const std::invalid_argument&){rejected=true;}
            Check(rejected&&p.events.empty(),"callback ABI failure touched a device");++arityChecks;
        };
        test({});test({1u});test({1u,2u});test({1u,2u,3u});test({1u,2u,3u,4u});
    }
    MemoryContract::Probe unknown;int32_t sentinel=123;
    Check(!PrStage2CallbackInitDirect::TryDispatch(unknown,0x800FFFFCu,{},sentinel)&&sentinel==123&&unknown.events.empty(),"unknown callback route changed output");
    for(uint32_t lower:{0x80047F5Cu,0x80048A30u,0x80048AE0u,0x80048AF0u,0x80048960u,0x80048A50u}) {
        NativeGpu s(scus,disc);s.failFunction=lower;bool failed=false;
        try{s.Call(0x80035744u,{});}catch(const std::runtime_error& e){failed=std::string(e.what())=="injected lower BIOS failure";}
        Check(failed&&s.external.back()==lower,"lower BIOS failure swallowed or continued");
        const auto flag=s.Read16(0x80055F78u);
        Check(flag==((lower==0x80047F5Cu||lower==0x80048A30u)?0u:1u),"failure rewrote original partial state");
    }
    NativeGpu bad(scus,disc);bad.InitIrq();bad.Call(0x80035F24u,{0u,0x800F4000u});bad.failVBlank=true;bad.VBlank();bool failed=false;
    try{bad.Deliver();}catch(const std::runtime_error& e){failed=std::string(e.what())=="injected VBlank failure";}
    Check(failed&&!bad.delivering&&bad.Read16(0x80055F7Au)==1u&&bad.Read32(0x80057034u)==1u&&bad.status==0u,"callback failure changed original partial effects");
    std::cout<<"callback-init-contract "<<arityChecks<<" 6 1\n";
}
}

int main(int argc,char** argv) {
    try {
        if(argc!=3&&argc!=4)return 1;
        CheckAbiAndFailures(argv[1],argv[2]);
        if(argc==4&&std::string(argv[3])=="--contract-only")return 0;
        for(int scale:{1,4})for(bool masked:{false,true}) {
            NativeGpu s(argv[1],argv[2]);CheckGpu(s,scale,masked);
        }
        std::cout<<"callback-init-pass\n";return 0;
    }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 2;}
    catch(const IrqReturn&){std::cerr<<"unexpected exception-return boundary\n";return 2;}
}
