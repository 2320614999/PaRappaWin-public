// Reuse the existing isolated D3D/DMA fixture, not its synthetic initialization.
#define main ExistingStage2OtDrawMain
#include "test_stage2_ot_draw.cpp"
#undef main
#include "pr/pr_stage2_graph_init_direct.h"

namespace {
struct GraphInitGpuFixture : QueueGpuFixture {
    using QueueGpuFixture::QueueGpuFixture;
    uint32_t callbackBoundaryCalls = 0, biosControlBoundaryCalls = 0, resets = 0;
    void Write32(uint32_t address, uint32_t value) override {
        if (address == 0x1F801814u && value == 0u) {
            Check(!pending && !otcPending, "reset crossed an unfinished test transfer");
            state = {}; displayEnabled = false; displayX = displayY = 0u; ++resets;
            return;
        }
        if (address == 0x1F801814u && value == 0x10000007u) {
            // Explicit version-3 device input; not a captured hardware status.
            gpuReadback = 2u; infoCommands.push_back(7u); return;
        }
        QueueGpuFixture::Write32(address, value);
    }
    int32_t Call(uint32_t fn, std::initializer_list<uint32_t> args) override {
        const std::vector<uint32_t> a(args);
        if (fn == 0x80047FFCu && a == std::vector<uint32_t>{0x8001243Cu,0x8005D6ECu,0x8005D734u})
            return 0; // Non-rendering diagnostic output boundary.
        if (fn == 0x80035744u && a.empty()) {
            // Named dependency receipt only. This fixture is NOT the original
            // interrupt controller, and does not assert it has been ported.
            ++callbackBoundaryCalls; return 0;
        }
        if (fn == 0x80048950u && a == std::vector<uint32_t>{0x0005D6ECu}) {
            ++biosControlBoundaryCalls; return 0;
        }
        return QueueGpuFixture::Call(fn,args);
    }
};

void OriginalLayout(GraphInitGpuFixture& s, int scale) {
    Window window; Check(window.hwnd != nullptr, "graph window missing");
    D3D11Renderer renderer;
    Check(renderer.Initialize(window.hwnd,320*scale,480*scale), "graph renderer missing");
    PrStage2VramAtlas::Projection atlas(s.vram);
    renderer.BeginFrame(40/255.0f,80/255.0f,120/255.0f);
    const auto initial = ReadFrame(renderer);
    s.Bind(renderer,atlas,{},{0,0,320,480,0,0,float(320*scale),float(480*scale)});
    s.pumpOnStatus = true;
    // Only platform video mode and selected initial buffer are explicit inputs.
    // Width/height, matrices, clip, offsets and two-slot layout come from the
    // original graph functions, in 1C470 order. GTE/CP0 init is not invoked.
    s.Write32(0x80057064u,0u); s.Write16(0x80096590u,0u);
    Check(PrStage2GraphInitDirect::InitGraphEnvironment8003FC14(s,320,240,4,0,0)
        == int32_t(0x80091790u), "original environment returned wrong pointer");
    PrStage2GraphInitDirect::InitGeometry8003FDE4(s,320,240);
    PrStage2GraphInitDirect::SetDoubleBufferOffsets80040AE4(s,0,0,0,240);
    PrStage2GraphInitDirect::SetScreenCenter80040B84(s);
    PrStage2GpuDirect::SetProjection80040C74(s,440);
    Check(PrStage2GpuDirect::DrawSync80044B3C(s,0)==0,"graph init queue not drained");
    Check(s.Read32(0x800917FCu)==320 && s.Read32(0x8009182Cu)==240 &&
          s.Read16(0x800928B0u)==4096 && s.Read16(0x800901C4u)==160 &&
          s.Read16(0x800901C6u)==120 && s.Read16(0x800965A0u)==4 && s.gte.h==440,
          "original geometry initialization wrong");
    Check(s.Read32(0x8008ECA8u)==0 && s.Read16(0x8008ECACu)==0 &&
          s.Read16(0x8008ECAEu)==240 && s.Read32(0x8009658Cu)==1,
          "original double-buffer layout wrong");
    Check(ReadFrame(renderer)==initial,"environment setup unexpectedly painted");
    const uint32_t setupTransfers=s.completions;
    Check(setupTransfers==4,"unexpected original setup environment packet count");
    constexpr uint32_t primitive=0x801FB100u;
    for (uint32_t frame=0;frame<2;++frame) {
        s.pumpOnStatus=false;
        PrStage2GpuDirect::SwapBuffers80040370(s);
        const uint32_t drawn=frame^1u;
        Check(s.displayEnabled && s.displayX==0 && s.displayY==frame*240u &&
              s.Read16(0x80096590u)==drawn,"original displayed/drawn buffer mismatch");
        s.pumpOnStatus=true;
        Check(PrStage2GpuDirect::DrawSync80044B3C(s,0)==0,"swap queue unfinished");
        Check(s.state.clipLeft==0 && s.state.clipRight==319 &&
              s.state.clipTop==drawn*240 && s.state.clipBottom==drawn*240+239 &&
              s.state.offsetX==160 && s.state.offsetY==drawn*240+120,
              "original clip/center not consumed by D3D backend");
        s.Write32(primitive,0x03FFFFFFu);
        s.Write32(primitive+4,frame==0?0x600000C8u:0x6000B400u);
        s.Write32(primitive+8,Xy(-156,-114)); s.Write32(primitive+12,Xy(8,5));
        PrStage2GpuDirect::DrawOrderingTable800450A0(s,primitive);
        Check(PrStage2GpuDirect::DrawSync80044B3C(s,0)==0,"probe primitive unfinished");
        const auto pixels=ReadFrame(renderer);
        for(int row=0;row<480*scale;++row) for(int col=0;col<320*scale;++col) {
            const int x=col/scale,y=row/scale;
            const bool red=x>=4 && x<12 && y>=246 && y<251;
            const bool green=frame!=0 && x>=4 && x<12 && y>=6 && y<11;
            Near(pixels[size_t(row)*320u*scale+col],red?200:(green?0:40),
                 green?180:(red?0:80),red||green?0:120,"original-layout buffer pixels wrong");
        }
    }
    Check(s.completions==setupTransfers+6 && !s.pending,"native graph transfers incomplete");
    std::cout << "original-layout " << scale << " 4 2 6 " << 307200*scale*scale << '\n';
}
}

int main(int argc,char** argv) {
    try {
        if(argc!=3) return 1;
        for(int scale : {1,4}) {
            GraphInitGpuFixture device(argv[1],argv[2]);
            OriginalLayout(device,scale);
            Check(device.callbackBoundaryCalls==1 && device.biosControlBoundaryCalls==1 && device.resets==1,
                  "unexpected initialization boundary count");
        }
        std::cout << "graph-init-gpu-pass\n"; return 0;
    } catch(const std::exception& e) { std::cerr<<e.what()<<'\n'; return 2; }
}
