#include "test_stage2_disc_file_services.h"
#include "pr/pr_stage2_ot_draw_backend.h"
#include "pr/pr_stage2_gpu_direct.h"
#include <algorithm>
#include <cmath>
#include <set>
#include <map>

namespace {
namespace Ot=PrStage2OtDrawBackend;
void Check(bool condition,const char* error) { if (!condition) throw std::runtime_error(error); }
// Explicit test-device scheduling: starting DMA only arms a transfer. PumpOne
// consumes the native RAM through the real D3D adapter, then acknowledges DMA
// and invokes the registered native drain callback. This is not the product
// interrupt controller, VBlank driver or a fabricated successful DrawOTag.
struct QueueGpuFixture : DiskServices {
    using DiskServices::DiskServices;
    std::map<uint32_t,uint32_t> io{{0x1F8010A0u,0},{0x1F8010A4u,0},{0x1F8010A8u,0},{0x1F8010F0u,0},
                                  {0x1F8010E0u,0},{0x1F8010E4u,0},{0x1F8010E8u,0}};
    uint32_t interruptMask=8u, dmaCallback=0u, starts=0u, completions=0u;
    bool pending=false;
    bool otcPending=false, pumpOnStatus=false, pumping=false;
    uint32_t otcTransfers=0u, otcWords=0u, gpuReadback=0u;
    std::vector<uint32_t> infoCommands;
    std::vector<uint32_t> environmentCommands;
    uint32_t displayX=0u,displayY=0u; bool displayEnabled=false;
    std::vector<uint32_t> consumedHeads;
    D3D11Renderer* renderer=nullptr;
    PrStage2VramAtlas::Projection* projection=nullptr;
    Ot::DrawState state{};
    Ot::Viewport view{};
    uint32_t Read32(uint32_t address) override {
        if (address==0x1F801814u) return 0x04000000u;
        if (address==0x1F801810u) return gpuReadback;
        if (address==0x1F8010A8u && pending && pumpOnStatus && !pumping) PumpOne();
        if (address==0x1F8010E8u && otcPending) {
            // Explicit test-device completion on polling, separate from the
            // original CPU function. Never synthesize a ClearOTagR return.
            const uint32_t count=io.at(0x1F8010E4u), last=io.at(0x1F8010E0u)&0xFFFFFFu;
            Check(count && count<=0x40000u && last<0x200000u && (last&3u)==0u && 4u*(count-1u)<=last,
                  "unsupported OTC test transaction");
            for (uint32_t i=0;i<count;++i) {
                const uint32_t link=last-4u*i;
                DiskServices::Write32(0x80000000u|link,i+1u==count?0xFFFFFFu:(link-4u)&0xFFFFFFu);
            }
            ++otcTransfers; otcWords+=count; otcPending=false; io[0x1F8010E8u]&=~0x01000000u;
        }
        const auto found=io.find(address);
        return found!=io.end()?found->second:DiskServices::Read32(address);
    }
    void Write32(uint32_t address,uint32_t value) override {
        if (address==0x1F801814u) {
            if ((value&0xFFFFFFF8u)==0x10000000u) {
                const uint32_t command=value&7u; infoCommands.push_back(command);
                if (command==3u) gpuReadback=uint32_t(state.clipLeft)|(uint32_t(state.clipTop)<<10u);
                else if (command==4u) gpuReadback=uint32_t(state.clipRight)|(uint32_t(state.clipBottom)<<10u);
                else if (command==5u) gpuReadback=(uint32_t(state.offsetX)&0x7FFu)|((uint32_t(state.offsetY)&0x7FFu)<<11u);
                else Check(false,"unsupported test GPU info request");
                return;
            }
            const uint32_t opcode=value>>24u;
            if (opcode==3u || (opcode>=5u && opcode<=8u)) {
                environmentCommands.push_back(value);
                if (opcode==3u) displayEnabled=(value&1u)==0u;
                if (opcode==5u) { displayX=value&0x3FFu; displayY=(value>>10u)&0x3FFu; }
                return;
            }
            Check(value==0x04000002u,"unexpected GP1 test command"); return;
        }
        const auto found=io.find(address);
        if (found==io.end()) { DiskServices::Write32(address,value); return; }
        if (address==0x1F8010A8u && (value&0x01000000u)) {
            Check(!pending && value==0x01000401u,"invalid fixture DMA start");
            pending=true; ++starts;
        }
        if (address==0x1F8010E8u && (value&0x01000000u)) {
            Check(!otcPending && value==0x11000002u,"invalid OTC control"); otcPending=true;
        }
        found->second=value;
    }
    int32_t Call(uint32_t fn,std::initializer_list<uint32_t> args) override {
        const std::vector<uint32_t> a(args);
        if (fn==0x80035560u && a==std::vector<uint32_t>{0xFFFFFFFFu}) return 0;
        if (fn==0x800358C0u && a.size()==1u) {
            const uint32_t old=interruptMask; interruptMask=a[0]; return static_cast<int32_t>(old);
        }
        if (fn==0x800357A4u && a.size()==2u && a[0]==2u) {
            const uint32_t old=dmaCallback; dmaCallback=a[1]; return static_cast<int32_t>(old);
        }
        if (fn==0x80047FBCu && a.size()==3u) {
            // Actual isolated BIOS-copy device, restricted to these buffers.
            Check(a[2]<=92u && a[0]>=0x80000000u && a[0]<=0x80200000u-a[2] &&
                  a[1]>=0x80000000u && a[1]<=0x80200000u-a[2],"invalid BIOS copy fixture");
            std::vector<uint8_t> bytes(a[2]);
            for (uint32_t i=0;i<a[2];++i) bytes[i]=Read8(a[1]+i);
            for (uint32_t i=0;i<a[2];++i) Write8(a[0]+i,bytes[i]);
            return static_cast<int32_t>(a[0]);
        }
        return DiskServices::Call(fn,args);
    }
    void Bind(D3D11Renderer& target,PrStage2VramAtlas::Projection& atlas,Ot::DrawState draw,Ot::Viewport port) {
        Check(!pending,"unfinished transfer before fixture bind");
        renderer=&target; projection=&atlas; state=draw; view=port;
        starts=completions=dmaCallback=0u; consumedHeads.clear(); interruptMask=8u;
        pumpOnStatus=false; pumping=false; infoCommands.clear(); environmentCommands.clear();
        io[0x1F8010A8u]=0u;
        Write32(0x8005D838u,0u); Write32(0x8005D83Cu,0u); Write32(0x8005D740u,0u);
        Write8(0x8005D735u,1u);
    }
    Ot::RenderResult PumpOne() {
        Check(pending && renderer && projection,"no actual DMA to consume");
        Check(!pumping,"recursive fixture DMA consumption"); pumping=true;
        const uint32_t head=io.at(0x1F8010A0u);
        const auto batch=Ot::DecodeLinkedList(*this,head,state);
        const auto result=Ot::Render(batch,*projection,*renderer,view);
        state=batch.finalState;
        consumedHeads.push_back(head); ++completions;
        pending=false; io[0x1F8010A8u]&=~0x01000000u;
        if (dmaCallback) {
            Check(dmaCallback==0x80046BC4u && interruptMask!=0u,"unexpected fixture callback");
            PrStage2GpuDirect::DrainQueue80046BC4(*this);
        }
        pumping=false; return result;
    }
};
struct Window {
    HWND hwnd=CreateWindowExW(0,L"STATIC",L"S2 OT isolated GPU test",WS_OVERLAPPEDWINDOW,
                              0,0,64,64,nullptr,nullptr,GetModuleHandleW(nullptr),nullptr);
    ~Window() { if (hwnd) DestroyWindow(hwnd); }
};
std::vector<uint32_t> ReadFrame(D3D11Renderer& renderer) {
    ID3D11ShaderResourceView* raw=nullptr;
    Check(renderer.CaptureFrameTexture(raw),"capture failed");
    ComPtr<ID3D11ShaderResourceView> view; view.Attach(raw);
    ComPtr<ID3D11Resource> resource; view->GetResource(&resource);
    ComPtr<ID3D11Texture2D> texture; Check(SUCCEEDED(resource.As(&texture)),"capture is not texture");
    D3D11_TEXTURE2D_DESC desc; texture->GetDesc(&desc);
    ComPtr<ID3D11Device> device; texture->GetDevice(&device);
    ComPtr<ID3D11DeviceContext> context; device->GetImmediateContext(&context);
    desc.Usage=D3D11_USAGE_STAGING; desc.CPUAccessFlags=D3D11_CPU_ACCESS_READ;
    desc.BindFlags=desc.MiscFlags=0;
    ComPtr<ID3D11Texture2D> staging;
    Check(SUCCEEDED(device->CreateTexture2D(&desc,nullptr,&staging)),"staging failed");
    context->CopyResource(staging.Get(),texture.Get());
    D3D11_MAPPED_SUBRESOURCE mapped;
    Check(SUCCEEDED(context->Map(staging.Get(),0,D3D11_MAP_READ,0,&mapped)),"map failed");
    std::vector<uint32_t> pixels(size_t(desc.Width)*desc.Height);
    for (uint32_t y=0; y<desc.Height; ++y)
        std::memcpy(pixels.data()+size_t(y)*desc.Width,static_cast<const uint8_t*>(mapped.pData)+y*mapped.RowPitch,desc.Width*4u);
    context->Unmap(staging.Get(),0); return pixels;
}
uint64_t Hash(const std::vector<uint32_t>& words) {
    uint64_t h=14695981039346656037ull;
    for (uint32_t w:words) for (uint32_t shift=0; shift<32u; shift+=8u) h=(h^uint8_t(w>>shift))*1099511628211ull;
    return h;
}
uint32_t Xy(int x,int y) { return uint16_t(x)|(uint32_t(uint16_t(y))<<16u); }
constexpr uint32_t scratch=0x801FC000u;
std::vector<uint32_t> Quad(uint8_t opcode,uint16_t page=0,uint16_t clut=0,uint32_t color=0x808080u) {
    if (opcode&4u) return {(uint32_t(opcode)<<24u)|color,Xy(0,0),uint32_t(clut)<<16u,
        Xy(16,0),3u|(uint32_t(page)<<16u),Xy(0,16),0u,Xy(16,16),3u};
    return {(uint32_t(opcode)<<24u)|color,Xy(0,0),Xy(16,0),Xy(0,16),Xy(16,16)};
}
uint32_t Chain(DiskServices& s,const std::vector<std::vector<uint32_t>>& nodes,uint32_t start=scratch) {
    uint32_t address=start;
    for (size_t i=0; i<nodes.size(); ++i) {
        Check(nodes[i].size()<=255u,"fixture packet too long");
        const uint32_t next=address+4u+4u*static_cast<uint32_t>(nodes[i].size());
        s.Write32(address,(uint32_t(nodes[i].size())<<24u)|(i+1<nodes.size()?next&0xFFFFFFu:0xFFFFFFu));
        for (size_t j=0;j<nodes[i].size();++j) s.Write32(address+4u+4u*static_cast<uint32_t>(j),nodes[i][j]);
        address=next;
    }
    return start;
}
void Near(uint32_t actual,int r,int g,int b,const char* label) {
    const int expected[3]={r,g,b};
    for (int channel=0;channel<3;++channel)
        if (std::abs(int((actual>>(8*channel))&255u)-expected[channel])>1)
            throw std::runtime_error(std::string(label)+" actual="+std::to_string(actual)+" channel="+std::to_string(channel));
}
void Synthetic(QueueGpuFixture& s,int scale) {
    Window window; Check(window.hwnd!=nullptr,"window missing");
    D3D11Renderer renderer; Check(renderer.Initialize(window.hwnd,64*scale,64*scale),"renderer missing");
    PrStage2VramDevice::Device vram; PrStage2VramAtlas::Projection atlas(vram);
    for (uint32_t i=0;i<16u;++i) s.Write16(scratch+2u*i,0u);
    s.Write16(scratch+2u,31u); s.Write16(scratch+4u,0x83E0u); s.Write16(scratch+6u,0x8000u);
    vram.UploadImageWords(s,{0,480,16,1},scratch);
    s.Write32(scratch,0x3210u); vram.UploadImageWords(s,{0,0,1,1},scratch);
    const uint16_t clut=480u<<6u;
    const Ot::DrawState state{0,0,0,0,0,63,63};
    const Ot::Viewport view{0,0,64,64,0,0,float(64*scale),float(64*scale)};
    auto sample=[&](const std::vector<uint32_t>& pixels,int x,int y) { return pixels[(y*scale+scale/2)*64*scale+x*scale+scale/2]; };
    auto draw=[&](const std::vector<std::vector<uint32_t>>& nodes,Ot::DrawState env) {
        renderer.BeginFrame(40/255.0f,80/255.0f,120/255.0f);
        auto batch=Ot::DecodeLinkedList(s,Chain(s,nodes),env);
        auto result=Ot::Render(batch,atlas,renderer,view);
        Check(result.commands!=0,"no draws"); return ReadFrame(renderer);
    };
    for (uint32_t abr=0;abr<4u;++abr) {
        auto pixels=draw({Quad(0x2Eu,uint16_t(abr<<5u),clut)},state);
        Near(sample(pixels,1,4),40,80,120,"transparent key");
        Near(sample(pixels,5,4),255,0,0,"non-STP must stay opaque");
        const int green[4][3]={{20,168,60},{40,255,120},{40,0,120},{40,144,120}};
        Near(sample(pixels,9,4),green[abr][0],green[abr][1],green[abr][2],"STP blend");
        Near(sample(pixels,13,4),abr==0?20:40,abr==0?40:80,abr==0?60:120,"STP black");
        std::cout<<"synthetic-abr "<<scale<<' '<<abr<<' '<<Hash(pixels)<<'\n';
    }
    auto normal=draw({Quad(0x2Cu,0,clut)},state);
    Near(sample(normal,9,4),0,255,0,"non-semi STP is opaque");
    Near(sample(normal,13,4),0,0,0,"opaque STP black");
    auto raw=draw({Quad(0x2Du,0,clut,0x010101u)},state);
    Near(sample(raw,5,4),255,0,0,"raw texture must ignore modulation color");
    auto tint=draw({Quad(0x2Cu,0,clut,0x404040u)},state);
    Near(sample(tint,5,4),128,0,0,"texture modulation");
    auto order=draw({Quad(0x28u,0,0,0x0000FFu),Quad(0x28u,0,0,0xFF0000u)},state);
    Near(sample(order,8,8),0,0,255,"DMA order changed");
    std::vector<uint32_t> environment={0xE5000000u|4u|(4u<<11u),0xE3000000u|8u|(8u<<10u),0xE4000000u|11u|(11u<<10u)};
    auto clipped=draw({environment,Quad(0x28u,0,0,0x3264C8u)},state);
    Near(sample(clipped,8,8),200,100,50,"draw offset/area");
    Near(sample(clipped,7,8),40,80,120,"left clip"); Near(sample(clipped,12,8),40,80,120,"right clip");
    auto q=Quad(0x2Cu,0,clut);
    auto split=draw({std::vector<uint32_t>(q.begin(),q.begin()+2),{},std::vector<uint32_t>(q.begin()+2,q.end())},state);
    Check(split==normal,"GP0 command split across DMA tags changed pixels");
    // All eight GP0 polygon layouts, including the two not currently emitted
    // by S2's six unlit TMD producers. Constant UV/tint makes the expected
    // interior pixel independent of the rasterizer's edge ownership.
    for (uint32_t opcode:{0x20u,0x24u,0x28u,0x2Cu,0x30u,0x34u,0x38u,0x3Cu}) {
        const uint32_t count=(opcode&8u)?4u:3u;
        const bool textured=(opcode&4u)!=0, gouraud=(opcode&16u)!=0;
        const uint32_t color=textured?0x808080u:0x3264C8u;
        std::vector<uint32_t> words{(opcode<<24u)|color};
        for (uint32_t i=0;i<count;++i) {
            if (i && gouraud) words.push_back(color);
            words.push_back(Xy((i&1u)?16:0,(i&2u)?16:0));
            if (textured) words.push_back(1u|(i==0u?uint32_t(clut)<<16u:0u));
        }
        auto pixels=draw({words},state);
        Near(sample(pixels,3,3),textured?255:200,textured?0:100,textured?0:50,"polygon layout");
    }
    // E1 page mode survives into subsequent untextured semi-transparency.
    auto additive=draw({{0xE1000020u},Quad(0x2Au,0,0,0x000020u)},state);
    Near(sample(additive,8,8),72,80,120,"E1 draw mode");
    // Signed 11-bit E5 coordinates, and GP0 fill ignoring the drawing area.
    auto negative=draw({{0xE5000000u|2044u|(2044u<<11u)},Quad(0x28u,0,0,0x3264C8u)},state);
    Near(sample(negative,10,10),200,100,50,"negative draw offset");
    Near(sample(negative,12,10),40,80,120,"negative offset edge");
    auto fill=draw({environment,{0x023264C8u,Xy(0,0),Xy(16,16)}},state);
    Near(sample(fill,1,1),200,100,50,"fill ignores draw area/offset");
    Near(sample(fill,16,1),40,80,120,"fill extent");
    std::cout<<"synthetic-layouts "<<scale<<" 8\n";
    renderer.BeginFrame(40/255.0f,80/255.0f,120/255.0f);
    const auto beforeQueue=ReadFrame(renderer);
    s.Bind(renderer,atlas,state,view);
    const uint32_t red=Chain(s,{Quad(0x28u,0,0,0x0000FFu)},scratch);
    const uint32_t blue=Chain(s,{Quad(0x28u,0,0,0xFF0000u)},scratch+0x100u);
    Check(PrStage2GpuDirect::DrawOrderingTable800450A0(s,red)==0,"first DMA start");
    Check(PrStage2GpuDirect::DrawOrderingTable800450A0(s,blue)==1,"second DMA enqueue");
    Check(s.starts==1u && s.completions==0u && beforeQueue==ReadFrame(renderer),"queued work drew before consumption");
    s.PumpOne();
    Near(sample(ReadFrame(renderer),8,8),255,0,0,"first consumed OT");
    Check(s.starts==2u && s.completions==1u && s.pending,"native drain did not start next DMA");
    s.PumpOne();
    Near(sample(ReadFrame(renderer),8,8),0,0,255,"second consumed OT");
    Check(!s.pending && s.completions==2u && s.consumedHeads==std::vector<uint32_t>{red,blue} &&
          s.Read32(0x8005D838u)==s.Read32(0x8005D83Cu),"queue did not finish in native order");
    std::cout<<"synthetic-native-queue "<<scale<<" 2 2\n";
    // Default renderer path still ignores intermediate alpha as before.
    renderer.BeginFrame(40/255.0f,80/255.0f,120/255.0f);
    auto* srv=atlas.Resolve(0,clut,{0,0,4,1},renderer,PrStage2VramAtlas::Encoding::Abr0Stp);
    D3D11Renderer::TexturedTriCmd legacy; legacy.texture=srv; legacy.vertexCount=3;
    legacy.vertices[0]={0,0,2.5f/256,0.5f/256,1,1,1,1};
    legacy.vertices[1]={float(32*scale),0,2.5f/256,0.5f/256,1,1,1,1};
    legacy.vertices[2]={0,float(32*scale),2.5f/256,0.5f/256,1,1,1,1};
    renderer.SubmitTexturedTriangles(legacy); renderer.FlushSprites();
    Near(sample(ReadFrame(renderer),4,4),0,255,0,"legacy alpha behavior changed");
    uint32_t failures=0;
    auto reject=[&](auto operation) { bool failed=false; try { operation(); } catch (const std::runtime_error&) { failed=true; } Check(failed,"invalid stream accepted"); ++failures; };
    s.Write32(scratch,scratch&0xFFFFFFu); reject([&]{Ot::DecodeLinkedList(s,scratch,state);});
    reject([&]{Ot::DecodeLinkedList(s,scratch+1u,state);});
    reject([&]{Ot::DecodeLinkedList(s,0x80200000u,state);});
    for (uint32_t bad:{0xFF000000u,0xE2000001u,0xE6000001u,0x2C000000u,0xE1000800u})
        reject([&]{Ot::DecodeLinkedList(s,Chain(s,{{bad}}),state);});
    reject([&]{Ot::DecodeLinkedList(s,Chain(s,{{0x02000000u,Xy(1,0),Xy(16,16)}}),state);});
    s.Write32(0x801FFFFCu,0x01FFFFFFu);
    reject([&]{Ot::DecodeLinkedList(s,0x801FFFFCu,state);});
    renderer.BeginFrame(40/255.0f,80/255.0f,120/255.0f);
    const auto before=ReadFrame(renderer);
    auto missing=Ot::DecodeLinkedList(s,Chain(s,{Quad(0x28u,0,0,255u),Quad(0x2Cu,0,0xFFFFu)}),state);
    reject([&]{Ot::Render(missing,atlas,renderer,view);});
    Check(before==ReadFrame(renderer),"failed preparation partially drew the frame");
    std::cout<<"synthetic-pass "<<scale<<' '<<failures<<'\n';
}

void SyncAndClear(QueueGpuFixture& s,int scale) {
    Window window; Check(window.hwnd!=nullptr,"clear window missing");
    D3D11Renderer renderer; Check(renderer.Initialize(window.hwnd,64*scale,64*scale),"clear renderer missing");
    PrStage2VramAtlas::Projection atlas(s.vram);
    const Ot::DrawState before{0,-2,3,7,5,21,26};
    constexpr uint32_t rect=0x801FB000u;
    for (uint32_t unaligned=0;unaligned<2u;++unaligned) {
        renderer.BeginFrame(40/255.0f,80/255.0f,120/255.0f);
        const auto background=ReadFrame(renderer);
        s.Bind(renderer,atlas,before,{0,0,64,64,0,0,float(64*scale),float(64*scale)});
        const int x=unaligned?3:0, y=4, width=unaligned?13:64, height=unaligned?17:16;
        s.Write16(0x8005D738u,1024u); s.Write16(0x8005D73Au,512u);
        s.Write32(rect,Xy(x,y)); s.Write32(rect+4u,Xy(width,height));
        Check(PrStage2GpuDirect::ClearImage800460AC(s,rect,0xAA3264C8u)==0,"native clear return");
        Check(s.pending && s.starts==1u && s.completions==0u && ReadFrame(renderer)==background,"clear drew before DMA consumption");
        Check(s.infoCommands==(unaligned?std::vector<uint32_t>{3u,4u,5u}:std::vector<uint32_t>{}),"clear GPU state readback order");
        const auto decoded=Ot::DecodeLinkedList(s,0x8008EB58u,before);
        Check(decoded.commands.size()==1u && decoded.commands[0].opcode==(unaligned?0x60u:2u),"wrong native clear command");
        Check(PrStage2GpuDirect::DrawSync80044B3C(s,1u)==1 && ReadFrame(renderer)==background,"nonblocking sync fabricated completion");
        s.pumpOnStatus=true; // Device completion now becomes observable to native polling.
        Check(PrStage2GpuDirect::DrawSync80044B3C(s,0u)==0,"blocking original sync failed");
        Check(!s.pending && s.completions==1u && s.consumedHeads==std::vector<uint32_t>{0x8008EB58u},"sync did not consume actual clear");
        const auto& after=s.state;
        Check(after.tpage==before.tpage && after.offsetX==before.offsetX && after.offsetY==before.offsetY &&
              after.clipLeft==before.clipLeft && after.clipTop==before.clipTop && after.clipRight==before.clipRight &&
              after.clipBottom==before.clipBottom,"native clear did not restore draw environment");
        const auto pixels=ReadFrame(renderer);
        for (int row=0;row<64*scale;++row) for (int col=0;col<64*scale;++col) {
            const bool filled=col>=x*scale && col<(x+width)*scale && row>=y*scale && row<(y+height)*scale;
            Near(pixels[size_t(row)*64u*scale+col],filled?200:40,filled?100:80,filled?50:120,"native clear pixel/edge");
        }
        Check(PrStage2GpuDirect::DrawSync80044B3C(s,1u)==0,"completed sync still busy");
    }
    std::cout<<"synthetic-sync-clear "<<scale<<" 2 2 "<<8192*scale*scale<<'\n';
}

void EnvironmentAndBuffers(QueueGpuFixture& s,int scale) {
    Window window; Check(window.hwnd!=nullptr,"environment window missing");
    D3D11Renderer renderer;
    Check(renderer.Initialize(window.hwnd,128*scale,32*scale),"environment renderer missing");
    PrStage2VramAtlas::Projection atlas(s.vram);
    renderer.BeginFrame(40/255.0f,80/255.0f,120/255.0f);
    const auto initial=ReadFrame(renderer);
    s.Bind(renderer,atlas,{0,0,0,0,0,127,31},{0,0,128,32,0,0,float(128*scale),float(32*scale)});
    for (uint32_t p=0x80091730u;p<0x800917B0u;p+=4u) s.Write32(p,0u);
    s.Write8(0x8005D734u,0u); s.Write8(0x8005D736u,0u); s.Write8(0x8005D737u,0u);
    s.Write16(0x8005D738u,1024u); s.Write16(0x8005D73Au,512u);
    s.Write32(0x80057064u,0u); s.Write32(0x8009658Cu,0u); s.Write16(0x80096590u,0u);
    s.Write16(0x800965A0u,1u); s.Write32(0x800901C4u,0u);
    s.Write32(0x800928D0u,0u); s.Write32(0x800928D4u,Xy(64,32));
    s.Write32(0x8008ECA8u,Xy(0,64)); s.Write32(0x8008ECACu,0u);
    s.Write32(0x80091794u,Xy(64,32));
    PrStage2GpuDirect::SetDisplayMask80044AA0(s,0u);
    Check(!s.displayEnabled,"original display mask did not disable");
    for (uint32_t i=0;i<20u;++i) Check(s.Read8(0x8005D7A0u+i)==255u,"display cache not invalidated");
    constexpr uint32_t primitive=0x801FB100u;
    for (uint32_t frame=0;frame<2u;++frame) {
        const uint32_t shown=frame,drawn=frame^1u;
        s.pumpOnStatus=false;
        PrStage2GpuDirect::SwapBuffers80040370(s);
        Check(s.displayEnabled && s.displayX==shown*64u && s.displayY==0u,"original display buffer wrong");
        Check(PrStage2GpuDirect::CurrentDrawBuffer8004019C(s)==int32_t(drawn) &&
              s.Read32(0x8009658Cu)==frame+1u,"original draw buffer/serial wrong");
        s.pumpOnStatus=true;
        Check(PrStage2GpuDirect::DrawSync80044B3C(s,0u)==0,"environment did not drain");
        Check(s.completions==frame*3u+2u && s.state.clipLeft==int16_t(drawn*64u) &&
              s.state.clipRight==int16_t(drawn*64u+63u) && s.state.clipTop==0 && s.state.clipBottom==31 &&
              s.state.offsetX==int16_t(drawn*64u) && s.state.offsetY==0,"native environment packet state wrong");
        if (frame==0u) Check(ReadFrame(renderer)==initial,"environment setup unexpectedly painted");
        s.Write32(primitive,0x03FFFFFFu);
        s.Write32(primitive+4u,frame==0u?0x600000C8u:0x6000B400u);
        s.Write32(primitive+8u,Xy(4,6)); s.Write32(primitive+12u,Xy(8,5));
        s.pumpOnStatus=false;
        PrStage2GpuDirect::DrawOrderingTable800450A0(s,primitive);
        s.pumpOnStatus=true;
        Check(PrStage2GpuDirect::DrawSync80044B3C(s,0u)==0,"buffer primitive not consumed");
        const auto pixels=ReadFrame(renderer);
        for (int row=0;row<32*scale;++row) for (int col=0;col<128*scale;++col) {
            const int x=col/scale,y=row/scale;
            const bool red=x>=68 && x<76 && y>=6 && y<11;
            const bool green=frame!=0u && x>=4 && x<12 && y>=6 && y<11;
            Near(pixels[size_t(row)*128u*scale+col],red?200:(green?0:40),green?180:(red?0:80),
                 red||green?0:120,"native double buffer pixel ownership");
        }
    }
    Check(s.completions==6u && !s.pending,"environment transfers unfinished");
    Check(s.Read16(0x800917AAu)==0u && s.Read16(0x800917ACu)==0u,"software-offset cache not cleared");
    // Separate original geometry-register path. It selects the opposite slot
    // and preserves its full signed additions before CTC2 truncates to 16 bits.
    s.Write16(0x800965A0u,0u); s.Write16(0x800901C4u,160u); s.Write16(0x800901C6u,120u);
    PrStage2GpuDirect::ApplyFrameOffset800401AC(s);
    PrStage2GpuDirect::SetProjection80040C74(s,884u);
    Check(s.gte.ofx==(224<<16) && s.gte.ofy==(120<<16) && s.gte.h==884u,"native geometry controls wrong");
    std::cout<<"synthetic-environment "<<scale<<" 2 6 "<<8192*scale*scale<<'\n';
}

void NativeModels(QueueGpuFixture& s,const std::vector<uint32_t>& sizes) {
    Window window; Check(window.hwnd!=nullptr,"model window missing");
    D3D11Renderer renderer; Check(renderer.Initialize(window.hwnd,640,480),"model renderer missing");
    PrStage2VramAtlas::Projection atlas(s.vram);
    constexpr uint32_t ot=0x8015F000u,slots=0x80160000u,packets=0x80170000u;
    using Handler=int32_t(*)(PrStage2LifecycleDirect::Services&,uint32_t,uint32_t,uint32_t,uint32_t,uint32_t,uint32_t);
    uint32_t models=0,allCommands=0,drawn=0,unknown=0;
    for (size_t slot=1;slot<=sizes.size();++slot) {
        const uint32_t start=s.Read32(0x80091858u+4u*uint32_t(slot));
        if (s.Read32(start)!=0x41u) continue;
        ++models; Check(s.Read32(start+4u)==0u,"model already mapped");
        s.Write32(ot,8u); s.Write32(ot+4u,slots); s.Write32(ot+8u,0u); s.Write32(ot+16u,slots+255u*4u);
        constexpr uint32_t commonTail=0x0005D7F0u;
        const uint32_t clearsBefore=s.otcTransfers;
        Check(uint32_t(PrStage2LifecycleDirect::ClearOrderingTable80040CC8(s,0u,0u,ot))==slots,"native clear wrapper result");
        Check(s.otcTransfers==clearsBefore+1u,"OTC not actually consumed");
        for (uint32_t i=0;i<256u;++i)
            Check(s.Read32(slots+4u*i)==(i?(slots+4u*(i-1u))&0xFFFFFFu:commonTail),"incorrect native OTC link");
        Check(s.Read32(0x8005D7F0u)==0x04FFFFFFu,"original common tail changed");
        for (uint32_t i=1;i<=4u;++i)
            Check(s.Read32(0x8005D7F0u+4u*i)==0u,"original common tail NOP changed");
        std::cout<<"native-otc "<<slot<<" 256\n";
        s.Write32(0x80095C48u,0u);
        s.gte={}; s.gte.matrix.words={4096u,0u,4096u,0u,4096u,0u,0u,2000u};
        s.gte.ofx=160<<16; s.gte.ofy=120<<16; s.gte.h=160; s.gte.zsf3=1365; s.gte.zsf4=1024;
        uint32_t packet=packets;
        const uint32_t objects=s.Read32(start+8u);
        Check(objects<1000u && objects*28u+12u<=sizes[slot-1u],"bad original TMD table");
        for (uint32_t obj=0;obj<objects;++obj) {
            const uint32_t record=start+12u+28u*obj;
            const uint32_t vertices=start+12u+s.Read32(record);
            uint32_t primitive=start+12u+s.Read32(record+16u), count=s.Read32(record+20u);
            for (uint32_t i=0;i<count;++i) {
                Handler handler=nullptr;
                switch (s.Read8(primitive+3u)&0xFDu) {
                case 0x21u: handler=PrStage2LifecycleDirect::DrawNf3_8003B88C; break;
                case 0x25u: handler=PrStage2LifecycleDirect::DrawTnf3_8003CFDC; break;
                case 0x29u: handler=PrStage2LifecycleDirect::DrawNf4_8003BD9C; break;
                case 0x2Du: handler=PrStage2LifecycleDirect::DrawTnf4_8003D58C; break;
                case 0x31u: handler=PrStage2LifecycleDirect::DrawNg3_8003C36C; break;
                case 0x35u: handler=PrStage2LifecycleDirect::DrawTng3_8003DC2C; break;
                default: throw std::runtime_error("unhandled original model primitive");
                }
                packet=uint32_t(handler(s,primitive,vertices,packet,1u,8u,ot));
                Check(packet<packets+0x10000u,"fixture packet area exceeded");
                primitive+=4u+4u*s.Read8(primitive+1u);
            }
        }
        const uint32_t head=s.Read32(ot+16u);
        auto batch=Ot::DecodeLinkedList(s,head,{0,0,0,0,0,319,239});
        allCommands+=uint32_t(batch.commands.size());
        // Independent Python decoder gets the native RAM words, not a
        // second list reconstructed from the host's decoded commands.
        std::cout<<"native-begin "<<slot<<' '<<head<<' '<<batch.commands.size()<<'\n';
        uint32_t link=head&0xFFFFFFu;
        std::set<uint32_t> seen;
        while (link!=0xFFFFFFu) {
            Check(seen.insert(link).second,"native producer cycle");
            const uint32_t tag=s.Read32(0x80000000u|link);
            std::cout<<"native-node "<<link<<' '<<tag;
            for (uint32_t i=0;i<(tag>>24u);++i) std::cout<<' '<<s.Read32(0x80000000u+link+4u+4u*i);
            std::cout<<'\n'; link=tag&0xFFFFFFu;
        }
        Check(seen.count(commonTail)==1u,"native OT skipped the shared tail packet");
        for (const auto& c:batch.commands) {
            std::cout<<"native-command "<<c.packet<<' '<<uint32_t(c.opcode)<<' '<<uint32_t(c.vertices)<<' '<<c.state.tpage<<' '<<c.clut;
            for (uint32_t i=0;i<c.vertices;++i) { const auto& v=c.vertex[i]; std::cout<<' '<<v.x<<' '<<v.y<<' '<<uint32_t(v.u)<<' '<<uint32_t(v.v)<<' '<<uint32_t(v.r)<<' '<<uint32_t(v.g)<<' '<<uint32_t(v.b); }
            std::cout<<'\n';
        }
        renderer.BeginFrame(152/255.0f,200/255.0f,248/255.0f);
        const auto background=ReadFrame(renderer);
        try {
            s.Bind(renderer,atlas,{0,0,0,0,0,319,239},{0,0,320,240,0,0,640,480});
            Check(PrStage2LifecycleDirect::SubmitOrderingTable80040CA4(s,ot)==0,"native submit failed");
            Check(s.pending && s.starts==1u && s.completions==0u && ReadFrame(renderer)==background,"DMA submission fabricated rendering");
            const auto result=s.PumpOne();
            Check(!s.pending && s.completions==1u && s.consumedHeads==std::vector<uint32_t>{head},"native OT not consumed");
            std::cout<<"native-queue "<<slot<<" 1 1\n";
            const auto frame=ReadFrame(renderer);
            uint32_t changed=0;
            for (size_t i=0;i<frame.size();++i) if ((frame[i]&0xFFFFFFu)!=(background[i]&0xFFFFFFu)) ++changed;
            std::cout<<"native-render "<<slot<<' '<<result.commands<<' '<<result.triangles<<' '<<result.gpuDraws<<' '<<Hash(frame)<<' '<<changed<<'\n';
            if (result.commands) ++drawn;
        } catch (const std::runtime_error& e) {
            Check(std::string(e.what()).find("unknown texture region")!=std::string::npos,"unexpected native renderer error");
            ++unknown; std::cout<<"native-unknown "<<slot<<' '<<e.what()<<'\n';
        }
    }
    Check(models==36u && allCommands>0u && drawn>0u,"no native rendering exercised");
    std::cout<<"native-summary "<<models<<' '<<allCommands<<' '<<drawn<<' '<<unknown<<'\n';
}
}

