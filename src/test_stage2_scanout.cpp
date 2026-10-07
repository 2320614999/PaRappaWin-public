#include "test_stage2_gpu_image_readback.h"
#include "pr/pr_stage2_scene_entry.h"
#include "pr/pr_stage2_gpu_device.h"
#include "pr/pr_stage2_vram_device.h"
#include <chrono>
#include <iostream>
#include <thread>
#include <vector>
#include <windows.h>

namespace {
using Args=std::initializer_list<uint32_t>;
namespace G=PrStage2GpuDevice;
uint32_t checks=0;
void Check(bool good,const char* why){++checks;if(!good)throw std::runtime_error(why);}
template<class F>void Reject(F&& f){bool rejected=false;try{f();}catch(const std::exception&){rejected=true;}Check(rejected,"Expected GPU failure not raised");}
struct Memory final:PrStage2SceneEntry::Services{
    using Services::Services;
    PrPsxGteDirect::MatrixRegisters gte{};
    int32_t CallPlatform(uint32_t,Args)override{throw std::logic_error("No GPU-contract function receipt");}
    void CallPlatformVoid(uint32_t,Args)override{throw std::logic_error("No GPU-contract void receipt");}
    uint8_t ReadDevice8(uint32_t)override{throw std::logic_error("Unbound memory fixture byte MMIO");}
    uint16_t ReadDevice16(uint32_t)override{throw std::logic_error("Unbound memory fixture halfword MMIO");}
    uint32_t ReadDevice32(uint32_t)override{throw std::logic_error("Unbound memory fixture word MMIO");}
    void WriteDevice8(uint32_t,uint8_t)override{throw std::logic_error("Unbound memory fixture byte MMIO");}
    void WriteDevice16(uint32_t,uint16_t)override{throw std::logic_error("Unbound memory fixture halfword MMIO");}
    void WriteDevice32(uint32_t,uint32_t)override{throw std::logic_error("Unbound memory fixture word MMIO");}
    PrStage2LifecycleDirect::Words64 Call64(uint32_t,Args)override{throw std::logic_error("No wide receipt");}
    PrStage2LifecycleDirect::Vector32 NormalizeVector8003A3DC(PrStage2LifecycleDirect::Vector32)override{throw std::logic_error("No vector receipt");}
    int32_t SetCdLocation800367A4(uint32_t)override{throw std::logic_error("No CD receipt");}
    int32_t LoadImage80044D64(PrStage2LifecycleDirect::ImageRect,uint32_t)override{throw std::logic_error("No upload receipt");}
    PrPsxGteDirect::MatrixRegisters& MatrixGte()override{return gte;}
    [[noreturn]]void Exit(uint32_t,Args)override{throw std::logic_error("Unexpected exit");}
    [[noreturn]]void Break(uint32_t,uint32_t)override{throw std::logic_error("Unexpected BREAK");}
};
void Write(G::Device& d,uint32_t address,uint32_t value){Check(d.TryWrite(address,4u,value),"GPU address not recognized");}
uint32_t Read(G::Device& d,uint32_t address){uint32_t value=0;Check(d.TryRead(address,4u,value),"GPU address not recognized");return value;}
void Arm(G::Device& d,uint32_t head){Write(d,0x1F8010A0u,head);Write(d,0x1F8010A4u,0u);Write(d,0x1F8010A8u,0x01000401u);}
void FullClear(Memory& m,uint32_t head){
    m.Write32(head,0x03FFFFFFu);m.Write32(head+4u,0x02201008u);
    m.Write32(head+8u,0u);m.Write32(head+12u,(480u<<16u)|320u);
}
void Fence(G::Device& d){
    const auto until=std::chrono::steady_clock::now()+std::chrono::seconds(5);
    while(d.Pending()){
        Check(std::chrono::steady_clock::now()<until,"Real GPU fence did not complete");
        const auto progress=d.Service();
        Check(progress==G::Progress::Waiting||progress==G::Progress::Completed,"Unexpected fence state");
        if(d.Pending())std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}
void Run(Memory& m,D3D11Renderer& renderer){
    PrStage2VramDevice::Device vram;PrStage2VramAtlas::Projection atlas(vram);
    PrStage2InterruptDevice::Controller irq;
    const PrStage2OtDrawBackend::Viewport view{0,0,320,480,0,0,64,48};
    G::Device d(m,atlas,irq,renderer,view);
    constexpr uint32_t head=0x800B8000u;
    uint32_t untouched=0xDEADBEEFu;
    Check(!d.TryRead(0x1F801C00u,4u,untouched)&&untouched==0xDEADBEEFu,"GPU claimed foreign device");
    for(uint32_t alias:{0u,0x80000000u,0xA0000000u})for(uint32_t base:{0x1F8010A0u,0x1F801810u}){
        for(uint32_t width:{1u,2u,8u}){Reject([&]{d.TryRead(base|alias,width,untouched);});Reject([&]{d.TryWrite(base|alias,width,0u);});}
        Reject([&]{d.TryRead((base+1u)|alias,4u,untouched);});
    }
    Reject([&]{Write(d,0x1F801814u,0x04000003u);});
    Check(d.Starts()==0u&&d.Completions()==0u,"Invalid GPU transactions changed transfer counts");
    FullClear(m,head);
    irq.TryWrite(0x1F8010F4u,4u,0x00840000u);irq.TryWrite(0x1F801074u,2u,8u);
    Write(d,0x1F801814u,0x04000002u);Arm(d,head);
    Check(d.Service()==G::Progress::Blocked&&!d.Submitted(),"Disabled DMA channel consumed commands");
    Check(d.CommandsRendered()==0u&&d.Completions()==0u&&irq.Pending()==0u,"Blocked DMA fabricated work");
    const uint32_t busy=Read(d,0x1F8010A8u);
    for(uint32_t i=0;i<64u;++i)Check(Read(d,0x1F8010A8u)==busy&&d.Completions()==0u,"Plain status read completed DMA");
    irq.TryWrite(0x1F8010F0u,4u,0x800u);
    Check(d.Service()==G::Progress::Submitted&&d.Pending()&&d.Submitted()&&d.Completions()==0u,"Submission acknowledged its own fence");
    Check(irq.Pending()==0u,"IRQ published before separate fence completion");
    Reject([&]{Write(d,0x1F8010A0u,0u);});Reject([&]{Write(d,0x1F8010A4u,1u);});
    Reject([&]{Write(d,0x1F8010A8u,0u);});
    for(uint32_t i=0;i<8u;++i)Check(Read(d,0x1F8010A8u)==busy&&d.Completions()==0u,"Read acknowledged submitted GPU work");

    // These are ordered D3D page copies, but native DMA is still busy. An
    // actual scanout cannot make the source-visible fence or IRQ happen.
    Write(d,0x1F801814u,0x03000000u);
    for(uint32_t pageY:{0u,240u}){
        Write(d,0x1F801814u,0x05000000u|(pageY<<10u));
        const auto completions=d.Completions();const auto pending=irq.Pending();
        const auto control=Read(d,0x1F8010A8u);const auto serial=d.Serial();
        Check(d.Pending()&&d.Submitted(),"Pending scanout test lost its transfer");
        Check(d.PresentDisplay(),"Submitted source page did not actually present");
        Check(d.Pending()&&d.Submitted()&&d.Completions()==completions&&irq.Pending()==pending&&
              Read(d,0x1F8010A8u)==control&&d.Serial()==serial,"Scanout forged DMA completion or IRQ");
        for(auto pixel:TestStage2GpuReadback::Read(d.PresentedImage().Get()).rgba)
            Check(pixel==0xFF201008u,"Ordered pending-DMA scanout copied stale pixels");
    }
    std::cout<<"scanout-pending two-pages 6144-pixels zero-dma-ack zero-irq\n";
    Fence(d);
    Check(d.Completions()==1u&&Read(d,0x1F8010A0u)==0xFFFFFFu&&(Read(d,0x1F8010A8u)&0x01000000u)==0u,"Wrong DMA completion registers");
    uint32_t flags=0;irq.TryRead(0x1F8010F4u,4u,flags);
    Check((flags&0x84000000u)==0x84000000u&&irq.Pending()==8u,"Real fence did not notify DMA2 IRQ");
    Check(d.Service()==G::Progress::Idle&&d.Completions()==1u,"Idle service repeated completion");
    Write(d,0x1F801814u,0x03000000u);
    uint32_t pixels=0;
    for(uint32_t y:{0u,240u}){
        Write(d,0x1F801814u,0x05000000u|(y<<10u));Check(d.PresentDisplay(),"Original page did not present");
        for(auto pixel:TestStage2GpuReadback::Read(d.PresentedImage().Get()).rgba){if(pixel!=0xFF201008u)std::cerr<<"pixel-page "<<y<<" sample="<<pixels<<" got="<<std::hex<<pixel<<" expected=ff201008"<<std::dec<<"\n";Check(pixel==0xFF201008u,"Actual framebuffer page differs from decoded clear");++pixels;}
    }
    Check(pixels==6144u&&d.DisplayPresents()==4u,"Missing two-page pixel/present verification");
    bool ownerRejected=false;
    std::thread other([&]{try{uint32_t value;d.TryRead(0x1F801814u,4u,value);}catch(const std::logic_error&){ownerRejected=true;}});other.join();
    Check(ownerRejected,"Foreign thread accessed immediate-context owner");
    for(bool missingTexture:{false,true}){
        PrStage2InterruptDevice::Controller badIrq;G::Device bad(m,atlas,badIrq,renderer,view);
        badIrq.TryWrite(0x1F8010F0u,4u,0x800u);
        if(missingTexture){
            FullClear(m,head);m.Write32(head,0x08FFFFFFu);m.Write32(head+16u,0xE1000200u);
            m.Write32(head+20u,0x65000000u);m.Write32(head+24u,0u);m.Write32(head+28u,0u);m.Write32(head+32u,0x00100010u);
        }else m.Write32(head,head&0xFFFFFFu);
        Write(bad,0x1F801814u,0x04000002u);Arm(bad,head);
        Reject([&]{bad.Service();});
        Check(bad.Pending()&&bad.Faulted()&&bad.Completions()==0u&&badIrq.Pending()==0u,"Malformed source fabricated successful GPU completion");
        Reject([&]{bad.Service();});Reject([&]{bad.PresentDisplay();});
    }
    std::cout<<"gpu-device-contract two-pages 6144 pixels 1-fence 4-presents no-read-pump invalid-input-gates\n";
}
}
int main(int argc,char**argv){
    try{
        Check(argc==3,"Arguments: SCUS COMOD2");
        const HWND window=CreateWindowExW(0,L"STATIC",L"Stage2 GPU contract",WS_OVERLAPPEDWINDOW,0,0,100,100,nullptr,nullptr,GetModuleHandleW(nullptr),nullptr);
        Check(window!=nullptr,"Test window creation failed");
        struct Guard{HWND h;~Guard(){DestroyWindow(h);}}guard{window};
        D3D11Renderer renderer;Check(renderer.Initialize(window,64,48),"D3D initialization failed");
        Memory memory(std::filesystem::u8path(argv[1]),std::filesystem::u8path(argv[2]));
        Run(memory,renderer);std::cout<<"gpu-device-pass "<<checks<<" assertions\n";return 0;
    }catch(const std::exception&e){std::cerr<<e.what()<<'\n';return 2;}
}
