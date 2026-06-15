#include "pr_sqevs1.h"

#include "logger.h"
#include "pr_game_context.h"
#include "pr_stage1_overlay_parser.h"

#include <vector>
#include <cstring>
#include <algorithm>
#include <cstdio>
#include <unordered_map>
#include <string>
#include <cctype>
#include <array>

namespace PrSqevs1 {

static bool s_inited = false;

static const uint8_t* s_eventSourceBytes = nullptr;
static size_t s_eventSourceSize = 0;
static std::vector<uint8_t> s_eventSourceOwned;
struct SubtitleTextSet {
    std::string texts[5];
};

static std::unordered_map<uint32_t, SubtitleTextSet> s_subtitleTextByEventId;
static std::unordered_map<uint32_t, int> s_subtitleLangByEventId;
static int s_currentLanguageIndex = 0;

static std::string ToLowerAscii(std::string s) {
    for (char& ch : s) {
        ch = (char)std::tolower((unsigned char)ch);
    }
    return s;
}

static bool IsAsciiAlpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

static int CountTokenHits(const std::string& lowerText, const char* token) {
    // token is assumed lowercase ASCII.
    const size_t n = std::strlen(token);
    if (n == 0 || lowerText.size() < n) {
        return 0;
    }

    int hits = 0;
    size_t pos = 0;
    while (true) {
        pos = lowerText.find(token, pos);
        if (pos == std::string::npos) {
            break;
        }
        const bool leftOk = (pos == 0) || !IsAsciiAlpha(lowerText[pos - 1]);
        const bool rightOk = (pos + n >= lowerText.size()) || !IsAsciiAlpha(lowerText[pos + n]);
        if (leftOk && rightOk) {
            hits++;
        }
        pos += n;
    }
    return hits;
}

static int GuessSubtitleLanguageIndex(const std::string& text) {
    if (text.empty()) {
        return -1;
    }
    std::string t = ToLowerAscii(text);

    // 0=EN,1=FR,2=DE,3=ES,4=IT
    int score[5] = { 0, 0, 0, 0, 0 };

    // English
    score[0] += 4 * CountTokenHits(t, "i'll");
    score[0] += 3 * CountTokenHits(t, "i'm");
    score[0] += 2 * CountTokenHits(t, "it's");
    score[0] += 2 * CountTokenHits(t, "you");
    score[0] += 2 * CountTokenHits(t, "the");
    score[0] += 2 * CountTokenHits(t, "and");
    score[0] += 2 * CountTokenHits(t, "here");
    score[0] += 2 * CountTokenHits(t, "please");
    score[0] += 2 * CountTokenHits(t, "thanks");
    score[0] += 2 * CountTokenHits(t, "thank");

    // French
    score[1] += 3 * CountTokenHits(t, "je");
    score[1] += 3 * CountTokenHits(t, "voudrais");
    score[1] += 2 * CountTokenHits(t, "aujourd");
    score[1] += 2 * CountTokenHits(t, "merci");
    score[1] += 2 * CountTokenHits(t, "des");
    score[1] += 2 * CountTokenHits(t, "une");
    score[1] += 2 * CountTokenHits(t, "un");
    score[1] += 2 * CountTokenHits(t, "frites");
    score[1] += 2 * CountTokenHits(t, "glace");
    score[1] += 2 * CountTokenHits(t, "vanille");
    score[1] += 2 * CountTokenHits(t, "et");

    // German
    score[2] += 3 * CountTokenHits(t, "ich");
    score[2] += 3 * CountTokenHits(t, "bitte");
    score[2] += 3 * CountTokenHits(t, "danke");
    score[2] += 3 * CountTokenHits(t, "knusper");
    score[2] += 3 * CountTokenHits(t, "fritten");
    score[2] += 3 * CountTokenHits(t, "shake");
    score[2] += 3 * CountTokenHits(t, "vanille");
    score[2] += 2 * CountTokenHits(t, "heute");
    score[2] += 2 * CountTokenHits(t, "wieder");
    score[2] += 2 * CountTokenHits(t, "ein");
    score[2] += 2 * CountTokenHits(t, "eine");
    score[2] += 2 * CountTokenHits(t, "einen");
    score[2] += 2 * CountTokenHits(t, "und");
    score[2] += 2 * CountTokenHits(t, "riesen");

    // Spanish
    score[3] += 3 * CountTokenHits(t, "gracias");
    score[3] += 3 * CountTokenHits(t, "por");
    score[3] += 3 * CountTokenHits(t, "favor");
    score[3] += 2 * CountTokenHits(t, "hoy");
    score[3] += 2 * CountTokenHits(t, "aqui");
    score[3] += 2 * CountTokenHits(t, "patatas");
    score[3] += 2 * CountTokenHits(t, "y");
    score[3] += 2 * CountTokenHits(t, "el");
    score[3] += 2 * CountTokenHits(t, "la");

    // Italian
    score[4] += 3 * CountTokenHits(t, "grazie");
    score[4] += 3 * CountTokenHits(t, "per");
    score[4] += 3 * CountTokenHits(t, "favore");
    score[4] += 2 * CountTokenHits(t, "il");
    score[4] += 2 * CountTokenHits(t, "una");
    score[4] += 2 * CountTokenHits(t, "un");
    score[4] += 2 * CountTokenHits(t, "ragazza");
    score[4] += 2 * CountTokenHits(t, "miei");
    score[4] += 2 * CountTokenHits(t, "sogni");
    score[4] += 2 * CountTokenHits(t, "prossimo");
    score[4] += 2 * CountTokenHits(t, "patatine");
    score[4] += 3 * CountTokenHits(t, "vaniglia");
    score[4] += 2 * CountTokenHits(t, "maxigelato");

    int bestLang = 0;
    int bestScore = score[0];
    int secondScore = -1;
    for (int i = 1; i < 5; i++) {
        if (score[i] > bestScore) {
            secondScore = bestScore;
            bestScore = score[i];
            bestLang = i;
        } else if (score[i] > secondScore) {
            secondScore = score[i];
        }
    }

    // Require some confidence to avoid misclassification on very short/neutral strings.
    if (bestScore <= 0) {
        return -1;
    }
    if (bestScore == secondScore) {
        return -1;
    }
    return bestLang;
}

static char DecodeByteToAscii(uint8_t c) {
    if (c < 0x80) {
        return (char)c;
    }
    switch (c) {
        case 0xA0: return ' ';
        case 0x91: return '\'';
        case 0x92: return '\'';
        case 0x93: return '"';
        case 0x94: return '"';
        case 0x96: return '-';
        case 0x97: return '-';
        case 0xA1: return '!';
        case 0xAB: return '"';
        case 0xBB: return '"';
        case 0xBF: return '?';

        case 0xC0: case 0xC1: case 0xC2: case 0xC3: case 0xC4: case 0xC5: return 'A';
        case 0xC6: return 'A';
        case 0xC7: return 'C';
        case 0xC8: case 0xC9: case 0xCA: case 0xCB: return 'E';
        case 0xCC: case 0xCD: case 0xCE: case 0xCF: return 'I';
        case 0xD0: return 'D';
        case 0xD1: return 'N';
        case 0xD2: case 0xD3: case 0xD4: case 0xD5: case 0xD6: case 0xD8: return 'O';
        case 0xD9: case 0xDA: case 0xDB: case 0xDC: return 'U';
        case 0xDD: return 'Y';
        case 0xDE: return 'T';
        case 0xDF: return 's';

        case 0xE0: case 0xE1: case 0xE2: case 0xE3: case 0xE4: case 0xE5: return 'a';
        case 0xE6: return 'a';
        case 0xE7: return 'c';
        case 0xE8: case 0xE9: case 0xEA: case 0xEB: return 'e';
        case 0xEC: case 0xED: case 0xEE: case 0xEF: return 'i';
        case 0xF0: return 'd';
        case 0xF1: return 'n';
        case 0xF2: case 0xF3: case 0xF4: case 0xF5: case 0xF6: case 0xF8: return 'o';
        case 0xF9: case 0xFA: case 0xFB: case 0xFC: return 'u';
        case 0xFD: case 0xFF: return 'y';
        case 0xFE: return 't';
        default:
            break;
    }
    return '?';
}

static bool IsTokenChar(uint8_t c) {
    if (c == 0 || c == '\n') {
        return false;
    }
    if (c < 0x20 || c == 0x7F) {
        return false;
    }
    const char a = DecodeByteToAscii(c);
    if ((a >= 'A' && a <= 'Z') || (a >= 'a' && a <= 'z')) {
        return true;
    }
    if ((a >= '0' && a <= '9') || a == '\'' || a == '-') {
        return true;
    }
    return false;
}

static bool IsSpaceLike(uint8_t c) {
    return c == ' ' || c == '\t' || c == 0xA0;
}

static bool TryReadSubtitleTextAtOffset(uint32_t off, std::string& out, int& outScore) {
    out.clear();
    outScore = 0;
    if (!s_eventSourceBytes || s_eventSourceSize == 0) {
        return false;
    }
    if (off >= s_eventSourceSize) {
        return false;
    }

    const size_t maxLen = 220;
    size_t i = (size_t)off;
    {
        const size_t backMax = 16;
        size_t back = 0;
        while (i > 0 && back < backMax) {
            const uint8_t prev = s_eventSourceBytes[i - 1];
            if (!IsTokenChar(prev)) {
                break;
            }
            i--;
            back++;
        }
        auto BacktrackWordPrefix = [&](size_t cur) -> size_t {
            if (cur < 2) return cur;
            if (!IsSpaceLike(s_eventSourceBytes[cur - 1])) return cur;
            if (!IsTokenChar(s_eventSourceBytes[cur - 2])) return cur;

            size_t end = cur - 2;
            size_t start = end;
            size_t len = 1;
            while (start > 0 && len < 8 && IsTokenChar(s_eventSourceBytes[start - 1])) {
                start--;
                len++;
            }

            if (len == 2) {
                return start;
            }

            if (len == 3) {
                const char c0 = DecodeByteToAscii(s_eventSourceBytes[start + 0]);
                const char c1 = DecodeByteToAscii(s_eventSourceBytes[start + 1]);
                const char c2 = DecodeByteToAscii(s_eventSourceBytes[start + 2]);
                if ((c0 == 'l' || c0 == 'L') && (c1 == 'e' || c1 == 'E') && (c2 == 't' || c2 == 'T')) {
                    return start;
                }
            }

            const char first = DecodeByteToAscii(s_eventSourceBytes[start]);
            if (len > 0 && len <= 4 && first >= 'A' && first <= 'Z') {
                return start;
            }

            return cur;
        };

        for (int pass = 0; pass < 2; pass++) {
            const size_t ni = BacktrackWordPrefix(i);
            if (ni == i) break;
            i = ni;
        }
    }
    size_t end = i;
    int asciiLetterCount = 0;
    int unmappedCount = 0;
    while (end < s_eventSourceSize && end - i < maxLen) {
        const uint8_t c = s_eventSourceBytes[end];
        if (c == 0) {
            break;
        }
        if (c == '\n') {
            end++;
            continue;
        }
        if (c < 0x20 || c == 0x7F) {
            return false;
        }
        const char a = DecodeByteToAscii(c);
        if (c >= 0x80 && a == '?') {
            unmappedCount++;
        }
        if ((a >= 'A' && a <= 'Z') || (a >= 'a' && a <= 'z')) {
            asciiLetterCount++;
        }
        end++;
    }

    if (end >= s_eventSourceSize) {
        return false;
    }
    if (end == i) {
        return false;
    }
    if (s_eventSourceBytes[end] != 0) {
        return false;
    }

    out.reserve(end - i);
    for (size_t p = i; p < end; p++) {
        const uint8_t c = s_eventSourceBytes[p];
        if (c == '\n') {
            out.push_back(' ');
        } else {
            out.push_back(DecodeByteToAscii(c));
        }
    }

    while (!out.empty() && out[0] == ' ') {
        out.erase(out.begin());
    }

    if (asciiLetterCount <= 0) {
        out.clear();
        return false;
    }

    const int len = (int)out.size();
    outScore = len * 3 + asciiLetterCount * 5 - unmappedCount * 120;
    return true;
}

static bool ReadSubtitleTextAtPtrAddress(uint32_t addr, uint32_t addressBase, std::string& out) {
    out.clear();
    if (!s_eventSourceBytes || s_eventSourceSize == 0) {
        return false;
    }
    if (addr < addressBase) {
        return false;
    }
    const uint32_t off = addr - addressBase;
    if (off >= s_eventSourceSize) {
        return false;
    }

    const size_t i = (size_t)off;
    size_t count = 0;
    while (i + count < s_eventSourceSize) {
        const uint8_t c = s_eventSourceBytes[i + count];
        if (c == 0) {
            break;
        }
        count++;
    }
    if (count == 0 || i + count > s_eventSourceSize) {
        return false;
    }

    out.reserve(count);
    for (size_t p = i; p < i + count; p++) {
        const uint8_t c = s_eventSourceBytes[p];
        if (c == 0x0A) {
            out.push_back('\n');
        } else {
            out.push_back(DecodeByteToAscii(c));
        }
    }

    while (!out.empty() && out.back() == ' ') {
        out.pop_back();
    }
    while (!out.empty() && out.front() == ' ') {
        out.erase(out.begin());
    }
    return !out.empty();
}

static void BuildSubtitleTextMapFromPtrTables(const std::vector<uint32_t>& neededLineIndices, uint32_t addressBase) {
    if (!s_eventSourceBytes || s_eventSourceSize == 0) {
        return;
    }

    uint32_t minIdx = 0xFFFFFFFFu;
    uint32_t maxIdx = 0;
    for (uint32_t idx : neededLineIndices) {
        if (idx < minIdx) minIdx = idx;
        if (idx > maxIdx) maxIdx = idx;
    }
    if (neededLineIndices.empty()) {
        minIdx = 0;
        maxIdx = 0;
    }

    std::vector<uint32_t> allPointsLocations;
    allPointsLocations.reserve(256);

    size_t scanBegin = 0x378;
    if (scanBegin < 3) scanBegin = 3;
    size_t scanEnd = s_eventSourceSize;
    if (scanEnd > 0x13A7C) scanEnd = 0x13A7C;
    if (scanEnd <= scanBegin) {
        scanBegin = 3;
        scanEnd = s_eventSourceSize;
    }

    uint32_t scanMarkers = 0;
    uint32_t scanCond1Pass = 0;
    uint32_t scanPoints = 0;
    uint32_t scanHit4 = 0;
    uint32_t scanHit12 = 0;

    for (size_t i = scanBegin; i + 13 < scanEnd; i++) {
        if (s_eventSourceBytes[i] != 0x80) {
            continue;
        }
        scanMarkers++;
        const bool cond1 = ((s_eventSourceBytes[i + 2] == 0 && s_eventSourceBytes[i + 3] == 0 && s_eventSourceBytes[i + 4] == 0 &&
                             s_eventSourceBytes[i + 7] == 0 && s_eventSourceBytes[i + 8] == 0) ||
                            (s_eventSourceBytes[i - 1] != 0 && s_eventSourceBytes[i - 2] != 0 && s_eventSourceBytes[i - 3] != 0));
        if (!cond1) {
            continue;
        }

        scanCond1Pass++;

        const bool hit12 = (i >= 12 && s_eventSourceBytes[i - 12] == 0x80) || (i + 12 < s_eventSourceSize && s_eventSourceBytes[i + 12] == 0x80);
        const bool hit4 = (i >= 4 && s_eventSourceBytes[i - 4] == 0x80) || (i + 4 < s_eventSourceSize && s_eventSourceBytes[i + 4] == 0x80);
        if (hit12 || hit4) {
            allPointsLocations.push_back((uint32_t)(i - 3));
            scanPoints++;
            if (hit4) scanHit4++;
            if (hit12) scanHit12++;
        }
    }

    Log::Printf("Sqevs1::PtrTableMap base=0x%X needed=%u idx=%u..%u scan=[0x%X..0x%X) markers=%u cond=%u points=%u hit4=%u hit12=%u",
                (unsigned)addressBase,
                (unsigned)neededLineIndices.size(),
                (unsigned)minIdx,
                (unsigned)maxIdx,
                (unsigned)scanBegin,
                (unsigned)scanEnd,
                (unsigned)scanMarkers,
                (unsigned)scanCond1Pass,
                (unsigned)scanPoints,
                (unsigned)scanHit4,
                (unsigned)scanHit12);

    struct TextGroup {
        std::vector<std::string> lines;
    };
    std::vector<TextGroup> groups;
    groups.reserve(16);
    TextGroup cur;

    uint32_t flushCount = 0;
    auto flush = [&]() {
        if (!cur.lines.empty()) {
            groups.push_back(std::move(cur));
            cur = TextGroup{};
            flushCount++;
        }
    };

    uint32_t pointSkip = 0;
    uint32_t readOk = 0;
    uint32_t readFail = 0;
    uint32_t kind4Count = 0;
    uint32_t kind12Count = 0;

    for (uint32_t point : allPointsLocations) {
        const size_t p = (size_t)point;
        if (p + 12 >= s_eventSourceSize) {
            pointSkip++;
            continue;
        }

        const uint32_t addr = (uint32_t)s_eventSourceBytes[p] | ((uint32_t)s_eventSourceBytes[p + 1] << 8) | ((uint32_t)s_eventSourceBytes[p + 2] << 16);

        std::string text;
        if (!ReadSubtitleTextAtPtrAddress(addr, addressBase, text)) {
            text.clear();
        }
        if (!text.empty()) {
            readOk++;
        } else {
            readFail++;
        }

        const size_t mark = p + 3;
        const bool kind4 = (mark + 4 < s_eventSourceSize && s_eventSourceBytes[mark + 4] == 0x80) || (mark >= 4 && s_eventSourceBytes[mark - 4] == 0x80);
        const bool kind12 = (mark + 12 < s_eventSourceSize && s_eventSourceBytes[mark + 12] == 0x80) || (mark >= 12 && s_eventSourceBytes[mark - 12] == 0x80);

        if (kind4) kind4Count++;
        if (kind12) kind12Count++;

        if (kind4) {
            cur.lines.push_back(std::move(text));
            const bool isEnd = (mark + 4 < s_eventSourceSize && s_eventSourceBytes[mark + 4] != 0x80) && (mark >= 4 && s_eventSourceBytes[mark - 4] == 0x80);
            if (isEnd) {
                flush();
            }
        } else if (kind12) {
            cur.lines.push_back(std::move(text));
            const bool isEnd = (mark + 12 < s_eventSourceSize && s_eventSourceBytes[mark + 12] != 0x80) && (mark >= 12 && s_eventSourceBytes[mark - 12] == 0x80);
            if (isEnd) {
                flush();
            }
        }
    }
    flush();

    size_t minGroupLines = (size_t)-1;
    size_t maxGroupLines = 0;
    size_t nonEmptyGroups = 0;
    for (const auto& g : groups) {
        if (g.lines.size() < minGroupLines) minGroupLines = g.lines.size();
        if (g.lines.size() > maxGroupLines) maxGroupLines = g.lines.size();
        bool hasText = false;
        for (const auto& t : g.lines) {
            if (!t.empty()) {
                hasText = true;
                break;
            }
        }
        if (hasText) nonEmptyGroups++;
    }
    if (minGroupLines == (size_t)-1) minGroupLines = 0;

    Log::Printf("Sqevs1::PtrTableMap base=0x%X groupBuild points=%u skip=%u groups=%u flush=%u kind4=%u kind12=%u readOk=%u readFail=%u groupLines=%u..%u nonEmptyGroups=%u",
                (unsigned)addressBase,
                (unsigned)allPointsLocations.size(),
                (unsigned)pointSkip,
                (unsigned)groups.size(),
                (unsigned)flushCount,
                (unsigned)kind4Count,
                (unsigned)kind12Count,
                (unsigned)readOk,
                (unsigned)readFail,
                (unsigned)minGroupLines,
                (unsigned)maxGroupLines,
                (unsigned)nonEmptyGroups);

    {
        int bestCoverageByLang[5] = { -1, -1, -1, -1, -1 };
        size_t bestTotalLenByLang[5] = { 0, 0, 0, 0, 0 };
        const TextGroup* bestGroupByLang[5] = { nullptr, nullptr, nullptr, nullptr, nullptr };
        int bestShiftByLang[5] = { 0, 0, 0, 0, 0 };
        uint32_t bestFirstMissingIdxByLang[5] = { 0, 0, 0, 0, 0 };

        int skipLang = 0;
        int skipCoverage = 0;
        int acceptGroups = 0;

        for (const auto& g : groups) {
            std::string sample;
            sample.reserve(512);
            for (const auto& t : g.lines) {
                if (t.empty()) {
                    continue;
                }
                if (!sample.empty()) {
                    sample.push_back(' ');
                }
                sample.append(t);
                if (sample.size() >= 420) {
                    break;
                }
            }

            const int lang = GuessSubtitleLanguageIndex(sample);
            if (lang < 0 || lang >= 5) {
                skipLang++;
                continue;
            }

            int bestCoverage = 0;
            size_t bestTotalLen = 0;
            int bestShift = 0;
            uint32_t bestFirstMissingIdx = 0;
            bool bestInit = false;
            int shiftMin = 0;
            int shiftMax = 0;
            if (!neededLineIndices.empty() && !g.lines.empty()) {
                shiftMin = -(int)maxIdx;
                shiftMax = (int)g.lines.size() - 1 - (int)minIdx;
            }

            for (int shift = shiftMin; shift <= shiftMax; shift++) {
                int coverage = 0;
                size_t totalLen = 0;
                uint32_t firstMissingIdx = 0xFFFFFFFFu;
                if (!neededLineIndices.empty()) {
                    for (uint32_t idx : neededLineIndices) {
                        const int64_t li = (int64_t)(uint64_t)idx + (int64_t)shift;
                        if (li >= 0 && (size_t)li < g.lines.size() && !g.lines[(size_t)li].empty()) {
                            coverage++;
                            totalLen += g.lines[(size_t)li].size();
                        } else if (firstMissingIdx == 0xFFFFFFFFu) {
                            firstMissingIdx = idx;
                        }
                    }
                    if (firstMissingIdx == 0xFFFFFFFFu) {
                        firstMissingIdx = 0xFFFFFFFFu;
                    }
                } else {
                    const int64_t start = (shift > 0) ? (int64_t)shift : 0;
                    for (int64_t i = start; i < (int64_t)g.lines.size(); i++) {
                        const auto& t = g.lines[(size_t)i];
                        if (!t.empty()) {
                            coverage++;
                            totalLen += t.size();
                        }
                    }
                    firstMissingIdx = 0xFFFFFFFFu;
                }

                if (!bestInit ||
                    coverage > bestCoverage ||
                    (coverage == bestCoverage && firstMissingIdx > bestFirstMissingIdx) ||
                    (coverage == bestCoverage && firstMissingIdx == bestFirstMissingIdx && totalLen > bestTotalLen)) {
                    bestInit = true;
                    bestCoverage = coverage;
                    bestTotalLen = totalLen;
                    bestShift = shift;
                    bestFirstMissingIdx = firstMissingIdx;
                }
            }

            if (bestCoverage < 10) {
                skipCoverage++;
                continue;
            }

            acceptGroups++;

            if (bestCoverage > bestCoverageByLang[lang] ||
                (bestCoverage == bestCoverageByLang[lang] && bestFirstMissingIdx > bestFirstMissingIdxByLang[lang]) ||
                (bestCoverage == bestCoverageByLang[lang] && bestFirstMissingIdx == bestFirstMissingIdxByLang[lang] && bestTotalLen > bestTotalLenByLang[lang])) {
                bestCoverageByLang[lang] = bestCoverage;
                bestTotalLenByLang[lang] = bestTotalLen;
                bestGroupByLang[lang] = &g;
                bestShiftByLang[lang] = bestShift;
                bestFirstMissingIdxByLang[lang] = bestFirstMissingIdx;
            }
        }

        Log::Printf("Sqevs1::PtrTableMap base=0x%X eval groups=%u accept=%d skipLang=%d skipCov=%d bestCov EN=%d FR=%d DE=%d ES=%d IT=%d",
                    (unsigned)addressBase,
                    (unsigned)groups.size(),
                    acceptGroups,
                    skipLang,
                    skipCoverage,
                    bestCoverageByLang[0],
                    bestCoverageByLang[1],
                    bestCoverageByLang[2],
                    bestCoverageByLang[3],
                    bestCoverageByLang[4]);

        for (int lang = 0; lang < 5; lang++) {
            const TextGroup* g = bestGroupByLang[lang];
            if (!g) {
                continue;
            }
            const int shift = bestShiftByLang[lang];
            for (uint32_t idx : neededLineIndices) {
                const int64_t li = (int64_t)(uint64_t)idx + (int64_t)shift;
                if (li < 0 || (size_t)li >= g->lines.size()) {
                    continue;
                }
                if (g->lines[(size_t)li].empty()) {
                    continue;
                }
                const uint32_t eventId = 1001u + idx;
                SubtitleTextSet& set = s_subtitleTextByEventId[eventId];
                if (set.texts[lang].empty()) {
                    set.texts[lang] = g->lines[(size_t)li];
                }
                if (s_subtitleLangByEventId.find(eventId) == s_subtitleLangByEventId.end()) {
                    s_subtitleLangByEventId[eventId] = lang;
                }
            }
        }

        for (const auto& g : groups) {
            std::string sample;
            sample.reserve(512);
            for (const auto& t : g.lines) {
                if (t.empty()) {
                    continue;
                }
                if (!sample.empty()) {
                    sample.push_back(' ');
                }
                sample.append(t);
                if (sample.size() >= 420) {
                    break;
                }
            }

            const int lang = GuessSubtitleLanguageIndex(sample);
            if (lang < 0 || lang >= 5) {
                continue;
            }

            const int refShift = bestShiftByLang[lang];

            int bestCoverage = 0;
            int bestMatchCount = -1;
            size_t bestTotalLen = 0;
            int bestShift = 0;
            int bestShiftDiff = 0;
            uint32_t bestFirstFilledIdx = 0;
            bool bestInit = false;
            int shiftMin = 0;
            int shiftMax = 0;
            if (!neededLineIndices.empty() && !g.lines.empty()) {
                shiftMin = -(int)maxIdx;
                shiftMax = (int)g.lines.size() - 1 - (int)minIdx;
            }
            for (int shift = shiftMin; shift <= shiftMax; shift++) {
                int coverage = 0;
                int matchCount = 0;
                size_t totalLen = 0;
                uint32_t firstFilledIdx = 0xFFFFFFFFu;
                for (uint32_t idx : neededLineIndices) {
                    const uint32_t eventId = 1001u + idx;
                    auto itSet = s_subtitleTextByEventId.find(eventId);
                    const int64_t li = (int64_t)(uint64_t)idx + (int64_t)shift;
                    if (li < 0 || (size_t)li >= g.lines.size() || g.lines[(size_t)li].empty()) {
                        continue;
                    }

                    // If already present, treat as consistency signal (prefer same group/shift).
                    if (itSet != s_subtitleTextByEventId.end() && !itSet->second.texts[lang].empty()) {
                        if (itSet->second.texts[lang] == g.lines[(size_t)li]) {
                            matchCount++;
                        }
                        continue;
                    }

                    // Missing line we could fill.
                    coverage++;
                    totalLen += g.lines[(size_t)li].size();
                    if (firstFilledIdx == 0xFFFFFFFFu) {
                        firstFilledIdx = idx;
                    }
                }

                if (firstFilledIdx == 0xFFFFFFFFu) {
                    firstFilledIdx = 0xFFFFFFFFu;
                }

                const int shiftDiff = (shift >= refShift) ? (shift - refShift) : (refShift - shift);

                if (!bestInit ||
                    matchCount > bestMatchCount ||
                    (matchCount == bestMatchCount && coverage > bestCoverage) ||
                    (matchCount == bestMatchCount && coverage == bestCoverage && shiftDiff < bestShiftDiff) ||
                    (matchCount == bestMatchCount && coverage == bestCoverage && shiftDiff == bestShiftDiff && firstFilledIdx < bestFirstFilledIdx) ||
                    (matchCount == bestMatchCount && coverage == bestCoverage && shiftDiff == bestShiftDiff && firstFilledIdx == bestFirstFilledIdx && totalLen > bestTotalLen)) {
                    bestInit = true;
                    bestCoverage = coverage;
                    bestMatchCount = matchCount;
                    bestTotalLen = totalLen;
                    bestShift = shift;
                    bestShiftDiff = shiftDiff;
                    bestFirstFilledIdx = firstFilledIdx;
                }
            }

            if (bestCoverage <= 0) {
                continue;
            }

            for (uint32_t idx : neededLineIndices) {
                const uint32_t eventId = 1001u + idx;
                auto itSet = s_subtitleTextByEventId.find(eventId);
                if (itSet != s_subtitleTextByEventId.end() && !itSet->second.texts[lang].empty()) {
                    continue;
                }

                const int64_t li = (int64_t)(uint64_t)idx + (int64_t)bestShift;
                if (li < 0 || (size_t)li >= g.lines.size()) {
                    continue;
                }
                if (g.lines[(size_t)li].empty()) {
                    continue;
                }

                SubtitleTextSet& set = s_subtitleTextByEventId[eventId];
                set.texts[lang] = g.lines[(size_t)li];
                if (s_subtitleLangByEventId.find(eventId) == s_subtitleLangByEventId.end()) {
                    s_subtitleLangByEventId[eventId] = lang;
                }
            }
        }

        unsigned filledByLang[5] = { 0, 0, 0, 0, 0 };
        for (const auto& kv : s_subtitleTextByEventId) {
            const SubtitleTextSet& set = kv.second;
            for (int lang = 0; lang < 5; lang++) {
                if (!set.texts[lang].empty()) {
                    filledByLang[lang]++;
                }
            }
        }
        Log::Printf("Sqevs1::PtrTableMap base=0x%X built size=%u filled EN=%u FR=%u DE=%u ES=%u IT=%u",
                    (unsigned)addressBase,
                    (unsigned)s_subtitleTextByEventId.size(),
                    filledByLang[0],
                    filledByLang[1],
                    filledByLang[2],
                    filledByLang[3],
                    filledByLang[4]);
    }
}

static void ApplyKnownStage1SubtitleOverrides() {
    if (!s_eventSourceBytes || s_eventSourceSize == 0) {
        return;
    }

    struct Override {
        uint32_t id;
        const char* needle;
    };

    const Override kOverrides[] = {
        { 1001, "Kick! Punch!" },
        { 1002, "If you wanna test me" },
        { 1003, "things I'll teach ya" },
        { 1004, "lesson from teacher" },
    };

    for (const auto& ov : kOverrides) {
        {
            auto it = s_subtitleTextByEventId.find(ov.id);
            if (it != s_subtitleTextByEventId.end()) {
                if (!it->second.texts[0].empty()) {
                    continue;
                }
            }
        }
        const char* begin = reinterpret_cast<const char*>(s_eventSourceBytes);
        const char* end = begin + s_eventSourceSize;
        const char* hit = std::search(begin, end, ov.needle, ov.needle + std::strlen(ov.needle));
        if (hit == end) {
            continue;
        }
        const uint32_t off = (uint32_t)(hit - begin);
        std::string text;
        int score = 0;
        if (TryReadSubtitleTextAtOffset(off, text, score) && !text.empty()) {
            SubtitleTextSet& set = s_subtitleTextByEventId[ov.id];
            set.texts[0] = text;
        }
    }
}

// 字幕文本映射（eventId -> 文本）
static const char* GetSubtitleText(uint32_t eventId) {
    {
        const auto it = s_subtitleTextByEventId.find(eventId);
        if (it != s_subtitleTextByEventId.end()) {
            const SubtitleTextSet& set = it->second;
            int lang = s_currentLanguageIndex;
            if (lang < 0 || lang >= 5) {
                lang = 0;
            }
            if (!set.texts[lang].empty()) {
                return set.texts[lang].c_str();
            }
            return nullptr;
        }
    }

    if (s_subtitleTextByEventId.empty()) {
        switch (eventId) {
            case 1001: return "Kick! Punch! It's all in the mind!";
            case 1002: return "If you wanna test me, I'm sure you'll find...";
            case 1003: return "The things I'll teach ya is sure to beat ya!";
            case 1004: return "Nevertheless you'll get a lesson from teacher!";
            default:
                break;
        }
    }

    if (eventId >= 1000 && eventId < 2000) {
        return nullptr;
    }
    return nullptr;
}

void SetEventSourceBytes(const uint8_t* data, size_t size) {
    s_eventSourceOwned.clear();
    if (data != nullptr && size > 0) {
        s_eventSourceOwned.assign(data, data + size);
        s_eventSourceBytes = s_eventSourceOwned.data();
        s_eventSourceSize = s_eventSourceOwned.size();
    } else {
        s_eventSourceBytes = nullptr;
        s_eventSourceSize = 0;
    }
    s_subtitleTextByEventId.clear();
    s_subtitleLangByEventId.clear();
}

// 当前活跃字幕
static SubtitleInfo s_activeSubtitle = { nullptr, 0, 0, 0 };
static uint32_t s_currentFrame = 0;
static uint32_t s_currentStageFrame = 0;
static uint32_t s_emitStageFrame = 0;
static uint32_t s_emitSubtitleDuration = 0;

struct ScriptEvent {
    uint32_t frame;
    uint32_t id;
};

static std::vector<ScriptEvent> s_eventTable;
static int s_nextEventIndex = 0;
static uint32_t s_fallbackMask = 0;
static int s_parserNextTextEventIndex = 0;
static uint32_t s_parserLastStageFrame = 0;
static uint8_t s_parserActiveStreamId = 1;
static ParserTextDebugInfo s_parserDebugInfo;
static bool s_latestResolvedSnapshotValid = false;
static uint32_t s_latestResolvedSnapshotQueryFrame = 0;
static PrStage1ResolvedTextEvent s_latestResolvedSnapshot;

static uint8_t SelectParserTextStreamId(const PrGameContext& ctx) {
    if (ctx.debugStage1TextStream >= 1 && ctx.debugStage1TextStream <= 8) {
        return static_cast<uint8_t>(ctx.debugStage1TextStream);
    }
    return 1;
}

static void ClearLatestResolvedSnapshot() {
    s_latestResolvedSnapshotValid = false;
    s_latestResolvedSnapshotQueryFrame = 0;
    s_latestResolvedSnapshot = PrStage1ResolvedTextEvent{};
}

static bool BuildLatestResolvedSnapshot(const PrGameContext& ctx,
                                        PrStage1ResolvedTextEvent& outResolved,
                                        uint32_t& outQueryFrame) {
    outResolved = PrStage1ResolvedTextEvent{};
    outQueryFrame = s_currentStageFrame;
    if (ctx.currentScene != PrSceneId::Scene1) {
        return false;
    }

    const auto data = ctx.stage1OverlayData;
    if (!data || !data->valid) {
        return false;
    }

    const uint8_t streamId = SelectParserTextStreamId(ctx);
    const PrStage1EventStream* stream = data->FindStream(streamId);
    if (!stream || stream->events.empty()) {
        return false;
    }

    const uint32_t queryFrame = s_currentStageFrame;

    const PrStage1ScriptEvent* ev = data->FindLatestEvent(streamId, queryFrame);
    if (!ev) {
        return false;
    }

    const uint8_t lang = (ctx.languageIndex >= 0 && ctx.languageIndex < 5) ? (uint8_t)ctx.languageIndex : 0u;
    const uint32_t dueFrame = ev->frame;
    const uint32_t elapsedFrames = (s_currentStageFrame > dueFrame) ? (s_currentStageFrame - dueFrame) : 0u;
    outQueryFrame = queryFrame;
    return data->ResolveTextEvent(streamId, *ev, lang, dueFrame, elapsedFrames, outResolved);
}

static void RefreshLatestResolvedSnapshot(const PrGameContext& ctx) {
    if (BuildLatestResolvedSnapshot(ctx, s_latestResolvedSnapshot, s_latestResolvedSnapshotQueryFrame) &&
        s_latestResolvedSnapshot.valid) {
        s_latestResolvedSnapshotValid = true;
        return;
    }
    ClearLatestResolvedSnapshot();
}

static bool TryPumpParsedStage1TextEvents(uint32_t stageFrame, PrGameContext& ctx) {
    if (ctx.currentScene != PrSceneId::Scene1) {
        return false;
    }
    const auto data = ctx.stage1OverlayData;
    if (!data || !data->valid) {
        return false;
    }
    const uint8_t streamId = SelectParserTextStreamId(ctx);
    const PrStage1EventStream* stream = data->FindStream(streamId);
    if (!stream) {
        return false;
    }

    if (stageFrame < s_parserLastStageFrame || streamId != s_parserActiveStreamId) {
        s_parserNextTextEventIndex = 0;
        s_activeSubtitle = { nullptr, 0, 0, 0 };
        s_parserActiveStreamId = streamId;
        s_parserDebugInfo = ParserTextDebugInfo{};
    }
    s_parserLastStageFrame = stageFrame;

    const uint8_t lang = (ctx.languageIndex >= 0 && ctx.languageIndex < 5) ? (uint8_t)ctx.languageIndex : 0u;
    auto resolveDueFrame = [&](const PrStage1ScriptEvent& ev) -> uint32_t {
        return ev.frame;
    };

    while (s_parserNextTextEventIndex >= 0 && s_parserNextTextEventIndex < (int)stream->events.size()) {
        const PrStage1ScriptEvent& ev0 = stream->events[(size_t)s_parserNextTextEventIndex];
        const uint32_t ev0DueFrame = resolveDueFrame(ev0);
        if (stageFrame < ev0DueFrame) {
            break;
        }

        const uint32_t curFrame = ev0.frame;
        const uint32_t curDueFrame = ev0DueFrame;
        const int startIndex = s_parserNextTextEventIndex;
        int endIndex = startIndex;
        while (endIndex < (int)stream->events.size() && stream->events[(size_t)endIndex].frame == curFrame) {
            endIndex++;
        }

        int selectedIndex = -1;
        const std::string* selectedText = nullptr;
        for (int i = startIndex; i < endIndex; i++) {
            const PrStage1ScriptEvent& ev = stream->events[(size_t)i];
            if (ev.textId == 0) {
                continue;
            }
            const std::string* text = data->ResolveText(lang, ev.textId);
            if (text && !text->empty()) {
                selectedIndex = i;
                selectedText = text;
                break;
            }
        }

        if (selectedIndex >= 0 && selectedText != nullptr) {
            uint32_t nextTextFrame = 0;
            for (int j = endIndex; j < (int)stream->events.size(); j++) {
                const PrStage1ScriptEvent& nx = stream->events[(size_t)j];
                if (nx.textId == 0) {
                    continue;
                }
                const std::string* nextText = data->ResolveText(lang, nx.textId);
                if (nextText && !nextText->empty()) {
                    nextTextFrame = resolveDueFrame(nx);
                    break;
                }
            }

            uint32_t dur = 90;
            if (nextTextFrame > curDueFrame) {
                dur = nextTextFrame - curDueFrame;
            }
            if (dur < 1u) dur = 1u;
            if (dur > 900u) dur = 900u;

            s_activeSubtitle.text = selectedText->c_str();
            s_activeSubtitle.startFrame = curDueFrame;
            s_activeSubtitle.durationFrames = dur;
            s_activeSubtitle.eventId = stream->events[(size_t)selectedIndex].psxAddr;
            s_parserDebugInfo.valid = true;
            s_parserDebugInfo.streamId = streamId;
            s_parserDebugInfo.eventPsxAddr = stream->events[(size_t)selectedIndex].psxAddr;
            s_parserDebugInfo.eventFrame = curFrame;
            s_parserDebugInfo.dueFrame = curDueFrame;
            s_parserDebugInfo.textId = stream->events[(size_t)selectedIndex].textId;
            s_parserDebugInfo.slot0 = stream->events[(size_t)selectedIndex].hudSlot0;
            s_parserDebugInfo.slot1 = stream->events[(size_t)selectedIndex].hudSlotByMode;
            s_parserDebugInfo.slot2 = stream->events[(size_t)selectedIndex].hudSlot2;

            ctx.lastSqevsEventId = stream->events[(size_t)selectedIndex].psxAddr;
            ctx.lastSqevsEventFrame = ctx.frame;

            Log::Printf("Sqevs1: parser subtitle stream=%u frame=%u due=%u textId=%u text='%s'",
                        (unsigned)streamId,
                        (unsigned)curFrame,
                        (unsigned)curDueFrame,
                        (unsigned)stream->events[(size_t)selectedIndex].textId,
                        selectedText->c_str());
        }

        s_parserNextTextEventIndex = endIndex;
    }

    return true;
}

void Init(PrGameContext& ctx) {
    (void)ctx;
    if (s_inited) return;
    s_inited = true;
    s_activeSubtitle = { nullptr, 0, 0, 0 };
    s_currentFrame = 0;
    s_currentStageFrame = 0;
    s_nextEventIndex = 0;
    s_fallbackMask = 0;
    s_parserNextTextEventIndex = 0;
    s_parserLastStageFrame = 0;
    s_parserActiveStreamId = 1;
    s_parserDebugInfo = ParserTextDebugInfo{};
    ClearLatestResolvedSnapshot();
    Log::Printf("Sqevs1::Init eventCount=%d", (int)s_eventTable.size());
}

void Shutdown(PrGameContext& ctx) {
    (void)ctx;
    if (!s_inited) return;
    s_inited = false;
    s_activeSubtitle = { nullptr, 0, 0, 0 };
    s_currentStageFrame = 0;
    s_nextEventIndex = 0;
    s_fallbackMask = 0;
    s_parserNextTextEventIndex = 0;
    s_parserLastStageFrame = 0;
    s_parserActiveStreamId = 1;
    s_parserDebugInfo = ParserTextDebugInfo{};
    ClearLatestResolvedSnapshot();
    Log::Printf("Sqevs1::Shutdown");
}

void Update(PrGameContext& ctx) {
    s_currentLanguageIndex = (int)ctx.languageIndex;
    s_currentFrame = ctx.frame;
}

void Emit(uint32_t id, uint32_t a0, uint32_t a1, uint32_t a2) {
    int guessLang = -1;
    {
        auto itLang = s_subtitleLangByEventId.find(id);
        if (itLang != s_subtitleLangByEventId.end()) {
            guessLang = itLang->second;
        }
    }
    Log::Printf("Sqevs1::Emit id=%u a0=%u a1=%u a2=%u frame=%u lang=%d guessLang=%d",
                (unsigned)id, (unsigned)a0, (unsigned)a1, (unsigned)a2, s_currentFrame,
                (int)s_currentLanguageIndex, (int)guessLang);
    
    // 触发字幕
    const char* text = GetSubtitleText(id);
    if (text != nullptr) {
        s_activeSubtitle.text = text;
        s_activeSubtitle.startFrame = s_emitStageFrame;
        s_activeSubtitle.durationFrames = (s_emitSubtitleDuration != 0) ? s_emitSubtitleDuration : 90;
        s_activeSubtitle.eventId = id;
        Log::Printf("Sqevs1: Subtitle activated: '%s'", text);
    }
}

const SubtitleInfo* GetActiveSubtitle() {
    if (s_activeSubtitle.text != nullptr) {
        return &s_activeSubtitle;
    }
    return nullptr;
}

bool HasActiveSubtitle() {
    return s_activeSubtitle.text != nullptr;
}

bool ResolveActiveParserEvent(const PrGameContext& ctx, PrStage1ResolvedTextEvent& out) {
    out = PrStage1ResolvedTextEvent{};
    if (ctx.currentScene != PrSceneId::Scene1) {
        return false;
    }
    const auto data = ctx.stage1OverlayData;
    if (!data || !data->valid) {
        return false;
    }
    const uint8_t lang = (ctx.languageIndex >= 0 && ctx.languageIndex < 5) ? (uint8_t)ctx.languageIndex : 0u;
    return data->ResolveParserTextDebugInfo(s_parserDebugInfo, lang, s_currentStageFrame, out);
}

bool GetLatestResolvedSnapshot(PrStage1ResolvedTextEvent& out, uint32_t& outQueryFrame) {
    out = PrStage1ResolvedTextEvent{};
    outQueryFrame = s_latestResolvedSnapshotQueryFrame;
    if (!s_latestResolvedSnapshotValid) {
        return false;
    }
    out = s_latestResolvedSnapshot;
    return out.valid;
}

bool ResolveLatestParserEvent(const PrGameContext& ctx, PrStage1ResolvedTextEvent& out) {
    (void)ctx;
    uint32_t queryFrame = 0;
    return GetLatestResolvedSnapshot(out, queryFrame);
}

bool LoadEventTable(const uint8_t* data, size_t size) {
    s_eventTable.clear();
    s_nextEventIndex = 0;
    s_fallbackMask = 0;
    s_subtitleTextByEventId.clear();
    s_subtitleLangByEventId.clear();
    std::unordered_map<size_t, std::array<int, 5>> votesByPtrOffset;
    struct SubtitleCandCache {
        uint32_t id;
        struct TextCand {
            size_t off;
            std::string text;
            int score;
            int guess;
        };
        std::vector<TextCand> cands;
    };
    std::vector<SubtitleCandCache> subtitleCandCache;
    
    if (data == nullptr || size < 0x40) {
        return false;
    }

    auto readU16 = [](const uint8_t* p) -> uint16_t {
        return (uint16_t)(p[0] | (p[1] << 8));
    };
    auto readU32 = [](const uint8_t* p) -> uint32_t {
        return (uint32_t)(p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24));
    };

