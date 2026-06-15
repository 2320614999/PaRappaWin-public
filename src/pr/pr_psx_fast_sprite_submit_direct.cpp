#include "pr_psx_fast_sprite_submit_direct.h"

namespace PrPsxFastSpriteSubmitDirect {
namespace {

constexpr const char* kOtSlotExpression8003FA20 =
    "4 * priority + ot.headAddr_04 - 4 * ot.length_08";
constexpr const char* kLocalSpriteRgbStaticOwnerGapReason8003FA20 =
    "8003FA20 reads local+0x14..0x16 as RGB for packet word2, but the "
    "current direct carrier has no static writer/owner for those bytes";

uint16_t WrapAdd16(int32_t lhs, int32_t rhs) {
    return static_cast<uint16_t>(
        static_cast<uint32_t>(lhs) + static_cast<uint32_t>(rhs));
}

uint32_t MakeDrawModeWord8003FA20(uint32_t attr, uint16_t tpage) {
    return (static_cast<uint32_t>(tpage) & 0x1Fu) |
           ((attr >> 17) & 0x180u) |
           0xE1000200u |
           ((attr >> 23) & 0x60u);
}

uint32_t MakeColorCodeWord8003FA20(uint32_t attr,
                                   uint8_t r,
                                   uint8_t g,
                                   uint8_t b) {
    return ((attr >> 5) & 0x02000000u) |
           ((attr << 18) & 0x01000000u) |
           0x64000000u |
           (static_cast<uint32_t>(b) << 16) |
           (static_cast<uint32_t>(g) << 8) |
           static_cast<uint32_t>(r);
}

uint32_t MakeXyWord8003FA20(const FastSpriteLocalFields8003FA20& sprite,
                            const DrawOffsetGlobals8003FA20& drawOffsets) {
    const uint16_t x =
        WrapAdd16(sprite.x_04, drawOffsets.word_800917AA);
    const uint16_t y =
        WrapAdd16(sprite.y_06, drawOffsets.word_800917AC);
    return static_cast<uint32_t>(x) | (static_cast<uint32_t>(y) << 16);
}

uint32_t MakeUvClutWord8003FA20(const FastSpriteLocalFields8003FA20& sprite) {
    return static_cast<uint32_t>(sprite.u_0E) |
           (static_cast<uint32_t>(sprite.v_0F) << 8) |
           (static_cast<uint32_t>(static_cast<uint16_t>(sprite.clutY_12))
            << 22) |
           ((static_cast<uint32_t>(static_cast<uint16_t>(sprite.clutX_10))
             << 12) &
            0x003F0000u);
}

uint32_t MakeWhWord8003FA20(const FastSpriteLocalFields8003FA20& sprite) {
    return static_cast<uint32_t>(sprite.width_08) |
           (static_cast<uint32_t>(sprite.height_0A) << 16);
}

void MarkRawLocalByteKnown8003FA20(FastSpriteRawLocalBytes8003FA20& raw,
                                   std::size_t offset,
                                   uint8_t value) {
    if (offset >= raw.bytes.size()) {
        return;
    }
    raw.bytes[offset] = value;
    raw.known[offset] = true;
}

void MarkRawLocalWordKnown8003FA20(FastSpriteRawLocalBytes8003FA20& raw,
                                   std::size_t offset,
                                   uint16_t value) {
    MarkRawLocalByteKnown8003FA20(raw,
                                  offset,
                                  static_cast<uint8_t>(value & 0xFFu));
    MarkRawLocalByteKnown8003FA20(
        raw,
        offset + 1u,
        static_cast<uint8_t>((value >> 8) & 0xFFu));
}

void MarkRawLocalDwordKnown8003FA20(FastSpriteRawLocalBytes8003FA20& raw,
                                    std::size_t offset,
                                    uint32_t value) {
    MarkRawLocalByteKnown8003FA20(raw,
                                  offset,
                                  static_cast<uint8_t>(value & 0xFFu));
    MarkRawLocalByteKnown8003FA20(
        raw,
        offset + 1u,
        static_cast<uint8_t>((value >> 8) & 0xFFu));
    MarkRawLocalByteKnown8003FA20(
        raw,
        offset + 2u,
        static_cast<uint8_t>((value >> 16) & 0xFFu));
    MarkRawLocalByteKnown8003FA20(
        raw,
        offset + 3u,
        static_cast<uint8_t>((value >> 24) & 0xFFu));
}

bool IsAuthoritativeRawLocalRgbProvenance8003FA20(
    FastSpriteRawLocalProvenance8003FA20 provenance) {
    switch (provenance) {
    case FastSpriteRawLocalProvenance8003FA20::StaticLocalFields:
    case FastSpriteRawLocalProvenance8003FA20::
        Stage1GlyphStackScratch8001B954:
    case FastSpriteRawLocalProvenance8003FA20::
        Stage1ScriptBoxCornerStackScratch8001B590:
        return true;
    case FastSpriteRawLocalProvenance8003FA20::None:
    case FastSpriteRawLocalProvenance8003FA20::PsxStackScratchResidue:
    case FastSpriteRawLocalProvenance8003FA20::ReplayReconstructedStackResidue:
    case FastSpriteRawLocalProvenance8003FA20::StaticGlobalObject:
        return false;
    }
    return false;
}

FastSpriteLocalFields8003FA20 ResolveSpriteLocalBytes8003FA20(
    const GsSortFastSpriteInput8003FA20& input) {
    FastSpriteLocalFields8003FA20 out = input.sprite;
    const FastSpriteRawLocalBytes8003FA20& raw = input.rawLocalBytes;
    if (HasKnownRgbTail8003FA20(raw)) {
        out.r_14 = raw.bytes[kFastSpriteLocalRgbR8003FA20];
        out.g_15 = raw.bytes[kFastSpriteLocalRgbG8003FA20];
        out.b_16 = raw.bytes[kFastSpriteLocalRgbB8003FA20];
    }
    return out;
}

uint32_t ResolveOtSlotAddr8003FA20(const GsSortFastSpriteInput8003FA20& input) {
    return input.ot.headAddr_04 +
           static_cast<uint32_t>(input.priority) * 4u -
           input.ot.length_08 * 4u;
}

GsSortFastSpriteSkipReason8003FA20 ResolveSkipReason8003FA20(
    const FastSpriteLocalFields8003FA20& sprite) {
    if ((sprite.attr_00 & 0x80000000u) != 0u) {
        return GsSortFastSpriteSkipReason8003FA20::NegativeAttr;
    }
    if (sprite.width_08 == 0u) {
        return GsSortFastSpriteSkipReason8003FA20::ZeroWidth;
    }
    if (sprite.height_0A == 0u) {
        return GsSortFastSpriteSkipReason8003FA20::ZeroHeight;
    }
    return GsSortFastSpriteSkipReason8003FA20::None;
}

GsSortFastSpriteReturnValue8003FA20 ResolveSkippedReturnValue8003FA20(
    GsSortFastSpriteSkipReason8003FA20 reason) {
    GsSortFastSpriteReturnValue8003FA20 out{};
    switch (reason) {
    case GsSortFastSpriteSkipReason8003FA20::NegativeAttr:
        out.source =
            GsSortFastSpriteReturnValueSource8003FA20::UnchangedV0NegativeAttr;
        return out;
    case GsSortFastSpriteSkipReason8003FA20::ZeroWidth:
        out.known = true;
        out.value = 0;
        out.source = GsSortFastSpriteReturnValueSource8003FA20::WidthZero;
        return out;
    case GsSortFastSpriteSkipReason8003FA20::ZeroHeight:
        out.known = true;
        out.value = 0;
        out.source = GsSortFastSpriteReturnValueSource8003FA20::HeightZero;
        return out;
    case GsSortFastSpriteSkipReason8003FA20::None:
        return out;
    }
    return out;
}

bool AppendRuntimePacketWrite8003FA20(
    RuntimeState8003FA20& runtime,
    const RuntimePacketWrite8003FA20& write) {
    for (RuntimePacketWrite8003FA20& slot : runtime.packetWrites) {
        if (!slot.valid) {
            slot = write;
            slot.valid = true;
            return true;
        }
    }
    return false;
}

bool IsStage1MovieTextGlyphStackScratch8001B954(
    const FastSpriteStackScratchTail8003FA20& stackTail) {
    if (stackTail.source !=
            FastSpriteStackScratchSource8003FA20::SavedRegisterSpill8001B26C ||
        stackTail.sourceFunction != kSub8001B25CFastSpritePrefixWriter ||
        stackTail.consumerFunction != 0x8001B954u ||
        stackTail.consumerCallsite != 0x8001BBF8u) {
        return false;
    }

    constexpr std::array<std::size_t, 3> kRgbOffsets = {{
        kFastSpriteLocalRgbR8003FA20,
        kFastSpriteLocalRgbG8003FA20,
        kFastSpriteLocalRgbB8003FA20,
    }};
    for (std::size_t i = 0; i < stackTail.rgbTail.size(); ++i) {
        const FastSpriteStackScratchByte8003FA20& byte =
            stackTail.rgbTail[i];
        if (!byte.known || byte.localOffset != kRgbOffsets[i] ||
            byte.writerPc != kStackSpill8001B26C ||
            byte.writerFunction != kSub8001B25CFastSpritePrefixWriter) {
            return false;
        }
    }
    return true;
}

GsSortFastSpriteSubmitProvenance8003FA20
BuildSubmitProvenance8003FA20(const GsSortFastSpriteInput8003FA20& input) {
    GsSortFastSpriteSubmitProvenance8003FA20 out = input.provenance;
    const FastSpriteRawLocalBytes8003FA20& raw = input.rawLocalBytes;
    out.active = out.active || raw.active || raw.sourceFunction != 0u ||
                 raw.callsite != 0u || raw.localObjectAddr != 0u ||
                 raw.lastWriterPc != 0u;
    if (out.rawLocalProvenance ==
        FastSpriteRawLocalProvenance8003FA20::None) {
        out.rawLocalProvenance = raw.provenance;
    }
    if (out.sourceFunction == 0u) {
        out.sourceFunction = raw.sourceFunction;
    }
    if (out.callsite == 0u) {
        out.callsite = raw.callsite;
    }
    if (out.localObjectAddr == 0u) {
        out.localObjectAddr = raw.localObjectAddr;
    }
    if (out.lastWriterPc == 0u) {
        out.lastWriterPc = raw.lastWriterPc;
    }
    out.priority = input.priority;
    return out;
}

RuntimePacketWrite8003FA20 BuildRuntimePacketWrite8003FA20(
    const GsSortFastSpritePacket8003FA20& packet,
    const GsSortFastSpriteSubmitProvenance8003FA20& provenance) {
    RuntimePacketWrite8003FA20 out{};
    out.valid = packet.written;
    out.addr = packet.addr;
    out.wordCount = kGsSortFastSpritePacketWordCount8003FA20;
    out.words = packet.Words();
    for (bool& known : out.wordKnown) {
        known = packet.written;
    }
    out.word2CommandKnown = packet.written;
    out.word2CommandCode =
        static_cast<uint8_t>((packet.word2_colorCode >> 24) & 0xFFu);
    out.provenance = provenance;
    return out;
}

RuntimePacketWrite8003FA20 BuildRuntimePacketWrite8003FA20(
    const GsSortFastSpritePartialPacket8003FA20& packet,
    const GsSortFastSpriteSubmitProvenance8003FA20& provenance) {
    RuntimePacketWrite8003FA20 out{};
    out.valid = packet.wouldWrite;
    out.addr = packet.addr;
    out.wordCount = kGsSortFastSpritePacketWordCount8003FA20;
    out.words = {{
        packet.word0_linkTag,
        packet.word1_drawMode,
        packet.word2_colorCode,
        packet.word3_xy,
        packet.word4_uvClut,
        packet.word5_wh,
    }};
    out.wordKnown = {{
        packet.word0Known,
        packet.word1Known,
        packet.word2ColorKnown,
        packet.word3Known,
        packet.word4Known,
        packet.word5Known,
    }};
    out.word2CommandKnown = packet.word2CommandKnown;
    out.word2CommandCode = packet.word2CommandCode;
    out.provenance = provenance;
    return out;
}

RuntimeUpdate8003FA20 ResolveRuntimeUpdate8003FA20(
    RuntimeState8003FA20& runtime,
    const GsSortFastSpriteResult8003FA20& result,
    bool apply) {
    RuntimeUpdate8003FA20 out{};
    out.dryRun = !apply;
    out.skipped = result.skipped;
    out.skipReason = result.skipReason;
    out.oldAllocatorAddr = runtime.dword_800901C8;
    out.newAllocatorAddr = runtime.dword_800901C8;
    out.allocatorOldMatchesRuntime =
        runtime.dword_800901C8 == result.allocator.oldAddr;
    out.packetWouldWrite = result.packet.written;
    out.packetAddr = result.packet.addr;
    out.otSlot = result.otSlot;
    out.returnValue = result.returnValue;

    const uint32_t runtimeSlotOldValue =
        GetRuntimeOtSlotOldValue8003FA20(runtime.ot, result.otSlot.addr);
    out.otSlotOldMatchesRuntime =
        runtimeSlotOldValue == result.otSlot.oldValue;

    if (result.skipped) {
        return out;
    }

    out.allocatorWouldUpdate = true;
    out.newAllocatorAddr = result.allocator.newAddr;
    out.otSlotWouldUpdate = true;

    if (!apply) {
        return out;
    }

    if (result.packet.written) {
        out.packetWriteMirrored = AppendRuntimePacketWrite8003FA20(
            runtime,
            BuildRuntimePacketWrite8003FA20(result.packet,
                                           result.provenance));
        out.packetWriteCapacityExceeded = !out.packetWriteMirrored;
    }
    out.otSlotUpdated =
        SetRuntimeOtSlotValue8003FA20(runtime.ot,
                                      result.otSlot.addr,
                                      result.otSlot.newValue);
    out.otSlotCapacityExceeded = !out.otSlotUpdated;
    if (!out.otSlotUpdated) {
        return out;
    }

    runtime.dword_800901C8 = result.allocator.newAddr;
    out.allocatorUpdated = true;
    out.newAllocatorAddr = result.allocator.newAddr;
    return out;
}

RuntimeUpdate8003FA20 ResolveRuntimeUpdate8003FA20(
    RuntimeState8003FA20& runtime,
    const GsSortFastSpritePartialResult8003FA20& result,
    bool apply) {
    RuntimeUpdate8003FA20 out{};
    out.dryRun = !apply;
    out.skipped = result.skipped;
    out.skipReason = result.skipReason;
    out.oldAllocatorAddr = runtime.dword_800901C8;
    out.newAllocatorAddr = runtime.dword_800901C8;
    out.allocatorOldMatchesRuntime =
        runtime.dword_800901C8 == result.allocator.oldAddr;
    out.packetWouldWrite = result.packet.wouldWrite;
    out.packetAddr = result.packet.addr;
    out.otSlot = result.otSlot;
    out.returnValue = result.returnValue;

    const uint32_t runtimeSlotOldValue =
        GetRuntimeOtSlotOldValue8003FA20(runtime.ot, result.otSlot.addr);
    out.otSlotOldMatchesRuntime =
        runtimeSlotOldValue == result.otSlot.oldValue;

    if (result.skipped) {
        return out;
    }

    out.allocatorWouldUpdate = true;
    out.newAllocatorAddr = result.allocator.newAddr;
    out.otSlotWouldUpdate = true;

    if (!apply) {
        return out;
    }

    if (result.packet.wouldWrite) {
        out.packetWriteMirrored = AppendRuntimePacketWrite8003FA20(
            runtime,
            BuildRuntimePacketWrite8003FA20(result.packet,
                                           result.provenance));
        out.packetWriteCapacityExceeded = !out.packetWriteMirrored;
    }
    out.otSlotUpdated =
        SetRuntimeOtSlotValue8003FA20(runtime.ot,
                                      result.otSlot.addr,
                                      result.otSlot.newValue);
    out.otSlotCapacityExceeded = !out.otSlotUpdated;
    if (!out.otSlotUpdated) {
        return out;
    }

    runtime.dword_800901C8 = result.allocator.newAddr;
    out.allocatorUpdated = true;
    out.newAllocatorAddr = result.allocator.newAddr;
    return out;
}

} // namespace

GsSortFastSpriteResult8003FA20 PsxCall8003FA20_GsSortFastSprite(
    const GsSortFastSpriteInput8003FA20& input) {
    GsSortFastSpriteResult8003FA20 out{};
    const FastSpriteLocalFields8003FA20 sprite =
        ResolveSpriteLocalBytes8003FA20(input);
    out.otSlot.expression = kOtSlotExpression8003FA20;
    out.otSlot.addr = ResolveOtSlotAddr8003FA20(input);
    out.otSlot.oldValue = input.ot.slotOldValue;
    out.otSlot.newValue = input.ot.slotOldValue;
    out.allocator.oldAddr = input.packetAllocatorAddr_dword_800901C8;
    out.allocator.newAddr = input.packetAllocatorAddr_dword_800901C8;
    out.provenance = BuildSubmitProvenance8003FA20(input);

    out.skipReason = ResolveSkipReason8003FA20(sprite);
    out.skipped =
        out.skipReason != GsSortFastSpriteSkipReason8003FA20::None;
    if (out.skipped) {
        out.returnValue = ResolveSkippedReturnValue8003FA20(out.skipReason);
        return out;
    }

    const uint32_t packetAddr =
        input.packetAllocatorAddr_dword_800901C8;
    out.packet.written = true;
    out.packet.addr = packetAddr;
    out.packet.word0_linkTag =
        input.ot.slotOldValue +
        kGsSortFastSpritePacketLinkTagAddend8003FA20;
    out.packet.word1_drawMode =
        MakeDrawModeWord8003FA20(sprite.attr_00,
                                 sprite.tpage_0C);
    out.packet.word2_colorCode =
        MakeColorCodeWord8003FA20(sprite.attr_00,
                                  sprite.r_14,
                                  sprite.g_15,
                                  sprite.b_16);
    out.packet.word3_xy =
        MakeXyWord8003FA20(sprite, input.drawOffsets);
    out.packet.word4_uvClut = MakeUvClutWord8003FA20(sprite);
    out.packet.word5_wh = MakeWhWord8003FA20(sprite);

    out.otSlot.newValue =
        packetAddr & kGsSortFastSpritePacketAddrMask8003FA20;
    out.allocator.advanceWords =
        kGsSortFastSpriteAllocatorAdvanceWords8003FA20;
    out.allocator.advanceBytes =
        kGsSortFastSpriteAllocatorAdvanceBytes8003FA20;
    out.allocator.newAddr =
        packetAddr + kGsSortFastSpriteAllocatorAdvanceBytes8003FA20;
    out.returnValue.known = true;
    out.returnValue.value = out.packet.word0_linkTag;
    out.returnValue.source =
        GsSortFastSpriteReturnValueSource8003FA20::OtLinkTag;
    return out;
}

bool HasKnownRgbTail8003FA20(
    const FastSpriteRawLocalBytes8003FA20& rawLocalBytes) {
    return rawLocalBytes.active &&
           IsAuthoritativeRawLocalRgbProvenance8003FA20(
               rawLocalBytes.provenance) &&
           rawLocalBytes.known[kFastSpriteLocalRgbR8003FA20] &&
           rawLocalBytes.known[kFastSpriteLocalRgbG8003FA20] &&
           rawLocalBytes.known[kFastSpriteLocalRgbB8003FA20];
}

bool HasAnyKnownRawLocalByte8003FA20(
    const FastSpriteRawLocalBytes8003FA20& rawLocalBytes) {
    if (!rawLocalBytes.active) {
        return false;
    }
    for (bool known : rawLocalBytes.known) {
        if (known) {
            return true;
        }
    }
    return false;
}

FastSpriteRawLocalBytes8003FA20
BuildRawLocalBytesFromKnownFields8003FA20(
    const FastSpriteLocalFields8003FA20& sprite,
    const FastSpriteRawLocalBytes8003FA20& stackSeed,
    uint32_t sourceFunction,
    uint32_t callsite,
    const char* note) {
    FastSpriteRawLocalBytes8003FA20 out = stackSeed;
    out.active = true;
    if (out.provenance == FastSpriteRawLocalProvenance8003FA20::None) {
        out.provenance =
            FastSpriteRawLocalProvenance8003FA20::PsxStackScratchResidue;
    }
    out.sourceFunction = sourceFunction;
    out.callsite = callsite;
    out.note = note;

    MarkRawLocalDwordKnown8003FA20(out, 0x00u, sprite.attr_00);
    MarkRawLocalWordKnown8003FA20(
        out,
        0x04u,
        static_cast<uint16_t>(sprite.x_04));
    MarkRawLocalWordKnown8003FA20(out, 0x06u, sprite.y_06);
    MarkRawLocalWordKnown8003FA20(out, 0x08u, sprite.width_08);
    MarkRawLocalWordKnown8003FA20(out, 0x0Au, sprite.height_0A);
    MarkRawLocalWordKnown8003FA20(out, 0x0Cu, sprite.tpage_0C);
    MarkRawLocalByteKnown8003FA20(out, 0x0Eu, sprite.u_0E);
    MarkRawLocalByteKnown8003FA20(out, 0x0Fu, sprite.v_0F);
    MarkRawLocalWordKnown8003FA20(
        out,
        0x10u,
        static_cast<uint16_t>(sprite.clutX_10));
    MarkRawLocalWordKnown8003FA20(
        out,
        0x12u,
        static_cast<uint16_t>(sprite.clutY_12));
    return out;
}

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
    const char* note) {
    FastSpriteStackScratchTail8003FA20 out{};
    out.active = true;
    out.source = source;
    out.sourceFunction = sourceFunction;
    out.consumerFunction = consumerFunction;
    out.consumerCallsite = consumerCallsite;
    out.semanticRgbProducer = false;
    out.defaultRgbForbidden = true;
    out.note = note;

    constexpr std::array<std::size_t, 3> kRgbOffsets = {{
        kFastSpriteLocalRgbR8003FA20,
        kFastSpriteLocalRgbG8003FA20,
        kFastSpriteLocalRgbB8003FA20,
    }};
    for (std::size_t i = 0; i < out.rgbTail.size(); ++i) {
        FastSpriteStackScratchByte8003FA20& byte = out.rgbTail[i];
        byte.localOffset = kRgbOffsets[i];
        byte.writerPc = writerPc;
        byte.writerFunction = writerFunction;
        if (savedRegisterKnown) {
            byte.known = true;
            byte.value = static_cast<uint8_t>(
                (savedRegisterValue >> (static_cast<uint32_t>(i) * 8u)) &
                0xFFu);
        }
    }
    return out;
}

