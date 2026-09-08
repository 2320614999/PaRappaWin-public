#pragma once

#include "pr_ss0_scene0_int_side_effect_direct.h"
#include "pr_stage1_loader_spu_hal.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace PrSS0Scene0IntSpuDirect {

constexpr uint32_t kVabActionCount8001A8F0 = 4u;
constexpr uint32_t kPadStartComCallCount80026E4C = 6u;
constexpr uint32_t kSsSequenceCallbackRowCount8002ADE0 = 32u;
constexpr uint32_t kSsSequenceCallbackColumnCount8002ADE0 = 16u;
constexpr uint32_t kSsSequenceCallbackEntryCount8002ADE0 =
    kSsSequenceCallbackRowCount8002ADE0 *
    kSsSequenceCallbackColumnCount8002ADE0;
constexpr uint32_t kSpuTransferBusyPollThreshold80029E6C = 0x0F01u;
constexpr uint32_t kSsTickFlagRouteCount8002B200 = 8u;
constexpr uint32_t kSsMidiStatusRouteCount8002BC40 = 5u;
constexpr uint32_t kSsMidiChannelCount8002BEEC = 16u;

enum class Status8001A8F0 : uint8_t {
    SourceUnknown = 0,
    IntLoadRejected,
    LoaderCandidateRejected,
    VabBlockMissing,
    VabRequestMalformed,
    ArchiveRangeInvalid,
    DecoderPreflightRejected,
    PadStartComSourceMalformed,
    Accepted,
};

struct PadStartComSource80026E4C {
    bool staticBodyKnown = false;
    bool cuePointerKnown = false;
    uint32_t cuePointer80094410 = 0u;
    bool replayValueAuthority = false;
};

struct PadStartComCall80026E4C {
    uint32_t order = 0u;
    uint32_t lowerFunction = 0u;
    std::array<int32_t, 3> args{};
    uint32_t argCount = 0u;
    bool lowerSideEffectCommitted = false;
};

struct SsSequenceCallbackSurface8002AC20 {
    bool known = false;
    bool completeWithinLimits = false;
    uint32_t sourceFunction = 0u;
    uint32_t initializerFunction = 0u;
    uint32_t consumerFunction = 0u;
    uint32_t consumerJalr = 0u;
    uint32_t callbackTableBase = 0u;
    uint32_t rowCount = 0u;
    uint32_t columnCount = 0u;
    std::array<uint32_t, kSsSequenceCallbackEntryCount8002ADE0>
        callbackTargets{};
    bool callbackTableZeroInitialized = false;
    int32_t dword80095C4CTickRate = 0;
    uint32_t dword800928CCActiveSequenceMask = 0u;
    uint32_t dword800917A4ReentryGuard = 0u;
    bool psxMemoryAuthority = false;
    bool interruptCallbackAuthority = false;
    bool dynamicReplayAuthority = false;
    bool oldWinS0Authority = false;
    bool stage2PlusAuthority = false;
};

struct SpuTransferCompletionSurface8002AC20 {
    bool known = false;
    bool completeWithinLimits = false;
    uint32_t sourceFunction = 0u;
    uint32_t lowerInitFunction = 0u;
    uint32_t transferInitFunction = 0u;
    uint32_t installFunction = 0u;
    uint32_t dmaCallbackWrapperFunction = 0u;
    uint32_t dmaChannel = 0u;
    uint32_t interruptHandlerFunction = 0u;
    uint32_t callbackJalr = 0u;
    uint32_t callbackSlotAddress = 0u;
    uint32_t callbackInitialValue = 0u;
    uint32_t callbackArgument = 0u;
    uint32_t fallbackEventClass = 0u;
    uint32_t fallbackEventSpec = 0u;
    uint32_t openEventMode = 0u;
    uint32_t openEventArgument = 0u;
    uint32_t installOnceGuardAddress = 0u;
    uint32_t installOnceGuardSetValue = 0u;
    uint32_t eventHandleAddress = 0u;
    uint32_t spuControlOffset = 0u;
    uint16_t spuControlClearMask = 0u;
    uint16_t spuControlBusyMask = 0u;
    uint32_t busyPollThreshold = 0u;
    uint32_t preInterruptDelayLoopCount = 0u;
    uint32_t preInterruptDelayIterations = 0u;
    uint32_t preInterruptDelaySeed = 0u;
    uint32_t preInterruptDelayMultiplier = 0u;
    uint32_t preInterruptDelayGuardAddress = 0u;
    bool preInterruptDelayRunsWhenGuardZero = false;
    bool callbackSlotZeroInitialized = false;
    bool fallbackEventOpenedAndEnabled = false;
    uint32_t reverbGuardFunction = 0u;
    bool reverbGuardSavesClearsRestoresSlot = false;
    bool callbackWriterClosure = false;
    bool psxMemoryAuthority = false;
    bool dmaCallbackAuthority = false;
    bool dynamicReplayAuthority = false;
    bool oldWinS0Authority = false;
    bool stage2PlusAuthority = false;
};

enum class SpuTransferCompletionDispatchKind80029E6C : uint8_t {
    Unknown = 0,
    Callback,
    DeliverEvent,
};

struct SpuTransferCompletionDispatch80029E6C {
    bool known = false;
    SpuTransferCompletionDispatchKind80029E6C kind =
        SpuTransferCompletionDispatchKind80029E6C::Unknown;
    uint32_t callbackTarget = 0u;
    uint32_t callbackArgument = 0u;
    uint32_t eventClass = 0u;
    uint32_t eventSpec = 0u;
    uint16_t spuControlClearMask = 0u;
    uint16_t spuControlBusyMask = 0u;
    uint32_t busyPollThreshold = 0u;
    bool callbackTargetAuthority = false;
    bool psxMemoryAuthority = false;
    bool dmaCallbackAuthority = false;
};

struct SsTickFlagRoute8002B200 {
    uint32_t order = 0u;
    uint32_t triggerMask = 0u;
    uint32_t lowerFunction = 0u;
    bool requiresBitOneBranch = false;
    bool clearsTrackFlags = false;
};

struct SsTickInterruptSurface8002B130 {
    bool known = false;
    bool completeWithinLimits = false;
    uint32_t sourceFunction = 0u;
    uint32_t startFunction = 0u;
    uint32_t installFunction = 0u;
    int32_t startModeArgument = 0;
    uint32_t interruptCallbackApi = 0u;
    uint32_t defaultHandlerSlotAddress = 0u;
    uint32_t defaultHandlerFunction = 0u;
    bool defaultHandlerStaticInitialized = false;
    uint32_t priorHandlerSlotAddress = 0u;
    uint32_t priorHandlerInitialValue = 0u;
    uint32_t priorHandlerWrapperFunction = 0u;
    uint32_t priorHandlerJalr = 0u;
    uint32_t defaultHandlerJalrFromPriorWrapper = 0u;
    uint32_t dividerWrapperFunction = 0u;
    uint32_t dividerGuardSlotAddress = 0u;
    uint32_t dividerGuardInitialValue = 0u;
    uint32_t dividerHandlerJalr = 0u;
    uint32_t reentryGuardAddress = 0u;
    uint32_t reentryGuardInitialValue = 0u;
    uint32_t activeSequenceMaskAddress = 0u;
    uint32_t activeSequenceMaskInitialValue = 0u;
    uint32_t sequenceCountAddress = 0u;
    uint32_t trackCountAddress = 0u;
    uint32_t sequenceStatePointerTableAddress = 0u;
    uint32_t trackStateStride = 0u;
    uint32_t trackFlagsOffset = 0u;
    uint32_t preTickFunction = 0u;
    std::array<SsTickFlagRoute8002B200,
               kSsTickFlagRouteCount8002B200>
        flagRoutes{};
    bool defaultHandlerWriterClosure = false;
    bool priorHandlerWriterClosure = false;
    bool psxMemoryAuthority = false;
    bool interruptCallbackAuthority = false;
    bool dynamicReplayAuthority = false;
    bool oldWinS0Authority = false;
    bool stage2PlusAuthority = false;
};

struct SsTickInterruptDispatch8002B170 {
    bool known = false;
    bool callsPriorHandler = false;
    uint32_t priorHandlerTarget = 0u;
    bool callsDefaultHandler = false;
    uint32_t defaultHandlerTarget = 0u;
    bool dividerGuardKnown = false;
    bool dividerGuardAfter = false;
    bool priorHandlerTargetAuthority = false;
    bool defaultHandlerTargetAuthority = false;
    bool interruptCallbackAuthority = false;
    bool lowerCallsCommitted = false;
};

struct SsTickTrackObservation8002B200 {
    bool known = false;
    int16_t sequenceIndex = -1;
    int16_t trackIndex = -1;
    std::array<uint32_t, kSsTickFlagRouteCount8002B200>
        flagsAtChecks{};
};

struct SsTickActionRequest8002B200 {
    uint32_t order = 0u;
    uint32_t routeOrder = 0u;
    uint32_t triggerMask = 0u;
    uint32_t lowerFunction = 0u;
    int16_t sequenceIndex = -1;
    int16_t trackIndex = -1;
    bool lowerCallCommitted = false;
};

struct SsTickTrackDispatch8002B200 {
    bool known = false;
    int16_t sequenceIndex = -1;
    int16_t trackIndex = -1;
    bool bitOneBranchEntered = false;
    std::vector<SsTickActionRequest8002B200> actions{};
    bool clearsTrackFlags = false;
    bool psxMemoryAuthority = false;
    bool interruptCallbackAuthority = false;
    bool lowerCallsCommitted = false;
};

enum class SsMidiEventKind8002BC40 : uint8_t {
    Unsupported = 0,
    NoteOn,
    ControlChange,
    ProgramChange,
    PitchBend,
    Meta,
};

struct SsMidiStatusRoute8002BC40 {
    uint8_t statusClass = 0u;
    SsMidiEventKind8002BC40 kind =
        SsMidiEventKind8002BC40::Unsupported;
    uint32_t lowerFunction = 0u;
    bool parserDecodesDeltaBeforeLower = false;
    bool lowerOwnsAdditionalBytes = false;
};

struct SsMidiParserSurface8002BA9C {
    bool known = false;
    bool headerAndDeltaCompleteWithinLimits = false;
    uint32_t tickEntryFunction = 0u;
    uint32_t schedulerFunction = 0u;
    uint32_t parserFunction = 0u;
    uint32_t deltaDecoderFunction = 0u;
    uint32_t sequenceStatePointerTableAddress = 0u;
    uint32_t trackStateStride = 0u;
    uint32_t cursorOffset = 0u;
    uint32_t runningStatusOffset = 0u;
    uint32_t channelOffset = 0u;
    uint32_t absoluteTimeOffset = 0u;
    uint32_t deltaOffset = 0u;
    std::array<SsMidiStatusRoute8002BC40,
               kSsMidiStatusRouteCount8002BC40>
        statusRoutes{};
    bool crossFileExplicitReferenceClosure = false;
    bool psxMemoryAuthority = false;
    bool lowerEventAuthority = false;
    bool dynamicReplayAuthority = false;
    bool oldWinS0Authority = false;
    bool stage2PlusAuthority = false;
};

struct SsMidiTrackObservation8002BC40 {
    bool known = false;
    int16_t sequenceIndex = -1;
    int16_t trackIndex = -1;
    std::vector<uint8_t> streamBytes{};
    std::size_t cursorBefore = 0u;
    uint8_t runningStatusBefore = 0u;
    uint8_t channelBefore = 0u;
    uint32_t absoluteTimeBefore = 0u;
};

struct SsMidiEventDispatch8002BC40 {
    bool known = false;
    int16_t sequenceIndex = -1;
    int16_t trackIndex = -1;
    bool explicitStatusByte = false;
    uint8_t eventByte = 0u;
    uint8_t statusClass = 0u;
    uint8_t runningStatusAfter = 0u;
    uint8_t channelAfter = 0u;
    SsMidiEventKind8002BC40 kind =
        SsMidiEventKind8002BC40::Unsupported;
    std::array<uint8_t, 2> parserDataBytes{};
    uint32_t parserDataByteCount = 0u;
    std::size_t cursorAfterParser = 0u;
    bool deltaDecoded = false;
    uint32_t deltaTicks10 = 0u;
    uint32_t absoluteTimeAfter = 0u;
    uint32_t lowerFunction = 0u;
    bool lowerOwnsAdditionalBytes = false;
    bool lowerCallCommitted = false;
    bool psxMemoryAuthority = false;
    bool lowerEventAuthority = false;
};

// The 8002BB30 scheduler is a separate state owner from the 8002BC40
// event-header parser.  The original track row keeps a signed countdown at
// +0x6E, a signed current tick at +0x70, and the next-event delta at +0x88.
// Keep those raw-width values explicit so the translated owner can preserve
// the negative-countdown and low-halfword stores instead of normalizing them
// into a host timer.
struct SsMidiSchedulerObservation8002BB30 {
    bool known = false;
    int16_t sequenceIndex = -1;
    int16_t trackIndex = -1;
    int16_t wordTrackOffset6E = 0;
    int16_t wordTrackOffset70 = 0;
    uint32_t dwordTrackOffset88 = 0u;
    // Values written to +0x88 by each bounded 8002BC40 call.  A zero value
    // means the scheduler must parse another immediate event before it can
    // re-evaluate the accumulated delta.
    std::vector<uint32_t> parserDeltaAfterCalls{};
};

enum class SsMidiSchedulerBranch8002BB30 : uint8_t {
    UnknownInput = 0,
    DueEventParserLoop,
    PositiveCountdownDecrement,
    NegativeCountdownRebase,
    CountdownArmAndDeltaDecrement,
};

struct SsMidiSchedulerDispatch8002BB30 {
    bool known = false;
    SsMidiSchedulerBranch8002BB30 branch =
        SsMidiSchedulerBranch8002BB30::UnknownInput;
    int16_t sequenceIndex = -1;
    int16_t trackIndex = -1;
    int16_t wordTrackOffset6EBefore = 0;
    int16_t wordTrackOffset6EAfter = 0;
    int16_t wordTrackOffset70 = 0;
    uint32_t dwordTrackOffset88Before = 0u;
    uint32_t dwordTrackOffset88After = 0u;
    int32_t result = 0;
    uint32_t parserCallCount = 0u;
    std::vector<uint32_t> parserDeltaAfterCalls{};
    bool parserInputExhausted = false;
    bool parserCallsBounded = false;
    bool psxMemoryAuthority = false;
    bool lowerEventAuthority = false;
    bool oldWinS0Authority = false;
    bool stage2PlusAuthority = false;
};

