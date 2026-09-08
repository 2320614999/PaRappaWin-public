#pragma once

#include <cstddef>
#include <cstdint>

#include "pr_stage_payload_bank_direct.h"

struct PrGameContext;
struct PrStage1ScorerDirectReplayBufferState;

enum class PrStage1SaveUi19148ActionKind : uint8_t {
    None = 0,
    Call80017E58InitEventArg,
    Call80017E6CSetEventResult,
    Call80025C8CPlayInputSfx,
    Call80018060InitNameInput,
    Call80017B08EnumerateEntries,
    Call80017B18SnapshotDirectory,
    Call80017B60FormatCard,
    Call80017A10WriteSaveBlock,
    Call80025C64CopySavePayload,
    Call80019458ConsumePrefixState15,
    Call80019458CommitResultState15,
    Call80015700BackupSaveStatusPrefix,
    Call80015744RestoreSaveStatusPrefix,
    Call80015CC4InitSavePayload,
    Call8001615CMapSaveStage,
    Call800161A8MapScene,
    Call800164B4LoadSavePayload,
    Call8001628CEnsureSaveProgress,
    Call8001635CUpdateSavePayload,
    Call800169E0SyncSavedScore,
    Call80017594PollCardIo,
    Call80016E18PollCardInfoLoad,
    Call80047EE4ResetHwCardEvents,
    Call80017900ScanCardDirectory,
    Call80017454SubmitWrite,
    Call80016FC0ClearSwCardEvents,
    Call80016EB8PollSwCardEvents,
    Call8001707CDrainHwCardEvents,
    Call80017008PollHwCardEvents,
    CardHalOpenCheck,
    CardHalOpenWrite,
    CardHalWrite,
    CardHalClose,
    CardHalFormat,
    CardHalInfo,
    CardHalLoad,
    Call80035560CardWait,
    Call8001E750DrawEvent,
    Call80035560ResetInput,
    Call8001EA00EndFrame,
    Call800181D0ListInput,
    HelperGap,
};

enum class PrStage1SaveUi19148ActionHostBoundary : uint8_t {
    DirectMemory = 0,
    HostHalBoundary,
    HelperGap,
    UnsupportedHostAction,
};

enum class PrStage1SaveUi19148HostActionRequestKind : uint8_t {
    None = 0,
    PlayInputSfx,
    GapReport,
};

enum class PrStage1SaveUi19148ActionGapReason : uint8_t {
    None = 0,
    CardEventHalNotPorted,
    CardFilesystemHostActionUnsupported,
    DirectHelperGap,
    UnsupportedHostAction,
};

constexpr std::size_t kSaveUiCardInfoPreviewCapacity80020BE4 = 96u;
constexpr int32_t kSaveUiPromptFlashFrameCount80017E6C = 20;

struct PrStage1SaveUiCardInfoRenderSnapshot80020BE4 {
    bool requestBound = false;
    uint32_t argAddress = 0u;
    bool selectedMarkerKnown = false;
    int32_t selectedMarker = -1;
    bool lowerModeKnown = false;
    int32_t lowerMode = 0;
    bool topIconTemplateSlotsKnown = false;
    uint32_t topIconOffTemplate = 0u;
    uint32_t topIconOnTemplate = 0u;
    bool encodedPreviewKnown = false;
    char encodedPreview[kSaveUiCardInfoPreviewCapacity80020BE4]{};
    std::size_t encodedPreviewByteCount = 0u;
    bool lowRamDescriptorKnown = false;
    uint32_t lowRamAttr = 0u;
    uint16_t lowRamTexX = 0u;
    uint16_t lowRamTexY = 0u;
    uint16_t lowRamWidth = 0u;
    uint16_t lowRamHeight = 0u;
    uint16_t lowRamClutX = 0u;
    uint16_t lowRamClutY = 0u;
};

static constexpr std::size_t kSaveUiCardGridItemCapacity80020F94 = 15u;
static constexpr std::size_t kSaveUiCardGridTextCapacity80020F94 = 32u;

struct PrStage1SaveUiCardGridRenderSnapshot80020F94 {
    bool requestBound = false;
    uint32_t argAddress = 0u;
    int16_t rows = 0;
    int16_t columns = 0;
    int16_t itemCount = 0;
    int16_t selected = 0;
    int16_t enabled[kSaveUiCardGridItemCapacity80020F94]{};
    char slotText[kSaveUiCardGridItemCapacity80020F94]
                 [kSaveUiCardGridTextCapacity80020F94]{};
};

