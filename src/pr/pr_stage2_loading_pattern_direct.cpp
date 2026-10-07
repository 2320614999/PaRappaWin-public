#include "pr_stage2_loading_pattern_direct.h"

namespace PrStage2LoadingPatternDirect {
namespace {
int32_t Signed(uint32_t value) {
    return value<=0x7FFFFFFFu?static_cast<int32_t>(value)
        :static_cast<int32_t>(static_cast<int64_t>(value)-0x100000000LL);
}
struct OriginalCalls final : DrawSink {
    Memory& memory;
    explicit OriginalCalls(Memory& m):memory(m){}
    void Box(uint32_t x,uint32_t y,uint32_t w,uint32_t h,uint32_t c,uint32_t p) override {
        (void)memory.Call(0x8001C4ECu,{x,y,w,h,c,p});
    }
    void Sprite(uint32_t x,uint32_t y,uint32_t source,uint32_t p) override {
        (void)memory.Call(0x8001C550u,{x,y,source,p});
    }
};
}
int32_t DrawPattern8001EF40(Memory& s,DrawSink& draw,uint32_t mode,uint32_t priority) {
    const uint32_t style=mode==2u?1u:mode==3u?2u:mode==4u?3u:0u;
    const uint32_t frame=s.Read32(0x8006EB0Cu);
    if(frame%3u==0u) {
        // SLTI is signed, while MULTU's division by three above is unsigned.
        if(Signed(s.Read32(0x8006EB04u))>=32) {
            const uint32_t word=s.Read32(0x8006EB08u);
            s.Write32(0x8006EB04u,0u);
            s.Write32(0x8006EB08u,word+1u);
        }
        const uint32_t word=s.Read32(0x8006EB08u);
        if(word==s.Read32(0x800508B4u+style*8u)) {
            s.Write32(0x8006EB08u,0u);
            s.Write32(0x8006EB04u,0u);
        }
        const uint32_t wordOffset=s.Read32(0x8006EB08u)*4u;
        const uint32_t bit=31u-s.Read32(0x8006EB04u);
        const uint32_t mask=1u<<(bit&31u); // Original SLLV masks the count.
        constexpr uint32_t patterns[]={0x80050730u,0x80050790u,0x800507F0u,0x80050850u};
        for(uint32_t row=0;row<12u;++row) {
            const uint32_t grid=0x80087330u+row*64u;
            for(uint32_t col=0;col<15u;++col)
                s.Write32(grid+col*4u,s.Read32(grid+(col+1u)*4u));
            const uint32_t value=s.Read32(patterns[style]+row*8u+wordOffset);
            s.Write32(grid+60u,(value&mask)!=0u?1u:0u);
        }
        s.Write32(0x8006EB04u,s.Read32(0x8006EB04u)+1u);
    }
    s.Write32(0x8006EB0Cu,s.Read32(0x8006EB0Cu)+1u);
    for(uint32_t row=0;row<12u;++row) {
        for(uint32_t col=0;col<16u;++col) {
            const uint32_t slot=row*16u+col;
            if(s.Read32(0x80087330u+slot*4u)!=0u)
                draw.Box(col*20u,row*20u,20u,20u,s.Read32(0x800508B0u+style*8u),priority);
            // Lower drawing may mutate the template table. Read it only now.
            const uint32_t tile=s.Read32(0x80050420u+slot*4u);
            const uint32_t source=s.Read32(0x80050720u+tile*4u);
            draw.Sprite(col*20u,row*20u,source,priority);
        }
    }
    return 0; // Final SLTI row,12 with row==12, independent of lower returns.
}
int32_t DrawPattern8001EF40(Memory& s,uint32_t mode,uint32_t priority) {
    OriginalCalls calls(s);
    return DrawPattern8001EF40(s,calls,mode,priority);
}
}
