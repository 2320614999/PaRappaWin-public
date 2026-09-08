#include "pr/pr_ss0_title_initial_clock_direct.h"

#include <array>
#include <cstdio>

namespace {

using namespace PrSS0TitleInitialClockDirect;

int g_failed = 0;

#define CHECK(expr)                                                           \
    do {                                                                      \
        if (!(expr)) {                                                        \
            std::printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr);       \
            ++g_failed;                                                       \
        }                                                                     \
    } while (0)

bool SameSource(const Source801C49C0& left, const Source801C49C0& right)
{
    return left.sceneIndexKnown == right.sceneIndexKnown &&
           left.sceneIndex == right.sceneIndex &&
           left.scene0BaseKnown == right.scene0BaseKnown &&
           left.scene0BaseAddress == right.scene0BaseAddress &&
           left.contextKnown == right.contextKnown &&
           left.contextAddress == right.contextAddress &&
           left.headerField08Known == right.headerField08Known &&
           left.headerField08 == right.headerField08 &&
           left.headerField0AKnown == right.headerField0AKnown &&
           left.headerField0A == right.headerField0A;
}

bool SameResult(const Result801C49C0& left, const Result801C49C0& right)
{
    return left.accepted == right.accepted &&
           left.complete == right.complete &&
           left.sceneIndex == right.sceneIndex &&
           left.scene0BaseAddress == right.scene0BaseAddress &&
           left.headerField08 == right.headerField08 &&
           left.headerField0A == right.headerField0A &&
           left.initialTick96Known == right.initialTick96Known &&
           left.initialTick96 == right.initialTick96 &&
           left.contextAddress == right.contextAddress &&
           left.ctx0CAddress == right.ctx0CAddress &&
           left.ctx0CValue == right.ctx0CValue &&
           left.cleanup8001A280CallAddress ==
               right.cleanup8001A280CallAddress &&
           left.cleanup8001A280SourceOpen ==
               right.cleanup8001A280SourceOpen &&
           left.cleanup8001A280Authority ==
               right.cleanup8001A280Authority &&
           left.cleanupDword80049428Known ==
               right.cleanupDword80049428Known &&
           left.cleanupDword80049428 ==
               right.cleanupDword80049428 &&
           left.cleanupSkippedNonZeroWorkBase ==
               right.cleanupSkippedNonZeroWorkBase &&
           left.cleanupCommandIssued80036678 ==
               right.cleanupCommandIssued80036678 &&
           left.cleanupCommand80036678 ==
               right.cleanupCommand80036678 &&
           left.cleanupCommandSinkAuthority800375BC ==
               right.cleanupCommandSinkAuthority800375BC &&
           left.call80026FA4Address == right.call80026FA4Address &&
           left.ctx0CWriteIn80026FA4DelaySlot ==
               right.ctx0CWriteIn80026FA4DelaySlot &&
           left.cursorKnown == right.cursorKnown &&
           left.cursor == right.cursor &&
           left.ctx60Address == right.ctx60Address &&
           left.ctx60Value == right.ctx60Value &&
           left.ctx54Address == right.ctx54Address &&
           left.ctx54Value == right.ctx54Value &&
           left.hostProjection == right.hostProjection &&
           left.staticScene0HeaderAuthority ==
               right.staticScene0HeaderAuthority &&
           left.psxMemoryBackingAuthority ==
               right.psxMemoryBackingAuthority &&
           left.stage1StateOrHeaderAuthority ==
               right.stage1StateOrHeaderAuthority &&
           left.oldWinS0Authority == right.oldWinS0Authority &&
           left.stage2PlusAuthority == right.stage2PlusAuthority &&
           left.replayValueAuthority == right.replayValueAuthority &&
           left.hostFilesystemAuthority == right.hostFilesystemAuthority &&
           left.gameLiveProbeAuthority == right.gameLiveProbeAuthority &&
           left.stepCount == right.stepCount &&
           left.stepOrder == right.stepOrder;
}

void ExpectDefault(const Result801C49C0& result)
{
    CHECK(SameResult(result, Result801C49C0{}));
}

