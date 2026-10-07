#include "pr_ss0_directory_dispatcher_direct.h"

namespace PrSS0DirectoryDispatcherDirect {
namespace {

static bool Append(DispatcherActionList& list, const DispatcherAction& action)
{
    if (list.count >= sizeof(list.actions) / sizeof(list.actions[0])) {
        list.truncated = true;
        return false;
    }
    list.actions[list.count++] = action;
    return true;
}

static void AppendAction(DispatcherActionList& list,
                         DispatcherActionKind kind,
                         uint32_t psxFunction,
                         int32_t arg0 = 0,
                         int32_t arg1 = 0,
                         int32_t arg2 = 0,
                         int32_t arg3 = 0)
{
    DispatcherAction action{};
    action.kind = kind;
    action.psxFunction = psxFunction;
    action.arg0 = arg0;
    action.arg1 = arg1;
    action.arg2 = arg2;
    action.arg3 = arg3;
    (void)Append(list, action);
}

static int32_t PackI16Pair(int32_t hi, int32_t lo)
{
    return static_cast<int32_t>(
        ((static_cast<uint32_t>(hi) & 0xFFFFu) << 16) |
        (static_cast<uint32_t>(lo) & 0xFFFFu));
}

static bool IsNextPad(uint32_t padMask)
{
    return padMask == kPadRight || padMask == kPadDown;
}

static bool IsPrevPad(uint32_t padMask)
{
    return padMask == kPadLeft || padMask == kPadUp;
}

static int32_t WrapCursor(int32_t cursor, int32_t count)
{
    if (count <= 0) {
        return 0;
    }
    while (cursor < 0) {
        cursor += count;
    }
    while (cursor >= count) {
        cursor -= count;
    }
    return cursor;
}

static int32_t CurrentMainMenuItemValue(const MainMenuState800264AC& state)
{
    if (state.cursor < 0 || state.cursor >= 5) {
        return 0;
    }
    return state.itemValue[state.cursor];
}

static int32_t PackStageEnabledMask(const StageSelectState80025F6C& state)
{
    int32_t mask = 0;
    const int32_t limit = state.count < 8 ? state.count : 8;
    for (int32_t cursor = 1; cursor <= limit; ++cursor) {
        if (state.enabled[cursor]) {
            mask |= 1 << cursor;
        }
    }
    return mask;
}

static int32_t MoveStageCursor(const StageSelectState80025F6C& state,
                               int32_t delta)
{
    int32_t cursor = state.cursor;
    for (int32_t i = 0; i < state.count; ++i) {
        cursor += delta;
        if (cursor < 1) {
            cursor = state.count;
        } else if (cursor > state.count) {
            cursor = 1;
        }
        if (state.enabled[cursor]) {
            return cursor;
        }
    }
    return state.cursor;
}

static bool IsKnownMainLoopResult80015788(int32_t result)
{
    switch (result) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 6:
    case 7:
    case 8:
        return true;
    default:
        return false;
    }
}

} // namespace

bool RuntimeCutoverAllowed()
{
    return false;
}

BlinkTickResult80025D70 TickBlink80025D70(
    int32_t counter800916E4,
    int32_t contextBlinkOnOff)
{
    BlinkTickResult80025D70 out{};
    out.counter800916E4 = counter800916E4;
    out.contextBlinkOnOff = contextBlinkOnOff;

    if (counter800916E4 == 0) {
        out.counter800916E4 = 1;
        const uint32_t toggled =
            static_cast<uint32_t>(contextBlinkOnOff) ^ 1u;
        out.contextBlinkOnOff = toggled != 0u ? 1 : 0;
        out.toggled = true;
    } else if (counter800916E4 < 19) {
        out.counter800916E4 = counter800916E4 + 1;
    } else {
        out.counter800916E4 = 0;
    }
    return out;
}

HiScoreEvent6State800267E4 InitHiScoreEvent6State800267E4(
    const HiScoreEvent6InitInput800267E4& in)
{
    HiScoreEvent6State800267E4 state{};
    if (!in.ctxKnown || in.ctxAddress != kEvent6HiScoreCtx80049278) {
        return state;
    }

    // 800267E4 replaces event-table +0x10 with the caller argument and clears
    // only arg+0x04. arg+0x00 is the persistent 80025D70 blink state and must
    // survive a later Event6 re-entry in the same Scene0 overlay lifetime.
    state.requestBound = true;
    state.ctxAddress = in.ctxAddress;
    state.exitIconStateCtx00 = in.exitIconStateCtx00;
    state.exitLabelStateCtx04 = 0;
    return state;
}

