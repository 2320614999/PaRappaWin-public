#include "pr/pr_stage2_xa_device.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>
namespace {
uint64_t checks=0;
void Check(bool v,const char* m){++checks;if(!v)throw std::runtime_error(m);}
template<class F>void Reject(F f){bool rejected=false;try{f();}catch(const std::exception&){rejected=true;}Check(rejected,"Invalid XA input was accepted");}
void Save(std::ofstream& file,const std::vector<int16_t>& pcm){for(auto v:pcm){file.put(char(uint16_t(v)));file.put(char(uint16_t(v)>>8u));}Check(bool(file),"PCM evidence write failed");}
void Register(PrStage2SpuDevice::Device& s,uint32_t offset,uint16_t value){Check(s.TryWrite(0x1F801C00u+offset,2u,value),"SPU input volume register unavailable");}
}
int main(int argc,char** argv){
    try{
        Check(argc==4,"Arguments: optical-image fixture-directory output-directory");
        const auto out=std::filesystem::u8path(argv[3]);
        std::ifstream disc(std::filesystem::u8path(argv[1]),std::ios::binary);Check(bool(disc),"Original disc unavailable");
        std::ofstream decoded(out/"xa_decoded.bin",std::ios::binary),mixed(out/"xa_mixed.bin",std::ios::binary);
        PrStage2SpuDevice::Device spu;PrStage2XaDevice::Device xa(spu);
        Register(spu,0x180u,0x2000u);Register(spu,0x182u,0x2000u);
        Register(spu,0x1B0u,0x4000u);Register(spu,0x1B2u,0xC000u);Register(spu,0x1AAu,1u);
        std::array<uint8_t,2352> raw{},first{};uint32_t count=0;uint64_t samples=0;
        disc.seekg(uint64_t(241038)*2352u);
        for(uint32_t i=0;i<240u;++i){
            disc.read(reinterpret_cast<char*>(raw.data()),raw.size());Check(disc.gcount()==2352,"Truncated original sector");
            if(!(raw[18]&4u))continue;
            if(!count)first=raw;
            Check(xa.Consume(raw,241038u+i,false,{128u,0u,128u,0u}),"XA output backpressure with empty consumer");
            const auto& pcm=xa.LastPcm();Check(pcm.size()==9408u,"Actual mono sector sample count changed");Save(decoded,pcm);
            std::vector<int16_t> output(pcm.size());spu.Render(output.data(),uint32_t(pcm.size()/2u));Save(mixed,output);
            samples+=pcm.size();++count;
        }
        Check(count==15u&&xa.Sectors()==15u&&xa.Frames()==70560u,"Original XA sector sequence incomplete");
        Check(spu.CdFramesQueued()==70560u&&spu.CdFramesConsumed()==70560u&&spu.CdUnderflowFrames()==0u,"Source samples dropped, repeated or underrun");
        Check(spu.CdNonzeroSamples()>0u&&spu.NonzeroSamples()>0u,"CD was incorrectly muted by SPU voice control");
        // Malformed sectors fail before changing any decoder state or queue.
        for(uint8_t coding:{uint8_t(2),uint8_t(4),uint8_t(16),uint8_t(64)}){
            auto bad=first;bad[19]=bad[23]=coding;Reject([&]{xa.Consume(bad,0,false,{128,0,128,0});});
        }
        {auto bad=first;bad[20]^=1;Reject([&]{xa.Consume(bad,0,false,{128,0,128,0});});}
        {auto bad=first;bad[24]^=1;Reject([&]{xa.Consume(bad,0,false,{128,0,128,0});});}
        Check(xa.Sectors()==15u&&spu.CdFramesQueued()==70560u,"Rejected XA sector advanced audio state");
        Reject([&]{xa.Consume(first,0,false,{255,255,255,255});});
        // Full output queues do not partially accept a sector or advance its predictor.
        Check(spu.QueueCdAudio(std::vector<int16_t>(88200u,1234)),"Cannot fill empty CD queue");
        const auto before=spu.CdFramesQueued();Check(!xa.Consume(first,1,false,{128,0,128,0}),"Full CD queue silently dropped a sector");
        Check(xa.Sectors()==15u&&spu.CdFramesQueued()==before,"Backpressure consumed input");
        std::vector<int16_t> drain(88200);spu.Render(drain.data(),44100u);
        Check(xa.Consume(first,1,true,{128,0,128,0}),"Muted XA sector not consumed");
        Check(std::all_of(xa.LastPcm().begin(),xa.LastPcm().end(),[](int16_t x){return x==0;}),"CD mute ignored");
        spu.Render(drain.data(),4704u);
        Reject([&]{spu.QueueCdAudio({1});});
        // Exact equations with independent volume, stereo and voice-mute controls.
        std::ofstream equations(out/"mix_equations.txt");
        for(uint16_t control:{uint16_t(0),uint16_t(1),uint16_t(0x8001),uint16_t(0x4001),uint16_t(0xC001)})
        for(uint16_t cd:{uint16_t(0),uint16_t(0x4000),uint16_t(0x7FFF),uint16_t(0x8000),uint16_t(0xFFFF)})
        for(int16_t sample:{int16_t(-32768),int16_t(-12345),int16_t(0),int16_t(12345),int16_t(32767)}){
            PrStage2SpuDevice::Device s;Register(s,0x180,0x2000);Register(s,0x182,0x3FFF);Register(s,0x1B0,cd);Register(s,0x1B2,cd);Register(s,0x1AA,control);
            Check(s.QueueCdAudio({sample,int16_t(sample/2)}),"Equation input queue failed");int16_t result[2];s.Render(result,1);
            equations<<control<<' '<<cd<<' '<<sample<<' '<<sample/2<<' '<<result[0]<<' '<<result[1]<<'\n';
            const auto cap=s.ReadRam(0,2);Check(uint16_t(uint8_t(cap[0])|(uint16_t(cap[1])<<8u))==uint16_t(sample),"CD capture applied output volume or mute");
        }
        std::ifstream fixtures(std::filesystem::u8path(argv[2])/"synthetic_xa.bin",std::ios::binary);
        std::ofstream synthetic(out/"synthetic_pcm.bin",std::ios::binary);uint32_t cases=0;
        while(fixtures.read(reinterpret_cast<char*>(raw.data()),2352)){
            PrStage2XaDevice::Decoder one;auto pcm=one.Decode(raw);const uint32_t n=uint32_t(pcm.size());
            for(unsigned j=0;j<4;++j)synthetic.put(char(n>>(8u*j)));Save(synthetic,pcm);++cases;
        }
        Check(fixtures.eof()&&fixtures.gcount()==0&&cases>0,"Synthetic sectors not whole or missing");
        std::cout<<"xa-contract 15-original-mono-sectors 70560-stereo-frames "<<samples<<"-samples "<<cases<<"-synthetic-cases\n";
        std::cout<<"xa-queue-contract no-drop no-partial-sector mute-reject capture-clock\n";
        std::cout<<"xa-pass "<<checks<<" assertions\n";return 0;
    }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 2;}
}
