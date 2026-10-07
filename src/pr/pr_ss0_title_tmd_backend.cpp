#include "pr_ss0_title_tmd_backend.h"

#include "pr_mime.h"
#include "pr_scene_drawbuffer_direct.h"
#include "pr_psx_graph_owner_direct.h"
#include "pr_psx_event_frame_direct.h"
#include "pr_psx_tmd_submit_direct.h"
#include "pr_ss0_scene0_int_renderer_direct.h"
#include "pr_ss0_title_draw_desc_direct.h"
#include "pr_ss0_title_hud_events_direct.h"
#include "pr_ss0_title_packet_commit_direct.h"
#include "pr_ss0_title_packet_work_direct.h"
#include "pr_ss0_title_packet_plan_direct.h"
#include "pr_ss0_title_primitive_group_direct.h"
#include "pr_ss0_transition_direct.h"
#include "pr_ss0_title_transform_direct.h"
#include "pr_stage1_loader_memory_direct.h"
#include "pr_tmd.h"
#include "pr_vram_atlas.h"
#include "resource_manager.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <limits>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace PrSS0TitleTmdBackend {
namespace {

constexpr std::size_t kModelCount =
    static_cast<std::size_t>(ModelKind::Count);
constexpr std::size_t kMimeCount =
    static_cast<std::size_t>(MimeKind::Count);
constexpr std::size_t kTitleTodCount =
    static_cast<std::size_t>(TitleTodKind::Count);
constexpr uint32_t kCompo00HandleFirst80091858 = 2u;
constexpr uint32_t kCompo00HandleLast80091858 = 60u;
constexpr std::size_t kCompo00HandleTableEntryCount80091858 =
    static_cast<std::size_t>(kCompo00HandleLast80091858 + 1u);
constexpr std::size_t kCompo00MemRecordCount80091858 =
    static_cast<std::size_t>(kCompo00HandleLast80091858 -
                             kCompo00HandleFirst80091858 + 1u);
constexpr uint32_t kPracticeYCompoCompositeTimCount80015618 =
    kScene0StartupCompositeTimCount80016B84 +
    PrSS0Scene0IntLoadDirect::kExpectedYCompoTimEntries80015618;
constexpr uint16_t kTitleGraphWidth = 320u;
constexpr uint16_t kTitleGraphHeight = 240u;
constexpr uint16_t kTitleGraphMode = 4u;
constexpr uint32_t kTitleProjection = 440u;
constexpr uint8_t kTitleCameraObjectResource104Index = 27u;
constexpr uint32_t kTitleCameraBezRecordCount = 91u;
constexpr std::size_t kTitleCameraBezRawBytes = 1464u;
constexpr uint32_t kPrimaryPaMimeCursorBegin80014164 = 0u;
constexpr uint32_t kPrimaryPaMimeCursorEnd80014164 = 999u;
constexpr uint32_t kLoHpEventIndex801C5190 = 5u;
constexpr uint32_t kLoHpEventTick96801C5190 = 1728u;
constexpr uint8_t kLoResourcePairIndex801C6C14 = 2u;
constexpr uint8_t kHpResourcePairIndex801C6C14 = 1u;
constexpr uint8_t kShortcutChannel2PairIndex801C64BC = 4u;
constexpr uint8_t kShortcutChannel1PairIndex801C6530 = 2u;
constexpr uint8_t kShortcutChannel3PairIndex801C6578 = 1u;
constexpr uint32_t kShortcutCameraCursor801CB59C = 299u;
constexpr std::size_t kLoMaximumPacketCount801C5EF0 =
    PrSS0TitleDrawDescDirect::kLoPrimitiveCount;
constexpr std::size_t kHpMaximumPacketCount801C5EF0 =
    PrSS0TitleDrawDescDirect::kHpPrimitiveCount;
constexpr int16_t kFirstTitleFaceTimHandle801C5094 = 28;
constexpr int16_t kLastTitleFaceTimHandle801C5094 = 60;
constexpr bool kTitleFaceTimUploadClut801C5094 = true;
constexpr bool kTitleFaceTimFilterRequiredClutRows801C5094 = false;

constexpr std::array<const char*, 33u> kTitleFaceTimNames801C5094 = {{
    "F_PAKU_0.TIM",
    "F_PAKU_1.TIM",
    "F_PAKU_2.TIM",
    "F_PAKU_3.TIM",
    "F_PAKU_4.TIM",
    "F_PAMEL0.TIM",
    "F_PAMEL1.TIM",
    "F_PAMEL2.TIM",
    "F_PAMEL3.TIM",
    "F_PAMEL4.TIM",
    "F_PAMEL5.TIM",
    "F_PAMEL6.TIM",
    "F_PAMEL7.TIM",
    "F_PAMER0.TIM",
    "F_PAMER1.TIM",
    "F_PAMER2.TIM",
    "F_PAMER3.TIM",
    "F_PAMER4.TIM",
    "F_PAMER5.TIM",
    "F_PAMER6.TIM",
    "F_PAMER7.TIM",
    "F_PATBL0.TIM",
    "F_PATBL1.TIM",
    "F_PATBL2.TIM",
    "F_PATBR0.TIM",
    "F_PATBR1.TIM",
    "F_PATBR2.TIM",
    "F_PATFL0.TIM",
    "F_PATFL1.TIM",
    "F_PATFL2.TIM",
    "F_PATFR0.TIM",
    "F_PATFR1.TIM",
    "F_PATFR2.TIM",
}};

static_assert(kTitleFaceTimNames801C5094.size() ==
                  static_cast<std::size_t>(
                      kLastTitleFaceTimHandle801C5094 -
                      kFirstTitleFaceTimHandle801C5094 + 1),
              "title face TIM handle map must remain complete");
static_assert(kTitleHudTimRequestCapacity801C5094 ==
                  PrSS0TitleHudEventsDirect::kHudTimelineMaxTimIds,
              "title HUD TIM request capacity must match the channel-0 list");

std::array<TmdModel, kModelCount> s_models{};
std::array<ModelResourceView, kModelCount> s_modelViews{};
std::array<VdfData, kMimeCount> s_vdfs{};
std::array<DatData, kMimeCount> s_dats{};
std::array<MimeResourceView, kMimeCount> s_mimeViews{};
std::array<TodData, kTitleTodCount> s_titleTods{};
std::array<TitleTodResourceView, kTitleTodCount> s_titleTodViews{};
TodCoordMatrix s_paLoc2Coord{};
PrPsxGraphOwnerDirect::PsxGraphState s_titleGraphState{};
PrSS0TitlePacketWorkDirect::RuntimeState801C609C s_titlePacketWork{};
PrSS0TitleDrawDescDirect::RuntimeState8001AF1C s_firstPaDrawDesc{};
PrSS0TitleDrawDescDirect::RuntimeState8001AF1C s_firstPaKageDrawDesc{};
PrSS0TitleDrawDescDirect::RuntimeState8001AF1C s_firstLoDrawDesc{};
PrSS0TitleDrawDescDirect::RuntimeState8001AF1C s_firstHpDrawDesc{};
PrSS0TitleTransformDirect::Mode25BaseTriangleCarrier
    s_firstPaMode25BaseTriangleCarrier{};
PrSS0TitleTransformDirect::Mode25DeformedTriangleCarrier
    s_firstPaMode25DeformedTriangleCarrier{};
PrSS0TitleTransformDirect::Mode25BaseTriangleCarrier
    s_firstPaKageMode25BaseTriangleCarrier{};
PrSS0TitleTransformDirect::Mode25DeformedTriangleCarrier
    s_firstPaKageMode25DeformedTriangleCarrier{};
PrSS0TitleTransformDirect::Mode25BaseTriangleCarrier
    s_firstLoMode25BaseTriangleCarrier{};
PrSS0TitleTransformDirect::Mode25DeformedTriangleCarrier
    s_firstLoMode25DeformedTriangleCarrier{};
PrSS0TitleTransformDirect::Mode25BaseTriangleCarrier
    s_firstHpMode25BaseTriangleCarrier{};
PrSS0TitleTransformDirect::Mode25DeformedTriangleCarrier
    s_firstHpMode25DeformedTriangleCarrier{};
PsxVramAtlas s_vramAtlas;
PsxVramAtlas* s_directoryAtlasProjection80015788 = nullptr;
PrPsxGraphOwnerDirect::PsxGraphState* s_directoryGraphProjection80015788 = nullptr;
ResourceState s_resourceState{};
TitleMimeInitState801C609C s_titleMimeInitState801C609C{};
std::array<std::vector<TmdVertex>, 4u> s_titleMimeBaseVertices8001371C{};
std::array<std::vector<TmdVertex>, 4u> s_titleMimeRuntimeVertices800139F8{};
ResourceManager* s_titleResourceManager801C5094 = nullptr;
uint32_t s_titleResourceGeneration801C5094 = 0u;
std::array<uint32_t, kCompo00HandleTableEntryCount80091858>
    s_compo00HandleTable80091858{};
std::array<std::string, kCompo00HandleTableEntryCount80091858>
    s_compo00HandleNames80091858{};
RuntimePrimaryPaMimeBinding80014164 s_runtimePrimaryPaMimeBinding{};
RuntimeChannel2MimeBinding801C6410 s_runtimeChannel2MimeBinding{};
RuntimeChannel1MimeBinding801C6410 s_runtimeChannel1MimeBinding{};
RuntimeChannel3MimeBinding801C6410 s_runtimeChannel3MimeBinding{};
RuntimeTitleTodBinding801C6410 s_runtimeTitleTodBinding{};
PrSS0TitleTransformDirect::TitleTodCursorState8001B000
    s_runtimeTitleTodCursor{};
PrPsxGteDirect::Matrix3x4 s_runtimeTitleDrawCoordWorld{};
PrPsxGteDirect::Matrix3x4 s_titlePanelCoord801CD7BC{};
PrSS0TitleTransformDirect::TitleCameraTable801C6FB4
    s_titleCameraTable{};
RuntimeTitleCameraBinding801C6410 s_runtimeTitleCameraBinding{};
PrSS0TitleTransformDirect::TitleCameraCursorState801CB59C
    s_runtimeTitleCameraCursor{};
PrPsxGteDirect::Matrix3x4 s_runtimeTitleCameraMatrix80092880{};
PrPsxGteDirect::Matrix3x4 s_runtimeTitlePanelWithCamera80041A68{};
PrPsxGteDirect::Matrix3x4 s_firstPaRuntimeMatrix80041A68{};
PrPsxGteDirect::Matrix3x4 s_firstPaKageRuntimeMatrix800406D8{};
PrPsxGteDirect::Matrix3x4 s_loCoord801CB5A0{};
PrPsxGteDirect::Matrix3x4 s_hpCoord801CC6BC{};
PrPsxGteDirect::Matrix3x4 s_firstLoRuntimeMatrix80041A68{};
PrPsxGteDirect::Matrix3x4 s_firstHpRuntimeMatrix80041A68{};
PrSS0TitleTransformDirect::Mode25Rtpt3Input
    s_firstPaKageMode25Rtpt3Input801C5E60{};
PrPsxGteDirect::Rtpt3ExactOutput280030
    s_firstPaKageMode25Rtpt3Output800428B0{};
PrPsxGteDirect::Mode25TriangleGeometryTrace
    s_firstPaKageMode25Geometry800428B0{};
PrSS0TitlePacketPlanDirect::BuildResult800428B0
    s_firstPaKageMode25PacketPlan800428B0{};
PrSS0TitlePacketCommitDirect::CommitResult800428B0
    s_firstPaKageMode25PacketCommit800428B0{};
PrSS0TitlePrimitiveGroupDirect::ExecuteResult8004274C
    s_paMode25PrimitiveGroup8004274C{};
PrSS0TitlePrimitiveGroupDirect::ExecuteResult8004274C
    s_paKageMode25PrimitiveGroup8004274C{};
PrSS0TitlePrimitiveGroupDirect::ExecuteResult8004274C
    s_loMode25PrimitiveGroup8004274C{};
PrSS0TitlePrimitiveGroupDirect::ExecuteResult8004274C
    s_hpMode25PrimitiveGroup8004274C{};

void ClearPaMode25PrimitiveGroup8004274C()
{
    s_paMode25PrimitiveGroup8004274C = {};
    s_resourceState.paMode25PrimitiveGroupResultKnown8004274C = false;
}

void ClearPaKageMode25PrimitiveGroup8004274C()
{
    s_paKageMode25PrimitiveGroup8004274C = {};
    s_resourceState.paKageMode25PrimitiveGroupResultKnown8004274C = false;
}

void ClearLoMode25PrimitiveGroup8004274C()
{
    s_loMode25PrimitiveGroup8004274C = {};
    s_resourceState.loMode25PrimitiveGroupResultKnown8004274C = false;
}

void ClearHpMode25PrimitiveGroup8004274C()
{
    s_hpMode25PrimitiveGroup8004274C = {};
    s_resourceState.hpMode25PrimitiveGroupResultKnown8004274C = false;
}

void ClearFirstPaKageMode25PacketCommit800428B0()
{
    s_firstPaKageMode25PacketCommit800428B0 = {};
    s_resourceState.firstPaKageMode25PacketCommitKnown = false;
    ClearPaKageMode25PrimitiveGroup8004274C();
}

void ClearFirstPaKageMode25PacketPlan800428B0()
{
    s_firstPaKageMode25PacketPlan800428B0 = {};
    s_resourceState.firstPaKageMode25PacketPlanKnown = false;
    ClearFirstPaKageMode25PacketCommit800428B0();
}

void ClearFirstPaKageMode25Rtpt3Carrier()
{
    s_firstPaKageMode25Rtpt3Input801C5E60 = {};
    s_firstPaKageMode25Rtpt3Output800428B0 = {};
    s_firstPaKageMode25Geometry800428B0 = {};
    s_resourceState.firstPaKageMode25Rtpt3InputKnown = false;
    s_resourceState.firstPaKageMode25Rtpt3OutputKnown = false;
    s_resourceState.firstPaKageMode25GeometryKnown = false;
    ClearFirstPaKageMode25PacketPlan800428B0();
}

std::size_t ModelIndex(ModelKind kind)
{
    return static_cast<std::size_t>(kind);
}

std::size_t MimeIndex(MimeKind kind)
{
    return static_cast<std::size_t>(kind);
}

std::size_t TitleTodIndex(TitleTodKind kind)
{
    return static_cast<std::size_t>(kind);
}

uint8_t TitleTodHandle(TitleTodKind kind)
{
    switch (kind) {
    case TitleTodKind::PaDance:
        return 2u;
    case TitleTodKind::PaLoc:
        return 3u;
    case TitleTodKind::PaLoc2:
        return 4u;
    case TitleTodKind::Count:
        break;
    }
    return 0u;
}

bool BuildTitleMimeChannelInit8001385C(
    uint8_t channel,
    ModelKind modelKind,
    MimeKind mimeKind,
    uint32_t destinationVertexAddressA4,
    TitleMimeChannelInit8001385C& out)
{
    out = {};
    if (channel == 0u || channel >= 4u ||
        destinationVertexAddressA4 == 0u) {
        return false;
    }
    const std::size_t modelIndex = ModelIndex(modelKind);
    const std::size_t mimeIndex = MimeIndex(mimeKind);
    if (modelIndex >= kModelCount || mimeIndex >= kMimeCount ||
        !s_modelViews[modelIndex].loaded ||
        !s_mimeViews[mimeIndex].pairLoaded ||
        s_models[modelIndex].objects.empty() ||
        s_vdfs[mimeIndex].keys == 0u ||
        s_vdfs[mimeIndex].keys != s_dats[mimeIndex].keys) {
        return false;
    }

    uint64_t vertexCount = 0u;
    std::vector<TmdVertex> snapshot{};
    for (const TmdObject& object : s_models[modelIndex].objects) {
        vertexCount += object.vertices.size();
        if (vertexCount > (std::numeric_limits<uint32_t>::max)()) {
            return false;
        }
        snapshot.insert(snapshot.end(),
                        object.vertices.begin(),
                        object.vertices.end());
    }
    if (snapshot.empty()) {
        return false;
    }

    out.known = true;
    out.channel = channel;
    out.modelKind = modelKind;
    out.mimeKind = mimeKind;
    out.channelStateAddress80074B08 =
        kTitleMimeChannelStateBase80074B08 +
        kTitleMimeChannelStateStride8001385C * channel;
    out.baseSnapshotAddress8006F6A8 =
        kTitleMimeBaseSnapshotBank8006F6A8 +
        kTitleMimeBaseSnapshotStride8001385C * channel;
    out.vdfStateAddress80074B80 =
        kTitleMimeVdfStateBase80074B80 +
        kTitleMimeChannelStateStride8001385C * channel;
    out.vdfWorkspaceAddress8006FB08 =
        kTitleMimeVdfWorkspaceBase8006FB08 +
        kTitleMimeVdfWorkspaceStride8001385C * channel;
    out.scratchAddress80090240 =
        kTitleMimeScratchBase80090240 +
        kTitleMimeScratchStride80013E40 * channel;
    out.destinationVertexAddressA4 = destinationVertexAddressA4;
    out.modelObjectCount =
        static_cast<uint32_t>(s_models[modelIndex].objects.size());
    out.modelVertexCount = static_cast<uint32_t>(vertexCount);
    out.vdfKeyCount = s_vdfs[mimeIndex].keys;
    out.datKeyCount = s_dats[mimeIndex].keys;
    out.tmdBound80013650 = true;
    out.baseSnapshotCommitted8001371C = true;
    out.vdfBound800137BC = true;
    s_titleMimeBaseVertices8001371C[channel] = snapshot;
    s_titleMimeRuntimeVertices800139F8[channel] = std::move(snapshot);
    return true;
}

void InitializeTitleMimeState801C609C()
{
    TitleMimeInitState801C609C next{};

    // 801C609C takes the dword_801C9544==0 route on the loaded S0 image and
    // calls 800139F8 for channels 0..3 before installing the new bindings.
    // The immutable parsed TMDs are the base data; retain an owned snapshot
    // for every initialized channel so a later title re-entry restores the
    // previous runtime vertices before the new 8001385C calls.
    next.baseRestoreCalled800139F8 = true;
    for (std::size_t channel = 0u;
         channel < next.baseChannelRestored800139F8.size();
         ++channel) {
        if (!s_titleMimeBaseVertices8001371C[channel].empty()) {
            s_titleMimeRuntimeVertices800139F8[channel] =
                s_titleMimeBaseVertices8001371C[channel];
        }
        next.baseChannelRestored800139F8[channel] = true;
        ++next.baseRestoreCallCount800139F8;
    }

    // 8001EEE8 clears gp[49..51] and calls 8001EEAC(1), producing an
    // all-active 12x16 transition mask before the title channel setup.
    next.transitionGridInitCalled8001EEE8 = true;
    next.gp49 = 0u;
    next.gp50 = 0u;
    next.gp51 = 0u;
    next.transitionGrid8001EEAC.fill(1u);

    static constexpr struct {
        uint8_t channel;
        ModelKind modelKind;
        MimeKind mimeKind;
        uint32_t destinationVertexAddressA4;
    } kInitialChannels[] = {
        {1u, ModelKind::Lo, MimeKind::Logo, 0x801CB66Cu},
        {2u, ModelKind::PaKage, MimeKind::PaOki, 0x801CD81Cu},
        {3u, ModelKind::Hp, MimeKind::Hiphop, 0x801CC71Cu},
    };
    bool channelsKnown = true;
    for (const auto& spec : kInitialChannels) {
        TitleMimeChannelInit8001385C channel{};
        if (!BuildTitleMimeChannelInit8001385C(
                spec.channel,
                spec.modelKind,
                spec.mimeKind,
                spec.destinationVertexAddressA4,
                channel)) {
            channelsKnown = false;
            continue;
        }
        next.channels[spec.channel] = channel;
        ++next.channelInitCallCount8001385C;
    }

    // 80013E40 clears ten consecutive 512-byte workspaces at 80090240.
    next.scratchClearCalled80013E40 = true;
    next.scratch80090240.fill(0u);
    next.scratchZeroByteCount80013E40 =
        static_cast<uint32_t>(next.scratch80090240.size());
    next.exactCallOrder =
        next.baseRestoreCallCount800139F8 == 4u &&
        next.transitionGridInitCalled8001EEE8 &&
        channelsKnown && next.channelInitCallCount8001385C == 3u &&
        next.scratchClearCalled80013E40;
    next.known = next.exactCallOrder;
    s_titleMimeInitState801C609C = std::move(next);

    s_resourceState.titleMimeInitKnown8001385C =
        s_titleMimeInitState801C609C.known;
    s_resourceState.titleMimeBaseRestoreKnown800139F8 =
        s_titleMimeInitState801C609C.baseRestoreCalled800139F8 &&
        s_titleMimeInitState801C609C.baseRestoreCallCount800139F8 == 4u;
    s_resourceState.titleMimeScratchZeroKnown80013E40 =
        s_titleMimeInitState801C609C.scratchClearCalled80013E40 &&
        s_titleMimeInitState801C609C.scratchZeroByteCount80013E40 ==
            kTitleMimeScratchBytes80013E40;
    s_resourceState.titleTransitionGridInitKnown8001EEE8 =
        s_titleMimeInitState801C609C.transitionGridInitCalled8001EEE8;
}

uint32_t ExpectedTitleTodRawBytes(TitleTodKind kind)
{
    switch (kind) {
    case TitleTodKind::PaDance:
        return 11848u;
    case TitleTodKind::PaLoc:
    case TitleTodKind::PaLoc2:
        return 144u;
    case TitleTodKind::Count:
        break;
    }
    return 0u;
}

uint32_t ExpectedTitleTodBlockCount(TitleTodKind kind)
{
    switch (kind) {
    case TitleTodKind::PaDance:
        return 267u;
    case TitleTodKind::PaLoc:
    case TitleTodKind::PaLoc2:
        return 1u;
    case TitleTodKind::Count:
        break;
    }
    return 0u;
}

bool HasExactEightByteZeroPadding(const std::vector<uint8_t>& raw,
                                  uint64_t payloadBytes)
{
    const uint64_t alignedBytes = (payloadBytes + 7u) & ~uint64_t{7u};
    if (alignedBytes != raw.size()) {
        return false;
    }
    for (uint64_t index = payloadBytes; index < alignedBytes; ++index) {
        if (raw[static_cast<std::size_t>(index)] != 0u) {
            return false;
        }
    }
    return true;
}

bool IsExactVdfPayload(const std::vector<uint8_t>& raw,
                       const VdfData& vdf)
{
    if (raw.size() < 4u || vdf.keys == 0u ||
        vdf.keyList.size() != vdf.keys) {
        return false;
    }
    const uint32_t declaredKeys =
        static_cast<uint32_t>(raw[0]) |
        (static_cast<uint32_t>(raw[1]) << 8u) |
        (static_cast<uint32_t>(raw[2]) << 16u) |
        (static_cast<uint32_t>(raw[3]) << 24u);
    if (declaredKeys != vdf.keys) {
        return false;
    }

    uint64_t expectedBytes = 4u;
    for (const VdfKey& key : vdf.keyList) {
        if (key.nVert != key.deltas.size()) {
            return false;
        }
        expectedBytes += 12u + static_cast<uint64_t>(key.nVert) * 8u;
        if (expectedBytes > raw.size()) {
            return false;
        }
    }
    return HasExactEightByteZeroPadding(raw, expectedBytes);
}

bool IsExactDatPayload(const std::vector<uint8_t>& raw,
                       const DatData& dat)
{
    if (raw.size() < 2u || dat.keys == 0u ||
        dat.keyList.size() != dat.keys) {
        return false;
    }
    const uint16_t declaredKeys = static_cast<uint16_t>(
        static_cast<uint16_t>(raw[0]) |
        static_cast<uint16_t>(static_cast<uint16_t>(raw[1]) << 8u));
    if (declaredKeys != dat.keys) {
        return false;
    }

    uint64_t expectedBytes = 2u;
    uint16_t maxFrames = 0u;
    for (const DatKey& key : dat.keyList) {
        if (key.frames == 0u || key.influence.size() != key.frames) {
            return false;
        }
        maxFrames = (std::max)(maxFrames, key.frames);
        expectedBytes += 2u + static_cast<uint64_t>(key.frames) * 2u;
        if (expectedBytes > raw.size()) {
            return false;
        }
    }
    return dat.maxFrames == maxFrames &&
           HasExactEightByteZeroPadding(raw, expectedBytes);
}

bool IsExactTitleTodPayload(const std::vector<uint8_t>& raw,
                            const TodData& tod,
                            TitleTodKind kind)
{
    const uint32_t expectedRawBytes = ExpectedTitleTodRawBytes(kind);
    const uint32_t expectedBlockCount = ExpectedTitleTodBlockCount(kind);
    if (expectedRawBytes == 0u || expectedBlockCount == 0u ||
        raw.size() != expectedRawBytes || tod.rawBytes.size() != raw.size() ||
        tod.rawBlockCount != expectedBlockCount ||
        tod.blockCount != expectedBlockCount ||
        tod.blocks.size() != expectedBlockCount) {
        return false;
    }

    uint64_t expectedOffset = 8u;
    uint32_t previousTrigger = 0u;
    bool firstBlock = true;
    for (const TodBlock& block : tod.blocks) {
        if (block.rawOffset != expectedOffset ||
            block.cmdCount != block.commands.size() ||
            (!firstBlock && block.triggerTime < previousTrigger)) {
            return false;
        }
        firstBlock = false;
        previousTrigger = block.triggerTime;

        uint64_t expectedBlockBytes = 8u;
        for (const TodCommand& command : block.commands) {
            uint32_t declaredDwords = (command.header >> 24u) & 0xFFu;
            if (declaredDwords == 0u) {
                declaredDwords = 1u;
            }
            if (declaredDwords != command.data.size() + 1u) {
                return false;
            }
            expectedBlockBytes += static_cast<uint64_t>(declaredDwords) * 4u;
        }
        if (block.rawSize != expectedBlockBytes) {
            return false;
        }
        expectedOffset += expectedBlockBytes;
        if (expectedOffset > raw.size()) {
            return false;
        }
    }
    return HasExactEightByteZeroPadding(raw, expectedOffset);
}

bool IsExactTitleCameraBezResource27(
    const std::vector<uint8_t>& raw)
{
    if (raw.size() != kTitleCameraBezRawBytes) {
        return false;
    }
    const uint32_t declaredCount =
        static_cast<uint32_t>(raw[0]) |
        (static_cast<uint32_t>(raw[1]) << 8u) |
        (static_cast<uint32_t>(raw[2]) << 16u) |
        (static_cast<uint32_t>(raw[3]) << 24u);
    if (declaredCount != kTitleCameraBezRecordCount) {
        return false;
    }
    const std::size_t payloadEnd =
        4u + static_cast<std::size_t>(declaredCount) * 16u;
    if (payloadEnd + 4u != raw.size()) {
        return false;
    }
    for (std::size_t index = payloadEnd; index < raw.size(); ++index) {
        if (raw[index] != 0u) {
            return false;
        }
    }
    return true;
}

bool IsFaceTimName(const std::string& name)
{
    return name.size() >= 2 &&
           (name[0] == 'f' || name[0] == 'F') &&
           name[1] == '_';
}

char FoldAsciiCase(char value)
{
    return value >= 'A' && value <= 'Z'
               ? static_cast<char>(value + ('a' - 'A'))
               : value;
}

bool EqualsAsciiCaseInsensitive(const std::string& lhs,
                                const std::string& rhs)
{
    if (lhs.size() != rhs.size()) {
        return false;
    }
    for (std::size_t index = 0; index < lhs.size(); ++index) {
        if (FoldAsciiCase(lhs[index]) != FoldAsciiCase(rhs[index])) {
            return false;
        }
    }
    return true;
}

const char* ExpectedTitleFaceTimName801C5094(int16_t handle)
{
    if (handle < kFirstTitleFaceTimHandle801C5094 ||
        handle > kLastTitleFaceTimHandle801C5094) {
        return nullptr;
    }
    return kTitleFaceTimNames801C5094[static_cast<std::size_t>(
        handle - kFirstTitleFaceTimHandle801C5094)];
}

bool IsExactTitleFaceTimPayload801C5094(
    const std::vector<uint8_t>& raw)
{
    TimImage tim{};
    if (!TimDecoder::Decode(raw.data(), raw.size(), tim)) {
        return false;
    }

    const int page = tim.orgX >= 0 ? tim.orgX / 64 : -1;
    return tim.bpp == 4u && tim.clutW == 16u && tim.clutH == 1u &&
           tim.palette.size() == 16u && tim.orgY >= 0 &&
           tim.orgY < 256 && page >= 5 && page <= 7;
}

struct ResolvedTitleHudTim801C5094 {
    std::string name{};
    std::vector<uint8_t> raw{};
};

bool BuildModelView(const TmdModel& model, ModelResourceView& out)
{
    if (model.objects.empty() ||
        model.objects.size() > (std::numeric_limits<uint32_t>::max)()) {
        return false;
    }

    uint64_t primitiveCount = 0;
    uint64_t texturedPrimitiveCount = 0;
    for (const TmdObject& object : model.objects) {
        primitiveCount += object.primitives.size();
        for (const TmdPrimitive& primitive : object.primitives) {
            if (primitive.textured) {
                ++texturedPrimitiveCount;
            }
        }
    }

    if (primitiveCount == 0 ||
        primitiveCount > (std::numeric_limits<uint32_t>::max)() ||
        texturedPrimitiveCount > (std::numeric_limits<uint32_t>::max)()) {
        return false;
    }

    out.loaded = true;
    out.objectCount = static_cast<uint32_t>(model.objects.size());
    out.primitiveCount = static_cast<uint32_t>(primitiveCount);
    out.texturedPrimitiveCount =
        static_cast<uint32_t>(texturedPrimitiveCount);
    return true;
}

void RegisterModelTextureInputs(const TmdModel& model)
{
    for (const TmdObject& object : model.objects) {
        for (const TmdPrimitive& primitive : object.primitives) {
            if (!primitive.textured) {
                continue;
            }
            s_vramAtlas.RegisterTpage(primitive.tpage);
            s_vramAtlas.RegisterClut(primitive.clut);
        }
    }
}

std::string Scene0IntTimName8001AE7C(
    const std::array<uint8_t, 16>& rawName) {
    std::size_t size = 0u;
    while (size < rawName.size() && rawName[size] != 0u) {
        ++size;
    }
    return std::string(reinterpret_cast<const char*>(rawName.data()), size);
}

bool CanResolveAllTitleTextureInputs8001AE7C(
    const PsxVramAtlas& atlas,
    uint32_t& resolvedCount) {
    resolvedCount = 0u;
    for (std::size_t modelIndex = 0u; modelIndex < kModelCount;
         ++modelIndex) {
        if (!s_modelViews[modelIndex].loaded) {
            return false;
        }
        for (const TmdObject& object : s_models[modelIndex].objects) {
            for (const TmdPrimitive& primitive : object.primitives) {
                if (!primitive.textured) {
                    continue;
                }
                if (!atlas.CanResolveTpageClut(
                        primitive.tpage, primitive.clut)) {
                    return false;
                }
                ++resolvedCount;
            }
        }
    }
    return resolvedCount > 0u;
}

bool IsExactTitleGraphControl801C609C(
    const PrPsxGraphOwnerDirect::PsxGraphState& graph)
{
    return PrPsxGraphOwnerDirect::IsExactTmdFastHandlerTable8001C1E8(graph) &&
           graph.word_80096590 == 0u &&
           graph.word_800965A0 == kTitleGraphMode &&
           graph.word_8008ECA8 ==
               (std::array<int16_t, 2>{{0, 0}}) &&
           graph.word_8008ECAC ==
               (std::array<int16_t, 2>{{0, 240}}) &&
           graph.word_8008EEF0 ==
               (std::array<int16_t, 2>{{0, 0}}) &&
           graph.word_8008EEF4 ==
               (std::array<int16_t, 2>{{0, 0}}) &&
           graph.word_800901C4 == 160 && graph.word_800901C6 == 120 &&
           graph.drawOffset.setDrawEnvCalled &&
           graph.drawOffset.word_800917AA == 0 &&
           graph.drawOffset.word_800917AC == 0 &&
           graph.drawOffset.word_80091730 == 0 &&
           graph.drawOffset.word_80091732 == 0 &&
           graph.drawOffset.word_80091734 == 320 &&
           graph.drawOffset.word_80091736 == 240 &&
           graph.drawOffset.word_80091738 == 160 &&
           graph.drawOffset.word_8009173A == 120 &&
           PrSS0TitleTransformDirect::IsExactTitleGteControl801C609C(
               graph.gte);
}

PrPsxGteDirect::Matrix3x4 BuildStaticTitleCoord8004049C(int32_t z)
{
    PrPsxGteDirect::Matrix3x4 coord{};
    coord.words[0] = 0x00001000u;
    coord.words[2] = 0x00001000u;
    coord.words[4] = 0x00001000u;
    coord.words[7] = static_cast<uint32_t>(z);
    return coord;
}

bool HasPacketCapacity801C5EF0(std::size_t requiredPacketCount)
{
    if (requiredPacketCount == 0u ||
        !s_resourceState.titlePacketFrameKnown801C6410 ||
        !s_titlePacketWork.initialized ||
        !s_titlePacketWork.framePrepared801C6410 ||
        s_titlePacketWork.currentDrawBuffer8004019C >=
            s_titlePacketWork.workLists801C9574.size() ||
        !s_titleGraphState.mainPageWorkLists80087288Initialized ||
        static_cast<uint8_t>(s_titleGraphState.word_80096590 & 1u) !=
            s_titlePacketWork.currentDrawBuffer8004019C ||
        s_titleGraphState.dword_800901C8 !=
            s_titlePacketWork.currentPacketAllocator800901C8) {
        return false;
    }

    const uint8_t lane = s_titlePacketWork.currentDrawBuffer8004019C;
    const uint8_t titleWorkLane =
        PrSS0TitlePacketWorkDirect::kTitleWorkFixedLane801C609C;
    if (titleWorkLane >= s_titlePacketWork.workLists801C9574.size()) {
        return false;
    }
    const auto& work = s_titlePacketWork.workLists801C9574[titleWorkLane];
    const uint64_t arenaBase = s_titlePacketWork.packetArenaBase80025B28;
    const uint64_t laneBytes =
        PrSS0TitlePacketWorkDirect::kPacketArenaLaneBytes801C5D28;
    const uint64_t arenaEnd = arenaBase + 2u * laneBytes;
    const uint64_t expectedHead = static_cast<uint64_t>(
        PrSS0TitlePacketWorkDirect::kWork0OtHead801C959C);
    const uint64_t expectedTail =
        expectedHead +
        (1u << PrSS0TitlePacketWorkDirect::kWorkOrder801C609C) * 4u - 4u;
    if (arenaBase == 0u ||
        arenaEnd > (std::numeric_limits<uint32_t>::max)() ||
        s_titlePacketWork.packetAllocatorBases801C956C[0] != arenaBase ||
        s_titlePacketWork.packetAllocatorBases801C956C[1] !=
            arenaBase + laneBytes ||
        work.order_00 != PrSS0TitlePacketWorkDirect::kWorkOrder801C609C ||
        work.headAddr_04 != expectedHead || work.lastAddr_10 != expectedTail ||
        work.x_08 != 0u || work.y_0C != 0u ||
        !work.tmdOtSlotMirrorKnown || !work.tmdPacketWriteMirrorKnown) {
        return false;
    }
    std::size_t freePacketSlots = 0u;
    for (const auto& packet : work.tmdPacketWriteMirror) {
        if (!packet.valid) {
            ++freePacketSlots;
        }
    }
    if (freePacketSlots < requiredPacketCount) {
        return false;
    }

    const uint64_t laneBase =
        s_titlePacketWork.packetAllocatorBases801C956C[lane];
    const uint64_t laneEnd = laneBase + laneBytes;
    const uint64_t allocator =
        s_titlePacketWork.currentPacketAllocator800901C8;
    const uint64_t requiredBytes =
        requiredPacketCount *
        PrPsxTmdSubmitDirect::kPacketByteSize;
    return allocator >= laneBase && allocator <= laneEnd &&
           requiredBytes <= laneEnd - allocator;
}

bool IsAppliedTitleEvent801C5190(
    const PrSS0TitleHudEventsDirect::TitleEventEffect801C5190& effect)
{
    return effect.known && effect.appliedTick96Known &&
           effect.appliedTick96 >= effect.thresholdTick96;
}

bool BuildRuntimeChannel2MimeBinding801C6410(
    const PrSS0TitleHudEventsDirect::TitleEventEffect801C5190& effect,
    RuntimeChannel2MimeBinding801C6410& next)
{
    if (!IsAppliedTitleEvent801C5190(effect) ||
        !effect.channel2PairWrite ||
        (effect.ctxFlagsSetMask & 0x00010000u) == 0u) {
        return false;
    }

    const MimeKind mimeKind =
        MimeKindFromResourcePairIndex801C6C14(effect.channel2PairIndex);
    const std::size_t mimeIndex = MimeIndex(mimeKind);
    if (mimeKind == MimeKind::Count || mimeIndex >= kMimeCount ||
        !s_mimeViews[mimeIndex].pairLoaded ||
        GetMimeVdf(mimeKind) == nullptr || GetMimeDat(mimeKind) == nullptr) {
        return false;
    }

    next = {};
    next.known = true;
    next.sourceEventIndex = effect.eventIndex;
    next.appliedTick96 = effect.appliedTick96;
    next.resourcePairIndex = effect.channel2PairIndex;
    next.mimeKind = mimeKind;
    next.cursorKnown = true;
    next.cursor = 0u;
    return true;
}

bool BuildRuntimePrimaryPaMimeBinding801C6410(
    const PrSS0TitleHudEventsDirect::TitleEventEffect801C5190& effect,
    RuntimePrimaryPaMimeBinding80014164& next)
{
    if (!IsAppliedTitleEvent801C5190(effect) ||
        !effect.channel2PairWrite ||
        (effect.ctxFlagsSetMask & 0x00010000u) == 0u) {
        return false;
    }

    const MimeKind mimeKind =
        MimeKindFromResourcePairIndex801C6C14(effect.channel2PairIndex);
    const std::size_t mimeIndex = MimeIndex(mimeKind);
    if (mimeKind == MimeKind::Count || mimeIndex >= kMimeCount ||
        !s_mimeViews[mimeIndex].pairLoaded ||
        GetMimeVdf(mimeKind) == nullptr || GetMimeDat(mimeKind) == nullptr) {
        return false;
    }

    next = {};
    next.known = true;
    next.sourceEventIndex = effect.eventIndex;
    next.appliedTick96 = effect.appliedTick96;
    next.resourcePairIndex = effect.channel2PairIndex;
    next.mimeKind = mimeKind;
    next.cursorKnown = true;
    next.cursor = kPrimaryPaMimeCursorBegin80014164;
    next.endKnown = true;
    next.end = kPrimaryPaMimeCursorEnd80014164;
    next.loopFlagKnown = true;
    // 801C6758 passes loopFlag=0 to 800140E0 for event/selector actions.
    next.loopFlag = 0u;
    return true;
}

bool BuildRuntimeChannel1MimeBinding801C6410(
    const PrSS0TitleHudEventsDirect::TitleEventEffect801C5190& effect,
    RuntimeChannel1MimeBinding801C6410& next)
{
    if (!IsAppliedTitleEvent801C5190(effect) ||
        !effect.channel1PairWrite ||
        (effect.ctxFlagsSetMask & 0x00400000u) == 0u) {
        return false;
    }

    const MimeKind mimeKind =
        MimeKindFromResourcePairIndex801C6C14(effect.channel1PairIndex);
    const std::size_t mimeIndex = MimeIndex(mimeKind);
    if (mimeKind == MimeKind::Count || mimeIndex >= kMimeCount ||
        !s_mimeViews[mimeIndex].pairLoaded ||
        GetMimeVdf(mimeKind) == nullptr || GetMimeDat(mimeKind) == nullptr) {
        return false;
    }

    next = {};
    next.known = true;
    next.sourceEventIndex = effect.eventIndex;
    next.thresholdTick96 = effect.thresholdTick96;
    next.appliedTick96 = effect.appliedTick96;
    next.resourcePairIndex = effect.channel1PairIndex;
    next.mimeKind = mimeKind;
    next.cursorKnown = true;
    next.cursor = 0u;
    return true;
}

bool BuildRuntimeChannel3MimeBinding801C6410(
    const PrSS0TitleHudEventsDirect::TitleEventEffect801C5190& effect,
    RuntimeChannel3MimeBinding801C6410& next)
{
    if (!IsAppliedTitleEvent801C5190(effect) ||
        !effect.channel3PairWrite ||
        (effect.ctxFlagsSetMask & 0x01000000u) == 0u) {
        return false;
    }

    const MimeKind mimeKind =
        MimeKindFromResourcePairIndex801C6C14(effect.channel3PairIndex);
    const std::size_t mimeIndex = MimeIndex(mimeKind);
    if (mimeKind == MimeKind::Count || mimeIndex >= kMimeCount ||
        !s_mimeViews[mimeIndex].pairLoaded ||
        GetMimeVdf(mimeKind) == nullptr || GetMimeDat(mimeKind) == nullptr) {
        return false;
    }

    next = {};
    next.known = true;
    next.sourceEventIndex = effect.eventIndex;
    next.thresholdTick96 = effect.thresholdTick96;
    next.appliedTick96 = effect.appliedTick96;
    next.resourcePairIndex = effect.channel3PairIndex;
    next.mimeKind = mimeKind;
    next.cursorKnown = true;
    next.cursor = 0u;
    return true;
}

bool BuildRuntimeTitleTodBinding801C6410(
    const PrSS0TitleHudEventsDirect::TitleEventEffect801C5190& effect,
    RuntimeTitleTodBinding801C6410& nextBinding,
    PrSS0TitleTransformDirect::TitleTodCursorState8001B000& nextCursor)
{
    if (!IsAppliedTitleEvent801C5190(effect) ||
        !effect.todResourceF4Write ||
        (effect.ctxFlagsSetMask & 0x00040000u) == 0u) {
        return false;
    }

    const TitleTodKind todKind =
        TitleTodKindFromHandle801C5190(effect.todResourceF4Index);
    const std::size_t todIndex = TitleTodIndex(todKind);
    if (todKind == TitleTodKind::Count || todIndex >= kTitleTodCount ||
        !s_titleTodViews[todIndex].loaded ||
        GetTitleTod(todKind) == nullptr) {
        return false;
    }

    nextBinding = {};
    nextBinding.known = true;
    nextBinding.sourceEventIndex = effect.eventIndex;
    nextBinding.appliedTick96 = effect.appliedTick96;
    nextBinding.resourceHandle = effect.todResourceF4Index;
    nextBinding.todKind = todKind;
    nextBinding.cursorKnown = true;

    nextCursor = {};
    nextCursor.bindingKnown = true;
    nextCursor.resourceHandle = effect.todResourceF4Index;
    nextCursor.sampleSeqKnown = true;
    return true;
}

bool BuildRuntimeTitleCameraBinding801C6410(
    const PrSS0TitleHudEventsDirect::TitleEventEffect801C5190& effect,
    RuntimeTitleCameraBinding801C6410& nextBinding,
    PrSS0TitleTransformDirect::TitleCameraCursorState801CB59C& nextCursor)
{
    if (!IsAppliedTitleEvent801C5190(effect) ||
        !effect.objectResource104Write ||
        (effect.ctxFlagsSetMask & 0x00000400u) == 0u ||
        effect.objectResource104Index !=
            kTitleCameraObjectResource104Index ||
        !s_resourceState.cmOpBezResource27Loaded ||
        !s_titleCameraTable.known ||
        !s_resourceState.titleCameraTableKnown801C6FB4) {
        return false;
    }

    nextBinding = {};
    nextBinding.known = true;
    nextBinding.objectResource104Known = true;
    nextBinding.objectResource104Index = effect.objectResource104Index;
    nextBinding.sourceEventIndex = effect.eventIndex;
    nextBinding.appliedTick96 = effect.appliedTick96;
    nextBinding.cursorKnown = true;

    nextCursor = {};
    nextCursor.cursorKnown = true;
    return true;
}

void PublishRuntimeChannel2MimeBinding801C6410(
    const RuntimeChannel2MimeBinding801C6410& next)
{
    s_runtimeChannel2MimeBinding = next;
    s_firstPaKageMode25DeformedTriangleCarrier = {};
    s_resourceState.firstPaKageRuntimeMimeBindingKnown = true;
    s_resourceState.firstPaKageRuntimeMimeCursorKnown = true;
    s_resourceState.firstPaKageRuntimeVertexDeformationKnown = false;
    ClearFirstPaKageMode25Rtpt3Carrier();
}

void PublishRuntimePrimaryPaMimeBinding801C6410(
    const RuntimePrimaryPaMimeBinding80014164& next)
{
    s_runtimePrimaryPaMimeBinding = next;
    s_firstPaMode25DeformedTriangleCarrier = {};
    s_resourceState.primaryPaMimeBindingKnown80014164 = next.known;
    s_resourceState.primaryPaMimeCursorKnown800141D8 =
        next.known && next.cursorKnown;
    s_resourceState.firstPaRuntimeVertexDeformationKnown = false;
    ClearPaMode25PrimitiveGroup8004274C();
}

void PublishRuntimeChannel1MimeBinding801C6410(
    const RuntimeChannel1MimeBinding801C6410& next)
{
    s_runtimeChannel1MimeBinding = next;
    s_firstLoMode25DeformedTriangleCarrier = {};
    s_resourceState.runtimeChannel1MimeBindingKnown = true;
    s_resourceState.runtimeChannel1MimeCursorKnown = true;
    s_resourceState.firstLoRuntimeVertexDeformationKnown = false;
    ClearLoMode25PrimitiveGroup8004274C();
}

void PublishRuntimeChannel3MimeBinding801C6410(
    const RuntimeChannel3MimeBinding801C6410& next)
{
    s_runtimeChannel3MimeBinding = next;
    s_firstHpMode25DeformedTriangleCarrier = {};
    s_resourceState.runtimeChannel3MimeBindingKnown = true;
    s_resourceState.runtimeChannel3MimeCursorKnown = true;
    s_resourceState.firstHpRuntimeVertexDeformationKnown = false;
    ClearHpMode25PrimitiveGroup8004274C();
}

void PublishRuntimeTitleTodBinding801C6410(
    const RuntimeTitleTodBinding801C6410& nextBinding,
    const PrSS0TitleTransformDirect::TitleTodCursorState8001B000& nextCursor)
{
    s_runtimeTitleTodBinding = nextBinding;
    s_runtimeTitleTodCursor = nextCursor;
    s_runtimeTitleDrawCoordWorld = {};
    s_resourceState.runtimeTitleTodBindingKnown = true;
    s_resourceState.runtimeTitleTodCursorKnown = true;
    s_resourceState.titleDrawCoordWorldKnown800417A4 = false;
    s_firstPaRuntimeMatrix80041A68 = {};
    s_resourceState.firstPaRuntimeMatrixKnown80041A68 = false;
    s_resourceState.firstPaKageRuntimeMatrixKnown = false;
    ClearPaMode25PrimitiveGroup8004274C();
    ClearFirstPaKageMode25Rtpt3Carrier();
}

void PublishRuntimeTitleCameraBinding801C6410(
    const RuntimeTitleCameraBinding801C6410& nextBinding,
    const PrSS0TitleTransformDirect::TitleCameraCursorState801CB59C&
        nextCursor)
{
    s_runtimeTitleCameraBinding = nextBinding;
    s_runtimeTitleCameraCursor = nextCursor;
    s_runtimeTitleCameraMatrix80092880 = {};
    s_runtimeTitlePanelWithCamera80041A68 = {};
    s_firstPaRuntimeMatrix80041A68 = {};
    s_firstPaKageRuntimeMatrix800406D8 = {};
    s_firstLoRuntimeMatrix80041A68 = {};
    s_firstHpRuntimeMatrix80041A68 = {};
    s_resourceState.runtimeTitleCameraBindingKnown = true;
    s_resourceState.runtimeTitleCameraCursorKnown = true;
    s_resourceState.titleCameraMatrixKnown80092880 = false;
    s_resourceState.titlePanelWithCameraKnown80041A68 = false;
    s_resourceState.firstPaRuntimeMatrixKnown80041A68 = false;
    s_resourceState.firstPaKageRuntimeMatrixKnown = false;
    s_resourceState.firstLoRuntimeMatrixKnown80041A68 = false;
    s_resourceState.firstHpRuntimeMatrixKnown80041A68 = false;
    ClearPaMode25PrimitiveGroup8004274C();
    ClearFirstPaKageMode25Rtpt3Carrier();
    ClearLoMode25PrimitiveGroup8004274C();
    ClearHpMode25PrimitiveGroup8004274C();
}

struct TitleEarlyInputShortcutSetupDraft801C6410 {
    RuntimePrimaryPaMimeBinding80014164 primary{};
    RuntimeChannel2MimeBinding801C6410 channel2{};
    RuntimeChannel1MimeBinding801C6410 channel1{};
    RuntimeChannel3MimeBinding801C6410 channel3{};
    RuntimeTitleCameraBinding801C6410 camera{};
    PrSS0TitleTransformDirect::TitleCameraCursorState801CB59C cameraCursor{};
    uint16_t channel1LoopCount801CC676 = 0u;
};

bool BuildTitleEarlyInputShortcutSetupDraft801C6410(
    TitleEarlyInputShortcutSetupDraft801C6410& draft)
{
    const std::size_t paLeftIndex = MimeIndex(MimeKind::PaLeft);
    const std::size_t logoNewIndex = MimeIndex(MimeKind::LogoNew);
    const std::size_t hiphopIndex = MimeIndex(MimeKind::Hiphop);
    if (!s_resourceState.resourceReady ||
        paLeftIndex >= kMimeCount || logoNewIndex >= kMimeCount ||
        hiphopIndex >= kMimeCount ||
        !s_mimeViews[paLeftIndex].pairLoaded ||
        !s_mimeViews[logoNewIndex].pairLoaded ||
        !s_mimeViews[hiphopIndex].pairLoaded ||
        GetMimeVdf(MimeKind::PaLeft) == nullptr ||
        GetMimeDat(MimeKind::PaLeft) == nullptr ||
        GetMimeVdf(MimeKind::LogoNew) == nullptr ||
        GetMimeDat(MimeKind::LogoNew) == nullptr ||
        GetMimeVdf(MimeKind::Hiphop) == nullptr ||
        GetMimeDat(MimeKind::Hiphop) == nullptr ||
        s_mimeViews[logoNewIndex].datMaxFrames == 0u ||
        !s_runtimeTitleCameraBinding.known ||
        !s_runtimeTitleCameraBinding.cursorKnown ||
        !s_runtimeTitleCameraCursor.cursorKnown ||
        !s_titleCameraTable.known ||
        !s_resourceState.titleCameraTableKnown801C6FB4) {
        return false;
    }

    draft = {};
    draft.channel1LoopCount801CC676 =
        s_mimeViews[logoNewIndex].datMaxFrames;

    draft.primary.known = true;
    draft.primary.earlyInputShortcutSetup801C64BC = true;
    draft.primary.resourcePairIndex = kShortcutChannel2PairIndex801C64BC;
    draft.primary.mimeKind = MimeKind::PaLeft;
    draft.primary.cursorKnown = true;
    draft.primary.cursor = kPrimaryPaMimeCursorBegin80014164;
    draft.primary.endKnown = true;
    draft.primary.end = kPrimaryPaMimeCursorEnd80014164;
    draft.primary.loopFlagKnown = true;
    draft.primary.loopFlag = 0u;

    draft.channel2.known = true;
    draft.channel2.earlyInputShortcutSetup801C64BC = true;
    draft.channel2.resourcePairIndex =
        kShortcutChannel2PairIndex801C64BC;
    draft.channel2.mimeKind = MimeKind::PaLeft;
    draft.channel2.cursorKnown = true;

    draft.channel1.known = true;
    draft.channel1.earlyInputShortcutSetup801C6530 = true;
    draft.channel1.resourcePairIndex =
        kShortcutChannel1PairIndex801C6530;
    draft.channel1.mimeKind = MimeKind::LogoNew;
    draft.channel1.cursorKnown = true;
    draft.channel1.cursor = draft.channel1LoopCount801CC676;

    draft.channel3.known = true;
    draft.channel3.earlyInputShortcutSetup801C6578 = true;
    draft.channel3.resourcePairIndex =
        kShortcutChannel3PairIndex801C6578;
    draft.channel3.mimeKind = MimeKind::Hiphop;
    draft.channel3.cursorKnown = true;
    draft.channel3.cursor = draft.channel1LoopCount801CC676;

    draft.camera = s_runtimeTitleCameraBinding;
    draft.camera.cursorKnown = true;
    draft.camera.cursor = kShortcutCameraCursor801CB59C;
    draft.cameraCursor = s_runtimeTitleCameraCursor;
    draft.cameraCursor.cursorKnown = true;
    draft.cameraCursor.cursor = kShortcutCameraCursor801CB59C;
    return true;
}

bool BuildTitleEarlyInputShortcutReadyTod801C5AB4(
    RuntimeTitleTodBinding801C6410& binding,
    PrSS0TitleTransformDirect::TitleTodCursorState8001B000& cursor)
{
    const TitleTodKind kind = TitleTodKind::PaLoc2;
    const std::size_t index = TitleTodIndex(kind);
    if (index >= kTitleTodCount || !s_titleTodViews[index].loaded ||
        GetTitleTod(kind) == nullptr) {
        return false;
    }

    binding = {};
    binding.known = true;
    binding.earlyInputShortcutReady801C5AB4 = true;
    binding.resourceHandle = TitleTodHandle(kind);
    binding.todKind = kind;
    binding.cursorKnown = true;

    cursor = {};
    cursor.bindingKnown = true;
    cursor.resourceHandle = TitleTodHandle(kind);
    cursor.sampleSeqKnown = true;
    return true;
}

void SyncTitleFrameGateResourceState801C6410()
{
    s_resourceState.titlePacketFrameKnown801C6410 =
        s_titlePacketWork.framePrepared801C6410;
    s_resourceState.titleFrameFlagsApplied801C6410 =
        s_titlePacketWork.frameFlagsApplied801C6410;
    s_resourceState.titleRenderActiveKnown801C9548 = true;
    s_resourceState.loHpGateKnown801CB600 = true;
    s_resourceState.presentExtraKnown801CFAEC = true;
    s_resourceState.titleRenderActive801C9548 =
        s_titlePacketWork.titleRenderActive801C9548;
    s_resourceState.loHpGate801CB600 =
        s_titlePacketWork.loHpGate801CB600;
    s_resourceState.presentExtra801CFAEC =
        s_titlePacketWork.presentExtra801CFAEC;
}

} // namespace

