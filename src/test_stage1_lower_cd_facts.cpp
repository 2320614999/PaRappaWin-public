#include "pr/pr_stage1_lower_cd_producer_direct.h"
#include "pr/pr_stage1_xa_cd_direct.h"

#include <array>
#include <cstdio>

static int g_failed = 0;

#define CHECK(expr)                                                           \
    do {                                                                      \
        if (!(expr)) {                                                        \
            std::printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr);       \
            ++g_failed;                                                       \
        }                                                                     \
    } while (0)

static PrStage1LowerCdProducerDirect::RawCdRegTransactionResult80036AF8
LiveRawSyncPump80036AF8() {
    PrStage1LowerCdProducerDirect::RawCdRegTransactionResult80036AF8 out{};
    out.produced = true;
    out.psxReturnKnown = true;
    out.psxReturn = 2;
    out.cdReg3InitialInterrupt = 3;
    out.cdReg3StableInterrupt = 3;
    out.byte800573D4Known = true;
    out.byte800573D4 = 2;
    out.response882F8Known = true;
    out.response882F8[0] = 2;
    return out;
}

static void TestLiveCdSync80037070Facts() {
    PrStage1LowerCdProducerDirect::CdSyncLoopFactsInput80037070 input{};
    input.a0WaitModeKnown = true;
    input.a0WaitMode = 0;
    input.timeoutKnown = true;
    input.timedOut = false;
    input.checkCallbackKnown = true;
    input.callbackPending = true;
    input.savedCdReg0SelectorKnown = true;
    input.savedCdReg0Selector = 0;
    input.rawPumpSequenceKnown = true;
    input.rawPumpCount = 1;
    input.rawPump[0] = LiveRawSyncPump80036AF8();
    input.rawPumpPsxReturns[0] = 2;
    input.callbackPumpDrained = true;
    input.selectorRestoredKnown = true;
    input.selectorRestored = true;
    input.restoredCdReg0Selector = 0;
    input.byte800573D4Known = true;
    input.byte800573D4 = 2;
    input.response882F8Known = true;
    input.response882F8ByteCount = 8;
    input.response882F8[0] = 2;

    const auto result =
        PrStage1LowerCdProducerDirect::BuildCdSyncLoopFacts80037070(input);
    CHECK(result.produced);
    CHECK(!result.incomplete);
    CHECK(result.coreFactsKnown);
    CHECK(result.coreFacts.syncResultKnown);
    CHECK(result.coreFacts.syncResult == 2);
    CHECK(result.coreFacts.responseBytesKnown);
    CHECK(result.coreFacts.responseBytes[0] == 2);
}

static void TestLiveFinalReady800372F0Facts() {
    PrStage1LowerCdProducerDirect::ReadyWaitInput800372F0 input{};
    input.a0WaitModeKnown = true;
    input.a0WaitMode = 1;
    input.a1OutputBufferPtrKnown = true;
    input.a1OutputBufferPtrNonNull = false;
    input.timeoutCheckKnown = true;
    input.timedOut = false;
    input.callbackCheckKnown = true;
    input.callbackPending = false;
    input.byte800573D6Known = true;
    input.byte800573D6 = 0;
    input.response88308Known = true;
    input.response88308[0] = 0;
    input.byte800573D5Known = true;
    input.byte800573D5 = 1;
    input.response88300Known = true;
    input.response88300[0] = 0x22;

    const auto result =
        PrStage1LowerCdProducerDirect::BuildReadyWaitResult800372F0(input);
    CHECK(result.produced);
    CHECK(!result.incomplete);
    CHECK(result.a0WaitModeKnown);
    CHECK(result.a0WaitMode == 1);
    CHECK(result.a1OutputBufferPtrKnown);
    CHECK(!result.a1OutputBufferPtrNonNull);
    CHECK(result.callbackCheckKnown);
    CHECK(!result.callbackPending);
    CHECK(result.byte800573D5Consumed);
    CHECK(!result.outputBufferWritten);
    CHECK(result.psxReturn == 1);
}

static PrStage1LoaderCdHal::LowerActionRequestMetadata
ReadSyncRequest800390C8() {
    PrStage1LoaderCdHal::LowerActionRequestMetadata request{};
    request.known = true;
    request.actionKind = PrStage1LoaderCdHal::ActionKind::ReadSync800390C8;
    request.callerFunction = PrStage1LoaderCdHal::kFn8001A818;
    request.lowerFunction = PrStage1LoaderCdHal::kFn800390C8;
    request.finalFunction = PrStage1LoaderCdHal::kFn800364F0;
    request.readSyncRequestKnown = true;
    request.readSyncFunction = PrStage1LoaderCdHal::kFn800390C8;
    request.readSyncArg0 = PrStage1LoaderCdHal::kRead8001A818SyncArg0;
    request.readSyncArg1 = PrStage1LoaderCdHal::kRead8001A818SyncArg1;
    return request;
}

