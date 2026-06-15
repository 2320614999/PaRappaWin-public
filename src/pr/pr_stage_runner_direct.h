#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

constexpr size_t kPrStageRunnerDirectMaxActions = 32u;

enum class PrStageRunnerDirectActionKind : uint8_t {
    None = 0u,
    StoreDword801D303CZero,
    Call80026FA4,
    Call801C7520InitUI,
    Call8001A478,
    Call8001A654,
    Call80014344,
    Call80024E98,
    Call80024FC0,
    Call801C7338ResetState,
    Call80024E54,
    Call80094440,
    Call800143F0,
    Call8001681C,
    Call80016758,
    Call8001670C,
    Call800259C0,
    Call801C7560UpdateTimecode,
    Call800357D4,
    Call801C75D0RenderFrame,
    Call801C76CCPresentFrame,
    Call8001A750,
    Call8001F518,
    Call80027194,
    Call80035560,
    Call8001A280,
    Call80035510,
    Call80024B54,
    Call80024FD0,
    Call801C9094FrameUpdate,
    Call80014614,
    Call80094434,
    Call8009443C,
    Call80014C5C,
    Call80026EF8,
    Call8001A4A4,
    Call80015350,
    Call8001A7F8,
    Call8001A3B8,
    Call8001A694,
    Call80035838,
    Call8001B120,
    Call800166AC,
    Call80024308,
    Call80024390,
    Call801C8D78EventStreamReset,
    Call801C89B0HudTimedListTickSlot,
    Call801C88C8HudSetRes10000,
    Call801C8828HudSetRes20000,
    Call801C895CApplyScriptEvent,
    Call801C8604ApplyScriptEventSetText,
    Call800169E0,
};

struct PrStageRunnerDirectAction {
    PrStageRunnerDirectActionKind kind = PrStageRunnerDirectActionKind::None;
    uint32_t psxAddress = 0u;
    bool arg0Known = false;
    int32_t arg0 = 0;
    bool arg1Known = false;
    int32_t arg1 = 0;
};

struct PrStageRunnerDirectActionTrace {
    std::array<PrStageRunnerDirectAction, kPrStageRunnerDirectMaxActions>
        actions{};
    size_t count = 0u;
    bool overflow = false;
};

struct PrStageRunnerDirectInitInput801C79E4 {
    int16_t stageXaChannelSeg04 = 1;
    int16_t stageBgmArgSeg06 = 0;
    int32_t stageRunnerResetStateReturn7338 = 0;
};

struct PrStageRunnerDirectInitResult801C79E4 {
    PrStageRunnerDirectActionTrace trace{};
    bool writeDword801D303CZero = false;
    bool audioResetBarrier26FA4 = false;
    bool initUi801C7520 = false;
    bool setBgm8001A478 = false;
    int16_t bgmArg = 0;
    bool setFilter8001A654 = false;
    uint8_t filterChannel = 0u;
    bool resetScorer80014344 = false;
    bool resetEventRuntime80024E98 = false;
    bool bindRunnerContext80024FC0 = false;
    bool callsResetState801C7338 = false;
    int32_t returnValue = 0;
};

struct PrStageRunnerDirectStartupSetup7A60Input {
    uint16_t transitionState916D0 = 0u;
    uint16_t recordsMode916DA = 0u;
    bool defaultSetupReturn1670CKnown = false;
    int32_t defaultSetupReturn1670C = 0;
    bool specialSetupReturn16758Known = false;
    int32_t specialSetupReturn16758 = 0;
};

struct PrStageRunnerDirectStartupSetup7A60Result {
    PrStageRunnerDirectActionTrace trace{};
    bool copyRecordsMode916DATo801D3040 = false;
    uint16_t dword801D3040Value = 0u;
    bool clearRecordsMode916DA = false;
    bool call24E54Zero = false;
    bool writeWord41 = false;
    uint16_t word41Value = 0u;
    bool call94440 = false;
    bool call1681C = false;
    bool call16758 = false;
    bool call1670C = false;
    bool firstCall143F0 = false;
    bool firstCall143F0ArgKnown = false;
    int32_t firstCall143F0Arg = 0;
    bool call259C0 = false;
    bool call259C0ArgKnown = false;
    int32_t call259C0Arg = 0;
    bool tailCall143F0Zero = false;
    bool tailCall259C0Zero = false;
};

struct PrStageRunnerDirectPreamble7A60Input {
    uint16_t recordsMode916DA = 0u;
    int32_t sceneEntryField360HalfSource = 0;
    uint16_t word916F6 = 0u;
};