Scene0IntRendererAtlasCandidate8001AE7C::
    Scene0IntRendererAtlasCandidate8001AE7C() = default;
Scene0IntRendererAtlasCandidate8001AE7C::
    ~Scene0IntRendererAtlasCandidate8001AE7C() = default;
Scene0IntRendererAtlasCandidate8001AE7C::
    Scene0IntRendererAtlasCandidate8001AE7C(
        Scene0IntRendererAtlasCandidate8001AE7C&& other) noexcept = default;
Scene0IntRendererAtlasCandidate8001AE7C&
Scene0IntRendererAtlasCandidate8001AE7C::operator=(
    Scene0IntRendererAtlasCandidate8001AE7C&& other) noexcept = default;

void Clear()
{
    s_directoryAtlasProjection80015788 = nullptr;
    s_directoryGraphProjection80015788 = nullptr;
    PrSS0TitleHudEventsDirect::ResetTitleHudComodSource801C6Dxx();
    s_models = {};
    s_modelViews = {};
    s_vdfs = {};
    s_dats = {};
    s_mimeViews = {};
    s_titleTods = {};
    s_titleTodViews = {};
    s_paLoc2Coord = {};
    s_titleGraphState = {};
    PrSS0TitlePacketWorkDirect::Clear(s_titlePacketWork);
    PrSS0TitleDrawDescDirect::Clear(s_firstPaDrawDesc);
    PrSS0TitleDrawDescDirect::Clear(s_firstPaKageDrawDesc);
    PrSS0TitleDrawDescDirect::Clear(s_firstLoDrawDesc);
    PrSS0TitleDrawDescDirect::Clear(s_firstHpDrawDesc);
    s_firstPaMode25BaseTriangleCarrier = {};
    s_firstPaMode25DeformedTriangleCarrier = {};
    s_firstPaKageMode25BaseTriangleCarrier = {};
    s_firstPaKageMode25DeformedTriangleCarrier = {};
    s_firstLoMode25BaseTriangleCarrier = {};
    s_firstLoMode25DeformedTriangleCarrier = {};
    s_firstHpMode25BaseTriangleCarrier = {};
    s_firstHpMode25DeformedTriangleCarrier = {};
    s_runtimePrimaryPaMimeBinding = {};
    s_runtimeChannel2MimeBinding = {};
    s_runtimeChannel1MimeBinding = {};
    s_runtimeChannel3MimeBinding = {};
    s_runtimeTitleTodBinding = {};
    s_runtimeTitleTodCursor = {};
    s_runtimeTitleDrawCoordWorld = {};
    s_titlePanelCoord801CD7BC = {};
    s_titleCameraTable = {};
    s_runtimeTitleCameraBinding = {};
    s_runtimeTitleCameraCursor = {};
    s_runtimeTitleCameraMatrix80092880 = {};
    s_runtimeTitlePanelWithCamera80041A68 = {};
    s_firstPaRuntimeMatrix80041A68 = {};
    s_firstPaKageRuntimeMatrix800406D8 = {};
    s_loCoord801CB5A0 = {};
    s_hpCoord801CC6BC = {};
    s_firstLoRuntimeMatrix80041A68 = {};
    s_firstHpRuntimeMatrix80041A68 = {};
    s_firstPaKageMode25Rtpt3Input801C5E60 = {};
    s_firstPaKageMode25Rtpt3Output800428B0 = {};
    s_firstPaKageMode25Geometry800428B0 = {};
    s_firstPaKageMode25PacketPlan800428B0 = {};
    s_firstPaKageMode25PacketCommit800428B0 = {};
    s_paMode25PrimitiveGroup8004274C = {};
    s_paKageMode25PrimitiveGroup8004274C = {};
    s_loMode25PrimitiveGroup8004274C = {};
    s_hpMode25PrimitiveGroup8004274C = {};
    s_titleMimeInitState801C609C = {};
    for (auto& vertices : s_titleMimeBaseVertices8001371C) {
        vertices.clear();
    }
    for (auto& vertices : s_titleMimeRuntimeVertices800139F8) {
        vertices.clear();
    }
    s_vramAtlas.Clear();
    s_titleResourceManager801C5094 = nullptr;
    s_titleResourceGeneration801C5094 = 0u;
    s_compo00HandleTable80091858.fill(0u);
    s_compo00HandleNames80091858.fill(std::string{});
    s_resourceState = {};
}

