#include "scene_event_parser.h"
#include "../logger.h"
#include <cstring>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <unordered_map>

// Lightweight JSON parsing (no external deps).
// Only handles the flat structure of psx_face_tables.json.

static uint32_t readU32(const uint8_t* p) {
    return p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24);
}

static int16_t readS16(const uint8_t* p) {
    return static_cast<int16_t>(p[0] | (p[1] << 8));
}

static bool psxToOff(uint32_t addr, uint32_t base, size_t fileSize, size_t& out) {
    if (addr < base) return false;
    size_t off = static_cast<size_t>(addr - base);
    if (off >= fileSize) return false;
    out = off;
    return true;
}

// ---- Minimal JSON helpers ----
static std::string readFileToString(const std::string& path) {
    std::ifstream fs(path, std::ios::binary);
    if (!fs) return {};
    std::ostringstream ss;
    ss << fs.rdbuf();
    return ss.str();
}

// Skip whitespace
static size_t skipWs(const std::string& s, size_t p) {
    while (p < s.size() && (s[p] == ' ' || s[p] == '\t' || s[p] == '\n' || s[p] == '\r'))
        p++;
    return p;
}

// Parse integer (may be negative)
static int parseInt(const std::string& s, size_t& p) {
    p = skipWs(s, p);
    int sign = 1;
    if (p < s.size() && s[p] == '-') { sign = -1; p++; }
    int val = 0;
    while (p < s.size() && s[p] >= '0' && s[p] <= '9') {
        val = val * 10 + (s[p] - '0');
        p++;
    }
    return sign * val;
}

// Find key in JSON object at current level
static size_t findKey(const std::string& s, size_t start, const std::string& key) {
    std::string needle = "\"" + key + "\"";
    size_t pos = s.find(needle, start);
    if (pos == std::string::npos) return std::string::npos;
    // Skip past colon
    pos += needle.size();
    pos = skipWs(s, pos);
    if (pos < s.size() && s[pos] == ':') pos++;
    return skipWs(s, pos);
}

static bool IsOverlayEntryValid(uint32_t frameOffset, const int16_t timIndices[4]) {
    if (frameOffset > 5000u) return false;

    bool seenZero = false;
    for (int i = 0; i < 4; i++) {
        const int16_t v = timIndices[i];
        if (v < 0 || v > 255) return false;
        if (seenZero && v != 0) return false;
        if (v == 0) seenZero = true;
    }
    return true;
}