struct PrStageRunnerDirectPreamble7A60Result {
    PrStageRunnerDirectActionTrace trace{};
    int32_t writeDword34 = 0;
    bool writeWord55 = false;
    uint16_t word55Value = 0u;
    bool writeWord54 = false;
    uint16_t word54Value = 0u;
    bool tailReset143F0_259C0 = false;
    bool writeDword801D303CZero = false;
};

struct PrStageRunnerDirectReplaySlot7A60 {
    bool available = false;
    int32_t tick96 = 0;
    uint32_t rawMask = 0u;
};

struct PrStageRunnerDirectInputDispatch7A60Input {
    bool replayModeWord41 = false;
    uint32_t replayReadIndex901C0 = 0u;
    uint32_t replayPublishedCount901BC = 0u;
    int32_t ctxTick0C = 0;
    PrStageRunnerDirectReplaySlot7A60 replaySlot{};
    bool livePoll35510Known = false;
    uint32_t livePoll35510 = 0u;
};

struct PrStageRunnerDirectInputDispatch7A60Result {
    PrStageRunnerDirectActionTrace trace{};
    bool incrementReplayReadIndex901C0 = false;
    uint32_t replayReadIndex901C0After = 0u;
    bool writeCtx18 = false;
    uint32_t ctx18Value = 0u;
    bool writeCtx20 = false;
    bool ctx20Known = false;
    int32_t ctx20Value = 0;
    bool setCtxFlagAcceptedInput = false;
    bool writeCtx10CurrentTick = false;
    bool writeCtx10ReplayTick = false;
    int32_t ctx10ReplayTick = 0;
    uint16_t ctxWord04Bar = 0u;
    uint8_t ctxByte0ABeat = 0u;
    uint8_t ctxByte0BTick = 0u;
    bool waitingForReplaySlot = false;
    bool liveNoInput = false;
    bool usedReplayFallbackInput2048 = false;
};

struct PrStageRunnerDirectCtx0FlagsSnapshot801C3640 {
    bool baseKnown = false;
    uint32_t baseFlags = 0u;
    bool finalKnown = false;
    uint32_t finalFlags = 0u;
};

struct PrStageRunnerDirectCtx0FinalFlagsSnapshot801C9094 {
    bool finalKnown = false;
    uint32_t finalFlags = 0u;
    bool sourceFrameUpdate801C9094 = false;
    bool returnEarlyAfterFlag40Reset = false;
    bool consumedFlagStreamEvent = false;
    bool consumedIdStreamEvent = false;
};

struct PrStageRunnerDirectPostFrame7A60Input {
    uint32_t ctxFlags00 = 0u;
    uint32_t ctxInput18 = 0u;
    uint32_t previousInputMask801CCBB8 = 0u;
    bool replayModeWord41 = false;
    bool scorerAccept14614Known = false;
    bool scorerAccept14614ReturnsNonZero = false;
    bool fixedBonusGate916FA = false;
};

struct PrStageRunnerDirectPostFrame7A60Result {
    PrStageRunnerDirectActionTrace trace{};
    uint32_t acceptedMask9FF = 0u;
    bool acceptedGateOpen = false;
    bool acceptedMaskChanged = false;
    bool backupCtx08FromCtx04 = false;
    bool backupCtx10FromCtx0C = false;
    bool callAcceptedProducer14614 = false;
    bool callAcceptedProducer94434 = false;
    bool callAcceptedProducer9443C = false;
    bool fixedBonus250Requested = false;
    bool writeCtx3C = false;
    int32_t ctx3CValue = 0;
    bool continueMainLoop = true;
    bool writeDword801CCBB8 = false;
    uint32_t dword801CCBB8Value = 0u;
};

struct PrStageRunnerDirectMainLoopTail7A60Input {
    bool postFrameContinues = true;
    int32_t sceneEntryField352FallbackTickAdvance = 0;
    bool continuationGate1A7F8Known = false;
    bool continuationGate1A7F8ReturnsOne = false;
    bool frameExit1A3B8Known = false;
    bool frameExit1A3B8ReturnsOne = false;
};

struct PrStageRunnerDirectMainLoopTail7A60Result {
    PrStageRunnerDirectActionTrace trace{};
    bool callWait15350 = false;
    int32_t wait15350Arg = 0;
    bool callWait35560 = false;
    int32_t wait35560Arg = 0;
    bool callPresentFrame = false;
    bool callContinuationGate1A7F8 = false;
    bool continuationGateKnown = false;
    bool continuationGateAllowsNextFrame = false;
    bool callFrameExit1A3B8 = false;
    bool frameExitKnown = false;
    bool frameExitTaken = false;
    bool callEarlyExit1A694 = false;
    bool callEarlyExit35838 = false;
    bool setWord59OnFrameExit = false;
    bool cleanupRequiredKnown = false;
    bool cleanupRequired = false;
    bool clearWord50 = false;
    bool clearWord42 = false;
    bool clearWord61 = false;
    uint8_t cleanupDrainFrameCount = 0u;
    bool finalCall1B120 = false;
    bool finalCall26FA4 = false;
    bool finalCall357D4Zero = false;
    bool finalCall1A694 = false;
};

