#include "vab_player.h"
#include "audio_engine.h"
#include "logger.h"

#include <fstream>
#include <cstring>
#include <cmath>
#include <algorithm>
#include <filesystem>
 #include <sstream>

// ---- PSX VAB format constants ----
// VH header: 32 bytes
// Followed by 128 program attributes (16 bytes each) = 2048 bytes
// Followed by nTones tone attrs (32 bytes each)
// Followed by VAG size table: 256 entries, each u16 (size in 8-byte units)

static constexpr uint32_t kVhHeaderSize = 32;
static constexpr uint32_t kProgramAttrSize = 16;
static constexpr uint32_t kMaxPrograms = 128;
static constexpr uint32_t kTonesPerProgram = 16;
static constexpr uint32_t kToneAttrSize = 32;

// PSX ADPCM filter coefficients (integer, matching duckstation SPU)
static const int32_t kFilterPos[5] = { 0, 60, 115, 98, 122 };
static const int32_t kFilterNeg[5] = { 0,  0, -52, -55, -60 };

static int16_t Clamp16(int32_t value) {
    if (value < -0x8000) return -0x8000;
    if (value > 0x7FFF)  return  0x7FFF;
    return (int16_t)value;
}

static bool WriteWavPcm16Mono(const std::filesystem::path& path, const int16_t* samples, size_t sampleCount, uint32_t sampleRate) {
    if (!samples || sampleCount == 0 || sampleRate == 0) return false;

    struct WavHeader {
        char riff[4];
        uint32_t riffSize;
        char wave[4];
        char fmt[4];
        uint32_t fmtSize;
        uint16_t audioFormat;
        uint16_t numChannels;
        uint32_t sampleRate;
        uint32_t byteRate;
        uint16_t blockAlign;
        uint16_t bitsPerSample;
        char data[4];
        uint32_t dataSize;
    };

    const uint32_t dataSize = (uint32_t)(sampleCount * sizeof(int16_t));
    WavHeader h = {};
    std::memcpy(h.riff, "RIFF", 4);
    std::memcpy(h.wave, "WAVE", 4);
    std::memcpy(h.fmt,  "fmt ", 4);
    std::memcpy(h.data, "data", 4);
    h.fmtSize = 16;
    h.audioFormat = 1;
    h.numChannels = 1;
    h.sampleRate = sampleRate;
    h.bitsPerSample = 16;
    h.blockAlign = (uint16_t)(h.numChannels * (h.bitsPerSample / 8));
    h.byteRate = h.sampleRate * h.blockAlign;
    h.dataSize = dataSize;
    h.riffSize = 36 + h.dataSize;

    std::ofstream f(path, std::ios::binary);
    if (!f) return false;
    f.write((const char*)&h, sizeof(h));
    f.write((const char*)samples, dataSize);
    return (bool)f;
}