// Program-change (C0) is one of the parser's lower handlers.  It consumes
// one program byte from the track, writes it into the channel-indexed table
// at track +0x2C, then decodes the following variable-length delta through
// 8002D7D0 and stores the result at +0x88.
struct SsMidiProgramChangeObservation8002BFDC {
    bool known = false;
    int16_t sequenceIndex = -1;
    int16_t trackIndex = -1;
    uint8_t channel = 0u;
    uint8_t program = 0u;
    std::array<uint8_t, kSsMidiChannelCount8002BEEC>
        channelProgramsBefore{};
    std::vector<uint8_t> deltaStreamBytes{};
    std::size_t deltaCursorBefore = 0u;
    uint32_t absoluteTimeBefore = 0u;
};

struct SsMidiProgramChangeDispatch8002BFDC {
    bool known = false;
    int16_t sequenceIndex = -1;
    int16_t trackIndex = -1;
    uint8_t channel = 0u;
    uint8_t program = 0u;
    std::array<uint8_t, kSsMidiChannelCount8002BEEC>
        channelProgramsAfter{};
    uint32_t programTableOffset = 0u;
    std::size_t deltaCursorAfter = 0u;
    uint32_t deltaTicks10 = 0u;
    uint32_t absoluteTimeAfter = 0u;
    uint32_t dwordTrackOffset88After = 0u;
    uint32_t lowerFunction = 0u;
    uint32_t deltaDecoderFunction = 0u;
    bool lowerCallCommitted = false;
    bool psxMemoryAuthority = false;
    bool lowerEventAuthority = false;
    bool oldWinS0Authority = false;
    bool stage2PlusAuthority = false;
};

enum class SsMidiControlChangeBranch8002C054 : uint8_t {
    DefaultDeltaOnly = 0,
    VabIdStore,
    ChannelVolumeStore,
    ChannelPanStore,
    ProgramVolumeUpdate,
    Threshold64Dispatch,
    Timer91Dispatch,
    LowerOwnsAdditionalBytes,
};

// B0/control-change lower-handler state.  The handler consumes the
// controller value itself, updates only the selected track/channel fields,
// and either decodes the following delta or delegates the remaining bytes to
// one of its specialized lower routines.
struct SsMidiControlChangeObservation8002C054 {
    bool known = false;
    int16_t sequenceIndex = -1;
    int16_t trackIndex = -1;
    uint8_t channel = 0u;
    uint8_t controller = 0u;
    uint8_t value = 0u;
    int16_t vabId = -1;
    std::array<uint8_t, kSsMidiChannelCount8002BEEC>
        channelPrograms{};
    std::array<uint16_t, kSsMidiChannelCount8002BEEC>
        channelVolumes{};
    std::array<uint8_t, kSsMidiChannelCount8002BEEC>
        channelPans{};
    std::vector<uint8_t> deltaStreamBytes{};
    std::size_t deltaCursorBefore = 0u;
    uint32_t absoluteTimeBefore = 0u;
};

struct SsMidiControlChangeDispatch8002C054 {
    bool known = false;
    SsMidiControlChangeBranch8002C054 branch =
        SsMidiControlChangeBranch8002C054::DefaultDeltaOnly;
    int16_t sequenceIndex = -1;
    int16_t trackIndex = -1;
    uint8_t channel = 0u;
    uint8_t controller = 0u;
    uint8_t value = 0u;
    int16_t vabIdAfter = -1;
    std::array<uint8_t, kSsMidiChannelCount8002BEEC>
        channelProgramsAfter{};
    std::array<uint16_t, kSsMidiChannelCount8002BEEC>
        channelVolumesAfter{};
    std::array<uint8_t, kSsMidiChannelCount8002BEEC>
        channelPansAfter{};
    bool callsGsVoiceUpdate80033D08 = false;
    uint32_t gsVoiceUpdateFunction = 0u;
    uint16_t packedSequenceTrack = 0u;
    int16_t gsVoiceUpdateVabId = -1;
    uint8_t gsVoiceUpdateProgram = 0u;
    uint16_t gsVoiceUpdateVolume = 0u;
    uint8_t gsVoiceUpdatePan = 0u;
    bool callsSsVmSetProgVol = false;
    uint32_t ssVmSetProgVolFunction = 0u;
    bool callsThresholdFunction = false;
    uint32_t thresholdFunction = 0u;
    bool callsTimerFunction = false;
    uint32_t timerFunction = 0u;
    uint8_t timerArg0 = 0u;
    uint8_t timerArg1 = 0u;
    uint32_t lowerFunction = 0u;
    bool lowerOwnsAdditionalBytes = false;
    std::size_t deltaCursorAfter = 0u;
    bool deltaDecoded = false;
    uint32_t deltaTicks10 = 0u;
    uint32_t absoluteTimeAfter = 0u;
    uint32_t dwordTrackOffset88After = 0u;
    bool lowerCallCommitted = false;
    bool psxMemoryAuthority = false;
    bool lowerEventAuthority = false;
    bool oldWinS0Authority = false;
    bool stage2PlusAuthority = false;
};

// Pitch-bend (E0) consumes one data byte in its lower handler, submits it to
// the voice update helper, and then follows the common delta-decoder tail.
struct SsMidiPitchBendObservation8002D3D0 {
    bool known = false;
    int16_t sequenceIndex = -1;
    int16_t trackIndex = -1;
    uint8_t channel = 0u;
    uint8_t bendValue = 0u;
    int16_t vabId = -1;
    std::array<uint8_t, kSsMidiChannelCount8002BEEC>
        channelPrograms{};
    std::vector<uint8_t> deltaStreamBytes{};
    std::size_t deltaCursorBefore = 0u;
    uint32_t absoluteTimeBefore = 0u;
};

struct SsMidiPitchBendDispatch8002D3D0 {
    bool known = false;
    int16_t sequenceIndex = -1;
    int16_t trackIndex = -1;
    uint8_t channel = 0u;
    uint8_t bendValue = 0u;
    uint16_t packedSequenceTrack = 0u;
    int16_t voiceUpdateVabId = -1;
    uint8_t voiceUpdateProgram = 0u;
    uint8_t voiceUpdateValue = 0u;
    uint32_t voiceUpdateFunction = 0u;
    std::size_t deltaCursorAfter = 0u;
    uint32_t deltaTicks10 = 0u;
    uint32_t absoluteTimeAfter = 0u;
    uint32_t dwordTrackOffset88After = 0u;
    uint32_t lowerFunction = 0u;
    uint32_t deltaDecoderFunction = 0u;
    bool lowerCallCommitted = false;
    bool psxMemoryAuthority = false;
    bool lowerEventAuthority = false;
    bool oldWinS0Authority = false;
    bool stage2PlusAuthority = false;
};

enum class SsMidiMetaBranch8002D47C : uint8_t {
    UnsupportedResult81 = 0,
    EndOfTrackReset,
    EndOfTrackComplete,
    TempoUpdate,
};

// F0/meta lower-handler state.  Only the bounded 0x2F end-of-track and 0x51
// tempo forms are modeled here; other meta types retain the original
// result=0x51/no-write behavior until their dedicated lower routines are
// translated.
struct SsMidiMetaObservation8002D47C {
    bool known = false;
    int16_t sequenceIndex = -1;
    int16_t trackIndex = -1;
    uint8_t metaType = 0u;
    int16_t wordTrackOffset6E = 0;
    int16_t wordTrackOffset70 = 0;
    uint16_t wordTrackOffset72 = 0u;
    uint16_t wordTrackOffset74 = 0u;
    uint32_t dwordTrackOffset08StreamStart = 0u;
    uint32_t dwordTrackOffset04Cursor = 0u;
    uint32_t dwordTrackOffset0C = 0u;
    uint32_t dwordTrackOffset80AbsoluteTime = 0u;
    uint32_t dwordTrackOffset88Delta = 0u;
    uint32_t dwordTrackOffset90Flags = 0u;
    uint8_t byteTrackOffset00 = 0u;
    uint8_t byteTrackOffset27 = 0u;
    uint8_t byteTrackOffset2B = 0u;
    uint8_t byteTrackOffset3C = 0u;
    uint32_t tickRate80095C4C = 0u;
    std::array<uint8_t, 3> tempoBytes{};
    std::vector<uint8_t> deltaStreamBytes{};
    std::size_t deltaCursorBefore = 0u;
    uint32_t absoluteTimeBefore = 0u;
};

struct SsMidiMetaDispatch8002D47C {
    bool known = false;
    SsMidiMetaBranch8002D47C branch =
        SsMidiMetaBranch8002D47C::UnsupportedResult81;
    int16_t sequenceIndex = -1;
    int16_t trackIndex = -1;
    uint8_t metaType = 0u;
    uint32_t result = 0x51u;
    uint16_t wordTrackOffset72After = 0u;
    uint16_t wordTrackOffset6EAfter = 0u;
    uint16_t wordTrackOffset70After = 0u;
    uint32_t dwordTrackOffset04After = 0u;
    uint32_t dwordTrackOffset0CAfter = 0u;
    uint32_t dwordTrackOffset80After = 0u;
    uint32_t dwordTrackOffset88After = 0u;
    uint32_t dwordTrackOffset90After = 0u;
    uint32_t dwordTrackOffset8CAfter = 0u;
    uint8_t byteTrackOffset27After = 0u;
    uint8_t byteTrackOffset2BAfter = 0u;
    uint32_t tempoMicrosecondsPerQuarter = 0u;
    uint32_t tempoBpm = 0u;
    bool callsTempoProgramVolumeReset = false;
    uint32_t tempoProgramVolumeResetFunction = 0u;
    bool callsTrackEndCleanup = false;
    uint32_t trackEndCleanupFunction = 0u;
    uint32_t trackEndCleanupArg = 0u;
    uint32_t trackEndCleanupArg1 = 0u;
    uint32_t callsSequenceTrackEnd = 0u;
    uint32_t sequenceTrackEndFunction = 0u;
    std::size_t deltaCursorAfter = 0u;
    bool deltaDecoded = false;
    uint32_t deltaTicks10 = 0u;
    uint32_t absoluteTimeAfter = 0u;
    uint32_t lowerFunction = 0u;
    uint32_t deltaDecoderFunction = 0u;
    bool lowerCallCommitted = false;
    bool psxMemoryAuthority = false;
    bool lowerEventAuthority = false;
    bool oldWinS0Authority = false;
    bool stage2PlusAuthority = false;
};

// Controller-98 callback lower handler (8002C808).  The original routine
// mutates a small set of per-track bytes before optionally dispatching the
// callback-table entry for mode 40, then always consumes the following
// variable-length delta through 8002D7D0.  Keep the byte-width state explicit;
// the callback target is observed but is not invoked by this bounded owner.
enum class SsMidiCallbackBranch8002C808 : uint8_t {
    Unknown = 0,
    ArmAndDecode,
    AccumulateAndCallback,
    AccumulateNoCallback,
    Mode20Or30Decode,
};

struct SsMidiCallbackObservation8002C808 {
    bool known = false;
    int16_t sequenceIndex = -1;
    int16_t trackIndex = -1;
    uint8_t callbackValue = 0u;
    uint8_t byteTrackOffset16 = 0u;
    uint8_t byteTrackOffset21 = 0u;
    uint8_t byteTrackOffset22 = 0u;
    uint8_t byteTrackOffset39 = 0u;
    uint8_t byteTrackOffset40 = 0u;
    uint8_t byteTrackOffset41 = 0u;
    uint8_t byteTrackOffset42 = 0u;
    uint32_t callbackTarget = 0u;
    std::vector<uint8_t> deltaStreamBytes{};
    std::size_t deltaCursorBefore = 0u;
    uint32_t absoluteTimeBefore = 0u;
};

struct SsMidiCallbackDispatch8002C808 {
    bool known = false;
    SsMidiCallbackBranch8002C808 branch =
        SsMidiCallbackBranch8002C808::Unknown;
    int16_t sequenceIndex = -1;
    int16_t trackIndex = -1;
    uint8_t callbackValue = 0u;
    uint8_t byteTrackOffset16After = 0u;
    uint8_t byteTrackOffset21After = 0u;
    uint8_t byteTrackOffset22After = 0u;
    uint8_t byteTrackOffset39After = 0u;
    uint8_t byteTrackOffset40After = 0u;
    uint8_t byteTrackOffset41After = 0u;
    uint8_t byteTrackOffset42After = 0u;
    uint32_t callbackTarget = 0u;
    bool callbackTargetPresent = false;
    bool callbackDispatchObserved = false;
    bool callbackCallCommitted = false;
    std::size_t deltaCursorAfter = 0u;
    bool deltaDecoded = false;
    uint32_t deltaTicks10 = 0u;
    uint32_t absoluteTimeAfter = 0u;
    uint32_t dwordTrackOffset88After = 0u;
    uint32_t lowerFunction = 0u;
    uint32_t deltaDecoderFunction = 0u;
    bool lowerCallCommitted = false;
    bool psxMemoryAuthority = false;
    bool lowerEventAuthority = false;
    bool oldWinS0Authority = false;
    bool stage2PlusAuthority = false;
};

// Controller-99 callback/state lower handler (8002C938).  Its mode byte at
// +22 selects the arm (20), normal (30), or accumulation paths; mode 30 also
// consumes the pending +40 counter and may restore the saved cursor at +12.
// The common delta decoder still runs on every accepted path.
enum class SsMidiCallbackGateBranch8002C938 : uint8_t {
    Unknown = 0,
    Mode20Arm,
    Mode30ClearGate,
    Mode30RetainPending,
    Mode30FinishPending,
    ModeOtherAccumulate,
};

struct SsMidiCallbackGateObservation8002C938 {
    bool known = false;
    int16_t sequenceIndex = -1;
    int16_t trackIndex = -1;
    uint8_t inputValue = 0u;
    uint32_t dwordTrackOffset04Cursor = 0u;
    uint32_t dwordTrackOffset0CStoredCursor = 0u;
    uint8_t byteTrackOffset16 = 0u;
    uint8_t byteTrackOffset22 = 0u;
    uint8_t byteTrackOffset39 = 0u;
    uint8_t byteTrackOffset40 = 0u;
    uint8_t byteTrackOffset42 = 0u;
    std::vector<uint8_t> deltaStreamBytes{};
    std::size_t deltaCursorBefore = 0u;
    uint32_t absoluteTimeBefore = 0u;
};

