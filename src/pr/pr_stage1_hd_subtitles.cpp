#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include "pr_stage1_hd_subtitles.h"

#include "d3d11_renderer.h"
#include "logger.h"
#include "pr_game_context.h"
#include "pr_scn1.h"
#include "pr_sqevs1.h"
#include "pr_ss0_scene0_runtime_direct.h"

#include <windows.h>

#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <limits>
#include <sstream>
#include <string>
#include <system_error>
#include <vector>

namespace PrStage1HdSubtitles {
namespace {

enum class Scope : uint8_t {
    Any = 0,
    Movie1,
    CommonLyrics,
    OverlayScriptText,
};

struct SubtitleEntry {
    std::string language = "*";
    Scope scope = Scope::Any;
    int mode = -1;
    int64_t frame = -1;
    int duration = -1;
    int textIndex = std::numeric_limits<int>::min();
    int textId = -1;
    bool psxAddrSet = false;
    uint32_t psxAddr = 0;
    std::string originalText;
    std::wstring text;
};

struct SubtitleTable {
    std::filesystem::path path;
    std::filesystem::file_time_type writeTime{};
    bool loaded = false;
    bool exists = false;
    std::vector<SubtitleEntry> entries;
};

struct TextTextureEntry {
    std::wstring text;
    std::wstring fontFace;
    int fontPx = 0;
    int widthPx = 0;
    int outlinePx = 0;
    int shadowOffsetXPx = 0;
    int shadowOffsetYPx = 0;
    uint32_t fillColor = 0xFFFFFFFFu;
    uint32_t outlineColor = 0xFF000000u;
    uint32_t shadowColor = 0xFF6F6F6Fu;
    ID3D11ShaderResourceView* srv = nullptr;
    D3D11Renderer* renderer = nullptr;
    int w = 0;
    int h = 0;
};

struct NativeSubtitleTextAnchor {
    bool valid = false;
    uint32_t frame = UINT32_MAX;
    PrStage1HdSubtitleSourceKind kind = PrStage1HdSubtitleSourceKind::None;
    float minX = 0.0f;
    float minY = 0.0f;
    float maxX = 0.0f;
    float maxY = 0.0f;
    uint32_t rectCount = 0;
};

struct StableNativeSubtitleTextAnchor {
    bool valid = false;
    uint32_t lastFrame = UINT32_MAX;
    PrStage1HdSubtitleSourceKind kind = PrStage1HdSubtitleSourceKind::None;
    std::string key;
    NativeSubtitleTextAnchor anchor{};
};

struct SubtitleRenderMetrics {
    float vx = 0.0f;
    float vy = 0.0f;
    float vs = 1.0f;
    std::wstring fontFace;
    int fontPx = 0;
    int widthPx = 0;
    int outlinePx = 0;
    int shadowOffsetXPx = 0;
    int shadowOffsetYPx = 0;
    uint32_t fillColor = 0xFFFFFFFFu;
    uint32_t outlineColor = 0xFF000000u;
    uint32_t shadowColor = 0xFF6F6F6Fu;
};

struct PreloadSignature {
    bool valid = false;
    D3D11Renderer* renderer = nullptr;
    std::filesystem::path path;
    std::filesystem::file_time_type writeTime{};
    std::string language;
    std::wstring fontFace;
    int fontPx = 0;
    int widthPx = 0;
    int outlinePx = 0;
    int shadowOffsetXPx = 0;
    int shadowOffsetYPx = 0;
    uint32_t fillColor = 0;
    uint32_t outlineColor = 0;
    uint32_t shadowColor = 0;
    uint32_t ready = 0;
    uint32_t requested = 0;
};

static SubtitleTable s_table;
static bool s_missingLogged = false;
static std::vector<TextTextureEntry> s_textCache;
static bool s_suppressCacheValid = false;
static uint32_t s_suppressCacheFrame = UINT32_MAX;
static bool s_suppressCacheValue = false;
static std::array<NativeSubtitleTextAnchor, 4> s_nativeTextAnchors;
static std::array<StableNativeSubtitleTextAnchor, 4>
    s_stableNativeTextAnchors;
static PreloadSignature s_preloadSignature;

static size_t NativeAnchorIndex(PrStage1HdSubtitleSourceKind kind) {
    switch (kind) {
    case PrStage1HdSubtitleSourceKind::Movie1:
        return 1u;
    case PrStage1HdSubtitleSourceKind::CommonLyrics:
        return 2u;
    case PrStage1HdSubtitleSourceKind::OverlayScriptText:
        return 3u;
    default:
        return 0u;
    }
}

static void ResetNativeAnchor(NativeSubtitleTextAnchor& anchor,
                              uint32_t frame,
                              PrStage1HdSubtitleSourceKind kind) {
    anchor.valid = false;
    anchor.frame = frame;
    anchor.kind = kind;
    anchor.minX = 0.0f;
    anchor.minY = 0.0f;
    anchor.maxX = 0.0f;
    anchor.maxY = 0.0f;
    anchor.rectCount = 0;
}

static bool TryGetNativeSubtitleAnchorForKind(
    PrGameContext& ctx,
    PrStage1HdSubtitleSourceKind kind,
    NativeSubtitleTextAnchor& out) {
    const NativeSubtitleTextAnchor& anchor =
        s_nativeTextAnchors[NativeAnchorIndex(kind)];
    if (anchor.valid && anchor.frame == ctx.frame &&
        anchor.rectCount != 0u) {
        out = anchor;
        return true;
    }
    return false;
}

static bool TryGetNativeSubtitleAnchor(PrGameContext& ctx,
                                       PrStage1HdSubtitleSourceKind kind,
                                       NativeSubtitleTextAnchor& out) {
    if (TryGetNativeSubtitleAnchorForKind(ctx, kind, out)) {
        return true;
    }
    if (kind == PrStage1HdSubtitleSourceKind::Movie1) {
        return TryGetNativeSubtitleAnchorForKind(
            ctx, PrStage1HdSubtitleSourceKind::CommonLyrics, out);
    }
    if (kind == PrStage1HdSubtitleSourceKind::CommonLyrics) {
        return TryGetNativeSubtitleAnchorForKind(
            ctx, PrStage1HdSubtitleSourceKind::Movie1, out);
    }
    return false;
}

static bool HasNativeSubtitleTextThisFrame(
    PrGameContext& ctx,
    PrStage1HdSubtitleSourceKind kind) {
    NativeSubtitleTextAnchor anchor{};
    return TryGetNativeSubtitleAnchor(ctx, kind, anchor);
}

static float AnchorArea(const NativeSubtitleTextAnchor& anchor) {
    if (!anchor.valid) {
        return 0.0f;
    }
    const float w = (std::max)(0.0f, anchor.maxX - anchor.minX);
    const float h = (std::max)(0.0f, anchor.maxY - anchor.minY);
    return w * h;
}

static bool IsStableAnchorContinuation(
    const StableNativeSubtitleTextAnchor& stable,
    uint32_t frame,
    PrStage1HdSubtitleSourceKind kind,
    const std::string& key) {
    if (!stable.valid || stable.kind != kind || stable.key != key) {
        return false;
    }
    return stable.lastFrame == frame ||
           (stable.lastFrame != UINT32_MAX &&
            stable.lastFrame + 1u == frame);
}

static void ResetStableNativeSubtitleAnchor(
    StableNativeSubtitleTextAnchor& stable,
    uint32_t frame,
    PrStage1HdSubtitleSourceKind kind,
    const std::string& key,
    const NativeSubtitleTextAnchor& observed) {
    stable.valid = true;
    stable.lastFrame = frame;
    stable.kind = kind;
    stable.key = key;
    stable.anchor = observed;
    stable.anchor.frame = frame;
}

static bool TryGetStableNativeSubtitleAnchor(
    PrGameContext& ctx,
    PrStage1HdSubtitleSourceKind kind,
    const std::string& key,
    NativeSubtitleTextAnchor& out) {
    NativeSubtitleTextAnchor observed{};
    if (!TryGetNativeSubtitleAnchor(ctx, kind, observed)) {
        return false;
    }

    StableNativeSubtitleTextAnchor& stable =
        s_stableNativeTextAnchors[NativeAnchorIndex(kind)];
    if (!IsStableAnchorContinuation(stable, ctx.frame, kind, key)) {
        ResetStableNativeSubtitleAnchor(stable,
                                        ctx.frame,
                                        kind,
                                        key,
                                        observed);
        out = stable.anchor;
        return true;
    }

    const float observedArea = AnchorArea(observed);
    const float stableArea = AnchorArea(stable.anchor);
    if (observed.rectCount >= stable.anchor.rectCount ||
        observedArea >= stableArea) {
        stable.anchor.minX = (std::min)(stable.anchor.minX, observed.minX);
        stable.anchor.minY = (std::min)(stable.anchor.minY, observed.minY);
        stable.anchor.maxX = (std::max)(stable.anchor.maxX, observed.maxX);
        stable.anchor.maxY = (std::max)(stable.anchor.maxY, observed.maxY);
        stable.anchor.rectCount =
            (std::max)(stable.anchor.rectCount, observed.rectCount);
    }
    stable.anchor.frame = ctx.frame;
    stable.lastFrame = ctx.frame;
    out = stable.anchor;
    return true;
}

static std::string Trim(std::string s) {
    const auto isWs = [](unsigned char c) {
        return c == ' ' || c == '\t' || c == '\r' || c == '\n';
    };
    while (!s.empty() && isWs((unsigned char)s.front())) {
        s.erase(s.begin());
    }
    while (!s.empty() && isWs((unsigned char)s.back())) {
        s.pop_back();
    }
    return s;
}

static std::string ToLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {
        return (char)std::tolower(c);
    });
    return s;
}