bool VabPlayer::DumpAllWav(const std::string& outDir) const {
    if (!m_loaded) return false;
    if (outDir.empty()) return false;

    std::filesystem::path base(outDir);
    std::error_code ec;
    std::filesystem::create_directories(base, ec);
    if (ec) return false;

    std::filesystem::path vagDir = base / "vag";
    std::filesystem::path toneDir = base / "tone";
    std::filesystem::create_directories(vagDir, ec);
    if (ec) return false;
    std::filesystem::create_directories(toneDir, ec);
    if (ec) return false;

    int vagWritten = 0;
    for (int i = 0; i < (int)m_vags.size(); i++) {
        const auto& v = m_vags[i];
        if (v.pcm.empty()) continue;
        std::ostringstream name;
        name << "vag_";
        if (i < 10) name << "00";
        else if (i < 100) name << "0";
        name << i << "_" << v.sampleRate << "hz.wav";
        if (WriteWavPcm16Mono(vagDir / name.str(), v.pcm.data(), v.pcm.size(), v.sampleRate)) {
            vagWritten++;
        }
    }

    int toneWritten = 0;
    for (int ti = 0; ti < (int)m_tones.size(); ti++) {
        const auto& t = m_tones[ti];
        const int vagIdx = (int)t.vagIndex;
        if (vagIdx < 0 || vagIdx >= (int)m_vags.size()) continue;
        const auto& v = m_vags[vagIdx];
        if (v.pcm.empty()) continue;

        const int program = ti / (int)kTonesPerProgram;
        const int note = ti % (int)kTonesPerProgram;

        const uint8_t key = (uint8_t)(0x18 + note);
        uint32_t playRate = v.sampleRate;
        {
            const float semis = (float)((int)key - (int)t.centerNote);
            const float ratio = std::pow(2.0f, semis / 12.0f);
            const double r = (double)v.sampleRate * (double)ratio;
            if (r >= 1000.0 && r <= 192000.0) {
                playRate = (uint32_t)std::lround(r);
            }
        }

        std::ostringstream name;
        name << "tone_";
        if (ti < 10) name << "00";
        else if (ti < 100) name << "0";
        name << ti;
        name << "_p" << program << "_n" << note;
        name << "_vag";
        if (vagIdx < 10) name << "00";
        else if (vagIdx < 100) name << "0";
        name << vagIdx;
        name << "_cn" << (int)t.centerNote;
        name << "_" << playRate << "hz.wav";

        if (WriteWavPcm16Mono(toneDir / name.str(), v.pcm.data(), v.pcm.size(), playRate)) {
            toneWritten++;
        }
    }

    Log::Printf("VabPlayer: DumpAllWav outDir='%s' wrote vag=%d tone=%d", outDir.c_str(), vagWritten, toneWritten);
    return true;
}

bool VabPlayer::DumpCmdWav(const std::string& outDir, const std::vector<SfxCmd>& cmds) const {
    if (!m_loaded) return false;
    if (outDir.empty()) return false;
    if (cmds.empty()) return false;

    std::filesystem::path base(outDir);
    std::error_code ec;
    std::filesystem::create_directories(base, ec);
    if (ec) return false;

    std::filesystem::path cmdDir = base / "cmd";
    std::filesystem::create_directories(cmdDir, ec);
    if (ec) return false;

    int written = 0;
    for (int i = 0; i < (int)cmds.size(); i++) {
        const SfxCmd& c = cmds[(size_t)i];

        const int toneIdx = (int)c.program * (int)kTonesPerProgram + (int)c.note;
        if (toneIdx < 0 || toneIdx >= (int)m_tones.size()) continue;

        const auto& t = m_tones[toneIdx];
        const int vagIdx = (int)t.vagIndex;
        if (vagIdx < 0 || vagIdx >= (int)m_vags.size()) continue;
        const auto& v = m_vags[vagIdx];
        if (v.pcm.empty()) continue;

        uint32_t playRate = v.sampleRate;
        if (c.key != 0) {
            const float semis = (float)((int)c.key - (int)t.centerNote);
            const float ratio = std::pow(2.0f, semis / 12.0f);
            const double r = (double)v.sampleRate * (double)ratio;
            if (r >= 8000.0 && r <= 192000.0) {
                playRate = (uint32_t)std::lround(r);
            }
        }

        std::ostringstream name;
        name << "cmd_";
        if (i < 10) name << "00";
        else if (i < 100) name << "0";
        name << i;
        name << "_p" << (int)c.program;
        name << "_n" << (int)c.note;
        name << "_k" << (int)c.key;
        name << "_v" << (int)c.volume;
        name << "_tone";
        if (toneIdx < 10) name << "00";
        else if (toneIdx < 100) name << "0";
        name << toneIdx;
        name << "_vag";
        if (vagIdx < 10) name << "00";
        else if (vagIdx < 100) name << "0";
        name << vagIdx;
        name << "_" << playRate << "hz.wav";

        if (WriteWavPcm16Mono(cmdDir / name.str(), v.pcm.data(), v.pcm.size(), playRate)) {
            written++;
        }
    }

    Log::Printf("VabPlayer: DumpCmdWav outDir='%s' wrote cmd=%d", outDir.c_str(), written);
    return written > 0;
}

