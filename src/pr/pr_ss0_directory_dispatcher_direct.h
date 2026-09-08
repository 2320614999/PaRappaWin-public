#pragma once

#include <cstdint>

namespace PrSS0DirectoryDispatcherDirect {

static constexpr uint32_t kFn800264AC = 0x800264ACu;
static constexpr uint32_t kFn80025F6C = 0x80025F6Cu;
static constexpr uint32_t kFn800267F8 = 0x800267F8u;
static constexpr uint32_t kFn80026910 = 0x80026910u;
static constexpr uint32_t kFn80026B54 = 0x80026B54u;
static constexpr uint32_t kFn80015788 = 0x80015788u;
static constexpr uint32_t kFn80025D70 = 0x80025D70u;
static constexpr uint32_t kFn80025C8C = 0x80025C8Cu;
static constexpr uint32_t kFn80025DBC = 0x80025DBCu;
static constexpr uint32_t kFn800267E4 = 0x800267E4u;
static constexpr uint32_t kFn80025E0C = 0x80025E0Cu;
static constexpr uint32_t kFn80025E48 = 0x80025E48u;
static constexpr uint32_t kWord800916E4BlinkCounter = 0x800916E4u;
static constexpr uint32_t kEv3Table80054550 = 0x80054550u;
static constexpr uint32_t kEv2Table8005453C = 0x8005453Cu;
static constexpr uint32_t kEv17Table800545A0 = 0x800545A0u;
static constexpr uint32_t kEv6Table80054578 = 0x80054578u;
static constexpr uint32_t kEvent6HiScoreCtx80049278 = 0x80049278u;

static constexpr uint32_t kPadTriangle = 0x10u;
static constexpr uint32_t kPadCircle = 0x20u;
static constexpr uint32_t kPadCross = 0x40u;
static constexpr uint32_t kPadSquare = 0x80u;
static constexpr uint32_t kPadUp = 0x1000u;
static constexpr uint32_t kPadRight = 0x2000u;
static constexpr uint32_t kPadDown = 0x4000u;
static constexpr uint32_t kPadLeft = 0x8000u;
static constexpr uint32_t kMainMenuActionCue800264AC = 0x20u;
static constexpr uint32_t kMainMenuNavigateCue800264AC = 0x2000u;
static constexpr uint32_t kStageSelectActionCue80025F6C = 0x20u;
static constexpr uint32_t kStageSelectNavigateCue80025F6C = 0x1000u;
static constexpr uint32_t kOptionsActionCue80026910 = 0x20u;
static constexpr uint32_t kOptionsNavigateCue80026910 = 0x2000u;
static constexpr uint32_t kEvent6ActionCue80025E0C = 0x20u;

enum class DispatcherActionKind : uint8_t {
    None = 0,
    GateMainMenuInputSource800264AC,
    GateStageSelectInputSource80025F6C,
    GateStageSelectOutScene80025F6C,
    GateOptionsInputSource80026910,
    GateEvent6InputSource80025E0C,
    GateMainLoopResultSource80015788,
    CallInputSfx80025C8C,
    CallOptionsLanguageSfx80025DBC,
    SetOptionsLanguageSnapshotCtx1C80026910,
    MoveCursor,
    SetItemValue,
    SetWord800916DA,
    SetWord800916D8,
    SetWord800916DC,
    SetWord801C36A6,
    SetCooldown,
    SetDoneFlag,
    SetEvent6ExitLabelCtx04,
    SetOutScene,
    ReturnResult,
    Gap,
};

enum class MainLoopActionKind : uint8_t {
    ContinueLoop = 0,
    HiScoreRecordsPage,
    ReplayLoad,
    Practice,
    StageSelect,
    LoadCard,
    ReturnScene0,
    Options,
    Gap,
};

struct DispatcherAction {
    DispatcherActionKind kind = DispatcherActionKind::None;
    uint32_t psxFunction = 0;
    int32_t arg0 = 0;
    int32_t arg1 = 0;
    int32_t arg2 = 0;
    int32_t arg3 = 0;
};

struct DispatcherActionList {
    DispatcherAction actions[16]{};
    uint32_t count = 0;
    bool truncated = false;
};

struct MainMenuState800264AC {
    int32_t cursor = 0;
    int32_t count = 5;
    int32_t itemValue[5]{};
    int32_t word800916DA = 0;
    bool doneFlag = false;
};

struct MainMenuHandleResult800264AC {
    MainMenuState800264AC state{};
    int32_t result = 0;
    uint32_t cueCode80025C8C = 0;
    DispatcherActionList actions{};
};

struct BlinkTickResult80025D70 {
    int32_t counter800916E4 = 0;
    int32_t contextBlinkOnOff = 0;
    bool toggled = false;
};

struct HiScoreEvent6InitInput800267E4 {
    bool ctxKnown = false;
    uint32_t ctxAddress = 0u;
    int32_t exitIconStateCtx00 = 0;
};

struct HiScoreEvent6State800267E4 {
    bool requestBound = false;
    uint32_t ctxAddress = 0u;
    int32_t exitIconStateCtx00 = 0;
    int32_t exitLabelStateCtx04 = 0;
};

struct HiScoreEvent6HandleResult80025E0C {
    HiScoreEvent6State800267E4 state{};
    int32_t result = 0;
    uint32_t cueCode80025C8C = 0u;
    DispatcherActionList actions{};
};

struct HiScoreEvent6TickResult80025E48 {
    HiScoreEvent6State800267E4 state{};
    BlinkTickResult80025D70 blink{};
    bool accepted = false;
};

struct StageSelectInitInput800267F8 {
    uint8_t status80092F1DTo23[7]{};
    int32_t word800916DA = 0;
    bool word800916F0Known = false;
    int32_t word800916F0 = 0;
};

struct StageSelectState80025F6C {
    int32_t cursor = 1;
    int32_t count = 8;
    // Post-800267F8 context values for ctx+0x0E..ctx+0x1A. The field keeps
    // the source-bank address in its name for compatibility, but F0/DA may
    // replace these values before 80020568 consumes them.
    uint8_t rawStatus80092F1DTo23[7]{};
    bool enabled[9]{};
    bool doneFlag = false;
    int32_t outScene = 0;
};

enum class StageSelectCueOrder80025F6C : uint8_t {
    None = 0,
    BeforeStateCommit,
    AfterStateCommit,
};

struct StageSelectHandleResult80025F6C {
    StageSelectState80025F6C state{};
    int32_t result = 0;
    uint32_t cueCode80025C8C = 0;
    StageSelectCueOrder80025F6C cueOrder =
        StageSelectCueOrder80025F6C::None;
    DispatcherActionList actions{};
};

struct OptionsState80026910 {
    int32_t cursor = 0;
    int32_t count = 3;
    int32_t word800916D8 = 0;
    int32_t word800916DC = 0;
    int32_t word801C36A6 = 0;
    int32_t cooldown = 0;
    int32_t opt0Value = 0;
    int32_t opt1Value = 0;
    int32_t opt2Value = 0;
    int32_t languageBeforeInputSnapshotCtx1C = 0;
    bool doneFlag = false;
};

enum class OptionsCueKind80026910 : uint8_t {
    None = 0,
    Input80025C8C,
    Language80025DBC,
};

struct OptionsHandleResult80026910 {
    OptionsState80026910 state{};
    int32_t result = 0;
    OptionsCueKind80026910 cueKind = OptionsCueKind80026910::None;
    uint32_t cueCode80025C8C = 0;
    bool languageSnapshotWrittenBeforeCue = false;
    DispatcherActionList actions{};
};

struct OptionsTickResult80026B54 {
    OptionsState80026910 state{};
    BlinkTickResult80025D70 blink{};
    bool cooldownDecremented = false;
};

struct MainLoopConsumeResult80015788 {
    MainLoopActionKind action = MainLoopActionKind::ContinueLoop;
    int32_t nextEventId = 0;
    bool returnsScene = false;
    int32_t returnScene = 0;
    bool writesWord800916D0 = false;
    int32_t word800916D0 = 0;
    bool blockedByP0Gap = false;
    DispatcherActionList actions{};
};

bool RuntimeCutoverAllowed();

BlinkTickResult80025D70 TickBlink80025D70(
    int32_t counter800916E4,
    int32_t contextBlinkOnOff);

HiScoreEvent6State800267E4 InitHiScoreEvent6State800267E4(
    const HiScoreEvent6InitInput800267E4& in);
HiScoreEvent6HandleResult80025E0C HandleHiScoreEvent6Input80025E0C(
    const HiScoreEvent6State800267E4& in,
    uint32_t padMask80026744);
HiScoreEvent6TickResult80025E48 TickHiScoreEvent6Blink80025E48(
    const HiScoreEvent6State800267E4& in,
    int32_t counter800916E4);

MainMenuState800264AC InitMainMenuState80026794();
MainMenuHandleResult800264AC HandleMainMenu800264AC(
    const MainMenuState800264AC& in,
    uint32_t padMask);

StageSelectState80025F6C InitStageSelect800267F8(
    const StageSelectInitInput800267F8& in);
StageSelectHandleResult80025F6C HandleStageSelect80025F6C(
    const StageSelectState80025F6C& in,
    uint32_t padMask);

OptionsState80026910 InitOptions800268E4(int32_t word800916D8,
                                         int32_t word800916DC);
OptionsHandleResult80026910 HandleOptions80026910(
    const OptionsState80026910& in,
    uint32_t padMask);
OptionsTickResult80026B54 TickOptions80026B54(
    const OptionsState80026910& in,
    int32_t counter800916E4,
    int32_t contextBlinkOnOff);

MainLoopConsumeResult80015788 ConsumeMainMenuResult80015788(int32_t result);

const char* DispatcherActionKindName(DispatcherActionKind kind);
const char* MainLoopActionKindName(MainLoopActionKind kind);

} // namespace PrSS0DirectoryDispatcherDirect
