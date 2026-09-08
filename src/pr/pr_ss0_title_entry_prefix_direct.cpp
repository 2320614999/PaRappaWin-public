#include "pr_ss0_title_entry_prefix_direct.h"

namespace PrSS0TitleEntryPrefixDirect {
namespace {

Operation801C4894 ZeroRange(uint32_t begin,
                            uint32_t end,
                            uint32_t byteCount)
{
    Operation801C4894 operation{};
    operation.kind = OperationKind801C4894::ZeroRange;
    operation.target = begin;
    operation.rangeEnd = end;
    operation.byteCount = byteCount;
    return operation;
}

Operation801C4894 WordWrite(uint32_t address, uint16_t value)
{
    Operation801C4894 operation{};
    operation.kind = OperationKind801C4894::WordWrite;
    operation.target = address;
    operation.value = value;
    return operation;
}

Operation801C4894 DwordWrite(uint32_t address, uint32_t value)
{
    Operation801C4894 operation{};
    operation.kind = OperationKind801C4894::DwordWrite;
    operation.target = address;
    operation.value = value;
    return operation;
}

Operation801C4894 GpDwordWrite(uint32_t offset, uint32_t value)
{
    Operation801C4894 operation{};
    operation.kind = OperationKind801C4894::GpDwordWrite;
    operation.target = offset;
    operation.value = value;
    return operation;
}

bool Append(Call801C4894& call, const Operation801C4894& operation)
{
    if (call.operationCount >= call.operations.size()) {
        return false;
    }
    call.operations[call.operationCount++] = operation;
    return true;
}

Call801C4894& BeginCall(Transaction801C4894& transaction,
                        std::size_t index,
                        Step801C4894 step,
                        uint32_t functionAddress)
{
    transaction.stepOrder[index] = step;
    Call801C4894& call = transaction.calls[index];
    call.step = step;
    call.functionAddress = functionAddress;
    return call;
}

} // namespace

bool BuildTransaction801C4894(const Source801C4894& source,
                              Transaction801C4894& out)
{
    out = Transaction801C4894{};
    if (!source.contextKnown ||
        source.contextAddress != kContextAddress801C3640 ||
        !source.scene0TablePointerKnown ||
        source.scene0TablePointer != kScene0TablePointer801C6DA4 ||
        !source.word800916DCKnown || source.word800916DC > 1u) {
        return false;
    }

    Transaction801C4894 transaction{};

    Call801C4894& call80014344 = BeginCall(
        transaction, 0u, Step801C4894::Call80014344, 0x80014344u);
    if (!Append(call80014344, GpDwordWrite(0x1Cu, 0x80092910u)) ||
        !Append(call80014344,
                ZeroRange(0x80092910u, 0x80092F10u, 0x600u)) ||
        !Append(call80014344,
                ZeroRange(0x80091800u, 0x8009182Cu, 0x2Cu)) ||
        !Append(call80014344, WordWrite(0x80091810u, 0u)) ||
        !Append(call80014344, WordWrite(0x80091812u, 0u)) ||
        !Append(call80014344, WordWrite(0x80091814u, 0u)) ||
        !Append(call80014344, WordWrite(0x80091816u, 0u)) ||
        !Append(call80014344, WordWrite(0x80091818u, 0u)) ||
        !Append(call80014344, WordWrite(0x8009181Au, 0u)) ||
        !Append(call80014344, WordWrite(0x8009181Cu, 0u)) ||
        !Append(call80014344, WordWrite(0x80091822u, 0u)) ||
        !Append(call80014344, DwordWrite(0x8009180Cu, 0u)) ||
        !Append(call80014344, DwordWrite(0x80091808u, 0u)) ||
        !Append(call80014344, GpDwordWrite(0x20u, 0u)) ||
        !Append(call80014344, DwordWrite(0x80091800u, 0u))) {
        return false;
    }

    Call801C4894& call80024E98 = BeginCall(
        transaction, 1u, Step801C4894::Call80024E98, 0x80024E98u);
    if (!Append(call80024E98,
                ZeroRange(0x8008ECE0u, 0x8008EDD4u, 0xF4u)) ||
        !Append(call80024E98, DwordWrite(0x8008ED20u, 0u)) ||
        !Append(call80024E98, DwordWrite(0x8008ED00u, 0u)) ||
        !Append(call80024E98, WordWrite(0x8008ED2Eu, 1u)) ||
        !Append(call80024E98, WordWrite(0x8008ED2Cu, 0u)) ||
        !Append(call80024E98, GpDwordWrite(0x320u, 0u)) ||
        !Append(call80024E98,
                ZeroRange(0x801C3640u, 0x801C3828u, 0x1E8u)) ||
        !Append(call80024E98, WordWrite(0x801C368Eu, 1u)) ||
        !Append(call80024E98, WordWrite(0x801C3690u, 0u)) ||
        !Append(call80024E98, WordWrite(0x801C3692u, 0u)) ||
        !Append(call80024E98, WordWrite(0x801C369Au, 0u)) ||
        !Append(call80024E98, WordWrite(0x801C36AAu, 1u)) ||
        !Append(call80024E98, WordWrite(0x801C36BAu, 1u)) ||
        !Append(call80024E98, WordWrite(0x801C36CAu, 0u)) ||
        !Append(call80024E98, WordWrite(0x801C36D0u, 0u)) ||
        !Append(call80024E98, WordWrite(0x801C36E2u, 0u)) ||
        !Append(call80024E98,
                DwordWrite(0x801C36D4u, 0x801C6DA8u)) ||
        !Append(call80024E98,
                DwordWrite(0x801C36D8u, 0x801C6DBCu)) ||
        !Append(call80024E98,
                WordWrite(0x801C36A8u, source.word800916DC)) ||
        !Append(call80024E98,
                WordWrite(0x801C3694u, source.word800916DC)) ||
        !Append(call80024E98, WordWrite(0x8008ED36u, 2u)) ||
        !Append(call80024E98, WordWrite(0x8008ED38u, 0u)) ||
        !Append(call80024E98, WordWrite(0x801C369Au, 0u)) ||
        !Append(call80024E98, WordWrite(0x80091818u, 0u)) ||
        !Append(call80024E98, WordWrite(0x8009181Au, 0u)) ||
        !Append(call80024E98, WordWrite(0x8009181Cu, 0u))) {
        return false;
    }

    Call801C4894& call80024FC0 = BeginCall(
        transaction, 2u, Step801C4894::Call80024FC0, 0x80024FC0u);
    call80024FC0.argumentKnown = true;
    call80024FC0.argument = kContextAddress801C3640;
    if (!Append(call80024FC0, DwordWrite(0x801C3670u, 0u))) {
        return false;
    }

    Call801C4894& call801C4FA0 = BeginCall(
        transaction, 3u, Step801C4894::Call801C4FA0, 0x801C4FA0u);
    call801C4FA0.staticReturnKnown = true;
    call801C4FA0.staticReturn = -12;
    if (!Append(call801C4FA0, WordWrite(0x801C6E58u, 0u)) ||
        !Append(call801C4FA0, WordWrite(0x801C6E54u, 0u)) ||
        !Append(call801C4FA0, WordWrite(0x801C6E50u, 0u)) ||
        !Append(call801C4FA0, WordWrite(0x801C6E4Cu, 0u))) {
        return false;
    }

    Call801C4894& call80024C84 = BeginCall(
        transaction, 4u, Step801C4894::Call80024C84, 0x80024C84u);
    call80024C84.argumentKnown = true;
    call80024C84.argument = 0u;
    if (!Append(call80024C84, WordWrite(0x8008ECFAu, 0u)) ||
        !Append(call80024C84, DwordWrite(0x8008ECE4u, 0u)) ||
        !Append(call80024C84, GpDwordWrite(0x37Cu, 0u)) ||
        !Append(call80024C84, GpDwordWrite(0x320u, 0u)) ||
        !Append(call80024C84, GpDwordWrite(0x36Cu, 0u)) ||
        !Append(call80024C84, GpDwordWrite(0x370u, 0u))) {
        return false;
    }

    transaction.stepCount = kStepCount801C4894;
    transaction.accepted = true;
    transaction.complete = true;
    transaction.hostProjection = true;
    transaction.psxMemoryBackingAuthority = false;
    transaction.stage1StateOrHeaderAuthority = false;
    transaction.oldWinS0Authority = false;
    transaction.stage2PlusAuthority = false;
    transaction.replayValueAuthority = false;
    transaction.hostFilesystemAuthority = false;
    transaction.gameLiveProbeAuthority = false;
    out = transaction;
    return true;
}

} // namespace PrSS0TitleEntryPrefixDirect