VabPlayer::VagSample VabPlayer::DecodeVag(const uint8_t* data, size_t size) {
    // PSX ADPCM: 16 bytes per block = 28 samples
    // Block: [shift_filter][flags][14 data bytes]
    // Each data byte = 2 nibbles (low first, then high) = 2 samples

    VagSample out = {};
    std::vector<int16_t>& pcm = out.pcm;
    pcm.reserve((size / 16) * 28);

    int16_t last[2] = {0, 0}; // last[0] = most recent, last[1] = previous

    for (size_t pos = 0; pos + 16 <= size; pos += 16) {
        uint8_t shiftFilter = data[pos];
        uint8_t flags = data[pos + 1];

        if ((flags & 0x04) && !out.hasLoop) {
            out.loopStartFrame = pcm.size();
            out.hasLoop = true;
        }

        uint8_t shift = shiftFilter & 0x0F;
        uint8_t filter = (shiftFilter >> 4) & 0x0F;

        // Clamp shift like PSX hardware (values > 12 act as 9)
        if (shift > 12) shift = 9;
        // Clamp filter to valid range
        if (filter > 4) filter = 0;

        int32_t fPos = kFilterPos[filter];
        int32_t fNeg = kFilterNeg[filter];

        for (int i = 0; i < 28; i++) {
            // Get nibble: low nibble first (index%2==0), high nibble second
            // Matching duckstation: (data[index/2] >> ((index%2)*4)) & 0x0F
            uint8_t byteVal = data[pos + 2 + (i / 2)];
            uint8_t nibble = (byteVal >> ((i % 2) * 4)) & 0x0F;

            // Sign-extend nibble to 16-bit, shift left 12, then arithmetic right shift
            // Matches duckstation: s32(static_cast<s16>(ZeroExtend16(nibble) << 12) >> shift)
            int32_t sample = (int32_t)(int16_t)((uint16_t)nibble << 12) >> shift;

            // Apply filter (matching duckstation integer math)
            sample += (last[0] * fPos) >> 6;
            sample += (last[1] * fNeg) >> 6;

            // Clamp and store
            last[1] = last[0];
            last[0] = Clamp16(sample);
            pcm.push_back(last[0]);
        }

        // Check end flags (for one-shot playback)
        // Bit 0 = loop end, Bit 1 = loop repeat, Bit 2 = loop start
        if (flags & 0x01) {
            out.loopEndFrame = pcm.size();
            break;
        }
    }

    if (out.loopEndFrame == 0 || out.loopEndFrame > pcm.size()) {
        out.loopEndFrame = pcm.size();
    }
    if (!out.hasLoop) {
        out.loopStartFrame = 0;
    } else if (out.loopStartFrame >= out.loopEndFrame) {
        out.hasLoop = false;
        out.loopStartFrame = 0;
    }

    return out;
}

