#pragma once

#include "pr_ss0_title_packet_work_direct.h"

#include <array>
#include <cstddef>
#include <cstdint>

namespace PrSS0TitlePacketRenderDirect {

constexpr std::size_t kMaxMode25DrawCommands801C689C =
    PrPsxGraphOwnerDirect::kTmdRuntimePacketWriteCapacity;

enum class Failure : uint8_t {
    None = 0,
    RuntimeNotReady,
    WorkShapeMismatch,
    OtMirrorUnknown,
    PacketMirrorUnknown,
    LinkNotFound,
    LinkAmbiguous,
    TraversalLimitExceeded,
    OtOrderMismatch,
    PacketBeforeOt,
    PacketLengthMismatch,
    PacketOpcodeMismatch,
    UnreachablePacketWrite,
};

struct Mode25DrawCommand801C689C {
    bool valid = false;
    uint32_t packetAddress = 0;
    uint32_t traversalOrder = 0;
    uint16_t priority = 0;
    uint16_t tpage = 0;
    uint16_t clut = 0;
    std::array<int16_t, 3> x{};
    std::array<int16_t, 3> y{};
    std::array<uint8_t, 3> u{};
    std::array<uint8_t, 3> v{};
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
};

struct BuildResult801C689C {
    bool complete = false;
    Failure failure = Failure::None;
    uint8_t drawBuffer = 0;
    uint32_t visitedOtSlots = 0;
    uint32_t visitedPackets = 0;
    uint32_t commandCount = 0;
    std::array<Mode25DrawCommand801C689C,
               kMaxMode25DrawCommands801C689C>
        commands{};
};

struct HostPresentProjection801C689C {
    bool accepted = false;
    bool titleCommandsVisibleOnCurrentHostFrame = false;
    bool offscreenWhiteFillRequested = false;
    bool currentHostFrameWhiteFillRequested = false;
};

struct HostVisibleTitleFrameCache801C689C {
    bool valid = false;
    uint32_t resourceGeneration = 0;
    uint8_t sourceDrawBuffer = 0;
    BuildResult801C689C decoded{};
};

BuildResult801C689C BuildCurrentMode25DrawCommands801C689C(
    const PrSS0TitlePacketWorkDirect::RuntimeState801C609C& runtime);

HostPresentProjection801C689C BuildHostPresentProjection801C689C(
    bool presentModelApplied,
    bool extraFlipGraphStateAdvanced,
    bool fullHeightWhiteFillRequested);

void ResetHostVisibleTitleFrameCache801C689C(
    HostVisibleTitleFrameCache801C689C& cache);

bool CommitHostVisibleTitleFrameCache801C689C(
    HostVisibleTitleFrameCache801C689C& cache,
    const BuildResult801C689C& decoded,
    const HostPresentProjection801C689C& projection,
    uint32_t resourceGeneration);

const BuildResult801C689C* ResolveHostVisibleTitleFrameCache801C689C(
    const HostVisibleTitleFrameCache801C689C& cache,
    uint32_t resourceGeneration);

} // namespace PrSS0TitlePacketRenderDirect
