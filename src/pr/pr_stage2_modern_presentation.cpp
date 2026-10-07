#include "pr_stage2_modern_presentation.h"
#include "pr_game_context.h"
#include "pr_modern_rail_animation.h"
#include "pr_display_viewport.h"
#include "pr_stage1_hd_subtitles.h"
#include "pr_stage1_p2_scorer_hud.h"
#include "pr_stage1_texture_replacements.h"
#include "logger.h"
#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <string>
#include <unordered_map>
#include <vector>

namespace {
namespace B=PrStage2OtDrawBackend;
using Clock=std::chrono::steady_clock;
using Texture=ComPtr<ID3D11ShaderResourceView>;
uint64_t Identity(uint32_t a,uint32_t b,uint32_t c) {
    uint64_t h=1469598103934665603ull;
    for(auto v:{a,b,c}) h=(h^v)*1099511628211ull;
    return h;
}
float Seconds(Clock::time_point a,Clock::time_point b) {return std::chrono::duration<float>(a-b).count();}
bool Rectangle(const B::Command& c) {return (c.opcode&0xE0u)==0x60u;}
bool OnPage(const B::Command& c,const B::Viewport& v) {
    if(c.opcode==2u) return c.vertex[0].y<int(v.frameY+v.frameHeight) &&
        int(c.vertex[0].y)+c.fillHeight>v.frameY;
    return c.state.clipTop<v.frameY+v.frameHeight && c.state.clipBottom>=v.frameY;
}
std::shared_ptr<B::HostPrimitive> Host(const B::Command& c) {
    auto h=std::make_shared<B::HostPrimitive>();
    const bool tex=c.opcode&4u,raw=tex&&(c.opcode&1u),rect=Rectangle(c);
    for(unsigned i=0;i<c.vertices;++i) {
        if(c.opcode==2u) {
            const auto& a=c.vertex[0];
            h->vertex[i]={float(a.x)+((i&1u)?c.fillWidth:0),float(a.y)+((i&2u)?c.fillHeight:0),
                0,0,a.r/255.0f,a.g/255.0f,a.b/255.0f};
            continue;
        }
        const auto& a=c.vertex[i];auto& v=h->vertex[i];
        v.x=float(a.x)+c.state.offsetX;v.y=float(a.y)+c.state.offsetY;
        v.u=(float(a.u)+(rect?((i&1u)?c.fillWidth:0):0.5f))/256.0f;
        v.v=(float(a.v)+(rect?((i&2u)?c.fillHeight:0):0.5f))/256.0f;
        const float denom=tex?128.0f:255.0f;
        v.r=raw?1.0f:a.r/denom;v.g=raw?1.0f:a.g/denom;v.b=raw?1.0f:a.b/denom;
    }
    return h;
}
B::Command Quad(const B::Command& anchor,ID3D11ShaderResourceView* texture,
    float x,float y,float w,float h,float u0=0,float v0=0,float u1=1,float v1=1) {
    B::Command out=anchor;out.opcode=0x2Du;out.vertices=4;
    out.host=std::make_shared<B::HostPrimitive>();out.host->texture=texture;out.host->textureOverride=true;
    out.fillWidth=out.fillHeight=0;
    for(unsigned i=0;i<4;++i) out.host->vertex[i]={x+((i&1)?w:0),y+((i&2)?h:0),
        (i&1)?u1:u0,(i&2)?v1:v0,1,1,1};
    return out;
}
void Tint(B::Command& c,float r,float g,float b) {
    if(!c.host)c.host=Host(c);
    for(auto& v:c.host->vertex){v.r*=r;v.g*=g;v.b*=b;}
}
struct Bounds {float x=1e9f,y=1e9f,right=-1e9f,bottom=-1e9f;};
Bounds Extent(const B::Command& c) {
    Bounds b;const auto h=c.host?c.host:Host(c);
    for(unsigned i=0;i<c.vertices;++i){const auto& v=h->vertex[i];b.x=(std::min)(b.x,v.x);b.y=(std::min)(b.y,v.y);b.right=(std::max)(b.right,v.x);b.bottom=(std::max)(b.bottom,v.y);}
    return b;
}
}

