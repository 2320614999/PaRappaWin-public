#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace PrSS0EventTextDirect {

static constexpr uint32_t kFn80027FAC = 0x80027FACu;
static constexpr uint32_t kFn80043354 = 0x80043354u;
static constexpr uint32_t kFn80043394 = 0x80043394u;
static constexpr uint32_t kFn80043438 = 0x80043438u;
static constexpr uint32_t kFn800436F0 = 0x800436F0u;
static constexpr uint32_t kFn80043A14 = 0x80043A14u;
static constexpr uint32_t kFn8004401C = 0x8004401Cu;
static constexpr uint32_t kFn800440B8 = 0x800440B8u;
static constexpr uint32_t kFn800440D0 = 0x800440D0u;
static constexpr uint32_t kFn800441C0 = 0x800441C0u;
static constexpr uint32_t kFn80044238 = 0x80044238u;
static constexpr uint32_t kFn80044AA0 = 0x80044AA0u;
static constexpr uint32_t kFn80044BA8 = 0x80044BA8u;
static constexpr uint32_t kFn80044D64 = 0x80044D64u;
static constexpr uint32_t kFn800462C4 = 0x800462C4u;
static constexpr uint32_t kFn80047144 = 0x80047144u;
static constexpr uint32_t kFn80047178 = 0x80047178u;
static constexpr uint32_t kFn80035560 = 0x80035560u;
static constexpr uint32_t kFn800450A0 = 0x800450A0u;
static constexpr uint32_t kFn800468E0 = 0x800468E0u;
static constexpr uint32_t kFn80046BC4 = 0x80046BC4u;
static constexpr uint32_t kFn80046840 = 0x80046840u;
static constexpr uint32_t kFn80026314 = 0x80026314u;
static constexpr uint32_t kFn80026B94 = 0x80026B94u;

static constexpr uint32_t kTextRecordBase8005CB5C = 0x8005CB5Cu;
static constexpr uint32_t kTextRecordStride800436F0 = 0x30u;
static constexpr uint32_t kTextRecordCount800436F0 = 8u;
static constexpr uint32_t kTextRecordBankClearBytes80043394 = 0x180u;
static constexpr uint32_t kTextRecordTextBufferBase8008A750 = 0x8008A750u;
static constexpr uint32_t kTextRecordGlyphBufferBase8008AB50 = 0x8008AB50u;
static constexpr uint32_t kTextGlyphPacketStride800436F0 = 0x10u;
static constexpr uint32_t kTextRecordMaxCapacity80043438 = 1024u;
static constexpr uint32_t kTextBufferBankByteCount80043394 = 1024u;
static constexpr uint32_t kTextGlyphPacketBankCount80043394 = 1024u;
static constexpr uint32_t kTextRecordCountGlobal8005CCDC = 0x8005CCDCu;
static constexpr uint32_t kTextCurrentRecordGlobal8005CCE0 = 0x8005CCE0u;
static constexpr uint32_t kTextAppendCallbackGlobal8005D730 = 0x8005D730u;
static constexpr uint32_t kTextGlyphCursorGlobal8005D6E4 = 0x8005D6E4u;
static constexpr uint32_t kTextHexDigitTablePointerSlot8005D6E8 = 0x8005D6E8u;
static constexpr uint32_t kTextHexDigitTableAddr8001229C = 0x8001229Cu;
static constexpr uint32_t kTextHexDigitTableByteCount80043A14 = 16u;
static constexpr uint32_t kTextFontPageGlobal8008EB50 = 0x8008EB50u;
static constexpr uint32_t kTextFontClutGlobal8008EB54 = 0x8008EB54u;
static constexpr uint32_t kEmbeddedFontSourceAddress8005CCE4 = 0x8005CCE4u;
static constexpr uint32_t kEmbeddedFontImageAddress8005CEE4 = 0x8005CEE4u;
static constexpr uint32_t kEmbeddedFontFilePayloadOffset = 0x800u;
static constexpr uint32_t kEmbeddedFontTextLoadAddress = 0x80010000u;
static constexpr uint32_t kEmbeddedFontClutByteCount = 0x20u;
static constexpr uint32_t kEmbeddedFontImageByteCount = 128u * 32u / 2u;
static constexpr uint32_t kEmbeddedFontWidth = 128u;
static constexpr uint32_t kEmbeddedFontHeight = 32u;
static constexpr uint32_t kEventTextDispatchQueueCapacity800468E0 = 64u;
static constexpr uint32_t kEventTextDispatchQueueStride800468E0 = 0x60u;
static constexpr uint32_t kEventTextDispatchQueueCallbackBase800468E0 =
    0x80094448u;
static constexpr uint32_t kEventTextDispatchQueuePayloadBase800468E0 =
    0x80094454u;
static constexpr uint32_t kEventTextDispatchQueuePayloadByteLimit800468E0 =
    kEventTextDispatchQueueStride800468E0 - 0x0Cu;
static constexpr uint32_t kEventTextDispatchDmaBusyMask80046BC4 =
    0x01000000u;
static constexpr uint32_t kEventTextDispatchGpuReadyMask80046BC4 =
    0x00400000u;
static constexpr uint32_t kEventTextDispatchDmaChannel800468E0 = 2u;
static constexpr uint32_t kEventTextGpuWaitDeadlineOffset80047144 = 0xF0u;
static constexpr uint32_t kEventTextGpuWaitPollLimit80047178 = 0xF00000u;

