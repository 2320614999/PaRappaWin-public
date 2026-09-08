#include "pr/pr_ss0_stage_progress_bank_direct.h"

#include <cstdio>

using namespace PrSS0StageProgressBankDirect;

namespace {

int g_failed = 0;

#define CHECK(expr)                                                           \
    do {                                                                      \
        if (!(expr)) {                                                        \
            std::printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr);       \
            ++g_failed;                                                       \
        }                                                                     \
    } while (0)

const ProgressBankAction* FindAction(const ProgressBankPlan& plan,
                                     ProgressBankActionKind kind,
                                     uint32_t start = 0) {
    for (uint32_t i = start; i < plan.count; ++i) {
        if (plan.actions[i].kind == kind) {
            return &plan.actions[i];
        }
    }
    return nullptr;
}

uint32_t CountActions(const ProgressBankPlan& plan,
                      ProgressBankActionKind kind) {
    uint32_t count = 0;
    for (uint32_t i = 0; i < plan.count; ++i) {
        if (plan.actions[i].kind == kind) {
            ++count;
        }
    }
    return count;
}

void CheckNoRuntimePayloadAuthority() {
    for (uint32_t i = 0; i < KnownProgressBankCopyProvenanceCount(); ++i) {
        CHECK(!KnownProgressBankCopyProvenanceAt(i).runtimePayloadAuthority);
    }
}

void TestRuntimeGateFalse() {
    CHECK(!RuntimeCutoverAllowed());

    const ProgressBankPlan backup = BuildBackup80015700Plan();
    const ProgressBankPlan restore =
        BuildRestore80015744Plan(true, kProgressBank80092F10);
    const ProgressBankPlan load =
        BuildLoadPayload800164B4Plan(
            true,
            0x8007ADE8u,
            ProgressBankPayloadSourceKind::RuntimeLowerCardProducer,
            true);

    CHECK(!backup.runtimeCutoverAllowed);
    CHECK(!restore.runtimeCutoverAllowed);
    CHECK(!load.runtimeCutoverAllowed);
}

void TestCopyProvenanceCatalogFlags() {
    CHECK(KnownProgressBankCopyProvenanceCount() == 3);

    const ProgressBankCopyProvenanceSpec& backup =
        KnownProgressBankCopyProvenanceAt(0);
    CHECK(backup.kind == ProgressBankCopyProvenanceKind::Backup80015700);
    CHECK(backup.psxFunction == kFn80015700);
    CHECK(backup.srcAddress == kProgressBank80092F10);
    CHECK(backup.dstAddress == kBackupBank80079008);
    CHECK(backup.byteCount == kProgressBankByteCount);
    CHECK(backup.sourceAddressFixed);
    CHECK(backup.destinationAddressFixed);
    CHECK(backup.requiresKnownSource);
    CHECK(!backup.requiresKnownBackup);
    CHECK(!backup.lowerCardPayloadRequired);

    const ProgressBankCopyProvenanceSpec& restore =
        KnownProgressBankCopyProvenanceAt(1);
    CHECK(restore.kind == ProgressBankCopyProvenanceKind::Restore80015744);
    CHECK(restore.psxFunction == kFn80015744);
    CHECK(restore.srcAddress == kBackupBank80079008);
    CHECK(restore.dstAddress == kProgressBank80092F10);
    CHECK(restore.byteCount == kProgressBankByteCount);
    CHECK(restore.sourceAddressFixed);
    CHECK(restore.destinationAddressFixed);
    CHECK(restore.requiresKnownSource);
    CHECK(restore.requiresKnownBackup);
    CHECK(!restore.lowerCardPayloadRequired);

    const ProgressBankCopyProvenanceSpec& load =
        KnownProgressBankCopyProvenanceAt(2);
    CHECK(load.kind == ProgressBankCopyProvenanceKind::LoadPayload800164B4);
    CHECK(load.psxFunction == kFn800164B4);
    CHECK(load.dstAddress == kProgressBank80092F10);
    CHECK(load.byteCount == kProgressBankByteCount);
    CHECK(!load.sourceAddressFixed);
    CHECK(load.destinationAddressFixed);
    CHECK(load.requiresKnownSource);
    CHECK(!load.requiresKnownBackup);
    CHECK(load.lowerCardPayloadRequired);

    CheckNoRuntimePayloadAuthority();
}

