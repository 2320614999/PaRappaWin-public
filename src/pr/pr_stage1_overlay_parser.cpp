#include "pr_stage1_overlay_parser.h"
#include "pr_sqevs1.h"

#include "../int_loader.h"
#include "../logger.h"

#include <algorithm>
#include <filesystem>
#include <fstream>

namespace {

uint32_t ReadU32LE(const uint8_t* p) {
    return (uint32_t)p[0]
         | ((uint32_t)p[1] << 8)
         | ((uint32_t)p[2] << 16)
         | ((uint32_t)p[3] << 24);
}

uint16_t ReadU16LE(const uint8_t* p) {
    return (uint16_t)p[0] | ((uint16_t)p[1] << 8);
}

int16_t ReadS16LE(const uint8_t* p) {
    return static_cast<int16_t>(ReadU16LE(p));
}

bool PsxToOffset(uint32_t addr, uint32_t base, size_t size, size_t& outOff) {
    if (addr < base) {
        return false;
    }
    const size_t off = static_cast<size_t>(addr - base);
    if (off >= size) {
        return false;
    }
    outOff = off;
    return true;
}

bool ReadFileBytes(const std::string& path, std::vector<uint8_t>& out) {
    out.clear();
    std::ifstream f(std::filesystem::u8path(path), std::ios::binary | std::ios::ate);
    if (!f.is_open()) {
        Log::Printf("PrStage1OverlayParser: open failed: %s", path.c_str());
        return false;
    }

    const std::streamsize sz = f.tellg();
    if (sz <= 0) {
        Log::Printf("PrStage1OverlayParser: empty file: %s", path.c_str());
        return false;
    }

    f.seekg(0);
    out.resize(static_cast<size_t>(sz));
    f.read(reinterpret_cast<char*>(out.data()), sz);
    if (!f.good()) {
        Log::Printf("PrStage1OverlayParser: read failed: %s", path.c_str());
        out.clear();
        return false;
    }
    return true;
}

std::string ReadCString(const uint8_t* bin, size_t size, uint32_t base, uint32_t psxAddr) {
    if (psxAddr == 0) {
        return {};
    }

    size_t off = 0;
    if (!PsxToOffset(psxAddr, base, size, off)) {
        return {};
    }

    std::string out;
    while (off < size && bin[off] != 0) {
        out.push_back(static_cast<char>(bin[off]));
        off++;
    }
    return out;
}

bool BuildMemHandleMap(const IntArchive& compo, std::vector<std::string>& outNames) {
    size_t memIndex = 0;
    for (const auto& entry : compo.entries) {
        if (entry.type != IntBlockType::Mem) {
            continue;
        }
        const size_t handle = memIndex + PrStage1OverlayData::kFirstMemHandle;
        if (handle >= outNames.size()) {
            outNames.resize(handle + 1);
        }
        outNames[handle] = entry.name;
        memIndex++;
    }

    return !outNames.empty();
}

bool ParseTextTables(const uint8_t* comodData, size_t comodSize, PrStage1OverlayData& out) {
    uint32_t ptrs[PrStage1OverlayData::kTextTableCount] = {};
    for (uint32_t i = 0; i < PrStage1OverlayData::kTextTableCount; i++) {
        size_t off = 0;
        if (!PsxToOffset(PrStage1OverlayData::kTextTablePtrsAddr + i * 4, out.psxBase, comodSize, off)) {
            return false;
        }
        ptrs[i] = ReadU32LE(comodData + off);
    }

    for (uint32_t i = 0; i < PrStage1OverlayData::kTextTableCount; i++) {
        uint32_t end = (i + 1 < PrStage1OverlayData::kTextTableCount)
                           ? ptrs[i + 1]
                           : PrStage1OverlayData::kTextTablePtrsAddr;
        const uint32_t count = (end - ptrs[i]) / 4;

        auto& table = out.textTables[i];
        table.tableIndex = static_cast<uint8_t>(i);
        table.psxAddr = ptrs[i];
        table.entryPtrs.reserve(count);
        table.decodedText.reserve(count);

        for (uint32_t textId = 0; textId < count; textId++) {
            size_t off = 0;
            if (!PsxToOffset(ptrs[i] + textId * 4, out.psxBase, comodSize, off)) {
                return false;
            }
            const uint32_t textPtr = ReadU32LE(comodData + off);
            table.entryPtrs.push_back(textPtr);
            table.decodedText.push_back(ReadCString(comodData, comodSize, out.psxBase, textPtr));
        }
    }

    return true;
}

bool ParsePairTable(const uint8_t* comodData, size_t comodSize, PrStage1OverlayData& out) {
    out.pairTable.clear();
    out.pairTable.reserve(PrStage1OverlayData::kPairTableCount);
    for (uint32_t i = 0; i < PrStage1OverlayData::kPairTableCount; i++) {
        size_t off = 0;
        if (!PsxToOffset(PrStage1OverlayData::kPairTableAddr + i * 4, out.psxBase, comodSize, off)) {
            return false;
        }
        PrStage1PairEntry entry;
        entry.idA = ReadS16LE(comodData + off);
        entry.idB = ReadS16LE(comodData + off + 2);
        out.pairTable.push_back(entry);
    }
    return true;
}

bool ParseStreamDescRows(const uint8_t* comodData, size_t comodSize, PrStage1OverlayData& out) {
    for (uint32_t row = 0; row < PrStage1OverlayData::kStreamDescRowCount; row++) {
        size_t off = 0;
        if (!PsxToOffset(PrStage1OverlayData::kStreamDescAddr + row * 8, out.psxBase, comodSize, off)) {
            return false;
        }
        for (uint32_t i = 0; i < 8; i++) {
            out.streamDescRows[row].bytes[i] = comodData[off + i];
        }
    }
    return true;
}

bool ParseCompactRailRec44(const uint8_t* comodData,
                           size_t comodSize,
                           PrStage1OverlayData& out) {
    for (uint32_t recIndex = 0; recIndex < out.compactRailRec44.size(); ++recIndex) {
        const uint32_t psxAddr =
            PrStage1OverlayData::kCompactRailRec44BaseAddr +
            recIndex * PrStage1OverlayData::kCompactRailRec44Size;
        size_t off = 0;
        if (!PsxToOffset(psxAddr, out.psxBase, comodSize, off) ||
            off + PrStage1OverlayData::kCompactRailRec44Size > comodSize) {
            break;
        }

        PrStage1CompactRailRec44& rec = out.compactRailRec44[recIndex];
        rec.valid = true;
        rec.recordIndex = static_cast<uint8_t>(recIndex);
        rec.psxAddr = psxAddr;
        rec.head0 = ReadS16LE(comodData + off);
        rec.head1 = ReadU16LE(comodData + off + 2);
        for (size_t i = 0; i < rec.streamA.size(); ++i) {
            rec.streamA[i] = static_cast<int8_t>(comodData[off + 4 + i]);
        }
        for (size_t i = 0; i < rec.streamB.size(); ++i) {
            rec.streamB[i] = static_cast<int8_t>(comodData[off + 24 + i]);
        }
    }
    return true;
}

bool ParseHudOverlayConfigure801CA0E0Rows(const uint8_t* comodData,
                                          size_t comodSize,
                                          PrStage1OverlayData& out) {
    for (uint32_t rowIndex = 0;
         rowIndex < PrStage1OverlayData::kHudOverlayConfigureRowCount;
         ++rowIndex) {
        const uint32_t psxAddr =
            PrStage1OverlayData::kHudOverlayConfigureTableAddr +
            rowIndex * PrStage1OverlayData::kHudOverlayConfigureRowSize;
        size_t off = 0;
        if (!PsxToOffset(psxAddr, out.psxBase, comodSize, off) ||
            off + PrStage1OverlayData::kHudOverlayConfigureRowSize >
                comodSize) {
            return false;
        }

        PrStage1HudOverlayConfigure801CA0E0Row& row =
            out.hudOverlayConfigureRows801CA0E0[rowIndex];
        row.valid = true;
        row.rowIndex = static_cast<uint8_t>(rowIndex);
        row.psxAddr = psxAddr;
        row.paDatResourceIndex = ReadS16LE(comodData + off);
        row.paVdfResourceIndex = ReadS16LE(comodData + off + 2);
        row.timerFrames = ReadS16LE(comodData + off + 4);
        row.ctx11CValue = ReadS16LE(comodData + off + 6);
        row.hudSlotId = ReadS16LE(comodData + off + 8);
    }
    return true;
}

bool ParseHudSlotDescs(const uint8_t* comodData, size_t comodSize, PrStage1OverlayData& out) {
    out.hudSlotDescs.clear();
    out.hudSlotDescs.reserve(PrStage1OverlayData::kHudSlotCount);

    for (uint32_t slot = 0; slot < PrStage1OverlayData::kHudSlotCount; slot++) {
        size_t off = 0;
        const uint32_t psxAddr =
            PrStage1OverlayData::kHudSlotDescAddr +
            slot * PrStage1OverlayData::kHudAnimEventSize;
        if (!PsxToOffset(psxAddr, out.psxBase, comodSize, off)) {
            return false;
        }

        PrStage1HudSlotDesc desc;
        desc.slotId = static_cast<uint8_t>(slot);
        desc.psxAddr = psxAddr;
        desc.eventsPtr = ReadU32LE(comodData + off);
        desc.eventCount = ReadU32LE(comodData + off + 4);
        desc.cursorInit = ReadU32LE(comodData + off + 8);

        if (desc.eventsPtr != 0 && desc.eventCount != 0) {
            desc.events.reserve(desc.eventCount);
            for (uint32_t i = 0; i < desc.eventCount; i++) {
                size_t evOff = 0;
                if (!PsxToOffset(desc.eventsPtr +
                                     i * PrStage1OverlayData::kHudAnimEventSize,
                                 out.psxBase,
                                 comodSize,
                                 evOff)) {
                    return false;
                }

                PrStage1HudAnimEvent ev;
                ev.deltaFrames = ReadU32LE(comodData + evOff);
                for (uint32_t k = 0; k < 4; k++) {
                    ev.timIds[k] = ReadU16LE(comodData + evOff + 4 + k * 2);
                }
                desc.events.push_back(ev);
            }
        }

        out.hudSlotDescs.push_back(std::move(desc));
    }

    return true;
}

bool ParseEventStreams(const uint8_t* comodData, size_t comodSize, PrStage1OverlayData& out) {
    out.streams.clear();
    out.streams.reserve(PrStage1OverlayData::kStreamCount);

    for (uint32_t streamId = 1; streamId <= PrStage1OverlayData::kStreamCount; streamId++) {
        size_t off = 0;
        const uint32_t psxAddr =
            PrStage1OverlayData::kStreamTableAddr +
            streamId * PrStage1OverlayData::kStreamEntrySize;
        if (!PsxToOffset(psxAddr, out.psxBase, comodSize, off)) {
            return false;
        }

        PrStage1EventStream stream;
        stream.streamId = static_cast<uint8_t>(streamId);
        stream.psxAddr = psxAddr;
        stream.eventsPtr = ReadU32LE(comodData + off);
        stream.count = ReadU32LE(comodData + off + 4);
        stream.cursorInit = ReadU32LE(comodData + off + 8);

        if (stream.eventsPtr != 0 && stream.count != 0) {
            stream.events.reserve(stream.count);
            for (uint32_t i = 0; i < stream.count; i++) {
                size_t evOff = 0;
                if (!PsxToOffset(stream.eventsPtr +
                                     i * PrStage1OverlayData::kScriptEventSize,
                                 out.psxBase,
                                 comodSize,
                                 evOff)) {
                    return false;
                }

                PrStage1ScriptEvent ev;
                ev.psxAddr =
                    stream.eventsPtr + i * PrStage1OverlayData::kScriptEventSize;
                ev.frame = ReadU32LE(comodData + evOff);
                ev.flags04 = ReadU32LE(comodData + evOff + 4);
                for (uint32_t k = 0; k < 4; k++) {
                    ev.bezIdByMode[k] = ReadS16LE(comodData + evOff + 8 + k * 2);
                }
                for (uint32_t k = 0; k < 4; k++) {
                    ev.pairByMode[k] = comodData[evOff + 0x10 + k];
                }
                ev.coolPair = comodData[evOff + 0x14];
                ev.noncoolPair = comodData[evOff + 0x15];
                ev.byte16 = comodData[evOff + 0x16];
                ev.hudSlot0 = comodData[evOff + 0x17];
                for (uint32_t k = 0; k < 4; k++) {
                    ev.hudSlotByMode[k] = comodData[evOff + 0x18 + k];
                }
                ev.hudSlot2 = comodData[evOff + 0x1C];
                ev.byte1D = comodData[evOff + 0x1D];
                ev.byte1E = comodData[evOff + 0x1E];
                ev.byte1F = comodData[evOff + 0x1F];
                ev.textId = comodData[evOff + 0x20];
                ev.tail21_23[0] = comodData[evOff + 0x21];
                ev.tail21_23[1] = comodData[evOff + 0x22];
                ev.tail21_23[2] = comodData[evOff + 0x23];
                stream.events.push_back(ev);
            }
        }

        out.streams.push_back(std::move(stream));
    }

    return true;
}

} // namespace

