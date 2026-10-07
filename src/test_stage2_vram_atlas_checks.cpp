#include "test_stage2_vram_atlas_checks.h"
#include "pr/pr_stage2_vram_atlas.h"
#include <algorithm>
#include <array>
#include <fstream>
#include <iostream>
#include <iterator>
#include <map>
#include <set>
#include <stdexcept>
#include <tuple>

namespace {
using PrStage2VramAtlas::Encoding;
using PrStage2VramAtlas::Projection;
using PrStage2VramAtlas::Rect;
void Require(bool value, const char* message) {
    if (!value) throw std::runtime_error(message);
}
struct HiddenWindow {
    HWND hwnd = CreateWindowExW(0, L"STATIC", L"S2 isolated GPU readback",
        WS_OVERLAPPEDWINDOW, 0, 0, 64, 64, nullptr, nullptr, GetModuleHandleW(nullptr), nullptr);
    ~HiddenWindow() { if (hwnd) DestroyWindow(hwnd); }
};
std::vector<uint32_t> ReadGpu(ID3D11ShaderResourceView* srv) {
    Require(srv != nullptr, "GPU view missing");
    ComPtr<ID3D11Resource> resource;
    srv->GetResource(&resource);
    ComPtr<ID3D11Texture2D> texture;
    Require(SUCCEEDED(resource.As(&texture)), "GPU view is not Texture2D");
    D3D11_TEXTURE2D_DESC desc{};
    texture->GetDesc(&desc);
    Require(desc.Width == 256 && desc.Height == 256 && desc.Format == DXGI_FORMAT_R8G8B8A8_UNORM,
            "unexpected GPU texture shape/format");
    ComPtr<ID3D11Device> device;
    texture->GetDevice(&device);
    ComPtr<ID3D11DeviceContext> context;
    device->GetImmediateContext(&context);
    desc.Usage = D3D11_USAGE_STAGING;
    desc.BindFlags = desc.MiscFlags = 0;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    ComPtr<ID3D11Texture2D> staging;
    Require(SUCCEEDED(device->CreateTexture2D(&desc, nullptr, &staging)), "staging allocation failed");
    context->CopyResource(staging.Get(), texture.Get());
    D3D11_MAPPED_SUBRESOURCE mapped{};
    Require(SUCCEEDED(context->Map(staging.Get(), 0, D3D11_MAP_READ, 0, &mapped)), "GPU readback failed");
    std::vector<uint32_t> pixels(256u * 256u);
    for (size_t row = 0; row < 256u; ++row)
        std::memcpy(pixels.data() + row * 256u,
                    static_cast<const uint8_t*>(mapped.pData) + row * mapped.RowPitch, 1024u);
    context->Unmap(staging.Get(), 0);
    return pixels;
}
uint64_t Hash(const std::vector<uint32_t>& pixels) {
    uint64_t h = 14695981039346656037ull;
    for (uint32_t value : pixels)
        for (unsigned byte = 0; byte < 4u; ++byte)
            h = (h ^ static_cast<uint8_t>(value >> (8u * byte))) * 1099511628211ull;
    return h;
}
void Synthetic(PrStage2LifecycleDirect::Services& s, D3D11Renderer& renderer) {
    PrStage2VramDevice::Device vram;
    Projection atlas(vram);
    constexpr uint32_t scratch = 0x801FD000u; // isolated test scratch, not product RAM allocation
    constexpr uint16_t palette = 480u << 6u;
    const Rect first{0, 0, 4, 1};
    std::vector<uint32_t> pixels;
    Require(!atlas.CopyRgbaRect(0, palette, first, pixels), "unwritten palette accepted");
    auto write = [&](uint16_t x, uint16_t y, const std::vector<uint16_t>& words) {
        for (size_t i = 0; i < words.size(); ++i) s.Write16(scratch + static_cast<uint32_t>(i) * 2u, words[i]);
        s.Write16(scratch + static_cast<uint32_t>(words.size()) * 2u, 0xA55Au);
        vram.UploadImageWords(s, {x, y, static_cast<uint16_t>(words.size()), 1u}, scratch);
    };
    write(0, 480, std::vector<uint16_t>(15u, 0x001Fu));
    write(0, 0, {0u});
    Require(!atlas.CopyRgbaRect(0, palette, first, pixels), "partial palette accepted as complete");
    // Use another page word below; the one above must not make unwritten
    // image positions valid through the otherwise complete palette.
    write(0, 480, std::vector<uint16_t>(256u, 0x8000u));
    Require(!atlas.CopyRgbaRect(0, palette, {4,0,1,1}, pixels), "unknown image accepted as palette zero");
    std::vector<uint16_t> colors(256u);
    colors[1] = 0x8000u; colors[2] = 0x001Fu; colors[3] = 0x83E0u; colors[4] = 0x7C00u;
    write(0, 480, colors);
    write(0, 0, {0x3210u});
    Require(atlas.CopyRgbaRect(0, palette, first, pixels) &&
        pixels == std::vector<uint32_t>{0, 0xFF000000u, 0xFF0000FFu, 0xFF00FF00u}, "4-bit native CPU colors");
    const std::array<std::array<uint32_t, 4>, 3> expected{{
        {{0, 0xFF000000u, 0xFF0000FFu, 0xFF00FF00u}},
        {{0, 0x80000000u, 0xFF0000FFu, 0x8000FF00u}},
        {{0, 0, 0xFF0000FFu, 0x0000FF00u}}}};
    ComPtr<ID3D11ShaderResourceView> old;
    for (int encoding = 0; encoding < 3; ++encoding) {
        auto* view = atlas.Resolve(0, palette, first, renderer, static_cast<Encoding>(encoding));
        auto actual = ReadGpu(view);
        Require(std::equal(expected[encoding].begin(), expected[encoding].end(), actual.begin()), "GPU STP encoding");
        Require(actual[4] == 0, "unknown texel colored through palette zero");
        Require(view == atlas.Resolve(0x60u, palette, first, renderer, static_cast<Encoding>(encoding)), "ABR changed page identity/cache");
        if (!encoding) old = view;
    }
    Require(!atlas.Resolve(0, palette, {4,0,1,1}, renderer, Encoding::Opaque), "unknown draw region accepted");
    Require(!atlas.CopyRgbaRect(0, palette, {1,0,0x7FFFFFFF,1}, pixels), "overflow rectangle accepted");
    Require(!atlas.CopyRgbaRect(0x100u, palette, first, pixels), "unsupported color mode accepted");
    Require(!atlas.CopyRgbaRect(0x8Fu, palette, first, pixels), "wrapping page silently clipped");
    colors[2] = 0x7C00u;
    write(0, 480, colors);
    auto updated = ReadGpu(atlas.Resolve(0, palette, first, renderer, Encoding::Opaque));
    Require(updated[2] == 0xFFFF0000u, "palette update left stale GPU cache");
    Require(ReadGpu(old.Get())[2] == 0xFF0000FFu, "retained prepared view overwritten");
    write(0, 0, {0x2222u});
    updated = ReadGpu(atlas.Resolve(0, palette, first, renderer, Encoding::Opaque));
    Require(std::all_of(updated.begin(), updated.begin()+4, [](uint32_t v) { return v == 0xFFFF0000u; }), "partial image write not reflected");
    write(64, 0, {0x0304u});
    Require(atlas.CopyRgbaRect(0x81u, palette, {0,0,2,1}, pixels) &&
        pixels == std::vector<uint32_t>{0xFFFF0000u, 0xFF00FF00u}, "8-bit index order");
    // A later 4-bit use of the same CLUT cannot truncate the 8-bit palette.
    write(0, 480, {0x001Fu});
    Require(atlas.CopyRgbaRect(0, palette, first, pixels), "shared 4-bit palette refresh");
    Require(ReadGpu(atlas.Resolve(0x81u, palette, {0,0,2,1}, renderer, Encoding::Opaque))[0] == 0xFFFF0000u,
            "shared 8-bit palette lost");
    const auto wordsBefore = vram.WrittenWords();
    atlas.Clear();
    Require(vram.WrittenWords() == wordsBefore && atlas.CopyRgbaRect(0, palette, first, pixels), "cache clear changed native VRAM");
    // Legacy TIM input remains strict and the native path does not allocate
    // standalone TIM snapshots or replace a file's encoded data.
    PsxVramAtlas legacy;
    Require(!legacy.CanLoadTim(nullptr, 0), "legacy preflight relaxed");
    std::cout << "atlas-synthetic pass\n";
}
}

