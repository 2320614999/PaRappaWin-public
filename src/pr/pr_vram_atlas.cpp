#include "pr_vram_atlas.h"
#include "logger.h"
#include <cstring>
#include <algorithm>
#include <unordered_set>

PsxVramAtlas::PsxVramAtlas() {}

namespace {

uint16_t NormalizeTpageTextureKey(uint16_t tpage) {
    // PSX TSB bits 5-6 are ABR blend mode, not VRAM page address.
    return (uint16_t)((tpage & 0x01FFu) & ~0x0060u);
}

}  // namespace

PsxVramAtlas::~PsxVramAtlas() {
    for (auto& [k, tp] : m_tpages) {
        DestroyClutSrvs(tp);
        if (tp.srv && m_renderer) {
            m_renderer->DestroyTexture(tp.srv);
            tp.srv = nullptr;
        }
    }
}

void PsxVramAtlas::Clear() {
    for (auto& [k, tp] : m_tpages) {
        DestroyClutSrvs(tp);
        if (tp.srv && m_renderer) {
            m_renderer->DestroyTexture(tp.srv);
            tp.srv = nullptr;
        }
    }
    m_tpages.clear();
    m_cluts.clear();
    m_requiredCluts.clear();
    m_loadedCount = 0;
}

TpageTexture& PsxVramAtlas::GetOrCreateTpage(uint16_t tpage) {
    tpage = NormalizeTpageTextureKey(tpage);
    auto it = m_tpages.find(tpage);
    if (it != m_tpages.end()) return it->second;

    TpageTexture& tp = m_tpages[tpage];
    tp.tpage = tpage;
    tp.colorMode = (tpage >> 7) & 3;
    tp.baseHW = (tpage & 0xF) * 64;
    tp.baseY = ((tpage >> 4) & 1) * 256;
    tp.indexedPixels.resize(TPAGE_W * TPAGE_H, 0);
    tp.pixels.resize(TPAGE_W * TPAGE_H, 0);
    tp.dirty = true;
    return tp;
}

void PsxVramAtlas::DestroyClutSrvs(TpageTexture& tp) {
    if (!m_renderer) {
        tp.clutSrvs.clear();
        return;
    }
    for (auto& [clut, srv] : tp.clutSrvs) {
        if (srv) {
            m_renderer->DestroyTexture(srv);
        }
    }
    tp.clutSrvs.clear();
}

void PsxVramAtlas::RegisterClutRows(const uint16_t* clut,
                                    int clutW,
                                    int clutH,
                                    int clutX,
                                    int clutY) {
    if (!clut || clutW <= 0 || clutH <= 0) {
        return;
    }

    for (int row = 0; row < clutH; ++row) {
        const uint16_t cba =
            (uint16_t)((((clutY + row) & 0x01FF) << 6) | ((clutX >> 4) & 0x3F));
        std::vector<uint16_t> colors;
        colors.resize((size_t)clutW);
        std::memcpy(colors.data(), clut + (size_t)row * (size_t)clutW,
                    sizeof(uint16_t) * (size_t)clutW);
        m_cluts[cba] = std::move(colors);
    }
}

void PsxVramAtlas::RegisterTpage(uint16_t tpage) {
    GetOrCreateTpage(tpage);
}

void PsxVramAtlas::RegisterClut(uint16_t clut) {
    m_requiredCluts.insert(clut);
}

bool PsxVramAtlas::TimHasRequiredClutRow(int clutX, int clutY, int clutH) const {
    if (m_requiredCluts.empty()) {
        return true;
    }
    if (clutH <= 0) {
        return false;
    }

    for (int row = 0; row < clutH; ++row) {
        const uint16_t cba =
            (uint16_t)((((clutY + row) & 0x01FF) << 6) | ((clutX >> 4) & 0x3F));
        if (m_requiredCluts.find(cba) != m_requiredCluts.end()) {
            return true;
        }
    }
    return false;
}

