#include "xa1_player.h"
#include "audio_engine.h"
#include "logger.h"

#include <algorithm>
#include <cmath>

namespace {
constexpr int kMaxCdSectorsPerUpdate = 8;
constexpr double kAudioQueueGovernorTargetFraction = 0.50;
constexpr double kAudioQueueGovernorDeadbandFraction = 0.12;
constexpr double kAudioQueueGovernorGain = 0.04;
constexpr double kAudioQueueGovernorMaxScale = 1.04;
constexpr double kAudioQueueGovernorMinSectorSeconds = 1.0 / 75.0;
constexpr double kAudioQueueGovernorMaxSectorSeconds = 0.150;
}

Xa1Player::Xa1Player() {
    m_sectorBuf.resize(kSectorSize);
}

Xa1Player::~Xa1Player() {
    Stop();
}

bool Xa1Player::IsPlaying() const {
    return m_state == State::Playing;
}

bool Xa1Player::IsFinished() const {
    return m_state == State::Finished;
}

double Xa1Player::GetPlayedSeconds() const {
    if (m_audioVoice < 0) {
        return 0.0;
    }

    double played = AudioEngine::Get().GetVoicePlayedSeconds(m_audioVoice);
    if (played < 0.0) {
        played = 0.0;
    }

    const double dur = GetEstimatedDurationSeconds();
    if (dur > 0.001 && played > dur) {
        played = dur;
    }
    return played;
}

double Xa1Player::GetEstimatedDurationSeconds() const {
    if (m_totalSectors == 0) return 0.0;
    constexpr double kSectorsPerSecond = 75.0;
    return (double)m_totalSectors / kSectorsPerSecond;
}

double Xa1Player::GetQueuedSeconds() const {
    if (m_audioVoice < 0) {
        return 0.0;
    }

    const uint32_t sampleRate = AudioEngine::Get().GetVoiceSampleRate(m_audioVoice);
    const int channels = AudioEngine::Get().GetVoiceChannels(m_audioVoice);
    const size_t queuedSamples = AudioEngine::Get().GetVoiceQueuedSamples(m_audioVoice);
    if (sampleRate == 0 || channels <= 0) {
        return 0.0;
    }

    return (double)queuedSamples / (double)sampleRate / (double)channels;
}

int Xa1Player::GetEstimatedAudibleSectorIndex() const {
    if (m_totalSectors == 0) {
        return 0;
    }

    double sector = (double)m_sectorIndex - GetQueuedSeconds() * 75.0;
    if (sector < 0.0) {
        sector = 0.0;
    }
    const double maxSector = (double)m_totalSectors;
    if (sector > maxSector) {
        sector = maxSector;
    }
    return (int)std::floor(sector + 1e-9);
}

int Xa1Player::GetCdClockSectorIndex() const {
    if (m_totalSectors == 0) {
        return 0;
    }

    double sector = m_cdClockSector75;
    if (sector < 0.0) {
        sector = 0.0;
    }
    const double maxSector = (double)m_totalSectors;
    if (sector > maxSector) {
        sector = maxSector;
    }
    return (int)std::floor(sector + 1e-9);
}

bool Xa1Player::FillRawSectorView(const StoredRawSector& stored, XaRawSectorView& out) {
    const uint8_t* sector = stored.sector.data();
    out = XaRawSectorView{};
    out.sectorIndex = stored.sectorIndex;
    out.file = sector[0x10];
    out.channel = sector[0x11];
    out.submode = sector[kSubmodeOffset];
    out.coding = sector[0x13];
    out.realtimeAudio =
        ((out.submode & (kSubmodeRealtime | kSubmodeAudio)) ==
         (kSubmodeRealtime | kSubmodeAudio));
    out.eof = (out.submode == kSubmodeEof);
    out.sector2352 = sector;
    out.sectorSize = kSectorSize;
    out.userData = sector + kUserDataOffset;
    out.userDataSize = kUserDataSize;
    return true;
}

