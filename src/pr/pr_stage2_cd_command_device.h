#pragma once
#include "pr_stage2_interrupt_device.h"
#include <array>
#include <chrono>
#include <cstdint>
#include <deque>
#include <filesystem>
#include <fstream>
#include <functional>
#include <thread>
#include <vector>

namespace PrStage2CdCommandDevice {
struct XaConsumer {
    virtual ~XaConsumer()=default;
    // false is bounded output backpressure, not a successful audio receipt.
    virtual bool Consume(const std::array<uint8_t,2352>&,uint32_t lba,bool muted,
                         const std::array<uint8_t,4>& matrix)=0;
};
// Read-only optical-image device. Source RAM is never owned or changed here.
// Writes enqueue commands; Service performs actual effects/response delivery.
// Register observations and acknowledgments never execute queued commands.
class Device final {
public:
    using Clock=std::chrono::steady_clock;
    Device(const std::filesystem::path& disc,PrStage2InterruptDevice::Controller& irq,
           std::function<Clock::time_point()> now={});
    // The product retains mechanical startup waits; isolated command/DMA
    // contracts may use the transport alone. Select before issuing commands.
    void EnableMechanicalTiming();
    // Optional host startup backpressure. Commands still complete, but no
    // first sector, playing status or audio is published until release.
    void HoldNextStreamStart();
    void ReleaseStreamStart();
    bool TryRead(uint32_t address,uint32_t width,uint32_t& value);
    bool TryWrite(uint32_t address,uint32_t width,uint32_t value);
    // The native scene owner controls when another optical read may begin.
    // Existing command replies and already-owned audio work still progress
    // while a retained source callback is finishing.
    bool Service(bool allowNewSector=true);
    void BindXaConsumer(XaConsumer& consumer);
    // Native file-producer backpressure, not a drive-register model. A real
    // sector may be read and retained before its source receiver has room.
    // The predicate observes receiver storage only; no readiness is invented.
    void BindVideoReadiness(std::function<bool(const std::array<uint8_t,2352>&)> ready);
    bool VideoDeliveryPending()const noexcept{return videoPending_;}
    uint64_t VideoDeferrals()const noexcept{return videoDeferrals_;}
    uint64_t UnrequestedSectorsReplaced()const noexcept{return unrequestedSectorsReplaced_;}
    uint64_t AudioSectors()const noexcept{return audioSectors_;}
    uint64_t FilteredAudioSectors()const noexcept{return filteredAudioSectors_;}
    // Explicit transfer-owner interface. No sector is released merely by a
    // status read; ordinary FIFO overread behavior remains separately defined.
    size_t DmaBytesAvailable() const;
    uint32_t ReadDmaWord();
    void ReleaseConsumedSector();
    uint64_t Commands()const noexcept{return commands_;}
    uint64_t Executed()const noexcept{return executed_;}
    uint64_t Responses()const noexcept{return responses_;}
    uint64_t Acknowledgments()const noexcept{return acknowledgments_;}
    uint64_t SectorReads()const noexcept{return sectorReads_;}
    uint64_t Serial()const noexcept{return serial_;}
    uint32_t Location()const noexcept{return location_;}
    uint32_t LastSector()const noexcept{return lastSector_;}
    uint8_t Mode()const noexcept{return mode_;}
    uint8_t FilterFile()const noexcept{return filterFile_;}
    uint8_t FilterChannel()const noexcept{return filterChannel_;}
    uint8_t Flags()const noexcept{return flags_;}
    uint8_t Mask()const noexcept{return mask_;}
    bool Reading()const noexcept{return reading_;}
    bool Pending()const noexcept{return pending_||!queued_.empty()||flags_;}
    const std::array<uint8_t,2352>& RawSector()const noexcept{return rawSector_;}
    const std::vector<uint8_t>& CommandTrace()const noexcept{return commandTrace_;}
private:
    struct Reply{uint8_t kind;std::vector<uint8_t> bytes;};
    PrStage2InterruptDevice::Controller& irq_;
    std::ifstream file_;
    uint64_t sectors_=0;
    static constexpr std::size_t ReadAheadSectors=256u;
    std::vector<uint8_t> readAhead_;
    uint64_t readAheadStart_=0;
    std::size_t readAheadCount_=0;
    std::thread::id owner_=std::this_thread::get_id();
    std::function<Clock::time_point()> now_;
    bool mechanicalTiming_=false,readStarting_=false;
    bool streamStartHeld_=false;
    Clock::time_point speedReady_{};
    std::chrono::microseconds ReadSetupTime()const;
    uint8_t bank_=0,mask_=0,flags_=0,mode_=0,driveStatus_=0;
    uint8_t filterFile_=0,filterChannel_=0,request_=0,command_=0;
    uint32_t delay_=0,location_=0,lastSector_=0;
    std::vector<uint8_t> parameters_,latchedParameters_,commandTrace_;
    std::deque<Reply> queued_;
    std::array<uint8_t,16> result_{};
    std::array<uint8_t,2352> rawSector_{};
    size_t resultSize_=0,resultPosition_=0,dataPosition_=0,dataSize_=0,dataOffset_=0;
    bool pending_=false,locationKnown_=false,reading_=false,muted_=false,rawKnown_=false;
    XaConsumer* xa_=nullptr;
    std::function<bool(const std::array<uint8_t,2352>&)> videoReady_;
    bool videoPending_=false;
    uint64_t videoDeferrals_=0;
    uint64_t unrequestedSectorsReplaced_=0;
    std::array<uint8_t,4> matrix_{128u,0u,128u,0u},pendingMatrix_=matrix_;
    bool audioPending_=false,adpcmMuted_=false;
    uint64_t audioSectors_=0,filteredAudioSectors_=0;
    std::chrono::steady_clock::time_point nextSector_{};
    uint64_t commands_=0,executed_=0,responses_=0,acknowledgments_=0,sectorReads_=0,serial_=0;
    static uint32_t Register(uint32_t address,uint32_t width);
    void Owner()const;
    void Irq();
    void ReplyStatus(uint8_t kind,uint8_t status);
    void Error(uint8_t detail);
    void ReadSector(uint64_t sector);
    void Execute();
    bool Publish();
    bool PublishVideo();
};
}