bool VabPlayer::LoadFromBuffers(const std::vector<uint8_t>& vh, const std::vector<uint8_t>& vb, const char* tag) {
    (void)tag;
    m_loaded = false;

    const size_t vhSize = vh.size();
    const size_t vbSize = vb.size();

    if (vhSize < kVhHeaderSize) {
        Log::Printf("VabPlayer: VH too small (%zu bytes)", vhSize);
        return false;
    }

    if (vh[0] != 0x70 || vh[1] != 0x42 || vh[2] != 0x41 || vh[3] != 0x56) {
        Log::Printf("VabPlayer: bad VH magic");
        return false;
    }

    uint16_t nPrograms = *(uint16_t*)(vh.data() + 18);
    uint16_t nTones    = *(uint16_t*)(vh.data() + 20);
    uint16_t nVags     = *(uint16_t*)(vh.data() + 22);

    if (nPrograms > kMaxPrograms) nPrograms = kMaxPrograms;
    if (nVags > 256) nVags = 256;

    Log::Printf("VabPlayer: VH %zu bytes, %d programs, %d tones, %d VAGs",
                vhSize, nPrograms, nTones, nVags);

    m_programs.clear();
    m_programs.resize(nPrograms);
    for (int i = 0; i < (int)nPrograms; i++) {
        size_t off = kVhHeaderSize + i * kProgramAttrSize;
        if (off + kProgramAttrSize > vhSize) break;
        m_programs[i].numTones = vh[off + 0];
        m_programs[i].volume   = vh[off + 1];
        m_programs[i].priority = vh[off + 2];
        m_programs[i].mode     = vh[off + 3];
        m_programs[i].pan      = vh[off + 4];
    }

    size_t toneBaseOff = kVhHeaderSize + kMaxPrograms * kProgramAttrSize;
    const size_t toneCount = (size_t)nPrograms * (size_t)kTonesPerProgram;
    m_tones.clear();
    m_tones.resize(toneCount);
    for (int i = 0; i < (int)toneCount; i++) {
        size_t off = toneBaseOff + (size_t)i * kToneAttrSize;
        if (off + kToneAttrSize > vhSize) break;
        m_tones[i].priority   = vh[off + 0];
        m_tones[i].mode       = vh[off + 1];
        m_tones[i].vol        = vh[off + 2];
        m_tones[i].pan        = vh[off + 3];
        m_tones[i].centerNote = vh[off + 4];
        m_tones[i].centerFine = vh[off + 5];
        m_tones[i].noteMin    = vh[off + 6];
        m_tones[i].noteMax    = vh[off + 7];
        m_tones[i].vagIndex   = *(uint16_t*)(vh.data() + off + 22);
        m_tones[i].adsr1      = *(uint16_t*)(vh.data() + off + 14);
        m_tones[i].adsr2      = *(uint16_t*)(vh.data() + off + 16);

        Log::Printf("VabPlayer: tone[%d] vol=%d pan=%d cnote=%d vagIdx=%d",
                    i, m_tones[i].vol, m_tones[i].pan, m_tones[i].centerNote, m_tones[i].vagIndex);
    }

    size_t vagTableOff = toneBaseOff + toneCount * kToneAttrSize;
    if (vagTableOff + 256 * 2 > vhSize) {
        Log::Printf("VabPlayer: VH truncated at VAG table (off=%zu, need=%zu, have=%zu)",
                    vagTableOff, vagTableOff + 256 * 2, vhSize);
        return false;
    }

    int effectiveVags = (int)nVags;
    if (effectiveVags < 256) {
        size_t summed = 0;
        for (int i = 0; i < effectiveVags; i++) {
            const uint16_t sz = *(uint16_t*)(vh.data() + vagTableOff + i * 2);
            summed += (size_t)sz * 8;
        }

        int extra = effectiveVags;
        while (extra < 256) {
            const uint16_t sz = *(uint16_t*)(vh.data() + vagTableOff + extra * 2);
            if (sz == 0) break;
            const size_t bytes = (size_t)sz * 8;
            if (summed + bytes > vbSize) break;
            summed += bytes;
            extra += 1;
        }
        if (extra > effectiveVags) {
            Log::Printf("VabPlayer: extending VAG count from %d to %d using trailing non-zero size table entries",
                        effectiveVags, extra);
            effectiveVags = extra;
        }
    }

    m_vags.clear();
    m_vags.resize((size_t)effectiveVags);

    size_t vbOffset = 0;
    for (int i = 0; i < effectiveVags; i++) {
        uint16_t sizeField = *(uint16_t*)(vh.data() + vagTableOff + i * 2);
        const size_t vagBytesFull = (size_t)sizeField * 8;
        size_t vagBytes = vagBytesFull;

        if (vagBytes == 0) {
            m_vags[i].pcm.clear();
            m_vags[i].sampleRate = 44100;
            Log::Printf("VabPlayer: VAG[%d] empty (sizeField=0)", i);
            continue;
        }

        if (vbOffset >= vbSize) {
            Log::Printf("VabPlayer: VB truncated at VAG %d (off=%zu, have=%zu) -> stop parsing remaining VAGs",
                        i, vbOffset, vbSize);
            for (int j = i; j < (int)nVags; j++) {
                m_vags[j].pcm.clear();
                m_vags[j].sampleRate = 44100;
            }
            break;
        }

        const bool truncated = (vbOffset + vagBytes > vbSize);
        if (truncated) {
            Log::Printf("VabPlayer: VB truncated at VAG %d (off=%zu, need=%zu, have=%zu)",
                        i, vbOffset, vbOffset + vagBytes, vbSize);
            vagBytes = vbSize - vbOffset;
        }

        m_vags[i] = DecodeVag(vb.data() + vbOffset, vagBytes);
        m_vags[i].sampleRate = 44100;

        Log::Printf("VabPlayer: VAG[%d] offset=%zu size=%zu decoded=%zu samples",
                    i, vbOffset, vagBytes, m_vags[i].pcm.size());

        vbOffset += vagBytesFull;
        if (truncated) {
            // No more data for following VAGs; avoid vbOffset>vbSize underflow in next iteration.
            for (int j = i + 1; j < (int)nVags; j++) {
                m_vags[j].pcm.clear();
                m_vags[j].sampleRate = 44100;
            }
            break;
        }
    }

    Log::Printf("VabPlayer: decoded %d VAGs from VB (%zu bytes, totalOffset=%zu)", effectiveVags, vbSize, vbOffset);
    m_loaded = true;
    return true;
}

