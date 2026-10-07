#include "pr_stage2_scene_setup_direct.h"

namespace PrStage2SceneSetupDirect {
namespace {
int32_t Signed(uint32_t value) {
    return value<=0x7FFFFFFFu?static_cast<int32_t>(value)
        :static_cast<int32_t>(static_cast<int64_t>(value)-0x100000000LL);
}
}
void ZeroBytes80025C44(Services& s,uint32_t destination,int32_t count) {
    while(count>0) {s.Write8(destination,0u);--count;++destination;}
}
int32_t ResetRating8001448C(Services& s) {
    const uint16_t previous=s.Read16(0x80091816u);
    s.Write16(0x80091818u,previous);s.Write16(0x8009181Au,previous);s.Write16(0x8009181Cu,previous);
    return previous;
}
int32_t ResetFeedback80014C1C(Services& s) {
    const uint32_t previous=s.Read32(0x80091808u);
    s.Write16(0x80091810u,0u);s.Write16(0x80091812u,0u);s.Write16(0x80091814u,0u);
    s.Write16(0x80091822u,0u);s.Write32(0x80091808u,0u);s.Write32(0x8009180Cu,previous);
    return Signed(previous);
}
int32_t ResetInputRow80024F8C(Services& s,uint32_t work) {
    s.Write16(0x8008ED36u,2u);s.Write16(0x8008ED38u,0u);s.Write16(work+90u,0u);
    return ResetRating8001448C(s);
}
// 原 LH 先符号扩展，再在返回延迟槽写入工作区；保留来源与目标重叠。
int32_t InitializeWorkRating80024FC0(Services& s,uint32_t work){
    const uint32_t value=s.Read16(0x80091816u);
    const int32_t result=Signed(value<0x8000u?value:value|0xFFFF0000u);
    s.Write32(work+48u,uint32_t(result));return result;
}
// 原函数的清空分支没有定义 V0；只保留索引和容量副作用。
void ResetReplay80024E54(Services& s,uint32_t reserve){
    s.Write32(0x800901C0u,0u);
    if(!reserve)s.Write32(0x800901BCu,0u);
    else if(!s.Read32(0x800901BCu))s.Write32(0x800901BCu,600u);
}
// 复制保存的时间/按键对；目标覆盖计数字段后，下一次比较必须回读新值。
int32_t LoadReplay8001681C(Services& s){
    s.Write32(0x800901BCu,s.Read32(0x80092F48u));
    for(uint32_t i=0;Signed(i)<Signed(s.Read32(0x800901BCu));++i){
        const uint32_t time=s.Read32(0x80092F5Cu+8u*i),pad=s.Read32(0x80092F60u+8u*i);
        s.Write32(0x8008EEF8u+8u*i,time);s.Write32(0x8008EEFCu+8u*i,pad);
    }
    return 0;
}
// 原场景参数只写入私有栈，不参与保存档位的判断。
int32_t ReplayDifficulty80016758(Services& s,uint32_t /*unusedScene*/){return Signed(s.Read32(0x80092F40u))>=2?1:0;}
// 索引来自原 SCUS 表，地址运算保持 SLL/ADDU 的 32 位回绕。
int32_t SaveStageIndex8001615C(Services& s,uint32_t scene){return Signed(s.Read32(0x80048DB8u+(scene<<2u)));}
// 已保存的评级按原 LBU 读取，不能把 0x80..0xFF 误作负数。
int32_t SavedStageRating800166AC(Services& s,uint32_t scene){return s.Read8(0x80092F1Du+uint32_t(SaveStageIndex8001615C(s,scene)));}
// 普通模式根据对应关卡保存的评级选择评分档位。
int32_t StageDifficulty8001670C(Services& s,uint32_t scene){return SavedStageRating800166AC(s,scene)>=2?1:0;}
int32_t AllStagesClear800161F4(Services& s,uint32_t ratings){
    for(uint32_t i=0;i<6u;++i)if(s.Read8(ratings+i)!=3u)return 0;
    return 1;
}
int32_t UnlockStage8001628C(Services& s,uint32_t scene){
    const uint32_t target=0x80092F1Du+uint32_t(SaveStageIndex8001615C(s,scene));
    const uint8_t rating=s.Read8(target);
    if(rating)return rating;
    s.Write8(target,1u);return 1;
}
int32_t RecordClear8001635C(Services& s,uint32_t scene,uint32_t rating,
                          uint32_t previousRating,uint32_t score){
    const uint32_t index=uint32_t(SaveStageIndex8001615C(s,scene));
    s.Write32(0x80092F40u,previousRating);
    if(!previousRating)s.Write32(0x80092F40u,1u);
    // LBU is compared with the full signed argument; only the store narrows.
    if(int32_t(s.Read8(0x80092F1Du+index))<Signed(rating))
        s.Write8(0x80092F1Du+index,uint8_t(rating));
    s.Write32(0x80092F24u+(index<<2u),score);
    s.Write32(0x80092F3Cu,index);
    s.Write32(0x80092F48u,s.Read32(0x800901BCu));
    // Original copies the complete replay backing, regardless of its count.
    PrStage2LifecycleDirect::CopyBytes80025C64(s,0x8008EEF8u,0x80092F5Cu,4800);
    const int32_t result=AllStagesClear800161F4(s,0x80092F1Du);
    s.Write32(0x80092F44u,uint32_t(result));return result;
}
int32_t BackupSave80015700(Services& s,uint32_t source){
    return PrStage2LifecycleDirect::CopyBytes80025C64(s,source,0x80079008u,4876);
}
int32_t RestoreSave80015744(Services& s,uint32_t destination){
    return PrStage2LifecycleDirect::CopyBytes80025C64(s,0x80079008u,destination,4876);
}
int32_t LoadSave800164B4(Services& s,uint32_t source){
    return PrStage2LifecycleDirect::CopyBytes80025C64(s,source,0x80092F10u,4876);
}
int32_t RestoreReplayScore800169E0(Services& s,uint32_t work){
    const uint32_t mode=s.Read16(0x800916D0u);
    if(mode!=2u)return Signed(mode<0x8000u?mode:mode|0xFFFF0000u);
    const uint32_t scene=s.Read16(0x800916E2u);
    const uint32_t index=uint32_t(SaveStageIndex8001615C(s,scene<0x8000u?scene:scene|0xFFFF0000u));
    s.Write32(work+48u,s.Read32(0x80092F24u+(index<<2u)));
    // Keep the source LHU after SW, including overlapping destinations.
    s.Write16(0x80091816u,s.Read16(work+48u));return Signed(work);
}
// 原 SH 仅保留评分档位的低半字。
void SetScorerDifficulty800143F0(Services& s,uint32_t difficulty){s.Write16(0x8009182Au,uint16_t(difficulty));}
// 事件系统使用独立半字，不能与评分器档位合并为本机状态。
void SetEventDifficulty800259C0(Services& s,uint32_t difficulty){s.Write16(0x8008ED34u,uint16_t(difficulty));}
int32_t ResetEventState80024E98(Services& s) {
    ZeroBytes80025C44(s,0x8008ECE0u,244);
    s.Write32(0x8008ED20u,0u);s.Write32(0x8008ED00u,0u);
    s.Write16(0x8008ED2Eu,1u);s.Write16(0x8008ED2Cu,0u);
    s.Write32(0x8006ED60u,0u); // Original GP 8006EA40 + 320h.
    ZeroBytes80025C44(s,0x801C3640u,488);
    const uint32_t rows=s.Read32(0x800943D0u);
    s.Write16(0x801C368Eu,1u);s.Write16(0x801C3690u,0u);s.Write16(0x801C3692u,0u);
    s.Write16(0x801C36AAu,1u);s.Write16(0x801C36BAu,1u);s.Write16(0x801C36CAu,0u);
    s.Write16(0x801C36D0u,0u);s.Write16(0x801C36E2u,0u);s.Write32(0x801C36D4u,rows+4u);
    const uint16_t subtitles=s.Read16(0x800916DCu);
    s.Write32(0x801C36D8u,rows+24u);s.Write16(0x801C36A8u,subtitles);s.Write16(0x801C3694u,subtitles);
    return ResetInputRow80024F8C(s,0x801C3640u);
}
int32_t ResetScorer80014344(Services& s) {
    s.Write32(0x8006EA5Cu,0x80092910u); // GP + 1Ch; native memory, not a host pointer.
    for(uint32_t i=0;i<4u;++i) ZeroBytes80025C44(s,0x80092910u+384u*i,384);
    ZeroBytes80025C44(s,0x80091800u,44);
    s.Write16(0x8009181Au,0u);s.Write16(0x8009181Cu,0u);
    s.Write16(0x80091816u,0u);s.Write16(0x80091818u,0u);
    ResetFeedback80014C1C(s);
    const int32_t result=ResetRating8001448C(s);
    s.Write32(0x8006EA60u,0u);s.Write32(0x80091800u,0u);
    return result;
}
int32_t ResetTransition8001EF14(Services& s) {
    s.Write32(0x8006EB0Cu,0u);s.Write32(0x8006EB04u,0u);s.Write32(0x8006EB08u,0u);
    return PrStage2LifecycleDirect::FillDrawFlags8001EEAC(s,0u);
}
int32_t DecodeDemoPad80024BC0(Services& s,uint32_t input) {
    if(input-1u>=8u) return 0;
    return Signed(s.Read32(0x80054168u+4u*input));
}
bool TryCall(Services& s,uint32_t function,std::initializer_list<uint32_t> args,int32_t& result) {
    size_t arity;
    switch(function) {
    case 0x8001448Cu:case 0x80014C1Cu:case 0x80024E98u:case 0x80014344u:case 0x8001EF14u:arity=0;break;
    case 0x80024F8Cu:case 0x80024FC0u:case 0x80024BC0u:arity=1;break;
    case 0x8001681Cu:arity=0;break;
    case 0x80016758u:case 0x8001615Cu:case 0x800166ACu:case 0x8001670Cu:arity=1;break;
    case 0x800161F4u:case 0x8001628Cu:case 0x80015700u:case 0x80015744u:
    case 0x800164B4u:case 0x800169E0u:arity=1;break;
    case 0x8001635Cu:arity=4;break;
    case 0x80025C64u:arity=3;break;
    case 0x8001BC48u:case 0x8001BC78u:arity=4;break;
    case 0x80025C44u:throw std::invalid_argument("ZeroBytes has no scalar return");
    case 0x80024E54u:case 0x800143F0u:case 0x800259C0u:throw std::invalid_argument("S2 mode setup requires CallVoid");
    default:return false;
    }
    if(args.size()!=arity) throw std::invalid_argument("S2 scene-setup argument count mismatch");
    switch(function) {
    case 0x8001448Cu:result=ResetRating8001448C(s);break;
    case 0x80014C1Cu:result=ResetFeedback80014C1C(s);break;
    case 0x80024F8Cu:result=ResetInputRow80024F8C(s,*args.begin());break;
    case 0x80024FC0u:result=InitializeWorkRating80024FC0(s,*args.begin());break;
    case 0x8001681Cu:result=LoadReplay8001681C(s);break;
    case 0x80016758u:result=ReplayDifficulty80016758(s,*args.begin());break;
    case 0x8001615Cu:result=SaveStageIndex8001615C(s,*args.begin());break;
    case 0x800166ACu:result=SavedStageRating800166AC(s,*args.begin());break;
    case 0x8001670Cu:result=StageDifficulty8001670C(s,*args.begin());break;
    case 0x800161F4u:result=AllStagesClear800161F4(s,*args.begin());break;
    case 0x8001628Cu:result=UnlockStage8001628C(s,*args.begin());break;
    case 0x8001635Cu:result=RecordClear8001635C(s,args.begin()[0],args.begin()[1],args.begin()[2],args.begin()[3]);break;
    // Save entry copies its 36-byte menu context through the shared call ABI.
    // Keep the original source-first, forward-byte copy and end-pointer return.
    case 0x80025C64u:result=PrStage2LifecycleDirect::CopyBytes80025C64(s,args.begin()[0],args.begin()[1],Signed(args.begin()[2]));break;
    case 0x80015700u:result=BackupSave80015700(s,*args.begin());break;
    case 0x80015744u:result=RestoreSave80015744(s,*args.begin());break;
    case 0x800164B4u:result=LoadSave800164B4(s,*args.begin());break;
    case 0x800169E0u:result=RestoreReplayScore800169E0(s,*args.begin());break;
    case 0x80024E98u:result=ResetEventState80024E98(s);break;
    case 0x80014344u:result=ResetScorer80014344(s);break;
    case 0x8001EF14u:result=ResetTransition8001EF14(s);break;
    case 0x80024BC0u:result=DecodeDemoPad80024BC0(s,*args.begin());break;
    case 0x8001BC48u:result=ConfigureTextRect8001BC48(s,args.begin()[0],args.begin()[1],args.begin()[2],args.begin()[3]);break;
    case 0x8001BC78u:result=ConfigureTextTexture8001BC78(s,args.begin()[0],args.begin()[1],args.begin()[2],args.begin()[3]);break;
    }
    return true;
}
// 无返回值入口单独分派，不为未定义的 V0 生成成功值。
bool TryCallVoid(Services& s,uint32_t function,std::initializer_list<uint32_t> args){
    switch(function){
    case 0x80024E54u:case 0x800143F0u:case 0x800259C0u:
        if(args.size()!=1u)throw std::invalid_argument("S2 mode setup argument count mismatch");
        if(function==0x80024E54u)ResetReplay80024E54(s,*args.begin());
        else if(function==0x800143F0u)SetScorerDifficulty800143F0(s,*args.begin());
        else SetEventDifficulty800259C0(s,*args.begin());
        return true;
    default:int32_t ignored;return TryCall(s,function,args,ignored);
    }
}
int32_t ConfigureTextRect8001BC48(Services& s,uint32_t x,uint32_t y,
                               uint32_t width,uint32_t height) {
    const uint32_t primitive = s.Read32(0x8004E690u);
    s.Write16(0x8006ED38u, static_cast<uint16_t>(x));
    s.Write16(0x8006ED3Cu, static_cast<uint16_t>(y));
    s.Write16(0x8007CEE0u, static_cast<uint16_t>(width));
    s.Write16(0x8007CEE2u, static_cast<uint16_t>(height));
    s.Write32(0x8007CED0u, primitive);
    return Signed(primitive);
}
int32_t ConfigureTextTexture8001BC78(Services& s,uint32_t u,uint32_t v,
                                  uint32_t palette,uint32_t /*unusedA3*/) {
    s.Write32(0x8006ED40u, u);
    s.Write32(0x8006ED44u, v);
    s.Write32(0x8006ED48u, palette);
    s.Write16(0x8007CED8u, 12u);
    s.Write16(0x8007CEDAu, 12u);
    return 12;
}
}
