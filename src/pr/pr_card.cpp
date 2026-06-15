#include "pr_card.h"

#include <windows.h>

#include <array>
#include <cctype>
#include <cstddef>
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

#include "logger.h"

namespace {

constexpr size_t kCardBlockSize = 8192;
constexpr size_t kCardHeaderSize = 512;
constexpr size_t kCardPayloadSize = 4876;
constexpr size_t kCardPayloadOffset = 0x200;

constexpr size_t kCardClutOffset = 0x60;
constexpr size_t kCardClutSize = 0x20;
constexpr size_t kCardIconsOffset = 0x80;
constexpr size_t kCardIconSize = 0x80;
constexpr size_t kCardIconCount = 3;
constexpr size_t kCardIconsSize = kCardIconSize * kCardIconCount;

constexpr size_t kMemCardImageSize = 128 * 1024;
constexpr size_t kMemCardBlockSize = 8192;
constexpr size_t kMemCardBlockCount = 16;
constexpr size_t kMemCardDataBlockCount = 15;
constexpr size_t kMemCardFrameSize = 128;

static_assert(kMemCardImageSize == kMemCardBlockCount * kMemCardBlockSize);
static_assert(kMemCardBlockSize == kCardBlockSize);
static_assert(kMemCardFrameSize * 64 == kMemCardBlockSize);

static_assert(kCardPayloadOffset + kCardPayloadSize <= kCardBlockSize);
static_assert(kCardClutOffset + kCardClutSize <= kCardHeaderSize);
static_assert(kCardIconsOffset + kCardIconsSize <= kCardHeaderSize);

static std::array<uint8_t, kCardPayloadSize> s_payload{};
static std::string s_suffix;

static bool s_assetsLoaded = false;
static std::array<uint8_t, kCardClutSize> s_clutBytes{};
static std::array<uint8_t, kCardIconsSize> s_iconBytes{};

static constexpr const char* kMemCardFilePrefix = "BASCUS-94183";
static constexpr const char* kMemCardFilePrefixLegacy = "PARAPPA";

static bool IsAllowedSuffixChar(uint8_t ch) {
    if (ch >= 0x30 && ch <= 0x39) return true;
    if (ch >= 0x41 && ch <= 0x5A) return true;
    if (ch >= 0x61 && ch <= 0x7A) return true;

    switch (ch) {
        case 0x92:
        case 0x94:
        case 0x96:
        case 0xA3:
        case 0xAF:
        case 0xC4:
        case 0xC5:
        case 0xCF:
        case 0xD4:
        case 0xD5:
        case 0xD6:
        case 0xDC:
        case 0xDD:
        case 0xDE:
        case 0xE4:
        case 0xE5:
        case 0xE6:
        case 0xEC:
        case 0xED:
        case 0xEE:
        case 0xF6:
        case 0xFC:
        case 0xFD:
        case 0xFE:
            return true;
        default:
            return false;
    }

    return false;
}

static void WriteU32LE(uint8_t* p, uint32_t v) {
    p[0] = (uint8_t)(v & 0xFF);
    p[1] = (uint8_t)((v >> 8) & 0xFF);
    p[2] = (uint8_t)((v >> 16) & 0xFF);
    p[3] = (uint8_t)((v >> 24) & 0xFF);
}

static void WriteU16LE(uint8_t* p, uint16_t v) {
    p[0] = (uint8_t)(v & 0xFF);
    p[1] = (uint8_t)((v >> 8) & 0xFF);
}

static uint32_t ReadU32LE(const uint8_t* p) {
    return (uint32_t)p[0]
        | ((uint32_t)p[1] << 8)
        | ((uint32_t)p[2] << 16)
        | ((uint32_t)p[3] << 24);
}

static uint8_t ComputeXorChecksum0x7F(const uint8_t* frame) {
    uint8_t x = 0;
    for (size_t i = 0; i < 0x7F; i++) {
        x ^= frame[i];
    }
    return x;
}

static bool IsFrameXorValid0x7F(const uint8_t* frame) {
    if (!frame) return false;
    return frame[0x7F] == ComputeXorChecksum0x7F(frame);
}

static void FixFrameXor0x7F(uint8_t* frame) {
    if (!frame) return;
    frame[0x7F] = ComputeXorChecksum0x7F(frame);
}

static void FixControlFramesXor(uint8_t* img) {
    if (!img) return;

    // Only control frames use the 0x7F XOR byte in .mcr images.
    // Frame0 header, frames 1..15 directory, frames 16..35 broken sector list.
    for (int frame = 0; frame <= 35; frame++) {
        uint8_t* fr = img + (size_t)frame * kMemCardFrameSize;
        FixFrameXor0x7F(fr);
    }
}

static void ValidateControlFramesXor(const uint8_t* img, const std::filesystem::path& path) {
    if (!img) return;

    for (int frame = 0; frame <= 35; frame++) {
        const uint8_t* fr = img + (size_t)frame * kMemCardFrameSize;
        if (!IsFrameXorValid0x7F(fr)) {
            Log::Printf("PrCard memcard XOR mismatch frame=%d file=%s", frame, path.string().c_str());
        }
    }
}

static std::string FilterSuffixForHeaderTitle(const std::string& rawSuffix) {
    std::string out;
    out.reserve(12);

    for (char c : rawSuffix) {
        if (out.size() >= 12) break;

        const uint8_t ch = static_cast<uint8_t>(c);
        out.push_back(IsAllowedSuffixChar(ch) ? c : '?');
    }

    return out;
}

static std::string EncodeSuffixForFilenameLegacyUnfiltered(const std::string& rawSuffix) {
    std::ostringstream oss;
    oss << std::uppercase;

    for (unsigned char ch : rawSuffix) {
        if (std::isalnum(ch)) {
            oss << (char)ch;
        } else {
            char buf[8];
            sprintf_s(buf, "_%02X", (unsigned)ch);
            oss << buf;
        }
    }

    std::string s = oss.str();
    if (s.size() > 64) s.resize(64);
    return s;
}

static std::string EncodeSuffixForFilename(const std::string& rawSuffix) {
    const std::string filtered = FilterSuffixForHeaderTitle(rawSuffix);
    std::ostringstream oss;
    oss << std::uppercase;

    for (unsigned char ch : filtered) {
        if (std::isalnum(ch)) {
            oss << (char)ch;
        } else {
            char buf[8];
            sprintf_s(buf, "_%02X", (unsigned)ch);
            oss << buf;
        }
    }

    std::string s = oss.str();
    if (s.size() > 64) s.resize(64);
    return s;
}

static bool WriteTitleShiftJis(uint8_t* header, const std::string& utf8Title) {
    if (!header) return false;

    int wlen = MultiByteToWideChar(CP_UTF8, 0, utf8Title.c_str(), -1, nullptr, 0);
    if (wlen <= 0) {
        return false;
    }

    std::wstring w;
    w.resize((size_t)wlen);
    if (MultiByteToWideChar(CP_UTF8, 0, utf8Title.c_str(), -1, w.data(), wlen) <= 0) {
        return false;
    }

    int slen = WideCharToMultiByte(932, 0, w.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (slen <= 0) {
        return false;
    }

    std::string sjis;
    sjis.resize((size_t)slen);
    if (WideCharToMultiByte(932, 0, w.c_str(), -1, sjis.data(), slen, nullptr, nullptr) <= 0) {
        return false;
    }

    constexpr size_t titleMax = 0x40;
    constexpr size_t titleOff = 0x04;
    const size_t copyN = (sjis.size() > titleMax) ? titleMax : sjis.size();
    for (size_t i = 0; i < copyN; i++) {
        header[titleOff + i] = (uint8_t)sjis[i];
    }

    return true;
}

static std::filesystem::path GetExecutableDir() {
    wchar_t path[MAX_PATH];
    DWORD len = GetModuleFileNameW(nullptr, path, MAX_PATH);
    if (len == 0 || len >= MAX_PATH) return std::filesystem::current_path();
    return std::filesystem::path(path).parent_path();
}

static std::filesystem::path GetSaveDir() {
    return GetExecutableDir() / L"save";
}

static std::filesystem::path GetMemCardImagePath(int slot) {
    // Align to PSX naming convention bu%1d%1d:
    // slot0 -> bu00, slot1 -> bu10
    const int id = (slot == 0) ? 0 : 10;
    char buf[16];
    sprintf_s(buf, "bu%02d.mcr", id);
    return GetSaveDir() / std::filesystem::path(buf);
}

static std::filesystem::path ResolvePrimaryMemCardImagePath() {
    std::error_code ec;
    std::filesystem::path imgPath = GetMemCardImagePath(0);
    if (std::filesystem::exists(imgPath, ec)) {
        return imgPath;
    }

    const std::filesystem::path legacy = GetSaveDir() / L"bu01.mcr";
    if (std::filesystem::exists(legacy, ec)) {
        return legacy;
    }

    const std::filesystem::path secondCard = GetMemCardImagePath(1);
    if (std::filesystem::exists(secondCard, ec)) {
        return secondCard;
    }

    return imgPath;
}

static bool ReadFileExact(const std::filesystem::path& path, void* dst, size_t size) {
    std::ifstream f(path, std::ios::binary);
    if (!f.is_open()) {
        return false;
    }

    f.read(reinterpret_cast<char*>(dst), (std::streamsize)size);
    if (f.gcount() != (std::streamsize)size) {
        return false;
    }

    return true;
}

static bool WriteFileAtomic(const std::filesystem::path& path, const void* data, size_t size) {
    std::filesystem::path dir = path.parent_path();
    std::error_code ec;
    std::filesystem::create_directories(dir, ec);

    std::filesystem::path tmp = path;
    tmp += L".tmp";

    {
        std::ofstream f(tmp, std::ios::binary | std::ios::trunc);
        if (!f.is_open()) {
            return false;
        }
        f.write(reinterpret_cast<const char*>(data), (std::streamsize)size);
        f.flush();
        if (!f) {
            return false;
        }
    }

    std::filesystem::rename(tmp, path, ec);
    if (ec) {
        std::filesystem::remove(path, ec);
        ec.clear();
        std::filesystem::rename(tmp, path, ec);
        if (ec) {
            return false;
        }
    }

    return true;
}

static void EnsureAssetsLoaded() {
    if (s_assetsLoaded) return;
    s_assetsLoaded = true;

    const std::filesystem::path base = GetSaveDir();
    const std::filesystem::path clutPath = base / L"prcard_clut.bin";
    const std::filesystem::path iconPath = base / L"prcard_icon.bin";

    if (ReadFileExact(clutPath, s_clutBytes.data(), s_clutBytes.size())) {
        Log::Printf("PrCard assets: loaded clut %s", clutPath.string().c_str());
    } else {
        Log::Printf("PrCard assets: missing clut %s", clutPath.string().c_str());
    }

    if (ReadFileExact(iconPath, s_iconBytes.data(), s_iconBytes.size())) {
        Log::Printf("PrCard assets: loaded icons %s", iconPath.string().c_str());
    } else {
        Log::Printf("PrCard assets: missing icons %s", iconPath.string().c_str());
    }
}

static std::filesystem::path GetSaveFilePath(int slot) {
    std::string suffix = s_suffix.empty() ? ("SLOT" + std::to_string(slot)) : s_suffix;
    std::string name = std::string(kMemCardFilePrefix) + EncodeSuffixForFilename(suffix);

    char bu[16];
    sprintf_s(bu, "bu%02d_", slot);
    std::string fileStem = std::string(bu) + name;

    return GetSaveDir() / std::filesystem::path(fileStem + ".bin");
}

static std::filesystem::path GetSaveFilePathLegacy(int slot) {
    std::string suffix = s_suffix.empty() ? ("SLOT" + std::to_string(slot)) : s_suffix;
    std::string name = std::string(kMemCardFilePrefixLegacy) + EncodeSuffixForFilenameLegacyUnfiltered(suffix);

    char bu[16];
    sprintf_s(bu, "bu%02d_", slot);
    std::string fileStem = std::string(bu) + name;

    return GetSaveDir() / std::filesystem::path(fileStem + ".bin");
}

static std::string FilterSuffixForDirName(const std::string& rawSuffix) {
    std::string out;
    out.reserve(12);

    for (char c : rawSuffix) {
        if (out.size() >= 12) break;
        const uint8_t ch = static_cast<uint8_t>(c);
        out.push_back(IsAllowedSuffixChar(ch) ? c : '?');
    }

    return out;
}

static std::string MakeInternalFilenameForMemCard(int slot) {
    const std::string rawSuffix = s_suffix.empty() ? ("SLOT" + std::to_string(slot)) : s_suffix;
    std::string suffix = FilterSuffixForDirName(rawSuffix);
    std::string name = std::string(kMemCardFilePrefix) + suffix;
    if (name.size() > 20) name.resize(20);
    return name;
}

static void FormatNewMemCardImage(uint8_t* img) {
    if (!img) return;

    for (size_t i = 0; i < kMemCardImageSize; i++) {
        img[i] = 0;
    }

    // Frame 0: "MC" + zeros + XOR checksum
    img[0] = 'M';
    img[1] = 'C';
    img[0x7F] = ComputeXorChecksum0x7F(img);

    // Directory frames 1..15
    for (int block = 1; block <= (int)kMemCardDataBlockCount; block++) {
        uint8_t* ent = img + (size_t)block * kMemCardFrameSize;
        for (size_t i = 0; i < kMemCardFrameSize; i++) {
            ent[i] = 0;
        }

        WriteU32LE(ent + 0x00, 0x000000A0);
        WriteU32LE(ent + 0x04, 0);
        WriteU16LE(ent + 0x08, 0xFFFF);
        for (size_t i = 0; i < 21; i++) {
            ent[0x0A + i] = 0;
        }
        ent[0x1F] = 0;
        ent[0x7F] = ComputeXorChecksum0x7F(ent);
    }

    // Broken sector list frames 16..35: first dword FFFFFFFF + XOR checksum
    for (int frame = 16; frame <= 35; frame++) {
        uint8_t* fr = img + (size_t)frame * kMemCardFrameSize;
        for (size_t i = 0; i < kMemCardFrameSize; i++) {
            fr[i] = 0;
        }
        WriteU32LE(fr + 0x00, 0xFFFFFFFF);
        fr[0x7F] = ComputeXorChecksum0x7F(fr);
    }

    FixControlFramesXor(img);
}

static bool LoadMemCardImage(const std::filesystem::path& path, uint8_t* outImg) {
    if (!outImg) return false;

    std::ifstream f(path, std::ios::binary);
    if (!f.is_open()) return false;

    f.read(reinterpret_cast<char*>(outImg), (std::streamsize)kMemCardImageSize);
    if (f.gcount() != (std::streamsize)kMemCardImageSize) return false;

    if (outImg[0] != 'M' || outImg[1] != 'C') return false;

    ValidateControlFramesXor(outImg, path);

    return true;
}

static std::string ReadHeaderTitleAscii(const uint8_t* block) {
    if (!block) return {};
    const uint8_t* title = block + 0x04;
    std::string out;
    out.reserve(32);
    for (size_t i = 0; i < 0x40; ++i) {
        const uint8_t c = title[i];
        if (c == 0) break;
        if (c >= 0x20 && c <= 0x7E) {
            out.push_back((char)c);
        } else {
            out.push_back(' ');
        }
    }
    while (!out.empty() && out.back() == ' ') out.pop_back();
    const std::string prefix = "PARAPPA \"";
    if (out.size() > prefix.size() + 1 && out.rfind(prefix, 0) == 0 && out.back() == '"') {
        return out.substr(prefix.size(), out.size() - prefix.size() - 1);
    }
    return out;
}

static int FindDirEntryByName(const uint8_t* img, const std::string& name) {
    for (int block = 1; block <= (int)kMemCardDataBlockCount; block++) {
        const uint8_t* ent = img + (size_t)block * kMemCardFrameSize;
        if (!IsFrameXorValid0x7F(ent)) {
            continue;
        }
        const uint32_t state = ReadU32LE(ent + 0x00);
        const uint8_t s = (uint8_t)(state & 0xFF);
        if (s != 0x51 && s != 0x52 && s != 0x53) continue;

        const char* fn = reinterpret_cast<const char*>(ent + 0x0A);
        if (strncmp(fn, name.c_str(), 20) == 0) {
            return block;
        }
    }
    return -1;
}

static int FindFirstFreeDirEntry(const uint8_t* img) {
    for (int block = 1; block <= (int)kMemCardDataBlockCount; block++) {
        const uint8_t* ent = img + (size_t)block * kMemCardFrameSize;
        const uint32_t state = ReadU32LE(ent + 0x00);
        const uint8_t s = (uint8_t)(state & 0xFF);
        if (s == 0xA0 || s == 0xA1 || s == 0xA2 || s == 0xA3) {
            return block;
        }
    }
    return -1;
}

static void WriteDirEntry(uint8_t* img, int block, const std::string& name) {
    uint8_t* ent = img + (size_t)block * kMemCardFrameSize;
    for (size_t i = 0; i < kMemCardFrameSize; i++) {
        ent[i] = 0;
    }

    WriteU32LE(ent + 0x00, 0x00000051);
    WriteU32LE(ent + 0x04, (uint32_t)kMemCardBlockSize);
    WriteU16LE(ent + 0x08, 0xFFFF);

    for (size_t i = 0; i < 21; i++) {
        ent[0x0A + i] = 0;
    }
    const size_t copyN = (name.size() > 20) ? 20 : name.size();
    memcpy(ent + 0x0A, name.data(), copyN);
    ent[0x1F] = 0;
    ent[0x7F] = ComputeXorChecksum0x7F(ent);
}

static bool UpdateMemCardImage(const std::filesystem::path& imgPath, int slot, const uint8_t* saveBlock8K) {
    std::array<uint8_t, kMemCardImageSize> img{};

    if (!LoadMemCardImage(imgPath, img.data())) {
        FormatNewMemCardImage(img.data());
    }

    const std::string internalName = MakeInternalFilenameForMemCard(slot);
    // Backward compatibility: older builds used "PARAPPA" as the internal filename prefix.
    const std::string rawSuffix = s_suffix.empty() ? ("SLOT" + std::to_string(slot)) : s_suffix;
    const std::string legacyName = std::string(kMemCardFilePrefixLegacy) + FilterSuffixForDirName(rawSuffix);

    int block = FindDirEntryByName(img.data(), internalName);
    if (block < 0) {
        block = FindDirEntryByName(img.data(), legacyName);
    }
    if (block < 0) {
        block = FindFirstFreeDirEntry(img.data());
    }

    if (block < 0) {
        return false;
    }

    WriteDirEntry(img.data(), block, internalName);

    uint8_t* dst = img.data() + (size_t)block * kMemCardBlockSize;
    memcpy(dst, saveBlock8K, kMemCardBlockSize);

    FixControlFramesXor(img.data());

    return WriteFileAtomic(imgPath, img.data(), img.size());
}

static bool UpdateMemCardImageBlock(const std::filesystem::path& imgPath,
                                    int blockIndex,
                                    int titleSlot,
                                    const uint8_t* saveBlock8K) {
    if (!saveBlock8K || blockIndex < 0 || blockIndex >= (int)kMemCardDataBlockCount) {
        return false;
    }

    std::array<uint8_t, kMemCardImageSize> img{};
    if (!LoadMemCardImage(imgPath, img.data())) {
        FormatNewMemCardImage(img.data());
    }

    const int block = blockIndex + 1;
    WriteDirEntry(img.data(), block, MakeInternalFilenameForMemCard(titleSlot));
    uint8_t* dst = img.data() + (size_t)block * kMemCardBlockSize;
    memcpy(dst, saveBlock8K, kMemCardBlockSize);

    FixControlFramesXor(img.data());
    return WriteFileAtomic(imgPath, img.data(), img.size());
}

static bool LoadBlockFromMemCardImage(const std::filesystem::path& imgPath, int slot, uint8_t* outBlock8K) {
    std::array<uint8_t, kMemCardImageSize> img{};
    if (!LoadMemCardImage(imgPath, img.data())) {
        return false;
    }

    const bool useExactName = !s_suffix.empty();
    const std::string wantName = MakeInternalFilenameForMemCard(slot);
    const std::string rawSuffix = s_suffix.empty() ? ("SLOT" + std::to_string(slot)) : s_suffix;
    const std::string wantNameLegacy = std::string(kMemCardFilePrefixLegacy) + FilterSuffixForDirName(rawSuffix);

    int foundBlock = -1;
    for (int block = 1; block <= (int)kMemCardDataBlockCount; block++) {
        const uint8_t* ent = img.data() + (size_t)block * kMemCardFrameSize;
        if (!IsFrameXorValid0x7F(ent)) {
            continue;
        }
        const uint32_t state = ReadU32LE(ent + 0x00);
        const uint8_t s = (uint8_t)(state & 0xFF);
        if (s != 0x51 && s != 0x52 && s != 0x53) continue;

        const char* fn = reinterpret_cast<const char*>(ent + 0x0A);
        const bool prefixOk = (strncmp(fn, kMemCardFilePrefix, strlen(kMemCardFilePrefix)) == 0)
            || (strncmp(fn, kMemCardFilePrefixLegacy, strlen(kMemCardFilePrefixLegacy)) == 0);
        if (!prefixOk) continue;

        if (useExactName) {
            if (strncmp(fn, wantName.c_str(), 20) != 0 && strncmp(fn, wantNameLegacy.c_str(), 20) != 0) continue;
        }

        foundBlock = block;
        break;
    }

    if (foundBlock < 0) {
        return false;
    }

    const uint8_t* src = img.data() + (size_t)foundBlock * kMemCardBlockSize;
    memcpy(outBlock8K, src, kMemCardBlockSize);
    return true;
}

static bool LoadBlockFromMemCardImageBlock(const std::filesystem::path& imgPath,
                                           int blockIndex,
                                           uint8_t* outBlock8K) {
    if (!outBlock8K || blockIndex < 0 || blockIndex >= (int)kMemCardDataBlockCount) {
        return false;
    }

    std::array<uint8_t, kMemCardImageSize> img{};
    if (!LoadMemCardImage(imgPath, img.data())) {
        return false;
    }

    const int block = blockIndex + 1;
    const uint8_t* ent = img.data() + (size_t)block * kMemCardFrameSize;
    if (!IsFrameXorValid0x7F(ent)) {
        return false;
    }
    const uint32_t state = ReadU32LE(ent + 0x00);
    const uint8_t s = (uint8_t)(state & 0xFF);
    if (s != 0x51 && s != 0x52 && s != 0x53) {
        return false;
    }

    const uint8_t* src = img.data() + (size_t)block * kMemCardBlockSize;
    if (src[0] != 'S' || src[1] != 'C') {
        return false;
    }
    memcpy(outBlock8K, src, kMemCardBlockSize);
    return true;
}

static void BuildHeader(uint8_t* header, int slot) {
    EnsureAssetsLoaded();

    for (size_t i = 0; i < kCardHeaderSize; i++) {
        header[i] = 0;
    }

    header[0] = 'S';
    header[1] = 'C';
    header[2] = 0x13;
    header[3] = 1;

    const std::string rawSuffix = s_suffix.empty() ? ("SLOT" + std::to_string(slot)) : s_suffix;
    const std::string suffix = FilterSuffixForHeaderTitle(rawSuffix);
    const std::string title = std::string("PARAPPA \"") + suffix + "\"";

    if (!WriteTitleShiftJis(header, title)) {
        const size_t titleMax = 0x40;
        const size_t titleOff = 0x04;
        for (size_t i = 0; i < titleMax && i < title.size(); i++) {
            header[titleOff + i] = static_cast<uint8_t>(title[i]);
        }
    }

    for (size_t i = 0; i < s_clutBytes.size(); i++) {
        header[kCardClutOffset + i] = s_clutBytes[i];
    }

    for (size_t i = 0; i < s_iconBytes.size(); i++) {
        header[kCardIconsOffset + i] = s_iconBytes[i];
    }
}

} // namespace

void PrCard::Init() {
    EnsureAssetsLoaded();

    std::error_code ec;
    std::filesystem::create_directories(GetSaveDir(), ec);

    Log::Printf("PrCard::Init");
}

void PrCard::Shutdown() {
    Log::Printf("PrCard::Shutdown");
}

bool PrCard::Load(int slot) {
    if (slot < 0 || slot > 1) {
        Log::Printf("PrCard::Load invalid slot=%d", slot);
        return false;
    }

    std::filesystem::path path = GetSaveFilePath(slot);
    std::filesystem::path legacyPath = GetSaveFilePathLegacy(slot);
    std::filesystem::path imgPath = GetMemCardImagePath(slot);
    std::filesystem::path imgPathUsed = imgPath;

    std::error_code ec;
    std::array<uint8_t, kCardBlockSize> block{};

    if (!std::filesystem::exists(path, ec) && std::filesystem::exists(legacyPath, ec)) {
        path = legacyPath;
    }

    if (std::filesystem::exists(path, ec)) {
        std::ifstream f(path, std::ios::binary);
        if (!f.is_open()) {
            Log::Printf("PrCard::Load slot=%d failed open file=%s", slot, path.string().c_str());
            return false;
        }
        f.read(reinterpret_cast<char*>(block.data()), (std::streamsize)block.size());
        if (f.gcount() != (std::streamsize)block.size()) {
            Log::Printf("PrCard::Load slot=%d short read=%lld file=%s", slot, (long long)f.gcount(), path.string().c_str());
            return false;
        }
    } else {
        if (!std::filesystem::exists(imgPathUsed, ec) && slot == 1) {
            const std::filesystem::path legacy = GetSaveDir() / L"bu01.mcr";
            if (std::filesystem::exists(legacy, ec)) {
                imgPathUsed = legacy;
            }
        }

        if (!std::filesystem::exists(imgPathUsed, ec)) {
            Log::Printf("PrCard::Load slot=%d missing file=%s and image=%s", slot, path.string().c_str(), imgPathUsed.string().c_str());
            return false;
        }
        if (!LoadBlockFromMemCardImage(imgPathUsed, slot, block.data())) {
            Log::Printf("PrCard::Load slot=%d failed parse image=%s", slot, imgPathUsed.string().c_str());
            return false;
        }
    }

    if (block[0] != 'S' || block[1] != 'C') {
        Log::Printf("PrCard::Load slot=%d bad header sig file=%s", slot, path.string().c_str());
        return false;
    }

    for (size_t i = 0; i < kCardPayloadSize; i++) {
        s_payload[i] = block[kCardPayloadOffset + i];
    }

    Log::Printf("PrCard::Load slot=%d ok", slot);
    return true;
}

bool PrCard::Save(int slot) {
    if (slot < 0 || slot > 1) {
        Log::Printf("PrCard::Save invalid slot=%d", slot);
        return false;
    }

    std::filesystem::path dir = GetSaveDir();
    std::filesystem::path path = GetSaveFilePath(slot);
    std::filesystem::path legacyPath = GetSaveFilePathLegacy(slot);
    std::filesystem::path imgPath = GetMemCardImagePath(slot);

    std::error_code ec;
    std::filesystem::create_directories(dir, ec);

    std::array<uint8_t, kCardBlockSize> block{};
    BuildHeader(block.data(), slot);

    for (size_t i = 0; i < kCardPayloadSize; i++) {
        block[kCardPayloadOffset + i] = s_payload[i];
    }

    if (!WriteFileAtomic(path, block.data(), block.size())) {
        Log::Printf("PrCard::Save slot=%d failed write file=%s", slot, path.string().c_str());
        return false;
    }

    // Also export/update a full 128KB memory card image for emulator/PS tools.
    if (!UpdateMemCardImage(imgPath, slot, block.data())) {
        Log::Printf("PrCard::Save slot=%d failed update image=%s", slot, imgPath.string().c_str());
        return false;
    }

    Log::Printf("PrCard::Save slot=%d ok file=%s legacy_file=%s image=%s", slot, path.string().c_str(), legacyPath.string().c_str(), imgPath.string().c_str());
    return true;
}

uint8_t* PrCard::Payload() {
    return s_payload.data();
}

const uint8_t* PrCard::PayloadConst() {
    return s_payload.data();
}

size_t PrCard::PayloadSize() {
    return kCardPayloadSize;
}

bool PrCard::EnumerateSlots(PrCardSlotInfo outSlots[15]) {
    if (!outSlots) return false;
    for (int i = 0; i < 15; ++i) {
        outSlots[i] = PrCardSlotInfo{};
    }

    std::array<uint8_t, kMemCardImageSize> img{};
    std::filesystem::path imgPath = GetMemCardImagePath(0);
    std::error_code ec;
    if (!std::filesystem::exists(imgPath, ec)) {
        imgPath = GetSaveDir() / L"bu01.mcr";
    }
    if (!std::filesystem::exists(imgPath, ec)) {
        imgPath = GetMemCardImagePath(1);
    }
    if (!LoadMemCardImage(imgPath, img.data())) {
        Log::Printf("PrCard::EnumerateSlots no memcard image path=%s", imgPath.string().c_str());
        return false;
    }

    int occupiedCount = 0;
    for (int block = 1; block <= (int)kMemCardDataBlockCount; ++block) {
        const uint8_t* ent = img.data() + (size_t)block * kMemCardFrameSize;
        if (!IsFrameXorValid0x7F(ent)) continue;
        const uint32_t state = ReadU32LE(ent + 0x00);
        const uint8_t s = (uint8_t)(state & 0xFF);
        if (s != 0x51 && s != 0x52 && s != 0x53) continue;

        const uint8_t* saveBlock = img.data() + (size_t)block * kMemCardBlockSize;
        if (saveBlock[0] != 'S' || saveBlock[1] != 'C') continue;

        PrCardSlotInfo info{};
        info.occupied = true;
        info.title = ReadHeaderTitleAscii(saveBlock);
        outSlots[block - 1] = std::move(info);
        occupiedCount++;
    }
    Log::Printf("PrCard::EnumerateSlots ok image=%s occupied=%d", imgPath.string().c_str(), occupiedCount);
    return true;
}

int PrCard::EnumerateEntriesCompact(PrCardEntryInfo outEntries[15]) {
    if (!outEntries) return 0;
    for (int i = 0; i < 15; ++i) {
        outEntries[i] = PrCardEntryInfo{};
    }

    PrCardSlotInfo slots[15] = {};
    if (!EnumerateSlots(slots)) {
        return 0;
    }

    int count = 0;
    for (int i = 0; i < 15 && count < 15; ++i) {
        if (!slots[i].occupied || slots[i].title.empty()) {
            continue;
        }
        outEntries[count].title = slots[i].title;
        outEntries[count].blockIndex = i;
        count++;
    }
    Log::Printf("PrCard::EnumerateEntriesCompact count=%d", count);
    return count;
}

bool PrCard::LoadFromMemCardEntry(int blockIndex) {
    std::array<uint8_t, kCardBlockSize> block{};
    const std::filesystem::path imgPath = ResolvePrimaryMemCardImagePath();
    if (!LoadBlockFromMemCardImageBlock(imgPath, blockIndex, block.data())) {
        Log::Printf("PrCard::LoadFromMemCardEntry block=%d failed image=%s",
                    blockIndex,
                    imgPath.string().c_str());
        return false;
    }

    for (size_t i = 0; i < kCardPayloadSize; ++i) {
        s_payload[i] = block[kCardPayloadOffset + i];
    }
    Log::Printf("PrCard::LoadFromMemCardEntry block=%d ok image=%s",
                blockIndex,
                imgPath.string().c_str());
    return true;
}

bool PrCard::SaveToMemCardEntry(int blockIndex) {
    std::array<uint8_t, kCardBlockSize> block{};
    BuildHeader(block.data(), blockIndex);
    for (size_t i = 0; i < kCardPayloadSize; ++i) {
        block[kCardPayloadOffset + i] = s_payload[i];
    }

    const std::filesystem::path imgPath = ResolvePrimaryMemCardImagePath();
    if (!UpdateMemCardImageBlock(imgPath, blockIndex, blockIndex, block.data())) {
        Log::Printf("PrCard::SaveToMemCardEntry block=%d failed image=%s",
                    blockIndex,
                    imgPath.string().c_str());
        return false;
    }

    Log::Printf("PrCard::SaveToMemCardEntry block=%d ok image=%s",
                blockIndex,
                imgPath.string().c_str());
    return true;
}

void PrCard::SetSuffix(const char* suffix) {
    s_suffix = suffix ? suffix : "";
}

const char* PrCard::GetSuffix() {
    return s_suffix.c_str();
}
