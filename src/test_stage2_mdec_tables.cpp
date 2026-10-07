#include "pr/pr_stage2_mdec_tables_device.h"
#include <iostream>
#include <map>
#include <thread>
#include <stdexcept>

namespace {
using Args=std::initializer_list<uint32_t>;
uint32_t checks=0;
void Check(bool v,const char* why){++checks;if(!v)throw std::runtime_error(why);}
template<class F>void Reject(F f){bool caught=false;try{f();}catch(const std::exception&){caught=true;}Check(caught,"Invalid MDEC operation was accepted");}
struct Memory:PrStage2LifecycleDirect::Services{
    std::map<uint32_t,uint8_t> bytes;PrPsxGteDirect::MatrixRegisters gte{};
    uint32_t Key(uint32_t a){a&=0x1FFFFFFFu;if(a>=0x200000u)throw std::out_of_range("Non-RAM test address");return a;}
    uint8_t Read8(uint32_t a)override{return bytes.at(Key(a));}
    uint16_t Read16(uint32_t a)override{return uint16_t(Read8(a)|(uint32_t(Read8(a+1))<<8u));}
    uint32_t Read32(uint32_t a)override{return Read16(a)|(uint32_t(Read16(a+2))<<16u);}
    void Write8(uint32_t a,uint8_t v)override{bytes[Key(a)]=v;}
    void Write16(uint32_t a,uint16_t v)override{Write8(a,uint8_t(v));Write8(a+1,uint8_t(v>>8u));}
    void Write32(uint32_t a,uint32_t v)override{Write16(a,uint16_t(v));Write16(a+2,uint16_t(v>>16u));}
    int32_t Call(uint32_t,Args)override{throw std::logic_error("No function success receipts");}
    PrStage2LifecycleDirect::Words64 Call64(uint32_t,Args)override{throw std::logic_error("No wide receipt");}
    PrStage2LifecycleDirect::Vector32 NormalizeVector8003A3DC(PrStage2LifecycleDirect::Vector32)override{throw std::logic_error("No normalize receipt");}
    int32_t SetCdLocation800367A4(uint32_t)override{throw std::logic_error("No CD receipt");}
    int32_t LoadImage80044D64(PrStage2LifecycleDirect::ImageRect,uint32_t)override{throw std::logic_error("No image receipt");}
    PrPsxGteDirect::MatrixRegisters& MatrixGte()override{return gte;}
    [[noreturn]]void Exit(uint32_t,Args)override{throw std::logic_error("Unexpected exit");}
    [[noreturn]]void Break(uint32_t,uint32_t)override{throw std::logic_error("Unexpected BREAK");}
};
using Device=PrStage2MdecTablesDevice::Device;
void Write(Device& d,uint32_t a,uint32_t v){Check(d.TryWrite(a,4u,v),"Unknown MDEC write");}
uint32_t Read(Device& d,uint32_t a){uint32_t v=0;Check(d.TryRead(a,4u,v),"Unknown MDEC read");return v;}
void Arm(Device& d,uint32_t alias,uint32_t source){
    Write(d,0x1F801080u|alias,source);Write(d,0x1F801084u|alias,0x00010020u);Write(d,0x1F801088u|alias,0x01000201u);
}
void Run(){
    for(uint32_t alias:{0u,0x80000000u,0xA0000000u}){
        Memory memory;PrStage2InterruptDevice::Controller irq;Device d(memory,irq);
        std::array<uint8_t,128> q{},scale{};
        for(uint32_t i=0;i<128u;++i){q[i]=uint8_t(i*73u+19u);scale[i]=uint8_t(i*117u+43u);memory.Write8(0x800A0000u+i,q[i]);memory.Write8(0x800A0100u+i,scale[i]);}
        Write(d,0x1F801824u|alias,0x80000000u);
        Check(Read(d,0x1F801824u|alias)==0x80040000u,"Reset status differs");
        Write(d,0x1F801824u|alias,0x60000000u);Write(d,0x1F801820u|alias,0x40000001u);Arm(d,alias,0x800A0000u);
        const auto busy=Read(d,0x1F801088u|alias);const auto serial=d.Serial();
        for(uint32_t i=0;i<20u;++i){Check(Read(d,0x1F801088u|alias)==busy&&d.Serial()==serial&&d.Completions()==0u,"Status read consumed DMA");}
        Check(!d.Service()&&d.Pending()&&!d.QuantKnown(),"Disabled channel consumed source");
        irq.TryWrite(0x1F8010F0u,4u,8u);irq.TryWrite(0x1F8010F4u,4u,0x00810000u);irq.TryWrite(0x1F801074u,2u,8u);
        Write(d,0x1F801824u|alias,0u);Check(!d.Service(),"Disabled MDEC request consumed source");
        Write(d,0x1F801824u|alias,0x60000000u);
        Check(d.Service()&&d.QuantKnown()&&!d.Pending()&&d.Completions()==1u,"Actual quant DMA did not complete");
        for(uint32_t i=0;i<128u;++i)Check(d.Quant()[i]==q[i],"Quant table is not actual source data");
        Check(Read(d,0x1F801080u|alias)==0xA0080u&&Read(d,0x1F801084u|alias)==32u,"DMA pointer/block progress differs");
        uint32_t flags=0;irq.TryRead(0x1F8010F4u,4u,flags);Check((flags&0x81000000u)==0x81000000u&&irq.Pending()==8u,"DMA0 completion IRQ missing");
        Check(!d.Service()&&d.Completions()==1u,"Idle service repeated completion");
        irq.TryWrite(0x1F8010F4u,4u,flags);irq.TryWrite(0x1F801070u,2u,0u);
        Write(d,0x1F801820u|alias,0x60000000u);Arm(d,alias,0x800A0100u);
        Check(d.Service()&&d.ScaleKnown()&&d.Completions()==2u&&d.WordsTransferred()==64u,"Scale table transfer incomplete");
        for(uint32_t i=0;i<128u;++i)Check(d.ScaleBytes()[i]==scale[i],"Scale table is not actual source data");
        Write(d,0x1F801824u|alias,0x80000000u);
        Check(d.Quant()==q&&d.ScaleBytes()==scale&&d.QuantKnown()&&d.ScaleKnown(),"Reset discarded real tables");
        const auto completed=d.Completions();
        Reject([&]{Write(d,0x1F801820u|alias,0x38000020u);});
        Reject([&]{Write(d,0x1F801098u|alias,0x01000200u);});
        Reject([&]{Read(d,0x1F801820u|alias);});
        for(uint32_t width:{1u,2u,8u}){Reject([&]{uint32_t v;d.TryRead(0x1F801824u|alias,width,v);});Reject([&]{d.TryWrite(0x1F801824u|alias,width,0);});}
        Check(d.Completions()==completed,"Rejected decode fabricated completion");
        uint32_t untouched=0x12345678u;Check(!d.TryRead(0x1F801C00u,4u,untouched)&&untouched==0x12345678u,"MDEC claimed SPU");
        bool wrongOwner=false;std::thread other([&]{try{d.Service();}catch(const std::logic_error&){wrongOwner=true;}});other.join();Check(wrongOwner,"Foreign owner used MDEC");
    }
    {Memory m;PrStage2InterruptDevice::Controller irq;Device d(m,irq);irq.TryWrite(0x1F8010F0u,4u,8u);
     Write(d,0x1F801824u,0x60000000u);Write(d,0x1F801820u,0x40000001u);Arm(d,0,0xA0000u);
     Reject([&]{d.Service();});Check(d.Faulted()&&d.Pending()&&d.Completions()==0u&&!d.QuantKnown()&&irq.Pending()==0u,"Missing source produced table or event");}
    {Memory m;PrStage2InterruptDevice::Controller irq;Device d(m,irq);
     Write(d,0x1F801820u,0x60000000u);Write(d,0x1F801080u,0x1FFFFCu);Write(d,0x1F801084u,0x00010020u);
     Reject([&]{Write(d,0x1F801088u,0x01000201u);});Check(d.Starts()==0u,"Out-of-RAM transfer started");}
    std::cout<<"mdec-table-contract 3-aliases 6-dma 192-words exact-source reset-preserves no-read-progress no-fake-picture\n";
}
}
int main(){try{Run();std::cout<<"mdec-table-pass "<<checks<<" assertions\n";return 0;}catch(const std::exception&e){std::cerr<<e.what()<<'\n';return 2;}}
