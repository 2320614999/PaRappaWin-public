#include "face_event_processor.h"
#include "../logger.h"
#include <algorithm>
#include <cctype>
#include <unordered_set>

static int ParseTrailingNumberRank(const std::string& s) {
    int i = (int)s.size() - 1;
    while (i >= 0) {
        const char c = s[(size_t)i];
        if (c >= '0' && c <= '9') {
            i--;
            continue;
        }
        break;
    }
    const int start = i + 1;
    if (start >= (int)s.size()) return 1000000;
    int v = 0;
    for (size_t p = (size_t)start; p < s.size(); p++) {
        const char c = s[p];
        if (c < '0' || c > '9') break;
        v = v * 10 + (c - '0');
        if (v > 1000000) return 1000000;
    }
    return v;
}

static int MapEyeRankAllowed(int r) {
    if (r == 0 || r == 1 || r == 2 || r == 4) return r;
    if (r == 3) return 2;
    return 0;
}

void FaceEventProcessor::Init(const SceneEventData& data,
                               const std::vector<std::string>& memFileNames) {
    m_data = data;
    m_memFileNames = memFileNames;
    m_valid = data.valid && !data.events.empty();
    m_faceTIMOffset = -1;
    Reset();

    // Auto-detect face TIM offset: find first file starting with "F_"
    // (face TIMs are named F_PAKU_*, F_PAMEL*, F_PAMER*, etc.)
    for (size_t i = 0; i < m_memFileNames.size(); i++) {
        std::string lower = m_memFileNames[i];
        std::transform(lower.begin(), lower.end(), lower.begin(),
                       [](unsigned char c) { return std::tolower(c); });
        if (lower.size() >= 2 && lower[0] == 'f' && lower[1] == '_') {
            m_faceTIMOffset = static_cast<int>(i);
            break;
        }
    }

    auto seedDefaultFamilyTim = [&](const char* famKey) {
        std::string best;
        int bestRank = 1000000;
        for (const auto& fn : m_memFileNames) {
            std::string n = fn;
            std::transform(n.begin(), n.end(), n.begin(),
                           [](unsigned char c) { return std::tolower(c); });
            auto dot = n.rfind('.');
            if (dot != std::string::npos) {
                n = n.substr(0, dot);
            }
            const std::string fam = TimFamilyKey(n);
            if (fam != famKey) {
                continue;
            }
            const int r = ParseTrailingNumberRank(n);
            if (best.empty() || r < bestRank) {
                best = n;
                bestRank = r;
            }
        }
        if (!best.empty()) {
            m_baseFamilyToTim[famKey] = best;
            m_currentFamilyToTim[famKey] = best;
        }
    };
    seedDefaultFamilyTim("f_pamel");
    seedDefaultFamilyTim("f_pamer");

    if (m_valid) {
        Log::Printf("FaceEventProcessor: initialized %zu events, %zu mem files, faceTIM offset=%d, %zu tim pairs",
                     m_data.events.size(), m_memFileNames.size(), m_faceTIMOffset,
                     m_data.timPairTable.size());
        if (m_faceTIMOffset >= 0 && m_faceTIMOffset < (int)m_memFileNames.size()) {
            Log::Printf("  first face TIM: mem[%d] = %s", m_faceTIMOffset,
                         m_memFileNames[m_faceTIMOffset].c_str());
        }
    }
}

