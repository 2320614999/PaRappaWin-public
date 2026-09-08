#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace PrSS0TitleInitialClockDirect {

static constexpr uint32_t kSceneIndex801C49C0 = 0u;
static constexpr uint32_t kScene0Base8005474C = 0x8005474Cu;
static constexpr uint32_t kContextAddress801C3640 = 0x801C3640u;
static constexpr uint32_t kHeaderField08Offset801C49C0 = 0x08u;
static constexpr uint32_t kHeaderField0AOffset801C49D0 = 0x0Au;
static constexpr int16_t kHeaderField08Value801C49C0 = 96;
static constexpr int16_t kHeaderField0AValue801C49D0 = 0;
static constexpr uint32_t kCleanupFunction8001A280 = 0x8001A280u;
static constexpr uint32_t kAudioResetFunction80026FA4 = 0x80026FA4u;
static constexpr uint32_t kCtx0CAddress801C364C = 0x801C364Cu;
static constexpr uint32_t kCtx60Address801C36A0 = 0x801C36A0u;
static constexpr uint32_t kCtx54Address801C3694 = 0x801C3694u;
static constexpr uint32_t kInitialTick96Value801C49DC = 96u;
static constexpr std::size_t kStepCount801C49C0 = 7u;

struct Source801C49C0 {
    bool sceneIndexKnown = false;
    uint32_t sceneIndex = 0u;
    bool scene0BaseKnown = false;
    uint32_t scene0BaseAddress = 0u;
    bool contextKnown = false;
    uint32_t contextAddress = 0u;
    bool headerField08Known = false;
    int16_t headerField08 = 0;
    bool headerField0AKnown = false;
    int16_t headerField0A = 0;
};

struct CleanupAuthorityInput8001A280 {
    bool known = false;
    bool called = false;
    bool dword80049428Known = false;
    int32_t dword80049428 = 0;
    bool skippedNonZeroWorkBase = false;
    bool commandWrapperCalled80036678 = false;
    uint8_t command80036678 = 0u;
    bool commandWrapperSucceeded80036678 = false;
    bool commandSinkCalled800375BC = false;
    bool commandSinkSkipWait800375BC = false;
    bool directCommandSinkAuthority = false;
    bool hardwareCallbackAuthority = false;
    bool hostProjection = false;
    bool replayValueAuthority = false;
    bool oldWinS0Authority = false;
    bool stage2PlusAuthority = false;
    bool comod2Authority = false;
};

enum class Step801C49C0 : uint8_t {
    None = 0,
    Call8001A280SourceOpen,
    ResolveScene0Header08Plus0A,
    Call80026FA4,
    WriteCtx0CIn80026FA4DelaySlot,
    SetCursor0,
    WriteCtx60Value1,
    WriteCtx54Value0,
};

struct Result801C49C0 {
    bool accepted = false;
    bool complete = false;

    uint32_t sceneIndex = 0u;
    uint32_t scene0BaseAddress = 0u;
    int16_t headerField08 = 0;
    int16_t headerField0A = 0;

    bool initialTick96Known = false;
    uint32_t initialTick96 = 0u;
    uint32_t contextAddress = 0u;
    uint32_t ctx0CAddress = 0u;
    uint32_t ctx0CValue = 0u;

    uint32_t cleanup8001A280CallAddress = 0u;
    bool cleanup8001A280SourceOpen = false;
    bool cleanup8001A280Authority = false;
    bool cleanupDword80049428Known = false;
    int32_t cleanupDword80049428 = 0;
    bool cleanupSkippedNonZeroWorkBase = false;
    bool cleanupCommandIssued80036678 = false;
    uint8_t cleanupCommand80036678 = 0u;
    bool cleanupCommandSinkAuthority800375BC = false;
    uint32_t call80026FA4Address = 0u;
    bool ctx0CWriteIn80026FA4DelaySlot = false;

    bool cursorKnown = false;
    int32_t cursor = 0;
    uint32_t ctx60Address = 0u;
    uint32_t ctx60Value = 0u;
    uint32_t ctx54Address = 0u;
    uint32_t ctx54Value = 0u;

    bool hostProjection = false;
    bool staticScene0HeaderAuthority = false;
    bool psxMemoryBackingAuthority = false;
    bool stage1StateOrHeaderAuthority = false;
    bool oldWinS0Authority = false;
    bool stage2PlusAuthority = false;
    bool replayValueAuthority = false;
    bool hostFilesystemAuthority = false;
    bool gameLiveProbeAuthority = false;

    std::size_t stepCount = 0u;
    std::array<Step801C49C0, kStepCount801C49C0> stepOrder{};
};

Source801C49C0 BuildKnownStaticScene0Source801C49C0();

bool ResolveStaticHeaderInitialClock801C49C0(
    const Source801C49C0& source,
    Result801C49C0& out);
bool BindCleanupAuthority8001A280(
    const CleanupAuthorityInput8001A280& input,
    Result801C49C0& inOut);

} // namespace PrSS0TitleInitialClockDirect
