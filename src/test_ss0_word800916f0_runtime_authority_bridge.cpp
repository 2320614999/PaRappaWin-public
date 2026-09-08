#include "pr/pr_ss0_directory_dispatcher_direct.h"
#include "pr/pr_ss0_word800916f0_direct.h"
#include "pr/pr_stage1_lifecycle_direct.h"
#include "pr/pr_stage_status_bank_direct.h"

#include <cstdio>

using namespace PrSS0Scene0RuntimeDirect;
using namespace PrStage1LifecycleDirect;

namespace {

int g_failed = 0;

#define CHECK(expr)                                                           \
    do {                                                                      \
        if (!(expr)) {                                                        \
            std::printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr);       \
            ++g_failed;                                                       \
        }                                                                     \
    } while (0)

uint32_t CountStageStatusActions(const PrStageStatusBankActionTrace& trace,
                                 PrStageStatusBankActionKind kind) {
    uint32_t count = 0;
    for (size_t i = 0; i < trace.count; ++i) {
        if (trace.actions[i].kind == kind) {
            ++count;
        }
    }
    return count;
}

uint32_t CountClearTailActions(const StepResult801C81EC& result,
                               ActionKind801C81EC kind) {
    uint32_t count = 0;
    for (const Action801C81EC& action : result.actions) {
        if (action.kind == kind) {
            ++count;
        }
    }
    return count;
}

Word800916F0Observation ExplicitF0Observation(uint16_t value) {
    Word800916F0Observation observation{};
    observation.source =
        Word800916F0ObservationSource::RuntimePsxMemoryObservation;
    observation.psxAddress = 0x800916F0u;
    observation.byteSize = 2u;
    observation.valueKnown = true;
    observation.value = value;
    observation.frameKnown = true;
    observation.frame = 1234u;
    observation.pcKnown = true;
    observation.pc = 0x800267F8u;
    return observation;
}

Word800916F0Observation ConsumerReadF0Observation(uint16_t value) {
    Word800916F0Observation observation = ExplicitF0Observation(value);
    observation.source =
        Word800916F0ObservationSource::RuntimePsxConsumerReadObservation;
    observation.pc = 0x801C84B0u;
    return observation;
}

Word800916F0RuntimePsxMemoryProviderRead ProviderF0Read(uint16_t value) {
    Word800916F0RuntimePsxMemoryProviderRead read{};
    read.attempted = true;
    read.readable = true;
    read.psxAddress = 0x800916F0u;
    read.byteSize = 2u;
    read.bytes[0] = static_cast<uint8_t>(value & 0xFFu);
    read.bytes[1] = static_cast<uint8_t>((value >> 8) & 0xFFu);
    read.frameKnown = true;
    read.frame = 4321u;
    read.pcKnown = true;
    read.pc = 0x800267F8u;
    return read;
}

void PromoteExplicitF0(uint16_t value) {
    const Word800916F0Observation observation = ExplicitF0Observation(value);
    CHECK(ApplyWord800916F0RuntimeObservation(observation));
    CHECK(IsWord800916F0Known());
    CHECK(IsWord800916F0RuntimeObservationKnown());
    CHECK(GetWord800916F0() == value);
}

PrSS0DirectoryDispatcherDirect::StageSelectState80025F6C
BuildStageSelectFromF0Authority(uint8_t bonusStatus = 0u) {
    PrSS0DirectoryDispatcherDirect::StageSelectInitInput800267F8 input{};
    for (int i = 0; i < 6; ++i) {
        input.status80092F1DTo23[i] = 1u;
    }
    input.status80092F1DTo23[6] = bonusStatus;
    input.word800916DA = 0;
    input.word800916F0Known = IsWord800916F0RuntimeObservationKnown();
    if (input.word800916F0Known) {
        input.word800916F0 = static_cast<int32_t>(GetWord800916F0());
    }
    return PrSS0DirectoryDispatcherDirect::InitStageSelect800267F8(input);
}

PrStageStatusBankClearProducerInput BuildStageStatusInputFromF0Authority() {
    PrStageStatusBankClearProducerInput input{};
    input.comod = PrStageStatusBankComod::Comod1;
    input.stageArg = 1;
    input.word800916D0 = 0u;
    input.word800916DA = 0u;
    input.word800916F0Known = IsWord800916F0RuntimeObservationKnown();
    if (input.word800916F0Known) {
        input.word800916F0 = GetWord800916F0();
    }
    input.word80091816 = 1234u;
    input.previousStatusKnown800166AC = true;
    input.previousStatus800166AC = 2;
    return input;
}

