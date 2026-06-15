#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace PrPsxFastSpriteSubmitDirect {

constexpr uint32_t kGsSortFastSpriteFunction8003FA20 = 0x8003FA20u;
constexpr uint32_t kGsSortFastSpritePacketLinkTagAddend8003FA20 =
    0x05000000u;
constexpr uint32_t kGsSortFastSpritePacketAddrMask8003FA20 = 0x00FFFFFFu;
constexpr uint32_t kGsSortFastSpritePacketWordCount8003FA20 = 6u;
constexpr uint32_t kGsSortFastSpriteAllocatorAdvanceWords8003FA20 = 6u;
constexpr uint32_t kGsSortFastSpriteAllocatorAdvanceBytes8003FA20 =
    kGsSortFastSpriteAllocatorAdvanceWords8003FA20 * 4u;
constexpr std::size_t kFastSpriteLocalByteCount8003FA20 = 24u;
constexpr std::size_t kFastSpriteLocalRgbR8003FA20 = 0x14u;
constexpr std::size_t kFastSpriteLocalRgbG8003FA20 = 0x15u;
constexpr std::size_t kFastSpriteLocalRgbB8003FA20 = 0x16u;
constexpr uint32_t kSub8001DF24HudSubmitChain = 0x8001DF24u;
constexpr uint32_t kSub8001BEE4FastSpriteWrapper = 0x8001BEE4u;
constexpr uint32_t kSub8001BE34FastSpritePrefixWriter = 0x8001BE34u;
constexpr uint32_t kSub8001B25CFastSpritePrefixWriter = 0x8001B25Cu;
constexpr uint32_t kSub8004800CFormatter = 0x8004800Cu;
constexpr uint32_t kStackSpill80048040 = 0x80048040u;
constexpr uint32_t kStackSpill8001B26C = 0x8001B26Cu;
constexpr uint32_t kStackSpill8001B274 = 0x8001B274u;
constexpr std::size_t kGsSortFastSpriteRuntimeOtSlotCapacity8003FA20 = 256u;
constexpr std::size_t kGsSortFastSpriteRuntimePacketWriteCapacity8003FA20 =
    512u;

struct FastSpriteLocalFields8003FA20 {
    uint32_t attr_00 = 0;
    int16_t x_04 = 0;
    uint16_t y_06 = 0;
    uint16_t width_08 = 0;
    uint16_t height_0A = 0;
    uint16_t tpage_0C = 0;
    uint8_t u_0E = 0;
    uint8_t v_0F = 0;
    int16_t clutX_10 = 0;
    int16_t clutY_12 = 0;
    uint8_t r_14 = 0;
    uint8_t g_15 = 0;
    uint8_t b_16 = 0;
};

enum class FastSpriteRawLocalProvenance8003FA20 : uint8_t {
    None = 0,
    // Stack residue can explain PSX bytes during audit, but is not a semantic
    // RGB owner and must not make packet word2 renderable.
    PsxStackScratchResidue,
    // Narrow PSX stack-scratch cutover for 8001B954 glyph submits: the
    // 8001BBF8 -> 8003FA20 consumer uses stable bytes seeded by the
    // 8001B25C prologue spill at 8001B26C. This is not a generic default.
    Stage1GlyphStackScratch8001B954,
    // Narrow PSX stack-scratch cutover for 80023E10 subtitle-box corners:
    // 8001C550 -> 8001B590 -> 8003FA20 consumes the observed stack tail for
    // the four 8x8 corner sprites. This is not a generic 8001B590 color.
    Stage1ScriptBoxCornerStackScratch8001B590,
    // Replay is only an entry/validation aid; it is not translation authority.
    ReplayReconstructedStackResidue,
    StaticLocalFields,
    // Candidate label only until a concrete producer/sourceKind closure proves
    // the RGB bytes. This must not authorize packet word2 by itself.
    StaticGlobalObject,
};

struct FastSpriteRawLocalBytes8003FA20 {
    bool active = false;
    std::array<uint8_t, kFastSpriteLocalByteCount8003FA20> bytes{};
    std::array<bool, kFastSpriteLocalByteCount8003FA20> known{};
    FastSpriteRawLocalProvenance8003FA20 provenance =
        FastSpriteRawLocalProvenance8003FA20::None;
    uint32_t sourceFunction = 0;
    uint32_t callsite = 0;
    uint32_t localObjectAddr = 0;
    uint32_t lastWriterPc = 0;
    const char* note = nullptr;
};

enum class FastSpriteStackScratchSource8003FA20 : uint8_t {
    Unknown = 0,
    SavedRegisterSpill80048040,
    SavedRegisterSpill8001B274,
    SavedRegisterSpill8001B26C,
    CallerOrder8001DF24,
};

