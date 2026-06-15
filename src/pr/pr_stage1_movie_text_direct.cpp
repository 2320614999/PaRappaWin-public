#include "pr_stage1_movie_text_direct.h"

#include "pr_stage1_overlay_parser.h"
#include "pr_vtext.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace PrStage1MovieTextDirect {
namespace {

static uint16_t ReadU16LE(const uint8_t* p) {
    return (uint16_t)((uint16_t)p[0] | ((uint16_t)p[1] << 8));
}

static uint32_t ReadU32LE(const uint8_t* p) {
    return (uint32_t)p[0] |
           ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) |
           ((uint32_t)p[3] << 24);
}

// Stage1 stays on COMOD1.BIN, but keep the mapping table-shaped so later
// stage overlays can add their own PSX base instead of reintroducing probing.
static constexpr PsxOverlayAddressResolver kStage1ComodOverlayAddressResolvers[] = {
    {PrStage1OverlayData::kPsxBase, 0u},
};

static constexpr PsxMovieTextLoadSpec kStage1Movie1TextLoadSpec = {
    kStage1ComodOverlayAddressResolvers,
    sizeof(kStage1ComodOverlayAddressResolvers) / sizeof(kStage1ComodOverlayAddressResolvers[0]),
    {kMovie1SubtitleDescAddr, kMovie1SubtitleEntryCount},
    {{
        {kStage1CommonLyricsDescAddrByMode[0], kStage1CommonLyricsEntryCountByMode[0]},
        {kStage1CommonLyricsDescAddrByMode[1], kStage1CommonLyricsEntryCountByMode[1]},
        {kStage1CommonLyricsDescAddrByMode[2], kStage1CommonLyricsEntryCountByMode[2]},
    }},
};

static PsxMovieTextLoadSpec BuildMovieTextLoadSpecFromPtr800943CC(
    uint32_t ptr800943CC) {
    PsxMovieTextLoadSpec out = kStage1Movie1TextLoadSpec;
    if (ptr800943CC == 0u) {
        return out;
    }

    for (size_t mode = 0; mode < out.commonLyrics.size(); ++mode) {
        out.commonLyrics[mode].descAddr =
            ptr800943CC + static_cast<uint32_t>(mode * sizeof(PrVTextEventTableDesc));
    }
    out.movieSubtitle.descAddr = out.commonLyrics[0].descAddr;
    return out;
}

static uint32_t ClampFrame30ToU32(int32_t frame30) {
    return frame30 > 0 ? static_cast<uint32_t>(frame30) : 0u;
}

static bool ResolvePsxOverlayAddressToOffset(
    const std::vector<uint8_t>& bytes,
    const PsxOverlayAddressResolver& resolver,
    uint32_t psxAddr,
    size_t byteCount,
    size_t& outOff) {
    outOff = 0;
    if (psxAddr < resolver.psxBase) {
        return false;
    }

    const uint64_t off64 =
        (uint64_t)resolver.fileOffsetBase + (uint64_t)(psxAddr - resolver.psxBase);
    if (off64 > (uint64_t)bytes.size()) {
        return false;
    }

    const uint64_t end64 = off64 + (uint64_t)byteCount;
    if (end64 > (uint64_t)bytes.size() || end64 < off64) {
        return false;
    }

    outOff = (size_t)off64;
    return true;
}

// IDA bytes from tools/auto_port/output/text_glyph_metrics_8001b954_20260509.
// Do not regenerate this table from on-disk SCUS offsets.
static constexpr char kGlyphMetricRaw8004945C[] =
    "\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\x00\x00\x00\x00"
    "\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\x00\x00\x00\x00\x59\x00\x3C\x00\x0C\x0F\x00\x00\x65\x00\x3C\x00\x0C\x0F\x00\x00"
    "\x00\x00\x00\x00\x05\x0F\x00\x00\x23\x00\x00\x00\x05\x0F\x00\x00\x43\x00\x00\x00\x07\x0F\x00\x00\x65\x00\x00\x00\x0B\x0F\x00\x00\x51\x00\x00\x00\x08\x0F\x00\x00\x59\x00\x00\x00\x0C\x0F\x00\x00\x70\x00\x00\x00\x0A\x0F\x00\x00\x3D\x00\x00\x00\x05\x0F\xFF\xFF\x30\x00\x00\x00\x07\x0F\x00\x00\x37\x00\x00\x00\x07\x0F\x00\x00\x7A\x00\x00\x00\x0A\x0F\x00\x00\x4D\x00\x3C\x00\x0C\x0F\x00\x00\x08\x00\x00\x00\x07\x0F\xFF\xFF\x4A\x00\x00\x00\x07\x0F\x00\x00\x0F\x00\x00\x00\x06\x0F\x00\x00\xFF\xFF\xFF\xFF\x00\x00\x00\x00"
    "\x84\x00\x00\x00\x08\x0F\x00\x00\x8C\x00\x00\x00\x07\x0F\x00\x00\x93\x00\x00\x00\x07\x0F\x00\x00\x9A\x00\x00\x00\x07\x0F\x00\x00\xA1\x00\x00\x00\x08\x0F\x00\x00\xA9\x00\x00\x00\x08\x0F\x00\x00\xB1\x00\x00\x00\x08\x0F\x00\x00\xB9\x00\x00\x00\x08\x0F\x00\x00\xC1\x00\x00\x00\x09\x0F\x00\x00\xCA\x00\x00\x00\x07\x0F\x00\x00\x05\x00\x3C\x00\x04\x0F\x00\x00\x00\x00\x3C\x00\x04\x0F\x00\x00\x2E\x00\x3C\x00\x0C\x0F\xFF\xFF\x0A\x00\x3C\x00\x0C\x0F\x00\x00\x39\x00\x3C\x00\x0C\x0F\x00\x00\x1B\x00\x00\x00\x08\x0F\x00\x00"
    "\xEF\x00\x2D\x00\x0B\x0F\x00\x00\xD1\x00\x00\x00\x08\x0F\x00\x00\xD9\x00\x00\x00\x09\x0F\x00\x00\xE2\x00\x00\x00\x07\x0F\x00\x00\xE9\x00\x00\x00\x07\x0F\x00\x00\xF1\x00\x00\x00\x06\x0F\x01\x00\xF6\x00\x00\x00\x08\x0F\x00\x00\x00\x00\x0F\x00\x08\x0F\x00\x00\x08\x00\x0F\x00\x07\x0F\x00\x00\x0F\x00\x0F\x00\x07\x0F\x01\x00\x16\x00\x0F\x00\x07\x0F\x00\x00\x1D\x00\x0F\x00\x07\x0F\x00\x00\x24\x00\x0F\x00\x08\x0F\x00\x00\x2C\x00\x0F\x00\x0B\x0F\x00\x00\x37\x00\x0F\x00\x08\x0F\x00\x00\x3F\x00\x0F\x00\x08\x0F\x00\x00"
    "\x47\x00\x0F\x00\x08\x0F\x00\x00\x4F\x00\x0F\x00\x09\x0F\x00\x00\x58\x00\x0F\x00\x08\x0F\x00\x00\x60\x00\x0F\x00\x07\x0F\x00\x00\x67\x00\x0F\x00\x07\x0F\x00\x00\x6F\x00\x0F\x00\x07\x0F\x01\x00\x75\x00\x0F\x00\x09\x0F\x00\x00\x7F\x00\x0F\x00\x0B\x0F\x01\x00\x89\x00\x0F\x00\x08\x0F\x00\x00\x91\x00\x0F\x00\x09\x0F\x00\x00\x9A\x00\x0F\x00\x08\x0F\x00\x00\x7D\x00\x3C\x00\x0C\x0F\x00\x00\x63\x00\x1E\x00\x08\x0F\x00\x00\x89\x00\x3C\x00\x0C\x0F\x00\x00\x28\x00\x00\x00\x08\x0F\x00\x00\x45\x00\x3C\x00\x08\x0F\x00\x00"
    "\x15\x00\x00\x00\x06\x0F\x00\x00\xA2\x00\x0F\x00\x08\x0F\x00\x00\xAA\x00\x0F\x00\x07\x0F\x00\x00\xB2\x00\x0F\x00\x07\x0F\x00\x00\xB9\x00\x0F\x00\x08\x0F\x00\x00\xC1\x00\x0F\x00\x07\x0F\x00\x00\xC8\x00\x0F\x00\x07\x0F\x00\x00\xCF\x00\x0F\x00\x07\x0F\x00\x00\xD6\x00\x0F\x00\x08\x0F\x00\x00\xDF\x00\x0F\x00\x05\x0F\x01\x00\xE3\x00\x0F\x00\x06\x0F\x00\x00\xE9\x00\x0F\x00\x06\x0F\x00\x00\xEF\x00\x0F\x00\x04\x0F\x00\x00\xF3\x00\x0F\x00\x08\x0F\x00\x00\x00\x00\x1E\x00\x07\x0F\x00\x00\x07\x00\x1E\x00\x07\x0F\x00\x00"
    "\x0F\x00\x1E\x00\x07\x0F\x00\x00\x15\x00\x1E\x00\x07\x0F\x00\x00\x1C\x00\x1E\x00\x07\x0F\x00\x00\x23\x00\x1E\x00\x08\x0F\x00\x00\x2B\x00\x1E\x00\x07\x0F\x00\x00\x32\x00\x1E\x00\x08\x0F\x00\x00\x3A\x00\x1E\x00\x08\x0F\x00\x00\x41\x00\x1E\x00\x09\x0F\x00\x00\x4B\x00\x1E\x00\x07\x0F\x00\x00\x52\x00\x1E\x00\x08\x0F\x00\x00\x5A\x00\x1E\x00\x09\x0F\x00\x00\x7A\x00\x00\x00\x0A\x0F\x00\x00\x17\x00\x3C\x00\x0C\x0F\x00\x00\x71\x00\x3C\x00\x0C\x0F\xFF\xFF\x22\x00\x3C\x00\x0C\x0F\x00\x00\xFF\xFF\xFF\xFF\x00\x00\x00\x00"
    "\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\x00\x00\x00\x00\x16\x00\x2D\x00\x0B\x0F\x00\x00\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\x00\x00\x00\x00"
    "\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\x00\x00\x00\x00\x2A\x00\x2D\x00\x0B\x0F\x00\x00\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\x00\x00\x00\x00"
    "\xFF\xFF\xFF\xFF\x00\x00\x00\x00\x3D\x00\x2D\x00\x05\x0F\x00\x00\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\x00\x00\x00\x00\x42\x00\x2D\x00\x08\x0F\x00\x00\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\x00\x00\x00\x00"
    "\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xF9\x00\x2D\x00\x05\x0F\x01\x00\x4A\x00\x2D\x00\x08\x0F\x00\x00\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\x00\x00\x00\x00\x35\x00\x2D\x00\x08\x0F\x00\x00"
    "\x52\x00\x2D\x00\x08\x0F\x00\x00\x5B\x00\x2D\x00\x08\x0F\x01\x00\x62\x00\x2D\x00\x08\x0F\x00\x00\xFF\xFF\xFF\xFF\x00\x00\x00\x00\x6B\x00\x2D\x00\x08\x0F\x01\x00\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\x00\x00\x00\x00\x0F\x00\x2D\x00\x07\x0F\x00\x00\x72\x00\x2D\x00\x06\x0F\x00\x00\x78\x00\x2D\x00\x07\x0F\x00\x00\x7F\x00\x2D\x00\x07\x0F\x00\x00\x86\x00\x2D\x00\x07\x0F\x00\x00\x8D\x00\x2D\x00\x07\x0F\x01\x00\x94\x00\x2D\x00\x07\x0F\x01\x00\x9B\x00\x2D\x00\x07\x0F\x01\x00\xA2\x00\x2D\x00\x07\x0F\x01\x00"
    "\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xE6\x00\x2D\x00\x08\x0F\x00\x00\xA9\x00\x2D\x00\x08\x0F\x00\x00\xB1\x00\x2D\x00\x08\x0F\x00\x00\xB9\x00\x2D\x00\x08\x0F\x00\x00\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xC1\x00\x2D\x00\x08\x0F\x00\x00\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xC9\x00\x2D\x00\x07\x0F\x00\x00\xD1\x00\x2D\x00\x07\x0F\x01\x00\xD7\x00\x2D\x00\x08\x0F\x00\x00\xDF\x00\x2D\x00\x07\x0F\x00\x00\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\x00\x00\x00\x00\x21\x00\x2D\x00\x09\x0F\x00\x00"
    "\x6B\x00\x1E\x00\x08\x0F\x00\x00\x73\x00\x1E\x00\x08\x0F\x00\x00\x7B\x00\x1E\x00\x08\x0F\x00\x00\xFF\xFF\xFF\xFF\x00\x00\x00\x00\x83\x00\x1E\x00\x08\x0F\x00\x00\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\x00\x00\x00\x00\x08\x00\x2D\x00\x07\x0F\x00\x00\x8B\x00\x1E\x00\x08\x0F\x00\x00\x92\x00\x1E\x00\x07\x0F\x00\x00\x99\x00\x1E\x00\x08\x0F\x00\x00\xA1\x00\x1E\x00\x07\x0F\x00\x00\xA8\x00\x1E\x00\x05\x0F\x00\x00\xAD\x00\x1E\x00\x05\x0F\x00\x00\xB2\x00\x1E\x00\x06\x0F\x00\x00\xB8\x00\x1E\x00\x06\x0F\x00\x00"
    "\xFF\xFF\xFF\xFF\x00\x00\x00\x00\x00\x00\x2D\x00\x08\x0F\x00\x00\xBE\x00\x1E\x00\x07\x0F\x00\x00\xC5\x00\x1E\x00\x07\x0F\x00\x00\xCD\x00\x1E\x00\x07\x0F\x00\x00\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xD3\x00\x1E\x00\x07\x0F\x00\x00\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xDB\x00\x1E\x00\x08\x0F\x00\x00\xE2\x00\x1E\x00\x08\x0F\x00\x00\xEB\x00\x1E\x00\x08\x0F\x00\x00\xF2\x00\x1E\x00\x08\x0F\x00\x00\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\x00\x00\x00\x00";

