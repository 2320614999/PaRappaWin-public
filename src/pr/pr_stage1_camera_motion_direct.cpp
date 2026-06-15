#include "pr_stage1_camera_motion_direct.h"

#include "logger.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstring>

namespace PrStage1CameraMotionDirect {

static std::string ToLowerAsciiCopy(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {
        return (char)std::tolower(c);
    });
    return s;
}

static bool EndsWithAscii(const std::string& s, const char* suffix) {
    const size_t n = suffix ? std::strlen(suffix) : 0u;
    if (n == 0u || s.size() < n) {
        return false;
    }
    return s.compare(s.size() - n, n, suffix) == 0;
}

bool ResolveStage1PairNames(const PrStage1OverlayData& data,
                            uint8_t pairIndex,
                            std::string& outDatName,
                            std::string& outVdfName) {
    outDatName.clear();
    outVdfName.clear();

    const PrStage1PairEntry* entry = data.FindPairEntry(pairIndex);
    if (!entry) {
        return false;
    }

    auto assignName = [&](int16_t handle) {
        if (handle <= 0) {
            return;
        }
        const std::string* memName = data.FindMemHandleName((uint16_t)handle);
        if (!memName || memName->empty()) {
            return;
        }

        const std::string lowerName = ToLowerAsciiCopy(*memName);
        if (EndsWithAscii(lowerName, ".dat")) {
            outDatName = lowerName;
        } else if (EndsWithAscii(lowerName, ".vdf")) {
            outVdfName = lowerName;
        }
    };

    assignName(entry->idA);
    assignName(entry->idB);
    return !outDatName.empty() && !outVdfName.empty();
}

bool ResolveStage1PairNamesByHandles(const PrStage1OverlayData& data,
                                     uint16_t datHandle,
                                     uint16_t vdfHandle,
                                     std::string& outDatName,
                                     std::string& outVdfName) {
    outDatName.clear();
    outVdfName.clear();

    auto resolveName = [&](uint16_t handle,
                           const char* suffix,
                           std::string& outName) -> bool {
        if (handle == 0u) {
            return false;
        }

        const std::string* memName = data.FindMemHandleName(handle);
        if (!memName || memName->empty()) {
            return false;
        }

        outName = ToLowerAsciiCopy(*memName);
        return EndsWithAscii(outName, suffix);
    };

    return resolveName(datHandle, ".dat", outDatName) &&
           resolveName(vdfHandle, ".vdf", outVdfName);
}

bool ResolveStage1BezName(const PrStage1OverlayData& data,
                          int16_t bezId,
                          std::string& outBezName) {
    outBezName.clear();
    if (bezId <= 0) {
        return false;
    }

    const std::string* memName = data.FindMemHandleName((uint16_t)bezId);
    if (!memName || memName->empty()) {
        return false;
    }

    outBezName = ToLowerAsciiCopy(*memName);
    if (!EndsWithAscii(outBezName, ".bez")) {
        outBezName.clear();
        return false;
    }
    return true;
}

bool MotionChannelState::Matches(uint8_t candidatePairIndex,
                                 uint32_t candidateStartFrame,
                                 const std::string& candidateDatName,
                                 const std::string& candidateVdfName) const {
    return pairIndex == candidatePairIndex &&
           startFrame == candidateStartFrame &&
           datName == candidateDatName &&
           vdfName == candidateVdfName;
}

void MotionChannelState::Set(uint8_t selectedPairIndex,
                             uint32_t selectedStartFrame,
                             const std::string& selectedDatName,
                             const std::string& selectedVdfName) {
    pairIndex = selectedPairIndex;
    startFrame = selectedStartFrame;
    datName = selectedDatName;
    vdfName = selectedVdfName;
}

bool CameraRuntimeState::NeutralResetWouldChange() const {
    return !usingNeutralFallback ||
           bezId != 0 ||
           sourceEventPsxAddr != 0 ||
           !bezName.empty();
}

