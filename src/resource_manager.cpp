#include "resource_manager.h"
#include <algorithm>
#include <cstdio>
#include <cstring>

static uint32_t ReadU32LE_RM(const uint8_t* p) {
    uint32_t v = 0;
    std::memcpy(&v, p, sizeof(v));
    return v;
}

static void ApplyIndex0Transparency(TimImage& tim) {
    if ((tim.bpp != 4 && tim.bpp != 8) || tim.width == 0 || tim.height == 0) {
        return;
    }
    if (tim.rgba.size() != (size_t)tim.width * (size_t)tim.height) {
        return;
    }
    const bool hasPsxAbr1Stp =
        tim.rgbaPsxAbr1Stp.size() ==
        (size_t)tim.width * (size_t)tim.height;
    if (tim.bpp == 8) {
        const size_t n = tim.rgba.size();
        for (size_t i = 0; i < n && i < tim.pixels.size(); i++) {
            if (tim.pixels[i] == 0) {
                tim.rgba[i] = 0u;
                if (hasPsxAbr1Stp) {
                    tim.rgbaPsxAbr1Stp[i] = 0u;
                }
            }
        }
        return;
    }

    for (uint32_t y = 0; y < tim.height; y++) {
        for (uint32_t x = 0; x < tim.width; x++) {
            const size_t pix = (size_t)y * (size_t)tim.width + (size_t)x;
            const size_t byteIdx = pix / 2u;
            if (byteIdx >= tim.pixels.size()) {
                continue;
            }
            const uint8_t byte = tim.pixels[byteIdx];
            const uint8_t idx = ((x & 1u) == 0u) ? (byte & 0x0Fu) : ((byte >> 4) & 0x0Fu);
            if (idx == 0u) {
                tim.rgba[pix] = 0u;
                if (hasPsxAbr1Stp) {
                    tim.rgbaPsxAbr1Stp[pix] = 0u;
                }
            }
        }
    }
}

static bool CropRgbaPlane(const std::vector<uint32_t>& source,
                          uint32_t sourceWidth,
                          uint32_t sourceHeight,
                          int sourceX,
                          int sourceY,
                          uint32_t width,
                          uint32_t height,
                          std::vector<uint32_t>& out) {
    if (source.size() !=
            (size_t)sourceWidth * (size_t)sourceHeight ||
        sourceX < 0 || sourceY < 0 ||
        (uint32_t)sourceX + width > sourceWidth ||
        (uint32_t)sourceY + height > sourceHeight) {
        return false;
    }
    out.resize((size_t)width * (size_t)height);
    for (uint32_t y = 0; y < height; ++y) {
        const size_t sourceRow =
            (size_t)(sourceY + (int)y) * (size_t)sourceWidth;
        const size_t destinationRow = (size_t)y * (size_t)width;
        const uint32_t* row =
            source.data() + sourceRow + (size_t)sourceX;
        std::copy(row, row + (size_t)width, out.data() + destinationRow);
    }
    return true;
}

static void DestroyTextureViews(D3D11Renderer* renderer,
                                TextureResource* texture) {
    if (!renderer || !texture) {
        return;
    }
    if (texture->srv) {
        renderer->DestroyTexture(texture->srv);
        texture->srv = nullptr;
    }
    if (texture->srvPsxAbr1Stp) {
        renderer->DestroyTexture(texture->srvPsxAbr1Stp);
        texture->srvPsxAbr1Stp = nullptr;
    }
}