HiScoreEvent6HandleResult80025E0C HandleHiScoreEvent6Input80025E0C(
    const HiScoreEvent6State800267E4& in,
    uint32_t padMask80026744)
{
    HiScoreEvent6HandleResult80025E0C out{};
    out.state = in;
    AppendAction(out.actions,
                 DispatcherActionKind::GateEvent6InputSource80025E0C,
                 kFn80025E0C,
                 static_cast<int32_t>(padMask80026744),
                 in.requestBound ? 1 : 0,
                 static_cast<int32_t>(in.ctxAddress),
                 static_cast<int32_t>(kEvent6HiScoreCtx80049278));
    if (!in.requestBound || in.ctxAddress != kEvent6HiScoreCtx80049278) {
        AppendAction(out.actions,
                     DispatcherActionKind::Gap,
                     kFn80025E0C,
                     static_cast<int32_t>(padMask80026744));
        return out;
    }
    if (padMask80026744 != kPadCross) {
        return out;
    }

    // Exact 80025E0C order: ctx+4=1, 80025C8C(0x20), return 1.
    out.state.exitLabelStateCtx04 = 1;
    AppendAction(out.actions,
                 DispatcherActionKind::SetEvent6ExitLabelCtx04,
                 kFn80025E0C,
                 1);
    out.cueCode80025C8C = kEvent6ActionCue80025E0C;
    AppendAction(out.actions,
                 DispatcherActionKind::CallInputSfx80025C8C,
                 kFn80025C8C,
                 static_cast<int32_t>(out.cueCode80025C8C));
    out.result = 1;
    AppendAction(out.actions,
                 DispatcherActionKind::ReturnResult,
                 kFn80025E0C,
                 out.result);
    return out;
}

HiScoreEvent6TickResult80025E48 TickHiScoreEvent6Blink80025E48(
    const HiScoreEvent6State800267E4& in,
    int32_t counter800916E4)
{
    HiScoreEvent6TickResult80025E48 out{};
    out.state = in;
    if (!in.requestBound || in.ctxAddress != kEvent6HiScoreCtx80049278) {
        return out;
    }

    out.accepted = true;
    out.blink = TickBlink80025D70(counter800916E4,
                                  in.exitIconStateCtx00);
    out.state.exitIconStateCtx00 = out.blink.contextBlinkOnOff;
    return out;
}

MainMenuState800264AC InitMainMenuState80026794(
    int32_t persistedDifficulty800544F8)
{
    MainMenuState800264AC state{};
    state.cursor = 0;
    state.count = 5;
    // Native 80026794 does not clear state+0x18 (itemValue[2]). It reads
    // that persistent NORMAL/EASY word and writes word_800916DA before the
    // first event-3 frame. Keep the direct port's state and carrier in sync
    // even when MENU is re-entered from a transition or Scene1 return.
    state.itemValue[2] = persistedDifficulty800544F8 != 0 ? 1 : 0;
    state.word800916DA = state.itemValue[2];
    return state;
}

