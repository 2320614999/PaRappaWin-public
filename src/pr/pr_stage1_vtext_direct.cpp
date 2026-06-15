#include "pr_stage1_vtext_direct.h"

namespace {

void ClearStage1VTextActiveSink(PrStage1VTextDirectRuntime& runtime) {
    // sub_80024C84/sub_80024CF8 clear dword_8008ECE4/ctx+0x10C.
    runtime.activeEventFrame = 0;
    runtime.activeTextPsxAddr = 0;
    runtime.activeTextPtr = nullptr;
    runtime.activeTextIndex = 0;
    runtime.text268MirrorPsxAddr = 0;
    runtime.text268MirrorPtr = nullptr;
}

bool HasStage1VTextActiveSink(const PrStage1VTextDirectRuntime& runtime) {
    return runtime.activeTextPtr != nullptr || runtime.activeTextIndex > 0;
}

uint8_t NormalizeStage1VTextLanguageIndex(uint8_t languageIndex) {
    return languageIndex < 5 ? languageIndex : 0u;
}

} // namespace

void PrStage1VTextDirectResetSub80024C84(PrStage1VTextDirectRuntime& runtime) {
    runtime.activeDurationFramesRemaining = 0;
    ClearStage1VTextActiveSink(runtime);
    runtime.eventCursor = 0;
    runtime.resetScratch200 = 0;
    runtime.selectedDescAddr = 0;
    runtime.selectedFirstTextTableAddr = 0;
    runtime.selectedEntriesAddr = 0;
    runtime.selectedEntryCount = 0;
    runtime.selectedTextTableAddr = 0;
    runtime.selectedLanguageIndex = 0;
}

void PrStage1VTextDirectSelectDescSub80024C84(PrStage1VTextDirectRuntime& runtime,
                                             const PrStage1VTextDirectDescInput& desc,
                                             uint8_t languageIndex) {
    PrStage1VTextDirectResetSub80024C84(runtime);
    const uint8_t selectedLanguageIndex =
        NormalizeStage1VTextLanguageIndex(languageIndex);
    runtime.selectedDescAddr = desc.descAddr;
    runtime.selectedFirstTextTableAddr = desc.textTableAddrs[0];
    runtime.selectedEntriesAddr = desc.entriesAddr;
    runtime.selectedEntryCount = desc.entryCount;
    runtime.selectedTextTableAddr = desc.textTableAddrs[selectedLanguageIndex];
    runtime.selectedLanguageIndex = selectedLanguageIndex;
}

void PrStage1VTextDirectAdvanceSub80024CF8(PrStage1VTextDirectRuntime& runtime,
                                           const PrMovieSubtitles::MovieSubtitleTrack* track,
                                           const PrStage1VTextDirectDescInput& desc,
                                           uint8_t languageIndex,
                                           uint32_t queryFrame30) {
    const uint32_t queryTimecodeKey =
        PrStage1VTextDirectPackFrame30AsPsxTimecodeKey(queryFrame30);

    if (!track || !track->loaded) {
        PrStage1VTextDirectResetSub80024C84(runtime);
        return;
    }

    languageIndex = NormalizeStage1VTextLanguageIndex(languageIndex);

    if (runtime.selectedDescAddr != desc.descAddr ||
        runtime.selectedEntriesAddr != desc.entriesAddr ||
        runtime.selectedEntryCount != desc.entryCount ||
        runtime.selectedTextTableAddr != desc.textTableAddrs[languageIndex] ||
        runtime.selectedLanguageIndex != languageIndex) {
        PrStage1VTextDirectSelectDescSub80024C84(runtime, desc, languageIndex);
    }

    if (runtime.activeDurationFramesRemaining > 0) {
        --runtime.activeDurationFramesRemaining;
        if (runtime.activeDurationFramesRemaining == 0) {
            ClearStage1VTextActiveSink(runtime);
        }
    }

    if (runtime.eventCursor >= track->lines.size()) {
        return;
    }

    const PrMovieSubtitles::MovieSubtitleLine& line =
        track->lines[(size_t)runtime.eventCursor];
    const uint32_t eventTimecodeKey =
        PrStage1VTextDirectPackFrame30AsPsxTimecodeKey(line.frame30);
    if (eventTimecodeKey > queryTimecodeKey) {
        return;
    }

    ++runtime.eventCursor;
    const int16_t textIndex = line.textIndex[languageIndex];
    if (textIndex <= 0) {
        // sub_80024CF8 still mirrors the current dword_8008ECE4 into text268.
        runtime.text268MirrorPsxAddr = runtime.activeTextPsxAddr;
        runtime.text268MirrorPtr = runtime.activeTextPtr;
        return;
    }

    runtime.activeEventFrame = line.frame30;
    runtime.activeTextIndex = textIndex;
    runtime.activeDurationFramesRemaining = line.duration;
    runtime.activeTextPsxAddr =
        (runtime.eventCursor - 1u < desc.lineMetaCount && desc.lineMeta != nullptr)
            ? desc.lineMeta[runtime.eventCursor - 1u].textAddrs[languageIndex]
            : 0u;
    runtime.activeTextPtr =
        line.texts[languageIndex].empty() ? nullptr : line.texts[languageIndex].c_str();
    runtime.text268MirrorPsxAddr = runtime.activeTextPsxAddr;
    runtime.text268MirrorPtr = runtime.activeTextPtr;
}

bool PrStage1VTextDirectResolveSnapshot(const PrStage1VTextDirectRuntime& runtime,
                                        const PrStage1VTextDirectDescInput& desc,
                                        uint8_t languageIndex,
                                        uint8_t mode,
                                        uint32_t queryFrame30,
                                        uint32_t queryFrame60,
                                        PrStage1CommonLyricsSnapshot& out) {
    out = PrStage1CommonLyricsSnapshot{};
    out.mode = mode;
    out.queryFrame = queryFrame30;
    out.queryFrame60 = queryFrame60;

    languageIndex = NormalizeStage1VTextLanguageIndex(languageIndex);
    out.producerActive =
        runtime.selectedDescAddr == desc.descAddr &&
        runtime.selectedEntriesAddr == desc.entriesAddr &&
        runtime.selectedEntryCount == desc.entryCount &&
        runtime.selectedTextTableAddr == desc.textTableAddrs[languageIndex] &&
        runtime.selectedLanguageIndex == languageIndex;
    if (!out.producerActive) {
        return false;
    }

    out.eventCursor = runtime.eventCursor;
    out.durationFrames = runtime.activeDurationFramesRemaining;
    out.textIndex = runtime.activeTextIndex;
    if (runtime.activeTextPtr != nullptr) {
        out.text = runtime.activeTextPtr;
    }

    if (!HasStage1VTextActiveSink(runtime)) {
        return false;
    }

    out.valid = true;
    out.eventFrame = runtime.activeEventFrame;
    return true;
}

uint32_t PrStage1VTextDirectPackFrame30AsPsxTimecodeKey(uint32_t frame30) {
    const uint32_t minute = frame30 / 1800u;
    const uint32_t frameInMinute = frame30 % 1800u;
    const uint32_t second = frameInMinute / 30u;
    const uint32_t frame = frameInMinute % 30u;
    return (minute << 16) | (second << 8) | frame;
}
