#pragma once
#include "pr_stage2_movie_setup_session.h"
#include "pr_stage2_cd_command_direct.h"
#include "pr_stage2_cd_command_device.h"
#include <cstdio>
#include <string>

namespace PrStage2CdCommandSession {
class Session:public PrStage2MovieSetupSession::Session {
public:
    Session(const std::filesystem::path& scus,const std::filesystem::path& overlay,
            const std::filesystem::path& disc,D3D11Renderer& renderer,IAudioSink& sink)
        :Base(scus,overlay,disc,renderer,sink),cd_(disc,Interrupts()){}
    PrStage2CdCommandDevice::Device& Cd()noexcept{return cd_;}
    bool CdInitialized()const noexcept{return cdInitialized_;}
protected:
    virtual int32_t CdDependency(uint32_t,std::initializer_list<uint32_t>)=0;
    // A native foreground may defer streaming data until its real receiver
    // has been configured. Already accepted SDK commands are independent.
    virtual bool AllowNewStreamSector(){return true;}
private:
    using Base=PrStage2MovieSetupSession::Session;
    using Args=std::initializer_list<uint32_t>;
    PrStage2CdCommandDevice::Device cd_;
    bool cdInitialized_=false;
    std::string Text(uint32_t address){
        std::string out;for(uint32_t i=0;i<512u;++i){const auto c=Read8(address+i);if(!c)return out;out.push_back(char(c));}
        throw std::runtime_error("Unterminated original CD diagnostic text");
    }
    bool Diagnostic(uint32_t fn,Args args,int32_t& result){
        const auto a=args.begin();std::string text;
        if(fn==0x80047F4Cu&&args.size()==1u&&
           (a[0]==0x80011E18u||a[0]==0x80011D28u||a[0]==0x80011D54u||a[0]==0x80011D7Cu))text=Text(a[0]);
        else if(fn==0x80047FFCu&&!args.size())return false;
        else if(fn==0x80047FFCu&&args.size()>=2u){
            char b[512]{};int n=-1;
            if(a[0]==0x80011E24u&&args.size()==2u)n=std::snprintf(b,sizeof(b),"addr=%08x\n",a[1]);
            else if((a[0]==0x80011DC4u||a[0]==0x80011DCCu)&&args.size()==2u){const auto s=Text(a[1]);n=std::snprintf(b,sizeof(b),a[0]==0x80011DC4u?"%s...\n":"%s: no param\n",s.c_str());}
            else if(a[0]==0x80011D38u&&args.size()==5u){const auto one=Text(a[1]),two=Text(a[2]),three=Text(a[3]),four=Text(a[4]);n=std::snprintf(b,sizeof(b),"%s:(%s) Sync=%s, Ready=%s\n",one.c_str(),two.c_str(),three.c_str(),four.c_str());}
            else if(a[0]==0x80011D60u&&args.size()==4u){const auto s=Text(a[1]);n=std::snprintf(b,sizeof(b),"com=%s,code=(%02x:%02x)\n",s.c_str(),a[2],a[3]);}
            else if(a[0]==0x80011D90u&&args.size()==2u)n=std::snprintf(b,sizeof(b),"(%d)\n",int(a[1]));
            else return false;
            if(n<0||size_t(n)>=sizeof(b))throw std::runtime_error("CD diagnostic formatting failed");text.assign(b,size_t(n));
        }else return false;
        const size_t written=std::fwrite(text.data(),1,text.size(),stderr);
        if(written!=text.size())throw std::runtime_error("CD diagnostic write failed");result=int32_t(written);return true;
    }
    int32_t MovieDependency(uint32_t fn,Args args)override{
        int32_t value;if(PrStage2CdCommandDirect::TryCall(*this,fn,args,value))return value;
        if(Diagnostic(fn,args,value))return value;
        return CdDependency(fn,args);
    }
    void InitializeAdditionalDevices()override{
        if(cdInitialized_)throw std::logic_error("CD bootstrap cannot run twice");
        // Explicit host attachment configuration, not a translated BIOS claim.
        // The old PSX library assumes the BIOS has enabled CD host IRQs. Our
        // concrete device starts masked; configure that real register here,
        // then let the original routine build all RAM state and callbacks.
        Write8(0x1F801800u,1u);Write8(0x1F801802u,7u);Write8(0x1F801800u,0u);
        if(Call(0x80037CB0u,{})!=0)throw std::runtime_error("Original CD library initialization failed");
        cdInitialized_=true;
    }
protected:
    // Derived transfer owners extend these exact device/event hooks without
    // reimplementing CD commands or bypassing the retained scene owner.
    uint8_t ReadDevice8(uint32_t a)override{uint32_t v;if(cd_.TryRead(a,1u,v))return uint8_t(v);return Base::ReadDevice8(a);}
    void WriteDevice8(uint32_t a,uint8_t v)override{if(!cd_.TryWrite(a,1u,v))Base::WriteDevice8(a,v);}
    uint32_t ReadDevice32(uint32_t a)override{uint32_t v;if(cd_.TryRead(a,4u,v))return v;return Base::ReadDevice32(a);}
    void WriteDevice32(uint32_t a,uint32_t v)override{if(!cd_.TryWrite(a,4u,v))Base::WriteDevice32(a,v);}
    bool ServiceAdditionalDevices()override{
        const bool previous=Base::ServiceAdditionalDevices();
        // Do not inject an unlimited sequence of new optical reads into an
        // IRQ activation that is still draining its previous sector. The
        // retained foreground must regain control before the next producer.
        // Command replies and DMA already in progress are not held by this.
        const bool cd=cd_.Service(AllowNewStreamSector());return previous||cd;
    }
    void AwaitDeviceProgress(uint32_t a)override{
        if((a&0x1FFFFFFFu)==0x1F801800u){
            if(NativeIrqActive()){cd_.Service(AllowNewStreamSector());return;}
            WaitDevice(a,uint32_t(cd_.Serial()));
        }
        else Base::AwaitDeviceProgress(a);
    }
};
}