void TestBackup80015700ExactSourceOnly() {
    const ProgressBankPlan ok = BuildBackup80015700Plan();
    CHECK(!ok.blockedByP0Gap);
    CHECK(CountActions(ok, ProgressBankActionKind::Gap) == 0);

    const ProgressBankAction* copy =
        FindAction(ok, ProgressBankActionKind::CopyBytes80015700);
    CHECK(copy != nullptr);
    if (copy != nullptr) {
        CHECK(copy->psxFunction == kFn80015700);
        CHECK(copy->srcAddress == kProgressBank80092F10);
        CHECK(copy->dstAddress == kBackupBank80079008);
        CHECK(copy->byteCount == kProgressBankByteCount);
    }

    const ProgressBankAction* gate =
        FindAction(ok, ProgressBankActionKind::GateBackupSource80015700);
    CHECK(gate != nullptr);
    if (gate != nullptr) {
        CHECK(gate->valueKnown);
        CHECK(gate->value == 1);
    }

    const ProgressBankPlan wrong =
        BuildBackup80015700Plan(kProgressBank80092F10 + 4u);
    CHECK(wrong.blockedByP0Gap);
    CHECK(CountActions(wrong, ProgressBankActionKind::Gap) == 1);
    gate = FindAction(wrong, ProgressBankActionKind::GateBackupSource80015700);
    CHECK(gate != nullptr);
    if (gate != nullptr) {
        CHECK(!gate->valueKnown);
        CHECK(gate->value == 0);
    }
}

void TestRestore80015744RequiresKnownBackupAndExactTarget() {
    const ProgressBankPlan ok =
        BuildRestore80015744Plan(true, kProgressBank80092F10);
    CHECK(!ok.blockedByP0Gap);
    CHECK(CountActions(ok, ProgressBankActionKind::Gap) == 0);

    const ProgressBankAction* gate =
        FindAction(ok, ProgressBankActionKind::GateRestoreTarget80015744);
    CHECK(gate != nullptr);
    if (gate != nullptr) {
        CHECK(gate->srcAddress == kBackupBank80079008);
        CHECK(gate->dstAddress == kProgressBank80092F10);
        CHECK(gate->valueKnown);
        CHECK(gate->value == 1);
    }

    const ProgressBankPlan unknownBackup =
        BuildRestore80015744Plan(false, kProgressBank80092F10);
    CHECK(unknownBackup.blockedByP0Gap);
    CHECK(CountActions(unknownBackup, ProgressBankActionKind::Gap) == 1);
    gate = FindAction(unknownBackup,
                      ProgressBankActionKind::GateRestoreTarget80015744);
    CHECK(gate != nullptr);
    if (gate != nullptr) {
        CHECK(!gate->valueKnown);
        CHECK(gate->value == 0);
    }

    const ProgressBankPlan wrongTarget =
        BuildRestore80015744Plan(true, kProgressBank80092F10 + 4u);
    CHECK(wrongTarget.blockedByP0Gap);
    CHECK(CountActions(wrongTarget, ProgressBankActionKind::Gap) == 1);
    gate = FindAction(wrongTarget,
                      ProgressBankActionKind::GateRestoreTarget80015744);
    CHECK(gate != nullptr);
    if (gate != nullptr) {
        CHECK(gate->valueKnown);
        CHECK(gate->value == 0);
    }
}