void FaceEventProcessor::Reset() {
    m_eventIndex = 0;
    m_lastFrameInCycle = 0;
    m_cycleInit = false;
    m_menuIdleOverlayFrameAbs = 0;
    m_menuIdleNextBlinkFrameAbs = 0;
    m_menuIdleRng = 0;
    m_baseFamilyToTim.clear();
    m_currentFamilyToTim.clear();
    m_patMaxRankByFamily.clear();
    m_smileEyeActive = false;
    m_smileEyeExpireFrameAbs = 0;
    m_smilePrevEyeL.clear();
    m_smilePrevEyeR.clear();
    for (auto& s : m_slots) {
        s = OverlaySlotState{};
    }

    auto seedDefaultFamilyTim = [&](const char* famKey) {
        std::string best;
        int bestRank = 1000000;
        for (const auto& fn : m_memFileNames) {
            std::string n = fn;
            std::transform(n.begin(), n.end(), n.begin(),
                           [](unsigned char c) { return std::tolower(c); });
            auto dot = n.rfind('.');
            if (dot != std::string::npos) {
                n = n.substr(0, dot);
            }
            const std::string fam = TimFamilyKey(n);
            if (fam != famKey) {
                continue;
            }
            const int r = ParseTrailingNumberRank(n);
            if (best.empty() || r < bestRank) {
                best = n;
                bestRank = r;
            }
        }
        if (!best.empty()) {
            m_baseFamilyToTim[famKey] = best;
            m_currentFamilyToTim[famKey] = best;
        }
    };
    seedDefaultFamilyTim("f_pamel");
    seedDefaultFamilyTim("f_pamer");

    {
        for (const auto& fn : m_memFileNames) {
            std::string n = fn;
            std::transform(n.begin(), n.end(), n.begin(), ::tolower);
            auto dot = n.rfind('.');
            if (dot != std::string::npos) {
                n = n.substr(0, dot);
            }
            if (n.size() < 4 || fn.size() < 4) {
                continue;
            }
            const std::string fam = TimFamilyKey(n);
            if (fam.rfind("f_pat", 0) != 0) {
                continue;
            }
            const int r = ParseTrailingNumberRank(n);
            if (r != 1000000) {
                auto it = m_patMaxRankByFamily.find(fam);
                if (it == m_patMaxRankByFamily.end() || r > it->second) {
                    m_patMaxRankByFamily[fam] = r;
                }
            }
        }
        for (const auto& kv : m_patMaxRankByFamily) {
            seedDefaultFamilyTim(kv.first.c_str());
        }
    }

    m_menuIdleSlotForFL = -1;
    m_menuIdleSlotForFR = -1;
    {
        auto scoreSlotForFamily = [&](int slotId, const char* wantFamily) -> int {
            int best = -1;
            if (slotId < 1 || slotId > 5 || !wantFamily) return best;
            const auto& entries = m_data.overlaySlots[slotId].entries;
            for (const auto& entry : entries) {
                for (int k = 0; k < 4 && entry.timIndices[k] != 0; k++) {
                    std::string name = IndexToTimName(entry.timIndices[k]);
                    if (name.empty()) continue;
                    const std::string fam = TimFamilyKey(name);
                    if (fam != wantFamily) continue;
                    const int r = ParseTrailingNumberRank(name);
                    if (r != 1000000 && r > best) {
                        best = r;
                    }
                }
            }
            return best;
        };

        int bestFL = -1;
        int bestFR = -1;
        for (int s = 3; s <= 5; s++) {
            const int scFL = scoreSlotForFamily(s, "f_patfl");
            if (scFL >= 0 && (m_menuIdleSlotForFL < 0 || scFL > bestFL || (scFL == bestFL && s < m_menuIdleSlotForFL))) {
                m_menuIdleSlotForFL = s;
                bestFL = scFL;
            }
            const int scFR = scoreSlotForFamily(s, "f_patfr");
            if (scFR >= 0 && (m_menuIdleSlotForFR < 0 || scFR > bestFR || (scFR == bestFR && s < m_menuIdleSlotForFR))) {
                m_menuIdleSlotForFR = s;
                bestFR = scFR;
            }
        }
    }
}

void FaceEventProcessor::SetSmileEyeHoldFrames(uint32_t holdFrames) {
    m_smileEyeHoldFrames = holdFrames;
}

void FaceEventProcessor::RestoreBase(const TimLoadCallback& loadTim) {
    if (!m_valid) return;

    for (auto& s : m_slots) {
        s = OverlaySlotState{};
    }

    m_smileEyeActive = false;
    m_smileEyeExpireFrameAbs = 0;
    m_smilePrevEyeL.clear();
    m_smilePrevEyeR.clear();

    {
        auto itL = m_baseFamilyToTim.find("f_pamel");
        if (itL != m_baseFamilyToTim.end() && !itL->second.empty()) {
            ApplyTim(itL->second, loadTim, false);
        }
        auto itR = m_baseFamilyToTim.find("f_pamer");
        if (itR != m_baseFamilyToTim.end() && !itR->second.empty()) {
            ApplyTim(itR->second, loadTim, false);
        }
    }

    for (const auto& kv : m_baseFamilyToTim) {
        if (kv.first == "f_pamel" || kv.first == "f_pamer") continue;
        if (kv.second.empty()) continue;
        ApplyTim(kv.second, loadTim, false);
    }

    m_eventIndex = 0;
    m_lastFrameInCycle = 0;
    m_cycleInit = false;
}

std::string FaceEventProcessor::TimFamilyKey(const std::string& timName) {
    if (timName.empty()) return {};
    std::string k = timName;
    while (!k.empty()) {
        unsigned char c = (unsigned char)k.back();
        if (c >= '0' && c <= '9') {
            k.pop_back();
            continue;
        }
        break;
    }
    if (!k.empty() && k.back() == '_') {
        k.pop_back();
    }
    return k;
}

void FaceEventProcessor::RememberBaseTim(const std::string& timName) {
    const std::string fam = TimFamilyKey(timName);
    if (fam.empty()) return;
    auto it = m_baseFamilyToTim.find(fam);
    if (it == m_baseFamilyToTim.end() || it->second.empty()) {
        m_baseFamilyToTim[fam] = timName;
    }
}

