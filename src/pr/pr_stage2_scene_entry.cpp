#include "pr_stage2_scene_entry.h"
#include "pr_stage2_scene_setup_direct.h"
#include "pr_stage2_save_ui_direct.h"
#include "pr_stage2_save_ui_render.h"
#include "pr_stage2_rating_direct.h"
#include "pr_stage2_soft_float_direct.h"
#include "pr_stage2_resource_setup_direct.h"
#include "pr_stage2_scene_loading_direct.h"
#include "pr_stage2_int_loader_direct.h"
#include <fstream>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace PrStage2SceneEntry {
namespace {
namespace S2 = PrStage2LifecycleDirect;
int32_t Signed(uint32_t value) {
    return value <= 0x7FFFFFFFu ? static_cast<int32_t>(value)
        : static_cast<int32_t>(static_cast<int64_t>(value) - 0x100000000LL);
}
template<class T> T Word(uint32_t value) {
    static_assert(std::is_same_v<T,uint32_t> || std::is_same_v<T,int32_t>, "Typed ABI required");
    if constexpr (std::is_same_v<T,int32_t>) return Signed(value);
    else return value;
}
template<class R,class... A,size_t... I>
R InvokeImpl(Memory& s,Arguments args,R(*function)(Memory&,A...),std::index_sequence<I...>) {
    return function(s,Word<A>(args.begin()[I])...);
}
template<class R,class... A>
R Invoke(Memory& s,Arguments args,R(*function)(Memory&,A...)) {
    if(args.size()!=sizeof...(A)) throw std::invalid_argument("S2 native argument count mismatch");
    return InvokeImpl(s,args,function,std::index_sequence_for<A...>{});
}
bool IsOverlayAddress(uint32_t function) {
    return function >= OverlayBase && function < OverlayBase+OverlayBytes;
}
std::vector<uint8_t> ReadOverlay(const std::filesystem::path& path) {
    std::ifstream stream(path,std::ios::binary|std::ios::ate);
    if(!stream || stream.tellg()!=std::streamoff(OverlayBytes))
        throw std::runtime_error("S2 COMOD2 image missing or wrong length");
    std::vector<uint8_t> bytes(OverlayBytes);
    stream.seekg(0);stream.read(reinterpret_cast<char*>(bytes.data()),bytes.size());
    if(!stream || stream.peek()!=std::ifstream::traits_type::eof())
        throw std::runtime_error("S2 COMOD2 image read failed or changed size");
    // Source-version fingerprint, not a cryptographic authenticity guarantee.
    // The external verifier also binds the complete original SHA-256.
    uint64_t fingerprint=14695981039346656037ull;
    for(uint8_t byte:bytes) fingerprint=(fingerprint^byte)*1099511628211ull;
    if(fingerprint!=0x8DFFB0F59672A2B8ull)
        throw std::runtime_error("S2 COMOD2 image does not match this native translation");
    return bytes;
}
}