MainMenuHandleResult800264AC HandleMainMenu800264AC(
    const MainMenuState800264AC& in,
    uint32_t padMask)
{
    MainMenuHandleResult800264AC out{};
    out.state = in;
    bool actionAccepted = false;

    AppendAction(out.actions,
                 DispatcherActionKind::GateMainMenuInputSource800264AC,
                 kFn800264AC,
                 in.cursor,
                 static_cast<int32_t>(padMask),
                 in.count,
                 PackI16Pair(CurrentMainMenuItemValue(in),
                             in.word800916DA));

    if (IsNextPad(padMask)) {
        const int32_t previous = out.state.cursor;
        out.state.cursor = WrapCursor(out.state.cursor + 1, out.state.count);
        AppendAction(out.actions,
                     DispatcherActionKind::MoveCursor,
                     kFn800264AC,
                     previous,
                     out.state.cursor);
        out.cueCode80025C8C = kMainMenuNavigateCue800264AC;
        AppendAction(out.actions,
                     DispatcherActionKind::CallInputSfx80025C8C,
                     kFn80025C8C,
                     static_cast<int32_t>(out.cueCode80025C8C));
        return out;
    }

    if (IsPrevPad(padMask)) {
        const int32_t previous = out.state.cursor;
        out.state.cursor = WrapCursor(out.state.cursor - 1, out.state.count);
        AppendAction(out.actions,
                     DispatcherActionKind::MoveCursor,
                     kFn800264AC,
                     previous,
                     out.state.cursor);
        out.cueCode80025C8C = kMainMenuNavigateCue800264AC;
        AppendAction(out.actions,
                     DispatcherActionKind::CallInputSfx80025C8C,
                     kFn80025C8C,
                     static_cast<int32_t>(out.cueCode80025C8C));
        return out;
    }

    const int32_t cursor = out.state.cursor;
    if (cursor == 0 && padMask == kPadCross) {
        out.state.itemValue[0] = 1;
        out.result = 8;
        actionAccepted = true;
        AppendAction(out.actions, DispatcherActionKind::SetItemValue,
                     kFn800264AC, 0, 1);
    } else if (cursor == 1 && padMask == kPadCross) {
        out.state.itemValue[1] = 0;
        out.result = 1;
        actionAccepted = true;
        AppendAction(out.actions, DispatcherActionKind::SetItemValue,
                     kFn800264AC, 1, 0);
    } else if (cursor == 2 && padMask == kPadCross) {
        out.state.itemValue[2] = 0;
        out.state.word800916DA = 0;
        actionAccepted = true;
        AppendAction(out.actions, DispatcherActionKind::SetItemValue,
                     kFn800264AC, 2, 0);
        AppendAction(out.actions, DispatcherActionKind::SetWord800916DA,
                     kFn800264AC, 0);
    } else if (cursor == 2 && padMask == kPadCircle) {
        out.state.itemValue[2] = 1;
        out.state.word800916DA = 1;
        actionAccepted = true;
        AppendAction(out.actions, DispatcherActionKind::SetItemValue,
                     kFn800264AC, 2, 1);
        AppendAction(out.actions, DispatcherActionKind::SetWord800916DA,
                     kFn800264AC, 1);
    } else if (cursor == 3 && padMask == kPadSquare) {
        out.state.itemValue[3] = 0;
        out.result = 3;
        actionAccepted = true;
        AppendAction(out.actions, DispatcherActionKind::SetItemValue,
                     kFn800264AC, 3, 0);
    } else if (cursor == 3 && padMask == kPadCross) {
        out.state.itemValue[3] = 1;
        out.result = 4;
        actionAccepted = true;
        AppendAction(out.actions, DispatcherActionKind::SetItemValue,
                     kFn800264AC, 3, 1);
    } else if (cursor == 3 && padMask == kPadCircle) {
        out.state.itemValue[3] = 2;
        out.result = 2;
        actionAccepted = true;
        AppendAction(out.actions, DispatcherActionKind::SetItemValue,
                     kFn800264AC, 3, 2);
    } else if (cursor == 3 && padMask == kPadTriangle) {
        out.state.itemValue[3] = 3;
        out.result = 6;
        actionAccepted = true;
        AppendAction(out.actions, DispatcherActionKind::SetItemValue,
                     kFn800264AC, 3, 3);
    } else if (cursor == 4 && padMask == kPadCross) {
        out.state.itemValue[4] = 0;
        out.state.doneFlag = true;
        out.result = 7;
        actionAccepted = true;
        AppendAction(out.actions, DispatcherActionKind::SetItemValue,
                     kFn800264AC, 4, 0);
        AppendAction(out.actions, DispatcherActionKind::SetDoneFlag,
                     kFn800264AC, 1);
    }

    if (actionAccepted) {
        out.cueCode80025C8C = kMainMenuActionCue800264AC;
        AppendAction(out.actions,
                     DispatcherActionKind::CallInputSfx80025C8C,
                     kFn80025C8C,
                     static_cast<int32_t>(out.cueCode80025C8C));
    }
    if (out.result != 0) {
        AppendAction(out.actions, DispatcherActionKind::ReturnResult,
                     kFn800264AC, out.result);
    }
    return out;
}

