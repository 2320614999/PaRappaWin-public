#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

constexpr size_t kPrStageStatusBankDirectMaxActions = 24u;

enum class PrStageStatusBankComod : uint8_t {
    Comod1 = 1u,
    Comod2 = 2u,
    Comod3 = 3u,
    Comod5 = 5u,
    Comod6 = 6u,
    Comod7 = 7u,
};

enum class PrStageStatusBankActionKind : uint8_t {
    None = 0u,
    StoreWord800916D0,
    StoreWord800916DA,
    StoreLocalStash,
    StoreWord800916E0,
    Call80024E54,
    Call80094440,
    Call800143F0,
    Call8001681C,
    Call80016758,
    Call8001670C,
    Call800259C0,
    Call800166AC,
    Call8001635C,
    Call8001628C,
    Call80015590,
    Call80019148,
    Call80015CC4,
    Call800169E0,
};

struct PrStageStatusBankAction {
    PrStageStatusBankActionKind kind = PrStageStatusBankActionKind::None;
    uint32_t psxAddress = 0u;
    bool arg0Known = false;
    int32_t arg0 = 0;
    bool arg1Known = false;
    int32_t arg1 = 0;
    bool arg2Known = false;
    int32_t arg2 = 0;
    bool arg3Known = false;
    int32_t arg3 = 0;
};

struct PrStageStatusBankActionTrace {
    std::array<PrStageStatusBankAction, kPrStageStatusBankDirectMaxActions>
        actions{};
    size_t count = 0u;
    bool overflow = false;
};

struct PrStageStatusBankRestoreInitInput {
    PrStageStatusBankComod comod = PrStageStatusBankComod::Comod1;
    uint16_t word800916D0 = 0u;
    uint16_t word800916DA = 0u;
    uint16_t word800916F6 = 0u;
    int32_t sceneEntryField360HalfSource = 0;
    bool defaultSetupReturn8001670CKnown = false;
    int32_t defaultSetupReturn8001670C = 0;
    bool restoreSetupReturn80016758Known = false;
    int32_t restoreSetupReturn80016758 = 0;
};

struct PrStageStatusBankRestoreInitResult {
    PrStageStatusBankActionTrace trace{};
    bool storeLocalStash = false;
    uint32_t localStashPsxAddress = 0u;
    uint16_t localStashValue = 0u;
    bool clearWord800916DA = false;
    uint16_t word800916DAAfter = 0u;
    bool writeCtxWord41 = false;
    uint16_t ctxWord41Value = 0u;
    bool writeCtxDword34 = false;
    int32_t ctxDword34Value = 0;
    bool writeCtxWord55 = false;
    uint16_t ctxWord55Value = 0u;
    bool writeCtxWord54 = false;
    uint16_t ctxWord54Value = 0u;
};

struct PrStageStatusBankClearProducerInput {
    PrStageStatusBankComod comod = PrStageStatusBankComod::Comod1;
    int32_t stageArg = 1;
    uint16_t word800916D0 = 0u;
    uint16_t word800916DA = 0u;
    bool word800916F0Known = false;
    uint16_t word800916F0 = 0u;
    uint16_t word80091816 = 0u;
    bool previousStatusKnown800166AC = false;
    int32_t previousStatus800166AC = 0;
    bool statusGateIsZero = false;
    uint16_t localStashValue = 0u;
};

struct PrStageStatusBankClearProducerResult {
    PrStageStatusBankActionTrace trace{};
    bool statusProducerCalled8001635C = false;
    int32_t statusA1 = 0;
    int32_t statusA2 = 0;
    int32_t statusA3 = 0;
    int32_t statusA4 = 0;
    bool nextStageUnlockCalled8001628C = false;
    int32_t nextStageUnlockArg = 0;
    bool saveMenuCalled = false;
    bool writeWord800916DA = false;
    uint16_t word800916DAValue = 0u;
    bool word800916DAFromLocalStash = false;
    uint32_t localStashPsxAddress = 0u;
    int32_t returnValue = 0;
    bool returnValueKnown = true;
    bool blockedByUnknownWord800916F0 = false;
};