static std::string ToUpperAscii(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {
        return (char)std::toupper(c);
    });
    return s;
}

static int HexNibble(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

static uint32_t ParseColor(std::string s, uint32_t fallback) {
    s = Trim(s);
    if (!s.empty() && s.front() == '#') {
        s.erase(s.begin());
    }
    if (s.size() != 6u && s.size() != 8u) {
        return fallback;
    }

    uint32_t value = 0;
    for (char c : s) {
        const int n = HexNibble(c);
        if (n < 0) {
            return fallback;
        }
        value = (value << 4) | static_cast<uint32_t>(n);
    }

    if (s.size() == 6u) {
        return 0xFF000000u | value;
    }
    return value;
}

static uint32_t RgbaPackedFromArgb(uint32_t argb, uint8_t alpha) {
    const uint32_t configuredA = (argb >> 24) & 0xFFu;
    const uint32_t a = (configuredA * static_cast<uint32_t>(alpha)) / 255u;
    const uint32_t r = (argb >> 16) & 0xFFu;
    const uint32_t g = (argb >> 8) & 0xFFu;
    const uint32_t b = argb & 0xFFu;
    return (a << 24) | (b << 16) | (g << 8) | r;
}

static uint32_t CompositeArgbOverRgba(uint32_t dstRgba,
                                      uint32_t srcArgb,
                                      uint8_t maskAlpha) {
    const uint32_t srcPacked = RgbaPackedFromArgb(srcArgb, maskAlpha);
    const uint32_t srcA = (srcPacked >> 24) & 0xFFu;
    if (srcA == 0u) {
        return dstRgba;
    }
    if (srcA == 255u) {
        return srcPacked;
    }

    const uint32_t dstA = (dstRgba >> 24) & 0xFFu;
    const uint32_t invA = 255u - srcA;
    const uint32_t outA = srcA + (dstA * invA + 127u) / 255u;
    if (outA == 0u) {
        return 0u;
    }

    const uint32_t srcR = srcPacked & 0xFFu;
    const uint32_t srcG = (srcPacked >> 8) & 0xFFu;
    const uint32_t srcB = (srcPacked >> 16) & 0xFFu;
    const uint32_t dstR = dstRgba & 0xFFu;
    const uint32_t dstG = (dstRgba >> 8) & 0xFFu;
    const uint32_t dstB = (dstRgba >> 16) & 0xFFu;

    const auto overChannel = [&](uint32_t s, uint32_t d) -> uint32_t {
        const uint32_t premul =
            s * srcA + (d * dstA * invA + 127u) / 255u;
        return (premul + outA / 2u) / outA;
    };

    const uint32_t outR = overChannel(srcR, dstR);
    const uint32_t outG = overChannel(srcG, dstG);
    const uint32_t outB = overChannel(srcB, dstB);
    return (outA << 24) | (outB << 16) | (outG << 8) | outR;
}

static std::wstring Utf8ToWide(const std::string& s) {
    if (s.empty()) {
        return {};
    }
    const int needed = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
                                           s.data(), (int)s.size(),
                                           nullptr, 0);
    if (needed <= 0) {
        return std::wstring(s.begin(), s.end());
    }
    std::wstring out((size_t)needed, L'\0');
    MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
                        s.data(), (int)s.size(),
                        out.data(), needed);
    return out;
}

static std::string NormalizeLanguage(std::string s) {
    s = ToLower(Trim(s));
    if (s.empty() || s == "*") return "*";
    if (s == "auto") return "auto";
    if (s == "cn" || s == "zh" || s == "zh-cn" || s == "chinese") return "CN";
    if (s == "fr" || s == "french") return "FR";
    if (s == "de" || s == "ger" || s == "german") return "DE";
    if (s == "es" || s == "sp" || s == "spanish") return "ES";
    if (s == "it" || s == "italian") return "IT";
    if (s == "en" || s == "english") return "EN";
    return ToUpperAscii(s);
}

static std::string ResolveContextLanguage(const PrGameContext& ctx) {
    const std::string configured = NormalizeLanguage(ctx.stage1HdSubtitleLanguage);
    if (configured != "auto") {
        return configured;
    }
    static const char* kOriginalLangs[5] = {"EN", "FR", "DE", "ES", "IT"};
    const int idx = (ctx.languageIndex >= 0 && ctx.languageIndex < 5)
        ? ctx.languageIndex
        : 0;
    return kOriginalLangs[idx];
}