StageSelectState80025F6C InitStageSelect800267F8(
    const StageSelectInitInput800267F8& in)
{
    StageSelectState80025F6C state{};
    state.cursor = 1;
    state.count = 8;

    if (in.word800916F0Known && in.word800916F0 == 1) {
        // 800267F8 writes ctx+0x0E..ctx+0x1A after first filling the
        // complete ctx+0x0C..ctx+0x1C enable-word bank with one. The stage-5
        // and stage-6 status words are then replaced with 2 and 3.
        constexpr uint8_t kForcedStatusCtx0ETo1A[7] = {
            1u, 1u, 1u, 1u, 2u, 3u, 1u,
        };
        for (int32_t cursor = 1; cursor <= 7; ++cursor) {
            state.rawStatus80092F1DTo23[cursor - 1] =
                kForcedStatusCtx0ETo1A[cursor - 1];
            state.enabled[cursor] = true;
        }
    } else if (in.word800916DA == 1) {
        // The records branch materializes a new context bank rather than
        // retaining the source status bytes: only cursor 1 remains enabled.
        state.rawStatus80092F1DTo23[0] = 1u;
        state.enabled[1] = true;
    } else {
        uint32_t completedStatusCount = 0u;
        for (int32_t cursor = 1; cursor <= 7; ++cursor) {
            state.rawStatus80092F1DTo23[cursor - 1] =
                in.status80092F1DTo23[cursor - 1];
            if (in.status80092F1DTo23[cursor - 1] == 3u) {
                ++completedStatusCount;
            }
            state.enabled[cursor] =
                cursor <= 6 && in.status80092F1DTo23[cursor - 1] != 0;
        }
        if (in.word800916F0Known) {
            // Native 800267F8 does not retain the source value at ctx+0x1A.
            // It replaces the final slot with the all-clear latch after
            // counting the seven status words (BONUS opens when >= 6 are 3).
            state.rawStatus80092F1DTo23[6] =
                completedStatusCount >= 6u ? 1u : 0u;
            state.enabled[7] = state.rawStatus80092F1DTo23[6] != 0u;
        }
    }

    state.enabled[8] = true;
    if (!state.enabled[state.cursor]) {
        state.cursor = MoveStageCursor(state, 1);
    }
    return state;
}

StageSelectHandleResult80025F6C HandleStageSelect80025F6C(
    const StageSelectState80025F6C& in,
    uint32_t padMask)
{
    StageSelectHandleResult80025F6C out{};
    out.state = in;

    AppendAction(out.actions,
                 DispatcherActionKind::GateStageSelectInputSource80025F6C,
                 kFn80025F6C,
                 in.cursor,
                 static_cast<int32_t>(padMask),
                 in.count,
                 PackStageEnabledMask(in));

    if (IsNextPad(padMask)) {
        const int32_t previous = out.state.cursor;
        out.cueCode80025C8C = kStageSelectNavigateCue80025F6C;
        out.cueOrder = StageSelectCueOrder80025F6C::BeforeStateCommit;
        AppendAction(out.actions,
                     DispatcherActionKind::CallInputSfx80025C8C,
                     kFn80025C8C,
                     static_cast<int32_t>(out.cueCode80025C8C));
        out.state.cursor = MoveStageCursor(out.state, 1);
        AppendAction(out.actions,
                     DispatcherActionKind::MoveCursor,
                     kFn80025F6C,
                     previous,
                     out.state.cursor);
        return out;
    }

    if (IsPrevPad(padMask)) {
        const int32_t previous = out.state.cursor;
        out.cueCode80025C8C = kStageSelectNavigateCue80025F6C;
        out.cueOrder = StageSelectCueOrder80025F6C::BeforeStateCommit;
        AppendAction(out.actions,
                     DispatcherActionKind::CallInputSfx80025C8C,
                     kFn80025C8C,
                     static_cast<int32_t>(out.cueCode80025C8C));
        out.state.cursor = MoveStageCursor(out.state, -1);
        AppendAction(out.actions,
                     DispatcherActionKind::MoveCursor,
                     kFn80025F6C,
                     previous,
                     out.state.cursor);
        return out;
    }

    if (padMask == kPadCross) {
        const int32_t cursor = out.state.cursor;
        const bool cursorEnabled =
            cursor >= 1 && cursor <= out.state.count &&
            out.state.enabled[cursor];
        if (!cursorEnabled) {
            AppendAction(out.actions,
                         DispatcherActionKind::Gap,
                         kFn80025F6C,
                         cursor,
                         0,
                         0,
                         PackStageEnabledMask(out.state));
            return out;
        }
        if (cursor >= 1 && cursor <= 7) {
            out.cueCode80025C8C = kStageSelectActionCue80025F6C;
            out.cueOrder = StageSelectCueOrder80025F6C::BeforeStateCommit;
            AppendAction(out.actions,
                         DispatcherActionKind::CallInputSfx80025C8C,
                         kFn80025C8C,
                         static_cast<int32_t>(out.cueCode80025C8C));
            out.state.doneFlag = cursor == 7;
            out.state.outScene = cursor <= 6 ? cursor : 8;
            out.result = 1;
            AppendAction(out.actions,
                         DispatcherActionKind::GateStageSelectOutScene80025F6C,
                         kFn80025F6C,
                         cursor,
                         out.state.outScene,
                         out.result,
                         cursor <= 6 ? 6 : 8);
            if (out.state.doneFlag) {
                AppendAction(out.actions,
                             DispatcherActionKind::SetDoneFlag,
                             kFn80025F6C,
                             1);
            }
            AppendAction(out.actions,
                         DispatcherActionKind::SetOutScene,
                         kFn80025F6C,
                         out.state.outScene);
        } else if (cursor == 8) {
            out.state.doneFlag = true;
            out.result = 2;
            AppendAction(out.actions,
                         DispatcherActionKind::SetDoneFlag,
                         kFn80025F6C,
                         1);
            out.cueCode80025C8C = kStageSelectActionCue80025F6C;
            out.cueOrder = StageSelectCueOrder80025F6C::AfterStateCommit;
            AppendAction(out.actions,
                         DispatcherActionKind::CallInputSfx80025C8C,
                         kFn80025C8C,
                         static_cast<int32_t>(out.cueCode80025C8C));
            AppendAction(out.actions,
                         DispatcherActionKind::GateStageSelectOutScene80025F6C,
                         kFn80025F6C,
                         cursor,
                         -1,
                         out.result,
                         0);
        }
    }

    if (out.result != 0) {
        AppendAction(out.actions, DispatcherActionKind::ReturnResult,
                     kFn80025F6C, out.result);
    }
    return out;
}