struct PrStage2ModernPresentation::Impl {
    PrGameContext& ctx;
    PrStage2LifecycleDirect::Services& memory;
    enum class Kind { Model, Note, Portrait, Caption, Status, Other };
    struct Snapshot;
    struct Meta { std::shared_ptr<Snapshot> frame; Kind kind=Kind::Model;uint64_t id=0;uint32_t source=0;int x=0,y=0,type=0,row=0;uint8_t primitiveMode=0;std::string text; };
    struct Row {int student=-1,teacher=-1;uint32_t signature=0;};
    struct Stamp {int row=0,type=0;float x=0,y=0;uint64_t frame=0,id=0;};
    struct Snapshot {
        uint64_t serial=0,scoreFrame=0,promptStart=0,promptLeave=0;
        uint32_t work=0,promptKey=0,settledPrompt=0;
        int layout=0,totalScore=0,rating=1;
        std::array<int32_t,4> scores{};
        int32_t totalDelta=0;
        bool active=false,railVisible=false,scoreVisible=false,promptVisible=false,promptLeaving=false;
        std::array<Row,2> rows{};
        std::vector<Stamp> stamps;
    };
    std::shared_ptr<Snapshot> sourceFrame;
    uint64_t renderSerial=0;
    struct Motion {std::array<B::HostVertex,4> vertex{};uint8_t count=0,opcode=0;uint16_t page=0,clut=0;float alpha=1;};
    struct Frame {uint64_t serial=0;Clock::time_point time{};Texture base;B::Batch batch;std::unordered_map<uint64_t,Motion> previous;};
    std::unordered_map<uint32_t,Meta> metadata;
    std::unordered_map<uint64_t,Motion> previousMotion,currentMotion;
    std::array<Frame,2> frames;
    std::array<Row,2> rows;
    std::vector<Stamp> stamps;
    uint64_t serial=0,stampSerial=0,scoreFrame=0,modernPresents=0;
    uint32_t work=0;
    int layout=0,totalScore=0,rating=1;
    std::array<int32_t,4> scores{};
    int32_t totalDelta=0;
    bool active=false,railVisible=false,railOpen=false;
    bool transition=false;
    bool scoreVisible=false;
    uint32_t promptKey=0,settledPrompt=0;
    uint64_t promptStart=0,promptLeave=0;
    bool promptVisible=false,promptLeaving=false;
    Clock::time_point frameTime=Clock::now();
    Texture glow;
    struct TextureMatch {Texture texture;};
    std::unordered_map<uint64_t,TextureMatch> textureMatches;
    uint64_t textureRevision=~uint64_t(0);
    uint64_t subtitleCount=0,textureHits=0,noteCount=0,scoreCount=0;
    Impl(PrGameContext& c,PrStage2LifecycleDirect::Services& m):ctx(c),memory(m){}
    Snapshot Capture() const {
        Snapshot p;
        p.serial=serial;
        p.scoreFrame=scoreFrame;
        p.promptStart=promptStart;
        p.promptLeave=promptLeave;
        p.work=work;
        p.promptKey=promptKey;
        p.settledPrompt=settledPrompt;
        p.layout=layout;
        p.totalScore=totalScore;
        p.rating=rating;
        p.scores=scores;
        p.totalDelta=totalDelta;
        p.active=active;
        p.railVisible=railVisible;
        p.scoreVisible=scoreVisible;
        p.promptVisible=promptVisible;
        p.promptLeaving=promptLeaving;
        p.rows=rows;
        p.stamps=stamps;
        return p;
    }
    void Apply(const Snapshot& p) {
        serial=p.serial;
        scoreFrame=p.scoreFrame;
        promptStart=p.promptStart;
        promptLeave=p.promptLeave;
        work=p.work;
        promptKey=p.promptKey;
        settledPrompt=p.settledPrompt;
        layout=p.layout;
        totalScore=p.totalScore;
        rating=p.rating;
        scores=p.scores;
        totalDelta=p.totalDelta;
        active=p.active;
        railVisible=p.railVisible;
        scoreVisible=p.scoreVisible;
        promptVisible=p.promptVisible;
        promptLeaving=p.promptLeaving;
        rows=p.rows;
        stamps=p.stamps;
    }
    void SyncSnapshot() { if(sourceFrame)*sourceFrame=Capture(); }
    struct ScopedSnapshot {
        Impl& owner;Snapshot saved;
        ScopedSnapshot(Impl& s,const Snapshot& p):owner(s),saved(s.Capture()){owner.Apply(p);}
        ~ScopedSnapshot(){owner.Apply(saved);}
    };
    int H(uint32_t p){return int16_t(memory.Read16(p));}
    void Range(uint32_t first,uint32_t end,const Meta& meta) {
        if(first>=end || end-first>65536u) return;
        for(uint32_t p=first;p<end;) {
            metadata[p]=meta;
            const uint32_t words=memory.Read32(p)>>24u;
            const uint32_t next=p+4u*(words+1u);
            if(next<=p || next>end)break;
            p=next;
        }
    }
    void Reset() {
        frames={};previousMotion.clear();currentMotion.clear();metadata.clear();sourceFrame.reset();renderSerial=0;
        rows={};stamps.clear();railVisible=false;scoreVisible=false;
        promptVisible=false;promptLeaving=false;promptKey=settledPrompt=0;
        scores={};totalDelta=0;scoreFrame=0;
    }
    void UpdateRows() {
        if(!work)return;
        railVisible=active && H(work+122u)==1;
        const int count=std::clamp(H(work+138u),0,2);
        for(int r=0;r<2;++r) {
            Row next;
            if(r<count && railVisible) {
                next.student=H(work+158u+2u*r);next.teacher=H(work+140u+2u*r);
                const auto notes=memory.Read32(work+148u+4u*r);
                if(notes) for(unsigned n=0;n<18;++n)next.signature=(next.signature^memory.Read8(notes+n))*16777619u;
            }
            const auto& old=rows[r];
            if(next.student<0 || old.student<0 || next.student<old.student || next.signature!=old.signature)
                stamps.erase(std::remove_if(stamps.begin(),stamps.end(),[r](const Stamp& s){return s.row==r;}),stamps.end());
            rows[r]=next;
        }
    }
    void UpdatePrompt() {
        if(!active || !ctx.presentation.railCreativePrompt || !railVisible || rating==0 || rating>=3 ||
           (memory.Read32(work)&0x140u)!=0u) {promptVisible=false;promptLeaving=false;return;}
        const uint32_t descriptor=memory.Read32(work+64u);
        uint32_t mask=descriptor?memory.Read32(descriptor+8u)&0xFCu:0u;
        unsigned buttons=0;for(;mask;mask&=mask-1u)++buttons;
        const bool wants=buttons>=2 && descriptor!=settledPrompt;
        if(wants) {
            if(!promptVisible || promptKey!=descriptor) {
                promptVisible=true;promptLeaving=false;promptKey=descriptor;promptStart=serial;
            }
            if(serial-promptStart>=240u) {settledPrompt=descriptor;if(!promptLeaving){promptLeaving=true;promptLeave=serial;}}
        }else if(promptVisible && !promptLeaving) {promptLeaving=true;promptLeave=serial;}
        if(promptLeaving && serial-promptLeave>=28u)promptVisible=false;
    }
    ID3D11ShaderResourceView* Glow() {
        if(glow)return glow.Get();
        const auto pixels=PrModernRailAnimation::GlowPixels();
        glow.Attach(ctx.renderer->CreateTexture(pixels.data(),64,64));return glow.Get();
    }
    B::Command Sprite(const B::Command& anchor,PrStage2VramAtlas::Projection& atlas,
        uint32_t source,float cx,float cy,float sx=1,float sy=1) {
        const uint16_t tx=memory.Read16(source+4u),ty=memory.Read16(source+6u);
        const int w=memory.Read16(source+8u),h=memory.Read16(source+10u);
        const uint16_t page=uint16_t(((tx>>6u)&15u)|((ty>>4u)&16u));
        const uint16_t clut=uint16_t((memory.Read16(source+14u)<<6u)|(memory.Read16(source+12u)>>4u));
        const int u=uint8_t(tx*4u),v=uint8_t(ty);
        if(auto* replacement=FindTexture(atlas,page,clut,u,v,w,h)) {
            auto cmd=Quad(anchor,replacement,cx-w*sx/2,cy-h*sy/2,w*sx,h*sy);
            cmd.state.tpage=page;cmd.clut=clut;
            return cmd;
        }
        auto* texture=atlas.Resolve(page,clut,{u,v,w,h},*ctx.renderer,PrStage2VramAtlas::Encoding::Opaque);
        if(!texture)throw std::runtime_error("S2 modern native sprite projection unavailable");
        auto cmd=Quad(anchor,texture,cx-w*sx/2,cy-h*sy/2,w*sx,h*sy,float(u)/256,float(v)/256,float(u+w)/256,float(v+h)/256);
        cmd.state.tpage=page;cmd.clut=clut;
        return cmd;
    }
    void Remember(B::Command& c,uint64_t id) {
        if(!c.host)c.host=Host(c);
        c.host->motionIdentity=id;
        currentMotion[id]={c.host->vertex,c.vertices,c.opcode,c.state.tpage,c.clut,c.host->alpha};
    }
    ID3D11ShaderResourceView* FindTexture(PrStage2VramAtlas::Projection& atlas,
        uint16_t page,uint16_t clut,int x,int y,int w,int h) {
        if(!ctx.presentation.textureReplacements || w<=0 || h<=0 || x+w>256 || y+h>256)return nullptr;
        const auto revision=atlas.Revision();
        if(textureRevision!=revision){textureMatches.clear();textureRevision=revision;}
        const uint64_t key=Identity(uint32_t(page)|(uint32_t(clut)<<16u),uint32_t(x)|(uint32_t(y)<<16u),uint32_t(w)|(uint32_t(h)<<16u));
        const auto cached=textureMatches.find(key);
        if(cached!=textureMatches.end()) {
            if(cached->second.texture)++textureHits;
            return cached->second.texture.Get();
        }
        textureMatches[key]={};
        std::vector<uint32_t> pixels;
        if(!atlas.CopyRgbaRect(page,clut,{x,y,w,h},pixels))return nullptr;
        PrStage1TextureReplacements::ReplacementTexture replacement;
        if(!PrStage1TextureReplacements::TryFindContentReplacement(ctx,pixels.data(),w,h,w,replacement))return nullptr;
        textureMatches[key].texture=replacement.srv;++textureHits;
        return replacement.srv;
    }
    void ReplaceTexture(B::Command& c,PrStage2VramAtlas::Projection& atlas) {
        if(!ctx.presentation.textureReplacements || !(c.opcode&4u) || (c.opcode&2u) || c.opcode==2u ||
           (c.host && c.host->textureOverride))return;
        int x=255,y=255,right=0,bottom=0;
        for(unsigned i=0;i<c.vertices;++i){const auto& v=c.vertex[i];x=(std::min)(x,int(v.u));y=(std::min)(y,int(v.v));right=(std::max)(right,int(v.u));bottom=(std::max)(bottom,int(v.v));}
        if(Rectangle(c)){right=x+c.fillWidth-1;bottom=y+c.fillHeight-1;}
        if(right>255 || bottom>255)return;
        const int w=right-x+1,h=bottom-y+1;
        auto* replacement=FindTexture(atlas,c.state.tpage,c.clut,x,y,w,h);
        if(!replacement)return;
        if(!c.host)c.host=Host(c);
        c.host->texture=replacement;c.host->textureOverride=true;
        for(auto& v:c.host->vertex){v.u=(v.u*256-x)/w;v.v=(v.v*256-y)/h;}
    }
    void Layout(B::Command& c,const B::Viewport& view,Kind kind,uint8_t primitiveMode=0) {
        if(ctx.presentation.aspectMode!=1)return;
        if(!c.host)c.host=Host(c);
        c.host->expandedViewport=true;
        const float extraX=view.x*view.frameWidth/view.width;
        const float extraY=view.y*view.frameHeight/view.height;
        const auto box=Extent(c);
        // The stage sky is often a large textured quad rather than a solid
        // GP0 clear. Treat only broad, tall textured quads as background so
        // they can fill the host's extra horizontal area; small textured
        // packets remain in the native 4:3 presentation space (this includes
        // transition/movie/UI artwork).
        const bool texturedBackground=kind==Kind::Other && c.vertices==4u &&
            !transition && (c.opcode&4u) &&
            box.right-box.x>=view.frameWidth*0.5f &&
            box.bottom-box.y>=view.frameHeight*0.5f;
        // Some S2 skies are emitted by the native TMD path as a large
        // untextured gradient quad, so its metadata is Model even though it
        // is a background layer. Textured models (car/characters) stay out.
        const bool modelSky=kind==Kind::Model && c.vertices>=3u && c.vertices<=4u &&
            !(c.opcode&4u) && box.right-box.x>=view.frameWidth*0.5f &&
            box.bottom-box.y>=view.frameHeight*0.5f;
        const bool skyTile=kind==Kind::Model && !transition && primitiveMode==0x2Du;
        const bool background=(c.vertices==3u || c.vertices==4u) &&
            ((kind!=Kind::Model && (c.opcode==2u || !(c.opcode&4u) || texturedBackground)) || modelSky);
        const bool fullX=background &&
            (texturedBackground || (box.x<=view.frameX && box.right>=view.frameX+view.frameWidth));
        const bool fullY=background && box.y<=view.frameY && box.bottom>=view.frameY+view.frameHeight;
        // COOL starts at y=137; the central lesson banner at y=107 must stay centered.
        const bool edgeHud=active && kind==Kind::Status && box.y-view.frameY>=128;
        // Only the small rectangle cells belong to the native tile curtain.
        // Other transition packets include the movie/UI animation and must
        // stay in the normal 4:3 presentation space.
        const bool tile=transition && kind==Kind::Other && !background &&
            Rectangle(c) && c.fillWidth>=16u && c.fillWidth<=24u &&
            c.fillHeight>=16u && c.fillHeight<=24u;
        if(!fullX && !fullY && !edgeHud && !tile && !skyTile)return;
        if(!c.host)c.host=Host(c);
        const float centerX=view.frameX+view.frameWidth*0.5f;
        const float centerY=view.frameY+view.frameHeight*0.5f;
        const float cover=(std::max)(float(ctx.renderer->GetWidth())/view.width,
                                    float(ctx.renderer->GetHeight())/view.height);
        for(unsigned i=0;i<c.vertices;++i) {
            auto& v=c.host->vertex[i];
            if(fullX)v.x=v.x<centerX?float(view.frameX)-extraX:float(view.frameX+view.frameWidth)+extraX;
            if(fullY)v.y=v.y<centerY?float(view.frameY)-extraY:float(view.frameY+view.frameHeight)+extraY;
            if(edgeHud)v.x+=((box.x+box.right)*0.5f<centerX?-1.0f:1.0f)*extraX*0.75f;
            // A transition mask covers the window with uniform scaling; its
            // icon cells keep their shape while the underlying scene is held.
            if(tile){v.x=centerX+(v.x-centerX)*cover;v.y=centerY+(v.y-centerY)*cover;}
            // HYSK is emitted as a set of small native 0x2D sky tiles. Spread
            // those tiles across the host-wide background while retaining
            // their original shape and depth ordering.
            if(skyTile)v.x=centerX+(v.x-centerX)*cover;
        }
    }
    void AddRail(B::Batch& out,const B::Command& anchor,PrStage2VramAtlas::Projection& atlas,int pageY) {
        if(!railVisible || ctx.presentation.railMode!=1)return;
        for(const auto& stamp:stamps) {
            const auto pose=PrModernRailAnimation::Sample(float(serial-stamp.frame),ctx.presentation.railPopFrames,
                ctx.presentation.railPopScale,ctx.presentation.railFlipFrames,ctx.presentation.railGlowFadeFrames);
            const uint32_t source=memory.Read32(0x800540BCu+4u*stamp.type);
            const float cx=stamp.x,cy=stamp.y+pageY;
            auto note=Sprite(anchor,atlas,source,cx,cy,pose.x,pose.y);
            if(note.host->texture) {Remember(note,Identity(0xF100u,uint32_t(stamp.id),1));out.commands.push_back(std::move(note));}
            // Shared with Stage1: scale the halo with the impact, then add its
            // light over the note so the enlarged opaque face cannot hide it.
            if(pose.glow>0 && ctx.presentation.railGlowAlpha>0) {
                const float base=float((std::max)(memory.Read16(source+8u),memory.Read16(source+10u)));
                const auto layers=PrModernRailAnimation::GlowLayers(base,pose.x,pose.y,
                    ctx.presentation.railGlowScale,pose.glow*ctx.presentation.railGlowAlpha);
                for(unsigned i=0;i<layers.size();++i) {
                    const auto& layer=layers[i];
                    auto light=Quad(anchor,Glow(),cx-layer.size/2,cy-layer.size/2,layer.size,layer.size);
                    light.host->alpha=layer.alpha;light.host->blend=D3D11Renderer::BlendMode::Additive;
                    Tint(light,layer.r,layer.g,layer.b);
                    Remember(light,Identity(0xF100u,uint32_t(stamp.id),2+i));out.commands.push_back(std::move(light));
                }
            }
        }
    }
    void AddHud(B::Batch& out,const B::Command& anchor,int pageY) {
        if(!active || !scoreVisible || ctx.presentation.railMode!=1)return;
        if(ctx.presentation.railScorerHud) {
            const float yOffset=layout==0?30.0f:0.0f;
            for(unsigned i=0;i<4;++i) {
                const auto label=PrStage1P2ScorerHud::SharedLabel(ctx,i);
                const float y=142+yOffset+8*i+pageY;
                const float pulse=std::clamp(1.0f-float(serial-scoreFrame)/24.0f,0.0f,1.0f);
                const float h=6,w=label.height>0?(std::min)(33.0f,h*label.width/label.height):0;
                if(label.srv && w>0)out.commands.push_back(Quad(anchor,label.srv,0,y,w,h,label.u0,label.v0,label.u1,label.v1));
                const auto value=PrStage1HdSubtitles::RasterizeNativeText(ctx,(scores[i]>0?"+":"")+std::to_string(scores[i]),{},false,6.5f);
                if(value.srv){auto cmd=Quad(anchor,value.srv,36,y-1,value.width,value.height);if(scores[i]>0)Tint(cmd,1,1-0.15f*pulse,1-0.65f*pulse);out.commands.push_back(std::move(cmd));}
            }
            const float alpha=std::clamp(1.0f-float(serial-scoreFrame)/36.0f,0.0f,1.0f);
            if(totalDelta && alpha>0) {
                const auto value=PrStage1HdSubtitles::RasterizeNativeText(ctx,(totalDelta>0?"+":"")+std::to_string(totalDelta),{},false,6.5f);
                if(value.srv){auto cmd=Quad(anchor,value.srv,61+9*float(std::to_string(totalScore).size()),176+yOffset+pageY,value.width,value.height);cmd.host->alpha=alpha;Tint(cmd,1,totalDelta>0?0.95f:0.42f,totalDelta>0?0.35f:0.32f);out.commands.push_back(std::move(cmd));}
            }
        }
        if(promptVisible && ctx.presentation.railCreativePrompt) {
            const auto label=PrStage1P2ScorerHud::SharedCreativePrompt(ctx);
            if(label.srv && label.width>0) {
                float sx=1,sy=1,alpha=1;
                constexpr float pi=3.14159265358979323846f;
                if(promptLeaving){const float t=std::clamp(float(serial-promptLeave)/28,0.0f,1.0f);sx=sy=(std::max)(0.12f,1+0.62f*std::sin(pi*t)-0.86f*t*t);alpha=1-t;}
                else{const float t=std::clamp(float(serial-promptStart)/40,0.0f,1.0f),bounce=std::exp(-3.2f*t)*std::sin(4*pi*t),scale=1-0.55f*std::exp(-8*t)+0.30f*bounce;sx=scale*(1+0.075f*bounce);sy=scale*(1-0.095f*bounce);}
                const float height=86*label.height/label.width;
                auto prompt=Quad(anchor,label.srv,160-43*sx,42-(height*sy-height)/2+pageY,86*sx,height*sy);
                prompt.host->alpha=alpha;Remember(prompt,Identity(0xF200u,promptKey,0));out.commands.push_back(std::move(prompt));
            }
        }
    }
};