struct SsMidiCallbackGateDispatch8002C938 {
    bool known = false;
    SsMidiCallbackGateBranch8002C938 branch =
        SsMidiCallbackGateBranch8002C938::Unknown;
    int16_t sequenceIndex = -1;
    int16_t trackIndex = -1;
    uint8_t inputValue = 0u;
    uint32_t dwordTrackOffset04After = 0u;
    uint32_t dwordTrackOffset0CAfter = 0u;
    uint8_t byteTrackOffset16After = 0u;
    uint8_t byteTrackOffset22After = 0u;
    uint8_t byteTrackOffset39After = 0u;
    uint8_t byteTrackOffset40After = 0u;
    uint8_t byteTrackOffset42After = 0u;
    std::size_t deltaCursorAfter = 0u;
    bool deltaDecoded = false;
    uint32_t deltaTicks10 = 0u;
    uint32_t absoluteTimeAfter = 0u;
    uint32_t dwordTrackOffset88After = 0u;
    uint32_t result = 0u;
    uint32_t lowerFunction = 0u;
    uint32_t deltaDecoderFunction = 0u;
    bool lowerCallCommitted = false;
    bool psxMemoryAuthority = false;
    bool lowerEventAuthority = false;
    bool oldWinS0Authority = false;
    bool stage2PlusAuthority = false;
};

// Controller-100/101 byte-counter lower handlers.  The original `8002CA7C`
// and `8002CAF4` routines differ only in whether the incoming byte is stored
// at +19 or +20; both increment the byte counter at +41 and finish through
// 8002D7D0.
struct SsMidiTrackByteCounterObservation8002CA7C {
    bool known = false;
    int16_t sequenceIndex = -1;
    int16_t trackIndex = -1;
    uint8_t inputValue = 0u;
    uint8_t byteTrackOffset19 = 0u;
    uint8_t byteTrackOffset41 = 0u;
    std::vector<uint8_t> deltaStreamBytes{};
    std::size_t deltaCursorBefore = 0u;
    uint32_t absoluteTimeBefore = 0u;
};

struct SsMidiTrackByteCounterDispatch8002CA7C {
    bool known = false;
    int16_t sequenceIndex = -1;
    int16_t trackIndex = -1;
    uint8_t inputValue = 0u;
    uint8_t byteTrackOffset19After = 0u;
    uint8_t byteTrackOffset41After = 0u;
    std::size_t deltaCursorAfter = 0u;
    bool deltaDecoded = false;
    uint32_t deltaTicks10 = 0u;
    uint32_t absoluteTimeAfter = 0u;
    uint32_t dwordTrackOffset88After = 0u;
    uint32_t lowerFunction = 0u;
    uint32_t deltaDecoderFunction = 0u;
    bool lowerCallCommitted = false;
    bool psxMemoryAuthority = false;
    bool lowerEventAuthority = false;
    bool oldWinS0Authority = false;
    bool stage2PlusAuthority = false;
};

struct SsMidiTrackByteCounterObservation8002CAF4 {
    bool known = false;
    int16_t sequenceIndex = -1;
    int16_t trackIndex = -1;
    uint8_t inputValue = 0u;
    uint8_t byteTrackOffset20 = 0u;
    uint8_t byteTrackOffset41 = 0u;
    std::vector<uint8_t> deltaStreamBytes{};
    std::size_t deltaCursorBefore = 0u;
    uint32_t absoluteTimeBefore = 0u;
};

struct SsMidiTrackByteCounterDispatch8002CAF4 {
    bool known = false;
    int16_t sequenceIndex = -1;
    int16_t trackIndex = -1;
    uint8_t inputValue = 0u;
    uint8_t byteTrackOffset20After = 0u;
    uint8_t byteTrackOffset41After = 0u;
    std::size_t deltaCursorAfter = 0u;
    bool deltaDecoded = false;
    uint32_t deltaTicks10 = 0u;
    uint32_t absoluteTimeAfter = 0u;
    uint32_t dwordTrackOffset88After = 0u;
    uint32_t lowerFunction = 0u;
    uint32_t deltaDecoderFunction = 0u;
    bool lowerCallCommitted = false;
    bool psxMemoryAuthority = false;
    bool lowerEventAuthority = false;
    bool oldWinS0Authority = false;
    bool stage2PlusAuthority = false;
};

// Controller-121 reset lower handler (8002C740).  It resets the current
// channel's program/pan scratch, restores volume 127 and pan 64, then runs
// the common delta decoder.  The two helper calls are retained as symbolic
// side effects; they are not replaced with host audio behavior.
struct SsMidiControllerResetObservation8002C740 {
    bool known = false;
    int16_t sequenceIndex = -1;
    int16_t trackIndex = -1;
    uint8_t channel = 0u;
    uint8_t byteTrackOffset19 = 0u;
    uint8_t byteTrackOffset20 = 0u;
    std::array<uint8_t, kSsMidiChannelCount8002BEEC>
        channelProgramScratch{};
    std::array<uint16_t, kSsMidiChannelCount8002BEEC>
        channelVolumeScratch{};
    std::array<uint8_t, kSsMidiChannelCount8002BEEC>
        channelPanScratch{};
    std::vector<uint8_t> deltaStreamBytes{};
    std::size_t deltaCursorBefore = 0u;
    uint32_t absoluteTimeBefore = 0u;
};

struct SsMidiControllerResetDispatch8002C740 {
    bool known = false;
    int16_t sequenceIndex = -1;
    int16_t trackIndex = -1;
    uint8_t channel = 0u;
    uint8_t byteTrackOffset19After = 0u;
    uint8_t byteTrackOffset20After = 0u;
    std::array<uint8_t, kSsMidiChannelCount8002BEEC>
        channelProgramScratchAfter{};
    std::array<uint16_t, kSsMidiChannelCount8002BEEC>
        channelVolumeScratchAfter{};
    std::array<uint8_t, kSsMidiChannelCount8002BEEC>
        channelPanScratchAfter{};
    uint8_t selectedProgramIndex = 0u;
    uint8_t selectedProgramScratchAfter = 0u;
    uint16_t selectedVolumeAfter = 0u;
    uint8_t selectedPanAfter = 0u;
    bool callsTimerReset = false;
    uint32_t timerResetFunction = 0u;
    bool callsThresholdReset = false;
    uint32_t thresholdResetFunction = 0u;
    std::size_t deltaCursorAfter = 0u;
    bool deltaDecoded = false;
    uint32_t deltaTicks10 = 0u;
    uint32_t absoluteTimeAfter = 0u;
    uint32_t dwordTrackOffset88After = 0u;
    uint32_t lowerFunction = 0u;
    uint32_t deltaDecoderFunction = 0u;
    bool lowerCallCommitted = false;
    bool psxMemoryAuthority = false;
    bool lowerEventAuthority = false;
    bool oldWinS0Authority = false;
    bool stage2PlusAuthority = false;
};

enum class SsMidiController65Band8002C5DC : uint8_t {
    NoTones = 0,
    Below64,
    Inclusive64To127,
    Above127,
};

// Controller-65 tone-attribute rewrite lower handler (8002C5DC).  The
// original reads the selected program's tone count through 8002F034, copies
// each tone record with 8002F200, writes it back with 8003037C, and then
// decodes the common delta.  The source function's temporary band variable
// is retained as an observation; no host tone mutation is invented here.
struct SsMidiController65Observation8002C5DC {
    bool known = false;
    int16_t sequenceIndex = -1;
    int16_t trackIndex = -1;
    int16_t vabId = -1;
    uint8_t program = 0u;
    uint8_t inputValue = 0u;
    uint8_t toneCount = 0u;
    std::vector<uint8_t> deltaStreamBytes{};
    std::size_t deltaCursorBefore = 0u;
    uint32_t absoluteTimeBefore = 0u;
};

struct SsMidiController65Dispatch8002C5DC {
    bool known = false;
    int16_t sequenceIndex = -1;
    int16_t trackIndex = -1;
    int16_t vabId = -1;
    uint8_t program = 0u;
    uint8_t inputValue = 0u;
    uint8_t toneCount = 0u;
    SsMidiController65Band8002C5DC band =
        SsMidiController65Band8002C5DC::NoTones;
    uint32_t toneReadCallCount = 0u;
    uint32_t toneWriteCallCount = 0u;
    uint32_t toneReadFunction = 0u;
    uint32_t toneWriteFunction = 0u;
    std::size_t deltaCursorAfter = 0u;
    bool deltaDecoded = false;
    uint32_t deltaTicks10 = 0u;
    uint32_t absoluteTimeAfter = 0u;
    uint32_t dwordTrackOffset88After = 0u;
    uint32_t lowerFunction = 0u;
    uint32_t deltaDecoderFunction = 0u;
    bool lowerCallCommitted = false;
    bool psxMemoryAuthority = false;
    bool lowerEventAuthority = false;
    bool oldWinS0Authority = false;
    bool stage2PlusAuthority = false;
};

enum class SsMidiController6Branch8002CB6C : uint8_t {
    Unknown = 0,
    ArmPending,
    ToneRewriteBase,
    ToneRewriteMode1,
    ToneRewriteMode2,
    TrackCounterReset,
    DynamicToneUpdateAll,
    DynamicToneUpdateSingle,
    DeltaOnly,
};

// Controller-6 lower handler (8002CB6C).  The routine has three independent
// byte counters (+16/+39, +41, +42), copies tone records through the original
// 8002F200/8003037C helpers, and optionally emits 8002D10C for every tone or
// one selected tone.  This owner records the bounded loop/call surfaces and
// the exact counter resets, while leaving the helper bodies to their own
// translated owners.
struct SsMidiController6Observation8002CB6C {
    bool known = false;
    int16_t sequenceIndex = -1;
    int16_t trackIndex = -1;
    int16_t vabId = -1;
    uint8_t channel = 0u;
    uint8_t program = 0u;
    uint8_t inputValue = 0u;
    uint8_t toneCount = 0u;
    uint8_t byteTrackOffset16 = 0u;
    uint8_t byteTrackOffset19 = 0u;
    uint8_t byteTrackOffset20 = 0u;
    uint8_t byteTrackOffset21 = 0u;
    uint8_t byteTrackOffset22 = 0u;
    uint8_t byteTrackOffset39 = 0u;
    uint8_t byteTrackOffset40 = 0u;
    uint8_t byteTrackOffset41 = 0u;
    uint8_t byteTrackOffset42 = 0u;
    std::vector<uint8_t> deltaStreamBytes{};
    std::size_t deltaCursorBefore = 0u;
    uint32_t absoluteTimeBefore = 0u;
};

struct SsMidiController6Dispatch8002CB6C {
    bool known = false;
    SsMidiController6Branch8002CB6C branch =
        SsMidiController6Branch8002CB6C::Unknown;
    int16_t sequenceIndex = -1;
    int16_t trackIndex = -1;
    int16_t vabId = -1;
    uint8_t channel = 0u;
    uint8_t program = 0u;
    uint8_t inputValue = 0u;
    uint8_t toneCount = 0u;
    uint8_t byteTrackOffset16After = 0u;
    uint8_t byteTrackOffset19After = 0u;
    uint8_t byteTrackOffset20After = 0u;
    uint8_t byteTrackOffset21After = 0u;
    uint8_t byteTrackOffset22After = 0u;
    uint8_t byteTrackOffset39After = 0u;
    uint8_t byteTrackOffset40After = 0u;
    uint8_t byteTrackOffset41After = 0u;
    uint8_t byteTrackOffset42After = 0u;
    uint32_t toneReadCallCount = 0u;
    uint32_t toneWriteCallCount = 0u;
    uint32_t toneUpdateCallCount = 0u;
    uint32_t toneReadFunction = 0u;
    uint32_t toneWriteFunction = 0u;
    uint32_t toneUpdateFunction = 0u;
    uint8_t toneUpdateMode = 0u;
    uint8_t toneUpdateValue = 0u;
    uint32_t baseToneRewriteValue = 0u;
    bool counter41Reset = false;
    bool counter42Reset = false;
    bool pendingCallbackArmed = false;
    std::size_t deltaCursorAfter = 0u;
    bool deltaDecoded = false;
    uint32_t deltaTicks10 = 0u;
    uint32_t absoluteTimeAfter = 0u;
    uint32_t dwordTrackOffset88After = 0u;
    uint32_t lowerFunction = 0u;
    uint32_t deltaDecoderFunction = 0u;
    bool lowerCallCommitted = false;
    bool psxMemoryAuthority = false;
    bool lowerEventAuthority = false;
    bool oldWinS0Authority = false;
    bool stage2PlusAuthority = false;
};

enum class SsMidiNoteAction8002BEEC : uint8_t {
    Suppressed = 0,
    NoteOn,
    NoteOff,
};

struct SsMidiNoteSurface8002BEEC {
    bool known = false;
    bool noteDecisionCompleteWithinLimits = false;
    bool hostToneLayerSelectionCompleteWithinLimits = false;
    bool hostNoteOffOwnershipCompleteWithinLimits = false;
    uint32_t noteDecisionFunction = 0u;
    uint32_t noteOnFunction = 0u;
    uint32_t noteOffFunction = 0u;
    uint32_t sequenceStatePointerTableAddress = 0u;
    uint32_t trackStateStride = 0u;
    uint32_t channelOffset = 0u;
    uint32_t channelPanBaseOffset = 0u;
    uint32_t channelProgramBaseOffset = 0u;
    uint32_t vabIdOffset = 0u;
    uint32_t channelVolumeBaseOffset = 0u;
    uint32_t sequenceEnabledOffset = 0u;
    uint32_t sequenceVolumeLeftOffset = 0u;
    uint32_t sequenceVolumeRightOffset = 0u;
    uint32_t lastVelocityOffset = 0u;
    uint32_t muteMaskOffset = 0u;
    uint32_t programAttributeStride = 0u;
    uint32_t toneAttributeStride = 0u;
    uint32_t toneNoteMinOffset = 0u;
    uint32_t toneNoteMaxOffset = 0u;
    uint32_t maxTonesPerProgram = 0u;
    bool crossFileExplicitReferenceScanCompleteWithinScope = false;
    bool psxMemoryAuthority = false;
    bool psxVoiceIdentityAuthority = false;
    bool psxSpuTimingAuthority = false;
    bool dynamicReplayAuthority = false;
    bool oldWinS0Authority = false;
    bool stage2PlusAuthority = false;
};

struct SsMidiNoteTrackState8002BEEC {
    bool known = false;
    int16_t sequenceIndex = -1;
    int16_t trackIndex = -1;
    uint8_t channel = 0u;
    int16_t vabId = -1;
    uint16_t sequenceEnabled = 0u;
    uint16_t sequenceVolumeRight = 0u;
    uint16_t muteMask = 0u;
    std::array<uint8_t, kSsMidiChannelCount8002BEEC> channelPan{};
    std::array<uint8_t, kSsMidiChannelCount8002BEEC> channelProgram{};
    std::array<uint16_t, kSsMidiChannelCount8002BEEC> channelVolume{};
    int16_t lastVelocity = 0;
};