static bool ShouldApplyIndex0Transparency(std::string keyLowerNoExt) {
    std::transform(keyLowerNoExt.begin(), keyLowerNoExt.end(), keyLowerNoExt.begin(), ::tolower);
    if (keyLowerNoExt == "g_frm18" || keyLowerNoExt == "g_frm19" ||
        keyLowerNoExt == "s_frm01" || keyLowerNoExt == "s_frm02" || keyLowerNoExt == "s_frm03" || keyLowerNoExt == "s_frm04" ||
        keyLowerNoExt == "s_frm31" || keyLowerNoExt == "s_frm32" || keyLowerNoExt == "s_frm33" || keyLowerNoExt == "s_frm34") {
        return true;
    }

    if (keyLowerNoExt == "frm_l_01" || keyLowerNoExt == "frm_r_01") {
        return true;
    }
    if (keyLowerNoExt.rfind("frm_un", 0) == 0 || keyLowerNoExt.rfind("frm_ob", 0) == 0) {
        return true;
    }
    if (keyLowerNoExt.rfind("g_st_", 0) == 0 || keyLowerNoExt.rfind("stag_", 0) == 0) {
        return true;
    }
    return false;
}

static int TextureKeyPriority(const std::string& key) {
    if (key.rfind("comod:", 0) == 0) return 0;
    if (key.rfind("ycompo:", 0) == 0) return 1;
    return 2;
}

static std::string StripTextureKeyPrefix(const std::string& key) {
    std::string baseKey = key;
    const size_t colonPos = baseKey.rfind(':');
    if (colonPos != std::string::npos) {
        baseKey = baseKey.substr(colonPos + 1);
    }
    return baseKey;
}

ResourceManager::ResourceManager() {}

ResourceManager::~ResourceManager() {
    Clear();
}

bool ResourceManager::LoadIntArchiveTimOnly(const std::string& path, const std::string& keyPrefix) {
    IntArchive archive;
    if (!IntLoader::Load(path, archive)) {
        return false;
    }

    ++m_generation;

    bool any = false;
    for (const auto& entry : archive.entries) {
        bool isTim = (entry.type == IntBlockType::Tim);
        bool isMemTim = false;
        if (!isTim && entry.type == IntBlockType::Mem) {
            std::string n = entry.name;
            std::transform(n.begin(), n.end(), n.begin(), ::tolower);
            if (n.size() >= 4 && n.rfind(".tim") == (n.size() - 4)) {
                isMemTim = true;
            }
        }
        if (!isTim && !isMemTim) {
            continue;
        }

        // Key = lowercase basename (no extension)
        std::string baseKey = entry.name;
        size_t dotPos = baseKey.rfind('.');
        if (dotPos != std::string::npos) {
            baseKey = baseKey.substr(0, dotPos);
        }
        std::transform(baseKey.begin(), baseKey.end(), baseKey.begin(), ::tolower);

        std::string key = keyPrefix;
        key += baseKey;
        if (isMemTim) {
            if (m_textures.find(key) != m_textures.end()) {
                key += "~mem";
            }
        }

        // Store raw TIM bytes under the same key.
        if (m_timRaw.find(key) == m_timRaw.end()) {
            m_timRawOrder.push_back(key);
        }
        m_timRaw[key] = entry.data;

        auto res = std::make_unique<TextureResource>();
        res->name = entry.name;

        if (!TimDecoder::Decode(entry.data.data(), entry.data.size(), res->tim)) {
            continue;
        }

        int paletteRow = 0;
        if (baseKey == "s_frm01" || baseKey == "s_frm02" || baseKey == "s_frm03" || baseKey == "s_frm04" ||
            baseKey == "s_frm31" || baseKey == "s_frm32" || baseKey == "s_frm33" || baseKey == "s_frm34" ||
            baseKey == "frm_l_01" || baseKey == "frm_r_01") {
            paletteRow = TimDecoder::PickBestGrayscalePaletteRow(res->tim);
        }

        TimDecoder::ApplyPalette(res->tim, paletteRow);

        if (ShouldApplyIndex0Transparency(baseKey)) {
            ApplyIndex0Transparency(res->tim);
        }

        // If overriding an existing key, release its srv to avoid leaks.
        auto it = m_textures.find(key);
        if (it != m_textures.end()) {
            DestroyTextureViews(m_renderer, it->second.get());
        }
        m_textures[key] = std::move(res);
        any = true;
    }

    return any;
}

