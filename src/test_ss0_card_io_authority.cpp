#include "pr/pr_stage1_save_card_hal_direct.h"
#include "pr/pr_stage1_save_ui_direct.h"
#include "pr/pr_ss0_card_image_storage_direct.h"
#include "pr/pr_pad.h"

#include <windows.h>

#include <array>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <vector>

using namespace PrStage1SaveCardHalDirect;

PrPadState PrPad::GetState(int) {
    return PrPadState{};
}

namespace {

int g_failed = 0;

constexpr std::size_t kCardImageBytes8007A318 = 128u * 1024u;

#define CHECK(expr)                                                           \
    do {                                                                      \
        if (!(expr)) {                                                        \
            std::printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr);       \
            ++g_failed;                                                       \
        }                                                                     \
    } while (0)

std::filesystem::path TestExecutableDirectory() {
    wchar_t path[MAX_PATH]{};
    const DWORD length = GetModuleFileNameW(nullptr, path, MAX_PATH);
    CHECK(length != 0u && length < MAX_PATH);
    return std::filesystem::path(path).parent_path();
}

std::vector<uint8_t> ReadWholeFile(const std::filesystem::path& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        return {};
    }
    return std::vector<uint8_t>(std::istreambuf_iterator<char>(file),
                                std::istreambuf_iterator<char>());
}

bool WriteWholeFile(const std::filesystem::path& path,
                    const std::vector<uint8_t>& bytes) {
    std::error_code error;
    std::filesystem::create_directories(path.parent_path(), error);
    if (error) {
        return false;
    }
    std::ofstream file(path, std::ios::binary | std::ios::trunc);
    if (!file.is_open()) {
        return false;
    }
    file.write(reinterpret_cast<const char*>(bytes.data()),
               static_cast<std::streamsize>(bytes.size()));
    file.flush();
    return static_cast<bool>(file);
}

class ScopedCardFilesRestore {
public:
    ScopedCardFilesRestore() {
        const std::filesystem::path save = TestExecutableDirectory() / L"save";
        paths_ = {{save / L"bu00.mcr", save / L"bu01.mcr",
                   save / L"bu10.mcr"}};
        for (std::size_t i = 0; i < paths_.size(); ++i) {
            std::error_code error;
            existed_[i] = std::filesystem::exists(paths_[i], error);
            if (existed_[i]) {
                bytes_[i] = ReadWholeFile(paths_[i]);
            }
            error.clear();
            std::filesystem::remove(paths_[i], error);
        }
    }

    ~ScopedCardFilesRestore() {
        for (std::size_t i = 0; i < paths_.size(); ++i) {
            std::error_code error;
            if (existed_[i]) {
                CHECK(WriteWholeFile(paths_[i], bytes_[i]));
            } else {
                std::filesystem::remove(paths_[i], error);
            }
        }
    }

    const std::filesystem::path& primary() const { return paths_[0]; }

private:
    std::array<std::filesystem::path, 3> paths_{};
    std::array<bool, 3> existed_{};
    std::array<std::vector<uint8_t>, 3> bytes_{};
};

PrStage1SaveUi19148LowerFeedbackRequest MakeCardIoRequest(int32_t state) {
    PrStage1SaveUi19148LowerFeedbackRequest request{};
    request.kind = PrStage1SaveUi19148LowerFeedbackRequestKind::CardIo80017594;
    request.psxFunction = 0x80017594u;
    request.cardIoState.dword800917E8 = state;
    request.cardIoState.gp700 = 300;
    return request;
}

void CheckNoTypedSwPollAuthority(const CardIoHostFacts80017594& facts) {
    CHECK(!facts.pollSwKnown80016E18);
    CHECK(facts.pollSwResult80016E18 == 0);
    CHECK(!facts.pollSwGp700BeforeKnown80016E18);
    CHECK(!facts.pollSwGp700AfterKnown80016E18);
    CHECK(!facts.pollSwTimedOutKnown80016E18);
    CHECK(!facts.pollSwTimedOut80016E18);
}

void TestRejectsNonCardIoRequest() {
    PrStage1SaveUi19148LowerFeedbackRequest request{};
    request.kind = PrStage1SaveUi19148LowerFeedbackRequestKind::Write80017A10;
    request.psxFunction = 0x80017594u;
    CardIoHostFacts80017594 facts{};

    CHECK(!BuildSaveUiCardIoObservedNormalPathFacts80017594(request, &facts));
    CHECK(!facts.factsKnown);
    CHECK(!facts.stateBeforeKnown);
    CHECK(!facts.stateAfterKnown);
    CheckNoTypedSwPollAuthority(facts);
}

void TestState0CarriesInfoSubmitShapeOnly() {
    CardIoHostFacts80017594 facts{};
    CHECK(BuildSaveUiCardIoObservedNormalPathFacts80017594(
        MakeCardIoRequest(0), &facts));
    CHECK(facts.factsKnown);
    CHECK(facts.stateBeforeKnown);
    CHECK(facts.stateAfterKnown);
    CHECK(facts.cardInfoKnown);
    CHECK(facts.cardInfoArgKnown);
    CHECK(facts.cardInfoArg == 0);
    CHECK(facts.stateAfter.dword800917E8 == 1);
    CHECK(facts.stateAfter.dword800917EC == 0);
    CHECK(facts.stateAfter.gp700 == 300);
    CheckNoTypedSwPollAuthority(facts);
}

void TestState1RequiresExplicitTypedPollFeedback() {
    CardIoHostFacts80017594 facts{};
    PrStage1SaveUi19148LowerFeedbackRequest request = MakeCardIoRequest(1);
    request.cardIoState.dword800917F4 = 0;
    request.cardIoState.gp700 = 300;

    CHECK(!BuildSaveUiCardIoObservedNormalPathFacts80017594(request, &facts));
    CHECK(!facts.factsKnown);
    CHECK(facts.stateBeforeKnown);
    CHECK(!facts.stateAfterKnown);
    CHECK(!facts.cardInfoKnown);
    CHECK(!facts.cardLoadKnown);
    CheckNoTypedSwPollAuthority(facts);
}

void TestExplicitState1TypedPollSuccessFeedback() {
    CardIoHostFacts80017594 facts{};
    facts.factsKnown = true;
    facts.stateBeforeKnown = true;
    facts.stateBefore.dword800917E8 = 1;
    facts.stateBefore.gp700 = 300;
    facts.stateAfterKnown = true;
    facts.stateAfter = facts.stateBefore;
    facts.stateAfter.dword800917E8 = 2;
    facts.stateAfter.dword800917F0 = 1;
    facts.stateAfter.gp700 = 299;
    facts.pollSwKnown80016E18 = true;
    facts.pollSwResult80016E18 = 1;
    facts.pollSwGp700BeforeKnown80016E18 = true;
    facts.pollSwGp700Before80016E18 = 300;
    facts.pollSwGp700AfterKnown80016E18 = true;
    facts.pollSwGp700After80016E18 = 299;
    facts.pollSwTimedOutKnown80016E18 = true;
    facts.pollSwTimedOut80016E18 = false;

    CardIoLowerFeedbackBuildResult80017594 build{};
    BuildSaveUiCardIoLowerFeedbackFromHostFacts80017594(facts, &build);

    CHECK(build.lowerFeedbackKnown);
    CHECK(build.lowerFeedback.cardIoFeedbackKnown80017594);
    const PrStage1SaveUiCardIoFeedback80017594& feedback =
        build.lowerFeedback.cardIoFeedback80017594;
    CHECK(feedback.stateBeforeKnown);
    CHECK(feedback.stateAfterKnown);
    CHECK(feedback.stateBefore.dword800917E8 == 1);
    CHECK(feedback.stateAfter.dword800917E8 == 2);
    CHECK(feedback.stateAfter.dword800917F0 == 1);
    CHECK(feedback.stateAfter.gp700 == 299);
    CHECK(feedback.pollSwKnown80016E18);
    CHECK(feedback.pollSwResult80016E18 == 1);
    CHECK(feedback.pollSwGp700BeforeKnown80016E18);
    CHECK(feedback.pollSwGp700Before80016E18 == 300);
    CHECK(feedback.pollSwGp700AfterKnown80016E18);
    CHECK(feedback.pollSwGp700After80016E18 == 299);
    CHECK(feedback.pollSwTimedOutKnown80016E18);
    CHECK(!feedback.pollSwTimedOut80016E18);
}