static_assert(sizeof(kGlyphMetricRaw8004945C) - 1u ==
              256u * kMovie1TextGlyphMetricTableStride8004945C,
              "8001B954 glyph metric table must stay byte-exact");

static bool ReadStringAtAddress(const std::vector<uint8_t>& bytes,
                                const PsxOverlayAddressResolver& resolver,
                                uint32_t addr,
                                std::string& out) {
    out.clear();
    size_t off = 0;
    if (!ResolvePsxOverlayAddressToOffset(bytes, resolver, addr, 1u, off)) {
        return false;
    }

    size_t i = off;
    while (i < bytes.size()) {
        const uint8_t c = bytes[i];
        if (c == 0) {
            break;
        }
        out.push_back((char)c);
        ++i;
        if (out.size() > 220) {
            break;
        }
    }
    return !out.empty();
}

static bool TryLoadVTextTrackAtDescriptor(const std::vector<uint8_t>& bytes,
                                          const PsxOverlayAddressResolver& resolver,
                                          uint32_t descAddr,
                                          size_t descOff,
                                          uint32_t expectedEntryCount,
                                          uint8_t languageIndex,
                                          PrMovieSubtitles::MovieSubtitleTrack& outTrack,
                                          std::vector<PrStage1VTextDirectLineMeta>* outLineMeta,
                                          PsxVTextDescriptorState* outDesc) {
    outTrack = {};
    if (outLineMeta) {
        outLineMeta->clear();
    }
    if (outDesc) {
        *outDesc = {};
    }
    if (descOff + sizeof(PrVTextEventTableDesc) > bytes.size()) {
        return false;
    }

    const uint8_t* p = bytes.data() + descOff;
    PrVTextEventTableDesc desc{};
    for (int i = 0; i < 5; ++i) {
        desc.textTables[i] = ReadU32LE(p + i * 4);
    }
    desc.entries = ReadU32LE(p + 20);
    desc.entryCount = ReadU32LE(p + 24);

    if (desc.entryCount < 1 || desc.entryCount > 512) {
        return false;
    }
    if (expectedEntryCount != 0 && desc.entryCount != expectedEntryCount) {
        return false;
    }

    PsxVTextDescriptorState descState{};
    descState.valid = true;
    descState.descAddr = descAddr;
    descState.descOffset = (uint32_t)descOff;
    descState.entriesAddr = desc.entries;
    descState.entryCount = desc.entryCount;
    for (int lang = 0; lang < 5; ++lang) {
        descState.textTableAddrs[(size_t)lang] = desc.textTables[lang];
    }
    descState.selectedTextTableAddr =
        desc.textTables[ResolveLanguageIndex(languageIndex)];

    size_t entriesOff = 0;
    if (!ResolvePsxOverlayAddressToOffset(
            bytes,
            resolver,
            desc.entries,
            (size_t)desc.entryCount * sizeof(PrVTextEventEntry),
            entriesOff)) {
        return false;
    }

    std::array<size_t, 5> textTableOffsets{};
    for (int lang = 0; lang < 5; ++lang) {
        if (!ResolvePsxOverlayAddressToOffset(
                bytes,
                resolver,
                desc.textTables[lang],
                4u,
                textTableOffsets[(size_t)lang])) {
            return false;
        }
    }

    std::vector<PrMovieSubtitles::MovieSubtitleLine> lines;
    std::vector<PrStage1VTextDirectLineMeta> lineMeta;
    lines.reserve(desc.entryCount);
    lineMeta.reserve(desc.entryCount);
    uint32_t prevKey = 0;
    bool first = true;
    for (uint32_t i = 0; i < desc.entryCount; ++i) {
        const uint8_t* ep =
            bytes.data() + entriesOff + (size_t)i * sizeof(PrVTextEventEntry);
        const uint16_t minute = ReadU16LE(ep + 0);
        const uint8_t second = ep[2];
        const uint8_t frame = ep[3];
        const uint16_t duration = ReadU16LE(ep + 4);

        if (second >= 60 || frame >= 30 || minute > 10 || duration > 300) {
            return false;
        }

        const uint32_t key =
            (uint32_t)minute * 1800u + (uint32_t)second * 30u + (uint32_t)frame;
        if (!first && key < prevKey) {
            return false;
        }
        first = false;
        prevKey = key;

        PrMovieSubtitles::MovieSubtitleLine line{};
        PrStage1VTextDirectLineMeta meta{};
        line.frame30 = key;
        line.duration = duration;

        for (int lang = 0; lang < 5; ++lang) {
            const int16_t idx = (int16_t)ReadU16LE(ep + 6 + lang * 2);
            line.textIndex[lang] = idx;
            if (idx <= 0) {
                continue;
            }

            const size_t tableOff = textTableOffsets[(size_t)lang];
            const uint64_t ptrPos = (uint64_t)tableOff + (uint64_t)(uint16_t)idx * 4ull;
            if (ptrPos + 4ull > (uint64_t)bytes.size()) {
                continue;
            }

            const uint32_t strAddr = ReadU32LE(bytes.data() + (size_t)ptrPos);
            meta.textAddrs[(size_t)lang] = strAddr;
            std::string text;
            if (ReadStringAtAddress(bytes, resolver, strAddr, text)) {
                line.texts[lang] = std::move(text);
            }
        }

        lines.push_back(std::move(line));
        lineMeta.push_back(meta);
    }

    outTrack.loaded = true;
    outTrack.descOffset = (uint32_t)descOff;
    outTrack.entryCount = desc.entryCount;
    outTrack.lines = std::move(lines);
    if (outLineMeta) {
        *outLineMeta = std::move(lineMeta);
    }
    if (outDesc) {
        *outDesc = descState;
    }
    return !outTrack.lines.empty();
}

static const PrMovieSubtitles::MovieSubtitleTrack* GetCommonLyricsTrackForMode(
    const Movie1TextRuntime& runtime,
    uint8_t mode) {
    if (mode >= runtime.commonLyricsTracks.size()) {
        return nullptr;
    }

    const PrMovieSubtitles::MovieSubtitleTrack& track = runtime.commonLyricsTracks[mode];
    return track.loaded ? &track : nullptr;
}

static uint32_t GetCommonLyricsDescAddrForMode(const Movie1TextRuntime& runtime,
                                               uint8_t mode) {
    if (mode >= runtime.commonLyricsDescAddrByMode.size()) {
        return 0;
    }

    const uint32_t loadedDescAddr = runtime.commonLyricsDescAddrByMode[mode];
    return loadedDescAddr;
}

static PrStage1VTextDirectDescInput BuildVTextDescInput(
    const PsxVTextDescriptorState& desc,
    uint32_t fallbackDescAddr,
    const std::vector<PrStage1VTextDirectLineMeta>* lineMeta) {
    PrStage1VTextDirectDescInput out{};
    out.descAddr = desc.valid ? desc.descAddr : fallbackDescAddr;
    out.textTableAddrs = desc.textTableAddrs;
    out.entriesAddr = desc.entriesAddr;
    out.entryCount = desc.entryCount;
    if (lineMeta && !lineMeta->empty()) {
        out.lineMeta = lineMeta->data();
        out.lineMetaCount = (uint32_t)lineMeta->size();
    }
    return out;
}

static PrStage1VTextDirectDescInput GetCommonLyricsDescForMode(
    const Movie1TextRuntime& runtime,
    uint8_t mode) {
    if (mode >= runtime.commonLyricsDescs.size()) {
        return {};
    }

    return BuildVTextDescInput(runtime.commonLyricsDescs[mode],
                               GetCommonLyricsDescAddrForMode(runtime, mode),
                               &runtime.commonLyricsLineMetas[mode]);
}

