#pragma once
#include "pr_stage2_ring_audio_session.h"
#include "pr_stage2_vlc_direct.h"
#include "pr_stage2_movie_codec_bridge.h"
#include "pr_stage2_movie_foreground_direct.h"
#include "pr_native_caption.h"
#include "logger.h"
#include <functional>
#include <utility>

namespace PrStage2VlcSession {
class Session:public PrStage2RingAudioSession::Session,private PrStage2VlcDirect::CacheControl {
public:
    using Base=PrStage2RingAudioSession::Session;
    using Base::Base;
    ~Session() { Cancel(); Output().Stop(); movieOutput_.Cancel(); }
    // Select the shared native codec before startup. The existing fail-closed
    // unbound path remains available for its already-established negative test.
    void EnableSharedMovieOutput() {
        if (sharedMovie_ || State()!=PrStage2FrameTask::State::Ready || InitializationStarts())
            throw std::logic_error("Shared movie backend must be selected before scene startup");
        sharedMovie_=true;
    }
    // Application-owned native input snapshot, sampled by the existing host
    // input implementation. No constant "no button" fallback is installed.
    void EnableMovieForeground(std::function<uint32_t()> readPad) {
        if(!sharedMovie_ || foreground_ || !readPad || State()!=PrStage2FrameTask::State::Ready)
            throw std::logic_error("Movie foreground requires the shared codec and an input owner before startup");
        Cd().BindVideoReadiness([this](const std::array<uint8_t,2352>& raw){
            return PrStage2MovieForegroundDirect::CanAcceptHostStreamSector(*this,raw);
        });
        readPad_=std::move(readPad);foreground_=true;
    }
    PrStage2MovieCodecBridge::Output& MovieOutput() noexcept { return movieOutput_; }
    const PrStage2MovieForegroundDirect::LocationQuery& MovieLocation()const noexcept{return movieLocation_;}
    uint32_t LastVlcSource()const noexcept{return lastSource_;}
    uint32_t LastVlcDestination()const noexcept{return lastDestination_;}
    uint64_t VlcCalls()const noexcept{return calls_;}
    bool SwapCacheBitKnownSet()const noexcept{return swapKnown_;}
protected:
    // Hold both the RAM output and its completion callback until the media
    // deadline. The source blitter reads RAM independently of the callback;
    // delaying only the callback can expose the next image a frame early.
    virtual bool MovieCallbackAllowed() { return true; }
    virtual void OnMovieFrameDelivered() {}
    virtual void OnMovieScanoutEnded() {}