static PrStage1LoaderCdHal::LowerActionRequestMetadata
SeekSyncRequest800367A4() {
    PrStage1LoaderCdHal::LowerActionRequestMetadata request{};
    request.known = true;
    request.actionKind = PrStage1LoaderCdHal::ActionKind::SeekSync800367A4;
    request.callerFunction = PrStage1LoaderCdHal::kFn8001A89C;
    request.directHelperFunction = PrStage1LoaderCdHal::kFn80036974;
    request.lowerFunction = PrStage1LoaderCdHal::kFn800367A4;
    request.finalFunction = 0;
    request.seekRequestKnown = true;
    request.seekMsfTargetPtrKnown = true;
    request.seekMsfTargetPtr = 0x801C4780;
    request.seekLbaKnown = true;
    request.seekLba = 12345;
    request.seekMsfTargetKnown = true;
    request.seekMsfTarget =
        PrStage1LoaderCdHal::LbaToBcdMsf80036974(request.seekLba);
    request.seekArg0 = PrStage1LoaderCdHal::kSeek8001A89CSyncMode;
    request.seekArg1 = static_cast<int32_t>(request.seekMsfTargetPtr);
    request.seekArg2 = PrStage1LoaderCdHal::kSeek8001A89CSyncArg2;
    return request;
}

static PrStage1LoaderCdHal::LowerActionRequestMetadata
ReadStartRequest80038FC0() {
    PrStage1LoaderCdHal::LowerActionRequestMetadata request{};
    request.known = true;
    request.actionKind = PrStage1LoaderCdHal::ActionKind::ReadStart80038FC0;
    request.callerFunction = PrStage1LoaderCdHal::kFn8001A818;
    request.lowerFunction = PrStage1LoaderCdHal::kFn80038FC0;
    request.finalFunction = PrStage1LoaderCdHal::kFn800390C8;
    request.readStartRequestKnown = true;
    request.readStartDstPtrKnown = true;
    request.readStartDstPtr = 0x80100000;
    request.readStartSectorCountKnown = true;
    request.readStartSectorCount = 4;
    request.readStartModeFlagKnown = true;
    request.readStartModeFlag = 0;
    request.readStartArg0 = request.readStartSectorCount;
    request.readStartArg1 = static_cast<int32_t>(request.readStartDstPtr);
    request.readSyncRequestKnown = true;
    request.readSyncFunction = PrStage1LoaderCdHal::kFn800390C8;
    request.readSyncArg0 = PrStage1LoaderCdHal::kRead8001A818SyncArg0;
    request.readSyncArg1 = PrStage1LoaderCdHal::kRead8001A818SyncArg1;
    return request;
}

static PrStage1LoaderDirect::CdOverlayTransferAttemptAttribution
OverlayAttempt(uint32_t attemptIndex) {
    PrStage1LoaderDirect::CdOverlayTransferAttemptAttribution attempt{};
    attempt.known = true;
    attempt.sourceFunction =
        PrMovieSegmentDirect::kSub800154B0OverlayTransferWrapper;
    attempt.transferFunction =
        PrMovieSegmentDirect::kSub8001ACF8OverlayTransfer;
    attempt.attemptIndexKnown = true;
    attempt.attemptIndex = attemptIndex;
    return attempt;
}

static void TestSeekSyncSeamConsumesHalFacts() {
    PrStage1LowerCdProducerDirect::LowerCdProducerFacts facts{};
    facts.request = SeekSyncRequest800367A4();
    facts.overlayTransferAttempt = OverlayAttempt(0);

    PrStage1LoaderCdHal::SeekSyncHalInput800367A4 halInput{};
    halInput.requestKnown = true;
    halInput.request = facts.request;
    facts.seekSyncHalFacts800367A4Known = true;
    facts.seekSyncHalFacts800367A4 =
        PrStage1LoaderCdHal::BuildSeekSyncHalFacts800367A4(halInput);

    const auto result =
        PrStage1LowerCdProducerDirect::BuildLowerCdSeamFromFacts(facts);
    CHECK(result.produced);
    CHECK(!result.incomplete);
    CHECK(result.cd.present);
    CHECK(result.cd.feedback.handled);
    CHECK(result.cd.feedback.success);
    CHECK(result.cd.feedback.psxReturn == 1);
}

static void TestSeekSyncSeamRejectsMissingHalFacts() {
    PrStage1LowerCdProducerDirect::LowerCdProducerFacts facts{};
    facts.request = SeekSyncRequest800367A4();
    facts.overlayTransferAttempt = OverlayAttempt(0);

    const auto result =
        PrStage1LowerCdProducerDirect::BuildLowerCdSeamFromFacts(facts);
    CHECK(!result.produced);
    CHECK(result.incomplete);
}