int main(int argc,char** argv) {
    try {
        if (argc!=3) return 1;
        uint32_t count; if (!(std::cin>>count) || count>1024u) return 1;
        std::vector<uint32_t> sizes(count); for (auto& size:sizes) if (!(std::cin>>size)) return 1;
        QueueGpuFixture s(argv[1],argv[2]);
        Check(PrStage2IntLoaderDirect::LoadInt8001AC18(s,0x80054A60u,0u)==1,"original INT load failed");
        Check(s.Read32(0x8006EB84u)==count,"resident count mismatch");
        for (uint32_t i=0;i<count;++i) {
            const uint32_t address=s.Read32(0x80091858u+4u*(i+1u));
            uint64_t hash=14695981039346656037ull;
            for (uint32_t j=0;j<sizes[i];++j) hash=(hash^s.Read8(address+j))*1099511628211ull;
            std::cout<<"resident "<<i+1u<<' '<<address<<' '<<sizes[i]<<' '<<hash<<'\n';
        }
        NativeModels(s,sizes);
        Synthetic(s,1); Synthetic(s,4);
        SyncAndClear(s,1); SyncAndClear(s,4);
        EnvironmentAndBuffers(s,1); EnvironmentAndBuffers(s,4);
        std::cout<<"ot-draw-pass\n"; return 0;
    } catch (const std::exception& e) { std::cerr<<e.what()<<'\n'; return 2; }
}