FrameInput801C81EC BuildClearTailInputFromF0Authority() {
    FrameInput801C81EC input{};
    input.sceneId = 1;
    input.word800916D0 = 0u;
    input.word800916DA = 0u;
    input.word800916F0Known = IsWord800916F0RuntimeObservationKnown();
    if (input.word800916F0Known) {
        input.word800916F0 = GetWord800916F0();
    }
    input.word80091816 = 1234u;
    input.clearTailPlayAndWaitResultKnown = true;
    input.clearTailPlayAndWaitResult = 0;
    return input;
}

Runtime801C81EC BuildClearTailRuntime() {
    Runtime801C81EC runtime{};
    runtime.phase = Phase801C81EC::ClearTailMovieRequested;
    runtime.clearTailStageStatusKnown = true;
    runtime.clearTailStageStatus166AC = 2;
    return runtime;
}

void CheckBridgeStateUnknown() {
    CHECK(!IsWord800916F0Known());
    CHECK(GetWord800916F0() == 0u);

    const PrSS0DirectoryDispatcherDirect::StageSelectState80025F6C stage =
        BuildStageSelectFromF0Authority();
    CHECK(stage.enabled[1]);
    CHECK(stage.enabled[6]);
    CHECK(!stage.enabled[7]);
    CHECK(stage.enabled[8]);

    const PrStageStatusBankClearProducerResult status =
        PrStageStatusBankDirectClearProducer(BuildStageStatusInputFromF0Authority());
    CHECK(status.blockedByUnknownWord800916F0);
    CHECK(!status.saveMenuCalled);
    CHECK(CountStageStatusActions(status.trace,
                                  PrStageStatusBankActionKind::Call80015590) ==
          0);
    CHECK(CountStageStatusActions(status.trace,
                                  PrStageStatusBankActionKind::Call80019148) ==
          0);

    Runtime801C81EC runtime = BuildClearTailRuntime();
    const SceneEntry801C7284 sceneEntry{};
    const StepResult801C81EC clearTail =
        Step801C81EC(runtime, sceneEntry, BuildClearTailInputFromF0Authority());
    CHECK(clearTail.blockedByUnknownWord800916F0);
    CHECK(clearTail.sceneResultKnown);
    CHECK(clearTail.sceneResult == 2);
    CHECK(CountClearTailActions(clearTail, ActionKind801C81EC::Bootstrap15590) ==
          0);
    CHECK(CountClearTailActions(clearTail, ActionKind801C81EC::SaveUi19148) ==
          0);
}

void CheckBridgeStateKnownZero() {
    CHECK(IsWord800916F0Known());
    CHECK(IsWord800916F0RuntimeObservationKnown());
    CHECK(GetWord800916F0() == 0u);

    const PrSS0DirectoryDispatcherDirect::StageSelectState80025F6C stage =
        BuildStageSelectFromF0Authority();
    CHECK(stage.enabled[1]);
    CHECK(stage.enabled[6]);
    CHECK(stage.rawStatus80092F1DTo23[6] == 0u);
    CHECK(!stage.enabled[7]);
    CHECK(stage.enabled[8]);
    const PrSS0DirectoryDispatcherDirect::StageSelectState80025F6C
        statusEnabledStage = BuildStageSelectFromF0Authority(1u);
    CHECK(statusEnabledStage.rawStatus80092F1DTo23[6] == 1u);
    CHECK(statusEnabledStage.enabled[7]);

    const PrStageStatusBankClearProducerResult status =
        PrStageStatusBankDirectClearProducer(BuildStageStatusInputFromF0Authority());
    CHECK(!status.blockedByUnknownWord800916F0);
    CHECK(status.saveMenuCalled);
    CHECK(CountStageStatusActions(status.trace,
                                  PrStageStatusBankActionKind::Call80015590) ==
          1);
    CHECK(CountStageStatusActions(status.trace,
                                  PrStageStatusBankActionKind::Call80019148) ==
          1);

    Runtime801C81EC runtime = BuildClearTailRuntime();
    const SceneEntry801C7284 sceneEntry{};
    const StepResult801C81EC clearTail =
        Step801C81EC(runtime, sceneEntry, BuildClearTailInputFromF0Authority());
    CHECK(!clearTail.blockedByUnknownWord800916F0);
    CHECK(clearTail.sceneResultKnown);
    CHECK(clearTail.sceneResult == 2);
    CHECK(CountClearTailActions(clearTail, ActionKind801C81EC::Bootstrap15590) ==
          1);
    CHECK(CountClearTailActions(clearTail, ActionKind801C81EC::SaveUi19148) ==
          1);
}

