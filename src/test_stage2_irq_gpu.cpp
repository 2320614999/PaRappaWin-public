// External-only scheduled device inputs; no product scene or save backend.
#define main ExistingStage2OtDrawMain
#include "test_stage2_ot_draw.cpp"
#undef main
#include "pr/pr_stage2_irq_direct.h"

namespace {
struct IrqGpuFixture : QueueGpuFixture {
    using QueueGpuFixture::QueueGpuFixture;
    uint16_t nativeMask = 0;
    uint32_t dmaControl = 0, pendingChannels = 0;
    uint32_t delivered = 0, acknowledged = 0, routedCalls = 0;
    bool delivering = false;

    uint16_t Read16(uint32_t address) override {
        return address == 0x1F801074u ? nativeMask : DiskServices::Read16(address);
    }
    void Write16(uint32_t address, uint16_t value) override {
        if (address == 0x1F801074u) { nativeMask = value; return; }
        DiskServices::Write16(address, value);
    }
    uint32_t Read32(uint32_t address) override {
        if (address == 0x1F8010F4u) {
            const bool active = (dmaControl & 0x00800000u) != 0u &&
                (((dmaControl >> 16u) & pendingChannels) != 0u);
            return dmaControl | (pendingChannels << 24u) | (active ? 0x80000000u : 0u);
        }
        if (address == 0x1F8010A8u) {
            if (pending && pumpOnStatus && !pumping && !delivering) CompleteDma();
            return io.at(address);
        }
        return QueueGpuFixture::Read32(address);
    }
    void Write32(uint32_t address, uint32_t value) override {
        if (address == 0x1F8010F4u) {
            // Explicit test register contract: high status bits acknowledge
            // delivered events; low control bits come from the original code.
            const uint32_t cleared = pendingChannels & (value >> 24u);
            if (cleared != 0u) ++acknowledged;
            pendingChannels &= ~(value >> 24u);
            dmaControl = value & 0x00FFFFFFu;
            return;
        }
        QueueGpuFixture::Write32(address, value);
    }
    int32_t Call(uint32_t function, std::initializer_list<uint32_t> args) override {
        int32_t result;
        if (PrStage2IrqDirect::TryDispatch(*this, function, args, result)) {
            ++routedCalls;
            return result;
        }
        return QueueGpuFixture::Call(function, args);
    }
    void InitializeIrq() {
        // The wider reset/BIOS boot remains an explicit precondition. This
        // fixture exercises the new DMA initializer and real callback arrays.
        Write16(0x80055F78u, 1u);
        Write16(0x80055FA8u, 0u);
        Write32(0x80057000u, 0x80056FE0u);
        Write32(0x80056FE8u, 0x80035BA0u);
        const uint32_t target = static_cast<uint32_t>(PrStage2IrqDirect::InitDmaCallbacks80035F7C(*this));
        Check(target == 0x80036150u, "native DMA initializer returned wrong registration entry");
        Write32(0x80056FE4u, target);
        Check(Read32(0x80055F88u) == 0x80035FCCu && nativeMask == 8u,
              "native DMA interrupt vector or mask missing");
        Check(dmaCallback == 0u, "legacy fixture callback must stay unused");
    }
    bool DeliverIrq() {
        if (delivering || (nativeMask & 8u) == 0u || (Read32(0x1F8010F4u) & 0x80000000u) == 0u)
            return false;
        delivering = true;
        const uint32_t vector = Read32(0x80055F88u);
        int32_t result = -1;
        Check(PrStage2IrqDirect::TryDispatch(*this, vector, {}, result) && result == 0,
              "native registered IRQ vector failed");
        ++delivered;
        delivering = false;
        return true;
    }
    Ot::RenderResult CompleteDma() {
        Check(dmaCallback == 0u, "old fixture bypassed native interrupt dispatch");
        const auto result = QueueGpuFixture::PumpOne();
        // Completion is supplied only AFTER actual D3D consumption. Hardware
        // timing is a declared test input, not a translated production clock.
        if ((dmaControl & 0x00040000u) != 0u) pendingChannels |= 4u;
        DeliverIrq();
        return result;
    }
};

void VerifyIrqPixels(IrqGpuFixture& s, int scale, bool maskFirst) {
    Window window; Check(window.hwnd != nullptr, "IRQ GPU window missing");
    D3D11Renderer renderer;
    Check(renderer.Initialize(window.hwnd, 64*scale, 64*scale), "IRQ GPU renderer missing");
    PrStage2VramAtlas::Projection atlas(s.vram);
    renderer.BeginFrame(40/255.0f, 80/255.0f, 120/255.0f);
    const auto initial = ReadFrame(renderer);
    Ot::DrawState state{}; state.clipLeft = state.clipTop = 0;
    state.clipRight = state.clipBottom = 63;
    s.Bind(renderer, atlas, state, {0,0,64,64,0,0,float(64*scale),float(64*scale)});
    s.InitializeIrq();
    constexpr uint32_t colors[3] = {0x600000C8u,0x6000B400u,0x60A00000u};
    constexpr int xs[3] = {2,15,30}, ys[3] = {2,8,20};
    for (uint32_t i=0;i<3u;++i) {
        const uint32_t packet=0x800B7800u+16u*i;
        s.Write32(packet,0x03FFFFFFu); s.Write32(packet+4u,colors[i]);
        s.Write32(packet+8u,Xy(xs[i],ys[i])); s.Write32(packet+12u,Xy(7,5));
        PrStage2GpuDirect::DrawOrderingTable800450A0(s,packet);
    }
    Check(s.starts==1u && s.completions==0u && s.pending && s.dmaCallback==0u,
          "DMA submission rendered or used a legacy completion callback");
    Check(s.Read32(0x80057048u)==0x80046BC4u && s.Read32(0x8005D838u)==2u &&
          s.Read32(0x8005D83Cu)==0u, "native callback registration or queued commands missing");
    Check(ReadFrame(renderer)==initial,"submission painted before completion");
    if (maskFirst) Check(PrStage2IrqDirect::SetInterruptMask800358C0(s,0u)==8,
                         "mask save did not return unsigned old value");
    for (uint32_t completed=1;completed<=3u;++completed) {
        s.CompleteDma();
        if (maskFirst && completed==1u) {
            Check(!s.pending && s.delivered==0u && s.Read32(0x8005D83Cu)==0u && s.pendingChannels==4u,
                  "masked IRQ unexpectedly drained the GPU queue");
            Check(PrStage2IrqDirect::SetInterruptMask800358C0(s,8u)==0 && s.DeliverIrq(),
                  "restored mask did not deliver the pending IRQ");
        }
        Check(s.completions==completed,"DMA completion count mismatch");
        const auto pixels=ReadFrame(renderer);
        for (int y=0;y<64*scale;++y) for (int x=0;x<64*scale;++x) {
            int red=40,green=80,blue=120;
            for (uint32_t i=0;i<completed;++i) if (x/scale>=xs[i] && x/scale<xs[i]+7 &&
                                                   y/scale>=ys[i] && y/scale<ys[i]+5) {
                red=i==0?200:0; green=i==1?180:0; blue=i==2?160:0;
            }
            Near(pixels[size_t(y)*64u*scale+x],red,green,blue,"native IRQ queue pixel mismatch");
        }
    }
    Check(s.delivered==2u && s.acknowledged==2u && s.starts==3u && !s.pending && s.pendingChannels==0u,
          "native IRQ/drain/ack sequence incomplete");
    Check(s.Read32(0x80057048u)==0u && s.Read32(0x8005D838u)==s.Read32(0x8005D83Cu) &&
          (s.dmaControl&0x00040000u)==0u && s.nativeMask==8u && s.routedCalls>0u && s.dmaCallback==0u,
          "final callback removal/mask restoration did not execute natively");
    std::cout<<"irq-gpu "<<scale<<' '<<int(maskFirst)<<" 3 2 "<<12288*scale*scale<<'\n';
}
}
int main(int argc,char** argv) {
    try {
        if(argc!=3)return 1;
        for(int scale:{1,4})for(bool masked:{false,true}) {
            IrqGpuFixture device(argv[1],argv[2]);
            int32_t sentinel=123;
            Check(!PrStage2IrqDirect::TryDispatch(device,0x800FFFFCu,{},sentinel) && sentinel==123,
                  "unknown route manufactured a result");
            bool rejected=false;
            try { PrStage2IrqDirect::TryDispatch(device,0x800358C0u,{},sentinel); }
            catch(const std::invalid_argument&) { rejected=true; }
            Check(rejected,"invalid native route argument count accepted");
            VerifyIrqPixels(device,scale,masked);
        }
        std::cout<<"irq-gpu-pass\n";return 0;
    } catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 2;}
}