void Xa1Player::PushAcceptedRawSector(const uint8_t* sector,
                                      size_t size,
                                      uint32_t sectorIndex) {
    if (!sector || size < kSectorSize) {
        return;
    }
    if (m_acceptedRawSectors.size() >= kAcceptedRawSectorQueueLimit) {
        m_acceptedRawSectors.pop_front();
    }

    StoredRawSector stored{};
    stored.sectorIndex = sectorIndex;
    std::copy(sector, sector + kSectorSize, stored.sector.begin());
    m_acceptedRawSectors.push_back(std::move(stored));
    ++m_acceptedRawPushCount;
    m_lastAcceptedSectorKnown = true;
    m_lastAcceptedSectorIndex = sectorIndex;
    m_lastAcceptedFile = sector[0x10];
    m_lastAcceptedChannel = sector[0x11];
    m_lastAcceptedCoding = sector[0x13];
}

bool Xa1Player::PollAcceptedRawSector(XaRawSectorView& out) {
    out = XaRawSectorView{};
    if (m_acceptedRawSectors.empty()) {
        return false;
    }

    m_polledRawSector = std::move(m_acceptedRawSectors.front());
    m_acceptedRawSectors.pop_front();
    m_polledRawSectorValid = true;
    return FillRawSectorView(m_polledRawSector, out);
}

bool Xa1Player::PollAcceptedRingPacket(Xa1RingPacketView& out) {
    out = Xa1RingPacketView{};

    XaRawSectorView raw{};
    if (!PollAcceptedRawSector(raw)) {
        return false;
    }
    if (!raw.userData || raw.userDataSize < (kRingHeaderSize + kRingPayloadSize)) {
        return false;
    }

    out.sectorIndex = raw.sectorIndex;
    out.file = raw.file;
    out.channel = raw.channel;
    out.coding = raw.coding;
    if (raw.sector2352 != nullptr && raw.sectorSize >= 0x14) {
        out.cdGetlocPResponseKnown = true;
        std::copy(raw.sector2352 + 0x0C,
                  raw.sector2352 + 0x14,
                  out.cdGetlocPResponse.begin());
        out.cdDataReadyInterruptKnown = true;
        out.cdDataReadyInterrupt = 2;
    }
    out.header32 = raw.userData;
    out.headerSize = kRingHeaderSize;
    out.payload2016 = raw.userData + kRingHeaderSize;
    out.payloadSize = kRingPayloadSize;
    return true;
}

bool Xa1Player::GetLastCdClockGetlocPResponse(
    std::array<uint8_t, 8>& out,
    uint32_t* outSectorIndex) const {
    out = {};
    if (!m_lastCdClockGetlocPResponseKnown) {
        return false;
    }

    out = m_lastCdClockGetlocPResponse;
    if (outSectorIndex != nullptr) {
        *outSectorIndex = m_lastCdClockGetlocPSectorIndex;
    }
    return true;
}

