#pragma once

#include <array>
#include <cstdint>
#include <cstddef>
#include <deque>
#include <filesystem>
#include <fstream>
#include <vector>

#include "xa_decoder.h"

struct XaRawSectorView {
    uint32_t sectorIndex = 0;
    uint8_t file = 0;
    uint8_t channel = 0;
    uint8_t submode = 0;
    uint8_t coding = 0;
    bool realtimeAudio = false;
    bool eof = false;

    const uint8_t* sector2352 = nullptr;
    size_t sectorSize = 0;

    const uint8_t* userData = nullptr;
    size_t userDataSize = 0;
};

struct Xa1RingPacketView {
    uint32_t sectorIndex = 0;
    uint8_t file = 0;
    uint8_t channel = 0;
    uint8_t coding = 0;
    bool cdGetlocPResponseKnown = false;
    std::array<uint8_t, 8> cdGetlocPResponse{};
    bool cdDataReadyInterruptKnown = false;
    uint8_t cdDataReadyInterrupt = 0;

    const uint8_t* header32 = nullptr;
    size_t headerSize = 0;

    const uint8_t* payload2016 = nullptr;
    size_t payloadSize = 0;
};

class Xa1Player {
public:
    Xa1Player();
    ~Xa1Player();

    bool Play(const std::filesystem::path& path);
    void Stop();
    void Update();
    bool SetFilter(uint8_t file, uint8_t channel);
    void SetVolume(float volume);
    void NotifyAudioEngineReset();

    bool IsPlaying() const;
    bool IsFinished() const;

    double GetPlayedSeconds() const;
    double GetEstimatedDurationSeconds() const;
    uint32_t GetCurrentSectorIndex() const { return m_sectorIndex; }
    int GetCdClockSectorIndex() const;
    uint32_t GetTotalSectors() const { return m_totalSectors; }
    double GetQueuedSeconds() const;
    int GetEstimatedAudibleSectorIndex() const;
    size_t GetAcceptedRawSectorQueueSize() const { return m_acceptedRawSectors.size(); }
    bool GetSelectedFilterKnown() const { return m_selValid; }
    uint8_t GetSelectedFile() const { return m_selFile; }
    uint8_t GetSelectedChannel() const { return m_selChannel; }
    bool GetSelectedCodingKnown() const { return m_selCodingValid; }
    uint8_t GetSelectedCoding() const { return m_selCoding; }
    uint32_t GetAcceptedRawPushCount() const { return m_acceptedRawPushCount; }
    bool GetLastAcceptedSectorKnown() const { return m_lastAcceptedSectorKnown; }
    uint32_t GetLastAcceptedSectorIndex() const { return m_lastAcceptedSectorIndex; }
    uint8_t GetLastAcceptedFile() const { return m_lastAcceptedFile; }
    uint8_t GetLastAcceptedChannel() const { return m_lastAcceptedChannel; }
    uint8_t GetLastAcceptedCoding() const { return m_lastAcceptedCoding; }
    uint32_t GetSetFilterChangeCount() const { return m_setFilterChangeCount; }
    bool GetLastSetFilterKnown() const { return m_lastSetFilterKnown; }
    uint32_t GetLastSetFilterSectorIndex() const { return m_lastSetFilterSectorIndex; }
    uint8_t GetLastSetFilterFile() const { return m_lastSetFilterFile; }
    uint8_t GetLastSetFilterChannel() const { return m_lastSetFilterChannel; }
    bool GetLastCdClockGetlocPResponse(std::array<uint8_t, 8>& out,
                                        uint32_t* outSectorIndex = nullptr) const;
    bool PollAcceptedRawSector(XaRawSectorView& out);
    bool PollAcceptedRingPacket(Xa1RingPacketView& out);

private:
    enum class State : uint8_t {
        Idle,
        Playing,
        Finished,
        Error
    };

    static constexpr size_t kSectorSize = 2352;
    static constexpr size_t kSubmodeOffset = 0x12;
    static constexpr size_t kUserDataOffset = 0x18;
    static constexpr size_t kUserDataSize = 2324;
    static constexpr size_t kRingHeaderSize = 32;
    static constexpr size_t kRingPayloadSize = 2016;
    static constexpr size_t kAcceptedRawSectorQueueLimit = 64;

    struct StoredRawSector {
        uint32_t sectorIndex = 0;
        std::array<uint8_t, kSectorSize> sector{};
    };

    static uint32_t ReadU32LE(const uint8_t* p);
    bool ParseRiffCdxa(std::ifstream& f, uint64_t& outDataOffset, uint32_t& outDataSize);

    bool ReadNextSector(std::vector<uint8_t>& outSector);
    bool TryConsumeSector(const uint8_t* sector, size_t size, uint32_t sectorIndex);
    void PumpQueuedSectors();
    void ApplyAudioQueueGovernor();
    static bool FillRawSectorView(const StoredRawSector& stored, XaRawSectorView& out);
    void PushAcceptedRawSector(const uint8_t* sector, size_t size, uint32_t sectorIndex);

    static constexpr uint8_t kSubmodeEof = 0x80;
    static constexpr uint8_t kSubmodeRealtime = 0x40;
    static constexpr uint8_t kSubmodeAudio = 0x04;

    State m_state = State::Idle;
    std::filesystem::path m_path{};
    std::ifstream m_file;

    uint64_t m_dataOffset = 0;
    uint32_t m_dataSize = 0;
    uint32_t m_totalSectors = 0;
    uint32_t m_sectorIndex = 0;
    double m_cdClockSector75 = 0.0;

    bool m_audioInited = false;
    bool m_selValid = false;
    bool m_selCodingValid = false;
    uint8_t m_selFile = 0;
    uint8_t m_selChannel = 0;
    uint8_t m_selCoding = 0;

    uint32_t m_sampleRate = 37800;
    bool m_stereo = false;
    float m_volume = 1.0f;

    XaDecoder m_decoder;
    int m_audioVoice = -1;
    double m_lastDecodedSectorSeconds = 0.0;
    double m_audioPlaybackRateScale = 1.0;
    std::vector<uint8_t> m_sectorBuf;
    std::deque<StoredRawSector> m_acceptedRawSectors;
    StoredRawSector m_polledRawSector;
    bool m_polledRawSectorValid = false;
    uint32_t m_acceptedRawPushCount = 0;
    bool m_lastAcceptedSectorKnown = false;
    uint32_t m_lastAcceptedSectorIndex = 0;
    uint8_t m_lastAcceptedFile = 0;
    uint8_t m_lastAcceptedChannel = 0;
    uint8_t m_lastAcceptedCoding = 0;
    uint32_t m_setFilterChangeCount = 0;
    bool m_lastSetFilterKnown = false;
    uint32_t m_lastSetFilterSectorIndex = 0;
    uint8_t m_lastSetFilterFile = 0;
    uint8_t m_lastSetFilterChannel = 0;
    bool m_lastCdClockGetlocPResponseKnown = false;
    uint32_t m_lastCdClockGetlocPSectorIndex = 0;
    std::array<uint8_t, 8> m_lastCdClockGetlocPResponse{};
};