void TestLoadPayload800164B4RequiresTypedLowerCardSource() {
    constexpr uint32_t kTypedLowerCardPayload = 0x8007ADE8u;

    const ProgressBankPlan ok =
        BuildLoadPayload800164B4Plan(
            true,
            kTypedLowerCardPayload,
            ProgressBankPayloadSourceKind::RuntimeLowerCardProducer,
            true);
    CHECK(!ok.blockedByP0Gap);
    CHECK(CountActions(ok, ProgressBankActionKind::Gap) == 0);

    const ProgressBankAction* gate =
        FindAction(ok, ProgressBankActionKind::GateLoadPayloadSource800164B4);
    CHECK(gate != nullptr);
    if (gate != nullptr) {
        CHECK(gate->psxFunction == kFn800164B4);
        CHECK(gate->srcAddress == kTypedLowerCardPayload);
        CHECK(gate->dstAddress == kProgressBank80092F10);
        CHECK(gate->byteCount == kProgressBankByteCount);
        CHECK(gate->valueKnown);
        CHECK(gate->value == 1);
        CHECK(gate->payloadSource ==
              ProgressBankPayloadSourceKind::RuntimeLowerCardProducer);
    }

    const ProgressBankPlan knownPointerOnly =
        BuildLoadPayload800164B4Plan(
            true,
            kTypedLowerCardPayload,
            ProgressBankPayloadSourceKind::RuntimeLowerCardProducer,
            false);
    CHECK(knownPointerOnly.blockedByP0Gap);
    CHECK(CountActions(knownPointerOnly, ProgressBankActionKind::Gap) == 1);
    gate = FindAction(knownPointerOnly,
                      ProgressBankActionKind::GateLoadPayloadSource800164B4);
    CHECK(gate != nullptr);
    if (gate != nullptr) {
        CHECK(!gate->valueKnown);
        CHECK(gate->value == 0);
        CHECK(gate->payloadSource ==
              ProgressBankPayloadSourceKind::RuntimeLowerCardProducer);
    }

    const ProgressBankPlan unknownPointer =
        BuildLoadPayload800164B4Plan(
            false,
            kTypedLowerCardPayload,
            ProgressBankPayloadSourceKind::RuntimeLowerCardProducer,
            true);
    CHECK(unknownPointer.blockedByP0Gap);
    CHECK(CountActions(unknownPointer, ProgressBankActionKind::Gap) == 1);
    gate = FindAction(unknownPointer,
                      ProgressBankActionKind::GateLoadPayloadSource800164B4);
    CHECK(gate != nullptr);
    if (gate != nullptr) {
        CHECK(!gate->valueKnown);
        CHECK(gate->value == 0);
        CHECK(gate->payloadSource ==
              ProgressBankPayloadSourceKind::RuntimeLowerCardProducer);
    }

    const ProgressBankPayloadSourceKind rejectedSources[] = {
        ProgressBankPayloadSourceKind::Unknown,
        ProgressBankPayloadSourceKind::DebugSyntheticFixture,
        ProgressBankPayloadSourceKind::HostFilesystem,
    };
    for (ProgressBankPayloadSourceKind source : rejectedSources) {
        const ProgressBankPlan rejected =
            BuildLoadPayload800164B4Plan(true,
                                         kTypedLowerCardPayload,
                                         source,
                                         true);
        CHECK(rejected.blockedByP0Gap);
        CHECK(CountActions(rejected, ProgressBankActionKind::Gap) == 1);
        gate = FindAction(rejected,
                          ProgressBankActionKind::GateLoadPayloadSource800164B4);
        CHECK(gate != nullptr);
        if (gate != nullptr) {
            CHECK(!gate->valueKnown);
            CHECK(gate->value == 0);
            CHECK(gate->payloadSource == source);
        }
    }
}

void TestReplayRestore8001681CUsesCountBoundedPrefix() {
    const ProgressBankPlan zeroCount =
        BuildReplayRestore8001681CPlan(true, true, false, 0);
    CHECK(!zeroCount.blockedByP0Gap);
    CHECK(CountActions(zeroCount, ProgressBankActionKind::Gap) == 0);
    const ProgressBankAction* copy = FindAction(
        zeroCount, ProgressBankActionKind::CopyReplayMirrorRestore8001681C);
    CHECK(copy != nullptr);
    if (copy != nullptr) {
        CHECK(copy->byteCount == 0);
        CHECK(copy->valueKnown);
    }

    const ProgressBankPlan knownPrefix =
        BuildReplayRestore8001681CPlan(true, true, true, 53);
    CHECK(!knownPrefix.blockedByP0Gap);
    copy = FindAction(
        knownPrefix, ProgressBankActionKind::CopyReplayMirrorRestore8001681C);
    CHECK(copy != nullptr);
    if (copy != nullptr) {
        CHECK(copy->byteCount == 53u * kReplayMirrorEntryStride);
        CHECK(copy->valueKnown);
    }

    const ProgressBankPlan unknownPrefix =
        BuildReplayRestore8001681CPlan(true, true, false, 53);
    CHECK(unknownPrefix.blockedByP0Gap);
    CHECK(CountActions(unknownPrefix, ProgressBankActionKind::Gap) == 1);

    const ProgressBankPlan outOfRange = BuildReplayRestore8001681CPlan(
        true, true, true, kReplayMirrorMaxEntryCount + 1u);
    CHECK(outOfRange.blockedByP0Gap);
    copy = FindAction(
        outOfRange, ProgressBankActionKind::CopyReplayMirrorRestore8001681C);
    CHECK(copy != nullptr);
    if (copy != nullptr) {
        CHECK(copy->byteCount == 0);
        CHECK(!copy->valueKnown);
    }
}

