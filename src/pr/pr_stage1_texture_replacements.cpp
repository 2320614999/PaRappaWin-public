#include "pr_stage1_texture_replacements.h"

#include "d3d11_renderer.h"
#include "logger.h"
#include "pr_game_context.h"

#include <wincodec.h>
#include <wrl/client.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cctype>
#include <climits>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <filesystem>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

using Microsoft::WRL::ComPtr;

namespace PrStage1TextureReplacements {
namespace {

struct ReplacementEntry {
    uint32_t textureHash = 0;
    uint32_t paletteHash = 0;
    int sourceWidth = 0;
    int sourceHeight = 0;
    int mipLevel = 0;
    std::string relativePath;
    std::filesystem::path path;
    int imageWidth = 0;
    int imageHeight = 0;
    bool sampleTried = false;
    bool sampleOk = false;
    bool textureTried = false;
    int sampleWidth = 0;
    int sampleHeight = 0;
    std::vector<uint32_t> sampleRgba;
    ID3D11ShaderResourceView* srv = nullptr;
    D3D11Renderer* renderer = nullptr;
};

struct AtlasSourceRect {
    uint16_t tpage = 0;
    uint16_t clut = 0;
    int x = 0;
    int y = 0;
    int w = 0;
    int h = 0;
    size_t entryIndex = 0;
    std::string sourceId;
    std::string params;
};

struct ReplacementCache {
    bool indexed = false;
    std::filesystem::path directory;
    std::vector<ReplacementEntry> entries;
    std::unordered_map<uint64_t, std::vector<size_t>> byDimension;
    std::unordered_map<uint64_t, int> matchCache;
    std::unordered_map<uint64_t, size_t> sourceMap;
    std::unordered_map<uint64_t, size_t> keyMap;
    std::unordered_map<uint64_t, std::string> keyParams;
    std::vector<AtlasSourceRect> atlasRects;
    std::unordered_set<uint64_t> keyMissCache;
    bool sourceMapLoaded = false;
    uint32_t matchBudgetFrame = UINT32_MAX;
    int newMatchesThisFrame = 0;
    int loggedHits = 0;
    int loggedMisses = 0;
};

ReplacementCache s_cache;
constexpr int kSignatureMaxSize = 16;
constexpr int kMaxNewMatchesPerFrame = 2;
constexpr const char* kSignatureIndexFile = "stage1_psphd_signature_index.tsv";
constexpr const char* kSourceMapFile = "stage1_psx_texture_map.tsv";
constexpr const char* kSourceKeyMapFile = "stage1_psx_texture_key_map.tsv";

uint64_t DimensionKey(int width, int height) {
    return (static_cast<uint64_t>(static_cast<uint32_t>(width)) << 32) |
           static_cast<uint32_t>(height);
}

uint64_t Mix64(uint64_t v) {
    v ^= v >> 33;
    v *= 0xff51afd7ed558ccdULL;
    v ^= v >> 33;
    v *= 0xc4ceb9fe1a85ec53ULL;
    v ^= v >> 33;
    return v;
}

uint64_t MakeMatchKey(uint64_t sourceKey,
                      uint64_t rgbaHash,
                      int width,
                      int height) {
    uint64_t key = Mix64(sourceKey);
    key ^= Mix64(rgbaHash + 0x9e3779b97f4a7c15ULL);
    key ^= Mix64(DimensionKey(width, height));
    return key;
}

uint64_t SourceMapKey(uint64_t rgbaHash, int width, int height) {
    return Mix64(rgbaHash) ^ Mix64(DimensionKey(width, height));
}

uint16_t NormalizeTpageTextureKey(uint16_t tpage) {
    return static_cast<uint16_t>((tpage & 0x01FFu) & ~0x0060u);
}

std::string ToHex64(uint64_t value) {
    char buf[17];
    std::snprintf(buf, sizeof(buf), "%016llX",
                  static_cast<unsigned long long>(value));
    return std::string(buf);
}

bool ParseHex64(const std::string& s, uint64_t& out) {
    if (s.size() != 16u) {
        return false;
    }
    for (char ch : s) {
        const bool hex =
            (ch >= '0' && ch <= '9') ||
            (ch >= 'a' && ch <= 'f') ||
            (ch >= 'A' && ch <= 'F');
        if (!hex) {
            return false;
        }
    }
    out = static_cast<uint64_t>(std::strtoull(s.c_str(), nullptr, 16));
    return true;
}

std::vector<std::string> SplitTabs(const std::string& line) {
    std::vector<std::string> out;
    size_t start = 0u;
    while (start <= line.size()) {
        const size_t pos = line.find('\t', start);
        if (pos == std::string::npos) {
            out.push_back(line.substr(start));
            break;
        }
        out.push_back(line.substr(start, pos - start));
        start = pos + 1u;
    }
    return out;
}

std::string ExtractParamValue(const std::string& params, const char* name) {
    if (name == nullptr || name[0] == '\0') {
        return {};
    }
    const std::string prefix = std::string(name) + "=";
    size_t start = 0u;
    while (start <= params.size()) {
        size_t end = params.find_first_of("\t; ", start);
        if (end == std::string::npos) {
            end = params.size();
        }
        const std::string token = params.substr(start, end - start);
        if (token.rfind(prefix, 0u) == 0u) {
            return token.substr(prefix.size());
        }
        if (end == params.size()) {
            break;
        }
        start = params.find_first_not_of("\t; ", end);
        if (start == std::string::npos) {
            break;
        }
    }
    return {};
}

bool ParseHexByte(const std::string& text, size_t pos, uint8_t& out) {
    if (pos + 2u > text.size()) {
        return false;
    }
    unsigned value = 0u;
    for (size_t i = 0u; i < 2u; ++i) {
        const char ch = text[pos + i];
        value <<= 4u;
        if (ch >= '0' && ch <= '9') {
            value |= static_cast<unsigned>(ch - '0');
        } else if (ch >= 'a' && ch <= 'f') {
            value |= static_cast<unsigned>(ch - 'a' + 10);
        } else if (ch >= 'A' && ch <= 'F') {
            value |= static_cast<unsigned>(ch - 'A' + 10);
        } else {
            return false;
        }
    }
    out = static_cast<uint8_t>(value);
    return true;
}

bool ParseTintParam(const std::string& params, float& r, float& g, float& b) {
    std::string value = ExtractParamValue(params, "tint");
    if (value.empty()) {
        value = ExtractParamValue(params, "rgb");
    }
    if (value.empty()) {
        return false;
    }
    if (value.rfind("0x", 0u) == 0u || value.rfind("0X", 0u) == 0u) {
        value = value.substr(2u);
    }
    if (!value.empty() && value[0] == '#') {
        value = value.substr(1u);
    }
    if (value.size() != 6u) {
        return false;
    }
    uint8_t rr = 0u;
    uint8_t gg = 0u;
    uint8_t bb = 0u;
    if (!ParseHexByte(value, 0u, rr) ||
        !ParseHexByte(value, 2u, gg) ||
        !ParseHexByte(value, 4u, bb)) {
        return false;
    }
    r = static_cast<float>(rr) / 255.0f;
    g = static_cast<float>(gg) / 255.0f;
    b = static_cast<float>(bb) / 255.0f;
    return true;
}

void ApplyParamsToReplacement(const std::string& params, ReplacementTexture& out) {
    float r = 1.0f;
    float g = 1.0f;
    float b = 1.0f;
    if (ParseTintParam(params, r, g, b)) {
        out.hasTint = true;
        out.tintR = r;
        out.tintG = g;
        out.tintB = b;
    }
}

std::string MakeRelativePath(const std::filesystem::path& root,
                             const std::filesystem::path& path) {
    std::error_code ec;
    std::filesystem::path rel = std::filesystem::relative(path, root, ec);
    if (ec || rel.empty()) {
        rel = path.filename();
    }
    return rel.generic_u8string();
}

std::string EncodeSignatureHex(const std::vector<uint32_t>& pixels) {
    static constexpr char kHex[] = "0123456789ABCDEF";
    std::string out;
    out.resize(pixels.size() * 8u);
    size_t pos = 0u;
    for (uint32_t px : pixels) {
        for (int shift = 28; shift >= 0; shift -= 4) {
            out[pos++] = kHex[(px >> shift) & 0xFu];
        }
    }
    return out;
}

bool DecodeSignatureHex(const std::string& hex,
                        std::vector<uint32_t>& out) {
    if ((hex.size() % 8u) != 0u) {
        return false;
    }
    out.clear();
    out.reserve(hex.size() / 8u);
    for (size_t i = 0u; i < hex.size(); i += 8u) {
        uint32_t value = 0u;
        for (size_t j = 0u; j < 8u; ++j) {
            const char ch = hex[i + j];
            uint32_t nibble = 0u;
            if (ch >= '0' && ch <= '9') {
                nibble = static_cast<uint32_t>(ch - '0');
            } else if (ch >= 'a' && ch <= 'f') {
                nibble = static_cast<uint32_t>(ch - 'a' + 10);
            } else if (ch >= 'A' && ch <= 'F') {
                nibble = static_cast<uint32_t>(ch - 'A' + 10);
            } else {
                out.clear();
                return false;
            }
            value = (value << 4) | nibble;
        }
        out.push_back(value);
    }
    return true;
}

bool IsHex8(const std::string& s) {
    if (s.size() != 8u) {
        return false;
    }
    for (char ch : s) {
        const bool hex =
            (ch >= '0' && ch <= '9') ||
            (ch >= 'a' && ch <= 'f') ||
            (ch >= 'A' && ch <= 'F');
        if (!hex) {
            return false;
        }
    }
    return true;
}

bool ParseUint32Hex(const std::string& s, uint32_t& out) {
    if (!IsHex8(s)) {
        return false;
    }
    out = static_cast<uint32_t>(std::strtoul(s.c_str(), nullptr, 16));
    return true;
}

bool ParsePositiveInt(const std::string& s, int& out) {
    if (s.empty()) {
        return false;
    }
    for (char ch : s) {
        if (ch < '0' || ch > '9') {
            return false;
        }
    }
    const long v = std::strtol(s.c_str(), nullptr, 10);
    if (v <= 0 || v > 16384) {
        return false;
    }
    out = static_cast<int>(v);
    return true;
}

bool ParseNonNegativeInt(const std::string& s, int& out) {
    if (s.empty()) {
        return false;
    }
    for (char ch : s) {
        if (ch < '0' || ch > '9') {
            return false;
        }
    }
    const long v = std::strtol(s.c_str(), nullptr, 10);
    if (v < 0 || v > 255) {
        return false;
    }
    out = static_cast<int>(v);
    return true;
}

bool ParsePsphdName(const std::filesystem::path& path,
                    ReplacementEntry& out) {
    const std::string stem = path.stem().string();
    std::vector<std::string> parts;
    size_t start = 0u;
    while (start <= stem.size()) {
        const size_t pos = stem.find('_', start);
        if (pos == std::string::npos) {
            parts.push_back(stem.substr(start));
            break;
        }
        parts.push_back(stem.substr(start, pos - start));
        start = pos + 1u;
    }
    if (parts.size() != 5u) {
        return false;
    }
    if (!ParseUint32Hex(parts[0], out.textureHash) ||
        !ParseUint32Hex(parts[1], out.paletteHash) ||
        !ParsePositiveInt(parts[2], out.sourceWidth) ||
        !ParsePositiveInt(parts[3], out.sourceHeight) ||
        !ParseNonNegativeInt(parts[4], out.mipLevel)) {
        return false;
    }
    out.path = path;
    return true;
}

bool HasSupportedExtension(const std::filesystem::path& path) {
    std::string ext = path.extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return ext == ".png" || ext == ".jpg" || ext == ".jpeg";
}

std::filesystem::path WeakCanonicalOrOriginal(const std::filesystem::path& path) {
    std::error_code ec;
    std::filesystem::path out = std::filesystem::weakly_canonical(path, ec);
    return ec ? path : out;
}

std::filesystem::path ResolveReplacementDirectory(const PrGameContext& ctx) {
    std::filesystem::path configured = ctx.stage1TextureReplacementDir;
    if (configured.empty()) {
        configured = "ex/image/texreplace";
    }
    if (configured.is_absolute()) {
        return WeakCanonicalOrOriginal(configured);
    }

    std::vector<std::filesystem::path> bases;
    if (!ctx.dataRoot.empty()) {
        bases.push_back(ctx.dataRoot);
        if (ctx.dataRoot.has_parent_path()) {
            bases.push_back(ctx.dataRoot.parent_path());
        }
    }
    bases.push_back(std::filesystem::current_path());
    if (!std::filesystem::current_path().empty() &&
        std::filesystem::current_path().has_parent_path()) {
        bases.push_back(std::filesystem::current_path().parent_path());
    }

    for (const std::filesystem::path& base : bases) {
        std::error_code ec;
        const std::filesystem::path candidate =
            WeakCanonicalOrOriginal(base / configured);
        if (std::filesystem::is_directory(candidate, ec)) {
            return candidate;
        }
    }
    return WeakCanonicalOrOriginal(bases.empty() ? configured : bases.front() / configured);
}

void ReleaseEntryTexture(ReplacementEntry& entry) {
    if (entry.srv != nullptr) {
        entry.srv->Release();
        entry.srv = nullptr;
    }
    entry.renderer = nullptr;
    entry.textureTried = false;
}

void ResetCacheContents() {
    for (ReplacementEntry& entry : s_cache.entries) {
        ReleaseEntryTexture(entry);
    }
    s_cache = ReplacementCache{};
}

bool CreateWicFactory(ComPtr<IWICImagingFactory>& factory) {
    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (FAILED(hr) && hr != RPC_E_CHANGED_MODE) {
        Log::Printf("Stage1TextureReplacements: CoInitializeEx failed hr=0x%08X",
                    static_cast<unsigned>(hr));
    }
    hr = CoCreateInstance(CLSID_WICImagingFactory,
                          nullptr,
                          CLSCTX_INPROC_SERVER,
                          IID_PPV_ARGS(&factory));
    if (FAILED(hr)) {
        Log::Printf("Stage1TextureReplacements: WIC factory failed hr=0x%08X",
                    static_cast<unsigned>(hr));
        return false;
    }
    return true;
}

bool LoadImageRgba(const std::filesystem::path& path,
                   int requestedWidth,
                   int requestedHeight,
                   std::vector<uint32_t>& outRgba,
                   int& outWidth,
                   int& outHeight) {
    outRgba.clear();
    outWidth = 0;
    outHeight = 0;

    ComPtr<IWICImagingFactory> factory;
    if (!CreateWicFactory(factory)) {
        return false;
    }

    ComPtr<IWICBitmapDecoder> decoder;
    HRESULT hr = factory->CreateDecoderFromFilename(path.wstring().c_str(),
                                                    nullptr,
                                                    GENERIC_READ,
                                                    WICDecodeMetadataCacheOnDemand,
                                                    &decoder);
    if (FAILED(hr)) {
        return false;
    }

    ComPtr<IWICBitmapFrameDecode> frame;
    hr = decoder->GetFrame(0, &frame);
    if (FAILED(hr)) {
        return false;
    }

    UINT nativeWidth = 0u;
    UINT nativeHeight = 0u;
    hr = frame->GetSize(&nativeWidth, &nativeHeight);
    if (FAILED(hr) || nativeWidth == 0u || nativeHeight == 0u) {
        return false;
    }

    ComPtr<IWICBitmapSource> source;
    hr = frame.As(&source);
    if (FAILED(hr)) {
        return false;
    }

    UINT finalWidth = nativeWidth;
    UINT finalHeight = nativeHeight;
    if (requestedWidth > 0 && requestedHeight > 0 &&
        (static_cast<UINT>(requestedWidth) != nativeWidth ||
         static_cast<UINT>(requestedHeight) != nativeHeight)) {
        ComPtr<IWICBitmapScaler> scaler;
        hr = factory->CreateBitmapScaler(&scaler);
        if (FAILED(hr)) {
            return false;
        }
        hr = scaler->Initialize(source.Get(),
                                static_cast<UINT>(requestedWidth),
                                static_cast<UINT>(requestedHeight),
                                WICBitmapInterpolationModeFant);
        if (FAILED(hr)) {
            return false;
        }
        source.Reset();
        hr = scaler.As(&source);
        if (FAILED(hr)) {
            return false;
        }
        finalWidth = static_cast<UINT>(requestedWidth);
        finalHeight = static_cast<UINT>(requestedHeight);
    }

    ComPtr<IWICFormatConverter> converter;
    hr = factory->CreateFormatConverter(&converter);
    if (FAILED(hr)) {
        return false;
    }
    hr = converter->Initialize(source.Get(),
                               GUID_WICPixelFormat32bppRGBA,
                               WICBitmapDitherTypeNone,
                               nullptr,
                               0.0,
                               WICBitmapPaletteTypeCustom);
    if (FAILED(hr)) {
        return false;
    }

    outRgba.resize(static_cast<size_t>(finalWidth) *
                   static_cast<size_t>(finalHeight));
    hr = converter->CopyPixels(nullptr,
                               finalWidth * 4u,
                               static_cast<UINT>(outRgba.size() * 4u),
                               reinterpret_cast<BYTE*>(outRgba.data()));
    if (FAILED(hr)) {
        outRgba.clear();
        return false;
    }

    outWidth = static_cast<int>(finalWidth);
    outHeight = static_cast<int>(finalHeight);
    return true;
}

bool EnsureSampleLoaded(ReplacementEntry& entry) {
    if (entry.sampleTried) {
        return entry.sampleOk;
    }
    entry.sampleTried = true;
    int width = 0;
    int height = 0;
    const int sampleWidth = (std::min)(kSignatureMaxSize, entry.sourceWidth);
    const int sampleHeight = (std::min)(kSignatureMaxSize, entry.sourceHeight);
    entry.sampleOk = LoadImageRgba(entry.path,
                                   sampleWidth,
                                   sampleHeight,
                                   entry.sampleRgba,
                                   width,
                                   height);
    entry.sampleWidth = width;
    entry.sampleHeight = height;
    return entry.sampleOk && width == sampleWidth && height == sampleHeight;
}

void PreloadReplacementSamples() {
    const auto start = std::chrono::steady_clock::now();
    int loaded = 0;
    int failed = 0;
    int already = 0;
    for (ReplacementEntry& entry : s_cache.entries) {
        if (entry.sampleOk) {
            ++already;
            continue;
        }
        if (EnsureSampleLoaded(entry)) {
            ++loaded;
        } else {
            ++failed;
        }
    }
    const auto end = std::chrono::steady_clock::now();
    const auto ms =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
            .count();
    Log::Printf("Stage1TextureReplacements: preloaded %d signature(s), cached %d, failed %d, %lld ms",
                loaded,
                already,
                failed,
                static_cast<long long>(ms));
}

int LoadSignatureIndex() {
    const std::filesystem::path path = s_cache.directory / kSignatureIndexFile;
    std::ifstream f(path);
    if (!f.is_open()) {
        return 0;
    }

    std::unordered_map<std::string, size_t> byPath;
    for (size_t i = 0u; i < s_cache.entries.size(); ++i) {
        byPath.emplace(s_cache.entries[i].relativePath, i);
    }

    int loaded = 0;
    std::string line;
    while (std::getline(f, line)) {
        if (line.empty() || line[0] == '#') {
            continue;
        }
        const std::vector<std::string> fields = SplitTabs(line);
        if (fields.size() < 6u) {
            continue;
        }
        auto it = byPath.find(fields[0]);
        if (it == byPath.end()) {
            continue;
        }

        int sourceWidth = 0;
        int sourceHeight = 0;
        int sampleWidth = 0;
        int sampleHeight = 0;
        if (!ParsePositiveInt(fields[1], sourceWidth) ||
            !ParsePositiveInt(fields[2], sourceHeight) ||
            !ParsePositiveInt(fields[3], sampleWidth) ||
            !ParsePositiveInt(fields[4], sampleHeight)) {
            continue;
        }

        ReplacementEntry& entry = s_cache.entries[it->second];
        if (entry.sourceWidth != sourceWidth ||
            entry.sourceHeight != sourceHeight ||
            sampleWidth != (std::min)(kSignatureMaxSize, entry.sourceWidth) ||
            sampleHeight != (std::min)(kSignatureMaxSize, entry.sourceHeight)) {
            continue;
        }

        std::vector<uint32_t> signature;
        if (!DecodeSignatureHex(fields[5], signature) ||
            signature.size() != static_cast<size_t>(sampleWidth) *
                                    static_cast<size_t>(sampleHeight)) {
            continue;
        }
        entry.sampleRgba = std::move(signature);
        entry.sampleWidth = sampleWidth;
        entry.sampleHeight = sampleHeight;
        entry.sampleTried = true;
        entry.sampleOk = true;
        ++loaded;
    }

    if (loaded > 0) {
        Log::Printf("Stage1TextureReplacements: loaded %d PSPHD signature(s) from '%s'",
                    loaded,
                    path.u8string().c_str());
    }
    return loaded;
}

void SaveSignatureIndex() {
    const std::filesystem::path path = s_cache.directory / kSignatureIndexFile;
    std::ofstream f(path, std::ios::binary | std::ios::trunc);
    if (!f.is_open()) {
        Log::Printf("Stage1TextureReplacements: failed to write signature index '%s'",
                    path.u8string().c_str());
        return;
    }

    f << "# stage1_psphd_signature_index_v1\n";
    f << "# rel_path\tsource_w\tsource_h\tsignature_w\tsignature_h\trgba8888_hex\n";
    int saved = 0;
    for (const ReplacementEntry& entry : s_cache.entries) {
        if (!entry.sampleOk || entry.sampleRgba.empty()) {
            continue;
        }
        f << entry.relativePath << '\t'
          << entry.sourceWidth << '\t'
          << entry.sourceHeight << '\t'
          << entry.sampleWidth << '\t'
          << entry.sampleHeight << '\t'
          << EncodeSignatureHex(entry.sampleRgba) << '\n';
        ++saved;
    }
    Log::Printf("Stage1TextureReplacements: wrote %d PSPHD signature(s) to '%s'",
                saved,
                path.u8string().c_str());
}

void EnsureReplacementSignatures() {
    const int loadedFromIndex = LoadSignatureIndex();
    bool anyMissing = false;
    for (const ReplacementEntry& entry : s_cache.entries) {
        if (!entry.sampleOk) {
            anyMissing = true;
            break;
        }
    }
    if (!anyMissing) {
        return;
    }
    PreloadReplacementSamples();
    if (loadedFromIndex < static_cast<int>(s_cache.entries.size())) {
        SaveSignatureIndex();
    }
}

void LoadSourceMap() {
    s_cache.sourceMapLoaded = true;
    s_cache.sourceMap.clear();

    const std::filesystem::path path = s_cache.directory / kSourceMapFile;
    std::ifstream f(path);
    if (!f.is_open()) {
        Log::Printf("Stage1TextureReplacements: source map missing '%s'",
                    path.u8string().c_str());
        return;
    }

    std::unordered_map<std::string, size_t> byPath;
    int loaded = 0;
    std::string line;
    while (std::getline(f, line)) {
        if (line.empty() || line[0] == '#') {
            continue;
        }
        const std::vector<std::string> fields = SplitTabs(line);
        if (fields.size() < 5u) {
            continue;
        }

        int width = 0;
        int height = 0;
        uint64_t rgbaHash = 0u;
        if (!ParsePositiveInt(fields[0], width) ||
            !ParsePositiveInt(fields[1], height) ||
            !ParseHex64(fields[2], rgbaHash)) {
            continue;
        }

        auto it = byPath.find(fields[3]);
        if (it == byPath.end()) {
            ReplacementEntry entry;
            entry.relativePath = fields[3];
            entry.path = s_cache.directory / std::filesystem::u8path(fields[3]);
            entry.sourceWidth = width;
            entry.sourceHeight = height;
            ParsePsphdName(entry.path, entry);
            entry.relativePath = fields[3];
            entry.path = s_cache.directory / std::filesystem::u8path(fields[3]);
            const size_t index = s_cache.entries.size();
            byPath.emplace(fields[3], index);
            s_cache.entries.push_back(std::move(entry));
            it = byPath.find(fields[3]);
            if (it == byPath.end()) {
                continue;
            }
        }

        s_cache.sourceMap[SourceMapKey(rgbaHash, width, height)] = it->second;
        ++loaded;
    }

    Log::Printf("Stage1TextureReplacements: loaded %d PSX source map entries (%llu texture file(s)) from '%s'",
                loaded,
                static_cast<unsigned long long>(s_cache.entries.size()),
                path.u8string().c_str());
}

size_t GetOrCreateMappedEntry(const std::string& relativePath,
                              int sourceWidth,
                              int sourceHeight,
                              std::unordered_map<std::string, size_t>& byPath) {
    auto it = byPath.find(relativePath);
    if (it != byPath.end()) {
        return it->second;
    }

    ReplacementEntry entry;
    entry.relativePath = relativePath;
    entry.path = s_cache.directory / std::filesystem::u8path(relativePath);
    entry.sourceWidth = sourceWidth;
    entry.sourceHeight = sourceHeight;
    ParsePsphdName(entry.path, entry);
    entry.relativePath = relativePath;
    entry.path = s_cache.directory / std::filesystem::u8path(relativePath);
    const size_t index = s_cache.entries.size();
    byPath.emplace(relativePath, index);
    s_cache.entries.push_back(std::move(entry));
    return index;
}

bool BuildTexdumpAtlasRect(int bpp,
                           int orgX,
                           int orgY,
                           int width,
                           int height,
                           int clutX,
                           int clutY,
                           AtlasSourceRect& out) {
    if (width <= 0 || height <= 0) {
        return false;
    }

    int colorMode = -1;
    int pxFactor = 1;
    if (bpp == 4) {
        colorMode = 0;
        pxFactor = 4;
    } else if (bpp == 8) {
        colorMode = 1;
        pxFactor = 2;
    } else if (bpp == 16) {
        colorMode = 2;
        pxFactor = 1;
    } else if (bpp == 24) {
        colorMode = 3;
        pxFactor = 1;
    } else {
        return false;
    }

    const uint16_t tpage = static_cast<uint16_t>(
        ((orgX / 64) & 0x0F) |
        ((orgY >= 256 ? 1 : 0) << 4) |
        (colorMode << 7));
    const int baseHw = (tpage & 0x0Fu) * 64;
    const int baseY = ((tpage >> 4) & 1u) * 256;
    const int u = (orgX - baseHw) * pxFactor;
    const int v = orgY - baseY;
    if (u < 0 || u > 255 || v < 0 || v > 255) {
        return false;
    }

    out.tpage = NormalizeTpageTextureKey(tpage);
    out.clut = (bpp == 4 || bpp == 8)
                   ? static_cast<uint16_t>(((clutY & 0x01FF) << 6) |
                                           ((clutX >> 4) & 0x3F))
                   : 0u;
    out.x = u;
    out.y = v;
    out.w = width;
    out.h = height;
    return true;
}

void LoadTexdumpAtlasRects(
    const std::unordered_map<std::string, size_t>& sourceIdToEntryIndex,
    const std::unordered_map<std::string, std::string>& sourceIdToParams) {
    s_cache.atlasRects.clear();
    if (sourceIdToEntryIndex.empty()) {
        return;
    }

    const std::filesystem::path path =
        s_cache.directory / "stage1_resource_dump" / "manifest.tsv";
    std::ifstream f(path);
    if (!f.is_open()) {
        return;
    }

    int loaded = 0;
    std::string line;
    while (std::getline(f, line)) {
        if (line.empty() || line[0] == '#') {
            continue;
        }
        const std::vector<std::string> fields = SplitTabs(line);
        if (fields.size() < 7u) {
            continue;
        }

        const auto sourceIt = sourceIdToEntryIndex.find(fields[0]);
        if (sourceIt == sourceIdToEntryIndex.end() ||
            sourceIt->second >= s_cache.entries.size()) {
            continue;
        }

        int width = 0;
        int height = 0;
        if (!ParsePositiveInt(fields[3], width) ||
            !ParsePositiveInt(fields[4], height)) {
            continue;
        }

        int bpp = 0;
        int orgX = 0;
        int orgY = 0;
        int clutX = 0;
        int clutY = 0;
        if (sscanf_s(fields[6].c_str(),
                     "bpp=%d org=%d,%d clut=%d,%d",
                     &bpp,
                     &orgX,
                     &orgY,
                     &clutX,
                     &clutY) != 5) {
            continue;
        }

        AtlasSourceRect rect{};
        if (!BuildTexdumpAtlasRect(bpp, orgX, orgY, width, height, clutX, clutY, rect)) {
            continue;
        }
        rect.entryIndex = sourceIt->second;
        rect.sourceId = fields[0];
        const auto paramsIt = sourceIdToParams.find(fields[0]);
        if (paramsIt != sourceIdToParams.end()) {
            rect.params = paramsIt->second;
        }
        s_cache.atlasRects.push_back(std::move(rect));
        ++loaded;
    }

    if (loaded != 0) {
        Log::Printf("Stage1TextureReplacements: loaded %d texdump atlas rect(s) from '%s'",
                    loaded,
                    path.u8string().c_str());
    }
}

void LoadSourceKeyMap() {
    s_cache.keyMap.clear();
    s_cache.sourceMap.clear();
    s_cache.keyParams.clear();
    s_cache.atlasRects.clear();
    s_cache.entries.clear();

    const std::filesystem::path path = s_cache.directory / kSourceKeyMapFile;
    std::ifstream f(path);
    if (!f.is_open()) {
        Log::Printf("Stage1TextureReplacements: source key map missing '%s'",
                    path.u8string().c_str());
        return;
    }

    std::unordered_map<std::string, size_t> byPath;
    std::unordered_map<std::string, size_t> sourceIdToEntryIndex;
    std::unordered_map<std::string, std::string> sourceIdToParams;
    int loaded = 0;
    std::string line;
    while (std::getline(f, line)) {
        if (line.empty() || line[0] == '#') {
            continue;
        }
        const std::vector<std::string> fields = SplitTabs(line);
        if (fields.size() < 4u) {
            continue;
        }

        uint64_t sourceKey = 0u;
        int width = 0;
        int height = 0;
        if (!ParseHex64(fields[0], sourceKey) ||
            !ParsePositiveInt(fields[2], width) ||
            !ParsePositiveInt(fields[3], height)) {
            continue;
        }
        const size_t entryIndex =
            GetOrCreateMappedEntry(fields[1], width, height, byPath);
        s_cache.keyMap[sourceKey] = entryIndex;
        if (fields.size() >= 6u && !fields[5].empty()) {
            std::string params = fields[5];
            for (size_t i = 6u; i < fields.size(); ++i) {
                params += '\t';
                params += fields[i];
            }
            const std::string sourceId =
                ExtractParamValue(params, "from_texdump");
            if (!sourceId.empty()) {
                sourceIdToEntryIndex[sourceId] = entryIndex;
                const auto paramsIt = sourceIdToParams.find(sourceId);
                const bool newHasTint = !ExtractParamValue(params, "tint").empty() ||
                                        !ExtractParamValue(params, "rgb").empty();
                const bool oldHasTint =
                    paramsIt != sourceIdToParams.end() &&
                    (!ExtractParamValue(paramsIt->second, "tint").empty() ||
                     !ExtractParamValue(paramsIt->second, "rgb").empty());
                if (paramsIt == sourceIdToParams.end() || (newHasTint && !oldHasTint)) {
                    sourceIdToParams[sourceId] = params;
                }
            }
            s_cache.keyParams[sourceKey] = std::move(params);
        }
        ++loaded;
    }

    LoadTexdumpAtlasRects(sourceIdToEntryIndex, sourceIdToParams);

    Log::Printf("Stage1TextureReplacements: loaded %d PSX source key entries (%llu texture file(s), %llu param row(s), %llu atlas rect(s)) from '%s'",
                loaded,
                static_cast<unsigned long long>(s_cache.entries.size()),
                static_cast<unsigned long long>(s_cache.keyParams.size()),
                static_cast<unsigned long long>(s_cache.atlasRects.size()),
                path.u8string().c_str());
}

void AppendSourceMapRecord(int width,
                           int height,
                           uint64_t rgbaHash,
                           const ReplacementEntry& entry,
                           double score) {
    const std::filesystem::path path = s_cache.directory / kSourceMapFile;
    const bool exists = std::filesystem::exists(path);
    std::ofstream f(path, std::ios::binary | std::ios::app);
    if (!f.is_open()) {
        Log::Printf("Stage1TextureReplacements: failed to append source map '%s'",
                    path.u8string().c_str());
        return;
    }
    if (!exists) {
        f << "# stage1_psx_texture_map_v1\n";
        f << "# width\theight\trgba_hash64\trel_path\tscore\n";
    }
    f << width << '\t'
      << height << '\t'
      << ToHex64(rgbaHash) << '\t'
      << entry.relativePath << '\t'
      << score << '\n';
}

bool EnsureTextureLoaded(PrGameContext& ctx, ReplacementEntry& entry) {
    if (!ctx.renderer) {
        return false;
    }
    if (entry.srv != nullptr && entry.renderer == ctx.renderer) {
        return true;
    }
    if (entry.srv != nullptr && entry.renderer != ctx.renderer) {
        ReleaseEntryTexture(entry);
    }
    if (entry.textureTried) {
        return false;
    }
    entry.textureTried = true;

    std::vector<uint32_t> pixels;
    int width = 0;
    int height = 0;
    if (!LoadImageRgba(entry.path, 0, 0, pixels, width, height) ||
        pixels.empty()) {
        Log::Printf("Stage1TextureReplacements: failed to load '%s'",
                    entry.path.u8string().c_str());
        return false;
    }

    entry.srv = ctx.renderer->CreateTexture(pixels.data(), width, height);
    if (entry.srv == nullptr) {
        return false;
    }
    entry.renderer = ctx.renderer;
    entry.imageWidth = width;
    entry.imageHeight = height;
    return true;
}

double ImageDifference(const uint32_t* source,
                       int stridePixels,
                       const std::vector<uint32_t>& candidate,
                       int width,
                       int height) {
    if (!source || stridePixels <= 0 || candidate.size() <
        static_cast<size_t>(width) * static_cast<size_t>(height)) {
        return 1.0;
    }

    double sum = 0.0;
    double denom = 0.0;
    for (int y = 0; y < height; ++y) {
        const uint32_t* srcRow = source + static_cast<size_t>(y) *
                                 static_cast<size_t>(stridePixels);
        const uint32_t* candRow = candidate.data() +
                                  static_cast<size_t>(y) *
                                  static_cast<size_t>(width);
        for (int x = 0; x < width; ++x) {
            const uint32_t s = srcRow[x];
            const uint32_t c = candRow[x];
            const int sr = static_cast<int>(s & 0xFFu);
            const int sg = static_cast<int>((s >> 8) & 0xFFu);
            const int sb = static_cast<int>((s >> 16) & 0xFFu);
            const int sa = static_cast<int>((s >> 24) & 0xFFu);
            const int cr = static_cast<int>(c & 0xFFu);
            const int cg = static_cast<int>((c >> 8) & 0xFFu);
            const int cb = static_cast<int>((c >> 16) & 0xFFu);
            const int ca = static_cast<int>((c >> 24) & 0xFFu);
            sum += static_cast<double>(std::abs(sa - ca)) * 2.0;
            if (sa > 8 || ca > 8) {
                sum += static_cast<double>(std::abs(sr - cr) +
                                           std::abs(sg - cg) +
                                           std::abs(sb - cb));
            }
            denom += 5.0 * 255.0;
        }
    }
    return denom > 0.0 ? sum / denom : 1.0;
}

void BuildSourceSignature(const uint32_t* source,
                          int width,
                          int height,
                          int stridePixels,
                          int signatureWidth,
                          int signatureHeight,
                          std::vector<uint32_t>& out) {
    out.clear();
    if (!source || width <= 0 || height <= 0 || stridePixels <= 0 ||
        signatureWidth <= 0 || signatureHeight <= 0) {
        return;
    }

    out.resize(static_cast<size_t>(signatureWidth) *
               static_cast<size_t>(signatureHeight));
    for (int sy = 0; sy < signatureHeight; ++sy) {
        const int y0 = (sy * height) / signatureHeight;
        const int y1 = ((sy + 1) * height + signatureHeight - 1) /
                       signatureHeight;
        for (int sx = 0; sx < signatureWidth; ++sx) {
            const int x0 = (sx * width) / signatureWidth;
            const int x1 = ((sx + 1) * width + signatureWidth - 1) /
                           signatureWidth;
            uint64_t r = 0;
            uint64_t g = 0;
            uint64_t b = 0;
            uint64_t a = 0;
            uint64_t count = 0;
            for (int y = y0; y < (std::max)(y0 + 1, y1) && y < height; ++y) {
                const uint32_t* row = source +
                                      static_cast<size_t>(y) *
                                          static_cast<size_t>(stridePixels);
                for (int x = x0; x < (std::max)(x0 + 1, x1) && x < width; ++x) {
                    const uint32_t px = row[x];
                    r += px & 0xFFu;
                    g += (px >> 8) & 0xFFu;
                    b += (px >> 16) & 0xFFu;
                    a += (px >> 24) & 0xFFu;
                    ++count;
                }
            }
            if (count == 0u) {
                continue;
            }
            const uint32_t rr = static_cast<uint32_t>(r / count);
            const uint32_t gg = static_cast<uint32_t>(g / count);
            const uint32_t bb = static_cast<uint32_t>(b / count);
            const uint32_t aa = static_cast<uint32_t>(a / count);
            out[static_cast<size_t>(sy) * static_cast<size_t>(signatureWidth) +
                static_cast<size_t>(sx)] =
                rr | (gg << 8) | (bb << 16) | (aa << 24);
        }
    }
}

bool EnsureIndex(PrGameContext& ctx) {
    if (!ctx.stage1TextureReplacements) {
        return false;
    }
    if (s_cache.indexed) {
        return !s_cache.keyMap.empty();
    }

    const std::filesystem::path directory = ResolveReplacementDirectory(ctx);
    ResetCacheContents();
    s_cache.indexed = true;
    s_cache.directory = directory;

    std::error_code ec;
    if (!std::filesystem::is_directory(directory, ec)) {
        Log::Printf("Stage1TextureReplacements: directory not found '%s'",
                    directory.u8string().c_str());
        return false;
    }

    LoadSourceKeyMap();
    int preloaded = 0;
    for (ReplacementEntry& entry : s_cache.entries) {
        if (EnsureTextureLoaded(ctx, entry)) {
            ++preloaded;
        }
    }
    if (!s_cache.entries.empty()) {
        Log::Printf("Stage1TextureReplacements: preloaded %d/%llu replacement texture(s)",
                    preloaded,
                    static_cast<unsigned long long>(s_cache.entries.size()));
    }
    return !s_cache.keyMap.empty();
}

int FindBestEntry(const uint32_t* rgba,
                  int width,
                  int height,
                  int stridePixels,
                  const std::vector<size_t>& candidates,
                  double& outScore) {
    const int signatureWidth = (std::min)(kSignatureMaxSize, width);
    const int signatureHeight = (std::min)(kSignatureMaxSize, height);
    std::vector<uint32_t> sourceSignature;
    BuildSourceSignature(rgba,
                         width,
                         height,
                         stridePixels,
                         signatureWidth,
                         signatureHeight,
                         sourceSignature);
    if (sourceSignature.empty()) {
        outScore = 1.0;
        return -1;
    }

    int bestIndex = -1;
    double bestScore = 1.0;
    double secondScore = 1.0;

    for (size_t index : candidates) {
        if (index >= s_cache.entries.size()) {
            continue;
        }
        ReplacementEntry& entry = s_cache.entries[index];
        if (!EnsureSampleLoaded(entry)) {
            continue;
        }
        if (entry.sampleWidth != signatureWidth ||
            entry.sampleHeight != signatureHeight) {
            continue;
        }
        const double score = ImageDifference(sourceSignature.data(),
                                             signatureWidth,
                                             entry.sampleRgba,
                                             signatureWidth,
                                             signatureHeight);
        if (score < bestScore) {
            secondScore = bestScore;
            bestScore = score;
            bestIndex = static_cast<int>(index);
        } else if (score < secondScore) {
            secondScore = score;
        }
    }

    outScore = bestScore;
    if (bestIndex < 0) {
        return -1;
    }

    const bool unique = candidates.size() == 1u;
    const double threshold = unique ? 0.46 : 0.40;
    const bool hasMargin = unique || bestScore + 0.02 <= secondScore ||
                           bestScore <= secondScore * 0.86;
    if (bestScore > threshold || !hasMargin) {
        return -1;
    }
    return bestIndex;
}

uint64_t MakePathLogHash(const std::filesystem::path& path) {
    const std::string s = path.u8string();
    uint64_t h = 1469598103934665603ULL;
    for (unsigned char ch : s) {
        h ^= static_cast<uint64_t>(ch);
        h *= 1099511628211ULL;
    }
    return h;
}

}  // namespace

uint64_t HashRgba(const uint32_t* rgba,
                  int width,
                  int height,
                  int stridePixels) {
    if (!rgba || width <= 0 || height <= 0 || stridePixels <= 0) {
        return 0;
    }
    uint64_t h = 1469598103934665603ULL;
    for (int y = 0; y < height; ++y) {
        const uint32_t* row = rgba + static_cast<size_t>(y) *
                              static_cast<size_t>(stridePixels);
        const unsigned char* bytes =
            reinterpret_cast<const unsigned char*>(row);
        const size_t byteCount = static_cast<size_t>(width) * 4u;
        for (size_t i = 0; i < byteCount; ++i) {
            h ^= static_cast<uint64_t>(bytes[i]);
            h *= 1099511628211ULL;
        }
    }
    return h;
}

void DebugLogSourceKeyLookup(const PrGameContext& ctx,
                             uint64_t sourceKey,
                             const char* kind,
                             const char* detail,
                             bool hit) {
    if (!ctx.debugStage1TextureReplacementTrace) {
        return;
    }
    int& counter = hit ? s_cache.loggedHits : s_cache.loggedMisses;
    const int limit = hit ? 120 : 360;
    if (counter >= limit) {
        return;
    }
    Log::Printf("Stage1TextureReplacements: lookup %s kind=%s source=%s %s",
                hit ? "hit" : "miss",
                kind != nullptr ? kind : "?",
                ToHex64(sourceKey).c_str(),
                detail != nullptr ? detail : "");
    ++counter;
}

bool TryFindReplacement(PrGameContext& ctx,
                        const uint32_t* rgba,
                        int width,
                        int height,
                        int stridePixels,
                        uint64_t sourceKey,
                        ReplacementTexture& out) {
    (void)sourceKey;
    out = ReplacementTexture{};
    if (!ctx.stage1TextureReplacements || !ctx.renderer || !rgba ||
        width <= 0 || height <= 0 || stridePixels < width) {
        return false;
    }
    if (!EnsureIndex(ctx)) {
        return false;
    }

    const uint64_t rgbaHash = HashRgba(rgba, width, height, stridePixels);
    const uint64_t sourceLookupKey = SourceMapKey(rgbaHash, width, height);
    auto sourceIt = s_cache.sourceMap.find(sourceLookupKey);
    if (sourceIt == s_cache.sourceMap.end()) {
        return false;
    }
    if (sourceIt->second >= s_cache.entries.size()) {
        return false;
    }
    ReplacementEntry& entry = s_cache.entries[sourceIt->second];
    if (!EnsureTextureLoaded(ctx, entry)) {
        return false;
    }

    out.srv = entry.srv;
    out.sourceWidth = entry.sourceWidth;
    out.sourceHeight = entry.sourceHeight;
    out.imageWidth = entry.imageWidth;
    out.imageHeight = entry.imageHeight;
    const auto paramsIt = s_cache.keyParams.find(sourceKey);
    if (paramsIt != s_cache.keyParams.end()) {
        ApplyParamsToReplacement(paramsIt->second, out);
    }
    return out.srv != nullptr;
}

bool TryFindReplacementBySourceKey(PrGameContext& ctx,
                                   uint64_t sourceKey,
                                   ReplacementTexture& out) {
    out = ReplacementTexture{};
    if (!ctx.stage1TextureReplacements || !ctx.renderer) {
        return false;
    }
    if (!EnsureIndex(ctx)) {
        return false;
    }
    if (s_cache.keyMissCache.find(sourceKey) != s_cache.keyMissCache.end()) {
        return false;
    }

    auto it = s_cache.keyMap.find(sourceKey);
    if (it == s_cache.keyMap.end() || it->second >= s_cache.entries.size()) {
        s_cache.keyMissCache.insert(sourceKey);
        return false;
    }

    ReplacementEntry& entry = s_cache.entries[it->second];
    if (!EnsureTextureLoaded(ctx, entry)) {
        return false;
    }

    out.srv = entry.srv;
    out.sourceWidth = entry.sourceWidth;
    out.sourceHeight = entry.sourceHeight;
    out.imageWidth = entry.imageWidth;
    out.imageHeight = entry.imageHeight;
    return out.srv != nullptr;
}

bool TryFindReplacementByAtlasRect(PrGameContext& ctx,
                                   uint16_t tpage,
                                   uint16_t clut,
                                   int minU,
                                   int minV,
                                   int maxU,
                                   int maxV,
                                   ReplacementTexture& out,
                                   int& outSourceX,
                                   int& outSourceY,
                                   int& outSourceW,
                                   int& outSourceH) {
    out = ReplacementTexture{};
    outSourceX = 0;
    outSourceY = 0;
    outSourceW = 0;
    outSourceH = 0;
    if (!ctx.stage1TextureReplacements || !ctx.renderer ||
        minU > maxU || minV > maxV) {
        return false;
    }
    if (!EnsureIndex(ctx) || s_cache.atlasRects.empty()) {
        return false;
    }

    const uint16_t normalizedTpage = NormalizeTpageTextureKey(tpage);
    const AtlasSourceRect* best = nullptr;
    int bestArea = INT_MAX;
    for (const AtlasSourceRect& rect : s_cache.atlasRects) {
        if (rect.tpage != normalizedTpage || rect.clut != clut ||
            rect.entryIndex >= s_cache.entries.size()) {
            continue;
        }
        const int right = rect.x + rect.w - 1;
        const int bottom = rect.y + rect.h - 1;
        if (minU < rect.x || minV < rect.y ||
            maxU > right || maxV > bottom) {
            continue;
        }
        const int area = rect.w * rect.h;
        if (area < bestArea) {
            best = &rect;
            bestArea = area;
        }
    }
    if (best == nullptr) {
        return false;
    }

    ReplacementEntry& entry = s_cache.entries[best->entryIndex];
    if (!EnsureTextureLoaded(ctx, entry)) {
        return false;
    }

    out.srv = entry.srv;
    out.sourceWidth = entry.sourceWidth;
    out.sourceHeight = entry.sourceHeight;
    out.imageWidth = entry.imageWidth;
    out.imageHeight = entry.imageHeight;
    ApplyParamsToReplacement(best->params, out);
    outSourceX = best->x;
    outSourceY = best->y;
    outSourceW = best->w;
    outSourceH = best->h;
    return out.srv != nullptr;
}

void Prime(PrGameContext& ctx) {
    (void)EnsureIndex(ctx);
}

void Reset() {
    ResetCacheContents();
}

}  // namespace PrStage1TextureReplacements