void ResetRuntimeTitleState801C609C()
{
    s_titleMimeInitState801C609C = {};
    s_resourceState.titleMimeInitKnown8001385C = false;
    s_resourceState.titleMimeBaseRestoreKnown800139F8 = false;
    s_resourceState.titleMimeScratchZeroKnown80013E40 = false;
    s_resourceState.titleTransitionGridInitKnown8001EEE8 = false;
    s_runtimePrimaryPaMimeBinding = {};
    s_runtimeChannel2MimeBinding = {};
    s_runtimeChannel1MimeBinding = {};
    s_runtimeChannel3MimeBinding = {};
    s_runtimeTitleTodBinding = {};
    s_runtimeTitleTodCursor = {};
    s_runtimeTitleDrawCoordWorld = {};
    s_titlePanelCoord801CD7BC = {};
    s_runtimeTitleCameraBinding = {};
    s_runtimeTitleCameraCursor = {};
    s_runtimeTitleCameraMatrix80092880 = {};
    s_runtimeTitlePanelWithCamera80041A68 = {};
    s_firstPaRuntimeMatrix80041A68 = {};
    s_firstPaKageRuntimeMatrix800406D8 = {};
    s_firstPaMode25DeformedTriangleCarrier = {};
    s_firstPaKageMode25DeformedTriangleCarrier = {};
    s_firstLoMode25DeformedTriangleCarrier = {};
    s_firstHpMode25DeformedTriangleCarrier = {};
    s_firstLoRuntimeMatrix80041A68 = {};
    s_firstHpRuntimeMatrix80041A68 = {};
    s_resourceState.runtimeTitleCameraBindingKnown = false;
    s_resourceState.runtimeTitleCameraCursorKnown = false;
    s_resourceState.titleCameraMatrixKnown80092880 = false;
    s_resourceState.titlePanelWithCameraKnown80041A68 = false;
    s_resourceState.runtimeTitleTodBindingKnown = false;
    s_resourceState.runtimeTitleTodCursorKnown = false;
    s_resourceState.runtimeChannel1MimeBindingKnown = false;
    s_resourceState.runtimeChannel1MimeCursorKnown = false;
    s_resourceState.runtimeChannel3MimeBindingKnown = false;
    s_resourceState.runtimeChannel3MimeCursorKnown = false;
    s_resourceState.titleDrawCoordWorldKnown800417A4 = false;
    s_resourceState.titlePanelCoordKnown801C5D8C = false;
    s_resourceState.firstPaDrawDescKnown8001AF1C = false;
    s_resourceState.firstPaPrimitiveGroupKnown8004274C = false;
    s_resourceState.primaryPaMimeBindingKnown80014164 = false;
    s_resourceState.primaryPaMimeCursorKnown800141D8 = false;
    s_resourceState.firstPaRuntimeVertexDeformationKnown = false;
    s_resourceState.firstPaRuntimeMatrixKnown80041A68 = false;
    s_resourceState.firstPaKageDrawDescKnown8001AF1C = false;
    s_resourceState.firstPaKagePrimitiveGroupKnown8004274C = false;
    s_resourceState.firstPaKageRuntimeMimeBindingKnown = false;
    s_resourceState.firstPaKageRuntimeMimeCursorKnown = false;
    s_resourceState.firstPaKageRuntimeVertexDeformationKnown = false;
    s_resourceState.firstPaKageRuntimeMatrixKnown = false;
    s_resourceState.firstLoDrawDescKnown8001AF1C = false;
    s_resourceState.firstLoPrimitiveGroupKnown8004274C = false;
    s_resourceState.firstLoRuntimeVertexDeformationKnown = false;
    s_resourceState.firstLoRuntimeMatrixKnown80041A68 = false;
    s_resourceState.firstHpDrawDescKnown8001AF1C = false;
    s_resourceState.firstHpPrimitiveGroupKnown8004274C = false;
    s_resourceState.firstHpRuntimeVertexDeformationKnown = false;
    s_resourceState.firstHpRuntimeMatrixKnown80041A68 = false;
    s_resourceState.loCoordKnown8004049C = false;
    s_resourceState.hpCoordKnown8004049C = false;
    PrSS0TitlePacketWorkDirect::ResetTitleFrameGates801C609C(
        s_titlePacketWork);
    s_titlePacketWork.framePrepared801C6410 = false;
    s_titlePacketWork.currentDrawBuffer8004019C = 0;
    s_titlePacketWork.currentPacketAllocator800901C8 = 0;
    s_resourceState.titlePacketFrameKnown801C6410 = false;
    s_resourceState.titleFrameFlagsApplied801C6410 = false;
    s_resourceState.titleRenderActiveKnown801C9548 =
        s_titlePacketWork.initialized;
    s_resourceState.loHpGateKnown801CB600 =
        s_titlePacketWork.initialized;
    s_resourceState.presentExtraKnown801CFAEC =
        s_titlePacketWork.initialized;
    s_resourceState.titleEarlyInputShortcutSetupKnown801C6410 = false;
    s_resourceState.titleEarlyInputShortcutReadyKnown801C6410 = false;
    s_resourceState.titleRenderActive801C9548 = 0u;
    s_resourceState.loHpGate801CB600 = 0u;
    s_resourceState.presentExtra801CFAEC = 0u;
    s_resourceState.titleEarlyInputShortcutLoopCount801CC676 = 0u;
    ClearPaMode25PrimitiveGroup8004274C();
    ClearFirstPaKageMode25Rtpt3Carrier();
    ClearLoMode25PrimitiveGroup8004274C();
    ClearHpMode25PrimitiveGroup8004274C();

    PrSS0TitleDrawDescDirect::Clear(s_firstPaDrawDesc);
    const std::size_t paModelIndex = ModelIndex(ModelKind::Pa);
    const auto paDrawDesc =
        PrSS0TitleDrawDescDirect::InitializePa8001AF1C(
            s_firstPaDrawDesc,
            paModelIndex < kModelCount && s_modelViews[paModelIndex].loaded
                ? &s_models[paModelIndex]
                : nullptr,
            paModelIndex < kModelCount &&
                s_modelViews[paModelIndex].loaded);
    s_resourceState.firstPaDrawDescKnown8001AF1C =
        paDrawDesc.initialized;
    s_resourceState.firstPaPrimitiveGroupKnown8004274C =
        paDrawDesc.initialized && s_firstPaDrawDesc.primitiveGroup.known;

    PrSS0TitleDrawDescDirect::Clear(s_firstPaKageDrawDesc);
    const std::size_t paKageModelIndex = ModelIndex(ModelKind::PaKage);
    const auto drawDesc =
        PrSS0TitleDrawDescDirect::InitializePaKage8001AF1C(
            s_firstPaKageDrawDesc,
            paKageModelIndex < kModelCount &&
                    s_modelViews[paKageModelIndex].loaded
                ? &s_models[paKageModelIndex]
                : nullptr,
            paKageModelIndex < kModelCount &&
                s_modelViews[paKageModelIndex].loaded);
    s_resourceState.firstPaKageDrawDescKnown8001AF1C =
        drawDesc.initialized;
    s_resourceState.firstPaKagePrimitiveGroupKnown8004274C =
        drawDesc.initialized &&
        s_firstPaKageDrawDesc.primitiveGroup.known;

    PrSS0TitleDrawDescDirect::Clear(s_firstLoDrawDesc);
    const std::size_t loModelIndex = ModelIndex(ModelKind::Lo);
    const auto loDrawDesc =
        PrSS0TitleDrawDescDirect::InitializeLo8001AF1C(
            s_firstLoDrawDesc,
            loModelIndex < kModelCount && s_modelViews[loModelIndex].loaded
                ? &s_models[loModelIndex]
                : nullptr,
            loModelIndex < kModelCount &&
                s_modelViews[loModelIndex].loaded);
    s_resourceState.firstLoDrawDescKnown8001AF1C =
        loDrawDesc.initialized;
    s_resourceState.firstLoPrimitiveGroupKnown8004274C =
        loDrawDesc.initialized && s_firstLoDrawDesc.primitiveGroup.known;

    PrSS0TitleDrawDescDirect::Clear(s_firstHpDrawDesc);
    const std::size_t hpModelIndex = ModelIndex(ModelKind::Hp);
    const auto hpDrawDesc =
        PrSS0TitleDrawDescDirect::InitializeHp8001AF1C(
            s_firstHpDrawDesc,
            hpModelIndex < kModelCount && s_modelViews[hpModelIndex].loaded
                ? &s_models[hpModelIndex]
                : nullptr,
            hpModelIndex < kModelCount &&
                s_modelViews[hpModelIndex].loaded);
    s_resourceState.firstHpDrawDescKnown8001AF1C =
        hpDrawDesc.initialized;
    s_resourceState.firstHpPrimitiveGroupKnown8004274C =
        hpDrawDesc.initialized && s_firstHpDrawDesc.primitiveGroup.known;

    if (s_resourceState.resourceReady) {
        s_loCoord801CB5A0 = BuildStaticTitleCoord8004049C(0);
        s_hpCoord801CC6BC = BuildStaticTitleCoord8004049C(-100);
        s_resourceState.loCoordKnown8004049C = true;
        s_resourceState.hpCoordKnown8004049C = true;
    }

    const VdfData* logoVdf = GetMimeVdf(MimeKind::Logo);
    const DatData* logoDat = GetMimeDat(MimeKind::Logo);
    if (s_resourceState.firstPaMode25BaseTriangleKnown &&
        logoVdf != nullptr && logoDat != nullptr) {
        s_runtimePrimaryPaMimeBinding.known = true;
        s_runtimePrimaryPaMimeBinding.initialBinding801C609C = true;
        s_runtimePrimaryPaMimeBinding.mimeKind = MimeKind::Logo;
        s_runtimePrimaryPaMimeBinding.cursorKnown = true;
        s_runtimePrimaryPaMimeBinding.cursor =
            kPrimaryPaMimeCursorBegin80014164;
        s_runtimePrimaryPaMimeBinding.endKnown = true;
        s_runtimePrimaryPaMimeBinding.end = kPrimaryPaMimeCursorEnd80014164;
        s_runtimePrimaryPaMimeBinding.loopFlagKnown = true;
        s_runtimePrimaryPaMimeBinding.loopFlag = 1u;
        s_resourceState.primaryPaMimeBindingKnown80014164 = true;
        s_resourceState.primaryPaMimeCursorKnown800141D8 = true;
    }

    PrSS0TitleTransformDirect::TitlePanelCoordInput801C5D8C panelInput{};
    panelInput.known = true;
    panelInput.deltaX = -163;
    panelInput.totalFrames = 204;
    panelInput.deltaY = 192;
    const auto panel =
        PrSS0TitleTransformDirect::BuildTitlePanelCoord801C5D8C(panelInput);
    if (panel.panelCoordKnown801CD7BC) {
        s_titlePanelCoord801CD7BC = panel.panelCoord801CD7BC;
        s_resourceState.titlePanelCoordKnown801C5D8C = true;
    }

    if (s_resourceState.cmOpBezResource27Loaded &&
        s_titleCameraTable.known &&
        s_resourceState.titleCameraTableKnown801C6FB4) {
        RuntimeTitleCameraBinding801C6410 initialCamera{};
        initialCamera.known = true;
        initialCamera.initialBinding801C609C = true;
        initialCamera.cursorKnown = true;

        PrSS0TitleTransformDirect::TitleCameraCursorState801CB59C
            initialCameraCursor{};
        initialCameraCursor.cursorKnown = true;
        PublishRuntimeTitleCameraBinding801C6410(
            initialCamera, initialCameraCursor);
    }

    const TitleTodKind initialKind = TitleTodKind::PaLoc2;
    const std::size_t initialIndex = TitleTodIndex(initialKind);
    if (initialIndex < kTitleTodCount &&
        s_titleTodViews[initialIndex].loaded) {
        RuntimeTitleTodBinding801C6410 initialBinding{};
        initialBinding.known = true;
        initialBinding.initialBinding801C609C = true;
        initialBinding.resourceHandle = TitleTodHandle(initialKind);
        initialBinding.todKind = initialKind;
        initialBinding.cursorKnown = true;

        PrSS0TitleTransformDirect::TitleTodCursorState8001B000 initialCursor{};
        initialCursor.bindingKnown = true;
        initialCursor.resourceHandle = TitleTodHandle(initialKind);
        initialCursor.sampleSeqKnown = true;
        PublishRuntimeTitleTodBinding801C6410(initialBinding, initialCursor);
        (void)AdvanceRuntimeTitleTod8001B000();
    }

    InitializeTitleMimeState801C609C();
}

const TitleMimeInitState801C609C& GetTitleMimeInitState801C609C()
{
    return s_titleMimeInitState801C609C;
}

bool LoadResources(ResourceManager* resources,
                   ImmutableScene0ComodView comod0,
                   PrStage1LoaderMemoryDirectState* loaderMemory)
{
    Clear();
    if (resources == nullptr) {
        return false;
    }
    if (!PrSS0TitleHudEventsDirect::LoadTitleHudTablesFromComod0(
            comod0.data,
            comod0.size,
            PrSS0TitleHudEventsDirect::kComod0MappedBase801C3870)) {
        return false;
    }
    s_resourceState.titleHudTablesLoadedFromComod0801C6Dxx = true;

    PrSS0TitleTransformDirect::TitleCameraTable801C6FB4 cameraTable{};
    if (!PrSS0TitleTransformDirect::ParseExactTitleCameraTable801C6FB4(
            comod0.data, comod0.size, cameraTable)) {
        return false;
    }
    const std::vector<uint8_t>* rawCameraBez =
        resources->GetMem("cm_op.bez");
    const bool cameraBezLoaded =
        rawCameraBez != nullptr &&
        IsExactTitleCameraBezResource27(*rawCameraBez);

    PrPsxGraphOwnerDirect::PsxInitializeGraphState8003FB9C(
        s_titleGraphState, kTitleGraphWidth, kTitleGraphHeight);
    PrPsxGraphOwnerDirect::PsxCall8003FC14_ApplyGraphModeFlags(
        s_titleGraphState, kTitleGraphMode);
    PrPsxGraphOwnerDirect::PsxCall80040AE4_SetDoubleBufferOffsets(
        s_titleGraphState, 0, 0, 0, kTitleGraphHeight);
    PrPsxGraphOwnerDirect::PsxCall80040B84_ApplyScreenCenterAndDrawOffset(
        s_titleGraphState);
    PrPsxGraphOwnerDirect::PsxCall80040C74_GsSetProjection(
        s_titleGraphState, kTitleProjection);
    s_resourceState.titleGraphControlKnown =
        IsExactTitleGraphControl801C609C(s_titleGraphState);
    for (std::size_t index = 0; index < kTitleTodCount; ++index) {
        const TitleTodKind kind = static_cast<TitleTodKind>(index);
        const std::vector<uint8_t>* rawTod =
            resources->GetMem(TitleTodName(kind));
        if (rawTod == nullptr || rawTod->empty()) {
            continue;
        }

        TodData tod{};
        if (!TodParser::Parse(rawTod->data(), rawTod->size(), tod) ||
            !IsExactTitleTodPayload(*rawTod, tod, kind)) {
            continue;
        }

        s_titleTods[index] = std::move(tod);
        s_titleTodViews[index].loaded = true;
        s_titleTodViews[index].handle = TitleTodHandle(kind);
        s_titleTodViews[index].blockCount =
            s_titleTods[index].blockCount;
        s_titleTodViews[index].rawByteCount =
            static_cast<uint32_t>(rawTod->size());
        ++s_resourceState.loadedTitleTodCount;
    }

    const TodData* paLoc2 = GetTitleTod(TitleTodKind::PaLoc2);
    s_resourceState.paLoc2TodLoaded = paLoc2 != nullptr;
    if (paLoc2 != nullptr) {
        s_resourceState.paLoc2CoordKnown =
            PrSS0TitleTransformDirect::ExtractPaLoc2Coord80028054(
                *paLoc2, s_paLoc2Coord);
    }

    for (std::size_t index = 0; index < kModelCount; ++index) {
        const ModelKind kind = static_cast<ModelKind>(index);
        const std::vector<uint8_t>* rawModel =
            resources->GetMem(ModelName(kind));
        if (rawModel == nullptr || rawModel->empty()) {
            continue;
        }

        TmdModel model{};
        TmdParseReport report{};
        const bool parsed = TmdParser::ParseDetailed(
            rawModel->data(), rawModel->size(), model, report);
        if (!parsed || !report.complete) {
            continue;
        }

        ModelResourceView view{};
        if (!BuildModelView(model, view)) {
            continue;
        }

        s_models[index] = std::move(model);
        s_modelViews[index] = view;
        ++s_resourceState.loadedModelCount;
    }

    for (std::size_t index = 0; index < kMimeCount; ++index) {
        const MimeKind kind = static_cast<MimeKind>(index);
        const std::vector<uint8_t>* rawVdf =
            resources->GetMem(MimeVdfName(kind));
        const std::vector<uint8_t>* rawDat =
            resources->GetMem(MimeDatName(kind));
        if (rawVdf == nullptr || rawDat == nullptr || rawVdf->empty() ||
            rawDat->empty()) {
            continue;
        }

        VdfData vdf{};
        DatData dat{};
        if (!VdfParser::Parse(rawVdf->data(), rawVdf->size(), vdf) ||
            !DatParser::Parse(rawDat->data(), rawDat->size(), dat) ||
            !IsExactVdfPayload(*rawVdf, vdf) ||
            !IsExactDatPayload(*rawDat, dat) || vdf.keys != dat.keys) {
            continue;
        }

        s_vdfs[index] = std::move(vdf);
        s_dats[index] = std::move(dat);
        s_mimeViews[index].pairLoaded = true;
        s_mimeViews[index].vdfKeyCount = s_vdfs[index].keys;
        s_mimeViews[index].datKeyCount = s_dats[index].keys;
        s_mimeViews[index].datMaxFrames = s_dats[index].maxFrames;
        ++s_resourceState.loadedMimePairCount;
    }

    for (std::size_t index = 0; index < kModelCount; ++index) {
        if (s_modelViews[index].loaded) {
            RegisterModelTextureInputs(s_models[index]);
        }
    }
    s_resourceState.registeredTpageCount =
        static_cast<uint32_t>(s_vramAtlas.GetTpageCount());

    const TmdObject* paObject = GetModelObject(ModelKind::Pa, 0u);
    const PrSS0TitleTransformDirect::BaseCarrierResult paBaseCarrier =
        PrSS0TitleTransformDirect::BuildMode25BaseTriangleCarrier801C5E60(
            s_resourceState.titleGraphControlKnown
                ? &s_titleGraphState.gte
                : nullptr,
            paObject,
            0u,
            0u,
            paObject != nullptr,
            PrSS0TitleTransformDirect::ModelPath::Pa);
    if (paBaseCarrier.baseCarrierReady) {
        s_firstPaMode25BaseTriangleCarrier = paBaseCarrier.carrier;
        s_resourceState.firstPaMode25BaseTriangleKnown = true;
    }

    const TmdObject* paKageObject =
        GetModelObject(ModelKind::PaKage, 0u);
    const PrSS0TitleTransformDirect::BaseCarrierResult paKageBaseCarrier =
        PrSS0TitleTransformDirect::BuildMode25BaseTriangleCarrier801C5E60(
            s_resourceState.titleGraphControlKnown
                ? &s_titleGraphState.gte
                : nullptr,
            paKageObject,
            0u,
            0u,
            paKageObject != nullptr,
            PrSS0TitleTransformDirect::ModelPath::PaKage);
    if (paKageBaseCarrier.baseCarrierReady) {
        s_firstPaKageMode25BaseTriangleCarrier = paKageBaseCarrier.carrier;
        s_resourceState.firstPaKageMode25BaseTriangleKnown = true;
        s_resourceState.firstPaKageRuntimeMimeBindingKnown =
            paKageBaseCarrier.carrier.runtimeMimeBindingKnown;
        s_resourceState.firstPaKageRuntimeMimeCursorKnown =
            paKageBaseCarrier.carrier.runtimeMimeCursorKnown;
        s_resourceState.firstPaKageRuntimeVertexDeformationKnown =
            paKageBaseCarrier.carrier.runtimeVertexDeformationKnown;
        s_resourceState.firstPaKageRuntimeMatrixKnown =
            paKageBaseCarrier.carrier.runtimeMatrixKnown;
        s_resourceState.firstPaKageMode25Rtpt3InputKnown =
            paKageBaseCarrier.carrier.rtpt3InputKnown;
        s_resourceState.firstPaKageMode25Rtpt3OutputKnown = false;
        s_resourceState.firstPaKageMode25GeometryKnown = false;
    }

    const TmdObject* loObject = GetModelObject(ModelKind::Lo, 0u);
    const auto loBaseCarrier =
        PrSS0TitleTransformDirect::BuildMode25BaseTriangleCarrier801C5E60(
            s_resourceState.titleGraphControlKnown
                ? &s_titleGraphState.gte
                : nullptr,
            loObject,
            0u,
            0u,
            loObject != nullptr,
            PrSS0TitleTransformDirect::ModelPath::Lo);
    if (loBaseCarrier.baseCarrierReady) {
        s_firstLoMode25BaseTriangleCarrier = loBaseCarrier.carrier;
        s_resourceState.firstLoMode25BaseTriangleKnown = true;
    }

    const TmdObject* hpObject = GetModelObject(ModelKind::Hp, 0u);
    const auto hpBaseCarrier =
        PrSS0TitleTransformDirect::BuildMode25BaseTriangleCarrier801C5E60(
            s_resourceState.titleGraphControlKnown
                ? &s_titleGraphState.gte
                : nullptr,
            hpObject,
            0u,
            0u,
            hpObject != nullptr,
            PrSS0TitleTransformDirect::ModelPath::Hp);
    if (hpBaseCarrier.baseCarrierReady) {
        s_firstHpMode25BaseTriangleCarrier = hpBaseCarrier.carrier;
        s_resourceState.firstHpMode25BaseTriangleKnown = true;
    }

    const std::vector<std::string> timNames = resources->GetTimRawNames();
    for (const std::string& timName : timNames) {
        if (IsFaceTimName(timName)) {
            continue;
        }

        const std::vector<uint8_t>* rawTim = resources->GetTimRaw(timName);
        if (rawTim == nullptr || rawTim->empty()) {
            continue;
        }
        if (s_vramAtlas.LoadTim(
                rawTim->data(), rawTim->size(), timName)) {
            ++s_resourceState.loadedTimCount;
        }
    }

    const bool baseResourceReady =
        s_resourceState.loadedModelCount ==
            static_cast<uint32_t>(ModelKind::Count) &&
        s_resourceState.registeredTpageCount > 0u &&
        s_resourceState.loadedTimCount > 0u;
    if (baseResourceReady) {
        s_titleCameraTable = cameraTable;
        s_resourceState.scene0ComodAccepted = true;
        s_resourceState.titleCameraTableKnown801C6FB4 =
            cameraTable.known;
        s_resourceState.cmOpBezResource27Loaded = cameraBezLoaded;
        const auto packetWork =
            PrSS0TitlePacketWorkDirect::Initialize801C609C(
                s_titlePacketWork, loaderMemory, s_titleGraphState);
        s_resourceState.titlePacketWorkKnown801C609C =
            packetWork.initialized &&
            packetWork.packetLaneGraphBindingApplied8001E33C;
    }
    s_resourceState.resourceReady = IsResourceIngressReady(s_resourceState);
    if (s_resourceState.resourceReady) {
        s_titleResourceManager801C5094 = resources;
        s_titleResourceGeneration801C5094 = resources->GetGeneration();
        s_resourceState.titleHudTimResourceBindingKnown801C5094 = true;
        s_resourceState.titleHudTimResourceGeneration801C5094 =
            s_titleResourceGeneration801C5094;
    }
    ResetRuntimeTitleState801C609C();
    return s_resourceState.resourceReady;
}

