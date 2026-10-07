#include "pr/pr_stage2_xa_device.h"
#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <thread>
namespace {
using D=PrStage2CdCommandDevice::Device;
uint64_t checks=0;
void Check(bool value,const char* message){++checks;if(!value)throw std::runtime_error(message);}
template<class F>void Reject(F f){bool caught=false;try{f();}catch(const std::exception&){caught=true;}Check(caught,"Expected transport rejection missing");}
void Write(D& d,uint32_t port,uint8_t v){Check(d.TryWrite(0x1F801800u+port,1u,v),"CD port unavailable");}
uint32_t Read(D& d,uint32_t port){uint32_t v=0;Check(d.TryRead(0x1F801800u+port,1u,v),"CD port unavailable");return v;}
void Bank(D& d,uint8_t bank){Write(d,0,bank);}
void Ack(D& d){Bank(d,1);Write(d,3,7);}
void Command(D& d,uint8_t op,std::initializer_list<uint8_t> args={}){
    Bank(d,0);for(auto arg:args)Write(d,2,arg);Write(d,1,op);
    Check(d.Service()&&d.Flags()==3u,"Command did not execute with its separate response");Ack(d);
}
uint8_t Bcd(uint32_t n){return uint8_t((n/10u)*16u+n%10u);}
void Start(D& d,uint8_t file=1,uint8_t channel=1,uint8_t mode=0xC8){
    constexpr uint32_t absolute=241053u+150u;
    Command(d,2,{Bcd(absolute/4500u),Bcd(absolute/75u%60u),Bcd(absolute%75u)});
    Command(d,13,{file,channel});Command(d,14,{mode});Command(d,27);
}
void Advance(D& d){
    const auto until=std::chrono::steady_clock::now()+std::chrono::seconds(2);
    while(!d.Service()){Check(std::chrono::steady_clock::now()<until,"Real sector deadline did not progress");std::this_thread::sleep_for(std::chrono::milliseconds(1));}
}
void Matrix(D& d,std::array<uint8_t,4> v,bool apply){
    Bank(d,2);Write(d,2,v[0]);Write(d,3,v[1]);Bank(d,3);Write(d,1,v[2]);Write(d,2,v[3]);
    if(apply)Write(d,3,32);
}
struct Context {
    PrStage2InterruptDevice::Controller irq;
    PrStage2SpuDevice::Device spu;
    D cd;
    PrStage2XaDevice::Device xa;
    explicit Context(const std::filesystem::path& path):cd(path,irq),xa(spu){cd.BindXaConsumer(xa);}
};
}
int main(int argc,char** argv){
    try{
        Check(argc==3,"Arguments: optical-image output-directory");
        const auto path=std::filesystem::u8path(argv[1]),out=std::filesystem::u8path(argv[2]);
        std::array<uint8_t,2352> sector{};
        {std::ifstream f(path,std::ios::binary);f.seekg(uint64_t(241053)*2352u);f.read(reinterpret_cast<char*>(sector.data()),2352);Check(f.gcount()==2352,"Missing original XA source");}
        PrStage2XaDevice::Decoder reference;const auto decoded=reference.Decode(sector);
        for(uint32_t kind=0;kind<7u;++kind){
            Context c(path);
            if(kind==1u)Start(c.cd,1,2);
            else if(kind==2u)Start(c.cd,1,2,0xC0u);
            else Start(c.cd);
            if(kind==3u)Matrix(c.cd,{0,0,0,0},false);
            if(kind==4u)Matrix(c.cd,{0,0,0,0},true);
            if(kind==5u)Command(c.cd,11);
            if(kind==6u){Bank(c.cd,3);Write(c.cd,3,1);}
            const auto before=c.cd.SectorReads();
            std::this_thread::sleep_for(std::chrono::milliseconds(15));
            for(uint32_t i=0;i<64u;++i)Read(c.cd,0);
            Check(c.cd.SectorReads()==before&&c.xa.Sectors()==0u,"Plain MMIO observation consumed audio");
            Advance(c.cd);
            Check(c.cd.SectorReads()==1u&&c.cd.Location()==241054u&&c.cd.Flags()==0u&&c.cd.DmaBytesAvailable()==0u,
                  "XA generated a video IRQ/FIFO or advanced the disc more than once");
            if(kind==1u){
                Check(c.cd.FilteredAudioSectors()==1u&&c.xa.Sectors()==0u&&c.spu.CdFramesQueued()==0u,"Wrong-channel audio escaped source filter");
            }else{
                Check(c.cd.AudioSectors()==1u&&c.xa.Sectors()==1u&&c.spu.CdFramesQueued()==4704u,"Selected XA did not enter actual sample queue");
                if(kind>=4u)Check(std::all_of(c.xa.LastPcm().begin(),c.xa.LastPcm().end(),[](int16_t v){return v==0;}),"Applied matrix/mute did not silence audio");
                else Check(c.xa.LastPcm()==decoded,"Pending matrix changed audio before apply or default channel routing changed");
            }
            // Even with a mismatched XA filter, the following video sector is
            // independent and must still reach the original data-ready FIFO.
            Advance(c.cd);Check(c.cd.Flags()==1u&&c.cd.LastSector()==241054u&&c.cd.SectorReads()==2u,"Video after XA was filtered, dropped or duplicated");
            Ack(c.cd);Bank(c.cd,0);Write(c.cd,3,128);
            Check(c.cd.DmaBytesAvailable()==2048u,"Video FIFO has the wrong available payload");
            for(uint32_t i=0;i<512u;++i){uint32_t want=0;for(unsigned j=0;j<4;++j)want|=uint32_t(c.cd.RawSector()[24u+i*4u+j])<<(8u*j);Check(c.cd.ReadDmaWord()==want,"Video words changed after XA delivery");}
            c.cd.ReleaseConsumedSector();Check(c.cd.DmaBytesAvailable()==0u,"Consumed video sector not released");
            Reject([&]{c.cd.BindXaConsumer(c.xa);});
        }
        {
            Context c(path);Start(c.cd);Check(c.spu.QueueCdAudio(std::vector<int16_t>(88200u,321)),"Cannot establish real bounded backpressure");
            Advance(c.cd);Check(c.cd.SectorReads()==1u&&c.cd.AudioSectors()==0u&&c.xa.Sectors()==0u,"Full queue dropped or partially accepted XA");
            const auto serial=c.cd.Serial();std::this_thread::sleep_for(std::chrono::milliseconds(20));
            for(unsigned i=0;i<20;++i)Check(!c.cd.Service()&&c.cd.Serial()==serial&&c.cd.SectorReads()==1u&&c.cd.Flags()==0u,"Blocked audio reread source or published a fake event");
            std::vector<int16_t> buffer(88200u);c.spu.Render(buffer.data(),44100u);
            Check(c.cd.Service()&&c.cd.AudioSectors()==1u&&c.cd.SectorReads()==1u&&c.xa.LastPcm()==decoded,"Retry did not consume the same pending audio once");
            Check(c.cd.Flags()==0u&&c.spu.CdFramesQueued()==48804u,"Backpressure fabricated CD interrupt or dropped sample frames");
        }
        {
            PrStage2InterruptDevice::Controller irq;D d(path,irq);Start(d);std::this_thread::sleep_for(std::chrono::milliseconds(15));
            Reject([&]{d.Service();});Check(d.AudioSectors()==0u&&d.Flags()==0u,"Unbound XA adapter silently succeeded");
        }
        std::cout<<"xa-transport 7-filter-matrix-mute 1-bounded-retry 1-unbound 3584-video-words no-audio-int1 no-read-pump\n";
        std::cout<<"xa-transport-pass "<<checks<<" assertions\n";return 0;
    }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 2;}
}
