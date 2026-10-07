#include "pr/pr_stage2_loading_work_direct.h"
#include "pr/pr_stage2_loading_session.h"
#include "pr/pr_stage2_otc_device.h"
#include "pr/pr_stage2_frame_task.h"
#include "pr/pr_stage2_frame_wait.h"
#include "pr/pr_stage2_disc_file_device.h"
#include "pr/pr_stage2_vram_device.h"
#include <array>
#include <functional>
#include <iostream>
#include <map>
#include <vector>

namespace {
namespace L = PrStage2LifecycleDirect;
namespace F = PrStage2FrameTask;
namespace W = PrStage2FrameWait;
using Args = std::initializer_list<uint32_t>;
uint64_t checks = 0;
void Check(bool ok, const char* error) { ++checks; if (!ok) throw std::runtime_error(error); }
template<class Fn> void Reject(Fn&& fn) {
    bool rejected = false; try { fn(); } catch (const std::exception&) { rejected = true; }
    Check(rejected, "Expected failure was not raised");
}
struct Missing : std::runtime_error {
    uint32_t address;
    explicit Missing(uint32_t a) : std::runtime_error("Unbound next Stage2 device"), address(a) {}
};
struct IrqReturn {};
struct TraceMemory final : L::Services {
    std::map<uint32_t, uint32_t> words;
    std::vector<std::array<uint32_t, 2>> writes;
    PrPsxGteDirect::MatrixRegisters gte{};
    size_t failAt = size_t(-1);
    std::function<void()> beforeWrite;
    uint8_t Read8(uint32_t a) override { return uint8_t(Read32(a & ~3u) >> ((a & 3u)*8u)); }
    uint16_t Read16(uint32_t a) override { return uint16_t(Read8(a) | (uint16_t(Read8(a+1u))<<8u)); }
    uint32_t Read32(uint32_t a) override { return words.at(a); }
    void Write8(uint32_t, uint8_t) override { throw std::logic_error("Unexpected byte write"); }
    void Write16(uint32_t, uint16_t) override { throw std::logic_error("Unexpected halfword write"); }
    void Write32(uint32_t a, uint32_t v) override {
        if (beforeWrite) beforeWrite();
        if (writes.size() == failAt) throw std::runtime_error("Injected RAM write failure");
        writes.push_back({a,v}); words[a] = v;
    }
    int32_t Call(uint32_t f, Args) override { throw Missing(f); }
    L::Words64 Call64(uint32_t f, Args) override { throw Missing(f); }
    L::Vector32 NormalizeVector8003A3DC(L::Vector32) override { throw Missing(0x8003A3DCu); }
    int32_t SetCdLocation800367A4(uint32_t) override { throw std::logic_error("Unexpected CD call"); }
    int32_t LoadImage80044D64(L::ImageRect, uint32_t) override { throw std::logic_error("Unexpected VRAM call"); }
    PrPsxGteDirect::MatrixRegisters& MatrixGte() override { return gte; }
    [[noreturn]] void Exit(uint32_t f, Args) override { throw Missing(f); }
    [[noreturn]] void Break(uint32_t f, uint32_t) override { throw Missing(f); }
};
uint32_t Read(PrStage2OtcDevice::Device& d, uint32_t a) {
    uint32_t v=0; Check(d.TryRead(a,4u,v),"OTC address not recognized"); return v;
}
void Write(PrStage2OtcDevice::Device& d,uint32_t a,uint32_t v) {
    Check(d.TryWrite(a,4u,v),"OTC address not recognized");
}
void Configure(PrStage2OtcDevice::Device& d,uint32_t last,uint32_t count) {
    Write(d,0x1F8010E0u,last); Write(d,0x1F8010E4u,count); Write(d,0x1F8010E8u,0x11000002u);
}
void WorkChecks() {
    TraceMemory m;
    for(uint32_t p=0x80087288u;p<0x80087330u;p+=4u)m.words[p]=0xA5A5A5A5u;
    PrStage2LoadingWorkDirect::Initialize8001E6D0(m);
    const std::vector<std::array<uint32_t,2>> expected={
        {0x8008728Cu,0x800872B0u},{0x80087288u,4u},{0x80087290u,0u},
        {0x800872A0u,0x800872F0u},{0x8008729Cu,4u},{0x800872A4u,0u},
        {0x8006ED50u,0x80080CF8u},{0x8006ED54u,0x80083FC0u}};
    Check(m.writes==expected,"Shared work write order differs");
    for(uint32_t p=0x80087288u;p<0x80087330u;p+=4u) {
        bool written=false; for(auto w:expected)if(w[0]==p)written=true;
        if(!written)Check(m.words.at(p)==0xA5A5A5A5u,"Shared initializer cleared unowned data");
    }
    m.writes.clear(); PrStage2LoadingWorkDirect::RestorePacketBanks8001E34C(m);
    Check(m.writes==std::vector<std::array<uint32_t,2>>(expected.end()-2,expected.end()),"Bank restore changed OT");
    std::cout<<"loading-work-contract 8-ordered-writes preserved-ot-contents\n";
}
void OtcChecks() {
    using Device=PrStage2OtcDevice::Device;
    using Controller=PrStage2InterruptDevice::Controller;
    for(uint32_t words:{1u,2u,16u,1024u,65535u,65536u})for(uint32_t alias:{0u,0x80000000u,0xA0000000u}) {
        Device d; Controller irq; TraceMemory m;
        const uint32_t last=0x100000u+4u*(words-1u), count=words&0xFFFFu;
        Configure(d,last,count);
        const auto serial=d.Serial();
        uint32_t flags=0;
        for(uint32_t i=0;i<8u;++i)Check((Read(d,0x1F8010E8u|alias)&0x11000000u)==0x11000000u,"Read progressed OTC");
        Check(m.writes.empty()&&d.CompletedTransfers()==0u,"Submission performed memory transfer");
        Check(!d.Complete(serial,m,irq)&&m.writes.empty(),"Disabled channel transferred");
        irq.TryWrite(0x1F8010F0u,4u,0x08000000u);
        irq.TryWrite(0x1F8010F4u,4u,0x00C00000u);
        irq.TryWrite(0x1F801074u,2u,8u);
        m.beforeWrite=[&]{
            if(m.writes.empty()||m.writes.size()+1u==words){
                Check((Read(d,0x1F8010E8u)&0x11000000u)==0x01000000u,"Trigger/busy ordering before RAM write");
                Check(d.CompletedTransfers()==0u&&irq.Pending()==0u,"Completion preceded final RAM write");
            }
        };
        Check(d.Complete(serial,m,irq),"Enabled OTC did not transfer");
        Check(m.writes.size()==words&&d.WrittenWords()==words&&d.CompletedTransfers()==1u,"OTC transfer count");
        for(uint32_t i=0;i<words;++i)
            Check(m.writes[i]==std::array<uint32_t,2>{0x80000000u+last-4u*i,i+1u==words?0xFFFFFFu:last-4u*i-4u},"OTC link/write order");
        Check(Read(d,0x1F8010E0u)==last&&Read(d,0x1F8010E4u)==count&&Read(d,0x1F8010E8u)==2u,"Sync0 descriptor or completion bits");
        irq.TryRead(0x1F8010F4u,4u,flags);
        Check((flags&0xC0000000u)==0xC0000000u&&irq.Pending()==8u,"DMA6 completion not notified");
        Reject([&]{d.Complete(serial,m,irq);});
    }
    {
        Device d; uint32_t unchanged=0x12345678u;
        Check(!d.TryRead(0x1F8010A8u,4u,unchanged)&&unchanged==0x12345678u,"Unknown read modified output");
        Check(!d.TryWrite(0x1F8010A8u,4u,1u),"OTC claimed GPU channel");
        for(uint32_t offset=0;offset<12u;++offset)for(uint32_t width:{1u,2u,4u,8u}) {
            if(!(offset&3u)&&width==4u)continue;
            Reject([&]{d.TryRead(0x1F8010E0u+offset,width,unchanged);});
            Reject([&]{d.TryWrite(0x1F8010E0u+offset,width,0u);});
        }
        Write(d,0x1F8010E8u,0x40000000u); Check(Read(d,0x1F8010E8u)==0x40000002u,"OTC fixed bit/mask");
        Write(d,0x1F8010E0u,0xFF000100u); Check(Read(d,0x1F8010E0u)==0x100u,"MADR mask");
    }
    for(auto descriptor:std::vector<std::array<uint32_t,2>>{{0x100001u,16u},{0x200000u,16u},{0u,16u}}) {
        Device d; Controller irq; TraceMemory m; irq.TryWrite(0x1F8010F0u,4u,0x08000000u);
        Configure(d,descriptor[0],descriptor[1]); Reject([&]{d.Complete(d.Serial(),m,irq);});
        Check(m.writes.empty()&&d.CompletedTransfers()==0u&&d.Pending(),"Invalid descriptor was acknowledged");
    }
    {
        Device d; Controller irq; TraceMemory m; irq.TryWrite(0x1F8010F0u,4u,0x08000000u);
        Configure(d,0x10003Cu,16u); const auto old=d.Serial();
        Reject([&]{Write(d,0x1F8010E0u,0u);});
        Write(d,0x1F8010E8u,0u); Reject([&]{d.Complete(old,m,irq);});
        Configure(d,0x10003Cu,16u); Reject([&]{d.Complete(old,m,irq);});
        m.failAt=3u; Reject([&]{d.Complete(d.Serial(),m,irq);});
        Check(d.Faulted()&&m.writes.size()==3u&&d.CompletedTransfers()==0u&&(Read(d,0x1F8010E8u)&0x01000000u),"Partial failure falsely completed");
        Reject([&]{d.Complete(d.Serial(),m,irq);});
        Write(d,0x1F8010E8u,0u); Check(!d.Faulted(),"Explicit reset retained fault");
    }
    std::cout<<"otc-contract completed 18-size-alias-transfers no-read-pumping failure-gates\n";
}
struct WorkSession final : PrStage2LoadingSession::Services, W::Progress {
    PrStage2DiscFileDevice::Device disc;
    PrStage2VramDevice::Device vram;
    PrStage2InterruptDevice::Controller irq;
    PrStage2OtcDevice::Device otc;
    PrPsxGteDirect::MatrixRegisters gte{};
    F::Task* task=nullptr;
    bool boot=false;
    uint32_t ticks=0,timer=0;
    std::vector<uint32_t> external;
    WorkSession(const char* scus,const char* overlay,const char* bin):Services(std::filesystem::u8path(scus),std::filesystem::u8path(overlay)),disc(std::filesystem::u8path(bin)){}
    void Await(F::WaitKind kind,uint32_t target)override{Check(task!=nullptr,"Missing retained task");task->Suspend(kind,0x80035560u,target);}
    int32_t CallLoadingDevice(uint32_t fn,Args args)override{
        external.push_back(fn);const std::vector<uint32_t>a(args);
        if(boot){
            if(fn==0x80047F5Cu&&a==std::vector<uint32_t>{0x80055FB0u})return 0;
            if(fn==0x80048A30u&&a==std::vector<uint32_t>{0x80055FB0u})return -73;
            if(fn==0x80048AE0u&&a==std::vector<uint32_t>{0u})return -77;
            if(fn==0x80048AF0u&&a==std::vector<uint32_t>{3u,0u})return -11;
            if(fn==0x80048960u&&a.empty())return -99;
            if(fn==0x80048A50u&&a.empty())return -17;
            throw Missing(fn);
        }
        if(fn==0x80035560u&&a.size()==1u)return W::VSync80035560(*this,*this,static_cast<int32_t>(a[0]));
        if(fn==0x800381F8u&&a.size()==2u)return disc.Lookup(*this,a[0],a[1]);
        if(fn==0x80038FC0u&&a.size()==3u)return disc.StartRead(*this,a[0],a[1],a[2]);
        if(fn==0x800390C8u&&a==std::vector<uint32_t>{1u,0u})return disc.PollRead();
        if(fn==0x80048A10u&&a.empty())throw IrqReturn{};
        throw Missing(fn);
    }
    void CallLoadingDeviceVoid(uint32_t fn,Args)override{throw Missing(fn);}
    uint8_t ReadDevice8(uint32_t a)override{throw Missing(a);}
    uint16_t ReadDevice16(uint32_t a)override{uint32_t v;if(irq.TryRead(a,2u,v))return uint16_t(v);throw Missing(a);}
    uint32_t ReadDevice32(uint32_t a)override{
        uint32_t v;if(irq.TryRead(a,4u,v))return v;
        if(otc.TryRead(a,4u,v)){
            while(a==0x1F8010E8u&&(v&0x01000000u)){
                Check(task!=nullptr,"No OTC scheduling owner");
                task->Suspend(F::WaitKind::PlatformDependency,a,uint32_t(otc.Serial()));
                otc.TryRead(a,4u,v);
            }
            return v;
        }
        if(a==0x1F801814u)return 0x04000000u; // Explicit test GPU status, not product GPU.
        if(a==0x1F801110u)return timer;
        throw Missing(a);
    }
    void WriteDevice8(uint32_t a,uint8_t)override{throw Missing(a);}
    void WriteDevice16(uint32_t a,uint16_t v)override{if(!irq.TryWrite(a,2u,v))throw Missing(a);}
    void WriteDevice32(uint32_t a,uint32_t v)override{if(irq.TryWrite(a,4u,v)||otc.TryWrite(a,4u,v))return;if(boot&&a==0x1F801114u)return;throw Missing(a);}
    L::Words64 Call64(uint32_t f,Args)override{throw Missing(f);}
    L::Vector32 NormalizeVector8003A3DC(L::Vector32)override{throw Missing(0x8003A3DCu);}
    int32_t SetCdLocation800367A4(uint32_t loc)override{return disc.SetLocation(loc);}
    int32_t LoadImage80044D64(L::ImageRect rect,uint32_t source)override{return vram.UploadImageWords(*this,rect,source);}
    PrPsxGteDirect::MatrixRegisters& MatrixGte()override{return gte;}
    [[noreturn]]void Exit(uint32_t f,Args)override{throw Missing(f);}
    [[noreturn]]void Break(uint32_t f,uint32_t)override{throw Missing(f);}
    void BootFixture(){boot=true;try{Check(uint32_t(Call(0x80035744u,{}))==0x80055F78u,"Cold BIOS prerequisite");}catch(...){boot=false;throw;}boot=false;external.clear();}
    void Tick(){++ticks;timer+=263u;irq.SetLine(PrStage2InterruptDevice::Source::VBlank,true);irq.SetLine(PrStage2InterruptDevice::Source::VBlank,false);if(irq.Pending()){bool returned=false;try{Call(0x800359B8u,{});}catch(const IrqReturn&){returned=true;}Check(returned,"Missing IRQ return");}}
};
void SceneChecks(const char* scus,const char* overlay,const char* bin){
    WorkSession s(scus,overlay,bin);s.BootFixture();s.InitializeGlobals();
    Check(s.Read32(0x80087288u)==0u&&s.Read32(0x8008728Cu)==0u&&s.Read32(0x8006ED50u)==0u,"Cold prerequisite changed; inspect source");
    PrStage2LoadingWorkDirect::Initialize8001E6D0(s);
    const uint32_t buffer=s.Read16(0x80096590u),head=0x800872B0u+64u*buffer;
    for(uint32_t i=0;i<16u;++i)s.Write32(head+4u*i,0xA5A5A5A5u); // Detect premature/partial clear.
    const uint32_t lowBefore=s.Read32(0x80010000u);uint32_t starts=0;
    F::Task task([&]{++starts;return s.InitializeScene();});s.task=&task;
    Check(task.Start()==F::State::Waiting,"No first VSync suspension");
    Check(task.Resume(task.Pending().ticket,[&]{s.Tick();})==F::State::Waiting,"OTC did not retain callback stack");
    Check(task.Pending().function==0x1F8010E8u&&s.otc.Pending()&&s.otc.CompletedTransfers()==0u,"Wrong pending OTC boundary");
    Check(Read(s.otc,0x1F8010E0u)==((head+60u)&0x00FFFFFFu),"Original OTC destination");
    Check(Read(s.otc,0x1F8010E4u)==16u,"Original OTC size");
    for(uint32_t i=0;i<3u;++i)Check(task.Resume(task.Pending().ticket)==F::State::Waiting,"Empty resume completed OTC");
    for(uint32_t i=0;i<16u;++i)Check(s.Read32(head+4u*i)==0xA5A5A5A5u,"Read/eventless wait cleared a word");
    Reject([&]{s.InitializeScene();});Reject([&]{s.RunScene();});
    const auto serial=s.otc.Serial();
    Check(task.Resume(task.Pending().ticket,[&]{Check(s.otc.Complete(serial,s,s.irq),"OTC event did not transfer");})==F::State::Failed,"Missing primitive binding falsely succeeded");
    uint32_t missing=0;try{task.RethrowFailure();}catch(const Missing&e){missing=e.address;}
    Check(missing==0x8001C550u,"Unexpected next source primitive; inspect evidence");
    Check(s.otc.CompletedTransfers()==1u&&s.otc.WrittenWords()==16u,"Actual work clear not complete");
    for(uint32_t i=0;i<16u;++i)Check(s.Read32(head+4u*i)==(i?((head+4u*i-4u)&0xFFFFFFu):0x5D7F0u),"Native shared tail/OT link mismatch");
    Check(s.Read32(0x80010000u)==lowBefore,"Incorrect OT descriptor wrote unrelated RAM");
    Check(starts==1u&&s.disc.LookupRequests()==7u&&s.disc.ReadRequests()==1u&&s.disc.BytesTransferred()==8192u,"Resource initializer restarted or fabricated reads");
    Check(s.Read32(0x8006ECD4u)==0u&&s.GetPhase()==PrStage2SceneEntry::Phase::Failed,"Incomplete frame acknowledged");
    std::cout<<"loading-work-next 8001c550 1-start 7-lookups 8192-bytes 16-otc-writes\n";
}
}
int main(int argc,char**argv){
    try{
        if(argc==2&&std::string(argv[1])=="--work-trace"){
            TraceMemory m;PrStage2LoadingWorkDirect::Initialize8001E6D0(m);
            for(auto w:m.writes)std::cout<<w[0]<<' '<<w[1]<<'\n';return 0;
        }
        WorkChecks();OtcChecks();if(argc==4)SceneChecks(argv[1],argv[2],argv[3]);else if(argc!=1)return 1;
        std::cout<<"loading-work-pass "<<checks<<" checks; initialization-incomplete\n";return 0;
    }catch(const std::exception&e){std::cerr<<e.what()<<'\n';return 2;}
}
