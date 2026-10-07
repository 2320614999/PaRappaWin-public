#include "pr_stage2_loading_device_session.h"
#include "pr_stage2_game_hud_direct.h"
#include "pr_stage2_retry_direct.h"
#include "pr_stage2_loading_packets_direct.h"
#include "pr_stage2_loading_work_direct.h"
#include "pr_stage2_gpu_direct.h"
#include "pr_stage2_soft_float_direct.h"
#include "pr_stage2_loading_packets_direct.h"
#include <algorithm>
#include <sstream>
#include <iostream>
#include <vector>
#include <cstdint>
#include <cmath>
#include <fstream>

namespace PrStage2LoadingDeviceSession {
namespace {
namespace F=PrStage2FrameTask;
struct IrqReturn{};
std::string Message(uint32_t address,uint32_t width,bool writing){
    std::ostringstream text;text<<"S2 unbound "<<(width?(writing?"write":"read"):"function")<<" 0x"<<std::hex<<address;
    if(width)text<<" width="<<std::dec<<width;
    return text.str();
}
int32_t Signed(uint32_t value){return value<=0x7FFFFFFFu?int32_t(value):int32_t(int64_t(value)-0x100000000LL);}
int32_t Ratan2Psx(int32_t y,int32_t x){
    if(x==0&&y==0)return 0;
    constexpr double tau=6.28318530717958647692;
    return int32_t(std::lround(std::atan2(double(y),double(x))*(4096.0/tau)));
}
uint64_t IntegerSqrt(uint64_t value){
    uint64_t bit=uint64_t(1)<<62,result=0;
    while(bit>value)bit>>=2;
    while(bit){if(value>=result+bit){value-=result+bit;result=(result>>1)+bit;}else result>>=1;bit>>=2;}
    return result;
}

int16_t SignedHalf(uint16_t value) {
    return value < 0x8000u ? static_cast<int16_t>(value)
        : static_cast<int16_t>(static_cast<int32_t>(value)-65536);
}


}
Unbound::Unbound(uint32_t a,uint32_t w,bool write):std::runtime_error(Message(a,w,write)),address(a),width(w),writing(write){}
Session::Session(const std::filesystem::path& scus,const std::filesystem::path& overlay,
                 const std::filesystem::path& disc,D3D11Renderer& renderer,IAudioSink& sink)
    :Services(scus,overlay),disc_(disc),discPath_(disc),dataPath_(overlay.parent_path().parent_path()),atlas_(vram_),
     gpu_(*this,atlas_,irq_,renderer,
          {0,0,320,480,0,0,float(renderer.GetWidth()),float(renderer.GetHeight())},
          // Movie pages are submitted at the native four-VBlank cadence. A
          // blocking Present(1) adds a second, compositor-owned clock and can
          // turn one late handoff into a 58/76 ms pair even when the native
          // callback remains exactly four VBlanks apart. Keep the handoff
          // nonblocking on every desktop refresh; the PSX clock remains the
          // sole owner of movie speed and the window compositor retains the
          // immutable page until the next native frame.
          false),
     output_(spu_,sink){}
void Session::SeedCompactRailAssets() {
    if (compactRailAssetsSeeded_) return;
    const std::array<std::string, 6> names{{
        "ON_SI.TIM", "PA_SI.TIM", "BAR_BAL1.TIM", "BAR_BAL2.TIM",
        "BAR_STR1.TIM", "BAR_STR2.TIM"}};
    std::vector<std::filesystem::path> roots;
    const auto addRoot = [&roots](std::filesystem::path root) {
        if (std::find(roots.begin(), roots.end(), root) == roots.end()) roots.push_back(std::move(root));
    };
    addRoot(dataPath_ / "S2" / "COMPO01" / "block000_Tim");
    addRoot(discPath_.parent_path() / "S2" / "COMPO01" / "block000_Tim");
    addRoot(discPath_.parent_path() / ".." / "out_all" / "S1" / "COMPO01" / "block000_Tim");
    addRoot(discPath_.parent_path() / "out_all" / "S1" / "COMPO01" / "block000_Tim");
    addRoot(std::filesystem::current_path() / "out_all" / "S1" / "COMPO01" / "block000_Tim");
    for (const auto& root : roots) {
        bool complete = true;
        for (const auto& name : names)
            if (!std::filesystem::is_regular_file(root / name)) { complete = false; break; }
        if (!complete) continue;
        for (const auto& name : names) {
            std::ifstream file(root / name, std::ios::binary | std::ios::ate);
            if (!file) throw std::runtime_error("S2 compact rail asset cannot be opened: " + (root / name).u8string());
            const auto length = file.tellg();
            if (length <= 0) throw std::runtime_error("S2 compact rail asset is empty: " + (root / name).u8string());
            std::vector<uint8_t> bytes(static_cast<std::size_t>(length));
            file.seekg(0); file.read(reinterpret_cast<char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
            if (!file || !vram_.SeedTim(bytes.data(), bytes.size()))
                throw std::runtime_error("S2 compact rail asset is not a supported TIM: " + (root / name).u8string());
        }
        compactRailAssetsSeeded_ = true;
        return;
    }
    throw std::runtime_error("S2 compact rail assets missing (expected S1/COMPO01/block000_Tim)");
}
Session::~Session(){Cancel();output_.Stop();}
uint64_t Session::Scanlines()const{
    const auto elapsed=std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now()-epoch_).count();
    const uint64_t ns=elapsed>0?uint64_t(elapsed):0u;
    // NTSC non-interlaced PSX scanout is 59.826 Hz, not the monitor's 60 Hz.
    // Keep the native clock independent of DXGI refresh. Rounding it up to
    // 60 moves the original VSync(2) consumer ahead of the optical producer
    // and can turn a one-sector STR variation into an extra full loop wait.
    // Source: psx-spx.consoledev.net/graphicsprocessingunitgpu/#gpu-timings
    constexpr uint64_t kMilliScanlinesPerSecond=263ull*59826ull;
    return ((ns/1000000000ull)*kMilliScanlinesPerSecond+
            ((ns%1000000000ull)*kMilliScanlinesPerSecond)/1000000000ull)/1000ull;
}
void Session::Await(F::WaitKind kind,uint32_t target){
    if(!task_)throw std::logic_error("S2 native wait has no retained owner");
    task_->Suspend(kind,0x80035560u,target);
}
void Session::WaitDevice(uint32_t address,uint32_t target){
    if(!task_)throw std::logic_error("S2 device wait has no retained owner");
    task_->Suspend(F::WaitKind::PlatformDependency,address,target);
}
void Session::AwaitDeviceProgress(uint32_t address){
    if(irqActive_){
        const uint32_t physical=address&0x1FFFFFFFu;
        // The retained fiber owns the complete IRQ activation, so a dependent
        // device wait may suspend here and resume on the same native stack.
        // Poll() services the device while irqActive_ remains set; dispatch is
        // intentionally deferred until the current exception continuation has
        // returned, avoiding recursive IRQ entry and host-side busy spinning.
        if(physical==0x1F8010A8u||physical==0x1F8010E8u){
            WaitDevice(address,uint32_t(physical==0x1F8010E8u?otc_.Serial():gpu_.Serial()));
            return;
        }
        throw std::logic_error("S2 IRQ callback attempted to await an unowned device");
    }
    WaitDevice(address,uint32_t((address&0x1FFFFFFFu)==0x1F8010E8u?otc_.Serial():gpu_.Serial()));
}
void Session::AwaitPadRelease80035510(){
    WaitDevice(0x80035510u,0u);
}
void Session::AwaitTransitionPresentation(){
    if(!task_)throw std::logic_error("S2 native presentation has no retained owner");
    // A native transition has just submitted its final display page. Mark it
    // for the host pump and retain the source stack until that page has had
    // one real PresentDisplay opportunity. This is a scheduling boundary only:
    // it does not advance the PSX VBlank counter or synthesize a frame.
    presentRequested_=true;
    presentationEdgePending_=false;
    presentationEdgeTarget_=0u;
    task_->Suspend(PrStage2FrameTask::WaitKind::NativePresentation,
                   0x8001EBF4u, gpu_.Serial());
}
F::State Session::StartGraphics(){
    if(task_||graphicsReady_||GetPhase()!=PrStage2SceneEntry::Phase::Loaded)
        throw std::logic_error("S2 graphics startup already owned or out of order");
    SeedCompactRailAssets();
    task_=std::make_unique<F::Task>([this]{
        // Execute the actual shared native graphics producer. Do not seed draw offsets,
        // DMA enables, packet tables or framebuffer contents from a test.
        Call(0x8001C470u,{});
        while(gpu_.Pending())WaitDevice(0x1F8010A8u,uint32_t(gpu_.Serial()));
        InitializeAdditionalDevices();
        graphicsReady_=true;return 0;
    });
    return task_->Start();
}
F::State Session::StartScene(){
    if(!task_||task_->GetState()!=F::State::Completed||GetPhase()!=PrStage2SceneEntry::Phase::InitializerReturned)
        throw std::logic_error("The native initializer has not returned; main scene cannot start");
    task_=std::make_unique<F::Task>([this]{return RunScene();});
    return task_->Start();
}
int32_t Session::TaskResult()const{
    if(!task_)throw std::logic_error("No native activation result");
    return task_->Result();
}
F::State Session::StartGraphicsHandoff(){
    if(!task_ || task_->GetState()!=F::State::Completed)
        throw std::logic_error("S2 graphics handoff requires the scene to return");
    task_=std::make_unique<F::Task>([this]{
        return PrStage2GpuDirect::DrawSync80044B3C(*this,0u);
    });
    return task_->Start();
}
F::State Session::StartInitialization(){
    if(!graphicsReady_||!task_||task_->GetState()!=F::State::Completed||initStarts_)
        throw std::logic_error("S2 initializer cannot be restarted or entered before graphics");
    output_.Start();
    task_=std::make_unique<F::Task>([this]{
        InitializeGlobals();
        PrStage2LoadingWorkDirect::Initialize8001E6D0(*this);
        ++initStarts_;return InitializeScene();
    });
    return task_->Start();
}
void Session::DispatchIrq(){
    if(irqActive_||!AdditionalInterruptsEnabled()||!irq_.Pending())return;
    irqActive_=true;++irqEntries_;
    try{Call(0x800359B8u,{});irqActive_=false;throw std::logic_error("S2 IRQ returned without its exception-return boundary");}
    catch(const IrqReturn&){irqActive_=false;}
    catch(...){irqActive_=false;throw;}
}
void Session::Events(bool allowFrameEvents){
    output_.RethrowFailure();
    // Sample the wall-clock scanout edge before servicing devices.  The
    // movie output is serviced below in the same event pass; if the edge is
    // accounted for afterwards, a frame that became ready on that edge sees
    // the previous VBlank deadline and waits for one more host pass.  That
    // adds a variable 0-16 ms delay to an otherwise fixed 15 Hz cadence.
    const uint64_t lines=Scanlines(),blank=(lines+23u)/263u;
    gpu_.SetScanline(uint32_t(lines%263u),((lines/263u)&1u)!=0u);
    const bool vertical=blank>seenBlank_;
    if(vertical){
        const uint64_t edges=blank-seenBlank_;
        seenBlank_=blank;
        // VBlank is a hardware edge clock, not a permission to run the
        // suspended source continuation. Keep every edge while a device wait
        // owns the native stack; dispatching one boolean here would make the
        // 80057034 counter run slower than the actual wall-clock display.
        // Count the edge when its interrupt is delivered below. This keeps
        // the observable VBlank timeline monotonic in one-edge steps even if
        // a host pass samples several physical edges at once.
        pendingVBlankEvents_+=edges;
        presentRequested_=true;
    }
    if(otc_.Pending())otc_.Complete(otc_.Serial(),*this,irq_);
    if(gpu_.Pending())gpu_.Service();
    if(pendingVBlankEvents_!=0u&&allowFrameEvents&&!irqActive_){
        // 每轮至多交付一个积压边沿。IRQ 内等待 GPU 时会嵌套 Events；
        // 无界 while 会把这期间新增的边沿也吞进旧回调，饿死前台续体。
        // 未交付边沿留到下一轮，原回调仍是 80057034 的唯一递增者。
        --pendingVBlankEvents_;
        ++wallBlanks_;
        irq_.SetLine(PrStage2InterruptDevice::Source::VBlank,true);
        irq_.SetLine(PrStage2InterruptDevice::Source::VBlank,false);
        DispatchIrq();
    }
    // Media deadlines observe the edge delivered above. Publishing before
    // that counter advances makes a ready frame miss this source VSync and
    // exposes a different image depending on how often the host polls.
    ServiceAdditionalDevices();
    DispatchIrq();
    // A DMA callback may submit a fresh OTC transfer while the interrupt
    // dispatcher is unwinding. Recheck after that callback so an outer source
    // wait never observes a transfer that was created late in this event pass.
    if(otc_.Pending())otc_.Complete(otc_.Serial(),*this,irq_);
    // Present is still performed by the host Poll owner below, never by the
    // translated IRQ stack. This pass may leave that stack suspended on a
    // real GPU fence, so the host must be allowed to publish its already
    // submitted page and keep the D3D queue advancing.
}
F::State Session::Poll(bool deliver){
    if(!task_)throw std::logic_error("S2 session has not started");
    output_.RethrowFailure();
    if(task_->GetState()!=F::State::Waiting)return task_->GetState();
    const auto request=task_->Pending();
    // 800201AC/80020110 return to their caller only after the four-frame tail
    // has finished.  Product presentation is an extra host scheduling edge,
    // so do not resume that retained source stack until its final page has
    // actually been accepted by the swap chain.  Previously a blocked GPU
    // fence could leave NativePresentation pending, then the next Poll()
    // resumed the source first; 801C74E4 could consequently submit its first
    // gameplay page before the last transition page became visible.
    if(request.kind==F::WaitKind::NativePresentation){
        // Poll(false) is deliberately eventless in the device contracts. It
        // cannot acknowledge a presentation boundary or resume the caller.
        // With a real host event, service the device owners and dispatch any
        // already-pending DMA work before publishing the immutable native
        // page.  The same event pass is required to close an OTC->GPU chain
        // and arm the GPU channel used by the final transition page.
        if(!deliver)return task_->GetState();
        Events(false);
        if(!presentationEdgePending_){
            if(!PresentIfReady())return task_->GetState();
            // The final page has now been accepted by the host swap chain.
            // Keep the source stack suspended until one subsequent sampled
            // scanout edge. This is a host visibility boundary only: the
            // native counter and IRQ callback remain untouched.
            presentationEdgePending_=true;
            presentationEdgeTarget_=seenBlank_+1u;
            return task_->GetState();
        }
        if(seenBlank_<presentationEdgeTarget_)return task_->GetState();
        presentationEdgePending_=false;
        presentationEdgeTarget_=0u;
    }
    if(!deliver)return task_->Resume(request.ticket);
    // VBlank is an independent hardware clock. It continues to edge while
    // the retained source waits on GPU/CD/SPU completion, so do not suppress
    // its callback merely because the current continuation is a platform
    // dependency wait.
    const auto state=task_->Resume(request.ticket,[this]{Events(true);});
    // A foreground transition can submit its next DMA before yielding again.
    // Waiting for an entirely idle source queue here would starve scanout.
    // Submitted page snapshots are immutable SRVs whose copies precede this
    // Present on the same D3D immediate context. Present does not acknowledge
    // their DMA fence: only the later Service/GetData path may do that.
    if(state!=F::State::Failed) PresentIfReady();
    return state;
}
bool Session::PresentIfReady(){
    if(presentationEdgePending_) return false;
    if(!presentRequested_ || !graphicsReady_ || !gpu_.DisplayEnabled()) return false;
    // The isolated device contract may scan out an immutable submitted page.
    // A product frame submits several ordering tables after its page flip, so
    // exposing a submitted-but-unfinished batch can show an intermediate page
    // and make UI/models disappear for one presentation.
    if(productPresentationRequiresIdleGpu_ && gpu_.Pending()) return false;
    if(!productPresentationRequiresIdleGpu_ && gpu_.Pending() && !gpu_.Submitted()) return false;
    gpu_.PresentDisplay();
    presentRequested_=false;
    return true;
}
F::State Session::State()const{return task_?task_->GetState():F::State::Ready;}
F::Request Session::Pending()const{if(!task_)throw std::logic_error("No S2 task");return task_->Pending();}
void Session::RethrowFailure()const{output_.RethrowFailure();if(task_)task_->RethrowFailure();}
void Session::Cancel(){if(task_&&task_->GetState()==F::State::Waiting)task_->Cancel();}
int32_t Session::CallLoadingDevice(uint32_t fn,Args args){
    int32_t result;
    if(PrStage2LoadingPacketsDirect::TryCall(*this,fn,args,result))return result;
    if(fn==0x80026E2Cu&&args.size()==0u)return PrStage2RetryDirect::PadVsync80026E2C(*this);
    const auto a=args.begin();
    if(fn==0x8001EA00u&&args.size()==1u)return PrStage2RetryDirect::EndFrame8001EA00(*this,a[0]);
    if(fn==0x80035560u&&args.size()==1u)return PrStage2FrameWait::VSync80035560(*this,*this,Signed(a[0]));
    if(fn==0x800381F8u&&args.size()==2u)return disc_.Lookup(*this,a[0],a[1]);
    if(fn==0x80038FC0u&&args.size()==3u)return disc_.StartRead(*this,a[0],a[1],a[2]);
    if(fn==0x800390C8u&&args.size()==2u&&a[0]==1u&&a[1]==0u)return disc_.PollRead();
    if(fn==0x8001B1B0u&&args.size()==3u)return PrStage2GpuDirect::ClearFramebuffer8001B1B0(*this,a[0],a[1],a[2]);
    if(fn==0x8003B70Cu&&args.size()==2u)return Ratan2Psx(Signed(a[0]),Signed(a[1]));
    // 8001E2E4 updates the live numeric status HUD. Keep the translated draw
    // path on the same OT/packet owner as the other S2 loading primitives so
    // the game loop does not stop at an empty host-side stub.
    if(fn==0x8001E2E4u&&args.size()==2u)
        return PrStage2GameHudDirect::DrawStatus8001E2E4(*this,a[0],Signed(a[1]));
    if(fn==0x80024744u&&args.size()==1u)return DrawCompactRail80024744(a[0]);
    if(fn==0x80047FBCu&&args.size()==3u){
        // BIOS memcpy data service: copy actual session bytes and return the
        // destination. No fabricated completion flag or stack address.
        if(a[2]>PrStage2MemoryServices::Services::RamBytes)throw std::out_of_range("S2 memcpy size");
        std::vector<uint8_t> bytes;bytes.reserve(a[2]);
        for(uint32_t n=0;n<a[2];++n)bytes.push_back(Read8(a[1]+n));
        for(uint32_t n=0;n<a[2];++n)Write8(a[0]+n,bytes[n]);
        return Signed(a[0]);
    }
    if(fn==0x80048A10u&&args.size()==0u){
        if(!irqActive_)throw std::logic_error("RFE outside an active native IRQ continuation");
        throw IrqReturn{};
    }
    return PlatformDependency(fn,args);
}

int32_t Session::DrawCompactRail80024744(uint32_t work) {
    ++compactRailCalls80024744_;
    return PrStage2GameHudDirect::DrawRail80024744(*this,work);
}
void Session::DrawRailNote80024418(uint16_t x,uint16_t y,uint16_t slot,uint16_t type) {
    PrStage2GameHudDirect::DrawNote80024418(*this,x,y,slot,type);
    ++railNotesSubmitted80024418_;
}
void Session::CallLoadingDeviceVoid(uint32_t fn,Args args){
    if(fn==0x80025C8Cu){
        if(args.size()!=1u)throw std::invalid_argument("InputSound argument count mismatch");
        PrStage2RetryDirect::InputSound80025C8C(*this,*args.begin());
        return;
    }
    (void)CallLoadingDevice(fn,args);
}
uint8_t Session::ReadDevice8(uint32_t a){throw Unbound(a,1,false);}
uint16_t Session::ReadDevice16(uint32_t a){
    uint32_t value;
    if(irq_.TryRead(a,2u,value)||spu_.TryRead(a,2u,value))return uint16_t(value);
    throw Unbound(a,2,false);
}
uint32_t Session::ReadDevice32(uint32_t a){
    uint32_t value;const uint32_t physical=a&0x1FFFFFFFu;
    if(irq_.TryRead(a,4u,value))return value;
    if(otc_.TryRead(a,4u,value)||gpu_.TryRead(a,4u,value))return value;
    // Status queries are observations, including SyncQueue(mode!=0) and
    // DrainQueue's early busy return. Only source wait loops may suspend.
    if(physical==0x1F801110u){
        if(timerMode_!=0x100u&&timerMode_!=0x107u)throw Unbound(a,4,false);
        const uint64_t now=Scanlines();
        return now<timerBase_?0u:uint32_t((now-timerBase_)&0xFFFFu);
    }
    throw Unbound(a,4,false);
}
void Session::WriteDevice8(uint32_t a,uint8_t){throw Unbound(a,1,true);}
void Session::WriteDevice16(uint32_t a,uint16_t value){
    if(irq_.TryWrite(a,2u,value)||spu_.TryWrite(a,2u,value))return;
    throw Unbound(a,2,true);
}
void Session::WriteDevice32(uint32_t a,uint32_t value){
    if(irq_.TryWrite(a,4u,value)||otc_.TryWrite(a,4u,value))return;
    if(gpu_.TryWrite(a,4u,value)){
        // A DMA start write is a real producer event. Consume source RAM now
        // while the original direct packet still belongs to this transfer;
        // later calls can reuse the shared draw-environment packet. D3D fence
        // completion and IRQ publication still occur only on later Polls.
        if((a&0x1FFFFFFFu)==0x1F8010A8u&&(value&0x01000000u))gpu_.Service();
        return;
    }
    if((a&0x1FFFFFFFu)==0x1F801114u&&(value==0x100u||value==0x107u)){
        timerMode_=value;const uint64_t now=Scanlines();timerBase_=now;
        // Original 80035E54 writes 107h: Timer1 waits for one VBlank edge,
        // then counts HBlank pulses freely. A mode write also resets count.
        if(value==0x107u)timerBase_=(now/263u)*263u+240u+((now%263u)>=240u?263u:0u);
        return;
    }
    throw Unbound(a,4,true);
}
// 游戏时钟保留原 v0/v1 返回；未知双字入口继续报告未绑定。
PrStage2LifecycleDirect::Words64 Session::Call64(uint32_t f,Args args){
    PrStage2LifecycleDirect::Words64 result;
    if(PrStage2SoftFloatDirect::TryCall64(*this,f,args,result))return result;
    throw Unbound(f);
}
PrStage2LifecycleDirect::Vector32 Session::NormalizeVector8003A3DC(PrStage2LifecycleDirect::Vector32 input){
    const int64_t x=Signed(input.x),y=Signed(input.y),z=Signed(input.z);
    const uint64_t length=IntegerSqrt(uint64_t(x*x)+uint64_t(y*y)+uint64_t(z*z));
    if(length==0u)return {0u,0u,0u};
    const auto scale=[&](int64_t value){return uint32_t((value*4096)/int64_t(length));};
    return {scale(x),scale(y),scale(z)};
}
int32_t Session::SetCdLocation800367A4(uint32_t location){return disc_.SetLocation(location);}
int32_t Session::LoadImage80044D64(PrStage2LifecycleDirect::ImageRect rect,uint32_t source){return vram_.UploadImageWords(*this,rect,source);}
[[noreturn]] void Session::Exit(uint32_t function,Args){throw Unbound(function);}
[[noreturn]] void Session::Break(uint32_t function,uint32_t){throw Unbound(function);}
}
