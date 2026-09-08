#include "pr_ss0_title_draw_desc_direct.h"

namespace PrSS0TitleDrawDescDirect {
namespace {

struct ModelContract8001AF1C {
    uint32_t drawDescAddress = 0;
    uint32_t coordAddress = 0;
    uint32_t vertexCount = 0;
    uint32_t primitiveCount = 0;
};

bool TryResolveModelContract8001AF1C(
    ModelPath modelPath,
    ModelContract8001AF1C& contract)
{
    contract = {};
    switch (modelPath) {
    case ModelPath::Pa:
        contract.drawDescAddress = kPaDrawDescAddress801CB65C;
        contract.coordAddress = kPaCoordAddress801CB608;
        contract.vertexCount = kPaVertexCount;
        contract.primitiveCount = kPaPrimitiveCount;
        return true;
    case ModelPath::PaKage:
        contract.drawDescAddress = kPaKageDrawDescAddress801CD80C;
        contract.coordAddress = kPaKageCoordAddress801CD76C;
        contract.vertexCount = kPaKageVertexCount;
        contract.primitiveCount = kPaKagePrimitiveCount;
        return true;
    case ModelPath::Lo:
        contract.drawDescAddress = kLoDrawDescAddress801CB5F0;
        contract.coordAddress = kLoCoordAddress801CB5A0;
        contract.vertexCount = kLoVertexCount;
        contract.primitiveCount = kLoPrimitiveCount;
        return true;
    case ModelPath::Hp:
        contract.drawDescAddress = kHpDrawDescAddress801CC70C;
        contract.coordAddress = kHpCoordAddress801CC6BC;
        contract.vertexCount = kHpVertexCount;
        contract.primitiveCount = kHpPrimitiveCount;
        return true;
    }
    return false;
}

bool HasExactMode25PrimitiveGroup8004274C(
    const TmdObject& object,
    const ModelContract8001AF1C& contract)
{
    if (object.vertices.size() != contract.vertexCount ||
        object.scale != kMode25ObjectScale ||
        object.rawPrimitivePackets.size() != contract.primitiveCount ||
        object.primitives.size() != contract.primitiveCount) {
        return false;
    }

    for (uint32_t index = 0; index < contract.primitiveCount; ++index) {
        const TmdRawPrimitivePacket& raw = object.rawPrimitivePackets[index];
        const TmdPrimitive& primitive = object.primitives[index];
        const uint32_t expectedOffset =
            kMode25FirstPrimitiveOffset + index * kMode25PacketBytes;
        if (raw.rawPrimitiveIndex != index ||
            raw.rawPacketOffset != expectedOffset ||
            raw.rawPacketByteSize != kMode25PacketBytes ||
            raw.olen != kMode25Olen || raw.ilen != kMode25Ilen ||
            raw.flag != kMode25Flag || raw.mode != kMode25Mode ||
            !raw.parsed || raw.parsedPrimitiveIndex != index ||
            !primitive.rawPacketKnown ||
            primitive.rawPrimitiveIndex != index ||
            primitive.rawPacketOffset != expectedOffset ||
            primitive.rawPacketByteSize != kMode25PacketBytes ||
            primitive.olen != kMode25Olen ||
            primitive.ilen != kMode25Ilen ||
            primitive.flag != kMode25Flag ||
            primitive.mode != kMode25Mode || !primitive.textured ||
            primitive.quad) {
            return false;
        }
    }
    return true;
}

InitializeResult8001AF1C Fail(RuntimeState8001AF1C& state,
                              Failure failure)
{
    Clear(state);
    InitializeResult8001AF1C out{};
    out.failure = failure;
    return out;
}

InitializeResult8001AF1C InitializeMode25Model8001AF1C(
    RuntimeState8001AF1C& state,
    const TmdModel* model,
    bool sourceParseComplete,
    ModelPath modelPath)
{
    Clear(state);
    if (!sourceParseComplete) {
        return Fail(state, Failure::SourceParseIncomplete);
    }
    if (model == nullptr) {
        return Fail(state, Failure::NullModel);
    }
    if (model->id != 0x41u || model->flags != 0u) {
        return Fail(state, Failure::WrongModelHeader);
    }
    if (model->objects.size() != kMode25ObjectCount) {
        return Fail(state, Failure::WrongObjectCount);
    }

    ModelContract8001AF1C contract{};
    if (!TryResolveModelContract8001AF1C(modelPath, contract)) {
        return Fail(state, Failure::WrongObjectShape);
    }

    const TmdObject& object = model->objects[0];
    if (object.vertices.size() != contract.vertexCount ||
        object.scale != kMode25ObjectScale) {
        return Fail(state, Failure::WrongObjectShape);
    }
    if (!HasExactMode25PrimitiveGroup8004274C(object, contract)) {
        return Fail(state, Failure::WrongPrimitiveShape);
    }

    state.drawDescAddress = contract.drawDescAddress;
    state.modelBindingKnown = true;
    state.model = model;
    state.modelPath = modelPath;
    state.attrKnown = true;
    state.attr = 0u;
    state.coordBindingKnown = true;
    state.coordAddress = contract.coordAddress;
    state.objectBindingKnown = true;
    state.objectEntryOffset = kMode25ObjectEntryOffset;
    state.objectIndex = 0u;
    state.object = &object;
    state.primitiveGroup.known = true;
    state.primitiveGroup.firstPrimitiveIndex = 0u;
    state.primitiveGroup.primitiveCount =
        static_cast<uint16_t>(contract.primitiveCount);
    state.primitiveGroup.mode = kMode25Mode;
    state.primitiveGroup.flag = kMode25Flag;
    state.primitiveGroup.wordStride = kMode25WordStride8004274C;
    state.initialized = true;

    InitializeResult8001AF1C out{};
    out.initialized = true;
    return out;
}

} // namespace

void Clear(RuntimeState8001AF1C& state)
{
    state = {};
}

InitializeResult8001AF1C InitializePa8001AF1C(
    RuntimeState8001AF1C& state,
    const TmdModel* model,
    bool sourceParseComplete)
{
    return InitializeMode25Model8001AF1C(
        state,
        model,
        sourceParseComplete,
        ModelPath::Pa);
}

InitializeResult8001AF1C InitializeLo8001AF1C(
    RuntimeState8001AF1C& state,
    const TmdModel* model,
    bool sourceParseComplete)
{
    return InitializeMode25Model8001AF1C(
        state,
        model,
        sourceParseComplete,
        ModelPath::Lo);
}

InitializeResult8001AF1C InitializeHp8001AF1C(
    RuntimeState8001AF1C& state,
    const TmdModel* model,
    bool sourceParseComplete)
{
    return InitializeMode25Model8001AF1C(
        state,
        model,
        sourceParseComplete,
        ModelPath::Hp);
}

InitializeResult8001AF1C InitializePaKage8001AF1C(
    RuntimeState8001AF1C& state,
    const TmdModel* model,
    bool sourceParseComplete)
{
    return InitializeMode25Model8001AF1C(
        state,
        model,
        sourceParseComplete,
        ModelPath::PaKage);
}

DrawAttr800428B0 DecodeAttr800428B0(
    const RuntimeState8001AF1C& state)
{
    DrawAttr800428B0 out{};
    if (!state.initialized || !state.attrKnown) {
        return out;
    }
    out.known = true;
    out.active = (state.attr & 0x80000000u) == 0u;
    out.bits0To2 = static_cast<uint8_t>(state.attr & 7u);
    out.bits3To4 = static_cast<uint8_t>((state.attr >> 3u) & 3u);
    out.bit5 = ((state.attr >> 5u) & 1u) != 0u;
    out.bit6 = ((state.attr >> 6u) & 1u) != 0u;
    out.bits9To11 = static_cast<uint8_t>((state.attr >> 9u) & 7u);
    out.bit30 = ((state.attr >> 30u) & 1u) != 0u;
    return out;
}

bool IsMode25PrimitiveReady800428B0(
    const RuntimeState8001AF1C& state,
    const TmdObject* expectedObject,
    std::size_t primitiveIndex)
{
    ModelContract8001AF1C contract{};
    if (!TryResolveModelContract8001AF1C(state.modelPath, contract)) {
        return false;
    }

    if (!state.initialized ||
        state.drawDescAddress != contract.drawDescAddress ||
        !state.modelBindingKnown || state.model == nullptr ||
        state.model->id != 0x41u || state.model->flags != 0u ||
        state.model->objects.size() != kMode25ObjectCount ||
        !state.objectBindingKnown || !state.coordBindingKnown ||
        state.coordAddress != contract.coordAddress ||
        state.objectIndex != 0u ||
        state.objectEntryOffset != kMode25ObjectEntryOffset) {
        return false;
    }

    const TmdObject* const boundObject = &state.model->objects[0];
    if (state.object == nullptr || state.object != boundObject ||
        state.object != expectedObject ||
        !HasExactMode25PrimitiveGroup8004274C(*boundObject, contract)) {
        return false;
    }

    const DrawAttr800428B0 attr = DecodeAttr800428B0(state);
    return state.primitiveGroup.known &&
           state.primitiveGroup.firstPrimitiveIndex == 0u &&
           state.primitiveGroup.primitiveCount == contract.primitiveCount &&
           state.primitiveGroup.mode == kMode25Mode &&
           state.primitiveGroup.flag == kMode25Flag &&
           state.primitiveGroup.wordStride == kMode25WordStride8004274C &&
           primitiveIndex < state.primitiveGroup.primitiveCount &&
           primitiveIndex < boundObject->primitives.size() && attr.known &&
           attr.active;
}

bool IsFirstPaKagePrimitiveReady800428B0(
    const RuntimeState8001AF1C& state,
    const TmdObject* expectedObject,
    std::size_t primitiveIndex)
{
    return state.modelPath == ModelPath::PaKage &&
           IsMode25PrimitiveReady800428B0(
               state, expectedObject, primitiveIndex);
}

bool IsFirstLoPrimitiveReady800428B0(
    const RuntimeState8001AF1C& state,
    const TmdObject* expectedObject,
    std::size_t primitiveIndex)
{
    return state.modelPath == ModelPath::Lo &&
           IsMode25PrimitiveReady800428B0(
               state, expectedObject, primitiveIndex);
}

bool IsFirstHpPrimitiveReady800428B0(
    const RuntimeState8001AF1C& state,
    const TmdObject* expectedObject,
    std::size_t primitiveIndex)
{
    return state.modelPath == ModelPath::Hp &&
           IsMode25PrimitiveReady800428B0(
               state, expectedObject, primitiveIndex);
}

} // namespace PrSS0TitleDrawDescDirect