struct FastSpriteStackScratchByte8003FA20 {
    bool known = false;
    uint8_t value = 0;
    std::size_t localOffset = 0;
    uint32_t writerPc = 0;
    uint32_t writerFunction = 0;
};

struct FastSpriteStackScratchTail8003FA20 {
    bool active = false;
    FastSpriteStackScratchSource8003FA20 source =
        FastSpriteStackScratchSource8003FA20::Unknown;
    uint32_t sourceFunction = 0;
    uint32_t consumerFunction = 0;
    uint32_t consumerCallsite = 0;
    bool semanticRgbProducer = false;
    bool defaultRgbForbidden = true;
    std::array<FastSpriteStackScratchByte8003FA20, 3> rgbTail{};
    const char* note = nullptr;
};

struct OrderingTableState8003FA20 {
    uint32_t headAddr_04 = 0;
    uint32_t length_08 = 0;
    uint32_t slotOldValue = 0;
};

struct DrawOffsetGlobals8003FA20 {
    int16_t word_800917AA = 0;
    int16_t word_800917AC = 0;
};

enum class FastSpriteSubmitSourceKind8003FA20 : uint8_t {
    Unknown = 0,
    Movie1RawDraw,
    Stage1Hud,
    Stage1CompactRail,
    Stage1ScriptBox,
    Stage1MovieText,
    Stage1EventFramePrompt,
    Stage1EventFrameStageSelect,
    Stage1EventFrameBackdrop,
    Stage1EventFrameSaveUi,
};

struct GsSortFastSpriteSubmitProvenance8003FA20 {
    bool active = false;
    FastSpriteSubmitSourceKind8003FA20 sourceKind =
        FastSpriteSubmitSourceKind8003FA20::Unknown;
    FastSpriteRawLocalProvenance8003FA20 rawLocalProvenance =
        FastSpriteRawLocalProvenance8003FA20::None;
    uint32_t sourceFunction = 0;
    uint32_t callsite = 0;
    uint32_t helper = 0;
    uint32_t psxCallOrder = 0;
    uint32_t localObjectAddr = 0;
    uint32_t lastWriterPc = 0;
    uint16_t priority = 0;
};

struct GsSortFastSpriteInput8003FA20 {
    FastSpriteLocalFields8003FA20 sprite{};
    FastSpriteRawLocalBytes8003FA20 rawLocalBytes{};
    GsSortFastSpriteSubmitProvenance8003FA20 provenance{};
    OrderingTableState8003FA20 ot{};
    DrawOffsetGlobals8003FA20 drawOffsets{};
    uint32_t packetAllocatorAddr_dword_800901C8 = 0;
    uint16_t priority = 0;
};

enum class GsSortFastSpriteSkipReason8003FA20 : uint8_t {
    None,
    NegativeAttr,
    ZeroWidth,
    ZeroHeight,
};

enum class GsSortFastSpriteReturnValueSource8003FA20 : uint8_t {
    Unknown,
    UnchangedV0NegativeAttr,
    WidthZero,
    HeightZero,
    OtLinkTag,
};

struct GsSortFastSpriteReturnValue8003FA20 {
    bool known = false;
    uint32_t value = 0;
    GsSortFastSpriteReturnValueSource8003FA20 source =
        GsSortFastSpriteReturnValueSource8003FA20::Unknown;
};

struct GsSortFastSpriteOtSlot8003FA20 {
    const char* expression = nullptr;
    uint32_t addr = 0;
    uint32_t oldValue = 0;
    uint32_t newValue = 0;
};

struct GsSortFastSpritePacket8003FA20 {
    bool written = false;
    uint32_t addr = 0;
    uint32_t word0_linkTag = 0;
    uint32_t word1_drawMode = 0;
    uint32_t word2_colorCode = 0;
    uint32_t word3_xy = 0;
    uint32_t word4_uvClut = 0;
    uint32_t word5_wh = 0;

    std::array<uint32_t, kGsSortFastSpritePacketWordCount8003FA20> Words()
        const {
        return {{
            word0_linkTag,
            word1_drawMode,
            word2_colorCode,
            word3_xy,
            word4_uvClut,
            word5_wh,
        }};
    }
};

struct GsSortFastSpriteAllocator8003FA20 {
    uint32_t oldAddr = 0;
    uint32_t advanceWords = 0;
    uint32_t advanceBytes = 0;
    uint32_t newAddr = 0;
};

struct GsSortFastSpriteResult8003FA20 {
    bool skipped = false;
    GsSortFastSpriteSkipReason8003FA20 skipReason =
        GsSortFastSpriteSkipReason8003FA20::None;
    GsSortFastSpritePacket8003FA20 packet{};
    GsSortFastSpriteSubmitProvenance8003FA20 provenance{};
    GsSortFastSpriteOtSlot8003FA20 otSlot{};
    GsSortFastSpriteAllocator8003FA20 allocator{};
    GsSortFastSpriteReturnValue8003FA20 returnValue{};
};