bool ResourceManager::LoadTimFromBytes(const uint8_t* data, size_t size, const std::string& keyPrefix) {
    if (!data || size < 16) {
        return false;
    }

    ++m_generation;

    bool any = false;
    size_t off = 0;
    while (off + 16 <= size) {
        // Embedded TIM blobs in COMOD/overlay data are word-aligned.
        // Scanning bytewise creates many overlapping false positives and is too slow.
        if ((off & 3u) != 0u) {
            ++off;
            continue;
        }
        const uint8_t* p = data + off;
        const uint32_t magic = ReadU32LE_RM(p);
        if (magic != 0x10u) {
            off += 4;
            continue;
        }

        const uint32_t flags = ReadU32LE_RM(p + 4);
        const uint32_t pmode = flags & 7u;
        const bool hasClut = ((flags >> 3) & 1u) != 0u;
        if (pmode > 3u) {
            off += 4;
            continue;
        }

        size_t len = 8;
        if (hasClut) {
            if (off + len + 4 > size) {
                off += 4;
                continue;
            }
            const uint32_t clutLen = ReadU32LE_RM(p + len);
            if (clutLen < 12u) {
                off += 4;
                continue;
            }
            if (off + len + (size_t)clutLen > size) {
                off += 4;
                continue;
            }
            len += (size_t)clutLen;
        }

        if (off + len + 4 > size) {
            off += 4;
            continue;
        }
        const uint32_t pixLen = ReadU32LE_RM(p + len);
        if (pixLen < 12u) {
            off += 4;
            continue;
        }
        if (off + len + (size_t)pixLen > size) {
            off += 4;
            continue;
        }
        len += (size_t)pixLen;

        TimImage tim;
        if (!TimDecoder::Decode(p, len, tim)) {
            off += 4;
            continue;
        }

        TimDecoder::ApplyPalette(tim, 0);

        char offKey[32];
        std::snprintf(offKey, sizeof(offKey), "off_%08X", (unsigned)off);
        std::string key = keyPrefix;
        key += offKey;
        std::transform(key.begin(), key.end(), key.begin(), ::tolower);

        auto it = m_textures.find(key);
        if (it != m_textures.end()) {
            DestroyTextureViews(m_renderer, it->second.get());
        }

        auto res = std::make_unique<TextureResource>();
        res->name = key;
        res->tim = std::move(tim);
        m_textures[key] = std::move(res);
        any = true;

        const size_t alignedLen = (len + 3u) & ~size_t(3u);
        off += alignedLen;
    }

    return any;
}

