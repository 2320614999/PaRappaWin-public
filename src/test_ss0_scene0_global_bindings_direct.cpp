#include "pr/pr_ss0_scene0_global_bindings_direct.h"

#include <cstdio>

namespace {

using namespace PrSS0Scene0GlobalBindingsDirect;

static_assert(kFn801C5B14 == 0x801C5B14u,
              "Fn0 address must remain exact");
static_assert(kFn801C4260 == 0x801C4260u,
              "Fn1 address must remain exact");
static_assert(kInitCallCount801C4260 == 5u,
              "Fn1 call count must remain exact");

int g_failed = 0;

#define CHECK(expr)                                                           \
    do {                                                                      \
        if (!(expr)) {                                                        \
            std::printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr);       \
            ++g_failed;                                                       \
        }                                                                     \
    } while (0)

void CheckEveryField(const State801C5B14& state)
{
    CHECK(state.known);
    CHECK(state.complete);
    CHECK(state.hostProjection);
    CHECK(state.returnValueKnown);
    CHECK(state.returnValue == 0u);
    CHECK(!state.psxMemoryBackingAuthority);
    CHECK(!state.oldWinS0Authority);
    CHECK(!state.stage1Authority);
    CHECK(!state.stage2PlusAuthority);
    CHECK(!state.replayValueAuthority);
    CHECK(!state.hostFilesystemAuthority);

    CHECK(state.slot800943C0 == 0x801C6F88u);
    CHECK(state.slot800943C4 == 0x801C6F68u);
    CHECK(state.slot800943C8 == 1u);
    CHECK(state.slot800943CC == 0x801C6BF8u);
    CHECK(state.slot800943D0 == 0x801C6DA4u);
    CHECK(state.slot800943D4 == 1u);
    CHECK(state.slot800943D8 == 0x801C6EF0u);
    CHECK(state.slot800943DC == 1u);
    CHECK(state.slot800943E0 == 0x801C6F18u);
    CHECK(state.slot800943E4 == 0x801C6F84u);
    CHECK(state.slot800943E8 == 0u);
    CHECK(state.slot800943EC == 0x801C6E5Cu);
    CHECK(state.slot800943F0 == 0x801C6E64u);
    CHECK(state.slot800943F4 == 0x801C6E6Cu);
    CHECK(state.slot800943F8 == 0x801C6E74u);
    CHECK(state.slot800943FC == 0x801C6E88u);
    CHECK(state.slot80094400 == 0x801C6E7Cu);
    CHECK(state.slot80094404 == 0x801C6E94u);
    CHECK(state.slot80094408 == 0x801C6E9Cu);
    CHECK(state.slot8009440C == 0x801C6EA4u);
    CHECK(state.slot80094410 == 0x801C6EACu);
    CHECK(state.slot80094414 == 0x801C6EB4u);
    CHECK(state.slot80094418 == 0x801C6EBCu);
    CHECK(state.slot8009441C == 0x801C6EC4u);
    CHECK(state.slot80094420 == 0x801C6ED0u);
    CHECK(state.slot80094424 == 0x801C6ED8u);
    CHECK(state.slot80094428 == 0x801C6EE0u);
    CHECK(state.slot8009442C == 0x801C6EE8u);
    CHECK(state.slot80094430 == 0x801C57C8u);
    CHECK(state.slot80094434 == 0x801C57D0u);
    CHECK(state.slot80094438 == 0x801C57D8u);
    CHECK(state.slot8009443C == 0x801C5B0Cu);
    CHECK(state.slot80094440 == 0x801C4FC8u);
}

void ExpectRejected(const State801C5B14& state)
{
    CHECK(!IsExactState801C5B14(state));
}

void TestExactState()
{
    const State801C5B14 state = BuildState801C5B14();
    CheckEveryField(state);
    CHECK(IsExactState801C5B14(state));
    CHECK(IsExactState801C5B14(BuildState801C5B14()));
}