enum class PrStage1SaveUiEventArgUpdate80018FB0 : int32_t {
    None = 0,
    ToggleWord0,
    ForceWord0AndWord2One,
};

struct PrStage1SaveUi19148Action {
    PrStage1SaveUi19148ActionKind kind =
        PrStage1SaveUi19148ActionKind::None;
    PrStage1SaveUi19148ActionHostBoundary hostBoundary =
        PrStage1SaveUi19148ActionHostBoundary::DirectMemory;
    uint32_t psxFunction = 0;
    int32_t stateBefore = 0;
    int32_t stateAfter = 0;
    int32_t arg0 = 0;
    int32_t arg1 = 0;
    int32_t arg2 = 0;
    int32_t arg3 = 0;
    int32_t arg4 = 0;
    PrStage1SaveUiCardInfoRenderSnapshot80020BE4 cardInfoSnapshot{};
    bool usesCardGridSnapshot80020F94 = false;
};

struct PrStage1SaveUi19148ActionList {
    PrStage1SaveUi19148Action actions[96]{};
    uint32_t count = 0;
    bool truncated = false;
    PrStage1SaveUiCardGridRenderSnapshot80020F94
        cardGridSnapshot80020F94{};
};

struct PrStage1SaveUi19148HostActionRequest {
    PrStage1SaveUi19148HostActionRequestKind kind =
        PrStage1SaveUi19148HostActionRequestKind::None;
    PrStage1SaveUi19148Action action{};
    PrStage1SaveUi19148ActionGapReason gapReason =
        PrStage1SaveUi19148ActionGapReason::None;
    uint16_t sfxCue = 0;
};

struct PrStage1SaveUi19148HostActionRequestList {
    PrStage1SaveUi19148HostActionRequest requests[96]{};
    uint32_t count = 0;
    bool truncated = false;
    bool sourceActionListTruncated = false;
};

struct PrStage1SaveUiCardIoState80017594 {
    int32_t dword800917E8 = 0;
    int32_t dword800917EC = 0;
    int32_t dword800917F0 = 0;
    int32_t dword800917F4 = 0;
    int32_t gp700 = 0;
};

enum class PrStage1SaveUi19148LowerFeedbackRequestKind : uint8_t {
    None = 0,
    CardIo80017594,
    DirectoryRows80019458,
    Format80017B60,
    Write80017A10,
};

struct PrStage1SaveUi19148LowerFeedbackRequest {
    PrStage1SaveUi19148LowerFeedbackRequestKind kind =
        PrStage1SaveUi19148LowerFeedbackRequestKind::None;
    PrStage1SaveUi19148Action action{};
    uint32_t psxFunction = 0;
    int32_t stateBefore = 0;
    int32_t stateAfter = 0;
    uint32_t nameAddress = 0;
    uint32_t dataAddress = 0;
    int32_t blockCount = 0;
    int32_t retryCount = 0;
    bool writeCloseGp696FactRequired80017A10 = false;
    uint32_t writeCloseGp696Address80017A10 = 0;
    bool writeFdMustMatchCloseGp69680017A10 = false;
    uint32_t formatArg0 = 0;
    uint32_t formatArg1 = 0;
    PrStage1SaveUiCardIoState80017594 cardIoState{};
};

struct PrStage1SaveUi19148LowerFeedbackRequestList {
    PrStage1SaveUi19148LowerFeedbackRequest requests[96]{};
    uint32_t count = 0;
    bool truncated = false;
    bool sourceActionListTruncated = false;
};

struct PrStage1SaveUi19148TickResult {
    bool active = false;
    bool done = false;
    int32_t saveResult = 0;
    bool saveSucceeded = false;
    int32_t psxState = 0;
    int32_t psxEventId = 0;
    bool inputStateConsumes80018FB0 = false;
    bool inputDispatcherResultDrive80018FB0 = false;
    int32_t inputDispatcherResultPendingBefore80018FB0 = 0;
    int32_t inputMaskRaw80035510 = 0;
    int32_t inputMaskBeforeDedup80018FB0 = 0;
    int32_t inputMaskAfterDedup80018FB0 = 0;
    int32_t gp708LastInputBefore80018FB0 = 0;
    int32_t gp708LastInputAfter80018FB0 = 0;
    bool inputDuplicateSuppressed80018FB0 = false;
    bool inputHandled800185D0 = false;
    int32_t inputStateBefore800185D0 = 0;
    int32_t inputStateAfter800185D0 = 0;
    bool ioResultKnown = false;
    int32_t ioResult = 0;
    bool cardIoStateBeforeKnown80017594 = false;
    bool cardIoStateAfterKnown80017594 = false;
    PrStage1SaveUiCardIoState80017594 cardIoStateBefore80017594{};
    PrStage1SaveUiCardIoState80017594 cardIoStateAfter80017594{};
    bool consumedBy80019458State15Known = false;
    bool consumedBy80019458State15 = false;
    uint32_t state15PrefixAddress = 0;
    bool state15CopyTo8007ADE8Known = false;
    bool state15CopyTo8007ADE8 = false;
    bool saveWriteResultKnown80017A10 = false;
    int32_t saveWriteResult80017A10 = 0;
    bool saveWriteSucceeded80019458 = false;
    bool helperGap = false;
    bool gp716After80019458Known = false;
    int32_t gp716After80019458 = 0;
    bool gp720After80019458Known = false;
    int32_t gp720After80019458 = 0;
    PrStage1SaveUi19148ActionList actions{};
};