bool CameraRuntimeState::Matches(int16_t candidateBezId,
                                 uint32_t candidateStartFrame,
                                 uint32_t candidateSourceEventPsxAddr,
                                 const std::string& candidateBezName) const {
    return !usingNeutralFallback &&
           bezId == candidateBezId &&
           startFrame == candidateStartFrame &&
           sourceEventPsxAddr == candidateSourceEventPsxAddr &&
           bezName == candidateBezName;
}

void CameraRuntimeState::ResetNeutral() {
    *this = CameraRuntimeState{};
}

void CameraRuntimeState::ResetPlaybackCounter() {
    renderTickCounter = 0;
    lastPlaybackFrame = 0;
    playbackFrameSeen = false;
}

bool CameraRuntimeState::ConsumePlaybackTickForFrame(uint32_t currentFrame,
                                                     int32_t& outTick) {
    if (playbackFrameSeen && lastPlaybackFrame == currentFrame) {
        return false;
    }

    playbackFrameSeen = true;
    lastPlaybackFrame = currentFrame;
    outTick = static_cast<int32_t>(renderTickCounter++);
    return true;
}

void CameraRuntimeState::Set(int16_t selectedBezId,
                             uint32_t selectedStartFrame,
                             uint32_t selectedSourceEventPsxAddr,
                             const std::string& selectedBezName) {
    bezId = selectedBezId;
    startFrame = selectedStartFrame;
    sourceEventPsxAddr = selectedSourceEventPsxAddr;
    bezName = selectedBezName;
    usingNeutralFallback = false;
    ResetPlaybackCounter();
}

bool CameraBezPlayer::LoadBez(const uint8_t* data,
                              size_t size,
                              int32_t period,
                              bool enableTrueCubic) {
    if (!data || size < 4) {
        return false;
    }
    const uint32_t count = *(const uint32_t*)data;
    if (count == 0 || count > kMaxEntries) {
        return false;
    }
    if (size < 4 + count * 16) {
        return false;
    }

    totalEntries = (int32_t)count;
    currentFrame = 0;
    tickDivision = period;
    posMode = 1;
    tgtMode = 1;
    useTrueCubic = enableTrueCubic;

    const uint8_t* ptr = data + 4;
    for (uint32_t i = 0; i < count; i++) {
        const int16_t pX = *(const int16_t*)(ptr + 0);
        const int16_t pY = *(const int16_t*)(ptr + 2);
        const int16_t pZ = *(const int16_t*)(ptr + 4);
        const int16_t tX = *(const int16_t*)(ptr + 6);
        const int16_t tY = *(const int16_t*)(ptr + 8);
        const int16_t tZ = *(const int16_t*)(ptr + 10);
        const uint32_t ex = *(const uint32_t*)(ptr + 12);

        posX[i * 4] = pX;
        posY[i * 4] = pY;
        posZ[i * 4] = pZ;
        tgtX[i * 4] = tX;
        tgtY[i * 4] = tY;
        tgtZ[i * 4] = tZ;
        extra[i] = ex;

        ptr += 16;
    }

    loaded = true;
    active = false;
    Log::Printf("CameraBez: loaded %d entries, period=%d, segments=%d",
                totalEntries,
                tickDivision,
                (totalEntries + 2) / 3 - 2);
    return true;
}

void CameraBezPlayer::Start() {
    if (!loaded) {
        return;
    }
    currentFrame = 0;
    active = true;
    outPosX = posX[0];
    outPosY = posY[0];
    outPosZ = posZ[0];
    outTgtX = tgtX[0];
    outTgtY = tgtY[0];
    outTgtZ = tgtZ[0];
    Log::Printf("CameraBez: Start pos=(%d,%d,%d) tgt=(%d,%d,%d)",
                outPosX,
                outPosY,
                outPosZ,
                outTgtX,
                outTgtY,
                outTgtZ);
}