void FaceEventProcessor::ApplyTim(const std::string& timName,
                                  const TimLoadCallback& loadTim,
                                  bool rememberBase) {
    if (timName.empty()) return;
    std::string mappedName = timName;
    const std::string fam0 = TimFamilyKey(mappedName);
    if (fam0 == "f_pamel" || fam0 == "f_pamer") {
        const int r = ParseTrailingNumberRank(mappedName);
        const int want = MapEyeRankAllowed(r);
        if (r != 1000000 && want != r) {
            const std::string candidate = fam0 + std::to_string(want);
            bool ok = false;
            for (const auto& fn : m_memFileNames) {
                std::string n = fn;
                std::transform(n.begin(), n.end(), n.begin(),
                               [](unsigned char c) { return std::tolower(c); });
                auto dot = n.rfind('.');
                if (dot != std::string::npos) {
                    n = n.substr(0, dot);
                }
                if (n == candidate) {
                    ok = true;
                    break;
                }
            }
            if (ok) {
                Log::Printf("FaceEvent WARN: unexpected eye TIM '%s' -> map to '%s'", mappedName.c_str(), candidate.c_str());
                mappedName = candidate;
            }
        }
    }

    const std::string fam = TimFamilyKey(mappedName);
    if (!fam.empty()) {
        if (m_smileEyeActive && rememberBase && (fam == "f_pamel" || fam == "f_pamer")) {
            const int r = ParseTrailingNumberRank(mappedName);
            if (r != 4) {
                m_smileEyeActive = false;
            }
        }
        auto it = m_currentFamilyToTim.find(fam);
        const bool changed = (it == m_currentFamilyToTim.end() || it->second != mappedName);
        if (!changed) {
            if (rememberBase) {
                RememberBaseTim(mappedName);
            }
            return;
        }
        if (changed && (fam == "f_pamel" || fam == "f_pamer")) {
            const char* oldName = (it == m_currentFamilyToTim.end()) ? "" : it->second.c_str();
            Log::Printf("FaceEvent: eye fam '%s' change '%s' -> '%s' (rememberBase=%d)",
                        fam.c_str(), oldName, mappedName.c_str(), rememberBase ? 1 : 0);
        }
        m_currentFamilyToTim[fam] = mappedName;
    }
    if (rememberBase) {
        RememberBaseTim(mappedName);
    }
    loadTim(mappedName);
}

void FaceEventProcessor::BuildRestoreListForSlot(int slotId, OverlaySlotState& slot) {
    slot.restoreTims.clear();
    if (slotId < 1 || slotId > 5) return;

    const auto& entries = m_data.overlaySlots[slotId].entries;
    if (entries.empty()) return;

    std::unordered_set<std::string> seenFamilies;
    seenFamilies.reserve(16);
    std::vector<std::string> familyOrder;
    familyOrder.reserve(16);

    struct Fallback {
        uint32_t frameOffset = 0;
        std::string name;
    };
    std::unordered_map<std::string, Fallback> fallbackByFamily;
    fallbackByFamily.reserve(16);

    for (const auto& entry : entries) {
        for (int k = 0; k < 4 && entry.timIndices[k] != 0; k++) {
            std::string name = IndexToTimName(entry.timIndices[k]);
            if (name.empty()) continue;
            std::string fam = TimFamilyKey(name);
            if (fam.empty()) continue;

            if (seenFamilies.insert(fam).second) {
                familyOrder.push_back(fam);
            }

            auto& fb = fallbackByFamily[fam];
            if (fb.name.empty() || entry.frameOffset < fb.frameOffset) {
                fb.frameOffset = entry.frameOffset;
                fb.name = name;
            }
        }
    }

    for (const auto& fam : familyOrder) {
        auto curIt = m_currentFamilyToTim.find(fam);
        if (curIt != m_currentFamilyToTim.end() && !curIt->second.empty()) {
            slot.restoreTims.push_back(curIt->second);
        } else {
            auto it = m_baseFamilyToTim.find(fam);
            if (it != m_baseFamilyToTim.end() && !it->second.empty()) {
                slot.restoreTims.push_back(it->second);
            } else {
                auto fbIt = fallbackByFamily.find(fam);
                if (fbIt != fallbackByFamily.end()) {
                    slot.restoreTims.push_back(fbIt->second.name);
                }
            }
        }
    }
}

uint32_t FaceEventProcessor::GetDanceStartFrame() const {
    if (!m_valid || m_data.events.empty()) return 0;
    return m_data.events[0].frame;
}

uint32_t FaceEventProcessor::GetMaxFrame() const {
    if (!m_valid || m_data.events.empty()) return 0;
    return m_data.events.back().frame;
}

std::string FaceEventProcessor::IndexToTimName(int index) const {
    // dword_80091858 is 1-based: index i maps to memFileNames[i-1]
    if (index <= 0 || index > static_cast<int>(m_memFileNames.size()))
        return {};

    const std::string& fullName = m_memFileNames[index - 1];
    {
        std::string lower = fullName;
        std::transform(lower.begin(), lower.end(), lower.begin(),
                       [](unsigned char c) { return std::tolower(c); });
        if (lower.size() < 4 || lower.rfind(".tim") != (lower.size() - 4)) {
            return {};
        }
    }

    std::string name = fullName;

    // Remove extension
    auto dot = name.rfind('.');
    if (dot != std::string::npos)
        name = name.substr(0, dot);

    // Lowercase
    for (auto& c : name) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));

    return name;
}

std::string FaceEventProcessor::FaceTimIndexToName(int faceTimIndex) const {
    // Face TIM index is 1-based within the face TIM section of the Mem block.
    // Face TIM 1 = first face TIM (e.g. F_PAKU_0.TIM) at m_faceTIMOffset.
    if (faceTimIndex <= 0) return {};
    int memIndex = faceTimIndex;
    if (m_faceTIMOffset >= 0) {
        memIndex = m_faceTIMOffset + faceTimIndex;
    }
    return IndexToTimName(memIndex);
}

