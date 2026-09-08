#pragma once

#include "pr_tmd.h"

#include <cstddef>
#include <cstdint>

namespace PrSS0TitleDrawDescDirect {

constexpr uint32_t kFn8001AF1C = 0x8001AF1Cu;
constexpr uint32_t kFn8004274C = 0x8004274Cu;
constexpr uint32_t kFn800428B0 = 0x800428B0u;
constexpr uint32_t kPaDrawDescAddress801CB65C = 0x801CB65Cu;
constexpr uint32_t kPaCoordAddress801CB608 = 0x801CB608u;
constexpr uint32_t kLoDrawDescAddress801CB5F0 = 0x801CB5F0u;
constexpr uint32_t kLoCoordAddress801CB5A0 = 0x801CB5A0u;
constexpr uint32_t kHpDrawDescAddress801CC70C = 0x801CC70Cu;
constexpr uint32_t kHpCoordAddress801CC6BC = 0x801CC6BCu;
constexpr uint32_t kPaKageDrawDescAddress801CD80C = 0x801CD80Cu;
constexpr uint32_t kPaKageCoordAddress801CD76C = 0x801CD76Cu;

constexpr uint32_t kMode25ObjectEntryOffset = 0x0Cu;
constexpr uint32_t kMode25FirstPrimitiveOffset = 0x28u;
constexpr uint32_t kMode25ObjectCount = 1u;
constexpr int32_t kMode25ObjectScale = 0;
constexpr uint8_t kMode25Mode = 0x25u;
constexpr uint8_t kMode25Flag = 0x01u;
constexpr uint8_t kMode25Ilen = 6u;
constexpr uint8_t kMode25Olen = 7u;
constexpr uint8_t kMode25WordStride8004274C = 7u;
constexpr uint32_t kMode25PacketBytes = 28u;

constexpr uint32_t kPaVertexCount = 79u;
constexpr uint32_t kPaPrimitiveCount = 190u;
constexpr uint32_t kLoVertexCount = 60u;
constexpr uint32_t kLoPrimitiveCount = 176u;
constexpr uint32_t kHpVertexCount = 27u;
constexpr uint32_t kHpPrimitiveCount = 64u;
constexpr uint32_t kPaKageVertexCount = 79u;
constexpr uint32_t kPaKagePrimitiveCount = 190u;

constexpr uint32_t kPaKageObjectEntryOffset = kMode25ObjectEntryOffset;
constexpr uint32_t kPaKageFirstPrimitiveOffset =
    kMode25FirstPrimitiveOffset;
constexpr uint32_t kPaKageObjectCount = kMode25ObjectCount;
constexpr uint8_t kPaKageMode = kMode25Mode;
constexpr uint8_t kPaKageFlag = kMode25Flag;
constexpr uint8_t kPaKageIlen = kMode25Ilen;
constexpr uint8_t kPaKageOlen = kMode25Olen;
constexpr uint8_t kPaKageWordStride8004274C =
    kMode25WordStride8004274C;
constexpr uint32_t kPaKagePacketBytes = kMode25PacketBytes;

// Retained for the current PA/PA_KAGE consumers, whose shapes are identical.
constexpr uint32_t kMode25VertexCount = kPaKageVertexCount;
constexpr uint32_t kMode25PrimitiveCount = kPaKagePrimitiveCount;

enum class ModelPath : uint8_t {
    Pa = 0,
    PaKage = 1,
    Lo = 2,
    Hp = 3,
};

enum class Failure : uint8_t {
    None = 0,
    SourceParseIncomplete,
    NullModel,
    WrongModelHeader,
    WrongObjectCount,
    WrongObjectShape,
    WrongPrimitiveShape,
};

struct PrimitiveGroup8004274C {
    bool known = false;
    uint32_t firstPrimitiveIndex = 0;
    uint16_t primitiveCount = 0;
    uint8_t mode = 0;
    uint8_t flag = 0;
    uint8_t wordStride = 0;
};

struct DrawAttr800428B0 {
    bool known = false;
    bool active = false;
    uint8_t bits0To2 = 0;
    uint8_t bits3To4 = 0;
    bool bit5 = false;
    bool bit6 = false;
    uint8_t bits9To11 = 0;
    bool bit30 = false;
};

struct RuntimeState8001AF1C {
    bool initialized = false;
    uint32_t drawDescAddress = 0;
    bool modelBindingKnown = false;
    const TmdModel* model = nullptr;
    ModelPath modelPath = ModelPath::Pa;
    bool attrKnown = false;
    uint32_t attr = 0;
    bool coordBindingKnown = false;
    uint32_t coordAddress = 0;
    bool objectBindingKnown = false;
    uint32_t objectEntryOffset = 0;
    uint32_t objectIndex = 0;
    const TmdObject* object = nullptr;
    PrimitiveGroup8004274C primitiveGroup{};
};

struct InitializeResult8001AF1C {
    Failure failure = Failure::None;
    bool initialized = false;
};

void Clear(RuntimeState8001AF1C& state);

InitializeResult8001AF1C InitializePa8001AF1C(
    RuntimeState8001AF1C& state,
    const TmdModel* model,
    bool sourceParseComplete);

InitializeResult8001AF1C InitializeLo8001AF1C(
    RuntimeState8001AF1C& state,
    const TmdModel* model,
    bool sourceParseComplete);

InitializeResult8001AF1C InitializeHp8001AF1C(
    RuntimeState8001AF1C& state,
    const TmdModel* model,
    bool sourceParseComplete);

InitializeResult8001AF1C InitializePaKage8001AF1C(
    RuntimeState8001AF1C& state,
    const TmdModel* model,
    bool sourceParseComplete);

DrawAttr800428B0 DecodeAttr800428B0(
    const RuntimeState8001AF1C& state);

bool IsMode25PrimitiveReady800428B0(
    const RuntimeState8001AF1C& state,
    const TmdObject* expectedObject,
    std::size_t primitiveIndex);

bool IsFirstPaKagePrimitiveReady800428B0(
    const RuntimeState8001AF1C& state,
    const TmdObject* expectedObject,
    std::size_t primitiveIndex);

bool IsFirstLoPrimitiveReady800428B0(
    const RuntimeState8001AF1C& state,
    const TmdObject* expectedObject,
    std::size_t primitiveIndex);

bool IsFirstHpPrimitiveReady800428B0(
    const RuntimeState8001AF1C& state,
    const TmdObject* expectedObject,
    std::size_t primitiveIndex);

} // namespace PrSS0TitleDrawDescDirect
