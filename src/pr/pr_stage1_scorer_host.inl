// Temporary same-TU extraction of the Stage1 scorer host orchestration.
// Keep the exact current behavior while shrinking `pr_scenes.cpp`. The next
// phase can lift private types/helpers into dedicated headers and turn this
// into a separate translation unit without changing the host call order again.

static bool ResolveStage1AcceptedInputFlagBit07A60(
    const Stage1NumericRuntimeState::AcceptedProducerCarrierRuntime& carrier) {
    switch (static_cast<PrStage1ScorerHostDirectAcceptedControlWriterSourceKind>(
        carrier.controlWriterSourceKind)) {
    case PrStage1ScorerHostDirectAcceptedControlWriterSourceKind::
        LocalLiveNormalized:
    case PrStage1ScorerHostDirectAcceptedControlWriterSourceKind::
        LocalPassthroughControl:
    case PrStage1ScorerHostDirectAcceptedControlWriterSourceKind::
        LocalControlSample800:
    case PrStage1ScorerHostDirectAcceptedControlWriterSourceKind::
        ScriptedSharedBufferSample:
        return true;
    case PrStage1ScorerHostDirectAcceptedControlWriterSourceKind::None:
    case PrStage1ScorerHostDirectAcceptedControlWriterSourceKind::LocalZeroClear:
    case PrStage1ScorerHostDirectAcceptedControlWriterSourceKind::
        LocalReplayPreserveClear:
    default:
        return false;
    }
}

static uint16_t ResolveStage1SteadyWriterControlSample18_7A60(
    const PrStage1ScorerHostDirectSteadyInput7A60Result& steadyInput,
    uint16_t fallbackHeldMask) {
    if (steadyInput.writeCtx18 && steadyInput.ctx18Known) {
        return static_cast<uint16_t>(steadyInput.ctx18Value & 0xFFFFu);
    }
    return fallbackHeldMask;
}

static void RunStage1FrameUpdate9094(PrGameContext& ctx,
                                     const PrStageRunner& runner,
                                     Stage1NumericRuntimeState& state,
                                     const Stage1RunnerTimingRoots30& timing) {
    if (ctx.stage1OverlayData && ctx.stage1OverlayData->valid) {
        const bool clearTerminalTailPulse =
            ConsumeStage1DirectFrameClearTerminalTailPulse(
                *ctx.stage1OverlayData,
                timing.tick96,
                timing.queryFrame);
        const bool ctxAcceptedInputFlagSet =
            ResolveStage1AcceptedInputFlagBit07A60(
                state.acceptedProducerCarrier);
        const PrStageRunnerDirectContext801C9094 sameFrameCtx =
            BuildStage1RunnerSameFrameCtx801C9094(
                state,
                timing,
                ctxAcceptedInputFlagSet);
        const PrStageRunnerDirectGlobals801C9094 sameFrameGlobals =
            BuildStage1RunnerSameFrameGlobals801C9094(state);
        CaptureStage1RunnerSameFrame801C9094InputSnapshot(
            state,
            timing,
            true,
            sameFrameCtx,
            true,
            sameFrameGlobals);
        const Stage1EventStreamFrameUpdate9094Result frameUpdate9094 =
            RunStage1EventStreamFrameUpdate9094(
                *ctx.stage1OverlayData,
                timing.tick96,
                timing.queryFrame,
                true,
                sameFrameCtx,
                true,
                sameFrameGlobals,
                true,
                clearTerminalTailPulse);
        StoreStage1RunnerFrameUpdate9094Snapshot(state, frameUpdate9094);
        ApplyStage1RunnerFrameUpdate9094ActionTrace(
            ctx,
            state,
            frameUpdate9094.frameUpdate.trace);
        UpdateStage1AcceptedProducerGateCarrierRuntime(
            ctx,
            state,
            timing.tick96);
    }
    ApplyStage1FrameUpdateFlag0200Branch9094(state);
    UpdateStage1RunnerTailGateRuntime(
        ResolveStage1RunnerTailGateFrameInputs(state));
}