struct RuntimeOtSlotValue8003FA20 {
    bool valid = false;
    uint32_t addr = 0;
    uint32_t value = 0;
};

struct RuntimePacketWrite8003FA20 {
    bool valid = false;
    uint32_t addr = 0;
    uint32_t wordCount = kGsSortFastSpritePacketWordCount8003FA20;
    std::array<uint32_t, kGsSortFastSpritePacketWordCount8003FA20> words{};
    std::array<bool, kGsSortFastSpritePacketWordCount8003FA20> wordKnown{};
    bool word2CommandKnown = false;
    uint8_t word2CommandCode = 0;
    GsSortFastSpriteSubmitProvenance8003FA20 provenance{};
};

struct RuntimeOrderingTableState8003FA20 {
    uint32_t headAddr_04 = 0;
    uint32_t length_08 = 0;
    std::array<RuntimeOtSlotValue8003FA20,
               kGsSortFastSpriteRuntimeOtSlotCapacity8003FA20>
        slotValues{};
};

struct RuntimeState8003FA20 {
    uint32_t dword_800901C8 = 0;
    int16_t word_800917AA = 0;
    int16_t word_800917AC = 0;
    bool drawEnvOffsetKnown800401AC = false;
    int16_t drawEnvOffsetX80091738 = 0;
    int16_t drawEnvOffsetY8009173A = 0;
    RuntimeOrderingTableState8003FA20 ot{};
    std::array<RuntimePacketWrite8003FA20,
               kGsSortFastSpriteRuntimePacketWriteCapacity8003FA20>
        packetWrites{};
};

struct RuntimeUpdate8003FA20 {
    bool dryRun = false;
    bool skipped = false;
    GsSortFastSpriteSkipReason8003FA20 skipReason =
        GsSortFastSpriteSkipReason8003FA20::None;
    bool allocatorWouldUpdate = false;
    bool allocatorUpdated = false;
    bool allocatorOldMatchesRuntime = false;
    uint32_t oldAllocatorAddr = 0;
    uint32_t newAllocatorAddr = 0;
    bool packetWouldWrite = false;
    uint32_t packetAddr = 0;
    bool packetWriteMirrored = false;
    bool packetWriteCapacityExceeded = false;
    bool otSlotWouldUpdate = false;
    bool otSlotUpdated = false;
    bool otSlotOldMatchesRuntime = false;
    bool otSlotCapacityExceeded = false;
    GsSortFastSpriteOtSlot8003FA20 otSlot{};
    GsSortFastSpriteReturnValue8003FA20 returnValue{};
};

enum class GsSortFastSpritePartialGap8003FA20 : uint8_t {
    None = 0,
    LocalSpriteRgbUnknown,
    LocalSpriteRgbStaticOwnerUnclosed,
};

struct GsSortFastSpriteRgbGapEvidence8003FA20 {
    bool active = false;
    uint32_t consumerFunction = kGsSortFastSpriteFunction8003FA20;
    uint32_t firstRgbOffset = 0x14u;
    uint32_t lastRgbOffset = 0x16u;
    uint32_t packetWordIndex = 2u;
    bool defaultRgbForbidden = true;
    const char* reason = nullptr;
};

struct GsSortFastSpritePartialPacket8003FA20 {
    bool wouldWrite = false;
    uint32_t addr = 0;
    bool word0Known = false;
    bool word1Known = false;
    bool word2ColorKnown = false;
    bool word2CommandKnown = false;
    bool word3Known = false;
    bool word4Known = false;
    bool word5Known = false;
    uint32_t word0_linkTag = 0;
    uint32_t word1_drawMode = 0;
    uint32_t word2_colorCode = 0;
    uint8_t word2CommandCode = 0;
    uint32_t word3_xy = 0;
    uint32_t word4_uvClut = 0;
    uint32_t word5_wh = 0;
};

struct GsSortFastSpritePartialResult8003FA20 {
    bool wouldCall8003FA20 = false;
    bool skipped = false;
    GsSortFastSpriteSkipReason8003FA20 skipReason =
        GsSortFastSpriteSkipReason8003FA20::None;
    bool packetGeometryKnown = false;
    bool packetColorKnown = false;
    GsSortFastSpritePartialGap8003FA20 gap =
        GsSortFastSpritePartialGap8003FA20::None;
    GsSortFastSpriteRgbGapEvidence8003FA20 rgbGapEvidence{};
    GsSortFastSpritePartialPacket8003FA20 packet{};
    GsSortFastSpriteSubmitProvenance8003FA20 provenance{};
    GsSortFastSpriteOtSlot8003FA20 otSlot{};
    GsSortFastSpriteAllocator8003FA20 allocator{};
    GsSortFastSpriteReturnValue8003FA20 returnValue{};
};