bool VabPlayer::LoadFromMemory(const uint8_t* vhData, size_t vhSize, const uint8_t* vbData, size_t vbSize) {
    if (!vhData || vhSize == 0 || !vbData || vbSize == 0) {
        Log::Printf("VabPlayer: LoadFromMemory invalid buffers");
        return false;
    }

    std::vector<uint8_t> vh(vhData, vhData + vhSize);
    std::vector<uint8_t> vb(vbData, vbData + vbSize);
    return LoadFromBuffers(vh, vb, "mem");
}

bool VabPlayer::Load(const std::string& vhPath, const std::string& vbPath) {
    std::ifstream vhFile(vhPath, std::ios::binary | std::ios::ate);
    if (!vhFile) {
        Log::Printf("VabPlayer: cannot open VH: %s", vhPath.c_str());
        return false;
    }
    size_t vhSize = (size_t)vhFile.tellg();
    vhFile.seekg(0);
    std::vector<uint8_t> vh(vhSize);
    vhFile.read((char*)vh.data(), vhSize);
    vhFile.close();

    std::ifstream vbFile(vbPath, std::ios::binary | std::ios::ate);
    if (!vbFile) {
        Log::Printf("VabPlayer: cannot open VB: %s", vbPath.c_str());
        return false;
    }
    size_t vbSize = (size_t)vbFile.tellg();
    vbFile.seekg(0);
    std::vector<uint8_t> vb(vbSize);
    vbFile.read((char*)vb.data(), vbSize);
    vbFile.close();

    return LoadFromBuffers(vh, vb, vhPath.c_str());
}

