#pragma once

#include "pr_stage2_lifecycle_direct.h"
#include <filesystem>

namespace PrStage2DiscFileDevice {

// Windows read-only storage boundary, not a translation of the CD controller
// or its directory-cache internals. It never acknowledges missing file data.
// VBlank, TIM and SPU ownership remain with the caller's actual device backend.
class Device {
public:
    explicit Device(std::filesystem::path disc);
    int32_t Lookup(PrStage2LifecycleDirect::Services& memory, uint32_t descriptor, uint32_t path);
    int32_t SetLocation(uint32_t packedBcd);
    int32_t StartRead(PrStage2LifecycleDirect::Services& memory, uint32_t sectors,
                      uint32_t destination, uint32_t mode);
    int32_t PollRead() const;
    uint32_t ReadRequests() const { return readRequests_; }
    uint32_t LookupRequests() const { return lookupRequests_; }
    uint64_t BytesTransferred() const { return bytesTransferred_; }
private:
    std::filesystem::path disc_;
    int64_t head_ = -1;
    int32_t status_ = -1;
    uint32_t readRequests_ = 0, lookupRequests_ = 0;
    uint64_t bytesTransferred_ = 0;
};
}
