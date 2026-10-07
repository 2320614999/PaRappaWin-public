#include "pr_stage2_resource_setup_direct.h"
#include "pr_stage2_source_word.h"

namespace PrStage2ResourceSetupDirect {
namespace {
// 避免有符号溢出，保留原指令的 32 位回绕和半字符号扩展。
int32_t S(uint32_t v){return v<0x80000000u?int32_t(v):int32_t(int64_t(v)-0x100000000LL);}
uint32_t H(uint32_t v){v&=0xFFFFu;return v<0x8000u?v:v|0xFFFF0000u;}
// 两个原入口仅参数表基址不同；逐项保留四次字写入、两次半字写入的顺序。
int32_t ResetTrackState(Services& s,uint32_t base){
    for(uint32_t i=0;i<36u;++i){
        const uint32_t row=base+16u*i;
        s.Write32(row,0u);s.Write32(row+4u,2048u);
        s.Write32(row+8u,2048u);s.Write32(row+12u,0u);
        s.Write16(0x80087668u+2u*i,4096u);
        s.Write16(0x800876B0u+2u*i,4096u);
    }
    return 0;
}
}
// 原 80024308 重置第一组参数，返回最后一次循环比较得到的零。
int32_t ResetFirstTrackState80024308(Services& s){return ResetTrackState(s,0x800876F8u);}
// 原 80024390 重置第二组参数，两组入口共用权重数组。
int32_t ResetSecondTrackState80024390(Services& s){return ResetTrackState(s,0x80087938u);}

// 80013994 为指定 VDF 通道建立原生 16 字节 key 表；工作区就是
// 8001385C 为该通道预留的 2 KiB 区域，所有源字段按原顺序重新读取。
int32_t ParseVdfChannel80013994(Services& s,uint32_t channel,uint32_t source){
    if(channel>=10u) return -1;
    const uint32_t offset=12u*channel, state=0x80074B80u+offset;
    const uint32_t workspace=0x8006FB08u+2048u*channel;
    s.Write32(state+4u,workspace);
    s.Write32(state+8u,0x80090240u+512u*channel);
    return BindMorphTable800137BC(s,source,state);
}

// 800140E0 是普通模式的主 MIMe 初始化入口；gp+20/gp+24 使用时钟
// 范围而非宿主帧号，便于后续 800141D8 原样推进 cursor。
int32_t InitializePrimaryMime800140E0(Services& s,uint32_t dat,uint32_t vdf,
                                      uint32_t loop,uint32_t begin,uint32_t length){
    s.Write32(0x8006EA54u,begin);
    s.Write32(0x8006EA58u,begin+length-1u);
    ParseVdfChannel80013994(s,0u,vdf);
    InitTracks80013D10(s,0x80078BF8u,0x80078C08u,dat,loop);
    for(uint32_t i=0;i<128u;++i)s.Write32(0x80090240u+4u*i,0u);
    return 0;
}

// 80014050 的 a6 是真实 RAM 输出槽；先写零，再建立 VDF/DAT，最后清理
// 指定通道的 128 个权重，返回原函数使用的末元素地址。
int32_t InitializeMimeChannel80014050(Services& s,uint32_t channel,uint32_t dat,uint32_t vdf,
                                      uint32_t datState,uint32_t datBuffer,uint32_t cursorOutput){
    s.Write32(cursorOutput,0u);
    ParseVdfChannel80013994(s,channel,vdf);
    InitTracks80013D10(s,datState,datBuffer,dat,0u);
    if(channel>=10u)return -1;
    const uint32_t base=0x80090240u+512u*channel;
    for(uint32_t i=0;i<128u;++i)s.Write32(base+4u*i,0u);
    return int32_t(0x8009043Cu);
}

namespace {
// The original BEZ records hold an endpoint, two polynomial coefficients,
// and the next endpoint. A zero coefficient vector selects integer linear
// interpolation; two zero vectors with equal endpoints select a held view.
void ClassifyCameraTrack(Services& s,uint32_t track,uint32_t mode,
                         uint32_t velocity,uint32_t matrix,uint32_t segment){
    const uint32_t base=track+24u*segment;
    const auto half=[&](uint32_t point,uint32_t axis){return S(H(s.Read16(base+8u*point+2u*axis)));};
    bool equal=true,zeroFirst=true,zeroSecond=true;
    for(uint32_t axis=0;axis<3u;++axis){
        equal=equal&&(half(0u,axis)==half(3u,axis));
        zeroFirst=zeroFirst&&(half(1u,axis)==0);
        zeroSecond=zeroSecond&&(half(2u,axis)==0);
    }
    if(equal&&zeroFirst&&zeroSecond){s.Write32(mode,1u);return;}
    if(zeroFirst||zeroSecond){
        const int32_t period=S(s.Read32(0x8006EA48u));
        if(!period)s.Break(mode==0x8006EA4Cu?0x80012C38u:0x800130B8u,7u);
        uint32_t step[3];
        for(uint32_t axis=0;axis<3u;++axis)
            step[axis]=uint32_t((half(3u,axis)-half(0u,axis))/period);
        s.Write32(mode,2u);
        for(uint32_t axis=0;axis<3u;++axis)s.Write32(velocity+4u*axis,step[axis]);
        return;
    }
    for(uint32_t axis=0;axis<3u;++axis)
        for(uint32_t column=0;column<3u;++column){
            if(axis==2u&&column==2u)s.Write32(mode,0u);
            s.Write16(matrix+6u*axis+2u*column,uint16_t(half(column,axis)));
        }
}
std::array<int32_t,3> ApplyCameraMatrix(Services& s,const std::array<uint32_t,5>& matrix,
                                      uint32_t xy,int32_t z){
    auto& g=s.MatrixGte();
    for(uint32_t i=0;i<4u;++i)g.matrix.words[i]=matrix[i];
    g.matrix.words[4]=H(matrix[4]);g.vectorXY0=xy;g.vectorZ0=z;
    PrPsxGteDirect::ExecuteRotationMvmva(g,false,true);
    return g.mac;
}
void EvaluateCameraTrack(Services& s,uint32_t track,uint32_t mode,uint32_t velocity,
                         uint32_t matrix,uint32_t output,uint32_t segment,uint32_t frame){
    const uint32_t base=track+24u*segment,value=s.Read32(mode);
    if(value==1u||value==2u){
        for(uint32_t axis=0;axis<3u;++axis){
            const uint32_t start=H(s.Read16(base+2u*axis));
            s.Write32(output+4u*axis,start+(value==2u?s.Read32(velocity+4u*axis)*frame:0u));
        }
        return;
    }
    std::array<uint32_t,5> coefficients{};
    for(uint32_t i=0;i<5u;++i)coefficients[i]=s.Read32(matrix+4u*i);
    const uint32_t weights=s.Read32(0x8006ECD0u)+8u*frame;
    const uint32_t xy=s.Read32(weights),zp=s.Read32(weights+4u);
    const auto first=ApplyCameraMatrix(s,coefficients,xy,S(H(zp)));
    // ApplyMatrix's MAC outputs are narrowed by SH into the private second
    // matrix before the endpoint term is added through another GTE command.
    const uint32_t endX=s.Read16(base+24u),endY=s.Read16(base+26u),endZ=s.Read16(base+28u);
    const std::array<uint32_t,5> finalMatrix{{
        (uint32_t(first[0])&0xFFFFu)|(endX<<16u),uint32_t(first[1])<<16u,
        endY,(uint32_t(first[2])&0xFFFFu)|(endZ<<16u),0u}};
    const uint32_t endWeight=s.Read16(s.Read32(0x8006ECD0u)+8u*frame+6u);
    const auto result=ApplyCameraMatrix(s,finalMatrix,4096u|(endWeight<<16u),0);
    for(uint32_t axis=0;axis<3u;++axis)s.Write32(output+4u*axis,uint32_t(result[axis]));
}
}
// Original segment progression and BEZ evaluation, including the terminal
// -1 latch. The incoming frame is the caller's pre-increment frame number.
int32_t UpdateCameraFrame80012960(Services& s,int32_t frame){
    uint32_t segment=s.Read32(0x8006EA44u);
    if(S(segment)<0)return -1;
    const int32_t period=S(s.Read32(0x8006EA48u));
    if(!period)s.Break(0x8001299Cu,7u);
    if(period==-1&&uint32_t(frame)==0x80000000u)s.Break(0x800129B4u,6u);
    const uint32_t within=uint32_t(frame%period);
    if(!within){
        if(segment>=((s.Read32(0x8006EA40u)+2u)/3u)-2u){
            s.Write32(0x8006EA44u,0xFFFFFFFFu);return -1;
        }
        if(frame)s.Write32(0x8006EA44u,++segment);
        ClassifyCameraTrack(s,0x8006EE10u,0x8006EA4Cu,0x8006ECB8u,0x8006EDD0u,segment);
        ClassifyCameraTrack(s,0x8006F180u,0x8006EA50u,0x8006ECC4u,0x8006EDF0u,segment);
    }
    segment=s.Read32(0x8006EA44u);
    EvaluateCameraTrack(s,0x8006EE10u,0x8006EA4Cu,0x8006ECB8u,0x8006EDD0u,0x80095C60u,segment,within);
    EvaluateCameraTrack(s,0x8006F180u,0x8006EA50u,0x8006ECC4u,0x8006EDF0u,0x80095C6Cu,segment,within);
    s.Call(0x80040FA0u,{0x80095C60u});
    return S(s.Read32(0x8006EA44u));
}
// switch 跳转表是原程序代码布局；保存的是模板地址，不是模板首字。
int32_t SelectPortrait800246A8(Services& s,uint32_t mode){
    uint32_t selected=0x8005400Cu;
    switch(mode){
    case 1u:selected=0x80053FFCu;break;
    case 2u:selected=0x80053FECu;break;
    case 3u:selected=0x80053FBCu;break;
    case 4u:selected=0x80053FCCu;break;
    case 5u:selected=0x80053FACu;break;
    case 7u:selected=0x80053FDCu;break;
    }
    s.Write32(0x8006ED5Cu,selected);return S(selected);
}
// 原版先读取坐标对象，再保存镜头记录；参数允许与这两个全局域别名。
int32_t BindCamera800127C4(Services& s,uint32_t camera){
    const uint32_t coordinate=s.Read32(0x80095C7Cu);
    s.Write32(0x8006ECD0u,camera);
    return PrStage2LifecycleDirect::InitCoordinate8004049C(s,0u,coordinate);
}
// BEZ 记录按两个三维半字向量和一个时间字逐项复制，不覆盖向量填充半字。
int32_t LoadCamera800127F0(Services& s,uint32_t source,uint32_t flags){
    s.Write32(0x8006EA48u,flags);s.Write32(0x8006EA44u,0u);
    const uint32_t count=s.Read32(source);source+=4u;s.Write32(0x8006EA40u,count);
    for(uint32_t i=0;i<count;++i){
        for(uint32_t component=0;component<3u;++component)
            s.Write16(0x8006EE10u+8u*i+2u*component,s.Read16(source+2u*component));
        for(uint32_t component=0;component<3u;++component)
            s.Write16(0x8006F180u+8u*i+2u*component,s.Read16(source+6u+2u*component));
        s.Write32(0x8006F4F0u+4u*i,s.Read32(source+12u));source+=16u;
    }
    return 0;
}
// 六个源半字全部读取后才发布视点；随后仍调用原镜头矩阵入口。
int32_t ResetCamera800128DC(Services& s){
    const uint32_t x=H(s.Read16(0x8006EE10u)),y=H(s.Read16(0x8006EE12u)),z=H(s.Read16(0x8006EE14u));
    const uint32_t tx=H(s.Read16(0x8006F180u)),ty=H(s.Read16(0x8006F182u)),tz=H(s.Read16(0x8006F184u));
    s.Write32(0x80095C60u,x);s.Write32(0x80095C64u,y);s.Write32(0x80095C68u,z);
    s.Write32(0x80095C6Cu,tx);s.Write32(0x80095C70u,ty);s.Write32(0x80095C74u,tz);
    return s.Call(0x80040FA0u,{0x80095C60u});
}
// 轨道长度决定下一段来源；保留每次回读，涵盖对象/条目/来源重叠。
int32_t InitTracks80013D10(Services& s,uint32_t object,uint32_t entries,uint32_t source,uint32_t mode){
    s.Write32(object+4u,entries);
    s.Write16(object,s.Read16(source));source+=2u;
    uint32_t count=s.Read16(object),maximum=0u,index=0u;
    s.Write16(object+8u,uint16_t(mode));s.Write16(object+12u,0u);
    while(index<count){
        const uint32_t offset=index<<3u,base=s.Read32(object+4u);
        s.Write16(base+offset,s.Read16(source));
        const uint32_t entry=s.Read32(object+4u)+offset,length=s.Read16(entry);
        source+=2u;if(maximum<length)maximum=length;
        s.Write32(entry+4u,source);
        const uint32_t current=s.Read32(object+4u)+offset;
        ++index;const uint32_t skip=s.Read16(current);count=s.Read16(object);source+=skip<<1u;
    }
    s.Write16(object+10u,uint16_t(maximum));
    return maximum?int32_t(s.Read16(object)):-1;
}
// 索引与帧位置按原版符号半字解释，长度为零也保留原先的负一寻址。
int32_t ReadTrack80013DB8(Services& s,uint32_t object,uint32_t index){
    const uint32_t entry=s.Read32(object+4u)+(H(index)<<3u);
    uint32_t frame=H(s.Read16(object+12u));const uint32_t count=s.Read16(entry);
    if(S(frame)>=int32_t(count))frame=count-1u;
    const uint32_t source=s.Read32(entry+4u);
    return S(H(s.Read16(source+(H(frame)<<1u))));
}
// 返回原始截断帧号；写入的循环帧号可以不同，除零保留原 BREAK。
int32_t SetTrackFrame80013E04(Services& s,uint32_t object,uint32_t frame){
    const uint16_t mode=s.Read16(object+8u),input=uint16_t(frame);
    uint32_t stored=frame;
    if(mode==1u){
        const uint32_t length=s.Read16(object+10u);
        if(!length)s.Break(0x80013E28u,7u);
        stored=uint32_t(input)%length;
    }
    s.Write16(object+12u,uint16_t(stored));return input;
}
// 十组权重各自倒序清零，保留可观察的原写入顺序。
int32_t ClearTrackWeights80013E40(Services& s){
    for(uint32_t bank=0;bank<10u;++bank)
        for(uint32_t left=128u;left;--left)s.Write32(0x80090240u+512u*bank+4u*(left-1u),0u);
    return 0;
}
// 每个描述符七个字；首字先读来源，其余字先回读目标表地址。
int32_t BindModelTable80013650(Services& s,uint32_t source,uint32_t object){
    const uint32_t count=s.Read32(source+8u);source+=12u;s.Write32(object,count);
    if(S(count)<=0)return S(count);
    for(uint32_t i=0;i<count;++i){
        const uint32_t first=s.Read32(source);source+=4u;
        s.Write32(s.Read32(object+8u)+28u*i,first);
        for(uint32_t field=1u;field<7u;++field){
            const uint32_t target=s.Read32(object+8u),value=s.Read32(source);source+=4u;
            s.Write32(target+28u*i+4u*field,value);
        }
    }
    return 0;
}
namespace {
// 两个 ULW 完成后才执行两个 USW，保留重叠复制与原始字节来源。
void CopyVertex(Services& s,uint32_t source,uint32_t output){
    const auto xy=PrStage2SourceWord::LoadUnaligned(s,source);
    const auto zp=PrStage2SourceWord::LoadUnaligned(s,source+4u);
    PrStage2SourceWord::StoreUnaligned(s,output,xy);
    PrStage2SourceWord::StoreUnaligned(s,output+4u,zp);
}
// 上一对象的顶点数推进备份游标；每个对象仍重新读取描述表指针。
void CopyModelVertices(Services& s,uint32_t object,uint32_t backup,uint32_t count,bool save){
    uint32_t previous=0u;
    for(uint32_t i=0;S(i)<S(count);++i){
        backup+=previous<<3u;
        const uint32_t record=s.Read32(object+8u)+28u*i;
        previous=s.Read32(record+4u);
        const uint32_t vertices=s.Read32(record);
        for(uint32_t vertex=0;S(vertex)<S(previous);++vertex){
            const uint32_t a=vertices+8u*vertex,b=backup+8u*vertex;
            CopyVertex(s,save?a:b,save?b:a);
        }
    }
}
// 零长轨道保留先前权重；对象与条目地址每次按原指令重新读取。
void LoadTrackWeights(Services& s,uint32_t object,uint32_t channel){
    uint32_t count=s.Read16(object);
    for(uint32_t i=0;i<count;++i){
        const uint32_t entries=s.Read32(object+4u);
        if(s.Read16(entries+8u*i))
            s.Write32(0x80090240u+512u*channel+4u*i,H(uint32_t(ReadTrack80013DB8(s,object,H(i)))));
        count=s.Read16(object);
    }
}
}
// 原函数没有定义空表路径的标量返回值，公共入口保持 void。
void SaveModelVertices8001371C(Services& s,uint32_t object,uint32_t output){
    const uint32_t count=s.Read32(object);s.Write32(object+4u,output);
    CopyModelVertices(s,object,output,count,true);
}
// VDF 记录包含模型号、首顶点、顶点数及紧随其后的八字节增量数组。
int32_t BindMorphTable800137BC(Services& s,uint32_t source,uint32_t object){
    const uint32_t count=s.Read32(source);source+=4u;s.Write32(object,count);
    if(S(count)<=0)return S(count);
    for(uint32_t i=0;i<count;++i){
        const uint32_t model=s.Read32(source);source+=4u;
        s.Write32(s.Read32(object+4u)+16u*i+12u,model);
        uint32_t target=s.Read32(object+4u),value=s.Read32(source);source+=4u;
        s.Write32(target+16u*i+4u,value);
        target=s.Read32(object+4u);value=s.Read32(source);
        s.Write32(target+16u*i+8u,value);
        target=s.Read32(object+4u);source+=4u;s.Write32(target+16u*i,source);
        target=s.Read32(object+4u);source+=s.Read32(target+16u*i+8u)<<3u;
    }
    return 0;
}
// 通道地址保留原 32 位乘法回绕；没有 VDF 时仍保留原先写入的计数 1。
int32_t InitMorphChannel8001385C(Services& s,uint32_t channel,uint32_t model,uint32_t morph,uint32_t backup){
    const uint32_t offset=12u*channel,object=0x80074B08u+offset;
    s.Write32(object+8u,0x8006F6A8u+112u*channel);
    s.Write32(0x80074B84u+offset,0x8006FB08u+2048u*channel);
    s.Write32(object,1u);s.Write32(object+4u,backup);s.Write32(0x80074B80u+offset,1u);
    s.Write32(0x80074B88u+offset,0x80090240u+512u*channel);
    BindModelTable80013650(s,model,object);
    if(morph){SaveModelVertices8001371C(s,object,backup);BindMorphTable800137BC(s,morph,0x80074B80u+offset);}
    return S(s.Read32(0x80074B80u+offset));
}
// 空模型表返回原通道偏移，非空表返回循环退出时的零比较结果。
int32_t RestoreModelVertices800139F8(Services& s,uint32_t channel){
    const uint32_t offset=12u*channel,object=0x80074B08u+offset;
    const uint32_t count=s.Read32(object),backup=s.Read32(object+4u);
    CopyModelVertices(s,object,backup,count,false);return S(count)>0?0:S(offset);
}
// 原 GPF 使用有符号半字权重并饱和 IR，顶点相加后的 SH 则截断半字。
int32_t ApplyMorphVertices8003A5DC(Services& s,uint32_t vertices,uint32_t deltas,uint32_t count,uint32_t weight){
    auto& g=s.MatrixGte();g.ir0=S(H(weight));
    const uint32_t end=deltas+(count<<3u);uint32_t result;
    do{
        const uint32_t deltaXY=s.Read32(deltas),deltaZ=H(s.Read16(deltas+4u));
        g.ir={S(H(deltaXY)),S(H(deltaXY>>16u)),S(deltaZ)};
        PrPsxGteDirect::ExecuteGeneralMultiply(g);
        const uint32_t xy=s.Read32(vertices),z=H(s.Read16(vertices+4u));
        deltas+=8u;result=(xy&65535u)+uint32_t(g.ir[0]);
        const uint32_t y=H(xy>>16u)+uint32_t(g.ir[1]),outZ=z+uint32_t(g.ir[2]);
        s.Write16(vertices,uint16_t(result));s.Write16(vertices+2u,uint16_t(y));s.Write16(vertices+4u,uint16_t(outZ));
        vertices+=8u;
    }while(deltas!=end);
    return S(result);
}
// 权重为零也先读取模型与 VDF 指针；只有增量顶点数在分支内读取。
int32_t ApplyMorphChannel80013AA8(Services& s,uint32_t channel){
    RestoreModelVertices800139F8(s,channel);
    const uint32_t offset=12u*channel,object=0x80074B80u+offset,count=s.Read32(object);
    if(S(count)<=0)return S(offset);
    for(uint32_t i=0;i<count;++i){
        const uint32_t record=s.Read32(object+4u)+16u*i,model=s.Read32(record+12u);
        const uint32_t table=s.Read32(0x80074B10u+offset),first=s.Read32(record+4u);
        const uint32_t vertices=s.Read32(table+28u*model)+8u*first;
        const uint32_t weights=s.Read32(object+8u),weight=s.Read32(weights+4u*i),deltas=s.Read32(record);
        if(weight)ApplyMorphVertices8003A5DC(s,vertices,deltas,s.Read32(record+8u),weight);
    }
    return 0;
}
// 负帧号先加总长度，截断为半字后交给原轨道循环逻辑。
int32_t ApplyTrackFrame80013EA8(Services& s,uint32_t object,uint32_t frame,uint32_t channel){
    if(S(frame)<0)frame+=s.Read16(object+10u);
    SetTrackFrame80013E04(s,object,frame&65535u);LoadTrackWeights(s,object,channel);
    return ApplyMorphChannel80013AA8(s,channel);
}
// 主角色固定使用第零通道及原程序保留的轨道和顶点备份区。
int32_t InitMainMorph80014164(Services& s,uint32_t tracks,uint32_t morph,uint32_t model,uint32_t mode){
    s.Write32(0x8006EA54u,0u);s.Write32(0x8006EA58u,999u);
    InitMorphChannel8001385C(s,0u,model,morph,0x80074BF8u);
    return InitTracks80013D10(s,0x80078BF8u,0x80078C08u,tracks,mode);
}
// 先递增计数，再应用旧帧；循环余数使用有符号 DIV，不改成无符号取模。
int32_t AdvanceMainMorph800141D8(Services& s){
    uint32_t frame=s.Read32(0x8006EA54u);const uint32_t limit=s.Read32(0x8006EA58u);
    if(S(frame)>=S(limit))return 0;
    s.Write32(0x8006EA54u,frame+1u);
    if(S(frame)<0)frame+=s.Read16(0x80078C02u);
    SetTrackFrame80013E04(s,0x80078BF8u,frame&65535u);LoadTrackWeights(s,0x80078BF8u,0u);
    ApplyMorphChannel80013AA8(s,0u);
    if(s.Read16(0x80078C00u)==1u){
        const uint32_t length=s.Read16(0x80078C02u),current=s.Read32(0x8006EA54u);
        if(!length)s.Break(0x800142D4u,7u);
        s.Write32(0x8006EA54u,uint32_t(S(current)%int32_t(length)));
    }
    return S(s.Read32(0x8006EA54u));
}
// 主通道恢复入口保留原无参数 ABI。
int32_t RestoreMainMorph80014324(Services& s){return RestoreModelVertices800139F8(s,0u);}
// 公共 ABI 逐一验证参数个数，未实现的后续依赖继续交给当前调用拥有者。
bool TryCall(Services& s,uint32_t function,std::initializer_list<uint32_t> args,int32_t& result){
    size_t arity;
    switch(function){
    case 0x800246A8u:case 0x800127C4u:case 0x800139F8u:case 0x80013AA8u:arity=1u;break;
    case 0x800127F0u:case 0x80013DB8u:case 0x80013E04u:
    case 0x80013650u:case 0x8001371Cu:case 0x800137BCu:arity=2u;break;
    case 0x800128DCu:case 0x80013E40u:case 0x800141D8u:case 0x80014324u:
    case 0x80024308u:case 0x80024390u:arity=0u;break;
    case 0x80013994u:arity=2u;break;
    case 0x800140E0u:arity=5u;break;
    case 0x80014050u:arity=6u;break;
    case 0x80012960u:arity=1u;break;
    case 0x80013D10u:case 0x8001385Cu:case 0x80014164u:case 0x8003A5DCu:arity=4u;break;
    case 0x80013EA8u:arity=3u;break;
    default:return false;
    }
    if(args.size()!=arity)throw std::invalid_argument("S2 resource-setup argument count mismatch");
    const auto a=args.begin();
    switch(function){
    case 0x800246A8u:result=SelectPortrait800246A8(s,a[0]);break;
    case 0x800127C4u:result=BindCamera800127C4(s,a[0]);break;
    case 0x800127F0u:result=LoadCamera800127F0(s,a[0],a[1]);break;
    case 0x800128DCu:result=ResetCamera800128DC(s);break;
    case 0x80024308u:result=ResetFirstTrackState80024308(s);break;
    case 0x80024390u:result=ResetSecondTrackState80024390(s);break;
    case 0x80013994u:result=ParseVdfChannel80013994(s,a[0],a[1]);break;
    case 0x800140E0u:result=InitializePrimaryMime800140E0(s,a[0],a[1],a[2],a[3],a[4]);break;
    case 0x80014050u:result=InitializeMimeChannel80014050(s,a[0],a[1],a[2],a[3],a[4],a[5]);break;
    case 0x80012960u:result=UpdateCameraFrame80012960(s,S(a[0]));break;
    case 0x80013D10u:result=InitTracks80013D10(s,a[0],a[1],a[2],a[3]);break;
    case 0x80013DB8u:result=ReadTrack80013DB8(s,a[0],a[1]);break;
    case 0x80013E04u:result=SetTrackFrame80013E04(s,a[0],a[1]);break;
    case 0x80013E40u:result=ClearTrackWeights80013E40(s);break;
    case 0x80013650u:result=BindModelTable80013650(s,a[0],a[1]);break;
    case 0x8001371Cu:throw std::invalid_argument("S2 vertex backup requires CallVoid");
    case 0x800137BCu:result=BindMorphTable800137BC(s,a[0],a[1]);break;
    case 0x8001385Cu:result=InitMorphChannel8001385C(s,a[0],a[1],a[2],a[3]);break;
    case 0x800139F8u:result=RestoreModelVertices800139F8(s,a[0]);break;
    case 0x80013AA8u:result=ApplyMorphChannel80013AA8(s,a[0]);break;
    case 0x8003A5DCu:result=ApplyMorphVertices8003A5DC(s,a[0],a[1],a[2],a[3]);break;
    case 0x80013EA8u:result=ApplyTrackFrame80013EA8(s,a[0],a[1],a[2]);break;
    case 0x80014164u:result=InitMainMorph80014164(s,a[0],a[1],a[2],a[3]);break;
    case 0x800141D8u:result=AdvanceMainMorph800141D8(s);break;
    case 0x80014324u:result=RestoreMainMorph80014324(s);break;
    }
    return true;
}
// 有返回值与无返回值的调用边界分开，避免读取未定义的标量结果。
bool TryCallVoid(Services& s,uint32_t function,std::initializer_list<uint32_t> args){
    if(function==0x8001371Cu){
        if(args.size()!=2u)throw std::invalid_argument("S2 vertex backup argument count mismatch");
        SaveModelVertices8001371C(s,args.begin()[0],args.begin()[1]);return true;
    }
    int32_t ignored;return TryCall(s,function,args,ignored);
}
}