void PsxVramAtlas::DecodeTim4bit(TpageTexture& tp,
                                  const uint8_t* pixelData, size_t pixelBytes,
                                  int imgX, int imgY, int imgW_hw, int imgH,
                                  const uint16_t* clut, int clutSize) {
    DestroyClutSrvs(tp);
    // imgX is in halfword units. Convert to pixel offset within tpage:
    // pixel offset = (imgX - tp.baseHW) * 4
    int pixOfsX = (imgX - tp.baseHW) * 4;
    int pixOfsY = imgY - tp.baseY;

    int srcIdx = 0;
    for (int y = 0; y < imgH; y++) {
        int ty = pixOfsY + y;
        if (ty < 0 || ty >= TPAGE_H) { srcIdx += imgW_hw * 2; continue; }
        for (int xw = 0; xw < imgW_hw; xw++) {
            if (srcIdx + 1 >= (int)pixelBytes) break;
            uint8_t b0 = pixelData[srcIdx++];
            uint8_t b1 = pixelData[srcIdx++];
            uint8_t indices[4] = {
                (uint8_t)(b0 & 0xF), (uint8_t)((b0 >> 4) & 0xF),
                (uint8_t)(b1 & 0xF), (uint8_t)((b1 >> 4) & 0xF)
            };
            for (int p = 0; p < 4; p++) {
                int tx = pixOfsX + xw * 4 + p;
                if (tx < 0 || tx >= TPAGE_W) continue;
                uint8_t idx = indices[p];
                tp.indexedPixels[ty * TPAGE_W + tx] = idx;
                uint32_t rgba = 0u;
                if (idx < clutSize) {
                    rgba = TimDecoder::ConvertABGR1555toRGBA8888(clut[idx]);
                }
                tp.pixels[ty * TPAGE_W + tx] = rgba;
            }
        }
    }
    tp.dirty = true;
}

void PsxVramAtlas::DecodeTim8bit(TpageTexture& tp,
                                  const uint8_t* pixelData, size_t pixelBytes,
                                  int imgX, int imgY, int imgW_hw, int imgH,
                                  const uint16_t* clut, int clutSize) {
    DestroyClutSrvs(tp);
    // imgX is in halfword units. Convert to pixel offset within tpage:
    // pixel offset = (imgX - tp.baseHW) * 2
    int pixOfsX = (imgX - tp.baseHW) * 2;
    int pixOfsY = imgY - tp.baseY;

    int srcIdx = 0;
    for (int y = 0; y < imgH; y++) {
        int ty = pixOfsY + y;
        if (ty < 0 || ty >= TPAGE_H) { srcIdx += imgW_hw * 2; continue; }
        for (int xw = 0; xw < imgW_hw; xw++) {
            if (srcIdx + 1 >= (int)pixelBytes) break;
            uint8_t p0 = pixelData[srcIdx++];
            uint8_t p1 = pixelData[srcIdx++];
            int tx0 = pixOfsX + xw * 2;
            int tx1 = tx0 + 1;
            if (tx0 >= 0 && tx0 < TPAGE_W) {
                tp.indexedPixels[ty * TPAGE_W + tx0] = p0;
                uint32_t rgba = 0u;
                if (p0 < clutSize) rgba = TimDecoder::ConvertABGR1555toRGBA8888(clut[p0]);
                tp.pixels[ty * TPAGE_W + tx0] = rgba;
            }
            if (tx1 >= 0 && tx1 < TPAGE_W) {
                tp.indexedPixels[ty * TPAGE_W + tx1] = p1;
                uint32_t rgba = 0u;
                if (p1 < clutSize) rgba = TimDecoder::ConvertABGR1555toRGBA8888(clut[p1]);
                tp.pixels[ty * TPAGE_W + tx1] = rgba;
            }
        }
    }
    tp.dirty = true;
}

