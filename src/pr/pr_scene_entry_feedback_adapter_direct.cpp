#include "pr_scene_entry_feedback_adapter_direct.h"

#include <cstring>

namespace PrSceneEntryFeedbackAdapterDirect {
namespace {

constexpr uint32_t kByte80092F10Address = 0x80092F10u;
constexpr uint32_t kDword80049278Address = 0x80049278u;

void SetGap(FeedbackAdapterResult80019414& out,
            FeedbackAdapterGap80019414 gap) {
    out.completed = false;
    out.gap = true;
    out.gapReason = gap;
}

bool IsCase17BankKnown(
    const PrSceneEntryDirect::HiScoreBankCarrier800164F8& bank) {
    for (const PrSceneEntryDirect::HiScoreBankRow800164F8& row :
         bank.rows) {
        for (const PrSceneEntryDirect::HiScoreBankSlot800164F8& slot :
             row.slots) {
            if (!slot.valid || !slot.scoreKnown || !slot.name.known) {
                return false;
            }
        }
    }
    return true;
}

}  // namespace

FeedbackAdapterResult80019414::FeedbackAdapterResult80019414(
    const FeedbackAdapterResult80019414& other) {
    *this = other;
}

FeedbackAdapterResult80019414& FeedbackAdapterResult80019414::operator=(
    const FeedbackAdapterResult80019414& other) {
    if (this == &other) {
        return *this;
    }

    completed = other.completed;
    gap = other.gap;
    gapReason = other.gapReason;
    inputMemory = other.inputMemory;
    call80019284 = other.call80019284;
    feedback = other.feedback;
    tablePsxAddress = other.tablePsxAddress;
    tableByteCount = other.tableByteCount;
    std::memcpy(tableStorage, other.tableStorage, sizeof(tableStorage));

    feedback.call80019284HostArgPtr =
        completed ? tableStorage : nullptr;
    return *this;
}

const char* FeedbackAdapterGapName80019414(
    FeedbackAdapterGap80019414 gap) {
    switch (gap) {
    case FeedbackAdapterGap80019414::None:
        return "none";
    case FeedbackAdapterGap80019414::MissingStatusPrefix80092F10:
        return "missing-status-prefix-80092f10";
    case FeedbackAdapterGap80019414::MissingStatusBank80092F1D:
        return "missing-status-bank-80092f1d";
    case FeedbackAdapterGap80019414::UnexpectedStatusPrefix80092F10:
        return "unexpected-status-prefix-80092f10";
    case FeedbackAdapterGap80019414::MissingCase17Result80019D7C:
        return "missing-case17-result-80019d7c";
    case FeedbackAdapterGap80019414::MissingCase17Gp72080019D7C:
        return "missing-case17-gp720-80019d7c";
    case FeedbackAdapterGap80019414::MissingCase17Bank80019D7C:
        return "missing-case17-bank-80019d7c";
    case FeedbackAdapterGap80019414::MissingInputMemory80019284:
        return "missing-input-memory-80019284";
    case FeedbackAdapterGap80019414::MissingRecordTable80019284:
        return "missing-record-table-80019284";
    }
    return "unknown";
}

void BuildFeedback80019414FromStatusPrefixAndCase17Bank(
    const PrStage1SaveStatusPrefix80092F10& statusPrefix80092F10,
    const PrSceneEntryDirect::Case17Result80019D7C& case17,
    FeedbackAdapterResult80019414* out) {
    if (!out) {
        return;
    }

    *out = FeedbackAdapterResult80019414{};

    if (!statusPrefix80092F10.known) {
        SetGap(
            *out,
            FeedbackAdapterGap80019414::MissingStatusPrefix80092F10);
        return;
    }
    if (!statusPrefix80092F10.statusBankKnown80092F1D) {
        SetGap(
            *out,
            FeedbackAdapterGap80019414::MissingStatusBank80092F1D);
        return;
    }
    if (statusPrefix80092F10.psxAddress != kByte80092F10Address ||
        statusPrefix80092F10.byteCount <
            PrSceneEntryDirect::kHiScoreStatusPrefixSize80019284) {
        SetGap(
            *out,
            FeedbackAdapterGap80019414::UnexpectedStatusPrefix80092F10);
        return;
    }
    if (!case17.resultKnown) {
        SetGap(
            *out,
            FeedbackAdapterGap80019414::MissingCase17Result80019D7C);
        return;
    }
    if (!case17.gp720Written) {
        SetGap(
            *out,
            FeedbackAdapterGap80019414::MissingCase17Gp72080019D7C);
        return;
    }
    if (!IsCase17BankKnown(case17.bank)) {
        SetGap(
            *out,
            FeedbackAdapterGap80019414::MissingCase17Bank80019D7C);
        return;
    }

    out->inputMemory =
        PrSceneEntryDirect::PsxBuild80019284InputMemoryFromStatusAndBank80015788(
            statusPrefix80092F10.bytes,
            PrSceneEntryDirect::kHiScoreStatusPrefixSize80019284,
            case17.bank);
    if (!out->inputMemory.inputMemoryKnown) {
        SetGap(
            *out,
            FeedbackAdapterGap80019414::MissingInputMemory80019284);
        return;
    }

    out->call80019284 =
        PrSceneEntryDirect::PsxCall80019284_BuildHiScoreRecords80015788(
            static_cast<int32_t>(kByte80092F10Address),
            true,
            out->inputMemory.a1Memory,
            sizeof(out->inputMemory.a1Memory),
            out->inputMemory.initialTableMemory,
            sizeof(out->inputMemory.initialTableMemory));
    if (!out->call80019284.resultKnown ||
        !out->call80019284.tablePsxGlyphBytesKnown) {
        SetGap(
            *out,
            FeedbackAdapterGap80019414::MissingRecordTable80019284);
        return;
    }

    std::memcpy(
        out->tableStorage,
        out->call80019284.tableBytes,
        sizeof(out->tableStorage));
    out->tablePsxAddress = kDword80049278Address;
    out->tableByteCount = sizeof(out->tableStorage);

    out->feedback.gp720Known = true;
    out->feedback.gp720 = case17.gp720;
    out->feedback.call80019284ResultKnown = true;
    out->feedback.call80019284Result = out->call80019284.result;
    out->feedback.call80019284HostArgPtr = out->tableStorage;

    out->completed = true;
    out->gap = false;
    out->gapReason = FeedbackAdapterGap80019414::None;
}

}  // namespace PrSceneEntryFeedbackAdapterDirect
