#pragma once

#include <cstdint>

#include "pr_ss0_word800916f0_direct.h"

struct PrGameContext;
class PsxVramAtlas;
namespace PrPsxGraphOwnerDirect { struct PsxGraphState; }

namespace PrSS0Scene0IntLoadDirect {
struct Transaction8001AC18;
}

namespace PrSS0CardMemcardHandoffDirect {
struct LoadReplayDirectoryScanFacts80019D7C;
struct LoadReplayState16CardIoResultCarrier80017594;
}

namespace PrStage1SaveCardHalDirect {
struct State16CardReadRuntimeTypedFacts800179B4;
}

namespace PrSS0Scene0RuntimeDirect {

// Deliver the existing 60Hz presentation clock to a suspended named-card
// VSync(0), independently of30Hz Menu logic. No new clock source is created.
void AdvanceHiScoreReadVblankClock(PrGameContext& ctx);

enum class Word800916F6AuthoritySource : uint8_t {
    Unknown = 0,
    ProcessStartupZero80028590 = 1,
};

enum class Word800916FCAuthoritySource : uint8_t {
    Unknown = 0,
    ProcessStartupZero80028590 = 1,
};

struct DebugSnapshot {
    int phaseRaw = -1;
    int phaseDebug = -1;
    int transitionReturnPhaseRaw = -1;
    int directDispState = 0;
    int directDispEventId = 0;
    int directMenuIndex = -1;
    int directMainMenuRecordsMode = 0;
    int directCardEntryCount = 0;
    int directCardDriverState = 0;
    int directCardDriverEvent = 0;
    int directCardDirectoryAuthority = 0;
    int directCardSelectedBlock = -1;
    int directOptionsLanguage = -1;
    int directOptionsSubtitle = -1;
    int directOptionsPreLoopReleasePending = -1;
    int directOptionsCooldown = -1;
    int directOptionsTimeoutRemaining = -1;
    int directOptionsInitialInputPending = -1;
    int directOptionsTailActive = -1;
    int directOptionsTailFramesRemaining = -1;
    int directOptionsTailResult = -1;
    int directPracticePhase = -1;
    int directPracticeRound = -1;
    int directPracticeRoundFrame = -1;
    int directPracticePadStopFrames = 0;
    int directPracticePadStopKind = -1;
    int directPracticeExitConfirmFrames = 0;
    int directPracticeScore = 0;
    int directPracticeHits = 0;
    int directPracticeMisses = 0;
    int directPracticeLastJudge = 9;
    int directPracticeLastDelta = 0;
    int directPracticeJudgeFlash = 0;
    int directPracticeSeqCurA = -1;
    int directPracticeSeqCurB = -1;
    int directPracticeSeqEnA = 0;
    int directPracticeSeqEnB = 0;
    int directHiScoreBlink = 0;
    int directHiScoreExitLabelState = -1;
    int directHiScorePreLoopReleasePending = -1;
    int directHiScoreTailActive = -1;
    int directHiScoreTailFramesRemaining = -1;
    int directHiScoreTailResult = -1;
    int directHiScoreOuterPadReleaseActive = -1;
    int directHiScoreOuterPadReleasePending = -1;
    int directHiScoreEvent6TableKnown = 0;
    uint32_t directHiScoreEvent6TablePsxAddress = 0;
    int directHiScoreEvent6TableByteCount = 0;
    int directStageSelectEnabledMask = 0;
    int directTitleExitWaitCounter = 0;
    int directTitleExitTargetScene = -1;
    int directTitleExitPendingMenu = 0;
    int directTitleLoopStateV8 = 0;
    int directTitleAttractTimeoutKnown = 0;
    int directTitleAttractTimeoutLimit = 0;
    int directTitleAttractTimer = 0;
    int loadingHoldKind = 0;
    int loadingScreenKind = 0;
    uint32_t loadingHoldStartFrame = 0;
    uint32_t loadingHoldUntilFrame = 0;
    int loadingPatternActive8001EF40 = 0;
    int loadingPatternStyle8001EF40 = 0;
    int loadingPatternHighlightCount8001EF40 = 0;
    int loadingPatternSubmittedHighlightCount8001EF40 = 0;
    uint32_t loadingPatternMutationSerial8001EF40 = 0;
    uint32_t loadingPatternCallbackCount8001537C = 0;
    uint32_t loadingPatternGridHashLow8001EF40 = 0;
    uint32_t loadingPatternSubmittedFrame8001EF40 = 0;
};

bool RuntimeEnabled();

// Resident SCUS 80015788 loop, independent of the current overlay scene.
// Begin uses the 80015D18 local v1; Tick never executes Stage1 Fn2.
bool BeginResidentDirectory80015788(PrGameContext& ctx, int previousScene,
    const PrPsxGraphOwnerDirect::PsxGraphState* outgoingGraph = nullptr,
    PsxVramAtlas* outgoingAtlas = nullptr);
bool IsResidentDirectoryActive80015788();
void TickResidentDirectory80015788(PrGameContext& ctx);
int ConsumeResidentDirectoryResult80015788();

// Immutable original-disc COMMON upload retained across the 80015D18 scene
// loop. A rendering projection may borrow it; no scene/menu state is exposed.
const PrSS0Scene0IntLoadDirect::Transaction8001AC18*
GetSharedStartupCommonIntLoad80016B84();
// The startup ZCOMPO transaction is retained beside COMMON for a direct
// stage handoff.  Directory rendering uses its TIM/CLUT records from the
// same accepted disc transaction instead of reopening the CD device.
const PrSS0Scene0IntLoadDirect::Transaction8001AC18*
GetSharedStartupZCompoIntLoad80015590();
// Execute the original-disc lookup and INT payload-read/parse portion of
// SCUS 80015D18 -> 80016B84 before the first logo frame.  The prepared
// COMMON/ZCOMPO transactions are later consumed by Scene0 without a second
// disc read.  The translated 80026FA4 software reset tail is committed here;
// TIM/VAB transfer and physical SPU side effects remain separately gated.
bool PrepareStartupDiscIntBeforeBootLogo80016B84(PrGameContext& ctx);
void SetWord800916F0DiscFullbootStartupPublisherDisabledForProbe(
    bool disabled);
int Fn0(PrGameContext& ctx);
void Fn1(PrGameContext& ctx);
int Fn2(PrGameContext& ctx);
void Main(PrGameContext& ctx);
void Render(PrGameContext& ctx);
void RenderLateSubtitles(PrGameContext& ctx);
// Scene0's direct scene handoff is owned by SS0.  PrMain consumes a target
// only after the translated 80020110 runtime has completed its final tail;
// this carrier deliberately has no PrTransition/Windows-S0 visual state.
int ConsumeSceneHandoffTarget();
bool IsSceneHandoffActive();
int GetSceneHandoffTarget();
int GetPhaseDebug();
void ApplyWord800916EEMainLoopWriteback80015D18(int32_t v2,
                                                uint16_t word800916D0);
bool PublishInitialWord800916F0RuntimeObservation(
    const Word800916F0Observation& observation);
bool InitializeWord800916F6FromProcessStartupZero80028590();
bool InitializeWord800916FCFromProcessStartupZero80028590();
bool DebugUnlockNextStage(const PrGameContext& ctx);
bool DebugFirstClearSelectableStages(const PrGameContext& ctx);
bool DebugSeedCardAuthGapEntries(int count, int firstBlockIndex);
bool DebugPublishRuntimeDirectoryScanFacts(
    const PrSS0CardMemcardHandoffDirect::
        LoadReplayDirectoryScanFacts80019D7C& facts);
bool DebugSeedCardInvalidIndexEntry(int invalidBlockIndex);
bool DebugSeedReplayRawSelector(int savedSlot, int blockIndex);
bool DebugArmHiScoreEvent6EntryFixture();
bool QueueRuntimeState16CardReadTypedFactsOneShot(
    const PrStage1SaveCardHalDirect::State16CardReadRuntimeTypedFacts800179B4&
        facts);
bool QueueRuntimeLoadReplayState16CardIoResult80017594(
    const PrSS0CardMemcardHandoffDirect::
        LoadReplayState16CardIoResultCarrier80017594& carrier);
bool DebugPublishState16TypedPayloadCarrier(int byteSeed);
bool DebugSeedAttractLastScene(int scene);
bool DebugSeedTitleExitInvalidTarget(int targetScene);
DebugSnapshot GetDebugSnapshot();

} // namespace PrSS0Scene0RuntimeDirect