void ExpectReject(const Source801C49C0& source)
{
    Result801C49C0 result{};
    CHECK(ResolveStaticHeaderInitialClock801C49C0(
        BuildKnownStaticScene0Source801C49C0(), result));
    CHECK(result.accepted);
    CHECK(!ResolveStaticHeaderInitialClock801C49C0(source, result));
    ExpectDefault(result);
}

void TestKnownSourceBuilder()
{
    const Source801C49C0 source =
        BuildKnownStaticScene0Source801C49C0();
    CHECK(source.sceneIndexKnown);
    CHECK(source.sceneIndex == kSceneIndex801C49C0);
    CHECK(source.scene0BaseKnown);
    CHECK(source.scene0BaseAddress == kScene0Base8005474C);
    CHECK(source.contextKnown);
    CHECK(source.contextAddress == kContextAddress801C3640);
    CHECK(source.headerField08Known);
    CHECK(source.headerField08 == kHeaderField08Value801C49C0);
    CHECK(source.headerField0AKnown);
    CHECK(source.headerField0A == kHeaderField0AValue801C49D0);
    CHECK(SameSource(source, BuildKnownStaticScene0Source801C49C0()));
}

void TestRejectsEveryUnknownAndMismatch()
{
    ExpectReject(Source801C49C0{});

    Source801C49C0 source = BuildKnownStaticScene0Source801C49C0();
    source.sceneIndexKnown = false;
    ExpectReject(source);

    source = BuildKnownStaticScene0Source801C49C0();
    source.sceneIndex = 1u;
    ExpectReject(source);

    source = BuildKnownStaticScene0Source801C49C0();
    source.scene0BaseKnown = false;
    ExpectReject(source);

    source = BuildKnownStaticScene0Source801C49C0();
    source.scene0BaseAddress = kScene0Base8005474C + 4u;
    ExpectReject(source);

    source = BuildKnownStaticScene0Source801C49C0();
    source.contextKnown = false;
    ExpectReject(source);

    source = BuildKnownStaticScene0Source801C49C0();
    source.contextAddress = kContextAddress801C3640 + 4u;
    ExpectReject(source);

    source = BuildKnownStaticScene0Source801C49C0();
    source.headerField08Known = false;
    ExpectReject(source);

    source = BuildKnownStaticScene0Source801C49C0();
    source.headerField08 = 95;
    ExpectReject(source);

    source = BuildKnownStaticScene0Source801C49C0();
    source.headerField0AKnown = false;
    ExpectReject(source);

    source = BuildKnownStaticScene0Source801C49C0();
    source.headerField0A = 1;
    ExpectReject(source);
}

