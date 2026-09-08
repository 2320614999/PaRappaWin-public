#include "pr/pr_ss0_title_draw_desc_direct.h"

#include <cassert>
#include <cstdio>

namespace {

TmdModel MakeExactMode25Model(uint32_t vertexCount,
                              uint32_t primitiveCount)
{
    using namespace PrSS0TitleDrawDescDirect;
    TmdModel model{};
    model.id = 0x41u;
    model.flags = 0u;
    model.objects.resize(1u);
    TmdObject& object = model.objects[0];
    object.vertices.resize(vertexCount);
    object.rawPrimitivePackets.resize(primitiveCount);
    object.primitives.resize(primitiveCount);
    object.scale = kMode25ObjectScale;

    for (uint32_t index = 0; index < primitiveCount; ++index) {
        const uint32_t offset =
            kMode25FirstPrimitiveOffset + index * kMode25PacketBytes;
        TmdRawPrimitivePacket& raw = object.rawPrimitivePackets[index];
        raw.rawPrimitiveIndex = index;
        raw.rawPacketOffset = offset;
        raw.rawPacketByteSize = kMode25PacketBytes;
        raw.olen = kMode25Olen;
        raw.ilen = kMode25Ilen;
        raw.flag = kMode25Flag;
        raw.mode = kMode25Mode;
        raw.parsed = true;
        raw.parsedPrimitiveIndex = index;

        TmdPrimitive& primitive = object.primitives[index];
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
    }
    return model;
}

void AssertInitializedState(const PrSS0TitleDrawDescDirect::RuntimeState8001AF1C& state,
                            const TmdModel& model,
                            PrSS0TitleDrawDescDirect::ModelPath modelPath,
                            uint32_t drawDescAddress,
                            uint32_t coordAddress,
                            uint32_t vertexCount,
                            uint32_t primitiveCount)
{
    using namespace PrSS0TitleDrawDescDirect;
    assert(state.initialized);
    assert(state.drawDescAddress == drawDescAddress);
    assert(state.modelBindingKnown && state.model == &model);
    assert(state.modelPath == modelPath);
    assert(state.attrKnown && state.attr == 0u);
    assert(state.coordBindingKnown && state.coordAddress == coordAddress);
    assert(state.objectBindingKnown && state.object == &model.objects[0]);
    assert(model.objects[0].vertices.size() == vertexCount);
    assert(model.objects[0].rawPrimitivePackets.size() == primitiveCount);
    assert(model.objects[0].primitives.size() == primitiveCount);
    assert(model.objects[0].scale == kMode25ObjectScale);
    assert(state.objectIndex == 0u);
    assert(state.objectEntryOffset == kMode25ObjectEntryOffset);
    assert(state.primitiveGroup.known);
    assert(state.primitiveGroup.firstPrimitiveIndex == 0u);
    assert(state.primitiveGroup.primitiveCount == primitiveCount);
    assert(state.primitiveGroup.mode == kMode25Mode);
    assert(state.primitiveGroup.flag == kMode25Flag);
    assert(state.primitiveGroup.wordStride == kMode25WordStride8004274C);
}

} // namespace