void CameraBezPlayer::ResetLike800128DC() {
    if (!loaded) {
        return;
    }

    currentFrame = 0;
    active = true;
    posMode = 1;
    tgtMode = 1;
    posVel[0] = 0;
    posVel[1] = 0;
    posVel[2] = 0;
    tgtVel[0] = 0;
    tgtVel[1] = 0;
    tgtVel[2] = 0;
    outPosX = posX[0];
    outPosY = posY[0];
    outPosZ = posZ[0];
    outTgtX = tgtX[0];
    outTgtY = tgtY[0];
    outTgtZ = tgtZ[0];
    Log::Printf("CameraBez: 800128DC reset pos=(%d,%d,%d) tgt=(%d,%d,%d)",
                outPosX,
                outPosY,
                outPosZ,
                outTgtX,
                outTgtY,
                outTgtZ);
}

int CameraBezPlayer::DetectMode(const int16_t* chX,
                                const int16_t* chY,
                                const int16_t* chZ,
                                int baseEntry,
                                int nextEntry) {
    const int base = baseEntry * 4;
    const int next = nextEntry * 4;
    const int cp1 = (baseEntry + 1) * 4;
    const int cp2 = (baseEntry + 2) * 4;

    const bool sameXYZ =
        chX[base] == chX[next] &&
        chY[base] == chY[next] &&
        chZ[base] == chZ[next];
    const bool cp1zero = chX[cp1] == 0 && chY[cp1] == 0 && chZ[cp1] == 0;
    const bool cp2zero = chX[cp2] == 0 && chY[cp2] == 0 && chZ[cp2] == 0;

    if (sameXYZ && cp1zero && cp2zero) {
        return 1;
    }
    if (cp1zero || cp2zero) {
        return 2;
    }
    return 0;
}

static int32_t SampleCubicBezierChannel(int16_t p0,
                                        int16_t p1,
                                        int16_t p2,
                                        int16_t p3,
                                        int32_t phase,
                                        int32_t period) {
    if (phase <= 0 || period <= 0) {
        return (int32_t)p0;
    }
    if (phase >= period) {
        return (int32_t)p3;
    }

    static constexpr int64_t kFrac = 4096;
    static constexpr int64_t kFrac3 = kFrac * kFrac * kFrac;

    const int64_t t = ((int64_t)phase * kFrac) / (int64_t)period;
    const int64_t u = kFrac - t;
    const int64_t uu = u * u;
    const int64_t tt = t * t;
    const int64_t w0 = uu * u;
    const int64_t w1 = 3 * uu * t;
    const int64_t w2 = 3 * u * tt;
    const int64_t w3 = tt * t;

    int64_t numer =
        (int64_t)p0 * w0 +
        (int64_t)p1 * w1 +
        (int64_t)p2 * w2 +
        (int64_t)p3 * w3;
    numer += (numer >= 0) ? (kFrac3 / 2) : -(kFrac3 / 2);
    return (int32_t)(numer / kFrac3);
}