struct SsMidiNoteDispatch8002BEEC {
    bool known = false;
    SsMidiNoteAction8002BEEC action =
        SsMidiNoteAction8002BEEC::Suppressed;
    int16_t sequenceIndex = -1;
    int16_t trackIndex = -1;
    uint16_t packedSequenceTrack = 0u;
    uint8_t channel = 0u;
    int16_t vabId = -1;
    uint8_t program = 0u;
    uint8_t note = 0u;
    uint8_t velocity = 0u;
    uint8_t pan = 0u;
    uint16_t channelVolume = 0u;
    uint16_t sequenceVolumeLeft = 0u;
    uint16_t sequenceVolumeRight = 0u;
    uint32_t scaledVelocity = 0u;
    uint32_t lowerFunction = 0u;
    bool muted = false;
    bool sequenceEnabled = false;
    bool lastVelocityUpdated = false;
    int16_t lastVelocityAfter = 0;
    bool lowerCallCommitted = false;
    bool hostProjectionReady = false;
    bool psxMemoryAuthority = false;
    bool psxVoiceIdentityAuthority = false;
    bool psxSpuTimingAuthority = false;
};

struct SsDriverFlushSurface80026ECC {
    bool known = false;
    bool wrapperAndReentryControlCompleteWithinLimits = false;
    uint32_t wrapperFunction = 0u;
    uint32_t wrapperBusyFlagAddress = 0u;
    uint32_t lowerFlushFunction = 0u;
    uint32_t reentryGuardAddress = 0u;
    uint32_t driverCommitFunction = 0u;
    uint32_t lowerFlushExplicitCodeXrefCount = 0u;
    uint32_t comod0DirectWrapperCallCount = 0u;
    uint32_t comod1DirectWrapperCallCount = 0u;
    bool comod0PathEntersSequenceScheduler = false;
    bool psxSpuRegisterAuthority = false;
    bool dynamicReplayAuthority = false;
    bool oldWinS0Authority = false;
    bool stage2PlusAuthority = false;
};

struct SsDriverFlushState80026ECC {
    int32_t wrapperBusyFlag800943B4 = 0;
    int32_t reentryGuard800917A4 = 0;
};

using SsDriverCommitSink80032B00 = int32_t (*)(void* userData);

struct SsDriverFlushDispatch80026ECC {
    bool known = false;
    bool wrapperSkippedByBusyFlag = false;
    bool lowerFlushEntered = false;
    bool lowerSkippedByReentryGuard = false;
    bool reentryGuardSetBeforeCommit = false;
    bool driverCommitExecuted = false;
    bool reentryGuardClearedAfterCommit = false;
    int32_t result = 0;
    bool hostProjection = false;
    bool psxSpuRegisterAuthority = false;
};

struct SsDriverVoiceCompletionSurface80032B00 {
    bool known = false;
    bool completionHistoryControlCompleteWithinLimits = false;
    uint32_t function = 0u;
    uint32_t voiceCountAddress = 0u;
    uint32_t completionRingIndexAddress = 0u;
    uint32_t completionRingBaseAddress = 0u;
    uint32_t completionScanDisableAddress = 0u;
    uint32_t spuVoiceBasePointerAddress = 0u;
    uint32_t voiceEnvelopeRegisterOffset = 0u;
    uint32_t voiceStateBaseAddress = 0u;
    uint32_t voiceStateStride = 0u;
    uint32_t noiseMaskClearFunction = 0u;
    uint32_t noiseMaskClearValue = 0u;
    uint32_t maxVoiceCount = 0u;
    uint32_t completionRingEntryCount = 0u;
    uint32_t stableCompletionEntryCount = 0u;
    bool psxEnvelopeAuthority = false;
    bool psxNoiseRegisterAuthority = false;
    bool psxSpuRegisterAuthority = false;
    bool dynamicReplayAuthority = false;
    bool oldWinS0Authority = false;
    bool stage2PlusAuthority = false;
};

struct SsDriverVoiceCompletionObservation80032B00 {
    uint8_t voiceCount = 0u;
    bool completionScanDisabled = false;
    std::array<uint16_t, 24> envelopeObservations{};
};

struct SsDriverVoiceCompletionState80032B00 {
    uint32_t completionRingIndex = 0u;
    std::array<uint32_t, 16> completionRing{};
    std::array<uint8_t, 24> voiceStatus{};
};

struct SsDriverVoiceCompletionDispatch80032B00 {
    bool known = false;
    uint32_t completionRingIndexBefore = 0u;
    uint32_t completionRingIndexAfter = 0u;
    uint32_t currentZeroEnvelopeMask = 0u;
    bool completionScanSkipped = false;
    uint32_t stableZeroEnvelopeMask = 0u;
    uint32_t clearedVoiceStatusMask = 0u;
    uint32_t noiseMaskClearCallCount = 0u;
    bool hostEnvelopeProjection = false;
    bool psxEnvelopeAuthority = false;
    bool psxNoiseRegisterAuthority = false;
    bool psxSpuRegisterAuthority = false;
};

enum class SsDriverVolumeRampKind80032B00 : uint8_t {
    First80031A28 = 0u,
    Second80031F28 = 1u,
};

struct SsDriverVolumeRampSurface80032B00 {
    bool known = false;
    bool twoRampFunctionsCompleteWithinLimits = false;
    uint32_t firstFunction = 0u;
    uint32_t secondFunction = 0u;
    uint32_t voiceStateBaseAddress = 0u;
    uint32_t voiceStateStride = 0u;
    uint32_t firstActiveOffset = 0u;
    uint32_t firstStepOffset = 0u;
    uint32_t firstPeriodOffset = 0u;
    uint32_t firstCountdownOffset = 0u;
    uint32_t firstCurrentOffset = 0u;
    uint32_t firstTargetOffset = 0u;
    uint32_t secondActiveOffset = 0u;
    uint32_t secondStepOffset = 0u;
    uint32_t secondPeriodOffset = 0u;
    uint32_t secondCountdownOffset = 0u;
    uint32_t secondCurrentOffset = 0u;
    uint32_t secondTargetOffset = 0u;
    uint32_t pendingVolumeLeftAddress = 0u;
    uint32_t pendingVolumeRightAddress = 0u;
    uint32_t dirtyFlagsAddress = 0u;
    uint32_t sharedScratchFirstAddress = 0u;
    uint32_t sharedScratchSecondAddress = 0u;
    bool psxVolumeAuthority = false;
    bool psxSpuRegisterAuthority = false;
};

struct SsDriverRampState80032B00 {
    int16_t active = 0;
    int16_t step = 0;
    int16_t period = 0;
    uint16_t countdown = 0u;
    uint16_t current = 0u;
    int16_t target = 0;
};

// 80031878 configures the first per-voice volume ramp.  80035110 is the
// bounded public wrapper: it rejects voice indices >= 24, while the lower
// routine preserves an equal start/target as a true no-op and otherwise
// writes the active/step/period/countdown/current/target fields used by
// 80031A28.  Division traps are reported explicitly and fail closed on the
// host instead of invoking the original PSX debugger break.
enum class SsDriverVolumeRampSetupBranch80035110 : uint8_t {
    Unknown = 0,
    WrapperVoiceReject,
    EqualNoOp,
    CoarseStep,
    FineStep,
    DivisionGuarded,
};

struct SsDriverVolumeRampSetupSurface80035110 {
    bool known = false;
    bool setupFunctionCompleteWithinLimits = false;
    uint32_t wrapperFunction = 0u;
    uint32_t setupFunction = 0u;
    uint32_t voiceStateBaseAddress = 0u;
    uint32_t voiceStateStride = 0u;
    uint32_t activeOffset = 0u;
    uint32_t stepOffset = 0u;
    uint32_t periodOffset = 0u;
    uint32_t countdownOffset = 0u;
    uint32_t currentOffset = 0u;
    uint32_t targetOffset = 0u;
    uint32_t maxVoiceCount = 0u;
    bool psxMemoryAuthority = false;
    bool psxSpuTimingAuthority = false;
};

struct SsDriverVolumeRampSetupDispatch80035110 {
    bool known = false;
    SsDriverVolumeRampSetupBranch80035110 branch =
        SsDriverVolumeRampSetupBranch80035110::Unknown;
    uint16_t voiceIndex = 0u;
    int16_t start = 0;
    int16_t target = 0;
    int16_t rate = 0;
    bool wrapperVoiceRejected = false;
    bool equalStartTargetNoOp = false;
    bool divisionGuarded = false;
    bool divisionOverflowGuarded = false;
    bool lowerCallCommitted = false;
    int16_t stepAfter = 0;
    int16_t periodAfter = 0;
    uint16_t countdownAfter = 0u;
    uint16_t currentAfter = 0u;
    int16_t targetAfter = 0;
    int16_t activeAfter = 0;
    uint32_t wrapperFunction = 0u;
    uint32_t setupFunction = 0u;
    bool psxMemoryAuthority = false;
    bool psxSpuTimingAuthority = false;
    bool oldWinS0Authority = false;
    bool stage2PlusAuthority = false;
};

enum class SsDriverDirectVolumeWriteKind80034E5C : uint8_t {
    OwnerIdentity80034E5C = 0,
    Direct80034F8C,
    Scaled12980035084,
};

// 80034E5C/80034F8C/80035084 are the driver's three bounded volume-write
// entry points.  The first requires the selected voice identity to match;
// the other two write the pending left/right words directly (the last after
// multiplying both inputs by 129).  All three set the original dirty bits and
// leave actual PSX SPU register ownership explicitly false.
struct SsDriverDirectVolumeWriteSurface80034E5C {
    bool known = false;
    bool threeWriteFunctionsCompleteWithinLimits = false;
    uint32_t ownerIdentityFunction = 0u;
    uint32_t directFunction = 0u;
    uint32_t scaledFunction = 0u;
    uint32_t voiceStateBaseAddress = 0u;
    uint32_t voiceStateStride = 0u;
    uint32_t sourceVabIdOffset = 0u;
    uint32_t programOffset = 0u;
    uint32_t noteOffset = 0u;
    uint32_t pendingVolumeLeftAddress = 0u;
    uint32_t pendingVolumeRightAddress = 0u;
    uint32_t dirtyFlagsAddress = 0u;
    uint8_t directDirtyMask = 0u;
    uint8_t identityDirtyMask = 0u;
    uint16_t scaledFactor = 0u;
    uint32_t maxVoiceCount = 0u;
    bool psxMemoryAuthority = false;
    bool psxSpuRegisterAuthority = false;
};

struct SsDriverDirectVolumeWriteDispatch80034E5C {
    bool known = false;
    SsDriverDirectVolumeWriteKind80034E5C kind =
        SsDriverDirectVolumeWriteKind80034E5C::OwnerIdentity80034E5C;
    uint16_t voiceIndex = 0u;
    int16_t sourceVabId = -1;
    int16_t program = 0;
    int16_t note = 0;
    int16_t leftRequested = 0;
    int16_t rightRequested = 0;
    bool wrapperVoiceRejected = false;
    bool ownerIdentityMatched = false;
    bool writeCommitted = false;
    uint16_t pendingVolumeLeftAfter = 0u;
    uint16_t pendingVolumeRightAfter = 0u;
    uint8_t dirtyFlagsAfter = 0u;
    uint32_t function = 0u;
    uint32_t ownerIdentityFunction = 0u;
    uint32_t directFunction = 0u;
    uint32_t scaledFunction = 0u;
    uint16_t scaledFactor = 0u;
    int32_t scaledLeft = 0;
    int32_t scaledRight = 0;
    int32_t result = -1;
    bool psxMemoryAuthority = false;
    bool psxSpuRegisterAuthority = false;
    bool oldWinS0Authority = false;
    bool stage2PlusAuthority = false;
};

struct SsDriverVoiceRegisterState80032B00 {
    uint8_t dirtyFlags = 0u;
    uint16_t pendingVolumeLeft = 0u;
    uint16_t pendingVolumeRight = 0u;
    uint16_t pendingPitch = 0u;
    uint16_t pendingStartAddress = 0u;
    uint16_t pendingAdsr1 = 0u;
    uint16_t pendingAdsr2 = 0u;
    SsDriverRampState80032B00 firstRamp{};
    SsDriverRampState80032B00 secondRamp{};
};

struct SsDriverVolumeMix80032B00 {
    uint8_t masterVolume = 0u;
    uint8_t mix800928E2 = 0u;
    uint8_t pan800928E3 = 0u;
    uint8_t mix800928E5 = 0u;
    uint8_t pan800928E6 = 0u;
    int16_t monoFlag80091728 = 0;
};

struct SsDriverVolumeScratch80032B00 {
    uint8_t firstCurrent800928DC = 0u;
    uint8_t secondCurrent800928DD = 0u;
};

struct SsDriverVolumeRampDispatch80032B00 {
    bool known = false;
    SsDriverVolumeRampKind80032B00 kind =
        SsDriverVolumeRampKind80032B00::First80031A28;
    bool countdownSkippedUpdate = false;
    bool currentAdvanced = false;
    bool targetReached = false;
    uint16_t currentAfter = 0u;
    uint16_t pendingVolumeLeft = 0u;
    uint16_t pendingVolumeRight = 0u;
    uint8_t dirtyFlagsAfter = 0u;
    bool hostVolumeProjectionReady = false;
    bool psxVolumeAuthority = false;
    bool psxSpuRegisterAuthority = false;
};

struct SsDriverRegisterCommitSurface80032B00 {
    bool known = false;
    bool dirtyRegisterAndMaskControlCompleteWithinLimits = false;
    uint32_t function = 0u;
    uint32_t dirtyLoopStart = 0u;
    uint32_t dirtyLoopEnd = 0u;
    uint32_t voiceCount = 0u;
    uint32_t voiceRegisterStride = 0u;
    uint8_t volumeDirtyMask = 0u;
    uint8_t pitchDirtyMask = 0u;
    uint8_t startAddressDirtyMask = 0u;
    uint8_t adsrDirtyMask = 0u;
    uint32_t keyOffLowAddress = 0u;
    uint32_t keyOffHighAddress = 0u;
    uint32_t keyOnLowAddress = 0u;
    uint32_t keyOnHighAddress = 0u;
    uint32_t reverbLowAddress = 0u;
    uint32_t reverbHighAddress = 0u;
    uint32_t spuKeyOffLowOffset = 0u;
    uint32_t spuKeyOffHighOffset = 0u;
    uint32_t spuKeyOnLowOffset = 0u;
    uint32_t spuKeyOnHighOffset = 0u;
    uint32_t spuReverbLowOffset = 0u;
    uint32_t spuReverbHighOffset = 0u;
    bool psxSpuRegisterAuthority = false;
};