    uint32_t ReadHostPadSnapshot() const {
        return readPad_ ? readPad_() : 0u;
    }
    virtual int32_t VideoDependency(uint32_t,std::initializer_list<uint32_t>)=0;
    int32_t SceneDependency(uint32_t fn,std::initializer_list<uint32_t> args)override{
        if(foreground_ && fn==0x80027664u){
            // 80027664 releases the decoder ring immediately after the
            // source movie stop routine.  A final DMA3 payload can still be
            // owned by the host device at this point; freeing/reusing that
            // ring first lets the old transfer land in the next movie's
            // buffers, which presents as an intermittent VLC failure on its
            // first frame.  Drain the real MMIO channel before the native
            // cleanup call instead of manufacturing a completion.
            const uint32_t dma = Read32(0x800574ECu);
            uint32_t passes = 0u;
            while (dma != 0u && (Read32(dma) & 0x01000000u) != 0u) {
                if (++passes > 0x10000u)
                    throw std::runtime_error("S2 movie stop DMA3 drain did not complete");
                AwaitDeviceProgress(dma);
            }
            if (passes != 0u) {
                Log::Printf("S2 movie stop drained DMA3 passes=%u", passes);
            }
            const int32_t result=Base::SceneDependency(fn,args);
            // The original decoder has stopped. The following page copy and
            // transition draw new images even though no new STR frame exists.
            // End de-duplication here for every movie, including an outro.
            Gpu().ClearMovieIdentity();
            OnMovieScanoutEnded();
            return result;
        }
        if(foreground_ && fn==0x800274D4u){
            // Let the original SetStream/decoder initialization complete on
            // this same stack before the host starts delivering optical data.
            // No ring bytes, source readiness flags or return values are seeded.
            const int32_t result=Base::SceneDependency(fn,args);
            movieStreamConfigured_=true;return result;
        }
        if(foreground_ && fn==0x80036678u && args.size()==2u && *args.begin()==16u){
            if(args.begin()[1]!=0u)throw std::invalid_argument("GetlocL has no input parameters");
            // Reuse the source synchronization and real optical producer,
            // replacing only this SDK operation, not extending the drive model.
            (void)PrStage2CdCommandDirect::Sync80037070(*this,0u,0u);
            while(!Cd().SectorReads())WaitDevice(0x1F801800u,uint32_t(Cd().Serial()));
            std::array<uint8_t,8> header{};
            for(uint32_t i=0;i<8u;++i)header[i]=Cd().RawSector()[12u+i];
            movieLocation_.Request(*this,header);return 1;
        }
        return Base::SceneDependency(fn,args);
    }
    void CallLoadingDeviceVoid(uint32_t fn,std::initializer_list<uint32_t> args)override{
        if(foreground_){
            if(PrNativeCaption::TryCallVoid(*this,fn,args))return;
            if(fn==0x80048A00u){
                if(args.size())throw std::invalid_argument("Native pad snapshot takes no arguments");
                const uint32_t held=readPad_();Write32(0x800882F0u,~held);return;
            }
            if(PrStage2MovieForegroundDirect::TryCallVoid(*this,fn,args))return;
        }
        if (sharedMovie_ && (fn==0x80047558u || fn==0x800475D4u ||
                             fn==0x80047778u || fn==0x8004780Cu)) {
            if(args.size()!=2u) throw std::invalid_argument("Native movie SDK argument count");
            const auto a=args.begin();
            if(fn==0x80047558u) PrStage2MovieCodecBridge::Input80047558(*this,a[0],a[1]);
            else if(fn==0x800475D4u) PrStage2MovieCodecBridge::Output800475D4(*this,a[0],a[1]);
            else if(fn==0x80047778u) {
                while(Mdec().Pending()) WaitDevice(0x1F801088u,uint32_t(Mdec().Serial()));
                if(!Mdec().QuantKnown() || !Mdec().ScaleKnown())
                    throw std::logic_error("Shared movie codec requires actual completed table producers");
                movieOutput_.Input(*this,a[0],a[1],Mdec().Quant(),Mdec().ScaleBytes());
            } else movieOutput_.Request(*this,a[0],a[1]);
            return;
        }
        if(fn==0x80039490u){
            if(args.size()!=1u)throw std::invalid_argument("Source frame release takes one argument");
            (void)PrStage2VlcDirect::ReleaseFrame80039490(*this,*args.begin());return;
        }
        Base::CallLoadingDeviceVoid(fn,args);
    }
    bool ServiceAdditionalDevices() override {
        const bool location=foreground_ && movieLocation_.Service(*this,HostCallbacksAllowed());
        const bool devices=Base::ServiceAdditionalDevices();
        const uint64_t callbacksBefore = movieOutput_.CallbackCalls();
        const bool movieDue = MovieCallbackAllowed();
        const bool movie=sharedMovie_ && movieOutput_.Service(
            *this, HostCallbacksAllowed() && movieDue, movieDue);
        if (movieOutput_.CallbackCalls() != callbacksBefore)
            OnMovieFrameDelivered();
        return devices || movie || location;
    }
    bool AllowNewStreamSector() override {
        if(!foreground_)return true;
        if(!HostCallbacksAllowed())return false;
        // 原 8001A4D0 在音频开流完成后写 active=1、cadence=4。
        // 演示/回放可跳过影片，不会初始化解码器，仍需实际 XA 扇区和位置应答。
        const bool audioConfigured=Read32(0x80049410u)==1u&&Read32(0x80049424u)==4u;
        return movieStreamConfigured_||audioConfigured;
    }
private:
    PrStage2MovieCodecBridge::Output movieOutput_;
    PrStage2MovieForegroundDirect::LocationQuery movieLocation_;
    bool sharedMovie_=false;
    bool foreground_=false;
    bool movieStreamConfigured_=false;
    bool movieBlit_=false;
    std::function<uint32_t()> readPad_;
    uint32_t lastSource_=0,lastDestination_=0;
    uint64_t calls_=0;
    bool swapKnown_=false;
    void SetSwapCacheBit80047E30()override{
        // Only the known OR of SR bit17 is retained. No interrupt-enable bits,
        // cache-isolation state, GPU fence or video-ready flag are invented.
        // Hardware testing documents no PSX memory effect for SwC itself.
        swapKnown_=true;
    }
    int32_t DecodeDependency(uint32_t fn,std::initializer_list<uint32_t> args)override{
        if(foreground_){
            int32_t value;
            if(PrNativeCaption::TryCall(*this,fn,args,value))return value;
            const bool previous=movieBlit_;
            if(fn==0x8002756Cu)movieBlit_=true;
            try{
                const bool bound=PrStage2MovieForegroundDirect::TryCall(*this,fn,args,value);
                movieBlit_=previous;if(bound)return value;
            }catch(...){movieBlit_=previous;throw;}
        }
        if(sharedMovie_ && (fn==0x80047558u || fn==0x800475D4u || fn==0x8004780Cu))
            throw std::invalid_argument("Native movie SDK output cannot manufacture a scalar return");
        if(fn==0x80047B30u){
            if(args.size()!=2u)throw std::invalid_argument("Source VLC decoder takes two arguments");
            const auto p=args.begin();lastSource_=p[0];lastDestination_=p[1];++calls_;
            return PrStage2VlcDirect::Decode80047B30(*this,*this,p[0],p[1]);
        }
        if(fn==0x80039490u){
            if(args.size()!=1u)throw std::invalid_argument("Source frame release takes one argument");
            return PrStage2VlcDirect::ReleaseFrame80039490(*this,*args.begin());
        }
        return VideoDependency(fn,args);
    }
protected:
    int32_t LoadImage80044D64(PrStage2LifecycleDirect::ImageRect rect,uint32_t source)override{
        const int32_t result=Base::LoadImage80044D64(rect,source);
        if(movieBlit_){
            // Consume the actual source words after the existing RAM->VRAM
            // service, not a cached decoder frame or an out-of-band screenshot.
            std::vector<uint16_t> words;words.reserve(size_t(rect.width)*rect.height);
            for(uint32_t n=0;n<uint32_t(rect.width)*rect.height;++n)words.push_back(Read16(source+2u*n));
            const uint32_t movieWidth=Read32(Read32(0x8006ED78u)+8u);
            if(movieWidth>1024u)throw std::invalid_argument("Native movie width exceeds GPU range");
            Gpu().CopyFramebufferImage(rect,words,MovieOutput().PublishedFrames(),uint16_t(movieWidth));
        }
        return result;
    }
};
}