static bool LoadPsxTablesFromBin(const uint8_t* binData, size_t binSize,
                                 uint32_t psxBase,
                                 uint32_t descOffHint,
                                 SceneEventData& out) {
    if (!binData || binSize < 64) return false;

    int maxPairLookup = 0;
    int maxOverlaySlot = 0;
    for (const auto& ev : out.events) {
        maxPairLookup = std::max<int>(maxPairLookup, ev.b10);
        maxPairLookup = std::max<int>(maxPairLookup, ev.b13);
        maxPairLookup = std::max<int>(maxPairLookup, ev.b14);
        maxOverlaySlot = std::max<int>(maxOverlaySlot, ev.b11);
    }

    // Scene0 TIM-pair table is 1-based and typically has indices [0..8].
    const int pairCountGoal = std::max<int>(9, maxPairLookup + 1);

    const size_t scanStart = (descOffHint > 0x300u) ? (size_t)(descOffHint - 0x300u) : 0u;
    const size_t scanEnd = std::min<size_t>(binSize, static_cast<size_t>(descOffHint) + 1u);
    if (scanEnd <= scanStart || scanEnd - scanStart < static_cast<size_t>(pairCountGoal) * 4u) {
        return false;
    }

    int bestScore = -0x3fffffff;
    size_t bestPairOff = static_cast<size_t>(-1);
    for (size_t off = scanStart; off + static_cast<size_t>(pairCountGoal) * 4u <= scanEnd; off += 2) {
        bool ok = true;
        int nonZeroPairs = 0;
        int monotonicA = 0;
        int prevA = -1;

        const int firstA = readS16(binData + off);
        const int firstB = readS16(binData + off + 2u);
        const bool leadingZeroPair = (firstA == 0 && firstB == 0);

        const int secondA = readS16(binData + off + 4u);
        const int secondB = readS16(binData + off + 6u);
        const bool secondIsFullNonZero = (secondA != 0 && secondB != 0);

        const int lastA = readS16(binData + off + static_cast<size_t>(pairCountGoal - 1) * 4u);
        const int lastB = readS16(binData + off + static_cast<size_t>(pairCountGoal - 1) * 4u + 2u);
        const bool trailingZeroPair = (lastA == 0 && lastB == 0);
        const bool suspiciousLastHalfZero = ((lastA != 0) != (lastB != 0));

        const bool aligned4 = ((off & 3u) == 0u);

        for (int i = 0; i < pairCountGoal; i++) {
            const int a = readS16(binData + off + static_cast<size_t>(i) * 4u);
            const int b = readS16(binData + off + static_cast<size_t>(i) * 4u + 2u);
            if (a < 0 || a > 255 || b < 0 || b > 255) {
                ok = false;
                break;
            }
            if (a != 0 || b != 0) nonZeroPairs++;
            if (prevA >= 0 && a >= prevA) monotonicA++;
            prevA = a;
        }
        if (!ok) continue;
        if (nonZeroPairs < std::max<int>(3, maxPairLookup)) continue;

        const int dist = (descOffHint >= off)
            ? static_cast<int>(descOffHint - off)
            : static_cast<int>(off - static_cast<size_t>(descOffHint));
        const int score = nonZeroPairs * 100
                        + monotonicA * 3
                        - (dist / 8)
                        + (leadingZeroPair ? 200 : 0)
                        + (leadingZeroPair && secondIsFullNonZero ? 120 : 0)
                        + (aligned4 ? 60 : 0)
                        + (trailingZeroPair ? 50 : 0);
        const int score2 = score
                         - (leadingZeroPair && !secondIsFullNonZero ? 150 : 0)
                         - (suspiciousLastHalfZero ? 120 : 0);
        if (score2 > bestScore) {
            bestScore = score2;
            bestPairOff = off;
        }
    }

    if (bestPairOff == static_cast<size_t>(-1)) {
        return false;
    }

    out.timPairTable.clear();
    out.timPairTable.reserve(static_cast<size_t>(pairCountGoal) + 1u);
    for (int i = 0; i < pairCountGoal; i++) {
        TimPairEntry e = {};
        e.timA = readS16(binData + bestPairOff + static_cast<size_t>(i) * 4u);
        e.timB = readS16(binData + bestPairOff + static_cast<size_t>(i) * 4u + 2u);
        out.timPairTable.push_back(e);
    }

    // Ensure index 0 exists as {0,0} for 1-based lookup compatibility.
    if (!out.timPairTable.empty() && (out.timPairTable[0].timA != 0 || out.timPairTable[0].timB != 0)) {
        out.timPairTable.insert(out.timPairTable.begin(), TimPairEntry{});
    }

    for (int s = 0; s < 6; s++) {
        out.overlaySlots[s].entries.clear();
    }

    // Preferred path: find overlay descriptor list in BIN
    // descriptor layout: {ptr,count,idx(=0)} * N
    bool overlayLoadedByDesc = false;
    int slotsLoadedByDesc = 0;
    size_t bestOverlayDescOff = static_cast<size_t>(-1);
    int bestOverlayDescScore = -0x3fffffff;

    const size_t overlayDescScanStart = (descOffHint > 0x200u) ? (size_t)(descOffHint - 0x200u) : 0u;
    const size_t overlayDescScanEnd = std::min<size_t>(binSize, static_cast<size_t>(descOffHint) + 0x200u);
    for (size_t doff = overlayDescScanStart; doff + 5u * 12u <= overlayDescScanEnd; doff += 4) {
        bool ok = true;
        int totalEntries = 0;

        for (int s = 0; s < 5; s++) {
            const size_t d = doff + static_cast<size_t>(s) * 12u;
            const uint32_t ptr = readU32(binData + d);
            const uint32_t cnt = readU32(binData + d + 4u);
            const uint32_t idx = readU32(binData + d + 8u);
            if ((ptr & 0xFF000000u) != 0x80000000u || cnt == 0u || cnt > 16u || idx != 0u) {
                ok = false;
                break;
            }

            size_t slotOff = 0;
            if (!psxToOff(ptr, psxBase, binSize, slotOff) || slotOff + static_cast<size_t>(cnt) * 12u > binSize) {
                ok = false;
                break;
            }

            for (uint32_t i = 0; i < cnt; i++) {
                HudOverlayEntry e = {};
                const size_t eo = slotOff + static_cast<size_t>(i) * 12u;
                e.frameOffset = readU32(binData + eo);
                for (int k = 0; k < 4; k++) {
                    e.timIndices[k] = readS16(binData + eo + 4u + static_cast<size_t>(k) * 2u);
                }
                if (!IsOverlayEntryValid(e.frameOffset, e.timIndices)) {
                    ok = false;
                    break;
                }
            }
            if (!ok) break;
            totalEntries += static_cast<int>(cnt);
        }

        if (!ok) continue;

        const int dist = (descOffHint >= doff)
            ? static_cast<int>(descOffHint - doff)
            : static_cast<int>(doff - static_cast<size_t>(descOffHint));
        const int score = totalEntries * 100 - (dist / 8);
        if (score > bestOverlayDescScore) {
            bestOverlayDescScore = score;
            bestOverlayDescOff = doff;
        }
    }

    if (bestOverlayDescOff != static_cast<size_t>(-1)) {
        for (int s = 1; s <= 5; s++) {
            const size_t d = bestOverlayDescOff + static_cast<size_t>(s - 1) * 12u;
            const uint32_t ptr = readU32(binData + d);
            const uint32_t cnt = readU32(binData + d + 4u);

            size_t slotOff = 0;
            if (!psxToOff(ptr, psxBase, binSize, slotOff) || slotOff + static_cast<size_t>(cnt) * 12u > binSize) {
                continue;
            }

            auto& dst = out.overlaySlots[s].entries;
            dst.reserve(cnt);
            for (uint32_t i = 0; i < cnt; i++) {
                HudOverlayEntry e = {};
                const size_t eo = slotOff + static_cast<size_t>(i) * 12u;
                e.frameOffset = readU32(binData + eo);
                for (int k = 0; k < 4; k++) {
                    e.timIndices[k] = readS16(binData + eo + 4u + static_cast<size_t>(k) * 2u);
                }
                dst.push_back(e);
            }
            if (!dst.empty()) {
                slotsLoadedByDesc++;
            }
        }

        if (slotsLoadedByDesc > 0) {
            overlayLoadedByDesc = true;
            Log::Printf("SceneEventParser: BIN overlay descriptors off=0x%X score=%d slotsLoaded=%d",
                        (unsigned)bestOverlayDescOff,
                        bestOverlayDescScore,
                        slotsLoadedByDesc);
            for (int s = 1; s <= 5; s++) {
                Log::Printf("SceneEventParser: BIN overlay slot %d: %zu entries",
                            s,
                            out.overlaySlots[s].entries.size());
            }
        }
    }

    if (overlayLoadedByDesc) {
        Log::Printf("SceneEventParser: BIN tables timPairOff=0x%X pairs=%zu",
                    (unsigned)bestPairOff,
                    out.timPairTable.size());
        return !out.timPairTable.empty();
    }

    struct FlatOverlay {
        HudOverlayEntry e;
    };
    std::vector<FlatOverlay> flat;
    flat.reserve(32);

    const size_t overlayStart = bestPairOff + static_cast<size_t>(pairCountGoal) * 4u;
    for (size_t off = overlayStart; off + 12 <= binSize && flat.size() < 64; off += 12) {
        FlatOverlay fe = {};
        fe.e.frameOffset = readU32(binData + off);
        for (int k = 0; k < 4; k++) {
            fe.e.timIndices[k] = readS16(binData + off + 4u + static_cast<size_t>(k) * 2u);
        }
        if (!IsOverlayEntryValid(fe.e.frameOffset, fe.e.timIndices)) {
            if (!flat.empty()) {
                break;
            }
            continue;
        }
        flat.push_back(fe);
    }

    while (!flat.empty()) {
        const auto& last = flat.back().e;
        bool allZero = (last.frameOffset == 0u);
        for (int k = 0; k < 4; k++) {
            if (last.timIndices[k] != 0) {
                allZero = false;
                break;
            }
        }
        if (!allZero) break;
        flat.pop_back();
    }

    std::vector<std::pair<size_t, size_t>> segments;
    if (!flat.empty()) {
        size_t segStart = 0;
        for (size_t i = 1; i < flat.size(); i++) {
            if (flat[i].e.frameOffset < flat[i - 1].e.frameOffset) {
                segments.emplace_back(segStart, i);
                segStart = i;
            }
        }
        segments.emplace_back(segStart, flat.size());
    }

    int slotTarget = std::min<int>(5, maxOverlaySlot);
    if (slotTarget <= 0) {
        slotTarget = std::min<int>(5, static_cast<int>(segments.size()));
    } else {
        slotTarget = std::min<int>(slotTarget, static_cast<int>(segments.size()));
    }

    for (int s = 1; s <= slotTarget; s++) {
        const auto seg = segments[(size_t)(s - 1)];
        auto& dst = out.overlaySlots[s].entries;
        dst.reserve(seg.second - seg.first);
        for (size_t i = seg.first; i < seg.second; i++) {
            dst.push_back(flat[i].e);
        }
    }

    Log::Printf("SceneEventParser: BIN tables(fallback) timPairOff=0x%X pairs=%zu overlayFlat=%zu overlaySeg=%zu slotsLoaded=%d maxSlotUsed=%d",
                (unsigned)bestPairOff,
                out.timPairTable.size(),
                flat.size(),
                segments.size(),
                slotTarget,
                maxOverlaySlot);
    for (int s = 1; s <= slotTarget; s++) {
        Log::Printf("SceneEventParser: BIN overlay slot %d: %zu entries",
                    s,
                    out.overlaySlots[s].entries.size());
    }

    return !out.timPairTable.empty();
}

