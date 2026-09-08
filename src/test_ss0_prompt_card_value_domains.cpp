#include "pr/pr_ss0_prompt_card_render_direct.h"

#include <cstdint>
#include <cstdio>

namespace {

using namespace PrSS0PromptCardRenderDirect;

int g_failedChecks = 0;

#define CHECK(condition)                                                        \
    do {                                                                        \
        if (!(condition)) {                                                     \
            std::printf("CHECK failed: %s:%d: %s\n", __FILE__, __LINE__,      \
                        #condition);                                            \
            ++g_failedChecks;                                                   \
        }                                                                       \
    } while (false)

const PromptCardAction* FindAction(const PromptCardPlan& plan,
                                   PromptCardActionKind kind) {
    for (uint32_t index = 0; index < plan.count; ++index) {
        if (plan.actions[index].kind == kind) {
            return &plan.actions[index];
        }
    }
    return nullptr;
}

void TestCardInfoKnownNegativeLowerModeIsPreserved() {
    const PromptCardPlan plan = BuildCardInfo80020BE4Plan(
        0x80049244u, true, 56, -32768, -1, true, true);
    const PromptCardAction* gate =
        FindAction(plan, PromptCardActionKind::GateCardInfoLowerState80020BE4);
    CHECK(gate != nullptr);
    if (!gate) {
        return;
    }
    CHECK(gate->args[0] == 56);
    CHECK(gate->args[1] == -32768);
    CHECK(gate->args[2] == -1);
    CHECK(gate->args[3] == 7);
}

void TestCardInfoUnknownLowerModeRemainsUnknown() {
    const PromptCardPlan plan = BuildCardInfo80020BE4Plan(
        0x80049244u, true, 56, -32768, 0, true, false);
    const PromptCardAction* gate =
        FindAction(plan, PromptCardActionKind::GateCardInfoLowerState80020BE4);
    CHECK(gate != nullptr);
    if (!gate) {
        return;
    }
    CHECK(gate->args[1] == -1);
    CHECK(gate->args[3] == 5);
}

void TestHiScoreKnownNoncanonicalExitStatesArePreserved() {
    const PromptCardPlan plan = BuildHiScore80021594Plan(
        0x80049278u, true, 6, 3, -7, 42, true, true);
    const PromptCardAction* gate =
        FindAction(plan, PromptCardActionKind::GateHiScoreExitState80021594);
    CHECK(gate != nullptr);
    if (!gate) {
        return;
    }
    CHECK(gate->args[0] == -7);
    CHECK(gate->args[1] == 42);
    CHECK(gate->args[2] == 15);
}

void TestHiScoreUnknownExitStatesRemainUnknown() {
    const PromptCardPlan plan = BuildHiScore80021594Plan(
        0x80049278u, true, 6, 3, -7, 42, false, false);
    const PromptCardAction* gate =
        FindAction(plan, PromptCardActionKind::GateHiScoreExitState80021594);
    CHECK(gate != nullptr);
    if (!gate) {
        return;
    }
    CHECK(gate->args[0] == -1);
    CHECK(gate->args[1] == -1);
    CHECK(gate->args[2] == 3);
}

}  // namespace

int main() {
    TestCardInfoKnownNegativeLowerModeIsPreserved();
    TestCardInfoUnknownLowerModeRemainsUnknown();
    TestHiScoreKnownNoncanonicalExitStatesArePreserved();
    TestHiScoreUnknownExitStatesRemainUnknown();
    if (g_failedChecks != 0) {
        std::printf("test_ss0_prompt_card_value_domains: %d failure(s)\n",
                    g_failedChecks);
        return 1;
    }
    std::printf("test_ss0_prompt_card_value_domains: ok\n");
    return 0;
}
