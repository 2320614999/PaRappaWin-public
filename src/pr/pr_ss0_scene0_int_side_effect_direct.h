#pragma once

#include "pr_ss0_scene0_int_load_direct.h"
#include "pr_stage1_loader_memory_direct.h"

#include <array>
#include <cstdint>
#include <memory>
#include <vector>

namespace PrSS0Scene0IntSideEffectDirect {

constexpr uint32_t kScene0PacketArenaLowWater801C4260 = 0x801AE430u;
constexpr uint32_t kVabVhHandle8001A8F0 = 1u;
constexpr uint32_t kMemBaseHandle8001A8F0 = 2u;
constexpr uint32_t kFinalHandle8001A8F0 = 60u;

enum class Status8001A8F0 : uint8_t {
    SourceUnknown = 0,
    IntLoadRejected,
    ResetStateMalformed,
    BlockMetadataMalformed,
    AllocationFailed,
    PayloadRangeInvalid,
    TimRequestMalformed,
    VabRequestMalformed,
    MemMapFailed,
    CandidateShapeMismatch,
    Accepted,
};

struct PayloadRequest8001A8F0 {
    uint32_t blockIndex = 0u;
    uint32_t entryIndex = 0u;
    PrSS0Scene0IntLoadDirect::BlockType8001A8F0 type =
        PrSS0Scene0IntLoadDirect::BlockType8001A8F0::Eof;
    uint32_t psxAddress = 0u;
    uint32_t size = 0u;
    std::array<uint8_t, 16> name{};
    uint64_t archiveDataOffset = 0u;
};

struct VabRequest8001A8F0 {
    bool known = false;
    uint32_t vhPsxAddress = 0u;
    uint32_t vhSize = 0u;
    std::array<uint8_t, 16> vhName{};
    uint32_t vbPsxAddress = 0u;
    uint32_t vbSize = 0u;
    std::array<uint8_t, 16> vbName{};
    uint32_t sectorPayloadBytes = 0u;
    bool closeRequested80027120 = false;
    bool openRequested80027078 = false;
    bool transferRequested800270D4 = false;
    bool enableRequested800270FC = false;
};

struct FinalResultSource8001AC18 {
    bool staticEofReturnChainKnown = false;
    bool eofSuccessKnown = false;
    bool eofSuccess = false;
    bool loaderMemoryCommitted = false;
    bool memHandleTableCommitted = false;
    bool timPartialVramCommitted = false;
    bool hostVabBankCommitted = false;
};

struct Transaction8001A8F0 {
    Status8001A8F0 status = Status8001A8F0::SourceUnknown;
    bool accepted = false;
    bool complete = false;
    bool sourceIntLoadKnown = false;
    PrSS0Scene0IntLoadDirect::ArchiveKind8001AC18 sourceArchiveKind =
        PrSS0Scene0IntLoadDirect::ArchiveKind8001AC18::Unknown;
    uint32_t sourceRequestByteCount = 0u;

    bool resetStateKnown = false;
    bool resetStateExact = false;
    bool loaderMemoryCandidateKnown = false;
    std::unique_ptr<PrStage1LoaderMemoryDirectState>
        loaderMemoryCandidate{};

    bool timTemporaryAllocationKnown = false;
    uint32_t timTemporaryPsxAddress = 0u;
    uint32_t timTemporaryHandle = 0u;
    uint32_t timSectorPayloadBytes = 0u;
    bool timTemporaryAllocationPopped = false;
    std::vector<PayloadRequest8001A8F0> timUploadRequests{};

    VabRequest8001A8F0 vabRequest{};
    bool vabVhHandleRetained = false;
    bool vabVbHandlePopped = false;

    bool memBlockAllocationKnown = false;
    uint32_t memBlockPsxAddress = 0u;
    uint32_t memBlockBaseHandle = 0u;
    uint32_t memSectorPayloadBytes = 0u;
    std::vector<PayloadRequest8001A8F0> memMapRequests{};
    uint32_t finalStackDepth = 0u;
    uint32_t finalHeapCursor = 0u;

    bool loaderMemoryCommitted = false;
    bool timVramCommitted = false;
    bool vabSpuCommitted = false;
    bool memHandleTableCommitted = false;
    bool sideEffectsCommitted = false;
    bool final8001AC18ResultKnown = false;
    int32_t final8001AC18Result = 0;
    bool final8001AC18ResultAuthority = false;
    bool replayValueAuthority = false;
    bool oldWinS0Authority = false;
    bool hostFilesystemAuthority = false;
    bool consumerReadAuthority = false;
    bool psxMemoryBackingAuthority = false;
    bool stage2PlusAuthority = false;
};

Transaction8001A8F0 BuildTransaction8001A8F0(
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& intLoad,
    std::unique_ptr<PrStage1LoaderMemoryDirectState> resetState);

bool IsExactAcceptedTransaction8001A8F0(
    const Transaction8001A8F0& transaction,
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& intLoad);

Transaction8001A8F0 BuildYCompoTransaction8001A8F0(
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& intLoad,
    std::unique_ptr<PrStage1LoaderMemoryDirectState> resetState);

// Startup COMMON/ZCOMPO use the same 8001A8F0 loader body as COMPO00, but
// have no MEM block and carry different TIM counts.  These builders keep the
// archive kind explicit so their TIM/VAB software side effects can be
// committed before the two boot logos without borrowing a title shell.
Transaction8001A8F0 BuildStartupCommonTransaction80016B84(
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& intLoad,
    std::unique_ptr<PrStage1LoaderMemoryDirectState> resetState);

Transaction8001A8F0 BuildStartupZCompoTransaction80015590(
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& intLoad,
    std::unique_ptr<PrStage1LoaderMemoryDirectState> resetState);

bool IsExactAcceptedYCompoTransaction8001A8F0(
    const Transaction8001A8F0& transaction,
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& intLoad);

bool IsExactAcceptedStartupCommonTransaction80016B84(
    const Transaction8001A8F0& transaction,
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& intLoad);

bool IsExactAcceptedStartupZCompoTransaction80015590(
    const Transaction8001A8F0& transaction,
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& intLoad);

bool CommitYCompoLoaderMemory8001A8F0(
    Transaction8001A8F0& transaction,
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& intLoad);

bool IsExactYCompoLoaderMemoryCommittedTransaction8001A8F0(
    const Transaction8001A8F0& transaction,
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& intLoad);

bool CommitLoaderMemoryAndHandleTable8001A8F0(
    Transaction8001A8F0& transaction,
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& intLoad);

bool IsExactLoaderMemoryCommittedTransaction8001A8F0(
    const Transaction8001A8F0& transaction,
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& intLoad);

bool CommitFinalResult8001AC18(
    Transaction8001A8F0& transaction,
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& intLoad,
    const FinalResultSource8001AC18& source);

bool IsExactFinalResultTransaction8001AC18(
    const Transaction8001A8F0& transaction,
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& intLoad);

}  // namespace PrSS0Scene0IntSideEffectDirect
