#pragma once

#include "pr_ss0_title_packet_render_direct.h"

#include <cstdint>

struct PrGameContext;

namespace PrSS0TitleDrawBackend {

struct SubmitResult801C689C {
    bool decodeComplete = false;
    bool textureUploadAttempted = false;
    bool textureUploadComplete = false;
    bool directScene0AtlasUploadCommitted = false;
    bool texturePreflightComplete = false;
    PrSS0TitlePacketRenderDirect::Failure decodeFailure{};
    uint32_t decodedTriangles = 0;
    uint32_t submittedTriangles = 0;
    uint32_t missingTextures = 0;
    uint32_t readyTpages = 0;
    uint32_t failedTpageUploads = 0;
};

SubmitResult801C689C SubmitMode25DrawCommands801C689C(
    PrGameContext& ctx,
    const PrSS0TitlePacketRenderDirect::BuildResult801C689C& decoded,
    float viewportX,
    float viewportY,
    float viewportScale,
    int layer);

SubmitResult801C689C SubmitCurrentMode25Packets801C689C(
    PrGameContext& ctx,
    const PrSS0TitlePacketWorkDirect::RuntimeState801C609C& runtime,
    float viewportX,
    float viewportY,
    float viewportScale,
    int layer);

} // namespace PrSS0TitleDrawBackend