bool TryCall(Memory& s,uint32_t function,Arguments args,int32_t& result) {
    switch(function) {
    case 0x801C97ECu: result=Invoke(s,args,S2::InitGlobals801C97EC); break;
    case 0x801C657Cu: result=Invoke(s,args,S2::InitScene801C657C); break;
    case 0x801C74E4u: result=Invoke(s,args,S2::RunScene801C74E4); break;
    case 0x801C78D4u: result=Invoke(s,args,S2::ClearEntries801C78D4); break;
    case 0x801CB244u: result=Invoke(s,args,S2::ConfigureText801CB244); break;
    case 0x801C6A3Cu: result=Invoke(s,args,S2::InitMovie801C6A3C); break;
    case 0x801C66C8u: result=Invoke(s,args,S2::UpdateMovieClock801C66C8); break;
    case 0x801C6804u: result=Invoke(s,args,S2::MovieFrame801C6804); break;
    case 0x801C9A00u: result=Invoke(s,args,S2::AllocatePackets801C9A00); break;
    case 0x801C9A64u: result=Invoke(s,args,S2::ConfigureModel801C9A64); break;
    case 0x801CB284u: result=Invoke(s,args,S2::InitResources801CB284); break;
    case 0x801CB170u: result=Invoke(s,args,S2::Present801CB170); break;
    case 0x801C9ABCu: result=Invoke(s,args,S2::DrawModelsAfterFirst801C9ABC); break;
    case 0x801C6858u: result=Invoke(s,args,S2::GameClock801C6858); break;
    case 0x801C7884u: result=Invoke(s,args,S2::ResetEventTimeline801C7884); break;
    case 0x801C78FCu: result=Invoke(s,args,S2::PublishEventText801C78FC); break;
    case 0x801C7A24u: result=Invoke(s,args,S2::AdvanceResourceQueue801C7A24); break;
    case 0x801C7B20u: result=Invoke(s,args,S2::RestoreSecondResourcePair801C7B20); break;
    case 0x801C7BC0u: result=Invoke(s,args,S2::RestoreFirstResourcePair801C7BC0); break;
    case 0x801C7C54u: result=Invoke(s,args,S2::DispatchEventResources801C7C54); break;
    case 0x801C870Cu: result=Invoke(s,args,S2::UpdateEvents801C870C); break;
    case 0x801C85CCu: result=Invoke(s,args,S2::ApplyInputRow801C85CC); break;
    case 0x801C9644u: result=Invoke(s,args,S2::InputFeedback801C9644); break;
    case 0x801C9730u: result=Invoke(s,args,S2::SpecialPose801C9730); break;
    case 0x801C7958u: result=Invoke(s,args,S2::InitDemo801C7958); break;
    case 0x801C6CDCu: result=Invoke(s,args,S2::InitGame801C6CDC); break;
    case 0x801C6D58u: result=Invoke(s,args,S2::RunGame801C6D58); break;
    case 0x801C6AB8u: result=Invoke(s,args,S2::RunMovie801C6AB8); break;
    case 0x801C9B5Cu: case 0x801C9E18u: case 0x801CA57Cu:
        throw std::invalid_argument("S2 void entry requires CallVoid");
    default: return false;
    }
    return true;
}
bool TryCallVoid(Memory& s,uint32_t function,Arguments args) {
    switch(function) {
    case 0x801C9B5Cu: Invoke(s,args,S2::DrawPreparedScene801C9B5C); return true;
    case 0x801C9E18u: Invoke(s,args,S2::AnimateAndDrawScene801C9E18); return true;
    case 0x801CA57Cu: Invoke(s,args,S2::PrepareFrame801CA57C); return true;
    default: int32_t ignored;return TryCall(s,function,args,ignored);
    }
}