void TestCurrentIdaPollResultFacts80016E18() {
    CardIoHostFacts80017594 facts{};

    PrStage1SaveUi19148LowerFeedbackRequest state1 = MakeCardIoRequest(1);
    state1.cardIoState.gp700 = 300;
    CHECK(BuildSaveUiCardIoPollFactsFromResult80016E18(state1, 1, &facts));
    CHECK(facts.factsKnown);
    CHECK(facts.pollSwKnown80016E18);
    CHECK(facts.pollSwResult80016E18 == 1);
    CHECK(facts.pollSwGp700Before80016E18 == 300);
    CHECK(facts.pollSwGp700After80016E18 == 299);
    CHECK(!facts.pollSwTimedOut80016E18);
    CHECK(facts.stateAfter.dword800917E8 == 2);
    CHECK(facts.stateAfter.dword800917F0 == 1);
    CHECK(facts.stateAfter.gp700 == 299);

    CHECK(BuildSaveUiCardIoPollFactsFromResult80016E18(state1, 2, &facts));
    CHECK(facts.pollSwResult80016E18 == 2);
    CHECK(facts.stateAfter.dword800917E8 == 4);
    CHECK(facts.stateAfter.dword800917F0 == -3);
    CHECK(facts.stateAfter.dword800917F4 == 0);

    state1.cardIoState.dword800917F4 = 1;
    CHECK(BuildSaveUiCardIoPollFactsFromResult80016E18(state1, 1, &facts));
    CHECK(facts.stateAfter.dword800917E8 == 4);
    CHECK(facts.stateAfter.dword800917F0 == 1);
    CHECK(facts.stateAfter.dword800917F4 == 1);
    state1.cardIoState.dword800917F4 = 0;
    CHECK(BuildSaveUiCardIoPollFactsFromResult80016E18(state1, 3, &facts));
    CHECK(facts.stateAfter.dword800917E8 == 4);
    CHECK(facts.stateAfter.dword800917F0 == 3);
    CHECK(facts.stateAfter.dword800917F4 == 0);
    CHECK(BuildSaveUiCardIoPollFactsFromResult80016E18(state1, 4, &facts));
    CHECK(facts.stateAfter.dword800917E8 == 2);
    CHECK(facts.stateAfter.dword800917F0 == 4);
    CHECK(facts.stateAfter.dword800917F4 == 0);

    PrStage1SaveUi19148LowerFeedbackRequest state3 = MakeCardIoRequest(3);
    state3.cardIoState.dword800917F0 = 1;
    state3.cardIoState.gp700 = 300;
    CHECK(BuildSaveUiCardIoPollFactsFromResult80016E18(state3, 1, &facts));
    CHECK(facts.pollSwResult80016E18 == 1);
    CHECK(facts.stateAfter.dword800917E8 == 4);
    CHECK(facts.stateAfter.dword800917F0 == 1);
    CHECK(facts.stateAfter.dword800917F4 == 1);
    CHECK(facts.stateAfter.gp700 == 299);
    CHECK(BuildSaveUiCardIoPollFactsFromResult80016E18(state3, 3, &facts));
    CHECK(facts.stateAfter.dword800917E8 == 4);
    CHECK(facts.stateAfter.dword800917F0 == 3);
    CHECK(facts.stateAfter.dword800917F4 == 0);
    CHECK(BuildSaveUiCardIoPollFactsFromResult80016E18(state3, 4, &facts));
    CHECK(facts.stateAfter.dword800917E8 == 4);
    CHECK(facts.stateAfter.dword800917F0 == 5);
    CHECK(facts.stateAfter.dword800917F4 == 0);

    state3.cardIoState.gp700 = 0;
    CHECK(BuildSaveUiCardIoPollFactsFromResult80016E18(state3, 1, &facts));
    CHECK(facts.pollSwResult80016E18 == 2);
    CHECK(facts.pollSwTimedOut80016E18);
    CHECK(facts.pollSwGp700After80016E18 == -1);
    CHECK(facts.stateAfter.dword800917E8 == 4);
    CHECK(facts.stateAfter.dword800917F0 == 2);
    CHECK(facts.stateAfter.dword800917F4 == 0);

    PrStage1SaveUi19148LowerFeedbackRequest invalid = state1;
    invalid.psxFunction = 0x80017598u;
    CHECK(!BuildSaveUiCardIoPollFactsFromResult80016E18(invalid, 1, &facts));
    CHECK(!facts.factsKnown);
    CHECK(!BuildSaveUiCardIoPollFactsFromResult80016E18(state1, 5, &facts));
    CHECK(!facts.factsKnown);
}

void TestNaturalSwCardEventIngressUsesIdaPriorityAndTimeout() {
    PrStage1SaveUi19148LowerFeedbackRequest request = MakeCardIoRequest(1);
    request.cardIoState.gp700 = 300;
    CardNaturalSwCardEventInput80016E18 input{};
    input.sourceKnown = true;
    input.source =
        CardNaturalEventIngressSource::DeviceTestEventProvider;
    for (bool& known : input.testEventResultKnown) {
        known = true;
    }
    input.testEventResults[0] = 1;
    input.testEventResults[1] = 1;
    input.testEventResults[2] = 0;
    input.testEventResults[3] = 1;
    input.gp700BeforeKnown = true;
    input.gp700Before = 300;

    CardIoHostFacts80017594 facts{};
    CHECK(BuildSaveUiCardIoPollFactsFromNaturalEvent80016E18(
        request, input, &facts));
    CHECK(facts.factsKnown);
    CHECK(facts.pollSwResult80016E18 == 4);
    CHECK(facts.pollSwGp700Before80016E18 == 300);
    CHECK(facts.pollSwGp700After80016E18 == 299);
    CHECK(facts.naturalSwCardEventSourceKnown80016E18);
    CHECK(facts.stateAfter.dword800917E8 == 2);

    input.gp700Before = 0;
    request.cardIoState.gp700 = 0;
    CHECK(BuildSaveUiCardIoPollFactsFromNaturalEvent80016E18(
        request, input, &facts));
    CHECK(facts.pollSwResult80016E18 == 2);
    CHECK(facts.pollSwTimedOut80016E18);
    CHECK(facts.naturalSwCardEventSourceKnown80016E18);

    input.gp700Before = 300;
    CHECK(!BuildSaveUiCardIoPollFactsFromNaturalEvent80016E18(
        request, input, &facts));
    CHECK(!facts.factsKnown);
}

void TestNaturalHwCardEventIngressRequiresFirstHit() {
    CardNaturalHwCardEventInput80017008 input{};
    int32_t pollResult = 99;
    CHECK(!ComputeNaturalHwCardPollResult80017008(input, &pollResult));
    CHECK(pollResult == 0);

    input.sourceKnown = true;
    input.source =
        CardNaturalEventIngressSource::DeviceTestEventProvider;
    for (bool& known : input.testEventResultKnown) {
        known = true;
    }
    input.testEventResults[0] = 0;
    input.testEventResults[1] = 1;
    input.testEventResults[2] = 1;
    input.testEventResults[3] = 0;
    CHECK(ComputeNaturalHwCardPollResult80017008(input, &pollResult));
    CHECK(pollResult == 2);

    input.testEventResults[1] = 0;
    input.testEventResults[2] = 0;
    input.testEventResults[3] = 1;
    CHECK(ComputeNaturalHwCardPollResult80017008(input, &pollResult));
    CHECK(pollResult == 4);
}