bool ResourceManager::LoadIntArchive(const std::string& path) {
    IntArchive archive;
    if (!IntLoader::Load(path, archive)) {
        return false;
    }
    ++m_generation;  // 资源变更，触发背景重选
    
    for (const auto& entry : archive.entries) {
        if (entry.type == IntBlockType::Tim) {
            // Store raw TIM bytes (for VRAM-atlas building that needs VRAM coords)
            {
                std::string key = entry.name;
                size_t dotPos = key.rfind('.');
                if (dotPos != std::string::npos) {
                    key = key.substr(0, dotPos);
                }
                std::transform(key.begin(), key.end(), key.begin(), ::tolower);
                if (m_timRaw.find(key) == m_timRaw.end()) {
                    m_timRawOrder.push_back(key);
                }
                m_timRaw[key] = entry.data;
            }

            auto res = std::make_unique<TextureResource>();
            res->name = entry.name;

            // Decode TIM data
            if (TimDecoder::Decode(entry.data.data(), entry.data.size(), res->tim)) {
                int paletteRow = 0;
                {
                    std::string key = entry.name;
                    size_t dotPos = key.rfind('.');
                    if (dotPos != std::string::npos) {
                        key = key.substr(0, dotPos);
                    }
                    std::transform(key.begin(), key.end(), key.begin(), ::tolower);
                    if (key == "s_frm01" || key == "s_frm02" || key == "s_frm03" || key == "s_frm04" ||
                        key == "s_frm31" || key == "s_frm32" || key == "s_frm33" || key == "s_frm34" ||
                        key == "frm_l_01" || key == "frm_r_01") {
                        paletteRow = TimDecoder::PickBestGrayscalePaletteRow(res->tim);
                    }
                }

                // Apply palette to get RGBA pixels
                TimDecoder::ApplyPalette(res->tim, paletteRow);

                {
                    std::string key = entry.name;
                    size_t dotPos = key.rfind('.');
                    if (dotPos != std::string::npos) {
                        key = key.substr(0, dotPos);
                    }
                    if (ShouldApplyIndex0Transparency(key)) {
                        ApplyIndex0Transparency(res->tim);
                    }
                }

                // Store by name (without .TIM extension)
                std::string key = entry.name;
                size_t dotPos = key.rfind('.');
                if (dotPos != std::string::npos) {
                    key = key.substr(0, dotPos);
                }
                // Convert to lowercase for case-insensitive lookup
                std::transform(key.begin(), key.end(), key.begin(), ::tolower);

                auto existing = m_textures.find(key);
                if (existing != m_textures.end()) {
                    DestroyTextureViews(m_renderer, existing->second.get());
                }
                m_textures[key] = std::move(res);
            }
            continue;
        }

        if (entry.type == IntBlockType::Mem) {
            {
                std::string key = entry.name;
                std::transform(key.begin(), key.end(), key.begin(), ::tolower);
                m_mem[key] = entry.data;
                m_memOrder.push_back(entry.name);
            }

            {
                std::string n = entry.name;
                std::transform(n.begin(), n.end(), n.begin(), ::tolower);
                const bool isTim = (n.size() >= 4 && n.rfind(".tim") == (n.size() - 4));
                if (isTim && !entry.data.empty()) {
                    TimImage tim;
                    if (TimDecoder::Decode(entry.data.data(), entry.data.size(), tim)) {
                        int paletteRow = 0;
                        {
                            std::string key = entry.name;
                            size_t dotPos = key.rfind('.');
                            if (dotPos != std::string::npos) {
                                key = key.substr(0, dotPos);
                            }
                            std::transform(key.begin(), key.end(), key.begin(), ::tolower);
                            if (key == "s_frm01" || key == "s_frm02" || key == "s_frm03" || key == "s_frm04" ||
                                key == "s_frm31" || key == "s_frm32" || key == "s_frm33" || key == "s_frm34" ||
                                key == "frm_l_01" || key == "frm_r_01") {
                                paletteRow = TimDecoder::PickBestGrayscalePaletteRow(tim);
                            }
                        }
                        TimDecoder::ApplyPalette(tim, paletteRow);

                        {
                            std::string key = entry.name;
                            size_t dotPos = key.rfind('.');
                            if (dotPos != std::string::npos) {
                                key = key.substr(0, dotPos);
                            }
                            if (ShouldApplyIndex0Transparency(key)) {
                                ApplyIndex0Transparency(tim);
                            }
                        }

                        std::string key = entry.name;
                        size_t dotPos = key.rfind('.');
                        if (dotPos != std::string::npos) {
                            key = key.substr(0, dotPos);
                        }
                        std::transform(key.begin(), key.end(), key.begin(), ::tolower);

                        if (m_textures.find(key) == m_textures.end()) {
                            auto res = std::make_unique<TextureResource>();
                            res->name = entry.name;
                            res->tim = std::move(tim);
                            m_textures[key] = std::move(res);
                        }
                        if (m_timRaw.find(key) == m_timRaw.end()) {
                            m_timRawOrder.push_back(key);
                        }
                        m_timRaw[key] = entry.data;
                    }
                }
            }
            continue;
        }
    }
    
    return !m_textures.empty() || !m_mem.empty();
}

TextureResource* ResourceManager::GetTexture(const std::string& name) {
    std::string key = name;
    std::transform(key.begin(), key.end(), key.begin(), ::tolower);
    
    auto it = m_textures.find(key);
    if (it != m_textures.end()) {
        return it->second.get();
    }
    return nullptr;
}

