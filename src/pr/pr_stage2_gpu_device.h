#pragma once
#include "pr_stage2_ot_draw_backend.h"
#include "pr_stage2_interrupt_device.h"
#include <thread>
#include <vector>

namespace PrStage2GpuDevice {
// Concrete, single-owner DMA2 -> native GP0 -> D3D adapter. Register reads
// observe only. Service() consumes actual RAM and then waits for a D3D event
// query before clearing CHCR or publishing the original DMA interrupt.
// CPU-driven geometry/image FIFOs, interlaced display and VRAM readback are
// intentionally outside this bounded Loading device and are rejected.
enum class Progress { Idle, Blocked, Submitted, Waiting, Completed };
class Presentation {
public:
    virtual ~Presentation()=default;
    virtual PrStage2OtDrawBackend::Batch Prepare(const PrStage2OtDrawBackend::Batch&,
        PrStage2VramAtlas::Projection&,const PrStage2OtDrawBackend::Viewport&,
        ID3D11ShaderResourceView* base)=0;
    // Optional render-only interpolation; native pages and DMA are untouched.
    virtual bool Present(PrStage2VramAtlas::Projection&,D3D11Renderer&,
        const PrStage2OtDrawBackend::Viewport&)=0;
    virtual void Invalidate()=0;
    virtual void ConfigureViewport(PrStage2OtDrawBackend::Viewport&) const {}
};
class Device final {
public:
    Device(PrStage2LifecycleDirect::Services& memory,
           PrStage2VramAtlas::Projection& atlas,
           PrStage2InterruptDevice::Controller& interrupts,
           D3D11Renderer& renderer, PrStage2OtDrawBackend::Viewport viewport,
           bool synchronizeMoviePresent = false);
    bool TryRead(uint32_t address,uint32_t width,uint32_t& result) const;
    bool TryWrite(uint32_t address,uint32_t width,uint32_t value);
    Progress Service();
    // Display the GP1-selected source page through the real swap chain. This
    // does not publish DMA completion or advance the original VBlank counter.
    bool PresentDisplay();
    void SetPresentation(Presentation* value) { presentation_=value; }
    // Native LoadImage output to an existing display page. The caller has
    // already produced these exact 15-bit words and updated source VRAM.
    // This is a host drawing operation, not a new GP0 FIFO/DMA implementation.
    void CopyFramebufferImage(PrStage2LifecycleDirect::ImageRect rect,
                              const std::vector<uint16_t>& words, uint64_t movieFrame = 0,
                              uint16_t movieWidth = 0);
    uint64_t CpuImageCopies()const noexcept{return cpuImageCopies_;}
    uint64_t MoveImageCopies()const noexcept{return moveImageCopies_;}
    uint64_t PresentedCpuImageCopy()const noexcept{return presentedCpuImageCopy_;}
    // Retained immutable source image used by the last successful Present.
    // Reading a DISCARD swap-chain backbuffer after Present is not valid
    // evidence. This shared texture reference never re-renders the scene.
    ComPtr<ID3D11ShaderResourceView> PresentedImage() const;
    // End STR de-duplication when the native decoder stops, before the
    // following transition copies/draws either framebuffer page.
    void ClearMovieIdentity() noexcept;
    // 只读页快照不提交绘图，不改变呈现状态或 DMA 完成信号。
    ComPtr<ID3D11ShaderResourceView> FramebufferImage(uint32_t page) const;
    uint64_t DisplayPresents() const noexcept {return presents_;}
    bool Pending() const noexcept {return (control_&0x01000000u)!=0u;}
    bool Submitted() const noexcept {return submitted_;}
    bool Faulted() const noexcept {return faulted_;}
    uint64_t Serial() const noexcept {return serial_;}
    uint64_t Starts() const noexcept {return starts_;}
    uint64_t Completions() const noexcept {return completions_;}
    uint64_t CommandsRendered() const noexcept {return commands_;}
    uint32_t LastConsumedHead() const noexcept {return lastHead_;}
    uint32_t DisplayAddress() const noexcept {return display_;}
    bool DisplayEnabled() const noexcept {return !displayDisabled_;}
    PrStage2OtDrawBackend::DrawState DrawState() const noexcept {return state_;}
    void SetScanline(uint32_t line,bool field);
private:
    PrStage2OtDrawBackend::Viewport PageViewport(uint32_t page) const;
    struct MovieImageAssembly {
        bool active = false;
        uint32_t page = 0;
        uint64_t frame = 0;
        int x = 0, y = 0, width = 0, height = 0, nextX = 0;
        std::vector<uint32_t> rgba;
    };
    PrStage2LifecycleDirect::Services& memory_;
    PrStage2VramAtlas::Projection& atlas_;
    PrStage2InterruptDevice::Controller& interrupts_;
    D3D11Renderer& renderer_;
    PrStage2OtDrawBackend::Viewport view_;
    bool synchronizeMoviePresent_ = false;
    Presentation* presentation_=nullptr;
    PrStage2OtDrawBackend::DrawState state_{};
    std::thread::id owner_;
    ComPtr<ID3D11Device> device_;
    ComPtr<ID3D11DeviceContext> context_;
    ComPtr<ID3D11Query> fence_;
    std::array<ComPtr<ID3D11ShaderResourceView>,2> pages_{};
    // Reused dynamic source for the assembled STR page. Recreating a dynamic
    // texture for every 15 FPS frame can occasionally block the D3D driver
    // long enough to turn one cadence interval into a 75/59 ms pair.
    ComPtr<ID3D11ShaderResourceView> movieImage_;
    int movieImageWidth_=0,movieImageHeight_=0;
    std::array<bool,2> pageKnown_{};
    std::array<uint64_t,2> pageCpuImageCopy_{};
    std::array<uint64_t,2> pageMovieFrame_{};
    uint64_t presentedMovieFrame_=0;
    uint64_t cpuImageCopies_=0,moveImageCopies_=0,presentedCpuImageCopy_=0;
    ComPtr<ID3D11ShaderResourceView> presented_;
    uint32_t address_=0,count_=0,control_=0,info_=0,display_=0;
    uint32_t horizontal_=0xC00200u,vertical_=0x40010u,mode_=0,direction_=0;
    uint32_t scanline_=0,lastHead_=0;
    bool displayDisabled_=true,field_=false,gpuIrq_=false,submitted_=false,faulted_=false;
    uint64_t serial_=0,starts_=0,completions_=0,commands_=0,presents_=0;
    MovieImageAssembly movieAssembly_{};
    uint64_t RenderPages(const PrStage2OtDrawBackend::Batch& batch);
    // 在 DMA 命令流内执行复制，完成通知仍等待 Service 的真实 D3D fence。
    void MoveFramebuffer(const PrStage2OtDrawBackend::Command& command);
    void FlushMovieImageAssembly();
    uint64_t RenderGeometry(const PrStage2OtDrawBackend::Batch& batch);
    void DrawFramebufferPage(ID3D11ShaderResourceView* page);
    void Owner() const;
    static uint32_t Register(uint32_t address,uint32_t width);
    uint32_t Status() const;
    void Control(uint32_t command);
};
}