void FaceEventProcessor::LoadTimPair(int lookupIndex, uint32_t currentFrameAbs,
                                     FaceEventMode mode,
                                     const TimLoadCallback& loadTim,
                                     const char* source,
                                     bool applySecond) {
    if (lookupIndex <= 0 || lookupIndex > static_cast<int>(m_data.timPairTable.size())) {
        Log::Printf("FaceEvent: %s[%d] TIM pair lookup index out of bounds", source ? source : "?", lookupIndex);
        return;
    }
    const auto& pair = m_data.timPairTable[lookupIndex - 1]; // 1-based index

    Log::Printf("FaceEvent: %s[%d] pair timA=%d timB=%d applySecond=%d",
                source ? source : "?",
                lookupIndex,
                (int)pair.timA,
                (int)pair.timB,
                applySecond ? 1 : 0);

    if (pair.timA <= 0) {
        Log::Printf("FaceEvent: %s[%d] pair timA<=0 (skip)",
                    source ? source : "?",
                    lookupIndex);
        return;
    }

    auto describeFaceTim = [&](int16_t v, const char* field) -> std::string {
        if (v <= 0) return {};
        const int memIdx = (m_faceTIMOffset >= 0) ? (m_faceTIMOffset + (int)v) : (int)v;
        std::string name = FaceTimIndexToName(v);
        if (name.empty()) {
            Log::Printf("FaceEvent: %s[%d].%s=%d -> face (mem idx=%d) -> EMPTY",
                        source ? source : "?",
                        lookupIndex,
                        field ? field : "?",
                        (int)v,
                        memIdx);
            return {};
        }
        Log::Printf("FaceEvent: %s[%d].%s=%d -> face (mem idx=%d) -> '%s'",
                    source ? source : "?",
                    lookupIndex,
                    field ? field : "?",
                    (int)v,
                    memIdx,
                    name.c_str());
        return name;
    };

    std::string nameA = describeFaceTim(pair.timA, "timA");
    std::string nameB;
    if (applySecond) {
        nameB = describeFaceTim(pair.timB, "timB");
    }

    (void)mode;

    std::string prevEyeL;
    std::string prevEyeR;
    {
        auto itL = m_currentFamilyToTim.find("f_pamel");
        auto itR = m_currentFamilyToTim.find("f_pamer");
        if (itL != m_currentFamilyToTim.end()) prevEyeL = itL->second;
        if (itR != m_currentFamilyToTim.end()) prevEyeR = itR->second;
    }

    auto hasTimName = [&](const std::string& wantLowerNoExt) -> bool {
        if (wantLowerNoExt.empty()) return false;
        for (const auto& fn : m_memFileNames) {
            std::string n = fn;
            std::transform(n.begin(), n.end(), n.begin(),
                           [](unsigned char c) { return std::tolower(c); });
            auto dot = n.rfind('.');
            if (dot != std::string::npos) {
                n = n.substr(0, dot);
            }
            if (n == wantLowerNoExt) {
                return true;
            }
        }
        return false;
    };

    const std::string famA = TimFamilyKey(nameA);
    const std::string famB = TimFamilyKey(nameB);
    const bool isEyePair = (!nameA.empty() && !nameB.empty()) &&
                           ((famA == "f_pamel" && famB == "f_pamer") || (famA == "f_pamer" && famB == "f_pamel"));
    if (isEyePair) {
        if (m_smileEyeActive) {
            const int rA0 = ParseTrailingNumberRank(nameA);
            const int rB0 = ParseTrailingNumberRank(nameB);
            if (rA0 != 4 || rB0 != 4) {
                m_smileEyeActive = false;
            }
        }
        const int rA = ParseTrailingNumberRank(nameA);
        const int rB = ParseTrailingNumberRank(nameB);
        if (rA != 1000000 && MapEyeRankAllowed(rA) != rA) {
            Log::Printf("FaceEvent WARN: unexpected eye TIM in pair '%s' (rank=%d)", nameA.c_str(), rA);
        }
        if (rB != 1000000 && MapEyeRankAllowed(rB) != rB) {
            Log::Printf("FaceEvent WARN: unexpected eye TIM in pair '%s' (rank=%d)", nameB.c_str(), rB);
        }
        if (rA != 1000000 && rB != 1000000 && rA != rB) {
            int rCur = 1000000;
            {
                auto itL = m_currentFamilyToTim.find("f_pamel");
                auto itR = m_currentFamilyToTim.find("f_pamer");
                if (itL != m_currentFamilyToTim.end() && itR != m_currentFamilyToTim.end()) {
                    const int a = ParseTrailingNumberRank(itL->second);
                    const int b = ParseTrailingNumberRank(itR->second);
                    if (a != 1000000 && a == b) {
                        rCur = a;
                    }
                }
            }

            int rBase = 1000000;
            {
                auto itL = m_baseFamilyToTim.find("f_pamel");
                auto itR = m_baseFamilyToTim.find("f_pamer");
                if (itL != m_baseFamilyToTim.end() && itR != m_baseFamilyToTim.end()) {
                    const int a = ParseTrailingNumberRank(itL->second);
                    const int b = ParseTrailingNumberRank(itR->second);
                    if (a != 1000000 && a == b) {
                        rBase = a;
                    }
                }
            }

            bool remember = false;
            int r = 1000000;
            if (rCur == 4 || rBase == 4) {
                r = 4;
                remember = true;
            } else if (rCur != 1000000 && rCur != 5 && rCur != 7) {
                r = rCur;
            } else if (rBase != 1000000 && rBase != 5 && rBase != 7) {
                r = rBase;
            } else {
                r = (rA < rB) ? rA : rB;
                if ((rA == 7) != (rB == 7)) {
                    r = (rA == 7) ? rB : rA;
                }
                if ((rA == 5) != (rB == 5)) {
                    r = (rA == 5) ? rB : rA;
                }
                if (rCur == 1000000 && rBase == 1000000) {
                    remember = true;
                }
            }
            const int rr = MapEyeRankAllowed(r);
            const std::string wantL = std::string("f_pamel") + std::to_string(rr);
            const std::string wantR = std::string("f_pamer") + std::to_string(rr);
            if (hasTimName(wantL) && hasTimName(wantR)) {
                Log::Printf("FaceEvent: eye pair mismatch '%s'/'%s' -> normalize to '%s'/'%s'",
                            nameA.c_str(), nameB.c_str(), wantL.c_str(), wantR.c_str());
                ApplyTim(wantL, loadTim, remember);
                ApplyTim(wantR, loadTim, remember);
                if (rr == 4 && m_smileEyeHoldFrames != 0u) {
                    m_smileEyeActive = true;
                    m_smileEyeExpireFrameAbs = currentFrameAbs + m_smileEyeHoldFrames;
                    m_smilePrevEyeL = prevEyeL;
                    m_smilePrevEyeR = prevEyeR;
                }
                return;
            }
        }
    }

    if (!nameA.empty()) {
        ApplyTim(nameA, loadTim, true);
    }
    if (applySecond && !nameB.empty()) {
        ApplyTim(nameB, loadTim, true);
    }

    if (isEyePair && m_smileEyeHoldFrames != 0u) {
        const int rA = ParseTrailingNumberRank(nameA);
        const int rB = ParseTrailingNumberRank(nameB);
        if (rA == 4 && rB == 4) {
            m_smileEyeActive = true;
            m_smileEyeExpireFrameAbs = currentFrameAbs + m_smileEyeHoldFrames;
            m_smilePrevEyeL = prevEyeL;
            m_smilePrevEyeR = prevEyeR;
        }
    }
}