static void TestReadStartSeamConsumesHalFacts() {
    PrStage1LowerCdProducerDirect::LowerCdProducerFacts facts{};
    facts.request = ReadStartRequest80038FC0();
    facts.overlayTransferAttempt = OverlayAttempt(0);
    facts.overlayTransferAttempt.dstKnown = true;
    facts.overlayTransferAttempt.dst = facts.request.readStartDstPtr;
    facts.overlayTransferAttempt.sectorCountKnown = true;
    facts.overlayTransferAttempt.sectorCount =
        static_cast<uint32_t>(facts.request.readStartSectorCount);

    PrStage1LoaderCdHal::ReadStartHalInput80038FC0 halInput{};
    halInput.requestKnown = true;
    halInput.request = facts.request;
    halInput.commandAcceptedKnown = true;
    halInput.commandAccepted = true;
    halInput.readStartedKnown = true;
    halInput.readStarted = true;
    halInput.readFailedKnown = true;
    halInput.readFailed = false;
    halInput.timeoutKnown = true;
    halInput.timeout = false;
    halInput.dstPtrKnown = true;
    halInput.dstPtr = facts.request.readStartDstPtr;
    halInput.sectorCountKnown = true;
    halInput.sectorCount = facts.request.readStartSectorCount;
    facts.readStartHalFacts80038FC0Known = true;
    facts.readStartHalFacts80038FC0 =
        PrStage1LoaderCdHal::BuildReadStartHalFacts80038FC0(halInput);

    const auto result =
        PrStage1LowerCdProducerDirect::BuildLowerCdSeamFromFacts(facts);
    CHECK(result.produced);
    CHECK(!result.incomplete);
    CHECK(result.cd.present);
    CHECK(result.cd.feedback.handled);
    CHECK(result.cd.feedback.success);
    CHECK(result.cd.feedback.psxReturn == 1);
    CHECK(result.cd.dstPtrKnown);
    CHECK(result.cd.dstPtr == facts.request.readStartDstPtr);
    CHECK(result.cd.sectorCountKnown);
    CHECK(result.cd.sectorCount == facts.request.readStartSectorCount);
}

static void TestReadStartSeamRejectsMissingHalFacts() {
    PrStage1LowerCdProducerDirect::LowerCdProducerFacts facts{};
    facts.request = ReadStartRequest80038FC0();
    facts.overlayTransferAttempt = OverlayAttempt(0);

    const auto result =
        PrStage1LowerCdProducerDirect::BuildLowerCdSeamFromFacts(facts);
    CHECK(!result.produced);
    CHECK(result.incomplete);
}

static void TestReadStartSeamAllowsZeroDstHalFacts() {
    PrStage1LowerCdProducerDirect::LowerCdProducerFacts facts{};
    facts.request = ReadStartRequest80038FC0();
    facts.request.readStartDstPtrKnown = false;
    facts.request.readStartDstPtr = 0;
    facts.request.readStartArg1 = 0;
    facts.overlayTransferAttempt = OverlayAttempt(0);
    facts.overlayTransferAttempt.sectorCountKnown = true;
    facts.overlayTransferAttempt.sectorCount =
        static_cast<uint32_t>(facts.request.readStartSectorCount);

    PrStage1LoaderCdHal::ReadStartHalInput80038FC0 halInput{};
    halInput.requestKnown = true;
    halInput.request = facts.request;
    halInput.commandAcceptedKnown = true;
    halInput.commandAccepted = true;
    halInput.readStartedKnown = true;
    halInput.readStarted = true;
    halInput.readFailedKnown = true;
    halInput.readFailed = false;
    halInput.timeoutKnown = true;
    halInput.timeout = false;
    halInput.sectorCountKnown = true;
    halInput.sectorCount = facts.request.readStartSectorCount;
    facts.readStartHalFacts80038FC0Known = true;
    facts.readStartHalFacts80038FC0 =
        PrStage1LoaderCdHal::BuildReadStartHalFacts80038FC0(halInput);

    const auto result =
        PrStage1LowerCdProducerDirect::BuildLowerCdSeamFromFacts(facts);
    CHECK(result.produced);
    CHECK(!result.incomplete);
    CHECK(result.cd.feedback.success);
    CHECK(!result.cd.dstPtrKnown);
    CHECK(result.cd.sectorCountKnown);
    CHECK(result.cd.sectorCount == facts.request.readStartSectorCount);
}

static PrStage1LowerCdProducerDirect::ReadyWaitInput800372F0
LiveFinalReadyInput800372F0() {
    PrStage1LowerCdProducerDirect::ReadyWaitInput800372F0 input{};
    input.a0WaitModeKnown = true;
    input.a0WaitMode = 1;
    input.a1OutputBufferPtrKnown = true;
    input.a1OutputBufferPtrNonNull = false;
    input.timeoutCheckKnown = true;
    input.timedOut = false;
    input.callbackCheckKnown = true;
    input.callbackPending = false;
    input.byte800573D6Known = true;
    input.byte800573D6 = 0;
    input.response88308Known = true;
    input.response88308[0] = 0;
    input.byte800573D5Known = true;
    input.byte800573D5 = 1;
    input.response88300Known = true;
    input.response88300[0] = 0x22;
    return input;
}