static PrStageRunnerDirectPostFrame7A60Result RunStage1AcceptedProducer14614(
    PrGameContext& ctx,
    const PrStageRunner& runner,
    Stage1NumericRuntimeState& state,
    const Stage1RunnerTimingRoots30& timing,
    uint16_t writerControlSample18,
    uint16_t heldMask,
    int runnerFrame,
    const PrStage1ScorerDirectPageClear14BDCResult* sameTickPageClear14BDC) {
    const bool allowDynamicCarryProbe =
        ShouldAllowStage1LateBranchDynamicCarryAcceptedMask(state);
    const bool busyGate24BF4Active =
        ResolveStage1DirectBusyGate24BF4(state, timing.tick96);
    uint16_t rawPressedMask =
        busyGate24BF4Active
            ? 0u
            : ResolveStage1AcceptedProducerObservedRawSample18(
                  ctx,
                  state.acceptedProducerCarrier,
                  heldMask);
    uint16_t rawAcceptedMask =
        static_cast<uint16_t>(rawPressedMask & 0x09FFu);
    const bool ctxAcceptedInputFlagSet =
        ResolveStage1AcceptedInputFlagBit07A60(state.acceptedProducerCarrier);
    const uint32_t ctxFlagsAfter9094 =
        ResolveStage1RunnerCtxFlagsForPostFrame7A60(
            state,
            ctxAcceptedInputFlagSet);
    const PrStageRunnerDirectPostFrame7A60Result initialPostFrame =
        PrStageRunnerDirectResolvePostFrame7A60(
            PrStageRunnerDirectPostFrame7A60Input{
                ctxFlagsAfter9094,
                writerControlSample18,
                state.runnerPostFramePreviousInputMask801CCBB8,
                ResolveStage1Ctx52ReplayMode7A60(state),
                false,
                false,
                IsStage1SpecialFixedBonusAdditiveGateActive(runner),
            });
    state.acceptedProducerBoundaryProbe =
        Stage1NumericRuntimeState::SharedAcceptedProducerBoundaryProbeRuntime{};
    state.acceptedProducerBoundaryProbe.rawPressedMask = rawPressedMask;
    state.acceptedProducerBoundaryProbe.rawAcceptedMask = rawAcceptedMask;
    state.acceptedProducerBoundaryProbe.initialPostKnown = true;
    state.acceptedProducerBoundaryProbe.initialPostCtxInput18 =
        writerControlSample18;
    state.acceptedProducerBoundaryProbe.initialPostPreviousInputMask801CCBB8 =
        state.runnerPostFramePreviousInputMask801CCBB8;
    state.acceptedProducerBoundaryProbe.initialPostAcceptedMask9FF =
        initialPostFrame.acceptedMask9FF;
    state.acceptedProducerBoundaryProbe.initialPostAcceptedGateOpen =
        initialPostFrame.acceptedGateOpen;
    state.acceptedProducerBoundaryProbe.initialPostAcceptedMaskChanged =
        initialPostFrame.acceptedMaskChanged;
    state.acceptedProducerBoundaryProbe.initialPostBackupCtx10FromCtx0C =
        initialPostFrame.backupCtx10FromCtx0C;
    state.acceptedProducerBoundaryProbe.initialPostCallAcceptedProducer14614 =
        initialPostFrame.callAcceptedProducer14614;
    PrStage1ScorerDirectAcceptedProducerRunResult directAcceptedRun{};
    bool directAcceptedRunCaptured = false;
    if (initialPostFrame.callAcceptedProducer14614) {
        directAcceptedRunCaptured = MirrorStage1SharedAcceptedProducerFromRawInput(
            ctx,
            state,
            timing.tick96,
            initialPostFrame.backupCtx10FromCtx0C,
            writerControlSample18,
            rawPressedMask,
            rawAcceptedMask,
            &directAcceptedRun,
            sameTickPageClear14BDC);
        MarkStage1LocalAcceptedProducerHoldMaskConsumed(
            state,
            static_cast<uint16_t>(initialPostFrame.acceptedMask9FF));
    }
    if (directAcceptedRunCaptured) {
        state.acceptedProducerBoundaryProbe.directAcceptedRunCaptured = true;
        state.acceptedProducerBoundaryProbe.directAcceptedRunResultCode =
            directAcceptedRun.resultCode;
        state.acceptedProducerBoundaryProbe.directAcceptedRunReplayAppendRan =
            directAcceptedRun.replayAppendRan;
        state.acceptedProducerBoundaryProbe.directAcceptedRunWriteRan =
            directAcceptedRun.writeRan;
        state.acceptedProducerBoundaryProbe.directAcceptedRunWriteResultCode =
            directAcceptedRun.writeResult.resultCode;
        state.acceptedProducerBoundaryProbe.directAcceptedRunSelectorResolved =
            directAcceptedRun.resolved.selectorResolved;
        state.acceptedProducerBoundaryProbe.directAcceptedRunTimingTemplateSlot48 =
            directAcceptedRun.resolved.timingTemplateSlot48;
        state.acceptedProducerBoundaryProbe.directAcceptedRunTimingTemplateAddress =
            kStage1TimingTemplateTableAddr +
            static_cast<uint32_t>(directAcceptedRun.resolved.selectorByte1) *
                static_cast<uint32_t>(kStage1TimingTemplateSelectorStride) +
            static_cast<uint32_t>(
                (std::min<uint8_t>)(
                    directAcceptedRun.resolved.timingTemplateSlot48,
                    static_cast<uint8_t>(
                        kStage1TimingTemplateSlotCount - 1u)));
        state.acceptedProducerBoundaryProbe.directAcceptedRunTimingTemplateState =
            directAcceptedRun.resolved.timingTemplateState;
        if (directAcceptedRun.resolved.selectorResolved &&
            directAcceptedRun.resolved.selectorByte1 != 0u) {
            uint8_t timingTemplateState = 0u;
            state.acceptedProducerBoundaryProbe.directAcceptedRunTimingTemplateKnown =
                TryResolveStage1TimingTemplateStateAtSlotIndexHost(
                    ctx,
                    directAcceptedRun.resolved.selectorByte1,
                    directAcceptedRun.resolved.timingTemplateSlot48,
                    timingTemplateState);
            if (state.acceptedProducerBoundaryProbe
                    .directAcceptedRunTimingTemplateKnown) {
                state.acceptedProducerBoundaryProbe
                    .directAcceptedRunTimingTemplateState = timingTemplateState;
            }
        }
        state.acceptedProducerBoundaryProbe.directAcceptedRunSourceCellValid =
            directAcceptedRun.resolved.sourceCell.valid;
    }
    (void)ApplyStage1PageMaintenanceProjectionFromLiteralPageClear14BDC_14614(
        state,
        sameTickPageClear14BDC,
        directAcceptedRunCaptured ? &directAcceptedRun : nullptr);
    if (directAcceptedRunCaptured &&
        directAcceptedRun.sideEffect.callbackArgPresent &&
        directAcceptedRun.sideEffect.callbackArgOpaque != 0u) {
        state.rightRankDirectFollowUp.hudOverlayConfigure801CA0E0Pulse = true;
        state.rightRankDirectFollowUp.hudOverlayConfigure801CA0E0Arg =
            static_cast<uint8_t>(
                directAcceptedRun.sideEffect.callbackArgOpaque & 0xFFu);
        state.rightRankDirectFollowUp.hudOverlayConfigure801CA0E0QueryFrame =
            timing.queryFrame;
        state.rightRankDirectFollowUp.hudOverlayConfigure801CA0E0ScriptFrame =
            timing.tick96 >= 0 ? static_cast<uint32_t>(timing.tick96) : 0u;
    }
    const PrStageRunnerDirectPostFrame7A60Result postFrame =
        ResolveStage1RunnerPostFrame7A60(
            state,
            runner,
            ctxAcceptedInputFlagSet,
            timing.tick96,
            writerControlSample18,
            directAcceptedRunCaptured,
            directAcceptedRunCaptured && directAcceptedRun.resultCode != 0);
    ApplyStage1AcceptedProducerCurrentTickLatchFromPostFrame(
        state.acceptedProducerCarrier,
        postFrame,
        timing.tick96,
        timing.queryFrame,
        state.runnerPostFrame7A60.ctxInput18,
        state.runnerPostFrame7A60.previousInputMask801CCBB8);
    CaptureStage1AcceptedInputSourceEventRuntime(
        state,
        timing.queryFrame,
        timing.tick96,
        postFrame);
    CaptureStage1RunnerTailHostPostFrame7A60(
        state,
        postFrame,
        timing.queryFrame,
        timing.tick96);
    CaptureStage1DescBankSelector7A60(
        state,
        timing.queryFrame,
        timing.tick96,
        writerControlSample18,
        postFrame.callAcceptedProducer94434,
        postFrame.callAcceptedProducer9443C);
    ApplyStage1PostAcceptedLoop7A60(
        state,
        postFrame);
    RefreshStage1LateBranchBridgeProbe(
        state,
        rawPressedMask,
        rawAcceptedMask,
        runnerFrame,
        allowDynamicCarryProbe);
    return postFrame;
}

