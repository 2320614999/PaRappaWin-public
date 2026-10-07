#include "pr_stage2_spu_output.h"
#include <algorithm>
#include <stdexcept>

namespace PrStage2SpuOutput {
Stream::Stream(PrStage2SpuDevice::Device& device,IAudioSink& sink):device_(device),sink_(sink){}
Stream::~Stream(){Stop();}
void Stream::Start(){
    if(started_)throw std::logic_error("S2 SPU stream already started");
    RethrowFailure();
    AudioSinkConfig config;config.sampleRate=44100u;config.channels=2u;config.bufferMs=10u;
    if(!sink_.InitializeSink(config,&Stream::Render,this)){
        sink_.ShutdownSink();throw std::runtime_error("S2 SPU output sink initialization failed");
    }
    started_=true;
}
void Stream::Stop(){if(started_){sink_.ShutdownSink();started_=false;}}
bool Stream::Running()const{return started_&&sink_.IsRunning();}
double Stream::SinkSeconds()const{return sink_.GetPlayedSeconds();}
void Stream::RethrowFailure()const{
    std::lock_guard<std::mutex> lock(mutex_);
    if(failure_)std::rethrow_exception(failure_);
}
void Stream::Render(void* user,int16_t* samples,uint32_t frames,uint32_t channels)noexcept{
    auto& self=*static_cast<Stream*>(user);
    try{
        self.RethrowFailure();
        if(channels!=2u)throw std::runtime_error("S2 SPU sink changed stereo contract");
        self.device_.Render(samples,frames);
    }catch(...){
        {std::lock_guard<std::mutex> lock(self.mutex_);if(!self.failure_)self.failure_=std::current_exception();}
        // Never unwind through the platform callback. Silence this failed
        // block and require the scene owner to observe/rethrow the fault.
        if(samples)std::fill_n(samples,size_t(frames)*channels,int16_t(0));
    }
}
}
