#include "pr/pr_stage2_spu_device.h"
#include "pr/pr_stage2_spu_output.h"
#include "wasapi_sink.h"
#include <algorithm>
#include <chrono>
#include <iostream>
#include <stdexcept>
#include <thread>
#include <vector>

namespace {
using namespace PrStage2SpuDevice;
uint32_t checks=0;
void Check(bool value,const char* message){++checks;if(!value)throw std::runtime_error(message);}
template<class F>void Reject(F fn){bool rejected=false;try{fn();}catch(const std::exception&){rejected=true;}Check(rejected,"Expected SPU rejection");}
void Write(Device& d,uint32_t off,uint16_t v){Check(d.TryWrite(0x1F801C00u+off,2u,v),"SPU write not claimed");}
uint32_t Read(Device& d,uint32_t off){uint32_t value=0;Check(d.TryRead(0x1F801C00u+off,2u,value),"SPU read not claimed");return value;}
void Tone(Device& d){
    std::vector<uint8_t> block(16,0u);block[0]=5u;block[1]=7u;
    const int wave[28]={0,2,3,4,5,6,7,7,7,6,5,4,3,2,0,-2,-3,-4,-5,-6,-7,-7,-7,-6,-5,-4,-3,-2};
    for(uint32_t n=0;n<28u;++n)block[2u+n/2u]|=uint8_t((uint32_t(wave[n])&15u)<<(4u*(n&1u)));
    d.Upload(0x1000u,block);
    Write(d,0,0x1FFF);Write(d,2,0x1FFF);Write(d,4,0x1000);Write(d,6,0x200);
    Write(d,8,0x00FF);Write(d,10,0x1FC0);Write(d,0x180,0x3FFF);Write(d,0x182,0x3FFF);
    Write(d,0x1AA,0xC000);Write(d,0x188,1u);
}
void Contract(){
    Device d;
    uint32_t unchanged=17;
    Check(!d.TryRead(0x1F801814,4,unchanged)&&unchanged==17,"SPU claimed GPU address");
    Reject([&]{d.TryWrite(0x1F801D88,4,1);});
    Reject([&]{d.TryRead(0x1F801C01,2,unchanged);});
    Reject([&]{d.TryWrite(0x1F801C00,2,0x8000);});
    Reject([&]{d.TryWrite(0x1F801DAA,2,0xC080);});
    const auto writes=d.RegisterWrites();
    Reject([&]{d.TryWrite(0x1F801D88,2,1);});
    Check(d.RegisterWrites()==writes&&d.ActiveMask()==0,"Failed key-on published voice");
    Reject([&]{d.ReadRam(0x1000,16);});
    Reject([&]{d.Upload(Device::RamBytes-1,std::vector<uint8_t>(2));});
    Tone(d);
    Check(d.ActiveMask()==1u,"Key-on did not activate decoder");
    const auto before=d.RenderedFrames();
    for(uint32_t n=0;n<100u;++n){Check(Read(d,12)==0u,"Read advanced ADSR");}
    Check(d.RenderedFrames()==before,"MMIO read pumped PCM");
    std::vector<int16_t> pcm(1024);
    d.Render(pcm.data(),512);
    if(d.RenderedFrames()!=512||d.NonzeroSamples()<=700){std::cerr<<"pcm-observation frames="<<d.RenderedFrames()<<" nonzero="<<d.NonzeroSamples()<<" envelope="<<Read(d,12)<<" active="<<d.ActiveMask()<<" samples=";for(size_t n=0;n<64u;++n)std::cerr<<pcm[n]<<',';std::cerr<<'\n';}
    Check(d.RenderedFrames()==512&&d.NonzeroSamples()>700,"Actual PCM was not generated");
    Check((Read(d,0x19C)&1u)==1u&&Read(d,14)==0x200u,"ADPCM loop/ENDX missing");
    Check(d.ReadRam(0x1000,16).size()==16&&d.UploadedBytes()==16,"Actual SPU memory transfer missing");
    Write(d,0x18C,0);Check(d.ActiveMask()==1,"Zero key-off stopped a voice");
    Write(d,0x18C,1);d.Render(pcm.data(),8);Check(d.ActiveMask()==0&&Read(d,12)==0,"Key-off did not run release");
    Write(d,0x188,1);Check((Read(d,0x19C)&1u)==0,"Retrigger did not clear ENDX");
    Write(d,0x198,0x1234);Write(d,0x19A,0x56);Check(Read(d,0x198)==0x1234,"Reverb send mask not stored");
    Reject([&]{Write(d,0x194,1);});Reject([&]{Write(d,0x190,2);});
    Write(d,0x1AA,0);Check(d.ActiveMask()==0,"Disable retained audible voice");
    d.Render(pcm.data(),512);Check(std::all_of(pcm.begin(),pcm.end(),[](int16_t n){return n==0;}),"Disabled core not silent");
    for(uint32_t alias:{0u,0x80000000u,0xA0000000u}){
        Check(d.TryWrite(0x1F801D98u|alias,2,0x5AA5),"SPU alias write");
        uint32_t v=0;Check(d.TryRead(0x1F801D98u|alias,2,v)&&v==0x5AA5,"SPU alias read");
    }
    Device one;Tone(one);auto block=one.ReadRam(0x1000,16);block[1]=1;one.Upload(0x1000,block);
    one.Render(pcm.data(),80);Check(one.ActiveMask()==0&&(Read(one,0x19C)&1u),"One-shot did not stop at ADPCM end");
    std::cout<<"spu-contract "<<checks<<" passed\n";
}
struct FakeSink final:IAudioSink{
    AudioRenderCallback callback=nullptr;void* user=nullptr;bool fail=false,running=false;
    bool InitializeSink(const AudioSinkConfig& c,AudioRenderCallback cb,void* u)override{
        Check(c.sampleRate==44100&&c.channels==2,"SPU sample rate changed with display");callback=cb;user=u;return running=!fail;}
    void ShutdownSink()override{running=false;}
    bool IsRunning()const override{return running;}double GetPlayedSeconds()const override{return 0;}
    uint32_t GetSampleRate()const override{return 44100;}uint32_t GetChannels()const override{return 2;}
    void SetVolume(float)override{}float GetVolume()const override{return 1;}
};
void StreamChecks(){
    Device d;FakeSink sink;
    {PrStage2SpuOutput::Stream stream(d,sink);sink.fail=true;Reject([&]{stream.Start();});Check(!sink.running,"Failed sink remained active");}
    sink.fail=false;
    {PrStage2SpuOutput::Stream stream(d,sink);stream.Start();std::vector<int16_t> data(64,123);
        sink.callback(sink.user,data.data(),32,2);stream.RethrowFailure();Check(d.RenderedFrames()==32,"Sink did not call actual core");
        sink.callback(sink.user,data.data(),8,1);Reject([&]{stream.RethrowFailure();});
        Check(std::all_of(data.begin(),data.begin()+8,[](int16_t n){return n==0;}),"Faulted callback did not silence block");}
    Check(!sink.running,"Stream destructor did not stop its sink");
    std::cout<<"spu-stream-contract passed\n";
}
void Wasapi(){
    Device d;Tone(d);WasapiSink sink;PrStage2SpuOutput::Stream stream(d,sink);
    sink.SetVolume(0.02f);stream.Start();
    const auto deadline=std::chrono::steady_clock::now()+std::chrono::seconds(4);
    while(d.RenderedFrames()<4410u&&std::chrono::steady_clock::now()<deadline){stream.RethrowFailure();std::this_thread::sleep_for(std::chrono::milliseconds(2));}
    stream.RethrowFailure();Check(stream.Running(),"Actual WASAPI endpoint not running");
    Check(d.RenderedFrames()>=4410u&&d.NonzeroSamples()>4000u&&stream.SinkSeconds()>0,"Actual WASAPI did not consume SPU PCM");
    stream.Stop();std::cout<<"spu-wasapi "<<d.RenderedFrames()<<' '<<d.NonzeroSamples()<<" actual-callback-samples\n";
}
}
int main(int argc,char**argv){
    try{
        if(argc==2&&std::string(argv[1])=="--vectors"){
            char kind;
            while(std::cin>>kind){
                if(kind=='D'){
                    int32_t p,q;std::cin>>p>>q;std::array<uint8_t,16>b;for(auto&v:b){uint32_t n;std::cin>>n;v=uint8_t(n);}
                    const auto data=PrStage2SpuDevice::DecodeBlock(b,p,q);for(auto n:data)std::cout<<n<<' ';std::cout<<p<<' '<<q<<'\n';
                }else if(kind=='G'){
                    std::array<int16_t,4>h;for(auto&n:h){int32_t v;std::cin>>v;n=int16_t(v);}uint32_t p;std::cin>>p;std::cout<<PrStage2SpuDevice::Interpolate(h,p)<<'\n';
                }else if(kind=='E'){
                    uint32_t phase,a,b,count;PrStage2SpuDevice::Envelope e;
                    std::cin>>phase>>e.level>>e.counter>>a>>b>>count;e.phase=static_cast<PrStage2SpuDevice::Phase>(phase);
                    for(uint32_t n=0;n<count;++n){PrStage2SpuDevice::StepEnvelope(e,uint16_t(a),uint16_t(b));std::cout<<static_cast<uint32_t>(e.phase)<<' '<<e.level<<' '<<e.counter<<' ';}std::cout<<'\n';
                }else throw std::invalid_argument("Unknown vector input");
            }
        }else if(argc==2&&std::string(argv[1])=="--wasapi")Wasapi();
        else{Contract();StreamChecks();}
        return 0;
    }catch(const std::exception&e){std::cerr<<e.what()<<'\n';return 2;}
}