const std::string* PrStage1OverlayData::FindMemHandleName(uint16_t handle) const {
    if (handle >= memHandleNames.size()) {
        return nullptr;
    }
    if (memHandleNames[handle].empty()) {
        return nullptr;
    }
    return &memHandleNames[handle];
}

const PrStage1PairEntry* PrStage1OverlayData::FindPairEntry(size_t index) const {
    if (index >= pairTable.size()) {
        return nullptr;
    }
    return &pairTable[index];
}

const PrStage1TextTable* PrStage1OverlayData::FindTextTable(size_t index) const {
    if (index >= textTables.size()) {
        return nullptr;
    }
    return &textTables[index];
}

const PrStage1HudSlotDesc* PrStage1OverlayData::FindHudSlot(uint8_t slotId) const {
    if (slotId >= hudSlotDescs.size()) {
        return nullptr;
    }
    return &hudSlotDescs[slotId];
}

const PrStage1HudAnimEvent* PrStage1OverlayData::FindHudSlotEvent(uint8_t slotId, size_t eventIndex) const {
    const PrStage1HudSlotDesc* slot = FindHudSlot(slotId);
    if (!slot || eventIndex >= slot->events.size()) {
        return nullptr;
    }
    return &slot->events[eventIndex];
}

const PrStage1HudAnimEvent* PrStage1OverlayData::FindHudSlotInitialEvent(uint8_t slotId) const {
    return FindHudSlotEvent(slotId, 0);
}

