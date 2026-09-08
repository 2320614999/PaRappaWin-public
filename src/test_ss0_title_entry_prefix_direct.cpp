#include "pr/pr_ss0_title_entry_prefix_direct.h"

#include <array>
#include <cstdio>

namespace {

using namespace PrSS0TitleEntryPrefixDirect;

int g_failed = 0;

#define CHECK(expr)                                                           \
    do {                                                                      \
        if (!(expr)) {                                                        \
            std::printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr);       \
            ++g_failed;                                                       \
        }                                                                     \
    } while (0)

bool SameOperation(const Operation801C4894& left,
                   const Operation801C4894& right)
{
    return left.kind == right.kind && left.target == right.target &&
           left.rangeEnd == right.rangeEnd &&
           left.byteCount == right.byteCount && left.value == right.value;
}

bool SameCall(const Call801C4894& left, const Call801C4894& right)
{
    if (left.step != right.step ||
        left.functionAddress != right.functionAddress ||
        left.argumentKnown != right.argumentKnown ||
        left.argument != right.argument ||
        left.staticReturnKnown != right.staticReturnKnown ||
        left.staticReturn != right.staticReturn ||
        left.operationCount != right.operationCount) {
        return false;
    }
    for (std::size_t index = 0u; index < left.operations.size(); ++index) {
        if (!SameOperation(left.operations[index], right.operations[index])) {
            return false;
        }
    }
    return true;
}

bool SameTransaction(const Transaction801C4894& left,
                     const Transaction801C4894& right)
{
    if (left.accepted != right.accepted ||
        left.complete != right.complete ||
        left.hostProjection != right.hostProjection ||
        left.psxMemoryBackingAuthority != right.psxMemoryBackingAuthority ||
        left.stage1StateOrHeaderAuthority !=
            right.stage1StateOrHeaderAuthority ||
        left.oldWinS0Authority != right.oldWinS0Authority ||
        left.stage2PlusAuthority != right.stage2PlusAuthority ||
        left.replayValueAuthority != right.replayValueAuthority ||
        left.hostFilesystemAuthority != right.hostFilesystemAuthority ||
        left.gameLiveProbeAuthority != right.gameLiveProbeAuthority ||
        left.stepCount != right.stepCount ||
        left.stepOrder != right.stepOrder) {
        return false;
    }
    for (std::size_t index = 0u; index < left.calls.size(); ++index) {
        if (!SameCall(left.calls[index], right.calls[index])) {
            return false;
        }
    }
    return true;
}

Source801C4894 MakeSource(uint16_t word800916DC)
{
    Source801C4894 source{};
    source.contextKnown = true;
    source.contextAddress = kContextAddress801C3640;
    source.scene0TablePointerKnown = true;
    source.scene0TablePointer = kScene0TablePointer801C6DA4;
    source.word800916DCKnown = true;
    source.word800916DC = word800916DC;
    return source;
}

void ExpectDefault(const Transaction801C4894& transaction)
{
    CHECK(SameTransaction(transaction, Transaction801C4894{}));
}

void ExpectReject(const Source801C4894& source)
{
    Transaction801C4894 transaction{};
    CHECK(BuildTransaction801C4894(MakeSource(1u), transaction));
    CHECK(transaction.accepted);
    CHECK(!BuildTransaction801C4894(source, transaction));
    ExpectDefault(transaction);
}

void ExpectCall(const Call801C4894& call,
                Step801C4894 step,
                uint32_t functionAddress,
                bool argumentKnown = false,
                uint32_t argument = 0u,
                bool staticReturnKnown = false,
                int32_t staticReturn = 0)
{
    CHECK(call.step == step);
    CHECK(call.functionAddress == functionAddress);
    CHECK(call.argumentKnown == argumentKnown);
    CHECK(call.argument == argument);
    CHECK(call.staticReturnKnown == staticReturnKnown);
    CHECK(call.staticReturn == staticReturn);
}

void ExpectZeroRange(const Operation801C4894& operation,
                     uint32_t begin,
                     uint32_t end,
                     uint32_t byteCount)
{
    CHECK(operation.kind == OperationKind801C4894::ZeroRange);
    CHECK(operation.target == begin);
    CHECK(operation.rangeEnd == end);
    CHECK(operation.byteCount == byteCount);
    CHECK(operation.value == 0u);
}

void ExpectWordWrite(const Operation801C4894& operation,
                     uint32_t address,
                     uint16_t value)
{
    CHECK(operation.kind == OperationKind801C4894::WordWrite);
    CHECK(operation.target == address);
    CHECK(operation.rangeEnd == 0u);
    CHECK(operation.byteCount == 0u);
    CHECK(operation.value == value);
}

void ExpectDwordWrite(const Operation801C4894& operation,
                      uint32_t address,
                      uint32_t value)
{
    CHECK(operation.kind == OperationKind801C4894::DwordWrite);
    CHECK(operation.target == address);
    CHECK(operation.rangeEnd == 0u);
    CHECK(operation.byteCount == 0u);
    CHECK(operation.value == value);
}

