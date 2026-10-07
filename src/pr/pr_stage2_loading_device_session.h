#pragma once
#include "pr_stage2_loading_session.h"
#include "pr_stage2_frame_task.h"
#include "pr_stage2_frame_wait.h"
#include "pr_stage2_disc_file_device.h"
#include "pr_stage2_gpu_device.h"
#include "pr_stage2_otc_device.h"
#include "pr_stage2_spu_output.h"
#include <chrono>
#include <memory>
#include <vector>

namespace PrStage2LoadingDeviceSession {
struct Unbound : std::runtime_error {
    uint32_t address,width;bool writing;
    Unbound(uint32_t address,uint32_t width=0,bool writing=false);
};
// Native session with concrete graphics/audio devices. The remaining platform
// ABI is explicit: applications must supply BIOS/PAD/etc. dependencies rather
// than inheriting successful test receipts. One retained InitScene is owned.
class Session : public PrStage2LoadingSession::Services, private PrStage2FrameWait::Progress {
public:
    Session(const std::filesystem::path& scus,const std::filesystem::path& overlay,
            const std::filesystem::path& disc,D3D11Renderer& renderer,IAudioSink& sink);
    ~Session();
    PrStage2FrameTask::State StartGraphics();
    PrStage2FrameTask::State StartInitialization();
    // Reuse the same RAM/devices after that initializer has actually returned.
    PrStage2FrameTask::State StartScene();
    // Finish queued GPU work on the same retained device owner before handing
    // its graph/VRAM to the shared resident directory.
    PrStage2FrameTask::State StartGraphicsHandoff();
    int32_t TaskResult()const;
    PrStage2FrameTask::State Poll(bool deliverEvents=true);
    // Publish a VBlank-requested framebuffer from the host pump even when the
    // retained source continuation is not currently yielding. This keeps the
    // presentation clock independent from decoder/CD progress.
    bool PresentIfReady();
    // Product scanout waits for a completed native DMA batch. The lower-level
    // GPU contract keeps its submitted-page behavior for isolated tests.
    void SetProductPresentationRequiresIdleGpu(bool enabled) noexcept {
        productPresentationRequiresIdleGpu_ = enabled;
    }
    PrStage2FrameTask::State State() const;
    PrStage2FrameTask::Request Pending() const;
    void RethrowFailure() const;
    void Cancel();
    uint32_t InitializationStarts() const noexcept{return initStarts_;}
    uint64_t WallClockVBlankEvents() const noexcept{return wallBlanks_;}
    uint64_t NativeIrqEntries() const noexcept{return irqEntries_;}
    uint64_t CompactRailCalls80024744() const noexcept{return compactRailCalls80024744_;}
    uint64_t RailNotesSubmitted80024418() const noexcept{return railNotesSubmitted80024418_;}
    bool NativeIrqActive() const noexcept{return irqActive_;}
    PrStage2DiscFileDevice::Device& Disc() noexcept{return disc_;}
    PrStage2VramDevice::Device& Vram() noexcept{return vram_;}
    PrStage2GpuDevice::Device& Gpu() noexcept{return gpu_;}
    PrStage2OtcDevice::Device& Otc() noexcept{return otc_;}
    PrStage2SpuDevice::Device& Spu() noexcept{return spu_;}
    PrStage2InterruptDevice::Controller& Interrupts() noexcept{return irq_;}
    PrStage2SpuOutput::Stream& Output() noexcept{return output_;}
protected:
    virtual int32_t PlatformDependency(uint32_t function,std::initializer_list<uint32_t> args)=0;
    // Additional concrete device owners participate in the same event pass;
    // completion cannot be manufactured by an MMIO read or a frame counter.
    virtual bool ServiceAdditionalDevices(){return false;}
    virtual bool AdditionalInterruptsEnabled()const{return true;}
    // Runs once on the already-retained startup stack, before InitScene.
    // Concrete owners may execute additional original device initialization.
    virtual void InitializeAdditionalDevices(){}
private:
    using Args=std::initializer_list<uint32_t>;
    PrStage2DiscFileDevice::Device disc_;
    std::filesystem::path discPath_;
    // The ISO normally lives beside the data root, so its parent is not
    // necessarily the directory containing S2/COMPO01 assets.
    std::filesystem::path dataPath_;
    PrStage2VramDevice::Device vram_;
    PrStage2VramAtlas::Projection atlas_;
    PrStage2InterruptDevice::Controller irq_;
    PrStage2OtcDevice::Device otc_;
    PrStage2SpuDevice::Device spu_;
    PrStage2GpuDevice::Device gpu_;
    PrStage2SpuOutput::Stream output_;
    uint64_t compactRailCalls80024744_ = 0;
    uint64_t railNotesSubmitted80024418_ = 0;
    PrPsxGteDirect::MatrixRegisters gte_{};
    std::unique_ptr<PrStage2FrameTask::Task> task_;
    std::chrono::steady_clock::time_point epoch_=std::chrono::steady_clock::now();
    uint64_t timerBase_=0,seenBlank_=0,wallBlanks_=0,irqEntries_=0;
    uint32_t timerMode_=0,initStarts_=0;
    bool graphicsReady_=false,irqActive_=false,presentRequested_=false;
    bool productPresentationRequiresIdleGpu_=false;
    // Product transition handoff has two host edges: accept the submitted
    // final native page, then wait for the next sampled scanout edge before
    // resuming the retained source stack. This never advances PSX VBlank or
    // dispatches an IRQ; it only prevents RunGame from overtaking visibility.
    bool presentationEdgePending_=false;
    uint64_t presentationEdgeTarget_=0;
    bool compactRailAssetsSeeded_=false;
    uint64_t pendingVBlankEvents_=0;
    uint64_t Scanlines() const;
    void Events(bool allowFrameEvents);
    void SeedCompactRailAssets();
    void DispatchIrq();
    void Await(PrStage2FrameTask::WaitKind kind,uint32_t target) override;
protected:
    bool SupportsNullsub801C9728() const override { return true; }
    void WaitDevice(uint32_t address,uint32_t target);
    // Host codec callbacks share the original single owner, but must not
    // reenter an active source IRQ or run through a disabled critical section.
    bool HostCallbacksAllowed() const { return !irqActive_ && AdditionalInterruptsEnabled(); }
    void AwaitDeviceProgress(uint32_t address) override;
    void AwaitPadRelease80035510() override;
    void AwaitTransitionPresentation() override;
    int32_t DrawCompactRail80024744(uint32_t work);
    void DrawRailNote80024418(uint16_t x,uint16_t y,uint16_t slot,uint16_t type) override;
    int32_t CallLoadingDevice(uint32_t function,Args args) override;
    void CallLoadingDeviceVoid(uint32_t function,Args args) override;
    uint8_t ReadDevice8(uint32_t address) override;
    uint16_t ReadDevice16(uint32_t address) override;
    uint32_t ReadDevice32(uint32_t address) override;
    void WriteDevice8(uint32_t address,uint8_t value) override;
    void WriteDevice16(uint32_t address,uint16_t value) override;
    void WriteDevice32(uint32_t address,uint32_t value) override;
    PrStage2LifecycleDirect::Words64 Call64(uint32_t function,Args args) override;
    PrStage2LifecycleDirect::Vector32 NormalizeVector8003A3DC(PrStage2LifecycleDirect::Vector32) override;
    int32_t SetCdLocation800367A4(uint32_t location) override;
    int32_t LoadImage80044D64(PrStage2LifecycleDirect::ImageRect rect,uint32_t source) override;
    PrPsxGteDirect::MatrixRegisters& MatrixGte() override{return gte_;}
    [[noreturn]] void Exit(uint32_t function,Args args) override;
    [[noreturn]] void Break(uint32_t function,uint32_t code) override;
};
}