const std::vector<uint8_t>* ResourceManager::GetTimRaw(const std::string& name) const {
    std::string key = name;
    std::transform(key.begin(), key.end(), key.begin(), ::tolower);

    auto it = m_timRaw.find(key);
    if (it != m_timRaw.end()) {
        return &it->second;
    }
    return nullptr;
}

ID3D11ShaderResourceView* ResourceManager::GetTextureView(const std::string& name) {
    TextureResource* res = GetTexture(name);
    if (!res) return nullptr;
    return GetTextureView(*res);
}

ID3D11ShaderResourceView* ResourceManager::GetTextureView(
    TextureResource& texture) {
    // Lazy create D3D11 texture
    if (!texture.srv && m_renderer && !texture.tim.rgba.empty()) {
        texture.srv = m_renderer->CreateTexture(
            texture.tim.rgba.data(),
            texture.tim.width,
            texture.tim.height
        );
    }

    return texture.srv;
}

ID3D11ShaderResourceView* ResourceManager::GetTexturePsxAbr1StpView(
    TextureResource& texture) {
    if (!texture.srvPsxAbr1Stp && m_renderer &&
        !texture.tim.rgbaPsxAbr1Stp.empty()) {
        texture.srvPsxAbr1Stp = m_renderer->CreateTexture(
            texture.tim.rgbaPsxAbr1Stp.data(),
            texture.tim.width,
            texture.tim.height);
    }

    return texture.srvPsxAbr1Stp;
}