void CheckBridgeStateKnownOne() {
    CHECK(IsWord800916F0Known());
    CHECK(IsWord800916F0RuntimeObservationKnown());
    CHECK(GetWord800916F0() == 1u);

    const PrSS0DirectoryDispatcherDirect::StageSelectState80025F6C stage =
        BuildStageSelectFromF0Authority();
    CHECK(stage.enabled[1]);
    CHECK(stage.enabled[6]);
    CHECK(stage.enabled[7]);
    CHECK(stage.enabled[8]);
    constexpr uint8_t kExpectedStatus[7] = {1u, 1u, 1u, 1u, 2u, 3u, 1u};
    for (int index = 0; index < 7; ++index) {
        CHECK(stage.rawStatus80092F1DTo23[index] ==
              kExpectedStatus[index]);
    }

    const PrStageStatusBankClearProducerResult status =
        PrStageStatusBankDirectClearProducer(BuildStageStatusInputFromF0Authority());
    CHECK(!status.blockedByUnknownWord800916F0);
    CHECK(!status.saveMenuCalled);
    CHECK(CountStageStatusActions(status.trace,
                                  PrStageStatusBankActionKind::Call80015590) ==
          0);
    CHECK(CountStageStatusActions(status.trace,
                                  PrStageStatusBankActionKind::Call80019148) ==
          0);

    Runtime801C81EC runtime = BuildClearTailRuntime();
    const SceneEntry801C7284 sceneEntry{};
    const StepResult801C81EC clearTail =
        Step801C81EC(runtime, sceneEntry, BuildClearTailInputFromF0Authority());
    CHECK(!clearTail.blockedByUnknownWord800916F0);
    CHECK(clearTail.sceneResultKnown);
    CHECK(clearTail.sceneResult == 2);
    CHECK(CountClearTailActions(clearTail, ActionKind801C81EC::Bootstrap15590) ==
          0);
    CHECK(CountClearTailActions(clearTail, ActionKind801C81EC::SaveUi19148) ==
          0);
}

void TestUnknownF0DoesNotBridgeToConsumers() {
    ClearWord800916F0RuntimeObservation();
    CheckBridgeStateUnknown();
}

void TestStartupKnownZeroDoesNotBridgeToStageSelectOrPostClearConsumers() {
    ClearWord800916F0RuntimeObservation();
    CHECK(PublishInitialWord800916F0FromDiscFullbootStartupSource());
    CHECK(IsWord800916F0Known());
    CHECK(!IsWord800916F0RuntimeObservationKnown());
    CHECK(GetWord800916F0() == 0u);

    const PrSS0DirectoryDispatcherDirect::StageSelectState80025F6C stage =
        BuildStageSelectFromF0Authority(1u);
    CHECK(stage.rawStatus80092F1DTo23[6] == 1u);
    CHECK(!stage.enabled[7]);

    const PrStageStatusBankClearProducerResult status =
        PrStageStatusBankDirectClearProducer(BuildStageStatusInputFromF0Authority());
    CHECK(status.blockedByUnknownWord800916F0);
    CHECK(!status.saveMenuCalled);

    Runtime801C81EC runtime = BuildClearTailRuntime();
    const SceneEntry801C7284 sceneEntry{};
    const StepResult801C81EC clearTail =
        Step801C81EC(runtime, sceneEntry, BuildClearTailInputFromF0Authority());
    CHECK(clearTail.blockedByUnknownWord800916F0);
    CHECK(clearTail.sceneResultKnown);
    CHECK(clearTail.sceneResult == 2);
}

void TestExplicitZeroBridgesToPostClearConsumersWithoutBonusForceEnable() {
    ClearWord800916F0RuntimeObservation();
    PromoteExplicitF0(0u);
    CheckBridgeStateKnownZero();
}

void TestExplicitOneBridgesToBonusForceEnableAndPostClearSkip() {
    ClearWord800916F0RuntimeObservation();
    PromoteExplicitF0(1u);
    CheckBridgeStateKnownOne();
}

void TestConsumerReadObservationDoesNotBridgeToConsumers() {
    ClearWord800916F0RuntimeObservation();
    CHECK(!ApplyWord800916F0RuntimeObservation(ConsumerReadF0Observation(1u)));
    CheckBridgeStateUnknown();

    PromoteExplicitF0(0u);
    CHECK(!ApplyWord800916F0RuntimeObservation(ConsumerReadF0Observation(1u)));
    CheckBridgeStateKnownZero();
}