void VabPlayer::PlayTone(int toneIdx, float volume) {
    if (!m_loaded) return;
    if (toneIdx < 0 || toneIdx >= (int)m_tones.size()) {
        Log::Printf("VabPlayer: PlayTone toneIdx=%d out of range (max=%d)", toneIdx, (int)m_tones.size());
        return;
    }

    const auto& tone = m_tones[toneIdx];
    int vagIdx = tone.vagIndex; // 0-based index into VAG table
    if (vagIdx < 0 || vagIdx >= (int)m_vags.size()) {
        Log::Printf("VabPlayer: PlayTone tone[%d].vagIndex=%d out of range (max=%d)", toneIdx, vagIdx, (int)m_vags.size());
        return;
    }

    const auto& vag = m_vags[vagIdx];
    if (vag.pcm.empty()) {
        Log::Printf("VabPlayer: PlayTone tone[%d] -> VAG[%d] has empty PCM", toneIdx, vagIdx);
        return;
    }

    // Combine tone volume with requested volume
    float toneVol = (float)tone.vol / 127.0f;
    float finalVol = toneVol * volume;

    auto& engine = AudioEngine::Get();
    if (!engine.IsRunning()) {
        Log::Printf("VabPlayer: AudioEngine not running");
        return;
    }

    int voice = engine.AllocVoice(1, vag.sampleRate, finalVol, true); // mono, oneShot
    if (voice >= 0) {
        engine.QueueSamples(voice, vag.pcm.data(), vag.pcm.size());
        Log::Printf("VabPlayer: PlayTone[%d] -> VAG[%d] %zu samples, vol=%.2f, voice=%d",
                    toneIdx, vagIdx, vag.pcm.size(), finalVol, voice);
    } else {
        Log::Printf("VabPlayer: PlayTone[%d] -> AllocVoice failed", toneIdx);
    }
}

void VabPlayer::PlaySfxCmd(uint8_t program, uint8_t note, uint8_t volume) {
    // Backward-compatible: assume key equals tone's center note (no pitch shift)
    PlaySfxCmd(program, note, 0, volume);
}

int VabPlayer::PlaySfxCmdInternal(uint8_t program,
                                  uint8_t note,
                                  uint8_t key,
                                  uint8_t volume,
                                  double startOffsetSeconds) {
    if (!m_loaded) return -1;
    if (program >= m_programs.size()) {
        Log::Printf("VabPlayer: PlaySfxCmd program=%d out of range", program);
        return -1;
    }

    int toneIdx = (int)program * kTonesPerProgram + (int)note;
    if (toneIdx < 0 || toneIdx >= (int)m_tones.size()) {
        Log::Printf("VabPlayer: PlaySfxCmd prog=%d note=%d -> toneIdx=%d out of range", program, note, toneIdx);
        return -1;
    }

    const auto& tone = m_tones[toneIdx];
    int vagIdx = (int)tone.vagIndex;
    if (vagIdx == (int)m_vags.size() && !m_vags.empty()) {
        int minUsed = 0x7FFFFFFF;
        int maxUsed = -1;
        int invalidCount = 0;
        std::vector<uint8_t> used(m_vags.size(), (uint8_t)0);
        const int progBase = (int)program * (int)kTonesPerProgram;
        for (int i = 0; i < (int)kTonesPerProgram; ++i) {
            const int idx = progBase + i;
            if (idx < 0 || idx >= (int)m_tones.size()) break;
            const auto& pt = m_tones[idx];
            if (pt.vol == 0 && pt.centerNote == 0 && pt.vagIndex == 0) continue;
            const int pv = (int)pt.vagIndex;
            if (pv >= 0 && pv < (int)m_vags.size()) {
                used[(size_t)pv] = 1;
                if (pv < minUsed) minUsed = pv;
                if (pv > maxUsed) maxUsed = pv;
            } else if (pv == (int)m_vags.size()) {
                invalidCount += 1;
            }
        }

        if (invalidCount == 1 && maxUsed == (int)m_vags.size() - 1 && minUsed > 0 && !used[(size_t)(minUsed - 1)]) {
            vagIdx = minUsed - 1;
        } else {
            Log::Printf("VabPlayer: PlaySfxCmd tone[%d].vagIndex=%d out of range (max=%d)",
                        toneIdx, vagIdx, (int)m_vags.size());
            return -1;
        }
    }
    if (vagIdx < 0 || vagIdx >= (int)m_vags.size()) {
        Log::Printf("VabPlayer: PlaySfxCmd tone[%d].vagIndex=%d out of range (max=%d)", toneIdx, vagIdx, (int)m_vags.size());
        return -1;
    }
    const auto& vag = m_vags[vagIdx];
    if (vag.pcm.empty()) {
        Log::Printf("VabPlayer: PlaySfxCmd tone[%d] -> VAG[%d] has empty PCM", toneIdx, vagIdx);
        return -1;
    }

    const float cmdVol = (float)volume / 127.0f;
    const float toneVol = (float)tone.vol / 127.0f;
    const float finalVol = std::clamp(toneVol * cmdVol, 0.0f, 1.0f);

    // Pitch: interpret key as a MIDI-like note number.
    // Use tone.centerNote as the reference note at which the sample plays at its base rate.
    uint32_t playRate = vag.sampleRate;
    if (key != 0) {
        const float semis = (float)((int)key - (int)tone.centerNote);
        const float ratio = std::pow(2.0f, semis / 12.0f);
        const double r = (double)vag.sampleRate * (double)ratio;
        if (r >= 8000.0 && r <= 192000.0) {
            playRate = (uint32_t)std::lround(r);
        }
    }

    auto& engine = AudioEngine::Get();
    if (!engine.IsRunning()) {
        Log::Printf("VabPlayer: AudioEngine not running");
        return -1;
    }

    int voice = engine.AllocVoice(1, playRate, finalVol, true);
    if (voice >= 0) {
        if (!std::isfinite(startOffsetSeconds) || startOffsetSeconds < 0.0) {
            startOffsetSeconds = 0.0;
        }
        size_t skipFrames =
            (size_t)std::floor(startOffsetSeconds * (double)playRate);
        if (skipFrames >= vag.pcm.size()) {
            skipFrames = 0;
        }
        engine.QueueSamples(voice,
                            vag.pcm.data() + skipFrames,
                            vag.pcm.size() - skipFrames);
    } else {
        Log::Printf("VabPlayer: PlaySfxCmd -> AllocVoice failed");
    }
    return voice;
}