static void ClearCommonLyricsWindowSnapshot(Movie1TextRuntime& runtime) {
    runtime.commonLyricsSnapshot = PrStage1CommonLyricsSnapshot{};
    runtime.commonLyricsSnapshotValid = false;
}

static bool ResolveCommonLyricsSnapshotForWindow(
    const Movie1TextRuntime& runtime,
    const PrStage1VTextDirectMovieWindowResult& window,
    uint8_t languageIndex,
    uint32_t queryFrame60,
    PrStage1CommonLyricsSnapshot& out) {
    out = PrStage1CommonLyricsSnapshot{};
    if (!window.active || window.mode >= runtime.commonLyricsTracks.size()) {
        return false;
    }

    const uint8_t selectedLanguageIndex = ResolveLanguageIndex(languageIndex);
    const PrStage1VTextDirectDescInput desc =
        GetCommonLyricsDescForMode(runtime, window.mode);
    return PrStage1VTextDirectResolveSnapshot(
        runtime.commonLyricsVText,
        desc,
        selectedLanguageIndex,
        window.mode,
        window.queryFrame30,
        queryFrame60,
        out);
}

static bool IsKnownCommonLyricsMode(uint8_t mode) {
    return mode < kMovie1CommonLyricsModeCount;
}

static Movie1TextDrawCommand MakeDrawCommand(Movie1TextDrawCommandKind kind,
                                             uint32_t psxFunctionAddr,
                                             uint8_t ctxMode) {
    Movie1TextDrawCommand command{};
    command.kind = kind;
    command.psxFunctionAddr = psxFunctionAddr;
    command.ctxMode = ctxMode;
    return command;
}

static Movie1TextCurrentGp872WorkCarrier BuildCurrentGp872WorkCarrier(
    const Movie1TextWindowInput& input) {
    Movie1TextCurrentGp872WorkCarrier out{};
    out.usesCurrentGp872DrawBuffer = true;
    out.workBasePsxAddr = kMovie1TextOtBufferBasePsxAddr;
    out.workStrideBytes = kMovie1TextOtBufferStrideBytes;
    if (input.currentGp872SlotKnown && input.currentGp872Slot < 2u) {
        out.gp872SlotKnown = true;
        out.gp872Slot = input.currentGp872Slot;
        out.workAddrKnown = true;
        out.workAddr = out.workBasePsxAddr +
            out.workStrideBytes * static_cast<uint32_t>(input.currentGp872Slot);
        out.drawOtagAddrKnown = true;
        out.drawOtagAddr = out.workAddr + out.workLastAddrOffset;
    }
    return out;
}

static Movie1TextOriginStateSub8001B730 BuildOriginStateSub8001B730(
    int16_t x,
    int16_t y,
    int32_t z) {
    Movie1TextOriginStateSub8001B730 out{};
    out.valid = true;
    out.gp_752_originX = x;
    out.gp_756_originY = y;
    out.gp_748_originZ = z;
    return out;
}

static void PushFinalizeSubmitSub8001C864(
    Movie1TextFinalizeStateSub8001C864& state,
    int16_t x,
    int16_t y,
    uint32_t templateAddr) {
    if (state.submitCount >= state.submits.size()) {
        return;
    }

    Movie1TextFinalizeSubmitSub8001C864& submit =
        state.submits[state.submitCount];
    submit = {};
    submit.valid = true;
    submit.rawDrawSubmitCarrierKnown = state.fastSpriteSubmitOwnerKnown;
    submit.callOrder = static_cast<uint16_t>(state.submitCount);
    submit.screenX = x;
    submit.screenY = y;
    submit.templateAddr = templateAddr;
    submit.priority = state.arg0_priority;
    submit.usesCurrentGp872DrawBuffer = state.usesCurrentGp872DrawBuffer;
    submit.workBasePsxAddr = state.workBasePsxAddr;
    submit.workStrideBytes = state.workStrideBytes;
    submit.currentWork = state.currentWork;
    ++state.submitCount;
}

static Movie1TextFinalizeStateSub8001C864 BuildFinalizeStateSub8001C864(
    uint16_t priority,
    const Movie1TextCurrentGp872WorkCarrier& currentWork) {
    Movie1TextFinalizeStateSub8001C864 out{};
    out.valid = true;
    out.arg0_priority = priority;
    out.fastSpriteSubmitOwnerKnown =
        out.usesCurrentGp872DrawBuffer &&
        out.workBasePsxAddr == kMovie1TextOtBufferBasePsxAddr &&
        out.workStrideBytes == kMovie1TextOtBufferStrideBytes;
    out.currentWork = currentWork;

    PushFinalizeSubmitSub8001C864(out, 280, 200, 0x8004E940u);
    PushFinalizeSubmitSub8001C864(out, 20, 200, 0x8004E950u);
    PushFinalizeSubmitSub8001C864(out, 280, 180, 0x8004E960u);
    PushFinalizeSubmitSub8001C864(out, 20, 180, 0x8004E970u);

    for (int i = 0; i < 7; ++i) {
        const int16_t x0 = static_cast<int16_t>(40 + i * 40);
        const int16_t x1 = static_cast<int16_t>(20 + i * 40);
        PushFinalizeSubmitSub8001C864(out, x0, 0, 0x8004E900u);
        PushFinalizeSubmitSub8001C864(out, x1, 0, 0x8004E910u);
        PushFinalizeSubmitSub8001C864(out, x0, 220, 0x8004E920u);
        PushFinalizeSubmitSub8001C864(out, x1, 220, 0x8004E930u);
    }

    for (int i = 0; i < 6; ++i) {
        const int16_t y0 = static_cast<int16_t>(20 + i * 40);
        const int16_t y1 = static_cast<int16_t>(i * 40);
        const int16_t x0 = static_cast<int16_t>(40 + i * 40);
        const int16_t x1 = static_cast<int16_t>(60 + i * 40);
        PushFinalizeSubmitSub8001C864(out, 0, y0, 0x8004E920u);
        PushFinalizeSubmitSub8001C864(out, 0, y1, 0x8004E900u);
        PushFinalizeSubmitSub8001C864(out, 300, y0, 0x8004E930u);
        PushFinalizeSubmitSub8001C864(out, 300, y1, 0x8004E910u);
        PushFinalizeSubmitSub8001C864(out, x0, 200, 0x8004E860u);
        PushFinalizeSubmitSub8001C864(out, x1, 200, 0x8004E870u);
        PushFinalizeSubmitSub8001C864(out, x0, 180, 0x8004E880u);
        PushFinalizeSubmitSub8001C864(out, x1, 180, 0x8004E890u);
    }

    PushFinalizeSubmitSub8001C864(out, 40, 160, 0x8004E8E0u);
    PushFinalizeSubmitSub8001C864(out, 160, 160, 0x8004E8F0u);
    PushFinalizeSubmitSub8001C864(out, 40, 20, 0x8004E8C0u);
    PushFinalizeSubmitSub8001C864(out, 160, 20, 0x8004E8D0u);
    PushFinalizeSubmitSub8001C864(out, 20, 20, 0x8004E8A0u);
    PushFinalizeSubmitSub8001C864(out, 280, 20, 0x8004E8B0u);

    return out;
}

static Movie1TextDisplayChainSub8001DB00 BuildDisplayChainSub8001DB00(
    const Movie1TextDisplayActionSub8001EC54& action) {
    Movie1TextDisplayChainSub8001DB00 out{};
    out.valid = true;
    out.textPsxAddr = action.text268PsxAddr;
    out.textPtr = action.text268Ptr;
    out.origin = BuildOriginStateSub8001B730(
        static_cast<int16_t>(action.textDrawXSub8001DB00),
        static_cast<int16_t>(action.textDrawYSub8001DB00),
        0);
    out.glyphScaleArg1 = action.textDrawScaleSub8001DB00;
    out.finalizeArg0 = action.textDrawFinalizeArgSub8001DB00;
    return out;
}

static void SetFirstGlyphLoopGap(Movie1TextFastSpriteSequenceSub8001B954& out,
                                 Movie1TextGlyphLoopGapSub8001B954 gap) {
    if (out.firstGap == Movie1TextGlyphLoopGapSub8001B954::None) {
        out.firstGap = gap;
    }
}

static Movie1TextGlyphRgbGapEvidenceSub8001B954
BuildGlyphRgbGapEvidenceSub8001B954(bool workOtOwnerKnown) {
    Movie1TextGlyphRgbGapEvidenceSub8001B954 out{};
    out.valid = true;
    out.kind =
        Movie1TextGlyphRgbEvidenceKindSub8001B954::StackLocalRgbUnresolved8001B954;
    out.workOtOwnerKnown = workOtOwnerKnown;
    out.localStackRgbWriteProven = false;
    out.dword8007CED0AppliesToThisStackLocal = false;
    return out;
}

static PrPsxFastSpriteSubmitDirect::FastSpriteRawLocalBytes8003FA20
BuildGlyphRawLocalBytesSub8001B954(
    const PrPsxFastSpriteSubmitDirect::FastSpriteLocalFields8003FA20&
        localFastSprite) {
    const char* note =
        "8001B954 glyph stack RGB tail is PSX stack residue from "
        "8001B26C saved-register spill, not a semantic RGB producer";
    const PrPsxFastSpriteSubmitDirect::FastSpriteStackScratchTail8003FA20
        stackTail =
            PrPsxFastSpriteSubmitDirect::
                BuildStackScratchTail8001B26CSpillToConsumer(
                    kMovie1TextGlyphLoopFunctionSub8001B954,
                    0x8001BBF8u,
                    0x0001B5CCu,
                    true,
                    note);
    const PrPsxFastSpriteSubmitDirect::FastSpriteRawLocalBytes8003FA20
        stackSeed =
            PrPsxFastSpriteSubmitDirect::
                ApplyStackScratchTailToRawLocalBytes8003FA20({},
                                                             stackTail);
    return PrPsxFastSpriteSubmitDirect::
        BuildRawLocalBytesFromKnownFields8003FA20(
            localFastSprite,
            stackSeed,
            kMovie1TextGlyphLoopFunctionSub8001B954,
            0x8001BBF8u,
            note);
}

