#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace PrSS0TitleEntryPrefixDirect {

static constexpr uint32_t kContextAddress801C3640 = 0x801C3640u;
static constexpr uint32_t kScene0TablePointer801C6DA4 = 0x801C6DA4u;
static constexpr std::size_t kStepCount801C4894 = 5u;
static constexpr std::size_t kMaxOperationsPerStep801C4894 = 26u;

struct Source801C4894 {
    bool contextKnown = false;
    uint32_t contextAddress = 0u;
    bool scene0TablePointerKnown = false;
    uint32_t scene0TablePointer = 0u;
    bool word800916DCKnown = false;
    uint16_t word800916DC = 0u;
};

enum class Step801C4894 : uint8_t {
    None = 0,
    Call80014344,
    Call80024E98,
    Call80024FC0,
    Call801C4FA0,
    Call80024C84,
};

enum class OperationKind801C4894 : uint8_t {
    None = 0,
    ZeroRange,
    WordWrite,
    DwordWrite,
    GpDwordWrite,
};

struct Operation801C4894 {
    OperationKind801C4894 kind = OperationKind801C4894::None;
    // Memory operations use an address; GP operations use a GP-relative offset.
    uint32_t target = 0u;
    uint32_t rangeEnd = 0u;
    uint32_t byteCount = 0u;
    uint32_t value = 0u;
};

struct Call801C4894 {
    Step801C4894 step = Step801C4894::None;
    uint32_t functionAddress = 0u;
    bool argumentKnown = false;
    uint32_t argument = 0u;
    bool staticReturnKnown = false;
    int32_t staticReturn = 0;
    std::size_t operationCount = 0u;
    std::array<Operation801C4894, kMaxOperationsPerStep801C4894>
        operations{};
};

struct Transaction801C4894 {
    bool accepted = false;
    bool complete = false;
    bool hostProjection = false;
    bool psxMemoryBackingAuthority = false;
    bool stage1StateOrHeaderAuthority = false;
    bool oldWinS0Authority = false;
    bool stage2PlusAuthority = false;
    bool replayValueAuthority = false;
    bool hostFilesystemAuthority = false;
    bool gameLiveProbeAuthority = false;
    std::size_t stepCount = 0u;
    std::array<Step801C4894, kStepCount801C4894> stepOrder{};
    std::array<Call801C4894, kStepCount801C4894> calls{};
};

bool BuildTransaction801C4894(const Source801C4894& source,
                              Transaction801C4894& out);

} // namespace PrSS0TitleEntryPrefixDirect