bool CommitCompo00HandleTable80091858(
    const PrStage1LoaderMemoryDirectState& loaderMemory,
    const ResourceManager& resources)
{
    // The loader-memory transaction is the translated owner of the PSX
    // dword_80091858 stack.  Do not infer a handle from an INT order until the
    // committed stack shape and the resource generation agree.
    if (!s_resourceState.resourceReady ||
        s_titleResourceManager801C5094 != &resources ||
        s_titleResourceGeneration801C5094 != resources.GetGeneration() ||
        loaderMemory.gpPlus324StackDepth != kCompo00HandleLast80091858 ||
        loaderMemory.stackTable80091858[0u] != 0u ||
        loaderMemory.stackTable80091858[1u] == 0u) {
        return false;
    }

    for (uint32_t handle = kCompo00HandleFirst80091858;
         handle <= kCompo00HandleLast80091858; ++handle) {
        if (loaderMemory.stackTable80091858[handle] == 0u) {
            return false;
        }
    }
    for (std::size_t index = kCompo00HandleLast80091858 + 1u;
         index < loaderMemory.stackTable80091858.size(); ++index) {
        if (loaderMemory.stackTable80091858[index] != 0u) {
            return false;
        }
    }

    const std::vector<std::string>& orderedNames =
        resources.GetMemNamesOrdered();
    if (orderedNames.size() != kCompo00MemRecordCount80091858) {
        return false;
    }
    for (const std::string& name : orderedNames) {
        if (name.empty()) {
            return false;
        }
    }

    std::array<uint32_t, kCompo00HandleTableEntryCount80091858>
        tableCandidate{};
    std::array<std::string, kCompo00HandleTableEntryCount80091858>
        nameCandidate{};
    for (std::size_t index = 0; index < orderedNames.size(); ++index) {
        const uint32_t handle =
            kCompo00HandleFirst80091858 + static_cast<uint32_t>(index);
        tableCandidate[handle] = loaderMemory.stackTable80091858[handle];
        nameCandidate[handle] = orderedNames[index];
    }

    s_compo00HandleTable80091858 = std::move(tableCandidate);
    s_compo00HandleNames80091858 = std::move(nameCandidate);
    s_resourceState.compo00HandleTableKnown80091858 = true;
    s_resourceState.compo00HandleTableCommitted80091858 = true;
    s_resourceState.compo00HandleTableGeneration80091858 =
        resources.GetGeneration();
    s_resourceState.compo00HandleTableNonZeroCount80091858 =
        static_cast<uint32_t>(kCompo00MemRecordCount80091858 + 1u);
    return true;
}

namespace {

using ArchiveKind8001AC18 =
    PrSS0Scene0IntLoadDirect::ArchiveKind8001AC18;

static_assert(
    kScene0StartupCompositeTimCount80016B84 ==
        PrSS0Scene0IntLoadDirect::kExpectedCommonTimEntries80016B84 +
            PrSS0Scene0IntLoadDirect::kExpectedZCompoTimEntries80015590 +
            PrSS0Scene0IntLoadDirect::kExpectedTimEntries8001AC18);

ArchiveKind8001AC18 ArchiveKindForRendererAtlasProfile8001AE7C(
    Scene0IntRendererAtlasProfile8001AE7C profile) {
    if (profile ==
            Scene0IntRendererAtlasProfile8001AE7C::Scene0Compo00 ||
        profile ==
            Scene0IntRendererAtlasProfile8001AE7C::Scene0StartupComposite) {
        return ArchiveKind8001AC18::Scene0Compo00;
    }
    if (profile ==
        Scene0IntRendererAtlasProfile8001AE7C::PracticeYCompo) {
        return ArchiveKind8001AC18::PracticeYCompo;
    }
    return ArchiveKind8001AC18::Unknown;
}

bool IsExactIntLoadForRendererAtlasProfile8001AE7C(
    Scene0IntRendererAtlasProfile8001AE7C profile,
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& intLoad) {
    const ArchiveKind8001AC18 archiveKind =
        ArchiveKindForRendererAtlasProfile8001AE7C(profile);
    if (intLoad.archiveKind != archiveKind) {
        return false;
    }
    if (profile ==
            Scene0IntRendererAtlasProfile8001AE7C::Scene0Compo00 ||
        profile ==
            Scene0IntRendererAtlasProfile8001AE7C::Scene0StartupComposite) {
        return PrSS0Scene0IntLoadDirect::
            IsExactAcceptedTransaction8001AC18(intLoad);
    }
    if (profile ==
        Scene0IntRendererAtlasProfile8001AE7C::PracticeYCompo) {
        return PrSS0Scene0IntLoadDirect::
            IsExactAcceptedYCompoTransaction80015618(intLoad);
    }
    return false;
}

bool LoadExactIntTimEntriesIntoAtlas8001AE7C(
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& intLoad,
    PsxVramAtlas& atlas,
    uint32_t& loadedTimCount) {
    loadedTimCount = 0u;
    for (const auto& entry : intLoad.entries) {
        if (entry.type !=
            PrSS0Scene0IntLoadDirect::BlockType8001A8F0::Tim) {
            continue;
        }
        if (entry.dataOffset > intLoad.archiveBytes.size() ||
            entry.size > intLoad.archiveBytes.size() - entry.dataOffset) {
            return false;
        }
        const auto offset = static_cast<std::size_t>(entry.dataOffset);
        const uint8_t* raw = intLoad.archiveBytes.data() + offset;
        const std::string name = Scene0IntTimName8001AE7C(entry.name);
        if (!atlas.CanLoadTim(raw, entry.size, name, true, false) ||
            !atlas.LoadTim(raw, entry.size, name, true, false)) {
            return false;
        }
        ++loadedTimCount;
    }
    return loadedTimCount == intLoad.timEntryCount;
}

Scene0IntRendererAtlasCandidate8001AE7C
BuildRendererAtlasCandidateForProfile8001AE7C(
    const PrSS0Scene0IntRendererDirect::Transaction8001AE7C& transaction,
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& intLoad,
    Scene0IntRendererAtlasProfile8001AE7C profile,
    uint32_t expectedSourceTimCount,
    uint32_t expectedAtlasTimCount,
    bool requireTitleTextureInputs,
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18*
        startupCommonIntLoad,
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18*
        startupZCompoIntLoad,
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18*
        startupCompoIntLoad) {
    Scene0IntRendererAtlasCandidate8001AE7C candidate{};
    const ArchiveKind8001AC18 archiveKind =
        ArchiveKindForRendererAtlasProfile8001AE7C(profile);
    if (!transaction.accepted || !transaction.complete ||
        transaction.status !=
            PrSS0Scene0IntRendererDirect::Status8001AE7C::Accepted ||
        !transaction.sourceIntLoadKnown ||
        !transaction.sourceLoaderCandidateKnown ||
        !transaction.sourceGpuTransactionKnown ||
        transaction.sourceArchiveKind != archiveKind ||
        !transaction.sourcePartialVramAuthority ||
        !transaction.requestBatchAuthority ||
        transaction.sourceTimRequestCount != expectedSourceTimCount ||
        transaction.timProjectionRequests.size() !=
            transaction.sourceTimRequestCount ||
        transaction.cpuAtlasCandidatePrepared ||
        transaction.cpuAtlasCommitted || transaction.d3dUploadCommitted ||
        transaction.fullVramBackingAuthority ||
        transaction.replayValueAuthority ||
        transaction.hostFilesystemAuthority ||
        transaction.consumerReadAuthority || transaction.oldWinS0Authority ||
        transaction.stage2PlusAuthority ||
        !IsExactIntLoadForRendererAtlasProfile8001AE7C(profile, intLoad) ||
        !s_resourceState.resourceReady) {
        return candidate;
    }

    const bool startupComposite =
        profile == Scene0IntRendererAtlasProfile8001AE7C::
                       Scene0StartupComposite;
    const bool practiceComposite =
        profile == Scene0IntRendererAtlasProfile8001AE7C::PracticeYCompo;
    if ((startupComposite || practiceComposite) &&
        (startupCommonIntLoad == nullptr ||
         startupZCompoIntLoad == nullptr ||
         !PrSS0Scene0IntLoadDirect::
             IsExactAcceptedStartupCommonTransaction80016B84(
                 *startupCommonIntLoad) ||
         !PrSS0Scene0IntLoadDirect::
             IsExactAcceptedZCompoTransaction80015590(
                 *startupZCompoIntLoad))) {
        return candidate;
    }
    if (practiceComposite &&
        (startupCompoIntLoad == nullptr ||
         !PrSS0Scene0IntLoadDirect::IsExactAcceptedTransaction8001AC18(
             *startupCompoIntLoad))) {
        return candidate;
    }
    if (!startupComposite && !practiceComposite &&
        (startupCommonIntLoad != nullptr || startupZCompoIntLoad != nullptr ||
         startupCompoIntLoad != nullptr)) {
        return candidate;
    }

    auto atlas = std::make_unique<PsxVramAtlas>();
    if (startupComposite || practiceComposite) {
        uint32_t commonTimCount = 0u;
        uint32_t zCompoTimCount = 0u;
        if (!LoadExactIntTimEntriesIntoAtlas8001AE7C(
                *startupCommonIntLoad, *atlas, commonTimCount) ||
            commonTimCount != PrSS0Scene0IntLoadDirect::
                                  kExpectedCommonTimEntries80016B84 ||
            !LoadExactIntTimEntriesIntoAtlas8001AE7C(
                *startupZCompoIntLoad, *atlas, zCompoTimCount) ||
            zCompoTimCount != PrSS0Scene0IntLoadDirect::
                                  kExpectedZCompoTimEntries80015590) {
            return candidate;
        }
        if (practiceComposite) {
            uint32_t compoTimCount = 0u;
            if (!LoadExactIntTimEntriesIntoAtlas8001AE7C(
                    *startupCompoIntLoad, *atlas, compoTimCount) ||
                compoTimCount !=
                    PrSS0Scene0IntLoadDirect::kExpectedTimEntries8001AC18) {
                return candidate;
            }
        }
    }
    for (const auto& request : transaction.timProjectionRequests) {
        if (request.archiveDataOffset > intLoad.archiveBytes.size() ||
            request.sourceBytes >
                intLoad.archiveBytes.size() - request.archiveDataOffset) {
            return candidate;
        }
        const uint8_t* raw =
            intLoad.archiveBytes.data() + request.archiveDataOffset;
        const std::string name = Scene0IntTimName8001AE7C(request.name);
        if (!atlas->CanLoadTim(raw,
                               request.sourceBytes,
                               name,
                               true,
                               false) ||
            !atlas->LoadTim(raw,
                            request.sourceBytes,
                            name,
                            true,
                            false)) {
            return candidate;
        }
    }

    uint32_t resolvedTextureInputs = 0u;
    if (atlas->GetLoadedCount() !=
            static_cast<int>(expectedAtlasTimCount) ||
        atlas->GetTpageCount() <= 0) {
        return candidate;
    }
    if ((startupComposite || practiceComposite) &&
        (!atlas->CanResolveTpageClut(0x003Cu, 0x7AD0u) ||
         !atlas->CanResolveTpageClut(0x0035u, 0x033Cu))) {
        return candidate;
    }
    if (requireTitleTextureInputs &&
        (!CanResolveAllTitleTextureInputs8001AE7C(
             *atlas, resolvedTextureInputs) ||
         resolvedTextureInputs == 0u)) {
        return candidate;
    }

    candidate.profile = profile;
    candidate.prepared = true;
    candidate.projectedTimCount =
        static_cast<uint32_t>(atlas->GetLoadedCount());
    candidate.projectedTpageCount =
        static_cast<uint32_t>(atlas->GetTpageCount());
    candidate.resolvedTextureInputCount = resolvedTextureInputs;
    candidate.atlas = std::move(atlas);
    return candidate;
}

bool IsExactRendererAtlasCandidateShapeForProfile8001AE7C(
    const Scene0IntRendererAtlasCandidate8001AE7C& candidate,
    const PrSS0Scene0IntRendererDirect::Transaction8001AE7C& transaction,
    Scene0IntRendererAtlasProfile8001AE7C profile,
    uint32_t expectedSourceTimCount,
    uint32_t expectedAtlasTimCount,
    bool requireTitleTextureInputs) {
    const ArchiveKind8001AC18 archiveKind =
        ArchiveKindForRendererAtlasProfile8001AE7C(profile);
    const bool textureInputsExact = requireTitleTextureInputs
        ? candidate.resolvedTextureInputCount > 0u
        : candidate.resolvedTextureInputCount == 0u;
    const bool preservesStartupComposite =
        profile == Scene0IntRendererAtlasProfile8001AE7C::
                       Scene0StartupComposite ||
        profile == Scene0IntRendererAtlasProfile8001AE7C::PracticeYCompo;
    const bool compositeTextureInputsExact =
        !preservesStartupComposite ||
        (candidate.atlas != nullptr &&
         candidate.atlas->CanResolveTpageClut(0x003Cu, 0x7AD0u) &&
         candidate.atlas->CanResolveTpageClut(0x0035u, 0x033Cu));
    return candidate.profile == profile && candidate.prepared &&
           candidate.atlas != nullptr &&
           candidate.projectedTimCount == expectedAtlasTimCount &&
           candidate.projectedTpageCount > 0u &&
           candidate.partialVramProjected &&
           candidate.partialVramKnownWordCount > 0u &&
           textureInputsExact && compositeTextureInputsExact &&
           candidate.atlas->GetLoadedCount() ==
               static_cast<int>(candidate.projectedTimCount) &&
           candidate.atlas->GetTpageCount() ==
               static_cast<int>(candidate.projectedTpageCount) &&
           transaction.accepted && transaction.complete &&
           transaction.status ==
               PrSS0Scene0IntRendererDirect::Status8001AE7C::Accepted &&
           transaction.sourceIntLoadKnown &&
           transaction.sourceLoaderCandidateKnown &&
           transaction.sourceGpuTransactionKnown &&
           transaction.sourceArchiveKind == archiveKind &&
           transaction.sourcePartialVramAuthority &&
           transaction.requestBatchAuthority &&
           transaction.sourceTimRequestCount == expectedSourceTimCount &&
           transaction.timProjectionRequests.size() ==
               expectedSourceTimCount &&
           !transaction.cpuAtlasCandidatePrepared &&
           !transaction.cpuAtlasCommitted &&
           !transaction.d3dUploadCommitted &&
           !transaction.fullVramBackingAuthority &&
           !transaction.replayValueAuthority &&
           !transaction.hostFilesystemAuthority &&
           !transaction.consumerReadAuthority &&
           !transaction.oldWinS0Authority &&
           !transaction.stage2PlusAuthority;
}

bool IsExactRendererAtlasCandidateForProfile8001AE7C(
    const Scene0IntRendererAtlasCandidate8001AE7C& candidate,
    const PrSS0Scene0IntRendererDirect::Transaction8001AE7C& transaction,
    Scene0IntRendererAtlasProfile8001AE7C profile,
    uint32_t expectedSourceTimCount,
    uint32_t expectedAtlasTimCount,
    bool requireTitleTextureInputs) {
    return IsExactRendererAtlasCandidateShapeForProfile8001AE7C(
               candidate,
               transaction,
               profile,
               expectedSourceTimCount,
               expectedAtlasTimCount,
               requireTitleTextureInputs) &&
           !candidate.d3dUploadAttempted &&
           !candidate.d3dUploadComplete && candidate.d3dTpageCount == 0u &&
           candidate.d3dDirtyTpageCount == 0u &&
           candidate.d3dCreatedTpageCount == 0u &&
           candidate.d3dUpdatedTpageCount == 0u &&
           candidate.d3dFailedTpageCount == 0u &&
           candidate.d3dReadyTpageCount == 0u;
}

bool CommitRendererAtlasCandidateForProfile8001AE7C(
    Scene0IntRendererAtlasCandidate8001AE7C&& candidate,
    PrSS0Scene0IntRendererDirect::Transaction8001AE7C& transaction,
    Scene0IntRendererAtlasProfile8001AE7C profile,
    uint32_t expectedSourceTimCount,
    uint32_t expectedAtlasTimCount,
    bool requireTitleTextureInputs) {
    if (!IsExactRendererAtlasCandidateForProfile8001AE7C(
            candidate,
            transaction,
            profile,
            expectedSourceTimCount,
            expectedAtlasTimCount,
            requireTitleTextureInputs)) {
        return false;
    }
    transaction.cpuAtlasCandidatePrepared = true;
    const bool committed =
        profile ==
                Scene0IntRendererAtlasProfile8001AE7C::PracticeYCompo
            ? PrSS0Scene0IntRendererDirect::
                  CommitYCompoCpuAtlasProjection8001AE7C(transaction)
            : PrSS0Scene0IntRendererDirect::
                  CommitCpuAtlasProjection8001AE7C(transaction);
    if (!committed) {
        transaction.cpuAtlasCandidatePrepared = false;
        return false;
    }

    s_vramAtlas = std::move(*candidate.atlas);
    s_resourceState.loadedTimCount = candidate.projectedTimCount;
    s_resourceState.scene0IntRendererAtlasProfile8001AE7C = profile;
    s_resourceState.scene0IntRendererProjectionKnown8001AE7C = true;
    s_resourceState.scene0IntRendererSourcePartialVramAuthority8001AE7C =
        true;
    s_resourceState.scene0IntRendererCpuAtlasCommitted8001AE7C = true;
    s_resourceState.scene0IntRendererD3DUploadAttempted8001AE7C = false;
    s_resourceState.scene0IntRendererD3DUploadCommitted8001AE7C = false;
    s_resourceState.scene0IntRendererFullVramBackingAuthority8001AE7C =
        false;
    s_resourceState.scene0IntRendererProjectedTimCount8001AE7C =
        candidate.projectedTimCount;
    s_resourceState.scene0IntRendererProjectedTpageCount8001AE7C =
        candidate.projectedTpageCount;
    s_resourceState.scene0IntRendererResolvedTextureInputCount8001AE7C =
        candidate.resolvedTextureInputCount;
    s_resourceState.scene0IntRendererD3DUploadAttemptCount8001AE7C = 0u;
    s_resourceState.scene0IntRendererD3DReadyTpageCount8001AE7C = 0u;
    s_resourceState.scene0IntRendererD3DFailedTpageCount8001AE7C = 0u;
    return true;
}

}  // namespace

Scene0IntRendererAtlasCandidate8001AE7C
BuildScene0IntRendererAtlasCandidate8001AE7C(
    const PrSS0Scene0IntRendererDirect::Transaction8001AE7C& transaction,
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& commonIntLoad,
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& zCompoIntLoad,
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& compoIntLoad) {
    return BuildRendererAtlasCandidateForProfile8001AE7C(
        transaction,
        compoIntLoad,
        Scene0IntRendererAtlasProfile8001AE7C::Scene0StartupComposite,
        PrSS0Scene0IntLoadDirect::kExpectedTimEntries8001AC18,
        kScene0StartupCompositeTimCount80016B84,
        true,
        &commonIntLoad,
        &zCompoIntLoad,
        &compoIntLoad);
}

bool IsExactScene0IntRendererAtlasCandidate8001AE7C(
    const Scene0IntRendererAtlasCandidate8001AE7C& candidate,
    const PrSS0Scene0IntRendererDirect::Transaction8001AE7C& transaction,
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& commonIntLoad,
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& zCompoIntLoad,
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& compoIntLoad) {
    return PrSS0Scene0IntLoadDirect::
               IsExactAcceptedStartupCommonTransaction80016B84(
                   commonIntLoad) &&
           PrSS0Scene0IntLoadDirect::
               IsExactAcceptedZCompoTransaction80015590(zCompoIntLoad) &&
           PrSS0Scene0IntLoadDirect::IsExactAcceptedTransaction8001AC18(
               compoIntLoad) &&
           IsExactRendererAtlasCandidateForProfile8001AE7C(
               candidate,
               transaction,
               Scene0IntRendererAtlasProfile8001AE7C::
                   Scene0StartupComposite,
               PrSS0Scene0IntLoadDirect::kExpectedTimEntries8001AC18,
               kScene0StartupCompositeTimCount80016B84,
               true);
}

bool CommitScene0IntRendererAtlasCandidate8001AE7C(
    Scene0IntRendererAtlasCandidate8001AE7C&& candidate,
    PrSS0Scene0IntRendererDirect::Transaction8001AE7C& transaction,
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& commonIntLoad,
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& zCompoIntLoad,
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& compoIntLoad) {
    if (!IsExactScene0IntRendererAtlasCandidate8001AE7C(
            candidate,
            transaction,
            commonIntLoad,
            zCompoIntLoad,
            compoIntLoad)) {
        return false;
    }
    return CommitRendererAtlasCandidateForProfile8001AE7C(
        std::move(candidate),
        transaction,
        Scene0IntRendererAtlasProfile8001AE7C::Scene0StartupComposite,
        PrSS0Scene0IntLoadDirect::kExpectedTimEntries8001AC18,
        kScene0StartupCompositeTimCount80016B84,
        true);
}

bool ApplyRendererAtlasPartialVram8001AE7C(
    Scene0IntRendererAtlasCandidate8001AE7C& candidate,
    const PrSS0Scene0IntGpuDirect::Transaction8001AE7C& gpuTransaction) {
    if (!candidate.prepared || candidate.atlas == nullptr ||
        !gpuTransaction.accepted || !gpuTransaction.complete ||
        !gpuTransaction.partialVramCandidateKnown ||
        !gpuTransaction.directVramCommitted ||
        !gpuTransaction.partialVramWordAuthority ||
        gpuTransaction.partialVramCandidate == nullptr ||
        gpuTransaction.finalKnownWordCount == 0u ||
        gpuTransaction.finalKnownWordCount !=
            gpuTransaction.partialVramCandidate->knownWordCount) {
        return false;
    }
    const auto& partial = *gpuTransaction.partialVramCandidate;
    if (!candidate.atlas->ApplyPartialVramWords(
            partial.words.data(), partial.known.data(), partial.words.size())) {
        return false;
    }
    candidate.partialVramProjected = true;
    candidate.partialVramKnownWordCount = partial.knownWordCount;
    return true;
}

Scene0IntRendererAtlasCandidate8001AE7C
BuildPracticeYCompoRendererAtlasCandidate8001AE7C(
    const PrSS0Scene0IntRendererDirect::Transaction8001AE7C& transaction,
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& intLoad,
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& startupCommonIntLoad,
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& startupZCompoIntLoad,
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& startupCompoIntLoad) {
    return BuildRendererAtlasCandidateForProfile8001AE7C(
        transaction,
        intLoad,
        Scene0IntRendererAtlasProfile8001AE7C::PracticeYCompo,
        PrSS0Scene0IntLoadDirect::kExpectedYCompoTimEntries80015618,
        kPracticeYCompoCompositeTimCount80015618,
        false,
        &startupCommonIntLoad,
        &startupZCompoIntLoad,
        &startupCompoIntLoad);
}

bool IsExactPracticeYCompoRendererAtlasCandidate8001AE7C(
    const Scene0IntRendererAtlasCandidate8001AE7C& candidate,
    const PrSS0Scene0IntRendererDirect::Transaction8001AE7C& transaction) {
    return IsExactRendererAtlasCandidateForProfile8001AE7C(
        candidate,
        transaction,
        Scene0IntRendererAtlasProfile8001AE7C::PracticeYCompo,
        PrSS0Scene0IntLoadDirect::kExpectedYCompoTimEntries80015618,
        kPracticeYCompoCompositeTimCount80015618,
        false);
}

bool IsExactPracticeYCompoRendererAtlasD3DCandidate8001AE7C(
    const Scene0IntRendererAtlasCandidate8001AE7C& candidate,
    const PrSS0Scene0IntRendererDirect::Transaction8001AE7C& transaction) {
    const uint64_t resolvedDirtyTpageCount =
        static_cast<uint64_t>(candidate.d3dCreatedTpageCount) +
        static_cast<uint64_t>(candidate.d3dUpdatedTpageCount) +
        static_cast<uint64_t>(candidate.d3dFailedTpageCount);
    return IsExactRendererAtlasCandidateShapeForProfile8001AE7C(
               candidate,
               transaction,
               Scene0IntRendererAtlasProfile8001AE7C::PracticeYCompo,
               PrSS0Scene0IntLoadDirect::kExpectedYCompoTimEntries80015618,
               kPracticeYCompoCompositeTimCount80015618,
               false) &&
           candidate.d3dUploadAttempted && candidate.d3dUploadComplete &&
           candidate.d3dTpageCount == candidate.projectedTpageCount &&
           candidate.d3dTpageCount > 0u &&
           candidate.d3dDirtyTpageCount <= candidate.d3dTpageCount &&
           resolvedDirtyTpageCount == candidate.d3dDirtyTpageCount &&
           candidate.d3dFailedTpageCount == 0u &&
           candidate.d3dReadyTpageCount == candidate.d3dTpageCount;
}