static void AppendGlyphSubmitResultSub8001B954(
    Movie1TextFastSpriteSequenceSub8001B954& out,
    const Movie1TextGlyphCommandSub8001B954& glyph,
    size_t glyphCommandIndex,
    const PrPsxFastSpriteSubmitDirect::RuntimeState8003FA20& runtime,
    const PrPsxGraphOwnerDirect::PsxFastSpriteRuntimeBuildResult8003FA20&
        runtimeBuild) {
    if (!glyph.emitsGsSortFastSprite ||
        out.glyphSubmitResultCount >= out.glyphSubmitResults.size()) {
        return;
    }

    Movie1TextGlyphSubmitResultSub8001B954& submit =
        out.glyphSubmitResults[out.glyphSubmitResultCount++];
    submit.valid = true;
    submit.glyphCommandIndex = static_cast<uint16_t>(glyphCommandIndex);
    submit.wouldCall8003FA20 = true;
    submit.fastSpriteOwnerKnown = glyph.fastSpriteOwnerKnown;
    submit.currentWork = out.currentWork;
    submit.currentWorkKnown = out.currentWork.workAddrKnown;
    submit.runtimeBuild8003FA20 = runtimeBuild;
    submit.runtimeOwnerKnown = runtimeBuild.built;
    submit.packetAllocatorKnown = runtimeBuild.built;
    submit.packetAllocatorAddr = runtime.dword_800901C8;
    submit.localFastSpriteStaticFieldsKnown =
        glyph.localFastSpriteStaticFieldsKnown &&
        glyph.localSpritePositionKnown;
    submit.localFastSpriteRgbKnown = glyph.localFastSpriteRgbKnown;
    submit.localFastSprite = glyph.localFastSprite;
    submit.rawLocalBytes8003FA20 = glyph.rawLocalBytes8003FA20;
    if (!submit.fastSpriteOwnerKnown) {
        submit.gap = Movie1TextGlyphLoopGapSub8001B954::FastSpriteOwnerMissing;
    } else if (!PrPsxFastSpriteSubmitDirect::HasKnownRgbTail8003FA20(
                   submit.rawLocalBytes8003FA20)) {
        submit.gap = Movie1TextGlyphLoopGapSub8001B954::LocalFastSpriteRgbMissing;
        submit.rgbGapEvidence = glyph.rgbGapEvidence;
    }
    if (submit.runtimeOwnerKnown &&
        submit.localFastSpriteStaticFieldsKnown) {
        const PrPsxFastSpriteSubmitDirect::GsSortFastSpriteInput8003FA20
            input =
                PrPsxFastSpriteSubmitDirect::BuildInputFromRuntime8003FA20(
                    runtime,
                    submit.localFastSprite,
                    submit.rawLocalBytes8003FA20,
                    glyph.priority);
        submit.partial8003FA20 =
            PrPsxFastSpriteSubmitDirect::
                PredictGsSortFastSpritePartial8003FA20(
                    input,
                    PrPsxFastSpriteSubmitDirect::HasKnownRgbTail8003FA20(
                        submit.rawLocalBytes8003FA20));
        submit.packetKnown = submit.partial8003FA20.packetColorKnown;
    }
}

static size_t PsxStrLenLimitedSub8001B954(const char* text) {
    if (!text) {
        return 0;
    }

    size_t len = 0;
    while (len < kMovie1TextGlyphCommandCapacitySub8001B954 &&
           text[len] != '\0') {
        ++len;
    }
    return len;
}

static int16_t ReadI16LESub8001B954(const uint8_t* p) {
    return static_cast<int16_t>(ReadU16LE(p));
}

static int16_t ResolveGlyphTexCoordRoundedV22Sub8001B954(int16_t u) {
    return static_cast<int16_t>(u <= 0 ? u + 3584 : u + 3583);
}

static uint16_t ResolveGlyphTexHelperA3Sub8001B954(int16_t v22) {
    return static_cast<uint16_t>(
        (static_cast<uint16_t>(v22) & 0xFF00u) >> 2);
}

static uint16_t ResolveGlyphTexHelperA4Sub8001B954(int16_t v) {
    return static_cast<uint16_t>(
        (static_cast<uint16_t>(v) + 256u) & 0xFF00u);
}

static uint16_t ResolveGlyphTpageSub80043DF4_Mode0_4bppSub8001B954(
    uint16_t texHelperA3,
    uint16_t texHelperA4) {
    // sub_80043DF4(0, 1, a3, a4) with sub_80044A24()==0 in SCUS_941.83.
    return static_cast<uint16_t>(
        0x20u |
        ((texHelperA4 & 0x0100u) >> 4) |
        ((texHelperA3 & 0x03FFu) >> 6) |
        (4u * (texHelperA4 & 0x0200u)));
}

static bool UsesTallGlyphBaselineSub8001B954(uint8_t ch) {
    return (static_cast<uint32_t>(ch) - 192u) < 0x1Eu && ch != 199u;
}

static PrPsxFastSpriteSubmitDirect::FastSpriteLocalFields8003FA20
BuildGlyphStaticFastSpriteSub8001B954(
    const Movie1TextGlyphMetricSub8001B954& metric,
    uint16_t clutY) {
    PrPsxFastSpriteSubmitDirect::FastSpriteLocalFields8003FA20 out{};
    out.attr_00 = kMovie1TextGlyphFastSpriteAttrSub8001B954;
    out.width_08 = metric.advanceWidth;
    out.height_0A = metric.height;
    out.tpage_0C =
        ResolveGlyphTpageSub80043DF4_Mode0_4bppSub8001B954(
            metric.texHelperA3,
            metric.texHelperA4);
    out.u_0E = static_cast<uint8_t>(metric.texCoordRoundedV22);
    out.v_0F = static_cast<uint8_t>(metric.word_8004945E);
    out.clutX_10 =
        static_cast<int16_t>(kMovie1TextGlyphFastSpriteClutXSub8001B954);
    out.clutY_12 = static_cast<int16_t>(clutY);
    return out;
}

static Movie1TextGlyphMetricSub8001B954 DecodeGlyphMetricSub8001B954(
    const uint8_t* raw) {
    Movie1TextGlyphMetricSub8001B954 out{};
    out.valid = true;
    for (size_t i = 0; i < out.raw.size(); ++i) {
        out.raw[i] = raw[i];
    }
    out.word0_8004945C = ReadI16LESub8001B954(raw + 0);
    out.word_8004945E = ReadI16LESub8001B954(raw + 2);
    out.advanceWidth = raw[4];
    out.height = raw[5];
    out.word_80049462 = ReadI16LESub8001B954(raw + 6);
    out.skipsGlyphSubmit = out.word_8004945E < 0;
    if (!out.skipsGlyphSubmit) {
        out.texCoordRoundedV22 =
            ResolveGlyphTexCoordRoundedV22Sub8001B954(out.word0_8004945C);
        out.texHelperA3 =
            ResolveGlyphTexHelperA3Sub8001B954(out.texCoordRoundedV22);
        out.texHelperA4 =
            ResolveGlyphTexHelperA4Sub8001B954(out.word_8004945E);
        out.localFastSpriteStaticFieldsKnown = true;
        out.localFastSpriteRgbKnown = false;
    }
    return out;
}

static bool TryGetEmbeddedGlyphMetricSub8001B954(
    uint8_t ch,
    Movie1TextGlyphMetricSub8001B954& out) {
    const size_t off =
        (size_t)ch * kMovie1TextGlyphMetricTableStride8004945C;
    if (off + kMovie1TextGlyphMetricTableStride8004945C >
        sizeof(kGlyphMetricRaw8004945C) - 1u) {
        out = {};
        return false;
    }

    const char* base = kGlyphMetricRaw8004945C + off;
    uint8_t raw[kMovie1TextGlyphMetricTableStride8004945C]{};
    for (size_t i = 0; i < kMovie1TextGlyphMetricTableStride8004945C; ++i) {
        raw[i] = static_cast<uint8_t>(base[i]);
    }
    out = DecodeGlyphMetricSub8001B954(raw);
    return true;
}

static bool TryGetGlyphMetricSub8001B954(
    const Movie1TextGlyphMetricTablesSub8001B954& metrics,
    uint8_t ch,
    Movie1TextGlyphMetricSub8001B954& out) {
    out = {};
    if (!metrics.valid || !metrics.glyphs ||
        (size_t)ch >= metrics.glyphCount) {
        return TryGetEmbeddedGlyphMetricSub8001B954(ch, out);
    }

    out = metrics.glyphs[(size_t)ch];
    return out.valid;
}

static int16_t WrapI16Sub8001B954(int32_t value) {
    return static_cast<int16_t>(static_cast<uint16_t>(value));
}

static void InitGlyphLineSub8001B954(
    Movie1TextFastSpriteSequenceSub8001B954& out,
    size_t lineIndex,
    size_t textStartIndex,
    int16_t y,
    bool widthKnown) {
    if (lineIndex >= out.lines.size()) {
        out.lineCapacityExceeded = true;
        SetFirstGlyphLoopGap(
            out,
            Movie1TextGlyphLoopGapSub8001B954::LineCapacityExceeded);
        return;
    }

    Movie1TextGlyphLineSub8001B954& line = out.lines[lineIndex];
    line = {};
    line.valid = true;
    line.lineIndex = (uint8_t)lineIndex;
    line.textStartIndex = (uint16_t)textStartIndex;
    line.y = y;
    line.widthKnown = widthKnown;
    out.lineCount = lineIndex + 1u;
}

static void PushDrawCommand(Movie1TextDisplayActionSub8001EC54& action,
                            const Movie1TextDrawCommand& command) {
    if (action.drawCommandCount < action.drawCommands.size()) {
        action.drawCommands[action.drawCommandCount++] = command;
    }
}

static PrStage1VTextDirectMovieWindowResult ResolvePlayAndWaitWindowSub801C77C0(
    const Movie1TextRuntime& runtime,
    const Movie1TextWindowInput& input) {
    if (runtime.playAndWaitActiveSub801C77C0) {
        PrStage1VTextDirectMovieWindowResult out{};
        if (!IsKnownCommonLyricsMode(runtime.playAndWaitModeSub801C77C0)) {
            return out;
        }

        out.active = true;
        out.mode = runtime.playAndWaitModeSub801C77C0;
        out.queryFrame30 = input.movieFrame30;
        return out;
    }

    return {};
}