static Scope ParseScope(std::string s) {
    s = ToLower(Trim(s));
    if (s == "*" || s == "any" || s.empty()) return Scope::Any;
    if (s == "movie" || s == "movie1" || s == "mov1") return Scope::Movie1;
    if (s == "common" || s == "lyrics" || s == "commonlyrics") {
        return Scope::CommonLyrics;
    }
    if (s == "script" || s == "gameplay" || s == "overlay" ||
        s == "steady") {
        return Scope::OverlayScriptText;
    }
    return Scope::Any;
}

static Scope ScopeFromSource(PrStage1HdSubtitleSourceKind kind) {
    switch (kind) {
    case PrStage1HdSubtitleSourceKind::Movie1:
        return Scope::Movie1;
    case PrStage1HdSubtitleSourceKind::CommonLyrics:
        return Scope::CommonLyrics;
    case PrStage1HdSubtitleSourceKind::OverlayScriptText:
        return Scope::OverlayScriptText;
    default:
        return Scope::Any;
    }
}

static bool ParseIntField(const std::string& s, int& out) {
    const std::string v = Trim(s);
    if (v.empty() || v == "*") {
        out = -1;
        return true;
    }
    char* end = nullptr;
    const long n = std::strtol(v.c_str(), &end, 0);
    if (end == v.c_str() || (end && *end != '\0')) {
        return false;
    }
    out = (int)n;
    return true;
}

static bool ParseTextIndexField(const std::string& s, int& out) {
    const std::string v = Trim(s);
    if (v.empty() || v == "*") {
        out = std::numeric_limits<int>::min();
        return true;
    }
    char* end = nullptr;
    const long n = std::strtol(v.c_str(), &end, 0);
    if (end == v.c_str() || (end && *end != '\0')) {
        return false;
    }
    out = (int)n;
    return true;
}

static bool ParseFrameField(const std::string& s, int64_t& out) {
    const std::string v = Trim(s);
    if (v.empty() || v == "*") {
        out = -1;
        return true;
    }
    char* end = nullptr;
    const long long n = std::strtoll(v.c_str(), &end, 0);
    if (end == v.c_str() || (end && *end != '\0')) {
        return false;
    }
    out = n;
    return true;
}

static bool ParsePsxAddrField(const std::string& s,
                              bool& set,
                              uint32_t& out) {
    const std::string v = Trim(s);
    if (v.empty() || v == "*" || v == "0") {
        set = false;
        out = 0;
        return true;
    }
    char* end = nullptr;
    const unsigned long n = std::strtoul(v.c_str(), &end, 0);
    if (end == v.c_str() || (end && *end != '\0')) {
        return false;
    }
    set = true;
    out = (uint32_t)n;
    return true;
}

static std::string UnescapeText(std::string s) {
    std::string out;
    out.reserve(s.size());
    for (size_t i = 0; i < s.size(); ++i) {
        if (s[i] != '\\' || i + 1 >= s.size()) {
            out.push_back(s[i]);
            continue;
        }
        const char c = s[++i];
        switch (c) {
        case 'n': out.push_back('\n'); break;
        case 'r': out.push_back('\r'); break;
        case 't': out.push_back('\t'); break;
        case '\\': out.push_back('\\'); break;
        default:
            out.push_back('\\');
            out.push_back(c);
            break;
        }
    }
    return out;
}

static std::string NormalizeMatchText(std::string s) {
    s = UnescapeText(std::move(s));
    s.erase(std::remove(s.begin(), s.end(), '\r'), s.end());
    return Trim(std::move(s));
}

static std::string BuildAnchorKey(
    const PrStage1HdSubtitleRuntimeSource& source) {
    std::ostringstream ss;
    ss << static_cast<int>(source.kind)
       << ':' << static_cast<int>(source.mode)
       << ':' << source.eventFrame
       << ':' << source.durationFrames
       << ':' << source.textIndex
       << ':' << static_cast<int>(source.textId)
       << ':' << source.psxAddr;
    if (source.originalText != nullptr) {
        ss << ':' << NormalizeMatchText(source.originalText);
    }
    return ss.str();
}

static std::vector<std::string> SplitTsvPreserveTail(const std::string& line,
                                                     int prefixColumns) {
    std::vector<std::string> fields;
    fields.reserve((size_t)prefixColumns + 1u);
    size_t pos = 0;
    for (int i = 0; i < prefixColumns; ++i) {
        const size_t tab = line.find('\t', pos);
        if (tab == std::string::npos) {
            fields.push_back(line.substr(pos));
            return fields;
        }
        fields.push_back(line.substr(pos, tab - pos));
        pos = tab + 1;
    }
    fields.push_back(line.substr(pos));
    return fields;
}

static int CountTabs(const std::string& line) {
    return (int)std::count(line.begin(), line.end(), '\t');
}

static bool ParseEntryLine(const std::string& rawLine, SubtitleEntry& out) {
    out = SubtitleEntry{};
    std::string line = rawLine;
    if (!line.empty() && line.back() == '\r') {
        line.pop_back();
    }
    if (Trim(line).empty()) {
        return false;
    }

    const bool hasOriginalColumns = CountTabs(line) >= 10;
    std::vector<std::string> fields =
        SplitTsvPreserveTail(line, hasOriginalColumns ? 10 : 8);
    if (fields.size() >= 2) {
        const std::string first = ToLower(Trim(fields[0]));
        if (first == "language" || first == "scope") {
            return false;
        }
    }

    int base = 0;
    if (fields.size() >= 9) {
        out.language = NormalizeLanguage(fields[0]);
        base = 1;
    } else if (fields.size() >= 8) {
        out.language = "*";
        base = 0;
    } else {
        return false;
    }

    out.scope = ParseScope(fields[(size_t)base + 0u]);
    if (!ParseIntField(fields[(size_t)base + 1u], out.mode)) return false;
    if (!ParseFrameField(fields[(size_t)base + 2u], out.frame)) return false;
    if (!ParseIntField(fields[(size_t)base + 3u], out.duration)) return false;
    if (!ParseTextIndexField(fields[(size_t)base + 4u], out.textIndex)) return false;
    if (!ParseIntField(fields[(size_t)base + 5u], out.textId)) return false;
    if (!ParsePsxAddrField(fields[(size_t)base + 6u],
                           out.psxAddrSet,
                           out.psxAddr)) {
        return false;
    }

    const size_t textField = hasOriginalColumns ? 10u : ((size_t)base + 7u);
    if (textField >= fields.size()) {
        return false;
    }
    if (hasOriginalColumns && fields.size() > 9u) {
        out.originalText = NormalizeMatchText(fields[9]);
    }
    const std::string text = UnescapeText(fields[textField]);
    out.text = Utf8ToWide(text);
    return true;
}

static std::filesystem::path WeakCanonicalPath(const std::filesystem::path& p) {
    std::error_code ec;
    std::filesystem::path out = std::filesystem::weakly_canonical(p, ec);
    return ec ? p : out;
}