static constexpr uint32_t kCallsite80027FB8_FntLoad = 0x80027FB8u;
static constexpr uint32_t kCallsite80027FD8_TextRecordAlloc = 0x80027FD8u;
static constexpr uint32_t kCallsite80027FE0_SetDumpFnt = 0x80027FE0u;
static constexpr uint32_t kCallsite80027FE8_TextEnable = 0x80027FE8u;
static constexpr uint32_t kCallsite80026348_MenuHelpLeading = 0x80026348u;
static constexpr uint32_t kCallsite80026380_MenuHelpGroupMarker = 0x80026380u;
static constexpr uint32_t kCallsite80026388_MenuHelpTitlePointer = 0x80026388u;
static constexpr uint32_t kCallsite8002638C_MenuHelpTitleFormat = 0x8002638Cu;
static constexpr uint32_t kCallsite80026394_MenuHelpTitleAppend = 0x80026394u;
static constexpr uint32_t kCallsite800263D4_MenuHelpItemPointer = 0x800263D4u;
static constexpr uint32_t kCallsite80026424_MenuHelpItemAppend = 0x80026424u;
static constexpr uint32_t kCallsite80026448_MenuHelpGroupNewline = 0x80026448u;
static constexpr uint32_t kCallsite80026470_MenuHelpFooter = 0x80026470u;
static constexpr uint32_t kCallsite80026478_MenuHelpFlush = 0x80026478u;
static constexpr uint32_t kCallsite80026D70_StageClearGate = 0x80026D70u;
static constexpr uint32_t kCallsite80026D8C_StageClearLiteral = 0x80026D8Cu;
static constexpr uint32_t kCallsite80026D94_StageClearFormatLoad = 0x80026D94u;
static constexpr uint32_t kCallsite80026DA8_StageClearStatusLoad = 0x80026DA8u;
static constexpr uint32_t kCallsite80026DAC_StageClearStatusAppend = 0x80026DACu;
static constexpr uint32_t kCallsite80026DD4_StageClearTailFlush = 0x80026DD4u;
static constexpr uint32_t kCallsite800437D0_TextFlushInitList = 0x800437D0u;
static constexpr uint32_t kCallsite80043914_TextFlushGlyphLink = 0x80043914u;
static constexpr uint32_t kCallsite80043994_TextFlushRecordLink = 0x80043994u;
static constexpr uint32_t kCallsite800439C8_TextFlushSubmit = 0x800439C8u;
static constexpr uint32_t kCallsite80043C64_TextHexTableLoad = 0x80043C64u;

static constexpr uint32_t kData8006EBF0_MenuHelpLeadingLiteral = 0x8006EBF0u;
static constexpr uint32_t kData8006EBF8_MenuHelpTitleFormat = 0x8006EBF8u;
static constexpr uint32_t kData8006EC14_StageClearFormat = 0x8006EC14u;
static constexpr uint32_t kGlobal800916F6_StageClearGate = 0x800916F6u;
static constexpr uint32_t kData80092F1D_StageClearStatusBytes = 0x80092F1Du;
static constexpr uint32_t kMenuHelpCtxGroupBankOffset80026314 = 0x00u;
static constexpr uint32_t kMenuHelpCtxSelectedGroupOffset80026314 = 0x04u;
static constexpr uint32_t kMenuHelpCtxGroupCountOffset80026314 = 0x08u;
static constexpr uint32_t kMenuHelpGroupStride80026314 = 0x24u;
static constexpr uint32_t kMenuHelpGroupItemCountOffset80026314 = 0x00u;
static constexpr uint32_t kMenuHelpGroupTitlePointerOffset80026314 = 0x04u;
static constexpr uint32_t kMenuHelpGroupItemLabelPointerBaseOffset80026314 = 0x08u;
static constexpr uint32_t kMenuHelpGroupSelectedItemOffset80026314 = 0x20u;
static constexpr uint32_t kMenuHelpMaxGroups80026314 = 8u;
static constexpr uint32_t kMenuHelpMaxItemsPerGroup80026314 = 8u;
static constexpr uint32_t kMenuHelpStackTextCapacity80026314 = 64u;
static constexpr uint32_t kStageClearStatusAppendCount80026DAC = 6u;

enum class EventTextEndpointKind : uint8_t {
    Unknown = 0,
    TextSystemBoot,
    FontRecord,
    AppendFormat,
    Flush,
    Producer,
    Helper,
};

enum class EventTextProducerKind : uint8_t {
    Unknown = 0,
    TextSystemBoot80027FAC,
    TextRecordBank,
    TextAppendFormatter80043A14,
    MenuHelp80026314,
    StageClear80026B94,
    TextFlush800436F0,
};

enum class EventTextActionKind : uint8_t {
    None = 0,
    CallFntLoad80043394,
    ClearRecordBank80043394,
    AllocRecord80043438,
    InitGlyphPackets800441C0,
    SelectDumpRecord80043354,
    RegisterAppendCallback80043354,
    EnableTextDisplay80044AA0,
    GateAppendRecord80043A14,
    AppendLiteral80043A14,
    AppendFormat80043A14,
    LoadHexDigitTable80043A14,
    GateMenuHelpContext80026314,
    BuildMenuItemStackBuffer,
    AppendMenuItemStack80043A14,
    StageClearGate,
    GateStageClearTextSource80026B94,
    AppendStageClearStatus80043A14,
    FlushText800436F0,
    InitListHead800440B8,
    ParseTextByteStream,
    LinkGlyphPacket8004401C,
    LinkRecord8004401C,
    SubmitTextOtag800450A0,
    ClearTextAfterFlush,
    Gap,
};