const PrStage1CompactRailRec44* PrStage1OverlayData::FindCompactRailRec44(
    uint8_t recordIndex) const {
    const PrStage1CompactRailRec44& rec = compactRailRec44[recordIndex];
    return rec.valid ? &rec : nullptr;
}

const PrStage1HudOverlayConfigure801CA0E0Row*
PrStage1OverlayData::FindHudOverlayConfigure801CA0E0Row(
    uint8_t rowIndex) const {
    if (rowIndex >= hudOverlayConfigureRows801CA0E0.size()) {
        return nullptr;
    }
    const PrStage1HudOverlayConfigure801CA0E0Row& row =
        hudOverlayConfigureRows801CA0E0[rowIndex];
    return row.valid ? &row : nullptr;
}

const PrStage1EventStream* PrStage1OverlayData::FindStream(uint8_t streamId) const {
    for (const auto& stream : streams) {
        if (stream.streamId == streamId) {
            return &stream;
        }
    }
    return nullptr;
}

const PrStage1ScriptEvent* PrStage1OverlayData::FindLatestEvent(uint8_t streamId, uint32_t frame) const {
    const PrStage1EventStream* stream = FindStream(streamId);
    if (!stream || stream->events.empty()) {
        return nullptr;
    }

    const PrStage1ScriptEvent* best = nullptr;
    for (const auto& ev : stream->events) {
        if (ev.frame > frame) {
            break;
        }
        best = &ev;
    }
    return best;
}