bool PreparePracticeYCompoRendererAtlasD3DCandidate8001AE7C(
    Scene0IntRendererAtlasCandidate8001AE7C& candidate,
    const PrSS0Scene0IntRendererDirect::Transaction8001AE7C&
        rendererTransaction,
    D3D11Renderer* renderer) {
    if (renderer == nullptr ||
        !IsExactPracticeYCompoRendererAtlasCandidate8001AE7C(
            candidate, rendererTransaction)) {
        return false;
    }

    const PsxVramAtlasUploadResult uploaded =
        candidate.atlas->UploadAll(renderer);
    candidate.d3dUploadAttempted = uploaded.rendererKnown;
    candidate.d3dUploadComplete = uploaded.complete;
    candidate.d3dTpageCount = uploaded.tpageCount;
    candidate.d3dDirtyTpageCount = uploaded.dirtyTpageCount;
    candidate.d3dCreatedTpageCount = uploaded.createdTpageCount;
    candidate.d3dUpdatedTpageCount = uploaded.updatedTpageCount;
    candidate.d3dFailedTpageCount = uploaded.failedTpageCount;
    candidate.d3dReadyTpageCount = uploaded.readyTpageCount;
    return IsExactPracticeYCompoRendererAtlasD3DCandidate8001AE7C(
        candidate, rendererTransaction);
}

bool CommitPracticeYCompoRendererAtlasCandidate8001AE7C(
    Scene0IntRendererAtlasCandidate8001AE7C&& candidate,
    PrSS0Scene0IntRendererDirect::Transaction8001AE7C& rendererTransaction,
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& intLoad,
    const PrSS0Scene0IntSideEffectDirect::Transaction8001A8F0&
        loaderCandidate,
    const PrSS0Scene0IntGpuDirect::Transaction8001AE7C& gpuTransaction) {
    if (!IsExactPracticeYCompoRendererAtlasD3DCandidate8001AE7C(
            candidate, rendererTransaction) ||
        !PrSS0Scene0IntRendererDirect::
            IsExactPreparedYCompoTransaction8001AE7C(
                rendererTransaction,
                intLoad,
                loaderCandidate,
                gpuTransaction)) {
        return false;
    }

    const auto transactionBeforeCommit = rendererTransaction;
    rendererTransaction.cpuAtlasCandidatePrepared = true;
    if (!PrSS0Scene0IntRendererDirect::
            CommitYCompoCpuAtlasProjection8001AE7C(rendererTransaction) ||
        !PrSS0Scene0IntRendererDirect::
            IsExactCpuAtlasCommittedYCompoTransaction8001AE7C(
                rendererTransaction,
                intLoad,
                loaderCandidate,
                gpuTransaction) ||
        !PrSS0Scene0IntRendererDirect::
            CommitYCompoD3DAtlasProjection8001AE7C(
                rendererTransaction,
                intLoad,
                loaderCandidate,
                gpuTransaction) ||
        !PrSS0Scene0IntRendererDirect::
            IsExactCommittedYCompoTransaction8001AE7C(
                rendererTransaction,
                intLoad,
                loaderCandidate,
                gpuTransaction)) {
        rendererTransaction = transactionBeforeCommit;
        return false;
    }

    s_vramAtlas = std::move(*candidate.atlas);
    s_resourceState.loadedTimCount = candidate.projectedTimCount;
    s_resourceState.scene0IntRendererAtlasProfile8001AE7C =
        Scene0IntRendererAtlasProfile8001AE7C::PracticeYCompo;
    s_resourceState.scene0IntRendererProjectionKnown8001AE7C = true;
    s_resourceState.scene0IntRendererSourcePartialVramAuthority8001AE7C =
        true;
    s_resourceState.scene0IntRendererCpuAtlasCommitted8001AE7C = true;
    s_resourceState.scene0IntRendererD3DUploadAttempted8001AE7C =
        candidate.d3dUploadAttempted;
    s_resourceState.scene0IntRendererD3DUploadCommitted8001AE7C =
        candidate.d3dUploadComplete;
    s_resourceState.scene0IntRendererFullVramBackingAuthority8001AE7C =
        false;
    s_resourceState.scene0IntRendererProjectedTimCount8001AE7C =
        candidate.projectedTimCount;
    s_resourceState.scene0IntRendererProjectedTpageCount8001AE7C =
        candidate.projectedTpageCount;
    s_resourceState.scene0IntRendererResolvedTextureInputCount8001AE7C =
        candidate.resolvedTextureInputCount;
    s_resourceState.scene0IntRendererD3DUploadAttemptCount8001AE7C = 1u;
    s_resourceState.scene0IntRendererD3DReadyTpageCount8001AE7C =
        candidate.d3dReadyTpageCount;
    s_resourceState.scene0IntRendererD3DFailedTpageCount8001AE7C =
        candidate.d3dFailedTpageCount;
    return true;
}

TitleHudTimApplyResult801C5094 ApplyTitleHudTimRequest801C5094(
    ResourceManager* resources,
    const PrSS0TitleHudEventsDirect::TitleHudTimRequest801C5094& request)
{
    TitleHudTimApplyResult801C5094 result{};
    for (std::size_t index = 0;
         index < kTitleHudTimRequestCapacity801C5094;
         ++index) {
        result.requestedIds[index] = request.timIds[index];
    }

    if (!request.valid) {
        return result;
    }

    std::size_t terminatorIndex = kTitleHudTimRequestCapacity801C5094;
    for (std::size_t index = 0;
         index < kTitleHudTimRequestCapacity801C5094;
         ++index) {
        if (request.timIds[index] == 0) {
            terminatorIndex = index;
            break;
        }
    }
    if (terminatorIndex == kTitleHudTimRequestCapacity801C5094) {
        result.status =
            TitleHudTimApplyStatus801C5094::MissingZeroTerminator;
        result.requestedCount = static_cast<uint32_t>(terminatorIndex);
        return result;
    }

    result.zeroTerminated = true;
    result.requestedCount = static_cast<uint32_t>(terminatorIndex);
    if (terminatorIndex == 0u) {
        result.status = TitleHudTimApplyStatus801C5094::EmptyRequest;
        return result;
    }
    if (request.timIdCount != result.requestedCount) {
        result.status = TitleHudTimApplyStatus801C5094::TimIdCountMismatch;
        return result;
    }
    for (std::size_t index = terminatorIndex + 1u;
         index < kTitleHudTimRequestCapacity801C5094;
         ++index) {
        if (request.timIds[index] != 0) {
            result.status =
                TitleHudTimApplyStatus801C5094::NonZeroAfterTerminator;
            return result;
        }
    }

    if (resources == nullptr) {
        result.status =
            TitleHudTimApplyStatus801C5094::ResourceManagerMissing;
        return result;
    }
    result.resourceGeneration = resources->GetGeneration();
    if (!s_resourceState.resourceReady ||
        !s_resourceState.titleHudTimResourceBindingKnown801C5094 ||
        !s_resourceState.compo00HandleTableKnown80091858 ||
        !s_resourceState.compo00HandleTableCommitted80091858 ||
        s_resourceState.compo00HandleTableGeneration80091858 !=
            resources->GetGeneration() ||
        s_titleResourceManager801C5094 == nullptr) {
        result.status = TitleHudTimApplyStatus801C5094::BackendNotReady;
        return result;
    }
    if (resources != s_titleResourceManager801C5094) {
        result.status =
            TitleHudTimApplyStatus801C5094::ResourceManagerMismatch;
        return result;
    }
    if (result.resourceGeneration != s_titleResourceGeneration801C5094) {
        result.status =
            TitleHudTimApplyStatus801C5094::ResourceGenerationMismatch;
        return result;
    }

    const std::vector<std::string>& orderedNames =
        resources->GetMemNamesOrdered();
    if (orderedNames.size() != kCompo00MemRecordCount80091858) {
        result.status = TitleHudTimApplyStatus801C5094::BackendNotReady;
        return result;
    }
    std::array<ResolvedTitleHudTim801C5094,
               kTitleHudTimRequestCapacity801C5094>
        resolved{};
    for (std::size_t index = 0; index < terminatorIndex; ++index) {
        const int16_t handle = request.timIds[index];
        TitleHudTimResolution801C5094& resolution =
            result.resolutions[index];
        resolution.handle = handle;

        const char* expectedName = ExpectedTitleFaceTimName801C5094(handle);
        if (expectedName == nullptr) {
            result.status =
                TitleHudTimApplyStatus801C5094::TimHandleOutOfRange;
            return result;
        }
        resolution.expectedName = expectedName;

        if (handle < static_cast<int16_t>(kCompo00HandleFirst80091858) ||
            handle > static_cast<int16_t>(kCompo00HandleLast80091858) ||
            s_compo00HandleTable80091858[static_cast<std::size_t>(handle)] ==
                0u ||
            s_compo00HandleNames80091858[static_cast<std::size_t>(handle)]
                .empty()) {
            result.status =
                TitleHudTimApplyStatus801C5094::MemHandleOutOfRange;
            return result;
        }

        resolution.memRecordResolved = true;
        const std::size_t memIndex = static_cast<std::size_t>(handle - 2);
        if (memIndex >= orderedNames.size()) {
            result.status = TitleHudTimApplyStatus801C5094::MemHandleOutOfRange;
            return result;
        }
        resolution.resolvedMemName = orderedNames[memIndex];
        if (!EqualsAsciiCaseInsensitive(
                resolution.resolvedMemName,
                s_compo00HandleNames80091858[static_cast<std::size_t>(handle)])) {
            result.status = TitleHudTimApplyStatus801C5094::MemNameMismatch;
            return result;
        }
        // Resolve from the committed dword_80091858 snapshot after the
        // generation/order cross-check; the ordered vector is only a guard
        // against a stale ResourceManager mutation.
        resolution.resolvedMemName =
            s_compo00HandleNames80091858[static_cast<std::size_t>(handle)];
        if (!EqualsAsciiCaseInsensitive(
                resolution.resolvedMemName, resolution.expectedName)) {
            result.status = TitleHudTimApplyStatus801C5094::MemNameMismatch;
            return result;
        }

        std::size_t expectedNameOccurrences = 0u;
        for (const std::string& orderedName : orderedNames) {
            if (EqualsAsciiCaseInsensitive(
                    orderedName, resolution.expectedName)) {
                ++expectedNameOccurrences;
            }
        }
        if (expectedNameOccurrences != 1u) {
            result.status =
                TitleHudTimApplyStatus801C5094::MemNameAmbiguous;
            return result;
        }
        resolution.expectedNameMatched = true;
        resolved[index].name = resolution.resolvedMemName;
    }

    if (resources->GetGeneration() != s_titleResourceGeneration801C5094) {
        result.status =
            TitleHudTimApplyStatus801C5094::ResourceGenerationMismatch;
        return result;
    }

    for (std::size_t index = 0; index < terminatorIndex; ++index) {
        TitleHudTimResolution801C5094& resolution =
            result.resolutions[index];
        const std::vector<uint8_t>* raw =
            resources->GetMem(resolution.resolvedMemName);
        if (raw == nullptr || raw->empty()) {
            result.status =
                TitleHudTimApplyStatus801C5094::MemBytesUnavailable;
            return result;
        }
        resolved[index].raw = *raw;
        resolution.rawBytesResolved = true;
        resolution.rawByteCount = resolved[index].raw.size();

        if (!IsExactTitleFaceTimPayload801C5094(resolved[index].raw)) {
            result.status =
                TitleHudTimApplyStatus801C5094::TimMetadataMismatch;
            return result;
        }
        resolution.metadataAccepted = true;
    }

    if (resources->GetGeneration() != s_titleResourceGeneration801C5094) {
        result.status =
            TitleHudTimApplyStatus801C5094::ResourceGenerationMismatch;
        return result;
    }

    bool allPreflightAccepted = true;
    for (std::size_t index = 0; index < terminatorIndex; ++index) {
        const ResolvedTitleHudTim801C5094& tim = resolved[index];
        const bool accepted = s_vramAtlas.CanLoadTim(
            tim.raw.data(),
            tim.raw.size(),
            tim.name,
            kTitleFaceTimUploadClut801C5094,
            kTitleFaceTimFilterRequiredClutRows801C5094);
        result.resolutions[index].preflightAccepted = accepted;
        allPreflightAccepted = allPreflightAccepted && accepted;
    }
    if (!allPreflightAccepted) {
        result.status =
            TitleHudTimApplyStatus801C5094::AtlasPreflightRejected;
        return result;
    }
    if (resources->GetGeneration() != s_titleResourceGeneration801C5094) {
        result.status =
            TitleHudTimApplyStatus801C5094::ResourceGenerationMismatch;
        return result;
    }

    const uint32_t requestedCount = result.requestedCount;
    const uint32_t maxCounter = (std::numeric_limits<uint32_t>::max)();
    if (s_resourceState.runtimeTitleHudTimLoadedCount801C5094 >
            maxCounter - requestedCount ||
        s_resourceState.runtimeTitleHudTimBatchCount801C5094 == maxCounter) {
        result.status = TitleHudTimApplyStatus801C5094::CounterOverflow;
        return result;
    }

    for (std::size_t index = 0; index < terminatorIndex; ++index) {
        const ResolvedTitleHudTim801C5094& tim = resolved[index];
        if (!s_vramAtlas.LoadTim(
                tim.raw.data(),
                tim.raw.size(),
                tim.name,
                kTitleFaceTimUploadClut801C5094,
                kTitleFaceTimFilterRequiredClutRows801C5094)) {
            result.status = TitleHudTimApplyStatus801C5094::
                CommitFailedResourcesInvalidated;
            Clear();
            return result;
        }
    }
    if (resources != s_titleResourceManager801C5094 ||
        resources->GetGeneration() != s_titleResourceGeneration801C5094) {
        result.status = TitleHudTimApplyStatus801C5094::
            CommitFailedResourcesInvalidated;
        Clear();
        return result;
    }

    result.status = TitleHudTimApplyStatus801C5094::Accepted;
    result.accepted = true;
    result.appliedCount = requestedCount;
    for (std::size_t index = 0; index < terminatorIndex; ++index) {
        result.resolutions[index].applied = true;
    }

    ++s_resourceState.runtimeTitleHudTimBatchCount801C5094;
    s_resourceState.runtimeTitleHudTimLoadedCount801C5094 += requestedCount;
    s_resourceState.runtimeTitleHudTimRequestKnown801C5094 = true;
    s_resourceState.runtimeTitleHudTimLastRequestedCount801C5094 =
        requestedCount;
    s_resourceState.runtimeTitleHudTimLastAppliedCount801C5094 =
        requestedCount;
    for (std::size_t index = 0;
         index < kTitleHudTimRequestCapacity801C5094;
         ++index) {
        s_resourceState.runtimeTitleHudTimLastIds801C5094[index] =
            result.requestedIds[index];
    }
    return result;
}

const char* TitleHudTimApplyStatusName801C5094(
    TitleHudTimApplyStatus801C5094 status)
{
    switch (status) {
    case TitleHudTimApplyStatus801C5094::InvalidRequest:
        return "InvalidRequest";
    case TitleHudTimApplyStatus801C5094::BackendNotReady:
        return "BackendNotReady";
    case TitleHudTimApplyStatus801C5094::ResourceManagerMissing:
        return "ResourceManagerMissing";
    case TitleHudTimApplyStatus801C5094::ResourceManagerMismatch:
        return "ResourceManagerMismatch";
    case TitleHudTimApplyStatus801C5094::ResourceGenerationMismatch:
        return "ResourceGenerationMismatch";
    case TitleHudTimApplyStatus801C5094::MissingZeroTerminator:
        return "MissingZeroTerminator";
    case TitleHudTimApplyStatus801C5094::EmptyRequest:
        return "EmptyRequest";
    case TitleHudTimApplyStatus801C5094::TimIdCountMismatch:
        return "TimIdCountMismatch";
    case TitleHudTimApplyStatus801C5094::NonZeroAfterTerminator:
        return "NonZeroAfterTerminator";
    case TitleHudTimApplyStatus801C5094::TimHandleOutOfRange:
        return "TimHandleOutOfRange";
    case TitleHudTimApplyStatus801C5094::MemHandleOutOfRange:
        return "MemHandleOutOfRange";
    case TitleHudTimApplyStatus801C5094::MemNameMismatch:
        return "MemNameMismatch";
    case TitleHudTimApplyStatus801C5094::MemNameAmbiguous:
        return "MemNameAmbiguous";
    case TitleHudTimApplyStatus801C5094::MemBytesUnavailable:
        return "MemBytesUnavailable";
    case TitleHudTimApplyStatus801C5094::TimMetadataMismatch:
        return "TimMetadataMismatch";
    case TitleHudTimApplyStatus801C5094::AtlasPreflightRejected:
        return "AtlasPreflightRejected";
    case TitleHudTimApplyStatus801C5094::CounterOverflow:
        return "CounterOverflow";
    case TitleHudTimApplyStatus801C5094::CommitFailedResourcesInvalidated:
        return "CommitFailedResourcesInvalidated";
    case TitleHudTimApplyStatus801C5094::Accepted:
        return "Accepted";
    }
    return "InvalidRequest";
}

ResourceState GetResourceState()
{
    return s_resourceState;
}

bool ApplyScene0LoaderResetPreservingTitlePacketArena80025A34(
    PrStage1LoaderMemoryDirectState& loaderMemory)
{
    const auto result =
        PrSS0TitlePacketWorkDirect::
            ApplyLoaderReset80025A34PreservingPacketArena801C4260(
                s_titlePacketWork, s_titleGraphState, loaderMemory);
    return result.applied;
}

void BindResidentDirectoryRenderProjection80015788(
    PsxVramAtlas* atlas, PrPsxGraphOwnerDirect::PsxGraphState* graph) {
    s_directoryAtlasProjection80015788 = atlas;
    s_directoryGraphProjection80015788 = graph;
}

TitleVramAtlasUploadResult801C689C PrepareTitleVramAtlas801C689C(
    D3D11Renderer* renderer)
{
    auto& atlas = s_directoryAtlasProjection80015788
        ? *s_directoryAtlasProjection80015788 : s_vramAtlas;
    const PsxVramAtlasUploadResult uploaded = atlas.UploadAll(renderer);
    TitleVramAtlasUploadResult801C689C result{};
    result.attempted = uploaded.rendererKnown;
    result.complete = uploaded.complete;
    result.sourceResidentDirectoryCpuAtlas = s_directoryAtlasProjection80015788 != nullptr;
    result.sourceScene0IntCpuAtlasAuthority =
        !result.sourceResidentDirectoryCpuAtlas &&
        s_resourceState.scene0IntRendererProjectionKnown8001AE7C &&
        s_resourceState.
            scene0IntRendererSourcePartialVramAuthority8001AE7C &&
        s_resourceState.scene0IntRendererCpuAtlasCommitted8001AE7C;
    result.sourceAtlasProfile8001AE7C =
        s_resourceState.scene0IntRendererAtlasProfile8001AE7C;
    result.visiblePresentAuthority = false;
    result.tpageCount = uploaded.tpageCount;
    result.dirtyTpageCount = uploaded.dirtyTpageCount;
    result.createdTpageCount = uploaded.createdTpageCount;
    result.updatedTpageCount = uploaded.updatedTpageCount;
    result.failedTpageCount = uploaded.failedTpageCount;
    result.readyTpageCount = uploaded.readyTpageCount;

    if (result.sourceScene0IntCpuAtlasAuthority && result.attempted) {
        s_resourceState.scene0IntRendererD3DUploadAttempted8001AE7C = true;
        ++s_resourceState.scene0IntRendererD3DUploadAttemptCount8001AE7C;
        s_resourceState.scene0IntRendererD3DReadyTpageCount8001AE7C =
            result.readyTpageCount;
        s_resourceState.scene0IntRendererD3DFailedTpageCount8001AE7C =
            result.failedTpageCount;
        s_resourceState.scene0IntRendererD3DUploadCommitted8001AE7C =
            result.complete;
    }
    return result;
}

ID3D11ShaderResourceView* ResolveTitleTpageSRV801C689C(
    D3D11Renderer* renderer,
    uint16_t tpage,
    uint16_t clut)
{
    return s_vramAtlas.GetTpageSRV(tpage, clut, renderer);
}

ID3D11ShaderResourceView* ResolveTitleStandaloneTimSRV801C689C(
    D3D11Renderer* renderer,
    uint16_t orgX,
    uint16_t orgY,
    uint16_t width,
    uint16_t height,
    uint16_t clutX,
    uint16_t clutY,
    int psxAbr)
{
    if (s_directoryAtlasProjection80015788 && renderer) {
        return s_directoryAtlasProjection80015788->GetStandaloneTimSRV(
            orgX, orgY, width, height, clutX, clutY, renderer, psxAbr);
    }
    if (!renderer ||
        !s_resourceState.scene0IntRendererProjectionKnown8001AE7C ||
        !s_resourceState.
             scene0IntRendererSourcePartialVramAuthority8001AE7C ||
        !s_resourceState.scene0IntRendererCpuAtlasCommitted8001AE7C) {
        return nullptr;
    }
    return s_vramAtlas.GetStandaloneTimSRV(
        orgX, orgY, width, height, clutX, clutY, renderer, psxAbr);
}

TitleTpageExactResolveResult801C689C
ResolveTitleTpageSRVExact801C689C(D3D11Renderer* renderer,
                                  uint16_t tpage,
                                  uint16_t clut)
{
    TitleTpageExactResolveResult801C689C result{};
    result.sourceResidentDirectoryCpuAtlas = s_directoryAtlasProjection80015788 != nullptr;
    auto& atlas = s_directoryAtlasProjection80015788
        ? *s_directoryAtlasProjection80015788 : s_vramAtlas;
    result.sourceScene0IntCpuAtlasAuthority =
        !result.sourceResidentDirectoryCpuAtlas &&
        s_resourceState.scene0IntRendererProjectionKnown8001AE7C &&
        s_resourceState.
            scene0IntRendererSourcePartialVramAuthority8001AE7C &&
        s_resourceState.scene0IntRendererCpuAtlasCommitted8001AE7C;
    if (!renderer || !result.HasNativeCpuAtlasAuthority() ||
        !atlas.CanResolveTpageClut(tpage, clut)) {
        return result;
    }

    result.tpageClutResolvable = true;
    result.clutStpKnown =
        atlas.TryGetClutHasStpBits(clut, result.clutHasStpBits);
    if (!result.clutStpKnown) {
        return result;
    }
    result.srv = atlas.GetTpageSRV(tpage, clut, renderer);
    if (result.clutHasStpBits) {
        result.psxAbr0StpSrv =
            atlas.GetTpagePsxAbr0StpSRV(tpage, clut, renderer);
        result.psxAbr1StpSrv =
            atlas.GetTpagePsxAbr1StpSRV(tpage, clut, renderer);
    }
    return result;
}

ModelResourceView GetModelResourceView(ModelKind kind)
{
    const std::size_t index = ModelIndex(kind);
    if (index >= kModelCount) {
        return {};
    }
    return s_modelViews[index];
}

MimeResourceView GetMimeResourceView(MimeKind kind)
{
    const std::size_t index = MimeIndex(kind);
    if (index >= kMimeCount) {
        return {};
    }
    return s_mimeViews[index];
}

TitleTodResourceView GetTitleTodResourceView(TitleTodKind kind)
{
    const std::size_t index = TitleTodIndex(kind);
    if (index >= kTitleTodCount) {
        return {};
    }
    return s_titleTodViews[index];
}

RuntimeChannel2MimeBinding801C6410 GetRuntimeChannel2MimeBinding801C6410()
{
    return s_runtimeChannel2MimeBinding;
}

RuntimeChannel1MimeBinding801C6410 GetRuntimeChannel1MimeBinding801C6410()
{
    return s_runtimeChannel1MimeBinding;
}

RuntimeChannel3MimeBinding801C6410 GetRuntimeChannel3MimeBinding801C6410()
{
    return s_runtimeChannel3MimeBinding;
}

RuntimePrimaryPaMimeBinding80014164
GetRuntimePrimaryPaMimeBinding80014164()
{
    return s_runtimePrimaryPaMimeBinding;
}

RuntimeTitleTodBinding801C6410 GetRuntimeTitleTodBinding801C6410()
{
    return s_runtimeTitleTodBinding;
}

RuntimeTitleCameraBinding801C6410 GetRuntimeTitleCameraBinding801C6410()
{
    return s_runtimeTitleCameraBinding;
}

bool CanCommitRuntimeChannel2MimeBinding801C6410(
    const PrSS0TitleHudEventsDirect::TitleEventEffect801C5190& effect)
{
    RuntimeChannel2MimeBinding801C6410 next{};
    return BuildRuntimeChannel2MimeBinding801C6410(effect, next);
}

PreparedRuntimeChannel2MimeBinding801C6410
PrepareRuntimeChannel2MimeBinding801C6410(
    const PrSS0TitleHudEventsDirect::TitleEventEffect801C5190& effect)
{
    PreparedRuntimeChannel2MimeBinding801C6410 prepared{};
    prepared.accepted =
        BuildRuntimePrimaryPaMimeBinding801C6410(
            effect, prepared.primaryBinding) &&
        BuildRuntimeChannel2MimeBinding801C6410(effect, prepared.binding);
    return prepared;
}

void CommitPreparedRuntimeChannel2MimeBinding801C6410(
    const PreparedRuntimeChannel2MimeBinding801C6410& prepared)
{
    if (!prepared.accepted) {
        return;
    }
    RuntimePrimaryPaMimeBinding80014164 primary =
        prepared.primaryBinding;
    RuntimeChannel2MimeBinding801C6410 binding = prepared.binding;
    // The 801C5AB4 shortcut immediately performs the same PA_LEFT pair write
    // through the selector's first 801C5854 tick.  Preserve the setup
    // provenance across that equivalent overwrite so the following 801C5EF0
    // frame keeps the shortcut cursor semantics; natural selector writes have
    // no prior shortcut provenance and remain unchanged.
    if (s_runtimeChannel2MimeBinding.known &&
        s_runtimeChannel2MimeBinding.earlyInputShortcutSetup801C64BC &&
        s_runtimeChannel2MimeBinding.resourcePairIndex ==
            kShortcutChannel2PairIndex801C64BC &&
        s_runtimeChannel2MimeBinding.mimeKind == MimeKind::PaLeft &&
        s_runtimeChannel2MimeBinding.cursorKnown &&
        s_runtimeChannel2MimeBinding.cursor == 0u && binding.known &&
        binding.resourcePairIndex == kShortcutChannel2PairIndex801C64BC &&
        binding.mimeKind == MimeKind::PaLeft && binding.cursorKnown &&
        binding.cursor == 0u) {
        binding.earlyInputShortcutSetup801C64BC = true;
    }
    if (s_runtimePrimaryPaMimeBinding.known &&
        s_runtimePrimaryPaMimeBinding.earlyInputShortcutSetup801C64BC &&
        s_runtimePrimaryPaMimeBinding.resourcePairIndex ==
            kShortcutChannel2PairIndex801C64BC &&
        s_runtimePrimaryPaMimeBinding.mimeKind == MimeKind::PaLeft &&
        primary.known &&
        primary.resourcePairIndex == kShortcutChannel2PairIndex801C64BC &&
        primary.mimeKind == MimeKind::PaLeft) {
        primary.earlyInputShortcutSetup801C64BC = true;
    }
    // Original 801C6410 order: 800140E0 visible PA, then 80014050 group 2.
    PublishRuntimePrimaryPaMimeBinding801C6410(primary);
    PublishRuntimeChannel2MimeBinding801C6410(binding);
}