void VabPlayer::PlaySfxCmd(uint8_t program, uint8_t note, uint8_t key, uint8_t volume) {
    PlaySfxCmdInternal(program, note, key, volume);
}

int VabPlayer::PlaySfxCmdEx(uint8_t program, uint8_t note, uint8_t key, uint8_t volume) {
    return PlaySfxCmdInternal(program, note, key, volume);
}

int VabPlayer::PlaySfxCmdExWithStartOffset(uint8_t program,
                                           uint8_t note,
                                           uint8_t key,
                                           uint8_t volume,
                                           double startOffsetSeconds) {
    return PlaySfxCmdInternal(program, note, key, volume, startOffsetSeconds);
}

bool VabPlayer::GetTonePcmInfo(uint8_t program, uint8_t note, uint8_t key,
                              size_t& outSamples, uint32_t& outPlayRate,
                              size_t* outLoopStart, size_t* outLoopEnd) const {
    outSamples = 0;
    outPlayRate = 0;
    if (!m_loaded) return false;
    if (program >= m_programs.size()) return false;

    const int toneIdx = (int)program * (int)kTonesPerProgram + (int)note;
    if (toneIdx < 0 || toneIdx >= (int)m_tones.size()) return false;
    const auto& tone = m_tones[toneIdx];

    const int vagIdx = (int)tone.vagIndex;
    if (vagIdx < 0 || vagIdx >= (int)m_vags.size()) return false;
    const auto& vag = m_vags[vagIdx];
    if (vag.pcm.empty() || vag.sampleRate == 0) return false;

    uint32_t playRate = vag.sampleRate;
    if (key != 0) {
        const float semis = (float)((int)key - (int)tone.centerNote);
        const float ratio = std::pow(2.0f, semis / 12.0f);
        const double r = (double)vag.sampleRate * (double)ratio;
        if (r >= 8000.0 && r <= 192000.0) {
            playRate = (uint32_t)std::lround(r);
        }
    }

    outSamples = vag.pcm.size();
    outPlayRate = playRate;
    if (outLoopStart) *outLoopStart = vag.hasLoop ? vag.loopStartFrame : 0;
    if (outLoopEnd) *outLoopEnd = vag.hasLoop ? vag.loopEndFrame : vag.pcm.size();
    return true;
}