GsSortFastSpriteResult8003FA20 PsxCall8003FA20_GsSortFastSprite(
    const GsSortFastSpriteInput8003FA20& input);

bool HasKnownRgbTail8003FA20(
    const FastSpriteRawLocalBytes8003FA20& rawLocalBytes);

bool HasAnyKnownRawLocalByte8003FA20(
    const FastSpriteRawLocalBytes8003FA20& rawLocalBytes);

FastSpriteRawLocalBytes8003FA20
BuildRawLocalBytesFromKnownFields8003FA20(
    const FastSpriteLocalFields8003FA20& sprite,
    const FastSpriteRawLocalBytes8003FA20& stackSeed,
    uint32_t sourceFunction,
    uint32_t callsite,
    const char* note);

FastSpriteStackScratchTail8003FA20
BuildStackScratchTailFromSavedRegisterSpill8003FA20(
    FastSpriteStackScratchSource8003FA20 source,
    uint32_t writerFunction,
    uint32_t writerPc,
    uint32_t sourceFunction,
    uint32_t consumerFunction,
    uint32_t consumerCallsite,
    uint32_t savedRegisterValue,
    bool savedRegisterKnown,
    const char* note);

FastSpriteStackScratchTail8003FA20
BuildStackScratchTail8001DF24To8001BEE4(
    uint32_t savedRegisterValue,
    bool savedRegisterKnown,
    const char* note);

FastSpriteStackScratchTail8003FA20
BuildStackScratchTail8001B25CSpillToConsumer(
    uint32_t consumerFunction,
    uint32_t consumerCallsite,
    uint32_t savedRegisterValue,
    bool savedRegisterKnown,
    const char* note);

FastSpriteStackScratchTail8003FA20
BuildStackScratchTail8001B26CSpillToConsumer(
    uint32_t consumerFunction,
    uint32_t consumerCallsite,
    uint32_t savedRegisterValue,
    bool savedRegisterKnown,
    const char* note);

FastSpriteRawLocalBytes8003FA20
ApplyStackScratchTailToRawLocalBytes8003FA20(
    const FastSpriteRawLocalBytes8003FA20& rawLocalBytes,
    const FastSpriteStackScratchTail8003FA20& stackTail);

GsSortFastSpritePartialResult8003FA20
PredictGsSortFastSpritePartial8003FA20(
    const GsSortFastSpriteInput8003FA20& input,
    bool localSpriteRgbKnown);

uint32_t ResolveRuntimeOtSlotAddr8003FA20(
    const RuntimeOrderingTableState8003FA20& ot,
    uint16_t priority);

bool TryGetRuntimeOtSlotValue8003FA20(
    const RuntimeOrderingTableState8003FA20& ot,
    uint32_t addr,
    uint32_t* outValue);

uint32_t GetRuntimeOtSlotOldValue8003FA20(
    const RuntimeOrderingTableState8003FA20& ot,
    uint32_t addr);

bool SetRuntimeOtSlotValue8003FA20(
    RuntimeOrderingTableState8003FA20& ot,
    uint32_t addr,
    uint32_t value);

bool SetRuntimeOtSlotValueForPriority8003FA20(
    RuntimeOrderingTableState8003FA20& ot,
    uint16_t priority,
    uint32_t value);

OrderingTableState8003FA20 BuildOrderingTableStateFromRuntime8003FA20(
    const RuntimeOrderingTableState8003FA20& ot,
    uint16_t priority);

GsSortFastSpriteInput8003FA20 BuildInputFromRuntime8003FA20(
    const RuntimeState8003FA20& runtime,
    const FastSpriteLocalFields8003FA20& sprite,
    uint16_t priority);

GsSortFastSpriteInput8003FA20 BuildInputFromRuntime8003FA20(
    const RuntimeState8003FA20& runtime,
    const FastSpriteLocalFields8003FA20& sprite,
    const FastSpriteRawLocalBytes8003FA20& rawLocalBytes,
    uint16_t priority);

RuntimeUpdate8003FA20 PredictRuntimeUpdate8003FA20(
    const RuntimeState8003FA20& runtime,
    const GsSortFastSpriteResult8003FA20& result);

RuntimeUpdate8003FA20 ApplyRuntimeUpdate8003FA20(
    RuntimeState8003FA20& runtime,
    const GsSortFastSpriteResult8003FA20& result);

RuntimeUpdate8003FA20 PredictRuntimeUpdate8003FA20(
    const RuntimeState8003FA20& runtime,
    const GsSortFastSpritePartialResult8003FA20& result);

RuntimeUpdate8003FA20 ApplyRuntimeUpdate8003FA20(
    RuntimeState8003FA20& runtime,
    const GsSortFastSpritePartialResult8003FA20& result);

} // namespace PrPsxFastSpriteSubmitDirect