    struct IdField {
        int score = 0;
        size_t offset = 0;
        int size = 0;   // 1/2
        bool directId = false;
    };

    struct Candidate {
        int score = -1;
        size_t baseOffset = 0;
        size_t stride = 0;
        size_t frameOffset = 0;
        int frameSize = 0; // 2/4
        size_t idOffset = 0;
        int idSize = 0; // 1/2
        bool idDirect = false;
        int count = 0;
        int earlyEmitCount = 0;
        uint32_t firstEmitFrame = 0;
        uint32_t firstFrame = 0;
        uint32_t lastFrame = 0;
    };

    constexpr int kMinCount = 40;
    constexpr int kMaxScanCount = 220;
    constexpr int kMaxIdSampleCount = 96;

    constexpr size_t kMaxBaseScanPhase1 = 0x400;
    constexpr size_t kMaxBaseScanPhase2 = 0x10000;

    constexpr size_t kStrideCandidates[] = {
        0x34, 0x30, 0x38, 0x2C, 0x28, 0x24, 0x20, 0x40, 0x44, 0x48, 0x50, 0x1C, 0x18, 0x14, 0x10
    };
    constexpr size_t kFrameOffCandidates16[] = { 0x00, 0x02, 0x04, 0x06, 0x08, 0x0C, 0x10 };
    constexpr size_t kFrameOffCandidates32[] = { 0x00, 0x04, 0x08, 0x0C, 0x10 };