FastSpriteStackScratchTail8003FA20
BuildStackScratchTail8001DF24To8001BEE4(
    uint32_t savedRegisterValue,
    bool savedRegisterKnown,
    const char* note) {
    return BuildStackScratchTailFromSavedRegisterSpill8003FA20(
        FastSpriteStackScratchSource8003FA20::SavedRegisterSpill80048040,
        kSub8004800CFormatter,
        kStackSpill80048040,
        kSub8001DF24HudSubmitChain,
        kSub8001BEE4FastSpriteWrapper,
        0x8001BF1Cu,
        savedRegisterValue,
        savedRegisterKnown,
        note);
}

FastSpriteStackScratchTail8003FA20
BuildStackScratchTail8001B25CSpillToConsumer(
    uint32_t consumerFunction,
    uint32_t consumerCallsite,
    uint32_t savedRegisterValue,
    bool savedRegisterKnown,
    const char* note) {
    return BuildStackScratchTailFromSavedRegisterSpill8003FA20(
        FastSpriteStackScratchSource8003FA20::SavedRegisterSpill8001B274,
        kSub8001B25CFastSpritePrefixWriter,
        kStackSpill8001B274,
        kSub8001B25CFastSpritePrefixWriter,
        consumerFunction,
        consumerCallsite,
        savedRegisterValue,
        savedRegisterKnown,
        note);
}