void TestState1Event4RequiresResetAndPostResetHwFacts() {
    CardIoHostFacts80017594 facts{};
    CHECK(!AreSaveUiCardIoEvent4ResetFactsComplete80047EE4(facts));

    facts.factsKnown = true;
    facts.stateBeforeKnown = true;
    facts.stateBefore.dword800917E8 = 1;
    facts.stateAfterKnown = true;
    facts.stateAfter.dword800917E8 = 2;
    facts.stateAfter.dword800917F0 = 4;
    facts.pollSwKnown80016E18 = true;
    facts.pollSwResult80016E18 = 4;
    facts.drainHwEventsKnown8001707C = true;
    facts.resetHwEventsKnown80047EE4 = true;
    facts.resetHwNewCardKnown80047EE4 = true;
    facts.resetHwCardWriteArgsKnown80047EE4 = true;
    facts.resetHwCardWriteArg1_80047EE4 = 63;
    facts.resetHwCardWriteResultKnown80047EE4 = true;
    facts.pollHwKnown80017008 = true;
    CHECK(AreSaveUiCardIoEvent4ResetFactsComplete80047EE4(facts));

    facts.resetHwCardWriteArg1_80047EE4 = 62;
    CHECK(!AreSaveUiCardIoEvent4ResetFactsComplete80047EE4(facts));
}

void TestState1Event4ResetProviderCopiesExactBiosShape() {
    CardIoHostFacts80017594 facts{};
    facts.factsKnown = true;
    facts.stateBeforeKnown = true;
    facts.stateBefore.dword800917E8 = 1;
    facts.stateAfterKnown = true;
    facts.stateAfter.dword800917E8 = 2;
    facts.stateAfter.dword800917F0 = 4;
    facts.pollSwKnown80016E18 = true;
    facts.pollSwResult80016E18 = 4;

    CardBiosResetProviderFacts80047EE4 provider{};
    provider.sourceKnown = true;
    provider.drainHwEventsKnown8001707C = true;
    provider.newCardKnown80047EE4 = true;
    provider.cardWriteArgsKnown80047EE4 = true;
    provider.cardWriteArg0_80047EE4 = 0;
    provider.cardWriteArg1_80047EE4 = 63;
    provider.cardWriteArg2_80047EE4 = 0;
    provider.cardWriteResultKnown80047EE4 = true;
    provider.cardWriteResult80047EE4 = -1;
    provider.pollHwKnown80017008 = true;
    provider.pollHwResult80017008 = 2;
    CHECK(ApplySaveUiCardIoEvent4ResetProviderFacts80047EE4(
        provider, &facts));
    CHECK(facts.resetHwCardWriteArg0_80047EE4 == 0);
    CHECK(facts.resetHwCardWriteArg1_80047EE4 == 63);
    CHECK(facts.resetHwCardWriteArg2_80047EE4 == 0);
    CHECK(facts.pollHwResult80017008 == 2);

    provider.cardWriteArg1_80047EE4 = 62;
    CHECK(!ApplySaveUiCardIoEvent4ResetProviderFacts80047EE4(
        provider, &facts));
}

void TestBiosResetProductionStorageProbeIsNonMutating() {
    ScopedCardFilesRestore restore;

    PrSS0CardImageStorageDirect::BiosCardResetProbe80047EE4 probe =
        PrSS0CardImageStorageDirect::ProbePrimaryCardBiosReset80047EE4();
    CHECK(probe.sourceInstalled);
    CHECK(probe.observationKnown);
    CHECK(!probe.mediaPresent);
    CHECK(!probe.imageReadable);
    CHECK(!probe.writable);
    CHECK(probe.cardWriteResultKnown);
    CHECK(probe.cardWriteResult == -1);

    std::vector<uint8_t> valid(kCardImageBytes8007A318, 0u);
    valid[0] = static_cast<uint8_t>('M');
    valid[1] = static_cast<uint8_t>('C');
    CHECK(WriteWholeFile(restore.primary(), valid));
    const std::vector<uint8_t> validBefore = ReadWholeFile(restore.primary());
    probe = PrSS0CardImageStorageDirect::ProbePrimaryCardBiosReset80047EE4();
    CHECK(probe.sourceInstalled);
    CHECK(probe.observationKnown);
    CHECK(probe.mediaPresent);
    CHECK(probe.imageReadable);
    CHECK(probe.writable);
    CHECK(probe.cardWriteResultKnown);
    CHECK(probe.cardWriteResult == 0);
    CHECK(ReadWholeFile(restore.primary()) == validBefore);

    std::vector<uint8_t> invalid(kCardImageBytes8007A318, 0xA5u);
    CHECK(WriteWholeFile(restore.primary(), invalid));
    const std::vector<uint8_t> invalidBefore = ReadWholeFile(restore.primary());
    probe = PrSS0CardImageStorageDirect::ProbePrimaryCardBiosReset80047EE4();
    CHECK(probe.sourceInstalled);
    CHECK(probe.observationKnown);
    CHECK(probe.mediaPresent);
    CHECK(!probe.imageReadable);
    CHECK(probe.writable);
    CHECK(probe.cardWriteResultKnown);
    CHECK(probe.cardWriteResult == -1);
    CHECK(ReadWholeFile(restore.primary()) == invalidBefore);
}

void TestTranslatedCardEventBrokerIsOneShotAndOrdered() {
    ResetTranslatedCardEventBroker800170C4();
    CardTranslatedEventBrokerState800170C4 broker =
        GetTranslatedCardEventBrokerState800170C4();
    CHECK(broker.initialized);
    for (int32_t i = 0; i < 4; ++i) {
        CHECK(!broker.swPending[i]);
        CHECK(!broker.hwPending[i]);
    }

    CHECK(SignalTranslatedSwCardEvent80016E18(
        CardTranslatedEventSignalSource::CardInfo80017594, 1));
    CHECK(SignalTranslatedSwCardEvent80016E18(
        CardTranslatedEventSignalSource::CardLoad80017594, 4));
    CHECK(!SignalTranslatedSwCardEvent80016E18(
        CardTranslatedEventSignalSource::CardLoad80017594, 4));

    CardNaturalSwCardEventInput80016E18 sw{};
    CHECK(PollTranslatedSwCardEvents80016E18(300, &sw));
    CHECK(sw.sourceKnown);
    CHECK(sw.source ==
          CardNaturalEventIngressSource::TranslatedDirectCardEventBroker);
    CHECK(sw.testEventResults[0] == 1);
    CHECK(sw.testEventResults[3] == 1);
    broker = GetTranslatedCardEventBrokerState800170C4();
    for (int32_t i = 0; i < 4; ++i) {
        CHECK(!broker.swPending[i]);
    }
    CHECK(SignalTranslatedSwCardEvent80016E18(
        CardTranslatedEventSignalSource::CardLoad80017594, 3));
    DrainTranslatedSwCardEvents80016FC0();
    broker = GetTranslatedCardEventBrokerState800170C4();
    CHECK(!broker.swPending[2]);

    CHECK(SignalTranslatedHwCardEvent80017008(
        CardTranslatedEventSignalSource::Format80017B60, 3));
    CHECK(SignalTranslatedHwCardEvent80017008(
        CardTranslatedEventSignalSource::Format80017B60, 2));
    CardNaturalHwCardEventInput80017008 hw{};
    int32_t pollResult = 0;
    CHECK(PollTranslatedHwCardEvents80017008(&hw));
    CHECK(ComputeNaturalHwCardPollResult80017008(hw, &pollResult));
    CHECK(pollResult == 2);
    broker = GetTranslatedCardEventBrokerState800170C4();
    CHECK(!broker.hwPending[1]);
    CHECK(broker.hwPending[2]);

    CHECK(PollTranslatedHwCardEvents80017008(&hw));
    CHECK(ComputeNaturalHwCardPollResult80017008(hw, &pollResult));
    CHECK(pollResult == 3);
    broker = GetTranslatedCardEventBrokerState800170C4();
    CHECK(!broker.hwPending[2]);
    CHECK(SignalTranslatedHwCardEvent80017008(
        CardTranslatedEventSignalSource::Format80017B60, 4));
    DrainTranslatedHwCardEvents8001707C();
    broker = GetTranslatedCardEventBrokerState800170C4();
    CHECK(!broker.hwPending[3]);

    CHECK(!SignalTranslatedHwCardEvent80017008(
        CardTranslatedEventSignalSource::CardInfo80017594, 1));
    CHECK(!SignalTranslatedSwCardEvent80016E18(
        CardTranslatedEventSignalSource::Format80017B60, 1));
    CHECK(SignalTranslatedSwCardEvent80016E18(
        CardTranslatedEventSignalSource::PhysicalHotplug80017594, 4));
    CardNaturalSwCardEventInput80016E18 hotplug{};
    CHECK(PollTranslatedSwCardEvents80016E18(300, &hotplug));
    CHECK(hotplug.testEventResults[3] == 1);
}