const std::string* PrStage1OverlayData::ResolveText(uint8_t tableIndex, uint8_t textId) const {
    const PrStage1TextTable* table = FindTextTable(tableIndex);
    if (!table) {
        return nullptr;
    }
    if ((size_t)textId >= table->decodedText.size()) {
        return nullptr;
    }
    return &table->decodedText[(size_t)textId];
}

bool PrStage1OverlayData::FindFirstEventWithSlot1Mode(size_t modeIndex,
                                                      uint8_t& outStreamId,
                                                      size_t& outEventIndex,
                                                      const PrStage1ScriptEvent*& outEvent) const {
    outStreamId = 0;
    outEventIndex = 0;
    outEvent = nullptr;
    if (modeIndex >= 4) {
        return false;
    }

    for (const auto& stream : streams) {
        for (size_t i = 0; i < stream.events.size(); i++) {
            const PrStage1ScriptEvent& ev = stream.events[i];
            if (ev.hudSlotByMode[modeIndex] == 0) {
                continue;
            }
            outStreamId = stream.streamId;
            outEventIndex = i;
            outEvent = &ev;
            return true;
        }
    }
    return false;
}

bool PrStage1OverlayData::ResolveHudSlotInitial(uint8_t slotId, PrStage1ResolvedHudSlot& out) const {
    return ResolveHudSlotAtElapsed(slotId, 0, out);
}

