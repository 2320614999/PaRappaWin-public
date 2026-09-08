#pragma once

#include <cstdint>
#include <filesystem>

#include "pr_scene_table.h"

struct PrGameContext;

struct PrMainStage1RuntimePsxMemorySourceReadAudit {
    bool mainGlobalReadAttempted = false;
    bool mainGlobalReadable = false;
    bool mainGlobalSelfBootstrapBlocked = false;
    bool loaderHeapReadAttempted = false;
    bool loaderHeapReadable = false;
    bool knownStateRelayReadAttempted = false;
    bool knownStateRelayReadable = false;
    bool cdMmioSnapshotReadAttempted = false;
    bool cdMmioSnapshotReadable = false;
    bool exactCdReadAttempted = false;
    bool exactCdReadable = false;
};

struct PrMainWord800916F0ProviderIngressDebug {
    bool attempted = false;
    bool readable = false;
    bool publishAttempted = false;
    bool published = false;
    bool knownAfter = false;
    bool sourceMainGlobalAttempted = false;
    bool sourceMainGlobalReadable = false;
    bool sourceMainGlobalSelfBootstrapBlocked = false;
    bool sourceLoaderHeapAttempted = false;
    bool sourceLoaderHeapReadable = false;
    bool sourceKnownStateRelayAttempted = false;
    bool sourceKnownStateRelayReadable = false;
    bool sourceCdMmioSnapshotAttempted = false;
    bool sourceCdMmioSnapshotReadable = false;
    bool sourceExactCdAttempted = false;
    bool sourceExactCdReadable = false;
    uint32_t frame = 0;
    uint32_t attemptCount = 0;
    uint32_t readableCount = 0;
    uint32_t sourceMissingCount = 0;
    uint32_t publishedCount = 0;
};

struct PrMain {
    static bool InitSceneTable(PrSceneTable& table, const std::filesystem::path& dataRoot);
    static void Run(PrGameContext& ctx, PrSceneTable& table);
    static int GetStage1Fn2ResultThisFrameDebug();
    static int GetStage1PendingSceneThisFrameDebug();
    static PrMainStage1RuntimePsxMemorySourceReadAudit
    AuditStage1RuntimePsxMemorySources(const PrGameContext& ctx,
                                       uint32_t psxAddress,
                                       uint32_t byteSize);
    static bool TryPublishWord800916F0FromStage1RuntimePsxMemoryProvider(
        PrGameContext& ctx);
    static PrMainWord800916F0ProviderIngressDebug
    GetWord800916F0ProviderIngressDebug();
};
