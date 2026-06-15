#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <filesystem>

// CD-XA sector submode flags
constexpr uint8_t CDXA_SUBMODE_EOF      = 0x80;
constexpr uint8_t CDXA_SUBMODE_REALTIME = 0x40;
constexpr uint8_t CDXA_SUBMODE_FORM2    = 0x20;
constexpr uint8_t CDXA_SUBMODE_TRIGGER  = 0x10;
constexpr uint8_t CDXA_SUBMODE_DATA     = 0x08;
constexpr uint8_t CDXA_SUBMODE_AUDIO    = 0x04;
constexpr uint8_t CDXA_SUBMODE_VIDEO    = 0x02;
constexpr uint8_t CDXA_SUBMODE_EOR      = 0x01;

// STR file constants
constexpr size_t STR_FILE_HEADER_SIZE = 44;
constexpr size_t CDXA_SECTOR_SIZE = 2352;
constexpr size_t CDXA_USER_DATA_OFFSET = 0x18;
constexpr size_t CDXA_USER_DATA_SIZE = 2324;
constexpr size_t CDXA_SUBMODE_OFFSET = 0x12;

// Video frame header (within user data)
constexpr size_t STR_VIDEO_HEADER_SIZE = 0x20;

constexpr size_t STR_CHUNK_HEADER_OFFSET = 0x18;
constexpr size_t STR_CHUNK_HEADER_SIZE = 0x20;
constexpr size_t STR_CHUNK_DATA_SIZE = 2016;

struct StrVideoFrameHeader {
    uint32_t frameNo;
    uint32_t frameSize;
    uint16_t width;
    uint16_t height;
    uint16_t blockCount;
    uint16_t unknown0E;
    uint16_t quantScale;
    uint16_t version;
    uint32_t reserved[2];
};

struct StrVideoFrame {
    uint32_t frameNo;
    uint16_t width;
    uint16_t height;
    uint16_t quantScale;   // From sector header offset 0x10
    uint16_t version;      // From sector header offset 0x12
    std::vector<uint8_t> bitstream;  // MDEC bitstream (concatenated sector payloads)
};

struct StrAudioSector {
    uint8_t file;
    uint8_t channel;
    uint8_t coding;
    std::vector<uint8_t> data;  // XA-ADPCM data
};

class StrParser {
public:
    StrParser();
    ~StrParser();

    // Load and parse STR file
    bool Load(const std::filesystem::path& path);
    void Unload();

    // Frame access
    size_t GetFrameCount() const { return m_frames.size(); }
    const StrVideoFrame* GetFrame(size_t index) const;

    // Audio access (for future use)
    size_t GetAudioSectorCount() const { return m_audioSectors.size(); }
    const StrAudioSector* GetAudioSector(size_t index) const;

    // Metadata
    uint16_t GetWidth() const { return m_width; }
    uint16_t GetHeight() const { return m_height; }
    uint32_t GetTotalSectorCount() const { return m_totalSectorCount; }
    uint32_t GetRealtimeSectorCount() const { return m_realtimeSectorCount; }
    double GetDurationSeconds() const;
    bool IsLoaded() const { return m_loaded; }
    const std::string& GetError() const { return m_error; }

private:
    bool ParseSectors(const std::vector<uint8_t>& data);
    void FlushPendingFrame();

    static constexpr uint32_t kNoPendingFrame = 0xFFFFFFFFu;

    std::vector<StrVideoFrame> m_frames;
    std::vector<StrAudioSector> m_audioSectors;

    // Pending frame accumulator
    uint32_t m_pendingFrameNo = kNoPendingFrame;
    uint32_t m_pendingFrameSize = 0;
    uint16_t m_pendingChunkCount = 0;
    uint16_t m_pendingChunksReceived = 0;
    uint16_t m_pendingWidth = 0;
    uint16_t m_pendingHeight = 0;
    uint16_t m_pendingQuantScale = 0;
    uint16_t m_pendingVersion = 0;
    std::vector<uint8_t> m_pendingBitstream;
    std::vector<uint8_t> m_pendingChunkPresent;

    uint16_t m_width = 0;
    uint16_t m_height = 0;
    uint32_t m_totalSectorCount = 0;
    uint32_t m_realtimeSectorCount = 0;
    bool m_loaded = false;
    std::string m_error;
};