FastSpriteStackScratchTail8003FA20
BuildStackScratchTail8001B26CSpillToConsumer(
    uint32_t consumerFunction,
    uint32_t consumerCallsite,
    uint32_t savedRegisterValue,
    bool savedRegisterKnown,
    const char* note) {
    return BuildStackScratchTailFromSavedRegisterSpill8003FA20(
        FastSpriteStackScratchSource8003FA20::SavedRegisterSpill8001B26C,
        kSub8001B25CFastSpritePrefixWriter,
        kStackSpill8001B26C,
        kSub8001B25CFastSpritePrefixWriter,
        consumerFunction,
        consumerCallsite,
        savedRegisterValue,
        savedRegisterKnown,
        note);
}

FastSpriteRawLocalBytes8003FA20
ApplyStackScratchTailToRawLocalBytes8003FA20(
    const FastSpriteRawLocalBytes8003FA20& rawLocalBytes,
    const FastSpriteStackScratchTail8003FA20& stackTail) {
    FastSpriteRawLocalBytes8003FA20 out = rawLocalBytes;
    if (!stackTail.active) {
        return out;
    }

    out.active = true;
    out.provenance =
        IsStage1MovieTextGlyphStackScratch8001B954(stackTail)
            ? FastSpriteRawLocalProvenance8003FA20::
                  Stage1GlyphStackScratch8001B954
            : FastSpriteRawLocalProvenance8003FA20::PsxStackScratchResidue;
    out.sourceFunction = stackTail.consumerFunction;
    out.callsite = stackTail.consumerCallsite;
    out.note = stackTail.note;

    for (const FastSpriteStackScratchByte8003FA20& byte :
         stackTail.rgbTail) {
        if (!byte.known) {
            continue;
        }
        MarkRawLocalByteKnown8003FA20(out, byte.localOffset, byte.value);
        out.lastWriterPc = byte.writerPc;
    }
    return out;
}