struct PrStageStatusBankTerminalConsumerInput {
    PrStageStatusBankComod comod = PrStageStatusBankComod::Comod1;
    uint32_t eventFlags04 = 0u;
};

struct PrStageStatusBankTerminalConsumerResult {
    PrStageStatusBankActionTrace trace{};
    bool setCtx54 = false;
    uint16_t ctx54Value = 0u;
    bool setUnk8008ED1C = false;
    uint16_t unk8008ED1CValue = 0u;
    bool call800169E0 = false;
};

struct PrStageStatusBankDirectState {
    bool word800916D0Known = false;
    uint16_t word800916D0 = 0u;
    bool word800916DAKnown = false;
    uint16_t word800916DA = 0u;
    bool word800916E0Known = false;
    uint16_t word800916E0 = 0u;
    bool localStashKnown = false;
    uint32_t localStashPsxAddress = 0u;
    uint16_t localStashValue = 0u;
    bool word800916E2Known = false;
    uint16_t word800916E2 = 0u;
    bool ctxScoreDwordKnown = false;
    uint32_t ctxScoreDword = 0u;
    bool word80091816Known = false;
    uint16_t word80091816 = 0u;
};

struct PrStageStatusBankDirectCallRequest {
    bool valid = false;
    PrStageStatusBankActionKind kind = PrStageStatusBankActionKind::None;
    uint32_t psxFunction = 0u;
    bool arg0Known = false;
    int32_t arg0 = 0;
    bool arg1Known = false;
    int32_t arg1 = 0;
    bool arg2Known = false;
    int32_t arg2 = 0;
    bool arg3Known = false;
    int32_t arg3 = 0;
    bool arg2DependsOnPrevious800166AC = false;
};

struct PrStageStatusBankTraceExecutionResult {
    bool ok = true;
    bool traceOverflow = false;
    uint32_t actionsVisited = 0u;
    uint32_t directMemoryRequestsReady = 0u;
    uint32_t stateStoresApplied = 0u;
    uint32_t missingArgumentActions = 0u;
    uint32_t hostBoundaryActions = 0u;
    uint32_t unsupportedDirectActions = 0u;
    uint32_t psxAddressMismatchActions = 0u;
    std::array<PrStageStatusBankDirectCallRequest,
               kPrStageStatusBankDirectMaxActions>
        directMemoryRequests{};
    size_t directMemoryRequestCount = 0u;
    PrStageStatusBankDirectCallRequest lastStatusQuery800166AC{};
    PrStageStatusBankDirectCallRequest lastSavePayload8001635C{};
    PrStageStatusBankDirectCallRequest lastUnlock8001628C{};
    PrStageStatusBankDirectCallRequest lastScoreSync800169E0{};
};

uint32_t PrStageStatusBankComodClearProducerAddress(
    PrStageStatusBankComod comod);
uint32_t PrStageStatusBankComodRestoreInitAddress(
    PrStageStatusBankComod comod);
uint32_t PrStageStatusBankComodTerminalConsumerAddress(
    PrStageStatusBankComod comod);

PrStageStatusBankRestoreInitResult
PrStageStatusBankDirectRestoreInit(
    const PrStageStatusBankRestoreInitInput& input);

PrStageStatusBankClearProducerResult
PrStageStatusBankDirectClearProducer(
    const PrStageStatusBankClearProducerInput& input);

PrStageStatusBankTerminalConsumerResult
PrStageStatusBankDirectTerminalConsumer(
    const PrStageStatusBankTerminalConsumerInput& input);

PrStageStatusBankTraceExecutionResult
PrStageStatusBankExecuteActionTrace(
    const PrStageStatusBankActionTrace& trace,
    PrStageStatusBankDirectState* state);
