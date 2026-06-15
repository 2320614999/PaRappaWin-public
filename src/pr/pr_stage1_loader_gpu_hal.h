#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace PrStage1LoaderGpuHal {

constexpr uint32_t kFn8001AE7C = 0x8001AE7Cu;
constexpr uint32_t kFnGsGetTimInfo80040EAC = 0x80040EACu;
constexpr uint32_t kFnLoadImage80044D64 = 0x80044D64u;
constexpr uint32_t kFnDrawSync80044B3C = 0x80044B3Cu;
constexpr uint32_t kFnLowerLoadImage800468E0 = 0x800468E0u;
constexpr uint32_t kFnLowerDrawSync80046FFC = 0x80046FFCu;

constexpr size_t kTimInfoPayloadOffset8001AE7C = 4u;
constexpr uint8_t kTimClutFlagShift8001AE7C = 3u;
constexpr uint32_t kTimClutFlagMask8001AE7C = 1u;
constexpr int32_t kDrawSyncMode8001AE7C = 0;
constexpr size_t kMaxGpuActions8001AE7C = 5u;

struct PsxRect {
    int16_t x = 0;
    int16_t y = 0;
    int16_t w = 0;
    int16_t h = 0;
};

struct TimPayloadView {
    const uint8_t* data = nullptr;
    size_t size = 0u;
    uint32_t psxAddress = 0u;
    bool psxAddressKnown = false;
};

struct TimDataView {
    const uint8_t* data = nullptr;
    size_t size = 0u;
    size_t offset = 0u;
    uint32_t psxAddress = 0u;
    bool psxAddressKnown = false;
};

struct GsImageTimInfo80040EAC {
    bool valid = false;
    uint32_t flags = 0u;
    uint32_t pixelMode = 0u;
    bool hasClut = false;

    PsxRect pixelRect{};
    TimDataView pixelData{};

    PsxRect clutRect{};
    TimDataView clutData{};
};

enum class TimParseStatus : uint8_t {
    Ok = 0u,
    MissingPayload,
    TooSmallForTimHeader,
    BadTimMagic,
    TooSmallForFlagsPayload,
    UnsupportedPixelMode,
    TruncatedClutBlockHeader,
    InvalidClutBlockLength,
    TruncatedImageBlockHeader,
    InvalidImageBlockLength,
};

struct TimParseResult {
    TimParseStatus status = TimParseStatus::MissingPayload;
    GsImageTimInfo80040EAC info{};
};

enum class GpuActionKind : uint8_t {
    None = 0u,
    GsGetTimInfo,
    LoadImage,
    DrawSync,
};

enum class GpuUploadSection : uint8_t {
    None = 0u,
    Pixel,
    Clut,
};

struct GpuAction8001AE7C {
    GpuActionKind kind = GpuActionKind::None;
    uint32_t sequenceIndex = 0u;
    uint32_t directFunction = kFn8001AE7C;
    uint32_t psxFunction = 0u;
    uint32_t lowerPsxFunction = 0u;

    size_t timInfoInputOffset = 0u;
    PsxRect rect{};
    TimDataView payload{};
    GpuUploadSection uploadSection = GpuUploadSection::None;
    int32_t drawSyncMode = 0;
};

struct GpuActionList8001AE7C {
    std::array<GpuAction8001AE7C, kMaxGpuActions8001AE7C> actions{};
    size_t count = 0u;
    bool overflow = false;

    TimParseResult timParse{};
    bool pixelUploadQueued = false;
    bool clutUploadQueued = false;
    bool firstDrawSyncQueued = false;
    bool secondDrawSyncQueued = false;
};

// Mirrors GsGetTimInfo's ABI: data points at the TIM flags dword, i.e.
// 8001AE7C passes the caller TIM payload as tim + 4.
TimParseResult ParseGsGetTimInfo80040EAC(
    const TimPayloadView& timInfoPayload);
GpuActionList8001AE7C BuildGpuActions8001AE7C(
    const TimPayloadView& timPayload);

}  // namespace PrStage1LoaderGpuHal