    Candidate best;

    auto scanWithBaseLimit = [&](size_t maxBase) {
        for (size_t stride : kStrideCandidates) {
            if (stride < 6) continue;
            if (stride * (size_t)kMinCount > size) continue;

            for (int frameSize : {2, 4}) {
                if (stride < (size_t)frameSize) continue;

                const size_t* frameOffList = (frameSize == 2) ? kFrameOffCandidates16 : kFrameOffCandidates32;
                const size_t frameOffCount = (frameSize == 2)
                    ? (sizeof(kFrameOffCandidates16) / sizeof(kFrameOffCandidates16[0]))
                    : (sizeof(kFrameOffCandidates32) / sizeof(kFrameOffCandidates32[0]));

                for (size_t k = 0; k < frameOffCount; k++) {
                    const size_t frameOff = frameOffList[k];
                    if (frameOff + (size_t)frameSize > stride) continue;

                    const size_t maxBaseClamped = (maxBase < size) ? maxBase : size;
                    for (size_t baseOff = 0; baseOff + stride * (size_t)kMinCount <= size && baseOff < maxBaseClamped; baseOff += 4) {
                        // 快速前置过滤：前 8 条 frame 必须合理且单调
                        {
                            uint32_t pf = 0;
                            bool ok = true;
                            for (int i = 0; i < 8; i++) {
                                const size_t pos = baseOff + (size_t)i * stride + frameOff;
                                if (pos + (size_t)frameSize > size) { ok = false; break; }
                                uint32_t f = 0;
                                if (frameSize == 2) f = (uint32_t)readU16(data + pos);
                                else f = readU32(data + pos);
                                if (f > 20000) { ok = false; break; }
                                if (i > 0 && f < pf) { ok = false; break; }
                                pf = f;
                            }
                            if (!ok) continue;
                        }

                        int validCount = 0;
                        int incCount = 0;
                        int zeroCount = 0;
                        uint32_t firstFrame = 0;
                        uint32_t prevFrame = 0;
                        uint32_t lastFrame = 0;

                        for (int i = 0; i < kMaxScanCount; i++) {
                            const size_t pos = baseOff + (size_t)i * stride + frameOff;
                            if (pos + (size_t)frameSize > size) break;

                            uint32_t frame = 0;
                            if (frameSize == 2) {
                                frame = (uint32_t)readU16(data + pos);
                            } else {
                                frame = readU32(data + pos);
                            }

                            if (frame > 20000) break;
                            if (validCount > 0 && frame < prevFrame) break;

                            if (frame == 0) {
                                zeroCount++;
                            }

                            if (validCount == 0) {
                                firstFrame = frame;
                            } else if (frame > prevFrame) {
                                incCount++;
                            }
                            prevFrame = frame;
                            lastFrame = frame;
                            validCount++;
                        }

                        if (validCount < kMinCount) continue;
                        if (lastFrame == 0 || incCount < 8) continue;
                        if (lastFrame <= firstFrame) continue;

                        // 识别 id 字段：只在结构尾部附近搜索，并只采样前若干条用于打分
                        const int sampleCount = (validCount < kMaxIdSampleCount) ? validCount : kMaxIdSampleCount;
                        const size_t idStart = (stride > 0x20) ? (stride - 0x20) : 0;
                        const size_t idEnd = stride;

                        IdField bestId;

                        if (idEnd >= 2) {
                            for (size_t idOff = idStart & ~1u; idOff + 2 <= idEnd; idOff += 2) {
                                int hitDirect = 0;
                                int hitSmall = 0;
                                int uniqDirect = 0;
                                int uniqSmall = 0;
                                bool seenDirect[256] = { false };
                                bool seenSmall[64] = { false };

                                for (int i = 0; i < sampleCount; i++) {
                                    const uint8_t* entry = data + baseOff + (size_t)i * stride;
                                    const uint16_t v = readU16(entry + idOff);
                                    if (v >= 1000 && v < 2000) {
                                        hitDirect++;
                                        const uint16_t idx = (uint16_t)(v - 1000);
                                        if (idx < 256 && !seenDirect[idx]) {
                                            seenDirect[idx] = true;
                                            uniqDirect++;
                                        }
                                    }
                                    if (v >= 1 && v <= 50) {
                                        hitSmall++;
                                        if (v < 64 && !seenSmall[v]) {
                                            seenSmall[v] = true;
                                            uniqSmall++;
                                        }
                                    }
                                }

                                if (hitDirect >= 2) {
                                    const int score = hitDirect * 60 + uniqDirect * 120 + hitSmall * 2 + uniqSmall * 10;
                                    if (score > bestId.score) {
                                        bestId.score = score;
                                        bestId.offset = idOff;
                                        bestId.size = 2;
                                        bestId.directId = true;
                                    }
                                } else if (hitSmall >= 8 && uniqSmall >= 2) {
                                    const int score = hitSmall * 4 + uniqSmall * 40;
                                    if (score > bestId.score) {
                                        bestId.score = score;
                                        bestId.offset = idOff;
                                        bestId.size = 2;
                                        bestId.directId = false;
                                    }
                                }
                            }
                        }

                        for (size_t idOff = idStart; idOff < idEnd; idOff += 1) {
                            int hitSmall = 0;
                            int hitTiny = 0;
                            int uniq = 0;
                            bool seen[64] = { false };

                            for (int i = 0; i < sampleCount; i++) {
                                const uint8_t* entry = data + baseOff + (size_t)i * stride;
                                const uint8_t v = entry[idOff];
                                if (v >= 1 && v <= 50) {
                                    hitSmall++;
                                    if (v <= 10) hitTiny++;
                                    if (v < 64 && !seen[v]) {
                                        seen[v] = true;
                                        uniq++;
                                    }
                                }
                            }

                            if (hitSmall >= 8 && uniq >= 2) {
                                const int score = hitSmall * 4 + hitTiny * 12 + uniq * 40;
                                if (score > bestId.score) {
                                    bestId.score = score;
                                    bestId.offset = idOff;
                                    bestId.size = 1;
                                    bestId.directId = false;
                                }
                            }
                        }

                        if (bestId.score <= 0) continue;

                        // 早期事件约束：必须在前 600 帧内至少有一个可发事件，避免误命中“晚开始表”导致 fallback 被关闭
                        int earlyEmit = 0;
                        uint32_t firstEmit = 0xFFFFFFFFu;
                        for (int i = 0; i < sampleCount; i++) {
                            uint32_t frame = 0;
                            const uint8_t* entry = data + baseOff + (size_t)i * stride;
                            if (frameSize == 2) {
                                frame = (uint32_t)readU16(entry + frameOff);
                            } else {
                                frame = readU32(entry + frameOff);
                            }

                            uint32_t id = 0;
                            if (bestId.size == 1) {
                                const uint8_t v = entry[bestId.offset];
                                if (v != 0) id = 1000u + (uint32_t)v;
                            } else {
                                const uint16_t v = readU16(entry + bestId.offset);
                                if (v != 0) id = bestId.directId ? (uint32_t)v : (1000u + (uint32_t)v);
                            }

                            if (id >= 1000 && id < 10000) {
                                if (frame < firstEmit) firstEmit = frame;
                                if (frame <= 600) earlyEmit++;
                            }
                        }

                        if (earlyEmit <= 0) {
                            continue;
                        }

                        const int frameScore = validCount * 10 + incCount * 5;
                        const uint32_t cappedLastFrame = (lastFrame < 6000u) ? lastFrame : 6000u;
                        const int spanScore = (int)(cappedLastFrame / 2u);
                        const int smallRangePenalty = (lastFrame < 800) ? 25000 : 0;
                        const int zeroPenalty = (zeroCount >= 8) ? (zeroCount * 800) : 0;
                        const int cappedEarlyEmit = (earlyEmit < 4) ? earlyEmit : 4;
                        const int earlyEmitBonus = 500 + cappedEarlyEmit * 50;
                        const int totalScore = frameScore + spanScore - smallRangePenalty - zeroPenalty + bestId.score + earlyEmitBonus;
                        if (totalScore > best.score) {
                            best.score = totalScore;
                            best.baseOffset = baseOff;
                            best.stride = stride;
                            best.frameOffset = frameOff;
                            best.frameSize = frameSize;
                            best.idOffset = bestId.offset;
                            best.idSize = bestId.size;
                            best.idDirect = bestId.directId;
                            best.count = validCount;
                            best.earlyEmitCount = earlyEmit;
                            best.firstEmitFrame = (firstEmit == 0xFFFFFFFFu) ? 0 : firstEmit;
                            best.firstFrame = firstFrame;
                            best.lastFrame = lastFrame;
                        }
                    }
                }
            }
        }
    };

