#include "pr/pr_ss0_title_primitive_group_direct.h"

#include <cassert>
#include <cstdio>
#include <memory>

namespace {

struct Fixture {
    TmdModel model{};
    PrSS0TitleDrawDescDirect::RuntimeState8001AF1C drawDesc{};
    VdfData vdf{};
    DatData dat{};
    PrSS0TitleTransformDirect::MimeRuntimeInput80013EA8 mime{};
    PrPsxGteDirect::Matrix3x4 matrix{};
    PrSS0TitlePacketWorkDirect::RuntimeState801C609C packetWork{};
    PrPsxGraphOwnerDirect::PsxGraphState graph{};
};

struct ModelContract {
    uint32_t vertexCount = 0u;
    uint32_t primitiveCount = 0u;
};

ModelContract ContractForPath(
    PrSS0TitleTransformDirect::ModelPath path)
{
    using namespace PrSS0TitleDrawDescDirect;
    using PrSS0TitleTransformDirect::ModelPath;
    switch (path) {
    case ModelPath::Pa:
        return {kPaVertexCount, kPaPrimitiveCount};
    case ModelPath::PaKage:
        return {kPaKageVertexCount, kPaKagePrimitiveCount};
    case ModelPath::Lo:
        return {kLoVertexCount, kLoPrimitiveCount};
    case ModelPath::Hp:
        return {kHpVertexCount, kHpPrimitiveCount};
    }
    assert(false);
    return {};
}

PrPsxGteDirect::GteControlState MakeTitleControl()
{
    PrPsxGteDirect::GteControlState control{};
    control.geomScreenKnown = true;
    control.geomScreen = 440u;
    control.geomOffsetKnown = true;
    control.geomOffsetX = 0;
    control.geomOffsetY = 0;
    control.depthCueKnown = true;
    control.depthCueA = -4194;
    control.depthCueB = 0x01400000;
    control.zScaleFactorKnown = true;
    control.zScaleFactor3 = 341;
    control.zScaleFactor4 = 256;
    return control;
}

TmdModel MakeExactMode25Model(
    PrSS0TitleTransformDirect::ModelPath path,
    bool cullPrimitiveOne)
{
    using namespace PrSS0TitleDrawDescDirect;
    const ModelContract contract = ContractForPath(path);
    TmdModel model{};
    model.id = 0x41u;
    model.flags = 0u;
    model.objects.resize(1u);
    TmdObject& object = model.objects[0];
    object.scale = 0;
    object.vertices.resize(contract.vertexCount);
    object.vertices[0] = {-854, -1116, 406, 0};
    object.vertices[1] = {-567, -871, 274, 0};
    object.vertices[2] = {-783, -808, 381, 0};
    object.rawPrimitivePackets.resize(contract.primitiveCount);
    object.primitives.resize(contract.primitiveCount);
    for (uint32_t index = 0; index < contract.primitiveCount; ++index) {
        const uint32_t offset =
            kMode25FirstPrimitiveOffset + index * kMode25PacketBytes;
        auto& raw = object.rawPrimitivePackets[index];
        raw.rawPrimitiveIndex = index;
        raw.rawPacketOffset = offset;
        raw.rawPacketByteSize = kMode25PacketBytes;
        raw.olen = kMode25Olen;
        raw.ilen = kMode25Ilen;
        raw.flag = kMode25Flag;
        raw.mode = kMode25Mode;
        raw.parsed = true;
        raw.parsedPrimitiveIndex = index;

        auto& primitive = object.primitives[index];
        primitive.rawPacketKnown = true;
        primitive.rawPrimitiveIndex = index;
        primitive.rawPacketOffset = offset;
        primitive.rawPacketByteSize = kMode25PacketBytes;
        primitive.olen = kMode25Olen;
        primitive.ilen = kMode25Ilen;
        primitive.flag = kMode25Flag;
        primitive.mode = kMode25Mode;
        primitive.textured = true;
        primitive.quad = false;
        primitive.v0_idx = 0u;
        primitive.v1_idx = 1u;
        primitive.v2_idx = 2u;
        primitive.r = 0x80u;
        primitive.g = 0x80u;
        primitive.b = 0x80u;
        primitive.u0 = 0x8Cu;
        primitive.v0 = 0x00u;
        primitive.clut = 0x48B5u;
        primitive.u1 = 0xB7u;
        primitive.v1 = 0x36u;
        primitive.tpage = 0x0009u;
        primitive.u2 = 0x8Cu;
        primitive.v2 = 0x36u;
    }
    if (cullPrimitiveOne) {
        object.primitives[1].v1_idx = 2u;
        object.primitives[1].v2_idx = 1u;
    }
    return model;
}

std::unique_ptr<Fixture> MakeFixture(
    PrSS0TitleTransformDirect::ModelPath path =
        PrSS0TitleTransformDirect::ModelPath::PaKage,
    bool cullPrimitiveOne = false)
{
    auto fixture = std::make_unique<Fixture>();
    fixture->model = MakeExactMode25Model(path, cullPrimitiveOne);
    PrSS0TitleDrawDescDirect::InitializeResult8001AF1C initialized{};
    switch (path) {
    case PrSS0TitleTransformDirect::ModelPath::Pa:
        initialized = PrSS0TitleDrawDescDirect::InitializePa8001AF1C(
            fixture->drawDesc, &fixture->model, true);
        break;
    case PrSS0TitleTransformDirect::ModelPath::PaKage:
        initialized = PrSS0TitleDrawDescDirect::InitializePaKage8001AF1C(
            fixture->drawDesc, &fixture->model, true);
        break;
    case PrSS0TitleTransformDirect::ModelPath::Lo:
        initialized = PrSS0TitleDrawDescDirect::InitializeLo8001AF1C(
            fixture->drawDesc, &fixture->model, true);
        break;
    case PrSS0TitleTransformDirect::ModelPath::Hp:
        initialized = PrSS0TitleDrawDescDirect::InitializeHp8001AF1C(
            fixture->drawDesc, &fixture->model, true);
        break;
    }
    assert(initialized.initialized);

    fixture->vdf.keys = 1u;
    fixture->vdf.keyList.resize(1u);
    auto& vdfKey = fixture->vdf.keyList[0];
    vdfKey.obj = 0u;
    vdfKey.vertTop = 0u;
    vdfKey.nVert = static_cast<uint32_t>(
        fixture->model.objects[0].vertices.size());
    vdfKey.deltas.resize(vdfKey.nVert);
    fixture->dat.keys = 1u;
    fixture->dat.maxFrames = 1u;
    fixture->dat.keyList.resize(1u);
    fixture->dat.keyList[0].frames = 1u;
    fixture->dat.keyList[0].influence = {4096};
    fixture->mime.bindingKnown = true;
    fixture->mime.vdf = &fixture->vdf;
    fixture->mime.dat = &fixture->dat;
    fixture->mime.objectIndex = 0u;
    fixture->mime.sampleFrameKnown = true;
    fixture->mime.sampleFrame = 0u;
    fixture->mime.loopFlagKnown = true;
    fixture->mime.loopFlag = 0u;

    fixture->matrix.words = {{
        0xF4D70433u,
        0x07FB0F6Eu,
        0xFDD10865u,
        0xF211F2C2u,
        0x0000039Eu,
        0xFFFFFFFDu,
        0x000004B0u,
        0x00001831u,
    }};

    auto& packetWork = fixture->packetWork;
    packetWork.initialized = true;
    packetWork.packetArenaBase80025B28 = 0x801AE430u;
    packetWork.packetAllocatorBases801C956C = {
        0x801AE430u, 0x801B8CF0u};
    packetWork.framePrepared801C6410 = true;
    packetWork.currentDrawBuffer8004019C = 0u;
    packetWork.currentPacketAllocator800901C8 = 0x801AE430u;
    auto& work = packetWork.workLists801C9574[0];
    work.order_00 = 10u;
    work.headAddr_04 = 0x801C959Cu;
    work.lastAddr_10 = 0x801CA598u;
    work.tmdOtSlotMirrorKnown = true;
    work.tmdOtSlotMirror[120].valid = true;
    work.tmdOtSlotMirror[120].addr = 0x801C977Cu;
    work.tmdOtSlotMirror[120].value = 0x001C9778u;
    work.tmdPacketWriteMirrorKnown = true;

    fixture->graph.mainPageWorkLists80087288Initialized = true;
    fixture->graph.dword_800901C8 = 0x801AE430u;
    fixture->graph.gte = MakeTitleControl();
    return fixture;
}

uint32_t CountPackets(const Fixture& fixture)
{
    uint32_t count = 0u;
    for (const auto& write :
         fixture.packetWork.workLists801C9574[0].tmdPacketWriteMirror) {
        if (write.valid) {
            ++count;
        }
    }
    return count;
}

} // namespace

