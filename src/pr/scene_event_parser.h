#pragma once
#include <cstdint>
#include <vector>
#include <string>

// PSX Scene Event - extracted from COMOD*.BIN overlay files
// Each event is 16 bytes matching PSX struct PrStageScriptEvent16:
// {frame(4), flags(4), b8(1), b9(1), b10(1), b11(1), b12(1), b13(1), b14(1), b15(1)}
struct SceneEvent {
    uint32_t frame;     // Global frame number when this event triggers
    uint32_t flags;     // Bit flags controlling which fields are active
    uint8_t  b8;        // TIM index for mouth texture (always checked if non-zero)
    uint8_t  b9;        // VDF animation index (flag 0x40000) — NOT a TIM
    uint8_t  b10;       // TIM pair lookup index (flag 0x10000) — indexes timPairTable
    uint8_t  b11;       // HUD overlay table selector (1-based, 0=none)
    uint8_t  b12;       // Direct TIM index (flag 0x800000)
    uint8_t  b13;       // TIM pair lookup index (flag 0x400000)
    uint8_t  b14;       // TIM pair lookup index (flag 0x1000000)
    uint8_t  b15;       // reserved
};

// TIM pair lookup entry — PSX unk_801C6C14 (main program resident data)
// Used by b10, b13, b14 fields to resolve two TIM indices per lookup index
struct TimPairEntry {
    int16_t timA;   // First TIM index into mem file table (0 = unused)
    int16_t timB;   // Second TIM index into mem file table (0 = unused)
};

// HUD Overlay Table Entry — 12 bytes in PSX
// {frameOffset(4), timIndices[4] as int16}
struct HudOverlayEntry {
    uint32_t frameOffset;       // Frames after slot activation to trigger
    int16_t  timIndices[4];     // TIM indices into mem file table (0-terminated)
};

// HUD Overlay Slot
struct HudOverlaySlot {
    std::vector<HudOverlayEntry> entries;
};

// Complete parsed scene event data
struct SceneEventData {
    std::vector<SceneEvent> events;         // Parsed from COMOD BIN
    HudOverlaySlot overlaySlots[6];         // PSX main program (unk_801C6D4C)
    std::vector<TimPairEntry> timPairTable; // PSX main program (unk_801C6C14)
    bool valid = false;
};

class SceneEventParser {
public:
    // Parse Scene0 face timeline from COMOD*.BIN.
    // overlay/TIM pair tables: prefer BIN extraction, fallback to JSON.
    // jsonPath: fallback path to psx_face_tables.json (can be empty when not used)
    static bool Parse(const uint8_t* binData, size_t binSize,
                      const std::string& jsonPath, SceneEventData& out);

    // Find PSX base address from BIN data
    static uint32_t FindBaseAddress(const uint8_t* binData, size_t binSize);

private:
    // Parse event stream from BIN
    static bool ParseEventStream(const uint8_t* binData, size_t binSize,
                                  uint32_t psxBase, uint32_t descAddr,
                                  SceneEventData& out);

    // Fallback loader: overlay slots and TIM pair table from JSON
    static bool LoadPsxTables(const std::string& jsonPath, SceneEventData& out);
};