int main()
{
    using namespace PrSS0TitleDrawDescDirect;

    TmdModel paModel =
        MakeExactMode25Model(kPaVertexCount, kPaPrimitiveCount);
    RuntimeState8001AF1C paState{};
    const auto paInitialized = InitializePa8001AF1C(paState, &paModel, true);
    assert(paInitialized.initialized);
    assert(paInitialized.failure == Failure::None);
    AssertInitializedState(
        paState,
        paModel,
        ModelPath::Pa,
        kPaDrawDescAddress801CB65C,
        kPaCoordAddress801CB608,
        kPaVertexCount,
        kPaPrimitiveCount);

    const DrawAttr800428B0 attr = DecodeAttr800428B0(paState);
    assert(attr.known && attr.active);
    assert(attr.bits0To2 == 0u && attr.bits3To4 == 0u);
    assert(!attr.bit5 && !attr.bit6 && attr.bits9To11 == 0u);
    assert(!attr.bit30);
    assert(IsMode25PrimitiveReady800428B0(
        paState, &paModel.objects[0], 0u));
    assert(!IsMode25PrimitiveReady800428B0(
        paState, &paModel.objects[0], kPaPrimitiveCount));
    assert(!IsFirstPaKagePrimitiveReady800428B0(
        paState, &paModel.objects[0], 0u));

    TmdModel paKageModel =
        MakeExactMode25Model(kPaKageVertexCount, kPaKagePrimitiveCount);
    RuntimeState8001AF1C paKageState{};
    const auto paKageInitialized =
        InitializePaKage8001AF1C(paKageState, &paKageModel, true);
    assert(paKageInitialized.initialized);
    assert(paKageInitialized.failure == Failure::None);
    AssertInitializedState(
        paKageState,
        paKageModel,
        ModelPath::PaKage,
        kPaKageDrawDescAddress801CD80C,
        kPaKageCoordAddress801CD76C,
        kPaKageVertexCount,
        kPaKagePrimitiveCount);
    assert(IsMode25PrimitiveReady800428B0(
        paKageState, &paKageModel.objects[0], 0u));
    assert(IsFirstPaKagePrimitiveReady800428B0(
        paKageState, &paKageModel.objects[0], 0u));
    assert(!IsFirstPaKagePrimitiveReady800428B0(
        paKageState, &paKageModel.objects[0], kPaKagePrimitiveCount));

    TmdModel loModel =
        MakeExactMode25Model(kLoVertexCount, kLoPrimitiveCount);
    RuntimeState8001AF1C loState{};
    const auto loInitialized = InitializeLo8001AF1C(
        loState, &loModel, true);
    assert(loInitialized.initialized);
    assert(loInitialized.failure == Failure::None);
    AssertInitializedState(
        loState,
        loModel,
        ModelPath::Lo,
        kLoDrawDescAddress801CB5F0,
        kLoCoordAddress801CB5A0,
        kLoVertexCount,
        kLoPrimitiveCount);
    assert(IsMode25PrimitiveReady800428B0(
        loState, &loModel.objects[0], 0u));
    assert(IsFirstLoPrimitiveReady800428B0(
        loState, &loModel.objects[0], kLoPrimitiveCount - 1u));
    assert(!IsFirstLoPrimitiveReady800428B0(
        loState, &loModel.objects[0], kLoPrimitiveCount));
    assert(!IsFirstHpPrimitiveReady800428B0(
        loState, &loModel.objects[0], 0u));

    TmdModel hpModel =
        MakeExactMode25Model(kHpVertexCount, kHpPrimitiveCount);
    RuntimeState8001AF1C hpState{};
    const auto hpInitialized = InitializeHp8001AF1C(
        hpState, &hpModel, true);
    assert(hpInitialized.initialized);
    assert(hpInitialized.failure == Failure::None);
    AssertInitializedState(
        hpState,
        hpModel,
        ModelPath::Hp,
        kHpDrawDescAddress801CC70C,
        kHpCoordAddress801CC6BC,
        kHpVertexCount,
        kHpPrimitiveCount);
    assert(IsMode25PrimitiveReady800428B0(
        hpState, &hpModel.objects[0], 0u));
    assert(IsFirstHpPrimitiveReady800428B0(
        hpState, &hpModel.objects[0], kHpPrimitiveCount - 1u));
    assert(!IsFirstHpPrimitiveReady800428B0(
        hpState, &hpModel.objects[0], kHpPrimitiveCount));
    assert(!IsFirstLoPrimitiveReady800428B0(
        hpState, &hpModel.objects[0], 0u));

    assert(!IsMode25PrimitiveReady800428B0(
        paState, &paKageModel.objects[0], 0u));
    assert(!IsMode25PrimitiveReady800428B0(
        paKageState, &paModel.objects[0], 0u));
    assert(!IsMode25PrimitiveReady800428B0(
        loState, &hpModel.objects[0], 0u));
    assert(!IsMode25PrimitiveReady800428B0(
        hpState, &loModel.objects[0], 0u));

    RuntimeState8001AF1C crossed = paState;
    crossed.drawDescAddress = kPaKageDrawDescAddress801CD80C;
    assert(!IsMode25PrimitiveReady800428B0(
        crossed, &paModel.objects[0], 0u));
    crossed = paState;
    crossed.coordAddress = kPaKageCoordAddress801CD76C;
    assert(!IsMode25PrimitiveReady800428B0(
        crossed, &paModel.objects[0], 0u));
    crossed = paState;
    crossed.modelPath = ModelPath::PaKage;
    assert(!IsMode25PrimitiveReady800428B0(
        crossed, &paModel.objects[0], 0u));

    crossed = paKageState;
    crossed.drawDescAddress = kPaDrawDescAddress801CB65C;
    assert(!IsMode25PrimitiveReady800428B0(
        crossed, &paKageModel.objects[0], 0u));
    crossed = paKageState;
    crossed.coordAddress = kPaCoordAddress801CB608;
    assert(!IsMode25PrimitiveReady800428B0(
        crossed, &paKageModel.objects[0], 0u));
    crossed = paKageState;
    crossed.modelPath = ModelPath::Pa;
    assert(!IsMode25PrimitiveReady800428B0(
        crossed, &paKageModel.objects[0], 0u));

    crossed = loState;
    crossed.drawDescAddress = kHpDrawDescAddress801CC70C;
    assert(!IsMode25PrimitiveReady800428B0(
        crossed, &loModel.objects[0], 0u));
    crossed = loState;
    crossed.coordAddress = kHpCoordAddress801CC6BC;
    assert(!IsMode25PrimitiveReady800428B0(
        crossed, &loModel.objects[0], 0u));
    crossed = loState;
    crossed.modelPath = ModelPath::Hp;
    assert(!IsMode25PrimitiveReady800428B0(
        crossed, &loModel.objects[0], 0u));
    crossed = loState;
    crossed.modelPath = static_cast<ModelPath>(0xFFu);
    assert(!IsMode25PrimitiveReady800428B0(
        crossed, &loModel.objects[0], 0u));
    crossed = hpState;
    crossed.modelPath = ModelPath::Lo;
    assert(!IsMode25PrimitiveReady800428B0(
        crossed, &hpModel.objects[0], 0u));
    crossed = loState;
    crossed.primitiveGroup.primitiveCount = kHpPrimitiveCount;
    assert(!IsMode25PrimitiveReady800428B0(
        crossed, &loModel.objects[0], 0u));

    TmdModel otherLoModel =
        MakeExactMode25Model(kLoVertexCount, kLoPrimitiveCount);
    assert(!IsMode25PrimitiveReady800428B0(
        loState, &otherLoModel.objects[0], 0u));
    RuntimeState8001AF1C wrongProvenance = loState;
    wrongProvenance.model = &otherLoModel;
    assert(!IsMode25PrimitiveReady800428B0(
        wrongProvenance, &loModel.objects[0], 0u));
    wrongProvenance = loState;
    wrongProvenance.object = &otherLoModel.objects[0];
    assert(!IsMode25PrimitiveReady800428B0(
        wrongProvenance, &otherLoModel.objects[0], 0u));

    TmdModel malformed =
        MakeExactMode25Model(kPaVertexCount, kPaPrimitiveCount);
    malformed.objects[0].rawPrimitivePackets[17].mode = 0x24u;
    RuntimeState8001AF1C rejectedState{};
    const auto rejected = InitializePa8001AF1C(
        rejectedState, &malformed, true);
    assert(!rejected.initialized);
    assert(rejected.failure == Failure::WrongPrimitiveShape);
    assert(!rejectedState.initialized && !rejectedState.modelBindingKnown);
    assert(rejectedState.model == nullptr && !rejectedState.attrKnown);
    assert(rejectedState.object == nullptr);

    TmdModel mutatedAfterInitialize =
        MakeExactMode25Model(kPaVertexCount, kPaPrimitiveCount);
    RuntimeState8001AF1C mutatedState{};
    assert(InitializePa8001AF1C(
               mutatedState, &mutatedAfterInitialize, true)
               .initialized);
    mutatedAfterInitialize.objects[0]
        .rawPrimitivePackets[17]
        .rawPacketByteSize = kMode25PacketBytes - 1u;
    assert(!IsMode25PrimitiveReady800428B0(
        mutatedState, &mutatedAfterInitialize.objects[0], 0u));

    TmdModel nonContiguousLo =
        MakeExactMode25Model(kLoVertexCount, kLoPrimitiveCount);
    nonContiguousLo.objects[0]
        .rawPrimitivePackets[kLoPrimitiveCount - 1u]
        .rawPacketOffset += 4u;
    const auto nonContiguousRejected = InitializeLo8001AF1C(
        rejectedState, &nonContiguousLo, true);
    assert(!nonContiguousRejected.initialized);
    assert(nonContiguousRejected.failure == Failure::WrongPrimitiveShape);
    assert(!rejectedState.initialized);

    TmdModel malformedHp =
        MakeExactMode25Model(kHpVertexCount, kHpPrimitiveCount);
    malformedHp.objects[0]
        .primitives[kHpPrimitiveCount - 1u]
        .ilen = kMode25Ilen - 1u;
    const auto malformedHpRejected = InitializeHp8001AF1C(
        rejectedState, &malformedHp, true);
    assert(!malformedHpRejected.initialized);
    assert(malformedHpRejected.failure == Failure::WrongPrimitiveShape);
    assert(!rejectedState.initialized);

    TmdModel wrongLoVertexCount = MakeExactMode25Model(
        kLoVertexCount - 1u, kLoPrimitiveCount);
    const auto wrongLoVertices = InitializeLo8001AF1C(
        rejectedState, &wrongLoVertexCount, true);
    assert(!wrongLoVertices.initialized);
    assert(wrongLoVertices.failure == Failure::WrongObjectShape);
    assert(!rejectedState.initialized);

    TmdModel wrongHpPrimitiveCount = MakeExactMode25Model(
        kHpVertexCount, kHpPrimitiveCount - 1u);
    const auto wrongHpPrimitives = InitializeHp8001AF1C(
        rejectedState, &wrongHpPrimitiveCount, true);
    assert(!wrongHpPrimitives.initialized);
    assert(wrongHpPrimitives.failure == Failure::WrongPrimitiveShape);
    assert(!rejectedState.initialized);

    RuntimeState8001AF1C wrongPathState = hpState;
    const auto wrongPath = InitializeHp8001AF1C(
        wrongPathState, &loModel, true);
    assert(!wrongPath.initialized);
    assert(wrongPath.failure == Failure::WrongObjectShape);
    assert(!wrongPathState.initialized);
    assert(!wrongPathState.modelBindingKnown && wrongPathState.model == nullptr);
    assert(!wrongPathState.coordBindingKnown);
    assert(!wrongPathState.objectBindingKnown && wrongPathState.object == nullptr);
    assert(!wrongPathState.primitiveGroup.known);

    TmdModel wrongObjectShape =
        MakeExactMode25Model(kPaVertexCount, kPaPrimitiveCount);
    wrongObjectShape.objects[0].vertices.pop_back();
    const auto objectRejected = InitializePa8001AF1C(
        rejectedState, &wrongObjectShape, true);
    assert(!objectRejected.initialized);
    assert(objectRejected.failure == Failure::WrongObjectShape);
    assert(!rejectedState.initialized);

    TmdModel wrongCountModel =
        MakeExactMode25Model(kPaKageVertexCount, kPaKagePrimitiveCount);
    wrongCountModel.objects.push_back(TmdObject{});
    const auto wrongCount = InitializePaKage8001AF1C(
        rejectedState, &wrongCountModel, true);
    assert(!wrongCount.initialized);
    assert(wrongCount.failure == Failure::WrongObjectCount);
    assert(!rejectedState.initialized);

    const auto unknown = InitializePaKage8001AF1C(
        rejectedState, &paKageModel, false);
    assert(!unknown.initialized);
    assert(unknown.failure == Failure::SourceParseIncomplete);
    assert(!rejectedState.initialized);

    std::printf("test_ss0_title_draw_desc_direct: ok\n");
    return 0;
}
