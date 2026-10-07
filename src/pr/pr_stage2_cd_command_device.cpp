#include "pr_stage2_cd_command_device.h"
#include <algorithm>
#include <cstring>
#include <cmath>
#include <stdexcept>
#include <string>

namespace PrStage2CdCommandDevice {
namespace {
[[noreturn]]void Fail(const char* why){throw std::runtime_error(std::string("S2 CD device: ")+why);}
uint32_t Bcd(uint8_t b){if((b&15u)>9u||(b>>4u)>9u)throw std::invalid_argument("Invalid CD BCD digit");return (b>>4u)*10u+(b&15u);}
}
Device::Device(const std::filesystem::path& path,PrStage2InterruptDevice::Controller& irq,
               std::function<Clock::time_point()> now):irq_(irq),file_(path,std::ios::binary),
    now_(now?std::move(now):[] {return Clock::now();}){
    if(!file_)Fail("optical image could not be opened");
    file_.seekg(0,std::ios::end);const auto size=file_.tellg();
    if(size<=0||uint64_t(size)%2352u)Fail("optical image is not whole raw sectors");
    sectors_=uint64_t(size)/2352u;file_.seekg(0);parameters_.reserve(16u);
    readAhead_.resize(ReadAheadSectors*2352u);
}
void Device::EnableMechanicalTiming(){
    Owner();if(commands_)throw std::logic_error("CD startup timing must be selected before commands");
    mechanicalTiming_=true;
}
void Device::HoldNextStreamStart(){
    Owner();
    if(reading_||streamStartHeld_)throw std::logic_error("Stream startup already active");
    streamStartHeld_=true;
}
void Device::ReleaseStreamStart(){
    Owner();
    if(!streamStartHeld_||!reading_||!readStarting_)
        throw std::logic_error("No held stream startup to release");
    // A delayed first sector has no playback debt. Preserve a later physical
    // seek deadline, but never burst old audio deadlines after host release.
    nextSector_=(std::max)(nextSector_,now_());
    streamStartHeld_=false;
}
std::chrono::microseconds Device::ReadSetupTime()const{
    // Mechanical estimates, separate from the source program's Ready/Sync
    // control flow. Based on the local DuckStation CDROM timing model:
    // 1 s spin-up, 0.6/0.7 s speed change, short seek or distance-dependent
    // sled seek. These are hardware estimates, not instruction timing proof.
    const bool motor=(driveStatus_&2u)!=0u;
    const uint32_t from=motor?lastSector_:0u;
    const uint32_t distance=location_>from?location_-from:from-location_;
    double seconds=motor?0.0:1.0;
    if(distance<7200u){
        if(distance<=8u)seconds+=double((std::max)(distance,2u))/((mode_&128u)?150.0:75.0);
        else seconds+=distance<330u?.05:.1;
    }else{
        constexpr double maximum=72.0*4500.0;
        seconds+=.05+.85*(.4*std::log(double(distance))/std::log(maximum)+.6*distance/maximum);
    }
    const auto now=now_();
    const auto remaining=speedReady_>now?speedReady_-now:Clock::duration::zero();
    return std::chrono::microseconds(int64_t(seconds*1000000.0))+
        std::chrono::duration_cast<std::chrono::microseconds>(remaining);
}
void Device::Owner()const{if(owner_!=std::this_thread::get_id())throw std::logic_error("CD device accessed by a different owner");}
void Device::BindXaConsumer(XaConsumer& consumer){
    Owner();if(xa_)throw std::logic_error("XA consumer already bound");xa_=&consumer;
}
void Device::BindVideoReadiness(std::function<bool(const std::array<uint8_t,2352>&)> ready){
    Owner();
    if(!ready||videoReady_||reading_||sectorReads_)
        throw std::logic_error("Native video receiver must be bound once before stream reads");
    videoReady_=std::move(ready);
}
size_t Device::DmaBytesAvailable()const{
    Owner();return rawKnown_&&(request_&128u)&&dataPosition_<dataSize_?dataSize_-dataPosition_:0u;
}
uint32_t Device::ReadDmaWord(){
    Owner();if(DmaBytesAvailable()<4u)Fail("DMA word exceeds produced sector bytes");
    uint32_t word=0u;
    for(uint32_t i=0;i<4u;++i){uint32_t byte=0u;TryRead(0x1F801802u,1u,byte);word|=byte<<(8u*i);}
    return word;
}
void Device::ReleaseConsumedSector(){
    Owner();
    if(!rawKnown_||!(request_&128u)||!dataSize_||dataPosition_!=dataSize_)
        Fail("cannot release an incomplete or unowned sector");
    rawKnown_=false;request_=0u;dataPosition_=dataSize_=0u;++serial_;
    // DMA 释放只归还缓冲区；光盘期限在实际读取时推进，不能再次叠加
    // 一个扇区周期，否则传输/IRQ 耗时会永久降低影片和 XA 的播放速度。
}
uint32_t Device::Register(uint32_t a,uint32_t width){
    const uint32_t segment=a&0xE0000000u;
    if(segment!=0u&&segment!=0x80000000u&&segment!=0xA0000000u)return 0;
    const uint32_t p=a&0x1FFFFFFFu;
    if(p==0x1F801020u){if(width!=4u)throw std::invalid_argument("CD delay requires source word width");return p;}
    if(p<0x1F801800u||p>0x1F801803u)return 0;
    if(width!=1u)throw std::invalid_argument("CD host interface requires source byte width");return p;
}
void Device::Irq(){irq_.SetLine(PrStage2InterruptDevice::Source::CdRom,(mask_&flags_&31u)!=0u);}
bool Device::TryRead(uint32_t a,uint32_t width,uint32_t& value){
    const uint32_t p=Register(a,width);if(!p)return false;Owner();
    if(p==0x1F801020u){value=delay_;return true;}
    switch(p&3u){
    case 0:value=bank_|(parameters_.empty()?8u:0u)|(parameters_.size()<16u?16u:0u)|
        (resultPosition_<resultSize_?32u:0u)|((request_&128u)&&dataPosition_<dataSize_?64u:0u)|(pending_?128u:0u);break;
    case 1:value=result_[resultPosition_&15u];++resultPosition_;break;
    case 2:
        if(!(request_&128u)||!rawKnown_)Fail("data read without a produced and requested sector");
        value=rawSector_[dataOffset_+(dataPosition_<dataSize_?dataPosition_:(dataSize_==2048u?2040u:2336u))];++dataPosition_;break;
    case 3:value=0xE0u|((bank_&1u)?flags_:mask_);break;
    }
    return true;
}
bool Device::TryWrite(uint32_t a,uint32_t width,uint32_t value){
    const uint32_t p=Register(a,width);if(!p)return false;Owner();
    if(p==0x1F801020u){delay_=value;return true;}
    const uint8_t v=uint8_t(value);
    if((p&3u)==0u){bank_=v&3u;return true;}
    if(bank_==0u){
        if((p&3u)==2u){if(parameters_.size()==16u)throw std::overflow_error("CD parameter FIFO overflow");parameters_.push_back(v);return true;}
        if((p&3u)==3u){
            if(v&0x60u)Fail("sound-map/write sector requests are not bound");
            if((v&128u)&&!rawKnown_)Fail("read request has no actual sector producer");
            request_=v;if(v&128u){dataPosition_=0;dataOffset_=(mode_&32u)?12u:24u;dataSize_=(mode_&32u)?2340u:2048u;}return true;
        }
        if(pending_||flags_||!queued_.empty())Fail("overlapping command while prior response remains owned");
        // A command write only captures its input. No drive effect, response,
        // sector read or completion interrupt happens before a Service pass.
        command_=v;latchedParameters_=parameters_;parameters_.clear();pending_=true;
        commandTrace_.push_back(v);++commands_;++serial_;return true;
    }
    if(bank_==1u){
        if((p&3u)==2u){mask_=v&31u;Irq();return true;}
        if((p&3u)==3u){
            if(v&0xA0u)Fail("full decoder/sound-map reset is not bound");
            if(v&64u)parameters_.clear();
            const uint8_t before=flags_;flags_&=uint8_t(~(v&31u));
            if(before!=flags_){++acknowledgments_;resultPosition_=resultSize_;}
            Irq();return true; // Never publish the next reply from a register write.
        }
        Fail("manual sound-map sector writes are not bound");
    }
    if(bank_==2u&&((p&3u)==2u||(p&3u)==3u)){pendingMatrix_[(p&3u)-2u]=v;return true;}
    if(bank_==3u){
        if((p&3u)==1u){pendingMatrix_[2]=v;return true;}
        if((p&3u)==2u){pendingMatrix_[3]=v;return true;}
        if(v&~0x21u)Fail("unbound XA apply/mute control bits");
        adpcmMuted_=(v&1u)!=0u;if(v&32u)matrix_=pendingMatrix_;return true;
    }
    Fail("CD sound-map ports are outside this command transport");
}
void Device::ReplyStatus(uint8_t kind,uint8_t status){queued_.push_back({kind,{status}});}
void Device::Error(uint8_t detail){queued_.push_back({5u,{uint8_t(driveStatus_|1u),detail}});}
void Device::ReadSector(uint64_t sector){
    Owner();
    if(sector>=sectors_)Fail("optical-image sector is outside the source");
    if(!(sector>=readAheadStart_&&sector<readAheadStart_+readAheadCount_)){
        const uint64_t remaining=sectors_-sector;
        readAheadCount_=static_cast<std::size_t>((std::min)(uint64_t(ReadAheadSectors),remaining));
        file_.clear();file_.seekg(std::streamoff(sector*2352u));
        file_.read(reinterpret_cast<char*>(readAhead_.data()),
                   std::streamsize(readAheadCount_*2352u));
        if(file_.gcount()!=std::streamsize(readAheadCount_*2352u))
            Fail("actual optical-image read-ahead failed; no successful data IRQ");
        readAheadStart_=sector;
    }
    const std::size_t offset=static_cast<std::size_t>(sector-readAheadStart_)*2352u;
    std::copy_n(readAhead_.data()+offset,2352u,rawSector_.data());
}
bool Device::Publish(){
    if(flags_||queued_.empty())return false;
    auto reply=std::move(queued_.front());queued_.pop_front();
    if(reply.bytes.size()>result_.size())Fail("oversized internal command response");
    result_.fill(0u);std::copy(reply.bytes.begin(),reply.bytes.end(),result_.begin());resultPosition_=0;resultSize_=reply.bytes.size();
    flags_=reply.kind;++responses_;++serial_;Irq();return true;
}
bool Device::PublishVideo(){
    if(!videoPending_)return false;
    if(videoReady_&&!videoReady_(rawSector_)){++videoDeferrals_;return false;}
    // Only the host-held sector is released here. The original receiver still
    // reads its header/payload and performs every RAM/OT/decoder side effect.
    ReplyStatus(1u,driveStatus_);
    rawKnown_=true;videoPending_=false;dataPosition_=dataSize_=0;
    return Publish();
}
void Device::Execute(){
    const auto& p=latchedParameters_;
    const size_t count=command_==2u?3u:command_==13u?2u:command_==14u?1u:0u;
    if(p.size()!=count){Error(0x20u);return;}
    switch(command_){
    case 1u:ReplyStatus(3u,driveStatus_);break;
    case 2u:{
        uint32_t minutes,seconds,frames;
        try{minutes=Bcd(p[0]);seconds=Bcd(p[1]);frames=Bcd(p[2]);}
        catch(const std::invalid_argument&){Error(0x10u);break;}
        const uint32_t absolute=(minutes*60u+seconds)*75u+frames;
        if(seconds>=60u||frames>=75u||absolute<150u||absolute-150u>=sectors_){Error(0x10u);break;}
        location_=absolute-150u;locationKnown_=true;ReplyStatus(3u,driveStatus_);break; // Setloc is not a sector read.
    }
    case 10u:
        mode_=0x20u;driveStatus_=2u;reading_=false;rawKnown_=false;audioPending_=false;videoPending_=false;request_=0;
        readStarting_=false;streamStartHeld_=false;speedReady_={};
        ReplyStatus(3u,driveStatus_);ReplyStatus(2u,driveStatus_);break;
    case 11u:muted_=true;ReplyStatus(3u,driveStatus_);break;
    case 12u:muted_=false;ReplyStatus(3u,driveStatus_);break;
    case 13u:filterFile_=p[0];filterChannel_=p[1];ReplyStatus(3u,driveStatus_);break;
    case 14u:
        if(p[0]&0x17u)Fail("CD ignore/report/autopause/CDDA mode is not bound");
        if(mechanicalTiming_&&((mode_^p[0])&128u))
            speedReady_=now_()+std::chrono::milliseconds((p[0]&128u)?600:700);
        mode_=p[0];ReplyStatus(3u,driveStatus_);break;
    case 6u:case 27u:{
        if(!locationKnown_){Error(0x80u);break;}
        const auto setup=mechanicalTiming_?ReadSetupTime():std::chrono::microseconds::zero();
        readStarting_=mechanicalTiming_||streamStartHeld_;
        driveStatus_=readStarting_?0x42u:0x22u;reading_=true;rawKnown_=false;videoPending_=false;request_=0;
        nextSector_=now_()+setup+std::chrono::microseconds((mode_&128u)?6667:13334);
        ReplyStatus(3u,driveStatus_);break;
    }
    case 8u:case 9u:
        ReplyStatus(3u,driveStatus_);reading_=false;readStarting_=false;streamStartHeld_=false;videoPending_=false;
        driveStatus_=command_==8u?0u:2u;ReplyStatus(2u,driveStatus_);break;
    default:Fail("command opcode is not implemented");
    }
}
bool Device::Service(bool allowNewSector){
    Owner();
    if(flags_)return false;
    if(!queued_.empty())return Publish();
    if(pending_){Execute();pending_=false;++executed_;++serial_;return Publish();}
    if(videoPending_)return allowNewSector&&reading_?PublishVideo():false;
    if(audioPending_){
        if(!xa_||!xa_->Consume(rawSector_,lastSector_,muted_||adpcmMuted_,matrix_))return false;
        audioPending_=false;++audioSectors_;++serial_;
        return true;
    }
    if(!allowNewSector||!reading_||streamStartHeld_||(rawKnown_&&(request_&128u))||now_()<nextSector_)return false;
    if(location_>=sectors_){reading_=false;driveStatus_=2u;ReplyStatus(4u,driveStatus_);return Publish();}
    // 已确认 INT1、但未请求进入数据 FIFO 的扇区不阻塞光盘推进。
    // 实际 DMA 持有的数据仍由 ReleaseConsumedSector 释放，不按时间丢弃。
    if(rawKnown_){rawKnown_=false;dataPosition_=dataSize_=0;++unrequestedSectorsReplaced_;}
    ReadSector(location_);
    if(rawSector_[0]!=0u||rawSector_[11]!=0u||rawSector_[15]!=2u||
       !std::equal(rawSector_.begin()+16,rawSector_.begin()+20,rawSector_.begin()+20))
        Fail("unsupported or malformed actual Mode2 sector");
    for(size_t i=1;i<11;++i)if(rawSector_[i]!=255u)Fail("invalid raw sector synchronization bytes");
    if(readStarting_){readStarting_=false;driveStatus_=0x22u;}
    // 光盘按绝对期限连续供给，接收方和 DMA 仍保留原背压。短暂调度
    // 抖动可在后续 Service 补上；长时间暂停只保留至多 100ms 的期限债务，
    // 避免恢复时无界追赶。一次 Service 仍最多读取一个真实扇区。
    const auto period=std::chrono::microseconds((mode_&128u)?6667:13334);
    const auto now=now_();
    nextSector_+=period;
    const auto catchUp=std::chrono::milliseconds(100);
    if(nextSector_+catchUp<now)nextSector_=now-catchUp;
    // Selected XA audio is decoded and queued to the real sample owner. It
    // produces no data-ready INT1 and never enters the video/DMA3 ring.
    if((mode_&64u)&&((rawSector_[18]&0x44u)==0x44u)){
        const bool filtered=(mode_&8u)&&(rawSector_[16]!=filterFile_||rawSector_[17]!=filterChannel_);
        if(!filtered&&!xa_)Fail("actual XA audio sector requires native decoder/mixer binding");
        lastSector_=location_;++location_;++sectorReads_;++serial_;
        if(filtered){++filteredAudioSectors_;return true;}
        audioPending_=true;
        if(xa_->Consume(rawSector_,lastSector_,muted_||adpcmMuted_,matrix_)){
            audioPending_=false;++audioSectors_;
        }
        return true;
    }
    lastSector_=location_;++location_;++sectorReads_;++serial_;videoPending_=true;
    (void)PublishVideo();return true; // Actual file read occurred, even if held.
}
}