TextureResource* ResourceManager::FindTextureByTimHeader(int bpp,
                                                         int16_t orgX, int16_t orgY,
                                                         uint32_t width, uint32_t height,
                                                         int16_t clutX, int16_t clutY) {
    TextureResource* bestExact = nullptr;
    int bestExactPri = 999;
    for (auto& pair : m_textures) {
        TextureResource* tr = pair.second.get();
        if (!tr) continue;
        const TimImage& tim = tr->tim;
        if ((int)tim.bpp != bpp) continue;
        if (tim.orgX != orgX || tim.orgY != orgY) continue;
        if (tim.width != width || tim.height != height) continue;
        if (tim.clutX != clutX || tim.clutY != clutY) continue;

        const int pri = TextureKeyPriority(pair.first);

        if (!bestExact || pri < bestExactPri) {
            bestExact = tr;
            bestExactPri = pri;
            if (pri == 0) {
                break;
            }
        }
    }
    if (bestExact) {
        return bestExact;
    }

    std::string bestCyKey;
    TextureResource* bestCyTr = nullptr;
    int bestCyRow = 0;
    int bestCyPri = 999;
    for (auto& pair : m_textures) {
        if (pair.first.find("~cy") != std::string::npos || pair.first.find("~sub") != std::string::npos) {
            continue;
        }
        TextureResource* tr = pair.second.get();
        if (!tr) continue;
        const TimImage& tim = tr->tim;
        if ((int)tim.bpp != bpp) continue;
        if (tim.orgX != orgX || tim.orgY != orgY) continue;
        if (tim.width != width || tim.height != height) continue;
        if (tim.clutX != clutX) continue;
        if (tim.clutH <= 1) continue;
        if (clutY < tim.clutY) continue;
        const int row = (int)clutY - (int)tim.clutY;
        if (row <= 0) continue;
        if (row >= (int)tim.clutH) continue;
        if (tim.palette.empty() || tim.pixels.empty()) continue;

        const int pri = TextureKeyPriority(pair.first);

        if (!bestCyTr || pri < bestCyPri) {
            bestCyTr = tr;
            bestCyKey = pair.first;
            bestCyRow = row;
            bestCyPri = pri;
            if (pri == 0) {
                break;
            }
        }
    }

    if (bestCyTr) {
        std::string derivedKey = bestCyKey;
        derivedKey += "~cy";
        derivedKey += std::to_string((int)clutY);
        auto it = m_textures.find(derivedKey);
        if (it != m_textures.end()) {
            return it->second.get();
        }

        const TimImage& tim = bestCyTr->tim;
        auto res = std::make_unique<TextureResource>();
        res->name = bestCyTr->name;
        res->tim = tim;
        TimDecoder::ApplyPalette(res->tim, bestCyRow);
        std::string baseKey = StripTextureKeyPrefix(bestCyKey);
        if (ShouldApplyIndex0Transparency(baseKey)) {
            ApplyIndex0Transparency(res->tim);
        }
        res->tim.clutY = clutY;
        m_textures[derivedKey] = std::move(res);
        return m_textures[derivedKey].get();
    }

    std::string bestKey;
    TextureResource* bestTr = nullptr;
    int bestRow = 0;
    int bestSrcPxX = 0;
    int bestSrcPxY = 0;
    uint64_t bestCost = 0;
    bool bestHas = false;
    int bestPri = 999;

    for (auto& pair : m_textures) {
        if (pair.first.find("~cy") != std::string::npos || pair.first.find("~sub") != std::string::npos) {
            continue;
        }
        TextureResource* tr = pair.second.get();
        if (!tr) continue;
        const TimImage& tim = tr->tim;
        if ((int)tim.bpp != bpp) continue;
        if (orgX < tim.orgX || orgY < tim.orgY) continue;
        if (tim.width == 0 || tim.height == 0) continue;
        if (tim.palette.empty() || tim.pixels.empty()) continue;
        if (tim.clutX != clutX) continue;
        if (tim.clutH <= 0) continue;
        if (clutY < tim.clutY) continue;
        const int row = (int)clutY - (int)tim.clutY;
        if (row < 0 || row >= (int)tim.clutH) continue;

        const int srcPxX = (int)(orgX - tim.orgX);
        const int srcPxY = (int)(orgY - tim.orgY);
        if (srcPxX < 0 || srcPxY < 0) continue;
        if ((uint32_t)srcPxX + width > tim.width) continue;
        if ((uint32_t)srcPxY + height > tim.height) continue;

        const int pri = TextureKeyPriority(pair.first);

        const uint64_t dx = (uint64_t)srcPxX;
        const uint64_t dy = (uint64_t)srcPxY;
        const uint64_t cost = dy * 4096ull + dx;
        if (!bestHas || pri < bestPri || (pri == bestPri && cost < bestCost)) {
            bestHas = true;
            bestPri = pri;
            bestCost = cost;
            bestKey = pair.first;
            bestTr = tr;
            bestRow = row;
            bestSrcPxX = srcPxX;
            bestSrcPxY = srcPxY;
        }
    }

    if (bestHas && bestTr) {
        std::string derivedKey = bestKey;
        derivedKey += "~cy";
        derivedKey += std::to_string((int)clutY);
        derivedKey += "~sub";
        derivedKey += std::to_string((int)orgX);
        derivedKey += "_";
        derivedKey += std::to_string((int)orgY);
        derivedKey += "_";
        derivedKey += std::to_string((int)width);
        derivedKey += "_";
        derivedKey += std::to_string((int)height);

        auto it = m_textures.find(derivedKey);
        if (it != m_textures.end()) {
            return it->second.get();
        }

        const TimImage& tim = bestTr->tim;
        auto res = std::make_unique<TextureResource>();
        res->name = bestTr->name;
        res->tim = tim;
        TimDecoder::ApplyPalette(res->tim, bestRow);
        std::string baseKey = StripTextureKeyPrefix(bestKey);
        if (ShouldApplyIndex0Transparency(baseKey)) {
            ApplyIndex0Transparency(res->tim);
        }
        if (res->tim.rgba.size() != (size_t)res->tim.width * (size_t)res->tim.height) {
            return nullptr;
        }

        std::vector<uint32_t> sub;
        std::vector<uint32_t> subPsxAbr1Stp;
        if (!CropRgbaPlane(res->tim.rgba,
                           res->tim.width,
                           res->tim.height,
                           bestSrcPxX,
                           bestSrcPxY,
                           width,
                           height,
                           sub) ||
            (!res->tim.rgbaPsxAbr1Stp.empty() &&
             !CropRgbaPlane(res->tim.rgbaPsxAbr1Stp,
                            res->tim.width,
                            res->tim.height,
                            bestSrcPxX,
                            bestSrcPxY,
                            width,
                            height,
                            subPsxAbr1Stp))) {
            return nullptr;
        }

        res->tim.rgba = std::move(sub);
        res->tim.rgbaPsxAbr1Stp = std::move(subPsxAbr1Stp);
        res->tim.width = width;
        res->tim.height = height;
        res->tim.orgX = orgX;
        res->tim.orgY = orgY;
        res->tim.clutX = clutX;
        res->tim.clutY = clutY;

        m_textures[derivedKey] = std::move(res);
        return m_textures[derivedKey].get();
    }

    std::string pixelKey;
    TextureResource* pixelTr = nullptr;
    int pixelPri = 999;
    uint64_t pixelCost = 0;
    bool pixelHas = false;
    int pixelSrcPxX = 0;
    int pixelSrcPxY = 0;

    for (auto& pair : m_textures) {
        if (pair.first.find("~cy") != std::string::npos || pair.first.find("~sub") != std::string::npos) {
            continue;
        }
        TextureResource* tr = pair.second.get();
        if (!tr) continue;
        const TimImage& tim = tr->tim;
        if ((int)tim.bpp != bpp) continue;
        if (orgX < tim.orgX || orgY < tim.orgY) continue;
        if (tim.width == 0 || tim.height == 0) continue;
        if (tim.pixels.empty()) continue;

        const int srcPxX = (int)(orgX - tim.orgX);
        const int srcPxY = (int)(orgY - tim.orgY);
        if (srcPxX < 0 || srcPxY < 0) continue;
        if ((uint32_t)srcPxX + width > tim.width) continue;
        if ((uint32_t)srcPxY + height > tim.height) continue;

        const int pri = TextureKeyPriority(pair.first);
        const uint64_t cost = (uint64_t)srcPxY * 4096ull + (uint64_t)srcPxX;
        if (!pixelHas || pri < pixelPri || (pri == pixelPri && cost < pixelCost)) {
            pixelHas = true;
            pixelPri = pri;
            pixelCost = cost;
            pixelKey = pair.first;
            pixelTr = tr;
            pixelSrcPxX = srcPxX;
            pixelSrcPxY = srcPxY;
        }
    }

    std::string palKey;
    TextureResource* palTr = nullptr;
    int palRow = 0;
    int palPri = 999;
    uint64_t palCost = 0;
    bool palHas = false;

    for (auto& pair : m_textures) {
        if (pair.first.find("~cy") != std::string::npos || pair.first.find("~sub") != std::string::npos) {
            continue;
        }
        TextureResource* tr = pair.second.get();
        if (!tr) continue;
        const TimImage& tim = tr->tim;
        if ((int)tim.bpp != bpp) continue;
        if (tim.clutX != clutX) continue;
        if (tim.clutH <= 0) continue;
        if (clutY < tim.clutY) continue;
        const int row = (int)clutY - (int)tim.clutY;
        if (row < 0 || row >= (int)tim.clutH) continue;
        if (tim.palette.empty()) continue;

        const int pri = TextureKeyPriority(pair.first);
        const uint64_t cost = ((uint64_t)(uint16_t)tim.clutH << 32) | (uint64_t)(uint16_t)tim.clutY;
        if (!palHas || pri < palPri || (pri == palPri && cost < palCost)) {
            palHas = true;
            palPri = pri;
            palCost = cost;
            palKey = pair.first;
            palTr = tr;
            palRow = row;
        }
    }

    if (pixelHas && palHas && pixelTr && palTr) {
        std::string derivedKey = pixelKey;
        derivedKey += "~pal";
        derivedKey += palKey;
        derivedKey += "~cy";
        derivedKey += std::to_string((int)clutY);
        derivedKey += "~sub";
        derivedKey += std::to_string((int)orgX);
        derivedKey += "_";
        derivedKey += std::to_string((int)orgY);
        derivedKey += "_";
        derivedKey += std::to_string((int)width);
        derivedKey += "_";
        derivedKey += std::to_string((int)height);

        auto it = m_textures.find(derivedKey);
        if (it != m_textures.end()) {
            return it->second.get();
        }

        auto res = std::make_unique<TextureResource>();
        res->name = pixelTr->name;
        res->tim = pixelTr->tim;
        res->tim.palette = palTr->tim.palette;
        res->tim.clutX = clutX;
        res->tim.clutY = palTr->tim.clutY;
        res->tim.clutW = palTr->tim.clutW;
        res->tim.clutH = palTr->tim.clutH;
        TimDecoder::ApplyPalette(res->tim, palRow);

        std::string baseKey = StripTextureKeyPrefix(pixelKey);
        if (ShouldApplyIndex0Transparency(baseKey)) {
            ApplyIndex0Transparency(res->tim);
        }
        if (res->tim.rgba.size() != (size_t)res->tim.width * (size_t)res->tim.height) {
            return nullptr;
        }

        std::vector<uint32_t> sub;
        std::vector<uint32_t> subPsxAbr1Stp;
        if (!CropRgbaPlane(res->tim.rgba,
                           res->tim.width,
                           res->tim.height,
                           pixelSrcPxX,
                           pixelSrcPxY,
                           width,
                           height,
                           sub) ||
            (!res->tim.rgbaPsxAbr1Stp.empty() &&
             !CropRgbaPlane(res->tim.rgbaPsxAbr1Stp,
                            res->tim.width,
                            res->tim.height,
                            pixelSrcPxX,
                            pixelSrcPxY,
                            width,
                            height,
                            subPsxAbr1Stp))) {
            return nullptr;
        }

        res->tim.rgba = std::move(sub);
        res->tim.rgbaPsxAbr1Stp = std::move(subPsxAbr1Stp);
        res->tim.width = width;
        res->tim.height = height;
        res->tim.orgX = orgX;
        res->tim.orgY = orgY;
        res->tim.clutX = clutX;
        res->tim.clutY = clutY;
        res->tim.clutH = 1;

        m_textures[derivedKey] = std::move(res);
        return m_textures[derivedKey].get();
    }

    return nullptr;
}