static Movie1TextDisplayActionSub8001EC54 BuildDisplayActionSub8001EC54Ctx7(
    const Movie1TextRuntime& runtime,
    const Movie1TextWindowInput& input) {
    Movie1TextDisplayActionSub8001EC54 action{};
    action.requested = true;
    action.ctxMode = 7;
    action.word800916DC = input.word800916DC;
    action.currentWork = BuildCurrentGp872WorkCarrier(input);
    action.text268Known = true;
    action.text268PsxAddr = runtime.commonLyricsVText.text268MirrorPsxAddr;
    action.text268Ptr = runtime.commonLyricsVText.text268MirrorPtr;
    Movie1TextDrawCommand acquire =
        MakeDrawCommand(Movie1TextDrawCommandKind::AcquireDrawBufferSub8004019C,
                        0x8004019Cu,
                        action.ctxMode);
    PushDrawCommand(action, acquire);

    Movie1TextDrawCommand select =
        MakeDrawCommand(Movie1TextDrawCommandKind::SelectDrawBufferSub80040F90,
                        0x80040F90u,
                        action.ctxMode);
    select.usesCurrentGp872DrawBuffer = true;
    PushDrawCommand(action, select);

    Movie1TextDrawCommand bind =
        MakeDrawCommand(Movie1TextDrawCommandKind::BindOtBufferSub80040CC8,
                        0x80040CC8u,
                        action.ctxMode);
    bind.usesCurrentGp872DrawBuffer = true;
    bind.otBufferBasePsxAddr = kMovie1TextOtBufferBasePsxAddr;
    bind.otBufferStrideBytes = kMovie1TextOtBufferStrideBytes;
    PushDrawCommand(action, bind);

    if (input.word800916DC != 0) {
        action.helper = Movie1TextDisplayHelperSub8001EC54::Sub8001DB00_Text268;
        action.displayChainSub8001DB00 = BuildDisplayChainSub8001DB00(action);
        Movie1TextDrawCommand origin =
            MakeDrawCommand(Movie1TextDrawCommandKind::SetTextOriginSub8001B730,
                            0x8001B730u,
                            action.ctxMode);
        origin.x = action.textDrawXSub8001DB00;
        origin.y = action.textDrawYSub8001DB00;
        origin.z = 0;
        origin.originStateSub8001B730 =
            action.displayChainSub8001DB00.origin;
        PushDrawCommand(action, origin);

        Movie1TextDrawCommand submit =
            MakeDrawCommand(
                Movie1TextDrawCommandKind::SubmitTextFastSpriteSequenceSub8001B954,
                0x8001B954u,
                action.ctxMode);
        submit.usesCurrentGp872DrawBuffer = true;
        submit.otBufferBasePsxAddr = kMovie1TextOtBufferBasePsxAddr;
        submit.otBufferStrideBytes = kMovie1TextOtBufferStrideBytes;
        submit.x = action.textDrawXSub8001DB00;
        submit.y = action.textDrawYSub8001DB00;
        submit.z = 0;
        submit.scale = action.textDrawScaleSub8001DB00;
        submit.textPtr = action.text268Ptr;
        submit.textPsxAddr = action.text268PsxAddr;
        submit.emitsGsSortFastSprite = true;
        submit.centeredLineWidth = kMovie1TextGlyphCenteredBodyWidthSub8001B954;
        submit.glyphWidthTablePsxAddr = kMovie1TextGlyphMetricTableBase8004945C;
        submit.glyphSequenceBuiltSub8001B954 = true;
        action.textFastSpriteSequenceSub8001B954 =
            BuildTextFastSpriteSequenceSub8001B954(submit,
                                                   Movie1TextGlyphMetricTablesSub8001B954{},
                                                   action.currentWork);
        PushDrawCommand(action, submit);

        Movie1TextDrawCommand finalize =
            MakeDrawCommand(Movie1TextDrawCommandKind::FinalizeTextSub8001C864,
                            0x8001C864u,
                            action.ctxMode);
        finalize.arg0 = action.textDrawFinalizeArgSub8001DB00;
        finalize.usesCurrentGp872DrawBuffer = true;
        finalize.otBufferBasePsxAddr = kMovie1TextOtBufferBasePsxAddr;
        finalize.otBufferStrideBytes = kMovie1TextOtBufferStrideBytes;
        finalize.finalizeStateSub8001C864 =
            BuildFinalizeStateSub8001C864(finalize.arg0,
                                          action.currentWork);
        PushDrawCommand(action, finalize);
    } else {
        action.helper =
            Movie1TextDisplayHelperSub8001EC54::Sub8001CE30_NoSubtitleFrame;
        Movie1TextDrawCommand noSubtitle =
            MakeDrawCommand(Movie1TextDrawCommandKind::DrawNoSubtitleFrameSub8001CE30,
                            0x8001CE30u,
                            action.ctxMode);
        noSubtitle.usesCurrentGp872DrawBuffer = true;
        noSubtitle.otBufferBasePsxAddr = kMovie1TextOtBufferBasePsxAddr;
        noSubtitle.otBufferStrideBytes = kMovie1TextOtBufferStrideBytes;
        noSubtitle.arg0 = action.noSubtitleFrameArgSub8001CE30;
        PushDrawCommand(action, noSubtitle);
    }
    return action;
}

static Movie1TextFlushActionSub8001ED3C BuildFlushActionSub8001ED3C(
    const Movie1TextWindowInput& input) {
    Movie1TextFlushActionSub8001ED3C action{};
    action.requested = true;
    action.currentWork = BuildCurrentGp872WorkCarrier(input);
    return action;
}

static void ApplyDisplayActionsSub801C77C0(Movie1TextWindowTickResult& result,
                                           const Movie1TextRuntime& runtime,
                                           const Movie1TextWindowInput& input) {
    if (runtime.movie1DisplayFlushGate801D3044Known &&
        runtime.movie1DisplayFlushGate801D3044) {
        result.displayActionSub8001EC54Ctx7 =
            BuildDisplayActionSub8001EC54Ctx7(runtime, input);
        result.flushActionSub8001ED3C = BuildFlushActionSub8001ED3C(input);
    }
}

}  // namespace

uint8_t ResolveLanguageIndex(int languageIndex) {
    return (languageIndex >= 0 && languageIndex < 5) ? (uint8_t)languageIndex : 0u;
}

static bool LoadFixedVTextTrackFromComod(const std::vector<uint8_t>& bytes,
                                         const PsxMovieTextLoadSpec& loadSpec,
                                         const PsxVTextTrackLoadSpec& trackSpec,
                                         uint8_t languageIndex,
                                         PrMovieSubtitles::MovieSubtitleTrack& outTrack,
                                         std::vector<PrStage1VTextDirectLineMeta>* outLineMeta,
                                         PsxVTextDescriptorState* outDesc);
static bool LoadMovieSubtitleTrackFromComod(const std::vector<uint8_t>& bytes,
                                            const PsxMovieTextLoadSpec& loadSpec,
                                            uint8_t languageIndex,
                                            PrMovieSubtitles::MovieSubtitleTrack& outTrack,
                                            std::vector<PrStage1VTextDirectLineMeta>* outLineMeta,
                                            PsxVTextDescriptorState* outDesc);

void ResetSub80024C84(Movie1TextRuntime& runtime) {
    runtime = {};
}

void ResetPlaybackStateSub801C77C0(Movie1TextRuntime& runtime) {
    PrStage1VTextDirectResetSub80024C84(runtime.commonLyricsVText);
    runtime.playAndWaitActiveSub801C77C0 = false;
    runtime.playAndWaitDescSelectedSub80024C84 = false;
    runtime.playAndWaitModeGapSub801C77C0 = false;
    runtime.playAndWaitModeSub801C77C0 = 0;
    runtime.playAndWaitDescAddrSub801C77C0 = 0;
    runtime.movie1DisplayFlushGate801D3044Known = false;
    runtime.movie1DisplayFlushGate801D3044 = false;
    runtime.movie1DisplayFlushGateSourceFunction = 0;
    runtime.movie1DisplayFlushGateSetupArgA2 = 0;
    ClearCommonLyricsWindowSnapshot(runtime);
}

void ApplyPrStrPlayerInitSub801C7744(Movie1TextRuntime& runtime,
                                     uint32_t initArgA2) {
    runtime.movie1DisplayFlushGate801D3044Known = true;
    runtime.movie1DisplayFlushGate801D3044 = initArgA2 != 1u;
    runtime.movie1DisplayFlushGateSourceFunction = 0x801C44E0u;
    runtime.movie1DisplayFlushGateSetupArgA2 = initArgA2;
}

void BeginPlayAndWaitSub801C77C0(Movie1TextRuntime& runtime,
                                 uint8_t mode,
                                 uint8_t languageIndex) {
    ClearCommonLyricsWindowSnapshot(runtime);
    runtime.playAndWaitActiveSub801C77C0 = true;
    runtime.playAndWaitModeSub801C77C0 = mode;
    runtime.playAndWaitModeGapSub801C77C0 = !IsKnownCommonLyricsMode(mode);
    runtime.playAndWaitDescSelectedSub80024C84 = false;
    runtime.playAndWaitDescAddrSub801C77C0 = 0;

    if (!IsKnownCommonLyricsMode(mode)) {
        PrStage1VTextDirectResetSub80024C84(runtime.commonLyricsVText);
        return;
    }

    const PrStage1VTextDirectDescInput desc =
        GetCommonLyricsDescForMode(runtime, mode);
    runtime.playAndWaitDescAddrSub801C77C0 = desc.descAddr;
    if (!GetCommonLyricsTrackForMode(runtime, mode) || desc.entryCount == 0) {
        PrStage1VTextDirectResetSub80024C84(runtime.commonLyricsVText);
        runtime.playAndWaitModeGapSub801C77C0 = true;
        return;
    }

    PrStage1VTextDirectSelectDescSub80024C84(runtime.commonLyricsVText,
                                            desc,
                                            ResolveLanguageIndex(languageIndex));
    runtime.playAndWaitDescSelectedSub80024C84 = true;
}

void EndPlayAndWaitSub801C77C0(Movie1TextRuntime& runtime) {
    runtime.playAndWaitActiveSub801C77C0 = false;
    runtime.playAndWaitDescSelectedSub80024C84 = false;
    runtime.playAndWaitModeGapSub801C77C0 = false;
    runtime.playAndWaitModeSub801C77C0 = 0;
    runtime.playAndWaitDescAddrSub801C77C0 = 0;
    ClearCommonLyricsWindowSnapshot(runtime);
}

void LoadFromComodSub80024C84(Movie1TextRuntime& runtime,
                              const std::vector<uint8_t>& comodBytes,
                              uint32_t ptr800943CC) {
    const PsxMovieTextLoadSpec loadSpec =
        BuildMovieTextLoadSpecFromPtr800943CC(ptr800943CC);
    LoadFromComodSub80024C84(runtime, comodBytes, loadSpec);
}

void LoadFromComodSub80024C84(Movie1TextRuntime& runtime,
                              const std::vector<uint8_t>& comodBytes,
                              const PsxMovieTextLoadSpec& loadSpec) {
    ResetSub80024C84(runtime);

    LoadMovieSubtitleTrackFromComod(comodBytes,
                                    loadSpec,
                                    0u,
                                    runtime.subtitleTrack,
                                    &runtime.subtitleLineMeta,
                                    &runtime.subtitleDesc);
    runtime.subtitleWindow =
        PrStage1Scene1Movie1Direct::SelectMovieTextDescSub80024C84(runtime.subtitleTrack);

    for (size_t mode = 0; mode < runtime.commonLyricsTracks.size(); ++mode) {
        const bool loaded =
            LoadFixedVTextTrackFromComod(comodBytes,
                                         loadSpec,
                                         loadSpec.commonLyrics[mode],
                                         0u,
                                         runtime.commonLyricsTracks[mode],
                                         &runtime.commonLyricsLineMetas[mode],
                                         &runtime.commonLyricsDescs[mode]);
        runtime.commonLyricsDescAddrByMode[mode] =
            loaded && runtime.commonLyricsDescs[mode].valid
                ? runtime.commonLyricsDescs[mode].descAddr
                : 0u;
    }

    PrStage1VTextDirectResetSub80024C84(runtime.commonLyricsVText);
}