void TestRejectsMutations()
{
    State801C5B14 state = BuildState801C5B14();
    state.slot800943C0 ^= 4u;
    ExpectRejected(state);

    state = BuildState801C5B14();
    state.slot800943C8 = 0u;
    ExpectRejected(state);

    state = BuildState801C5B14();
    state.slot8009442C ^= 4u;
    ExpectRejected(state);

    state = BuildState801C5B14();
    state.slot80094440 ^= 4u;
    ExpectRejected(state);

    state = BuildState801C5B14();
    state.known = false;
    ExpectRejected(state);

    state = BuildState801C5B14();
    state.complete = false;
    ExpectRejected(state);

    state = BuildState801C5B14();
    state.hostProjection = false;
    ExpectRejected(state);

    state = BuildState801C5B14();
    state.returnValueKnown = false;
    ExpectRejected(state);

    state = BuildState801C5B14();
    state.returnValue = 1u;
    ExpectRejected(state);

    state = BuildState801C5B14();
    state.psxMemoryBackingAuthority = true;
    ExpectRejected(state);

    state = BuildState801C5B14();
    state.oldWinS0Authority = true;
    ExpectRejected(state);

    state = BuildState801C5B14();
    state.stage1Authority = true;
    ExpectRejected(state);

    state = BuildState801C5B14();
    state.stage2PlusAuthority = true;
    ExpectRejected(state);

    state = BuildState801C5B14();
    state.replayValueAuthority = true;
    ExpectRejected(state);

    state = BuildState801C5B14();
    state.hostFilesystemAuthority = true;
    ExpectRejected(state);
}

InitSource801C4260 ExactInitSource()
{
    InitSource801C4260 source{};
    source.sceneIndexKnown = true;
    source.sceneIndex = 0u;
    source.sceneEntryBaseKnown = true;
    source.sceneEntryBase = 0x8005474Cu;
    source.sceneHeaderKnown = true;
    source.bpm100 = 9600u;
    source.tickOffset = 96;
    source.extraTick = 0;
    source.resetSourcesKnown = true;
    source.reset80025A34Ready = true;
    source.reset801C4FA0Ready = true;
    source.reset80024E98Ready = true;
    source.reset80014344Ready = true;
    source.resourceIngress801C4780Known = true;
    source.resourceIngress801C4780Ready = true;
    return source;
}

void CheckEveryInitField(const InitTransaction801C4260& transaction)
{
    CHECK(transaction.status == InitStatus801C4260::Accepted);
    CHECK(transaction.accepted);
    CHECK(transaction.complete);
    CHECK(transaction.hostProjection);
    CHECK(!transaction.psxMemoryBackingAuthority);
    CHECK(!transaction.oldWinS0Authority);
    CHECK(!transaction.stage1Authority);
    CHECK(!transaction.stage2PlusAuthority);
    CHECK(!transaction.replayValueAuthority);
    CHECK(!transaction.hostFilesystemAuthority);

    CHECK(transaction.callCount == 5u);
    CHECK(transaction.callOrder[0] == 0x80025A34u);
    CHECK(transaction.callOrder[1] == 0x801C4FA0u);
    CHECK(transaction.callOrder[2] == 0x80024E98u);
    CHECK(transaction.callOrder[3] == 0x80014344u);
    CHECK(transaction.callOrder[4] == 0x801C4780u);

    CHECK(transaction.sceneEntryPointerDestination == 0x8006EDB8u);
    CHECK(transaction.sceneEntryPointerValue == 0x8005474Cu);
    CHECK(transaction.tickPerMinFieldAddress == 0x800548A8u);
    CHECK(transaction.tickPerMinValue == 9216u);
    CHECK(transaction.tickPerFrameFieldAddress == 0x800548ACu);
    CHECK(transaction.tickPerFrameValue == 3u);
    CHECK(transaction.baseTickFieldAddress == 0x800548B0u);
    CHECK(transaction.baseTickValue == 96u);
    CHECK(transaction.gridColumnsFieldAddress == 0x800548B4u);
    CHECK(transaction.gridColumnsValue == 16u);
    CHECK(transaction.timingPublishedBeforeResourceIngress);
    CHECK(transaction.preopenRowCount == 7u);
    CHECK(transaction.compoRowIndex == 1u);
    CHECK(transaction.resourceIngressLoadArg == 0u);
}