static std::filesystem::path ResolveSubtitlePath(const PrGameContext& ctx) {
    std::filesystem::path configured = ctx.stage1HdSubtitleFile;
    if (configured.empty()) {
        configured = "ex/subtitles/stage1_hd_zh.tsv";
    }
    if (configured.is_absolute()) {
        return WeakCanonicalPath(configured);
    }

    std::vector<std::filesystem::path> bases;
    if (!ctx.dataRoot.empty()) {
        bases.push_back(ctx.dataRoot);
        bases.push_back(ctx.dataRoot / "win");
    }

    std::error_code ec;
    const std::filesystem::path cwd = std::filesystem::current_path(ec);
    if (!ec) {
        bases.push_back(cwd);
        bases.push_back(cwd / "win");
    }

    for (const std::filesystem::path& base : bases) {
        const std::filesystem::path candidate = WeakCanonicalPath(base / configured);
        if (std::filesystem::exists(candidate, ec) && !ec) {
            return candidate;
        }
        ec.clear();
    }

    return bases.empty() ? WeakCanonicalPath(configured)
                         : WeakCanonicalPath(bases.front() / configured);
}

static bool LoadSubtitleTable(const std::filesystem::path& path,
                              SubtitleTable& out) {
    out = SubtitleTable{};
    out.path = path;
    std::error_code ec;
    out.exists = std::filesystem::exists(path, ec) && !ec;
    if (!out.exists) {
        return false;
    }
    out.writeTime = std::filesystem::last_write_time(path, ec);

    std::ifstream fs(path, std::ios::binary);
    if (!fs.is_open()) {
        return false;
    }

    std::string line;
    uint32_t lineNo = 0;
    while (std::getline(fs, line)) {
        ++lineNo;
        std::string trimmed = Trim(line);
        if (trimmed.empty() || trimmed[0] == '#' || trimmed[0] == ';') {
            continue;
        }
        if (trimmed.size() >= 3 &&
            (uint8_t)trimmed[0] == 0xEF &&
            (uint8_t)trimmed[1] == 0xBB &&
            (uint8_t)trimmed[2] == 0xBF) {
            trimmed.erase(0, 3);
        }
        const std::string firstColumn =
            ToLower(Trim(trimmed.substr(0, trimmed.find('\t'))));
        if (firstColumn == "language" || firstColumn == "scope") {
            continue;
        }

        SubtitleEntry entry{};
        if (ParseEntryLine(trimmed, entry)) {
            if (entry.text.empty()) {
                continue;
            }
            out.entries.push_back(std::move(entry));
        } else {
            Log::Printf("Stage1HdSubtitles: ignored malformed row %u in %s",
                        (unsigned)lineNo,
                        path.u8string().c_str());
        }
    }

    out.loaded = true;
    Log::Printf("Stage1HdSubtitles: loaded %u row(s) from %s",
                (unsigned)out.entries.size(),
                path.u8string().c_str());
    return true;
}

static const SubtitleTable& EnsureSubtitleTable(const PrGameContext& ctx) {
    const std::filesystem::path path = ResolveSubtitlePath(ctx);
    std::error_code ec;
    const bool exists = std::filesystem::exists(path, ec) && !ec;
    const std::filesystem::file_time_type writeTime =
        exists ? std::filesystem::last_write_time(path, ec)
               : std::filesystem::file_time_type{};

    if (s_table.loaded &&
        s_table.path == path &&
        s_table.exists == exists &&
        (!exists || s_table.writeTime == writeTime)) {
        return s_table;
    }

    if (!exists) {
        s_table = SubtitleTable{};
        s_table.path = path;
        s_table.loaded = true;
        s_table.exists = false;
        if (!s_missingLogged) {
            Log::Printf("Stage1HdSubtitles: sidecar not found: %s",
                        path.u8string().c_str());
            s_missingLogged = true;
        }
        return s_table;
    }

    s_missingLogged = false;
    (void)LoadSubtitleTable(path, s_table);
    return s_table;
}

static int MatchEntry(const SubtitleEntry& entry,
                      const PrStage1HdSubtitleRuntimeSource& source,
                      Scope sourceScope,
                      const std::string& language,
                      const std::string* normalizedOriginalText) {
    if (!(entry.language == "*" || entry.language == language)) {
        return -1;
    }
    int score = (entry.language == language) ? 128 : 1;
    const bool originalTextMatches =
        !entry.originalText.empty() &&
        normalizedOriginalText != nullptr &&
        entry.originalText == *normalizedOriginalText;
    if (originalTextMatches) {
        score += 96;
    }

    if (entry.scope != Scope::Any) {
        if (entry.scope != sourceScope) {
            return -1;
        }
        score += 64;
    }

    if (entry.mode >= 0) {
        if (source.mode != (uint8_t)entry.mode) {
            return -1;
        }
        score += 16;
    }

    if (entry.frame >= 0) {
        const uint32_t f = static_cast<uint32_t>(entry.frame);
        if (source.eventFrame != f && source.queryFrame != f) {
            if (!originalTextMatches) {
                return -1;
            }
        } else {
            score += (source.eventFrame == f) ? 32 : 8;
        }
    }

    if (entry.duration >= 0 &&
        !(entry.psxAddrSet || entry.textId >= 0)) {
        if (source.durationFrames != (uint16_t)entry.duration) {
            return -1;
        }
        score += 4;
    } else if (entry.duration >= 0 &&
               source.durationFrames == (uint16_t)entry.duration) {
        score += 4;
    }

    if (entry.textIndex != std::numeric_limits<int>::min()) {
        if (source.textIndex != (int16_t)entry.textIndex) {
            return -1;
        }
        score += 16;
    }

    if (entry.textId >= 0) {
        if (source.textId != (uint8_t)entry.textId) {
            if (!originalTextMatches) {
                return -1;
            }
        } else {
            score += 24;
        }
    }

    if (entry.psxAddrSet && source.psxAddr != 0u) {
        if (source.psxAddr != entry.psxAddr) {
            if (!originalTextMatches) {
                return -1;
            }
        } else {
            score += 16;
        }
    }

    return score;
}

static const std::wstring* FindSubtitleText(
    const SubtitleTable& table,
    const PrStage1HdSubtitleRuntimeSource& source,
    const std::string& language) {
    if (!table.loaded || table.entries.empty() || !source.valid) {
        return nullptr;
    }

    const Scope sourceScope = ScopeFromSource(source.kind);
    std::string normalizedOriginalText;
    const std::string* normalizedOriginalTextPtr = nullptr;
    if (source.originalText != nullptr && source.originalText[0] != '\0') {
        normalizedOriginalText = NormalizeMatchText(source.originalText);
        normalizedOriginalTextPtr = &normalizedOriginalText;
    }
    const SubtitleEntry* best = nullptr;
    int bestScore = -1;
    for (const SubtitleEntry& entry : table.entries) {
        const int score = MatchEntry(entry,
                                     source,
                                     sourceScope,
                                     language,
                                     normalizedOriginalTextPtr);
        if (score > bestScore) {
            bestScore = score;
            best = &entry;
        }
    }
    return best ? &best->text : nullptr;
}

static void CalcPs1Viewport(const D3D11Renderer* renderer,
                            float& outX,
                            float& outY,
                            float& outScale) {
    outX = 0.0f;
    outY = 0.0f;
    outScale = 1.0f;
    if (!renderer) return;
    const float winW = (float)renderer->GetWidth();
    const float winH = (float)renderer->GetHeight();
    const float fitX = winW / 320.0f;
    const float fitY = winH / 240.0f;
    outScale = (std::min)(fitX, fitY);
    outX = (winW - 320.0f * outScale) * 0.5f;
    outY = (winH - 240.0f * outScale) * 0.5f;
}