void FaceEventProcessor::ProcessEvent(const SceneEvent& evt,
                                      uint32_t currentFrameAbs,
                                      FaceEventMode mode,
                                      const TimLoadCallback& loadTim) {
    if (mode == FaceEventMode::Slot2Only) {
        if (evt.b11 != 0) {
            if (evt.b11 <= 5) {
                if (evt.b11 == 2) {
                    ActivateOverlaySlot(2, currentFrameAbs);
                }
            } else {
                const uint8_t mask = (uint8_t)(evt.b11 & 0x1Fu);
                if (mask & (uint8_t)(1u << 1u)) {
                    ActivateOverlaySlot(2, currentFrameAbs);
                }
            }
        }
        return;
    }

    // b8: mouth/face TIM — uses full mem 1-based index (always checked, no flag)
    if (evt.b8 != 0) {
        std::string name = IndexToTimName(evt.b8);
        if (!name.empty()) {
            Log::Printf("FaceEvent: b8=%d -> mem -> '%s'", evt.b8, name.c_str());
            ApplyTim(name, loadTim, true);
        }
    }

    // b10: TIM pair lookup index (flag 0x10000)
    if ((evt.flags & 0x10000)) {
        LoadTimPair(evt.b10, currentFrameAbs, mode, loadTim, "b10", true);
    }

    // b9: VDF animation index (flag 0x40000) — NOT a TIM
    if ((evt.flags & 0x40000) && evt.b9 != 0) {
        Log::Printf("FaceEvent: b9=%d (flag 0x40000) — VDF anim index (skipped)", evt.b9);
    }

    // b12: Direct TIM index (flag 0x800000) — uses full mem 1-based index
    if ((evt.flags & 0x800000) && evt.b12 != 0) {
        std::string name = FaceTimIndexToName(evt.b12);
        if (!name.empty()) {
            Log::Printf("FaceEvent: b12=%d -> face -> '%s'", evt.b12, name.c_str());
            ApplyTim(name, loadTim, true);
        }
    }

    // b13/b14: TIM pair lookup indices
    if ((evt.flags & 0x400000)) {
        LoadTimPair(evt.b13, currentFrameAbs, mode, loadTim, "b13", true);
    }
    if ((evt.flags & 0x1000000)) {
        LoadTimPair(evt.b14, currentFrameAbs, mode, loadTim, "b14", true);
    }

    // b11: activate overlay slot (no flag check, just non-zero)
    if (evt.b11 != 0) {
        if (evt.b11 <= 5) {
            ActivateOverlaySlot(evt.b11, currentFrameAbs);
        } else {
            const uint8_t mask = (uint8_t)(evt.b11 & 0x1Fu);
            for (int s = 1; s <= 5; s++) {
                if (mask & (uint8_t)(1u << (unsigned)(s - 1))) {
                    ActivateOverlaySlot(s, currentFrameAbs);
                }
            }
        }
    }
}