struct SsDriverGlobalMaskState80032B00 {
    uint16_t keyOffLow = 0u;
    uint16_t keyOffHigh = 0u;
    uint16_t keyOnLow = 0u;
    uint16_t keyOnHigh = 0u;
    uint16_t reverbLow = 0u;
    uint16_t reverbHigh = 0u;
};

struct SsDriverInitializeSurface8003226C {
    bool known = false;
    bool driverStateProducerCompleteWithinLimits = false;
    uint32_t function = 0u;
    uint32_t callerFunction = 0u;
    uint32_t callerCall = 0u;
    uint32_t callerRequestedVoiceCount = 0u;
    uint32_t driverControlFunction = 0u;
    uint32_t driverControlCall = 0u;
    uint32_t driverControlAddress = 0u;
    uint32_t spuInitMallocFunction = 0u;
    uint32_t spuInitMallocCall = 0u;
    uint32_t spuInitMallocRecordCount = 0u;
    uint32_t spuInitMallocTableAddress = 0u;
    uint32_t voiceCountAddress = 0u;
    uint32_t voiceStateBaseAddress = 0u;
    uint32_t voiceStateStride = 0u;
    uint32_t pendingVoiceRegisterBaseAddress = 0u;
    uint32_t pendingVoiceRegisterStride = 0u;
    uint32_t dirtyFlagsAddress = 0u;
    uint32_t completionScratchBaseAddress = 0u;
    uint32_t completionScratchByteCount = 0u;
    uint32_t lastVoiceIndexAddress = 0u;
    uint32_t masterVolumeLeftAddress = 0u;
    uint32_t masterVolumeRightAddress = 0u;
    uint32_t completionScanDisableAddress = 0u;
    uint32_t monoFlagAddress = 0u;
    uint32_t driverScalarAddress = 0u;
    uint32_t driverCommitFunction = 0u;
    uint32_t driverCommitCall = 0u;
    uint32_t maxVoiceCount = 0u;
    uint16_t defaultPitch = 0u;
    uint16_t defaultStartAddress = 0u;
    uint16_t defaultAdsr1 = 0u;
    uint16_t defaultAdsr2 = 0u;
    uint16_t defaultMasterVolume = 0u;
    uint16_t defaultDriverScalar = 0u;
    bool psxSpuAllocatorAuthority = false;
    bool psxSpuRegisterAuthority = false;
    bool dynamicReplayAuthority = false;
    bool oldWinS0Authority = false;
    bool stage2PlusAuthority = false;
};

struct SsDriverVoiceOwnerState8003226C {
    uint16_t field80087D40 = 0u;
    uint16_t field80087D42 = 0u;
    uint16_t field80087D44 = 0u;
    uint16_t field80087D46 = 0u;
    uint16_t velocity80087D48 = 0u;
    uint8_t pan80087D4A = 0u;
    uint16_t note80087D4C = 0u;
    int16_t packedSequenceTrack80087D4E = 0;
    uint16_t toneTableProgram80087D50 = 0u;
    uint16_t program80087D52 = 0u;
    uint16_t toneIndex80087D54 = 0u;
    uint16_t sourceVabId80087D56 = 0u;
    uint16_t priority80087D58 = 0u;
};

struct SsDriverInitializeState8003226C {
    int32_t driverControl800555F8 = 0;
    uint32_t spuMallocRecordCount = 0u;
    uint32_t spuMallocTableAddress = 0u;
    uint16_t word800928C8 = 0u;
    uint16_t word80091688 = 0u;
    uint16_t word801C35F0 = 0u;
    std::array<uint8_t, 16> completionScratch800928F8{};
    uint8_t configuredVoiceCount800928A0 = 0u;
    uint16_t lastVoiceIndex800928F2 = 0u;
    // 80030C90 publishes these two transient selectors before touching the
    // pending per-voice register bank: 800928F4 is 8*voice and 800928F6 is
    // the dense tone-table index (toneSlot + 16*toneTableProgram).
    uint16_t word800928F4 = 0u;
    uint16_t word800928F6 = 0u;
    uint16_t masterVolumeLeft8008ECC8 = 0u;
    uint16_t masterVolumeRight8008ECCA = 0u;
    bool completionScanDisabled8009290C = false;
    uint16_t driverScalar800917A8 = 0u;
};

struct SsDriverInitializeDispatch8003226C {
    bool known = false;
    uint8_t requestedVoiceCount = 0u;
    uint8_t configuredVoiceCount = 0u;
    uint32_t clearedPendingRegisterVoiceMask = 0u;
    uint32_t clearedDirtyVoiceMask = 0u;
    uint32_t initializedVoiceMask = 0u;
    bool driverControlExecuted = false;
    bool spuInitMallocProjected = false;
    bool driverCommitExecuted = false;
    int32_t driverCommitResult = 0;
    bool hostProjection = false;
    bool psxSpuAllocatorAuthority = false;
    bool psxSpuRegisterAuthority = false;
};

struct SsDriverResetSurface800351B8 {
    bool known = false;
    bool resetStateProducerCompleteWithinLimits = false;
    uint32_t wrapperFunction = 0u;
    uint32_t wrapperCall = 0u;
    uint32_t resetFunction = 0u;
    uint32_t voiceCountAddress = 0u;
    uint32_t voiceStateBaseAddress = 0u;
    uint32_t voiceStateStride = 0u;
    uint32_t completionStatusOffset = 0u;
    uint32_t pendingVoiceRegisterBaseAddress = 0u;
    uint32_t pendingVoiceRegisterStride = 0u;
    uint32_t lastVoiceIndexAddress = 0u;
    uint32_t keyOffLowAddress = 0u;
    uint32_t keyOffHighAddress = 0u;
    uint32_t keyOnLowAddress = 0u;
    uint32_t keyOnHighAddress = 0u;
    uint32_t maxVoiceCount = 0u;
    uint16_t defaultOwnerKind = 0u;
    uint16_t defaultOwnerSentinel = 0u;
    uint16_t defaultPitch = 0u;
    uint16_t defaultStartAddress = 0u;
    uint16_t defaultAdsr1 = 0u;
    uint16_t defaultAdsr2 = 0u;
    bool callsDriverCommit = false;
    bool psxSpuRegisterAuthority = false;
    bool psxInterruptTimingAuthority = false;
    bool dynamicReplayAuthority = false;
    bool oldWinS0Authority = false;
    bool stage2PlusAuthority = false;
};

struct SsDriverResetDispatch800351B8 {
    bool known = false;
    uint8_t returnedVoiceCount = 0u;
    bool zeroVoiceCountNoOp = false;
    uint32_t resetVoiceMask = 0u;
    uint16_t keyOffLowAfter = 0u;
    uint16_t keyOffHighAfter = 0u;
    uint16_t keyOnLowAfter = 0u;
    uint16_t keyOnHighAfter = 0u;
    bool pendingRegistersReinitialized = false;
    bool dirtyFlagsPreserved = false;
    bool rampStatePreserved = false;
    bool untouchedOwnerFieldsPreserved = false;
    bool reverbMasksPreserved = false;
    bool driverCommitExecuted = false;
    bool hostVoiceResetProjectionReady = false;
    bool psxSpuRegisterAuthority = false;
    bool psxInterruptTimingAuthority = false;
};

struct SsSpuFirstAllocationSurface8002E87C {
    bool known = false;
    bool currentScene0FirstAllocationCompleteWithinLimits = false;
    uint32_t scene0CallerFunction = 0u;
    uint32_t vabOpenWrapperFunction = 0u;
    uint32_t vabLoaderFunction = 0u;
    uint32_t spuMallocFunction = 0u;
    uint32_t allocatorNormalizeFunction = 0u;
    uint32_t spuInitMallocFunction = 0u;
    uint32_t recordTableAddress = 0u;
    uint32_t recordCount = 0u;
    uint32_t initialFreeAddress = 0u;
    uint32_t spuRamEndAddress = 0u;
    uint32_t initialFreeFlag = 0u;
    uint32_t addressMask = 0u;
    uint32_t alignmentMask = 0u;
    uint32_t alignmentShift = 0u;
    uint32_t activeReserveBytes = 0u;
    bool fullSpuAllocatorAuthority = false;
    bool psxSpuRamTransferAuthority = false;
    bool dynamicReplayAuthority = false;
    bool oldWinS0Authority = false;
    bool stage2PlusAuthority = false;
};

struct SsSpuAllocationRecord8002E87C {
    uint32_t addressAndFlags = 0u;
    uint32_t size = 0u;
};

struct SsSpuFirstAllocationState8002E87C {
    bool initialized = false;
    uint32_t recordTableAddress = 0u;
    uint32_t recordCount = 0u;
    uint32_t highWaterRecordIndex = 0u;
    uint32_t allocationCount = 0u;
    std::array<SsSpuAllocationRecord8002E87C, 32> records{};
};

struct SsSpuFirstAllocationDispatch8002E87C {
    bool known = false;
    bool initialized = false;
    bool allocationAttempted = false;
    bool allocationSucceeded = false;
    uint32_t requestedBytes = 0u;
    uint32_t alignedBytes = 0u;
    uint32_t allocatedAddress = 0u;
    uint32_t remainingFreeAddress = 0u;
    uint32_t remainingFreeBytes = 0u;
    bool sampleStartProductionReady = false;
    bool fullSpuAllocatorAuthority = false;
    bool psxSpuRamTransferAuthority = false;
};

struct SsVabBodyTransferSurface8002EB80 {
    bool known = false;
    bool firstScene0VabTransferCompleteWithinLimits = false;
    bool completionControlCompleteWithinLimits = false;
    uint32_t initializeFunction = 0u;
    uint32_t openFunction = 0u;
    uint32_t transferWrapperFunction = 0u;
    uint32_t transferFunction = 0u;
    uint32_t setTransferModeFunction = 0u;
    uint32_t setTransferStartFunction = 0u;
    uint32_t writeFunction = 0u;
    uint32_t transferReadyFunction = 0u;
    uint32_t completionWrapperFunction = 0u;
    uint32_t completionFunction = 0u;
    uint32_t testEventFunction = 0u;
    uint32_t vabStatusAddress = 0u;
    uint32_t vabSpuBaseAddress = 0u;
    uint32_t vabTransferBytesAddress = 0u;
    uint32_t transferStartAddress = 0u;
    uint32_t transferReadyAddress = 0u;
    uint32_t transferModeAddress = 0u;
    uint32_t transferPathAddress = 0u;
    uint32_t initializedVabSlotCount = 0u;
    uint32_t transferAcceptedStatus = 0u;
    uint32_t transferPendingStatus = 0u;
    uint32_t transferAlignmentBytes = 0u;
    uint32_t transferAddressShift = 0u;
    uint32_t maximumTransferBytes = 0u;
    uint32_t spuRamEndAddress = 0u;
    bool psxDmaAuthority = false;
    bool psxEventAuthority = false;
    bool psxSpuRamAuthority = false;
    bool dynamicReplayAuthority = false;
    bool oldWinS0Authority = false;
    bool stage2PlusAuthority = false;
};

struct SsVabBodyTransferState8002EB80 {
    std::array<uint8_t, 17> vabStatus800928F8{};
    std::array<uint32_t, 17> vabSpuBase801C35F8{};
    std::array<uint32_t, 17> vabTransferBytes801C35B0{};
    uint16_t transferStart800555C4 = 0u;
    bool transferReady800555F8 = true;
    uint32_t transferMode80055624 = 0u;
    bool alternateTransferPath800555E0 = false;
};

struct SsVabBodyTransferDispatch8002EB80 {
    bool known = false;
    bool accepted = false;
    int32_t result = -1;
    uint16_t vabId = 0u;
    uint8_t statusBefore = 0u;
    uint8_t statusAfter = 0u;
    uint32_t sourceBytesAvailable = 0u;
    uint32_t requestedTransferBytes = 0u;
    uint32_t committedTransferBytes = 0u;
    uint32_t spuBaseBeforeAlignment = 0u;
    uint32_t spuBaseAfterAlignment = 0u;
    uint16_t transferStartUnits = 0u;
    bool sourceRangeAvailable = false;
    bool hostPayloadProjectionReady = false;
    bool psxDmaCommitted = false;
    bool psxDmaAuthority = false;
    bool psxSpuRamAuthority = false;
};

struct SsVabTransferCompletionDispatch8002EF28 {
    bool known = false;
    bool waitRequested = false;
    bool eventObserved = false;
    bool testEventRequested = false;
    bool wouldBlock = false;
    bool completionLatched = false;
    int32_t result = 0;
    bool psxEventAuthority = false;
    bool psxDmaAuthority = false;
};

struct SsDriverVoiceAllocationSurface80032EAC {
    bool known = false;
    bool allocationAndRegisterProducerCompleteWithinKnownInputs = false;
    bool noteOffStateProducerCompleteWithinLimits = false;
    uint32_t function = 0u;
    uint32_t validationFunction = 0u;
    uint32_t allocatorFunction = 0u;
    uint32_t prepareFunction = 0u;
    uint32_t noiseStartFunction = 0u;
    uint32_t pitchFunction = 0u;
    uint32_t regularStartFunction = 0u;
    uint32_t noteOffFunction = 0u;
    uint32_t pitchTableAddress = 0u;
    uint32_t pitchTableEntryCount = 0u;
    uint32_t maxVoiceCount = 0u;
    uint32_t maxToneLayers = 0u;
    uint32_t voiceStateBaseAddress = 0u;
    uint32_t voiceStateStride = 0u;
    uint32_t pendingVoiceRegisterBaseAddress = 0u;
    uint32_t pendingVoiceRegisterStride = 0u;
    uint32_t completionRingBaseAddress = 0u;
    uint32_t completionRingEntryCount = 0u;
    bool psxSampleStartAddressAuthority = false;
    bool psxNoiseRegisterAuthority = false;
    bool psxSpuRegisterAuthority = false;
    bool psxSpuTimingAuthority = false;
    bool dynamicReplayAuthority = false;
    bool oldWinS0Authority = false;
    bool stage2PlusAuthority = false;
};

struct SsDriverProgramAttributes80032EAC {
    bool valid = false;
    uint8_t toneCount = 0u;
    uint8_t volume = 0u;
    uint8_t priority = 0u;
    uint8_t mode = 0u;
    uint8_t pan = 0u;
    uint8_t toneTableProgram = 0u;
};

