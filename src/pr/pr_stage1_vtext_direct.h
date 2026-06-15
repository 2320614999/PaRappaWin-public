#pragma once

#include "pr_movie_subtitles.h"
#include "pr_scn1.h"

#include <array>
#include <cstdint>

struct PrStage1VTextDirectLineMeta {
    std::array<uint32_t, 5> textAddrs{};
};

struct PrStage1VTextDirectDescInput {
    uint32_t descAddr = 0;
    std::array<uint32_t, 5> textTableAddrs{};
    uint32_t entriesAddr = 0;
    uint32_t entryCount = 0;
    const PrStage1VTextDirectLineMeta* lineMeta = nullptr;
    uint32_t lineMetaCount = 0;
};

struct PrStage1VTextDirectRuntime {
    // sub_80024C84 maps desc fields into gp+217/219/220/221/223.
    uint32_t selectedDescAddr = 0;
    uint32_t selectedFirstTextTableAddr = 0;
    uint32_t selectedEntriesAddr = 0;
    uint32_t selectedEntryCount = 0;
    uint32_t selectedTextTableAddr = 0;
    uint8_t selectedLanguageIndex = 0;
    uint32_t eventCursor = 0;
    uint32_t resetScratch200 = 0;
    uint32_t activeEventFrame = 0;
    uint32_t activeTextPsxAddr = 0;
    const char* activeTextPtr = nullptr;
    uint16_t activeDurationFramesRemaining = 0;
    int16_t activeTextIndex = 0;
    uint32_t text268MirrorPsxAddr = 0;
    const char* text268MirrorPtr = nullptr;
};

struct PrStage1VTextDirectMovieWindowResult {
    bool active = false;
    uint8_t mode = 0xFF;
    uint32_t queryFrame30 = 0;
};

void PrStage1VTextDirectResetSub80024C84(PrStage1VTextDirectRuntime& runtime);

void PrStage1VTextDirectSelectDescSub80024C84(PrStage1VTextDirectRuntime& runtime,
                                             const PrStage1VTextDirectDescInput& desc,
                                             uint8_t languageIndex);

void PrStage1VTextDirectAdvanceSub80024CF8(PrStage1VTextDirectRuntime& runtime,
                                           const PrMovieSubtitles::MovieSubtitleTrack* track,
                                           const PrStage1VTextDirectDescInput& desc,
                                           uint8_t languageIndex,
                                           uint32_t queryFrame30);

bool PrStage1VTextDirectResolveSnapshot(const PrStage1VTextDirectRuntime& runtime,
                                        const PrStage1VTextDirectDescInput& desc,
                                        uint8_t languageIndex,
                                        uint8_t mode,
                                        uint32_t queryFrame30,
                                        uint32_t queryFrame60,
                                        PrStage1CommonLyricsSnapshot& out);

uint32_t PrStage1VTextDirectPackFrame30AsPsxTimecodeKey(uint32_t frame30);