OptionsState80026910 InitOptions800268E4(int32_t word800916D8,
                                         int32_t word800916DC)
{
    OptionsState80026910 state{};
    state.cursor = 1;
    state.count = 3;
    state.word800916D8 = word800916D8;
    state.word800916DC = word800916DC;
    state.word801C36A6 = word800916D8;
    state.opt0Value = word800916DC == 0 ? 1 : 0;
    state.opt1Value = word800916D8;
    return state;
}

OptionsHandleResult80026910 HandleOptions80026910(
    const OptionsState80026910& in,
    uint32_t padMask)
{
    OptionsHandleResult80026910 out{};
    out.state = in;

    AppendAction(out.actions,
                 DispatcherActionKind::GateOptionsInputSource80026910,
                 kFn80026910,
                 in.cursor,
                 static_cast<int32_t>(padMask),
                 in.count,
                 PackI16Pair(in.word800916D8, in.word800916DC));

    // Original 80026910 rejects the whole handler while ctx+24 is positive.
    // The separate table tick 80026B54 owns the decrement.
    if (in.cooldown > 0) {
        return out;
    }

    if (out.state.cursor == 0 && padMask == kPadCross) {
        out.cueKind = OptionsCueKind80026910::Input80025C8C;
        out.cueCode80025C8C = kOptionsActionCue80026910;
        AppendAction(out.actions, DispatcherActionKind::CallInputSfx80025C8C,
                     kFn80025C8C, out.cueCode80025C8C);
        out.state.word800916DC = 1;
        out.state.opt0Value = 0;
        AppendAction(out.actions, DispatcherActionKind::SetItemValue,
                     kFn80026910, 0, out.state.opt0Value);
        AppendAction(out.actions, DispatcherActionKind::SetWord800916DC,
                     kFn80026910, 1);
    } else if (out.state.cursor == 0 && padMask == kPadCircle) {
        out.cueKind = OptionsCueKind80026910::Input80025C8C;
        out.cueCode80025C8C = kOptionsActionCue80026910;
        AppendAction(out.actions, DispatcherActionKind::CallInputSfx80025C8C,
                     kFn80025C8C, out.cueCode80025C8C);
        out.state.word800916DC = 0;
        out.state.opt0Value = 1;
        AppendAction(out.actions, DispatcherActionKind::SetItemValue,
                     kFn80026910, 0, out.state.opt0Value);
        AppendAction(out.actions, DispatcherActionKind::SetWord800916DC,
                     kFn80026910, 0);
    } else if (out.state.cursor == 1) {
        const int32_t previous = out.state.word800916D8;
        // 80026910 snapshots ctx+0x10 into ctx+0x1C before deciding whether
        // this cursor-1 input owns the dedicated language cue.
        out.state.languageBeforeInputSnapshotCtx1C = previous;
        out.languageSnapshotWrittenBeforeCue = true;
        AppendAction(
            out.actions,
            DispatcherActionKind::SetOptionsLanguageSnapshotCtx1C80026910,
            kFn80026910,
            previous);
        if (padMask == kPadRight || padMask == kPadLeft) {
            out.cueKind = OptionsCueKind80026910::Language80025DBC;
            AppendAction(out.actions,
                         DispatcherActionKind::CallOptionsLanguageSfx80025DBC,
                         kFn80025DBC);
            const int32_t delta = padMask == kPadRight ? 1 : -1;
            out.state.word800916D8 = WrapCursor(previous + delta, 5);
        }
        out.state.opt1Value = out.state.word800916D8;
        out.state.word801C36A6 = out.state.word800916D8;
        if (out.state.languageBeforeInputSnapshotCtx1C !=
            out.state.word800916D8) {
            out.state.cooldown = 16;
            AppendAction(out.actions, DispatcherActionKind::SetCooldown,
                         kFn80026910, out.state.cooldown);
        }
        AppendAction(out.actions, DispatcherActionKind::SetItemValue,
                     kFn80026910, 1, out.state.opt1Value);
        AppendAction(out.actions, DispatcherActionKind::SetWord800916D8,
                     kFn80026910, out.state.word800916D8);
        AppendAction(out.actions, DispatcherActionKind::SetWord801C36A6,
                     kFn80026910, out.state.word801C36A6);
    } else if (out.state.cursor == 2 && padMask == kPadCross) {
        out.cueKind = OptionsCueKind80026910::Input80025C8C;
        out.cueCode80025C8C = kOptionsActionCue80026910;
        AppendAction(out.actions, DispatcherActionKind::CallInputSfx80025C8C,
                     kFn80025C8C, out.cueCode80025C8C);
        out.state.doneFlag = true;
        out.state.opt2Value = 0;
        out.result = 1;
        AppendAction(out.actions, DispatcherActionKind::SetItemValue,
                     kFn80026910, 2, out.state.opt2Value);
        AppendAction(out.actions, DispatcherActionKind::SetDoneFlag,
                     kFn80026910, 1);
        AppendAction(out.actions, DispatcherActionKind::ReturnResult,
                     kFn80026910, out.result);
        return out;
    }

    if (padMask == kPadDown || padMask == kPadUp) {
        out.cueKind = OptionsCueKind80026910::Input80025C8C;
        out.cueCode80025C8C = kOptionsNavigateCue80026910;
        AppendAction(out.actions, DispatcherActionKind::CallInputSfx80025C8C,
                     kFn80025C8C, out.cueCode80025C8C);
        const int32_t delta = padMask == kPadDown ? 1 : -1;
        const int32_t previous = out.state.cursor;
        out.state.cursor = WrapCursor(out.state.cursor + delta,
                                      out.state.count);
        AppendAction(out.actions, DispatcherActionKind::MoveCursor,
                     kFn80026910, previous, out.state.cursor);
    }

    return out;
}