int main()
{
    using namespace PrSS0TitlePrimitiveGroupDirect;
    using PrSS0TitleTransformDirect::ModelPath;

    auto fixture = MakeFixture(ModelPath::PaKage);
    const auto result = ExecuteMode25PrimitiveRange8004274C(
        fixture->drawDesc,
        ModelPath::PaKage,
        fixture->mime,
        &fixture->matrix,
        true,
        fixture->packetWork,
        fixture->graph,
        0u,
        190u);
    assert(result.complete);
    assert(result.failure == Failure::None);
    assert(result.visitedCount == 190u);
    assert(result.preparedCount == 190u);
    assert(result.culledCount == 0u);
    assert(result.committedCount == 190u);
    assert(result.firstExecutionKnown);
    assert(result.firstExecution.input.primitiveIndex == 0u);
    assert(result.firstExecution.input.path == ModelPath::PaKage);
    assert(result.firstPlanKnown);
    assert(result.firstPlan.plan.packetWrite.words[0] == 0x071C9778u);
    assert(result.firstCommitKnown);
    assert(result.firstCommit.packetAddress == 0x801AE430u);
    assert(result.allocatorBefore == 0x801AE430u);
    assert(result.allocatorAfter == 0x801AFBF0u);
    assert(fixture->graph.dword_800901C8 == 0x801AFBF0u);
    assert(fixture->packetWork.currentPacketAllocator800901C8 ==
           0x801AFBF0u);
    const auto& work = fixture->packetWork.workLists801C9574[0];
    assert(work.tmdOtSlotMirror[120].value == 0x001AFBD0u);
    assert(CountPackets(*fixture) == 190u);
    assert(work.tmdPacketWriteMirror[1].words[0] == 0x071AE430u);
    assert(work.tmdPacketWriteMirror[189].addr == 0x801AFBD0u);
    assert(work.tmdPacketWriteMirror[189].words[0] == 0x071AFBB0u);
    assert(!fixture->graph.mainPageWorkLists80087288[0]
                .work.tmdPacketWriteMirrorKnown);
    assert(!fixture->packetWork.workLists801C9574[1]
                .tmdPacketWriteMirrorKnown);

    auto pa = MakeFixture(ModelPath::Pa);
    const auto paResult = ExecuteMode25PrimitiveRange8004274C(
        pa->drawDesc,
        ModelPath::Pa,
        pa->mime,
        &pa->matrix,
        true,
        pa->packetWork,
        pa->graph,
        0u,
        190u);
    assert(paResult.complete);
    assert(paResult.failure == Failure::None);
    assert(paResult.visitedCount == 190u);
    assert(paResult.preparedCount == 190u);
    assert(paResult.committedCount == 190u);
    assert(paResult.firstExecutionKnown);
    assert(paResult.firstExecution.input.path == ModelPath::Pa);
    assert(paResult.firstPlanKnown);
    assert(paResult.firstCommitKnown);
    assert(paResult.allocatorAfter == 0x801AFBF0u);
    assert(CountPackets(*pa) == 190u);

    auto lo = MakeFixture(ModelPath::Lo);
    const auto loResult = ExecuteMode25PrimitiveRange8004274C(
        lo->drawDesc,
        ModelPath::Lo,
        lo->mime,
        &lo->matrix,
        true,
        lo->packetWork,
        lo->graph,
        0u,
        PrSS0TitleDrawDescDirect::kLoPrimitiveCount);
    assert(loResult.complete);
    assert(loResult.failure == Failure::None);
    assert(loResult.visitedCount ==
           PrSS0TitleDrawDescDirect::kLoPrimitiveCount);
    assert(loResult.preparedCount ==
           PrSS0TitleDrawDescDirect::kLoPrimitiveCount);
    assert(loResult.committedCount ==
           PrSS0TitleDrawDescDirect::kLoPrimitiveCount);
    assert(loResult.firstExecutionKnown);
    assert(loResult.firstExecution.input.path == ModelPath::Lo);
    assert(loResult.firstPlanKnown);
    assert(loResult.firstCommitKnown);
    assert(loResult.allocatorAfter ==
           0x801AE430u +
               PrSS0TitleDrawDescDirect::kLoPrimitiveCount *
                   PrPsxTmdSubmitDirect::kPacketByteSize);
    assert(CountPackets(*lo) ==
           PrSS0TitleDrawDescDirect::kLoPrimitiveCount);

    auto hp = MakeFixture(ModelPath::Hp);
    const auto hpResult = ExecuteMode25PrimitiveRange8004274C(
        hp->drawDesc,
        ModelPath::Hp,
        hp->mime,
        &hp->matrix,
        true,
        hp->packetWork,
        hp->graph,
        0u,
        PrSS0TitleDrawDescDirect::kHpPrimitiveCount);
    assert(hpResult.complete);
    assert(hpResult.failure == Failure::None);
    assert(hpResult.visitedCount ==
           PrSS0TitleDrawDescDirect::kHpPrimitiveCount);
    assert(hpResult.preparedCount ==
           PrSS0TitleDrawDescDirect::kHpPrimitiveCount);
    assert(hpResult.committedCount ==
           PrSS0TitleDrawDescDirect::kHpPrimitiveCount);
    assert(hpResult.firstExecutionKnown);
    assert(hpResult.firstExecution.input.path == ModelPath::Hp);
    assert(hpResult.firstPlanKnown);
    assert(hpResult.firstCommitKnown);
    assert(hpResult.allocatorAfter ==
           0x801AE430u +
               PrSS0TitleDrawDescDirect::kHpPrimitiveCount *
                   PrPsxTmdSubmitDirect::kPacketByteSize);
    assert(CountPackets(*hp) ==
           PrSS0TitleDrawDescDirect::kHpPrimitiveCount);

    auto invalidPath = MakeFixture(ModelPath::Pa);
    const auto invalidPathResult = ExecuteMode25PrimitiveRange8004274C(
        invalidPath->drawDesc,
        static_cast<ModelPath>(0xFFu),
        invalidPath->mime,
        &invalidPath->matrix,
        true,
        invalidPath->packetWork,
        invalidPath->graph,
        0u,
        190u);
    assert(!invalidPathResult.complete);
    assert(invalidPathResult.failure == Failure::UnsupportedModelPath);
    assert(invalidPathResult.committedCount == 0u);
    assert(!invalidPathResult.partialWrites);
    assert(invalidPath->graph.dword_800901C8 == 0x801AE430u);
    assert(invalidPath->packetWork.currentPacketAllocator800901C8 ==
           0x801AE430u);
    assert(invalidPath->packetWork.workLists801C9574[0]
               .tmdOtSlotMirror[120].value == 0x001C9778u);
    assert(CountPackets(*invalidPath) == 0u);

    auto wrongProvenance = MakeFixture(ModelPath::PaKage);
    const auto wrongProvenanceResult =
        ExecuteMode25PrimitiveRange8004274C(
            wrongProvenance->drawDesc,
            ModelPath::Pa,
            wrongProvenance->mime,
            &wrongProvenance->matrix,
            true,
            wrongProvenance->packetWork,
            wrongProvenance->graph,
            0u,
            190u);
    assert(!wrongProvenanceResult.complete);
    assert(wrongProvenanceResult.failure ==
           Failure::PrimitiveProvenanceMismatch);
    assert(wrongProvenanceResult.committedCount == 0u);
    assert(!wrongProvenanceResult.partialWrites);
    assert(wrongProvenance->graph.dword_800901C8 == 0x801AE430u);
    assert(CountPackets(*wrongProvenance) == 0u);

    auto wrongLoHpPath = MakeFixture(ModelPath::Lo);
    const auto wrongLoHpPathResult = ExecuteMode25PrimitiveRange8004274C(
        wrongLoHpPath->drawDesc,
        ModelPath::Hp,
        wrongLoHpPath->mime,
        &wrongLoHpPath->matrix,
        true,
        wrongLoHpPath->packetWork,
        wrongLoHpPath->graph,
        0u,
        PrSS0TitleDrawDescDirect::kHpPrimitiveCount);
    assert(!wrongLoHpPathResult.complete);
    assert(wrongLoHpPathResult.failure ==
           Failure::PrimitiveProvenanceMismatch);
    assert(wrongLoHpPathResult.committedCount == 0u);
    assert(!wrongLoHpPathResult.partialWrites);
    assert(wrongLoHpPath->graph.dword_800901C8 == 0x801AE430u);
    assert(CountPackets(*wrongLoHpPath) == 0u);

    auto loCountMismatch = MakeFixture(ModelPath::Lo);
    const auto loCountMismatchResult = ExecuteMode25PrimitiveRange8004274C(
        loCountMismatch->drawDesc,
        ModelPath::Lo,
        loCountMismatch->mime,
        &loCountMismatch->matrix,
        true,
        loCountMismatch->packetWork,
        loCountMismatch->graph,
        0u,
        PrSS0TitleDrawDescDirect::kLoPrimitiveCount + 1u);
    assert(!loCountMismatchResult.complete);
    assert(loCountMismatchResult.failure == Failure::PrimitiveRangeInvalid);
    assert(loCountMismatchResult.committedCount == 0u);
    assert(!loCountMismatchResult.partialWrites);
    assert(loCountMismatch->graph.dword_800901C8 == 0x801AE430u);
    assert(CountPackets(*loCountMismatch) == 0u);

    auto hpCountMismatch = MakeFixture(ModelPath::Hp);
    const auto hpCountMismatchResult = ExecuteMode25PrimitiveRange8004274C(
        hpCountMismatch->drawDesc,
        ModelPath::Hp,
        hpCountMismatch->mime,
        &hpCountMismatch->matrix,
        true,
        hpCountMismatch->packetWork,
        hpCountMismatch->graph,
        0u,
        PrSS0TitleDrawDescDirect::kHpPrimitiveCount + 1u);
    assert(!hpCountMismatchResult.complete);
    assert(hpCountMismatchResult.failure == Failure::PrimitiveRangeInvalid);
    assert(hpCountMismatchResult.committedCount == 0u);
    assert(!hpCountMismatchResult.partialWrites);
    assert(hpCountMismatch->graph.dword_800901C8 == 0x801AE430u);
    assert(CountPackets(*hpCountMismatch) == 0u);

    auto wrongDescriptorCount = MakeFixture(ModelPath::Lo);
    wrongDescriptorCount->drawDesc.primitiveGroup.primitiveCount =
        static_cast<uint16_t>(
            PrSS0TitleDrawDescDirect::kPaPrimitiveCount);
    const auto wrongDescriptorCountResult =
        ExecuteMode25PrimitiveRange8004274C(
            wrongDescriptorCount->drawDesc,
            ModelPath::Lo,
            wrongDescriptorCount->mime,
            &wrongDescriptorCount->matrix,
            true,
            wrongDescriptorCount->packetWork,
            wrongDescriptorCount->graph,
            0u,
            PrSS0TitleDrawDescDirect::kLoPrimitiveCount);
    assert(!wrongDescriptorCountResult.complete);
    assert(wrongDescriptorCountResult.failure ==
           Failure::DrawDescriptorNotReady);
    assert(wrongDescriptorCountResult.committedCount == 0u);
    assert(!wrongDescriptorCountResult.partialWrites);
    assert(wrongDescriptorCount->graph.dword_800901C8 == 0x801AE430u);
    assert(CountPackets(*wrongDescriptorCount) == 0u);

    auto wrongShape = MakeFixture(ModelPath::Pa);
    ++wrongShape->drawDesc.primitiveGroup.wordStride;
    const auto wrongShapeResult = ExecuteMode25PrimitiveRange8004274C(
        wrongShape->drawDesc,
        ModelPath::Pa,
        wrongShape->mime,
        &wrongShape->matrix,
        true,
        wrongShape->packetWork,
        wrongShape->graph,
        0u,
        190u);
    assert(!wrongShapeResult.complete);
    assert(wrongShapeResult.failure == Failure::DrawDescriptorNotReady);
    assert(wrongShapeResult.committedCount == 0u);
    assert(CountPackets(*wrongShape) == 0u);

    auto culled = MakeFixture(ModelPath::PaKage, true);
    const auto culledResult = ExecuteMode25PrimitiveRange8004274C(
        culled->drawDesc,
        ModelPath::PaKage,
        culled->mime,
        &culled->matrix,
        true,
        culled->packetWork,
        culled->graph,
        0u,
        190u);
    assert(culledResult.complete);
    assert(culledResult.preparedCount == 190u);
    assert(culledResult.culledCount == 1u);
    assert(culledResult.committedCount == 189u);
    assert(CountPackets(*culled) == 189u);

    auto flagRejected = MakeFixture(ModelPath::PaKage);
    for (std::size_t index = 0u; index < 3u; ++index) {
        flagRejected->model.objects[0].vertices[index] =
            {32767, 32767, 1, 0};
    }
    flagRejected->matrix.words = {{
        0x00001000u,
        0x00000000u,
        0x00001000u,
        0x00000000u,
        0x00001000u,
        0u,
        0u,
        0u,
    }};
    const auto flagRejectedResult = ExecuteMode25PrimitiveRange8004274C(
        flagRejected->drawDesc,
        ModelPath::PaKage,
        flagRejected->mime,
        &flagRejected->matrix,
        true,
        flagRejected->packetWork,
        flagRejected->graph,
        0u,
        190u);
    assert(flagRejectedResult.complete);
    assert(flagRejectedResult.firstExecutionKnown);
    assert(static_cast<int32_t>(
               flagRejectedResult.firstExecution.output.flagAfterRtpt) < 0);
    assert(!flagRejectedResult.firstExecution.geometry.visible);
    assert(flagRejectedResult.culledCount == 190u);
    assert(flagRejectedResult.committedCount == 0u);
    assert(flagRejectedResult.allocatorAfter == 0x801AE430u);
    assert(CountPackets(*flagRejected) == 0u);

    auto missingMime = MakeFixture(ModelPath::PaKage);
    missingMime->mime.sampleFrameKnown = false;
    const auto missingMimeResult = ExecuteMode25PrimitiveRange8004274C(
        missingMime->drawDesc,
        ModelPath::PaKage,
        missingMime->mime,
        &missingMime->matrix,
        true,
        missingMime->packetWork,
        missingMime->graph,
        0u,
        190u);
    assert(!missingMimeResult.complete);
    assert(missingMimeResult.failure == Failure::MimeDeformationRejected);
    assert(missingMimeResult.committedCount == 0u);
    assert(missingMime->graph.dword_800901C8 == 0x801AE430u);
    assert(CountPackets(*missingMime) == 0u);

    auto rollback = MakeFixture(ModelPath::PaKage);
    rollback->drawDesc.attr = 1u << 30u;
    const auto rollbackResult = ExecuteMode25PrimitiveRange8004274C(
        rollback->drawDesc,
        ModelPath::PaKage,
        rollback->mime,
        &rollback->matrix,
        true,
        rollback->packetWork,
        rollback->graph,
        0u,
        190u);
    assert(!rollbackResult.complete);
    assert(rollbackResult.failure == Failure::PacketPlanRejected);
    assert(rollbackResult.packetPlanFailure ==
           PrSS0TitlePacketPlanDirect::Failure::SubmitRejected);
    assert(rollbackResult.committedCount == 0u);
    assert(!rollbackResult.partialWrites);
    assert(rollback->graph.dword_800901C8 == 0x801AE430u);
    assert(rollback->packetWork.currentPacketAllocator800901C8 ==
           0x801AE430u);
    assert(CountPackets(*rollback) == 0u);
    assert(rollback->packetWork.workLists801C9574[0]
               .tmdOtSlotMirror[120].value == 0x001C9778u);

    auto capacity = MakeFixture(ModelPath::PaKage);
    for (auto& write :
         capacity->packetWork.workLists801C9574[0]
             .tmdPacketWriteMirror) {
        write.valid = true;
        write.addr = 0x80001000u;
    }
    capacity->packetWork.workLists801C9574[0]
        .tmdPacketWriteMirror.back().valid = false;
    const auto capacityResult = ExecuteMode25PrimitiveRange8004274C(
        capacity->drawDesc,
        ModelPath::PaKage,
        capacity->mime,
        &capacity->matrix,
        true,
        capacity->packetWork,
        capacity->graph,
        0u,
        190u);
    assert(!capacityResult.complete);
    assert(capacityResult.failure == Failure::PacketMirrorCapacityExceeded);
    assert(capacityResult.committedCount == 1u);
    assert(capacityResult.partialWrites);
    assert(capacity->graph.dword_800901C8 == 0x801AE450u);

    std::printf("test_ss0_title_primitive_group_direct: ok\n");
    return 0;
}