struct EventTextEndpointSpec {
    uint32_t function = 0;
    EventTextEndpointKind kind = EventTextEndpointKind::Unknown;
    const char* name = nullptr;
    const char* callers = nullptr;
    const char* params = nullptr;
    const char* effects = nullptr;
};

struct EventTextRawSpec {
    uint32_t address = 0;
    const char* name = nullptr;
    const char* shape = nullptr;
    const char* owner = nullptr;
    const char* use = nullptr;
};

struct EventTextCallsiteSpec {
    uint32_t callsite = 0;
    uint32_t callee = 0;
    EventTextProducerKind producer = EventTextProducerKind::Unknown;
    EventTextActionKind action = EventTextActionKind::None;
    uint32_t dataAddress = 0;
    uint32_t auxAddress = 0;
    uint32_t repeatCount = 0;
    const char* effectiveArgs = nullptr;
    const char* rule = nullptr;
};

struct EventTextAction {
    EventTextActionKind kind = EventTextActionKind::None;
    EventTextProducerKind producer = EventTextProducerKind::Unknown;
    uint32_t psxFunction = 0;
    uint32_t callsite = 0;
    uint32_t dataAddress = 0;
    uint32_t auxAddress = 0;
    int32_t args[6]{};
    uint32_t repeatCount = 0;
    const char* literal = nullptr;
    bool conditional = false;
};

struct EventTextPlan {
    const char* name = nullptr;
    bool runtimeCutoverAllowed = false;
    bool blockedByGap = true;
    EventTextProducerKind producer = EventTextProducerKind::Unknown;
    EventTextAction actions[96]{};
    uint32_t count = 0;
    bool truncated = false;
};

struct EventTextGlyphPacket800436F0 {
    uint32_t word0 = 0;
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
    uint8_t code = 0;
    int16_t x = 0;
    int16_t y = 0;
    uint8_t u = 0;
    uint8_t v = 0;
    uint16_t clut = 0;
};

static_assert(sizeof(EventTextGlyphPacket800436F0) == 0x10u);
static_assert(offsetof(EventTextGlyphPacket800436F0, r) == 0x04u);
static_assert(offsetof(EventTextGlyphPacket800436F0, x) == 0x08u);
static_assert(offsetof(EventTextGlyphPacket800436F0, u) == 0x0Cu);

struct EventTextRecord800436F0 {
    uint32_t word0 = 0;
    uint8_t byte04 = 0;
    uint8_t byte05 = 0;
    uint8_t byte06 = 0;
    uint8_t control07 = 0;
    int16_t x = 0;
    int16_t y = 0;
    int16_t w = 0;
    int16_t h = 0;
    uint32_t listHeadWord10 = 0;
    uint32_t word14 = 0;
    uint32_t word18 = 0;
    int32_t parseBudget1C = 0;
    uint32_t glyphPacketCursor20 = 0;
    uint32_t textPointer24 = 0;
    uint32_t appendCursor28 = 0;
    int32_t measureMode2C = 0;
};

static_assert(sizeof(EventTextRecord800436F0) == 0x30u);
static_assert(offsetof(EventTextRecord800436F0, control07) == 0x07u);
static_assert(offsetof(EventTextRecord800436F0, x) == 0x08u);
static_assert(offsetof(EventTextRecord800436F0, listHeadWord10) == 0x10u);
static_assert(offsetof(EventTextRecord800436F0, parseBudget1C) == 0x1Cu);
static_assert(offsetof(EventTextRecord800436F0, glyphPacketCursor20) == 0x20u);
static_assert(offsetof(EventTextRecord800436F0, textPointer24) == 0x24u);
static_assert(offsetof(EventTextRecord800436F0, appendCursor28) == 0x28u);
static_assert(offsetof(EventTextRecord800436F0, measureMode2C) == 0x2Cu);

struct EventTextRuntimeState800436F0 {
    bool recordBankKnown = false;
    bool globalsKnown = false;
    bool recordCountKnown = false;
    bool currentRecordKnown = false;
    bool allocationCursorKnown = false;
    bool appendCallbackKnown = false;
    bool textBankKnown = false;
    bool glyphBankKnown = false;
    bool fontBindingKnown = false;
    bool loadImageTransactionKnown = false;
    int32_t recordCount8005CCDC = 0;
    int32_t currentRecord8005CCE0 = 0;
    int32_t allocationCursor8005D6E4 = 0;
    uint32_t appendCallback8005D730 = 0;
    uint16_t fontTPage8008EB50 = 0;
    uint16_t fontClut8008EB54 = 0;
    uint8_t gpuType8005D734 = 0;
    std::array<EventTextRecord800436F0, kTextRecordCount800436F0> records{};
    std::array<uint8_t, kTextBufferBankByteCount80043394> textBytes{};
    std::array<EventTextGlyphPacket800436F0,
               kTextGlyphPacketBankCount80043394>
        glyphPackets{};
};

struct EventTextPsxRect80044D64 {
    int16_t x = 0;
    int16_t y = 0;
    int16_t w = 0;
    int16_t h = 0;
};

static_assert(sizeof(EventTextPsxRect80044D64) == 8u);