GsSortFastSpritePartialResult8003FA20
PredictGsSortFastSpritePartial8003FA20(
    const GsSortFastSpriteInput8003FA20& input,
    bool localSpriteRgbKnown) {
    GsSortFastSpritePartialResult8003FA20 out{};
    const FastSpriteLocalFields8003FA20 sprite =
        ResolveSpriteLocalBytes8003FA20(input);
    const bool rgbKnown =
        localSpriteRgbKnown || HasKnownRgbTail8003FA20(input.rawLocalBytes);
    out.wouldCall8003FA20 = true;
    out.otSlot.expression = kOtSlotExpression8003FA20;
    out.otSlot.addr = ResolveOtSlotAddr8003FA20(input);
    out.otSlot.oldValue = input.ot.slotOldValue;
    out.otSlot.newValue = input.ot.slotOldValue;
    out.allocator.oldAddr = input.packetAllocatorAddr_dword_800901C8;
    out.allocator.newAddr = input.packetAllocatorAddr_dword_800901C8;
    out.provenance = BuildSubmitProvenance8003FA20(input);

    out.skipReason = ResolveSkipReason8003FA20(sprite);
    out.skipped =
        out.skipReason != GsSortFastSpriteSkipReason8003FA20::None;
    if (out.skipped) {
        out.returnValue = ResolveSkippedReturnValue8003FA20(out.skipReason);
        return out;
    }

    const uint32_t packetAddr =
        input.packetAllocatorAddr_dword_800901C8;
    out.packet.wouldWrite = true;
    out.packet.addr = packetAddr;
    out.packet.word0Known = true;
    out.packet.word0_linkTag =
        input.ot.slotOldValue +
        kGsSortFastSpritePacketLinkTagAddend8003FA20;
    out.packet.word1Known = true;
    out.packet.word1_drawMode =
        MakeDrawModeWord8003FA20(sprite.attr_00,
                                 sprite.tpage_0C);
    out.packet.word2CommandKnown = true;
    out.packet.word2CommandCode =
        static_cast<uint8_t>(((sprite.attr_00 >> 29) & 0x02u) |
                             ((sprite.attr_00 >> 6) & 0x01u) |
                             0x64u);
    out.packet.word3Known = true;
    out.packet.word3_xy =
        MakeXyWord8003FA20(sprite, input.drawOffsets);
    out.packet.word4Known = true;
    out.packet.word4_uvClut = MakeUvClutWord8003FA20(sprite);
    out.packet.word5Known = true;
    out.packet.word5_wh = MakeWhWord8003FA20(sprite);
    out.packetGeometryKnown = true;

    if (rgbKnown) {
        out.packet.word2ColorKnown = true;
        out.packet.word2_colorCode =
            MakeColorCodeWord8003FA20(sprite.attr_00,
                                      sprite.r_14,
                                      sprite.g_15,
                                      sprite.b_16);
        out.packet.word2CommandCode =
            static_cast<uint8_t>((out.packet.word2_colorCode >> 24) & 0xFFu);
        out.packetColorKnown = true;
    } else {
        out.gap =
            GsSortFastSpritePartialGap8003FA20::
                LocalSpriteRgbStaticOwnerUnclosed;
        out.rgbGapEvidence.active = true;
        out.rgbGapEvidence.reason =
            kLocalSpriteRgbStaticOwnerGapReason8003FA20;
    }

    out.otSlot.newValue =
        packetAddr & kGsSortFastSpritePacketAddrMask8003FA20;
    out.allocator.advanceWords =
        kGsSortFastSpriteAllocatorAdvanceWords8003FA20;
    out.allocator.advanceBytes =
        kGsSortFastSpriteAllocatorAdvanceBytes8003FA20;
    out.allocator.newAddr =
        packetAddr + kGsSortFastSpriteAllocatorAdvanceBytes8003FA20;
    out.returnValue.known = true;
    out.returnValue.value = out.packet.word0_linkTag;
    out.returnValue.source =
        GsSortFastSpriteReturnValueSource8003FA20::OtLinkTag;
    return out;
}