struct PrStage1SaveUiWriteAttemptFeedback80017A10 {
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

struct PrStage1SaveUiWriteFeedbackInput80017A10 {
    PrStage1SaveUiWriteAttemptFeedback80017A10 attempts[4]{};
};

struct PrStage1SaveUiWriteFeedbackCarrier80017A10 {
    bool translated = true;
    bool resultKnown = false;
    bool helperGap = false;
    int32_t result = -1;
    int32_t attemptsUsed = 0;
    bool stoppedOnSuccess = false;
    PrStage1SaveUi19148ActionList actions{};
};

struct PrStage1SaveUiFormatAttemptFeedback80017B60 {
    bool drainHwEventsKnown8001707C = false;
    bool formatKnown = false;
    bool formatArgsKnown = false;
    uint32_t formatArg0 = 0;
    uint32_t formatArg1 = 0;
    bool pollResultKnown80017008 = false;
    int32_t pollResult80017008 = 0;
};

struct PrStage1SaveUiFormatFeedbackInput80017B60 {
    PrStage1SaveUiFormatAttemptFeedback80017B60 attempts[3]{};
};

struct PrStage1SaveUiFormatFeedbackCarrier80017B60 {
    bool translated = true;
    bool callCompleted = false;
    bool resultKnown = false;
    bool retryExhaustedReturnUnknown = false;
    bool helperGap = false;
    int32_t result = 0;
    int32_t attemptsUsed = 0;
    bool stoppedOnSuccess = false;
    bool stoppedOnTimeout = false;
    PrStage1SaveUi19148ActionList actions{};
};

struct PrStage1SaveUiCardIoFeedback80017594 {
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

struct PrStage1SaveUiCardIoCarrier80017594 {
    bool translated = true;
    bool resultKnown = false;
    bool stateAfterKnown = false;
    bool helperGap = false;
    int32_t result = 0;
    PrStage1SaveUiCardIoState80017594 stateBefore{};
    PrStage1SaveUiCardIoState80017594 stateAfter{};
    PrStage1SaveUi19148ActionList actions{};
};

enum class PrStage1SaveUiDirectoryRowsSource80019458 : uint8_t {
    None = 0,
    RuntimeCardDirectoryProducer,
};

struct PrStage1SaveUiDirectoryRow80019458 {
    bool active = false;
    bool freeSlot = false;
    bool blockIndexKnown = false;
    int32_t blockIndex = -1;
    bool suffixKnown = false;
    char suffix[32]{};
};

struct PrStage1SaveUiDirectoryRowsFeedback80019458 {
    bool translated = true;
    bool sourceKnown = false;
    PrStage1SaveUiDirectoryRowsSource80019458 source =
        PrStage1SaveUiDirectoryRowsSource80019458::None;
    bool entryCountKnown = false;
    int32_t entryCount = 0;
    bool freeSlotsKnown = false;
    int32_t freeSlots = 0;
    bool rowsKnown = false;
    PrStage1SaveUiDirectoryRow80019458 rows[15]{};
};

struct PrStage1SaveUiDirectoryScanFacts80019458 {
    bool known = false;
    bool directoryRowsKnown80017B08 = false;
    bool snapshotKnown80017B18 = false;
    bool listRowsBuilt80019458 = false;
    bool entryCountKnown = false;
    int32_t entryCount = 0;
    bool freeSlotsKnown = false;
    int32_t freeSlots = 0;
    PrStage1SaveUiDirectoryRow80019458 rows[15]{};
};

struct PrStage1SaveUiDirectoryRawBankSnapshot8007A318 {
    bool known = false;
    bool anyNonZero = false;
    bool anySavePrefix = false;
    int32_t nonZeroRows = 0;
    int32_t savePrefixRows = 0;
    int32_t firstNonZeroRow = -1;
    int32_t firstSavePrefixRow = -1;
};

struct PrStage1SaveUiDirectoryRawBankView8007A318 {
    bool known = false;
    uint32_t psxAddress = 0x8007A318u;
    uint32_t byteSize = 600u;
    const uint8_t* bytes = nullptr;
    std::size_t byteCount = 0u;
};

struct PrStage1SaveUiDirectoryNameScan80017900 {
    bool attempted = false;
    bool directoryKnown = false;
    bool nameKnown = false;
    bool found = false;
    int32_t rowIndex = -1;
    int32_t psxReturn = 0;
};

struct PrStage1SaveUiDirectoryRawBankRestore8007A318 {
    bool attempted = false;
    bool sourceKnown = false;
    bool restored = false;
};

struct PrStage1SaveUiCardImageWriteRollback8007A318 {
    bool attempted = false;
    bool previousImageKnown = false;
    bool candidateCleared = false;
    bool pendingPersistenceCleared = false;
    bool previousImageRestored = false;
    int32_t blockIndex = -1;
};

struct PrStage1SaveUiDirectoryRawBankUpdate8007A318 {
    bool attempted = false;
    bool nameKnown = false;
    bool directoryKnown = false;
    bool slotKnown = false;
    bool updated = false;
    bool overwrote = false;
    bool durablePolicyKnown = false;
    bool durableCommitted = false;
    int32_t blockIndex = -1;
    uint32_t psxAddress = 0x8007A318u;
    uint32_t byteSize = 600u;
};

struct PrStage1SaveUiDirectFormatResult80017B60 {
    bool attempted = false;
    bool directoryKnown = false;
    bool formatted = false;
    bool cardImageCandidateCleared = false;
    bool cardImageCandidateKnown = false;
    bool pendingPersistenceCleared = false;
    bool slotPolicyKnown = false;
    int32_t blockIndex = -1;
    bool durablePolicyKnown = false;
    bool durableCommitted = false;
    uint32_t directoryPsxAddress = 0x8007A318u;
    uint32_t directoryByteSize = 600u;
    const char* missingOwner = nullptr;
};

struct PrStage1SaveUiCardImageSerialization8007A318 {
    bool attempted = false;
    bool directoryKnown = false;
    bool directorySlotKnown = false;
    bool blockViewKnown = false;
    bool imageSerialized = false;
    bool durablePolicyKnown = false;
    bool durableCommitted = false;
    int32_t blockIndex = -1;
    uint32_t directoryPsxAddress = 0x8007A318u;
    uint32_t blockPsxAddress = 0x8007ABE8u;
    uint32_t imageBytes = 128u * 1024u;
};

struct PrStage1SaveUiCardImageView8007A318 {
    bool known = false;
    bool durablePolicyKnown = false;
    bool durableCommitted = false;
    bool slotPolicyKnown = false;
    int32_t blockIndex = -1;
    uint32_t byteSize = 128u * 1024u;
    const uint8_t* bytes = nullptr;
    std::size_t byteCount = 0u;
};

struct PrStage1SaveUiCardImagePersistencePolicy8007A318 {
    bool attempted = false;
    bool cardImageCandidateKnown = false;
    bool directDurableStorageApiKnown = false;
    bool directDurableCommitBackendKnown = false;
    bool persistenceSinkKnown = false;
    bool persistenceSinkCommitted = false;
    bool slotPolicyKnown = false;
    int32_t blockIndex = -1;
    bool explicitNoDurablePolicyKnown = false;
    bool explicitNoDurablePolicy = false;
    bool explicitNoSavePersistencePolicyKnown = false;
    bool explicitNoSavePersistencePolicyAccepted = false;
    bool explicitNoSavePersistencePolicyFinalized = false;
    bool durablePolicyKnown = false;
    bool durableCommitted = false;
    uint32_t imageBytes = 128u * 1024u;
    const char* missingOwner = nullptr;
};

struct PrStage1SaveUiCardImagePersistenceSink8007A318 {
    bool attempted = false;
    bool cardImageCandidateKnown = false;
    bool sinkCommitted = false;
    bool slotPolicyKnown = false;
    int32_t blockIndex = -1;
    bool durablePolicyKnown = false;
    bool durableCommitted = false;
    uint32_t imageBytes = 128u * 1024u;
};

struct PrStage1SaveUiCardImagePersistenceView8007A318 {
    bool known = false;
    bool slotPolicyKnown = false;
    int32_t blockIndex = -1;
    bool durablePolicyKnown = false;
    bool durableCommitted = false;
    uint32_t byteSize = 128u * 1024u;
    const uint8_t* bytes = nullptr;
    std::size_t byteCount = 0u;
};

struct PrStage1SaveUiDirectCardLoadResult80017594 {
    bool attempted = false;
    bool persistenceKnown = false;
    bool durableReadKnown = false;
    bool imageHeaderKnown = false;
    bool directoryFramesKnown = false;
    bool directoryLoaded = false;
    int32_t activeRows = 0;
    int32_t blockIndex = -1;
    uint32_t directoryPsxAddress = 0x8007A318u;
    uint32_t directoryByteSize = 600u;
};

struct PrStage1SaveUiCardImageDurableReadIngress8007A318 {
    bool attempted = false;
    bool bytesKnown = false;
    bool slotPolicyKnown = false;
    int32_t blockIndex = -1;
    bool durablePolicyKnown = false;
    bool durableCommitted = false;
    bool sinkCommitted = false;
    uint32_t imageBytes = 128u * 1024u;
};

using PrStage1SaveUiDirectDurableCardImageCommitFn8007A318 =
    bool (*)(int32_t blockIndex,
             const uint8_t* bytes,
             std::size_t byteCount,
             void* user);

struct PrStage1SaveUiCardImageDurableCommitPrimitive8007A318 {
    bool attempted = false;
    bool persistenceSinkKnown = false;
    bool slotPolicyKnown = false;
    int32_t blockIndex = -1;
    bool directBackendKnown = false;
    bool directBackendCalled = false;
    bool directBackendAccepted = false;
    bool explicitNoDurablePolicyKnown = false;
    bool explicitNoDurablePolicy = false;
    bool explicitNoSavePersistencePolicyKnown = false;
    bool explicitNoSavePersistencePolicyAccepted = false;
    bool explicitNoSavePersistencePolicyFinalized = false;
    bool durablePolicyKnown = false;
    bool durableCommitted = false;
    uint32_t imageBytes = 128u * 1024u;
    const char* missingOwner = nullptr;
};

struct PrStage1SaveUiWriteBlockView80017A10 {
    bool known = false;
    bool saveHeaderBuilt = false;
    bool savePayloadCopied = false;
    uint32_t psxAddress = 0x8007ABE8u;
    uint32_t byteSize = 0x2000u;
    const uint8_t* bytes = nullptr;
    std::size_t byteCount = 0u;
};

struct PrStage1SaveUiNameBufferView8007CBE8 {
    bool known = false;
    uint32_t psxAddress = 0x8007CBE8u;
    const char* bytes = nullptr;
    std::size_t byteCount = 0u;
};

struct PrStage1SaveUi19148LowerFeedback {
    bool cardIoFeedbackKnown80017594 = false;
    PrStage1SaveUiCardIoFeedback80017594 cardIoFeedback80017594{};
    bool directoryRowsFeedbackKnown80019458 = false;
    PrStage1SaveUiDirectoryRowsFeedback80019458
        directoryRowsFeedback80019458{};
    bool formatFeedbackKnown80017B60 = false;
    PrStage1SaveUiFormatFeedbackInput80017B60 formatFeedback80017B60{};
    bool writeFeedbackKnown80017A10 = false;
    PrStage1SaveUiWriteFeedbackInput80017A10 writeFeedback80017A10{};
};

struct PrStage1SavePayloadProducerResult {
    bool ok = false;
    bool payloadKnown = false;
    bool helperGap = false;
    int32_t result = 0;
    uint32_t lastFaultAddress = 0;
    PrStage1SaveUi19148ActionList actions{};
};

struct PrStage1ColdBootStatusSeedContext800154F4 {
    bool seedOpportunityConsumed = false;
};

struct PrStage1SaveStatusBackupResult80015700 {
    bool ok = false;
    bool backupKnown = false;
    bool backupStatusBankKnown80092F1D = false;
    bool restoreKnown = false;
    bool helperGap = false;
    int32_t result = 0;
    uint32_t psxFunction = 0;
    uint32_t a1Address = 0;
    uint32_t lastFaultAddress = 0;
    PrStage1SaveUi19148ActionList actions{};
};

struct PrStage1SaveStatusPrefix80092F10 {
    static constexpr uint32_t kPsxAddress =
        PrStagePayloadBankDirect::kBaseAddress80092F10;
    static constexpr uint32_t kByteCount =
        PrStagePayloadBankDirect::kByteCount80092F10;