// Host-bound input for the formal 80044D64 LoadImage wrapper. The source
// address must be explicit; this model never dereferences a guessed PSX
// pointer and never claims that Windows executed the PSX DMA/HAL.
struct EventTextLoadImageRequest80044D64 {
    EventTextPsxRect80044D64 rect{};
    const uint8_t* pixelBytes = nullptr;
    size_t pixelByteCount = 0u;
    uint32_t pixelAddress = 0u;
    bool pixelAddressKnown = false;
};

enum class EventTextLoadImageStatus80044D64 : uint8_t {
    NotRun = 0,
    RectBindingLimit,
    PixelBindingLimit,
    DispatchBindingLimit,
    Executed,
};

struct EventTextLoadImageResult80044D64 {
    EventTextLoadImageStatus80044D64 status =
        EventTextLoadImageStatus80044D64::NotRun;
    bool accepted = false;
    bool rectValidationWarning = false;
    bool exactPsxHalParity = false;
    uint32_t wrapperFunction = kFn80044D64;
    uint32_t rectLoggerFunction = kFn80044BA8;
    uint32_t lowerDispatchFunction = kFn800468E0;
    uint32_t lowerUploadCallback = kFn800462C4;
    uint32_t dispatchWidth = 0u;
    EventTextPsxRect80044D64 rect{};
    uint32_t pixelAddress = 0u;
    uint32_t pixelByteCount = 0u;
};

struct EventTextDispatchRequest800468E0 {
    uint32_t callbackFunction = 0u;
    uint32_t headAddress = 0u;
    bool headAddressKnown = false;
    const uint8_t* headBytes = nullptr;
    size_t headByteCount = 0u;
    uint32_t copyByteCount = 0u;
    uint32_t callbackArgument = 0u;
    bool callbackArgumentKnown = false;
};

enum class EventTextDispatchStatus800468E0 : uint8_t {
    NotRun = 0,
    CallbackBindingLimit,
    QueuePayloadBindingLimit,
    QueuePayloadSizeLimit,
    Executed,
};

struct EventTextDispatchResult800468E0 {
    EventTextDispatchStatus800468E0 status =
        EventTextDispatchStatus800468E0::NotRun;
    bool accepted = false;
    bool exactPsxHalParity = false;
    bool queueShapeKnown = false;
    bool immediateCallbackArgumentsKnown = false;
    uint32_t dispatcherFunction = kFn800468E0;
    uint32_t callbackFunction = 0u;
    uint32_t headAddress = 0u;
    uint32_t callbackArgument = 0u;
    uint32_t copyByteCount = 0u;
    uint32_t copyWordCount = 0u;
    uint32_t queueCapacity = kEventTextDispatchQueueCapacity800468E0;
    uint32_t queueStride = kEventTextDispatchQueueStride800468E0;
    uint32_t queueCallbackBase = kEventTextDispatchQueueCallbackBase800468E0;
    uint32_t queuePayloadBase = kEventTextDispatchQueuePayloadBase800468E0;
    uint32_t queuePayloadByteLimit =
        kEventTextDispatchQueuePayloadByteLimit800468E0;
    std::array<uint8_t, kEventTextDispatchQueuePayloadByteLimit800468E0>
        copiedHeadBytes{};
};

// Explicit host representation of the formal 800468E0/80046BC4 ring. The
// producer and consumer indices mirror dword_8005D838 and dword_8005D83C.
// Payload storage is local and bounded; it never dereferences a guessed PSX
// address. DMA/GPU readiness is supplied by the caller, not inferred from a
// Windows clock or renderer.
struct EventTextDispatchQueueEntry800468E0 {
    bool valid = false;
    uint32_t callbackFunction = 0u;
    uint32_t payloadAddress = 0u;
    uint32_t callbackArgument = 0u;
    uint32_t copyByteCount = 0u;
    std::array<uint8_t, kEventTextDispatchQueuePayloadByteLimit800468E0>
        payload{};
};

struct EventTextDispatchCallbackInvocation80046BC4 {
    uint32_t callbackFunction = 0u;
    uint32_t headAddress = 0u;
    uint32_t callbackArgument = 0u;
    uint32_t payloadAddress = 0u;
    uint32_t payloadByteCount = 0u;
    const uint8_t* payloadBytes = nullptr;
};

using EventTextDispatchCallbackSink80046BC4 =
    bool (*)(const EventTextDispatchCallbackInvocation80046BC4& invocation,
             void* userData);

struct EventTextDispatchQueueState800468E0 {
    uint32_t producerIndex8005D838 = 0u;
    uint32_t consumerIndex8005D83C = 0u;
    bool dmaActive = false;
    bool gpuReady = true;
    bool dmaCallbackRegistered = false;
    bool completionPending = false;
    uint32_t completionCallbackFunction = 0u;
    uint32_t completionCallbackArgument = 0u;
    std::array<EventTextDispatchQueueEntry800468E0,
               kEventTextDispatchQueueCapacity800468E0>
        entries{};
};

enum class EventTextDispatchQueueStatus80046BC4 : uint8_t {
    NotRun = 0,
    CallbackBindingLimit,
    PayloadBindingLimit,
    PayloadSizeLimit,
    QueueFull,
    DmaBusy,
    GpuNotReady,
    CallbackSinkBindingLimit,
    Executed,
};

