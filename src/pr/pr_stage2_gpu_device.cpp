#include "pr_stage2_gpu_device.h"
#include <stdexcept>
#include <string>
#include "logger.h"

namespace PrStage2GpuDevice {
namespace {
[[noreturn]] void Fail(const char* text){throw std::runtime_error(std::string("S2 GPU device: ")+text);}
void Hresult(HRESULT result,const char* text){if(FAILED(result))Fail(text);}
}
Device::Device(PrStage2LifecycleDirect::Services& memory,PrStage2VramAtlas::Projection& atlas,
               PrStage2InterruptDevice::Controller& interrupts,D3D11Renderer& renderer,
               PrStage2OtDrawBackend::Viewport view,bool synchronizeMoviePresent)
    :memory_(memory),atlas_(atlas),interrupts_(interrupts),renderer_(renderer),view_(view),
     synchronizeMoviePresent_(synchronizeMoviePresent),owner_(std::this_thread::get_id()){
    if(view.frameWidth<=0||view.frameHeight<=0||(view.frameHeight&1)||view.width<=0||view.height<=0)Fail("invalid two-page framebuffer projection");
    const uint32_t pixel=0;
    ComPtr<ID3D11ShaderResourceView> texture;texture.Attach(renderer_.CreateTexture(&pixel,1,1));
    if(!texture)Fail("D3D device unavailable");
    texture->GetDevice(device_.GetAddressOf());device_->GetImmediateContext(context_.GetAddressOf());
    if(!context_)Fail("D3D immediate context unavailable");
}
void Device::Owner()const{if(owner_!=std::this_thread::get_id())throw std::logic_error("S2 GPU accessed from a different execution owner");}
uint32_t Device::Register(uint32_t address,uint32_t width){
    const uint32_t segment=address&0xE0000000u;
    if(segment!=0u&&segment!=0x80000000u&&segment!=0xA0000000u)return 0;
    const uint32_t a=address&0x1FFFFFFFu;
    if(!((a>=0x1F8010A0u&&a<0x1F8010ACu)||(a>=0x1F801810u&&a<0x1F801818u)))return 0;
    if(width!=4u||(a&3u))throw std::invalid_argument("S2 GPU requires aligned word transactions");
    return a;
}
uint32_t Device::Status()const{
    uint32_t result=(state_.tpage&0x1FFu)|(state_.drawModeFlags&0x600u)|0x2000u;
    result|=(mode_&0x40u)<<10u;result|=(mode_&3u)<<17u;
    result|=(mode_&0x3Cu)<<17u;
    if(displayDisabled_)result|=0x00800000u;
    if(gpuIrq_)result|=0x01000000u;
    // Backpressure is tied to this native consumer's actual capacity. This is
    // not an invented cycle-accurate PSX raster/FIFO timing model.
    const bool ready=!submitted_&&!faulted_;
    if(ready)result|=0x14000000u;
    if(ready&&(direction_==1u||direction_==2u))result|=0x02000000u;
    result|=direction_<<29u;
    if(scanline_<240u&&(scanline_&1u))result|=0x80000000u;
    return result;
}
bool Device::TryRead(uint32_t address,uint32_t width,uint32_t& result)const{
    const uint32_t reg=Register(address,width);if(!reg)return false;Owner();
    switch(reg){
    case 0x1F8010A0u:result=address_;break;
    case 0x1F8010A4u:result=count_;break;
    case 0x1F8010A8u:result=control_;break;
    case 0x1F801810u:result=info_;break;
    case 0x1F801814u:result=Status();break;
    default:return false;
    }
    return true;
}
void Device::Control(uint32_t command){
    const uint32_t operation=(command>>24u)&63u;
    switch(operation){
    case 0:
        if(Pending())Fail("reset requires the transfer owner to finish or cancel DMA first");
        state_={};info_=display_=mode_=direction_=0;horizontal_=0xC00200u;vertical_=0x40010u;
        displayDisabled_=true;gpuIrq_=false;interrupts_.SetLine(PrStage2InterruptDevice::Source::Gpu,false);break;
    case 1:
        if(Pending())Fail("cannot reset an in-flight command consumer");
        break;
    case 2:gpuIrq_=false;interrupts_.SetLine(PrStage2InterruptDevice::Source::Gpu,false);break;
    case 3:displayDisabled_=(command&1u)!=0u;break;
    case 4:
        if((command&3u)==3u)Fail("VRAM-to-CPU DMA is not bound");
        direction_=command&3u;break;
    case 5:display_=command&0x7FFFFu;break;
    case 6:horizontal_=command&0xFFFFFFu;break;
    case 7:vertical_=command&0xFFFFFu;break;
    case 8:
        if(command&0xB8u)Fail("PAL/interlaced/24-bit/reversed display is not bound");
        mode_=command&255u;break;
    case 9:if(command&1u)Fail("2 MiB GPU mode is not supported");break;
    default:
        if(operation<0x10u||operation>0x1Fu)Fail("unbound GP1 command");
        switch(command&15u){
        case 2:info_=0;break; // Nonzero texture windows are rejected by GP0 decoder.
        case 3:info_=(uint32_t(state_.clipLeft)&1023u)|((uint32_t(state_.clipTop)&511u)<<10u);break;
        case 4:info_=(uint32_t(state_.clipRight)&1023u)|((uint32_t(state_.clipBottom)&511u)<<10u);break;
        case 5:info_=(uint32_t(state_.offsetX)&2047u)|((uint32_t(state_.offsetY)&2047u)<<11u);break;
        case 7:info_=2u;break; // Explicit standard v2 GPU contract.
        case 8:info_=0u;break;
        default:break; // Hardware retains the GPUREAD latch for these queries.
        }
    }
}
bool Device::TryWrite(uint32_t address,uint32_t width,uint32_t value){
    const uint32_t reg=Register(address,width);if(!reg)return false;Owner();
    switch(reg){
    case 0x1F8010A0u:
        if(Pending())Fail("active DMA address overwritten");address_=value&0xFFFFFFu;break;
    case 0x1F8010A4u:
        if(Pending())Fail("active DMA count overwritten");count_=value;break;
    case 0x1F8010A8u:
        if(!(value&0x01000000u)){
            if(submitted_)Fail("cannot cancel after D3D submission without observing its fence");
            control_=value;faulted_=false;fence_.Reset();++serial_;break;
        }
        if(Pending())Fail("DMA reentered while active");
        if(value!=0x01000401u||count_!=0u)Fail("only source linked-list GPU DMA is bound");
        if((address_&3u)||address_>0x1FFFFCu)Fail("DMA head is outside aligned main RAM");
        control_=value;submitted_=false;faulted_=false;++starts_;++serial_;break;
    case 0x1F801814u:Control(value);break;
    case 0x1F801810u:{
        if(Pending())Fail("GP0 port write during active native DMA");
        const uint32_t op=value>>24u;
        if(op==0u||op==1u)break; // NOP/cache clear; atlas resolves from known VRAM versions.
        if(op<0xE1u||op>0xE6u)Fail("CPU geometry/image command FIFO is not bound");
        const auto batch=PrStage2OtDrawBackend::DecodeCommandStream({value},state_);
        if(!batch.commands.empty())Fail("state command unexpectedly produced geometry");
        state_=batch.finalState;break;
    }
    default:return false;
    }
    return true;
}
Progress Device::Service(){
    Owner();if(faulted_)Fail("transfer is faulted; no completion may be fabricated");
    if(!Pending())return Progress::Idle;
    try{
        if(!submitted_){
            uint32_t priority=0;
            if(!interrupts_.TryRead(0x1F8010F0u,4u,priority))Fail("DMA controller unavailable");
            if(!(priority&0x800u)||direction_!=2u)return Progress::Blocked;
            auto batch=PrStage2OtDrawBackend::DecodeLinkedList(memory_,address_,state_);
            D3D11_QUERY_DESC description{};description.Query=D3D11_QUERY_EVENT;
            Hresult(device_->CreateQuery(&description,fence_.ReleaseAndGetAddressOf()),"D3D fence creation failed");
            const uint64_t drawn=RenderPages(batch);
            renderer_.FlushSprites();
            context_->End(fence_.Get());context_->Flush();
            Hresult(device_->GetDeviceRemovedReason(),"D3D device removed during submission");
            state_=batch.finalState;lastHead_=address_;commands_+=drawn;submitted_=true;
            return Progress::Submitted; // Submission is deliberately NOT completion.
        }
        BOOL complete=FALSE;
        // Permit the driver to flush deferred query work. DONOTFLUSH can
        // indefinitely leave an event pending on a quiet command stream.
        // This is still a real completion query, never a timer-based receipt.
        const HRESULT status=context_->GetData(fence_.Get(),&complete,sizeof(complete),0u);
        if(status==S_FALSE)return Progress::Waiting;
        Hresult(status,"D3D fence query failed");
        Hresult(device_->GetDeviceRemovedReason(),"D3D device removed while waiting");
        if(!complete)return Progress::Waiting;
        // All original RAM decode and actual D3D operations preceded this
        // signal. Native IRQ dispatch remains the scene owner's responsibility.
        address_=0xFFFFFFu;control_&=~0x01000000u;submitted_=false;fence_.Reset();++completions_;
        interrupts_.NotifyDma(2u,PrStage2InterruptDevice::DmaBoundary::TransferComplete);
        return Progress::Completed;
    }catch(...){faulted_=true;throw;}
}
// 原始链表顺序是绘图/复制共同的顺序，不能先画完两页再集中复制。
uint64_t Device::RenderPages(const PrStage2OtDrawBackend::Batch& batch){
    uint64_t drawn=0;
    PrStage2OtDrawBackend::Batch geometry;
    for(const auto& command:batch.commands){
        if((command.opcode&0xE0u)==0x80u){
            drawn+=RenderGeometry(geometry);geometry.commands.clear();
            if(presentation_) presentation_->Invalidate();
            MoveFramebuffer(command);++drawn;
        }else geometry.commands.push_back(command);
    }
    return drawn+RenderGeometry(geometry);
}
void Device::DrawFramebufferPage(ID3D11ShaderResourceView* page){
    // The page already contains composited PSX RGB. Its D3D alpha is a byproduct
    // of the last blend, not opacity to apply again on the next OT or Present.
    const float w=float(renderer_.GetWidth()),h=float(renderer_.GetHeight());
    const TexturedVertex vertices[]={{0,0,0,0,1,1,1,1},{w,0,1,0,1,1,1,1},
        {0,h,0,1,1,1,1,1},{0,h,0,1,1,1,1,1},{w,0,1,0,1,1,1,1},{w,h,1,1,1,1,1,1}};
    renderer_.DrawTexturedTriangleBatch(page,vertices,6);
}
PrStage2OtDrawBackend::Viewport Device::PageViewport(uint32_t page) const {
    auto view=view_;view.frameHeight/=2;view.frameY+=int(page)*view.frameHeight;
    if(presentation_)presentation_->ConfigureViewport(view);
    return view;
}
uint64_t Device::RenderGeometry(const PrStage2OtDrawBackend::Batch& batch){
    if(batch.commands.empty())return 0;
    uint64_t drawn=0;
    for(uint32_t page=0;page<2u;++page){
        auto view=PageViewport(page);
        if(!pageKnown_[page]){
            const auto& first=batch.commands.front();const auto& xy=first.vertex[0];
            if(first.opcode!=2u||xy.x>view.frameX||xy.y>view.frameY||
               int(xy.x)+first.fillWidth<view.frameX+view.frameWidth||int(xy.y)+first.fillHeight<view.frameY+view.frameHeight)
                Fail("framebuffer page has no original full-clear producer");
        }
        renderer_.BeginFrame(0,0,0);
        if(pageKnown_[page]){
            DrawFramebufferPage(pages_[page].Get());
            renderer_.FlushSprites();
        }
        const auto enhanced=presentation_?presentation_->Prepare(batch,atlas_,view,pages_[page].Get()):PrStage2OtDrawBackend::Batch{};
        const auto result=PrStage2OtDrawBackend::Render(presentation_?enhanced:batch,atlas_,renderer_,view);
        drawn+=result.commands;
        ID3D11ShaderResourceView* captured=nullptr;
        if(!renderer_.CaptureFrameTexture(captured)||!captured)Fail("framebuffer page capture failed");
        pages_[page].Attach(captured);pageKnown_[page]=true;
    }
    return drawn;
}
void Device::FlushMovieImageAssembly(){
    if(!movieAssembly_.active)return;
    if(presentation_) presentation_->Invalidate();
    Owner();
    const uint32_t page=movieAssembly_.page;
    const int pageHeight=view_.frameHeight/2;
    if(page>=2u||!pageKnown_[page]||movieAssembly_.nextX!=movieAssembly_.x+movieAssembly_.width||
       movieAssembly_.rgba.size()!=size_t(movieAssembly_.width)*size_t(movieAssembly_.height))
        Fail("Native movie image assembly is incomplete");
    if(movieImageWidth_!=movieAssembly_.width||movieImageHeight_!=movieAssembly_.height)
        movieImage_.Reset();
    if(!movieImage_) {
        movieImage_.Attach(renderer_.CreateTexture(movieAssembly_.rgba.data(),
                                                    movieAssembly_.width,movieAssembly_.height));
        if(!movieImage_)Fail("Native movie image texture creation failed");
        movieImageWidth_=movieAssembly_.width;movieImageHeight_=movieAssembly_.height;
    } else if(!renderer_.TryUpdateTexture(movieImage_.Get(), movieAssembly_.rgba.data(),
                                          movieAssembly_.width, movieAssembly_.height)) {
        // A lost/recreated device or a changed projection invalidates the
        // dynamic resource. Recreate it before submitting this frame; the
        // normal device-removal path still fails closed below.
        movieImage_.Reset();
        movieImage_.Attach(renderer_.CreateTexture(movieAssembly_.rgba.data(),
                                                    movieAssembly_.width,movieAssembly_.height));
        if(!movieImage_)Fail("Native movie image texture refresh failed");
    }
    renderer_.BeginFrame(0,0,0);
    DrawFramebufferPage(pages_[page].Get());
    renderer_.FlushSprites();
    const auto view=PageViewport(page);
    const float sx=view.width/float(view.frameWidth),sy=view.height/float(view.frameHeight);
    renderer_.DrawSprite(movieImage_.Get(),view.x+movieAssembly_.x*sx,view.y+movieAssembly_.y*sy,
                         movieAssembly_.width*sx,movieAssembly_.height*sy);
    renderer_.FlushSprites();
    // Reuse the page snapshot allocated by the first producer. CaptureFrame
    // copies into an existing matching resource and therefore avoids a second
    // per-frame allocation while preserving the immutable page identity used
    // by PresentDisplay().
    ID3D11ShaderResourceView* captured=pages_[page].Get();
    if(!renderer_.CaptureFrameTexture(captured)||!captured)Fail("Native movie image page capture failed");
    if(captured!=pages_[page].Get()) pages_[page].Attach(captured);
    pageCpuImageCopy_[page]=++cpuImageCopies_;
    pageMovieFrame_[page]=movieAssembly_.frame;
    movieAssembly_={};
}

void Device::CopyFramebufferImage(PrStage2LifecycleDirect::ImageRect rect,const std::vector<uint16_t>& words,uint64_t movieFrame,uint16_t movieWidth){
    Owner();
    if(faulted_ || (Pending()&&!submitted_))Fail("CPU image cannot overtake unconsumed source drawing");
    const int pageHeight=view_.frameHeight/2;
    const int x=int(rect.x)-view_.frameX,y=int(rect.y)-view_.frameY;
    if(!rect.width||!rect.height||x<0||y<0||uint64_t(rect.width)*rect.height!=words.size()||
       x+int(rect.width)>view_.frameWidth||y+int(rect.height)>view_.frameHeight)
        throw std::invalid_argument("Native framebuffer image rectangle/word count is invalid");
    const uint32_t page=uint32_t(y/pageHeight);
    if(page>=2u || (y%pageHeight)+int(rect.height)>pageHeight || !pageKnown_[page])
        throw std::invalid_argument("Native image must belong to one already-produced display page");

    // The translated blitter emits one 16-pixel strip at a time. Creating a
    // texture and capturing the full page for every strip eventually stalls
    // the immediate context and starves the real CD ring. Assemble the full
    // movie page in host memory and do one texture/capture pair per frame.
    // Use the decoder's actual width. MOVIE2 is 256x144 inside the 320x240
    // page; requiring a full-height strip left that product path allocating
    // and capturing a whole framebuffer for every one of its 16 strips.
    const bool movieStrip = movieFrame != 0u && movieWidth != 0u && rect.width == 16u;
    if(movieStrip){
        if((movieWidth&15u)||movieWidth>view_.frameWidth)
            throw std::invalid_argument("Native movie width is not whole strips within the page");
        if(!movieAssembly_.active || movieAssembly_.frame != movieFrame ||
           movieAssembly_.page != page){
            FlushMovieImageAssembly();
            if(x+movieWidth>view_.frameWidth)
                throw std::invalid_argument("Native movie exceeds its display page");
            movieAssembly_.active=true;
            movieAssembly_.page=page;
            movieAssembly_.frame=movieFrame;
            movieAssembly_.x=x;movieAssembly_.y=y%pageHeight;
            movieAssembly_.width=movieWidth;movieAssembly_.height=rect.height;
            movieAssembly_.nextX=x;
            movieAssembly_.rgba.assign(size_t(movieWidth)*size_t(rect.height),0xFF000000u);
        }
        if(x!=movieAssembly_.nextX||y%pageHeight!=movieAssembly_.y||
           movieWidth!=movieAssembly_.width||rect.height!=movieAssembly_.height||
           x+16>movieAssembly_.x+movieAssembly_.width)
            throw std::invalid_argument("Native movie strips changed geometry or order");
        for(int row=0;row<rect.height;++row){
            uint32_t* dst=movieAssembly_.rgba.data()+size_t(row)*size_t(movieWidth)+size_t(x-movieAssembly_.x);
            for(int col=0;col<16;++col){
                const uint16_t word=words[size_t(row)*16u+size_t(col)];
                const uint32_t r=word&31u,g=(word>>5u)&31u,b=(word>>10u)&31u;
                dst[col]=((r<<3u)|(r>>2u))|
                    (((g<<3u)|(g>>2u))<<8u)|
                    (((b<<3u)|(b>>2u))<<16u)|0xFF000000u;
            }
        }
        movieAssembly_.nextX+=16;
        if(movieAssembly_.nextX==movieAssembly_.x+movieAssembly_.width)FlushMovieImageAssembly();
        return;
    }

    FlushMovieImageAssembly();
    std::vector<uint32_t> rgba;rgba.reserve(words.size());
    for(uint16_t word:words){
        const uint32_t r=word&31u,g=(word>>5u)&31u,b=(word>>10u)&31u;
        rgba.push_back(((r<<3u)|(r>>2u))|(((g<<3u)|(g>>2u))<<8u)|(((b<<3u)|(b>>2u))<<16u)|0xFF000000u);
    }
    ComPtr<ID3D11ShaderResourceView> image;
    image.Attach(renderer_.CreateTexture(rgba.data(),int(rect.width),int(rect.height)));
    if(!image)Fail("Native image texture creation failed");
    renderer_.BeginFrame(0,0,0);
    DrawFramebufferPage(pages_[page].Get());
    renderer_.FlushSprites();
    const auto view=PageViewport(page);
    const float sx=view.width/float(view.frameWidth),sy=view.height/float(view.frameHeight);
    renderer_.DrawSprite(image.Get(),view.x+float(x)*sx,view.y+float(y%pageHeight)*sy,float(rect.width)*sx,float(rect.height)*sy);
    renderer_.FlushSprites();
    ID3D11ShaderResourceView* captured=nullptr;
    if(!renderer_.CaptureFrameTexture(captured)||!captured)Fail("Native image page capture failed");
    pages_[page].Attach(captured);pageCpuImageCopy_[page]=++cpuImageCopies_;
    pageMovieFrame_[page]=movieFrame;
    // No source DMA state, completion counter, IRQ, VBlank or Present changes.
}
// 显示页复制支持页内和跨页的非重叠区域；未实现的环绕/重叠显式报错。
void Device::MoveFramebuffer(const PrStage2OtDrawBackend::Command& c){
    const int h=view_.frameHeight/2,w=view_.frameWidth;
    const int sx=int(c.copySourceX)-view_.frameX,sy=int(c.copySourceY)-view_.frameY;
    const int dx=int(c.copyDestinationX)-view_.frameX,dy=int(c.copyDestinationY)-view_.frameY;
    const int cw=c.fillWidth,ch=c.fillHeight;
    if(sx<0||sy<0||dx<0||dy<0||sx+cw>w||dx+cw>w||sy+ch>2*h||dy+ch>2*h)
        Fail("VRAM copy outside produced framebuffer pages is not bound");
    if(sx==dx&&sy==dy){++moveImageCopies_;return;}
    if(sx<dx+cw&&dx<sx+cw&&sy<dy+ch&&dy<sy+ch)
        Fail("overlapping VRAM copy requires original GPU block-copy ordering");
    // 所有来源必须来自实际绘图/上传，且主机像素边界不能被四舍五入。
    const auto scale=[](int coordinate,int pixels,int native){
        if(int64_t(coordinate)*pixels%native)Fail("VRAM copy edge is not representable at this render scale");
        return UINT(int64_t(coordinate)*pixels/native);
    };
    const auto sources=pages_;
    struct Part{uint32_t source,destination;int sy,dy,height;};
    std::vector<Part> parts;
    for(int offset=0;offset<ch;){
        const int sourceY=sy+offset,destinationY=dy+offset;
        const int rows=(std::min)(ch-offset,(std::min)(h-sourceY%h,h-destinationY%h));
        const uint32_t source=uint32_t(sourceY/h),destination=uint32_t(destinationY/h);
        if(!pageKnown_[source])Fail("VRAM copy source has not been produced");
        if(!pageKnown_[destination]&&(dx!=0||cw!=w||destinationY%h!=0||rows!=h))
            Fail("partial VRAM copy has an unproduced destination");
        (void)scale(sx,renderer_.GetWidth(),w);(void)scale(dx,renderer_.GetWidth(),w);
        (void)scale(sx+cw,renderer_.GetWidth(),w);
        (void)scale(sourceY%h,renderer_.GetHeight(),h);(void)scale(destinationY%h,renderer_.GetHeight(),h);
        (void)scale(sourceY%h+rows,renderer_.GetHeight(),h);
        parts.push_back({source,destination,sourceY%h,destinationY%h,rows});offset+=rows;
    }
    std::array<ComPtr<ID3D11Texture2D>,2> targets;
    std::array<ComPtr<ID3D11ShaderResourceView>,2> views;
    renderer_.FlushSprites();
    for(const auto& part:parts){
        ComPtr<ID3D11Resource> source;sources[part.source]->GetResource(source.GetAddressOf());
        if(!targets[part.destination]){
            ComPtr<ID3D11Texture2D> sourceTexture;
            Hresult(source.As(&sourceTexture),"VRAM source texture interface");
            D3D11_TEXTURE2D_DESC desc{};sourceTexture->GetDesc(&desc);
            Hresult(device_->CreateTexture2D(&desc,nullptr,targets[part.destination].GetAddressOf()),"VRAM copy texture allocation");
            Hresult(device_->CreateShaderResourceView(targets[part.destination].Get(),nullptr,views[part.destination].GetAddressOf()),"VRAM copy view allocation");
            if(pageKnown_[part.destination]){
                ComPtr<ID3D11Resource> previous;pages_[part.destination]->GetResource(previous.GetAddressOf());
                context_->CopyResource(targets[part.destination].Get(),previous.Get());
            }
        }
        D3D11_BOX box{scale(sx,renderer_.GetWidth(),w),scale(part.sy,renderer_.GetHeight(),h),0u,
            scale(sx+cw,renderer_.GetWidth(),w),scale(part.sy+part.height,renderer_.GetHeight(),h),1u};
        context_->CopySubresourceRegion(targets[part.destination].Get(),0,
            scale(dx,renderer_.GetWidth(),w),scale(part.dy,renderer_.GetHeight(),h),0,source.Get(),0,&box);
    }
    for(uint32_t page=0;page<2u;++page)if(views[page]){
        pages_[page]=views[page];pageKnown_[page]=true;
        pageCpuImageCopy_[page]=0;
    }
    ++moveImageCopies_;
    // 此处不改变 DMA 寄存器/中断；外层 Service 在同一 context 提交并等待 fence。
}
bool Device::PresentDisplay(){
    Owner();if(faulted_)Fail("cannot present a faulted graphics owner");
    if(displayDisabled_)return false;
    const uint32_t x=display_&1023u,y=(display_>>10u)&511u;
    const int height=view_.frameHeight/2;
    if(int(x)!=view_.frameX||int(y)<view_.frameY||(int(y)-view_.frameY)%height)
        Fail("display address does not select a supported framebuffer page");
    const uint32_t page=uint32_t((int(y)-view_.frameY)/height);
    if(page>=2u||!pageKnown_[page])Fail("display page has not been produced");
    // A native STR frame is held for four NTSC VBlanks.  The page texture is
    // immutable after the movie blit, so submitting the same image on every
    // VBlank only queues duplicate presents and lets the window compositor
    // choose an uneven cadence. Keep the already-presented
    // front-buffer image until a new movie copy (or a gameplay page) exists.
    // A movie frame is written as many 16-pixel strips. The per-strip CPU
    // copy counter therefore changes while the visible source frame does
    // not. Use the native movie frame identity for de-duplication; otherwise
    // one frame can submit twice and produce a visible cadence hitch.
    const bool sameMovieImage = pageMovieFrame_[page] != 0u &&
        pageMovieFrame_[page] == presentedMovieFrame_ && presented_;
    if (sameMovieImage) return true;
    renderer_.BeginFrame(0,0,0);
    auto view=PageViewport(page);
    const bool enhanced=pageMovieFrame_[page]==0u && presentation_ && presentation_->Present(atlas_,renderer_,view);
    if(!enhanced) DrawFramebufferPage(pages_[page].Get());
    renderer_.FlushSprites();
    // The native STR callback supplies the source cadence, while the product
    // session can ask the swap chain to supply the actual desktop scanout
    // cadence. In that mode wait for the next compositor refresh so each
    // movie frame starts on a stable display boundary. Standalone device
    // contracts keep their nonblocking movie handoff.
    const bool moviePage = pageMovieFrame_[page] != 0u;
    ComPtr<ID3D11ShaderResourceView> enhancedImage;
    if(enhanced) {
        ID3D11ShaderResourceView* image=nullptr;
        if(!renderer_.CaptureFrameTexture(image) || !image) Fail("enhanced frame capture failed");
        enhancedImage.Attach(image);
    }
    const auto before=renderer_.GetSuccessfulPresentCount();
    renderer_.EndFrame(!moviePage || synchronizeMoviePresent_);
    if(renderer_.GetSuccessfulPresentCount()!=before+1u)Fail("actual swap-chain Present failed");
    presented_=enhanced?enhancedImage:pages_[page]; // Publish only after real swap-chain acceptance.
    presentedCpuImageCopy_=pageCpuImageCopy_[page];
    if(pageMovieFrame_[page] && presentedMovieFrame_ != pageMovieFrame_[page]) {
        presentedMovieFrame_=pageMovieFrame_[page];
        Log::PrintfNoFlush("S2 movie present frame=%llu present=%llu page=%u", presentedMovieFrame_, presents_+1u, page);
    }
    ++presents_;return true;
}
ComPtr<ID3D11ShaderResourceView> Device::PresentedImage()const{Owner();return presented_;}
void Device::ClearMovieIdentity() noexcept {
    pageMovieFrame_.fill(0u);
    presentedMovieFrame_=0u;
}
// 供消费者检查已经生产的页图像；不能把未初始化页面伪装成黑色图像。
ComPtr<ID3D11ShaderResourceView> Device::FramebufferImage(uint32_t page)const{
    Owner();if(page>=2u||!pageKnown_[page])Fail("framebuffer snapshot has no producer");
    return pages_[page];
}
void Device::SetScanline(uint32_t line,bool field){Owner();scanline_=line;field_=field;}
}
