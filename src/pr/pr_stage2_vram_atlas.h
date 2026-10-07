#pragma once
#include "pr_stage2_vram_device.h"
#include "pr_vram_atlas.h"
#include <unordered_map>

namespace PrStage2VramAtlas {
enum class Encoding { Opaque, Abr0Stp, Abr1Stp };
struct Rect { int x, y, width, height; };

// Host presentation adapter, not another translated PSX function. Its owner
// must outlive the atlas and renderer must outlive its cached SRVs. It binds
// one real VRAM device; it never borrows Scene1/SS0's global atlas or reloads
// files. Scene changes do not implicitly clear the shared native VRAM.
class Projection {
public:
    explicit Projection(const PrStage2VramDevice::Device& vram) : vram_(vram) {}
    Projection(const Projection&) = delete;
    Projection& operator=(const Projection&) = delete;
    bool CopyRgbaRect(uint16_t tpage, uint16_t clut, Rect rect,
                      std::vector<uint32_t>& pixels);
    // A failed/unknown region or GPU allocation returns null, not an old
    // texture. A returned view is borrowed; prepared batches must hold a COM
    // reference if VRAM may change before they are consumed.
    ID3D11ShaderResourceView* Resolve(uint16_t tpage, uint16_t clut, Rect rect,
                                     D3D11Renderer& renderer, Encoding encoding);
    uint64_t Revision() const noexcept { return vram_.WrittenWords(); }
    void Clear(); // only host caches; no native RAM/VRAM side effects
private:
    bool Prepare(uint16_t tpage, uint16_t clut);
    const PrStage2VramDevice::Device& vram_;
    PsxVramAtlas atlas_;
    std::unordered_map<uint32_t, uint64_t> revisions_;
    D3D11Renderer* renderer_ = nullptr;
};
}
