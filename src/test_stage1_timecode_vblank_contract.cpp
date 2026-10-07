#include "pr/pr_stage_runner_direct.h"
#include "pr/pr_psx_vsync_direct.h"
#include "pr/pr_psx_vblank_callback_direct.h"
#include "pr/pr_stage1_terminal_presentation_direct.h"
#include "xa1_player.h"
#include <cstdio>

// Native COMOD1 801C7A60 / 801C7560 and SCUS 80035EAC contract.
// Uses real translated arithmetic and interrupt dispatch. Synthetic CD input,
// not a live runner test, emulator replay, or proof of host callback wiring.
namespace {
namespace V = PrPsxVblankCallbackDirect;
namespace C = PrPsxVSyncDirect;
constexpr uint32_t kTimecode = 0x801C7560u;
int failures = 0;
#define CHECK(value) do { if (!(value)) { \
    std::printf("FAIL line %d: %s\n", __LINE__, #value); ++failures; } } while (0)

struct Probe {
    PrStageRunnerDirectTimecodeInput801C7560 input{};
    unsigned calls = 0;
    Probe() {
        input.sceneEntryField348 = 18000;
        input.sceneEntryField352FallbackTickAdvance = 7;
        input.sceneEntryField356TickOffset = 14;
        // 7A60 resets previous XA to zero before preroll, then seeds ctx+0C
        // immediately before registration. Registration is not an invocation.
        input.state.tick801C364C = input.sceneEntryField356TickOffset;
    }
    static void Invoke(void* user) {
        auto& probe = *static_cast<Probe*>(user);
        probe.input.state = PrStageRunnerDirectUpdateTimecode801C7560(probe.input).state;
        ++probe.calls;
    }
    void Register() {
        CHECK(V::VSyncCallback800357D4({kTimecode, Invoke, this}).known);
        CHECK(calls == 0);
        CHECK(input.state.tick801C364C == 14);
    }
};

int RunOneSecond(int deliverySize) {
    C::ResetProcessVSyncState80035E54();
    auto& clock = C::ProcessVSyncState80035560();
    Probe probe;
    probe.Register();
    for (int elapsed = 0; elapsed < 60; elapsed += deliverySize)
        C::AdvanceHostVblankClock80035EAC(clock, deliverySize);
    CHECK(probe.calls == 60);
    CHECK(clock.vblankCounter80057034 == 60);
    CHECK(probe.input.state.tick801C364C == 434);
    V::VSyncCallback800357D4({}); // no dangling test callback
    return probe.input.state.tick801C364C;
}

void CheckRatesAndInlineCounterexample() {
    const int native30 = RunOneSecond(2);
    const int native60 = RunOneSecond(1);
    CHECK(native30 == native60);
    Probe inline30;
    for (int frame = 0; frame < 30; ++frame) Probe::Invoke(&inline30);
    CHECK(inline30.input.state.tick801C364C == 224);
    CHECK(inline30.input.state.tick801C364C != native30);
    std::printf("COUNTEREXAMPLE synthetic stalled XA: native 60 VBlanks=%d, "
                "inline 30 logic calls=%d (not a measured live offset)\n",
                native30, inline30.input.state.tick801C364C);
}

void CheckCdReadBranches() {
    C::ResetProcessVSyncState80035E54();
    auto& clock = C::ProcessVSyncState80035560();
    Probe probe;
    probe.Register();
    const int sectors[] = {0, 3, 3, 2, 5};
    const int ticks[] = {21, 26, 33, 40, 34};
    for (int i = 0; i < 5; ++i) {
        probe.input.xaSectorReadValueA7A4 = sectors[i];
        C::AdvanceHostVblankClock80035EAC(clock, 1);
        CHECK(probe.input.state.tick801C364C == ticks[i]);
        CHECK(probe.input.state.dword801D303CPreviousXaReadValue == sectors[i]);
    }
    // Native latest-sector rounding may move backward after fallback; do not
    // introduce a monotonic clamp or scale fallback by the render rate.
    V::VSyncCallback800357D4({});
}

void CheckTerminalLifetimeAndReentry() {
    C::ResetProcessVSyncState80035E54();
    auto& clock = C::ProcessVSyncState80035560();
    Probe probe;
    probe.Register();
    unsigned unrelatedCalls = 0;
    V::Exchange80035F24(V::ProcessSlots80057014(), 1,
        {0x80000004u, +[](void* user) { ++*static_cast<unsigned*>(user); }, &unrelatedCalls});
    C::AdvanceHostVblankClock80035EAC(clock, 2);
    const auto beforeTail = probe.calls;
    for (int frame = 0; frame < 4; ++frame) {
        C::BeginVSync80035560(clock, 2);
        const auto result = C::AdvanceHostVblankClock80035EAC(clock, 2);
        CHECK(result.softwareWaitComplete);
    }
    CHECK(probe.calls == beforeTail + 8);
    const int tickBeforeClear = probe.input.state.tick801C364C;
    const auto clockBeforeClear = clock.vblankCounter80057034;
    const auto removed = V::VSyncCallback800357D4({});
    CHECK(removed.known && removed.previous == kTimecode);
    CHECK(clock.vblankCounter80057034 == clockBeforeClear);
    C::AdvanceHostVblankClock80035EAC(clock, 3);
    CHECK(probe.input.state.tick801C364C == tickBeforeClear);
    CHECK(probe.calls == beforeTail + 8);
    CHECK(unrelatedCalls == clock.vblankCounter80057034);
    Probe retry;
    retry.Register();
    C::AdvanceHostVblankClock80035EAC(clock, 1);
    CHECK(retry.calls == 1 && retry.input.state.tick801C364C == 21);
    CHECK(probe.calls == beforeTail + 8);
    C::ResetProcessVSyncState80035E54();
}

void CheckSingleExternalClockThroughTerminal() {
    namespace T = PrStage1TerminalPresentationDirect;
    C::ResetProcessVSyncState80035E54();
    auto& clock = C::ProcessVSyncState80035560();
    Probe probe;
    probe.Register();
    T::State terminal{};
    uint64_t host = 0;
    for (unsigned frame = 0; frame < 4; ++frame) {
        CHECK(T::Submit(terminal, static_cast<uint8_t>(frame & 1), host, true, false, clock));
        host += 2;
        C::AdvanceHostVblankClock80035EAC(clock, 2); // main-loop sole delivery
        CHECK(T::AdvanceWait(terminal, clock, host, true)); // render observes
        CHECK(probe.calls == host); // no second dispatch from AdvanceWait
        CHECK(T::AdvanceWait(terminal, clock, host, true)); // duplicate draw
        CHECK(probe.calls == host);
        CHECK(T::Publish(terminal, frame));
        CHECK(T::Poll(terminal, frame + 1));
    }
    // The next logic step can clear the callback before drawing; its arriving
    // VBlank still must be delivered, not lost at the render->logic handoff.
    C::AdvanceHostVblankClock80035EAC(clock, 1);
    ++host;
    CHECK(probe.calls == 9);
    CHECK(V::VSyncCallback800357D4({}).previous == kTimecode);
    const auto delivered = clock.vblankCounter80057034;
    T::AdvanceWait(terminal, clock, host, true);
    CHECK(clock.vblankCounter80057034 == delivered);
    // Once unbound on a fresh pending wait, terminal's fallback can own that
    // fresh elapsed tick. It may not replay the earlier clear-frame receipt.
    terminal = {};
    CHECK(T::Submit(terminal, 0, host, true, false, clock));
    T::AdvanceWait(terminal, clock, ++host, false);
    CHECK(clock.vblankCounter80057034 == delivered + 1);
    CHECK(probe.calls == 9);
    C::ResetProcessVSyncState80035E54();
}

void CheckCdTransportRate() {
    double normal30 = 0, native60 = 0;
    for (int i = 0; i < 30; ++i) normal30 += Xa1Player::CdSectorsForVblanks60(2);
    for (int i = 0; i < 60; ++i) native60 += Xa1Player::CdSectorsForVblanks60(1);
    CHECK(normal30 == 75.0 && native60 == 75.0);
    CHECK(Xa1Player::CdSectorsForVblanks60(0) == 0.0);
}
}

int main() {
    CheckRatesAndInlineCounterexample();
    CheckCdReadBranches();
    CheckTerminalLifetimeAndReentry();
    CheckSingleExternalClockThroughTerminal();
    CheckCdTransportRate();
    std::printf("Stage1 native VBlank/timecode contract: %s; live host binding NOT verified\n",
                failures ? "FAIL" : "PASS");
    return failures ? 1 : 0;
}
