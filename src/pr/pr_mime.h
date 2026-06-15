#pragma once
#include <cstdint>
#include <vector>
#include <string>

#include "pr_tmd.h"

// PSX VDF/DAT/TOD format parser and MIMe animation player
// Based on OpenPara reference + memory.md reverse-engineering

// ============================================================
// VDF: Vertex Deformation Format (per-key vertex delta array)
// ============================================================
struct VdfKeyInfo {
    uint32_t obj;       // which TMD object this key applies to
    uint32_t vertTop;   // starting vertex index in the object
    uint32_t nVert;     // number of vertex deltas in this key
    // followed by nVert SVECTORs (int16_t x,y,z,pad)
};

struct VdfVertexDelta {
    int16_t x, y, z;
    int16_t pad;
};

struct VdfKey {
    uint32_t obj;
    uint32_t vertTop;
    uint32_t nVert;
    std::vector<VdfVertexDelta> deltas;
};

struct VdfData {
    uint32_t keys;
    std::vector<VdfKey> keyList;
};

// ============================================================
// DAT: Influence Timeline (per-key frame curve)
// ============================================================
struct DatKey {
    uint16_t frames = 0;          // number of animation frames
    std::vector<int16_t> influence;
};

struct DatData {
    uint16_t keys = 0;
    uint16_t maxFrames = 0;
    std::vector<DatKey> keyList;
};

// ============================================================
// TOD: Timeline Object Data (block-based transform animation)
// ============================================================
struct TodCommand {
    uint32_t header;     // HIBYTE = len (dwords), HIWORD&0xF = type
    std::vector<uint32_t> data; // remaining dwords (len-1 entries)
};

struct TodBlock {
    uint32_t rawOffset = 0;
    uint32_t rawSize = 0;
    uint16_t unk0;
    uint16_t cmdCount;
    uint32_t triggerTime;
    std::vector<TodCommand> commands;
};

struct TodData {
    uint32_t blockCount = 0;
    uint32_t rawBlockCount = 0;
    std::vector<uint8_t> rawBytes;
    std::vector<TodBlock> blocks;
};

// ============================================================
// MIMe Channel: combined VDF+DAT+timer for one animation layer
// ============================================================
static const int kMaxMimeChannels = 4;

struct MimeChannel {
    bool active = false;
    const VdfData* vdf = nullptr;
    const DatData* dat = nullptr;
    uint32_t startFrame = 0;      // frame when this channel started
    uint32_t speed = 0x100;       // 8.8 fixed-point speed (0x100 = 1x)
    bool loop = false;
};

// PSX 3x3 rotation matrix (fixed-point 4.12) + translation
struct TodCoordMatrix {
    int16_t m[3][3] = {};  // 3x3 rotation, 4096 = 1.0
    int32_t t[3]    = {};  // translation vector
};

// Per-object transform produced by TOD type4 commands
struct TodObjectTransform {
    bool valid = false;
    uint16_t objId = 0;
    TodCoordMatrix matrix;
    // Convenience: extract rotation + translation as floats
    float GetRotY() const;  // approximate Y rotation in radians
    float GetRotZ() const;  // approximate Z rotation in radians
    float GetTx() const { return (float)matrix.t[0]; }
    float GetTy() const { return (float)matrix.t[1]; }
    float GetTz() const { return (float)matrix.t[2]; }
};

// Camera state from TOD type7
struct TodCamera {
    bool valid = false;
    int32_t projDist = 0;    // projection distance (first dword)
    int32_t pos[3] = {};     // camera position (or look-at related)
    int32_t target[3] = {};  // second set of coords
};

// Viewport from TOD type9
struct TodViewport {
    bool valid = false;
    int32_t param0 = 0;  // e.g. near/projDist
    int32_t param1 = 0;
    int32_t param2 = 0;  // e.g. far plane
};

// Object attribute from TOD type0
struct TodObjAttribute {
    bool valid = false;
    uint32_t flags = 0;
    uint32_t color = 0;   // RGBA or similar
};

static const int kMaxTodObjects = 8;

// TOD playback state
struct TodPlayback {
    bool active = false;
    const TodData* tod = nullptr;
    int currentBlock = 0;
    uint32_t frameCounter = 0;
    bool loop = false;           // loop when all blocks consumed

    // Per-object COORD transforms (indexed by slot, objId stored inside)
    TodObjectTransform objects[kMaxTodObjects];
    int objectCount = 0;

    // Camera and viewport
    TodCamera camera;
    TodViewport viewport;

    // Object attributes
    TodObjAttribute objAttrs[kMaxTodObjects];

    // Helper: find transform by objId (nullptr if not found)
    const TodObjectTransform* FindObject(uint16_t objId) const;
    TodObjectTransform* FindOrAddObject(uint16_t objId);

    // Legacy compat: primary object rotation/translation (from obj 1)
    float rotX = 0.0f, rotY = 0.0f, rotZ = 0.0f;
    float tx = 0.0f, ty = 0.0f, tz = 0.0f;
};

// ============================================================
// Parsers
// ============================================================
namespace VdfParser {
    bool Parse(const uint8_t* data, size_t size, VdfData& out);
}

namespace DatParser {
    bool Parse(const uint8_t* data, size_t size, DatData& out);
}

namespace TodParser {
    bool Parse(const uint8_t* data, size_t size, TodData& out);
}

// ============================================================
// MIMe Animation Engine
// ============================================================
namespace MimeEngine {
    // Initialize engine (call once)
    void Init();

    // Load VDF/DAT pairs from resource manager by name
    // Returns index into internal storage, or -1 on failure
    int LoadVdf(const std::string& name, const uint8_t* data, size_t size);
    int LoadDat(const std::string& name, const uint8_t* data, size_t size);
    int LoadTod(const std::string& name, const uint8_t* data, size_t size);

    // Get parsed data by index
    const VdfData* GetVdf(int idx);
    const DatData* GetDat(int idx);
    const TodData* GetTod(int idx);

    // Get parsed data by name
    const VdfData* GetVdfByName(const std::string& name);
    const DatData* GetDatByName(const std::string& name);
    const TodData* GetTodByName(const std::string& name);

    // Channel management
    void SetChannel(int chan, const VdfData* vdf, const DatData* dat,
                    uint32_t currentFrame, uint32_t speed = 0x100, bool loop = false);
    void ClearChannel(int chan);
    void ClearAllChannels();
    MimeChannel& GetChannel(int chan);

    // TOD playback
    void SetTod(const TodData* tod, bool loop = false);
    void ClearTod();
    TodPlayback& GetTodPlayback();
    void TickTod(); // advance one frame

    // Apply MIMe vertex deformation to a vertex buffer
    // baseVerts: original TMD vertices (input)
    // outVerts: deformed vertices (output, must be same size)
    // currentFrame: global frame counter
    // channelMask: bitmask of channels to apply (default 0xF = all 4 channels)
    // subFrame8 is a render-only display phase. Logic remains on integer PSX
    // frames; renderers may use it to blend toward the next DAT influence.
    void ApplyDeformation(const std::vector<TmdVertex>& baseVerts,
                          std::vector<TmdVertex>& outVerts,
                          uint32_t currentFrame,
                          uint32_t channelMask = 0xF,
                          uint32_t subFrame8 = 0);

    // Diagnostic
    std::string DumpInfo();

    // Clear all state
    void Reset();
}