struct SsDriverToneAttributes80032EAC {
    bool valid = false;
    uint8_t toneSlot = 0u;
    uint16_t toneIndex = 0u;
    uint8_t priority = 0u;
    uint8_t mode = 0u;
    uint8_t volume = 0u;
    uint8_t pan = 0u;
    uint8_t centerNote = 0u;
    uint8_t centerFine = 0u;
    uint8_t noteMin = 0u;
    uint8_t noteMax = 0u;
    uint16_t adsr1 = 0u;
    uint16_t adsr2 = 0u;
    uint16_t sampleId = 0u;
    bool sampleStartAddressKnown = false;
    uint16_t sampleStartAddress = 0u;
};

struct SsCompactSfxSurface80034240 {
    bool known = false;
    bool current80026EF8RouteCompleteWithinLimits = false;
    uint32_t wrapperFunction = 0u;
    uint32_t function = 0u;
    uint32_t validationFunction = 0u;
    uint32_t allocatorFunction = 0u;
    uint32_t allocatorNoiseMaskClearFunction = 0u;
    uint32_t allocatorNoiseMaskClearValue = 0u;
    uint32_t prepareFunction = 0u;
    uint32_t noiseStartFunction = 0u;
    uint32_t pitchFunction = 0u;
    uint32_t regularStartFunction = 0u;
    uint32_t sourceVabIdAddress = 0u;
    uint32_t resultVoiceAddress = 0u;
    uint32_t reentryGuardAddress = 0u;
    uint32_t programCountAddress = 0u;
    uint32_t vabStatusAddress = 0u;
    uint32_t programAttributeTableAddress = 0u;
    uint32_t toneAttributeTableAddress = 0u;
    uint32_t voiceStateBaseAddress = 0u;
    uint32_t voiceStateStride = 0u;
    uint32_t cuePitchAdd = 0u;
    uint32_t acceptedVabSlotCount = 0u;
    uint32_t maximumProgramCount = 0u;
    uint32_t maximumToneSlotCount = 0u;
    uint32_t compactOwnerTag = 0u;
    bool equalVolumeCurrentWrapperOnly = false;
    bool fineAdjustZeroCurrentWrapperOnly = false;
    bool psxVoiceIdentityAuthority = false;
    bool psxNoiseRegisterAuthority = false;
    bool psxSpuRegisterAuthority = false;
    bool psxSpuTimingAuthority = false;
    bool dynamicReplayAuthority = false;
    bool oldWinS0Authority = false;
    bool stage2PlusAuthority = false;
};

struct SsCompactSfxRequest80034240 {
    bool bankReady = false;
    int16_t sourceVabId = -1;
    std::array<uint8_t, 4> command{};
    uint8_t vabMasterVolume = 0u;
    SsDriverProgramAttributes80032EAC programAttributes{};
    SsDriverToneAttributes80032EAC tone{};
};

struct SsCompactSfxDispatch80034240 {
    bool known = false;
    bool commandMutated80026EF8 = false;
    std::array<uint8_t, 4> commandBefore{};
    std::array<uint8_t, 4> commandAfter{};
    bool reentrySkipped = false;
    bool reentryGuardSet = false;
    bool reentryGuardCleared = false;
    bool validationRejected = false;
    bool allocationFailed = false;
    bool replacedActiveVoice = false;
    bool replacedNoiseVoice = false;
    uint32_t allocatorNoiseMaskClearCallCount = 0u;
    uint32_t allocatorNoiseMaskClearValue = 0u;
    uint8_t selectedVoice = 0xFFu;
    uint16_t word800928F4 = 0u;
    uint16_t word800928F6 = 0u;
    // Transient scratch committed by 80034240 before it enters the selected
    // tone/voice lower path.  These are software state values, not host
    // pointers; retaining them makes the compact SFX side effects auditable.
    uint8_t byte800928DA = 0u;
    uint8_t byte800928DB = 0u;
    uint8_t byte800928D8 = 0u;
    uint8_t byte800928DF = 0u;
    uint8_t byte800928E4 = 0u;
    uint8_t byte800928E5 = 0u;
    uint8_t byte800928E6 = 0u;
    uint8_t byte800928E7 = 0u;
    uint8_t byte800928E8 = 0u;
    uint8_t byte800928E9 = 0u;
    uint8_t byte800928EA = 0u;
    uint8_t byte800928EB = 0u;
    uint8_t byte800928EC = 0u;
    uint16_t word800928F0 = 0u;
    bool transientScratchCommitted = false;
    uint16_t sampleId = 0u;
    bool noiseSample = false;
    uint16_t pitch = 0u;
    uint16_t pendingVolumeLeft = 0u;
    uint16_t pendingVolumeRight = 0u;
    uint8_t dirtyFlagsAfter = 0u;
    bool sampleStartAddressKnown = false;
    int32_t result = -1;
    bool hostPlaybackReady = false;
    bool psxVoiceIdentityAuthority = false;
    bool psxNoiseRegisterAuthority = false;
    bool psxSpuRegisterAuthority = false;
    bool psxSpuTimingAuthority = false;
};

struct SsDriverNoteOnRequest80032EAC {
    bool bankReady = false;
    uint16_t packedSequenceTrack = 0u;
    int16_t sourceVabId = -1;
    uint8_t program = 0u;
    uint8_t note = 0u;
    uint16_t velocity = 0u;
    uint16_t channelVolume = 0u;
    uint8_t channelPan = 0u;
    uint16_t trackVolumeLeft = 0u;
    uint16_t trackVolumeRight = 0u;
    uint8_t vabMasterVolume = 0u;
    SsDriverProgramAttributes80032EAC programAttributes{};
    std::array<SsDriverToneAttributes80032EAC, 16> tones{};
    uint8_t matchedToneCount = 0u;
};

struct SsDriverToneLayerDispatch80032EAC {
    bool matched = false;
    uint8_t toneSlot = 0u;
    uint16_t toneIndex = 0u;
    uint8_t selectedVoice = 0xFFu;
    bool allocationFailed = false;
    bool replacedActiveVoice = false;
    bool replacedNoiseVoice = false;
    uint16_t sampleId = 0u;
    bool noiseSample = false;
    uint16_t effectiveVelocity = 0u;
    uint16_t pitch = 0u;
    uint16_t pendingVolumeLeft = 0u;
    uint16_t pendingVolumeRight = 0u;
    uint8_t dirtyFlagsAfter = 0u;
    bool sampleStartAddressKnown = false;
    bool hostPlaybackReady = false;
};

struct SsDriverNoteOnDispatch80032EAC {
    bool known = false;
    bool validationRejected = false;
    uint8_t configuredVoiceCount = 0u;
    uint8_t matchedToneCount = 0u;
    uint8_t allocatedToneCount = 0u;
    uint32_t allocatedVoiceMask = 0u;
    uint32_t replacedVoiceMask = 0u;
    uint32_t noiseMaskClearRequestCount = 0u;
    int32_t result = -1;
    std::array<SsDriverToneLayerDispatch80032EAC, 16> layers{};
    bool hostProjectionReady = false;
    bool psxSampleStartAddressAuthority = false;
    bool psxNoiseRegisterAuthority = false;
    bool psxSpuRegisterAuthority = false;
    bool psxSpuTimingAuthority = false;
};

struct SsDriverNoteOffDispatch8003349C {
    bool known = false;
    uint32_t matchedVoiceCount = 0u;
    uint32_t matchedVoiceMask = 0u;
    uint32_t regularKeyOffVoiceMask = 0u;
    uint32_t noiseRegisterClearRequestCount = 0u;
    bool hostProjectionReady = false;
    bool psxNoiseRegisterAuthority = false;
    bool psxSpuRegisterAuthority = false;
};

struct SsDriverVoiceRegisterRequest80032B00 {
    bool writeVolume = false;
    bool writePitch = false;
    bool writeStartAddress = false;
    bool writeAdsr = false;
    uint16_t volumeLeft = 0u;
    uint16_t volumeRight = 0u;
    uint16_t pitch = 0u;
    uint16_t startAddress = 0u;
    uint16_t adsr1 = 0u;
    uint16_t adsr2 = 0u;
};

struct SsDriverRegisterCommitDispatch80032B00 {
    bool known = false;
    std::array<SsDriverVoiceRegisterRequest80032B00, 24>
        voiceRequests{};
    uint32_t dirtyVoiceMask = 0u;
    uint16_t keyOffLow = 0u;
    uint16_t keyOffHigh = 0u;
    uint16_t keyOnLow = 0u;
    uint16_t keyOnHigh = 0u;
    uint16_t reverbLow = 0u;
    uint16_t reverbHigh = 0u;
    bool pendingKeyOnMaskedByKeyOff = false;
    bool pendingKeyOnAndKeyOffCleared = false;
    bool reverbMasksRetained = false;
    bool hostVolumeProjectionReady = false;
    bool psxSpuRegisterAuthority = false;
};

struct VabActionRequest8001A8F0 {
    uint32_t order = 0u;
    uint32_t wrapperFunction = 0u;
    uint32_t lowerFunction = 0u;
    uint32_t pointerArg = 0u;
    int32_t scalarArg = 0;
    bool lowerCallConditional = false;
    bool lowerResultAuthority = false;
};

struct Transaction8001A8F0 {
    Status8001A8F0 status = Status8001A8F0::SourceUnknown;
    bool accepted = false;
    bool complete = false;
    bool sourceIntLoadKnown = false;
    bool sourceLoaderCandidateKnown = false;
    PrSS0Scene0IntLoadDirect::ArchiveKind8001AC18 sourceArchiveKind =
        PrSS0Scene0IntLoadDirect::ArchiveKind8001AC18::Unknown;
    bool requestBound = false;
    bool originalDiscPayloadAuthority = false;

    uint64_t vhArchiveDataOffset = 0u;
    uint32_t vhPsxAddress = 0u;
    std::array<uint8_t, 16> vhName{};
    std::vector<uint8_t> vhBytes{};
    uint64_t vhHash = 0u;

    uint64_t vbArchiveDataOffset = 0u;
    uint32_t vbPsxAddress = 0u;
    std::array<uint8_t, 16> vbName{};
    std::vector<uint8_t> vbBytes{};
    uint64_t vbHash = 0u;

    uint16_t decoderProgramCount = 0u;
    uint16_t decoderDeclaredToneCount = 0u;
    uint16_t decoderVagCount = 0u;
    uint16_t decoderEffectiveVagCount = 0u;
    uint32_t decoderToneSlotCount = 0u;
    uint32_t decoderVagTableOffset = 0u;
    bool decoderPreflightKnown = false;
    bool decoderPreflightReady = false;

    std::array<VabActionRequest8001A8F0,
               kVabActionCount8001A8F0>
        actionRequests{};
    bool actionSequenceKnown = false;
    bool padStartComRequired = false;

    bool padStartComSsSequenceCallbackSurfaceKnown = false;
    SsSequenceCallbackSurface8002AC20
        padStartComSsSequenceCallbackSurface{};
    bool padStartComSpuTransferCompletionSurfaceKnown = false;
    SpuTransferCompletionSurface8002AC20
        padStartComSpuTransferCompletionSurface{};
    bool padStartComSsTickInterruptSurfaceKnown = false;
    SsTickInterruptSurface8002B130
        padStartComSsTickInterruptSurface{};
    std::array<PadStartComCall80026E4C,
               kPadStartComCallCount80026E4C>
        padStartComCalls{};
    bool padStartComStaticBodyKnown = false;
    bool padStartComGlobalWritesKnown = false;
    PrStage1LoaderSpuHal::State padStartComGlobalState{};
    uint32_t padStartComCuePointerBefore = 0u;
    uint32_t padStartComCuePointerAfter = 0u;
    bool padStartComCuePointerPreserved = false;
    bool padStartComGlobalWritesCommitted = false;
    bool padStartComLowerCallsCommitted = false;

    bool hostVabCandidatePrepared = false;
    bool hostVabBankCommitted = false;
    bool psxVabIdAuthority = false;
    bool psxSpuRamAuthority = false;
    bool padStartComCommitted = false;
    bool audibleOutputAuthority = false;
    bool replayValueAuthority = false;
    bool hostFilesystemAuthority = false;
    bool consumerReadAuthority = false;
    bool oldWinS0Authority = false;
    bool stage2PlusAuthority = false;
};

SsSequenceCallbackSurface8002AC20
BuildSsSequenceCallbackSurface8002AC20();

bool IsExactSsSequenceCallbackSurface8002AC20(
    const SsSequenceCallbackSurface8002AC20& surface);

bool TryResolveSsSequenceCallback8002C808(
    const SsSequenceCallbackSurface8002AC20& surface,
    int16_t sequenceIndex,
    int16_t trackIndex,
    uint32_t& outCallbackTarget);

SpuTransferCompletionSurface8002AC20
BuildSpuTransferCompletionSurface8002AC20();

bool IsExactSpuTransferCompletionSurface8002AC20(
    const SpuTransferCompletionSurface8002AC20& surface);

bool TryBuildSpuTransferCompletionDispatch80029E6C(
    const SpuTransferCompletionSurface8002AC20& surface,
    uint32_t callbackTarget,
    SpuTransferCompletionDispatch80029E6C& out);

SsTickInterruptSurface8002B130
BuildSsTickInterruptSurface8002B130();

bool IsExactSsTickInterruptSurface8002B130(
    const SsTickInterruptSurface8002B130& surface);

bool TryBuildSsTickPriorWrapperDispatch8002B170(
    const SsTickInterruptSurface8002B130& surface,
    uint32_t priorHandlerTarget,
    SsTickInterruptDispatch8002B170& out);

bool TryBuildSsTickDividerDispatch8002B1B0(
    const SsTickInterruptSurface8002B130& surface,
    bool dividerGuardSet,
    SsTickInterruptDispatch8002B170& out);

bool TryBuildSsTickTrackDispatch8002B200(
    const SsTickInterruptSurface8002B130& surface,
    const SsTickTrackObservation8002B200& observation,
    SsTickTrackDispatch8002B200& out);

SsMidiParserSurface8002BA9C
BuildSsMidiParserSurface8002BA9C();

bool IsExactSsMidiParserSurface8002BA9C(
    const SsMidiParserSurface8002BA9C& surface);

bool TryDecodeSsMidiDelta8002D7D0(
    const std::vector<uint8_t>& streamBytes,
    std::size_t& cursor,
    uint32_t& absoluteTime,
    uint32_t& outDeltaTicks10);

bool TryBuildSsMidiEventDispatch8002BC40(
    const SsMidiParserSurface8002BA9C& surface,
    const SsMidiTrackObservation8002BC40& observation,
    SsMidiEventDispatch8002BC40& out);