static bool HasAnyOverlayEntries(const SceneEventData& d) {
    for (int s = 1; s <= 5; s++) {
        if (!d.overlaySlots[s].entries.empty()) return true;
    }
    return false;
}

// ---- LoadPsxTables ----
bool SceneEventParser::LoadPsxTables(const std::string& jsonPath, SceneEventData& out) {
    std::string json = readFileToString(jsonPath);
    if (json.empty()) {
        Log::Printf("SceneEventParser: cannot read %s", jsonPath.c_str());
        return false;
    }

    // Parse timPairTable
    size_t tptPos = findKey(json, 0, "timPairTable");
    if (tptPos != std::string::npos && json[tptPos] == '[') {
        // Find each object in the array
        size_t p = tptPos + 1;
        while (p < json.size()) {
            p = skipWs(json, p);
            if (json[p] == ']') break;
            if (json[p] == '{') {
                // Parse object: {"index":N,"timA":N,"timB":N}
                size_t endBrace = json.find('}', p);
                if (endBrace == std::string::npos) break;
                std::string obj = json.substr(p, endBrace - p + 1);

                TimPairEntry entry = {};
                size_t kp = findKey(obj, 0, "timA");
                if (kp != std::string::npos) entry.timA = static_cast<int16_t>(parseInt(obj, kp));
                kp = findKey(obj, 0, "timB");
                if (kp != std::string::npos) entry.timB = static_cast<int16_t>(parseInt(obj, kp));

                out.timPairTable.push_back(entry);
                p = endBrace + 1;
            } else {
                p++;
            }
        }
        Log::Printf("SceneEventParser: loaded %zu TIM pair entries from JSON", out.timPairTable.size());
    }

    // Parse overlaySlots
    size_t osPos = findKey(json, 0, "overlaySlots");
    if (osPos != std::string::npos && json[osPos] == '{') {
        // For each slot 1-5
        for (int s = 1; s <= 5; s++) {
            std::string slotKey = std::to_string(s);
            size_t slotPos = findKey(json, osPos, slotKey);
            if (slotPos == std::string::npos || json[slotPos] != '[') continue;

            size_t p = slotPos + 1;
            while (p < json.size()) {
                p = skipWs(json, p);
                if (json[p] == ']') break;
                if (json[p] == '{') {
                    size_t endBrace = json.find('}', p);
                    if (endBrace == std::string::npos) break;
                    std::string obj = json.substr(p, endBrace - p + 1);

                    HudOverlayEntry entry = {};
                    size_t kp = findKey(obj, 0, "frameOffset");
                    if (kp != std::string::npos) entry.frameOffset = static_cast<uint32_t>(parseInt(obj, kp));

                    kp = findKey(obj, 0, "timIndices");
                    if (kp != std::string::npos && obj[kp] == '[') {
                        size_t ap = kp + 1;
                        for (int k = 0; k < 4; k++) {
                            ap = skipWs(obj, ap);
                            if (ap >= obj.size() || obj[ap] == ']') break;
                            entry.timIndices[k] = static_cast<int16_t>(parseInt(obj, ap));
                            ap = skipWs(obj, ap);
                            if (ap < obj.size() && obj[ap] == ',') ap++;
                        }
                    }

                    out.overlaySlots[s].entries.push_back(entry);
                    p = endBrace + 1;
                } else {
                    p++;
                }
            }
            Log::Printf("SceneEventParser: overlay slot %d: %zu entries",
                         s, out.overlaySlots[s].entries.size());
        }
    }

    return true;
}

