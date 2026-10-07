#include "pr_stage2_spu_boot_direct.h"
#include "pr_stage2_vab_direct.h"
#include <array>
#include <stdexcept>

namespace PrStage2SpuBootDirect {
namespace {
uint32_t U(int32_t v){return static_cast<uint32_t>(v);}
int32_t S(uint32_t v){return v<0x80000000u?int32_t(v):int32_t(int64_t(v)-0x100000000LL);}
uint32_t H(uint32_t v){v&=65535u;return v<32768u?v:v|0xFFFF0000u;}
void Delay240(){
    // Original stack-only delay arithmetic, deliberately no fabricated frame,
    // sample, hardware completion, RAM address, or calibrated host sleep.
    volatile uint32_t value=13u;
    for(uint32_t i=0;i<240u;++i)value=uint32_t(value)*3u;
}
void StatusWait(Services& s,uint32_t mask,uint32_t expected,uint32_t diagnostic){
    uint32_t status=s.Read16(s.Read32(0x800555C8u)+0x1AEu)&mask;
    s.Write32(0x800555C0u,0u);
    while(status!=expected){
        const uint32_t count=s.Read32(0x800555C0u)+1u;s.Write32(0x800555C0u,count);
        if(S(count)>=5001){s.Call(0x80047FFCu,{0x80011810u,diagnostic});break;}
        s.AwaitDeviceProgress(0x1F801DAEu);
        status=s.Read16(s.Read32(0x800555C8u)+0x1AEu)&mask;
    }
}
struct AttributeSource {
    virtual ~AttributeSource()=default;
    virtual uint16_t Half(uint32_t offset)=0;
    virtual uint32_t Word(uint32_t offset)=0;
};
struct PublicAttribute final:AttributeSource{
    Services& memory;uint32_t base;
    PublicAttribute(Services& m,uint32_t b):memory(m),base(b){}
    uint16_t Half(uint32_t p)override{return memory.Read16(base+p);}
    uint32_t Word(uint32_t p)override{return memory.Read32(base+p);}
};
struct PrivateAttribute final:AttributeSource{
    std::array<uint8_t,40> bytes{},known{};
    void Put(uint32_t p,uint32_t n,uint32_t v){for(uint32_t i=0;i<n;++i){bytes.at(p+i)=uint8_t(v>>(8*i));known.at(p+i)=1;}}
    uint32_t Read(uint32_t p,uint32_t n){uint32_t v=0;for(uint32_t i=0;i<n;++i){if(!known.at(p+i))throw std::logic_error("Original private SPU attribute has no value producer");v|=uint32_t(bytes.at(p+i))<<(8*i);}return v;}
    uint16_t Half(uint32_t p)override{return uint16_t(Read(p,2));}
    uint32_t Word(uint32_t p)override{return Read(p,4);}
};
void ApplyAttributes(Services& s,AttributeSource& a){
    const uint32_t mask=a.Word(0);const bool all=mask==0u;
    const auto volume=[&](uint32_t volumeOffset,uint32_t modeOffset,bool readMode,uint32_t reg){
        uint32_t mode=0u;
        if(readMode){const uint32_t raw=H(a.Half(modeOffset));if(raw>=1u&&raw<=7u)mode=0x8000u+(raw-1u)*0x1000u;}
        uint32_t value=a.Half(volumeOffset);
        if(mode){const int32_t signedValue=S(H(value));value=signedValue<0?0u:signedValue>=128?127u:U(signedValue);}
        const uint32_t base=s.Read32(0x800555C8u);s.Write16(base+reg,uint16_t((value&0x7FFFu)|mode));
    };
    if(all||(mask&1u))volume(4u,8u,all||(mask&4u),0x180u);
    if(all||(mask&2u))volume(6u,10u,all||(mask&8u),0x182u);
    const auto half=[&](uint32_t field,uint32_t reg){const uint32_t base=s.Read32(0x800555C8u);s.Write16(base+reg,a.Half(field));};
    const auto flag=[&](uint32_t field,uint32_t bit){
        const uint32_t enabled=a.Word(field),base=s.Read32(0x800555C8u);const uint16_t old=s.Read16(base+0x1AAu);
        s.Write16(base+0x1AAu,uint16_t(enabled?(old|bit):(old&~bit)));
    };
    if(all||(mask&0x40u))half(0x10u,0x1B0u);
    if(all||(mask&0x80u))half(0x12u,0x1B2u);
    if(all||(mask&0x100u))flag(0x14u,4u);
    if(all||(mask&0x200u))flag(0x18u,1u);
    if(all||(mask&0x400u))half(0x1Cu,0x1B4u);
    if(all||(mask&0x800u))half(0x1Eu,0x1B6u);
    if(all||(mask&0x1000u))flag(0x20u,8u);
    if(all||(mask&0x2000u))flag(0x24u,2u);
}
}
void CommonAttributes8002A6FC(Services& s,uint32_t attributes){PublicAttribute a(s,attributes);ApplyAttributes(s,a);}
void MasterVolume8002A6AC(Services& s,uint32_t left,uint32_t right){
    PrivateAttribute a;a.Put(0,4,3u);a.Put(4,2,H(left)*129u);a.Put(6,2,H(right)*129u);ApplyAttributes(s,a);
}
void SetMix8002AA90(Services& s,uint32_t device,uint32_t kind,uint32_t value){
    PrivateAttribute a;device&=255u;kind&=255u;
    if(device==0u&&kind==0u){a.Put(0,4,0x200u);a.Put(0x18u,4,value&255u);}
    if(device==0u&&kind==1u){a.Put(0,4,0x100u);a.Put(0x14u,4,value&255u);}
    if(device==1u&&kind==0u){a.Put(0,4,0x2000u);a.Put(0x24u,4,value&255u);}
    if(device==1u&&kind==1u){a.Put(0,4,0x1000u);a.Put(0x20u,4,value&255u);}
    ApplyAttributes(s,a);
}
void ExternalVolume8002AB24(Services& s,uint32_t device,uint32_t left,uint32_t right){
    PrivateAttribute a;device&=255u;
    if(device<=1u){
        const uint32_t p=device?0x1Cu:0x10u;a.Put(0,4,device?0xC00u:0xC0u);
        const int32_t l=S(H(left)),r=S(H(right));
        a.Put(p,2,U(l>=128?127:l)*258u);a.Put(p+2u,2,U(r>=128?127:r)*258u);
    }
    ApplyAttributes(s,a);
}

void WriteFifo80029B38(Services& s,uint32_t source,uint32_t bytes){
    uint32_t base=s.Read32(0x800555C8u);const uint16_t address=s.Read16(0x800555C4u);
    const uint16_t oldStatus=s.Read16(base+0x1AEu);
    s.Write16(base+0x1A6u,address);Delay240();
    uint32_t remaining=bytes;
    while(remaining){
        const uint32_t chunk=remaining<=64u?remaining:64u;
        if(S(chunk)>0){
            base=s.Read32(0x800555C8u);
            for(uint32_t i=0u;S(i)<S(chunk);i+=2u){const uint16_t v=s.Read16(source);source+=2u;s.Write16(base+0x1A8u,v);}
        }
        base=s.Read32(0x800555C8u);const uint16_t control=s.Read16(base+0x1AAu);
        s.Write16(base+0x1AAu,uint16_t((control&0xFFCFu)|0x10u));Delay240();
        StatusWait(s,0x400u,0u,0x80011830u);
        Delay240();Delay240();remaining-=chunk;
    }
    base=s.Read32(0x800555C8u);s.Write16(base+0x1AAu,uint16_t(s.Read16(base+0x1AAu)&0xFFCFu));
    StatusWait(s,0x7FFu,oldStatus&0x7FFu,0x80011844u);
}
int32_t ResetSpu8002961C(Services& s,uint32_t warm){
    const uint32_t priority=s.Read32(0x800555D8u);s.Write32(priority,s.Read32(priority)|0xB0000u);
    uint32_t base=s.Read32(0x800555C8u);
    s.Write16(base+0x180u,0u);s.Write16(base+0x182u,0u);s.Write16(base+0x1AAu,0u);
    s.Write32(0x800555E0u,0u);s.Write32(0x800555E4u,0u);s.Write16(0x800555C4u,0u);Delay240();
    base=s.Read32(0x800555C8u);s.Write16(base+0x180u,0u);s.Write16(base+0x182u,0u);
    StatusWait(s,0x7FFu,0u,0x80011820u);
    base=s.Read32(0x800555C8u);
    s.Write32(0x800555E8u,2u);s.Write32(0x800555ECu,3u);s.Write32(0x800555F0u,8u);s.Write32(0x800555F4u,7u);
    s.Write16(base+0x1ACu,4u);s.Write16(base+0x184u,0u);s.Write16(base+0x186u,0u);
    s.Write16(base+0x18Cu,65535u);s.Write16(base+0x18Eu,65535u);
    s.Write16(base+0x198u,0u);s.Write16(base+0x19Au,0u);
    if(!warm){
        for(uint32_t p:{0x190u,0x192u,0x194u,0x196u,0x1B0u,0x1B2u,0x1B4u,0x1B6u})s.Write16(base+p,0u);
        s.Write16(0x800555C4u,0x200u);WriteFifo80029B38(s,0x80055604u,16u);
        base=s.Read32(0x800555C8u);
        for(uint32_t i=0;i<24u;++i,base+=16u){
            s.Write16(base,0u);s.Write16(base+2u,0u);s.Write16(base+4u,0x3FFFu);
            s.Write16(base+6u,0x200u);s.Write16(base+8u,0u);s.Write16(base+10u,0u);
        }
        base=s.Read32(0x800555C8u);(void)s.Read16(base+0x188u);s.Write16(base+0x188u,65535u);
        s.Write16(base+0x18Au,uint16_t(s.Read16(base+0x18Au)|255u));
        for(uint32_t i=0;i<4u;++i)Delay240();
        base=s.Read32(0x800555C8u);(void)s.Read16(base+0x18Cu);s.Write16(base+0x18Cu,65535u);
        s.Write16(base+0x18Eu,uint16_t(s.Read16(base+0x18Eu)|255u));
        for(uint32_t i=0;i<4u;++i)Delay240();
    }
    base=s.Read32(0x800555C8u);s.Write32(0x800555F8u,1u);s.Write16(base+0x1AAu,0xC000u);
    s.Write32(0x800555FCu,0u);s.Write32(0x80055600u,0u);return 0;
}
int32_t SetDmaCallback8002ADBC(Services& s,uint32_t callback){return s.Call(0x800357A4u,{4u,callback});}
int32_t StartEvents8002AD38(Services& s){
    const uint32_t old=s.Read32(0x80055A80u);if(old)return S(old);
    s.Write32(0x80055A80u,1u);(void)s.Call(0x80048A40u,{});s.Write32(0x80055620u,0u);
    SetDmaCallback8002ADBC(s,0x80029E6Cu);
    const uint32_t event=U(s.Call(0x80048990u,{0xF0000009u,0x20u,0x2000u,0u}));
    s.Write32(0x80055678u,event);const int32_t result=s.Call(0x800489D0u,{event});
    s.CallVoid(0x80048A50u,{});return result;
}
int32_t SpuInitMode8002AC70(Services& s,uint32_t warm){
    s.Call(0x80035744u,{});ResetSpu8002961C(s,warm);
    if(!warm)for(uint32_t i=0;i<24u;++i)s.Write16(0x80055676u-2u*i,0xC000u);
    StartEvents8002AD38(s);const uint32_t reverb=s.Read32(0x80055A84u);
    s.Write32(0x80055628u,0u);s.Write32(0x8005562Cu,0u);s.Write32(0x80055638u,0u);
    s.Write16(0x8005563Cu,0u);s.Write16(0x8005563Eu,0u);s.Write32(0x80055640u,0u);s.Write32(0x80055644u,0u);
    s.Write32(0x80055630u,reverb);
    const int32_t result=PrStage2VabDirect::WriteRegister8002A584(s,0xD1u,reverb,0u);
    s.Write32(0x80055624u,0u);s.Write32(0x800555E0u,0u);s.Write32(0x8005567Cu,0u);return result;
}
int32_t SpuInit8002AC50(Services& s){return SpuInitMode8002AC70(s,0u);}
int32_t InitializeVoices8003226C(Services& s,uint32_t count){
    PrStage2VabDirect::SetTransferBusy8002EB44(s,0u);
    s.Write16(0x800928C8u,0u);s.Write16(0x80091688u,0u);
    PrStage2VabDirect::InitializeAllocator80035394(s,32u,0x80088268u);
    for(uint32_t i=0;i<192u;++i)s.Write16(0x80087BA8u+2u*i,0u);
    for(uint32_t i=0;i<24u;++i)s.Write8(0x80087D28u+i,0u);
    s.Write16(0x801C35F0u,0u);for(uint32_t i=0;i<16u;++i)s.Write8(0x800928F8u+i,0u);
    const uint32_t requested=count&255u;s.Write8(0x800928A0u,uint8_t(requested<24u?requested:24u));
    uint32_t i=0;
    if(s.Read8(0x800928A0u)!=0u)do{
        const uint32_t offset=52u*(i&65535u);
        s.Write16(0x80087D42u+offset,24u);s.Write16(0x80087D4Eu+offset,65535u);
        s.Write16(0x80087D40u+offset,255u);s.Write8(0x80087D5Bu+offset,0u);
        s.Write16(0x80087D44u+offset,0u);s.Write16(0x80087D46u+offset,0u);
        s.Write16(0x80087D50u+offset,0u);s.Write16(0x80087D52u+offset,0u);
        s.Write16(0x80087D54u+offset,255u);s.Write16(0x80087D48u+offset,0u);s.Write8(0x80087D4Au+offset,64u);
        for(uint32_t p:{0x80087D5Cu,0x80087D5Eu,0x80087D60u,0x80087D62u,0x80087D68u,0x80087D6Au,0x80087D6Cu,0x80087D6Eu,0x80087D70u,0x80087D64u})s.Write16(p+offset,0u);
        const uint32_t base=s.Read32(0x80055DD8u)+2u*((8u*(i&65535u))&65535u);
        s.Write16(base+6u,0x200u);s.Write16(base+4u,0x1000u);s.Write16(base+8u,0x80FFu);
        s.Write16(base,0u);s.Write16(base+2u,0u);s.Write16(base+10u,0x4000u);
        s.Write16(0x800928F2u,uint16_t(i));const uint32_t voice=s.Read16(0x800928F2u);
        const uint32_t low=voice<16u?(1u<<(voice&31u)):0u,high=voice<16u?0u:(1u<<((voice-16u)&31u));
        const uint32_t actualOffset=52u*voice;s.Write8(0x80087D5Bu+actualOffset,0u);
        const uint32_t offLow=s.Read16(0x801C386Cu),offHigh=s.Read16(0x801C386Eu);++i;
        s.Write16(0x80087D44u+actualOffset,0u);s.Write16(0x80087D40u+actualOffset,0u);
        const uint32_t onLow=s.Read16(0x8008EC98u),newLow=low|offLow,newHigh=high|offHigh;
        s.Write16(0x801C386Cu,uint16_t(newLow));s.Write16(0x801C386Eu,uint16_t(newHigh));
        s.Write16(0x8008EC98u,uint16_t(onLow&~newLow));
        const uint32_t onHigh=s.Read16(0x8008EC9Au),limit=s.Read8(0x800928A0u);
        s.Write16(0x8008EC9Au,uint16_t(onHigh&~newHigh));if((i&65535u)>=limit)break;
    }while(true);
    s.Write16(0x8008ECC8u,0x3FFFu);s.Write16(0x8008ECCAu,0x3FFFu);
    for(uint32_t p:{0x8008EC98u,0x8008EC9Au,0x801C386Cu,0x8008EC9Cu,0x8008EC9Eu})s.Write16(p,0u);
    s.Write32(0x8008ECC0u,0u);s.Write32(0x8008ECC4u,0u);s.Write8(0x8009290Cu,0u);
    s.Write16(0x80091728u,0u);s.Write16(0x800917A8u,128u);
    return PrStage2LifecycleDirect::CommitAudio80032B00(s);
}
int32_t InitializeLibrary8002ADE0(Services& s){
    uint32_t reg=0x1F801C00u;
    for(uint32_t voice=0;voice<24u;++voice)for(uint32_t i=0;i<8u;++i,reg+=2u)s.Write16(reg,s.Read16(0x80055D7Cu+2u*i));
    for(uint32_t i=0;i<16u;++i)s.Write16(0x1F801D80u+2u*i,s.Read16(0x80055D8Cu+2u*i));
    InitializeVoices8003226C(s,24u);
    for(uint32_t row=0;row<32u;++row)for(uint32_t i=16u;i>0u;--i)s.Write32(0x80095D88u+64u*row+4u*(i-1u),0u);
    s.Write32(0x80095C4Cu,60u);s.Write32(0x800928CCu,0u);s.Write32(0x800917A4u,0u);return 60;
}
int32_t SsInitHot8002AC20(Services& s){s.Call(0x80035744u,{});SpuInit8002AC50(s);return InitializeLibrary8002ADE0(s);}
void SetTickMode8002DA78(Services& s,uint32_t mode){
    const uint32_t video=U(s.Call(0x8003623Cu,{}));
    if(mode&0x1000u){s.Write32(0x80055DB0u,1u);s.Write32(0x80055DACu,mode&0xFFFu);}
    else{s.Write32(0x80055DB0u,0u);s.Write32(0x80055DACu,mode);}
    const uint32_t selected=s.Read32(0x80055DACu);
    switch(selected){
    case 0u:case 5u:s.Write32(0x80095C4Cu,video==1u?50u:60u);break;
    case 1u:s.Write32(0x80095C4Cu,60u);s.Write32(0x80055DACu,video==0u?5u:60u);break;
    case 2u:s.Write32(0x80095C4Cu,240u);break;
    case 3u:s.Write32(0x80095C4Cu,120u);break;
    case 4u:s.Write32(0x80095C4Cu,50u);s.Write32(0x80055DACu,video==1u?5u:50u);break;
    default:s.Write32(0x80095C4Cu,S(selected)>=6?selected:60u);break;
    }
}
int32_t StartTickMode8002AEC8(Services& s,uint32_t alternate){
    s.Write8(0x80055DBCu,0u);const uint32_t mode=s.Read32(0x80055DACu);
    s.Write8(0x80055DBEu,6u);s.Write8(0x80055DBDu,0u);s.Write32(0x80055DB8u,0u);
    uint32_t timer=0xF2000002u,count;
    if(mode==0u){s.Write8(0x80055DBEu,255u);return 255;}
    if(mode==5u){s.Write8(0x80055DBEu,0u);if(!alternate){s.Write8(0x80055DBCu,1u);count=0u;}else{timer=0xF2000003u;count=1u;}}
    else if(mode==3u)count=0x89D0u;
    else if(mode==2u)count=0x44E8u;
    else{
        const uint32_t manual=s.Read32(0x80055DB0u);if(manual)return S(manual);
        const uint32_t frequency=s.Read32(0x80055DACu);
        if(frequency==0u)s.Break(S(frequency)<70?0x8002AFE4u:0x8002B02Cu,7u);
        if(S(frequency)<70){count=U(0x204CC0/S(frequency));s.Write8(0x80055DBDu,uint8_t(s.Read8(0x80055DBDu)+1u));}
        else count=U(0x409980/S(frequency));
    }
    int32_t result;
    if(s.Read8(0x80055DBCu)!=0u){s.Call(0x80048A40u,{});result=s.Call(0x800357D4u,{s.Read32(0x80055DB4u)});}
    else{
        s.Call(0x80048A40u,{});s.Call(0x80048C40u,{timer});s.Call(0x80048B00u,{timer,count&65535u,0x1000u});
        uint32_t channel=s.Read8(0x80055DBEu),callback;
        if(channel==0u){const uint32_t previous=U(s.Call(0x80035774u,{0u,0u}));channel=s.Read8(0x80055DBEu);callback=0x8002B170u;s.Write32(0x80055DB8u,previous);}
        else callback=s.Read8(0x80055DBDu)?0x8002B1B0u:s.Read32(0x80055DB4u);
        result=s.Call(0x80035774u,{channel,callback});
    }
    s.CallVoid(0x80048A50u,{});return result;
}
int32_t StartTick8002B130(Services& s){return StartTickMode8002AEC8(s,0u);}
int32_t InitializeSound80026E4C(Services& s){
    SsInitHot8002AC20(s);SetTickMode8002DA78(s,0x1000u);StartTick8002B130(s);
    MasterVolume8002A6AC(s,90u,90u);SetMix8002AA90(s,0u,0u,1u);ExternalVolume8002AB24(s,0u,127u,127u);
    s.Write16(0x800943A8u,0u);s.Write16(0x800943AAu,65535u);s.Write16(0x800943ACu,65535u);s.Write32(0x800943B4u,0u);return -1;
}

int32_t TransferCommand8002A1EC(Services& s,uint32_t command,uint32_t first,uint32_t second){
    if(command==2u){
        const uint32_t units=first>>(s.Read32(0x800555ECu)&31u),base=s.Read32(0x800555C8u);
        s.Write16(0x800555C4u,uint16_t(units));s.Write16(base+0x1A6u,uint16_t(units));return 0;
    }
    if(command<=1u){
        uint32_t base=s.Read32(0x800555C8u);const uint16_t expected=s.Read16(0x800555C4u);uint16_t actual=s.Read16(base+0x1A6u);
        s.Write32(0x80055614u,command==0u?1u:0u);
        uint32_t n=0u;
        while(actual!=expected){if(++n>0xF00u)return -2;s.AwaitDeviceProgress(0x1F801DA6u);actual=s.Read16(base+0x1A6u);}
        base=s.Read32(0x800555C8u);const uint16_t old=s.Read16(base+0x1AAu);
        s.Write16(base+0x1AAu,uint16_t(command==1u?((old&0xFFCFu)|0x20u):(old|0x30u)));return 0;
    }
    if(command!=3u)return 0;
    const uint32_t expected=s.Read32(0x80055614u)==1u?0x30u:0x20u,base=s.Read32(0x800555C8u);
    uint32_t n=0u,value=s.Read16(base+0x1AAu)&0x30u;
    while(value!=expected){if(++n>0xF00u)return -2;s.AwaitDeviceProgress(0x1F801DAAu);value=s.Read16(base+0x1AAu)&0x30u;}
    const uint32_t readMode=s.Read32(0x80055614u),control=s.Read32(0x800555DCu);
    s.Write32(control,(s.Read32(control)&0xF0FFFFFFu)|(readMode==1u?0x22000000u:0x20000000u));
    const uint32_t dmaAddress=s.Read32(0x800555CCu);s.Write32(0x80055618u,first);
    const uint32_t blocks=(second>>6u)+((second&63u)!=0u?1u:0u),source=s.Read32(0x80055618u);
    s.Write32(0x8005561Cu,blocks);s.Write32(dmaAddress,source);
    const uint32_t encoded=s.Read32(0x8005561Cu),bcr=s.Read32(0x800555D0u);s.Write32(bcr,(encoded<<16u)|16u);
    const uint32_t transfer=s.Read32(0x80055614u),chcr=s.Read32(0x800555D4u);
    s.Write32(chcr,transfer==1u?0x01000200u:0x01000201u);return 0;
}
int32_t TransferWrite8002A494(Services& s,uint32_t source,uint32_t bytes){
    if(s.Read32(0x800555E0u)==0u){
        const uint32_t address=uint32_t(s.Read16(0x800555C4u))<<(s.Read32(0x800555ECu)&31u);
        TransferCommand8002A1EC(s,2u,address,0u);TransferCommand8002A1EC(s,1u,0u,0u);TransferCommand8002A1EC(s,3u,source,bytes);
    }else WriteFifo80029B38(s,source,bytes);
    return S(bytes);
}
void DmaInterrupt80029E6C(Services& s){
    if(s.Read32(0x80055614u)==0u)for(uint32_t i=0;i<3u;++i)Delay240();
    const uint32_t base=s.Read32(0x800555C8u);s.Write16(base+0x1AAu,uint16_t(s.Read16(base+0x1AAu)&0xFFCFu));
    uint32_t value=s.Read16(base+0x1AAu)&0x30u,n=0u;
    while(value){if(++n>0xF00u)break;s.AwaitDeviceProgress(0x1F801DAAu);value=s.Read16(base+0x1AAu)&0x30u;}
    if(s.Read32(0x800555FCu))s.CallVoid(s.Read32(0x800555FCu),{});
    else s.CallVoid(0x80048980u,{0xF0000009u,0x20u});
}

bool TryCall(Services& s,uint32_t fn,std::initializer_list<uint32_t> args,int32_t& result){
    size_t n;
    switch(fn){
    case 0x80026E4Cu:case 0x8002AC20u:case 0x8002AC50u:case 0x8002AD38u:case 0x8002ADE0u:case 0x8002B130u:n=0;break;
    case 0x8002AC70u:case 0x8002961Cu:case 0x8002ADBCu:case 0x8003226Cu:case 0x8002AEC8u:n=1;break;
    case 0x8002A494u:n=2;break;
    case 0x8002A1ECu:n=3;break;
    case 0x8002DA78u:case 0x8002A6FCu:case 0x8002A6ACu:case 0x8002AA90u:case 0x8002AB24u:case 0x80029B38u:case 0x80029E6Cu:
        throw std::invalid_argument("Source SPU effect-only call requires CallVoid");
    default:return false;
    }
    if(args.size()!=n)throw std::invalid_argument("SPU source argument count mismatch");
    const auto a=args.begin();
    switch(fn){
    case 0x80026E4Cu:result=InitializeSound80026E4C(s);break;
    case 0x8002AC20u:result=SsInitHot8002AC20(s);break;
    case 0x8002AC50u:result=SpuInit8002AC50(s);break;
    case 0x8002AD38u:result=StartEvents8002AD38(s);break;
    case 0x8002ADE0u:result=InitializeLibrary8002ADE0(s);break;
    case 0x8002B130u:result=StartTick8002B130(s);break;
    case 0x8002AC70u:result=SpuInitMode8002AC70(s,a[0]);break;
    case 0x8002961Cu:result=ResetSpu8002961C(s,a[0]);break;
    case 0x8002ADBCu:result=SetDmaCallback8002ADBC(s,a[0]);break;
    case 0x8003226Cu:result=InitializeVoices8003226C(s,a[0]);break;
    case 0x8002AEC8u:result=StartTickMode8002AEC8(s,a[0]);break;
    case 0x8002A494u:result=TransferWrite8002A494(s,a[0],a[1]);break;
    case 0x8002A1ECu:result=TransferCommand8002A1EC(s,a[0],a[1],a[2]);break;
    }
    return true;
}
bool TryCallVoid(Services& s,uint32_t fn,std::initializer_list<uint32_t> args){
    size_t n;
    switch(fn){
    case 0x80029E6Cu:n=0;break;
    case 0x8002DA78u:case 0x8002A6FCu:n=1;break;
    case 0x8002A6ACu:case 0x80029B38u:n=2;break;
    case 0x8002AA90u:case 0x8002AB24u:n=3;break;
    default:{int32_t ignored;return TryCall(s,fn,args,ignored);}
    }
    if(args.size()!=n)throw std::invalid_argument("Void SPU source argument count mismatch");const auto a=args.begin();
    switch(fn){
    case 0x80029E6Cu:DmaInterrupt80029E6C(s);break;
    case 0x8002DA78u:SetTickMode8002DA78(s,a[0]);break;
    case 0x8002A6FCu:CommonAttributes8002A6FC(s,a[0]);break;
    case 0x8002A6ACu:MasterVolume8002A6AC(s,a[0],a[1]);break;
    case 0x80029B38u:WriteFifo80029B38(s,a[0],a[1]);break;
    case 0x8002AA90u:SetMix8002AA90(s,a[0],a[1],a[2]);break;
    case 0x8002AB24u:ExternalVolume8002AB24(s,a[0],a[1],a[2]);break;
    }
    return true;
}
}