static bool BuildSubtitleRenderMetrics(PrGameContext& ctx,
                                       SubtitleRenderMetrics& out) {
    out = SubtitleRenderMetrics{};
    if (!ctx.renderer) {
        return false;
    }

    CalcPs1Viewport(ctx.renderer, out.vx, out.vy, out.vs);
    const float widthPsx =
        (std::max)(80.0f, ctx.stage1HdSubtitleWidth);
    out.widthPx = (std::max)(64, (int)std::lround(widthPsx * out.vs));
    out.fontPx =
        (std::max)(8,
                   (int)std::lround(ctx.stage1HdSubtitleFontSizePsx *
                                    out.vs));
    out.fontFace = Utf8ToWide(ctx.stage1HdSubtitleFont);
    out.outlinePx =
        (std::max)(0,
                   (int)std::lround(ctx.stage1HdSubtitleOutlinePsx *
                                    out.vs));
    out.shadowOffsetXPx =
        (int)std::lround(ctx.stage1HdSubtitleShadowOffsetXPsx * out.vs);
    out.shadowOffsetYPx =
        (int)std::lround(ctx.stage1HdSubtitleShadowOffsetYPsx * out.vs);
    out.fillColor =
        ParseColor(ctx.stage1HdSubtitleFillColor, 0xFFFFFFFFu);
    out.outlineColor =
        ParseColor(ctx.stage1HdSubtitleOutlineColor, 0xFF000000u);
    out.shadowColor =
        ParseColor(ctx.stage1HdSubtitleShadowColor, 0xFF6F6F6Fu);
    return true;
}

static bool MatchesPreloadSignature(const PreloadSignature& sig,
                                    D3D11Renderer* renderer,
                                    const SubtitleTable& table,
                                    const std::string& language,
                                    const SubtitleRenderMetrics& metrics) {
    return sig.valid &&
           sig.renderer == renderer &&
           sig.path == table.path &&
           sig.writeTime == table.writeTime &&
           sig.language == language &&
           sig.fontFace == metrics.fontFace &&
           sig.fontPx == metrics.fontPx &&
           sig.widthPx == metrics.widthPx &&
           sig.outlinePx == metrics.outlinePx &&
           sig.shadowOffsetXPx == metrics.shadowOffsetXPx &&
           sig.shadowOffsetYPx == metrics.shadowOffsetYPx &&
           sig.fillColor == metrics.fillColor &&
           sig.outlineColor == metrics.outlineColor &&
           sig.shadowColor == metrics.shadowColor &&
           sig.ready == sig.requested;
}

static void StorePreloadSignature(D3D11Renderer* renderer,
                                  const SubtitleTable& table,
                                  const std::string& language,
                                  const SubtitleRenderMetrics& metrics,
                                  uint32_t ready,
                                  uint32_t requested) {
    s_preloadSignature.valid = true;
    s_preloadSignature.renderer = renderer;
    s_preloadSignature.path = table.path;
    s_preloadSignature.writeTime = table.writeTime;
    s_preloadSignature.language = language;
    s_preloadSignature.fontFace = metrics.fontFace;
    s_preloadSignature.fontPx = metrics.fontPx;
    s_preloadSignature.widthPx = metrics.widthPx;
    s_preloadSignature.outlinePx = metrics.outlinePx;
    s_preloadSignature.shadowOffsetXPx = metrics.shadowOffsetXPx;
    s_preloadSignature.shadowOffsetYPx = metrics.shadowOffsetYPx;
    s_preloadSignature.fillColor = metrics.fillColor;
    s_preloadSignature.outlineColor = metrics.outlineColor;
    s_preloadSignature.shadowColor = metrics.shadowColor;
    s_preloadSignature.ready = ready;
    s_preloadSignature.requested = requested;
}

static HFONT CreateSubtitleFont(int heightPx, const std::wstring& face) {
    return CreateFontW(-heightPx, 0, 0, 0,
                       FW_NORMAL, FALSE, FALSE, FALSE,
                       DEFAULT_CHARSET,
                       OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                       ANTIALIASED_QUALITY,
                       DEFAULT_PITCH | FF_DONTCARE,
                       face.empty() ? L"Microsoft YaHei" : face.c_str());
}

static int MeasureTextWidthPx(HDC hdc, const std::wstring& text) {
    if (!hdc || text.empty()) return 0;
    SIZE sz{};
    if (!GetTextExtentPoint32W(hdc,
                               text.c_str(),
                               (int)text.size(),
                               &sz)) {
        return 0;
    }
    return (int)sz.cx;
}

static std::vector<std::wstring> SplitLines(const std::wstring& text) {
    std::vector<std::wstring> out;
    size_t start = 0;
    while (start <= text.size()) {
        const size_t end = text.find(L'\n', start);
        std::wstring line =
            (end == std::wstring::npos)
                ? text.substr(start)
                : text.substr(start, end - start);
        if (!line.empty() && line.back() == L'\r') {
            line.pop_back();
        }
        out.push_back(std::move(line));
        if (end == std::wstring::npos) {
            break;
        }
        start = end + 1;
    }
    return out;
}

static std::vector<std::wstring> WrapText(HDC hdc,
                                          const std::wstring& text,
                                          int maxWidthPx) {
    std::vector<std::wstring> out;
    for (const std::wstring& rawLine : SplitLines(text)) {
        if (rawLine.size() <= 8u ||
            MeasureTextWidthPx(hdc, rawLine) <= maxWidthPx) {
            out.push_back(rawLine);
            continue;
        }

        std::wstring line;
        for (wchar_t ch : rawLine) {
            std::wstring next = line;
            next.push_back(ch);
            if (!line.empty() &&
                MeasureTextWidthPx(hdc, next) > maxWidthPx) {
                out.push_back(line);
                line.clear();
            }
            line.push_back(ch);
        }
        if (!line.empty() || rawLine.empty()) {
            out.push_back(line);
        }
    }
    if (out.empty()) {
        out.push_back(L"");
    }
    return out;
}

static void DestroyTextTextureEntry(TextTextureEntry& e) {
    if (e.srv && e.renderer) {
        e.renderer->DestroyTexture(e.srv);
    }
    e.srv = nullptr;
    e.renderer = nullptr;
    e.w = 0;
    e.h = 0;
}

static TextTextureEntry* FindTextTexture(const std::wstring& text,
                                         const std::wstring& fontFace,
                                         int fontPx,
                                         int widthPx,
                                         int outlinePx,
                                         int shadowOffsetXPx,
                                         int shadowOffsetYPx,
                                         uint32_t fillColor,
                                         uint32_t outlineColor,
                                         uint32_t shadowColor) {
    for (auto& e : s_textCache) {
        if (e.text == text &&
            e.fontFace == fontFace &&
            e.fontPx == fontPx &&
            e.widthPx == widthPx &&
            e.outlinePx == outlinePx &&
            e.shadowOffsetXPx == shadowOffsetXPx &&
            e.shadowOffsetYPx == shadowOffsetYPx &&
            e.fillColor == fillColor &&
            e.outlineColor == outlineColor &&
            e.shadowColor == shadowColor) {
            return &e;
        }
    }
    return nullptr;
}

