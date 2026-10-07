#pragma once

#include "pr_stage1_xa_cd_direct.h"

namespace PrStage1CdStopExecutionDirect {

// A software CD-device receipt, NOT a PSX MMIO observation. The Windows
// transport must actually stop its reader before reporting completed. Unit
// tests can withhold either phase without reissuing the native command.
struct DeviceReply {
    bool acknowledged = false;
    uint8_t acknowledgementStatus = 0;
    bool completed = false;
    uint8_t completionStatus = 0;
};
using DeviceCall = DeviceReply (*)(void*, bool submit, uint8_t command, uint64_t serial);
struct State {
    PrStage1CdStopRequest8001A694 command{};
    bool issued = false;
    bool deviceSubmitted = false;
    bool acknowledgementApplied = false;
    bool completionApplied = false;
    uint32_t nativeCommandSerial = 0;
    PrStage1CdStopFeedback8001A694 commandReturn{};
    PrStage1CdStopFeedback8001A694 syncReturn{};
};
inline bool Same(const PrStage1CdStopRequest8001A694& a,
                 const PrStage1CdStopRequest8001A694& b) {
    return a.serial == b.serial && a.function == b.function && a.args == b.args;
}
inline bool DeliverEventBytes(PrStage1XaCdDirectState& cd, uint8_t interrupt,
                              const std::array<uint8_t, 8>& bytes, uint32_t count) {
    PrStage1XaCdDirectApplySub80035898CheckCallback(cd, {true, 1u});
    PrMovieSegmentDirect::CdCallbackEventInput80036AF8 event{};
    event.interruptKnown = event.resultBytesKnown = true;
    event.interruptCode = interrupt;
    event.resultByteCount = count;
    event.resultBytes = bytes;
    const auto result = PrStage1XaCdDirectApplySub80036AF8CdLowerEvent(cd, event);
    if (!result.eventKnown || !result.byte800573D4Known) return false;
    PrStage1XaCdDirectDispatchCommandCallbacks800375BC(cd);
    PrStage1XaCdDirectApplySub80035898CheckCallback(cd, {true, 0u});
    return true;
}
inline bool DeliverEvent(PrStage1XaCdDirectState& cd, uint8_t interrupt, uint8_t status) {
    return DeliverEventBytes(cd, interrupt, {status}, 1u);
}
inline bool CopyResponse80049414(PrStage1XaCdDirectState& cd) {
    if (!cd.response_800882F8Known) return false;
    cd.response_80049414 = cd.response_800882F8;
    cd.response_80049414Known = true;
    return true;
}
inline bool ApplyPendingStatusReceipt(PrStage1XaCdDirectState& cd,
                                      uint32_t commandSerial,
                                      bool acknowledged, uint8_t status) {
    // 1A4D0 ends with36678(1,0), whose375BC uses skip-wait. Its outstanding
    // INT3 must be serviced before a later blocking command's pre-sync.
    // Do not issue another command or turn an absent receipt into success.
    if (!acknowledged || !commandSerial || cd.commandSerial != commandSerial ||
        cd.command1Serial != commandSerial || cd.lastCdCommand != 1u ||
        !cd.byte_80057119Known || cd.byte_80057119 != 1u ||
        !cd.byte_800573D4Known || cd.byte_800573D4 != 0u) return false;
    return DeliverEvent(cd, 3u, status);
}
inline bool ApplyPendingLocationReceipt(PrStage1XaCdDirectState& cd,
                                        uint32_t commandSerial, bool responseKnown,
                                        const std::array<uint8_t, 8>& sectorHeader) {
    // 1A280 ->36678(0x10,0): INT3 carries raw sector bytes0C..13, NOT
    // drive-status bytes. The native572BC table suppresses status publication.
    if (!responseKnown || !commandSerial || cd.commandSerial != commandSerial ||
        cd.lastCdCommand != 0x10u || !cd.byte_80057119Known ||
        cd.byte_80057119 != 0x10u || !cd.byte_800573D4Known ||
        cd.byte_800573D4 != 0u) return false;
    return DeliverEventBytes(cd, 3u, sectorHeader, 8u);
}
inline PrStage1CdStopFeedback8001A694 Execute(
    State& state, PrStage1XaCdDirectState& cd,
    const PrStage1CdStopRequest8001A694& request, int32_t clockNow,
    DeviceCall device, void* user) {
    if (!device || !request.serial) return {};
    if (request.function == 0x800375BCu) {
        const bool statusCommand = request.args == std::array<uint32_t, 4>{1u, 0u, 0u, 0u};
        const bool stopCommand = request.args == std::array<uint32_t, 4>{8u, 0u, 0x80049414u, 0u};
        if (!statusCommand && !stopCommand) return {};
        if (!Same(state.command, request)) {
            if (request.serial <= state.command.serial) return {};
            // 375BC performs blocking pre-sync BEFORE issuing the new command.
            // Reuse the current translated producer's receipt, never a bare
            // hard-coded success or a stale stop acknowledgement.
            if (!cd.byte_800573D4Known ||
                (cd.byte_800573D4 != 2u && cd.byte_800573D4 != 5u)) return {};
            PrStage1XaCdDirectCdSyncInput80037070 preSync{};
            preSync.feedback.known = preSync.feedback.syncResultKnown = true;
            preSync.feedback.syncResult = cd.byte_800573D4;
            const auto completed = PrStage1XaCdDirectApplySub80037070(cd, preSync);
            if (!completed.syncResultKnown) return {};
            state = {};
            state.command = request;
        }
        if (state.commandReturn.known) return state.commandReturn;
        if (!state.issued) {
            PrStage1XaCdDirectCommandInput800375BC command{};
            command.command = static_cast<uint8_t>(request.args[0]);
            command.argsKnown = true;
            command.skipWait = false;
            command.clockKnown = true;
            command.clockNow = clockNow;
            command.preSyncFeedback.known = command.preSyncFeedback.syncResultKnown = true;
            command.preSyncFeedback.syncResult = cd.cdSync80037070.psxReturn;
            command.preSyncFeedback.timedOut = cd.cdSync80037070.timeout;
            PrStage1XaCdDirectApplySub800375BCCommand(cd, command);
            state.nativeCommandSerial = cd.commandSerial;
            state.issued = true;
        }
        if (cd.commandSerial != state.nativeCommandSerial ||
            cd.lastCdCommand != request.args[0]) return {};
        const auto reply = device(user, !state.deviceSubmitted,
            static_cast<uint8_t>(request.args[0]), request.serial);
        state.deviceSubmitted = true;
        if (!reply.acknowledged) return {};
        if (!state.acknowledgementApplied) {
            if (!DeliverEvent(cd, 3u, reply.acknowledgementStatus)) return {};
            state.acknowledgementApplied = true;
        }
        if (!cd.byte_800573D4Known || cd.byte_800573D4 == 0u) return {};
        if (stopCommand && !CopyResponse80049414(cd)) return {};
        // 800379C8..800379D0 returns -(byte573D4==5), not the raw IRQ status.
        state.commandReturn = {true, request, cd.byte_800573D4 == 5u ? -1 : 0};
        return state.commandReturn;
    }
    if (request.function != 0x80037070u ||
        request.args != std::array<uint32_t, 4>{0u, 0x80049414u, 0u, 0u} ||
        !state.issued || !state.commandReturn.known || state.commandReturn.result != 0 ||
        state.command.args[0] != 8u || request.serial != state.command.serial + 1u ||
        cd.commandSerial != state.nativeCommandSerial || cd.lastCdCommand != 8u) return {};
    if (state.syncReturn.known) return Same(state.syncReturn.request, request) ? state.syncReturn : PrStage1CdStopFeedback8001A694{};
    const auto reply = device(user, false, 8u, state.command.serial);
    if (!reply.completed) return {};
    if (!state.completionApplied) {
        if (!DeliverEvent(cd, 2u, reply.completionStatus)) return {};
        state.completionApplied = true;
    }
    PrStage1XaCdDirectCdSyncInput80037070 sync{};
    sync.a0WaitMode = 0;
    sync.a1OutputBufferPtrNonNull = true;
    if (!cd.cdLowerFeedback80036AF8Known || cd.cdLowerFeedback80036AF8FromGetlocP) return {};
    sync.feedback = cd.cdLowerFeedback80036AF8;
    const auto result = PrStage1XaCdDirectApplySub80037070(cd, sync);
    if (!result.syncResultKnown || !result.copiedOutputBytes || result.outputByteCount != 8u) return {};
    for (size_t i = 0; i < cd.response_80049414.size(); ++i) cd.response_80049414[i] = result.outputBytes[i];
    cd.response_80049414Known = true;
    state.syncReturn = {true, request, result.psxReturn};
    return state.syncReturn;
}

} // namespace PrStage1CdStopExecutionDirect