uint32_t ResolveRuntimeOtSlotAddr8003FA20(
    const RuntimeOrderingTableState8003FA20& ot,
    uint16_t priority) {
    return ot.headAddr_04 + static_cast<uint32_t>(priority) * 4u -
           ot.length_08 * 4u;
}

bool TryGetRuntimeOtSlotValue8003FA20(
    const RuntimeOrderingTableState8003FA20& ot,
    uint32_t addr,
    uint32_t* outValue) {
    for (const RuntimeOtSlotValue8003FA20& slot : ot.slotValues) {
        if (slot.valid && slot.addr == addr) {
            if (outValue != nullptr) {
                *outValue = slot.value;
            }
            return true;
        }
    }
    return false;
}

uint32_t GetRuntimeOtSlotOldValue8003FA20(
    const RuntimeOrderingTableState8003FA20& ot,
    uint32_t addr) {
    uint32_t value = 0;
    if (TryGetRuntimeOtSlotValue8003FA20(ot, addr, &value)) {
        return value;
    }
    return 0;
}

bool SetRuntimeOtSlotValue8003FA20(
    RuntimeOrderingTableState8003FA20& ot,
    uint32_t addr,
    uint32_t value) {
    for (RuntimeOtSlotValue8003FA20& slot : ot.slotValues) {
        if (slot.valid && slot.addr == addr) {
            slot.value = value;
            return true;
        }
    }

    for (RuntimeOtSlotValue8003FA20& slot : ot.slotValues) {
        if (!slot.valid) {
            slot.valid = true;
            slot.addr = addr;
            slot.value = value;
            return true;
        }
    }
    return false;
}

