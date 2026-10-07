#include "pr/pr_stage2_frame_task.h"
#include "pr/pr_stage2_frame_wait.h"
#include "pr/pr_stage2_scene_entry.h"
#include "pr/pr_stage2_disc_file_device.h"
#include "pr/pr_stage2_vram_device.h"
#include "pr/pr_stage2_interrupt_device.h"
#include "test_stage2_memory_services_contract.h"
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#include <array>
#include <fstream>
#include <iterator>
#include <thread>

namespace {
namespace F = PrStage2FrameTask;
namespace W = PrStage2FrameWait;
namespace E = PrStage2SceneEntry;
uint32_t checks = 0;
void Check(bool yes, const char* why) { ++checks; if (!yes) throw std::runtime_error(why); }
template<class Fn> void Reject(Fn fn) {
    bool failed = false; try { fn(); } catch (const std::exception&) { failed = true; }
    Check(failed, "invalid operation accepted");
}
struct Guard {
    std::vector<int>& destroyed; int id;
    ~Guard() { destroyed.push_back(id); }
};
void TaskChecks() {
    Reject([] { F::Task task(F::Task::Entry{}); });
    std::vector<int> destroyed;
    int entered = 0, nested = 0;
    const DWORD thread = GetCurrentThreadId();
    F::Task* owner = nullptr;
    F::Task task([&] {
        Guard guard{destroyed, 1}; std::array<uint32_t, 128> local{};
        for (size_t i=0; i<local.size(); ++i) local[i] = uint32_t(i ^ 0xDEADBEEFu);
        ++entered; owner->Suspend(F::WaitKind::VBlankCounter, 0x80035560u, 2u);
        for (size_t i=0; i<local.size(); ++i) Check(local[i] == uint32_t(i ^ 0xDEADBEEFu), "lost local stack");
        Check(GetCurrentThreadId()==thread && nested==2, "task moved thread or skipped nested event");
        return -73;
    }); owner=&task;
    Reject([&] { task.Result(); }); Reject([&] { task.Pending(); });
    Reject([&] { task.Suspend(F::WaitKind::VBlankCounter, 0, 0); });
    Check(task.Start()==F::State::Waiting && entered==1 && destroyed.empty(), "start did not retain stack");
    const auto first=task.Pending(); Reject([&] { task.Start(); });
    Reject([&] { task.Resume(first.ticket+1u); });
    bool wrongThread=false;
    std::thread other([&] { try { task.Resume(first.ticket); } catch (const std::logic_error&) { wrongThread=true; } });
    other.join(); Check(wrongThread && task.GetState()==F::State::Waiting, "wrong-thread resume had effects");
    Check(task.Resume(first.ticket, [&] {
        Guard eventGuard{destroyed, 2}; ++nested;
        task.Suspend(F::WaitKind::PlatformDependency, 0x8001EA74u, 0);
        ++nested;
    })==F::State::Waiting, "nested callback cannot suspend");
    const auto second=task.Pending(); Check(second.ticket!=first.ticket && destroyed.empty(), "nested stack lost");
    Reject([&] { task.Resume(first.ticket); });
    Check(task.Resume(second.ticket)==F::State::Completed && task.Result()==-73, "nested task did not finish");
    Check(destroyed==std::vector<int>{2,1} && entered==1 && !IsThreadAFiber(), "cleanup or caller conversion leaked");
    Reject([&] { task.Resume(second.ticket); });
    destroyed.clear();
    {
        F::Task* ptr=nullptr;
        F::Task cancelled([&] { Guard g{destroyed,3};ptr->Suspend(F::WaitKind::VBlankCounter,1,1);return 99; });ptr=&cancelled;
        Check(cancelled.Start()==F::State::Waiting, "cancel test start");
        cancelled.Resume(cancelled.Pending().ticket,[&] {Guard g{destroyed,4};ptr->Suspend(F::WaitKind::GpuField,2,2);});
        Check(cancelled.Cancel()==F::State::Cancelled, "cancel failed");
        Check(destroyed==std::vector<int>{4,3}, "cancel skipped destructors");
        Reject([&] { cancelled.Result(); });
    }
    destroyed.clear();
    {
        F::Task* ptr=nullptr;
        F::Task automatic([&] {Guard g{destroyed,5};ptr->Suspend(F::WaitKind::VBlankCounter,0,0);return 0;});ptr=&automatic;
        automatic.Start();
    }
    Check(destroyed==std::vector<int>{5}, "destructor dropped live stack");
    F::Task early([&] {++entered;return 0;});Check(early.Cancel()==F::State::Cancelled&&entered==1,"ready cancel executed entry");
    {
        F::Task* ptr=nullptr;
        F::Task error([&] {ptr->Suspend(F::WaitKind::VBlankCounter,0,0);return 0;});ptr=&error;error.Start();
        Check(error.Resume(error.Pending().ticket,[] {throw std::runtime_error("real event failure");})==F::State::Failed,"event error swallowed");
        bool matched=false;try{error.RethrowFailure();}catch(const std::runtime_error& e){matched=std::string(e.what())=="real event failure";}
        Check(matched,"exception identity lost");
    }
    // Existing caller fiber conversion belongs to its owner, not this task.
    void* root=ConvertThreadToFiberEx(nullptr,FIBER_FLAG_FLOAT_SWITCH);Check(root!=nullptr,"caller fiber conversion");
    {F::Task single([]{return 7;});Check(single.Start()==F::State::Completed&&single.Result()==7,"preconverted caller");}
    Check(IsThreadAFiber()&&GetCurrentFiber()==root,"task reverted someone else's fiber");Check(ConvertFiberToThread()!=0,"restore caller thread");
    F::Task* pa=nullptr;F::Task* pb=nullptr;
    F::Task a([&]{pa->Suspend(F::WaitKind::VBlankCounter,1,1);return 11;});pa=&a;
    F::Task b([&]{pb->Suspend(F::WaitKind::VBlankCounter,2,2);return 22;});pb=&b;
    a.Start();b.Start();b.Resume(b.Pending().ticket);a.Resume(a.Pending().ticket);
    Check(a.Result()==11&&b.Result()==22&&!IsThreadAFiber(),"interleaved task ownership");
    uint32_t visits=0;F::Task* stressPtr=nullptr;
    F::Task stress([&]{for(uint32_t i=0;i<256u;++i){++visits;stressPtr->Suspend(F::WaitKind::VBlankCounter,3,i);}return 256;});stressPtr=&stress;
    stress.Start();uint64_t previous=0;
    while(stress.GetState()==F::State::Waiting){auto q=stress.Pending();Check(q.ticket>previous,"ticket not monotonic");previous=q.ticket;stress.Resume(q.ticket);}
    Check(visits==256&&stress.Result()==256,"repeated suspension restarted body");
    std::cout<<"frame-task-contract passed 256-resumes\n";
}
struct Clock : MemoryContract::Probe {
    uint32_t gpu=0, timer=0x12345u, alternateTimer=0x76543u, alternateGpu=0x80080000u;
    std::vector<uint32_t> reads;
    Clock() {
        Write32(0x80055F68u,0x1F801814u);Write32(0x80055F6Cu,0x1F801110u);
        Write32(0x80055F70u,0xFFF0u);Write32(0x80055F74u,0u);Write32(0x80057034u,0u);
    }
    uint32_t ReadDevice32(uint32_t a) override {
        reads.push_back(a);
        if(a==0x1F801814u)return gpu;if(a==0x1F801110u)return timer;
        if(a==0x1F801120u)return alternateTimer;if(a==0x1F801818u)return alternateGpu;
        throw std::runtime_error("unexpected clock MMIO");
    }
};
void ClockChecks() {
    for(int32_t mode:{-1,(-2147483647-1),1}) {
        Clock s; s.Write32(0x80057034u,0x87654321u);
        F::Task* taskPtr=nullptr;F::Task task([&]{W::TaskProgress p(*taskPtr);return W::VSync80035560(s,p,mode);});taskPtr=&task;
        Check(task.Start()==F::State::Completed,"query yielded");
        Check(uint32_t(task.Result())==(mode<0?0x87654321u:0x2355u),"query result");
        Check(s.reads==std::vector<uint32_t>{0x1F801814u,0x1F801110u},"query skipped/reordered original device reads");
        Check(s.Read32(0x80055F70u)==0xFFF0u&&s.Read32(0x80055F74u)==0u,"query changed committed state");
    }
    for(int32_t mode:{0,2,3}) {
        Clock s;F::Task* taskPtr=nullptr;F::Task task([&]{W::TaskProgress p(*taskPtr);return W::VSync80035560(s,p,mode);});taskPtr=&task;
        Check(task.Start()==F::State::Waiting,"wait returned before event");
        auto first=task.Pending();Check(task.Resume(first.ticket)==F::State::Waiting,"no-event resume completed wait");
        Check(s.Read32(0x80057034u)==0u&&s.Read32(0x80055F74u)==0u,"wait manufactured counter");
        uint32_t frames=0;
        while(task.GetState()==F::State::Waiting) {
            Check(++frames<8,"frame wait did not finish");
            auto q=task.Pending();task.Resume(q.ticket,[&]{s.Write32(0x80057034u,frames);++s.timer;});
        }
        task.RethrowFailure();Check(frames==uint32_t(mode==0?1:mode)&&task.Result()==0x2355,"two-target frame count/result");
        Check(s.Read32(0x80055F74u)==frames&&s.Read32(0x80055F70u)==s.timer,"wait commit missing");
    }
    {
        Clock s;s.gpu=0x80000u;F::Task* ptr=nullptr;
        F::Task task([&]{W::TaskProgress p(*ptr);return W::VSync80035560(s,p,0);});ptr=&task;task.Start();
        task.Resume(task.Pending().ticket,[&]{s.Write32(0x80057034u,1u);s.Write32(0x80055F6Cu,0x1F801120u);});
        Check(task.GetState()==F::State::Waiting&&task.Pending().kind==F::WaitKind::GpuField,"interlace field gate missing");
        Check(s.Read32(0x80055F70u)==0xFFF0u,"committed before field change");
        task.Resume(task.Pending().ticket,[&]{s.Write32(0x80055F68u,0x1F801818u);});
        Check(task.GetState()==F::State::Waiting,"field poll reloaded cached address");
        task.Resume(task.Pending().ticket,[&]{s.gpu^=0x80000000u;});task.RethrowFailure();
        Check(task.GetState()==F::State::Completed&&s.Read32(0x80055F70u)==s.alternateTimer,"final timer pointer not reloaded");
    }
    {
        Clock s;s.Write32(0x80055F74u,0x7FFFFFFFu);s.Write32(0x80057034u,0x7FFFFFFFu);
        F::Task* ptr=nullptr;F::Task task([&]{W::TaskProgress p(*ptr);return W::VSync80035560(s,p,2);});ptr=&task;
        Check(task.Start()==F::State::Completed,"signed wrap target changed to unsigned wait");
    }
    std::cout<<"frame-wait-contract passed queries-two-targets-field-cancel\n";
}
struct Missing : std::runtime_error {
    uint32_t function;
    Missing(uint32_t fn):std::runtime_error("unbound actual S2 dependency"),function(fn){}
};
struct IrqReturn {};
struct Scene final : E::Services, W::Progress {
    PrStage2DiscFileDevice::Device disc;
    PrStage2VramDevice::Device vram;
    PrStage2InterruptDevice::Controller irq;
    PrPsxGteDirect::MatrixRegisters gte{};
    F::Task* task=nullptr;
    bool boot=false, isolatedCallbacks=false;
    uint32_t timer=0, gpu=0x04000000u, ticks=0, waited=0, prepares=0, draws=0, audio=0;
    std::vector<std::array<uint32_t,3>> reads;
    Scene(const char* scus,const char* image,const char* bin):E::Services(std::filesystem::u8path(scus),std::filesystem::u8path(image)),disc(std::filesystem::u8path(bin)){}
    void Await(F::WaitKind kind,uint32_t target) override {
        Check(task!=nullptr,"frame task not bound");task->Suspend(kind,0x80035560u,target);
    }
    int32_t CallPlatform(uint32_t fn,E::Arguments args) override {
        const std::vector<uint32_t> a(args);
        if(boot) {
            if(fn==0x80047F5Cu&&a==std::vector<uint32_t>{0x80055FB0u})return 0;
            if(fn==0x80048A30u&&a==std::vector<uint32_t>{0x80055FB0u})return -73;
            if(fn==0x80048AE0u&&a==std::vector<uint32_t>{0})return -77;
            if(fn==0x80048AF0u&&a==std::vector<uint32_t>{3,0})return -11;
            if(fn==0x80048960u&&a.empty())return -99;
            if(fn==0x80048A50u&&a.empty())return -17;
            throw Missing(fn);
        }
        if(fn==0x80035560u&&a.size()==1u){++waited;return W::VSync80035560(*this,*this,int32_t(a[0]));}
        if(fn==0x800381F8u&&a.size()==2u)return disc.Lookup(*this,a[0],a[1]);
        if(fn==0x80038FC0u&&a.size()==3u){reads.push_back({a[0],a[1],a[2]});return disc.StartRead(*this,a[0],a[1],a[2]);}
        if(fn==0x800390C8u&&a==std::vector<uint32_t>{1,0})return disc.PollRead();
        if(fn==0x80048A10u&&a.empty())throw IrqReturn{};
        // Explicitly isolated rendering/audio boundaries ONLY in the second
        // test. Never counted as actual Loading pixels or product devices.
        if(isolatedCallbacks) {
            if(fn==0x8001EA74u&&a==std::vector<uint32_t>{1,0}){++prepares;return 0;}
            if(fn==0x80026ECCu&&a.empty()){++audio;return 0;}
            if(fn==0x8001EBF4u&&a==std::vector<uint32_t>{0}){++draws;return 0;}
        }
        throw Missing(fn);
    }
    void CallPlatformVoid(uint32_t fn,E::Arguments) override {throw Missing(fn);}
    uint8_t ReadDevice8(uint32_t a) override {throw Missing(a);}
    uint16_t ReadDevice16(uint32_t a) override {uint32_t v;if(irq.TryRead(a,2,v))return uint16_t(v);throw Missing(a);}
    uint32_t ReadDevice32(uint32_t a) override {
        uint32_t v;if(irq.TryRead(a,4,v))return v;
        if(a==0x1F801814u)return gpu;if(a==0x1F801110u)return timer;
        throw Missing(a);
    }
    void WriteDevice8(uint32_t a,uint8_t) override {throw Missing(a);}
    void WriteDevice16(uint32_t a,uint16_t v) override {if(!irq.TryWrite(a,2,v))throw Missing(a);}
    void WriteDevice32(uint32_t a,uint32_t v) override {if(irq.TryWrite(a,4,v))return;if(boot&&a==0x1F801114u)return;throw Missing(a);}
    void BootFixture() {
        boot=true;try {Check(uint32_t(Call(0x80035744u,{}))==0x80055F78u,"boot fixture initialization");}
        catch(...){boot=false;throw;}boot=false;
    }
    void Tick() {
        ++ticks;timer+=263u; // Explicit deterministic test timer, not wall clock.
        irq.SetLine(PrStage2InterruptDevice::Source::VBlank,true);
        irq.SetLine(PrStage2InterruptDevice::Source::VBlank,false);
        if(irq.Pending()) {bool returned=false;try{Call(0x800359B8u,{});}catch(const IrqReturn&){returned=true;}Check(returned,"native IRQ return missing");}
    }
    PrStage2LifecycleDirect::Words64 Call64(uint32_t f,E::Arguments) override {throw Missing(f);}
    PrStage2LifecycleDirect::Vector32 NormalizeVector8003A3DC(PrStage2LifecycleDirect::Vector32) override {throw Missing(0x8003A3DCu);}
    int32_t SetCdLocation800367A4(uint32_t loc) override {return disc.SetLocation(loc);}
    int32_t LoadImage80044D64(PrStage2LifecycleDirect::ImageRect rect,uint32_t source) override {return vram.UploadImageWords(*this,rect,source);}
    PrPsxGteDirect::MatrixRegisters& MatrixGte() override {return gte;}
    [[noreturn]]void Exit(uint32_t f,E::Arguments) override {throw Missing(f);}
    [[noreturn]]void Break(uint32_t f,uint32_t) override {throw Missing(f);}
};
uint32_t Failure(F::Task& task) {
    try {task.RethrowFailure();}catch(const Missing& e){return e.function;}
    return 0;
}
void SceneChecks(const char* scus,const char* overlay,const char* bin) {
    {
        Scene s(scus,overlay,bin);s.BootFixture();s.InitializeGlobals();
        uint32_t starts=0;F::Task task([&]{++starts;return s.InitializeScene();});s.task=&task;
        Check(task.Start()==F::State::Waiting&&s.GetPhase()==E::Phase::SceneInitializing,"S2 wait lost exclusive initializer ownership");
        Reject([&]{s.InitializeScene();});Reject([&]{s.InitializeGlobals();});Reject([&]{s.RunScene();});
        Check(s.GetPhase()==E::Phase::SceneInitializing,"rejected reentry changed live phase");
        Check(s.disc.ReadRequests()==1u&&s.disc.BytesTransferred()==8192u&&s.disc.LookupRequests()==7u,"first real S2 read");
        for(int i=0;i<3;++i)Check(task.Resume(task.Pending().ticket)==F::State::Waiting,"no event bypassed VSync");
        Check(starts==1&&s.disc.ReadRequests()==1u&&s.disc.LookupRequests()==7u,"resume restarted S2 initialization");
        Check(task.Resume(task.Pending().ticket,[&]{s.Tick();})==F::State::Failed,"unbound Loading renderer falsely completed");
        Check(Failure(task)==0x8001EA74u&&s.Read32(0x8006ECD4u)==0&&s.GetPhase()==E::Phase::Failed,"strict Loading boundary lost");
        std::cout<<"s2-strict-resume 1-start 7-lookups 8192-bytes blocked-8001ea74\n";
    }
    {
        Scene s(scus,overlay,bin);s.BootFixture();s.InitializeGlobals();
        F::Task task([&]{return s.InitializeScene();});s.task=&task;task.Start();
        Check(task.Cancel()==F::State::Cancelled&&s.GetPhase()==E::Phase::Failed,"S2 cancellation did not unwind");
        Reject([&]{s.RunScene();});Reject([&]{s.InitializeScene();});
        Check(s.disc.ReadRequests()==1u&&s.disc.BytesTransferred()==8192u,"cancel read more bytes");
        std::cout<<"s2-cancel-resume preserves-phase-gate\n";
    }
    {
        Scene s(scus,overlay,bin);s.BootFixture();s.InitializeGlobals();s.isolatedCallbacks=true;
        uint32_t starts=0;F::Task task([&]{++starts;return s.InitializeScene();});s.task=&task;task.Start();
        while(task.GetState()==F::State::Waiting) {
            Check(s.ticks<30u,"isolated source path did not reach next dependency");
            task.Resume(task.Pending().ticket,[&]{s.Tick();});
        }
        const uint32_t blocked=Failure(task);
        Check(task.GetState()==F::State::Failed&&blocked!=0,"incomplete S2 falsely completed");
        Check(starts==1&&s.disc.LookupRequests()==7u&&s.disc.ReadRequests()>=2u,"resumed source reread header or lost descriptor");
        Check(s.reads.size()>=2u&&s.reads[0][0]==4u&&s.reads[1][0]==291u,"original INT read sequence changed");
        const auto path=std::filesystem::u8path(overlay).parent_path()/"COMPO02.INT";
        std::ifstream input(path,std::ios::binary);std::vector<uint8_t> original((std::istreambuf_iterator<char>(input)),{});
        for(uint32_t i=0;i<291u*2048u;++i)Check(s.Read8(s.reads[1][1]+i)==original.at(8192u+i),"resumed private descriptor read wrong sectors");
        Check(s.prepares>0&&s.draws>0&&s.audio==s.prepares,"registered native Loading callback not executed");
        std::cout<<"s2-isolated-resume "<<starts<<' '<<s.disc.LookupRequests()<<' '<<s.disc.ReadRequests()<<' '<<s.disc.BytesTransferred()<<' '
                 <<s.ticks<<' '<<s.prepares<<' '<<s.draws<<' '<<s.vram.UploadCount()<<' '<<std::hex<<blocked<<std::dec<<" render-audio-boundaries-isolated\n";
    }
}
}
int main(int argc,char** argv) {
    try {
        TaskChecks();ClockChecks();
        if(argc==4)SceneChecks(argv[1],argv[2],argv[3]);else if(argc!=1)return 1;
        std::cout<<"frame-resume-pass "<<checks<<" assertions\n";return 0;
    }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 2;}
}
