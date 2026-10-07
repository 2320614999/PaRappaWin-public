#include "test_stage2_gpu_image_readback.h"
#include "pr/pr_stage2_scene_entry.h"
#include "pr/pr_stage2_gpu_device.h"
#include "pr/pr_stage2_vram_device.h"
#include "pr/pr_stage2_game_hud_direct.h"
#include "pr/pr_display_viewport.h"
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
// 用独立像素数组核对命令顺序、跨页复制和已呈现快照的生命周期。
void RunLegacyAspect(D3D11Renderer& renderer) {
    const float w=float(renderer.GetWidth()), h=float(renderer.GetHeight());
    const float scale=(std::min)(w/320,h/240), x0=(w-320*scale)/2, y0=(h-240*scale)/2;
    const uint32_t white=0xFFFFFFFFu;
    ComPtr<ID3D11ShaderResourceView> texture;
    texture.Attach(renderer.CreateTexture(&white,1,1));
    for(int mode:{1,2,0,1}) {
        renderer.BeginFrame(0,0,0);
        renderer.SetStageAspectMode(mode);
        renderer.DrawRect(0,0,w,h,0,1,0,1);
        renderer.DrawSpriteTint(texture.Get(),x0+40*scale,y0+90*scale,40*scale,40*scale,0,0,1,1,1,0,0,1);
        ColorVertex color[6];TexturedVertex textured[6];
        const int corners[6]={0,1,2,2,1,3};
        for(int n=0;n<6;++n) {
            const int c=corners[n];
            color[n]={x0+(140+40*(c&1))*scale,y0+(90+40*((c>>1)&1))*scale,0,0,1,1};
            textured[n]={x0+(240+40*(c&1))*scale,y0+(90+40*((c>>1)&1))*scale,float(c&1),float((c>>1)&1),1,1,0,1};
        }
        renderer.DrawTriangleBatch(color,6,D3D11Renderer::BlendMode::Alpha,true);
        renderer.DrawTexturedTriangleBatch(texture.Get(),textured,6,D3D11Renderer::BlendMode::Alpha,D3D11Renderer::TextureMask::All,true);
        ComPtr<ID3D11ShaderResourceView> frame;
        Check(renderer.CaptureFrameTexture(*frame.GetAddressOf()),"Legacy aspect capture");
        const auto image=TestStage2GpuReadback::Read(frame.Get());
        for(unsigned y=0;y<image.height;++y)for(unsigned x=0;x<image.width;++x) {
            const float px=mode==0?(x+.5f)*320/w:(x+.5f-x0)/scale;
            const float py=mode==0?(y+.5f)*240/h:(y+.5f-y0)/scale;
            uint32_t expected=0xFF00FF00u;
            if(mode==2 && (px<0 || px>=320 || py<0 || py>=240))expected=0xFF000000u;
            else if(py>=90 && py<130) {
                if(px>=40 && px<80)expected=0xFF0000FFu;
                if(px>=140 && px<180)expected=0xFFFF0000u;
                if(px>=240 && px<280)expected=0xFF00FFFFu;
            }
            if(image.rgba[size_t(y)*image.width+x]!=expected) {
                std::cerr<<"legacy-aspect mode="<<mode<<" size="<<w<<"x"<<h<<" xy="<<x<<","<<y<<" got="<<std::hex<<image.rgba[size_t(y)*image.width+x]<<" expected="<<expected<<std::dec<<"\n";
                Check(false,"Legacy shared aspect output pixel");
            }
        }
    }
    // BeginFrame must clear the legacy mapping before a native S2 or menu draw.
    renderer.SetStageAspectMode(0);renderer.BeginFrame(0,0,0);
    renderer.DrawRect(0,0,10,10,1,0,0,1);
    ComPtr<ID3D11ShaderResourceView> reset;
    Check(renderer.CaptureFrameTexture(*reset.GetAddressOf()),"Aspect reset capture");
    const auto image=TestStage2GpuReadback::Read(reset.Get());
    Check(image.rgba[5*image.width+5]==0xFF0000FFu && image.rgba[5*image.width+11]==0xFF000000u,"Aspect leaked across scene boundary");
    std::cout<<"legacy-stage-aspect "<<int(w)<<"x"<<int(h)<<" auto classic stretch reset sprites color texture pixels\n";
}
void RunMoveImage(Memory& m,D3D11Renderer& renderer){
    PrStage2VramDevice::Device vram;PrStage2VramAtlas::Projection atlas(vram);
    PrStage2InterruptDevice::Controller irq;
    G::Device d(m,atlas,irq,renderer,{0,0,320,480,0,0,64,48});
    constexpr uint32_t head=0x800B8000u,next=0x800B9000u;
    std::vector<uint32_t> expected(64u*96u,0xFF000000u),words;
    uint32_t pixels=0;
    // 测试颜色和区域直接写入独立 CPU 预期，不读取被测解码器的结果。
    const auto fill=[&](int x,int y,int width,int height,uint32_t rgb){
        words.insert(words.end(),{0x02000000u|rgb,uint32_t(x)|(uint32_t(y)<<16u),uint32_t(width)|(uint32_t(height)<<16u)});
        for(int py=y/5;py<(y+height)/5;++py)for(int px=x/5;px<(x+width)/5;++px)
            expected[size_t(py)*64u+px]=0xFF000000u|rgb;
    };
    // 仅用于测试非重叠区域；来源快照独立于产品的页纹理实现。
    const auto copy=[&](int sx,int sy,int dx,int dy,int width,int height){
        words.insert(words.end(),{0x80000000u,uint32_t(sx)|(uint32_t(sy)<<16u),uint32_t(dx)|(uint32_t(dy)<<16u),uint32_t(width)|(uint32_t(height)<<16u)});
        const auto before=expected;
        for(int y=0;y<height/5;++y)for(int x=0;x<width/5;++x)
            expected[size_t(dy/5+y)*64u+dx/5+x]=before[size_t(sy/5+y)*64u+sx/5+x];
    };
    // DMA 分段可以切在 GP0 指令中间，完成信号必须等待同一真实 fence。
    const auto submit=[&](uint32_t split=0u){
        if(!split)split=uint32_t(words.size());
        Check(split<=words.size()&&split<=255u,"复制测试包长度错误");
        m.Write32(head,(split<<24u)|(split==words.size()?0xFFFFFFu:next&0xFFFFFFu));
        for(uint32_t i=0;i<split;++i)m.Write32(head+4u+4u*i,words[i]);
        if(split<words.size()){
            m.Write32(next,(uint32_t(words.size()-split)<<24u)|0xFFFFFFu);
            for(uint32_t i=split;i<words.size();++i)m.Write32(next+4u+4u*(i-split),words[i]);
        }
        irq.TryWrite(0x1F8010F4u,4u,0x04840000u);irq.TryWrite(0x1F801070u,2u,0u);
        const auto completed=d.Completions();Arm(d,head);
        Check(d.Service()==G::Progress::Submitted&&d.Pending()&&d.Completions()==completed,"复制提交伪造完成");
        Check(irq.Pending()==0u,"复制在 fence 前发布中断");
        for(int i=0;i<8;++i)Check((Read(d,0x1F8010A8u)&0x01000000u)&&d.Completions()==completed,"状态读取提前完成复制");
        Fence(d);Check(d.Completions()==completed+1u&&irq.Pending()==8u,"复制 fence 未发布 DMA2 完成");
        words.clear();
    };
    // 每次提交后读回两页所有像素，同时检查呈现实际成功。
    const auto compare=[&]{
        for(uint32_t page=0;page<2u;++page){
            Write(d,0x1F801814u,0x05000000u|((page*240u)<<10u));
            Check(d.PresentDisplay(),"复制后页面没有呈现");
            const auto actual=TestStage2GpuReadback::Read(d.PresentedImage().Get()).rgba;
            Check(actual.size()==3072u,"复制页面尺寸错误");
            for(size_t i=0;i<actual.size();++i){Check(actual[i]==expected[size_t(page)*3072u+i],"复制页面像素或执行顺序错误");++pixels;}
        }
    };
    irq.TryWrite(0x1F8010F0u,4u,0x800u);irq.TryWrite(0x1F801074u,2u,8u);
    Write(d,0x1F801814u,0x04000002u);Write(d,0x1F801814u,0x03000000u);
    fill(0,0,320,480,0);fill(0,0,320,240,0x0000FFu);fill(0,240,320,240,0xFF0000u);submit();compare();
    const auto snapshot=d.PresentedImage();
    const auto snapshotPixels=TestStage2GpuReadback::Read(snapshot.Get()).rgba;
    // 前置绘图、复制、后置绘图、再次复制必须按原指令顺序执行。
    fill(0,0,320,240,0x001122u);copy(0,0,0,240,320,240);
    fill(0,240,80,240,0x00FF00u);copy(0,240,0,0,320,240);submit();compare();
    Check(TestStage2GpuReadback::Read(snapshot.Get()).rgba==snapshotPixels,"复制修改了旧呈现快照");
    // 小区域目标保留未覆盖部分，随后改写来源不能改变复制结果。
    copy(0,0,160,240,80,120);fill(0,0,80,120,0x00FFFFu);submit();compare();
    copy(0,120,160,0,80,240);submit();compare();
    copy(0,0,0,0,320,240);copy(0,0,0,240,320,240);submit(2u);compare();
    Check(d.MoveImageCopies()==6u&&d.Completions()==5u,"复制次数或 fence 数量错误");
    // 解码边界独立验证：坐标掩码和零尺寸不受绘图偏移、裁剪影响。
    PrStage2OtDrawBackend::DrawState state;state.offsetX=99;state.offsetY=-33;
    const auto decoded=PrStage2OtDrawBackend::DecodeCommandStream({0x9FFFFFFFu,0xFFFFFC05u,0xFE05FC07u,0u},state);
    const auto& c=decoded.commands.at(0);
    Check(c.copySourceX==5u&&c.copySourceY==511u&&c.copyDestinationX==7u&&c.copyDestinationY==5u&&
          c.fillWidth==1024u&&c.fillHeight==512u,"复制坐标或零尺寸解码错误");
    std::cout<<"gpu-move-contract copies "<<d.MoveImageCopies()<<" fences "<<d.Completions()<<" pixels "<<pixels<<" ordered split-node snapshot irq\n";
}
void RunMoviePresent(Memory& m,D3D11Renderer& renderer){
    PrStage2VramDevice::Device vram;PrStage2VramAtlas::Projection atlas(vram);
    PrStage2InterruptDevice::Controller irq;
    G::Device d(m,atlas,irq,renderer,{0,0,320,480,0,0,64,48});
    constexpr uint32_t head=0x800B8000u;
    irq.TryWrite(0x1F8010F0u,4u,0x800u);
    Write(d,0x1F801814u,0x04000002u);FullClear(m,head);Arm(d,head);
    Check(d.Service()==G::Progress::Submitted,"Movie fixture clear not submitted");Fence(d);
    Write(d,0x1F801814u,0x03000000u);
    const auto present=[&](uint32_t page){
        Write(d,0x1F801814u,0x05000000u|((page*240u)<<10u));
        Check(d.PresentDisplay(),"Movie page not presented");
    };
    const auto solid=[&](uint32_t color){
        for(uint32_t pixel:TestStage2GpuReadback::Read(d.PresentedImage().Get()).rgba)
            Check(pixel==color,"Movie/transition actual visible pixels differ");
    };
    const std::vector<uint16_t> red(320u*240u,31u),blue(320u*240u,31u<<10u);
    d.CopyFramebufferImage({0,0,320,240},red,1u);present(0);solid(0xFF0000FFu);
    const auto snapshot=d.PresentedImage();
    // The source uploads the same frame into both pages as separate strips.
    d.CopyFramebufferImage({0,240,320,240},red,1u);present(1);
    d.CopyFramebufferImage({0,240,16,240},std::vector<uint16_t>(16u*240u,31u),1u);present(1);
    Check(d.DisplayPresents()==1u,"Repeated movie strips queued duplicate presents");
    Check(d.PresentedImage().Get()==snapshot.Get(),"Duplicate changed the visible snapshot");
    d.CopyFramebufferImage({0,0,320,240},blue,2u);present(0);solid(0xFFFF0000u);
    Check(d.DisplayPresents()==2u,"Next decoded movie frame not displayed once");
    // Stop precedes transition geometry and permits both pages again.
    d.ClearMovieIdentity();present(1);solid(0xFF0000FFu);present(0);solid(0xFFFF0000u);
    Check(d.DisplayPresents()==4u,"Movie stop suppressed transition page refreshes");
    d.CopyFramebufferImage({0,0,320,240},red,3u);present(0);solid(0xFF0000FFu);
    present(0);Check(d.DisplayPresents()==5u,"A later movie lost de-duplication");
    Check(d.Completions()==1u,"Movie scanout fabricated DMA completion");
    std::cout<<"gpu-movie-present-contract repeated-strips next-frame stop-transition later-movie 15360 pixels\n";
}
void RunMovieStrips(Memory& m,D3D11Renderer& renderer){
    PrStage2VramDevice::Device vram;PrStage2VramAtlas::Projection atlas(vram);
    PrStage2InterruptDevice::Controller irq;
    G::Device d(m,atlas,irq,renderer,{0,0,320,480,0,0,float(renderer.GetWidth()),float(renderer.GetHeight())});
    constexpr uint32_t head=0x800B8000u;
    irq.TryWrite(0x1F8010F0u,4u,0x800u);Write(d,0x1F801814u,0x04000002u);
    FullClear(m,head);Arm(d,head);d.Service();Fence(d);Write(d,0x1F801814u,0x03000000u);
    const int scale=renderer.GetWidth()/320;
    for(uint32_t frame=1;frame<=4;++frame){
        const uint32_t page=(frame-1)%2;
        const int ox=frame<=2?32:0,oy=frame<=2?48:0,w=frame<=2?256:320,h=frame<=2?144:240;
        const uint64_t before=d.CpuImageCopies();
        for(int x=0;x<w;x+=16){
            std::vector<uint16_t> strip(16*h);
            for(int y=0;y<h;++y)for(int col=0;col<16;++col)
                strip[y*16+col]=uint16_t((x+col<w/2?31:31<<10)|(y<h/2?0:31<<5));
            d.CopyFramebufferImage({uint16_t(ox+x),uint16_t(oy+page*240),16,uint16_t(h)},strip,frame,uint16_t(w));
            Check(d.CpuImageCopies()==before+(x+16==w?1:0),"Movie strips must commit exactly one complete page image");
        }
        Write(d,0x1F801814u,0x05000000u|((page*240u)<<10u));d.PresentDisplay();
        const auto pixels=TestStage2GpuReadback::Read(d.PresentedImage().Get()).rgba;
        for(int y=0;y<renderer.GetHeight();++y)for(int x=0;x<renderer.GetWidth();++x){
            const int px=x/scale-ox,py=y/scale-oy;
            uint32_t expected=0xFF201008u;
            if(px>=0&&px<w&&py>=0&&py<h)
                expected=0xFF000000u|(px<w/2?0xFFu:0xFF0000u)|(py<h/2?0:0xFF00u);
            Check(pixels[y*renderer.GetWidth()+x]==expected,"Batched movie changed border, strip order or scaled rectangle pixels");
        }
    }
    Check(d.Completions()==1u&&d.DisplayPresents()==4u,"Movie upload changed DMA or display accounting");
    std::cout<<"gpu-movie-strips 256x144 320x240 scale="<<scale<<" copies=4 border-and-quadrant-pixels\n";
}
void RunLayerPersistence(Memory& m,D3D11Renderer& renderer){
    PrStage2VramDevice::Device vram;PrStage2VramAtlas::Projection atlas(vram);
    PrStage2InterruptDevice::Controller irq;
    G::Device d(m,atlas,irq,renderer,{0,0,320,480,0,0,64,48});
    constexpr uint32_t head=0x800B8000u;
    irq.TryWrite(0x1F8010F0u,4u,0x800u);Write(d,0x1F801814u,0x04000002u);
    const auto submit=[&](const std::vector<uint32_t>& words){
        m.Write32(head,(uint32_t(words.size())<<24u)|0xFFFFFFu);
        for(uint32_t i=0;i<words.size();++i)m.Write32(head+4u+4u*i,words[i]);
        Arm(d,head);Check(d.Service()==G::Progress::Submitted,"Layer submit failed");Fence(d);
    };
    submit({0x02201008u,0u,320u|(480u<<16u)});
    // Original half blend: red (200,0,0) over (8,16,32) is (104,8,16).
    submit({0xE3000000u,0xE4000000u|319u|(479u<<10u),0xE1000200u,
            0x620000C8u,50u|(50u<<16u),100u|(100u<<16u)});
    const auto before=TestStage2GpuReadback::Read(d.FramebufferImage(0).Get()).rgba;
    const uint32_t sample=before[20u*64u+20u]&0xFFFFFFu;
    Check(sample==0x100868u,"Native half blend has incorrect color");
    // A later ordering table draws elsewhere. The already-composited RGB must
    // survive its page restore; framebuffer alpha is not texture transparency.
    submit({0x6000FF00u,250u|(180u<<16u),20u|(20u<<16u)});
    const auto after=TestStage2GpuReadback::Read(d.FramebufferImage(0).Get()).rgba;
    Check((after[20u*64u+20u]&0xFFFFFFu)==sample,"A later ordering table blended the framebuffer alpha again");
    submit({0x200000FFu,200u|(40u<<16u),200u|(120u<<16u),280u|(40u<<16u)});
    const auto winding=TestStage2GpuReadback::Read(d.FramebufferImage(0).Get()).rgba;
    Check((winding[10u*64u+42u]&0xFFFFFFu)==255u,"Native GP0 polygon was culled a second time by D3D");
    std::cout<<"gpu-layer-persistence native-half-blend survives-later-ordering-table both-windings\n";
}
void RunNoteSprites(Memory& m,D3D11Renderer& renderer){
    PrStage2VramDevice::Device vram;PrStage2VramAtlas::Projection atlas(vram);
    PrStage2InterruptDevice::Controller irq;
    G::Device d(m,atlas,irq,renderer,{0,0,320,480,0,0,320,240});
    constexpr uint32_t source=0x800B4000u,pixels=0x800B5000u,table=0x80087288u;
    constexpr uint32_t slots=0x800B7000u,packet=0x800B8000u,head=0x800B9000u;
    for(uint32_t i=0;i<64u;++i)m.Write16(pixels+2u*i,(i%4u)<2u?0x1111u:0x2222u);
    vram.UploadImageWords(m,{640u,0u,4u,16u},pixels);
    for(uint32_t i=0;i<16u;++i)m.Write16(pixels+2u*i,i==1u?31u:i==2u?992u:0u);
    vram.UploadImageWords(m,{768u,500u,16u,1u},pixels);
    m.Write32(source,0x10000040u);m.Write16(source+4u,640u);m.Write16(source+6u,0u);
    m.Write16(source+8u,16u);m.Write16(source+10u,16u);m.Write16(source+12u,768u);m.Write16(source+14u,500u);
    m.Write32(0x8006EDA8u,0u);m.Write8(0x8005D734u,0u);
    m.Write16(0x800917AAu,160u);m.Write16(0x800917ACu,120u);
    m.gte.h=160u;m.gte.ofx=160<<16;m.gte.ofy=120<<16;
    for(uint32_t i=0;i<8u;++i)m.Write32(0x80091838u+4u*i,(i==0||i==2||i==4)?4096u:0u);
    irq.TryWrite(0x1F8010F0u,4u,0x800u);Write(d,0x1F801814u,0x04000002u);
    for(uint16_t type=1;type<=8u;++type)for(int scale:{2048,4096,8192,-4096}){
        FullClear(m,head);Arm(d,head);Check(d.Service()==G::Progress::Submitted,"Note clear failed");Fence(d);
        Write(d,0x1F801810u,0xE3000000u);Write(d,0x1F801810u,0xE4000000u|319u|(239u<<10u));
        m.Write32(0x800540BCu+4u*type,source);
        m.Write16(0x80087668u,uint16_t(scale));m.Write16(0x800876B0u,4096u);
        m.Write32(table+4u,slots);m.Write32(table+8u,0u);m.Write32(slots+4u,0xFFFFFFu);
        m.Write32(0x800901C8u,packet);
        PrStage2GameHudDirect::DrawNote80024418(m,160u,120u,0u,type);
        Check(m.Read32(0x800901C8u)==packet+(scale==4096?24u:40u),"Note packet path or allocator is wrong");
        Arm(d,slots+4u);Check(d.Service()==G::Progress::Submitted,"Note packet was not submitted");Fence(d);
        const auto image=TestStage2GpuReadback::Read(d.FramebufferImage(0).Get()).rgba;
        const int half=scale==2048?2:scale==8192?8:4;
        const uint32_t left=image[120u*320u+160u-half]&0xFFFFFFu,right=image[120u*320u+160u+half]&0xFFFFFFu;
        Check(left==(scale<0?0xFF00u:0xFFu)&&right==(scale<0?0xFFu:0xFF00u),"Scaled or mirrored note pixels are wrong");
        Check((image[90u*320u+160u]&0xFFFFFFu)==0x201008u,"Note drawing overwrote unrelated scene pixels");
    }
    std::cout<<"gpu-note-sprites eight-types four-scales centered-mirrored-live-OT\n";
}
void RunAspectViewport(Memory& m,D3D11Renderer& renderer) {
    namespace B=PrStage2OtDrawBackend;
    struct Presentation final:G::Presentation {
        int mode;D3D11Renderer& renderer;
        Presentation(int value,D3D11Renderer& r):mode(value),renderer(r){}
        void ConfigureViewport(B::Viewport& v) const override {
            v.x=v.y=0;v.width=float(renderer.GetWidth());v.height=float(renderer.GetHeight());
            if(mode==0)return;
            const auto fit=PrDisplayViewport::PreserveAspect(v.width,v.height,float(v.frameWidth),float(v.frameHeight));
            v.x=fit.x;v.y=fit.y;v.width=fit.width;v.height=fit.height;
        }
        B::Batch Prepare(const B::Batch& b,PrStage2VramAtlas::Projection&,const B::Viewport&,ID3D11ShaderResourceView*) override {
            auto out=b;
            for(auto& c:out.commands)if(c.opcode!=2u) {
                c.host=std::make_shared<B::HostPrimitive>();c.host->expandedViewport=mode==1;
                for(unsigned i=0;i<c.vertices;++i){const auto& a=c.vertex[i];c.host->vertex[i]={float(a.x+c.state.offsetX),float(a.y+c.state.offsetY),0,0,a.r/255.0f,a.g/255.0f,a.b/255.0f};}
            }
            return out;
        }
        bool Present(PrStage2VramAtlas::Projection&,D3D11Renderer&,const B::Viewport&) override{return false;}
        void Invalidate() override{}
    };
    const int w=renderer.GetWidth(),h=renderer.GetHeight();
    constexpr uint32_t head=0x800B8000u;
    const auto xy=[](int x,int y){return uint32_t(uint16_t(x))|(uint32_t(uint16_t(y))<<16u);};
    for(int mode:{0,1,2}) {
        PrStage2VramDevice::Device vram;PrStage2VramAtlas::Projection atlas(vram);
        PrStage2InterruptDevice::Controller irq;
        G::Device d(m,atlas,irq,renderer,{0,0,320,480,0,0,float(w),float(h)});
        Presentation presentation(mode,renderer);d.SetPresentation(&presentation);
        irq.TryWrite(0x1F8010F0u,4u,0x800u);Write(d,0x1F801814u,0x04000002u);
        FullClear(m,head);Arm(d,head);Check(d.Service()==G::Progress::Submitted,"Aspect clear submission");Fence(d);
        const uint32_t words[]={0xE3000000u,0xE4000000u|319u|(239u<<10u),
            0x280000FFu,xy(144,104),xy(176,104),xy(144,136),xy(176,136),
            0x2800FF00u,xy(-40,104),xy(-8,104),xy(-40,136),xy(-8,136)};
        m.Write32(head,(uint32_t(std::size(words))<<24u)|0xFFFFFFu);
        for(unsigned i=0;i<std::size(words);++i)m.Write32(head+4u+4u*i,words[i]);
        Arm(d,head);Check(d.Service()==G::Progress::Submitted,"Aspect model submission");Fence(d);
        auto pixels=TestStage2GpuReadback::Read(d.FramebufferImage(0).Get()).rgba;
        int redWidth=0,redHeight=0;
        for(int x=0;x<w;++x)redWidth+=(pixels[size_t(h/2)*w+x]&0xFFFFFFu)==0xFFu;
        for(int y=0;y<h;++y)redHeight+=(pixels[size_t(y)*w+w/2]&0xFFFFFFu)==0xFFu;
        Check(redHeight==48 && redWidth==(mode==0?w/10:48),"Aspect mode stretched a square or changed native framing");
        if(w>480) {
            const int edge=(w-480)/2-36;
            const bool green=(pixels[size_t(h/2)*w+edge]&0xFFFFFFu)==0xFF00u;
            Check(green==(mode==1),"Automatic scene extension or classic side clipping is wrong");
        }
        // Exercise the batched movie-strip path with the same viewport owner.
        for(int x=128;x<192;x+=16)d.CopyFramebufferImage({uint16_t(x),104,16,32},std::vector<uint16_t>(16u*32u,0x7C00u),1u,64u);
        pixels=TestStage2GpuReadback::Read(d.FramebufferImage(0).Get()).rgba;
        int blueWidth=0,blueHeight=0;
        for(int x=0;x<w;++x)blueWidth+=(pixels[size_t(h/2)*w+x]&0xFFFFFFu)==0xFF0000u;
        for(int y=0;y<h;++y)blueHeight+=(pixels[size_t(y)*w+w/2]&0xFFFFFFu)==0xFF0000u;
        Check(blueHeight==48 && blueWidth==(mode==0?w/5:96),"Movie output ignored aspect-preserving viewport");
        d.SetPresentation(nullptr);
    }
    std::cout<<"gpu-aspect-viewport width="<<w<<" auto classic stretch square side-content movie\n";
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
    Check(pixels==6144u&&d.DisplayPresents()==2u,"Missing two-page pixel/present verification");
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
    std::cout<<"gpu-device-contract two-pages 6144 pixels 1-fence 2-presents no-read-pump invalid-input-gates\n";
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
        Run(memory,renderer);RunMoveImage(memory,renderer);RunMoviePresent(memory,renderer);
        RunLayerPersistence(memory,renderer);
        for(int scale:{1,2}){
            const HWND movieWindow=CreateWindowExW(0,L"STATIC",L"Movie strip contract",WS_OVERLAPPEDWINDOW,0,0,700,550,nullptr,nullptr,GetModuleHandleW(nullptr),nullptr);
            Check(movieWindow!=nullptr,"Movie scale window creation failed");Guard movieGuard{movieWindow};
            D3D11Renderer movieRenderer;
            Check(movieRenderer.Initialize(movieWindow,320*scale,240*scale),"Movie scale renderer initialization failed");
            RunMovieStrips(memory,movieRenderer);
            if(scale==1)RunNoteSprites(memory,movieRenderer);
        }
        for(int width:{480,640,960}) {
            const HWND aspectWindow=CreateWindowExW(0,L"STATIC",L"Aspect contract",WS_OVERLAPPEDWINDOW,0,0,width+30,400,nullptr,nullptr,GetModuleHandleW(nullptr),nullptr);
            Check(aspectWindow!=nullptr,"Aspect test window creation");Guard aspectGuard{aspectWindow};
            D3D11Renderer aspectRenderer;Check(aspectRenderer.Initialize(aspectWindow,width,360),"Aspect renderer initialization");
            RunAspectViewport(memory,aspectRenderer);
            RunLegacyAspect(aspectRenderer);
        }
        {
            const HWND portraitWindow=CreateWindowExW(0,L"STATIC",L"Portrait aspect contract",WS_OVERLAPPEDWINDOW,0,0,390,680,nullptr,nullptr,GetModuleHandleW(nullptr),nullptr);
            Check(portraitWindow!=nullptr,"Portrait aspect test window creation");Guard portraitGuard{portraitWindow};
            D3D11Renderer portraitRenderer;
            Check(portraitRenderer.Initialize(portraitWindow,360,640),"Portrait aspect renderer initialization");
            RunLegacyAspect(portraitRenderer);
        }
        std::cout<<"gpu-device-pass "<<checks<<" assertions\n";return 0;
    }catch(const std::exception&e){std::cerr<<e.what()<<'\n';return 2;}
}
