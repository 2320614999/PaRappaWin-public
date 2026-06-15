#include "str_parser.h"
#include "logger.h"
#include <fstream>
#include <cstring>
 #include <algorithm>

StrParser::StrParser() = default;
StrParser::~StrParser() { Unload(); }

bool StrParser::Load(const std::filesystem::path& path) {
    Unload();

    // Read entire file
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        m_error = "Failed to open file";
        Log::Printf("StrParser: %s: %s", m_error.c_str(), path.u8string().c_str());
        return false;
    }

    const auto fileSize = file.tellg();
    if (fileSize < (std::streamoff)(STR_FILE_HEADER_SIZE + CDXA_SECTOR_SIZE)) {
        m_error = "File too small";
        Log::Printf("StrParser: %s", m_error.c_str());
        return false;
    }

    file.seekg(0, std::ios::beg);
    std::vector<uint8_t> data((size_t)fileSize);
    file.read(reinterpret_cast<char*>(data.data()), fileSize);
    file.close();

    Log::Printf("StrParser: Loading %s (%llu bytes)",
                path.filename().u8string().c_str(),
                (unsigned long long)fileSize);

    if (!ParseSectors(data)) {
        return false;
    }

    m_loaded = true;
    Log::Printf("StrParser: Loaded %zu video frames, %zu audio sectors, resolution %ux%u",
                m_frames.size(), m_audioSectors.size(), m_width, m_height);
    return true;
}

void StrParser::Unload() {
    m_frames.clear();
    m_audioSectors.clear();
    m_pendingBitstream.clear();
    m_pendingFrameNo = kNoPendingFrame;
    m_pendingFrameSize = 0;
    m_pendingChunkCount = 0;
    m_pendingChunksReceived = 0;
    m_pendingWidth = 0;
    m_pendingHeight = 0;
    m_pendingQuantScale = 0;
    m_pendingVersion = 0;
    m_pendingChunkPresent.clear();
    m_width = 0;
    m_height = 0;
    m_totalSectorCount = 0;
    m_realtimeSectorCount = 0;
    m_loaded = false;
    m_error.clear();
}

double StrParser::GetDurationSeconds() const {
    constexpr double kSectorsPerSecond = 150.0;
    const uint32_t sectors = m_totalSectorCount;
    return (sectors != 0) ? ((double)sectors / kSectorsPerSecond) : 0.0;
}

