#include "pr_ss0_card_memcard_handoff_direct.h"

#include "pr_ss0_stage_progress_bank_direct.h"

namespace PrSS0CardMemcardHandoffDirect {
namespace {

static LoadReplayDirectoryTypedCarrier80019D7C
    s_loadReplayDirectoryTypedCarrier80019D7C{};

static bool IsLoadReplayDirectoryMode80019D7C(CardMode800191E4 mode)
{
    return mode == CardMode800191E4::Load ||
           mode == CardMode800191E4::Replay;
}

static void CopyLoadReplayDirectoryTitle80019D7C(
    char (&dst)[kCardDirectoryNameMax80017900 + 1u],
    const char (&src)[kCardDirectoryNameMax80017900 + 1u])
{
    for (uint32_t i = 0; i < kCardDirectoryNameMax80017900; ++i) {
        dst[i] = src[i];
        if (src[i] == '\0') {
            return;
        }
    }
    dst[kCardDirectoryNameMax80017900] = '\0';
}

static bool ValidateLoadReplayDirectoryTypedCarrier80019D7C(
    LoadReplayDirectoryTypedCarrier80019D7C& carrier)
{
    if (!carrier.known ||
        !IsLoadReplayDirectoryMode80019D7C(carrier.mode) ||
        !carrier.entryCountKnown ||
        carrier.source !=
            LoadReplayDirectoryTypedCarrierSource80019D7C::
                RuntimeDirectoryProducer ||
        !carrier.producerWired80017B08_80017B18_80019D7C ||
        carrier.entryCount < 0 ||
        carrier.entryCount > static_cast<int32_t>(kSaveListRowCount80019458)) {
        carrier.incomplete = true;
        carrier.source =
            LoadReplayDirectoryTypedCarrierSource80019D7C::Unknown;
        carrier.producerWired80017B08_80017B18_80019D7C = false;
        return false;
    }

    for (int32_t row = 0; row < carrier.entryCount; ++row) {
        LoadReplayDirectoryRow80019D7C& entry = carrier.rows[row];
        if (!entry.blockIndexKnown ||
            entry.blockIndex < 0 ||
            entry.blockIndex >=
                static_cast<int32_t>(kSaveListRowCount80019458) ||
            !entry.titleKnown) {
            carrier.incomplete = true;
            carrier.source =
                LoadReplayDirectoryTypedCarrierSource80019D7C::Unknown;
            carrier.producerWired80017B08_80017B18_80019D7C = false;
            return false;
        }
        char normalizedTitle[kCardDirectoryNameMax80017900 + 1u]{};
        CopyLoadReplayDirectoryTitle80019D7C(normalizedTitle, entry.title);
        CopyLoadReplayDirectoryTitle80019D7C(entry.title, normalizedTitle);
        if (entry.rowNameKnown8007A590) {
            if (entry.rowName8007A590[0] == '\0') {
                carrier.incomplete = true;
                carrier.source =
                    LoadReplayDirectoryTypedCarrierSource80019D7C::Unknown;
                carrier.producerWired80017B08_80017B18_80019D7C = false;
                return false;
            }
            char normalizedRowName[
                kCardDirectoryNameMax80017900 + 1u]{};
            CopyLoadReplayDirectoryTitle80019D7C(
                normalizedRowName, entry.rowName8007A590);
            CopyLoadReplayDirectoryTitle80019D7C(
                entry.rowName8007A590, normalizedRowName);
        } else {
            entry.rowName8007A590[0] = '\0';
        }
    }
    carrier.incomplete = false;
    return true;
}

static bool Append(CardHandoffPlan& plan,
                   const CardHandoffAction& action)
{
    if (plan.count >= sizeof(plan.actions) / sizeof(plan.actions[0])) {
        plan.truncated = true;
        return false;
    }
    plan.actions[plan.count++] = action;
    return true;
}

static void AppendAction(CardHandoffPlan& plan,
                         CardHandoffActionKind kind,
                         uint32_t psxFunction,
                         int32_t arg0 = 0,
                         int32_t arg1 = 0,
                         int32_t arg2 = 0,
                         int32_t arg3 = 0,
                         int32_t rawResult = 0,
                         int32_t scene = 0,
                         CardMode800191E4 cardMode =
                             CardMode800191E4::Unknown,
                         CardEventFrameId eventFrame =
                             CardEventFrameId::Unknown)
{
    CardHandoffAction action{};
    action.kind = kind;
    action.psxFunction = psxFunction;
    action.args[0] = arg0;
    action.args[1] = arg1;
    action.args[2] = arg2;
    action.args[3] = arg3;
    action.rawResult = rawResult;
    action.scene = scene;
    action.cardMode = cardMode;
    action.eventFrame = eventFrame;
    (void)Append(plan, action);
}

static void AppendPlan(CardHandoffPlan& dst,
                       const CardHandoffPlan& src)
{
    for (uint32_t i = 0; i < src.count; ++i) {
        (void)Append(dst, src.actions[i]);
    }
    dst.truncated = dst.truncated || src.truncated;
    dst.blockedByP0Gap = dst.blockedByP0Gap || src.blockedByP0Gap;
}

static CardHandoffPlan MakePlan(const char* name)
{
    CardHandoffPlan plan{};
    plan.name = name;
    plan.runtimeCutoverAllowed = RuntimeCutoverAllowed();
    return plan;
}

static void AppendCardIoStateCommit(CardHandoffPlan& plan,
                                    const CardIoState80017594& state)
{
    AppendAction(plan, CardHandoffActionKind::CommitCardIoState80017594,
                 kFn80017594,
                 static_cast<int32_t>(kCardIoState800917E8),
                 state.dword800917E8,
                 static_cast<int32_t>(kCardIoResult800917EC),
                 state.dword800917EC);
    AppendAction(plan, CardHandoffActionKind::CommitCardIoState80017594,
                 kFn80017594,
                 static_cast<int32_t>(kCardIoPending800917F0),
                 state.dword800917F0,
                 static_cast<int32_t>(kCardIoLoaded800917F4),
                 state.dword800917F4);
    AppendAction(plan, CardHandoffActionKind::CommitCardIoState80017594,
                 kFn80017594,
                 static_cast<int32_t>(kGpCardIoCountdownOffset),
                 state.gp700);
}

static void AppendCardIoReturn(CardHandoffPlan& plan,
                               const CardIoState80017594& before,
                               const CardIoState80017594& after,
                               int32_t result)
{
    AppendCardIoStateCommit(plan, after);
    AppendAction(plan, CardHandoffActionKind::ReturnCardIo80017594,
                 kFn80017594,
                 before.dword800917E8,
                 after.dword800917E8,
                 result,
                 after.gp700,
                 result);
}

static void AppendCardIoFeedbackGate80017594(
    CardHandoffPlan& plan,
    CardIoFeedbackTransitionKind kind,
    const CardIoState80017594& before,
    const CardIoState80017594& after,
    bool swKnown,
    int32_t swPollResult80016E18,
    bool hwKnown,
    int32_t hwPollResult80017008,
    bool authorized)
{
    AppendAction(plan,
                 CardHandoffActionKind::GateCardIoFeedbackTransition80017594,
                 kFn80017594,
                 static_cast<int32_t>(kind),
                 before.dword800917E8,
                 swKnown ? swPollResult80016E18 : -1,
                 hwKnown ? hwPollResult80017008 : -1,
                 after.dword800917E8,
                 authorized ? 1 : 0);
}

static CardEventFrameId CardModeEventFrame(CardMode800191E4 mode)
{
    switch (mode) {
    case CardMode800191E4::Save:
        return CardEventFrameId::Save;
    case CardMode800191E4::Load:
        return CardEventFrameId::Load;
    case CardMode800191E4::Replay:
        return CardEventFrameId::Replay;
    case CardMode800191E4::HiScore:
        return CardEventFrameId::Unknown;
    case CardMode800191E4::Unknown:
        return CardEventFrameId::Unknown;
    }
    return CardEventFrameId::Unknown;
}

static constexpr CardStateRouteSpec kCardStateRoutes[] = {
    {2,
     CardMode800191E4::Save,
     CardEventFrameId::SaveUi,
     4,
     0,
     kFn800185D0,
     4,
     23,
     false,
     "save entry confirm prompt",
     "800180D8 maps state 2 to event 11; 800185D0 state 2 handles confirm/cancel"},
    {21,
     CardMode800191E4::Save,
     CardEventFrameId::SaveUi,
     4,
     0,
     kFn800185D0,
     2,
     -1,
     false,
     "save entry bootstrap prompt",
     "80019148 calls 80018FB0(savePtr,800185D0,80019458,21,11); 800180D8 maps 21 to event 11"},
    {5,
     CardMode800191E4::Save,
     CardEventFrameId::InsertCardPrompt,
     1,
     0,
     kFn800185D0,
     2,
     -1,
     false,
     "save insert-card retry prompt",
     "80019458 state 4 with io 3 returns state 5; 800180D8 maps state 5 to event 12"},
    {5,
     CardMode800191E4::Load,
     CardEventFrameId::InsertCardPrompt,
     1,
     0,
     kFn80018E10,
     23,
     -1,
     true,
     "load insert-card exit prompt",
     "80019D7C state 4 with io 3 returns state 5; 80018E10 state 5 exits after event 12 flash"},
    {5,
     CardMode800191E4::Replay,
     CardEventFrameId::InsertCardPrompt,
     1,
     0,
     kFn80018E10,
     23,
     -1,
     true,
     "replay insert-card exit prompt",
     "80019D7C state 4 with io 3 returns state 5; 80018E10 state 5 exits after event 12 flash"},
    {5,
     CardMode800191E4::HiScore,
     CardEventFrameId::InsertCardPrompt,
     1,
     0,
     kFn80018E10,
     23,
     -1,
     true,
     "hi-score insert-card exit prompt",
     "80019D7C state 4 with io 3 returns state 5; 80018E10 state 5 exits after event 12 flash"},
    {7,
     CardMode800191E4::Save,
     CardEventFrameId::NoSpacePrompt,
     3,
     0,
     kFn800185D0,
     2,
     -1,
     false,
     "save no-space prompt",
     "800185D0 state 10 returns state 7 when no vacant block remains; 800180D8 maps state 7 to event 14"},
    {8,
     CardMode800191E4::Save,
     CardEventFrameId::FormatPrompt,
     2,
     0,
     kFn800185D0,
     14,
     2,
     false,
     "save format confirmation prompt",
     "80019458 state 4 with io 5 returns state 8 for save; 800185D0 state 8 confirm advances to format state 14"},
    {10,
     CardMode800191E4::Save,
     CardEventFrameId::CardInfo,
     -1,
     kCardNameArg80049244,
     kFn800185D0,
     -1,
     -1,
     false,
     "save name-entry and card-info page",
     "800180D8 maps state 10 to event 5 and arg 80049244"},
    {11,
     CardMode800191E4::Save,
     CardEventFrameId::Save,
     -1,
     kCardGridArg80048E50,
     kFn800185D0,
     -1,
     -1,
     false,
     "save grid page",
     "800180D8 maps state 11 to event 7 and arg 80048E50"},
    {12,
     CardMode800191E4::Load,
     CardEventFrameId::Load,
     -1,
     kCardGridArg80048E50,
     kFn80018E10,
     -1,
     -1,
     false,
     "load grid page",
     "80019D7C maps load mode to state 12; 800180D8 maps state 12 to event 8 and arg 80048E50"},
    {13,
     CardMode800191E4::Replay,
     CardEventFrameId::Replay,
     -1,
     kCardGridArg80048E50,
     kFn80018E10,
     -1,
     -1,
     false,
     "replay grid page",
     "80019D7C maps replay mode to state 13; 800180D8 maps state 13 to event 9 and arg 80048E50"},
    {18,
     CardMode800191E4::Save,
     CardEventFrameId::RenamePrompt,
     5,
     0,
     kFn800185D0,
     10,
     -1,
     false,
     "save duplicate-name prompt",
     "800185D0 state 10 returns state 18 on duplicate name; state 18 flashes event 15 and clears the suffix"},
    {19,
     CardMode800191E4::Load,
     CardEventFrameId::UnreadablePrompt,
     6,
     0,
     kFn80018E10,
     23,
     -1,
     true,
     "load unreadable-card exit prompt",
     "80019D7C state 4 with io 5 returns state 19 for non-save modes; 800180D8 maps state 19 to event 18"},
    {19,
     CardMode800191E4::Replay,
     CardEventFrameId::UnreadablePrompt,
     6,
     0,
     kFn80018E10,
     23,
     -1,
     true,
     "replay unreadable-card exit prompt",
     "80019D7C state 4 with io 5 returns state 19 for non-save modes; 800180D8 maps state 19 to event 18"},
    {19,
     CardMode800191E4::HiScore,
     CardEventFrameId::UnreadablePrompt,
     6,
     0,
     kFn80018E10,
     23,
     -1,
     true,
     "hi-score unreadable-card exit prompt",
     "80019D7C state 4 with io 5 returns state 19 for non-save modes; 800180D8 maps state 19 to event 18"},
    {22,
     CardMode800191E4::Save,
     CardEventFrameId::OverwritePrompt,
     7,
     0,
     kFn800185D0,
     15,
     11,
     false,
     "save overwrite confirmation prompt",
     "800180D8 maps state 22 to event 19; 800185D0 state 22 confirm advances to state 15"},
};

static constexpr CardIoRouteSpec kCardIoRoutes[] = {
    {CardIoCode80017594::InfoErrorOrTimeout,
     CardMode800191E4::Unknown,
     4,
     4,
     CardEventFrameId::Unknown,
     "card-info error or local timeout keeps polling",
     "80019458/80019D7C case 4 does not leave state 4 for io -3"},
    {CardIoCode80017594::IoSuccess,
     CardMode800191E4::Unknown,
     4,
     6,
     CardEventFrameId::Unknown,
     "I/O success enters directory/list build",
     "80019458/80019D7C case 4 maps io 1 to state 6"},
    {CardIoCode80017594::LoadErrorOrTimeout,
     CardMode800191E4::Unknown,
     4,
     4,
     CardEventFrameId::Unknown,
     "card-load error or local timeout keeps polling",
     "80019458/80019D7C case 4 does not leave state 4 for io 2"},
    {CardIoCode80017594::Timeout,
     CardMode800191E4::Unknown,
     4,
     5,
     CardEventFrameId::InsertCardPrompt,
     "card timeout enters insert-card prompt",
     "80019458/80019D7C case 4 maps io 3 to state 5"},
    {CardIoCode80017594::NewAtInfo,
     CardMode800191E4::Unknown,
     4,
     6,
     CardEventFrameId::Unknown,
     "new card during info phase enters directory/list build",
     "80019458/80019D7C case 4 maps io 4 to state 6"},
    {CardIoCode80017594::NewAtLoad,
     CardMode800191E4::Save,
     4,
     8,
     CardEventFrameId::FormatPrompt,
     "save load-phase card change enters format prompt",
     "80019458 case 4 maps io 5 to state 8 when gp+732 is 0"},
    {CardIoCode80017594::NewAtLoad,
     CardMode800191E4::Load,
     4,
     19,
     CardEventFrameId::UnreadablePrompt,
     "load load-phase card change enters unreadable prompt",
     "80019D7C case 4 maps io 5 to state 19 when gp+732 is nonzero"},
    {CardIoCode80017594::NewAtLoad,
     CardMode800191E4::Replay,
     4,
     19,
     CardEventFrameId::UnreadablePrompt,
     "replay load-phase card change enters unreadable prompt",
     "80019D7C case 4 maps io 5 to state 19 when gp+732 is nonzero"},
    {CardIoCode80017594::NewAtLoad,
     CardMode800191E4::HiScore,
     4,
     19,
     CardEventFrameId::UnreadablePrompt,
     "hi-score load-phase card change enters unreadable prompt",
     "80019D7C case 4 maps io 5 to state 19 when gp+732 is nonzero"},
};

static constexpr CardEventPollSourceSpec kCardEventPollSources[] = {
    {CardEventPollSourceKind::SwCardPoll80016EB8,
     kFn80016EB8,
     kSwCardEventClass,
     kGpCardSwEvent0Offset,
     kCardEventHandleCount,
     kCardPollTimeout80016EB8,
     0,
     -1,
     1,
     4,
     1,
     2,
     false,
     false,
     false,
     "80016EB8 polls SwCARD gp+664/668/672/676 up to 300 iterations and returns 1..4 for the observed event or 2 on timeout.",
     "Only typed SwCARD event facts authorize 800179B4/80017A10 success; host filesystem/card status and replay samples are not authority."},
    {CardEventPollSourceKind::SwCardPoll80016E18,
     kFn80016E18,
     kSwCardEventClass,
     kGpCardSwEvent0Offset,
     kCardEventHandleCount,
     0,
     kGpCardIoCountdownOffset,
     0,
     1,
     4,
     1,
     2,
     true,
     false,
     false,
     "80016E18 polls SwCARD gp+664/668/672/676 once per 80017594 tick, decrements gp+700, returns 0 while pending, and returns 2 when the decremented counter is negative.",
     "Only typed SwCARD event plus gp+700 counter facts authorize 80017594 feedback; host pending/failure state is not authority."},
    {CardEventPollSourceKind::HwCardPoll80017008,
     kFn80017008,
     kHwCardEventClass,
     kGpCardHwEvent0Offset,
     kCardEventHandleCount,
     0,
     0,
     -1,
     1,
     4,
     1,
     -1,
     false,
     true,
     false,
     "80017008 polls HwCARD gp+680/684/688/692 until a hardware event returns 1..4.",
     "Only typed HwCARD event facts authorize format completion or the 80017594 new-card reset branch; host memcard image creation is not authority."},
};

static const CardEventPollSourceSpec& CardEventPollSw80016EB8Spec()
{
    return kCardEventPollSources[0];
}

static const CardEventPollSourceSpec& CardEventPollSw80016E18Spec()
{
    return kCardEventPollSources[1];
}

static const CardEventPollSourceSpec& CardEventPollHw80017008Spec()
{
    return kCardEventPollSources[2];
}

static constexpr LowerCardResultSpec kLowerCardResults[] = {
    {LowerCardResultKind::ReadSuccess800179B4,
     kFn800179B4,
     CardEventPollSourceKind::SwCardPoll80016EB8,
     kFn80016EB8,
     1,
     1,
     0,
     false,
     1,
     false,
     true,
     true,
     false,
     "800179B4 read returns 0 only when typed 80016EB8 SwCARD poll result is 1.",
     "Read payload authority still requires separately known block bytes; host file/card success is not authority."},
    {LowerCardResultKind::ReadFailure800179B4,
     kFn800179B4,
     CardEventPollSourceKind::SwCardPoll80016EB8,
     kFn80016EB8,
     2,
     4,
     -1,
     false,
     1,
     false,
     false,
     false,
     false,
     "800179B4 read returns -1 for typed 80016EB8 non-success results 2..4.",
     "This catalogs PSX return mapping only; it does not synthesize event facts from host read failure."},
    {LowerCardResultKind::WriteSuccess80017A10,
     kFn80017A10,
     CardEventPollSourceKind::SwCardPoll80016EB8,
     kFn80016EB8,
     1,
     1,
     0,
     false,
     kCardWriteRetryCount80017A10,
     false,
     true,
     true,
     false,
     "80017A10 write returns 0 when an attempt observes typed 80016EB8 SwCARD poll result 1.",
     "The captured Stage1 save path proves this branch; Win save-file success is not PSX success authority."},
    {LowerCardResultKind::WriteRetryFailure80017A10,
     kFn80017A10,
     CardEventPollSourceKind::SwCardPoll80016EB8,
     kFn80016EB8,
     2,
     4,
     -1,
     false,
     kCardWriteRetryCount80017A10,
     true,
     false,
     false,
     false,
     "80017A10 write returns -1 only after all four attempts end with typed 80016EB8 non-success results.",
     "Retry exhaustion must be backed by typed poll facts for each attempt; host save failure is not authority."},
    {LowerCardResultKind::FormatSuccess80017B60,
     kFn80017B60,
     CardEventPollSourceKind::HwCardPoll80017008,
     kFn80017008,
     1,
     1,
     1,
     false,
     kCardFormatRetryCount80017B60,
     false,
     true,
     true,
     false,
     "80017B60 format returns 1 when typed 80017008 HwCARD poll result is 1.",
     "Host memcard image creation or format success cannot replace the HwCARD event fact."},
    {LowerCardResultKind::FormatPoll3Failure80017B60,
     kFn80017B60,
     CardEventPollSourceKind::HwCardPoll80017008,
     kFn80017008,
     3,
     3,
     3,
     false,
     kCardFormatRetryCount80017B60,
     false,
     false,
     false,
     false,
     "80017B60 format returns 3 immediately when typed 80017008 HwCARD poll result is 3.",
     "This is an event-code mapping, not a host-side format-error shortcut."},
    {LowerCardResultKind::FormatRetryExhausted80017B60,
     kFn80017B60,
     CardEventPollSourceKind::HwCardPoll80017008,
     kFn80017008,
     2,
     4,
     0,
     true,
     kCardFormatRetryCount80017B60,
     true,
     false,
     false,
     false,
     "80017B60 format returns the terminal typed 80017008 poll result when the third retry is exhausted.",
     "Only a terminal HwCARD event fact can close this branch; host format failure is not authority."},
};

static const LowerCardResultSpec& LowerCardReadSuccess800179B4Spec()
{
    return kLowerCardResults[0];
}

static const LowerCardResultSpec& LowerCardReadFailure800179B4Spec()
{
    return kLowerCardResults[1];
}

static const LowerCardResultSpec& LowerCardWriteSuccess80017A10Spec()
{
    return kLowerCardResults[2];
}

static const LowerCardResultSpec& LowerCardWriteFailure80017A10Spec()
{
    return kLowerCardResults[3];
}

static const LowerCardResultSpec& LowerCardFormatSuccess80017B60Spec()
{
    return kLowerCardResults[4];
}

static const LowerCardResultSpec& LowerCardFormatPoll3Failure80017B60Spec()
{
    return kLowerCardResults[5];
}

static const LowerCardResultSpec& LowerCardFormatRetryExhausted80017B60Spec()
{
    return kLowerCardResults[6];
}

static constexpr CardIoFeedbackTransitionSpec80017594
    kCardIoFeedbackTransitions80017594[] = {
        {CardIoFeedbackTransitionKind::State0InfoSubmit80017594,
         0,
         -1,
         false,
         false,
         1,
         1,
         0,
         false,
         -1,
         false,
         "state0 submits card_info(0), clears EC, seeds gp+700=300, and enters SwCARD info polling state1.",
         "Card-info submission is a lower-card side effect; this catalog fact does not prove card presence or host card data."},
        {CardIoFeedbackTransitionKind::State1InfoNoEvent80016E18,
         1,
         0,
         true,
         false,
         1,
         1,
         0,
         false,
         -1,
         false,
         "state1 SwCARD poll result 0 preserves EC/F0/F4 and keeps polling state1 after decrementing gp+700.",
         "Requires typed 80016E18 SwCARD event and counter facts; host idle/pending status is not authority."},
        {CardIoFeedbackTransitionKind::State1InfoSuccess80016E18,
         1,
         1,
         true,
         false,
         2,
         4,
         1,
         true,
         -1,
         false,
         "state1 SwCARD poll result 1 writes F0=1 and moves to state2 unless F4 was already loaded, in which case it publishes through state4.",
         "Requires typed 80016E18()==1; host card-info success or a loaded-slot bool is not authority."},
        {CardIoFeedbackTransitionKind::State1InfoTimeout80016E18,
         1,
         2,
         true,
         false,
         4,
         4,
         -3,
         true,
         0,
         false,
         "state1 SwCARD poll result 2 records info error/timeout as F0=-3, clears F4, and moves to publish state4.",
         "Return 2 may be gp+700 timeout; it must come from typed 80016E18 event/counter facts."},
        {CardIoFeedbackTransitionKind::State1InfoError80016E18,
         1,
         3,
         true,
         false,
         4,
         4,
         3,
         true,
         0,
         false,
         "state1 SwCARD poll result 3 records F0=3, clears F4, and moves to publish state4.",
         "Requires typed 80016E18()==3; host card-load failure is not authority."},
        {CardIoFeedbackTransitionKind::State1InfoNewCardReset80016E18_80017008,
         1,
         4,
         true,
         true,
         2,
         2,
         4,
         true,
         0,
         false,
         "state1 SwCARD poll result 4 records F0=4, drains/resets HwCARD events, polls 80017008, clears F4, and returns to load-submit state2.",
         "Requires typed 80016E18()==4 plus typed 80017008 HwCARD fact; reset side effects cannot be inferred from host card-change detection."},
        {CardIoFeedbackTransitionKind::State2LoadSubmit80017594,
         2,
         -1,
         false,
         false,
         3,
         3,
         0,
         false,
         -1,
         false,
         "state2 drains SwCARD events, submits card_load(0), seeds gp+700=300, and enters load polling state3.",
         "Card-load submission is a lower-card side effect; this catalog fact does not prove payload availability."},
        {CardIoFeedbackTransitionKind::State3LoadNoEvent80016E18,
         3,
         0,
         true,
         false,
         3,
         3,
         0,
         false,
         -1,
         false,
         "state3 SwCARD poll result 0 preserves EC/F0/F4 and keeps polling state3 after decrementing gp+700.",
         "Requires typed 80016E18 SwCARD event and counter facts; host pending status is not authority."},
        {CardIoFeedbackTransitionKind::State3LoadSuccess80016E18,
         3,
         1,
         true,
         false,
         4,
         4,
         0,
         false,
         1,
         false,
         "state3 SwCARD poll result 1 marks F4=1 and moves to publish state4; F0 remains the pending success code from state1.",
         "Requires typed 80016E18()==1 and prior pending-code authority; payload bytes are still separate authority."},
        {CardIoFeedbackTransitionKind::State3LoadTimeout80016E18,
         3,
         2,
         true,
         false,
         4,
         4,
         2,
         true,
         0,
         false,
         "state3 SwCARD poll result 2 records load error/timeout as F0=2, clears F4, and moves to publish state4.",
         "Return 2 may be gp+700 timeout; it must come from typed 80016E18 event/counter facts."},
        {CardIoFeedbackTransitionKind::State3LoadError80016E18,
         3,
         3,
         true,
         false,
         4,
         4,
         3,
         true,
         0,
         false,
         "state3 SwCARD poll result 3 records F0=3, clears F4, and moves to publish state4.",
         "Requires typed 80016E18()==3; host card-load failure is not authority."},
        {CardIoFeedbackTransitionKind::State3LoadNewCard80016E18,
         3,
         4,
         true,
         false,
         4,
         4,
         5,
         true,
         0,
         false,
         "state3 SwCARD poll result 4 records new-card-at-load as F0=5, clears F4, and moves to publish state4.",
         "Requires typed 80016E18()==4; host card-change detection is not authority."},
        {CardIoFeedbackTransitionKind::State4PublishPending80017594,
         4,
         -1,
         false,
         false,
         0,
         0,
         0,
         false,
         -1,
         true,
         "state4 publishes dword_800917F0 into dword_800917EC and returns that code while resetting E8 to state0.",
         "Only a previously gated pending F0 code is authoritative; callers must not invent EC from host card status."},
};

static constexpr LowerCardPayloadProducerSpec kLowerCardPayloadProducers[] = {
    {LowerCardPayloadProducerKind::State16ReadPayload80019D7C,
     kFn80019D7C,
     kFn800179B4,
     kFn80016EB8,
     kFn800164B4,
     kCardNameBuffer8007CBE8,
     kCardBlockBuffer8007ABE8,
     kCardPayloadBuffer8007ADE8,
     kCardPayloadOffset800179B4,
     kCardBlockBytes800179B4,
     kProgressBankBytes80092F10,
     0,
     0,
     1,
     1,
     true,
     true,
     false,
     false,
     false,
     false,
     "80019D7C state16 reads one lower-card block into 8007ABE8 and loads payload at 8007ADE8 into 80092F10.",
     "Requires typed 800179B4 read success through 80016EB8==1 and separately known payload bytes; host save booleans or old card buffers are not authority."},
    {LowerCardPayloadProducerKind::Case17ReadPayload80019D7C,
     kFn80019D7C,
     kFn800179B4,
     kFn80016EB8,
     kFn800164F8,
     kCardNameBuffer8007CBE8,
     kCardBlockBuffer8007ABE8,
     kCardPayloadBuffer8007ADE8,
     kCardPayloadOffset800179B4,
     kCardBlockBytes800179B4,
     kProgressBankBytes80092F10,
     kCardReadMetadata8007AE14,
     kCase17CardRowCount80019D7C,
     1,
     1,
     true,
     true,
     false,
     true,
     true,
     true,
     "80019D7C case17 reads each enabled card-list row into 8007ABE8, takes metadata at 8007AE14, and merges payload 8007ADE8 through 800164F8.",
     "Requires per-row enabled plus 800173A8 path/open/read, typed 80016EB8 event-source/read-success, close, payload, and metadata facts; row iteration alone, host card data, or old card buffers are not authority."},
};

static const LowerCardPayloadProducerSpec&
LowerCardPayloadState16Producer80019D7C()
{
    return kLowerCardPayloadProducers[0];
}

static const LowerCardPayloadProducerSpec&
LowerCardPayloadCase17Producer80019D7C()
{
    return kLowerCardPayloadProducers[1];
}

static bool IsCardPromptEvent(CardEventFrameId eventFrame)
{
    switch (eventFrame) {
    case CardEventFrameId::SaveUi:
    case CardEventFrameId::InsertCardPrompt:
    case CardEventFrameId::FormatPrompt:
    case CardEventFrameId::NoSpacePrompt:
    case CardEventFrameId::RenamePrompt:
    case CardEventFrameId::UnreadablePrompt:
    case CardEventFrameId::OverwritePrompt:
        return true;
    case CardEventFrameId::Unknown:
    case CardEventFrameId::CardInfo:
    case CardEventFrameId::HiScore:
    case CardEventFrameId::Save:
    case CardEventFrameId::Load:
    case CardEventFrameId::Replay:
    case CardEventFrameId::LoadReplayStart:
    case CardEventFrameId::SaveStart:
        return false;
    }
    return false;
}

static uint32_t PromptFlashArgSource(const CardStateRouteSpec& route)
{
    if (route.argAddress != 0) {
        return route.argAddress;
    }
    return kGpCardPromptArgOffset;
}

static void AppendRouteState800180D8(CardHandoffPlan& plan,
                                     const CardStateRouteSpec& route,
                                     int32_t state,
                                     CardMode800191E4 mode)
{
    AppendAction(plan, CardHandoffActionKind::RouteState800180D8,
                 kFn800180D8,
                 state,
                 static_cast<int32_t>(route.eventFrame),
                 static_cast<int32_t>(route.promptType),
                 static_cast<int32_t>(route.argAddress),
                 0,
                 0,
                 mode,
                 route.eventFrame);
    AppendAction(plan, CardHandoffActionKind::GateCurrentEvent800180D8,
                 kFn800180D8,
                 static_cast<int32_t>(kGpCardCurrentEventOffset),
                 static_cast<int32_t>(route.eventFrame),
                 static_cast<int32_t>(route.argAddress),
                 static_cast<int32_t>(kFn80017E58),
                 -1,
                 0,
                 mode,
                 route.eventFrame);
    AppendAction(plan, CardHandoffActionKind::StoreCurrentEvent800180D8,
                 kFn800180D8,
                 static_cast<int32_t>(kGpCardCurrentEventOffset),
                 static_cast<int32_t>(route.eventFrame),
                 0,
                 0,
                 0,
                 0,
                 mode,
                 route.eventFrame);
}

static bool IsListDirectionInput800181D0(int32_t inputMask)
{
    return inputMask == kInputNameUp800185D0 ||
           inputMask == kInputNameLeft800185D0 ||
           inputMask == kInputNameRight800185D0 ||
           inputMask == kInputNameDown800185D0;
}

static CardMode800191E4 ModeFrom80018E10ListState(int32_t state)
{
    if (state == 12) {
        return CardMode800191E4::Load;
    }
    if (state == 13) {
        return CardMode800191E4::Replay;
    }
    return CardMode800191E4::Unknown;
}

static CardEventFrameId EventFrom80018E10ListState(int32_t state)
{
    if (state == 12) {
        return CardEventFrameId::Load;
    }
    if (state == 13) {
        return CardEventFrameId::Replay;
    }
    return CardEventFrameId::Unknown;
}

static int32_t State6ResultFor80019D7CMode(CardMode800191E4 mode)
{
    if (mode == CardMode800191E4::Load) {
        return 12;
    }
    if (mode == CardMode800191E4::Replay) {
        return 13;
    }
    if (mode == CardMode800191E4::HiScore) {
        return 17;
    }
    return -1;
}

static int32_t SelectedArgFor80018E10ListConfirm(int32_t state)
{
    return state == 12 ? 2 : 1;
}

static CardHandoffActionKind InputCallbackKind(uint32_t callback)
{
    if (callback == kFn800185D0) {
        return CardHandoffActionKind::Call800185D0;
    }
    if (callback == kFn80018E10) {
        return CardHandoffActionKind::Call80018E10;
    }
    return CardHandoffActionKind::Gap;
}

static uint32_t CardInputCallbackForMode(CardMode800191E4 mode)
{
    switch (mode) {
    case CardMode800191E4::Save:
        return kFn800185D0;
    case CardMode800191E4::Load:
    case CardMode800191E4::Replay:
    case CardMode800191E4::HiScore:
        return kFn80018E10;
    case CardMode800191E4::Unknown:
        return 0;
    }
    return 0;
}

static uint32_t CardTickCallbackForMode(CardMode800191E4 mode)
{
    switch (mode) {
    case CardMode800191E4::Save:
        return kFn80019458;
    case CardMode800191E4::Load:
    case CardMode800191E4::Replay:
    case CardMode800191E4::HiScore:
        return kFn80019D7C;
    case CardMode800191E4::Unknown:
        return 0;
    }
    return 0;
}

static CardHandoffActionKind TickCallbackKind(uint32_t callback)
{
    if (callback == kFn80019458) {
        return CardHandoffActionKind::Call80019458;
    }
    if (callback == kFn80019D7C) {
        return CardHandoffActionKind::Call80019D7C;
    }
    return CardHandoffActionKind::Gap;
}

static void AppendSaveDirectoryBuild80019458(CardHandoffPlan& plan,
                                             int32_t state,
                                             uint32_t savePtr,
                                             bool entryCountKnown,
                                             int32_t entryCount)
{
    AppendAction(plan, CardHandoffActionKind::ScanSaveDirectory80017B08,
                 kFn80017B08,
                 state,
                 static_cast<int32_t>(kCardDirectoryRows8007A318),
                 static_cast<int32_t>(kSaveDirectoryBytes80019458));
    AppendAction(plan, CardHandoffActionKind::SnapshotSaveDirectory80017B18,
                 kFn80017B18,
                 static_cast<int32_t>(kCardDirectoryRows8007A318),
                 static_cast<int32_t>(kSaveDirectoryBytes80019458));
    AppendAction(plan, CardHandoffActionKind::ClearSaveListRows80025C44,
                 kFn80025C44,
                 static_cast<int32_t>(kCardRowTable8007A590),
                 static_cast<int32_t>(kSaveListClearBytes80019458));
    AppendAction(plan, CardHandoffActionKind::BuildSaveListRows80019458,
                 kFn80019458,
                 static_cast<int32_t>(kCardDirectoryRows8007A318),
                 static_cast<int32_t>(kCardRowTable8007A590),
                 static_cast<int32_t>(kGpCardFilenamePrefixOffset),
                 static_cast<int32_t>(savePtr + 1u),
                 0,
                 state);
    AppendAction(plan,
                 CardHandoffActionKind::GateDirectoryListSource80017B08,
                 kFn80017B08,
                 static_cast<int32_t>(kCardDirectoryRows8007A318),
                 static_cast<int32_t>(kCardRowTable8007A590),
                 state,
                 entryCountKnown ? entryCount : -1,
                 entryCountKnown ? 1 : 0,
                 state,
                 CardMode800191E4::Save);
}

static void AppendNamePreview80017FC4(CardHandoffPlan& plan,
                                      uint32_t src)
{
    AppendAction(plan, CardHandoffActionKind::RenderNamePreview80017FC4,
                 kFn80017FC4,
                 static_cast<int32_t>(kCardNameInputPreview8004926C),
                 static_cast<int32_t>(src));
}

static bool IsNameCursorInput800185D0(int32_t inputMask)
{
    return inputMask == kInputNameUp800185D0 ||
           inputMask == kInputNameLeft800185D0 ||
           inputMask == kInputNameRight800185D0 ||
           inputMask == kInputNameDown800185D0;
}

static bool IsCardEventPollResult(int32_t result)
{
    return result >= 1 && result <= 4;
}

static bool LowerCardPollResultMatches(const LowerCardResultSpec& spec,
                                       int32_t pollResult)
{
    return pollResult >= spec.minPollResult &&
           pollResult <= spec.maxPollResult;
}

static int32_t LowerCardFunctionResult(const LowerCardResultSpec& spec,
                                       int32_t pollResult)
{
    return spec.returnsPollResult ? pollResult : spec.fixedFunctionResult;
}

static bool IsStageSelectOutScene80025F6C(int32_t outScene)
{
    return (outScene >= 1 && outScene <= 6) || outScene == 8;
}

static bool IsKnownMainMenuResult80015788(int32_t result)
{
    switch (result) {
    case static_cast<int32_t>(MainMenuResult80015788::ContinueLoop):
    case static_cast<int32_t>(MainMenuResult80015788::HiScore):
    case static_cast<int32_t>(MainMenuResult80015788::Replay):
    case static_cast<int32_t>(MainMenuResult80015788::Practice):
    case static_cast<int32_t>(MainMenuResult80015788::StageSelect):
    case static_cast<int32_t>(MainMenuResult80015788::Load):
    case static_cast<int32_t>(MainMenuResult80015788::Exit):
    case static_cast<int32_t>(MainMenuResult80015788::Options):
        return true;
    default:
        return false;
    }
}

static bool IsLoadReplayListCardDriverState80018FB0(
    const CardDriverVisualRuntime80018FB0& runtime)
{
    return (runtime.state == 12 &&
            runtime.eventId == CardEventFrameId::Load) ||
           (runtime.state == 13 &&
            runtime.eventId == CardEventFrameId::Replay);
}

static bool IsKnownLoadReplayListInputRuntime800181D0(
    const LoadReplayListInputRuntime800181D0& runtime)
{
    if (!runtime.requestBound ||
        runtime.mode == CardMode800191E4::Unknown ||
        runtime.state != (runtime.mode == CardMode800191E4::Load ? 12 : 13) ||
        (runtime.mode != CardMode800191E4::Load &&
         runtime.mode != CardMode800191E4::Replay) ||
        runtime.step != kLoadReplayListGridStep800181D0 ||
        runtime.itemCount != kLoadReplayListItemCount800181D0 ||
        runtime.entryCount < 0 ||
        runtime.entryCount > kLoadReplayListSlotCount800181D0 ||
        runtime.selected < 0 ||
        runtime.selected >= kLoadReplayListItemCount800181D0) {
        return false;
    }
    for (int32_t index = 0;
         index < kLoadReplayListSlotCount800181D0;
         ++index) {
        const int16_t expected = index < runtime.entryCount ? 1 : 0;
        if (runtime.enabled[index] != expected) {
            return false;
        }
    }
    return runtime.enabled[kLoadReplayListTerminalRow800181D0] == 1;
}

static bool IsLoadReplayListItemEnabled800181D0(
    const LoadReplayListInputRuntime800181D0& runtime,
    int32_t index)
{
    return index >= 0 && index < runtime.itemCount &&
           runtime.enabled[index] == 1;
}

static int32_t ScanLoadReplayListEnabledItems800181D0(
    const LoadReplayListInputRuntime800181D0& runtime,
    int32_t start,
    int32_t step)
{
    int32_t candidate = start;
    for (int32_t guard = 0; guard < runtime.itemCount; ++guard) {
        if (IsLoadReplayListItemEnabled800181D0(runtime, candidate)) {
            return candidate;
        }
        candidate += step;
        if (candidate < 0) {
            candidate = kLoadReplayListTerminalRow800181D0;
        } else if (candidate >= runtime.itemCount) {
            candidate = 0;
        }
    }
    return runtime.selected;
}

static int32_t MoveLoadReplayListUp800181D0(
    const LoadReplayListInputRuntime800181D0& runtime)
{
    int32_t candidate =
        runtime.selected == kLoadReplayListTerminalRow800181D0
            ? kLoadReplayListTerminalRow800181D0 - 1
            : runtime.selected - runtime.step;
    if (candidate < 0) {
        candidate = kLoadReplayListTerminalRow800181D0 - 1;
    }
    return ScanLoadReplayListEnabledItems800181D0(runtime, candidate, -1);
}

static int32_t MoveLoadReplayListDown800181D0(
    const LoadReplayListInputRuntime800181D0& runtime)
{
    if (runtime.selected == kLoadReplayListTerminalRow800181D0) {
        return ScanLoadReplayListEnabledItems800181D0(runtime, 0, 1);
    }
    const int32_t candidate = runtime.selected + runtime.step;
    if (candidate >= kLoadReplayListTerminalRow800181D0 ||
        !IsLoadReplayListItemEnabled800181D0(runtime, candidate)) {
        return kLoadReplayListTerminalRow800181D0;
    }
    return candidate;
}

static int32_t MoveLoadReplayListRight800181D0(
    const LoadReplayListInputRuntime800181D0& runtime)
{
    return ScanLoadReplayListEnabledItems800181D0(
        runtime, (runtime.selected + 1) % runtime.itemCount, 1);
}

static int32_t MoveLoadReplayListLeft800181D0(
    const LoadReplayListInputRuntime800181D0& runtime)
{
    const int32_t candidate = runtime.selected > 0
        ? runtime.selected - 1
        : kLoadReplayListTerminalRow800181D0;
    return ScanLoadReplayListEnabledItems800181D0(runtime, candidate, -1);
}

static bool IsKnownLoadReplayCardDriverVisualRuntime80018FB0(
    const CardDriverVisualRuntime80018FB0& runtime)
{
    return runtime.known &&
        (IsLoadReplayListCardDriverState80018FB0(runtime) ||
         (runtime.state == 16 &&
          (runtime.eventId == CardEventFrameId::Load ||
           runtime.eventId == CardEventFrameId::Replay)) ||
         (runtime.state == 5 &&
          runtime.eventId == CardEventFrameId::InsertCardPrompt) ||
         (runtime.state == 23 &&
          (runtime.eventId == CardEventFrameId::Load ||
           runtime.eventId == CardEventFrameId::Replay ||
           runtime.eventId == CardEventFrameId::InsertCardPrompt))) &&
        runtime.blinkCounter8006ED18 <= 19u &&
        runtime.exitFrameStateArg0 >= 0 &&
        runtime.exitFrameStateArg0 <= 1 &&
        runtime.exitBlinkStateArg4 >= -1 &&
        runtime.exitBlinkStateArg4 <= 2 &&
        runtime.cardIoFlagArg8 >= 0 &&
        runtime.cardIoFlagArg8 <= 1 &&
        (!runtime.gp720Known || runtime.gp720 == 1);
}

static CardMode800191E4 LoadReplayModeForEvent800180D8(
    CardEventFrameId eventId)
{
    if (eventId == CardEventFrameId::Load) {
        return CardMode800191E4::Load;
    }
    if (eventId == CardEventFrameId::Replay) {
        return CardMode800191E4::Replay;
    }
    return CardMode800191E4::Unknown;
}

static bool IsLoadReplaySelectionFlashContext80017E6C(
    const CardDriverVisualRuntime80018FB0& runtime)
{
    if (runtime.gp720Known || runtime.gp720 != 0 ||
        runtime.cardIoFlagArg8 != 0) {
        return false;
    }
    if (IsLoadReplayListCardDriverState80018FB0(runtime) ||
        (runtime.state == 23 &&
         (runtime.eventId == CardEventFrameId::Load ||
          runtime.eventId == CardEventFrameId::Replay))) {
        return runtime.exitBlinkStateArg4 == 2;
    }
    return (runtime.state == 5 || runtime.state == 23) &&
        runtime.eventId == CardEventFrameId::InsertCardPrompt &&
        runtime.exitBlinkStateArg4 == 1;
}

static int32_t State16PromptArg4ForEvent80019D7C(
    CardEventFrameId eventId)
{
    if (eventId == CardEventFrameId::Load) {
        return 2;
    }
    if (eventId == CardEventFrameId::Replay) {
        return 1;
    }
    return 0;
}

static bool IsPreservedFinalFlashContext80017E6C(
    const CardDriverVisualRuntime80018FB0& runtime)
{
    if (runtime.state != 23 ||
        runtime.phase != CardDriverVisualPhase80018FB0::FinalFlash80017E6C ||
        runtime.exitFrameStateArg0 != 1 || runtime.cardIoFlagArg8 != 0) {
        return false;
    }
    if (runtime.gp720Known) {
        return runtime.gp720 == 1 &&
            (runtime.eventId == CardEventFrameId::Load ||
             runtime.eventId == CardEventFrameId::Replay) &&
            runtime.exitBlinkStateArg4 ==
                State16PromptArg4ForEvent80019D7C(runtime.eventId);
    }
    if (runtime.gp720 != 0) {
        return false;
    }
    if (runtime.eventId == CardEventFrameId::InsertCardPrompt) {
        return runtime.exitBlinkStateArg4 == 1;
    }
    return (runtime.eventId == CardEventFrameId::Load ||
            runtime.eventId == CardEventFrameId::Replay) &&
        runtime.exitBlinkStateArg4 == 2;
}

static void AdvanceCardDriverBlinkCounter80018FB0(
    CardDriverVisualRuntime80018FB0& runtime)
{
    if (runtime.blinkCounter8006ED18 == 0u) {
        runtime.blinkCounter8006ED18 = 1u;
        runtime.exitFrameStateArg0 =
            runtime.exitFrameStateArg0 == 0 ? 1 : 0;
    } else if (runtime.blinkCounter8006ED18 < 19u) {
        ++runtime.blinkCounter8006ED18;
    } else {
        runtime.blinkCounter8006ED18 = 0u;
    }
}

} // namespace

bool InitLoadReplayCardDriverVisualRuntime80018FB0(
    CardMode800191E4 mode,
    CardDriverVisualRuntime80018FB0* out)
{
    if (out == nullptr) {
        return false;
    }
    *out = CardDriverVisualRuntime80018FB0{};
    if (mode != CardMode800191E4::Load &&
        mode != CardMode800191E4::Replay) {
        return false;
    }

    out->known = true;
    out->state = mode == CardMode800191E4::Load ? 12 : 13;
    out->eventId = mode == CardMode800191E4::Load
        ? CardEventFrameId::Load
        : CardEventFrameId::Replay;
    out->phase = CardDriverVisualPhase80018FB0::ListIdle;
    out->blinkCounter8006ED18 = 0u;
    out->promptFlashFramesRemaining80017E6C = 0u;
    out->exitFrameStateArg0 = 1;
    out->exitBlinkStateArg4 = 0;
    out->cardIoFlagArg8 = 0;
    return true;
}

bool InitLoadReplayListInputRuntime800181D0(
    CardMode800191E4 mode,
    int32_t entryCount,
    LoadReplayListInputRuntime800181D0* out)
{
    if (out == nullptr) {
        return false;
    }
    *out = LoadReplayListInputRuntime800181D0{};
    if ((mode != CardMode800191E4::Load &&
         mode != CardMode800191E4::Replay) ||
        entryCount < 0 ||
        entryCount > kLoadReplayListSlotCount800181D0) {
        return false;
    }

    out->requestBound = true;
    out->mode = mode;
    out->state = mode == CardMode800191E4::Load ? 12 : 13;
    out->step = kLoadReplayListGridStep800181D0;
    out->itemCount = kLoadReplayListItemCount800181D0;
    out->entryCount = entryCount;
    out->selected = entryCount == 0
        ? kLoadReplayListTerminalRow800181D0
        : 0;
    for (int32_t index = 0; index < entryCount; ++index) {
        out->enabled[index] = 1;
    }
    out->enabled[kLoadReplayListTerminalRow800181D0] = 1;
    return IsKnownLoadReplayListInputRuntime800181D0(*out);
}

LoadReplayListInputResult800181D0 TickLoadReplayListInput800181D0(
    LoadReplayListInputRuntime800181D0* runtime,
    int32_t inputMask)
{
    LoadReplayListInputResult800181D0 out{};
    out.inputMask = inputMask;
    if (runtime == nullptr ||
        !IsKnownLoadReplayListInputRuntime800181D0(*runtime)) {
        return out;
    }

    out.accepted = true;
    out.action = LoadReplayListInputAction800181D0::NoChange;
    out.previousSelected = runtime->selected;
    out.selected = runtime->selected;
    out.eventId = runtime->mode == CardMode800191E4::Load
        ? CardEventFrameId::Load
        : CardEventFrameId::Replay;
    out.nextState = runtime->state;

    if (inputMask == kInputCross800185D0) {
        if (runtime->selected == kLoadReplayListTerminalRow800181D0) {
            out.action = LoadReplayListInputAction800181D0::SelectExit;
            out.listResult = 2;
            out.playSfx = true;
            out.sfx = 0x20;
            out.promptArg2 = 2;
            out.promptArg3 = 0;
            out.nextState = 23;
        } else if (runtime->selected < runtime->entryCount &&
                   IsLoadReplayListItemEnabled800181D0(
                       *runtime, runtime->selected)) {
            out.action = LoadReplayListInputAction800181D0::SelectEntry;
            out.listResult = 1;
            out.playSfx = true;
            out.sfx = 0x20;
            out.promptArg2 = runtime->mode == CardMode800191E4::Load ? 2 : 1;
            out.promptArg3 = 1;
            out.nextState = 16;
        }
        return out;
    }

    if (!IsListDirectionInput800181D0(inputMask)) {
        return out;
    }

    out.action = LoadReplayListInputAction800181D0::MoveSelection;
    out.playSfx = true;
    out.sfx = 0x1000;
    switch (inputMask) {
    case kInputNameUp800185D0:
        runtime->selected = MoveLoadReplayListUp800181D0(*runtime);
        break;
    case kInputNameRight800185D0:
        runtime->selected = MoveLoadReplayListRight800181D0(*runtime);
        break;
    case kInputNameLeft800185D0:
        runtime->selected = MoveLoadReplayListLeft800181D0(*runtime);
        break;
    case kInputNameDown800185D0:
        runtime->selected = MoveLoadReplayListDown800181D0(*runtime);
        break;
    default:
        break;
    }
    out.selected = runtime->selected;
    return out;
}

bool CommitLoadReplaySelectedRowIdentity800181D0(
    const LoadReplayDirectoryTypedCarrier80019D7C& directory,
    const LoadReplayListInputResult800181D0& inputResult,
    LoadReplaySelectedRowIdentity800181D0* out)
{
    if (out == nullptr) {
        return false;
    }
    *out = LoadReplaySelectedRowIdentity800181D0{};

    LoadReplayDirectoryTypedCarrier80019D7C normalized = directory;
    if (!ValidateLoadReplayDirectoryTypedCarrier80019D7C(normalized) ||
        !inputResult.accepted ||
        inputResult.action !=
            LoadReplayListInputAction800181D0::SelectEntry ||
        inputResult.listResult != 1 ||
        inputResult.inputMask != kInputCross800185D0 ||
        inputResult.previousSelected != inputResult.selected ||
        inputResult.selected < 0 ||
        inputResult.selected >= normalized.entryCount ||
        inputResult.nextState != 16 ||
        inputResult.promptArg3 != 1) {
        return false;
    }

    const CardEventFrameId expectedEvent =
        normalized.mode == CardMode800191E4::Load
            ? CardEventFrameId::Load
            : CardEventFrameId::Replay;
    const int32_t expectedPromptArg2 =
        normalized.mode == CardMode800191E4::Load ? 2 : 1;
    if (inputResult.eventId != expectedEvent ||
        inputResult.promptArg2 != expectedPromptArg2) {
        return false;
    }

    const LoadReplayDirectoryRow80019D7C& row =
        normalized.rows[inputResult.selected];
    if (!row.rowNameKnown8007A590 ||
        row.rowName8007A590[0] == '\0') {
        return false;
    }

    out->mode = normalized.mode;
    out->selectedRow = inputResult.selected;
    out->blockIndexKnown = row.blockIndexKnown;
    out->blockIndex = row.blockIndex;
    out->gp716Known = true;
    out->gp716 = 1;
    out->nameBuffer8007CBE8Known = true;
    CopyLoadReplayDirectoryTitle80019D7C(
        out->nameBuffer8007CBE8, row.rowName8007A590);
    out->committed = true;
    return true;
}

bool TickCardDriverVisualRuntime80018FB0(
    CardDriverVisualRuntime80018FB0* runtime)
{
    const bool listIdle = runtime != nullptr &&
        IsLoadReplayListCardDriverState80018FB0(*runtime) &&
        runtime->phase == CardDriverVisualPhase80018FB0::ListIdle;
    const bool state5PromptIdle = runtime != nullptr &&
        runtime->state == 5 &&
        runtime->eventId == CardEventFrameId::InsertCardPrompt &&
        runtime->phase ==
            CardDriverVisualPhase80018FB0::State5PromptIdle800180D8;
    if (runtime == nullptr ||
        !IsKnownLoadReplayCardDriverVisualRuntime80018FB0(*runtime) ||
        (!listIdle && !state5PromptIdle) ||
        runtime->promptFlashFramesRemaining80017E6C != 0u ||
        runtime->exitBlinkStateArg4 != 0 ||
        runtime->cardIoFlagArg8 != 0 || runtime->gp720Known ||
        runtime->gp720 != 0) {
        return false;
    }

    AdvanceCardDriverBlinkCounter80018FB0(*runtime);
    return true;
}

bool BeginListEntrySelectionFlash80017E6C(
    CardDriverVisualRuntime80018FB0* runtime,
    const LoadReplayListInputResult800181D0& inputResult)
{
    if (runtime == nullptr ||
        !IsKnownLoadReplayCardDriverVisualRuntime80018FB0(*runtime) ||
        !IsLoadReplayListCardDriverState80018FB0(*runtime) ||
        runtime->phase != CardDriverVisualPhase80018FB0::ListIdle ||
        runtime->promptFlashFramesRemaining80017E6C != 0u ||
        runtime->exitBlinkStateArg4 != 0 ||
        runtime->cardIoFlagArg8 != 0 ||
        !inputResult.accepted ||
        inputResult.action !=
            LoadReplayListInputAction800181D0::SelectEntry ||
        inputResult.listResult != 1 || inputResult.nextState != 16 ||
        inputResult.promptArg3 != 1 ||
        inputResult.eventId != runtime->eventId ||
        inputResult.promptArg2 != (runtime->state == 12 ? 2 : 1)) {
        return false;
    }

    runtime->phase =
        CardDriverVisualPhase80018FB0::EntrySelectionFlash80017E6C;
    runtime->promptFlashFramesRemaining80017E6C = 20u;
    runtime->exitFrameStateArg0 = 1;
    runtime->exitBlinkStateArg4 = inputResult.promptArg2;
    runtime->cardIoFlagArg8 = inputResult.promptArg3;
    return true;
}

CardDriverEntryTickResult80017E6C
TickListEntrySelectionFlash80017E6C(
    CardDriverVisualRuntime80018FB0* runtime)
{
    if (runtime == nullptr ||
        !IsKnownLoadReplayCardDriverVisualRuntime80018FB0(*runtime) ||
        !IsLoadReplayListCardDriverState80018FB0(*runtime) ||
        runtime->phase !=
            CardDriverVisualPhase80018FB0::EntrySelectionFlash80017E6C ||
        runtime->promptFlashFramesRemaining80017E6C == 0u ||
        runtime->promptFlashFramesRemaining80017E6C > 20u ||
        runtime->exitBlinkStateArg4 != (runtime->state == 12 ? 2 : 1) ||
        runtime->cardIoFlagArg8 != 1) {
        return CardDriverEntryTickResult80017E6C::Rejected;
    }
    if (runtime->promptFlashFramesRemaining80017E6C > 1u) {
        --runtime->promptFlashFramesRemaining80017E6C;
        return CardDriverEntryTickResult80017E6C::RenderSelectionFlash;
    }

    runtime->promptFlashFramesRemaining80017E6C = 0u;
    runtime->state = 16;
    runtime->phase =
        CardDriverVisualPhase80018FB0::State16AwaitingPayload80019D7C;
    return CardDriverEntryTickResult80017E6C::EnterState16;
}

bool IsLoadReplayState16AwaitingPayload80019D7C(
    const CardDriverVisualRuntime80018FB0& runtime)
{
    return IsKnownLoadReplayCardDriverVisualRuntime80018FB0(runtime) &&
        runtime.state == 16 &&
        runtime.phase ==
            CardDriverVisualPhase80018FB0::
                State16AwaitingPayload80019D7C &&
        runtime.promptFlashFramesRemaining80017E6C == 0u &&
        runtime.exitFrameStateArg0 == 1 &&
        runtime.exitBlinkStateArg4 ==
            State16PromptArg4ForEvent80019D7C(runtime.eventId) &&
        runtime.cardIoFlagArg8 == 1 && !runtime.gp720Known &&
        runtime.gp720 == 0;
}

bool IsRequestBoundLoadReplayState16CardIoResult80017594(
    const LoadReplayState16CardIoResultCarrier80017594& carrier,
    CardMode800191E4 expectedMode,
    int32_t expectedBlock)
{
    return carrier.known && carrier.requestBound &&
        carrier.source ==
            LoadReplayState16CardIoResultSource80017594::
                RuntimeCardEventProducer &&
        carrier.producerWired80017594 && !carrier.incomplete &&
        (expectedMode == CardMode800191E4::Load ||
         expectedMode == CardMode800191E4::Replay) &&
        carrier.mode == expectedMode && carrier.stateKnown &&
        carrier.state == 16 && carrier.selectedBlockKnown &&
        expectedBlock >= 0 &&
        carrier.selectedBlock == expectedBlock &&
        carrier.ioResultKnown80017594 &&
        carrier.ioResult80017594 ==
            static_cast<int32_t>(CardIoCode80017594::Timeout);
}

bool ApplyLoadReplayState16CardIoResult80019D7C(
    CardDriverVisualRuntime80018FB0* runtime,
    const LoadReplayState16CardIoResultCarrier80017594& carrier,
    CardMode800191E4 expectedMode,
    int32_t expectedBlock)
{
    if (runtime == nullptr ||
        !IsLoadReplayState16AwaitingPayload80019D7C(*runtime) ||
        LoadReplayModeForEvent800180D8(runtime->eventId) != expectedMode ||
        !IsRequestBoundLoadReplayState16CardIoResult80017594(
            carrier, expectedMode, expectedBlock)) {
        return false;
    }

    runtime->state = 5;
    runtime->eventId = CardEventFrameId::InsertCardPrompt;
    runtime->phase =
        CardDriverVisualPhase80018FB0::State5PromptIdle800180D8;
    runtime->blinkCounter8006ED18 = 0u;
    runtime->promptFlashFramesRemaining80017E6C = 0u;
    runtime->exitFrameStateArg0 = 1;
    runtime->exitBlinkStateArg4 = 0;
    runtime->cardIoFlagArg8 = 0;
    runtime->gp720Known = false;
    runtime->gp720 = 0;
    return true;
}

bool IsLoadReplayState5PromptIdle800180D8(
    const CardDriverVisualRuntime80018FB0& runtime)
{
    return IsKnownLoadReplayCardDriverVisualRuntime80018FB0(runtime) &&
        runtime.state == 5 &&
        runtime.eventId == CardEventFrameId::InsertCardPrompt &&
        runtime.phase ==
            CardDriverVisualPhase80018FB0::State5PromptIdle800180D8 &&
        runtime.promptFlashFramesRemaining80017E6C == 0u &&
        runtime.exitBlinkStateArg4 == 0 && runtime.cardIoFlagArg8 == 0 &&
        !runtime.gp720Known && runtime.gp720 == 0;
}

bool BeginLoadReplayState5PromptFlash80017E6C(
    CardDriverVisualRuntime80018FB0* runtime,
    int32_t inputMask)
{
    if (runtime == nullptr || inputMask != kInputCross800185D0 ||
        !IsLoadReplayState5PromptIdle800180D8(*runtime)) {
        return false;
    }

    runtime->phase =
        CardDriverVisualPhase80018FB0::SelectionFlash80017E6C;
    runtime->promptFlashFramesRemaining80017E6C = 20u;
    runtime->exitFrameStateArg0 = 1;
    runtime->exitBlinkStateArg4 = 1;
    runtime->cardIoFlagArg8 = 0;
    return true;
}

bool BeginLoadReplayState16Completion80019D7C(
    CardDriverVisualRuntime80018FB0* runtime)
{
    if (runtime == nullptr ||
        !IsLoadReplayState16AwaitingPayload80019D7C(*runtime)) {
        return false;
    }

    runtime->gp720Known = true;
    runtime->gp720 = 1;
    runtime->state = 23;
    runtime->phase =
        CardDriverVisualPhase80018FB0::TerminalFrame80018FB0;
    return true;
}

bool BeginLoadReplayExitPromptFlash80017E6C(
    CardDriverVisualRuntime80018FB0* runtime)
{
    if (runtime == nullptr ||
        !IsKnownLoadReplayCardDriverVisualRuntime80018FB0(*runtime) ||
        !IsLoadReplayListCardDriverState80018FB0(*runtime) ||
        runtime->phase != CardDriverVisualPhase80018FB0::ListIdle ||
        runtime->promptFlashFramesRemaining80017E6C != 0u ||
        runtime->exitBlinkStateArg4 != 0 ||
        runtime->cardIoFlagArg8 != 0 || runtime->gp720Known ||
        runtime->gp720 != 0) {
        return false;
    }

    runtime->phase =
        CardDriverVisualPhase80018FB0::SelectionFlash80017E6C;
    runtime->promptFlashFramesRemaining80017E6C = 20u;
    runtime->exitFrameStateArg0 = 1;
    runtime->exitBlinkStateArg4 = 2;
    runtime->cardIoFlagArg8 = 0;
    return true;
}

CardDriverExitTickResult80017E6C TickLoadReplayExitPromptFlash80017E6C(
    CardDriverVisualRuntime80018FB0* runtime)
{
    if (runtime == nullptr ||
        !IsKnownLoadReplayCardDriverVisualRuntime80018FB0(*runtime)) {
        return CardDriverExitTickResult80017E6C::Rejected;
    }

    switch (runtime->phase) {
    case CardDriverVisualPhase80018FB0::SelectionFlash80017E6C:
        if (!IsLoadReplaySelectionFlashContext80017E6C(*runtime)) {
            return CardDriverExitTickResult80017E6C::Rejected;
        }
        if (runtime->promptFlashFramesRemaining80017E6C == 0u ||
            runtime->promptFlashFramesRemaining80017E6C > 20u) {
            return CardDriverExitTickResult80017E6C::Rejected;
        }
        if (runtime->promptFlashFramesRemaining80017E6C > 1u) {
            --runtime->promptFlashFramesRemaining80017E6C;
            return CardDriverExitTickResult80017E6C::RenderSelectionFlash;
        }
        runtime->promptFlashFramesRemaining80017E6C = 0u;
        runtime->state = 23;
        runtime->phase =
            CardDriverVisualPhase80018FB0::TerminalFrame80018FB0;
        AdvanceCardDriverBlinkCounter80018FB0(*runtime);
        return CardDriverExitTickResult80017E6C::RenderTerminalFrame;

    case CardDriverVisualPhase80018FB0::TerminalFrame80018FB0:
        if (runtime->promptFlashFramesRemaining80017E6C != 0u ||
            runtime->state != 23) {
            return CardDriverExitTickResult80017E6C::Rejected;
        }
        if (runtime->gp720Known) {
            if (runtime->gp720 != 1 || runtime->exitFrameStateArg0 != 1 ||
                runtime->exitBlinkStateArg4 !=
                    State16PromptArg4ForEvent80019D7C(runtime->eventId) ||
                runtime->cardIoFlagArg8 != 1) {
                return CardDriverExitTickResult80017E6C::Rejected;
            }
        } else if (!IsLoadReplaySelectionFlashContext80017E6C(*runtime)) {
            return CardDriverExitTickResult80017E6C::Rejected;
        }
        runtime->phase =
            CardDriverVisualPhase80018FB0::FinalFlash80017E6C;
        runtime->promptFlashFramesRemaining80017E6C = 20u;
        runtime->exitFrameStateArg0 = 1;
        // 80017E6C receives selected=-1 here. Its signed-negative branch
        // skips the ctx+4 store, so the preceding selection value survives.
        runtime->cardIoFlagArg8 = 0;
        return CardDriverExitTickResult80017E6C::RenderFinalFlash;

    case CardDriverVisualPhase80018FB0::FinalFlash80017E6C:
        if (!IsPreservedFinalFlashContext80017E6C(*runtime) ||
            runtime->promptFlashFramesRemaining80017E6C == 0u ||
            runtime->promptFlashFramesRemaining80017E6C > 20u) {
            return CardDriverExitTickResult80017E6C::Rejected;
        }
        if (runtime->promptFlashFramesRemaining80017E6C > 1u) {
            --runtime->promptFlashFramesRemaining80017E6C;
            return CardDriverExitTickResult80017E6C::RenderFinalFlash;
        }
        runtime->promptFlashFramesRemaining80017E6C = 0u;
        runtime->phase = CardDriverVisualPhase80018FB0::Complete;
        return CardDriverExitTickResult80017E6C::ReturnScene0;

    default:
        return CardDriverExitTickResult80017E6C::Rejected;
    }
}

bool RuntimeCutoverAllowed()
{
    return false;
}

static bool PublishLoadReplayDirectoryTypedCarrier80019D7CWithSource(
    const LoadReplayDirectoryTypedCarrier80019D7C& carrier,
    LoadReplayDirectoryTypedCarrierSource80019D7C source)
{
    LoadReplayDirectoryTypedCarrier80019D7C normalized = carrier;
    normalized.source = source;
    normalized.producerWired80017B08_80017B18_80019D7C =
        source ==
        LoadReplayDirectoryTypedCarrierSource80019D7C::
            RuntimeDirectoryProducer;
    const bool valid =
        ValidateLoadReplayDirectoryTypedCarrier80019D7C(normalized);
    if (!valid) {
        s_loadReplayDirectoryTypedCarrier80019D7C =
            LoadReplayDirectoryTypedCarrier80019D7C{};
        return false;
    }
    s_loadReplayDirectoryTypedCarrier80019D7C = normalized;
    return true;
}

bool PublishLoadReplayDirectoryTypedCarrier80019D7C(
    const LoadReplayDirectoryTypedCarrier80019D7C& carrier)
{
    return PublishLoadReplayDirectoryTypedCarrier80019D7CWithSource(
        carrier,
        LoadReplayDirectoryTypedCarrierSource80019D7C::Unknown);
}

bool PublishRuntimeLoadReplayDirectoryTypedCarrier80019D7C(
    const LoadReplayDirectoryTypedCarrier80019D7C& carrier)
{
    return PublishLoadReplayDirectoryTypedCarrier80019D7CWithSource(
        carrier,
        LoadReplayDirectoryTypedCarrierSource80019D7C::
            RuntimeDirectoryProducer);
}

bool GetLoadReplayDirectoryTypedCarrier80019D7C(
    CardMode800191E4 mode,
    LoadReplayDirectoryTypedCarrier80019D7C* out)
{
    if (out == nullptr) {
        return false;
    }
    *out = LoadReplayDirectoryTypedCarrier80019D7C{};
    LoadReplayDirectoryTypedCarrier80019D7C carrier =
        s_loadReplayDirectoryTypedCarrier80019D7C;
    if (carrier.mode != mode ||
        !ValidateLoadReplayDirectoryTypedCarrier80019D7C(carrier)) {
        return false;
    }
    *out = carrier;
    return true;
}

void ClearLoadReplayDirectoryTypedCarrier80019D7C()
{
    s_loadReplayDirectoryTypedCarrier80019D7C =
        LoadReplayDirectoryTypedCarrier80019D7C{};
}

bool BuildRuntimeLoadReplayDirectoryCarrierFromScanFacts80019D7C(
    const LoadReplayDirectoryScanFacts80019D7C& facts,
    LoadReplayDirectoryTypedCarrier80019D7C* out)
{
    if (out == nullptr) {
        return false;
    }
    *out = LoadReplayDirectoryTypedCarrier80019D7C{};
    out->known = facts.known;
    out->mode = facts.mode;
    out->entryCountKnown = facts.entryCountKnown;
    out->entryCount = facts.entryCount;
    out->incomplete = true;

    if (!facts.known ||
        !IsLoadReplayDirectoryMode80019D7C(facts.mode) ||
        !facts.directoryRowsKnown80017B08 ||
        !facts.snapshotKnown80017B18 ||
        !facts.listRowsBuilt80019D7C ||
        !facts.entryCountKnown ||
        facts.entryCount < 0 ||
        facts.entryCount >
            static_cast<int32_t>(kSaveListRowCount80019458)) {
        return false;
    }

    for (int32_t row = 0; row < facts.entryCount; ++row) {
        out->rows[row] = facts.rows[row];
    }
    out->source =
        LoadReplayDirectoryTypedCarrierSource80019D7C::
            RuntimeDirectoryProducer;
    out->producerWired80017B08_80017B18_80019D7C = true;
    return ValidateLoadReplayDirectoryTypedCarrier80019D7C(*out);
}

bool TryMapReplayScene800161A8(int32_t sel, int32_t* scene)
{
    return PrSS0StageProgressBankDirect::TryMapReplayScene800161A8(sel, scene);
}

bool BuildHiScoreCellSpec80019284(uint32_t cell,
                                  HiScoreCellSpec80019284* out)
{
    if (cell >= kHiScoreCellCount80019284 || out == nullptr) {
        return false;
    }
    const uint32_t row = cell / kHiScoreColumnCount80019284;
    const uint32_t column = cell % kHiScoreColumnCount80019284;

    HiScoreCellSpec80019284 spec{};
    spec.cell = cell;
    spec.row = row;
    spec.column = column;
    spec.scoreOffset =
        kHiScoreScoreBase80019284 + row * 64u + column * 16u;
    spec.nameOffset =
        kHiScoreStatusPrefixSize80019284 + row * 64u + column * 16u;
    spec.tableRecordOffset =
        kHiScoreTableHeaderSize80019284 +
        cell * kHiScoreRecordStride80019284;
    spec.tableRecordAddress =
        kHiScoreTable80049278 + spec.tableRecordOffset;
    *out = spec;
    return true;
}

bool BuildCase17RowSpec80019D7C(uint32_t row,
                                Case17RowSpec80019D7C* out)
{
    if (row >= kCase17CardRowCount80019D7C || out == nullptr) {
        return false;
    }
    Case17RowSpec80019D7C spec{};
    spec.row = row;
    spec.rowAddress =
        kCardRowTable8007A590 + row * kCase17CardRowStride80019D7C;
    spec.enabledAddress =
        spec.rowAddress + kCase17RowEnabledOffset80019D7C;
    spec.metadataAddress =
        spec.rowAddress + kCase17RowMetadataOffset80019D7C;
    *out = spec;
    return true;
}

uint32_t KnownCardStateRouteSpecCount()
{
    return static_cast<uint32_t>(sizeof(kCardStateRoutes) /
                                sizeof(kCardStateRoutes[0]));
}

const CardStateRouteSpec& KnownCardStateRouteSpecAt(uint32_t index)
{
    return kCardStateRoutes[index];
}

const CardStateRouteSpec* FindCardStateRouteSpec(int32_t state,
                                                 CardMode800191E4 mode)
{
    const CardStateRouteSpec* fallback = nullptr;
    for (uint32_t i = 0; i < KnownCardStateRouteSpecCount(); ++i) {
        const CardStateRouteSpec& spec = kCardStateRoutes[i];
        if (spec.state != state) {
            continue;
        }
        if (spec.mode == mode) {
            return &spec;
        }
        if (spec.mode == CardMode800191E4::Unknown && fallback == nullptr) {
            fallback = &spec;
        }
    }
    return fallback;
}

uint32_t KnownCardIoRouteSpecCount()
{
    return static_cast<uint32_t>(sizeof(kCardIoRoutes) /
                                sizeof(kCardIoRoutes[0]));
}

const CardIoRouteSpec& KnownCardIoRouteSpecAt(uint32_t index)
{
    return kCardIoRoutes[index];
}

const CardIoRouteSpec* FindCardIoRouteSpec(CardIoCode80017594 ioCode,
                                           CardMode800191E4 mode)
{
    const CardIoRouteSpec* fallback = nullptr;
    for (uint32_t i = 0; i < KnownCardIoRouteSpecCount(); ++i) {
        const CardIoRouteSpec& spec = kCardIoRoutes[i];
        if (spec.ioCode != ioCode) {
            continue;
        }
        if (spec.mode == mode) {
            return &spec;
        }
        if (spec.mode == CardMode800191E4::Unknown && fallback == nullptr) {
            fallback = &spec;
        }
    }
    return fallback;
}

uint32_t KnownCardEventPollSourceSpecCount()
{
    return static_cast<uint32_t>(sizeof(kCardEventPollSources) /
                                sizeof(kCardEventPollSources[0]));
}

const CardEventPollSourceSpec& KnownCardEventPollSourceSpecAt(uint32_t index)
{
    if (index >= KnownCardEventPollSourceSpecCount()) {
        index = KnownCardEventPollSourceSpecCount() - 1u;
    }
    return kCardEventPollSources[index];
}

uint32_t KnownLowerCardResultSpecCount()
{
    return static_cast<uint32_t>(sizeof(kLowerCardResults) /
                                sizeof(kLowerCardResults[0]));
}

const LowerCardResultSpec& KnownLowerCardResultSpecAt(uint32_t index)
{
    if (index >= KnownLowerCardResultSpecCount()) {
        index = KnownLowerCardResultSpecCount() - 1u;
    }
    return kLowerCardResults[index];
}

uint32_t KnownCardIoFeedbackTransitionSpecCount()
{
    return static_cast<uint32_t>(
        sizeof(kCardIoFeedbackTransitions80017594) /
        sizeof(kCardIoFeedbackTransitions80017594[0]));
}

const CardIoFeedbackTransitionSpec80017594&
KnownCardIoFeedbackTransitionSpecAt(uint32_t index)
{
    if (index >= KnownCardIoFeedbackTransitionSpecCount()) {
        index = KnownCardIoFeedbackTransitionSpecCount() - 1u;
    }
    return kCardIoFeedbackTransitions80017594[index];
}

uint32_t KnownLowerCardPayloadProducerSpecCount()
{
    return static_cast<uint32_t>(sizeof(kLowerCardPayloadProducers) /
                                sizeof(kLowerCardPayloadProducers[0]));
}

const LowerCardPayloadProducerSpec& KnownLowerCardPayloadProducerSpecAt(
    uint32_t index)
{
    if (index >= KnownLowerCardPayloadProducerSpecCount()) {
        index = KnownLowerCardPayloadProducerSpecCount() - 1u;
    }
    return kLowerCardPayloadProducers[index];
}

CardHandoffPlan BuildGenericSwitchPrelude80015788Plan(int32_t prevScene)
{
    CardHandoffPlan plan = MakePlan("GenericSwitchPrelude80015788");
    plan.outcome = HandoffOutcome::ContinueLoop;
    plan.blockedByP0Gap = true;

    AppendAction(plan, CardHandoffActionKind::Call80026FA4,
                 kFn80026FA4, prevScene);
    AppendAction(plan, CardHandoffActionKind::PlayCue80026EF8_94410,
                 kFn80026EF8, static_cast<int32_t>(kCueGlobal94410));
    AppendAction(plan, CardHandoffActionKind::Flush80026ECC,
                 kFn80026ECC);
    AppendAction(plan, CardHandoffActionKind::Bootstrap80015590,
                 kFn80015590, prevScene);
    AppendAction(plan, CardHandoffActionKind::Call80026B94,
                 kFn80026B94,
                 static_cast<int32_t>(kDispatcherEventMainMenu),
                 static_cast<int32_t>(kWord800916D0));
    AppendAction(plan, CardHandoffActionKind::Gap, kFn80015788,
                 prevScene);
    return plan;
}

CardHandoffPlan BuildMainMenuResult80015788Plan(int32_t result)
{
    CardHandoffPlan plan = MakePlan("MainMenuResult80015788");
    plan.blockedByP0Gap = true;
    AppendAction(plan, CardHandoffActionKind::Call80026B94,
                 kFn80026B94,
                 static_cast<int32_t>(kDispatcherEventMainMenu),
                 static_cast<int32_t>(kWord800916D0),
                 0,
                 0,
                 result);
    AppendAction(plan,
                 CardHandoffActionKind::GateMainMenuResult80015788,
                 kFn80015788,
                 result,
                 IsKnownMainMenuResult80015788(result) ? 1 : 0,
                 static_cast<int32_t>(kDispatcherEventMainMenu),
                 static_cast<int32_t>(kWord800916D0));

    switch (static_cast<MainMenuResult80015788>(result)) {
    case MainMenuResult80015788::ContinueLoop:
        plan.outcome = HandoffOutcome::ContinueLoop;
        break;
    case MainMenuResult80015788::HiScore:
        plan.outcome = HandoffOutcome::Gap;
        AppendPlan(plan, BuildHiScore80019414Plan());
        break;
    case MainMenuResult80015788::Replay:
        plan.outcome = HandoffOutcome::Gap;
        AppendPlan(plan, BuildReplayHandoff80015788Plan(false, -1));
        break;
    case MainMenuResult80015788::Practice:
        plan.outcome = HandoffOutcome::ContinueLoop;
        AppendAction(plan, CardHandoffActionKind::Call8002776CPractice,
                     kFn8002776C,
                     static_cast<int32_t>(kScene0WorkAddress));
        AppendAction(plan, CardHandoffActionKind::Gap, kFn8002776C,
                     result);
        break;
    case MainMenuResult80015788::StageSelect:
        plan.outcome = HandoffOutcome::Gap;
        AppendPlan(plan, BuildStageSelectHandoffPlan(false, 0, false, 0));
        break;
    case MainMenuResult80015788::Load:
        plan.outcome = HandoffOutcome::PendingCardLoop;
        AppendPlan(plan, BuildLoadFlow800193B0Plan());
        break;
    case MainMenuResult80015788::Exit:
        plan.outcome = HandoffOutcome::ReturnScene0;
        AppendAction(plan, CardHandoffActionKind::ReturnScene,
                     kFn80015788, 0, 0, 0, 0, result, 0);
        break;
    case MainMenuResult80015788::Options:
        plan.outcome = HandoffOutcome::ContinueLoop;
        AppendAction(plan, CardHandoffActionKind::Call80026B94,
                     kFn80026B94,
                     static_cast<int32_t>(kDispatcherEventOptions),
                     static_cast<int32_t>(kWord800916D0));
        break;
    case MainMenuResult80015788::Unknown:
        plan.outcome = HandoffOutcome::Gap;
        AppendAction(plan, CardHandoffActionKind::Gap, kFn80015788,
                     result);
        break;
    default:
        plan.outcome = HandoffOutcome::Gap;
        AppendAction(plan, CardHandoffActionKind::Gap, kFn80015788,
                     result);
        break;
    }
    return plan;
}

CardHandoffPlan BuildReplayHandoff80015788Plan(bool selKnown, int32_t sel)
{
    CardHandoffPlan plan = MakePlan("ReplayHandoff80015788");
    plan.outcome = HandoffOutcome::Gap;
    plan.blockedByP0Gap = true;

    AppendAction(plan, CardHandoffActionKind::Call80015700,
                 kFn80015700);
    AppendAction(plan, CardHandoffActionKind::Call800193F4,
                 kFn800193F4,
                 static_cast<int32_t>(kProgressBank80092F10),
                 selKnown ? sel : -1);
    AppendAction(plan, CardHandoffActionKind::GateReplaySelector800193F4,
                 kFn800193F4,
                 static_cast<int32_t>(kProgressBank80092F10),
                 static_cast<int32_t>(kReplaySelOffset80092F3C),
                 selKnown ? 1 : 0,
                 selKnown ? sel : -1,
                 selKnown && sel >= 0 ? 1 : 0);
    AppendPlan(plan,
               BuildCardMode800191E4Plan(
                   CardMode800191E4::Replay,
                   selKnown,
                   selKnown && sel >= 0 ? 1 : 0,
                   selKnown && sel >= 0,
                   sel));

    int32_t scene = 0;
    const bool selectorMaps =
        selKnown && TryMapReplayScene800161A8(sel, &scene);
    if (selKnown && sel < 0) {
        plan.outcome = HandoffOutcome::ContinueLoop;
        return plan;
    }
    if (selKnown && sel >= 0) {
        AppendAction(plan, CardHandoffActionKind::WriteWord800916D0,
                     kFn80015788,
                     static_cast<int32_t>(kWord800916D0),
                     2);
        AppendAction(plan,
                     CardHandoffActionKind::GateReplaySceneMap800161A8,
                     kFn800161A8,
                     sel,
                     static_cast<int32_t>(kReplaySceneMap80048DD8),
                     0,
                     5,
                     selectorMaps ? 1 : 0,
                     selectorMaps ? scene : -1,
                     CardMode800191E4::Replay);
    }
    if (selectorMaps) {
        plan.outcome = HandoffOutcome::ReturnReplayScene;
        AppendAction(plan, CardHandoffActionKind::Call800161A8,
                     kFn800161A8, sel, 0, 0, 0, 0, scene);
        AppendAction(plan, CardHandoffActionKind::ReturnScene,
                     kFn80015788, scene, 0, 0, 0, 0, scene);
    } else if (selKnown && sel >= 0) {
        AppendAction(plan, CardHandoffActionKind::Gap, kFn800161A8,
                     sel,
                     static_cast<int32_t>(kReplaySceneMap80048DD8),
                     0,
                     5);
    } else {
        AppendAction(plan, CardHandoffActionKind::Gap, kFn800193F4,
                     selKnown ? sel : -1,
                     static_cast<int32_t>(kReplaySelOffset80092F3C));
    }
    return plan;
}

CardHandoffPlan BuildLoadSnapshot80018F70Plan()
{
    CardHandoffPlan plan = MakePlan("LoadSnapshot80018F70");
    plan.outcome = HandoffOutcome::PendingCardLoop;
    plan.blockedByP0Gap = true;

    AppendAction(plan, CardHandoffActionKind::Call80018F70,
                 kFn80018F70);
    AppendAction(plan, CardHandoffActionKind::SnapshotSaveDirectory80017B18,
                 kFn80017B18,
                 -8,
                 -4);
    AppendAction(plan, CardHandoffActionKind::ScanSaveDirectory80017B08,
                 kFn80017B08,
                 -1,
                 static_cast<int32_t>(kCardDirectoryRows8007A318),
                 static_cast<int32_t>(kSaveDirectoryBytes80019458));
    AppendAction(plan, CardHandoffActionKind::LoadSnapshot80018F70,
                 kFn80018F70,
                 static_cast<int32_t>(kCardDirectoryRows8007A318),
                 static_cast<int32_t>(kCardPreviousSnapshot8007CC74),
                 static_cast<int32_t>(kSaveDirectoryBytes80019458));
    AppendAction(plan, CardHandoffActionKind::Copy80025C64,
                 kFn80025C64,
                 static_cast<int32_t>(kCardDirectoryRows8007A318),
                 static_cast<int32_t>(kCardPreviousSnapshot8007CC74),
                 static_cast<int32_t>(kSaveDirectoryBytes80019458));
    return plan;
}

CardHandoffPlan BuildLoadFlow800193B0Plan(bool cardResultKnown,
                                          int32_t cardResult)
{
    CardHandoffPlan plan = MakePlan("LoadFlow800193B0");
    plan.outcome = HandoffOutcome::PendingCardLoop;
    plan.blockedByP0Gap = true;

    AppendAction(plan, CardHandoffActionKind::Call800193B0,
                 kFn800193B0,
                 static_cast<int32_t>(kProgressBank80092F10));
    AppendPlan(plan,
               BuildCardMode800191E4Plan(
                   CardMode800191E4::Load,
                   cardResultKnown,
                   cardResultKnown && cardResult >= 0 ? 1 : 0,
                   cardResultKnown && cardResult >= 0,
                   cardResult));
    AppendAction(plan, CardHandoffActionKind::GateLoadFlowResult800193B0,
                 kFn800193B0,
                 cardResultKnown ? 1 : 0,
                 cardResultKnown ? cardResult : -1,
                 static_cast<int32_t>(kGpCardOverwriteScanFlagOffset));

    if (!cardResultKnown) {
        AppendAction(plan, CardHandoffActionKind::Gap, kFn800193B0,
                     static_cast<int32_t>(kFn800191E4),
                     static_cast<int32_t>(CardMode800191E4::Load));
        return plan;
    }
    plan.outcome = HandoffOutcome::ContinueLoop;
    if (cardResult < 0) {
        return plan;
    }

    AppendAction(plan, CardHandoffActionKind::SetGp712,
                 kFn800193B0,
                 static_cast<int32_t>(kGpCardOverwriteScanFlagOffset),
                 1,
                 cardResult);
    AppendPlan(plan, BuildLoadSnapshot80018F70Plan());
    return plan;
}

CardHandoffPlan BuildHiScore80019414Plan(bool gp720Known,
                                         int32_t gp720,
                                         bool tableResultKnown,
                                         int32_t tableResult)
{
    CardHandoffPlan plan = MakePlan("HiScore80019414");
    plan.outcome = HandoffOutcome::Gap;
    plan.blockedByP0Gap = true;

    AppendAction(plan, CardHandoffActionKind::Call80019414,
                 kFn80019414,
                 static_cast<int32_t>(kProgressBank80092F10));
    AppendPlan(plan,
               BuildCardMode800191E4Plan(CardMode800191E4::HiScore,
                                          false,
                                          0,
                                          false,
                                          0,
                                          false));
    AppendAction(plan, CardHandoffActionKind::GateGp720,
                 kFn80019414,
                 static_cast<int32_t>(kGpCardResultOffset),
                 gp720Known ? gp720 : -1,
                 0,
                 0,
                 gp720Known ? gp720 : 0);

    if (!gp720Known) {
        AppendAction(plan, CardHandoffActionKind::Gap, kFn80019414,
                     static_cast<int32_t>(kGpCardResultOffset));
        return plan;
    }
    if (gp720 != 1) {
        plan.outcome = HandoffOutcome::ContinueLoop;
        return plan;
    }

    AppendAction(plan, CardHandoffActionKind::Call80019284,
                 kFn80019284,
                 static_cast<int32_t>(kProgressBank80092F10),
                 tableResultKnown ? tableResult : 0,
                 0,
                 0,
                 tableResultKnown ? tableResult : 0);
    if (!tableResultKnown || tableResult != static_cast<int32_t>(kHiScoreTable80049278)) {
        AppendAction(plan, CardHandoffActionKind::Gap, kFn80019284,
                     tableResultKnown ? tableResult : 0);
        return plan;
    }

    plan.outcome = HandoffOutcome::ReturnHiScoreTable;
    AppendAction(plan, CardHandoffActionKind::Call80026B94,
                 kFn80026B94,
                 static_cast<int32_t>(kDispatcherEventHiScore),
                 tableResult,
                 0,
                 0,
                 tableResult,
                 0,
                 CardMode800191E4::HiScore,
                 CardEventFrameId::HiScore);
    return plan;
}

CardHandoffPlan BuildHiScoreTable80019284Plan(uint32_t a1)
{
    CardHandoffPlan plan = MakePlan("HiScoreTable80019284");
    plan.outcome = HandoffOutcome::ReturnHiScoreTable;
    plan.blockedByP0Gap = true;

    AppendAction(plan, CardHandoffActionKind::Call80019284,
                 kFn80019284,
                 static_cast<int32_t>(a1),
                 static_cast<int32_t>(kHiScoreInputRequiredSize80019284),
                 static_cast<int32_t>(kHiScoreTable80049278));

    for (uint32_t cell = 0; cell < kHiScoreCellCount80019284; ++cell) {
        HiScoreCellSpec80019284 spec{};
        if (!BuildHiScoreCellSpec80019284(cell, &spec)) {
            AppendAction(plan, CardHandoffActionKind::Gap,
                         kFn80019284, static_cast<int32_t>(cell));
            continue;
        }
        AppendAction(plan, CardHandoffActionKind::FormatHiScoreRecord80019284,
                     kFn8004800C,
                     static_cast<int32_t>(spec.cell),
                     static_cast<int32_t>(spec.scoreOffset),
                     static_cast<int32_t>(spec.nameOffset),
                     static_cast<int32_t>(spec.tableRecordOffset),
                     0,
                     static_cast<int32_t>(spec.tableRecordAddress),
                     CardMode800191E4::HiScore,
                     CardEventFrameId::HiScore);
        AppendAction(plan, CardHandoffActionKind::CopyGlyph80017FC4,
                     kFn80017FC4,
                     static_cast<int32_t>(spec.tableRecordAddress),
                     static_cast<int32_t>(kHiScoreGlyphMap800491C4),
                     static_cast<int32_t>(kHiScoreRecordStride80019284),
                     static_cast<int32_t>(spec.cell),
                     0,
                     static_cast<int32_t>(spec.tableRecordAddress),
                     CardMode800191E4::HiScore,
                     CardEventFrameId::HiScore);
    }
    AppendAction(plan, CardHandoffActionKind::Gap, kFn80019284,
                 static_cast<int32_t>(a1),
                 static_cast<int32_t>(kHiScoreInputRequiredSize80019284));
    return plan;
}

CardHandoffPlan BuildStageSelectHandoffPlan(bool resultKnown,
                                            int32_t result,
                                            bool outSceneKnown,
                                            int32_t outScene)
{
    CardHandoffPlan plan = MakePlan("StageSelectHandoff");
    plan.blockedByP0Gap = true;
    plan.outcome = HandoffOutcome::Gap;

    AppendAction(plan, CardHandoffActionKind::Call80026B94,
                 kFn80026B94,
                 static_cast<int32_t>(kDispatcherEventStageSelect),
                 outSceneKnown ? outScene : 0,
                 0,
                 0,
                 resultKnown ? result : 0);
    const bool selectedSceneValid =
        resultKnown && result == 1 && outSceneKnown &&
        IsStageSelectOutScene80025F6C(outScene);
    const bool resultValid =
        !resultKnown ||
        result == 2 ||
        selectedSceneValid;
    AppendAction(plan, CardHandoffActionKind::GateStageSelectResult80026B94,
                 kFn80026B94,
                 resultKnown ? 1 : 0,
                 resultKnown ? result : -1,
                 outSceneKnown ? 1 : 0,
                 outSceneKnown ? outScene : -1,
                 resultValid ? 1 : 0);
    if (selectedSceneValid) {
        plan.outcome = HandoffOutcome::ReturnSelectedScene;
        AppendAction(plan, CardHandoffActionKind::ReturnScene,
                     kFn80015788, outScene, 0, 0, 0, result, outScene);
    } else if (resultKnown && result == 2) {
        plan.outcome = HandoffOutcome::ContinueLoop;
    } else {
        AppendAction(plan, CardHandoffActionKind::Gap, kFn80026B94,
                     resultKnown ? result : -1,
                     outSceneKnown ? outScene : -1);
    }
    return plan;
}

CardHandoffPlan BuildCardMode800191E4Plan(CardMode800191E4 mode,
                                          bool gp716Known,
                                          int32_t gp716,
                                          bool resultKnown,
                                          int32_t result,
                                          bool resultConsumed)
{
    CardHandoffPlan plan = MakePlan("CardMode800191E4");
    plan.outcome = HandoffOutcome::PendingCardLoop;
    plan.blockedByP0Gap = true;

    const CardEventFrameId eventFrame = CardModeEventFrame(mode);
    AppendAction(plan, CardHandoffActionKind::Call800191E4,
                 kFn800191E4,
                 static_cast<int32_t>(kProgressBank80092F10),
                 static_cast<int32_t>(mode),
                 0,
                 0,
                 0,
                 0,
                 mode,
                 eventFrame);
    AppendAction(plan, CardHandoffActionKind::Call80026784,
                 kFn80026784,
                 0,
                 0,
                 0,
                 0,
                 -1);
    AppendAction(plan, CardHandoffActionKind::Copy80025C64,
                 kFn80025C64,
                 -1,
                 static_cast<int32_t>(kCardModeCopyDst8007CC50),
                 36);
    AppendAction(plan, CardHandoffActionKind::WriteGp716,
                 kFn800191E4,
                 static_cast<int32_t>(kGpCardSuccessFlagOffset),
                 0);
    AppendAction(plan, CardHandoffActionKind::WriteGp732,
                 kFn800191E4,
                 static_cast<int32_t>(kGpCardModeOffset),
                 static_cast<int32_t>(mode));
    AppendAction(plan, CardHandoffActionKind::Call80017524,
                 kFn80017524);
    AppendAction(plan, CardHandoffActionKind::Call80018FB0,
                 kFn80018FB0,
                 static_cast<int32_t>(kProgressBank80092F10),
                 static_cast<int32_t>(kFn80018E10),
                 static_cast<int32_t>(kFn80019D7C),
                 static_cast<int32_t>(CardEventFrameId::LoadReplayStart),
                 0,
                 0,
                 mode,
                 eventFrame);
    AppendPlan(plan, BuildCardModeBootstrap80019D7CPlan(mode));
    AppendAction(plan, CardHandoffActionKind::Call80017574,
                 kFn80017574);
    AppendAction(plan, CardHandoffActionKind::GateGp716,
                 kFn800191E4,
                 static_cast<int32_t>(kGpCardSuccessFlagOffset),
                 gp716Known ? gp716 : -1,
                 0,
                 0,
                 gp716Known ? gp716 : 0);
    if (gp716Known && gp716 != 1) {
        plan.outcome = HandoffOutcome::ContinueLoop;
        return plan;
    }
    const bool resultReadRelevant =
        resultConsumed || (gp716Known && gp716 == 1);
    if (resultReadRelevant) {
        AppendAction(plan, CardHandoffActionKind::ReadA1Plus44,
                     kFn800191E4,
                     static_cast<int32_t>(kProgressBank80092F10),
                     44,
                     resultKnown ? result : -1,
                     0,
                     resultKnown ? result : 0);
    }
    if (resultConsumed && !resultKnown) {
        AppendAction(plan, CardHandoffActionKind::Gap, kFn800191E4,
                     static_cast<int32_t>(mode));
    } else {
        plan.outcome = HandoffOutcome::ContinueLoop;
    }
    return plan;
}

CardHandoffPlan BuildCardModeBootstrap80019D7CPlan(CardMode800191E4 mode)
{
    CardHandoffPlan plan = MakePlan("CardModeBootstrap80019D7C");
    plan.outcome = HandoffOutcome::PendingCardLoop;
    plan.blockedByP0Gap = true;

    AppendAction(plan, CardHandoffActionKind::State20CardModeBootstrap80019D7C,
                 kFn80019D7C,
                 20,
                 static_cast<int32_t>(mode),
                 static_cast<int32_t>(CardEventFrameId::LoadReplayStart),
                 0,
                 0,
                 3,
                 mode,
                 CardEventFrameId::LoadReplayStart);
    AppendAction(plan, CardHandoffActionKind::ReturnScene,
                 kFn80019D7C, 3, 0, 0, 0, 3, 3,
                 mode,
                 CardEventFrameId::LoadReplayStart);
    return plan;
}

CardHandoffPlan BuildSaveUi80019148Plan(uint32_t savePtr,
                                        bool resultKnown,
                                        int32_t result)
{
    CardHandoffPlan plan = MakePlan("SaveUi80019148");
    plan.outcome = HandoffOutcome::PendingCardLoop;
    plan.blockedByP0Gap = true;

    AppendAction(plan, CardHandoffActionKind::Call80019148,
                 kFn80019148, static_cast<int32_t>(savePtr));
    AppendAction(plan, CardHandoffActionKind::Call80020110,
                 kFn80020110,
                 0,
                 3,
                 2,
                 1);
    AppendAction(plan, CardHandoffActionKind::Call80026784,
                 kFn80026784,
                 0,
                 0,
                 0,
                 0,
                 -1);
    AppendAction(plan, CardHandoffActionKind::Copy80025C64,
                 kFn80025C64,
                 -1,
                 static_cast<int32_t>(kCardModeCopyDst8007CC50),
                 36);
    AppendAction(plan, CardHandoffActionKind::WriteGp716,
                 kFn80019148,
                 static_cast<int32_t>(kGpCardSuccessFlagOffset),
                 0);
    AppendAction(plan, CardHandoffActionKind::WriteGp732,
                 kFn80019148,
                 static_cast<int32_t>(kGpCardModeOffset),
                 static_cast<int32_t>(CardMode800191E4::Save));
    AppendAction(plan, CardHandoffActionKind::Call80017524,
                 kFn80017524);
    AppendAction(plan, CardHandoffActionKind::Call80018FB0,
                 kFn80018FB0,
                 static_cast<int32_t>(savePtr),
                 static_cast<int32_t>(kFn800185D0),
                 static_cast<int32_t>(kFn80019458),
                 static_cast<int32_t>(CardEventFrameId::SaveStart),
                 0,
                 0,
                 CardMode800191E4::Save,
                 CardEventFrameId::SaveUi);
    AppendPlan(plan, BuildSaveBootstrap80019458Plan());
    AppendPlan(plan, BuildLoadSnapshot80018F70Plan());
    AppendAction(plan, CardHandoffActionKind::Call80017574,
                 kFn80017574);
    AppendAction(plan, CardHandoffActionKind::GateSaveUiResult80019148,
                 kFn80019148,
                 static_cast<int32_t>(kGpCardSuccessFlagOffset),
                 resultKnown ? result : -1,
                 0,
                 0,
                 resultKnown ? result : 0);
    if (!resultKnown) {
        AppendAction(plan, CardHandoffActionKind::Gap, kFn80019148,
                     static_cast<int32_t>(savePtr),
                     static_cast<int32_t>(kGpCardSuccessFlagOffset));
    } else {
        plan.outcome = HandoffOutcome::ContinueLoop;
    }
    return plan;
}

CardHandoffPlan BuildSaveBootstrap80019458Plan()
{
    CardHandoffPlan plan = MakePlan("SaveBootstrap80019458");
    plan.outcome = HandoffOutcome::PendingCardLoop;
    plan.blockedByP0Gap = true;

    AppendAction(plan, CardHandoffActionKind::State21SaveBootstrap80019458,
                 kFn80019458,
                 21,
                 static_cast<int32_t>(kGpCardPromptArgOffset),
                 static_cast<int32_t>(CardEventFrameId::SaveUi),
                 0,
                 0,
                 2,
                 CardMode800191E4::Save,
                 CardEventFrameId::SaveUi);
    AppendAction(plan, CardHandoffActionKind::InitEventArg80017E58,
                 kFn80017E58,
                 static_cast<int32_t>(kGpCardPromptArgOffset),
                 static_cast<int32_t>(CardEventFrameId::SaveUi));
    AppendAction(plan, CardHandoffActionKind::ReturnScene,
                 kFn80019458, 2, 0, 0, 0, 2, 2,
                 CardMode800191E4::Save,
                 CardEventFrameId::SaveUi);
    AppendPlan(plan, BuildCardStateRoute800180D8Plan(2,
                                                     CardMode800191E4::Save));
    return plan;
}

CardHandoffPlan BuildCardDriverLoop80018FB0Plan(
    int32_t startState,
    CardEventFrameId eventId,
    CardMode800191E4 mode)
{
    CardHandoffPlan plan = MakePlan("CardDriverLoop80018FB0");
    plan.outcome = HandoffOutcome::PendingCardLoop;
    plan.blockedByP0Gap = true;

    AppendAction(plan, CardHandoffActionKind::Call80018FB0,
                 kFn80018FB0, startState,
                 static_cast<int32_t>(eventId),
                 static_cast<int32_t>(mode));
    AppendAction(plan, CardHandoffActionKind::InitCardDriver80018FB0,
                 kFn80018FB0,
                 static_cast<int32_t>(kGpCardPromptArgOffset),
                 static_cast<int32_t>(kGpCardResultOffset),
                 static_cast<int32_t>(kGpCardBlinkOffset),
                 static_cast<int32_t>(eventId));
    AppendAction(plan, CardHandoffActionKind::SetGp720,
                 kFn80018FB0,
                 static_cast<int32_t>(kGpCardResultOffset),
                 0);
    AppendAction(plan, CardHandoffActionKind::BlinkState80018FB0,
                 kFn80018FB0,
                 static_cast<int32_t>(kGpCardBlinkOffset),
                 0);
    AppendAction(plan, CardHandoffActionKind::InitEventArg80017E58,
                 kFn80017E58,
                 static_cast<int32_t>(kGpCardPromptArgOffset),
                 static_cast<int32_t>(eventId));
    AppendAction(plan, CardHandoffActionKind::GateCardDriverExit80018FB0,
                 kFn80018FB0,
                 startState,
                 23);
    AppendAction(plan, CardHandoffActionKind::Call80035510,
                 kFn80035510,
                 1);
    AppendAction(plan, CardHandoffActionKind::InputEdge80018FB0,
                 kFn80018FB0,
                 static_cast<int32_t>(kGpCardLastInputOffset),
                 -1);

    const uint32_t inputCallback = CardInputCallbackForMode(mode);
    const uint32_t tickCallback = CardTickCallbackForMode(mode);
    if (inputCallback == 0 || tickCallback == 0) {
        AppendAction(plan, CardHandoffActionKind::Gap, kFn80018FB0,
                     startState,
                     static_cast<int32_t>(eventId),
                     static_cast<int32_t>(mode));
        return plan;
    }
    AppendAction(plan, CardHandoffActionKind::RouteInputCallback80018FB0,
                 kFn80018FB0,
                 static_cast<int32_t>(inputCallback),
                 startState,
                 static_cast<int32_t>(kProgressBank80092F10),
                 -1,
                 0,
                 0,
                 mode,
                 eventId);
    AppendAction(plan, InputCallbackKind(inputCallback),
                 inputCallback,
                 -1,
                 startState,
                 static_cast<int32_t>(kProgressBank80092F10),
                 0,
                 0,
                 0,
                 mode,
                 eventId);
    AppendAction(plan, CardHandoffActionKind::Poll80017594,
                 kFn80017594);
    AppendAction(plan, CardHandoffActionKind::RouteIoCallback80018FB0,
                 kFn80018FB0,
                 static_cast<int32_t>(tickCallback),
                 startState,
                 static_cast<int32_t>(kGpCardIoCodeOffset),
                 -1,
                 0,
                 0,
                 mode,
                 eventId);
    AppendAction(plan, TickCallbackKind(tickCallback),
                 tickCallback,
                 startState,
                 -1,
                 static_cast<int32_t>(kProgressBank80092F10),
                 0,
                 0,
                 0,
                 mode,
                 eventId);
    AppendAction(plan, CardHandoffActionKind::RemapEvent800180D8,
                 kFn800180D8, startState);
    const CardStateRouteSpec* route = FindCardStateRouteSpec(startState, mode);
    const CardEventFrameId drawEvent =
        route != nullptr ? route->eventFrame : eventId;
    const uint32_t drawArg = route != nullptr ? route->argAddress : 0;
    if (route != nullptr) {
        AppendRouteState800180D8(plan, *route, startState, mode);
    }
    AppendAction(plan, CardHandoffActionKind::BlinkState80018FB0,
                 kFn80018FB0,
                 startState,
                 static_cast<int32_t>(kGpCardBlinkOffset));
    AppendAction(plan, CardHandoffActionKind::DrawEventFrame8001E750,
                 kFn8001E750,
                 static_cast<int32_t>(drawEvent),
                 static_cast<int32_t>(drawArg),
                 0,
                 0,
                 0,
                 0,
                 mode,
                 drawEvent);
    AppendAction(plan, CardHandoffActionKind::WaitCard80035560,
                 kFn80035560, 0);
    AppendAction(plan, CardHandoffActionKind::EndFrame8001EA00,
                 kFn8001EA00, 0);
    AppendAction(plan, CardHandoffActionKind::FlashPrompt80017E6C,
                 kFn80017E6C,
                 static_cast<int32_t>(drawEvent),
                 static_cast<int32_t>(drawArg),
                 -1,
                 0,
                 0,
                 0,
                 mode,
                 drawEvent);
    AppendAction(plan, CardHandoffActionKind::GateGp720,
                 kFn80018FB0,
                 static_cast<int32_t>(kGpCardResultOffset),
                 -1);
    AppendAction(plan, CardHandoffActionKind::Gap, kFn80018FB0,
                 startState,
                 static_cast<int32_t>(eventId),
                 static_cast<int32_t>(mode));
    return plan;
}

CardHandoffPlan BuildCardStateRoute800180D8Plan(int32_t state,
                                               CardMode800191E4 mode)
{
    CardHandoffPlan plan = MakePlan("CardStateRoute800180D8");
    plan.outcome = HandoffOutcome::PendingCardLoop;
    plan.blockedByP0Gap = true;

    AppendAction(plan, CardHandoffActionKind::RemapEvent800180D8,
                 kFn800180D8, state, static_cast<int32_t>(mode));

    const CardStateRouteSpec* route = FindCardStateRouteSpec(state, mode);
    if (route == nullptr) {
        AppendAction(plan, CardHandoffActionKind::Gap, kFn800180D8,
                     state, static_cast<int32_t>(mode));
        return plan;
    }

    AppendRouteState800180D8(plan, *route, state, mode);
    AppendAction(plan, CardHandoffActionKind::DrawEventFrame8001E750,
                 kFn8001E750,
                 static_cast<int32_t>(route->eventFrame),
                 static_cast<int32_t>(route->argAddress),
                 0,
                 0,
                 0,
                 0,
                 mode,
                 route->eventFrame);
    if (route->eventFrame == CardEventFrameId::CardInfo) {
        const bool knownCardInfoArg = route->argAddress == kCardNameArg80049244;
        AppendAction(plan,
                     CardHandoffActionKind::GateCardInfoArg80049244,
                     kFn800180D8,
                     state,
                     static_cast<int32_t>(route->argAddress),
                     static_cast<int32_t>(kCardNameArg80049244),
                     knownCardInfoArg ? 1 : 0,
                     0,
                     static_cast<int32_t>(route->argAddress),
                     mode,
                     route->eventFrame);
        if (!knownCardInfoArg) {
            AppendAction(plan,
                         CardHandoffActionKind::Gap,
                         kFn800180D8,
                         state,
                         static_cast<int32_t>(route->argAddress),
                         static_cast<int32_t>(kCardNameArg80049244));
        }
    }
    return plan;
}

CardHandoffPlan BuildCardIoRouteCase4Plan(CardMode800191E4 mode,
                                          CardIoCode80017594 ioCode)
{
    CardHandoffPlan plan = MakePlan("CardIoRouteCase4");
    plan.outcome = HandoffOutcome::PendingCardLoop;
    plan.blockedByP0Gap = true;

    AppendAction(plan, CardHandoffActionKind::Poll80017594,
                 kFn80017594, 4, static_cast<int32_t>(ioCode));

    const CardIoRouteSpec* ioRoute = FindCardIoRouteSpec(ioCode, mode);
    if (ioRoute == nullptr) {
        AppendAction(plan, CardHandoffActionKind::Gap, kFn80017594,
                     static_cast<int32_t>(ioCode),
                     static_cast<int32_t>(mode));
        return plan;
    }

    AppendAction(plan, CardHandoffActionKind::RouteIoCodeState4,
                 mode == CardMode800191E4::Save ? kFn80019458 : kFn80019D7C,
                 ioRoute->sourceState,
                 static_cast<int32_t>(ioCode),
                 ioRoute->nextState,
                 static_cast<int32_t>(ioRoute->eventFrame),
                 0,
                 0,
                 mode,
                 ioRoute->eventFrame);

    if (ioRoute->nextState != 4 && ioRoute->eventFrame != CardEventFrameId::Unknown) {
        AppendPlan(plan, BuildCardStateRoute800180D8Plan(ioRoute->nextState,
                                                         mode));
    }
    if (ioRoute->nextState == 6 && mode != CardMode800191E4::Save) {
        AppendPlan(plan, BuildLoadReplayDirectoryScan80019D7CPlan(mode));
    }
    AppendAction(plan, CardHandoffActionKind::Gap,
                 mode == CardMode800191E4::Save ? kFn80019458 : kFn80019D7C,
                 ioRoute->sourceState,
                 ioRoute->nextState,
                 static_cast<int32_t>(mode));
    return plan;
}

CardHandoffPlan BuildCardPromptInputPlan(int32_t state,
                                         CardMode800191E4 mode,
                                         bool confirm)
{
    CardHandoffPlan plan = MakePlan("CardPromptInput");
    plan.outcome = HandoffOutcome::PendingCardLoop;
    plan.blockedByP0Gap = true;

    const CardStateRouteSpec* route = FindCardStateRouteSpec(state, mode);
    if (route == nullptr || !IsCardPromptEvent(route->eventFrame)) {
        AppendAction(plan, CardHandoffActionKind::Gap, kFn800180D8,
                     state, static_cast<int32_t>(mode));
        return plan;
    }

    const CardHandoffActionKind inputKind =
        InputCallbackKind(route->inputCallback);
    AppendAction(plan, inputKind,
                 route->inputCallback,
                 state,
                 confirm ? 1 : 0,
                 static_cast<int32_t>(mode),
                 0,
                 0,
                 confirm ? route->confirmNextState : route->cancelNextState,
                 mode,
                 route->eventFrame);

    const int32_t nextState =
        confirm ? route->confirmNextState : route->cancelNextState;
    const int32_t selected = confirm ? 1 : 2;
    const int32_t flag =
        confirm && state == 8 && mode == CardMode800191E4::Save ? 1 : 0;
    const uint32_t promptArg = PromptFlashArgSource(*route);
    AppendAction(plan, CardHandoffActionKind::PlayInputSfx80025C8C,
                 kFn80025C8C, kNameConfirmSfx800185D0);
    AppendAction(plan, CardHandoffActionKind::FlashPrompt80017E6C,
                 kFn80017E6C,
                 static_cast<int32_t>(route->eventFrame),
                 static_cast<int32_t>(promptArg),
                 selected,
                 flag,
                 0,
                 nextState,
                 mode,
                 route->eventFrame);

    if (confirm && state == 8 && mode == CardMode800191E4::Save) {
        AppendPlan(plan, BuildState14Format80019458Plan());
    }
    if (confirm && state == 18 && mode == CardMode800191E4::Save) {
        AppendAction(plan, CardHandoffActionKind::ClearName80018060,
                     kFn80018060, 10, static_cast<int32_t>(mode));
        AppendPlan(plan, BuildNameEntryReset80018060Plan());
    }
    if (route->exitsOnConfirm && confirm) {
        plan.outcome = HandoffOutcome::ReturnScene0;
    }
    AppendAction(plan, CardHandoffActionKind::Gap,
                 route->inputCallback,
                 state,
                 nextState,
                 static_cast<int32_t>(mode));
    return plan;
}

CardHandoffPlan BuildNameEntryReset80018060Plan()
{
    CardHandoffPlan plan = MakePlan("NameEntryReset80018060");
    plan.outcome = HandoffOutcome::PendingCardLoop;
    plan.blockedByP0Gap = true;

    AppendAction(plan, CardHandoffActionKind::InitNameEntry80018060,
                 kFn80017E58,
                 static_cast<int32_t>(kCardNameArg80049244));
    AppendAction(plan, CardHandoffActionKind::ClearNameEntryFlags80018060,
                 kFn80018060,
                 static_cast<int32_t>(kCardNameInputCursor8004925A),
                 static_cast<int32_t>(kCardNameInputEditFlag8004925C),
                 static_cast<int32_t>(kCardNameInputDecision8004925E),
                 0);
    AppendAction(plan, CardHandoffActionKind::ClearNameEntryBuffers80018060,
                 kFn80018060,
                 static_cast<int32_t>(kCardNameArg80049244),
                 static_cast<int32_t>(kCardNameInputRawBuffer80049260),
                 static_cast<int32_t>(kCardNameInputPreview8004926C),
                 6);
    AppendNamePreview80017FC4(plan, kCardNameInputDefaultText8006EAF0);
    return plan;
}

CardHandoffPlan BuildSaveNameEntryInput800185D0Plan(
    int32_t inputMask,
    bool currentCharKnown,
    int32_t currentChar,
    uint32_t savePtr,
    bool duplicateKnown,
    bool duplicate,
    bool freeBlocksKnown,
    int32_t freeBlocks)
{
    CardHandoffPlan plan = MakePlan("SaveNameEntryInput800185D0");
    plan.outcome = HandoffOutcome::PendingCardLoop;
    plan.blockedByP0Gap = true;

    AppendAction(plan, CardHandoffActionKind::State10NameEntry800185D0,
                 kFn800185D0,
                 inputMask,
                 currentCharKnown ? 1 : 0,
                 currentChar,
                 static_cast<int32_t>(savePtr));

    if (IsNameCursorInput800185D0(inputMask)) {
        AppendAction(plan, CardHandoffActionKind::PlayInputSfx80025C8C,
                     kFn80025C8C, kNameMoveSfx800185D0);
        AppendAction(plan, CardHandoffActionKind::RouteNameCursor800185D0,
                     kFn800185D0,
                     inputMask,
                     static_cast<int32_t>(kCardNameInputCursor8004925A),
                     static_cast<int32_t>(kCardNameInputCharCount80049258),
                     0,
                     -1);
    }

    if ((inputMask & kInputNameMask800185D0) == 0) {
        if (inputMask == kInputTriangle800185D0) {
            AppendAction(plan, CardHandoffActionKind::PlayInputSfx80025C8C,
                         kFn80025C8C, kNameDeleteSfx800185D0);
            AppendAction(plan,
                         CardHandoffActionKind::DeleteNameCharacter800185D0,
                         kFn800185D0,
                         inputMask,
                         kNameCharBackspace800185D0,
                         static_cast<int32_t>(
                             kCardNameInputEditFlag8004925C),
                         static_cast<int32_t>(
                             kCardNameInputRawBuffer80049260));
        }
        AppendNamePreview80017FC4(plan, kCardNameInputRawBuffer80049260);
        AppendAction(plan, CardHandoffActionKind::ReturnScene,
                     kFn800185D0, 10, 0, 0, 0, 10);
        return plan;
    }

    AppendAction(plan, CardHandoffActionKind::GateNameCharacter800185D0,
                 kFn800185D0,
                 static_cast<int32_t>(kCardNameInputCharTable800490E8),
                 currentCharKnown ? 1 : 0,
                 currentChar,
                 inputMask);
    if (!currentCharKnown) {
        AppendAction(plan, CardHandoffActionKind::Gap,
                     kFn800185D0, 10, inputMask);
        return plan;
    }

    if (currentChar == kNameCharBackspace800185D0) {
        AppendAction(plan, CardHandoffActionKind::PlayInputSfx80025C8C,
                     kFn80025C8C, kNameDeleteSfx800185D0);
        AppendAction(plan, CardHandoffActionKind::DeleteNameCharacter800185D0,
                     kFn800185D0,
                     inputMask,
                     currentChar,
                     static_cast<int32_t>(kCardNameInputEditFlag8004925C),
                     static_cast<int32_t>(kCardNameInputRawBuffer80049260));
        AppendNamePreview80017FC4(plan, kCardNameInputRawBuffer80049260);
        AppendAction(plan, CardHandoffActionKind::ReturnScene,
                     kFn800185D0, 10, 0, 0, 0, 10);
        return plan;
    }

    if (currentChar == kNameCharNewline800185D0) {
        if (inputMask == kInputCross800185D0) {
            CardHandoffPlan confirm =
                BuildSaveNameEntryConfirm800185D0Plan(savePtr,
                                                       duplicateKnown,
                                                       duplicate,
                                                       freeBlocksKnown,
                                                       freeBlocks);
            AppendPlan(plan, confirm);
            plan.outcome = confirm.outcome;
            return plan;
        }
        if (inputMask == kInputCircle800185D0) {
            AppendAction(plan, CardHandoffActionKind::PlayInputSfx80025C8C,
                         kFn80025C8C, kNameConfirmSfx800185D0);
            AppendAction(plan,
                         CardHandoffActionKind::CancelNameEntry800185D0,
                         kFn800185D0,
                         static_cast<int32_t>(kCardNameInputDecision8004925E),
                         2,
                         static_cast<int32_t>(kCardNameInputSubmit80049248),
                         2);
            AppendAction(plan, CardHandoffActionKind::FlashPrompt80017E6C,
                         kFn80017E6C,
                         static_cast<int32_t>(CardEventFrameId::CardInfo),
                         static_cast<int32_t>(kCardNameArg80049244),
                         2,
                         0,
                         0,
                         2,
                         CardMode800191E4::Save,
                         CardEventFrameId::CardInfo);
            AppendPlan(plan, BuildCardStateRoute800180D8Plan(
                                 2, CardMode800191E4::Save));
            return plan;
        }
        AppendNamePreview80017FC4(plan, kCardNameInputRawBuffer80049260);
        AppendAction(plan, CardHandoffActionKind::ReturnScene,
                     kFn800185D0, 10, 0, 0, 0, 10);
        return plan;
    }

    if (currentChar >= kNameCharPrintableMin800185D0) {
        AppendAction(plan, CardHandoffActionKind::PlayInputSfx80025C8C,
                     kFn80025C8C, kNameAppendSfx800185D0);
        AppendAction(plan, CardHandoffActionKind::AppendNameCharacter800185D0,
                     kFn800185D0,
                     inputMask,
                     currentChar,
                     static_cast<int32_t>(kCardNameInputEditFlag8004925C),
                     static_cast<int32_t>(kCardNameInputRawBuffer80049260),
                     static_cast<int32_t>(kNameEntryMaxChars800185D0));
    }
    AppendNamePreview80017FC4(plan, kCardNameInputRawBuffer80049260);
    AppendAction(plan, CardHandoffActionKind::ReturnScene,
                 kFn800185D0, 10, 0, 0, 0, 10);
    return plan;
}

CardHandoffPlan BuildSaveOverwritePrompt800185D0Plan(
    bool confirm,
    uint32_t savePtr,
    bool selectedNameKnown,
    bool selectedNameHasPrefix)
{
    CardHandoffPlan plan = MakePlan("SaveOverwritePrompt800185D0");
    plan.outcome = HandoffOutcome::PendingCardLoop;
    plan.blockedByP0Gap = true;

    AppendAction(plan, CardHandoffActionKind::State22OverwritePrompt800185D0,
                 kFn800185D0,
                 confirm ? kInputCross800185D0 : kInputCircle800185D0,
                 static_cast<int32_t>(savePtr),
                 selectedNameKnown ? 1 : 0,
                 selectedNameHasPrefix ? 1 : 0,
                 0,
                 22,
                 CardMode800191E4::Save,
                 CardEventFrameId::OverwritePrompt);
    AppendAction(plan, CardHandoffActionKind::PlayInputSfx80025C8C,
                 kFn80025C8C, kNameConfirmSfx800185D0);

    if (!confirm) {
        AppendAction(plan, CardHandoffActionKind::FlashPrompt80017E6C,
                     kFn80017E6C,
                     static_cast<int32_t>(CardEventFrameId::OverwritePrompt),
                     static_cast<int32_t>(kGpCardPromptArgOffset),
                     2,
                     0,
                     0,
                     11,
                     CardMode800191E4::Save,
                     CardEventFrameId::OverwritePrompt);
        AppendAction(plan, CardHandoffActionKind::CancelOverwritePrompt800185D0,
                     kFn800185D0,
                     22,
                     11,
                     static_cast<int32_t>(kCardNameBuffer8007CBE8),
                     static_cast<int32_t>(savePtr + 1u),
                     11,
                     0,
                     CardMode800191E4::Save,
                     CardEventFrameId::Save);
        AppendPlan(plan, BuildCardStateRoute800180D8Plan(
                             11, CardMode800191E4::Save));
        return plan;
    }

    AppendAction(plan, CardHandoffActionKind::FlashPrompt80017E6C,
                 kFn80017E6C,
                 static_cast<int32_t>(CardEventFrameId::OverwritePrompt),
                 static_cast<int32_t>(kGpCardPromptArgOffset),
                 1,
                 1,
                 0,
                 15,
                 CardMode800191E4::Save,
                 CardEventFrameId::OverwritePrompt);
    AppendAction(plan, CardHandoffActionKind::ExtractOverwriteSuffix800185D0,
                 kFn800185D0,
                 static_cast<int32_t>(kCardNameBuffer8007CBE8),
                 static_cast<int32_t>(kCardDirectorySuffixScratch8007A570),
                 static_cast<int32_t>(kGpCardFilenamePrefixOffset),
                 static_cast<int32_t>(savePtr + 1u),
                 selectedNameKnown && selectedNameHasPrefix ? 1 : 0,
                 15,
                 CardMode800191E4::Save,
                 CardEventFrameId::OverwritePrompt);
    AppendAction(plan, CardHandoffActionKind::ExtractOverwriteSuffix800185D0,
                 kFn800185D0,
                 static_cast<int32_t>(kOverwriteNameCopyBytes800185D0),
                 static_cast<int32_t>(kCardDirectorySuffixScratch8007A570),
                 static_cast<int32_t>(kCardNameBuffer8007CBE8),
                 static_cast<int32_t>(savePtr + 1u),
                 selectedNameKnown && selectedNameHasPrefix ? 1 : 0,
                 15,
                 CardMode800191E4::Save,
                 CardEventFrameId::OverwritePrompt);
    if (!selectedNameKnown || !selectedNameHasPrefix) {
        AppendAction(plan, CardHandoffActionKind::Gap,
                     kFn800185D0,
                     22,
                     selectedNameKnown ? 1 : 0,
                     selectedNameHasPrefix ? 1 : 0,
                     static_cast<int32_t>(savePtr + 1u));
    }
    AppendAction(plan, CardHandoffActionKind::ReturnScene,
                 kFn800185D0, 15, 0, 0, 0, 15);
    return plan;
}

CardHandoffPlan BuildRead800179B4Plan(uint32_t namePtr,
                                      uint32_t blockBuffer,
                                      int32_t blocks,
                                      bool eventKnown,
                                      int32_t eventResult80016EB8)
{
    const CardEventPollSourceSpec& pollSpec = CardEventPollSw80016EB8Spec();
    const LowerCardResultSpec& successSpec =
        LowerCardReadSuccess800179B4Spec();
    const LowerCardResultSpec& failureSpec =
        LowerCardReadFailure800179B4Spec();
    CardHandoffPlan plan = MakePlan("Read800179B4");
    plan.outcome = HandoffOutcome::PendingCardLoop;
    plan.blockedByP0Gap = true;

    const int32_t byteCount = blocks << 13;
    const bool readSucceeded =
        eventKnown &&
        LowerCardPollResultMatches(successSpec, eventResult80016EB8);
    const bool readFailed =
        eventKnown &&
        LowerCardPollResultMatches(failureSpec, eventResult80016EB8);
    const int32_t readResult =
        readSucceeded ? LowerCardFunctionResult(successSpec,
                                                eventResult80016EB8) :
        LowerCardFunctionResult(failureSpec, eventResult80016EB8);

    AppendAction(plan, CardHandoffActionKind::Read800179B4,
                 kFn800179B4,
                 static_cast<int32_t>(namePtr),
                 static_cast<int32_t>(blockBuffer),
                 blocks,
                 byteCount);
    AppendAction(plan, CardHandoffActionKind::Call800173A8,
                 kFn800173A8,
                 static_cast<int32_t>(kGpCardPortOffset),
                 static_cast<int32_t>(kGpCardSlotOffset),
                 static_cast<int32_t>(namePtr),
                 static_cast<int32_t>(blockBuffer),
                 blocks);
    AppendAction(plan, CardHandoffActionKind::FormatCardPath800173A8,
                 kFn8004800C,
                 static_cast<int32_t>(namePtr),
                 blocks,
                 byteCount);
    AppendAction(plan, CardHandoffActionKind::OpenCardFile800173A8,
                 kFn800173A8,
                 static_cast<int32_t>(kGpCardFdOffset),
                 0x8001);
    AppendAction(plan, CardHandoffActionKind::ClearEvents80016FC0,
                 kFn80016FC0);
    AppendAction(plan, CardHandoffActionKind::SubmitCardRead800173A8,
                 kFn800173A8,
                 static_cast<int32_t>(blockBuffer),
                 byteCount,
                 blocks);
    AppendAction(plan, CardHandoffActionKind::PollEvents80016EB8,
                 pollSpec.pollFunction,
                 eventKnown ? eventResult80016EB8 : -1,
                 static_cast<int32_t>(pollSpec.localTimeoutCount));
    const bool eventPollValid =
        !eventKnown || IsCardEventPollResult(eventResult80016EB8);
    AppendAction(plan, CardHandoffActionKind::GateSwCardEventSource80016EB8,
                 pollSpec.pollFunction,
                 static_cast<int32_t>(pollSpec.eventClass),
                 static_cast<int32_t>(pollSpec.firstHandleGpOffset),
                 static_cast<int32_t>(pollSpec.handleCount),
                 static_cast<int32_t>(pollSpec.localTimeoutCount),
                 eventKnown ? eventResult80016EB8 : -1,
                 eventPollValid ? 1 : 0);
    AppendAction(plan, CardHandoffActionKind::GateSwPollResult80016EB8,
                 pollSpec.pollFunction,
                 eventKnown ? 1 : 0,
                 eventKnown ? eventResult80016EB8 : -1,
                 pollSpec.minEventReturn,
                 pollSpec.maxEventReturn,
                 eventPollValid ? 1 : 0);
    AppendAction(plan, CardHandoffActionKind::CloseGp696,
                 kFn800179B4,
                 static_cast<int32_t>(kGpCardFdOffset));
    AppendAction(plan, CardHandoffActionKind::GateReadSuccess800179B4,
                 kFn800179B4,
                 eventKnown ? eventResult80016EB8 : -1,
                 readSucceeded ? 1 : 0,
                 readFailed ? 1 : 0,
                 0,
                 0,
                 eventKnown ? readResult : 0);
    if (!eventKnown) {
        AppendAction(plan, CardHandoffActionKind::Gap, kFn800179B4,
                     static_cast<int32_t>(namePtr),
                     static_cast<int32_t>(blockBuffer),
                     blocks);
    } else if (!eventPollValid) {
        AppendAction(plan, CardHandoffActionKind::Gap, pollSpec.pollFunction,
                     eventResult80016EB8,
                     static_cast<int32_t>(namePtr),
                     static_cast<int32_t>(blockBuffer),
                     blocks);
    }
    return plan;
}

CardHandoffPlan BuildLoadReplayDirectoryScan80019D7CPlan(
    CardMode800191E4 mode,
    bool entryCountKnown,
    int32_t entryCount)
{
    CardHandoffPlan plan = MakePlan("LoadReplayDirectoryScan80019D7C");
    plan.outcome = HandoffOutcome::PendingCardLoop;
    plan.blockedByP0Gap = true;

    const int32_t nextState = State6ResultFor80019D7CMode(mode);
    AppendAction(plan, CardHandoffActionKind::Call80019D7C,
                 kFn80019D7C,
                 6,
                 0,
                 static_cast<int32_t>(mode));
    if (nextState < 0) {
        AppendAction(plan, CardHandoffActionKind::Gap,
                     kFn80019D7C, 6, static_cast<int32_t>(mode));
        return plan;
    }

    AppendAction(plan, CardHandoffActionKind::ScanSaveDirectory80017B08,
                 kFn80017B08,
                 6,
                 static_cast<int32_t>(kCardDirectoryRows8007A318),
                 static_cast<int32_t>(kSaveDirectoryBytes80019458));
    AppendAction(plan, CardHandoffActionKind::SnapshotSaveDirectory80017B18,
                 kFn80017B18,
                 static_cast<int32_t>(kCardDirectoryRows8007A318),
                 static_cast<int32_t>(kSaveDirectoryBytes80019458));
    AppendAction(plan, CardHandoffActionKind::BuildLoadReplayListRows80019D7C,
                 kFn80019D7C,
                 static_cast<int32_t>(kCardDirectoryRows8007A318),
                 static_cast<int32_t>(kCardRowTable8007A590),
                 static_cast<int32_t>(kGpCardFilenamePrefixOffset),
                 static_cast<int32_t>(kCardDirectorySuffixScratch8007A570),
                 entryCountKnown ? entryCount : -1,
                 nextState,
                 mode);
    AppendAction(plan,
                 CardHandoffActionKind::GateDirectoryListSource80017B08,
                 kFn80017B08,
                 static_cast<int32_t>(kCardDirectoryRows8007A318),
                 static_cast<int32_t>(kCardRowTable8007A590),
                 6,
                 entryCountKnown ? entryCount : -1,
                 entryCountKnown ? 1 : 0,
                 nextState,
                 mode);
    AppendAction(plan, CardHandoffActionKind::GateWord8007ABE4,
                 kFn80019D7C,
                 static_cast<int32_t>(kWord8007ABE4),
                 entryCountKnown ? entryCount : -1);
    AppendAction(plan, CardHandoffActionKind::InitEventArg80017E58,
                 kFn80017E58,
                 static_cast<int32_t>(kCardGridArg80048E50),
                 nextState);
    AppendAction(plan, CardHandoffActionKind::CopyGlyph80017FC4,
                 kFn80017FC4,
                 static_cast<int32_t>(kCardGridArg80048E50),
                 static_cast<int32_t>(kCardRowTable8007A590),
                 static_cast<int32_t>(kSaveListRowCount80019458));
    AppendAction(plan, CardHandoffActionKind::SetLoadReplayListEventArg80019D7C,
                 kFn80019D7C,
                 static_cast<int32_t>(kCardGridArg80048E50),
                 static_cast<int32_t>(kCardGridPageRow80048E5C),
                 static_cast<int32_t>(kCardGridStep80048E5E),
                 static_cast<int32_t>(kCardGridSelectedRow80048E64),
                 entryCountKnown ? entryCount : -1,
                 nextState,
                 mode);
    if (!entryCountKnown) {
        AppendAction(plan, CardHandoffActionKind::Gap,
                     kFn80019D7C, 6, static_cast<int32_t>(mode));
        return plan;
    }

    AppendAction(plan, CardHandoffActionKind::ReturnScene,
                 kFn80019D7C, nextState, 0, 0, 0, nextState, nextState,
                 mode);
    if (mode == CardMode800191E4::Load ||
        mode == CardMode800191E4::Replay) {
        AppendPlan(plan, BuildCardStateRoute800180D8Plan(nextState, mode));
    }
    return plan;
}

CardHandoffPlan BuildLoadReplayTickIoGate80019D7CPlan(
    int32_t state,
    CardMode800191E4 mode,
    bool ioKnown,
    int32_t ioResult)
{
    CardHandoffPlan plan = MakePlan("LoadReplayTickIoGate80019D7C");
    plan.outcome = HandoffOutcome::PendingCardLoop;
    plan.blockedByP0Gap = true;

    if (state != 8 && state != 12 && state != 13 && state != 14) {
        AppendAction(plan, CardHandoffActionKind::Gap,
                     kFn80019D7C, state, static_cast<int32_t>(mode));
        return plan;
    }

    const int32_t nextState = ioKnown && ioResult == 3 ? 5 : state;
    AppendAction(plan, CardHandoffActionKind::LoadReplayTickIoGate80019D7C,
                 kFn80019D7C,
                 state,
                 ioKnown ? ioResult : -1,
                 5,
                 nextState,
                 ioKnown ? ioResult : -1,
                 nextState,
                 mode,
                 EventFrom80018E10ListState(state));
    if (!ioKnown) {
        AppendAction(plan, CardHandoffActionKind::Gap,
                     kFn80019D7C, state, static_cast<int32_t>(mode));
        return plan;
    }
    AppendAction(plan, CardHandoffActionKind::ReturnScene,
                 kFn80019D7C, nextState, 0, 0, 0, nextState, nextState,
                 mode,
                 EventFrom80018E10ListState(state));
    if (ioResult == 3) {
        AppendPlan(plan, BuildCardStateRoute800180D8Plan(5, mode));
    }
    return plan;
}

CardHandoffPlan BuildState16LoadPayload80019D7CPlan(bool a2Known,
                                                    int32_t a2,
                                                    bool payloadKnown,
                                                    CardMode800191E4 mode,
                                                    bool readEventKnown,
                                                    int32_t readEventResult80016EB8)
{
    const LowerCardPayloadProducerSpec& payloadSpec =
        LowerCardPayloadState16Producer80019D7C();
    const LowerCardResultSpec& readSuccessSpec =
        LowerCardReadSuccess800179B4Spec();
    const bool readSucceeded =
        readEventKnown &&
        LowerCardPollResultMatches(readSuccessSpec,
                                   readEventResult80016EB8);
    const bool payloadAuthorized = payloadKnown && readSucceeded;
    CardHandoffPlan plan = MakePlan("State16LoadPayload80019D7C");
    plan.outcome = HandoffOutcome::PendingCardLoop;
    plan.blockedByP0Gap = true;

    AppendAction(plan, CardHandoffActionKind::Call80019D7C,
                 payloadSpec.stateFunction,
                 16,
                 a2Known ? a2 : -1,
                 static_cast<int32_t>(mode));
    if (!a2Known) {
        AppendAction(plan, CardHandoffActionKind::Gap,
                     payloadSpec.stateFunction, 16);
        return plan;
    }
    if (a2 == 3) {
        plan.outcome = HandoffOutcome::ContinueLoop;
        AppendAction(plan, CardHandoffActionKind::ReturnScene,
                     payloadSpec.stateFunction, 5, 0, 0, 0, 5);
        return plan;
    }

    AppendAction(plan, CardHandoffActionKind::State16LoadPayload80019D7C,
                 payloadSpec.stateFunction,
                 16,
                 a2,
                 static_cast<int32_t>(payloadSpec.nameBuffer),
                 static_cast<int32_t>(payloadSpec.blockBuffer),
                 0,
                 16,
                 mode);
    AppendPlan(plan,
               BuildRead800179B4Plan(payloadSpec.nameBuffer,
                                      payloadSpec.blockBuffer,
                                      payloadSpec.requiredBlockCount,
                                      readEventKnown,
                                      readEventResult80016EB8));
    AppendAction(plan,
                 CardHandoffActionKind::GateLowerCardPayloadProducer80019D7C,
                 payloadSpec.stateFunction,
                 static_cast<int32_t>(payloadSpec.payloadAddress),
                 static_cast<int32_t>(payloadSpec.blockBuffer),
                 readEventKnown ? readEventResult80016EB8 : -1,
                 payloadKnown ? 1 : 0,
                 payloadAuthorized ? 1 : 0,
                 23,
                 mode);
    AppendAction(plan, CardHandoffActionKind::LoadPayload800164B4,
                 payloadSpec.loadFunction,
                 static_cast<int32_t>(payloadSpec.payloadAddress),
                 static_cast<int32_t>(kProgressBank80092F10),
                 static_cast<int32_t>(payloadSpec.payloadByteCount),
                 payloadAuthorized ? 1 : 0,
                 payloadAuthorized ? 1 : 0,
                 23,
                 mode);
    AppendAction(plan,
                 CardHandoffActionKind::GateLoadedPayloadSelector80019D7C,
                 payloadSpec.stateFunction,
                 static_cast<int32_t>(kProgressBank80092F10),
                 static_cast<int32_t>(kReplaySelOffset80092F3C),
                 payloadAuthorized ? 1 : 0,
                 static_cast<int32_t>(kFn800193F4),
                 payloadAuthorized ? 1 : 0,
                 23,
                 mode);
    if (!payloadAuthorized) {
        AppendAction(plan, CardHandoffActionKind::Gap,
                     payloadSpec.loadFunction,
                     static_cast<int32_t>(payloadSpec.payloadAddress),
                     static_cast<int32_t>(kProgressBank80092F10),
                     static_cast<int32_t>(payloadSpec.payloadByteCount));
    }
    AppendAction(plan, CardHandoffActionKind::SetGp720,
                 payloadSpec.stateFunction,
                 static_cast<int32_t>(kGpCardResultOffset),
                 1);
    AppendAction(plan, CardHandoffActionKind::ReturnScene,
                 payloadSpec.stateFunction, 23, 0, 0, 0, 23);
    return plan;
}

CardHandoffPlan BuildCase17HiScoreRefresh80019D7CPlan(
    bool a2Known,
    int32_t a2,
    bool word8007ABE4Known,
    int32_t word8007ABE4,
    const Case17RowFeedback80019D7C* rowFeedback,
    uint32_t rowFeedbackCount)
{
    const LowerCardPayloadProducerSpec& payloadSpec =
        LowerCardPayloadCase17Producer80019D7C();
    const LowerCardResultSpec& readSuccessSpec =
        LowerCardReadSuccess800179B4Spec();
    CardHandoffPlan plan = MakePlan("Case17HiScoreRefresh80019D7C");
    plan.outcome = HandoffOutcome::PendingCardLoop;
    plan.blockedByP0Gap = true;

    AppendAction(plan, CardHandoffActionKind::Call80019D7C,
                 kFn80019D7C,
                 17,
                 a2Known ? a2 : -1,
                 static_cast<int32_t>(CardMode800191E4::HiScore));
    if (!a2Known) {
        AppendAction(plan, CardHandoffActionKind::Gap, kFn80019D7C, 17);
        return plan;
    }
    if (a2 == 3) {
        plan.outcome = HandoffOutcome::ContinueLoop;
        AppendAction(plan, CardHandoffActionKind::ReturnScene,
                     kFn80019D7C, 5, 0, 0, 0, 5);
        return plan;
    }

    AppendAction(plan, CardHandoffActionKind::ClearHiScoreBank800168DC,
                 kFn800168DC);
    AppendAction(plan, CardHandoffActionKind::GateWord8007ABE4,
                 kFn80019D7C,
                 static_cast<int32_t>(kWord8007ABE4),
                 word8007ABE4Known ? word8007ABE4 : -1);
    if (!word8007ABE4Known) {
        AppendAction(plan, CardHandoffActionKind::Gap, kFn80019D7C,
                     static_cast<int32_t>(kWord8007ABE4));
        return plan;
    }

    if (word8007ABE4 != 0) {
        bool case17PayloadBlocked = false;
        AppendAction(plan, CardHandoffActionKind::VSyncCallback80017F38,
                     kFn80017F38,
                     static_cast<int32_t>(kGpCase17VsyncAOffset),
                     static_cast<int32_t>(kGpCase17VsyncBOffset));
        for (uint32_t row = 0; row < kCase17CardRowCount80019D7C; ++row) {
            const Case17RowFeedback80019D7C* feedback =
                rowFeedback != nullptr && row < rowFeedbackCount ?
                    &rowFeedback[row] : nullptr;
            const bool rowFeedbackKnown = feedback != nullptr;
            const bool rowEnabledKnown =
                rowFeedbackKnown && feedback->rowEnabledKnown;
            const bool rowEnabled = rowEnabledKnown && feedback->rowEnabled;
            const bool readProducerKnown =
                rowEnabled && feedback->pathBuilt800173A8 &&
                feedback->openKnown800173A8 &&
                feedback->readSubmitted800173A8 &&
                feedback->swCardEventSourceKnown80016EB8 &&
                feedback->closeKnown800179B4;
            const bool readSucceeded =
                readProducerKnown && feedback->readEventKnown &&
                LowerCardPollResultMatches(
                    readSuccessSpec,
                    feedback->readEventResult80016EB8);
            const bool metadataKnown =
                readSucceeded && feedback->metadataKnown;
            const bool payloadAuthorized =
                readSucceeded && feedback->payloadKnown && metadataKnown;
            case17PayloadBlocked =
                case17PayloadBlocked || !rowEnabledKnown ||
                (rowEnabled && !payloadAuthorized);

            Case17RowSpec80019D7C spec{};
            if (!BuildCase17RowSpec80019D7C(row, &spec)) {
                AppendAction(plan, CardHandoffActionKind::Gap,
                             kFn80019D7C, static_cast<int32_t>(row));
                continue;
            }
            AppendAction(plan, CardHandoffActionKind::ClearBlock80025C44,
                         kFn80025C44,
                         static_cast<int32_t>(spec.readBlockBuffer),
                         static_cast<int32_t>(kCardBlockBytes800179B4),
                         static_cast<int32_t>(row));
            AppendAction(plan, CardHandoffActionKind::GateCardRowEnabled,
                         kFn80019D7C,
                         static_cast<int32_t>(spec.enabledAddress),
                         static_cast<int32_t>(row),
                         rowEnabledKnown ? 1 : 0,
                         rowEnabled ? 1 : 0);
            AppendAction(plan, CardHandoffActionKind::CopyRowName80019D7C,
                         kFn80019D7C,
                         static_cast<int32_t>(spec.readNameBuffer),
                         static_cast<int32_t>(spec.rowAddress),
                         static_cast<int32_t>(row));
            AppendAction(plan, CardHandoffActionKind::Read800179B4,
                         payloadSpec.readFunction,
                         static_cast<int32_t>(spec.readNameBuffer),
                         static_cast<int32_t>(spec.readBlockBuffer),
                         payloadSpec.requiredBlockCount,
                         static_cast<int32_t>(payloadSpec.blockByteCount),
                         rowFeedbackKnown && feedback->readEventKnown ?
                             feedback->readEventResult80016EB8 : -1,
                         static_cast<int32_t>(row));
            AppendAction(plan,
                         CardHandoffActionKind::GateCase17ReadProducer80019D7C,
                         payloadSpec.readFunction,
                         static_cast<int32_t>(row),
                         rowEnabled ? 1 : 0,
                         readProducerKnown ? 1 : 0,
                         rowFeedbackKnown &&
                                 feedback->swCardEventSourceKnown80016EB8 ?
                             1 : 0,
                         rowFeedbackKnown && feedback->closeKnown800179B4 ?
                             1 : 0);
            AppendAction(plan, CardHandoffActionKind::GateReadSuccess800179B4,
                         payloadSpec.readFunction,
                         static_cast<int32_t>(row),
                         rowFeedbackKnown && feedback->readEventKnown ?
                             feedback->readEventResult80016EB8 : -1,
                         readSucceeded ? 1 : 0,
                         rowEnabled ? 1 : 0);
            AppendAction(plan,
                         CardHandoffActionKind::GateCase17PayloadProducer80019D7C,
                         payloadSpec.stateFunction,
                         static_cast<int32_t>(spec.payloadAddress),
                         static_cast<int32_t>(spec.sourceMetadataAddress),
                         static_cast<int32_t>(row),
                         feedback != nullptr && feedback->payloadKnown ? 1 : 0,
                         payloadAuthorized ? 1 : 0);
            AppendAction(plan, CardHandoffActionKind::WriteRowMetadata80019D7C,
                         payloadSpec.stateFunction,
                         static_cast<int32_t>(spec.metadataAddress),
                         static_cast<int32_t>(spec.sourceMetadataAddress),
                         static_cast<int32_t>(row),
                         metadataKnown ? 1 : 0,
                         metadataKnown ?
                             static_cast<int32_t>(
                                 feedback->metadata8007AE14 & 0xFFFFu) : -1);
            AppendAction(plan, CardHandoffActionKind::MergePayload800164F8,
                         payloadSpec.loadFunction,
                         static_cast<int32_t>(spec.payloadAddress),
                         static_cast<int32_t>(row),
                         payloadAuthorized ? 1 : 0);
        }
        AppendAction(plan, CardHandoffActionKind::VSyncCallback80017F38,
                     kFn80019D7C, 0);
        if (case17PayloadBlocked) {
            AppendAction(plan, CardHandoffActionKind::Gap,
                         payloadSpec.loadFunction,
                         static_cast<int32_t>(payloadSpec.payloadAddress),
                         static_cast<int32_t>(payloadSpec.metadataAddress),
                         static_cast<int32_t>(payloadSpec.rowCount));
        }
    }

    AppendAction(plan, CardHandoffActionKind::SetGp720,
                 kFn80019D7C,
                 static_cast<int32_t>(kGpCardResultOffset),
                 1);
    AppendAction(plan, CardHandoffActionKind::ReturnScene,
                 kFn80019D7C, 23, 0, 0, 0, 23);
    return plan;
}

CardHandoffPlan BuildWrite80017A10Plan(uint32_t namePtr,
                                       uint32_t blockBuffer,
                                       int32_t blocks,
                                       bool resultKnown,
                                       bool succeeds,
                                       uint32_t successAttempt,
                                       int32_t terminalPoll80016EB8,
                                       int32_t scanResult80017900)
{
    const CardEventPollSourceSpec& pollSpec = CardEventPollSw80016EB8Spec();
    const LowerCardResultSpec& successSpec =
        LowerCardWriteSuccess80017A10Spec();
    const LowerCardResultSpec& failureSpec =
        LowerCardWriteFailure80017A10Spec();
    CardHandoffPlan plan = MakePlan("Write80017A10");
    plan.outcome = HandoffOutcome::PendingCardLoop;
    plan.blockedByP0Gap = true;

    if (successAttempt >= kCardWriteRetryCount80017A10) {
        successAttempt = kCardWriteRetryCount80017A10 - 1u;
    }
    const int32_t byteCount = blocks << 13;
    const uint32_t attemptsToRecord =
        resultKnown && succeeds ? successAttempt + 1u :
        kCardWriteRetryCount80017A10;

    AppendAction(plan, CardHandoffActionKind::Write80017A10,
                 kFn80017A10,
                 static_cast<int32_t>(namePtr),
                 static_cast<int32_t>(blockBuffer),
                 blocks,
                 static_cast<int32_t>(kCardWriteRetryCount80017A10));

    for (uint32_t attempt = 0; attempt < attemptsToRecord; ++attempt) {
        const bool terminalSuccess =
            resultKnown && succeeds && attempt == successAttempt;
        const bool terminalFailure =
            resultKnown && !succeeds &&
            attempt == kCardWriteRetryCount80017A10 - 1u;
        const bool terminalKnown = terminalSuccess || terminalFailure;
        const int32_t scanResult = resultKnown ? scanResult80017900 : -1;
        const int32_t checkExists =
            resultKnown ? (scanResult80017900 == 1 ? 0 : 1) : -1;
        const int32_t pollResult = terminalSuccess ? 1 :
            terminalFailure ? terminalPoll80016EB8 : -1;
        const bool terminalSuccessPoll =
            terminalSuccess &&
            LowerCardPollResultMatches(successSpec, pollResult);

        AppendAction(plan,
                     CardHandoffActionKind::ScanCardDirectory80017900,
                     kFn80017900,
                     static_cast<int32_t>(namePtr),
                     static_cast<int32_t>(kCardDirectoryRows8007A318),
                     static_cast<int32_t>(kCardDirectoryRowCount80017900),
                     static_cast<int32_t>(attempt),
                     scanResult);
        AppendAction(plan, CardHandoffActionKind::Call80017454,
                     kFn80017454,
                     static_cast<int32_t>(kGpCardPortOffset),
                     static_cast<int32_t>(kGpCardSlotOffset),
                     static_cast<int32_t>(namePtr),
                     static_cast<int32_t>(blockBuffer),
                     0,
                     checkExists);
        AppendAction(plan, CardHandoffActionKind::FormatCardPath80017454,
                     kFn8004800C,
                     static_cast<int32_t>(namePtr),
                     blocks,
                     byteCount,
                     static_cast<int32_t>(attempt));
        if (!resultKnown || checkExists != 0) {
            AppendAction(plan, CardHandoffActionKind::OpenCheck80017454,
                         kFn80017454,
                         static_cast<int32_t>(
                             (blocks << 16) |
                             kCardWriteOpenCheckFlags80017454),
                         checkExists,
                         static_cast<int32_t>(attempt));
        }
        AppendAction(plan, CardHandoffActionKind::OpenWrite80017454,
                     kFn80017454,
                     static_cast<int32_t>(kCardWriteOpenFlags80017454),
                     static_cast<int32_t>(kGpCardFdOffset),
                     static_cast<int32_t>(attempt));
        AppendAction(plan, CardHandoffActionKind::ClearEvents80016FC0,
                     kFn80016FC0,
                     static_cast<int32_t>(kGpCardSwEvent0Offset),
                     4,
                     static_cast<int32_t>(attempt));
        AppendAction(plan, CardHandoffActionKind::SubmitCardWrite80017454,
                     kFn80017454,
                     static_cast<int32_t>(blockBuffer),
                     byteCount,
                     blocks,
                     static_cast<int32_t>(attempt));
        AppendAction(plan, CardHandoffActionKind::WaitCard80035560,
                     kFn80035560, 4, static_cast<int32_t>(attempt));
        AppendAction(plan, CardHandoffActionKind::PollEvents80016EB8,
                     pollSpec.pollFunction,
                     pollResult,
                     static_cast<int32_t>(pollSpec.localTimeoutCount),
                     static_cast<int32_t>(attempt));
        AppendAction(plan, CardHandoffActionKind::GateSwCardEventSource80016EB8,
                     pollSpec.pollFunction,
                     static_cast<int32_t>(pollSpec.eventClass),
                     static_cast<int32_t>(pollSpec.firstHandleGpOffset),
                     static_cast<int32_t>(pollSpec.handleCount),
                     static_cast<int32_t>(pollSpec.localTimeoutCount),
                     terminalKnown ? pollResult : -1,
                     (!terminalKnown ||
                      IsCardEventPollResult(pollResult)) ? 1 : 0);
        AppendAction(plan, CardHandoffActionKind::GateSwPollResult80016EB8,
                     pollSpec.pollFunction,
                     terminalKnown ? 1 : 0,
                     terminalKnown ? pollResult : -1,
                     pollSpec.minEventReturn,
                     pollSpec.maxEventReturn,
                     (!terminalKnown ||
                      IsCardEventPollResult(pollResult)) ? 1 : 0,
                     static_cast<int32_t>(attempt));
        AppendAction(plan, CardHandoffActionKind::CloseGp696,
                     kFn80017A10,
                     static_cast<int32_t>(kGpCardFdOffset),
                     static_cast<int32_t>(attempt));

        if (terminalSuccess) {
            AppendAction(plan,
                         CardHandoffActionKind::GateWriteSuccess80017A10,
                         kFn80017A10,
                         LowerCardFunctionResult(successSpec, pollResult),
                         static_cast<int32_t>(attempt),
                         terminalSuccessPoll ? 1 : 0,
                         0,
                         0);
            if (!terminalSuccessPoll) {
                AppendAction(plan, CardHandoffActionKind::Gap,
                             successSpec.pollFunction,
                             pollResult,
                             static_cast<int32_t>(attempt));
            }
            return plan;
        }
    }

    if (resultKnown && !succeeds) {
        if (!IsCardEventPollResult(terminalPoll80016EB8) ||
            !LowerCardPollResultMatches(failureSpec,
                                        terminalPoll80016EB8)) {
            AppendAction(plan, CardHandoffActionKind::Gap,
                         failureSpec.pollFunction,
                         terminalPoll80016EB8,
                         static_cast<int32_t>(
                             kCardWriteRetryCount80017A10 - 1u));
        }
        AppendAction(plan, CardHandoffActionKind::GateWriteSuccess80017A10,
                     kFn80017A10,
                     LowerCardFunctionResult(failureSpec,
                                             terminalPoll80016EB8),
                     static_cast<int32_t>(
                         kCardWriteRetryCount80017A10 - 1u),
                     LowerCardPollResultMatches(failureSpec,
                                                terminalPoll80016EB8) ? 1 : 0,
                     failureSpec.requiresTerminalAttempt ? 1 : 0,
                     -1);
        return plan;
    }

    AppendAction(plan, CardHandoffActionKind::Gap, kFn80017A10,
                 static_cast<int32_t>(namePtr),
                 static_cast<int32_t>(blockBuffer),
                 blocks,
                 static_cast<int32_t>(kCardWriteRetryCount80017A10),
                 -1);
    return plan;
}

CardHandoffPlan BuildFormat80017B60Plan(bool terminalKnown,
                                        uint32_t terminalAttempt,
                                        int32_t terminalPoll80017008)
{
    const CardEventPollSourceSpec& pollSpec = CardEventPollHw80017008Spec();
    const LowerCardResultSpec& successSpec =
        LowerCardFormatSuccess80017B60Spec();
    const LowerCardResultSpec& poll3FailureSpec =
        LowerCardFormatPoll3Failure80017B60Spec();
    const LowerCardResultSpec& retryExhaustedSpec =
        LowerCardFormatRetryExhausted80017B60Spec();
    CardHandoffPlan plan = MakePlan("Format80017B60");
    plan.outcome = HandoffOutcome::PendingCardLoop;
    plan.blockedByP0Gap = true;

    if (terminalAttempt >= kCardFormatRetryCount80017B60) {
        terminalAttempt = kCardFormatRetryCount80017B60 - 1u;
    }
    const uint32_t attemptsToRecord =
        terminalKnown ? terminalAttempt + 1u :
        kCardFormatRetryCount80017B60;

    AppendAction(plan, CardHandoffActionKind::FormatCard80017B60,
                 kFn80017B60,
                 static_cast<int32_t>(kCardFormatRetryCount80017B60));
    AppendAction(plan, CardHandoffActionKind::FormatCardPath80017B60,
                 kFn8004800C,
                 static_cast<int32_t>(kGpCardPortOffset),
                 static_cast<int32_t>(kGpCardSlotOffset));

    for (uint32_t attempt = 0; attempt < attemptsToRecord; ++attempt) {
        const bool terminal = terminalKnown && attempt == terminalAttempt;
        const int32_t pollResult =
            terminal ? terminalPoll80017008 : -1;
        AppendAction(plan, CardHandoffActionKind::DrainHwEvents8001707C,
                     kFn8001707C,
                     static_cast<int32_t>(kGpCardHwEvent0Offset),
                     4,
                     static_cast<int32_t>(attempt));
        AppendAction(plan, CardHandoffActionKind::SubmitFormat80017B60,
                     kFn80017B60,
                     static_cast<int32_t>(attempt));
        AppendAction(plan, CardHandoffActionKind::PollHwEvents80017008,
                     pollSpec.pollFunction,
                     pollResult,
                     static_cast<int32_t>(attempt));
        AppendAction(plan, CardHandoffActionKind::GateHwCardEventSource80017008,
                     pollSpec.pollFunction,
                     static_cast<int32_t>(pollSpec.eventClass),
                     static_cast<int32_t>(pollSpec.firstHandleGpOffset),
                     static_cast<int32_t>(pollSpec.handleCount),
                     static_cast<int32_t>(attempt),
                     terminal ? pollResult : -1,
                     (!terminal ||
                      IsCardEventPollResult(pollResult)) ? 1 : 0);
        AppendAction(plan, CardHandoffActionKind::GateHwPollResult80017008,
                     pollSpec.pollFunction,
                     terminal ? 1 : 0,
                     terminal ? pollResult : -1,
                     pollSpec.minEventReturn,
                     pollSpec.maxEventReturn,
                     (!terminal ||
                      IsCardEventPollResult(pollResult)) ? 1 : 0,
                     static_cast<int32_t>(attempt));
    }

    if (terminalKnown) {
        const bool pollValid =
            IsCardEventPollResult(terminalPoll80017008);
        const bool pollSuccess =
            pollValid &&
            LowerCardPollResultMatches(successSpec,
                                       terminalPoll80017008);
        const bool poll3Failure =
            pollValid &&
            LowerCardPollResultMatches(poll3FailureSpec,
                                       terminalPoll80017008);
        const bool retryExhausted =
            pollValid &&
            terminalAttempt == kCardFormatRetryCount80017B60 - 1u &&
            LowerCardPollResultMatches(retryExhaustedSpec,
                                       terminalPoll80017008);
        const bool validTerminal =
            pollSuccess || poll3Failure || retryExhausted;
        const int32_t formatResult =
            pollSuccess ?
                LowerCardFunctionResult(successSpec,
                                        terminalPoll80017008) :
            poll3Failure ?
                LowerCardFunctionResult(poll3FailureSpec,
                                        terminalPoll80017008) :
            retryExhausted ?
                LowerCardFunctionResult(retryExhaustedSpec,
                                        terminalPoll80017008) : 0;
        AppendAction(plan, CardHandoffActionKind::GateFormatResult80017B60,
                     kFn80017B60,
                     terminalPoll80017008,
                     static_cast<int32_t>(terminalAttempt),
                     pollSuccess ? 1 : 0,
                     retryExhausted ? 1 : 0,
                     validTerminal ? formatResult : 0);
        if (!validTerminal) {
            AppendAction(plan, CardHandoffActionKind::Gap, kFn80017B60,
                         terminalPoll80017008,
                         static_cast<int32_t>(terminalAttempt));
        }
        return plan;
    }

    AppendAction(plan, CardHandoffActionKind::Gap, kFn80017B60,
                 static_cast<int32_t>(kCardFormatRetryCount80017B60));
    return plan;
}

CardHandoffPlan BuildState14Format80019458Plan(
    bool ioKnown,
    int32_t ioResult,
    bool terminalKnown,
    uint32_t terminalAttempt,
    int32_t terminalPoll80017008)
{
    CardHandoffPlan plan = MakePlan("State14Format80019458");
    plan.outcome = HandoffOutcome::PendingCardLoop;
    plan.blockedByP0Gap = true;

    AppendAction(plan, CardHandoffActionKind::State14Format80019458,
                 kFn80019458,
                 14,
                 ioKnown ? ioResult : -1,
                 static_cast<int32_t>(CardMode800191E4::Save),
                 0,
                 0,
                 ioKnown ? (ioResult == 3 ? 2 : 4) : -1,
                 CardMode800191E4::Save,
                 CardEventFrameId::FormatPrompt);
    if (!ioKnown) {
        AppendAction(plan, CardHandoffActionKind::Gap, kFn80019458, 14);
        return plan;
    }
    if (ioResult == 3) {
        AppendAction(plan, CardHandoffActionKind::ReturnScene,
                     kFn80019458, 2, 0, 0, 0, 2, 2,
                     CardMode800191E4::Save,
                     CardEventFrameId::FormatPrompt);
        return plan;
    }

    AppendPlan(plan,
               BuildFormat80017B60Plan(terminalKnown,
                                       terminalAttempt,
                                       terminalPoll80017008));
    AppendAction(plan, CardHandoffActionKind::GateState14Format80019458,
                 kFn80019458,
                 ioResult,
                 terminalKnown ? 1 : 0,
                 terminalKnown ? static_cast<int32_t>(terminalAttempt) : -1,
                 terminalKnown ? terminalPoll80017008 : -1,
                 terminalKnown ? terminalPoll80017008 : -1,
                 4,
                 CardMode800191E4::Save,
                 CardEventFrameId::FormatPrompt);
    AppendAction(plan, CardHandoffActionKind::ReturnScene,
                 kFn80019458, 4, 0, 0, 0, 4, 4,
                 CardMode800191E4::Save,
                 CardEventFrameId::FormatPrompt);
    return plan;
}

CardHandoffPlan BuildSaveTickIoGate80019458Plan(int32_t state,
                                                bool ioKnown,
                                                int32_t ioResult)
{
    CardHandoffPlan plan = MakePlan("SaveTickIoGate80019458");
    plan.outcome = HandoffOutcome::PendingCardLoop;
    plan.blockedByP0Gap = true;

    if (state != 8 && state != 10 && state != 11) {
        AppendAction(plan, CardHandoffActionKind::Gap,
                     kFn80019458, state);
        return plan;
    }

    const int32_t io3NextState = state == 8 ? 5 : 4;
    const int32_t nextState =
        ioKnown && ioResult == 3 ? io3NextState : state;
    AppendAction(plan, CardHandoffActionKind::SaveTickIoGate80019458,
                 kFn80019458,
                 state,
                 ioKnown ? ioResult : -1,
                 io3NextState,
                 nextState,
                 ioKnown ? ioResult : -1,
                 nextState,
                 CardMode800191E4::Save,
                 state == 8 ? CardEventFrameId::FormatPrompt
                            : CardEventFrameId::Unknown);
    if (!ioKnown) {
        AppendAction(plan, CardHandoffActionKind::Gap,
                     kFn80019458, state);
        return plan;
    }

    AppendAction(plan, CardHandoffActionKind::ReturnScene,
                 kFn80019458,
                 nextState,
                 0,
                 0,
                 0,
                 nextState,
                 nextState,
                 CardMode800191E4::Save,
                 state == 8 ? CardEventFrameId::FormatPrompt
                            : CardEventFrameId::Unknown);
    if (state == 8 && ioResult == 3) {
        AppendPlan(plan, BuildCardStateRoute800180D8Plan(
                             5, CardMode800191E4::Save));
    }
    return plan;
}

CardHandoffPlan BuildCardIoPoll80017594Plan(
    const CardIoState80017594& inputState,
    bool inputStateKnown,
    bool pollSwKnown80016E18,
    int32_t pollSwResult80016E18,
    bool pollHwKnown80017008,
    int32_t pollHwResult80017008)
{
    const CardEventPollSourceSpec& swPollSpec = CardEventPollSw80016E18Spec();
    const CardEventPollSourceSpec& hwPollSpec = CardEventPollHw80017008Spec();
    CardHandoffPlan plan = MakePlan("CardIoPoll80017594");
    plan.outcome = HandoffOutcome::PendingCardLoop;
    plan.blockedByP0Gap = true;

    const CardIoState80017594 unknownState{};
    const CardIoState80017594& before =
        inputStateKnown ? inputState : unknownState;
    AppendAction(plan, CardHandoffActionKind::Poll80017594,
                 kFn80017594,
                 inputStateKnown ? inputState.dword800917E8 : -1,
                 inputStateKnown ? inputState.dword800917EC : -1,
                 inputStateKnown ? inputState.dword800917F0 : -1,
                 inputStateKnown ? inputState.dword800917F4 : -1,
                 0,
                 inputStateKnown ? inputState.gp700 : -1);
    if (!inputStateKnown) {
        AppendAction(plan, CardHandoffActionKind::Gap, kFn80017594);
        return plan;
    }

    CardIoState80017594 after = before;
    const auto appendPollSw = [&]() -> bool {
        after.gp700 = before.gp700 - 1;
        AppendAction(plan, CardHandoffActionKind::PollSwEvents80016E18,
                     swPollSpec.pollFunction,
                     before.dword800917E8,
                     before.gp700,
                     pollSwKnown80016E18 ? pollSwResult80016E18 : -1,
                     after.gp700);
        AppendAction(plan, CardHandoffActionKind::GateSwCardEventSource80016E18,
                     swPollSpec.pollFunction,
                     static_cast<int32_t>(swPollSpec.eventClass),
                     static_cast<int32_t>(swPollSpec.firstHandleGpOffset),
                     static_cast<int32_t>(swPollSpec.handleCount),
                     before.gp700,
                     pollSwKnown80016E18 ? pollSwResult80016E18 : -1,
                     (pollSwKnown80016E18 &&
                      pollSwResult80016E18 >= swPollSpec.noEventReturn &&
                      pollSwResult80016E18 <= swPollSpec.maxEventReturn &&
                      (after.gp700 >= 0 || pollSwResult80016E18 == 2)) ? 1 : 0);
        AppendAction(plan, CardHandoffActionKind::GateSwPollCounter80016E18,
                     swPollSpec.pollFunction,
                     static_cast<int32_t>(swPollSpec.firstHandleGpOffset),
                     static_cast<int32_t>(swPollSpec.handleCount),
                     before.gp700,
                     after.gp700,
                     pollSwKnown80016E18 ? pollSwResult80016E18 : -1,
                     after.gp700 < 0 ? 1 : 0);
        if (!pollSwKnown80016E18) {
            AppendAction(plan, CardHandoffActionKind::Gap, swPollSpec.pollFunction,
                         before.dword800917E8, before.gp700);
            return false;
        }
        if (pollSwResult80016E18 < swPollSpec.noEventReturn ||
            pollSwResult80016E18 > swPollSpec.maxEventReturn) {
            AppendAction(plan, CardHandoffActionKind::Gap,
                         swPollSpec.pollFunction,
                         before.dword800917E8, before.gp700,
                         pollSwResult80016E18, after.gp700);
            return false;
        }
        if (after.gp700 < 0 && pollSwResult80016E18 != 2) {
            AppendAction(plan, CardHandoffActionKind::Gap,
                         swPollSpec.pollFunction,
                         before.dword800917E8, before.gp700,
                         pollSwResult80016E18, after.gp700);
            return false;
        }
        return true;
    };

    switch (before.dword800917E8) {
    case 0:
        AppendAction(plan, CardHandoffActionKind::CardInfo80017594,
                     kFn80017594, 0);
        after.dword800917E8 = 1;
        after.dword800917EC = 0;
        after.gp700 = 300;
        AppendCardIoFeedbackGate80017594(
            plan, CardIoFeedbackTransitionKind::State0InfoSubmit80017594,
            before, after, false, -1, false, -1, true);
        AppendAction(plan, CardHandoffActionKind::Gap, kFn80017594, 0);
        AppendCardIoReturn(plan, before, after, after.dword800917EC);
        return plan;

    case 1:
        if (!appendPollSw()) {
            return plan;
        }
        if (pollSwResult80016E18 == 0) {
            AppendCardIoFeedbackGate80017594(
                plan,
                CardIoFeedbackTransitionKind::State1InfoNoEvent80016E18,
                before, after, true, pollSwResult80016E18, false, -1,
                true);
            AppendCardIoReturn(plan, before, after, before.dword800917EC);
            return plan;
        }
        if (pollSwResult80016E18 == 1) {
            after.dword800917F0 = 1;
            after.dword800917E8 =
                before.dword800917F4 == 1 ? 4 : 2;
            AppendCardIoFeedbackGate80017594(
                plan,
                CardIoFeedbackTransitionKind::State1InfoSuccess80016E18,
                before, after, true, pollSwResult80016E18, false, -1,
                true);
            AppendCardIoReturn(plan, before, after, before.dword800917EC);
            return plan;
        }
        if (pollSwResult80016E18 == 3) {
            after.dword800917F0 = 3;
            after.dword800917E8 = 4;
            after.dword800917F4 = 0;
            AppendCardIoFeedbackGate80017594(
                plan,
                CardIoFeedbackTransitionKind::State1InfoError80016E18,
                before, after, true, pollSwResult80016E18, false, -1,
                true);
            AppendCardIoReturn(plan, before, after, before.dword800917EC);
            return plan;
        }
        if (pollSwResult80016E18 == 4) {
            after.dword800917F0 = 4;
            AppendAction(plan, CardHandoffActionKind::DrainHwEvents8001707C,
                         kFn8001707C,
                         static_cast<int32_t>(kGpCardHwEvent0Offset),
                         4);
            AppendAction(plan, CardHandoffActionKind::ResetHwEvents80047EE4,
                         kFn80047EE4, 0, 63, 0);
            AppendAction(plan, CardHandoffActionKind::PollHwEvents80017008,
                         hwPollSpec.pollFunction,
                         pollHwKnown80017008 ? pollHwResult80017008 : -1);
            AppendAction(plan,
                         CardHandoffActionKind::GateHwCardEventSource80017008,
                         hwPollSpec.pollFunction,
                         static_cast<int32_t>(hwPollSpec.eventClass),
                         static_cast<int32_t>(hwPollSpec.firstHandleGpOffset),
                         static_cast<int32_t>(hwPollSpec.handleCount),
                         before.dword800917E8,
                         pollHwKnown80017008 ? pollHwResult80017008 : -1,
                         (!pollHwKnown80017008 ||
                          IsCardEventPollResult(pollHwResult80017008)) ? 1 : 0);
            AppendAction(plan, CardHandoffActionKind::GateHwPollResult80017008,
                         hwPollSpec.pollFunction,
                         pollHwKnown80017008 ? 1 : 0,
                         pollHwKnown80017008 ? pollHwResult80017008 : -1,
                         hwPollSpec.minEventReturn,
                         hwPollSpec.maxEventReturn,
                         (!pollHwKnown80017008 ||
                          IsCardEventPollResult(pollHwResult80017008)) ? 1 : 0);
            if (!pollHwKnown80017008) {
                AppendAction(plan, CardHandoffActionKind::Gap,
                             hwPollSpec.pollFunction,
                             before.dword800917E8);
            }
            if (pollHwKnown80017008 &&
                !IsCardEventPollResult(pollHwResult80017008)) {
                AppendAction(plan, CardHandoffActionKind::Gap,
                             hwPollSpec.pollFunction,
                             pollHwResult80017008,
                             before.dword800917E8);
            }
            after.dword800917E8 = 2;
            after.dword800917F4 = 0;
            AppendCardIoFeedbackGate80017594(
                plan,
                CardIoFeedbackTransitionKind::
                    State1InfoNewCardReset80016E18_80017008,
                before, after, true, pollSwResult80016E18,
                pollHwKnown80017008, pollHwResult80017008,
                pollHwKnown80017008 &&
                    IsCardEventPollResult(pollHwResult80017008));
            AppendCardIoReturn(plan, before, after, before.dword800917EC);
            return plan;
        }
        after.dword800917F0 = -3;
        after.dword800917E8 = 4;
        after.dword800917F4 = 0;
        AppendCardIoFeedbackGate80017594(
            plan, CardIoFeedbackTransitionKind::State1InfoTimeout80016E18,
            before, after, true, pollSwResult80016E18, false, -1, true);
        AppendCardIoReturn(plan, before, after, before.dword800917EC);
        return plan;

    case 2:
        AppendAction(plan, CardHandoffActionKind::ClearEvents80016FC0,
                     kFn80016FC0,
                     static_cast<int32_t>(kGpCardSwEvent0Offset),
                     4);
        AppendAction(plan, CardHandoffActionKind::CardLoad80017594,
                     kFn80017594, 0);
        after.dword800917E8 = 3;
        after.gp700 = 300;
        AppendCardIoFeedbackGate80017594(
            plan, CardIoFeedbackTransitionKind::State2LoadSubmit80017594,
            before, after, false, -1, false, -1, true);
        AppendAction(plan, CardHandoffActionKind::Gap, kFn80017594, 2);
        AppendCardIoReturn(plan, before, after, before.dword800917EC);
        return plan;

    case 3:
        if (!appendPollSw()) {
            return plan;
        }
        if (pollSwResult80016E18 == 0) {
            AppendCardIoFeedbackGate80017594(
                plan,
                CardIoFeedbackTransitionKind::State3LoadNoEvent80016E18,
                before, after, true, pollSwResult80016E18, false, -1,
                true);
            AppendCardIoReturn(plan, before, after, before.dword800917EC);
            return plan;
        }
        after.dword800917E8 = 4;
        after.dword800917F4 = 0;
        if (pollSwResult80016E18 == 1) {
            after.dword800917F4 = 1;
            AppendCardIoFeedbackGate80017594(
                plan,
                CardIoFeedbackTransitionKind::State3LoadSuccess80016E18,
                before, after, true, pollSwResult80016E18, false, -1,
                true);
        } else if (pollSwResult80016E18 == 3) {
            after.dword800917F0 = 3;
            AppendCardIoFeedbackGate80017594(
                plan,
                CardIoFeedbackTransitionKind::State3LoadError80016E18,
                before, after, true, pollSwResult80016E18, false, -1,
                true);
        } else if (pollSwResult80016E18 == 4) {
            after.dword800917F0 = 5;
            AppendCardIoFeedbackGate80017594(
                plan,
                CardIoFeedbackTransitionKind::State3LoadNewCard80016E18,
                before, after, true, pollSwResult80016E18, false, -1,
                true);
        } else {
            after.dword800917F0 = 2;
            AppendCardIoFeedbackGate80017594(
                plan,
                CardIoFeedbackTransitionKind::State3LoadTimeout80016E18,
                before, after, true, pollSwResult80016E18, false, -1,
                true);
        }
        AppendCardIoReturn(plan, before, after, before.dword800917EC);
        return plan;

    case 4:
        after.dword800917E8 = 0;
        after.dword800917EC = before.dword800917F0;
        AppendCardIoFeedbackGate80017594(
            plan, CardIoFeedbackTransitionKind::State4PublishPending80017594,
            before, after, false, -1, false, -1, true);
        AppendCardIoReturn(plan, before, after, after.dword800917EC);
        return plan;

    default:
        AppendAction(plan, CardHandoffActionKind::Gap, kFn80017594,
                     before.dword800917E8);
        AppendCardIoReturn(plan, before, after, before.dword800917EC);
        return plan;
    }
}

CardHandoffPlan BuildState15SaveBlock80019458Plan(uint32_t savePtr,
                                                  bool ioKnown,
                                                  int32_t ioResult,
                                                  bool payloadKnown,
                                                  bool writeResultKnown,
                                                  bool writeSucceeded)
{
    CardHandoffPlan plan = MakePlan("State15SaveBlock80019458");
    plan.outcome = HandoffOutcome::PendingCardLoop;
    plan.blockedByP0Gap = true;

    AppendAction(plan, CardHandoffActionKind::State15Save80019458,
                 kFn80019458,
                 15,
                 ioKnown ? ioResult : -1,
                 static_cast<int32_t>(savePtr));
    if (!ioKnown) {
        AppendAction(plan, CardHandoffActionKind::Gap, kFn80019458, 15);
        return plan;
    }
    if (ioResult == 3) {
        AppendAction(plan, CardHandoffActionKind::ReturnScene,
                     kFn80019458, 2, 0, 0, 0, 2);
        return plan;
    }

    AppendAction(plan, CardHandoffActionKind::BuildSaveFilename80019458,
                 kFn8004800C,
                 static_cast<int32_t>(kCardNameBuffer8007CBE8),
                 static_cast<int32_t>(kGpCardFilenamePrefixOffset),
                 static_cast<int32_t>(savePtr + 1u));
    AppendAction(plan, CardHandoffActionKind::BuildSaveTitle80019458,
                 kFn8004800C,
                 static_cast<int32_t>(kCardEncodedTitleBuffer8007CC08),
                 static_cast<int32_t>(kGpCardTitlePrefixOffset),
                 static_cast<int32_t>(savePtr + 1u),
                 static_cast<int32_t>(kCardTitleSuffixMax80019458));
    AppendAction(plan, CardHandoffActionKind::ClearSaveBlock80025C44,
                 kFn80025C44,
                 static_cast<int32_t>(kCardBlockBuffer8007ABE8),
                 static_cast<int32_t>(kCardBlockBytes800179B4));
    AppendAction(plan, CardHandoffActionKind::BuildSaveHeader80017C08,
                 kFn80017C08,
                 static_cast<int32_t>(kCardBlockBuffer8007ABE8),
                 static_cast<int32_t>(kCardEncodedTitleBuffer8007CC08),
                 static_cast<int32_t>(kCardHeaderBytes80017C08));
    AppendAction(plan,
                 CardHandoffActionKind::GateState15PayloadSource80019458,
                 kFn80019458,
                 static_cast<int32_t>(savePtr),
                 static_cast<int32_t>(savePtr + 1u),
                 static_cast<int32_t>(kCardPayloadBuffer8007ADE8),
                 static_cast<int32_t>(kCardSavePayloadBytes80019458),
                 payloadKnown ? 1 : 0,
                 15,
                 CardMode800191E4::Save);
    AppendAction(plan, CardHandoffActionKind::CopyState15Payload80025C64,
                 kFn80025C64,
                 static_cast<int32_t>(savePtr),
                 static_cast<int32_t>(kCardPayloadBuffer8007ADE8),
                 static_cast<int32_t>(kCardSavePayloadBytes80019458),
                 payloadKnown ? 1 : 0);
    if (!payloadKnown) {
        AppendAction(plan, CardHandoffActionKind::Gap, kFn80025C64,
                     static_cast<int32_t>(savePtr),
                     static_cast<int32_t>(kCardPayloadBuffer8007ADE8),
                     static_cast<int32_t>(kCardSavePayloadBytes80019458));
    }

    AppendPlan(plan,
               BuildWrite80017A10Plan(kCardNameBuffer8007CBE8,
                                      kCardBlockBuffer8007ABE8,
                                      1,
                                      writeResultKnown,
                                      writeSucceeded));
    if (!payloadKnown || !writeResultKnown) {
        AppendAction(plan, CardHandoffActionKind::GateState15Write80019458,
                     kFn80019458,
                     payloadKnown ? 1 : 0,
                     writeResultKnown ? 1 : 0,
                     writeSucceeded ? 1 : 0);
        AppendAction(plan, CardHandoffActionKind::Gap, kFn80019458, 15);
        return plan;
    }
    if (!writeSucceeded) {
        AppendAction(plan, CardHandoffActionKind::GateState15Write80019458,
                     kFn80019458, 1, 1, 0, 0, 2);
        AppendAction(plan, CardHandoffActionKind::ReturnScene,
                     kFn80019458, 2, 0, 0, 0, 2);
        return plan;
    }

    AppendAction(plan, CardHandoffActionKind::WriteGp716,
                 kFn80019458,
                 static_cast<int32_t>(kGpCardSuccessFlagOffset),
                 1);
    AppendAction(plan, CardHandoffActionKind::SetGp720,
                 kFn80019458,
                 static_cast<int32_t>(kGpCardResultOffset),
                 1);
    AppendAction(plan, CardHandoffActionKind::GateState15Write80019458,
                 kFn80019458, 1, 1, 1, 0, 23);
    AppendAction(plan, CardHandoffActionKind::ReturnScene,
                 kFn80019458, 23, 0, 0, 0, 23);
    return plan;
}

CardHandoffPlan BuildSaveNameEntryConfirm800185D0Plan(uint32_t savePtr,
                                                      bool duplicateKnown,
                                                      bool duplicate,
                                                      bool freeBlocksKnown,
                                                      int32_t freeBlocks)
{
    CardHandoffPlan plan = MakePlan("SaveNameEntryConfirm800185D0");
    plan.outcome = HandoffOutcome::PendingCardLoop;
    plan.blockedByP0Gap = true;

    AppendAction(plan, CardHandoffActionKind::State10NameEntry800185D0,
                 kFn800185D0,
                 10,
                 kInputCross800185D0,
                 static_cast<int32_t>(savePtr),
                 static_cast<int32_t>(kCardNameInputCharTable800490E8));
    AppendAction(plan, CardHandoffActionKind::PlayInputSfx80025C8C,
                 kFn80025C8C, 10, 10, 0x20);
    AppendAction(plan, CardHandoffActionKind::WriteSaveSuffix800185D0,
                 kFn800185D0,
                 static_cast<int32_t>(savePtr + 1u),
                 static_cast<int32_t>(kGpCardSaveNameSourceOffset));
    AppendAction(plan, CardHandoffActionKind::FlashPrompt80017E6C,
                 kFn80017E6C,
                 static_cast<int32_t>(CardEventFrameId::CardInfo),
                 static_cast<int32_t>(kCardNameArg80049244),
                 1,
                 1,
                 0,
                 10,
                 CardMode800191E4::Save,
                 CardEventFrameId::CardInfo);
    AppendSaveDirectoryBuild80019458(plan, 10, savePtr, false, -1);
    AppendAction(plan, CardHandoffActionKind::GateNameEntryDirectory800185D0,
                 kFn800185D0,
                 duplicateKnown ? 1 : 0,
                 duplicate ? 1 : 0,
                 freeBlocksKnown ? 1 : 0,
                 freeBlocks);

    if (!duplicateKnown || (!duplicate && !freeBlocksKnown)) {
        AppendAction(plan, CardHandoffActionKind::Gap, kFn800185D0, 10);
        return plan;
    }
    if (duplicate) {
        AppendAction(plan, CardHandoffActionKind::ReturnScene,
                     kFn800185D0, 18, 0, 0, 0, 18);
        AppendPlan(plan,
                   BuildCardStateRoute800180D8Plan(18,
                                                   CardMode800191E4::Save));
        return plan;
    }
    if (freeBlocks <= 0) {
        AppendAction(plan, CardHandoffActionKind::ReturnScene,
                     kFn800185D0, 7, 0, 0, 0, 7);
        AppendPlan(plan,
                   BuildCardStateRoute800180D8Plan(7,
                                                   CardMode800191E4::Save));
        return plan;
    }

    AppendAction(plan, CardHandoffActionKind::SetOverwriteScanFlag800185D0,
                 kFn800185D0,
                 static_cast<int32_t>(kGpCardOverwriteScanFlagOffset),
                 1);
    AppendAction(plan, CardHandoffActionKind::FlashPrompt80017E6C,
                 kFn80017E6C,
                 static_cast<int32_t>(CardEventFrameId::CardInfo),
                 static_cast<int32_t>(kCardNameArg80049244),
                 1,
                 1,
                 0,
                 15,
                 CardMode800191E4::Save,
                 CardEventFrameId::CardInfo);
    AppendAction(plan, CardHandoffActionKind::ReturnScene,
                 kFn800185D0, 15, 0, 0, 0, 15);
    return plan;
}

CardHandoffPlan BuildSaveListInput800181D0Plan(int32_t inputMask,
                                               bool terminalRowKnown,
                                               bool terminalRowSelected,
                                               bool selectedNameKnown,
                                               bool selectedNamePresent)
{
    CardHandoffPlan plan = MakePlan("SaveListInput800181D0");
    plan.outcome = HandoffOutcome::PendingCardLoop;
    plan.blockedByP0Gap = true;

    AppendAction(plan, CardHandoffActionKind::State11ListInput800181D0,
                 kFn800181D0,
                 11,
                 inputMask,
                 static_cast<int32_t>(kCardGridArg80048E50),
                 static_cast<int32_t>(kCardRowTable8007A590));

    if (inputMask == kInputCross800185D0) {
        AppendAction(plan, CardHandoffActionKind::PlayInputSfx80025C8C,
                     kFn80025C8C, 11, 11, 0x20);
        AppendAction(plan, CardHandoffActionKind::GateSaveListTerminalRow800181D0,
                     kFn800181D0,
                     static_cast<int32_t>(kCardGridSelectedRow80048E64),
                     static_cast<int32_t>(kCardGridRowCount80048E60),
                     terminalRowKnown ? 1 : 0,
                     terminalRowSelected ? 1 : 0,
                     terminalRowKnown ? (terminalRowSelected ? 2 : 1) : 0,
                     11,
                     CardMode800191E4::Save,
                     CardEventFrameId::Save);
        if (!terminalRowKnown) {
            AppendAction(plan, CardHandoffActionKind::Gap,
                         kFn800181D0, 11);
            return plan;
        }
        if (terminalRowSelected) {
            AppendAction(plan, CardHandoffActionKind::FlashPrompt80017E6C,
                         kFn80017E6C,
                         static_cast<int32_t>(CardEventFrameId::Save),
                         static_cast<int32_t>(kCardGridArg80048E50),
                         1,
                         0,
                         2,
                         2,
                         CardMode800191E4::Save,
                         CardEventFrameId::Save);
            AppendAction(plan, CardHandoffActionKind::ReturnScene,
                         kFn800185D0, 2, 0, 0, 0, 2, 2);
            AppendPlan(plan,
                       BuildCardStateRoute800180D8Plan(
                           2, CardMode800191E4::Save));
            return plan;
        }
        AppendAction(plan, CardHandoffActionKind::WriteGp716,
                     kFn800181D0,
                     static_cast<int32_t>(kGpCardSuccessFlagOffset),
                     1);
        AppendAction(plan, CardHandoffActionKind::CopySelectedSuffix800181D0,
                     kFn800181D0,
                     static_cast<int32_t>(kCardRowTable8007A590),
                     static_cast<int32_t>(kCardNameBuffer8007CBE8),
                     static_cast<int32_t>(kCardDirectorySuffixScratch8007A570),
                     selectedNameKnown ? 1 : 0,
                     0,
                     selectedNamePresent ? 1 : 0);
        if (!selectedNameKnown) {
            AppendAction(plan, CardHandoffActionKind::Gap,
                         kFn800181D0, 11);
            return plan;
        }
        if (selectedNamePresent) {
            AppendAction(plan, CardHandoffActionKind::ReturnScene,
                         kFn800185D0, 22, 0, 0, 0, 22);
            AppendPlan(plan,
                       BuildCardStateRoute800180D8Plan(
                           22, CardMode800191E4::Save));
            return plan;
        }
        AppendAction(plan, CardHandoffActionKind::ClearName80018060,
                     kFn80018060, 11, 10);
        AppendAction(plan, CardHandoffActionKind::ReturnScene,
                     kFn800185D0, 10, 0, 0, 0, 10);
        AppendPlan(plan,
                   BuildCardStateRoute800180D8Plan(10,
                                                   CardMode800191E4::Save));
        return plan;
    }

    if (IsListDirectionInput800181D0(inputMask)) {
        AppendAction(plan, CardHandoffActionKind::PlayInputSfx80025C8C,
                     kFn80025C8C, 11, 11, 0x1000);
        AppendAction(plan, CardHandoffActionKind::SelectListRow800181D0,
                     kFn800181D0,
                     inputMask,
                     static_cast<int32_t>(kCardGridSelectedRow80048E64),
                     static_cast<int32_t>(kCardGridRowCount80048E60),
                     static_cast<int32_t>(kCardGridStep80048E5E),
                     0,
                     11);
        AppendAction(plan, CardHandoffActionKind::ScanSaveListEnabledRows800181D0,
                     kFn800181D0,
                     inputMask,
                     static_cast<int32_t>(kCardGridEnabledFlags80048E66),
                     static_cast<int32_t>(kCardGridRowCount80048E60),
                     static_cast<int32_t>(kCardGridSelectedRow80048E64),
                     0,
                     11);
        AppendAction(plan, CardHandoffActionKind::ReturnScene,
                     kFn800181D0, 11, 0, 0, 0, 11);
        return plan;
    }

    AppendAction(plan, CardHandoffActionKind::ReturnScene,
                 kFn800181D0, 11, 0, 0, 0, 11);
    return plan;
}

CardHandoffPlan BuildLoadReplayListInput80018E10Plan(int32_t state,
                                                     int32_t inputMask,
                                                     bool listResultKnown,
                                                     int32_t listResult)
{
    CardHandoffPlan plan = MakePlan("LoadReplayListInput80018E10");
    plan.outcome = HandoffOutcome::PendingCardLoop;
    plan.blockedByP0Gap = true;

    const CardMode800191E4 mode = ModeFrom80018E10ListState(state);
    const CardEventFrameId eventFrame = EventFrom80018E10ListState(state);
    if (mode == CardMode800191E4::Unknown ||
        eventFrame == CardEventFrameId::Unknown) {
        AppendAction(plan, CardHandoffActionKind::Gap,
                     kFn80018E10, state, inputMask);
        return plan;
    }

    AppendAction(plan, CardHandoffActionKind::Call80018E10,
                 kFn80018E10,
                 state,
                 inputMask,
                 static_cast<int32_t>(mode),
                 static_cast<int32_t>(eventFrame),
                 listResultKnown ? listResult : 0,
                 state,
                 mode,
                 eventFrame);
    AppendAction(plan, CardHandoffActionKind::State12Or13ListInput80018E10,
                 kFn80018E10,
                 state,
                 inputMask,
                 static_cast<int32_t>(kFn800181D0),
                 static_cast<int32_t>(kCardGridArg80048E50),
                 listResultKnown ? listResult : 0,
                 state,
                 mode,
                 eventFrame);

    if (IsListDirectionInput800181D0(inputMask)) {
        AppendAction(plan, CardHandoffActionKind::PlayInputSfx80025C8C,
                     kFn80025C8C, state, state, 0x1000);
        AppendAction(plan, CardHandoffActionKind::SelectListRow800181D0,
                     kFn800181D0,
                     inputMask,
                     static_cast<int32_t>(kCardGridSelectedRow80048E64),
                     static_cast<int32_t>(kCardGridRowCount80048E60),
                     static_cast<int32_t>(kCardGridStep80048E5E),
                     0,
                     state,
                     mode,
                     eventFrame);
        AppendAction(plan, CardHandoffActionKind::ScanSaveListEnabledRows800181D0,
                     kFn800181D0,
                     inputMask,
                     static_cast<int32_t>(kCardGridEnabledFlags80048E66),
                     static_cast<int32_t>(kCardGridRowCount80048E60),
                     static_cast<int32_t>(kCardGridSelectedRow80048E64),
                     0,
                     state,
                     mode,
                     eventFrame);
        AppendAction(plan, CardHandoffActionKind::ReturnScene,
                     kFn80018E10, state, 0, 0, 0, 0, state, mode, eventFrame);
        return plan;
    }

    if (inputMask != kInputCross800185D0) {
        AppendAction(plan, CardHandoffActionKind::ReturnScene,
                     kFn80018E10, state, 0, 0, 0, 0, state, mode, eventFrame);
        return plan;
    }

    AppendAction(plan, CardHandoffActionKind::PlayInputSfx80025C8C,
                 kFn80025C8C, state, state, 0x20);
    AppendAction(plan, CardHandoffActionKind::GateListInputResult80018E10,
                 kFn80018E10,
                 state,
                 inputMask,
                 listResultKnown ? 1 : 0,
                 listResult,
                 listResultKnown ? listResult : 0,
                 state,
                 mode,
                 eventFrame);
    if (!listResultKnown) {
        AppendAction(plan, CardHandoffActionKind::Gap,
                     kFn800181D0, state, inputMask);
        return plan;
    }
    if (listResult == 0) {
        AppendAction(plan, CardHandoffActionKind::ReturnScene,
                     kFn80018E10, state, 0, 0, 0, 0, state, mode, eventFrame);
        return plan;
    }
    if (listResult == 1) {
        AppendAction(plan, CardHandoffActionKind::WriteGp716,
                     kFn800181D0,
                     static_cast<int32_t>(kGpCardSuccessFlagOffset),
                     1);
        AppendAction(plan, CardHandoffActionKind::CopySelectedSuffix800181D0,
                     kFn800181D0,
                     static_cast<int32_t>(kCardRowTable8007A590),
                     static_cast<int32_t>(kCardNameBuffer8007CBE8),
                     static_cast<int32_t>(kCardDirectorySuffixScratch8007A570),
                     0,
                     0,
                     0);
        AppendAction(plan, CardHandoffActionKind::FlashPrompt80017E6C,
                     kFn80017E6C,
                     static_cast<int32_t>(eventFrame),
                     static_cast<int32_t>(kCardGridArg80048E50),
                     SelectedArgFor80018E10ListConfirm(state),
                     1,
                     listResult,
                     16,
                     mode,
                     eventFrame);
        AppendAction(plan, CardHandoffActionKind::ReturnScene,
                     kFn80018E10, 16, 0, 0, 0, listResult, 16,
                     mode, eventFrame);
        AppendAction(plan, CardHandoffActionKind::Gap,
                     kFn80019D7C, 16, static_cast<int32_t>(mode));
        return plan;
    }
    if (listResult == 2) {
        AppendAction(plan, CardHandoffActionKind::FlashPrompt80017E6C,
                     kFn80017E6C,
                     static_cast<int32_t>(eventFrame),
                     static_cast<int32_t>(kCardGridArg80048E50),
                     2,
                     0,
                     listResult,
                     23,
                     mode,
                     eventFrame);
        AppendAction(plan, CardHandoffActionKind::ReturnScene,
                     kFn80018E10, 23, 0, 0, 0, listResult, 23,
                     mode, eventFrame);
        plan.outcome = HandoffOutcome::ReturnScene0;
        return plan;
    }

    AppendAction(plan, CardHandoffActionKind::Gap,
                 kFn80018E10, state, inputMask, listResult);
    return plan;
}

CardHandoffPlan BuildSaveDirectoryScan80019458Plan(
    int32_t state,
    uint32_t savePtr,
    bool entryCountKnown,
    int32_t entryCount,
    bool freeBlocksKnown,
    int32_t freeBlocks,
    bool overwriteScanFlagKnown,
    bool overwriteScanFlag,
    bool directoryChangedKnown,
    bool directoryChanged)
{
    CardHandoffPlan plan = MakePlan("SaveDirectoryScan80019458");
    plan.outcome = HandoffOutcome::PendingCardLoop;
    plan.blockedByP0Gap = true;

    if (state != 6 && state != 9) {
        AppendAction(plan, CardHandoffActionKind::Gap,
                     kFn80019458, state);
        return plan;
    }

    AppendAction(plan, CardHandoffActionKind::Call80019458,
                 kFn80019458,
                 state,
                 0,
                 static_cast<int32_t>(savePtr));

    if (state == 9) {
        AppendAction(plan, CardHandoffActionKind::CompareSaveDirectory800488E4,
                     kFn800488E4,
                     overwriteScanFlagKnown ? 1 : 0,
                     overwriteScanFlag ? 1 : 0,
                     directoryChangedKnown ? 1 : 0,
                     directoryChanged ? 1 : 0,
                     directoryChangedKnown ? (directoryChanged ? 1 : 0) : -1);
        if (!overwriteScanFlagKnown ||
            (overwriteScanFlag && !directoryChangedKnown)) {
            AppendAction(plan, CardHandoffActionKind::Gap,
                         kFn800488E4,
                         static_cast<int32_t>(kCardDirectoryRows8007A318),
                         static_cast<int32_t>(kCardPreviousSnapshot8007CC74),
                         static_cast<int32_t>(kSaveDirectoryBytes80019458));
        }
    }

    AppendSaveDirectoryBuild80019458(plan, state, savePtr,
                                     entryCountKnown, entryCount);
    AppendAction(plan, CardHandoffActionKind::GateSaveDirectory80019458,
                 kFn80019458,
                 entryCountKnown ? 1 : 0,
                 entryCount,
                 freeBlocksKnown ? 1 : 0,
                 freeBlocks);
    if (!entryCountKnown || !freeBlocksKnown) {
        AppendAction(plan, CardHandoffActionKind::Gap,
                     kFn80019458, state);
        return plan;
    }

    if (state == 6) {
        const int32_t nextState = entryCount > 0 ? 9 : 7;
        AppendAction(plan, CardHandoffActionKind::ReturnScene,
                     kFn80019458, nextState, 0, 0, 0, nextState);
        if (nextState == 7) {
            AppendPlan(plan,
                       BuildCardStateRoute800180D8Plan(
                           7, CardMode800191E4::Save));
        }
        return plan;
    }

    AppendAction(plan, CardHandoffActionKind::InitEventArg80017E58,
                 kFn80017E58,
                 static_cast<int32_t>(kCardGridArg80048E50),
                 11);
    AppendAction(plan, CardHandoffActionKind::CopyGlyph80017FC4,
                 kFn80017FC4,
                 static_cast<int32_t>(kCardGridArg80048E50),
                 static_cast<int32_t>(kCardRowTable8007A590),
                 static_cast<int32_t>(kSaveListRowCount80019458));
    AppendAction(plan, CardHandoffActionKind::SelectSaveListInitialRow80019458,
                 kFn80019458,
                 static_cast<int32_t>(kCardGridSelectedRow80048E64),
                 static_cast<int32_t>(savePtr + 1u),
                 static_cast<int32_t>(kCardGridPageRow80048E5C),
                 static_cast<int32_t>(kCardGridStep80048E5E),
                 -1,
                 entryCount);
    AppendAction(plan, CardHandoffActionKind::SetSaveListEventArg80019458,
                 kFn80019458,
                 static_cast<int32_t>(kCardGridArg80048E50),
                 entryCount,
                 15,
                 1);
    AppendAction(plan, CardHandoffActionKind::ReturnScene,
                 kFn80019458, 11, 0, 0, 0, 11);
    AppendPlan(plan,
               BuildCardStateRoute800180D8Plan(11,
                                               CardMode800191E4::Save));
    return plan;
}

const char* MainMenuResult80015788Name(MainMenuResult80015788 result)
{
    switch (result) {
    case MainMenuResult80015788::Unknown:
        return "Unknown";
    case MainMenuResult80015788::ContinueLoop:
        return "ContinueLoop";
    case MainMenuResult80015788::HiScore:
        return "HiScore";
    case MainMenuResult80015788::Replay:
        return "Replay";
    case MainMenuResult80015788::Practice:
        return "Practice";
    case MainMenuResult80015788::StageSelect:
        return "StageSelect";
    case MainMenuResult80015788::Load:
        return "Load";
    case MainMenuResult80015788::Exit:
        return "Exit";
    case MainMenuResult80015788::Options:
        return "Options";
    }
    return "Unknown";
}

const char* CardMode800191E4Name(CardMode800191E4 mode)
{
    switch (mode) {
    case CardMode800191E4::Save:
        return "Save";
    case CardMode800191E4::Load:
        return "Load";
    case CardMode800191E4::Replay:
        return "Replay";
    case CardMode800191E4::HiScore:
        return "HiScore";
    case CardMode800191E4::Unknown:
        return "Unknown";
    }
    return "Unknown";
}

const char* CardEventFrameIdName(CardEventFrameId eventId)
{
    switch (eventId) {
    case CardEventFrameId::Unknown:
        return "Unknown";
    case CardEventFrameId::CardInfo:
        return "CardInfo";
    case CardEventFrameId::HiScore:
        return "HiScore";
    case CardEventFrameId::Save:
        return "Save";
    case CardEventFrameId::Load:
        return "Load";
    case CardEventFrameId::Replay:
        return "Replay";
    case CardEventFrameId::SaveUi:
        return "SaveUi";
    case CardEventFrameId::InsertCardPrompt:
        return "InsertCardPrompt";
    case CardEventFrameId::FormatPrompt:
        return "FormatPrompt";
    case CardEventFrameId::NoSpacePrompt:
        return "NoSpacePrompt";
    case CardEventFrameId::RenamePrompt:
        return "RenamePrompt";
    case CardEventFrameId::UnreadablePrompt:
        return "UnreadablePrompt";
    case CardEventFrameId::OverwritePrompt:
        return "OverwritePrompt";
    case CardEventFrameId::LoadReplayStart:
        return "LoadReplayStart";
    case CardEventFrameId::SaveStart:
        return "SaveStart";
    }
    return "Unknown";
}

const char* CardIoCode80017594Name(CardIoCode80017594 code)
{
    switch (code) {
    case CardIoCode80017594::Unknown:
        return "Unknown";
    case CardIoCode80017594::InfoErrorOrTimeout:
        return "InfoErrorOrTimeout";
    case CardIoCode80017594::IoSuccess:
        return "IoSuccess";
    case CardIoCode80017594::LoadErrorOrTimeout:
        return "LoadErrorOrTimeout";
    case CardIoCode80017594::Timeout:
        return "Timeout";
    case CardIoCode80017594::NewAtInfo:
        return "NewAtInfo";
    case CardIoCode80017594::NewAtLoad:
        return "NewAtLoad";
    }
    return "Unknown";
}

const char* CardEventPollSourceKindName(CardEventPollSourceKind kind)
{
    switch (kind) {
    case CardEventPollSourceKind::SwCardPoll80016EB8:
        return "SwCardPoll80016EB8";
    case CardEventPollSourceKind::SwCardPoll80016E18:
        return "SwCardPoll80016E18";
    case CardEventPollSourceKind::HwCardPoll80017008:
        return "HwCardPoll80017008";
    }
    return "Unknown";
}

const char* LowerCardResultKindName(LowerCardResultKind kind)
{
    switch (kind) {
    case LowerCardResultKind::ReadSuccess800179B4:
        return "ReadSuccess800179B4";
    case LowerCardResultKind::ReadFailure800179B4:
        return "ReadFailure800179B4";
    case LowerCardResultKind::WriteSuccess80017A10:
        return "WriteSuccess80017A10";
    case LowerCardResultKind::WriteRetryFailure80017A10:
        return "WriteRetryFailure80017A10";
    case LowerCardResultKind::FormatSuccess80017B60:
        return "FormatSuccess80017B60";
    case LowerCardResultKind::FormatPoll3Failure80017B60:
        return "FormatPoll3Failure80017B60";
    case LowerCardResultKind::FormatRetryExhausted80017B60:
        return "FormatRetryExhausted80017B60";
    }
    return "Unknown";
}

const char* CardIoFeedbackTransitionKindName(
    CardIoFeedbackTransitionKind kind)
{
    switch (kind) {
    case CardIoFeedbackTransitionKind::Unknown:
        return "Unknown";
    case CardIoFeedbackTransitionKind::State0InfoSubmit80017594:
        return "State0InfoSubmit80017594";
    case CardIoFeedbackTransitionKind::State1InfoNoEvent80016E18:
        return "State1InfoNoEvent80016E18";
    case CardIoFeedbackTransitionKind::State1InfoSuccess80016E18:
        return "State1InfoSuccess80016E18";
    case CardIoFeedbackTransitionKind::State1InfoTimeout80016E18:
        return "State1InfoTimeout80016E18";
    case CardIoFeedbackTransitionKind::State1InfoError80016E18:
        return "State1InfoError80016E18";
    case CardIoFeedbackTransitionKind::State1InfoNewCardReset80016E18_80017008:
        return "State1InfoNewCardReset80016E18_80017008";
    case CardIoFeedbackTransitionKind::State2LoadSubmit80017594:
        return "State2LoadSubmit80017594";
    case CardIoFeedbackTransitionKind::State3LoadNoEvent80016E18:
        return "State3LoadNoEvent80016E18";
    case CardIoFeedbackTransitionKind::State3LoadSuccess80016E18:
        return "State3LoadSuccess80016E18";
    case CardIoFeedbackTransitionKind::State3LoadTimeout80016E18:
        return "State3LoadTimeout80016E18";
    case CardIoFeedbackTransitionKind::State3LoadError80016E18:
        return "State3LoadError80016E18";
    case CardIoFeedbackTransitionKind::State3LoadNewCard80016E18:
        return "State3LoadNewCard80016E18";
    case CardIoFeedbackTransitionKind::State4PublishPending80017594:
        return "State4PublishPending80017594";
    }
    return "Unknown";
}

const char* LowerCardPayloadProducerKindName(
    LowerCardPayloadProducerKind kind)
{
    switch (kind) {
    case LowerCardPayloadProducerKind::State16ReadPayload80019D7C:
        return "State16ReadPayload80019D7C";
    case LowerCardPayloadProducerKind::Case17ReadPayload80019D7C:
        return "Case17ReadPayload80019D7C";
    }
    return "Unknown";
}

const char* HandoffOutcomeName(HandoffOutcome outcome)
{
    switch (outcome) {
    case HandoffOutcome::ContinueLoop:
        return "ContinueLoop";
    case HandoffOutcome::ReturnScene0:
        return "ReturnScene0";
    case HandoffOutcome::ReturnSelectedScene:
        return "ReturnSelectedScene";
    case HandoffOutcome::ReturnReplayScene:
        return "ReturnReplayScene";
    case HandoffOutcome::ReturnHiScoreTable:
        return "ReturnHiScoreTable";
    case HandoffOutcome::PendingCardLoop:
        return "PendingCardLoop";
    case HandoffOutcome::Gap:
        return "Gap";
    }
    return "Unknown";
}

const char* CardHandoffActionKindName(CardHandoffActionKind kind)
{
    switch (kind) {
    case CardHandoffActionKind::None:
        return "None";
    case CardHandoffActionKind::Call80026FA4:
        return "Call80026FA4";
    case CardHandoffActionKind::PlayCue80026EF8_94410:
        return "PlayCue80026EF8_94410";
    case CardHandoffActionKind::Flush80026ECC:
        return "Flush80026ECC";
    case CardHandoffActionKind::Bootstrap80015590:
        return "Bootstrap80015590";
    case CardHandoffActionKind::Call80026B94:
        return "Call80026B94";
    case CardHandoffActionKind::GateMainMenuResult80015788:
        return "GateMainMenuResult80015788";
    case CardHandoffActionKind::GateStageSelectResult80026B94:
        return "GateStageSelectResult80026B94";
    case CardHandoffActionKind::Call80019414:
        return "Call80019414";
    case CardHandoffActionKind::Call80015700:
        return "Call80015700";
    case CardHandoffActionKind::Call800193F4:
        return "Call800193F4";
    case CardHandoffActionKind::GateReplaySelector800193F4:
        return "GateReplaySelector800193F4";
    case CardHandoffActionKind::WriteWord800916D0:
        return "WriteWord800916D0";
    case CardHandoffActionKind::Call800161A8:
        return "Call800161A8";
    case CardHandoffActionKind::GateReplaySceneMap800161A8:
        return "GateReplaySceneMap800161A8";
    case CardHandoffActionKind::Call800193B0:
        return "Call800193B0";
    case CardHandoffActionKind::Call800191E4:
        return "Call800191E4";
    case CardHandoffActionKind::Call80026784:
        return "Call80026784";
    case CardHandoffActionKind::Copy80025C64:
        return "Copy80025C64";
    case CardHandoffActionKind::Call80020110:
        return "Call80020110";
    case CardHandoffActionKind::WriteGp716:
        return "WriteGp716";
    case CardHandoffActionKind::WriteGp732:
        return "WriteGp732";
    case CardHandoffActionKind::Call80017524:
        return "Call80017524";
    case CardHandoffActionKind::Call80017574:
        return "Call80017574";
    case CardHandoffActionKind::GateGp716:
        return "GateGp716";
    case CardHandoffActionKind::GateGp720:
        return "GateGp720";
    case CardHandoffActionKind::SetGp712:
        return "SetGp712";
    case CardHandoffActionKind::ReadA1Plus44:
        return "ReadA1Plus44";
    case CardHandoffActionKind::Call80019284:
        return "Call80019284";
    case CardHandoffActionKind::FormatHiScoreRecord80019284:
        return "FormatHiScoreRecord80019284";
    case CardHandoffActionKind::CopyGlyph80017FC4:
        return "CopyGlyph80017FC4";
    case CardHandoffActionKind::Call80018F70:
        return "Call80018F70";
    case CardHandoffActionKind::LoadSnapshot80018F70:
        return "LoadSnapshot80018F70";
    case CardHandoffActionKind::GateLoadFlowResult800193B0:
        return "GateLoadFlowResult800193B0";
    case CardHandoffActionKind::Call80019148:
        return "Call80019148";
    case CardHandoffActionKind::GateSaveUiResult80019148:
        return "GateSaveUiResult80019148";
    case CardHandoffActionKind::Call80018FB0:
        return "Call80018FB0";
    case CardHandoffActionKind::InitCardDriver80018FB0:
        return "InitCardDriver80018FB0";
    case CardHandoffActionKind::GateCardDriverExit80018FB0:
        return "GateCardDriverExit80018FB0";
    case CardHandoffActionKind::InputEdge80018FB0:
        return "InputEdge80018FB0";
    case CardHandoffActionKind::RouteInputCallback80018FB0:
        return "RouteInputCallback80018FB0";
    case CardHandoffActionKind::RouteIoCallback80018FB0:
        return "RouteIoCallback80018FB0";
    case CardHandoffActionKind::BlinkState80018FB0:
        return "BlinkState80018FB0";
    case CardHandoffActionKind::EndFrame8001EA00:
        return "EndFrame8001EA00";
    case CardHandoffActionKind::Call800185D0:
        return "Call800185D0";
    case CardHandoffActionKind::Call80018E10:
        return "Call80018E10";
    case CardHandoffActionKind::Call80019458:
        return "Call80019458";
    case CardHandoffActionKind::State21SaveBootstrap80019458:
        return "State21SaveBootstrap80019458";
    case CardHandoffActionKind::Call80019D7C:
        return "Call80019D7C";
    case CardHandoffActionKind::State20CardModeBootstrap80019D7C:
        return "State20CardModeBootstrap80019D7C";
    case CardHandoffActionKind::Call80035510:
        return "Call80035510";
    case CardHandoffActionKind::Poll80017594:
        return "Poll80017594";
    case CardHandoffActionKind::Read800179B4:
        return "Read800179B4";
    case CardHandoffActionKind::Call800173A8:
        return "Call800173A8";
    case CardHandoffActionKind::FormatCardPath800173A8:
        return "FormatCardPath800173A8";
    case CardHandoffActionKind::OpenCardFile800173A8:
        return "OpenCardFile800173A8";
    case CardHandoffActionKind::ClearEvents80016FC0:
        return "ClearEvents80016FC0";
    case CardHandoffActionKind::SubmitCardRead800173A8:
        return "SubmitCardRead800173A8";
    case CardHandoffActionKind::PollEvents80016EB8:
        return "PollEvents80016EB8";
    case CardHandoffActionKind::GateSwCardEventSource80016EB8:
        return "GateSwCardEventSource80016EB8";
    case CardHandoffActionKind::GateSwPollResult80016EB8:
        return "GateSwPollResult80016EB8";
    case CardHandoffActionKind::CloseGp696:
        return "CloseGp696";
    case CardHandoffActionKind::GateReadSuccess800179B4:
        return "GateReadSuccess800179B4";
    case CardHandoffActionKind::GateCase17ReadProducer80019D7C:
        return "GateCase17ReadProducer80019D7C";
    case CardHandoffActionKind::ClearBlock80025C44:
        return "ClearBlock80025C44";
    case CardHandoffActionKind::GateWord8007ABE4:
        return "GateWord8007ABE4";
    case CardHandoffActionKind::VSyncCallback80017F38:
        return "VSyncCallback80017F38";
    case CardHandoffActionKind::GateCardRowEnabled:
        return "GateCardRowEnabled";
    case CardHandoffActionKind::CopyRowName80019D7C:
        return "CopyRowName80019D7C";
    case CardHandoffActionKind::WriteRowMetadata80019D7C:
        return "WriteRowMetadata80019D7C";
    case CardHandoffActionKind::ClearHiScoreBank800168DC:
        return "ClearHiScoreBank800168DC";
    case CardHandoffActionKind::BuildLoadReplayListRows80019D7C:
        return "BuildLoadReplayListRows80019D7C";
    case CardHandoffActionKind::SetLoadReplayListEventArg80019D7C:
        return "SetLoadReplayListEventArg80019D7C";
    case CardHandoffActionKind::LoadReplayTickIoGate80019D7C:
        return "LoadReplayTickIoGate80019D7C";
    case CardHandoffActionKind::State16LoadPayload80019D7C:
        return "State16LoadPayload80019D7C";
    case CardHandoffActionKind::GateLowerCardPayloadProducer80019D7C:
        return "GateLowerCardPayloadProducer80019D7C";
    case CardHandoffActionKind::GateCase17PayloadProducer80019D7C:
        return "GateCase17PayloadProducer80019D7C";
    case CardHandoffActionKind::LoadPayload800164B4:
        return "LoadPayload800164B4";
    case CardHandoffActionKind::GateLoadedPayloadSelector80019D7C:
        return "GateLoadedPayloadSelector80019D7C";
    case CardHandoffActionKind::MergePayload800164F8:
        return "MergePayload800164F8";
    case CardHandoffActionKind::SetGp720:
        return "SetGp720";
    case CardHandoffActionKind::Write80017A10:
        return "Write80017A10";
    case CardHandoffActionKind::ScanCardDirectory80017900:
        return "ScanCardDirectory80017900";
    case CardHandoffActionKind::Call80017454:
        return "Call80017454";
    case CardHandoffActionKind::FormatCardPath80017454:
        return "FormatCardPath80017454";
    case CardHandoffActionKind::OpenCheck80017454:
        return "OpenCheck80017454";
    case CardHandoffActionKind::OpenWrite80017454:
        return "OpenWrite80017454";
    case CardHandoffActionKind::SubmitCardWrite80017454:
        return "SubmitCardWrite80017454";
    case CardHandoffActionKind::WaitCard80035560:
        return "WaitCard80035560";
    case CardHandoffActionKind::GateWriteSuccess80017A10:
        return "GateWriteSuccess80017A10";
    case CardHandoffActionKind::FormatCardPath80017B60:
        return "FormatCardPath80017B60";
    case CardHandoffActionKind::DrainHwEvents8001707C:
        return "DrainHwEvents8001707C";
    case CardHandoffActionKind::SubmitFormat80017B60:
        return "SubmitFormat80017B60";
    case CardHandoffActionKind::PollHwEvents80017008:
        return "PollHwEvents80017008";
    case CardHandoffActionKind::GateHwCardEventSource80017008:
        return "GateHwCardEventSource80017008";
    case CardHandoffActionKind::GateHwPollResult80017008:
        return "GateHwPollResult80017008";
    case CardHandoffActionKind::GateFormatResult80017B60:
        return "GateFormatResult80017B60";
    case CardHandoffActionKind::CardInfo80017594:
        return "CardInfo80017594";
    case CardHandoffActionKind::PollSwEvents80016E18:
        return "PollSwEvents80016E18";
    case CardHandoffActionKind::GateSwCardEventSource80016E18:
        return "GateSwCardEventSource80016E18";
    case CardHandoffActionKind::GateSwPollCounter80016E18:
        return "GateSwPollCounter80016E18";
    case CardHandoffActionKind::GateCardIoFeedbackTransition80017594:
        return "GateCardIoFeedbackTransition80017594";
    case CardHandoffActionKind::CardLoad80017594:
        return "CardLoad80017594";
    case CardHandoffActionKind::ResetHwEvents80047EE4:
        return "ResetHwEvents80047EE4";
    case CardHandoffActionKind::CommitCardIoState80017594:
        return "CommitCardIoState80017594";
    case CardHandoffActionKind::ReturnCardIo80017594:
        return "ReturnCardIo80017594";
    case CardHandoffActionKind::SaveTickIoGate80019458:
        return "SaveTickIoGate80019458";
    case CardHandoffActionKind::State14Format80019458:
        return "State14Format80019458";
    case CardHandoffActionKind::GateState14Format80019458:
        return "GateState14Format80019458";
    case CardHandoffActionKind::State15Save80019458:
        return "State15Save80019458";
    case CardHandoffActionKind::BuildSaveFilename80019458:
        return "BuildSaveFilename80019458";
    case CardHandoffActionKind::BuildSaveTitle80019458:
        return "BuildSaveTitle80019458";
    case CardHandoffActionKind::ClearSaveBlock80025C44:
        return "ClearSaveBlock80025C44";
    case CardHandoffActionKind::BuildSaveHeader80017C08:
        return "BuildSaveHeader80017C08";
    case CardHandoffActionKind::GateState15PayloadSource80019458:
        return "GateState15PayloadSource80019458";
    case CardHandoffActionKind::CopyState15Payload80025C64:
        return "CopyState15Payload80025C64";
    case CardHandoffActionKind::GateState15Write80019458:
        return "GateState15Write80019458";
    case CardHandoffActionKind::State10NameEntry800185D0:
        return "State10NameEntry800185D0";
    case CardHandoffActionKind::WriteSaveSuffix800185D0:
        return "WriteSaveSuffix800185D0";
    case CardHandoffActionKind::GateNameEntryDirectory800185D0:
        return "GateNameEntryDirectory800185D0";
    case CardHandoffActionKind::SetOverwriteScanFlag800185D0:
        return "SetOverwriteScanFlag800185D0";
    case CardHandoffActionKind::State11ListInput800181D0:
        return "State11ListInput800181D0";
    case CardHandoffActionKind::State12Or13ListInput80018E10:
        return "State12Or13ListInput80018E10";
    case CardHandoffActionKind::GateListInputResult80018E10:
        return "GateListInputResult80018E10";
    case CardHandoffActionKind::GateSaveListTerminalRow800181D0:
        return "GateSaveListTerminalRow800181D0";
    case CardHandoffActionKind::CopySelectedSuffix800181D0:
        return "CopySelectedSuffix800181D0";
    case CardHandoffActionKind::SelectListRow800181D0:
        return "SelectListRow800181D0";
    case CardHandoffActionKind::ScanSaveListEnabledRows800181D0:
        return "ScanSaveListEnabledRows800181D0";
    case CardHandoffActionKind::ScanSaveDirectory80017B08:
        return "ScanSaveDirectory80017B08";
    case CardHandoffActionKind::SnapshotSaveDirectory80017B18:
        return "SnapshotSaveDirectory80017B18";
    case CardHandoffActionKind::ClearSaveListRows80025C44:
        return "ClearSaveListRows80025C44";
    case CardHandoffActionKind::CompareSaveDirectory800488E4:
        return "CompareSaveDirectory800488E4";
    case CardHandoffActionKind::BuildSaveListRows80019458:
        return "BuildSaveListRows80019458";
    case CardHandoffActionKind::GateDirectoryListSource80017B08:
        return "GateDirectoryListSource80017B08";
    case CardHandoffActionKind::GateSaveDirectory80019458:
        return "GateSaveDirectory80019458";
    case CardHandoffActionKind::InitEventArg80017E58:
        return "InitEventArg80017E58";
    case CardHandoffActionKind::SelectSaveListInitialRow80019458:
        return "SelectSaveListInitialRow80019458";
    case CardHandoffActionKind::SetSaveListEventArg80019458:
        return "SetSaveListEventArg80019458";
    case CardHandoffActionKind::PlayInputSfx80025C8C:
        return "PlayInputSfx80025C8C";
    case CardHandoffActionKind::InitNameEntry80018060:
        return "InitNameEntry80018060";
    case CardHandoffActionKind::ClearNameEntryFlags80018060:
        return "ClearNameEntryFlags80018060";
    case CardHandoffActionKind::ClearNameEntryBuffers80018060:
        return "ClearNameEntryBuffers80018060";
    case CardHandoffActionKind::RenderNamePreview80017FC4:
        return "RenderNamePreview80017FC4";
    case CardHandoffActionKind::RouteNameCursor800185D0:
        return "RouteNameCursor800185D0";
    case CardHandoffActionKind::GateNameCharacter800185D0:
        return "GateNameCharacter800185D0";
    case CardHandoffActionKind::DeleteNameCharacter800185D0:
        return "DeleteNameCharacter800185D0";
    case CardHandoffActionKind::AppendNameCharacter800185D0:
        return "AppendNameCharacter800185D0";
    case CardHandoffActionKind::CancelNameEntry800185D0:
        return "CancelNameEntry800185D0";
    case CardHandoffActionKind::State22OverwritePrompt800185D0:
        return "State22OverwritePrompt800185D0";
    case CardHandoffActionKind::ExtractOverwriteSuffix800185D0:
        return "ExtractOverwriteSuffix800185D0";
    case CardHandoffActionKind::CancelOverwritePrompt800185D0:
        return "CancelOverwritePrompt800185D0";
    case CardHandoffActionKind::RemapEvent800180D8:
        return "RemapEvent800180D8";
    case CardHandoffActionKind::RouteState800180D8:
        return "RouteState800180D8";
    case CardHandoffActionKind::GateCurrentEvent800180D8:
        return "GateCurrentEvent800180D8";
    case CardHandoffActionKind::StoreCurrentEvent800180D8:
        return "StoreCurrentEvent800180D8";
    case CardHandoffActionKind::GateCardInfoArg80049244:
        return "GateCardInfoArg80049244";
    case CardHandoffActionKind::RouteIoCodeState4:
        return "RouteIoCodeState4";
    case CardHandoffActionKind::DrawEventFrame8001E750:
        return "DrawEventFrame8001E750";
    case CardHandoffActionKind::FlashPrompt80017E6C:
        return "FlashPrompt80017E6C";
    case CardHandoffActionKind::FormatCard80017B60:
        return "FormatCard80017B60";
    case CardHandoffActionKind::ClearName80018060:
        return "ClearName80018060";
    case CardHandoffActionKind::Call8002776CPractice:
        return "Call8002776CPractice";
    case CardHandoffActionKind::Gap:
        return "Gap";
    case CardHandoffActionKind::ReturnScene:
        return "ReturnScene";
    }
    return "Unknown";
}

} // namespace PrSS0CardMemcardHandoffDirect
