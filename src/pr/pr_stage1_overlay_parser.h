#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

struct IntArchive;
struct ParserTextDebugInfo;

// Stage1 overlay parser that powers Win-side parsing of COMOD1.BIN / COMPO01.INT.
// It now runs alongside the rest of the runtime, so parsed overlay data is consumed directly.

struct PrStage1PairEntry {
    int16_t idA = 0;
    int16_t idB = 0;
};

struct PrStage1StreamDescRow {
    std::array<uint8_t, 8> bytes{};
};

struct PrStage1CompactRailRec44 {
    bool valid = false;
    uint8_t recordIndex = 0;
    uint32_t psxAddr = 0;
    int16_t head0 = 0;
    uint16_t head1 = 0;
    std::array<int8_t, 20> streamA{};
    std::array<int8_t, 20> streamB{};
};

struct PrStage1HudAnimEvent {
    uint32_t deltaFrames = 0;
    std::array<uint16_t, 4> timIds{};
};

struct PrStage1ResolvedHudSlot {
    bool valid = false;
    uint8_t slotId = 0;
    uint32_t eventCount = 0;
    uint32_t activeEventIndex = 0;
    uint32_t activeFrame = 0;
    std::array<uint16_t, 4> timIds{};
    std::array<const std::string*, 4> memNames{};
};

struct PrStage1HudSlotDesc {
    uint8_t slotId = 0;
    uint32_t psxAddr = 0;
    uint32_t eventsPtr = 0;
    uint32_t eventCount = 0;
    uint32_t cursorInit = 0;
    std::vector<PrStage1HudAnimEvent> events;
};

struct PrStage1HudOverlayConfigure801CA0E0Row {
    bool valid = false;
    uint8_t rowIndex = 0;
    uint32_t psxAddr = 0;
    int16_t paDatResourceIndex = 0;
    int16_t paVdfResourceIndex = 0;
    int16_t timerFrames = 0;
    int16_t ctx11CValue = 0;
    int16_t hudSlotId = 0;
};

struct PrStage1TextTable {
    uint8_t tableIndex = 0;
    uint32_t psxAddr = 0;
    std::vector<uint32_t> entryPtrs;
    std::vector<std::string> decodedText;
};

struct PrStage1ScriptEvent {
    uint32_t psxAddr = 0;
    uint32_t frame = 0;
    uint32_t flags04 = 0;
    std::array<int16_t, 4> bezIdByMode{};
    std::array<uint8_t, 4> pairByMode{};
    uint8_t coolPair = 0;
    uint8_t noncoolPair = 0;
    uint8_t byte16 = 0;
    uint8_t hudSlot0 = 0;
    std::array<uint8_t, 4> hudSlotByMode{};
    uint8_t hudSlot2 = 0;
    uint8_t byte1D = 0;
    uint8_t byte1E = 0;
    uint8_t byte1F = 0;
    uint8_t textId = 0;
    std::array<uint8_t, 3> tail21_23{};
};

struct PrStage1EventStream {
    uint8_t streamId = 0;
    uint32_t psxAddr = 0;
    uint32_t eventsPtr = 0;
    uint32_t count = 0;
    uint32_t cursorInit = 0;
    std::vector<PrStage1ScriptEvent> events;
};

struct PrStage1ResolvedTextEvent {
    bool valid = false;
    uint8_t streamId = 0;
    uint32_t eventPsxAddr = 0;
    uint32_t eventFrame = 0;
    uint32_t dueFrame = 0;
    uint8_t eventByte16 = 0;
    uint8_t textId = 0;
    const std::string* text = nullptr;
    PrStage1ResolvedHudSlot slot0;
    std::array<PrStage1ResolvedHudSlot, 4> slot1ByMode{};
    PrStage1ResolvedHudSlot slot2;
};