static bool BuildTextTexture(TextTextureEntry& entry) {
    if (!entry.renderer || entry.text.empty() ||
        entry.fontPx <= 0 || entry.widthPx <= 0) {
        return false;
    }

    HDC hdc = CreateCompatibleDC(nullptr);
    if (!hdc) {
        return false;
    }

    HFONT font = CreateSubtitleFont(entry.fontPx, entry.fontFace);
    if (!font) {
        DeleteDC(hdc);
        return false;
    }
    HGDIOBJ oldFont = SelectObject(hdc, font);

    TEXTMETRICW tm{};
    GetTextMetricsW(hdc, &tm);
    const int outline = (std::max)(0, entry.outlinePx);
    const int shadowX = entry.shadowOffsetXPx;
    const int shadowY = entry.shadowOffsetYPx;
    const int padLeft = outline + 2 + (std::max)(0, -shadowX);
    const int padRight = outline + 2 + (std::max)(0, shadowX);
    const int padTop = outline + 1 + (std::max)(0, -shadowY);
    const int padBottom = outline + 1 + (std::max)(0, shadowY);
    int texW = entry.widthPx;
    for (const std::wstring& rawLine : SplitLines(entry.text)) {
        if (rawLine.size() <= 8u) {
            texW = (std::max)(texW,
                              MeasureTextWidthPx(hdc, rawLine) + padLeft +
                                  padRight);
        }
    }
    texW = (std::max)(1, (std::min)(4096, texW));
    const int maxTextW = (std::max)(1, texW - padLeft - padRight);
    const std::vector<std::wstring> lines = WrapText(hdc, entry.text, maxTextW);
    const int textMetricHeight = static_cast<int>(tm.tmHeight);
    const int lineStep = (std::max)(textMetricHeight + 2,
                                    (int)std::lround(entry.fontPx * 1.22f));
    const int texH =
        (std::max)(1,
                   padTop + textMetricHeight +
                       lineStep * ((int)lines.size() - 1) + padBottom);

    BITMAPINFO bmi{};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = texW;
    bmi.bmiHeader.biHeight = -texH;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    void* bits = nullptr;
    HBITMAP bmp = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &bits, nullptr, 0);
    if (!bmp || !bits) {
        if (bmp) DeleteObject(bmp);
        SelectObject(hdc, oldFont);
        DeleteObject(font);
        DeleteDC(hdc);
        return false;
    }
    HGDIOBJ oldBmp = SelectObject(hdc, bmp);

    RECT rc = {0, 0, texW, texH};
    FillRect(hdc, &rc, (HBRUSH)GetStockObject(WHITE_BRUSH));
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, RGB(0, 0, 0));

    int y = padTop;
    for (const std::wstring& line : lines) {
        const int w = MeasureTextWidthPx(hdc, line);
        int x = padLeft + (maxTextW - w) / 2;
        if (x < padLeft) x = padLeft;
        if (!line.empty()) {
            TextOutW(hdc, x, y, line.c_str(), (int)line.size());
        }
        y += lineStep;
    }

    const uint8_t* src = (const uint8_t*)bits;
    std::vector<uint8_t> alpha((size_t)texW * (size_t)texH, 0);
    for (int py = 0; py < texH; ++py) {
        for (int px = 0; px < texW; ++px) {
            const size_t i = (size_t)py * (size_t)texW + (size_t)px;
            const uint8_t b = src[i * 4 + 0];
            const uint8_t g = src[i * 4 + 1];
            const uint8_t r = src[i * 4 + 2];
            const uint8_t lum =
                (uint8_t)((77u * (uint32_t)r +
                           150u * (uint32_t)g +
                           29u * (uint32_t)b) >> 8);
            uint8_t a = (uint8_t)(255u - lum);
            if (a < 8u) a = 0u;
            alpha[i] = a;
        }
    }

    std::vector<uint8_t> outlineAlpha(alpha.size(), 0);
    for (int py = 0; py < texH; ++py) {
        for (int px = 0; px < texW; ++px) {
            uint8_t best = 0;
            for (int oy = -outline; oy <= outline; ++oy) {
                const int sy = py + oy;
                if (sy < 0 || sy >= texH) continue;
                for (int ox = -outline; ox <= outline; ++ox) {
                    const int sx = px + ox;
                    if (sx < 0 || sx >= texW) continue;
                    const uint8_t a =
                        alpha[(size_t)sy * (size_t)texW + (size_t)sx];
                    if (a > best) best = a;
                }
            }
            outlineAlpha[(size_t)py * (size_t)texW + (size_t)px] = best;
        }
    }

    std::vector<uint32_t> rgba((size_t)texW * (size_t)texH, 0u);
    for (int py = 0; py < texH; ++py) {
        for (int px = 0; px < texW; ++px) {
            const size_t i = (size_t)py * (size_t)texW + (size_t)px;
            const int sx = px - shadowX;
            const int sy = py - shadowY;
            if (sx >= 0 && sx < texW && sy >= 0 && sy < texH) {
                const uint8_t shadowA =
                    alpha[(size_t)sy * (size_t)texW + (size_t)sx];
                if (shadowA != 0u) {
                    rgba[i] = CompositeArgbOverRgba(rgba[i],
                                                    entry.shadowColor,
                                                    shadowA);
                }
            }

            const uint8_t strokeA =
                outlineAlpha[i] > alpha[i]
                    ? static_cast<uint8_t>(outlineAlpha[i] - alpha[i])
                    : 0u;
            if (strokeA != 0u) {
                rgba[i] = CompositeArgbOverRgba(rgba[i],
                                                entry.outlineColor,
                                                strokeA);
            }
            const uint8_t fillA = alpha[i];
            if (fillA != 0u) {
                rgba[i] =
                    CompositeArgbOverRgba(rgba[i], entry.fillColor, fillA);
            }
        }
    }

    SelectObject(hdc, oldBmp);
    DeleteObject(bmp);
    SelectObject(hdc, oldFont);
    DeleteObject(font);
    DeleteDC(hdc);

    entry.srv = entry.renderer->CreateTexture(rgba.data(), texW, texH);
    entry.w = texW;
    entry.h = texH;
    return entry.srv != nullptr;
}