// ---- FindBaseAddress ----
uint32_t SceneEventParser::FindBaseAddress(const uint8_t* binData, size_t binSize) {
    if (binSize < 128) return 0;

    uint32_t freqBestBase = 0;
    int freqBestCount = 0;
    {
        std::unordered_map<uint32_t, int> freq;
        freq.reserve(binSize / 4);
        for (size_t off = 0; off + 4 <= binSize; off += 4) {
            uint32_t v = readU32(binData + off);
            if ((v & 0xFF000000) != 0x80000000) continue;
            uint32_t base = v - (uint32_t)off;
            if ((base & 0xFF000000) != 0x80000000) continue;
            freq[base]++;
        }
        for (const auto& kv : freq) {
            if (kv.second > freqBestCount) {
                freqBestBase = kv.first;
                freqBestCount = kv.second;
            }
        }
        if (freqBestBase != 0 && freqBestCount >= 8) {
            Log::Printf("SceneEventParser: base guess=0x%08X (hits=%d)", freqBestBase, freqBestCount);
        }
    }

    uint64_t bestScore = 0;
    uint32_t bestBase = 0;
    size_t bestEvtOff = 0;
    size_t bestDescOff = 0;
    uint32_t bestCount = 0;
    uint32_t bestPayload = 0;
    uint32_t bestSpan = 0;
    bool bestAdvance = false;

    for (size_t off = 0; off + 7 * 16 <= binSize; off += 4) {
        uint32_t f0 = readU32(binData + off);
        if (f0 > 10000) continue;

        bool anyPayload0 = false;
        for (int i = 0; i < 8 && off + (size_t)(i + 1) * 16 <= binSize; i++) {
            const uint8_t* p = binData + off + (size_t)i * 16;
            if (readU32(p + 4) != 0) {
                anyPayload0 = true;
                break;
            }
            for (int k = 8; k < 16; k++) {
                if (p[k] != 0) {
                    anyPayload0 = true;
                    break;
                }
            }
            if (anyPayload0) break;
        }
        if (!anyPayload0) continue;

        int monotonicCount = 1;
        uint32_t prevFrame = f0;
        for (int i = 1; i < 5000 && off + (i + 1) * 16 <= binSize; i++) {
            uint32_t fi = readU32(binData + off + i * 16);
            if (fi < prevFrame || fi > 10000) break;
            prevFrame = fi;
            monotonicCount++;
        }
        if (monotonicCount < 5) continue;

        // Evaluate all descriptors against this candidate event stream and pick best score.
        for (size_t doff = 0; doff + 12 <= binSize; doff += 4) {
            uint32_t dPtr = readU32(binData + doff);
            uint32_t dCount = readU32(binData + doff + 4);
            uint32_t dIdx = readU32(binData + doff + 8);

            if (dCount < 3 || dCount > 5000) continue;
            if ((dPtr & 0xFF000000) != 0x80000000) continue;
            if (dCount > static_cast<uint32_t>(monotonicCount)) continue;
            if (off + static_cast<size_t>(dCount) * 16 > binSize) continue;

            uint32_t candidateBase = dPtr - static_cast<uint32_t>(off);
            if ((candidateBase & 0xFF000000) != 0x80000000) continue;

            bool ok = true;
            uint32_t payloadCount = 0;
            uint32_t firstFrame = 0;
            uint32_t maxFrame = 0;
            bool hasFrameAdvance = false;
            uint32_t prev = 0;

            for (uint32_t i = 0; i < dCount; i++) {
                const uint8_t* p = binData + off + static_cast<size_t>(i) * 16;
                uint32_t frame = readU32(p);
                if (frame > 10000 || (i > 0 && frame < prev)) {
                    ok = false;
                    break;
                }
                if (i == 0) {
                    firstFrame = frame;
                    maxFrame = frame;
                } else {
                    if (frame > prev) {
                        hasFrameAdvance = true;
                    }
                    if (frame > maxFrame) {
                        maxFrame = frame;
                    }
                }
                prev = frame;

                uint32_t flags = readU32(p + 4);
                bool any = (flags != 0);
                for (int k = 8; k <= 14; k++) {
                    if (p[k] != 0) {
                        any = true;
                        break;
                    }
                }
                if (any) payloadCount++;
            }

            if (!ok || payloadCount == 0) continue;

            uint32_t frameSpan = (maxFrame >= firstFrame) ? (maxFrame - firstFrame) : 0;
            uint64_t score = (hasFrameAdvance ? 100000000ull : 0ull)
                           + static_cast<uint64_t>(payloadCount) * 100000ull
                           + static_cast<uint64_t>(frameSpan) * 10ull
                           + static_cast<uint64_t>(dCount);

            if (score > bestScore) {
                bestScore = score;
                bestBase = candidateBase;
                bestEvtOff = off;
                bestDescOff = doff;
                bestCount = dCount;
                bestPayload = payloadCount;
                bestSpan = frameSpan;
                bestAdvance = hasFrameAdvance;
            }
        }
    }

    if (bestBase != 0) {
        Log::Printf("SceneEventParser: found base=0x%08X (events@0x%04zX count=%u desc@0x%04zX payload=%u span=%u adv=%d score=%llu)",
                     bestBase,
                     bestEvtOff,
                     bestCount,
                     bestDescOff,
                     bestPayload,
                     bestSpan,
                     bestAdvance ? 1 : 0,
                     static_cast<unsigned long long>(bestScore));
        return bestBase;
    }

    if (freqBestBase != 0) {
        Log::Printf("SceneEventParser: using base guess fallback=0x%08X (hits=%d)", freqBestBase, freqBestCount);
        return freqBestBase;
    }

    return 0;
}

