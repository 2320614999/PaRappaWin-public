#include "pr/pr_stage_status_bank_direct.h"

#include <cstdio>

namespace {

int g_failed = 0;

#define CHECK(expr)                                                           \
    do {                                                                      \
        if (!(expr)) {                                                        \
            std::printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr);       \
            ++g_failed;                                                       \
        }                                                                     \
    } while (0)

const PrStageStatusBankAction* FindAction(
    const PrStageStatusBankActionTrace& trace,
    PrStageStatusBankActionKind kind) {
    for (size_t i = 0; i < trace.count; ++i) {
        if (trace.actions[i].kind == kind) {
            return &trace.actions[i];
        }
    }
    return nullptr;
}

uint32_t CountActions(const PrStageStatusBankActionTrace& trace,
                      PrStageStatusBankActionKind kind) {
    uint32_t count = 0;
    for (size_t i = 0; i < trace.count; ++i) {
        if (trace.actions[i].kind == kind) {
            ++count;
        }
    }
    return count;
}

PrStageStatusBankClearProducerInput BasePostClearInput() {
    PrStageStatusBankClearProducerInput input{};
    input.comod = PrStageStatusBankComod::Comod1;
    input.stageArg = 1;
    input.word800916D0 = 0;
    input.word800916DA = 0;
    input.word80091816 = 1234;
    input.previousStatusKnown800166AC = true;
    input.previousStatus800166AC = 2;
    return input;
}

void CheckNoPostClearSaveActions(
    const PrStageStatusBankClearProducerResult& result) {
    CHECK(FindAction(result.trace, PrStageStatusBankActionKind::Call80015590) ==
          nullptr);
    CHECK(FindAction(result.trace, PrStageStatusBankActionKind::Call80019148) ==
          nullptr);
    CHECK(!result.saveMenuCalled);
}

void CheckPostClearStatusActions(
    const PrStageStatusBankClearProducerResult& result) {
    CHECK(result.statusProducerCalled8001635C);
    CHECK(result.nextStageUnlockCalled8001628C);
    CHECK(result.nextStageUnlockArg == 2);
    const PrStageStatusBankAction* status =
        FindAction(result.trace, PrStageStatusBankActionKind::Call8001635C);
    CHECK(status != nullptr);
    if (status != nullptr) {
        CHECK(status->arg0Known);
        CHECK(status->arg0 == 1);
        CHECK(status->arg1Known);
        CHECK(status->arg1 == 2);
        CHECK(status->arg2Known);
        CHECK(status->arg2 == 2);
        CHECK(status->arg3Known);
        CHECK(status->arg3 == 1234);
    }
}

void TestUnknownF0BlocksNormalPostClearSaveGate() {
    PrStageStatusBankClearProducerInput input = BasePostClearInput();
    input.word800916F0Known = false;
    input.word800916F0 = 0;

    const PrStageStatusBankClearProducerResult result =
        PrStageStatusBankDirectClearProducer(input);
    CheckPostClearStatusActions(result);
    CheckNoPostClearSaveActions(result);
    CHECK(result.blockedByUnknownWord800916F0);
    CHECK(!result.returnValueKnown);
    CHECK(result.returnValue == 2);
}

void TestKnownZeroAllowsNormalPostClearSaveGate() {
    PrStageStatusBankClearProducerInput input = BasePostClearInput();
    input.word800916F0Known = true;
    input.word800916F0 = 0;

    const PrStageStatusBankClearProducerResult result =
        PrStageStatusBankDirectClearProducer(input);
    CheckPostClearStatusActions(result);
    CHECK(!result.blockedByUnknownWord800916F0);
    CHECK(result.returnValueKnown);
    CHECK(result.returnValue == 2);
    CHECK(result.saveMenuCalled);
    CHECK(CountActions(result.trace, PrStageStatusBankActionKind::Call80015590) ==
          1);
    CHECK(CountActions(result.trace, PrStageStatusBankActionKind::Call80019148) ==
          1);
}

void TestKnownOneSkipsNormalPostClearSaveGate() {
    PrStageStatusBankClearProducerInput input = BasePostClearInput();
    input.word800916F0Known = true;
    input.word800916F0 = 1;

    const PrStageStatusBankClearProducerResult result =
        PrStageStatusBankDirectClearProducer(input);
    CheckPostClearStatusActions(result);
    CheckNoPostClearSaveActions(result);
    CHECK(!result.blockedByUnknownWord800916F0);
    CHECK(result.returnValueKnown);
    CHECK(result.returnValue == 2);
}

void TestUnknownF0BlocksTerminalComod7SaveGate() {
    PrStageStatusBankClearProducerInput input = BasePostClearInput();
    input.comod = PrStageStatusBankComod::Comod7;
    input.stageArg = 6;
    input.word800916F0Known = false;

    const PrStageStatusBankClearProducerResult result =
        PrStageStatusBankDirectClearProducer(input);
    CHECK(result.statusProducerCalled8001635C);
    CHECK(!result.nextStageUnlockCalled8001628C);
    CheckNoPostClearSaveActions(result);
    CHECK(result.blockedByUnknownWord800916F0);
    CHECK(!result.returnValueKnown);
}

void TestKnownZeroAllowsTerminalComod7SaveGate() {
    PrStageStatusBankClearProducerInput input = BasePostClearInput();
    input.comod = PrStageStatusBankComod::Comod7;
    input.stageArg = 6;
    input.word800916F0Known = true;
    input.word800916F0 = 0;

    const PrStageStatusBankClearProducerResult result =
        PrStageStatusBankDirectClearProducer(input);
    CHECK(result.statusProducerCalled8001635C);
    CHECK(!result.nextStageUnlockCalled8001628C);
    CHECK(!result.blockedByUnknownWord800916F0);
    CHECK(result.returnValueKnown);
    CHECK(result.returnValue == 1);
    CHECK(result.saveMenuCalled);
    CHECK(CountActions(result.trace, PrStageStatusBankActionKind::Call80015590) ==
          1);
    CHECK(CountActions(result.trace, PrStageStatusBankActionKind::Call80019148) ==
          1);
}

void TestKnownOneSkipsTerminalComod7SaveGate() {
    PrStageStatusBankClearProducerInput input = BasePostClearInput();
    input.comod = PrStageStatusBankComod::Comod7;
    input.stageArg = 6;
    input.word800916F0Known = true;
    input.word800916F0 = 1;

    const PrStageStatusBankClearProducerResult result =
        PrStageStatusBankDirectClearProducer(input);
    CHECK(result.statusProducerCalled8001635C);
    CHECK(!result.nextStageUnlockCalled8001628C);
    CheckNoPostClearSaveActions(result);
    CHECK(!result.blockedByUnknownWord800916F0);
    CHECK(result.returnValueKnown);
    CHECK(result.returnValue == 0);
}

} // namespace

int main() {
    TestUnknownF0BlocksNormalPostClearSaveGate();
    TestKnownZeroAllowsNormalPostClearSaveGate();
    TestKnownOneSkipsNormalPostClearSaveGate();
    TestUnknownF0BlocksTerminalComod7SaveGate();
    TestKnownZeroAllowsTerminalComod7SaveGate();
    TestKnownOneSkipsTerminalComod7SaveGate();

    if (g_failed != 0) {
        std::printf("test_ss0_stage_status_bank_f0_gate: failed checks=%d\n",
                    g_failed);
        return 1;
    }
    std::printf("test_ss0_stage_status_bank_f0_gate: ok\n");
    return 0;
}
