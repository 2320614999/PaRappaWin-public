#ifdef NDEBUG
#undef NDEBUG
#endif
#include "pr/pr_stage1_terminal_presentation_direct.h"
#include "d3d11_renderer.h"
#include <cassert>
#include <cstdio>

static void CheckTexture(ID3D11ShaderResourceView* view, uint32_t color) {
    ComPtr<ID3D11Resource> resource;
    view->GetResource(&resource);
    ComPtr<ID3D11Texture2D> texture;
    assert(SUCCEEDED(resource.As(&texture)));
    D3D11_TEXTURE2D_DESC desc{};
    texture->GetDesc(&desc);
    assert(desc.Width == 800 && desc.Height == 600);
    assert(desc.Format == DXGI_FORMAT_R8G8B8A8_UNORM);
    ComPtr<ID3D11Device> device;
    texture->GetDevice(&device);
    ComPtr<ID3D11DeviceContext> context;
    device->GetImmediateContext(&context);
    desc.Usage = D3D11_USAGE_STAGING;
    desc.BindFlags = 0;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    desc.MiscFlags = 0;
    ComPtr<ID3D11Texture2D> readback;
    assert(SUCCEEDED(device->CreateTexture2D(&desc, nullptr, &readback)));
    context->CopyResource(readback.Get(), texture.Get());
    D3D11_MAPPED_SUBRESOURCE mapped{};
    assert(SUCCEEDED(context->Map(readback.Get(), 0, D3D11_MAP_READ, 0, &mapped)));
    for (uint32_t y = 0; y < desc.Height; ++y) {
        const auto* row = reinterpret_cast<const uint32_t*>(
            static_cast<const uint8_t*>(mapped.pData) + y * mapped.RowPitch);
        for (uint32_t x = 0; x < desc.Width; ++x) assert(row[x] == color);
    }
    context->Unmap(readback.Get(), 0);
}