static void TestReadSyncSeamRejectsMissingFinalReadyFacts() {
    PrStage1LowerCdProducerDirect::LowerCdProducerFacts facts{};
    facts.request = ReadSyncRequest800390C8();
    facts.overlayTransferAttempt.known = true;
    facts.overlayTransferAttempt.sourceFunction =
        PrMovieSegmentDirect::kSub800154B0OverlayTransferWrapper;
    facts.overlayTransferAttempt.transferFunction =
        PrMovieSegmentDirect::kSub8001ACF8OverlayTransfer;
    facts.overlayTransferAttempt.attemptIndexKnown = true;
    facts.overlayTransferAttempt.attemptIndex = 0;
    facts.readSyncWait = true;
    facts.clockKnown = true;
    facts.clockNow = 0;
    facts.startClockKnown = true;
    facts.startClock8005742C = 0;
    facts.lastPumpClockKnown = true;
    facts.lastPumpClock80057428 = 0;
    facts.remainingKnown = true;
    facts.remaining80057424 = 0;
    facts.finalReadyInput800372F0Known = false;

    const auto result =
        PrStage1LowerCdProducerDirect::BuildLowerCdSeamFromFacts(facts);
    CHECK(!result.produced);
    CHECK(result.incomplete);
    CHECK(!result.cd.present);
    CHECK(!result.cd.feedback.success);
}

static void TestReadSyncSeamConsumesLiveFinalReadyFacts() {
    PrStage1LowerCdProducerDirect::LowerCdProducerFacts facts{};
    facts.request = ReadSyncRequest800390C8();
    facts.overlayTransferAttempt.known = true;
    facts.overlayTransferAttempt.sourceFunction =
        PrMovieSegmentDirect::kSub800154B0OverlayTransferWrapper;
    facts.overlayTransferAttempt.transferFunction =
        PrMovieSegmentDirect::kSub8001ACF8OverlayTransfer;
    facts.overlayTransferAttempt.attemptIndexKnown = true;
    facts.overlayTransferAttempt.attemptIndex = 0;
    facts.readSyncWait = true;
    facts.clockKnown = true;
    facts.clockNow = 0;
    facts.startClockKnown = true;
    facts.startClock8005742C = 0;
    facts.lastPumpClockKnown = true;
    facts.lastPumpClock80057428 = 0;
    facts.remainingKnown = true;
    facts.remaining80057424 = 0;
    facts.finalReadyInput800372F0Known = true;
    facts.finalReadyInput800372F0 = LiveFinalReadyInput800372F0();

    const auto result =
        PrStage1LowerCdProducerDirect::BuildLowerCdSeamFromFacts(facts);
    CHECK(result.produced);
    CHECK(!result.incomplete);
    CHECK(result.cd.present);
    CHECK(result.cd.feedback.handled);
    CHECK(result.cd.feedback.success);
    CHECK(result.cd.feedback.psxReturn == 0);
}

static void TestReadyWaitRawFactsOverrideExplicitReadyBankFacts() {
    PrStage1LowerCdProducerDirect::ReadyWaitInput800372F0 input =
        LiveFinalReadyInput800372F0();
    input.callbackPending = true;
    input.callbackPumpDrainedKnown = true;
    input.callbackPumpDrained = true;
    input.callbackMaskKnown = true;
    input.callbackMask = 4;
    input.readyCallbackRegistered = true;
    input.rawCallbackTransactionSequenceKnown = true;
    input.rawCallbackTransactionCount = 1;
    input.rawCallbackPsxReturns[0] = 0;
    input.rawCallbackTransactions[0].produced = true;
    input.rawCallbackTransactions[0].psxReturnKnown = true;
    input.rawCallbackTransactions[0].psxReturn = 0;
    input.rawCallbackTransactions[0].byte800573D6Known = true;
    input.rawCallbackTransactions[0].byte800573D6 = 0;
    input.rawCallbackTransactions[0].byte800573D5Known = true;
    input.rawCallbackTransactions[0].byte800573D5 = 2;
    input.rawCallbackTransactions[0].response88300Known = true;
    input.rawCallbackTransactions[0].response88300[0] = 0x11;

    const auto result =
        PrStage1LowerCdProducerDirect::BuildReadyWaitResult800372F0(input);
    CHECK(!result.produced);
    CHECK(result.incomplete);
}