static void CheckNativeAtlas(PrStage2LifecycleDirect::Services& s,
                              const std::vector<uint32_t>& sizes, uint32_t pass,
                              Projection& atlas, D3D11Renderer& renderer) {
    using Point = std::tuple<uint16_t,uint16_t,int,int>;
    std::set<Point> points;
    uint32_t models = 0;
    for (size_t slot = 0; slot < sizes.size(); ++slot) {
        const uint32_t start = s.Read32(0x80091858u + static_cast<uint32_t>(slot+1u) * 4u);
        const uint32_t size = sizes[slot];
        if (size < 12u || s.Read32(start) != 0x41u) continue;
        Require(s.Read32(start+4u) == 0u, "unexpected mapped TMD in raw resource test");
        ++models;
        const uint32_t objects = s.Read32(start+8u);
        Require(objects <= (size-12u)/28u, "TMD object table out of bounds");
        for (uint32_t object = 0; object < objects; ++object) {
            const uint32_t record = start+12u+28u*object;
            uint32_t primitive = 12u+s.Read32(record+16u);
            const uint32_t count = s.Read32(record+20u);
            for (uint32_t index = 0; index < count; ++index) {
                Require(primitive < size && size-primitive >= 4u, "TMD primitive header range");
                const uint32_t p = start+primitive;
                const uint32_t bytes = 4u+4u*s.Read8(p+1u);
                Require(bytes <= size-primitive, "TMD primitive payload range");
                const uint8_t mode = s.Read8(p+3u);
                if ((mode & 4u) && (mode & 0xE0u) == 0x20u) {
                    const uint16_t clut = s.Read16(p+6u), tpage = s.Read16(p+10u);
                    const uint32_t vertices = (mode & 8u) ? 4u : 3u;
                    Require(bytes >= 4u+4u*vertices, "TMD UV payload range");
                    for (uint32_t v = 0; v < vertices; ++v)
                        points.emplace(tpage, clut, s.Read8(p+4u+4u*v), s.Read8(p+5u+4u*v));
                }
                primitive += bytes;
            }
        }
    }
    uint32_t known = 0, unknown = 0;
    uint64_t cpuHash = 14695981039346656037ull;
    std::map<std::pair<uint16_t,uint16_t>, Rect> materials;
    for (auto [page, clut, u, v] : points) {
        std::vector<uint32_t> pixel;
        const bool ready = atlas.CopyRgbaRect(page, clut, {u,v,1,1}, pixel);
        for (uint32_t value : {uint32_t(page),uint32_t(clut),uint32_t(u),uint32_t(v),uint32_t(ready),ready ? pixel[0] : 0u})
            for (unsigned byte = 0; byte < 4u; ++byte)
                cpuHash = (cpuHash ^ static_cast<uint8_t>(value >> (8u*byte))) * 1099511628211ull;
        if (ready) { ++known; materials.emplace(std::make_pair(page,clut), Rect{u,v,1,1}); }
        else ++unknown;
    }
    for (const auto& item : materials) {
        for (int encoding = 0; encoding < 3; ++encoding) {
            auto* srv = atlas.Resolve(item.first.first, item.first.second, item.second,
                                       renderer, static_cast<Encoding>(encoding));
            const auto pixels = ReadGpu(srv);
            std::cout << "atlas-gpu " << pass << ' ' << item.first.first << ' ' << item.first.second
                      << ' ' << encoding << ' ' << Hash(pixels) << '\n';
        }
    }
    Require(models != 0 && known != 0 && !materials.empty(), "no real S2 texture exercised");
    std::cout << "atlas-native " << pass << ' ' << models << ' ' << known << ' ' << unknown
              << ' ' << materials.size() << ' ' << cpuHash << '\n';
    Synthetic(s, renderer);
}