bool TryBuildSsMidiSchedulerDispatch8002BB30(
    const SsMidiParserSurface8002BA9C& surface,
    const SsMidiSchedulerObservation8002BB30& observation,
    SsMidiSchedulerDispatch8002BB30& out);

bool TryBuildSsMidiProgramChangeDispatch8002BFDC(
    const SsMidiParserSurface8002BA9C& surface,
    const SsMidiProgramChangeObservation8002BFDC& observation,
    SsMidiProgramChangeDispatch8002BFDC& out);

bool TryBuildSsMidiControlChangeDispatch8002C054(
    const SsMidiParserSurface8002BA9C& surface,
    const SsMidiControlChangeObservation8002C054& observation,
    SsMidiControlChangeDispatch8002C054& out);

bool TryBuildSsMidiPitchBendDispatch8002D3D0(
    const SsMidiParserSurface8002BA9C& surface,
    const SsMidiPitchBendObservation8002D3D0& observation,
    SsMidiPitchBendDispatch8002D3D0& out);

bool TryBuildSsMidiMetaDispatch8002D47C(
    const SsMidiParserSurface8002BA9C& surface,
    const SsMidiMetaObservation8002D47C& observation,
    SsMidiMetaDispatch8002D47C& out);

bool TryBuildSsMidiCallbackDispatch8002C808(
    const SsMidiParserSurface8002BA9C& surface,
    const SsMidiCallbackObservation8002C808& observation,
    SsMidiCallbackDispatch8002C808& out);

bool TryBuildSsMidiCallbackGateDispatch8002C938(
    const SsMidiParserSurface8002BA9C& surface,
    const SsMidiCallbackGateObservation8002C938& observation,
    SsMidiCallbackGateDispatch8002C938& out);

bool TryBuildSsMidiTrackByteCounterDispatch8002CA7C(
    const SsMidiParserSurface8002BA9C& surface,
    const SsMidiTrackByteCounterObservation8002CA7C& observation,
    SsMidiTrackByteCounterDispatch8002CA7C& out);

bool TryBuildSsMidiTrackByteCounterDispatch8002CAF4(
    const SsMidiParserSurface8002BA9C& surface,
    const SsMidiTrackByteCounterObservation8002CAF4& observation,
    SsMidiTrackByteCounterDispatch8002CAF4& out);

bool TryBuildSsMidiControllerResetDispatch8002C740(
    const SsMidiParserSurface8002BA9C& surface,
    const SsMidiControllerResetObservation8002C740& observation,
    SsMidiControllerResetDispatch8002C740& out);

bool TryBuildSsMidiController65Dispatch8002C5DC(
    const SsMidiParserSurface8002BA9C& surface,
    const SsMidiController65Observation8002C5DC& observation,
    SsMidiController65Dispatch8002C5DC& out);

bool TryBuildSsMidiController6Dispatch8002CB6C(
    const SsMidiParserSurface8002BA9C& surface,
    const SsMidiController6Observation8002CB6C& observation,
    SsMidiController6Dispatch8002CB6C& out);

// The 8002B474/8002B750 flag routes are per-track volume-envelope ticks.
// They share the 0x94/0x98 counter pair, signed +0x42 step and +0x40 current
// value, but their ramp direction and terminal write differ.  The helper
// calls (800337B4/80033928) are recorded as symbolic surfaces; this owner
// does not invoke a host audio or unknown PSX callback.
enum class SsMidiEnvelopeBranch8002B474 : uint8_t {
    Unknown = 0,
    ZeroStepCopy,
    PositiveModuloSkip,
    PositiveRamp,
    PositiveReset,
    NegativeRamp,
    NegativeReset,
};

struct SsMidiEnvelopeObservation8002B474 {
    bool known = false;
    int16_t sequenceIndex = -1;
    int16_t trackIndex = -1;
    int16_t wordTrackOffset3E = 0;
    int16_t wordTrackOffset40 = 0;
    int16_t wordTrackOffset42 = 0;
    uint32_t dwordTrackOffset94 = 0u;
    uint32_t dwordTrackOffset98 = 0u;
    uint32_t dwordTrackOffset90Flags = 0u;
    uint16_t wordTrackOffset74 = 0u;
    uint16_t wordTrackOffset76 = 0u;
    uint16_t wordTrackOffset78 = 0u;
    uint16_t wordTrackOffset7A = 0u;
};

struct SsMidiEnvelopeDispatch8002B474 {
    bool known = false;
    SsMidiEnvelopeBranch8002B474 branch =
        SsMidiEnvelopeBranch8002B474::Unknown;
    int16_t sequenceIndex = -1;
    int16_t trackIndex = -1;
    int16_t wordTrackOffset40After = 0;
    uint32_t dwordTrackOffset98After = 0u;
    uint32_t dwordTrackOffset90FlagsAfter = 0u;
    uint16_t wordTrackOffset74After = 0u;
    uint16_t wordTrackOffset76After = 0u;
    uint16_t wordTrackOffset78After = 0u;
    uint16_t wordTrackOffset7AAfter = 0u;
    uint16_t packedSequenceTrack = 0u;
    uint32_t helperReadFunction = 0u;
    uint32_t helperWriteFunction = 0u;
    bool helperReadObserved = false;
    uint32_t helperReadCallCount = 0u;
    bool helperWriteObserved = false;
    uint32_t helperWriteCallCount = 0u;
    int16_t helperWriteLeftRequested = 0;
    int16_t helperWriteRightRequested = 0;
    uint16_t helperWriteLeftApplied = 0u;
    uint16_t helperWriteRightApplied = 0u;
    bool flag10Cleared = false;
    uint32_t result = 0u;
    bool lowerCallCommitted = false;
    bool psxMemoryAuthority = false;
    bool lowerEventAuthority = false;
    bool oldWinS0Authority = false;
    bool stage2PlusAuthority = false;
};

enum class SsMidiEnvelopeBranch8002B750 : uint8_t {
    Unknown = 0,
    NonPositiveReset,
    NonPositiveRamp,
    PositiveModuloSkip,
    PositiveReset,
    PositiveRamp,
};

struct SsMidiEnvelopeDispatch8002B750 {
    bool known = false;
    SsMidiEnvelopeBranch8002B750 branch =
        SsMidiEnvelopeBranch8002B750::Unknown;
    int16_t sequenceIndex = -1;
    int16_t trackIndex = -1;
    int16_t wordTrackOffset40After = 0;
    uint32_t dwordTrackOffset98After = 0u;
    uint32_t dwordTrackOffset90FlagsAfter = 0u;
    uint16_t wordTrackOffset74After = 0u;
    uint16_t wordTrackOffset76After = 0u;
    uint16_t wordTrackOffset78After = 0u;
    uint16_t wordTrackOffset7AAfter = 0u;
    uint16_t packedSequenceTrack = 0u;
    uint32_t helperReadFunction = 0u;
    uint32_t helperWriteFunction = 0u;
    bool helperReadObserved = false;
    uint32_t helperReadCallCount = 0u;
    bool helperWriteObserved = false;
    uint32_t helperWriteCallCount = 0u;
    int16_t helperWriteLeftRequested = 0;
    int16_t helperWriteRightRequested = 0;
    uint16_t helperWriteLeftApplied = 0u;
    uint16_t helperWriteRightApplied = 0u;
    bool flag20Cleared = false;
    uint32_t result = 0u;
    bool lowerCallCommitted = false;
    bool psxMemoryAuthority = false;
    bool lowerEventAuthority = false;
    bool oldWinS0Authority = false;
    bool stage2PlusAuthority = false;
};

// 8002DDA4 updates the per-track tempo accumulator at +0x8C, derives the
// low-halfword tick interval at +0x70, and clears the 0x40/0x80 flags when the
// counter or target is exhausted.  The global tick-rate is an explicit input
// so division-by-zero remains fail-closed instead of emulating _break().
enum class SsMidiTempoTickBranch8002DDA4 : uint8_t {
    Unknown = 0,
    PositiveModuloSkip,
    PositiveStepDown,
    NonPositiveHold,
    NonPositiveRebase,
    NonPositiveClamp,
};

struct SsMidiTempoTickObservation8002DDA4 {
    bool known = false;
    int16_t sequenceIndex = -1;
    int16_t trackIndex = -1;
    int16_t wordTrackOffset44 = 0;
    int16_t wordTrackOffset74 = 0;
    uint32_t dwordTrackOffset8C = 0u;
    uint32_t dwordTrackOffsetA0 = 0u;
    uint32_t dwordTrackOffsetA4 = 0u;
    uint32_t dwordTrackOffset90Flags = 0u;
    uint32_t tickRate80095C4C = 0u;
};

struct SsMidiTempoTickDispatch8002DDA4 {
    bool known = false;
    SsMidiTempoTickBranch8002DDA4 branch =
        SsMidiTempoTickBranch8002DDA4::Unknown;
    int16_t sequenceIndex = -1;
    int16_t trackIndex = -1;
    uint32_t dwordTrackOffset8CAfter = 0u;
    uint32_t dwordTrackOffsetA0After = 0u;
    uint16_t wordTrackOffset70After = 0u;
    uint32_t dwordTrackOffset90FlagsAfter = 0u;
    uint32_t result = 0u;
    uint32_t positiveModulo = 0u;
    bool flags40And80Cleared = false;
    bool divisorKnown = false;
    uint64_t numerator = 0u;
    uint64_t denominator = 0u;
    bool lowerCallCommitted = false;
    bool psxMemoryAuthority = false;
    bool lowerEventAuthority = false;
    bool oldWinS0Authority = false;
    bool stage2PlusAuthority = false;
};

bool TryBuildSsMidiEnvelopeDispatch8002B474(
    const SsMidiParserSurface8002BA9C& surface,
    const SsMidiEnvelopeObservation8002B474& observation,
    SsMidiEnvelopeDispatch8002B474& out);

bool TryBuildSsMidiEnvelopeDispatch8002B750(
    const SsMidiParserSurface8002BA9C& surface,
    const SsMidiEnvelopeObservation8002B474& observation,
    SsMidiEnvelopeDispatch8002B750& out);

bool TryBuildSsMidiTempoTickDispatch8002DDA4(
    const SsMidiParserSurface8002BA9C& surface,
    const SsMidiTempoTickObservation8002DDA4& observation,
    SsMidiTempoTickDispatch8002DDA4& out);

// Remaining 8002B200 flag routes.  B9FC/BAC8 only toggle the per-track
// active byte and clear one flag after notifying 80033A34.  DBE4 performs the
// full track reset used by the 0x04 route, including the 16-channel scratch
// loop and stream/cursor field copies.  All call targets remain symbolic and
// are never invoked through a host substitute.
struct SsMidiTrackFlagToggleObservation8002B9FC {
    bool known = false;
    int16_t sequenceIndex = -1;
    int16_t trackIndex = -1;
    uint8_t byteTrackOffset2B = 0u;
    uint32_t dwordTrackOffset90Flags = 0u;
};

struct SsMidiTrackFlagToggleDispatch8002B9FC {
    bool known = false;
    int16_t sequenceIndex = -1;
    int16_t trackIndex = -1;
    uint8_t byteTrackOffset2BAfter = 0u;
    uint32_t dwordTrackOffset90FlagsAfter = 0u;
    uint16_t packedSequenceTrack = 0u;
    uint32_t cleanupFunction = 0u;
    bool cleanupObserved = false;
    uint32_t result = 0u;
    bool lowerCallCommitted = false;
    bool psxMemoryAuthority = false;
    bool lowerEventAuthority = false;
    bool oldWinS0Authority = false;
    bool stage2PlusAuthority = false;
};

struct SsMidiTrackResetObservation8002DBE4 {
    bool known = false;
    int16_t sequenceIndex = -1;
    int16_t trackIndex = -1;
    uint32_t dwordTrackOffset90Flags = 0u;
    uint32_t dwordTrackOffset7C = 0u;
    uint32_t dwordTrackOffset84 = 0u;
    uint16_t wordTrackOffset72 = 0u;
    uint32_t dwordTrackOffset08 = 0u;
    uint8_t byteTrackOffset2B = 0u;
    uint8_t byteTrackOffset16 = 0u;
    uint8_t byteTrackOffset17 = 0u;
    uint8_t byteTrackOffset18 = 0u;
    uint8_t byteTrackOffset19 = 0u;
    uint8_t byteTrackOffset20 = 0u;
    uint8_t byteTrackOffset21 = 0u;
    uint8_t byteTrackOffset22 = 0u;
    uint8_t byteTrackOffset39 = 0u;
    uint8_t byteTrackOffset40 = 0u;
    uint8_t byteTrackOffset41 = 0u;
    uint8_t byteTrackOffset42 = 0u;
    uint8_t byteTrackOffset43 = 0u;
    std::array<uint8_t, kSsMidiChannelCount8002BEEC>
        channelProgramMarkers{};
    std::array<uint8_t, kSsMidiChannelCount8002BEEC> channelPans{};
    std::array<uint16_t, kSsMidiChannelCount8002BEEC> channelVolumes{};
};

struct SsMidiTrackResetDispatch8002DBE4 {
    bool known = false;
    int16_t sequenceIndex = -1;
    int16_t trackIndex = -1;
    uint32_t dwordTrackOffset90FlagsAfter = 0u;
    uint32_t dwordTrackOffset80After = 0u;
    uint32_t dwordTrackOffset88After = 0u;
    uint32_t dwordTrackOffset8CAfter = 0u;
    uint32_t dwordTrackOffset04After = 0u;
    uint32_t dwordTrackOffset0CAfter = 0u;
    uint16_t wordTrackOffset72After = 0u;
    uint16_t wordTrackOffset70After = 0u;
    uint8_t byteTrackOffset16After = 0u;
    uint8_t byteTrackOffset17After = 0u;
    uint8_t byteTrackOffset18After = 0u;
    uint8_t byteTrackOffset19After = 0u;
    uint8_t byteTrackOffset20After = 0u;
    uint8_t byteTrackOffset21After = 0u;
    uint8_t byteTrackOffset22After = 0u;
    uint8_t byteTrackOffset2BAfter = 0u;
    uint8_t byteTrackOffset39After = 0u;
    uint8_t byteTrackOffset40After = 0u;
    uint8_t byteTrackOffset41After = 0u;
    uint8_t byteTrackOffset42After = 0u;
    uint8_t byteTrackOffset43After = 0u;
    std::array<uint8_t, kSsMidiChannelCount8002BEEC>
        channelProgramMarkersAfter{};
    std::array<uint8_t, kSsMidiChannelCount8002BEEC> channelPansAfter{};
    std::array<uint16_t, kSsMidiChannelCount8002BEEC> channelVolumesAfter{};
    uint16_t wordTrackOffset78After = 0u;
    uint16_t wordTrackOffset7AAfter = 0u;
    uint16_t packedSequenceTrack = 0u;
    uint32_t cleanupFunction = 0u;
    bool cleanupObserved = false;
    uint32_t channelLoopCount = 0u;
    uint32_t result = 0u;
    bool lowerCallCommitted = false;
    bool psxMemoryAuthority = false;
    bool lowerEventAuthority = false;
    bool oldWinS0Authority = false;
    bool stage2PlusAuthority = false;
};

