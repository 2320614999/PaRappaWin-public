#include "pr_vram_atlas.h"
#include "logger.h"
#include <cstring>
#include <algorithm>
#include <limits>
#include <unordered_set>
#include <utility>

PsxVramAtlas::PsxVramAtlas() {}

PsxVramAtlas::PsxVramAtlas(PsxVramAtlas&& other) noexcept {
    MoveFrom(std::move(other));
}

PsxVramAtlas& PsxVramAtlas::operator=(PsxVramAtlas&& other) noexcept {
    if (this != &other) {
        Clear();
        MoveFrom(std::move(other));
    }
    return *this;
}

namespace {

uint16_t NormalizeTpageTextureKey(uint16_t tpage) {
    // PSX TSB bits 5-6 are ABR blend mode, not VRAM page address.
    return (uint16_t)((tpage & 0x01FFu) & ~0x0060u);
}

struct ParsedTimLoad {
    uint32_t pmode = 0;
    const uint16_t* clutData = nullptr;
    int clutSize = 0;
    uint16_t clutX = 0;
    uint16_t clutY = 0;
    uint16_t clutW = 0;
    uint16_t clutH = 0;
    const uint8_t* pixelData = nullptr;
    size_t pixelBytes = 0;
    uint16_t imgX = 0;
    uint16_t imgY = 0;
    uint16_t imgW = 0;
    uint16_t imgH = 0;
    uint16_t tpage = 0;
};

uint32_t ReadTimU32(const uint8_t* data) {
    uint32_t value = 0;
    std::memcpy(&value, data, sizeof(value));
    return value;
}

uint16_t ReadTimU16(const uint8_t* data) {
    uint16_t value = 0;
    std::memcpy(&value, data, sizeof(value));
    return value;
}

bool TimBlockFits(size_t offset, uint32_t blockLength, size_t size) {
    constexpr size_t kBlockHeaderSize = 12u;
    return offset <= size && blockLength >= kBlockHeaderSize &&
           static_cast<size_t>(blockLength) <= size - offset;
}

bool TimPayloadMatchesDimensions(uint16_t width,
                                 uint16_t height,
                                 size_t payloadBytes) {
    if (width == 0u || height == 0u) {
        return false;
    }

    const size_t elementCount = static_cast<size_t>(width) *
                                static_cast<size_t>(height);
    if (elementCount > (std::numeric_limits<size_t>::max)() / 2u) {
        return false;
    }
    return elementCount * 2u == payloadBytes;
}

bool ParseTimLoad(const uint8_t* raw, size_t size, ParsedTimLoad& out) {
    constexpr size_t kFileHeaderSize = 8u;
    constexpr size_t kBlockHeaderSize = 12u;

    out = ParsedTimLoad{};
    if (raw == nullptr || size < 20u) {
        return false;
    }

    const uint32_t magic = ReadTimU32(raw);
    const uint32_t flags = ReadTimU32(raw + 4u);
    if (magic != 0x10u) {
        return false;
    }

    out.pmode = flags & 7u;
    const bool hasClut = (flags & 8u) != 0u;
    if (!hasClut || (out.pmode != 0u && out.pmode != 1u)) {
        return false;
    }

    size_t offset = kFileHeaderSize;
    if (size - offset < kBlockHeaderSize) {
        return false;
    }

    const uint32_t clutLength = ReadTimU32(raw + offset);
    if (!TimBlockFits(offset, clutLength, size)) {
        return false;
    }

    out.clutX = ReadTimU16(raw + offset + 4u);
    out.clutY = ReadTimU16(raw + offset + 6u);
    out.clutW = ReadTimU16(raw + offset + 8u);
    out.clutH = ReadTimU16(raw + offset + 10u);
    if (out.clutX >= 1024u || out.clutY >= 512u) {
        return false;
    }
    const size_t clutPayloadBytes =
        static_cast<size_t>(clutLength) - kBlockHeaderSize;
    if (!TimPayloadMatchesDimensions(
            out.clutW, out.clutH, clutPayloadBytes)) {
        return false;
    }

    const size_t clutColorCount = static_cast<size_t>(out.clutW) *
                                  static_cast<size_t>(out.clutH);
    if (clutColorCount >
        static_cast<size_t>((std::numeric_limits<int>::max)())) {
        return false;
    }
    out.clutData = reinterpret_cast<const uint16_t*>(
        raw + offset + kBlockHeaderSize);
    out.clutSize = static_cast<int>(clutColorCount);
    offset += static_cast<size_t>(clutLength);

    if (offset > size || size - offset < kBlockHeaderSize) {
        return false;
    }

    const uint32_t imageLength = ReadTimU32(raw + offset);
    if (!TimBlockFits(offset, imageLength, size)) {
        return false;
    }

    out.imgX = ReadTimU16(raw + offset + 4u);
    out.imgY = ReadTimU16(raw + offset + 6u);
    out.imgW = ReadTimU16(raw + offset + 8u);
    out.imgH = ReadTimU16(raw + offset + 10u);
    if (out.imgX >= 1024u || out.imgY >= 512u) {
        return false;
    }
    out.pixelBytes = static_cast<size_t>(imageLength) - kBlockHeaderSize;
    if (!TimPayloadMatchesDimensions(out.imgW, out.imgH, out.pixelBytes)) {
        return false;
    }
    out.pixelData = raw + offset + kBlockHeaderSize;

    const int tpageIdx = static_cast<int>(out.imgX) / 64;
    const int tpageY = (out.imgY >= 256u) ? 1 : 0;
    out.tpage = NormalizeTpageTextureKey(static_cast<uint16_t>(
        tpageIdx | (tpageY << 4) | (static_cast<int>(out.pmode) << 7)));
    return true;
}

}  // namespace

