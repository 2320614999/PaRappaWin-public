#pragma once

#include <array>
#include <cstdint>

namespace PrPsxClearImageDirect {

constexpr uint32_t kFn80040420_ClearImage = 0x80040420u;
constexpr uint32_t kFn80044CD0_ClearImage = 0x80044CD0u;

struct PsxClearImageInput80040420 {
    uint16_t word_80096590 = 0;
    std::array<int16_t, 2> word_8008ECA8{};
    std::array<int16_t, 2> word_8008ECAC{};
    uint32_t dword_800917FC = 0;
    uint32_t dword_8009182C = 0;
    uint32_t priorCallCount = 0;
};

struct PsxClearImageResult80040420 {
    bool attempted = false;
    bool sourceKnown = false;
    bool softwareStateCommitted = false;
    bool clearImage80044CD0Called = false;
    bool hostGpuClearSubmitted = false;
    bool exactPsxGpuParity = false;
    uint16_t slot = 0;
    int16_t x = 0;
    int16_t y = 0;
    uint16_t width = 0;
    uint16_t height = 0;
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
    uint32_t callCount = 0;
};

PsxClearImageResult80040420 PsxCall80040420_ClearImage(
    const PsxClearImageInput80040420& input,
    uint32_t r,
    uint32_t g,
    uint32_t b);

} // namespace PrPsxClearImageDirect
