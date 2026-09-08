#include "pr_ss0_event_text_direct.h"

#include <charconv>
#include <cstring>
#include <limits>
#include <string>

namespace PrSS0EventTextDirect {
namespace {

constexpr EventTextEndpointSpec kEndpoints[] = {
    {kFn80027FAC,
     EventTextEndpointKind::TextSystemBoot,
     "text system boot",
     "800154F4",
     "no explicit args",
     "FntLoad(960,256), alloc default record, SetDumpFnt(actual returned slot)"},
    {kFn80043394,
     EventTextEndpointKind::FontRecord,
     "FntLoad",
     "80027FAC",
     "x=960, y=256 in boot path",
     "loads font page/clut and clears 8005CB5C record bank"},
    {kFn80043438,
     EventTextEndpointKind::FontRecord,
     "text record alloc",
     "80027FAC",
     "x,y,w,h,mode,capacity",
     "allocates 0x30-byte record, text buffer and glyph packet range"},
    {kFn80043354,
     EventTextEndpointKind::FontRecord,
     "SetDumpFnt",
     "80027FAC",
     "slot",
     "selects current text record and installs 80043A14 callback"},
    {kFn80043A14,
     EventTextEndpointKind::AppendFormat,
     "text append formatter",
     "80026314, 80026B94, SetDumpFnt callback",
     "slot/format plus MIPS varargs",
     "appends literal or formatted bytes into selected record"},
    {kFn800436F0,
     EventTextEndpointKind::Flush,
     "text flush",
     "80026314, 80026B94 frame tail",
     "slot or -1 for current",
     "builds glyph packet list, submits OTag, clears text cursor"},
    {kFn800440B8,
     EventTextEndpointKind::Helper,
     "text list-head init",
     "800436F0",
     "record+0x10",
     "initializes flush list before glyph links"},
    {kFn8004401C,
     EventTextEndpointKind::Helper,
     "text link node",
     "800436F0",
     "list head, previous, packet/node, word0",
     "links glyph packets or owning record into the text list"},
    {kFn800440D0,
     EventTextEndpointKind::Helper,
     "text record mode control",
     "80043438",
     "record, mode==2 flag",
     "sets record mode fields after header initialization"},
    {kFn800441C0,
     EventTextEndpointKind::Helper,
     "text glyph packet init",
     "80043438",
     "packet address",
     "writes packet +3=3 and +7=0x74; caller writes font clut at +0x0E"},
    {kFn80044238,
     EventTextEndpointKind::Helper,
     "text record header init",
     "80043438",
     "record",
     "initializes nonzero-mode record header"},
    {kFn80044AA0,
     EventTextEndpointKind::Helper,
     "text display enable",
     "80027FAC",
     "arg 1 in boot path",
     "final text-system boot toggle after SetDumpFnt"},
    {kFn80044D64,
     EventTextEndpointKind::Helper,
     "LoadImage",
     "8001ADEC, 800431E0, 8004308C",
     "rect, explicit pixel source",
     "logs/checks RECT then dispatches 800462C4 through 800468E0 with width 8"},
    {kFn800468E0,
     EventTextEndpointKind::Helper,
     "GPU callback dispatcher",
     "80044D64, 800450A0",
     "callback, head, copy width, callback argument",
     "immediate callback when ready or 64-slot ring enqueue with copied head words"},
    {kFn80026314,
     EventTextEndpointKind::Producer,
     "menu help text producer",
     "event frame helper path",
     "ctx: group bank, selected group, group count",
     "appends help text/menu labels and flushes current text record"},
};

constexpr EventTextRawSpec kRawSpecs[] = {
    {kTextRecordBase8005CB5C,
     "text record bank",
     "8 x 0x30-byte records; 0x180 bytes cleared by 80043394",
     "80043394/80043438/800436F0",
     "records x/y/w/h/budget/text/glyph cursors and list head"},
    {kTextRecordCountGlobal8005CCDC,
     "text record count global",
     "s32 count, max 8",
     "80043354/80043394/80043438",
     "record allocator bound and SetDumpFnt validation"},
    {kTextCurrentRecordGlobal8005CCE0,
     "current text record global",
     "s32 selected record slot",
     "80043354/80043A14/800436F0",
     "selected record when append/flush slot is negative or out of range"},
    {kTextAppendCallbackGlobal8005D730,
     "text append callback global",
     "function pointer set to 80043A14",
     "80043354",
     "PSX callback slot for current text appender"},
    {kTextGlyphCursorGlobal8005D6E4,
     "text glyph cursor global",
     "u32 cumulative text/glyph allocation cursor",
     "80043438",
     "offset into 8008A750/8008AB50 for new records"},
    {kTextRecordTextBufferBase8008A750,
     "text record byte buffer",
     "1024-byte text backing buffer",
     "80043438/80043A14/800436F0",
     "literal/formatted text storage for active records"},
    {kTextRecordGlyphBufferBase8008AB50,
     "text glyph packet buffer",
     "1024 x 0x10-byte packet backing bank",
     "80043438/800436F0",
     "glyph packet storage consumed under each record's +0x1C budget"},
    {kTextHexDigitTablePointerSlot8005D6E8,
     "text hex digit table pointer slot",
     "u32 pointer to 16-byte hex digit table",
     "80043A14",
     "formatting source for %x/%X"},
    {kTextHexDigitTableAddr8001229C,
     "text hex digit table",
     "16 bytes",
     "80043A14",
     "PSX digit bytes used by hex formatting"},
    {kTextFontPageGlobal8008EB50,
     "text font page global",
     "s16 font texture page handle",
     "80043394/80043438/800436F0",
     "font handle produced by FntLoad for text packet setup"},
    {kTextFontClutGlobal8008EB54,
     "text font clut global",
     "s16 font clut handle",
     "80043394/80043438",
     "clut word copied into each glyph packet"},
    {kData8006EBF0_MenuHelpLeadingLiteral,
     "menu help leading literal",
     "string literal family starting with four newlines",
     "80026314",
     "first append before group iteration"},
    {kData8006EBF8_MenuHelpTitleFormat,
     "menu help title format",
     "format string for group title, observed as %s:",
     "80026314",
     "vararg title append at 80026394"},
    {kData8006EC14_StageClearFormat,
     "StageClear status format/data",
     "first bytes decode as %d, followed by adjacent strings/data",
     "80026B94",
     "six vararg appends using byte_80092F1D[i]"},
    {kGlobal800916F6_StageClearGate,
     "StageClear text gate",
     "s16 flag read before event2 StageClear text branch",
     "80026B94",
     "guards all event2 StageClear append calls"},
    {kData80092F1D_StageClearStatusBytes,
     "StageClear status bytes",
     "6 status bytes consumed by event2 text branch",
     "80026B94",
     "runtime data, not a static replacement string"},
};

constexpr EventTextCallsiteSpec kCallsites[] = {
    {kCallsite80027FB8_FntLoad,
     kFn80043394,
     EventTextProducerKind::TextSystemBoot80027FAC,
     EventTextActionKind::CallFntLoad80043394,
     0,
     0,
     1,
     "FntLoad(960, 256)",
     "boot path first call"},
    {kCallsite80027FD8_TextRecordAlloc,
     kFn80043438,
     EventTextProducerKind::TextSystemBoot80027FAC,
     EventTextActionKind::AllocRecord80043438,
     kTextRecordBase8005CB5C,
     kTextRecordTextBufferBase8008A750,
     1,
     "alloc(-156,-120,320,200,0,512)",
     "default record for menu/event text"},
    {kCallsite80027FE0_SetDumpFnt,
     kFn80043354,
     EventTextProducerKind::TextSystemBoot80027FAC,
     EventTextActionKind::SelectDumpRecord80043354,
     0,
     0,
     1,
     "SetDumpFnt(FntOpen return); bootstrap return is slot 0",
     "passes the allocator return unchanged and installs 80043A14 callback"},
    {kCallsite80027FE8_TextEnable,
     kFn80044AA0,
     EventTextProducerKind::TextSystemBoot80027FAC,
     EventTextActionKind::EnableTextDisplay80044AA0,
     0,
     0,
     1,
     "80044AA0(1)",
     "final boot text-system toggle"},
    {kCallsite80026348_MenuHelpLeading,
     kFn80043A14,
     EventTextProducerKind::MenuHelp80026314,
     EventTextActionKind::AppendLiteral80043A14,
     kData8006EBF0_MenuHelpLeadingLiteral,
     0,
     1,
     "\"\\n\\n\\n\\n\"",
     "safe literal append"},
    {kCallsite80026380_MenuHelpGroupMarker,
     kFn80043A14,
     EventTextProducerKind::MenuHelp80026314,
     EventTextActionKind::AppendLiteral80043A14,
     0,
     0,
     kMenuHelpMaxGroups80026314,
     "normal \"~c000    \" or selected \"~c888 >>>\"",
     "selection comes from ctx+4 group index"},
    {kCallsite80026394_MenuHelpTitleAppend,
     kFn80043A14,
     EventTextProducerKind::MenuHelp80026314,
     EventTextActionKind::AppendFormat80043A14,
     kData8006EBF8_MenuHelpTitleFormat,
     0,
     kMenuHelpMaxGroups80026314,
     "\"%s:\" with a1=*(group+0x04)",
     "requires vararg/pointer-string map, not literal collapse"},
    {kCallsite80026424_MenuHelpItemAppend,
     kFn80043A14,
     EventTextProducerKind::MenuHelp80026314,
     EventTextActionKind::AppendMenuItemStack80043A14,
     0,
     0,
     kMenuHelpMaxItemsPerGroup80026314,
     "a0=stack marker+label padded to label length 6",
     "stack buffer is built before the append call"},
    {kCallsite80026448_MenuHelpGroupNewline,
     kFn80043A14,
     EventTextProducerKind::MenuHelp80026314,
     EventTextActionKind::AppendLiteral80043A14,
     0,
     0,
     kMenuHelpMaxGroups80026314,
     "\"\\n\"",
     "one newline after each group"},
    {kCallsite80026470_MenuHelpFooter,
     kFn80043A14,
     EventTextProducerKind::MenuHelp80026314,
     EventTextActionKind::AppendLiteral80043A14,
     0,
     0,
     1,
     "\"\\n\\n~c222      O: OK   X: CANCEL~c888\\n\"",
     "safe literal footer append"},
    {kCallsite80026478_MenuHelpFlush,
     kFn800436F0,
     EventTextProducerKind::MenuHelp80026314,
     EventTextActionKind::FlushText800436F0,
     0,
     0,
     1,
     "800436F0(-1)",
     "flush-only callsite, not an append producer"},
    {kCallsite80026D70_StageClearGate,
     kFn80026B94,
     EventTextProducerKind::StageClear80026B94,
     EventTextActionKind::StageClearGate,
     kGlobal800916F6_StageClearGate,
     0,
     1,
     "event==2 && word_800916F6 != 0",
     "branch gate only; it does not prove status-bank provenance"},
    {kCallsite80026D8C_StageClearLiteral,
     kFn80043A14,
     EventTextProducerKind::StageClear80026B94,
     EventTextActionKind::AppendLiteral80043A14,
     0,
     0,
     1,
     "\"\\n\\n\\n~c000StageClear: \"",
     "guarded by event==2 && word_800916F6 != 0"},
    {kCallsite80026DA8_StageClearStatusLoad,
     kFn80026B94,
     EventTextProducerKind::StageClear80026B94,
     EventTextActionKind::GateStageClearTextSource80026B94,
     kGlobal800916F6_StageClearGate,
     kData80092F1D_StageClearStatusBytes,
     kStageClearStatusAppendCount80026DAC,
     "word_800916F6 gate plus byte_80092F1D[i], i=0..5",
     "binds the text branch to the typed status-bank source"},
    {kCallsite80026DAC_StageClearStatusAppend,
     kFn80043A14,
     EventTextProducerKind::StageClear80026B94,
     EventTextActionKind::AppendStageClearStatus80043A14,
     kData8006EC14_StageClearFormat,
     kData80092F1D_StageClearStatusBytes,
     kStageClearStatusAppendCount80026DAC,
     "a0=&8006EC14, a1=byte_80092F1D[i], i=0..5",
     "vararg/data-table call, not a hardcoded status string"},
    {kCallsite80026DD4_StageClearTailFlush,
     kFn800436F0,
     EventTextProducerKind::StageClear80026B94,
     EventTextActionKind::FlushText800436F0,
     0,
     0,
     1,
     "800436F0(-1)",
     "frame-tail flush after wait/end-frame"},
    {kCallsite80043C64_TextHexTableLoad,
     kFn80043A14,
     EventTextProducerKind::TextAppendFormatter80043A14,
     EventTextActionKind::LoadHexDigitTable80043A14,
     kTextHexDigitTablePointerSlot8005D6E8,
     kTextHexDigitTableAddr8001229C,
     1,
     "off_8005D6E8 -> 0x8001229C, 16 digit bytes",
     "source for %x/%X; do not use C printf case rules"},
    {kCallsite800437D0_TextFlushInitList,
     kFn800440B8,
     EventTextProducerKind::TextFlush800436F0,
     EventTextActionKind::InitListHead800440B8,
     0,
     0,
     1,
     "record+0x10 list head",
     "first accepted normal flush step, before empty/budget checks"},
    {kCallsite80043914_TextFlushGlyphLink,
     kFn8004401C,
     EventTextProducerKind::TextFlush800436F0,
     EventTextActionKind::LinkGlyphPacket8004401C,
     kTextRecordGlyphBufferBase8008AB50,
     0,
     kTextRecordMaxCapacity80043438,
     "link glyph packet while parsing text bytes",
     "subject to local record +0x1C budget and clipping"},
    {kCallsite80043994_TextFlushRecordLink,
     kFn8004401C,
     EventTextProducerKind::TextFlush800436F0,
     EventTextActionKind::LinkRecord8004401C,
     kTextRecordBase8005CB5C,
     0,
     1,
     "optional record link when control byte is nonzero",
     "conditional record-level node"},
    {kCallsite800439C8_TextFlushSubmit,
     kFn800450A0,
     EventTextProducerKind::TextFlush800436F0,
     EventTextActionKind::SubmitTextOtag800450A0,
     0,
     0,
     1,
     "DrawOtag(record+0x10)",
     "render HAL boundary for text list submit"},
};

bool Append(EventTextPlan& plan, const EventTextAction& action)
{
    if (plan.count >= sizeof(plan.actions) / sizeof(plan.actions[0])) {
        plan.truncated = true;
        return false;
    }
    plan.actions[plan.count++] = action;
    return true;
}

void AppendAction(EventTextPlan& plan,
                  EventTextActionKind kind,
                  uint32_t psxFunction,
                  uint32_t callsite = 0,
                  uint32_t dataAddress = 0,
                  uint32_t auxAddress = 0,
                  int32_t arg0 = 0,
                  int32_t arg1 = 0,
                  int32_t arg2 = 0,
                  int32_t arg3 = 0,
                  int32_t arg4 = 0,
                  int32_t arg5 = 0,
                  uint32_t repeatCount = 1,
                  const char* literal = nullptr,
                  bool conditional = false)
{
    EventTextAction action{};
    action.kind = kind;
    action.producer = plan.producer;
    action.psxFunction = psxFunction;
    action.callsite = callsite;
    action.dataAddress = dataAddress;
    action.auxAddress = auxAddress;
    action.args[0] = arg0;
    action.args[1] = arg1;
    action.args[2] = arg2;
    action.args[3] = arg3;
    action.args[4] = arg4;
    action.args[5] = arg5;
    action.repeatCount = repeatCount;
    action.literal = literal;
    action.conditional = conditional;
    (void)Append(plan, action);
}

EventTextPlan MakePlan(const char* name, EventTextProducerKind producer)
{
    EventTextPlan plan{};
    plan.name = name;
    plan.runtimeCutoverAllowed = RuntimeCutoverAllowed();
    plan.producer = producer;
    return plan;
}

constexpr uint32_t kLow24Mask8004401C = 0x00FFFFFFu;
constexpr uint32_t kHigh8Mask8004401C = 0xFF000000u;

void InitListHead800440B8(uint32_t& headWord)
{
    headWord = (headWord & kHigh8Mask8004401C) | kLow24Mask8004401C;
}

void LinkNode8004401C(uint32_t nodeAddress,
                      uint32_t& nodeWord,
                      uint32_t& headWord)
{
    const uint32_t oldHeadWord = headWord;
    nodeWord = (nodeWord & kHigh8Mask8004401C) |
               (oldHeadWord & kLow24Mask8004401C);
    headWord = (headWord & kHigh8Mask8004401C) |
               (nodeAddress & kLow24Mask8004401C);
}

bool TextBankOffset800436F0(uint32_t address, uint32_t& offset)
{
    if (address < kTextRecordTextBufferBase8008A750) {
        return false;
    }
    offset = address - kTextRecordTextBufferBase8008A750;
    return offset < kTextBufferBankByteCount80043394;
}

bool GlyphBankIndex800436F0(uint32_t address,
                           uint32_t& index,
                           bool allowEnd)
{
    if (address < kTextRecordGlyphBufferBase8008AB50) {
        return false;
    }
    const uint32_t byteOffset = address - kTextRecordGlyphBufferBase8008AB50;
    const uint32_t bankBytes =
        kTextGlyphPacketBankCount80043394 * kTextGlyphPacketStride800436F0;
    if ((byteOffset % kTextGlyphPacketStride800436F0) != 0u ||
        byteOffset > bankBytes || (!allowEnd && byteOffset == bankBytes)) {
        return false;
    }
    index = byteOffset / kTextGlyphPacketStride800436F0;
    return true;
}

bool FailTextFlush800436F0(EventTextFlushExecutionResult800436F0& out,
                           EventTextFlushStatus800436F0 status)
{
    out.status = status;
    return false;
}

int32_t AddMipsS32(int32_t lhs, int32_t rhs)
{
    return static_cast<int32_t>(static_cast<uint32_t>(lhs) +
                                static_cast<uint32_t>(rhs));
}

int32_t SubMipsS32(int32_t lhs, int32_t rhs)
{
    return static_cast<int32_t>(static_cast<uint32_t>(lhs) -
                                static_cast<uint32_t>(rhs));
}

bool RecordCountKnown8005CCDC(const EventTextRuntimeState800436F0& state)
{
    return state.globalsKnown || state.recordCountKnown;
}

bool CurrentRecordKnown8005CCE0(const EventTextRuntimeState800436F0& state)
{
    return state.globalsKnown || state.currentRecordKnown;
}

bool AllocationCursorKnown8005D6E4(
    const EventTextRuntimeState800436F0& state)
{
    return state.globalsKnown || state.allocationCursorKnown;
}

void RefreshAggregateGlobalsKnown800436F0(
    EventTextRuntimeState800436F0& state)
{
    state.globalsKnown =
        state.globalsKnown ||
        (state.recordCountKnown && state.currentRecordKnown &&
         state.allocationCursorKnown && state.appendCallbackKnown);
}

uint32_t BuildFntOpenDrawTPageWord80045C30(uint16_t tpage,
                                           uint8_t gpuType8005D734)
{
    const uint32_t mask =
        gpuType8005D734 == 1u || gpuType8005D734 == 2u ? 0x27FFu
                                                       : 0x09FFu;
    return 0xE1000000u | (static_cast<uint32_t>(tpage) & mask);
}

void InitFntOpenRecordHeader80045934(EventTextRecord800436F0& record,
                                     uint16_t tpage,
                                     uint8_t gpuType8005D734)
{
    record.listHeadWord10 =
        (record.listHeadWord10 & 0x00FFFFFFu) | 0x02000000u;
    record.word14 = BuildFntOpenDrawTPageWord80045C30(
        tpage, gpuType8005D734);
    record.word18 = 0xE2000000u;
}

void InitFntOpenGlyphPacket800441C0(EventTextGlyphPacket800436F0& packet,
                                    uint16_t clut)
{
    packet.word0 = (packet.word0 & 0x00FFFFFFu) | 0x03000000u;
    packet.code = 0x74u;
    packet.clut = clut;
}

bool ComputeFntLoadFontBinding80043394Impl(
    int32_t x,
    int32_t y,
    int32_t imageMode,
    uint8_t gpuType8005D734,
    EventTextFontBinding80043394& out)
{
    out = EventTextFontBinding80043394{};
    // 8004308C accepts only the three PSX texture modes and FntLoad's
    // 800431E0 CLUT coordinates must stay in the 10-bit VRAM domain.
    if (imageMode < 0 || imageMode > 2 || x < 0 || x > 0x3FF ||
        y < 0 || y > 0x3FF || y + 128 > 0x3FF) {
        return false;
    }

    // 80043DF4: GetTPage(imageMode, 0, x, y).  The branch is selected by
    // 80044A24's GPU type exactly as the original helper does.
    const uint32_t ux = static_cast<uint32_t>(x);
    const uint32_t uy = static_cast<uint32_t>(y);
    uint32_t tpage = 0u;
    if (gpuType8005D734 == 1u || gpuType8005D734 == 2u) {
        tpage = ((static_cast<uint32_t>(imageMode) & 3u) << 9u) |
                ((uy & 0x300u) >> 3u) | ((ux & 0x3FFu) >> 6u);
    } else {
        tpage = ((static_cast<uint32_t>(imageMode) & 3u) << 7u) |
                ((uy & 0x100u) >> 4u) |
                ((ux & 0x3FFu) >> 6u) | ((uy & 0x200u) << 2u);
    }

    // 80043EBC: GetClut(x, y + 128), returned as a 16-bit PSX handle.
    const uint32_t clutY = static_cast<uint32_t>(y + 128);
    const uint32_t clut = ((clutY << 6u) | ((ux >> 4u) & 0x3Fu)) & 0xFFFFu;
    out.known = true;
    out.tpage = static_cast<uint16_t>(tpage & 0xFFFFu);
    out.clut = static_cast<uint16_t>(clut);
    out.gpuType8005D734 = gpuType8005D734;
    return true;
}

bool AppendBootStep80027FAC(EventTextBootResult80027FAC& out,
                            EventTextBootStep80027FAC step)
{
    if (out.stepCount >= out.steps.size()) {
        return false;
    }
    out.steps[out.stepCount++] = step;
    return true;
}

} // namespace

bool ComputeFntLoadFontBinding80043394(
    int32_t x,
    int32_t y,
    int32_t imageMode,
    uint8_t gpuType8005D734,
    EventTextFontBinding80043394& out)
{
    return ComputeFntLoadFontBinding80043394Impl(
        x, y, imageMode, gpuType8005D734, out);
}

bool DecodeEmbeddedFont8005CCE4(
    const uint8_t* originalExe,
    size_t originalExeSize,
    size_t fileOffset,
    EventTextEmbeddedFont8005CCE4& out)
{
    out = EventTextEmbeddedFont8005CCE4{};
    out.fileOffset = static_cast<uint32_t>(fileOffset);
    if (originalExe == nullptr ||
        fileOffset > originalExeSize ||
        originalExeSize - fileOffset <
            static_cast<size_t>(kEmbeddedFontClutByteCount) +
                static_cast<size_t>(0x200u) +
                static_cast<size_t>(kEmbeddedFontImageByteCount)) {
        return false;
    }

    std::memcpy(out.clutBytes.data(),
                originalExe + fileOffset,
                out.clutBytes.size());

    for (size_t index = 0; index < out.clut.size(); ++index) {
        const size_t byteOffset = fileOffset + index * 2u;
        out.clut[index] = static_cast<uint16_t>(originalExe[byteOffset]) |
                          static_cast<uint16_t>(
                              static_cast<uint16_t>(originalExe[byteOffset + 1u])
                              << 8u);
    }

    const size_t imageOffset = fileOffset + 0x200u;
    std::memcpy(out.imageBytes.data(),
                originalExe + imageOffset,
                out.imageBytes.size());
    const auto convertColor = [](uint16_t color) -> uint32_t {
        uint8_t r = static_cast<uint8_t>((color & 0x1Fu) << 3u);
        uint8_t g = static_cast<uint8_t>(((color >> 5u) & 0x1Fu) << 3u);
        uint8_t b = static_cast<uint8_t>(((color >> 10u) & 0x1Fu) << 3u);
        const uint16_t rgb = static_cast<uint16_t>(color & 0x7FFFu);
        const uint8_t alpha =
            (rgb == 0u && (color & 0x8000u) == 0u) ? 0u : 255u;
        r = static_cast<uint8_t>(r | (r >> 5u));
        g = static_cast<uint8_t>(g | (g >> 5u));
        b = static_cast<uint8_t>(b | (b >> 5u));
        return (static_cast<uint32_t>(alpha) << 24u) |
               (static_cast<uint32_t>(b) << 16u) |
               (static_cast<uint32_t>(g) << 8u) |
               static_cast<uint32_t>(r);
    };

    size_t source = 0u;
    for (uint32_t y = 0; y < kEmbeddedFontHeight; ++y) {
        for (uint32_t x = 0; x < kEmbeddedFontWidth; x += 2u) {
            const uint8_t packed = out.imageBytes[source++];
            const uint8_t lo = static_cast<uint8_t>(packed & 0x0Fu);
            const uint8_t hi = static_cast<uint8_t>((packed >> 4u) & 0x0Fu);
            out.rgba[static_cast<size_t>(y) * kEmbeddedFontWidth + x] =
                convertColor(out.clut[lo]);
            out.rgba[static_cast<size_t>(y) * kEmbeddedFontWidth + x + 1u] =
                convertColor(out.clut[hi]);
        }
    }
    out.sourceKnown = true;
    return true;
}

bool RuntimeCutoverAllowed()
{
    return false;
}

uint32_t KnownEventTextEndpointSpecCount()
{
    return sizeof(kEndpoints) / sizeof(kEndpoints[0]);
}

const EventTextEndpointSpec& KnownEventTextEndpointSpecAt(uint32_t index)
{
    if (index >= KnownEventTextEndpointSpecCount()) {
        index = KnownEventTextEndpointSpecCount() - 1u;
    }
    return kEndpoints[index];
}

uint32_t KnownEventTextRawSpecCount()
{
    return sizeof(kRawSpecs) / sizeof(kRawSpecs[0]);
}

const EventTextRawSpec& KnownEventTextRawSpecAt(uint32_t index)
{
    if (index >= KnownEventTextRawSpecCount()) {
        index = KnownEventTextRawSpecCount() - 1u;
    }
    return kRawSpecs[index];
}

uint32_t KnownEventTextCallsiteSpecCount()
{
    return sizeof(kCallsites) / sizeof(kCallsites[0]);
}

const EventTextCallsiteSpec& KnownEventTextCallsiteSpecAt(uint32_t index)
{
    if (index >= KnownEventTextCallsiteSpecCount()) {
        index = KnownEventTextCallsiteSpecCount() - 1u;
    }
    return kCallsites[index];
}

const EventTextCallsiteSpec* FindEventTextCallsiteSpec(uint32_t callsite)
{
    for (uint32_t i = 0; i < KnownEventTextCallsiteSpecCount(); ++i) {
        if (kCallsites[i].callsite == callsite) {
            return &kCallsites[i];
        }
    }
    return nullptr;
}

EventTextPlan BuildTextSystemBoot80027FACPlan()
{
    EventTextPlan plan = MakePlan("TextSystemBoot80027FAC",
                                  EventTextProducerKind::TextSystemBoot80027FAC);
    AppendAction(plan,
                 EventTextActionKind::CallFntLoad80043394,
                 kFn80043394,
                 kCallsite80027FB8_FntLoad,
                 0,
                 0,
                 960,
                 256);
    AppendAction(plan,
                 EventTextActionKind::ClearRecordBank80043394,
                 kFn80043394,
                 0,
                 kTextRecordBase8005CB5C,
                 0,
                 static_cast<int32_t>(kTextRecordBankClearBytes80043394));
    AppendAction(plan,
                 EventTextActionKind::AllocRecord80043438,
                 kFn80043438,
                 kCallsite80027FD8_TextRecordAlloc,
                 kTextRecordBase8005CB5C,
                 kTextRecordTextBufferBase8008A750,
                 -156,
                 -120,
                 320,
                 200,
                 0,
                 512);
    AppendAction(plan,
                 EventTextActionKind::InitGlyphPackets800441C0,
                 kFn800441C0,
                 0,
                 kTextRecordGlyphBufferBase8008AB50,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 512);
    AppendAction(plan,
                 EventTextActionKind::SelectDumpRecord80043354,
                 kFn80043354,
                 kCallsite80027FE0_SetDumpFnt,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 1,
                 "actual FntOpen returned slot; bootstrap resolves to 0");
    AppendAction(plan,
                 EventTextActionKind::RegisterAppendCallback80043354,
                 kFn80043354,
                 kCallsite80027FE0_SetDumpFnt,
                 kFn80043A14,
                 0,
                 0);
    AppendAction(plan,
                 EventTextActionKind::EnableTextDisplay80044AA0,
                 kFn80044AA0,
                 kCallsite80027FE8_TextEnable,
                 0,
                 0,
                 1);
    AppendAction(plan,
                 EventTextActionKind::Gap,
                 kFn80043394,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 1,
                 "software boot core accepts formal font binding; GPU upload and display HAL remain external");
    return plan;
}

EventTextPlan BuildTextAppendFormatter80043A14Plan(bool recordKnown,
                                                   bool hexDigitTableKnown)
{
    EventTextPlan plan = MakePlan("TextAppendFormatter80043A14",
                                  EventTextProducerKind::TextAppendFormatter80043A14);
    AppendAction(plan,
                 EventTextActionKind::GateAppendRecord80043A14,
                 kFn80043A14,
                 0,
                 kTextRecordBase8005CB5C,
                 kTextRecordTextBufferBase8008A750,
                 recordKnown ? 1 : 0,
                 static_cast<int32_t>(kTextRecordMaxCapacity80043438),
                 static_cast<int32_t>(kTextRecordStride800436F0),
                 0,
                 0,
                 0,
                 1,
                 "slot/current record, +0x24 text pointer, +0x28 length, +0x1C capacity",
                 true);
    AppendAction(plan,
                 EventTextActionKind::AppendFormat80043A14,
                 kFn80043A14,
                 0,
                 kTextRecordTextBufferBase8008A750,
                 kTextHexDigitTablePointerSlot8005D6E8,
                 recordKnown ? 1 : 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 1,
                 "literal/%%/%d/%x/%X/%c/%s PSX vararg parser",
                 true);
    AppendAction(plan,
                 EventTextActionKind::LoadHexDigitTable80043A14,
                 kFn80043A14,
                 kCallsite80043C64_TextHexTableLoad,
                 kTextHexDigitTablePointerSlot8005D6E8,
                 kTextHexDigitTableAddr8001229C,
                 hexDigitTableKnown ? 1 : 0,
                 static_cast<int32_t>(kTextHexDigitTableByteCount80043A14),
                 0,
                 0,
                 0,
                 0,
                 1,
                 "off_8005D6E8 -> 0x8001229C; table bytes 0123456789ABCDEF; source for %x/%X",
                 true);
    AppendAction(plan,
                 EventTextActionKind::Gap,
                 kFn80043A14,
                 0,
                 kTextHexDigitTablePointerSlot8005D6E8,
                 kTextHexDigitTableAddr8001229C,
                 recordKnown ? 1 : 0,
                 hexDigitTableKnown ? 1 : 0,
                 0,
                 0,
                 0,
                 0,
                 1,
                 "PSX vararg ABI, caller pointer sources, and formatter runtime remain open");
    return plan;
}

EventTextPlan BuildMenuHelp80026314Plan(uint32_t ctxAddress)
{
    EventTextPlan plan = MakePlan("MenuHelpTextProducer80026314",
                                  EventTextProducerKind::MenuHelp80026314);
    const bool ctxKnown = ctxAddress != 0u;
    const uint32_t knownCtxAddress = ctxKnown ? ctxAddress : 0u;
    const uint32_t groupBankSlot =
        ctxKnown ? ctxAddress + kMenuHelpCtxGroupBankOffset80026314 : 0u;
    const uint32_t selectedGroupSlot =
        ctxKnown ? ctxAddress + kMenuHelpCtxSelectedGroupOffset80026314 : 0u;
    const uint32_t groupCountSlot =
        ctxKnown ? ctxAddress + kMenuHelpCtxGroupCountOffset80026314 : 0u;

    AppendAction(plan,
                 EventTextActionKind::GateMenuHelpContext80026314,
                 kFn80026314,
                 0,
                 knownCtxAddress,
                 groupBankSlot,
                 static_cast<int32_t>(kMenuHelpCtxGroupCountOffset80026314),
                 static_cast<int32_t>(kMenuHelpCtxSelectedGroupOffset80026314),
                 static_cast<int32_t>(kMenuHelpGroupStride80026314),
                 static_cast<int32_t>(kMenuHelpMaxGroups80026314),
                 static_cast<int32_t>(kMenuHelpMaxItemsPerGroup80026314),
                 ctxKnown ? 1 : 0,
                 1,
                 "ctx: group bank pointer, selected group, group count",
                 true);
    AppendAction(plan,
                 EventTextActionKind::AppendLiteral80043A14,
                 kFn80043A14,
                 kCallsite80026348_MenuHelpLeading,
                 kData8006EBF0_MenuHelpLeadingLiteral,
                 knownCtxAddress,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 1,
                 "\n\n\n\n");
    AppendAction(plan,
                 EventTextActionKind::AppendLiteral80043A14,
                 kFn80043A14,
                 kCallsite80026380_MenuHelpGroupMarker,
                 0,
                 selectedGroupSlot,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 kMenuHelpMaxGroups80026314,
                 "normal group \"~c000    \", selected group \"~c888 >>>\"",
                 true);
    AppendAction(plan,
                 EventTextActionKind::AppendFormat80043A14,
                 kFn80043A14,
                 kCallsite80026394_MenuHelpTitleAppend,
                 kData8006EBF8_MenuHelpTitleFormat,
                 kCallsite80026388_MenuHelpTitlePointer,
                 static_cast<int32_t>(kMenuHelpGroupTitlePointerOffset80026314),
                 static_cast<int32_t>(kMenuHelpGroupStride80026314),
                 0,
                 0,
                 0,
                 0,
                 kMenuHelpMaxGroups80026314,
                 "%s:",
                 true);
    AppendAction(plan,
                 EventTextActionKind::BuildMenuItemStackBuffer,
                 kFn80026314,
                 kCallsite800263D4_MenuHelpItemPointer,
                 0,
                 groupBankSlot,
                 static_cast<int32_t>(kMenuHelpGroupItemLabelPointerBaseOffset80026314),
                 static_cast<int32_t>(kMenuHelpGroupSelectedItemOffset80026314),
                 static_cast<int32_t>(kMenuHelpStackTextCapacity80026314),
                 6,
                 0,
                 0,
                 kMenuHelpMaxItemsPerGroup80026314,
                 "item marker + label padded with spaces to label length 6",
                 true);
    AppendAction(plan,
                 EventTextActionKind::AppendMenuItemStack80043A14,
                 kFn80043A14,
                 kCallsite80026424_MenuHelpItemAppend,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 kMenuHelpMaxItemsPerGroup80026314,
                 "completed stack string",
                 true);
    AppendAction(plan,
                 EventTextActionKind::AppendLiteral80043A14,
                 kFn80043A14,
                 kCallsite80026448_MenuHelpGroupNewline,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 kMenuHelpMaxGroups80026314,
                 "\n",
                 true);
    AppendAction(plan,
                 EventTextActionKind::AppendLiteral80043A14,
                 kFn80043A14,
                 kCallsite80026470_MenuHelpFooter,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 1,
                 "\n\n~c222      O: OK   X: CANCEL~c888\n");
    AppendAction(plan,
                 EventTextActionKind::FlushText800436F0,
                 kFn800436F0,
                 kCallsite80026478_MenuHelpFlush,
                 0,
                 0,
                 -1);
    AppendAction(plan,
                 EventTextActionKind::Gap,
                 kFn80026314,
                 0,
                 groupCountSlot,
                 knownCtxAddress,
                 ctxKnown ? 1 : 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 1,
                 ctxKnown
                     ? "menu title/item PSX pointer to host string map remains open"
                     : "menu help ctx source is unknown; title/item pointer map remains open");
    return plan;
}

EventTextPlan BuildStageClearText80026B94Plan(bool gateKnown,
                                               bool stageClearEnabled,
                                               bool statusBankKnown)
{
    EventTextPlan plan = MakePlan("StageClearTextProducer80026B94",
                                  EventTextProducerKind::StageClear80026B94);
    AppendAction(plan,
                 EventTextActionKind::StageClearGate,
                 kFn80026B94,
                 kCallsite80026D70_StageClearGate,
                 0,
                 0,
                 gateKnown ? 1 : 0,
                 stageClearEnabled ? 1 : 0,
                 2,
                 0,
                 0,
                 0,
                 1,
                 "event==2 && word_800916F6 != 0",
                 true);
    AppendAction(plan,
                 EventTextActionKind::GateStageClearTextSource80026B94,
                 kFn80026B94,
                 kCallsite80026DA8_StageClearStatusLoad,
                 kGlobal800916F6_StageClearGate,
                 kData80092F1D_StageClearStatusBytes,
                 gateKnown ? 1 : 0,
                 stageClearEnabled ? 1 : 0,
                 statusBankKnown ? 1 : 0,
                 static_cast<int32_t>(kStageClearStatusAppendCount80026DAC),
                 2,
                 0,
                 1,
                 "gate/status source for event2 StageClear text",
                 true);
    if (!gateKnown || stageClearEnabled) {
        AppendAction(plan,
                     EventTextActionKind::AppendLiteral80043A14,
                     kFn80043A14,
                     kCallsite80026D8C_StageClearLiteral,
                     0,
                     0,
                     0,
                     0,
                     0,
                     0,
                     0,
                     0,
                     1,
                     "\n\n\n~c000StageClear: ",
                     true);
        AppendAction(plan,
                     EventTextActionKind::AppendStageClearStatus80043A14,
                     kFn80043A14,
                     kCallsite80026DAC_StageClearStatusAppend,
                     kData8006EC14_StageClearFormat,
                     kData80092F1D_StageClearStatusBytes,
                     statusBankKnown ? 1 : 0,
                     static_cast<int32_t>(kStageClearStatusAppendCount80026DAC),
                     0,
                     0,
                     0,
                     0,
                     kStageClearStatusAppendCount80026DAC,
                     "&8006EC14 with byte_80092F1D[i]",
                     true);
    }
    AppendAction(plan,
                 EventTextActionKind::FlushText800436F0,
                 kFn800436F0,
                 kCallsite80026DD4_StageClearTailFlush,
                 0,
                 0,
                 -1,
                 0,
                 0,
                 0,
                 0,
                 0,
                 1,
                 "after 80035560(0) and 8001EA00(event)",
                 true);
    AppendAction(plan,
                 EventTextActionKind::Gap,
                 kFn80026B94,
                 0,
                 kData8006EC14_StageClearFormat,
                 kData80092F1D_StageClearStatusBytes,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 1,
                 statusBankKnown
                     ? "StageClear vararg formatter remains open"
                     : "StageClear vararg formatter/status producer remains open");
    return plan;
}

EventTextPlan BuildTextFlush800436F0Plan(int32_t slot)
{
    EventTextPlan plan = MakePlan("TextFlush800436F0",
                                  EventTextProducerKind::TextFlush800436F0);
    AppendAction(plan,
                 EventTextActionKind::InitListHead800440B8,
                 kFn800440B8,
                 kCallsite800437D0_TextFlushInitList,
                 kTextRecordBase8005CB5C,
                 0,
                 slot);
    AppendAction(plan,
                 EventTextActionKind::ParseTextByteStream,
                 kFn800436F0,
                 0,
                 kTextRecordTextBufferBase8008A750,
                 0,
                 slot,
                 static_cast<int32_t>(kTextRecordMaxCapacity80043438),
                 8,
                 0,
                 0,
                 0,
                 1,
                 "space/tab/newline/~cRGB/glyph parsing");
    AppendAction(plan,
                 EventTextActionKind::LinkGlyphPacket8004401C,
                 kFn8004401C,
                 kCallsite80043914_TextFlushGlyphLink,
                 kTextRecordGlyphBufferBase8008AB50,
                 0,
                 slot,
                 static_cast<int32_t>(kTextGlyphPacketStride800436F0),
                 0,
                 0,
                 0,
                 0,
                 kTextRecordMaxCapacity80043438,
                 "one node per visible glyph while local record +0x1C budget remains",
                 true);
    AppendAction(plan,
                 EventTextActionKind::LinkRecord8004401C,
                 kFn8004401C,
                 kCallsite80043994_TextFlushRecordLink,
                 kTextRecordBase8005CB5C,
                 0,
                 slot,
                 0,
                 0,
                 0,
                 0,
                 0,
                 1,
                 "record control byte nonzero",
                 true);
    AppendAction(plan,
                 EventTextActionKind::SubmitTextOtag800450A0,
                 kFn800450A0,
                 kCallsite800439C8_TextFlushSubmit,
                 kTextRecordBase8005CB5C,
                 0,
                 slot);
    AppendAction(plan,
                 EventTextActionKind::ClearTextAfterFlush,
                 kFn800436F0,
                 0,
                 kTextRecordTextBufferBase8008A750,
                 0,
                 slot);
    AppendAction(plan,
                 EventTextActionKind::Gap,
                 kFn800436F0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 1,
                 "software core is executable; record allocation and host submit/render bridge remain open");
    return plan;
}

bool ExecuteFntLoadSoftware80043394(
    EventTextRuntimeState800436F0& state,
    const EventTextFontBinding80043394& binding,
    EventTextFntLoadResult80043394& out)
{
    out = EventTextFntLoadResult80043394{};
    if (!binding.known) {
        out.status = EventTextFntLoadStatus80043394::FontBindingUnknown;
        return false;
    }

    EventTextRuntimeState800436F0 candidate = state;
    candidate.records.fill(EventTextRecord800436F0{});
    candidate.recordCount8005CCDC = 0;
    candidate.fontTPage8008EB50 = binding.tpage;
    candidate.fontClut8008EB54 = binding.clut;
    candidate.gpuType8005D734 = binding.gpuType8005D734;
    candidate.recordBankKnown = true;
    candidate.recordCountKnown = true;
    candidate.fontBindingKnown = true;
    RefreshAggregateGlobalsKnown800436F0(candidate);
    state = candidate;

    out.status = EventTextFntLoadStatus80043394::Executed;
    out.accepted = true;
    out.returnedRecordBankAddress = kTextRecordBase8005CB5C;
    return true;
}

bool ExecuteFntOpenSoftware80043438(
    EventTextRuntimeState800436F0& state,
    const EventTextFntOpenRequest80043438& request,
    EventTextFntOpenResult80043438& out)
{
    out = EventTextFntOpenResult80043438{};
    if (!RecordCountKnown8005CCDC(state)) {
        out.status = EventTextFntOpenStatus80043438::SourceUnknown;
        return false;
    }
    if (state.recordCount8005CCDC >=
        static_cast<int32_t>(kTextRecordCount800436F0)) {
        out.status = EventTextFntOpenStatus80043438::RecordLimit;
        out.accepted = true;
        if (AllocationCursorKnown8005D6E4(state)) {
            out.oldCursor = state.allocationCursor8005D6E4;
            out.newCursor = state.allocationCursor8005D6E4;
        }
        return true;
    }
    if (state.recordCount8005CCDC < 0) {
        out.status = EventTextFntOpenStatus80043438::HostPoolBindingLimit;
        return false;
    }
    if (!state.recordBankKnown || !state.textBankKnown ||
        !state.fontBindingKnown) {
        out.status = EventTextFntOpenStatus80043438::SourceUnknown;
        return false;
    }
    if (request.mode != 0) {
        out.status =
            EventTextFntOpenStatus80043438::HostUnsupportedNonzeroMode;
        return false;
    }

    EventTextRuntimeState800436F0 candidate = state;
    const int32_t slot = candidate.recordCount8005CCDC;
    if (slot == 0) {
        candidate.allocationCursor8005D6E4 = 0;
        candidate.allocationCursorKnown = true;
    } else if (!AllocationCursorKnown8005D6E4(candidate)) {
        out.status = EventTextFntOpenStatus80043438::SourceUnknown;
        return false;
    }
    const int32_t cursor = candidate.allocationCursor8005D6E4;
    int32_t effectiveCapacity = request.requestedCapacity;
    const int32_t summed = AddMipsS32(effectiveCapacity, cursor);
    if (summed >= 1025) {
        effectiveCapacity = SubMipsS32(
            static_cast<int32_t>(kTextRecordMaxCapacity80043438), cursor);
    }

    if (cursor < 0 ||
        cursor >= static_cast<int32_t>(kTextBufferBankByteCount80043394) ||
        effectiveCapacity < 0 ||
        effectiveCapacity >
            static_cast<int32_t>(kTextGlyphPacketBankCount80043394) - cursor) {
        out.status = EventTextFntOpenStatus80043438::HostPoolBindingLimit;
        return false;
    }
    if (effectiveCapacity > 0 && !state.glyphBankKnown) {
        out.status = EventTextFntOpenStatus80043438::SourceUnknown;
        return false;
    }

    EventTextRecord800436F0& record = candidate.records[slot];
    record.measureMode2C = request.width == 0 ? 1 : 0;
    InitFntOpenRecordHeader80045934(record,
                                     candidate.fontTPage8008EB50,
                                     candidate.gpuType8005D734);
    record.x = static_cast<int16_t>(request.x);
    record.y = static_cast<int16_t>(request.y);
    record.w = static_cast<int16_t>(request.width);
    record.h = static_cast<int16_t>(request.height);
    record.parseBudget1C = effectiveCapacity;
    record.appendCursor28 = 0u;
    record.textPointer24 =
        kTextRecordTextBufferBase8008A750 + static_cast<uint32_t>(cursor);
    record.glyphPacketCursor20 =
        kTextRecordGlyphBufferBase8008AB50 +
        static_cast<uint32_t>(cursor) * kTextGlyphPacketStride800436F0;
    candidate.textBytes[static_cast<uint32_t>(cursor)] = 0u;

    for (int32_t index = 0; index < effectiveCapacity; ++index) {
        InitFntOpenGlyphPacket800441C0(
            candidate.glyphPackets[static_cast<uint32_t>(cursor + index)],
            candidate.fontClut8008EB54);
    }

    const int32_t newCursor = AddMipsS32(cursor, effectiveCapacity);
    candidate.allocationCursor8005D6E4 = newCursor;
    candidate.allocationCursorKnown = true;
    candidate.recordCount8005CCDC = slot + 1;
    candidate.recordCountKnown = true;
    RefreshAggregateGlobalsKnown800436F0(candidate);
    state = candidate;

    out.status = EventTextFntOpenStatus80043438::Executed;
    out.accepted = true;
    out.returnedSlot = slot;
    out.oldCursor = cursor;
    out.effectiveCapacity = effectiveCapacity;
    out.newCursor = newCursor;
    out.initializedPacketCount = static_cast<uint32_t>(effectiveCapacity);
    out.recordAddress =
        kTextRecordBase8005CB5C +
        static_cast<uint32_t>(slot) * kTextRecordStride800436F0;
    out.textAddress = record.textPointer24;
    out.packetAddress = record.glyphPacketCursor20;
    return true;
}

bool ExecuteSetDumpFntSoftware80043354(
    EventTextRuntimeState800436F0& state,
    int32_t slot,
    EventTextSetDumpFntResult80043354& out)
{
    out = EventTextSetDumpFntResult80043354{};
    out.requestedSlot = slot;
    if (slot < 0) {
        out.accepted = true;
        return true;
    }
    if (!RecordCountKnown8005CCDC(state)) {
        return false;
    }
    out.sourceKnown = true;
    out.accepted = true;
    if (slot > state.recordCount8005CCDC) {
        return true;
    }
    state.currentRecord8005CCE0 = slot;
    state.currentRecordKnown = true;
    state.appendCallback8005D730 = kFn80043A14;
    state.appendCallbackKnown = true;
    RefreshAggregateGlobalsKnown800436F0(state);
    out.updated = true;
    return true;
}

bool ExecuteLoadImageSoftware80044D64(
    EventTextRuntimeState800436F0& state,
    const EventTextLoadImageRequest80044D64& request,
    EventTextLoadImageResult80044D64& out)
{
    out = EventTextLoadImageResult80044D64{};
    if (request.pixelBytes == nullptr || request.pixelByteCount == 0u ||
        !request.pixelAddressKnown) {
        out.status = EventTextLoadImageStatus80044D64::PixelBindingLimit;
        return false;
    }
    if (request.pixelByteCount >
        static_cast<size_t>((std::numeric_limits<uint32_t>::max)())) {
        out.status = EventTextLoadImageStatus80044D64::PixelBindingLimit;
        return false;
    }

    // 80044D64 first invokes 80044BA8 for the named LoadImage RECT check,
    // then dispatches 800462C4 through 800468E0 with width 8 and the source
    // pointer. The debug RECT logger may report malformed dimensions without
    // changing the subsequent dispatch, so the warning is diagnostic only.
    EventTextRuntimeState800436F0 candidate = state;
    candidate.loadImageTransactionKnown = true;
    state = candidate;

    out.status = EventTextLoadImageStatus80044D64::Executed;
    out.accepted = true;
    out.rectValidationWarning = request.rect.w <= 0 || request.rect.h <= 0;
    out.dispatchWidth = 8u;
    out.rect = request.rect;
    out.pixelAddress = request.pixelAddress;
    out.pixelByteCount = static_cast<uint32_t>(request.pixelByteCount);
    return true;
}

bool ExecuteTextDispatchSoftware800468E0(
    const EventTextDispatchRequest800468E0& request,
    EventTextDispatchResult800468E0& out)
{
    out = EventTextDispatchResult800468E0{};
    out.callbackFunction = request.callbackFunction;
    out.headAddress = request.headAddress;
    out.callbackArgument = request.callbackArgument;
    out.copyByteCount = request.copyByteCount;
    if (request.callbackFunction == 0u) {
        out.status = EventTextDispatchStatus800468E0::CallbackBindingLimit;
        return false;
    }
    if (request.copyByteCount >
        kEventTextDispatchQueuePayloadByteLimit800468E0) {
        out.status = EventTextDispatchStatus800468E0::QueuePayloadSizeLimit;
        return false;
    }

    // 800468E0 rounds the positive width up to whole words before copying the
    // next 0x60-byte ring record at +0x0C. Keep that copy explicit; no guessed
    // PSX pointer is dereferenced by this host model.
    const uint32_t copyWords =
        (request.copyByteCount + (sizeof(uint32_t) - 1u)) / sizeof(uint32_t);
    const size_t copiedBytes =
        static_cast<size_t>(copyWords) * sizeof(uint32_t);
    if (copyWords != 0u &&
        (request.headBytes == nullptr || request.headByteCount < copiedBytes)) {
        out.status = EventTextDispatchStatus800468E0::QueuePayloadBindingLimit;
        return false;
    }
    if (copiedBytes != 0u) {
        std::memcpy(out.copiedHeadBytes.data(), request.headBytes, copiedBytes);
    }
    out.copyWordCount = copyWords;
    out.queueShapeKnown = true;
    out.immediateCallbackArgumentsKnown =
        request.headAddressKnown && request.callbackArgumentKnown;
    out.status = EventTextDispatchStatus800468E0::Executed;
    out.accepted = true;
    return true;
}

namespace {

uint32_t DispatchQueueCount80046BC4(
    const EventTextDispatchQueueState800468E0& state)
{
    return (state.producerIndex8005D838 - state.consumerIndex8005D83C) &
           (kEventTextDispatchQueueCapacity800468E0 - 1u);
}

uint32_t DispatchQueuePayloadAddress80046BC4(uint32_t slot)
{
    return kEventTextDispatchQueuePayloadBase800468E0 +
           slot * kEventTextDispatchQueueStride800468E0;
}

} // namespace

bool ExecuteTextDispatchConsumerSoftware80046BC4(
    EventTextDispatchQueueState800468E0& state,
    EventTextDispatchCallbackSink80046BC4 callbackSink,
    void* callbackSinkUserData,
    EventTextDispatchQueueResult80046BC4& out)
{
    out = EventTextDispatchQueueResult80046BC4{};
    out.producerBefore = state.producerIndex8005D838;
    out.consumerBefore = state.consumerIndex8005D83C;
    out.producerAfter = state.producerIndex8005D838;
    out.consumerAfter = state.consumerIndex8005D83C;
    out.remainingCount = DispatchQueueCount80046BC4(state);

    // 80046BC4 returns while DMA2 is active. The host model must receive this
    // hardware state explicitly; it never polls a guessed Windows register.
    if (state.dmaActive) {
        out.status = EventTextDispatchQueueStatus80046BC4::DmaBusy;
        return false;
    }
    if (!state.gpuReady) {
        out.status = EventTextDispatchQueueStatus80046BC4::GpuNotReady;
        return false;
    }

    while (state.consumerIndex8005D83C != state.producerIndex8005D838) {
        const uint32_t slot = state.consumerIndex8005D83C;
        EventTextDispatchQueueEntry800468E0& entry = state.entries[slot];
        if (!entry.valid || entry.callbackFunction == 0u) {
            out.status = EventTextDispatchQueueStatus80046BC4::CallbackBindingLimit;
            return false;
        }
        if (callbackSink == nullptr) {
            out.status =
                EventTextDispatchQueueStatus80046BC4::CallbackSinkBindingLimit;
            return false;
        }

        EventTextDispatchCallbackInvocation80046BC4 invocation{};
        invocation.callbackFunction = entry.callbackFunction;
        invocation.headAddress = entry.payloadAddress;
        invocation.callbackArgument = entry.callbackArgument;
        invocation.payloadAddress = entry.payloadAddress;
        invocation.payloadByteCount = entry.copyByteCount;
        invocation.payloadBytes = entry.payload.data();
        out.callbackFunction = invocation.callbackFunction;
        out.headAddress = invocation.headAddress;
        out.callbackArgument = invocation.callbackArgument;
        out.payloadAddress = invocation.payloadAddress;
        out.payloadByteCount = invocation.payloadByteCount;
        out.callbackInvoked = true;
        if (!callbackSink(invocation, callbackSinkUserData)) {
            out.callbackFailed = true;
        }

        // The original advances the consumer after the callback returns.
        entry.valid = false;
        state.consumerIndex8005D83C =
            (state.consumerIndex8005D83C + 1u) &
            (kEventTextDispatchQueueCapacity800468E0 - 1u);
        ++out.processedCount;
    }

    // The optional completion callback is one-shot and runs only after the
    // queue is empty and DMA2 is idle. Clear the pending bit before the call.
    if (state.completionPending && state.completionCallbackFunction != 0u) {
        if (callbackSink == nullptr) {
            out.status =
                EventTextDispatchQueueStatus80046BC4::CallbackSinkBindingLimit;
            return false;
        }
        EventTextDispatchCallbackInvocation80046BC4 invocation{};
        invocation.callbackFunction = state.completionCallbackFunction;
        invocation.callbackArgument = state.completionCallbackArgument;
        state.completionPending = false;
        out.completionCallbackInvoked = true;
        if (!callbackSink(invocation, callbackSinkUserData)) {
            out.callbackFailed = true;
        }
    }

    out.producerAfter = state.producerIndex8005D838;
    out.consumerAfter = state.consumerIndex8005D83C;
    out.remainingCount = DispatchQueueCount80046BC4(state);
    out.status = EventTextDispatchQueueStatus80046BC4::Executed;
    out.accepted = true;
    return true;
}

bool ExecuteTextDispatchQueueSoftware800468E0(
    const EventTextDispatchQueueRequest800468E0& request,
    EventTextDispatchQueueState800468E0& state,
    EventTextDispatchQueueResult80046BC4& out)
{
    out = EventTextDispatchQueueResult80046BC4{};
    out.producerBefore = state.producerIndex8005D838;
    out.consumerBefore = state.consumerIndex8005D83C;
    out.producerAfter = state.producerIndex8005D838;
    out.consumerAfter = state.consumerIndex8005D83C;
    out.remainingCount = DispatchQueueCount80046BC4(state);

    const EventTextDispatchRequest800468E0& dispatch = request.dispatch;
    if (dispatch.callbackFunction == 0u) {
        out.status = EventTextDispatchQueueStatus80046BC4::CallbackBindingLimit;
        return false;
    }
    if (dispatch.copyByteCount >
        kEventTextDispatchQueuePayloadByteLimit800468E0) {
        out.status = EventTextDispatchQueueStatus80046BC4::PayloadSizeLimit;
        return false;
    }
    const uint32_t copyWords =
        (dispatch.copyByteCount + (sizeof(uint32_t) - 1u)) /
        sizeof(uint32_t);
    const size_t copiedBytes = static_cast<size_t>(copyWords) * sizeof(uint32_t);
    if (copiedBytes != 0u &&
        (dispatch.headBytes == nullptr ||
         dispatch.headByteCount < copiedBytes)) {
        out.status = EventTextDispatchQueueStatus80046BC4::PayloadBindingLimit;
        return false;
    }

    const bool immediate =
        !request.forceQueue &&
        state.producerIndex8005D838 == state.consumerIndex8005D83C &&
        !state.dmaActive && state.gpuReady && !state.completionPending;
    if (immediate) {
        if (request.callbackSink == nullptr) {
            out.status =
                EventTextDispatchQueueStatus80046BC4::CallbackSinkBindingLimit;
            return false;
        }
        EventTextDispatchCallbackInvocation80046BC4 invocation{};
        invocation.callbackFunction = dispatch.callbackFunction;
        invocation.headAddress = dispatch.headAddress;
        invocation.callbackArgument = dispatch.callbackArgument;
        invocation.payloadAddress = dispatch.headAddress;
        invocation.payloadByteCount = static_cast<uint32_t>(copiedBytes);
        invocation.payloadBytes = dispatch.headBytes;
        out.immediatePath = true;
        out.callbackInvoked = true;
        out.callbackFunction = invocation.callbackFunction;
        out.headAddress = invocation.headAddress;
        out.callbackArgument = invocation.callbackArgument;
        out.payloadAddress = invocation.payloadAddress;
        out.payloadByteCount = invocation.payloadByteCount;
        if (!request.callbackSink(invocation, request.callbackSinkUserData)) {
            out.callbackFailed = true;
        }
        out.status = EventTextDispatchQueueStatus80046BC4::Executed;
        out.accepted = true;
        return true;
    }

    const uint32_t nextProducer =
        (state.producerIndex8005D838 + 1u) &
        (kEventTextDispatchQueueCapacity800468E0 - 1u);
    if (nextProducer == state.consumerIndex8005D83C) {
        out.status = EventTextDispatchQueueStatus80046BC4::QueueFull;
        out.dmaCallbackRequested = !state.completionPending;
        return false;
    }

    const uint32_t slot = state.producerIndex8005D838;
    EventTextDispatchQueueEntry800468E0 candidate{};
    candidate.valid = true;
    candidate.callbackFunction = dispatch.callbackFunction;
    candidate.payloadAddress = DispatchQueuePayloadAddress80046BC4(slot);
    candidate.callbackArgument = dispatch.callbackArgument;
    candidate.copyByteCount = static_cast<uint32_t>(copiedBytes);
    if (copiedBytes != 0u) {
        std::memcpy(candidate.payload.data(), dispatch.headBytes, copiedBytes);
    }
    state.entries[slot] = candidate;
    state.producerIndex8005D838 = nextProducer;
    state.dmaCallbackRegistered = true;
    out.callbackFunction = candidate.callbackFunction;
    out.headAddress = candidate.payloadAddress;
    out.callbackArgument = candidate.callbackArgument;
    out.payloadAddress = candidate.payloadAddress;
    out.payloadByteCount = candidate.copyByteCount;
    out.accepted = true;

    // The original calls 80046BC4 after publishing the slot. With no host
    // sink, retain the slot for a later explicit consumer call.
    if (request.callbackSink != nullptr) {
        EventTextDispatchQueueResult80046BC4 consumed{};
        ExecuteTextDispatchConsumerSoftware80046BC4(
            state, request.callbackSink, request.callbackSinkUserData, consumed);
        out.callbackInvoked = consumed.callbackInvoked;
        out.callbackFailed = consumed.callbackFailed;
        out.processedCount = consumed.processedCount;
        out.completionCallbackInvoked = consumed.completionCallbackInvoked;
        out.consumerAfter = consumed.consumerAfter;
        out.remainingCount = consumed.remainingCount;
    }

    out.producerAfter = state.producerIndex8005D838;
    out.consumerAfter = state.consumerIndex8005D83C;
    out.remainingCount = DispatchQueueCount80046BC4(state);
    out.status = EventTextDispatchQueueStatus80046BC4::Executed;
    return true;
}

bool ExecuteGpuUploadPrepareSoftware80047144(
    uint32_t currentTick,
    EventTextGpuWaitState80047144& state,
    EventTextGpuWaitResult80047178& out)
{
    out = EventTextGpuWaitResult80047178{};
    EventTextGpuWaitState80047144 candidate = state;
    candidate.prepared = true;
    candidate.deadlineTick =
        currentTick + kEventTextGpuWaitDeadlineOffset80047144;
    candidate.pollCount = 0u;
    state = candidate;

    out.status = EventTextGpuWaitStatus80047178::PollCompleted;
    out.accepted = true;
    out.currentTick = currentTick;
    out.deadlineTick = candidate.deadlineTick;
    out.pollCount = candidate.pollCount;
    return true;
}

bool ExecuteGpuUploadWaitSoftware80047178(
    uint32_t currentTick,
    EventTextGpuWaitState80047144& state,
    EventTextGpuWaitResult80047178& out)
{
    out = EventTextGpuWaitResult80047178{};
    if (!state.prepared) {
        out.status = EventTextGpuWaitStatus80047178::PreparationBindingLimit;
        return false;
    }

    EventTextGpuWaitState80047144 candidate = state;
    out.currentTick = currentTick;
    out.deadlineTick = candidate.deadlineTick;
    out.deadlineOverdue =
        static_cast<int32_t>(candidate.deadlineTick) <
        static_cast<int32_t>(currentTick);
    if (out.deadlineOverdue) {
        const uint32_t previousPollCount = candidate.pollCount;
        candidate.pollCount = previousPollCount + 1u;
        if (previousPollCount > kEventTextGpuWaitPollLimit80047178) {
            // 80047178's timeout branch resets the DMA/GPU control words and
            // returns -1. The register values are recorded as intent only.
            candidate.prepared = false;
            state = candidate;
            out.status = EventTextGpuWaitStatus80047178::TimedOut;
            out.resetIssued = true;
            out.accepted = false;
            out.pollCount = candidate.pollCount;
            return false;
        }
    }

    state = candidate;
    out.status = EventTextGpuWaitStatus80047178::PollCompleted;
    out.accepted = true;
    out.pollCount = candidate.pollCount;
    return true;
}

bool ExecuteLoadImageUploadSoftware800462C4(
    const EventTextLoadImageRequest80044D64& request,
    EventTextUploadResult800462C4& out)
{
    out = EventTextUploadResult800462C4{};
    out.rect = request.rect;
    out.effectiveRect = request.rect;

    // 800462C4 clamps negative dimensions to zero and writes the clamped
    // values back before deriving the transfer count. The positive upper
    // bounds live in IDA globals (8005D738/8005D73A); this host model does not
    // invent those values, so positive dimensions are preserved verbatim.
    if (out.effectiveRect.w < 0) {
        out.effectiveRect.w = 0;
        out.rectClamped = true;
    }
    if (out.effectiveRect.h < 0) {
        out.effectiveRect.h = 0;
        out.rectClamped = true;
    }
    if (out.effectiveRect.w == 0 || out.effectiveRect.h == 0) {
        out.status = EventTextUploadStatus800462C4::RectBindingLimit;
        return false;
    }
    if (request.pixelBytes == nullptr || request.pixelByteCount == 0u ||
        !request.pixelAddressKnown) {
        out.status = EventTextUploadStatus800462C4::PixelBindingLimit;
        return false;
    }

    const uint64_t area =
        static_cast<uint64_t>(out.effectiveRect.w) *
        static_cast<uint64_t>(out.effectiveRect.h);
    // IDA: (area + 1) / 2, with a 16-word DMA block size. The CPU writes the
    // remainder first; DMA2 starts at the source address after that prefix.
    const uint64_t transferWords = (area + 1u) / 2u;
    const uint64_t dmaBlockCount = transferWords / 16u;
    const uint64_t cpuTailWords = transferWords % 16u;
    const uint64_t requiredBytes = transferWords * sizeof(uint32_t);
    if (transferWords == 0u || requiredBytes > request.pixelByteCount ||
        transferWords > (std::numeric_limits<uint32_t>::max)() ||
        dmaBlockCount > 0xFFFFu ||
        static_cast<uint64_t>(request.pixelAddress) + cpuTailWords * 4u >
            (std::numeric_limits<uint32_t>::max)()) {
        out.status = EventTextUploadStatus800462C4::TransferSizeLimit;
        return false;
    }

    out.status = EventTextUploadStatus800462C4::Executed;
    out.accepted = true;
    out.waitCompleted = true;
    out.cpuTailWritten = cpuTailWords != 0u;
    out.transferWords = static_cast<uint32_t>(transferWords);
    out.cpuTailWords = static_cast<uint32_t>(cpuTailWords);
    out.dmaBlockCount = static_cast<uint32_t>(dmaBlockCount);
    out.gp0Command = 0xA0000000u;
    out.dma2Madr = request.pixelAddress + out.cpuTailWords * 4u;
    if (dmaBlockCount != 0u) {
        out.dmaSubmitted = true;
        out.gp1DmaDirection = 0x04000002u;
        out.dma2Bcr = (out.dmaBlockCount << 16) | 0x10u;
        out.dma2Chcr = 0x01000201u;
    }
    return true;
}

bool ExecuteTextAppendSoftware80043A14(
    EventTextRuntimeState800436F0& state,
    const EventTextAppendRequest80043A14& request,
    EventTextAppendResult80043A14& out)
{
    out = EventTextAppendResult80043A14{};
    if (!state.recordBankKnown || !RecordCountKnown8005CCDC(state) ||
        request.argCount > kTextAppendMaxArgs80043A14) {
        out.status = EventTextAppendStatus80043A14::SourceUnknown;
        return false;
    }
    if (request.format == nullptr) {
        out.status = EventTextAppendStatus80043A14::FormatBindingLimit;
        return false;
    }

    const bool explicitSlot =
        request.slot >= 0 && request.slot < state.recordCount8005CCDC;
    if (!explicitSlot && !CurrentRecordKnown8005CCE0(state)) {
        out.status = EventTextAppendStatus80043A14::SourceUnknown;
        return false;
    }
    const int32_t selectedSlot =
        explicitSlot ? request.slot : state.currentRecord8005CCE0;
    out.usedFallbackSlot = !explicitSlot;
    if (selectedSlot < 0 ||
        selectedSlot >= static_cast<int32_t>(kTextRecordCount800436F0)) {
        out.status = EventTextAppendStatus80043A14::SourceUnknown;
        return false;
    }
    out.selectedSlot = static_cast<uint32_t>(selectedSlot);

    const EventTextRecord800436F0& sourceRecord =
        state.records[out.selectedSlot];
    if (sourceRecord.textPointer24 == 0u) {
        out.status = EventTextAppendStatus80043A14::CurrentTextPointerNull;
        return false;
    }
    uint32_t textOffset = 0;
    if (!TextBankOffset800436F0(sourceRecord.textPointer24, textOffset)) {
        out.status = EventTextAppendStatus80043A14::StringBindingLimit;
        return false;
    }

    EventTextRuntimeState800436F0 candidate = state;
    EventTextRecord800436F0& record = candidate.records[out.selectedSlot];
    uint32_t cursor = record.appendCursor28;
    out.initialCursor = cursor;
    const uint32_t capacity = record.parseBudget1C < 0
                                  ? 0u
                                  : static_cast<uint32_t>(record.parseBudget1C);

    auto appendByte = [&](uint8_t byte) -> bool {
        if (cursor >= capacity ||
            cursor >= kTextBufferBankByteCount80043394 - textOffset) {
            out.status = EventTextAppendStatus80043A14::CapacityBindingLimit;
            return false;
        }
        candidate.textBytes[textOffset + cursor] = byte;
        ++cursor;
        ++out.appendedBytes;
        return true;
    };

    auto appendBytes = [&](const std::string& bytes) -> bool {
        for (const unsigned char byte : bytes) {
            if (!appendByte(byte)) {
                return false;
            }
        }
        return true;
    };

    auto formatSigned = [](int32_t value) -> std::string {
        std::array<char, 32> buffer{};
        const auto converted = std::to_chars(
            buffer.data(), buffer.data() + buffer.size(), value, 10);
        return converted.ec == std::errc{}
                   ? std::string(buffer.data(), converted.ptr)
                   : std::string{};
    };
    auto formatUnsignedHex = [](uint32_t value, bool upper) -> std::string {
        std::array<char, 32> buffer{};
        const auto converted = std::to_chars(
            buffer.data(), buffer.data() + buffer.size(), value, 16);
        if (converted.ec != std::errc{}) {
            return {};
        }
        std::string result(buffer.data(), converted.ptr);
        if (upper) {
            for (char& byte : result) {
                if (byte >= 'a' && byte <= 'f') {
                    byte = static_cast<char>(byte - 'a' + 'A');
                }
            }
        }
        return result;
    };

    uint32_t argumentIndex = 0;
    uint32_t formatIndex = 0;
    for (;;) {
        if (formatIndex >= kTextBufferBankByteCount80043394) {
            out.status = EventTextAppendStatus80043A14::FormatBindingLimit;
            return false;
        }
        const uint8_t formatByte =
            static_cast<uint8_t>(request.format[formatIndex]);
        if (formatByte == 0u) {
            break;
        }
        if (formatByte != static_cast<uint8_t>('%')) {
            if (!appendByte(formatByte)) {
                return false;
            }
            ++formatIndex;
            continue;
          }

          ++formatIndex;
          if (formatIndex >= kTextBufferBankByteCount80043394) {
              out.status = EventTextAppendStatus80043A14::FormatBindingLimit;
              return false;
          }
          const uint8_t next =
              static_cast<uint8_t>(request.format[formatIndex]);
        if (next == 0u) {
            out.status = EventTextAppendStatus80043A14::UnsupportedSpecifier;
            return false;
        }
        if (next == static_cast<uint8_t>('%')) {
            if (!appendByte(next)) {
                return false;
            }
            ++formatIndex;
            continue;
        }

        bool zeroPad = false;
        uint32_t width = 0;
        if (next == static_cast<uint8_t>('0')) {
            zeroPad = true;
        }
        while (formatIndex < kTextBufferBankByteCount80043394 &&
               request.format[formatIndex] >= '0' &&
               request.format[formatIndex] <= '9') {
            const uint32_t digit = static_cast<uint32_t>(
                request.format[formatIndex] - '0');
            if (width > 1000u) {
                out.status = EventTextAppendStatus80043A14::FormatBindingLimit;
                return false;
            }
            width = width * 10u + digit;
            ++formatIndex;
        }
        if (formatIndex >= kTextBufferBankByteCount80043394) {
            out.status = EventTextAppendStatus80043A14::FormatBindingLimit;
            return false;
        }
        const uint8_t specifier =
            static_cast<uint8_t>(request.format[formatIndex]);
        if (specifier == 0u) {
            out.status = EventTextAppendStatus80043A14::UnsupportedSpecifier;
            return false;
        }
        ++formatIndex;

        if (specifier != static_cast<uint8_t>('d') &&
            specifier != static_cast<uint8_t>('x') &&
            specifier != static_cast<uint8_t>('X') &&
            specifier != static_cast<uint8_t>('c') &&
            specifier != static_cast<uint8_t>('s')) {
            out.status = EventTextAppendStatus80043A14::UnsupportedSpecifier;
            return false;
        }
        if (argumentIndex >= request.argCount) {
            out.status = EventTextAppendStatus80043A14::MissingArgument;
            return false;
        }
        const EventTextAppendArg80043A14& argument =
            request.args[argumentIndex++];
        ++out.argumentsConsumed;

        std::string rendered;
        if (specifier == static_cast<uint8_t>('d')) {
            int32_t value = argument.signedValue;
            if (argument.kind == EventTextAppendArgKind80043A14::Unsigned32) {
                value = static_cast<int32_t>(argument.unsignedValue);
            }
            if (argument.kind != EventTextAppendArgKind80043A14::Signed32 &&
                argument.kind != EventTextAppendArgKind80043A14::Unsigned32) {
                out.status = EventTextAppendStatus80043A14::MissingArgument;
                return false;
            }
            rendered = formatSigned(value);
            if (width > rendered.size()) {
                const size_t padding = width - rendered.size();
                rendered.insert(0, padding, ' ');
            }
        } else if (specifier == static_cast<uint8_t>('x') ||
                   specifier == static_cast<uint8_t>('X')) {
            uint32_t value = argument.unsignedValue;
            if (argument.kind == EventTextAppendArgKind80043A14::Signed32) {
                value = static_cast<uint32_t>(argument.signedValue);
            }
            if (argument.kind != EventTextAppendArgKind80043A14::Signed32 &&
                argument.kind != EventTextAppendArgKind80043A14::Unsigned32) {
                out.status = EventTextAppendStatus80043A14::MissingArgument;
                return false;
            }
            rendered = formatUnsignedHex(
                value, specifier == static_cast<uint8_t>('X'));
            if (width > rendered.size()) {
                const size_t padding = width - rendered.size();
                rendered.insert(0, padding, zeroPad ? '0' : ' ');
            }
        } else if (specifier == static_cast<uint8_t>('c')) {
            uint8_t value = argument.characterValue;
            if (argument.kind == EventTextAppendArgKind80043A14::Signed32) {
                value = static_cast<uint8_t>(argument.signedValue);
            } else if (argument.kind ==
                       EventTextAppendArgKind80043A14::Unsigned32) {
                value = static_cast<uint8_t>(argument.unsignedValue);
            }
            if (argument.kind != EventTextAppendArgKind80043A14::Character &&
                argument.kind != EventTextAppendArgKind80043A14::Signed32 &&
                argument.kind != EventTextAppendArgKind80043A14::Unsigned32) {
                out.status = EventTextAppendStatus80043A14::MissingArgument;
                return false;
            }
            rendered.push_back(static_cast<char>(value));
        } else {
            if (argument.kind != EventTextAppendArgKind80043A14::String ||
                argument.stringValue == nullptr) {
                out.status = EventTextAppendStatus80043A14::StringBindingLimit;
                return false;
            }
            size_t length = 0;
            while (length < kTextBufferBankByteCount80043394 &&
                   argument.stringValue[length] != '\0') {
                ++length;
            }
            if (length >= kTextBufferBankByteCount80043394) {
                out.status = EventTextAppendStatus80043A14::StringBindingLimit;
                return false;
            }
            rendered.assign(argument.stringValue, length);
            if (width > rendered.size()) {
                rendered.insert(0, width - rendered.size(), ' ');
            }
        }
        if (!appendBytes(rendered)) {
            return false;
        }
    }

    if (!appendByte(0u)) {
        return false;
    }
    --out.appendedBytes;
    out.formatBytesConsumed = formatIndex;
    out.finalCursor = cursor - 1u;
    out.returnedCursor = static_cast<int32_t>(out.finalCursor);
    candidate.records[out.selectedSlot].appendCursor28 = out.finalCursor;
    state = candidate;
    out.status = EventTextAppendStatus80043A14::Executed;
    out.accepted = true;
    return true;
}

bool ExecuteTextSystemBoot80027FACSoftwareCore(
    EventTextRuntimeState800436F0& state,
    const EventTextFontBinding80043394& binding,
    EventTextDisplaySink80044AA0 displaySink,
    void* userData,
    EventTextBootResult80027FAC& out)
{
    out = EventTextBootResult80027FAC{};
    if (!binding.known) {
        out.status = EventTextBootStatus80027FAC::FontBindingUnknown;
        return false;
    }
    if (displaySink == nullptr) {
        out.status = EventTextBootStatus80027FAC::DisplaySinkMissing;
        return false;
    }
    if (!state.textBankKnown || !state.glyphBankKnown) {
        out.status = EventTextBootStatus80027FAC::HostPoolBackingUnknown;
        return false;
    }
    out.fontBindingInjected = true;

    EventTextRuntimeState800436F0 candidate = state;
    if (!AppendBootStep80027FAC(
            out, EventTextBootStep80027FAC::FntLoad80043394) ||
        !ExecuteFntLoadSoftware80043394(candidate, binding, out.fntLoad)) {
        out.status = EventTextBootStatus80027FAC::SoftwareStateLimit;
        return false;
    }

    EventTextFntOpenRequest80043438 request{};
    request.x = -156;
    request.y = -120;
    request.width = 320;
    request.height = 200;
    request.mode = 0;
    request.requestedCapacity = 512;
    if (!AppendBootStep80027FAC(
            out, EventTextBootStep80027FAC::FntOpen80043438) ||
        !ExecuteFntOpenSoftware80043438(candidate, request, out.fntOpen) ||
        out.fntOpen.status != EventTextFntOpenStatus80043438::Executed) {
        out.status = EventTextBootStatus80027FAC::SoftwareStateLimit;
        return false;
    }
    out.returnedSlot = out.fntOpen.returnedSlot;

    if (!AppendBootStep80027FAC(
            out, EventTextBootStep80027FAC::SetDumpFnt80043354) ||
        !ExecuteSetDumpFntSoftware80043354(
            candidate, out.returnedSlot, out.setDump) ||
        !out.setDump.updated ||
        !AppendBootStep80027FAC(
            out, EventTextBootStep80027FAC::DisplayEnable80044AA0)) {
        out.status = EventTextBootStatus80027FAC::SoftwareStateLimit;
        return false;
    }

    state = candidate;
    out.displaySinkInvoked = true;
    out.displaySinkReturn = displaySink(1, userData);
    out.fontUploadExecuted = out.fntLoad.fontUploadExecuted;
    out.status = EventTextBootStatus80027FAC::Executed;
    out.accepted = true;
    return true;
}

bool ExecuteTextFlush800436F0(
    EventTextRuntimeState800436F0& state,
    int32_t slot,
    EventTextSubmitSink800450A0 submitSink,
    void* userData,
    EventTextFlushExecutionResult800436F0& out)
{
    out = EventTextFlushExecutionResult800436F0{};
    if (!state.recordBankKnown || !RecordCountKnown8005CCDC(state)) {
        return FailTextFlush800436F0(out,
                                    EventTextFlushStatus800436F0::SourceUnknown);
    }
    const bool explicitSlot =
        slot >= 0 && slot < state.recordCount8005CCDC;
    if (!explicitSlot && !CurrentRecordKnown8005CCE0(state)) {
        return FailTextFlush800436F0(out,
                                    EventTextFlushStatus800436F0::SourceUnknown);
    }
    const int32_t selectedSlot =
        explicitSlot ? slot : state.currentRecord8005CCE0;
    out.usedFallbackSlot = !explicitSlot;
    if (selectedSlot < 0 ||
        selectedSlot >= static_cast<int32_t>(kTextRecordCount800436F0)) {
        return FailTextFlush800436F0(
            out, EventTextFlushStatus800436F0::InvalidSelectedSlot);
    }
    out.selectedSlot = static_cast<uint32_t>(selectedSlot);

    const EventTextRecord800436F0& sourceRecord =
        state.records[out.selectedSlot];
    if (out.usedFallbackSlot && sourceRecord.textPointer24 == 0u) {
        out.status = EventTextFlushStatus800436F0::CurrentTextPointerNull;
        out.accepted = true;
        return true;
    }
    if (!state.textBankKnown) {
        return FailTextFlush800436F0(out,
                                    EventTextFlushStatus800436F0::SourceUnknown);
    }
    if (submitSink == nullptr) {
        return FailTextFlush800436F0(
            out, EventTextFlushStatus800436F0::SubmitSinkMissing);
    }
    uint32_t textOffset = 0;
    if (!TextBankOffset800436F0(sourceRecord.textPointer24, textOffset)) {
        return FailTextFlush800436F0(
            out, EventTextFlushStatus800436F0::HostTextPointerBindingLimit);
    }
    EventTextRuntimeState800436F0 candidate = state;
    EventTextRecord800436F0& record = candidate.records[out.selectedSlot];
    const uint32_t recordAddress =
        kTextRecordBase8005CB5C +
        out.selectedSlot * kTextRecordStride800436F0;
    const uint32_t headAddress = recordAddress + 0x10u;
    uint32_t packetAddress = record.glyphPacketCursor20;
    int32_t remaining = record.parseBudget1C;
    int32_t x = record.x;
    int32_t y = record.y;
    const int32_t right = x + static_cast<int32_t>(record.w);
    const int32_t bottom = y + static_cast<int32_t>(record.h);
    int32_t maxX = 0;
    uint8_t red = 0x80u;
    uint8_t green = 0x80u;
    uint8_t blue = 0x80u;

    EventTextFlushSubmission800436F0 submission{};
    submission.selectedSlot = out.selectedSlot;
    submission.recordAddress = recordAddress;
    submission.headAddress = headAddress;
    InitListHead800440B8(record.listHeadWord10);

    while (candidate.textBytes[textOffset] != 0u && remaining != 0) {
        const uint8_t textByte = candidate.textBytes[textOffset];
        uint32_t byteAdvance = 1u;
        bool lineBreak = false;
        ++out.parserIterations;

        if (textByte == 0x20u) {
            x += 8;
        } else if (textByte <= 0x20u) {
            if (textByte == 0x09u) {
                x += 0x20;
            } else if (textByte == 0x0Au) {
                lineBreak = true;
            } else {
                if (!candidate.glyphBankKnown) {
                    return FailTextFlush800436F0(
                        out, EventTextFlushStatus800436F0::SourceUnknown);
                }
                uint32_t currentPacketIndex = 0;
                if (!GlyphBankIndex800436F0(packetAddress,
                                            currentPacketIndex,
                                            true)) {
                    return FailTextFlush800436F0(
                        out,
                        EventTextFlushStatus800436F0::InvalidGlyphPacketCursor);
                }
                if (currentPacketIndex >=
                    kTextGlyphPacketBankCount80043394) {
                    return FailTextFlush800436F0(
                        out,
                        EventTextFlushStatus800436F0::GlyphPacketRangeExhausted);
                }
                EventTextGlyphPacket800436F0& packet =
                    candidate.glyphPackets[currentPacketIndex];
                const int32_t glyphIndex =
                    static_cast<int32_t>(textByte) - 0x20;
                const int32_t page = glyphIndex / 16;
                const int32_t column = glyphIndex - page * 16;
                packet.u = static_cast<uint8_t>(column * 8);
                packet.v = static_cast<uint8_t>(page * 8);
                packet.x = static_cast<int16_t>(x);
                packet.y = static_cast<int16_t>(y);
                packet.r = red;
                packet.g = green;
                packet.b = blue;
                LinkNode8004401C(packetAddress,
                                 packet.word0,
                                 record.listHeadWord10);
                EventTextSubmittedGlyph800436F0& frozen =
                    submission.glyphs[submission.glyphCount++];
                frozen.psxAddress = packetAddress;
                frozen.packet = packet;
                packetAddress += kTextGlyphPacketStride800436F0;
                x += 8;
            }
        } else if (textByte == 0x7Eu) {
            if (textOffset + 1u >= kTextBufferBankByteCount80043394) {
                return FailTextFlush800436F0(
                    out, EventTextFlushStatus800436F0::TextRangeExhausted);
            }
            const uint8_t command = candidate.textBytes[textOffset + 1u];
            if (command == 0x63u) {
                if (textOffset + 4u >= kTextBufferBankByteCount80043394) {
                    return FailTextFlush800436F0(
                        out, EventTextFlushStatus800436F0::TextRangeExhausted);
                }
                red = static_cast<uint8_t>(
                    (static_cast<int32_t>(candidate.textBytes[textOffset + 2u]) -
                     0x30) *
                    16);
                green = static_cast<uint8_t>(
                    (static_cast<int32_t>(candidate.textBytes[textOffset + 3u]) -
                     0x30) *
                    16);
                blue = static_cast<uint8_t>(
                    (static_cast<int32_t>(candidate.textBytes[textOffset + 4u]) -
                     0x30) *
                    16);
                byteAdvance = 5u;
            } else {
                byteAdvance = 2u;
            }
        } else {
            if (!candidate.glyphBankKnown) {
                return FailTextFlush800436F0(
                    out, EventTextFlushStatus800436F0::SourceUnknown);
            }
            uint32_t currentPacketIndex = 0;
            if (!GlyphBankIndex800436F0(packetAddress,
                                        currentPacketIndex,
                                        true)) {
                return FailTextFlush800436F0(
                    out,
                    EventTextFlushStatus800436F0::InvalidGlyphPacketCursor);
            }
            if (currentPacketIndex >= kTextGlyphPacketBankCount80043394) {
                return FailTextFlush800436F0(
                    out,
                    EventTextFlushStatus800436F0::GlyphPacketRangeExhausted);
            }
            EventTextGlyphPacket800436F0& packet =
                candidate.glyphPackets[currentPacketIndex];
            const int32_t glyphIndex =
                textByte >= 0x61u && textByte <= 0x7Au
                    ? static_cast<int32_t>(textByte) - 0x40
                    : static_cast<int32_t>(textByte) - 0x20;
            const int32_t page = glyphIndex / 16;
            const int32_t column = glyphIndex - page * 16;
            packet.u = static_cast<uint8_t>(column * 8);
            packet.v = static_cast<uint8_t>(page * 8);
            packet.x = static_cast<int16_t>(x);
            packet.y = static_cast<int16_t>(y);
            packet.r = red;
            packet.g = green;
            packet.b = blue;
            LinkNode8004401C(packetAddress,
                             packet.word0,
                             record.listHeadWord10);
            EventTextSubmittedGlyph800436F0& frozen =
                submission.glyphs[submission.glyphCount++];
            frozen.psxAddress = packetAddress;
            frozen.packet = packet;
            packetAddress += kTextGlyphPacketStride800436F0;
            x += 8;
        }

        if (!lineBreak && textByte != 0x7Eu && x >= right &&
            record.measureMode2C == 0) {
            lineBreak = true;
        }
        if (lineBreak) {
            if (maxX < x) {
                maxX = x;
            }
            y += 8;
            x = record.x;
            if (y >= bottom) {
                out.stoppedAtBottom = true;
                break;
            }
        }

        if (byteAdvance >= kTextBufferBankByteCount80043394 - textOffset) {
            return FailTextFlush800436F0(
                out, EventTextFlushStatus800436F0::TextRangeExhausted);
        }
        textOffset += byteAdvance;
        remaining = static_cast<int32_t>(
            static_cast<uint32_t>(remaining) - 1u);
    }

    if (record.control07 != 0u) {
        LinkNode8004401C(recordAddress, record.word0, record.listHeadWord10);
        submission.recordLinked = true;
    }
    if (record.measureMode2C != 0) {
        const uint32_t width =
            static_cast<uint32_t>(maxX) - static_cast<uint16_t>(record.x);
        const uint32_t height =
            static_cast<uint32_t>(y) -
            (static_cast<uint32_t>(static_cast<uint16_t>(record.y)) - 8u);
        record.w = static_cast<int16_t>(width & 0xFFFFu);
        record.h = static_cast<int16_t>(height & 0xFFFFu);
    }

    submission.valid = true;
    submission.headWord = record.listHeadWord10;
    submission.recordBeforeCleanup = record;
    out.finalTextAddress =
        kTextRecordTextBufferBase8008A750 + textOffset;
    out.finalPacketAddress = packetAddress;
    out.localBudgetRemaining = remaining;
    out.finalX = x;
    out.finalY = y;
    out.maxX = maxX;
    out.submission = submission;
    state = candidate;
    out.submitInvoked = true;
    out.submitResultIgnored = submitSink(submission, userData);

    EventTextRecord800436F0& committedRecord = state.records[out.selectedSlot];
    const uint32_t postSubmitTextPointer = committedRecord.textPointer24;
    committedRecord.appendCursor28 = 0u;
    uint32_t postSubmitTextOffset = 0;
    if (!TextBankOffset800436F0(postSubmitTextPointer,
                                postSubmitTextOffset)) {
        return FailTextFlush800436F0(
            out, EventTextFlushStatus800436F0::HostTextPointerBindingLimit);
    }
    state.textBytes[postSubmitTextOffset] = 0u;

    out.status = EventTextFlushStatus800436F0::Executed;
    out.accepted = true;
    out.returnedHeadAddress = headAddress;
    return true;
}

bool ExecuteMenuHelpSoftware80026314(
    EventTextRuntimeState800436F0& state,
    const EventTextMenuHelpContext80026314& context,
    EventTextSubmitSink800450A0 submitSink,
    void* userData,
    EventTextMenuHelpResult80026314& out)
{
    out = EventTextMenuHelpResult80026314{};
    if (!context.sourceKnown ||
        (context.groupCount != 0u && context.groups == nullptr)) {
        out.status = EventTextMenuHelpStatus80026314::SourceUnknown;
        return false;
    }
    if (context.groupCount > kMenuHelpMaxGroups80026314) {
        out.status = EventTextMenuHelpStatus80026314::InvalidGroupCount;
        return false;
    }
    if (context.groupCount == 0u) {
        if (context.selectedGroup != -1) {
            out.status = EventTextMenuHelpStatus80026314::InvalidSelectedGroup;
            return false;
        }
    } else if (context.selectedGroup < 0 ||
               context.selectedGroup >=
                   static_cast<int32_t>(context.groupCount)) {
        out.status = EventTextMenuHelpStatus80026314::InvalidSelectedGroup;
        return false;
    }

    EventTextRuntimeState800436F0 candidate = state;
    auto appendLiteral = [&](const char* literal) -> bool {
        EventTextAppendRequest80043A14 request{};
        request.slot = -1;
        request.format = literal;
        EventTextAppendResult80043A14 appendResult{};
        if (!ExecuteTextAppendSoftware80043A14(
                candidate, request, appendResult)) {
            out.status = EventTextMenuHelpStatus80026314::AppendFailed;
            return false;
        }
        out.appendedBytes += appendResult.appendedBytes;
        return true;
    };
    auto appendTitle = [&](const char* title) -> bool {
        EventTextAppendRequest80043A14 request{};
        request.slot = -1;
        request.format = "%s:";
        request.argCount = 1u;
        request.args[0].kind = EventTextAppendArgKind80043A14::String;
        request.args[0].stringValue = title;
        EventTextAppendResult80043A14 appendResult{};
        if (!ExecuteTextAppendSoftware80043A14(
                candidate, request, appendResult)) {
            out.status = EventTextMenuHelpStatus80026314::AppendFailed;
            return false;
        }
        out.appendedBytes += appendResult.appendedBytes;
        return true;
    };

    // 80026314 emits the leading spacer before iterating its group bank.
    if (!appendLiteral("\n\n\n\n")) {
        return false;
    }
    for (uint32_t groupIndex = 0; groupIndex < context.groupCount;
         ++groupIndex) {
        const EventTextMenuHelpGroup80026314& group =
            context.groups[groupIndex];
        if (group.title == nullptr) {
            out.status = EventTextMenuHelpStatus80026314::InvalidTitle;
            return false;
        }
        if (group.itemCount > kMenuHelpMaxItemsPerGroup80026314) {
            out.status = EventTextMenuHelpStatus80026314::InvalidItemCount;
            return false;
        }

        if (!appendLiteral(groupIndex ==
                                   static_cast<uint32_t>(context.selectedGroup)
                               ? "~c888 >>>"
                               : "~c000    ") ||
            !appendTitle(group.title)) {
            return false;
        }

        for (uint32_t itemIndex = 0; itemIndex < group.itemCount;
             ++itemIndex) {
            const char* label = group.itemLabels[itemIndex];
            if (label == nullptr) {
                out.status = EventTextMenuHelpStatus80026314::InvalidItemLabel;
                return false;
            }
            size_t labelLength = 0;
            while (labelLength < kTextBufferBankByteCount80043394 &&
                   label[labelLength] != '\0') {
                ++labelLength;
            }
            if (labelLength >= kTextBufferBankByteCount80043394) {
                out.status = EventTextMenuHelpStatus80026314::InvalidItemLabel;
                return false;
            }
            std::string stackText =
                itemIndex == static_cast<uint32_t>(group.selectedItem)
                    ? "~c888*"
                    : "~c000 ";
            stackText.append(label, labelLength);
            if (labelLength < 6u) {
                stackText.append(6u - labelLength, ' ');
            }
            if (stackText.size() + 1u >
                kMenuHelpStackTextCapacity80026314) {
                out.status = EventTextMenuHelpStatus80026314::StackBufferLimit;
                return false;
            }
            if (!appendLiteral(stackText.c_str())) {
                return false;
            }
            ++out.itemsProcessed;
        }
        if (!appendLiteral("\n")) {
            return false;
        }
        ++out.groupsProcessed;
    }

    if (!appendLiteral("\n\n~c222      O: OK   X: CANCEL~c888\n")) {
        return false;
    }

    out.flushInvoked = true;
    if (!ExecuteTextFlush800436F0(
            candidate, -1, submitSink, userData, out.flush)) {
        out.status = EventTextMenuHelpStatus80026314::FlushFailed;
        out.flushAccepted = out.flush.accepted;
        return false;
    }
    out.flushAccepted = out.flush.accepted;
    state = candidate;
    out.status = EventTextMenuHelpStatus80026314::Executed;
    out.accepted = true;
    return true;
}

bool ExecuteStageClearSoftware80026B94(
    EventTextRuntimeState800436F0& state,
    const EventTextStageClearContext80026B94& context,
    EventTextSubmitSink800450A0 submitSink,
    void* userData,
    EventTextStageClearResult80026B94& out)
{
    out = EventTextStageClearResult80026B94{};
    if (!context.sourceKnown) {
        out.status = EventTextStageClearStatus80026B94::SourceUnknown;
        return false;
    }
    if (!context.gateKnown) {
        out.status = EventTextStageClearStatus80026B94::GateUnknown;
        return false;
    }
    if (!context.stageClearEnabled) {
        out.status = EventTextStageClearStatus80026B94::GateDisabled;
        return false;
    }

    EventTextRuntimeState800436F0 candidate = state;
    auto appendLiteral = [&](const char* literal) -> bool {
        EventTextAppendRequest80043A14 request{};
        request.slot = -1;
        request.format = literal;
        EventTextAppendResult80043A14 appendResult{};
        if (!ExecuteTextAppendSoftware80043A14(
                candidate, request, appendResult)) {
            out.status = EventTextStageClearStatus80026B94::AppendFailed;
            return false;
        }
        out.appendedBytes += appendResult.appendedBytes;
        return true;
    };

    // 80026D8C emits the prefix, then 80026DA8/80026DAC pass six lbu values
    // through the 8006EC14 "%d" formatter before the tail flush.
    if (!appendLiteral("\n\n\n~c000StageClear: ")) {
        return false;
    }
    for (uint32_t index = 0; index < kStageClearStatusAppendCount80026DAC;
         ++index) {
        EventTextAppendRequest80043A14 request{};
        request.slot = -1;
        request.format = "%d";
        request.argCount = 1u;
        request.args[0].kind = EventTextAppendArgKind80043A14::Unsigned32;
        request.args[0].unsignedValue = context.statusBytes[index];
        EventTextAppendResult80043A14 appendResult{};
        if (!ExecuteTextAppendSoftware80043A14(
                candidate, request, appendResult)) {
            out.status = EventTextStageClearStatus80026B94::AppendFailed;
            return false;
        }
        out.appendedBytes += appendResult.appendedBytes;
        ++out.statusesAppended;
    }

    out.flushInvoked = true;
    if (!ExecuteTextFlush800436F0(
            candidate, -1, submitSink, userData, out.flush)) {
        out.status = EventTextStageClearStatus80026B94::FlushFailed;
        out.flushAccepted = out.flush.accepted;
        return false;
    }
    out.flushAccepted = out.flush.accepted;
    state = candidate;
    out.status = EventTextStageClearStatus80026B94::Executed;
    out.accepted = true;
    return true;
}

bool ReplayFrozenTextFlushSubmission800450A0(
    const EventTextFlushSubmission800436F0& submission,
    EventTextSubmitSink800450A0 submitSink,
    void* userData,
    int32_t& submitResultIgnored)
{
    submitResultIgnored = 0;
    if (!submission.valid || submitSink == nullptr) {
        return false;
    }
    submitResultIgnored = submitSink(submission, userData);
    return true;
}

bool ExecuteTextSubmitSoftware800450A0(
    const EventTextFlushSubmission800436F0& submission,
    EventTextSubmitContext800450A0& context,
    EventTextSubmitExecutionResult800450A0& out)
{
    out = EventTextSubmitExecutionResult800450A0{};
    out.hostRuntimeConsumerWired = context.hostRuntimeConsumerWired;
    if (!submission.valid) {
        out.status = EventTextSubmitStatus800450A0::InvalidSubmission;
        context.last = out;
        return false;
    }
    if (submission.headAddress == 0u ||
        submission.recordAddress > UINT32_MAX - 0x10u ||
        submission.headAddress != submission.recordAddress + 0x10u) {
        out.status = EventTextSubmitStatus800450A0::InvalidHeadAddress;
        context.last = out;
        return false;
    }

    EventTextDispatchRequest800468E0 dispatchRequest{};
    dispatchRequest.callbackFunction = kFn80046840;
    dispatchRequest.headAddress = submission.headAddress;
    dispatchRequest.headAddressKnown = true;
    dispatchRequest.callbackArgumentKnown = true;
    if (!ExecuteTextDispatchSoftware800468E0(dispatchRequest,
                                              out.dispatch)) {
        out.status = EventTextSubmitStatus800450A0::HostGpuBoundary;
        context.last = out;
        return false;
    }

    // 800450A0 dispatches 80046840 through 800468E0.  80046840 writes
    // exactly these PSX GPU/DMA values; the Windows port records the atomic
    // transaction until an independent host renderer is available.
    out.transaction.valid = true;
    out.transaction.callbackFunction = kFn80046840;
    out.transaction.otagHeadAddress = submission.headAddress;
    out.transaction.gp1DmaDirection = 0x04000002u;
    out.transaction.dma2Madr = submission.headAddress;
    out.transaction.dma2Bcr = 0u;
    out.transaction.dma2Chcr = 0x01000401u;
    out.callbackDispatched = true;
    out.accepted = true;
    out.exactPsxHalParity = false;
    out.hostGlyphCount = submission.glyphCount;
    if (context.hostGlyphSink != nullptr) {
        for (uint32_t index = 0; index < submission.glyphCount; ++index) {
            const EventTextSubmittedGlyph800436F0& submitted =
                submission.glyphs[index];
            const EventTextGlyphPacket800436F0& packet = submitted.packet;
            EventTextHostGlyphDraw800450A0 draw{};
            draw.x = packet.x;
            draw.y = packet.y;
            draw.u = packet.u;
            draw.v = packet.v;
            draw.r = packet.r;
            draw.g = packet.g;
            draw.b = packet.b;
            draw.clut = packet.clut;
            draw.psxAddress = submitted.psxAddress;
            if (!context.hostGlyphSink(draw,
                                       context.hostGlyphSinkUserData)) {
                out.hostGlyphSubmitFailed = true;
                out.status = EventTextSubmitStatus800450A0::HostGpuBoundary;
                context.last = out;
                return true;
            }
        }
        out.hostGlyphsSubmitted = true;
    }
    out.status = context.hostGpuBoundaryKnown && context.hostRuntimeConsumerWired
                     ? EventTextSubmitStatus800450A0::Executed
                     : EventTextSubmitStatus800450A0::HostGpuBoundary;
    context.last = out;
    return true;
}

int32_t SubmitTextOtagSoftware800450A0(
    const EventTextFlushSubmission800436F0& submission,
    void* userData)
{
    if (userData == nullptr) {
        return -1;
    }
    auto* context = static_cast<EventTextSubmitContext800450A0*>(userData);
    EventTextSubmitExecutionResult800450A0 result{};
    return ExecuteTextSubmitSoftware800450A0(submission, *context, result)
               ? 0
               : -1;
}

const char* EventTextEndpointKindName(EventTextEndpointKind kind)
{
    switch (kind) {
    case EventTextEndpointKind::Unknown:
        return "Unknown";
    case EventTextEndpointKind::TextSystemBoot:
        return "TextSystemBoot";
    case EventTextEndpointKind::FontRecord:
        return "FontRecord";
    case EventTextEndpointKind::AppendFormat:
        return "AppendFormat";
    case EventTextEndpointKind::Flush:
        return "Flush";
    case EventTextEndpointKind::Producer:
        return "Producer";
    case EventTextEndpointKind::Helper:
        return "Helper";
    }
    return "Unknown";
}

const char* EventTextProducerKindName(EventTextProducerKind producer)
{
    switch (producer) {
    case EventTextProducerKind::Unknown:
        return "Unknown";
    case EventTextProducerKind::TextSystemBoot80027FAC:
        return "TextSystemBoot80027FAC";
    case EventTextProducerKind::TextRecordBank:
        return "TextRecordBank";
    case EventTextProducerKind::TextAppendFormatter80043A14:
        return "TextAppendFormatter80043A14";
    case EventTextProducerKind::MenuHelp80026314:
        return "MenuHelp80026314";
    case EventTextProducerKind::StageClear80026B94:
        return "StageClear80026B94";
    case EventTextProducerKind::TextFlush800436F0:
        return "TextFlush800436F0";
    }
    return "Unknown";
}

const char* EventTextActionKindName(EventTextActionKind kind)
{
    switch (kind) {
    case EventTextActionKind::None:
        return "None";
    case EventTextActionKind::CallFntLoad80043394:
        return "CallFntLoad80043394";
    case EventTextActionKind::ClearRecordBank80043394:
        return "ClearRecordBank80043394";
    case EventTextActionKind::AllocRecord80043438:
        return "AllocRecord80043438";
    case EventTextActionKind::InitGlyphPackets800441C0:
        return "InitGlyphPackets800441C0";
    case EventTextActionKind::SelectDumpRecord80043354:
        return "SelectDumpRecord80043354";
    case EventTextActionKind::RegisterAppendCallback80043354:
        return "RegisterAppendCallback80043354";
    case EventTextActionKind::EnableTextDisplay80044AA0:
        return "EnableTextDisplay80044AA0";
    case EventTextActionKind::GateAppendRecord80043A14:
        return "GateAppendRecord80043A14";
    case EventTextActionKind::AppendLiteral80043A14:
        return "AppendLiteral80043A14";
    case EventTextActionKind::AppendFormat80043A14:
        return "AppendFormat80043A14";
    case EventTextActionKind::LoadHexDigitTable80043A14:
        return "LoadHexDigitTable80043A14";
    case EventTextActionKind::GateMenuHelpContext80026314:
        return "GateMenuHelpContext80026314";
    case EventTextActionKind::BuildMenuItemStackBuffer:
        return "BuildMenuItemStackBuffer";
    case EventTextActionKind::AppendMenuItemStack80043A14:
        return "AppendMenuItemStack80043A14";
    case EventTextActionKind::StageClearGate:
        return "StageClearGate";
    case EventTextActionKind::GateStageClearTextSource80026B94:
        return "GateStageClearTextSource80026B94";
    case EventTextActionKind::AppendStageClearStatus80043A14:
        return "AppendStageClearStatus80043A14";
    case EventTextActionKind::FlushText800436F0:
        return "FlushText800436F0";
    case EventTextActionKind::InitListHead800440B8:
        return "InitListHead800440B8";
    case EventTextActionKind::ParseTextByteStream:
        return "ParseTextByteStream";
    case EventTextActionKind::LinkGlyphPacket8004401C:
        return "LinkGlyphPacket8004401C";
    case EventTextActionKind::LinkRecord8004401C:
        return "LinkRecord8004401C";
    case EventTextActionKind::SubmitTextOtag800450A0:
        return "SubmitTextOtag800450A0";
    case EventTextActionKind::ClearTextAfterFlush:
        return "ClearTextAfterFlush";
    case EventTextActionKind::Gap:
        return "Gap";
    }
    return "Unknown";
}

} // namespace PrSS0EventTextDirect