const std::vector<uint8_t>* ResourceManager::GetMem(const std::string& name) const {
    std::string key = name;
    std::transform(key.begin(), key.end(), key.begin(), ::tolower);

    auto it = m_mem.find(key);
    if (it != m_mem.end()) {
        return &it->second;
    }
    return nullptr;
}

std::vector<std::string> ResourceManager::GetTextureNames() const {
    std::vector<std::string> names;
    names.reserve(m_textures.size());
    for (const auto& pair : m_textures) {
        names.push_back(pair.first);
    }
    return names;
}

std::vector<std::string> ResourceManager::GetMemNames() const {
    std::vector<std::string> names;
    names.reserve(m_mem.size());
    for (const auto& pair : m_mem) {
        names.push_back(pair.first);
    }
    return names;
}

std::vector<std::string> ResourceManager::GetTimRawNames() const {
    std::vector<std::string> names;
    names.reserve(m_timRawOrder.size());
    for (const std::string& key : m_timRawOrder) {
        if (m_timRaw.find(key) != m_timRaw.end()) {
            names.push_back(key);
        }
    }
    return names;
}

void ResourceManager::Clear() {
    ++m_generation;  // 资源变更，触发背景重选
    for (auto& pair : m_textures) {
        DestroyTextureViews(m_renderer, pair.second.get());
    }
    m_textures.clear();
    m_mem.clear();
    m_memOrder.clear();
    m_timRaw.clear();
    m_timRawOrder.clear();
}