int main() {
    using namespace PrStage1TerminalPresentationDirect;
    for (uint32_t initial = 0; initial < 2; ++initial) {
        State state{};
        PrPsxVSyncDirect::PsxVSyncState80035560 clock{};
        uint64_t hostVblank = 100;
        uint64_t presents = 10;
        uint32_t active = initial;
        for (int i = 0; i < 4; ++i) {
            assert(!CanMove(state, active));
            assert(!Submit(state, static_cast<uint8_t>(active), hostVblank, true, true, clock));
            assert(!Submit(state, static_cast<uint8_t>(active), hostVblank, false, false, clock));
            assert(!Submit(state, 2, hostVblank, true, false, clock));
            assert(Submit(state, static_cast<uint8_t>(active), hostVblank, true, false, clock));
            active ^= 1;
            for (int j = 0; j < 20; ++j) {
                assert(!Poll(state, presents)); // failed/not-yet-presented frame
                assert(!CanMove(state, active));
                assert(!AdvanceWait(state, clock, hostVblank));
                assert(!Publish(state, presents));
            }
            assert(!Submit(state, static_cast<uint8_t>(active), hostVblank, true, false, clock));
            // Old-front redraws (even successful ones) cannot acknowledge the
            // pending back page. No future tick is fabricated on duplicate calls.
            ++presents;
            assert(!Poll(state, presents));
            assert(!AdvanceWait(state, clock, ++hostVblank));
            assert(!AdvanceWait(state, clock, hostVblank));
            assert(!Publish(state, presents));
            assert(!Poll(state, ++presents));
            assert(AdvanceWait(state, clock, ++hostVblank));
            assert(clock.consumedHostVblankCount == 2);
            assert(!Poll(state, presents)); // eligible is not presented
            assert(Publish(state, presents));
            assert(!Publish(state, presents));
            assert(!Poll(state, presents));
            presents += 2; // extra 60Hz redraws do not count as cleanup frames
            assert(Poll(state, presents));
            assert(!Poll(state, presents));
            assert(state.frames == i + 1);
        }
        assert(CanMove(state, active));
        assert(!CanMove(state, 2));
        assert(SourcePage(active) == (initial ^ 1));
        assert(!Submit(state, 0, hostVblank, true, false, clock));
    }

    {
        State state{};
        PrPsxVSyncDirect::PsxVSyncState80035560 clock{};
        assert(Submit(state, 0, 100, true, false, clock));
        assert(!AdvanceWait(state, clock, 99)); // no unsigned clock rewind
        assert(!PrPsxVSyncDirect::ConsumeHostVblanks80035560(clock, 0).softwareWaitComplete);
        assert(!PrPsxVSyncDirect::ConsumeHostVblanks80035560(clock, -3).softwareWaitComplete);
        assert(clock.vblankCounter80057034 == 0);
        assert(AdvanceWait(state, clock, 102)); // 30Hz / two VBlanks in one delivery
        assert(clock.vblankCounter80057034 == 2);
        assert(Publish(state, 5));
        assert(Poll(state, 6));
        assert(Submit(state, 1, 103, true, false, clock));
        PrPsxVSyncDirect::BeginVSync80035560(clock, 4);
        assert(!AdvanceWait(state, clock, 107)); // cannot steal another wait
        assert(clock.vblankCounter80057034 == 2);
    }
    {
        State state{};
        PrPsxVSyncDirect::PsxVSyncState80035560 clock{};
        assert(Submit(state, 0, 100, true, false, clock));
        assert(AdvanceWait(state, clock, 102));
        assert(Publish(state, 1));
        assert(Poll(state, 2));
        // One host tick passes between completed Present and the next render.
        // Native VSync(2) is based on the previous wait's lastVblank, so the
        // next frame needs only ONE more tick, not two freshly invented ticks.
        assert(!AdvanceWait(state, clock, 103));
        assert(clock.vblankCounter80057034 == 3);
        assert(clock.lastVblank80055F74 == 2);
        assert(Submit(state, 1, 103, true, false, clock));
        assert(clock.pendingVblanks == 1);
        assert(AdvanceWait(state, clock, 104));
        assert(clock.consumedHostVblankCount == 1);
        assert(Publish(state, 2));
        assert(Poll(state, 3));
        assert(Submit(state, 0, 104, true, false, clock));
        assert(AdvanceWait(state, clock, 109)); // delivered overrun is not lost
        assert(clock.lastVblank80055F74 == 6);
        assert(clock.vblankCounter80057034 == 9);
        assert(clock.consumedHostVblankCount == 2);
    }

    const auto instance = GetModuleHandleW(nullptr);
    WNDCLASSW wc{};
    wc.lpfnWndProc = DefWindowProcW;
    wc.hInstance = instance;
    wc.lpszClassName = L"Stage1TerminalPageCopyTest";
    assert(RegisterClassW(&wc));
    HWND window = CreateWindowW(wc.lpszClassName, L"Stage1 page copy test", WS_OVERLAPPEDWINDOW,
        0, 0, 800, 600, nullptr, nullptr, instance, nullptr);
    assert(window); // Intentionally hidden, never steals user input.
    D3D11Renderer renderer;
    assert(renderer.Initialize(window, 800, 600));
    ID3D11ShaderResourceView* pages[2]{};
    renderer.BeginFrame(1, 0, 0);
    assert(renderer.CaptureFrameTexture(pages[0]));
    renderer.EndFrame();
    renderer.BeginFrame(0, 1, 0);
    assert(renderer.CaptureFrameTexture(pages[1]));
    renderer.EndFrame();
    assert(renderer.GetSuccessfulPresentCount() == 2);
    CheckTexture(pages[0], 0xFF0000FFu);
    CheckTexture(pages[1], 0xFF00FF00u);
    ID3D11ShaderResourceView* visible = nullptr;
    renderer.BeginFrame(0, 1, 0); // newly rendered back is green
    renderer.DrawSprite(pages[0], 0, 0, 800, 600); // wait: old front remains red
    assert(renderer.CaptureFrameTexture(visible));
    CheckTexture(visible, 0xFF0000FFu);
    renderer.DrawSprite(pages[1], 0, 0, 800, 600); // eligible: publish green
    assert(renderer.CaptureFrameTexture(visible));
    CheckTexture(visible, 0xFF00FF00u);
    renderer.DestroyTexture(visible);
    assert(!renderer.CopyFrameTexture(nullptr, pages[0]));
    assert(!renderer.CopyFrameTexture(pages[0], pages[0]));
    // Exercise both native draw-page directions with actual GPU contents.
    assert(renderer.CopyFrameTexture(pages[1], pages[0]));
    CheckTexture(pages[1], 0xFF0000FFu);
    renderer.BeginFrame(0, 0, 1);
    assert(renderer.CaptureFrameTexture(pages[1]));
    assert(renderer.CopyFrameTexture(pages[0], pages[1]));
    CheckTexture(pages[0], 0xFFFF0000u);
    renderer.DestroyTexture(pages[0]);
    renderer.DestroyTexture(pages[1]);
    renderer.Shutdown();
    DestroyWindow(window);
    UnregisterClassW(wc.lpszClassName, instance);
    std::puts("Stage1 terminal presentation: PASS (shared VSync(2) before four actual presents, old-front redraw exclusion, bidirectional 800x600 GPU pixels)");
}
