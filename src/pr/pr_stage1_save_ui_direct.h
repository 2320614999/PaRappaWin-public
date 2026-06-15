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
};

struct PrStage1SaveUi19148ActionList {
    PrStage1SaveUi19148Action actions[96]{};
    uint32_t count = 0;
    bool truncated = false;
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
    bool resultKnown = false;
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

struct PrStage1SaveUi19148LowerFeedback {
    bool cardIoFeedbackKnown80017594 = false;
    PrStage1SaveUiCardIoFeedback80017594 cardIoFeedback80017594{};
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
    uint32_t psxAddress = kPsxAddress;
    uint32_t byteCount = kByteCount;
    uint32_t lastWriterFunction = 0;
    uint32_t lastFaultAddress = 0;
    bool wrote80015CC4 = false;
    bool wrote800164B4 = false;
    bool wrote8001635C = false;
    bool wrote8001628C = false;
    bool wrote800167A8 = false;
    bool wrote80015744 = false;
    uint8_t bytes[kByteCount]{};
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
PrStage1SavePayloadProducerResult SeedColdBootStatusPrefix800154F4();
PrStageClearAllStatusQueryResult Sub800161F4();
PrSavedScoreSync169E0Result Sub800169E0(int32_t word800916D0,
                                        int32_t word800916E2);
PrStageClearStatusQueryResult Sub800166AC(int32_t a1);
PrStageClearStatusQueryResult Sub800167A8(int32_t a1, int32_t a2);
bool ImportSaveStatusPrefix80092F10(
    const PrStage1SaveStatusPrefix80092F10& seed);
PrStageClearStatusBankSnapshot GetStageClearStatusBankSnapshot();
PrStage1SaveStatusPrefix80092F10 GetSaveStatusPrefix80092F10();
void SetReplayMirrorSource(
    const PrStage1ScorerDirectReplayBufferState& replay);

}  // namespace PrStage1SaveUiDirect