void TestState2CarriesLoadSubmitShapeOnly() {
    CardIoHostFacts80017594 facts{};
    CHECK(BuildSaveUiCardIoObservedNormalPathFacts80017594(
        MakeCardIoRequest(2), &facts));
    CHECK(facts.factsKnown);
    CHECK(facts.stateBeforeKnown);
    CHECK(facts.stateAfterKnown);
    CHECK(facts.clearSwEventsKnown80016FC0);
    CHECK(facts.cardLoadKnown);
    CHECK(facts.cardLoadArgKnown);
    CHECK(facts.cardLoadArg == 0);
    CHECK(facts.stateAfter.dword800917E8 == 3);
    CHECK(facts.stateAfter.gp700 == 300);
    CheckNoTypedSwPollAuthority(facts);
}

void TestState3RequiresExplicitTypedPollFeedback() {
    CardIoHostFacts80017594 facts{};
    PrStage1SaveUi19148LowerFeedbackRequest request = MakeCardIoRequest(3);
    request.cardIoState.gp700 = 229;

    CHECK(!BuildSaveUiCardIoObservedNormalPathFacts80017594(request, &facts));
    CHECK(!facts.factsKnown);
    CHECK(facts.stateBeforeKnown);
    CHECK(!facts.stateAfterKnown);
    CHECK(!facts.cardInfoKnown);
    CHECK(!facts.cardLoadKnown);
    CheckNoTypedSwPollAuthority(facts);
}

void TestState3FormatCandidateShapeStillRequiresTypedPollFeedback() {
    CardIoHostFacts80017594 facts{};
    PrStage1SaveUi19148LowerFeedbackRequest request = MakeCardIoRequest(3);
    request.cardIoState.dword800917EC = 0;
    request.cardIoState.dword800917F0 = 1;
    request.cardIoState.dword800917F4 = 0;
    request.cardIoState.gp700 = 229;

    CHECK(!BuildSaveUiCardIoObservedNormalPathFacts80017594(request, &facts));
    CHECK(!facts.factsKnown);
    CHECK(facts.stateBeforeKnown);
    CHECK(!facts.stateAfterKnown);
    CHECK(!facts.cardInfoKnown);
    CHECK(!facts.cardLoadKnown);
    CheckNoTypedSwPollAuthority(facts);
}

void TestExplicitState3TypedPollSuccessFeedback() {
    CardIoHostFacts80017594 facts{};
    facts.factsKnown = true;
    facts.stateBeforeKnown = true;
    facts.stateBefore.dword800917E8 = 3;
    facts.stateBefore.dword800917F0 = 1;
    facts.stateBefore.gp700 = 300;
    facts.stateAfterKnown = true;
    facts.stateAfter = facts.stateBefore;
    facts.stateAfter.dword800917E8 = 4;
    facts.stateAfter.dword800917F4 = 1;
    facts.stateAfter.gp700 = 299;
    facts.pollSwKnown80016E18 = true;
    facts.pollSwResult80016E18 = 1;
    facts.pollSwGp700BeforeKnown80016E18 = true;
    facts.pollSwGp700Before80016E18 = 300;
    facts.pollSwGp700AfterKnown80016E18 = true;
    facts.pollSwGp700After80016E18 = 299;
    facts.pollSwTimedOutKnown80016E18 = true;
    facts.pollSwTimedOut80016E18 = false;

    CardIoLowerFeedbackBuildResult80017594 build{};
    BuildSaveUiCardIoLowerFeedbackFromHostFacts80017594(facts, &build);

    CHECK(build.lowerFeedbackKnown);
    CHECK(build.lowerFeedback.cardIoFeedbackKnown80017594);
    const PrStage1SaveUiCardIoFeedback80017594& feedback =
        build.lowerFeedback.cardIoFeedback80017594;
    CHECK(feedback.stateBeforeKnown);
    CHECK(feedback.stateAfterKnown);
    CHECK(feedback.stateBefore.dword800917E8 == 3);
    CHECK(feedback.stateAfter.dword800917E8 == 4);
    CHECK(feedback.stateAfter.dword800917F0 == 1);
    CHECK(feedback.stateAfter.dword800917F4 == 1);
    CHECK(feedback.stateAfter.gp700 == 299);
    CHECK(feedback.pollSwKnown80016E18);
    CHECK(feedback.pollSwResult80016E18 == 1);
    CHECK(feedback.pollSwGp700BeforeKnown80016E18);
    CHECK(feedback.pollSwGp700Before80016E18 == 300);
    CHECK(feedback.pollSwGp700AfterKnown80016E18);
    CHECK(feedback.pollSwGp700After80016E18 == 299);
    CHECK(feedback.pollSwTimedOutKnown80016E18);
    CHECK(!feedback.pollSwTimedOut80016E18);
}

void TestExplicitState3TypedPollNewCardPublishesFormatIoResult() {
    CardIoHostFacts80017594 facts{};
    facts.factsKnown = true;
    facts.stateBeforeKnown = true;
    facts.stateBefore.dword800917E8 = 3;
    facts.stateBefore.dword800917F0 = 1;
    facts.stateBefore.gp700 = 300;
    facts.stateAfterKnown = true;
    facts.stateAfter = facts.stateBefore;
    facts.stateAfter.dword800917E8 = 4;
    facts.stateAfter.dword800917F0 = 5;
    facts.stateAfter.dword800917F4 = 0;
    facts.stateAfter.gp700 = 299;
    facts.pollSwKnown80016E18 = true;
    facts.pollSwResult80016E18 = 4;
    facts.pollSwGp700BeforeKnown80016E18 = true;
    facts.pollSwGp700Before80016E18 = 300;
    facts.pollSwGp700AfterKnown80016E18 = true;
    facts.pollSwGp700After80016E18 = 299;
    facts.pollSwTimedOutKnown80016E18 = true;
    facts.pollSwTimedOut80016E18 = false;

    CardIoLowerFeedbackBuildResult80017594 build{};
    BuildSaveUiCardIoLowerFeedbackFromHostFacts80017594(facts, &build);

    CHECK(build.lowerFeedbackKnown);
    CHECK(build.lowerFeedback.cardIoFeedbackKnown80017594);
    const PrStage1SaveUiCardIoFeedback80017594& feedback =
        build.lowerFeedback.cardIoFeedback80017594;
    CHECK(feedback.pollSwKnown80016E18);
    CHECK(feedback.pollSwResult80016E18 == 4);

    PrStage1SaveUiDirect::Reset19148();
    const PrStage1SaveUiCardIoCarrier80017594 state3Carrier =
        PrStage1SaveUiDirect::BuildCardIoFeedback80017594(
            facts.stateBefore,
            &feedback);
    CHECK(state3Carrier.resultKnown);
    CHECK(!state3Carrier.helperGap);
    CHECK(state3Carrier.stateAfterKnown);
    CHECK(state3Carrier.stateAfter.dword800917E8 == 4);
    CHECK(state3Carrier.stateAfter.dword800917F0 == 5);
    CHECK(state3Carrier.stateAfter.dword800917F4 == 0);

    const PrStage1SaveUiCardIoCarrier80017594 state4Carrier =
        PrStage1SaveUiDirect::BuildCardIoFeedback80017594(
            state3Carrier.stateAfter,
            nullptr);
    CHECK(state4Carrier.resultKnown);
    CHECK(!state4Carrier.helperGap);
    CHECK(state4Carrier.result == 5);
    CHECK(state4Carrier.stateAfterKnown);
    CHECK(state4Carrier.stateAfter.dword800917E8 == 0);
    CHECK(state4Carrier.stateAfter.dword800917EC == 5);
}