OptionsTickResult80026B54 TickOptions80026B54(
    const OptionsState80026910& in,
    int32_t counter800916E4,
    int32_t contextBlinkOnOff)
{
    OptionsTickResult80026B54 out{};
    out.state = in;
    if (out.state.cooldown > 0) {
        --out.state.cooldown;
        out.cooldownDecremented = true;
    }
    out.blink = TickBlink80025D70(counter800916E4, contextBlinkOnOff);
    return out;
}

MainLoopConsumeResult80015788 ConsumeMainMenuResult80015788(int32_t result)
{
    MainLoopConsumeResult80015788 out{};
    AppendAction(out.actions,
                 DispatcherActionKind::GateMainLoopResultSource80015788,
                 kFn80015788,
                 result,
                 IsKnownMainLoopResult80015788(result) ? 1 : 0);
    switch (result) {
    case 0:
        out.action = MainLoopActionKind::ContinueLoop;
        break;
    case 1:
        out.action = MainLoopActionKind::HiScoreRecordsPage;
        out.nextEventId = 6;
        out.blockedByP0Gap = true;
        break;
    case 2:
        out.action = MainLoopActionKind::ReplayLoad;
        out.writesWord800916D0 = true;
        out.word800916D0 = 2;
        out.blockedByP0Gap = true;
        break;
    case 3:
        out.action = MainLoopActionKind::Practice;
        out.blockedByP0Gap = true;
        break;
    case 4:
        out.action = MainLoopActionKind::StageSelect;
        out.nextEventId = 2;
        break;
    case 6:
        out.action = MainLoopActionKind::LoadCard;
        out.blockedByP0Gap = true;
        break;
    case 7:
        out.action = MainLoopActionKind::ReturnScene0;
        out.returnsScene = true;
        out.returnScene = 0;
        break;
    case 8:
        out.action = MainLoopActionKind::Options;
        out.nextEventId = 17;
        break;
    default:
        out.action = MainLoopActionKind::Gap;
        out.blockedByP0Gap = true;
        break;
    }
    return out;
}

