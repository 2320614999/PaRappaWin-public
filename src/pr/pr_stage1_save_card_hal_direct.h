#pragma once

#include "pr_stage1_save_ui_direct.h"

#include <array>
#include <cstddef>
#include <cstdint>

namespace PrStage1SaveCardHalDirect {

static constexpr uint32_t kFn80017A10 = 0x80017A10u;
static constexpr uint32_t kFn80019414 = 0x80019414u;
static constexpr uint32_t kFn80019D7C = 0x80019D7Cu;
static constexpr uint32_t kFn800179B4 = 0x800179B4u;
static constexpr uint32_t kFn80017900 = 0x80017900u;
static constexpr uint32_t kFn80017454 = 0x80017454u;
static constexpr uint32_t kFn800173A8 = 0x800173A8u;
static constexpr uint32_t kFn80016FC0 = 0x80016FC0u;
static constexpr uint32_t kFn80016EB8 = 0x80016EB8u;
static constexpr uint32_t kFn80035560 = 0x80035560u;
static constexpr int32_t kWriteAttemptCount80017A10 = 4;
static constexpr uint32_t kWriteCloseGp696Address80017A10 = 0x8006ECF8u;
static constexpr int32_t kReadAttemptCount800179B4 = 15;
static constexpr uint32_t kCardReadBlockBufferAddr800179B4 = 0x8007ABE8u;
static constexpr uint32_t kCardReadNameBufferAddr8007CBE8 = 0x8007CBE8u;
static constexpr uint32_t kCardReadPayloadAddr8007ADE8 = 0x8007ADE8u;
static constexpr int32_t kCardReadBlockCount800179B4 = 1;
static constexpr std::size_t kCardReadBlockBytes800179B4 = 0x2000u;
static constexpr int32_t kCardPathOpenFlags800173A8 = 0x8001;
static constexpr int32_t kCardPollLimit80016EB8 = 300;
static constexpr int32_t kCase17Arg2EarlyReturn80019D7C = 3;
static constexpr uint32_t kReadCloseGp696Address800179B4 = 0x8006ECF8u;

struct CardWriteAttemptFeedback80017A10 {
    bool scanResultKnown80017900 = false;
    int32_t scanResult80017900 = 0;
    bool openCheckKnown80017454 = false;
    bool openCheckReturnKnown80017454 = false;
    int32_t openCheckReturn80017454 = 0;
    bool openCheckFdKnown80017454 = false;
    int32_t openCheckFd80017454 = -1;
    bool openCheckCloseKnown80017454 = false;
    int32_t openCheckCloseFd80017454 = -1;
    bool openWriteKnown80017454 = false;
    bool openWriteFdKnown80017454 = false;
    int32_t openWriteFd80017454 = -1;
    bool openWriteReturnKnown80017454 = false;
    int32_t openWriteReturn80017454 = 0;
    bool gp696FdWriteKnown80017454 = false;
    int32_t gp696Fd80017454 = -1;
    bool clearSwEventsKnown80016FC0 = false;
    bool writeKnown80017454 = false;
    bool writeByteCountKnown80017454 = false;
    int32_t writeByteCount80017454 = 0;
    bool writeReturnKnown80017454 = false;
    int32_t writeReturn80017454 = 0;
    bool submitReturnKnown80017454 = false;
    int32_t submitReturn80017454 = 0;
    bool waitCallKnown80035560 = false;
    int32_t waitArg80035560 = 0;
    bool pollResultKnown80016EB8 = false;
    int32_t pollResult80016EB8 = 0;
    bool closeResultKnown = false;
    int32_t closeResult = 0;
    bool closeFdKnown = false;
    int32_t closeFd = -1;
    bool gp696FdCloseKnown80017A10 = false;
    int32_t gp696FdClose80017A10 = -1;
};

struct CardWriteFeedback80017A10 {
    bool feedbackKnown = false;
    CardWriteAttemptFeedback80017A10
        attempts[kWriteAttemptCount80017A10]{};
};

struct CardWriteHostAttemptFacts80017A10 {
    bool scanResultKnown80017900 = false;
    int32_t scanResult80017900 = 0;
    bool openCheckKnown80017454 = false;
    bool openCheckReturnKnown80017454 = false;
    int32_t openCheckReturn80017454 = 0;
    bool openCheckFdKnown80017454 = false;
    int32_t openCheckFd80017454 = -1;
    bool openCheckCloseKnown80017454 = false;
    int32_t openCheckCloseFd80017454 = -1;
    bool openWriteKnown80017454 = false;
    bool openWriteFdKnown80017454 = false;
    int32_t openWriteFd80017454 = -1;
    bool openWriteReturnKnown80017454 = false;
    int32_t openWriteReturn80017454 = 0;
    bool gp696FdWriteKnown80017454 = false;
    int32_t gp696Fd80017454 = -1;
    bool clearSwEventsKnown80016FC0 = false;
    bool writeKnown80017454 = false;
    bool writeByteCountKnown80017454 = false;
    int32_t writeByteCount80017454 = 0;
    bool writeReturnKnown80017454 = false;
    int32_t writeReturn80017454 = 0;
    bool submitReturnKnown80017454 = false;
    int32_t submitReturn80017454 = 0;
    bool waitCallKnown80035560 = false;
    int32_t waitArg80035560 = 0;
    bool pollResultKnown80016EB8 = false;
    int32_t pollResult80016EB8 = 0;
    bool closeResultKnown = false;
    int32_t closeResult = 0;
    bool closeFdKnown = false;
    int32_t closeFd = -1;
    bool gp696FdCloseKnown80017A10 = false;
    int32_t gp696FdClose80017A10 = -1;
};

struct CardWriteHostFacts80017A10 {
    bool factsKnown = false;
    CardWriteHostAttemptFacts80017A10 attempts[kWriteAttemptCount80017A10]{};
};

enum class CardWriteObservedSource80017A10 : uint8_t {
    None = 0,
    Recorder20260515 = 1,
};

struct CardWriteObservedSuccessEvidence80017A10 {
    CardWriteObservedSource80017A10 source =
        CardWriteObservedSource80017A10::None;
    bool callChainKnown = false;
    int32_t frameCall80017A10 = -1;
    uint32_t callPc80017A10 = 0;
    uint32_t callRa80017A10 = 0;
    bool writeSubmitKnown = false;
    int32_t frameWriteSubmit80017454 = -1;
    uint32_t writeBufferAddress = 0;
    int32_t writeByteCount = 0;
    bool pollSuccessKnown = false;
    int32_t framePollSuccess80016EB8 = -1;
    bool commitKnown = false;
    int32_t frameCommit80019458 = -1;
    int32_t gp696AfterCommit = 0;
    int32_t gp716AfterCommit = 0;
    int32_t gp720AfterCommit = 0;
    int32_t gp724AfterCommit = 0;
};

struct CardWriteFeedbackProducerInput80017A10 {
    bool requestKnown = false;
    PrStage1SaveUi19148LowerFeedbackRequest request{};
    bool explicitFeedbackKnown = false;
    CardWriteFeedback80017A10 explicitFeedback{};
    bool hostFactsKnown = false;
    CardWriteHostFacts80017A10 hostFacts{};
};

struct CardWriteFeedbackProducerResult80017A10 {
    bool produced = false;
    bool incomplete = false;
    bool hostFactsUsed = false;
    bool explicitFeedbackUsed = false;
    bool requestUsed = false;
    bool requestMatched = false;
    bool writeResultsCarried = false;
    CardWriteFeedback80017A10 feedback{};
};

struct CardWriteLowerFeedbackBuildResult80017A10 {
    bool lowerFeedbackKnown = false;
    bool anyMissingRequiredFact = false;
    PrStage1SaveUi19148LowerFeedback lowerFeedback{};
};

struct CardIoHostFacts80017594 {
    bool factsKnown = false;
    bool stateBeforeKnown = false;
    PrStage1SaveUiCardIoState80017594 stateBefore{};
    bool stateAfterKnown = false;
    PrStage1SaveUiCardIoState80017594 stateAfter{};
    bool cardInfoKnown = false;
    bool cardInfoArgKnown = false;
    int32_t cardInfoArg = 0;
    bool pollSwKnown80016E18 = false;
    int32_t pollSwResult80016E18 = 0;
    bool pollSwGp700BeforeKnown80016E18 = false;
    int32_t pollSwGp700Before80016E18 = 0;
    bool pollSwGp700AfterKnown80016E18 = false;
    int32_t pollSwGp700After80016E18 = 0;
    bool pollSwTimedOutKnown80016E18 = false;
    bool pollSwTimedOut80016E18 = false;
    bool clearSwEventsKnown80016FC0 = false;
    bool cardLoadKnown = false;
    bool cardLoadArgKnown = false;
    int32_t cardLoadArg = 0;
    bool drainHwEventsKnown8001707C = false;
    bool resetHwEventsKnown80047EE4 = false;
    bool resetHwNewCardKnown80047EE4 = false;
    bool resetHwCardWriteArgsKnown80047EE4 = false;
    int32_t resetHwCardWriteArg0_80047EE4 = 0;
    int32_t resetHwCardWriteArg1_80047EE4 = 0;
    int32_t resetHwCardWriteArg2_80047EE4 = 0;
    bool resetHwCardWriteResultKnown80047EE4 = false;
    int32_t resetHwCardWriteResult80047EE4 = 0;
    bool pollHwKnown80017008 = false;
    int32_t pollHwResult80017008 = 0;
};

struct CardIoLowerFeedbackBuildResult80017594 {
    bool lowerFeedbackKnown = false;
    bool incomplete = false;
    PrStage1SaveUi19148LowerFeedback lowerFeedback{};
};

struct CardFormatHostAttemptFacts80017B60 {
    bool drainHwEventsKnown8001707C = false;
    bool formatKnown = false;
    bool formatArgsKnown = false;
    uint32_t formatArg0 = 0;
    uint32_t formatArg1 = 0;
    bool pollResultKnown80017008 = false;
    int32_t pollResult80017008 = 0;
};

struct CardFormatHostFacts80017B60 {
    bool factsKnown = false;
    CardFormatHostAttemptFacts80017B60 attempts[3]{};
};

struct CardFormatLowerFeedbackBuildResult80017B60 {
    bool lowerFeedbackKnown = false;
    bool incomplete = false;
    PrStage1SaveUi19148LowerFeedback lowerFeedback{};
};

struct CardClearEventsFeedback80016FC0 {
    bool called = false;
    bool eventHandlesKnown = false;
    uint32_t eventHandles[4]{};
    bool testEventResultsKnown[4]{};
    int32_t testEventResults[4]{};
};

struct CardPollFeedback80016EB8 {
    bool called = false;
    bool eventHandlesKnown = false;
    uint32_t eventHandles[4]{};
    bool resultKnown = false;
    int32_t psxReturn = 0;
    bool timedOutKnown = false;
    bool timedOut = false;
    bool hitEventIndexKnown = false;
    int32_t hitEventIndex = -1;
    bool pollIterationCountKnown = false;
    int32_t pollIterationCount = 0;
    bool waitCallCountKnown80035560 = false;
    int32_t waitCallCount80035560 = 0;
};

struct CardReadSubmissionFeedback800173A8 {
    bool called = false;
    bool fdKnown = false;
    int32_t fd = -1;
    bool bufferAddressKnown = false;
    uint32_t bufferAddress = kCardReadBlockBufferAddr800179B4;
    bool byteCountKnown = false;
    std::size_t byteCount = kCardReadBlockBytes800179B4;
    bool readReturnKnown = false;
    int32_t readReturn = 0;
};

struct CardReadAttemptFeedback800179B4 {
    bool rowEnabledKnown = false;
    bool rowEnabled = false;
    bool rowNameKnown = false;
    char rowName[32]{};
    bool rowNameBuffer8007CBE8Known = false;
    bool cardSelectorKnown = false;
    int32_t cardSelectorGp128 = 0;
    int32_t cardSelectorGp124 = 0;
    bool liveCase17PayloadViewKnown = false;
    bool successAuthorityKnown800179B4 = false;
    bool success800179B4 = false;
    bool pathBuilt800173A8 = false;
    bool pathOpenFlagsKnown800173A8 = false;
    int32_t pathOpenFlags800173A8 = kCardPathOpenFlags800173A8;
    bool openAttempted800173A8 = false;
    bool fdKnown800173A8 = false;
    int32_t fd800173A8 = -1;
    bool gp696FdWriteKnown800173A8 = false;
    int32_t gp696Fd800173A8 = -1;
    bool openFailCloseKnown800173A8 = false;
    int32_t openFailCloseFd800173A8 = -1;
    bool targetBufferKnown = false;
    uint32_t targetBufferAddress = kCardReadBlockBufferAddr800179B4;
    bool readLengthKnown = false;
    std::size_t readLength = kCardReadBlockBytes800179B4;
    bool payloadPointerKnown = false;
    uint32_t payloadPointer = kCardReadPayloadAddr8007ADE8;
    bool payloadPassedTo800164F8 = false;
    bool blockCountKnown = false;
    int32_t blockCount = kCardReadBlockCount800179B4;
    CardClearEventsFeedback80016FC0 clearEvents{};
    CardReadSubmissionFeedback800173A8 readSubmission{};
    CardPollFeedback80016EB8 poll{};
    bool closeKnown800179B4 = false;
    bool closeFdKnown800179B4 = false;
    int32_t closeFd800179B4 = -1;
    bool blockBytesKnown = false;
    const uint8_t* blockBytes = nullptr;
    std::size_t blockByteCount = 0u;
};

struct CardReadAttemptResult800179B4 {
    bool produced = false;
    bool incomplete = false;
    bool rowSkipped = false;
    bool openFailed800173A8 = false;
    bool readSubmitted800173A8 = false;
    bool readSucceeded = false;
    bool payloadBytesAvailable = false;
    bool readLengthKnown = false;
    std::size_t readLength = kCardReadBlockBytes800179B4;
    bool payloadPointerKnown = false;
    uint32_t payloadPointer = kCardReadPayloadAddr8007ADE8;
    bool payloadPassedTo800164F8 = false;
    int32_t psxReturn800179B4 = -1;
    int32_t eventResult80016EB8 = 0;
    const uint8_t* blockBytes = nullptr;
    std::size_t blockByteCount = 0u;
};

struct CardReadFeedback800179B4 {
    bool feedbackKnown = false;
    bool word8007ABE4Known = false;
    int32_t word8007ABE4 = 0;
    CardReadAttemptFeedback800179B4 attempts[kReadAttemptCount800179B4]{};
};

struct CardReadHostAttemptFacts800179B4 {
    bool rowEnabledKnown = false;
    bool rowEnabled = false;
    bool rowNameKnown = false;
    char rowName[32]{};
    bool rowNameBuffer8007CBE8Known = false;
    bool cardSelectorKnown = false;
    int32_t cardSelectorGp128 = 0;
    int32_t cardSelectorGp124 = 0;
    bool readBufferBytesKnown = false;
    const uint8_t* readBufferBytes = nullptr;
    std::size_t readBufferByteCount = 0u;
};

struct CardReadHostFacts800179B4 {
    bool factsKnown = false;
    bool triggerChainKnown = false;
    bool arg2Known = false;
    int32_t arg2 = 0;
    bool word8007ABE4Known = false;
    int32_t word8007ABE4 = 0;
    bool rowNameBuffer8007CBE8Known = false;
    bool readBufferKnown = false;
    uint32_t readBuffer = kCardReadBlockBufferAddr800179B4;
    bool readLengthKnown = false;
    std::size_t readLength = kCardReadBlockBytes800179B4;
    bool payloadPointerKnown = false;
    uint32_t payloadPointer = kCardReadPayloadAddr8007ADE8;
    bool payloadPassedTo800164F8 = false;
    CardReadHostAttemptFacts800179B4 attempts[kReadAttemptCount800179B4]{};
};

struct State16CardReadRuntimeTypedFacts800179B4 {
    bool factsKnown = false;
    bool state16CallKnown = false;
    bool selectedBlockKnown = false;
    int32_t selectedBlockIndex = -1;
    bool selectedTitleKnown = false;
    char selectedTitle[32]{};
    bool rowCountKnown = false;
    int32_t rowCount = 0;
    bool arg2Known = false;
    int32_t arg2 = 0;
    bool nameAddressKnown = false;
    uint32_t nameAddress = kCardReadNameBufferAddr8007CBE8;
    bool targetBufferAddressKnown = false;
    uint32_t targetBufferAddress = kCardReadBlockBufferAddr800179B4;
    bool payloadAddressKnown = false;
    uint32_t payloadAddress = kCardReadPayloadAddr8007ADE8;
    bool blockCountKnown = false;
    int32_t blockCount = kCardReadBlockCount800179B4;
    bool pathCallKnown = false;
    bool cardSelectorKnown = false;
    int32_t cardPortGp128 = 0;
    int32_t cardSlotGp124 = 0;
    bool gp696FdWriteKnown = false;
    int32_t gp696Fd = -1;
    bool clearEventsCallKnown = false;
    bool readSubmissionKnown = false;
    bool readFdKnown = false;
    int32_t readFd = -1;
    bool readBufferAddressKnown = false;
    uint32_t readBufferAddress = kCardReadBlockBufferAddr800179B4;
    bool readByteCountKnown = false;
    std::size_t readByteCount = kCardReadBlockBytes800179B4;
    bool pollCallKnown = false;
    bool pollEventHandlesKnown80016EB8 = false;
    uint32_t pollEventHandle0_80016EB8 = 0;
    uint32_t pollEventHandle1_80016EB8 = 0;
    uint32_t pollEventHandle2_80016EB8 = 0;
    uint32_t pollEventHandle3_80016EB8 = 0;
    bool pollResultKnown = false;
    int32_t pollResult80016EB8 = 0;
    bool pollTimedOutKnown = false;
    bool pollTimedOut = false;
    bool pollIterationCountKnown = false;
    int32_t pollIterationCount = 0;
    bool waitCallCountKnown80035560 = false;
    int32_t waitCallCount80035560 = 0;
    bool closeKnown = false;
    bool closeFdKnown = false;
    int32_t closeFd = -1;
    bool returnKnown = false;
    int32_t psxReturn800179B4 = -1;
    bool payloadLoadCallKnown = false;
    bool payloadArgumentKnown = false;
    uint32_t payloadArgument = kCardReadPayloadAddr8007ADE8;
    bool fullPayloadBytesKnown = false;
    const uint8_t* fullPayloadBytes = nullptr;
    std::size_t fullPayloadByteCount = 0u;
};

struct CardReadFeedbackRequest800179B4 {
    uint32_t callFunction80019414 = kFn80019414;
    uint32_t caseFunction80019D7C = kFn80019D7C;
    bool state16LoadPayloadRequest80019D7C = false;
    bool case17HiScoreRequest80019D7C = false;
    bool arg2Known = false;
    int32_t arg2 = 0;
    uint32_t psxFunction = kFn800179B4;
    int32_t retryCount = kReadAttemptCount800179B4;
    uint32_t pathFunction800173A8 = kFn800173A8;
    int32_t pathOpenFlags800173A8 = kCardPathOpenFlags800173A8;
    uint32_t clearEventsFunction80016FC0 = kFn80016FC0;
    uint32_t pollFunction80016EB8 = kFn80016EB8;
    uint32_t nameAddress = kCardReadNameBufferAddr8007CBE8;
    uint32_t targetBufferAddress = kCardReadBlockBufferAddr800179B4;
    uint32_t payloadAddress = kCardReadPayloadAddr8007ADE8;
    int32_t blockCount = kCardReadBlockCount800179B4;
    std::size_t blockBytes = kCardReadBlockBytes800179B4;
    bool closeGp696FactRequired = true;
    uint32_t closeGp696Address = kReadCloseGp696Address800179B4;
    bool closeFdMustMatchGp696 = true;
};

CardReadFeedbackRequest800179B4
MakeState16LoadPayloadReadRequest800179B4(int32_t arg2);

struct CardReadFeedbackProducerInput800179B4 {
    bool requestKnown = false;
    CardReadFeedbackRequest800179B4 request{};
    bool explicitFeedbackKnown = false;
    CardReadFeedback800179B4 explicitFeedback{};
    bool hostFactsKnown = false;
    CardReadHostFacts800179B4 hostFacts{};
};

struct CardReadFeedbackProducerResult800179B4 {
    bool produced = false;
    bool incomplete = false;
    bool hostFactsUsed = false;
    bool explicitFeedbackUsed = false;
    bool requestUsed = false;
    bool requestMatched = false;
    bool triggerChainKnown = false;
    bool arg2Known = false;
    int32_t arg2 = 0;
    bool word8007ABE4Known = false;
    int32_t word8007ABE4 = 0;
    bool rowNameBuffer8007CBE8Known = false;
    bool readLengthKnown = false;
    std::size_t readLength = kCardReadBlockBytes800179B4;
    bool payloadPointerKnown = false;
    uint32_t payloadPointer = kCardReadPayloadAddr8007ADE8;
    bool payloadPassedTo800164F8 = false;
    CardReadFeedback800179B4 feedback{};
};

struct CardReadHalBuildResult800179B4 {
    bool produced = false;
    bool incomplete = false;
    bool anyPayloadBytesAvailable = false;
    CardReadAttemptResult800179B4 attempts[kReadAttemptCount800179B4]{};
};

enum class CardReadTypedCarrierSource800179B4 {
    Unknown = 0,
    RuntimeLowerCardProducer,
    DebugSyntheticFixture,
};

struct Case17CardReadTypedCarrier800179B4 {
    bool known = false;
    CardReadTypedCarrierSource800179B4 source =
        CardReadTypedCarrierSource800179B4::Unknown;
    bool producerWired800173A8_80016EB8_800179B4 = false;
    bool case17HiScorePayloadLaneKnown = false;
    bool typedReadSuccessKnown800179B4 = false;
    bool payloadBytesKnown8007ADE8 = false;
    bool incomplete = false;
    CardReadFeedback800179B4 feedback{};
    CardReadHalBuildResult800179B4 hal{};
    std::array<std::array<uint8_t, kCardReadBlockBytes800179B4>,
               kReadAttemptCount800179B4>
        blockStorage{};
};

struct State16CardReadTypedCarrier800179B4 {
    bool known = false;
    CardReadTypedCarrierSource800179B4 source =
        CardReadTypedCarrierSource800179B4::Unknown;
    bool producerWired800173A8_80016EB8_800179B4 = false;
    bool state16LoadPayloadLaneKnown = false;
    bool selectedBlockKnown = false;
    int32_t selectedBlockIndex = -1;
    bool typedReadSuccessKnown800179B4 = false;
    bool payloadBytesKnown8007ADE8 = false;
    bool incomplete = false;
    CardReadFeedback800179B4 feedback{};
    CardReadHalBuildResult800179B4 hal{};
    std::array<std::array<uint8_t, kCardReadBlockBytes800179B4>,
               kReadAttemptCount800179B4>
        blockStorage{};
};

void BuildSaveUiWriteLowerFeedback80017A10(
    const CardWriteFeedback80017A10& input,
    CardWriteLowerFeedbackBuildResult80017A10* out);
void BuildSaveUiWriteLowerFeedbackFromProducerInput80017A10(
    const CardWriteFeedbackProducerInput80017A10& input,
    CardWriteLowerFeedbackBuildResult80017A10* out);
void BuildSaveUiCardIoLowerFeedbackFromHostFacts80017594(
    const CardIoHostFacts80017594& input,
    CardIoLowerFeedbackBuildResult80017594* out);
bool BuildSaveUiCardIoObservedNormalPathFacts80017594(
    const PrStage1SaveUi19148LowerFeedbackRequest& request,
    CardIoHostFacts80017594* out);
bool BuildSaveUiWriteObservedSuccessFacts80017A10(
    const PrStage1SaveUi19148LowerFeedbackRequest& request,
    const CardWriteObservedSuccessEvidence80017A10& evidence,
    CardWriteHostFacts80017A10* out);
void BuildSaveUiFormatLowerFeedbackFromHostFacts80017B60(
    const CardFormatHostFacts80017B60& input,
    CardFormatLowerFeedbackBuildResult80017B60* out);
void BuildCardWriteFeedbackFromHostFacts80017A10(
    const CardWriteFeedbackProducerInput80017A10& input,
    CardWriteFeedbackProducerResult80017A10* out);

void BuildCardReadHalResult800179B4(
    const CardReadFeedback800179B4& input,
    CardReadHalBuildResult800179B4* out);
void BuildCardReadFeedbackFromHostFacts800179B4(
    const CardReadFeedbackProducerInput800179B4& input,
    CardReadFeedbackProducerResult800179B4* out);
bool GetState16CardReadTypedCarrier800179B4(
    State16CardReadTypedCarrier800179B4* out);
bool PublishState16CardReadTypedCarrier800179B4(
    const CardReadFeedback800179B4& feedback);
bool PublishState16CardReadTypedCarrier800179B4ForBlock(
    const CardReadFeedback800179B4& feedback,
    int32_t selectedBlockIndex);
bool PublishDebugState16CardReadTypedCarrier800179B4ForBlock(
    const CardReadFeedback800179B4& feedback,
    int32_t selectedBlockIndex);
bool PublishRuntimeState16CardReadTypedCarrier800179B4ForBlock(
    const CardReadFeedbackRequest800179B4& request,
    const CardReadFeedback800179B4& feedback,
    int32_t selectedBlockIndex);
bool IsImportableState16RuntimeTypedFacts800179B4(
    const State16CardReadRuntimeTypedFacts800179B4& facts,
    int32_t selectedBlockIndex);
bool PublishRuntimeState16CardReadTypedCarrier800179B4FromTypedFacts(
    const State16CardReadRuntimeTypedFacts800179B4& facts,
    int32_t selectedBlockIndex);
void ClearState16CardReadTypedCarrier800179B4();
bool GetCase17CardReadTypedCarrier800179B4(
    Case17CardReadTypedCarrier800179B4* out);
bool PublishCase17CardReadTypedCarrier800179B4(
    const CardReadFeedback800179B4& feedback,
    CardReadTypedCarrierSource800179B4 source =
        CardReadTypedCarrierSource800179B4::Unknown);
void ClearCase17CardReadTypedCarrier800179B4();

}  // namespace PrStage1SaveCardHalDirect