bool PrStage1OverlayData::ResolveHudSlotAtElapsed(uint8_t slotId, uint32_t elapsedFrames, PrStage1ResolvedHudSlot& out) const {
    out = PrStage1ResolvedHudSlot{};
    const PrStage1HudSlotDesc* slot = FindHudSlot(slotId);
    if (!slot || slot->events.empty()) {
        return false;
    }

    size_t activeIndex = 0;
    uint32_t activeFrame = 0;
    bool any = false;

    for (size_t i = 0; i < slot->events.size(); i++) {
        const PrStage1HudAnimEvent& ev = slot->events[i];
        uint32_t fireFrame = ev.deltaFrames;
        if (any) {
            const uint32_t minNextFrame = activeFrame + 1;
            if (fireFrame < minNextFrame) {
                fireFrame = minNextFrame;
            }
        }
        if (fireFrame > elapsedFrames) {
            break;
        }
        activeIndex = i;
        activeFrame = fireFrame;
        any = true;
    }

    if (!any) {
        return false;
    }

    const PrStage1HudAnimEvent& ev = slot->events[activeIndex];
    out.valid = true;
    out.slotId = slotId;
    out.eventCount = slot->eventCount;
    out.activeEventIndex = (uint32_t)activeIndex;
    out.activeFrame = activeFrame;
    out.timIds = ev.timIds;
    for (size_t i = 0; i < ev.timIds.size(); i++) {
        out.memNames[i] = FindMemHandleName(ev.timIds[i]);
    }
    return true;
}

bool PrStage1OverlayData::ResolveTextEvent(uint8_t streamId,
                                           const PrStage1ScriptEvent& ev,
                                           uint8_t lang,
                                           uint32_t dueFrame,
                                           uint32_t elapsedFrames,
                                           PrStage1ResolvedTextEvent& out) const {
    out = PrStage1ResolvedTextEvent{};
    out.valid = true;
    out.streamId = streamId;
    out.eventPsxAddr = ev.psxAddr;
    out.eventFrame = ev.frame;
    out.dueFrame = dueFrame;
    out.eventByte16 = ev.byte16;
    out.textId = ev.textId;
    out.text = ResolveText(lang, ev.textId);
    ResolveHudSlotAtElapsed(ev.hudSlot0, elapsedFrames, out.slot0);
    for (size_t i = 0; i < ev.hudSlotByMode.size(); i++) {
        ResolveHudSlotAtElapsed(ev.hudSlotByMode[i], elapsedFrames, out.slot1ByMode[i]);
    }
    ResolveHudSlotAtElapsed(ev.hudSlot2, elapsedFrames, out.slot2);
    return true;
}

bool PrStage1OverlayData::FindFirstResolvedTextEvent(uint8_t streamId, uint8_t lang, PrStage1ResolvedTextEvent& out) const {
    out = PrStage1ResolvedTextEvent{};
    const PrStage1EventStream* stream = FindStream(streamId);
    if (!stream) {
        return false;
    }

    for (const auto& ev : stream->events) {
        if (ev.textId == 0) {
            continue;
        }
        const std::string* text = ResolveText(lang, ev.textId);
        if (!text || text->empty()) {
            continue;
        }
        return ResolveTextEvent(streamId, ev, lang, ev.frame, 0, out);
    }
    return false;
}