void CheckRejectedInitTransaction(
    const InitTransaction801C4260& transaction,
    InitStatus801C4260 expectedStatus)
{
    CHECK(transaction.status == expectedStatus);
    CHECK(!transaction.accepted);
    CHECK(!transaction.complete);
    CHECK(!transaction.hostProjection);
    CHECK(!transaction.psxMemoryBackingAuthority);
    CHECK(!transaction.oldWinS0Authority);
    CHECK(!transaction.stage1Authority);
    CHECK(!transaction.stage2PlusAuthority);
    CHECK(!transaction.replayValueAuthority);
    CHECK(!transaction.hostFilesystemAuthority);
    CHECK(transaction.callCount == 0u);
    for (uint32_t call : transaction.callOrder) {
        CHECK(call == 0u);
    }
    CHECK(transaction.sceneEntryPointerDestination == 0u);
    CHECK(transaction.sceneEntryPointerValue == 0u);
    CHECK(transaction.tickPerMinFieldAddress == 0u);
    CHECK(transaction.tickPerMinValue == 0u);
    CHECK(transaction.tickPerFrameFieldAddress == 0u);
    CHECK(transaction.tickPerFrameValue == 0u);
    CHECK(transaction.baseTickFieldAddress == 0u);
    CHECK(transaction.baseTickValue == 0u);
    CHECK(transaction.gridColumnsFieldAddress == 0u);
    CHECK(transaction.gridColumnsValue == 0u);
    CHECK(!transaction.timingPublishedBeforeResourceIngress);
    CHECK(transaction.preopenRowCount == 0u);
    CHECK(transaction.compoRowIndex == 0u);
    CHECK(transaction.resourceIngressLoadArg == 0u);
    CHECK(!IsExactInitTransaction801C4260(transaction));
}

void TestExactInitTransaction()
{
    const InitTransaction801C4260 transaction =
        BuildInitTransaction801C4260(ExactInitSource());
    CheckEveryInitField(transaction);
    CHECK(IsExactInitTransaction801C4260(transaction));
}

void TestInitRejectsUnknownSources()
{
    CheckRejectedInitTransaction(
        BuildInitTransaction801C4260(InitSource801C4260{}),
        InitStatus801C4260::SourceUnknown);

    InitSource801C4260 source = ExactInitSource();
    source.sceneIndexKnown = false;
    CheckRejectedInitTransaction(
        BuildInitTransaction801C4260(source),
        InitStatus801C4260::SourceUnknown);

    source = ExactInitSource();
    source.sceneEntryBaseKnown = false;
    CheckRejectedInitTransaction(
        BuildInitTransaction801C4260(source),
        InitStatus801C4260::SourceUnknown);

    source = ExactInitSource();
    source.sceneHeaderKnown = false;
    CheckRejectedInitTransaction(
        BuildInitTransaction801C4260(source),
        InitStatus801C4260::SourceUnknown);

    source = ExactInitSource();
    source.resetSourcesKnown = false;
    CheckRejectedInitTransaction(
        BuildInitTransaction801C4260(source),
        InitStatus801C4260::SourceUnknown);

    source = ExactInitSource();
    source.resourceIngress801C4780Known = false;
    CheckRejectedInitTransaction(
        BuildInitTransaction801C4260(source),
        InitStatus801C4260::SourceUnknown);
}

void TestInitRejectsMalformedSources()
{
    InitSource801C4260 source = ExactInitSource();
    source.sceneIndex = 1u;
    CheckRejectedInitTransaction(
        BuildInitTransaction801C4260(source),
        InitStatus801C4260::MalformedSource);

    source = ExactInitSource();
    source.sceneEntryBase += 0x16Cu;
    CheckRejectedInitTransaction(
        BuildInitTransaction801C4260(source),
        InitStatus801C4260::MalformedSource);

    source = ExactInitSource();
    source.bpm100 = 9599u;
    CheckRejectedInitTransaction(
        BuildInitTransaction801C4260(source),
        InitStatus801C4260::MalformedSource);

    source = ExactInitSource();
    source.tickOffset = 95;
    CheckRejectedInitTransaction(
        BuildInitTransaction801C4260(source),
        InitStatus801C4260::MalformedSource);

    source = ExactInitSource();
    source.extraTick = 1;
    CheckRejectedInitTransaction(
        BuildInitTransaction801C4260(source),
        InitStatus801C4260::MalformedSource);
}

void TestInitRejectsUnavailableResets()
{
    InitSource801C4260 source = ExactInitSource();
    source.reset80025A34Ready = false;
    CheckRejectedInitTransaction(
        BuildInitTransaction801C4260(source),
        InitStatus801C4260::ResetUnavailable);

    source = ExactInitSource();
    source.reset801C4FA0Ready = false;
    CheckRejectedInitTransaction(
        BuildInitTransaction801C4260(source),
        InitStatus801C4260::ResetUnavailable);

    source = ExactInitSource();
    source.reset80024E98Ready = false;
    CheckRejectedInitTransaction(
        BuildInitTransaction801C4260(source),
        InitStatus801C4260::ResetUnavailable);

    source = ExactInitSource();
    source.reset80014344Ready = false;
    CheckRejectedInitTransaction(
        BuildInitTransaction801C4260(source),
        InitStatus801C4260::ResetUnavailable);
}