    scanWithBaseLimit(kMaxBaseScanPhase1);
    scanWithBaseLimit(kMaxBaseScanPhase2);

    if (best.count < kMinCount || best.score <= 0) {
        return false;
    }

    // 对 1-byte id 字段做一次稳健校验：若检测到明显“偏移错位”（常见是误命中 id@0x4），
    // 自动回退到更可信的 id@0x1，避免字幕内容整体前移/错位。
    if (best.idSize == 1 && best.stride > 1 && best.idOffset != 1) {
        const int sampleCount = (best.count < kMaxIdSampleCount) ? best.count : kMaxIdSampleCount;
        if (sampleCount > 0) {
            auto evalByteIdField = [&](size_t idOff,
                                       int& outScore,
                                       int& outEarlyEmit,
                                       uint32_t& outMinId,
                                       uint32_t& outMaxId) {
                outScore = 0;
                outEarlyEmit = 0;
                outMinId = 0xFFFFFFFFu;
                outMaxId = 0;
                if (idOff >= best.stride) {
                    outMinId = 0;
                    return;
                }

                int hitSmall = 0;
                int hitTiny = 0;
                int uniq = 0;
                bool seen[64] = { false };

                for (int i = 0; i < sampleCount; i++) {
                    const uint8_t* entry = data + best.baseOffset + (size_t)i * best.stride;
                    const uint8_t v = entry[idOff];

                    if (v >= 1 && v <= 50) {
                        hitSmall++;
                        if (v <= 10) {
                            hitTiny++;
                        }
                        if (v < 64 && !seen[v]) {
                            seen[v] = true;
                            uniq++;
                        }
                    }

                    if (v != 0) {
                        const uint32_t id = 1000u + (uint32_t)v;
                        if (id < outMinId) {
                            outMinId = id;
                        }
                        if (id > outMaxId) {
                            outMaxId = id;
                        }

                        uint32_t frame = 0;
                        if (best.frameSize == 2) {
                            frame = (uint32_t)readU16(entry + best.frameOffset);
                        } else {
                            frame = readU32(entry + best.frameOffset);
                        }
                        if (frame <= 600u) {
                            outEarlyEmit++;
                        }
                    }
                }

                if (hitSmall >= 8 && uniq >= 2) {
                    outScore = hitSmall * 4 + hitTiny * 12 + uniq * 40;
                }

                if (outMinId == 0xFFFFFFFFu) {
                    outMinId = 0;
                }
            };

            int curScore = 0;
            int curEarlyEmit = 0;
            uint32_t curMinId = 0;
            uint32_t curMaxId = 0;
            evalByteIdField(best.idOffset, curScore, curEarlyEmit, curMinId, curMaxId);

            int off1Score = 0;
            int off1EarlyEmit = 0;
            uint32_t off1MinId = 0;
            uint32_t off1MaxId = 0;
            evalByteIdField(1, off1Score, off1EarlyEmit, off1MinId, off1MaxId);

            const bool off1BetterByScore = (off1Score > curScore + 120);
            const bool off1BetterByMinId = (off1MinId != 0 && curMinId != 0 && off1MinId <= 1005u && curMinId > 1010u);
            const bool off1BetterByMaxId = (off1MaxId != 0 && curMaxId != 0 && off1MaxId <= 1100u && curMaxId > 1200u);

            if (off1Score > 0 && off1EarlyEmit > 0 && (off1BetterByScore || off1BetterByMinId || off1BetterByMaxId)) {
                Log::Printf("Sqevs1::LoadEventTable: id field override 0x%X->0x1 cur(score=%d early=%d id=%u..%u) alt(score=%d early=%d id=%u..%u)",
                            (unsigned)best.idOffset,
                            curScore,
                            curEarlyEmit,
                            (unsigned)curMinId,
                            (unsigned)curMaxId,
                            off1Score,
                            off1EarlyEmit,
                            (unsigned)off1MinId,
                            (unsigned)off1MaxId);
                best.idOffset = 1;
                best.idSize = 1;
                best.idDirect = false;
            }
        }
    }

