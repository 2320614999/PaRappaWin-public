#include "pr_stage2_vram_atlas.h"

namespace PrStage2VramAtlas {
void Projection::Clear() {
    atlas_.Clear();
    revisions_.clear();
    renderer_ = nullptr;
}
bool Projection::Prepare(uint16_t tpage, uint16_t clut) {
    const uint16_t page = static_cast<uint16_t>(tpage & 0x019Fu);
    const uint32_t key = (uint32_t(page) << 16u) | clut;
    const auto previous = revisions_.find(key);
    if (previous != revisions_.end() && previous->second == vram_.WrittenWords())
        return true;
    if (!atlas_.ProjectNativeIndexedPage(vram_.Words().data(), vram_.Known().data(),
                                         vram_.Words().size(), page, clut))
        return false;
    revisions_[key] = vram_.WrittenWords();
    return true;
}
bool Projection::CopyRgbaRect(uint16_t tpage, uint16_t clut, Rect r,
                              std::vector<uint32_t>& pixels) {
    pixels.clear();
    return Prepare(tpage, clut) &&
        atlas_.IsNativeRectKnown(tpage, r.x, r.y, r.width, r.height) &&
        atlas_.CopyRgbaRect(tpage, clut, r.x, r.y, r.width, r.height, pixels);
}
ID3D11ShaderResourceView* Projection::Resolve(uint16_t tpage, uint16_t clut, Rect r,
                                              D3D11Renderer& renderer, Encoding encoding) {
    // SRVs cannot be reused across devices. Keep this explicit; the runtime
    // owner calls Clear before replacing/destroying its renderer.
    if (renderer_ && renderer_ != &renderer) return nullptr;
    if (!Prepare(tpage, clut) ||
        !atlas_.IsNativeRectKnown(tpage, r.x, r.y, r.width, r.height)) return nullptr;
    renderer_ = &renderer;
    switch (encoding) {
    case Encoding::Opaque: return atlas_.GetTpageSRV(tpage, clut, &renderer);
    case Encoding::Abr0Stp: return atlas_.GetTpagePsxAbr0StpSRV(tpage, clut, &renderer);
    case Encoding::Abr1Stp: return atlas_.GetTpagePsxAbr1StpSRV(tpage, clut, &renderer);
    }
    return nullptr;
}
}