PsxVramAtlas::~PsxVramAtlas() {
    Clear();
}

void PsxVramAtlas::Clear() {
    DestroyStandaloneTimViews();
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
    m_standaloneTimImages.clear();
    m_standaloneTimNames.clear();
    m_standaloneTimRenderer = nullptr;
    m_loadedCount = 0;
}

void PsxVramAtlas::MoveFrom(PsxVramAtlas&& other) noexcept {
    m_tpages = std::move(other.m_tpages);
    m_cluts = std::move(other.m_cluts);
    m_requiredCluts = std::move(other.m_requiredCluts);
    m_standaloneTimImages = std::move(other.m_standaloneTimImages);
    m_standaloneTimNames = std::move(other.m_standaloneTimNames);
    m_standaloneTimViews = std::move(other.m_standaloneTimViews);
    m_standaloneTimRenderer = other.m_standaloneTimRenderer;
    m_renderer = other.m_renderer;
    m_loadedCount = other.m_loadedCount;
    other.m_tpages.clear();
    other.m_cluts.clear();
    other.m_requiredCluts.clear();
    other.m_standaloneTimImages.clear();
    other.m_standaloneTimNames.clear();
    other.m_standaloneTimViews.clear();
    other.m_standaloneTimRenderer = nullptr;
    other.m_renderer = nullptr;
    other.m_loadedCount = 0;
}

void PsxVramAtlas::DestroyStandaloneTimViews() {
    for (auto& view : m_standaloneTimViews) {
        if (view.srv != nullptr && view.renderer != nullptr) {
            view.renderer->DestroyTexture(view.srv);
        }
        view.srv = nullptr;
        view.renderer = nullptr;
    }
    m_standaloneTimViews.clear();
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
        tp.psxAbr0StpClutSrvs.clear();
        tp.psxAbr1StpClutSrvs.clear();
        return;
    }
    for (auto& [clut, srv] : tp.clutSrvs) {
        if (srv) {
            m_renderer->DestroyTexture(srv);
        }
    }
    tp.clutSrvs.clear();
    for (auto& [clut, srv] : tp.psxAbr0StpClutSrvs) {
        if (srv) {
            m_renderer->DestroyTexture(srv);
        }
    }
    tp.psxAbr0StpClutSrvs.clear();
    for (auto& [clut, srv] : tp.psxAbr1StpClutSrvs) {
        if (srv) {
            m_renderer->DestroyTexture(srv);
        }
    }
    tp.psxAbr1StpClutSrvs.clear();
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

