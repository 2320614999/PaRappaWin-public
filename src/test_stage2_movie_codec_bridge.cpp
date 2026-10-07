#include "pr/pr_stage2_movie_codec_bridge.h"
#include <algorithm>
#include <fstream>
#include <functional>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <tuple>

namespace {
namespace B=PrStage2MovieCodecBridge;
using S=PrStage2LifecycleDirect::Services;
uint32_t checks=0,negative=0;
void Check(bool v,const char* m){++checks;if(!v)throw std::runtime_error(m);}
template<class F> void Reject(F f){try{f();}catch(const std::exception&){++negative;return;}throw std::runtime_error("Invalid codec operation accepted");}
constexpr uint32_t control=0x800B0000u,packet=0x800C0000u,image=0x800E0000u;
struct Memory final:S{
    std::vector<uint8_t> ram=std::vector<uint8_t>(0x200000u,0xCDu);
    std::vector<std::pair<uint32_t,std::vector<uint32_t>>> calls;
    std::vector<uint32_t> packetWrites;
    PrPsxGteDirect::MatrixRegisters gte{};
    bool rewriteFirst=false,throwCallback=false;
    uint32_t failWrite=0;
    uint32_t At(uint32_t a){const auto p=a&0x1FFFFFFFu;if(p>=ram.size())throw std::out_of_range("Fixture RAM");return p;}
    uint8_t Read8(uint32_t a)override{return ram.at(At(a));}
    uint16_t Read16(uint32_t a)override{return uint16_t(Read8(a))|(uint16_t(Read8(a+1))<<8);}
    uint32_t Read32(uint32_t a)override{return uint32_t(Read16(a))|(uint32_t(Read16(a+2))<<16);}
    void Write8(uint32_t a,uint8_t v)override{if(a==failWrite)throw std::runtime_error("Injected output write failure");ram.at(At(a))=v;}
    void Write16(uint32_t a,uint16_t v)override{Write8(a,uint8_t(v));Write8(a+1,uint8_t(v>>8));}
    void Write32(uint32_t a,uint32_t v)override{Write16(a,uint16_t(v));Write16(a+2,uint16_t(v>>16));if(a==packet){packetWrites.push_back(v);if(rewriteFirst&&packetWrites.size()==1u){ram[At(packet)]^=1u;}}}
    int32_t Call(uint32_t fn,std::initializer_list<uint32_t> args)override{
        calls.emplace_back(fn,std::vector<uint32_t>(args));
        if(fn==0x80027220u||fn==0x800F0000u){
            Check(Read32(control+32u)==1u,"Output callback ran before caller request");
            if(throwCallback)throw std::runtime_error("Injected callback failure");
            Write32(Read32(0x8006ED78u)+32u,0u);
        }
        return -37; // Explicit external return input; wrappers must discard it.
    }
    PrStage2LifecycleDirect::Words64 Call64(uint32_t,std::initializer_list<uint32_t>)override{throw std::logic_error("No wide calls");}
    PrStage2LifecycleDirect::Vector32 NormalizeVector8003A3DC(PrStage2LifecycleDirect::Vector32)override{throw std::logic_error("No GTE calls");}
    int32_t SetCdLocation800367A4(uint32_t)override{throw std::logic_error("No CD calls");}
    int32_t LoadImage80044D64(PrStage2LifecycleDirect::ImageRect,uint32_t)override{throw std::logic_error("No GPU calls");}
    PrPsxGteDirect::MatrixRegisters& MatrixGte()override{return gte;}
    [[noreturn]]void Exit(uint32_t,std::initializer_list<uint32_t>)override{throw std::logic_error("No exit");}
    [[noreturn]]void Break(uint32_t,uint32_t)override{throw std::logic_error("No BREAK");}
};
std::vector<uint8_t> Read(const char* path){std::ifstream f(path,std::ios::binary);if(!f)throw std::runtime_error("Input missing");return {(std::istreambuf_iterator<char>(f)),{}};}
void Init(Memory& m,const std::vector<uint8_t>& rle){
    for(uint32_t i=0;i<rle.size();++i)m.Write8(packet+i,rle[i]);
    m.Write32(0x8006ED78u,control);m.Write32(control,packet);m.Write32(control+4u,image);
    m.Write32(control+8u,256u);m.Write32(control+12u,144u);m.Write32(control+32u,0u);
    m.Write32(0x80057044u,0x80027220u);B::Input80047558(m,packet,2u);
}
void Wrappers(){
    for(uint32_t mode=0;mode<256u;++mode){
        Memory m;const uint32_t start=0xAD817331u;m.Write32(packet,start);m.packetWrites.clear();
        B::Input80047558(m,packet,mode);
        const uint32_t one=(mode&1u)?start&0xF7FFFFFFu:start|0x08000000u;
        const uint32_t two=(mode&2u)?one|0x02000000u:one&0xFDFFFFFFu;
        Check(m.packetWrites==std::vector<uint32_t>{one,two},"Source input stores differ");
        Check(m.calls.size()==1u&&m.calls[0].first==0x80047778u&&m.calls[0].second==std::vector<uint32_t>{packet,two&65535u},"Source input tail call differs");
        B::Output800475D4(m,image,18432u);
        Check(m.calls.back()==std::make_pair(0x8004780Cu,std::vector<uint32_t>{image,18432u}),"Source output wrapper arguments differ");
    }
    Memory m;m.Write32(packet,0x38000360u);m.packetWrites.clear();m.rewriteFirst=true;
    B::Input80047558(m,packet,2u);Check(m.Read32(packet)==0x3A000361u,"Second source format load was cached");
}
void Run(const std::vector<uint8_t>& scus,const std::vector<uint8_t>& rle){
    std::array<uint8_t,128> quant{},scale{};
    const size_t q=0x8005D858u-0x80010000u+0x800u,s=0x8005D8DCu-0x80010000u+0x800u;
    Check(scus.size()>s+128u,"SCUS tables absent");std::copy_n(scus.begin()+q,128,quant.begin());std::copy_n(scus.begin()+s,128,scale.begin());
    Memory m;Init(m,rle);B::Output out;out.Input(m,packet,0x360u,quant,scale);
    Check(out.DecodedFrames()==1u&&!out.LastCompleted(),"Decoded input was prematurely published");
    Reject([&]{out.Request(m,image,18431u);});Reject([&]{out.Input(m,packet,0x360u,quant,scale);});
    out.Request(m,image,18432u);Check(m.Read8(image)==0xCDu&&out.CallbackCalls()==0u,"Queue submission forged output");
    m.Write32(control+32u,1u);const auto calls=m.calls.size();
    Check(!out.Service(m,true,false)&&out.PublishedFrames()==0u&&out.CallbackCalls()==0u&&
          m.calls.size()==calls&&m.Read32(control+32u)==1u,
          "Future media deadline published output or acknowledged completion");
    for(uint32_t i=0;i<73728u;++i)Check(m.Read8(image+i)==0xCDu,
          "Future media frame overwrote the image still owned by the blitter");
    Check(out.Service(m,false)&&out.PublishedFrames()==1u&&out.CallbackCalls()==0u&&m.calls.size()==calls,"Masked callback was executed or image not copied");
    Check(m.Read32(control+32u)==1u&&!out.Service(m,false),"Masked completion was fabricated or repeated");
    const auto result=out.LastCompleted();Check(result&&result->strips15.size()==73728u,"Output missing");
    for(uint32_t i=0;i<73728u;++i)Check(m.Read8(image+i)==result->strips15[i],"Native output buffer differs from decoded pixels");
    Check(m.Read8(image+73728u)==0xCDu,"Native output overran source buffer");
    m.Write32(0x80057044u,0x800F0000u);
    Check(out.Service(m,true)&&m.calls.back().first==0x800F0000u&&out.CallbackCalls()==1u,"Callback slot was cached at submission");
    Check(m.Read32(control+32u)==0u&&!out.Service(m,true),"Original callback effect missing or repeated");
    out.Input(m,packet,0x360u,quant,scale);out.Request(m,image,18432u);out.Cancel();
    Check(!out.Pending()&&!out.Service(m,true)&&out.LastCompleted()==result,"Cancellation changed completed image or emitted a callback");
    for(uint32_t kind=0;kind<8u;++kind){
        Memory n;Init(n,rle);B::Output o;auto q2=quant,s2=scale;
        if(kind==0u)q2[64]^=1u;if(kind==1u)s2[127]^=1u;
        if(kind==2u)n.Write32(control+8u,65536u);if(kind==3u)n.Write32(control+12u,0u);
        if(kind==4u)n.Write32(packet,0x38000360u);if(kind==5u)n.Write32(control,packet+4u);
        if(kind==6u)n.Write32(control+8u,255u);if(kind==7u)n.Write16(packet,0xFFFFu);
        Reject([&]{o.Input(n,packet,0x360u,q2,s2);});
        Check(o.DecodedFrames()==0u&&!o.LastCompleted()&&n.Read8(image)==0xCDu,"Invalid decode published data");
    }
    {
        Memory n;Init(n,rle);B::Output o;o.Input(n,packet,0x360u,quant,scale);
        n.Write32(control+4u,0x1F801820u);Reject([&]{o.Request(n,0x1F801820u,18432u);});
    }
    {
        Memory n;Init(n,rle);B::Output o;o.Input(n,packet,0x360u,quant,scale);o.Request(n,image,18432u);
        Reject([&]{o.Service(n,true);});Check(!o.LastCompleted()&&o.CallbackCalls()==0u,"Pre-publication callback escaped");
    }
    {
        Memory n;Init(n,rle);B::Output o;o.Input(n,packet,0x360u,quant,scale);o.Request(n,image,18432u);n.Write32(control+32u,1u);n.failWrite=image+17u;
        const auto before=n.calls.size();Reject([&]{o.Service(n,true);});n.failWrite=0;
        Check(n.Read8(image)==result->strips15[0]&&n.Read8(image+17u)==0xCDu&&!o.LastCompleted()&&o.PublishedFrames()==0u&&n.calls.size()==before,"Failed write invented complete output");
        Reject([&]{o.Service(n,true);});
    }
    {
        Memory n;Init(n,rle);B::Output o;o.Input(n,packet,0x360u,quant,scale);o.Request(n,image,18432u);n.Write32(control+32u,1u);n.throwCallback=true;
        Reject([&]{o.Service(n,true);});Check(o.PublishedFrames()==1u&&o.CallbackCalls()==0u,"Callback failure hidden");Reject([&]{o.Service(n,true);});
    }
    std::cout<<"shared-movie-output-contract "<<checks<<" assertions "<<negative<<" negative-cases no-mmio no-dma-irq\n";
}
}
int main(int argc,char**argv){try{Check(argc==3,"Arguments: SCUS original-RLE-allocation");Wrappers();Run(Read(argv[1]),Read(argv[2]));return 0;}catch(const std::exception&e){std::cerr<<e.what()<<'\n';return 2;}}