static void TestReadyWaitRawFactsProduceReadyBankFacts() {
    PrStage1LowerCdProducerDirect::ReadyWaitInput800372F0 input{};
    input.a0WaitModeKnown = true;
    input.a0WaitMode = 1;
    input.a1OutputBufferPtrKnown = true;
    input.a1OutputBufferPtrNonNull = false;
    input.timeoutCheckKnown = true;
    input.timedOut = false;
    input.callbackCheckKnown = true;
    input.callbackPending = true;
    input.callbackPumpDrainedKnown = true;
    input.callbackPumpDrained = true;
    input.callbackMaskKnown = true;
    input.callbackMask = 4;
    input.readyCallbackRegistered = true;
    input.rawCallbackTransactionSequenceKnown = true;
    input.rawCallbackTransactionCount = 1;
    input.rawCallbackPsxReturns[0] = 0;
    input.rawCallbackTransactions[0].produced = true;
    input.rawCallbackTransactions[0].psxReturnKnown = true;
    input.rawCallbackTransactions[0].psxReturn = 0;
    input.rawCallbackTransactions[0].byte800573D6Known = true;
    input.rawCallbackTransactions[0].byte800573D6 = 0;
    input.rawCallbackTransactions[0].byte800573D5Known = true;
    input.rawCallbackTransactions[0].byte800573D5 = 1;
    input.rawCallbackTransactions[0].response88300Known = true;
    input.rawCallbackTransactions[0].response88300[0] = 0x22;

    const auto readyBank =
        PrStage1LowerCdProducerDirect::ResolveReadyWaitReadyBankFacts800372F0(
            input);
    CHECK(!readyBank.incomplete);
    CHECK(readyBank.byte800573D6Known);
    CHECK(readyBank.byte800573D6 == 0);
    CHECK(readyBank.byte800573D5Known);
    CHECK(readyBank.byte800573D5 == 1);
    CHECK(readyBank.response88300Known);
    CHECK(readyBank.response88300[0] == 0x22);

    const auto result =
        PrStage1LowerCdProducerDirect::BuildReadyWaitResult800372F0(input);
    CHECK(result.produced);
    CHECK(!result.incomplete);
    CHECK(result.callbackPumpRequired);
    CHECK(result.callbackPumpDrained);
    CHECK(result.rawCallbackTransactionSequenceKnown);
    CHECK(result.rawCallbackTransactionCount == 1);
    CHECK(result.readyCallbackDispatch);
    CHECK(!result.syncCallbackDispatch);
    CHECK(result.byte800573D5Consumed);
    CHECK(result.psxReturn == 1);
}

static void TestReadyWaitRawFactsRejectUnauthorizedSequence() {
    PrStage1LowerCdProducerDirect::ReadyWaitInput800372F0 input =
        LiveFinalReadyInput800372F0();
    input.rawCallbackTransactionSequenceKnown = true;
    input.rawCallbackTransactionCount = 1;
    input.rawCallbackPsxReturns[0] = 0;
    input.rawCallbackTransactions[0].produced = true;
    input.rawCallbackTransactions[0].psxReturnKnown = true;
    input.rawCallbackTransactions[0].psxReturn = 0;
    input.rawCallbackTransactions[0].byte800573D5Known = true;
    input.rawCallbackTransactions[0].byte800573D5 = 1;
    input.rawCallbackTransactions[0].response88300Known = true;
    input.rawCallbackTransactions[0].response88300[0] = 0x22;

    const auto readyBank =
        PrStage1LowerCdProducerDirect::ResolveReadyWaitReadyBankFacts800372F0(
            input);
    CHECK(readyBank.incomplete);

    const auto result =
        PrStage1LowerCdProducerDirect::BuildReadyWaitResult800372F0(input);
    CHECK(!result.produced);
    CHECK(result.incomplete);
}

static void TestStage1XaCdClockPollConsumesV3ExplicitStatusFacts() {
    PrStage1XaCdDirectState state{};
    state.byte_80057119Known = true;
    state.byte_80057119 = 0x10;
    state.cdSyncExplicitStatusKnown = true;
    state.cdSyncExplicitStatus = 2;
    state.cdSyncExplicitResponseBytesKnown = true;
    state.cdSyncExplicitResponseBytes = {0x28, 0x12, 0x41, 0x02,
                                         0x01, 0x05, 0x64, 0x00};

    const auto result =
        PrStage1XaCdDirectApplySub8001A3C8ClockPollFromLowerState(state);

    CHECK(result.called);
    CHECK(result.psxReturn == 1);
    CHECK(!result.gapMissingSub800364D0Feedback);
    CHECK(!result.gapMissingSub800363A4Feedback);
    CHECK(result.acceptedByte800493F4);
    CHECK(result.dword80049428Known);
    CHECK(result.dword80049428 == 0);
    CHECK(state.cdLowerFeedback80036AF8Known);
    CHECK(state.cdSync80037070Known);
    CHECK(state.cdSync80037070.syncResultKnown);
    CHECK(state.cdSync80037070.psxReturn == 2);
    CHECK(state.cdSync80037070.outputBytesKnown);
    CHECK(state.cdSync80037070.outputByteCount == 8);
    CHECK(state.cdSync80037070.outputBytes[0] == 0x28);
    CHECK(state.cdSync80037070.outputBytes[1] == 0x12);
    CHECK(state.cdSync80037070.outputBytes[2] == 0x41);
    CHECK(state.byte_800493F4Known);
    CHECK(state.byte_800493F4.minute == 0x28);
    CHECK(state.byte_800493F4.second == 0x12);
    CHECK(state.byte_800493F4.frame == 0x41);
    CHECK(state.dword_80049428Known);
    CHECK(state.dword_80049428 == 0);
}