bool CommitRuntimeChannel2MimeBinding801C6410(
    const PrSS0TitleHudEventsDirect::TitleEventEffect801C5190& effect)
{
    RuntimePrimaryPaMimeBinding80014164 nextPrimary{};
    RuntimeChannel2MimeBinding801C6410 next{};
    if (!BuildRuntimePrimaryPaMimeBinding801C6410(effect, nextPrimary) ||
        !BuildRuntimeChannel2MimeBinding801C6410(effect, next)) {
        return false;
    }
    PublishRuntimePrimaryPaMimeBinding801C6410(nextPrimary);
    PublishRuntimeChannel2MimeBinding801C6410(next);
    return true;
}

bool CommitRuntimeTitleEventResources801C6410(
    const PrSS0TitleHudEventsDirect::TitleEventEffect801C5190& effect)
{
    if (!IsAppliedTitleEvent801C5190(effect)) {
        return false;
    }

    RuntimeChannel1MimeBinding801C6410 nextChannel1 =
        s_runtimeChannel1MimeBinding;
    RuntimePrimaryPaMimeBinding80014164 nextPrimary =
        s_runtimePrimaryPaMimeBinding;
    RuntimeChannel2MimeBinding801C6410 nextMime =
        s_runtimeChannel2MimeBinding;
    RuntimeChannel3MimeBinding801C6410 nextChannel3 =
        s_runtimeChannel3MimeBinding;
    RuntimeTitleTodBinding801C6410 nextTod = s_runtimeTitleTodBinding;
    PrSS0TitleTransformDirect::TitleTodCursorState8001B000 nextTodCursor =
        s_runtimeTitleTodCursor;
    RuntimeTitleCameraBinding801C6410 nextCamera =
        s_runtimeTitleCameraBinding;
    PrSS0TitleTransformDirect::TitleCameraCursorState801CB59C
        nextCameraCursor = s_runtimeTitleCameraCursor;
    if (effect.channel1PairWrite &&
        !BuildRuntimeChannel1MimeBinding801C6410(effect, nextChannel1)) {
        return false;
    }
    if (effect.channel2PairWrite &&
        (!BuildRuntimePrimaryPaMimeBinding801C6410(
             effect, nextPrimary) ||
         !BuildRuntimeChannel2MimeBinding801C6410(effect, nextMime))) {
        return false;
    }
    if (effect.channel3PairWrite &&
        !BuildRuntimeChannel3MimeBinding801C6410(effect, nextChannel3)) {
        return false;
    }
    if (effect.todResourceF4Write &&
        !BuildRuntimeTitleTodBinding801C6410(
            effect, nextTod, nextTodCursor)) {
        return false;
    }
    if (effect.objectResource104Write &&
        !BuildRuntimeTitleCameraBinding801C6410(
            effect, nextCamera, nextCameraCursor)) {
        return false;
    }

    if (effect.channel1PairWrite) {
        PublishRuntimeChannel1MimeBinding801C6410(nextChannel1);
    }
    if (effect.channel2PairWrite) {
        PublishRuntimePrimaryPaMimeBinding801C6410(nextPrimary);
        PublishRuntimeChannel2MimeBinding801C6410(nextMime);
    }
    if (effect.channel3PairWrite) {
        PublishRuntimeChannel3MimeBinding801C6410(nextChannel3);
    }
    if (effect.todResourceF4Write) {
        PublishRuntimeTitleTodBinding801C6410(nextTod, nextTodCursor);
    }
    if (effect.objectResource104Write) {
        PublishRuntimeTitleCameraBinding801C6410(
            nextCamera, nextCameraCursor);
    }
    return true;
}

RuntimeChannel2MimeAdvanceResult80013EA8
AdvanceRuntimeChannel2Mime80013EA8()
{
    RuntimeChannel2MimeAdvanceResult80013EA8 result{};
    if (!s_runtimeChannel2MimeBinding.known ||
        !s_runtimeChannel2MimeBinding.cursorKnown ||
        !s_resourceState.firstPaKageMode25BaseTriangleKnown) {
        return result;
    }

    const MimeKind mimeKind = s_runtimeChannel2MimeBinding.mimeKind;
    const VdfData* vdf = GetMimeVdf(mimeKind);
    const DatData* dat = GetMimeDat(mimeKind);
    if (mimeKind == MimeKind::Count || vdf == nullptr || dat == nullptr) {
        return result;
    }

    PrSS0TitleTransformDirect::MimeRuntimeInput80013EA8 runtime{};
    runtime.bindingKnown = true;
    runtime.vdf = vdf;
    runtime.dat = dat;
    runtime.objectIndex =
        s_firstPaKageMode25BaseTriangleCarrier.objectIndex;
    runtime.sampleFrameKnown = true;
    runtime.sampleFrame = static_cast<uint16_t>(
        s_runtimeChannel2MimeBinding.cursor & 0xFFFFu);
    runtime.loopFlagKnown = true;
    runtime.loopFlag = 0u;
    const auto deformation =
        PrSS0TitleTransformDirect::ApplyMode25Mime80013EA8(
            s_firstPaKageMode25BaseTriangleCarrier, runtime);
    if (!deformation.deformedTriangleReady ||
        !deformation.carrier.known) {
        return result;
    }

    RuntimeChannel2MimeBinding801C6410 nextBinding =
        s_runtimeChannel2MimeBinding;
    const uint32_t sampledCursor = nextBinding.cursor;
    ++nextBinding.cursor;

    s_firstPaKageMode25DeformedTriangleCarrier = deformation.carrier;
    s_runtimeChannel2MimeBinding = nextBinding;
    s_resourceState.firstPaKageRuntimeVertexDeformationKnown = true;
    ClearFirstPaKageMode25Rtpt3Carrier();
    result.accepted = true;
    result.rawCursor = sampledCursor;
    result.sampleFrame = runtime.sampleFrame;
    return result;
}

RuntimeChannel1MimeAdvanceResult80013EA8
AdvanceRuntimeChannel1Mime80013EA8()
{
    RuntimeChannel1MimeAdvanceResult80013EA8 result{};
    if (!s_runtimeChannel1MimeBinding.known ||
        !s_runtimeChannel1MimeBinding.cursorKnown ||
        s_runtimeChannel1MimeBinding.mimeKind != MimeKind::LogoNew ||
        !s_resourceState.firstLoMode25BaseTriangleKnown) {
        return result;
    }

    const VdfData* vdf = GetMimeVdf(MimeKind::LogoNew);
    const DatData* dat = GetMimeDat(MimeKind::LogoNew);
    if (vdf == nullptr || dat == nullptr) {
        return result;
    }

    PrSS0TitleTransformDirect::MimeRuntimeInput80013EA8 runtime{};
    runtime.bindingKnown = true;
    runtime.vdf = vdf;
    runtime.dat = dat;
    runtime.objectIndex = s_firstLoMode25BaseTriangleCarrier.objectIndex;
    runtime.sampleFrameKnown = true;
    const bool earlyInputShortcut =
        s_runtimeChannel1MimeBinding.earlyInputShortcutSetup801C6530;
    // 801C5EF0 advances the LogoNew channel on every accepted title frame.
    // The natural (non-shortcut) path used to force sample 0 and reset the
    // cursor to 1, which left LO frozen on its first pose and made the logo
    // character/face animation look like a broken shell.  The shortcut path
    // already carried the source cursor, so use that same monotonic source
    // for both paths.  ApplyMode25 clamps past-end samples to the final frame.
    runtime.sampleFrame = static_cast<uint16_t>(
        s_runtimeChannel1MimeBinding.cursor & 0xFFFFu);
    runtime.loopFlagKnown = true;
    runtime.loopFlag = 0u;
    const auto deformation =
        PrSS0TitleTransformDirect::ApplyMode25Mime80013EA8(
            s_firstLoMode25BaseTriangleCarrier, runtime);
    if (!deformation.deformedTriangleReady ||
        !deformation.carrier.known ||
        deformation.carrier.path !=
            PrSS0TitleTransformDirect::ModelPath::Lo) {
        return result;
    }

    RuntimeChannel1MimeBinding801C6410 nextBinding =
        s_runtimeChannel1MimeBinding;
    ++nextBinding.cursor;
    s_firstLoMode25DeformedTriangleCarrier = deformation.carrier;
    s_runtimeChannel1MimeBinding = nextBinding;
    s_resourceState.firstLoRuntimeVertexDeformationKnown = true;
    ClearLoMode25PrimitiveGroup8004274C();

    result.accepted = true;
    result.cursorAfter = nextBinding.cursor;
    result.sampleFrame = runtime.sampleFrame;
    return result;
}

RuntimeChannel3MimeAdvanceResult80013EA8
AdvanceRuntimeChannel3Mime80013EA8()
{
    RuntimeChannel3MimeAdvanceResult80013EA8 result{};
    if (!s_runtimeChannel3MimeBinding.known ||
        !s_runtimeChannel3MimeBinding.cursorKnown ||
        s_runtimeChannel3MimeBinding.mimeKind != MimeKind::Hiphop ||
        !s_resourceState.firstHpMode25BaseTriangleKnown) {
        return result;
    }

    const VdfData* vdf = GetMimeVdf(MimeKind::Hiphop);
    const DatData* dat = GetMimeDat(MimeKind::Hiphop);
    if (vdf == nullptr || dat == nullptr) {
        return result;
    }

    PrSS0TitleTransformDirect::MimeRuntimeInput80013EA8 runtime{};
    runtime.bindingKnown = true;
    runtime.vdf = vdf;
    runtime.dat = dat;
    runtime.objectIndex = s_firstHpMode25BaseTriangleCarrier.objectIndex;
    runtime.sampleFrameKnown = true;
    runtime.sampleFrame = static_cast<uint16_t>(
        s_runtimeChannel3MimeBinding.cursor & 0xFFFFu);
    runtime.loopFlagKnown = true;
    runtime.loopFlag = 0u;
    const auto deformation =
        PrSS0TitleTransformDirect::ApplyMode25Mime80013EA8(
            s_firstHpMode25BaseTriangleCarrier, runtime);
    if (!deformation.deformedTriangleReady ||
        !deformation.carrier.known ||
        deformation.carrier.path !=
            PrSS0TitleTransformDirect::ModelPath::Hp) {
        return result;
    }

    RuntimeChannel3MimeBinding801C6410 nextBinding =
        s_runtimeChannel3MimeBinding;
    const uint32_t sampledCursor = nextBinding.cursor;
    ++nextBinding.cursor;
    s_firstHpMode25DeformedTriangleCarrier = deformation.carrier;
    s_runtimeChannel3MimeBinding = nextBinding;
    s_resourceState.firstHpRuntimeVertexDeformationKnown = true;
    ClearHpMode25PrimitiveGroup8004274C();

    result.accepted = true;
    result.rawCursor = sampledCursor;
    result.sampleFrame = runtime.sampleFrame;
    return result;
}

RuntimePrimaryPaMimeAdvanceResult800141D8
AdvanceRuntimePrimaryPaMime800141D8()
{
    RuntimePrimaryPaMimeAdvanceResult800141D8 result{};
    if (!s_runtimePrimaryPaMimeBinding.known ||
        !s_runtimePrimaryPaMimeBinding.cursorKnown ||
        !s_runtimePrimaryPaMimeBinding.endKnown ||
        !s_runtimePrimaryPaMimeBinding.loopFlagKnown ||
        s_runtimePrimaryPaMimeBinding.mimeKind == MimeKind::Count ||
        !s_resourceState.firstPaMode25BaseTriangleKnown) {
        return result;
    }

    if (s_runtimePrimaryPaMimeBinding.cursor >=
        s_runtimePrimaryPaMimeBinding.end) {
        // 800141D8 carries the currently bound primary MIME loop flag.  The
        // lower 80013EA8 deformation path applies the same loop by sampling
        // modulo maxFrames; event-selected loopFlag=0 actions instead retain
        // their final applied vertices after the configured 0..998 range.
        if (s_runtimePrimaryPaMimeBinding.loopFlag != 1u ||
            s_runtimePrimaryPaMimeBinding.end <=
                kPrimaryPaMimeCursorBegin80014164) {
            result.sourceExhausted = true;
            return result;
        }
        s_runtimePrimaryPaMimeBinding.cursor =
            kPrimaryPaMimeCursorBegin80014164;
    }

    const MimeKind mimeKind = s_runtimePrimaryPaMimeBinding.mimeKind;
    const VdfData* vdf = GetMimeVdf(mimeKind);
    const DatData* dat = GetMimeDat(mimeKind);
    if (vdf == nullptr || dat == nullptr) {
        return result;
    }

    PrSS0TitleTransformDirect::MimeRuntimeInput80013EA8 runtime{};
    runtime.bindingKnown = true;
    runtime.vdf = vdf;
    runtime.dat = dat;
    runtime.objectIndex = s_firstPaMode25BaseTriangleCarrier.objectIndex;
    runtime.sampleFrameKnown = true;
    runtime.sampleFrame = static_cast<uint16_t>(
        s_runtimePrimaryPaMimeBinding.cursor & 0xFFFFu);
    runtime.loopFlagKnown = true;
    runtime.loopFlag = s_runtimePrimaryPaMimeBinding.loopFlag;
    const auto deformation =
        PrSS0TitleTransformDirect::ApplyMode25Mime80013EA8(
            s_firstPaMode25BaseTriangleCarrier, runtime);
    if (!deformation.deformedTriangleReady ||
        !deformation.carrier.known ||
        deformation.carrier.path != PrSS0TitleTransformDirect::ModelPath::Pa) {
        return result;
    }

    const uint32_t sampledCursor = s_runtimePrimaryPaMimeBinding.cursor;
    ++s_runtimePrimaryPaMimeBinding.cursor;
    s_firstPaMode25DeformedTriangleCarrier = deformation.carrier;
    s_resourceState.firstPaRuntimeVertexDeformationKnown = true;
    ClearPaMode25PrimitiveGroup8004274C();

    result.accepted = true;
    result.sourceExhausted = s_runtimePrimaryPaMimeBinding.cursor >=
                             s_runtimePrimaryPaMimeBinding.end;
    result.rawCursor = sampledCursor;
    result.sampleFrame = runtime.sampleFrame;
    return result;
}

RuntimeTitleTodAdvanceResult8001B000 AdvanceRuntimeTitleTod8001B000()
{
    RuntimeTitleTodAdvanceResult8001B000 result{};
    if (!s_runtimeTitleTodBinding.known ||
        !s_runtimeTitleTodBinding.cursorKnown) {
        return result;
    }

    const TodData* tod = GetTitleTod(s_runtimeTitleTodBinding.todKind);
    const auto advance =
        PrSS0TitleTransformDirect::AdvanceTitleTodDrawCoord8001B000(
            s_runtimeTitleTodCursor, tod);
    if (!advance.accepted) {
        return result;
    }

    s_runtimeTitleTodCursor = advance.nextState;
    s_runtimeTitleTodBinding.sampleSeq = advance.nextState.sampleSeq;
    s_runtimeTitleTodBinding.blockIndex = advance.nextState.blockIndex;
    s_runtimeTitleTodBinding.drawCoordWorldKnown800417A4 =
        advance.drawCoordWorldKnown800417A4;
    s_resourceState.runtimeTitleTodCursorKnown = true;
    s_resourceState.titleDrawCoordWorldKnown800417A4 =
        advance.drawCoordWorldKnown800417A4;
    if (advance.drawCoordWorldKnown800417A4) {
        s_runtimeTitleDrawCoordWorld = advance.drawCoordWorld800417A4;
    }
    s_firstPaRuntimeMatrix80041A68 = {};
    s_firstPaKageRuntimeMatrix800406D8 = {};
    s_resourceState.firstPaRuntimeMatrixKnown80041A68 = false;
    s_resourceState.firstPaKageRuntimeMatrixKnown = false;
    ClearPaMode25PrimitiveGroup8004274C();
    ClearFirstPaKageMode25Rtpt3Carrier();

    result.accepted = true;
    result.blockAdvanced = advance.blockAdvanced;
    result.sourceExhausted = advance.sourceExhausted;
    result.sampleSeq = advance.nextState.sampleSeq;
    result.blockIndex = advance.nextState.blockIndex;
    result.drawCoordWorldKnown800417A4 =
        advance.drawCoordWorldKnown800417A4;
    return result;
}

RuntimeTitleCameraAdvanceResult80041D3C
AdvanceRuntimeTitleCamera80041D3C()
{
    RuntimeTitleCameraAdvanceResult80041D3C result{};
    if (!s_runtimeTitleCameraBinding.known ||
        !s_runtimeTitleCameraBinding.cursorKnown ||
        !s_runtimeTitleCameraCursor.cursorKnown ||
        !s_titleCameraTable.known ||
        !s_resourceState.titleCameraTableKnown801C6FB4 ||
        !s_resourceState.titlePanelCoordKnown801C5D8C) {
        return result;
    }

    const auto advance =
        PrSS0TitleTransformDirect::AdvanceTitleCamera801C6410(
            s_runtimeTitleCameraCursor, &s_titleCameraTable);
    result.sourceExhausted = advance.sourceExhausted;
    result.nextCursor = advance.nextState.cursor;
    if (!advance.accepted || !advance.cameraMatrixKnown80092880) {
        if (advance.sourceExhausted &&
            s_resourceState.titleCameraMatrixKnown80092880 &&
            s_resourceState.titlePanelWithCameraKnown80041A68) {
            result.accepted = true;
            result.terminalStateHeld = true;
            result.sampledIndex = advance.nextState.cursor > 0u
                ? advance.nextState.cursor - 1u
                : 0u;
            result.cameraMatrixKnown80092880 = true;
        }
        return result;
    }

    PrSS0TitleTransformDirect::TitlePanelWithCameraInput80041A68
        panelInput{};
    panelInput.panelCoordKnown801CD7BC = true;
    panelInput.panelCoord801CD7BC = s_titlePanelCoord801CD7BC;
    panelInput.cameraMatrixKnown80092880 = true;
    panelInput.cameraMatrix80092880 = advance.cameraMatrix80092880;
    const auto panel =
        PrSS0TitleTransformDirect::BuildTitlePanelWithCamera80041A68(
            panelInput);
    if (!panel.panelWithCameraKnown80041A68) {
        return result;
    }

    s_runtimeTitleCameraCursor = advance.nextState;
    s_runtimeTitleCameraBinding.cursor = advance.nextState.cursor;
    s_runtimeTitleCameraMatrix80092880 = advance.cameraMatrix80092880;
    s_runtimeTitlePanelWithCamera80041A68 =
        panel.panelWithCamera80041A68;
    s_firstPaRuntimeMatrix80041A68 = {};
    s_firstPaKageRuntimeMatrix800406D8 = {};
    s_firstLoRuntimeMatrix80041A68 = {};
    s_firstHpRuntimeMatrix80041A68 = {};
    s_resourceState.runtimeTitleCameraCursorKnown = true;
    s_resourceState.titleCameraMatrixKnown80092880 = true;
    s_resourceState.titlePanelWithCameraKnown80041A68 = true;
    s_resourceState.firstPaRuntimeMatrixKnown80041A68 = false;
    s_resourceState.firstPaKageRuntimeMatrixKnown = false;
    s_resourceState.firstLoRuntimeMatrixKnown80041A68 = false;
    s_resourceState.firstHpRuntimeMatrixKnown80041A68 = false;
    ClearPaMode25PrimitiveGroup8004274C();
    ClearFirstPaKageMode25Rtpt3Carrier();
    ClearLoMode25PrimitiveGroup8004274C();
    ClearHpMode25PrimitiveGroup8004274C();

    result.accepted = true;
    result.sampledIndex = advance.sampledIndex;
    result.nextCursor = advance.nextState.cursor;
    result.cameraMatrixKnown80092880 = true;
    return result;
}

bool ComposeRuntimeTitleMatrix801C5E60()
{
    PrSS0TitleTransformDirect::TitleRuntimeMatrixInput801C5E60 input{};
    input.drawCoordWorldKnown800417A4 =
        s_resourceState.titleDrawCoordWorldKnown800417A4;
    if (input.drawCoordWorldKnown800417A4) {
        input.drawCoordWorld800417A4 = s_runtimeTitleDrawCoordWorld;
    }
    input.panelWithCameraKnown80041A68 =
        s_resourceState.titlePanelWithCameraKnown80041A68;
    if (input.panelWithCameraKnown80041A68) {
        input.panelWithCamera80041A68 =
            s_runtimeTitlePanelWithCamera80041A68;
    }

    const auto matrix =
        PrSS0TitleTransformDirect::BuildTitleRuntimeMatrix801C5E60(input);
    if (!matrix.runtimeMatrixKnown) {
        s_firstPaKageRuntimeMatrix800406D8 = {};
        s_resourceState.firstPaKageRuntimeMatrixKnown = false;
        ClearFirstPaKageMode25Rtpt3Carrier();
        return false;
    }

    s_firstPaKageRuntimeMatrix800406D8 = matrix.runtimeMatrix800406D8;
    s_resourceState.firstPaKageRuntimeMatrixKnown = true;
    ClearFirstPaKageMode25Rtpt3Carrier();
    return true;
}

bool ComposeRuntimePaMatrix8001B084()
{
    PrSS0TitleTransformDirect::TitleCoordWithCameraInput80041A68 input{};
    input.drawCoordWorldKnown800417A4 =
        s_resourceState.titleDrawCoordWorldKnown800417A4;
    if (input.drawCoordWorldKnown800417A4) {
        input.drawCoordWorld800417A4 = s_runtimeTitleDrawCoordWorld;
    }
    input.cameraMatrixKnown80092880 =
        s_resourceState.titleCameraMatrixKnown80092880;
    if (input.cameraMatrixKnown80092880) {
        input.cameraMatrix80092880 = s_runtimeTitleCameraMatrix80092880;
    }

    const auto matrix =
        PrSS0TitleTransformDirect::BuildTitleCoordWithCamera80041A68(input);
    if (!matrix.coordWithCameraKnown80041A68) {
        s_firstPaRuntimeMatrix80041A68 = {};
        s_resourceState.firstPaRuntimeMatrixKnown80041A68 = false;
        ClearPaMode25PrimitiveGroup8004274C();
        return false;
    }

    s_firstPaRuntimeMatrix80041A68 = matrix.coordWithCamera80041A68;
    s_resourceState.firstPaRuntimeMatrixKnown80041A68 = true;
    ClearPaMode25PrimitiveGroup8004274C();
    return true;
}

bool ComposeRuntimeLoMatrix8001B084()
{
    PrSS0TitleTransformDirect::TitleCoordWithCameraInput80041A68 input{};
    input.drawCoordWorldKnown800417A4 =
        s_resourceState.loCoordKnown8004049C;
    if (input.drawCoordWorldKnown800417A4) {
        input.drawCoordWorld800417A4 = s_loCoord801CB5A0;
    }
    input.cameraMatrixKnown80092880 =
        s_resourceState.titleCameraMatrixKnown80092880;
    if (input.cameraMatrixKnown80092880) {
        input.cameraMatrix80092880 = s_runtimeTitleCameraMatrix80092880;
    }

    const auto matrix =
        PrSS0TitleTransformDirect::BuildTitleCoordWithCamera80041A68(input);
    if (!matrix.coordWithCameraKnown80041A68) {
        s_firstLoRuntimeMatrix80041A68 = {};
        s_resourceState.firstLoRuntimeMatrixKnown80041A68 = false;
        ClearLoMode25PrimitiveGroup8004274C();
        return false;
    }

    s_firstLoRuntimeMatrix80041A68 = matrix.coordWithCamera80041A68;
    s_resourceState.firstLoRuntimeMatrixKnown80041A68 = true;
    ClearLoMode25PrimitiveGroup8004274C();
    return true;
}

bool ComposeRuntimeHpMatrix8001B084()
{
    PrSS0TitleTransformDirect::TitleCoordWithCameraInput80041A68 input{};
    input.drawCoordWorldKnown800417A4 =
        s_resourceState.hpCoordKnown8004049C;
    if (input.drawCoordWorldKnown800417A4) {
        input.drawCoordWorld800417A4 = s_hpCoord801CC6BC;
    }
    input.cameraMatrixKnown80092880 =
        s_resourceState.titleCameraMatrixKnown80092880;
    if (input.cameraMatrixKnown80092880) {
        input.cameraMatrix80092880 = s_runtimeTitleCameraMatrix80092880;
    }

    const auto matrix =
        PrSS0TitleTransformDirect::BuildTitleCoordWithCamera80041A68(input);
    if (!matrix.coordWithCameraKnown80041A68) {
        s_firstHpRuntimeMatrix80041A68 = {};
        s_resourceState.firstHpRuntimeMatrixKnown80041A68 = false;
        ClearHpMode25PrimitiveGroup8004274C();
        return false;
    }

    s_firstHpRuntimeMatrix80041A68 = matrix.coordWithCamera80041A68;
    s_resourceState.firstHpRuntimeMatrixKnown80041A68 = true;
    ClearHpMode25PrimitiveGroup8004274C();
    return true;
}

bool ExecuteFirstPaKageMode25Rtpt3At800428B0()
{
    const TmdObject* paKageObject =
        GetModelObject(ModelKind::PaKage, 0u);
    if (!s_resourceState.firstPaKageDrawDescKnown8001AF1C ||
        !s_resourceState.firstPaKagePrimitiveGroupKnown8004274C ||
        !PrSS0TitleDrawDescDirect::IsFirstPaKagePrimitiveReady800428B0(
            s_firstPaKageDrawDesc,
            paKageObject,
            s_firstPaKageMode25DeformedTriangleCarrier.primitiveIndex)) {
        ClearFirstPaKageMode25Rtpt3Carrier();
        return false;
    }

    const auto joined =
        PrSS0TitleTransformDirect::BuildMode25Rtpt3Input801C5E60(
            s_firstPaKageMode25DeformedTriangleCarrier,
            s_resourceState.firstPaKageRuntimeMatrixKnown
                ? &s_firstPaKageRuntimeMatrix800406D8
                : nullptr,
            s_resourceState.firstPaKageRuntimeMatrixKnown);
    if (!joined.ready || !joined.input.known) {
        ClearFirstPaKageMode25Rtpt3Carrier();
        return false;
    }

    const auto executed =
        PrSS0TitleTransformDirect::ExecuteMode25Rtpt3At800428B0(
            joined.input);
    if (!executed.ready || !executed.input.known ||
        !executed.output.known || !executed.geometryReady ||
        !executed.geometry.projectedSxyKnown ||
        !executed.geometry.visibilityKnown || !executed.geometry.otzKnown) {
        ClearFirstPaKageMode25Rtpt3Carrier();
        return false;
    }

    s_firstPaKageMode25Rtpt3Input801C5E60 = executed.input;
    s_firstPaKageMode25Rtpt3Output800428B0 = executed.output;
    s_firstPaKageMode25Geometry800428B0 = executed.geometry;
    s_resourceState.firstPaKageMode25Rtpt3InputKnown = true;
    s_resourceState.firstPaKageMode25Rtpt3OutputKnown = true;
    s_resourceState.firstPaKageMode25GeometryKnown = true;
    return true;
}