void RunStage2VramAtlasChecks(PrStage2LifecycleDirect::Services& s,
                              const PrStage2VramDevice::Device& vram,
                              const std::vector<uint32_t>& sizes, uint32_t pass) {
    HiddenWindow window;
    Require(window.hwnd != nullptr, "hidden GPU window failed");
    D3D11Renderer renderer;
    Require(renderer.Initialize(window.hwnd, 64, 64), "hardware D3D initialization failed");
    Projection atlas(vram); // destroyed before renderer
    CheckNativeAtlas(s, sizes, pass, atlas, renderer);
}

void RunStage2RuntimeTimChecks(PrStage2LifecycleDirect::Services& s,
                              const PrStage2VramDevice::Device& vram,
                              const std::vector<uint32_t>& sizes, const char* modulePath) {
    // Only this isolated fixture copies the original module into test RAM.
    // This is not the product's module loader or its entry/reload lifetime.
    std::ifstream file(modulePath, std::ios::binary);
    const std::vector<uint8_t> module((std::istreambuf_iterator<char>(file)), {});
    Require(module.size() == 79904u, "unexpected original S2 module size");
    for (uint32_t i = 0; i < module.size(); ++i) s.Write8(0x801C3870u+i, module[i]);
    std::array<uint32_t,14> slots{};
    for (uint32_t i = 0; i < slots.size(); ++i) {
        slots[i] = s.Read32(0x801D28D8u+4u*i);
        Require(slots[i] > 0u && slots[i]+1u <= sizes.size(), "original TIM slot out of bounds");
        for (uint32_t slot : {slots[i],slots[i]+1u}) {
            const uint32_t tim = s.Read32(0x80091858u+4u*slot);
            Require(s.Read32(tim) == 0x10u, "runtime fixture slot is not TIM");
        }
    }
    HiddenWindow window;
    Require(window.hwnd != nullptr, "runtime hidden GPU window failed");
    D3D11Renderer renderer;
    Require(renderer.Initialize(window.hwnd, 64, 64), "runtime hardware D3D initialization failed");
    Projection atlas(vram); // SAME real cache before/after both native upload blocks
    auto state = [&](const char* phase) {
        uint64_t hash = 14695981039346656037ull;
        for (size_t i = 0; i < vram.WordCount; ++i) {
            const uint16_t word = vram.Words()[i];
            for (uint8_t byte : {static_cast<uint8_t>(word),static_cast<uint8_t>(word>>8u),vram.Known()[i]})
                hash = (hash^byte)*1099511628211ull;
        }
        std::cout << "runtime-state " << phase << ' ' << vram.UploadCount() << ' ' << vram.SyncCount()
                  << ' ' << vram.KnownWords() << ' ' << vram.WrittenWords() << ' ' << hash << '\n';
    };
    std::cout << "runtime-begin\n";
    state("baseline");
    CheckNativeAtlas(s, sizes, 2u, atlas, renderer);
    PrStage2LifecycleDirect::UploadInitialRuntimeTims801CB284(s);
    state("initial");
    CheckNativeAtlas(s, sizes, 3u, atlas, renderer);

    // Deliberate list INPUT, not a claim of executing the original event
    // timeline: first 7 real variants with CLUT, last 7 image-only. Execute
    // the exact original frame upload block, including pointer rereads.
    constexpr uint32_t work = 0x801F8000u, first = 0x801F9000u, second = 0x801F9100u;
    s.Write32(work+172u, first); s.Write32(work+176u, second);
    for (uint32_t i = 0; i < slots.size(); ++i)
        s.Write16((i < 7u ? first : second)+2u*(i%7u), static_cast<uint16_t>(slots[i]+1u));
    s.Write16(first+14u, 0u); s.Write16(second+14u, 0u);
    PrStage2LifecycleDirect::UploadFrameRuntimeTims801CA57C(s, work);
    state("frame_fixture");
    CheckNativeAtlas(s, sizes, 4u, atlas, renderer);
}
