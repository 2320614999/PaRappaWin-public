#include "pr/pr_ss0_state16_runtime_envelope_import_direct.h"

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>

namespace {

std::string PayloadHex(uint8_t value) {
    constexpr char kHex[] = "0123456789abcdef";
    std::string out;
    out.reserve(PrStage1SaveCardHalDirect::kCardReadBlockBytes800179B4 * 2u);
    for (std::size_t i = 0;
         i < PrStage1SaveCardHalDirect::kCardReadBlockBytes800179B4;
         ++i) {
        out.push_back(kHex[(value >> 4) & 0x0F]);
        out.push_back(kHex[value & 0x0F]);
    }
    return out;
}

std::string ValidArgs() {
    std::ostringstream out;
    out
        << "source=duck_gdb_state16_card_read_capture.py "
        << "authority=validated_live_gdb_state16_runtime_typed_facts "
        << "factsKnown=1 state16CallKnown=1 selectedBlockKnown=1 "
        << "selectedBlockIndex=4 selectedTitleKnown=1 "
        << "selectedTitle=BISLPS-00000GDB rowCountKnown=1 rowCount=5 "
        << "arg2Known=1 arg2=16 "
        << "nameAddressKnown=1 nameAddress=0x8007CBE8 "
        << "targetBufferAddressKnown=1 targetBufferAddress=0x8007ABE8 "
        << "payloadAddressKnown=1 payloadAddress=0x8007ADE8 "
        << "blockCountKnown=1 blockCount=1 "
        << "pathCallKnown=1 cardSelectorKnown=1 cardPortGp128=0 "
        << "cardSlotGp124=0 gp696FdWriteKnown=1 gp696Fd=7 "
        << "clearEventsCallKnown=1 readSubmissionKnown=1 "
        << "readFdKnown=1 readFd=7 "
        << "readBufferAddressKnown=1 readBufferAddress=0x8007ABE8 "
        << "readByteCountKnown=1 readByteCount=8192 "
        << "pollCallKnown=1 pollEventHandlesKnown80016EB8=1 "
        << "pollEventHandle0_80016EB8=1 pollEventHandle1_80016EB8=2 "
        << "pollEventHandle2_80016EB8=3 pollEventHandle3_80016EB8=4 "
        << "pollResultKnown=1 pollResult80016EB8=1 "
        << "pollTimedOutKnown=1 pollTimedOut=0 "
        << "pollIterationCountKnown=1 pollIterationCount=1 "
        << "waitCallCountKnown80035560=1 waitCallCount80035560=1 "
        << "closeKnown=1 closeFdKnown=1 closeFd=7 "
        << "returnKnown=1 psxReturn800179B4=0 "
        << "payloadLoadCallKnown=1 payloadArgumentKnown=1 "
        << "payloadArgument=0x8007ADE8 "
        << "fullPayloadBytesKnown=1 fullPayloadByteCount=8192 "
        << "fullPayloadBytesSha256="
        << "1ae62b3110141bf43af6a7a14875442afaea8460122b814e36466febf39ca654 "
        << "fullPayloadBytesHex=" << PayloadHex(0x5Au);
    return out.str();
}

std::string ValidHeaderArgs() {
    std::ostringstream out;
    out
        << "source=duck_gdb_state16_card_read_capture.py "
        << "authority=validated_live_gdb_state16_runtime_typed_facts "
        << "factsKnown=1 state16CallKnown=1 selectedBlockKnown=1 "
        << "selectedBlockIndex=4 selectedTitleKnown=1 "
        << "selectedTitle=BISLPS-00000GDB rowCountKnown=1 rowCount=5 "
        << "arg2Known=1 arg2=16 "
        << "nameAddressKnown=1 nameAddress=0x8007CBE8 "
        << "targetBufferAddressKnown=1 targetBufferAddress=0x8007ABE8 "
        << "payloadAddressKnown=1 payloadAddress=0x8007ADE8 "
        << "blockCountKnown=1 blockCount=1 "
        << "pathCallKnown=1 cardSelectorKnown=1 cardPortGp128=0 "
        << "cardSlotGp124=0 gp696FdWriteKnown=1 gp696Fd=7 "
        << "clearEventsCallKnown=1 readSubmissionKnown=1 "
        << "readFdKnown=1 readFd=7 "
        << "readBufferAddressKnown=1 readBufferAddress=0x8007ABE8 "
        << "readByteCountKnown=1 readByteCount=8192 "
        << "pollCallKnown=1 pollEventHandlesKnown80016EB8=1 "
        << "pollEventHandle0_80016EB8=1 pollEventHandle1_80016EB8=2 "
        << "pollEventHandle2_80016EB8=3 pollEventHandle3_80016EB8=4 "
        << "pollResultKnown=1 pollResult80016EB8=1 "
        << "pollTimedOutKnown=1 pollTimedOut=0 "
        << "pollIterationCountKnown=1 pollIterationCount=1 "
        << "waitCallCountKnown80035560=1 waitCallCount80035560=1 "
        << "closeKnown=1 closeFdKnown=1 closeFd=7 "
        << "returnKnown=1 psxReturn800179B4=0 "
        << "payloadLoadCallKnown=1 payloadArgumentKnown=1 "
        << "payloadArgument=0x8007ADE8 "
        << "fullPayloadBytesKnown=1 fullPayloadByteCount=8192 "
        << "fullPayloadBytesSha256="
        << "1ae62b3110141bf43af6a7a14875442afaea8460122b814e36466febf39ca654";
    return out.str();
}

void Check(bool condition, const char* message) {
    if (!condition) {
        std::cerr << message << "\n";
        std::exit(1);
    }
}

void TestValidEnvelopeParsesImportableRuntimeFacts() {
    PrSS0State16RuntimeEnvelopeImportDirect::State16RuntimeTypedFactsEnvelope
        envelope{};
    std::string error;
    Check(PrSS0State16RuntimeEnvelopeImportDirect::
              ParseValidatedLiveGdbState16RuntimeEnvelopeArgs(
                  ValidArgs(),
                  envelope,
                  error),
          error.c_str());
    Check(envelope.facts.selectedBlockIndex == 4, "selected block mismatch");
    Check(envelope.facts.selectedTitleKnown, "selected title not known");
    Check(std::string(envelope.facts.selectedTitle) == "BISLPS-00000GDB",
          "selected title mismatch");
    Check(envelope.facts.rowCount == 5, "row count mismatch");
    Check(envelope.facts.pollEventHandlesKnown80016EB8,
          "poll event handles not known");
    Check(envelope.facts.pollEventHandle0_80016EB8 == 1,
          "poll event handle0 mismatch");
    Check(envelope.facts.fullPayloadBytes == envelope.fullPayloadBytes.data(),
          "payload pointer must reference owned envelope storage");
    Check(PrStage1SaveCardHalDirect::
              IsImportableState16RuntimeTypedFacts800179B4(
                  envelope.facts,
                  4),
          "parsed facts must satisfy HAL importable predicate");
}

void TestRejectsWrongAuthority() {
    std::string args = ValidArgs();
    const std::string from =
        "authority=validated_live_gdb_state16_runtime_typed_facts";
    const std::size_t pos = args.find(from);
    Check(pos != std::string::npos, "authority token missing");
    args.replace(pos, from.size(), "authority=DebugSyntheticFixture");
    PrSS0State16RuntimeEnvelopeImportDirect::State16RuntimeTypedFactsEnvelope
        envelope{};
    std::string error;
    Check(!PrSS0State16RuntimeEnvelopeImportDirect::
               ParseValidatedLiveGdbState16RuntimeEnvelopeArgs(
                   args,
                   envelope,
                   error),
          "wrong authority should reject");
}

void TestRejectsPayloadShaMismatch() {
    std::string args = ValidArgs();
    const std::string from =
        "1ae62b3110141bf43af6a7a14875442afaea8460122b814e36466febf39ca654";
    const std::size_t pos = args.find(from);
    Check(pos != std::string::npos, "sha token missing");
    args.replace(pos,
                 from.size(),
                 "9f1dcbc35c350d6027f98be0f5c8b43b42ca52b7604459c0c42be3aa88913d47");
    PrSS0State16RuntimeEnvelopeImportDirect::State16RuntimeTypedFactsEnvelope
        envelope{};
    std::string error;
    Check(!PrSS0State16RuntimeEnvelopeImportDirect::
               ParseValidatedLiveGdbState16RuntimeEnvelopeArgs(
                   args,
                   envelope,
                   error),
          "sha mismatch should reject");
}

void TestRejectsNonSuccessPoll() {
    std::string args = ValidArgs();
    const std::string from = "pollResult80016EB8=1";
    const std::size_t pos = args.find(from);
    Check(pos != std::string::npos, "poll token missing");
    args.replace(pos, from.size(), "pollResult80016EB8=2");
    PrSS0State16RuntimeEnvelopeImportDirect::State16RuntimeTypedFactsEnvelope
        envelope{};
    std::string error;
    Check(!PrSS0State16RuntimeEnvelopeImportDirect::
               ParseValidatedLiveGdbState16RuntimeEnvelopeArgs(
                   args,
                   envelope,
                   error),
          "non-success poll should reject");
}

void TestRejectsMissingSelectedTitle() {
    std::string args = ValidArgs();
    const std::string from = " selectedTitleKnown=1 selectedTitle=BISLPS-00000GDB";
    const std::size_t pos = args.find(from);
    Check(pos != std::string::npos, "selected title token missing");
    args.erase(pos, from.size());
    PrSS0State16RuntimeEnvelopeImportDirect::State16RuntimeTypedFactsEnvelope
        envelope{};
    std::string error;
    Check(!PrSS0State16RuntimeEnvelopeImportDirect::
               ParseValidatedLiveGdbState16RuntimeEnvelopeArgs(
                   args,
                   envelope,
                   error),
          "missing selected title should reject");
}

void TestStagedEnvelopeCommitParsesImportableRuntimeFacts() {
    PrSS0State16RuntimeEnvelopeImportDirect::State16RuntimeEnvelopeStaging
        staging{};
    std::string error;
    Check(PrSS0State16RuntimeEnvelopeImportDirect::
              BeginStagedValidatedLiveGdbState16RuntimeEnvelopeArgs(
                  staging,
                  ValidHeaderArgs(),
                  error),
          error.c_str());
    Check(staging.active, "staging must be active after begin");
    Check(PrSS0State16RuntimeEnvelopeImportDirect::
              AppendStagedValidatedLiveGdbState16RuntimeEnvelopeChunk(
                  staging,
                  "offset=0 hex=" + PayloadHex(0x5Au),
                  error),
          error.c_str());
    PrSS0State16RuntimeEnvelopeImportDirect::State16RuntimeTypedFactsEnvelope
        envelope{};
    Check(PrSS0State16RuntimeEnvelopeImportDirect::
              CommitStagedValidatedLiveGdbState16RuntimeEnvelope(
                  staging,
                  envelope,
                  error),
          error.c_str());
    Check(!staging.active, "commit must clear staging");
    Check(envelope.facts.fullPayloadBytes == envelope.fullPayloadBytes.data(),
          "staged payload pointer must reference owned envelope storage");
    Check(PrStage1SaveCardHalDirect::
              IsImportableState16RuntimeTypedFacts800179B4(
                  envelope.facts,
                  4),
          "staged facts must satisfy HAL importable predicate");
}

void TestStagedBeginRejectsInlinePayloadHex() {
    PrSS0State16RuntimeEnvelopeImportDirect::State16RuntimeEnvelopeStaging
        staging{};
    std::string error;
    Check(!PrSS0State16RuntimeEnvelopeImportDirect::
               BeginStagedValidatedLiveGdbState16RuntimeEnvelopeArgs(
                   staging,
                   ValidArgs(),
                   error),
          "staged begin should reject inline payload hex");
    Check(!staging.active, "failed begin must leave staging inactive");
}

void TestStagedChunkRejectsOverlap() {
    PrSS0State16RuntimeEnvelopeImportDirect::State16RuntimeEnvelopeStaging
        staging{};
    std::string error;
    Check(PrSS0State16RuntimeEnvelopeImportDirect::
              BeginStagedValidatedLiveGdbState16RuntimeEnvelopeArgs(
                  staging,
                  ValidHeaderArgs(),
                  error),
          error.c_str());
    Check(PrSS0State16RuntimeEnvelopeImportDirect::
              AppendStagedValidatedLiveGdbState16RuntimeEnvelopeChunk(
                  staging,
                  "offset=0 hex=5a5a",
                  error),
          error.c_str());
    Check(!PrSS0State16RuntimeEnvelopeImportDirect::
               AppendStagedValidatedLiveGdbState16RuntimeEnvelopeChunk(
                   staging,
                   "offset=1 hex=5a5a",
                   error),
          "overlapping staged chunk should reject");
}

void TestStagedIncompleteCommitClears() {
    PrSS0State16RuntimeEnvelopeImportDirect::State16RuntimeEnvelopeStaging
        staging{};
    std::string error;
    Check(PrSS0State16RuntimeEnvelopeImportDirect::
              BeginStagedValidatedLiveGdbState16RuntimeEnvelopeArgs(
                  staging,
                  ValidHeaderArgs(),
                  error),
          error.c_str());
    Check(PrSS0State16RuntimeEnvelopeImportDirect::
              AppendStagedValidatedLiveGdbState16RuntimeEnvelopeChunk(
                  staging,
                  "offset=0 hex=5a5a",
                  error),
          error.c_str());
    PrSS0State16RuntimeEnvelopeImportDirect::State16RuntimeTypedFactsEnvelope
        envelope{};
    Check(!PrSS0State16RuntimeEnvelopeImportDirect::
               CommitStagedValidatedLiveGdbState16RuntimeEnvelope(
                   staging,
                   envelope,
                   error),
          "incomplete staged commit should reject");
    Check(!staging.active, "failed incomplete commit must clear staging");
}

}  // namespace

int main() {
    TestValidEnvelopeParsesImportableRuntimeFacts();
    TestRejectsWrongAuthority();
    TestRejectsPayloadShaMismatch();
    TestRejectsNonSuccessPoll();
    TestRejectsMissingSelectedTitle();
    TestStagedEnvelopeCommitParsesImportableRuntimeFacts();
    TestStagedBeginRejectsInlinePayloadHex();
    TestStagedChunkRejectsOverlap();
    TestStagedIncompleteCommitClears();
    std::cout << "test_ss0_state16_runtime_envelope_import: ok\n";
    return 0;
}
