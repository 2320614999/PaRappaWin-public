#pragma once
#include "pr_stage2_spu_device.h"
#include "pr_stage2_cd_command_device.h"
#include <array>
#include <cstdint>
#include <vector>

namespace PrStage2XaDevice {
// Concrete XA data decoder, not a game-function substitute. State persists
// between selected sectors. Initial predictor/FIR state and six-step phase
// are explicit native attachment choices; no cycle/analogue equivalence claim.
class Decoder {
public:
    std::vector<int16_t> Decode(const std::array<uint8_t,2352>& raw);
private:
    std::array<int32_t,2> previous_{},older_{};
    std::array<std::array<int16_t,32>,2> history_{};
    uint32_t position_=0,six_=6;
    int32_t coding_=-1;
    int16_t Sample(const uint8_t* group,uint32_t block,uint32_t nibble,uint32_t sample,uint32_t channel);
    void Resample(int16_t left,int16_t right,std::vector<int16_t>& output);
};
class Device final:public PrStage2CdCommandDevice::XaConsumer {
public:
    explicit Device(PrStage2SpuDevice::Device& spu):spu_(spu){}
    bool Consume(const std::array<uint8_t,2352>& raw,uint32_t lba,bool muted,
                 const std::array<uint8_t,4>& matrix) override;
    uint64_t Sectors()const noexcept{return sectors_;}
    uint64_t Frames()const noexcept{return frames_;}
    uint32_t LastLba()const noexcept{return lastLba_;}
    const std::vector<int16_t>& LastPcm()const noexcept{return lastPcm_;}
private:
    PrStage2SpuDevice::Device& spu_;
    Decoder decoder_;
    std::vector<int16_t> lastPcm_;
    uint64_t sectors_=0,frames_=0;
    uint32_t lastLba_=0;
};
}