bool PsxVramAtlas::LoadTim(const uint8_t* raw,
                            size_t size,
                            const std::string& name,
                            bool uploadClut,
                            bool filterToRequiredClutRow) {
    if (!raw || size < 20) return false;

    uint32_t magic, flags;
    std::memcpy(&magic, raw, 4);
    std::memcpy(&flags, raw + 4, 4);
    if (magic != 0x10) return false;

    uint32_t pmode = flags & 7;
    bool hasClut = (flags >> 3) & 1;
    size_t off = 8;

    const uint16_t* clutData = nullptr;
    int clutSize = 0;
    uint16_t clutX = 0;
    uint16_t clutY = 0;
    uint16_t clutW = 0;
    uint16_t clutH = 0;
    if (hasClut) {
        if (off + 12 > size) return false;
        uint32_t clutLen;
        std::memcpy(&clutLen, raw + off, 4);
        std::memcpy(&clutX, raw + off + 4, 2);
        std::memcpy(&clutY, raw + off + 6, 2);
        std::memcpy(&clutW, raw + off + 8, 2);
        std::memcpy(&clutH, raw + off + 10, 2);
        clutData = (const uint16_t*)(raw + off + 12);
        clutSize = (int)(clutW * clutH);
        off += clutLen;
    }

    if (off + 12 > size) return false;
    uint32_t imgLen;
    uint16_t imgX, imgY, imgW, imgH;
    std::memcpy(&imgLen, raw + off, 4);
    std::memcpy(&imgX, raw + off + 4, 2);
    std::memcpy(&imgY, raw + off + 6, 2);
    std::memcpy(&imgW, raw + off + 8, 2);
    std::memcpy(&imgH, raw + off + 10, 2);
    const uint8_t* pixelData = raw + off + 12;
    size_t pixelBytes = (off + imgLen <= size) ? (imgLen - 12) : (size - off - 12);

    // Determine which tpage(s) this TIM falls into
    if (pmode == 0 && hasClut) {
        // 4-bit: each halfword = 4 pixels, tpage = 64 halfwords wide
        int tpageIdx = imgX / 64;
        int tpageY = (imgY >= 256) ? 1 : 0;
        // Reconstruct tpage value: idx | (Y << 4) | (mode << 7)
        uint16_t tpage = NormalizeTpageTextureKey(
            (uint16_t)(tpageIdx | (tpageY << 4) | (0 << 7)));

        auto it = m_tpages.find(tpage);
        TpageTexture* target = nullptr;
        if (it != m_tpages.end()) {
            target = &it->second;
        } else if (!filterToRequiredClutRow) {
            target = &GetOrCreateTpage(tpage);
        } else {
            static std::unordered_set<uint16_t> s_loggedMissing;
            if (s_loggedMissing.insert(tpage).second) {
                Log::Printf("LoadTim '%s': skipped - no matching tpage (tpage=0x%04X)", name.c_str(), tpage);
            }
            return false;
        }
        if (filterToRequiredClutRow &&
            !TimHasRequiredClutRow((int)clutX, (int)clutY, (int)clutH)) {
            return false;
        } else {
            if (uploadClut) {
                RegisterClutRows(clutData, (int)clutW, (int)clutH, (int)clutX, (int)clutY);
            }
            DecodeTim4bit(*target, pixelData, pixelBytes,
                          imgX, imgY, imgW, imgH, clutData, clutSize);
        }
    } else if (pmode == 1 && hasClut) {
        // 8-bit: each halfword = 2 pixels, tpage = 128 halfwords wide
        // But tpage index is still in 64-halfword steps, so an 8-bit tpage spans 2 indices
        int tpageIdx = imgX / 64;
        int tpageY = (imgY >= 256) ? 1 : 0;
        uint16_t tpage = NormalizeTpageTextureKey(
            (uint16_t)(tpageIdx | (tpageY << 4) | (1 << 7)));

        auto it = m_tpages.find(tpage);
        TpageTexture* target = nullptr;
        if (it != m_tpages.end()) {
            target = &it->second;
        } else if (!filterToRequiredClutRow) {
            target = &GetOrCreateTpage(tpage);
        } else {
            static std::unordered_set<uint16_t> s_loggedMissing;
            if (s_loggedMissing.insert(tpage).second) {
                Log::Printf("LoadTim '%s': skipped - no matching tpage (tpage=0x%04X)", name.c_str(), tpage);
            }
            return false;
        }
        if (filterToRequiredClutRow &&
            !TimHasRequiredClutRow((int)clutX, (int)clutY, (int)clutH)) {
            return false;
        } else {
            if (uploadClut) {
                RegisterClutRows(clutData, (int)clutW, (int)clutH, (int)clutX, (int)clutY);
            }
            DecodeTim8bit(*target, pixelData, pixelBytes,
                          imgX, imgY, imgW, imgH, clutData, clutSize);
        }
    } else {
        return false;
    }

    m_loadedCount++;
    return true;
}

