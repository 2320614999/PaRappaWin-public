#include "pr_memcard_backend.h"

#include "logger.h"

#include <windows.h>

#include <algorithm>
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
#include <system_error>
#include <vector>

namespace PrMemCardBackend {
namespace {

constexpr std::size_t kCardBlockSize = 8192u;
constexpr std::size_t kCardHeaderSize = 512u;
constexpr std::size_t kCardPayloadSize = 4876u;
constexpr std::size_t kCardPayloadOffset = 0x200u;
constexpr std::size_t kCardClutOffset = 0x60u;
constexpr std::size_t kCardClutSize = 0x20u;
constexpr std::size_t kCardIconsOffset = 0x80u;
constexpr std::size_t kCardIconSize = 0x80u;
constexpr std::size_t kCardIconCount = 3u;
constexpr std::size_t kCardIconsSize = kCardIconSize * kCardIconCount;
constexpr std::size_t kMemCardImageSize = 128u * 1024u;
constexpr std::size_t kMemCardBlockSize = 8192u;
constexpr std::size_t kMemCardBlockCount = 16u;
constexpr std::size_t kMemCardDataBlockCount = 15u;
constexpr std::size_t kMemCardFrameSize = 128u;
constexpr std::size_t kStageStatusPayloadBase = 0x0Cu;
constexpr std::size_t kStageLastSavedSlotPayloadOffset = 0x2Cu;

static_assert(kMemCardImageSize == kMemCardBlockCount * kMemCardBlockSize);
static_assert(kMemCardBlockSize == kCardBlockSize);
static_assert(kMemCardFrameSize * 64u == kMemCardBlockSize);
static_assert(kCardPayloadOffset + kCardPayloadSize <= kCardBlockSize);
static_assert(kCardClutOffset + kCardClutSize <= kCardHeaderSize);
static_assert(kCardIconsOffset + kCardIconsSize <= kCardHeaderSize);

constexpr const char* kMemCardFilePrefix = "BASCUS-94183";
constexpr const char* kMemCardFilePrefixLegacy = "PARAPPA";

std::array<uint8_t, kCardPayloadSize> s_payload{};
bool s_assetsLoaded = false;
std::array<uint8_t, kCardClutSize> s_clutBytes{};
std::array<uint8_t, kCardIconsSize> s_iconBytes{};

bool IsAllowedSuffixChar(uint8_t ch)
{
    if (ch >= 0x30u && ch <= 0x39u) {
        return true;
    }
    if (ch >= 0x41u && ch <= 0x5Au) {
        return true;
    }
    if (ch >= 0x61u && ch <= 0x7Au) {
        return true;
    }

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
}

void WriteU32LE(uint8_t* p, uint32_t v)
{
    p[0] = static_cast<uint8_t>(v & 0xFFu);
    p[1] = static_cast<uint8_t>((v >> 8) & 0xFFu);
    p[2] = static_cast<uint8_t>((v >> 16) & 0xFFu);
    p[3] = static_cast<uint8_t>((v >> 24) & 0xFFu);
}

void WriteU16LE(uint8_t* p, uint16_t v)
{
    p[0] = static_cast<uint8_t>(v & 0xFFu);
    p[1] = static_cast<uint8_t>((v >> 8) & 0xFFu);
}

uint32_t ReadU32LE(const uint8_t* p)
{
    return static_cast<uint32_t>(p[0]) |
           (static_cast<uint32_t>(p[1]) << 8) |
           (static_cast<uint32_t>(p[2]) << 16) |
           (static_cast<uint32_t>(p[3]) << 24);
}

uint8_t ComputeXorChecksum0x7F(const uint8_t* frame)
{
    uint8_t x = 0;
    for (std::size_t i = 0; i < 0x7Fu; ++i) {
        x ^= frame[i];
    }
    return x;
}

bool IsFrameXorValid0x7F(const uint8_t* frame)
{
    return frame != nullptr && frame[0x7F] == ComputeXorChecksum0x7F(frame);
}

void FixFrameXor0x7F(uint8_t* frame)
{
    if (!frame) {
        return;
    }
    frame[0x7F] = ComputeXorChecksum0x7F(frame);
}

void FixControlFramesXor(uint8_t* img)
{
    if (!img) {
        return;
    }
    for (int frame = 0; frame <= 35; ++frame) {
        FixFrameXor0x7F(img + static_cast<std::size_t>(frame) *
                                  kMemCardFrameSize);
    }
}

void ValidateControlFramesXor(const uint8_t* img,
                              const std::filesystem::path& path)
{
    if (!img) {
        return;
    }
    for (int frame = 0; frame <= 35; ++frame) {
        const uint8_t* fr =
            img + static_cast<std::size_t>(frame) * kMemCardFrameSize;
        if (!IsFrameXorValid0x7F(fr)) {
            Log::Printf(
                "PrMemCardBackend memcard XOR mismatch frame=%d file=%s",
                frame,
                path.string().c_str());
        }
    }
}

std::string FilterSuffixForHeaderTitle(const std::string& rawSuffix)
{
    std::string out;
    out.reserve(12);
    for (char c : rawSuffix) {
        if (out.size() >= 12u) {
            break;
        }
        const uint8_t ch = static_cast<uint8_t>(c);
        out.push_back(IsAllowedSuffixChar(ch) ? c : '?');
    }
    return out;
}

std::string FilterSuffixForDirName(const std::string& rawSuffix)
{
    std::string out;
    out.reserve(12);
    for (char c : rawSuffix) {
        if (out.size() >= 12u) {
            break;
        }
        const uint8_t ch = static_cast<uint8_t>(c);
        out.push_back(IsAllowedSuffixChar(ch) ? c : '?');
    }
    return out;
}

std::string NormalizeInternalFilename(const char* rawName)
{
    if (!rawName || rawName[0] == '\0') {
        return {};
    }

    std::string out;
    out.reserve(20);
    for (std::size_t i = 0; i < 20u && rawName[i] != '\0'; ++i) {
        const uint8_t ch = static_cast<uint8_t>(rawName[i]);
        if (ch < 0x20u || ch > 0x7Eu) {
            return {};
        }
        out.push_back(static_cast<char>(ch));
    }
    return out;
}

bool WriteTitleShiftJis(uint8_t* header, const std::string& utf8Title)
{
    if (!header) {
        return false;
    }

    const int wlen =
        MultiByteToWideChar(CP_UTF8, 0, utf8Title.c_str(), -1, nullptr, 0);
    if (wlen <= 0) {
        return false;
    }

    std::wstring w;
    w.resize(static_cast<std::size_t>(wlen));
    if (MultiByteToWideChar(
            CP_UTF8, 0, utf8Title.c_str(), -1, w.data(), wlen) <= 0) {
        return false;
    }

    const int slen = WideCharToMultiByte(
        932, 0, w.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (slen <= 0) {
        return false;
    }

    std::string sjis;
    sjis.resize(static_cast<std::size_t>(slen));
    if (WideCharToMultiByte(
            932, 0, w.c_str(), -1, sjis.data(), slen, nullptr, nullptr) <=
        0) {
        return false;
    }

    constexpr std::size_t titleMax = 0x40u;
    constexpr std::size_t titleOff = 0x04u;
    const std::size_t copyN = sjis.size() < titleMax ? sjis.size() : titleMax;
    for (std::size_t i = 0; i < copyN; ++i) {
        header[titleOff + i] = static_cast<uint8_t>(sjis[i]);
    }
    return true;
}

std::filesystem::path GetExecutableDir()
{
    wchar_t path[MAX_PATH];
    const DWORD len = GetModuleFileNameW(nullptr, path, MAX_PATH);
    if (len == 0 || len >= MAX_PATH) {
        return std::filesystem::current_path();
    }
    return std::filesystem::path(path).parent_path();
}

std::filesystem::path GetSaveDir()
{
    return GetExecutableDir() / L"save";
}

std::filesystem::path GetMemCardImagePath(int slot)
{
    const int id = (slot == 0) ? 0 : 10;
    char buf[16];
    sprintf_s(buf, "bu%02d.mcr", id);
    return GetSaveDir() / std::filesystem::path(buf);
}

std::filesystem::path ResolvePrimaryMemCardImagePath()
{
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

bool ReadFileExact(const std::filesystem::path& path,
                   void* dst,
                   std::size_t size)
{
    std::ifstream f(path, std::ios::binary);
    if (!f.is_open()) {
        return false;
    }
    f.read(reinterpret_cast<char*>(dst), static_cast<std::streamsize>(size));
    return f.gcount() == static_cast<std::streamsize>(size);
}

bool WriteFileAtomic(const std::filesystem::path& path,
                     const void* data,
                     std::size_t size)
{
    std::error_code ec;
    std::filesystem::create_directories(path.parent_path(), ec);

    std::filesystem::path tmp = path;
    tmp += L".tmp";
    {
        std::ofstream f(tmp, std::ios::binary | std::ios::trunc);
        if (!f.is_open()) {
            return false;
        }
        f.write(reinterpret_cast<const char*>(data),
                static_cast<std::streamsize>(size));
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

void EnsureAssetsLoaded()
{
    if (s_assetsLoaded) {
        return;
    }
    s_assetsLoaded = true;

    const std::filesystem::path base = GetSaveDir();
    const std::filesystem::path clutPath = base / L"prcard_clut.bin";
    const std::filesystem::path iconPath = base / L"prcard_icon.bin";

    if (ReadFileExact(clutPath, s_clutBytes.data(), s_clutBytes.size())) {
        Log::Printf("PrMemCardBackend assets: loaded clut %s",
                    clutPath.string().c_str());
    } else {
        Log::Printf("PrMemCardBackend assets: missing clut %s",
                    clutPath.string().c_str());
    }

    if (ReadFileExact(iconPath, s_iconBytes.data(), s_iconBytes.size())) {
        Log::Printf("PrMemCardBackend assets: loaded icons %s",
                    iconPath.string().c_str());
    } else {
        Log::Printf("PrMemCardBackend assets: missing icons %s",
                    iconPath.string().c_str());
    }
}

std::string MakeInternalFilenameForMemCard(int slot)
{
    const std::string rawSuffix = "SLOT" + std::to_string(slot);
    std::string name =
        std::string(kMemCardFilePrefix) + FilterSuffixForDirName(rawSuffix);
    if (name.size() > 20u) {
        name.resize(20u);
    }
    return name;
}

void FormatNewMemCardImage(uint8_t* img)
{
    if (!img) {
        return;
    }
    std::memset(img, 0, kMemCardImageSize);
    img[0] = 'M';
    img[1] = 'C';
    img[0x7F] = ComputeXorChecksum0x7F(img);

    for (int block = 1; block <= static_cast<int>(kMemCardDataBlockCount);
         ++block) {
        uint8_t* ent =
            img + static_cast<std::size_t>(block) * kMemCardFrameSize;
        std::memset(ent, 0, kMemCardFrameSize);
        WriteU32LE(ent + 0x00, 0x000000A0u);
        WriteU32LE(ent + 0x04, 0u);
        WriteU16LE(ent + 0x08, 0xFFFFu);
        ent[0x1F] = 0;
        ent[0x7F] = ComputeXorChecksum0x7F(ent);
    }

    for (int frame = 16; frame <= 35; ++frame) {
        uint8_t* fr =
            img + static_cast<std::size_t>(frame) * kMemCardFrameSize;
        std::memset(fr, 0, kMemCardFrameSize);
        WriteU32LE(fr + 0x00, 0xFFFFFFFFu);
        fr[0x7F] = ComputeXorChecksum0x7F(fr);
    }

    FixControlFramesXor(img);
}

bool LoadMemCardImage(const std::filesystem::path& path, uint8_t* outImg)
{
    if (!outImg) {
        return false;
    }

    std::ifstream f(path, std::ios::binary);
    if (!f.is_open()) {
        return false;
    }
    f.read(reinterpret_cast<char*>(outImg),
           static_cast<std::streamsize>(kMemCardImageSize));
    if (f.gcount() != static_cast<std::streamsize>(kMemCardImageSize)) {
        return false;
    }
    if (outImg[0] != 'M' || outImg[1] != 'C') {
        return false;
    }

    ValidateControlFramesXor(outImg, path);
    return true;
}

std::string ReadDirEntryNameAscii(const uint8_t* ent)
{
    if (!ent) {
        return {};
    }

    std::string out;
    out.reserve(20);
    const uint8_t* name = ent + 0x0Au;
    for (std::size_t i = 0; i < 20u; ++i) {
        const uint8_t c = name[i];
        if (c == 0u) {
            break;
        }
        out.push_back((c >= 0x20u && c <= 0x7Eu) ? static_cast<char>(c)
                                                  : '?');
    }
    return out;
}

std::string ReadHeaderTitleAscii(const uint8_t* block)
{
    if (!block) {
        return {};
    }

    const uint8_t* title = block + 0x04u;
    std::string out;
    out.reserve(32);
    for (std::size_t i = 0; i < 0x40u; ++i) {
        const uint8_t c = title[i];
        if (c == 0u) {
            break;
        }
        out.push_back((c >= 0x20u && c <= 0x7Eu) ? static_cast<char>(c)
                                                  : ' ');
    }
    while (!out.empty() && out.back() == ' ') {
        out.pop_back();
    }

    const std::string prefix = "PARAPPA \"";
    if (out.size() > prefix.size() + 1u && out.rfind(prefix, 0) == 0 &&
        out.back() == '"') {
        return out.substr(prefix.size(), out.size() - prefix.size() - 1u);
    }
    return out;
}

int FindDirEntryByName(const uint8_t* img, const std::string& name)
{
    for (int block = 1; block <= static_cast<int>(kMemCardDataBlockCount);
         ++block) {
        const uint8_t* ent =
            img + static_cast<std::size_t>(block) * kMemCardFrameSize;
        if (!IsFrameXorValid0x7F(ent)) {
            continue;
        }
        const uint8_t s = static_cast<uint8_t>(ReadU32LE(ent + 0x00) & 0xFFu);
        if (s != 0x51u && s != 0x52u && s != 0x53u) {
            continue;
        }
        const char* fn = reinterpret_cast<const char*>(ent + 0x0A);
        if (std::strncmp(fn, name.c_str(), 20) == 0) {
            return block;
        }
    }
    return -1;
}

int FindFirstFreeDirEntry(const uint8_t* img)
{
    for (int block = 1; block <= static_cast<int>(kMemCardDataBlockCount);
         ++block) {
        const uint8_t* ent =
            img + static_cast<std::size_t>(block) * kMemCardFrameSize;
        const uint8_t s = static_cast<uint8_t>(ReadU32LE(ent + 0x00) & 0xFFu);
        if (s == 0xA0u || s == 0xA1u || s == 0xA2u || s == 0xA3u) {
            return block;
        }
    }
    return -1;
}

void WriteDirEntry(uint8_t* img, int block, const std::string& name)
{
    uint8_t* ent = img + static_cast<std::size_t>(block) * kMemCardFrameSize;
    std::memset(ent, 0, kMemCardFrameSize);
    WriteU32LE(ent + 0x00, 0x00000051u);
    WriteU32LE(ent + 0x04, static_cast<uint32_t>(kMemCardBlockSize));
    WriteU16LE(ent + 0x08, 0xFFFFu);
    const std::size_t copyN = name.size() < 20u ? name.size() : 20u;
    std::memcpy(ent + 0x0A, name.data(), copyN);
    ent[0x1F] = 0;
    ent[0x7F] = ComputeXorChecksum0x7F(ent);
}

bool UpdateMemCardImageBlock(const std::filesystem::path& imgPath,
                             int blockIndex,
                             int titleSlot,
                             const uint8_t* saveBlock8K)
{
    if (!saveBlock8K || blockIndex < 0 ||
        blockIndex >= static_cast<int>(kMemCardDataBlockCount)) {
        return false;
    }

    std::array<uint8_t, kMemCardImageSize> img{};
    if (!LoadMemCardImage(imgPath, img.data())) {
        Log::Printf(
            "PrMemCardBackend::UpdateMemCardImageBlock rejected unreadable image=%s",
            imgPath.string().c_str());
        return false;
    }

    const int block = blockIndex + 1;
    WriteDirEntry(img.data(), block, MakeInternalFilenameForMemCard(titleSlot));
    uint8_t* dst = img.data() + static_cast<std::size_t>(block) *
                                    kMemCardBlockSize;
    std::memcpy(dst, saveBlock8K, kMemCardBlockSize);
    FixControlFramesXor(img.data());
    return WriteFileAtomic(imgPath, img.data(), img.size());
}

EntryWriteResult SaveMemCardImageBlockByName(
    const std::filesystem::path& imgPath,
    const std::string& internalName,
    const uint8_t* saveBlock8K)
{
    EntryWriteResult result{};
    result.attempted = true;
    if (internalName.empty() || !saveBlock8K ||
        saveBlock8K[0] != 'S' || saveBlock8K[1] != 'C') {
        return result;
    }

    std::array<uint8_t, kMemCardImageSize> img{};
    if (!LoadMemCardImage(imgPath, img.data())) {
        Log::Printf(
            "PrMemCardBackend::SaveMemCardImageBlockByName rejected unreadable image=%s",
            imgPath.string().c_str());
        return result;
    }

    int block = FindDirEntryByName(img.data(), internalName);
    if (block >= 1) {
        result.overwrote = true;
    } else {
        block = FindFirstFreeDirEntry(img.data());
    }
    if (block < 1 ||
        block > static_cast<int>(kMemCardDataBlockCount)) {
        return result;
    }

    result.blockIndex = block - 1;
    WriteDirEntry(img.data(), block, internalName);
    uint8_t* dst = img.data() + static_cast<std::size_t>(block) *
                                    kMemCardBlockSize;
    std::memcpy(dst, saveBlock8K, kMemCardBlockSize);
    FixControlFramesXor(img.data());
    result.saved = WriteFileAtomic(imgPath, img.data(), img.size());
    if (result.saved) {
        std::memcpy(s_payload.data(),
                    saveBlock8K + kCardPayloadOffset,
                    s_payload.size());
    }
    return result;
}

bool LoadBlockFromMemCardImageBlock(const std::filesystem::path& imgPath,
                                     int blockIndex,
                                    uint8_t* outBlock8K)
{
    if (!outBlock8K || blockIndex < 0 ||
        blockIndex >= static_cast<int>(kMemCardDataBlockCount)) {
        return false;
    }

    std::array<uint8_t, kMemCardImageSize> img{};
    if (!LoadMemCardImage(imgPath, img.data())) {
        return false;
    }

    const int block = blockIndex + 1;
    const uint8_t* ent =
        img.data() + static_cast<std::size_t>(block) * kMemCardFrameSize;
    if (!IsFrameXorValid0x7F(ent)) {
        return false;
    }
    const uint8_t s = static_cast<uint8_t>(ReadU32LE(ent + 0x00) & 0xFFu);
    if (s != 0x51u && s != 0x52u && s != 0x53u) {
        return false;
    }

    const uint8_t* src =
        img.data() + static_cast<std::size_t>(block) * kMemCardBlockSize;
    if (src[0] != 'S' || src[1] != 'C') {
        return false;
    }
    std::memcpy(outBlock8K, src, kCardBlockSize);
    return true;
}

void BuildHeader(uint8_t* header, int slot)
{
    EnsureAssetsLoaded();
    std::memset(header, 0, kCardHeaderSize);
    header[0] = 'S';
    header[1] = 'C';
    header[2] = 0x13u;
    header[3] = 1u;

    const std::string suffix =
        FilterSuffixForHeaderTitle("SLOT" + std::to_string(slot));
    const std::string title = std::string("PARAPPA \"") + suffix + "\"";
    if (!WriteTitleShiftJis(header, title)) {
        constexpr std::size_t titleMax = 0x40u;
        constexpr std::size_t titleOff = 0x04u;
        const std::size_t copyN =
            title.size() < titleMax ? title.size() : titleMax;
        for (std::size_t i = 0; i < copyN; ++i) {
            header[titleOff + i] = static_cast<uint8_t>(title[i]);
        }
    }

    std::memcpy(header + kCardClutOffset, s_clutBytes.data(),
                s_clutBytes.size());
    std::memcpy(header + kCardIconsOffset, s_iconBytes.data(),
                s_iconBytes.size());
}

void RestorePayloadSnapshot(const std::vector<uint8_t>& snapshot)
{
    if (snapshot.size() != s_payload.size()) {
        return;
    }
    std::memcpy(s_payload.data(), snapshot.data(), snapshot.size());
}

int ResolveReplaySceneFromPayload()
{
    const uint32_t savedSlot = ReadPayloadU32(kStageLastSavedSlotPayloadOffset);
    return (savedSlot < 6u) ? static_cast<int>(savedSlot + 1u) : -1;
}

uint32_t ReadReplaySavedSlotFromPayload()
{
    return ReadPayloadU32(kStageLastSavedSlotPayloadOffset);
}

} // namespace

uint32_t ReadPayloadU32(std::size_t offset)
{
    if (offset + sizeof(uint32_t) > s_payload.size()) {
        return 0u;
    }
    return static_cast<uint32_t>(s_payload[offset + 0u]) |
           (static_cast<uint32_t>(s_payload[offset + 1u]) << 8) |
           (static_cast<uint32_t>(s_payload[offset + 2u]) << 16) |
           (static_cast<uint32_t>(s_payload[offset + 3u]) << 24);
}

uint8_t ReadStageStatus(int stage)
{
    bool anyPrimaryStatus = false;
    if (s_payload.size() > kStageStatusPayloadBase + 7u) {
        for (int i = 1; i <= 6; ++i) {
            anyPrimaryStatus |=
                std::clamp(static_cast<int>(
                               s_payload[kStageStatusPayloadBase +
                                         static_cast<std::size_t>(i)]),
                           0,
                           3) != 0;
        }
    }
    if (!anyPrimaryStatus) {
        return (stage == 1) ? 1u : 0u;
    }
    if (stage < 1 || stage > 7 ||
        s_payload.size() <=
            kStageStatusPayloadBase + static_cast<std::size_t>(stage)) {
        return 0u;
    }
    return static_cast<uint8_t>(
        std::clamp(static_cast<int>(
                       s_payload[kStageStatusPayloadBase +
                                 static_cast<std::size_t>(stage)]),
                   0,
                   3));
}

const uint8_t* CurrentPayload()
{
    return s_payload.data();
}

uint8_t* MutablePayload()
{
    return s_payload.data();
}

std::size_t PayloadSize()
{
    return s_payload.size();
}

int EnumerateEntriesCompact(EntryInfo outEntries[15])
{
    if (!outEntries) {
        return 0;
    }
    for (int i = 0; i < 15; ++i) {
        outEntries[i] = EntryInfo{};
    }

    SlotInfo slots[15]{};
    if (!EnumerateSlots(slots)) {
        return 0;
    }

    int count = 0;
    for (int i = 0; i < 15 && count < 15; ++i) {
        if (!slots[i].occupied) {
            continue;
        }
        const std::string displayTitle =
            !slots[i].title.empty() ? slots[i].title : slots[i].internalName;
        if (displayTitle.empty()) {
            continue;
        }
        outEntries[count].title = displayTitle;
        outEntries[count].internalName = slots[i].internalName;
        outEntries[count].blockIndex = i;
        ++count;
    }
    Log::Printf("PrMemCardBackend::EnumerateEntriesCompact count=%d", count);
    return count;
}

bool EnumerateSlots(SlotInfo outSlots[15])
{
    if (!outSlots) {
        return false;
    }
    for (int i = 0; i < 15; ++i) {
        outSlots[i] = SlotInfo{};
    }

    std::array<uint8_t, kMemCardImageSize> img{};
    const std::filesystem::path imgPath = ResolvePrimaryMemCardImagePath();
    if (!LoadMemCardImage(imgPath, img.data())) {
        Log::Printf("PrMemCardBackend::EnumerateSlots no memcard image path=%s",
                    imgPath.string().c_str());
        return false;
    }

    int occupiedCount = 0;
    for (int block = 1; block <= static_cast<int>(kMemCardDataBlockCount);
         ++block) {
        const uint8_t* ent =
            img.data() + static_cast<std::size_t>(block) * kMemCardFrameSize;
        if (!IsFrameXorValid0x7F(ent)) {
            continue;
        }
        const uint8_t s = static_cast<uint8_t>(ReadU32LE(ent + 0x00) & 0xFFu);
        if (s != 0x51u && s != 0x52u && s != 0x53u) {
            continue;
        }

        const uint8_t* saveBlock =
            img.data() + static_cast<std::size_t>(block) * kMemCardBlockSize;
        if (saveBlock[0] != 'S' || saveBlock[1] != 'C') {
            continue;
        }

        SlotInfo info{};
        info.occupied = true;
        info.title = ReadHeaderTitleAscii(saveBlock);
        info.internalName = ReadDirEntryNameAscii(ent);
        outSlots[block - 1] = info;
        ++occupiedCount;
    }
    Log::Printf("PrMemCardBackend::EnumerateSlots ok image=%s occupied=%d",
                imgPath.string().c_str(),
                occupiedCount);
    return true;
}

bool FormatPrimaryCard()
{
    std::array<uint8_t, kMemCardImageSize> img{};
    FormatNewMemCardImage(img.data());
    const std::filesystem::path imgPath = ResolvePrimaryMemCardImagePath();
    const bool ok = WriteFileAtomic(imgPath, img.data(), img.size());
    Log::Printf("PrMemCardBackend::FormatPrimaryCard ok=%d image=%s",
                ok ? 1 : 0,
                imgPath.string().c_str());
    return ok;
}

bool LoadEntry(int blockIndex)
{
    std::array<uint8_t, kCardBlockSize> block{};
    const std::filesystem::path imgPath = ResolvePrimaryMemCardImagePath();
    if (!LoadBlockFromMemCardImageBlock(imgPath, blockIndex, block.data())) {
        Log::Printf("PrMemCardBackend::LoadEntry block=%d failed image=%s",
                    blockIndex,
                    imgPath.string().c_str());
        return false;
    }

    std::memcpy(s_payload.data(), block.data() + kCardPayloadOffset,
                s_payload.size());
    Log::Printf("PrMemCardBackend::LoadEntry block=%d ok image=%s",
                blockIndex,
                imgPath.string().c_str());
    return true;
}

bool SaveEntry(int blockIndex)
{
    std::array<uint8_t, kCardBlockSize> block{};
    BuildHeader(block.data(), blockIndex);
    std::memcpy(block.data() + kCardPayloadOffset, s_payload.data(),
                s_payload.size());

    const std::filesystem::path imgPath = ResolvePrimaryMemCardImagePath();
    if (!UpdateMemCardImageBlock(imgPath, blockIndex, blockIndex, block.data())) {
        Log::Printf("PrMemCardBackend::SaveEntry block=%d failed image=%s",
                    blockIndex,
                    imgPath.string().c_str());
        return false;
    }

    Log::Printf("PrMemCardBackend::SaveEntry block=%d ok image=%s",
                blockIndex,
                imgPath.string().c_str());
    return true;
}

EntryWritePreflightResult CanSaveEntryBlockByName(const char* internalName,
                                                  const uint8_t* saveBlock8K,
                                                  std::size_t saveBlockSize)
{
    EntryWritePreflightResult result{};
    const std::string normalizedName =
        NormalizeInternalFilename(internalName);
    if (normalizedName.empty() ||
        !saveBlock8K ||
        saveBlockSize < kMemCardBlockSize) {
        return result;
    }

    result.requestValid = true;
    result.blockBytesValid = saveBlock8K[0] == 'S' && saveBlock8K[1] == 'C';
    if (!result.blockBytesValid) {
        return result;
    }

    std::array<uint8_t, kMemCardImageSize> img{};
    const std::filesystem::path imgPath = ResolvePrimaryMemCardImagePath();
    if (!LoadMemCardImage(imgPath, img.data())) {
        return result;
    }
    result.imageReadable = true;

    int block = FindDirEntryByName(img.data(), normalizedName);
    if (block >= 1) {
        result.overwrote = true;
    } else {
        block = FindFirstFreeDirEntry(img.data());
    }
    if (block < 1 ||
        block > static_cast<int>(kMemCardDataBlockCount)) {
        return result;
    }

    result.writable = true;
    result.blockIndex = block - 1;
    return result;
}

EntryWriteResult SaveEntryBlockByName(const char* internalName,
                                      const uint8_t* saveBlock8K,
                                      std::size_t saveBlockSize)
{
    EntryWriteResult result{};
    const std::string normalizedName =
        NormalizeInternalFilename(internalName);
    if (normalizedName.empty() ||
        !saveBlock8K ||
        saveBlockSize < kMemCardBlockSize) {
        result.attempted = normalizedName.empty() || saveBlock8K != nullptr;
        Log::Printf(
            "PrMemCardBackend::SaveEntryBlockByName rejected nameKnown=%d blockKnown=%d size=%zu",
            normalizedName.empty() ? 0 : 1,
            saveBlock8K ? 1 : 0,
            saveBlockSize);
        return result;
    }

    const std::filesystem::path imgPath = ResolvePrimaryMemCardImagePath();
    result =
        SaveMemCardImageBlockByName(imgPath, normalizedName, saveBlock8K);
    Log::Printf(
        "PrMemCardBackend::SaveEntryBlockByName name=%s block=%d attempted=%d saved=%d overwrote=%d image=%s",
        normalizedName.c_str(),
        result.blockIndex,
        result.attempted ? 1 : 0,
        result.saved ? 1 : 0,
        result.overwrote ? 1 : 0,
        imgPath.string().c_str());
    return result;
}

bool LoadEntryPayloadToBlock(int blockIndex,
                             uint8_t* outBlock,
                             std::size_t outBlockSize,
                             std::size_t payloadOffset)
{
    if (!outBlock || !LoadEntry(blockIndex)) {
        return false;
    }
    if (payloadOffset > outBlockSize ||
        s_payload.size() > outBlockSize - payloadOffset) {
        return false;
    }
    std::memcpy(outBlock + payloadOffset, s_payload.data(), s_payload.size());
    return true;
}

ReplayEntryLoadResult LoadReplayEntry(int blockIndex)
{
    ReplayEntryLoadResult result{};

    std::vector<uint8_t> snapshot;
    snapshot.assign(s_payload.begin(), s_payload.end());

    if (!LoadEntry(blockIndex)) {
        RestorePayloadSnapshot(snapshot);
        return result;
    }

    result.entryLoaded = true;
    result.rawSavedSlotKnown = true;
    result.rawSavedSlot = ReadReplaySavedSlotFromPayload();
    result.replayScene = ResolveReplaySceneFromPayload();
    if (result.replayScene >= 1 && result.replayScene <= 6) {
        result.payloadCommitted = true;
        return result;
    }

    RestorePayloadSnapshot(snapshot);
    result.replayScene = -1;
    return result;
}

} // namespace PrMemCardBackend
