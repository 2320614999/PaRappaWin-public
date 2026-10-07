#ifdef NDEBUG
#undef NDEBUG // Keep executable contract checks active in Release builds.
#endif
#include "pr/pr_stage1_xa_cd_direct.h"
#include "pr/pr_stage1_cd_stop_execution_direct.h"
#include <cassert>
#include <cstdio>
#include <memory>

using Phase = PrStage1CdStopPhase8001A694;
constexpr uint32_t callback = 0x8001A210u;

static void Seed(PrStage1XaCdDirectState& state) {
    state = {};
    state.dword_800570F8Known = true;
    state.dword_800570F8 = callback;
    state.streamClockCallback8001A210Registered = true;
    state.dword_80057108Known = true;
    state.dword_800570FCKnown = true;
    state.dword_800570FC = 0x80039240u;
    state.readS27Serial = 41;
    state.dword_80049428 = 123;
}

static PrStage1CdStopStep8001A694 Reply(PrStage1CdStopRuntime8001A694& run,
                                     PrStage1XaCdDirectState& state, int result) {
    return PrStage1XaCdDirectAdvanceStop8001A694(run, state, {true, run.request, result});
}

static void CheckUnrelated(const PrStage1XaCdDirectState& state) {
    assert(state.dword_800570FCKnown && state.dword_800570FC == 0x80039240u);
    assert(state.readS27Serial == 41 && state.dword_80049428 == 123);
}

static void CheckActualLowerExecution(PrStage1XaCdDirectState& cd) {
    namespace E = PrStage1CdStopExecutionDirect;
    struct Device {
        E::DeviceReply reply{};
        uint32_t submissions = 0;
        uint32_t polls = 0;
        uint8_t command = 0;
        uint64_t serial = 0;
    } device;
    const auto transport = +[](void* user, bool submit, uint8_t command, uint64_t serial) {
        auto& d = *static_cast<Device*>(user);
        if (submit) { ++d.submissions; d.command = command; d.serial = serial; }
        else { ++d.polls; assert(d.command == command && d.serial == serial); }
        return d.reply;
    };
    Seed(cd);
    PrStage1CdStopRuntime8001A694 run{};
    E::State executor{};
    PrStage1XaCdDirectAdvanceStop8001A694(run, cd);
    assert(!E::Execute(executor, cd, run.request, 10, nullptr, &device).known);
    assert(!E::Execute(executor, cd, run.request, 10, transport, &device).known);
    assert(cd.commandSerial == 0 && device.submissions == 0); // missing pre-sync
    cd.byte_800573D4Known = true;
    cd.byte_800573D4 = 2;
    cd.dword_80057108 = 0x22;
    for (int i = 0; i < 10; ++i) {
        assert(!E::Execute(executor, cd, run.request, 10 + i, transport, &device).known);
        assert(cd.dword_800570F8 == callback);
    }
    assert(device.submissions == 1 && cd.commandSerial == 1 && cd.lastCdCommand == 8);
    assert(cd.dword_80057108 == 0x22); // issue does not clear drive status
    auto wrong = run.request;
    wrong.args[2] ^= 4;
    assert(!E::Execute(executor, cd, wrong, 20, transport, &device).known);
    device.reply.acknowledged = true;
    auto feedback = E::Execute(executor, cd, run.request, 20, transport, &device);
    assert(feedback.known && feedback.result == 0);
    assert(cd.byte_800573D4 == 3); // command8 INT3 acknowledgement != completion
    assert(cd.response_80049414Known && cd.response_80049414[0] == 0);
    assert(E::Execute(executor, cd, run.request, 21, transport, &device).known);
    assert(device.submissions == 1);
    assert(PrStage1XaCdDirectAdvanceStop8001A694(run, cd, feedback).requestIssued);
    for (int i = 0; i < 10; ++i) {
        assert(!E::Execute(executor, cd, run.request, 22 + i, transport, &device).known);
        assert(cd.dword_800570F8 == callback);
    }
    device.reply.completed = true;
    feedback = E::Execute(executor, cd, run.request, 33, transport, &device);
    assert(feedback.known && feedback.result == 2 && cd.cdSync80037070.sourceFunction == 0x80037070u);
    assert(cd.cdSync80037070.copiedOutputBytes && cd.cdSync80037070.outputByteCount == 8);
    assert(cd.cdSyncCallback8001A210DispatchCount == 1);
    const auto eventSerial = cd.cdLowerEvent80036AF8Serial;
    assert(E::Execute(executor, cd, run.request, 34, transport, &device).known);
    assert(cd.cdLowerEvent80036AF8Serial == eventSerial); // no replayed IRQ/callback
    assert(PrStage1XaCdDirectAdvanceStop8001A694(run, cd, feedback).complete);
    assert(cd.dword_800570F8 == 0 && run.returnKnown && static_cast<uint32_t>(run.result) == callback);
    CheckUnrelated(cd);

    // Completion errors return5 to367A4 but37070 stores2 at573D4. Retry can
    // issue a new command without using the prior request's completion.
    Seed(cd); run = {}; executor = {}; device = {};
    cd.byte_800573D4Known = true; cd.byte_800573D4 = 2;
    device.reply = {true, 0u, true, 1u};
    PrStage1XaCdDirectAdvanceStop8001A694(run, cd);
    feedback = E::Execute(executor, cd, run.request, 0, transport, &device);
    PrStage1XaCdDirectAdvanceStop8001A694(run, cd, feedback);
    feedback = E::Execute(executor, cd, run.request, 0, transport, &device);
    assert(feedback.known && feedback.result == 5 && cd.byte_800573D4 == 2);
    assert(cd.response_80049414[0] == 1u);
    PrStage1XaCdDirectAdvanceStop8001A694(run, cd, feedback);
    assert(run.phase == Phase::StopCommand && run.calls800367A4 == 2);
    device.reply = {true, 0u, true, 0u};
    feedback = E::Execute(executor, cd, run.request, 1, transport, &device);
    assert(feedback.known && device.submissions == 2);
    PrStage1XaCdDirectAdvanceStop8001A694(run, cd, feedback);
    feedback = E::Execute(executor, cd, run.request, 1, transport, &device);
    assert(PrStage1XaCdDirectAdvanceStop8001A694(run, cd, feedback).complete);
    CheckUnrelated(cd);
}