void PsxVramAtlas::UploadAll(D3D11Renderer* renderer) {
    if (!renderer) return;
    m_renderer = renderer;

    int updated = 0, created = 0;
    for (auto& [k, tp] : m_tpages) {
        if (!tp.dirty) continue;
        if (tp.srv) {
            renderer->UpdateTexture(tp.srv, tp.pixels.data(), TPAGE_W, TPAGE_H);
            updated++;
        } else {
            tp.srv = renderer->CreateTexture(tp.pixels.data(), TPAGE_W, TPAGE_H);
            if (tp.srv) {
                if (updated == 0 && created == 0) {
                    Log::Printf("VramAtlas: uploaded tpage 0x%04X (mode=%d, base_hw=%d, baseY=%d)",
                                 tp.tpage, tp.colorMode, tp.baseHW, tp.baseY);
                }
                created++;
            }
        }
        tp.dirty = false;
    }
    if (created > 0) {
        Log::Printf("VramAtlas: recreated %d tpage texture(s)", created);
    }
}

ID3D11ShaderResourceView* PsxVramAtlas::GetTpageSRV(uint16_t tpage) const {
    tpage = NormalizeTpageTextureKey(tpage);
    auto it = m_tpages.find(tpage);
    if (it != m_tpages.end()) return it->second.srv;
    return nullptr;
}

ID3D11ShaderResourceView* PsxVramAtlas::BuildClutSrv(TpageTexture& tp,
                                                     uint16_t clut,
                                                     D3D11Renderer* renderer) {
    if (!renderer || tp.indexedPixels.size() != (size_t)TPAGE_W * (size_t)TPAGE_H) {
        return tp.srv;
    }

    const auto clutIt = m_cluts.find(clut);
    if (clutIt == m_cluts.end() || clutIt->second.empty()) {
        return tp.srv;
    }

    std::vector<uint32_t> pixels;
    pixels.resize((size_t)TPAGE_W * (size_t)TPAGE_H, 0u);
    const std::vector<uint16_t>& colors = clutIt->second;
    for (size_t i = 0; i < pixels.size(); ++i) {
        const uint8_t idx = tp.indexedPixels[i];
        if (idx < colors.size()) {
            pixels[i] = TimDecoder::ConvertABGR1555toRGBA8888(colors[idx]);
        }
    }

    return renderer->CreateTexture(pixels.data(), TPAGE_W, TPAGE_H);
}

ID3D11ShaderResourceView* PsxVramAtlas::GetTpageSRV(uint16_t tpage,
                                                    uint16_t clut,
                                                    D3D11Renderer* renderer) {
    tpage = NormalizeTpageTextureKey(tpage);
    auto it = m_tpages.find(tpage);
    if (it == m_tpages.end()) {
        return nullptr;
    }

    TpageTexture& tp = it->second;
    if (tp.colorMode > 1) {
        return tp.srv;
    }

    auto srvIt = tp.clutSrvs.find(clut);
    if (srvIt != tp.clutSrvs.end()) {
        return srvIt->second;
    }

    ID3D11ShaderResourceView* srv = BuildClutSrv(tp, clut, renderer);
    if (srv && srv != tp.srv) {
        m_renderer = renderer;
        tp.clutSrvs[clut] = srv;
    }
    return srv;
}

bool PsxVramAtlas::TryGetClutHasStpBits(uint16_t clut,
                                        bool& outHasStpBits) const {
    const auto clutIt = m_cluts.find(clut);
    if (clutIt == m_cluts.end() || clutIt->second.empty()) {
        outHasStpBits = false;
        return false;
    }

    outHasStpBits = false;
    for (uint16_t color : clutIt->second) {
        if ((color & 0x8000u) != 0u) {
            outHasStpBits = true;
            break;
        }
    }
    return true;
}