bool PrStage1OverlayData::ResolveFirstSlot1ModePreview(size_t modeIndex,
                                                       uint8_t lang,
                                                       uint32_t elapsedFrames,
                                                       uint8_t& outStreamId,
                                                       size_t& outEventIndex,
                                                       PrStage1ResolvedTextEvent& out) const {
    outStreamId = 0;
    outEventIndex = 0;
    out = PrStage1ResolvedTextEvent{};

    const PrStage1ScriptEvent* ev = nullptr;
    if (!FindFirstEventWithSlot1Mode(modeIndex, outStreamId, outEventIndex, ev) || !ev) {
        return false;
    }
    return ResolveTextEvent(outStreamId, *ev, lang, ev->frame, elapsedFrames, out);
}

bool PrStage1OverlayData::ResolveParserTextDebugInfo(const ParserTextDebugInfo& info,
                                                     uint8_t lang,
                                                     uint32_t currentStageFrame,
                                                     PrStage1ResolvedTextEvent& out) const {
    out = PrStage1ResolvedTextEvent{};
    if (!info.valid) {
        return false;
    }
    const PrStage1EventStream* stream = FindStream(info.streamId);
    if (!stream) {
        return false;
    }
    for (const auto& ev : stream->events) {
        if (ev.psxAddr != info.eventPsxAddr) {
            continue;
        }
        const uint32_t elapsedFrames = (currentStageFrame > info.dueFrame) ? (currentStageFrame - info.dueFrame) : 0u;
        return ResolveTextEvent(info.streamId, ev, lang, info.dueFrame, elapsedFrames, out);
    }
    return false;
}

bool PrStage1OverlayParser::ParseFromFiles(const std::string& comodPath,
                                           const std::string& compoPath,
                                           PrStage1OverlayData& out) {
    std::vector<uint8_t> comodBytes;
    if (!ReadFileBytes(comodPath, comodBytes)) {
        return false;
    }

    IntArchive archive;
    if (!IntLoader::Load(compoPath, archive)) {
        Log::Printf("PrStage1OverlayParser: IntLoader failed: %s", compoPath.c_str());
        return false;
    }

    return ParseFromBytes(comodBytes.data(), comodBytes.size(), archive, out);
}

bool PrStage1OverlayParser::ParseFromBytes(const uint8_t* comodData,
                                           size_t comodSize,
                                           const IntArchive& compo,
                                           PrStage1OverlayData& out) {
    out = PrStage1OverlayData{};
    if (!comodData || comodSize < 0x1000) {
        return false;
    }

    if (!BuildMemHandleMap(compo, out.memHandleNames)) {
        Log::Printf("PrStage1OverlayParser: no MEM handles built from COMPO archive");
        return false;
    }

    if (!ParseTextTables(comodData, comodSize, out)) {
        Log::Printf("PrStage1OverlayParser: ParseTextTables failed");
        return false;
    }
    if (!ParsePairTable(comodData, comodSize, out)) {
        Log::Printf("PrStage1OverlayParser: ParsePairTable failed");
        return false;
    }
    if (!ParseStreamDescRows(comodData, comodSize, out)) {
        Log::Printf("PrStage1OverlayParser: ParseStreamDescRows failed");
        return false;
    }
    if (!ParseCompactRailRec44(comodData, comodSize, out)) {
        Log::Printf("PrStage1OverlayParser: ParseCompactRailRec44 failed");
        return false;
    }
    if (!ParseHudOverlayConfigure801CA0E0Rows(comodData, comodSize, out)) {
        Log::Printf("PrStage1OverlayParser: ParseHudOverlayConfigure801CA0E0Rows failed");
        return false;
    }
    if (!ParseHudSlotDescs(comodData, comodSize, out)) {
        Log::Printf("PrStage1OverlayParser: ParseHudSlotDescs failed");
        return false;
    }
    if (!ParseEventStreams(comodData, comodSize, out)) {
        Log::Printf("PrStage1OverlayParser: ParseEventStreams failed");
        return false;
    }

    out.valid = true;
    Log::Printf("PrStage1OverlayParser: parsed textTables=%zu pairTable=%zu streamRows=%zu hudOverlayConfigureRows=%zu hudSlots=%zu streams=%zu",
                out.textTables.size(),
                out.pairTable.size(),
                out.streamDescRows.size(),
                out.hudOverlayConfigureRows801CA0E0.size(),
                out.hudSlotDescs.size(),
                out.streams.size());
    return true;
}