// ---- ParseEventStream ----
bool SceneEventParser::ParseEventStream(const uint8_t* binData, size_t binSize,
                                         uint32_t psxBase, uint32_t descAddr,
                                         SceneEventData& out) {
    size_t descOff;
    if (!psxToOff(descAddr, psxBase, binSize, descOff) || descOff + 12 > binSize)
        return false;

    uint32_t evtPtr   = readU32(binData + descOff);
    uint32_t evtCount = readU32(binData + descOff + 4);
    if (evtCount == 0 || evtCount > 5000) return false;

    size_t evtOff;
    if (!psxToOff(evtPtr, psxBase, binSize, evtOff)) return false;
    if (evtOff + evtCount * 16 > binSize) return false;

    out.events.resize(evtCount);
    uint32_t prevFrame = 0;
    const uint32_t headPrint = 16;
    const uint32_t tailPrint = 8;
    for (uint32_t i = 0; i < evtCount; i++) {
        const uint8_t* p = binData + evtOff + i * 16;
        out.events[i].frame = readU32(p);
        out.events[i].flags = readU32(p + 4);
        out.events[i].b8    = p[8];
        out.events[i].b9    = p[9];
        out.events[i].b10   = p[10];
        out.events[i].b11   = p[11];
        out.events[i].b12   = p[12];
        out.events[i].b13   = p[13];
        out.events[i].b14   = p[14];
        out.events[i].b15   = p[15];
        if (out.events[i].frame > 10000 || (i > 0 && out.events[i].frame < prevFrame)) {
            return false;
        }
        prevFrame = out.events[i].frame;

        const uint32_t tailStart = (evtCount > tailPrint) ? (evtCount - tailPrint) : 0;
        if (i < headPrint || i >= tailStart) {
            Log::Printf("  Event %d: frame=%d flags=0x%08X b8=%d b9=%d b10=%d b11=%d b12=%d b13=%d b14=%d",
                         i, out.events[i].frame, out.events[i].flags,
                         out.events[i].b8, out.events[i].b9, out.events[i].b10, out.events[i].b11,
                         out.events[i].b12, out.events[i].b13, out.events[i].b14);
        } else if (i == headPrint) {
            Log::Printf("  ... (%u events omitted) ...", (unsigned)(tailStart > headPrint ? (tailStart - headPrint) : 0));
        }
    }
    return true;
}