static void TestStage1XaCdWorkBaseCommandConsumesBssZero49428() {
    PrStage1XaCdDirectState state{};

    const auto result =
        PrStage1XaCdDirectApplySub8001A280WorkBaseCommand(state);

    CHECK(result.called);
    CHECK(result.dword80049428Known);
    CHECK(result.dword80049428 == 0);
    CHECK(!result.gapMissingDword80049428);
    CHECK(result.commandIssued);
    CHECK(result.commandResult.called);
    CHECK(result.commandResult.byte80057119Known);
    CHECK(result.commandResult.byte80057119 == 0x10);
    CHECK(state.byte_80057119Known);
    CHECK(state.byte_80057119 == 0x10);
    CHECK(state.byte80057119ProducerFunction ==
          PrMovieSegmentDirect::kSub800375BCCdCommand);
}

static void TestStage1XaCdReadyStatusInvalidatesExplicitGetlocPBytes() {
    PrStage1XaCdDirectState state{};
    state.cdSyncExplicitStatusKnown = true;
    state.cdSyncExplicitStatus = 2;
    state.cdSyncExplicitResponseBytesKnown = true;
    state.cdSyncExplicitResponseBytes =
        {0x28, 0x12, 0x41, 0x02, 0x01, 0x05, 0x64, 0x00};

    PrStage1XaCdDirectCdReadyStatusInput800363A4 input{};
    input.byte80057119Known = true;
    input.byte80057119 = 13;

    const auto result =
        PrStage1XaCdDirectApplySub800363A4CdReadyStatus(state, input);

    CHECK(result.resultKnown);
    CHECK(result.psxReturn == 13);
    CHECK(state.byte_80057119Known);
    CHECK(state.byte_80057119 == 13);
    CHECK(!state.cdSyncExplicitStatusKnown);
    CHECK(!state.cdSyncExplicitResponseBytesKnown);
}

static void WriteU16Le(std::array<uint8_t, 32>& bytes,
                       size_t offset,
                       uint16_t value) {
    bytes[offset + 0] = static_cast<uint8_t>(value & 0x00FFu);
    bytes[offset + 1] = static_cast<uint8_t>((value >> 8) & 0x00FFu);
}

static void TestStage1XaCdAcceptedSectorCarriesHalGetlocPFacts() {
    PrStage1XaCdDirectState state{};
    PrStage1XaCdDirectClearRing(state, 32u);
    (void)PrStage1XaCdDirectApplySub8001A280WorkBaseCommand(state);

    PrStage1XaCdDirectHalGetlocPFactsInput getloc{};
    getloc.cdGetlocPResponseKnown = true;
    getloc.cdGetlocPResponse =
        {0x30, 0x09, 0x08, 0x02, 0x01, 0x05, 0x64, 0x00};
    getloc.cdDataReadyInterruptKnown = true;
    getloc.cdDataReadyInterrupt = 2;
    const auto facts = PrStage1XaCdDirectApplyHalGetlocPFacts(state, getloc);
    CHECK(facts.called);
    CHECK(facts.applied);
    CHECK(!facts.incomplete);
    CHECK(state.cdSyncExplicitStatusKnown);
    CHECK(state.cdSyncExplicitStatus == 2);
    CHECK(state.cdSyncExplicitResponseBytesKnown);
    CHECK(state.cdSyncExplicitResponseBytes[0] == 0x30);
    CHECK(state.cdLowerFeedback80036AF8Known);
    CHECK(state.cdLowerFeedback80036AF8FromGetlocP);
    CHECK(state.cdLowerFeedback80036AF8.responseBytes[0] == 0x30);
    CHECK(state.cdLowerFeedback80036AF8.responseBytes[1] == 0x09);
    CHECK(state.cdLowerFeedback80036AF8.responseBytes[2] == 0x08);

    std::array<uint8_t, 32> header{};
    WriteU16Le(header, 0, 0x0160u);
    WriteU16Le(header, 2, 0x0000u);
    WriteU16Le(header, 4, 0u);
    WriteU16Le(header, 6, 1u);
    WriteU16Le(header, 8, 1u);
    std::array<uint8_t, 2016> payload{};

    PrStage1XaCdDirectRingPacketInput packet{};
    packet.header32 = header.data();
    packet.headerSize = header.size();
    packet.payload2016 = payload.data();
    packet.payloadSize = payload.size();

    const auto ring = PrStage1XaCdDirectApplySub80039670Packet(state, packet);
    CHECK(ring.accepted);
    CHECK(state.cdLowerFeedback80036AF8Known);
    CHECK(state.cdLowerFeedback80036AF8FromGetlocP);

    const auto poll =
        PrStage1XaCdDirectApplySub8001A3C8ClockPollFromLowerState(state);
    CHECK(poll.called);
    CHECK(poll.acceptedByte800493F4);
    CHECK(!poll.gapMissingSub800364D0Feedback);
    CHECK(!poll.gapMissingSub800363A4Feedback);
    CHECK(state.byte_800493F4Known);
    CHECK(state.byte_800493F4.minute == 0x30);
    CHECK(state.byte_800493F4.second == 0x09);
    CHECK(state.byte_800493F4.frame == 0x08);
    CHECK(state.cdLowerFeedback80036AF8Known);
    CHECK(state.cdSyncExplicitStatusKnown);
    CHECK(state.cdSyncExplicitStatus == 2);
    CHECK(state.cdSyncExplicitResponseBytesKnown);
    CHECK(state.cdSyncExplicitResponseBytes[0] == 0x30);
    CHECK(state.cdSyncExplicitResponseBytes[1] == 0x09);
    CHECK(state.cdSyncExplicitResponseBytes[2] == 0x08);
    CHECK(state.cdLowerFeedback80036AF8.responseBytes[0] == 0x30);
    CHECK(state.cdLowerFeedback80036AF8.responseBytes[1] == 0x09);
    CHECK(state.cdLowerFeedback80036AF8.responseBytes[2] == 0x08);
}

