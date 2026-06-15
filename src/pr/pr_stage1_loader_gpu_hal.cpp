#include "pr_stage1_loader_gpu_hal.h"

#include <cstring>

namespace PrStage1LoaderGpuHal {

namespace {

constexpr uint32_t kTimMagic = 0x10u;
constexpr size_t kTimHeaderBytes = 8u;
constexpr size_t kFlagsBytes = 4u;
constexpr size_t kBlockHeaderBytes = 12u;

uint32_t ReadU32LE(const uint8_t* data) {
    uint32_t out = 0u;
    std::memcpy(&out, data, sizeof(out));
    return out;
}

int16_t ReadI16LE(const uint8_t* data) {
    uint16_t out = 0u;
    std::memcpy(&out, data, sizeof(out));
    return static_cast<int16_t>(out);
}

PsxRect ReadRectLE(const uint8_t* data) {
    PsxRect out{};
    out.x = ReadI16LE(data + 0);
    out.y = ReadI16LE(data + 2);
    out.w = ReadI16LE(data + 4);
    out.h = ReadI16LE(data + 6);
    return out;
}

TimDataView MakeDataView(const TimPayloadView& timPayload,
                         size_t sourceOffset,
                         size_t outputOffset,
                         size_t size) {
    TimDataView out{};
    out.offset = outputOffset;
    out.size = size;
    if (timPayload.data != nullptr && sourceOffset <= timPayload.size) {
        out.data = timPayload.data + sourceOffset;
    }
    if (timPayload.psxAddressKnown) {
        out.psxAddressKnown = true;
        out.psxAddress =
            timPayload.psxAddress + static_cast<uint32_t>(sourceOffset);
    }
    return out;
}

bool AppendAction(GpuActionList8001AE7C& list,
                  const GpuAction8001AE7C& action) {
    if (list.count >= list.actions.size()) {
        list.overflow = true;
        return false;
    }

    GpuAction8001AE7C out = action;
    out.sequenceIndex = static_cast<uint32_t>(list.count);
    list.actions[list.count] = out;
    ++list.count;
    return true;
}

TimParseResult ParseGsGetTimInfoFlagsPayload80040EAC(
    const TimPayloadView& timInfoPayload,
    size_t outputOffsetBase) {
    TimParseResult out{};
    if (timInfoPayload.data == nullptr) {
        out.status = TimParseStatus::MissingPayload;
        return out;
    }
    if (timInfoPayload.size < kFlagsBytes) {
        out.status = TimParseStatus::TooSmallForFlagsPayload;
        return out;
    }

    GsImageTimInfo80040EAC info{};
    info.flags = ReadU32LE(timInfoPayload.data);
    info.pixelMode = info.flags & 0x7u;
    info.hasClut =
        ((info.flags >> kTimClutFlagShift8001AE7C) &
         kTimClutFlagMask8001AE7C) != 0u;

    if (info.pixelMode > 3u) {
        out.status = TimParseStatus::UnsupportedPixelMode;
        return out;
    }

    size_t cursor = kFlagsBytes;
    if (info.hasClut) {
        const size_t clutBlockOffsetInFlagsPayload = cursor;
        if (timInfoPayload.size < cursor + kBlockHeaderBytes) {
            out.status = TimParseStatus::TruncatedClutBlockHeader;
            return out;
        }

        const uint32_t clutBlockLength =
            ReadU32LE(timInfoPayload.data + cursor);
        if (clutBlockLength < kBlockHeaderBytes ||
            timInfoPayload.size <
                clutBlockOffsetInFlagsPayload + clutBlockLength) {
            out.status = TimParseStatus::InvalidClutBlockLength;
            return out;
        }

        info.clutRect =
            ReadRectLE(timInfoPayload.data + cursor + sizeof(uint32_t));
        info.clutData = MakeDataView(
            timInfoPayload,
            cursor + kBlockHeaderBytes,
            outputOffsetBase + cursor + kBlockHeaderBytes,
            clutBlockLength - kBlockHeaderBytes);
        cursor = clutBlockOffsetInFlagsPayload + clutBlockLength;
    }

    const size_t imageBlockOffsetInFlagsPayload = cursor;
    if (timInfoPayload.size < cursor + kBlockHeaderBytes) {
        out.status = TimParseStatus::TruncatedImageBlockHeader;
        return out;
    }

    const uint32_t imageBlockLength =
        ReadU32LE(timInfoPayload.data + cursor);
    if (imageBlockLength < kBlockHeaderBytes ||
        timInfoPayload.size < imageBlockOffsetInFlagsPayload + imageBlockLength) {
        out.status = TimParseStatus::InvalidImageBlockLength;
        return out;
    }

    info.pixelRect =
        ReadRectLE(timInfoPayload.data + cursor + sizeof(uint32_t));
    info.pixelData = MakeDataView(
        timInfoPayload,
        cursor + kBlockHeaderBytes,
        outputOffsetBase + cursor + kBlockHeaderBytes,
        imageBlockLength - kBlockHeaderBytes);
    info.valid = true;

    out.status = TimParseStatus::Ok;
    out.info = info;
    return out;
}

}  // namespace

TimParseResult ParseGsGetTimInfo80040EAC(const TimPayloadView& timPayload) {
    return ParseGsGetTimInfoFlagsPayload80040EAC(timPayload, 0u);
}

GpuActionList8001AE7C BuildGpuActions8001AE7C(
    const TimPayloadView& timPayload) {
    GpuActionList8001AE7C out{};

    GpuAction8001AE7C timInfoAction{};
    timInfoAction.kind = GpuActionKind::GsGetTimInfo;
    timInfoAction.psxFunction = kFnGsGetTimInfo80040EAC;
    timInfoAction.timInfoInputOffset = kTimInfoPayloadOffset8001AE7C;
    AppendAction(out, timInfoAction);

    if (timPayload.data == nullptr) {
        out.timParse.status = TimParseStatus::MissingPayload;
        return out;
    }
    if (timPayload.size < kTimHeaderBytes) {
        out.timParse.status = TimParseStatus::TooSmallForTimHeader;
        return out;
    }
    if (ReadU32LE(timPayload.data) != kTimMagic) {
        out.timParse.status = TimParseStatus::BadTimMagic;
        return out;
    }

    TimPayloadView timInfoPayload{};
    timInfoPayload.data = timPayload.data + kTimInfoPayloadOffset8001AE7C;
    timInfoPayload.size = timPayload.size - kTimInfoPayloadOffset8001AE7C;
    if (timPayload.psxAddressKnown) {
        timInfoPayload.psxAddressKnown = true;
        timInfoPayload.psxAddress =
            timPayload.psxAddress +
            static_cast<uint32_t>(kTimInfoPayloadOffset8001AE7C);
    }

    out.timParse = ParseGsGetTimInfoFlagsPayload80040EAC(
        timInfoPayload, kTimInfoPayloadOffset8001AE7C);
    if (out.timParse.status != TimParseStatus::Ok) {
        return out;
    }

    const GsImageTimInfo80040EAC& info = out.timParse.info;

    GpuAction8001AE7C pixelUpload{};
    pixelUpload.kind = GpuActionKind::LoadImage;
    pixelUpload.psxFunction = kFnLoadImage80044D64;
    pixelUpload.lowerPsxFunction = kFnLowerLoadImage800468E0;
    pixelUpload.rect = info.pixelRect;
    pixelUpload.payload = info.pixelData;
    pixelUpload.uploadSection = GpuUploadSection::Pixel;
    if (AppendAction(out, pixelUpload)) {
        out.pixelUploadQueued = true;
    }

    GpuAction8001AE7C firstSync{};
    firstSync.kind = GpuActionKind::DrawSync;
    firstSync.psxFunction = kFnDrawSync80044B3C;
    firstSync.lowerPsxFunction = kFnLowerDrawSync80046FFC;
    firstSync.drawSyncMode = kDrawSyncMode8001AE7C;
    if (AppendAction(out, firstSync)) {
        out.firstDrawSyncQueued = true;
    }

    if (info.hasClut) {
        GpuAction8001AE7C clutUpload{};
        clutUpload.kind = GpuActionKind::LoadImage;
        clutUpload.psxFunction = kFnLoadImage80044D64;
        clutUpload.lowerPsxFunction = kFnLowerLoadImage800468E0;
        clutUpload.rect = info.clutRect;
        clutUpload.payload = info.clutData;
        clutUpload.uploadSection = GpuUploadSection::Clut;
        if (AppendAction(out, clutUpload)) {
            out.clutUploadQueued = true;
        }

        GpuAction8001AE7C secondSync{};
        secondSync.kind = GpuActionKind::DrawSync;
        secondSync.psxFunction = kFnDrawSync80044B3C;
        secondSync.lowerPsxFunction = kFnLowerDrawSync80046FFC;
        secondSync.drawSyncMode = kDrawSyncMode8001AE7C;
        if (AppendAction(out, secondSync)) {
            out.secondDrawSyncQueued = true;
        }
    }

    return out;
}

}  // namespace PrStage1LoaderGpuHal