Services::Services(const std::filesystem::path& scus,const std::filesystem::path& overlay)
    :PrStage2DataServices::Services(scus) {
    const auto bytes=ReadOverlay(overlay); // Validate before publishing S2 bytes.
    if(Read16(SceneRecord+4u)!=2u || Read32(0x80048D40u)!=0x801C97ECu ||
       Read32(0x80048D44u)!=0x801C657Cu || Read32(0x80048D48u)!=0x801C74E4u)
        throw std::runtime_error("SCUS scene record/callback table does not identify S2");
    for(uint32_t i=0;i<OverlayBytes;i+=4u) {
        const uint32_t word=uint32_t(bytes[i])|(uint32_t(bytes[i+1u])<<8u)|
            (uint32_t(bytes[i+2u])<<16u)|(uint32_t(bytes[i+3u])<<24u);
        Write32(OverlayBase+i,word);
    }
    // Memory beyond the image remains unknown. Shared options, saves and
    // neighbouring work are never fabricated from gameplay-test fixtures.
}
void Services::Require(Phase expected) const {
    if(phase_!=expected) throw std::logic_error("S2 entry called out of order; create a fresh session after failure");
}
int32_t Services::InitializeGlobals() {
    Require(Phase::Loaded);
    phase_=Phase::GlobalsInitializing;
    try {
        const int32_t result=Call(Read32(0x80048D40u),{});
        phase_=Phase::GlobalsReturned;return result;
    }catch(...){phase_=Phase::Failed;throw;}
}
int32_t Services::InitializeScene() {
    Require(Phase::GlobalsReturned);
    // This source call may remain suspended across several host frames.
    // Publish ownership before entering it, so another caller cannot restart
    // the initializer and invalidate its live heap/private file descriptor.
    phase_=Phase::SceneInitializing;
    try {
        const int32_t result=Call(Read32(0x80048D44u),{SceneRecord,2u});
        phase_=Phase::InitializerReturned;return result;
    }catch(...){phase_=Phase::Failed;throw;}
}
int32_t Services::RunScene() {
    Require(Phase::InitializerReturned);phase_=Phase::Running;
    try {
        const int32_t result=Call(Read32(0x80048D48u),{2u});
        phase_=Phase::Returned;return result;
    }catch(...){phase_=Phase::Failed;throw;}
}
int32_t Services::CallExternal(uint32_t function,Arguments args) {
    int32_t result;
    // 原 80094438 明确安装 801C9728 作为 S2 nullsub 回调。评级尾路径
    // 以 work 参数调用它，其他入口则无参数；两种 ABI 都返回零且不触碰 RAM。
    if(function==0x801C9728u&&(args.size()==0u||args.size()==1u)&&SupportsNullsub801C9728())return 0;
    if(PrStage2SceneLoadingDirect::TryCall(*this,function,args,result)) return result;
    if(function==0x8001AC18u)
        return Invoke(*this,args,PrStage2IntLoaderDirect::LoadInt8001AC18);
    if(PrStage2SceneSetupDirect::TryCall(*this,function,args,result)) return result;
    if(PrStage2SaveUiDirect::TryCall(*this,function,args,result)) return result;
    if(PrStage2SaveUiRender::TryCall(*this,function,args,result)) return result;
    if(PrStage2RatingDirect::TryCall(*this,function,args,result)) return result;
    if(PrStage2SoftFloatDirect::TryCall(*this,function,args,result)) return result;
    if(PrStage2ResourceSetupDirect::TryCall(*this,function,args,result)) return result;
    if(PrStage2SceneEntry::TryCall(*this,function,args,result)) return result;
    if(IsOverlayAddress(function))
        throw std::runtime_error("S2 native function is not translated/bound: "+std::to_string(function));
    return CallPlatform(function,args);
}
void Services::CallExternalVoid(uint32_t function,Arguments args) {
    if(function==0x801C9728u&&(args.size()==0u||args.size()==1u)&&SupportsNullsub801C9728())return;
    if(function==0x80025C44u) {
        if(args.size()!=2u) throw std::invalid_argument("ZeroBytes argument count mismatch");
        PrStage2SceneSetupDirect::ZeroBytes80025C44(*this,args.begin()[0],Signed(args.begin()[1]));
        return;
    }
    int32_t ignored;
    if(PrStage2SceneLoadingDirect::TryCall(*this,function,args,ignored)) return;
    if(function==0x8001AC18u) {
        (void)Invoke(*this,args,PrStage2IntLoaderDirect::LoadInt8001AC18);return;
    }
    if(PrStage2SceneSetupDirect::TryCallVoid(*this,function,args)) return;
    if(PrStage2SaveUiDirect::TryCall(*this,function,args,ignored)) return;
    if(PrStage2SaveUiRender::TryCall(*this,function,args,ignored)) return;
    if(PrStage2RatingDirect::TryCallVoid(*this,function,args)) return;
    if(PrStage2SoftFloatDirect::TryCall(*this,function,args,ignored)) return;
    if(PrStage2ResourceSetupDirect::TryCallVoid(*this,function,args)) return;
    if(PrStage2SceneEntry::TryCallVoid(*this,function,args)) return;
    if(IsOverlayAddress(function))
        throw std::runtime_error("S2 native void function is not translated/bound: "+std::to_string(function));
    CallPlatformVoid(function,args);
}
}