void ExpectGpDwordWrite(const Operation801C4894& operation,
                        uint32_t offset,
                        uint32_t value)
{
    CHECK(operation.kind == OperationKind801C4894::GpDwordWrite);
    CHECK(operation.target == offset);
    CHECK(operation.rangeEnd == 0u);
    CHECK(operation.byteCount == 0u);
    CHECK(operation.value == value);
}

bool HasGpWrite(const Call801C4894& call, uint32_t offset)
{
    for (std::size_t index = 0u; index < call.operationCount; ++index) {
        const auto& operation = call.operations[index];
        if (operation.kind == OperationKind801C4894::GpDwordWrite &&
            operation.target == offset) {
            return true;
        }
    }
    return false;
}

void ExpectAcceptedTransaction(const Transaction801C4894& transaction,
                               uint16_t word800916DC)
{
    CHECK(transaction.accepted);
    CHECK(transaction.complete);
    CHECK(transaction.hostProjection);
    CHECK(!transaction.psxMemoryBackingAuthority);
    CHECK(!transaction.stage1StateOrHeaderAuthority);
    CHECK(!transaction.oldWinS0Authority);
    CHECK(!transaction.stage2PlusAuthority);
    CHECK(!transaction.replayValueAuthority);
    CHECK(!transaction.hostFilesystemAuthority);
    CHECK(!transaction.gameLiveProbeAuthority);
    CHECK(transaction.stepCount == kStepCount801C4894);

    const std::array<Step801C4894, kStepCount801C4894> expectedOrder = {
        Step801C4894::Call80014344,
        Step801C4894::Call80024E98,
        Step801C4894::Call80024FC0,
        Step801C4894::Call801C4FA0,
        Step801C4894::Call80024C84,
    };
    CHECK(transaction.stepOrder == expectedOrder);

    const auto& reset = transaction.calls[0];
    ExpectCall(reset, Step801C4894::Call80014344, 0x80014344u);
    CHECK(reset.operationCount == 15u);
    ExpectGpDwordWrite(reset.operations[0], 0x1Cu, 0x80092910u);
    ExpectZeroRange(
        reset.operations[1], 0x80092910u, 0x80092F10u, 0x600u);
    ExpectZeroRange(
        reset.operations[2], 0x80091800u, 0x8009182Cu, 0x2Cu);
    ExpectWordWrite(reset.operations[3], 0x80091810u, 0u);
    ExpectWordWrite(reset.operations[4], 0x80091812u, 0u);
    ExpectWordWrite(reset.operations[5], 0x80091814u, 0u);
    ExpectWordWrite(reset.operations[6], 0x80091816u, 0u);
    ExpectWordWrite(reset.operations[7], 0x80091818u, 0u);
    ExpectWordWrite(reset.operations[8], 0x8009181Au, 0u);
    ExpectWordWrite(reset.operations[9], 0x8009181Cu, 0u);
    ExpectWordWrite(reset.operations[10], 0x80091822u, 0u);
    ExpectDwordWrite(reset.operations[11], 0x8009180Cu, 0u);
    ExpectDwordWrite(reset.operations[12], 0x80091808u, 0u);
    ExpectGpDwordWrite(reset.operations[13], 0x20u, 0u);
    ExpectDwordWrite(reset.operations[14], 0x80091800u, 0u);

    const auto& eventReset = transaction.calls[1];
    ExpectCall(eventReset, Step801C4894::Call80024E98, 0x80024E98u);
    CHECK(eventReset.operationCount == 26u);
    ExpectZeroRange(
        eventReset.operations[0], 0x8008ECE0u, 0x8008EDD4u, 0xF4u);
    ExpectDwordWrite(eventReset.operations[1], 0x8008ED20u, 0u);
    ExpectDwordWrite(eventReset.operations[2], 0x8008ED00u, 0u);
    ExpectWordWrite(eventReset.operations[3], 0x8008ED2Eu, 1u);
    ExpectWordWrite(eventReset.operations[4], 0x8008ED2Cu, 0u);
    ExpectGpDwordWrite(eventReset.operations[5], 0x320u, 0u);
    ExpectZeroRange(
        eventReset.operations[6], 0x801C3640u, 0x801C3828u, 0x1E8u);
    ExpectWordWrite(eventReset.operations[7], 0x801C368Eu, 1u);
    ExpectWordWrite(eventReset.operations[8], 0x801C3690u, 0u);
    ExpectWordWrite(eventReset.operations[9], 0x801C3692u, 0u);
    ExpectWordWrite(eventReset.operations[10], 0x801C369Au, 0u);
    ExpectWordWrite(eventReset.operations[11], 0x801C36AAu, 1u);
    ExpectWordWrite(eventReset.operations[12], 0x801C36BAu, 1u);
    ExpectWordWrite(eventReset.operations[13], 0x801C36CAu, 0u);
    ExpectWordWrite(eventReset.operations[14], 0x801C36D0u, 0u);
    ExpectWordWrite(eventReset.operations[15], 0x801C36E2u, 0u);
    ExpectDwordWrite(
        eventReset.operations[16], 0x801C36D4u, 0x801C6DA8u);
    ExpectDwordWrite(
        eventReset.operations[17], 0x801C36D8u, 0x801C6DBCu);
    ExpectWordWrite(
        eventReset.operations[18], 0x801C36A8u, word800916DC);
    ExpectWordWrite(
        eventReset.operations[19], 0x801C3694u, word800916DC);
    ExpectWordWrite(eventReset.operations[20], 0x8008ED36u, 2u);
    ExpectWordWrite(eventReset.operations[21], 0x8008ED38u, 0u);
    ExpectWordWrite(eventReset.operations[22], 0x801C369Au, 0u);
    ExpectWordWrite(eventReset.operations[23], 0x80091818u, 0u);
    ExpectWordWrite(eventReset.operations[24], 0x8009181Au, 0u);
    ExpectWordWrite(eventReset.operations[25], 0x8009181Cu, 0u);

    const auto& mirror = transaction.calls[2];
    ExpectCall(mirror,
               Step801C4894::Call80024FC0,
               0x80024FC0u,
               true,
               kContextAddress801C3640);
    CHECK(mirror.operationCount == 1u);
    ExpectDwordWrite(mirror.operations[0], 0x801C3670u, 0u);

    const auto& staticReset = transaction.calls[3];
    ExpectCall(staticReset,
               Step801C4894::Call801C4FA0,
               0x801C4FA0u,
               false,
               0u,
               true,
               -12);
    CHECK(staticReset.operationCount == 4u);
    ExpectWordWrite(staticReset.operations[0], 0x801C6E58u, 0u);
    ExpectWordWrite(staticReset.operations[1], 0x801C6E54u, 0u);
    ExpectWordWrite(staticReset.operations[2], 0x801C6E50u, 0u);
    ExpectWordWrite(staticReset.operations[3], 0x801C6E4Cu, 0u);

    const auto& finalReset = transaction.calls[4];
    ExpectCall(finalReset,
               Step801C4894::Call80024C84,
               0x80024C84u,
               true,
               0u);
    CHECK(finalReset.operationCount == 6u);
    ExpectWordWrite(finalReset.operations[0], 0x8008ECFAu, 0u);
    ExpectDwordWrite(finalReset.operations[1], 0x8008ECE4u, 0u);
    ExpectGpDwordWrite(finalReset.operations[2], 0x37Cu, 0u);
    ExpectGpDwordWrite(finalReset.operations[3], 0x320u, 0u);
    ExpectGpDwordWrite(finalReset.operations[4], 0x36Cu, 0u);
    ExpectGpDwordWrite(finalReset.operations[5], 0x370u, 0u);
    CHECK(!HasGpWrite(finalReset, 0x364u));
    CHECK(!HasGpWrite(finalReset, 0x374u));
}