bool SetRuntimeOtSlotValueForPriority8003FA20(
    RuntimeOrderingTableState8003FA20& ot,
    uint16_t priority,
    uint32_t value) {
    return SetRuntimeOtSlotValue8003FA20(
        ot,
        ResolveRuntimeOtSlotAddr8003FA20(ot, priority),
        value);
}

OrderingTableState8003FA20 BuildOrderingTableStateFromRuntime8003FA20(
    const RuntimeOrderingTableState8003FA20& ot,
    uint16_t priority) {
    OrderingTableState8003FA20 out{};
    out.headAddr_04 = ot.headAddr_04;
    out.length_08 = ot.length_08;
    out.slotOldValue = GetRuntimeOtSlotOldValue8003FA20(
        ot,
        ResolveRuntimeOtSlotAddr8003FA20(ot, priority));
    return out;
}

GsSortFastSpriteInput8003FA20 BuildInputFromRuntime8003FA20(
    const RuntimeState8003FA20& runtime,
    const FastSpriteLocalFields8003FA20& sprite,
    uint16_t priority) {
    GsSortFastSpriteInput8003FA20 out{};
    out.sprite = sprite;
    out.ot = BuildOrderingTableStateFromRuntime8003FA20(runtime.ot, priority);
    out.drawOffsets.word_800917AA = runtime.word_800917AA;
    out.drawOffsets.word_800917AC = runtime.word_800917AC;
    out.packetAllocatorAddr_dword_800901C8 = runtime.dword_800901C8;
    out.priority = priority;
    out.provenance.priority = priority;
    return out;
}