static void SampleCubicBezier3(const int16_t* chX,
                               const int16_t* chY,
                               const int16_t* chZ,
                               int baseEntry,
                               int32_t phase,
                               int32_t period,
                               int32_t& outX,
                               int32_t& outY,
                               int32_t& outZ) {
    static constexpr int16_t kStage1PsxBezierBasis11[11][4] = {
        {4096, 0, 0, 0},
        {3077, 923, 92, 3},
        {2243, 1495, 332, 24},
        {1575, 1772, 664, 83},
        {1055, 1809, 1034, 196},
        {664, 1661, 1384, 384},
        {384, 1384, 1661, 664},
        {196, 1034, 1809, 1055},
        {83, 664, 1772, 1575},
        {24, 332, 1495, 2243},
        {3, 92, 923, 3077},
    };

    auto sar12 = [](int64_t value) -> int32_t {
        return value >= 0 ? (int32_t)(value >> 12)
                          : -(int32_t)(((-value) + 4095) >> 12);
    };

    auto samplePsxTableChannel = [&](int16_t p0,
                                     int16_t p1,
                                     int16_t p2,
                                     int16_t p3) -> int32_t {
        const int16_t* w = kStage1PsxBezierBasis11[phase];
        const int32_t mid = sar12((int64_t)p0 * w[0] +
                                  (int64_t)p1 * w[1] +
                                  (int64_t)p2 * w[2]);
        return mid + sar12((int64_t)p3 * w[3]);
    };

    const int base = baseEntry * 4;
    const int cp1 = (baseEntry + 1) * 4;
    const int cp2 = (baseEntry + 2) * 4;
    const int next = (baseEntry + 3) * 4;

    if (period == 11 && phase >= 0 && phase < 11) {
        outX = samplePsxTableChannel(chX[base], chX[cp1], chX[cp2], chX[next]);
        outY = samplePsxTableChannel(chY[base], chY[cp1], chY[cp2], chY[next]);
        outZ = samplePsxTableChannel(chZ[base], chZ[cp1], chZ[cp2], chZ[next]);
        return;
    }

    outX = SampleCubicBezierChannel(chX[base], chX[cp1], chX[cp2], chX[next], phase, period);
    outY = SampleCubicBezierChannel(chY[base], chY[cp1], chY[cp2], chY[next], phase, period);
    outZ = SampleCubicBezierChannel(chZ[base], chZ[cp1], chZ[cp2], chZ[next], phase, period);
}

static void SampleLinearSegment3(const int16_t* chX,
                                 const int16_t* chY,
                                 const int16_t* chZ,
                                 int baseEntry,
                                 int32_t phase,
                                 int32_t period,
                                 int32_t& outX,
                                 int32_t& outY,
                                 int32_t& outZ) {
    const int base = baseEntry * 4;
    const int next = (baseEntry + 3) * 4;
    const float t = (period > 0) ? ((float)phase / (float)period) : 0.0f;
    outX = (int32_t)((1.0f - t) * chX[base] + t * chX[next]);
    outY = (int32_t)((1.0f - t) * chY[base] + t * chY[next]);
    outZ = (int32_t)((1.0f - t) * chZ[base] + t * chZ[next]);
}

static void SampleCubicBezier3Float(const int16_t* chX,
                                    const int16_t* chY,
                                    const int16_t* chZ,
                                    int baseEntry,
                                    float phase,
                                    float period,
                                    float& outX,
                                    float& outY,
                                    float& outZ) {
    const int base = baseEntry * 4;
    const int cp1 = (baseEntry + 1) * 4;
    const int cp2 = (baseEntry + 2) * 4;
    const int next = (baseEntry + 3) * 4;
    if (std::fabs(period - 11.0f) < 0.001f) {
        static constexpr int16_t kStage1PsxBezierBasis11Float[12][4] = {
            {4096, 0, 0, 0},
            {3077, 923, 92, 3},
            {2243, 1495, 332, 24},
            {1575, 1772, 664, 83},
            {1055, 1809, 1034, 196},
            {664, 1661, 1384, 384},
            {384, 1384, 1661, 664},
            {196, 1034, 1809, 1055},
            {83, 664, 1772, 1575},
            {24, 332, 1495, 2243},
            {3, 92, 923, 3077},
            {0, 0, 0, 4096},
        };
        const float clampedPhase = (std::clamp)(phase, 0.0f, 11.0f);
        const int row0 = static_cast<int>(std::floor(clampedPhase));
        const int row1 = (std::min)(row0 + 1, 11);
        const float blend = clampedPhase - static_cast<float>(row0);
        float w[4] = {};
        for (int i = 0; i < 4; ++i) {
            w[i] = (1.0f - blend) *
                       static_cast<float>(kStage1PsxBezierBasis11Float[row0][i]) +
                   blend *
                       static_cast<float>(kStage1PsxBezierBasis11Float[row1][i]);
            w[i] /= 4096.0f;
        }

        outX = chX[base] * w[0] + chX[cp1] * w[1] +
               chX[cp2] * w[2] + chX[next] * w[3];
        outY = chY[base] * w[0] + chY[cp1] * w[1] +
               chY[cp2] * w[2] + chY[next] * w[3];
        outZ = chZ[base] * w[0] + chZ[cp1] * w[1] +
               chZ[cp2] * w[2] + chZ[next] * w[3];
        return;
    }

    const float t =
        (period > 0.0f) ? (std::clamp)(phase / period, 0.0f, 1.0f) : 0.0f;
    const float u = 1.0f - t;
    const float w0 = u * u * u;
    const float w1 = 3.0f * u * u * t;
    const float w2 = 3.0f * u * t * t;
    const float w3 = t * t * t;

    outX = chX[base] * w0 + chX[cp1] * w1 + chX[cp2] * w2 +
           chX[next] * w3;
    outY = chY[base] * w0 + chY[cp1] * w1 + chY[cp2] * w2 +
           chY[next] * w3;
    outZ = chZ[base] * w0 + chZ[cp1] * w1 + chZ[cp2] * w2 +
           chZ[next] * w3;
}

