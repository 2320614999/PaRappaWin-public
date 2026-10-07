#include "pr/pr_stage2_scene_entry.h"
#include "pr/pr_stage2_spu_transfer_device.h"
#include "pr/pr_stage2_spu_events.h"
#include "pr/pr_stage2_spu_boot_direct.h"
#include <iostream>
#include <thread>
#include <vector>

namespace {
using Args=std::initializer_list<uint32_t>;
namespace E=PrStage2SpuEvents;
uint64_t checks=0;
void Check(bool b,const char* why){++checks;if(!b)throw std::runtime_error(why);}
template<class F>void Reject(F&& f){bool caught=false;try{f();}catch(const std::exception&){caught=true;}Check(caught,"Expected transfer/event rejection absent");}
struct Memory final:PrStage2SceneEntry::Services{
    PrStage2SpuDevice::Device spu;
    PrStage2InterruptDevice::Controller irq;
    PrStage2SpuTransferDevice::Device transfer{*this,spu,irq};
    E::Controller events{true};
    PrPsxGteDirect::MatrixRegisters gte{};
    using Services::Services;
    int32_t CallPlatform(uint32_t fn,Args args)override{int32_t r;if(events.TryCall(fn,args,r))return r;throw std::logic_error("Unexpected scalar transfer fixture call");}
    void CallPlatformVoid(uint32_t fn,Args args)override{if(PrStage2SpuBootDirect::TryCallVoid(*this,fn,args)||events.TryCallVoid(fn,args))return;throw std::logic_error("Unexpected void transfer fixture call");}
    uint8_t ReadDevice8(uint32_t a)override{uint32_t v;if(transfer.TryRead(a,1u,v))return uint8_t(v);throw std::logic_error("Unbound MMIO byte");}
    uint16_t ReadDevice16(uint32_t a)override{uint32_t v;if(transfer.TryRead(a,2u,v)||irq.TryRead(a,2u,v)||spu.TryRead(a,2u,v))return uint16_t(v);throw std::logic_error("Unbound MMIO half");}
    uint32_t ReadDevice32(uint32_t a)override{uint32_t v;if(transfer.TryRead(a,4u,v)||irq.TryRead(a,4u,v))return v;throw std::logic_error("Unbound MMIO word");}
    void WriteDevice8(uint32_t a,uint8_t v)override{if(!transfer.TryWrite(a,1u,v))throw std::logic_error("Unbound byte write");}
    void WriteDevice16(uint32_t a,uint16_t v)override{if(transfer.TryWrite(a,2u,v)||irq.TryWrite(a,2u,v)||spu.TryWrite(a,2u,v))return;throw std::logic_error("Unbound half write");}
    void WriteDevice32(uint32_t a,uint32_t v)override{if(transfer.TryWrite(a,4u,v)||irq.TryWrite(a,4u,v))return;throw std::logic_error("Unbound word write");}
    PrStage2LifecycleDirect::Words64 Call64(uint32_t,Args)override{throw std::logic_error("No wide receipt");}
    PrStage2LifecycleDirect::Vector32 NormalizeVector8003A3DC(PrStage2LifecycleDirect::Vector32)override{throw std::logic_error("No vector receipt");}
    int32_t SetCdLocation800367A4(uint32_t)override{throw std::logic_error("No CD receipt");}
    int32_t LoadImage80044D64(PrStage2LifecycleDirect::ImageRect,uint32_t)override{throw std::logic_error("No VRAM receipt");}
    PrPsxGteDirect::MatrixRegisters& MatrixGte()override{return gte;}
    [[noreturn]]void Exit(uint32_t,Args)override{throw std::logic_error("Unexpected exit");}
    [[noreturn]]void Break(uint32_t,uint32_t)override{throw std::logic_error("Unexpected BREAK");}
};
void Events(){
    E::Controller e(true);
    Reject([&]{e.Open(0xF0000009u,0x20u,0x2000u,0u);});
    Check(e.Opened()==0u&&e.InterruptsEnabled(),"Rejected event changed state");
    Check(e.EnterCritical()==1&&e.EnterCritical()==0&&!e.InterruptsEnabled(),"Critical state is incorrectly a nesting counter");
    Reject([&]{e.Open(0xF0000009u,0x20u,0x1000u,0u);});
    const uint32_t h=e.Open(0xF0000009u,0x20u,0x2000u,0u);
    Check(h==0xF1000000u&&e.Test(h)==0,"Opened event should be disabled");
    e.Deliver(0xF0000009u,0x20u);Check(e.Test(h)==0,"Disabled event accepted delivery");
    Check(e.Enable(h)==1&&e.Test(h)==0,"Enable fabricated completion");
    e.ExitCritical();Check(e.InterruptsEnabled(),"Critical section did not exit");
    e.Deliver(0xF0000009u,0x10u);Check(e.Test(h)==0,"Wrong event specification completed wait");
    e.Deliver(0xF0000009u,0x20u);e.Deliver(0xF0000009u,0x20u);
    Check(e.Test(h)==1&&e.Test(h)==0&&e.Consumed()==1u,"Event was not coalesced and consumed once");
    e.Disable(h);e.Deliver(0xF0000009u,0x20u);Check(e.Test(h)==0,"Disabled event became ready");
    Check(e.Close(h)==1&&e.Close(h)==1&&e.Enable(0u)==1,"BIOS-compatible invalid close/enable result differs");
    Reject([&]{e.Test(h);});
    e.EnterCritical();for(uint32_t i=0;i<16u;++i)Check(e.Open(0xF0000009u,0x20u,0x2000u,0u)==0xF1000000u+i,"Event slot allocation order");
    Check(e.Open(0xF0000009u,0x20u,0x2000u,0u)==0xFFFFFFFFu,"Event exhaustion not reported");
    bool foreign=false;std::thread t([&]{try{e.EnterCritical();}catch(const std::logic_error&){foreign=true;}});t.join();Check(foreign,"Foreign thread entered native event owner");
    int32_t unchanged=123;Check(!e.TryCall(0x800FCAFEu,{},unchanged)&&unchanged==123,"Unknown event function consumed result");
    Reject([&]{e.TryCall(0x80048A50u,{},unchanged);});Reject([&]{e.TryCall(0x80048990u,{},unchanged);});
    std::cout<<"spu-events-contract disabled-busy-ready critical-owner bounded-handles no-fake-testevent\n";
}
uint32_t Read(PrStage2SpuTransferDevice::Device& d,uint32_t a,uint32_t n){uint32_t v=0;Check(d.TryRead(a,n,v),"Unknown transfer register");return v;}
void Write(PrStage2SpuTransferDevice::Device& d,uint32_t a,uint32_t n,uint32_t v){Check(d.TryWrite(a,n,v),"Unknown transfer register");}
void Test(const char* scus,const char* overlay){
    for(uint32_t alias:{0u,0x80000000u,0xA0000000u}){
        Memory m(std::filesystem::u8path(scus),std::filesystem::u8path(overlay));auto& d=m.transfer;
        for(uint32_t words:{1u,8u,32u}){
            Write(d,0x1F801DA6u|alias,2u,0x200u);std::vector<uint8_t> expected;
            for(uint32_t i=0;i<words;++i){const uint16_t v=uint16_t(0xA5B0u+i);Write(d,0x1F801DA8u|alias,2u,v);expected.push_back(uint8_t(v));expected.push_back(uint8_t(v>>8u));}
            const auto completed=d.FifoCompletions();Write(d,0x1F801DAAu|alias,2u,0x10u);
            Check(d.Pending()&&(Read(d,0x1F801DAEu|alias,2u)&0x400u),"Manual FIFO did not become pending");
            for(uint32_t i=0;i<8u;++i)Check((Read(d,0x1F801DAEu|alias,2u)&0x400u)&&d.FifoCompletions()==completed,"SPU status read completed FIFO");
            Reject([&]{Write(d,0x1F801DA6u|alias,2u,0u);});
            Check(d.Service()&&!d.Pending()&&d.FifoCompletions()==completed+1u,"FIFO service failed");
            Check(m.spu.ReadRam(0x1000u,words*2u)==expected,"Actual FIFO/SPU contents differ");
            Check((Read(d,0x1F801DAEu|alias,2u)&0x400u)==0u&&d.DmaCompletions()==0u&&m.irq.Pending()==0u,"Manual FIFO falsely delivered DMA IRQ");
            Write(d,0x1F801DAAu|alias,2u,0u);
        }
        for(uint32_t length:{64u,128u,4096u}){
            const uint32_t destination=0x2000u+length,source=0x800B0000u;
            std::vector<uint8_t> expected;
            for(uint32_t i=0;i<length;++i){const uint8_t v=uint8_t(i*37u+(i>>8u));m.Write8(source+i,v);expected.push_back(v);}
            m.events.EnterCritical();const uint32_t h=m.events.Open(0xF0000009u,0x20u,0x2000u,0u);m.events.Enable(h);m.events.ExitCritical();
            m.Write32(0x80055678u,h);m.Write32(0x800555FCu,0u);m.Write32(0x80055614u,0u);
            m.irq.TryWrite(0x1F8010F0u,4u,0u);m.irq.TryWrite(0x1F8010F4u,4u,0x00900000u);m.irq.TryWrite(0x1F801074u,2u,8u);
            Write(d,0x1F801DA6u|alias,2u,destination/8u);Write(d,0x1F801DAAu|alias,2u,0x20u);
            Write(d,0x1F8010C0u|alias,4u,source);Write(d,0x1F8010C4u|alias,4u,((length/64u)<<16u)|16u);
            Write(d,0x1F8010C8u|alias,4u,0x01000201u);const auto before=d.DmaCompletions();
            Check(!d.Service()&&d.Pending()&&d.DmaCompletions()==before,"Disabled SPU DMA transferred data");
            for(uint32_t i=0;i<16u;++i)Check((Read(d,0x1F8010C8u|alias,4u)&0x01000000u)&&d.DmaCompletions()==before&&m.events.Test(h)==0,"Status/event read progressed transfer");
            Reject([&]{Write(d,0x1F8010C0u|alias,4u,0u);});Reject([&]{Write(d,0x1F8010C8u|alias,4u,0u);});
            m.irq.TryWrite(0x1F8010F0u,4u,0x80000u);Check(d.Service()&&!d.Pending()&&d.DmaCompletions()==before+1u,"Actual DMA4 did not complete");
            Check(m.spu.ReadRam(destination,length)==expected,"Actual DMA4/SPU RAM differs");
            Check(Read(d,0x1F8010C0u|alias,4u)==((source+length)&0xFFFFFFu)&&Read(d,0x1F8010C4u|alias,4u)==16u,"Slice-mode DMA postcondition differs");
            Check(m.irq.Pending()==8u&&m.events.Test(h)==0,"DMA device bypassed original ISR software event");
            m.CallVoid(0x80029E6Cu,{});
            Check(m.events.Test(h)==1&&m.events.Test(h)==0&&(Read(d,0x1F801DAAu,2u)&0x30u)==0u,"Original ISR did not clear mode/deliver one event");
            m.events.Close(h);m.irq.TryWrite(0x1F8010F4u,4u,0x10000000u);m.irq.TryWrite(0x1F801070u,2u,0u);
        }
        // The reverse direction is a supported explicit capability too: use
        // the documented stable-read bus mode and compare every copied byte.
        {
            constexpr uint32_t source=0x5000u,destination=0x800C0000u,length=256u;
            std::vector<uint8_t> expected;
            for(uint32_t i=0;i<length;++i)expected.push_back(uint8_t(i*17u+9u));
            m.spu.Upload(source,expected);
            Write(d,0x1F801014u|alias,4u,0x220931E1u);
            Write(d,0x1F801DA6u|alias,2u,source/8u);Write(d,0x1F801DAAu|alias,2u,0x30u);
            Write(d,0x1F8010C0u|alias,4u,destination);Write(d,0x1F8010C4u|alias,4u,0x00040010u);
            const auto before=d.DmaCompletions();Write(d,0x1F8010C8u|alias,4u,0x01000200u);
            Check(d.Pending()&&d.DmaCompletions()==before,"DMA read completed in register write");
            Check(d.Service()&&d.DmaCompletions()==before+1u,"DMA read service did not complete");
            for(uint32_t i=0;i<length;++i)Check(m.Read8(destination+i)==expected[i],"Stable SPU-to-RAM readback differs");
            Write(d,0x1F801DAAu|alias,2u,0u);
        }
        uint32_t unchanged=77;Check(!d.TryRead(0x1F8010A8u,4u,unchanged)&&unchanged==77,"SPU device claimed GPU channel");
        for(uint32_t a:{0x1F8010C0u,0x1F801014u})for(uint32_t n:{1u,2u,8u}){Reject([&]{d.TryRead(a|alias,n,unchanged);});Reject([&]{d.TryWrite(a|alias,n,0u);});}
        for(uint32_t a:{0x1F801DA6u,0x1F801DA8u,0x1F801DAAu})for(uint32_t n:{1u,4u,8u}){Reject([&]{d.TryRead(a|alias,n,unchanged);});Reject([&]{d.TryWrite(a|alias,n,0u);});}
        Reject([&]{Write(d,0x1F801DAEu|alias,2u,0u);});Reject([&]{Write(d,0x1F801DA6u|alias,2u,65536u);});
    }
    {
        Memory m(std::filesystem::u8path(scus),std::filesystem::u8path(overlay));auto& d=m.transfer;
        Write(d,0x1F801DA6u,2u,0xFFFFu);Write(d,0x1F801DAAu,2u,0x20u);
        Write(d,0x1F8010C0u,4u,0x100000u);Write(d,0x1F8010C4u,4u,0x10010u);
        Reject([&]{Write(d,0x1F8010C8u,4u,0x01000201u);});Check(d.DmaStarts()==0u&&!d.Pending(),"Out-of-SPU span started DMA");
        Write(d,0x1F801DAAu,2u,0u);Write(d,0x1F801DA6u,2u,0x200u);
        for(uint32_t i=0;i<32u;++i)Write(d,0x1F801DA8u,2u,i);
        Reject([&]{Write(d,0x1F801DA8u,2u,1u);});Write(d,0x1F801DAAu,2u,0x10u);Check(d.Service(),"Valid FIFO lost after overflow rejection");
    }
    {
        Memory m(std::filesystem::u8path(scus),std::filesystem::u8path(overlay));auto& d=m.transfer;
        m.irq.TryWrite(0x1F8010F0u,4u,0x80000u);m.irq.TryWrite(0x1F8010F4u,4u,0x00900000u);m.irq.TryWrite(0x1F801074u,2u,8u);
        Write(d,0x1F801DA6u,2u,0x400u);Write(d,0x1F801DAAu,2u,0x20u);
        Write(d,0x1F8010C0u,4u,0u);Write(d,0x1F8010C4u,4u,0x10010u);Write(d,0x1F8010C8u,4u,0x01000201u);
        Reject([&]{d.Service();});Check(d.Faulted()&&d.Pending()&&d.DmaCompletions()==0u&&m.irq.Pending()==0u,"Failed source read acknowledged DMA");
        Reject([&]{m.spu.ReadRam(0x2000u,64u);});Reject([&]{d.Service();});
    }
    std::cout<<"spu-transfer-contract 9-fifo 9-dma4 3-readback aliases actual-ram original-isr-event failure-gates\n";
}
}
int main(int argc,char**argv){try{Check(argc==3,"Arguments: SCUS COMOD2");Events();Test(argv[1],argv[2]);std::cout<<"spu-transfer-pass "<<checks<<" assertions\n";return 0;}catch(const std::exception&e){std::cerr<<e.what()<<'\n';return 2;}}
