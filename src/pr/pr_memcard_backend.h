#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

namespace PrMemCardBackend {

struct EntryInfo {
    std::string title;
    std::string internalName;
    int blockIndex = -1;
};

struct SlotInfo {
    bool occupied = false;
    std::string title;
    std::string internalName;
};

struct EntryWriteResult {
    bool attempted = false;
    bool saved = false;
    bool overwrote = false;
    int blockIndex = -1;
};

struct EntryWritePreflightResult {
    bool requestValid = false;
    bool blockBytesValid = false;
    bool imageReadable = false;
    bool writable = false;
    bool overwrote = false;
    int blockIndex = -1;
};

struct ReplayEntryLoadResult {
    bool entryLoaded = false;
    bool payloadCommitted = false;
    int replayScene = -1;
    bool rawSavedSlotKnown = false;
    uint32_t rawSavedSlot = 0;
};

uint32_t ReadPayloadU32(std::size_t offset);
uint8_t ReadStageStatus(int stage);
const uint8_t* CurrentPayload();
uint8_t* MutablePayload();
std::size_t PayloadSize();
int EnumerateEntriesCompact(EntryInfo outEntries[15]);
bool EnumerateSlots(SlotInfo outSlots[15]);
bool FormatPrimaryCard();
bool LoadEntry(int blockIndex);
bool SaveEntry(int blockIndex);
EntryWritePreflightResult CanSaveEntryBlockByName(const char* internalName,
                                                  const uint8_t* saveBlock8K,
                                                  std::size_t saveBlockSize);
EntryWriteResult SaveEntryBlockByName(const char* internalName,
                                      const uint8_t* saveBlock8K,
                                      std::size_t saveBlockSize);
bool LoadEntryPayloadToBlock(int blockIndex,
                             uint8_t* outBlock,
                             std::size_t outBlockSize,
                             std::size_t payloadOffset);
ReplayEntryLoadResult LoadReplayEntry(int blockIndex);

} // namespace PrMemCardBackend