static void SampleLinearSegment3Float(const int16_t* chX,
                                      const int16_t* chY,
                                      const int16_t* chZ,
                                      int baseEntry,
                                      float phase,
                                      float period,
                                      float& outX,
                                      float& outY,
                                      float& outZ) {
    const int base = baseEntry * 4;
    const int next = (baseEntry + 3) * 4;
    const float t =
        (period > 0.0f) ? (std::clamp)(phase / period, 0.0f, 1.0f) : 0.0f;
    outX = (1.0f - t) * chX[base] + t * chX[next];
    outY = (1.0f - t) * chY[base] + t * chY[next];
    outZ = (1.0f - t) * chZ[base] + t * chZ[next];
}

int CameraBezPlayer::Tick(int tick) {
    if (!active || currentFrame < 0) {
        return -1;
    }
    if (tickDivision <= 0) {
        return -1;
    }

    const int phase = tick % tickDivision;

    if (phase == 0) {
        const int maxFrame = (totalEntries + 2) / 3 - 2;
        if (currentFrame >= maxFrame) {
            currentFrame = -1;
            active = false;
            return -1;
        }
        if (tick != 0) {
            currentFrame++;
        }

        const int baseEntry = 3 * currentFrame;
        const int nextEntry = baseEntry + 3;

        posMode = DetectMode(posX, posY, posZ, baseEntry, nextEntry);
        if (posMode == 2) {
            posVel[0] = ((int32_t)posX[nextEntry * 4] - (int32_t)posX[baseEntry * 4]) / tickDivision;
            posVel[1] = ((int32_t)posY[nextEntry * 4] - (int32_t)posY[baseEntry * 4]) / tickDivision;
            posVel[2] = ((int32_t)posZ[nextEntry * 4] - (int32_t)posZ[baseEntry * 4]) / tickDivision;
        }

        tgtMode = DetectMode(tgtX, tgtY, tgtZ, baseEntry, nextEntry);
        if (tgtMode == 2) {
            tgtVel[0] = ((int32_t)tgtX[nextEntry * 4] - (int32_t)tgtX[baseEntry * 4]) / tickDivision;
            tgtVel[1] = ((int32_t)tgtY[nextEntry * 4] - (int32_t)tgtY[baseEntry * 4]) / tickDivision;
            tgtVel[2] = ((int32_t)tgtZ[nextEntry * 4] - (int32_t)tgtZ[baseEntry * 4]) / tickDivision;
        }
    }

    const int baseFrame = 3 * currentFrame;

    switch (posMode) {
        case 1:
            outPosX = posX[baseFrame * 4];
            outPosY = posY[baseFrame * 4];
            outPosZ = posZ[baseFrame * 4];
            break;
        case 2:
            outPosX = posX[baseFrame * 4] + posVel[0] * phase;
            outPosY = posY[baseFrame * 4] + posVel[1] * phase;
            outPosZ = posZ[baseFrame * 4] + posVel[2] * phase;
            break;
        case 0:
            if (useTrueCubic) {
                SampleCubicBezier3(
                    posX, posY, posZ, baseFrame, phase, tickDivision, outPosX, outPosY, outPosZ);
            } else {
                SampleLinearSegment3(
                    posX, posY, posZ, baseFrame, phase, tickDivision, outPosX, outPosY, outPosZ);
            }
            break;
    }

    switch (tgtMode) {
        case 1:
            outTgtX = tgtX[baseFrame * 4];
            outTgtY = tgtY[baseFrame * 4];
            outTgtZ = tgtZ[baseFrame * 4];
            break;
        case 2:
            outTgtX = tgtX[baseFrame * 4] + tgtVel[0] * phase;
            outTgtY = tgtY[baseFrame * 4] + tgtVel[1] * phase;
            outTgtZ = tgtZ[baseFrame * 4] + tgtVel[2] * phase;
            break;
        case 0:
            if (useTrueCubic) {
                SampleCubicBezier3(
                    tgtX, tgtY, tgtZ, baseFrame, phase, tickDivision, outTgtX, outTgtY, outTgtZ);
            } else {
                SampleLinearSegment3(
                    tgtX, tgtY, tgtZ, baseFrame, phase, tickDivision, outTgtX, outTgtY, outTgtZ);
            }
            break;
    }

    return currentFrame;
}