GsSortFastSpriteInput8003FA20 BuildInputFromRuntime8003FA20(
    const RuntimeState8003FA20& runtime,
    const FastSpriteLocalFields8003FA20& sprite,
    const FastSpriteRawLocalBytes8003FA20& rawLocalBytes,
    uint16_t priority) {
    GsSortFastSpriteInput8003FA20 out =
        BuildInputFromRuntime8003FA20(runtime, sprite, priority);
    out.rawLocalBytes = rawLocalBytes;
    return out;
}

RuntimeUpdate8003FA20 PredictRuntimeUpdate8003FA20(
    const RuntimeState8003FA20& runtime,
    const GsSortFastSpriteResult8003FA20& result) {
    RuntimeState8003FA20 copy = runtime;
    return ResolveRuntimeUpdate8003FA20(copy, result, false);
}

RuntimeUpdate8003FA20 ApplyRuntimeUpdate8003FA20(
    RuntimeState8003FA20& runtime,
    const GsSortFastSpriteResult8003FA20& result) {
    return ResolveRuntimeUpdate8003FA20(runtime, result, true);
}

RuntimeUpdate8003FA20 PredictRuntimeUpdate8003FA20(
    const RuntimeState8003FA20& runtime,
    const GsSortFastSpritePartialResult8003FA20& result) {
    RuntimeState8003FA20 copy = runtime;
    return ResolveRuntimeUpdate8003FA20(copy, result, false);
}

RuntimeUpdate8003FA20 ApplyRuntimeUpdate8003FA20(
    RuntimeState8003FA20& runtime,
    const GsSortFastSpritePartialResult8003FA20& result) {
    return ResolveRuntimeUpdate8003FA20(runtime, result, true);
}

} // namespace PrPsxFastSpriteSubmitDirect
