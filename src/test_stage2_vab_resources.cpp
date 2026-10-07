#include "pr/pr_stage2_vab_session.h"
#include "test_stage2_gpu_image_readback.h"
#include "pr/pr_stage2_tim_direct.h"
#include "pr/pr_stage2_gpu_direct.h"
#include "wasapi_sink.h"
#include <chrono>
#include <fstream>
#include <iostream>
#include <iterator>
#include <thread>
#include <vector>
#include <windows.h>

namespace {
namespace D=PrStage2LoadingDeviceSession;
namespace F=PrStage2FrameTask;
void Check(bool value,const char* message){if(!value)throw std::runtime_error(message);}
struct TestSession final:PrStage2VabSession::Session{
    using Session::Session;
    bool coldPrerequisites=true;
    uint64_t isolatedCalls=0;
    int32_t RemainingDependency(uint32_t fn,std::initializer_list<uint32_t> args)override{
        // Explicit test-only BIOS/PAD/GTE bootstrap ABI. Graphics, sound,
        // files, packet allocation and completion are NEVER success receipts.
        if(coldPrerequisites){
            const std::vector<uint32_t>a(args);
            if(fn==0x80047F5Cu&&a==std::vector<uint32_t>{0x80055FB0u}){++isolatedCalls;return 0;}
            if(fn==0x80048A30u&&a==std::vector<uint32_t>{0x80055FB0u}){++isolatedCalls;return -73;}
            if(fn==0x80048AE0u&&a==std::vector<uint32_t>{0u}){++isolatedCalls;return -77;}
            if(fn==0x80048AF0u&&a==std::vector<uint32_t>{3u,0u}){++isolatedCalls;return -11;}
            if(fn==0x80048960u&&a.empty()){++isolatedCalls;return -99;}
            if(fn==0x80048A50u&&a.empty()){++isolatedCalls;return -17;}
            if(fn==0x800489F0u&&a==std::vector<uint32_t>{0x20000001u,0x800882F0u}){++isolatedCalls;return -31;}
            if(fn==0x80048950u&&a.size()==1u){++isolatedCalls;return -47;}
            if(fn==0x80040D20u&&a.empty()){++isolatedCalls;return -23;}
            if(fn==0x80047FFCu){++isolatedCalls;return -19;}
        }
        throw D::Unbound(fn);
    }
};
uint32_t Word(const std::vector<uint8_t>& b,size_t p){
    Check(p+4u<=b.size(),"Resident archive is truncated");
    return uint32_t(b[p])|(uint32_t(b[p+1])<<8u)|(uint32_t(b[p+2])<<16u)|(uint32_t(b[p+3])<<24u);
}
void PreloadCommon(TestSession& s,const std::filesystem::path& path){
    // Explicit resident texture precondition inherited from the earlier
    // independent Loading tests. This is NOT COMPO02 loading or a source
    // resident-scene transfer handshake, which still requires integration.
    std::ifstream file(path,std::ios::binary);Check(bool(file),"Common asset missing");
    const std::vector<uint8_t> bytes((std::istreambuf_iterator<char>(file)),{});
    constexpr uint32_t archive=0x800A0000u,info=0x801B0000u;
    Check(bytes.size()<info-archive,"Common archive test bound");
    for(uint32_t n=0;n<bytes.size();++n)s.Write8(archive+n,bytes[n]);
    uint32_t tims=0;
    for(uint32_t at=0;at+8192u<=bytes.size();){
        const uint32_t kind=Word(bytes,at),count=Word(bytes,at+4u),sectors=Word(bytes,at+8u);
        if(kind==0xFFFFFFFFu)break;
        uint32_t payload=at+8192u;
        if(kind==1u)for(uint32_t n=0;n<count;++n){
            const uint32_t size=Word(bytes,at+16u+n*20u);
            Check(payload<=bytes.size()&&size<=bytes.size()-payload&&Word(bytes,payload)==16u,"Resident TIM layout");
            PrStage2TimDirect::GetTimInfo80040EAC(s,archive+payload+4u,info);
            const auto upload=[&](uint32_t rect,uint32_t source){
                s.Vram().UploadImageWords(s,{s.Read16(rect),s.Read16(rect+2u),s.Read16(rect+4u),s.Read16(rect+6u)},s.Read32(source));
            };
            upload(info+4u,info+12u);if(s.Read32(info)&8u)upload(info+16u,info+24u);
            payload+=size;++tims;
        }
        Check(sectors<=bytes.size()/2048u,"Resident sector overflow");at+=8192u+sectors*2048u;
    }
    Check(tims==87u,"Resident prerequisite changed");
    std::cout<<"resident-prerequisite "<<tims<<' '<<s.Vram().UploadCount()<<'\n';
}
void Pump(){MSG m;while(PeekMessageW(&m,nullptr,0,0,PM_REMOVE)){TranslateMessage(&m);DispatchMessageW(&m);}std::this_thread::sleep_for(std::chrono::milliseconds(1));}
void RunWaiting(TestSession& s,uint32_t seconds,D3D11Renderer* renderer=nullptr,const std::filesystem::path& capture={}){
    bool captured=false;
    const auto deadline=std::chrono::steady_clock::now()+std::chrono::seconds(seconds);
    uint64_t steps=0;
    while(s.State()==F::State::Waiting){
        if(std::chrono::steady_clock::now()>=deadline){
            const auto request=s.Pending();std::cerr<<"waiting-timeout fn="<<std::hex<<request.function<<" target="<<request.target<<std::dec<<" gpu-starts="<<s.Gpu().Starts()<<" gpu-completed="<<s.Gpu().Completions()<<" blanks="<<s.WallClockVBlankEvents()<<'\n';
            uint32_t dma=0,status=0,priority=0,head=0;
            s.Gpu().TryRead(0x1F8010A8u,4u,dma);s.Gpu().TryRead(0x1F801814u,4u,status);s.Gpu().TryRead(0x1F8010A0u,4u,head);s.Interrupts().TryRead(0x1F8010F0u,4u,priority);
            std::cerr<<"gpu-wait-state dma="<<std::hex<<dma<<" status="<<status<<" dpcr="<<priority<<" head="<<head<<std::dec<<" submitted="<<s.Gpu().Submitted()<<" faulted="<<s.Gpu().Faulted()<<'\n';
            throw std::runtime_error("Native device session did not reach its next boundary");
        }
        const auto before=s.Gpu().DisplayPresents();
        s.Poll();
        if(renderer&&!captured&&s.Gpu().DisplayPresents()>before&&s.Gpu().CommandsRendered()>50u){
            const auto image=TestStage2GpuReadback::Read(s.Gpu().PresentedImage().Get());
            Check(renderer->SaveRgbaPng(capture.wstring(),image.rgba.data(),int(image.width),int(image.height)),"Actual Loading presentation capture failed");
            captured=true;std::cout<<"retained-loading-capture actual-swapchain-not-gameplay\n";
        }
        Pump();++steps;
    }
    std::cout<<"host-event-polls "<<steps<<'\n';
}
void Observations(TestSession& s,const char* label){
    std::cout<<label<<" phase="<<uint32_t(s.GetPhase())<<" starts="<<s.InitializationStarts()
        <<" lookups="<<s.Disc().LookupRequests()<<" reads="<<s.Disc().ReadRequests()<<" bytes="<<s.Disc().BytesTransferred()
        <<" vram="<<s.Vram().UploadCount()<<" gpu-starts="<<s.Gpu().Starts()<<" gpu-fences="<<s.Gpu().Completions()
        <<" commands="<<s.Gpu().CommandsRendered()<<" presents="<<s.Gpu().DisplayPresents()
        <<" wall-blanks="<<s.WallClockVBlankEvents()<<" irq-entries="<<s.NativeIrqEntries()
        <<" spu-writes="<<s.Spu().RegisterWrites()<<" pcm-frames="<<s.Spu().RenderedFrames()
        <<" pcm-nonzero="<<s.Spu().NonzeroSamples()<<" flag="<<s.Read32(0x8006ECD4u)
        <<" dma4-starts="<<s.SpuTransfer().DmaStarts()<<" dma4-completions="<<s.SpuTransfer().DmaCompletions()
        <<" dma4-bytes="<<s.SpuTransfer().DmaBytes()<<" fifo-completions="<<s.SpuTransfer().FifoCompletions()
        <<" fifo-bytes="<<s.SpuTransfer().FifoBytes()<<" event-deliveries="<<s.SpuEvents().Deliveries()
        <<" event-consumed="<<s.SpuEvents().Consumed()<<'\n';
}
void DumpVram(TestSession& s,const std::filesystem::path& output){
    std::ofstream words(output/"retained_vram_words.bin",std::ios::binary), known(output/"retained_vram_known.bin",std::ios::binary);
    Check(bool(words)&&bool(known),"Cannot open retained VRAM evidence");
    for(uint16_t word:s.Vram().Words()){words.put(char(word&255u));words.put(char(word>>8u));}
    known.write(reinterpret_cast<const char*>(s.Vram().Known().data()),std::streamsize(s.Vram().Known().size()));
    Check(bool(words)&&bool(known),"Retained VRAM evidence write failed");
    if(s.SpuTransfer().DmaCompletions()){
        const uint32_t bank=s.Read16(0x800943A8u);
        Check(bank<16u,"Active VAB bank id invalid");
        const uint32_t address=s.Read32(0x801C35F8u+4u*bank),bytes=s.Read32(0x801C35B0u+4u*bank);
        const auto data=s.Spu().ReadRam(address,bytes);
        std::ofstream vb(output/"retained_spu_vb.bin",std::ios::binary);vb.write(reinterpret_cast<const char*>(data.data()),std::streamsize(data.size()));
        Check(bool(vb),"SPU data evidence failed");
        std::cout<<"retained-vab "<<bank<<' '<<address<<' '<<bytes<<'\n';
    }
}
struct Window{
    HWND handle=CreateWindowExW(0,L"STATIC",L"PaRappaWin native Stage2 Loading device test",WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,CW_USEDEFAULT,400,300,nullptr,nullptr,GetModuleHandleW(nullptr),nullptr);
    ~Window(){if(handle)DestroyWindow(handle);}
};
}
int main(int argc,char**argv){
    try{
        Check(argc==6||(argc==7&&std::string(argv[6])=="--probe-next"),"Arguments: SCUS COMOD2 disc common output [--probe-next]");
        Window window;Check(window.handle!=nullptr,"Window unavailable");
        D3D11Renderer renderer;Check(renderer.Initialize(window.handle,640,480),"D3D unavailable");
        WasapiSink sink;sink.SetVolume(0.05f);
        TestSession session(std::filesystem::u8path(argv[1]),std::filesystem::u8path(argv[2]),std::filesystem::u8path(argv[3]),renderer,sink);
        bool earlyScene=false;try{session.StartScene();}catch(const std::logic_error&){earlyScene=true;}
        Check(earlyScene&&session.InitializationStarts()==0u,"Scene entered before graph/init ownership");
        session.StartGraphics();
        Check(session.State()==F::State::Waiting&&session.Gpu().Pending()&&session.Gpu().Submitted(),
              "GPU start must submit real work without acknowledging its fence in the same service call");
        if(session.Gpu().Pending()){
            const auto completed=session.Gpu().Completions();
            Check((session.Read32(0x1F8010A8u)&0x01000000u)!=0u,"DMA read fabricated completion");
            Check(PrStage2GpuDirect::SyncQueue80046FFC(session,1u)>0,"Nonblocking sync did not report busy");
            Check(session.Gpu().Completions()==completed,"Nonblocking sync consumed GPU work");
            for(uint32_t n=0;n<3u;++n)Check(session.Poll(false)==F::State::Waiting,"Eventless graphics resume completed DMA");
            std::cout<<"gpu-nonblocking-status no-suspension-no-completion\n";
        }
        RunWaiting(session,20u);
        Observations(session,"graphics-observation");session.RethrowFailure();
        Check(session.State()==F::State::Completed,"Source graph bootstrap did not complete");
        std::cout<<"source-graphics-ready isolated-abi="<<session.isolatedCalls<<'\n';
        session.coldPrerequisites=false;
        PreloadCommon(session,std::filesystem::u8path(argv[4]));
        session.StartInitialization();
        for(uint32_t n=0;n<3u&&session.State()==F::State::Waiting;++n)session.Poll(false);
        Check(session.InitializationStarts()==1u,"Initializer entered repeatedly");
        const auto output=std::filesystem::u8path(argv[5]);
        try{RunWaiting(session,20u,&renderer,output/"stage2_retained_loading.png");}
        catch(...){Observations(session,"initializer-observation");DumpVram(session,output);throw;}
        Observations(session,"initializer-observation");
        DumpVram(session,output);
        Check(session.InitializationStarts()==1u&&session.Disc().LookupRequests()==7u,
              "Retained initializer or file location restarted");
        Check(session.Output().Running()&&session.Spu().RenderedFrames()>0u,
              "Actual audio endpoint did not consume native samples");
        try{session.RethrowFailure();}
        catch(const D::Unbound& error){
            std::cout<<"next-unbound "<<std::hex<<error.address<<std::dec<<' '<<error.width<<' '<<error.writing<<'\n';
            throw;
        }
        Check(session.State()==F::State::Completed,"Initializer remains incomplete");
        const uint32_t depth=session.Read32(0x8006EB84u),cursor=session.Read32(0x8006EDC8u);
        Check(depth==273u,"Final resident resource table depth differs");
        const uint32_t first=session.Read32(0x8009185Cu);
        Check(first==0x800965B0u&&cursor>=first&&cursor-first<0x100000u,"Final resource heap range differs");
        std::ofstream table(output/"resident_resource_table.bin",std::ios::binary),heap(output/"resident_resource_heap.bin",std::ios::binary);
        Check(bool(table)&&bool(heap),"Resource evidence files unavailable");
        for(uint32_t slot=1u;slot<=depth;++slot){const uint32_t address=session.Read32(0x80091858u+4u*slot);for(uint32_t j=0;j<4u;++j)table.put(char(address>>(8u*j)));}
        for(uint32_t at=first;at<cursor;++at)heap.put(char(session.Read8(at)));
        Check(bool(table)&&bool(heap),"Resource evidence write failed");
        std::cout<<"retained-residents "<<depth<<' '<<first<<' '<<cursor<<' '<<(cursor-first)<<'\n';
        Check(session.SpuTransfer().DmaStarts()==1u&&session.SpuTransfer().DmaCompletions()==1u&&
              session.SpuTransfer().DmaBytes()==479168u&&session.SpuTransfer().FifoBytes()==16u,
              "Source FIFO/DMA4 transfer sequence differs");
        Check(session.SpuEvents().Deliveries()==1u&&session.SpuEvents().Consumed()==1u,
              "Original DMA ISR did not deliver/consume exactly one completion event");
        Check(session.Read8(0x800928F8u)==1u&&session.Read16(0x801C35F0u)==1u&&
              session.Read32(0x800555F8u)==1u,"VAB is not ready after real transfer");
        Check(session.GetPhase()==PrStage2SceneEntry::Phase::InitializerReturned,"InitScene completion phase missing");
        bool reentry=false;try{session.InitializeScene();}catch(const std::logic_error&){reentry=true;}
        Check(reentry&&session.InitializationStarts()==1u,"Completed initializer can be accidentally restarted");
        // EndLoading returns its final DrawOTag queue depth (or the unchanged
        // draw flag when no final Present is needed), not a constant status.
        // Read immediately, before another event can consume queued work.
        const uint32_t finalFlag=session.Read32(0x8006ECD4u);
        const uint32_t queueHead=session.Read32(0x8005D838u),queueTail=session.Read32(0x8005D83Cu);
        const uint32_t expectedReturn=finalFlag==1u?((queueHead-queueTail)&63u):finalFlag;
        Check(uint32_t(session.TaskResult())==expectedReturn,"Initializer return does not match the original final queue/flag result");
        std::cout<<"native-init-return-state "<<finalFlag<<' '<<queueHead<<' '<<queueTail<<' '<<expectedReturn<<'\n';
        std::cout<<"native-init-result "<<session.TaskResult()<<'\n';
        std::cout<<"native-initializer-returned\n";
        if(argc==7){
            session.StartScene();RunWaiting(session,20u);
            try{session.RethrowFailure();}
            catch(const D::Unbound&e){
                Check(session.InitializationStarts()==1u&&session.Disc().LookupRequests()==7u,
                      "Starting the main function repeated initialization");
                std::cout<<"post-init-next "<<std::hex<<e.address<<std::dec<<' '<<e.width<<' '<<e.writing<<'\n';
                return 3;
            }
            std::cout<<"post-init-scene-returned\n";
        }
        return 0;
    }catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 2;}
}
