#include "pr_psx_pad_direct.h"

#include "pr_pad.h"

namespace PrPsxPadDirect {

PadInitState800354C0 PsxCall800354C0_InitPadRuntime(int32_t inputArg) {
    PadInitState800354C0 out{};
    out.sourceKnown = true;
    out.inputArg = inputArg;
    // 800354C0 is only called with zero in the 8001C470 startup seam.  Keep
    // other arguments fail-closed instead of inventing a second PAD mode.
    if (inputArg != 0) {
        return out;
    }
    out.accepted = true;
    out.modeGlobalValue = inputArg;
    out.statusGlobalValue = -1;
    out.resetCallbackCalled = true;
    out.padInit2Called = true;
    out.padInit2Protocol = kPadInit2Protocol800354C0;
    out.padInit2StatusAddress = kPadStatusGlobal800882F0;
    out.changeClearPadCalled = true;
    out.changeClearPadArg = 0;
    out.softwareStateCommitted = true;
    return out;
}

uint16_t NormalizeDebugServerPsxPadMaskToReturnedMask80035510(
    uint16_t psxPadMask) {
    // Debug commands can still supply the PSX high directional quartet; the
    // Stage1 gameplay seam needs the post-PAD_dr returned mask seen by 80035510.
    uint16_t returnedMask = static_cast<uint16_t>(psxPadMask & 0x0FFFu);
    if ((psxPadMask & 0x1000u) != 0u) {
        returnedMask = static_cast<uint16_t>(returnedMask | 0x0010u);
    }
    if ((psxPadMask & 0x2000u) != 0u) {
        returnedMask = static_cast<uint16_t>(returnedMask | 0x0020u);
    }
    if ((psxPadMask & 0x4000u) != 0u) {
        returnedMask = static_cast<uint16_t>(returnedMask | 0x0040u);
    }
    if ((psxPadMask & 0x8000u) != 0u) {
        returnedMask = static_cast<uint16_t>(returnedMask | 0x0080u);
    }
    return returnedMask;
}

uint16_t NormalizeLocalPrPadMaskToReturnedMask80035510(
    uint16_t localPrPadMask) {
    uint16_t returnedMask =
        static_cast<uint16_t>(localPrPadMask &
            ((uint16_t)PrPadButton::Triangle |
             (uint16_t)PrPadButton::Circle |
             (uint16_t)PrPadButton::Cross |
             (uint16_t)PrPadButton::Square));
    if ((localPrPadMask & (uint16_t)PrPadButton::Up) != 0u) {
        returnedMask = static_cast<uint16_t>(returnedMask | 0x0010u);
    }
    if ((localPrPadMask & (uint16_t)PrPadButton::Right) != 0u) {
        returnedMask = static_cast<uint16_t>(returnedMask | 0x0020u);
    }
    if ((localPrPadMask & (uint16_t)PrPadButton::Down) != 0u) {
        returnedMask = static_cast<uint16_t>(returnedMask | 0x0040u);
    }
    if ((localPrPadMask & (uint16_t)PrPadButton::Left) != 0u) {
        returnedMask = static_cast<uint16_t>(returnedMask | 0x0080u);
    }
    if ((localPrPadMask & (uint16_t)PrPadButton::L1) != 0u) {
        returnedMask = static_cast<uint16_t>(returnedMask | 0x0004u);
    }
    if ((localPrPadMask & (uint16_t)PrPadButton::L2) != 0u) {
        returnedMask = static_cast<uint16_t>(returnedMask | 0x0001u);
    }
    if ((localPrPadMask & (uint16_t)PrPadButton::R1) != 0u) {
        returnedMask = static_cast<uint16_t>(returnedMask | 0x0008u);
    }
    if ((localPrPadMask & (uint16_t)PrPadButton::R2) != 0u) {
        returnedMask = static_cast<uint16_t>(returnedMask | 0x0002u);
    }
    if ((localPrPadMask & (uint16_t)PrPadButton::Select) != 0u) {
        returnedMask = static_cast<uint16_t>(returnedMask | 0x0100u);
    }
    if ((localPrPadMask & (uint16_t)PrPadButton::Start) != 0u) {
        returnedMask = static_cast<uint16_t>(returnedMask | 0x0800u);
    }
    return returnedMask;
}

uint16_t BuildReturnedMask80035510FromLocalAndDebugPad(
    uint16_t localPrPadMask,
    uint16_t debugServerPsxPadMask) {
    const uint16_t localReturnedMask =
        NormalizeLocalPrPadMaskToReturnedMask80035510(localPrPadMask);
    const uint16_t debugReturnedMask =
        NormalizeDebugServerPsxPadMaskToReturnedMask80035510(
            debugServerPsxPadMask);
    return static_cast<uint16_t>(localReturnedMask | debugReturnedMask);
}

PadReadResult80035510 PsxReadPadMask80035510(uint16_t padReturnedMask) {
    PadReadResult80035510 out{};
    out.called = true;
    out.padDrCalled = true;
    out.dword800882F0 = static_cast<uint16_t>(~padReturnedMask);
    out.psxReturnMask = static_cast<uint16_t>(~out.dword800882F0);
    return out;
}

StartupPadWaitResult80016AB4 ExecuteStartupPadWait80016AB4(
    uint16_t padReturnedMask,
    PrPsxVSyncDirect::PsxVSyncState80035560& sharedVSync) {
    StartupPadWaitResult80016AB4 out{};
    out.known = true;

    uint32_t result = 0u;
    for (uint32_t poll = 0u;
         poll < kStartupPadWaitPollLimit80016AB4;
         ++poll) {
        const PadReadResult80035510 read =
            PsxReadPadMask80035510(padReturnedMask);
        ++out.pollCount;
        out.returnedPadMask = read.psxReturnMask;
        result = static_cast<uint32_t>(read.psxReturnMask);
        if (result != 0u) {
            break;
        }

        const auto begin = PrPsxVSyncDirect::BeginVSync80035560(
            sharedVSync, kStartupPadWaitArgument80016AB4);
        auto wait = begin;
        if (!wait.softwareWaitComplete) {
            wait = PrPsxVSyncDirect::ConsumeHostVblanks80035560(
                sharedVSync, 1);
        }
        if (!wait.softwareWaitComplete ||
            wait.mode != PrPsxVSyncDirect::PsxVSyncMode80035560::
                WaitForVblanks) {
            return out;
        }
        ++out.waitCallCount;
        ++out.consumedVblankCount;
    }

    out.pollLimitReached =
        out.pollCount == kStartupPadWaitPollLimit80016AB4 && result == 0u;
    out.resultBeforeSpecialMap = result;
    if (result == kStartupPadWaitSpecialMask80016AB4) {
        out.specialMaskMatched = true;
        out.word800916FAWritten = true;
        out.word800916FAValue = 1u;
        result = 1u;
    }
    out.returnValue = result;
    out.softwareStateCommitted = true;
    out.committed = true;
    return out;
}

}  // namespace PrPsxPadDirect
