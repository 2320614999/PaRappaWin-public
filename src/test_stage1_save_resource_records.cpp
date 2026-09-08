#include "pr/pr_stage1_loader_direct.h"
#include "pr/pr_stage1_loader_gpu_hal.h"
#include "pr/pr_stage1_loading_direct.h"
#include "pr/pr_vram_atlas.h"
#include "int_loader.h"
#include <cassert>
#include <algorithm>
#include <cstring>
#include <fstream>
#include <iterator>
#include <iostream>
#include <memory>

namespace L = PrStage1LoaderDirect;
namespace G = PrStage1LoaderGpuHal;

int main(int argc, char** argv) {
    {
        namespace Loading = PrStage1LoadingDirect;
        Loading::State state;
        assert(!Loading::BeginAfterReset8001EF14(state, 3));
        std::array<uint8_t, 192> grid{};
        grid.fill(1u); // native preceding 8001FFD4(2)
        assert(!Loading::Begin(state, 3, nullptr, grid.size()));
        assert(Loading::Begin(state, 3, grid.data(), grid.size()));
        assert(Loading::Tick(state, 100));
        assert(state.frame.style == 2u && state.frame.highlightCount > 0u);
        const auto firstMask = state.frame.liveGrid;
        assert(!Loading::Tick(state, 100));
        assert(state.callbacks == 1u && state.frame.liveGrid == firstMask);
        assert(!Loading::MinimumPresentationComplete(state, 1000));
        for (uint32_t tick = 100; tick < 130; ++tick) {
            Loading::Tick(state, tick);
            Loading::RecordPresentation(state, tick);
            Loading::RecordPresentation(state, tick); // 60 Hz redraw
            assert(!Loading::MinimumPresentationComplete(state, tick));
        }
        assert(state.presentedTicks == 30u);
        assert(Loading::MinimumPresentationComplete(state, 130));
        Loading::Stop(state);
        assert(!state.pattern.active && !state.frame.known);
        assert(!Loading::MinimumPresentationComplete(state, 130));
        assert(!Loading::Tick(state, 130));
        const auto oldCount = state.pattern.frameCounterGp51;
        grid.fill(0u);
        assert(Loading::Begin(state, 2, grid.data(), grid.size()));
        assert(state.pattern.frameCounterGp51 == oldCount);
        assert(!Loading::MinimumPresentationComplete(state, 2000));
        assert(Loading::Tick(state, 2000) && state.frame.style == 1u);
        PrSS0TransitionDirect::ResetLoadingPatternState8001EF14(state.pattern);
        assert(state.pattern.frameCounterGp51 == 0u);
        // Actual 801C81EC order: post-movie transition -> 8001EF14 ->
        // 80015590. The movie's temporary mask has already been released.
        assert(Loading::BeginAfterReset8001EF14(state, 3));
        for (const auto cell : state.pattern.liveGrid) assert(cell == 0u);
        assert(Loading::Tick(state, 3000));
        // Callback zero immediately shifts the first source column. The
        // native style-2 table has seven leading set bits, not a blank frame.
        assert(state.frame.style == 2u && state.frame.highlightCount == 7u);
        const auto fullGrid = PrSS0TransitionDirect::ResolveSlowTransitionVisualFrame80020110(
            PrSS0TransitionDirect::kScene0WorkAddress, 2, 1, 2, false, 23u);
        const auto draw = PrSS0TransitionDirect::BuildSlowTransitionFramePlan8001FDC0(fullGrid);
        assert(draw.known && !draw.truncated && draw.commandCount == 192u);
        std::cout << "PASS: native Loading mask/style, single callback per logic tick, visible one-second floor, stop/reentry\n";
    }
    assert(argc == 4);
    std::ifstream file(argv[1], std::ios::binary);
    std::vector<uint8_t> bytes((std::istreambuf_iterator<char>(file)), {});
    assert(bytes.size() == 643072u);
    L::Bootstrap15590Plan plan{};
    plan.valid = true;
    plan.parsePlan1A8F0.valid = true;
    L::RunnerState runner{};
    runner.plan = plan;
    runner.planKnown = true;
    const size_t offsets[] = {0u, 458752u, 567296u, 634880u};
    const size_t counts[] = {408u, 173u, 2u, 0u};
    size_t timCount = 0;
    PsxVramAtlas saveAtlas;
    // Original 80016B84 loads COMMON before the scene loop; scene resource
    // replacement does not erase those VRAM words. Test the real CPU atlas
    // with the startup upload followed by the stage COMPO before ZCOMPO.
    for (int source = 2; source < 4; ++source) {
        IntArchive archive;
        assert(IntLoader::Load(argv[source], archive));
        for (const auto& entry : archive.entries) {
            if (entry.type == IntBlockType::Tim)
                assert(saveAtlas.LoadTim(entry.data.data(), entry.data.size(), entry.name, true, false));
        }
    }
    for (uint16_t i = 0; i < 4; ++i) {
        L::ResolvedPayload header{};
        header.valid = true;
        header.recordIndex = i;
        header.recordType = L::LoaderRecordType::Unknown;
        header.liveBytesPresent = header.liveBytesSizeKnown = true;
        header.liveBytesData = bytes.data() + offsets[i];
        header.liveBytesSize = 8192u;
        runner.currentResolvedPayloadKnown = true;
        runner.currentResolvedPayload = header;
        runner.resolvedPayloads.push_back(header);
        L::RecordData record{};
        assert(L::TryBuildRecordDataFromDescriptorPayload(runner, i, record));
        assert(record.entries.size() == counts[i]);
        assert(record.recordCount == counts[i]);
        if (i < 2) {
            G::TimPayloadView payload{};
            payload.data = bytes.data() + offsets[i] + 8192u;
            payload.size = record.sectorCount * 2048u;
            std::vector<G::TimRecordUpload8001A8F0> uploads;
            assert(G::BuildTimRecordUploads8001A8F0(payload, record.entries, uploads));
            assert(uploads.size() == counts[i]);
            size_t cursor = 0;
            for (size_t n = 0; n < uploads.size(); ++n) {
                assert(uploads[n].name == record.entries[n].name);
                assert(uploads[n].bytes.size() == record.entries[n].size);
                assert(std::memcmp(uploads[n].bytes.data(), payload.data + cursor,
                                   uploads[n].bytes.size()) == 0);
                const auto nameEnd = std::find(uploads[n].name.begin(), uploads[n].name.end(), '\0');
                assert(saveAtlas.LoadTim(uploads[n].bytes.data(), uploads[n].bytes.size(),
                    std::string(uploads[n].name.begin(), nameEnd), true, false));
                cursor += record.entries[n].size;
            }
            timCount += uploads.size();
            auto broken = record.entries;
            broken.back().size = UINT32_MAX;
            assert(!G::BuildTimRecordUploads8001A8F0(payload, broken, uploads));
            assert(uploads.empty());
        }
        if (i == 3) assert(record.recordType == L::LoaderRecordType::End);
    }
    assert(timCount == 581);
    for (const uint32_t address : {0x800503E0u, 0x800503F0u, 0x80050400u, 0x80050410u}) {
        const auto tpl = PrSS0TransitionDirect::ResolveFastTransitionSpriteTemplate800201AC(address);
        assert(tpl.known);
        const uint16_t tpage = ((tpl.texY & 0x100u) >> 4u) | ((tpl.texX >> 6u) & 0xFu);
        const uint16_t clut = (tpl.clutY << 6u) | (tpl.clutX >> 4u);
        std::vector<uint32_t> rgba;
        const bool copied = saveAtlas.CopyRgbaRect(tpage, clut,
            static_cast<uint8_t>(tpl.texX * 4u), static_cast<uint8_t>(tpl.texY),
            tpl.width, tpl.height, rgba);
        const auto visible = std::count_if(rgba.begin(), rgba.end(),
            [](uint32_t pixel) { return (pixel >> 24u) != 0u; });
        std::cout << "Save role tile " << std::hex << address << std::dec
            << " resolve=" << saveAtlas.CanResolveTpageClut(tpage, clut)
            << " copy=" << copied << " visibleTexels=" << visible << std::endl;
        assert(copied && visible > 0);
    }

    plan.loaderMaxAttempts1AC18 = 4;
    const auto actions = L::BuildActionSkeleton(plan);
    size_t headerAdvanceCount = 0;
    size_t recordAdvanceCount = 0;
    size_t vabAllocations = 0;
    size_t firstAttemptAudio = 0;
    for (size_t i = 0; i < actions.size(); ++i) {
        const auto& a = actions[i];
        if (a.kind == L::ActionKind::ReadHeader1A818) {
            assert(actions[i+1].kind == L::ActionKind::Seek1A89C);
            assert(actions[i+1].cd.seekRelativeKnown);
            assert(actions[i+1].cd.seekRelativeSectors == 4);
            ++headerAdvanceCount;
        }
        if (a.cd.seekByRecordSectors) {
            assert(a.branchTemplate);
            assert(a.kind == L::ActionKind::Seek1A89C);
            assert(i+1 == actions.size() || actions[i+1].recordType != a.recordType);
            ++recordAdvanceCount;
        }
        if (a.recordType == L::LoaderRecordType::Type2Vab &&
            a.kind == L::ActionKind::StackAlloc25A70) {
            ++vabAllocations;
            if (a.memory.sizeBytesWord == 4) {
                assert(actions[i+1].kind == L::ActionKind::StackAlloc25A70);
                assert(actions[i+1].memory.sizeBytesWord == 9);
                assert(actions[i+2].kind == L::ActionKind::ReadPayload1A818);
            }
        }
        if (a.recordType == L::LoaderRecordType::Type2Vab &&
            a.kind == L::ActionKind::RetrySfxReset26FA4) {
            ++firstAttemptAudio;
            assert(a.parserFlag == 1);
            assert(actions[i-1].kind == L::ActionKind::StackFree25AF8);
        }
    }
    assert(headerAdvanceCount != 0);
    assert(recordAdvanceCount == 3u * headerAdvanceCount);
    assert(vabAllocations == 2u * headerAdvanceCount && firstAttemptAudio == 1);

    // Exercise the production coroutine control/byte carriers with bounded
    // synchronous HAL test doubles. This proves read addresses and record
    // dispatch, not GPU presentation or native asynchronous CD timing.
    L::RunnerState walk{};
    assert(L::Begin(walk, plan));
    auto memory = std::make_unique<PrStage1LoaderMemoryDirectState>();
    size_t sectorCursor = 0;
    std::vector<size_t> headerReads;
    std::vector<size_t> payloadReads;
    for (size_t guard = 0; guard < 200 && !L::IsTerminalStatus(walk.status); ++guard) {
        L::Action action{};
        assert(L::PopNextAction(walk, action));
        if (!action.blocksForFeedback) continue;
        L::ActionFeedback feedback{};
        feedback.kind = action.kind;
        feedback.psxOrder = action.psxOrder;
        feedback.completed = feedback.success = true;
        L::ProducerStep step{};
        assert(L::DescribeWaitingProducerStep(walk, step));
        if (action.kind == L::ActionKind::Seek1A89C) {
            int32_t nativeLba = -1;
            assert(L::ResolveSeekLba8001A89C(action, true, 138859,
                true, static_cast<int32_t>(138859 + sectorCursor), nativeLba));
            sectorCursor = static_cast<size_t>(nativeLba - 138859);
        } else if (step.category == L::ProducerCategory::Memory) {
            feedback.hasMemoryResult = true;
            assert(L::BuildMemorySeamResultForProducerStep(walk, *memory, step, feedback.memoryResult));
            assert(L::MemorySeamResultSucceededForAction(action, feedback.memoryResult));
        } else if (action.kind == L::ActionKind::ReadHeader1A818 ||
                   action.kind == L::ActionKind::ReadPayload1A818) {
            const bool header = action.kind == L::ActionKind::ReadHeader1A818;
            const size_t sectors = header ? 4u : action.recordData.sectorCount;
            assert((sectorCursor + sectors) * 2048u <= bytes.size());
            (header ? headerReads : payloadReads).push_back(sectorCursor * 2048u);
            feedback.hasCdResult = true;
            feedback.cdResult.present = true;
            feedback.cdResult.dstPtrKnown = true;
            L::ResolvedPayload readTarget = action.resolvedPayload;
            if (!action.payloadResolved || !readTarget.psxAddressKnown)
                assert(L::TryGetResolvedPayloadForRecord(walk, action.recordIndex, readTarget));
            assert(readTarget.psxAddressKnown);
            feedback.cdResult.dstPtr = readTarget.psxAddress;
            if (!header && action.recordType == L::LoaderRecordType::Type2Vab) {
                assert(action.recordData.vabBodyPsxAddressKnown);
                assert(action.recordData.vabBodyPsxAddress ==
                    feedback.cdResult.dstPtr + PrStage1LoaderMemoryDirectAlign8Bytes(
                        action.recordData.payloadBytes));
                assert(memory->gpPlus324StackDepth == 2);
            }
            feedback.cdResult.sectorCountKnown = true;
            feedback.cdResult.sectorCount = static_cast<int32_t>(sectors);
            feedback.cdResult.livePayloadBytesKnown = true;
            feedback.cdResult.livePayloadData = bytes.data() + sectorCursor * 2048u;
            feedback.cdResult.livePayloadSize = sectors * 2048u;
        } else if (action.kind == L::ActionKind::DispatchRecord1A8F0) {
            feedback.hasRecordData = feedback.hasRecordType = true;
            assert(L::TryBuildRecordDataFromDescriptorPayload(walk,
                action.recordIndex, feedback.recordData));
            feedback.recordType = feedback.recordData.recordType;
            assert(feedback.recordType != L::LoaderRecordType::Unknown);
        }
        assert(L::ApplyFeedback(walk, action, feedback));
    }
    assert(walk.status == L::RunnerStatus::Completed);
    assert(headerReads == std::vector<size_t>({0u, 458752u, 567296u, 634880u}));
    assert(payloadReads == std::vector<size_t>({8192u, 466944u, 575488u}));
    assert(memory->gpPlus324StackDepth == 1); // retained VH, freed temporary VB

    namespace S = PrStage1LoaderSpuHal;
    const auto le32 = [&](size_t p) { uint32_t v; std::memcpy(&v, bytes.data()+p, 4); return v; };
    const size_t vhSize = le32(567296u + 16u);
    std::vector<uint8_t> vh(bytes.begin()+575488u, bytes.begin()+575488u+vhSize);
    S::VabSlots8002E474 slots{};
    S::InitializeVabSlots8003226C(slots);
    uint32_t allocationBytes = 0;
    auto allocate = [](uint32_t n, int32_t& base, void* user) {
        *static_cast<uint32_t*>(user) = n; base = 0x1010; return true;
    };
    int16_t slot = -2;
    auto openedHeader = vh;
    assert(S::OpenVab8002E474(slots, openedHeader, -1, allocate, &allocationBytes, slot));
    assert(slot == 0 && slots.count801C35F0 == 1 && slots.status800928F8[0] == 2);
    assert(!slots.transferReady800555F8 && slots.base801C35F8[0] == 0x1010);
    assert(allocationBytes != 0 && slots.bytes801C35B0[0] == allocationBytes);
    assert(S::OpenVab8002E474(slots, openedHeader, -1, allocate, &allocationBytes, slot));
    assert(slot == -1 && slots.count801C35F0 == 1); // pending transfer guard
    S::InitializeVabSlots8003226C(slots);
    auto badHeader = vh; badHeader[1] = 0;
    assert(S::OpenVab8002E474(slots, badHeader, -1, allocate, &allocationBytes, slot));
    assert(slot == -1 && slots.count801C35F0 == 0 && slots.transferReady800555F8);
    auto allocFail = [](uint32_t, int32_t& base, void*) { base = -1; return true; };
    assert(S::OpenVab8002E474(slots, vh, -1, allocFail, nullptr, slot));
    assert(slot == -1 && slots.status800928F8[0] == 0 && slots.transferReady800555F8);
    auto fullSlots = slots;
    fullSlots.status800928F8.fill(1);
    fullSlots.count801C35F0 = 16;
    assert(S::OpenVab8002E474(fullSlots, vh, -1, allocate, &allocationBytes, slot));
    assert(slot == -1 && fullSlots.count801C35F0 == 16 && fullSlots.transferReady800555F8);
    auto excessPrograms = vh;
    excessPrograms[18] = 129; excessPrograms[19] = 0;
    assert(S::OpenVab8002E474(slots, excessPrograms, -1, allocate, &allocationBytes, slot));
    assert(slot == -1 && slots.count801C35F0 == 0);
    auto legacyHeader = vh;
    legacyHeader.resize(32u + 64u * 16u + 512u);
    legacyHeader[4] = 4;
    legacyHeader[5] = legacyHeader[6] = legacyHeader[7] = 0;
    legacyHeader[18] = legacyHeader[19] = 0;
    legacyHeader[22] = 255;
    const auto originalLegacy = legacyHeader;
    allocationBytes = 0;
    assert(!S::OpenVab8002E474(slots, legacyHeader, -1, allocate, &allocationBytes, slot));
    assert(legacyHeader == originalLegacy && allocationBytes == 0);
    assert(slot == -1 && slots.count801C35F0 == 0 && slots.transferReady800555F8);
    L::RunnerState failedOpen{};
    failedOpen.status = L::RunnerStatus::Running;
    for (const auto kind : {L::ActionKind::VabOpen27078, L::ActionKind::VabTransfer270D4,
                           L::ActionKind::VabEnable270FC, L::ActionKind::StackFree25AF8}) {
        L::Action a{}; a.kind = kind; a.psxOrder = uint32_t(failedOpen.actions.size());
        a.blocksForFeedback = true; failedOpen.actions.push_back(a);
    }
    L::Action openAction{};
    assert(L::PopNextAction(failedOpen, openAction));
    L::ActionFeedback openFeedback{};
    openFeedback.kind = openAction.kind; openFeedback.psxOrder = openAction.psxOrder;
    openFeedback.completed = openFeedback.success = true;
    openFeedback.hasSpuResult = openFeedback.spuResult.present = true;
    openFeedback.spuResult.lowerResultKnown = true;
    openFeedback.spuResult.lowerResult = -1;
    assert(L::ApplyFeedback(failedOpen, openAction, openFeedback));
    assert(failedOpen.status == L::RunnerStatus::Running);
    assert(L::PopNextAction(failedOpen, openAction));
    assert(openAction.kind == L::ActionKind::StackFree25AF8);
    std::cout << "PASS: native VH/VB allocations, retained VH/free VB, first-parser audio, VAB slot/busy/header/allocation failure branches\n";
    L::RunnerState retries{};
    assert(L::Begin(retries, plan));
    size_t dispatches = 0;
    for (size_t guard = 0; guard < 200 && !L::IsTerminalStatus(retries.status); ++guard) {
        L::Action action{};
        assert(L::PopNextAction(retries, action));
        if (!action.blocksForFeedback) continue;
        L::ActionFeedback feedback{};
        feedback.kind = action.kind;
        feedback.psxOrder = action.psxOrder;
        feedback.completed = feedback.success = true;
        if (action.kind == L::ActionKind::DispatchRecord1A8F0) {
            ++dispatches;
            feedback.hasRecordData = feedback.hasRecordType = true;
            feedback.recordType = L::LoaderRecordType::Unknown;
            feedback.recordData.valid = feedback.recordData.recordTypeKnown = true;
            feedback.recordData.recordType = L::LoaderRecordType::Unknown;
        }
        assert(L::ApplyFeedback(retries, action, feedback));
    }
    assert(dispatches == 4);
    assert(retries.status == L::RunnerStatus::Failed);
    L::Action relative{};
    relative.kind = L::ActionKind::Seek1A89C;
    relative.cd.seekRelativeKnown = true;
    relative.cd.seekRelativeSectors = 4;
    int32_t lba = -1;
    assert(!L::ResolveSeekLba8001A89C(relative, true, 138859, false, 0, lba));
    assert(lba == -1);
    assert(!L::ResolveSeekLba8001A89C(relative, true, 0, true, INT32_MAX, lba));

    L::RunnerState malformed{};
    malformed.status = L::RunnerStatus::Running;
    L::Action fail{};
    fail.kind = L::ActionKind::ParserFailure1A8F0;
    malformed.actions.push_back(fail);
    L::Action popped{};
    assert(L::PopNextAction(malformed, popped));
    assert(malformed.status == L::RunnerStatus::Failed);
    assert(malformed.completionKnown && malformed.completionResult == 0);
    std::cout << "PASS: two TIM records / 581 exact ordered uploads, malformed tail atomic rejection, native header/payload seeks, failed parse is not completion\n";
}