    // 解析事件表
    for (int i = 0; i < best.count; i++) {
        const uint8_t* entry = data + best.baseOffset + (size_t)i * best.stride;

        uint32_t frame = 0;
        if (best.frameSize == 2) {
            frame = (uint32_t)readU16(entry + best.frameOffset);
        } else {
            frame = readU32(entry + best.frameOffset);
        }

        uint32_t id = 0;
        if (best.idSize == 1) {
            const uint8_t v = entry[best.idOffset];
            if (v == 0) continue;
            id = 1000u + (uint32_t)v;
        } else if (best.idSize == 2) {
            const uint16_t v = readU16(entry + best.idOffset);
            if (v == 0) continue;
            if (best.idDirect) {
                id = (uint32_t)v;
            } else {
                id = 1000u + (uint32_t)v;
            }
        }

        if (id < 1000 || id >= 10000) {
            continue;
        }

        ScriptEvent ev;
        ev.frame = frame;
        ev.id = id;
        s_eventTable.push_back(ev);

        if (s_eventSourceBytes && s_eventSourceSize > 0 && id >= 1000 && id < 2000) {
            SubtitleCandCache cache;
            cache.id = id;
            cache.cands.reserve(8);

            for (size_t off = 0; off + 4 <= best.stride; off += 4) {
                const uint32_t ptr = readU32(entry + off);
                if (ptr == 0) {
                    continue;
                }
                std::string tmp;
                int sc = 0;
                if (!TryReadSubtitleTextAtOffset(ptr, tmp, sc)) {
                    continue;
                }
                if (sc <= 0 || tmp.empty()) {
                    continue;
                }
                SubtitleCandCache::TextCand tc;
                tc.off = off;
                tc.text = std::move(tmp);
                tc.score = sc;
                tc.guess = GuessSubtitleLanguageIndex(tc.text);
                cache.cands.push_back(std::move(tc));
            }

            if (!cache.cands.empty()) {
                for (const auto& tc : cache.cands) {
                    if (tc.guess >= 0 && tc.guess < 5) {
                        auto& v = votesByPtrOffset[tc.off];
                        v[(size_t)tc.guess]++;
                    }
                }

                std::sort(cache.cands.begin(), cache.cands.end(), [](const SubtitleCandCache::TextCand& a, const SubtitleCandCache::TextCand& b) {
                    if (a.off != b.off) return a.off < b.off;
                    return a.score > b.score;
                });

                SubtitleTextSet& set = s_subtitleTextByEventId[id];
                for (size_t i = 0; i < cache.cands.size(); i++) {
                    // Skip duplicates
                    bool dup = false;
                    for (int j = 0; j < 5; j++) {
                        if (!set.texts[j].empty() && set.texts[j] == cache.cands[i].text) {
                            dup = true;
                            break;
                        }
                    }
                    if (dup) {
                        continue;
                    }

                    const int guess = cache.cands[i].guess;
                    if (guess >= 0 && guess < 5) {
                        if (set.texts[guess].empty()) {
                            set.texts[guess] = cache.cands[i].text;
                            if (s_subtitleLangByEventId.find(id) == s_subtitleLangByEventId.end()) {
                                s_subtitleLangByEventId[id] = guess;
                            }
                            continue;
                        }
                    }
                }

                subtitleCandCache.push_back(std::move(cache));
            }
        }
    }

