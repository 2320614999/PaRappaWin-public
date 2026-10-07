#include "pr_stage2_ot_draw_backend.h"
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <string>
#include <unordered_set>

namespace PrStage2OtDrawBackend {
namespace {
[[noreturn]] void Fail(const char* reason, uint32_t address) {
    throw std::runtime_error(std::string("S2 native OT: ")+reason+" at "+std::to_string(address));
}
int16_t Signed16(uint32_t word) {
    const uint32_t v = word&0xFFFFu;
    return static_cast<int16_t>(v < 0x8000u ? static_cast<int32_t>(v) : static_cast<int32_t>(v)-65536);
}
int16_t Signed11(uint32_t word) {
    const uint32_t v = word&0x7FFu;
    return static_cast<int16_t>(v < 0x400u ? static_cast<int32_t>(v) : static_cast<int32_t>(v)-2048);
}
void Color(Vertex& v, uint32_t word) {
    v.r=static_cast<uint8_t>(word); v.g=static_cast<uint8_t>(word>>8u); v.b=static_cast<uint8_t>(word>>16u);
}
bool VariableRectangle(uint8_t opcode) {
    return opcode==0x60u || opcode==0x62u || (opcode>=0x64u && opcode<=0x67u);
}
uint32_t PacketWords(uint8_t opcode) {
    if ((opcode&0xE0u) == 0x20u) {
        const uint32_t count = (opcode&8u) ? 4u : 3u;
        return 1u+count*(1u+((opcode&4u) ? 1u : 0u))+((opcode&16u) ? count-1u : 0u);
    }
    if (opcode == 2u) return 3u;
    if ((opcode&0xE0u)==0x80u) return 4u;
    if (VariableRectangle(opcode)) return (opcode&4u)?4u:3u;
    if (opcode >= 0xE1u && opcode <= 0xE6u) return 1u;
    if (opcode == 0u) return 1u;
    return 0u;
}
Batch DecodeStream(const std::vector<std::pair<uint32_t,uint32_t>>& stream, DrawState state, uint32_t nodes);
}

Batch DecodeLinkedList(PrStage2LifecycleDirect::Services& s, uint32_t head, DrawState state) {
    Batch out;
    std::vector<std::pair<uint32_t,uint32_t>> stream;
    std::unordered_set<uint32_t> visited;
    uint32_t link = head&0xFFFFFFu;
    while (link != 0xFFFFFFu) {
        if ((link&3u) || link > 0x1FFFFCu) Fail("invalid RAM link",link);
        if (!visited.insert(link).second) Fail("cyclic DMA chain",link);
        const uint32_t tag = s.Read32(0x80000000u|link), count = tag>>24u;
        if (count > (0x1FFFFCu-link)/4u) Fail("packet beyond RAM",link);
        for (uint32_t i=0; i<count; ++i) {
            if (stream.size() >= 0x80000u) Fail("overlapping command stream too long",link);
            stream.emplace_back(0x80000000u|link,s.Read32(0x80000000u+link+4u+4u*i));
        }
        ++out.nodes;
        link = tag&0xFFFFFFu;
    }
    return DecodeStream(stream,state,out.nodes);
}
namespace {
Batch DecodeStream(const std::vector<std::pair<uint32_t,uint32_t>>& stream, DrawState state, uint32_t nodes) {
    Batch out;out.nodes=nodes;
    // DMA tags delimit transfers, not GP0 commands. A command can straddle
    // two tags; decode the resulting stream instead of assuming 1 tag=1 draw.
    for (size_t p=0; p<stream.size();) {
        const uint32_t word = stream[p].second;
        const uint8_t opcode = static_cast<uint8_t>(word>>24u);
        const uint32_t length = PacketWords(opcode);
        if (!length) Fail("unsupported GP0 opcode",stream[p].first);
        if (length > stream.size()-p) Fail("truncated GP0 command",stream[p].first);
        auto w = [&](uint32_t i) { return stream[p+i].second; };
        if (opcode == 0u) { ++p; continue; }
        if (opcode >= 0xE1u) {
            if (opcode == 0xE1u) {
                // Dithering remains the existing Windows high-color path;
                // texture-disable/rectangle flips require their own adapter.
                if (word&0x3800u) Fail("unsupported draw-mode flags",stream[p].first);
                state.tpage=static_cast<uint16_t>(word&0x1FFu);
                state.drawModeFlags=static_cast<uint16_t>(word&0x600u);
            } else if (opcode == 0xE2u) {
                if (word&0xFFFFFu) Fail("texture window not implemented",stream[p].first);
            } else if (opcode == 0xE3u) {
                state.clipLeft=static_cast<int16_t>(word&1023u);
                state.clipTop=static_cast<int16_t>((word>>10u)&511u);
            } else if (opcode == 0xE4u) {
                state.clipRight=static_cast<int16_t>(word&1023u);
                state.clipBottom=static_cast<int16_t>((word>>10u)&511u);
            } else if (opcode == 0xE5u) {
                state.offsetX=Signed11(word); state.offsetY=Signed11(word>>11u);
            } else if (word&3u) Fail("VRAM mask operation not implemented",stream[p].first);
            p+=length; continue;
        }
        Command command;
        command.packet=stream[p].first; command.wordOffset=static_cast<uint32_t>(p);
        command.opcode=opcode;
        if ((opcode&0xE0u)==0x80u) {
            // 原 GPU 复制命令的坐标/尺寸掩码；零尺寸编码表示整行/整列。
            command.copySourceX=uint16_t(w(1)&1023u);
            command.copySourceY=uint16_t((w(1)>>16u)&511u);
            command.copyDestinationX=uint16_t(w(2)&1023u);
            command.copyDestinationY=uint16_t((w(2)>>16u)&511u);
            command.fillWidth=uint16_t(((w(3)-1u)&1023u)+1u);
            command.fillHeight=uint16_t((((w(3)>>16u)-1u)&511u)+1u);
        } else if (opcode == 2u) {
            const uint32_t xy=w(1), size=w(2);
            // Stage2's native framebuffer clear uses aligned, in-bounds
            // rectangles. Do not silently approximate hardware wrap/rounding.
            command.vertex[0].x=static_cast<int16_t>(xy&0xFFFFu);
            command.vertex[0].y=static_cast<int16_t>(xy>>16u);
            command.fillWidth=static_cast<uint16_t>(size);
            command.fillHeight=static_cast<uint16_t>(size>>16u);
            if ((command.vertex[0].x&15) || (command.fillWidth&15u) ||
                command.vertex[0].x<0 || command.vertex[0].y<0 ||
                command.vertex[0].x+command.fillWidth>1024 || command.vertex[0].y+command.fillHeight>512)
                Fail("unsupported fill rectangle",command.packet);
            Color(command.vertex[0],word);
        } else if (VariableRectangle(opcode)) {
            // GsSortFastSprite and GsSortBoxFill emit variable rectangles.
            // Their tpage comes from the preceding E1, not a polygon UV word.
            const bool textured=(opcode&4u)!=0u;
            // Rectangle coordinates and extents are the packed GPU fields:
            // XY uses signed 11-bit coordinates, while size is 10-bit wide
            // by 9-bit high. Upper packet bits are not part of the geometry.
            const int16_t x=Signed11(w(1)), y=Signed11(w(1)>>16u);
            const uint32_t size=w(textured?3u:2u);
            const uint32_t width=size&0x3FFu, height=(size>>16u)&0x1FFu;
            const uint32_t uv=textured?w(2):0u;
            const uint32_t u=uv&255u, v=(uv>>8u)&255u;
            // Native PC sprites sample the source texel coordinates directly,
            // including the odd-U glyphs produced by original 8001B954. Do not
            // rewrite their RAM packets, or emulate GPU sampling glitches.
            // Texture-window wrapping still needs a distinct native mapping;
            // never hide that missing mapping by clamping its coordinates.
            if (textured && (u+width>256u || v+height>256u))
                Fail("unsupported rectangle UV wrap",command.packet);
            command.clut=static_cast<uint16_t>(uv>>16u);
            command.fillWidth=static_cast<uint16_t>(width);
            command.fillHeight=static_cast<uint16_t>(height);
            command.vertices=4u;
            for (uint32_t i=0;i<4u;++i) {
                auto& vertex=command.vertex[i]; Color(vertex,word);
                vertex.x=static_cast<int16_t>(int32_t(x)+int32_t((i&1u)?width:0u));
                vertex.y=static_cast<int16_t>(int32_t(y)+int32_t((i&2u)?height:0u));
                vertex.u=static_cast<uint8_t>(u); vertex.v=static_cast<uint8_t>(v);
            }
        } else {
            const bool textured=(opcode&4u)!=0u, gouraud=(opcode&16u)!=0u;
            command.vertices=(opcode&8u) ? 4u : 3u;
            uint32_t cursor=1u, color=word;
            for (uint32_t i=0; i<command.vertices; ++i) {
                if (i && gouraud) color=w(cursor++);
                Vertex& v=command.vertex[i]; Color(v,color);
                const uint32_t xy=w(cursor++);
                v.x=Signed16(xy); v.y=Signed16(xy>>16u);
                if (v.x < -1024 || v.x > 1023 || v.y < -1024 || v.y > 1023)
                    Fail("noncanonical projected coordinate",command.packet);
                if (textured) {
                    const uint32_t uv=w(cursor++);
                    v.u=static_cast<uint8_t>(uv); v.v=static_cast<uint8_t>(uv>>8u);
                    if (i==0u) command.clut=static_cast<uint16_t>(uv>>16u);
                    if (i==1u) state.tpage=static_cast<uint16_t>((uv>>16u)&0x1FFu);
                }
            }
        }
        command.state=state;
        out.commands.push_back(command); p+=length;
    }
    out.finalState=state;
    return out;
}

}
Batch DecodeCommandStream(const std::vector<uint32_t>& words, DrawState state) {
    if(words.size()>0x80000u)Fail("port command stream too large",0x1F801810u);
    std::vector<std::pair<uint32_t,uint32_t>> stream;stream.reserve(words.size());
    for(uint32_t word:words)stream.emplace_back(0x1F801810u,word);
    return DecodeStream(stream,state,0u);
}
namespace {
struct FloatVertex { float x,y,u,v,r,g,b; };
FloatVertex Mix(const FloatVertex& a,const FloatVertex& b,float t) {
    return {a.x+(b.x-a.x)*t,a.y+(b.y-a.y)*t,a.u+(b.u-a.u)*t,a.v+(b.v-a.v)*t,
            a.r+(b.r-a.r)*t,a.g+(b.g-a.g)*t,a.b+(b.b-a.b)*t};
}
std::vector<FloatVertex> Clip(std::vector<FloatVertex> input,int axis,float bound,bool lower) {
    std::vector<FloatVertex> out;
    if (input.empty()) return out;
    auto coordinate=[&](const FloatVertex& v) { return axis ? v.y : v.x; };
    auto inside=[&](float v) { return lower ? v>=bound : v<=bound; };
    FloatVertex a=input.back();
    for (const auto& b:input) {
        const float x=coordinate(a), y=coordinate(b);
        if (inside(x)!=inside(y)) out.push_back(Mix(a,b,(bound-x)/(y-x)));
        if (inside(y)) out.push_back(b);
        a=b;
    }
    return out;
}
struct Prepared { Command command; ComPtr<ID3D11ShaderResourceView> texture; std::vector<FloatVertex> triangles; };
}

RenderResult Render(const Batch& batch,PrStage2VramAtlas::Projection& atlas,
                    D3D11Renderer& renderer,const Viewport& view) {
    if (view.frameWidth<=0 || view.frameHeight<=0 || !(view.width>0) || !(view.height>0) ||
        !std::isfinite(view.x) || !std::isfinite(view.y) || !std::isfinite(view.width) || !std::isfinite(view.height))
        Fail("invalid viewport",0u);
    std::vector<Prepared> prepared;
    for (const auto& c:batch.commands) {
        // 显存复制必须由持有两个页图像的 GPU owner 按命令顺序消费。
        if((c.opcode&0xE0u)==0x80u)Fail("VRAM copy requires the GPU page owner",c.packet);
        Prepared draw; draw.command=c;
        const bool textured=c.opcode!=2u && (c.opcode&4u);
        const bool rectangle=VariableRectangle(c.opcode);
        if (rectangle && (!c.fillWidth || !c.fillHeight)) continue;
        float left=static_cast<float>(view.frameX), top=static_cast<float>(view.frameY);
        float right=left+view.frameWidth, bottom=top+view.frameHeight;
        const bool expanded=c.host && c.host->expandedViewport;
        if(expanded) {
            left-=view.x*view.frameWidth/view.width;
            top-=view.y*view.frameHeight/view.height;
            right+=(renderer.GetWidth()-view.x-view.width)*view.frameWidth/view.width;
            bottom+=(renderer.GetHeight()-view.y-view.height)*view.frameHeight/view.height;
        }
        if (c.opcode!=2u) {
            if(!expanded || c.state.clipLeft>view.frameX)left=(std::max)(left,float(c.state.clipLeft));
            if(!expanded || c.state.clipTop>view.frameY)top=(std::max)(top,float(c.state.clipTop));
            if(!expanded || c.state.clipRight+1<view.frameX+view.frameWidth)right=(std::min)(right,float(c.state.clipRight)+1.0f);
            if(!expanded || c.state.clipBottom+1<view.frameY+view.frameHeight)bottom=(std::min)(bottom,float(c.state.clipBottom)+1.0f);
        }
        if (left>=right || top>=bottom) continue;
        std::array<FloatVertex,4> v{};
        if (c.opcode==2u) {
            const auto& a=c.vertex[0];
            for (uint32_t i=0; i<4u; ++i)
                v[i]={float(a.x)+((i&1u)?c.fillWidth:0),float(a.y)+((i&2u)?c.fillHeight:0),0,0,
                      a.r/255.0f,a.g/255.0f,a.b/255.0f};
        } else {
            if (c.vertices!=3u && c.vertices!=4u) Fail("invalid decoded polygon",c.packet);
            for (uint32_t i=0; i<c.vertices; ++i) {
                const auto& a=c.vertex[i]; const bool raw=textured && (c.opcode&1u);
                const float scale=textured ? 128.0f : 255.0f;
                v[i]={float(a.x)+c.state.offsetX,float(a.y)+c.state.offsetY,
                      (a.u+0.5f)/256.0f,(a.v+0.5f)/256.0f,
                      raw?1.0f:a.r/scale,raw?1.0f:a.g/scale,raw?1.0f:a.b/scale};
                if (rectangle && textured) {
                    // Rectangle UVs refer to texel edges. Do not apply the
                    // polygon half-texel endpoint rule: that distorts 4x tiles.
                    v[i].u=(float(a.u)+((i&1u)?c.fillWidth:0u))/256.0f;
                    v[i].v=(float(a.v)+((i&2u)?c.fillHeight:0u))/256.0f;
                }
            }
        }
        if(c.host) for(uint32_t i=0;i<c.vertices;++i) {
            const auto& h=c.host->vertex[i];
            v[i]={h.x,h.y,h.u,h.v,h.r,h.g,h.b};
        }
        constexpr uint32_t indices[6]={0u,1u,2u,2u,1u,3u};
        for (uint32_t first=0; first<((c.opcode==2u || c.vertices==4u)?6u:3u); first+=3u) {
            std::vector<FloatVertex> triangle{v[indices[first]],v[indices[first+1u]],v[indices[first+2u]]};
            triangle=Clip(Clip(Clip(Clip(std::move(triangle),0,left,true),0,right,false),1,top,true),1,bottom,false);
            for (size_t i=1; i+1<triangle.size(); ++i) {
                for (const auto& point:{triangle[0],triangle[i],triangle[i+1]}) draw.triangles.push_back(point);
            }
        }
        if (draw.triangles.empty()) continue;
        if(c.host && c.host->textureOverride) draw.texture=c.host->texture;
        else if (textured) {
            uint8_t minU=255u,maxU=0,minV=255u,maxV=0;
            for (uint32_t i=0; i<c.vertices; ++i) {
                minU=(std::min)(minU,c.vertex[i].u); maxU=(std::max)(maxU,c.vertex[i].u);
                minV=(std::min)(minV,c.vertex[i].v); maxV=(std::max)(maxV,c.vertex[i].v);
            }
            if (rectangle) {
                maxU=static_cast<uint8_t>(uint32_t(minU)+c.fillWidth-1u);
                maxV=static_cast<uint8_t>(uint32_t(minV)+c.fillHeight-1u);
            }
            // PSX STP is consulted only when the primitive's GP0 command
            // enables semi-transparency (bit 1).  Using the STP-preserving
            // atlas for every textured primitive makes ordinary model faces
            // half-transparent and lets behind-the-model pixels bleed
            // through, which looks like an OT priority failure.  Opaque
            // commands use the normal palette projection; the real
            // semi-transparent path below keeps the STP split.
            const bool semi = c.opcode != 2u && (c.opcode & 2u) != 0u;
            const auto encoding = semi
                ? PrStage2VramAtlas::Encoding::Abr0Stp
                : PrStage2VramAtlas::Encoding::Opaque;
            draw.texture=atlas.Resolve(c.state.tpage,c.clut,{minU,minV,maxU-minU+1,maxV-minV+1},renderer,
                                        encoding);
            if (!draw.texture) {
                const std::string detail="unknown texture region or GPU allocation failure tpage="+
                    std::to_string(c.state.tpage)+" clut="+std::to_string(c.clut)+
                    " uv="+std::to_string(minU)+","+std::to_string(minV)+
                    " size="+std::to_string(maxU-minU+1)+"x"+std::to_string(maxV-minV+1);
                Fail(detail.c_str(),c.packet);
            }
        }
        prepared.push_back(std::move(draw));
    }
    renderer.FlushSprites(); // Preserve preceding host draws, then native chain order.
    RenderResult out;
    for (const auto& draw:prepared) {
        ++out.commands; out.triangles+=static_cast<uint32_t>(draw.triangles.size()/3u);
        const auto& c=draw.command;
        const bool hostTexture=c.host && c.host->textureOverride;
        const bool semi=!hostTexture && c.opcode!=2u && (c.opcode&2u);
        const uint32_t abr=(c.state.tpage>>5u)&3u;
        const auto blend=abr==0u ? D3D11Renderer::BlendMode::Alpha :
                         abr==2u ? D3D11Renderer::BlendMode::Subtractive : D3D11Renderer::BlendMode::Additive;
        const float alpha=abr==0u ? 0.5f : abr==3u ? 0.25f : 1.0f;
        std::vector<TexturedVertex> tv; std::vector<ColorVertex> cv;
        for (const auto& v:draw.triangles) {
            const float x=view.x+(v.x-view.frameX)*view.width/view.frameWidth;
            const float y=view.y+(v.y-view.frameY)*view.height/view.frameHeight;
            tv.push_back({x,y,v.u,v.v,v.r,v.g,v.b,c.host?c.host->alpha:1.0f});
            cv.push_back({x,y,v.r,v.g,v.b,semi?alpha:1.0f});
        }
        if (!draw.texture) {
            // GP0 has no back-face culling. The original model producer already
            // applies NCLIP where required; note flips and folded quads must
            // not be culled a second time by D3D's default rasterizer.
            renderer.DrawTriangleBatch(cv.data(),static_cast<int>(cv.size()),semi?blend:D3D11Renderer::BlendMode::Alpha,true);
            ++out.gpuDraws;
        } else {
            if (!semi) {
                renderer.DrawTexturedTriangleBatch(draw.texture.Get(),tv.data(),static_cast<int>(tv.size()),
                    hostTexture?c.host->blend:D3D11Renderer::BlendMode::Alpha,D3D11Renderer::TextureMask::All,true);
                ++out.gpuDraws;
            } else {
                // Keep original triangle order even for overlapping quads;
                // grouping every opaque pass ahead of every STP pass would
                // change how the second triangle blends with the first.
                for (size_t i=0; i<tv.size(); i+=3u) {
                    renderer.DrawTexturedTriangleBatch(draw.texture.Get(),tv.data()+i,3,
                        D3D11Renderer::BlendMode::Alpha,D3D11Renderer::TextureMask::NonStpOnly,true);
                    for (size_t j=0; j<3u; ++j) tv[i+j].a=alpha;
                    renderer.DrawTexturedTriangleBatch(draw.texture.Get(),tv.data()+i,3,blend,
                                                      D3D11Renderer::TextureMask::StpOnly,true);
                    out.gpuDraws+=2u;
                }
            }
        }
    }
    return out;
}
}