static bool DidStage1BucketStepCrossTickAdvance801C9094(
    uint8_t previousBucket,
    uint8_t currentBucket) {
    if (currentBucket == 0xFFu) {
        return false;
    }
    if (previousBucket == 0xFFu) {
        return (currentBucket & 1u) == 0u;
    }

    uint8_t bucket = previousBucket;
    for (uint8_t guard = 0u; guard < 32u; ++guard) {
        bucket = static_cast<uint8_t>((bucket + 1u) & 31u);
        if ((bucket & 1u) == 0u) {
            return true;
        }
        if (bucket == currentBucket) {
            return false;
        }
    }
    return (currentBucket & 1u) == 0u;
}

static const PrStage1ScorerDirectPageClear14BDCResult*
RunStage1ScorerDispatcher24FD0(
    PrGameContext& ctx,
    Stage1NumericRuntimeState& state,
    const Stage1RunnerTimingRoots30& timing,
    uint8_t currentBucket,
    PrStage1ScorerDirectPageClear14BDCResult& sameTickPageClear14BDC) {
    if (currentBucket == state.bucketCadence.currentBucket) {
        return nullptr;
    }

    state.bucketCadence.previousBucket = state.bucketCadence.currentBucket;
    state.bucketCadence.currentBucket = currentBucket;
    state.bucketCadence.bucketChanged = true;
    state.bucketCadence.bucketAdvanceCount++;
    state.bucketCadence.ctxFlagTickAdvanceKnown801C9094 =
        currentBucket != 0xFFu;
    state.bucketCadence.ctxFlagTickAdvance801C9094 =
        DidStage1BucketStepCrossTickAdvance801C9094(
            state.bucketCadence.previousBucket,
            currentBucket);
    state.bucketCadence.bucket0Advanced =
        (currentBucket == kStage1GameplayCadenceBuckets[0]);
    state.bucketCadence.bucket30Advanced =
        (currentBucket == kStage1GameplayCadenceBuckets[1]);
    state.bucketCadence.bucket31Advanced =
        (currentBucket == kStage1HistoricalBucket31ConsumerCadenceBucket);

    if (state.bucketCadence.bucket30Advanced) {
        ProcessStage1Bucket30OwnerWindow(state, timing.tick96);
    }
    if (state.bucketCadence.bucket0Advanced) {
        ProcessStage1Bucket0OwnerWindow(ctx, state, timing.tick96);
    }

    const bool literalPageClear14BDCAdvanced =
        currentBucket == kStage1LiteralPageClear14BDCBucket;

    if (state.bucketCadence.bucket31Advanced) {
        PrStage1ScorerDirectBucket31DispatcherResult directBucket31Result{};
        ProcessStage1Bucket31OwnerWindow(
            state,
            &directBucket31Result,
            timing.tick96,
            literalPageClear14BDCAdvanced);
        sameTickPageClear14BDC = directBucket31Result.pageClear;
        return &sameTickPageClear14BDC;
    }
    if (literalPageClear14BDCAdvanced) {
        sameTickPageClear14BDC =
            ProcessStage1LiteralPageClear14BDCWindow(state, timing.tick96);
        return &sameTickPageClear14BDC;
    }
    return nullptr;
}