    // Second pass: for ambiguous/short strings where GuessSubtitleLanguageIndex failed,
    // classify by pointer-field offset's global language votes.
    std::unordered_map<size_t, int> langByPtrOffset;
    langByPtrOffset.reserve(votesByPtrOffset.size());
    for (const auto& kv : votesByPtrOffset) {
        const size_t off = kv.first;
        const auto& v = kv.second;
        int bestLang = -1;
        int best = 0;
        int second = 0;
        for (int i = 0; i < 5; i++) {
            const int c = v[(size_t)i];
            if (c > best) {
                second = best;
                best = c;
                bestLang = i;
            } else if (c > second) {
                second = c;
            }
        }

        // Need some confidence: at least 3 votes and not tied.
        if (bestLang >= 0 && best >= 3 && best > second) {
            langByPtrOffset[off] = bestLang;
        }
    }

    if (!langByPtrOffset.empty()) {
        for (const auto& ce : subtitleCandCache) {
            auto itSet = s_subtitleTextByEventId.find(ce.id);
            if (itSet == s_subtitleTextByEventId.end()) {
                continue;
            }
            SubtitleTextSet& set = itSet->second;

            for (const auto& tc : ce.cands) {
                int lang = tc.guess;
                if (lang < 0 || lang >= 5) {
                    auto itLang = langByPtrOffset.find(tc.off);
                    if (itLang != langByPtrOffset.end()) {
                        lang = itLang->second;
                    }
                }
                if (lang < 0 || lang >= 5) {
                    continue;
                }
                if (set.texts[lang].empty()) {
                    set.texts[lang] = tc.text;
                }
            }
        }
    }