static void TestStage1XaCdCurrentPhysicalGetlocPOverridesAcceptedPacketFacts() {
    PrStage1XaCdDirectState state{};
    (void)PrStage1XaCdDirectApplySub8001A280WorkBaseCommand(state);

    PrStage1XaCdDirectHalGetlocPFactsInput acceptedPacketGetloc{};
    acceptedPacketGetloc.sectorIndexKnown = true;
    acceptedPacketGetloc.sectorIndex = 3000u;
    acceptedPacketGetloc.cdGetlocPResponseKnown = true;
    acceptedPacketGetloc.cdGetlocPResponse =
        {0x28, 0x03, 0x01, 0x02, 0x01, 0x05, 0x64, 0x00};
    acceptedPacketGetloc.cdDataReadyInterruptKnown = true;
    acceptedPacketGetloc.cdDataReadyInterrupt = 2;
    const auto acceptedFacts =
        PrStage1XaCdDirectApplyHalGetlocPFacts(state, acceptedPacketGetloc);
    CHECK(acceptedFacts.called);
    CHECK(acceptedFacts.applied);

    PrStage1XaCdDirectHalGetlocPFactsInput currentPhysicalGetloc{};
    currentPhysicalGetloc.sectorIndexKnown = true;
    currentPhysicalGetloc.sectorIndex = 3005u;
    currentPhysicalGetloc.cdGetlocPResponseKnown = true;
    currentPhysicalGetloc.cdGetlocPResponse =
        {0x28, 0x03, 0x06, 0x02, 0x02, 0x07, 0x64, 0x00};
    currentPhysicalGetloc.cdDataReadyInterruptKnown = true;
    currentPhysicalGetloc.cdDataReadyInterrupt = 2;
    const auto currentFacts =
        PrStage1XaCdDirectApplyHalGetlocPFacts(state, currentPhysicalGetloc);
    CHECK(currentFacts.called);
    CHECK(currentFacts.applied);
    CHECK(state.halGetlocPFactsApplyCount == 2u);
    CHECK(state.lastHalGetlocPSectorIndexKnown);
    CHECK(state.lastHalGetlocPSectorIndex == 3005u);

    const auto poll =
        PrStage1XaCdDirectApplySub8001A3C8ClockPollFromLowerState(state);
    CHECK(poll.called);
    CHECK(poll.acceptedByte800493F4);
    CHECK(!poll.gapMissingSub800364D0Feedback);
    CHECK(state.byte_800493F4Known);
    CHECK(state.byte_800493F4.minute == 0x28);
    CHECK(state.byte_800493F4.second == 0x03);
    CHECK(state.byte_800493F4.frame == 0x06);
    CHECK(state.cdLowerFeedback80036AF8.responseBytes[0] == 0x28);
    CHECK(state.cdLowerFeedback80036AF8.responseBytes[1] == 0x03);
    CHECK(state.cdLowerFeedback80036AF8.responseBytes[2] == 0x06);
}