static void RunStage1DirectFrame7A60(PrGameContext& ctx,
                                     const PrStageRunner& runner,
                                     Stage1NumericRuntimeState& state,
                                     const Stage1RunnerTimingRoots30& timing,
                                     uint16_t heldMask,
                                     const Stage1NumericRuntimeTimecodeInput801C7560&
                                         timecodeInput) {
    const int runnerFrame = timing.runnerFrame;

    state.queryFrame = timing.queryFrame;
    Stage1NumericRuntimeState::RunnerTimecode801C7560Runtime&
        timecodeRuntime = state.runnerTimecode801C7560;
    const PrMovieSegmentDirect::StreamClockPollResult8001A3C8
        streamClockPoll8001A3C8 =
            PrStage1XaCdDirectApplySub8001A3C8ClockPollFromLowerState(
                ctx.stage1XaCdDirect);
    const PrStage1XaCdDirectInitResult8001A280 command8001A280 =
        PrStage1XaCdDirectApplySub8001A280WorkBaseCommand(
            ctx.stage1XaCdDirect);
    timecodeRuntime.clockPoll8001A3C8Called =
        streamClockPoll8001A3C8.called;
    timecodeRuntime.clockPoll8001A3C8Return =
        streamClockPoll8001A3C8.psxReturn;
    timecodeRuntime.clockPoll8001A3C8AcceptedByte =
        streamClockPoll8001A3C8.acceptedByte800493F4;
    timecodeRuntime.clockPoll8001A3C8Gap364D0 =
        streamClockPoll8001A3C8.gapMissingSub800364D0Feedback;
    timecodeRuntime.clockPoll8001A3C8Gap363A4 =
        streamClockPoll8001A3C8.gapMissingSub800363A4Feedback;
    timecodeRuntime.command8001A280Called =
        command8001A280.called;
    timecodeRuntime.command8001A280Issued =
        command8001A280.commandIssued;
    timecodeRuntime.command8001A280SkippedNonZeroWorkBase =
        command8001A280.skippedNonZeroWorkBase;
    timecodeRuntime.command8001A280Gap49428 =
        command8001A280.gapMissingDword80049428;
    timecodeRuntime.command8001A280WorkBase =
        command8001A280.dword80049428;

    int32_t xaReadValueA7A4 = 0;
    const bool xaSectorKnown =
        ResolveStage1TimecodeXaSector801C7560(
            ctx,
            timecodeInput,
            xaReadValueA7A4);
    AdvanceStage1RunnerTimecode801C7560(
        state,
        timecodeInput,
        xaSectorKnown,
        xaReadValueA7A4);
    Stage1RunnerTimingRoots30 directTiming = timing;
    if (state.runnerTimecode801C7560.known) {
        // 801C7A60 runs 801C7560 before 80024FD0/801C9094/80014614.
        // Those callees read ctx+0x0C, not the host runner's presentation tick.
        directTiming.tick96 =
            state.runnerTimecode801C7560.state.tick801C364C;
    }
    const uint8_t directCurrentBucket =
        GetStage1GameplayBucketForTick96(directTiming.tick96);

    PrepareStage1PageRecordMirrorForTick(state.pageRecordMirror, directTiming.tick96);
    RefreshStage1DescriptorDrivenRuntimeStateForTick(ctx, state, directTiming.tick96);
    const PrStage1ScorerHostDirectSteadyInput7A60Result steadyInput7A60 =
        CaptureStage1SteadyInput7A60(state, heldMask, directTiming.tick96);
    UpdateStage1AcceptedProducerCarrierRuntime(
        ctx,
        state,
        steadyInput7A60,
        heldMask,
        directTiming.tick96,
        directTiming.queryFrame);

    state.bucketCadence.bucketChanged = false;
    state.bucketCadence.bucket0Advanced = false;
    state.bucketCadence.bucket30Advanced = false;
    state.bucketCadence.bucket31Advanced = false;
    state.bucketCadence.ctxFlagTickAdvanceKnown801C9094 = false;
    state.bucketCadence.ctxFlagTickAdvance801C9094 = false;
    state.acceptedProducerNarrowClearFired = false;
    state.sourceCellVoice.replaceRestartPulse = false;
    state.steadySfx.firedBucket30 = false;
    state.steadySfx.firedBucket31Completion = false;
    state.pageOwner.literalBucket31PageClear14BDCWindowAdvanced = false;
    state.pageOwner.literalBucket31PageClear14BDCRequested = false;
    state.pageOwner.literalBucket31PageClear14BDCTargetKnown = false;
    state.pageOwner.literalBucket31PageClear14BDCTargetOrdinal1Based = 0u;
    state.pageOwner.literalBucket31PageClear14BDCClearApplied = false;
    state.rightRankHelperShadow.bucket0Ctx118WritePulse = false;
    state.rightRankDirectFollowUp.flag0200Pulse = false;
    state.rightRankDirectFollowUp.flag0200FrameUpdateBranchTaken = false;
    state.rightRankDirectFollowUp.flag0200FrameUpdateResourcePairKnown = false;
    state.rightRankDirectFollowUp.flag0200FrameUpdateCtxE0ResourceIndex = 0u;
    state.rightRankDirectFollowUp.flag0200FrameUpdateCtxECResourceIndex = 0u;
    state.rightRankDirectFollowUp.flag0200FrameUpdateCtxFlag20000Set = false;
    state.rightRankDirectFollowUp.flag0200PostTail1A654Requested = false;
    state.rightRankDirectFollowUp.flag0200PostTail1A654Arg = 0u;
    state.rightRankDirectFollowUp.flag2000Pulse = false;
    state.rightRankDirectFollowUp.flag4000Pulse = false;
    state.rightRankDirectFollowUp.hudOverlayConfigure801CA0E0Pulse = false;
    state.rightRankDirectFollowUp.hudOverlayConfigure801CA0E0Arg = 0u;
    state.rightRankDirectFollowUp.hudOverlayConfigure801CA0E0QueryFrame = 0u;
    state.rightRankDirectFollowUp.hudOverlayConfigure801CA0E0ScriptFrame = 0u;
    state.rightRankDirectFollowUp.descBankSelectorPulse = false;
    state.rightRankDirectFollowUp.descBankSelector = 0u;
    state.rightRankDirectFollowUp.descBankSelectorQueryFrame = 0u;
    state.rightRankDirectFollowUp.descBankCtxFlagsPulse801CBFDC = 0u;
    state.rightRankDirectFollowUp.cameraPulse9443C = false;
    state.rightRankDirectFollowUp.cameraBezHandle9443C = 0u;
    state.rightRankDirectFollowUp.cameraQueryFrame9443C = 0u;
    state.rightRankDirectFollowUp.acceptedTailTimer9443CPulse = false;
    state.rightRankDirectFollowUp.acceptedTailTimer9443CFrames = 0;
    state.rightRankDirectFollowUp.acceptedTailTimer9443CQueryFrame = 0u;
    state.runnerPostFrame7A60.ctx0Owner801C3640 =
        Stage1NumericRuntimeState::RunnerCtx0Owner801C3640Runtime{};
    state.runnerPostFrame7A60.sameFrameCtx801C9094 =
        Stage1NumericRuntimeState::RunnerSameFrameCtxOwner801C9094Runtime{};
    state.runnerPostFrame7A60.sameFrameGlobals801C9094 =
        Stage1NumericRuntimeState::
            RunnerSameFrameGlobalsOwner801C9094Runtime{};
    state.runnerPostFrame7A60.compactLane801C9094 =
        Stage1NumericRuntimeState::RunnerCompactLaneOwner801C9094Runtime{};
    state.runnerPostFrame7A60.frameUpdate9094ReturnEarlyAfterFlag40Reset =
        false;
    {
        const uint32_t priorXaSetFilter13RequestCount =
            state.runnerTailHost7A60.xaSetFilter13RequestCount;
        const bool priorXaSetFilter13LastRequestKnown =
            state.runnerTailHost7A60.xaSetFilter13LastRequestKnown;
        const uint32_t priorXaSetFilter13LastRequestQueryFrame =
            state.runnerTailHost7A60.xaSetFilter13LastRequestQueryFrame;
        const int32_t priorXaSetFilter13LastRequestTick96 =
            state.runnerTailHost7A60.xaSetFilter13LastRequestTick96;
        const uint8_t priorXaSetFilter13LastRequestRow =
            state.runnerTailHost7A60.xaSetFilter13LastRequestRow;
        const uint8_t priorXaSetFilter13LastRequestArg =
            state.runnerTailHost7A60.xaSetFilter13LastRequestArg;
        state.runnerTailHost7A60 =
            Stage1NumericRuntimeState::RunnerTailHost7A60Runtime{};
        state.runnerTailHost7A60.xaSetFilter13RequestCount =
            priorXaSetFilter13RequestCount;
        state.runnerTailHost7A60.xaSetFilter13LastRequestKnown =
            priorXaSetFilter13LastRequestKnown;
        state.runnerTailHost7A60.xaSetFilter13LastRequestQueryFrame =
            priorXaSetFilter13LastRequestQueryFrame;
        state.runnerTailHost7A60.xaSetFilter13LastRequestTick96 =
            priorXaSetFilter13LastRequestTick96;
        state.runnerTailHost7A60.xaSetFilter13LastRequestRow =
            priorXaSetFilter13LastRequestRow;
        state.runnerTailHost7A60.xaSetFilter13LastRequestArg =
            priorXaSetFilter13LastRequestArg;
    }
    if (!state.acceptedProducerNarrowClearPending) {
        state.acceptedProducerNarrowClearPhase =
            Stage1NumericRuntimeState::AcceptedProducerNarrowClearPhase::None;
    }
    state.additiveLane.bucket30CommitVisible = false;
    PrStage1ScorerDirectPageClear14BDCResult sameTickPageClear14BDC{};
    const PrStage1ScorerDirectPageClear14BDCResult* sameTickPageClear14BDCForAccepted =
        nullptr;

    // Direct authority block for the PSX steady frame:
    // 80024FD0 bucket dispatcher, then 801C9094 event-stream frame update,
    // then the gated 80014614 accepted producer tail.
    sameTickPageClear14BDCForAccepted =
        RunStage1ScorerDispatcher24FD0(ctx,
                                       state,
                                       directTiming,
                                       directCurrentBucket,
                                       sameTickPageClear14BDC);

    UpdateStage1NextDescriptorConsumerMirror(
        ctx,
        state,
        directTiming.tick96,
        heldMask);
    UpdateStage1GameplayRailCadenceProducerMirror(ctx, state, directTiming.tick96);
    const uint16_t writerControlSample18 =
        ResolveStage1SteadyWriterControlSample18_7A60(
            steadyInput7A60,
            heldMask);
    RunStage1FrameUpdate9094(ctx, runner, state, directTiming);
    const PrStageRunnerDirectPostFrame7A60Result postFrame =
        RunStage1AcceptedProducer14614(ctx,
                                       runner,
                                       state,
                                       directTiming,
                                       writerControlSample18,
                                       heldMask,
                                       runnerFrame,
                                       sameTickPageClear14BDCForAccepted);
    ResolveStage1RunnerMainLoopTail7A60(
        state,
        timecodeInput.sceneEntryField352FallbackTickAdvance,
        ctx.stage1XaCdDirect.sub8001A7F8Known,
        ctx.stage1XaCdDirect.sub8001A7F8Result == 1u,
        ctx.stage1XaCdDirect.dword_80049428Known,
        ctx.stage1XaCdDirect.dword_80049428 == 1,
        postFrame);

    state.lastRunnerFrame = runnerFrame;
}