PrStage2ModernPresentation::PrStage2ModernPresentation(PrGameContext& c,PrStage2LifecycleDirect::Services& m):impl_(std::make_unique<Impl>(c,m)) {
    Log::Printf("S2 presentation master=%u rail=%d render60=%u subtitles=%u textures=%u aspect=%d window=%dx%d",unsigned(c.presentation.enabled),c.presentation.railMode,unsigned(c.presentation.render60fps),unsigned(c.presentation.hdSubtitles),unsigned(c.presentation.textureReplacements),c.presentation.aspectMode,c.renderer->GetWidth(),c.renderer->GetHeight());
}
PrStage2ModernPresentation::~PrStage2ModernPresentation()=default;
void PrStage2ModernPresentation::SetGameplay(bool active){auto& s=*impl_;s.active=active;s.transition=false;if(!active)s.Reset();}
void PrStage2ModernPresentation::SetTransition(bool active){SetGameplay(false);impl_->transition=active;}
void PrStage2ModernPresentation::ConfigureViewport(B::Viewport& view) const {
    const auto& s=*impl_;
    const float w=float(s.ctx.renderer->GetWidth()),h=float(s.ctx.renderer->GetHeight());
    view.x=view.y=0;view.width=w;view.height=h;
    if(s.ctx.presentation.aspectMode==0)return;
    const auto fit=PrDisplayViewport::PreserveAspect(w,h,float(view.frameWidth),float(view.frameHeight));
    view.x=fit.x;view.y=fit.y;view.width=fit.width;view.height=fit.height;
}
void PrStage2ModernPresentation::BeginFrame(uint32_t work,int32_t kind) {
    auto& s=*impl_;s.SyncSnapshot();s.work=work;++s.serial;s.frameTime=Clock::now();
    s.scoreVisible=false;s.railOpen=false;
    for(auto it=s.metadata.begin();it!=s.metadata.end();) {
        if(it->second.frame && it->second.frame->serial+4u<s.serial)it=s.metadata.erase(it);
        else ++it;
    }
    s.sourceFrame=std::make_shared<Impl::Snapshot>();
    if(kind!=7 || !s.active){s.stamps.clear();s.SyncSnapshot();return;}
    s.totalScore=int32_t(s.memory.Read32(work+48u));s.rating=s.H(work+78u);
    s.UpdateRows();s.UpdatePrompt();s.SyncSnapshot();
}
void PrStage2ModernPresentation::Tmd(uint32_t packet,uint32_t descriptor,uint32_t object,uint32_t primitive) {
    auto& m=impl_->metadata[packet];m.frame=impl_->sourceFrame;m.kind=Impl::Kind::Model;m.id=Identity(descriptor,object,primitive);
    m.primitiveMode=impl_->memory.Read8(primitive+3u);
}
void PrStage2ModernPresentation::Rail(uint32_t work,bool begin){auto& s=*impl_;s.work=work;s.railOpen=begin;if(begin)s.UpdateRows();s.SyncSnapshot();}
void PrStage2ModernPresentation::Note(uint32_t packet,uint16_t x,uint16_t y,uint16_t type) {
    auto& s=*impl_;auto& m=s.metadata[packet];m.frame=s.sourceFrame;m.kind=Impl::Kind::Note;m.x=x;m.y=y;m.type=type;m.row=(int(y)-24)/20;
}
void PrStage2ModernPresentation::Portrait(uint32_t packet,uint32_t source,int32_t x,int32_t y) {
    auto& s=*impl_;auto& m=s.metadata[packet];m.frame=s.sourceFrame;m.kind=Impl::Kind::Portrait;m.source=source;m.x=x;m.y=y;m.row=(y-16)/20;
}
void PrStage2ModernPresentation::Caption(uint32_t text,uint32_t first,uint32_t end) {
    auto& s=*impl_;
    if(!s.ctx.presentation.hdSubtitles || s.ctx.subtitleFlag==0 || text<0x801C3870u || text>=0x801C6870u)return;
    Impl::Meta m;m.frame=s.sourceFrame;m.kind=Impl::Kind::Caption;m.id=Identity(text,first,0);
    for(unsigned n=0;n<512;++n){const auto ch=s.memory.Read8(text+n);if(!ch)break;m.text.push_back(char(ch));}
    if(!m.text.empty())s.Range(first,end,m);
}
void PrStage2ModernPresentation::Status(uint32_t work,int32_t layout,uint32_t first,uint32_t end) {
    auto& s=*impl_;s.work=work;s.layout=layout;s.scoreVisible=true;
    Impl::Meta m;m.frame=s.sourceFrame;m.kind=Impl::Kind::Status;s.Range(first,end,m);s.SyncSnapshot();
}
void PrStage2ModernPresentation::Input(uint32_t pad,uint32_t type,int32_t result,uint32_t work) {
    auto& s=*impl_;s.work=work;
    if(!s.active || s.ctx.presentation.railMode!=1 || !(pad&0xFCu) || type<1 || type>8)return;
    s.UpdateRows();
    for(int row=0;row<2;++row)if(s.railVisible && s.rows[row].student>=0) {
        const int tick=int32_t(s.memory.Read32(work+16u));
        float slot=float((tick%384+384)%384)/24.0f;
        const float portraitX=15.0f*s.rows[row].student+32+4*std::clamp(s.H(0x8006EB78u),0,4);
        float x=s.ctx.presentation.railCoreAlign?32+15*slot:portraitX;
        x-=15*s.ctx.presentation.railLeadSlots;
        s.stamps.push_back({row,int(type),x,float(24+20*row),s.serial,++s.stampSerial});
        if(s.stamps.size()>128)s.stamps.erase(s.stamps.begin());
        ++s.noteCount;
        if(s.ctx.presentation.railTraceAlign)Log::PrintfNoFlush("S2 modern stamp id=%llu tick=%d row=%d type=%u x=%.2f judge=%d",s.stampSerial,tick,row,type,x,result);
    }
    s.SyncSnapshot();
}
void PrStage2ModernPresentation::Score(uint32_t work,int32_t flow,int32_t rhyme,int32_t drop,int32_t hype,int32_t total) {
    auto& s=*impl_;s.work=work;s.scores={{flow,rhyme,drop,hype}};s.totalDelta=total;s.scoreFrame=s.serial;++s.scoreCount;
    if(s.promptVisible){s.settledPrompt=s.promptKey;s.promptLeaving=true;s.promptLeave=s.serial;}
    s.SyncSnapshot();
    Log::PrintfNoFlush("S2 modern score flow=%d rhyme=%d drop=%d hype=%d total=%d",flow,rhyme,drop,hype,total);
}