int CameraBezPlayer::SampleAtTick(int tick) {
    if (!active || currentFrame < 0) {
        return -1;
    }
    if (tickDivision <= 0) {
        return -1;
    }
    if (tick < 0) {
        tick = 0;
    }

    const int maxFrame = (totalEntries + 2) / 3 - 2;
    const int targetFrame = tick / tickDivision;
    if (targetFrame >= maxFrame) {
        currentFrame = -1;
        active = false;
        return -1;
    }

    currentFrame = targetFrame;
    const int phase = tick % tickDivision;
    const int baseEntry = 3 * currentFrame;
    const int nextEntry = baseEntry + 3;

    posMode = DetectMode(posX, posY, posZ, baseEntry, nextEntry);
    if (posMode == 2) {
        posVel[0] = ((int32_t)posX[nextEntry * 4] - (int32_t)posX[baseEntry * 4]) / tickDivision;
        posVel[1] = ((int32_t)posY[nextEntry * 4] - (int32_t)posY[baseEntry * 4]) / tickDivision;
        posVel[2] = ((int32_t)posZ[nextEntry * 4] - (int32_t)posZ[baseEntry * 4]) / tickDivision;
    }

    tgtMode = DetectMode(tgtX, tgtY, tgtZ, baseEntry, nextEntry);
    if (tgtMode == 2) {
        tgtVel[0] = ((int32_t)tgtX[nextEntry * 4] - (int32_t)tgtX[baseEntry * 4]) / tickDivision;
        tgtVel[1] = ((int32_t)tgtY[nextEntry * 4] - (int32_t)tgtY[baseEntry * 4]) / tickDivision;
        tgtVel[2] = ((int32_t)tgtZ[nextEntry * 4] - (int32_t)tgtZ[baseEntry * 4]) / tickDivision;
    }

    switch (posMode) {
        case 1:
            outPosX = posX[baseEntry * 4];
            outPosY = posY[baseEntry * 4];
            outPosZ = posZ[baseEntry * 4];
            break;
        case 2:
            outPosX = posX[baseEntry * 4] + posVel[0] * phase;
            outPosY = posY[baseEntry * 4] + posVel[1] * phase;
            outPosZ = posZ[baseEntry * 4] + posVel[2] * phase;
            break;
        case 0:
            if (useTrueCubic) {
                SampleCubicBezier3(
                    posX, posY, posZ, baseEntry, phase, tickDivision, outPosX, outPosY, outPosZ);
            } else {
                SampleLinearSegment3(
                    posX, posY, posZ, baseEntry, phase, tickDivision, outPosX, outPosY, outPosZ);
            }
            break;
    }

    switch (tgtMode) {
        case 1:
            outTgtX = tgtX[baseEntry * 4];
            outTgtY = tgtY[baseEntry * 4];
            outTgtZ = tgtZ[baseEntry * 4];
            break;
        case 2:
            outTgtX = tgtX[baseEntry * 4] + tgtVel[0] * phase;
            outTgtY = tgtY[baseEntry * 4] + tgtVel[1] * phase;
            outTgtZ = tgtZ[baseEntry * 4] + tgtVel[2] * phase;
            break;
        case 0:
            if (useTrueCubic) {
                SampleCubicBezier3(
                    tgtX, tgtY, tgtZ, baseEntry, phase, tickDivision, outTgtX, outTgtY, outTgtZ);
            } else {
                SampleLinearSegment3(
                    tgtX, tgtY, tgtZ, baseEntry, phase, tickDivision, outTgtX, outTgtY, outTgtZ);
            }
            break;
    }

    return currentFrame;
}

