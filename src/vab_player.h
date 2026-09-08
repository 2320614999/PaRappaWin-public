#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

// PSX VAB (Voice Attribute Bank) player
// Loads MINIMUM.VH/VB and decodes VAG ADPCM samples for playback via AudioEngine
class VabPlayer {
public:
    struct MidiProgramAttributes80032EAC {
        bool valid = false;
        uint8_t toneCount = 0u;
        uint8_t volume = 0u;
        uint8_t priority = 0u;
        uint8_t mode = 0u;
        uint8_t pan = 0u;
        uint8_t toneTableProgram = 0u;
    };

    struct MidiToneAttributes80032EAC {
        bool valid = false;
        uint8_t toneSlot = 0u;
        uint16_t toneIndex = 0u;
        uint8_t priority = 0u;
        uint8_t mode = 0u;
        uint8_t volume = 0u;
        uint8_t pan = 0u;
        uint8_t centerNote = 0u;
        uint8_t centerFine = 0u;
        uint8_t noteMin = 0u;
        uint8_t noteMax = 0u;
        uint16_t adsr1 = 0u;
        uint16_t adsr2 = 0u;
        uint16_t sampleId = 0u;
        bool sampleStartAddressKnown = false;
        uint16_t sampleStartAddress = 0u;
    };

    // Load a VAB from separate VH (header) and VB (body) files
    bool Load(const std::string& vhPath, const std::string& vbPath);

    // Load a VAB from memory buffers (VH header + VB body)
    bool LoadFromMemory(const uint8_t* vhData, size_t vhSize,
                        const uint8_t* vbData, size_t vbSize);

    // Play a sound effect using the original PSX SE command format
    // program: program index within the VAB (typically 0)
    // note: note/tone index (maps to specific VAG sample)
    // volume: 0-127
    void PlaySfxCmd(uint8_t program, uint8_t note, uint8_t volume);
    void PlaySfxCmd(uint8_t program, uint8_t note, uint8_t key, uint8_t volume);
    int  PlaySfxCmdEx(uint8_t program, uint8_t note, uint8_t key, uint8_t volume); // returns voice ID or -1
    int  PlaySfxCmdExWithStartOffset(uint8_t program, uint8_t note, uint8_t key,
                                      uint8_t volume,
                                      double startOffsetSeconds);

    // Direct Ss MIDI path: select every tone layer in the program whose
    // note range contains key, matching 80032EAC rather than compact SE
    // command tone-slot addressing.
    std::vector<int> ResolveMidiNoteToneLayers80032EAC(
        uint8_t program, uint8_t key) const;
    bool ResolveMidiNoteAttributes80032EAC(
        uint8_t program,
        uint8_t key,
        MidiProgramAttributes80032EAC& outProgram,
        std::vector<MidiToneAttributes80032EAC>& outTones) const;
    bool ResolveCompactSfxAttributes80034240(
        uint8_t program,
        uint8_t toneSlot,
        MidiProgramAttributes80032EAC& outProgram,
        MidiToneAttributes80032EAC& outTone) const;
    int PlayMidiToneLayerAtVoice80032EAC(
        const MidiToneAttributes80032EAC& tone,
        uint16_t pitch,
        float volume,
        int voiceId);
    uint32_t GetSpuAllocationBytes8002E474() const {
        return m_spuAllocationBytes8002E474;
    }
    bool ApplySpuAllocationBase8002E474(uint32_t baseAddress);
    uint8_t GetMasterVolume80032EAC() const {
        return m_masterVolume80032EAC;
    }

    bool DumpAllWav(const std::string& outDir) const;

    struct SfxCmd {
        uint8_t program;
        uint8_t note;
        uint8_t key;
        uint8_t volume;
    };
    bool DumpCmdWav(const std::string& outDir, const std::vector<SfxCmd>& cmds) const;

    // Play a specific tone by index directly
    void PlayTone(int toneIdx, float volume = 1.0f);

    bool IsLoaded() const { return m_loaded; }
    int  GetToneCount() const { return (int)m_tones.size(); }
    int  GetVagCount() const { return (int)m_vags.size(); }

    bool GetTonePcmInfo(uint8_t program, uint8_t note, uint8_t key,
                        size_t& outSamples, uint32_t& outPlayRate,
                        size_t* outLoopStart = nullptr, size_t* outLoopEnd = nullptr) const;

private:
    struct Tone {
        uint8_t  priority;
        uint8_t  mode;
        uint8_t  vol;          // 0-127
        uint8_t  pan;          // 0-127, 64=center
        uint8_t  centerNote;   // MIDI note number at which sample plays at original rate
        uint8_t  centerFine;
        uint8_t  noteMin;
        uint8_t  noteMax;
        uint16_t vagIndex;     // 0-based index into VAG table
        uint16_t adsr1;
        uint16_t adsr2;
    };

    struct VagSample {
        std::vector<int16_t> pcm;   // decoded PCM16 mono
        uint32_t sampleRate;        // default 44100
        size_t loopStartFrame = 0;
        size_t loopEndFrame = 0;
        bool hasLoop = false;
    };

    struct Program {
        uint8_t numTones;
        uint8_t volume;     // 0-127
        uint8_t priority;
        uint8_t mode;
        uint8_t pan;        // 0-127
        uint8_t toneTableProgram;
    };

    // Internal SFX playback (returns voice ID or -1)
    int PlaySfxCmdInternal(uint8_t program, uint8_t note, uint8_t key,
                           uint8_t volume,
                           double startOffsetSeconds = 0.0);

    bool LoadFromBuffers(const std::vector<uint8_t>& vh, const std::vector<uint8_t>& vb, const char* tag);

    // Decode PSX VAG ADPCM data to PCM16
    static VagSample DecodeVag(const uint8_t* data, size_t size);

    std::vector<Program> m_programs;
    std::vector<Tone>    m_tones;
    std::vector<VagSample> m_vags;
    std::array<uint32_t, 257> m_vagStartOffsets8002E474{};
    uint32_t m_spuAllocationBytes8002E474 = 0u;
    uint32_t m_spuAllocationBase8002E474 = 0u;
    bool m_spuSampleStartAddressesKnown8002E474 = false;
    uint8_t m_masterVolume80032EAC = 0u;
    bool m_loaded = false;
};
