#include "pr/pr_modern_rail_animation.h"
#include "test_stage2_gpu_image_readback.h"
#include <filesystem>
#include <iostream>

namespace {
void Require(bool value, const char* message) {
    if (!value) throw std::runtime_error(message);
}
void Quad(D3D11Renderer& renderer, bool triangles, ID3D11ShaderResourceView* texture,
          float size, float r, float g, float b, float alpha, D3D11Renderer::BlendMode blend) {
    const float x = 160 - size / 2, y = 120 - size / 2;
    if (!triangles) {
        renderer.DrawSpriteTint(texture, x, y, size, size, 0, 0, 1, 1, r, g, b, alpha, blend);
        return;
    }
    TexturedVertex vertices[6];
    constexpr int corners[]{0, 1, 2, 2, 1, 3};
    for (int i = 0; i < 6; ++i) {
        const int c = corners[i];
        vertices[i] = {x + (c & 1) * size, y + ((c >> 1) & 1) * size,
            float(c & 1), float((c >> 1) & 1), r, g, b, alpha};
    }
    renderer.DrawTexturedTriangleBatch(texture, vertices, 6, blend, D3D11Renderer::TextureMask::All, true);
}
TestStage2GpuReadback::Image Frame(D3D11Renderer& renderer, bool triangles,
    ID3D11ShaderResourceView* glow, ID3D11ShaderResourceView* note, float age, float alpha,
    const std::filesystem::path& output, bool lightBehind = false) {
    renderer.BeginFrame(0, 0, 0);
    const auto pose = PrModernRailAnimation::Sample(age, 5, 2, 14, 16);
    // A blue opaque note makes any additive contribution measurable, including
    // the brightest pixels which used to be covered by the oversized note.
    const auto drawNote = [&] { Quad(renderer, triangles, note, 48 * pose.y,
        0.1f, 0.25f, 0.45f, 1, D3D11Renderer::BlendMode::Alpha); };
    if (!lightBehind) drawNote();
    for (const auto& light : PrModernRailAnimation::GlowLayers(48, pose.x, pose.y, 1.75f, alpha * pose.glow)) {
        Quad(renderer, triangles, glow, light.size, light.r, light.g, light.b, light.alpha,
             D3D11Renderer::BlendMode::Additive);
    }
    if (lightBehind) drawNote();
    ComPtr<ID3D11ShaderResourceView> frame;
    Require(renderer.CaptureFrameTexture(*frame.GetAddressOf()), "Glow GPU capture failed");
    const auto image = TestStage2GpuReadback::Read(frame.Get());
    if (!output.empty()) Require(renderer.SaveRgbaPng(output.wstring(), image.rgba.data(), image.width, image.height), "Glow PNG failed");
    return image;
}
}

int main(int argc, char** argv) {
    try {
        Require(argc == 2, "Expected output directory");
        const auto output = std::filesystem::u8path(argv[1]);
        const HWND window = CreateWindowExW(0, L"STATIC", L"Modern note glow verification",
            WS_OVERLAPPEDWINDOW, 0, 0, 320, 240, nullptr, nullptr, GetModuleHandleW(nullptr), nullptr);
        Require(window != nullptr, "Test window failed");
        struct Guard { HWND h; ~Guard() { DestroyWindow(h); } } guard{window};
        D3D11Renderer renderer;
        Require(renderer.Initialize(window, 320, 240), "D3D initialization failed");
        const auto pixels = PrModernRailAnimation::GlowPixels();
        ComPtr<ID3D11ShaderResourceView> glow, note;
        glow.Attach(renderer.CreateTexture(pixels.data(), 64, 64));
        const uint32_t white = 0xFFFFFFFFu;
        note.Attach(renderer.CreateTexture(&white, 1, 1));
        Require(glow && note, "Test textures failed");
        const size_t center = 120 * 320 + 160;
        for (float age : {0.0f, 0.5f, 1.0f, 2.5f, 5.0f, 8.0f, 19.0f}) {
            TestStage2GpuReadback::Image sprite;
            for (bool triangles : {false, true}) {
                const std::string label = (triangles ? "triangles_" : "sprites_") + std::to_string(age);
                const auto unlit = Frame(renderer, triangles, glow.Get(), note.Get(), age, 0, {});
                const auto lit = Frame(renderer, triangles, glow.Get(), note.Get(), age, 0.55f, output / (label + ".png"));
                const int red = int(lit.rgba[center] & 255) - int(unlit.rgba[center] & 255);
                if (age <= 1) Require(red > 80, "Early impact has no visible light on the note");
                if (age == 2.5f) Require(red > 15, "Mid-impact light disappeared too early");
                if (age >= 5) Require(lit.rgba == unlit.rgba, "Glow outlived the impact window");
                Require(lit.rgba[0] == unlit.rgba[0], "Additive black border changed the background");
                if (!triangles) sprite = lit;
                else for (size_t i = 0; i < sprite.rgba.size(); ++i) for (int shift : {0, 8, 16})
                    Require(std::abs(int((sprite.rgba[i] >> shift) & 255) - int((lit.rgba[i] >> shift) & 255)) <= 2,
                            "Sprite and native triangle glow falloff diverged");
                std::cout << label << " center_red_increase=" << red << '\n';
            }
        }
        const auto covered = Frame(renderer, false, glow.Get(), note.Get(), 0, 0.55f, output / "light_behind_note.png", true);
        const auto dark = Frame(renderer, false, glow.Get(), note.Get(), 0, 0, {});
        Require((covered.rgba[center] & 0xFFFFFFu) == (dark.rgba[center] & 0xFFFFFFu), "Occlusion diagnosis did not reproduce");
        std::cout << "PASS visible impact, fast fade, disabled glow, shader parity and occlusion diagnosis\n";
    } catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
