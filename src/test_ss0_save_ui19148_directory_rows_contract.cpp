#include "pr/pr_game_context.h"
#include "pr/pr_pad.h"
#include "pr/pr_stage1_save_card_hal_direct.h"
#include "pr/pr_stage1_save_ui_direct.h"

#include <array>
#include <cstddef>
#include <cstdio>
#include <cstring>

namespace {

uint16_t g_nextPressedMask = 0;

} // namespace

PrPadState PrPad::GetState(int port) {
    (void)port;
    PrPadState state{};
    state.held = g_nextPressedMask;
    state.pressed = g_nextPressedMask;
    g_nextPressedMask = 0;
    return state;
}

namespace {

int g_failedChecks = 0;
bool g_directDurableBackendCalled = false;
int32_t g_directDurableBackendBlockIndex = -1;
const uint8_t* g_directDurableBackendBytes = nullptr;
std::size_t g_directDurableBackendByteCount = 0u;
void* g_directDurableBackendUser = nullptr;

#define CHECK(expr)                                                         \
    do {                                                                    \
        if (!(expr)) {                                                      \
            std::printf("CHECK failed %s:%d: %s\n", __FILE__, __LINE__,    \
                        #expr);                                             \
            ++g_failedChecks;                                               \
        }                                                                   \
    } while (0)

bool HasDirectoryRequest(
    const PrStage1SaveUi19148LowerFeedbackRequestList& requests) {
    for (uint32_t i = 0; i < requests.count; ++i) {
        if (requests.requests[i].kind ==
            PrStage1SaveUi19148LowerFeedbackRequestKind::
                DirectoryRows80019458) {
            return true;
        }
    }
    return false;
}

PrStage1SaveUi19148LowerFeedbackRequest FindFormatRequest(
    const PrStage1SaveUi19148TickResult& result) {
    const PrStage1SaveUi19148LowerFeedbackRequestList requests =
        PrStage1SaveUiDirect::BuildLowerFeedbackRequests19148(
            result.actions);
    for (uint32_t i = 0; i < requests.count; ++i) {
        const PrStage1SaveUi19148LowerFeedbackRequest& request =
            requests.requests[i];
        if (request.kind ==
                PrStage1SaveUi19148LowerFeedbackRequestKind::
                    Format80017B60 &&
            request.psxFunction == 0x80017B60u &&
            request.action.kind ==
                PrStage1SaveUi19148ActionKind::Call80017B60FormatCard) {
            return request;
        }
    }
    CHECK(false);
    return {};
}

PrStage1SaveCardHalDirect::SaveUiFormatRuntimeFacts80017B60
MakeFormatRuntimeFactsForRequest(
    const PrStage1SaveUi19148LowerFeedbackRequest& request,
    int32_t poll0,
    int32_t poll1,
    int32_t poll2) {
    PrStage1SaveCardHalDirect::SaveUiFormatRuntimeFacts80017B60 facts{};
    facts.factsKnown = true;
    const int32_t polls[PrStage1SaveCardHalDirect::kFormatAttemptCount80017B60] =
        {poll0, poll1, poll2};
    for (int32_t i = 0;
         i < PrStage1SaveCardHalDirect::kFormatAttemptCount80017B60;
         ++i) {
        PrStage1SaveCardHalDirect::SaveUiFormatRuntimeAttemptFacts80017B60&
            attempt = facts.attempts[i];
        attempt.drainHwEventsKnown8001707C = true;
        attempt.formatKnown = true;
        attempt.formatArgsKnown = true;
        attempt.formatArg0 = request.formatArg0;
        attempt.formatArg1 = request.formatArg1;
        attempt.pollResultKnown80017008 = true;
        attempt.pollResult80017008 = polls[i];
    }
    return facts;
}

PrStage1SaveStatusPrefix80092F10 MakeSaveStatusSeed80092F10() {
    PrStage1SaveStatusPrefix80092F10 seed{};
    seed.known = true;
    seed.statusBankKnown80092F1D = true;
    seed.helperGap = false;
    seed.psxAddress = PrStage1SaveStatusPrefix80092F10::kPsxAddress;
    seed.byteCount = PrStage1SaveStatusPrefix80092F10::kByteCount;
    seed.lastWriterFunction = PrStagePayloadBankDirect::kFn800164B4;
    seed.seedAuthorityFunction = PrStagePayloadBankDirect::kFn800164B4;
    seed.wrote800164B4 = true;
    for (uint32_t i = 0; i < seed.byteCount; ++i) {
        seed.bytes[i] = static_cast<uint8_t>(i & 0xFFu);
    }
    return seed;
}

bool HasHelperGap(const PrStage1SaveUi19148TickResult& result,
                  uint32_t psxFunction) {
    for (uint32_t i = 0; i < result.actions.count; ++i) {
        const PrStage1SaveUi19148Action& action = result.actions.actions[i];
        if (action.kind == PrStage1SaveUi19148ActionKind::HelperGap &&
            action.psxFunction == psxFunction) {
            return true;
        }
    }
    return false;
}

const PrStage1SaveUi19148Action* FindAction(
    const PrStage1SaveUi19148TickResult& result,
    PrStage1SaveUi19148ActionKind kind,
    int32_t arg0 = -1) {
    for (uint32_t i = 0; i < result.actions.count; ++i) {
        const PrStage1SaveUi19148Action& action = result.actions.actions[i];
        if (action.kind == kind && (arg0 < 0 || action.arg0 == arg0)) {
            return &action;
        }
    }
    return nullptr;
}

PrStage1SaveUiDirectoryScanFacts80019458 MakeScanFacts(int32_t occupiedEntries,
                                                       int32_t freeSlots) {
    PrStage1SaveUiDirectoryScanFacts80019458 facts{};
    const int32_t entryCount = occupiedEntries + freeSlots;
    facts.known = true;
    facts.directoryRowsKnown80017B08 = true;
    facts.snapshotKnown80017B18 = true;
    facts.listRowsBuilt80019458 = true;
    facts.entryCountKnown = true;
    facts.entryCount = entryCount;
    facts.freeSlotsKnown = true;
    facts.freeSlots = freeSlots;
    for (int32_t i = 0; i < occupiedEntries; ++i) {
        facts.rows[i].active = true;
        facts.rows[i].blockIndexKnown = true;
        facts.rows[i].blockIndex = i;
        facts.rows[i].suffixKnown = true;
        std::snprintf(facts.rows[i].suffix,
                      sizeof(facts.rows[i].suffix),
                      "ROW%d",
                      i);
    }
    for (int32_t i = occupiedEntries; i < entryCount && i < 15; ++i) {
        facts.rows[i].active = true;
        facts.rows[i].freeSlot = true;
        facts.rows[i].blockIndexKnown = true;
        facts.rows[i].blockIndex = i;
        facts.rows[i].suffixKnown = true;
        facts.rows[i].suffix[0] = '\0';
    }
    return facts;
}

PrStage1SaveCardHalDirect::SaveUiDirectoryScanRuntimeFacts80019458
MakeRuntimeScanFacts(int32_t entryCount, int32_t freeSlots) {
    const PrStage1SaveUiDirectoryScanFacts80019458 facts =
        MakeScanFacts(entryCount, freeSlots);
    PrStage1SaveCardHalDirect::SaveUiDirectoryScanRuntimeFacts80019458
        runtime{};
    runtime.factsKnown = facts.known;
    runtime.directoryRowsKnown80017B08 =
        facts.directoryRowsKnown80017B08;
    runtime.snapshotKnown80017B18 = facts.snapshotKnown80017B18;
    runtime.listRowsBuilt80019458 = facts.listRowsBuilt80019458;
    runtime.entryCountKnown = facts.entryCountKnown;
    runtime.entryCount = facts.entryCount;
    runtime.freeSlotsKnown = facts.freeSlotsKnown;
    runtime.freeSlots = facts.freeSlots;
    for (int i = 0; i < 15; ++i) {
        runtime.rows[i] = facts.rows[i];
    }
    return runtime;
}

PrStage1SaveUi19148LowerFeedback MakeDirectoryFeedback(
    int32_t occupiedEntries,
    int32_t freeSlots) {
    const PrStage1SaveUiDirectoryScanFacts80019458 facts =
        MakeScanFacts(occupiedEntries, freeSlots);
    PrStage1SaveUi19148LowerFeedback feedback{};
    feedback.directoryRowsFeedbackKnown80019458 = true;
    PrStage1SaveUiDirectoryRowsFeedback80019458& directory =
        feedback.directoryRowsFeedback80019458;
    directory.translated = true;
    directory.sourceKnown = true;
    directory.source =
        PrStage1SaveUiDirectoryRowsSource80019458::
            RuntimeCardDirectoryProducer;
    directory.entryCountKnown = true;
    directory.entryCount = facts.entryCount;
    directory.freeSlotsKnown = true;
    directory.freeSlots = facts.freeSlots;
    directory.rowsKnown = true;
    for (int i = 0; i < 15; ++i) {
        directory.rows[i] = facts.rows[i];
    }
    return feedback;
}

std::array<uint8_t, 600> MakeRawDirectoryBankWithOneSave() {
    std::array<uint8_t, 600> bank{};
    constexpr int kRowIndex = 3;
    uint8_t* row = bank.data() + kRowIndex * 40;
    const char* name = "BASCUS-94183A";
    std::memcpy(row, name, std::strlen(name) + 1u);
    const uint32_t byteCount = 8192u;
    row[24] = static_cast<uint8_t>(byteCount & 0xFFu);
    row[25] = static_cast<uint8_t>((byteCount >> 8) & 0xFFu);
    row[26] = static_cast<uint8_t>((byteCount >> 16) & 0xFFu);
    row[27] = static_cast<uint8_t>((byteCount >> 24) & 0xFFu);
    return bank;
}

bool TestDirectDurableBackend8007A318(int32_t blockIndex,
                                      const uint8_t* bytes,
                                      std::size_t byteCount,
                                      void* user) {
    g_directDurableBackendCalled = true;
    g_directDurableBackendBlockIndex = blockIndex;
    g_directDurableBackendBytes = bytes;
    g_directDurableBackendByteCount = byteCount;
    g_directDurableBackendUser = user;
    return bytes != nullptr && byteCount == 128u * 1024u && blockIndex == 0;
}

PrStage1SaveUi19148LowerFeedback MakeCardIoFeedback(
    const PrStage1SaveUi19148LowerFeedbackRequest& request) {
    PrStage1SaveUi19148LowerFeedback feedback{};
    feedback.cardIoFeedbackKnown80017594 = true;
    PrStage1SaveUiCardIoFeedback80017594& card =
        feedback.cardIoFeedback80017594;
    card.stateBeforeKnown = true;
    card.stateBefore = request.cardIoState;
    switch (request.cardIoState.dword800917E8) {
    case 0:
        card.cardInfoKnown = true;
        card.cardInfoArgKnown = true;
        card.cardInfoArg = 0;
        break;
    case 1:
    case 3:
        card.pollSwKnown80016E18 = true;
        card.pollSwResult80016E18 = 1;
        card.pollSwGp700BeforeKnown80016E18 = true;
        card.pollSwGp700Before80016E18 = request.cardIoState.gp700;
        card.pollSwGp700AfterKnown80016E18 = true;
        card.pollSwGp700After80016E18 = request.cardIoState.gp700 - 1;
        card.pollSwTimedOutKnown80016E18 = true;
        card.pollSwTimedOut80016E18 = false;
        break;
    case 2:
        card.clearSwEventsKnown80016FC0 = true;
        card.cardLoadKnown = true;
        card.cardLoadArgKnown = true;
        card.cardLoadArg = 0;
        break;
    default:
        break;
    }
    return feedback;
}

PrStage1SaveUi19148LowerFeedback MakeCardIoNewAtLoadFeedback(
    const PrStage1SaveUi19148LowerFeedbackRequest& request) {
    PrStage1SaveUi19148LowerFeedback feedback = MakeCardIoFeedback(request);
    if (request.cardIoState.dword800917E8 == 3) {
        PrStage1SaveUiCardIoFeedback80017594& card =
            feedback.cardIoFeedback80017594;
        card.pollSwResult80016E18 = 4;
    }
    return feedback;
}

PrStage1SaveUi19148LowerFeedbackRequest FindCardIoRequest(
    const PrStage1SaveUi19148TickResult& result) {
    const PrStage1SaveUi19148LowerFeedbackRequestList requests =
        PrStage1SaveUiDirect::BuildLowerFeedbackRequests19148(result.actions);
    for (uint32_t i = 0; i < requests.count; ++i) {
        if (requests.requests[i].kind ==
            PrStage1SaveUi19148LowerFeedbackRequestKind::CardIo80017594) {
            return requests.requests[i];
        }
    }
    CHECK(false);
    return {};
}

PrStage1SaveUi19148LowerFeedbackRequest FindDirectoryRowsRequest(
    const PrStage1SaveUi19148TickResult& result) {
    const PrStage1SaveUi19148LowerFeedbackRequestList requests =
        PrStage1SaveUiDirect::BuildLowerFeedbackRequests19148(result.actions);
    for (uint32_t i = 0; i < requests.count; ++i) {
        if (requests.requests[i].kind ==
            PrStage1SaveUi19148LowerFeedbackRequestKind::
                DirectoryRows80019458) {
            return requests.requests[i];
        }
    }
    CHECK(false);
    return {};
}

PrStage1SaveUi19148TickResult PumpCardIoUntilResult(
    PrGameContext& ctx,
    int32_t expectedResult) {
    PrStage1SaveUi19148TickResult last{};
    for (int i = 0; i < 8; ++i) {
        PrStage1SaveUi19148TickResult requestTick =
            PrStage1SaveUiDirect::Tick19148(ctx);
        if (requestTick.ioResultKnown && requestTick.ioResult == expectedResult) {
            return requestTick;
        }
        const PrStage1SaveUi19148LowerFeedbackRequest request =
            FindCardIoRequest(requestTick);
        PrStage1SaveUi19148LowerFeedback feedback =
            MakeCardIoFeedback(request);
        last = PrStage1SaveUiDirect::Tick19148(ctx, &feedback);
        if (last.ioResultKnown && last.ioResult == expectedResult) {
            return last;
        }
    }
    CHECK(false);
    return last;
}

PrStage1SaveUi19148TickResult PumpCardIoUntilResult(
    PrGameContext& ctx,
    int32_t expectedResult,
    PrStage1SaveUi19148LowerFeedback (*makeFeedback)(
        const PrStage1SaveUi19148LowerFeedbackRequest&)) {
    PrStage1SaveUi19148TickResult last{};
    for (int i = 0; i < 8; ++i) {
        PrStage1SaveUi19148TickResult requestTick =
            PrStage1SaveUiDirect::Tick19148(ctx);
        if (requestTick.ioResultKnown && requestTick.ioResult == expectedResult) {
            return requestTick;
        }
        const PrStage1SaveUi19148LowerFeedbackRequest request =
            FindCardIoRequest(requestTick);
        PrStage1SaveUi19148LowerFeedback feedback = makeFeedback(request);
        last = PrStage1SaveUiDirect::Tick19148(ctx, &feedback);
        if (last.ioResultKnown && last.ioResult == expectedResult) {
            return last;
        }
    }
    CHECK(false);
    return last;
}

PrStage1SaveUi19148TickResult PumpCardIoUntilFormatGap(
    PrGameContext& ctx) {
    PrStage1SaveUi19148TickResult last{};
    for (int i = 0; i < 8; ++i) {
        PrStage1SaveUi19148TickResult requestTick =
            PrStage1SaveUiDirect::Tick19148(ctx);
        if (requestTick.psxState == 4 &&
            HasHelperGap(requestTick, 0x80017B60u)) {
            return requestTick;
        }
        const PrStage1SaveUi19148LowerFeedbackRequest request =
            FindCardIoRequest(requestTick);
        PrStage1SaveUi19148LowerFeedback feedback =
            MakeCardIoFeedback(request);
        last = PrStage1SaveUiDirect::Tick19148(ctx, &feedback);
        if (last.psxState == 4 && HasHelperGap(last, 0x80017B60u)) {
            return last;
        }
    }
    CHECK(false);
    return last;
}

void AdvanceToState6(PrGameContext& ctx) {
    g_nextPressedMask = 0;
    PrStage1SaveUiDirect::Reset19148();
    const PrStage1SaveStatusPrefix80092F10 seed =
        MakeSaveStatusSeed80092F10();
    CHECK(PrStage1SaveUiDirect::Start19148(ctx, &seed));
    PrStage1SaveUi19148TickResult initial =
        PrStage1SaveUiDirect::Tick19148(ctx);
    CHECK(initial.psxState == 2);

    g_nextPressedMask = static_cast<uint16_t>(PrPadButton::Cross);
    PrStage1SaveUi19148TickResult inputTick =
        PrStage1SaveUiDirect::Tick19148(ctx);
    CHECK(inputTick.psxState == 4);
    CHECK(inputTick.inputHandled800185D0);

    PrStage1SaveUi19148TickResult cardTick = PumpCardIoUntilResult(ctx, 1);
    CHECK(cardTick.psxState == 6);
    CHECK(cardTick.ioResultKnown);
    CHECK(cardTick.ioResult == 1);
}

void AdvanceToListState11(
    PrGameContext& ctx,
    const PrStage1SaveUi19148LowerFeedback& directoryFeedback) {
    AdvanceToState6(ctx);
    PrStage1SaveUi19148TickResult state9 =
        PrStage1SaveUiDirect::Tick19148(ctx, &directoryFeedback);
    CHECK(state9.psxState == 9);
    CHECK(!state9.helperGap);
    PrStage1SaveUi19148TickResult state11 =
        PrStage1SaveUiDirect::Tick19148(ctx, &directoryFeedback);
    CHECK(state11.psxState == 11);
    CHECK(!state11.helperGap);
}

const PrStage1SaveUi19148Action* MoveListSelection(
    PrGameContext& ctx,
    PrPadButton button,
    PrStage1SaveUi19148TickResult& result) {
    g_nextPressedMask = static_cast<uint16_t>(button);
    result = PrStage1SaveUiDirect::Tick19148(ctx);
    CHECK(result.psxState == 11);
    CHECK(result.inputHandled800185D0);
    CHECK(result.inputStateBefore800185D0 == 11);
    CHECK(result.inputStateAfter800185D0 == 11);
    const PrStage1SaveUi19148Action* action = FindAction(
        result, PrStage1SaveUi19148ActionKind::Call800181D0ListInput);
    CHECK(action != nullptr);
    if (action) {
        CHECK(action->arg2 == 0);
    }
    PrStage1SaveUi19148TickResult released =
        PrStage1SaveUiDirect::Tick19148(ctx);
    CHECK(released.psxState == 11);
    return action;
}

void AdvanceToState4(PrGameContext& ctx) {
    g_nextPressedMask = 0;
    PrStage1SaveUiDirect::Reset19148();
    const PrStage1SaveStatusPrefix80092F10 seed =
        MakeSaveStatusSeed80092F10();
    CHECK(PrStage1SaveUiDirect::Start19148(ctx, &seed));
    PrStage1SaveUi19148TickResult initial =
        PrStage1SaveUiDirect::Tick19148(ctx);
    CHECK(initial.psxState == 2);

    g_nextPressedMask = static_cast<uint16_t>(PrPadButton::Cross);
    PrStage1SaveUi19148TickResult inputTick =
        PrStage1SaveUiDirect::Tick19148(ctx);
    CHECK(inputTick.psxState == 4);
    CHECK(inputTick.inputHandled800185D0);
}

void TestState4NewAtLoadReachesFormatPromptThenFormatGap() {
    PrGameContext ctx{};
    AdvanceToState4(ctx);

    PrStage1SaveUi19148TickResult newAtLoad =
        PumpCardIoUntilResult(ctx, 5, MakeCardIoNewAtLoadFeedback);
    CHECK(newAtLoad.ioResultKnown);
    CHECK(newAtLoad.ioResult == 5);
    CHECK(newAtLoad.psxState == 8);
    CHECK(!newAtLoad.helperGap);

    g_nextPressedMask = static_cast<uint16_t>(PrPadButton::Cross);
    PrStage1SaveUi19148TickResult state14 =
        PrStage1SaveUiDirect::Tick19148(ctx);
    CHECK(state14.inputHandled800185D0);
    CHECK(state14.inputStateBefore800185D0 == 8);
    CHECK(state14.inputStateAfter800185D0 == 14);
    CHECK(state14.psxState == 14);

    PrStage1SaveUi19148TickResult formatGap =
        PumpCardIoUntilFormatGap(ctx);
    CHECK(formatGap.psxState == 4);
    CHECK(formatGap.helperGap);
    CHECK(HasHelperGap(formatGap, 0x80017B60u));

    const PrStage1SaveUi19148LowerFeedbackRequestList requests =
        PrStage1SaveUiDirect::BuildLowerFeedbackRequests19148(
            formatGap.actions);
    bool sawFormatRequest = false;
    for (uint32_t i = 0; i < requests.count; ++i) {
        const PrStage1SaveUi19148LowerFeedbackRequest& request =
            requests.requests[i];
        if (request.kind ==
                PrStage1SaveUi19148LowerFeedbackRequestKind::
                    Format80017B60 &&
            request.psxFunction == 0x80017B60u &&
            request.action.kind ==
                PrStage1SaveUi19148ActionKind::Call80017B60FormatCard) {
            sawFormatRequest = true;
        }
    }
    CHECK(sawFormatRequest);
}

void TestFormatRuntimeCarrierRequiresExactRequest() {
    PrGameContext ctx{};
    AdvanceToState4(ctx);

    PrStage1SaveUi19148TickResult newAtLoad =
        PumpCardIoUntilResult(ctx, 5, MakeCardIoNewAtLoadFeedback);
    CHECK(newAtLoad.psxState == 8);

    g_nextPressedMask = static_cast<uint16_t>(PrPadButton::Cross);
    PrStage1SaveUi19148TickResult state14 =
        PrStage1SaveUiDirect::Tick19148(ctx);
    CHECK(state14.psxState == 14);

    PrStage1SaveUi19148TickResult formatGap =
        PumpCardIoUntilFormatGap(ctx);
    CHECK(HasHelperGap(formatGap, 0x80017B60u));
    const PrStage1SaveUi19148LowerFeedbackRequest request =
        FindFormatRequest(formatGap);

    PrStage1SaveCardHalDirect::SaveUiFormatRuntimeFacts80017B60 facts =
        MakeFormatRuntimeFactsForRequest(request, 1, 2, 2);

    CHECK(PrStage1SaveCardHalDirect::
              PublishRuntimeSaveUiFormatTypedCarrier80017B60(request,
                                                             facts));
    PrStage1SaveCardHalDirect::SaveUiFormatTypedCarrier80017B60 carrier{};
    CHECK(PrStage1SaveCardHalDirect::GetSaveUiFormatTypedCarrier80017B60(
        &carrier));
    CHECK(carrier.known);
    CHECK(carrier.producerWired8001707C_80017008_80017B60);
    CHECK(carrier.formatCallCompleted80017B60);
    CHECK(carrier.typedFormatResultKnown80017B60);
    CHECK(!carrier.retryExhaustedReturnUnknown80017B60);
    CHECK(carrier.result80017B60 == 1);
    CHECK(!carrier.incomplete);
    CHECK(carrier.lower.lowerFeedbackKnown);
    CHECK(carrier.lower.lowerFeedback.formatFeedbackKnown80017B60);
    CHECK(carrier.lower.lowerFeedback.formatFeedback80017B60
              .attempts[0]
              .pollResult80017008 == 1);

    PrStage1SaveUi19148LowerFeedbackRequest wrongRequest = request;
    wrongRequest.psxFunction = 0x80017B64u;
    CHECK(!PrStage1SaveCardHalDirect::
               PublishRuntimeSaveUiFormatTypedCarrier80017B60(wrongRequest,
                                                              facts));
    CHECK(!PrStage1SaveCardHalDirect::GetSaveUiFormatTypedCarrier80017B60(
        &carrier));
}

void TestFormatRuntimeCarrierBindsTerminalBranchesToExactRequest() {
    PrGameContext ctx{};
    AdvanceToState4(ctx);

    PrStage1SaveUi19148TickResult newAtLoad =
        PumpCardIoUntilResult(ctx, 5, MakeCardIoNewAtLoadFeedback);
    CHECK(newAtLoad.psxState == 8);

    g_nextPressedMask = static_cast<uint16_t>(PrPadButton::Cross);
    PrStage1SaveUi19148TickResult state14 =
        PrStage1SaveUiDirect::Tick19148(ctx);
    CHECK(state14.psxState == 14);

    PrStage1SaveUi19148TickResult formatGap =
        PumpCardIoUntilFormatGap(ctx);
    CHECK(HasHelperGap(formatGap, 0x80017B60u));
    const PrStage1SaveUi19148LowerFeedbackRequest request =
        FindFormatRequest(formatGap);
    CHECK(request.retryCount ==
          PrStage1SaveCardHalDirect::kFormatAttemptCount80017B60);

    const PrStage1SaveCardHalDirect::SaveUiFormatRuntimeFacts80017B60
        pollThreeFacts = MakeFormatRuntimeFactsForRequest(request, 3, 1, 1);
    CHECK(PrStage1SaveCardHalDirect::
              PublishRuntimeSaveUiFormatTypedCarrier80017B60(request,
                                                             pollThreeFacts));
    PrStage1SaveCardHalDirect::SaveUiFormatTypedCarrier80017B60 carrier{};
    CHECK(PrStage1SaveCardHalDirect::GetSaveUiFormatTypedCarrier80017B60(
        &carrier));
    CHECK(carrier.formatCallCompleted80017B60);
    CHECK(carrier.typedFormatResultKnown80017B60);
    CHECK(!carrier.retryExhaustedReturnUnknown80017B60);
    CHECK(carrier.result80017B60 == 3);
    CHECK(carrier.lower.lowerFeedback.formatFeedback80017B60
              .attempts[0]
              .pollResult80017008 == 3);
    PrStage1SaveUiFormatFeedbackCarrier80017B60 pollThreeCarrier =
        PrStage1SaveUiDirect::BuildFormatFeedback80017B60(
            &carrier.lower.lowerFeedback.formatFeedback80017B60);
    CHECK(pollThreeCarrier.resultKnown);
    CHECK(pollThreeCarrier.result == 3);
    CHECK(pollThreeCarrier.attemptsUsed == 1);
    CHECK(pollThreeCarrier.stoppedOnTimeout);

    const PrStage1SaveCardHalDirect::SaveUiFormatRuntimeFacts80017B60
        retryFacts = MakeFormatRuntimeFactsForRequest(request, 2, 2, 2);
    CHECK(PrStage1SaveCardHalDirect::
              PublishRuntimeSaveUiFormatTypedCarrier80017B60(request,
                                                             retryFacts));
    CHECK(PrStage1SaveCardHalDirect::GetSaveUiFormatTypedCarrier80017B60(
        &carrier));
    CHECK(carrier.formatCallCompleted80017B60);
    CHECK(!carrier.typedFormatResultKnown80017B60);
    CHECK(carrier.retryExhaustedReturnUnknown80017B60);
    CHECK(carrier.lower.lowerFeedback.formatFeedback80017B60
              .attempts[2]
              .pollResult80017008 == 2);
    PrStage1SaveUiFormatFeedbackCarrier80017B60 retryCarrier =
        PrStage1SaveUiDirect::BuildFormatFeedback80017B60(
            &carrier.lower.lowerFeedback.formatFeedback80017B60);
    CHECK(retryCarrier.callCompleted);
    CHECK(!retryCarrier.resultKnown);
    CHECK(retryCarrier.retryExhaustedReturnUnknown);
    CHECK(retryCarrier.attemptsUsed ==
          PrStage1SaveCardHalDirect::kFormatAttemptCount80017B60);
    CHECK(!retryCarrier.stoppedOnSuccess);
    CHECK(!retryCarrier.stoppedOnTimeout);

    PrStage1SaveCardHalDirect::ClearSaveUiFormatTypedCarrier80017B60();
}

void TestDirectoryRowsMissingFeedbackFailsClosed() {
    PrGameContext ctx{};
    AdvanceToState6(ctx);

    PrStage1SaveUi19148TickResult blocked = PumpCardIoUntilResult(ctx, 1);
    CHECK(blocked.psxState == 6);
    CHECK(blocked.helperGap);
    CHECK(HasHelperGap(blocked, 0x80019458u));

    const PrStage1SaveUi19148LowerFeedbackRequestList requests =
        PrStage1SaveUiDirect::BuildLowerFeedbackRequests19148(blocked.actions);
    CHECK(HasDirectoryRequest(requests));
}

void TestDirectoryRowsRejectHostlessUnknownSource() {
    PrGameContext ctx{};
    AdvanceToState6(ctx);

    PrStage1SaveUi19148LowerFeedback feedback{};
    feedback.directoryRowsFeedbackKnown80019458 = true;
    feedback.directoryRowsFeedback80019458.translated = true;
    feedback.directoryRowsFeedback80019458.sourceKnown = false;
    feedback.directoryRowsFeedback80019458.entryCountKnown = true;
    feedback.directoryRowsFeedback80019458.entryCount = 0;
    feedback.directoryRowsFeedback80019458.freeSlotsKnown = true;
    feedback.directoryRowsFeedback80019458.freeSlots = 15;
    feedback.directoryRowsFeedback80019458.rowsKnown = true;

    PrStage1SaveUi19148TickResult blocked =
        PrStage1SaveUiDirect::Tick19148(ctx, &feedback);
    CHECK(blocked.psxState == 6);
    CHECK(blocked.helperGap);
    CHECK(HasHelperGap(blocked, 0x80019458u));
}

void TestDirectoryRowsNoEligibleRowsAdvancesToState7() {
    PrGameContext ctx{};
    AdvanceToState6(ctx);

    PrStage1SaveUi19148LowerFeedback feedback{};
    feedback.directoryRowsFeedbackKnown80019458 = true;
    PrStage1SaveUiDirectoryRowsFeedback80019458& directory =
        feedback.directoryRowsFeedback80019458;
    directory.translated = true;
    directory.sourceKnown = true;
    directory.source =
        PrStage1SaveUiDirectoryRowsSource80019458::
            RuntimeCardDirectoryProducer;
    directory.entryCountKnown = true;
    directory.entryCount = 0;
    directory.freeSlotsKnown = true;
    directory.freeSlots = 0;
    directory.rowsKnown = true;

    PrStage1SaveUi19148TickResult advanced =
        PrStage1SaveUiDirect::Tick19148(ctx, &feedback);
    CHECK(advanced.psxState == 7);
    CHECK(!advanced.helperGap);
}

void TestDirectoryRowsValidEntryAdvancesToState9() {
    PrGameContext ctx{};
    AdvanceToState6(ctx);

    PrStage1SaveUi19148LowerFeedback feedback{};
    feedback.directoryRowsFeedbackKnown80019458 = true;
    PrStage1SaveUiDirectoryRowsFeedback80019458& directory =
        feedback.directoryRowsFeedback80019458;
    directory.translated = true;
    directory.sourceKnown = true;
    directory.source =
        PrStage1SaveUiDirectoryRowsSource80019458::
            RuntimeCardDirectoryProducer;
    directory.entryCountKnown = true;
    directory.entryCount = 15;
    directory.freeSlotsKnown = true;
    directory.freeSlots = 14;
    directory.rowsKnown = true;
    directory.rows[0].active = true;
    directory.rows[0].blockIndexKnown = true;
    directory.rows[0].blockIndex = 3;
    directory.rows[0].suffixKnown = true;
    std::strcpy(directory.rows[0].suffix, "TEST");
    for (int i = 1; i < 15; ++i) {
        directory.rows[i].active = true;
        directory.rows[i].freeSlot = true;
        directory.rows[i].blockIndexKnown = true;
        directory.rows[i].blockIndex = i;
        directory.rows[i].suffixKnown = true;
    }

    PrStage1SaveUi19148TickResult advanced =
        PrStage1SaveUiDirect::Tick19148(ctx, &feedback);
    CHECK(advanced.psxState == 9);
    CHECK(!advanced.helperGap);
}

void TestListInputEmptyDirectorySelectsFirstFreeRow800181D0() {
    PrGameContext ctx{};
    const PrStage1SaveUi19148LowerFeedback feedback =
        MakeDirectoryFeedback(0, 15);
    AdvanceToListState11(ctx, feedback);

    g_nextPressedMask = static_cast<uint16_t>(PrPadButton::Cross);
    const PrStage1SaveUi19148TickResult state10 =
        PrStage1SaveUiDirect::Tick19148(ctx);
    CHECK(state10.psxState == 10);
    CHECK(state10.inputHandled800185D0);
    CHECK(state10.inputStateBefore800185D0 == 11);
    CHECK(state10.inputStateAfter800185D0 == 10);
    const PrStage1SaveUi19148Action* input = FindAction(
        state10, PrStage1SaveUi19148ActionKind::Call800181D0ListInput);
    CHECK(input != nullptr);
    if (input) {
        CHECK(input->arg0 == 0x40);
        CHECK(input->arg1 == 0);
        CHECK(input->arg2 == 1);
        CHECK(input->arg3 == 0);
    }
    CHECK(!PrStage1SaveUiDirect::GetSaveUiNameBufferView8007CBE8().known);
}

void TestListInputFullDirectoryFallsBackToTerminal800181D0() {
    PrGameContext ctx{};
    const PrStage1SaveUi19148LowerFeedback feedback =
        MakeDirectoryFeedback(15, 0);
    AdvanceToListState11(ctx, feedback);

    g_nextPressedMask = static_cast<uint16_t>(PrPadButton::Cross);
    const PrStage1SaveUi19148TickResult state2 =
        PrStage1SaveUiDirect::Tick19148(ctx);
    CHECK(state2.psxState == 2);
    CHECK(state2.inputHandled800185D0);
    CHECK(state2.inputStateBefore800185D0 == 11);
    CHECK(state2.inputStateAfter800185D0 == 2);
    const PrStage1SaveUi19148Action* input = FindAction(
        state2, PrStage1SaveUi19148ActionKind::Call800181D0ListInput);
    CHECK(input != nullptr);
    if (input) {
        CHECK(input->arg0 == 0x40);
        CHECK(input->arg1 == 15);
        CHECK(input->arg2 == 2);
    }
    const PrStage1SaveUi19148Action* event7 = FindAction(
        state2,
        PrStage1SaveUi19148ActionKind::Call80017E6CSetEventResult,
        7);
    CHECK(event7 != nullptr);
    if (event7) {
        CHECK(event7->arg1 == 1);
        CHECK(event7->arg2 == 0);
        CHECK(event7->arg3 == 20);
        CHECK(event7->usesCardGridSnapshot80020F94);
        const PrStage1SaveUiCardGridRenderSnapshot80020F94& snapshot =
            state2.actions.cardGridSnapshot80020F94;
        CHECK(snapshot.requestBound);
        CHECK(snapshot.argAddress == 0x80048E50u);
        CHECK(snapshot.rows == 5);
        CHECK(snapshot.columns == 3);
        CHECK(snapshot.itemCount == 15);
        CHECK(snapshot.selected == 15);
        for (std::size_t i = 0u;
             i < kSaveUiCardGridItemCapacity80020F94;
             ++i) {
            CHECK(snapshot.enabled[i] == 1);
            CHECK(std::memchr(
                      snapshot.slotText[i],
                      '\0',
                      kSaveUiCardGridTextCapacity80020F94) != nullptr);
        }
    }
    CHECK(!PrStage1SaveUiDirect::GetSaveUiNameBufferView8007CBE8().known);
}

void TestListInputSparseNavigationAndExistingFilename800181D0() {
    PrStage1SaveUi19148LowerFeedback feedback = MakeDirectoryFeedback(0, 0);
    PrStage1SaveUiDirectoryRowsFeedback80019458& directory =
        feedback.directoryRowsFeedback80019458;
    directory.entryCount = 3;
    directory.freeSlots = 1;

    directory.rows[0].active = true;
    directory.rows[0].freeSlot = true;
    directory.rows[0].blockIndexKnown = true;
    directory.rows[0].blockIndex = 0;
    directory.rows[0].suffixKnown = true;

    directory.rows[3].active = true;
    directory.rows[3].blockIndexKnown = true;
    directory.rows[3].blockIndex = 3;
    directory.rows[3].suffixKnown = true;
    std::strcpy(directory.rows[3].suffix, "ROW3");

    directory.rows[14].active = true;
    directory.rows[14].blockIndexKnown = true;
    directory.rows[14].blockIndex = 14;
    directory.rows[14].suffixKnown = true;
    std::strcpy(directory.rows[14].suffix, "ROW14");

    PrGameContext ctx{};
    AdvanceToListState11(ctx, feedback);

    PrStage1SaveUi19148TickResult moved{};
    const PrStage1SaveUi19148Action* action =
        MoveListSelection(ctx, PrPadButton::Right, moved);
    if (action) {
        CHECK(action->arg0 == 0x2000);
        CHECK(action->arg1 == 3);
    }
    action = MoveListSelection(ctx, PrPadButton::Left, moved);
    if (action) {
        CHECK(action->arg0 == 0x8000);
        CHECK(action->arg1 == 0);
    }
    action = MoveListSelection(ctx, PrPadButton::Left, moved);
    if (action) {
        CHECK(action->arg1 == 15);
    }
    action = MoveListSelection(ctx, PrPadButton::Right, moved);
    if (action) {
        CHECK(action->arg1 == 0);
    }
    action = MoveListSelection(ctx, PrPadButton::Down, moved);
    if (action) {
        CHECK(action->arg0 == 0x4000);
        CHECK(action->arg1 == 3);
    }
    action = MoveListSelection(ctx, PrPadButton::Up, moved);
    if (action) {
        CHECK(action->arg0 == 0x1000);
        CHECK(action->arg1 == 0);
    }
    action = MoveListSelection(ctx, PrPadButton::Down, moved);
    if (action) {
        CHECK(action->arg1 == 3);
    }

    g_nextPressedMask = static_cast<uint16_t>(PrPadButton::Cross);
    const PrStage1SaveUi19148TickResult state22 =
        PrStage1SaveUiDirect::Tick19148(ctx);
    CHECK(state22.psxState == 22);
    const PrStage1SaveUi19148Action* selected = FindAction(
        state22, PrStage1SaveUi19148ActionKind::Call800181D0ListInput);
    CHECK(selected != nullptr);
    if (selected) {
        CHECK(selected->arg1 == 3);
        CHECK(selected->arg2 == 1);
        CHECK(selected->arg3 == 1);
    }
    const PrStage1SaveUiNameBufferView8007CBE8 name =
        PrStage1SaveUiDirect::GetSaveUiNameBufferView8007CBE8();
    CHECK(name.known);
    if (name.known) {
        CHECK(std::strcmp(name.bytes, "BASCUS-94183ROW3") == 0);
    }

    const PrStage1SaveUi19148TickResult released =
        PrStage1SaveUiDirect::Tick19148(ctx);
    CHECK(released.psxState == 22);
    g_nextPressedMask = static_cast<uint16_t>(PrPadButton::Cross);
    const PrStage1SaveUi19148TickResult state15 =
        PrStage1SaveUiDirect::Tick19148(ctx);
    CHECK(state15.psxState == 15);
    CHECK(!HasHelperGap(state15, 0x800185D0u));
    const PrStage1SaveStatusPrefix80092F10 prefix =
        PrStage1SaveUiDirect::GetSaveStatusPrefix80092F10();
    CHECK(prefix.known);
    if (prefix.known) {
        CHECK(std::strcmp(reinterpret_cast<const char*>(prefix.bytes + 1u),
                          "ROW3") == 0);
    }
}

void TestScanFactsBuildDirectoryRowsFeedback() {
    PrStage1SaveUiDirectoryScanFacts80019458 facts = MakeScanFacts(2, 13);
    PrStage1SaveUiDirectoryRowsFeedback80019458 feedback{};

    CHECK(PrStage1SaveUiDirect::
              BuildSaveUiDirectoryRowsFeedbackFromScanFacts80019458(
                  facts,
                  &feedback));
    CHECK(feedback.translated);
    CHECK(feedback.sourceKnown);
    CHECK(feedback.source ==
          PrStage1SaveUiDirectoryRowsSource80019458::
              RuntimeCardDirectoryProducer);
    CHECK(feedback.entryCountKnown);
    CHECK(feedback.entryCount == 15);
    CHECK(feedback.freeSlotsKnown);
    CHECK(feedback.freeSlots == 13);
    CHECK(feedback.rowsKnown);
    CHECK(feedback.rows[0].active);
    CHECK(feedback.rows[0].blockIndexKnown);
    CHECK(feedback.rows[0].blockIndex == 0);
    CHECK(feedback.rows[0].suffixKnown);
    CHECK(feedback.rows[0].suffix[0] == 'R');
}

void TestScanFactsRejectMissingDirectorySource() {
    PrStage1SaveUiDirectoryScanFacts80019458 facts = MakeScanFacts(1, 14);
    facts.snapshotKnown80017B18 = false;
    PrStage1SaveUiDirectoryRowsFeedback80019458 feedback{};
    feedback.sourceKnown = true;

    CHECK(!PrStage1SaveUiDirect::
               BuildSaveUiDirectoryRowsFeedbackFromScanFacts80019458(
                   facts,
                   &feedback));
    CHECK(!feedback.sourceKnown);
    CHECK(!feedback.rowsKnown);
}

void TestScanFactsRejectBadRowAndCount() {
    PrStage1SaveUiDirectoryScanFacts80019458 badCount = MakeScanFacts(1, 14);
    badCount.entryCount = 16;
    PrStage1SaveUiDirectoryRowsFeedback80019458 feedback{};
    CHECK(!PrStage1SaveUiDirect::
               BuildSaveUiDirectoryRowsFeedbackFromScanFacts80019458(
                   badCount,
                   &feedback));

    PrStage1SaveUiDirectoryScanFacts80019458 badRow = MakeScanFacts(1, 14);
    badRow.rows[0].suffixKnown = false;
    CHECK(!PrStage1SaveUiDirect::
               BuildSaveUiDirectoryRowsFeedbackFromScanFacts80019458(
                   badRow,
                   &feedback));
}

void TestScanFactsFeedbackAdvancesState6() {
    PrGameContext ctx{};
    AdvanceToState6(ctx);

    PrStage1SaveUiDirectoryScanFacts80019458 facts = MakeScanFacts(1, 14);
    PrStage1SaveUi19148LowerFeedback lowerFeedback{};
    lowerFeedback.directoryRowsFeedbackKnown80019458 = true;
    PrStage1SaveUiDirectoryRowsFeedback80019458& directory =
        lowerFeedback.directoryRowsFeedback80019458;
    directory.translated = true;
    directory.sourceKnown = true;
    directory.source =
        PrStage1SaveUiDirectoryRowsSource80019458::
            RuntimeCardDirectoryProducer;
    directory.entryCountKnown = true;
    directory.entryCount = facts.entryCount;
    directory.freeSlotsKnown = true;
    directory.freeSlots = facts.freeSlots;
    directory.rowsKnown = true;
    for (int i = 0; i < 15; ++i) {
        directory.rows[i] = facts.rows[i];
    }

    PrStage1SaveUi19148TickResult advanced =
        PrStage1SaveUiDirect::Tick19148(ctx, &lowerFeedback);
    CHECK(advanced.psxState == 9);
    CHECK(!advanced.helperGap);
}

void TestRuntimeDirectoryCarrierFailsClosedWhenIncomplete() {
    PrStage1SaveCardHalDirect::
        ClearSaveUiDirectoryScanTypedCarrier80019458();
    PrStage1SaveCardHalDirect::SaveUiDirectoryScanRuntimeFacts80019458
        facts = MakeRuntimeScanFacts(1, 14);
    facts.snapshotKnown80017B18 = false;

    CHECK(!PrStage1SaveCardHalDirect::
               PublishRuntimeSaveUiDirectoryScanTypedCarrier80019458(
                   facts));
    PrStage1SaveCardHalDirect::SaveUiDirectoryScanTypedCarrier80019458
        carrier{};
    CHECK(!PrStage1SaveCardHalDirect::
               GetSaveUiDirectoryScanTypedCarrier80019458(&carrier));
    CHECK(!carrier.known);
}

void TestRuntimeDirectoryCarrierFeedsState6() {
    PrStage1SaveCardHalDirect::
        ClearSaveUiDirectoryScanTypedCarrier80019458();
    PrStage1SaveCardHalDirect::SaveUiDirectoryScanRuntimeFacts80019458
        facts = MakeRuntimeScanFacts(1, 14);

    CHECK(PrStage1SaveCardHalDirect::
              PublishRuntimeSaveUiDirectoryScanTypedCarrier80019458(
                  facts));
    PrStage1SaveCardHalDirect::SaveUiDirectoryScanTypedCarrier80019458
        carrier{};
    CHECK(PrStage1SaveCardHalDirect::
              GetSaveUiDirectoryScanTypedCarrier80019458(&carrier));
    CHECK(carrier.known);
    CHECK(carrier.producerWired80017B08_80017B18_80019458);
    CHECK(carrier.typedDirectoryRowsKnown80019458);
    CHECK(!carrier.incomplete);
    CHECK(carrier.feedback.sourceKnown);
    CHECK(carrier.feedback.source ==
          PrStage1SaveUiDirectoryRowsSource80019458::
              RuntimeCardDirectoryProducer);

    PrGameContext ctx{};
    AdvanceToState6(ctx);
    PrStage1SaveUi19148LowerFeedback lowerFeedback{};
    lowerFeedback.directoryRowsFeedbackKnown80019458 = true;
    lowerFeedback.directoryRowsFeedback80019458 = carrier.feedback;

    PrStage1SaveUi19148TickResult advanced =
        PrStage1SaveUiDirect::Tick19148(ctx, &lowerFeedback);
    CHECK(advanced.psxState == 9);
    CHECK(!advanced.helperGap);
    PrStage1SaveCardHalDirect::
        ClearSaveUiDirectoryScanTypedCarrier80019458();
}

void TestState10NameEntryCursorPersistsToConfirm() {
    const std::array<uint8_t, 600> bank{};
    PrStage1SaveCardHalDirect::SaveUiDirectoryRawBankFacts80019458 rawFacts{};
    rawFacts.factsKnown = true;
    rawFacts.directoryRowsKnown80017B08 = true;
    rawFacts.snapshotKnown80017B18 = true;
    rawFacts.rawBankKnown8007A318 = true;
    rawFacts.rawBank8007A318 = bank.data();
    rawFacts.rawBankByteCount8007A318 = bank.size();

    PrStage1SaveCardHalDirect::SaveUiDirectoryScanRuntimeFacts80019458 facts{};
    CHECK(PrStage1SaveCardHalDirect::
              BuildRuntimeSaveUiDirectoryScanFactsFromRawBank80019458(
                  rawFacts,
                  &facts));

    PrGameContext ctx{};
    AdvanceToState6(ctx);
    PrStage1SaveUi19148LowerFeedback lowerFeedback{};
    lowerFeedback.directoryRowsFeedbackKnown80019458 = true;
    PrStage1SaveUiDirectoryRowsFeedback80019458& directory =
        lowerFeedback.directoryRowsFeedback80019458;
    directory.translated = true;
    directory.sourceKnown = true;
    directory.source =
        PrStage1SaveUiDirectoryRowsSource80019458::
            RuntimeCardDirectoryProducer;
    directory.entryCountKnown = true;
    directory.entryCount = facts.entryCount;
    directory.freeSlotsKnown = true;
    directory.freeSlots = facts.freeSlots;
    directory.rowsKnown = true;
    for (int i = 0; i < 15; ++i) {
        directory.rows[i] = facts.rows[i];
    }

    PrStage1SaveUi19148TickResult state9 =
        PrStage1SaveUiDirect::Tick19148(ctx, &lowerFeedback);
    CHECK(state9.psxState == 9);
    PrStage1SaveUi19148TickResult state11 =
        PrStage1SaveUiDirect::Tick19148(ctx, &lowerFeedback);
    CHECK(state11.psxState == 11);
    const PrStage1SaveUi19148Action* state11Draw = FindAction(
        state11,
        PrStage1SaveUi19148ActionKind::Call8001E750DrawEvent,
        7);
    CHECK(state11Draw != nullptr);
    if (state11Draw != nullptr) {
        CHECK(state11Draw->usesCardGridSnapshot80020F94);
        const PrStage1SaveUiCardGridRenderSnapshot80020F94& snapshot =
            state11.actions.cardGridSnapshot80020F94;
        CHECK(snapshot.requestBound);
        CHECK(snapshot.argAddress == 0x80048E50u);
        CHECK(snapshot.rows == 5);
        CHECK(snapshot.columns == 3);
        CHECK(snapshot.itemCount == facts.entryCount);
        CHECK(snapshot.selected >= 0);
        CHECK(snapshot.selected <= 15);
    }

    g_nextPressedMask = static_cast<uint16_t>(PrPadButton::Cross);
    PrStage1SaveUi19148TickResult state10 = PrStage1SaveUiDirect::Tick19148(ctx);
    CHECK(state10.inputHandled800185D0);
    CHECK(state10.inputStateBefore800185D0 == 11);
    CHECK(state10.inputStateAfter800185D0 == 10);
    const PrStage1SaveUi19148Action* state10Draw = nullptr;
    for (uint32_t i = 0; i < state10.actions.count; ++i) {
        const PrStage1SaveUi19148Action& action = state10.actions.actions[i];
        if (action.kind ==
                PrStage1SaveUi19148ActionKind::Call8001E750DrawEvent &&
            action.arg0 == 5) {
            state10Draw = &action;
            break;
        }
    }
    CHECK(state10Draw != nullptr);
    if (state10Draw != nullptr) {
        CHECK(state10Draw->cardInfoSnapshot.requestBound);
        CHECK(state10Draw->cardInfoSnapshot.argAddress == 0x80049244u);
        CHECK(state10Draw->cardInfoSnapshot.selectedMarkerKnown);
        CHECK(state10Draw->cardInfoSnapshot.selectedMarker == 0);
        CHECK(state10Draw->cardInfoSnapshot.lowerModeKnown);
        CHECK(state10Draw->cardInfoSnapshot.lowerMode == 0);
        CHECK(state10Draw->cardInfoSnapshot.topIconTemplateSlotsKnown);
        CHECK(state10Draw->cardInfoSnapshot.topIconOffTemplate ==
              0x80052100u);
        CHECK(state10Draw->cardInfoSnapshot.topIconOnTemplate ==
              0x80052110u);
        CHECK(state10Draw->cardInfoSnapshot.encodedPreviewKnown);
        CHECK(state10Draw->cardInfoSnapshot.encodedPreviewByteCount == 0u);
        CHECK(state10Draw->cardInfoSnapshot.lowRamDescriptorKnown);
        CHECK(state10Draw->cardInfoSnapshot.lowRamWidth == 0u);
        CHECK(state10Draw->cardInfoSnapshot.lowRamHeight == 0u);
    }

    // Release the Cross edge that entered state 10 before selecting "A".
    PrStage1SaveUi19148TickResult state10Released =
        PrStage1SaveUiDirect::Tick19148(ctx);
    CHECK(state10Released.psxState == 10);

    g_nextPressedMask = static_cast<uint16_t>(PrPadButton::Cross);
    PrStage1SaveUi19148TickResult append =
        PrStage1SaveUiDirect::Tick19148(ctx);
    CHECK(append.inputHandled800185D0);
    CHECK(append.inputStateBefore800185D0 == 10);
    CHECK(append.inputStateAfter800185D0 == 10);
    const PrStage1SaveUi19148Action* appendDraw = nullptr;
    for (uint32_t i = 0; i < append.actions.count; ++i) {
        const PrStage1SaveUi19148Action& action = append.actions.actions[i];
        if (action.kind ==
                PrStage1SaveUi19148ActionKind::Call8001E750DrawEvent &&
            action.arg0 == 5) {
            appendDraw = &action;
            break;
        }
    }
    CHECK(appendDraw != nullptr);
    if (appendDraw != nullptr) {
        CHECK(appendDraw->cardInfoSnapshot.requestBound);
        CHECK(appendDraw->cardInfoSnapshot.encodedPreviewKnown);
        CHECK(appendDraw->cardInfoSnapshot.encodedPreviewByteCount == 1u);
        CHECK(static_cast<uint8_t>(
                  appendDraw->cardInfoSnapshot.encodedPreview[0]) == 0x41u);
    }

    g_nextPressedMask = static_cast<uint16_t>(PrPadButton::Triangle);
    PrStage1SaveUi19148TickResult erase =
        PrStage1SaveUiDirect::Tick19148(ctx);
    CHECK(erase.inputHandled800185D0);
    CHECK(erase.inputStateAfter800185D0 == 10);
    const PrStage1SaveUi19148Action* eraseDraw = nullptr;
    for (uint32_t i = 0; i < erase.actions.count; ++i) {
        const PrStage1SaveUi19148Action& action = erase.actions.actions[i];
        if (action.kind ==
                PrStage1SaveUi19148ActionKind::Call8001E750DrawEvent &&
            action.arg0 == 5) {
            eraseDraw = &action;
            break;
        }
    }
    CHECK(eraseDraw != nullptr);
    if (eraseDraw != nullptr) {
        CHECK(eraseDraw->cardInfoSnapshot.requestBound);
        CHECK(eraseDraw->cardInfoSnapshot.encodedPreviewKnown);
        CHECK(eraseDraw->cardInfoSnapshot.encodedPreviewByteCount == 0u);
    }

    g_nextPressedMask = static_cast<uint16_t>(PrPadButton::Left);
    PrStage1SaveUi19148TickResult down = PrStage1SaveUiDirect::Tick19148(ctx);
    CHECK(down.inputHandled800185D0);
    CHECK(down.inputStateBefore800185D0 == 10);
    CHECK(down.inputStateAfter800185D0 == 10);
    const PrStage1SaveUi19148Action* endMarkerDraw = nullptr;
    for (uint32_t i = 0; i < down.actions.count; ++i) {
        const PrStage1SaveUi19148Action& action = down.actions.actions[i];
        if (action.kind ==
                PrStage1SaveUi19148ActionKind::Call8001E750DrawEvent &&
            action.arg0 == 5) {
            endMarkerDraw = &action;
            break;
        }
    }
    CHECK(endMarkerDraw != nullptr);
    if (endMarkerDraw != nullptr) {
        CHECK(endMarkerDraw->cardInfoSnapshot.requestBound);
        CHECK(endMarkerDraw->cardInfoSnapshot.selectedMarker == 56);
        CHECK(endMarkerDraw->cardInfoSnapshot.lowerMode == 0);
    }

    g_nextPressedMask = static_cast<uint16_t>(PrPadButton::Cross);
    PrStage1SaveUi19148TickResult confirm =
        PrStage1SaveUiDirect::Tick19148(ctx, &lowerFeedback);
    CHECK(confirm.inputHandled800185D0);
    CHECK(confirm.inputStateBefore800185D0 == 10);
    CHECK(confirm.inputStateAfter800185D0 == 15);
    CHECK(confirm.psxState == 15);
    const PrStage1SaveUi19148Action* confirmDraw = nullptr;
    uint32_t promptFlashCount80017E6C = 0u;
    bool sawRouteUpdate800180D8 = false;
    for (uint32_t i = 0; i < confirm.actions.count; ++i) {
        const PrStage1SaveUi19148Action& action = confirm.actions.actions[i];
        if (action.kind == PrStage1SaveUi19148ActionKind::
                               Call80017E6CSetEventResult) {
            if (action.psxFunction == 0x80017E6Cu) {
                ++promptFlashCount80017E6C;
                CHECK(action.arg3 ==
                      kSaveUiPromptFlashFrameCount80017E6C);
                CHECK(action.arg0 == 5);
                CHECK(action.cardInfoSnapshot.requestBound);
                CHECK(action.cardInfoSnapshot.argAddress == 0x80049244u);
                CHECK(action.cardInfoSnapshot.selectedMarkerKnown);
                CHECK(action.cardInfoSnapshot.selectedMarker == 56);
                CHECK(action.cardInfoSnapshot.lowerModeKnown);
                CHECK(action.cardInfoSnapshot.lowerMode == 1);
                CHECK(action.cardInfoSnapshot.encodedPreviewKnown);
            } else if (action.psxFunction == 0x800180D8u) {
                sawRouteUpdate800180D8 = true;
                CHECK(action.arg3 == 0);
            }
        }
        if (action.kind ==
                PrStage1SaveUi19148ActionKind::Call8001E750DrawEvent &&
            action.arg0 == 5) {
            confirmDraw = &action;
            break;
        }
    }
    CHECK(promptFlashCount80017E6C == 2u);
    CHECK(sawRouteUpdate800180D8);
    CHECK(confirmDraw != nullptr);
    if (confirmDraw != nullptr) {
        CHECK(confirmDraw->cardInfoSnapshot.requestBound);
        CHECK(confirmDraw->cardInfoSnapshot.selectedMarker == 56);
        CHECK(confirmDraw->cardInfoSnapshot.lowerMode == 1);
        CHECK(confirmDraw->arg1 == static_cast<int32_t>(
            PrStage1SaveUiEventArgUpdate80018FB0::
                ForceWord0AndWord2One));
    }
    const PrStage1SaveStatusPrefix80092F10 savePrefix =
        PrStage1SaveUiDirect::GetSaveStatusPrefix80092F10();
    CHECK(savePrefix.known);
    CHECK(std::memcmp(savePrefix.bytes + 1u, "PARAPP", 7u) == 0);
}

void TestRawDirectoryBankRejectsMissingSnapshot() {
    const std::array<uint8_t, 600> bank = MakeRawDirectoryBankWithOneSave();
    PrStage1SaveCardHalDirect::SaveUiDirectoryRawBankFacts80019458
        rawFacts{};
    rawFacts.factsKnown = true;
    rawFacts.directoryRowsKnown80017B08 = true;
    rawFacts.snapshotKnown80017B18 = false;
    rawFacts.rawBankKnown8007A318 = true;
    rawFacts.rawBank8007A318 = bank.data();
    rawFacts.rawBankByteCount8007A318 = bank.size();

    PrStage1SaveCardHalDirect::SaveUiDirectoryScanRuntimeFacts80019458
        facts{};
    CHECK(!PrStage1SaveCardHalDirect::
               BuildRuntimeSaveUiDirectoryScanFactsFromRawBank80019458(
                   rawFacts,
                   &facts));
    CHECK(!facts.factsKnown);
}

void TestRawDirectoryBankBuildsCarrierAndFeedsState6() {
    const std::array<uint8_t, 600> bank = MakeRawDirectoryBankWithOneSave();
    PrStage1SaveCardHalDirect::SaveUiDirectoryRawBankFacts80019458
        rawFacts{};
    rawFacts.factsKnown = true;
    rawFacts.directoryRowsKnown80017B08 = true;
    rawFacts.snapshotKnown80017B18 = true;
    rawFacts.rawBankKnown8007A318 = true;
    rawFacts.rawBank8007A318 = bank.data();
    rawFacts.rawBankByteCount8007A318 = bank.size();

    PrStage1SaveCardHalDirect::SaveUiDirectoryScanRuntimeFacts80019458
        facts{};
    CHECK(PrStage1SaveCardHalDirect::
              BuildRuntimeSaveUiDirectoryScanFactsFromRawBank80019458(
                  rawFacts,
                  &facts));
    CHECK(facts.factsKnown);
    CHECK(facts.directoryRowsKnown80017B08);
    CHECK(facts.snapshotKnown80017B18);
    CHECK(facts.listRowsBuilt80019458);
    CHECK(facts.entryCountKnown);
    CHECK(facts.entryCount == 15);
    CHECK(facts.freeSlotsKnown);
    CHECK(facts.freeSlots == 14);
    CHECK(facts.rows[0].active);
    CHECK(!facts.rows[0].freeSlot);
    CHECK(facts.rows[0].blockIndexKnown);
    CHECK(facts.rows[0].blockIndex == 3);
    CHECK(facts.rows[0].suffixKnown);
    CHECK(std::strcmp(facts.rows[0].suffix, "A") == 0);
    CHECK(facts.rows[1].active);
    CHECK(facts.rows[1].freeSlot);
    CHECK(facts.rows[1].suffixKnown);
    CHECK(facts.rows[1].suffix[0] == '\0');

    PrStage1SaveCardHalDirect::
        ClearSaveUiDirectoryScanTypedCarrier80019458();
    CHECK(PrStage1SaveCardHalDirect::
              PublishRuntimeSaveUiDirectoryScanTypedCarrier80019458(
                  facts));
    PrStage1SaveCardHalDirect::SaveUiDirectoryScanTypedCarrier80019458
        carrier{};
    CHECK(PrStage1SaveCardHalDirect::
              GetSaveUiDirectoryScanTypedCarrier80019458(&carrier));

    PrGameContext ctx{};
    AdvanceToState6(ctx);
    PrStage1SaveUi19148LowerFeedback lowerFeedback{};
    lowerFeedback.directoryRowsFeedbackKnown80019458 = true;
    lowerFeedback.directoryRowsFeedback80019458 = carrier.feedback;

    PrStage1SaveUi19148TickResult advanced =
        PrStage1SaveUiDirect::Tick19148(ctx, &lowerFeedback);
    CHECK(advanced.psxState == 9);
    CHECK(!advanced.helperGap);
    PrStage1SaveCardHalDirect::
        ClearSaveUiDirectoryScanTypedCarrier80019458();
}

void TestEmptyRawDirectoryBankBuildsFreeRowsAndAdvancesState6ToState9() {
    const std::array<uint8_t, 600> bank{};
    PrStage1SaveCardHalDirect::SaveUiDirectoryRawBankFacts80019458
        rawFacts{};
    rawFacts.factsKnown = true;
    rawFacts.directoryRowsKnown80017B08 = true;
    rawFacts.snapshotKnown80017B18 = true;
    rawFacts.rawBankKnown8007A318 = true;
    rawFacts.rawBank8007A318 = bank.data();
    rawFacts.rawBankByteCount8007A318 = bank.size();

    PrStage1SaveCardHalDirect::SaveUiDirectoryScanRuntimeFacts80019458
        facts{};
    CHECK(PrStage1SaveCardHalDirect::
              BuildRuntimeSaveUiDirectoryScanFactsFromRawBank80019458(
                  rawFacts,
                  &facts));
    CHECK(facts.factsKnown);
    CHECK(facts.entryCountKnown);
    CHECK(facts.entryCount == 15);
    CHECK(facts.freeSlotsKnown);
    CHECK(facts.freeSlots == 15);
    CHECK(facts.rows[0].active);
    CHECK(facts.rows[0].freeSlot);
    CHECK(facts.rows[0].blockIndexKnown);
    CHECK(facts.rows[0].blockIndex == 0);
    CHECK(facts.rows[0].suffixKnown);
    CHECK(facts.rows[0].suffix[0] == '\0');

    PrStage1SaveCardHalDirect::
        ClearSaveUiDirectoryScanTypedCarrier80019458();
    CHECK(PrStage1SaveCardHalDirect::
              PublishRuntimeSaveUiDirectoryScanTypedCarrier80019458(
                  facts));
    PrStage1SaveCardHalDirect::SaveUiDirectoryScanTypedCarrier80019458
        carrier{};
    CHECK(PrStage1SaveCardHalDirect::
              GetSaveUiDirectoryScanTypedCarrier80019458(&carrier));

    PrGameContext ctx{};
    AdvanceToState6(ctx);
    PrStage1SaveUi19148LowerFeedback lowerFeedback{};
    lowerFeedback.directoryRowsFeedbackKnown80019458 = true;
    lowerFeedback.directoryRowsFeedback80019458 = carrier.feedback;

    PrStage1SaveUi19148TickResult advanced =
        PrStage1SaveUiDirect::Tick19148(ctx, &lowerFeedback);
    CHECK(advanced.psxState == 9);
    CHECK(!advanced.helperGap);
    PrStage1SaveCardHalDirect::
        ClearSaveUiDirectoryScanTypedCarrier80019458();
}

void TestSerializedEntryUpdatesDirectDirectoryRawBank() {
    PrStage1SaveUiDirect::Reset19148();
    PrStage1SaveUiDirectoryRawBankUpdate8007A318 update =
        PrStage1SaveUiDirect::
            ApplySaveUiDirectoryRawBankSerializedEntry8007A318(
                "BASCUS-94183Z",
                8192u);
    CHECK(update.attempted);
    CHECK(update.nameKnown);
    CHECK(update.directoryKnown);
    CHECK(update.slotKnown);
    CHECK(update.updated);
    CHECK(!update.durablePolicyKnown);
    CHECK(!update.durableCommitted);
    CHECK(update.blockIndex == 0);

    const PrStage1SaveUiDirectoryRawBankView8007A318 view =
        PrStage1SaveUiDirect::GetSaveUiDirectoryRawBankView8007A318();
    PrStage1SaveCardHalDirect::SaveUiDirectoryRawBankFacts80019458
        rawFacts{};
    rawFacts.factsKnown = true;
    rawFacts.directoryRowsKnown80017B08 = true;
    rawFacts.snapshotKnown80017B18 = true;
    rawFacts.rawBankKnown8007A318 = view.known;
    rawFacts.rawBank8007A318 = view.bytes;
    rawFacts.rawBankByteCount8007A318 = view.byteCount;

    PrStage1SaveCardHalDirect::SaveUiDirectoryScanRuntimeFacts80019458
        facts{};
    CHECK(PrStage1SaveCardHalDirect::
              BuildRuntimeSaveUiDirectoryScanFactsFromRawBank80019458(
                  rawFacts,
                  &facts));
    CHECK(facts.entryCountKnown);
    CHECK(facts.entryCount == 15);
    CHECK(facts.freeSlotsKnown);
    CHECK(facts.freeSlots == 14);
    CHECK(facts.rows[0].active);
    CHECK(!facts.rows[0].freeSlot);
    CHECK(facts.rows[0].blockIndexKnown);
    CHECK(facts.rows[0].blockIndex == 0);
    CHECK(facts.rows[0].suffixKnown);
    CHECK(std::strcmp(facts.rows[0].suffix, "Z") == 0);
}

void TestSerializedEntryDirectoryUpdateFailsClosed() {
    PrStage1SaveUiDirect::Reset19148();
    PrStage1SaveUiDirectoryRawBankUpdate8007A318 badName =
        PrStage1SaveUiDirect::
            ApplySaveUiDirectoryRawBankSerializedEntry8007A318(
                "HOST-SLOT",
                8192u);
    CHECK(badName.attempted);
    CHECK(!badName.nameKnown);
    CHECK(!badName.updated);
    CHECK(!badName.durablePolicyKnown);
    CHECK(!badName.durableCommitted);

    for (int i = 0; i < 14; ++i) {
        char name[32]{};
        std::snprintf(name,
                      sizeof(name),
                      "BASCUS-94183%c",
                      static_cast<char>('A' + i));
        PrStage1SaveUiDirectoryRawBankUpdate8007A318 fill =
            PrStage1SaveUiDirect::
                ApplySaveUiDirectoryRawBankSerializedEntry8007A318(name,
                                                                   8192u);
        CHECK(fill.updated);
    }
    PrStage1SaveUiDirectoryRawBankUpdate8007A318 overwrite =
        PrStage1SaveUiDirect::
            ApplySaveUiDirectoryRawBankSerializedEntry8007A318(
                "BASCUS-94183A",
                8192u);
    CHECK(overwrite.updated);
    CHECK(overwrite.overwrote);
    PrStage1SaveUiDirectoryRawBankUpdate8007A318 full =
        PrStage1SaveUiDirect::
            ApplySaveUiDirectoryRawBankSerializedEntry8007A318(
                "BASCUS-94183FULL",
                8192u);
    CHECK(full.nameKnown);
    CHECK(!full.slotKnown);
    CHECK(!full.updated);
    CHECK(!full.durablePolicyKnown);
    CHECK(!full.durableCommitted);
}

uint8_t FrameChecksum(const uint8_t* frame) {
    uint8_t checksum = 0;
    for (std::size_t i = 0; i < 0x7Fu; ++i) {
        checksum ^= frame[i];
    }
    return checksum;
}

PrStage1SaveUi19148TickResult AdvanceToState15WriteAttempt(
    PrGameContext& ctx) {
    AdvanceToState6(ctx);

    const std::array<uint8_t, 600> bank{};
    PrStage1SaveCardHalDirect::SaveUiDirectoryRawBankFacts80019458
        rawFacts{};
    rawFacts.factsKnown = true;
    rawFacts.directoryRowsKnown80017B08 = true;
    rawFacts.snapshotKnown80017B18 = true;
    rawFacts.rawBankKnown8007A318 = true;
    rawFacts.rawBank8007A318 = bank.data();
    rawFacts.rawBankByteCount8007A318 = bank.size();

    PrStage1SaveCardHalDirect::SaveUiDirectoryScanRuntimeFacts80019458
        facts{};
    CHECK(PrStage1SaveCardHalDirect::
              BuildRuntimeSaveUiDirectoryScanFactsFromRawBank80019458(
                  rawFacts,
                  &facts));
    PrStage1SaveUi19148LowerFeedback lowerFeedback{};
    lowerFeedback.directoryRowsFeedbackKnown80019458 = true;
    PrStage1SaveUiDirectoryRowsFeedback80019458& directory =
        lowerFeedback.directoryRowsFeedback80019458;
    directory.translated = true;
    directory.sourceKnown = true;
    directory.source =
        PrStage1SaveUiDirectoryRowsSource80019458::
            RuntimeCardDirectoryProducer;
    directory.entryCountKnown = true;
    directory.entryCount = facts.entryCount;
    directory.freeSlotsKnown = true;
    directory.freeSlots = facts.freeSlots;
    directory.rowsKnown = true;
    for (int i = 0; i < 15; ++i) {
        directory.rows[i] = facts.rows[i];
    }

    PrStage1SaveUi19148TickResult state9 =
        PrStage1SaveUiDirect::Tick19148(ctx, &lowerFeedback);
    CHECK(state9.psxState == 9);
    PrStage1SaveUi19148TickResult state11 =
        PrStage1SaveUiDirect::Tick19148(ctx, &lowerFeedback);
    CHECK(state11.psxState == 11);
    g_nextPressedMask = static_cast<uint16_t>(PrPadButton::Cross);
    PrStage1SaveUi19148TickResult state10 =
        PrStage1SaveUiDirect::Tick19148(ctx);
    CHECK(state10.psxState == 10);
    g_nextPressedMask = static_cast<uint16_t>(PrPadButton::Left);
    PrStage1SaveUi19148TickResult nameInput =
        PrStage1SaveUiDirect::Tick19148(ctx);
    CHECK(nameInput.psxState == 10);
    g_nextPressedMask = static_cast<uint16_t>(PrPadButton::Cross);
    PrStage1SaveUi19148TickResult confirm =
        PrStage1SaveUiDirect::Tick19148(ctx, &lowerFeedback);
    CHECK(confirm.psxState == 15);
    PrStage1SaveUi19148TickResult state15 =
        PrStage1SaveUiDirect::Tick19148(ctx, &lowerFeedback);
    CHECK(state15.psxState == 15);
    CHECK(state15.state15CopyTo8007ADE8Known);
    return state15;
}

void TestSerializedEntryBuildsDirectCardImageCandidate() {
    PrGameContext ctx{};
    PrStage1SaveUi19148TickResult state15 =
        AdvanceToState15WriteAttempt(ctx);
    CHECK(state15.state15CopyTo8007ADE8);

    PrStage1SaveUiWriteBlockView80017A10 blockView =
        PrStage1SaveUiDirect::GetSaveUiWriteBlockView80017A10();
    CHECK(blockView.known);
    CHECK(blockView.byteCount >= 8192u);
    if (!blockView.known || blockView.bytes == nullptr ||
        blockView.byteCount < 8192u) {
        return;
    }
    CHECK(blockView.bytes[0] == 'S');
    CHECK(blockView.bytes[1] == 'C');

    PrStage1SaveUiDirectoryRawBankUpdate8007A318 update =
        PrStage1SaveUiDirect::
            ApplySaveUiDirectoryRawBankSerializedEntry8007A318(
                "BASCUS-94183Z",
                8192u);
    CHECK(update.updated);
    CHECK(update.blockIndex == 0);

    std::array<uint8_t, 128u * 1024u> durableBaseline{};
    durableBaseline[0] = static_cast<uint8_t>('M');
    durableBaseline[1] = static_cast<uint8_t>('C');
    durableBaseline[10u * 8192u + 3u] = 0x6Bu;
    const PrStage1SaveUiCardImageDurableReadIngress8007A318 baseline =
        PrStage1SaveUiDirect::
            ImportSaveUiCardImagePersistenceSinkFromDirectDurableRead8007A318(
                durableBaseline.data(), durableBaseline.size(), 0);
    CHECK(baseline.sinkCommitted);

    PrStage1SaveUiCardImageSerialization8007A318 image =
        PrStage1SaveUiDirect::
            SerializeSaveUiCardImageCandidateFromDirectBuffers8007A318();
    CHECK(image.attempted);
    CHECK(image.directoryKnown);
    CHECK(image.directorySlotKnown);
    CHECK(image.blockViewKnown);
    CHECK(image.imageSerialized);
    CHECK(!image.durablePolicyKnown);
    CHECK(!image.durableCommitted);
    CHECK(image.blockIndex == 0);
    CHECK(image.imageBytes == 128u * 1024u);

    PrStage1SaveUiCardImageView8007A318 imageView =
        PrStage1SaveUiDirect::GetSaveUiCardImageCandidateView8007A318();
    CHECK(imageView.known);
    CHECK(imageView.slotPolicyKnown);
    CHECK(imageView.blockIndex == 0);
    CHECK(!imageView.durablePolicyKnown);
    CHECK(!imageView.durableCommitted);
    CHECK(imageView.byteCount == 128u * 1024u);
    CHECK(imageView.bytes[0] == static_cast<uint8_t>('M'));
    CHECK(imageView.bytes[1] == static_cast<uint8_t>('C'));
    CHECK(imageView.bytes[10u * 8192u + 3u] == 0x6Bu);
    const uint8_t* directoryEntry = imageView.bytes + 128u;
    CHECK(directoryEntry[0] == 0x51u);
    CHECK(directoryEntry[4] == 0x00u);
    CHECK(directoryEntry[5] == 0x20u);
    CHECK(std::memcmp(directoryEntry + 0x0Au, "BASCUS-94183Z", 13u) == 0);
    CHECK(directoryEntry[0x7F] == FrameChecksum(directoryEntry));
    const uint8_t* imageBlock = imageView.bytes + 8192u;
    CHECK(std::memcmp(imageBlock, blockView.bytes, 8192u) == 0);
}

void TestDirectCardImageCandidateFailsClosedWithoutDirectorySlot() {
    PrGameContext ctx{};
    PrStage1SaveUi19148TickResult state15 =
        AdvanceToState15WriteAttempt(ctx);
    CHECK(state15.state15CopyTo8007ADE8);

    PrStage1SaveUiCardImageSerialization8007A318 image =
        PrStage1SaveUiDirect::
            SerializeSaveUiCardImageCandidateFromDirectBuffers8007A318();
    CHECK(image.attempted);
    CHECK(image.directoryKnown);
    CHECK(!image.directorySlotKnown);
    CHECK(image.blockViewKnown);
    CHECK(!image.imageSerialized);
    CHECK(!image.durablePolicyKnown);
    CHECK(!image.durableCommitted);
}

void TestDirectCardImagePersistencePolicyRequiresDurableApi() {
    PrStage1SaveUiDirect::SetSaveUiCardImageDirectDurableCommitBackend8007A318(
        nullptr,
        nullptr);
    PrGameContext ctx{};
    PrStage1SaveUi19148TickResult state15 =
        AdvanceToState15WriteAttempt(ctx);
    CHECK(state15.state15CopyTo8007ADE8);

    PrStage1SaveUiCardImagePersistencePolicy8007A318 missingCandidate =
        PrStage1SaveUiDirect::EvaluateSaveUiCardImagePersistencePolicy8007A318();
    CHECK(missingCandidate.attempted);
    CHECK(!missingCandidate.cardImageCandidateKnown);
    CHECK(!missingCandidate.directDurableStorageApiKnown);
    CHECK(!missingCandidate.persistenceSinkKnown);
    CHECK(!missingCandidate.persistenceSinkCommitted);
    CHECK(!missingCandidate.slotPolicyKnown);
    CHECK(!missingCandidate.durablePolicyKnown);
    CHECK(!missingCandidate.durableCommitted);
    CHECK(std::strcmp(missingCandidate.missingOwner,
                      "Scene8-final-save-direct-card-image-candidate") == 0);
    PrStage1SaveUiCardImagePersistenceSink8007A318 missingSink =
        PrStage1SaveUiDirect::CommitSaveUiCardImagePersistenceSink8007A318();
    CHECK(missingSink.attempted);
    CHECK(!missingSink.cardImageCandidateKnown);
    CHECK(!missingSink.sinkCommitted);
    CHECK(!missingSink.durablePolicyKnown);
    CHECK(!missingSink.durableCommitted);

    PrStage1SaveUiDirectoryRawBankUpdate8007A318 update =
        PrStage1SaveUiDirect::
            ApplySaveUiDirectoryRawBankSerializedEntry8007A318(
                "BASCUS-94183Z",
                8192u);
    CHECK(update.updated);
    PrStage1SaveUiCardImageSerialization8007A318 image =
        PrStage1SaveUiDirect::
            SerializeSaveUiCardImageCandidateFromDirectBuffers8007A318();
    CHECK(image.imageSerialized);
    PrStage1SaveUiCardImageView8007A318 candidateView =
        PrStage1SaveUiDirect::GetSaveUiCardImageCandidateView8007A318();
    CHECK(candidateView.known);
    CHECK(candidateView.bytes != nullptr);
    CHECK(candidateView.bytes[0] == static_cast<uint8_t>('M'));
    CHECK(candidateView.bytes[1] == static_cast<uint8_t>('C'));

    PrStage1SaveUiCardImagePersistencePolicy8007A318 preSinkPolicy =
        PrStage1SaveUiDirect::EvaluateSaveUiCardImagePersistencePolicy8007A318();
    CHECK(preSinkPolicy.attempted);
    CHECK(preSinkPolicy.cardImageCandidateKnown);
    CHECK(preSinkPolicy.directDurableStorageApiKnown);
    CHECK(!preSinkPolicy.persistenceSinkKnown);
    CHECK(!preSinkPolicy.persistenceSinkCommitted);
    CHECK(!preSinkPolicy.slotPolicyKnown);
    CHECK(!preSinkPolicy.durablePolicyKnown);
    CHECK(!preSinkPolicy.durableCommitted);
    CHECK(std::strcmp(preSinkPolicy.missingOwner,
                      "Scene8-final-save-card-image-persistence-sink") == 0);

    PrStage1SaveUiCardImagePersistenceSink8007A318 sink =
        PrStage1SaveUiDirect::CommitSaveUiCardImagePersistenceSink8007A318();
    CHECK(sink.attempted);
    CHECK(sink.cardImageCandidateKnown);
    CHECK(sink.sinkCommitted);
    CHECK(sink.slotPolicyKnown);
    CHECK(sink.blockIndex == 0);
    CHECK(!sink.durablePolicyKnown);
    CHECK(!sink.durableCommitted);
    CHECK(sink.imageBytes == 128u * 1024u);
    PrStage1SaveUiCardImagePersistenceView8007A318 sinkView =
        PrStage1SaveUiDirect::GetSaveUiCardImagePersistenceSinkView8007A318();
    CHECK(sinkView.known);
    CHECK(sinkView.slotPolicyKnown);
    CHECK(sinkView.blockIndex == 0);
    CHECK(!sinkView.durablePolicyKnown);
    CHECK(!sinkView.durableCommitted);
    CHECK(sinkView.byteSize == 128u * 1024u);
    CHECK(sinkView.byteCount == 128u * 1024u);
    CHECK(sinkView.bytes != nullptr);
    CHECK(std::memcmp(sinkView.bytes, candidateView.bytes, 128u * 1024u) == 0);

    PrStage1SaveUiCardImagePersistencePolicy8007A318 policy =
        PrStage1SaveUiDirect::EvaluateSaveUiCardImagePersistencePolicy8007A318();
    CHECK(policy.attempted);
    CHECK(policy.cardImageCandidateKnown);
    CHECK(policy.directDurableStorageApiKnown);
    CHECK(!policy.directDurableCommitBackendKnown);
    CHECK(policy.persistenceSinkKnown);
    CHECK(policy.persistenceSinkCommitted);
    CHECK(policy.slotPolicyKnown);
    CHECK(policy.blockIndex == 0);
    CHECK(policy.explicitNoDurablePolicyKnown);
    CHECK(policy.explicitNoDurablePolicy);
    CHECK(policy.explicitNoSavePersistencePolicyKnown);
    CHECK(policy.explicitNoSavePersistencePolicyAccepted);
    CHECK(policy.explicitNoSavePersistencePolicyFinalized);
    CHECK(!policy.durablePolicyKnown);
    CHECK(!policy.durableCommitted);
    CHECK(policy.imageBytes == 128u * 1024u);
    CHECK(std::strcmp(policy.missingOwner,
                      "Scene8-final-save-explicit-no-durable-policy") == 0);

    PrStage1SaveUiCardImageDurableCommitPrimitive8007A318 missingBackend =
        PrStage1SaveUiDirect::
            CommitSaveUiCardImageDirectDurablePrimitive8007A318();
    CHECK(missingBackend.attempted);
    CHECK(missingBackend.persistenceSinkKnown);
    CHECK(missingBackend.slotPolicyKnown);
    CHECK(missingBackend.blockIndex == 0);
    CHECK(!missingBackend.directBackendKnown);
    CHECK(!missingBackend.directBackendCalled);
    CHECK(!missingBackend.directBackendAccepted);
    CHECK(missingBackend.explicitNoDurablePolicyKnown);
    CHECK(missingBackend.explicitNoDurablePolicy);
    CHECK(missingBackend.explicitNoSavePersistencePolicyKnown);
    CHECK(missingBackend.explicitNoSavePersistencePolicyAccepted);
    CHECK(missingBackend.explicitNoSavePersistencePolicyFinalized);
    CHECK(!missingBackend.durablePolicyKnown);
    CHECK(!missingBackend.durableCommitted);
    CHECK(std::strcmp(missingBackend.missingOwner,
                      "Scene8-final-save-explicit-no-durable-policy") == 0);

    int backendMarker = 7;
    g_directDurableBackendCalled = false;
    g_directDurableBackendBlockIndex = -1;
    g_directDurableBackendBytes = nullptr;
    g_directDurableBackendByteCount = 0u;
    g_directDurableBackendUser = nullptr;
    PrStage1SaveUiDirect::SetSaveUiCardImageDirectDurableCommitBackend8007A318(
        &TestDirectDurableBackend8007A318,
        &backendMarker);
    PrStage1SaveUiCardImagePersistencePolicy8007A318 preCommitPolicy =
        PrStage1SaveUiDirect::EvaluateSaveUiCardImagePersistencePolicy8007A318();
    CHECK(preCommitPolicy.directDurableCommitBackendKnown);
    CHECK(preCommitPolicy.persistenceSinkCommitted);
    CHECK(preCommitPolicy.explicitNoDurablePolicyKnown);
    CHECK(preCommitPolicy.explicitNoDurablePolicy);
    CHECK(preCommitPolicy.explicitNoSavePersistencePolicyKnown);
    CHECK(preCommitPolicy.explicitNoSavePersistencePolicyAccepted);
    CHECK(preCommitPolicy.explicitNoSavePersistencePolicyFinalized);
    CHECK(!preCommitPolicy.durablePolicyKnown);
    CHECK(!preCommitPolicy.durableCommitted);
    CHECK(std::strcmp(preCommitPolicy.missingOwner,
                      "Scene8-final-save-explicit-no-durable-policy") == 0);

    PrStage1SaveUiCardImageDurableCommitPrimitive8007A318 finalizedCommit =
        PrStage1SaveUiDirect::
            CommitSaveUiCardImageDirectDurablePrimitive8007A318();
    CHECK(finalizedCommit.attempted);
    CHECK(finalizedCommit.persistenceSinkKnown);
    CHECK(finalizedCommit.slotPolicyKnown);
    CHECK(finalizedCommit.blockIndex == 0);
    CHECK(finalizedCommit.directBackendKnown);
    CHECK(!finalizedCommit.directBackendCalled);
    CHECK(!finalizedCommit.directBackendAccepted);
    CHECK(finalizedCommit.explicitNoDurablePolicyKnown);
    CHECK(finalizedCommit.explicitNoDurablePolicy);
    CHECK(finalizedCommit.explicitNoSavePersistencePolicyKnown);
    CHECK(finalizedCommit.explicitNoSavePersistencePolicyAccepted);
    CHECK(finalizedCommit.explicitNoSavePersistencePolicyFinalized);
    CHECK(!finalizedCommit.durablePolicyKnown);
    CHECK(!finalizedCommit.durableCommitted);
    CHECK(std::strcmp(finalizedCommit.missingOwner,
                      "Scene8-final-save-explicit-no-durable-policy") == 0);
    CHECK(!g_directDurableBackendCalled);

    PrStage1SaveUiCardImagePersistenceSink8007A318 freshSink =
        PrStage1SaveUiDirect::CommitSaveUiCardImagePersistenceSink8007A318();
    CHECK(freshSink.sinkCommitted);

    PrStage1SaveUiCardImagePersistencePolicy8007A318 freshPolicy =
        PrStage1SaveUiDirect::EvaluateSaveUiCardImagePersistencePolicy8007A318();
    CHECK(freshPolicy.directDurableCommitBackendKnown);
    CHECK(freshPolicy.persistenceSinkCommitted);
    CHECK(!freshPolicy.explicitNoDurablePolicyKnown);
    CHECK(!freshPolicy.explicitNoDurablePolicy);
    CHECK(!freshPolicy.explicitNoSavePersistencePolicyKnown);
    CHECK(!freshPolicy.explicitNoSavePersistencePolicyAccepted);
    CHECK(!freshPolicy.explicitNoSavePersistencePolicyFinalized);
    CHECK(!freshPolicy.durablePolicyKnown);
    CHECK(!freshPolicy.durableCommitted);
    CHECK(std::strcmp(freshPolicy.missingOwner,
                      "Scene8-final-save-direct-durable-commit-primitive") == 0);

    PrStage1SaveUiCardImageDurableCommitPrimitive8007A318 directCommit =
        PrStage1SaveUiDirect::
            CommitSaveUiCardImageDirectDurablePrimitive8007A318();
    CHECK(directCommit.attempted);
    CHECK(directCommit.persistenceSinkKnown);
    CHECK(directCommit.slotPolicyKnown);
    CHECK(directCommit.blockIndex == 0);
    CHECK(directCommit.directBackendKnown);
    CHECK(directCommit.directBackendCalled);
    CHECK(directCommit.directBackendAccepted);
    CHECK(!directCommit.explicitNoDurablePolicyKnown);
    CHECK(!directCommit.explicitNoDurablePolicy);
    CHECK(!directCommit.explicitNoSavePersistencePolicyKnown);
    CHECK(!directCommit.explicitNoSavePersistencePolicyAccepted);
    CHECK(!directCommit.explicitNoSavePersistencePolicyFinalized);
    CHECK(directCommit.durablePolicyKnown);
    CHECK(directCommit.durableCommitted);
    CHECK(directCommit.missingOwner == nullptr);
    CHECK(g_directDurableBackendCalled);
    CHECK(g_directDurableBackendBlockIndex == 0);
    CHECK(g_directDurableBackendBytes == sinkView.bytes);
    CHECK(g_directDurableBackendByteCount == 128u * 1024u);
    CHECK(g_directDurableBackendUser == &backendMarker);

    PrStage1SaveUiCardImagePersistencePolicy8007A318 committedPolicy =
        PrStage1SaveUiDirect::EvaluateSaveUiCardImagePersistencePolicy8007A318();
    CHECK(committedPolicy.directDurableCommitBackendKnown);
    CHECK(committedPolicy.durablePolicyKnown);
    CHECK(committedPolicy.durableCommitted);
    CHECK(committedPolicy.missingOwner == nullptr);
    PrStage1SaveUiDirect::SetSaveUiCardImageDirectDurableCommitBackend8007A318(
        nullptr,
        nullptr);
}

void TestDirectDurableBackendRegistrationMustBeExplicitAndClearable() {
    PrStage1SaveUiDirect::SetSaveUiCardImageDirectDurableCommitBackend8007A318(
        nullptr,
        nullptr);
    PrGameContext ctx{};
    PrStage1SaveUi19148TickResult state15 =
        AdvanceToState15WriteAttempt(ctx);
    CHECK(state15.state15CopyTo8007ADE8);

    PrStage1SaveUiDirectoryRawBankUpdate8007A318 update =
        PrStage1SaveUiDirect::
            ApplySaveUiDirectoryRawBankSerializedEntry8007A318(
                "BASCUS-94183Z",
                8192u);
    CHECK(update.updated);
    PrStage1SaveUiCardImageSerialization8007A318 image =
        PrStage1SaveUiDirect::
            SerializeSaveUiCardImageCandidateFromDirectBuffers8007A318();
    CHECK(image.imageSerialized);
    PrStage1SaveUiCardImagePersistenceSink8007A318 sink =
        PrStage1SaveUiDirect::CommitSaveUiCardImagePersistenceSink8007A318();
    CHECK(sink.sinkCommitted);

    PrStage1SaveUiCardImageDurableCommitPrimitive8007A318 defaultCommit =
        PrStage1SaveUiDirect::
            CommitSaveUiCardImageDirectDurablePrimitive8007A318();
    CHECK(defaultCommit.explicitNoSavePersistencePolicyFinalized);
    CHECK(!defaultCommit.directBackendKnown);
    CHECK(!defaultCommit.directBackendCalled);
    CHECK(!defaultCommit.durableCommitted);

    int backendMarker = 11;
    g_directDurableBackendCalled = false;
    PrStage1SaveUiDirect::SetSaveUiCardImageDirectDurableCommitBackend8007A318(
        &TestDirectDurableBackend8007A318,
        &backendMarker);
    PrStage1SaveUiCardImageDurableCommitPrimitive8007A318 registeredCommit =
        PrStage1SaveUiDirect::
            CommitSaveUiCardImageDirectDurablePrimitive8007A318();
    CHECK(registeredCommit.directBackendKnown);
    CHECK(!registeredCommit.directBackendCalled);
    CHECK(!registeredCommit.directBackendAccepted);
    CHECK(registeredCommit.explicitNoSavePersistencePolicyFinalized);
    CHECK(!registeredCommit.durableCommitted);
    CHECK(!g_directDurableBackendCalled);

    PrStage1SaveUiCardImagePersistenceSink8007A318 freshSink =
        PrStage1SaveUiDirect::CommitSaveUiCardImagePersistenceSink8007A318();
    CHECK(freshSink.sinkCommitted);
    PrStage1SaveUiCardImageDurableCommitPrimitive8007A318 freshCommit =
        PrStage1SaveUiDirect::
            CommitSaveUiCardImageDirectDurablePrimitive8007A318();
    CHECK(freshCommit.directBackendKnown);
    CHECK(freshCommit.directBackendCalled);
    CHECK(freshCommit.directBackendAccepted);
    CHECK(freshCommit.durableCommitted);
    CHECK(g_directDurableBackendCalled);
    CHECK(g_directDurableBackendUser == &backendMarker);

    PrStage1SaveUiDirect::SetSaveUiCardImageDirectDurableCommitBackend8007A318(
        nullptr,
        nullptr);
    g_directDurableBackendCalled = false;
    PrStage1SaveUiCardImageDurableCommitPrimitive8007A318 clearedCommit =
        PrStage1SaveUiDirect::
            CommitSaveUiCardImageDirectDurablePrimitive8007A318();
    CHECK(!clearedCommit.explicitNoSavePersistencePolicyFinalized);
    CHECK(!clearedCommit.directBackendKnown);
    CHECK(!clearedCommit.directBackendCalled);
    CHECK(!clearedCommit.directBackendAccepted);
    CHECK(clearedCommit.durableCommitted);
    CHECK(!g_directDurableBackendCalled);
}

void TestDirectFormatPreparesBlankCardImageCandidate() {
    PrGameContext ctx{};
    PrStage1SaveUi19148TickResult state15 =
        AdvanceToState15WriteAttempt(ctx);
    CHECK(state15.state15CopyTo8007ADE8);

    PrStage1SaveUiDirectoryRawBankUpdate8007A318 update =
        PrStage1SaveUiDirect::
            ApplySaveUiDirectoryRawBankSerializedEntry8007A318(
                "BASCUS-94183Z",
                8192u);
    CHECK(update.updated);
    PrStage1SaveUiDirectoryRawBankSnapshot8007A318 before =
        PrStage1SaveUiDirect::GetSaveUiDirectoryRawBankSnapshot8007A318();
    CHECK(before.known);
    CHECK(before.anyNonZero);
    CHECK(before.anySavePrefix);

    PrStage1SaveUiCardImageSerialization8007A318 image =
        PrStage1SaveUiDirect::
            SerializeSaveUiCardImageCandidateFromDirectBuffers8007A318();
    CHECK(image.imageSerialized);
    PrStage1SaveUiCardImageView8007A318 candidateBefore =
        PrStage1SaveUiDirect::GetSaveUiCardImageCandidateView8007A318();
    CHECK(candidateBefore.known);
    PrStage1SaveUiCardImagePersistenceSink8007A318 sink =
        PrStage1SaveUiDirect::CommitSaveUiCardImagePersistenceSink8007A318();
    CHECK(sink.sinkCommitted);
    PrStage1SaveUiCardImagePersistenceView8007A318 sinkBefore =
        PrStage1SaveUiDirect::GetSaveUiCardImagePersistenceSinkView8007A318();
    CHECK(sinkBefore.known);

    PrStage1SaveUiDirectFormatResult80017B60 format =
        PrStage1SaveUiDirect::FormatSaveUiDirectCardImage80017B60();
    CHECK(format.attempted);
    CHECK(format.directoryKnown);
    CHECK(format.formatted);
    CHECK(!format.cardImageCandidateCleared);
    CHECK(format.cardImageCandidateKnown);
    CHECK(format.pendingPersistenceCleared);
    CHECK(format.slotPolicyKnown);
    CHECK(format.blockIndex == sink.blockIndex);
    CHECK(!format.durablePolicyKnown);
    CHECK(!format.durableCommitted);
    CHECK(format.directoryPsxAddress == 0x8007A318u);
    CHECK(format.directoryByteSize == 600u);

    PrStage1SaveUiDirectoryRawBankSnapshot8007A318 after =
        PrStage1SaveUiDirect::GetSaveUiDirectoryRawBankSnapshot8007A318();
    CHECK(after.known);
    CHECK(!after.anyNonZero);
    CHECK(!after.anySavePrefix);
    CHECK(after.nonZeroRows == 0);
    CHECK(after.savePrefixRows == 0);
    PrStage1SaveUiCardImageView8007A318 candidateAfter =
        PrStage1SaveUiDirect::GetSaveUiCardImageCandidateView8007A318();
    CHECK(candidateAfter.known);
    CHECK(candidateAfter.slotPolicyKnown);
    CHECK(candidateAfter.blockIndex == sink.blockIndex);
    CHECK(candidateAfter.bytes != nullptr);
    CHECK(candidateAfter.byteCount == 128u * 1024u);
    CHECK(candidateAfter.bytes[0] == static_cast<uint8_t>('M'));
    CHECK(candidateAfter.bytes[1] == static_cast<uint8_t>('C'));
    CHECK(!candidateAfter.durablePolicyKnown);
    CHECK(!candidateAfter.durableCommitted);
    PrStage1SaveUiCardImagePersistenceView8007A318 sinkAfter =
        PrStage1SaveUiDirect::GetSaveUiCardImagePersistenceSinkView8007A318();
    CHECK(!sinkAfter.known);
    CHECK(!sinkAfter.durablePolicyKnown);
    CHECK(!sinkAfter.durableCommitted);

    PrStage1SaveUiCardImagePersistencePolicy8007A318 policy =
        PrStage1SaveUiDirect::EvaluateSaveUiCardImagePersistencePolicy8007A318();
    CHECK(policy.attempted);
    CHECK(policy.cardImageCandidateKnown);
    CHECK(!policy.durablePolicyKnown);
    CHECK(!policy.durableCommitted);
    CHECK(std::strcmp(policy.missingOwner,
                      "Scene8-final-save-card-image-persistence-sink") == 0);

    int backendMarker = 17;
    g_directDurableBackendCalled = false;
    PrStage1SaveUiDirect::SetSaveUiCardImageDirectDurableCommitBackend8007A318(
        &TestDirectDurableBackend8007A318,
        &backendMarker);
    const PrStage1SaveUiCardImagePersistenceSink8007A318 blankSink =
        PrStage1SaveUiDirect::CommitSaveUiCardImagePersistenceSink8007A318();
    CHECK(blankSink.sinkCommitted);
    CHECK(blankSink.blockIndex == format.blockIndex);
    const PrStage1SaveUiCardImageDurableCommitPrimitive8007A318 blankCommit =
        PrStage1SaveUiDirect::
            CommitSaveUiCardImageDirectDurablePrimitive8007A318();
    CHECK(blankCommit.directBackendKnown);
    CHECK(blankCommit.directBackendCalled);
    CHECK(blankCommit.directBackendAccepted);
    CHECK(blankCommit.durableCommitted);
    CHECK(g_directDurableBackendCalled);
    CHECK(g_directDurableBackendBlockIndex == format.blockIndex);
    CHECK(g_directDurableBackendByteCount == 128u * 1024u);
    CHECK(g_directDurableBackendBytes[0] == static_cast<uint8_t>('M'));
    CHECK(g_directDurableBackendBytes[1] == static_cast<uint8_t>('C'));
    CHECK(g_directDurableBackendUser == &backendMarker);
    PrStage1SaveUiDirect::SetSaveUiCardImageDirectDurableCommitBackend8007A318(
        nullptr,
        nullptr);
}

void TestDirectCardLoadValidatesAndAtomicallyImportsDirectory80017594() {
    PrStage1SaveUiDirect::Reset19148();
    const PrStage1SaveUiDirectFormatResult80017B60 format =
        PrStage1SaveUiDirect::FormatSaveUiDirectCardImage80017B60();
    CHECK(format.formatted);
    const PrStage1SaveUiCardImageView8007A318 formatted =
        PrStage1SaveUiDirect::GetSaveUiCardImageCandidateView8007A318();
    CHECK(formatted.known);
    CHECK(formatted.bytes != nullptr);
    CHECK(formatted.byteCount == 128u * 1024u);
    if (!formatted.known || formatted.bytes == nullptr ||
        formatted.byteCount != 128u * 1024u) {
        return;
    }

    std::array<uint8_t, 128u * 1024u> image{};
    std::memcpy(image.data(), formatted.bytes, image.size());
    uint8_t* entry = image.data() + 128u;
    entry[0] = 0x51u;
    entry[4] = 0x00u;
    entry[5] = 0x20u;
    std::memcpy(entry + 0x0Au, "BASCUS-94183Q", 13u);
    entry[0x7Fu] = FrameChecksum(entry);

    const PrStage1SaveUiCardImageDurableReadIngress8007A318 imported =
        PrStage1SaveUiDirect::
            ImportSaveUiCardImagePersistenceSinkFromDirectDurableRead8007A318(
                image.data(), image.size(), 0);
    CHECK(imported.sinkCommitted);
    CHECK(imported.durableCommitted);

    const PrStage1SaveUiDirectCardLoadResult80017594 loaded =
        PrStage1SaveUiDirect::LoadSaveUiDirectCardImageDirectory80017594();
    CHECK(loaded.attempted);
    CHECK(loaded.persistenceKnown);
    CHECK(loaded.durableReadKnown);
    CHECK(loaded.imageHeaderKnown);
    CHECK(loaded.directoryFramesKnown);
    CHECK(loaded.directoryLoaded);
    CHECK(loaded.activeRows == 1);
    CHECK(loaded.blockIndex == 0);
    const PrStage1SaveUiDirectoryRawBankView8007A318 loadedDirectory =
        PrStage1SaveUiDirect::GetSaveUiDirectoryRawBankView8007A318();
    CHECK(loadedDirectory.known);
    CHECK(loadedDirectory.bytes != nullptr);
    CHECK(std::memcmp(loadedDirectory.bytes, "BASCUS-94183Q", 13u) == 0);
    CHECK(loadedDirectory.bytes[24] == 0x00u);
    CHECK(loadedDirectory.bytes[25] == 0x20u);

    const PrStage1SaveUiDirectoryRawBankUpdate8007A318 sentinel =
        PrStage1SaveUiDirect::
            ApplySaveUiDirectoryRawBankSerializedEntry8007A318(
                "BASCUS-94183S", 8192u);
    CHECK(sentinel.updated);
    const PrStage1SaveUiDirectoryRawBankSnapshot8007A318 beforeBadLoad =
        PrStage1SaveUiDirect::GetSaveUiDirectoryRawBankSnapshot8007A318();
    CHECK(beforeBadLoad.known);
    const PrStage1SaveUiDirectoryRawBankView8007A318 beforeBadLoadView =
        PrStage1SaveUiDirect::GetSaveUiDirectoryRawBankView8007A318();
    CHECK(beforeBadLoadView.known);
    CHECK(beforeBadLoadView.bytes != nullptr);
    CHECK(beforeBadLoadView.byteCount == 600u);
    std::array<uint8_t, 600u> directoryBeforeBadLoad{};
    if (beforeBadLoadView.known && beforeBadLoadView.bytes != nullptr &&
        beforeBadLoadView.byteCount == directoryBeforeBadLoad.size()) {
        std::memcpy(directoryBeforeBadLoad.data(),
                    beforeBadLoadView.bytes,
                    directoryBeforeBadLoad.size());
    }

    image[2u * 128u + 1u] ^= 0x01u;
    const PrStage1SaveUiCardImageDurableReadIngress8007A318 corrupted =
        PrStage1SaveUiDirect::
            ImportSaveUiCardImagePersistenceSinkFromDirectDurableRead8007A318(
                image.data(), image.size(), 0);
    CHECK(corrupted.sinkCommitted);
    const PrStage1SaveUiDirectCardLoadResult80017594 rejected =
        PrStage1SaveUiDirect::LoadSaveUiDirectCardImageDirectory80017594();
    CHECK(rejected.attempted);
    CHECK(rejected.persistenceKnown);
    CHECK(rejected.durableReadKnown);
    CHECK(rejected.imageHeaderKnown);
    CHECK(!rejected.directoryFramesKnown);
    CHECK(!rejected.directoryLoaded);

    const PrStage1SaveUiDirectoryRawBankSnapshot8007A318 afterBadLoad =
        PrStage1SaveUiDirect::GetSaveUiDirectoryRawBankSnapshot8007A318();
    CHECK(afterBadLoad.known);
    const PrStage1SaveUiDirectoryRawBankView8007A318 afterBadLoadView =
        PrStage1SaveUiDirect::GetSaveUiDirectoryRawBankView8007A318();
    CHECK(afterBadLoadView.known);
    CHECK(afterBadLoadView.bytes != nullptr);
    CHECK(afterBadLoadView.byteCount == directoryBeforeBadLoad.size());
    CHECK(std::memcmp(directoryBeforeBadLoad.data(),
                      afterBadLoadView.bytes,
                      directoryBeforeBadLoad.size()) == 0);
}

void TestDirectDurableReadIngressPublishesExactCardImage() {
    PrStage1SaveUiDirect::Reset19148();
    std::array<uint8_t, 128u * 1024u> image{};
    image[0] = static_cast<uint8_t>('M');
    image[1] = static_cast<uint8_t>('C');
    image[0x1234u] = 0x5Au;

    const PrStage1SaveUiCardImageDurableReadIngress8007A318 imported =
        PrStage1SaveUiDirect::
            ImportSaveUiCardImagePersistenceSinkFromDirectDurableRead8007A318(
                image.data(), image.size(), 0);
    CHECK(imported.attempted);
    CHECK(imported.bytesKnown);
    CHECK(imported.slotPolicyKnown);
    CHECK(imported.blockIndex == 0);
    CHECK(imported.durablePolicyKnown);
    CHECK(imported.durableCommitted);
    CHECK(imported.sinkCommitted);

    const PrStage1SaveUiCardImagePersistenceView8007A318 view =
        PrStage1SaveUiDirect::GetSaveUiCardImagePersistenceSinkView8007A318();
    CHECK(view.known);
    CHECK(view.slotPolicyKnown);
    CHECK(view.blockIndex == 0);
    CHECK(view.durablePolicyKnown);
    CHECK(view.durableCommitted);
    CHECK(view.bytes != nullptr);
    CHECK(view.byteCount == image.size());
    CHECK(view.byteSize == image.size());
    CHECK(view.bytes[0] == static_cast<uint8_t>('M'));
    CHECK(view.bytes[1] == static_cast<uint8_t>('C'));
    CHECK(view.bytes[0x1234u] == 0x5Au);

    const PrStage1SaveUiCardImageDurableReadIngress8007A318 rejected =
        PrStage1SaveUiDirect::
            ImportSaveUiCardImagePersistenceSinkFromDirectDurableRead8007A318(
                image.data(), image.size() - 1u, 0);
    CHECK(rejected.attempted);
    CHECK(!rejected.bytesKnown);
    CHECK(rejected.slotPolicyKnown);
    CHECK(!rejected.durablePolicyKnown);
    CHECK(!rejected.durableCommitted);
    CHECK(!rejected.sinkCommitted);

    const PrStage1SaveUiCardImagePersistenceView8007A318 preserved =
        PrStage1SaveUiDirect::GetSaveUiCardImagePersistenceSinkView8007A318();
    CHECK(preserved.known);
    CHECK(preserved.bytes != nullptr);
    CHECK(preserved.bytes[0x1234u] == 0x5Au);

    const PrStage1SaveUiCardImageWriteRollback8007A318 restored =
        PrStage1SaveUiDirect::RollbackSaveUiCardImageAfterFailedWrite8007A318(
            image.data(), image.size(), 0, true);
    CHECK(restored.attempted);
    CHECK(restored.previousImageKnown);
    CHECK(restored.candidateCleared);
    CHECK(restored.pendingPersistenceCleared);
    CHECK(restored.previousImageRestored);
    CHECK(restored.blockIndex == 0);
    const PrStage1SaveUiCardImagePersistenceView8007A318 restoredView =
        PrStage1SaveUiDirect::GetSaveUiCardImagePersistenceSinkView8007A318();
    CHECK(restoredView.known);
    CHECK(restoredView.durableCommitted);
    CHECK(restoredView.bytes[0x1234u] == 0x5Au);

    const PrStage1SaveUiCardImageWriteRollback8007A318 cleared =
        PrStage1SaveUiDirect::RollbackSaveUiCardImageAfterFailedWrite8007A318(
            nullptr, 0u, -1, false);
    CHECK(cleared.attempted);
    CHECK(!cleared.previousImageKnown);
    CHECK(cleared.candidateCleared);
    CHECK(cleared.pendingPersistenceCleared);
    CHECK(!cleared.previousImageRestored);
    const PrStage1SaveUiCardImagePersistenceView8007A318 clearedView =
        PrStage1SaveUiDirect::GetSaveUiCardImagePersistenceSinkView8007A318();
    CHECK(!clearedView.known);
}

PrStage1SaveCardHalDirect::CardWriteHostAttemptFacts80017A10
MakeDirectWriteAttemptBeforeCompletion80017A10() {
    PrStage1SaveCardHalDirect::CardWriteHostAttemptFacts80017A10 attempt{};
    attempt.scanResultKnown80017900 = true;
    attempt.scanResult80017900 = 1;
    attempt.openWriteKnown80017454 = true;
    attempt.openWriteFdKnown80017454 = true;
    attempt.openWriteFd80017454 = 2;
    attempt.openWriteReturnKnown80017454 = true;
    attempt.openWriteReturn80017454 = 2;
    attempt.gp696FdWriteKnown80017454 = true;
    attempt.gp696Fd80017454 = 2;
    attempt.clearSwEventsKnown80016FC0 = true;
    attempt.writeKnown80017454 = true;
    attempt.writeByteCountKnown80017454 = true;
    attempt.writeByteCount80017454 = 0x2000;
    attempt.submitReturnKnown80017454 = true;
    attempt.submitReturn80017454 = 0;
    attempt.waitCallKnown80035560 = true;
    attempt.waitArg80035560 = 4;
    return attempt;
}

void TestDirectWriteCompletionPublishesPollOnlyAfterWait() {
    PrStage1SaveCardHalDirect::CardWriteHostAttemptFacts80017A10 accepted =
        MakeDirectWriteAttemptBeforeCompletion80017A10();
    PrStage1SaveCardHalDirect::SaveUiDirectWriteCompletion80017A10
        completion{};
    completion.backendCompletionKnown = true;
    completion.backendAccepted = true;
    completion.virtualFd80017454 = 2;
    CHECK(!PrStage1SaveCardHalDirect::
               FinalizeSaveUiDirectWriteAttempt80017A10(
                   completion, &accepted));
    CHECK(!accepted.writeReturnKnown80017454);
    CHECK(!accepted.pollResultKnown80016EB8);
    CHECK(!accepted.closeResultKnown);

    completion.waitCompleted80035560 = true;
    CHECK(PrStage1SaveCardHalDirect::
              FinalizeSaveUiDirectWriteAttempt80017A10(
                  completion, &accepted));
    CHECK(accepted.writeReturnKnown80017454);
    CHECK(accepted.writeReturn80017454 == 0x2000);
    CHECK(accepted.pollResultKnown80016EB8);
    CHECK(accepted.pollResult80016EB8 == 1);
    CHECK(accepted.closeResultKnown);
    CHECK(accepted.closeResult == 0);
    CHECK(accepted.closeFdKnown);
    CHECK(accepted.closeFd == 2);
    CHECK(accepted.gp696FdCloseKnown80017A10);
    CHECK(accepted.gp696FdClose80017A10 == 2);

    PrStage1SaveCardHalDirect::CardWriteHostAttemptFacts80017A10 rejected =
        MakeDirectWriteAttemptBeforeCompletion80017A10();
    completion.backendAccepted = false;
    CHECK(PrStage1SaveCardHalDirect::
              FinalizeSaveUiDirectWriteAttempt80017A10(
                  completion, &rejected));
    CHECK(rejected.writeReturnKnown80017454);
    CHECK(rejected.writeReturn80017454 == -1);
    CHECK(rejected.pollResultKnown80016EB8);
    CHECK(rejected.pollResult80016EB8 == 2);
    CHECK(rejected.closeFdKnown);
    CHECK(rejected.closeFd == 2);
}

void TestDirectoryNameScanAndFailedWriteRestore80017900() {
    PrStage1SaveUiDirect::Reset19148();
    constexpr char kName[] = "BASCUS-94183Z";
    const PrStage1SaveUiDirectoryRawBankView8007A318 before =
        PrStage1SaveUiDirect::GetSaveUiDirectoryRawBankView8007A318();
    CHECK(before.known);
    CHECK(before.bytes != nullptr);
    CHECK(before.byteCount == 600u);
    std::array<uint8_t, 600> original{};
    std::memcpy(original.data(), before.bytes, original.size());

    PrStage1SaveUiDirectoryNameScan80017900 scan =
        PrStage1SaveUiDirect::ScanSaveUiDirectoryRawBankName80017900(kName);
    CHECK(scan.attempted);
    CHECK(scan.directoryKnown);
    CHECK(scan.nameKnown);
    CHECK(!scan.found);
    CHECK(scan.rowIndex == -1);
    CHECK(scan.psxReturn == 0);

    const PrStage1SaveUiDirectoryRawBankUpdate8007A318 update =
        PrStage1SaveUiDirect::
            ApplySaveUiDirectoryRawBankSerializedEntry8007A318(kName, 8192u);
    CHECK(update.updated);
    scan = PrStage1SaveUiDirect::
        ScanSaveUiDirectoryRawBankName80017900(kName);
    CHECK(scan.found);
    CHECK(scan.rowIndex == update.blockIndex);
    CHECK(scan.psxReturn == 1);

    const PrStage1SaveUiDirectoryRawBankRestore8007A318 restored =
        PrStage1SaveUiDirect::
            RestoreSaveUiDirectoryRawBankAfterFailedWrite8007A318(
                original.data(), original.size());
    CHECK(restored.attempted);
    CHECK(restored.sourceKnown);
    CHECK(restored.restored);
    scan = PrStage1SaveUiDirect::
        ScanSaveUiDirectoryRawBankName80017900(kName);
    CHECK(!scan.found);
    CHECK(scan.psxReturn == 0);
}

void TestRawDirectoryProducerRequiresMatchingRequest() {
    const std::array<uint8_t, 600> bank = MakeRawDirectoryBankWithOneSave();
    PrGameContext ctx{};
    AdvanceToState6(ctx);
    PrStage1SaveUi19148TickResult blocked = PumpCardIoUntilResult(ctx, 1);
    const PrStage1SaveUi19148LowerFeedbackRequest request =
        FindDirectoryRowsRequest(blocked);

    PrStage1SaveCardHalDirect::
        SaveUiDirectoryRawBankProducerInput80019458 input{};
    input.requestKnown = true;
    input.request = request;
    input.rawBankKnown8007A318 = true;
    input.rawBank8007A318 = bank.data();
    input.rawBankByteCount8007A318 = bank.size();

    PrStage1SaveCardHalDirect::
        SaveUiDirectoryRawBankProducerResult80019458 result{};
    PrStage1SaveCardHalDirect::
        BuildSaveUiDirectoryScanFactsFromRawBankProducerInput80019458(
            input,
            &result);
    CHECK(result.produced);
    CHECK(!result.incomplete);
    CHECK(result.requestUsed);
    CHECK(result.requestMatched);
    CHECK(result.directoryRowsKnown80017B08);
    CHECK(result.snapshotKnown80017B18);
    CHECK(result.rawBankKnown8007A318);
    CHECK(result.facts.factsKnown);
    CHECK(result.facts.entryCount == 15);

    input.request.psxFunction = 0x80017B08u;
    result = {};
    PrStage1SaveCardHalDirect::
        BuildSaveUiDirectoryScanFactsFromRawBankProducerInput80019458(
            input,
            &result);
    CHECK(!result.produced);
    CHECK(result.incomplete);
    CHECK(result.requestUsed);
    CHECK(!result.requestMatched);
    CHECK(!result.facts.factsKnown);
}

} // namespace

int main() {
    TestState4NewAtLoadReachesFormatPromptThenFormatGap();
    TestFormatRuntimeCarrierRequiresExactRequest();
    TestFormatRuntimeCarrierBindsTerminalBranchesToExactRequest();
    TestDirectoryRowsMissingFeedbackFailsClosed();
    TestDirectoryRowsRejectHostlessUnknownSource();
    TestDirectoryRowsNoEligibleRowsAdvancesToState7();
    TestDirectoryRowsValidEntryAdvancesToState9();
    TestListInputEmptyDirectorySelectsFirstFreeRow800181D0();
    TestListInputFullDirectoryFallsBackToTerminal800181D0();
    TestListInputSparseNavigationAndExistingFilename800181D0();
    TestScanFactsBuildDirectoryRowsFeedback();
    TestScanFactsRejectMissingDirectorySource();
    TestScanFactsRejectBadRowAndCount();
    TestScanFactsFeedbackAdvancesState6();
    TestRuntimeDirectoryCarrierFailsClosedWhenIncomplete();
    TestRuntimeDirectoryCarrierFeedsState6();
    TestState10NameEntryCursorPersistsToConfirm();
    TestRawDirectoryBankRejectsMissingSnapshot();
    TestRawDirectoryBankBuildsCarrierAndFeedsState6();
    TestEmptyRawDirectoryBankBuildsFreeRowsAndAdvancesState6ToState9();
    TestSerializedEntryUpdatesDirectDirectoryRawBank();
    TestSerializedEntryDirectoryUpdateFailsClosed();
    TestSerializedEntryBuildsDirectCardImageCandidate();
    TestDirectCardImageCandidateFailsClosedWithoutDirectorySlot();
    TestDirectCardImagePersistencePolicyRequiresDurableApi();
    TestDirectDurableBackendRegistrationMustBeExplicitAndClearable();
    TestDirectFormatPreparesBlankCardImageCandidate();
    TestDirectCardLoadValidatesAndAtomicallyImportsDirectory80017594();
    TestDirectDurableReadIngressPublishesExactCardImage();
    TestDirectWriteCompletionPublishesPollOnlyAfterWait();
    TestDirectoryNameScanAndFailedWriteRestore80017900();
    TestRawDirectoryProducerRequiresMatchingRequest();

    if (g_failedChecks != 0) {
        std::printf("test_ss0_save_ui19148_directory_rows_contract: %d failures\n",
                    g_failedChecks);
        return 1;
    }
    std::printf("test_ss0_save_ui19148_directory_rows_contract: ok\n");
    return 0;
}