uint32_t GetCommonLyricsDescAddrSub80024C84(const Movie1TextRuntime& runtime,
                                            uint8_t mode) {
    return GetCommonLyricsDescAddrForMode(runtime, mode);
}

static bool LoadFixedVTextTrackFromComod(const std::vector<uint8_t>& bytes,
                                         const PsxMovieTextLoadSpec& loadSpec,
                                         const PsxVTextTrackLoadSpec& trackSpec,
                                         uint8_t languageIndex,
                                         PrMovieSubtitles::MovieSubtitleTrack& outTrack,
                                         std::vector<PrStage1VTextDirectLineMeta>* outLineMeta,
                                         PsxVTextDescriptorState* outDesc) {
    outTrack = {};
    if (outLineMeta) {
        outLineMeta->clear();
    }
    if (outDesc) {
        *outDesc = {};
    }
    if (!loadSpec.overlayResolvers || loadSpec.overlayResolverCount == 0 || trackSpec.descAddr == 0) {
        return false;
    }
    if (bytes.size() < sizeof(PrVTextEventTableDesc) + sizeof(PrVTextEventEntry)) {
        return false;
    }

    for (size_t i = 0; i < loadSpec.overlayResolverCount; ++i) {
        const PsxOverlayAddressResolver& resolver = loadSpec.overlayResolvers[i];
        size_t descOff = 0;
        if (!ResolvePsxOverlayAddressToOffset(
                bytes,
                resolver,
                trackSpec.descAddr,
                sizeof(PrVTextEventTableDesc),
                descOff)) {
            continue;
        }
        if (TryLoadVTextTrackAtDescriptor(
                bytes,
                resolver,
                trackSpec.descAddr,
                descOff,
                trackSpec.expectedEntryCount,
                languageIndex,
                outTrack,
                outLineMeta,
                outDesc)) {
            return true;
        }
    }

    return false;
}

static bool LoadMovieSubtitleTrackFromComod(const std::vector<uint8_t>& bytes,
                                            const PsxMovieTextLoadSpec& loadSpec,
                                            uint8_t languageIndex,
                                            PrMovieSubtitles::MovieSubtitleTrack& outTrack,
                                            std::vector<PrStage1VTextDirectLineMeta>* outLineMeta,
                                            PsxVTextDescriptorState* outDesc) {
    return LoadFixedVTextTrackFromComod(bytes,
                                       loadSpec,
                                       loadSpec.movieSubtitle,
                                       languageIndex,
                                       outTrack,
                                       outLineMeta,
                                       outDesc);
}

const PrMovieSubtitles::MovieSubtitleTrack& GetMovieSubtitleTrack(const Movie1TextRuntime& runtime) {
    return runtime.subtitleTrack;
}

const PrMovieSubtitles::MovieSubtitleTrack& GetActiveMovieSubtitleTrack(
    const Movie1TextRuntime& runtime) {
    if (runtime.playAndWaitActiveSub801C77C0) {
        const PrMovieSubtitles::MovieSubtitleTrack* track =
            GetCommonLyricsTrackForMode(
                runtime,
                runtime.playAndWaitModeSub801C77C0);
        if (track != nullptr) {
            return *track;
        }
    }
    return runtime.subtitleTrack;
}

PrStage1Scene1Movie1Direct::SubtitleFrameWindow
GetActiveMovieSubtitleFrameWindow(const Movie1TextRuntime& runtime) {
    const PrMovieSubtitles::MovieSubtitleTrack& track =
        GetActiveMovieSubtitleTrack(runtime);
    return PrStage1Scene1Movie1Direct::SelectMovieTextDescSub80024C84(track);
}

void ApplyMovieSubtitleWindow(Movie1TextRuntime& textRuntime,
                              PrStage1Scene1Movie1Direct::Movie1RuntimeState& movieRuntime) {
    textRuntime.subtitleWindow = GetActiveMovieSubtitleFrameWindow(textRuntime);
    PrStage1Scene1Movie1Direct::SetSubtitleWindow(movieRuntime,
                                                  textRuntime.subtitleWindow);
}

void AdvanceCommonLyricsSub80024CF8(Movie1TextRuntime& runtime,
                                    uint8_t mode,
                                    uint8_t languageIndex,
                                    uint32_t queryFrame30) {
    const PrMovieSubtitles::MovieSubtitleTrack* track = GetCommonLyricsTrackForMode(runtime, mode);
    if (!track) {
        PrStage1VTextDirectResetSub80024C84(runtime.commonLyricsVText);
        return;
    }

    const uint8_t selectedLanguageIndex = ResolveLanguageIndex(languageIndex);
    const PrStage1VTextDirectDescInput desc =
        GetCommonLyricsDescForMode(runtime, mode);
    PrStage1VTextDirectAdvanceSub80024CF8(runtime.commonLyricsVText,
                                         track,
                                         desc,
                                         selectedLanguageIndex,
                                         queryFrame30);
}