void TestRuntimeState3TypedPollCarrierPublishesFormatIoResult() {
    PrStage1SaveUi19148LowerFeedbackRequest request = MakeCardIoRequest(3);
    request.cardIoState.dword800917F0 = 1;
    request.cardIoState.gp700 = 300;

    CardIoHostFacts80017594 facts{};
    facts.factsKnown = true;
    facts.stateBeforeKnown = true;
    facts.stateBefore = request.cardIoState;
    facts.stateAfterKnown = true;
    facts.stateAfter = request.cardIoState;
    facts.stateAfter.dword800917E8 = 4;
    facts.stateAfter.dword800917F0 = 5;
    facts.stateAfter.dword800917F4 = 0;
    facts.stateAfter.gp700 = 299;
    facts.pollSwKnown80016E18 = true;
    facts.pollSwResult80016E18 = 4;
    facts.pollSwGp700BeforeKnown80016E18 = true;
    facts.pollSwGp700Before80016E18 = 300;
    facts.pollSwGp700AfterKnown80016E18 = true;
    facts.pollSwGp700After80016E18 = 299;
    facts.pollSwTimedOutKnown80016E18 = true;
    facts.pollSwTimedOut80016E18 = false;

    ClearSaveUiCardIoState3TypedPollCarrier80017594();
    CHECK(PublishRuntimeSaveUiCardIoState3TypedPollCarrier80017594(
        request, facts));

    SaveUiCardIoState3TypedPollCarrier80017594 carrier{};
    CHECK(GetSaveUiCardIoState3TypedPollCarrier80017594(&carrier));
    CHECK(carrier.known);
    CHECK(carrier.producerWired80016E18_80017594);
    CHECK(carrier.typedPollResultKnown80016E18);
    CHECK(carrier.pollResult80016E18 == 4);
    CHECK(carrier.lower.lowerFeedbackKnown);
    CHECK(carrier.lower.lowerFeedback.cardIoFeedbackKnown80017594);

    PrStage1SaveUiDirect::Reset19148();
    const PrStage1SaveUiCardIoCarrier80017594 state3Carrier =
        PrStage1SaveUiDirect::BuildCardIoFeedback80017594(
            request.cardIoState,
            &carrier.lower.lowerFeedback.cardIoFeedback80017594);
    CHECK(state3Carrier.resultKnown);
    CHECK(!state3Carrier.helperGap);
    CHECK(state3Carrier.stateAfterKnown);
    CHECK(state3Carrier.stateAfter.dword800917E8 == 4);
    CHECK(state3Carrier.stateAfter.dword800917F0 == 5);
    CHECK(state3Carrier.stateAfter.dword800917F4 == 0);
    ClearSaveUiCardIoState3TypedPollCarrier80017594();
}

void TestRuntimeState3TypedPollCarrierClearsOnWrongRequest() {
    PrStage1SaveUi19148LowerFeedbackRequest request = MakeCardIoRequest(3);
    request.cardIoState.dword800917F0 = 1;
    request.cardIoState.gp700 = 300;

    CardIoHostFacts80017594 facts{};
    facts.factsKnown = true;
    facts.stateBeforeKnown = true;
    facts.stateBefore = request.cardIoState;
    facts.stateAfterKnown = true;
    facts.stateAfter = request.cardIoState;
    facts.stateAfter.dword800917E8 = 4;
    facts.stateAfter.dword800917F0 = 5;
    facts.stateAfter.dword800917F4 = 0;
    facts.stateAfter.gp700 = 299;
    facts.pollSwKnown80016E18 = true;
    facts.pollSwResult80016E18 = 4;
    facts.pollSwGp700BeforeKnown80016E18 = true;
    facts.pollSwGp700Before80016E18 = 300;
    facts.pollSwGp700AfterKnown80016E18 = true;
    facts.pollSwGp700After80016E18 = 299;
    facts.pollSwTimedOutKnown80016E18 = true;
    facts.pollSwTimedOut80016E18 = false;

    ClearSaveUiCardIoState3TypedPollCarrier80017594();
    CHECK(PublishRuntimeSaveUiCardIoState3TypedPollCarrier80017594(
        request, facts));

    PrStage1SaveUi19148LowerFeedbackRequest wrongRequest = request;
    wrongRequest.psxFunction = 0x80017598u;
    CHECK(!PublishRuntimeSaveUiCardIoState3TypedPollCarrier80017594(
        wrongRequest, facts));

    SaveUiCardIoState3TypedPollCarrier80017594 carrier{};
    CHECK(!GetSaveUiCardIoState3TypedPollCarrier80017594(&carrier));
}

void TestState4CarriesPublishShapeOnly() {
    CardIoHostFacts80017594 facts{};
    PrStage1SaveUi19148LowerFeedbackRequest request = MakeCardIoRequest(4);
    request.cardIoState.dword800917F0 = 3;

    CHECK(BuildSaveUiCardIoObservedNormalPathFacts80017594(request, &facts));
    CHECK(facts.factsKnown);
    CHECK(facts.stateBeforeKnown);
    CHECK(facts.stateAfterKnown);
    CHECK(facts.stateAfter.dword800917E8 == 0);
    CHECK(facts.stateAfter.dword800917EC == 3);
    CheckNoTypedSwPollAuthority(facts);
}

PrStage1SaveUi19148LowerFeedbackRequest MakeWriteRequest() {
    PrStage1SaveUi19148LowerFeedbackRequest request{};
    request.kind = PrStage1SaveUi19148LowerFeedbackRequestKind::Write80017A10;
    request.psxFunction = kFn80017A10;
    request.retryCount = kWriteAttemptCount80017A10;
    request.nameAddress = kCardReadNameBufferAddr8007CBE8;
    request.dataAddress = kCardReadBlockBufferAddr800179B4;
    request.blockCount = kCardReadBlockCount800179B4;
    request.writeCloseGp696FactRequired80017A10 = true;
    request.writeCloseGp696Address80017A10 =
        kWriteCloseGp696Address80017A10;
    request.writeFdMustMatchCloseGp69680017A10 = true;
    request.action.kind =
        PrStage1SaveUi19148ActionKind::Call80017A10WriteSaveBlock;
    return request;
}