bool SceneEventParser::Parse(const uint8_t* binData, size_t binSize,
                              const std::string& jsonPath, SceneEventData& out) {
    out = SceneEventData{};
    if (!binData || binSize < 128) return false;

    Log::Printf("SceneEventParser::Parse: binSize=%zu json=%s", binSize, jsonPath.c_str());

    // Step 1: Find PSX base address
    uint32_t psxBase = FindBaseAddress(binData, binSize);
    if (psxBase == 0) {
        Log::Printf("SceneEventParser: could not find PSX base address");
        return false;
    }

    // Step 2: Find event descriptor
    uint32_t descAddr = 0;
    uint32_t bestScore = 0;
    uint32_t bestCount = 0;
    uint32_t bestPayload = 0;
    uint32_t bestSpan = 0;
    uint32_t bestMaxOverlaySlot = 0;
    uint32_t bestB11Events = 0;
    uint32_t bestMaxPairLookup = 0;
    uint32_t bestDescOff = 0;
    for (size_t doff = 0; doff + 12 <= binSize; doff += 4) {
        uint32_t dPtr   = readU32(binData + doff);
        uint32_t dCount = readU32(binData + doff + 4);
        uint32_t dIdx   = readU32(binData + doff + 8);
        if (dCount < 3 || dCount > 5000) continue;
        if ((dPtr & 0xFF000000) != 0x80000000) continue;

        size_t evtOff;
        if (!psxToOff(dPtr, psxBase, binSize, evtOff)) continue;
        if (evtOff + static_cast<size_t>(dCount) * 16 > binSize) continue;

        bool ok = true;
        uint32_t prevFrame = 0;
        uint32_t payloadCount = 0;
        uint32_t firstFrame = 0;
        uint32_t maxFrame = 0;
        bool hasFrameAdvance = false;
        uint32_t maxOverlaySlot = 0;
        uint32_t b11Events = 0;
        uint32_t maxPairLookup = 0;
        uint32_t suspiciousFields = 0;
        for (uint32_t i = 0; i < dCount; i++) {
            const uint8_t* p = binData + evtOff + i * 16;
            uint32_t frame = readU32(p);
            if (frame > 10000 || (i > 0 && frame < prevFrame)) {
                ok = false;
                break;
            }
            if (i == 0) {
                firstFrame = frame;
                maxFrame = frame;
            } else {
                if (frame > prevFrame) {
                    hasFrameAdvance = true;
                }
                if (frame > maxFrame) {
                    maxFrame = frame;
                }
            }
            prevFrame = frame;

            uint32_t flags = readU32(p + 4);
            bool any = (flags != 0);
            for (int k = 8; k <= 14; k++) {
                if (p[k] != 0) {
                    any = true;
                    break;
                }
            }
            if (any) payloadCount++;

            // Heuristic: overlay slot id should be in [0..5].
            // Pair lookup indices are small in Scene0 (typically <= 8).
            const uint8_t b10 = p[10];
            const uint8_t b11 = p[11];
            const uint8_t b13 = p[13];
            const uint8_t b14 = p[14];
            if (b11 != 0) {
                b11Events++;
                uint32_t slot = 0;
                if (b11 <= 5) {
                    slot = b11;
                } else {
                    const uint8_t mask = (uint8_t)(b11 & 0x1Fu);
                    for (uint32_t s = 5; s >= 1; s--) {
                        if (mask & (uint8_t)(1u << (s - 1u))) {
                            slot = s;
                            break;
                        }
                    }
                }
                if (slot > maxOverlaySlot) maxOverlaySlot = slot;
            }

            maxPairLookup = std::max<uint32_t>(maxPairLookup, (uint32_t)b10);
            maxPairLookup = std::max<uint32_t>(maxPairLookup, (uint32_t)b13);
            maxPairLookup = std::max<uint32_t>(maxPairLookup, (uint32_t)b14);
            if (b10 > 64 || b13 > 64 || b14 > 64) {
                suspiciousFields++;
            }
        }
        if (!ok) continue;
        if (payloadCount == 0) continue;
        if (!hasFrameAdvance) continue;

        uint32_t frameSpan = (maxFrame >= firstFrame) ? (maxFrame - firstFrame) : 0;
        uint32_t score = payloadCount * 100000 + frameSpan * 10 + dCount;

        // Prefer candidates that actually trigger overlay slots (esp. slots 3-5 used by loop/finger).
        // Penalize suspiciously large pair lookup values.
        score += maxOverlaySlot * 20000000u;
        score += b11Events * 50000u;
        if (suspiciousFields) {
            const uint32_t pen = std::min<uint32_t>(suspiciousFields, 32u) * 200000u;
            score = (score > pen) ? (score - pen) : 0u;
        }

        if (descAddr == 0 || score > bestScore) {
            bestScore = score;
            bestCount = dCount;
            bestPayload = payloadCount;
            bestSpan = frameSpan;
            bestMaxOverlaySlot = maxOverlaySlot;
            bestB11Events = b11Events;
            bestMaxPairLookup = maxPairLookup;
            bestDescOff = static_cast<uint32_t>(doff);
            descAddr = psxBase + static_cast<uint32_t>(doff);
        }
    }

    if (descAddr == 0) {
        Log::Printf("SceneEventParser: could not find event descriptor");
        return false;
    }

    Log::Printf("SceneEventParser: picked descriptor off=0x%X count=%u payload=%u span=%u score=%u maxB11=%u b11Events=%u maxPair=%u",
                 (unsigned)bestDescOff,
                 (unsigned)bestCount,
                 (unsigned)bestPayload,
                 (unsigned)bestSpan,
                 (unsigned)bestScore,
                 (unsigned)bestMaxOverlaySlot,
                 (unsigned)bestB11Events,
                 (unsigned)bestMaxPairLookup);

    // Step 3: Parse events from BIN
    if (!ParseEventStream(binData, binSize, psxBase, descAddr, out)) {
        Log::Printf("SceneEventParser: failed to parse event stream");
        return false;
    }

    // Step 4: Load overlay + TIM-pair tables.
    // Prefer extracting directly from COMOD BIN; fallback to JSON when extraction fails.
    if (!LoadPsxTablesFromBin(binData, binSize, psxBase, bestDescOff, out)) {
        if (!LoadPsxTables(jsonPath, out)) {
            Log::Printf("SceneEventParser: failed to load PSX tables from BIN and JSON (non-fatal)");
        }
    }

    // Fill missing tables from JSON (BIN may only contain a subset of overlay slots).
    SceneEventData jsonTables;
    if (LoadPsxTables(jsonPath, jsonTables)) {
        if (out.timPairTable.empty() && !jsonTables.timPairTable.empty()) {
            Log::Printf("SceneEventParser: timPairTable missing; using JSON (%zu entries)",
                        jsonTables.timPairTable.size());
            out.timPairTable = jsonTables.timPairTable;
        }
        for (int s = 1; s <= 5; s++) {
            if (out.overlaySlots[s].entries.empty() && !jsonTables.overlaySlots[s].entries.empty()) {
                Log::Printf("SceneEventParser: overlay slot %d missing; using JSON (%zu entries)",
                            s,
                            jsonTables.overlaySlots[s].entries.size());
                out.overlaySlots[s] = jsonTables.overlaySlots[s];
            }
        }
    }

    out.valid = true;
    return true;
}
