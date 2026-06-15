#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

#include "pr_stage1_overlay_parser.h"

namespace PrStage1CameraMotionDirect {

// Direct-port carrier for PSX sub_800127F0/sub_800128DC/sub_80012960 BEZ
// camera playback. The renderer owns resource IO and presentation interpolation;
// this type owns only the PSX-style BEZ state and tick math.
struct CameraBezFloatSample {
    float posX = 0.0f;
    float posY = 0.0f;
    float posZ = 0.0f;
    float tgtX = 0.0f;
    float tgtY = 0.0f;
    float tgtZ = 0.0f;
};

struct CameraBezPlayer {
    static constexpr int kMaxEntries = 256;

    int16_t posX[kMaxEntries * 4] = {};
    int16_t posY[kMaxEntries * 4] = {};
    int16_t posZ[kMaxEntries * 4] = {};
    int16_t tgtX[kMaxEntries * 4] = {};
    int16_t tgtY[kMaxEntries * 4] = {};
    int16_t tgtZ[kMaxEntries * 4] = {};
    uint32_t extra[kMaxEntries] = {};

    int32_t totalEntries = 0;
    int32_t currentFrame = -1;
    int32_t tickDivision = 1;
    int32_t posMode = 1;
    int32_t tgtMode = 1;
    int32_t posVel[3] = {};
    int32_t tgtVel[3] = {};

    int32_t outPosX = 0;
    int32_t outPosY = 0;
    int32_t outPosZ = 0;
    int32_t outTgtX = 0;
    int32_t outTgtY = 0;
    int32_t outTgtZ = 0;

    bool loaded = false;
    bool active = false;
    bool useTrueCubic = false;

    bool LoadBez(const uint8_t* data, size_t size, int32_t period,
                 bool enableTrueCubic = false);
    void Start();
    void ResetLike800128DC();
    int Tick(int tick);
    int SampleAtTick(int tick);
    bool SampleFloatAtTick(float tick, CameraBezFloatSample& out) const;

    static int DetectMode(const int16_t* chX,
                          const int16_t* chY,
                          const int16_t* chZ,
                          int baseEntry,
                          int nextEntry);
};

bool ResolveStage1PairNames(const PrStage1OverlayData& data,
                            uint8_t pairIndex,
                            std::string& outDatName,
                            std::string& outVdfName);
bool ResolveStage1PairNamesByHandles(const PrStage1OverlayData& data,
                                     uint16_t datHandle,
                                     uint16_t vdfHandle,
                                     std::string& outDatName,
                                     std::string& outVdfName);
bool ResolveStage1BezName(const PrStage1OverlayData& data,
                          int16_t bezId,
                          std::string& outBezName);

struct MotionChannelState {
    uint8_t pairIndex = 0;
    uint32_t startFrame = 0;
    std::string datName;
    std::string vdfName;

    bool Matches(uint8_t candidatePairIndex,
                 uint32_t candidateStartFrame,
                 const std::string& candidateDatName,
                 const std::string& candidateVdfName) const;
    void Set(uint8_t selectedPairIndex,
             uint32_t selectedStartFrame,
             const std::string& selectedDatName,
             const std::string& selectedVdfName);
};

struct CameraRuntimeState {
    int16_t bezId = 0;
    uint32_t startFrame = 0;
    uint32_t sourceEventPsxAddr = 0;
    std::string bezName;
    bool usingNeutralFallback = true;
    uint32_t renderTickCounter = 0;
    uint32_t lastPlaybackFrame = 0;
    bool playbackFrameSeen = false;

    bool NeutralResetWouldChange() const;
    bool Matches(int16_t candidateBezId,
                 uint32_t candidateStartFrame,
                 uint32_t candidateSourceEventPsxAddr,
                 const std::string& candidateBezName) const;
    void ResetNeutral();
    void ResetPlaybackCounter();
    bool ConsumePlaybackTickForFrame(uint32_t currentFrame, int32_t& outTick);
    void Set(int16_t selectedBezId,
             uint32_t selectedStartFrame,
             uint32_t selectedSourceEventPsxAddr,
             const std::string& selectedBezName);
};

}  // namespace PrStage1CameraMotionDirect
