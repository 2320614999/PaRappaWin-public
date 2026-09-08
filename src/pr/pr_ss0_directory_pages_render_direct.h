#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace PrPsxGraphOwnerDirect {
struct PsxGraphState;
}

namespace PrSS0DirectoryPagesRenderDirect {

static constexpr uint32_t kFn8001D74C = 0x8001D74Cu;
static constexpr uint32_t kFn80020568 = 0x80020568u;
static constexpr uint32_t kFn80020F94 = 0x80020F94u;
static constexpr uint32_t kFn80021910 = 0x80021910u;
static constexpr uint32_t kFn80021E60 = 0x80021E60u;
static constexpr uint32_t kFn80023618 = 0x80023618u;

static constexpr uint32_t kFn8001B730 = 0x8001B730u;
static constexpr uint32_t kFn8001B744 = 0x8001B744u;
static constexpr uint32_t kFn8001C550 = 0x8001C550u;
static constexpr uint32_t kFn8001C5A8 = 0x8001C5A8u;
static constexpr uint32_t kFn8001C604 = 0x8001C604u;
static constexpr uint32_t kFn8001C668 = 0x8001C668u;
static constexpr uint32_t kFn80023518 = 0x80023518u;
static constexpr uint32_t kFn80023F20 = 0x80023F20u;
static constexpr uint32_t kFn80024418 = 0x80024418u;
static constexpr uint32_t kFn80024600 = 0x80024600u;
static constexpr uint32_t kFn800246A8 = 0x800246A8u;

static constexpr uint32_t kGlobalWord800916D8 = 0x800916D8u;
static constexpr uint32_t kGlobalWord800916DC = 0x800916DCu;
static constexpr uint32_t kStageSelectCtx80087B78 = 0x80087B78u;
static constexpr uint32_t kTableSharedExitText80053000 = 0x80053000u;
static constexpr uint32_t kGpBase8006EA40 = 0x8006EA40u;
static constexpr uint32_t kGpSharedExitIconSlotOn8006EB10 = 0x8006EB10u;
static constexpr uint32_t kGpSharedExitIconSlotOff8006EB14 = 0x8006EB14u;
static constexpr uint32_t kGpPracticePortraitRepeatFrame8006EB70 =
    0x8006EB70u;
static constexpr uint32_t kGpPracticePortraitLastValue8006EB74 =
    0x8006EB74u;
static constexpr uint32_t kGpPracticePortraitTemplate8006ED5C =
    0x8006ED5Cu;
static constexpr uint32_t kTablePracticePortraitModes80011238 =
    0x80011238u;
static constexpr uint32_t kTableStageSelectPoints80053104 = 0x80053104u;
static constexpr uint32_t kTableStageSelectSlices800531D0 = 0x800531D0u;
static constexpr uint32_t kTableMainLanguage80053554 = 0x80053554u;
static constexpr uint32_t kTableMainHiScore800535B0 = 0x800535B0u;
static constexpr uint32_t kTableMainNormal80053640 = 0x80053640u;
static constexpr uint32_t kTableMainEasy80053690 = 0x80053690u;
static constexpr uint32_t kTableMainPractice80053710 = 0x80053710u;
static constexpr uint32_t kTableMainStageSelect80053760 = 0x80053760u;
static constexpr uint32_t kTableMainReplay800537B0 = 0x800537B0u;
static constexpr uint32_t kTableMainLoad80053800 = 0x80053800u;
static constexpr uint32_t kTableOptionsSubtitleOn80053850 = 0x80053850u;
static constexpr uint32_t kTableOptionsSubtitleOff800538A0 = 0x800538A0u;
static constexpr uint32_t kTableOptionsLanguageA800538F0 = 0x800538F0u;
static constexpr uint32_t kTableOptionsLanguageB80053940 = 0x80053940u;
static constexpr uint32_t kTablePracticeTitle80053D48 = 0x80053D48u;
static constexpr uint32_t kTablePracticeSubtitle80053BB8 = 0x80053BB8u;
static constexpr uint32_t kTablePracticeOverlayA80053C08 = 0x80053C08u;
static constexpr uint32_t kTablePracticeIconTemplate800540BC = 0x800540BCu;
static constexpr uint32_t kTableCardPrompt80053DA4 = 0x80053DA4u;
static constexpr uint32_t kTableCardTitle80053DF4 = 0x80053DF4u;
static constexpr uint32_t kTableCardFooter80053E44 = 0x80053E44u;
static constexpr uint32_t kTableCardSaveTextA80053E94 = 0x80053E94u;
static constexpr uint32_t kTableCardSaveTextB80053EBC = 0x80053EBCu;
static constexpr uint32_t kTableCardLoadTextA80053EE4 = 0x80053EE4u;
static constexpr uint32_t kTableCardLoadTextB80053F0C = 0x80053F0Cu;
static constexpr uint32_t kTableCardReplayTextA80053F34 = 0x80053F34u;
static constexpr uint32_t kTableCardReplayTextB80053F5C = 0x80053F5Cu;
static constexpr uint32_t kSpriteSharedExitIconOff80050AC0 = 0x80050AC0u;
static constexpr uint32_t kSpriteSharedExitIconOn80050AD0 = 0x80050AD0u;
static constexpr int32_t kSharedExitIconGpSlotOn = 0xD0;
static constexpr int32_t kSharedExitIconGpSlotOff = 0xD4;
static constexpr int32_t kDirectoryGp0ReplayBaselineCount = 29;

enum class DirectoryPage : uint8_t {
    Unknown = 0,
    StageSelectEv2,
    MainDirectoryEv3,
    CardGridFamily,
    CardSaveEv7,
    CardLoadEv8,
    CardReplayEv9,
    PracticeEv16,
    OptionsEv17,
};

enum class DirectoryTableKind : uint8_t {
    Unknown = 0,
    SharedExitText,
    StageSelectPoints,
    StageSelectSlices,
    MainLanguage,
    MainHiScore,
    MainNormal,
    MainEasy,
    MainPractice,
    MainStageSelect,
    MainReplay,
    MainLoad,
    CardPrompt,
    CardTitle,
    CardFooter,
    CardSaveTextA,
    CardSaveTextB,
    CardLoadTextA,
    CardLoadTextB,
    CardReplayTextA,
    CardReplayTextB,
    OptionsSubtitleOn,
    OptionsSubtitleOff,
    OptionsLanguageA,
    OptionsLanguageB,
    PracticeTitle,
    PracticeSubtitle,
    PracticeOverlay,
    PracticeIconTemplate,
};

enum class DirectoryRenderActionKind : uint8_t {
    None = 0,
    GatePageDrawRoute,
    BeginDrawWork8001D74C,
    DrawPageFunction,
    FixedSprite,
    LanguageTextTable,
    StagePointTable,
    StageSliceTable,
    GateStageSelectDrawSource80020568,
    StageStatusState,
    GateCardGridArgSource80020F94,
    CardGridGeometry,
    CardGridExitFromArg,
    GateOptionsGlobalSource80021910,
    OptionsLanguageList,
    OptionsSubtitleToggle,
    PracticeHeader,
    PracticeOverlay,
    PracticeRec44Layer,
    PracticeIconRow,
    GatePracticeExitSource80023618,
    PracticeExitPrompt,
    SharedExit,
    SharedExitIconTemplate,
    GateMainDirectoryStateSource80021E60,
    StateField,
    DirectoryGp0ReplayBaseline,
    Gap,
};

struct PageDrawRouteSpec {
    DirectoryPage page = DirectoryPage::Unknown;
    int32_t eventId = 0;
    uint32_t drawFunction = 0;
    int32_t workSlot = -1;
    const char* name = nullptr;
    const char* ownerRule = nullptr;
};

struct FixedAnchorSpec {
    DirectoryPage page = DirectoryPage::Unknown;
    const char* name = nullptr;
    int16_t x = 0;
    int16_t y = 0;
    uint32_t template0 = 0;
    uint32_t template1 = 0;
    uint32_t template2 = 0;
    uint32_t tableAddress = 0;
    const char* stateRule = nullptr;
};

struct LanguageTableSpec {
    DirectoryPage page = DirectoryPage::Unknown;
    DirectoryTableKind kind = DirectoryTableKind::Unknown;
    uint32_t baseAddress = 0;
    uint32_t strideBytes = 0;
    uint32_t entryCount = 0;
    const char* recordShape = nullptr;
    const char* selectionRule = nullptr;
};

struct StagePointSpec {
    uint8_t language = 0;
    uint8_t pointIndex = 0;
    int16_t x = 0;
    int16_t y = 0;
};

struct StageSliceSpec {
    uint8_t index = 0;
    uint16_t uOffsetPx = 0;
    uint16_t widthPx = 0;
};

struct CardGridSpec {
    int16_t startX = 0;
    int16_t stepX = 0;
    int16_t startY = 0;
    int16_t stepY = 0;
    int16_t slotTextX = 0;
    int16_t slotTextY = 0;
    uint16_t glyphClutX = 0;
    int16_t glyphClutYBase = 0;
    const char* selectionRule = nullptr;
};

static constexpr uint32_t kCardGridArgAddress80048E50 = 0x80048E50u;
static constexpr std::size_t kCardGridItemCapacity80020F94 = 15u;
static constexpr std::size_t kCardGridSlotTextCapacity80020F94 = 32u;
static constexpr std::size_t kCardGridSpriteCapacity80020F94 = 490u;

struct CardGridState80020F94 {
    bool requestBound = false;
    uint32_t argAddress = 0;
    int32_t eventId = 0;
    int32_t language = 0;
    int32_t exitFrameState = 0;
    int32_t exitBlinkState = 0;
    int32_t cardIoFlag = 0;
    int16_t rows = 0;
    int16_t columns = 0;
    int16_t itemCount = 0;
    int16_t selected = 0;
    std::array<int16_t, kCardGridItemCapacity80020F94> enabled{};
    std::array<std::array<char, kCardGridSlotTextCapacity80020F94>,
               kCardGridItemCapacity80020F94>
        slotText{};
};

struct PracticeIconTemplateSpec {
    uint8_t code = 0;
    uint32_t templateAddress = 0;
    const char* name = nullptr;
};

struct DirectoryGp0ReplayBaselineSpec {
    DirectoryPage page = DirectoryPage::Unknown;
    int32_t eventId = 0;
    int32_t frame = 0;
    const char* recordingTag = nullptr;
    const char* label = nullptr;
    uint32_t packetCount = 0;
    uint32_t flatWordCount = 0;
    const char* packetStreamSha256 = nullptr;
    const char* flatWordsSha256 = nullptr;
};

struct DirectoryRenderAction {
    DirectoryRenderActionKind kind = DirectoryRenderActionKind::None;
    DirectoryPage page = DirectoryPage::Unknown;
    uint32_t psxFunction = 0;
    uint32_t tableAddress = 0;
    uint32_t templateAddress = 0;
    int32_t eventId = 0;
    int16_t x = 0;
    int16_t y = 0;
    int32_t args[4]{};
    bool conditional = false;
};

struct DirectoryRenderPlan {
    const char* name = nullptr;
    bool runtimeCutoverAllowed = false;
    bool blockedByGap = true;
    DirectoryPage page = DirectoryPage::Unknown;
    int32_t eventId = 0;
    DirectoryRenderAction actions[128]{};
    uint32_t count = 0;
    bool truncated = false;
};

static constexpr uint32_t kMainDirectoryBaseSpriteCount80021E60 = 26u;
static constexpr uint32_t kMainDirectoryChoice2FallthroughSpriteCount80021E60 =
    8u;
static constexpr uint32_t kMainDirectorySpriteCapacity80021E60 =
    kMainDirectoryBaseSpriteCount80021E60 +
    kMainDirectoryChoice2FallthroughSpriteCount80021E60;

struct MainDirectoryState80021E60 {
    bool contextPresent = true;
    int32_t language = 0;
    int32_t blinkOnOff = 0;
    int32_t cursor = 3;
    int32_t itemValue[5]{0, -1, 0, -1, 0};
    bool exitConfirmed = false;
};

struct MainDirectorySpriteTemplate8001B25C {
    bool known = false;
    uint32_t psxAddress = 0;
    uint32_t attr = 0;
    uint16_t texX = 0;
    uint16_t texY = 0;
    uint16_t width = 0;
    uint16_t height = 0;
    uint16_t clutX = 0;
    uint16_t clutY = 0;
};

enum class CardGridSpriteRole80020F94 : uint8_t {
    LanguageText = 0,
    FixedPanel,
    SlotGlyph,
    SlotMarker,
    ExitFrame,
    ExitLabel,
    ExitBar,
};

struct CardGridSpriteCommand80020F94 {
    bool known = false;
    CardGridSpriteRole80020F94 role =
        CardGridSpriteRole80020F94::LanguageText;
    uint32_t psxFunction = 0;
    uint32_t sourceCallOrder = 0;
    bool textureCoordinatesResolved = false;
    int16_t x = 0;
    int16_t y = 0;
    uint16_t priority = 0;
    uint32_t callOrder = 0;
    MainDirectorySpriteTemplate8001B25C sprite{};
    uint16_t tpage = 0;
    uint8_t u = 0;
    uint8_t v = 0;
    uint8_t glyphCode = 0;
};

struct CardGridDrawList80020F94 {
    bool sourceKnown = false;
    bool accepted = false;
    bool complete = false;
    bool runtimeSubmitAllowed = false;
    bool rawTextureOnly = false;
    bool cardIoOverlay80020A3CRequired = false;
    bool blockedByCardIoOverlay80020A3C = false;
    bool cardIoOverlayInsertIndexKnown = false;
    uint32_t cardIoOverlayInsertIndex = 0;
    int32_t cardIoMessageType = -1;
    std::array<CardGridSpriteCommand80020F94,
               kCardGridSpriteCapacity80020F94>
        commands{};
    uint32_t count = 0;
    uint32_t glyphSpriteCount = 0;
    uint32_t glyphNoPacketCount = 0;
    uint32_t markerCount = 0;
    bool truncated = false;
};

struct MainDirectorySpriteCommand80021E60 {
    bool known = false;
    int16_t x = 0;
    int16_t y = 0;
    uint16_t priority = 0;
    uint32_t callOrder = 0;
    MainDirectorySpriteTemplate8001B25C sprite{};
};

struct MainDirectoryDrawList80021E60 {
    bool sourceKnown = false;
    bool accepted = false;
    bool complete = false;
    bool rawTextureOnly = false;
    bool cardIoOverlay80020A3CRequired = false;
    bool blockedByCardIoOverlay80020A3C = false;
    bool cardIoOverlayInsertIndexKnown = false;
    bool choice2ReplayThenLoadFallthrough = false;
    uint32_t cardIoOverlayInsertIndex = 0;
    MainDirectorySpriteCommand80021E60
        commands[kMainDirectorySpriteCapacity80021E60]{};
    uint32_t count = 0;
    bool truncated = false;
};

static constexpr uint32_t kOptionsSpriteCapacity80021910 = 22u;

struct OptionsState80021910 {
    int32_t language = 0;
    int32_t blinkOnOff = 0;
    int32_t doneFlag = 0;
    int32_t cursor = 1;
    int32_t opt0Value = 0;
    int32_t opt1Value = 0;
    int32_t opt2Value = 0;
};

struct OptionsSpriteCommand80021910 {
    bool known = false;
    int16_t x = 0;
    int16_t y = 0;
    uint16_t priority = 0;
    uint32_t callOrder = 0;
    MainDirectorySpriteTemplate8001B25C sprite{};
};

struct OptionsDrawList80021910 {
    bool sourceKnown = false;
    bool accepted = false;
    bool complete = false;
    bool rawTextureOnly = false;
    OptionsSpriteCommand80021910 commands[kOptionsSpriteCapacity80021910]{};
    uint32_t count = 0;
    bool truncated = false;
};

static constexpr uint32_t kStageSelectBaseSpriteCount80020568 = 41u;
static constexpr uint32_t kStageSelectSpriteCapacity80020568 = 43u;

struct StageSelectState80020568 {
    int32_t language = 0;
    int32_t blinkOnOff = 0;
    int32_t doneFlag = 0;
    int32_t cursor = 1;
    uint8_t rawStatus80092F1DTo23[7]{};
};

struct StageSelectSpriteCommand80020568 {
    bool known = false;
    bool textureCoordinatesResolved = false;
    int16_t x = 0;
    int16_t y = 0;
    uint16_t priority = 0;
    uint32_t callOrder = 0;
    MainDirectorySpriteTemplate8001B25C sprite{};
    uint16_t tpage = 0;
    uint8_t u = 0;
    uint8_t v = 0;
};

struct StageSelectDrawList80020568 {
    bool sourceKnown = false;
    bool accepted = false;
    bool complete = false;
    bool rawTextureOnly = false;
    bool bonusOverlayPresent = false;
    StageSelectSpriteCommand80020568
        commands[kStageSelectSpriteCapacity80020568]{};
    uint32_t count = 0;
    bool truncated = false;
};

static constexpr uint32_t kPracticeDrawCallCapacity80023618 = 48u;
static constexpr uint32_t kPracticeGuideSpriteCount80023518 = 18u;
static constexpr uint32_t kPracticeSliceSpriteCount8001C604 = 4u;
static constexpr uint32_t kPracticePostSliceFixedSpriteCount80023618 = 4u;
static constexpr uint32_t kPracticePostSliceTailSpriteCount80023618 = 8u;
static constexpr uint32_t kPracticeLeadingOverlaySpriteCapacity80023618 = 10u;
static constexpr uint32_t kPracticeWobbleSlotCount80024308 = 36u;

struct PracticeGuideState80023518 {
    bool progressKnown = false;
    int32_t progress = 0;
};

struct PracticeGuideSprite80023518 {
    uint32_t psxFunction = 0;
    int32_t threshold = 0;
    int16_t x = 0;
    int16_t y = 0;
    uint32_t templateAddress = 0;
    bool staticDescriptorKnown = false;
    uint32_t attr = 0;
    uint16_t texX = 0;
    uint16_t texY = 0;
    uint16_t width = 0;
    uint16_t height = 0;
    uint16_t clutX = 0;
    uint16_t clutY = 0;
    uint16_t priority = 0;
    bool thresholdReached = false;
    uint32_t callOrder = 0;
};

struct PracticeGuideDrawList80023518 {
    bool sourceKnown = false;
    bool arithmeticSupported = false;
    bool accepted = false;
    bool complete = false;
    bool helperLevelOnly = true;
    bool runtimeSubmitAllowed = false;
    bool rendererAuthorityPublished = false;
    bool runtimeDrawPublished = false;
    bool hostRendererUsed = false;
    bool replaySourceUsed = false;
    bool staticTemplateSourcesResolved = false;
    int32_t scaledProgress = 0;
    PracticeGuideSprite80023518
        sprites[kPracticeGuideSpriteCount80023518]{};
    uint32_t count = 0;
    bool truncated = false;
};

struct PracticeIconState80024418 {
    bool argumentsKnown = false;
    bool scaleWordsKnown = false;
    bool localRgbKnown = false;
    uint8_t localR = 0;
    uint8_t localG = 0;
    uint8_t localB = 0;
    int16_t centerX = 0;
    int16_t centerY = 0;
    int16_t slotOrdinal = 0;
    int16_t type = 0;
    int16_t scaleX = 0;
    int16_t scaleY = 0;
};

struct PracticeIconSubmit80024418 {
    bool argumentsKnown = false;
    bool argumentsAccepted = false;
    bool scaleWordsKnown = false;
    bool sourceKnown = false;
    bool staticPrefixResolved = false;
    bool templateResolved = false;
    bool localRgbKnown = false;
    bool rgbSourceRequired = true;
    bool accepted = false;
    bool complete = false;
    bool helperLevelOnly = true;
    bool runtimeSubmitAllowed = false;
    bool packetSubmitAllowed = false;
    bool rendererAuthorityPublished = false;
    bool runtimeDrawPublished = false;
    bool hostRendererUsed = false;
    bool replaySourceUsed = false;
    uint8_t localR = 0;
    uint8_t localG = 0;
    uint8_t localB = 0;
    uint32_t templateTableBase = 0;
    uint32_t templateSourceAddress = 0;
    uint32_t templateAddress = 0;
    uint32_t scaleXSourceAddress = 0;
    uint32_t scaleYSourceAddress = 0;
    uint32_t localAttr = 0;
    int16_t centerX = 0;
    int16_t centerY = 0;
    int16_t localX = 0;
    int16_t localY = 0;
    uint16_t texX = 0;
    uint16_t texY = 0;
    uint16_t width = 0;
    uint16_t height = 0;
    uint16_t clutX = 0;
    uint16_t clutY = 0;
    int16_t pivotX = 0;
    int16_t pivotY = 0;
    int16_t scaleX = 0;
    int16_t scaleY = 0;
    uint16_t priority = 0;
};

struct PracticeIconDirectRenderPayload80024418 {
    bool sourceKnown = false;
    bool graphControlKnown = false;
    bool gsSortSpriteEvaluated = false;
    bool fastPacketPath = false;
    bool transformPacketPath = false;
    bool transformGeometryExactRtpt = false;
    bool drawEnvOffsetApplied = false;
    bool renderPayloadKnown = false;
    bool packetOtMutationPublished = false;
    bool replaySourceUsed = false;
    uint32_t attr = 0;
    uint32_t packetWordCount = 0;
    uint16_t priority = 0;
    uint16_t tpage = 0;
    uint16_t clut = 0;
    std::array<uint8_t, 4> u{};
    std::array<uint8_t, 4> v{};
    std::array<int16_t, 4> x{};
    std::array<int16_t, 4> y{};
    // 8003F1B4 applies the active GS draw-environment page offset to the
    // packet XY words.  Preserve that source offset so the host can map the
    // packet back into the visible 320x240 logical page when the second
    // double-buffer lane is active (Y=360 on the PSX packet).
    int16_t drawOffsetX = 0;
    int16_t drawOffsetY = 0;
    uint16_t width = 0;
    uint16_t height = 0;
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
};

struct PracticeWobbleSlot80023F20 {
    int32_t counter = 0;
    int32_t phase = 0;
    int32_t linearAcc = 0;
    int32_t linearVel = 0;
};

struct PracticeWobbleBank80023F20 {
    bool initialized = false;
    int16_t scaleX[kPracticeWobbleSlotCount80024308]{};
    int16_t scaleY[kPracticeWobbleSlotCount80024308]{};
    PracticeWobbleSlot80023F20
        slots[kPracticeWobbleSlotCount80024308]{};
};

struct PracticePortraitState80024600 {
    bool argumentsKnown = false;
    int32_t row = 0;
    int32_t baseX = 0;
    int32_t baseY = 0;
    int32_t maxRepeatFrame = 0;
    int32_t value = 0;
};

struct PracticePortraitSubmit80024600 {
    bool argumentsKnown = false;
    bool argumentsAccepted = false;
    bool templateKnown = false;
    bool sourceKnown = false;
    bool accepted = false;
    bool complete = false;
    bool drawRequested = false;
    bool helperLevelOnly = true;
    bool runtimeSubmitAllowed = false;
    bool rendererAuthorityPublished = false;
    bool runtimeDrawPublished = false;
    bool hostRendererUsed = false;
    bool replaySourceUsed = false;
    uint32_t psxFunction = kFn8001C550;
    uint32_t repeatFrameSourceAddress =
        kGpPracticePortraitRepeatFrame8006EB70;
    uint32_t lastValueSourceAddress =
        kGpPracticePortraitLastValue8006EB74;
    uint32_t templateSourceAddress =
        kGpPracticePortraitTemplate8006ED5C;
    uint32_t templateAddress = 0;
    bool staticDescriptorKnown = false;
    uint32_t attr = 0;
    uint16_t texX = 0;
    uint16_t texY = 0;
    uint16_t width = 0;
    uint16_t height = 0;
    uint16_t clutX = 0;
    uint16_t clutY = 0;
    int32_t value = 0;
    int32_t repeatFrame = 0;
    int16_t x = 0;
    int16_t y = 0;
    uint16_t priority = 0;
};

struct PracticePortraitBank80024600 {
    bool initialized = false;
    int32_t repeatFrame = 0;
    int32_t lastValue = 0;
    bool templateKnown = false;
    uint32_t templateAddress = 0;
};

struct PracticeSliceProducerState8001C604 {
    bool eventDispatchKnown = false;
    uint8_t eventId = 0;
    bool precedingPortraitDrawProducerKnown = false;
};

struct PracticeSliceSprite8001C604 {
    bool known = false;
    bool staticDescriptorKnown = false;
    bool textureCoordinatesResolved = false;
    bool localRgbKnown = false;
    uint32_t psxFunction = kFn8001C604;
    uint32_t templateAddress = 0;
    uint32_t attr = 0;
    uint16_t texX = 0;
    uint16_t texY = 0;
    uint16_t width = 0;
    uint16_t height = 0;
    uint16_t clutX = 0;
    uint16_t clutY = 0;
    uint16_t tpage = 0;
    uint8_t u = 0;
    uint8_t v = 0;
    uint8_t localR = 0;
    uint8_t localG = 0;
    uint8_t localB = 0;
    int16_t x = 0;
    int16_t y = 0;
    int16_t uOffset = 0;
    uint16_t priority = 0;
    uint32_t callOrder = 0;
};

struct PracticeSliceDrawList8001C604 {
    bool eventDispatchKnown = false;
    bool eventIdAccepted = false;
    bool precedingPortraitDrawProducerKnown = false;
    bool staticWriterSemanticsKnown = false;
    bool localRgbKnown = false;
    bool sourceKnown = false;
    bool accepted = false;
    bool complete = false;
    bool rawTextureOnly = true;
    bool runtimeSubmitAllowed = false;
    bool rendererAuthorityPublished = false;
    bool runtimeDrawPublished = false;
    bool packetOtMutationPublished = false;
    bool hostRendererUsed = false;
    bool replaySourceUsed = false;
    PracticeSliceSprite8001C604
        sprites[kPracticeSliceSpriteCount8001C604]{};
    uint32_t count = 0;
    bool truncated = false;
};

struct PracticePostSliceFixedProducerState80023618 {
    bool eventDispatchKnown = false;
    uint8_t eventId = 0;
    bool precedingSliceDrawPublished = false;
};

struct PracticePostSliceFixedSprite8001C550 {
    bool known = false;
    uint32_t psxFunction = kFn8001C550;
    int16_t x = 0;
    int16_t y = 0;
    uint16_t priority = 0;
    uint32_t callOrder = 0;
    MainDirectorySpriteTemplate8001B25C sprite{};
};

struct PracticePostSliceFixedDrawList80023618 {
    bool eventDispatchKnown = false;
    bool eventIdAccepted = false;
    bool precedingSliceDrawPublished = false;
    bool staticDescriptorSourcesKnown = false;
    bool sourceKnown = false;
    bool accepted = false;
    bool complete = false;
    bool rawTextureOnly = true;
    bool runtimeSubmitAllowed = false;
    bool rendererAuthorityPublished = false;
    bool runtimeDrawPublished = false;
    bool packetOtMutationPublished = false;
    bool hostRendererUsed = false;
    bool replaySourceUsed = false;
    PracticePostSliceFixedSprite8001C550
        sprites[kPracticePostSliceFixedSpriteCount80023618]{};
    uint32_t count = 0;
    bool truncated = false;
};

struct PracticePostSliceTailProducerState80023618 {
    bool eventDispatchKnown = false;
    uint8_t eventId = 0;
    bool precedingSliceDrawPublished = false;
    bool languageKnown = false;
    int32_t language = 0;
    bool exitSelectionKnown = false;
    int32_t exitSelection48 = 0;
    bool exitBlinkKnown = false;
    int16_t exitBlink4C = 0;
};

struct PracticePostSliceTailSprite80023618 {
    bool known = false;
    uint32_t psxFunction = 0;
    int16_t x = 0;
    int16_t y = 0;
    uint16_t priority = 0;
    uint32_t callOrder = 0;
    MainDirectorySpriteTemplate8001B25C sprite{};
};

struct PracticePostSliceTailDrawList80023618 {
    bool eventDispatchKnown = false;
    bool eventIdAccepted = false;
    bool precedingSliceDrawPublished = false;
    // The source 80023618 tail is a permanent page layer (caption, fixed
    // character/title sprites and EXIT), not conditional on the optional
    // Rec44 slice lane.  Keep this provenance bit distinct from the legacy
    // post-slice transaction so the entry/intro frame can submit the same
    // fixed tail before a slice has been produced.
    bool alwaysVisibleFixedTail = false;
    bool languageAccepted = false;
    bool exitSelectionAccepted = false;
    bool exitBlinkAccepted = false;
    bool staticWriterSemanticsKnown = false;
    bool staticDescriptorSourcesKnown = false;
    bool sourceKnown = false;
    bool accepted = false;
    bool complete = false;
    bool rawTextureOnly = true;
    bool runtimeSubmitAllowed = false;
    bool rendererAuthorityPublished = false;
    bool runtimeDrawPublished = false;
    bool packetOtMutationPublished = false;
    bool hostRendererUsed = false;
    bool replaySourceUsed = false;
    PracticePostSliceTailSprite80023618
        sprites[kPracticePostSliceTailSpriteCount80023618]{};
    uint32_t count = 0;
    bool truncated = false;
};

struct PracticeLeadingOverlayProducerState80023618 {
    bool eventDispatchKnown = false;
    uint8_t eventId = 0;
    bool languageKnown = false;
    int32_t language = 0;
    bool flagsKnown = false;
    uint32_t flags00 = 0;
    bool overlayStateKnown = false;
    int32_t overlayState1C = 0;
};

struct PracticeLeadingOverlaySprite80023618 {
    bool known = false;
    uint32_t psxFunction = 0;
    int16_t x = 0;
    int16_t y = 0;
    uint16_t priority = 0;
    uint32_t callOrder = 0;
    MainDirectorySpriteTemplate8001B25C sprite{};
};

struct PracticeLeadingOverlayDrawList80023618 {
    bool eventDispatchKnown = false;
    bool eventIdAccepted = false;
    bool languageAccepted = false;
    bool flagsAccepted = false;
    bool overlayGateActive = false;
    bool overlayStateAccepted = false;
    bool staticWriterSemanticsKnown = false;
    bool staticDescriptorSourcesKnown = false;
    bool sourceKnown = false;
    bool accepted = false;
    bool complete = false;
    bool rawTextureOnly = true;
    bool runtimeSubmitAllowed = false;
    bool rendererAuthorityPublished = false;
    bool runtimeDrawPublished = false;
    bool packetOtMutationPublished = false;
    bool hostRendererUsed = false;
    bool replaySourceUsed = false;
    PracticeLeadingOverlaySprite80023618
        sprites[kPracticeLeadingOverlaySpriteCapacity80023618]{};
    uint32_t count = 0;
    bool truncated = false;
};

struct PracticeState80023618 {
    bool contextPresent = true;
    int32_t language = 0;
    uint32_t flags00 = 0;
    int32_t overlayState1C = 0;
    int16_t laneA8C = -1;
    int16_t laneB9E = -1;
    bool iconBytesKnown = true;
    uint8_t iconCodes[18]{};
    int32_t exitSelection48 = 0;
    int16_t exitBlink4C = 0;
};

enum class PracticeDrawCallKind80023618 : uint8_t {
    None = 0,
    FastSprite8001C5A8,
    Sprite8001C550,
    WobbleUpdate80023F20,
    Rec44Mode800246A8,
    Rec44Draw80024600,
    Guide80023518,
    Icon80024418,
    Slice8001C604,
};

struct PracticeDrawCall80023618 {
    PracticeDrawCallKind80023618 kind =
        PracticeDrawCallKind80023618::None;
    uint32_t psxFunction = 0;
    uint32_t positionSourceAddress = 0;
    uint32_t templateSourceAddress = 0;
    bool templatePointerIndirect = false;
    bool positionResolved = false;
    int16_t resolvedX = 0;
    int16_t resolvedY = 0;
    bool templateResolved = false;
    uint32_t resolvedTemplateAddress = 0;
    bool dynamicPositionRequired = false;
    bool dynamicScaleRequired = false;
    uint32_t scaleXSourceAddress = 0;
    uint32_t scaleYSourceAddress = 0;
    bool helperStaticPrefixResolved = false;
    bool helperExpansionResolved = false;
    uint32_t helperExpandedCallCount = 0;
    int32_t args[8]{};
    uint32_t callOrder = 0;
};

struct PracticeDrawCallTrace80023618 {
    bool sourceKnown = false;
    bool accepted = false;
    bool complete = false;
    bool helperLevelOnly = true;
    bool runtimeSubmitAllowed = false;
    bool rendererAuthorityPublished = false;
    bool runtimeDrawPublished = false;
    bool hostRendererUsed = false;
    bool replaySourceUsed = false;
    bool staticOperandSourcesResolved = false;
    PracticeDrawCall80023618 calls[kPracticeDrawCallCapacity80023618]{};
    uint32_t count = 0;
    bool truncated = false;
};

bool RuntimeCutoverAllowed();

uint32_t KnownPageDrawRouteSpecCount();
const PageDrawRouteSpec& KnownPageDrawRouteSpecAt(uint32_t index);
const PageDrawRouteSpec* FindPageDrawRouteSpec(int32_t eventId);

uint32_t KnownFixedAnchorSpecCount();
const FixedAnchorSpec& KnownFixedAnchorSpecAt(uint32_t index);

uint32_t KnownLanguageTableSpecCount();
const LanguageTableSpec& KnownLanguageTableSpecAt(uint32_t index);

uint32_t KnownStagePointSpecCount();
const StagePointSpec& KnownStagePointSpecAt(uint32_t index);

uint32_t KnownStageSliceSpecCount();
const StageSliceSpec& KnownStageSliceSpecAt(uint32_t index);

CardGridSpec KnownCardGridSpec();

uint32_t KnownPracticeIconTemplateSpecCount();
const PracticeIconTemplateSpec& KnownPracticeIconTemplateSpecAt(
    uint32_t index);

uint32_t KnownDirectoryGp0ReplayBaselineSpecCount();
const DirectoryGp0ReplayBaselineSpec& KnownDirectoryGp0ReplayBaselineSpecAt(
    uint32_t index);

DirectoryRenderPlan BuildMainDirectory80021E60Plan(uint32_t ctxAddress = 0,
                                                   bool ctxKnown = false);
MainDirectoryDrawList80021E60 BuildMainDirectoryDrawList80021E60(
    const MainDirectoryState80021E60& state);
OptionsDrawList80021910 BuildOptionsDrawList80021910(
    const OptionsState80021910& state);
StageSelectDrawList80020568 BuildStageSelectDrawList80020568(
    const StageSelectState80020568& state);
CardGridDrawList80020F94 BuildCardGridDrawList80020F94(
    const CardGridState80020F94& state);
PracticeGuideDrawList80023518 BuildPracticeGuideDrawList80023518(
    const PracticeGuideState80023518& state);
PracticeIconSubmit80024418 BuildPracticeIconSubmit80024418(
    const PracticeIconState80024418& state);
void ResetPracticeWobbleBank80024308(
    PracticeWobbleBank80023F20& bank);
bool UpdatePracticeWobbleBank80023F20(
    PracticeWobbleBank80023F20& bank,
    int32_t count);
bool ReadPracticeWobbleScale80024418(
    const PracticeWobbleBank80023F20& bank,
    int32_t slotOrdinal,
    int16_t& scaleX,
    int16_t& scaleY);
PracticeIconSubmit80024418 BuildPracticeIconSubmitFromWobbleBank80024418(
    const PracticeWobbleBank80023F20& bank,
    int16_t centerX,
    int16_t centerY,
    int16_t slotOrdinal,
    int16_t type);
PracticeIconSubmit80024418
BuildPracticeIconSubmitFromWobbleBankAndRgb80024418(
    const PracticeWobbleBank80023F20& bank,
    int16_t centerX,
    int16_t centerY,
    int16_t slotOrdinal,
    int16_t type,
    uint8_t localR,
    uint8_t localG,
    uint8_t localB);
PracticeIconDirectRenderPayload80024418
BuildPracticeIconDirectRenderPayload80024418(
    const PracticeIconSubmit80024418& submit,
    const PrPsxGraphOwnerDirect::PsxGraphState* graph);
void ResetPracticePortraitBank80024600(
    PracticePortraitBank80024600& bank);
bool SelectPracticePortraitTemplate800246A8(
    PracticePortraitBank80024600& bank,
    int32_t mode);
PracticePortraitSubmit80024600 UpdatePracticePortraitBank80024600(
    PracticePortraitBank80024600& bank,
    const PracticePortraitState80024600& state);
PracticeSliceDrawList8001C604 BuildPracticeSliceDrawList8001C604(
    const PracticeSliceProducerState8001C604& state);
PracticePostSliceFixedDrawList80023618
BuildPracticePostSliceFixedDrawList80023618(
    const PracticePostSliceFixedProducerState80023618& state);
PracticePostSliceTailDrawList80023618
BuildPracticePostSliceTailDrawList80023618(
    const PracticePostSliceTailProducerState80023618& state);
PracticePostSliceTailDrawList80023618
BuildPracticeAlwaysVisibleTailDrawList80023618(
    const PracticePostSliceTailProducerState80023618& state);
PracticeLeadingOverlayDrawList80023618
BuildPracticeLeadingOverlayDrawList80023618(
    const PracticeLeadingOverlayProducerState80023618& state);
PracticeDrawCallTrace80023618 BuildPracticeDrawCallTrace80023618(
    const PracticeState80023618& state);
DirectoryRenderPlan BuildStageSelect80020568Plan();
DirectoryRenderPlan BuildCardGrid80020F94Plan(int32_t eventId,
                                              uint32_t argAddress = 0,
                                              bool argKnown = false);
DirectoryRenderPlan BuildOptions80021910Plan();
DirectoryRenderPlan BuildPracticeDraw80023618Plan(uint32_t ctxAddress = 0,
                                                  bool ctxKnown = false);
DirectoryRenderPlan BuildDirectoryPageDrawPlan(int32_t eventId);

DirectoryPage DirectoryPageFromEventId(int32_t eventId);
const char* DirectoryPageName(DirectoryPage page);
const char* DirectoryTableKindName(DirectoryTableKind kind);
const char* DirectoryRenderActionKindName(DirectoryRenderActionKind kind);

} // namespace PrSS0DirectoryPagesRenderDirect
