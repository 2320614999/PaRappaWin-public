#include "pr_ss0_practice_lifecycle_direct.h"

namespace PrSS0PracticeLifecycleDirect {
namespace {

constexpr PracticeCueSpec kCueSpecs[] = {
    {PracticeCueKind::LoopStart,
     kCuePracticeLoop8006EC90,
     {0x01, 0x00, 0x18, 0x5A},
     0,
     -1,
     -1,
     "practice loop start"},
    {PracticeCueKind::Beat,
     kCuePracticeBeat8006EC98,
     {0x01, 0x01, 0x19, 0x5A},
     0,
     -1,
     -1,
     "automatic beat"},
    {PracticeCueKind::RoundPrompt0,
     kCuePracticeRoundPromptTable80055494 + 0x00u,
     {0x02, 0x00, 0x18, 0x5A},
     0,
     -1,
     0,
     "round0 triangle prompt"},
    {PracticeCueKind::RoundPrompt1,
     kCuePracticeRoundPromptTable80055494 + 0x06u,
     {0x02, 0x01, 0x19, 0x5A},
     0,
     -1,
     1,
     "round1 triangle prompt"},
    {PracticeCueKind::RoundPrompt2,
     kCuePracticeRoundPromptTable80055494 + 0x0Cu,
     {0x02, 0x02, 0x1A, 0x5A},
     0,
     -1,
     2,
     "round2 triangle prompt"},
    {PracticeCueKind::RoundPrompt3,
     kCuePracticeRoundPromptTable80055494 + 0x12u,
     {0x02, 0x03, 0x1B, 0x5A},
     0,
     -1,
     3,
     "round3 triangle prompt"},
    {PracticeCueKind::VoiceOnBeat,
     kCuePracticeVoiceTable800554AC + 0x18u,
     {0x03, 0x01, 0x19, 0x6E},
     75,
     0,
     -1,
     "on-beat voice"},
    {PracticeCueKind::VoiceTooQuick,
     kCuePracticeVoiceTable800554AC + 0x1Eu,
     {0x03, 0x07, 0x1F, 0x6E},
     75,
     1,
     -1,
     "too quick voice"},
    {PracticeCueKind::VoiceTooSlow,
     kCuePracticeVoiceTable800554AC + 0x24u,
     {0x03, 0x08, 0x20, 0x6E},
     75,
     2,
     -1,
     "too slow voice"},
    {PracticeCueKind::VoiceIntro0,
     kCuePracticeVoiceTable800554AC + 0x00u,
     {0x03, 0x03, 0x1B, 0x6E},
     80,
     3,
     0,
     "round0 intro voice"},
    {PracticeCueKind::VoiceIntro1,
     kCuePracticeVoiceTable800554AC + 0x06u,
     {0x03, 0x06, 0x1E, 0x6E},
     46,
     4,
     1,
     "round1 intro voice"},
    {PracticeCueKind::VoiceIntro2,
     kCuePracticeVoiceTable800554AC + 0x0Cu,
     {0x03, 0x02, 0x1A, 0x6E},
     42,
     5,
     2,
     "round2 intro voice"},
    {PracticeCueKind::VoiceIntro3,
     kCuePracticeVoiceTable800554AC + 0x12u,
     {0x03, 0x05, 0x1D, 0x6E},
     49,
     6,
     3,
     "round3 intro voice"},
    {PracticeCueKind::VoiceExit7,
     kCuePracticeExitKind7_8006ECA8,
     {0x03, 0x04, 0x1C, 0x6E},
     88,
     7,
     -1,
     "exit prompt voice kind7"},
    {PracticeCueKind::VoiceExit8,
     kCuePracticeExitKind8_8006ECA0,
     {0x03, 0x00, 0x18, 0x6E},
     95,
     8,
     -1,
     "exit prompt voice kind8"},
};

constexpr PracticeCtxFieldSpec kCtxFields[] = {
    {PracticeCtxFieldKind::Flags,
     0x00u,
     4u,
     "flags",
     "8002776C and 800276EC",
     "80023618 overlay gate"},
    {PracticeCtxFieldKind::OverlayKind,
     0x1Cu,
     4u,
     "overlay kind",
     "800276EC writes kind",
     "80023618 overlay switch"},
    {PracticeCtxFieldKind::ExitSelected,
     0x48u,
     4u,
     "exit selected",
     "8002776C exit confirm Cross",
     "80023618 exit prompt draw"},
    {PracticeCtxFieldKind::Blink,
     0x4Cu,
     2u,
     "exit blink",
     "8002776C exit confirm loop",
     "80023618 exit icon toggle"},
    {PracticeCtxFieldKind::SeqMode,
     0x8Au,
     2u,
     "sequence mode",
     "8002776C round frame pulses",
     "sequence advance gate"},
    {PracticeCtxFieldKind::SeqCurA,
     0x8Cu,
     2u,
     "sequence cursor A",
     "8002776C Rec44 advance",
     "80023618 row A highlight"},
    {PracticeCtxFieldKind::SeqEnableA,
     0x90u,
     2u,
     "sequence enable A",
     "8002776C Rec44 advance",
     "8002776C advance gate"},
    {PracticeCtxFieldKind::SeqStreamA,
     0x94u,
     4u,
     "sequence stream A",
     "8002776C round Rec44 pointer",
     "8002776C stream read"},
    {PracticeCtxFieldKind::SeqCurB,
     0x9Eu,
     2u,
     "sequence cursor B",
     "8002776C Rec44 advance",
     "80023618 row B highlight"},
    {PracticeCtxFieldKind::SeqEnableB,
     0xA2u,
     2u,
     "sequence enable B",
     "8002776C Rec44 advance",
     "8002776C advance gate"},
    {PracticeCtxFieldKind::SeqStreamB,
     0xA4u,
     4u,
     "sequence stream B",
     "8002776C round Rec44 pointer",
     "8002776C stream read"},
};

constexpr PracticeRec44Spec kRec44Specs[] = {
    {0, kPracticeRec44RecordBase800554D8 + 0x00u,
        kPracticeRec44StreamBase800554DC + 0x00u, 1, 0, 2, 16},
    {1, kPracticeRec44RecordBase800554D8 + 0x2Cu,
        kPracticeRec44StreamBase800554DC + 0x2Cu, 1, 0, 6, 16},
    {2, kPracticeRec44RecordBase800554D8 + 0x58u,
        kPracticeRec44StreamBase800554DC + 0x58u, 1, 0, 10, 16},
    {3, kPracticeRec44RecordBase800554D8 + 0x84u,
        kPracticeRec44StreamBase800554DC + 0x84u, 1, 0, 14, 16},
};

constexpr int8_t
    kRec44Streams[kPracticeRoundCount][kPracticeRec44StreamByteCount] = {
        {0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
         -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
         0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
         -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
         0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0,
         -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
         0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0,
         -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
         0, 0, 0, 0, 0, 0, 0, 0},
};

bool Append(PracticePlan& plan, const PracticeAction& action)
{
    if (plan.count >= sizeof(plan.actions) / sizeof(plan.actions[0])) {
        plan.truncated = true;
        return false;
    }
    plan.actions[plan.count++] = action;
    return true;
}

void AppendAction(PracticePlan& plan,
                  PracticeActionKind kind,
                  uint32_t psxFunction,
                  uint32_t address = 0,
                  int32_t round = -1,
                  int32_t frame = -1,
                  int32_t arg0 = 0,
                  int32_t arg1 = 0,
                  int32_t arg2 = 0,
                  int32_t arg3 = 0,
                  PracticeCueKind cue = PracticeCueKind::Unknown,
                  bool conditional = false)
{
    PracticeAction action{};
    action.kind = kind;
    action.psxFunction = psxFunction;
    action.address = address;
    action.round = round;
    action.frame = frame;
    action.args[0] = arg0;
    action.args[1] = arg1;
    action.args[2] = arg2;
    action.args[3] = arg3;
    action.cue = cue;
    action.conditional = conditional;
    (void)Append(plan, action);
}

void AppendPlan(PracticePlan& dst, const PracticePlan& src)
{
    for (uint32_t i = 0; i < src.count; ++i) {
        (void)Append(dst, src.actions[i]);
    }
    dst.truncated = dst.truncated || src.truncated;
    dst.blockedByGap = dst.blockedByGap || src.blockedByGap;
}

PracticePlan MakePlan(const char* name)
{
    PracticePlan plan{};
    plan.name = name;
    plan.runtimeCutoverAllowed = RuntimeCutoverAllowed();
    return plan;
}

PracticeCueKind RoundIntroCue(int32_t round)
{
    switch (round) {
    case 0:
        return PracticeCueKind::VoiceIntro0;
    case 1:
        return PracticeCueKind::VoiceIntro1;
    case 2:
        return PracticeCueKind::VoiceIntro2;
    case 3:
        return PracticeCueKind::VoiceIntro3;
    }
    return PracticeCueKind::Unknown;
}

PracticeCueKind RoundPromptCue(int32_t round)
{
    switch (round) {
    case 0:
        return PracticeCueKind::RoundPrompt0;
    case 1:
        return PracticeCueKind::RoundPrompt1;
    case 2:
        return PracticeCueKind::RoundPrompt2;
    case 3:
        return PracticeCueKind::RoundPrompt3;
    }
    return PracticeCueKind::Unknown;
}

} // namespace

bool RuntimeCutoverAllowed()
{
    return false;
}

uint32_t KnownPracticeCueSpecCount()
{
    return sizeof(kCueSpecs) / sizeof(kCueSpecs[0]);
}

const PracticeCueSpec& KnownPracticeCueSpecAt(uint32_t index)
{
    if (index >= KnownPracticeCueSpecCount()) {
        index = KnownPracticeCueSpecCount() - 1u;
    }
    return kCueSpecs[index];
}

const PracticeCueSpec* FindPracticeCueSpec(PracticeCueKind cue)
{
    for (uint32_t i = 0; i < KnownPracticeCueSpecCount(); ++i) {
        if (kCueSpecs[i].kind == cue) {
            return &kCueSpecs[i];
        }
    }
    return nullptr;
}

uint32_t KnownPracticeCtxFieldCount()
{
    return sizeof(kCtxFields) / sizeof(kCtxFields[0]);
}

const PracticeCtxFieldSpec& KnownPracticeCtxFieldAt(uint32_t index)
{
    if (index >= KnownPracticeCtxFieldCount()) {
        index = KnownPracticeCtxFieldCount() - 1u;
    }
    return kCtxFields[index];
}

uint32_t KnownPracticeRec44SpecCount()
{
    return sizeof(kRec44Specs) / sizeof(kRec44Specs[0]);
}

const PracticeRec44Spec& KnownPracticeRec44SpecAt(uint32_t index)
{
    if (index >= KnownPracticeRec44SpecCount()) {
        index = KnownPracticeRec44SpecCount() - 1u;
    }
    return kRec44Specs[index];
}

PracticeRec44StreamValue ReadPracticeRec44StreamValue(
    int32_t round,
    int32_t slot)
{
    PracticeRec44StreamValue out{};
    if (round < 0 ||
        round >= static_cast<int32_t>(kPracticeRoundCount) ||
        slot < 0 ||
        slot >= static_cast<int32_t>(kPracticeRec44StreamByteCount)) {
        return out;
    }
    const auto& spec = KnownPracticeRec44SpecAt(
        static_cast<uint32_t>(round));
    out.sourceAddress = spec.streamAddress + static_cast<uint32_t>(slot);
    out.value = kRec44Streams[round][slot];
    out.sourceKnown = true;
    return out;
}

PracticePlan BuildPracticeResourceLoad80015618Plan()
{
    PracticePlan plan = MakePlan("PracticeResourceLoad80015618");
    plan.blockedByGap = true;

    AppendAction(plan,
                 PracticeActionKind::CallLoadYCompo80015618,
                 kFn80015618,
                 kYCompoDescriptor800546EC);
    AppendAction(plan,
                 PracticeActionKind::ClearResourceTable80025A34,
                 kFn80025A34);
    AppendAction(plan,
                 PracticeActionKind::LoadInt8001AC18,
                 kFn8001AC18,
                 kYCompoDescriptor800546EC,
                 -1,
                 -1,
                 static_cast<int32_t>(kYCompoDescriptor800546EC),
                 1);
    AppendAction(plan,
                 PracticeActionKind::TimUpload,
                 kFn8001AC18,
                 kYCompoDescriptor800546EC,
                 -1,
                 -1,
                 91);
    AppendAction(plan,
                 PracticeActionKind::VabLoadPractice,
                 kFn8001AC18,
                 kYCompoDescriptor800546EC,
                 -1,
                 -1,
                 2);
    AppendAction(plan,
                 PracticeActionKind::Gap,
                 kFn80015618,
                 kYCompoDescriptor800546EC);
    return plan;
}

PracticePlan BuildPracticePadStop800276ECPlan(PracticeCueKind cue)
{
    PracticePlan plan = MakePlan("PracticePadStop800276EC");
    plan.blockedByGap = true;

    const PracticeCueSpec* spec = FindPracticeCueSpec(cue);
    if (spec == nullptr) {
        AppendAction(plan, PracticeActionKind::Gap, kFn800276EC);
        return plan;
    }

    AppendAction(plan,
                 PracticeActionKind::GatePadStopCueSource800276EC,
                 kFn800276EC,
                 spec->address,
                 spec->round,
                 -1,
                 static_cast<int32_t>(spec->address),
                 spec->frames,
                 spec->padStopKind,
                 1,
                 cue,
                 true);
    AppendAction(plan,
                 PracticeActionKind::PadStopCom800276EC,
                 kFn800276EC,
                 spec->address,
                 spec->round,
                 -1,
                 static_cast<int32_t>(kScene0WorkAddress),
                 static_cast<int32_t>(spec->address),
                 spec->frames,
                 spec->padStopKind,
                 cue);
    AppendAction(plan,
                 PracticeActionKind::PlayCue80026EF8,
                 kFn80026EF8,
                 spec->address,
                 spec->round,
                 -1,
                 static_cast<int32_t>(spec->address),
                 0,
                 0,
                 0,
                 cue);
    AppendAction(plan,
                 PracticeActionKind::Flush80026ECC,
                 kFn80026ECC,
                 spec->address,
                 spec->round,
                 -1,
                 0,
                 0,
                 0,
                 0,
                 cue);
    return plan;
}

PracticePlan BuildPracticeDrawFramePlan(uint32_t menuCtx)
{
    PracticePlan plan = MakePlan("PracticeDrawFrame8002776C");
    plan.blockedByGap = true;

    AppendAction(plan,
                 PracticeActionKind::ResetEventText80024E98,
                 kFn80024E98);
    AppendAction(plan,
                 PracticeActionKind::DrawEvent16_8001E750,
                 kFn8001E750,
                 menuCtx,
                 -1,
                 -1,
                 16,
                 static_cast<int32_t>(menuCtx));
    AppendAction(plan,
                 PracticeActionKind::DrawPractice80023618,
                 kFn80023618,
                 menuCtx);
    AppendAction(plan,
                 PracticeActionKind::WaitFrame80035560_2,
                 kFn80035560,
                 0,
                 -1,
                 -1,
                 2);
    AppendAction(plan,
                 PracticeActionKind::EndFrame8001EA00_0,
                 kFn8001EA00,
                 0,
                 -1,
                 -1,
                 0);
    return plan;
}

PracticePlan BuildPracticeRound8002776CPlan(int32_t round)
{
    PracticePlan plan = MakePlan("PracticeRound8002776C");
    plan.blockedByGap = true;

    if (round < 0 ||
        static_cast<uint32_t>(round) >= KnownPracticeRec44SpecCount()) {
        AppendAction(plan, PracticeActionKind::Gap, kFn8002776C);
        return plan;
    }

    const PracticeRec44Spec& rec =
        KnownPracticeRec44SpecAt(static_cast<uint32_t>(round));
    const PracticeCueKind introCue = RoundIntroCue(round);
    const PracticeCueKind promptCue = RoundPromptCue(round);
    const PracticeCueSpec* intro = FindPracticeCueSpec(introCue);
    const PracticeCueSpec* prompt = FindPracticeCueSpec(promptCue);

    AppendAction(plan,
                 PracticeActionKind::GatePracticeRoundRec44Source8002776C,
                 kFn8002776C,
                 rec.recordAddress,
                 round,
                 -1,
                 static_cast<int32_t>(rec.streamAddress),
                 rec.pulseSlot,
                 rec.sentinelSlot,
                 1,
                 PracticeCueKind::Unknown,
                 true);
    AppendPlan(plan, BuildPracticePadStop800276ECPlan(introCue));
    if (intro != nullptr) {
        AppendAction(plan,
                     PracticeActionKind::AdvanceRec44,
                     kFn8002776C,
                     rec.recordAddress,
                     round,
                     -1,
                     rec.pulseSlot,
                     rec.sentinelSlot,
                     intro->padStopKind);
    }
    AppendAction(plan,
                 PracticeActionKind::PlayCue80026EF8,
                 kFn80026EF8,
                 kCuePracticeLoop8006EC90,
                 round,
                 112,
                 static_cast<int32_t>(kCuePracticeLoop8006EC90),
                 0,
                 0,
                 0,
                 PracticeCueKind::LoopStart);
    AppendAction(plan,
                 PracticeActionKind::PlayCue80026EF8,
                 kFn80026EF8,
                 kCuePracticeBeat8006EC98,
                 round,
                 0,
                 static_cast<int32_t>(kCuePracticeBeat8006EC98),
                 0,
                 0,
                 0,
                 PracticeCueKind::Beat);
    AppendAction(plan,
                 PracticeActionKind::PlayCue80026EF8,
                 kFn80026EF8,
                 kCuePracticeBeat8006EC98,
                 round,
                 75,
                 static_cast<int32_t>(kCuePracticeBeat8006EC98),
                 0,
                 0,
                 0,
                 PracticeCueKind::Beat);
    AppendAction(plan,
                 PracticeActionKind::PollInput80035510,
                 kFn80035510,
                 0,
                 round);
    AppendAction(plan,
                 PracticeActionKind::DebounceInputGp844,
                 kFn8002776C,
                 0,
                 round);
    if (prompt != nullptr) {
        AppendAction(plan,
                     PracticeActionKind::JudgeTriangle,
                     kFn8002776C,
                     kPracticeRoundTargetTable80055484,
                     round,
                     -1,
                     prompt->round,
                     static_cast<int32_t>(prompt->address),
                     0,
                     0,
                     promptCue,
                     true);
        AppendAction(plan,
                     PracticeActionKind::PlayCue80026EF8,
                     kFn80026EF8,
                     prompt->address,
                     round,
                     -1,
                     static_cast<int32_t>(prompt->address),
                     0,
                     0,
                     0,
                     promptCue,
                     true);
    }
    AppendAction(plan,
                 PracticeActionKind::FastForwardCross,
                 kFn8002776C,
                 0,
                 round,
                 -1,
                 135,
                 1,
                 1,
                 0,
                 PracticeCueKind::Unknown,
                 true);
    AppendPlan(plan, BuildPracticePadStop800276ECPlan(
                         PracticeCueKind::VoiceOnBeat));
    AppendPlan(plan, BuildPracticePadStop800276ECPlan(
                         PracticeCueKind::VoiceTooQuick));
    AppendPlan(plan, BuildPracticePadStop800276ECPlan(
                         PracticeCueKind::VoiceTooSlow));
    return plan;
}

PracticePlan BuildPracticeLoop8002776CPlan(uint32_t menuCtx,
                                           int32_t prevScene)
{
    PracticePlan plan = MakePlan("PracticeLoop8002776C");
    plan.blockedByGap = true;
    const bool ctxSourceKnown = menuCtx == kScene0WorkAddress;

    AppendPlan(plan, BuildPracticeResourceLoad80015618Plan());
    AppendAction(plan,
                 PracticeActionKind::GatePracticeLoopCtxSource8002776C,
                 kFn8002776C,
                 ctxSourceKnown ? menuCtx : 0u,
                 -1,
                 -1,
                 static_cast<int32_t>(menuCtx),
                 prevScene,
                 static_cast<int32_t>(kScene0WorkAddress),
                 ctxSourceKnown ? 1 : 0,
                 PracticeCueKind::Unknown,
                 true);
    if (!ctxSourceKnown) {
        AppendAction(plan,
                     PracticeActionKind::Gap,
                     kFn8002776C,
                     menuCtx,
                     -1,
                     -1,
                     prevScene,
                     static_cast<int32_t>(kScene0WorkAddress));
    }
    AppendAction(plan,
                 PracticeActionKind::SetVSyncPadStart,
                 kFn8002776C,
                 0,
                 -1,
                 -1,
                 static_cast<int32_t>(menuCtx));
    AppendAction(plan,
                 PracticeActionKind::InitCtx,
                 kFn8002776C,
                 menuCtx);

    for (uint32_t round = 0; round < kPracticeRoundCount; ++round) {
        AppendPlan(plan, BuildPracticeRound8002776CPlan(
                             static_cast<int32_t>(round)));
        AppendPlan(plan, BuildPracticeDrawFramePlan(menuCtx));
    }

    AppendPlan(plan, BuildPracticePadStop800276ECPlan(
                         PracticeCueKind::VoiceExit7));
    AppendPlan(plan, BuildPracticePadStop800276ECPlan(
                         PracticeCueKind::VoiceExit8));
    AppendAction(plan,
                 PracticeActionKind::ExitConfirmLoop,
                 kFn8002776C,
                 menuCtx,
                 -1,
                 static_cast<int32_t>(kPracticeExitConfirmFrames),
                 0x20,
                 0x40);
    AppendAction(plan,
                 PracticeActionKind::RestorePrevScene80015590,
                 kFn80015590,
                 0,
                 -1,
                 -1,
                 prevScene);
    AppendAction(plan,
                 PracticeActionKind::Gap,
                 kFn8002776C,
                 menuCtx);
    return plan;
}

bool BeginPracticeExitConfirm8002776C(
    PracticeExitConfirmRuntime8002776C& runtime,
    int16_t initialBlink4C) {
    runtime = {};
    runtime.initialized = true;
    runtime.framesRemaining =
        static_cast<int32_t>(kPracticeExitConfirmFrames);
    runtime.exitSelection48 = 0;
    runtime.exitBlink4C = initialBlink4C;
    return true;
}

bool TickPracticeExitConfirm8002776C(
    PracticeExitConfirmRuntime8002776C& runtime,
    const PracticeExitConfirmInput8002776C& input) {
    if (!runtime.initialized || !input.edgeKnown ||
        runtime.result != PracticeExitConfirmResult8002776C::Pending ||
        runtime.framesRemaining <= 0 ||
        runtime.framesRemaining >
            static_cast<int32_t>(kPracticeExitConfirmFrames) ||
        runtime.blinkCounter >= 20u) {
        return false;
    }

    if (input.edge == 0x20u) {
        runtime.choiceArmed = true;
        runtime.retrySelected = true;
    } else if (input.edge == 0x40u) {
        runtime.choiceArmed = true;
        runtime.retrySelected = false;
        runtime.exitSelection48 = 1;
        runtime.exitBlink4C = 1;
    }

    if (runtime.choiceArmed) {
        --runtime.framesRemaining;
    }

    if (runtime.blinkCounter == 0u) {
        // 80027F30..44 maps exactly one to zero and every other halfword to one.
        runtime.exitBlink4C = runtime.exitBlink4C == 1 ? 0 : 1;
        runtime.blinkCounter = 1u;
    } else if (runtime.blinkCounter == 19u) {
        runtime.blinkCounter = 0u;
    } else {
        ++runtime.blinkCounter;
    }

    if (runtime.choiceArmed && runtime.framesRemaining == 0) {
        runtime.result = runtime.retrySelected
            ? PracticeExitConfirmResult8002776C::RetryPractice
            : PracticeExitConfirmResult8002776C::RestorePreviousScene;
    }
    return true;
}

const char* PracticeActionKindName(PracticeActionKind kind)
{
    switch (kind) {
    case PracticeActionKind::None:
        return "None";
    case PracticeActionKind::CallLoadYCompo80015618:
        return "CallLoadYCompo80015618";
    case PracticeActionKind::ClearResourceTable80025A34:
        return "ClearResourceTable80025A34";
    case PracticeActionKind::LoadInt8001AC18:
        return "LoadInt8001AC18";
    case PracticeActionKind::TimUpload:
        return "TimUpload";
    case PracticeActionKind::VabLoadPractice:
        return "VabLoadPractice";
    case PracticeActionKind::SetVSyncPadStart:
        return "SetVSyncPadStart";
    case PracticeActionKind::GatePracticeLoopCtxSource8002776C:
        return "GatePracticeLoopCtxSource8002776C";
    case PracticeActionKind::InitCtx:
        return "InitCtx";
    case PracticeActionKind::GatePadStopCueSource800276EC:
        return "GatePadStopCueSource800276EC";
    case PracticeActionKind::PadStopCom800276EC:
        return "PadStopCom800276EC";
    case PracticeActionKind::PlayCue80026EF8:
        return "PlayCue80026EF8";
    case PracticeActionKind::Flush80026ECC:
        return "Flush80026ECC";
    case PracticeActionKind::ResetEventText80024E98:
        return "ResetEventText80024E98";
    case PracticeActionKind::PollInput80035510:
        return "PollInput80035510";
    case PracticeActionKind::DebounceInputGp844:
        return "DebounceInputGp844";
    case PracticeActionKind::GatePracticeRoundRec44Source8002776C:
        return "GatePracticeRoundRec44Source8002776C";
    case PracticeActionKind::JudgeTriangle:
        return "JudgeTriangle";
    case PracticeActionKind::FastForwardCross:
        return "FastForwardCross";
    case PracticeActionKind::AdvanceRec44:
        return "AdvanceRec44";
    case PracticeActionKind::DrawEvent16_8001E750:
        return "DrawEvent16_8001E750";
    case PracticeActionKind::DrawPractice80023618:
        return "DrawPractice80023618";
    case PracticeActionKind::WaitFrame80035560_2:
        return "WaitFrame80035560_2";
    case PracticeActionKind::EndFrame8001EA00_0:
        return "EndFrame8001EA00_0";
    case PracticeActionKind::ExitConfirmLoop:
        return "ExitConfirmLoop";
    case PracticeActionKind::RestorePrevScene80015590:
        return "RestorePrevScene80015590";
    case PracticeActionKind::Gap:
        return "Gap";
    }
    return "Unknown";
}

const char* PracticeCueKindName(PracticeCueKind kind)
{
    switch (kind) {
    case PracticeCueKind::Unknown:
        return "Unknown";
    case PracticeCueKind::LoopStart:
        return "LoopStart";
    case PracticeCueKind::Beat:
        return "Beat";
    case PracticeCueKind::RoundPrompt0:
        return "RoundPrompt0";
    case PracticeCueKind::RoundPrompt1:
        return "RoundPrompt1";
    case PracticeCueKind::RoundPrompt2:
        return "RoundPrompt2";
    case PracticeCueKind::RoundPrompt3:
        return "RoundPrompt3";
    case PracticeCueKind::VoiceOnBeat:
        return "VoiceOnBeat";
    case PracticeCueKind::VoiceTooQuick:
        return "VoiceTooQuick";
    case PracticeCueKind::VoiceTooSlow:
        return "VoiceTooSlow";
    case PracticeCueKind::VoiceIntro0:
        return "VoiceIntro0";
    case PracticeCueKind::VoiceIntro1:
        return "VoiceIntro1";
    case PracticeCueKind::VoiceIntro2:
        return "VoiceIntro2";
    case PracticeCueKind::VoiceIntro3:
        return "VoiceIntro3";
    case PracticeCueKind::VoiceExit7:
        return "VoiceExit7";
    case PracticeCueKind::VoiceExit8:
        return "VoiceExit8";
    }
    return "Unknown";
}

const char* PracticeCtxFieldKindName(PracticeCtxFieldKind kind)
{
    switch (kind) {
    case PracticeCtxFieldKind::Flags:
        return "Flags";
    case PracticeCtxFieldKind::OverlayKind:
        return "OverlayKind";
    case PracticeCtxFieldKind::ExitSelected:
        return "ExitSelected";
    case PracticeCtxFieldKind::Blink:
        return "Blink";
    case PracticeCtxFieldKind::SeqMode:
        return "SeqMode";
    case PracticeCtxFieldKind::SeqCurA:
        return "SeqCurA";
    case PracticeCtxFieldKind::SeqEnableA:
        return "SeqEnableA";
    case PracticeCtxFieldKind::SeqStreamA:
        return "SeqStreamA";
    case PracticeCtxFieldKind::SeqCurB:
        return "SeqCurB";
    case PracticeCtxFieldKind::SeqEnableB:
        return "SeqEnableB";
    case PracticeCtxFieldKind::SeqStreamB:
        return "SeqStreamB";
    }
    return "Unknown";
}

} // namespace PrSS0PracticeLifecycleDirect