Movie1TextFastSpriteSequenceSub8001B954 BuildTextFastSpriteSequenceSub8001B954(
    const Movie1TextDrawCommand& command,
    const Movie1TextGlyphMetricTablesSub8001B954& metrics,
    const Movie1TextCurrentGp872WorkCarrier& currentWork) {
    Movie1TextFastSpriteSequenceSub8001B954 out{};
    out.sourceFunction = kMovie1TextGlyphLoopFunctionSub8001B954;
    out.fastSpriteSourceKind = command.fastSpriteSourceKind;
    out.textPsxAddr = command.textPsxAddr;
    out.textPtr = command.textPtr;
    out.arg1_480 = command.scale;
    out.gp_752_originX = static_cast<int16_t>(command.x);
    out.gp_756_originY = static_cast<int16_t>(command.y);
    out.gp_748_originZ = static_cast<int16_t>(command.z);
    out.centeredBodyWidth = command.centeredLineWidth;
    out.lineAdvanceY = kMovie1TextGlyphLineAdvanceYSub8001B954;
    out.byte_80049460 = kMovie1TextGlyphWidthTableByte80049460;
    out.metricTableBase_8004945C = kMovie1TextGlyphMetricTableBase8004945C;
    out.metricTableStride = kMovie1TextGlyphMetricTableStride8004945C;
    out.glyphMetricTablesKnown =
        (metrics.valid && metrics.glyphs && metrics.glyphCount > 0u) ||
        (sizeof(kGlyphMetricRaw8004945C) - 1u ==
         256u * kMovie1TextGlyphMetricTableStride8004945C);
    out.fastSpriteOwnerKnown =
        command.usesCurrentGp872DrawBuffer &&
        command.otBufferBasePsxAddr == kMovie1TextOtBufferBasePsxAddr &&
        command.otBufferStrideBytes == kMovie1TextOtBufferStrideBytes;
    out.currentWork = currentWork;
    PrPsxFastSpriteSubmitDirect::RuntimeState8003FA20 runtime8003FA20{};
    PrPsxGraphOwnerDirect::PsxFastSpriteRuntimeBuildResult8003FA20
        runtimeBuild8003FA20{};
    if (currentWork.gp872SlotKnown) {
        PrPsxGraphOwnerDirect::PsxGraphState graph{};
        PrPsxGraphOwnerDirect::PsxInitializeGraphState8003FB9C(graph,
                                                              320u,
                                                              240u);
        PrPsxGraphOwnerDirect::PsxCall80040D6C_InitGteDefaults(graph);
        PrPsxGraphOwnerDirect::PsxCall800401AC_ApplyDrawOffset(graph);
        graph.word_80096590 =
            static_cast<uint16_t>(currentWork.gp872Slot & 1u);
        runtimeBuild8003FA20 =
            PrPsxGraphOwnerDirect::
                BuildMovie1TextMainRuntimeState8003FA20FromGraphOwner(
                    graph,
                    runtime8003FA20);
    }

    if (command.kind !=
            Movie1TextDrawCommandKind::SubmitTextFastSpriteSequenceSub8001B954 ||
        command.psxFunctionAddr != kMovie1TextGlyphLoopFunctionSub8001B954) {
        return out;
    }

    if (!command.textPtr) {
        SetFirstGlyphLoopGap(out,
                             Movie1TextGlyphLoopGapSub8001B954::MissingText);
        return out;
    }

    out.valid = true;
    const size_t textLen = PsxStrLenLimitedSub8001B954(command.textPtr);
    out.strlenBytes = static_cast<uint16_t>(textLen);
    if (!out.glyphMetricTablesKnown) {
        SetFirstGlyphLoopGap(
            out,
            Movie1TextGlyphLoopGapSub8001B954::MissingGlyphMetricTables);
    }

    if (textLen == 0u) {
        return out;
    }

    InitGlyphLineSub8001B954(out,
                             0u,
                             0u,
                             out.gp_756_originY,
                             out.glyphMetricTablesKnown);

    size_t lineIndex = 0;
    size_t lineStart = 0;
    bool stopAtLineCapacity = false;
    for (size_t i = 0; i < textLen; ++i) {
        const uint8_t ch = static_cast<uint8_t>(command.textPtr[i]);
        if (ch == '\n') {
            if (lineIndex < out.lines.size()) {
                out.lines[lineIndex].textByteCount =
                    static_cast<uint16_t>(i - lineStart);
            }
            ++lineIndex;
            lineStart = i + 1u;
            if (lineIndex >= out.lines.size()) {
                out.lineCapacityExceeded = true;
                SetFirstGlyphLoopGap(
                    out,
                    Movie1TextGlyphLoopGapSub8001B954::LineCapacityExceeded);
                stopAtLineCapacity = true;
                break;
            }

            InitGlyphLineSub8001B954(
                out,
                lineIndex,
                lineStart,
                WrapI16Sub8001B954(
                    static_cast<int32_t>(out.gp_756_originY) +
                    static_cast<int32_t>(lineIndex) *
                        static_cast<int32_t>(
                            kMovie1TextGlyphLineAdvanceYSub8001B954)),
                out.glyphMetricTablesKnown);
            continue;
        }

        if (lineIndex >= out.lines.size()) {
            stopAtLineCapacity = true;
            break;
        }

        Movie1TextGlyphLineSub8001B954& line = out.lines[lineIndex];
        line.textByteCount = static_cast<uint16_t>(i - lineStart + 1u);
        ++line.drawableGlyphCount;

        Movie1TextGlyphMetricSub8001B954 metric{};
        if (TryGetGlyphMetricSub8001B954(metrics, ch, metric)) {
            line.widthPx = static_cast<uint16_t>(
                static_cast<uint32_t>(line.widthPx) +
                static_cast<uint32_t>(metric.advanceWidth));
        } else {
            line.widthKnown = false;
            SetFirstGlyphLoopGap(
                out,
                Movie1TextGlyphLoopGapSub8001B954::MissingGlyphMetricTables);
        }
    }

    if (!stopAtLineCapacity && lineIndex < out.lines.size() &&
        out.lines[lineIndex].valid) {
        out.lines[lineIndex].textByteCount =
            static_cast<uint16_t>(textLen - lineStart);
    }

    for (size_t i = 0; i < out.lineCount; ++i) {
        Movie1TextGlyphLineSub8001B954& line = out.lines[i];
        if (!line.valid || !line.widthKnown) {
            continue;
        }

        line.centeredX = WrapI16Sub8001B954(
            static_cast<int32_t>(out.gp_752_originX) +
            (static_cast<int32_t>(out.centeredBodyWidth) -
             static_cast<int32_t>(line.widthPx)) /
                2);
        line.centeredXKnown = true;
    }

    lineIndex = 0;
    std::array<int16_t, kMovie1TextGlyphLineCapacitySub8001B954> penX{};
    for (size_t i = 0; i < out.lineCount; ++i) {
        penX[i] = out.lines[i].centeredXKnown ? out.lines[i].centeredX
                                              : out.gp_752_originX;
    }

    for (size_t i = 0; i < textLen; ++i) {
        if (out.glyphCommandCount >= out.glyphCommands.size()) {
            out.glyphCommandCapacityExceeded = true;
            SetFirstGlyphLoopGap(
                out,
                Movie1TextGlyphLoopGapSub8001B954::GlyphCommandCapacityExceeded);
            break;
        }
        if (lineIndex >= out.lines.size()) {
            break;
        }

        const uint8_t ch = static_cast<uint8_t>(command.textPtr[i]);
        Movie1TextGlyphCommandSub8001B954& glyph =
            out.glyphCommands[out.glyphCommandCount++];
        glyph = {};
        glyph.valid = true;
        glyph.textByteIndex = static_cast<uint16_t>(i);
        glyph.lineIndex = static_cast<uint8_t>(lineIndex);
        glyph.charCode = ch;
        glyph.arg1_480 = command.scale;
        glyph.priority = kMovie1TextGlyphPrioritySub8001B954;
        glyph.fastSpriteOwnerKnown = out.fastSpriteOwnerKnown;

        if (ch == '\n') {
            glyph.newline = true;
            ++lineIndex;
            if (lineIndex >= out.lines.size()) {
                break;
            }
            continue;
        }

        glyph.drawable = true;

        Movie1TextGlyphMetricSub8001B954 metric{};
        if (!TryGetGlyphMetricSub8001B954(metrics, ch, metric)) {
            SetFirstGlyphLoopGap(
                out,
                Movie1TextGlyphLoopGapSub8001B954::MissingGlyphMetricTables);
            continue;
        }

        glyph.glyphMetricKnown = true;
        glyph.word0_8004945C = metric.word0_8004945C;
        glyph.word_8004945E = metric.word_8004945E;
        glyph.skipsGlyphSubmit = metric.skipsGlyphSubmit;
        glyph.advanceWidth = metric.advanceWidth;
        glyph.height = metric.height;
        glyph.word_80049462 = metric.word_80049462;
        glyph.localFastSpriteStaticFieldsKnown =
            metric.localFastSpriteStaticFieldsKnown;
        glyph.localFastSpriteRgbKnown = metric.localFastSpriteRgbKnown;
        glyph.texCoordRoundedV22 = metric.texCoordRoundedV22;
        glyph.texHelperA3 = metric.texHelperA3;
        glyph.texHelperA4 = metric.texHelperA4;
        if (lineIndex < out.lineCount &&
            out.lines[lineIndex].centeredXKnown) {
            glyph.positionKnown = true;
            glyph.x = penX[lineIndex];
            glyph.y = out.lines[lineIndex].y;
            glyph.localSpritePositionKnown = true;
            glyph.localSpriteX = WrapI16Sub8001B954(
                static_cast<int32_t>(metric.word_80049462) +
                static_cast<int32_t>(glyph.x) -
                155);
            glyph.localSpriteY = static_cast<uint16_t>(
                WrapI16Sub8001B954(
                    static_cast<int32_t>(glyph.y) -
                    (UsesTallGlyphBaselineSub8001B954(ch) ? 123 : 120)));
            penX[lineIndex] = WrapI16Sub8001B954(
                static_cast<int32_t>(penX[lineIndex]) +
                static_cast<int32_t>(metric.advanceWidth));
        }

        if (metric.skipsGlyphSubmit) {
            glyph.emitsGsSortFastSprite = false;
            continue;
        }

        glyph.emitsGsSortFastSprite = true;
        if (!glyph.fastSpriteOwnerKnown) {
            SetFirstGlyphLoopGap(
                out,
                Movie1TextGlyphLoopGapSub8001B954::FastSpriteOwnerMissing);
        } else if (!glyph.localFastSpriteRgbKnown) {
            glyph.rgbGapEvidence =
                BuildGlyphRgbGapEvidenceSub8001B954(glyph.fastSpriteOwnerKnown);
            if (!out.rgbGapEvidence.valid) {
                out.rgbGapEvidence = glyph.rgbGapEvidence;
            }
            SetFirstGlyphLoopGap(
                out,
                Movie1TextGlyphLoopGapSub8001B954::LocalFastSpriteRgbMissing);
        }

        if (glyph.localFastSpriteStaticFieldsKnown &&
            glyph.localSpritePositionKnown) {
            glyph.localFastSprite = BuildGlyphStaticFastSpriteSub8001B954(
                metric,
                command.scale);
            glyph.localFastSprite.x_04 = glyph.localSpriteX;
            glyph.localFastSprite.y_06 = glyph.localSpriteY;
            glyph.rawLocalBytes8003FA20 =
                BuildGlyphRawLocalBytesSub8001B954(glyph.localFastSprite);
            glyph.localFastSpriteRgbKnown =
                PrPsxFastSpriteSubmitDirect::HasKnownRgbTail8003FA20(
                    glyph.rawLocalBytes8003FA20);
        }
        AppendGlyphSubmitResultSub8001B954(out,
                                           glyph,
                                           out.glyphCommandCount - 1u,
                                           runtime8003FA20,
                                           runtimeBuild8003FA20);
    }

    return out;
}

Movie1TextFastSpriteSequenceApplyResultSub8001B954
ApplyTextFastSpriteSequenceSub8001B954(
    const Movie1TextFastSpriteSequenceSub8001B954& sequence,
    PrPsxFastSpriteSubmitDirect::RuntimeState8003FA20& runtime) {
    Movie1TextFastSpriteSequenceApplyResultSub8001B954 out{};
    out.valid = sequence.valid;
    if (!sequence.valid || !sequence.fastSpriteOwnerKnown) {
        return out;
    }

    for (size_t i = 0; i < sequence.glyphSubmitResultCount &&
                       i < sequence.glyphSubmitResults.size();
         ++i) {
        const Movie1TextGlyphSubmitResultSub8001B954& submit =
            sequence.glyphSubmitResults[i];
        if (!submit.valid || !submit.wouldCall8003FA20) {
            continue;
        }

        if (!submit.localFastSpriteStaticFieldsKnown ||
            !PrPsxFastSpriteSubmitDirect::HasKnownRgbTail8003FA20(
                submit.rawLocalBytes8003FA20)) {
            continue;
        }

        uint16_t priority = kMovie1TextGlyphPrioritySub8001B954;
        if (submit.glyphCommandIndex < sequence.glyphCommandCount &&
            submit.glyphCommandIndex < sequence.glyphCommands.size()) {
            priority = sequence.glyphCommands[submit.glyphCommandIndex].priority;
        }

        PrPsxFastSpriteSubmitDirect::GsSortFastSpriteInput8003FA20 input =
            PrPsxFastSpriteSubmitDirect::BuildInputFromRuntime8003FA20(
                runtime,
                submit.localFastSprite,
                submit.rawLocalBytes8003FA20,
                priority);
        input.provenance.active = true;
        input.provenance.sourceKind = sequence.fastSpriteSourceKind;
        input.provenance.rawLocalProvenance =
            submit.rawLocalBytes8003FA20.provenance;
        input.provenance.sourceFunction = submit.sourceFunction;
        input.provenance.callsite = submit.rawLocalBytes8003FA20.callsite;
        input.provenance.helper =
            PrPsxFastSpriteSubmitDirect::kGsSortFastSpriteFunction8003FA20;
        input.provenance.psxCallOrder =
            static_cast<uint32_t>(i + 1u);
        input.provenance.localObjectAddr =
            submit.rawLocalBytes8003FA20.localObjectAddr;
        input.provenance.lastWriterPc =
            submit.rawLocalBytes8003FA20.lastWriterPc;

        const PrPsxFastSpriteSubmitDirect::GsSortFastSpriteResult8003FA20
            callResult =
                PrPsxFastSpriteSubmitDirect::PsxCall8003FA20_GsSortFastSprite(
                    input);
        const PrPsxFastSpriteSubmitDirect::RuntimeUpdate8003FA20 update =
            PrPsxFastSpriteSubmitDirect::ApplyRuntimeUpdate8003FA20(
                runtime,
                callResult);
        if (update.skipped) {
            continue;
        }
        if (!update.packetWriteMirrored || update.packetWriteCapacityExceeded ||
            update.otSlotCapacityExceeded) {
            continue;
        }
        ++out.appliedSubmitCount;
    }
    return out;
}

static void ApplyFlushWorkListSubmitSub8001ED3C(
    Movie1TextFlushActionSub8001ED3C& action,
    PrPsxGraphOwnerDirect::PsxGraphState& graph) {
    if (!action.requested || !action.currentWork.gp872SlotKnown ||
        !graph.mainPageWorkLists80087288Initialized) {
        return;
    }

    const uint8_t slot =
        static_cast<uint8_t>(action.currentWork.gp872Slot & 1u);
    const PrPsxGraphOwnerDirect::PsxGraphPageWorkList8001E374& pageWork =
        graph.mainPageWorkLists80087288[slot];
    PrPsxEventFrameDirect::PsxCall80040CA4_SubmitWorkList(pageWork.workAddr,
                                                          slot,
                                                          pageWork.work);
}

