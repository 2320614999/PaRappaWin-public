#include "pr/pr_ss0_resource_audio_direct.h"

#include <cstdio>

using namespace PrSS0ResourceAudioDirect;

namespace {

int g_failures = 0;

#define CHECK(expr)                                                         \
    do {                                                                    \
        if (!(expr)) {                                                      \
            std::fprintf(stderr, "check failed: %s:%d: %s\n",             \
                         __FILE__, __LINE__, #expr);                       \
            ++g_failures;                                                   \
        }                                                                   \
    } while (false)

struct CueSinkCapture {
    int order = 0;
    int playOrder = 0;
    int flushOrder = 0;
    uint16_t driverState = 0;
    uint8_t bytes[4]{};
    uint16_t arg4 = 0xFFFFu;
    uint8_t byte3Right = 0;
};

int16_t CapturePlay80034240(uint16_t driverState,
                            uint8_t byte0,
                            uint8_t byte1,
                            uint8_t byte2,
                            uint16_t arg4,
                            uint8_t byte3Left,
                            uint8_t byte3Right,
                            void* userData) {
    auto& capture = *static_cast<CueSinkCapture*>(userData);
    capture.playOrder = ++capture.order;
    capture.driverState = driverState;
    capture.bytes[0] = byte0;
    capture.bytes[1] = byte1;
    capture.bytes[2] = byte2;
    capture.bytes[3] = byte3Left;
    capture.arg4 = arg4;
    capture.byte3Right = byte3Right;
    return 0x0042;
}

int32_t CaptureFlush80026ECC(void* userData) {
    auto& capture = *static_cast<CueSinkCapture*>(userData);
    capture.flushOrder = ++capture.order;
    return 0x1357;
}

void CheckCue(uint32_t inputMask,
              CueKind expectedCue,
              uint8_t expectedNote,
              uint8_t expectedKey) {
    CueSinkCapture capture{};
    CueSoftwareSinks80025C8C sinks{};
    sinks.play80034240 = CapturePlay80034240;
    sinks.flush80026ECC = CaptureFlush80026ECC;
    sinks.userData = &capture;
    const CueSpec sourceBinding = KnownCueSpec(expectedCue);
    CueSoftwareTransaction80025C8C transaction{};
    CHECK(ExecuteInputSfxSoftware80025C8C(
        inputMask, sourceBinding, 0x1234u, sinks, transaction));
    CHECK(transaction.inputMask == inputMask);
    CHECK(transaction.cue == expectedCue);
    CHECK(transaction.sourceAccepted);
    CHECK(transaction.sourceMutated);
    CHECK(transaction.voiceStored800943AC);
    CHECK(transaction.flushExecuted80026ECC);
    CHECK(transaction.driverBindingKind ==
          CueDriverBindingKind800943A8::ExactPsxHandle);
    CHECK(transaction.exactPsxDriverHandleKnown);
    CHECK(!transaction.hostStage1VabProjection);
    CHECK(!transaction.exactPsxHalParity);
    CHECK(transaction.driverState800943A8 == 0x1234u);
    CHECK(transaction.playResult80034240 == 0x0042);
    CHECK(transaction.flushResult80026ECC == 0x1357);
    CHECK(transaction.command[0] == 0u);
    CHECK(transaction.command[1] == expectedNote);
    CHECK(transaction.command[2] == expectedKey);
    CHECK(transaction.command[2] ==
          static_cast<uint8_t>(expectedNote + 0x18u));
    CHECK(transaction.command[3] == 0x7Fu);
    CHECK(capture.playOrder == 1);
    CHECK(capture.flushOrder == 2);
    CHECK(capture.driverState == 0x1234u);
    CHECK(capture.bytes[0] == transaction.command[0]);
    CHECK(capture.bytes[1] == transaction.command[1]);
    CHECK(capture.bytes[2] == transaction.command[2]);
    CHECK(capture.bytes[3] == transaction.command[3]);
    CHECK(capture.arg4 == 0u);
    CHECK(capture.byte3Right == transaction.command[3]);
}

void CheckStage1Event4Binding(CueKind cue,
                              uint32_t globalPointerAddress,
                              uint32_t commandAddress,
                              uint8_t byte0,
                              uint8_t byte1,
                              uint8_t byte2,
                              uint8_t byte3) {
    const CueSpec spec = KnownStage1Event4CueSpec(cue);
    CHECK(spec.kind == cue);
    CHECK(spec.globalPointerAddress == globalPointerAddress);
    CHECK(spec.commandAddress == commandAddress);
    CHECK(spec.bytes[0] == byte0);
    CHECK(spec.bytes[1] == byte1);
    CHECK(spec.bytes[2] == byte2);
    CHECK(spec.bytes[3] == byte3);
}

void CheckStage1Event4HostProjection() {
    CheckStage1Event4Binding(
        CueKind::Stage1Event4Primary9441C,
        kCueGlobal9441C,
        kStage1Event4CueData801CCC68,
        0x00u, 0x0Au, 0x22u, 0x5Au);
    CheckStage1Event4Binding(
        CueKind::Stage1Event4Secondary9441CPlus6,
        kCueGlobal9441C,
        kStage1Event4CueData801CCC68 + 6u,
        0x00u, 0x0Bu, 0x23u, 0x5Au);
    CheckStage1Event4Binding(
        CueKind::Direction94420,
        kCueGlobal94420,
        kStage1Event4DirectionData801CCC74,
        0x00u, 0x0Cu, 0x24u, 0x7Fu);
    CheckStage1Event4Binding(
        CueKind::Cross94424,
        kCueGlobal94424,
        kStage1Event4CrossData801CCC7C,
        0x00u, 0x0Du, 0x25u, 0x7Fu);
    CheckStage1Event4Binding(
        CueKind::Circle94428,
        kCueGlobal94428,
        kStage1Event4CircleData801CCC84,
        0x00u, 0x0Eu, 0x26u, 0x7Fu);
    CheckStage1Event4Binding(
        CueKind::Select9442C,
        kCueGlobal9442C,
        kStage1Event4SelectData801CCC8C,
        0x00u, 0x0Fu, 0x27u, 0x7Fu);

    CueDriverBinding800943A8 hostBinding{};
    hostBinding.kind =
        CueDriverBindingKind800943A8::HostStage1VabProjection;
    hostBinding.value = 0u;
    hostBinding.hostStage1VabReady = true;

    CueSinkCapture capture{};
    CueSoftwareTransaction80026EF8 periodic{};
    CHECK(ExecuteCueSoftware80026EF8(
        KnownStage1Event4CueSpec(
            CueKind::Stage1Event4Primary9441C),
        hostBinding,
        CapturePlay80034240,
        &capture,
        periodic));
    CHECK(periodic.sourceAccepted);
    CHECK(periodic.sourceMutated);
    CHECK(periodic.voiceStored800943AC);
    CHECK(periodic.driverBindingKind ==
          CueDriverBindingKind800943A8::HostStage1VabProjection);
    CHECK(!periodic.exactPsxDriverHandleKnown);
    CHECK(periodic.hostStage1VabProjection);
    CHECK(!periodic.exactPsxHalParity);
    CHECK(periodic.command[0] == 0x00u);
    CHECK(periodic.command[1] == 0x0Au);
    CHECK(periodic.command[2] == 0x22u);
    CHECK(periodic.command[3] == 0x5Au);
    CHECK(periodic.playResult80034240 == 0x0042);
    CHECK(capture.playOrder == 1);
    CHECK(capture.flushOrder == 0);

    CueSoftwareSinks80025C8C sinks{};
    sinks.play80034240 = CapturePlay80034240;
    sinks.flush80026ECC = CaptureFlush80026ECC;
    sinks.userData = &capture;
    CueSoftwareTransaction80025C8C input{};
    CHECK(ExecuteInputSfxSoftware80025C8C(
        0x20u,
        KnownStage1Event4CueSpec(CueKind::Circle94428),
        hostBinding,
        sinks,
        input));
    CHECK(input.sourceCommandAddress ==
          kStage1Event4CircleData801CCC84);
    CHECK(input.hostStage1VabProjection);
    CHECK(!input.exactPsxDriverHandleKnown);
    CHECK(!input.exactPsxHalParity);
    CHECK(capture.playOrder == 2);
    CHECK(capture.flushOrder == 3);

    hostBinding.hostStage1VabReady = false;
    capture = {};
    CHECK(!ExecuteCueSoftware80026EF8(
        KnownStage1Event4CueSpec(
            CueKind::Stage1Event4Secondary9441CPlus6),
        hostBinding,
        CapturePlay80034240,
        &capture,
        periodic));
    CHECK(!periodic.sourceAccepted);
    CHECK(periodic.hostStage1VabProjection);
    CHECK(!periodic.exactPsxDriverHandleKnown);
    CHECK(!periodic.exactPsxHalParity);
    CHECK(capture.order == 0);
}

void CheckStartupCallbackRoute80016B84() {
    const ResourceAudioPlan plan = BuildStartupPreload80016B84Plan();
    CHECK(!plan.runtimeCutoverAllowed);
    CHECK(plan.hasOpenP0Gap);
    CHECK(!plan.truncated);
    CHECK(plan.count >= 6u);

    const uint32_t base = plan.count - 6u;
    const ResourceAudioAction& firstFade = plan.actions[base + 0u];
    CHECK(firstFade.kind ==
          ResourceAudioActionKind::Call80015A4CStartupFadeLoop);
    CHECK(firstFade.psxFunction == kFn80015A4C);
    CHECK(firstFade.arg0 == kFn8001E408);
    CHECK(firstFade.arg1 == kFn8001E54C);
    CHECK(firstFade.arg2 == kFn8001C0A0);

    const ResourceAudioAction& firstWait = plan.actions[base + 1u];
    CHECK(firstWait.kind ==
          ResourceAudioActionKind::Call80015B00StartupInputWait);
    CHECK(firstWait.psxFunction == kFn80015B00);
    CHECK(firstWait.arg0 == 150u);
    CHECK(firstWait.arg1 == 60u);

    const ResourceAudioAction& firstExit = plan.actions[base + 2u];
    CHECK(firstExit.kind ==
          ResourceAudioActionKind::Call80015C20StartupFadeLoop);
    CHECK(firstExit.psxFunction == kFn80015C20);
    CHECK(firstExit.arg0 == kFn8001E408);
    CHECK(firstExit.arg1 == kFn8001E54C);

    const ResourceAudioAction& secondFade = plan.actions[base + 3u];
    CHECK(secondFade.kind ==
          ResourceAudioActionKind::Call80015A4CStartupFadeLoop);
    CHECK(secondFade.psxFunction == kFn80015A4C);
    CHECK(secondFade.arg0 == kFn8001E5A4);
    CHECK(secondFade.arg1 == kFn8001E6B0);
    CHECK(secondFade.arg2 == kFn8001C1B8);

    const ResourceAudioAction& secondWait = plan.actions[base + 4u];
    CHECK(secondWait.kind ==
          ResourceAudioActionKind::Call80015B00StartupInputWait);
    CHECK(secondWait.psxFunction == kFn80015B00);
    CHECK(secondWait.arg0 == 150u);
    CHECK(secondWait.arg1 == 60u);

    const ResourceAudioAction& secondExit = plan.actions[base + 5u];
    CHECK(secondExit.kind ==
          ResourceAudioActionKind::Call80015C20StartupFadeLoop);
    CHECK(secondExit.psxFunction == kFn80015C20);
    CHECK(secondExit.arg0 == kFn8001E5A4);
    CHECK(secondExit.arg1 == kFn8001E6B0);
}

void CheckStartupAudioBindings80016C8C() {
    const auto bindings = BuildStartupAudioBindings80016C8C();
    CHECK(IsExactStartupAudioBindings80016C8C(bindings));
    CHECK(bindings.sourceFunction == kFn80016C8C);
    CHECK(bindings.slot800943FC == 0x80048DA0u);
    CHECK(bindings.slot80094400 == 0x80048D94u);
    CHECK(bindings.slot80094410 == 0x8006EA84u);
    CHECK(bindings.slot8009441C == 0x80048DACu);
    CHECK(bindings.slot80094420 == 0x8006EA9Cu);
    CHECK(bindings.slot80094424 == 0x8006EAA4u);
    CHECK(bindings.slot80094428 == 0x8006EAACu);
    CHECK(bindings.slot8009442C == 0x8006EAB4u);
    CHECK(bindings.returnValueKnown);
    CHECK(bindings.returnValue == 0x8006EA84u);
    CHECK(bindings.directSoftwareStateAuthority);
    CHECK(!bindings.physicalSpuAuthority);

    auto mutation = bindings;
    mutation.slot80094410 ^= 4u;
    CHECK(!IsExactStartupAudioBindings80016C8C(mutation));
    mutation = bindings;
    mutation.physicalSpuAuthority = true;
    CHECK(!IsExactStartupAudioBindings80016C8C(mutation));
}

} // namespace

int main() {
    CheckCue(0x20u, CueKind::Circle94428, 0x0Eu, 0x26u);
    CheckCue(0x40u, CueKind::Cross94424, 0x0Du, 0x25u);
    CheckCue(0x100u, CueKind::Select9442C, 0x0Fu, 0x27u);
    CheckCue(0x1000u, CueKind::Direction94420, 0x0Cu, 0x24u);
    CheckCue(0x2000u, CueKind::Direction94420, 0x0Cu, 0x24u);
    CheckCue(0x4000u, CueKind::Direction94420, 0x0Cu, 0x24u);
    CheckCue(0x8000u, CueKind::Direction94420, 0x0Cu, 0x24u);
    CheckStage1Event4HostProjection();
    CheckStartupAudioBindings80016C8C();
    CheckStartupCallbackRoute80016B84();

    CueSinkCapture capture{};
    CueSoftwareSinks80025C8C sinks{};
    sinks.play80034240 = CapturePlay80034240;
    sinks.flush80026ECC = CaptureFlush80026ECC;
    sinks.userData = &capture;
    CueSoftwareTransaction80025C8C rejected{};
    CHECK(!ExecuteInputSfxSoftware80025C8C(
        0x0001u, CueSpec{}, 0xFFFFu, sinks, rejected));
    CHECK(rejected.cue == CueKind::Unknown);
    CHECK(!rejected.sourceAccepted);
    CHECK(!rejected.sourceMutated);
    CHECK(!rejected.voiceStored800943AC);
    CHECK(!rejected.flushExecuted80026ECC);
    CHECK(rejected.playResult80034240 == -1);
    CHECK(capture.order == 0);

    CueSoftwareSinks80025C8C missingSinks{};
    CHECK(!ExecuteInputSfxSoftware80025C8C(
        0x20u, KnownCueSpec(CueKind::Circle94428),
        0xFFFFu, missingSinks, rejected));
    CHECK(capture.order == 0);

    CueSpec mismatched = KnownCueSpec(CueKind::Cross94424);
    CHECK(!ExecuteInputSfxSoftware80025C8C(
        0x20u, mismatched, 0xFFFFu, sinks, rejected));
    CHECK(capture.order == 0);

    if (g_failures != 0) {
        std::fprintf(stderr, "SS0 resource audio direct: %d failures\n",
                     g_failures);
        return 1;
    }
    std::printf("SS0 resource audio direct: ok\n");
    return 0;
}