SaveUiWriteRuntimeFacts80017A10 MakeWriteRuntimeFacts() {
    SaveUiWriteRuntimeFacts80017A10 facts{};
    facts.factsKnown = true;
    facts.scanResultKnown80017900 = true;
    facts.scanResult80017900 = 1;
    facts.openWriteKnown80017454 = true;
    facts.openWriteFdKnown80017454 = true;
    facts.openWriteFd80017454 = 2;
    facts.openWriteReturnKnown80017454 = true;
    facts.openWriteReturn80017454 = 2;
    facts.gp696FdWriteKnown80017454 = true;
    facts.gp696Fd80017454 = 2;
    facts.clearSwEventsKnown80016FC0 = true;
    facts.writeKnown80017454 = true;
    facts.writeByteCountKnown80017454 = true;
    facts.writeByteCount80017454 =
        static_cast<int32_t>(kCardReadBlockBytes800179B4);
    facts.writeReturnKnown80017454 = true;
    facts.writeReturn80017454 = 0;
    facts.submitReturnKnown80017454 = true;
    facts.submitReturn80017454 = 0;
    facts.waitCallKnown80035560 = true;
    facts.waitArg80035560 = 4;
    facts.pollResultKnown80016EB8 = true;
    facts.pollResult80016EB8 = 1;
    facts.closeResultKnown = true;
    facts.closeResult = 0;
    facts.closeFdKnown = true;
    facts.closeFd = 2;
    facts.gp696FdCloseKnown80017A10 = true;
    facts.gp696FdClose80017A10 = 2;
    return facts;
}

void TestRuntimeWritePublishesTypedCarrier() {
    ClearSaveUiWriteTypedCarrier80017A10();

    const PrStage1SaveUi19148LowerFeedbackRequest request =
        MakeWriteRequest();
    const SaveUiWriteRuntimeFacts80017A10 facts = MakeWriteRuntimeFacts();

    SaveUiWriteRuntimeProducerResult80017A10 producer{};
    BuildSaveUiWriteFactsFromRuntimeProducerInput80017A10(
        request, facts, &producer);
    CHECK(producer.produced);
    CHECK(!producer.incomplete);
    CHECK(producer.requestMatched);
    CHECK(producer.hostFacts.factsKnown);

    CardWriteFeedbackProducerInput80017A10 input{};
    input.requestKnown = true;
    input.request = request;
    input.hostFactsKnown = true;
    input.hostFacts = producer.hostFacts;
    CardWriteLowerFeedbackBuildResult80017A10 lower{};
    BuildSaveUiWriteLowerFeedbackFromProducerInput80017A10(input, &lower);
    CHECK(lower.lowerFeedbackKnown);
    CHECK(lower.lowerFeedback.writeFeedbackKnown80017A10);
    CHECK(!lower.anyMissingRequiredFact);

    CHECK(PublishRuntimeSaveUiWriteTypedCarrier80017A10(request, facts));
    SaveUiWriteTypedCarrier80017A10 carrier{};
    CHECK(GetSaveUiWriteTypedCarrier80017A10(&carrier));
    CHECK(carrier.known);
    CHECK(carrier.producerWired80017900_80017454_80016EB8_80017A10);
    CHECK(carrier.typedWriteSuccessKnown80017A10);
    CHECK(carrier.lower.lowerFeedback.writeFeedbackKnown80017A10);
    CHECK(!carrier.lower.anyMissingRequiredFact);

    ClearSaveUiWriteTypedCarrier80017A10();
}

void TestRuntimeWriteFailureClearsTypedCarrier() {
    ClearSaveUiWriteTypedCarrier80017A10();

    const PrStage1SaveUi19148LowerFeedbackRequest request =
        MakeWriteRequest();
    const SaveUiWriteRuntimeFacts80017A10 successFacts =
        MakeWriteRuntimeFacts();
    CHECK(PublishRuntimeSaveUiWriteTypedCarrier80017A10(request,
                                                       successFacts));

    SaveUiWriteRuntimeFacts80017A10 failedPollFacts = successFacts;
    failedPollFacts.pollResult80016EB8 = 2;
    SaveUiWriteRuntimeProducerResult80017A10 failedPollProducer{};
    BuildSaveUiWriteFactsFromRuntimeProducerInput80017A10(
        request,
        failedPollFacts,
        &failedPollProducer);
    CHECK(!failedPollProducer.produced);
    CHECK(failedPollProducer.incomplete);
    CHECK(failedPollProducer.requestMatched);
    CHECK(!PublishRuntimeSaveUiWriteTypedCarrier80017A10(request,
                                                        failedPollFacts));
    SaveUiWriteTypedCarrier80017A10 carrier{};
    CHECK(!GetSaveUiWriteTypedCarrier80017A10(&carrier));

    CHECK(PublishRuntimeSaveUiWriteTypedCarrier80017A10(request,
                                                       successFacts));
    SaveUiWriteRuntimeFacts80017A10 closeMismatchFacts = successFacts;
    closeMismatchFacts.closeFd = successFacts.closeFd + 1;
    closeMismatchFacts.gp696FdClose80017A10 = successFacts.gp696Fd80017454;
    SaveUiWriteRuntimeProducerResult80017A10 closeMismatchProducer{};
    BuildSaveUiWriteFactsFromRuntimeProducerInput80017A10(
        request,
        closeMismatchFacts,
        &closeMismatchProducer);
    CHECK(!closeMismatchProducer.produced);
    CHECK(closeMismatchProducer.incomplete);
    CHECK(closeMismatchProducer.requestMatched);
    CHECK(!PublishRuntimeSaveUiWriteTypedCarrier80017A10(
        request,
        closeMismatchFacts));
    CHECK(!GetSaveUiWriteTypedCarrier80017A10(&carrier));

    CHECK(PublishRuntimeSaveUiWriteTypedCarrier80017A10(request,
                                                       successFacts));
    PrStage1SaveUi19148LowerFeedbackRequest wrongRequest = request;
    wrongRequest.dataAddress = kCardReadPayloadAddr8007ADE8;
    SaveUiWriteRuntimeProducerResult80017A10 wrongRequestProducer{};
    BuildSaveUiWriteFactsFromRuntimeProducerInput80017A10(
        wrongRequest,
        successFacts,
        &wrongRequestProducer);
    CHECK(!wrongRequestProducer.produced);
    CHECK(wrongRequestProducer.incomplete);
    CHECK(!wrongRequestProducer.requestMatched);
    CHECK(!PublishRuntimeSaveUiWriteTypedCarrier80017A10(wrongRequest,
                                                        successFacts));
    CHECK(!GetSaveUiWriteTypedCarrier80017A10(&carrier));

    ClearSaveUiWriteTypedCarrier80017A10();
}

CardFormatHostFacts80017B60 MakeFormatHostFacts(
    int32_t firstPoll,
    int32_t secondPoll,
    int32_t thirdPoll) {
    CardFormatHostFacts80017B60 facts{};
    facts.factsKnown = true;
    const int32_t polls[3] = {firstPoll, secondPoll, thirdPoll};
    for (int32_t i = 0; i < 3; ++i) {
        CardFormatHostAttemptFacts80017B60& attempt = facts.attempts[i];
        attempt.drainHwEventsKnown8001707C = true;
        attempt.formatKnown = true;
        attempt.formatArgsKnown = true;
        attempt.formatArg0 = 0x8006EABCu;
        attempt.formatArg1 = 0x8006EAC0u;
        attempt.pollResultKnown80017008 = true;
        attempt.pollResult80017008 = polls[i];
    }
    return facts;
}

PrStage1SaveUi19148LowerFeedbackRequest MakeFormatRequest() {
    PrStage1SaveUi19148LowerFeedbackRequest request{};
    request.kind = PrStage1SaveUi19148LowerFeedbackRequestKind::Format80017B60;
    request.psxFunction = kFn80017B60;
    request.retryCount = kFormatAttemptCount80017B60;
    request.formatArg0 = kFormatArg0_80017B60;
    request.formatArg1 = kFormatArg1_80017B60;
    request.action.kind =
        PrStage1SaveUi19148ActionKind::Call80017B60FormatCard;
    return request;
}

