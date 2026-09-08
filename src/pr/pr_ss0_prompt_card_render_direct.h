#pragma once

#include <cstdint>

namespace PrSS0PromptCardRenderDirect {

static constexpr uint32_t kFn800203D4 = 0x800203D4u;
static constexpr uint32_t kFn80020A3C = 0x80020A3Cu;
static constexpr uint32_t kFn80020BE4 = 0x80020BE4u;
static constexpr uint32_t kFn80021594 = 0x80021594u;
static constexpr uint32_t kFn80022CBC = 0x80022CBCu;
static constexpr uint32_t kFn80017E6C = 0x80017E6Cu;
static constexpr uint32_t kFn8001C550 = 0x8001C550u;
static constexpr uint32_t kFn8001B590 = 0x8001B590u;
static constexpr uint32_t kFn8001B25C = 0x8001B25Cu;
static constexpr uint32_t kFn8003FA20 = 0x8003FA20u;
static constexpr uint32_t kFn8001C5A8 = 0x8001C5A8u;
static constexpr uint32_t kFn8001C668 = 0x8001C668u;
static constexpr uint32_t kFn8001C7A8 = 0x8001C7A8u;
static constexpr uint32_t kFn8001B730 = 0x8001B730u;
static constexpr uint32_t kFn8001C6E0 = 0x8001C6E0u;
static constexpr uint32_t kFn8001C4EC = 0x8001C4ECu;

static constexpr uint32_t kWorkListBase80087288 = 0x80087288u;
static constexpr uint32_t kWorkListStride80087288 = 20u;
static constexpr int32_t kEvent4PromptSpriteCount800203D4 = 3;
static constexpr int32_t kFastSpritePacketWords8003FA20 = 6;
static constexpr int32_t kFastSpritePacketAdvance8003FA20 = 0x18;
static constexpr int32_t kFastSpriteTemplateLastWrittenOffset8001B25C = 0x12;
static constexpr int32_t kFastSpriteLocalRgbR8003FA20 = 0x14;
static constexpr int32_t kFastSpriteLocalRgbG8003FA20 = 0x15;
static constexpr int32_t kFastSpriteLocalRgbB8003FA20 = 0x16;

static constexpr uint32_t kTableSharedExitText80053000 = 0x80053000u;
static constexpr uint32_t kTableCardInfoMarkerSprites800532AC = 0x800532ACu;
static constexpr uint32_t kTableCardInfoMarkerPositions800532B0 = 0x800532B0u;
static constexpr uint32_t kTableCardInfoLowerTopText8005349C = 0x8005349Cu;
static constexpr uint32_t kTableCardInfoLowerBottomText800534EC = 0x800534ECu;
static constexpr uint32_t kTableHiScoreCellStatusSprites8005353C = 0x8005353Cu;
static constexpr uint32_t kTableHiScoreTitleText80053F84 = 0x80053F84u;
static constexpr uint32_t kTableHiScoreTitlePos80053F88 = 0x80053F88u;
static constexpr uint32_t kTableSharedExitLabelOff80053004 = 0x80053004u;
static constexpr uint32_t kTableSharedExitLabelOn80053008 = 0x80053008u;
static constexpr uint32_t kTableSharedExitLabelPos8005300C = 0x8005300Cu;
static constexpr uint32_t kTableSharedExitBarPos80052FFC = 0x80052FFCu;
static constexpr uint32_t kGpBase8006EA40 = 0x8006EA40u;
static constexpr uint32_t kGpSharedExitIconSlotOn8006EB10 = 0x8006EB10u;
static constexpr uint32_t kGpSharedExitIconSlotOff8006EB14 = 0x8006EB14u;
static constexpr uint32_t kTablePromptType2Header80053990 = 0x80053990u;
static constexpr uint32_t kTablePromptType3Header800539B8 = 0x800539B8u;
static constexpr uint32_t kTablePromptType7Header800539E0 = 0x800539E0u;
static constexpr uint32_t kTablePromptType1Header80053A08 = 0x80053A08u;
static constexpr uint32_t kTablePromptType4Header80053A30 = 0x80053A30u;
static constexpr uint32_t kTablePromptType5Header80053A58 = 0x80053A58u;
static constexpr uint32_t kTablePromptType6Header80053A80 = 0x80053A80u;
static constexpr uint32_t kTablePromptChoice0_80053AAC = 0x80053AACu;
static constexpr uint32_t kTablePromptChoice1_80053AFC = 0x80053AFCu;
static constexpr uint32_t kTableCardIoRemoveText8005326C = 0x8005326Cu;
static constexpr uint32_t kTableCardIoPleaseWaitText80053280 = 0x80053280u;
static constexpr uint32_t kTextCardIoNowSaving800111CC = 0x800111CCu;
static constexpr uint32_t kSpriteCardIoCornerTopLeft80050900 = 0x80050900u;
static constexpr uint32_t kSpriteCardIoCornerBottomLeft800508F0 = 0x800508F0u;
static constexpr uint32_t kSpriteCardIoCornerTopRight800508E0 = 0x800508E0u;
static constexpr uint32_t kSpriteCardIoCornerBottomRight800508D0 = 0x800508D0u;
static constexpr uint32_t kSpriteCardInfoLowerMidDefault80051FE0 = 0x80051FE0u;
static constexpr uint32_t kSpriteCardInfoLowerMidSelected80051FF0 = 0x80051FF0u;
static constexpr uint32_t kSpriteCardInfoLowerMidMode1_80052000 = 0x80052000u;
static constexpr uint32_t kSpriteCardInfoLowerFinalDefault80052120 = 0x80052120u;
static constexpr uint32_t kSpriteCardInfoLowerFinalMode1_80052130 = 0x80052130u;
static constexpr uint32_t kSpriteCardInfoLowerFinalMode2_80052140 = 0x80052140u;
static constexpr uint32_t kSpriteHiScoreTitlePanel80052E50 = 0x80052E50u;
static constexpr uint32_t kSpriteHiScoreColumn0_80052EB0 = 0x80052EB0u;
static constexpr uint32_t kSpriteHiScoreColumn1_80052EC0 = 0x80052EC0u;
static constexpr uint32_t kSpriteHiScoreColumn2_80052ED0 = 0x80052ED0u;
static constexpr uint32_t kSpriteHiScoreCellNonEmpty80052EE0 = 0x80052EE0u;
static constexpr uint32_t kSpriteHiScoreCellEmpty80052F00 = 0x80052F00u;
static constexpr uint32_t kSpriteHiScoreRow0_80052F10 = 0x80052F10u;
static constexpr uint32_t kSpriteSharedExitIconOff80050AC0 = 0x80050AC0u;
static constexpr uint32_t kSpriteSharedExitIconOn80050AD0 = 0x80050AD0u;
static constexpr uint32_t kSpriteHiScoreExitBarOff800509B0 = 0x800509B0u;
static constexpr uint32_t kSpriteHiScoreExitBarOn800509C0 = 0x800509C0u;

static constexpr int32_t kCardInfoMarkerCount80020BE4 = 57;
static constexpr int32_t kCardInfoMarkerStride80020BE4 = 8;
static constexpr int32_t kCardInfoMarkerSelectedOffset80020BE4 = 0x16;
static constexpr int32_t kCardInfoStringOffset80020BE4 = 0x28;
static constexpr int32_t kCardInfoIoFlagOffset80020BE4 = 0x08;
static constexpr int32_t kCardInfoLowerModeOffset80020BE4 = 0x1A;
static constexpr int32_t kCardInfoLowerTopFlagOffset80020BE4 = 0x00;
static constexpr int32_t kCardInfoMarkerLastIndex80020BE4 = 56;
static constexpr int32_t kCardInfoLowerGpTemplateSlotDefault80020BE4 = 0xD8;
static constexpr int32_t kCardInfoLowerGpTemplateSlotActive80020BE4 = 0xDC;
static constexpr int32_t kHiScoreRowsOffset80021594 = 0x0C;
static constexpr int32_t kHiScoreColsOffset80021594 = 0x0E;
static constexpr int32_t kHiScoreExitIconStateOffset80021594 = 0x00;
static constexpr int32_t kHiScoreExitLabelStateOffset80021594 = 0x04;
static constexpr int32_t kHiScoreCellBaseOffset80021594 = 0x14;
static constexpr int32_t kHiScoreCellStride80021594 = 0x10;
static constexpr int32_t kHiScoreMaxRows80021594 = 6;
static constexpr int32_t kHiScoreMaxCols80021594 = 3;
static constexpr int32_t kHiScoreCellTextX80021594 = 57;
static constexpr int32_t kHiScoreCellTextY80021594 = 72;
static constexpr int32_t kHiScoreCellIconX80021594 = 55;
static constexpr int32_t kHiScoreCellIconY80021594 = 68;
static constexpr int32_t kHiScoreCellColStep80021594 = 79;
static constexpr int32_t kHiScoreCellRowStep80021594 = 18;
static constexpr int32_t kHiScoreExitIconGpSlotOn80021594 = 0xD0;
static constexpr int32_t kHiScoreExitIconGpSlotOff80021594 = 0xD4;
static constexpr int32_t kHiScoreGp0ReplayBaselineCount = 3;
static constexpr int32_t kPromptCardGp0ReplayBaselineCount = 16;
static constexpr int32_t kCardIoLanguageCount80020A3C = 5;
static constexpr int32_t kCardIoBannerTextX80020A3C = 30;
static constexpr int32_t kCardIoBannerTextY80020A3C = 121;
static constexpr int32_t kCardIoBannerTextOt80020A3C = 480;
static constexpr int32_t kCardIoBannerScreenWidth80020A3C = 320;
static constexpr int32_t kCardIoBannerPadding80020A3C = 8;
static constexpr int32_t kCardIoBannerFillColor80020A3C = 0x400F0F0F;

enum class PromptCardPage : uint8_t {
    Unknown = 0,
    Event4Prompt,
    CardInfoEv5,
    HiScoreEv6,
    PromptType1,
    PromptType2,
    PromptType3,
    PromptType4,
    PromptType5,
    PromptType6,
    PromptType7,
};

enum class PromptCardTableKind : uint8_t {
    Unknown = 0,
    Event4PromptTemplates,
    CardInfoMarkerSprites,
    CardInfoMarkerPositions,
    HiScoreLayout,
    SharedExitText,
    PromptHeaderType1,
    PromptHeaderType2,
    PromptHeaderType3,
    PromptHeaderType4,
    PromptHeaderType5,
    PromptHeaderType6,
    PromptHeaderType7,
    PromptChoice0,
    PromptChoice1,
    CardIoText,
};

enum class PromptCardActionKind : uint8_t {
    None = 0,
    DrawRoute,
    FixedSprite,
    LanguageTable,
    SharedExit,
    GateEvent4ChoiceSource800203D4,
    Event4ChoiceState,
    Event4PromptFastSpriteChain8001C550,
    Event4PromptLocalSprite8001B590,
    Event4PromptTemplateCopy8001B25C,
    GateEvent4PromptRgbTail8003FA20,
    Event4PromptPacketIntent8003FA20,
    GateCardInfoArgSource80020BE4,
    CardInfoMarkerStrip,
    CardInfoMarkerEntry,
    CardInfoStringBuffer,
    GateCardInfoLowerState80020BE4,
    CardInfoLowerBranch,
    CardInfoLowerLanguageText,
    CardInfoLowerFixedSprite,
    GateHiScoreArgSource80021594,
    HiScoreRows,
    HiScoreCellGrid,
    HiScoreCellTextEntry,
    HiScoreCellEntry,
    HiScoreLanguageTitle,
    HiScoreFixedSprite,
    GateHiScoreExitState80021594,
    HiScoreExitLabel,
    HiScoreExitBar,
    HiScoreGp0ReplayBaseline,
    PromptCardGp0ReplayBaseline,
    GatePromptTypeRoute80022CBC,
    PromptHeader,
    PromptSingleButton,
    PromptDualChoice,
    PromptChoiceState,
    GatePromptFlashSource80017E6C,
    PromptFlash20Frames,
    CardIoPrompt,
    GateCardIoMsgSource80020A3C,
    CardIoBannerText,
    CardIoBannerLayout,
    CardIoBannerBoxFill,
    CardIoBannerCornerSprite,
    Gap,
};

struct PromptCardRouteSpec {
    PromptCardPage page = PromptCardPage::Unknown;
    int32_t eventId = 0;
    int32_t promptType = 0;
    uint32_t drawFunction = 0;
    const char* name = nullptr;
    const char* ownerRule = nullptr;
};

struct PromptCardAnchorSpec {
    PromptCardPage page = PromptCardPage::Unknown;
    const char* name = nullptr;
    int16_t x = 0;
    int16_t y = 0;
    uint32_t template0 = 0;
    uint32_t template1 = 0;
    uint32_t tableAddress = 0;
    const char* stateRule = nullptr;
};

struct PromptHeaderTableSpec {
    PromptCardPage page = PromptCardPage::Unknown;
    int32_t promptType = 0;
    uint32_t tableAddress = 0;
    uint32_t strideBytes = 0;
    uint32_t entryCount = 0;
    const char* semantic = nullptr;
};

struct PromptHeaderEntrySpec {
    int32_t promptType = 0;
    uint8_t language = 0;
    uint32_t templateAddress = 0;
    int16_t x = 0;
    int16_t y = 0;
};

struct PromptChoiceEntrySpec {
    uint8_t group = 0;
    uint8_t language = 0;
    uint32_t templateA = 0;
    uint32_t templateB = 0;
    int16_t x = 0;
    int16_t y = 0;
};

struct CardInfoMarkerEntrySpec {
    uint8_t index = 0;
    uint32_t spriteTemplate = 0;
    int16_t x = 0;
    int16_t y = 0;
};

struct CardIoTextEntrySpec {
    int32_t msgType = 0;
    uint8_t language = 0;
    uint32_t tableAddress = 0;
    uint32_t textAddress = 0;
    const char* semantic = nullptr;
};

struct CardInfoLowerTextEntrySpec {
    uint8_t group = 0;
    uint8_t language = 0;
    uint32_t recordAddress = 0;
    uint32_t defaultTemplate = 0;
    uint32_t selectedTemplate = 0;
    uint32_t forcedTemplate = 0;
    int16_t x = 0;
    int16_t y = 0;
};

struct HiScoreLanguageTitleEntrySpec {
    uint8_t language = 0;
    uint32_t templateAddress = 0;
    int16_t x = 0;
    int16_t y = 0;
};

struct HiScoreExitLabelEntrySpec {
    uint8_t language = 0;
    uint32_t offTemplate = 0;
    uint32_t onTemplate = 0;
    int16_t x = 0;
    int16_t y = 0;
};

struct HiScoreFixedSpriteSpec {
    uint8_t group = 0;
    uint8_t index = 0;
    uint32_t templateAddress = 0;
    int16_t x = 0;
    int16_t y = 0;
};

struct HiScoreGp0ReplayBaselineSpec {
    int32_t frame = 0;
    const char* label = nullptr;
    uint32_t packetCount = 0;
    uint32_t flatWordCount = 0;
    const char* packetStreamSha256 = nullptr;
    const char* flatWordsSha256 = nullptr;
};

struct PromptCardGp0ReplayBaselineSpec {
    PromptCardPage page = PromptCardPage::Unknown;
    int32_t eventId = 0;
    int32_t promptType = 0;
    int32_t frame = 0;
    const char* tag = nullptr;
    const char* label = nullptr;
    uint32_t packetCount = 0;
    uint32_t flatWordCount = 0;
    const char* packetStreamSha256 = nullptr;
    const char* flatWordsSha256 = nullptr;
};

struct PromptCardAction {
    PromptCardActionKind kind = PromptCardActionKind::None;
    PromptCardPage page = PromptCardPage::Unknown;
    uint32_t psxFunction = 0;
    uint32_t tableAddress = 0;
    uint32_t templateAddress = 0;
    int32_t eventId = 0;
    int32_t promptType = 0;
    int16_t x = 0;
    int16_t y = 0;
    int32_t args[4]{};
    bool conditional = false;
};

struct PromptCardPlan {
    const char* name = nullptr;
    bool runtimeCutoverAllowed = false;
    bool blockedByGap = true;
    PromptCardPage page = PromptCardPage::Unknown;
    int32_t eventId = 0;
    int32_t promptType = 0;
    PromptCardAction actions[128]{};
    uint32_t count = 0;
    bool truncated = false;
};

bool RuntimeCutoverAllowed();

uint32_t KnownPromptCardRouteSpecCount();
const PromptCardRouteSpec& KnownPromptCardRouteSpecAt(uint32_t index);
const PromptCardRouteSpec* FindPromptCardRouteSpecByEvent(int32_t eventId);
const PromptCardRouteSpec* FindPromptCardRouteSpecByType(int32_t type);

uint32_t KnownPromptCardAnchorSpecCount();
const PromptCardAnchorSpec& KnownPromptCardAnchorSpecAt(uint32_t index);

uint32_t KnownPromptHeaderTableSpecCount();
const PromptHeaderTableSpec& KnownPromptHeaderTableSpecAt(uint32_t index);

uint32_t KnownPromptHeaderEntrySpecCount();
const PromptHeaderEntrySpec& KnownPromptHeaderEntrySpecAt(uint32_t index);

uint32_t KnownPromptChoiceEntrySpecCount();
const PromptChoiceEntrySpec& KnownPromptChoiceEntrySpecAt(uint32_t index);

uint32_t KnownCardInfoMarkerEntrySpecCount();
const CardInfoMarkerEntrySpec& KnownCardInfoMarkerEntrySpecAt(uint32_t index);

uint32_t KnownCardIoTextEntrySpecCount();
const CardIoTextEntrySpec& KnownCardIoTextEntrySpecAt(uint32_t index);

uint32_t KnownCardInfoLowerTextEntrySpecCount();
const CardInfoLowerTextEntrySpec& KnownCardInfoLowerTextEntrySpecAt(uint32_t index);

uint32_t KnownHiScoreLanguageTitleEntrySpecCount();
const HiScoreLanguageTitleEntrySpec& KnownHiScoreLanguageTitleEntrySpecAt(uint32_t index);

uint32_t KnownHiScoreExitLabelEntrySpecCount();
const HiScoreExitLabelEntrySpec& KnownHiScoreExitLabelEntrySpecAt(uint32_t index);

uint32_t KnownHiScoreFixedSpriteSpecCount();
const HiScoreFixedSpriteSpec& KnownHiScoreFixedSpriteSpecAt(uint32_t index);

uint32_t KnownHiScoreGp0ReplayBaselineSpecCount();
const HiScoreGp0ReplayBaselineSpec& KnownHiScoreGp0ReplayBaselineSpecAt(uint32_t index);

uint32_t KnownPromptCardGp0ReplayBaselineSpecCount();
const PromptCardGp0ReplayBaselineSpec& KnownPromptCardGp0ReplayBaselineSpecAt(
    uint32_t index);

PromptCardPlan BuildEvent4Prompt800203D4Plan(int32_t ctx0 = -1,
                                             bool choiceSourceKnown = false);
PromptCardPlan BuildCardInfo80020BE4Plan(uint32_t argAddress = 0,
                                         bool argKnown = false,
                                         int32_t selectedMarker = -1,
                                         int32_t lowerMode = -1,
                                         int32_t topFlag = 0,
                                         bool topFlagKnown = false,
                                         bool lowerModeKnown = false);
PromptCardPlan BuildHiScore80021594Plan(uint32_t argAddress = 0,
                                        bool argKnown = false,
                                        int32_t rows = -1,
                                        int32_t cols = -1,
                                        int32_t exitIconState = -1,
                                        int32_t exitLabelState = -1,
                                        bool exitIconStateKnown = false,
                                        bool exitLabelStateKnown = false);
PromptCardPlan BuildCardIoBanner80020A3CPlan(int32_t msgType = -1,
                                             uint8_t language = 0,
                                             bool msgSourceKnown = false,
                                             bool languageKnown = false);
PromptCardPlan BuildPromptFamily80022CBCPlan(int32_t promptType);
PromptCardPlan BuildPromptFlash80017E6CPlan(int32_t eventId,
                                             int32_t selected,
                                             int32_t flag);
PromptCardPlan BuildPromptCardRenderPlanForEvent(int32_t eventId);

PromptCardPage PromptCardPageFromEventId(int32_t eventId);
PromptCardPage PromptCardPageFromType(int32_t promptType);
const char* PromptCardPageName(PromptCardPage page);
const char* PromptCardTableKindName(PromptCardTableKind kind);
const char* PromptCardActionKindName(PromptCardActionKind kind);

} // namespace PrSS0PromptCardRenderDirect
