// External-only GPU validation. No game process or memory-card backend.
#define main ExistingStage2OtDrawMain
#include "test_stage2_ot_draw.cpp"
#undef main
#include "pr/pr_stage2_graph_bootstrap_direct.h"

namespace {
struct BootstrapGpuFixture : QueueGpuFixture {
    using QueueGpuFixture::QueueGpuFixture;
    std::vector<uint32_t> boundaries;
    uint32_t resets = 0;

    void Write32(uint32_t address, uint32_t value) override {
        if (address == 0x1F801814u && value == 0u) {
            Check(!pending && !otcPending, "bootstrap reset crossed an unfinished test transfer");
            state = {}; displayEnabled = false; displayX = displayY = 0u;
            ++resets;
            return;
        }
        if (address == 0x1F801814u && value == 0x10000007u) {
            gpuReadback = 2u; infoCommands.push_back(7u); return;
        }
        QueueGpuFixture::Write32(address, value);
    }

    int32_t Call(uint32_t fn, std::initializer_list<uint32_t> args) override {
        const std::vector<uint32_t> a(args);
        if (fn == 0x80047FFCu && a == std::vector<uint32_t>{0x8001243Cu, 0x8005D6ECu, 0x8005D734u})
            return 0; // Diagnostic output only.
        if (fn == 0x80035744u && a.empty()) {
            boundaries.push_back(fn); return 0;
        }
        if (fn == 0x80048950u && a == std::vector<uint32_t>{0x0005D6ECu}) {
            boundaries.push_back(fn); return 0;
        }
        if (fn == 0x800489F0u && a == std::vector<uint32_t>{0x20000001u, 0x800882F0u}) {
            Check(Read32(0x800882F0u) == 0xFFFFFFFFu && Read32(0x80091830u) == 0u,
                  "native PAD input initialization missing");
            boundaries.push_back(fn); return 0;
        }
        if (fn == 0x80048AE0u && a == std::vector<uint32_t>{0u}) {
            boundaries.push_back(fn); return -77;
        }
        if (fn == 0x80040D20u && a.empty()) {
            // Explicit, isolated GTE/BIOS receipt. This does not initialize a
            // production CP0/IRQ service or prove a native game has started.
            boundaries.push_back(fn); return 0;
        }
        if (fn == 0x8001B1B0u && a == std::vector<uint32_t>{0u, 0u, 0u}) {
            boundaries.push_back(fn); return -19;
        }
        return QueueGpuFixture::Call(fn, args);
    }
};

void VerifyBootstrap(BootstrapGpuFixture& s, int scale) {
    Window window;
    Check(window.hwnd != nullptr, "bootstrap window missing");
    D3D11Renderer renderer;
    Check(renderer.Initialize(window.hwnd, 320*scale, 480*scale), "bootstrap renderer missing");
    PrStage2VramAtlas::Projection atlas(s.vram);
    renderer.BeginFrame(40/255.0f, 80/255.0f, 120/255.0f);
    const auto initial = ReadFrame(renderer);
    s.Bind(renderer, atlas, {}, {0, 0, 320, 480, 0, 0, float(320*scale), float(480*scale)});
    s.pumpOnStatus = true;
    s.Write32(0x80057064u, 0u);
    Check(PrStage2GraphBootstrapDirect::InitGraphics8001C470(s) == -19,
          "bootstrap lost the final callee return value");
    const std::vector<uint32_t> expectedBoundaries{
        0x80035744u, 0x80048950u, 0x80035744u, 0x800489F0u, 0x80048AE0u,
        0x80035744u, 0x80048950u, 0x80040D20u, 0x8001B1B0u};
    Check(s.boundaries == expectedBoundaries && s.resets == 2u,
          "bootstrap callback/PAD/GTE order or duplicate reset changed");
    Check(s.Read32(0x8008EDE0u) == 0x8003B9C8u && s.Read32(0x8008EED0u) == 0x8003E26Cu &&
          s.Read32(0x8008EDD8u) == 0u && s.Read32(0x8008EED4u) == 0u,
          "native primitive dispatch initialization missing");
    Check(PrStage2GpuDirect::DrawSync80044B3C(s, 0) == 0, "bootstrap setup queue incomplete");
    Check(s.Read32(0x800917FCu) == 320u && s.Read32(0x8009182Cu) == 240u &&
          s.Read16(0x800928B0u) == 4096u && s.Read16(0x800901C4u) == 160u &&
          s.Read16(0x800901C6u) == 120u && s.Read16(0x800965A0u) == 4u && s.gte.h == 440u,
          "bootstrap geometry/center/projection mismatch");
    Check(s.Read32(0x8008ECA8u) == 0u && s.Read16(0x8008ECACu) == 0u &&
          s.Read16(0x8008ECAEu) == 240u && s.Read16(0x80096590u) == 0u,
          "bootstrap buffer origins mismatch");
    Check(ReadFrame(renderer) == initial, "bootstrap setup unexpectedly painted");
    const uint32_t setupTransfers = s.completions;
    Check(setupTransfers == 6u, "bootstrap original setup environment packet count changed");
    constexpr uint32_t packet = 0x801FB100u;
    for (uint32_t frame = 0; frame < 2; ++frame) {
        s.pumpOnStatus = false;
        PrStage2GpuDirect::SwapBuffers80040370(s);
        const uint32_t draw = frame ^ 1u;
        Check(s.displayEnabled && s.displayX == 0u && s.displayY == frame*240u &&
              s.Read16(0x80096590u) == draw, "bootstrap swap display/draw mismatch");
        s.pumpOnStatus = true;
        Check(PrStage2GpuDirect::DrawSync80044B3C(s, 0) == 0, "bootstrap swap queue incomplete");
        Check(s.state.clipLeft == 0 && s.state.clipRight == 319 &&
              s.state.clipTop == draw*240 && s.state.clipBottom == draw*240 + 239 &&
              s.state.offsetX == 160 && s.state.offsetY == draw*240 + 120,
              "bootstrap environment not consumed by renderer");
        s.Write32(packet, 0x03FFFFFFu);
        s.Write32(packet + 4u, frame == 0u ? 0x600000C8u : 0x6000B400u);
        s.Write32(packet + 8u, Xy(-156, -114));
        s.Write32(packet + 12u, Xy(8, 5));
        PrStage2GpuDirect::DrawOrderingTable800450A0(s, packet);
        Check(PrStage2GpuDirect::DrawSync80044B3C(s, 0) == 0, "bootstrap primitive queue incomplete");
        const auto pixels = ReadFrame(renderer);
        for (int row = 0; row < 480*scale; ++row) for (int col = 0; col < 320*scale; ++col) {
            const int x = col/scale, y = row/scale;
            const bool red = x >= 4 && x < 12 && y >= 246 && y < 251;
            const bool green = frame != 0u && x >= 4 && x < 12 && y >= 6 && y < 11;
            Near(pixels[size_t(row)*320u*scale + col], red ? 200 : (green ? 0 : 40),
                 green ? 180 : (red ? 0 : 80), red || green ? 0 : 120,
                 "bootstrap double-buffer pixel mismatch");
        }
    }
    Check(s.completions == setupTransfers + 6u && !s.pending, "bootstrap transfers incomplete");
    std::cout << "bootstrap-layout " << scale << " 6 2 6 " << 307200*scale*scale << '\n';
}
}

#ifndef PR_STAGE2_BOOTSTRAP_GPU_FIXTURE_ONLY
int main(int argc, char** argv) {
    try {
        if (argc != 3) return 1;
        for (int scale : {1, 4}) {
            BootstrapGpuFixture device(argv[1], argv[2]);
            VerifyBootstrap(device, scale);
        }
        std::cout << "graph-bootstrap-gpu-pass\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 2;
    }
}
#endif