static void TestStage1XaCdClockPollExplicitStatusOverridesStaleGetlocPFeedback() {
    PrStage1XaCdDirectState state{};
    state.cdLowerFeedback80036AF8Known = true;
    state.cdLowerFeedback80036AF8.known = true;
    state.cdLowerFeedback80036AF8.timedOut = false;
    state.cdLowerFeedback80036AF8.syncResultKnown = true;
    state.cdLowerFeedback80036AF8.syncResult = 2;
    state.cdLowerFeedback80036AF8.responseBytesKnown = true;
    state.cdLowerFeedback80036AF8.responseByteCount = 8u;
    state.cdLowerFeedback80036AF8.responseBytes[0] = 0x30;
    state.cdLowerFeedback80036AF8.responseBytes[1] = 0x09;
    state.cdLowerFeedback80036AF8.responseBytes[2] = 0x08;
    state.byte_80057119Known = true;
    state.byte_80057119 = 0x10;
    PrStage1XaCdDirectHalGetlocPFactsInput getloc{};
    getloc.cdGetlocPResponseKnown = true;
    getloc.cdGetlocPResponse =
        {0x28, 0x18, 0x07, 0x02, 0x01, 0x05, 0x64, 0x00};
    getloc.cdDataReadyInterruptKnown = true;
    getloc.cdDataReadyInterrupt = 2;
    const auto getlocFacts = PrStage1XaCdDirectApplyHalGetlocPFacts(state, getloc);
    CHECK(getlocFacts.called);
    CHECK(getlocFacts.applied);
    CHECK(!getlocFacts.incomplete);
    CHECK(state.cdSyncExplicitStatusKnown);
    CHECK(state.cdSyncExplicitStatus == 2);
    CHECK(state.cdSyncExplicitResponseBytesKnown);

    const auto result =
        PrStage1XaCdDirectApplySub8001A3C8ClockPollFromLowerState(state);

    CHECK(result.called);
    CHECK(result.psxReturn == 1);
    CHECK(result.acceptedByte800493F4);
    CHECK(state.cdSync80037070Known);
    CHECK(state.cdSync80037070.outputBytesKnown);
    CHECK(state.cdSync80037070.outputBytes[0] == 0x28);
    CHECK(state.cdSync80037070.outputBytes[1] == 0x18);
    CHECK(state.cdSync80037070.outputBytes[2] == 0x07);
    CHECK(state.cdLowerFeedback80036AF8.responseBytes[0] == 0x28);
    CHECK(state.cdLowerFeedback80036AF8.responseBytes[1] == 0x18);
    CHECK(state.cdLowerFeedback80036AF8.responseBytes[2] == 0x07);
    CHECK(state.byte_800493F4Known);
    CHECK(state.byte_800493F4.minute == 0x28);
    CHECK(state.byte_800493F4.second == 0x18);
    CHECK(state.byte_800493F4.frame == 0x07);
}

static void TestStage1XaCdStatusPollRejectsGetlocPFeedback() {
    PrStage1XaCdDirectState state{};
    (void)PrStage1XaCdDirectApplySub8001A280WorkBaseCommand(state);

    PrStage1XaCdDirectHalGetlocPFactsInput getloc{};
    getloc.cdGetlocPResponseKnown = true;
    getloc.cdGetlocPResponse =
        {0x30, 0x09, 0x08, 0x02, 0x01, 0x05, 0x64, 0x00};
    getloc.cdDataReadyInterruptKnown = true;
    getloc.cdDataReadyInterrupt = 2;
    const auto getlocFacts =
        PrStage1XaCdDirectApplyHalGetlocPFacts(state, getloc);
    CHECK(getlocFacts.called);
    CHECK(getlocFacts.applied);
    CHECK(!getlocFacts.incomplete);

    const auto clock =
        PrStage1XaCdDirectApplySub8001A3C8ClockPollFromLowerState(state);
    CHECK(clock.called);
    CHECK(clock.acceptedByte800493F4);
    CHECK(state.cdLowerFeedback80036AF8Known);
    CHECK(state.cdLowerFeedback80036AF8FromGetlocP);
    CHECK(state.cdSyncExplicitStatusKnown);
    CHECK(state.cdSyncExplicitStatus == 2);
    CHECK(state.cdSyncExplicitResponseBytesKnown);
    CHECK(state.cdSyncExplicitResponseBytes[0] == 0x30);
    CHECK(state.cdSyncExplicitResponseBytes[1] == 0x09);
    CHECK(state.cdSyncExplicitResponseBytes[2] == 0x08);

    const auto status =
        PrStage1XaCdDirectApplySub8001A750StatusPollFromLowerState(state);
    CHECK(status.called);
    CHECK(!status.resultKnown);
    CHECK(status.gapMissingSub800364D0Feedback);
}

int main() {
    TestLiveCdSync80037070Facts();
    TestLiveFinalReady800372F0Facts();
    TestSeekSyncSeamConsumesHalFacts();
    TestSeekSyncSeamRejectsMissingHalFacts();
    TestReadStartSeamConsumesHalFacts();
    TestReadStartSeamRejectsMissingHalFacts();
    TestReadStartSeamAllowsZeroDstHalFacts();
    TestReadSyncSeamRejectsMissingFinalReadyFacts();
    TestReadSyncSeamConsumesLiveFinalReadyFacts();
    TestReadyWaitRawFactsOverrideExplicitReadyBankFacts();
    TestReadyWaitRawFactsProduceReadyBankFacts();
    TestReadyWaitRawFactsRejectUnauthorizedSequence();
    TestStage1XaCdClockPollConsumesV3ExplicitStatusFacts();
    TestStage1XaCdWorkBaseCommandConsumesBssZero49428();
    TestStage1XaCdReadyStatusInvalidatesExplicitGetlocPBytes();
    TestStage1XaCdAcceptedSectorCarriesHalGetlocPFacts();
    TestStage1XaCdCurrentPhysicalGetlocPOverridesAcceptedPacketFacts();
    TestStage1XaCdClockPollExplicitStatusOverridesStaleGetlocPFeedback();
    TestStage1XaCdStatusPollRejectsGetlocPFeedback();
    if (g_failed != 0) {
        std::printf("%d lower-CD facts test(s) failed\n", g_failed);
        return 1;
    }
    std::puts("lower-CD live facts tests passed");
    return 0;
}
