#pragma once

#include <cstdint>
#include <initializer_list>
#include <memory>
#include <stdexcept>
#include <vector>
#include "pr_psx_gte_direct.h"

namespace PrStage2LifecycleDirect {

struct Words64 { uint32_t low, high; };
struct Vector32 { uint32_t x, y, z; };
struct Matrix32 { uint32_t words[8]; };
struct ImageRect { uint16_t x, y, width, height; };

// Optional read-only provenance for native TMD packets produced by one frame.
// This metadata never participates in packet allocation, OT linking, or draw
// decisions; it exists only to compare the translated OT with the source.
struct TmdPacketTrace {
    uint32_t packet = 0;
    uint32_t descriptor = 0;
    uint32_t object = 0;
    uint32_t primitive = 0;
    uint32_t primitiveMode = 0;
    uint32_t handler = 0;
    uint32_t ot = 0;
    uint32_t bucket = 0;
    uint32_t otz = 0;
    uint32_t bias = 0;
    uint32_t shift = 0;
    uint32_t length = 0;
};

// Native pointer identity for the original GPU last-command source field.
// A local token owns identity only, never the RECT bytes or a PSX address.
// Pending queues contain their original RAM copies, not these tokens.
struct GpuCommandSource {
    uint32_t address = 0;
    std::shared_ptr<const uint8_t> local;
};

// S2/COMOD2.BIN and its explicitly translated native data/audio helpers.
// Addresses are PSX identities, never host pointers.
// Every Call must execute the actual callee through completion. No default
// success, replay-derived state, or old StageRunner fallback is provided.
// This synchronous source layer is not yet bound to the Windows frame driver.
struct Services {
    virtual ~Services() = default;
    virtual uint8_t Read8(uint32_t address) = 0;
    virtual uint16_t Read16(uint32_t address) = 0;
    virtual uint32_t Read32(uint32_t address) = 0;
    virtual void Write8(uint32_t address, uint8_t value) = 0;
    virtual void Write16(uint32_t address, uint16_t value) = 0;
    virtual void Write32(uint32_t address, uint32_t value) = 0;
    // Called only from a source-level hardware wait loop, never from a plain
    // MMIO read or a nonblocking status query. A coroutine platform may retain
    // this exact activation until real device work progresses; the default
    // synchronous/reference adapter leaves the original polling loop intact.
    virtual void AwaitDeviceProgress(uint32_t /*address*/) {}
    // The original modal dispatcher polls until PAD is released. Yield only
    // to service host input/real interrupts; do not synthesize a frame or pad.
    virtual void AwaitPadRelease80035510() {}
    virtual int32_t Call(uint32_t function,
                         std::initializer_list<uint32_t> arguments) = 0;
    // Product hosts may observe the original gameplay input boundary without
    // replacing the translated call or changing its return value.
    // Read-only Windows presentation observations. Default adapters perform no
    // additional RAM reads, so instruction-diff contracts keep their traces.
    virtual void ObservePresentationFrame(uint32_t /*work*/, int32_t /*kind*/) {}
    virtual void ObserveTmdPacket(uint32_t /*packet*/, uint32_t /*descriptor*/,
                                  uint32_t /*object*/, uint32_t /*primitive*/) {}
    virtual bool WantsTmdPresentation() const { return false; }
    virtual void ObserveRail(uint32_t /*work*/, bool /*begin*/) {}
    virtual void ObserveRailNote(uint32_t /*packet*/, uint16_t /*x*/, uint16_t /*y*/,
                                 uint16_t /*type*/) {}
    virtual void ObservePortrait(uint32_t /*packet*/, uint32_t /*source*/,
                                 int32_t /*x*/, int32_t /*y*/) {}
    virtual void ObserveCaption(uint32_t /*text*/, uint32_t /*first*/, uint32_t /*end*/) {}
    virtual void ObserveStatus(uint32_t /*work*/, int32_t /*layout*/, uint32_t /*first*/, uint32_t /*end*/) {}
    virtual void ObserveScore(uint32_t /*work*/, int32_t /*flow*/, int32_t /*rhyme*/,
                              int32_t /*drop*/, int32_t /*hype*/, int32_t /*total*/) {}
    virtual void ObserveInput80035510(uint32_t /*rawPad*/, uint32_t /*mappedPad*/,
                                      uint32_t /*work*/) {}
    virtual void ObserveJudgeInput80014614(uint32_t /*pad*/, uint32_t /*mappedInput*/,
                                           int32_t /*result*/, uint32_t /*work*/) {}
    // Product hosts may inspect the exact source boundary around a translated
    // transition/game handoff. Reference adapters keep this observational.
    virtual void ObserveTransitionBoundary(uint32_t /*function*/, uint32_t /*stage*/,
                                           uint32_t /*mode*/, uint32_t /*phase*/) {}
    virtual void ObserveGameBoundary(uint32_t /*stage*/, uint32_t /*work*/) {}
    // Observe the bounded pre-loop before the gameplay clock is installed.
    // Extra diagnostic RAM reads belong to the product override, so reference
    // adapters execute only the original source reads and writes.
    virtual void ObserveGamePreLoop(uint32_t /*iteration*/, uint32_t /*work*/,
                                     uint32_t /*ready*/) {}
    // Product hosts may yield once after a completed native transition so the
    // transition's final page becomes visible before the source enters the
    // next scene/game operation. Reference adapters keep the source synchronous.
    virtual void AwaitTransitionPresentation() {}
    // Product hosts may add a second, explicit handoff between a completed
    // transition and 8001A4D0/801C6D58. This is a presentation fence only;
    // reference adapters keep the original pseudo-C call order unchanged.
    virtual void AwaitGameEntryPresentation() {}
    virtual void TransitionPresentationCompleted() {}
    // Windows may hold the next stream's first sector until the pre-loop's
    // reveal has reached scanout. Reference adapters add no source operations.
    virtual void BeginGameStreamStartup() {}
    virtual void GameStartupFramePresented() {}
    // Product hosts may keep the translated frame on the source's static
    // pre-loop branch until the 8001A750/render-only handoff has completed.
    // The reference adapter keeps the original RAM-controlled branch.
    virtual bool AllowGameplayFrame() const { return true; }
    // Product hosts may animate the destination scene below the transition
    // tile mask while the gameplay clock/input boundary is still closed.
    // This is deliberately separate from AllowGameplayFrame(): the original
    // pre-loop can present scene animation without consuming gameplay input.
    virtual bool AllowStartupSceneAnimation() const { return false; }
    // A product presenter may keep the static scene itself off the scanout
    // while the source pre-loop is revealing the transition mask.  The
    // reference adapter still submits the original 801C9B5C draw calls.
    virtual bool AllowStartupSceneDraw() const { return true; }
    virtual void GameStartupPreLoopEnded() {}
    // 80024744 passes a private four-halfword note record. Preserve its values
    // without assigning the C++ temporary a fabricated PSX stack address.
    virtual void DrawRailNote80024418(uint16_t, uint16_t, uint16_t, uint16_t) {
        throw std::logic_error("Native compact rail note output is not bound");
    }
    // The translated 80035510 entry has a different lifetime from the
    // generic 80048A00 PAD snapshot used by the movie/device layer. Product
    // hosts may bind this boundary so queued debug input is consumed by the
    // actual gameplay read instead of an earlier device probe.
    virtual uint32_t ReadPad80035510() {
        CallVoid(0x80048A00u, {});
        return ~Read32(0x800882F0u);
    }
    // Preserve explicitly discarded/void source calls without fabricating a scalar.
    virtual void CallVoid(uint32_t function, std::initializer_list<uint32_t> arguments) {
        (void)Call(function, arguments);
    }
    virtual Words64 Call64(uint32_t function,
                           std::initializer_list<uint32_t> arguments) = 0;
    // Three-word input/output of 8003A3DC. The original temporaries live on
    // the stack; transport their values, never invent persistent PSX addresses.
    virtual Vector32 NormalizeVector8003A3DC(Vector32 input) = 0;
    // Setloc(2, CdlLOC*, nullptr): transport its three input bytes by value.
    // AC18's descriptor is private stack storage, never a fabricated RAM slot.
    virtual int32_t SetCdLocation800367A4(uint32_t packedBcdLocation) = 0;
    // 1AE7C's RECT is private stack storage. Transport its four raw halfwords,
    // not a fabricated PSX pointer. The device must actually upload the data.
    virtual int32_t LoadImage80044D64(ImageRect rect, uint32_t source) = 0;
    // Default transport retains the public-RAM ABI. Native private images
    // require ImageSourceServices, whose raw source-slot access fails closed.
    virtual bool SupportsPrivateGpuSources() const { return false; }
    virtual GpuCommandSource ReadGpuCommandSource() {
        return {Read32(0x8005D82Cu), {}};
    }
    virtual void WriteGpuCommandSource(const GpuCommandSource& source) {
        if (source.local) throw std::logic_error("Private GPU source transport is not bound");
        Write32(0x8005D82Cu, source.address);
    }
    virtual int32_t CallWithGpuCommandSource(uint32_t function, uint32_t format,
        uint32_t command, const GpuCommandSource& source, uint32_t argument) {
        if (source.local) throw std::logic_error("Native GPU source printf ABI is not bound");
        return Call(function, {format, command, source.address, argument});
    }
    // The pointer-bearing argument is a live native reference. An alternate
    // driver must consume/copy it before return, just as with the original stack.
    virtual int32_t CallWithPrivateImage(uint32_t, ImageRect&,
        const GpuCommandSource&, std::initializer_list<uint32_t>,
        std::initializer_list<uint32_t>) {
        throw std::logic_error("Native private-image callee ABI is not bound");
    }
    // The adapter supplies shared register storage, not a whole-function
    // matrix receipt. Original cache walks, matrix arithmetic and GTE loads
    // execute below and leave their register side effects in this state.
    virtual PrPsxGteDirect::MatrixRegisters& MatrixGte() = 0;
    // Original BIOS exit is non-returning. A host adapter must unwind/terminate
    // this execution, never continue with zero packet addresses.
    [[noreturn]] virtual void Exit(uint32_t function,
                                   std::initializer_list<uint32_t> arguments) = 0;
    // Preserve an original MIPS BREAK rather than invoking C++ division UB.
    [[noreturn]] virtual void Break(uint32_t instruction, uint32_t code) = 0;
};

int32_t InitGlobals801C97EC(Services& s);
void LoadRotation8003F6B0(Services& s, uint32_t matrix);
void LoadTranslation8003F6E0(Services& s, uint32_t matrix);
void LoadGteMatrix80040544(Services& s, uint32_t matrix);
int32_t ApplyMatrixLongVector8003ABA4(Services& s, uint32_t matrix, uint32_t vector, uint32_t output);
int32_t MultiplyMatrixLeft80040884(Services& s, uint32_t left, uint32_t right);
int32_t MultiplyMatrixRight80040994(Services& s, uint32_t left, uint32_t right);
int32_t ComposeMatrixLeft8004075C(Services& s, uint32_t left, uint32_t right);
int32_t ComposeMatrixRight800406D8(Services& s, uint32_t left, uint32_t right);
int32_t ResolveWorldMatrix80041A68(Services& s, uint32_t coordinate, uint32_t output);
int32_t ResolveCoordinateMatrix800417A4(Services&,uint32_t coordinate,uint32_t output);
int32_t CameraMagnitude80041464(Services&,uint32_t source);
int32_t PositiveBitLength8004152C(Services&,uint32_t value);
int32_t ScaleCameraPoints80041374(Services&,uint32_t source,uint32_t output);
int32_t CameraSquareRoot80041548(Services&,uint32_t value);
int32_t TransposeCameraMatrix800415D8(Services&,uint32_t source,uint32_t output);
int32_t BuildCameraAxis800416E0(Services&,uint32_t output,uint32_t sine,uint32_t cosine,uint32_t axis);
int32_t CameraSine8003A1D0(Services&,uint32_t angle);
int32_t CameraCosine8003A29C(Services&,uint32_t angle);
int32_t RollCamera80041628(Services&,uint32_t matrix,uint32_t angle);
int32_t SetCamera80040FA0(Services&,uint32_t view);
int32_t MapModelData80040BFC(Services& s, uint32_t flagsAddress);
int32_t GroupModelPrimitives8004274C(Services& s, uint32_t table,
    uint32_t descriptor, uint32_t index);
int32_t BindModel8001AF1C(Services& s, uint32_t source,
    uint32_t descriptor, uint32_t parent);
int32_t InitAnimationCursor8001AFD8(Services& s, uint32_t source,
    uint32_t cursorAddress, uint32_t countAddress);
int32_t InitCoordinate8004049C(Services& s, uint32_t parent, uint32_t coordinate);
int32_t RotateMatrixY8003B3CC(Services& s, uint32_t angle, uint32_t matrix);
int32_t RotateMatrixX8003B22C(Services& s, uint32_t angle, uint32_t matrix);
int32_t RotateMatrixZ8003B56C(Services& s, uint32_t angle, uint32_t matrix);
int32_t ScaleMatrix8003B0FC(Services& s, uint32_t matrix, uint32_t scale);
int32_t TranslateMatrix8003B0CC(Services& s, uint32_t matrix, uint32_t translation);
int32_t ApplyTodCommand80028054(Services& s, uint32_t command, uint32_t descriptor);
int32_t ApplyAnimationBlock80028504(Services& s, uint32_t frame, uint32_t cursor,
    uint32_t descriptor, uint32_t mode);
int32_t AdvanceAnimation8001B000(Services& s, uint32_t frame,
    uint32_t countAddress, uint32_t cursorAddress, uint32_t descriptor);
// Same original body with 9E18's stack-local count; no fabricated PSX address.
int32_t AdvanceAnimationLocalCount8001B000(Services& s, uint32_t frame,
    uint32_t& count, uint32_t cursorAddress, uint32_t descriptor);
int32_t ResetHeapPointers80025A00(Services& s);
int32_t ResetResourceHeap80025A34(Services& s);
int32_t PushResourceHeap80025A70(Services& s, uint32_t bytes);
int32_t PopResourceHeap80025AF8(Services& s);
int32_t AllocatePacketHeap80025B28(Services& s, uint32_t bytes);
void BindDrawBuffers8001E33C(Services& s, uint32_t first, uint32_t second);
int32_t FillDrawFlags8001EEAC(Services& s, uint32_t value);
int32_t ResetDrawFlags8001EEE8(Services& s);
int32_t ClearOrderingTable80040CC8(Services& s, uint32_t offset, uint32_t point, uint32_t table);
int32_t SubmitOrderingTable80040CA4(Services& s, uint32_t table);
int32_t ClearWorkOrderingTable8001E374(Services& s, uint32_t buffer);
int32_t SubmitWorkOrderingTable8001E3B0(Services& s, uint32_t buffer);
int32_t LinkPrimitive8004401C(Services& s, uint32_t tag, uint32_t primitive);
int32_t EnqueueClearPrimitive80040060(Services& s, uint32_t red, uint32_t green,
    uint32_t blue, uint32_t table);
int32_t ClearEntries801C78D4(Services& s);
int32_t ConfigureText801CB244(Services& s);
int32_t InitScene801C657C(Services& s, uint32_t sceneEntry, int32_t scene);
int32_t InitMovie801C6A3C(Services& s, uint32_t record, int32_t mode);
int32_t UpdateMovieClock801C66C8(Services& s, uint32_t record, uint32_t work);
int32_t MovieFrame801C6804(Services& s, uint32_t record, uint32_t work);
int32_t AllocatePackets801C9A00(Services& s);
int32_t ConfigureModel801C9A64(Services& s, uint32_t index, uint32_t flag, uint32_t descriptor);
int32_t InitResources801CB284(Services& s);
// Exact runtime TIM loop blocks extracted from these two original callers.
// These are not new original functions or complete init/frame entry points.
void UploadInitialRuntimeTims801CB284(Services& s);
void UploadFrameRuntimeTims801CA57C(Services& s, uint32_t work);
int32_t Present801CB170(Services& s);
void ResetTmdPacketTrace();
const std::vector<TmdPacketTrace>& GetTmdPacketTrace();
// S2 consumes only these routines' drawing effects. In particular, 1B084's
// count <= 0 path leaves V0 undefined; do not fabricate a success result.
void DrawModels8001B084(Services& s, uint32_t descriptor,
    int32_t count, uint32_t ot, int32_t depth);
void SubmitModel800428B0(Services& s, uint32_t descriptor,
    uint32_t ot, uint32_t depthShift, uint32_t scratch);
int32_t DrawNf3_8003B88C(Services& s, uint32_t primitive, uint32_t vertices,
    uint32_t packet, uint32_t count, uint32_t shift, uint32_t ot);
int32_t DrawTnf3_8003CFDC(Services& s, uint32_t primitive, uint32_t vertices,
    uint32_t packet, uint32_t count, uint32_t shift, uint32_t ot);
int32_t DrawNf4_8003BD9C(Services& s, uint32_t primitive, uint32_t vertices,
    uint32_t packet, uint32_t count, uint32_t shift, uint32_t ot);
int32_t DrawTnf4_8003D58C(Services& s, uint32_t primitive, uint32_t vertices,
    uint32_t packet, uint32_t count, uint32_t shift, uint32_t ot);
int32_t DrawNg3_8003C36C(Services& s, uint32_t primitive, uint32_t vertices,
    uint32_t packet, uint32_t count, uint32_t shift, uint32_t ot);
int32_t DrawTng3_8003DC2C(Services& s, uint32_t primitive, uint32_t vertices,
    uint32_t packet, uint32_t count, uint32_t shift, uint32_t ot);
int32_t DrawModelsAfterFirst801C9ABC(Services& s, uint32_t descriptor,
                                    int32_t count, uint32_t ot, int32_t depth);
void DrawPreparedScene801C9B5C(Services& s);
void AnimateAndDrawScene801C9E18(Services& s);
void PrepareFrame801CA57C(Services& s, uint32_t work, int32_t kind);
int32_t GameClock801C6858(Services& s);
int32_t ResetEventTimeline801C7884(Services& s, uint32_t work, uint32_t eventId);
int32_t PublishEventText801C78FC(Services& s, uint32_t work, uint32_t event);
int32_t AdvanceResourceQueue801C7A24(Services& s, uint32_t work, uint32_t lane);
int32_t RestoreSecondResourcePair801C7B20(Services& s, uint32_t work);
int32_t RestoreFirstResourcePair801C7BC0(Services& s, uint32_t work);
int32_t DispatchEventResources801C7C54(Services& s, uint32_t work, uint32_t event);
int32_t UpdateEvents801C870C(Services& s, uint32_t work);
int32_t ApplyInputRow801C85CC(Services& s, uint32_t work, uint32_t row);
int32_t DecodeInput80024B54(uint32_t pad);
int32_t IsJudgeBlocked80024BF4(Services& s, uint32_t work);
int32_t JudgeInput80014614(Services& s, uint32_t work);
int32_t CopyBytes80025C64(Services& s, uint32_t source, uint32_t destination, int32_t count);
int32_t SelectAudioBank8002F13C(Services& s, uint32_t bank, uint32_t program);
int32_t AllocateVoice80030544(Services& s);
int32_t PrepareVoiceRegisters80030C90(Services& s);
int32_t ComputePitch800315C8(Services& s, uint32_t key, uint32_t fine);
int32_t SetRegularVoice800307AC(Services& s, uint32_t mode, uint32_t pitch);
int32_t SetNoiseVoice80030EA4(Services& s, uint32_t voice);
int32_t UpdateRegisterMask8003540C(Services& s, uint32_t mode, uint32_t mask,
    uint32_t lowRegister, uint32_t highRegister);
int32_t SetNoiseMask800353E8(Services& s, uint32_t mode, uint32_t mask);
int32_t AdvanceVolumeRamp80031A28(Services& s, uint32_t voice);
int32_t AdvancePanRamp80031F28(Services& s, uint32_t voice);
int32_t CommitAudio80032B00(Services& s);
int32_t StopVoice800345E4(Services& s, uint32_t voice, uint32_t bank,
    uint32_t program, uint32_t note, uint32_t key);
int32_t PlayVoice80034240(Services& s, uint32_t bank, uint32_t program,
    uint32_t note, uint32_t key, uint32_t fine, uint32_t left, uint32_t right);
void ReplaceCue80026FC4(Services& s, uint32_t row);
int32_t PlayCue80026EF8(Services& s, uint32_t row);
int32_t ResetVoices800351B8(Services& s);
int32_t ResetAudio80026FA4(Services& s);
int32_t FlushAudioDriver8002EFF4(Services& s);
int32_t FlushAudio80026ECC(Services& s);
int32_t InputFeedback801C9644(Services& s, uint32_t work);
int32_t SpecialPose801C9730(Services& s, uint32_t work);
int32_t InitDemo801C7958(Services& s);
int32_t InitGame801C6CDC(Services& s, uint32_t record, uint32_t work);
int32_t RunGame801C6D58(Services& s, uint32_t record, uint32_t work, int32_t scene);
int32_t RunMovie801C6AB8(Services& s, uint32_t record, uint32_t work, uint32_t kind);
int32_t RunScene801C74E4(Services& s, int32_t scene);

} // namespace PrStage2LifecycleDirect