bool CameraBezPlayer::SampleFloatAtTick(float tick,
                                        CameraBezFloatSample& out) const {
    if (!loaded || !active || tickDivision <= 0) {
        return false;
    }
    if (tick < 0.0f) {
        tick = 0.0f;
    }

    const int maxFrame = (totalEntries + 2) / 3 - 2;
    const int targetFrame =
        static_cast<int>(std::floor(tick / static_cast<float>(tickDivision)));
    if (targetFrame < 0 || targetFrame >= maxFrame) {
        return false;
    }

    const int baseEntry = 3 * targetFrame;
    const int nextEntry = baseEntry + 3;
    const float phase =
        tick - static_cast<float>(targetFrame * tickDivision);
    const float period = static_cast<float>(tickDivision);

    auto sampleChannel = [&](const int16_t* chX,
                             const int16_t* chY,
                             const int16_t* chZ,
                             bool trueCubic,
                             float& outX,
                             float& outY,
                             float& outZ) {
        const int mode = DetectMode(chX, chY, chZ, baseEntry, nextEntry);
        switch (mode) {
            case 1:
                outX = static_cast<float>(chX[baseEntry * 4]);
                outY = static_cast<float>(chY[baseEntry * 4]);
                outZ = static_cast<float>(chZ[baseEntry * 4]);
                break;
            case 2:
                SampleLinearSegment3Float(
                    chX, chY, chZ, baseEntry, phase, period, outX, outY, outZ);
                break;
            case 0:
            default:
                if (trueCubic) {
                    SampleCubicBezier3Float(chX,
                                            chY,
                                            chZ,
                                            baseEntry,
                                            phase,
                                            period,
                                            outX,
                                            outY,
                                            outZ);
                } else {
                    SampleLinearSegment3Float(chX,
                                              chY,
                                              chZ,
                                              baseEntry,
                                              phase,
                                              period,
                                              outX,
                                              outY,
                                              outZ);
                }
                break;
        }
    };

    sampleChannel(posX,
                  posY,
                  posZ,
                  useTrueCubic,
                  out.posX,
                  out.posY,
                  out.posZ);
    sampleChannel(tgtX,
                  tgtY,
                  tgtZ,
                  useTrueCubic,
                  out.tgtX,
                  out.tgtY,
                  out.tgtZ);
    return true;
}

}  // namespace PrStage1CameraMotionDirect
