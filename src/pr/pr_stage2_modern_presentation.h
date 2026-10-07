#pragma once
#include "pr_stage2_gpu_device.h"
#include <memory>

struct PrGameContext;

// Optional Windows presentation over the retained Stage2 session. Observers
// only read source state; all modifications apply to copied host commands.
class PrStage2ModernPresentation final : public PrStage2GpuDevice::Presentation {
public:
    PrStage2ModernPresentation(PrGameContext&,PrStage2LifecycleDirect::Services&);
    ~PrStage2ModernPresentation();
    void BeginFrame(uint32_t work,int32_t kind);
    void SetGameplay(bool active);
    void SetTransition(bool active);
    void Tmd(uint32_t packet,uint32_t descriptor,uint32_t object,uint32_t primitive);
    void Rail(uint32_t work,bool begin);
    void Note(uint32_t packet,uint16_t x,uint16_t y,uint16_t type);
    void Portrait(uint32_t packet,uint32_t source,int32_t x,int32_t y);
    void Caption(uint32_t text,uint32_t first,uint32_t end);
    void Status(uint32_t work,int32_t layout,uint32_t first,uint32_t end);
    void Input(uint32_t pad,uint32_t type,int32_t result,uint32_t work);
    void Score(uint32_t work,int32_t flow,int32_t rhyme,int32_t drop,int32_t hype,int32_t total);
    PrStage2OtDrawBackend::Batch Prepare(const PrStage2OtDrawBackend::Batch&,
        PrStage2VramAtlas::Projection&,const PrStage2OtDrawBackend::Viewport&,
        ID3D11ShaderResourceView* base) override;
    bool Present(PrStage2VramAtlas::Projection&,D3D11Renderer&,
        const PrStage2OtDrawBackend::Viewport&) override;
    void Invalidate() override;
    void ConfigureViewport(PrStage2OtDrawBackend::Viewport&) const override;
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
