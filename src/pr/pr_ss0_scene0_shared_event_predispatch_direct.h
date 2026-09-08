#pragma once

#include "pr_ss0_scene0_global_bindings_direct.h"

#include <array>
#include <cstdint>

namespace PrSS0Scene0SharedEventPredispatchDirect {

constexpr uint32_t kCallsite801C4C9C = 0x801C4C9Cu;
constexpr uint32_t kCallsite801C4D14 = 0x801C4D14u;
constexpr uint32_t kRangeBegin800250E4 = 0x800250E4u;
constexpr uint32_t kRangeEnd800251D4 = 0x800251D4u;
constexpr uint32_t kNextUnclaimed800251D8 = 0x800251D8u;
constexpr uint32_t kFirstDispatchBranch800251F0 = 0x800251F0u;
constexpr uint32_t kDispatchRangeBegin800251D8 = 0x800251D8u;
constexpr uint32_t kDispatchRangeEnd800259B8 = 0x800259B8u;
constexpr uint32_t kDispatchKey0 = 0u;
constexpr uint32_t kDispatchKey30 = 30u;
constexpr uint32_t kDispatchKey31 = 31u;
constexpr uint32_t kRingBase80092910 = 0x80092910u;
constexpr uint32_t kRingPageBytes80014BDC = 384u;
constexpr uint32_t kRingPageCount80014BDC = 4u;

struct ContextState80024FD0 {
    bool resetKnown = false;
    bool ctxDword56Known = false;
    uint32_t ctxDword56 = 0u;
    bool ctxDword64PsxAddressKnown = false;
    uint32_t ctxDword64PsxAddress = 0u;
    bool ctxDword68PsxAddressKnown = false;
    uint32_t ctxDword68PsxAddress = 0u;
};

struct Input80024FD0 {
    bool callsiteKnown = false;
    uint32_t callsite = 0u;
    bool ctxTick96Known = false;
    uint32_t ctxTick96 = 0u;
    bool word800916D0Known = false;
    uint16_t word800916D0 = 0u;
    bool prefixAppliedKnown = false;
    bool prefixApplied = false;
    bool bucketChangedKnown = false;
    bool bucketChanged = false;
    bool bucketKnown = false;
    uint32_t bucket = 0u;
    bool ctxFlagsKnown = false;
    uint32_t ctxFlags = 0u;
    PrSS0Scene0GlobalBindingsDirect::State801C5B14 globalBindings{};
};

enum class Status80024FD0 : uint8_t {
    SourceUnknown = 0,
    UnsupportedCallsite,
    TickOutOfRange,
    PrefixMismatch,
    GlobalBindingMismatch,
    ContextStateMismatch,
    TableRowAuthorityRequired,
    Applied,
};

struct Result80024FD0 {
    Status80024FD0 status = Status80024FD0::SourceUnknown;
    bool accepted = false;
    bool applied = false;
    bool stoppedBeforeDispatch = false;
    bool tableRowDereferenceRequired = false;
    bool tableRowDereferenceAttempted = false;
    bool ctxDword56Written = false;
    bool ctxDword64Written = false;
    bool ctxDword68Written = false;
    bool dword8008ED08Written = false;
    bool ctxFlagsWritten = false;
    uint32_t rowIndexV7 = 0u;
    uint32_t ctxDword56V6 = 0u;
    bool nextCtxFlagsKnown = false;
    uint32_t nextCtxFlags = 0u;
    ContextState80024FD0 nextContext{};

    bool psxMemoryBackingAuthority = false;
    bool oldWinS0Authority = false;
    bool stage1Authority = false;
    bool stage2PlusAuthority = false;
    bool replayValueAuthority = false;
    bool hostFilesystemAuthority = false;
};

struct StaticDispatchState80024FD0 {
    bool resetKnown = false;
    bool dword8008ED00Known = false;
    uint32_t dword8008ED00 = 0u;
    bool dword8008ED08Known = false;
    uint32_t dword8008ED08 = 0u;
    bool dword8008ED14Known = false;
    uint32_t dword8008ED14 = 0u;
    bool stageEventStreamIdKnown = false;
    uint16_t stageEventStreamId = 0u;
    bool word80091816Known = false;
    uint16_t word80091816 = 0u;
    bool ctxDword48Known = false;
    uint32_t ctxDword48 = 0u;
    bool ctxWord92Known = false;
    uint16_t ctxWord92 = 0u;
    bool ctxWord94Known = false;
    uint16_t ctxWord94 = 0u;
    bool currentRingPagePsxAddressKnown = false;
    uint32_t currentRingPagePsxAddress = 0u;
    std::array<bool, kRingPageCount80014BDC> ringPageKnownZero{};
};

struct StaticDispatchInput80024FD0 {
    bool callsiteKnown = false;
    uint32_t callsite = 0u;
    bool ctxTick96Known = false;
    uint32_t ctxTick96 = 0u;
    bool bucketKnown = false;
    uint32_t bucket = 0u;
    bool predispatchAppliedKnown = false;
    bool predispatchApplied = false;
    bool predispatchStoppedBeforeDispatchKnown = false;
    bool predispatchStoppedBeforeDispatch = false;
    bool noRowDereferenceKnown = false;
    bool noRowDereference = false;
    bool ctxDword56Known = false;
    uint32_t ctxDword56 = 0u;
    PrSS0Scene0GlobalBindingsDirect::State801C5B14 globalBindings{};
};

enum class StaticDispatchKey80024FD0 : uint8_t {
    NoMatch = 0,
    Key0,
    Key30,
    Key31,
};

enum class StaticDispatchStatus80024FD0 : uint8_t {
    SourceUnknown = 0,
    UnsupportedCallsite,
    BucketOutOfRange,
    PredispatchMismatch,
    GlobalBindingMismatch,
    StateOutsideScene0ZeroRowPath,
    Applied,
};

struct StaticDispatchResult80024FD0 {
    StaticDispatchStatus80024FD0 status =
        StaticDispatchStatus80024FD0::SourceUnknown;
    StaticDispatchKey80024FD0 key = StaticDispatchKey80024FD0::NoMatch;
    bool accepted = false;
    bool applied = false;
    bool completedThrough800259B8 = false;
    bool helper80024BF4Called = false;
    bool helper80024BF4ResultKnown = false;
    bool helper80024BF4Result = false;
    bool helper80024FC0Called = false;
    bool helper80014BDCCalled = false;
    bool ctxDword48Written = false;
    bool ctxWord92Written = false;
    bool currentRingPageWritten = false;
    bool ringPageCleared = false;
    uint8_t ringPageIndex = 0u;
    uint32_t ringPagePsxAddress = 0u;
    StaticDispatchState80024FD0 nextState{};

    bool psxMemoryBackingAuthority = false;
    bool oldWinS0Authority = false;
    bool stage1Authority = false;
    bool stage2PlusAuthority = false;
    bool replayValueAuthority = false;
    bool hostFilesystemAuthority = false;
};

void ResetContextState80024E98(ContextState80024FD0& state);
Result80024FD0 ApplyPredispatch800250E4To800251D4(
    const ContextState80024FD0& state,
    const Input80024FD0& input);
void ResetStaticDispatchState80014344To80024E98(
    StaticDispatchState80024FD0& state);
bool ApplyStaticDispatchReset80024E98(
    StaticDispatchState80024FD0& state);
StaticDispatchResult80024FD0 ApplyStaticScene0Dispatch800251D8To800259B8(
    const StaticDispatchState80024FD0& state,
    const StaticDispatchInput80024FD0& input);

} // namespace PrSS0Scene0SharedEventPredispatchDirect