SaveUiFormatRuntimeFacts80017B60 MakeFormatRuntimeFacts(
    int32_t firstPoll,
    int32_t secondPoll,
    int32_t thirdPoll) {
    SaveUiFormatRuntimeFacts80017B60 facts{};
    facts.factsKnown = true;
    const int32_t polls[3] = {firstPoll, secondPoll, thirdPoll};
    for (int32_t i = 0; i < kFormatAttemptCount80017B60; ++i) {
        SaveUiFormatRuntimeAttemptFacts80017B60& attempt =
            facts.attempts[i];
        attempt.drainHwEventsKnown8001707C = true;
        attempt.formatKnown = true;
        attempt.formatArgsKnown = true;
        attempt.formatArg0 = kFormatArg0_80017B60;
        attempt.formatArg1 = kFormatArg1_80017B60;
        attempt.pollResultKnown80017008 = true;
        attempt.pollResult80017008 = polls[i];
    }
    return facts;
}

void TestFormatHalCarriesPollOneSuccessFacts() {
    const CardFormatHostFacts80017B60 facts =
        MakeFormatHostFacts(1, 2, 2);
    CardFormatLowerFeedbackBuildResult80017B60 lower{};
    BuildSaveUiFormatLowerFeedbackFromHostFacts80017B60(facts, &lower);

    CHECK(lower.lowerFeedbackKnown);
    CHECK(!lower.incomplete);
    CHECK(lower.lowerFeedback.formatFeedbackKnown80017B60);
    const PrStage1SaveUiFormatAttemptFeedback80017B60& attempt =
        lower.lowerFeedback.formatFeedback80017B60.attempts[0];
    CHECK(attempt.drainHwEventsKnown8001707C);
    CHECK(attempt.formatKnown);
    CHECK(attempt.formatArgsKnown);
    CHECK(attempt.formatArg0 == 0x8006EABCu);
    CHECK(attempt.formatArg1 == 0x8006EAC0u);
    CHECK(attempt.pollResultKnown80017008);
    CHECK(attempt.pollResult80017008 == 1);
}

void TestFormatHalCarriesPollThreeFailureFacts() {
    const CardFormatHostFacts80017B60 facts =
        MakeFormatHostFacts(3, 1, 1);
    CardFormatLowerFeedbackBuildResult80017B60 lower{};
    BuildSaveUiFormatLowerFeedbackFromHostFacts80017B60(facts, &lower);

    CHECK(lower.lowerFeedbackKnown);
    CHECK(lower.lowerFeedback.formatFeedbackKnown80017B60);
    CHECK(lower.lowerFeedback.formatFeedback80017B60.attempts[0]
              .pollResultKnown80017008);
    CHECK(lower.lowerFeedback.formatFeedback80017B60.attempts[0]
              .pollResult80017008 == 3);
}

void TestFormatHalCarriesRetryExhaustionFacts() {
    const CardFormatHostFacts80017B60 facts =
        MakeFormatHostFacts(2, 2, 2);
    CardFormatLowerFeedbackBuildResult80017B60 lower{};
    BuildSaveUiFormatLowerFeedbackFromHostFacts80017B60(facts, &lower);

    CHECK(lower.lowerFeedbackKnown);
    CHECK(lower.lowerFeedback.formatFeedbackKnown80017B60);
    for (int32_t i = 0; i < 3; ++i) {
        const PrStage1SaveUiFormatAttemptFeedback80017B60& attempt =
            lower.lowerFeedback.formatFeedback80017B60.attempts[i];
        CHECK(attempt.drainHwEventsKnown8001707C);
        CHECK(attempt.formatKnown);
        CHECK(attempt.pollResultKnown80017008);
        CHECK(attempt.pollResult80017008 == 2);
    }
}

PrStage1SaveUiFormatFeedbackInput80017B60 MakeFormatFeedbackInput(
    int32_t firstPoll,
    int32_t secondPoll,
    int32_t thirdPoll) {
    const CardFormatHostFacts80017B60 facts =
        MakeFormatHostFacts(firstPoll, secondPoll, thirdPoll);
    CardFormatLowerFeedbackBuildResult80017B60 lower{};
    BuildSaveUiFormatLowerFeedbackFromHostFacts80017B60(facts, &lower);
    CHECK(lower.lowerFeedbackKnown);
    CHECK(!lower.incomplete);
    CHECK(lower.lowerFeedback.formatFeedbackKnown80017B60);
    return lower.lowerFeedback.formatFeedback80017B60;
}

void TestFormatTypedFeedbackDrivesSaveUiSuccessCarrier() {
    PrStage1SaveUiDirect::Reset19148();
    const PrStage1SaveUiFormatFeedbackInput80017B60 feedback =
        MakeFormatFeedbackInput(1, 2, 2);

    const PrStage1SaveUiFormatFeedbackCarrier80017B60 carrier =
        PrStage1SaveUiDirect::BuildFormatFeedback80017B60(&feedback);

    CHECK(carrier.translated);
    CHECK(carrier.callCompleted);
    CHECK(carrier.resultKnown);
    CHECK(!carrier.retryExhaustedReturnUnknown);
    CHECK(!carrier.helperGap);
    CHECK(carrier.result == 1);
    CHECK(carrier.attemptsUsed == 1);
    CHECK(carrier.stoppedOnSuccess);
    CHECK(!carrier.stoppedOnTimeout);
}

void TestFormatTypedFeedbackDrivesSaveUiPollThreeFailureCarrier() {
    PrStage1SaveUiDirect::Reset19148();
    const PrStage1SaveUiFormatFeedbackInput80017B60 feedback =
        MakeFormatFeedbackInput(3, 1, 1);

    const PrStage1SaveUiFormatFeedbackCarrier80017B60 carrier =
        PrStage1SaveUiDirect::BuildFormatFeedback80017B60(&feedback);

    CHECK(carrier.callCompleted);
    CHECK(carrier.resultKnown);
    CHECK(!carrier.retryExhaustedReturnUnknown);
    CHECK(!carrier.helperGap);
    CHECK(carrier.result == 3);
    CHECK(carrier.attemptsUsed == 1);
    CHECK(!carrier.stoppedOnSuccess);
    CHECK(carrier.stoppedOnTimeout);
}

void TestFormatTypedFeedbackDrivesSaveUiRetryExhaustionCarrier() {
    PrStage1SaveUiDirect::Reset19148();
    const PrStage1SaveUiFormatFeedbackInput80017B60 feedback =
        MakeFormatFeedbackInput(2, 2, 2);

    const PrStage1SaveUiFormatFeedbackCarrier80017B60 carrier =
        PrStage1SaveUiDirect::BuildFormatFeedback80017B60(&feedback);

    CHECK(carrier.callCompleted);
    CHECK(!carrier.resultKnown);
    CHECK(carrier.retryExhaustedReturnUnknown);
    CHECK(!carrier.helperGap);
    CHECK(carrier.attemptsUsed == 3);
    CHECK(!carrier.stoppedOnSuccess);
    CHECK(!carrier.stoppedOnTimeout);
}

void TestFormatTypedFeedbackMissingPollFailsClosedInSaveUiCarrier() {
    PrStage1SaveUiDirect::Reset19148();

    const PrStage1SaveUiFormatFeedbackCarrier80017B60 carrier =
        PrStage1SaveUiDirect::BuildFormatFeedback80017B60(nullptr);

    CHECK(!carrier.resultKnown);
    CHECK(carrier.helperGap);
    CHECK(carrier.attemptsUsed == 1);
    CHECK(!carrier.stoppedOnSuccess);
    CHECK(!carrier.stoppedOnTimeout);
}