B::Batch PrStage2ModernPresentation::Prepare(const B::Batch& native,PrStage2VramAtlas::Projection& atlas,
    const B::Viewport& view,ID3D11ShaderResourceView* base) {
    if(!impl_->ctx.presentation.enabled)return native;
    auto& s=*impl_;B::Batch out;out.finalState=native.finalState;out.nodes=native.nodes;
    const unsigned page=unsigned(view.frameY/view.frameHeight);
    if(page>=2)return native;
    std::shared_ptr<Impl::Snapshot> selected;
    for(const auto& c:native.commands)if(OnPage(c,view)) {
        const auto it=s.metadata.find(c.packet);
        if(it!=s.metadata.end() && it->second.frame &&
           (!selected || it->second.frame->serial>selected->serial))selected=it->second.frame;
    }
    // Native DMA can consume a queued OT after the next source iteration has
    // begun. Packet provenance retains its own immutable logical snapshot.
    const auto fallback=s.Capture();
    Impl::ScopedSnapshot source(s,selected?*selected:fallback);
    if(selected && s.active && s.renderSerial!=s.serial) {
        s.previousMotion=std::move(s.currentMotion);s.currentMotion.clear();s.renderSerial=s.serial;
    }
    auto& frame=s.frames[page];
    bool touched=false,hasRail=false,hasStatus=false;
    B::Command railAnchor{},statusAnchor{};
    std::unordered_map<uint64_t,Bounds> captionBounds;
    for(const auto& c:native.commands) {
        if(!OnPage(c,view))continue;
        const auto it=s.metadata.find(c.packet);
        if(it==s.metadata.end() || it->second.kind!=Impl::Kind::Caption)continue;
        const auto box=Extent(c);auto& all=captionBounds[it->second.id];
        all.x=(std::min)(all.x,box.x);all.y=(std::min)(all.y,box.y);all.right=(std::max)(all.right,box.right);all.bottom=(std::max)(all.bottom,box.bottom);
    }
    std::unordered_map<uint64_t,bool> replacedCaptions;
    for(const auto& original:native.commands) {
        if(!OnPage(original,view))continue;
        touched=true;auto c=original;
        const auto found=s.metadata.find(c.packet);
        if(found!=s.metadata.end()) {
            const auto& m=found->second;
            if(m.kind==Impl::Kind::Caption) {
                const auto done=replacedCaptions.find(m.id);
                if(done!=replacedCaptions.end() && done->second)continue;
                const auto text=PrStage1HdSubtitles::RasterizeNativeText(s.ctx,m.text,s.ctx.presentation.subtitleFiles[1],true);
                if(text.srv) {
                    const auto& box=captionBounds[m.id];
                    const float x=std::clamp((box.x+box.right-text.width)/2,float(view.frameX),float(view.frameX+view.frameWidth)-text.width);
                    const float y=std::clamp((box.y+box.bottom-text.height)/2,float(view.frameY)+2,float(view.frameY+view.frameHeight)-text.height-4);
                    c=Quad(c,text.srv,x,y,text.width,text.height);replacedCaptions[m.id]=true;++s.subtitleCount;
                }
            }else if(m.kind==Impl::Kind::Note) {
                hasRail=true;railAnchor=c;
                if(s.active && s.ctx.presentation.railMode==1 && m.row>=0 && m.row<2 && s.rows[m.row].student>=0) {
                    c=s.Sprite(c,atlas,s.memory.Read32(0x800540BCu+4u*m.type),float(m.x),float(m.y+view.frameY));
                    const float shade=std::clamp(s.ctx.presentation.railDarken,0.0f,1.0f);Tint(c,shade,shade,shade);
                }
            }else if(m.kind==Impl::Kind::Portrait) {
                hasRail=true;railAnchor=c;
                if(s.active && s.ctx.presentation.railMode==1 && m.source==0x8005400Cu) {
                    c.host=Host(c);
                    float offset=-15*s.ctx.presentation.railLeadSlots;
                    if(s.ctx.presentation.railCoreAlign) {
                        const int tick=int32_t(s.memory.Read32(s.work+12u));
                        const float core=32+15*float((tick%384+384)%384)/24;
                        const auto box=Extent(c);offset+=core-(box.x+box.right)/2;
                    }
                    for(auto& v:c.host->vertex)v.x+=offset;
                    s.Remember(c,Identity(0xF300u,m.source,unsigned(m.row)));
                    if(s.ctx.presentation.railAssist) {
                        const auto box=Extent(c);const float x=(box.x+box.right)/2,y=float(24+20*m.row+view.frameY);
                        auto marker=Quad(c,s.Glow(),x-9,y-6,18,12);marker.host->alpha=0.25f;marker.host->blend=D3D11Renderer::BlendMode::Additive;
                        Tint(marker,0.25f,0.9f,1);out.commands.push_back(std::move(marker));
                    }
                }
            }else if(m.kind==Impl::Kind::Model && s.active && s.ctx.presentation.render60fps) {
                s.Remember(c,m.id);
            }else if(m.kind==Impl::Kind::Status){hasStatus=true;statusAnchor=c;}
        }
        s.ReplaceTexture(c,atlas);
        s.Layout(c,view,found!=s.metadata.end()?found->second.kind:Impl::Kind::Other,
                 found!=s.metadata.end()?found->second.primitiveMode:0);
        if(c.host && c.host->motionIdentity)s.Remember(c,c.host->motionIdentity);
        out.commands.push_back(std::move(c));
    }
    // A movie may recycle the same packet arena without PrepareFrame801CA57C.
    // Retire consumed caption tags so a later border/tile cannot inherit them.
    for(const auto& group:captionBounds) {
        for(auto it=s.metadata.begin();it!=s.metadata.end();) {
            if(it->second.kind==Impl::Kind::Caption && it->second.id==group.first)it=s.metadata.erase(it);
            else ++it;
        }
    }
    const auto railStart=out.commands.size();
    if(hasRail)s.AddRail(out,railAnchor,atlas,view.frameY);
    for(size_t i=railStart;i<out.commands.size();++i){auto& c=out.commands[i];s.Layout(c,view,Impl::Kind::Note);if(c.host && c.host->motionIdentity)s.Remember(c,c.host->motionIdentity);}
    const auto hudStart=out.commands.size();
    if(hasStatus)s.AddHud(out,statusAnchor,view.frameY);
    for(size_t i=hudStart;i<out.commands.size();++i){auto& c=out.commands[i];s.Layout(c,view,Impl::Kind::Status);if(c.host && c.host->motionIdentity)s.Remember(c,c.host->motionIdentity);}
    if(touched && s.active && selected) {
        if(frame.serial!=s.serial){frame={};frame.serial=s.serial;frame.base=base;frame.previous=s.previousMotion;}
        frame.time=Clock::now();
        frame.batch.commands.insert(frame.batch.commands.end(),out.commands.begin(),out.commands.end());
        // Bound retained host work even if a malformed source never advances.
        if(frame.batch.commands.size()>32768u)frame={};
    }
    return out;
}