void TestExactResolutionAndAuthorityBoundary()
{
    Result801C49C0 result{};
    CHECK(ResolveStaticHeaderInitialClock801C49C0(
        BuildKnownStaticScene0Source801C49C0(), result));
    CHECK(result.accepted);
    CHECK(result.complete);
    CHECK(result.sceneIndex == 0u);
    CHECK(result.scene0BaseAddress == 0x8005474Cu);
    CHECK(result.headerField08 == 96);
    CHECK(result.headerField0A == 0);
    CHECK(result.initialTick96Known);
    CHECK(result.initialTick96 == 96u);
    CHECK(result.contextAddress == 0x801C3640u);
    CHECK(result.ctx0CAddress == 0x801C364Cu);
    CHECK(result.ctx0CValue == 96u);

    CHECK(result.cleanup8001A280CallAddress == 0x8001A280u);
    CHECK(result.cleanup8001A280SourceOpen);
    CHECK(!result.cleanup8001A280Authority);
    CleanupAuthorityInput8001A280 cleanup{};
    cleanup.known = true;
    cleanup.called = true;
    cleanup.dword80049428Known = true;
    cleanup.dword80049428 = 0;
    cleanup.commandWrapperCalled80036678 = true;
    cleanup.command80036678 = 0x10u;
    cleanup.commandWrapperSucceeded80036678 = true;
    cleanup.commandSinkCalled800375BC = true;
    cleanup.commandSinkSkipWait800375BC = true;
    cleanup.directCommandSinkAuthority = true;
    CHECK(BindCleanupAuthority8001A280(cleanup, result));
    CHECK(!result.cleanup8001A280SourceOpen);
    CHECK(result.cleanup8001A280Authority);
    CHECK(result.cleanupDword80049428Known);
    CHECK(result.cleanupDword80049428 == 0);
    CHECK(!result.cleanupSkippedNonZeroWorkBase);
    CHECK(result.cleanupCommandIssued80036678);
    CHECK(result.cleanupCommand80036678 == 0x10u);
    CHECK(result.cleanupCommandSinkAuthority800375BC);
    CHECK(result.call80026FA4Address == 0x80026FA4u);
    CHECK(result.ctx0CWriteIn80026FA4DelaySlot);
    CHECK(result.cursorKnown);
    CHECK(result.cursor == 0);
    CHECK(result.ctx60Address == 0x801C36A0u);
    CHECK(result.ctx60Value == 1u);
    CHECK(result.ctx54Address == 0x801C3694u);
    CHECK(result.ctx54Value == 0u);

    CHECK(result.hostProjection);
    CHECK(result.staticScene0HeaderAuthority);
    CHECK(!result.psxMemoryBackingAuthority);
    CHECK(!result.stage1StateOrHeaderAuthority);
    CHECK(!result.oldWinS0Authority);
    CHECK(!result.stage2PlusAuthority);
    CHECK(!result.replayValueAuthority);
    CHECK(!result.hostFilesystemAuthority);
    CHECK(!result.gameLiveProbeAuthority);

    const std::array<Step801C49C0, kStepCount801C49C0> expectedOrder = {
        Step801C49C0::Call8001A280SourceOpen,
        Step801C49C0::ResolveScene0Header08Plus0A,
        Step801C49C0::Call80026FA4,
        Step801C49C0::WriteCtx0CIn80026FA4DelaySlot,
        Step801C49C0::SetCursor0,
        Step801C49C0::WriteCtx60Value1,
        Step801C49C0::WriteCtx54Value0,
    };
    CHECK(result.stepCount == expectedOrder.size());
    CHECK(result.stepOrder == expectedOrder);

    Result801C49C0 skipped{};
    CHECK(ResolveStaticHeaderInitialClock801C49C0(
        BuildKnownStaticScene0Source801C49C0(), skipped));
    cleanup = CleanupAuthorityInput8001A280{};
    cleanup.known = true;
    cleanup.called = true;
    cleanup.dword80049428Known = true;
    cleanup.dword80049428 = 1;
    cleanup.skippedNonZeroWorkBase = true;
    cleanup.directCommandSinkAuthority = true;
    CHECK(BindCleanupAuthority8001A280(cleanup, skipped));
    CHECK(!skipped.cleanup8001A280SourceOpen);
    CHECK(skipped.cleanup8001A280Authority);
    CHECK(skipped.cleanupSkippedNonZeroWorkBase);
    CHECK(!skipped.cleanupCommandIssued80036678);
    CHECK(skipped.cleanupCommand80036678 == 0u);

    Result801C49C0 rejected{};
    CHECK(ResolveStaticHeaderInitialClock801C49C0(
        BuildKnownStaticScene0Source801C49C0(), rejected));
    cleanup.hostProjection = true;
    CHECK(!BindCleanupAuthority8001A280(cleanup, rejected));
    CHECK(!rejected.cleanup8001A280Authority);
}

void TestDeterministicIdempotentResolution()
{
    const Source801C49C0 source =
        BuildKnownStaticScene0Source801C49C0();
    Result801C49C0 first{};
    Result801C49C0 second{};
    CHECK(ResolveStaticHeaderInitialClock801C49C0(source, first));
    CHECK(ResolveStaticHeaderInitialClock801C49C0(source, second));
    CHECK(SameResult(first, second));

    second.ctx0CValue = 0u;
    second.hostProjection = false;
    CHECK(ResolveStaticHeaderInitialClock801C49C0(source, second));
    CHECK(SameResult(first, second));
}

} // namespace

int main()
{
    TestKnownSourceBuilder();
    TestRejectsEveryUnknownAndMismatch();
    TestExactResolutionAndAuthorityBoundary();
    TestDeterministicIdempotentResolution();
    if (g_failed != 0) {
        std::printf("test_ss0_title_initial_clock_direct: %d failed\n",
                    g_failed);
        return 1;
    }
    std::printf("test_ss0_title_initial_clock_direct: ok\n");
    return 0;
}
