#include "pr/pr_stage_runner_direct.h"

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

PrStageRunnerDirectFinalReturn7A60Result Resolve(
    const PrStageRunnerDirectFinalReturn7A60Input& input) {
    return PrStageRunnerDirectResolveFinalReturn7A60(input);
}

void TestCurrentFailLatchShortCircuitsBefore166AC() {
    PrStageRunnerDirectFinalReturn7A60Input input{};
    input.word59Known = true;
    input.word59IsOne = true;
    input.word60Known = true;
    input.word60IsOne = false;
    input.stageStatus166ACKnown = true;
    input.stageStatus166AC = 2;

    const PrStageRunnerDirectFinalReturn7A60Result result = Resolve(input);

    CHECK(result.resultKnown);
    CHECK(result.result == 2);
    CHECK(result.word59Known);
    CHECK(result.word59IsOne);
    CHECK(result.word60Known);
    CHECK(!result.word60IsOne);
    CHECK(!result.calls166AC);
    CHECK(!result.recordsModeReturnsOne);
    CHECK(!result.stageStatusReturnsOne);
}

void TestUnknownWord59KeepsResultUnknown() {
    PrStageRunnerDirectFinalReturn7A60Input input{};
    input.word59Known = false;
    input.word60Known = true;
    input.word60IsOne = true;

    const PrStageRunnerDirectFinalReturn7A60Result result = Resolve(input);

    CHECK(!result.resultKnown);
    CHECK(!result.calls166AC);
}

void TestUnknownWord60KeepsResultUnknownAfterWord59False() {
    PrStageRunnerDirectFinalReturn7A60Input input{};
    input.word59Known = true;
    input.word59IsOne = false;
    input.word60Known = false;

    const PrStageRunnerDirectFinalReturn7A60Result result = Resolve(input);

    CHECK(!result.resultKnown);
    CHECK(result.result == 3);
    CHECK(!result.calls166AC);
}

void TestWord60FalseReturnsThreeWithout166AC() {
    PrStageRunnerDirectFinalReturn7A60Input input{};
    input.word59Known = true;
    input.word59IsOne = false;
    input.word60Known = true;
    input.word60IsOne = false;

    const PrStageRunnerDirectFinalReturn7A60Result result = Resolve(input);

    CHECK(result.resultKnown);
    CHECK(result.result == 3);
    CHECK(!result.calls166AC);
}

void TestRecordsModeOneReturnsOneAfterWord60Open() {
    PrStageRunnerDirectFinalReturn7A60Input input{};
    input.word59Known = true;
    input.word59IsOne = false;
    input.word60Known = true;
    input.word60IsOne = true;
    input.recordsModeDAEqualsOne = true;

    const PrStageRunnerDirectFinalReturn7A60Result result = Resolve(input);

    CHECK(result.resultKnown);
    CHECK(result.result == 1);
    CHECK(result.calls166AC);
    CHECK(result.recordsModeReturnsOne);
    CHECK(!result.stageStatusReturnsOne);
    CHECK(result.gate78OpenResultKnown);
    CHECK(result.gate78OpenResult == 1);
}

void TestStageStatusBelowFourReturnsOneAfterWord60Open() {
    PrStageRunnerDirectFinalReturn7A60Input input{};
    input.word59Known = true;
    input.word59IsOne = false;
    input.word60Known = true;
    input.word60IsOne = true;
    input.stageStatus166ACKnown = true;
    input.stageStatus166AC = 3;

    const PrStageRunnerDirectFinalReturn7A60Result result = Resolve(input);

    CHECK(result.resultKnown);
    CHECK(result.result == 1);
    CHECK(result.calls166AC);
    CHECK(!result.recordsModeReturnsOne);
    CHECK(result.stageStatusReturnsOne);
    CHECK(result.gate78OpenResultKnown);
    CHECK(result.gate78OpenResult == 1);
}

void TestStageStatusFourReturnsTwoAfterWord60Open() {
    PrStageRunnerDirectFinalReturn7A60Input input{};
    input.word59Known = true;
    input.word59IsOne = false;
    input.word60Known = true;
    input.word60IsOne = true;
    input.stageStatus166ACKnown = true;
    input.stageStatus166AC = 4;

    const PrStageRunnerDirectFinalReturn7A60Result result = Resolve(input);

    CHECK(result.resultKnown);
    CHECK(result.result == 2);
    CHECK(result.calls166AC);
    CHECK(!result.recordsModeReturnsOne);
    CHECK(!result.stageStatusReturnsOne);
    CHECK(result.gate78OpenResultKnown);
    CHECK(result.gate78OpenResult == 2);
}

void TestUnknownStageStatusKeepsResultUnknownAfterWord60Open() {
    PrStageRunnerDirectFinalReturn7A60Input input{};
    input.word59Known = true;
    input.word59IsOne = false;
    input.word60Known = true;
    input.word60IsOne = true;
    input.stageStatus166ACKnown = false;

    const PrStageRunnerDirectFinalReturn7A60Result result = Resolve(input);

    CHECK(!result.resultKnown);
    CHECK(result.result == 2);
    CHECK(result.calls166AC);
    CHECK(!result.recordsModeReturnsOne);
    CHECK(!result.stageStatusReturnsOne);
}

} // namespace

int main() {
    TestCurrentFailLatchShortCircuitsBefore166AC();
    TestUnknownWord59KeepsResultUnknown();
    TestUnknownWord60KeepsResultUnknownAfterWord59False();
    TestWord60FalseReturnsThreeWithout166AC();
    TestRecordsModeOneReturnsOneAfterWord60Open();
    TestStageStatusBelowFourReturnsOneAfterWord60Open();
    TestStageStatusFourReturnsTwoAfterWord60Open();
    TestUnknownStageStatusKeepsResultUnknownAfterWord60Open();

    if (g_failed != 0) {
        std::printf(
            "test_ss0_stage1_runner_final_return_gate: failed checks=%d\n",
            g_failed);
        return 1;
    }
    std::printf("test_ss0_stage1_runner_final_return_gate: ok\n");
    return 0;
}
