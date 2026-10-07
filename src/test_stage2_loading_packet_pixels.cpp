#include "pr/pr_stage2_loading_packets_direct.h"
#include "pr/pr_stage2_loading_work_direct.h"
#include "pr/pr_stage2_otc_device.h"
#include "pr/pr_stage2_ot_draw_backend.h"
#include "pr/pr_stage2_scene_entry.h"
#include "pr/pr_stage2_loading_draw_backend.h"
#include "pr/pr_stage2_tim_direct.h"
#include <array>
#include <fstream>
#include <iostream>
#include <iterator>
#include <vector>
#include <windows.h>

namespace {
using Args=std::initializer_list<uint32_t>;
void Check(bool b,const char* why){if(!b)throw std::runtime_error(why);}
struct Scene final:PrStage2SceneEntry::Services {
    using Services::Services;
    PrPsxGteDirect::MatrixRegisters gte{};
    int32_t CallPlatform(uint32_t fn,Args args)override{int32_t r;if(PrStage2LoadingPacketsDirect::TryCall(*this,fn,args,r))return r;throw std::runtime_error("No packet platform receipt");}
    void CallPlatformVoid(uint32_t,Args)override{throw std::runtime_error("No platform void receipt");}
    uint8_t ReadDevice8(uint32_t)override{throw std::runtime_error("No MMIO8");}
    uint16_t ReadDevice16(uint32_t)override{throw std::runtime_error("No MMIO16");}
    uint32_t ReadDevice32(uint32_t)override{throw std::runtime_error("No MMIO32");}
    void WriteDevice8(uint32_t,uint8_t)override{throw std::runtime_error("No MMIO8");}
    void WriteDevice16(uint32_t,uint16_t)override{throw std::runtime_error("No MMIO16");}
    void WriteDevice32(uint32_t,uint32_t)override{throw std::runtime_error("No MMIO32");}
    PrStage2LifecycleDirect::Words64 Call64(uint32_t,Args)override{throw std::runtime_error("No wide call");}
    PrStage2LifecycleDirect::Vector32 NormalizeVector8003A3DC(PrStage2LifecycleDirect::Vector32)override{throw std::runtime_error("No vector call");}
    int32_t SetCdLocation800367A4(uint32_t)override{throw std::runtime_error("No CD call");}
    int32_t LoadImage80044D64(PrStage2LifecycleDirect::ImageRect,uint32_t)override{throw std::runtime_error("No implicit upload");}
    PrPsxGteDirect::MatrixRegisters& MatrixGte()override{return gte;}
    [[noreturn]]void Exit(uint32_t,Args)override{throw std::runtime_error("Exit");}
    [[noreturn]]void Break(uint32_t,uint32_t)override{throw std::runtime_error("Break");}
};
struct Window {
    HWND handle=CreateWindowExW(0,L"STATIC",L"Stage2 native Loading output probe",WS_OVERLAPPEDWINDOW,
                              CW_USEDEFAULT,CW_USEDEFAULT,400,300,nullptr,nullptr,GetModuleHandleW(nullptr),nullptr);
    ~Window(){if(handle)DestroyWindow(handle);}
};
std::vector<uint32_t> Pixels(D3D11Renderer& renderer) {
    ID3D11ShaderResourceView* captured=nullptr;
    Check(renderer.CaptureFrameTexture(captured),"Capture GPU frame");
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> view;view.Attach(captured);
    Microsoft::WRL::ComPtr<ID3D11Resource> resource;view->GetResource(resource.GetAddressOf());
    Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;Check(SUCCEEDED(resource.As(&texture)),"Captured resource type");
    Microsoft::WRL::ComPtr<ID3D11Device> device;texture->GetDevice(device.GetAddressOf());
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;device->GetImmediateContext(context.GetAddressOf());
    D3D11_TEXTURE2D_DESC desc{};texture->GetDesc(&desc);
    Check(desc.Format==DXGI_FORMAT_R8G8B8A8_UNORM,"Unexpected pixel format");
    desc.Usage=D3D11_USAGE_STAGING;desc.BindFlags=0;desc.CPUAccessFlags=D3D11_CPU_ACCESS_READ;desc.MiscFlags=0;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> staging;Check(SUCCEEDED(device->CreateTexture2D(&desc,nullptr,staging.GetAddressOf())),"Staging texture");
    context->CopyResource(staging.Get(),texture.Get());D3D11_MAPPED_SUBRESOURCE mapped{};
    Check(SUCCEEDED(context->Map(staging.Get(),0,D3D11_MAP_READ,0,&mapped)),"GPU map");
    std::vector<uint32_t> pixels(desc.Width*desc.Height);
    for(uint32_t y=0;y<desc.Height;++y)memcpy(pixels.data()+y*desc.Width,static_cast<const uint8_t*>(mapped.pData)+y*mapped.RowPitch,desc.Width*4u);
    context->Unmap(staging.Get(),0);return pixels;
}
uint32_t Word(const std::vector<uint8_t>& b,size_t p){Check(p+4<=b.size(),"Archive truncated");return uint32_t(b[p])|(uint32_t(b[p+1])<<8)|(uint32_t(b[p+2])<<16)|(uint32_t(b[p+3])<<24);}
void PreloadCommon(Scene& s,PrStage2VramDevice::Device& vram,const std::filesystem::path& path) {
    // Explicit resident-common-assets precondition; not a claim that S2's
    // asynchronous InitScene/INT loader has completed or a DMA completion.
    std::ifstream file(path,std::ios::binary);Check(bool(file),"Common asset missing");
    std::vector<uint8_t> bytes((std::istreambuf_iterator<char>(file)),{});
    constexpr uint32_t archive=0x800A0000u,info=0x801B0000u;
    Check(bytes.size()<info-archive,"Common archive bound");
    for(uint32_t i=0;i<bytes.size();++i)s.Write8(archive+i,bytes[i]);
    uint32_t tims=0;
    for(uint32_t at=0;at+8192u<=bytes.size();) {
        const auto kind=Word(bytes,at),count=Word(bytes,at+4),sectors=Word(bytes,at+8);if(kind==0xFFFFFFFFu)break;
        uint32_t payload=at+8192u;
        if(kind==1u)for(uint32_t i=0;i<count;++i) {
            const uint32_t size=Word(bytes,at+16u+i*20u);Check(size<=bytes.size()-payload&&Word(bytes,payload)==16u,"Common TIM layout");
            PrStage2TimDirect::GetTimInfo80040EAC(s,archive+payload+4u,info);
            auto upload=[&](uint32_t rect,uint32_t src){vram.UploadImageWords(s,{s.Read16(rect),s.Read16(rect+2),s.Read16(rect+4),s.Read16(rect+6)},s.Read32(src));};
            upload(info+4u,info+12u);if(s.Read32(info)&8u)upload(info+16u,info+24u);
            payload+=size;++tims;
        }
        Check(sectors<=bytes.size()/2048u,"Common sector count");at+=8192u+sectors*2048u;
    }
    std::cout<<"common-resident-tims "<<tims<<' '<<vram.UploadCount()<<'\n';
}
struct PixelRange {uint32_t low,high;};
std::vector<PixelRange> Reference(Scene& s,const PrStage2VramDevice::Device& vram,uint32_t mode) {
    std::vector<PixelRange> out(320u*240u,{0xFF2F1F11u,0xFF2F1F11u});
    const uint32_t style=mode==2?1:mode==3?2:mode==4?3:0;
    const uint32_t color=s.Read32(0x800508B0u+8u*style);
    Check((color&0xFF000000u)==0x40000000u,"Original highlight must select ABR0 semi-transparency");
    const uint32_t box=0xFF000000u|((color&255u)<<16u)|(color&0xFF00u)|((color>>16u)&255u);
    for(uint32_t row=0;row<12;++row)for(uint32_t col=0;col<16;++col) {
        uint32_t tile=s.Read32(0x80050420u+4u*(row*16u+col));uint32_t tpl=s.Read32(0x80050720u+4u*tile);
        const uint32_t tx=s.Read16(tpl+4),ty=s.Read16(tpl+6),cx=s.Read16(tpl+12),cy=s.Read16(tpl+14);
        const bool highlight=s.Read32(0x80087330u+4u*(row*16u+col))!=0;
        for(uint32_t y=0;y<20;++y)for(uint32_t x=0;x<20;++x) {
            const uint32_t a=(ty+y)*1024u+tx+x/4u;Check(vram.Known()[a]!=0,"Unknown reference texel");
            const uint16_t packed=vram.Words()[a];const uint32_t index=(packed>>((x%4u)*4u))&15u;
            const uint32_t palette=cy*1024u+cx+index;Check(vram.Known()[palette]!=0,"Unknown reference palette");
            const uint16_t c=vram.Words()[palette];uint32_t value=out[(row*20+y)*320+col*20+x].low;
            if(c!=0u) {
                const auto expand=[](uint32_t five){return (five<<3u)|(five>>2u);};
                value=0xFF000000u|expand(c&31u)|(expand((c>>5u)&31u)<<8u)|(expand((c>>10u)&31u)<<16u);
            }
            PixelRange pixel{value,value};
            if(highlight) {
                pixel={0xFF000000u,0xFF000000u};
                for(uint32_t shift:{0u,8u,16u}) {
                    const uint32_t sum=((value>>shift)&255u)+((box>>shift)&255u);
                    // Only exact half-integer ties have two permitted UNORM
                    // outcomes. Every other channel remains byte-exact.
                    pixel.low|=(sum/2u)<<shift;pixel.high|=((sum+1u)/2u)<<shift;
                }
            }
            out[(row*20+y)*320+col*20+x]=pixel;
        }
    }
    return out;
}
}
int main(int argc,char** argv) {
    try {
        Check(argc==5,"Arguments: SCUS COMOD2 common.INT output-directory");
        uint64_t checked=0;uint32_t negative=0;
        for(int scale:{1,4}) {
            Window window;Check(window.handle!=nullptr,"Create D3D window");D3D11Renderer renderer;
            Check(renderer.Initialize(window.handle,320*scale,240*scale),"D3D initialization");
            Scene scene(std::filesystem::u8path(argv[1]),std::filesystem::u8path(argv[2]));Check(scene.InitializeGlobals()==2,"S2 globals");
            PrStage2VramDevice::Device vram;PreloadCommon(scene,vram,std::filesystem::u8path(argv[3]));PrStage2VramAtlas::Projection atlas(vram);
            for(uint32_t mode:{0u,2u,3u,4u}) {

                scene.Call(0x8001EF14u,{});
                PrStage2LoadingWorkDirect::Initialize8001E6D0(scene);
                PrStage2OtcDevice::Device otc;PrStage2InterruptDevice::Controller interrupts;
                // Independent pixel projection precondition, not a claim of
                // completed scene bootstrap or a wall-clock frame producer.
                scene.Write16(0x800917AAu,160u);scene.Write16(0x800917ACu,120u);
                scene.Write32(0x8006EDA8u,0u);
                interrupts.TryWrite(0x1F8010F0u,4u,0x08000000u);
                for(uint32_t tick=0;tick<96u;++tick){
                    otc.TryWrite(0x1F8010E0u,4u,0x872ECu);
                    otc.TryWrite(0x1F8010E4u,4u,16u);
                    otc.TryWrite(0x1F8010E8u,4u,0x11000002u);
                    Check(otc.Complete(otc.Serial(),scene,interrupts),"Real OTC pixel prerequisite failed");
                    scene.Write32(0x800872B0u,0x5D7F0u); // Original ClearOTagR shared-tail postcondition.
                    scene.Write32(0x800901C8u,scene.Read32(0x8006ED50u));
                    PrStage2LoadingPatternDirect::DrawPattern8001EF40(scene,mode,0u);
                }
                // Raw RGB must be visually inert even when its payload is not
                // the native canonical zero. Mutate only those ignored bytes.
                uint32_t link=scene.Read32(0x800872B0u)&0xFFFFFFu;
                while(link!=0x5D7F0u){
                    const uint32_t packet=0x80000000u|link;
                    if(scene.Read8(packet+11u)==0x65u)
                        scene.Write32(packet+8u,0x65000000u|((packet*0x10203u)&0xFFFFFFu));
                    link=scene.Read32(packet)&0xFFFFFFu;
                }
                auto frame=PrStage2OtDrawBackend::DecodeLinkedList(scene,0x800872ECu,{0,0,0,0,0,319,239});
                Check(otc.CompletedTransfers()==96u&&otc.WrittenWords()==1536u,"OTC pixel transfer count");
                Check(scene.Read32(0x8006EB0Cu)==96u,"Pattern state duplicated");
                if(mode==0u) {
                    PrStage2VramDevice::Device missing;PrStage2VramAtlas::Projection empty(missing);renderer.BeginFrame(1,1,1);
                    bool failed=false;try{PrStage2OtDrawBackend::Render(frame,empty,renderer,{0,0,320,240,0,0,float(320*scale),float(240*scale)});}catch(const std::exception&){failed=true;}
                    Check(failed,"Missing texture was silently accepted");for(auto p:Pixels(renderer))Check((p&0xFFFFFFu)==0xFFFFFFu,"Partial frame on texture failure");++negative;
                }
                renderer.BeginFrame(17.0f/255.0f,31.0f/255.0f,47.0f/255.0f);
                PrStage2OtDrawBackend::Render(frame,atlas,renderer,{0,0,320,240,0,0,float(320*scale),float(240*scale)});const auto pixels=Pixels(renderer);const auto expected=Reference(scene,vram,mode);
                for(int y=0;y<240*scale;++y)for(int x=0;x<320*scale;++x) {
                    const uint32_t a=pixels[y*320*scale+x];const auto b=expected[(y/scale)*320+x/scale];
                    for(uint32_t shift:{0u,8u,16u}) {
                        const uint32_t channel=(a>>shift)&255u;
                        if(channel<((b.low>>shift)&255u)||channel>((b.high>>shift)&255u)) {
                            std::cerr<<"pixel "<<scale<<' '<<mode<<' '<<x<<' '<<y<<' '<<std::hex<<a<<' '<<b.low<<' '<<b.high<<std::dec<<'\n';throw std::runtime_error("Loading pixel mismatch");
                        }
                    }
                    ++checked;
                }
                if(mode==2u) {auto path=std::filesystem::u8path(argv[4])/("stage2_loading_packets_"+std::to_string(scale)+"x.png");Check(renderer.SaveScreenshot(path.wstring()),"Loading screenshot");}
                renderer.EndFrame();
                std::cout<<"loading-pixels "<<scale<<' '<<mode<<' '<<frame.commands.size()<<' '<<pixels.size()<<" source-vram\n";
            }
        }

        // Direct decoder negatives use fully described packets, not implicit
        // clipping/wrapping substitutes. Raw packet RGB is intentionally inert.
        Scene negativeScene(std::filesystem::u8path(argv[1]),std::filesystem::u8path(argv[2]));
        constexpr uint32_t p=0x800B1000u;
        auto set=[&](uint32_t uv,uint32_t size,uint32_t tag=0x04FFFFFFu){
            negativeScene.Write32(p,tag);negativeScene.Write32(p+4u,0x65ABCDEFu);
            negativeScene.Write32(p+8u,0u);negativeScene.Write32(p+12u,uv);negativeScene.Write32(p+16u,size);
        };
        uint32_t rejects=0;
        for(auto pair:std::vector<std::array<uint32_t,2>>{{255u,0x10002u},{256u*255u,0x20002u},{0u,0x10400u}}){
            set(pair[0],pair[1]);bool caught=false;
            try{PrStage2OtDrawBackend::DecodeLinkedList(negativeScene,p,{});}catch(const std::exception&){caught=true;}
            Check(caught,"Unsupported rectangle silently accepted");++rejects;
        }
        set(0u,0x10001u,0x03FFFFFFu);bool truncated=false;
        try{PrStage2OtDrawBackend::DecodeLinkedList(negativeScene,p,{});}catch(const std::exception&){truncated=true;}
        Check(truncated,"Truncated sprite packet accepted");++rejects;
        set(1u,0x10001u);const auto odd=PrStage2OtDrawBackend::DecodeLinkedList(negativeScene,p,{});
        Check(odd.commands.size()==1u&&odd.commands[0].vertex[0].u==1u,
              "Native odd-U sprite coordinate was rejected or rewritten");
        std::cout<<"packet-decoder-negatives "<<rejects<<"\n";
        std::cout<<"loading-pixels-pass "<<checked<<' '<<negative<<" missing-texture-rejections\n";return 0;
    }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 2;}
}