struct EventTextDispatchQueueResult80046BC4 {
    EventTextDispatchQueueStatus80046BC4 status =
        EventTextDispatchQueueStatus80046BC4::NotRun;
    bool accepted = false;
    bool exactPsxHalParity = false;
    bool queueShapeKnown = true;
    bool immediatePath = false;
    bool dmaCallbackRequested = false;
    bool callbackInvoked = false;
    bool callbackFailed = false;
    bool completionCallbackInvoked = false;
    uint32_t dispatcherFunction = kFn800468E0;
    uint32_t consumerFunction = kFn80046BC4;
    uint32_t producerBefore = 0u;
    uint32_t consumerBefore = 0u;
    uint32_t producerAfter = 0u;
    uint32_t consumerAfter = 0u;
    uint32_t processedCount = 0u;
    uint32_t remainingCount = 0u;
    uint32_t callbackFunction = 0u;
    uint32_t headAddress = 0u;
    uint32_t callbackArgument = 0u;
    uint32_t payloadAddress = 0u;
    uint32_t payloadByteCount = 0u;
};

struct EventTextDispatchQueueRequest800468E0 {
    EventTextDispatchRequest800468E0 dispatch{};
    bool forceQueue = false;
    EventTextDispatchCallbackSink80046BC4 callbackSink = nullptr;
    void* callbackSinkUserData = nullptr;
};

struct EventTextGpuWaitState80047144 {
    bool prepared = false;
    uint32_t deadlineTick = 0u;
    uint32_t pollCount = 0u;
};

enum class EventTextGpuWaitStatus80047178 : uint8_t {
    NotRun = 0,
    PreparationBindingLimit,
    ClockBindingLimit,
    PollCompleted,
    TimedOut,
};

struct EventTextGpuWaitResult80047178 {
    EventTextGpuWaitStatus80047178 status =
        EventTextGpuWaitStatus80047178::NotRun;
    bool accepted = false;
    bool exactPsxHalParity = false;
    bool deadlineOverdue = false;
    bool resetIssued = false;
    uint32_t prepareFunction = kFn80047144;
    uint32_t waitFunction = kFn80047178;
    uint32_t clockFunction = kFn80035560;
    uint32_t deadlineOffset = kEventTextGpuWaitDeadlineOffset80047144;
    uint32_t pollLimit = kEventTextGpuWaitPollLimit80047178;
    uint32_t currentTick = 0u;
    uint32_t deadlineTick = 0u;
    uint32_t pollCount = 0u;
    uint32_t resetDma2Chcr = 0x401u;
    uint32_t resetDmaControlMask = 0x800u;
    uint32_t resetGpuStatusFirst = 0x20000000u;
    uint32_t resetGpuStatusSecond = 0x10000000u;
};

// Lower upload callback reached by 80044D64 -> 800468E0. This records the
// formal transfer split without pretending that the Windows renderer executed
// the PSX GPU/DMA registers.
enum class EventTextUploadStatus800462C4 : uint8_t {
    NotRun = 0,
    RectBindingLimit,
    PixelBindingLimit,
    TransferSizeLimit,
    Executed,
};

struct EventTextUploadResult800462C4 {
    EventTextUploadStatus800462C4 status =
        EventTextUploadStatus800462C4::NotRun;
    bool accepted = false;
    bool exactPsxHalParity = false;
    bool rectClamped = false;
    bool waitCompleted = false;
    bool cpuTailWritten = false;
    bool dmaSubmitted = false;
    uint32_t callbackFunction = kFn800462C4;
    uint32_t prepareFunction = kFn80047144;
    uint32_t waitFunction = kFn80047178;
    EventTextPsxRect80044D64 rect{};
    EventTextPsxRect80044D64 effectiveRect{};
    uint32_t transferWords = 0u;
    uint32_t cpuTailWords = 0u;
    uint32_t dmaBlockCount = 0u;
    uint32_t gp0Command = 0u;
    uint32_t gp1DmaDirection = 0u;
    uint32_t dma2Madr = 0u;
    uint32_t dma2Bcr = 0u;
    uint32_t dma2Chcr = 0u;
};

struct EventTextFontBinding80043394 {
    bool known = false;
    uint16_t tpage = 0;
    uint16_t clut = 0;
    uint8_t gpuType8005D734 = 0;
};

struct EventTextEmbeddedFont8005CCE4 {
    bool sourceKnown = false;
    uint32_t sourceAddress = kEmbeddedFontSourceAddress8005CCE4;
    uint32_t imageAddress = kEmbeddedFontImageAddress8005CEE4;
    uint32_t fileOffset = 0u;
    std::array<uint8_t, kEmbeddedFontClutByteCount> clutBytes{};
    std::array<uint8_t, kEmbeddedFontImageByteCount> imageBytes{};
    std::array<uint16_t, 16> clut{};
    std::array<uint32_t, kEmbeddedFontWidth * kEmbeddedFontHeight> rgba{};
};

// Formal software equivalent of the two bindings produced by FntLoad:
// 8004308C -> 80043DF4 (GetTPage), 800431E0 -> 80043EBC (GetClut).
// The image/CLUT upload through 80044D64 remains a separate host HAL boundary.
bool ComputeFntLoadFontBinding80043394(
    int32_t x,
    int32_t y,
    int32_t imageMode,
    uint8_t gpuType8005D734,
    EventTextFontBinding80043394& out);

bool DecodeEmbeddedFont8005CCE4(
    const uint8_t* originalExe,
    size_t originalExeSize,
    size_t fileOffset,
    EventTextEmbeddedFont8005CCE4& out);

enum class EventTextFntLoadStatus80043394 : uint8_t {
    NotRun = 0,
    FontBindingUnknown,
    Executed,
};