static TextTextureEntry* GetOrCreateTextTexture(D3D11Renderer* renderer,
                                                const std::wstring& text,
                                                const std::wstring& fontFace,
                                                int fontPx,
                                                int widthPx,
                                                int outlinePx,
                                                int shadowOffsetXPx,
                                                int shadowOffsetYPx,
                                                uint32_t fillColor,
                                                uint32_t outlineColor,
                                                uint32_t shadowColor) {
    if (TextTextureEntry* cached =
            FindTextTexture(text,
                            fontFace,
                            fontPx,
                            widthPx,
                            outlinePx,
                            shadowOffsetXPx,
                            shadowOffsetYPx,
                            fillColor,
                            outlineColor,
                            shadowColor)) {
        if (cached->renderer == renderer && cached->srv) {
            return cached;
        }
        DestroyTextTextureEntry(*cached);
        cached->renderer = renderer;
        if (BuildTextTexture(*cached)) {
            return cached;
        }
        return nullptr;
    }

    s_textCache.emplace_back();
    TextTextureEntry& entry = s_textCache.back();
    entry.text = text;
    entry.fontFace = fontFace;
    entry.fontPx = fontPx;
    entry.widthPx = widthPx;
    entry.outlinePx = outlinePx;
    entry.shadowOffsetXPx = shadowOffsetXPx;
    entry.shadowOffsetYPx = shadowOffsetYPx;
    entry.fillColor = fillColor;
    entry.outlineColor = outlineColor;
    entry.shadowColor = shadowColor;
    entry.renderer = renderer;
    if (!BuildTextTexture(entry)) {
        DestroyTextTextureEntry(entry);
        s_textCache.pop_back();
        return nullptr;
    }
    return &entry;
}

static float SubtitleYForSource(const PrGameContext& ctx,
                                PrStage1HdSubtitleSourceKind kind) {
    switch (kind) {
    case PrStage1HdSubtitleSourceKind::Movie1:
    case PrStage1HdSubtitleSourceKind::CommonLyrics:
        return ctx.stage1HdSubtitleMovieY;
    case PrStage1HdSubtitleSourceKind::OverlayScriptText:
        return ctx.stage1HdSubtitleGameplayY;
    default:
        return ctx.stage1HdSubtitleY;
    }
}

enum class NativeAnchorPolicy : uint8_t {
    RequiredForRender = 0,
    AllowOverlayBeforeObserveForSuppress,
};

static bool HasRequiredNativeSubtitleText(
    PrGameContext& ctx,
    PrStage1HdSubtitleSourceKind kind,
    NativeAnchorPolicy anchorPolicy) {
    if (anchorPolicy ==
            NativeAnchorPolicy::AllowOverlayBeforeObserveForSuppress &&
        kind == PrStage1HdSubtitleSourceKind::OverlayScriptText) {
        return true;
    }
    return HasNativeSubtitleTextThisFrame(ctx, kind);
}

struct ActiveSubtitleText {
    const std::wstring* text = nullptr;
    PrStage1HdSubtitleSourceKind kind = PrStage1HdSubtitleSourceKind::None;
    std::string anchorKey;
};

static void SubmitSubtitleTexture(PrGameContext& ctx,
                                  const std::wstring& text,
                                  PrStage1HdSubtitleSourceKind kind,
                                  const std::string& anchorKey) {
    if (!ctx.renderer || text.empty()) {
        return;
    }

    SubtitleRenderMetrics metrics{};
    if (!BuildSubtitleRenderMetrics(ctx, metrics)) {
        return;
    }
    TextTextureEntry* entry =
        GetOrCreateTextTexture(ctx.renderer,
                               text,
                               metrics.fontFace,
                               metrics.fontPx,
                               metrics.widthPx,
                               metrics.outlinePx,
                               metrics.shadowOffsetXPx,
                               metrics.shadowOffsetYPx,
                               metrics.fillColor,
                               metrics.outlineColor,
                               metrics.shadowColor);
    if (!entry || !entry->srv || entry->w <= 0 || entry->h <= 0) {
        return;
    }

    const float vx = metrics.vx;
    const float vy = metrics.vy;
    const float vs = metrics.vs;
    float x = vx + 160.0f * vs - (float)entry->w * 0.5f;
    float y = vy + SubtitleYForSource(ctx, kind) * vs;
    NativeSubtitleTextAnchor nativeAnchor{};
    if (TryGetStableNativeSubtitleAnchor(ctx,
                                         kind,
                                         anchorKey,
                                         nativeAnchor)) {
        const float anchorCenterX =
            (nativeAnchor.minX + nativeAnchor.maxX) * 0.5f;
        const float anchorCenterY =
            (nativeAnchor.minY + nativeAnchor.maxY) * 0.5f;
        x = anchorCenterX - (float)entry->w * 0.5f;
        y = anchorCenterY - (float)entry->h * 0.5f;
    }
    const float viewportBottom = vy + 240.0f * vs;
    const float viewportLeft = vx;
    const float viewportRight = vx + 320.0f * vs;
    const float topPad = 2.0f * vs;
    const float bottomPad = 4.0f * vs;
    if (x < viewportLeft) {
        x = viewportLeft;
    }
    if (x + (float)entry->w > viewportRight) {
        x = viewportRight - (float)entry->w;
    }
    if (y < vy + topPad) {
        y = vy + topPad;
    }
    if (y + (float)entry->h + bottomPad > viewportBottom) {
        y = viewportBottom - (float)entry->h - bottomPad;
    }

    if (ctx.stage1HdSubtitleDrawBox) {
        const float boxPadX = 5.0f * vs;
        const float boxPadY = 2.0f * vs;
        D3D11Renderer::SolidRectCmd box{};
        box.x = x - boxPadX;
        box.y = y - boxPadY;
        box.w = (float)entry->w + boxPadX * 2.0f;
        box.h = (float)entry->h + boxPadY * 2.0f;
        box.r = 0.0f;
        box.g = 0.0f;
        box.b = 0.0f;
        box.a = 0.48f;
        box.layer = 980;
        ctx.renderer->SubmitSolidRect(box);
    }

    D3D11Renderer::SpriteCmd cmd{};
    cmd.texture = entry->srv;
    cmd.x = x;
    cmd.y = y;
    cmd.w = (float)entry->w;
    cmd.h = (float)entry->h;
    cmd.u0 = 0.0f;
    cmd.v0 = 0.0f;
    cmd.u1 = 1.0f;
    cmd.v1 = 1.0f;
    cmd.r = 1.0f;
    cmd.g = 1.0f;
    cmd.b = 1.0f;
    cmd.a = 1.0f;
    cmd.blend = D3D11Renderer::BlendMode::Alpha;
    cmd.layer = 981;
    ctx.renderer->SubmitSprite(cmd);
}

static ActiveSubtitleText ResolveActiveSubtitleText(
    PrGameContext& ctx,
    NativeAnchorPolicy anchorPolicy) {
    ActiveSubtitleText out{};
    if (!ctx.stage1HdSubtitles ||
        ctx.currentScene != PrSceneId::Scene1 ||
        ctx.subtitleFlag == 0) {
        return out;
    }

    const SubtitleTable& table = EnsureSubtitleTable(ctx);
    if (!table.exists || table.entries.empty()) {
        return out;
    }

    const std::string language = ResolveContextLanguage(ctx);
    PrStage1HdSubtitleRuntimeSource source{};
    if (PrScn1::GetStage1HdSubtitleRuntimeSource(ctx, source) &&
        source.valid &&
        HasRequiredNativeSubtitleText(ctx, source.kind, anchorPolicy)) {
        if (const std::wstring* text =
                FindSubtitleText(table, source, language)) {
            out.text = text;
            out.kind = source.kind;
            out.anchorKey = BuildAnchorKey(source);
            return out;
        }
    }

    if (PrSS0Scene0RuntimeDirect::RuntimeEnabled() &&
        ctx.currentScene == PrSceneId::Scene1) {
        return out;
    }

    const SubtitleInfo* active = PrSqevs1::GetActiveSubtitle();
    if (active == nullptr ||
        active->text == nullptr ||
        active->text[0] == '\0') {
        return out;
    }

    PrStage1HdSubtitleRuntimeSource sqevSource{};
    sqevSource.valid = true;
    sqevSource.kind = PrStage1HdSubtitleSourceKind::OverlayScriptText;
    sqevSource.mode = 0xFFu;
    sqevSource.queryFrame = active->startFrame;
    sqevSource.eventFrame = active->startFrame;
    sqevSource.durationFrames =
        active->durationFrames > UINT16_MAX
            ? UINT16_MAX
            : static_cast<uint16_t>(active->durationFrames);
    sqevSource.psxAddr = active->eventId;
    sqevSource.originalText = active->text;
    if (HasRequiredNativeSubtitleText(ctx, sqevSource.kind, anchorPolicy)) {
        if (const std::wstring* text =
                FindSubtitleText(table, sqevSource, language)) {
            out.text = text;
            out.kind = sqevSource.kind;
            out.anchorKey = BuildAnchorKey(sqevSource);
        }
    }
    return out;
}