struct PrStage1OverlayData {
    static constexpr uint32_t kPsxBase = 0x801C3870u;
    static constexpr uint32_t kTextTablePtrsAddr = 0x801CE804u;
    static constexpr uint32_t kPairTableAddr = 0x801CE818u;
    static constexpr uint32_t kStreamDescAddr = 0x801CE8D4u;
    static constexpr uint32_t kCompactRailRec44BaseAddr = 0x801CFA54u;
    static constexpr uint32_t kHudSlotDescAddr = 0x801CF924u;
    static constexpr uint32_t kHudOverlayConfigureTableAddr = 0x801D2DD0u;
    static constexpr uint32_t kStreamTableAddr = 0x801D2D64u;
    static constexpr uint32_t kTextTableCount = 5u;
    static constexpr uint32_t kPairTableCount =
        (kStreamDescAddr - kPairTableAddr) / 4u;
    static constexpr uint32_t kStreamDescRowCount = 9u;
    static constexpr uint32_t kStreamCount = 8u;
    static constexpr uint32_t kHudSlotCount = 25u;
    static constexpr uint32_t kStreamEntrySize = 0x0Cu;
    static constexpr uint32_t kScriptEventSize = 0x24u;
    static constexpr uint32_t kHudAnimEventSize = 0x0Cu;
    static constexpr uint32_t kCompactRailRec44Size = 0x2Cu;
    static constexpr uint32_t kHudOverlayConfigureRowCount = 9u;
    static constexpr uint32_t kHudOverlayConfigureRowSize = 0x0Au;
    static constexpr uint16_t kFirstMemHandle = 2u;

    uint32_t psxBase = kPsxBase;
    std::array<PrStage1TextTable, 5> textTables{};
    std::vector<PrStage1PairEntry> pairTable;
    std::array<PrStage1StreamDescRow, 9> streamDescRows{};
    std::array<PrStage1CompactRailRec44, 256> compactRailRec44{};
    std::array<PrStage1HudOverlayConfigure801CA0E0Row, 9>
        hudOverlayConfigureRows801CA0E0{};
    std::vector<PrStage1HudSlotDesc> hudSlotDescs;
    std::vector<PrStage1EventStream> streams;
    std::vector<std::string> memHandleNames;
    bool valid = false;

    const std::string* FindMemHandleName(uint16_t handle) const;
    const PrStage1PairEntry* FindPairEntry(size_t index) const;
    const PrStage1TextTable* FindTextTable(size_t index) const;
    const PrStage1HudSlotDesc* FindHudSlot(uint8_t slotId) const;
    const PrStage1HudAnimEvent* FindHudSlotEvent(uint8_t slotId, size_t eventIndex) const;
    const PrStage1HudAnimEvent* FindHudSlotInitialEvent(uint8_t slotId) const;
    const PrStage1CompactRailRec44* FindCompactRailRec44(uint8_t recordIndex) const;
    const PrStage1HudOverlayConfigure801CA0E0Row*
    FindHudOverlayConfigure801CA0E0Row(uint8_t rowIndex) const;
    const PrStage1EventStream* FindStream(uint8_t streamId) const;
    const PrStage1ScriptEvent* FindLatestEvent(uint8_t streamId, uint32_t frame) const;
    const std::string* ResolveText(uint8_t tableIndex, uint8_t textId) const;
    bool FindFirstEventWithSlot1Mode(size_t modeIndex,
                                     uint8_t& outStreamId,
                                     size_t& outEventIndex,
                                     const PrStage1ScriptEvent*& outEvent) const;
    bool ResolveHudSlotInitial(uint8_t slotId, PrStage1ResolvedHudSlot& out) const;
    bool ResolveHudSlotAtElapsed(uint8_t slotId, uint32_t elapsedFrames, PrStage1ResolvedHudSlot& out) const;
    bool ResolveTextEvent(uint8_t streamId,
                          const PrStage1ScriptEvent& ev,
                          uint8_t lang,
                          uint32_t dueFrame,
                          uint32_t elapsedFrames,
                          PrStage1ResolvedTextEvent& out) const;
    bool FindFirstResolvedTextEvent(uint8_t streamId, uint8_t lang, PrStage1ResolvedTextEvent& out) const;
    bool ResolveFirstSlot1ModePreview(size_t modeIndex,
                                      uint8_t lang,
                                      uint32_t elapsedFrames,
                                      uint8_t& outStreamId,
                                      size_t& outEventIndex,
                                      PrStage1ResolvedTextEvent& out) const;
    bool ResolveParserTextDebugInfo(const ParserTextDebugInfo& info,
                                    uint8_t lang,
                                    uint32_t currentStageFrame,
                                    PrStage1ResolvedTextEvent& out) const;
};

class PrStage1OverlayParser {
public:
    static bool ParseFromFiles(const std::string& comodPath,
                               const std::string& compoPath,
                               PrStage1OverlayData& out);

    static bool ParseFromBytes(const uint8_t* comodData,
                               size_t comodSize,
                               const IntArchive& compo,
                               PrStage1OverlayData& out);

};