void TestProviderReadMustBeExactAndReadableBeforeBridge() {
    ClearWord800916F0RuntimeObservation();

    Word800916F0RuntimePsxMemoryProviderRead read = ProviderF0Read(0x2468u);
    read.readable = false;
    CHECK(!PublishWord800916F0FromRuntimePsxMemoryProviderRead(read));
    CheckBridgeStateUnknown();

    read = ProviderF0Read(0x2468u);
    read.psxAddress = 0x800916EEu;
    CHECK(!PublishWord800916F0FromRuntimePsxMemoryProviderRead(read));
    CheckBridgeStateUnknown();

    read = ProviderF0Read(0x2468u);
    read.byteSize = 4u;
    CHECK(!PublishWord800916F0FromRuntimePsxMemoryProviderRead(read));
    CheckBridgeStateUnknown();

    read = ProviderF0Read(0x2468u);
    CHECK(PublishWord800916F0FromRuntimePsxMemoryProviderRead(read));
    CHECK(IsWord800916F0Known());
    CHECK(GetWord800916F0() == 0x2468u);
}

void TestNativeSoftwareStartupAndObservationAreIndependent() {
    ClearWord800916F0RuntimeObservation();
    uint16_t value = 0xCAFEu;
    CHECK(!TryReadWord800916F0Software(value));
    CHECK(value == 0xCAFEu);
    CHECK(!GetWord800916F0SoftwareState().initialized);
    InitializeWord800916F0FromProcessStartupZero80028590();
    CHECK(TryReadWord800916F0Software(value));
    CHECK(value == 0u);
    CHECK(GetWord800916F0SoftwareState().startupCommitCount == 1u);
    CHECK(!IsWord800916F0Known());
    CHECK(!IsWord800916F0RuntimeObservationKnown());
    CHECK(GetWord800916F0ObservationDebugSnapshot().acceptedCount == 0);

    // Historical observation APIs remain available, but neither importing nor
    // clearing a diagnostic observation changes the native game's BSS word.
    PromoteExplicitF0(1u);
    CHECK(TryReadWord800916F0Software(value));
    CHECK(value == 0u);
    CHECK(GetWord800916F0() == 1u);
    ClearWord800916F0RuntimeObservation();
    InitializeWord800916F0FromProcessStartupZero80028590();
    CHECK(TryReadWord800916F0Software(value));
    CHECK(value == 0u);
    CHECK(GetWord800916F0SoftwareState().startupCommitCount == 1u);
    CHECK(GetWord800916F0ObservationDebugSnapshot().acceptedCount == 0);

    FrameInput801C81EC input = BuildClearTailInputFromF0Authority();
    input.word800916F0Known = TryReadWord800916F0Software(input.word800916F0);
    Runtime801C81EC runtime = BuildClearTailRuntime();
    const auto clearTail = Step801C81EC(runtime, {}, input);
    CHECK(!clearTail.blockedByUnknownWord800916F0);
    CHECK(clearTail.clearTailSaveMenuCalled);
    CHECK(CountClearTailActions(clearTail, ActionKind801C81EC::SaveStatus1635C) == 1u);
    CHECK(CountClearTailActions(clearTail, ActionKind801C81EC::UnlockNextStage1628C) == 1u);
    CHECK(CountClearTailActions(clearTail, ActionKind801C81EC::Bootstrap15590) == 1u);
    CHECK(CountClearTailActions(clearTail, ActionKind801C81EC::SaveUi19148) == 1u);
    CHECK(clearTail.sceneResultKnown && clearTail.sceneResult == 2);

    PrSS0DirectoryDispatcherDirect::StageSelectInitInput800267F8 stageInput{};
    stageInput.status80092F1DTo23[0] = 1u;
    stageInput.word800916F0Known = TryReadWord800916F0Software(value);
    stageInput.word800916F0 = value;
    const auto stage = PrSS0DirectoryDispatcherDirect::InitStageSelect800267F8(stageInput);
    CHECK(stage.enabled[1]);
    CHECK(!stage.enabled[2]);
    CHECK(!stage.enabled[7]);
    CHECK(stage.enabled[8]);
}

} // namespace

int main() {
    TestUnknownF0DoesNotBridgeToConsumers();
    TestStartupKnownZeroDoesNotBridgeToStageSelectOrPostClearConsumers();
    TestExplicitZeroBridgesToPostClearConsumersWithoutBonusForceEnable();
    TestExplicitOneBridgesToBonusForceEnableAndPostClearSkip();
    TestConsumerReadObservationDoesNotBridgeToConsumers();
    TestProviderReadMustBeExactAndReadableBeforeBridge();
    TestNativeSoftwareStartupAndObservationAreIndependent();

    if (g_failed != 0) {
        std::printf(
            "test_ss0_word800916f0_runtime_authority_bridge: failed checks=%d\n",
            g_failed);
        return 1;
    }
    std::printf("test_ss0_word800916f0_runtime_authority_bridge: ok\n");
    return 0;
}