void FaceEventProcessor::ActivateOverlaySlot(int slotId, uint32_t baseFrame) {
    if (slotId < 1 || slotId > 5) return;

    auto& slot = m_slots[slotId];
    if (slot.active) {
        Log::Printf("FaceEvent: overlay slot %d already active (ignore re-activate), newBaseFrame=%d oldBaseFrame=%d curIndex=%d",
                    slotId,
                    (int)baseFrame,
                    (int)slot.baseFrame,
                    slot.currentIndex);
        return;
    }
    slot.active = true;
    slot.tableSlot = slotId;
    slot.currentIndex = 0;
    slot.baseFrame = baseFrame;
    slot.pendingRestore = false;
    slot.exhaustedFrame = 0;
    slot.restoreTims.clear();
    BuildRestoreListForSlot(slotId, slot);

    Log::Printf("FaceEvent: activate overlay slot %d, baseFrame=%d, entries=%zu",
                 slotId, baseFrame, m_data.overlaySlots[slotId].entries.size());
}

void FaceEventProcessor::ProcessOverlaySlots(uint32_t currentFrameAbs,
                                              const TimLoadCallback& loadTim) {
    auto hasTimName = [&](const std::string& wantLowerNoExt) -> bool {
        if (wantLowerNoExt.empty()) return false;
        for (const auto& fn : m_memFileNames) {
            std::string n = fn;
            std::transform(n.begin(), n.end(), n.begin(),
                           [](unsigned char c) { return (char)std::tolower(c); });
            auto dot = n.rfind('.');
            if (dot != std::string::npos) {
                n = n.substr(0, dot);
            }
            if (n == wantLowerNoExt) {
                return true;
            }
        }
        return false;
    };

    for (int s = 1; s <= 5; s++) {
        auto& state = m_slots[s];
        if (!state.active) continue;
        const bool menuIdle = (m_lastMode == FaceEventMode::MenuIdle);
        const uint32_t kOverlayHoldFrames = (s == 2) ? (menuIdle ? 2u : 48u) : 24u;

        // Restore base if we reached the end and the hold window expired.
        if (state.pendingRestore && currentFrameAbs > state.exhaustedFrame) {
            for (const auto& tn : state.restoreTims) {
                if (!tn.empty()) {
                    Log::Printf("FaceEvent: overlay slot %d restore '%s'", s, tn.c_str());
                    ApplyTim(tn, loadTim, false);
                }
            }
            state.active = false;
            state.pendingRestore = false;
            state.currentIndex = 0;
            state.restoreTims.clear();
            Log::Printf("FaceEvent: overlay slot %d exhausted, deactivated", s);
            continue;
        }

        const auto& entries = m_data.overlaySlots[s].entries;
        if (entries.empty()) {
            state.active = false;
            continue;
        }

        uint32_t slot2MaxOff = 0;
        if (menuIdle && s == 2) {
            for (const auto& e : entries) {
                if (e.frameOffset > slot2MaxOff) slot2MaxOff = e.frameOffset;
            }
            if (slot2MaxOff == 0) slot2MaxOff = 1;
        }

        uint32_t lastTriggerFrame = state.baseFrame;
        while (state.currentIndex < static_cast<int>(entries.size())) {
            const auto& entry = entries[state.currentIndex];
            uint32_t triggerFrame = state.baseFrame + entry.frameOffset;
            if (menuIdle && s == 2) {
                constexpr uint32_t kBlinkTotalFrames = 6u;
                const uint32_t scaled = (uint32_t)(((uint64_t)entry.frameOffset * (uint64_t)kBlinkTotalFrames) / (uint64_t)slot2MaxOff);
                triggerFrame = state.baseFrame + scaled;
                if (state.currentIndex != 0 && triggerFrame <= lastTriggerFrame) {
                    triggerFrame = lastTriggerFrame + 1;
                }
            }

            if (currentFrameAbs < triggerFrame)
                break;

            std::string allowPatFamilyA;
            std::string allowPatFamilyB;
            if (menuIdle && s >= 3) {
                if (m_menuIdlePointDir == 0) {
                    allowPatFamilyA = "f_patfr";
                    allowPatFamilyB = "f_patbr";
                } else if (m_menuIdlePointDir == 1) {
                    allowPatFamilyA = "f_patfl";
                    allowPatFamilyB = "f_patbl";
                }
            }

            if (!allowPatFamilyA.empty()) {
                auto applyBestForFam = [&](const std::string& wantFam) {
                    bool entryHasFam = false;
                    for (int k = 0; k < 4 && entry.timIndices[k] != 0; k++) {
                        std::string n = IndexToTimName(entry.timIndices[k]);
                        if (n.empty()) continue;
                        if (TimFamilyKey(n) == wantFam) {
                            entryHasFam = true;
                            break;
                        }
                    }
                    if (!entryHasFam) {
                        return;
                    }

                    auto itMax = m_patMaxRankByFamily.find(wantFam);
                    if (itMax != m_patMaxRankByFamily.end()) {
                        const std::string wantName = wantFam + std::to_string(itMax->second);
                        if (hasTimName(wantName)) {
                            Log::Printf("FaceEvent: overlay slot %d entry %d -> load '%s' (forced max rank)",
                                         s, state.currentIndex, wantName.c_str());
                            ApplyTim(wantName, loadTim, false);
                            return;
                        }
                    }
                    std::string best;
                    int bestRank = -1;
                    int bestMemIdx = 0;
                    for (int k = 0; k < 4 && entry.timIndices[k] != 0; k++) {
                        std::string name = IndexToTimName(entry.timIndices[k]);
                        if (name.empty()) continue;
                        const std::string fam = TimFamilyKey(name);
                        if (fam != wantFam) continue;
                        const int r = ParseTrailingNumberRank(name);
                        if (r != 1000000 && r > bestRank) {
                            bestRank = r;
                            best = name;
                            bestMemIdx = entry.timIndices[k];
                        }
                    }
                    if (!best.empty()) {
                        Log::Printf("FaceEvent: overlay slot %d entry %d -> load '%s' (mem idx=%d)",
                                     s, state.currentIndex, best.c_str(), bestMemIdx);
                        ApplyTim(best, loadTim, false);
                    }
                };

                applyBestForFam(allowPatFamilyA);
                if (!allowPatFamilyB.empty()) {
                    applyBestForFam(allowPatFamilyB);
                }
            } else {
                for (int k = 0; k < 4 && entry.timIndices[k] != 0; k++) {
                    std::string name = IndexToTimName(entry.timIndices[k]);
                    if (!name.empty()) {
                        if (s == 1) {
                            const std::string fam1 = TimFamilyKey(name);
                            if (fam1 == "f_pamel" || fam1 == "f_pamer") {
                                const int r1 = ParseTrailingNumberRank(name);
                                if (r1 == 5) {
                                    const std::string want = fam1 + "4";
                                    if (hasTimName(want)) {
                                        name = want;
                                    }
                                }
                            }
                        }
                        if (s != 2 && s != 1) {
                            const std::string fam = TimFamilyKey(name);
                            if (fam == "f_pamel" || fam == "f_pamer" || fam == "f_paku") {
                                continue;
                            }
                        }
                        Log::Printf("FaceEvent: overlay slot %d entry %d -> load '%s' (mem idx=%d)",
                                     s, state.currentIndex, name.c_str(), entry.timIndices[k]);
                        ApplyTim(name, loadTim, false);
                    }
                }
            }
            lastTriggerFrame = triggerFrame;
            state.currentIndex++;
        }

        if (state.currentIndex >= static_cast<int>(entries.size())) {
            if (menuIdle && s >= 3) {
                state.currentIndex = 0;
                state.baseFrame = currentFrameAbs;
            } else {
                if (!state.pendingRestore) {
                    state.pendingRestore = true;
                    const uint32_t base = (currentFrameAbs > lastTriggerFrame) ? currentFrameAbs : lastTriggerFrame;
                    state.exhaustedFrame = base + kOverlayHoldFrames;
                }
            }
        }
    }
}