void UpdateStage1NumericRuntimeState(
    PrGameContext& ctx,
    const PrStageRunner& runner,
    const Stage1NumericRuntimeTimecodeInput801C7560& timecodeInput,
    bool allowTransitionFreeze) {
    const bool gateActive = IsStage1NumericRuntimeGateActive(ctx, runner) ||
        (allowTransitionFreeze &&
         runner.GetSceneId() == 1 &&
         runner.GetState() == StageRunnerState::Playing);
    if (!gateActive) {
        ResetStage1ScorerHostNumericRuntimeState();
        return;
    }

    const Stage1RunnerTimingRoots30 timing = ResolveStage1RunnerTimingRoots30(runner);
    Stage1NumericRuntimeState& state = s_stage1NumericRuntime;
    const int runnerFrame = timing.runnerFrame;
    if (runnerFrame < 0) {
        ResetStage1ScorerHostNumericRuntimeState();
        return;
    }
    if (state.active && runnerFrame < state.lastRunnerFrame) {
        ResetStage1ScorerHostNumericRuntimeState();
    }

    if (!state.active) {
        state.active = true;
        state.lastRunnerFrame = runnerFrame;
        state.queryFrame = timing.queryFrame;
        // `sub_80024E98` initializes `ctx+0x6A` to 1; no COMOD1 writer is
        // confirmed in the scorer host hot set, so carry it as bucket context.
        state.rightRankBucketContext.ctx6AConsumerGate = true;
        // The same init writes `ctx+0x7A = 1`; later `sub_80024FD0`
        // transitions toggle it for the compact painter.
        state.rightRankBucketContext.ctx7A = 1u;
        state.rightRankState.rightRankActiveRow = kStage1InitialRightRankActiveRow;
        state.rightRankState.rightRankBlinkTargetRow =
            kStage1InitialRightRankActiveRow;
        state.rightRankState.rightRankBlinkEnabled = false;
        InitializeStage1FormulaAccumulatorBaseline(state);
        CaptureStage1StartupSetup7A60(ctx, state, runner, timecodeInput);
        // COMOD1 setup always runs `sub_80024E54(0)` before it writes
        // `ctx+0x52` and enters the steady loop. Only the `transitionState==2`
        // special setup path follows with `sub_8001681C()` restore.
        ApplyStage1AcceptedSpecialSetupCore24E54_1681C(ctx, state);
        InitializeStage1DescriptorCadenceRuntime(ctx, state, timing.tick96);
        SeedStage1AuthoritativeAdditiveLaneRuntime(state);
        const uint16_t heldMask = GetStage1NumericRuntimeHeldMask(state, ctx);
        RunStage1DirectFrame7A60(
            ctx,
            runner,
            state,
            timing,
            heldMask,
            timecodeInput);
        return;
    }

    if (runnerFrame == state.lastRunnerFrame) {
        return;
    }

    const uint16_t heldMask = GetStage1NumericRuntimeHeldMask(state, ctx);
    RunStage1DirectFrame7A60(
        ctx,
        runner,
        state,
        timing,
        heldMask,
        timecodeInput);
}
