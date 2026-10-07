#include "pr/pr_native_movie_frame.h"
#include <algorithm>
#include <array>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <vector>
namespace {
uint32_t checks = 0, negatives = 0;
void Check(bool b, const char* m) { ++checks; if (!b) throw std::runtime_error(m); }
template<class F> void Reject(F f) {
    try { f(); } catch (const std::exception&) { ++negatives; return; }
    throw std::runtime_error("Invalid codec request was accepted");
}
uint32_t Read(const std::vector<uint8_t>& b) {
    return uint32_t(b[0]) | (uint32_t(b[1]) << 8u) |
           (uint32_t(b[2]) << 16u) | (uint32_t(b[3]) << 24u);
}
void Word(std::vector<uint8_t>& b, size_t at, uint32_t v) {
    for (uint32_t i=0;i<4u;++i) b.at(at+i)=uint8_t(v >> (8u*i));
}
void Save(const char* name, const uint8_t* p, size_t n) {
    std::ofstream f(name,std::ios::binary); f.write(reinterpret_cast<const char*>(p),std::streamsize(n));
    Check(bool(f), "Output evidence write failed");
}
void SyntheticDecodeTests() {
    // Codec reuse is tested with nonuniform chroma/luma as well as the actual
    // white source frame. Returning a blank image cannot satisfy these cases.
    for (const auto& size : {std::pair<int,int>{16,16},{31,33},{256,144}}) {
        const uint32_t cols=uint32_t(size.first+15)/16u, rows=uint32_t(size.second+15)/16u;
        const uint32_t blocks=cols*rows;
        std::vector<uint8_t> input(4u+blocks*24u);
        Word(input,0,0x3A000000u+blocks*6u);
        size_t at=4;
        for(uint32_t mb=0;mb<blocks;++mb) for(uint32_t channel=0;channel<6u;++channel) {
            const int dc=int((mb*37u+channel*83u)%601u)-300;
            const uint16_t value=uint16_t(0x400u|(uint32_t(dc)&1023u));
            input[at++]=uint8_t(value);input[at++]=uint8_t(value>>8u);
            input[at++]=0u;input[at++]=0xFEu;
        }
        const auto copy=input;
        const auto frame=PrNativeMovieFrame::Decode15bppRle(input.data(),input.size(),uint16_t(size.first),uint16_t(size.second));
        std::vector<uint8_t> reference(frame.strips15.size());
        const auto d=PrSS0MdecOutputDirect::ExecuteMdec15bppCurrentScus(input.data(),input.size(),
            uint16_t(size.first),uint16_t(size.second),reference.data(),reference.size());
        Check(d.known&&d.executed&&reference==frame.strips15,"Shared kernel data changed in wrapper");
        Check(input==copy,"Synthetic source was modified");
        const auto expected=PrNativeMovieFrame::Project15bpp(reference.data(),reference.size(),
            uint16_t(size.first),uint16_t(size.second));
        Check(frame.rgba==expected,"Actual decoded color frame was replaced");
        Check(std::any_of(frame.rgba.begin()+1,frame.rgba.end(),[&](uint32_t p){return p!=frame.rgba[0];}),
            "Nonuniform source decoded to a placeholder image");
        for (uint32_t p:frame.rgba) Check((p>>24u)==255u,"Movie pixels lost their opaque alpha");
    }
}
void ProjectionTests() {
    // Independent x/y color pattern catches row-major-vs-strip confusion and
    // both partial edge strips. This does not test the codec against itself.
    for (const auto& size : {std::pair<int,int>{16,16},{32,48},{31,33},{256,144},{320,240}}) {
        const auto w=size.first,h=size.second;
        std::vector<uint8_t> pixels(size_t((w+15)/16)*((h+15)/16)*512u,0xCDu);
        for (int strip=0;strip<(w+15)/16;++strip) for(int y=0;y<h;++y) for(int u=0;u<16;++u) {
            const int x=strip*16+u;
            const uint16_t p=uint16_t((x&31)|((y&31)<<5)|(((x+y)&31)<<10)|0x8000);
            const size_t a=size_t(strip)*((h+15)/16)*512u+size_t(y*16+u)*2u;
            pixels[a]=uint8_t(p);pixels[a+1u]=uint8_t(p>>8u);
        }
        auto rgba=PrNativeMovieFrame::Project15bpp(pixels.data(),pixels.size(),uint16_t(w),uint16_t(h));
        for(int y=0;y<h;++y) for(int x=0;x<w;++x) {
            const auto expand=[](int v){v&=31;return uint32_t((v<<3)|(v>>2));};
            const uint32_t expected=expand(x)|(expand(y)<<8)|(expand(x+y)<<16)|0xFF000000u;
            Check(rgba[size_t(y)*w+x]==expected,"Shared projection changed pixel/strip order");
        }
        Reject([&]{PrNativeMovieFrame::Project15bpp(pixels.data(),pixels.size()-1u,uint16_t(w),uint16_t(h));});
    }
}
}
int main(int argc,char**argv) {
    try {
        Check(argc==2,"Arguments: actual original Stage2 RLE allocation");
        std::ifstream in(argv[1],std::ios::binary);Check(bool(in),"Original frame data missing");
        std::vector<uint8_t> rle((std::istreambuf_iterator<char>(in)),{});
        Check(rle.size()==73728u,"Unexpected captured source allocation");
        const auto original=rle;
        Check(Read(rle)==0x38000360u,"Captured source command changed");
        // Exact two source stores in 80047558(mode=2), applied only to this
        // private test copy. No live scene memory or completion is modified.
        Word(rle,0,Read(rle)|0x08000000u);
        Word(rle,0,Read(rle)|0x02000000u);
        const auto formatted=rle;
        const auto frame=PrNativeMovieFrame::Decode15bppRle(rle.data(),rle.size(),256,144);
        Check(rle==formatted,"Pure codec changed its input");
        Check(frame.decode.macroblocksDecoded==144u,"Incomplete macroblock decode");
        Check(frame.strips15.size()==73728u && frame.rgba.size()==36864u,"Frame output size mismatch");
        std::vector<uint8_t> direct(frame.strips15.size());
        const auto reference=PrSS0MdecOutputDirect::ExecuteMdec15bppCurrentScus(
            formatted.data(),formatted.size(),256,144,direct.data(),direct.size());
        Check(reference.known&&reference.executed&&direct==frame.strips15,"Wrapper did not use the existing codec exactly");
        // Re-entrancy/ownership: decoding a second frame cannot mutate a
        // previously returned frame, and no global scene/audio state is used.
        auto other=PrNativeMovieFrame::Decode15bppRle(rle.data(),rle.size(),256,144);
        Check(other.rgba==frame.rgba,"Repeat decode changed deterministic output");
        other.rgba[0]^=1u;Check(other.rgba[0]!=frame.rgba[0],"Frames share mutable output storage");
        ProjectionTests();
        SyntheticDecodeTests();
        Reject([&]{PrNativeMovieFrame::Decode15bppRle(nullptr,0,256,144);});
        Reject([&]{PrNativeMovieFrame::Decode15bppRle(rle.data(),3,256,144);});
        Reject([&]{PrNativeMovieFrame::Decode15bppRle(rle.data(),rle.size(),0,144);});
        Reject([&]{PrNativeMovieFrame::Decode15bppRle(rle.data(),rle.size(),1025,144);});
        Reject([&]{PrNativeMovieFrame::Decode15bppRle(rle.data(),rle.size(),256,513);});
        Reject([&]{PrNativeMovieFrame::Decode15bppRle(rle.data(),3459,256,144);});
        Reject([&]{PrNativeMovieFrame::Decode15bppRle(original.data(),original.size(),256,144);});
        auto broken=rle;Word(broken,0,0x3A00FFFFu);
        Reject([&]{PrNativeMovieFrame::Decode15bppRle(broken.data(),broken.size(),256,144);});
        broken=rle;Word(broken,0,0x3A000360u);std::fill(broken.begin()+4,broken.end(),0);
        Reject([&]{PrNativeMovieFrame::Decode15bppRle(broken.data(),broken.size(),256,144);});
        Save("stage2_frame1_strips15.bin",frame.strips15.data(),frame.strips15.size());
        Save("stage2_frame1_rgba.bin",reinterpret_cast<const uint8_t*>(frame.rgba.data()),frame.rgba.size()*4u);
        Save("stage2_frame1_formatted_rle.bin",rle.data(),rle.size());
        std::cout<<"{\"passed\":true,\"assertions\":"<<checks<<",\"negative_cases\":"<<negatives
                 <<",\"width\":256,\"height\":144,\"macroblocks\":"<<frame.decode.macroblocksDecoded
                 <<",\"rle_halfwords_consumed\":"<<frame.decode.inputHalfwordsConsumed
                 <<",\"pixels\":"<<frame.rgba.size()<<",\"hardware_bit_exact_claim\":false"
                 <<",\"new_device_simulation\":false,\"integrated_into_live_stage2\":false}\n";
        return 0;
    } catch(const std::exception& e) { std::cerr<<e.what()<<'\n';return 1; }
}