struct EventTextFntLoadResult80043394 {
    EventTextFntLoadStatus80043394 status =
        EventTextFntLoadStatus80043394::NotRun;
    bool accepted = false;
    bool fontUploadExecuted = false;
    uint32_t returnedRecordBankAddress = 0;
};

struct EventTextFntOpenRequest80043438 {
    int32_t x = 0;
    int32_t y = 0;
    int32_t width = 0;
    int32_t height = 0;
    int32_t mode = 0;
    int32_t requestedCapacity = 0;
};

enum class EventTextFntOpenStatus80043438 : uint8_t {
    NotRun = 0,
    SourceUnknown,
    RecordLimit,
    HostUnsupportedNonzeroMode,
    HostPoolBindingLimit,
    Executed,
};

struct EventTextFntOpenResult80043438 {
    EventTextFntOpenStatus80043438 status =
        EventTextFntOpenStatus80043438::NotRun;
    bool accepted = false;
    int32_t returnedSlot = -1;
    int32_t oldCursor = 0;
    int32_t effectiveCapacity = 0;
    int32_t newCursor = 0;
    uint32_t initializedPacketCount = 0;
    uint32_t recordAddress = 0;
    uint32_t textAddress = 0;
    uint32_t packetAddress = 0;
};

struct EventTextSetDumpFntResult80043354 {
    bool sourceKnown = false;
    bool accepted = false;
    bool updated = false;
    int32_t requestedSlot = 0;
};

enum class EventTextAppendArgKind80043A14 : uint8_t {
    Signed32 = 0,
    Unsigned32,
    Character,
    String,
};

struct EventTextAppendArg80043A14 {
    EventTextAppendArgKind80043A14 kind =
        EventTextAppendArgKind80043A14::Signed32;
    int32_t signedValue = 0;
    uint32_t unsignedValue = 0;
    uint8_t characterValue = 0;
    const char* stringValue = nullptr;
};

static constexpr uint32_t kTextAppendMaxArgs80043A14 = 16u;

// Host-side representation of the original O32 vararg list. The formatter
// accepts only explicitly bound host values; it never dereferences a guessed
// PSX pointer or borrows state from the old S0 shell.
struct EventTextAppendRequest80043A14 {
    int32_t slot = -1;
    const char* format = nullptr;
    std::array<EventTextAppendArg80043A14,
               kTextAppendMaxArgs80043A14>
        args{};
    uint32_t argCount = 0;
};

enum class EventTextAppendStatus80043A14 : uint8_t {
    NotRun = 0,
    SourceUnknown,
    CurrentTextPointerNull,
    FormatBindingLimit,
    MissingArgument,
    StringBindingLimit,
    UnsupportedSpecifier,
    CapacityBindingLimit,
    Executed,
};

struct EventTextAppendResult80043A14 {
    EventTextAppendStatus80043A14 status =
        EventTextAppendStatus80043A14::NotRun;
    bool accepted = false;
    bool usedFallbackSlot = false;
    uint32_t selectedSlot = 0;
    uint32_t argumentsConsumed = 0;
    uint32_t formatBytesConsumed = 0;
    uint32_t appendedBytes = 0;
    uint32_t initialCursor = 0;
    uint32_t finalCursor = 0;
    int32_t returnedCursor = -1;
};

enum class EventTextBootStep80027FAC : uint8_t {
    None = 0,
    FntLoad80043394,
    FntOpen80043438,
    SetDumpFnt80043354,
    DisplayEnable80044AA0,
};

enum class EventTextBootStatus80027FAC : uint8_t {
    NotRun = 0,
    FontBindingUnknown,
    DisplaySinkMissing,
    HostPoolBackingUnknown,
    SoftwareStateLimit,
    Executed,
};

struct EventTextBootResult80027FAC {
    EventTextBootStatus80027FAC status = EventTextBootStatus80027FAC::NotRun;
    bool accepted = false;
    bool fontBindingInjected = false;
    bool fontUploadExecuted = false;
    bool displaySinkInvoked = false;
    bool exactPsxHalParity = false;
    bool runtimeConsumerWired = false;
    int32_t displaySinkReturn = 0;
    int32_t returnedSlot = -1;
    std::array<EventTextBootStep80027FAC, 4> steps{};
    uint32_t stepCount = 0;
    EventTextFntLoadResult80043394 fntLoad{};
    EventTextFntOpenResult80043438 fntOpen{};
    EventTextSetDumpFntResult80043354 setDump{};
};

using EventTextDisplaySink80044AA0 =
    int32_t (*)(int32_t enabled, void* userData);

struct EventTextSubmittedGlyph800436F0 {
    uint32_t psxAddress = 0;
    EventTextGlyphPacket800436F0 packet{};
};

struct EventTextFlushSubmission800436F0 {
    bool valid = false;
    uint32_t selectedSlot = 0;
    uint32_t recordAddress = 0;
    uint32_t headAddress = 0;
    uint32_t headWord = 0;
    bool recordLinked = false;
    EventTextRecord800436F0 recordBeforeCleanup{};
    uint32_t glyphCount = 0;
    std::array<EventTextSubmittedGlyph800436F0,
               kTextGlyphPacketBankCount80043394>
        glyphs{};
};

struct EventTextHostGlyphDraw800450A0 {
    int16_t x = 0;
    int16_t y = 0;
    uint8_t u = 0;
    uint8_t v = 0;
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
    uint16_t clut = 0;
    uint32_t psxAddress = 0;
};

using EventTextHostGlyphSink800450A0 =
    bool (*)(const EventTextHostGlyphDraw800450A0& draw, void* userData);