struct PrStageRunnerDirectFinalReturn7A60Input {
    bool word59Known = false;
    bool word59IsOne = false;
    bool word60Known = false;
    bool word60IsOne = false;
    bool recordsModeDAEqualsOne = false;
    bool stageStatus166ACKnown = false;
    uint8_t stageStatus166AC = 0u;
};

struct PrStageRunnerDirectFinalReturn7A60Result {
    bool resultKnown = false;
    int32_t result = -1;
    bool word59Known = false;
    bool word59IsOne = false;
    bool word60Known = false;
    bool word60IsOne = false;
    bool calls166AC = false;
    bool gate78OpenResultKnown = false;
    int32_t gate78OpenResult = -1;
    bool recordsModeReturnsOne = false;
    bool stageStatusReturnsOne = false;
};

struct PrStageRunnerDirectTimecodeState801C7560 {
    int32_t dword801D303CPreviousXaReadValue = 0;
    int32_t tick801C364C = 0;
    int32_t bar801C3644 = 1;
    int32_t beat801C3646 = 1;
    int32_t tickInBeat801C3647 = 1;
};

struct PrStageRunnerDirectTimecodeSnapshot801C7560 {
    bool known = false;
    PrStageRunnerDirectTimecodeState801C7560 state{};
    uint32_t word801C3644 = 0u;
};

struct PrStageRunnerDirectTimecodeInput801C7560 {
    PrStageRunnerDirectTimecodeState801C7560 state{};
    int32_t sceneEntryField348 = 0;
    int32_t sceneEntryField352FallbackTickAdvance = 0;
    int32_t sceneEntryField356TickOffset = 0;
    int32_t xaSectorReadValueA7A4 = 0;
};

struct PrStageRunnerDirectTimecodeResult801C7560 {
    PrStageRunnerDirectTimecodeState801C7560 state{};
    bool usedFallbackField352Increment = false;
    int32_t previousXaReadValue801D303C = 0;
    int32_t xaSectorReadValueA7A4 = 0;
    int32_t sceneEntryField348 = 0;
    int32_t sceneEntryField352FallbackTickAdvance = 0;
    int32_t sceneEntryField356TickOffset = 0;
    int32_t xaProduct348ByReadValue32 = 0;
    int32_t xaRoundedTickBeforeOffset = 0;
    int32_t callbackReturnValue = 0;
};

struct PrStageRunnerDirectEventStreamCursor801C9094 {
    bool valid = false;
    uint32_t count = 0u;
    uint32_t index = 0u;
    bool dueEventKnown = false;
    int32_t dueFrame = 0;
    uint32_t flags04 = 0u;
    uint8_t byte29 = 0u;
    uint8_t byte30 = 0u;
};

struct PrStageRunnerDirectContext801C9094 {
    uint32_t flags00 = 0u;
    int32_t tick0C = 0;
    uint16_t word4E = 0u;
    uint16_t word54 = 0u;
    uint16_t word72 = 0u;
    uint16_t word8A = 0u;
    int16_t word8C = -1;
    int16_t word8E = -1;
    uint16_t word90 = 0u;
    int16_t word9E = -1;
    int16_t wordA0 = -1;
    uint16_t wordA2 = 0u;
    uint16_t word72SceneFamily = 0u;
    uint16_t word72RightRank = 0u;
    uint16_t wordE4 = 0u;
    uint16_t word72Rating = 0u;
    uint16_t word76 = 0u;
    uint16_t word78 = 0u;
};

struct PrStageRunnerDirectGlobals801C9094 {
    int16_t word8008ECF8 = 0;
    int16_t word8008ECFA = 0;
    int16_t word8008ECFC = 0;
    int16_t word8008ECFE = 0;
    uint16_t unk8008ED00 = 0u;
    bool unk8008ED1C = false;
    uint16_t unk8008ED20 = 0u;
    uint16_t eventStreamFlag = 0u;
    uint16_t eventStreamId = 0u;
    bool eventStreamDone = false;
    int32_t dword801D3054 = 0;
    int32_t dword801D3058 = 0;
    uint32_t dword801CCBB8 = 0u;
    int32_t dword801D3048 = 0;
    uint32_t dword801D304C = 0u;
    uint32_t dword801D3050 = 0u;
};