bool BeginTitlePacketFrame801C6410()
{
    ClearPaMode25PrimitiveGroup8004274C();
    ClearFirstPaKageMode25PacketPlan800428B0();
    ClearLoMode25PrimitiveGroup8004274C();
    ClearHpMode25PrimitiveGroup8004274C();
    const auto frame = PrSS0TitlePacketWorkDirect::BeginFrame801C6410(
        s_titlePacketWork, s_titleGraphState);
    s_resourceState.titlePacketFrameKnown801C6410 = frame.prepared;
    s_resourceState.titleFrameFlagsApplied801C6410 = false;
    if (!frame.prepared) {
        ClearFirstPaKageMode25Rtpt3Carrier();
    }
    return frame.prepared;
}

bool ApplyTitleFrameFlags801C6410(uint32_t ctxFlags)
{
    if (!s_resourceState.resourceReady ||
        !s_resourceState.titlePacketFrameKnown801C6410) {
        return false;
    }
    const auto applied =
        PrSS0TitlePacketWorkDirect::ApplyTitleFrameFlags801C6410(
            s_titlePacketWork, ctxFlags);
    if (!applied.applied) {
        return false;
    }
    SyncTitleFrameGateResourceState801C6410();
    return true;
}

TitleEarlyInputShortcutResult801C6410
ApplyTitleEarlyInputShortcutSetupFrame801C6410()
{
    TitleEarlyInputShortcutResult801C6410 out{};
    if (!s_resourceState.resourceReady ||
        !s_resourceState.titlePacketWorkKnown801C609C ||
        !s_resourceState.titleGraphControlKnown) {
        return out;
    }
    if (s_resourceState.titleEarlyInputShortcutSetupKnown801C6410 ||
        s_resourceState.titleEarlyInputShortcutReadyKnown801C6410 ||
        s_titlePacketWork.titleShortcutSetupApplied801C6410 ||
        s_titlePacketWork.titleShortcutReadyApplied801C6410) {
        out.status =
            TitleEarlyInputShortcutStatus801C6410::RuntimeOrderMismatch;
        return out;
    }

    TitleEarlyInputShortcutSetupDraft801C6410 draft{};
    if (!BuildTitleEarlyInputShortcutSetupDraft801C6410(draft)) {
        out.status =
            TitleEarlyInputShortcutStatus801C6410::ResourceSourceMissing;
        return out;
    }

    const auto packet =
        PrSS0TitlePacketWorkDirect::
            ApplyEarlyInputShortcutSetupFrame801C6410(
                s_titlePacketWork, s_titleGraphState);
    if (!packet.committed) {
        out.status =
            TitleEarlyInputShortcutStatus801C6410::PacketTransactionRejected;
        return out;
    }

    PublishRuntimePrimaryPaMimeBinding801C6410(draft.primary);
    PublishRuntimeChannel2MimeBinding801C6410(draft.channel2);
    PublishRuntimeChannel1MimeBinding801C6410(draft.channel1);
    PublishRuntimeChannel3MimeBinding801C6410(draft.channel3);
    PublishRuntimeTitleCameraBinding801C6410(
        draft.camera, draft.cameraCursor);
    s_resourceState.titleEarlyInputShortcutSetupKnown801C6410 = true;
    s_resourceState.titleEarlyInputShortcutReadyKnown801C6410 = false;
    s_resourceState.titleEarlyInputShortcutLoopCount801CC676 =
        draft.channel1LoopCount801CC676;
    SyncTitleFrameGateResourceState801C6410();

    out.status = TitleEarlyInputShortcutStatus801C6410::SetupCommitted;
    out.committed = true;
    out.channel1LoopCount801CC676 = draft.channel1LoopCount801CC676;
    out.channel1CursorAfter = s_runtimeChannel1MimeBinding.cursor;
    out.channel3CursorAfter = s_runtimeChannel3MimeBinding.cursor;
    out.cameraCursorAfter801CB59C = s_runtimeTitleCameraBinding.cursor;
    return out;
}

TitleEarlyInputShortcutResult801C6410
ApplyTitleEarlyInputShortcutReadyFrame801C6410()
{
    TitleEarlyInputShortcutResult801C6410 out{};
    const uint32_t loopCount =
        s_resourceState.titleEarlyInputShortcutLoopCount801CC676;
    if (!s_resourceState.resourceReady ||
        !s_resourceState.titlePacketWorkKnown801C609C ||
        !s_resourceState.titleGraphControlKnown) {
        return out;
    }
    if (!s_resourceState.titleEarlyInputShortcutSetupKnown801C6410 ||
        s_resourceState.titleEarlyInputShortcutReadyKnown801C6410 ||
        loopCount == 0u ||
        !s_runtimePrimaryPaMimeBinding.known ||
        !s_runtimePrimaryPaMimeBinding.earlyInputShortcutSetup801C64BC ||
        s_runtimePrimaryPaMimeBinding.resourcePairIndex !=
            kShortcutChannel2PairIndex801C64BC ||
        s_runtimePrimaryPaMimeBinding.mimeKind != MimeKind::PaLeft ||
        !s_runtimePrimaryPaMimeBinding.cursorKnown ||
        s_runtimePrimaryPaMimeBinding.cursor !=
            kPrimaryPaMimeCursorBegin80014164 ||
        !s_runtimePrimaryPaMimeBinding.endKnown ||
        s_runtimePrimaryPaMimeBinding.end !=
            kPrimaryPaMimeCursorEnd80014164 ||
        !s_runtimePrimaryPaMimeBinding.loopFlagKnown ||
        s_runtimePrimaryPaMimeBinding.loopFlag != 0u ||
        !s_runtimeChannel2MimeBinding.known ||
        !s_runtimeChannel2MimeBinding.earlyInputShortcutSetup801C64BC ||
        s_runtimeChannel2MimeBinding.resourcePairIndex !=
            kShortcutChannel2PairIndex801C64BC ||
        s_runtimeChannel2MimeBinding.mimeKind != MimeKind::PaLeft ||
        !s_runtimeChannel1MimeBinding.known ||
        !s_runtimeChannel1MimeBinding.earlyInputShortcutSetup801C6530 ||
        s_runtimeChannel1MimeBinding.resourcePairIndex !=
            kShortcutChannel1PairIndex801C6530 ||
        s_runtimeChannel1MimeBinding.mimeKind != MimeKind::LogoNew ||
        !s_runtimeChannel1MimeBinding.cursorKnown ||
        s_runtimeChannel1MimeBinding.cursor != loopCount ||
        !s_runtimeChannel3MimeBinding.known ||
        !s_runtimeChannel3MimeBinding.earlyInputShortcutSetup801C6578 ||
        s_runtimeChannel3MimeBinding.resourcePairIndex !=
            kShortcutChannel3PairIndex801C6578 ||
        s_runtimeChannel3MimeBinding.mimeKind != MimeKind::Hiphop ||
        !s_runtimeChannel3MimeBinding.cursorKnown ||
        s_runtimeChannel3MimeBinding.cursor != loopCount ||
        !s_runtimeTitleCameraBinding.known ||
        !s_runtimeTitleCameraBinding.cursorKnown ||
        s_runtimeTitleCameraBinding.cursor !=
            kShortcutCameraCursor801CB59C ||
        !s_runtimeTitleCameraCursor.cursorKnown ||
        s_runtimeTitleCameraCursor.cursor !=
            kShortcutCameraCursor801CB59C) {
        out.status =
            TitleEarlyInputShortcutStatus801C6410::RuntimeOrderMismatch;
        return out;
    }

    RuntimeTitleTodBinding801C6410 readyTod{};
    PrSS0TitleTransformDirect::TitleTodCursorState8001B000 readyTodCursor{};
    if (!BuildTitleEarlyInputShortcutReadyTod801C5AB4(
            readyTod, readyTodCursor)) {
        out.status =
            TitleEarlyInputShortcutStatus801C6410::ResourceSourceMissing;
        return out;
    }

    const auto packet =
        PrSS0TitlePacketWorkDirect::
            ApplyEarlyInputShortcutReadyFrame801C6410(
                s_titlePacketWork, s_titleGraphState);
    if (!packet.committed) {
        out.status =
            TitleEarlyInputShortcutStatus801C6410::PacketTransactionRejected;
        return out;
    }

    PublishRuntimeTitleTodBinding801C6410(readyTod, readyTodCursor);
    s_resourceState.titleEarlyInputShortcutReadyKnown801C6410 = true;
    SyncTitleFrameGateResourceState801C6410();

    out.status = TitleEarlyInputShortcutStatus801C6410::ReadyCommitted;
    out.committed = true;
    out.channel1LoopCount801CC676 = static_cast<uint16_t>(loopCount);
    out.channel1CursorAfter = s_runtimeChannel1MimeBinding.cursor;
    out.channel3CursorAfter = s_runtimeChannel3MimeBinding.cursor;
    out.cameraCursorAfter801CB59C = s_runtimeTitleCameraBinding.cursor;
    return out;
}

bool IsTitleEarlyInputShortcutReady801C6410()
{
    const uint32_t loopCount =
        s_resourceState.titleEarlyInputShortcutLoopCount801CC676;
    return s_resourceState.titleEarlyInputShortcutSetupKnown801C6410 &&
           s_resourceState.titleEarlyInputShortcutReadyKnown801C6410 &&
           loopCount != 0u &&
           s_titlePacketWork.titleShortcutSetupApplied801C6410 &&
           s_titlePacketWork.titleShortcutReadyApplied801C6410 &&
           // frameSetupSource is the source of the most recent frame-flags
           // write and therefore becomes EventCtxFlags on later selector
           // frames.  The persistent setup/ready bits above carry the
           // shortcut provenance after the one-time ready commit.
           s_titlePacketWork.titleRenderActive801C9548 == 1u &&
           s_titlePacketWork.loHpGate801CB600 == 1u &&
           s_titlePacketWork.presentExtra801CFAEC == 0u &&
           s_runtimePrimaryPaMimeBinding.known &&
           s_runtimePrimaryPaMimeBinding.earlyInputShortcutSetup801C64BC &&
           s_runtimePrimaryPaMimeBinding.resourcePairIndex ==
               kShortcutChannel2PairIndex801C64BC &&
           s_runtimePrimaryPaMimeBinding.mimeKind == MimeKind::PaLeft &&
           s_runtimePrimaryPaMimeBinding.cursorKnown &&
           s_runtimePrimaryPaMimeBinding.endKnown &&
           s_runtimePrimaryPaMimeBinding.loopFlagKnown &&
           s_runtimePrimaryPaMimeBinding.loopFlag == 0u &&
           s_runtimeChannel2MimeBinding.known &&
           s_runtimeChannel2MimeBinding.earlyInputShortcutSetup801C64BC &&
           s_runtimeChannel2MimeBinding.resourcePairIndex ==
               kShortcutChannel2PairIndex801C64BC &&
           s_runtimeChannel2MimeBinding.mimeKind == MimeKind::PaLeft &&
           s_runtimeChannel1MimeBinding.known &&
           s_runtimeChannel1MimeBinding.earlyInputShortcutSetup801C6530 &&
           s_runtimeChannel1MimeBinding.cursorKnown &&
           // The exact loopCount/camera cursor values are required by the
           // one-time 801C6410 ready-frame commit above.  After that commit,
           // 801C5EF0 advances the same sources every selector frame; keep
           // this activation predicate valid for their monotonic cursors.
           s_runtimeChannel1MimeBinding.cursor >= loopCount &&
           s_runtimeChannel1MimeBinding.resourcePairIndex ==
               kShortcutChannel1PairIndex801C6530 &&
           s_runtimeChannel1MimeBinding.mimeKind == MimeKind::LogoNew &&
           s_runtimeChannel3MimeBinding.known &&
           s_runtimeChannel3MimeBinding.earlyInputShortcutSetup801C6578 &&
           s_runtimeChannel3MimeBinding.cursorKnown &&
           s_runtimeChannel3MimeBinding.cursor >= loopCount &&
           s_runtimeChannel3MimeBinding.resourcePairIndex ==
               kShortcutChannel3PairIndex801C6578 &&
           s_runtimeChannel3MimeBinding.mimeKind == MimeKind::Hiphop &&
           s_runtimeTitleCameraBinding.known &&
           s_runtimeTitleCameraBinding.cursorKnown &&
           s_runtimeTitleCameraBinding.cursor >=
               kShortcutCameraCursor801CB59C &&
           s_runtimeTitleTodBinding.known &&
           s_runtimeTitleTodBinding.earlyInputShortcutReady801C5AB4 &&
           s_runtimeTitleTodBinding.todKind == TitleTodKind::PaLoc2;
}

TitleMovie0TState0GraphFlipResult801C4B8C
ApplyTitleMovie0TState0GraphFlip801C4B8C()
{
    TitleMovie0TState0GraphFlipResult801C4B8C out{};
    if (!s_resourceState.resourceReady ||
        !s_resourceState.titleGraphControlKnown ||
        !s_titleGraphState.mainPageWorkLists80087288Initialized ||
        s_titleGraphState.word_80096590 > 1u) {
        return out;
    }

    auto nextGraph = s_titleGraphState;
    const uint16_t expectedNextSlot =
        nextGraph.word_80096590 == 0u ? 1u : 0u;
    const auto flip =
        PrPsxGraphOwnerDirect::PsxCall80040370_FlipGraph(nextGraph);
    if (flip.previousSlot != s_titleGraphState.word_80096590 ||
        flip.nextSlot != expectedNextSlot ||
        flip.previousFrameCounter8009658C !=
            s_titleGraphState.dword_8009658C ||
        flip.nextFrameCounter8009658C != nextGraph.dword_8009658C ||
        !flip.sub800452ECCalled || !flip.sub80044AA0Called ||
        !flip.sub800402E0Called || !flip.sub800401ACCalled ||
        nextGraph.word_80096590 != expectedNextSlot) {
        return out;
    }

    s_titleGraphState = nextGraph;
    out.committed = true;
    out.drawSlotBefore = flip.previousSlot;
    out.drawSlotAfter = flip.nextSlot;
    out.frameCounterBefore8009658C =
        flip.previousFrameCounter8009658C;
    out.frameCounterAfter8009658C = flip.nextFrameCounter8009658C;
    return out;
}

TitleMovie0TState0GraphFlipResult801C4B8C
ApplyTitleMovie0TEarlyInputGraphFlip801C4B7C()
{
    return ApplyTitleMovie0TState0GraphFlip801C4B8C();
}

TitleMovie0TState0GraphFlipResult801C4B8C
ApplyOpeningMovie0GraphFlip8001ED74()
{
    // Current SCUS 8001ED74 is only a wrapper around 80040370. Reuse the
    // already direct graph transaction; no host-present or old-S0 state is
    // imported here.
    return ApplyTitleMovie0TState0GraphFlip801C4B8C();
}

OpeningMovie0WorkListFlushResult8001ED3C
ApplyOpeningMovie0WorkListFlush8001ED3C(uint16_t gp368WorkSlot,
                                       uint32_t priorCallCount80040CA4)
{
    OpeningMovie0WorkListFlushResult8001ED3C out{};
    if (!s_resourceState.resourceReady ||
        !s_resourceState.titleGraphControlKnown ||
        !s_titleGraphState.mainPageWorkLists80087288Initialized ||
        s_titleGraphState.word_80096590 > 1u ||
        gp368WorkSlot > 1u) {
        return out;
    }

    // Current SCUS 8001ED3C selects 80087288 + gp[0x368] * 20 and calls
    // 80040CA4. gp+0x368 is the separate 8006EDA8 cached work slot, not
    // 80096590's graph-flip selector.
    const uint16_t slot = gp368WorkSlot;
    const auto& page = s_titleGraphState.mainPageWorkLists80087288[slot];
    const auto submit =
        PrPsxEventFrameDirect::PsxCall80040CA4_SubmitWorkListDetailed(
            page.workAddr,
            slot,
            page.work,
            priorCallCount80040CA4);
    out.cachedWorkSlotKnown8006EDA8 = true;
    out.cachedWorkSlot8006EDA8 = slot;
    out.mainPageWorkAddress80087288 = page.workAddr;
    out.mainPageOtHeadAddress80088288 = page.otHeadAddr;
    out.submitCalled80040CA4 = submit.drawOtag800450A0Called;
    out.softwareStateCommitted = submit.softwareStateCommitted;
    out.callCount80040CA4 = submit.callCount;
    out.currentScusSemanticAuthority = true;
    out.currentComod0CallerAuthority = true;
    out.hostMmioWritten = submit.hostMmioWritten;
    out.hostGpuSubmitted = submit.hostGpuSubmitted;
    out.exactPsxHalParity = submit.exactPsxHalParity;
    out.committed =
        submit.attempted &&
        submit.sourceKnown &&
        submit.workListOffset10Read &&
        submit.drawOtag800450A0Called &&
        submit.softwareStateCommitted &&
        !submit.hostMmioWritten &&
        !submit.hostGpuSubmitted &&
        !submit.exactPsxHalParity &&
        submit.callCount == priorCallCount80040CA4 + 1u;
    return out;
}

TitlePresentModelResult801C689C ApplyTitlePresentModel801C689C()
{
    TitlePresentModelResult801C689C out{};
    if (!s_resourceState.resourceReady ||
        !s_resourceState.titlePacketFrameKnown801C6410) {
        return out;
    }
    if (s_resourceState.titlePresentModelApplyCount801C689C ==
        (std::numeric_limits<uint32_t>::max)()) {
        out.status = TitlePresentModelStatus801C689C::CounterOverflow;
        return out;
    }

    const auto applied =
        PrSS0TitlePacketWorkDirect::ApplyPresentFrameModel801C689C(
            s_titlePacketWork, s_titleGraphState);
    out.cachedDrawLane8006EDA8 = applied.cachedDrawLane8006EDA8;
    if (!applied.modelApplied) {
        out.status = TitlePresentModelStatus801C689C::FrameRejected;
        return out;
    }

    out.status = TitlePresentModelStatus801C689C::ModelApplied;
    out.modelApplied = true;
    out.nextDrawLane80096590 =
        static_cast<uint8_t>(applied.flip.nextSlot);
    out.titleWorkAddress801C9574 = applied.titleWorkAddress801C9574;
    out.mainPageWorkAddress80087288 =
        applied.mainPageWorkAddress80087288;
    out.whiteDrawEnv80040060Requested =
        applied.whiteDrawEnv80040060Requested;
    out.titleWork80040CA4SubmitRequested =
        applied.titleWork80040CA4SubmitRequested;
    out.mainPageWork8001E3B0SubmitRequested =
        applied.mainPageWork8001E3B0SubmitRequested;
    out.presentExtraSource801CFAEC =
        applied.presentExtraSource801CFAEC;
    out.extraFlipGraphStateAdvanced80040370 =
        applied.extraFlipGraphStateAdvanced80040370;
    out.fullHeightWhiteFill8001B1B0Requested =
        applied.fullHeightWhiteFill8001B1B0Requested;
    out.fullHeightWhiteFillX8001B1B0 =
        applied.fullHeightWhiteFillX8001B1B0;
    out.fullHeightWhiteFillY8001B1B0 =
        applied.fullHeightWhiteFillY8001B1B0;
    out.fullHeightWhiteFillWidth8001B1B0 =
        applied.fullHeightWhiteFillWidth8001B1B0;
    out.fullHeightWhiteFillHeight8001B1B0 =
        applied.fullHeightWhiteFillHeight8001B1B0;
    out.titleWorkSubmit80040CA4Executed =
        applied.titleWorkSubmit80040CA4Executed;
    out.mainPageWorkSubmit8001E3B0Executed =
        applied.mainPageWorkSubmit8001E3B0Executed;
    out.titleWorkSubmitCallCount80040CA4 =
        applied.titleWorkSubmitCallCount80040CA4;
    out.mainPageWorkSubmitCallCount8001E3B0 =
        applied.mainPageWorkSubmitCallCount8001E3B0;
    out.drawEnv80040060 = applied.drawEnv80040060;

    s_resourceState.titlePacketFrameKnown801C6410 = false;
    s_resourceState.titleFrameFlagsApplied801C6410 = false;
    s_resourceState.titlePresentModelApplied801C689C = true;
    ++s_resourceState.titlePresentModelApplyCount801C689C;
    s_resourceState.titlePresentModelLastCachedLane8006EDA8 =
        out.cachedDrawLane8006EDA8;
    s_resourceState.titlePresentModelNextDrawLane80096590 =
        out.nextDrawLane80096590;
    return out;
}

namespace {

bool BuildTransitionGraphInput8001EBF4(
    PrSS0TransitionDirect::FastTransitionGraphInput8001EA74& input)
{
    input = {};
    const auto& graph = s_directoryGraphProjection80015788
        ? *s_directoryGraphProjection80015788 : s_titleGraphState;
    const bool resident = s_directoryGraphProjection80015788 != nullptr;
    if ((!s_directoryGraphProjection80015788 &&
         (!s_resourceState.resourceReady || !s_resourceState.titleGraphControlKnown)) ||
        !graph.mainPageWorkLists80087288Initialized ||
        graph.word_80096590 > 1u ||
        graph.dword_8006ED50[0] != (resident
            ? PrSceneDrawBufferDirect::kDrawBufferBase80080CF8 : 0x801AE430u) ||
        graph.dword_8006ED50[1] != (resident
            ? PrSceneDrawBufferDirect::kDrawBufferBase80083FC0 : 0x801B8CF0u)) {
        return false;
    }

    const uint16_t slot = graph.word_80096590;
    const auto& page = graph.mainPageWorkLists80087288[slot];
    input.known = true;
    input.residentMainPacketLanes8001E34C = resident;
    input.drawSlot8004019C = slot;
    input.packetAllocator8006ED50 = graph.dword_8006ED50[slot];
    input.mainPageWorkAddress80087288 = page.workAddr;
    input.mainPageOtHeadAddress80088288 = page.otHeadAddr;
    input.mainPageWorkHeadAddress80040CC8 = page.work.headAddr_04;
    input.mainPageWorkOrder = page.work.order_00;
    return true;
}

FastTransitionPresentResult8001EBF4 CommitTransitionPresentPlan8001EBF4(
    const PrSS0TransitionDirect::FastTransitionPresentPlan8001EBF4& plan);

} // namespace

FastTransitionPresentResult8001EBF4 ApplyFastTransitionPresent8001EBF4(
    const PrSS0TransitionDirect::FastTransitionVisualFrame800201AC&
        visualFrame)
{
    PrSS0TransitionDirect::FastTransitionGraphInput8001EA74 input{};
    if (!BuildTransitionGraphInput8001EBF4(input)) {
        return {};
    }
    const auto plan =
        PrSS0TransitionDirect::BuildFastTransitionPresentPlan8001EBF4(
            visualFrame, input);
    return CommitTransitionPresentPlan8001EBF4(plan);
}

namespace {

FastTransitionPresentResult8001EBF4 CommitTransitionPresentPlan8001EBF4(
    const PrSS0TransitionDirect::FastTransitionPresentPlan8001EBF4& plan)
{
    FastTransitionPresentResult8001EBF4 out{};
    if (!plan.known) {
        return out;
    }

    auto& graph = s_directoryGraphProjection80015788
        ? *s_directoryGraphProjection80015788 : s_titleGraphState;
    auto nextGraph = graph;
    PrPsxGraphOwnerDirect::PsxCall80040F90_SetPacketAllocator(
        nextGraph, plan.packetAllocator8006ED50);
    const auto cleared =
        PrPsxGraphOwnerDirect::PsxCall8001E374_ClearMainPageWork(
            nextGraph, static_cast<uint8_t>(plan.drawSlotBefore));
    if (cleared.clearOtagRLength != (1u << 14u) ||
        nextGraph.dword_800901C8 != plan.packetAllocator8006ED50) {
        return out;
    }
    const auto flip =
        PrPsxGraphOwnerDirect::PsxCall80040370_FlipGraph(nextGraph);
    if (flip.previousSlot != plan.drawSlotBefore ||
        flip.nextSlot != plan.drawSlotAfter ||
        nextGraph.word_80096590 != plan.drawSlotAfter) {
        return out;
    }

    graph = nextGraph;
    out.committed = true;
    out.drawSlotBefore = plan.drawSlotBefore;
    out.drawSlotAfter = plan.drawSlotAfter;
    out.packetAllocator8006ED50 = plan.packetAllocator8006ED50;
    out.mainPageWorkAddress80087288 = plan.mainPageWorkAddress80087288;
    out.clearColorRequested80040420 =
        plan.clearColorRequested80040420;
    out.mainPageWorkSubmitRequested80040CA4 =
        plan.mainPageWorkSubmitRequested80040CA4;
    return out;
}

} // namespace

FastTransitionPresentResult8001EBF4 ApplySlowTransitionPresent8001EBF4(
    const PrSS0TransitionDirect::SlowTransitionVisualFrame80020110&
        visualFrame)
{
    PrSS0TransitionDirect::FastTransitionGraphInput8001EA74 input{};
    if (!BuildTransitionGraphInput8001EBF4(input)) {
        return {};
    }
    const auto plan =
        PrSS0TransitionDirect::BuildSlowTransitionPresentPlan8001EBF4(
            visualFrame, input);
    return CommitTransitionPresentPlan8001EBF4(plan);
}

bool BuildFirstPaKageMode25PacketPlan800428B0()
{
    const auto built =
        PrSS0TitlePacketPlanDirect::BuildFirstPaKageMode25PacketPlan800428B0(
            s_firstPaKageDrawDesc,
            s_titlePacketWork,
            s_titleGraphState,
            s_firstPaKageMode25Geometry800428B0,
            s_firstPaKageMode25DeformedTriangleCarrier.primitiveIndex);
    if (!built.ready || !built.plan.readyToCommit) {
        ClearFirstPaKageMode25PacketPlan800428B0();
        return false;
    }
    s_firstPaKageMode25PacketPlan800428B0 = built;
    s_resourceState.firstPaKageMode25PacketPlanKnown = true;
    return true;
}

