#include "pr/pr_ss0_directory_dispatcher_direct.h"

#include <cstdio>
#include <initializer_list>

using namespace PrSS0DirectoryDispatcherDirect;

namespace {

int g_failed = 0;

#define CHECK(expr)                                                           \
    do {                                                                      \
        if (!(expr)) {                                                        \
            std::printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr);       \
            ++g_failed;                                                       \
        }                                                                     \
    } while (0)

const DispatcherAction* FindAction(const DispatcherActionList& list,
                                   DispatcherActionKind kind) {
    for (uint32_t i = 0; i < list.count; ++i) {
        if (list.actions[i].kind == kind) {
            return &list.actions[i];
        }
    }
    return nullptr;
}

uint32_t CountActions(const DispatcherActionList& list,
                      DispatcherActionKind kind) {
    uint32_t count = 0;
    for (uint32_t i = 0; i < list.count; ++i) {
        if (list.actions[i].kind == kind) {
            ++count;
        }
    }
    return count;
}

int ActionIndex(const DispatcherActionList& list,
                DispatcherActionKind kind) {
    for (uint32_t i = 0; i < list.count; ++i) {
        if (list.actions[i].kind == kind) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

bool SameOptionsState(const OptionsState80026910& left,
                      const OptionsState80026910& right) {
    return left.cursor == right.cursor &&
           left.count == right.count &&
           left.word800916D8 == right.word800916D8 &&
           left.word800916DC == right.word800916DC &&
           left.word801C36A6 == right.word801C36A6 &&
           left.cooldown == right.cooldown &&
           left.opt0Value == right.opt0Value &&
           left.opt1Value == right.opt1Value &&
           left.opt2Value == right.opt2Value &&
           left.languageBeforeInputSnapshotCtx1C ==
               right.languageBeforeInputSnapshotCtx1C &&
           left.doneFlag == right.doneFlag;
}

void TestRuntimeGateFalse() {
    CHECK(!RuntimeCutoverAllowed());
}

void TestSharedBlinkClock20FrameCadence() {
    int32_t counter800916E4 = 0;
    int32_t mainMenuBlink = 0;
    int32_t stageSelectBlink = 0;

    auto tick = TickBlink80025D70(counter800916E4, mainMenuBlink);
    counter800916E4 = tick.counter800916E4;
    mainMenuBlink = tick.contextBlinkOnOff;
    CHECK(tick.toggled);
    CHECK(counter800916E4 == 1);
    CHECK(mainMenuBlink == 1);

    for (int frame = 0; frame < 18; ++frame) {
        tick = TickBlink80025D70(counter800916E4, stageSelectBlink);
        counter800916E4 = tick.counter800916E4;
        stageSelectBlink = tick.contextBlinkOnOff;
        CHECK(!tick.toggled);
    }
    CHECK(counter800916E4 == 19);
    CHECK(stageSelectBlink == 0);

    tick = TickBlink80025D70(counter800916E4, stageSelectBlink);
    counter800916E4 = tick.counter800916E4;
    stageSelectBlink = tick.contextBlinkOnOff;
    CHECK(!tick.toggled);
    CHECK(counter800916E4 == 0);
    CHECK(stageSelectBlink == 0);

    tick = TickBlink80025D70(counter800916E4, stageSelectBlink);
    CHECK(tick.toggled);
    CHECK(tick.counter800916E4 == 1);
    CHECK(tick.contextBlinkOnOff == 1);

    tick = TickBlink80025D70(0, 1);
    CHECK(tick.contextBlinkOnOff == 0);
    tick = TickBlink80025D70(0, 7);
    CHECK(tick.contextBlinkOnOff == 1);
}

void TestMainMenuResultMapAndRecordsMode() {
    MainMenuState800264AC state = InitMainMenuState80026794();
    CHECK(state.cursor == 0);
    CHECK(state.count == 5);

    MainMenuHandleResult800264AC handled =
        HandleMainMenu800264AC(state, kPadDown);
    CHECK(handled.state.cursor == 1);
    CHECK(handled.result == 0);
    CHECK(handled.cueCode80025C8C == kMainMenuNavigateCue800264AC);
    CHECK(FindAction(handled.actions,
                     DispatcherActionKind::GateMainMenuInputSource800264AC) != nullptr);
    CHECK(FindAction(handled.actions, DispatcherActionKind::MoveCursor) != nullptr);

    state.cursor = 2;
    handled = HandleMainMenu800264AC(state, kPadCircle);
    CHECK(handled.result == 0);
    CHECK(handled.state.word800916DA == 1);
    CHECK(handled.cueCode80025C8C == kMainMenuActionCue800264AC);
    CHECK(FindAction(handled.actions, DispatcherActionKind::SetWord800916DA) != nullptr);

    handled = HandleMainMenu800264AC(handled.state, kPadCross);
    CHECK(handled.result == 0);
    CHECK(handled.state.word800916DA == 0);
    CHECK(handled.cueCode80025C8C == kMainMenuActionCue800264AC);
    CHECK(FindAction(handled.actions, DispatcherActionKind::SetWord800916DA) != nullptr);

    state.cursor = 1;
    handled = HandleMainMenu800264AC(state, kPadCross);
    CHECK(handled.result == 1);
    CHECK(handled.cueCode80025C8C == kMainMenuActionCue800264AC);
    CHECK(FindAction(handled.actions, DispatcherActionKind::ReturnResult) != nullptr);

    state.cursor = 3;
    handled = HandleMainMenu800264AC(state, kPadSquare);
    CHECK(handled.result == 3);
    CHECK(handled.cueCode80025C8C == kMainMenuActionCue800264AC);
    handled = HandleMainMenu800264AC(state, kPadCross);
    CHECK(handled.result == 4);
    CHECK(handled.cueCode80025C8C == kMainMenuActionCue800264AC);
    handled = HandleMainMenu800264AC(state, kPadCircle);
    CHECK(handled.result == 2);
    CHECK(handled.cueCode80025C8C == kMainMenuActionCue800264AC);
    handled = HandleMainMenu800264AC(state, kPadTriangle);
    CHECK(handled.result == 6);
    CHECK(handled.cueCode80025C8C == kMainMenuActionCue800264AC);

    state.cursor = 4;
    handled = HandleMainMenu800264AC(state, kPadCross);
    CHECK(handled.result == 7);
    CHECK(handled.cueCode80025C8C == kMainMenuActionCue800264AC);
    CHECK(handled.state.doneFlag);
    CHECK(FindAction(handled.actions, DispatcherActionKind::SetDoneFlag) != nullptr);

    handled = HandleMainMenu800264AC(state, kPadTriangle);
    CHECK(handled.result == 0);
    CHECK(handled.cueCode80025C8C == 0u);
}

void TestStageSelectUnknownF0FallbackAndCancel() {
    StageSelectInitInput800267F8 input{};
    input.word800916F0Known = false;
    input.word800916F0 = 0;
    input.word800916DA = 0;
    input.status80092F1DTo23[0] = 1;
    input.status80092F1DTo23[6] = 1;

    StageSelectState80025F6C state = InitStageSelect800267F8(input);
    CHECK(state.cursor == 1);
    CHECK(state.enabled[1]);
    CHECK(!state.enabled[2]);
    CHECK(!state.enabled[7]);
    CHECK(state.enabled[8]);

    StageSelectHandleResult80025F6C handled =
        HandleStageSelect80025F6C(state, kPadTriangle);
    CHECK(handled.result == 0);
    CHECK(handled.cueCode80025C8C == 0u);
    CHECK(handled.cueOrder == StageSelectCueOrder80025F6C::None);
    CHECK(handled.state.cursor == 1);
    CHECK(!handled.state.doneFlag);
    CHECK(CountActions(handled.actions, DispatcherActionKind::ReturnResult) == 0);

    const StageSelectHandleResult80025F6C cursor1 =
        HandleStageSelect80025F6C(state, kPadCross);
    CHECK(cursor1.result == 1);
    CHECK(cursor1.state.outScene == 1);
    CHECK(cursor1.cueCode80025C8C == kStageSelectActionCue80025F6C);
    CHECK(cursor1.cueOrder ==
          StageSelectCueOrder80025F6C::BeforeStateCommit);
    CHECK(ActionIndex(cursor1.actions,
                      DispatcherActionKind::CallInputSfx80025C8C) <
          ActionIndex(cursor1.actions, DispatcherActionKind::SetOutScene));

    handled = HandleStageSelect80025F6C(state, kPadDown);
    CHECK(handled.result == 0);
    CHECK(handled.cueCode80025C8C == kStageSelectNavigateCue80025F6C);
    CHECK(handled.cueOrder ==
          StageSelectCueOrder80025F6C::BeforeStateCommit);
    CHECK(handled.state.cursor == 8);
    CHECK(FindAction(handled.actions,
                     DispatcherActionKind::CallInputSfx80025C8C) != nullptr);
    CHECK(FindAction(handled.actions, DispatcherActionKind::MoveCursor) != nullptr);
    CHECK(ActionIndex(handled.actions,
                      DispatcherActionKind::CallInputSfx80025C8C) <
          ActionIndex(handled.actions, DispatcherActionKind::MoveCursor));

    handled = HandleStageSelect80025F6C(handled.state, kPadCross);
    CHECK(handled.result == 2);
    CHECK(handled.cueCode80025C8C == kStageSelectActionCue80025F6C);
    CHECK(handled.cueOrder ==
          StageSelectCueOrder80025F6C::AfterStateCommit);
    CHECK(handled.state.doneFlag);
    CHECK(handled.state.outScene == 0);
    CHECK(ActionIndex(handled.actions, DispatcherActionKind::SetDoneFlag) <
          ActionIndex(handled.actions,
                      DispatcherActionKind::CallInputSfx80025C8C));
    CHECK(ActionIndex(handled.actions,
                      DispatcherActionKind::CallInputSfx80025C8C) <
          ActionIndex(handled.actions,
                      DispatcherActionKind::GateStageSelectOutScene80025F6C));
    const DispatcherAction* gate =
        FindAction(handled.actions, DispatcherActionKind::GateStageSelectOutScene80025F6C);
    CHECK(gate != nullptr);
    CHECK(gate->arg0 == 8);
    CHECK(gate->arg1 == -1);
}

void TestStageSelectNavigationCueAllDirections() {
    StageSelectInitInput800267F8 input{};
    input.word800916F0Known = true;
    input.word800916F0 = 1;
    StageSelectState80025F6C state = InitStageSelect800267F8(input);
    CHECK(state.cursor == 1);

    const uint32_t padMasks[] = {kPadDown, kPadRight, kPadUp, kPadLeft};
    const int32_t expectedCursors[] = {2, 2, 8, 8};
    for (int i = 0; i < 4; ++i) {
        const StageSelectHandleResult80025F6C handled =
            HandleStageSelect80025F6C(state, padMasks[i]);
        CHECK(handled.result == 0);
        CHECK(handled.state.cursor == expectedCursors[i]);
        CHECK(handled.cueCode80025C8C == kStageSelectNavigateCue80025F6C);
        CHECK(handled.cueOrder ==
              StageSelectCueOrder80025F6C::BeforeStateCommit);
        CHECK(ActionIndex(handled.actions,
                          DispatcherActionKind::CallInputSfx80025C8C) >= 0);
        CHECK(ActionIndex(handled.actions, DispatcherActionKind::MoveCursor) >= 0);
        CHECK(ActionIndex(handled.actions,
                          DispatcherActionKind::CallInputSfx80025C8C) <
              ActionIndex(handled.actions, DispatcherActionKind::MoveCursor));
    }
}

void TestStageSelectDisabledCursorRejectsCross() {
    StageSelectInitInput800267F8 input{};
    input.word800916F0Known = false;
    input.word800916F0 = 0;
    input.word800916DA = 0;
    input.status80092F1DTo23[0] = 1;
    input.status80092F1DTo23[6] = 1;

    StageSelectState80025F6C state = InitStageSelect800267F8(input);
    state.cursor = 7;
    CHECK(!state.enabled[7]);

    const StageSelectHandleResult80025F6C handled =
        HandleStageSelect80025F6C(state, kPadCross);
    CHECK(handled.result == 0);
    CHECK(handled.cueCode80025C8C == 0u);
    CHECK(handled.cueOrder == StageSelectCueOrder80025F6C::None);
    CHECK(!handled.state.doneFlag);
    CHECK(handled.state.outScene == 0);
    CHECK(FindAction(handled.actions,
                     DispatcherActionKind::GateStageSelectOutScene80025F6C) ==
          nullptr);
    CHECK(FindAction(handled.actions, DispatcherActionKind::SetOutScene) ==
          nullptr);
    CHECK(FindAction(handled.actions, DispatcherActionKind::SetDoneFlag) ==
          nullptr);
    CHECK(FindAction(handled.actions, DispatcherActionKind::ReturnResult) ==
          nullptr);
    const DispatcherAction* gap =
        FindAction(handled.actions, DispatcherActionKind::Gap);
    CHECK(gap != nullptr);
    if (gap != nullptr) {
        CHECK(gap->arg0 == 7);
        CHECK((gap->arg3 & (1 << 7)) == 0);
    }
}

void TestStageSelectKnownF0ForceEnableBonusOutScene() {
    StageSelectInitInput800267F8 input{};
    input.word800916F0Known = true;
    input.word800916F0 = 1;
    input.word800916DA = 0;

    StageSelectState80025F6C state = InitStageSelect800267F8(input);
    CHECK(state.cursor == 1);
    constexpr uint8_t kExpectedStatus[7] = {1u, 1u, 1u, 1u, 2u, 3u, 1u};
    for (int index = 0; index < 7; ++index) {
        CHECK(state.rawStatus80092F1DTo23[index] ==
              kExpectedStatus[index]);
    }
    for (int cursor = 1; cursor <= 8; ++cursor) {
        CHECK(state.enabled[cursor]);
    }

    state.cursor = 7;
    StageSelectHandleResult80025F6C handled =
        HandleStageSelect80025F6C(state, kPadCross);
    CHECK(handled.result == 1);
    CHECK(handled.cueCode80025C8C == kStageSelectActionCue80025F6C);
    CHECK(handled.cueOrder ==
          StageSelectCueOrder80025F6C::BeforeStateCommit);
    CHECK(handled.state.doneFlag);
    CHECK(handled.state.outScene == 8);
    const DispatcherAction* gate =
        FindAction(handled.actions, DispatcherActionKind::GateStageSelectOutScene80025F6C);
    CHECK(gate != nullptr);
    CHECK(gate->arg0 == 7);
    CHECK(gate->arg1 == 8);
    CHECK(ActionIndex(handled.actions,
                      DispatcherActionKind::CallInputSfx80025C8C) <
          ActionIndex(handled.actions, DispatcherActionKind::SetDoneFlag));
    CHECK(ActionIndex(handled.actions, DispatcherActionKind::SetDoneFlag) <
          ActionIndex(handled.actions, DispatcherActionKind::SetOutScene));
    CHECK(gate->arg2 == 1);
    CHECK(gate->arg3 == 8);
    CHECK(FindAction(handled.actions, DispatcherActionKind::SetOutScene) != nullptr);
    CHECK(FindAction(handled.actions, DispatcherActionKind::SetDoneFlag) != nullptr);
    CHECK(FindAction(handled.actions, DispatcherActionKind::ReturnResult) != nullptr);
}

void TestStageSelectRecordsModeMaterializesContextStatus() {
    StageSelectInitInput800267F8 input{};
    input.word800916F0Known = true;
    input.word800916F0 = 0;
    input.word800916DA = 1;
    for (uint8_t& status : input.status80092F1DTo23) {
        status = 3u;
    }

    const StageSelectState80025F6C state = InitStageSelect800267F8(input);
    CHECK(state.rawStatus80092F1DTo23[0] == 1u);
    CHECK(state.enabled[1]);
    for (int index = 1; index < 7; ++index) {
        CHECK(state.rawStatus80092F1DTo23[index] == 0u);
        CHECK(!state.enabled[index + 1]);
    }
    CHECK(state.enabled[8]);
}

void TestOptionsCooldownAndDone() {
    OptionsState80026910 state = InitOptions800268E4(0, 0);
    CHECK(state.cursor == 1);
    CHECK(state.count == 3);
    CHECK(state.word801C36A6 == 0);
    CHECK(state.opt0Value == 1);
    CHECK(state.opt1Value == 0);
    CHECK(state.languageBeforeInputSnapshotCtx1C == 0);

    const OptionsState80026910 subtitleOn = InitOptions800268E4(4, 1);
    CHECK(subtitleOn.word800916D8 == 4);
    CHECK(subtitleOn.word800916DC == 1);
    CHECK(subtitleOn.word801C36A6 == 4);
    CHECK(subtitleOn.opt0Value == 0);
    CHECK(subtitleOn.opt1Value == 4);
    CHECK(subtitleOn.languageBeforeInputSnapshotCtx1C == 0);

    OptionsHandleResult80026910 handled = HandleOptions80026910(state, kPadRight);
    CHECK(handled.state.word800916D8 == 1);
    CHECK(handled.state.opt1Value == 1);
    CHECK(handled.state.languageBeforeInputSnapshotCtx1C == 0);
    CHECK(handled.state.word801C36A6 == 1);
    CHECK(handled.state.cooldown == 16);
    CHECK(handled.languageSnapshotWrittenBeforeCue);
    CHECK(handled.cueKind == OptionsCueKind80026910::Language80025DBC);
    CHECK(handled.cueCode80025C8C == 0u);
    CHECK(ActionIndex(
              handled.actions,
              DispatcherActionKind::SetOptionsLanguageSnapshotCtx1C80026910) <
          ActionIndex(handled.actions,
                      DispatcherActionKind::CallOptionsLanguageSfx80025DBC));
    CHECK(ActionIndex(handled.actions,
                      DispatcherActionKind::CallOptionsLanguageSfx80025DBC) <
          ActionIndex(handled.actions, DispatcherActionKind::SetCooldown));
    CHECK(ActionIndex(handled.actions,
                      DispatcherActionKind::CallOptionsLanguageSfx80025DBC) <
          ActionIndex(handled.actions, DispatcherActionKind::SetWord800916D8));
    CHECK(FindAction(handled.actions, DispatcherActionKind::SetCooldown) != nullptr);

    OptionsTickResult80026B54 tick =
        TickOptions80026B54(handled.state, 0, 0);
    CHECK(tick.cooldownDecremented);
    CHECK(tick.state.cooldown == 15);
    CHECK(tick.blink.toggled);
    CHECK(tick.blink.counter800916E4 == 1);
    CHECK(tick.blink.contextBlinkOnOff == 1);

    const uint32_t blockedMasks[] = {
        kPadUp, kPadDown, kPadLeft, kPadRight, kPadCross, kPadCircle,
    };
    for (const uint32_t mask : blockedMasks) {
        const auto blocked = HandleOptions80026910(tick.state, mask);
        CHECK(SameOptionsState(blocked.state, tick.state));
        CHECK(blocked.result == 0);
        CHECK(blocked.cueKind == OptionsCueKind80026910::None);
        CHECK(blocked.cueCode80025C8C == 0u);
        CHECK(blocked.actions.count == 1u);
        CHECK(FindAction(blocked.actions,
                         DispatcherActionKind::SetCooldown) == nullptr);
        CHECK(FindAction(blocked.actions,
                         DispatcherActionKind::CallInputSfx80025C8C) == nullptr);
        CHECK(FindAction(
                  blocked.actions,
                  DispatcherActionKind::CallOptionsLanguageSfx80025DBC) ==
              nullptr);
    }

    state = tick.state;
    int32_t blinkCounter = tick.blink.counter800916E4;
    int32_t optionsBlink = tick.blink.contextBlinkOnOff;
    for (int blockedFrame = 0; blockedFrame < 15; ++blockedFrame) {
        const auto blocked = HandleOptions80026910(state, kPadDown);
        CHECK(SameOptionsState(blocked.state, state));
        tick = TickOptions80026B54(state, blinkCounter, optionsBlink);
        state = tick.state;
        blinkCounter = tick.blink.counter800916E4;
        optionsBlink = tick.blink.contextBlinkOnOff;
    }
    CHECK(state.cooldown == 0);

    handled = HandleOptions80026910(state, kPadDown);
    CHECK(handled.state.cursor == 2);
    CHECK(handled.cueKind == OptionsCueKind80026910::Input80025C8C);
    CHECK(handled.cueCode80025C8C == kOptionsNavigateCue80026910);
    CHECK(ActionIndex(handled.actions,
                      DispatcherActionKind::CallInputSfx80025C8C) <
          ActionIndex(handled.actions, DispatcherActionKind::MoveCursor));

    state = InitOptions800268E4(0, 0);
    state.cursor = 0;
    handled = HandleOptions80026910(state, kPadCross);
    CHECK(handled.state.word800916DC == 1);
    CHECK(handled.state.opt0Value == 0);
    CHECK(handled.cueKind == OptionsCueKind80026910::Input80025C8C);
    CHECK(handled.cueCode80025C8C == kOptionsActionCue80026910);
    CHECK(ActionIndex(handled.actions,
                      DispatcherActionKind::CallInputSfx80025C8C) <
          ActionIndex(handled.actions, DispatcherActionKind::SetWord800916DC));

    handled = HandleOptions80026910(handled.state, kPadCircle);
    CHECK(handled.state.word800916DC == 0);
    CHECK(handled.state.opt0Value == 1);
    CHECK(handled.cueKind == OptionsCueKind80026910::Input80025C8C);
    CHECK(handled.cueCode80025C8C == kOptionsActionCue80026910);

    state = InitOptions800268E4(0, 0);
    state.cursor = 2;
    handled = HandleOptions80026910(state, kPadCross);
    CHECK(handled.result == 1);
    CHECK(handled.state.doneFlag);
    CHECK(handled.state.opt2Value == 0);
    CHECK(handled.cueKind == OptionsCueKind80026910::Input80025C8C);
    CHECK(handled.cueCode80025C8C == kOptionsActionCue80026910);
    CHECK(ActionIndex(handled.actions,
                      DispatcherActionKind::CallInputSfx80025C8C) <
          ActionIndex(handled.actions, DispatcherActionKind::SetDoneFlag));
    CHECK(ActionIndex(handled.actions, DispatcherActionKind::SetDoneFlag) <
          ActionIndex(handled.actions, DispatcherActionKind::ReturnResult));
    CHECK(FindAction(handled.actions, DispatcherActionKind::ReturnResult) != nullptr);

    state = InitOptions800268E4(0, 0);
    handled = HandleOptions80026910(state, kPadLeft);
    CHECK(handled.state.word800916D8 == 4);
    CHECK(handled.state.opt1Value == 4);
    CHECK(handled.state.languageBeforeInputSnapshotCtx1C == 0);
    CHECK(handled.state.word801C36A6 == 4);
    CHECK(handled.state.cooldown == 16);
    CHECK(handled.cueKind == OptionsCueKind80026910::Language80025DBC);

    state = InitOptions800268E4(2, 1);
    handled = HandleOptions80026910(state, kPadTriangle);
    CHECK(handled.state.cursor == state.cursor);
    CHECK(handled.state.word800916D8 == state.word800916D8);
    CHECK(handled.state.word800916DC == state.word800916DC);
    CHECK(handled.state.opt1Value == state.word800916D8);
    CHECK(handled.state.languageBeforeInputSnapshotCtx1C ==
          state.word800916D8);
    CHECK(handled.languageSnapshotWrittenBeforeCue);
    CHECK(handled.result == 0);
    CHECK(handled.cueKind == OptionsCueKind80026910::None);
    CHECK(FindAction(handled.actions,
                     DispatcherActionKind::CallInputSfx80025C8C) == nullptr);
    CHECK(FindAction(
              handled.actions,
              DispatcherActionKind::CallOptionsLanguageSfx80025DBC) == nullptr);
    CHECK(FindAction(
              handled.actions,
              DispatcherActionKind::SetOptionsLanguageSnapshotCtx1C80026910) !=
          nullptr);
}

void TestHiScoreEvent6ExactControlLoop() {
    HiScoreEvent6InitInput800267E4 init{};
    HiScoreEvent6State800267E4 state =
        InitHiScoreEvent6State800267E4(init);
    CHECK(!state.requestBound);

    init.ctxKnown = true;
    init.ctxAddress = 0x8005424Cu;
    init.exitIconStateCtx00 = 1;
    state = InitHiScoreEvent6State800267E4(init);
    CHECK(!state.requestBound);

    init.ctxAddress = kEvent6HiScoreCtx80049278;
    for (const int32_t initialIcon : {0, 1, 7}) {
        init.exitIconStateCtx00 = initialIcon;
        state = InitHiScoreEvent6State800267E4(init);
        CHECK(state.requestBound);
        CHECK(state.ctxAddress == kEvent6HiScoreCtx80049278);
        CHECK(state.exitIconStateCtx00 == initialIcon);
        CHECK(state.exitLabelStateCtx04 == 0);
    }
    init.exitIconStateCtx00 = 1;
    state = InitHiScoreEvent6State800267E4(init);

    const uint32_t ignoredMasks[] = {
        0u,
        kPadTriangle,
        kPadCircle,
        kPadSquare,
        kPadUp,
        kPadRight,
        kPadDown,
        kPadLeft,
        0x0100u,
        0x0800u,
        kPadCross | 0x0001u,
        kPadCross | 0x0002u,
        kPadCross | 0x0004u,
        kPadCross | 0x0008u,
        kPadCross | kPadTriangle,
        kPadCross | kPadCircle,
        kPadCross | kPadSquare,
        kPadCross | 0x0100u,
        kPadCross | 0x0800u,
        kPadCross | kPadUp,
        kPadCross | kPadRight,
        kPadCross | kPadDown,
        kPadCross | kPadLeft,
    };
    for (const uint32_t mask : ignoredMasks) {
        const auto ignored = HandleHiScoreEvent6Input80025E0C(state, mask);
        CHECK(ignored.result == 0);
        CHECK(ignored.cueCode80025C8C == 0u);
        CHECK(ignored.state.exitLabelStateCtx04 == 0);
        CHECK(FindAction(ignored.actions,
                         DispatcherActionKind::CallInputSfx80025C8C) ==
              nullptr);
        CHECK(FindAction(ignored.actions,
                         DispatcherActionKind::ReturnResult) == nullptr);
    }

    const auto handled =
        HandleHiScoreEvent6Input80025E0C(state, kPadCross);
    CHECK(handled.state.requestBound);
    CHECK(handled.state.exitLabelStateCtx04 == 1);
    CHECK(handled.result == 1);
    CHECK(handled.cueCode80025C8C == kEvent6ActionCue80025E0C);
    CHECK(ActionIndex(handled.actions,
                      DispatcherActionKind::SetEvent6ExitLabelCtx04) <
          ActionIndex(handled.actions,
                      DispatcherActionKind::CallInputSfx80025C8C));
    CHECK(ActionIndex(handled.actions,
                      DispatcherActionKind::CallInputSfx80025C8C) <
          ActionIndex(handled.actions, DispatcherActionKind::ReturnResult));

    const auto actionTick =
        TickHiScoreEvent6Blink80025E48(handled.state, 0);
    CHECK(actionTick.accepted);
    CHECK(actionTick.blink.toggled);
    CHECK(actionTick.blink.counter800916E4 == 1);
    CHECK(actionTick.state.exitIconStateCtx00 == 0);
    CHECK(actionTick.state.exitLabelStateCtx04 == 1);

    const auto tailTick = TickHiScoreEvent6Blink80025E48(
        actionTick.state, actionTick.blink.counter800916E4);
    CHECK(tailTick.accepted);
    CHECK(!tailTick.blink.toggled);
    CHECK(tailTick.blink.counter800916E4 == 2);
    CHECK(tailTick.state.exitIconStateCtx00 == 0);
    CHECK(tailTick.state.exitLabelStateCtx04 == 1);

    const HiScoreEvent6State800267E4 rejected{};
    const auto rejectedInput =
        HandleHiScoreEvent6Input80025E0C(rejected, kPadCross);
    CHECK(rejectedInput.result == 0);
    CHECK(rejectedInput.cueCode80025C8C == 0u);
    CHECK(FindAction(rejectedInput.actions, DispatcherActionKind::Gap) !=
          nullptr);
    CHECK(!TickHiScoreEvent6Blink80025E48(rejected, 0).accepted);
}

void TestMainLoopResultMap() {
    MainLoopConsumeResult80015788 out = ConsumeMainMenuResult80015788(1);
    CHECK(out.action == MainLoopActionKind::HiScoreRecordsPage);
    CHECK(out.nextEventId == 6);
    CHECK(out.blockedByP0Gap);

    out = ConsumeMainMenuResult80015788(2);
    CHECK(out.action == MainLoopActionKind::ReplayLoad);
    CHECK(out.writesWord800916D0);
    CHECK(out.word800916D0 == 2);
    CHECK(out.blockedByP0Gap);

    out = ConsumeMainMenuResult80015788(4);
    CHECK(out.action == MainLoopActionKind::StageSelect);
    CHECK(out.nextEventId == 2);
    CHECK(!out.blockedByP0Gap);

    out = ConsumeMainMenuResult80015788(7);
    CHECK(out.action == MainLoopActionKind::ReturnScene0);
    CHECK(out.returnsScene);
    CHECK(out.returnScene == 0);

    out = ConsumeMainMenuResult80015788(99);
    CHECK(out.action == MainLoopActionKind::Gap);
    CHECK(out.blockedByP0Gap);
    CHECK(FindAction(out.actions, DispatcherActionKind::GateMainLoopResultSource80015788) != nullptr);
}

} // namespace

int main() {
    TestRuntimeGateFalse();
    TestSharedBlinkClock20FrameCadence();
    TestMainMenuResultMapAndRecordsMode();
    TestStageSelectUnknownF0FallbackAndCancel();
    TestStageSelectNavigationCueAllDirections();
    TestStageSelectDisabledCursorRejectsCross();
    TestStageSelectKnownF0ForceEnableBonusOutScene();
    TestStageSelectRecordsModeMaterializesContextStatus();
    TestOptionsCooldownAndDone();
    TestHiScoreEvent6ExactControlLoop();
    TestMainLoopResultMap();

    if (g_failed != 0) {
        std::printf("test_ss0_directory_dispatcher: failed checks=%d\n", g_failed);
        return 1;
    }
    std::printf("test_ss0_directory_dispatcher: ok\n");
    return 0;
}