struct PrStageRunnerDirectFrameUpdate9094Input {
    bool sameFrameCtxKnown = false;
    bool sameFrameGlobalsKnown = false;
    PrStageRunnerDirectContext801C9094 ctx{};
    PrStageRunnerDirectGlobals801C9094 globals{};
    PrStageRunnerDirectEventStreamCursor801C9094 activeFlagStream{};
    PrStageRunnerDirectEventStreamCursor801C9094 activeIdStream{};
    bool eventStreamFlagDescriptorValid = false;
    bool eventStreamIdDescriptorValid = false;
    bool compactRailRecKnown = false;
    int16_t compactRailRecSignedMode = 0;
    int16_t compactRailRecStartIndex = 0;
    uint32_t compactRailRecPrimaryStreamPtr = 0;
    uint32_t compactRailRecSecondaryStreamPtr = 0;
};

struct PrStageRunnerDirectFrameUpdate9094Result {
    PrStageRunnerDirectActionTrace trace{};
    PrStageRunnerDirectContext801C9094 ctx{};
    PrStageRunnerDirectGlobals801C9094 globals{};
    PrStageRunnerDirectCtx0FinalFlagsSnapshot801C9094
        ctx0FinalFlagsSnapshot801C9094{};
    bool ctx0FinalInputsComplete801C9094 = false;
    bool returnEarlyAfterFlag40Reset = false;
    bool consumedFlagStreamEvent = false;
    bool consumedIdStreamEvent = false;
    uint32_t consumedEventFlags04 = 0u;
    bool setCtxFlag0800 = false;
    bool resetCompactRailLanes = false;
    bool setCtxFlag100000 = false;
    bool setCtxFlag0100 = false;
    bool setCtxFlag40000 = false;
    bool setCtxFlag80000 = false;
    bool setCtxFlag20000 = false;
    uint8_t resolvedEventStreamId = 0u;
    bool eventStreamDoneRaised = false;
};

PrStageRunnerDirectInitResult801C79E4
PrStageRunnerDirectApplySub801C79E4(
    const PrStageRunnerDirectInitInput801C79E4& input);

PrStageRunnerDirectStartupSetup7A60Result
PrStageRunnerDirectResolveStartupSetup7A60(
    const PrStageRunnerDirectStartupSetup7A60Input& input);

PrStageRunnerDirectFinalReturn7A60Result
PrStageRunnerDirectResolveFinalReturn7A60(
    const PrStageRunnerDirectFinalReturn7A60Input& input);

PrStageRunnerDirectPreamble7A60Result
PrStageRunnerDirectResolvePreamble7A60(
    const PrStageRunnerDirectPreamble7A60Input& input);

PrStageRunnerDirectInputDispatch7A60Result
PrStageRunnerDirectResolveInputDispatch7A60(
    const PrStageRunnerDirectInputDispatch7A60Input& input);

PrStageRunnerDirectCtx0FlagsSnapshot801C3640
PrStageRunnerDirectMakeCtx0BaseFlagsSnapshot801C7A60(
    bool acceptedInputHit);

PrStageRunnerDirectPostFrame7A60Result
PrStageRunnerDirectResolvePostFrame7A60(
    const PrStageRunnerDirectPostFrame7A60Input& input);

PrStageRunnerDirectMainLoopTail7A60Result
PrStageRunnerDirectResolveMainLoopTail7A60(
    const PrStageRunnerDirectMainLoopTail7A60Input& input);

PrStageRunnerDirectTimecodeResult801C7560
PrStageRunnerDirectUpdateTimecode801C7560(
    const PrStageRunnerDirectTimecodeInput801C7560& input);

uint32_t PrStageRunnerDirectPackTimecodeWord801C3644(
    const PrStageRunnerDirectTimecodeState801C7560& state);

PrStageRunnerDirectTimecodeSnapshot801C7560
PrStageRunnerDirectMakeTimecodeSnapshot801C7560(
    const PrStageRunnerDirectTimecodeState801C7560& state);

PrStageRunnerDirectFrameUpdate9094Result
PrStageRunnerDirectFrameUpdate801C9094(
    const PrStageRunnerDirectFrameUpdate9094Input& input);

PrStageRunnerDirectCtx0FinalFlagsSnapshot801C9094
PrStageRunnerDirectMakeCtx0FinalFlagsSnapshot801C9094(
    const PrStageRunnerDirectFrameUpdate9094Result& result);
