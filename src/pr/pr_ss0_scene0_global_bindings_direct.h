#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace PrSS0Scene0GlobalBindingsDirect {

constexpr uint32_t kFn801C5B14 = 0x801C5B14u;
constexpr uint32_t kFn801C4260 = 0x801C4260u;
constexpr std::size_t kInitCallCount801C4260 = 5u;

// Values are exact PSX address carriers; they are never host pointers.
struct State801C5B14 {
    bool known = false;
    bool complete = false;
    bool hostProjection = false;

    bool returnValueKnown = false;
    uint32_t returnValue = 0u;

    bool psxMemoryBackingAuthority = false;
    bool oldWinS0Authority = false;
    bool stage1Authority = false;
    bool stage2PlusAuthority = false;
    bool replayValueAuthority = false;
    bool hostFilesystemAuthority = false;

    uint32_t slot800943C0 = 0u;
    uint32_t slot800943C4 = 0u;
    uint32_t slot800943C8 = 0u;
    uint32_t slot800943CC = 0u;
    uint32_t slot800943D0 = 0u;
    uint32_t slot800943D4 = 0u;
    uint32_t slot800943D8 = 0u;
    uint32_t slot800943DC = 0u;
    uint32_t slot800943E0 = 0u;
    uint32_t slot800943E4 = 0u;
    uint32_t slot800943E8 = 0u;
    uint32_t slot800943EC = 0u;
    uint32_t slot800943F0 = 0u;
    uint32_t slot800943F4 = 0u;
    uint32_t slot800943F8 = 0u;
    uint32_t slot800943FC = 0u;
    uint32_t slot80094400 = 0u;
    uint32_t slot80094404 = 0u;
    uint32_t slot80094408 = 0u;
    uint32_t slot8009440C = 0u;
    uint32_t slot80094410 = 0u;
    uint32_t slot80094414 = 0u;
    uint32_t slot80094418 = 0u;
    uint32_t slot8009441C = 0u;
    uint32_t slot80094420 = 0u;
    uint32_t slot80094424 = 0u;
    uint32_t slot80094428 = 0u;
    uint32_t slot8009442C = 0u;
    uint32_t slot80094430 = 0u;
    uint32_t slot80094434 = 0u;
    uint32_t slot80094438 = 0u;
    uint32_t slot8009443C = 0u;
    uint32_t slot80094440 = 0u;
};

State801C5B14 BuildState801C5B14();
bool IsExactState801C5B14(const State801C5B14& state);

enum class InitStatus801C4260 : uint8_t {
    SourceUnknown = 0,
    MalformedSource,
    ResetUnavailable,
    ResourceIngressUnavailable,
    Accepted,
};

struct InitSource801C4260 {
    bool sceneIndexKnown = false;
    uint32_t sceneIndex = 0u;
    bool sceneEntryBaseKnown = false;
    uint32_t sceneEntryBase = 0u;
    bool sceneHeaderKnown = false;
    uint16_t bpm100 = 0u;
    int16_t tickOffset = 0;
    int16_t extraTick = 0;

    bool resetSourcesKnown = false;
    bool reset80025A34Ready = false;
    bool reset801C4FA0Ready = false;
    bool reset80024E98Ready = false;
    bool reset80014344Ready = false;

    bool resourceIngress801C4780Known = false;
    bool resourceIngress801C4780Ready = false;
};

struct InitTransaction801C4260 {
    InitStatus801C4260 status = InitStatus801C4260::SourceUnknown;
    bool accepted = false;
    bool complete = false;
    bool hostProjection = false;

    bool psxMemoryBackingAuthority = false;
    bool oldWinS0Authority = false;
    bool stage1Authority = false;
    bool stage2PlusAuthority = false;
    bool replayValueAuthority = false;
    bool hostFilesystemAuthority = false;

    std::size_t callCount = 0u;
    std::array<uint32_t, kInitCallCount801C4260> callOrder{};

    uint32_t sceneEntryPointerDestination = 0u;
    uint32_t sceneEntryPointerValue = 0u;

    uint32_t tickPerMinFieldAddress = 0u;
    uint32_t tickPerMinValue = 0u;
    uint32_t tickPerFrameFieldAddress = 0u;
    uint32_t tickPerFrameValue = 0u;
    uint32_t baseTickFieldAddress = 0u;
    uint32_t baseTickValue = 0u;
    uint32_t gridColumnsFieldAddress = 0u;
    uint32_t gridColumnsValue = 0u;

    bool timingPublishedBeforeResourceIngress = false;
    uint32_t preopenRowCount = 0u;
    uint32_t compoRowIndex = 0u;
    uint32_t resourceIngressLoadArg = 0u;
};

InitTransaction801C4260 BuildInitTransaction801C4260(
    const InitSource801C4260& source);
bool IsExactInitTransaction801C4260(
    const InitTransaction801C4260& transaction);

} // namespace PrSS0Scene0GlobalBindingsDirect