    if (s_eventTable.size() < (size_t)kMinCount) {
        s_eventTable.clear();
        s_nextEventIndex = 0;
        return false;
    }

    std::sort(s_eventTable.begin(), s_eventTable.end(), [](const ScriptEvent& a, const ScriptEvent& b) {
        if (a.frame != b.frame) return a.frame < b.frame;
        return a.id < b.id;
    });

    s_eventTable.erase(
        std::unique(s_eventTable.begin(), s_eventTable.end(), [](const ScriptEvent& a, const ScriptEvent& b) {
            return a.frame == b.frame && a.id == b.id;
        }),
        s_eventTable.end());

    {
        std::vector<uint32_t> neededLineIndices;
        neededLineIndices.reserve(256);
        for (const ScriptEvent& ev : s_eventTable) {
            if (ev.id >= 1001u && ev.id < 2000u) {
                neededLineIndices.push_back(ev.id - 1001u);
            }
        }
        std::sort(neededLineIndices.begin(), neededLineIndices.end());
        neededLineIndices.erase(std::unique(neededLineIndices.begin(), neededLineIndices.end()), neededLineIndices.end());

        auto heuristicText = std::move(s_subtitleTextByEventId);
        auto heuristicLang = std::move(s_subtitleLangByEventId);
        std::unordered_map<uint32_t, SubtitleTextSet> bestText;
        std::unordered_map<uint32_t, int> bestLang;
        uint32_t bestBase = 0;
        int bestScore = -1;

        const uint32_t kBases[] = {
            0x1C3870u, // PTR USA
            0x1CC21Cu, // PTR JP
            0x1C8AF8u, // PTR EUR
        };

        for (uint32_t base : kBases) {
            s_subtitleTextByEventId.clear();
            s_subtitleLangByEventId.clear();
            BuildSubtitleTextMapFromPtrTables(neededLineIndices, base);
            if (s_subtitleTextByEventId.empty()) {
                continue;
            }

            int coverageTotal = 0;
            size_t totalLen = 0;
            for (uint32_t idx : neededLineIndices) {
                const uint32_t eventId = 1001u + idx;
                auto it = s_subtitleTextByEventId.find(eventId);
                if (it == s_subtitleTextByEventId.end()) {
                    continue;
                }
                const SubtitleTextSet& set = it->second;
                for (int lang = 0; lang < 5; lang++) {
                    if (!set.texts[lang].empty()) {
                        coverageTotal++;
                        totalLen += set.texts[lang].size();
                    }
                }
            }
            const int score = coverageTotal * 100000 + (int)(totalLen > (size_t)0x7FFFFFFF ? (size_t)0x7FFFFFFF : totalLen);
            if (score > bestScore) {
                bestScore = score;
                bestBase = base;
                bestText = std::move(s_subtitleTextByEventId);
                bestLang = std::move(s_subtitleLangByEventId);
            }
        }

        if (bestScore > 0 && !bestText.empty()) {
            s_subtitleTextByEventId = std::move(bestText);
            s_subtitleLangByEventId = std::move(bestLang);

            for (uint32_t idx : neededLineIndices) {
                const uint32_t eventId = 1001u + idx;

                auto itHeu = heuristicText.find(eventId);
                if (itHeu == heuristicText.end()) {
                    continue;
                }

                auto itCur = s_subtitleTextByEventId.find(eventId);
                if (itCur == s_subtitleTextByEventId.end()) {
                    s_subtitleTextByEventId[eventId] = itHeu->second;
                } else {
                    SubtitleTextSet& curSet = itCur->second;
                    const SubtitleTextSet& heuSet = itHeu->second;
                    for (int lang = 0; lang < 5; lang++) {
                        if (curSet.texts[lang].empty() && !heuSet.texts[lang].empty()) {
                            curSet.texts[lang] = heuSet.texts[lang];
                        }
                    }

                    if (curSet.texts[0].empty() && !heuSet.texts[0].empty()) {
                        curSet.texts[0] = heuSet.texts[0];
                    }
                }

                if (s_subtitleLangByEventId.find(eventId) == s_subtitleLangByEventId.end()) {
                    auto itHL = heuristicLang.find(eventId);
                    if (itHL != heuristicLang.end()) {
                        s_subtitleLangByEventId[eventId] = itHL->second;
                    }
                }
            }

            unsigned filledByLang[5] = { 0, 0, 0, 0, 0 };
            for (const auto& kv : s_subtitleTextByEventId) {
                const SubtitleTextSet& set = kv.second;
                for (int lang = 0; lang < 5; lang++) {
                    if (!set.texts[lang].empty()) {
                        filledByLang[lang]++;
                    }
                }
            }
            Log::Printf("Sqevs1::LoadEventTable: PtrTable subtitle map selected base=0x%X size=%u filled EN=%u FR=%u DE=%u ES=%u IT=%u",
                        (unsigned)bestBase,
                        (unsigned)s_subtitleTextByEventId.size(),
                        filledByLang[0], filledByLang[1], filledByLang[2], filledByLang[3], filledByLang[4]);
        } else {
            s_subtitleTextByEventId = std::move(heuristicText);
            s_subtitleLangByEventId = std::move(heuristicLang);
            Log::Printf("Sqevs1::LoadEventTable: PtrTable subtitle map empty for all bases, keep heuristic map size=%u",
                        (unsigned)s_subtitleTextByEventId.size());
        }
    }

