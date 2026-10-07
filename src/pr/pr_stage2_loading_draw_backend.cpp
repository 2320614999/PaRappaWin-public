#include "pr_stage2_loading_draw_backend.h"
#include <algorithm>
#include <cmath>
#include <numeric>

namespace PrStage2LoadingDrawBackend {
namespace {
struct CaptureSink final : PrStage2LoadingPatternDirect::DrawSink {
    PrStage2LifecycleDirect::Services& memory;
    Frame frame;
    explicit CaptureSink(PrStage2LifecycleDirect::Services& s):memory(s){}
    void Box(uint32_t x,uint32_t y,uint32_t w,uint32_t h,uint32_t color,uint32_t priority) override {
        Command c;c.box=true;c.x=x;c.y=y;c.width=w;c.height=h;c.color=color;
        c.priority=static_cast<uint16_t>(priority);frame.sourceOrder.push_back(c);
    }
    void Sprite(uint32_t x,uint32_t y,uint32_t source,uint32_t priority) override {
        const uint32_t attr=memory.Read32(source);
        // All four original Loading templates use raw, opaque, 4-bit sprites.
        // Do not silently white-modulate unknown RGB-tail variants.
        if(attr!=0x10000040u) throw std::runtime_error("Unsupported Loading sprite attributes");
        const uint16_t tx=memory.Read16(source+4u),ty=memory.Read16(source+6u);
        Command c;c.x=x;c.y=y;c.source=source;c.priority=static_cast<uint16_t>(priority);
        c.width=memory.Read16(source+8u);c.height=memory.Read16(source+10u);
        const uint16_t cx=memory.Read16(source+12u),cy=memory.Read16(source+14u);
        c.tpage=static_cast<uint16_t>(((tx/64u)&15u)|((ty&256u)>>4u));
        c.clut=static_cast<uint16_t>(((cy&511u)<<6u)|((cx>>4u)&63u));
        c.u=static_cast<uint8_t>((tx&63u)*4u);c.v=static_cast<uint8_t>(ty);
        if(!c.width||!c.height||c.width>256u-c.u||c.height>256u-c.v)
            throw std::runtime_error("Unsupported Loading sprite extent");
        frame.sourceOrder.push_back(c);
    }
};
}
Frame Capture(PrStage2LifecycleDirect::Services& memory,uint32_t mode,uint32_t priority) {
    CaptureSink sink(memory);
    PrStage2LoadingPatternDirect::DrawPattern8001EF40(memory,sink,mode,priority);
    return std::move(sink.frame);
}
void Render(const Frame& frame,PrStage2VramAtlas::Projection& atlas,
            D3D11Renderer& renderer,float x,float y,float scale) {
    if(!std::isfinite(x)||!std::isfinite(y)||!std::isfinite(scale)||scale<=0)
        throw std::invalid_argument("Invalid Loading viewport");
    using Lease=Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>;
    std::vector<Lease> textures(frame.sourceOrder.size());
    for(size_t i=0;i<frame.sourceOrder.size();++i) {
        const auto& c=frame.sourceOrder[i];
        if(c.box) continue;
        textures[i]=atlas.Resolve(c.tpage,c.clut,{c.u,c.v,int(c.width),int(c.height)},
                                 renderer,PrStage2VramAtlas::Encoding::Opaque);
        if(!textures[i]) throw std::runtime_error("Loading texture is not present in native VRAM");
    }
    std::vector<size_t> order(frame.sourceOrder.size());std::iota(order.begin(),order.end(),size_t{0});
    std::sort(order.begin(),order.end(),[&](size_t a,size_t b){
        const auto pa=frame.sourceOrder[a].priority,pb=frame.sourceOrder[b].priority;
        return pa!=pb?pa>pb:a>b; // OT traversal + AddPrim head insertion.
    });
    renderer.FlushSprites();
    constexpr size_t index[]={0,1,2,2,1,3};
    for(size_t n:order) {
        const auto& c=frame.sourceOrder[n];
        ColorVertex colors[6];TexturedVertex textured[6];
        for(size_t i=0;i<6u;++i) {
            const size_t corner=index[i];const float dx=float((corner&1u)?c.width:0u),dy=float((corner&2u)?c.height:0u);
            const float px=x+(float(c.x)+dx)*scale,py=y+(float(c.y)+dy)*scale;
            colors[i]={px,py,float((c.color>>16u)&255u)/255.0f,float((c.color>>8u)&255u)/255.0f,
                       float(c.color&255u)/255.0f,(c.color&0x40000000u)?0.5f:1.0f};
            textured[i]={px,py,(float(c.u)+dx)/256.0f,(float(c.v)+dy)/256.0f,1,1,1,1};
        }
        if(c.box) renderer.DrawTriangleBatch(colors,6);
        else renderer.DrawTexturedTriangleBatch(textures[n].Get(),textured,6);
    }
}
}