static bool TableSupportsLanguage(const SubtitleTable& table,
                                  const std::string& language) {
    if (!table.loaded || !table.exists || table.entries.empty()) {
        return false;
    }
    for (const SubtitleEntry& entry : table.entries) {
        if (entry.language == "*" || entry.language == language) {
            return true;
        }
    }
    return false;
}

static bool HasExternalReplacementLanguage(PrGameContext& ctx) {
    if (!ctx.stage1HdSubtitles ||
        ctx.currentScene != PrSceneId::Scene1 ||
        ctx.subtitleFlag == 0) {
        return false;
    }

    const std::string language = ResolveContextLanguage(ctx);
    const SubtitleTable& table =
        s_table.loaded ? s_table : EnsureSubtitleTable(ctx);
    return TableSupportsLanguage(table, language);
}

} // namespace

bool HasActiveExternalSubtitle(PrGameContext& ctx) {
    const ActiveSubtitleText active = ResolveActiveSubtitleText(
        ctx,
        NativeAnchorPolicy::AllowOverlayBeforeObserveForSuppress);
    return active.text != nullptr && !active.text->empty();
}

bool ShouldSuppressNativeSubtitleText(PrGameContext& ctx) {
    if (s_suppressCacheValid && s_suppressCacheFrame == ctx.frame) {
        return s_suppressCacheValue;
    }
    s_suppressCacheFrame = ctx.frame;
    s_suppressCacheValue = HasActiveExternalSubtitle(ctx);
    s_suppressCacheValid = true;
    return s_suppressCacheValue;
}

bool ShouldSuppressNativeSubtitleFrame(PrGameContext& ctx) {
    (void)ctx;
    return false;
}

void Preload(PrGameContext& ctx) {
    if (!ctx.stage1HdSubtitles || !ctx.renderer) {
        return;
    }

    SubtitleRenderMetrics metrics{};
    if (!BuildSubtitleRenderMetrics(ctx, metrics)) {
        return;
    }

    const SubtitleTable& table = EnsureSubtitleTable(ctx);
    if (!table.loaded || !table.exists || table.entries.empty()) {
        return;
    }

    const std::string language = ResolveContextLanguage(ctx);
    if (MatchesPreloadSignature(s_preloadSignature,
                                ctx.renderer,
                                table,
                                language,
                                metrics)) {
        return;
    }

    uint32_t requested = 0u;
    uint32_t ready = 0u;
    for (const SubtitleEntry& entry : table.entries) {
        if (entry.text.empty() ||
            !(entry.language == "*" || entry.language == language)) {
            continue;
        }
        ++requested;
        TextTextureEntry* texture =
            GetOrCreateTextTexture(ctx.renderer,
                                   entry.text,
                                   metrics.fontFace,
                                   metrics.fontPx,
                                   metrics.widthPx,
                                   metrics.outlinePx,
                                   metrics.shadowOffsetXPx,
                                   metrics.shadowOffsetYPx,
                                   metrics.fillColor,
                                   metrics.outlineColor,
                                   metrics.shadowColor);
        if (texture != nullptr && texture->srv != nullptr) {
            ++ready;
        }
    }
    StorePreloadSignature(ctx.renderer,
                          table,
                          language,
                          metrics,
                          ready,
                          requested);
    Log::Printf("Stage1HdSubtitles: preloaded %u/%u texture(s) for %s",
                (unsigned)ready,
                (unsigned)requested,
                language.c_str());
}

void ObserveNativeSubtitleTextRect(PrGameContext& ctx,
                                   PrStage1HdSubtitleSourceKind kind,
                                   float x,
                                   float y,
                                   float w,
                                   float h) {
    if (kind == PrStage1HdSubtitleSourceKind::None ||
        !std::isfinite(x) || !std::isfinite(y) ||
        !std::isfinite(w) || !std::isfinite(h) ||
        w <= 0.0f || h <= 0.0f) {
        return;
    }

    NativeSubtitleTextAnchor& anchor =
        s_nativeTextAnchors[NativeAnchorIndex(kind)];
    if (anchor.frame != ctx.frame || anchor.kind != kind) {
        ResetNativeAnchor(anchor, ctx.frame, kind);
    }
    if (s_suppressCacheValid &&
        s_suppressCacheFrame == ctx.frame &&
        !s_suppressCacheValue) {
        s_suppressCacheValid = false;
    }

    const float x0 = x;
    const float y0 = y;
    const float x1 = x + w;
    const float y1 = y + h;
    if (!anchor.valid) {
        anchor.valid = true;
        anchor.minX = x0;
        anchor.minY = y0;
        anchor.maxX = x1;
        anchor.maxY = y1;
    } else {
        anchor.minX = (std::min)(anchor.minX, x0);
        anchor.minY = (std::min)(anchor.minY, y0);
        anchor.maxX = (std::max)(anchor.maxX, x1);
        anchor.maxY = (std::max)(anchor.maxY, y1);
    }
    anchor.rectCount++;
}

void Render(PrGameContext& ctx) {
    if (!ctx.stage1HdSubtitles ||
        ctx.currentScene != PrSceneId::Scene1 ||
        ctx.subtitleFlag == 0 ||
        !ctx.renderer) {
        return;
    }

    const ActiveSubtitleText active = ResolveActiveSubtitleText(
        ctx,
        NativeAnchorPolicy::RequiredForRender);
    if (!active.text || active.text->empty()) {
        return;
    }

    SubmitSubtitleTexture(ctx, *active.text, active.kind, active.anchorKey);
}

void ClearCache() {
    for (TextTextureEntry& entry : s_textCache) {
        DestroyTextTextureEntry(entry);
    }
    s_textCache.clear();
    s_table = SubtitleTable{};
    s_missingLogged = false;
    s_suppressCacheValid = false;
    s_suppressCacheFrame = UINT32_MAX;
    s_suppressCacheValue = false;
    s_nativeTextAnchors = {};
    s_stableNativeTextAnchors = {};
    s_preloadSignature = PreloadSignature{};
}

} // namespace PrStage1HdSubtitles