enum class EventTextFlushStatus800436F0 : uint8_t {
    NotRun = 0,
    SourceUnknown,
    InvalidSelectedSlot,
    CurrentTextPointerNull,
    HostTextPointerBindingLimit,
    InvalidGlyphPacketCursor,
    TextRangeExhausted,
    GlyphPacketRangeExhausted,
    SubmitSinkMissing,
    Executed,
};

struct EventTextFlushExecutionResult800436F0 {
    EventTextFlushStatus800436F0 status = EventTextFlushStatus800436F0::NotRun;
    bool accepted = false;
    bool usedFallbackSlot = false;
    bool submitInvoked = false;
    int32_t submitResultIgnored = 0;
    uint32_t selectedSlot = 0;
    uint32_t returnedHeadAddress = 0;
    uint32_t parserIterations = 0;
    int32_t localBudgetRemaining = 0;
    uint32_t finalTextAddress = 0;
    uint32_t finalPacketAddress = 0;
    int32_t finalX = 0;
    int32_t finalY = 0;
    int32_t maxX = 0;
    bool stoppedAtBottom = false;
    EventTextFlushSubmission800436F0 submission{};
};

// Typed host facts for the formal 80026314 MenuHelp context.  The original
// routine receives PSX pointers, but SS0 accepts only explicitly supplied
// strings and bounded group/item counts; it never dereferences guessed RAM or
// the old S0 shell.
struct EventTextMenuHelpGroup80026314 {
    const char* title = nullptr;
    std::array<const char*, kMenuHelpMaxItemsPerGroup80026314> itemLabels{};
    uint32_t itemCount = 0;
    int32_t selectedItem = -1;
};

struct EventTextMenuHelpContext80026314 {
    bool sourceKnown = false;
    const EventTextMenuHelpGroup80026314* groups = nullptr;
    uint32_t groupCount = 0;
    int32_t selectedGroup = -1;
};

enum class EventTextMenuHelpStatus80026314 : uint8_t {
    NotRun = 0,
    SourceUnknown,
    InvalidGroupCount,
    InvalidSelectedGroup,
    InvalidTitle,
    InvalidItemCount,
    InvalidItemLabel,
    StackBufferLimit,
    AppendFailed,
    FlushFailed,
    Executed,
};

struct EventTextMenuHelpResult80026314 {
    EventTextMenuHelpStatus80026314 status =
        EventTextMenuHelpStatus80026314::NotRun;
    bool accepted = false;
    bool flushInvoked = false;
    bool flushAccepted = false;
    uint32_t groupsProcessed = 0;
    uint32_t itemsProcessed = 0;
    uint32_t appendedBytes = 0;
    EventTextFlushExecutionResult800436F0 flush{};
};

// Typed host facts for the formal Event2 StageClear producer.  Current IDA
// proves the gate, prefix, six byte loads and %d formatter calls, but not a
// runtime writer for the gate/status bank.  SS0 therefore accepts only an
// explicit source and never reads replay RAM or the old S0 shell.
struct EventTextStageClearContext80026B94 {
    bool sourceKnown = false;
    bool gateKnown = false;
    bool stageClearEnabled = false;
    std::array<uint8_t, kStageClearStatusAppendCount80026DAC> statusBytes{};
};

enum class EventTextStageClearStatus80026B94 : uint8_t {
    NotRun = 0,
    SourceUnknown,
    GateUnknown,
    GateDisabled,
    AppendFailed,
    FlushFailed,
    Executed,
};

struct EventTextStageClearResult80026B94 {
    EventTextStageClearStatus80026B94 status =
        EventTextStageClearStatus80026B94::NotRun;
    bool accepted = false;
    bool flushInvoked = false;
    bool flushAccepted = false;
    uint32_t statusesAppended = 0;
    uint32_t appendedBytes = 0;
    EventTextFlushExecutionResult800436F0 flush{};
};

// Formal current-IDA 800450A0 -> 800468E0 -> 80046840 transaction.
// This is a host-side transaction record; it is not a claim that Windows
// MMIO or the original PSX DMA engine has been executed.
struct EventTextDma2Transaction800450A0 {
    bool valid = false;
    uint32_t callbackFunction = 0;
    uint32_t otagHeadAddress = 0;
    uint32_t gp1DmaDirection = 0;
    uint32_t dma2Madr = 0;
    uint32_t dma2Bcr = 0;
    uint32_t dma2Chcr = 0;
};

enum class EventTextSubmitStatus800450A0 : uint8_t {
    NotRun = 0,
    InvalidSubmission,
    InvalidHeadAddress,
    HostGpuBoundary,
    Executed,
};

struct EventTextSubmitExecutionResult800450A0 {
    EventTextSubmitStatus800450A0 status =
        EventTextSubmitStatus800450A0::NotRun;
    bool accepted = false;
    bool callbackDispatched = false;
    bool exactPsxHalParity = false;
    bool hostRuntimeConsumerWired = false;
    bool hostGlyphsSubmitted = false;
    bool hostGlyphSubmitFailed = false;
    uint32_t hostGlyphCount = 0;
    EventTextDispatchResult800468E0 dispatch{};
    EventTextDma2Transaction800450A0 transaction{};
};

struct EventTextSubmitContext800450A0 {
    bool hostGpuBoundaryKnown = false;
    bool hostRuntimeConsumerWired = false;
    EventTextHostGlyphSink800450A0 hostGlyphSink = nullptr;
    void* hostGlyphSinkUserData = nullptr;
    EventTextSubmitExecutionResult800450A0 last{};
};

