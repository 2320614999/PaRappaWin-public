// Reuse the already-verified native memory fixture; this test adds no device
// emulator or codec. Original source functions remain linked unchanged.
#define main ExistingForegroundServicesMain
#include "test_stage2_foreground_services.cpp"
#undef main
#include "pr/pr_stage2_cd_command_device.h"
#include <chrono>
#include <fstream>
#include <thread>

namespace {
namespace File=PrStage2CdCommandDevice;
constexpr uint32_t Ring=0x800B0000u;
std::array<uint8_t,2352> Header(uint32_t part,uint32_t count){
    std::array<uint8_t,2352> raw{};raw[18]=2u;raw[24]=0x60u;raw[25]=1u;
    raw[28]=uint8_t(part);raw[29]=uint8_t(part>>8u);
    raw[30]=uint8_t(count);raw[31]=uint8_t(count>>8u);return raw;
}
void ReceiverTests(){
    uint32_t scenarios=0;
    // 原版停止时撤销 STR 回调；后续音频流的数据不再访问已释放 ring。
    {
        Memory m;m.Put(0x800570FCu,4u,0u);
        const std::array<uint8_t,2352> data{};
        Check(F::CanAcceptHostStreamSector(m,data),"Stopped movie still validates STR input");
        Check(m.data.size()==4u&&m.writes==0u&&m.calls==0u,"Stopped movie accessed retired ring");
        m.Put(0x800570FCu,4u,0x80039240u);
        Reject([&]{F::CanAcceptHostStreamSector(m,data);});
        m.Put(0x80092858u,4u,1u);
        Check(!F::CanAcceptHostStreamSector(m,Header(0,10)),"Active movie bypassed backpressure");
    }
    for(uint32_t index=0;index<32u;++index)for(uint32_t count=1;count<32u;++count)
    for(bool continuation:{false,true})for(bool occupied:{false,true})for(bool wrapOccupied:{false,true}){
        if(continuation&&count==1u)continue;
        Memory m;m.Put(0x80092858u,4u,0u);m.Put(0x801C3868u,4u,32u);
        m.Put(0x80095C50u,4u,index);m.Put(0x800965ACu,4u,Ring);m.Put(0x8009659Cu,4u,0xFFFFFFFFu);
        m.Put(Ring,2u,wrapOccupied?2u:0u);m.Put(Ring+32u*index,2u,occupied?4u:0u);
        const auto before=m.data;const auto raw=Header(continuation?1u:0u,count);
        const bool wraps=!continuation&&count>31u-index;
        const bool baseBusy=index==0u?occupied:wrapOccupied;
        const bool expected=!occupied&&!(wraps&&baseBusy);
        Check(F::CanAcceptHostVideoSector(m,raw)==expected,"Receiver capacity/wrap ownership differs");
        Check(m.data==before&&m.writes==0u&&m.calls==0u&&m.waits==0u,"Readiness query mutated source state or time");
        ++scenarios;
    }
    {
        Memory m;m.Put(0x80092858u,4u,1u);
        Check(!F::CanAcceptHostVideoSector(m,Header(0,10)),"Unfinished original frame accepted new input");
        Check(m.writes==0u&&m.calls==0u&&m.data.size()==4u,"Busy query touched unrelated uninitialized memory");
    }
    for(uint32_t bad=0;bad<7u;++bad){
        Memory m;m.Put(0x80092858u,4u,0u);m.Put(0x801C3868u,4u,bad==0?0u:32u);
        m.Put(0x80095C50u,4u,bad==1?32u:0u);m.Put(0x800965ACu,4u,Ring);
        auto raw=Header(bad==2?10u:0u,bad==3?0u:bad==4?32u:10u);
        if(bad==5)raw[24]=0;if(bad==6)raw[18]=4u;
        const auto old=m.data;Reject([&]{F::CanAcceptHostVideoSector(m,raw);});
        Check(m.data==old&&m.writes==0u&&m.calls==0u,"Invalid receiver input was committed");
    }
    std::cout<<"video-receiver-capacity scenarios="<<scenarios<<" malformed=7 busy=1 no-source-write\n";
}
void Write(File::Device& d,uint32_t offset,uint8_t value){Check(d.TryWrite(0x1F801800u+offset,1u,value),"File SDK register not recognized");}
void Ack(File::Device& d){Write(d,0,1);Write(d,3,31u);Write(d,0,0);}
void Command(File::Device& d,uint8_t command,std::initializer_list<uint8_t> parameters,uint32_t replies=1u){
    Write(d,0,0);for(auto v:parameters)Write(d,2,v);Write(d,1,command);
    uint32_t observed=0;
    for(uint32_t i=0;i<16u&&observed<replies;++i){
        d.Service(false);
        if(d.Flags()) {Check(d.Flags()!=5u,"Actual SDK command failed");Ack(d);++observed;}
    }
    Check(observed==replies,"Held video blocked an accepted command response");
}
void Start(File::Device& d,uint32_t lba){
    const auto bcd=[](uint32_t n){return uint8_t((n/10u)*16u+n%10u);};
    const uint32_t absolute=lba+150u;
    Command(d,10u,{},2u);
    Command(d,2u,{bcd(absolute/4500u),bcd(absolute/75u%60u),bcd(absolute%75u)});
    Command(d,14u,{0x80u});Command(d,27u,{});
}
void Due(File::Device& d){
    const auto deadline=std::chrono::steady_clock::now()+std::chrono::seconds(2);
    while(!d.SectorReads()){
        Check(std::chrono::steady_clock::now()<deadline,"Actual file producer did not read its due sector");
        d.Service(true);std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}
void FileTests(const std::filesystem::path& disc){
    constexpr uint32_t lba=241068u;
    std::ifstream source(disc,std::ios::binary);Check(bool(source),"Original optical file missing");
    source.seekg(uint64_t(lba)*2352u);std::array<uint8_t,2352> expected{};
    source.read(reinterpret_cast<char*>(expected.data()),expected.size());Check(source.gcount()==2352,"Original sector truncated");
    for(uint32_t mode=0;mode<4u;++mode){
        PrStage2InterruptDevice::Controller irq;File::Device file(disc,irq);
        bool ready=false,throwing=false;uint32_t checks=0;
        file.BindVideoReadiness([&](const auto& raw){
            Check(raw==expected,"Pending data was replaced by a different file sector");++checks;
            if(throwing)throw std::runtime_error("Injected native receiver failure");
            return ready;
        });
        Reject([&]{file.BindVideoReadiness([](const auto&){return true;});});
        Start(file,lba);Due(file);
        Check(file.VideoDeliveryPending()&&file.Flags()==0u&&file.DmaBytesAvailable()==0u,
              "Unreadable receiver was given a success event or exposed bytes");
        const auto serial=file.Serial(),responses=file.Responses(),reads=file.SectorReads();
        for(uint32_t i=0;i<128u;++i){
            uint32_t value=0;file.TryRead(0x1F801800u,1u,value);
            Check(!file.Service(true)&&file.VideoDeliveryPending(),"Blocked receiver unexpectedly completed");
            Check(file.Serial()==serial&&file.SectorReads()==reads&&file.Responses()==responses&&file.Location()==lba+1u,
                  "Blocked receiver read ahead, acknowledged work or advanced location");
        }
        Check(file.VideoDeferrals()>=129u&&checks>=129u,"Readiness predicate was bypassed");
        Command(file,1u,{});
        Check(file.VideoDeliveryPending()&&file.SectorReads()==1u,"Status command discarded pending file data");
        if(mode==1u){
            Command(file,9u,{},2u);ready=true;
            Check(!file.Reading()&&!file.VideoDeliveryPending()&&!file.Service(true)&&file.Flags()==0u,
                  "Cancelled pending frame was later published");
            continue;
        }
        if(mode==2u){
            throwing=true;Reject([&]{file.Service(true);});
            Check(file.VideoDeliveryPending()&&file.Flags()==0u&&file.DmaBytesAvailable()==0u&&file.SectorReads()==1u,
                  "Receiver exception manufactured success or lost actual data");
            throwing=false;
        }
        ready=true;
        Check(!file.Service(false)&&file.VideoDeliveryPending()&&file.Flags()==0u,"Closed host delivery gate was ignored");
        Check(file.Service(true)&&file.Flags()==1u&&!file.VideoDeliveryPending()&&file.SectorReads()==1u,
              "Reopened receiver failed to deliver exactly the held sector");
        const auto published=file.Responses();Ack(file);Write(file,3,128u);
        Check(file.DmaBytesAvailable()==2048u,"Reopened receiver exposed an invented payload size");
        for(uint32_t i=0;i<512u;++i){
            const uint32_t word=file.ReadDmaWord();
            for(uint32_t b=0;b<4u;++b)Check(uint8_t(word>>(8u*b))==expected[24u+4u*i+b],"Actual retained file bytes were corrupted");
        }
        file.ReleaseConsumedSector();
        Check(!file.Service(false)&&file.Responses()==published&&file.SectorReads()==1u&&file.Flags()==0u,
              "Completed retained sector was delivered twice");
    }
    std::cout<<"video-file-backpressure retained=4 exact-payloads=3 command-responses=4 cancellation=1 exception=1\n";
}
}
int main(int argc,char** argv){
    try{
        Check(argc==2,"Arguments: original optical image");
        ReceiverTests();FileTests(std::filesystem::u8path(argv[1]));
        std::cout<<"PASS native movie backpressure "<<assertions<<" assertions\n";return 0;
    }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 2;}
}