bool StrParser::ParseSectors(const std::vector<uint8_t>& data) {
    // Skip 44-byte file header
    size_t offset = STR_FILE_HEADER_SIZE;
    size_t sectorCount = 0;
    size_t realtimeSectorCount = 0;
    size_t videoSectorCount = 0;
    size_t audioSectorCount = 0;

    while (offset + CDXA_SECTOR_SIZE <= data.size()) {
        const uint8_t* sector = data.data() + offset;

        // Validate sync pattern (optional, for robustness)
        // Expected: 00 FF FF FF FF FF FF FF FF FF FF 00

        // Get submode from XA subheader
        uint8_t submode = sector[CDXA_SUBMODE_OFFSET];

        // Get user data pointer
        const uint8_t* userData = sector + CDXA_USER_DATA_OFFSET;

        if (submode == 0x80) {
            // EOF marker
            Log::Printf("StrParser: EOF sector at offset 0x%zX", offset);
            break;
        }

        if (submode & CDXA_SUBMODE_REALTIME) {
            realtimeSectorCount++;
        }

        if (submode & CDXA_SUBMODE_VIDEO) {
            // Video sector
            videoSectorCount++;

            const uint8_t* chunkHeader = sector + STR_CHUNK_HEADER_OFFSET;
            const uint16_t chunkIndex = *(const uint16_t*)(chunkHeader + 0x04);
            const uint16_t chunkCount = *(const uint16_t*)(chunkHeader + 0x06);
            const uint32_t frameNo = *(const uint32_t*)(chunkHeader + 0x08);
            const uint32_t frameSize = *(const uint32_t*)(chunkHeader + 0x0C);
            const uint16_t width = *(const uint16_t*)(chunkHeader + 0x10);
            const uint16_t height = *(const uint16_t*)(chunkHeader + 0x12);
            const uint16_t quantScale = *(const uint16_t*)(chunkHeader + 0x18);
            const uint16_t version = *(const uint16_t*)(chunkHeader + 0x1A);

            const uint8_t* chunkData = sector + STR_CHUNK_HEADER_OFFSET + STR_CHUNK_HEADER_SIZE;
            const size_t chunkDataSize = STR_CHUNK_DATA_SIZE;

            // First frame sets global dimensions
            if (m_width == 0 && m_height == 0) {
                m_width = width;
                m_height = height;
                Log::Printf("StrParser: Video resolution %ux%u (frame 1)", width, height);
            }

            // Check if this is a new frame
            if (frameNo != m_pendingFrameNo && m_pendingFrameNo != kNoPendingFrame) {
                FlushPendingFrame();
            }

            // Start or continue accumulating frame data
            if (m_pendingFrameNo == kNoPendingFrame || frameNo != m_pendingFrameNo) {
                m_pendingFrameNo = frameNo;
                m_pendingFrameSize = frameSize;
                m_pendingChunkCount = chunkCount;
                m_pendingChunksReceived = 0;
                m_pendingWidth = width;
                m_pendingHeight = height;
                m_pendingQuantScale = quantScale;
                m_pendingVersion = version;
                m_pendingBitstream.assign(frameSize, 0);
                m_pendingChunkPresent.assign(chunkCount, 0);
            }

            if (m_pendingFrameSize != 0 && chunkIndex < m_pendingChunkCount && chunkIndex < m_pendingChunkPresent.size()) {
                if (!m_pendingChunkPresent[chunkIndex]) {
                    const size_t dstOff = size_t(chunkIndex) * STR_CHUNK_DATA_SIZE;
                    if (dstOff < m_pendingBitstream.size()) {
                        const size_t copySize = (std::min)(chunkDataSize, m_pendingBitstream.size() - dstOff);
                        memcpy(m_pendingBitstream.data() + dstOff, chunkData, copySize);
                    }
                    m_pendingChunkPresent[chunkIndex] = 1;
                    m_pendingChunksReceived++;
                }

                if (m_pendingChunkCount != 0 && m_pendingChunksReceived >= m_pendingChunkCount) {
                    FlushPendingFrame();
                }
            }
        }
        else if (submode & CDXA_SUBMODE_AUDIO) {
            // Audio sector
            audioSectorCount++;

            StrAudioSector audioSec;
            audioSec.file = sector[0x10];
            audioSec.channel = sector[0x11];
            audioSec.coding = sector[0x13];

            // Copy audio data (full user data for XA-ADPCM)
            audioSec.data.assign(userData, userData + CDXA_USER_DATA_SIZE);
            m_audioSectors.push_back(std::move(audioSec));
        }

        offset += CDXA_SECTOR_SIZE;
        sectorCount++;
    }

    // Flush last pending frame
    if (m_pendingFrameNo != kNoPendingFrame) {
        FlushPendingFrame();
    }

    Log::Printf("StrParser: Parsed %zu sectors (video=%zu, audio=%zu)",
                sectorCount, videoSectorCount, audioSectorCount);

    m_totalSectorCount = (uint32_t)sectorCount;
    m_realtimeSectorCount = (uint32_t)realtimeSectorCount;

    if (m_frames.empty()) {
        m_error = "No video frames found";
        Log::Printf("StrParser: %s", m_error.c_str());
        return false;
    }

    return true;
}

void StrParser::FlushPendingFrame() {
    if (m_pendingFrameNo == kNoPendingFrame || m_pendingBitstream.empty()) {
        return;
    }

    if (m_pendingChunkCount != 0 && m_pendingChunksReceived < m_pendingChunkCount) {
        m_pendingFrameNo = kNoPendingFrame;
        m_pendingFrameSize = 0;
        m_pendingChunkCount = 0;
        m_pendingChunksReceived = 0;
        m_pendingWidth = 0;
        m_pendingHeight = 0;
        m_pendingQuantScale = 0;
        m_pendingVersion = 0;
        m_pendingBitstream.clear();
        m_pendingChunkPresent.clear();
        return;
    }

    StrVideoFrame frame;
    frame.frameNo = m_pendingFrameNo;
    frame.width = m_pendingWidth;
    frame.height = m_pendingHeight;
    frame.quantScale = m_pendingQuantScale;
    frame.version = m_pendingVersion;
    frame.bitstream = std::move(m_pendingBitstream);
    m_frames.push_back(std::move(frame));

    m_pendingFrameNo = kNoPendingFrame;
    m_pendingFrameSize = 0;
    m_pendingChunkCount = 0;
    m_pendingChunksReceived = 0;
    m_pendingWidth = 0;
    m_pendingHeight = 0;
    m_pendingQuantScale = 0;
    m_pendingVersion = 0;
    m_pendingBitstream.clear();
    m_pendingChunkPresent.clear();
}

const StrVideoFrame* StrParser::GetFrame(size_t index) const {
    if (index >= m_frames.size()) return nullptr;
    return &m_frames[index];
}

const StrAudioSector* StrParser::GetAudioSector(size_t index) const {
    if (index >= m_audioSectors.size()) return nullptr;
    return &m_audioSectors[index];
}
