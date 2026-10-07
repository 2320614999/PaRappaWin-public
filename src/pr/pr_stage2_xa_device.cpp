#include "pr_stage2_xa_device.h"
#include "pr_stage2_xa_coefficients.h"
#include <algorithm>
#include <stdexcept>

namespace PrStage2XaDevice {
namespace {
int32_t Shift(int64_t v,unsigned bits){
    const int64_t d=int64_t(1)<<bits;
    return int32_t(v>=0?v/d:-((-v+d-1)/d));
}
int16_t Clamp(int64_t value){return int16_t((std::max)(int64_t(-32768),(std::min)(int64_t(32767),value)));}
void Validate(const std::array<uint8_t,2352>& raw){
    if(raw[0]||raw[11]||raw[15]!=2u||(raw[18]&0x64u)!=0x64u||
       !std::equal(raw.begin()+16,raw.begin()+20,raw.begin()+20))
        throw std::invalid_argument("XA requires an intact Mode2 realtime audio sector");
    for(uint32_t i=1;i<11;++i)if(raw[i]!=255u)throw std::invalid_argument("XA sync bytes");
    // Native bounded decoder supports the actual 4-bit/37800-Hz source form.
    // Reject rather than misplay 18900-Hz, 8-bit, emphasis or reserved forms.
    if(raw[19]>1u)throw std::invalid_argument("XA coding requires 4-bit 37800-Hz mono/stereo without emphasis");
    for(uint32_t group=0;group<18u;++group){
        const uint8_t* p=raw.data()+24u+128u*group;
        if(!std::equal(p,p+4,p+4)||!std::equal(p+8,p+12,p+12))
            throw std::invalid_argument("XA duplicated ADPCM header is inconsistent");
        for(uint32_t i=4;i<12;++i)if(p[i]&0xC0u)throw std::invalid_argument("XA reserved predictor bits");
    }
}
}
int16_t Decoder::Sample(const uint8_t* group,uint32_t block,uint32_t nibble,uint32_t j,uint32_t channel){
    const uint8_t param=group[4u+block*2u+nibble];
    uint32_t shift=param&15u;if(shift>12u)shift=9u;
    const uint32_t filter=(param>>4u)&3u;
    const int32_t positive[4]={0,60,115,98},negative[4]={0,0,-52,-55};
    int32_t value=(group[16u+block+j*4u]>>(4u*nibble))&15u;if(value&8)value-=16;
    value=Shift(int64_t(value)*4096,shift)+Shift(int64_t(previous_[channel])*positive[filter]+int64_t(older_[channel])*negative[filter]+32,6);
    const int16_t sample=Clamp(value);older_[channel]=previous_[channel];previous_[channel]=sample;return sample;
}
void Decoder::Resample(int16_t left,int16_t right,std::vector<int16_t>& output){
    history_[0][position_&31u]=left;history_[1][position_&31u]=right;++position_;
    if(--six_)return;six_=6;
    for(uint32_t phase=0;phase<7;++phase)for(uint32_t channel=0;channel<2;++channel){
        int32_t sum=0;
        // Per-product arithmetic shift, not a single shift after summing.
        // Measured FIR/initial-phase limitations are recorded in the authority.
        for(uint32_t tap=0;tap<29;++tap)sum+=Shift(int64_t(history_[channel][(position_-tap-1u)&31u])*Coefficients[tap][phase],15);
        output.push_back(Clamp(sum));
    }
}
std::vector<int16_t> Decoder::Decode(const std::array<uint8_t,2352>& raw){
    Validate(raw);
    if(coding_>=0&&coding_!=int32_t(raw[19]))throw std::invalid_argument("XA format changed without a new explicit decoder owner");
    coding_=raw[19];std::vector<int16_t> result;result.reserve(raw[19]?4704u:9408u);
    for(uint32_t group=0;group<18;++group){
        const auto* p=raw.data()+24u+128u*group;
        for(uint32_t block=0;block<4;++block){
            if(raw[19])for(uint32_t j=0;j<28;++j){
                const auto left=Sample(p,block,0u,j,0u),right=Sample(p,block,1u,j,1u);
                Resample(left,right,result);
            }else for(uint32_t nibble=0;nibble<2;++nibble)for(uint32_t j=0;j<28;++j){
                const auto mono=Sample(p,block,nibble,j,0u);Resample(mono,mono,result);
            }
        }
    }
    return result;
}
bool Device::Consume(const std::array<uint8_t,2352>& raw,uint32_t lba,bool muted,const std::array<uint8_t,4>& matrix){
    // The documented saturation beyond double-volume is irregular. Fail
    // closed for that unused mode instead of silently inventing a mixer.
    if(uint32_t(matrix[0])+matrix[3]>256u||uint32_t(matrix[1])+matrix[2]>256u)
        throw std::invalid_argument("XA matrix exceeds the verified double-volume range");
    const size_t frames=raw[19]==1u?2352u:4704u;
    if(!spu_.CanQueueCdFrames(frames))return false;
    Decoder next=decoder_;auto pcm=next.Decode(raw);
    for(size_t i=0;i<pcm.size();i+=2u){
        const int32_t left=pcm[i],right=pcm[i+1u];
        pcm[i]=muted?0:Clamp(Shift(int64_t(left)*matrix[0]+int64_t(right)*matrix[3],7));
        pcm[i+1u]=muted?0:Clamp(Shift(int64_t(left)*matrix[1]+int64_t(right)*matrix[2],7));
    }
    if(!spu_.QueueCdAudio(pcm))return false;
    decoder_=next;lastPcm_=std::move(pcm);lastLba_=lba;++sectors_;frames_+=lastPcm_.size()/2u;return true;
}
}
