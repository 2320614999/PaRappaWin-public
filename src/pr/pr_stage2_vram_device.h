#pragma once
#include "pr_stage2_lifecycle_direct.h"
#include <cstddef>
#include <vector>

namespace PrStage2VramDevice {
// Actual CPU-side VRAM storage for the host image-upload boundary. It does
// not emulate GPU DMA/IRQ queues or present pixels. The scene owner must
// project these words into its existing Windows atlas, without rescanning
// files or replacing native upload order with a filename-based cache.
class Device {
public:
    static constexpr std::size_t WordCount = 1024u*512u;
    Device();
    int32_t UploadImageWords(PrStage2LifecycleDirect::Services& memory,
                      PrStage2LifecycleDirect::ImageRect rect,uint32_t source);
    // Seed an authored Windows TIM asset into the same CPU VRAM model used by
    // native LoadImage. This is a runtime asset path, not a SCUS code/data
    // dependency; later native uploads still override these words in order.
    bool SeedTim(const uint8_t* raw, std::size_t size);
    int32_t DrawSync(uint32_t mode);
    const std::vector<uint16_t>& Words() const { return words_; }
    const std::vector<uint8_t>& Known() const { return known_; }
    uint32_t UploadCount() const { return uploads_; }
    uint32_t SyncCount() const { return syncs_; }
    uint32_t KnownWords() const { return knownWords_; }
    uint64_t WrittenWords() const { return writtenWords_; }
private:
    std::vector<uint16_t> words_;
    std::vector<uint8_t> known_;
    uint32_t uploads_ = 0, syncs_ = 0, knownWords_ = 0;
    uint64_t writtenWords_ = 0;
};
}