bool CommitFirstPaKageMode25PacketPlan800428B0()
{
    if (!s_resourceState.firstPaKageMode25PacketPlanKnown) {
        ClearFirstPaKageMode25PacketCommit800428B0();
        return false;
    }
    const auto committed =
        PrSS0TitlePacketCommitDirect::
            CommitFirstPaKageMode25PacketPlan800428B0(
                s_titlePacketWork,
                s_titleGraphState,
                s_firstPaKageMode25PacketPlan800428B0);
    if (!committed.committed) {
        ClearFirstPaKageMode25PacketCommit800428B0();
        return false;
    }
    s_firstPaKageMode25PacketCommit800428B0 = committed;
    s_resourceState.firstPaKageMode25PacketCommitKnown = true;
    return true;
}

bool ExecutePaMode25PrimitiveGroup8004274C()
{
    ClearPaMode25PrimitiveGroup8004274C();
    if (!s_firstPaMode25DeformedTriangleCarrier.known ||
        !s_resourceState.firstPaRuntimeMatrixKnown80041A68 ||
        !s_runtimePrimaryPaMimeBinding.known ||
        s_runtimePrimaryPaMimeBinding.mimeKind == MimeKind::Count ||
        !s_runtimePrimaryPaMimeBinding.loopFlagKnown ||
        s_runtimePrimaryPaMimeBinding.loopFlag > 1u ||
        !s_firstPaDrawDesc.primitiveGroup.known) {
        return false;
    }

    const MimeKind mimeKind = s_runtimePrimaryPaMimeBinding.mimeKind;
    const VdfData* vdf = GetMimeVdf(mimeKind);
    const DatData* dat = GetMimeDat(mimeKind);
    if (vdf == nullptr || dat == nullptr) {
        return false;
    }
    PrSS0TitleTransformDirect::MimeRuntimeInput80013EA8 mime{};
    mime.bindingKnown = true;
    mime.vdf = vdf;
    mime.dat = dat;
    mime.objectIndex = s_firstPaDrawDesc.objectIndex;
    mime.sampleFrameKnown = true;
    mime.sampleFrame = s_firstPaMode25DeformedTriangleCarrier.sampleFrame;
    mime.loopFlagKnown = true;
    mime.loopFlag = s_runtimePrimaryPaMimeBinding.loopFlag;

    const auto executed =
        PrSS0TitlePrimitiveGroupDirect::
            ExecuteMode25PrimitiveRange8004274C(
                s_firstPaDrawDesc,
                PrSS0TitleTransformDirect::ModelPath::Pa,
                mime,
                &s_firstPaRuntimeMatrix80041A68,
                true,
                s_titlePacketWork,
                s_titleGraphState,
                s_firstPaDrawDesc.primitiveGroup.firstPrimitiveIndex,
                s_firstPaDrawDesc.primitiveGroup.primitiveCount);
    s_paMode25PrimitiveGroup8004274C = executed;
    s_resourceState.paMode25PrimitiveGroupResultKnown8004274C = true;
    return executed.complete;
}

bool ExecutePaKageMode25PrimitiveGroup8004274C()
{
    ClearFirstPaKageMode25Rtpt3Carrier();
    if (!s_firstPaKageMode25DeformedTriangleCarrier.known ||
        !s_resourceState.firstPaKageRuntimeMatrixKnown ||
        !s_runtimeChannel2MimeBinding.known ||
        !s_firstPaKageDrawDesc.primitiveGroup.known) {
        return false;
    }

    const VdfData* vdf = GetMimeVdf(s_runtimeChannel2MimeBinding.mimeKind);
    const DatData* dat = GetMimeDat(s_runtimeChannel2MimeBinding.mimeKind);
    if (vdf == nullptr || dat == nullptr) {
        return false;
    }
    PrSS0TitleTransformDirect::MimeRuntimeInput80013EA8 mime{};
    mime.bindingKnown = true;
    mime.vdf = vdf;
    mime.dat = dat;
    mime.objectIndex = s_firstPaKageDrawDesc.objectIndex;
    mime.sampleFrameKnown = true;
    mime.sampleFrame =
        s_firstPaKageMode25DeformedTriangleCarrier.sampleFrame;
    mime.loopFlagKnown = true;
    mime.loopFlag = 0u;

    const auto executed =
        PrSS0TitlePrimitiveGroupDirect::
            ExecuteMode25PrimitiveRange8004274C(
                s_firstPaKageDrawDesc,
                PrSS0TitleTransformDirect::ModelPath::PaKage,
                mime,
                &s_firstPaKageRuntimeMatrix800406D8,
                true,
                s_titlePacketWork,
                s_titleGraphState,
                s_firstPaKageDrawDesc.primitiveGroup.firstPrimitiveIndex,
                s_firstPaKageDrawDesc.primitiveGroup.primitiveCount);
    s_paKageMode25PrimitiveGroup8004274C = executed;
    s_resourceState.paKageMode25PrimitiveGroupResultKnown8004274C = true;
    if (executed.firstExecutionKnown) {
        s_firstPaKageMode25Rtpt3Input801C5E60 =
            executed.firstExecution.input;
        s_firstPaKageMode25Rtpt3Output800428B0 =
            executed.firstExecution.output;
        s_firstPaKageMode25Geometry800428B0 =
            executed.firstExecution.geometry;
        s_resourceState.firstPaKageMode25Rtpt3InputKnown = true;
        s_resourceState.firstPaKageMode25Rtpt3OutputKnown = true;
        s_resourceState.firstPaKageMode25GeometryKnown = true;
    }
    if (executed.firstPlanKnown) {
        s_firstPaKageMode25PacketPlan800428B0 = executed.firstPlan;
        s_resourceState.firstPaKageMode25PacketPlanKnown = true;
    }
    if (executed.firstCommitKnown) {
        s_firstPaKageMode25PacketCommit800428B0 = executed.firstCommit;
        s_resourceState.firstPaKageMode25PacketCommitKnown = true;
    }
    return executed.complete;
}

static bool HasLoHpActivationProvenance801C5EF0()
{
    return s_titlePacketWork.loHpGate801CB600 == 1u &&
           s_resourceState.loHpGateKnown801CB600 &&
           s_runtimeChannel1MimeBinding.known &&
           s_runtimeChannel1MimeBinding.cursorKnown &&
           s_runtimeChannel1MimeBinding.sourceEventIndex ==
               kLoHpEventIndex801C5190 &&
           s_runtimeChannel1MimeBinding.thresholdTick96 ==
               kLoHpEventTick96801C5190 &&
           s_runtimeChannel1MimeBinding.appliedTick96 >=
               s_runtimeChannel1MimeBinding.thresholdTick96 &&
           s_runtimeChannel1MimeBinding.resourcePairIndex ==
               kLoResourcePairIndex801C6C14 &&
           s_runtimeChannel1MimeBinding.mimeKind == MimeKind::LogoNew &&
           s_runtimeChannel3MimeBinding.known &&
           s_runtimeChannel3MimeBinding.cursorKnown &&
           s_runtimeChannel3MimeBinding.sourceEventIndex ==
               s_runtimeChannel1MimeBinding.sourceEventIndex &&
           s_runtimeChannel3MimeBinding.thresholdTick96 ==
               s_runtimeChannel1MimeBinding.thresholdTick96 &&
           s_runtimeChannel3MimeBinding.appliedTick96 ==
               s_runtimeChannel1MimeBinding.appliedTick96 &&
           s_runtimeChannel3MimeBinding.resourcePairIndex ==
               kHpResourcePairIndex801C6C14 &&
           s_runtimeChannel3MimeBinding.mimeKind == MimeKind::Hiphop;
}

static bool HasHpActivationProvenance801C5EF0()
{
    return s_titlePacketWork.loHpGate801CB600 == 1u &&
           s_resourceState.loHpGateKnown801CB600 &&
           s_runtimeChannel3MimeBinding.known &&
           s_runtimeChannel3MimeBinding.cursorKnown &&
           s_runtimeChannel3MimeBinding.sourceEventIndex ==
               kLoHpEventIndex801C5190 &&
           s_runtimeChannel3MimeBinding.thresholdTick96 ==
               kLoHpEventTick96801C5190 &&
           s_runtimeChannel3MimeBinding.appliedTick96 >=
               s_runtimeChannel3MimeBinding.thresholdTick96 &&
           s_runtimeChannel3MimeBinding.resourcePairIndex ==
               kHpResourcePairIndex801C6C14 &&
           s_runtimeChannel3MimeBinding.mimeKind == MimeKind::Hiphop;
}

static bool HasTitleShortcutActivationProvenance801C6410()
{
    const uint32_t loopCount =
        s_resourceState.titleEarlyInputShortcutLoopCount801CC676;
    return loopCount != 0u &&
           s_resourceState.titleEarlyInputShortcutSetupKnown801C6410 &&
           s_resourceState.titleEarlyInputShortcutReadyKnown801C6410 &&
           s_resourceState.titleRenderActiveKnown801C9548 &&
           s_resourceState.titleRenderActive801C9548 == 1u &&
           s_resourceState.loHpGateKnown801CB600 &&
           s_resourceState.loHpGate801CB600 == 1u &&
           s_resourceState.presentExtraKnown801CFAEC &&
           s_resourceState.presentExtra801CFAEC == 0u &&
           // The selector immediately overwrites the PA/PA_KAGE pair with
           // the cursor-specific TurnLeft/TurnRight resource.  That is the
           // normal 801C5854 selector write, not a loss of the one-time
           // 801C5AB4 LO/HP activation.  Keep the current PA pair valid but
           // key shortcut provenance to the dedicated LO/HP/camera/TOD
           // bindings below; otherwise selecting MENU after an early skip
           // incorrectly drops LO/HP and makes the title exit wait stall.
           s_runtimePrimaryPaMimeBinding.known &&
           s_runtimePrimaryPaMimeBinding.mimeKind != MimeKind::Count &&
           s_runtimeChannel2MimeBinding.known &&
           s_runtimeChannel2MimeBinding.mimeKind != MimeKind::Count &&
           s_runtimeChannel1MimeBinding.known &&
           s_runtimeChannel1MimeBinding.earlyInputShortcutSetup801C6530 &&
           s_runtimeChannel1MimeBinding.cursorKnown &&
           // The exact loopCount/loopCount+1 window belongs to the initial
           // 801C6410 ready check.  Once the shortcut is committed, the
           // selector keeps advancing the same MIME source while it waits;
           // retain activation provenance for that monotonic cursor.
           s_runtimeChannel1MimeBinding.cursor >= loopCount &&
           s_runtimeChannel1MimeBinding.resourcePairIndex ==
               kShortcutChannel1PairIndex801C6530 &&
           s_runtimeChannel1MimeBinding.mimeKind == MimeKind::LogoNew &&
           s_runtimeChannel3MimeBinding.known &&
           s_runtimeChannel3MimeBinding.earlyInputShortcutSetup801C6578 &&
           s_runtimeChannel3MimeBinding.cursorKnown &&
           s_runtimeChannel3MimeBinding.cursor >= loopCount &&
           s_runtimeChannel3MimeBinding.resourcePairIndex ==
               kShortcutChannel3PairIndex801C6578 &&
           s_runtimeChannel3MimeBinding.mimeKind == MimeKind::Hiphop &&
           s_runtimeTitleTodBinding.known &&
           s_runtimeTitleTodBinding.earlyInputShortcutReady801C5AB4 &&
           s_runtimeTitleTodBinding.todKind == TitleTodKind::PaLoc2;
}

bool CanExecuteLoHpPrimitiveGroups801C5EF0()
{
    return (HasLoHpActivationProvenance801C5EF0() ||
            HasTitleShortcutActivationProvenance801C6410()) &&
           s_resourceState.firstLoDrawDescKnown8001AF1C &&
           s_resourceState.firstLoPrimitiveGroupKnown8004274C &&
           s_resourceState.firstLoMode25BaseTriangleKnown &&
           s_resourceState.firstHpDrawDescKnown8001AF1C &&
           s_resourceState.firstHpPrimitiveGroupKnown8004274C &&
           s_resourceState.firstHpMode25BaseTriangleKnown &&
           s_resourceState.loCoordKnown8004049C &&
           s_resourceState.hpCoordKnown8004049C &&
           s_resourceState.titleCameraMatrixKnown80092880 &&
           HasPacketCapacity801C5EF0(kLoMaximumPacketCount801C5EF0);
}

bool CanExecuteHpPrimitiveGroup801C5EF0()
{
    return (HasHpActivationProvenance801C5EF0() ||
            HasTitleShortcutActivationProvenance801C6410()) &&
           s_resourceState.firstHpDrawDescKnown8001AF1C &&
           s_resourceState.firstHpPrimitiveGroupKnown8004274C &&
           s_resourceState.firstHpMode25BaseTriangleKnown &&
           s_resourceState.hpCoordKnown8004049C &&
           s_resourceState.titleCameraMatrixKnown80092880 &&
           HasPacketCapacity801C5EF0(kHpMaximumPacketCount801C5EF0);
}

bool ExecuteLoMode25PrimitiveGroup8004274C()
{
    if ((!HasLoHpActivationProvenance801C5EF0() &&
         !HasTitleShortcutActivationProvenance801C6410()) ||
        !s_firstLoMode25DeformedTriangleCarrier.known ||
        !s_resourceState.firstLoRuntimeMatrixKnown80041A68 ||
        !s_runtimeChannel1MimeBinding.known ||
        s_runtimeChannel1MimeBinding.mimeKind != MimeKind::LogoNew ||
        !s_firstLoDrawDesc.primitiveGroup.known ||
        !HasPacketCapacity801C5EF0(kLoMaximumPacketCount801C5EF0)) {
        return false;
    }
    ClearLoMode25PrimitiveGroup8004274C();

    const VdfData* vdf = GetMimeVdf(MimeKind::LogoNew);
    const DatData* dat = GetMimeDat(MimeKind::LogoNew);
    if (vdf == nullptr || dat == nullptr) {
        return false;
    }
    PrSS0TitleTransformDirect::MimeRuntimeInput80013EA8 mime{};
    mime.bindingKnown = true;
    mime.vdf = vdf;
    mime.dat = dat;
    mime.objectIndex = s_firstLoDrawDesc.objectIndex;
    mime.sampleFrameKnown = true;
    mime.sampleFrame = s_firstLoMode25DeformedTriangleCarrier.sampleFrame;
    mime.loopFlagKnown = true;
    mime.loopFlag = 0u;

    const auto executed =
        PrSS0TitlePrimitiveGroupDirect::
            ExecuteMode25PrimitiveRange8004274C(
                s_firstLoDrawDesc,
                PrSS0TitleTransformDirect::ModelPath::Lo,
                mime,
                &s_firstLoRuntimeMatrix80041A68,
                true,
                s_titlePacketWork,
                s_titleGraphState,
                s_firstLoDrawDesc.primitiveGroup.firstPrimitiveIndex,
                s_firstLoDrawDesc.primitiveGroup.primitiveCount);
    s_loMode25PrimitiveGroup8004274C = executed;
    s_resourceState.loMode25PrimitiveGroupResultKnown8004274C = true;
    return executed.complete;
}

bool ExecuteHpMode25PrimitiveGroup8004274C()
{
    if ((!HasHpActivationProvenance801C5EF0() &&
         !HasTitleShortcutActivationProvenance801C6410()) ||
        !s_firstHpMode25DeformedTriangleCarrier.known ||
        !s_resourceState.firstHpRuntimeMatrixKnown80041A68 ||
        !s_runtimeChannel3MimeBinding.known ||
        s_runtimeChannel3MimeBinding.mimeKind != MimeKind::Hiphop ||
        !s_firstHpDrawDesc.primitiveGroup.known ||
        !HasPacketCapacity801C5EF0(kHpMaximumPacketCount801C5EF0)) {
        return false;
    }
    ClearHpMode25PrimitiveGroup8004274C();

    const VdfData* vdf = GetMimeVdf(MimeKind::Hiphop);
    const DatData* dat = GetMimeDat(MimeKind::Hiphop);
    if (vdf == nullptr || dat == nullptr) {
        return false;
    }
    PrSS0TitleTransformDirect::MimeRuntimeInput80013EA8 mime{};
    mime.bindingKnown = true;
    mime.vdf = vdf;
    mime.dat = dat;
    mime.objectIndex = s_firstHpDrawDesc.objectIndex;
    mime.sampleFrameKnown = true;
    mime.sampleFrame = s_firstHpMode25DeformedTriangleCarrier.sampleFrame;
    mime.loopFlagKnown = true;
    mime.loopFlag = 0u;

    const auto executed =
        PrSS0TitlePrimitiveGroupDirect::
            ExecuteMode25PrimitiveRange8004274C(
                s_firstHpDrawDesc,
                PrSS0TitleTransformDirect::ModelPath::Hp,
                mime,
                &s_firstHpRuntimeMatrix80041A68,
                true,
                s_titlePacketWork,
                s_titleGraphState,
                s_firstHpDrawDesc.primitiveGroup.firstPrimitiveIndex,
                s_firstHpDrawDesc.primitiveGroup.primitiveCount);
    s_hpMode25PrimitiveGroup8004274C = executed;
    s_resourceState.hpMode25PrimitiveGroupResultKnown8004274C = true;
    return executed.complete;
}

const TmdObject* GetModelObject(ModelKind kind, std::size_t objectIndex)
{
    const std::size_t index = ModelIndex(kind);
    if (index >= kModelCount || !s_modelViews[index].loaded ||
        objectIndex >= s_models[index].objects.size()) {
        return nullptr;
    }
    return &s_models[index].objects[objectIndex];
}

const VdfData* GetMimeVdf(MimeKind kind)
{
    const std::size_t index = MimeIndex(kind);
    return index < kMimeCount && s_mimeViews[index].pairLoaded
               ? &s_vdfs[index]
               : nullptr;
}

const DatData* GetMimeDat(MimeKind kind)
{
    const std::size_t index = MimeIndex(kind);
    return index < kMimeCount && s_mimeViews[index].pairLoaded
               ? &s_dats[index]
               : nullptr;
}

const TodData* GetTitleTod(TitleTodKind kind)
{
    const std::size_t index = TitleTodIndex(kind);
    return index < kTitleTodCount && s_titleTodViews[index].loaded
               ? &s_titleTods[index]
               : nullptr;
}

const TodCoordMatrix* GetPaLoc2Coord()
{
    return s_resourceState.paLoc2CoordKnown ? &s_paLoc2Coord : nullptr;
}

const PrPsxGteDirect::Matrix3x4* GetTitlePanelCoord801CD7BC()
{
    return s_resourceState.titlePanelCoordKnown801C5D8C
               ? &s_titlePanelCoord801CD7BC
               : nullptr;
}

const PrPsxGteDirect::Matrix3x4*
GetRuntimeTitleDrawCoordWorld800417A4()
{
    return s_resourceState.titleDrawCoordWorldKnown800417A4
               ? &s_runtimeTitleDrawCoordWorld
               : nullptr;
}

const PrPsxGteDirect::Matrix3x4* GetRuntimeTitleCameraMatrix80092880()
{
    return s_resourceState.titleCameraMatrixKnown80092880
               ? &s_runtimeTitleCameraMatrix80092880
               : nullptr;
}

const PrPsxGteDirect::Matrix3x4*
GetRuntimeTitlePanelWithCamera80041A68()
{
    return s_resourceState.titlePanelWithCameraKnown80041A68
               ? &s_runtimeTitlePanelWithCamera80041A68
               : nullptr;
}

const PrPsxGteDirect::Matrix3x4*
GetFirstPaKageRuntimeMatrix800406D8()
{
    return s_resourceState.firstPaKageRuntimeMatrixKnown
               ? &s_firstPaKageRuntimeMatrix800406D8
               : nullptr;
}

const PrSS0TitleTransformDirect::Mode25Rtpt3Input*
GetFirstPaKageMode25Rtpt3Input801C5E60()
{
    return s_resourceState.firstPaKageMode25Rtpt3InputKnown
               ? &s_firstPaKageMode25Rtpt3Input801C5E60
               : nullptr;
}

const PrPsxGteDirect::Rtpt3ExactOutput280030*
GetFirstPaKageMode25Rtpt3Output800428B0()
{
    return s_resourceState.firstPaKageMode25Rtpt3OutputKnown
               ? &s_firstPaKageMode25Rtpt3Output800428B0
               : nullptr;
}

const PrPsxGteDirect::Mode25TriangleGeometryTrace*
GetFirstPaKageMode25Geometry800428B0()
{
    return s_resourceState.firstPaKageMode25GeometryKnown
               ? &s_firstPaKageMode25Geometry800428B0
               : nullptr;
}

const PrSS0TitlePacketWorkDirect::RuntimeState801C609C*
GetTitlePacketWork801C609C()
{
    return s_resourceState.titlePacketWorkKnown801C609C
               ? &s_titlePacketWork
               : nullptr;
}

const PrSS0TitleDrawDescDirect::RuntimeState8001AF1C*
GetFirstPaKageDrawDesc8001AF1C()
{
    return s_resourceState.firstPaKageDrawDescKnown8001AF1C
               ? &s_firstPaKageDrawDesc
               : nullptr;
}

const PrSS0TitleDrawDescDirect::RuntimeState8001AF1C*
GetFirstPaDrawDesc8001AF1C()
{
    return s_resourceState.firstPaDrawDescKnown8001AF1C
               ? &s_firstPaDrawDesc
               : nullptr;
}

const PrSS0TitleDrawDescDirect::RuntimeState8001AF1C*
GetFirstLoDrawDesc8001AF1C()
{
    return s_resourceState.firstLoDrawDescKnown8001AF1C
               ? &s_firstLoDrawDesc
               : nullptr;
}

const PrSS0TitleDrawDescDirect::RuntimeState8001AF1C*
GetFirstHpDrawDesc8001AF1C()
{
    return s_resourceState.firstHpDrawDescKnown8001AF1C
               ? &s_firstHpDrawDesc
               : nullptr;
}

const PrSS0TitlePacketPlanDirect::BuildResult800428B0*
GetFirstPaKageMode25PacketPlan800428B0()
{
    return s_resourceState.firstPaKageMode25PacketPlanKnown
               ? &s_firstPaKageMode25PacketPlan800428B0
               : nullptr;
}

const PrSS0TitlePacketCommitDirect::CommitResult800428B0*
GetFirstPaKageMode25PacketCommit800428B0()
{
    return s_resourceState.firstPaKageMode25PacketCommitKnown
               ? &s_firstPaKageMode25PacketCommit800428B0
               : nullptr;
}

const PrSS0TitlePrimitiveGroupDirect::ExecuteResult8004274C*
GetPaKageMode25PrimitiveGroup8004274C()
{
    return s_resourceState.paKageMode25PrimitiveGroupResultKnown8004274C
               ? &s_paKageMode25PrimitiveGroup8004274C
               : nullptr;
}

const PrSS0TitlePrimitiveGroupDirect::ExecuteResult8004274C*
GetPaMode25PrimitiveGroup8004274C()
{
    return s_resourceState.paMode25PrimitiveGroupResultKnown8004274C
               ? &s_paMode25PrimitiveGroup8004274C
               : nullptr;
}

const PrSS0TitlePrimitiveGroupDirect::ExecuteResult8004274C*
GetLoMode25PrimitiveGroup8004274C()
{
    return s_resourceState.loMode25PrimitiveGroupResultKnown8004274C
               ? &s_loMode25PrimitiveGroup8004274C
               : nullptr;
}

const PrSS0TitlePrimitiveGroupDirect::ExecuteResult8004274C*
GetHpMode25PrimitiveGroup8004274C()
{
    return s_resourceState.hpMode25PrimitiveGroupResultKnown8004274C
               ? &s_hpMode25PrimitiveGroup8004274C
               : nullptr;
}

const PrPsxGraphOwnerDirect::PsxGraphState* GetTitleGraphState()
{
    return s_resourceState.titleGraphControlKnown ? &s_titleGraphState
                                                   : nullptr;
}

PrPsxGraphOwnerDirect::PsxGraphState* GetTitleGraphStateMutable()
{
    return s_resourceState.titleGraphControlKnown ? &s_titleGraphState
                                                   : nullptr;
}

const PrSS0TitleTransformDirect::Mode25BaseTriangleCarrier*
GetFirstPaKageMode25BaseTriangleCarrier()
{
    return s_resourceState.firstPaKageMode25BaseTriangleKnown
               ? &s_firstPaKageMode25BaseTriangleCarrier
               : nullptr;
}

const PrSS0TitleTransformDirect::Mode25DeformedTriangleCarrier*
GetFirstPaKageMode25DeformedTriangleCarrier()
{
    return s_resourceState.firstPaKageRuntimeVertexDeformationKnown
               ? &s_firstPaKageMode25DeformedTriangleCarrier
               : nullptr;
}

const char* ModelName(ModelKind kind)
{
    switch (kind) {
    case ModelKind::Hp:
        return "hp.tmd";
    case ModelKind::Lo:
        return "lo.tmd";
    case ModelKind::Pa:
        return "pa.tmd";
    case ModelKind::PaKage:
        return "pa_kage.tmd";
    case ModelKind::Count:
        break;
    }
    return "unknown";
}

const char* MimeVdfName(MimeKind kind)
{
    switch (kind) {
    case MimeKind::Hiphop:
        return "hippop.vdf";
    case MimeKind::Logo:
        return "logo.vdf";
    case MimeKind::LogoNew:
        return "logonew.vdf";
    case MimeKind::PaDance:
        return "pa_dance.vdf";
    case MimeKind::PaLeft:
        return "pa_l.vdf";
    case MimeKind::PaOki:
        return "pa_oki.vdf";
    case MimeKind::PaRight:
        return "pa_r.vdf";
    case MimeKind::PaTurnLeft:
        return "pa_t_l.vdf";
    case MimeKind::PaTurnRight:
        return "pa_t_r.vdf";
    case MimeKind::Count:
        break;
    }
    return "unknown.vdf";
}

const char* MimeDatName(MimeKind kind)
{
    switch (kind) {
    case MimeKind::Hiphop:
        return "hippop.dat";
    case MimeKind::Logo:
        return "logo.dat";
    case MimeKind::LogoNew:
        return "logonew.dat";
    case MimeKind::PaDance:
        return "pa_dance.dat";
    case MimeKind::PaLeft:
        return "pa_l.dat";
    case MimeKind::PaOki:
        return "pa_oki.dat";
    case MimeKind::PaRight:
        return "pa_r.dat";
    case MimeKind::PaTurnLeft:
        return "pa_t_l.dat";
    case MimeKind::PaTurnRight:
        return "pa_t_r.dat";
    case MimeKind::Count:
        break;
    }
    return "unknown.dat";
}

const char* TitleTodName(TitleTodKind kind)
{
    switch (kind) {
    case TitleTodKind::PaDance:
        return "pa_dance.tod";
    case TitleTodKind::PaLoc:
        return "pa_loc.tod";
    case TitleTodKind::PaLoc2:
        return "pa_loc2.tod";
    case TitleTodKind::Count:
        break;
    }
    return "unknown.tod";
}

} // namespace PrSS0TitleTmdBackend
