#include "pr_ss0_scene0_global_bindings_direct.h"

namespace PrSS0Scene0GlobalBindingsDirect {

State801C5B14 BuildState801C5B14()
{
    State801C5B14 state{};
    state.known = true;
    state.complete = true;
    state.hostProjection = true;
    state.returnValueKnown = true;
    state.returnValue = 0u;
    state.psxMemoryBackingAuthority = false;
    state.oldWinS0Authority = false;
    state.stage1Authority = false;
    state.stage2PlusAuthority = false;
    state.replayValueAuthority = false;
    state.hostFilesystemAuthority = false;

    state.slot800943C0 = 0x801C6F88u;
    state.slot800943C4 = 0x801C6F68u;
    state.slot800943C8 = 1u;
    state.slot800943CC = 0x801C6BF8u;
    state.slot800943D0 = 0x801C6DA4u;
    state.slot800943D4 = 1u;
    state.slot800943D8 = 0x801C6EF0u;
    state.slot800943DC = 1u;
    state.slot800943E0 = 0x801C6F18u;
    state.slot800943E4 = 0x801C6F84u;
    state.slot800943E8 = 0u;
    state.slot800943EC = 0x801C6E5Cu;
    state.slot800943F0 = 0x801C6E64u;
    state.slot800943F4 = 0x801C6E6Cu;
    state.slot800943F8 = 0x801C6E74u;
    state.slot800943FC = 0x801C6E88u;
    state.slot80094400 = 0x801C6E7Cu;
    state.slot80094404 = 0x801C6E94u;
    state.slot80094408 = 0x801C6E9Cu;
    state.slot8009440C = 0x801C6EA4u;
    state.slot80094410 = 0x801C6EACu;
    state.slot80094414 = 0x801C6EB4u;
    state.slot80094418 = 0x801C6EBCu;
    state.slot8009441C = 0x801C6EC4u;
    state.slot80094420 = 0x801C6ED0u;
    state.slot80094424 = 0x801C6ED8u;
    state.slot80094428 = 0x801C6EE0u;
    state.slot8009442C = 0x801C6EE8u;
    state.slot80094430 = 0x801C57C8u;
    state.slot80094434 = 0x801C57D0u;
    state.slot80094438 = 0x801C57D8u;
    state.slot8009443C = 0x801C5B0Cu;
    state.slot80094440 = 0x801C4FC8u;
    return state;
}

bool IsExactState801C5B14(const State801C5B14& state)
{
    return state.known &&
           state.complete &&
           state.hostProjection &&
           state.returnValueKnown &&
           state.returnValue == 0u &&
           !state.psxMemoryBackingAuthority &&
           !state.oldWinS0Authority &&
           !state.stage1Authority &&
           !state.stage2PlusAuthority &&
           !state.replayValueAuthority &&
           !state.hostFilesystemAuthority &&
           state.slot800943C0 == 0x801C6F88u &&
           state.slot800943C4 == 0x801C6F68u &&
           state.slot800943C8 == 1u &&
           state.slot800943CC == 0x801C6BF8u &&
           state.slot800943D0 == 0x801C6DA4u &&
           state.slot800943D4 == 1u &&
           state.slot800943D8 == 0x801C6EF0u &&
           state.slot800943DC == 1u &&
           state.slot800943E0 == 0x801C6F18u &&
           state.slot800943E4 == 0x801C6F84u &&
           state.slot800943E8 == 0u &&
           state.slot800943EC == 0x801C6E5Cu &&
           state.slot800943F0 == 0x801C6E64u &&
           state.slot800943F4 == 0x801C6E6Cu &&
           state.slot800943F8 == 0x801C6E74u &&
           state.slot800943FC == 0x801C6E88u &&
           state.slot80094400 == 0x801C6E7Cu &&
           state.slot80094404 == 0x801C6E94u &&
           state.slot80094408 == 0x801C6E9Cu &&
           state.slot8009440C == 0x801C6EA4u &&
           state.slot80094410 == 0x801C6EACu &&
           state.slot80094414 == 0x801C6EB4u &&
           state.slot80094418 == 0x801C6EBCu &&
           state.slot8009441C == 0x801C6EC4u &&
           state.slot80094420 == 0x801C6ED0u &&
           state.slot80094424 == 0x801C6ED8u &&
           state.slot80094428 == 0x801C6EE0u &&
           state.slot8009442C == 0x801C6EE8u &&
           state.slot80094430 == 0x801C57C8u &&
           state.slot80094434 == 0x801C57D0u &&
           state.slot80094438 == 0x801C57D8u &&
           state.slot8009443C == 0x801C5B0Cu &&
           state.slot80094440 == 0x801C4FC8u;
}

InitTransaction801C4260 BuildInitTransaction801C4260(
    const InitSource801C4260& source)
{
    InitTransaction801C4260 transaction{};
    if (!source.sceneIndexKnown ||
        !source.sceneEntryBaseKnown ||
        !source.sceneHeaderKnown ||
        !source.resetSourcesKnown ||
        !source.resourceIngress801C4780Known) {
        return transaction;
    }

    if (source.sceneIndex != 0u ||
        source.sceneEntryBase != 0x8005474Cu ||
        source.bpm100 != 9600u ||
        source.tickOffset != 96 ||
        source.extraTick != 0) {
        transaction.status = InitStatus801C4260::MalformedSource;
        return transaction;
    }

    if (!source.reset80025A34Ready ||
        !source.reset801C4FA0Ready ||
        !source.reset80024E98Ready ||
        !source.reset80014344Ready) {
        transaction.status = InitStatus801C4260::ResetUnavailable;
        return transaction;
    }

    if (!source.resourceIngress801C4780Ready) {
        transaction.status =
            InitStatus801C4260::ResourceIngressUnavailable;
        return transaction;
    }

    const uint32_t tickPerMin =
        (96u * static_cast<uint32_t>(source.bpm100)) / 100u;
    const uint32_t tickPerFrame = (tickPerMin + 1800u) / 3600u;
    const uint32_t baseTick = static_cast<uint32_t>(
        static_cast<int32_t>(source.tickOffset) +
        static_cast<int32_t>(source.extraTick));

    transaction.status = InitStatus801C4260::Accepted;
    transaction.accepted = true;
    transaction.complete = true;
    transaction.hostProjection = true;
    transaction.psxMemoryBackingAuthority = false;
    transaction.oldWinS0Authority = false;
    transaction.stage1Authority = false;
    transaction.stage2PlusAuthority = false;
    transaction.replayValueAuthority = false;
    transaction.hostFilesystemAuthority = false;

    transaction.callCount = kInitCallCount801C4260;
    transaction.callOrder = {
        0x80025A34u,
        0x801C4FA0u,
        0x80024E98u,
        0x80014344u,
        0x801C4780u,
    };

    transaction.sceneEntryPointerDestination = 0x8006EDB8u;
    transaction.sceneEntryPointerValue = source.sceneEntryBase;
    transaction.tickPerMinFieldAddress = source.sceneEntryBase + 0x15Cu;
    transaction.tickPerMinValue = tickPerMin;
    transaction.tickPerFrameFieldAddress = source.sceneEntryBase + 0x160u;
    transaction.tickPerFrameValue = tickPerFrame;
    transaction.baseTickFieldAddress = source.sceneEntryBase + 0x164u;
    transaction.baseTickValue = baseTick;
    transaction.gridColumnsFieldAddress = source.sceneEntryBase + 0x168u;
    transaction.gridColumnsValue = 16u;

    transaction.timingPublishedBeforeResourceIngress = true;
    transaction.preopenRowCount = 7u;
    transaction.compoRowIndex = 1u;
    transaction.resourceIngressLoadArg = 0u;
    return transaction;
}

bool IsExactInitTransaction801C4260(
    const InitTransaction801C4260& transaction)
{
    return transaction.status == InitStatus801C4260::Accepted &&
           transaction.accepted &&
           transaction.complete &&
           transaction.hostProjection &&
           !transaction.psxMemoryBackingAuthority &&
           !transaction.oldWinS0Authority &&
           !transaction.stage1Authority &&
           !transaction.stage2PlusAuthority &&
           !transaction.replayValueAuthority &&
           !transaction.hostFilesystemAuthority &&
           transaction.callCount == kInitCallCount801C4260 &&
           transaction.callOrder[0] == 0x80025A34u &&
           transaction.callOrder[1] == 0x801C4FA0u &&
           transaction.callOrder[2] == 0x80024E98u &&
           transaction.callOrder[3] == 0x80014344u &&
           transaction.callOrder[4] == 0x801C4780u &&
           transaction.sceneEntryPointerDestination == 0x8006EDB8u &&
           transaction.sceneEntryPointerValue == 0x8005474Cu &&
           transaction.tickPerMinFieldAddress == 0x800548A8u &&
           transaction.tickPerMinValue == 9216u &&
           transaction.tickPerFrameFieldAddress == 0x800548ACu &&
           transaction.tickPerFrameValue == 3u &&
           transaction.baseTickFieldAddress == 0x800548B0u &&
           transaction.baseTickValue == 96u &&
           transaction.gridColumnsFieldAddress == 0x800548B4u &&
           transaction.gridColumnsValue == 16u &&
           transaction.timingPublishedBeforeResourceIngress &&
           transaction.preopenRowCount == 7u &&
           transaction.compoRowIndex == 1u &&
           transaction.resourceIngressLoadArg == 0u;
}

} // namespace PrSS0Scene0GlobalBindingsDirect