static void CheckStageRecordPendingStatus(PrStage1XaCdDirectState& cd) {
    namespace E = PrStage1CdStopExecutionDirect;
    cd = {};
    PrStage1XaCdDirectStartInput start{};
    start.segPresent = true;
    const auto started = PrStage1XaCdDirectApplySub8001A4D0StageRecordTick(cd, start);
    assert(started.start.started);
    start.cdCommandCompletionKnown = true;
    start.cdCommandSyncResultKnown = true;
    start.cdCommandSyncResult = 2;
    const auto tick = PrStage1XaCdDirectApplySub8001A4D0StageRecordTick(cd, start);
    assert(tick.resultKnown && cd.lastCdCommand == 1 && cd.byte_800573D4 == 0);
    assert(!cd.dword_80057108Known); // no pre-seeded drive status
    const auto serial = cd.commandSerial;
    assert(!E::ApplyPendingStatusReceipt(cd, serial, false, 0x20));
    assert(!E::ApplyPendingStatusReceipt(cd, serial + 1, true, 0x20));
    assert(E::ApplyPendingStatusReceipt(cd, serial, true, 0x20));
    assert(cd.dword_80057108Known && cd.dword_80057108 == 0x20);
    assert(cd.byte_800573D4 == 2 && cd.commandSerial == serial);
    const auto eventSerial = cd.cdLowerEvent80036AF8Serial;
    assert(!E::ApplyPendingStatusReceipt(cd, serial, true, 0x20));
    assert(cd.cdLowerEvent80036AF8Serial == eventSerial);
    // Match the live frame order: publish actual sector header, consume the
    // clock, then1A280 issues command10 before terminal375BC's pre-sync.
    PrStage1XaCdDirectHalGetlocPFactsInput location{};
    location.source = PrStage1XaCdDirectHalGetlocPSource::CurrentPhysicalClock;
    location.sectorIndexKnown = location.cdGetlocPResponseKnown = true;
    location.sectorIndex = 123u;
    location.cdGetlocPResponse = {0x28, 0x07, 0x01, 0x02, 0x01, 0x04, 0x64, 0x00};
    location.cdDataReadyInterruptKnown = true;
    location.cdDataReadyInterrupt = 2u;
    PrStage1XaCdDirectApplyHalGetlocPFacts(cd, location);
    const auto clock = PrStage1XaCdDirectApplySub8001A3C8ClockPollFromLowerState(cd);
    assert(clock.psxReturn == 1 && cd.dword_80049428Known && cd.dword_80049428 == 0);
    assert(PrStage1XaCdDirectApplySub8001A280WorkBaseCommand(cd).commandIssued);
    const auto locationSerial = cd.commandSerial;
    assert(cd.lastCdCommand == 0x10 && cd.byte_800573D4 == 0);
    assert(!E::ApplyPendingStatusReceipt(cd, locationSerial, true, 0x20));
    PrStage1CdStopRuntime8001A694 run{};
    E::State executor{};
    const auto transport = +[](void*, bool, uint8_t command, uint64_t) {
        assert(command == 8u);
        return E::DeviceReply{true, 0u, true, 0u};
    };
    assert(PrStage1XaCdDirectAdvanceStop8001A694(run, cd).requestIssued);
    assert(!E::Execute(executor, cd, run.request, 0, transport, nullptr).known);
    assert(!E::ApplyPendingLocationReceipt(cd, locationSerial, false, location.cdGetlocPResponse));
    assert(!E::ApplyPendingLocationReceipt(cd, locationSerial + 1u, true, location.cdGetlocPResponse));
    assert(E::ApplyPendingLocationReceipt(cd, locationSerial, true, location.cdGetlocPResponse));
    assert(cd.commandSerial == locationSerial && cd.byte_800573D4 == 2);
    assert(cd.response_800882F8Known && cd.response_800882F8 == location.cdGetlocPResponse);
    assert(cd.dword_80057108Known && cd.dword_80057108 == 0x20); // NOT header[0]=0x28
    const auto locationEventSerial = cd.cdLowerEvent80036AF8Serial;
    assert(!E::ApplyPendingLocationReceipt(cd, locationSerial, true, location.cdGetlocPResponse));
    assert(cd.cdLowerEvent80036AF8Serial == locationEventSerial);
    auto feedback = E::Execute(executor, cd, run.request, 1, transport, nullptr);
    assert(feedback.known && feedback.result == 0);
    PrStage1XaCdDirectAdvanceStop8001A694(run, cd, feedback);
    feedback = E::Execute(executor, cd, run.request, 2, transport, nullptr);
    assert(feedback.known && feedback.result == 2);
    assert(PrStage1XaCdDirectAdvanceStop8001A694(run, cd, feedback).complete);
    assert(run.returnKnown && cd.response_80049414Known && cd.dword_800570F8 == 0);
    std::puts("Stage1 real start -> status receipt -> clock/query10 -> pending header receipt -> CD Stop: PASS");
}