uint32_t Xa1Player::ReadU32LE(const uint8_t* p) {
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

bool Xa1Player::ParseRiffCdxa(std::ifstream& f, uint64_t& outDataOffset, uint32_t& outDataSize) {
    outDataOffset = 0;
    outDataSize = 0;

    uint8_t riff[12] = {};
    f.read((char*)riff, sizeof(riff));
    if (f.gcount() != (std::streamsize)sizeof(riff)) {
        return false;
    }

    if (riff[0] != 'R' || riff[1] != 'I' || riff[2] != 'F' || riff[3] != 'F') {
        return false;
    }
    if (riff[8] != 'C' || riff[9] != 'D' || riff[10] != 'X' || riff[11] != 'A') {
        return false;
    }

    while (true) {
        uint8_t h[8] = {};
        f.read((char*)h, sizeof(h));
        if (f.gcount() != (std::streamsize)sizeof(h)) {
            break;
        }

        const uint32_t chunkSize = ReadU32LE(h + 4);
        const bool isData = (h[0] == 'd' && h[1] == 'a' && h[2] == 't' && h[3] == 'a');
        if (isData) {
            outDataOffset = (uint64_t)f.tellg();
            outDataSize = chunkSize;
            return outDataSize != 0;
        }

        std::streamoff skip = (std::streamoff)chunkSize;
        if (chunkSize & 1) {
            skip += 1;
        }
        f.seekg(skip, std::ios::cur);
        if (!f.good()) {
            break;
        }
    }

    return false;
}

bool Xa1Player::Play(const std::filesystem::path& path) {
    Stop();

    m_path = path;
    m_file = std::ifstream(path, std::ios::binary);
    if (!m_file.is_open()) {
        Log::Printf("Xa1Player: open failed: %s", path.u8string().c_str());
        m_state = State::Error;
        return false;
    }

    uint64_t dataOffset = 0;
    uint32_t dataSize = 0;
    if (!ParseRiffCdxa(m_file, dataOffset, dataSize)) {
        Log::Printf("Xa1Player: RIFF CDXA parse failed: %s", path.u8string().c_str());
        Stop();
        m_state = State::Error;
        return false;
    }

    m_dataOffset = dataOffset;
    m_dataSize = dataSize;
    m_totalSectors = (uint32_t)(m_dataSize / (uint32_t)kSectorSize);
    m_sectorIndex = 0;
    m_cdClockSector75 = 0.0;

    m_audioInited = false;
    m_selValid = false;
    m_selCodingValid = false;
    m_selFile = 0;
    m_selChannel = 0;
    m_selCoding = 0;
    m_acceptedRawSectors.clear();
    m_polledRawSectorValid = false;
    m_acceptedRawPushCount = 0;
    m_lastAcceptedSectorKnown = false;
    m_lastAcceptedSectorIndex = 0;
    m_lastAcceptedFile = 0;
    m_lastAcceptedChannel = 0;
    m_lastAcceptedCoding = 0;
    m_setFilterChangeCount = 0;
    m_lastSetFilterKnown = false;
    m_lastSetFilterSectorIndex = 0;
    m_lastSetFilterFile = 0;
    m_lastSetFilterChannel = 0;
    m_lastCdClockGetlocPResponseKnown = false;
    m_lastCdClockGetlocPSectorIndex = 0;
    m_lastCdClockGetlocPResponse = {};
    m_lastDecodedSectorSeconds = 0.0;
    m_audioPlaybackRateScale = 1.0;

    m_sampleRate = 37800;
    m_stereo = false;

    m_decoder.Reset();

    m_file.clear();
    m_file.seekg((std::streamoff)m_dataOffset, std::ios::beg);

    m_state = State::Playing;

    Log::Printf("Xa1Player: Play '%s' dataSize=%u sectors=%u", path.u8string().c_str(), (unsigned)m_dataSize, (unsigned)m_totalSectors);

    return true;
}

void Xa1Player::Stop() {
    if (m_audioVoice >= 0) {
        AudioEngine::Get().FreeVoice(m_audioVoice);
        m_audioVoice = -1;
    }
    m_decoder.Reset();

    if (m_file.is_open()) {
        m_file.close();
    }

    m_state = State::Idle;
    m_path.clear();

    m_dataOffset = 0;
    m_dataSize = 0;
    m_totalSectors = 0;
    m_sectorIndex = 0;
    m_cdClockSector75 = 0.0;

    m_audioInited = false;
    m_selValid = false;
    m_selCodingValid = false;
    m_selFile = 0;
    m_selChannel = 0;
    m_selCoding = 0;
    m_lastDecodedSectorSeconds = 0.0;
    m_audioPlaybackRateScale = 1.0;
    m_acceptedRawSectors.clear();
    m_polledRawSectorValid = false;
    m_acceptedRawPushCount = 0;
    m_lastAcceptedSectorKnown = false;
    m_lastAcceptedSectorIndex = 0;
    m_lastAcceptedFile = 0;
    m_lastAcceptedChannel = 0;
    m_lastAcceptedCoding = 0;
    m_setFilterChangeCount = 0;
    m_lastSetFilterKnown = false;
    m_lastSetFilterSectorIndex = 0;
    m_lastSetFilterFile = 0;
    m_lastSetFilterChannel = 0;
    m_lastCdClockGetlocPResponseKnown = false;
    m_lastCdClockGetlocPSectorIndex = 0;
    m_lastCdClockGetlocPResponse = {};

    m_sampleRate = 37800;
    m_stereo = false;
}

bool Xa1Player::SetFilter(uint8_t file, uint8_t channel) {
    if (m_state != State::Playing) {
        return false;
    }

    if (m_selValid && m_selFile == file && m_selChannel == channel) {
        return true;
    }

    m_selValid = true;
    m_selCodingValid = false;
    m_selFile = file;
    m_selChannel = channel;
    m_selCoding = 0;
    m_decoder.Reset();
    ++m_setFilterChangeCount;
    m_lastSetFilterKnown = true;
    m_lastSetFilterSectorIndex = m_sectorIndex;
    m_lastSetFilterFile = file;
    m_lastSetFilterChannel = channel;

    Log::Printf("Xa1Player: SetFilter file=%u ch=%u sector=%u",
                (unsigned)m_selFile,
                (unsigned)m_selChannel,
                (unsigned)m_sectorIndex);
    return true;
}

void Xa1Player::SetVolume(float volume) {
    m_volume = std::clamp(volume, 0.0f, 1.0f);
    if (m_audioInited && m_audioVoice >= 0) {
        AudioEngine::Get().SetVoiceVolume(m_audioVoice, m_volume);
    }
}

void Xa1Player::NotifyAudioEngineReset() {
    if (m_state != State::Playing) {
        return;
    }

    // The shared PSX audio reset can invalidate the host voice without
    // rewinding the XA/CD stream or changing the selected file/channel.
    m_audioVoice = -1;
    m_audioInited = false;
    m_selCodingValid = false;
    m_lastDecodedSectorSeconds = 0.0;
    m_audioPlaybackRateScale = 1.0;
    m_decoder.Reset();
}

void Xa1Player::ApplyAudioQueueGovernor() {
    if (!m_audioInited || m_audioVoice < 0) {
        m_audioPlaybackRateScale = 1.0;
        return;
    }

    double sectorSeconds = m_lastDecodedSectorSeconds;
    if (!std::isfinite(sectorSeconds) || sectorSeconds <= 0.0) {
        sectorSeconds = kAudioQueueGovernorMinSectorSeconds;
    }
    sectorSeconds = std::clamp(sectorSeconds,
                               kAudioQueueGovernorMinSectorSeconds,
                               kAudioQueueGovernorMaxSectorSeconds);

    const double queuedSeconds = GetQueuedSeconds();
    const double targetSeconds =
        sectorSeconds * kAudioQueueGovernorTargetFraction;
    const double deadbandSeconds =
        sectorSeconds * kAudioQueueGovernorDeadbandFraction;

    double scale = 1.0;
    if (queuedSeconds > targetSeconds + deadbandSeconds) {
        const double normalizedError =
            (queuedSeconds - targetSeconds - deadbandSeconds) / sectorSeconds;
        scale += std::clamp(normalizedError * kAudioQueueGovernorGain,
                            0.0,
                            kAudioQueueGovernorMaxScale - 1.0);
    }

    if (std::abs(scale - m_audioPlaybackRateScale) >= 0.001) {
        AudioEngine::Get().SetVoicePlaybackRateScale(m_audioVoice, scale);
        m_audioPlaybackRateScale = scale;
    }
}

void Xa1Player::PumpQueuedSectors() {
    // Stage XA1 uses CdlSetmode(0x48): realtime + sector filter.  The PSX CD
    // stream advances at 75 raw sectors per second and the current filter only
    // decides which future sectors are accepted; it does not build a long host
    // decoded queue that has to be dropped or rewound at rating changes.
    const uint32_t targetSector = (uint32_t)GetCdClockSectorIndex();
    int sectorsRead = 0;
    while (m_sectorIndex < targetSector &&
           sectorsRead < kMaxCdSectorsPerUpdate) {
        const uint32_t sectorIndex = m_sectorIndex;
        if (!ReadNextSector(m_sectorBuf)) {
            m_state = State::Finished;
            break;
        }

        const uint8_t submode = m_sectorBuf[kSubmodeOffset];
        if (submode == kSubmodeEof) {
            m_state = State::Finished;
            break;
        }

        if (!TryConsumeSector(m_sectorBuf.data(), m_sectorBuf.size(), sectorIndex)) {
            m_state = State::Finished;
            break;
        }

        sectorsRead++;
    }
}

bool Xa1Player::ReadNextSector(std::vector<uint8_t>& outSector) {
    if (!m_file.is_open()) return false;
    if (m_sectorIndex >= m_totalSectors) return false;

    m_file.read((char*)outSector.data(), (std::streamsize)kSectorSize);
    if (m_file.gcount() != (std::streamsize)kSectorSize) {
        return false;
    }

    m_sectorIndex++;
    return true;
}

bool Xa1Player::TryConsumeSector(const uint8_t* sector, size_t size, uint32_t sectorIndex) {
    if (!sector || size < kSectorSize) return false;

    const uint8_t submode = sector[kSubmodeOffset];
    if (submode == kSubmodeEof) {
        return false;
    }

    if (size >= 0x14u) {
        m_lastCdClockGetlocPResponseKnown = true;
        m_lastCdClockGetlocPSectorIndex = sectorIndex;
        std::copy(sector + 0x0C,
                  sector + 0x14,
                  m_lastCdClockGetlocPResponse.begin());
    }

    const bool isRealtimeAudio = ((submode & (kSubmodeRealtime | kSubmodeAudio)) == (kSubmodeRealtime | kSubmodeAudio));
    if (!isRealtimeAudio) {
        return true;
    }

    const uint8_t file = sector[0x10];
    const uint8_t ch = sector[0x11];
    const uint8_t coding = sector[0x13];

    if (!m_selValid) {
        m_selValid = true;
        m_selFile = file;
        m_selChannel = ch;
    }

    if (file != m_selFile || ch != m_selChannel) {
        return true;
    }

    PushAcceptedRawSector(sector, size, sectorIndex);

    if (!m_selCodingValid) {
        m_selCodingValid = true;
        m_selCoding = coding;

        const bool newStereo = (coding & 0x01) != 0;
        const bool halfRate = (coding & 0x04) != 0;
        const uint32_t newSampleRate = halfRate ? 18900u : 37800u;

        if (m_audioInited &&
            (m_stereo != newStereo || m_sampleRate != newSampleRate)) {
            AudioEngine::Get().FreeVoice(m_audioVoice);
            m_audioVoice = -1;
            m_audioInited = false;
            m_decoder.Reset();
        }

        m_stereo = newStereo;
        m_sampleRate = newSampleRate;

        if (m_audioInited && !AudioEngine::Get().IsVoiceActive(m_audioVoice)) {
            m_audioVoice = -1;
            m_audioInited = false;
            m_decoder.Reset();
        }

        if (!m_audioInited) {
            m_audioVoice =
                AudioEngine::Get().AllocVoice(
                    m_stereo ? 2 : 1,
                    m_sampleRate,
                    m_volume);
            if (m_audioVoice >= 0) {
                m_audioInited = true;
                m_audioPlaybackRateScale = 1.0;
                AudioEngine::Get().SetVoicePlaybackRateScale(m_audioVoice, 1.0);
            }
        }

        Log::Printf("Xa1Player: Select audio file=%u ch=%u coding=0x%02X rate=%u %s init=%d",
                    (unsigned)m_selFile,
                    (unsigned)m_selChannel,
                    (unsigned)m_selCoding,
                    (unsigned)m_sampleRate,
                    m_stereo ? "stereo" : "mono",
                    m_audioInited ? 1 : 0);
    }

    if (m_audioInited && !AudioEngine::Get().IsVoiceActive(m_audioVoice)) {
        m_audioVoice = -1;
        m_audioInited = false;
        m_decoder.Reset();
    }

    if (!m_audioInited) {
        return true;
    }

    if (coding != m_selCoding) {
        return true;
    }

    const uint8_t* userData = sector + kUserDataOffset;
    if (m_decoder.DecodeSector(userData, kUserDataSize, coding)) {
        const auto& samples = m_decoder.GetSamples();
        if (!samples.empty()) {
            const int channels = m_stereo ? 2 : 1;
            if (m_sampleRate != 0 && channels > 0) {
                m_lastDecodedSectorSeconds =
                    (double)samples.size() /
                    ((double)m_sampleRate * (double)channels);
            }
            AudioEngine::Get().QueueSamples(m_audioVoice, samples.data(), samples.size());
            ApplyAudioQueueGovernor();
        }
    }

    return true;
}

void Xa1Player::Update() {
    if (m_state != State::Playing) return;

    if (m_totalSectors != 0 && m_sectorIndex >= m_totalSectors) {
        m_state = State::Finished;
        return;
    }

    // Update() is called from the 30Hz game logic tick. PSX CD time advances
    // at 75 raw sectors per second, so the HAL clock must advance 2.5 sectors
    // per logic frame; render60fps must not halve the stream clock.
    m_cdClockSector75 += 75.0 / 30.0;
    if (m_totalSectors != 0 &&
        m_cdClockSector75 > (double)m_totalSectors) {
        m_cdClockSector75 = (double)m_totalSectors;
    }

    PumpQueuedSectors();
    ApplyAudioQueueGovernor();
}
