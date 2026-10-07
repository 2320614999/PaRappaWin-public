#include "pr/pr_stage2_scene_entry.h"
#include "pr/pr_stage2_disc_file_device.h"
#include "pr/pr_stage2_vram_device.h"
#include "pr/pr_stage2_interrupt_device.h"
#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <vector>

namespace {
namespace E=PrStage2SceneEntry;
struct Missing : std::runtime_error {
    uint32_t function;std::vector<uint32_t> args;
    Missing(uint32_t f,E::Arguments a):std::runtime_error("unbound S2 platform call"),function(f),args(a){}
};
void Check(bool value,const char* message) {if(!value)throw std::runtime_error(message);}
template<class F> void Reject(F&& fn) {
    bool rejected=false;try{fn();}catch(const std::exception&){rejected=true;}
    Check(rejected,"expected rejection");
}
struct Session final:E::Services {
    PrStage2DiscFileDevice::Device disc;
    PrStage2VramDevice::Device vram;
    PrPsxGteDirect::MatrixRegisters gte;
    std::vector<uint32_t> external;
    PrStage2InterruptDevice::Controller irq;
    bool biosFixture = false;
    uint32_t firstReadAddress = 0u;
    uint32_t timerMode = 0u;
    Session(const std::filesystem::path& scus,const std::filesystem::path& overlay,const std::filesystem::path& bin)
        :E::Services(scus,overlay),disc(bin){}
    int32_t CallPlatform(uint32_t fn,E::Arguments args) override {
        external.push_back(fn);
        if(biosFixture) {
            const std::vector<uint32_t> a(args);
            if(fn==0x80047F5Cu && a==std::vector<uint32_t>{0x80055FB0u})return 0;
            if(fn==0x80048A30u && a==std::vector<uint32_t>{0x80055FB0u}) {
                Check(Read32(0x80055FB4u)==0x80056F90u,"original context publication");return -73;
            }
            if(fn==0x80048AE0u && a==std::vector<uint32_t>{0u})return -77;
            if(fn==0x80048AF0u && a==std::vector<uint32_t>{3u,0u})return -11;
            if(fn==0x80048960u && a.empty())return -99;
            if(fn==0x80048A50u && a.empty())return -17;
            throw Missing(fn,args);
        }
        if(fn==0x800381F8u && args.size()==2u) return disc.Lookup(*this,args.begin()[0],args.begin()[1]);
        if(fn==0x80038FC0u && args.size()==3u) {
            Check(firstReadAddress==0u && args.begin()[0]==4u && args.begin()[2]==128u,
                  "unexpected first S2 INT read");
            firstReadAddress=args.begin()[1];
            return disc.StartRead(*this,args.begin()[0],args.begin()[1],args.begin()[2]);
        }
        // Outside the explicitly enabled boot fixture: no reset, asset-load,
        // timing, scoring, BIOS or audio success receipts.
        throw Missing(fn,args);
    }
    void CallPlatformVoid(uint32_t fn,E::Arguments args) override {throw Missing(fn,args);}
    uint8_t ReadDevice8(uint32_t) override {throw std::runtime_error("unbound MMIO8");}
    uint16_t ReadDevice16(uint32_t a) override {
        uint32_t value;if(irq.TryRead(a,2u,value))return static_cast<uint16_t>(value);
        throw std::runtime_error("unbound MMIO16");
    }
    uint32_t ReadDevice32(uint32_t a) override {
        uint32_t value;if(irq.TryRead(a,4u,value))return value;
        throw std::runtime_error("unbound MMIO32");
    }
    void WriteDevice8(uint32_t,uint8_t) override {throw std::runtime_error("unbound MMIO8");}
    void WriteDevice16(uint32_t a,uint16_t v) override {
        if(irq.TryWrite(a,2u,v))return;throw std::runtime_error("unbound MMIO16");
    }
    void WriteDevice32(uint32_t a,uint32_t v) override {
        if(irq.TryWrite(a,4u,v))return;
        if(biosFixture && a==0x1F801114u){timerMode=v;return;}
        throw std::runtime_error("unbound MMIO32");
    }
    void InitializeBootFixture() {
        // Explicit platform prerequisite ONLY for this isolated startup test.
        // Run the original initializer rather than seed setter slots or flags.
        // These six lower BIOS inputs do not claim a production BIOS binding.
        Check(external.empty(),"fixture must precede scene device calls");
        biosFixture=true;
        try {Check(uint32_t(Call(0x80035744u,{}))==0x80055F78u,"callback boot failed");}
        catch(...){biosFixture=false;throw;}
        biosFixture=false;
        Check(external==std::vector<uint32_t>{0x80047F5Cu,0x80048A30u,0x80048AE0u,
              0x80048AF0u,0x80048960u,0x80048A50u},"unexpected BIOS prerequisite");
        Check(timerMode==0x107u && Read32(0x80056FF4u)==0x80035F24u,"callback setup missing");
        std::cout<<"s2-boot-prerequisite 6 explicit-bios-inputs\n";
        external.clear();
    }
    PrStage2LifecycleDirect::Words64 Call64(uint32_t f,E::Arguments a) override {throw Missing(f,a);}
    PrStage2LifecycleDirect::Vector32 NormalizeVector8003A3DC(PrStage2LifecycleDirect::Vector32) override {throw Missing(0x8003A3DCu,{});}
    int32_t SetCdLocation800367A4(uint32_t loc) override {return disc.SetLocation(loc);}
    int32_t LoadImage80044D64(PrStage2LifecycleDirect::ImageRect rect,uint32_t src) override {return vram.UploadImageWords(*this,rect,src);}
    PrPsxGteDirect::MatrixRegisters& MatrixGte() override {return gte;}
    [[noreturn]] void Exit(uint32_t f,E::Arguments a) override {throw Missing(f,a);}
    [[noreturn]] void Break(uint32_t f,uint32_t code) override {throw Missing(f,{code});}
};
std::vector<uint8_t> Read(const std::filesystem::path& path) {
    std::ifstream file(path,std::ios::binary);Check(bool(file),"test source file missing");
    return std::vector<uint8_t>((std::istreambuf_iterator<char>(file)),{});
}
uint32_t Word(const std::vector<uint8_t>& bytes,size_t at) {
    return uint32_t(bytes.at(at))|(uint32_t(bytes.at(at+1))<<8u)|
        (uint32_t(bytes.at(at+2))<<16u)|(uint32_t(bytes.at(at+3))<<24u);
}
void Inspect(Session& s,const std::vector<uint8_t>& image,const std::vector<uint8_t>& scus,
             const std::vector<uint8_t>& resources) {
    Check(image.size()==E::OverlayBytes,"test image length");
    for(uint32_t i=0;i<image.size();++i) Check(s.Read8(E::OverlayBase+i)==image[i],"overlay data mismatch");
    Check(s.Read8(E::OverlayBase-1u)==0u,"SCUS/overlay boundary modified");
    Reject([&]{s.Read8(E::OverlayBase+E::OverlayBytes);});
    Reject([&]{s.Read32(0x800000A0u);});
    Reject([&]{s.InitializeScene();});Reject([&]{s.RunScene();});
    Check(s.GetPhase()==E::Phase::Loaded&&s.external.empty(),"bad order changed state");
    Check(s.InitializeGlobals()==2&&s.GetPhase()==E::Phase::GlobalsReturned,"Fn0 not executed");
    Check(s.external.empty(),"Fn0 used platform receipt");
    Check(s.Read32(0x800943D4u)==38u&&s.Read32(0x800943DCu)==13u&&s.Read32(0x800943E8u)==90u,"wrong S2 table counts");
    Check(s.Read32(0x80094430u)==0x801C85CCu&&s.Read32(0x80094434u)==0x801C9644u&&
        s.Read32(0x80094438u)==0x801C9728u&&s.Read32(0x8009443Cu)==0x801C9730u&&
        s.Read32(0x80094440u)==0x801C7958u,"wrong S2 callbacks");
    Reject([&]{s.InitializeGlobals();});
    std::cout<<"s2-image "<<image.size()<<" globals 38 13 90\n";
    // Exercise the actual S2 callback table, not a copied Stage1/demo decoder.
    Check(s.Call(s.Read32(0x80094440u),{})==90,"S2 demo callback result");
    for(uint32_t i=0;i<90u;++i) {
        const size_t at=0x801D2010u-E::OverlayBase+4u*i;
        const int16_t tick=static_cast<int16_t>(image.at(at)|(uint16_t(image.at(at+1))<<8u));
        const int16_t key=static_cast<int16_t>(image.at(at+2)|(uint16_t(image.at(at+3))<<8u));
        const uint32_t expected=key>=1&&key<=8?Word(scus,0x80054168u+4u*uint32_t(key)-0x80010000u+0x800u):0u;
        Check(s.Read32(0x8008EEF8u+8u*i)==24u*uint32_t(int32_t(tick))&&s.Read32(0x8008EEFCu+8u*i)==expected,"S2 demo event differs from original data");
    }
    Check(s.external.empty(),"demo used host success receipt");
    std::cout<<"s2-demo 90 original-events\n";
    s.InitializeBootFixture();
    // Run the real initializer continuously until its first unbound callee.
    // Stopping here is expected coverage of a remaining gap, NOT success at Fn1.
    bool stopped=false;
    try{s.InitializeScene();}
    catch(const Missing& e) {
        Check(e.function==0x80035560u&&e.args==std::vector<uint32_t>{3u},"unexpected initialization wait boundary");
        stopped=true;
    }
    Check(stopped&&s.GetPhase()==E::Phase::Failed,"incomplete initialization reported success");
    Check(s.external.size()==9u&&s.disc.LookupRequests()==7u,"unexpected original disc call sequence");
    for(size_t i=0;i<7u;++i) Check(s.external[i]==0x800381F8u,"initializer call ordering");
    Check(s.external[7]==0x80038FC0u && s.external[8]==0x80035560u,"INT transfer/wait ordering");
    Check(s.disc.ReadRequests()==1u && s.disc.BytesTransferred()==8192u && resources.size()>=8192u,
          "S2 INT header was not actually transferred");
    for(uint32_t i=0;i<8192u;++i)
        Check(s.Read8(s.firstReadAddress+i)==resources[i],"first INT header differs from COMPO02");
    Check(s.Read32(0x80057014u)==0x8001537Cu && s.Read16(0x800916E0u)==2u,
          "loading owner not registered by original caller");
    std::cout<<"s2-int-header 8192 bytes-from-disc wait-not-bypassed\n";
    Check(s.Read32(0x8006EDB8u)==E::SceneRecord,"scene record not published by original initializer");
    Check(s.Read32(0x801C36D4u)==0x801CDAE4u&&s.Read32(0x801C36D8u)==0x801CDAF8u,"event reset not bound to S2 rows");
    for(uint32_t i=0;i<7u;++i) {
        const uint32_t record=E::SceneRecord+12u+48u*i;
        Check(s.Read32(record+12u)==1u&&s.Read32(record+20u)>0u,"original file was not resolved");
        std::cout<<"s2-file "<<i<<' '<<s.Read32(record+20u)<<' '<<s.Read32(record+40u)<<'\n';
    }
    std::cout<<"s2-scene-fields "<<s.Read32(E::SceneRecord+348u)<<' '<<s.Read32(E::SceneRecord+352u)<<' '
        <<s.Read32(E::SceneRecord+356u)<<' '<<s.Read32(E::SceneRecord+360u)<<'\n';
    std::cout<<"s2-initializer-blocked 80035560 3\n";
    Reject([&]{s.RunScene();});Reject([&]{s.InitializeScene();});
    // The registered Loading callback now routes to native control flow.
    // Drawing is deliberately unbound here: no fake completed-frame flag.
    stopped=false;
    try{s.Call(s.Read32(0x80057014u),{});}
    catch(const Missing& e){Check(e.function==0x8001EA74u&&e.args==std::vector<uint32_t>{1u,0u},"loading callback escaped native route");stopped=true;}
    Check(stopped&&s.Read32(0x8006ECD4u)==0u,"unrendered loading frame was acknowledged");
    std::cout<<"s2-loading-frame-native blocked-at-draw-no-ack\n";
    const auto beforeText=s.external.size();
    Check(s.Call(0x801CB244u,{})==12&&s.external.size()==beforeText,"S2 text setup used platform receipts");
    Check(s.Read16(0x8006ED38u)==28u&&s.Read16(0x8006ED3Cu)==189u&&
          s.Read16(0x8007CEE0u)==272u&&s.Read16(0x8007CEE2u)==480u,"S2 text rectangle");
    Check(s.Read32(0x8006ED40u)==832u&&s.Read32(0x8006ED44u)==256u&&s.Read32(0x8006ED48u)==20u&&
          s.Read16(0x8007CED8u)==12u&&s.Read16(0x8007CEDAu)==12u,"S2 text texture layout");
    Check(s.Read32(0x8007CED0u)==s.Read32(0x8004E690u),"S2 text primitive template not loaded");
    std::cout<<"s2-text-setup 28 189 272 480 832 256 20 12 native\n";
    // Direct root reachability only; the app must not bypass the phase guard.
    stopped=false;
    try{s.Call(0x801C74E4u,{2u});}
    catch(const Missing& e){
        if(e.function!=0x800201ACu)std::cerr<<"observed-main-boundary "<<std::hex<<e.function<<std::dec<<'\n';
        Check(e.function==0x800201ACu&&e.args==std::vector<uint32_t>{0x801C3640u,6u,2u,1u},"unexpected scene root boundary");stopped=true;}
    Check(stopped,"main root falsely completed");
    std::cout<<"s2-main-boundary 800201ac\n";
    const auto external=s.external.size();int32_t sentinel=123;
    Check(!E::TryCall(s,0x801FFFF0u,{},sentinel)&&sentinel==123,"unknown route modified result");
    Reject([&]{s.Call(0x801C9728u,{});});
    Check(external==s.external.size(),"unknown overlay target escaped to legacy/platform");
}
}
int main(int argc,char** argv) {
    try {
        if(argc!=4&&argc!=5)return 1;
        const auto scus=std::filesystem::u8path(argv[1]);const auto overlay=std::filesystem::u8path(argv[2]);const auto bin=std::filesystem::u8path(argv[3]);
        if(argc==5&&std::string(argv[4])=="--expect-image-reject") {
            Reject([&]{Session s(scus,overlay,bin);});std::cout<<"s2-image-rejected\n";return 0;
        }
        // Cold-only startup must not silently manufacture BIOS initialization.
        {
            Session cold(scus,overlay,bin);cold.InitializeGlobals();bool blocked=false;
            try{cold.InitializeScene();}
            catch(const Missing& error){blocked=error.function==0u&&error.args==std::vector<uint32_t>{0u,0x8001537Cu};}
            Check(blocked && cold.GetPhase()==E::Phase::Failed && cold.disc.ReadRequests()==0u,
                  "cold platform prerequisite was silently bypassed");
            std::cout<<"s2-cold-platform-unbound no-fake-callback\n";
        }
        Session s(scus,overlay,bin);Inspect(s,Read(overlay),Read(scus),Read(overlay.parent_path()/"COMPO02.INT"));
        std::cout<<"s2-entry-contract-pass gameplay-not-ready\n";return 0;
    }catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 2;}
}