    bool known = false;
    bool statusBankKnown80092F1D = false;
    bool helperGap = false;
    PrStagePayloadBankDirect::ReplayPayloadBackingProvenance80092F5C
        replayPayloadBackingProvenance80092F5C =
            PrStagePayloadBankDirect::
                ReplayPayloadBackingProvenance80092F5C::Unknown;
    uint32_t psxAddress = kPsxAddress;
    uint32_t byteCount = kByteCount;
    uint32_t lastWriterFunction = 0;
    uint32_t seedAuthorityFunction = 0;
    uint32_t lastFaultAddress = 0;
    bool wrote80015CC4 = false;
    bool wrote800164B4 = false;
    bool wrote8001635C = false;
    bool wrote8001628C = false;
    bool wrote800167A8 = false;
    bool wrote80015744 = false;
    uint8_t bytes[kByteCount]{};
};

struct PrStage1SavePayloadBankRuntimeSnapshot {
    bool payloadKnown = false;
    bool statusBankKnown80092F1D = false;
    bool helperGap = false;
    bool savePayloadSourceKnown = false;
    PrStagePayloadBankDirect::ReplayPayloadBackingProvenance80092F5C
        replayPayloadBackingProvenance80092F5C =
            PrStagePayloadBankDirect::
                ReplayPayloadBackingProvenance80092F5C::Unknown;
    bool replayMirrorCandidateKnown8008EEF8 = false;
    bool replayMirrorCandidateStartupZeroAuthorityKnown80028590 = false;
    bool replayMirrorCandidateProducerKnown8008EEF8 = false;
    uint32_t replayMirrorCandidateProducerFunction = 0;
    bool replayMirrorCandidateByteCountKnown8008EEF8 = false;
    uint32_t replayMirrorCandidateKnownByteCount8008EEF8 = 0;
    bool replayMirrorCandidateFullBackingKnown8008EEF8 = false;
    uint32_t replayMirrorCandidatePublishedCount901BC = 0;
    uint32_t replayMirrorCandidateWriteCount901C0 = 0;
    bool replayMirrorSourceKnown8008EEF8 = false;
    bool replayMirrorSourceShapeKnown8008EEF8 = false;
    bool replayMirrorSourceStartupZeroAuthorityKnown80028590 = false;
    bool replayMirrorSourceProducerKnown8008EEF8 = false;
    uint32_t replayMirrorSourceProducerFunction = 0;
    bool replayMirrorSourceByteCountKnown8008EEF8 = false;
    uint32_t replayMirrorSourceKnownByteCount8008EEF8 = 0;
    bool replayMirrorSourceFullBackingKnown8008EEF8 = false;
    uint32_t replayMirrorSourcePublishedCount901BC = 0;
    uint32_t replayMirrorSourceWriteCount901C0 = 0;
    uint32_t replayMirrorSourceSetCount = 0;
    uint32_t replayMirrorSourceInvalidSetCount = 0;
    uint32_t replayMirrorSourceHydrateCount = 0;
    bool replayMirrorAuthorityKnown8001635C = false;
    uint32_t lastWriterFunction = 0;
    uint32_t seedAuthorityFunction = 0;
    uint32_t lastFaultAddress = 0;
    bool wrote800164B4 = false;
    bool wrote8001635C = false;
    bool sub80015CC4Attempted = false;
    bool sub80015CC4Ok = false;
    int32_t sub80015CC4Result = 0;
    bool sub8001635CAttempted = false;
    bool sub8001635COk = false;
    int32_t sub8001635CResult = 0;
    bool sub8001635CPreflightPayloadKnown = false;
    bool sub8001635CPreflightStatusBankKnown = false;
    bool sub8001635CPreflightMapped = false;
    bool sub8001635CPreflightCarrierSourceKnown = false;
    uint32_t sub8001635CPreflightCarrierSource = 0;
    bool sub8001635CPreflightMirrorSourceKnown = false;
    bool sub8001635CPreflightReplayMirrorProducerSourceKnown = false;
    bool sub8001635CPreflightStartupZeroSourceKnown = false;
    bool sub8001635CReplayMirrorSourceKnownAtEntry = false;
    bool sub8001635CReplayMirrorSourceShapeKnownAtEntry = false;
    uint32_t sub8001635CReplayMirrorSourceSetCountAtEntry = 0;
    uint32_t sub8001635CReplayMirrorSourceInvalidSetCountAtEntry = 0;
    uint32_t sub8001635CReplayMirrorSourceHydrateCountAtEntry = 0;
    bool sub8001635CScratchAuthorityKnown = false;
    bool sub8001635CMirrorCopied = false;
    bool sub8001635CAllClearQueried = false;
    bool sub8001635CAllClearWritten = false;
    bool sub800164B4Attempted = false;
    bool sub800164B4Ok = false;
    int32_t sub800164B4Result = 0;
    bool typed800164B4Attempted = false;
    bool typed800164B4Ok = false;
    bool import80092F10Attempted = false;
    bool import80092F10Ok = false;
    bool seedColdBootAttempted = false;
    bool seedColdBootOk = false;
    int32_t seedColdBootResult = 0;
    bool seedColdBootStartupZeroAccepted = false;
};

struct PrStageClearStatusBankSnapshot {
    bool statusBytesKnown80092F1D = false;
    uint8_t byte80092F1D[6]{};
    bool scoreDwordsKnown80092F24 = false;
    uint32_t dword80092F24[6]{};
    bool lastSavedSlotKnown80092F3C = false;
    uint32_t dword80092F3C = 0;
    bool allClearKnown80092F44 = false;
    uint32_t dword80092F44 = 0;
};

struct PrStageClearStatusQueryResult {
    bool ok = false;
    bool statusBankKnown = false;
    bool mapped = false;
    bool helperGap = false;
    uint32_t psxFunction = 0;
    int32_t sceneId = 0;
    int32_t slotIndex = -1;
    uint8_t status = 0;
};

struct PrStageClearAllStatusQueryResult {
    bool ok = false;
    bool statusBankKnown = false;
    bool statusBytesKnown80092F1D = false;
    bool helperGap = false;
    uint32_t psxFunction = 0;
    uint32_t statusBaseAddress80092F1D = 0;
    uint8_t byte80092F1D[6]{};
    uint32_t result = 0;
};

struct PrSavedScoreSync169E0Result {
    bool ok = false;
    bool applied = false;
    bool statusBankKnown = false;
    bool mapped = false;
    bool helperGap = false;
    uint32_t psxFunction = 0;
    int32_t word800916D0 = 0;
    int32_t word800916E2 = 0;
    int32_t slotIndex = -1;
    uint32_t ctxScoreDword = 0;
    uint16_t word80091816 = 0;
    PrStage1SaveUi19148ActionList actions{};
};

namespace PrStage1SaveUiDirect {

void Reset19148();
bool Start19148(PrGameContext& ctx);
bool Start19148(PrGameContext& ctx,
                const PrStage1SaveStatusPrefix80092F10* seed80092F10);
PrStage1SaveUi19148TickResult Tick19148(PrGameContext& ctx);
PrStage1SaveUi19148TickResult Tick19148(
    PrGameContext& ctx,
    const PrStage1SaveUi19148LowerFeedback* lowerFeedback);
bool IsActive19148();
// Call only after 80018FB0's final feedback presentation has returned.
void SnapshotDirectory80018F70();

const char* ActionKindName19148(PrStage1SaveUi19148ActionKind kind);
const char* ActionHostBoundaryName19148(
    PrStage1SaveUi19148ActionHostBoundary boundary);
const char* ActionGapReasonName19148(
    PrStage1SaveUi19148ActionGapReason reason);
PrStage1SaveUi19148HostActionRequestList BuildHostActionRequests19148(
    const PrStage1SaveUi19148ActionList& actions);
PrStage1SaveUi19148LowerFeedbackRequestList
BuildLowerFeedbackRequests19148(
    const PrStage1SaveUi19148ActionList& actions);

PrStage1SaveUiWriteFeedbackCarrier80017A10 BuildWriteFeedback80017A10(
    uint32_t nameAddress,
    uint32_t dataAddress,
    int32_t blocks,
    const PrStage1SaveUiWriteFeedbackInput80017A10* feedback);
PrStage1SaveUiFormatFeedbackCarrier80017B60 BuildFormatFeedback80017B60(
    const PrStage1SaveUiFormatFeedbackInput80017B60* feedback);
PrStage1SaveUiCardIoCarrier80017594 BuildCardIoFeedback80017594(
    const PrStage1SaveUiCardIoState80017594& state,
    const PrStage1SaveUiCardIoFeedback80017594* feedback);
bool BuildSaveUiDirectoryRowsFeedbackFromScanFacts80019458(
    const PrStage1SaveUiDirectoryScanFacts80019458& facts,
    PrStage1SaveUiDirectoryRowsFeedback80019458* out);

PrStage1SavePayloadProducerResult Sub80015CC4();
PrStage1SaveStatusBackupResult80015700 Sub80015700(uint32_t a1Address);
PrStage1SaveStatusBackupResult80015700 Sub80015744(uint32_t a1Address);
PrStage1SavePayloadProducerResult Sub800164B4(uint32_t srcAddress);
PrStage1SavePayloadProducerResult CommitTypedPayload800164B4(
    uint32_t srcAddress,
    const uint8_t* source,
    std::size_t sourceBytes,
    const PrStagePayloadBankDirect::LoadSavePayloadAuthority800164B4&
        authority);
PrStage1SavePayloadProducerResult Sub8001628C(int32_t a1);
PrStage1SavePayloadProducerResult Sub8001635C(int32_t a1,
                                              int32_t a2,
                                              int32_t a3,
                                              int32_t a4);
PrStage1SavePayloadProducerResult SeedColdBootStatusPrefix800154F4(
    PrStage1ColdBootStatusSeedContext800154F4& bootContext);
PrStageClearAllStatusQueryResult Sub800161F4();
PrSavedScoreSync169E0Result Sub800169E0(int32_t word800916D0,
                                        int32_t word800916E2);
PrStageClearStatusQueryResult Sub800166AC(int32_t a1);
PrStageClearStatusQueryResult Sub800167A8(int32_t a1, int32_t a2);
bool ImportSaveStatusPrefix80092F10(
    const PrStage1SaveStatusPrefix80092F10& seed);
void InvalidateSaveStatusPrefixAuthority80092F10(uint32_t faultAddress);
PrStageClearStatusBankSnapshot GetStageClearStatusBankSnapshot();
PrStage1SaveStatusPrefix80092F10 GetSaveStatusPrefix80092F10();
PrStage1SavePayloadBankRuntimeSnapshot
GetSavePayloadBankRuntimeSnapshot();
PrStage1SaveUiDirectoryRawBankSnapshot8007A318
GetSaveUiDirectoryRawBankSnapshot8007A318();
PrStage1SaveUiDirectoryRawBankView8007A318
GetSaveUiDirectoryRawBankView8007A318();
PrStage1SaveUiDirectoryNameScan80017900
ScanSaveUiDirectoryRawBankName80017900(const char* internalName);
PrStage1SaveUiDirectoryRawBankRestore8007A318
RestoreSaveUiDirectoryRawBankAfterFailedWrite8007A318(
    const uint8_t* bytes,
    std::size_t byteCount);
PrStage1SaveUiCardImageWriteRollback8007A318
RollbackSaveUiCardImageAfterFailedWrite8007A318(
    const uint8_t* previousBytes,
    std::size_t previousByteCount,
    int32_t previousBlockIndex,
    bool previousDurableCommitted);
PrStage1SaveUiDirectoryRawBankUpdate8007A318
ApplySaveUiDirectoryRawBankSerializedEntry8007A318(
    const char* internalName,
    uint32_t blockBytes);
PrStage1SaveUiDirectFormatResult80017B60
FormatSaveUiDirectCardImage80017B60();
PrStage1SaveUiCardImageSerialization8007A318
SerializeSaveUiCardImageCandidateFromDirectBuffers8007A318();
PrStage1SaveUiCardImageView8007A318
GetSaveUiCardImageCandidateView8007A318();
PrStage1SaveUiCardImagePersistencePolicy8007A318
EvaluateSaveUiCardImagePersistencePolicy8007A318();
PrStage1SaveUiCardImagePersistenceSink8007A318
CommitSaveUiCardImagePersistenceSink8007A318();
PrStage1SaveUiCardImagePersistenceView8007A318
GetSaveUiCardImagePersistenceSinkView8007A318();
PrStage1SaveUiDirectCardLoadResult80017594
LoadSaveUiDirectCardImageDirectory80017594();
PrStage1SaveUiCardImageDurableReadIngress8007A318
ImportSaveUiCardImagePersistenceSinkFromDirectDurableRead8007A318(
    const uint8_t* bytes,
    std::size_t byteCount,
    int32_t blockIndex);
void SetSaveUiCardImageDirectDurableCommitBackend8007A318(
    PrStage1SaveUiDirectDurableCardImageCommitFn8007A318 fn,
    void* user);
PrStage1SaveUiCardImageDurableCommitPrimitive8007A318
CommitSaveUiCardImageDirectDurablePrimitive8007A318();
PrStage1SaveUiWriteBlockView80017A10
GetSaveUiWriteBlockView80017A10();
PrStage1SaveUiNameBufferView8007CBE8
GetSaveUiNameBufferView8007CBE8();
void PublishAuthoritativeReplayMirrorSourceFromStage1(
    const PrStage1ScorerDirectReplayBufferState& replay);

}  // namespace PrStage1SaveUiDirect