bool PrStage2ModernPresentation::Present(PrStage2VramAtlas::Projection& atlas,D3D11Renderer& renderer,const B::Viewport& view) {
    auto& s=*impl_;if(!s.active || !s.ctx.presentation.render60fps)return false;
    const unsigned page=unsigned(view.frameY/view.frameHeight);if(page>=2)return false;
    const auto& frame=s.frames[page];
    if(!frame.base || frame.batch.commands.empty() || frame.serial+2u<s.serial)return false;
    const float now=Seconds(Clock::now(),frame.time);
    if(now>0.15f)return false;
    const float t=std::clamp(now*30.0f,0.0f,1.0f);
    // The completed page already contains t=1. Replaying it only adds GPU
    // work and can steal the next native update's presentation interval.
    if(t>=1.0f)return false;
    const float w=float(renderer.GetWidth()),h=float(renderer.GetHeight());
    const bool cropBase=impl_->ctx.presentation.aspectMode==1;
    const float bx=cropBase?view.x:0.0f,by=cropBase?view.y:0.0f;
    const float bw=cropBase?view.width:w,bh=cropBase?view.height:h;
    const float u0=cropBase?view.x/w:0.0f,v0=cropBase?view.y/h:0.0f;
    const float u1=cropBase?(view.x+view.width)/w:1.0f,v1=cropBase?(view.y+view.height)/h:1.0f;
    const TexturedVertex quad[]={{bx,by,u0,v0,1,1,1,1},{bx+bw,by,u1,v0,1,1,1,1},{bx,by+bh,u0,v1,1,1,1,1},
        {bx,by+bh,u0,v1,1,1,1,1},{bx+bw,by,u1,v0,1,1,1,1},{bx+bw,by+bh,u1,v1,1,1,1,1}};
    renderer.DrawTexturedTriangleBatch(frame.base.Get(),quad,6);
    B::Batch draw=frame.batch;
    unsigned interpolated=0,moving=0,matched=0;
    for(auto& c:draw.commands) {
        if(!c.host || !c.host->motionIdentity)continue;
        ++moving;
        const auto found=frame.previous.find(c.host->motionIdentity);if(found==frame.previous.end())continue;
        ++matched;const auto& previous=found->second;
        if(previous.count!=c.vertices || previous.opcode!=c.opcode || previous.page!=c.state.tpage || previous.clut!=c.clut)continue;
        bool compatible=true;
        for(unsigned i=0;i<c.vertices;++i) {
            const auto& a=previous.vertex[i];const auto& b=c.host->vertex[i];
            // The framebuffer page changes every source frame. Compare local Y.
            const float dy=std::remainder(b.y-a.y,240.0f);
            if(std::abs(b.x-a.x)>32 || std::abs(dy)>32 || std::abs(a.u-b.u)>0.0001f || std::abs(a.v-b.v)>0.0001f)compatible=false;
        }
        if(!compatible)continue;
        c.host=std::make_shared<B::HostPrimitive>(*c.host);
        for(unsigned i=0;i<c.vertices;++i) {
            const auto& a=previous.vertex[i];auto& b=c.host->vertex[i];
            b.x=a.x+(b.x-a.x)*t;b.y-=std::remainder(b.y-a.y,240.0f)*(1-t);
        }
        c.host->alpha=previous.alpha+(c.host->alpha-previous.alpha)*t;++interpolated;
    }
    B::Render(draw,atlas,renderer,view);
    if((++s.modernPresents%120u)==0u)Log::PrintfNoFlush("S2 modern present frames=%llu source=%llu interpolated=%u phase=%.3f moving=%u matched=%u previous=%zu stamps=%llu scores=%llu subtitles=%llu textures=%llu",
        s.modernPresents,frame.serial,interpolated,t,moving,matched,frame.previous.size(),s.noteCount,s.scoreCount,s.subtitleCount,s.textureHits);
    return true;
}
void PrStage2ModernPresentation::Invalidate(){auto& s=*impl_;s.frames={};s.previousMotion.clear();s.currentMotion.clear();}