Movie1TextFastSpriteSequenceApplyResultSub8001B954
ApplyMovieTextWindowSubmitSub801C77C0(
    Movie1TextWindowTickResult& tickResult,
    PrPsxGraphOwnerDirect::PsxGraphState& graph,
    PrPsxFastSpriteSubmitDirect::RuntimeState8003FA20* fastSpriteRuntime) {
    Movie1TextFastSpriteSequenceApplyResultSub8001B954 apply{};
    if (fastSpriteRuntime != nullptr &&
        tickResult.displayActionSub8001EC54Ctx7.requested) {
        apply = ApplyTextFastSpriteSequenceSub8001B954(
            tickResult.displayActionSub8001EC54Ctx7
                .textFastSpriteSequenceSub8001B954,
            *fastSpriteRuntime);

        const Movie1TextCurrentGp872WorkCarrier& currentWork =
            tickResult.displayActionSub8001EC54Ctx7.currentWork;
        if (currentWork.gp872SlotKnown) {
            PrPsxGraphOwnerDirect::CommitRuntimeState8003FA20ToMainPageWork(
                graph,
                currentWork.gp872Slot,
                *fastSpriteRuntime);
        }
    }

    ApplyFlushWorkListSubmitSub8001ED3C(
        tickResult.flushActionSub8001ED3C,
        graph);
    return apply;
}

static PrStage1CommonLyricsTextGeometrySub8001B954
BuildCommonLyricsTextGeometrySub8001B954(
    const Movie1TextFastSpriteSequenceSub8001B954& sequence) {
    PrStage1CommonLyricsTextGeometrySub8001B954 out{};
    if (!sequence.valid ||
        !sequence.glyphMetricTablesKnown ||
        sequence.lineCapacityExceeded ||
        sequence.lineCount == 0u) {
        return out;
    }

    out.valid = true;
    out.lineCount = static_cast<uint8_t>(
        (std::min)(sequence.lineCount, out.lines.size()));
    for (size_t i = 0; i < out.lineCount; ++i) {
        const Movie1TextGlyphLineSub8001B954& src = sequence.lines[i];
        PrStage1CommonLyricsLineGeometrySub8001B954& dst = out.lines[i];
        dst.valid = src.valid && src.centeredXKnown && src.widthKnown;
        dst.textStartIndex = src.textStartIndex;
        dst.textByteCount = src.textByteCount;
        dst.widthPx = src.widthPx;
        dst.centeredX = src.centeredX;
        dst.y = src.y;
        out.valid = out.valid && dst.valid;
    }
    return out;
}

static PrStage1CommonLyricsFrameSub8001C864
BuildCommonLyricsFrameSub8001C864(
    const Movie1TextDisplayActionSub8001EC54& action) {
    PrStage1CommonLyricsFrameSub8001C864 out{};
    if (!action.requested ||
        action.helper !=
            Movie1TextDisplayHelperSub8001EC54::Sub8001DB00_Text268) {
        return out;
    }

    for (size_t i = 0; i < action.drawCommandCount; ++i) {
        const Movie1TextDrawCommand& command = action.drawCommands[i];
        if (command.kind !=
                Movie1TextDrawCommandKind::FinalizeTextSub8001C864 ||
            !command.finalizeStateSub8001C864.valid) {
            continue;
        }

        out.valid = true;
        out.arg0Priority =
            command.finalizeStateSub8001C864.arg0_priority;
        const size_t count =
            (std::min)(command.finalizeStateSub8001C864.submitCount,
                       out.submits.size());
        out.submitCount = static_cast<uint8_t>(count);
        for (size_t j = 0; j < count; ++j) {
            const Movie1TextFinalizeSubmitSub8001C864& src =
                command.finalizeStateSub8001C864.submits[j];
            PrStage1CommonLyricsFrameSubmitSub8001C864& dst =
                out.submits[j];
            dst.valid = src.valid;
            dst.callOrder = src.callOrder;
            dst.screenX = src.screenX;
            dst.screenY = src.screenY;
            dst.templateAddr = src.templateAddr;
            dst.priority = src.priority;
            dst.layer = PrStage1Scene1Movie1Direct::kMovie1PsxDefaultLayer;
        }
        return out;
    }

    return out;
}

Movie1TextWindowTickResult TickMovie1TextWindowSub801C77C0(
    Movie1TextRuntime& runtime,
    const Movie1TextWindowInput& input,
    uint8_t languageIndex,
    uint32_t queryFrame60) {
    Movie1TextWindowTickResult result{};
    ClearCommonLyricsWindowSnapshot(runtime);

    const PrStage1VTextDirectMovieWindowResult window =
        ResolvePlayAndWaitWindowSub801C77C0(
            runtime,
            input);

    if (!window.active || window.mode >= runtime.commonLyricsTracks.size()) {
        PrStage1VTextDirectResetSub80024C84(runtime.commonLyricsVText);
        return result;
    }

    if (!GetCommonLyricsTrackForMode(runtime, window.mode)) {
        PrStage1VTextDirectResetSub80024C84(runtime.commonLyricsVText);
        ApplyDisplayActionsSub801C77C0(result, runtime, input);
        return result;
    }

    AdvanceCommonLyricsSub80024CF8(runtime,
                                  window.mode,
                                  ResolveLanguageIndex(languageIndex),
                                  window.queryFrame30);
    ApplyDisplayActionsSub801C77C0(result, runtime, input);

    result.commonLyricsSnapshotValid =
        ResolveCommonLyricsSnapshotForWindow(runtime,
                                             window,
                                             ResolveLanguageIndex(languageIndex),
                                             queryFrame60,
                                             result.commonLyricsSnapshot);
    runtime.commonLyricsSnapshot = result.commonLyricsSnapshot;
    runtime.commonLyricsSnapshotValid = result.commonLyricsSnapshotValid;
    return result;
}

static Movie1TextWindowInput BuildMovie1TextWindowInputFromSceneFrameSub801C77C0(
    const Movie1TextScene1FrameInputSub801C77C0& frameInput) {
    Movie1TextWindowInput input{};
    input.word800916DC = frameInput.globalOptions.word800916DCKnown
                             ? frameInput.globalOptions.word800916DC
                             : (frameInput.globalOptions.subtitleEnabled ? 1u : 0u);
    input.movieFrame30 = ClampFrame30ToU32(frameInput.movieFrame30);
    input.currentGp872SlotKnown = frameInput.currentGp872SlotKnown;
    input.currentGp872Slot = frameInput.currentGp872Slot;
    return input;
}

static uint32_t ResolveMovie1TextWindowQueryFrame60Sub801C77C0(
    const Movie1TextScene1FrameInputSub801C77C0& frameInput) {
    const uint32_t movieFrame30 = ClampFrame30ToU32(frameInput.movieFrame30);
    const uint32_t runnerFrame30 = ClampFrame30ToU32(frameInput.runnerFrame30);
    return frameInput.stageRunning
               ? runnerFrame30 * 2u + (frameInput.renderHalfStep ? 1u : 0u)
               : movieFrame30 * 2u;
}

Movie1TextWindowTickResult
BuildMovie1TextWindowAfterOuterAdvanceSub801C455C(
    Movie1TextRuntime& runtime,
    const Movie1TextScene1FrameInputSub801C77C0& frameInput,
    uint8_t languageIndex) {
    Movie1TextWindowTickResult result{};
    ClearCommonLyricsWindowSnapshot(runtime);

    const Movie1TextWindowInput input =
        BuildMovie1TextWindowInputFromSceneFrameSub801C77C0(frameInput);
    const uint32_t queryFrame60 =
        ResolveMovie1TextWindowQueryFrame60Sub801C77C0(frameInput);
    const PrStage1VTextDirectMovieWindowResult window =
        ResolvePlayAndWaitWindowSub801C77C0(runtime, input);

    if (!window.active || window.mode >= runtime.commonLyricsTracks.size()) {
        PrStage1VTextDirectResetSub80024C84(runtime.commonLyricsVText);
        return result;
    }

    if (!GetCommonLyricsTrackForMode(runtime, window.mode)) {
        PrStage1VTextDirectResetSub80024C84(runtime.commonLyricsVText);
        ApplyDisplayActionsSub801C77C0(result, runtime, input);
        return result;
    }

    const uint8_t selectedLanguageIndex = ResolveLanguageIndex(languageIndex);
    result.commonLyricsSnapshotValid =
        ResolveCommonLyricsSnapshotForWindow(runtime,
                                             window,
                                             selectedLanguageIndex,
                                             queryFrame60,
                                             result.commonLyricsSnapshot);
    if (!result.commonLyricsSnapshot.producerActive ||
        !result.commonLyricsSnapshot.valid) {
        AdvanceCommonLyricsSub80024CF8(runtime,
                                      window.mode,
                                      selectedLanguageIndex,
                                      window.queryFrame30);
        result.commonLyricsSnapshotValid =
            ResolveCommonLyricsSnapshotForWindow(runtime,
                                                 window,
                                                 selectedLanguageIndex,
                                                 queryFrame60,
                                                 result.commonLyricsSnapshot);
    }

    ApplyDisplayActionsSub801C77C0(result, runtime, input);
    runtime.commonLyricsSnapshot = result.commonLyricsSnapshot;
    runtime.commonLyricsSnapshotValid = result.commonLyricsSnapshotValid;
    return result;
}

Movie1TextCommonLyricsHudCarrierSub8001EC54
BuildCommonLyricsHudCarrierSub8001EC54(
    const Movie1TextWindowTickResult& tickResult,
    const PrStage1CommonLyricsSnapshot& commonLyrics) {
    Movie1TextCommonLyricsHudCarrierSub8001EC54 out{};
    out.displayActionRequested =
        tickResult.displayActionSub8001EC54Ctx7.requested;
    out.displayActionUsesText268 =
        tickResult.displayActionSub8001EC54Ctx7.helper ==
        Movie1TextDisplayHelperSub8001EC54::Sub8001DB00_Text268;
    if (!out.displayActionRequested || !out.displayActionUsesText268) {
        return out;
    }

    out.textGeometrySub8001B954 = BuildCommonLyricsTextGeometrySub8001B954(
        tickResult.displayActionSub8001EC54Ctx7
            .textFastSpriteSequenceSub8001B954);
    out.hasTextGeometrySub8001B954 = out.textGeometrySub8001B954.valid;
    out.frameSub8001C864 =
        BuildCommonLyricsFrameSub8001C864(
            tickResult.displayActionSub8001EC54Ctx7);
    out.hasFrameSub8001C864 = out.frameSub8001C864.valid;
    out.valid = true;
    out.commonLyrics = commonLyrics;
    if (out.hasTextGeometrySub8001B954) {
        out.commonLyrics.textGeometrySub8001B954 =
            out.textGeometrySub8001B954;
    }
    if (out.hasFrameSub8001C864) {
        out.commonLyrics.valid = true;
        out.commonLyrics.frameSub8001C864 = out.frameSub8001C864;
    }
    return out;
}

bool GetCommonLyricsSnapshot(const Movie1TextRuntime& runtime,
                             PrStage1CommonLyricsSnapshot& out) {
    out = runtime.commonLyricsSnapshot;
    return runtime.commonLyricsSnapshotValid;
}

}  // namespace PrStage1MovieTextDirect
