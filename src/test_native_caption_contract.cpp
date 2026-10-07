#include "pr/pr_native_caption.h"
#include <iostream>
#include <map>
#include <tuple>
#include <vector>

namespace {
uint32_t checks=0;
void Check(bool value,const char* why){++checks;if(!value)throw std::runtime_error(why);}
template<class F>void Reject(F&& function){bool rejected=false;try{function();}catch(const std::exception&){rejected=true;}Check(rejected,"Expected native caption rejection");}
struct Memory:PrStage2LifecycleDirect::Services {
    std::map<uint32_t,uint8_t> bytes;
    uint32_t readCalls=0;
    std::vector<std::tuple<uint32_t,uint32_t,uint32_t>> writes;
    PrPsxGteDirect::MatrixRegisters gte{};
    uint8_t Read8(uint32_t a)override{++readCalls;const auto i=bytes.find(a);if(i==bytes.end())throw std::runtime_error("Uninitialized input byte");return i->second;}
    uint16_t Read16(uint32_t a)override{const auto lo=Read8(a),hi=Read8(a+1u);return uint16_t(lo|(uint16_t(hi)<<8u));}
    uint32_t Read32(uint32_t a)override{const auto lo=Read16(a),hi=Read16(a+2u);return uint32_t(lo)|(uint32_t(hi)<<16u);}
    void Put(uint32_t a,uint32_t n,uint32_t v){for(uint32_t i=0;i<n;++i)bytes[a+i]=uint8_t(v>>(8u*i));writes.emplace_back(a,n,v);}
    void Write8(uint32_t a,uint8_t v)override{Put(a,1u,v);}
    void Write16(uint32_t a,uint16_t v)override{Put(a,2u,v);}
    void Write32(uint32_t a,uint32_t v)override{Put(a,4u,v);}
    int32_t Call(uint32_t f,std::initializer_list<uint32_t> args)override{int32_t v;if(PrNativeCaption::TryCall(*this,f,args,v))return v;throw std::logic_error("No scalar device receipts");}
    void CallVoid(uint32_t f,std::initializer_list<uint32_t> args)override{if(!PrNativeCaption::TryCallVoid(*this,f,args))throw std::logic_error("No void device receipts");}
    PrStage2LifecycleDirect::Words64 Call64(uint32_t,std::initializer_list<uint32_t>)override{throw std::logic_error("No wide call");}
    PrStage2LifecycleDirect::Vector32 NormalizeVector8003A3DC(PrStage2LifecycleDirect::Vector32)override{throw std::logic_error("No vector call");}
    int32_t SetCdLocation800367A4(uint32_t)override{throw std::logic_error("No CD");}
    int32_t LoadImage80044D64(PrStage2LifecycleDirect::ImageRect,uint32_t)override{throw std::logic_error("No upload");}
    PrPsxGteDirect::MatrixRegisters& MatrixGte()override{return gte;}
    [[noreturn]]void Exit(uint32_t,std::initializer_list<uint32_t>)override{throw std::logic_error("No exit receipt");}
    [[noreturn]]void Break(uint32_t,uint32_t)override{throw std::logic_error("No BREAK receipt");}
};
void Abi(){
    Memory m;int32_t result=0x12345678;uint32_t negatives=0;
    // 新字幕框与工作表文本入口同样必须在参数错误时保持零副作用。
    for(uint32_t f:{0x8001B730u,0x8001B954u,0x80047FACu,0x80023E10u,0x8001C6A0u}){
        const size_t arity=(f==0x80047FACu||f==0x80023E10u)?1u:(f==0x8001C6A0u?2u:3u);
        for(const auto args:{std::initializer_list<uint32_t>{},{0u},{0u,0u},{0u,0u,0u},{0u,0u,0u,0u}}){
            if(args.size()==arity)continue;
            const auto old=m.bytes;Reject([&]{PrNativeCaption::TryCallVoid(m,f,args);});++negatives;
            Check(m.bytes==old&&m.writes.empty(),"Bad ABI changed memory");
        }
    }
    Reject([&]{PrNativeCaption::TryCall(m,0x8001B730u,{1,2,3},result);});++negatives;
    Check(result==0x12345678&&m.bytes.empty(),"Void misuse manufactured a result");
    Check(!PrNativeCaption::TryCall(m,0x80001234u,{1,2,3},result)&&result==0x12345678,"Unknown function claimed");
    Check(!PrNativeCaption::TryCallVoid(m,0x80001234u,{1,2,3}),"Unknown void function claimed");
    std::cout<<"caption-abi rejections="<<negatives<<" unknown-preserved=2\n";
}
void OriginAndStrings(){
    Memory m;m.Write32(0x8006ED30u,0xABCD5555u);m.Write32(0x8006ED34u,0x5678FFFFu);m.writes.clear();
    m.CallVoid(0x8001B730u,{0x12348000u,0xABCDFFFFu,0xFFFFFFFFu});
    Check(m.Read32(0x8006ED2Cu)==0xFFFFFFFFu&&m.Read32(0x8006ED30u)==0xABCD8000u&&m.Read32(0x8006ED34u)==0x5678FFFFu,"Origin widths or unsigned carrier changed");
    Check(m.writes==std::vector<std::tuple<uint32_t,uint32_t,uint32_t>>{{0x8006ED2Cu,4u,0xFFFFFFFFu},{0x8006ED30u,2u,0x8000u},{0x8006ED34u,2u,0xFFFFu}},"Origin write order changed");
    m.Write8(0x800B0000u,0u);Check(m.Call(0x80047FACu,{0x800B0000u})==0,"Empty string not read");
    m.Write8(0x800B0000u,0xFFu);m.Write8(0x800B0001u,'A');m.Write8(0x800B0002u,0u);
    Check(m.Call(0x80047FACu,{0x800B0000u})==2,"High byte terminated string");
    const auto old=m.bytes;m.writes.clear();
    const auto reads=m.readCalls;
    Check(m.Call(0x80047FACu,{0u})==0&&m.readCalls==reads,"BIOS strlen(null) must return zero without reading RAM");
    Reject([&]{m.Call(0x80047FACu,{0x800B1000u});});
    Reject([&]{m.Call(0x8001B954u,{0x800B1000u,480u,0x800B2000u});});
    Check(m.bytes==old&&m.writes.empty(),"Invalid text caused a successful write");
    m.Write8(0x800B1000u,'A');m.writes.clear();Reject([&]{m.Call(0x80047FACu,{0x800B1000u});});
    Check(m.writes.empty(),"Unterminated text manufactured state");
}
void PrivateTransport(){
    using namespace PrStage2LoadingPacketsDirect;
    Memory m;constexpr uint32_t packet=0x800C0000u,table=0x800B2000u,head=0x800B2100u;
    m.Write32(0x800901C8u,packet);m.Write32(table+4u,head);m.Write32(table+8u,0u);m.Write32(head,0x00FFFFFFu);
    m.Write16(0x800917AAu,160u);m.Write16(0x800917ACu,120u);m.writes.clear();
    PrivateSprite sprite;sprite.attributes=0x50000040u;sprite.x=4;sprite.y=7;sprite.width=8;sprite.height=15;sprite.page=62;sprite.clutX=256;sprite.clutY=480;
    (void)SortPrivateSprite8003FA20(m,sprite,table,0u,123u);
    Check(m.Read32(0x800901C8u)==packet+24u&&m.Read32(head)==(packet&0xFFFFFFu),"Private sprite bypassed allocator/OT");
    Check(m.Read32(packet+8u)==0x67000000u,"Ignored RGB canonicalization changed opcode");
    Check(m.Read32(packet+12u)==(164u|(127u<<16u))&&m.Read32(packet+20u)==(8u|(15u<<16u)),"Visible sprite fields changed");
    const auto allocator=m.Read32(0x800901C8u),tag=m.Read32(head);
    sprite.attributes=0u;Reject([&]{SortPrivateSprite8003FA20(m,sprite,table,0u,0u);});
    Check(m.Read32(0x800901C8u)==allocator&&m.Read32(head)==tag,"Unknown modulated RGB published an OT primitive");
    m.writes.clear();sprite.attributes=0x80000040u;
    Check(SortPrivateSprite8003FA20(m,sprite,table,0u,0x12345678u)==0x12345678,"Skip path invented scalar");
    Check(m.writes.empty(),"Skipped private sprite wrote a packet");
    sprite.attributes=0x50000040u;sprite.width=0u;Check(SortPrivateSprite8003FA20(m,sprite,table,0u,123u)==0&&m.writes.empty(),"Zero-width primitive emitted");
}
}
int main(){try{Abi();OriginAndStrings();PrivateTransport();std::cout<<"native-caption-contract-pass "<<checks<<" checks\n";return 0;}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 2;}}
