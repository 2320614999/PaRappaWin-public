#include "pr/pr_stage2_movie_foreground_direct.h"
#include "pr/pr_stage2_cd_command_direct.h"
#include <functional>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
namespace F=PrStage2MovieForegroundDirect;
namespace C=PrStage2CdCommandDirect;
using Args=std::initializer_list<uint32_t>;
uint64_t assertions=0;
void Check(bool b,const char* why){++assertions;if(!b)throw std::runtime_error(why);}
template<class Fn>void Reject(Fn f){bool rejected=false;try{f();}catch(const std::exception&){rejected=true;}Check(rejected,"Expected rejection did not occur");}
struct Memory final:PrStage2LifecycleDirect::Services {
    std::map<uint32_t,uint8_t> data;
    std::vector<std::string> events;
    std::function<int32_t(uint32_t,Args)> callback;
    PrPsxGteDirect::MatrixRegisters gte{};
    uint64_t reads=0,writes=0,calls=0,waits=0;
    uint32_t failWrite=0,makeReadyOnWait=0;
    void Put(uint32_t a,uint32_t n,uint32_t v){for(uint32_t i=0;i<n;++i)data[a+i]=uint8_t(v>>(8u*i));}
    uint32_t Load(uint32_t a,uint32_t n){
        uint32_t v=0;++reads;
        for(uint32_t i=0;i<n;++i){const auto at=data.find(a+i);if(at==data.end())throw std::runtime_error("Unseeded test byte "+std::to_string(a+i));v|=uint32_t(at->second)<<(8u*i);}
        events.push_back("R "+std::to_string(a)+" "+std::to_string(n)+" "+std::to_string(v));return v;
    }
    void Store(uint32_t a,uint32_t n,uint32_t v){
        if(failWrite&&a<=failWrite&&uint64_t(a)+n>failWrite)throw std::runtime_error("Injected actual RAM write failure");
        ++writes;Put(a,n,v);events.push_back("W "+std::to_string(a)+" "+std::to_string(n)+" "+std::to_string(v));
    }
    uint8_t Read8(uint32_t a)override{return uint8_t(Load(a,1));}
    uint16_t Read16(uint32_t a)override{return uint16_t(Load(a,2));}
    uint32_t Read32(uint32_t a)override{return Load(a,4);}
    void Write8(uint32_t a,uint8_t v)override{Store(a,1,v);}
    void Write16(uint32_t a,uint16_t v)override{Store(a,2,v);}
    void Write32(uint32_t a,uint32_t v)override{Store(a,4,v);}
    int32_t Call(uint32_t fn,Args args)override{
        ++calls;events.push_back("C "+std::to_string(fn));
        if(callback)return callback(fn,args);
        if(fn==0x80035560u)return 0;
        if(fn==0x80035898u)return 0;
        if(fn==0x800363A4u)return Read8(0x80057119u);
        if(fn==0x80036678u)return -73; // Explicit leaf input, never product behavior.
        throw std::runtime_error("Unexpected service call "+std::to_string(fn));
    }
    void AwaitDeviceProgress(uint32_t a)override{
        Check(a==0x1F801800u,"Unexpected wait channel");++waits;
        if(!makeReadyOnWait)throw std::runtime_error("Test wait has no explicit completion");
        Put(0x800573D4u,1u,makeReadyOnWait);
    }
    PrStage2LifecycleDirect::Words64 Call64(uint32_t,Args)override{throw std::logic_error("No wide receipt");}
    PrStage2LifecycleDirect::Vector32 NormalizeVector8003A3DC(PrStage2LifecycleDirect::Vector32)override{throw std::logic_error("No vector receipt");}
    int32_t SetCdLocation800367A4(uint32_t)override{throw std::logic_error("No location receipt");}
    int32_t LoadImage80044D64(PrStage2LifecycleDirect::ImageRect,uint32_t)override{throw std::logic_error("No image receipt");}
    PrPsxGteDirect::MatrixRegisters& MatrixGte()override{return gte;}
    [[noreturn]]void Exit(uint32_t,Args)override{throw std::logic_error("Source exit");}
    [[noreturn]]void Break(uint32_t,uint32_t)override{throw std::logic_error("Source break");}
};
Memory Seed(uint32_t status=2u){
    Memory m;m.Put(0x800573D4u,1u,status);m.Put(0x80057119u,1u,16u);m.Put(0x800570F8u,4u,0u);
    for(uint32_t i=0;i<8u;++i)m.Put(0x800882F8u+i,1u,0x20u+i);
    return m;
}
void ReplyTests(){
    uint32_t scenarios=0;
    for(uint32_t status:{0u,2u,5u})for(uint32_t nonblocking:{0u,1u}){
        Memory a=Seed(status),b=Seed(status);a.makeReadyOnWait=b.makeReadyOnWait=2u;
        const int32_t publicResult=C::Sync80037070(a,nonblocking,0x800B6000u);
        const auto reply=C::SyncReply80037070(b,nonblocking);++scenarios;
        Check(reply.status==publicResult,"Public/private Sync status differs");
        const bool copied=status!=0u||nonblocking==0u;
        Check(reply.copied==copied,"Busy Sync claimed valid private bytes");
        if(copied)for(uint32_t i=0;i<8u;++i)Check(a.data.at(0x800B6000u+i)==reply.bytes[i]&&reply.bytes[i]==0x20u+i,"Owned CD response bytes differ");
        auto events=a.events;
        for(auto i=events.begin();i!=events.end();)if(i->rfind("W "+std::to_string(0x800B6000u),0)==0)i=events.erase(i);else++i;
        // Remove the eight destination writes, retaining read/call/source-write order.
        events.clear();for(const auto& e:a.events){bool dst=false;for(uint32_t i=0;i<8u;++i)if(e.rfind("W "+std::to_string(0x800B6000u+i)+" ",0)==0)dst=true;if(!dst)events.push_back(e);}
        Check(events==b.events,"Private Sync changed original source reads, calls or writes");
    }
    for(uint32_t byte=0;byte<256u;++byte){
        auto m=Seed(2);m.Put(0x800882F8u,1,byte);
        Check(F::Ready8001A750(m)==((byte&0x20u)?1:0),"Reading-state bit was ignored");++scenarios;
    }
    for(uint32_t status:{0u,2u,5u})for(uint32_t first:{0u,1u,16u,32u,255u})for(uint32_t command:{0u,13u,16u}){
        auto m=Seed(status);m.Put(0x800882F8u,1,first);m.Put(0x80057119u,1,command);m.Put(0x80049428u,4,0xABCDEF12u);
        for(uint32_t i=0;i<3;++i)m.Put(0x800493F4u+i,1,0xDDu);
        const int32_t result=F::RefreshLocation8001A3C8(m);++scenarios;
        const bool good=status==2u&&command!=13u;
        Check(result==int32_t(good),"Location result uses a fabricated status");
        Check(m.Read32(0x80049428u)==(good?0u:status==5u&&(first&16u)?1u:0xABCDEF12u),"CD error state was erased");
        if(good)for(uint32_t i=0;i<3u;++i)Check(m.Read8(0x800493F4u+i)==(i?0x20u+i:first),"Location copy differs");
    }
    auto changed=Seed(2);changed.Put(0x80049428u,4,0);
    changed.callback=[&](uint32_t fn,Args)->int32_t{
        if(fn==0x80035560u||fn==0x80035898u)return 0;
        if(fn==0x800363A4u){for(uint32_t i=0;i<8;++i)changed.Put(0x800882F8u+i,1,255u);return 16;}
        throw std::runtime_error("Unexpected reread test call");
    };
    Check(F::RefreshLocation8001A3C8(changed)==1,"Owned location response did not complete");
    for(uint32_t i=0;i<3;++i)Check(changed.Read8(0x800493F4u+i)==0x20u+i,"Private response reread mutable shared buffer after call");
    std::cout<<"private-response-scenarios "<<scenarios<<" mutable-source-copy 1\n";
}
void LocationTests(){
    const std::array<uint8_t,8> header{{0x53,0x40,0x14,2,1,0,2,0}};
    auto m=Seed();F::LocationQuery q;auto input=header;
    q.Request(m,input);input.fill(0);const auto data=m.data;
    Check(q.Pending()&&q.Requests()==1u&&q.Replies()==0u,"Request manufactured a reply");
    for(uint32_t i=0;i<8;++i)Check(!q.Service(m,false)&&m.data==data,"Masked callback service advanced query");
    m.Put(0x800570F8u,4,0x800F0050u);uint32_t callbacks=0;
    m.callback=[&](uint32_t fn,Args args){
        Check(fn==0x800F0050u&&std::vector<uint32_t>(args)==std::vector<uint32_t>({2u,0x800882F8u}),"Wrong native SDK callback");
        Check(m.Read8(0x800573D4u)==2u,"Callback ran before actual response publication");
        for(uint32_t i=0;i<8;++i)Check(m.Read8(0x800882F8u+i)==header[i],"Callback observed unowned or incomplete bytes");
        ++callbacks;return -117;
    };
    Check(q.Service(m,true)&&q.Replies()==1u&&!q.Pending()&&callbacks==1u,"Actual native reply did not publish");
    Check(!q.Service(m,true)&&callbacks==1u,"Reply was duplicated by another service");
    for(uint32_t i=0;i<4u;++i){auto bad=header;bad[i]=255;auto memory=Seed();F::LocationQuery state;const auto old=memory.data;Reject([&]{state.Request(memory,bad);});Check(old==memory.data&&!state.Pending()&&state.Requests()==0u,"Invalid location changed source state");}
    for(uint32_t fail=0;fail<8u;++fail){auto memory=Seed();F::LocationQuery state;state.Request(memory,header);memory.failWrite=0x800882F8u+fail;Reject([&]{state.Service(memory,true);});Check(state.Replies()==0u&&memory.data[0x800573D4u]==0u,"Failed response bytes were acknowledged");Reject([&]{state.Service(memory,true);});Reject([&]{state.Request(memory,header);});}
    auto memory=Seed();F::LocationQuery state;state.Request(memory,header);memory.Put(0x800570F8u,4,0x800F0050u);
    memory.callback=[&](uint32_t,Args){memory.Put(0x800570F8u,4,0u);state.Request(memory,header);return 0;};
    Check(state.Service(memory,true)&&state.Pending()&&state.Requests()==2u&&state.Replies()==1u,"Reentrant callback erased new native request");
    Check(state.Service(memory,true)&&state.Replies()==2u&&!state.Pending(),"Reentrant response did not finish");
    std::cout<<"native-location-tests owned-bytes deferred-publication mutable-callback partial-failures reentry\n";
}
void ArityTests(){
    uint32_t rejected=0;
    for(auto pair:std::vector<std::pair<uint32_t,size_t>>{{0x80024CF8,1},{0x8001A750,0},{0x8001A3C8,0},{0x8001A280,0},{0x800363A4,0},{0x8001A3B8,0},{0x8001A7A4,1},{0x8001A7F8,1},{0x8001EC54,2},{0x8001ED3C,1},{0x8001ED74,0},{0x80027528,0},{0x8002756C,0},{0x8001DB00,2},{0x80035510,1}}){
        for(Args args:std::initializer_list<Args>{{},{0u},{0u,0u},{0u,0u,0u}}){if(args.size()==pair.second)continue;
            Memory m;int32_t result=99;bool failed=false;try{F::TryCall(m,pair.first,args,result);}catch(const std::invalid_argument&){failed=true;}
            Check(failed&&result==99&&m.reads==0&&m.writes==0&&m.calls==0,"Wrong-arity call had side effects");++rejected;
        }
    }
    for(uint32_t fn:{0x80024C84u,0x80024CF0u}){Memory m;int32_t result=44;Reject([&]{F::TryCall(m,fn,{0u},result);});Check(m.reads==0u&&m.writes==0u&&result==44,"Void call manufactured a result");}
    Memory m;int32_t result=85;Check(!F::TryCall(m,0x800F7770u,{},result)&&result==85,"Unknown call was claimed");
    std::cout<<"foreground-arity-rejected "<<rejected<<" void-scalar 2 unknown-unclaimed 1\n";
}
}
int main(){try{ReplyTests();LocationTests();ArityTests();std::cout<<"PASS foreground native services "<<assertions<<" assertions\n";return 0;}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 2;}}