const char* DispatcherActionKindName(DispatcherActionKind kind)
{
    switch (kind) {
    case DispatcherActionKind::None:
        return "None";
    case DispatcherActionKind::GateMainMenuInputSource800264AC:
        return "GateMainMenuInputSource800264AC";
    case DispatcherActionKind::GateStageSelectInputSource80025F6C:
        return "GateStageSelectInputSource80025F6C";
    case DispatcherActionKind::GateStageSelectOutScene80025F6C:
        return "GateStageSelectOutScene80025F6C";
    case DispatcherActionKind::GateOptionsInputSource80026910:
        return "GateOptionsInputSource80026910";
    case DispatcherActionKind::GateEvent6InputSource80025E0C:
        return "GateEvent6InputSource80025E0C";
    case DispatcherActionKind::GateMainLoopResultSource80015788:
        return "GateMainLoopResultSource80015788";
    case DispatcherActionKind::CallInputSfx80025C8C:
        return "CallInputSfx80025C8C";
    case DispatcherActionKind::MoveCursor:
        return "MoveCursor";
    case DispatcherActionKind::SetItemValue:
        return "SetItemValue";
    case DispatcherActionKind::SetWord800916DA:
        return "SetWord800916DA";
    case DispatcherActionKind::SetWord800916D8:
        return "SetWord800916D8";
    case DispatcherActionKind::SetWord800916DC:
        return "SetWord800916DC";
    case DispatcherActionKind::SetWord801C36A6:
        return "SetWord801C36A6";
    case DispatcherActionKind::SetCooldown:
        return "SetCooldown";
    case DispatcherActionKind::SetDoneFlag:
        return "SetDoneFlag";
    case DispatcherActionKind::SetEvent6ExitLabelCtx04:
        return "SetEvent6ExitLabelCtx04";
    case DispatcherActionKind::SetOutScene:
        return "SetOutScene";
    case DispatcherActionKind::ReturnResult:
        return "ReturnResult";
    case DispatcherActionKind::Gap:
        return "Gap";
    }
    return "Unknown";
}

const char* MainLoopActionKindName(MainLoopActionKind kind)
{
    switch (kind) {
    case MainLoopActionKind::ContinueLoop:
        return "ContinueLoop";
    case MainLoopActionKind::HiScoreRecordsPage:
        return "HiScoreRecordsPage";
    case MainLoopActionKind::ReplayLoad:
        return "ReplayLoad";
    case MainLoopActionKind::Practice:
        return "Practice";
    case MainLoopActionKind::StageSelect:
        return "StageSelect";
    case MainLoopActionKind::LoadCard:
        return "LoadCard";
    case MainLoopActionKind::ReturnScene0:
        return "ReturnScene0";
    case MainLoopActionKind::Options:
        return "Options";
    case MainLoopActionKind::Gap:
        return "Gap";
    }
    return "Unknown";
}

} // namespace PrSS0DirectoryDispatcherDirect
