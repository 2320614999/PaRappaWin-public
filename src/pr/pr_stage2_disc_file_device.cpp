#include "pr_stage2_disc_file_device.h"
#include "pr_stage1_loader_cd_hal.h"

#include <limits>
#include <string>
#include <utility>

namespace PrStage2DiscFileDevice {
namespace {
bool ValidBcd(uint32_t value) { return (value&15u) < 10u && (value>>4u) < 10u; }
uint32_t DecodeBcd(uint32_t value) { return 10u*(value>>4u)+(value&15u); }
}
Device::Device(std::filesystem::path disc) : disc_(std::move(disc)) {}

int32_t Device::Lookup(PrStage2LifecycleDirect::Services& memory, uint32_t descriptor, uint32_t path) {
    ++lookupRequests_;
    if (!path || !descriptor) return 0;
    std::string name;
    for (uint32_t address = path;; ++address) {
        const uint8_t byte = memory.Read8(address);
        if (!byte) break;
        name.push_back(static_cast<char>(byte));
    }
    // Original game paths are absolute ISO paths; don't accept a host path.
    if (name.empty() || name.front() != '\\') return 0;
    PrStage1LoaderCdHal::Iso9660LookupInput800381F8 input{};
    input.valid = true; input.binPath = disc_; input.psxPath = name.c_str();
    input.cdlFilePtr = descriptor; input.pathPtr = path;
    const auto found = PrStage1LoaderCdHal::BuildIso9660LookupFeedback800381F8(input);
    if (!found.success) return 0;
    const auto& file = found.feedback;
    // Only defined CdlFILE fields are owned here. The location pad byte is
    // opaque; no invented track number or synthetic PSX cache address.
    memory.Write8(descriptor,file.cdlFilePos.minute);
    memory.Write8(descriptor+1u,file.cdlFilePos.second);
    memory.Write8(descriptor+2u,file.cdlFilePos.frame);
    memory.Write32(descriptor+4u,file.cdlFileSize);
    for (uint32_t i = 0; i < file.cdlFileName.size(); ++i)
        memory.Write8(descriptor+8u+i,file.cdlFileName[i]);
    // The direct A2B0 caller only tests nonzero and returns its own descriptor.
    return 1;
}

int32_t Device::SetLocation(uint32_t packedBcd) {
    const uint32_t minute = packedBcd&255u, second = (packedBcd>>8u)&255u, frame = (packedBcd>>16u)&255u;
    if (!ValidBcd(minute) || !ValidBcd(second) || !ValidBcd(frame) ||
        DecodeBcd(second) >= 60u || DecodeBcd(frame) >= 75u) return 0;
    const int64_t lba = int64_t(DecodeBcd(minute))*4500+DecodeBcd(second)*75+DecodeBcd(frame)-150;
    std::error_code error;
    const uintmax_t size = std::filesystem::file_size(disc_,error);
    if (error || lba < 0 || uint64_t(lba) >= size/2352u) return 0;
    head_ = lba;
    return 1;
}

int32_t Device::StartRead(PrStage2LifecycleDirect::Services& memory, uint32_t sectors,
                          uint32_t destination, uint32_t mode) {
    ++readRequests_;
    status_ = -1;
    const uint64_t bytes = uint64_t(sectors)*2048u;
    // A bounded host transfer can only address the original 2 MiB RAM.
    // Reject invalid command arguments. An accepted filesystem transaction
    // reports actual I/O failure through PollRead and never fabricates bytes.
    if (head_ < 0 || (mode != 0u && mode != 128u) || bytes == 0u || bytes > 0x200000u ||
        destination < 0x80000000u || uint64_t(destination)+bytes > 0x80200000ull ||
        head_ > std::numeric_limits<int32_t>::max()) return 0;
    PrStage1LoaderCdHal::Iso9660UserDataReadInput8001A818 input{};
    input.valid = true; input.binPath = disc_; input.lba = static_cast<int32_t>(head_);
    input.byteCount = static_cast<uint32_t>(bytes);
    const auto read = PrStage1LoaderCdHal::ReadIso9660UserDataBytes8001A818(input);
    if (!read.success || read.bytes.size() != bytes) return 1;
    for (uint32_t i = 0; i < read.bytes.size(); i += 4u) {
        const uint32_t word = uint32_t(read.bytes[i]) | (uint32_t(read.bytes[i+1u])<<8u)
                            | (uint32_t(read.bytes[i+2u])<<16u) | (uint32_t(read.bytes[i+3u])<<24u);
        memory.Write32(destination+i,word);
    }
    head_ += sectors;
    bytesTransferred_ += bytes;
    status_ = 0;
    return 1;
}
int32_t Device::PollRead() const { return status_; }
}
