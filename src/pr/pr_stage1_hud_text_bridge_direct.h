#pragma once

#include "pr_scn1.h"
#include "pr_stage1_live_hud.h"
#include "pr_stage1_overlay_parser.h"

#include <cctype>
#include <cstdint>
#include <string>

namespace PrStage1HudTextBridgeDirect {

struct NumericStatusRuntimeInput {
    bool valid = false;
    int32_t scoreDisplayValue = 0;
    PrStage1RightRankSnapshot rightRankState{};
    bool rightRankFlag0200Pulse = false;
    uint16_t rightRankTransitionAnim18E = 0;
    PrStage1TopLessonPairSnapshot topLessonPairState{};
    bool steadyGameplayGateActive = false;
    bool highLayoutMode = true;
};

struct DisplayGateFromRuntimeSnapshotsInput {
    bool currentSceneIsStage1 = false;
    bool introTransitionActive = false;
    bool subtitleEnabled = false;
    bool overlayScriptSnapshotAvailable = false;
    PrStage1OverlayScriptTextSnapshot overlayScript{};
    PrStage1CommonLyricsSnapshot commonLyrics{};
    bool numericStatusSnapshotAvailable = false;
    PrStage1NumericStatusSnapshot numericStatus{};
};

struct TextRouteFromRuntimeSnapshotsInput {
    const PrStage1OverlayData* overlayData = nullptr;
    PrStage1DisplayGateSnapshot displayGate{};
    bool overlayScriptSnapshotAvailable = false;
    PrStage1OverlayScriptTextSnapshot overlayScript{};
    PrStage1CommonLyricsSnapshot commonLyrics{};
};

inline std::string NormalizeHudText(const std::string& text) {
    std::string out;
    out.reserve(text.size());

    bool pendingSpace = false;
    bool atLineStart = true;
    for (unsigned char c : text) {
        if (c == '\r') {
            continue;
        }
        if (c == '\n') {
            while (!out.empty() && out.back() == ' ') {
                out.pop_back();
            }
            if (!out.empty() && out.back() != '\n') {
                out.push_back('\n');
            }
            pendingSpace = false;
            atLineStart = true;
            continue;
        }
        if (std::isspace(static_cast<int>(c)) != 0) {
            if (!atLineStart) {
                pendingSpace = true;
            }
            continue;
        }

        if (pendingSpace && !out.empty() && out.back() != '\n') {
            out.push_back(' ');
        }
        out.push_back((char)c);
        pendingSpace = false;
        atLineStart = false;
    }

    while (!out.empty() && (out.back() == ' ' || out.back() == '\n')) {
        out.pop_back();
    }
    return out;
}

inline std::string NormalizeHudText(const char* text) {
    if (text == nullptr) {
        return std::string{};
    }
    return NormalizeHudText(std::string(text));
}

inline PrStage1NumericStatusSnapshot BuildNumericStatusSnapshotFromRuntimeState(
    const NumericStatusRuntimeInput& input) {
    PrStage1NumericStatusSnapshot out{};
    if (!input.valid) {
        return out;
    }

    out.valid = true;
    out.scoreDisplayState.scoreDisplayValue = input.scoreDisplayValue;
    out.rightRankState = input.rightRankState;
    out.rightRankFlag0200Pulse = input.rightRankFlag0200Pulse;
    out.rightRankTransitionAnim18E = input.rightRankTransitionAnim18E;
    out.topLessonPairState = input.topLessonPairState;
    out.steadyGameplayGateActive = input.steadyGameplayGateActive;
    out.highLayoutMode = input.highLayoutMode;
    return out;
}

inline bool BuildDisplayGateSnapshotFromRuntimeSnapshots(
    const DisplayGateFromRuntimeSnapshotsInput& input,
    PrStage1DisplayGateSnapshot& out) {
    out = PrStage1DisplayGateSnapshot{};
    if (!input.currentSceneIsStage1) {
        return false;
    }

    out.valid = true;
    out.subtitleHighLayout = input.subtitleEnabled;

    if (input.overlayScriptSnapshotAvailable) {
        out.directScriptBoxEnabled =
            !input.introTransitionActive &&
            input.overlayScript.directScriptBoxPermit4E;
    }

    out.commonLyricVisible = input.subtitleEnabled && input.commonLyrics.valid;
    out.commonLyricMuteGate = out.directScriptBoxEnabled;

    if (input.numericStatusSnapshotAvailable && input.numericStatus.valid) {
        out.numericStatusHudEnabled =
            input.numericStatus.steadyGameplayGateActive;
        out.subtitleHighLayout = input.numericStatus.highLayoutMode;
        out.lessonVisible =
            input.numericStatus.topLessonPairState.topLessonPairChangeVisible;
        out.lessonSelector =
            input.numericStatus.topLessonPairState.topLessonPairLessonId;
        out.rightRankBlinkVisible =
            input.numericStatus.rightRankState.rightRankBlinkEnabled;
        out.rightRankBlinkTarget =
            input.numericStatus.rightRankState.rightRankBlinkTargetRow;
    }

    return true;
}

inline PrStage1LiveHud::Stage1ResolvedHudTextVisual
BuildSteadyGameplayScriptTextVisual(const TextRouteFromRuntimeSnapshotsInput& input) {
    PrStage1LiveHud::Stage1ResolvedHudTextVisual out{};
    out.sourceKind =
        PrStage1LiveHud::Stage1ResolvedHudTextSourceKind::OverlayScriptTextRuntimeSnapshot;
    out.pathKind =
        PrStage1LiveHud::Stage1ResolvedHudTextPathKind::SteadyGameplayScriptText;

    out.available = input.displayGate.directScriptBoxEnabled;
    if (!out.available || !input.overlayScriptSnapshotAvailable) {
        return out;
    }

    const PrStage1OverlayScriptTextSnapshot& snapshot = input.overlayScript;
    if (snapshot.activeTextMirrorPtr == nullptr ||
        snapshot.activeTextMirrorPtr[0] == '\0' ||
        snapshot.activeTimeoutFramesRemaining == 0) {
        return out;
    }

    const std::string normalizedText = NormalizeHudText(snapshot.activeTextMirrorPtr);
    if (normalizedText.empty()) {
        return out;
    }

    out.hasText = true;
    out.rawText = normalizedText;
    return out;
}

inline PrStage1LiveHud::Stage1ResolvedHudTextVisual
BuildCommonLyricsTextVisual(const TextRouteFromRuntimeSnapshotsInput& input) {
    PrStage1LiveHud::Stage1ResolvedHudTextVisual out{};
    out.sourceKind =
        PrStage1LiveHud::Stage1ResolvedHudTextSourceKind::CommonLyricsRuntimeSnapshot;
    out.pathKind =
        PrStage1LiveHud::Stage1ResolvedHudTextPathKind::GenericLyricsBox;

    out.available =
        input.displayGate.commonLyricVisible &&
        !input.displayGate.commonLyricMuteGate;
    if (!out.available || !input.commonLyrics.valid) {
        return out;
    }
    out.commonLyricsFrameSub8001C864 =
        input.commonLyrics.frameSub8001C864;

    if (input.commonLyrics.text == nullptr ||
        input.commonLyrics.text[0] == '\0') {
        return out;
    }

    const std::string normalizedText =
        NormalizeHudText(input.commonLyrics.text);
    if (normalizedText.empty()) {
        return out;
    }

    out.hasText = true;
    out.rawText = normalizedText;
    return out;
}

inline PrStage1LiveHud::Stage1ResolvedHudTextRouteState
BuildTextRouteStateFromRuntimeSnapshots(const TextRouteFromRuntimeSnapshotsInput& input) {
    PrStage1LiveHud::Stage1ResolvedHudTextRouteState out{};
    out.steadyGameplayScript = BuildSteadyGameplayScriptTextVisual(input);
    out.commonLyrics = BuildCommonLyricsTextVisual(input);

    const bool steadyHasText =
        out.steadyGameplayScript.hasText &&
        !out.steadyGameplayScript.rawText.empty();
    if ((input.displayGate.directScriptBoxEnabled || steadyHasText) &&
        out.steadyGameplayScript.sourceKind ==
            PrStage1LiveHud::Stage1ResolvedHudTextSourceKind::
                OverlayScriptTextRuntimeSnapshot) {
        out.active = out.steadyGameplayScript;
    } else if (input.displayGate.commonLyricVisible &&
               !input.displayGate.commonLyricMuteGate &&
               out.commonLyrics.sourceKind ==
                   PrStage1LiveHud::Stage1ResolvedHudTextSourceKind::
                       CommonLyricsRuntimeSnapshot) {
        out.active = out.commonLyrics;
    }

    out.anyAvailable =
        out.steadyGameplayScript.available || out.commonLyrics.available;
    return out;
}

inline PrStage1LiveHud::Stage1ResolvedHudScoreDisplayState
BuildResolvedScoreDisplayState(const PrStage1NumericStatusSnapshot& snapshot) {
    PrStage1LiveHud::Stage1ResolvedHudScoreDisplayState out{};
    out.scoreDisplayValue = snapshot.scoreDisplayState.scoreDisplayValue;
    return out;
}

inline PrStage1LiveHud::Stage1ResolvedHudRightRankState
BuildResolvedRightRankState(const PrStage1NumericStatusSnapshot& snapshot,
                            const PrStage1DisplayGateSnapshot& displayGate) {
    PrStage1LiveHud::Stage1ResolvedHudRightRankState out{};
    out.rightRankActiveRow = snapshot.rightRankState.rightRankActiveRow;
    out.rightRankBlinkTargetRow = displayGate.rightRankBlinkTarget;
    out.rightRankBlinkEnabled = displayGate.rightRankBlinkVisible;
    out.rightRankFlag0200Pulse = snapshot.rightRankFlag0200Pulse;
    out.rightRankTransitionAnim18E = snapshot.rightRankTransitionAnim18E;
    return out;
}

inline PrStage1LiveHud::Stage1ResolvedHudTopLessonPairState
BuildResolvedTopLessonPairState(const PrStage1DisplayGateSnapshot& displayGate) {
    PrStage1LiveHud::Stage1ResolvedHudTopLessonPairState out{};
    out.topLessonPairChangeVisible = displayGate.lessonVisible;
    out.topLessonPairLessonId = displayGate.lessonSelector;
    return out;
}

inline PrStage1LiveHud::Stage1ResolvedHudNumericStatusRouteState
BuildNumericStatusRouteStateFromRuntimeSnapshot(
    const PrStage1NumericStatusSnapshot& snapshot,
    const PrStage1DisplayGateSnapshot& displayGate) {
    PrStage1LiveHud::Stage1ResolvedHudNumericStatusRouteState out{};
    if (!snapshot.valid) {
        return out;
    }

    out.available = true;
    out.scoreDisplayState = BuildResolvedScoreDisplayState(snapshot);
    out.rightRankState = BuildResolvedRightRankState(snapshot, displayGate);
    out.topLessonPairState = BuildResolvedTopLessonPairState(displayGate);
    out.steadyGameplayGateActive = displayGate.numericStatusHudEnabled;
    out.highLayoutMode = displayGate.subtitleHighLayout;
    return out;
}

}  // namespace PrStage1HudTextBridgeDirect