using EventTextSubmitSink800450A0 =
    int32_t (*)(const EventTextFlushSubmission800436F0& submission,
                void* userData);

bool RuntimeCutoverAllowed();

uint32_t KnownEventTextEndpointSpecCount();
const EventTextEndpointSpec& KnownEventTextEndpointSpecAt(uint32_t index);

uint32_t KnownEventTextRawSpecCount();
const EventTextRawSpec& KnownEventTextRawSpecAt(uint32_t index);

uint32_t KnownEventTextCallsiteSpecCount();
const EventTextCallsiteSpec& KnownEventTextCallsiteSpecAt(uint32_t index);
const EventTextCallsiteSpec* FindEventTextCallsiteSpec(uint32_t callsite);

EventTextPlan BuildTextSystemBoot80027FACPlan();
EventTextPlan BuildTextAppendFormatter80043A14Plan(bool recordKnown = false,
                                                   bool hexDigitTableKnown = false);
EventTextPlan BuildMenuHelp80026314Plan(uint32_t ctxAddress = 0);
EventTextPlan BuildStageClearText80026B94Plan(bool gateKnown = false,
                                               bool stageClearEnabled = false,
                                               bool statusBankKnown = false);
EventTextPlan BuildTextFlush800436F0Plan(int32_t slot = -1);

bool ExecuteFntLoadSoftware80043394(
    EventTextRuntimeState800436F0& state,
    const EventTextFontBinding80043394& binding,
    EventTextFntLoadResult80043394& out);

bool ExecuteFntOpenSoftware80043438(
    EventTextRuntimeState800436F0& state,
    const EventTextFntOpenRequest80043438& request,
    EventTextFntOpenResult80043438& out);

bool ExecuteSetDumpFntSoftware80043354(
    EventTextRuntimeState800436F0& state,
    int32_t slot,
    EventTextSetDumpFntResult80043354& out);

bool ExecuteLoadImageSoftware80044D64(
    EventTextRuntimeState800436F0& state,
    const EventTextLoadImageRequest80044D64& request,
    EventTextLoadImageResult80044D64& out);

bool ExecuteTextDispatchSoftware800468E0(
    const EventTextDispatchRequest800468E0& request,
    EventTextDispatchResult800468E0& out);

bool ExecuteTextDispatchQueueSoftware800468E0(
    const EventTextDispatchQueueRequest800468E0& request,
    EventTextDispatchQueueState800468E0& state,
    EventTextDispatchQueueResult80046BC4& out);

bool ExecuteTextDispatchConsumerSoftware80046BC4(
    EventTextDispatchQueueState800468E0& state,
    EventTextDispatchCallbackSink80046BC4 callbackSink,
    void* callbackSinkUserData,
    EventTextDispatchQueueResult80046BC4& out);

bool ExecuteGpuUploadPrepareSoftware80047144(
    uint32_t currentTick,
    EventTextGpuWaitState80047144& state,
    EventTextGpuWaitResult80047178& out);

bool ExecuteGpuUploadWaitSoftware80047178(
    uint32_t currentTick,
    EventTextGpuWaitState80047144& state,
    EventTextGpuWaitResult80047178& out);

bool ExecuteLoadImageUploadSoftware800462C4(
    const EventTextLoadImageRequest80044D64& request,
    EventTextUploadResult800462C4& out);

bool ExecuteTextAppendSoftware80043A14(
    EventTextRuntimeState800436F0& state,
    const EventTextAppendRequest80043A14& request,
    EventTextAppendResult80043A14& out);

bool ExecuteTextSystemBoot80027FACSoftwareCore(
    EventTextRuntimeState800436F0& state,
    const EventTextFontBinding80043394& binding,
    EventTextDisplaySink80044AA0 displaySink,
    void* userData,
    EventTextBootResult80027FAC& out);

bool ExecuteTextFlush800436F0(
    EventTextRuntimeState800436F0& state,
    int32_t slot,
    EventTextSubmitSink800450A0 submitSink,
    void* userData,
    EventTextFlushExecutionResult800436F0& out);

bool ExecuteMenuHelpSoftware80026314(
    EventTextRuntimeState800436F0& state,
    const EventTextMenuHelpContext80026314& context,
    EventTextSubmitSink800450A0 submitSink,
    void* userData,
    EventTextMenuHelpResult80026314& out);

bool ExecuteStageClearSoftware80026B94(
    EventTextRuntimeState800436F0& state,
    const EventTextStageClearContext80026B94& context,
    EventTextSubmitSink800450A0 submitSink,
    void* userData,
    EventTextStageClearResult80026B94& out);

bool ReplayFrozenTextFlushSubmission800450A0(
    const EventTextFlushSubmission800436F0& submission,
    EventTextSubmitSink800450A0 submitSink,
    void* userData,
    int32_t& submitResultIgnored);

bool ExecuteTextSubmitSoftware800450A0(
    const EventTextFlushSubmission800436F0& submission,
    EventTextSubmitContext800450A0& context,
    EventTextSubmitExecutionResult800450A0& out);

int32_t SubmitTextOtagSoftware800450A0(
    const EventTextFlushSubmission800436F0& submission,
    void* userData);

const char* EventTextEndpointKindName(EventTextEndpointKind kind);
const char* EventTextProducerKindName(EventTextProducerKind producer);
const char* EventTextActionKindName(EventTextActionKind kind);

} // namespace PrSS0EventTextDirect
