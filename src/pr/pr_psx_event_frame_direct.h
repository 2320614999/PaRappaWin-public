#pragma once

#include "pr_psx_fast_sprite_submit_direct.h"
#include "pr_psx_graph_owner_direct.h"

#include <cstdint>

namespace PrPsxEventFrameDirect {

constexpr uint32_t kFn8001E750_DrawEventFrame = 0x8001E750u;
constexpr uint32_t kFn8001EA00_EndEventFrame = 0x8001EA00u;
constexpr uint32_t kFn8001B120 = 0x8001B120u;
constexpr uint32_t kFn8001B6C4 = 0x8001B6C4u;
constexpr uint32_t kFn8001C550_FastSpriteAt = 0x8001C550u;
constexpr uint32_t kFn8001C5A8_FastSpriteFromPair = 0x8001C5A8u;
constexpr uint32_t kFn8001B590_FastSpriteSubmit = 0x8001B590u;
constexpr uint32_t kFn8001B25C_FastSpriteLocal = 0x8001B25Cu;
constexpr uint32_t kFn8001C604_StageSelectSlice = 0x8001C604u;
constexpr uint32_t kFn8001B654_StageSelectSliceSubmit = 0x8001B654u;
constexpr uint32_t kFn8001B4E0_StageSelectSliceLocal = 0x8001B4E0u;
constexpr uint32_t kFn8001B744_TextGlyphLine = 0x8001B744u;
constexpr uint32_t kFn8001B954_TextGlyphBlock = 0x8001B954u;
constexpr uint32_t kFn800203D4 = 0x800203D4u;
constexpr uint32_t kFn80020568_Event2StageSelectDraw = 0x80020568u;
constexpr uint32_t kFn80027FAC_TextSystemBoot = 0x80027FACu;
constexpr uint32_t kFn80035560_WaitFrame = 0x80035560u;
constexpr uint32_t kFn800356A8_WaitFrameTimeoutHelper = 0x800356A8u;
constexpr uint32_t kFn8003EE84_GsSortBoxFill = 0x8003EE84u;
constexpr uint32_t kFn8003EF5C_LinkPacket = 0x8003EF5Cu;
constexpr uint32_t kFn80043354_SetDumpFnt = 0x80043354u;
constexpr uint32_t kFn80043394_FntLoad = 0x80043394u;
constexpr uint32_t kFn80043438_TextRecordAlloc = 0x80043438u;
constexpr uint32_t kFn800436F0_TextFlush = 0x800436F0u;
constexpr uint32_t kFn80043A14_TextAppend = 0x80043A14u;
constexpr uint32_t kFn8004401C_TextLinkNode = 0x8004401Cu;
constexpr uint32_t kFn800440D0_TextRecordModeControl = 0x800440D0u;
constexpr uint32_t kFn800440B8_TextInitListHead = 0x800440B8u;
constexpr uint32_t kFn800441C0_TextGlyphPacketInit = 0x800441C0u;
constexpr uint32_t kFn80044238_TextRecordHeaderInit = 0x80044238u;
constexpr uint32_t kCallsite800437D0_TextFlushInitListHead = 0x800437D0u;
constexpr uint32_t kCallsite80043914_TextFlushGlyphLink = 0x80043914u;
constexpr uint32_t kCallsite80043994_TextFlushRecordLink = 0x80043994u;
constexpr uint32_t kCallsite800439C8_TextFlushSubmit = 0x800439C8u;
constexpr uint32_t kFn80044E2C_MoveImage = 0x80044E2Cu;
constexpr uint32_t kFn80040CA4_SubmitWorkList = 0x80040CA4u;
constexpr uint32_t kFn800450A0_DrawOtag = 0x800450A0u;
constexpr uint32_t kFn800468E0_DmaDispatch = 0x800468E0u;
constexpr uint32_t kFn80046840_DmaLinkedList = 0x80046840u;
constexpr uint32_t kWorkListBase80087288 = 0x80087288u;
constexpr uint32_t kWorkListStride80087288 = 20u;
constexpr uint32_t kGp368WorkListSlotOffset8001C604 = 0x368u;
constexpr uint32_t kGpuGp1Addr1F801814 = 0x1F801814u;
constexpr uint32_t kDma2MadrAddr1F8010A0 = 0x1F8010A0u;
constexpr uint32_t kDma2BcrAddr1F8010A4 = 0x1F8010A4u;
constexpr uint32_t kDma2ChcrAddr1F8010A8 = 0x1F8010A8u;
constexpr uint32_t kGpuGp1DmaDirectionValue80046840 = 0x04000002u;
constexpr uint32_t kDma2BcrValue80046840 = 0x00000000u;
constexpr uint32_t kDma2ChcrValue80046840 = 0x01000401u;
constexpr uint32_t kDmaDispatchCallbackGlobal8005D828 = 0x8005D828u;
constexpr uint32_t kDmaDispatchArg1Global8005D82C = 0x8005D82Cu;
constexpr uint32_t kDmaDispatchArg3Global8005D830 = 0x8005D830u;
constexpr uint32_t kPsxPacketAddrMask8003EF5C = 0x00FFFFFFu;
constexpr uint32_t kMoveImagePacketAddr8005D7DC = 0x8005D7DCu;
constexpr uint32_t kMoveImageRectAddr8005D7E4 = 0x8005D7E4u;
constexpr uint32_t kMoveImageDestAddr8005D7E8 = 0x8005D7E8u;
constexpr uint32_t kMoveImageSizeAddr8005D7EC = 0x8005D7ECu;
constexpr uint32_t kTextRecordBase8005CB5C = 0x8005CB5Cu;
constexpr uint32_t kTextRecordStride800436F0 = 0x30u;
constexpr uint32_t kTextRecordCount800436F0 = 8u;
constexpr uint32_t kTextRecordBankClearBytes80043394 = 0x180u;
constexpr uint32_t kTextRecordTextBufferBase8008A750 = 0x8008A750u;
constexpr uint32_t kTextRecordGlyphBufferBase8008AB50 = 0x8008AB50u;
constexpr uint32_t kTextGlyphPacketStride800436F0 = 0x10u;
constexpr uint32_t kTextMaxGlyphPackets800436F0 = 128u;
constexpr uint32_t kTextRecordMaxTextCapacity80043438 = 1024u;
constexpr uint32_t kTextHexDigitTablePointerSlot8005D6E8 = 0x8005D6E8u;
constexpr uint32_t kTextHexDigitTableAddr8001229C = 0x8001229Cu;
constexpr uint32_t kTextHexDigitTableByteCount80043A14 = 16u;
constexpr uint32_t kCallsite80043C64_TextHexDigitTableLoad = 0x80043C64u;
constexpr uint32_t kEvent4PromptSpriteCount800203D4 = 3u;
constexpr uint32_t kEvent2StageSelectCtxAddr80087B78 = 0x80087B78u;
constexpr uint32_t kStageSelectEntryCount80020568 = 6u;
constexpr uint32_t kFn80026B94_EventDispatcher = 0x80026B94u;
constexpr uint32_t kFn80026314_MenuHelpTextProducer = 0x80026314u;
constexpr uint32_t kCallsite80026348_MenuHelpLeadingNewlines = 0x80026348u;
constexpr uint32_t kCallsite80026380_MenuHelpGroupMarker = 0x80026380u;
constexpr uint32_t kCallsite80026388_MenuHelpGroupTitlePointerLoad =
    0x80026388u;
constexpr uint32_t kCallsite8002638C_MenuHelpGroupTitleFormatLoad =
    0x8002638Cu;
constexpr uint32_t kCallsite80026394_MenuHelpGroupTitle = 0x80026394u;
constexpr uint32_t kCallsite800263D4_MenuHelpItemLabelPointerLoad =
    0x800263D4u;
constexpr uint32_t kCallsite80026424_MenuHelpItemStackText = 0x80026424u;
constexpr uint32_t kCallsite80026448_MenuHelpGroupNewline = 0x80026448u;
constexpr uint32_t kCallsite80026470_MenuHelpFooter = 0x80026470u;
constexpr uint32_t kCallsite80026478_MenuHelpFlush = 0x80026478u;
constexpr uint32_t kData8006EBF8_MenuHelpGroupTitleFormat = 0x8006EBF8u;
constexpr uint32_t kCallsite80026D8C_StageClearLiteralAppend = 0x80026D8Cu;
constexpr uint32_t kCallsite80026D94_StageClearWordFormatLoad = 0x80026D94u;
constexpr uint32_t kCallsite80026D9C_StageClearStatusBaseLoad = 0x80026D9Cu;
constexpr uint32_t kCallsite80026DA8_StageClearStatusByteLoad = 0x80026DA8u;
constexpr uint32_t kCallsite80026DAC_StageClearWordAppend = 0x80026DACu;
constexpr uint32_t kData8006EC14_StageClearWordFormat = 0x8006EC14u;
constexpr uint32_t kData80092F1D_StageClearStatusBytes = 0x80092F1Du;
constexpr uint32_t kMenuHelpCtxGroupBankOffset80026314 = 0x00u;
constexpr uint32_t kMenuHelpCtxSelectedGroupOffset80026314 = 0x04u;
constexpr uint32_t kMenuHelpCtxGroupCountOffset80026314 = 0x08u;
constexpr uint32_t kMenuHelpGroupStride80026314 = 0x24u;
constexpr uint32_t kMenuHelpGroupItemCountOffset80026314 = 0x00u;
constexpr uint32_t kMenuHelpGroupTitlePointerOffset80026314 = 0x04u;
constexpr uint32_t kMenuHelpGroupItemLabelPointerBaseOffset80026314 = 0x08u;
constexpr uint32_t kMenuHelpGroupSelectedItemOffset80026314 = 0x20u;
constexpr uint32_t kMenuHelpMaxGroups80026314 = 8u;
constexpr uint32_t kMenuHelpMaxItemsPerGroup80026314 = 8u;
constexpr uint32_t kMenuHelpStackTextCapacity80026314 = 64u;
constexpr uint32_t kStageClearStatusWordAppendCount80026DAC = 6u;

struct EventFrameMoveImageBoxFill8001B120 {
    bool valid = false;
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
    uint8_t commandCode = 0;
    int16_t x = 0;
    int16_t y = 0;
    uint16_t w = 0;
    uint16_t h = 0;
};

struct EventFrameState8001E750 {
    PrPsxGraphOwnerDirect::PsxGraphState graph{};
    uint32_t gp368WorkSlot = 0;
    uint32_t gp38CEvent4State = 0;
    bool stage1Event4MoveImageUnderlayValid8001B120 = false;
    bool stage1Event4PrepareUnderlayValid8001B120 = false;
    EventFrameMoveImageBoxFill8001B120 stage1Event4MoveImageBoxFill8001B120{};
    struct FastSpritePageRuntime8003FA20 {
        bool valid = false;
        bool dirty = false;
        uint8_t slot8004019C = 0;
        uint32_t workListAddr = 0;
        PrPsxFastSpriteSubmitDirect::RuntimeState8003FA20 runtime{};
    } fastSpritePageRuntime8003FA20{};
    struct TextFlushRecord800436F0 {
        uint32_t recordAddr = 0;
        uint32_t word0 = 0;
        uint8_t control_03 = 0;
        uint8_t r_04 = 0;
        uint8_t g_05 = 0;
        uint8_t b_06 = 0;
        uint8_t control_07 = 0;
        int16_t x_08 = 0;
        int16_t y_0A = 0;
        int16_t w_0C = 0;
        int16_t h_0E = 0;
        uint32_t listHead_10 = 0;
        int32_t budget_1C = 0;
        uint32_t packetCursor_20 = 0;
        uint32_t textAddr_24 = 0;
        const char* text_24 = nullptr;
        char textStorage_24[kTextRecordMaxTextCapacity80043438 + 1u]{};
        uint32_t word_28 = 0;
        int32_t measureMode_2C = 0;
    };
    struct TextFlushState800436F0 {
        bool bankKnown = false;
        int32_t slotLimit8005CCDC = 0;
        int32_t currentSlot8005CCE0 = 0;
        bool currentSlotActiveKnown = false;
        bool currentSlotActive8005CB80 = false;
        bool textAppendCallback8005D730Known = false;
        uint32_t textAppendCallback8005D730 = 0;
        uint32_t glyphCursorDword8005D6E4 = 0;
        bool fontLoadKnown = false;
        bool fontPageWord8008EB50Known = false;
        int16_t fontPageWord8008EB50 = 0;
        bool fontClutWord8008EB54Known = false;
        int16_t fontClutWord8008EB54 = 0;
        TextFlushRecord800436F0 records[kTextRecordCount800436F0]{};
    } textFlush{};
    struct WaitFrameState80035560 {
        uint32_t requestCount = 0;
        int32_t lastArg = 0;
        int32_t pendingVblanks = 0;
        bool hostVblankHalPending = false;
    } waitFrame80035560{};
    bool initialized = false;
};

struct BoxFillPacketCommand8003EE84 {
    bool valid = false;
    uint32_t packetAddr = 0;
    uint32_t psxCallOrder = 0;
    uint16_t priority = 0;
    bool wordsKnown = false;
    uint32_t word0LinkTag = 0;
    uint32_t word1DrawMode = 0;
    uint32_t word2ColorCode = 0;
    uint32_t word3Xy = 0;
    uint32_t word4Wh = 0;
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
    uint8_t commandCode = 0;
    int16_t x = 0;
    int16_t y = 0;
    uint16_t w = 0;
    uint16_t h = 0;
};

using BoxFillPacketCommandConsumer8003EE84 = void (*)(
    const BoxFillPacketCommand8003EE84& command,
    void* userData);

struct Rect8001B120 {
    int16_t x = 0;
    int16_t y = 0;
    int16_t w = 0;
    int16_t h = 0;
};

struct StageSelectDrawInput80020568 {
    uint32_t ctxAddr = kEvent2StageSelectCtxAddr80087B78;
    int16_t blinkFlag_00 = 0;
    int32_t selected_04 = 0;
    int16_t cursor_08 = 0;
    int16_t count_0A = 0;
    int16_t status_0E[kStageSelectEntryCount80020568]{};
    int16_t bonusStatus_1A = 0;
    int16_t languageIndex_800916D8 = 0;
};

struct StageSelectSpriteTemplate8001B4E0 {
    bool known = false;
    uint32_t addr = 0;
    uint32_t attr_00 = 0;
    uint16_t texX_04 = 0;
    uint16_t texY_06 = 0;
    uint16_t width_08 = 0;
    uint16_t height_0A = 0;
    uint16_t clutX_0C = 0;
    uint16_t clutY_0E = 0;
};

enum class TextFormatterArgKind80043A14 : uint8_t {
    Word = 0,
    String,
};

struct TextFormatterArg80043A14 {
    TextFormatterArgKind80043A14 kind = TextFormatterArgKind80043A14::Word;
    uint32_t word = 0;
    const char* string = nullptr;
    bool psxPointerKnown = false;
    uint32_t psxPointer = 0;
    bool stringPointerMapKnown = false;
    uint32_t sourceCallsite = 0;
    const char* source = nullptr;
};

struct MenuHelpItemInput80026314 {
    uint32_t itemAddr = 0;
    bool labelKnown = false;
    uint32_t labelAddr = 0;
    const char* label = nullptr;
};

struct MenuHelpGroupInput80026314 {
    uint32_t groupAddr = 0;
    bool titleKnown = false;
    uint32_t titleAddr = 0;
    const char* title = nullptr;
    uint32_t itemCount = 0;
    uint32_t selectedItemIndex = 0;
    const MenuHelpItemInput80026314* items = nullptr;
};

struct MenuHelpTextInput80026314 {
    uint32_t ctxAddr = 0;
    uint32_t groupBankAddr = 0;
    uint32_t selectedGroupIndex = 0;
    uint32_t groupCount = 0;
    const MenuHelpGroupInput80026314* groups = nullptr;
};

struct MenuHelpItemStackText80026424 {
    bool called = false;
    uint32_t callsite = kCallsite80026424_MenuHelpItemStackText;
    uint32_t groupIndex = 0;
    uint32_t itemIndex = 0;
    uint32_t groupAddr = 0;
    bool itemPointerSlotAddrKnown = false;
    uint32_t itemPointerSlotAddr = 0;
    uint32_t itemPointerLoadCallsite =
        kCallsite800263D4_MenuHelpItemLabelPointerLoad;
    uint32_t itemPointerSourceOffset =
        kMenuHelpGroupItemLabelPointerBaseOffset80026314;
    bool selected = false;
    const char* marker = nullptr;
    bool labelKnown = false;
    uint32_t labelAddr = 0;
    const char* label = nullptr;
    bool labelPointerMapGap = true;
    const char* labelPointerMapSource = nullptr;
    bool stackBufferKnown = false;
    bool stackBufferProducerGap = true;
    bool stackBufferTruncatedGap = false;
    char stackText[kMenuHelpStackTextCapacity80026314]{};
};

struct MenuHelpGroupAppend80026314 {
    bool called = false;
    uint32_t groupIndex = 0;
    uint32_t groupAddr = 0;
    bool selected = false;
    uint32_t markerCallsite = kCallsite80026380_MenuHelpGroupMarker;
    const char* markerLiteral = nullptr;
    uint32_t titleCallsite = kCallsite80026394_MenuHelpGroupTitle;
    uint32_t titlePointerLoadCallsite =
        kCallsite80026388_MenuHelpGroupTitlePointerLoad;
    uint32_t titleFormatLoadCallsite =
        kCallsite8002638C_MenuHelpGroupTitleFormatLoad;
    uint32_t titleFormatAddr = kData8006EBF8_MenuHelpGroupTitleFormat;
    uint32_t groupStride = kMenuHelpGroupStride80026314;
    uint32_t groupTitlePointerOffset =
        kMenuHelpGroupTitlePointerOffset80026314;
    bool titlePointerSlotAddrKnown = false;
    uint32_t titlePointerSlotAddr = 0;
    bool titleKnown = false;
    uint32_t titleAddr = 0;
    bool titlePointerMapGap = false;
    uint32_t titlePointerMapCallsite = kCallsite80026394_MenuHelpGroupTitle;
    uint32_t titlePointerMapAddr = 0;
    const char* titlePointerMapSource = nullptr;
    TextFormatterArg80043A14 titleArg{};
    uint32_t itemCount = 0;
    uint32_t selectedItemIndex = 0;
    uint32_t itemAppendCount = 0;
    bool itemOutputTruncatedGap = false;
    bool itemSourceGap = false;
    MenuHelpItemStackText80026424
        itemAppends[kMenuHelpMaxItemsPerGroup80026314]{};
    uint32_t newlineCallsite = kCallsite80026448_MenuHelpGroupNewline;
};

struct StageClearTextInput80026B94 {
    int32_t eventId = 0;
    bool word800916F6Known = false;
    uint16_t word_800916F6 = 0;
    const char* format8006EC14 = nullptr;
    bool statusBytesKnown80092F1D = false;
    uint8_t byte_80092F1D[kStageClearStatusWordAppendCount80026DAC]{};
};

void ResetEventFrameState8003FB9C(EventFrameState8001E750& state,
                                  uint16_t width,
                                  uint16_t height);

void ResetStage1Event4Gp38CSeed8006EDCC();
void SeedStage1Event4Gp38C8006EDCC(uint32_t value);
bool ConsumeStage1Event4Gp38CSeed8006EDCC(uint32_t& outValue);

void PsxCall80027FAC_TextSystemBoot(EventFrameState8001E750& state);

void PsxCall80043394_FntLoad(EventFrameState8001E750& state,
                             int16_t x,
                             int16_t y);

void PsxCall80043354_SetDumpFnt(EventFrameState8001E750& state,
                                int32_t slot);

void PsxCall80043438_TextRecordAlloc(EventFrameState8001E750& state,
                                     int16_t x,
                                     int16_t y,
                                     int16_t w,
                                     int16_t h,
                                     int32_t mode,
                                     int32_t capacity);

void PsxCall80043A14_TextAppend(EventFrameState8001E750& state,
                                int32_t arg0,
                                const char* text);

void PsxCall80043A14_TextAppend(EventFrameState8001E750& state,
                                int32_t arg0,
                                const char* text,
                                const TextFormatterArg80043A14* args,
                                uint32_t argCount);

void PsxCall80043A14_TextAppendSlotFmt(EventFrameState8001E750& state,
                                       int32_t slot,
                                       const char* fmt,
                                       const TextFormatterArg80043A14* args,
                                       uint32_t argCount);

void PsxCall80043A14_TextAppendCurrentFmt(EventFrameState8001E750& state,
                                          const char* fmt,
                                          const TextFormatterArg80043A14* args,
                                          uint32_t argCount);

MenuHelpItemStackText80026424 PsxBuild80026314_MenuHelpItemStackText(
    uint32_t groupIndex,
    uint32_t itemIndex,
    uint32_t groupAddr,
    bool selected,
    const MenuHelpItemInput80026314& input);

void PsxProduce80026314_MenuHelpText(
    EventFrameState8001E750& state,
    const MenuHelpTextInput80026314& input);

void PsxProduce80026B94_Event2StageClearText(
    EventFrameState8001E750& state,
    const StageClearTextInput80026B94& input);

void PsxCall8001E750_Event4(
    EventFrameState8001E750& state,
    int32_t promptCtx0);

bool PsxCall8001E750_SaveUiEventFrame(
    EventFrameState8001E750& state,
    int32_t eventId,
    int32_t contextWord0,
    int32_t contextWord1,
    int32_t contextWord2,
    int32_t languageIndex);

bool PsxCall8001E750_Event2StageSelectFrameCloseBlocked(
    EventFrameState8001E750& state,
    const StageSelectDrawInput80020568* input);

void PsxCall80035560_WaitFrame(EventFrameState8001E750& state,
                               int32_t arg0);
void PsxConsume80035560_WaitFrameHostVblank(
    EventFrameState8001E750& state,
    int32_t consumedVblanks);

void PsxCall80040CA4_SubmitWorkList(
    uint32_t workListAddr,
    uint32_t workListSlot,
    const PrPsxGraphOwnerDirect::PsxGraphWorkList80040CC8& work);

void PsxCall8001EA00_EndFrame(EventFrameState8001E750& state,
                              int32_t eventId);

void PsxCall800436F0_TextFlush(EventFrameState8001E750& state,
                               int32_t arg0);

void PsxCall80026B94_EventFrameTail(
    EventFrameState8001E750& state,
    int32_t eventId,
    int32_t event4PromptCtx0,
    const StageSelectDrawInput80020568* event2StageSelectInput,
    const StageClearTextInput80026B94* event2StageClearInput);

void BuildEventFrameBoxFillPacketCommands8003EE84(
    const EventFrameState8001E750& state,
    BoxFillPacketCommandConsumer8003EE84 consumeCommand,
    void* commandUserData);

}  // namespace PrPsxEventFrameDirect
