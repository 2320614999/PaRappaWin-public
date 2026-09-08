#include "pr_ss0_title_initial_clock_direct.h"

namespace PrSS0TitleInitialClockDirect {

Source801C49C0 BuildKnownStaticScene0Source801C49C0()
{
    Source801C49C0 source{};
    source.sceneIndexKnown = true;
    source.sceneIndex = kSceneIndex801C49C0;
    source.scene0BaseKnown = true;
    source.scene0BaseAddress = kScene0Base8005474C;
    source.contextKnown = true;
    source.contextAddress = kContextAddress801C3640;
    source.headerField08Known = true;
    source.headerField08 = kHeaderField08Value801C49C0;
    source.headerField0AKnown = true;
    source.headerField0A = kHeaderField0AValue801C49D0;
    return source;
}

bool ResolveStaticHeaderInitialClock801C49C0(
    const Source801C49C0& source,
    Result801C49C0& out)
{
    out = Result801C49C0{};
    if (!source.sceneIndexKnown ||
        source.sceneIndex != kSceneIndex801C49C0 ||
        !source.scene0BaseKnown ||
        source.scene0BaseAddress != kScene0Base8005474C ||
        !source.contextKnown ||
        source.contextAddress != kContextAddress801C3640 ||
        !source.headerField08Known ||
        source.headerField08 != kHeaderField08Value801C49C0 ||
        !source.headerField0AKnown ||
        source.headerField0A != kHeaderField0AValue801C49D0) {
        return false;
    }

    const int32_t initialTick96 =
        static_cast<int32_t>(source.headerField08) +
        static_cast<int32_t>(source.headerField0A);
    if (initialTick96 !=
        static_cast<int32_t>(kInitialTick96Value801C49DC)) {
        return false;
    }

    Result801C49C0 result{};
    result.accepted = true;
    result.complete = true;
    result.sceneIndex = source.sceneIndex;
    result.scene0BaseAddress = source.scene0BaseAddress;
    result.headerField08 = source.headerField08;
    result.headerField0A = source.headerField0A;
    result.initialTick96Known = true;
    result.initialTick96 = static_cast<uint32_t>(initialTick96);
    result.contextAddress = source.contextAddress;
    result.ctx0CAddress = kCtx0CAddress801C364C;
    result.ctx0CValue = static_cast<uint32_t>(initialTick96);
    result.cleanup8001A280CallAddress = kCleanupFunction8001A280;
    result.cleanup8001A280SourceOpen = true;
    result.cleanup8001A280Authority = false;
    result.call80026FA4Address = kAudioResetFunction80026FA4;
    result.ctx0CWriteIn80026FA4DelaySlot = true;
    result.cursorKnown = true;
    result.cursor = 0;
    result.ctx60Address = kCtx60Address801C36A0;
    result.ctx60Value = 1u;
    result.ctx54Address = kCtx54Address801C3694;
    result.ctx54Value = 0u;
    result.hostProjection = true;
    result.staticScene0HeaderAuthority = true;
    result.psxMemoryBackingAuthority = false;
    result.stage1StateOrHeaderAuthority = false;
    result.oldWinS0Authority = false;
    result.stage2PlusAuthority = false;
    result.replayValueAuthority = false;
    result.hostFilesystemAuthority = false;
    result.gameLiveProbeAuthority = false;
    result.stepCount = kStepCount801C49C0;
    result.stepOrder = {
        Step801C49C0::Call8001A280SourceOpen,
        Step801C49C0::ResolveScene0Header08Plus0A,
        Step801C49C0::Call80026FA4,
        Step801C49C0::WriteCtx0CIn80026FA4DelaySlot,
        Step801C49C0::SetCursor0,
        Step801C49C0::WriteCtx60Value1,
        Step801C49C0::WriteCtx54Value0,
    };
    out = result;
    return true;
}

bool BindCleanupAuthority8001A280(
    const CleanupAuthorityInput8001A280& input,
    Result801C49C0& inOut)
{
    if (!inOut.accepted ||
        !inOut.complete ||
        !inOut.cleanup8001A280SourceOpen ||
        inOut.cleanup8001A280CallAddress != kCleanupFunction8001A280 ||
        !input.known ||
        !input.called ||
        !input.dword80049428Known ||
        !input.directCommandSinkAuthority ||
        input.hardwareCallbackAuthority ||
        input.hostProjection ||
        input.replayValueAuthority ||
        input.oldWinS0Authority ||
        input.stage2PlusAuthority ||
        input.comod2Authority) {
        return false;
    }

    const bool skippedBranch =
        input.dword80049428 != 0 &&
        input.skippedNonZeroWorkBase &&
        !input.commandWrapperCalled80036678 &&
        !input.commandSinkCalled800375BC;
    const bool commandBranch =
        input.dword80049428 == 0 &&
        !input.skippedNonZeroWorkBase &&
        input.commandWrapperCalled80036678 &&
        input.command80036678 == 0x10u &&
        input.commandWrapperSucceeded80036678 &&
        input.commandSinkCalled800375BC &&
        input.commandSinkSkipWait800375BC;
    if (!skippedBranch && !commandBranch) {
        return false;
    }

    inOut.cleanup8001A280SourceOpen = false;
    inOut.cleanup8001A280Authority = true;
    inOut.cleanupDword80049428Known = true;
    inOut.cleanupDword80049428 = input.dword80049428;
    inOut.cleanupSkippedNonZeroWorkBase = skippedBranch;
    inOut.cleanupCommandIssued80036678 = commandBranch;
    inOut.cleanupCommand80036678 =
        commandBranch ? input.command80036678 : 0u;
    inOut.cleanupCommandSinkAuthority800375BC = true;
    return true;
}

} // namespace PrSS0TitleInitialClockDirect