bool PsxVramAtlas::CanLoadTimTarget(
    uint16_t tpage,
    uint16_t clutX,
    uint16_t clutY,
    uint16_t clutH,
    bool filterToRequiredClutRow) const {
    if (!filterToRequiredClutRow) {
        return true;
    }

    tpage = NormalizeTpageTextureKey(tpage);
    if (m_tpages.find(tpage) == m_tpages.end()) {
        return false;
    }
    return TimHasRequiredClutRow(
        static_cast<int>(clutX),
        static_cast<int>(clutY),
        static_cast<int>(clutH));
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

    size_t srcIdx = 0;
    for (int y = 0; y < imgH; y++) {
        int ty = pixOfsY + y;
        if (ty < 0 || ty >= TPAGE_H) {
            srcIdx += static_cast<size_t>(imgW_hw) * 2u;
            continue;
        }
        for (int xw = 0; xw < imgW_hw; xw++) {
            if (srcIdx + 1u >= pixelBytes) break;
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

    size_t srcIdx = 0;
    for (int y = 0; y < imgH; y++) {
        int ty = pixOfsY + y;
        if (ty < 0 || ty >= TPAGE_H) {
            srcIdx += static_cast<size_t>(imgW_hw) * 2u;
            continue;
        }
        for (int xw = 0; xw < imgW_hw; xw++) {
            if (srcIdx + 1u >= pixelBytes) break;
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

bool PsxVramAtlas::CanLoadTim(const uint8_t* raw,
                              size_t size,
                              const std::string& name,
                              bool uploadClut,
                              bool filterToRequiredClutRow) const {
    (void)name;
    (void)uploadClut;

    ParsedTimLoad tim;
    return ParseTimLoad(raw, size, tim) &&
           CanLoadTimTarget(tim.tpage,
                            tim.clutX,
                            tim.clutY,
                            tim.clutH,
                            filterToRequiredClutRow);
}

bool PsxVramAtlas::CanResolveTpageClut(uint16_t tpage,
                                       uint16_t clut) const {
    tpage = NormalizeTpageTextureKey(tpage);
    const auto tpageIt = m_tpages.find(tpage);
    const auto clutIt = m_cluts.find(clut);
    if (tpageIt == m_tpages.end() || clutIt == m_cluts.end()) {
        return false;
    }
    const int colorMode = tpageIt->second.colorMode;
    const std::size_t requiredColors =
        colorMode == 0 ? 16u : (colorMode == 1 ? 256u : 0u);
    return requiredColors != 0u &&
           clutIt->second.size() >= requiredColors;
}

bool PsxVramAtlas::LoadTim(const uint8_t* raw,
                            size_t size,
                            const std::string& name,
                            bool uploadClut,
                            bool filterToRequiredClutRow) {
    ParsedTimLoad tim;
    if (!ParseTimLoad(raw, size, tim)) {
        return false;
    }

    if (!CanLoadTimTarget(tim.tpage,
                          tim.clutX,
                          tim.clutY,
                          tim.clutH,
                          filterToRequiredClutRow)) {
        if (filterToRequiredClutRow &&
            m_tpages.find(tim.tpage) == m_tpages.end()) {
            static std::unordered_set<uint16_t> s_loggedMissing;
            if (s_loggedMissing.insert(tim.tpage).second) {
                Log::Printf(
                    "LoadTim '%s': skipped - no matching tpage (tpage=0x%04X)",
                    name.c_str(),
                    tim.tpage);
            }
        }
        return false;
    }

    // Retain the decoded upload independently of the shared page.  Several
    // native Scene0 menu TIMs intentionally overlap in VRAM; the PSX loader
    // draws each upload before the next one overwrites that rectangle.
    TimImage standaloneImage{};
    const bool standaloneDecoded =
        TimDecoder::Decode(raw, size, standaloneImage) &&
        (standaloneImage.bpp == 4u || standaloneImage.bpp == 8u);

    auto targetIt = m_tpages.find(tim.tpage);
    TpageTexture* target = targetIt != m_tpages.end()
                               ? &targetIt->second
                               : &GetOrCreateTpage(tim.tpage);

    if (uploadClut) {
        RegisterClutRows(tim.clutData,
                         static_cast<int>(tim.clutW),
                         static_cast<int>(tim.clutH),
                         static_cast<int>(tim.clutX),
                         static_cast<int>(tim.clutY));
    }

    if (tim.pmode == 0u) {
        DecodeTim4bit(*target,
                      tim.pixelData,
                      tim.pixelBytes,
                      static_cast<int>(tim.imgX),
                      static_cast<int>(tim.imgY),
                      static_cast<int>(tim.imgW),
                      static_cast<int>(tim.imgH),
                      tim.clutData,
                      tim.clutSize);
    } else {
        DecodeTim8bit(*target,
                      tim.pixelData,
                      tim.pixelBytes,
                      static_cast<int>(tim.imgX),
                      static_cast<int>(tim.imgY),
                      static_cast<int>(tim.imgW),
                      static_cast<int>(tim.imgH),
                      tim.clutData,
                      tim.clutSize);
    }

    if (standaloneDecoded) {
        m_standaloneTimImages.push_back(std::move(standaloneImage));
        m_standaloneTimNames.push_back(name);
    }

    m_loadedCount++;
    return true;
}

bool PsxVramAtlas::ApplyPartialVramWords(const uint16_t* words,
                                         const uint8_t* known,
                                         size_t wordCount) {
    constexpr size_t kVramWordCount = static_cast<size_t>(1024u) * 512u;
    if (words == nullptr || known == nullptr || wordCount != kVramWordCount) {
        return false;
    }

    // LoadImage writes are the final authority for the scene's partial VRAM
    // projection.  Decode only known words so untouched portions keep the
    // TIM-replayed source image that seeded this atlas.
    for (auto& [key, tp] : m_tpages) {
        (void)key;
        const int wordsPerRow = tp.colorMode == 0
                                    ? 64
                                    : (tp.colorMode == 1 ? 128 : 256);
        const int pixelsPerWord = tp.colorMode == 0
                                      ? 4
                                      : (tp.colorMode == 1 ? 2 : 1);
        bool pageChanged = false;
        for (int y = 0; y < TPAGE_H; ++y) {
            const int globalY = tp.baseY + y;
            if (globalY < 0 || globalY >= 512) {
                continue;
            }
            for (int wordX = 0; wordX < wordsPerRow; ++wordX) {
                const int globalX = tp.baseHW + wordX;
                if (globalX < 0 || globalX >= 1024) {
                    continue;
                }
                const size_t vramIndex =
                    static_cast<size_t>(globalY) * 1024u +
                    static_cast<size_t>(globalX);
                if (known[vramIndex] == 0u) {
                    continue;
                }

                const uint16_t value = words[vramIndex];
                const int pixelX = wordX * pixelsPerWord;
                if (tp.colorMode == 0) {
                    for (int pixel = 0; pixel < 4; ++pixel) {
                        const uint8_t index = static_cast<uint8_t>(
                            (value >> (pixel * 4)) & 0x0Fu);
                        tp.indexedPixels[static_cast<size_t>(y) * TPAGE_W +
                                           static_cast<size_t>(pixelX + pixel)] =
                            index;
                    }
                } else if (tp.colorMode == 1) {
                    tp.indexedPixels[static_cast<size_t>(y) * TPAGE_W +
                                       static_cast<size_t>(pixelX)] =
                        static_cast<uint8_t>(value & 0xFFu);
                    tp.indexedPixels[static_cast<size_t>(y) * TPAGE_W +
                                       static_cast<size_t>(pixelX + 1)] =
                        static_cast<uint8_t>((value >> 8) & 0xFFu);
                } else if (tp.colorMode == 2) {
                    tp.pixels[static_cast<size_t>(y) * TPAGE_W +
                               static_cast<size_t>(pixelX)] =
                        TimDecoder::ConvertABGR1555toRGBA8888(value);
                }
                pageChanged = true;
            }
        }

        if (pageChanged) {
            tp.dirty = true;
        }
    }

    // A CLUT key stores the PSX row (Y) and the 16-word aligned X position.
    // Keep the original row width from TIM registration, but replace entries
    // only where the native partial write actually touched VRAM.
    for (auto& [clut, colors] : m_cluts) {
        const int clutX = static_cast<int>(clut & 0x3Fu) * 16;
        const int clutY = static_cast<int>((clut >> 6) & 0x01FFu);
        bool clutChanged = false;
        for (size_t index = 0; index < colors.size(); ++index) {
            const int globalX = clutX + static_cast<int>(index);
            if (globalX < 0 || globalX >= 1024) {
                break;
            }
            const size_t vramIndex = static_cast<size_t>(clutY) * 1024u +
                                     static_cast<size_t>(globalX);
            if (known[vramIndex] != 0u) {
                colors[index] = words[vramIndex];
                clutChanged = true;
            }
        }
        (void)clutChanged;
    }

    // Palette-specific SRVs cache the old rows and must be rebuilt after any
    // raw VRAM overlay.  The base tpage SRV is marked dirty above when its
    // indexed payload changed.
    for (auto& [key, tp] : m_tpages) {
        (void)key;
        DestroyClutSrvs(tp);
    }
    return true;
}

PsxVramAtlasUploadResult PsxVramAtlas::UploadAll(D3D11Renderer* renderer) {
    PsxVramAtlasUploadResult result{};
    result.rendererKnown = renderer != nullptr;
    result.tpageCount = static_cast<uint32_t>(m_tpages.size());
    if (!renderer) return result;
    m_renderer = renderer;

    for (auto& [k, tp] : m_tpages) {
        if (tp.dirty) {
            ++result.dirtyTpageCount;
            bool uploaded = false;
            if (tp.srv) {
                uploaded = renderer->TryUpdateTexture(
                    tp.srv, tp.pixels.data(), TPAGE_W, TPAGE_H);
                if (uploaded) {
                    ++result.updatedTpageCount;
                }
            } else {
                tp.srv = renderer->CreateTexture(
                    tp.pixels.data(), TPAGE_W, TPAGE_H);
                uploaded = tp.srv != nullptr;
                if (uploaded) {
                    if (result.updatedTpageCount == 0u &&
                        result.createdTpageCount == 0u) {
                        Log::Printf("VramAtlas: uploaded tpage 0x%04X (mode=%d, base_hw=%d, baseY=%d)",
                                    tp.tpage, tp.colorMode, tp.baseHW, tp.baseY);
                    }
                    ++result.createdTpageCount;
                }
            }
            if (uploaded) {
                tp.dirty = false;
            } else {
                ++result.failedTpageCount;
            }
        }
        if (tp.srv != nullptr) {
            ++result.readyTpageCount;
        }
    }
    if (result.createdTpageCount > 0u) {
        Log::Printf("VramAtlas: recreated %u tpage texture(s)",
                    result.createdTpageCount);
    }
    if (result.failedTpageCount > 0u) {
        Log::Printf("VramAtlas: %u tpage upload(s) failed and remain dirty",
                    result.failedTpageCount);
    }
    result.complete = result.tpageCount > 0u &&
                      result.failedTpageCount == 0u &&
                      result.readyTpageCount == result.tpageCount;
    return result;
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

ID3D11ShaderResourceView* PsxVramAtlas::GetStandaloneTimSRV(
    uint16_t orgX,
    uint16_t orgY,
    uint16_t width,
    uint16_t height,
    uint16_t clutX,
    uint16_t clutY,
    D3D11Renderer* renderer) {
    if (renderer == nullptr || width == 0u || height == 0u) {
        return nullptr;
    }

    if (m_standaloneTimRenderer != renderer) {
        DestroyStandaloneTimViews();
        m_standaloneTimRenderer = renderer;
    }

    // A later TIM upload overwrites the same PSX VRAM rectangle.  Resolve
    // standalone views newest-first so a title re-entry's neutral face upload
    // replaces the expression that was uploaded by the previous title loop.
    for (std::size_t reverseIndex = m_standaloneTimImages.size();
         reverseIndex != 0u; --reverseIndex) {
        const std::size_t index = reverseIndex - 1u;
        const TimImage& image = m_standaloneTimImages[index];
        // Practice 80023618 uses the 8bpp G_REN_30/G_REN_40 character
        // panels, while the menu plates and small icons are 4bpp.  Both are
        // valid standalone TIM uploads; rejecting 8bpp here forces those
        // panels through the overwritten shared page and produces the large
        // striped/corrupted judge-side images.
        if ((image.bpp != 4u && image.bpp != 8u) ||
            image.orgX != static_cast<int16_t>(orgX) ||
            image.orgY != static_cast<int16_t>(orgY) ||
            image.width != width || image.height != height ||
            image.clutX != static_cast<int16_t>(clutX) ||
            clutY < static_cast<uint16_t>(image.clutY) ||
            clutY >= static_cast<uint16_t>(image.clutY) + image.clutH) {
            continue;
        }

        const int paletteRow =
            static_cast<int>(clutY - static_cast<uint16_t>(image.clutY));
        for (const auto& view : m_standaloneTimViews) {
            if (view.imageIndex == index && view.paletteRow == paletteRow &&
                view.renderer == renderer) {
                return view.srv;
            }
        }

        TimImage paletteImage = image;
        TimDecoder::ApplyPalette(paletteImage, paletteRow);
        if (paletteImage.rgba.empty()) {
            return nullptr;
        }
        ID3D11ShaderResourceView* srv = renderer->CreateTexture(
            paletteImage.rgba.data(), static_cast<int>(paletteImage.width),
            static_cast<int>(paletteImage.height));
        if (srv == nullptr) {
            return nullptr;
        }
        m_standaloneTimViews.push_back(
            {index, paletteRow, srv, renderer});
        const char* name = index < m_standaloneTimNames.size()
                               ? m_standaloneTimNames[index].c_str()
                               : "";
        Log::Printf(
            "VramAtlas: standalone TIM name=%s org=(%u,%u) size=%ux%u clut=(%u,%u) row=%d",
            name, static_cast<unsigned>(orgX), static_cast<unsigned>(orgY),
            static_cast<unsigned>(width), static_cast<unsigned>(height),
            static_cast<unsigned>(clutX), static_cast<unsigned>(clutY),
            paletteRow);
        return srv;
    }

    // Some native Scene0 descriptors intentionally reuse the pixel rectangle
    // of a STAG_* upload with a CLUT row supplied by a different upload.  The
    // unlocked stage label (80051B00) is the concrete case: its descriptor
    // asks for row 31 while STAG_SE1 carries the same pixels at row 34 and
    // STAG_1M1 supplies the row-31 palette.  The final shared tpage contains
    // whichever STAG_* image was uploaded last, so falling back to that page
    // produces the wrong digit/label.  Recombine the newest matching TIM's
    // indexed pixels with the requested VRAM CLUT instead, preserving the
    // PSX upload semantics without changing the pseudo-C descriptor.
    const uint16_t requestedClut = static_cast<uint16_t>(
        (((static_cast<uint32_t>(clutY) & 0x01FFu) << 6u) |
         ((static_cast<uint32_t>(clutX) >> 4u) & 0x3Fu)));
    const auto clutIt = m_cluts.find(requestedClut);
    if (clutIt != m_cluts.end() && !clutIt->second.empty()) {
        for (std::size_t reverseIndex = m_standaloneTimImages.size();
             reverseIndex != 0u; --reverseIndex) {
            const std::size_t index = reverseIndex - 1u;
            const TimImage& image = m_standaloneTimImages[index];
            if ((image.bpp != 4u && image.bpp != 8u) ||
                image.orgX != static_cast<int16_t>(orgX) ||
                image.orgY != static_cast<int16_t>(orgY) ||
                image.width != width || image.height != height ||
                image.clutX != static_cast<int16_t>(clutX)) {
                continue;
            }

            constexpr int kSynthesizedPaletteRow = 0x10000;
            const int synthesizedPaletteRow =
                kSynthesizedPaletteRow + static_cast<int>(clutY);
            for (const auto& view : m_standaloneTimViews) {
                if (view.imageIndex == index &&
                    view.paletteRow == synthesizedPaletteRow &&
                    view.renderer == renderer) {
                    return view.srv;
                }
            }

            TimImage recombined = image;
            recombined.clutX = static_cast<int16_t>(clutX);
            recombined.clutY = static_cast<int16_t>(clutY);
            recombined.clutW = static_cast<uint16_t>(clutIt->second.size());
            recombined.clutH = 1u;
            recombined.palette = clutIt->second;
            TimDecoder::ApplyPalette(recombined, 0);
            if (recombined.rgba.empty()) {
                continue;
            }

            ID3D11ShaderResourceView* srv = renderer->CreateTexture(
                recombined.rgba.data(), static_cast<int>(recombined.width),
                static_cast<int>(recombined.height));
            if (srv == nullptr) {
                return nullptr;
            }
            m_standaloneTimViews.push_back(
                {index, synthesizedPaletteRow, srv, renderer});
            const char* name = index < m_standaloneTimNames.size()
                                   ? m_standaloneTimNames[index].c_str()
                                   : "";
            Log::Printf(
                "VramAtlas: standalone TIM recombined name=%s org=(%u,%u) size=%ux%u clut=(%u,%u) from requested VRAM row",
                name, static_cast<unsigned>(orgX),
                static_cast<unsigned>(orgY), static_cast<unsigned>(width),
                static_cast<unsigned>(height), static_cast<unsigned>(clutX),
                static_cast<unsigned>(clutY));
            return srv;
        }
    }
    return nullptr;
}

ID3D11ShaderResourceView* PsxVramAtlas::BuildPsxAbr0StpClutSrv(
    TpageTexture& tp,
    uint16_t clut,
    D3D11Renderer* renderer) {
    const size_t pixelCount = (size_t)TPAGE_W * (size_t)TPAGE_H;
    if (!renderer || tp.colorMode > 1 ||
        tp.indexedPixels.size() != pixelCount) {
        return nullptr;
    }

    const auto clutIt = m_cluts.find(clut);
    const size_t requiredColors = tp.colorMode == 0 ? 16u : 256u;
    if (clutIt == m_cluts.end() ||
        clutIt->second.size() < requiredColors) {
        return nullptr;
    }

    std::vector<uint32_t> pixels(pixelCount, 0u);
    const std::vector<uint16_t>& colors = clutIt->second;
    for (size_t i = 0; i < pixels.size(); ++i) {
        const uint8_t idx = tp.indexedPixels[i];
        pixels[i] =
            TimDecoder::ConvertABGR1555toPsxAbr0StpRGBA8888(colors[idx]);
    }

    return renderer->CreateTexture(pixels.data(), TPAGE_W, TPAGE_H);
}

ID3D11ShaderResourceView* PsxVramAtlas::GetTpagePsxAbr0StpSRV(
    uint16_t tpage,
    uint16_t clut,
    D3D11Renderer* renderer) {
    if (!renderer) {
        return nullptr;
    }

    tpage = NormalizeTpageTextureKey(tpage);
    auto it = m_tpages.find(tpage);
    if (it == m_tpages.end()) {
        return nullptr;
    }

    TpageTexture& tp = it->second;
    if (tp.colorMode > 1 || m_cluts.find(clut) == m_cluts.end()) {
        return nullptr;
    }

    auto srvIt = tp.psxAbr0StpClutSrvs.find(clut);
    if (srvIt != tp.psxAbr0StpClutSrvs.end()) {
        return srvIt->second;
    }

    ID3D11ShaderResourceView* srv =
        BuildPsxAbr0StpClutSrv(tp, clut, renderer);
    if (!srv) {
        return nullptr;
    }

    m_renderer = renderer;
    tp.psxAbr0StpClutSrvs[clut] = srv;
    return srv;
}

ID3D11ShaderResourceView* PsxVramAtlas::BuildPsxAbr1StpClutSrv(
    TpageTexture& tp,
    uint16_t clut,
    D3D11Renderer* renderer) {
    const size_t pixelCount = (size_t)TPAGE_W * (size_t)TPAGE_H;
    if (!renderer || tp.colorMode > 1 ||
        tp.indexedPixels.size() != pixelCount) {
        return nullptr;
    }

    const auto clutIt = m_cluts.find(clut);
    const size_t requiredColors = tp.colorMode == 0 ? 16u : 256u;
    if (clutIt == m_cluts.end() ||
        clutIt->second.size() < requiredColors) {
        return nullptr;
    }

    std::vector<uint32_t> pixels(pixelCount, 0u);
    const std::vector<uint16_t>& colors = clutIt->second;
    for (size_t i = 0; i < pixels.size(); ++i) {
        const uint8_t idx = tp.indexedPixels[i];
        pixels[i] =
            TimDecoder::ConvertABGR1555toPsxAbr1StpRGBA8888(colors[idx]);
    }

    return renderer->CreateTexture(pixels.data(), TPAGE_W, TPAGE_H);
}

ID3D11ShaderResourceView* PsxVramAtlas::GetTpagePsxAbr1StpSRV(
    uint16_t tpage,
    uint16_t clut,
    D3D11Renderer* renderer) {
    if (!renderer) {
        return nullptr;
    }

    tpage = NormalizeTpageTextureKey(tpage);
    auto it = m_tpages.find(tpage);
    if (it == m_tpages.end()) {
        return nullptr;
    }

    TpageTexture& tp = it->second;
    if (tp.colorMode > 1 || m_cluts.find(clut) == m_cluts.end()) {
        return nullptr;
    }

    auto srvIt = tp.psxAbr1StpClutSrvs.find(clut);
    if (srvIt != tp.psxAbr1StpClutSrvs.end()) {
        return srvIt->second;
    }

    ID3D11ShaderResourceView* srv =
        BuildPsxAbr1StpClutSrv(tp, clut, renderer);
    if (!srv) {
        return nullptr;
    }

    m_renderer = renderer;
    tp.psxAbr1StpClutSrvs[clut] = srv;
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
