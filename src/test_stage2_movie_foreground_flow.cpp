#include "pr/pr_stage2_vlc_session.h"
#include "pr/pr_pad.h"
#include "pr/pr_psx_pad_direct.h"
#include <cstdlib>
#include <map>
#include <array>
#include <set>
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
struct TestSession final:PrStage2VlcSession::Session{
    using Session::Session;
    std::map<uint32_t,uint32_t> calls;
    uint64_t padSamples=0,blitCalls=0,imageStrips=0,imageBytes=0;
    bool inBlit=false,observeMovie=false,movieObserved=false,runToBoundary=false;
    uint64_t presentedPixelChecks=0;
    uint64_t movePixelChecks=0,moveSubmissions=0;
    uint32_t observationFrames=12u;
    uint32_t cleanupHeapDepth=0,naturalMovieEnds=0;
    std::vector<uint32_t> decodedSourceFrames;
    std::array<std::vector<uint32_t>,2> expectedPage{{std::vector<uint32_t>(320u*240u),std::vector<uint32_t>(320u*240u)}};
    std::array<std::vector<uint8_t>,2> expectedKnown{{std::vector<uint8_t>(320u*240u),std::vector<uint8_t>(320u*240u)}};
    void WriteDevice32(uint32_t address,uint32_t value)override{
        PrStage2OtDrawBackend::Batch batch;bool submitted=false;
        std::vector<uint32_t> beforeMove;uint32_t moveDestination=0u;
        const uint64_t completed=Gpu().Completions();
        if(observeMovie&&(address&0x1FFFFFFFu)==0x1F8010A8u&&(value&0x01000000u)){
            uint32_t head=0;Check(Gpu().TryRead(0x1F8010A0u,4u,head),"Cannot inspect actual OT producer");
            batch=PrStage2OtDrawBackend::DecodeLinkedList(*this,head,Gpu().DrawState());submitted=true;
            if(batch.commands.size()==1u&&(batch.commands[0].opcode&0xE0u)==0x80u){
                const auto& c=batch.commands[0];
                Check(c.copySourceX==0u&&c.copyDestinationX==0u&&c.fillWidth==320u&&c.fillHeight==240u&&
                    c.copySourceY%240u==0u&&c.copyDestinationY%240u==0u,"Natural MoveImage is not a complete framebuffer page");
                beforeMove=TestStage2GpuReadback::Read(Gpu().FramebufferImage(c.copySourceY/240u).Get()).rgba;
                moveDestination=c.copyDestinationY/240u;
            }
        }
        PrStage2VlcSession::Session::WriteDevice32(address,value);
        if(!beforeMove.empty()){
            Check(Gpu().Submitted()&&Gpu().Pending()&&Gpu().Completions()==completed,"Natural MoveImage bypassed DMA fence");
            const auto after=TestStage2GpuReadback::Read(Gpu().FramebufferImage(moveDestination).Get()).rgba;
            Check(after==beforeMove,"Natural MoveImage did not copy actual source pixels");
            movePixelChecks+=after.size();++moveSubmissions;
        }
        if(submitted&&Gpu().Submitted())for(const auto& command:batch.commands){
            if((command.opcode&0xE0u)==0x80u){
                // 复制传递来源像素及其已知状态，不能误当作零顶点绘图命令。
                const auto pages=expectedPage;const auto known=expectedKnown;
                for(uint32_t y=0;y<command.fillHeight;++y)for(uint32_t x=0;x<command.fillWidth;++x){
                    const uint32_t sy=command.copySourceY+y,dy=command.copyDestinationY+y;
                    const uint32_t sx=command.copySourceX+x,dx=command.copyDestinationX+x;
                    Check(sy<480u&&dy<480u&&sx<320u&&dx<320u,"Observed copy escapes framebuffer");
                    expectedPage[dy/240u][dy%240u*320u+dx]=pages[sy/240u][sy%240u*320u+sx];
                    expectedKnown[dy/240u][dy%240u*320u+dx]=known[sy/240u][sy%240u*320u+sx];
                }
                continue;
            }
            int left=command.vertex[0].x,right=left,top=command.vertex[0].y,bottom=top;
            if(command.opcode==2u){right+=command.fillWidth;bottom+=command.fillHeight;}
            else{
                for(uint32_t i=1;i<command.vertices;++i){
                    left=(std::min)(left,int(command.vertex[i].x));right=(std::max)(right,int(command.vertex[i].x));
                    top=(std::min)(top,int(command.vertex[i].y));bottom=(std::max)(bottom,int(command.vertex[i].y));
                }
                left+=command.state.offsetX;right+=command.state.offsetX;
                top+=command.state.offsetY;bottom+=command.state.offsetY;
                left=(std::max)(left,int(command.state.clipLeft));right=(std::min)(right,int(command.state.clipRight)+1);
                top=(std::max)(top,int(command.state.clipTop));bottom=(std::min)(bottom,int(command.state.clipBottom)+1);
            }
            // Later original border/glyph commands legitimately cover source
            // image pixels. The exact CPU-copy contract is tested separately;
            // scene readback compares only data not covered by a later packet.
            for(int y=(std::max)(0,top);y<(std::min)(480,bottom);++y)
                for(int x=(std::max)(0,left);x<(std::min)(320,right);++x)expectedKnown[unsigned(y)/240u][unsigned(y)%240u*320u+unsigned(x)]=0u;
        }
    }
    int32_t CallLoadingDevice(uint32_t fn,std::initializer_list<uint32_t> args)override{
        ++calls[fn];
        if(observeMovie&&fn==0x80047B30u&&args.size()==2u&&args.begin()[0]){
            const uint32_t ring=Read32(0x800965ACu),slots=Read32(0x801C3868u);
            const uint32_t offset=args.begin()[0]-ring-slots*32u;
            Check(offset%2016u==0u&&offset/2016u<slots,"Decoder source is not an owned ring frame");
            decodedSourceFrames.push_back(Read32(ring+offset/2016u*32u+8u));
        }
        const bool previous=inBlit;if(fn==0x8002756Cu){inBlit=true;++blitCalls;}
        try{
            const int32_t result=PrStage2VlcSession::Session::CallLoadingDevice(fn,args);inBlit=previous;
            if(observeMovie&&fn==0x8001A7F8u&&result==1){
                const uint32_t record=*args.begin(),position=Read32(0x80049404u);
                const uint32_t limit=Read32(record+44u)+Read32(record+8u);
                ++naturalMovieEnds;
                std::cout<<"movie-natural-end record="<<record<<" position="<<position<<" limit="<<limit<<'\n';
            }
            return result;
        }
        catch(...){inBlit=previous;throw;}
    }
    void CallLoadingDeviceVoid(uint32_t fn,std::initializer_list<uint32_t> args)override{
        ++calls[fn];PrStage2VlcSession::Session::CallLoadingDeviceVoid(fn,args);
    }
    int32_t LoadImage80044D64(PrStage2LifecycleDirect::ImageRect rect,uint32_t source)override{
        const int32_t result=PrStage2VlcSession::Session::LoadImage80044D64(rect,source);
        if(inBlit){
            ++imageStrips;imageBytes+=uint64_t(rect.width)*rect.height*2u;
            const uint32_t page=rect.y/240u;
            Check(page<2u&&rect.y%240u+rect.height<=240u&&rect.x+rect.width<=320u,"Original movie upload escapes display page");
            for(uint32_t y=0;y<rect.height;++y)for(uint32_t x=0;x<rect.width;++x){
                const uint16_t word=Read16(source+2u*(y*rect.width+x));
                const uint32_t r=word&31u,g=(word>>5u)&31u,b=(word>>10u)&31u;
                const uint32_t rgba=(r*8u+r/4u)|((g*8u+g/4u)<<8u)|((b*8u+b/4u)<<16u)|0xFF000000u;
                const uint32_t at=(rect.y%240u+y)*320u+rect.x+x;
                expectedPage[page][at]=rgba;expectedKnown[page][at]=1u;
            }
        }
        return result;
    }
    bool coldPrerequisites=true;
    uint64_t isolatedCalls=0;
    int32_t VideoDependency(uint32_t fn,std::initializer_list<uint32_t> args)override{
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
        if(!capture.empty()&&(steps%1000u)==0u){
            std::ofstream progress(capture.parent_path()/"progress.txt",std::ios::app);
            const auto pending=s.Pending();
            const uint32_t maskAddress=s.Read32(0x80057008u),statusAddress=s.Read32(0x80057004u);
            progress<<steps<<" decoded="<<s.MovieOutput().DecodedFrames()
                <<" published="<<s.MovieOutput().PublishedFrames()
                <<" callbacks="<<s.MovieOutput().CallbackCalls()
                <<" sectors="<<s.Cd().SectorReads()
                <<" locations="<<s.MovieLocation().Replies()
                <<" presents="<<s.Gpu().DisplayPresents()
                <<" vblank="<<s.Read32(0x80057034u)
                <<" registered="<<s.Read16(0x80055FA8u)
                <<" mask="<<s.Read16(maskAddress)
                <<" status="<<s.Read16(statusAddress)
                <<" vblank-callback="<<std::hex<<s.Read32(0x80055F7Cu)
                <<" dma-serial="<<std::dec<<s.RingDma().Serial()
                <<" dma-starts="<<s.RingDma().Starts()
                <<" dma-completed="<<s.RingDma().Completions()
                <<" spu-irqs="<<(s.SpuEvents().InterruptsEnabled()?1:0)
                <<" pending="<<pending.function<<std::dec<<":"<<pending.target<<'\n';
        }

        if(std::chrono::steady_clock::now()>=deadline){
            const auto request=s.Pending();const uint32_t maskAddress=s.Read32(0x80057008u),statusAddress=s.Read32(0x80057004u);
            std::cerr<<"waiting-timeout fn="<<std::hex<<request.function<<" target="<<request.target<<" vblank="<<s.Read32(0x80057034u)<<" registered="<<s.Read16(0x80055FA8u)<<" mask="<<s.Read16(maskAddress)<<" status="<<s.Read16(statusAddress)<<" vblank-callback="<<s.Read32(0x80055F7C)<<std::dec<<" dma-serial="<<s.RingDma().Serial()<<" dma-starts="<<s.RingDma().Starts()<<" dma-completed="<<s.RingDma().Completions()<<" spu-irqs="<<(s.SpuEvents().InterruptsEnabled()?1:0)<<" gpu-starts="<<s.Gpu().Starts()<<" gpu-completed="<<s.Gpu().Completions()<<" blanks="<<s.WallClockVBlankEvents()<<'\n';
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
        if(s.observeMovie&&s.Gpu().DisplayPresents()>before){
            const auto image=TestStage2GpuReadback::Read(s.Gpu().PresentedImage().Get());
            const uint32_t page=((s.Gpu().DisplayAddress()>>10u)&511u)/240u;
            Check(page<2u&&image.width==640u&&image.height==480u,"Natural movie display dimensions changed");
            uint64_t compared=0;std::set<uint32_t> colors;
            for(uint32_t y=0;y<240u;++y)for(uint32_t x=0;x<320u;++x)if(s.expectedKnown[page][y*320u+x]){
                const uint32_t expected=s.expectedPage[page][y*320u+x];colors.insert(expected);
                for(uint32_t sy=0;sy<2u;++sy)for(uint32_t sx=0;sx<2u;++sx){
                    const uint32_t actual=image.rgba[(2u*y+sy)*640u+2u*x+sx];
                    if(actual!=expected){
                        std::cerr<<"pixel-mismatch x="<<x<<" y="<<y<<" sx="<<sx<<" sy="<<sy<<" page="<<page
                            <<" actual="<<std::hex<<actual<<" expected="<<expected<<std::dec
                            <<" presented-copy="<<s.Gpu().PresentedCpuImageCopy()<<" copies="<<s.Gpu().CpuImageCopies()<<'\n';
                        if(renderer){
                            renderer->SaveRgbaPng((capture.parent_path()/"mismatch_actual.png").wstring(),image.rgba.data(),640,480);
                            renderer->SaveRgbaPng((capture.parent_path()/"mismatch_expected.png").wstring(),s.expectedPage[page].data(),320,240);
                        }
                    }
                    Check(actual==expected,"Actual presented movie pixel differs from original LoadImage words");++compared;
                }
            }
            s.presentedPixelChecks+=compared;
            if(compared>=128u*100u*4u&&colors.size()>2u&&s.MovieOutput().PublishedFrames()>=s.observationFrames){
                if(!s.runToBoundary){
                    Check(renderer&&renderer->SaveRgbaPng(capture.wstring(),image.rgba.data(),640,480),"Cannot save actual naturally presented movie frame");
                    s.movieObserved=true;
                    std::cout<<"natural-movie-presented pixels="<<s.presentedPixelChecks<<" colors="<<colors.size()
                        <<" copies="<<s.Gpu().CpuImageCopies()<<" presented-copy="<<s.Gpu().PresentedCpuImageCopy()
                        <<" location-requests="<<s.MovieLocation().Requests()<<" location-replies="<<s.MovieLocation().Replies()<<'\n';
                    break; // Observation ends; the original movie has not returned.
                }
            }
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
        PrPad::Init();session.EnableSharedMovieOutput();
        session.EnableMovieForeground([&]{
            ++session.padSamples;
            PrPad::Update(GetForegroundWindow()==window.handle);
            const uint32_t one=PrPsxPadDirect::NormalizeLocalPrPadMaskToReturnedMask80035510(PrPad::GetState(0).held);
            const uint32_t two=PrPsxPadDirect::NormalizeLocalPrPadMaskToReturnedMask80035510(PrPad::GetState(1).held);
            return one|(two<<16u);
        });
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
            if(const char* subtitle=std::getenv("PARAPPA_S2_TEST_SUBTITLES")){
                Check(std::string(subtitle)=="0"||std::string(subtitle)=="1","Invalid test subtitle option");
                session.Write16(0x800916DCu,uint16_t(subtitle[0]-'0'));
            }
            if(const char* frames=std::getenv("PARAPPA_S2_OBSERVE_FRAMES")){
                const std::string value(frames);
                Check(value=="12"||value=="64"||value=="120","Invalid movie observation length");
                session.observationFrames=value=="120"?120u:value=="64"?64u:12u;
            }
            session.runToBoundary=std::getenv("PARAPPA_S2_RUN_TO_BOUNDARY")!=nullptr;
            session.cleanupHeapDepth=session.Read32(0x8006EB84u);
            session.observeMovie=true;session.StartScene();
            // The original 12-frame test keeps its 20-second deadline. Longer
            // observation requests get a separate bound; source clocks do not change.
            const uint32_t observationSeconds=session.runToBoundary
                ? uint32_t(std::strtoul(std::getenv("PARAPPA_S2_RUN_SECONDS")?std::getenv("PARAPPA_S2_RUN_SECONDS"):"900",nullptr,10))
                : (session.observationFrames==12u?20u:session.observationFrames);
            try{RunWaiting(session,observationSeconds,&renderer,output/"stage2_actual_movie_frame.png");}catch(...){
                Observations(session,"foreground-observation");
                std::cerr<<"movie-progress decoded="<<session.MovieOutput().DecodedFrames()<<" published="<<session.MovieOutput().PublishedFrames()
                    <<" callbacks="<<session.MovieOutput().CallbackCalls()<<" blits="<<session.blitCalls
                    <<" copies="<<session.Gpu().CpuImageCopies()<<" locations="<<session.MovieLocation().Replies()<<'\n';throw;
            }
            Observations(session,"foreground-observation");
            if(session.runToBoundary){
                std::cout<<"movie-move-state copies="<<session.Gpu().MoveImageCopies()<<" submissions="<<session.moveSubmissions
                    <<" pixels="<<session.movePixelChecks<<" pending="<<session.Gpu().Pending()
                    <<" starts="<<session.Gpu().Starts()<<" completions="<<session.Gpu().Completions()<<'\n';
                std::cout<<"movie-cleanup-state natural-ends="<<session.naturalMovieEnds
                    <<" heap-before="<<session.cleanupHeapDepth<<" heap-after="<<session.Read32(0x8006EB84u)
                    <<" mdec-callback="<<session.Read32(0x80057044u)<<" dma3-callback="<<session.Read32(0x8005704Cu)
                    <<" ready-callback="<<session.Read32(0x800570FCu)<<" sync-callback="<<session.Read32(0x800570F8u)
                    <<" interrupts-enabled="<<session.SpuEvents().InterruptsEnabled()<<'\n';
            }
            std::cout<<"movie-source-frames";
            for(uint32_t frame:session.decodedSourceFrames)std::cout<<' '<<frame;
            std::cout<<'\n'<<"movie-stream-observation sectors="<<session.Cd().SectorReads()
                <<" last-lba="<<session.Cd().LastSector()<<" next-lba="<<session.Cd().Location()<<" mode="<<unsigned(session.Cd().Mode())
                <<" unrequested-replaced="<<session.Cd().UnrequestedSectorsReplaced()
                <<" deferrals="<<session.Cd().VideoDeferrals()<<" held="<<session.Cd().VideoDeliveryPending()
                <<" xa-sectors="<<session.Xa().Sectors()<<" xa-queued="<<session.Spu().CdFramesQueued()
                <<" xa-consumed="<<session.Spu().CdFramesConsumed()<<" xa-nonzero="<<session.Spu().CdNonzeroSamples()
                <<" xa-underrun="<<session.Spu().CdUnderflowFrames()<<'\n';
            std::cout<<"foreground-output decoded="<<session.MovieOutput().DecodedFrames()
                <<" published="<<session.MovieOutput().PublishedFrames()<<" callbacks="<<session.MovieOutput().CallbackCalls()
                <<" pending="<<session.MovieOutput().Pending()<<" vlc="<<session.VlcCalls()<<" pad-samples="<<session.padSamples
                <<" blits="<<session.blitCalls<<" strips="<<session.imageStrips<<" image-bytes="<<session.imageBytes<<'\n';
            for(const auto& c:session.calls)std::cout<<"source-call "<<std::hex<<c.first<<std::dec<<' '<<c.second<<'\n';
            const auto picture=session.MovieOutput().LastCompleted();
            if(picture){
                std::ofstream pixels(output/"natural_movie_strips.bin",std::ios::binary);
                pixels.write(reinterpret_cast<const char*>(picture->strips15.data()),std::streamsize(picture->strips15.size()));
                Check(bool(pixels),"Could not save natural codec output");
                Check(renderer.SaveRgbaPng((output/"natural_decoded_not_presented.png").wstring(),picture->rgba.data(),picture->width,picture->height),"Could not save decoded picture");
            }
            if(session.movieObserved){
                Check(session.State()==F::State::Waiting&&session.GetPhase()==PrStage2SceneEntry::Phase::Running,"Observation fabricated an original scene return");
                std::cout<<"NATURAL_MOVIE_FRAME_PRESENTED bounded-observation-not-scene-return\n";return 4;
            }

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