void FaceEventProcessor::Update(uint32_t currentFrameAbs, FaceEventMode mode, const TimLoadCallback& loadTim) {
    if (!m_valid) return;

    const FaceEventMode prevMode = m_lastMode;
    m_lastMode = mode;

    uint32_t overlayFrameAbs = currentFrameAbs;
    if (mode == FaceEventMode::MenuIdle) {
        if (prevMode != FaceEventMode::MenuIdle) {
            m_menuIdleOverlayFrameAbs = 0;
        } else {
            m_menuIdleOverlayFrameAbs++;
        }
        overlayFrameAbs = m_menuIdleOverlayFrameAbs;
    }

    if (mode == FaceEventMode::MenuIdle) {
        if (prevMode != FaceEventMode::MenuIdle) {
            if (m_menuIdleRng == 0u) {
                m_menuIdleRng = currentFrameAbs ^ 0x9E3779B9u;
            }
            if (m_menuIdleNextBlinkFrameAbs == 0u) {
                m_menuIdleNextBlinkFrameAbs = overlayFrameAbs + 360u;
            }
        }

        if (m_slots[1].active || m_slots[1].pendingRestore) {
            m_slots[1].active = false;
            m_slots[1].pendingRestore = false;
            m_slots[1].currentIndex = 0;
            m_slots[1].restoreTims.clear();
        }

        int wantSlot = -1;
        if (m_menuIdlePointDir == 0) {
            wantSlot = m_menuIdleSlotForFR;
        } else if (m_menuIdlePointDir == 1) {
            wantSlot = m_menuIdleSlotForFL;
        }

        {
            static int s_lastDir = -999;
            static int s_lastSlot = -999;
            if (s_lastDir != m_menuIdlePointDir || s_lastSlot != wantSlot) {
                Log::Printf("FaceEvent: MenuIdle dir=%d wantSlot=%d (slotFL=%d slotFR=%d)",
                            m_menuIdlePointDir,
                            wantSlot,
                            m_menuIdleSlotForFL,
                            m_menuIdleSlotForFR);
                s_lastDir = m_menuIdlePointDir;
                s_lastSlot = wantSlot;
            }
        }

        auto forceRestoreAndDeactivate = [&](int s) {
            auto& st = m_slots[s];
            if (!st.active && !st.pendingRestore) {
                st = OverlaySlotState{};
                return;
            }
            for (const auto& tn : st.restoreTims) {
                if (!tn.empty()) {
                    ApplyTim(tn, loadTim, false);
                }
            }
            st = OverlaySlotState{};
        };

        for (int s = 3; s <= 5; s++) {
            if (s != wantSlot) {
                forceRestoreAndDeactivate(s);
            }
        }
        if (wantSlot >= 3 && wantSlot <= 5) {
            if (!m_slots[wantSlot].active && !m_slots[wantSlot].pendingRestore && !m_data.overlaySlots[wantSlot].entries.empty()) {
                ActivateOverlaySlot(wantSlot, overlayFrameAbs);
            }
        }

        if (m_menuIdleNextBlinkFrameAbs != 0u && overlayFrameAbs >= m_menuIdleNextBlinkFrameAbs) {
            auto scheduleNext = [&]() {
                m_menuIdleRng = m_menuIdleRng * 1103515245u + 12345u;
                const uint32_t extra = (m_menuIdleRng >> 16) & 0xFFu;
                const uint32_t interval = 420u + (extra * 2u);
                const uint32_t roll = (m_menuIdleRng >> 24) & 0xFFu;
                if (roll < 32u) {
                    const uint32_t extra2 = (m_menuIdleRng >> 8) & 0x1Fu;
                    m_menuIdleNextBlinkFrameAbs = overlayFrameAbs + (18u + extra2);
                } else {
                    m_menuIdleNextBlinkFrameAbs = overlayFrameAbs + interval;
                }
            };

            if (!m_slots[2].active && !m_slots[2].pendingRestore && !m_data.overlaySlots[2].entries.empty()) {
                ActivateOverlaySlot(2, overlayFrameAbs);
                scheduleNext();
            } else {
                scheduleNext();
            }
        }
    }

    const uint32_t maxFrame = GetMaxFrame();
    uint32_t frameInCycle = currentFrameAbs;
    if (mode == FaceEventMode::Slot2Only || mode == FaceEventMode::MenuIdle) {
        const uint32_t cycle = maxFrame + 1u;
        if (cycle != 0u) {
            frameInCycle = currentFrameAbs % cycle;
            if (currentFrameAbs != 0u && frameInCycle == 0u) {
                frameInCycle = maxFrame;
            }
        }
    } else {
        if (frameInCycle > maxFrame) {
            frameInCycle = maxFrame;
        }
    }

    if (!m_cycleInit) {
        m_cycleInit = true;
        m_lastFrameInCycle = frameInCycle;
    } else if (frameInCycle < m_lastFrameInCycle) {
        m_eventIndex = 0;
        m_lastFrameInCycle = frameInCycle;
    } else {
        m_lastFrameInCycle = frameInCycle;
    }

    if (m_smileEyeActive && m_smileEyeHoldFrames != 0u && currentFrameAbs >= m_smileEyeExpireFrameAbs) {
        if (!m_slots[2].active) {
            std::string curL;
            std::string curR;
            {
                auto itL = m_currentFamilyToTim.find("f_pamel");
                auto itR = m_currentFamilyToTim.find("f_pamer");
                if (itL != m_currentFamilyToTim.end()) curL = itL->second;
                if (itR != m_currentFamilyToTim.end()) curR = itR->second;
            }
            const bool curIsSmile = (ParseTrailingNumberRank(curL) == 4) || (ParseTrailingNumberRank(curR) == 4);
            if (!curIsSmile) {
                m_smileEyeActive = false;
            } else {
                if (!m_smilePrevEyeL.empty()) {
                    ApplyTim(m_smilePrevEyeL, loadTim, false);
                }
                if (!m_smilePrevEyeR.empty()) {
                    ApplyTim(m_smilePrevEyeR, loadTim, false);
                }
                m_smileEyeActive = false;
            }
        }
    }

    if (mode != FaceEventMode::MenuIdle) {
        while (m_eventIndex < static_cast<int>(m_data.events.size())) {
            const auto& evt = m_data.events[m_eventIndex];
            if (frameInCycle < evt.frame)
                break;

            Log::Printf("FaceEvent: process event %d at frame %d (current=%d) flags=0x%08X",
                         m_eventIndex, evt.frame, frameInCycle, evt.flags);

            ProcessEvent(evt, currentFrameAbs, mode, loadTim);
            m_eventIndex++;
        }
    }

    // Process active overlay slots
    ProcessOverlaySlots(overlayFrameAbs, loadTim);
}
