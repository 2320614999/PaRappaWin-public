#include "pr/pr_stage2_ring_dma_device.h"
#include "pr/pr_stage2_ring_direct.h"
#include "pr/pr_stage2_memory_services.h"
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <thread>
#include <map>

namespace {
using Args=std::initializer_list<uint32_t>;
uint64_t checks=0;void Check(bool b,const char* m){++checks;if(!b)throw std::runtime_error(m);}
template<class F>void Reject(F f){bool b=false;try{f();}catch(const std::exception&){b=true;}Check(b,"Expected rejection was absent");}
struct Memory:PrStage2MemoryServices::Services {
    PrPsxGteDirect::MatrixRegisters gte{};
    uint8_t ReadDevice8(uint32_t)override{throw std::runtime_error("Unexpected byte MMIO");}
    uint16_t ReadDevice16(uint32_t)override{throw std::runtime_error("Unexpected halfword MMIO");}
    uint32_t ReadDevice32(uint32_t)override{throw std::runtime_error("Unexpected word MMIO");}
    void WriteDevice8(uint32_t,uint8_t)override{throw std::runtime_error("Unexpected byte MMIO");}
    void WriteDevice16(uint32_t,uint16_t)override{throw std::runtime_error("Unexpected halfword MMIO");}
    void WriteDevice32(uint32_t,uint32_t)override{throw std::runtime_error("Unexpected word MMIO");}
    int32_t CallExternal(uint32_t,Args)override{throw std::runtime_error("No external function receipts");}
    void CallExternalVoid(uint32_t,Args)override{throw std::runtime_error("No external void receipts");}
    PrStage2LifecycleDirect::Words64 Call64(uint32_t,Args)override{throw std::runtime_error("No wide receipts");}
    PrStage2LifecycleDirect::Vector32 NormalizeVector8003A3DC(PrStage2LifecycleDirect::Vector32)override{throw std::runtime_error("No vector receipts");}
    int32_t SetCdLocation800367A4(uint32_t)override{throw std::runtime_error("No CD receipts");}
    int32_t LoadImage80044D64(PrStage2LifecycleDirect::ImageRect,uint32_t)override{throw std::runtime_error("No image receipts");}
    PrPsxGteDirect::MatrixRegisters& MatrixGte()override{return gte;}
    [[noreturn]]void Exit(uint32_t,Args)override{throw std::runtime_error("Unexpected exit");}
    [[noreturn]]void Break(uint32_t,uint32_t)override{throw std::runtime_error("Unexpected BREAK");}
};
struct FailingMemory:PrStage2LifecycleDirect::Services {
    Memory memory;uint32_t writes=0;
    uint8_t Read8(uint32_t a)override{return memory.Read8(a);}uint16_t Read16(uint32_t a)override{return memory.Read16(a);}uint32_t Read32(uint32_t a)override{return memory.Read32(a);}
    void Write8(uint32_t a,uint8_t v)override{memory.Write8(a,v);}void Write16(uint32_t a,uint16_t v)override{memory.Write16(a,v);}
    void Write32(uint32_t a,uint32_t v)override{if(writes==2u)throw std::runtime_error("Explicit third RAM word failure");memory.Write32(a,v);++writes;}
    int32_t Call(uint32_t,Args)override{throw std::runtime_error("No function receipt");}
    PrStage2LifecycleDirect::Words64 Call64(uint32_t f,Args a)override{return memory.Call64(f,a);}
    PrStage2LifecycleDirect::Vector32 NormalizeVector8003A3DC(PrStage2LifecycleDirect::Vector32 v)override{return memory.NormalizeVector8003A3DC(v);}
    int32_t SetCdLocation800367A4(uint32_t a)override{return memory.SetCdLocation800367A4(a);}
    int32_t LoadImage80044D64(PrStage2LifecycleDirect::ImageRect r,uint32_t a)override{return memory.LoadImage80044D64(r,a);}
    PrPsxGteDirect::MatrixRegisters& MatrixGte()override{return memory.gte;}
    [[noreturn]]void Exit(uint32_t f,Args a)override{memory.Exit(f,a);}
    [[noreturn]]void Break(uint32_t f,uint32_t c)override{memory.Break(f,c);}
};
using Cd=PrStage2CdCommandDevice::Device;using Dma=PrStage2RingDmaDevice::Device;using Irq=PrStage2InterruptDevice::Controller;
void W(Cd& d,uint32_t p,uint32_t v){Check(d.TryWrite(p,1u,v),"Unknown CD port");}
void Ack(Cd& d){W(d,0x1F801800u,1u);W(d,0x1F801803u,7u);}
void Command(Cd& d,uint8_t op,Args bytes={}){
    W(d,0x1F801800u,0u);for(uint32_t b:bytes)W(d,0x1F801802u,b);W(d,0x1F801801u,op);
    Check(d.Service()&&d.Flags()==3u,"Actual command not acknowledged");Ack(d);
}
uint8_t Bcd(uint32_t v){return uint8_t(v/10u*16u+v%10u);}
void Begin(Cd& d){const uint32_t absolute=241038u+150u;Command(d,2u,{Bcd(absolute/4500u),Bcd(absolute/75u%60u),Bcd(absolute%75u)});Command(d,14u,{0xC8u});Command(d,27u);}
void Sector(Cd& d){
    const auto until=std::chrono::steady_clock::now()+std::chrono::seconds(2);
    while(d.Flags()==0u){Check(std::chrono::steady_clock::now()<until,"No actual sector event");d.Service();std::this_thread::sleep_for(std::chrono::milliseconds(1));}
    Check(d.Flags()==1u,"Wrong actual data event");Ack(d);W(d,0x1F801800u,0u);W(d,0x1F801803u,128u);
}
void Arm(Dma& d,uint32_t destination,uint32_t words,uint32_t control){Check(d.TryWrite(0x1F8010B0u,4u,destination),"MADR");Check(d.TryWrite(0x1F8010B4u,4u,words),"BCR");Check(d.TryWrite(0x1F8010B8u,4u,control),"CHCR");}
void Provenance(Memory& m){
    uint32_t cases=0;
    for(uint32_t segment:{0u,0x80000000u,0xA0000000u})for(uint32_t mirror=0;mirror<4u;++mirror)
    for(uint32_t mask=0;mask<16u;++mask)for(uint32_t shift=0;shift<4u;++shift){
        const uint32_t src=0x800B0000u+shift,dst=(segment|mirror*0x200000u)+0xB0010u+shift;
        for(uint32_t i=0;i<8u;++i){m.Write8(0x800B0000u+i,0xCCu);m.Write8((dst&~3u)+i,0xA5u);}
        m.WriteSourceWord32(src,{0x12345678u,uint8_t(mask)});
        const auto copy=PrStage2SourceWord::LoadUnaligned(m,src);Check(copy.known==mask,"Source lane validity changed");
        PrStage2SourceWord::StoreUnaligned(m,dst,copy);
        const auto got=m.ReadSourceWord32(dst);Check(got.known==mask,"Copied unknown lanes became valid");
        for(uint32_t i=0;i<4u;++i){if(mask&(1u<<i))Check(m.Read8(dst+i)==uint8_t(0x12345678u>>(i*8u)),"Copied known byte differs");else Reject([&]{(void)m.Read8(dst+i);});}
        if(mask!=15u)Reject([&]{(void)m.Read32(dst);});else Check(m.Read32(dst)==0x12345678u,"Full known word changed");
        ++cases;
    }
    Reject([&]{m.WriteSourceWord32(0x800B0000u,{0,16u});});
    for(uint32_t a:{0x1F8010F4u,0xFFFFFFFFu,0x8005D82Bu,0x8005D82Cu,0x8005D82Fu})Reject([&]{m.WriteSourceWord32(a,{});});
    for(uint32_t a:{0x1F8010F4u,0xFFFFFFFFu})Reject([&]{m.ReadSourceWord32(a);});
    m.WriteSourceWord32(0x800B0000u,{});m.Write16(0x800B0000u,0x1234u);
    Check(m.ReadSourceWord32(0x800B0000u).known==3u,"Partial known write initializes too much");Reject([&]{m.Read32(0x800B0000u);});
    m.Write16(0x800B0002u,0x5678u);Check(m.Read32(0x800B0000u)==0x56781234u,"Complete overwrite not recognized");
    std::cout<<"source-word-contract "<<cases<<" alias-mask-unaligned-copies unknown-numeric-read-rejected\n";
}
void Bus(Memory& m,const std::filesystem::path& disc){
    uint32_t cases=0;
    for(uint32_t flags=0;flags<128u;++flags)for(uint32_t source:{0u,0x88u,0x7Fu,0xFFFFu,0x12345678u,0xFF008080u}){
        Irq irq;Cd cd(disc,irq);Dma dma(m,cd,irq);irq.TryWrite(0x1F8010F4u,4u,0x00FF007Fu);
        for(uint32_t n=0;n<7u;++n)if(flags&(1u<<n))irq.NotifyDma(n,PrStage2InterruptDevice::DmaBoundary::TransferComplete);
        Check(dma.LoadOnDieByte(0x1F8010F6u)==255u,"DICR byte read differs");dma.StoreOnDieByte(0x1F8010F6u,source);
        uint32_t actual=0;irq.TryRead(0x1F8010F4u,4u,actual);const uint32_t bus=source<<16u,ctrl=bus&0x00FF807Fu,pending=flags&~((bus>>24u)&127u);
        const uint32_t expected=ctrl|(pending<<24u)|((ctrl&0x00800000u)&&pending?0x80000000u:0u);
        Check(actual==expected,"SB incorrectly byte-masked DICR or dropped source upper bits");++cases;
    }
    Irq irq;Cd cd(disc,irq);Dma dma(m,cd,irq);
    for(uint32_t a:{0x1F8010F4u,0x1F8010F5u,0x1F8010F7u,0xDF8010F6u}){Reject([&]{dma.LoadOnDieByte(a);});Reject([&]{dma.StoreOnDieByte(a,0u);});}
    for(uint32_t a:{0x1F8010F6u,0x9F8010F6u,0xBF8010F6u}){dma.StoreOnDieByte(a,0x88u);Check(dma.LoadOnDieByte(a)==0x88u,"Canonical SB alias failed");}
    std::cout<<"on-die-sb-contract "<<cases<<" full-source-register-shifts no-byte-mask\n";
}
void Transfers(Memory& m,const std::filesystem::path& disc,const std::filesystem::path& out){
    Irq irq;Cd cd(disc,irq);Dma dma(m,cd,irq);Begin(cd);irq.TryWrite(0x1F801074u,2u,8u);
    std::ofstream words(out/"dma3_actual_sectors.bin",std::ios::binary);Check(bool(words),"Output unavailable");
    for(uint32_t sector=0;sector<10u;++sector){
        Sector(cd);const auto raw=cd.RawSector();Check(cd.LastSector()==241038u+sector,"Actual optical location changed");
        const uint32_t destination=0x800C0000u+sector*2048u;
        dma.StoreOnDieByte(0x1F8010F6u,0x80u);Arm(dma,destination,8u,0x11000000u);
        const auto before=dma.Completions(),written=dma.WordsWritten();uint32_t busy=0;
        for(uint32_t i=0;i<20u;++i){dma.TryRead(0x1F8010B8u,4u,busy);Check((busy&0x01000000u)&&dma.Completions()==before&&dma.WordsWritten()==written,"Observation secretly completed DMA");}
        if(!sector){Check(!dma.Service()&&dma.Pending(),"Disabled DMA3 transferred data");irq.TryWrite(0x1F8010F0u,4u,0x8000u);}
        Check(dma.Service()&&!dma.Pending()&&dma.Completions()==before+1u,"Header DMA did not complete actual writes");
        Check(cd.DmaBytesAvailable()==2016u&&irq.Pending()==0u,"Header transfer consumed payload or signaled frame completion");
        uint32_t reg=0;dma.TryRead(0x1F8010B0u,4u,reg);Check(reg==(destination&0xFFFFFFu),"Manual DMA unexpectedly advanced MADR");
        dma.StoreOnDieByte(0x1F8010F6u,sector==9u?0x88u:0x80u);Arm(dma,destination+32u,504u,0x11400100u);
        Check(dma.Pending()&&dma.Completions()==before+1u,"Payload acknowledged on submission");Check(dma.Service(),"Payload not transferred");
        Check(cd.DmaBytesAvailable()==0u&&dma.Completions()==before+2u,"Payload or release incomplete");
        for(uint32_t i=0;i<2048u;++i){const uint8_t byte=m.Read8(destination+i);Check(byte==raw[24u+i],"Actual DMA RAM differs from raw sector");words.put(char(byte));}
        dma.TryRead(0x1F8010B0u,4u,reg);Check(reg==((destination+2048u)&0xFFFFFFu),"Chopped DMA final address differs");
        dma.TryRead(0x1F8010B4u,4u,reg);Check((reg&65535u)==0u,"Chopped DMA count not exhausted");
        Check((irq.Pending()==8u)==(sector==9u),"Frame IRQ occurred before last payload");
    }
    Check(dma.Starts()==20u&&dma.Completions()==20u&&dma.WordsWritten()==5120u,"Actual DMA split count changed");
    Reject([&]{cd.ReleaseConsumedSector();});Reject([&]{cd.ReadDmaWord();});
    const auto size=dma.Transfers().size();Check(!dma.Service()&&dma.Transfers().size()==size,"Idle DMA repeated transfer");
    std::cout<<"dma3-contract 10-original-sectors 20-transfers 20480-bytes header-payload-separated\n";
}
void Failures(const std::filesystem::path& disc){
    for(bool partial:{false,true}){
        Irq irq;Cd cd(disc,irq);FailingMemory mem;Dma dma(mem,cd,irq);Begin(cd);Sector(cd);irq.TryWrite(0x1F8010F0u,4u,0x8000u);irq.TryWrite(0x1F801074u,2u,8u);dma.StoreOnDieByte(0x1F8010F6u,0x88u);
        Arm(dma,0x800C0000u,partial?8u:513u,0x11000000u);Reject([&]{dma.Service();});
        Check(dma.Faulted()&&dma.Pending()&&dma.Completions()==0u&&irq.Pending()==0u,"Failed DMA fabricated completion");
        Check(mem.writes==(partial?2u:0u)&&dma.ProgressWords()==mem.writes,"Partial failure lost actual write prefix");Reject([&]{dma.Service();});
    }
    Memory m;Irq irq;Cd cd(disc,irq);Dma dma(m,cd,irq);uint32_t untouched=123;
    Check(!dma.TryRead(0x1F801810u,4u,untouched)&&untouched==123u,"DMA claimed other device");
    for(uint32_t a:{0x1F8010B0u,0x9F8010B4u,0xBF8010B8u})for(uint32_t w:{1u,2u,8u}){Reject([&]{dma.TryRead(a,w,untouched);});Reject([&]{dma.TryWrite(a,w,0u);});}
    bool owner=false;std::thread worker([&]{try{dma.TryRead(0x1F8010B8u,4u,untouched);}catch(const std::logic_error&){owner=true;}});worker.join();Check(owner,"Foreign thread got transfer ownership");
    std::cout<<"dma3-failure-contract 2-faults partial-prefix no-fake-irq no-read-pump\n";
}
}
int main(int argc,char**argv){try{
    Check(argc==3,"Arguments: disc output");const auto disc=std::filesystem::u8path(argv[1]),out=std::filesystem::u8path(argv[2]);
    Memory m;Provenance(m);Bus(m,disc);Transfers(m,disc,out);Failures(disc);
    std::cout<<"ring-device-pass "<<checks<<" assertions\n";return 0;
}catch(const std::exception&e){std::cerr<<e.what()<<'\n';return 2;}}
