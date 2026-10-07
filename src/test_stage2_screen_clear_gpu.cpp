// External-only public ClearImage test; no native game or card backend.
#define main ExistingStage2OtDrawMain
#include "test_stage2_ot_draw.cpp"
#undef main

namespace {
struct ImageEntryGpuFixture : QueueGpuFixture {
    using QueueGpuFixture::QueueGpuFixture;
    std::vector<std::vector<uint32_t>> messages;
    int32_t Call(uint32_t fn,std::initializer_list<uint32_t> args) override {
        if (fn==0x800F30A0u) {
            std::vector<uint32_t> a(args);
            Check((a.size()==2u && (a[0]==0x8001252Cu || a[0]==0x8001250Cu)) ||
                  (a.size()==5u && a[0]==0x80012518u),"unexpected original advisory call");
            messages.push_back(a);
            return -73; // Must not become a gate on the original submission.
        }
        return QueueGpuFixture::Call(fn,args);
    }
};

void PublicImageCases(ImageEntryGpuFixture& s,int scale) {
    Window window; Check(window.hwnd!=nullptr,"image entry window missing");
    D3D11Renderer renderer;
    Check(renderer.Initialize(window.hwnd,64*scale,64*scale),"image entry renderer missing");
    PrStage2VramAtlas::Projection atlas(s.vram);
    const Ot::DrawState initialState{0,-2,3,7,5,21,26};
    constexpr uint32_t rect=0x801FB000u, noop=0x801FB100u;
    uint32_t transfers=0;
    for (uint32_t level:{0u,1u,2u}) for (uint32_t queued:{0u,1u}) for (uint32_t unaligned:{0u,1u}) {
        renderer.BeginFrame(40/255.0f,80/255.0f,120/255.0f);
        const auto background=ReadFrame(renderer);
        s.Bind(renderer,atlas,initialState,{0,0,64,64,0,0,float(64*scale),float(64*scale)});
        s.messages.clear();
        s.Write8(0x8005D736u,static_cast<uint8_t>(level));
        s.Write32(0x8005D730u,0x800F30A0u);
        s.Write16(0x8005D738u,1024u); s.Write16(0x8005D73Au,512u);
        const int x=unaligned?3:0,y=4,width=unaligned?13:64,height=unaligned?17:16;
        s.Write32(rect,Xy(x,y)); s.Write32(rect+4u,Xy(width,height));
        if (queued) {
            // An actual environment-only DMA holds the queue busy. No fake
            // ClearImage completion is returned to the translated entry.
            s.Write32(noop,0x01FFFFFFu); s.Write32(noop+4u,0xE1000000u);
            PrStage2GpuDirect::StartLinkedDma80046840(s,noop);
        }
        const int32_t returned=PrStage2GpuDirect::ClearImage80044CD0(s,rect,0x1234C8u,0xABCD64u,0xFFFF32u);
        Check(returned==(queued?1:0),"public clear queue return mismatch");
        Check(s.messages.size()==(level==2u?2u:0u),"public clear advisory branches changed");
        if (level==2u) {
            Check(s.messages[0]==std::vector<uint32_t>{0x8001252Cu,0x80012530u},"wrong original operation name");
            Check(s.messages[1]==std::vector<uint32_t>{0x80012518u,uint32_t(x),uint32_t(y),uint32_t(width),uint32_t(height)},
                  "wrong original rectangle fields");
        }
        Check(s.pending && s.starts==1u && s.completions==0u && ReadFrame(renderer)==background,
              "public entry fabricated synchronous drawing");
        if (queued) {
            Check(s.Read32(0x80094448u)==0x800460ACu && s.Read32(0x8009444Cu)==0x80094454u &&
                  s.Read32(0x80094450u)==0x3264C8u,"queued command identity/color mismatch");
            Check(s.Read32(0x80094454u)==Xy(x,y) && s.Read32(0x80094458u)==Xy(width,height),
                  "original eight-byte rectangle copy missing");
            s.Write32(rect,Xy(31,31)); s.Write32(rect+4u,Xy(1,1));
            s.PumpOne();
            Check(s.pending && s.starts==2u && s.completions==1u && ReadFrame(renderer)==background,
                  "queue did not start copied clear after first DMA");
        }
        Check(s.infoCommands==(unaligned?std::vector<uint32_t>{3u,4u,5u}:std::vector<uint32_t>{}),
              "clear environment readbacks changed");
        s.PumpOne();
        Check(!s.pending && s.completions==1u+queued && s.Read32(0x8005D838u)==s.Read32(0x8005D83Cu),
              "public clear did not finish through actual queue consumption");
        Check(s.state.offsetX==initialState.offsetX && s.state.offsetY==initialState.offsetY &&
              s.state.clipLeft==initialState.clipLeft && s.state.clipTop==initialState.clipTop &&
              s.state.clipRight==initialState.clipRight && s.state.clipBottom==initialState.clipBottom,
              "public clear lost previous draw environment");
        const auto pixels=ReadFrame(renderer);
        for (int row=0;row<64*scale;++row) for (int col=0;col<64*scale;++col) {
            const bool filled=col>=x*scale && col<(x+width)*scale && row>=y*scale && row<(y+height)*scale;
            Near(pixels[size_t(row)*64u*scale+col],filled?200:40,filled?100:80,filled?50:120,
                 "public clear pixel/edge/copy mismatch");
        }
        transfers+=s.completions;
    }
    Check(transfers==18u,"public image GPU transfer coverage changed");
    std::cout<<"public-clear "<<scale<<" 12 18 "<<49152*scale*scale<<'\n';
}

void PublicFramebufferRect(ImageEntryGpuFixture& s,int scale) {
    Window window; Check(window.hwnd!=nullptr,"framebuffer window missing");
    D3D11Renderer renderer;
    Check(renderer.Initialize(window.hwnd,320*scale,480*scale),"framebuffer renderer missing");
    PrStage2VramAtlas::Projection atlas(s.vram);
    renderer.BeginFrame(1.0f,1.0f,1.0f);
    const auto background=ReadFrame(renderer);
    s.Bind(renderer,atlas,{0,17,19,4,5,31,43},{0,0,320,480,0,0,float(320*scale),float(480*scale)});
    s.Write8(0x8005D736u,0u); s.Write16(0x8005D738u,1024u); s.Write16(0x8005D73Au,512u);
    constexpr uint32_t rect=0x801FB000u;
    s.Write32(rect,0u); s.Write32(rect+4u,Xy(320,480));
    // Same rectangle dimensions, but explicitly public RAM: this does NOT
    // exercise or claim to implement 8001B1B0's private stack transport.
    Check(PrStage2GpuDirect::ClearImage80044CD0(s,rect,0u,0u,0u)==0,"framebuffer clear return");
    Check(s.pending && ReadFrame(renderer)==background,"framebuffer clear was not asynchronous");
    s.PumpOne();
    const auto pixels=ReadFrame(renderer);
    for (const auto pixel:pixels) Near(pixel,0,0,0,"framebuffer clear missed a pixel");
    Check(pixels.size()==size_t(153600)*scale*scale,"framebuffer pixel coverage changed");
    std::cout<<"public-framebuffer "<<scale<<" 1 "<<pixels.size()<<'\n';
}
}

int main(int argc,char** argv) {
    try {
        if(argc!=3) return 1;
        for (int scale:{1,4}) {
            ImageEntryGpuFixture device(argv[1],argv[2]);
            PublicImageCases(device,scale);
            PublicFramebufferRect(device,scale);
        }
        std::cout<<"public-clear-gpu-pass\n";
        return 0;
    } catch(const std::exception& error) {
        std::cerr<<error.what()<<'\n'; return 2;
    }
}