bool TryBuildSsMidiTrackFlagToggleDispatch8002B9FC(
    const SsMidiParserSurface8002BA9C& surface,
    const SsMidiTrackFlagToggleObservation8002B9FC& observation,
    SsMidiTrackFlagToggleDispatch8002B9FC& out);

bool TryBuildSsMidiTrackFlagToggleDispatch8002BAC8(
    const SsMidiParserSurface8002BA9C& surface,
    const SsMidiTrackFlagToggleObservation8002B9FC& observation,
    SsMidiTrackFlagToggleDispatch8002B9FC& out);

bool TryBuildSsMidiTrackResetDispatch8002DBE4(
    const SsMidiParserSurface8002BA9C& surface,
    const SsMidiTrackResetObservation8002DBE4& observation,
    SsMidiTrackResetDispatch8002DBE4& out);

SsMidiNoteSurface8002BEEC BuildSsMidiNoteSurface8002BEEC();

bool IsExactSsMidiNoteSurface8002BEEC(
    const SsMidiNoteSurface8002BEEC& surface);

bool TryBuildSsMidiNoteDispatch8002BEEC(
    const SsMidiNoteSurface8002BEEC& surface,
    SsMidiNoteTrackState8002BEEC& state,
    uint8_t note,
    uint8_t velocity,
    SsMidiNoteDispatch8002BEEC& out);

SsDriverFlushSurface80026ECC BuildSsDriverFlushSurface80026ECC();

bool IsExactSsDriverFlushSurface80026ECC(
    const SsDriverFlushSurface80026ECC& surface);

bool TryExecuteSsDriverFlush80026ECC(
    const SsDriverFlushSurface80026ECC& surface,
    SsDriverFlushState80026ECC& state,
    SsDriverCommitSink80032B00 driverCommit,
    void* userData,
    SsDriverFlushDispatch80026ECC& out);

SsDriverVoiceCompletionSurface80032B00
BuildSsDriverVoiceCompletionSurface80032B00();

bool IsExactSsDriverVoiceCompletionSurface80032B00(
    const SsDriverVoiceCompletionSurface80032B00& surface);

bool TryExecuteSsDriverVoiceCompletion80032B00(
    const SsDriverVoiceCompletionSurface80032B00& surface,
    const SsDriverVoiceCompletionObservation80032B00& observation,
    SsDriverVoiceCompletionState80032B00& state,
    SsDriverVoiceCompletionDispatch80032B00& out);

SsDriverVolumeRampSurface80032B00
BuildSsDriverVolumeRampSurface80032B00();

bool IsExactSsDriverVolumeRampSurface80032B00(
    const SsDriverVolumeRampSurface80032B00& surface);

bool TryExecuteSsDriverVolumeRamp80032B00(
    const SsDriverVolumeRampSurface80032B00& surface,
    SsDriverVolumeRampKind80032B00 kind,
    const SsDriverVolumeMix80032B00& mix,
    SsDriverVolumeScratch80032B00& scratch,
    SsDriverVoiceRegisterState80032B00& voice,
    SsDriverVolumeRampDispatch80032B00& out);

SsDriverVolumeRampSetupSurface80035110
BuildSsDriverVolumeRampSetupSurface80035110();

bool IsExactSsDriverVolumeRampSetupSurface80035110(
    const SsDriverVolumeRampSetupSurface80035110& surface);

bool TryExecuteSsDriverVolumeRampSetup80035110(
    const SsDriverVolumeRampSetupSurface80035110& surface,
    uint16_t voiceIndex,
    int16_t start,
    int16_t target,
    int16_t rate,
    SsDriverVoiceRegisterState80032B00& voice,
    SsDriverVolumeRampSetupDispatch80035110& out);

SsDriverDirectVolumeWriteSurface80034E5C
BuildSsDriverDirectVolumeWriteSurface80034E5C();

bool IsExactSsDriverDirectVolumeWriteSurface80034E5C(
    const SsDriverDirectVolumeWriteSurface80034E5C& surface);

bool TryExecuteSsDriverDirectVolumeWrite80034E5C(
    const SsDriverDirectVolumeWriteSurface80034E5C& surface,
    SsDriverDirectVolumeWriteKind80034E5C kind,
    uint16_t voiceIndex,
    int16_t sourceVabId,
    int16_t program,
    int16_t note,
    int16_t left,
    int16_t right,
    const SsDriverVoiceOwnerState8003226C& owner,
    SsDriverVoiceRegisterState80032B00& voice,
    SsDriverDirectVolumeWriteDispatch80034E5C& out);

SsDriverRegisterCommitSurface80032B00
BuildSsDriverRegisterCommitSurface80032B00();

bool IsExactSsDriverRegisterCommitSurface80032B00(
    const SsDriverRegisterCommitSurface80032B00& surface);

bool TryExecuteSsDriverRegisterCommit80032B00(
    const SsDriverRegisterCommitSurface80032B00& surface,
    std::array<SsDriverVoiceRegisterState80032B00, 24>& voices,
    SsDriverGlobalMaskState80032B00& masks,
    SsDriverRegisterCommitDispatch80032B00& out);

SsDriverInitializeSurface8003226C
BuildSsDriverInitializeSurface8003226C();

bool IsExactSsDriverInitializeSurface8003226C(
    const SsDriverInitializeSurface8003226C& surface);

bool TryInitializeSsDriverState8003226C(
    const SsDriverInitializeSurface8003226C& surface,
    uint8_t requestedVoiceCount,
    std::array<SsDriverVoiceOwnerState8003226C, 24>& voiceOwners,
    SsDriverVoiceCompletionState80032B00& completionState,
    std::array<SsDriverVoiceRegisterState80032B00, 24>& voices,
    SsDriverVolumeMix80032B00& mix,
    SsDriverGlobalMaskState80032B00& masks,
    SsDriverInitializeState8003226C& state,
    SsDriverCommitSink80032B00 driverCommit,
    void* userData,
    SsDriverInitializeDispatch8003226C& out);

SsDriverResetSurface800351B8
BuildSsDriverResetSurface800351B8();

bool IsExactSsDriverResetSurface800351B8(
    const SsDriverResetSurface800351B8& surface);

bool TryExecuteSsDriverReset800351B8(
    const SsDriverResetSurface800351B8& surface,
    std::array<SsDriverVoiceOwnerState8003226C, 24>& voiceOwners,
    SsDriverVoiceCompletionState80032B00& completionState,
    std::array<SsDriverVoiceRegisterState80032B00, 24>& voices,
    SsDriverGlobalMaskState80032B00& masks,
    SsDriverInitializeState8003226C& state,
    SsDriverResetDispatch800351B8& out);

bool TryProjectSsDriverVoiceEnvelope80032B00(
    const SsDriverVoiceCompletionSurface80032B00& surface,
    const SsDriverVoiceCompletionObservation80032B00& observation,
    std::array<SsDriverVoiceOwnerState8003226C, 24>& voiceOwners);

SsSpuFirstAllocationSurface8002E87C
BuildSsSpuFirstAllocationSurface8002E87C();

bool IsExactSsSpuFirstAllocationSurface8002E87C(
    const SsSpuFirstAllocationSurface8002E87C& surface);

bool TryInitializeSsSpuAllocator80035394(
    const SsSpuFirstAllocationSurface8002E87C& surface,
    SsSpuFirstAllocationState8002E87C& state,
    SsSpuFirstAllocationDispatch8002E87C& out);

bool TryAllocateFirstSsSpuBlock8002E87C(
    const SsSpuFirstAllocationSurface8002E87C& surface,
    uint32_t requestedBytes,
    SsSpuFirstAllocationState8002E87C& state,
    SsSpuFirstAllocationDispatch8002E87C& out);

SsVabBodyTransferSurface8002EB80
BuildSsVabBodyTransferSurface8002EB80();

bool IsExactSsVabBodyTransferSurface8002EB80(
    const SsVabBodyTransferSurface8002EB80& surface);

bool TryPrepareFirstSsVabBodyTransferState8002E474(
    const SsVabBodyTransferSurface8002EB80& surface,
    uint32_t allocatedSpuBase,
    uint32_t transferBytes,
    SsVabBodyTransferState8002EB80& state);

bool TryBeginSsVabBodyTransfer8002EB80(
    const SsVabBodyTransferSurface8002EB80& surface,
    uint16_t vabId,
    uint32_t sourceBytesAvailable,
    SsVabBodyTransferState8002EB80& state,
    SsVabBodyTransferDispatch8002EB80& out);

bool TryCompleteSsVabBodyTransfer8002EEFC(
    const SsVabBodyTransferSurface8002EB80& surface,
    bool waitRequested,
    bool eventObserved,
    SsVabBodyTransferState8002EB80& state,
    SsVabTransferCompletionDispatch8002EF28& out);

SsCompactSfxSurface80034240 BuildSsCompactSfxSurface80034240();

bool IsExactSsCompactSfxSurface80034240(
    const SsCompactSfxSurface80034240& surface);

bool TryExecuteSsCompactSfxCommand80026EF8(
    const SsCompactSfxSurface80034240& surface,
    const SsCompactSfxRequest80034240& request,
    SsDriverFlushState80026ECC& reentryState,
    SsDriverInitializeState8003226C& initializeState,
    std::array<SsDriverVoiceOwnerState8003226C, 24>& voiceOwners,
    SsDriverVoiceCompletionState80032B00& completionState,
    std::array<SsDriverVoiceRegisterState80032B00, 24>& voices,
    SsDriverVolumeMix80032B00& mix,
    SsDriverVolumeScratch80032B00& scratch,
    SsDriverGlobalMaskState80032B00& masks,
    SsCompactSfxDispatch80034240& out);

SsDriverVoiceAllocationSurface80032EAC
BuildSsDriverVoiceAllocationSurface80032EAC();

bool IsExactSsDriverVoiceAllocationSurface80032EAC(
    const SsDriverVoiceAllocationSurface80032EAC& surface);

bool TryExecuteSsDriverNoteOn80032EAC(
    const SsDriverVoiceAllocationSurface80032EAC& surface,
    const SsDriverNoteOnRequest80032EAC& request,
    SsDriverInitializeState8003226C& initializeState,
    std::array<SsDriverVoiceOwnerState8003226C, 24>& voiceOwners,
    SsDriverVoiceCompletionState80032B00& completionState,
    std::array<SsDriverVoiceRegisterState80032B00, 24>& voices,
    SsDriverVolumeMix80032B00& mix,
    SsDriverVolumeScratch80032B00& scratch,
    SsDriverGlobalMaskState80032B00& masks,
    SsDriverNoteOnDispatch80032EAC& out);

bool TryExecuteSsDriverNoteOff8003349C(
    const SsDriverVoiceAllocationSurface80032EAC& surface,
    uint16_t packedSequenceTrack,
    int16_t sourceVabId,
    uint8_t program,
    uint8_t note,
    SsDriverInitializeState8003226C& initializeState,
    std::array<SsDriverVoiceOwnerState8003226C, 24>& voiceOwners,
    SsDriverVoiceCompletionState80032B00& completionState,
    SsDriverGlobalMaskState80032B00& masks,
    SsDriverNoteOffDispatch8003349C& out);

Transaction8001A8F0 BuildTransaction8001A8F0(
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& intLoad,
    const PrSS0Scene0IntSideEffectDirect::Transaction8001A8F0&
        loaderCandidate,
    const PadStartComSource80026E4C& padStartComSource);

bool IsExactAcceptedTransaction8001A8F0(
    const Transaction8001A8F0& transaction,
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& intLoad,
    const PrSS0Scene0IntSideEffectDirect::Transaction8001A8F0&
        loaderCandidate,
    const PadStartComSource80026E4C& padStartComSource);

Transaction8001A8F0 BuildYCompoTransaction8001A8F0(
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& intLoad,
    const PrSS0Scene0IntSideEffectDirect::Transaction8001A8F0&
        loaderCandidate,
    const PadStartComSource80026E4C& padStartComSource);

bool IsExactAcceptedYCompoTransaction8001A8F0(
    const Transaction8001A8F0& transaction,
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& intLoad,
    const PrSS0Scene0IntSideEffectDirect::Transaction8001A8F0&
        loaderCandidate,
    const PadStartComSource80026E4C& padStartComSource);

bool CommitYCompoPadStartComGlobalWrites80026E4C(
    Transaction8001A8F0& transaction,
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& intLoad,
    const PrSS0Scene0IntSideEffectDirect::Transaction8001A8F0&
        loaderCandidate,
    const PadStartComSource80026E4C& padStartComSource);

bool IsExactYCompoPadStartComGlobalWritesCommittedTransaction80026E4C(
    const Transaction8001A8F0& transaction,
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& intLoad,
    const PrSS0Scene0IntSideEffectDirect::Transaction8001A8F0&
        loaderCandidate,
    const PadStartComSource80026E4C& padStartComSource);

bool CommitPadStartComGlobalWrites80026E4C(
    Transaction8001A8F0& transaction,
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& intLoad,
    const PrSS0Scene0IntSideEffectDirect::Transaction8001A8F0&
        loaderCandidate,
    const PadStartComSource80026E4C& padStartComSource);

bool IsExactPadStartComGlobalWritesCommittedTransaction80026E4C(
    const Transaction8001A8F0& transaction,
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& intLoad,
    const PrSS0Scene0IntSideEffectDirect::Transaction8001A8F0&
        loaderCandidate,
    const PadStartComSource80026E4C& padStartComSource);

bool CommitHostVabBank8001A8F0(Transaction8001A8F0& transaction);

bool IsExactCommittedTransaction8001A8F0(
    const Transaction8001A8F0& transaction,
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& intLoad,
    const PrSS0Scene0IntSideEffectDirect::Transaction8001A8F0&
        loaderCandidate,
    const PadStartComSource80026E4C& padStartComSource);

bool IsExactCommittedYCompoTransaction8001A8F0(
    const Transaction8001A8F0& transaction,
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& intLoad,
    const PrSS0Scene0IntSideEffectDirect::Transaction8001A8F0&
        loaderCandidate,
    const PadStartComSource80026E4C& padStartComSource);

}  // namespace PrSS0Scene0IntSpuDirect