    ApplyKnownStage1SubtitleOverrides();

    Log::Printf("Sqevs1::LoadEventTable: Loaded %d events base=0x%X stride=0x%X frame@0x%X(%d) id@0x%X(%d) direct=%d range=%u..%u",
                (int)s_eventTable.size(),
                (unsigned)best.baseOffset,
                (unsigned)best.stride,
                (unsigned)best.frameOffset,
                best.frameSize,
                (unsigned)best.idOffset,
                best.idSize,
                best.idDirect ? 1 : 0,
                (unsigned)best.firstFrame,
                (unsigned)best.lastFrame);

    if (!s_subtitleTextByEventId.empty()) {
        Log::Printf("Sqevs1::LoadEventTable: SubtitleTextMap size=%u",
                    (unsigned)s_subtitleTextByEventId.size());
    }

    for (int i = 0; i < 5 && i < (int)s_eventTable.size(); i++) {
        const ScriptEvent& ev = s_eventTable[i];
        Log::Printf("  Event[%d]: frame=%u id=%u", i, (unsigned)ev.frame, (unsigned)ev.id);
    }

    return true;
}

int GetEventCount() {
    return (int)s_eventTable.size();
}

uint32_t GetLastEventFrame() {
    if (s_eventTable.empty()) {
        return 0;
    }
    return s_eventTable.back().frame;
}

uint32_t GetLastSubtitleFrame() {
    if (s_eventTable.empty()) {
        return 0;
    }
    for (size_t i = s_eventTable.size(); i > 0; i--) {
        const ScriptEvent& ev = s_eventTable[i - 1];
        if (ev.id >= 1000u && ev.id < 2000u) {
            return ev.frame;
        }
    }
    return 0;
}

void PumpEvents(uint32_t stageFrame, PrGameContext& ctx) {
    s_currentLanguageIndex = (int)ctx.languageIndex;
    s_currentStageFrame = stageFrame;
    s_currentFrame = stageFrame;

    if (s_activeSubtitle.text != nullptr) {
        const uint32_t endFrame = s_activeSubtitle.startFrame + s_activeSubtitle.durationFrames;
        if (stageFrame >= endFrame) {
            s_activeSubtitle.text = nullptr;
        }
    }
    if (TryPumpParsedStage1TextEvents(stageFrame, ctx)) {
        RefreshLatestResolvedSnapshot(ctx);
        return;
    }
    ClearLatestResolvedSnapshot();
    // 有事件表：按 frame 推进
    if (!s_eventTable.empty()) {
        while (s_nextEventIndex >= 0 && s_nextEventIndex < (int)s_eventTable.size()) {
            const ScriptEvent& ev0 = s_eventTable[(size_t)s_nextEventIndex];
            if (stageFrame < (uint32_t)ev0.frame) {
                break;
            }

            const uint32_t curFrame = ev0.frame;
            const int startIndex = s_nextEventIndex;
            int endIndex = startIndex;
            while (endIndex >= 0 && endIndex < (int)s_eventTable.size() && s_eventTable[(size_t)endIndex].frame == curFrame) {
                endIndex++;
            }

            int subCount = 0;
            int subFirst = -1;
            for (int i = startIndex; i < endIndex; i++) {
                const ScriptEvent& ev = s_eventTable[(size_t)i];
                if (ev.id >= 1000u && ev.id < 2000u) {
                    if (subFirst < 0) subFirst = i;
                    subCount++;
                }
            }

            int selectedSubIndex = -1;
            if (subCount == 1) {
                selectedSubIndex = subFirst;
            } else if (subCount == 5) {
                const int lang = (ctx.languageIndex >= 0 && ctx.languageIndex < 5) ? (int)ctx.languageIndex : 0;

                // Prefer: pick the subtitle variant that actually has a text for current language.
                for (int i = startIndex; i < endIndex; i++) {
                    const ScriptEvent& ev = s_eventTable[(size_t)i];
                    if (ev.id < 1000u || ev.id >= 2000u) {
                        continue;
                    }
                    const char* t = GetSubtitleText(ev.id);
                    if (t != nullptr) {
                        selectedSubIndex = i;
                        break;
                    }
                }

                // Fallback: ordinal pick
                if (selectedSubIndex < 0) {
                    const int pick = (lang < subCount) ? lang : (subCount - 1);
                    int n = 0;
                    for (int i = startIndex; i < endIndex; i++) {
                        const ScriptEvent& ev = s_eventTable[(size_t)i];
                        if (ev.id >= 1000u && ev.id < 2000u) {
                            if (n == pick) {
                                selectedSubIndex = i;
                                break;
                            }
                            n++;
                        }
                    }
                }

                if (selectedSubIndex < 0) {
                    selectedSubIndex = subFirst;
                }
            }

            for (int i = startIndex; i < endIndex; i++) {
                const ScriptEvent& ev = s_eventTable[(size_t)i];
                const bool isSubtitle = (ev.id >= 1000u && ev.id < 2000u);
                if (isSubtitle && selectedSubIndex >= 0 && i != selectedSubIndex) {
                    continue;
                }

                s_emitStageFrame = ev.frame;
                s_emitSubtitleDuration = 0;
                if (isSubtitle) {
                    uint32_t nextSubFrame = 0;
                    for (int j = endIndex; j >= 0 && j < (int)s_eventTable.size(); j++) {
                        const ScriptEvent& nx = s_eventTable[(size_t)j];
                        if (nx.frame <= ev.frame) {
                            continue;
                        }
                        if (nx.id >= 1000u && nx.id < 2000u) {
                            nextSubFrame = nx.frame;
                            break;
                        }
                    }
                    uint32_t dur = 90;
                    if (nextSubFrame > ev.frame) {
                        dur = nextSubFrame - ev.frame;
                    }
                    if (dur < 1u) dur = 1u;
                    if (dur > 900u) dur = 900u;
                    s_emitSubtitleDuration = dur;
                }

                ctx.lastSqevsEventId = ev.id;
                ctx.lastSqevsEventFrame = ctx.frame;
                Emit(ev.id, 0, 0, 0);
            }

            s_nextEventIndex = endIndex;
        }
        return;
    }

    // 无事件表：fallback（旧固定时间点）
    struct EvDef { uint32_t frame; uint32_t id; };
    static const EvDef kEvs[] = {
        { 30, 1001 },
        { 150, 1002 },
        { 300, 1003 },
        { 600, 1004 },
    };
    for (uint32_t i = 0; i < (uint32_t)(sizeof(kEvs) / sizeof(kEvs[0])); i++) {
        const uint32_t bit = (1u << i);
        if ((s_fallbackMask & bit) == 0 && stageFrame >= kEvs[i].frame) {
            s_fallbackMask |= bit;
            ctx.lastSqevsEventId = kEvs[i].id;
            ctx.lastSqevsEventFrame = ctx.frame;
            Emit(kEvs[i].id, 0, 0, 0);
        }
    }
}

}
