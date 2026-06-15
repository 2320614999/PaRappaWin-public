#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

struct PrCardSlotInfo {
    bool occupied = false;
    std::string title;
};

struct PrCardEntryInfo {
    std::string title;
    int blockIndex = -1; // physical memcard directory block, 0-based [0..14]
};

struct PrCard {
    static void Init();
    static void Shutdown();

    static bool Load(int slot);
    static bool Save(int slot);
    static bool LoadFromMemCardEntry(int blockIndex);
    static bool SaveToMemCardEntry(int blockIndex);

    static uint8_t* Payload();
    static const uint8_t* PayloadConst();
    static size_t PayloadSize();

    static bool EnumerateSlots(PrCardSlotInfo outSlots[15]);
    static int EnumerateEntriesCompact(PrCardEntryInfo outEntries[15]);

    static void SetSuffix(const char* suffix);
    static const char* GetSuffix();
};
