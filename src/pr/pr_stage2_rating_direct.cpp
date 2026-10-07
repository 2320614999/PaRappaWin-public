#include "pr_stage2_rating_direct.h"
#include "pr_stage2_scene_setup_direct.h"
#include <array>

namespace PrStage2RatingDirect {
namespace {
namespace Setup=PrStage2SceneSetupDirect;
int32_t S(uint32_t v){return v<=0x7FFFFFFFu?int32_t(v):int32_t(int64_t(v)-0x100000000LL);}
int32_t H(uint16_t v){return v<0x8000u?int32_t(v):int32_t(v)-65536;}
constexpr uint32_t Bucket=0x8008ECF4u,EventState=0x8008ED00u,EventFlags=0x8008ED08u;
}
int32_t ResetJudge80014400(Services& s){
    // 保留原清零顺序，随后用当前总分重新建立三个比较基准。
    for(uint32_t p:{0x80091820u,0x8009181Eu,0x80091814u,0x80091812u,0x80091810u,0x80091826u,0x80091828u})s.Write16(p,0u);
    return Setup::ResetRating8001448C(s);
}
int32_t CountEmpty80014458(Services& s){
    // 返回自增前 LHU 加一的完整字；写回半字可能已回绕。
    const int32_t hits=H(s.Read16(0x80091810u));if(hits)return hits;
    const uint32_t next=uint32_t(s.Read16(0x80091820u))+1u;
    s.Write16(0x80091820u,uint16_t(next));return S(next);
}
int32_t ConsumeRecovery800144B8(Services& s,uint32_t work){
    // BAD/Cool 恢复条件读完后，两个一次性标志无条件清零。
    const int32_t rating=H(s.Read16(work+78u));int32_t result=0;
    if(rating==3&&H(s.Read16(0x80091820u))>0&&s.Read16(work+106u)!=0u)result=1;
    else if(rating==0||rating==3){
        if(H(s.Read16(0x8009181Eu))==1)result=s.Read16(work+106u)!=0u?1:0;
    }
    s.Write16(0x8009181Eu,0u);s.Write16(0x80091820u,0u);return result;
}
int32_t FailedCool80014538(Services& s){
    // 原 LH 的符号扩展必须保留。
    return H(s.Read16(0x80091826u));
}
int32_t CompareRating80014548(Services& s,uint32_t work){
    // 描述符为空时不触碰评分基准；后续阈值读取重新取描述符。
    if(!s.Read32(work+64u))return 2;
    const uint16_t score=s.Read16(0x80091816u);
    const int32_t previous=H(s.Read16(0x8009181Cu));
    s.Write16(0x8009181Au,score);s.Write16(0x8009181Cu,score);
    const int32_t delta=H(score)-previous,rating=H(s.Read16(work+78u));
    if(rating==1){if(H(s.Read16(s.Read32(work+64u)+4u))<delta)return 1;}
    else if(rating==2){if(delta>0)return 1;}
    else{
        if(delta>0)return 1;
        if(delta>=0)return 2;
        s.Write16(0x8009181Eu,1u);return 0;
    }
    if(delta<0)return 0;
    return H(s.Read16(0x8009181Eu))==1?0:2;
}
int32_t PatternBonus80014A80(Services& s,uint32_t history,uint32_t first,uint32_t end){
    // 原评分按相邻两个记录分组，末组仍会读取 next+6，不截断奇数范围。
    if(!s.Read16(0x8009182Au))return 0;
    uint32_t empty=0,secondOnly=0,firstOnly=0,both=0,mask=0;
    uint32_t address=history+12u*first;
    for(uint32_t index=first;S(index)<S(end);index+=2u,address+=24u){
        if(s.Read16(address+6u)){
            if(s.Read16(address+18u)){mask|=8u;++both;}
            else{mask|=4u;++firstOnly;}
        }else if(s.Read16(address+18u)){mask|=2u;++secondOnly;}
        else ++empty;
    }
    uint32_t result=0;
    if(mask==14u){result=15u*secondOnly+6u*firstOnly+9u*both;if(!first&&S(empty)>0)result+=18u;}
    else if(mask==12u)result=9u*both+6u*firstOnly;
    else if(mask==10u)result=9u*both+15u*secondOnly;
    else if(mask==6u)result=15u*secondOnly+6u*firstOnly;
    return S(result);
}
void ResetHistory80014BDC(Services& s,uint32_t bar){
    // 原私有调用没有定义标量返回；四段循环历史每段恰为 384 字节。
    const uint32_t destination=0x80092910u+384u*(bar&3u);
    s.Write32(0x8006EA5Cu,destination);Setup::ZeroBytes80025C44(s,destination,384);
}
int32_t CoolDelta80014C80(Services& s){
    // 比较前先保存新分数；失败次数 SH 后按有符号半字判断。
    const int32_t score=H(s.Read16(0x80091816u)),previous=H(s.Read16(0x80091818u));
    s.Write16(0x80091818u,uint16_t(score));
    if(score-previous<66){
        const uint16_t count=uint16_t(s.Read16(0x80091828u)+1u);s.Write16(0x80091828u,count);
        if(H(count)<2)return 1;
    }else if(H(s.Read16(0x80091828u))<2){
        s.Write16(0x80091826u,0u);s.Write16(0x80091828u,0u);return 0;
    }
    s.Write16(0x80091826u,1u);return 1;
}
int32_t PositiveDelta80014D28(Services& s){
    // 两次 LH 的顺序与写回别名行为保持原样。
    const int32_t score=H(s.Read16(0x80091816u)),previous=H(s.Read16(0x80091818u));
    s.Write16(0x80091818u,uint16_t(score));return score-previous>0?1:0;
}
int32_t ClearPatternHits800152D0(Services& s){
    // 原内层每次都重读表基址，外层在九格写回后重读长度。
    const uint32_t count=s.Read32(0x800943DCu);if(S(count)<=0)return S(count);
    uint32_t row=0,offset=0;
    do{
        for(uint32_t i=0;i<9u;++i){
            const uint32_t target=s.Read32(s.Read32(0x800943D8u)+offset+4u*i);
            if(target)s.Write16(target+6u,0u);
        }
        const int32_t limit=S(s.Read32(0x800943DCu));++row;offset+=36u;
        if(S(row)>=limit)return 0;
    }while(true);
}
int32_t AdvanceFeedbackDelay80015350(Services& s,uint32_t /*unusedWork*/,uint32_t elapsed){
    // 原 A0 被计数器地址覆盖；仅正倒计时扣减，返回未截成半字的 SUBU 结果。
    const int32_t previous=H(s.Read16(0x80091824u));if(previous<=0)return previous;
    const uint32_t next=uint32_t(previous)-elapsed;s.Write16(0x80091824u,uint16_t(next));return S(next);
}
int32_t Score80014D58(Services& s,uint32_t work){
    // 两个历史指针属于原函数私有栈，只在 1/2 段分支产生，不能伪造 RAM 地址。
    const auto histories=[](int32_t count,uint32_t bar){
        if(count!=1&&count!=2)throw std::logic_error("S2 scorer history pointer was not initialized by the source");
        return std::array<uint32_t,2>{{0x80092910u+384u*((bar-uint32_t(count))&3u),
                                      0x80092910u+384u*((bar-1u)&3u)}};
    };
    uint32_t delta=0, rhyme=0, drop=0;
    if(H(s.Read16(0x80091810u))==0)delta=0xFFFFFFFFu;
    else{
        const int32_t bank=H(s.Read16(work+80u));
        if(bank==0){
            const uint32_t descriptor=s.Read32(work+64u);
            const int32_t count=H(s.Read16(descriptor+14u));
            const uint32_t selected=s.Read8(descriptor+1u);
            if(count>0){
                const uint32_t maximum=12u*uint32_t(count);
                const bool within=S(maximum)>=H(s.Read16(0x80091810u));
                const uint32_t bar=s.Read32(work+56u);
                const auto history=histories(count,bar);
                const uint32_t row=s.Read32(work+64u);
                const int32_t recorded=H(s.Read16(history[0]+12u*selected+4u));
                const uint32_t required=s.Read8(row+2u),requiredMask=s.Read32(row+8u);
                const int32_t alternate=H(s.Read16(0x800916E2u));
                uint32_t all=0,hits=0,bonus=0;
                for(int32_t i=0;i<count;++i){
                    int32_t length=S(s.Read32(0x80048CA8u+16u*uint32_t(count)+4u*uint32_t(i)));
                    if(alternate==1&&length==15)length=14;
                    for(int32_t j=0;j<length;++j){
                        const uint32_t p=history[size_t(i)]+12u*uint32_t(j),input=s.Read32(p);
                        const uint16_t hit=s.Read16(p+6u);all|=input;
                        if(hit&&(!requiredMask||(requiredMask&input)))++hits;
                    }
                }
                if(requiredMask&&(all&requiredMask)!=requiredMask)hits=0;
                uint32_t score=3u*hits;
                if(recorded==int32_t(required)&&within){
                    for(int32_t i=0;i<count;++i)if(S(hits)>0){
                        const int32_t alternateNow=H(s.Read16(0x800916E2u));
                        uint32_t length=s.Read32(0x80048CA8u+16u*uint32_t(count)+4u*uint32_t(i));
                        if(alternateNow==1&&length==15u)length=14u;
                        bonus+=uint32_t(PatternBonus80014A80(s,history[size_t(i)],0u,length));
                    }
                }else score-=s.Read8(s.Read32(work+64u)+3u);
                const uint32_t penalty=s.Read8(s.Read32(work+64u)+3u);
                uint32_t loss=0u-penalty*uint32_t(H(s.Read16(0x80091814u)));
                const uint32_t floor=~(3u*penalty);if(S(loss)<S(floor))loss=floor;
                const int32_t currentHits=H(s.Read16(0x80091810u));
                delta=S(maximum)<currentHits?0u:score+bonus+loss;
                if(S(maximum)>=currentHits){ rhyme=bonus; drop=loss; }
            }
        }else{
            const uint32_t descriptor=s.Read32(work+64u);
            const int32_t inputHits=H(s.Read16(0x80091810u));
            const int32_t count=H(s.Read16(descriptor+6u*uint32_t(bank)+14u));
            const uint32_t bar=s.Read32(work+56u),maximum=12u*uint32_t(count);
            const bool within=S(maximum)>=inputHits;
            uint32_t hits=0,bonus=0;
            if(count>0){
                const auto history=histories(count,bar);
                const int32_t alternate=H(s.Read16(0x800916E2u));
                for(int32_t i=0;i<count;++i){
                    int32_t length=S(s.Read32(0x80048CA8u+16u*uint32_t(count)+4u*uint32_t(i)));
                    if(alternate==1&&length==15)length=14;
                    for(int32_t j=0;j<length;++j)if(s.Read16(history[size_t(i)]+12u*uint32_t(j)+6u))++hits;
                }
                if(within)for(int32_t i=0;i<count;++i){
                    const int32_t alternateNow=H(s.Read16(0x800916E2u));
                    uint32_t length=s.Read32(0x80048CA8u+16u*uint32_t(count)+4u*uint32_t(i));
                    if(alternateNow==1&&length==15u)length=14u;
                    bonus+=uint32_t(PatternBonus80014A80(s,history[size_t(i)],0u,length));
                }
            }
            if(!within)(void)s.Read16(0x80091810u);
            drop=0u-2u*uint32_t(H(s.Read16(0x80091814u))); rhyme=bonus;
            delta=3u*hits+drop+bonus;
        }
    }
    const int32_t extra=H(s.Read16(0x80091822u));
    const uint32_t score=s.Read16(0x80091816u),next=score+delta+uint32_t(extra);
    s.Write16(0x80091816u,uint16_t(next));
    const uint32_t result=next<<16u;if(S(result)<0)s.Write16(0x80091816u,0u);
    s.ObserveScore(work,S(delta-rhyme-drop),S(rhyme),S(drop),extra,S(delta+uint32_t(extra)));
    return S(result);
}
int32_t UpdateRating80024FD0(Services& s,uint32_t work){
    // 原 32 拍桶派发器：所有回调后的状态和表地址按原指令重新读取。
    if(s.Read32(0x8008ED20u)==1u)return S(work);
    const uint32_t tick=s.Read32(work+12u);const int32_t quarter=S(tick)/384,remainder=S(tick)%384;
    const int32_t mode=H(s.Read16(0x800916D0u));
    const uint32_t bar=uint32_t(quarter)+1u;
    uint32_t nextBar=bar;
    if(remainder>=(mode==1||mode==2?372:368))nextBar=uint32_t(S(tick+(mode==1||mode==2?12u:16u))/384)+1u;
    s.Write32(0x8008ECE8u,uint32_t(remainder));
    const uint32_t previous=s.Read32(0x8008ECF0u),bucket=uint32_t(remainder/12);
    s.Write32(Bucket,bucket);if(previous==bucket)return S(work);
    const int32_t count=S(s.Read32(0x800943C8u));s.Write32(0x8008ECF0u,bucket);
    if(S(nextBar)<count)s.Write32(work+68u,s.Read32(0x800943C4u)+24u*nextBar);
    s.Write32(work+56u,bar);
    uint32_t cue=0;
    if(!s.Read32(Bucket)){
        const uint32_t flags=s.Read32(work),current=s.Read32(work+56u);s.Write32(work,flags|2u);
        if(S(current)<S(s.Read32(0x800943C8u))){
            const uint32_t row=s.Read32(0x800943C4u)+24u*current;s.Write32(work+64u,row);
            cue=s.Read8(row);
            const uint32_t bits=row?s.Read16(row+6u*uint32_t(H(s.Read16(work+80u)))+16u):0u;
            s.Write32(EventFlags,bits);
        }
    }
    if((s.Read32(Bucket)&1u)==0u){
        const uint32_t flags=s.Read32(work);s.Write32(work,flags|8u);
        if((s.Read32(Bucket)&7u)==0u)s.Write32(work,flags|12u);
    }
    uint32_t table=s.Read32(0x800943C0u),now=s.Read32(Bucket);
    if(now==s.Read32(table)){
        if(PrStage2LifecycleDirect::IsJudgeBlocked80024BF4(s,work)!=1){
            bool recovery=false;
            if((s.Read32(EventFlags)&16u)!=0u)recovery=uint32_t(s.Read16(work+78u))-2u<2u;
            if(!recovery&&(s.Read32(EventFlags)&8u)!=0u)recovery=ConsumeRecovery800144B8(s,work)==1;
            if(recovery){const uint32_t flags=s.Read32(work);s.Write16(work+118u,1u);s.Write16(work+84u,1u);s.Write32(work,flags|64u);}
        }
        if(s.Read32(EventFlags)&128u)ClearPatternHits800152D0(s);
        if(H(s.Read16(work+94u))==S(cue)||!cue)s.Write16(work+92u,0u);
        else{const int32_t rating=H(s.Read16(work+78u));s.Write16(work+94u,uint16_t(cue));if(rating)s.Write16(work+92u,1u);}
    }
    table=s.Read32(0x800943C0u);now=s.Read32(Bucket);
    if(now==s.Read32(table+4u)){
        if(!PrStage2LifecycleDirect::IsJudgeBlocked80024BF4(s,work)){
            if(s.Read32(EventFlags)&3u){Score80014D58(s,work);CountEmpty80014458(s);}
            if(s.Read32(EventFlags)&2u){
                uint32_t delta=0;
                if(!s.Read16(work+78u)){
                    const int32_t result=CoolDelta80014C80(s);
                    if(result==0){delta=1u;s.Write16(work+90u,0u);}
                    else if(result==1||result==2){s.Write16(work+88u,result==1?1u:3u);s.Write16(work+90u,1u);}
                }else{
                    delta=uint32_t(PositiveDelta80014D28(s));
                    if(!s.Read16(0x8008ED38u)){
                        const int32_t rating=H(s.Read16(work+78u));
                        const bool suppress=rating==1&&delta==1u&&!s.Read16(0x8008ED34u);
                        if(!suppress){
                            const uint32_t ratingNow=uint32_t(H(s.Read16(work+78u)));s.Write16(work+90u,1u);
                            s.Write16(work+88u,s.Read16(0x80055420u+12u*ratingNow+2u*delta));
                        }
                        const uint16_t old=s.Read16(0x8008ED38u);s.Write16(0x8008ED36u,uint16_t(delta));s.Write16(0x8008ED38u,uint16_t(old+1u));
                    }else{
                        const int32_t rating=H(s.Read16(work+78u));uint16_t recovered=0;int32_t old;
                        if(rating==3){old=H(s.Read16(0x8008ED36u));if(!old){recovered=delta==0u?1u:0u;old=H(s.Read16(0x8008ED36u));}}
                        else old=H(s.Read16(0x8008ED36u));
                        s.Write16(0x8009181Eu,recovered);if(old!=S(delta))s.Write16(0x8008ED36u,2u);
                        s.Write16(0x8008ED38u,0u);s.Write16(work+90u,0u);
                    }
                }
                const uint32_t state=s.Read32(EventState);bool changed=false;
                if(state==0u&&(s.Read32(EventFlags)&4u)){
                    const int32_t old=H(s.Read16(0x8008ED36u));uint32_t candidate=2u;
                    if(old==1){
                        if(H(s.Read16(work+78u))!=old||CompareRating80014548(s,work)==old)candidate=1u;
                    }else if(old==0){candidate=0u;if(H(s.Read16(work+78u))==3)candidate=CompareRating80014548(s,work)!=0?2u:0u;}
                    Setup::ResetInputRow80024F8C(s,work);
                    if(candidate!=2u){
                        const int32_t rating=H(s.Read16(work+78u));
                        if(!(rating==3&&candidate==0u)&&!(rating==1&&candidate==1u&&!s.Read16(0x8009182Au))){
                            s.Call(0x80026EF8u,{s.Read32(0x800943FCu)+6u*candidate});
                            if(H(s.Read16(work+78u))==1&&candidate==1u){
                                const uint32_t flags=s.Read32(work);const uint16_t transitions=uint16_t(s.Read16(work+114u)+1u);
                                s.Write16(work+114u,transitions);s.Write32(work,flags|0x2000u);
                                const uint32_t phrase=s.Read32(H(transitions)<2?0x800943F4u:0x800943F8u);
                                s.Call(0x80026EF8u,{phrase});const int32_t duration=H(s.Read16(phrase+4u));const uint32_t tickNow=s.Read32(work+12u);
                                s.Write32(0x8008ED14u,1u);s.Write32(0x8008ED0Cu,tickNow+uint32_t(duration));
                                s.Write16(work+80u,1u);s.Write16(work+138u,0u);s.Write32(EventState,7u);s.Write32(0x8008ED24u,1u);
                            }else{s.Write32(EventState,6u);s.Write16(work+80u,0u);}
                            const int32_t ratingNow=H(s.Read16(work+78u));
                            if(ratingNow==1){
                                if(candidate==1u){s.Write16(work+78u,0u);s.Write16(work+398u,5u);s.Write16(work+122u,0u);}
                                else{s.Write16(work+78u,2u);const uint16_t drops=s.Read16(work+116u);s.Write16(work+398u,1u);s.Write16(work+116u,uint16_t(drops+1u));}
                            }else if(ratingNow==2){s.Write16(work+78u,candidate==1u?1u:3u);s.Write16(work+398u,candidate==1u?4u:2u);}
                            else if(ratingNow==3&&candidate==1u){s.Write16(work+78u,2u);s.Write16(work+398u,3u);}
                            if(!s.Read16(work+78u)&&!candidate){s.Write16(work+78u,1u);s.Write16(work+398u,6u);}
                            changed=true;
                        }
                    }
                }
                if(!changed){
                    if(state==0u||state==1u||state==4u){s.Call(0x80026EF8u,{s.Read32(0x80094400u)+6u*delta});Setup::ResetFeedback80014C1C(s);}
                    if(delta==1u)s.Call(s.Read32(0x80094438u),{work});
                }
            }
        }
        if(s.Read32(EventState)){
            const uint32_t state=s.Read32(EventState);
            if(state==1u&&PrStage2LifecycleDirect::IsJudgeBlocked80024BF4(s,work)!=S(state)&&
                (s.Read32(EventFlags)&32u)&&FailedCool80014538(s))s.Write32(EventState,4u);
            uint32_t next=s.Read32(EventState);
            if(next==4u&&(s.Read32(EventFlags)&32u)){
                s.Call(0x80026EF8u,{s.Read32(0x800943F0u)});s.Write32(work,s.Read32(work)|0x4000u);
                s.Write32(0x8008ED14u,1u);s.Write32(EventState,5u);next=s.Read32(EventState);
            }
            if(next==5u&&(s.Read32(EventFlags)&64u)){
                s.Write32(EventState,8u);s.Write32(0x8008ED28u,1u);s.Write32(0x8008ED14u,0u);
                s.Write16(work+78u,1u);s.Write16(work+398u,6u);s.Write16(work+80u,0u);s.Write16(work+84u,1u);s.Write16(work+122u,1u);s.Write16(0x8008ED36u,1u);
            }
        }
        Setup::InitializeWorkRating80024FC0(s,work);
    }
    if(s.Read32(Bucket)==31u)ResetHistory80014BDC(s,s.Read32(work+56u));
    table=s.Read32(0x800943C0u);now=s.Read32(Bucket);
    if(now==s.Read32(table+8u)){
        const uint32_t state=s.Read32(EventState);
        if(S(state)>=6){
            if(state==6u)s.Write32(EventState,0u);
            else if(state==7u)s.Write32(EventState,1u);
            else if(state==8u){const uint32_t phrase=s.Read32(0x800943FCu);s.Write32(EventState,0u);s.Call(0x80026EF8u,{phrase});}
            s.Write32(work,s.Read32(work)|0x200u);ResetJudge80014400(s);Setup::ResetFeedback80014C1C(s);Setup::ResetInputRow80024F8C(s,work);
        }
    }
    return S(work);
}
bool TryCall(Services& s,uint32_t fn,std::initializer_list<uint32_t> args,int32_t& result){
    // 未使用的原 A0 不构成公开参数；直接调用方显式保留有效输入。
    size_t n;
    switch(fn){
    case 0x80014400u:case 0x80014458u:case 0x80014538u:case 0x80014C80u:case 0x80014D28u:case 0x800152D0u:n=0;break;
    case 0x800144B8u:case 0x80014548u:case 0x80014D58u:case 0x80024FD0u:n=1;break;
    case 0x80014A80u:n=3;break;
    case 0x80015350u:n=2;break;
    case 0x80014BDCu:throw std::invalid_argument("S2 history reset requires CallVoid");
    default:return false;
    }
    if(args.size()!=n)throw std::invalid_argument("S2 rating argument count mismatch");const auto a=args.begin();
    switch(fn){
    case 0x80014400u:result=ResetJudge80014400(s);break;
    case 0x80014458u:result=CountEmpty80014458(s);break;
    case 0x800144B8u:result=ConsumeRecovery800144B8(s,a[0]);break;
    case 0x80014538u:result=FailedCool80014538(s);break;
    case 0x80014548u:result=CompareRating80014548(s,a[0]);break;
    case 0x80014A80u:result=PatternBonus80014A80(s,a[0],a[1],a[2]);break;
    case 0x80014C80u:result=CoolDelta80014C80(s);break;
    case 0x80014D28u:result=PositiveDelta80014D28(s);break;
    case 0x80014D58u:result=Score80014D58(s,a[0]);break;
    case 0x800152D0u:result=ClearPatternHits800152D0(s);break;
    case 0x80015350u:result=AdvanceFeedbackDelay80015350(s,a[0],a[1]);break;
    case 0x80024FD0u:result=UpdateRating80024FD0(s,a[0]);break;
    }
    return true;
}
bool TryCallVoid(Services& s,uint32_t fn,std::initializer_list<uint32_t> args){
    // 历史清空无标量结果，不能进入 TryCall 的整数路径。
    if(fn==0x80014BDCu){if(args.size()!=1u)throw std::invalid_argument("S2 history reset argument count mismatch");ResetHistory80014BDC(s,*args.begin());return true;}
    int32_t ignored;return TryCall(s,fn,args,ignored);
}
}