void TestRuntimeFormatPublishesTypedCarrier() {
    ClearSaveUiFormatTypedCarrier80017B60();

    const PrStage1SaveUi19148LowerFeedbackRequest request =
        MakeFormatRequest();
    const SaveUiFormatRuntimeFacts80017B60 facts =
        MakeFormatRuntimeFacts(1, 2, 2);

    SaveUiFormatRuntimeProducerResult80017B60 producer{};
    BuildSaveUiFormatFactsFromRuntimeProducerInput80017B60(
        request,
        facts,
        &producer);
    CHECK(producer.produced);
    CHECK(!producer.incomplete);
    CHECK(producer.requestMatched);
    CHECK(producer.runtimeFactsKnown);
    CHECK(producer.callCompleted);
    CHECK(producer.resultKnown);
    CHECK(!producer.retryExhaustedReturnUnknown);
    CHECK(producer.result80017B60 == 1);
    CHECK(producer.lower.lowerFeedback.formatFeedbackKnown80017B60);

    CHECK(PublishRuntimeSaveUiFormatTypedCarrier80017B60(request, facts));
    SaveUiFormatTypedCarrier80017B60 carrier{};
    CHECK(GetSaveUiFormatTypedCarrier80017B60(&carrier));
    CHECK(carrier.known);
    CHECK(carrier.producerWired8001707C_80017008_80017B60);
    CHECK(carrier.formatCallCompleted80017B60);
    CHECK(carrier.typedFormatResultKnown80017B60);
    CHECK(!carrier.retryExhaustedReturnUnknown80017B60);
    CHECK(carrier.result80017B60 == 1);
    CHECK(carrier.lower.lowerFeedback.formatFeedbackKnown80017B60);

    ClearSaveUiFormatTypedCarrier80017B60();
}

void TestRuntimeFormatPollThreePublishesTerminalCarrier() {
    ClearSaveUiFormatTypedCarrier80017B60();

    const PrStage1SaveUi19148LowerFeedbackRequest request =
        MakeFormatRequest();
    const SaveUiFormatRuntimeFacts80017B60 facts =
        MakeFormatRuntimeFacts(3, 1, 1);

    CHECK(PublishRuntimeSaveUiFormatTypedCarrier80017B60(request, facts));
    SaveUiFormatTypedCarrier80017B60 carrier{};
    CHECK(GetSaveUiFormatTypedCarrier80017B60(&carrier));
    CHECK(carrier.formatCallCompleted80017B60);
    CHECK(carrier.typedFormatResultKnown80017B60);
    CHECK(!carrier.retryExhaustedReturnUnknown80017B60);
    CHECK(carrier.result80017B60 == 3);
    CHECK(carrier.lower.lowerFeedback.formatFeedback80017B60.attempts[0]
              .pollResultKnown80017008);
    CHECK(carrier.lower.lowerFeedback.formatFeedback80017B60.attempts[0]
              .pollResult80017008 == 3);

    ClearSaveUiFormatTypedCarrier80017B60();
}

void TestRuntimeFormatFailureClearsTypedCarrier() {
    ClearSaveUiFormatTypedCarrier80017B60();

    const PrStage1SaveUi19148LowerFeedbackRequest request =
        MakeFormatRequest();
    const SaveUiFormatRuntimeFacts80017B60 successFacts =
        MakeFormatRuntimeFacts(1, 2, 2);
    CHECK(PublishRuntimeSaveUiFormatTypedCarrier80017B60(request,
                                                        successFacts));

    SaveUiFormatRuntimeFacts80017B60 missingPollFacts = successFacts;
    missingPollFacts.attempts[0].pollResultKnown80017008 = false;
    SaveUiFormatRuntimeProducerResult80017B60 missingPollProducer{};
    BuildSaveUiFormatFactsFromRuntimeProducerInput80017B60(
        request,
        missingPollFacts,
        &missingPollProducer);
    CHECK(!missingPollProducer.produced);
    CHECK(missingPollProducer.incomplete);
    CHECK(missingPollProducer.requestMatched);
    CHECK(!PublishRuntimeSaveUiFormatTypedCarrier80017B60(
        request,
        missingPollFacts));
    SaveUiFormatTypedCarrier80017B60 carrier{};
    CHECK(!GetSaveUiFormatTypedCarrier80017B60(&carrier));

    CHECK(PublishRuntimeSaveUiFormatTypedCarrier80017B60(request,
                                                        successFacts));
    PrStage1SaveUi19148LowerFeedbackRequest wrongRequest = request;
    wrongRequest.formatArg1 = 0x8006EAC4u;
    SaveUiFormatRuntimeProducerResult80017B60 wrongRequestProducer{};
    BuildSaveUiFormatFactsFromRuntimeProducerInput80017B60(
        wrongRequest,
        successFacts,
        &wrongRequestProducer);
    CHECK(!wrongRequestProducer.produced);
    CHECK(wrongRequestProducer.incomplete);
    CHECK(!wrongRequestProducer.requestMatched);
    CHECK(!PublishRuntimeSaveUiFormatTypedCarrier80017B60(wrongRequest,
                                                         successFacts));
    CHECK(!GetSaveUiFormatTypedCarrier80017B60(&carrier));

    ClearSaveUiFormatTypedCarrier80017B60();
}

} // namespace

int main() {
    TestRejectsNonCardIoRequest();
    TestState0CarriesInfoSubmitShapeOnly();
    TestState1RequiresExplicitTypedPollFeedback();
    TestExplicitState1TypedPollSuccessFeedback();
    TestCurrentIdaPollResultFacts80016E18();
    TestNaturalSwCardEventIngressUsesIdaPriorityAndTimeout();
    TestNaturalHwCardEventIngressRequiresFirstHit();
    TestState1Event4RequiresResetAndPostResetHwFacts();
    TestState1Event4ResetProviderCopiesExactBiosShape();
    TestBiosResetProductionStorageProbeIsNonMutating();
    TestTranslatedCardEventBrokerIsOneShotAndOrdered();
    TestState2CarriesLoadSubmitShapeOnly();
    TestState3RequiresExplicitTypedPollFeedback();
    TestState3FormatCandidateShapeStillRequiresTypedPollFeedback();
    TestExplicitState3TypedPollSuccessFeedback();
    TestExplicitState3TypedPollNewCardPublishesFormatIoResult();
    TestRuntimeState3TypedPollCarrierPublishesFormatIoResult();
    TestRuntimeState3TypedPollCarrierClearsOnWrongRequest();
    TestState4CarriesPublishShapeOnly();
    TestRuntimeWritePublishesTypedCarrier();
    TestRuntimeWriteFailureClearsTypedCarrier();
    TestFormatHalCarriesPollOneSuccessFacts();
    TestFormatHalCarriesPollThreeFailureFacts();
    TestFormatHalCarriesRetryExhaustionFacts();
    TestFormatTypedFeedbackDrivesSaveUiSuccessCarrier();
    TestFormatTypedFeedbackDrivesSaveUiPollThreeFailureCarrier();
    TestFormatTypedFeedbackDrivesSaveUiRetryExhaustionCarrier();
    TestFormatTypedFeedbackMissingPollFailsClosedInSaveUiCarrier();
    TestRuntimeFormatPublishesTypedCarrier();
    TestRuntimeFormatPollThreePublishesTerminalCarrier();
    TestRuntimeFormatFailureClearsTypedCarrier();

    if (g_failed != 0) {
        std::printf("test_ss0_card_io_authority: failed checks=%d\n",
                    g_failed);
        return 1;
    }

    std::printf("test_ss0_card_io_authority: ok\n");
    return 0;
}