void TestInitRejectsUnavailableResourceIngress()
{
    InitSource801C4260 source = ExactInitSource();
    source.resourceIngress801C4780Ready = false;
    CheckRejectedInitTransaction(
        BuildInitTransaction801C4260(source),
        InitStatus801C4260::ResourceIngressUnavailable);
}

void ExpectRejectedInitMutation(const InitTransaction801C4260& transaction)
{
    CHECK(!IsExactInitTransaction801C4260(transaction));
}

void TestInitRejectsTransactionMutations()
{
    InitTransaction801C4260 transaction =
        BuildInitTransaction801C4260(ExactInitSource());
    transaction.status = InitStatus801C4260::MalformedSource;
    ExpectRejectedInitMutation(transaction);

    transaction = BuildInitTransaction801C4260(ExactInitSource());
    transaction.accepted = false;
    ExpectRejectedInitMutation(transaction);

    transaction = BuildInitTransaction801C4260(ExactInitSource());
    transaction.complete = false;
    ExpectRejectedInitMutation(transaction);

    transaction = BuildInitTransaction801C4260(ExactInitSource());
    transaction.hostProjection = false;
    ExpectRejectedInitMutation(transaction);

    for (std::size_t index = 0u; index < kInitCallCount801C4260; ++index) {
        transaction = BuildInitTransaction801C4260(ExactInitSource());
        transaction.callOrder[index] ^= 4u;
        ExpectRejectedInitMutation(transaction);
    }

    transaction = BuildInitTransaction801C4260(ExactInitSource());
    transaction.callCount = 4u;
    ExpectRejectedInitMutation(transaction);

#define CHECK_INIT_MUTATION(field, value)                                    \
    do {                                                                      \
        transaction = BuildInitTransaction801C4260(ExactInitSource());        \
        transaction.field = value;                                            \
        ExpectRejectedInitMutation(transaction);                              \
    } while (0)

    CHECK_INIT_MUTATION(sceneEntryPointerDestination, 0x8006EDBCu);
    CHECK_INIT_MUTATION(sceneEntryPointerValue, 0x800548B8u);
    CHECK_INIT_MUTATION(tickPerMinFieldAddress, 0x800548ACu);
    CHECK_INIT_MUTATION(tickPerMinValue, 9215u);
    CHECK_INIT_MUTATION(tickPerFrameFieldAddress, 0x800548B0u);
    CHECK_INIT_MUTATION(tickPerFrameValue, 2u);
    CHECK_INIT_MUTATION(baseTickFieldAddress, 0x800548B4u);
    CHECK_INIT_MUTATION(baseTickValue, 95u);
    CHECK_INIT_MUTATION(gridColumnsFieldAddress, 0x800548B8u);
    CHECK_INIT_MUTATION(gridColumnsValue, 15u);
    CHECK_INIT_MUTATION(timingPublishedBeforeResourceIngress, false);
    CHECK_INIT_MUTATION(preopenRowCount, 6u);
    CHECK_INIT_MUTATION(compoRowIndex, 0u);
    CHECK_INIT_MUTATION(resourceIngressLoadArg, 1u);
    CHECK_INIT_MUTATION(psxMemoryBackingAuthority, true);
    CHECK_INIT_MUTATION(oldWinS0Authority, true);
    CHECK_INIT_MUTATION(stage1Authority, true);
    CHECK_INIT_MUTATION(stage2PlusAuthority, true);
    CHECK_INIT_MUTATION(replayValueAuthority, true);
    CHECK_INIT_MUTATION(hostFilesystemAuthority, true);

#undef CHECK_INIT_MUTATION
}

} // namespace

int main()
{
    TestExactState();
    TestRejectsMutations();
    TestExactInitTransaction();
    TestInitRejectsUnknownSources();
    TestInitRejectsMalformedSources();
    TestInitRejectsUnavailableResets();
    TestInitRejectsUnavailableResourceIngress();
    TestInitRejectsTransactionMutations();
    if (g_failed != 0) {
        std::printf("test_ss0_scene0_global_bindings_direct: %d failed\n",
                    g_failed);
        return 1;
    }
    std::printf("test_ss0_scene0_global_bindings_direct: ok\n");
    return 0;
}