void TestRejectsUnknownAndWrongSources()
{
    ExpectReject(Source801C4894{});

    Source801C4894 source = MakeSource(0u);
    source.contextKnown = false;
    ExpectReject(source);

    source = MakeSource(0u);
    source.scene0TablePointerKnown = false;
    ExpectReject(source);

    source = MakeSource(0u);
    source.word800916DCKnown = false;
    ExpectReject(source);

    source = MakeSource(0u);
    source.contextAddress = kContextAddress801C3640 + 4u;
    ExpectReject(source);

    source = MakeSource(0u);
    source.scene0TablePointer = kScene0TablePointer801C6DA4 + 4u;
    ExpectReject(source);

    source = MakeSource(2u);
    ExpectReject(source);
}

void TestAcceptedValuesAndIdempotence()
{
    Transaction801C4894 word0{};
    CHECK(BuildTransaction801C4894(MakeSource(0u), word0));
    ExpectAcceptedTransaction(word0, 0u);

    Transaction801C4894 word1{};
    CHECK(BuildTransaction801C4894(MakeSource(1u), word1));
    ExpectAcceptedTransaction(word1, 1u);

    Transaction801C4894 repeated = word0;
    CHECK(BuildTransaction801C4894(MakeSource(1u), repeated));
    CHECK(SameTransaction(repeated, word1));
    CHECK(BuildTransaction801C4894(MakeSource(1u), repeated));
    CHECK(SameTransaction(repeated, word1));
}

} // namespace

int main()
{
    TestRejectsUnknownAndWrongSources();
    TestAcceptedValuesAndIdempotence();
    if (g_failed != 0) {
        std::printf("test_ss0_title_entry_prefix_direct: %d failed\n",
                    g_failed);
        return 1;
    }
    std::printf("test_ss0_title_entry_prefix_direct: ok\n");
    return 0;
}