bool PsxVramAtlas::CopyRgbaRect(uint16_t tpage,
                                uint16_t clut,
                                int x,
                                int y,
                                int w,
                                int h,
                                std::vector<uint32_t>& out) const {
    out.clear();
    if (w <= 0 || h <= 0 || x < 0 || y < 0 ||
        x + w > TPAGE_W || y + h > TPAGE_H) {
        return false;
    }

    tpage = NormalizeTpageTextureKey(tpage);
    auto it = m_tpages.find(tpage);
    if (it == m_tpages.end()) {
        return false;
    }

    const TpageTexture& tp = it->second;
    if (tp.pixels.size() != static_cast<size_t>(TPAGE_W) *
                            static_cast<size_t>(TPAGE_H)) {
        return false;
    }

    out.resize(static_cast<size_t>(w) * static_cast<size_t>(h), 0u);
    const auto clutIt = m_cluts.find(clut);
    const bool useClut =
        tp.colorMode <= 1 && tp.indexedPixels.size() == tp.pixels.size() &&
        clutIt != m_cluts.end() && !clutIt->second.empty();
    const std::vector<uint16_t>* colors =
        useClut ? &clutIt->second : nullptr;

    for (int row = 0; row < h; ++row) {
        const size_t dstBase = static_cast<size_t>(row) *
                               static_cast<size_t>(w);
        const size_t srcBase = static_cast<size_t>(y + row) *
                                   static_cast<size_t>(TPAGE_W) +
                               static_cast<size_t>(x);
        if (colors != nullptr) {
            for (int col = 0; col < w; ++col) {
                const uint8_t idx = tp.indexedPixels[srcBase +
                                                     static_cast<size_t>(col)];
                if (idx < colors->size()) {
                    out[dstBase + static_cast<size_t>(col)] =
                        TimDecoder::ConvertABGR1555toRGBA8888((*colors)[idx]);
                }
            }
        } else {
            std::copy(tp.pixels.begin() + static_cast<std::ptrdiff_t>(srcBase),
                      tp.pixels.begin() +
                          static_cast<std::ptrdiff_t>(srcBase +
                                                      static_cast<size_t>(w)),
                      out.begin() + static_cast<std::ptrdiff_t>(dstBase));
        }
    }
    return true;
}

void PsxVramAtlas::UVtoNormalized(uint8_t u, uint8_t v, float& out_u, float& out_v) {
    out_u = ((float)u + 0.5f) / 256.0f;
    out_v = ((float)v + 0.5f) / 256.0f;
}

bool PsxVramAtlas::DumpTpagePixels(uint16_t tpage, D3D11Renderer* renderer) const {
    tpage = NormalizeTpageTextureKey(tpage);
    auto it = m_tpages.find(tpage);
    if (it == m_tpages.end()) return false;
    const TpageTexture& tp = it->second;
    if (tp.pixels.empty() || !renderer) return false;

    // Save to logs dir
    wchar_t path[512];
    swprintf(path, 512, L"logs/tpage_0x%04X_dump.png", tpage);
    bool ok = renderer->SaveRgbaPng(path, tp.pixels.data(), TPAGE_W, TPAGE_H);
    Log::Printf("DumpTpagePixels: tpage=0x%04X saved=%d dirty=%d", tpage, ok ? 1 : 0, tp.dirty ? 1 : 0);
    return ok;
}

void PsxVramAtlas::FillTpageColor(uint16_t tpage, uint32_t rgba) {
    tpage = NormalizeTpageTextureKey(tpage);
    auto it = m_tpages.find(tpage);
    if (it == m_tpages.end()) return;
    TpageTexture& tp = it->second;
    DestroyClutSrvs(tp);
    std::fill(tp.pixels.begin(), tp.pixels.end(), rgba);
    std::fill(tp.indexedPixels.begin(), tp.indexedPixels.end(), 0u);
    tp.dirty = true;
}