void TestStageStatusGateRequiresKnownMappedStatus() {
    const ProgressBankPlan clear =
        BuildStageStatusGate8001670CPlan(1, true, 2);
    CHECK(!clear.blockedByP0Gap);
    CHECK(CountActions(clear, ProgressBankActionKind::Gap) == 0);

    const ProgressBankAction* gate =
        FindAction(clear, ProgressBankActionKind::GateStageStatusSource8001670C);
    CHECK(gate != nullptr);
    if (gate != nullptr) {
        CHECK(gate->psxFunction == kFn8001670C);
        CHECK(gate->srcAddress == kStatusBank80092F1D);
        CHECK(gate->stageOrSelector == 1);
        CHECK(gate->slot == 0);
        CHECK(gate->value == 2);
        CHECK(gate->valueKnown);
        CHECK(gate->conditional);
    }

    const ProgressBankAction* read =
        FindAction(clear, ProgressBankActionKind::ReadStageStatusGate8001670C);
    CHECK(read != nullptr);
    if (read != nullptr) {
        CHECK(read->value == 1);
        CHECK(read->valueKnown);
        CHECK(read->conditional);
    }

    const ProgressBankPlan unlocked =
        BuildStageStatusGate8001670CPlan(1, true, 1);
    read = FindAction(unlocked,
                      ProgressBankActionKind::ReadStageStatusGate8001670C);
    CHECK(read != nullptr);
    if (read != nullptr) {
        CHECK(read->value == 0);
        CHECK(read->valueKnown);
    }

    const ProgressBankPlan unknown =
        BuildStageStatusGate8001670CPlan(1, false, 3);
    CHECK(unknown.blockedByP0Gap);
    CHECK(CountActions(unknown, ProgressBankActionKind::Gap) == 1);
    gate = FindAction(unknown,
                      ProgressBankActionKind::GateStageStatusSource8001670C);
    read = FindAction(unknown,
                      ProgressBankActionKind::ReadStageStatusGate8001670C);
    CHECK(gate != nullptr);
    CHECK(read != nullptr);
    if (gate != nullptr) {
        CHECK(gate->value == 3);
        CHECK(!gate->valueKnown);
    }
    if (read != nullptr) {
        CHECK(!read->valueKnown);
        CHECK(read->conditional);
    }

    const ProgressBankPlan unmapped =
        BuildStageStatusGate8001670CPlan(99, true, 2);
    CHECK(unmapped.blockedByP0Gap);
    CHECK(CountActions(unmapped, ProgressBankActionKind::Gap) == 1);
    CHECK(FindAction(unmapped,
                     ProgressBankActionKind::GateStageStatusSource8001670C) ==
          nullptr);
    CHECK(FindAction(unmapped,
                     ProgressBankActionKind::ReadStageStatusGate8001670C) ==
          nullptr);
}

void TestSetupGateDefaultBranchPropagatesUnknownStatus() {
    const ProgressBankPlan plan =
        BuildSetupGateSource801C7A60Plan(
            true,
            0,
            1,
            false,
            3,
            true,
            0,
            true,
            0,
            true,
            true,
            true,
            0);
    CHECK(plan.blockedByP0Gap);
    CHECK(CountActions(plan, ProgressBankActionKind::Gap) == 2);

    const ProgressBankAction* statusGate =
        FindAction(plan, ProgressBankActionKind::GateStageStatusSource8001670C);
    CHECK(statusGate != nullptr);
    if (statusGate != nullptr) {
        CHECK(statusGate->value == 3);
        CHECK(!statusGate->valueKnown);
    }

    const ProgressBankAction* latchGate =
        FindAction(plan, ProgressBankActionKind::GateSetupLatchSource801C7A60);
    CHECK(latchGate != nullptr);
    if (latchGate != nullptr) {
        CHECK(latchGate->value == 1);
        CHECK(!latchGate->valueKnown);
        CHECK(latchGate->conditional);
    }

    const ProgressBankAction* firstWrite =
        FindAction(plan, ProgressBankActionKind::WriteWord8009182A800143F0);
    const ProgressBankAction* secondWrite =
        FindAction(plan, ProgressBankActionKind::WriteWord8008ED34800259C0);
    CHECK(firstWrite != nullptr);
    CHECK(secondWrite != nullptr);
    if (firstWrite != nullptr) {
        CHECK(firstWrite->value == 1);
        CHECK(!firstWrite->valueKnown);
    }
    if (secondWrite != nullptr) {
        CHECK(secondWrite->value == 1);
        CHECK(!secondWrite->valueKnown);
    }
}

} // namespace

int main() {
    TestRuntimeGateFalse();
    TestCopyProvenanceCatalogFlags();
    TestBackup80015700ExactSourceOnly();
    TestRestore80015744RequiresKnownBackupAndExactTarget();
    TestLoadPayload800164B4RequiresTypedLowerCardSource();
    TestReplayRestore8001681CUsesCountBoundedPrefix();
    TestStageStatusGateRequiresKnownMappedStatus();
    TestSetupGateDefaultBranchPropagatesUnknownStatus();

    if (g_failed != 0) {
        std::printf(
            "test_ss0_stage_progress_copy_provenance: failed checks=%d\n",
            g_failed);
        return 1;
    }
    std::printf("test_ss0_stage_progress_copy_provenance: ok\n");
    return 0;
}