int main() {
    auto state = std::make_unique<PrStage1XaCdDirectState>();
    CheckActualLowerExecution(*state);
    CheckStageRecordPendingStatus(*state);
    Seed(*state);
    const auto swapped = PrStage1XaCdDirectApplySub80036510SetCdCallback(*state, 0);
    assert(swapped.psxReturnKnown && static_cast<uint32_t>(swapped.psxReturn) == callback);
    assert(state->dword_800570F8 == 0 && !state->streamClockCallback8001A210Registered);
    const auto ready = PrStage1XaCdDirectApplySub80036528SetCdReadyCallback(*state, 0);
    assert(ready.psxReturnKnown && static_cast<uint32_t>(ready.psxReturn) == 0x80039240u);
    state->dword_800570F8Known = false;
    assert(!PrStage1XaCdDirectApplySub80036510SetCdCallback(*state, callback).psxReturnKnown);
    state->dword_800570FCKnown = false;
    assert(!PrStage1XaCdDirectApplySub80036528SetCdReadyCallback(*state, 0).psxReturnKnown);
    const auto same = PrStage1XaCdDirectApplySub80036510SetCdCallback(*state, callback);
    assert(same.psxReturnKnown && static_cast<uint32_t>(same.psxReturn) == callback);

    Seed(*state);
    PrStage1CdStopRuntime8001A694 run{};
    state->dword_800570F8Known = false;
    assert(PrStage1XaCdDirectAdvanceStop8001A694(run, *state).waitingForSource);
    assert(run.phase == Phase::Idle && !state->dword_800570F8Known);
    state->dword_800570F8Known = true;
    state->dword_80057108Known = false;
    assert(PrStage1XaCdDirectAdvanceStop8001A694(run, *state).waitingForSource);
    assert(run.requestSerial == 0 && state->dword_800570F8 == callback);
    state->dword_80057108Known = true;
    assert(PrStage1XaCdDirectAdvanceStop8001A694(run, *state).requestIssued);
    assert(run.phase == Phase::StopCommand && run.request.function == 0x800375BCu);
    assert((run.request.args == std::array<uint32_t,4>{8,0,0x80049414u,0}));
    assert(state->dword_800570F8 == callback);
    const auto first = run.request;
    for (int i = 0; i < 20; ++i) {
        const auto pending = PrStage1XaCdDirectAdvanceStop8001A694(run, *state);
        assert(!pending.complete && !pending.requestIssued && !pending.feedbackConsumed);
        assert(run.request.serial == first.serial && state->dword_800570F8 == callback);
    }
    auto wrong = PrStage1CdStopFeedback8001A694{true, first, 0};
    ++wrong.request.serial;
    assert(PrStage1XaCdDirectAdvanceStop8001A694(run, *state, wrong).feedbackRejected);
    wrong.request = first;
    wrong.request.args[0] = 1;
    assert(PrStage1XaCdDirectAdvanceStop8001A694(run, *state, wrong).feedbackRejected);
    wrong.request = first;
    wrong.request.function = 0x80037070u;
    assert(PrStage1XaCdDirectAdvanceStop8001A694(run, *state, wrong).feedbackRejected);
    assert(Reply(run, *state, 0).requestIssued && run.phase == Phase::Sync);
    assert((run.request.args == std::array<uint32_t,4>{0,0x80049414u,0,0}));
    assert(run.request.function == 0x80037070u);
    assert(PrStage1XaCdDirectAdvanceStop8001A694(run, *state, {true, first, 0}).feedbackRejected);
    assert(state->dword_800570F8 == callback);
    assert(Reply(run, *state, 2).complete);
    assert(run.returnKnown && static_cast<uint32_t>(run.result) == callback);
    assert(state->dword_800570F8 == 0 && !state->streamClockCallback8001A210Registered);
    const auto serial = run.requestSerial;
    assert(PrStage1XaCdDirectAdvanceStop8001A694(run, *state).complete);
    assert(PrStage1XaCdDirectAdvanceStop8001A694(run, *state, {true, run.request, 2}).feedbackRejected);
    assert(run.requestSerial == serial);
    CheckUnrelated(*state);

    // The low BYTE is tested, not a similarly positioned high status bit.
    Seed(*state); run = {};
    state->dword_80057108 = 0x1000;
    assert(PrStage1XaCdDirectAdvanceStop8001A694(run, *state).requestIssued);
    assert(run.phase == Phase::StopCommand && state->dword_800570F8 == callback);
    Reply(run, *state, 0);
    state->dword_800570F8Known = false;
    assert(Reply(run, *state, 5).waitingForSource);
    assert(run.phase == Phase::Idle && !state->dword_800570F8Known);
    state->dword_800570F8Known = true;
    state->dword_800570F8 = 0;
    assert(PrStage1XaCdDirectAdvanceStop8001A694(run, *state).requestIssued);
    Reply(run, *state, 0);
    assert(Reply(run, *state, 2).complete && run.returnKnown && run.result == 0);
    CheckUnrelated(*state);

    // Four command failures are one367A4 attempt group, not outer termination.
    Seed(*state); run = {};
    PrStage1XaCdDirectAdvanceStop8001A694(run, *state);
    for (int i = 0; i < 8; ++i) {
        assert(Reply(run, *state, -1).requestIssued);
        assert(run.phase == Phase::StopCommand && state->dword_800570F8 == callback);
        assert(run.calls800367A4 == static_cast<uint32_t>(1 + (i + 1) / 4));
    }
    assert(Reply(run, *state, 0).requestIssued && run.phase == Phase::Sync);
    assert(Reply(run, *state, 5).requestIssued && run.phase == Phase::StopCommand);
    assert(run.calls800367A4 == 4 && state->dword_800570F8 == callback);
    Reply(run, *state, 0);
    assert(Reply(run, *state, 2).complete);
    CheckUnrelated(*state);

    // Status bit0x10 adds command1 under a null callback, then restores it.
    Seed(*state); run = {};
    state->dword_80057108 = 0x10;
    PrStage1XaCdDirectAdvanceStop8001A694(run, *state);
    assert(run.phase == Phase::StatusCommand && state->dword_800570F8 == 0);
    assert((run.request.args == std::array<uint32_t,4>{1,0,0,0}));
    assert(Reply(run, *state, -1).requestIssued); // ignored recovery result
    assert(run.phase == Phase::StopCommand && state->dword_800570F8 == callback);
    state->dword_80057108 = 0;
    assert(Reply(run, *state, -1).requestIssued && run.phase == Phase::StopCommand);
    Reply(run, *state, 0);
    // Final36510 returns the CURRENT callback, not the earlier saved one.
    PrStage1XaCdDirectApplySub80036510SetCdCallback(*state, 0x80012340u);
    assert(Reply(run, *state, 2).complete);
    assert(static_cast<uint32_t>(run.result) == 0x80012340u);
    CheckUnrelated(*state);
    std::puts("Stage1 CD stop continuation: PASS (callbacks, retry, wait, request identity)");
}
