#include "pr_stage2_lifecycle_direct.h"
#include "pr_stage2_int_loader_direct.h"
#include "pr_stage2_tim_direct.h"
#include "pr_stage2_gpu_direct.h"
#include "logger.h"

#include <cstdlib>
#include <vector>

namespace PrStage2LifecycleDirect {
namespace {
std::vector<TmdPacketTrace> s_tmdPacketTrace;
TmdPacketTrace s_tmdTraceContext{};
bool s_tmdTraceEnabled = false;
bool s_tmdTraceEnabledInitialized = false;

bool IsTmdPacketTraceEnabled() {
    if (!s_tmdTraceEnabledInitialized) {
        s_tmdTraceEnabledInitialized = true;
        s_tmdTraceEnabled = std::getenv("PARAPPA_S2_DRAW_TRACE") != nullptr;
    }
    return s_tmdTraceEnabled;
}

constexpr uint32_t kTransition = 0x801C3640u;
constexpr uint32_t kSaveBank = 0x80092F10u;
constexpr uint32_t kSceneEntry = 0x8006EDB8u;
constexpr uint32_t kModeD0 = 0x800916D0u;
constexpr uint32_t kEasyDA = 0x800916DAu;
constexpr uint32_t kExitE0 = 0x800916E0u;
constexpr uint32_t kHeapLowWater = 0x8006EB80u; // GP + 320, SCUS startup GP.
constexpr uint32_t kHeapDepth = 0x8006EB84u;
constexpr uint32_t kHeapBase = 0x8006EDC0u;
constexpr uint32_t kHeapEnd = 0x8006EDC4u;
constexpr uint32_t kHeapCursor = 0x8006EDC8u;
constexpr uint32_t kResourceTable = 0x80091858u;

int32_t Signed16(uint16_t v) {
    return v < 0x8000u ? static_cast<int32_t>(v)
                       : static_cast<int32_t>(v) - 0x10000;
}
uint32_t U(int32_t v) { return static_cast<uint32_t>(v); }
int32_t Signed32(uint32_t v) {
    return v <= 0x7FFFFFFFu ? static_cast<int32_t>(v)
        : static_cast<int32_t>(static_cast<int64_t>(v) - 0x100000000LL);
}
uint32_t Sra(uint32_t value, uint32_t shift) {
    shift &= 31u;
    if (shift == 0u) return value;
    return (value >> shift) | ((value & 0x80000000u) != 0u ? (~0u << (32u - shift)) : 0u);
}
uint32_t Extend16(uint32_t value) { return U(Signed16(static_cast<uint16_t>(value))); }
}

void ResetTmdPacketTrace() {
    if (!IsTmdPacketTraceEnabled()) return;
    s_tmdPacketTrace.clear();
    s_tmdTraceContext = {};
}

const std::vector<TmdPacketTrace>& GetTmdPacketTrace() {
    return s_tmdPacketTrace;
}

namespace {
// Original DrawModels uses a private stack matrix. Keep it native; never
// reserve a made-up PSX RAM address. Public helpers also accept genuine RAM
// matrices so their original overlap and read/write ordering is retained.
struct MatrixRef {
    Services& s;
    uint32_t address;
    Matrix32* local = nullptr;
    uint32_t Read(uint32_t offset) const {
        return local ? local->words[offset / 4u] : s.Read32(address + offset);
    }
    uint32_t Half(uint32_t offset) const {
        return local ? ((local->words[offset / 4u] >> (8u * (offset & 3u))) & 0xFFFFu)
                     : s.Read16(address + offset);
    }
    void Write(uint32_t offset, uint32_t value) const {
        if (local) local->words[offset / 4u] = value;
        else s.Write32(address + offset, value);
    }
    // 私有矩阵的半字写入保留同一字中的另一半，公共矩阵仍使用原 SH。
    void WriteHalf(uint32_t offset,uint32_t value) const {
        if(local){
            const uint32_t shift=8u*(offset&3u),mask=0xFFFFu<<shift;
            auto& word=local->words[offset/4u];word=(word&~mask)|((value&0xFFFFu)<<shift);
        }else s.Write16(address+offset,uint16_t(value));
    }
};

void LoadRotation(const MatrixRef& matrix) {
    uint32_t words[5];
    for (uint32_t i = 0; i < 5; ++i) words[i] = matrix.Read(4u * i);
    auto& state = matrix.s.MatrixGte();
    for (uint32_t i = 0; i < 4; ++i) state.matrix.words[i] = words[i];
    state.matrix.words[4] = Extend16(words[4]);
}
void LoadTranslation(const MatrixRef& matrix) {
    const uint32_t x = matrix.Read(20u), y = matrix.Read(24u), z = matrix.Read(28u);
    auto& state = matrix.s.MatrixGte();
    state.matrix.words[5] = x; state.matrix.words[6] = y; state.matrix.words[7] = z;
}
Vector32 ApplyLongVector(const MatrixRef& matrix, const MatrixRef& vector, uint32_t offset) {
    LoadRotation(matrix);
    const uint32_t values[3] = {vector.Read(offset), vector.Read(offset + 4u), vector.Read(offset + 8u)};
    uint32_t high[3], low[3];
    for (uint32_t i = 0; i < 3; ++i) {
        const bool negative = Signed32(values[i]) < 0;
        const uint32_t magnitude = negative ? 0u - values[i] : values[i];
        high[i] = negative ? 0u - Sra(magnitude, 15u) : Sra(magnitude, 15u);
        low[i] = negative ? 0u - (magnitude & 0x7FFFu) : magnitude & 0x7FFFu;
    }
    auto& gte = matrix.s.MatrixGte();
    for (uint32_t i = 0; i < 3; ++i) gte.ir[i] = Signed16(static_cast<uint16_t>(high[i]));
    PrPsxGteDirect::ExecuteRotationMvmva(gte, true, false);
    const auto upper = gte.mac;
    for (uint32_t i = 0; i < 3; ++i) gte.ir[i] = Signed16(static_cast<uint16_t>(low[i]));
    PrPsxGteDirect::ExecuteRotationMvmva(gte, true, true);
    // Original NEG/SHL/NEG in the negative branch has the same wrapped bits.
    return {U(gte.mac[0]) + (U(upper[0]) << 3u),
            U(gte.mac[1]) + (U(upper[1]) << 3u),
            U(gte.mac[2]) + (U(upper[2]) << 3u)};
}
void MultiplyMatrix(const MatrixRef& left, const MatrixRef& right, const MatrixRef& out) {
    LoadRotation(left);
    auto& gte = left.s.MatrixGte();
    uint32_t xy = right.Half(0u), middle = right.Read(4u), z = right.Read(12u);
    gte.vectorXY0 = xy | (middle & 0xFFFF0000u); gte.vectorZ0 = Signed16(static_cast<uint16_t>(z));
    PrPsxGteDirect::ExecuteRotationMvmva(gte, false, true);
    xy = right.Half(2u); middle = right.Read(8u); z = right.Half(14u);
    const auto first = gte.ir;
    gte.vectorXY0 = xy | (middle << 16u); gte.vectorZ0 = Signed16(static_cast<uint16_t>(z));
    PrPsxGteDirect::ExecuteRotationMvmva(gte, false, true);
    xy = right.Half(4u); middle = right.Read(8u); z = right.Read(16u);
    const auto second = gte.ir;
    gte.vectorXY0 = xy | (middle & 0xFFFF0000u); gte.vectorZ0 = Signed16(static_cast<uint16_t>(z));
    PrPsxGteDirect::ExecuteRotationMvmva(gte, false, true);
    out.Write(0u, (U(second[0]) << 16u) | (U(first[0]) & 0xFFFFu));
    out.Write(12u, (U(second[2]) << 16u) | (U(first[2]) & 0xFFFFu));
    out.Write(4u, (U(first[1]) << 16u) | (U(gte.ir[0]) & 0xFFFFu));
    out.Write(8u, (U(gte.ir[1]) << 16u) | (U(second[1]) & 0xFFFFu));
    out.Write(16u, U(gte.ir[2])); // SWC2 writes sign extension into the pad too.
}
int32_t ComposeMatrix(const MatrixRef& left, const MatrixRef& right, const MatrixRef& out) {
    const Vector32 translation = ApplyLongVector(left, right, 20u);
    MultiplyMatrix(left, right, out);
    out.Write(20u, translation.x + left.Read(20u));
    out.Write(24u, translation.y + left.Read(24u));
    const uint32_t z = translation.z + left.Read(28u);
    out.Write(28u, z);
    return Signed32(z);
}
void CopyMatrix(const MatrixRef& from, uint32_t sourceOffset, const MatrixRef& to, uint32_t destOffset) {
    for (uint32_t block : {0u, 16u}) {
        const uint32_t a = from.Read(sourceOffset + block), b = from.Read(sourceOffset + block + 4u);
        const uint32_t c = from.Read(sourceOffset + block + 8u), d = from.Read(sourceOffset + block + 12u);
        to.Write(destOffset + block, a); to.Write(destOffset + block + 4u, b);
        to.Write(destOffset + block + 8u, c); to.Write(destOffset + block + 12u, d);
    }
}
int32_t ResolveWorldMatrix(Services& s, uint32_t coordinate, const MatrixRef& out,bool applyView=true) {
    uint32_t depth = 0u, dirty = 100u;
    while (true) {
        s.Write32(0x80095C80u + 4u * depth, coordinate);
        const uint32_t parent = s.Read32(coordinate + 72u);
        const uint32_t flag = s.Read32(coordinate);
        const uint32_t frame = s.Read32(0x8009658Cu);
        if (parent == 0u) {
            if (flag == frame || flag == 0u) {
                CopyMatrix({s, coordinate}, 4u, {s, coordinate}, 36u);
                CopyMatrix({s, coordinate}, 36u, out, 0u);
                s.Write32(coordinate, s.Read32(0x8009658Cu));
            } else {
                depth = dirty + 1u;
                const uint32_t cached = s.Read32(0x80095C80u + (dirty == 100u ? 0u : 4u * depth));
                CopyMatrix({s, cached}, 36u, out, 0u);
                if (dirty == 100u) depth = 0u;
            }
            break;
        }
        if (flag == frame) {
            CopyMatrix({s, coordinate}, 36u, out, 0u);
            break;
        }
        coordinate = parent;
        if (flag == 0u) dirty = depth;
        ++depth;
    }
    uint32_t slot = 0x80095C7Cu + 4u * depth;
    uint32_t result=depth<<2u;
    while (Signed32(depth) > 0) {
        ComposeMatrix(out, {s, s.Read32(slot) + 4u}, out);
        const uint32_t child = s.Read32(slot);
        --depth;
        CopyMatrix(out, 0u, {s, child}, 36u);
        const uint32_t target = s.Read32(slot);
        const uint32_t frame = s.Read32(0x8009658Cu);
        result=frame;
        slot -= 4u;
        s.Write32(target, frame);
    }
    return applyView?ComposeMatrix({s, 0x80092880u}, out, out):Signed32(result);
}
}

void LoadRotation8003F6B0(Services& s, uint32_t matrix) { LoadRotation({s, matrix}); }
void LoadTranslation8003F6E0(Services& s, uint32_t matrix) { LoadTranslation({s, matrix}); }
void LoadGteMatrix80040544(Services& s, uint32_t matrix) {
    LoadRotation({s, matrix}); LoadTranslation({s, matrix});
}
int32_t ApplyMatrixLongVector8003ABA4(Services& s, uint32_t matrix, uint32_t vector, uint32_t output) {
    const Vector32 result = ApplyLongVector({s, matrix}, {s, vector}, 0u);
    s.Write32(output, result.x); s.Write32(output + 4u, result.y); s.Write32(output + 8u, result.z);
    return Signed32(output);
}
int32_t MultiplyMatrixLeft80040884(Services& s, uint32_t left, uint32_t right) {
    MultiplyMatrix({s, left}, {s, right}, {s, left}); return Signed32(left);
}
int32_t MultiplyMatrixRight80040994(Services& s, uint32_t left, uint32_t right) {
    MultiplyMatrix({s, left}, {s, right}, {s, right}); return Signed32(right);
}
int32_t ComposeMatrixLeft8004075C(Services& s, uint32_t left, uint32_t right) {
    return ComposeMatrix({s, left}, {s, right}, {s, left});
}
int32_t ComposeMatrixRight800406D8(Services& s, uint32_t left, uint32_t right) {
    return ComposeMatrix({s, left}, {s, right}, {s, right});
}
int32_t ResolveWorldMatrix80041A68(Services& s, uint32_t coordinate, uint32_t output) {
    return ResolveWorldMatrix(s, coordinate, {s, output});
}
// 800417A4 只解析坐标层级，不追加 80041A68 的视图矩阵组合。
int32_t ResolveCoordinateMatrix800417A4(Services& s,uint32_t coordinate,uint32_t output){
    return ResolveWorldMatrix(s,coordinate,{s,output},false);
}

namespace {
// 镜头路径的原复制分组为三字、三字、两字，与通用四字分组不同。
void CopyCameraMatrix(const MatrixRef& from,const MatrixRef& to){
    for(uint32_t offset:{0u,12u}){
        const uint32_t a=from.Read(offset),b=from.Read(offset+4u),c=from.Read(offset+8u);
        to.Write(offset,a);to.Write(offset+4u,b);to.Write(offset+8u,c);
    }
    const uint32_t a=from.Read(24u),b=from.Read(28u);to.Write(24u,a);to.Write(28u,b);
}
// 转置的读取和交错写入次序决定源/目标重叠时的结果。
void TransposeCameraMatrix(const MatrixRef& from,const MatrixRef& to){
    uint32_t a=from.Half(0u);to.WriteHalf(0u,a);
    uint32_t b=from.Half(6u);a=from.Half(12u);to.WriteHalf(2u,b);
    uint32_t c=from.Half(2u);to.WriteHalf(4u,a);b=from.Half(8u);to.WriteHalf(6u,c);
    a=from.Half(14u);to.WriteHalf(8u,b);c=from.Half(4u);to.WriteHalf(10u,a);
    b=from.Half(10u);to.WriteHalf(12u,c);a=from.Half(16u);to.WriteHalf(14u,b);to.WriteHalf(16u,a);
}
// 构造使用镜像中的模板，不能以主机单位矩阵替代可修改的来源数据。
int32_t BuildCameraAxis(Services& s,const MatrixRef& out,uint32_t sine,uint32_t cosine,uint32_t axis){
    CopyCameraMatrix({s,0x80091838u},out);
    switch(axis&255u){
    case 'X':case 'x':
        out.WriteHalf(8u,cosine);out.WriteHalf(16u,cosine);out.WriteHalf(10u,0u-sine);out.WriteHalf(14u,sine);break;
    case 'Y':case 'y':
        out.WriteHalf(0u,cosine);out.WriteHalf(16u,cosine);out.WriteHalf(4u,sine);out.WriteHalf(12u,0u-sine);break;
    case 'Z':case 'z':
        out.WriteHalf(0u,cosine);out.WriteHalf(8u,cosine);out.WriteHalf(2u,0u-sine);out.WriteHalf(6u,sine);break;
    default:
        // 原 switch 的 V0 是范围判断或跳转目标，需在公共包装层保留。
        return 0;
    }
    return Signed32(0u-sine);
}
// 镜头点缩放保留重复读取及输出别名语义，私有输出由本机矩阵承载。
int32_t ScaleCameraPoints(Services& s,uint32_t source,const MatrixRef& out){
    const uint32_t bits=U(PositiveBitLength8004152C(s,U(CameraMagnitude80041464(s,source))));
    uint32_t value=0u;
    for(uint32_t i=0;i<6u;++i){
        value=s.Read32(source+4u*i);if(bits>=16u)value=Sra(value,bits-15u);
        out.Write(4u*i,value);
    }
    return Signed32(value);
}
// 原 DIV 后的显式 BREAK 不允许转换为 C++ 有符号除法溢出。
uint32_t CameraDivide(Services& s,uint32_t numerator,uint32_t denominator,uint32_t zeroPc,uint32_t overflowPc){
    if(!denominator)s.Break(zeroPc,7u);
    if(numerator==0x80000000u&&denominator==0xFFFFFFFFu)s.Break(overflowPc,6u);
    return U(Signed32(numerator)/Signed32(denominator));
}
}
// INT_MIN 的取负仍保持原位模式，随后用原有符号比较选择幅值。
int32_t CameraMagnitude80041464(Services& s,uint32_t source){
    uint32_t maximum=s.Read32(source),value=s.Read32(source+4u);
    if(Signed32(maximum)<0)maximum=0u-maximum;
    if(Signed32(value)<0)value=0u-value;
    if(Signed32(maximum)<Signed32(value))maximum=value;
    for(uint32_t offset=8u;offset<24u;offset+=4u){
        value=s.Read32(source+offset);if(Signed32(value)<0)value=0u-value;
        if(Signed32(maximum)<Signed32(value))maximum=value;
    }
    return Signed32(maximum);
}
// 原函数对零和负数返回零，只对有符号正数计算位数。
int32_t PositiveBitLength8004152C(Services&,uint32_t value){
    int32_t bits=0;while(Signed32(value)>0){value=Sra(value,1u);++bits;}return bits;
}
int32_t ScaleCameraPoints80041374(Services& s,uint32_t source,uint32_t output){
    return ScaleCameraPoints(s,source,{s,output});
}
// LZCS/LZCR 按前导符号位更新，平方根仍读取原 SCUS 查找表。
int32_t CameraSquareRoot80041548(Services& s,uint32_t value){
    auto& g=s.MatrixGte();g.lzcs=value;
    uint32_t bits=(value&0x80000000u)?~value:value,count=0u;
    while(count<32u&&!(bits&0x80000000u)){++count;bits<<=1u;}
    g.lzcr=count;if(count==32u)return 0;
    const uint32_t even=count&~1u,shift=(31u-even)>>1u;
    const uint32_t normalized=even>=24u?value<<((even-24u)&31u):Sra(value,24u-even);
    const uint32_t table=Extend16(s.Read16(0x8005C9D4u+((normalized-64u)<<1u)));
    return Signed32((table<<(shift&31u))>>12u);
}
int32_t TransposeCameraMatrix800415D8(Services& s,uint32_t source,uint32_t output){
    TransposeCameraMatrix({s,source},{s,output});return Signed32(output);
}
int32_t BuildCameraAxis800416E0(Services& s,uint32_t output,uint32_t sine,uint32_t cosine,uint32_t axis){
    const int32_t result=BuildCameraAxis(s,{s,output},sine,cosine,axis);
    const uint32_t index=(axis&255u)-88u;
    // 越界分支也执行延迟槽中的左移，必须保留其返回值。
    if(index>=35u)return Signed32(index<<2u);
    if((axis&255u)!=88u&&(axis&255u)!=89u&&(axis&255u)!=90u&&
       (axis&255u)!=120u&&(axis&255u)!=121u&&(axis&255u)!=122u)return Signed32(0x8004179Cu);
    return result;
}
// 原正弦先处理角度符号，再按四象限读取有符号半字表。
int32_t CameraSine8003A1D0(Services& s,uint32_t angle){
    const bool negative=Signed32(angle)<0;
    const uint32_t index=(negative?0u-angle:angle)&4095u;
    uint32_t value;
    if(index<=1024u)value=Extend16(s.Read16(0x8005750Cu+2u*index));
    else if(index<=2048u)value=Extend16(s.Read16(0x8005750Cu+2u*(2048u-index)));
    else if(index<=3072u)value=0u-Extend16(s.Read16(0x8005650Cu+2u*index));
    else value=0u-Extend16(s.Read16(0x8005750Cu+2u*(4096u-index)));
    return Signed32(negative?0u-value:value);
}
// 余弦使用原有符号幅值和象限边界，INT_MIN 仍按 32 位取负回绕。
int32_t CameraCosine8003A29C(Services& s,uint32_t angle){
    const uint32_t index=(Signed32(angle)<0?0u-angle:angle)&4095u;
    if(index<=1024u)return Signed32(Extend16(s.Read16(0x8005750Cu+2u*(1024u-index))));
    if(index<=2048u)return Signed32(0u-Extend16(s.Read16(0x80056D0Cu+2u*index)));
    if(index<=3072u)return Signed32(0u-Extend16(s.Read16(0x8005750Cu+2u*(3072u-index))));
    return Signed32(Extend16(s.Read16(0x80055D0Cu+2u*index)));
}
// 角度先用原有符号截断除以 360；零滚转仍执行正弦与余弦查表。
int32_t RollCamera80041628(Services& s,uint32_t matrix,uint32_t angle){
    const uint32_t divided=U(Signed32(angle)/360);
    const uint32_t cosine=U(CameraCosine8003A29C(s,divided));
    const uint32_t sine=U(CameraSine8003A1D0(s,divided));
    if(!angle)return Signed32(sine);
    Matrix32 rotation{};const MatrixRef local{s,0u,&rotation};
    local.WriteHalf(2u,0u-sine);local.WriteHalf(0u,cosine);
    local.WriteHalf(4u,0u);local.WriteHalf(6u,sine);local.WriteHalf(8u,cosine);
    local.WriteHalf(10u,0u);local.WriteHalf(12u,0u);local.WriteHalf(14u,0u);local.WriteHalf(16u,4096u);
    local.Write(20u,0u);local.Write(24u,0u);local.Write(28u,0u);
    MultiplyMatrix({s,matrix},local,{s,matrix});return Signed32(matrix);
}
// 原 80040FA0 的私有矩阵和向量保持本机存储，共享矩阵运算保留 GTE 副作用。
int32_t SetCamera80040FA0(Services& s,uint32_t view){
    const MatrixRef camera{s,0x80092880u};
    CopyCameraMatrix({s,0x800928A8u},camera);
    RollCamera80041628(s,camera.address,0u-s.Read32(view+24u));
    Matrix32 points{},rotation{},inverse{},vector{};
    const MatrixRef p{s,0u,&points},r{s,0u,&rotation},inv{s,0u,&inverse},v{s,0u,&vector};
    ScaleCameraPoints(s,view,p);
    const uint32_t dx=p.Read(12u)-p.Read(0u),dy=p.Read(16u)-p.Read(4u),dz=p.Read(20u)-p.Read(8u);
    const uint32_t length=U(CameraSquareRoot80041548(s,dx*dx+dy*dy+dz*dz));
    if(!length)return 1;
    const uint32_t pitch=0u-CameraDivide(s,(p.Read(4u)-p.Read(16u))<<12u,length,0x800410B0u,0x800410C8u);
    const uint32_t horizontal=U(CameraSquareRoot80041548(s,dx*dx+dz*dz));
    const uint32_t cosine=CameraDivide(s,horizontal<<12u,length,0x80041120u,0x80041138u);
    BuildCameraAxis(s,r,Extend16(pitch),Extend16(cosine),'x');MultiplyMatrix(camera,r,camera);
    if(horizontal){
        const uint32_t sine=0u-CameraDivide(s,dx<<12u,horizontal,0x80041194u,0x800411ACu);
        const uint32_t z=CameraDivide(s,dz<<12u,horizontal,0x800411D4u,0x800411ECu);
        BuildCameraAxis(s,r,Extend16(sine),Extend16(z),'y');MultiplyMatrix(camera,r,camera);
    }
    v.Write(0u,0u-s.Read32(view));v.Write(4u,0u-s.Read32(view+4u));v.Write(8u,0u-s.Read32(view+8u));
    const auto translated=ApplyLongVector(camera,v,0u);
    camera.Write(20u,translated.x);camera.Write(24u,translated.y);camera.Write(28u,translated.z);
    const uint32_t parent=s.Read32(view+28u);
    if(parent){
        ResolveWorldMatrix(s,parent,r,false);TransposeCameraMatrix(r,inv);
        const auto parentTranslation=ApplyLongVector(inv,r,20u);
        inv.Write(20u,0u-parentTranslation.x);inv.Write(28u,0u-parentTranslation.z);inv.Write(24u,0u-parentTranslation.y);
        ComposeMatrix(camera,inv,inv);CopyMatrix(inv,0u,camera,0u);
    }
    CopyCameraMatrix(camera,{s,0x800901D8u});return 0;
}

int32_t MapModelData80040BFC(Services& s, uint32_t flagsAddress) {
    const uint32_t flags = s.Read32(flagsAddress);
    if ((flags & 1u) != 0u) return 1;
    s.Write32(flagsAddress, flags | 1u);
    const int32_t count = Signed32(s.Read32(flagsAddress + 4u));
    const uint32_t table = flagsAddress + 8u;
    if (count <= 0) return Signed32(flags | 1u);
    for (uint32_t i = 0; Signed32(i) < count; ++i) {
        const uint32_t object = table + 28u * i;
        const uint32_t vertices = s.Read32(object);
        const uint32_t primitives = s.Read32(object + 16u);
        s.Write32(object, table + vertices);
        const uint32_t normals = s.Read32(object + 8u);
        s.Write32(object + 16u, table + primitives);
        s.Write32(object + 8u, table + normals);
    }
    return 0;
}

int32_t GroupModelPrimitives8004274C(Services& s, uint32_t table,
    uint32_t descriptor, uint32_t index) {
    const uint32_t offset = 28u * index;
    const uint32_t object = table + offset;
    s.Write32(descriptor + 8u, object);
    uint32_t primitive = s.Read32(object + 16u);
    const uint32_t count = s.Read32(object + 20u);
    uint32_t group = primitive, groupCount = 0u, mode = 0u, flag = 0u;
    for (uint32_t i = 0; i < count; ++i) {
        const uint32_t header = s.Read32(primitive);
        const uint32_t previousMode = mode, previousFlag = flag;
        mode = header >> 24u;
        flag = (header >> 16u) & 0xFFu;
        if (previousMode != 0u && (mode != previousMode || flag != previousFlag)) {
            s.Write16(group, static_cast<uint16_t>(groupCount));
            groupCount = 0u;
            group = primitive;
        }
        // Only the stride dispatch masks bit 1; grouping uses the full bytes.
        switch (mode & 0xFDu) {
        case 0x20u: primitive += (flag & 4u) != 0u ? 24u : 16u; break;
        case 0x21u: case 0x29u: primitive += 16u; break;
        case 0x24u: case 0x31u: case 0x38u: primitive += 24u; break;
        case 0x25u: case 0x34u: case 0x39u: primitive += 28u; break;
        case 0x28u: primitive += 20u; break;
        case 0x2Cu: case 0x2Du: primitive += 32u; break;
        case 0x30u: primitive += (flag & 4u) != 0u ? 28u : 20u; break;
        case 0x35u: case 0x3Cu: primitive += 36u; break;
        case 0x3Du: primitive += 44u; break;
        default:
            s.Call(0x80047FFCu, {0x8001216Cu, mode});
            break; // Original diagnostic does not advance the primitive pointer.
        }
        ++groupCount;
    }
    // Even an empty object writes a zero halfword at its primitive pointer.
    s.Write16(group, static_cast<uint16_t>(groupCount));
    return count == 0u ? Signed32(offset) : 0;
}

int32_t BindModel8001AF1C(Services& s, uint32_t source,
    uint32_t descriptor, uint32_t parent) {
    MapModelData80040BFC(s, source + 4u);
    const int32_t count = Signed32(s.Read32(source + 8u));
    const uint32_t table = source + 12u;
    for (uint32_t i = 0; Signed32(i) < count; ++i)
        GroupModelPrimitives8004274C(s, table, descriptor + 16u * i, i);
    // This is a separate second pass, after every object's primitive grouping.
    for (uint32_t i = 0; Signed32(i) < count; ++i) {
        s.Write32(descriptor + 16u * i + 4u, parent);
        s.Write32(descriptor + 16u * i, 0u);
    }
    return count;
}

int32_t InitAnimationCursor8001AFD8(Services& s, uint32_t source,
    uint32_t cursorAddress, uint32_t countAddress) {
    const uint32_t first = source + 4u;
    s.Write32(cursorAddress, first);
    s.Write32(countAddress, s.Read32(first));
    const uint32_t cursor = s.Read32(cursorAddress) + 4u;
    s.Write32(cursorAddress, cursor);
    return Signed32(cursor);
}

int32_t InitCoordinate8004049C(Services& s, uint32_t parent, uint32_t coordinate) {
    // Original three/three/two read groups matter when the template aliases
    // the destination. A host memcpy or preloaded identity matrix differs.
    for (uint32_t offset : {0u, 12u}) {
        const uint32_t a = s.Read32(0x80091838u + offset);
        const uint32_t b = s.Read32(0x8009183Cu + offset);
        const uint32_t c = s.Read32(0x80091840u + offset);
        s.Write32(coordinate + 4u + offset, a);
        s.Write32(coordinate + 8u + offset, b);
        s.Write32(coordinate + 12u + offset, c);
    }
    const uint32_t a = s.Read32(0x80091850u);
    const uint32_t b = s.Read32(0x80091854u);
    s.Write32(coordinate + 28u, a);
    s.Write32(coordinate + 32u, b);
    s.Write32(coordinate + 72u, parent);
    s.Write32(coordinate, 0u);
    if (parent == 0u) return Signed32(a);
    if (parent == 1u) return 1;
    const uint32_t linkedParent = s.Read32(coordinate + 72u);
    s.Write32(linkedParent + 76u, coordinate);
    return Signed32(linkedParent);
}

namespace {
int32_t RotateMatrixRows(Services& s, uint32_t angle, uint32_t matrix,
    uint32_t firstRow, uint32_t secondRow, bool negatePositive) {
    const bool negative = Signed32(angle) < 0;
    const uint32_t magnitude = negative ? 0u - angle : angle;
    // Original packed signed-16 sin/cos, including INT_MIN wrapping and the
    // 12-bit index. Do not substitute host floating-point trigonometry.
    const uint32_t packed = s.Read32(0x800581C0u + 4u * (magnitude & 0xFFFu));
    const uint32_t cosine = Sra(packed, 16u);
    uint32_t sine = Extend16(packed);
    if (negative != negatePositive) sine = 0u - sine;
    uint32_t first[3], second[3];
    for (uint32_t i = 0u; i < 3u; ++i) {
        first[i] = Extend16(s.Read16(matrix + firstRow + 2u * i));
        second[i] = Extend16(s.Read16(matrix + secondRow + 2u * i));
    }
    // All six inputs precede the first SH. MULTU/MFLO and ADDU/SUBU retain
    // only 32 bits before SRA, even when the packed table aliases the matrix.
    for (uint32_t i = 0u; i < 3u; ++i)
        s.Write16(matrix + firstRow + 2u * i,
            static_cast<uint16_t>(Sra(cosine * first[i] - sine * second[i], 12u)));
    for (uint32_t i = 0u; i < 3u; ++i)
        s.Write16(matrix + secondRow + 2u * i,
            static_cast<uint16_t>(Sra(sine * first[i] + cosine * second[i], 12u)));
    return Signed32(matrix);
}
}

int32_t RotateMatrixY8003B3CC(Services& s, uint32_t angle, uint32_t matrix) {
    return RotateMatrixRows(s, angle, matrix, 0u, 12u, true);
}

int32_t RotateMatrixX8003B22C(Services& s, uint32_t angle, uint32_t matrix) {
    return RotateMatrixRows(s, angle, matrix, 6u, 12u, false);
}

int32_t RotateMatrixZ8003B56C(Services& s, uint32_t angle, uint32_t matrix) {
    return RotateMatrixRows(s, angle, matrix, 0u, 6u, false);
}

int32_t ScaleMatrix8003B0FC(Services& s, uint32_t matrix, uint32_t scale) {
    uint32_t packed = s.Read32(matrix);
    const uint32_t x = s.Read32(scale);
    const uint32_t y = s.Read32(scale + 4u);
    const uint32_t z = s.Read32(scale + 8u);
    const uint32_t factors[3] = {x, y, z};
    for (uint32_t i = 0u; i < 4u; ++i) {
        // Original pipelined LW: capture the next pair before storing this
        // pair, rather than re-reading after SW or preloading the whole matrix.
        const uint32_t next = s.Read32(matrix + 4u * (i + 1u));
        const uint32_t low = Sra(Extend16(packed) * factors[(2u * i) % 3u], 12u);
        const uint32_t high = Sra(Sra(packed, 16u) * factors[(2u * i + 1u) % 3u], 12u);
        s.Write32(matrix + 4u * i, (low & 0xFFFFu) | (high << 16u));
        packed = next;
    }
    // This is SW, not SH: the original also overwrites the padding halfword.
    s.Write32(matrix + 16u, Sra(Extend16(packed) * z, 12u));
    return Signed32(matrix);
}

int32_t TranslateMatrix8003B0CC(Services& s, uint32_t matrix, uint32_t translation) {
    const uint32_t x = s.Read32(translation);
    const uint32_t y = s.Read32(translation + 4u);
    const uint32_t z = s.Read32(translation + 8u);
    s.Write32(matrix + 20u, x);
    s.Write32(matrix + 24u, y);
    s.Write32(matrix + 28u, z);
    return Signed32(matrix);
}

int32_t ApplyTodCommand80028054(Services& s, uint32_t command, uint32_t descriptor) {
    const uint32_t header = s.Read32(command);
    const uint32_t kind = (header >> 16u) & 15u;
    const uint32_t flags = (header >> 20u) & 15u;
    const uint32_t next = command + 4u * (header >> 24u);
    // Original null-descriptor results live only in private stack locals
    // (sp+10/20/40, saved registers start at sp+68). All five callees are pure
    // integer memory transforms: no GTE or device state survives this call.
    // Eliminate those dead local results; do not fabricate PSX scratch memory
    // or assign zero to the original uninitialized locals. Resource command
    // storage is independent of the callee's private activation record.
    if (descriptor == 0u) return Signed32(next);
    const uint32_t coordinate = s.Read32(descriptor + 4u);
    const uint32_t parameters = s.Read32(coordinate + 68u);
    const uint32_t matrix = coordinate + 4u;
    s.Write32(coordinate, 0u); // Also done for unrecognized command kinds.
    uint32_t data = command + 4u;
    if (kind == 0u) {
        const uint32_t current = s.Read32(descriptor);
        const uint32_t mask = s.Read32(data);
        const uint32_t bits = s.Read32(data + 4u);
        s.Write32(descriptor, (current & mask) | bits);
    } else if (kind == 4u) {
        constexpr uint32_t offsets[9] = {0u, 6u, 12u, 2u, 8u, 14u, 4u, 10u, 16u};
        for (uint32_t i = 0u; i < 9u; ++i) {
            const uint16_t value = s.Read16(data + 2u * i);
            s.Write16(matrix + offsets[i], value);
        }
        for (uint32_t i = 0u; i < 3u; ++i) {
            const uint32_t value = s.Read32(data + 20u + 4u * i);
            s.Write32(matrix + 20u + 4u * i, value);
        }
    } else if (kind == 1u) {
        for (uint32_t offset : {0u, 8u, 16u}) s.Write16(matrix + offset, 4096u);
        for (uint32_t offset : {4u, 2u, 10u, 6u, 14u, 12u}) s.Write16(matrix + offset, 0u);
        const auto rotate = [&]() {
            RotateMatrixY8003B3CC(s, Extend16(s.Read16(parameters + 18u)), matrix);
            RotateMatrixX8003B22C(s, Extend16(s.Read16(parameters + 16u)), matrix);
            RotateMatrixZ8003B56C(s, Extend16(s.Read16(parameters + 20u)), matrix);
        };
        if ((flags & 1u) != 0u) {
            if ((flags & 2u) != 0u) {
                for (uint32_t i = 0u; i < 3u; ++i) {
                    const int32_t input = Signed32(s.Read32(data + 4u * i));
                    const uint32_t previous = s.Read16(parameters + 16u + 2u * i);
                    s.Write16(parameters + 16u + 2u * i,
                        static_cast<uint16_t>(previous + U(input / 360)));
                }
                data += 12u;
            }
            if ((flags & 4u) != 0u) {
                for (uint32_t i = 0u; i < 3u; ++i) {
                    const uint32_t input = Extend16(s.Read16(data + 2u * i));
                    const uint32_t previous = s.Read32(parameters + 4u * i);
                    s.Write32(parameters + 4u * i, U(Signed32(previous * input) / 4096));
                }
                data += 8u; // Includes the original padding halfword.
            }
            if ((flags & 8u) != 0u) {
                for (uint32_t i = 0u; i < 3u; ++i) {
                    const uint32_t previous = s.Read32(parameters + 24u + 4u * i);
                    const uint32_t input = s.Read32(data + 4u * i);
                    s.Write32(parameters + 24u + 4u * i, previous + input);
                }
            }
            rotate();
            ScaleMatrix8003B0FC(s, matrix, parameters);
            TranslateMatrix8003B0CC(s, matrix, parameters + 24u);
        } else {
            if ((flags & 2u) != 0u) {
                for (uint32_t i = 0u; i < 2u; ++i) {
                    const int32_t input = Signed32(s.Read32(data + 4u * i));
                    s.Write16(parameters + 16u + 2u * i, static_cast<uint16_t>(input / 360));
                }
                const int32_t z = Signed32(s.Read32(data + 8u));
                data += 12u;
                // The original captures Y before the Z store (JAL delay slot).
                const uint32_t y = Extend16(s.Read16(parameters + 18u));
                s.Write16(parameters + 20u, static_cast<uint16_t>(z / 360));
                RotateMatrixY8003B3CC(s, y, matrix);
                RotateMatrixX8003B22C(s, Extend16(s.Read16(parameters + 16u)), matrix);
                RotateMatrixZ8003B56C(s, Extend16(s.Read16(parameters + 20u)), matrix);
            }
            if ((flags & 4u) != 0u) {
                for (uint32_t i = 0u; i < 3u; ++i) {
                    const uint32_t input = Extend16(s.Read16(data + 2u * i));
                    s.Write32(parameters + 4u * i, input);
                }
                data += 8u;
                if ((flags & 2u) == 0u) rotate();
                ScaleMatrix8003B0FC(s, matrix, parameters);
            }
            if ((flags & 8u) != 0u) {
                for (uint32_t i = 0u; i < 3u; ++i) {
                    const uint32_t input = s.Read32(data + 4u * i);
                    s.Write32(parameters + 24u + 4u * i, input);
                }
                TranslateMatrix8003B0CC(s, matrix, parameters + 24u);
            }
            // No translation bit: preserve the matrix's previous translation.
        }
    }
    return Signed32(next); // Captured header length, not the consumed payload.
}

int32_t ApplyAnimationBlock80028504(Services& s, uint32_t frame, uint32_t cursor,
    uint32_t descriptor, uint32_t mode) {
    const uint32_t trigger = s.Read32(cursor + 4u);
    const uint32_t commands = s.Read16(cursor + 2u);
    if (frame < trigger) return Signed32(cursor);
    cursor += 8u;
    for (uint32_t i = 0; i < commands; ++i)
        cursor = U(ApplyTodCommand80028054(s, cursor, descriptor));
    (void)mode; // Original 28054 never reads its third incoming register.
    // One block only, including an empty block. Do not catch up to host FPS.
    return Signed32(cursor);
}

namespace {
template<class ReadCount, class WriteCount>
int32_t AdvanceAnimationCount8001B000(Services& s, uint32_t frame,
    uint32_t cursorAddress, uint32_t descriptor, ReadCount readCount, WriteCount writeCount) {
    const uint32_t previous = s.Read32(cursorAddress);
    if (previous == 0u || Signed32(readCount()) <= 0) return -1;
    const uint32_t next = U(ApplyAnimationBlock80028504(s, frame, previous, descriptor, 0u));
    s.Write32(cursorAddress, next);
    if (previous != next) writeCount(readCount() - 1u);
    return Signed32(readCount());
}
}

int32_t AdvanceAnimation8001B000(Services& s, uint32_t frame,
    uint32_t countAddress, uint32_t cursorAddress, uint32_t descriptor) {
    return AdvanceAnimationCount8001B000(s, frame, cursorAddress, descriptor,
        [&]() { return s.Read32(countAddress); },
        [&](uint32_t count) { s.Write32(countAddress, count); });
}

int32_t AdvanceAnimationLocalCount8001B000(Services& s, uint32_t frame,
    uint32_t& count, uint32_t cursorAddress, uint32_t descriptor) {
    return AdvanceAnimationCount8001B000(s, frame, cursorAddress, descriptor,
        [&]() { return count; }, [&](uint32_t value) { count = value; });
}

int32_t ResetHeapPointers80025A00(Services& s) {
    s.Write32(kHeapBase, 0x800965B0u);
    s.Write32(kHeapCursor, 0x800965B0u);
    s.Write32(kHeapEnd, 0x801C35B0u);
    s.Write32(kHeapDepth, 0u);
    s.Write32(kResourceTable, 0u);
    // The original does not reset GP+320 (the packet reservation watermark).
    return Signed32(0x801C35B0u);
}

int32_t ResetResourceHeap80025A34(Services& s) {
    for (uint32_t remaining = 1024u; remaining != 0u; --remaining)
        s.Write32(kResourceTable + 4u * (remaining - 1u), 0u);
    return ResetHeapPointers80025A00(s);
}

namespace {
uint32_t AlignHeapRequest80025B28(uint32_t bytes) {
    const uint32_t adjusted = bytes + 7u;
    return Sra(Signed32(adjusted) < 0 ? bytes + 14u : adjusted, 3u) << 3u;
}
}

int32_t PushResourceHeap80025A70(Services& s, uint32_t bytes) {
    const uint32_t depth = s.Read32(kHeapDepth);
    if (Signed32(depth) >= 1024 || bytes - 1u > 0x12CFFEu) return 0;
    const uint32_t aligned = AlignHeapRequest80025B28(bytes);
    const uint32_t cursor = s.Read32(kHeapCursor);
    const uint32_t end = s.Read32(kHeapEnd);
    const uint32_t next = cursor + aligned;
    if (next >= end) return 0;
    s.Write32(kHeapDepth, depth + 1u);
    s.Write32(kResourceTable + (depth + 1u) * 4u, cursor);
    s.Write32(kHeapCursor, next);
    return Signed32(cursor);
}

int32_t PopResourceHeap80025AF8(Services& s) {
    const uint32_t depth = s.Read32(kHeapDepth);
    const uint32_t address = kResourceTable + depth * 4u;
    const uint32_t cursor = s.Read32(address);
    s.Write32(address, 0u);
    s.Write32(kHeapDepth, depth - 1u);
    s.Write32(kHeapCursor, cursor);
    // No zero-depth gate; the result is the table-entry address, not cursor.
    return Signed32(address);
}

int32_t AllocatePacketHeap80025B28(Services& s, uint32_t bytes) {
    const uint32_t aligned = AlignHeapRequest80025B28(bytes);
    const uint32_t cursor = s.Read32(kHeapCursor);
    const uint32_t end = s.Read32(kHeapEnd);
    const uint32_t result = end - aligned;
    if (cursor + aligned >= end) return 0;
    const uint32_t oldLowWater = s.Read32(kHeapLowWater);
    s.Write32(kHeapLowWater, oldLowWater < result ? result : oldLowWater);
    return Signed32(result);
}

void BindDrawBuffers8001E33C(Services& s, uint32_t first, uint32_t second) {
    s.Write32(0x8006ED50u, first);
    s.Write32(0x8006ED54u, second);
}

int32_t FillDrawFlags8001EEAC(Services& s, uint32_t value) {
    for (uint32_t row = 0; row < 12u; ++row)
        for (uint32_t remaining = 16u; remaining != 0u; --remaining)
            s.Write32(0x80087330u + row * 64u + (remaining - 1u) * 4u, value);
    return 0;
}

int32_t ResetDrawFlags8001EEE8(Services& s) {
    s.Write32(0x8006EB0Cu, 0u);
    s.Write32(0x8006EB04u, 0u);
    s.Write32(0x8006EB08u, 0u);
    return FillDrawFlags8001EEAC(s, 1u);
}

int32_t ClearOrderingTable80040CC8(Services& s, uint32_t offset, uint32_t point, uint32_t table) {
    const uint32_t depth = s.Read32(table);
    s.Write32(table + 8u, offset & 0xFFFFu);
    const uint32_t entries = s.Read32(table + 4u);
    s.Write32(table + 12u, point & 0xFFFFu);
    const uint32_t countDepth = s.Read32(table);
    s.Write32(table + 16u, entries + (4u << (depth & 31u)) - 4u);
    const uint32_t currentEntries = s.Read32(table + 4u);
    return PrStage2GpuDirect::ClearOrderingTable80044FA8(s,currentEntries,1u<<(countDepth&31u));
}

int32_t SubmitOrderingTable80040CA4(Services& s, uint32_t table) {
    return PrStage2GpuDirect::DrawOrderingTable800450A0(s,s.Read32(table + 16u));
}

int32_t ClearWorkOrderingTable8001E374(Services& s, uint32_t buffer) {
    return ClearOrderingTable80040CC8(s, 0u, 0u, 0x80087288u + 20u * buffer);
}

int32_t SubmitWorkOrderingTable8001E3B0(Services& s, uint32_t buffer) {
    return SubmitOrderingTable80040CA4(s, 0x80087288u + 20u * buffer);
}

int32_t LinkPrimitive8004401C(Services& s, uint32_t tag, uint32_t primitive) {
    const uint32_t packetTag = s.Read32(primitive);
    const uint32_t previous = s.Read32(tag);
    s.Write32(primitive, (packetTag & 0xFF000000u) | (previous & 0x00FFFFFFu));
    const uint32_t result = (s.Read32(tag) & 0xFF000000u) | (primitive & 0x00FFFFFFu);
    s.Write32(tag, result);
    return Signed32(result);
}

int32_t EnqueueClearPrimitive80040060(Services& s, uint32_t red, uint32_t green,
    uint32_t blue, uint32_t table) {
    const auto index = [&]() { return Extend16(s.Read16(0x80096590u)); };
    s.Write8(0x8008A734u + 16u * index(), static_cast<uint8_t>(red));
    s.Write8(0x8008A735u + 16u * index(), static_cast<uint8_t>(green));
    s.Write8(0x8008A736u + 16u * index(), static_cast<uint8_t>(blue));
    const uint32_t selected = index();
    const uint32_t packet = 0x8008A730u + 16u * selected;
    const uint16_t x = s.Read16(0x8008ECA8u + 2u * selected);
    const uint16_t height = s.Read16(0x8009182Cu);
    s.Write16(packet + 8u, x);
    const uint16_t y = s.Read16(0x8008ECACu + 2u * selected);
    s.Write16(packet + 14u, height);
    s.Write16(packet + 10u, y);
    if (s.Read8(0x800917A1u) != 0u) {
        const uint32_t width = s.Read32(0x800917FCu) * 3u;
        s.Write16(packet + 12u, static_cast<uint16_t>(Signed32(width) / 2));
    } else s.Write16(packet + 12u, s.Read16(0x800917FCu));
    const uint32_t finalPacket = 0x8008A730u + 16u * index();
    const uint32_t tag = s.Read32(table + 16u);
    return LinkPrimitive8004401C(s, tag, finalPacket);
}

int32_t InitGlobals801C97EC(Services& s) {
    // These are LW reads, not addresses or immutable IDA-folded counts.
    // Original executes all three before publishing any shared slot.
    const uint32_t d4 = s.Read32(0x801CE168u);
    const uint32_t dc = s.Read32(0x801D1A44u);
    const uint32_t e8 = s.Read32(0x801D2178u);
    s.Write32(0x800943C0u, 0x801D217Cu);
    s.Write32(0x800943C4u, 0x801D1B18u);
    s.Write32(0x800943C8u, 53u);
    s.Write32(0x800943CCu, 0x801CBF0Cu);
    s.Write32(0x800943D0u, 0x801CDAE0u);
    s.Write32(0x800943D8u, 0x801D1870u);
    s.Write32(0x800943E0u, 0x801D1A48u);
    s.Write32(0x800943E4u, 0x801D2010u);
    s.Write32(0x800943FCu, 0x801D11B4u);
    s.Write32(0x80094400u, 0x801D11A8u);
    s.Write32(0x80094414u, 0x801D11E0u);
    s.Write32(0x80094418u, 0x801D11E8u);
    s.Write32(0x8009441Cu, 0x801D11F0u);
    s.Write32(0x80094420u, 0x801D11FCu);
    s.Write32(0x80094424u, 0x801D1204u);
    s.Write32(0x80094428u, 0x801D120Cu);
    s.Write32(0x8009442Cu, 0x801D1214u);
    s.Write32(0x80094404u, 0x801D11C0u);
    s.Write32(0x80094408u, 0x801D11C8u);
    s.Write32(0x8009440Cu, 0x801D11D0u);
    s.Write32(0x80094410u, 0x801D11D8u);
    s.Write32(0x800943ECu, 0x801D1188u);
    s.Write32(0x800943F0u, 0x801D1190u);
    s.Write32(0x800943F4u, 0x801D1198u);
    s.Write32(0x800943F8u, 0x801D11A0u);
    s.Write32(0x80094430u, 0x801C85CCu);
    s.Write32(0x80094434u, 0x801C9644u);
    s.Write32(0x800943D4u, d4);
    s.Write32(0x800943DCu, dc);
    s.Write32(0x800943E8u, e8);
    s.Write32(0x80094438u, 0x801C9728u); // S2 nullsub, not S1 accepted tail
    s.Write32(0x8009443Cu, 0x801C9730u);
    s.Write32(0x80094440u, 0x801C7958u);
    return 2;
}

int32_t ClearEntries801C78D4(Services& s) {
    int32_t offset = 96;
    for (; offset >= 0; offset -= 12)
        s.Write32(0x801D1124u + U(offset), 0u);
    return offset;
}

int32_t ConfigureText801CB244(Services& s) {
    s.Call(0x8001BC48u, {28u, 189u, 272u, 480u});
    return s.Call(0x8001BC78u, {832u, 256u, 20u, 21u});
}

int32_t ResetEventTimeline801C7884(Services& s, uint32_t work, uint32_t eventId) {
    if (eventId - 1u < 8u) s.Write32(0x801D1124u + eventId * 12u, 0u);
    const uint32_t tick = s.Read32(work + 12u);
    s.Write32(0x801D2990u, tick);
    return Signed32(tick);
}

int32_t PublishEventText801C78FC(Services& s, uint32_t work, uint32_t event) {
    const uint32_t id = s.Read8(event + 55u);
    if (id != 0u) {
        const uint32_t language = U(Signed16(s.Read16(work + 102u)));
        const uint32_t table = s.Read32(0x801CC4A0u + 4u * language);
        const uint32_t text = s.Read32(table + 4u * id);
        s.Write16(0x8008ECFAu, 120u);
        s.Write32(0x8008ECE4u, text);
    }
    const uint32_t text = s.Read32(0x8008ECE4u);
    s.Write32(work + 268u, text);
    return Signed32(text);
}

int32_t AdvanceResourceQueue801C7A24(Services& s, uint32_t work, uint32_t lane) {
    const uint32_t offset = lane * 12u;
    uint32_t result = s.Read32(0x8008ED44u + offset);
    uint32_t selected = 0u;
    if (result != 0u) {
        const uint32_t queue = s.Read32(0x8008ED4Cu + offset);
        if (queue != 0u) {
            const uint32_t index = s.Read32(queue + 8u);
            const uint32_t count = s.Read32(queue + 4u);
            result = index * 2u; // Native branch delay slot, including exhaustion.
            if (Signed32(index) >= Signed32(count)) {
                s.Write32(queue + 8u, 0u);
                s.Write32(0x8008ED4Cu + offset, 0u);
                s.Write32(0x8008ED44u + offset, 0u);
            } else {
                const uint32_t record = s.Read32(queue) + 12u * index;
                const uint32_t baseline = s.Read32(0x8008ED48u + offset);
                const uint32_t time = s.Read32(record);
                result = Signed32(s.Read32(work + 12u)) < Signed32(baseline + time) ? 1u : 0u;
                if (result == 0u) {
                    result = index + 1u;
                    s.Write32(queue + 8u, result);
                    selected = record;
                }
            }
        }
    }
    if (selected != 0u) {
        s.Write32(work, s.Read32(work) | 0x8000u);
        result = work + lane * 4u;
        s.Write32(result + 172u, selected + 4u);
    }
    return Signed32(result);
}

namespace {
uint32_t Resource(Services& s, uint32_t index) {
    return s.Read32(0x80091858u + 4u * index);
}
// Shared source sequence in B20/BC0/7C54. Both signed indices are read before
// checking either zero. Publishing the first resource can affect the second.
bool PublishResourcePair(Services& s, uint32_t work, uint32_t offset,
                         uint32_t flag, uint32_t field, uint32_t& result) {
    const int32_t first = Signed16(s.Read16(0x801CC4B4u + offset));
    const int32_t second = Signed16(s.Read16(0x801CC4B6u + offset));
    if (first == 0 || second == 0) return false;
    s.Write32(work, s.Read32(work) | flag);
    s.Write32(work + field, Resource(s, U(first)));
    result = Resource(s, U(second));
    s.Write32(work + field + 12u, result);
    return true;
}
}

int32_t RestoreSecondResourcePair801C7B20(Services& s, uint32_t work) {
    uint32_t result = s.Read32(0x801CC560u) * 4u;
    if (PublishResourcePair(s, work, result, 0x20000u, 224u, result))
        s.Write16(0x8008ECFEu, 33u);
    return Signed32(result);
}

int32_t RestoreFirstResourcePair801C7BC0(Services& s, uint32_t work) {
    uint32_t result = s.Read32(0x801CC55Cu) * 4u;
    PublishResourcePair(s, work, result, 0x10000u, 220u, result);
    return Signed32(result);
}

int32_t DispatchEventResources801C7C54(Services& s, uint32_t work, uint32_t event) {
    const auto flags = [&]() { return s.Read32(event + 4u); };
    const auto grade = [&]() { return Signed16(s.Read16(work + 78u)); };
    const auto add = [&](uint32_t mask) { s.Write32(work, s.Read32(work) | mask); };
    const auto publish = [&](uint32_t index, uint32_t mask, uint32_t field) {
        add(mask);
        s.Write32(work + field, Resource(s, index));
    };
    if ((flags() & 0x20u) != 0u) {
        const uint32_t old = s.Read32(work);
        s.Write32(0x801D299Cu, 0u);
        s.Write32(work, old | 0x20u);
    }
    if ((flags() & 0x40000u) != 0u) {
        int32_t index = 0;
        if (s.Read16(work + 78u) < 2u) index = Signed16(s.Read16(event + 36u));
        if (index <= 0) {
            const int32_t g = grade();
            if (g == 2) index = Signed16(s.Read16(event + 38u));
            else if (g == 3) index = Signed16(s.Read16(event + 40u));
        }
        if (index > 0) publish(U(index), 0x40000u, 244u);
    }
    if (s.Read16(0x8008ED2Cu) != 0u) s.Write16(0x8008ECFCu, 0u);
    if (Signed16(s.Read16(0x8008ECFCu)) <= 0 && (flags() & 0x10000u) != 0u) {
        uint32_t id = grade() == 0 ? s.Read8(event + 48u) : 0u;
        if (id == 0u) id = s.Read8(event + 49u);
        uint32_t ignored = 0u;
        if (id != 0u) PublishResourcePair(s, work, id * 4u, 0x10000u, 220u, ignored);
    }
    if ((flags() & 0x80000u) != 0u) {
        int32_t index = 0;
        if (grade() == 0) index = Signed16(s.Read16(event + 28u));
        if (index <= 0) {
            const int32_t g = grade();
            if (g == 1) index = Signed16(s.Read16(event + 30u));
            else if (g == 2) index = Signed16(s.Read16(event + 32u));
            else if (grade() == 3) index = Signed16(s.Read16(event + 34u));
        }
        if (index > 0) publish(U(index), 0x80000u, 248u);
    }
    if ((flags() & 0x20000u) != 0u) {
        uint32_t id = grade() == 0 ? s.Read8(event + 46u) : 0u;
        if (id == 0u) id = s.Read8(event + 47u);
        uint32_t ignored = 0u;
        if (id != 0u && PublishResourcePair(s, work, id * 4u, 0x20000u, 224u, ignored))
            s.Write16(0x8008ECFEu, 132u);
    }
    uint32_t camera = 0u;
    if (grade() == 0) camera = s.Read8(event + 14u);
    if (camera == 0u) {
        const int32_t g = grade();
        if (g == 2 || g == 3) camera = s.Read8(event + 16u);
        if (camera == 0u && grade() == 1) camera = s.Read8(event + 15u);
    }
    if (camera != 0u) publish(camera, 0x400u, 260u);
    if ((flags() & 0x400000u) != 0u) add(0x400000u);
    if ((flags() & 0x10000000u) != 0u) {
        // Each byte is reloaded after the preceding write; event/work may alias.
        const auto pose = [&](uint32_t first, uint32_t last) {
            add(0x10000000u);
            s.Write8(work + 374u, s.Read8(event + first));
            s.Write8(work + 375u, s.Read8(event + first + 1u));
            s.Write8(work + 376u, s.Read8(event + last));
        };
        if (grade() == 0 && s.Read8(event + 17u) != 0u) pose(18u, 17u);
        else if (grade() == 1 && s.Read8(event + 18u) != 0u) pose(18u, 20u);
        else if (grade() == 2 && s.Read8(event + 21u) != 0u) pose(21u, 23u);
        else if (grade() == 3 && s.Read8(event + 24u) != 0u) pose(24u, 26u);
    }
    if ((flags() & 0x800000u) != 0u) {
        const uint32_t old = s.Read32(work);
        const int32_t g = grade();
        s.Write32(work, old | 0x800000u);
        uint32_t choice = 1u;
        if (g != 0) {
            const uint32_t phase = s.Read32(work + 56u);
            if (phase == 22u) choice = ((s.Read32(0x8009180Cu) >> 2u) ^ 1u) & 1u;
            else if (phase == 26u) choice = ((s.Read32(0x8009180Cu) >> 3u) ^ 1u) & 1u;
        }
        if (choice == 1u) {
            if (grade() == 0) {
                add(0x10000000u);
                s.Write8(work + 374u, s.Read8(event + 18u));
                s.Write8(work + 375u, s.Read8(event + 19u));
            }
        } else if (grade() != 0) {
            add(0x400u);
            s.Write32(work + 260u, s.Read32(0x80091BA8u));
            add(0x40000u);
            s.Write32(work + 244u, s.Read32(0x800918F4u));
            add(0x80000u);
            const uint32_t model = s.Read32(0x800918BCu);
            s.Write8(work + 374u, 10u);
            s.Write8(work + 375u, 10u);
            const uint32_t saved = s.Read32(work);
            s.Write32(work + 248u, model);
            s.Write32(work, saved | 0x10000000u);
        }
    }
    if ((flags() & 0x1000000u) != 0u) {
        const uint32_t old = s.Read32(work);
        const int32_t g = grade();
        const uint32_t updated = old | 0x1000000u;
        s.Write32(work, updated);
        if (g == 0) {
            s.Write32(work, updated | 0x10000000u);
            s.Write8(work + 374u, s.Read8(event + 18u));
            s.Write8(work + 375u, s.Read8(event + 19u));
        }
    }
    if ((flags() & 0x8000000u) == 0u) {
        s.Write8(work + 372u, 0u);
        s.Write8(work + 373u, 0u);
    } else if (Signed32(s.Read32(0x801D299Cu)) <= 0) {
        const uint32_t old = s.Read32(work);
        const int32_t g = grade();
        s.Write32(work, old | 0x8000000u);
        const uint32_t offset = g == 2 ? 10u : (g == 3 ? 12u : 8u);
        s.Write8(work + 372u, s.Read8(event + offset));
        s.Write8(work + 373u, s.Read8(event + offset + 1u));
    }
    if (grade() != 0) {
        s.Write32(work + 252u, U(Signed16(s.Read16(event + 42u))));
        s.Write32(work + 256u, U(Signed16(s.Read16(event + 44u))));
    }
    const uint32_t firstQueue = s.Read8(event + 51u);
    if (firstQueue != 0u) {
        const uint32_t tick = s.Read32(work + 12u);
        const uint32_t queue = 0x801CD980u + firstQueue * 12u;
        s.Write32(0x8008ED44u, firstQueue);
        s.Write32(0x8008ED4Cu, queue);
        s.Write32(0x8008ED48u, tick);
        s.Write32(queue + 8u, 0u);
    }
    const uint32_t secondQueue = s.Read8(event + 50u);
    uint32_t result = secondQueue * 2u;
    if (secondQueue != 0u) {
        const uint32_t tick = s.Read32(work + 12u);
        result = 0x801CD980u + secondQueue * 12u;
        s.Write32(0x8008ED50u, secondQueue);
        s.Write32(0x8008ED58u, result);
        s.Write32(0x8008ED54u, tick);
        s.Write32(result + 8u, 0u);
    }
    return Signed32(result);
}

namespace {
// The four copies inside 870C differ only in the sequence field offsets and
// reset callee. In particular the mode-2 second half can run in the same tick.
void AdvanceEventSequence(Services& s, uint32_t work, bool second) {
    const uint32_t firstIndex = second ? 158u : 140u;
    const uint32_t nextIndex = second ? 160u : 142u;
    const uint32_t stateField = second ? 162u : 144u;
    const uint32_t firstPointer = second ? 164u : 148u;
    const uint32_t nextPointer = second ? 168u : 152u;
    const uint32_t reset = second ? 0x80024390u : 0x80024308u;
    const int32_t state = Signed16(s.Read16(work + stateField));
    if (state == 0 || (s.Read32(work) & 8u) == 0u) return;
    const int32_t mode = Signed16(s.Read16(work + 138u));
    const auto advance = [&](uint32_t indexField, uint32_t pointerField) {
        const int32_t index = Signed16(s.Read16(work + indexField));
        if (index < 19 && s.Read8(s.Read32(work + pointerField) + U(index)) != 0xFFu) {
            s.Write16(work + indexField, static_cast<uint16_t>(index + 1));
            return true;
        }
        return false;
    };
    if (mode == 1) {
        if (state == 1 && !advance(firstIndex, firstPointer)) {
            s.Write16(work + stateField, 0u);
            s.Write16(work + firstIndex, 0xFFFFu);
            s.Call(reset, {});
        }
    } else if (mode == 2) {
        if (state == 1 && !advance(firstIndex, firstPointer)) {
            s.Write16(work + firstIndex, 0xFFFFu);
            s.Write16(work + nextIndex, 2u);
            s.Write16(work + stateField, 2u);
        }
        if (s.Read16(work + stateField) == 2u && !advance(nextIndex, nextPointer)) {
            // Two SH instructions, not the merged unaligned int store in pseudo-C.
            s.Write16(work + nextIndex, 0xFFFFu);
            s.Write16(work + stateField, 0u);
            s.Call(reset, {});
        }
    }
}

void ClearEventSequences(Services& s, uint32_t work) {
    s.Write16(work + 142u, 0xFFFFu);
    s.Write16(work + 140u, 0xFFFFu);
    s.Write32(work + 148u, 0x801CDAE4u);
    s.Write32(work + 152u, 0x801CDAF8u);
    s.Write16(work + 138u, 0u);
    s.Write16(work + 144u, 0u);
    s.Write16(work + 160u, 0xFFFFu);
    s.Write16(work + 158u, 0xFFFFu);
    s.Write32(work + 164u, 0x801CDAE4u);
    s.Write32(work + 168u, 0x801CDAF8u);
    s.Write16(work + 138u, 0u); // Repeated native write is intentional.
    s.Write16(work + 162u, 0u);
    s.Call(0x80024390u, {});
    s.Call(0x80024308u, {});
}

uint32_t EventQueue(uint32_t id) {
    return id - 1u < 8u ? 0x801D111Cu + 12u * id : 0u;
}
}

int32_t UpdateEvents801C870C(Services& s, uint32_t work) {
    const auto expired = [&](uint32_t address) {
        const int32_t count = Signed16(s.Read16(address));
        if (count <= 0) return false;
        s.Write16(address, static_cast<uint16_t>(count - 1));
        return count == 1;
    };
    if (expired(0x8008ECF8u)) {
        s.Write32(0x8008ECE0u, 0u);
        s.Write32(work + 264u, 0u);
    }
    if (expired(0x8008ECFAu)) {
        s.Write32(0x8008ECE4u, 0u);
        s.Write32(work + 268u, 0u);
    }
    if (expired(0x8008ECFCu) && s.Read16(0x8008ED2Eu) != 0u) {
        RestoreFirstResourcePair801C7BC0(s, work);
        const uint32_t id = s.Read32(0x801CDADCu);
        if (id != 0u) {
            const uint32_t tick = s.Read32(work + 12u);
            const uint32_t queue = 0x801CD980u + 12u * id;
            s.Write32(0x8008ED44u, id);
            s.Write32(0x8008ED4Cu, queue);
            s.Write32(0x8008ED48u, tick);
            s.Write32(queue + 8u, 0u);
        }
    }
    if (s.Read32(0x8008ED00u) == 5u && expired(0x8008ECFEu)) {
        RestoreSecondResourcePair801C7B20(s, work);
        s.Write16(0x8008ECFEu, 33u); // Native writes again even when B20 wrote 33.
    }
    AdvanceEventSequence(s, work, false);
    AdvanceEventSequence(s, work, true);
    AdvanceResourceQueue801C7A24(s, work, 0u);
    AdvanceResourceQueue801C7A24(s, work, 1u);
    if (expired(work + 380u)) s.Write8(work + 377u, 0u);
    if (expired(work + 382u)) s.Write8(work + 378u, 0u);
    const uint32_t cooldown = s.Read32(0x801D299Cu);
    if (Signed32(cooldown) > 0) s.Write32(0x801D299Cu, cooldown - 1u);

    const uint32_t flags = s.Read32(work);
    if ((flags & 0x40u) != 0u) {
        s.Write16(0x8008ED2Eu, 0u);
        s.Write16(0x8008ED2Cu, s.Read16(work + 78u) == 0u ? 5u : 4u);
        ResetEventTimeline801C7884(s, work, U(Signed16(s.Read16(0x8008ED2Cu))));
        s.Write32(0x8008ED20u, 1u);
        s.Write16(0x8008ECFCu, 0u);
        ClearEventSequences(s, work);
        return Signed32(work); // No event from either timeline this tick.
    }
    bool clearSequences = false;
    if (s.Read32(0x8008ED1Cu) == 1u) {
        s.Write32(0x8008ED1Cu, 0u);
        s.Write16(0x8008ED2Eu, 0u);
        const int32_t grade = Signed16(s.Read16(work + 78u));
        s.Write16(work + 120u, 1u);
        if (grade == 0) s.Write16(0x8008ED2Cu, 3u);
        else if (grade == 1) s.Write16(0x8008ED2Cu, 2u);
        else {
            s.Write16(0x8008ED2Cu, 4u);
            const uint32_t old = s.Read32(work);
            s.Write16(work + 118u, 1u);
            s.Write32(work, old | 0x40u);
        }
        clearSequences = true;
    } else if ((flags & 0x2000u) != 0u) {
        s.Write16(0x8008ED2Cu, Signed16(s.Read16(work + 114u)) < 2 ? 6u : 8u);
        clearSequences = true;
    } else if ((flags & 0x4000u) != 0u) {
        s.Write16(0x8008ED2Cu, 7u);
        ResetEventTimeline801C7884(s, work, U(Signed16(s.Read16(0x8008ED2Cu))));
    }
    if (clearSequences) {
        ResetEventTimeline801C7884(s, work, U(Signed16(s.Read16(0x8008ED2Cu))));
        ClearEventSequences(s, work);
    }
    const uint32_t currentFlags = s.Read32(work);
    if ((currentFlags & 0x200u) != 0u && s.Read16(work + 398u) == 4u) {
        s.Write32(work, currentFlags | 0x10000000u);
        s.Write8(work + 374u, 5u);
        s.Write8(work + 375u, 5u);
        s.Write8(work + 376u, 5u);
    }

    uint32_t absoluteEvent = 0u;
    const uint32_t absoluteId = U(Signed16(s.Read16(0x8008ED2Eu)));
    if (absoluteId != 0u) {
        const uint32_t tick = s.Read32(work + 12u);
        const uint32_t queue = EventQueue(absoluteId);
        if (queue != 0u) {
            const uint32_t index = s.Read32(queue + 8u);
            const uint32_t count = s.Read32(queue + 4u);
            if (Signed32(index) < Signed32(count)) {
                const uint32_t event = s.Read32(queue) + index * 56u;
                if (Signed32(tick) >= Signed32(s.Read32(event))) {
                    s.Write32(queue + 8u, index + 1u);
                    absoluteEvent = event;
                }
            }
        }
        if (s.Read16(0x8008ED2Eu) != 0u && absoluteEvent != 0u) {
            const auto eventFlags = [&]() { return s.Read32(absoluteEvent + 4u); };
            if ((eventFlags() & 0x2000000u) != 0u) s.Write32(work, s.Read32(work) | 0x2000000u);
            if ((eventFlags() & 0x4000000u) != 0u) s.Write32(work, s.Read32(work) | 0x4000000u);
            const uint32_t sequenceFlags = eventFlags();
            if ((sequenceFlags & 0x800u) != 0u) {
                s.Write32(work, s.Read32(work) | 0x800u);
                const int32_t mode = Signed16(s.Read16(0x801CDAE0u + 44u * s.Read8(absoluteEvent + 52u)));
                if (mode > 0) {
                    s.Write16(work + 138u, static_cast<uint16_t>(mode));
                    const uint16_t start = s.Read16(0x801CDAE2u + 44u * s.Read8(absoluteEvent + 52u));
                    s.Write16(work + 142u, 0xFFFFu);
                    s.Write16(work + 144u, 1u);
                    s.Write16(work + 140u, start);
                } else {
                    s.Write16(work + 138u, static_cast<uint16_t>(-mode));
                }
                s.Write32(work + 148u, 0x801CDAE4u + 44u * s.Read8(absoluteEvent + 52u));
                const uint32_t secondPointer = 0x801CDAF8u + 44u * s.Read8(absoluteEvent + 53u);
                const uint32_t firstPointer = s.Read32(work + 148u);
                s.Write32(work + 152u, secondPointer);
                s.Write32(0x801D2994u, firstPointer);
                s.Write32(0x801D2998u, s.Read32(work + 152u));
                if (mode > 0) s.Write16(work + 144u, 1u);
                else {
                    s.Write16(work + 140u, 0xFFFFu);
                    s.Write16(work + 142u, 0xFFFFu);
                    s.Write16(work + 144u, 0u);
                }
                if (s.Read8(s.Read32(work + 148u)) == 0xFEu) {
                    s.Write16(work + 158u, 0xFFFFu);
                    s.Write16(work + 160u, 0xFFFFu);
                    s.Write16(work + 162u, 0u);
                }
                if (s.Read16(work + 138u) == 0u) {
                    s.Write16(work + 140u, 0xFFFFu);
                    s.Write16(work + 142u, 0xFFFFu);
                    s.Write16(work + 144u, 0u);
                    s.Write16(work + 158u, 0xFFFFu);
                    s.Write16(work + 160u, 0xFFFFu);
                    s.Write16(work + 162u, 0u);
                }
                AdvanceEventSequence(s, work, false);
            } else if ((sequenceFlags & 0x1000u) != 0u || (s.Read32(work) & 0x2000u) != 0u) {
                ClearEventSequences(s, work);
            }
            if ((eventFlags() & 0x100000u) != 0u) {
                s.Write16(work + 162u, 1u);
                const uint32_t old = s.Read32(work);
                s.Write16(work + 158u, 0u);
                s.Write16(work + 160u, 0xFFFFu);
                s.Write32(work, old | 0x100000u);
                s.Write32(work + 164u, s.Read32(0x801D2994u));
                s.Write32(work + 168u, s.Read32(0x801D2998u));
                if (Signed16(s.Read16(0x801CDAE0u + 44u * s.Read8(absoluteEvent + 52u))) < 0)
                    s.Call(0x80024390u, {});
                AdvanceEventSequence(s, work, true);
            }
            if ((eventFlags() & 0x80u) != 0u) {
                s.Write32(0x8008ED1Cu, 1u);
                s.Write16(work + 84u, 1u);
                s.Call(0x800169E0u, {work});
            }
        }
    }
    if (s.Read16(0x8008ED2Cu) != 0u) {
        const uint32_t id = U(Signed16(s.Read16(0x8008ED2Cu)));
        const uint32_t tick = s.Read32(work + 12u);
        const uint32_t queue = EventQueue(id);
        uint32_t selected = 0u;
        if (queue != 0u) {
            const uint32_t index = s.Read32(queue + 8u);
            const uint32_t count = s.Read32(queue + 4u);
            if (Signed32(index) < Signed32(count)) {
                const uint32_t event = s.Read32(queue) + 56u * index;
                const uint32_t baseline = s.Read32(0x801D2990u);
                if (Signed32(tick) >= Signed32(baseline + s.Read32(event))) {
                    s.Write32(queue + 8u, index + 1u);
                    selected = event;
                }
            }
        }
        if (selected == 0u) {
            const uint32_t currentQueue = EventQueue(U(Signed16(s.Read16(0x8008ED2Cu))));
            bool exhausted = false;
            if (currentQueue != 0u) {
                const uint32_t index = s.Read32(currentQueue + 8u);
                exhausted = Signed32(index) >= Signed32(s.Read32(currentQueue + 4u));
            }
            if (exhausted) {
                const uint32_t currentId = s.Read16(0x8008ED2Cu);
                if (currentId - 4u < 2u || currentId - 2u < 2u)
                    s.Write32(work, s.Read32(work) | 0x100u);
                else if (currentId == 6u || currentId == 8u) s.Write32(0x8008ED28u, 1u);
                const uint32_t finalQueue = EventQueue(U(Signed16(s.Read16(0x8008ED2Cu))));
                if (finalQueue != 0u) s.Write32(finalQueue + 8u, 0u);
                s.Write16(0x8008ED2Cu, 0u);
            }
        }
        if (selected != 0u) {
            DispatchEventResources801C7C54(s, work, selected);
            PublishEventText801C78FC(s, work, selected);
        }
    } else if (s.Read16(0x8008ED2Eu) != 0u && absoluteEvent != 0u) {
        DispatchEventResources801C7C54(s, work, absoluteEvent);
        if (s.Read16(work + 78u) != 0u) PublishEventText801C78FC(s, work, absoluteEvent);
    }
    return Signed32(work);
}

int32_t InitScene801C657C(Services& s, uint32_t entry, int32_t scene) {
    ResetResourceHeap80025A34(s);
    ClearEntries801C78D4(s);
    s.Call(0x80024E98u, {});
    s.Call(0x80014344u, {});
    const int32_t rate96 = 96 * Signed16(s.Read16(entry + 6u));
    const int32_t sum = Signed16(s.Read16(entry + 8u)) +
                        Signed16(s.Read16(entry + 10u));
    s.Write32(kSceneEntry, entry);
    s.Write32(entry + 360u, 16u);
    s.Write32(entry + 348u, U(rate96 / 100));
    s.Write32(entry + 356u, U(sum));
    s.Write32(entry + 352u, U((rate96 / 3600 + 50) / 100));
    for (uint32_t offset = 12u; offset < 12u + 7u * 48u; offset += 48u)
        PrStage2IntLoaderDirect::OpenFile8001A324(s, s.Read32(kSceneEntry) + offset);
    ResetHeapPointers80025A00(s);
    const uint32_t mode = U(Signed16(s.Read16(0x801CBA34u + U(scene) * 2u)));
    s.Call(0x8001EF14u, {});
    return s.Call(0x80015660u, {U(scene), mode, 1u});
}

int32_t InitMovie801C6A3C(Services& s, uint32_t record, int32_t mode) {
    ResetAudio80026FA4(s);
    ConfigureText801CB244(s);
    s.Call(0x80024E98u, {});
    const int32_t segment = Signed16(s.Read16(record + 6u));
    s.Write32(0x801D298Cu, mode != 1 ? 1u : 0u);
    s.CallVoid(0x8001A478u, {U(segment)});
    s.Call(0x80027288u, {U(mode)});
    s.Call(0x8001A4D0u, {record, 1u});
    return s.Call(0x800274D4u, {});
}

int32_t UpdateMovieClock801C66C8(Services& s, uint32_t record, uint32_t work) {
    const int32_t position = s.Call(0x8001A7A4u, {s.Read32(record + 40u)});
    if (position < 0) return 1;
    // The original LW at 801C66F8 is mutable. Hex-Rays folds it to zero.
    // Keep SUBU wrapping followed by signed BLEZ/SLTI (not abs(position)).
    const uint32_t previous = s.Read32(0x801D2984u);
    const int32_t forward = Signed32(U(position) - previous);
    if (forward > 0) {
        if (forward >= 301) return 1;
    } else if (Signed32(previous - U(position)) >= 301) {
        return 1;
    }
    const int32_t ticks = position / 5;
    s.Write32(0x801D2984u, U(position));
    s.Write16(work + 4u, static_cast<uint16_t>(ticks / 1800));
    s.Write8(work + 7u, static_cast<uint8_t>(ticks % 30));
    s.Write8(work + 6u, static_cast<uint8_t>(ticks % 1800 / 30));
    return s.Call(0x8001A7F8u, {record}) != 1 ? 1 : 0;
}

int32_t MovieFrame801C6804(Services& s, uint32_t record, uint32_t work) {
    s.Call(0x80035560u, {2u});
    s.Call(0x8001A3C8u, {});
    s.Call(0x8001A280u, {});
    return UpdateMovieClock801C66C8(s, record, work);
}

int32_t AllocatePackets801C9A00(Services& s) {
    const uint32_t base = U(AllocatePacketHeap80025B28(s, 86400u));
    if (base == 0u) s.Exit(0x80047F3Cu, {1u, 1u});
    s.Write32(0x801D29A4u, base + 43200u);
    s.Write32(0x801D29A0u, base);
    return -43200;
}

int32_t ConfigureModel801C9A64(Services& s, uint32_t index, uint32_t flag, uint32_t descriptor) {
    const uint32_t object = s.Read32(descriptor + 4u);
    s.Write32(object, 0u);
    s.Write16(object + 20u, flag != 0u ? 5120u : 4096u);
    s.Write16(object + 12u, s.Read16(0x801D2218u + 8u * index));
    const uint32_t result = s.Read32(0x801D221Cu + 8u * index);
    s.Write32(object + 32u, result);
    return Signed32(result);
}

void UploadInitialRuntimeTims801CB284(Services& s) {
    static uint32_t diagnosticCalls = 0u;
    if (diagnosticCalls < 8u) {
        Log::Printf("S2 runtime TIM initial call=%u heapDepth=%08X sceneFlags=%08X",
                    static_cast<unsigned>(diagnosticCalls), s.Read32(kHeapDepth),
                    s.Read32(0x801D297Cu));
    }
    for (uint32_t i = 0; i < 14u; ++i) {
        const uint32_t index = s.Read32(0x801D28D8u + 4u * i);
        if (diagnosticCalls < 8u) {
            Log::Printf("S2 runtime TIM initial slot=%u index=%u resource=%08X",
                        static_cast<unsigned>(i), static_cast<unsigned>(index),
                        s.Read32(kResourceTable + 4u * index));
        }
        PrStage2TimDirect::UploadRuntimeTim8001ADEC(s, s.Read32(0x80091858u + 4u * index), 1u);
    }
    ++diagnosticCalls;
}

int32_t InitResources801CB284(Services& s) {
    AllocatePackets801C9A00(s);
    const uint32_t firstPackets = s.Read32(0x801D29A0u);
    const uint32_t secondPackets = s.Read32(0x801D29A4u);
    BindDrawBuffers8001E33C(s, firstPackets, secondPackets);
    for (uint32_t i = 0; i < 2u; ++i) {
        const uint32_t offset = 20u * i;
        s.Write32(0x801D29A8u + offset, 10u);
        s.Write32(0x801D49D0u + offset, 4u);
        s.Write32(0x801D4A78u + offset, 2u);
        s.Write32(0x801D29ACu + offset, 0x801D29D0u + 4096u * i);
        s.Write32(0x801D49D4u + offset, 0x801D49F8u + 64u * i);
        s.Write32(0x801D4A7Cu + offset, 0x801D4AA0u + 16u * i);
        s.Write32(0x801D4A80u + offset, 0u);
        s.Write32(0x801D49D8u + offset, 0u);
        s.Write32(0x801D29B0u + offset, 0u);
    }
    s.Call(0x800246A8u, {2u}); // Callee consumes only a0, not stale a1-a3.
    s.Write32(0x801D2978u, 0u);
    ResetDrawFlags8001EEE8(s);
    s.Write32(0x801D2968u, 0u);
    s.Write32(0x801D2930u, 0u);
    s.Write32(0x8006EDCCu, 1u);
    s.Write32(0x801D2918u, 1u);
    for (uint32_t address : {0x801D2970u, 0x801D2958u, 0x801D294Cu, 0x801D2974u,
                             0x801D2928u, 0x801D2924u, 0x801D2920u, 0x801D291Cu})
        s.Write32(address, 0u);
    for (uint32_t i = 0; i < 12u; ++i) {
        const uint32_t parent = 0x801D5420u + 80u * i;
        InitCoordinate8004049C(s, parent, 0x801D4AC0u + 160u * i);
        InitCoordinate8004049C(s, parent, 0x801D4B10u + 160u * i);
        InitCoordinate8004049C(s, 0u, parent);
        // Preserve original three/three/two load-store groups.
        for (uint32_t offset : {0u, 12u}) {
            const uint32_t a = s.Read32(0x80091838u + offset);
            const uint32_t b = s.Read32(0x8009183Cu + offset);
            const uint32_t c = s.Read32(0x80091840u + offset);
            s.Write32(parent + 4u + offset, a);
            s.Write32(parent + 8u + offset, b);
            s.Write32(parent + 12u + offset, c);
        }
        const uint32_t a = s.Read32(0x80091850u);
        const uint32_t b = s.Read32(0x80091854u);
        s.Write32(parent + 28u, a);
        s.Write32(parent + 32u, b);
        s.Write32(0x801D4AD8u + 160u * i, 3500u);
        s.Write32(0x801D4B28u + 160u * i, U(-3500));
        s.Write32(0x801D53C4u + 8u * i, 0u);
        s.Write32(0x801D53C0u + 8u * i, 0u);
        s.Write32(0x801D5960u + 4u * i, 17u * i);
    }
    for (uint32_t parent : {0x801D5990u, 0x801D5A7Cu, 0x801D5A18u})
        InitCoordinate8004049C(s, 0u, parent);
    for (uint32_t i = 0; i < 4u; ++i) {
        InitCoordinate8004049C(s, 0u, 0x801D5AE0u + 80u * i);
        s.Write32(0x801D5B00u + 80u * i, U(-20000));
    }
    InitCoordinate8004049C(s, 0x801D5AE0u, 0x801D5C20u);
    if (s.Read32(0x801D297Cu) != 0u) {
        BindModel8001AF1C(s, s.Read32(0x800919B0u), 0x801D5ACCu, 0x801D5A7Cu);
        BindModel8001AF1C(s, s.Read32(0x800919B4u), 0x801D5A68u, 0x801D5A18u);
        for (uint32_t i = 0; i < 12u; ++i) {
            const uint32_t parent = 0x801D5420u + 80u * i;
            BindModel8001AF1C(s, s.Read32(0x800919B8u), 0x801D57E0u + 16u * i, parent);
            const uint32_t index = s.Read32(0x801D28A8u + 4u * i);
            BindModel8001AF1C(s, s.Read32(0x80091858u + 4u * index), 0x801D58A0u + 16u * i, parent);
        }
        const uint32_t first = U(BindModel8001AF1C(s, s.Read32(0x800919A8u), 0x801D59E0u, 0x801D5990u));
        const uint32_t secondResource = s.Read32(0x800919ACu);
        s.Write32(0x801D5A00u, first);
        const uint32_t second = U(BindModel8001AF1C(s, secondResource, 0x801D59F0u, 0x801D5990u));
        const uint32_t previous = s.Read32(0x801D5A00u);
        const uint32_t thirdResource = s.Read32(0x800919BCu);
        s.Write32(0x801D5A00u, previous + second);
        const uint32_t third = U(BindModel8001AF1C(s, thirdResource, 0x801D5A04u, 0x801D5990u));
        const uint32_t fourthResource = s.Read32(0x8009195Cu);
        s.Write32(0x801D5A14u, third);
        BindModel8001AF1C(s, fourthResource, 0x801D5E24u, 0x801D5AE0u);
        BindModel8001AF1C(s, s.Read32(0x800919A0u), 0x801D5E04u, 0x801D5AE0u);
        BindModel8001AF1C(s, s.Read32(0x80091960u), 0x801D5E64u, 0x801D5AE0u);
        BindModel8001AF1C(s, s.Read32(0x800919A4u), 0x801D5E44u, 0x801D5AE0u);
        s.Write32(0x801D5E00u, U(BindModel8001AF1C(s, s.Read32(0x80091964u), 0x801D5C70u, 0x801D5AE0u)));
        s.Write32(0x801D5C74u, 0x801D5B30u);
        s.Write32(0x801D5C84u, 0x801D5B80u);
        s.Write32(0x801D5C94u, 0x801D5BD0u);
        InitCoordinate8004049C(s, 0x801D5AE0u, 0x801D5C20u);
        s.Write32(0x801D5CB4u, 0x801D5C20u);
        s.Write32(0x801D297Cu, 0u);
    } else {
        s.Call(0x80014324u, {});
        s.Call(0x800139F8u, {1u});
    }
    PrStage2GpuDirect::SetProjection80040C74(s,884u);
    InitAnimationCursor8001AFD8(s, s.Read32(0x800918A8u), 0x801D2964u, 0x801D5E98u);
    AdvanceAnimation8001B000(s, 0u, 0x801D5E98u, 0x801D2964u, 0x801D5ACCu);
    InitAnimationCursor8001AFD8(s, s.Read32(0x800918E0u), 0x801D292Cu, 0x801D5A78u);
    AdvanceAnimation8001B000(s, 0u, 0x801D5A78u, 0x801D292Cu, 0x801D5A68u);
    InitAnimationCursor8001AFD8(s, s.Read32(0x8009186Cu), 0x801D2940u, 0x801D5E88u);
    InitAnimationCursor8001AFD8(s, s.Read32(0x8009186Cu), 0x801D295Cu, 0x801D5E94u);
    for (uint32_t i = 0; i < 12u; ++i) {
        const uint32_t index = s.Read32(0x801D5960u + 4u * i);
        ConfigureModel801C9A64(s, index, Signed32(index) < 20 ? 1u : 0u, 0x801D57E0u + 16u * i);
    }
    UploadInitialRuntimeTims801CB284(s);
    // 800127C4 overwrites a1; its tail callee 8004049C consumes only a0/a1.
    s.Call(0x800127C4u, {0x801D2188u});
    s.Call(0x800127F0u, {s.Read32(0x80091B80u), 17u});
    s.Call(0x800128DCu, {});
    s.Call(0x8001385Cu, {1u, s.Read32(0x800919B0u), s.Read32(0x800919C0u), 0x801D5E9Cu});
    s.Call(0x80013D10u, {0x801D6E9Cu, 0x801D6EACu, s.Read32(0x80091A60u), 0u});
    s.Call(0x80013EA8u, {0x801D6E9Cu, 0u, 1u});
    s.Call(0x80014164u, {s.Read32(0x80091AB8u), s.Read32(0x80091A18u), s.Read32(0x800919B4u), 0u});
    s.Call(0x800141D8u, {});
    return s.Call(0x80013E40u, {});
}

int32_t Present801CB170(Services& s) {
    PrStage2GpuDirect::SwapBuffers80040370(s);
    EnqueueClearPrimitive80040060(s, 152u, 200u, 248u, 0x801D4A78u + 20u * s.Read32(0x8006EDA8u));
    SubmitOrderingTable80040CA4(s, 0x801D4A78u + 20u * s.Read32(0x8006EDA8u));
    SubmitOrderingTable80040CA4(s, 0x801D49D0u + 20u * s.Read32(0x8006EDA8u));
    SubmitOrderingTable80040CA4(s, 0x801D29A8u + 20u * s.Read32(0x8006EDA8u));
    return SubmitWorkOrderingTable8001E3B0(s, s.Read32(0x8006EDA8u));
}

namespace {
struct PrimitiveVertices { uint32_t a, b, c; };
PrimitiveVertices ReadPrimitiveVertices(Services& s, uint32_t primitive,
    uint32_t offset, uint32_t vertices) {
    const uint32_t first = s.Read32(primitive + offset);
    const uint32_t third = s.Read16(primitive + offset + 4u);
    return {vertices + 8u * (first & 0xFFFFu),
        vertices + 8u * (first >> 16u), vertices + 8u * third};
}
void LoadPrimitiveVertices(Services& s, const PrimitiveVertices& v) {
    auto& g = s.MatrixGte();
    g.vectorXY0 = s.Read32(v.a); g.vectorZ0 = Signed16(static_cast<uint16_t>(s.Read32(v.a + 4u)));
    g.vectorXY12[0] = s.Read32(v.b); g.vectorZ12[0] = Signed16(static_cast<uint16_t>(s.Read32(v.b + 4u)));
    g.vectorXY12[1] = s.Read32(v.c); g.vectorZ12[1] = Signed16(static_cast<uint16_t>(s.Read32(v.c + 4u)));
}
void LinkTmdPacket(Services& s, uint32_t packet, uint32_t base,
    uint32_t bias, uint32_t shift, uint32_t length) {
    const uint32_t slot = base + 4u * (((uint32_t(s.MatrixGte().otz) - bias) >> (shift & 31u)) & 0xFFFFu);
    const uint32_t old = s.Read32(slot);
    if (IsTmdPacketTraceEnabled()) {
        TmdPacketTrace trace = s_tmdTraceContext;
        trace.packet = packet;
        trace.ot = base;
        trace.bucket = (slot - base) / 4u;
        trace.otz = static_cast<uint32_t>(s.MatrixGte().otz);
        trace.bias = bias;
        trace.shift = shift;
        trace.length = length;
        s_tmdPacketTrace.push_back(trace);
    }
    s.ObserveTmdPacket(packet, s_tmdTraceContext.descriptor,
                       s_tmdTraceContext.object, s_tmdTraceContext.primitive);
    s.Write32(packet, (old & 0xFFFFFFu) | (length << 24u));
    s.Write32(slot, ((packet ^ old) & 0xFFFFFFu) ^ old);
}

void SetTmdTracePrimitive(Services& s, uint32_t primitive, uint32_t stride) {
    if (!IsTmdPacketTraceEnabled() && !s.WantsTmdPresentation()) return;
    s_tmdTraceContext.primitive = primitive - stride;
    s_tmdTraceContext.primitiveMode = s.Read8(s_tmdTraceContext.primitive + 3u) & 0xFDu;
}
uint32_t TextureCommand(Services& s, uint32_t primitive, uint32_t global, uint32_t color) {
    const uint32_t mode = U(static_cast<int8_t>(s.Read8(primitive + 3u)));
    return (color & 0xFFFFFFu) | (((mode | (global << 1u)) >> 1u) << 25u);
}

template<bool Textured, bool Gouraud, bool Quad>
int32_t DrawUnlitTmd(Services& s, uint32_t primitive, uint32_t vertices,
    uint32_t packet, uint32_t count, uint32_t shift, uint32_t ot) {
    constexpr uint32_t stride = Quad ? (Textured ? 32u : 16u)
        : Gouraud ? (Textured ? 36u : 24u) : (Textured ? 28u : 16u);
    constexpr uint32_t indexOffset = Quad ? (Textured ? 24u : 8u)
        : Gouraud ? (Textured ? 28u : 16u) : (Textured ? 20u : 8u);
    const uint32_t base = s.Read32(ot + 4u); // Original count=0 branch delay slot.
    if (count == 0u) return Signed32(packet);
    const uint32_t bias = s.Read32(ot + 8u);
    PrimitiveVertices indices = ReadPrimitiveVertices(s, primitive, indexOffset, vertices);
    auto& g = s.MatrixGte();
    do {
        LoadPrimitiveVertices(s, indices);
        PrPsxGteDirect::ExecutePerspective(g, true);
        uint32_t fourth = 0u;
        if constexpr (Quad && Textured) {
            const uint32_t color = s.Read32(primitive + 20u);
            fourth = vertices + 8u * s.Read16(primitive + 30u);
            const uint32_t global = s.Read32(0x80095C48u);
            s.Write32(packet + 4u, TextureCommand(s, primitive, global, color));
        } else {
            uint32_t color;
            if constexpr (Textured && !Gouraud) {
                color = s.Read32(primitive + 16u);
                const uint32_t global = s.Read32(0x80095C48u);
                color = TextureCommand(s, primitive, global, color);
            } else if constexpr (Textured) {
                const uint32_t global = s.Read32(0x80095C48u);
                const uint32_t mode = U(static_cast<int8_t>(s.Read8(primitive + 3u)));
                color = s.Read32(primitive + 16u);
                color = (color & 0xFFFFFFu) | (((mode | (global << 1u)) >> 1u) << 25u);
            } else {
                const uint32_t global = s.Read32(0x80095C48u);
                color = s.Read32(primitive + 4u) | (global << 25u);
            }
            g.rgb[Gouraud ? 0u : 2u] = color;
            if constexpr (Gouraud) {
                g.rgb[1] = s.Read32(primitive + (Textured ? 20u : 8u));
                g.rgb[2] = s.Read32(primitive + (Textured ? 24u : 12u));
            }
            if constexpr (Quad) fourth = vertices + 8u * s.Read16(primitive + 14u);
            if constexpr (Textured) {
                const uint32_t uv0 = s.Read32(primitive + 4u), uv1 = s.Read32(primitive + 8u);
                const uint32_t uv2 = s.Read32(primitive + 12u);
                s.Write32(packet + 12u, uv0);
                s.Write32(packet + (Gouraud ? 24u : 20u), uv1);
                s.Write32(packet + (Gouraud ? 36u : 28u), uv2);
            }
        }
        primitive += stride;
        // Original prefetch also runs for the final primitive, before culling.
        indices = ReadPrimitiveVertices(s, primitive, indexOffset, vertices);
        if (Signed32(g.flags) >= 0) {
            PrPsxGteDirect::ExecuteNclip(g);
            if constexpr (Quad) {
                // These loads precede the NCLIP sign branch, even if culled.
                g.vectorXY0 = s.Read32(fourth);
                g.vectorZ0 = Signed16(static_cast<uint16_t>(s.Read32(fourth + 4u)));
            }
            if (g.mac0 > 0) {
                if constexpr (Quad) {
                    if constexpr (!Textured) s.Write32(packet + 4u, g.rgb[2]);
                    s.Write32(packet + 8u, g.sxy[0]);
                    s.Write32(packet + (Textured ? 16u : 12u), g.sxy[1]);
                    s.Write32(packet + (Textured ? 24u : 16u), g.sxy[2]);
                    PrPsxGteDirect::ExecutePerspective(g, false);
                    if constexpr (Textured) {
                        const uint32_t previous = primitive - stride;
                        const uint32_t uv0 = s.Read32(previous + 4u), uv1 = s.Read32(previous + 8u);
                        s.Write32(packet + 12u, uv0); s.Write32(packet + 20u, uv1);
                        const uint32_t uv2 = s.Read32(previous + 12u), uv3 = s.Read32(previous + 16u);
                        s.Write32(packet + 28u, uv2); s.Write32(packet + 36u, uv3);
                    }
                    if (Signed32(g.flags) >= 0) {
                        PrPsxGteDirect::ExecuteAverageZ(g, true);
                        s.Write32(packet + (Textured ? 32u : 20u), g.sxy[2]);
                        SetTmdTracePrimitive(s, primitive, stride);
                        LinkTmdPacket(s, packet, base, bias, shift, Textured ? 9u : 5u);
                        packet += Textured ? 40u : 24u;
                    }
                } else {
                    PrPsxGteDirect::ExecuteAverageZ(g, false);
                    if constexpr (Gouraud) {
                        if constexpr (Textured) {
                            s.Write32(packet + 8u, g.sxy[0]); s.Write32(packet + 20u, g.sxy[1]);
                            s.Write32(packet + 32u, g.sxy[2]);
                            s.Write32(packet + 4u, g.rgb[0]); s.Write32(packet + 16u, g.rgb[1]);
                            s.Write32(packet + 28u, g.rgb[2]);
                        } else {
                            s.Write32(packet + 4u, g.rgb[0]); s.Write32(packet + 12u, g.rgb[1]);
                            s.Write32(packet + 20u, g.rgb[2]);
                            s.Write32(packet + 8u, g.sxy[0]); s.Write32(packet + 16u, g.sxy[1]);
                            s.Write32(packet + 24u, g.sxy[2]);
                        }
                        SetTmdTracePrimitive(s, primitive, stride);
                        LinkTmdPacket(s, packet, base, bias, shift, Textured ? 9u : 6u);
                        packet += Textured ? 40u : 28u;
                    } else {
                        SetTmdTracePrimitive(s, primitive, stride);
                        LinkTmdPacket(s, packet, base, bias, shift, Textured ? 7u : 4u);
                        s.Write32(packet + 4u, g.rgb[2]); s.Write32(packet + 8u, g.sxy[0]);
                        s.Write32(packet + (Textured ? 16u : 12u), g.sxy[1]);
                        s.Write32(packet + (Textured ? 24u : 16u), g.sxy[2]);
                        packet += Textured ? 32u : 20u;
                    }
                }
            }
        }
        --count;
    } while (count != 0u);
    return Signed32(packet);
}
}

int32_t DrawNf3_8003B88C(Services& s, uint32_t p, uint32_t v, uint32_t out, uint32_t n, uint32_t shift, uint32_t ot) {
    return DrawUnlitTmd<false, false, false>(s, p, v, out, n, shift, ot);
}
int32_t DrawTnf3_8003CFDC(Services& s, uint32_t p, uint32_t v, uint32_t out, uint32_t n, uint32_t shift, uint32_t ot) {
    return DrawUnlitTmd<true, false, false>(s, p, v, out, n, shift, ot);
}
int32_t DrawNf4_8003BD9C(Services& s, uint32_t p, uint32_t v, uint32_t out, uint32_t n, uint32_t shift, uint32_t ot) {
    return DrawUnlitTmd<false, false, true>(s, p, v, out, n, shift, ot);
}
int32_t DrawTnf4_8003D58C(Services& s, uint32_t p, uint32_t v, uint32_t out, uint32_t n, uint32_t shift, uint32_t ot) {
    return DrawUnlitTmd<true, false, true>(s, p, v, out, n, shift, ot);
}
int32_t DrawNg3_8003C36C(Services& s, uint32_t p, uint32_t v, uint32_t out, uint32_t n, uint32_t shift, uint32_t ot) {
    return DrawUnlitTmd<false, true, false>(s, p, v, out, n, shift, ot);
}
int32_t DrawTng3_8003DC2C(Services& s, uint32_t p, uint32_t v, uint32_t out, uint32_t n, uint32_t shift, uint32_t ot) {
    return DrawUnlitTmd<true, true, false>(s, p, v, out, n, shift, ot);
}

namespace {
int32_t InvokeModelHandler(Services& s, uint32_t function, std::initializer_list<uint32_t> args) {
    const uint32_t* a = args.begin();
    switch (function) {
    case 0x8003B88Cu: return DrawNf3_8003B88C(s,a[0],a[1],a[2],a[3],a[4],a[5]);
    case 0x8003CFDCu: return DrawTnf3_8003CFDC(s,a[0],a[1],a[2],a[3],a[4],a[5]);
    case 0x8003BD9Cu: return DrawNf4_8003BD9C(s,a[0],a[1],a[2],a[3],a[4],a[5]);
    case 0x8003D58Cu: return DrawTnf4_8003D58C(s,a[0],a[1],a[2],a[3],a[4],a[5]);
    case 0x8003C36Cu: return DrawNg3_8003C36C(s,a[0],a[1],a[2],a[3],a[4],a[5]);
    case 0x8003DC2Cu: return DrawTng3_8003DC2C(s,a[0],a[1],a[2],a[3],a[4],a[5]);
    default: return s.Call(function, args); // Remaining original indirect callees, not a fallback.
    }
}
}

void SubmitModel800428B0(Services& s, uint32_t descriptor,
    uint32_t ot, uint32_t depthShift, uint32_t scratch) {
    const uint32_t firstFlags = s.Read32(descriptor);
    if (Signed32(firstFlags) < 0) return;
    // Original reloads interleave with global stores: descriptors/scratch may
    // alias those fields. Do not collapse these into one cached flags value.
    const uint32_t flags3 = s.Read32(descriptor);
    s.Write32(0x800901CCu, firstFlags & 7u);
    const uint32_t flags5 = s.Read32(descriptor);
    s.Write32(0x8008ECB4u, (flags3 >> 3u) & 3u);
    const uint32_t flags6 = s.Read32(descriptor);
    s.Write32(0x8008ECB8u, (flags5 >> 5u) & 1u);
    const uint32_t flags9 = s.Read32(descriptor);
    s.Write32(0x8008ECB0u, (flags6 >> 6u) & 1u);
    const uint32_t flags30 = s.Read32(descriptor);
    const uint32_t special = (flags9 >> 9u) & 7u;
    s.Write32(0x801C3638u, special);
    s.Write32(0x80095C48u, (flags30 >> 30u) & 1u);
    const uint32_t specialIndex = special != 0u ? 1u : 0u;
    if (special != 0u) {
        const uint32_t first = s.Read32(0x800917FCu);
        const uint32_t second = s.Read32(0x8009182Cu);
        s.Write32(scratch, special);
        s.Write32(scratch + 4u, first);
        s.Write32(scratch + 8u, second);
    }
    uint32_t lightIndex = 2u;
    if (s.Read32(0x8008ECB0u) != 1u) {
        const uint32_t flags = s.Read32(0x8008ECB8u);
        if (flags == 0u && (s.Read32(0x80095D00u) & 1u) != 0u) lightIndex = 1u;
        else if (flags == 1u) lightIndex = (s.Read32(0x8008ECB4u) & 1u) != 0u ? 1u : 0u;
        else lightIndex = 0u;
    }
    const uint32_t object = s.Read32(descriptor + 8u);
    if (IsTmdPacketTraceEnabled() || s.WantsTmdPresentation()) {
        s_tmdTraceContext.descriptor = descriptor;
        s_tmdTraceContext.object = object;
    }
    uint32_t primitive = s.Read32(object + 16u);
    uint32_t remaining = s.Read32(object + 20u);
    const uint32_t vertices = s.Read32(object);
    const uint32_t normals = s.Read32(object + 8u);
    while (remaining != 0u) {
        const uint32_t mode = s.Read8(primitive + 3u) & 0xFDu;
        uint32_t table = 0u, stride = 0u;
        bool withNormals = (mode & 1u) == 0u;
        uint32_t tableIndex = withNormals ? 3u * specialIndex + lightIndex : specialIndex;
        switch (mode) {
        case 0x20u:
            if ((s.Read16(primitive + 2u) & 4u) != 0u) {
                table = 0x8008EED8u; tableIndex = lightIndex; stride = 24u;
            } else { table = 0x8008EDD8u; stride = 16u; }
            break;
        case 0x21u: table = 0x8008EDF0u; stride = 16u; break;
        case 0x24u: table = 0x8008EE18u; stride = 24u; break;
        case 0x25u: table = 0x8008EE30u; stride = 28u; break;
        case 0x28u: table = 0x8008EE58u; stride = 20u; break;
        case 0x29u: table = 0x8008EE70u; stride = 16u; break;
        case 0x2Cu: table = 0x8008EE98u; stride = 32u; break;
        case 0x2Du: table = 0x8008EEB0u; stride = 32u; break;
        case 0x30u:
            if ((s.Read16(primitive + 2u) & 4u) != 0u) {
                table = 0x8008EEE4u; tableIndex = lightIndex; stride = 28u;
            } else { table = 0x8008EDF8u; stride = 20u; }
            break;
        case 0x31u: table = 0x8008EE10u; stride = 24u; break;
        case 0x34u: table = 0x8008EE38u; stride = 28u; break;
        case 0x35u: table = 0x8008EE50u; stride = 36u; break;
        case 0x38u: table = 0x8008EE78u; stride = 24u; break;
        case 0x39u: table = 0x8008EE90u; stride = 28u; break;
        case 0x3Cu: table = 0x8008EEB8u; stride = 36u; break;
        case 0x3Du: table = 0x8008EED0u; stride = 44u; break;
        default:
            // Original diagnostic retries the same cursor; it does not skip
            // unknown geometry or report a successful submission.
            s.Call(0x80047FFCu, {0x80012204u, mode, primitive});
            continue;
        }
        uint32_t count, packet;
        if (withNormals) {
            count = s.Read16(primitive);
            packet = s.Read32(0x800901C8u);
        } else {
            packet = s.Read32(0x800901C8u);
            count = s.Read16(primitive);
        }
        const uint32_t handler = s.Read32(table + 4u * tableIndex);
        if (IsTmdPacketTraceEnabled() || s.WantsTmdPresentation()) {
            s_tmdTraceContext.descriptor = descriptor;
            s_tmdTraceContext.object = object;
            s_tmdTraceContext.handler = handler;
            s_tmdTraceContext.primitiveMode = mode;
        }
        const int32_t nextPacket = withNormals
            ? InvokeModelHandler(s, handler, {primitive, vertices, normals, packet, count, depthShift, ot, scratch})
            : InvokeModelHandler(s, handler, {primitive, vertices, packet, count, depthShift, ot, scratch});
        s.Write32(0x800901C8u, U(nextPacket));
        // The handler can change the grouped count. Original rereads it after
        // publishing the packet pointer, and uses uint32 wrapping subtraction.
        const uint32_t consumed = s.Read16(primitive);
        remaining -= consumed;
        primitive += stride * consumed;
    }
}

void DrawModels8001B084(Services& s, uint32_t descriptor,
    int32_t count, uint32_t ot, int32_t depth) {
    for (int32_t i = 0; i < count; ++i) {
        Matrix32 matrix{};
        const MatrixRef local{s, 0u, &matrix};
        ResolveWorldMatrix(s, s.Read32(descriptor + 4u), local);
        LoadRotation(local); LoadTranslation(local);
        SubmitModel800428B0(s, descriptor, ot, 14u - U(depth), 0x1F800000u);
        descriptor += 16u;
    }
}

int32_t DrawModelsAfterFirst801C9ABC(Services& s, uint32_t descriptor,
                                    int32_t count, uint32_t ot, int32_t depth) {
    descriptor += 16u;
    for (int32_t i = 1; i < count; ++i) {
        Matrix32 matrix{};
        const MatrixRef local{s, 0u, &matrix};
        ResolveWorldMatrix(s, s.Read32(descriptor + 4u), local);
        LoadRotation(local); LoadTranslation(local);
        SubmitModel800428B0(s, descriptor, ot, 14u - U(depth), 0x1F800000u);
        descriptor += 16u;
    }
    return 0; // Original SLT is false both on initial rejection and loop exit.
}

namespace {
void SubmitModels(Services& s, uint32_t descriptor, uint32_t count,
                     uint32_t otBase, uint32_t depth) {
    DrawModels8001B084(s, descriptor, Signed32(count),
        otBase + 20u * s.Read32(0x8006EDA8u), Signed32(depth));
}
void SubmitSelectedDescriptors(Services& s) {
    SubmitModels(s, 0x801D5E24u + 16u * s.Read32(0x801D2928u), 1u, 0x801D29A8u, 10u);
    SubmitModels(s, 0x801D5E04u + 16u * s.Read32(0x801D2924u), 1u, 0x801D29A8u, 10u);
    SubmitModels(s, 0x801D5E64u + 16u * s.Read32(0x801D2920u), 1u, 0x801D29A8u, 10u);
    SubmitModels(s, 0x801D5E44u + 16u * s.Read32(0x801D291Cu), 1u, 0x801D29A8u, 10u);
}
}

void DrawPreparedScene801C9B5C(Services& s) {
    SubmitModels(s, 0x801D5A68u, 1u, 0x801D29A8u, 10u);
    SubmitModels(s, 0x801D5ACCu, 1u, 0x801D29A8u, 10u);
    const uint32_t buffer = s.Read32(0x8006EDA8u);
    const int32_t count = Signed32(s.Read32(0x801D5E00u));
    DrawModelsAfterFirst801C9ABC(s, 0x801D5C70u, count, 0x801D29A8u + 20u * buffer, 10);
    if (s.Read32(0x801D2978u) == 0u)
        SubmitModels(s, 0x801D5C70u, 1u, 0x801D29A8u, 10u);
    SubmitSelectedDescriptors(s);
    for (uint32_t i = 0; i < 24u; ++i) {
        if (s.Read32(0x801D53C0u + 4u * i) != 0u)
            SubmitModels(s, 0x801D5240u + 16u * i, 1u, 0x801D29A8u, 10u);
    }
    SubmitModels(s, 0x801D57E0u, 12u, 0x801D49D0u, 4u);
    return SubmitModels(s, 0x801D58A0u, 12u, 0x801D29A8u, 10u);
}

void AnimateAndDrawScene801C9E18(Services& s) {
    s.Call(0x800141D8u, {});
    const uint32_t frame2930 = s.Read32(0x801D2930u);
    s.Write32(0x801D2930u, frame2930 + 1u);
    AdvanceAnimation8001B000(s, frame2930, 0x801D5A78u, 0x801D292Cu, 0x801D5A68u);
    SubmitModels(s, 0x801D5A68u, 1u, 0x801D29A8u, 10u);
    const uint32_t frame2934 = s.Read32(0x801D2934u);
    s.Write32(0x801D2934u, frame2934 + 1u);
    s.Call(0x80013EA8u, {0x801D6E9Cu, frame2934, 1u});
    const uint32_t frame2968 = s.Read32(0x801D2968u);
    s.Write32(0x801D2968u, frame2968 + 1u);
    AdvanceAnimation8001B000(s, frame2968, 0x801D5E98u, 0x801D2964u, 0x801D5ACCu);
    SubmitModels(s, 0x801D5ACCu, 1u, 0x801D29A8u, 10u);
    if (s.Read32(0x801D294Cu) != 0u) {
        const uint32_t cursor = s.Read32(0x801D2948u);
        const uint32_t frame = s.Read32(0x801D2944u);
        if (cursor != 0u) {
            if (AdvanceAnimation8001B000(s, frame, 0x801D5E8Cu, 0x801D2948u, 0x801D5C70u) > 0)
                SubmitModels(s, 0x801D5C70u, 1u, 0x801D29A8u, 10u);
            else s.Write32(0x801D294Cu, 0u);
        }
    }
    if (s.Read32(0x801D2958u) != 0u) {
        const uint32_t frame = s.Read32(0x801D2954u);
        const uint32_t cursor = s.Read32(0x801D2950u);
        s.Write32(0x801D2954u, frame + 1u);
        if (cursor != 0u) {
            if (AdvanceAnimation8001B000(s, frame, 0x801D5E90u, 0x801D2950u, 0x801D5CB0u) > 0)
                SubmitModels(s, 0x801D5CB0u, 1u, 0x801D29A8u, 10u);
            else s.Write32(0x801D2958u, 0u);
        }
    }
    const uint32_t frame2944 = s.Read32(0x801D2944u);
    const uint32_t savedCursor = s.Read32(0x801D2940u);
    const uint32_t savedMode = s.Read32(0x801D2970u);
    s.Write32(0x801D2944u, frame2944 + 1u);
    if (savedCursor != 0u) {
        uint32_t localCount = s.Read32(0x801D5E88u);
        if (Signed32(localCount) > 0) {
            AdvanceAnimationLocalCount8001B000(s, frame2944, localCount, 0x801D2940u, 0x801D5CE0u);
            s.Write32(0x801D2940u, savedCursor);
            AdvanceAnimation8001B000(s, frame2944, 0x801D5E88u, 0x801D2940u,
                                savedMode != 0u ? 0x801D5C90u : 0x801D5C80u);
        }
    }
    if (s.Read32(0x801D2970u) != 0u) {
        const uint32_t frame = s.Read32(0x801D293Cu);
        s.Write32(0x801D293Cu, frame + 1u);
        AdvanceAnimation8001B000(s, frame, 0x801D5E84u, 0x801D2938u, 0x801D5C80u);
    } else {
        const uint32_t frame = s.Read32(0x801D2960u);
        s.Write32(0x801D2960u, frame + 1u);
        AdvanceAnimation8001B000(s, frame, 0x801D5E94u, 0x801D295Cu, 0x801D5C90u);
    }
    const uint32_t buffer = s.Read32(0x8006EDA8u);
    const int32_t count = Signed32(s.Read32(0x801D5E00u));
    DrawModelsAfterFirst801C9ABC(s, 0x801D5C70u, count, 0x801D29A8u + 20u * buffer, 10);
    if (s.Read32(0x801D2978u) == 0u)
        SubmitModels(s, 0x801D5C70u, 1u, 0x801D29A8u, 10u);
    SubmitSelectedDescriptors(s);
    const bool advance = s.Read32(0x801D2918u) != 0u;
    for (uint32_t i = 0; i < 12u; ++i) {
        const uint32_t address = 0x801D5960u + 4u * i;
        if (advance && Signed32(s.Read32(address)) >= 204) {
            s.Write32(address, 0u);
            s.Write32(0x801D2910u, i);
            s.Write32(0x801D2914u, 2u * i);
            s.Write32(0x801D53C0u + 8u * i, 0u);
            s.Write32(0x801D53C4u + 8u * i, 0u);
        }
        const uint32_t index = s.Read32(address);
        ConfigureModel801C9A64(s, index, Signed32(index) < 20 ? 1u : 0u, 0x801D57E0u + 16u * i);
        if (advance) s.Write32(address, s.Read32(address) + 1u);
    }
    if (s.Read32(0x801D2974u) != 0u) {
        const uint32_t firstBuffer = s.Read32(0x8006EDA8u);
        const uint32_t firstCount = s.Read32(0x801D5A00u);
        DrawModels8001B084(s, 0x801D59E0u, Signed32(firstCount), 0x801D49D0u + 20u * firstBuffer, 4);
        const uint32_t secondBuffer = s.Read32(0x8006EDA8u);
        const uint32_t secondCount = s.Read32(0x801D5A14u);
        return DrawModels8001B084(s, 0x801D5A04u, Signed32(secondCount), 0x801D29A8u + 20u * secondBuffer, 10);
    }
    for (uint32_t i = 0; i < 24u; ++i) {
        if (s.Read32(0x801D53C0u + 4u * i) != 0u)
            SubmitModels(s, 0x801D5240u + 16u * i, 1u, 0x801D29A8u, 10u);
    }
    SubmitModels(s, 0x801D57E0u, 12u, 0x801D49D0u, 4u);
    return SubmitModels(s, 0x801D58A0u, 12u, 0x801D29A8u, 10u);
}

void UploadFrameRuntimeTims801CA57C(Services& s, uint32_t work) {
    static uint32_t diagnosticCalls = 0u;
    const bool diagnose = diagnosticCalls < 64u;
    if (diagnose) {
        Log::Printf("S2 runtime TIM frame call=%u work=%08X flags=%08X drawFlag=%08X",
                    static_cast<unsigned>(diagnosticCalls), work, s.Read32(work),
                    s.Read32(0x801D2978u));
    }
    for (uint32_t lane = 0; lane < 2u; ++lane) {
        const uint32_t slot = work + 172u + 4u * lane;
        if (diagnose) {
            const uint32_t list = s.Read32(slot);
            Log::Printf("S2 runtime TIM frame lane=%u slot=%08X list=%08X first=%04X",
                        static_cast<unsigned>(lane), slot, list,
                        list ? static_cast<unsigned>(s.Read16(list)) : 0u);
        }
        uint32_t offset = 0u;
        if (s.Read16(s.Read32(slot)) != 0u) {
            do {
                const uint32_t index = U(Signed16(s.Read16(s.Read32(slot) + offset)));
                if (diagnose) {
                    Log::Printf("S2 runtime TIM frame upload lane=%u offset=%u index=%u resource=%08X",
                                static_cast<unsigned>(lane), static_cast<unsigned>(offset),
                                static_cast<unsigned>(index),
                                s.Read32(kResourceTable + 4u * index));
                }
                PrStage2TimDirect::UploadRuntimeTim8001ADEC(s, s.Read32(0x80091858u + 4u * index), lane == 0u ? 1u : 0u);
                offset += 2u;
            } while (s.Read16(s.Read32(slot) + offset) != 0u);
        }
    }
    ++diagnosticCalls;
}

void PrepareFrame801CA57C(Services& s, uint32_t work, int32_t kind) {
    s.ObservePresentationFrame(work, kind);
    ResetTmdPacketTrace();
    const uint32_t buffer = U(PrStage2GpuDirect::CurrentDrawBuffer8004019C(s));
    const uint32_t packet = s.Read32(0x801D29A0u + 4u * buffer);
    s.Write32(0x8006EDA8u, buffer);
    PrStage2GpuDirect::SetWorkBase80040F90(s,packet);
    for (uint32_t ot : {0x801D4A78u, 0x801D49D0u, 0x801D29A8u})
        ClearOrderingTable80040CC8(s, 0u, 0u, ot + 20u * s.Read32(0x8006EDA8u));
    ClearWorkOrderingTable8001E374(s, s.Read32(0x8006EDA8u));
    if ((s.Read32(work) & 0x20u) != 0u) s.Write32(0x801D2978u, 1u);
    if (kind == 2) {
        s.Call(0x8001FCBCu, {4u, 4u});
        s.Call(0x8001FDC0u, {2u});
        return DrawPreparedScene801C9B5C(s);
    }
    s.Call(0x8001FC40u, {1u, 3u});
    s.Call(0x8001FDC0u, {2u});
    // 801CA57C's normal entry is also used by the pre-loop.  The destination
    // scene is already prepared beneath the tile mask, and the product keeps
    // its gameplay/input boundary closed until 8001A750 returns ready.  During
    // this window the product deliberately uses the prepared scene draw only;
    // the dynamic branch would advance the camera/animation below the mask.
    const bool startupSceneAnimation = s.AllowStartupSceneAnimation();
    if (startupSceneAnimation && s.Read32(0x801D2978u) == 0u)
        s.Write32(0x801D2978u, 1u);
    if (s.Read32(0x801D2978u) == 0u ||
        (!s.AllowGameplayFrame() && !startupSceneAnimation)) {
        // Keep the source-level 801C9B5C call available to reference and
        // instruction-diff adapters, while a product presenter can hold the
        // scene off the scanout until the reveal/clock handoff is complete.
        if (!s.AllowStartupSceneDraw()) return;
        return DrawPreparedScene801C9B5C(s);
    }
    const uint32_t flags = s.Read32(work);
    s.Write32(0x801D708Cu, U(Signed16(s.Read16(work + 78u))));
    if ((flags & 0x8000u) != 0u) {
        Log::Printf("S2 runtime TIM frame gate work=%08X kind=%d flags=%08X",
                    work, kind, flags);
        UploadFrameRuntimeTims801CA57C(s, work);
    }
    const auto resetIndices = [&]() {
        for (uint32_t i = 0; i < 12u; ++i) {
            const uint32_t address = 0x801D5960u + 4u * i;
            s.Write32(address, U(17 * (Signed32(s.Read32(address)) / 17)));
        }
    };
    if ((s.Read32(work) & 0x02000000u) != 0u) {
        resetIndices();
        s.Write32(0x801D2918u, 0u);
    }
    if ((s.Read32(work) & 0x04000000u) != 0u) s.Write32(0x801D2918u, 1u);
    if ((s.Read32(work) & 0x08000000u) != 0u) {
        if (Signed32(s.Read32(0x801D2910u)) >= 12) {
            s.Write32(0x801D2910u, 0u);
            s.Write32(0x801D2914u, 0u);
        }
        for (uint32_t lane = 0; lane < 2u; ++lane) {
            const uint32_t resource = s.Read8(work + 372u + lane);
            const uint32_t index = s.Read32(0x801D2914u);
            if (resource != 0u) {
                BindModel8001AF1C(s, s.Read32(0x80091858u + 4u * resource),
                    0x801D5240u + 16u * lane + 16u * index,
                    0x801D4AC0u + 80u * lane + 80u * index);
                s.Write32(0x801D53C0u + 4u * lane + 4u * s.Read32(0x801D2914u), 1u);
            } else {
                s.Write32(0x801D53C0u + 4u * lane + 4u * index, 0u);
            }
        }
        const uint32_t count = s.Read32(0x801D2910u) + 1u;
        s.Write32(0x801D2910u, count);
        s.Write32(0x801D2914u, 2u * count);
    }
    if ((s.Read32(work) & 0x40000u) != 0u) {
        InitAnimationCursor8001AFD8(s, s.Read32(work + 244u), 0x801D292Cu, 0x801D5A78u);
        s.Write32(0x801D2930u, 0u);
    }
    const uint32_t animationFlags = s.Read32(work);
    if ((animationFlags & 0x10u) != 0u) {
        const uint32_t length = U(Signed16(s.Read16(work + 288u)));
        const uint32_t start = U(Signed16(s.Read16(work + 286u)));
        s.Call(0x800140E0u, {s.Read32(work + 220u), s.Read32(work + 232u), 0u, start, length});
    } else if ((animationFlags & 0x10000u) != 0u) {
        s.Call(0x800140E0u, {s.Read32(work + 220u), s.Read32(work + 232u), 0u, 0u, 999u});
    }
    if ((s.Read32(work) & 0x80000u) != 0u) {
        InitAnimationCursor8001AFD8(s, s.Read32(work + 248u), 0x801D2964u, 0x801D5E98u);
        s.Write32(0x801D2968u, 0u);
    }
    if ((s.Read32(work) & 0x20000u) != 0u && s.Read32(work + 224u) != 0u)
        s.Call(0x80014050u, {1u, s.Read32(work + 224u), s.Read32(work + 236u), 0x801D6E9Cu, 0x801D6EACu, 0x801D2934u});
    const uint32_t overlayFlags = s.Read32(work);
    if ((overlayFlags & 0x400000u) != 0u) {
        InitAnimationCursor8001AFD8(s, s.Read32(0x80091910u), 0x801D2948u, 0x801D5E8Cu);
        s.Write32(0x801D2944u, 0u);
        s.Write32(0x801D294Cu, 1u);
    } else if ((overlayFlags & 0x10000000u) != 0u) {
        if (s.Read32(0x801D708Cu) == 0u) {
            InitAnimationCursor8001AFD8(s, s.Read32(0x80091858u + 4u * s.Read8(work + 376u)), 0x801D2938u, 0x801D5E84u);
            s.Write32(0x801D293Cu, 0u);
            s.Write32(0x801D2970u, 1u);
        } else s.Write32(0x801D2970u, 0u);
        s.Write32(0x801D2944u, 0u);
        InitAnimationCursor8001AFD8(s, s.Read32(0x80091858u + 4u * s.Read8(work + 374u)), 0x801D2940u, 0x801D5E88u);
        InitAnimationCursor8001AFD8(s, s.Read32(0x80091858u + 4u * s.Read8(work + 375u)), 0x801D295Cu, 0x801D5E94u);
        s.Write32(0x801D2960u, 0u);
    }
    if ((s.Read32(work) & 0x20000000u) != 0u) {
        InitAnimationCursor8001AFD8(s, s.Read32(0x8009188Cu), 0x801D2950u, 0x801D5E90u);
        s.Write32(0x801D2954u, 0u);
        s.Write32(0x801D2958u, 1u);
    }
    if ((s.Read32(work) & 0x800000u) != 0u) {
        PrStage2GpuDirect::SetProjection80040C74(s,440u);
        s.Write32(0x801D2974u, 1u);
    }
    if ((s.Read32(work) & 0x1000000u) != 0u) {
        PrStage2GpuDirect::SetProjection80040C74(s,884u);
        s.Write32(0x801D2974u, 0u);
    }
    if ((s.Read32(work) & 0x40u) != 0u) resetIndices();
    const uint32_t value256 = s.Read32(work + 256u);
    const uint32_t value252 = s.Read32(work + 252u);
    s.Write32(0x801D2928u, value256);
    s.Write32(0x801D2924u, value252);
    s.Write32(0x801D2920u, s.Read8(work + 378u));
    const uint32_t cameraFlags = s.Read32(work);
    s.Write32(0x801D291Cu, s.Read8(work + 377u));
    if ((cameraFlags & 0x400u) != 0u) {
        const uint32_t camera = s.Read32(work + 260u);
        s.Write32(0x801D296Cu, 0u);
        s.Call(0x800127F0u, {camera, 17u});
    }
    if (s.Read32(0x801D2978u) != 0u) {
        const uint32_t prior = s.Read32(0x801D296Cu);
        s.Write32(0x801D296Cu, prior + 1u);
        s.Call(0x80012960u, {prior}); // raw a0 is the PRE-increment frame.
    }
    const uint32_t targetX = s.Read32(0x80095C6Cu);
    const uint32_t eyeX = s.Read32(0x80095C60u);
    const uint32_t eyeY = s.Read32(0x80095C64u);
    const uint32_t eyeZ = s.Read32(0x80095C68u);
    const uint32_t targetY = s.Read32(0x80095C70u);
    const uint32_t targetZ = s.Read32(0x80095C74u);
    const Vector32 vector = s.NormalizeVector8003A3DC({targetX - eyeX, targetY - eyeY, targetZ - eyeZ});
    const uint32_t y = Sra(0u - 884u * vector.y, 12u);
    uint32_t x = 0u;
    if (vector.x != 0u || vector.z != 0u) x = Sra(U(s.Call(0x8003B70Cu, {vector.z, vector.x})), 1u) - 520u;
    const auto sprite = [&](int32_t dx, int32_t dy, uint32_t descriptor) {
        s.Call(0x8001B590u, {Extend16(x + U(dx)), Extend16(y + U(dy)), descriptor, 0u, 0u, 0u,
                            0x801D4A78u + 20u * s.Read32(0x8006EDA8u)});
    };
    for (int32_t dx : {-135, 185, 505}) sprite(dx, 76, 0x801D2878u);
    for (int32_t dx : {-278, 42, 362}) sprite(dx, 30, 0x801D2888u);
    for (int32_t dx : {-256, 0, 256}) sprite(dx, 93, 0x801D2898u);
    s.Call(0x8001B6C4u, {0u, Extend16(y + 159u), 320u, (80u - y) & 0xFFFFu,
                         0x00C89632u, 0u, 0x801D4A78u + 20u * s.Read32(0x8006EDA8u)});
    if (s.Read16(work + 84u) != 0u && s.Read16(work + 104u) != 0u)
        s.Call(0x80023E10u, {s.Read32(work + 268u)});
    if (s.Read16(work + 100u) != 0u)
        s.Call(0x8001E2E4u, {work, U(Signed16(s.Read16(work + 104u)))});
    s.Call(0x80024744u, {work});
    return AnimateAndDrawScene801C9E18(s);
}

int32_t GameClock801C6858(Services& s) {
    FlushAudio80026ECC(s);
    const uint32_t entry = s.Read32(kSceneEntry);
    s.Call(0x8001A3C8u, {});
    s.Call(0x8001A280u, {});
    const int32_t position = s.Call(0x8001A7A4u, {s.Read32(entry + 196u)});
    if (Signed32(s.Read32(0x801D2984u)) < position) {
        const uint32_t product = s.Read32(s.Read32(kSceneEntry) + 348u) * U(position);
        Words64 value = s.Call64(0x8002902Cu, {product});
        value = s.Call64(0x80028B40u, {value.low, value.high, 0u, 0x40B19400u});
        value = s.Call64(0x8002864Cu, {value.low, value.high, 0u, 0x3FE00000u});
        const uint32_t rounded = U(s.Call(0x80028F2Cu, {value.low, value.high}));
        s.Write32(kTransition + 12u, rounded + s.Read32(s.Read32(kSceneEntry) + 356u));
    } else {
        const uint32_t currentEntry = s.Read32(kSceneEntry);
        const uint32_t current = s.Read32(kTransition + 12u);
        s.Write32(kTransition + 12u, current + s.Read32(currentEntry + 352u));
    }
    const int32_t barTick = Signed32(s.Read32(kTransition + 12u));
    const int32_t beatTick = Signed32(s.Read32(kTransition + 12u));
    const int32_t stepTick = Signed32(s.Read32(kTransition + 12u));
    s.Write32(0x801D2984u, U(position));
    s.Write16(kTransition + 4u, static_cast<uint16_t>(barTick / 384 + 1));
    s.Write8(kTransition + 6u, static_cast<uint8_t>(beatTick % 384 / 96 + 1));
    s.Write8(kTransition + 7u, static_cast<uint8_t>(stepTick % 384 % 96 + 1));
    return 96 * (stepTick % 384 / 96);
}

int32_t CopyBytes80025C64(Services& s, uint32_t source, uint32_t destination, int32_t count) {
    while (count > 0) {
        const uint8_t value = s.Read8(source++);
        --count;
        s.Write8(destination++, value); // Forward byte copy, including overlapping ranges.
    }
    return Signed32(destination);
}

int32_t SelectAudioBank8002F13C(Services& s, uint32_t bank, uint32_t program) {
    if (static_cast<uint16_t>(bank) >= 16u) return -1;
    const uint32_t index = Extend16(bank);
    if (s.Read8(0x800928F8u + index) != 1u) return -1;
    const int32_t limit = Signed16(s.Read16(0x800917A8u));
    const int32_t signedProgram = Signed16(static_cast<uint16_t>(program));
    if (signedProgram >= limit) return -1;
    const uint32_t header = s.Read32(0x80091648u + 4u * index);
    const uint32_t programs = s.Read32(0x800901F8u + 4u * index);
    const uint32_t tones = s.Read32(0x80091690u + 4u * index);
    s.Write8(0x800928D9u, static_cast<uint8_t>(bank));
    s.Write8(0x800928DEu, static_cast<uint8_t>(program));
    s.Write32(0x800917DCu, tones);
    s.Write32(0x800917D8u, header);
    s.Write32(0x800917D0u, programs);
    s.Write8(0x800928DFu, s.Read8(programs + 16u * U(signedProgram) + 8u));
    return 0;
}

namespace {
// Shared instruction sequence in 345E4 and 351B8. The caller has written
// 928F2; both routines read it back before deriving masks and voice addresses.
void MarkVoiceOff(Services& s) {
    const uint32_t voice = s.Read16(0x800928F2u);
    const uint32_t low = voice < 16u ? 1u << (voice & 31u) : 0u;
    const uint32_t high = voice >= 16u ? 1u << ((voice - 16u) & 31u) : 0u;
    const uint32_t offset = 52u * voice;
    s.Write8(0x80087D5Bu + offset, 0u);
    const uint16_t oldLow = s.Read16(0x801C386Cu);
    const uint16_t oldHigh = s.Read16(0x801C386Eu);
    s.Write16(0x80087D44u + offset, 0u);
    s.Write16(0x80087D40u + offset, 0u);
    const uint16_t activeLow = s.Read16(0x8008EC98u);
    s.Write16(0x801C386Cu, static_cast<uint16_t>(low | oldLow));
    s.Write16(0x8008EC98u, static_cast<uint16_t>(activeLow & ~(low | oldLow)));
    const uint16_t activeHigh = s.Read16(0x8008EC9Au);
    s.Write16(0x801C386Eu, static_cast<uint16_t>(high | oldHigh));
    s.Write16(0x8008EC9Au, static_cast<uint16_t>(activeHigh & ~(high | oldHigh)));
}
}

int32_t StopVoice800345E4(Services& s, uint32_t voice, uint32_t bank,
                        uint32_t program, uint32_t note, uint32_t key) {
    if (s.Read32(0x800917A4u) == 1u) return -1;
    s.Write32(0x800917A4u, 1u);
    const uint32_t offset = 52u * Extend16(voice);
    if (static_cast<uint16_t>(voice) >= 24u ||
        s.Read16(0x80087D56u + offset) != static_cast<uint16_t>(bank) ||
        s.Read16(0x80087D52u + offset) != static_cast<uint16_t>(program) ||
        s.Read16(0x80087D54u + offset) != static_cast<uint16_t>(note) ||
        s.Read16(0x80087D4Cu + offset) != static_cast<uint16_t>(key)) {
        s.Write32(0x800917A4u, 0u);
        return -1;
    }
    if (s.Read16(0x80087D40u + offset) == 255u) {
        const uint32_t byteOffset = 52u * static_cast<uint8_t>(voice);
        s.Write8(0x80087D5Bu + byteOffset, 0u);
        const uint32_t mmio = s.Read32(0x80055DD8u);
        s.Write16(0x80087D44u + byteOffset, 0u);
        s.Write16(mmio + 0x194u, 0u);
        s.Write16(mmio + 0x196u, 0u);
    } else {
        s.Write16(0x800928F2u, static_cast<uint16_t>(voice));
        MarkVoiceOff(s);
    }
    s.Write32(0x800917A4u, 0u);
    return 0;
}

int32_t UpdateRegisterMask8003540C(Services& s, uint32_t mode, uint32_t mask,
    uint32_t lowRegister, uint32_t highRegister) {
    const uint32_t base = s.Read32(0x800555C8u);
    const uint32_t highAddress = base + highRegister * 2u;
    const uint32_t lowAddress = base + lowRegister * 2u;
    const uint32_t high = s.Read16(highAddress);
    const uint32_t low = s.Read16(lowAddress);
    uint32_t result = low | ((high & 0xFFu) << 16u);
    if (mode == 0u) {
        result &= ~(mask & 0xFFFFFFu);
        s.Write16(lowAddress, static_cast<uint16_t>(s.Read16(lowAddress) & ~mask));
        s.Write16(highAddress, static_cast<uint16_t>(s.Read16(highAddress) & ~((mask >> 16u) & 0xFFu)));
    } else if (mode == 1u) {
        result |= mask & 0xFFFFFFu;
        s.Write16(lowAddress, static_cast<uint16_t>(s.Read16(lowAddress) | mask));
        s.Write16(highAddress, static_cast<uint16_t>(s.Read16(highAddress) | ((mask >> 16u) & 0xFFu)));
    }
    return static_cast<int32_t>(result & 0xFFFFFFu);
}

int32_t SetNoiseMask800353E8(Services& s, uint32_t mode, uint32_t mask) {
    return UpdateRegisterMask8003540C(s, mode, mask, 0xCAu, 0xCBu);
}

int32_t AllocateVoice80030544(Services& s) {
    uint32_t selected = 99u, candidate = 99u, matches = 0u;
    uint16_t completion = 0xFFFFu;
    int32_t age = 0;
    const uint32_t initialCount = s.Read8(0x800928A0u);
    uint16_t priority = s.Read8(0x800928E7u);
    if (initialCount != 0u) {
        uint32_t voice = 0u;
        do {
            const uint32_t base = 0x80087D40u + 52u * voice;
            if (s.Read8(base + 27u) == 0u && s.Read16(base + 6u) == 0u) {
                selected = voice;
                break;
            }
            const int32_t currentPriority = Signed16(s.Read16(base + 24u));
            if (currentPriority < static_cast<int32_t>(priority)) {
                priority = static_cast<uint16_t>(currentPriority);
                candidate = voice;
                completion = s.Read16(base + 6u);
                age = s.Read16(base + 2u);
                matches = 1u;
            } else if (currentPriority == static_cast<int32_t>(priority)) {
                const uint16_t currentCompletion = s.Read16(base + 6u);
                ++matches;
                if (currentCompletion < completion) {
                    age = s.Read16(base + 2u);
                    completion = currentCompletion;
                    candidate = voice;
                } else if (currentCompletion == completion) {
                    const int32_t currentAge = Signed16(s.Read16(base + 2u));
                    if (age < currentAge) { age = currentAge; candidate = voice; }
                }
            }
            voice = (voice + 1u) & 0xFFu;
        } while (voice < s.Read8(0x800928A0u));
    }
    if (selected == 99u) {
        selected = candidate;
        if ((matches & 0xFFu) == 0u) selected = s.Read8(0x800928A0u);
    }
    const uint32_t count = s.Read8(0x800928A0u);
    if (selected >= count) return static_cast<int32_t>(selected);
    for (uint32_t voice = 0; voice < count; ++voice) {
        const uint32_t address = 0x80087D42u + 52u * voice;
        s.Write16(address, static_cast<uint16_t>(s.Read16(address) + 1u));
    }
    const uint32_t base = 0x80087D40u + 52u * selected;
    s.Write16(base + 2u, 0u);
    s.Write16(base + 24u, s.Read8(0x800928E7u));
    if (s.Read8(base + 27u) == 2u) SetNoiseMask800353E8(s, 0u, 0xFFFFFFu);
    return static_cast<int32_t>(selected);
}

int32_t PrepareVoiceRegisters80030C90(Services& s) {
    const uint32_t voice = Extend16(s.Read16(0x800928F2u));
    s.Write16(0x800928F4u, static_cast<uint16_t>(voice * 8u));
    const uint32_t program = s.Read8(0x800928DFu);
    const uint32_t note = s.Read8(0x800928E4u);
    s.Write16(0x800928F6u, static_cast<uint16_t>(note + 16u * program));
    s.Write16(0x80087D46u + 52u * voice, 0x7FFFu);
    for (uint32_t i = 0; i < 16u; ++i) {
        const uint32_t bit = 1u << (s.Read16(0x800928F2u) & 31u);
        const uint32_t address = 0x80088224u + 4u * i;
        s.Write32(address, s.Read32(address) & ~bit);
    }
    const uint16_t sample = s.Read16(0x800928F0u);
    const int32_t pair = (Signed16(sample) - 1) / 2;
    const uint32_t programs = s.Read32(0x800917D0u);
    uint32_t offset = Extend16(s.Read16(0x800928F4u)) * 2u;
    const uint16_t start = s.Read16(programs + U(pair) * 16u + ((sample & 1u) ? 12u : 14u));
    s.Write16(0x80087BAEu + offset, start);
    uint32_t dirty = 0x80087D28u + Extend16(s.Read16(0x800928F2u));
    s.Write8(dirty, static_cast<uint8_t>(s.Read8(dirty) | 8u));
    uint32_t toneProgram = s.Read8(0x800928DFu);
    uint32_t toneNote = s.Read8(0x800928E4u);
    const uint32_t tones = s.Read32(0x800917DCu);
    offset = Extend16(s.Read16(0x800928F4u)) * 2u;
    s.Write16(0x80087BB0u + offset, s.Read16(tones + 32u * (16u * toneProgram + toneNote) + 16u));
    toneProgram = s.Read8(0x800928DFu);
    toneNote = s.Read8(0x800928E4u);
    offset = Extend16(s.Read16(0x800928F4u)) * 2u;
    const uint16_t adsr = s.Read16(tones + 32u * (16u * toneProgram + toneNote) + 18u);
    const uint16_t bias = s.Read16(0x80091688u);
    s.Write16(0x80087BB2u + offset, static_cast<uint16_t>(adsr + bias));
    dirty = 0x80087D28u + Extend16(s.Read16(0x800928F2u));
    const uint8_t result = static_cast<uint8_t>(s.Read8(dirty) | 0x30u);
    s.Write8(dirty, result);
    return result;
}

int32_t ComputePitch800315C8(Services& s, uint32_t key, uint32_t fine) {
    const uint32_t program = s.Read8(0x800928DFu);
    const uint32_t note = s.Read8(0x800928E4u);
    const uint32_t tone = s.Read32(0x800917DCu) + 32u * (note + 16u * program);
    const uint32_t tuning = s.Read8(tone + 5u);
    uint32_t fractional = ((fine & 0xFFFFu) + tuning) / 8u;
    uint32_t carry = 0u;
    if (fractional >= 16u) { carry = 1u; fractional -= 16u; }
    const uint32_t centre = s.Read8(tone + 4u);
    const int32_t semitone = Signed16(static_cast<uint16_t>(key + 60u - centre + carry));
    const int32_t octave = semitone / 12;
    const int32_t index = (semitone - 12 * octave) * 16 + Signed16(static_cast<uint16_t>(fractional));
    uint32_t pitch = s.Read16(0x80055DDCu + U(index) * 2u);
    const int32_t shift = Signed16(static_cast<uint16_t>(octave - 5));
    if (shift > 0) pitch <<= U(shift) & 31u;
    else if (shift < 0) pitch = Sra(pitch, U(-shift));
    return static_cast<int32_t>(pitch & 0xFFFFu);
}

namespace {
void ApplyAudioPan(uint32_t pan, uint32_t& left, uint32_t& right) {
    // Each product is the original 32-bit LO before unsigned division.
    if (pan < 64u) right = (right * pan) / 63u;
    else left = (left * (127u - pan)) / 63u;
}
void ApplyAudioReverb(Services& s, uint32_t flags, uint32_t low, uint32_t high) {
    if ((flags & 4u) != 0u) {
        const uint16_t oldLow = s.Read16(0x8008EC9Cu);
        const uint16_t oldHigh = s.Read16(0x8008EC9Eu);
        s.Write16(0x8008EC9Cu, static_cast<uint16_t>(oldLow | low));
        s.Write16(0x8008EC9Eu, static_cast<uint16_t>(oldHigh | high));
    } else {
        s.Write16(0x8008EC9Cu, static_cast<uint16_t>(s.Read16(0x8008EC9Cu) & ~low));
        s.Write16(0x8008EC9Eu, static_cast<uint16_t>(s.Read16(0x8008EC9Eu) & ~high));
    }
}
}

int32_t SetRegularVoice800307AC(Services& s, uint32_t mode, uint32_t pitch) {
    (void)mode; // A0 is overwritten before use in the original function.
    const uint32_t header = s.Read32(0x800917D8u);
    const uint32_t master = s.Read8(header + 24u);
    const uint32_t velocity = s.Read8(0x800928DCu);
    uint32_t volume = (velocity * (master * 0x3FFFu)) / 16129u;
    volume *= s.Read8(0x800928E2u);
    volume *= s.Read8(0x800928E5u);
    const uint32_t initialVoice = Extend16(s.Read16(0x800928F2u));
    volume /= 16129u;
    const uint16_t sequence = s.Read16(0x800928EEu);
    const uint32_t sequenceBase = s.Read32(0x80095D08u + 4u * (sequence & 0xFFu)) + 172u * (sequence >> 8u);
    uint32_t left = volume, right = volume;
    if (sequence != 33u) {
        left = (volume * s.Read16(sequenceBase + 116u)) / 127u;
        right = (volume * s.Read16(sequenceBase + 118u)) / 127u;
    }
    ApplyAudioPan(s.Read8(0x800928E6u), left, right);
    ApplyAudioPan(s.Read8(0x800928E3u), left, right);
    ApplyAudioPan(s.Read8(0x800928DDu), left, right);
    if (s.Read16(0x80091728u) == 1u) {
        if (left < right) left = right; else right = left;
    }
    const uint32_t offset = ((initialVoice * 8u) & 0xFFFFu) * 2u;
    s.Write16(0x80087BACu + offset, static_cast<uint16_t>(pitch));
    s.Write16(0x80087BA8u + offset, static_cast<uint16_t>((left * left) / 0x3FFFu));
    s.Write16(0x80087BAAu + offset, static_cast<uint16_t>((right * right) / 0x3FFFu));
    uint32_t voice = Extend16(s.Read16(0x800928F2u));
    s.Write8(0x80087D28u + voice, static_cast<uint8_t>(s.Read8(0x80087D28u + voice) | 7u));
    voice = Extend16(s.Read16(0x800928F2u));
    s.Write16(0x80087D44u + 52u * voice, static_cast<uint16_t>(pitch));
    voice = Extend16(s.Read16(0x800928F2u));
    s.Write8(0x80087D5Bu + 52u * voice, 1u);
    voice = Extend16(s.Read16(0x800928F2u));
    const uint32_t low = Signed32(voice) < 16 ? 1u << (voice & 31u) : 0u;
    const uint32_t high = Signed32(voice) < 16 ? 0u : 1u << ((voice - 16u) & 31u);
    ApplyAudioReverb(s, s.Read8(0x800928ECu), low, high);
    const uint32_t onLow = s.Read16(0x8008EC98u) | low;
    const uint32_t oldOnHigh = s.Read16(0x8008EC9Au);
    const uint32_t offLow = s.Read16(0x801C386Cu);
    s.Write16(0x8008EC98u, static_cast<uint16_t>(onLow));
    s.Write16(0x801C386Cu, static_cast<uint16_t>(offLow & ~onLow));
    const uint32_t offHigh = s.Read16(0x801C386Eu);
    const uint32_t onHigh = oldOnHigh | high;
    s.Write16(0x8008EC9Au, static_cast<uint16_t>(onHigh));
    const uint16_t result = static_cast<uint16_t>(offHigh & ~onHigh);
    s.Write16(0x801C386Eu, result);
    return result;
}

int32_t SetNoiseVoice80030EA4(Services& s, uint32_t voiceArgument) {
    const uint16_t sequence = s.Read16(0x800928EEu);
    const uint32_t sequenceBase = s.Read32(0x80095D08u + 4u * (sequence & 0xFFu)) + 172u * (sequence >> 8u);
    const uint32_t sequenceLeft = s.Read16(sequenceBase + 116u);
    const uint32_t programVolume = s.Read8(0x800928E2u);
    const uint32_t sequenceRight = s.Read16(sequenceBase + 118u);
    const uint32_t toneVolume = s.Read8(0x800928E5u);
    uint32_t left = (((129u * sequenceLeft * programVolume) / 127u) * toneVolume) / 127u;
    uint32_t right = (((129u * sequenceRight * programVolume) / 127u) * toneVolume) / 127u;
    const uint32_t oldBase = s.Read32(0x80055DD8u);
    const uint32_t oldControl = s.Read16(oldBase + 0x1AAu);
    ApplyAudioPan(s.Read8(0x800928E6u), left, right);
    ApplyAudioPan(s.Read8(0x800928E3u), left, right);
    ApplyAudioPan(s.Read8(0x800928DDu), left, right);
    if (s.Read16(0x80091728u) == 1u) {
        if (left < right) left = right; else right = left;
    }
    const uint32_t key = s.Read8(0x800928DAu);
    const uint32_t centre = s.Read8(0x800928E8u);
    const uint16_t control = static_cast<uint16_t>((oldControl & 0xC0FFu) | (((key - centre) & 0x3Fu) << 8u));
    const uint32_t base = s.Read32(0x80055DD8u);
    const uint32_t voice = voiceArgument & 0xFFu;
    s.Write16(base + 0x1AAu, control);
    s.Write16(0x80087BAAu + 16u * voice, static_cast<uint16_t>(right));
    const uint8_t dirty = s.Read8(0x80087D28u + voice);
    s.Write16(0x80087BA8u + 16u * voice, static_cast<uint16_t>(left));
    s.Write8(0x80087D28u + voice, static_cast<uint8_t>(dirty | 3u));
    const uint32_t low = voice < 16u ? 1u << (voice & 31u) : 0u;
    const uint32_t high = voice < 16u ? 0u : 1u << ((voice - 16u) & 31u);
    const uint32_t count = s.Read8(0x800928A0u);
    s.Write16(0x80087D44u + 52u * voice, 10u);
    if (count != 0u) {
        uint32_t i = 0u;
        do {
            const uint32_t address = 0x80087D5Bu + 52u * Extend16(i);
            s.Write8(address, s.Read8(address) & 1u);
            ++i;
        } while (Signed16(static_cast<uint16_t>(i)) < s.Read8(0x800928A0u));
    }
    s.Write8(0x80087D5Bu + 52u * voice, 2u);
    const uint32_t onLow = s.Read16(0x8008EC98u) | low;
    const uint32_t onHigh = s.Read16(0x8008EC9Au) | high;
    const uint32_t offLow = s.Read16(0x801C386Cu);
    s.Write16(0x8008EC98u, static_cast<uint16_t>(onLow));
    s.Write16(0x8008EC9Au, static_cast<uint16_t>(onHigh));
    s.Write16(0x801C386Cu, static_cast<uint16_t>(offLow & ~onLow));
    const uint32_t offHigh = s.Read16(0x801C386Eu);
    const uint32_t flags = s.Read8(0x800928ECu);
    s.Write16(0x801C386Eu, static_cast<uint16_t>(offHigh & ~onHigh));
    ApplyAudioReverb(s, flags, low, high);
    const uint32_t finalBase = s.Read32(0x80055DD8u);
    s.Write16(finalBase + 0x194u, static_cast<uint16_t>(low));
    s.Write16(finalBase + 0x196u, static_cast<uint16_t>(high));
    return Signed32(finalBase);
}

namespace {
bool AdvanceAudioRamp(Services& s, uint32_t ramp, int32_t& earlyResult) {
    if (s.Read16(ramp + 4u) != 0u) {
        const uint32_t counter = s.Read16(ramp + 6u);
        s.Write16(ramp + 6u, static_cast<uint16_t>(counter - 1u));
        earlyResult = Signed32(counter << 16u);
        if (earlyResult > 0) return false;
        s.Write16(ramp + 6u, s.Read16(ramp + 4u));
    }
    const uint16_t current = s.Read16(ramp + 8u);
    const uint16_t delta = s.Read16(ramp + 2u);
    const int32_t direction = Signed16(s.Read16(ramp + 2u));
    const uint16_t next = static_cast<uint16_t>(current + delta);
    s.Write16(ramp + 8u, next);
    if (direction != 0) {
        const int32_t target = Signed16(s.Read16(ramp + 10u));
        if ((direction > 0 && Signed16(next) >= target) ||
            (direction < 0 && Signed16(next) <= target)) {
            s.Write16(ramp + 8u, static_cast<uint16_t>(target));
            s.Write16(ramp, 0u);
        }
    }
    return true;
}

void ApplyRampPan16(uint32_t pan, uint32_t& left, uint32_t& right) {
    uint32_t& channel = pan < 64u ? right : left;
    uint32_t product = (channel & 0xFFFFu) * (pan < 64u ? pan : 127u - pan);
    if (Signed32(product) < 0) product += 63u;
    // The original uses logical SRL, including after its negative correction.
    channel = product >> 6u;
}

int32_t FinishAudioRamp(Services& s, uint32_t voice, uint32_t product,
    bool useCapturedPan, uint32_t capturedPan) {
    uint32_t volume = U(Signed32(product) / 16129);
    volume *= s.Read8(0x800928E2u);
    volume *= s.Read8(0x800928E5u);
    const uint32_t tonePan = s.Read8(0x800928E6u);
    volume /= 16129u;
    uint32_t left = volume, right = volume;
    if (tonePan < 64u) right = (volume * tonePan) >> 6u;
    else left = (volume * (127u - tonePan)) >> 6u;
    ApplyRampPan16(s.Read8(0x800928E3u), left, right);
    ApplyRampPan16(useCapturedPan ? capturedPan : s.Read8(0x800928DDu), left, right);
    if (s.Read16(0x80091728u) == 1u) {
        if ((left & 0xFFFFu) < (right & 0xFFFFu)) left = right; else right = left;
    }
    const uint32_t offset = Extend16(voice * 8u) * 2u;
    s.Write16(0x80087BAAu + offset, static_cast<uint16_t>(right));
    const uint8_t dirty = s.Read8(0x80087D28u + voice);
    s.Write16(0x80087BA8u + offset, static_cast<uint16_t>(left));
    const uint8_t result = static_cast<uint8_t>(dirty | 3u);
    s.Write8(0x80087D28u + voice, result);
    return result;
}
}

int32_t AdvanceVolumeRamp80031A28(Services& s, uint32_t voiceArgument) {
    const uint32_t voice = Extend16(voiceArgument);
    const uint32_t ramp = 0x80087D5Cu + 52u * voice;
    int32_t earlyResult = 0;
    if (!AdvanceAudioRamp(s, ramp, earlyResult)) return earlyResult;
    const uint16_t current = s.Read16(ramp + 8u);
    const uint32_t header = s.Read32(0x800917D8u);
    s.Write8(0x800928DCu, static_cast<uint8_t>(current));
    const uint32_t master = s.Read8(header + 24u);
    return FinishAudioRamp(s, voice, U(Signed16(current)) * (master * 0x3FFFu), false, 0u);
}

int32_t AdvancePanRamp80031F28(Services& s, uint32_t voiceArgument) {
    const uint32_t voice = Extend16(voiceArgument);
    const uint32_t ramp = 0x80087D68u + 52u * voice;
    int32_t earlyResult = 0;
    if (!AdvanceAudioRamp(s, ramp, earlyResult)) return earlyResult;
    const uint32_t pan = s.Read8(ramp + 8u);
    const uint32_t header = s.Read32(0x800917D8u);
    s.Write8(0x800928DDu, static_cast<uint8_t>(pan));
    const uint32_t master = s.Read8(header + 24u);
    const uint32_t velocity = s.Read8(0x800928DCu);
    return FinishAudioRamp(s, voice, velocity * (master * 0x3FFFu), true, pan);
}

int32_t CommitAudio80032B00(Services& s) {
    const uint32_t oldCursor = s.Read32(0x80088220u);
    const uint32_t count = s.Read8(0x800928A0u);
    const uint32_t cursor = (oldCursor + 1u) & 15u;
    s.Write32(0x80088220u, cursor);
    const uint32_t ring = 0x80088224u + 4u * cursor;
    s.Write32(ring, 0u);
    if (count != 0u) {
        const uint32_t base = s.Read32(0x80055DD8u);
        for (uint32_t voice = 0; voice < count; ++voice) {
            const uint32_t address = 0x80087D46u + 52u * voice;
            s.Write16(address, s.Read16(base + 16u * voice + 12u));
            if (s.Read16(address) == 0u) s.Write32(ring, s.Read32(ring) | (1u << (voice & 31u)));
        }
    }
    if (s.Read8(0x8009290Cu) == 0u) {
        uint32_t completed = 0xFFFFFFFFu;
        // Deliberately fifteen entries, not the whole sixteen-entry ring.
        for (uint32_t i = 0; i < 15u; ++i) completed &= s.Read32(0x80088224u + 4u * i);
        if (s.Read8(0x800928A0u) != 0u) {
            uint32_t voice = 0;
            do {
                if ((completed & (1u << (voice & 31u))) != 0u) {
                    const uint32_t status = 0x80087D5Bu + 52u * voice;
                    if (s.Read8(status) == 2u) SetNoiseMask800353E8(s, 0u, 0xFFFFFFu);
                    s.Write8(status, 0u);
                }
                const uint32_t currentCount = s.Read8(0x800928A0u);
                ++voice;
                if (voice >= currentCount) break;
            } while (true);
        }
    }
    const uint32_t offLow = s.Read16(0x801C386Cu);
    const uint32_t onLow = s.Read16(0x8008EC98u);
    const uint32_t offHigh = s.Read16(0x801C386Eu);
    s.Write16(0x8008EC98u, static_cast<uint16_t>(onLow & ~offLow));
    s.Write16(0x8008EC9Au, static_cast<uint16_t>(s.Read16(0x8008EC9Au) & ~offHigh));
    for (uint32_t voice = 0; voice < 24u; ++voice) {
        if (s.Read16(0x80087D5Cu + 52u * voice) != 0u) AdvanceVolumeRamp80031A28(s, voice);
        if (s.Read16(0x80087D68u + 52u * voice) != 0u) AdvancePanRamp80031F28(s, voice);
    }
    for (uint32_t voice = 0; voice < 24u; ++voice) {
        const uint32_t dirty = 0x80087D28u + voice, offset = voice * 16u;
        if ((s.Read8(dirty) & 1u) != 0u) {
            const uint32_t base = s.Read32(0x80055DD8u) + offset;
            s.Write16(base, s.Read16(0x80087BA8u + offset));
            s.Write16(base + 2u, s.Read16(0x80087BAAu + offset));
        }
        if ((s.Read8(dirty) & 4u) != 0u) {
            const uint32_t base = s.Read32(0x80055DD8u) + offset;
            s.Write16(base + 4u, s.Read16(0x80087BACu + offset));
        }
        if ((s.Read8(dirty) & 8u) != 0u) {
            const uint32_t base = s.Read32(0x80055DD8u) + offset;
            s.Write16(base + 6u, s.Read16(0x80087BAEu + offset));
        }
        if ((s.Read8(dirty) & 0x10u) != 0u) {
            const uint32_t base = s.Read32(0x80055DD8u) + offset;
            s.Write16(base + 8u, s.Read16(0x80087BB0u + offset));
            s.Write16(base + 10u, s.Read16(0x80087BB2u + offset));
        }
        s.Write8(dirty, 0u);
    }
    const uint32_t base = s.Read32(0x80055DD8u);
    const uint16_t finalOffLow = s.Read16(0x801C386Cu), finalOffHigh = s.Read16(0x801C386Eu);
    const uint16_t finalOnLow = s.Read16(0x8008EC98u), finalOnHigh = s.Read16(0x8008EC9Au);
    const uint16_t reverbLow = s.Read16(0x8008EC9Cu), reverbHigh = s.Read16(0x8008EC9Eu);
    s.Write16(0x801C386Cu, 0u); s.Write16(0x801C386Eu, 0u);
    s.Write16(0x8008EC98u, 0u); s.Write16(0x8008EC9Au, 0u);
    s.Write16(base + 0x18Cu, finalOffLow); s.Write16(base + 0x18Eu, finalOffHigh);
    s.Write16(base + 0x188u, finalOnLow); s.Write16(base + 0x18Au, finalOnHigh);
    s.Write16(base + 0x198u, reverbLow); s.Write16(base + 0x19Au, reverbHigh);
    return Signed32(base);
}

int32_t PlayVoice80034240(Services& s, uint32_t bank, uint32_t program,
    uint32_t note, uint32_t key, uint32_t fine, uint32_t left, uint32_t right) {
    if (s.Read32(0x800917A4u) == 1u) return -1;
    s.Write32(0x800917A4u, 1u);
    const auto reject = [&]() { s.Write32(0x800917A4u, 0u); return -1; };
    if (SelectAudioBank8002F13C(s, Extend16(bank), Extend16(program)) != 0) return reject();
    s.Write16(0x800928EEu, 33u);
    const int32_t l = Signed16(static_cast<uint16_t>(left));
    const int32_t r = Signed16(static_cast<uint16_t>(right));
    s.Write8(0x800928DAu, static_cast<uint8_t>(key));
    s.Write8(0x800928DBu, static_cast<uint8_t>(fine));
    s.Write8(0x800928E4u, static_cast<uint8_t>(note));
    if (l == r) {
        s.Write8(0x800928DDu, 64u);
        s.Write8(0x800928DCu, static_cast<uint8_t>(left));
    } else if (r < l) {
        if (l == 0) s.Break(0x80034334u, 7u);
        const int32_t pan = (r * 64) / l; // int16 operands cannot trigger INT_MIN/-1.
        s.Write8(0x800928DCu, static_cast<uint8_t>(left));
        s.Write8(0x800928DDu, static_cast<uint8_t>(pan));
    } else {
        if (r == 0) s.Break(0x8003437Cu, 7u);
        const int32_t pan = 127 - (l * 64) / r;
        s.Write8(0x800928DCu, static_cast<uint8_t>(right));
        s.Write8(0x800928DDu, static_cast<uint8_t>(pan));
    }
    const uint32_t programs = s.Read32(0x800917D0u);
    const uint32_t entry = programs + 16u * Extend16(program);
    s.Write8(0x800928E2u, s.Read8(entry + 1u));
    s.Write8(0x800928E3u, s.Read8(entry + 4u));
    const uint8_t toneCount = s.Read8(entry);
    const uint32_t toneProgram = s.Read8(0x800928DFu);
    s.Write8(0x800928D8u, toneCount);
    const uint32_t toneNote = s.Read8(0x800928E4u);
    const uint32_t tones = s.Read32(0x800917DCu);
    const uint32_t tone = tones + 32u * (toneNote + 16u * toneProgram);
    s.Write8(0x800928E7u, s.Read8(tone));
    const uint16_t sample = s.Read16(tone + 22u);
    s.Write16(0x800928F0u, sample);
    s.Write8(0x800928E5u, s.Read8(tone + 2u));
    s.Write8(0x800928E6u, s.Read8(tone + 3u));
    s.Write8(0x800928E8u, s.Read8(tone + 4u));
    s.Write8(0x800928E9u, s.Read8(tone + 5u));
    s.Write8(0x800928ECu, s.Read8(tone + 1u));
    s.Write8(0x800928EAu, s.Read8(tone + 6u));
    s.Write8(0x800928EBu, s.Read8(tone + 7u));
    if (sample == 0u) return reject();
    const uint32_t voice = U(AllocateVoice80030544(s)) & 0xFFu;
    if (voice == s.Read8(0x800928A0u)) return reject();
    const uint32_t offset = 52u * voice;
    s.Write16(0x800928F2u, static_cast<uint16_t>(voice));
    s.Write16(0x80087D4Eu + offset, 33u);
    s.Write16(0x80087D56u + offset, static_cast<uint16_t>(bank));
    const uint8_t selectedProgram = s.Read8(0x800928DFu);
    s.Write16(0x80087D52u + offset, static_cast<uint16_t>(program));
    s.Write16(0x80087D50u + offset, selectedProgram);
    s.Write16(0x80087D40u + offset, s.Read16(0x800928F0u));
    const uint8_t selectedNote = s.Read8(0x800928E4u);
    s.Write16(0x80087D4Cu + offset, static_cast<uint16_t>(key));
    s.Write8(0x80087D5Bu + offset, 1u);
    s.Write16(0x80087D42u + offset, 0u);
    s.Write16(0x80087D54u + offset, selectedNote);
    PrepareVoiceRegisters80030C90(s);
    if (s.Read16(0x800928F0u) == 255u) {
        SetNoiseVoice80030EA4(s, voice);
    } else {
        const uint32_t pitch = U(ComputePitch800315C8(s, key & 0xFFFFu, fine & 0xFFFFu));
        SetRegularVoice800307AC(s, 1u, pitch & 0xFFFFu);
    }
    s.Write32(0x800917A4u, 0u);
    return static_cast<int32_t>(voice);
}

void ReplaceCue80026FC4(Services& s, uint32_t row) {
    if (row == 0u) return;
    const uint8_t oldKey = s.Read8(0x800943B0u);
    const uint32_t oldVoice = Extend16(s.Read16(0x800943AAu));
    const uint32_t oldBank = Extend16(s.Read16(0x800943A8u));
    const uint8_t oldProgram = s.Read8(0x800943AEu);
    const uint8_t oldNote = s.Read8(0x800943AFu);
    StopVoice800345E4(s, oldVoice, oldBank, oldProgram, oldNote, oldKey);
    const uint8_t program = s.Read8(row);
    const uint8_t key = static_cast<uint8_t>(s.Read8(row + 1u) + 24u);
    const uint8_t note = s.Read8(row + 1u);
    s.Write8(row + 2u, key);
    const uint8_t left = s.Read8(row + 3u);
    const uint32_t bank = Extend16(s.Read16(0x800943A8u));
    const uint8_t right = s.Read8(row + 3u);
    const int32_t voice = PlayVoice80034240(s, bank, program, note, key, 0u, left, right);
    s.Write16(0x800943AAu, static_cast<uint16_t>(voice));
    CopyBytes80025C64(s, row, 0x800943AEu, 6);
}

int32_t PlayCue80026EF8(Services& s, uint32_t row) {
    const uint8_t key = static_cast<uint8_t>(s.Read8(row + 1u) + 24u);
    const uint8_t program = s.Read8(row);
    const uint8_t note = s.Read8(row + 1u);
    s.Write8(row + 2u, key);
    const uint8_t left = s.Read8(row + 3u);
    const uint8_t right = s.Read8(row + 3u);
    const uint32_t bank = Extend16(s.Read16(0x800943A8u));
    const int32_t voice = PlayVoice80034240(s, bank, program, note, key, 0u, left, right);
    s.Write16(0x800943ACu, static_cast<uint16_t>(voice));
    return voice;
}

int32_t ResetVoices800351B8(Services& s) {
    uint32_t count = s.Read8(0x800928A0u);
    for (uint32_t voice = 0u; Signed16(static_cast<uint16_t>(voice)) < static_cast<int32_t>(count); ++voice) {
        const uint32_t offset = 52u * Extend16(voice);
        s.Write16(0x80087D42u + offset, 24u);
        s.Write16(0x80087D40u + offset, 255u);
        s.Write8(0x80087D5Bu + offset, 0u);
        s.Write16(0x80087D44u + offset, 0u);
        s.Write16(0x80087D46u + offset, 0u);
        s.Write16(0x80087D4Eu + offset, 255u);
        s.Write16(0x80087D50u + offset, 0u);
        s.Write16(0x80087D52u + offset, 0u);
        s.Write16(0x80087D54u + offset, 255u);
        const uint32_t mmio = s.Read32(0x80055DD8u) + Sra(Extend16(voice) << 19u, 15u);
        s.Write16(mmio + 6u, 512u);
        s.Write16(mmio + 4u, 4096u);
        s.Write16(mmio + 8u, 0x80FFu);
        s.Write16(mmio, 0u);
        s.Write16(mmio + 2u, 0u);
        s.Write16(mmio + 10u, 0x4000u);
        s.Write16(0x800928F2u, static_cast<uint16_t>(voice));
        MarkVoiceOff(s);
        count = s.Read8(0x800928A0u); // Mutable voice count is re-read every iteration.
    }
    return static_cast<int32_t>(count);
}

int32_t ResetAudio80026FA4(Services& s) { return ResetVoices800351B8(s); }

int32_t FlushAudioDriver8002EFF4(Services& s) {
    const uint32_t busy = s.Read32(0x800917A4u);
    if (busy == 1u) return 1;
    s.Write32(0x800917A4u, 1u);
    const int32_t result = CommitAudio80032B00(s);
    s.Write32(0x800917A4u, 0u);
    return result;
}

int32_t FlushAudio80026ECC(Services& s) {
    const uint32_t busy = s.Read32(0x800943B4u);
    return busy != 0u ? Signed32(busy) : FlushAudioDriver8002EFF4(s);
}

int32_t ApplyInputRow801C85CC(Services& s, uint32_t work, uint32_t row) {
    s.Write32(work, s.Read32(work) | 0x10u);
    uint32_t index = Extend16(s.Read16(row));
    uint32_t resource = Extend16(s.Read16(0x801CC4B4u + 4u * index));
    s.Write32(work + 220u, Resource(s, resource));
    // 85CC reads the row again after the first resource write. Unlike 7BC0,
    // it neither caches both pair indices nor rejects a zero resource index.
    index = Extend16(s.Read16(row));
    resource = Extend16(s.Read16(0x801CC4B6u + 4u * index));
    s.Write32(work + 232u, Resource(s, resource));
    s.Write16(work + 286u, s.Read16(row + 2u));
    s.Write16(0x8008ECFCu, s.Read16(row + 6u));
    s.Write16(work + 288u, s.Read16(row + 4u));
    if (s.Read32(row + 12u) == 1u) {
        s.Write8(work + 377u, 1u);
        s.Write16(work + 380u, s.Read16(row + 16u));
    }
    if (s.Read32(row + 20u) == 1u) {
        s.Write8(work + 378u, 1u);
        s.Write16(work + 382u, s.Read16(row + 24u));
    }
    const uint32_t queueId = Extend16(s.Read16(row + 8u));
    if (queueId == 0u) return 0;
    const uint32_t tick = s.Read32(work + 12u);
    const uint32_t queue = 0x801CD980u + 12u * queueId;
    s.Write32(0x8008ED44u, queueId);
    s.Write32(0x8008ED4Cu, queue);
    s.Write32(0x8008ED48u, tick);
    s.Write32(queue + 8u, 0u);
    return Signed32(queue);
}

int32_t DecodeInput80024B54(uint32_t pad) {
    if ((pad & 0x10u) != 0u) return 1;
    if ((pad & 0x20u) != 0u) return 2;
    if ((pad & 0x40u) != 0u) return 3;
    if ((pad & 0x80u) != 0u) return 4;
    if ((pad & 4u) != 0u) return 5;
    if ((pad & 1u) != 0u) return 5;
    if ((pad & 8u) != 0u) return 7;
    if ((pad & 2u) != 0u) return 7;
    return 0;
}

int32_t IsJudgeBlocked80024BF4(Services& s, uint32_t work) {
    const uint32_t mode = s.Read32(0x8008ED00u);
    if (mode == 1u) {
        if (s.Read32(0x8008ED14u) == 0u) return 0;
        const int32_t expiry = Signed32(s.Read32(0x8008ED0Cu));
        const int32_t tick = Signed32(s.Read32(work + 12u));
        if (expiry < tick) {
            s.Write32(0x8008ED14u, 0u);
            return 0;
        }
        return 1;
    }
    if (s.Read16(0x8008ED2Cu) != 0u) return 1;
    if (s.Read32(0x8008ED14u) == 0u) return 0;
    return mode == 5u ? 1 : 0;
}

int32_t JudgeInput80014614(Services& s, uint32_t work) {
    // SCUS startup 80028610/14 establishes GP=8006EA40. These identities
    // are the original GP+1C/+20 globals, not invented host scratch storage.
    constexpr uint32_t kGp = 0x8006EA40u;
    if (IsJudgeBlocked80024BF4(s, work) == 1) return -1;
    if (s.Read16(0x8008ED2Eu) == 0u) return -2;
    if (s.Read32(work + 68u) == 0u) return -3;
    const auto remap = [](uint32_t pad) { return pad == 1u ? 4u : pad == 2u ? 8u : pad; };
    const uint32_t originalPad = remap(s.Read32(work + 24u));
    const uint32_t eventIndex = Extend16(s.Read16(work + 80u));
    const uint32_t event = s.Read32(work + 68u) + 6u * eventIndex;
    const uint32_t cue = s.Read8(event + 12u);
    if (cue == 0u) return -5;
    const uint32_t table = s.Read32(0x800943D8u);
    const uint32_t input = s.Read32(work + 32u);
    const uint32_t descriptor = s.Read32(table + 36u * cue + 4u * input);
    if (descriptor == 0u) return -6;
    const uint32_t rhythm = s.Read8(event + 13u);
    if (rhythm == 0u) return -7;
    const uint32_t tick = s.Read32(work + 16u);
    const uint32_t window = s.Read32(work + 52u);
    const int32_t remainder = Signed32(tick + window) % 384;
    const int32_t subdivision = remainder / 24;
    const bool close = Signed32(2u * window) >= remainder % 24;
    const uint32_t timing = s.Read32(0x800943E0u);
    const uint32_t classification = s.Read8(timing + 16u * rhythm + U(subdivision / 2));
    if (classification == 0u) return -8;
    const uint32_t previous = s.Read32(0x80091800u);
    const uint32_t current = s.Read32(work + 32u);
    if (previous == current || current == 0u) {
        s.Write32(kGp + 32u, 0u);
    } else {
        s.Write16(descriptor + 6u, 0u);
        s.Write32(kGp + 32u, 1u);
    }
    const uint32_t nextInput = s.Read32(work + 32u);
    const uint32_t changed = s.Read32(kGp + 32u);
    s.Write32(0x80091800u, nextInput);
    if (changed == 0u) {
        const int32_t count = Signed16(s.Read16(descriptor + 4u));
        if (count >= 2 && (originalPad & 0x2000u) != 0u) {
            const int32_t cursor = Signed16(s.Read16(descriptor + 6u));
            s.Write16(descriptor + 6u, static_cast<uint16_t>(cursor <= 0 ? count - 1 : cursor - 1));
        }
    }
    if ((originalPad & 0x8000u) != 0u) s.Write16(descriptor + 6u, 0u);
    const int32_t cursor = Signed16(s.Read16(descriptor + 6u));
    const uint32_t rows = s.Read32(descriptor);
    const int32_t count = Signed16(s.Read16(descriptor + 4u));
    const uint32_t selected = rows + 12u * U(cursor);
    if (count >= 2) {
        const int32_t next = cursor + 1;
        const int32_t divisor = Signed16(s.Read16(descriptor + 4u));
        if (divisor == 0) s.Break(0x80014894u, 7u);
        // The dividend is sign-extended int16: the original INT_MIN/-1
        // BREAK 6 guard is unreachable for every possible row cursor.
        const int32_t wrapped = Signed16(static_cast<uint16_t>(next)) % divisor;
        s.Write16(descriptor + 6u, static_cast<uint16_t>(next));
        s.Write16(descriptor + 6u, static_cast<uint16_t>(wrapped));
    }
    ReplaceCue80026FC4(s, selected);
    const uint32_t callbackRow = s.Read32(selected + 8u);
    if (callbackRow != 0u) {
        const uint32_t callback = s.Read32(0x80094430u);
        if (callback == 0x801C85CCu) ApplyInputRow801C85CC(s, work, callbackRow);
        else s.Call(callback, {work, callbackRow}); // A1 is visible at 800148C4/JALR, omitted by Hex-Rays.
    }
    if (classification != 2u) return -9;
    const uint32_t pad = remap(s.Read32(work + 24u));
    if (s.Read16(work + 82u) == 0u) {
        const uint32_t replayIndex = s.Read32(0x800901C0u);
        const uint32_t replayTick = s.Read32(work + 16u);
        if (Signed32(replayIndex) < 600) {
            s.Write32(0x8008EEFCu + 8u * replayIndex, pad);
            s.Write32(0x8008EEF8u + 8u * s.Read32(0x800901C0u), replayTick);
            const uint32_t next = s.Read32(0x800901C0u) + 1u;
            s.Write32(0x800901C0u, next);
            s.Write32(0x800901BCu, next);
        }
    }
    if (close) {
        const uint32_t offset = 12u * U(subdivision);
        s.Write32(s.Read32(kGp + 28u) + offset, pad);
        const int32_t decoded = DecodeInput80024B54(pad);
        const uint32_t packet = s.Read32(kGp + 28u) + offset;
        const uint16_t countBeforeWrite = s.Read16(packet + 6u);
        s.Write16(packet + 4u, static_cast<uint16_t>(decoded));
        s.Write32(packet + 8u, selected);
        s.Write16(packet + 6u, static_cast<uint16_t>(countBeforeWrite + 1u));
        s.Write16(0x80091812u, static_cast<uint16_t>(s.Read16(0x80091812u) + 1u));
    } else {
        s.Write16(0x80091814u, static_cast<uint16_t>(s.Read16(0x80091814u) + 1u));
    }
    const uint16_t total = s.Read16(0x80091810u);
    const uint32_t mask = s.Read32(0x80091808u);
    s.Write16(0x80091810u, static_cast<uint16_t>(total + 1u));
    s.Write32(0x80091808u, mask | pad);
    if (close) s.Write16(0x80091824u, s.Read16(selected + 6u));
    return 0;
}

int32_t InputFeedback801C9644(Services& s, uint32_t work) {
    if (s.Read16(work + 78u) == 0u) return 0;
    const uint32_t current = s.Read32(work + 68u);
    const uint32_t currentMask = current != 0u ? s.Read32(current + 8u) : 0u;
    const uint32_t previous = s.Read32(work + 64u);
    const uint32_t previousMask = previous != 0u ? s.Read32(previous + 8u) : 0u;
    const uint32_t pad = s.Read32(work + 24u);
    // Return values include the original branch delay slots.
    if ((pad & currentMask) != 0u) return Signed32(pad & previousMask);
    if ((pad & previousMask) != 0u) return 0x20000000;
    const uint32_t flags = s.Read32(work) | 0x20000000u;
    s.Write32(work, flags);
    s.Write32(work, flags | 0x10000u);
    const uint32_t resource = s.Read32(0x80091AE4u);
    const uint32_t tick = s.Read32(work + 12u);
    s.Write32(work + 220u, resource);
    s.Write32(work + 232u, s.Read32(0x80091A44u));
    s.Write32(0x8008ED44u, 11u);
    s.Write32(0x8008ED4Cu, 0x801CDA04u);
    s.Write32(0x8008ED48u, tick);
    s.Write32(0x801CDA0Cu, 0u);
    s.Write16(work + 288u, 17u);
    s.Write16(0x8008ECFCu, 17u);
    s.Write16(work + 286u, 1u);
    return 1;
}

int32_t SpecialPose801C9730(Services& s, uint32_t work) {
    const uint32_t pad = s.Read32(work + 24u);
    const uint32_t time = s.Read16(work + 4u);
    if ((pad & 0x1000u) == 0u) return 0;
    const int32_t cooldown = Signed32(s.Read32(0x801D299Cu));
    if (cooldown > 0) return cooldown;
    if (s.Read16(work + 78u) != 0u) {
        if (time != 20u && time != 24u) return 24;
    } else if (time - 28u < 11u) {
        return 1;
    }
    const uint32_t currentPad = s.Read32(work + 24u);
    if ((currentPad & 0xAu) != 0u) {
        s.Write8(work + 373u, 68u);
        s.Write8(work + 372u, 0u);
    } else {
        if ((currentPad & 5u) == 0u) return 68;
        s.Write8(work + 372u, 68u);
        s.Write8(work + 373u, 0u);
    }
    s.Write32(0x801D299Cu, 141u);
    const uint32_t result = s.Read32(work) | 0x08000000u;
    s.Write32(work, result);
    return Signed32(result);
}

int32_t InitDemo801C7958(Services& s) {
    if (Signed32(s.Read32(0x801D2178u)) > 0) {
        uint32_t index = 0u;
        do {
            s.Write32(0x8008EEF8u + 8u * index, 24u * U(Signed16(s.Read16(0x801D2010u + 4u * index))));
            const int32_t pad = s.Call(0x80024BC0u, {U(Signed16(s.Read16(0x801D2012u + 4u * index)))});
            s.Write32(0x8008EEFCu + 8u * index, U(pad));
            ++index;
            // Mutable count is reloaded after each decoder call; not constant 90.
            if (Signed32(index) >= Signed32(s.Read32(0x801D2178u))) break;
        } while (true);
    }
    const uint32_t count = s.Read32(0x801D2178u);
    s.Write32(0x800901BCu, count);
    return Signed32(count);
}

int32_t InitGame801C6CDC(Services& s, uint32_t record, uint32_t work) {
    s.Write32(0x801D2984u, 0u);
    ResetAudio80026FA4(s);
    InitResources801CB284(s);
    s.CallVoid(0x8001A478u, {U(Signed16(s.Read16(record + 6u)))});
    s.Call(0x8001A654u, {U(Signed16(s.Read16(record + 4u)))});
    s.Call(0x80014344u, {});
    s.Call(0x80024E98u, {});
    s.Call(0x80024FC0u, {work});
    return ClearEntries801C78D4(s);
}

int32_t RunMovie801C6AB8(Services& s, uint32_t record, uint32_t work, uint32_t kind) {
    s.CallVoid(0x80024C84u, {kind < 4u ? s.Read32(0x800943CCu) + 28u * kind : 0u});
    s.Write32(0x801D2984u, 0u);
    for (int32_t remaining = 1800; remaining > 0; --remaining) {
        if (s.Call(0x8001A750u, {}) == 1) break;
        s.Call(0x8001EC54u, {work, 7u});
        s.Call(0x80035560u, {2u});
        s.Call(0x8001ED3Cu, {work});
    }
    s.Call(0x8001A280u, {});
    s.Write32(work + 12u, s.Read32(s.Read32(kSceneEntry) + 356u));
    s.Write16(work + 102u, s.Read16(0x800916D8u));
    const uint16_t subtitle = s.Read16(0x800916DCu);
    s.Write16(work + 104u, subtitle);
    s.Write16(work + 84u, subtitle);
    int32_t result = 0;
    for (;;) {
        s.Write32(work, 0u);
        const uint32_t pad = U(s.Call(0x80035510u, {1u}));
        if (pad == 256u) { result = 1; break; }
        if (pad == 2048u || (pad & 0x840u) != 0u) break;
        s.Call(0x80024CF8u, {work});
        s.Call(0x80027528u, {});
        // Hex-Rays omits these branches after folding 801D298C. Preserve
        // the actual LW/BNE at 801C6C14 and 801C6C54, not its initial value.
        if (s.Read32(0x801D298Cu) == 1u) s.Call(0x8001EC54u, {work, 7u});
        s.Call(0x80027528u, {});
        const int32_t frame = MovieFrame801C6804(s, record, work);
        s.Call(0x8001ED74u, {});
        s.Call(0x8002756Cu, {});
        if (s.Read32(0x801D298Cu) == 1u) s.Call(0x8001ED3Cu, {work});
        if (s.Call(0x8001A3B8u, {}) == 1) {
            s.Call(0x8001A694u, {});
            s.Call(0x80035838u, {});
            break;
        }
        if (frame != 1) break;
    }
    s.Call(0x80027664u, {});
    s.CallVoid(0x8001A4A4u, {1u});
    s.Call(0x8001A694u, {});
    s.CallVoid(0x80024CF0u, {work});
    s.Call(0x8001B120u, {1u});
    return result;
}

int32_t RunGame801C6D58(Services& s, uint32_t record, uint32_t work, int32_t scene) {
    s.ObserveGameBoundary(0u,work);
    InitGame801C6CDC(s, record, work);
    s.ObserveGameBoundary(1u,work);
    const int32_t mode = Signed16(s.Read16(kModeD0));
    int32_t bank = 0;
    if (mode == 1 || mode == 2) {
        const int32_t easy = Signed16(s.Read16(kEasyDA));
        s.Write16(kEasyDA, 0u);
        s.Write32(0x801D2988u, U(easy));
        s.CallVoid(0x80024E54u, {0u});
        s.Write16(work + 82u, 1u);
        if (mode == 1) {
            const uint32_t initialize = s.Read32(0x80094440u);
            if (initialize == 0x801C7958u) InitDemo801C7958(s);
            else s.Call(initialize, {});
        } else {
            s.Call(0x8001681Cu, {});
            bank = s.Call(0x80016758u, {U(scene)});
        }
    } else {
        s.Write16(work + 82u, 0u);
        s.CallVoid(0x80024E54u, {0u});
        bank = s.Call(0x8001670Cu, {U(scene)});
    }
    s.CallVoid(0x800143F0u, {U(bank)});
    s.CallVoid(0x800259C0u, {U(bank)});
    const int32_t window = s.Read16(kEasyDA) != 0u ? 24
        : Signed32(s.Read32(s.Read32(kSceneEntry) + 360u)) / 2;
    s.Write32(work + 52u, U(window));
    const uint16_t option = s.Read16(0x800916F6u);
    s.Write16(work + 110u, option);
    if (option == 1u) s.Write16(work + 108u, 1u);
    if (s.Read16(kEasyDA) == 1u) {
        s.CallVoid(0x800143F0u, {0u});
        s.CallVoid(0x800259C0u, {0u});
    }
    s.Write32(0x801D2984u, 0u);
    bool observedPreLoop = false;
    uint32_t preLoopFrames = 0u;
    for (int32_t remaining = 1800; remaining > 0; --remaining) {
        const uint32_t ready = U(s.Call(0x8001A750u, {}));
        if (ready == 1u) {
            s.ObserveGamePreLoop(preLoopFrames, work, ready);
            break;
        }
        if (!observedPreLoop) {
            observedPreLoop = true;
            s.ObserveGameBoundary(2u,work);
        }
        PrepareFrame801CA57C(s, work, 7);
        // Verified main-program callee consumes no arguments, not stale a0.
        if (s.Call(0x8001F518u, {}) != 0) s.Call(0x80027194u, {30u});
        s.Call(0x80035560u, {2u});
        Present801CB170(s);
        ++preLoopFrames;
        s.ObserveGamePreLoop(preLoopFrames, work, 0u);
        s.GameStartupFramePresented();
    }
    if (!observedPreLoop)
        s.ObserveGameBoundary(2u, work);
    s.ObserveGameBoundary(3u, work);
    s.GameStartupPreLoopEnded();
    s.Call(0x8001A280u, {});
    s.Write32(work + 12u, s.Read32(s.Read32(kSceneEntry) + 356u));
    s.Call(0x800357D4u, {0x801C6858u});
    s.Write16(work + 96u, 1u);
    s.Write16(work + 100u, 1u);
    const uint16_t language = s.Read16(0x800916D8u);
    const uint32_t initialTick = s.Read32(work + 12u);
    s.Write16(work + 102u, language);
    const uint16_t subtitle = s.Read16(0x800916DCu);
    s.Write32(0x801CBA30u, 0u);
    s.Write32(0x801D2980u, initialTick);
    s.Write16(work + 104u, subtitle);
    s.Write16(work + 84u, subtitle);

    const auto remapPad = [](uint32_t pad) {
        if ((pad & 1u) != 0u) return (pad & ~1u) | 4u;
        if ((pad & 2u) != 0u) return (pad & ~2u) | 8u;
        return pad;
    };
    const auto mapInput = [&](uint32_t pad) {
        const int32_t mapped = DecodeInput80024B54(pad);
        const uint32_t flags = s.Read32(work);
        s.Write32(work + 32u, U(mapped));
        s.Write32(work, flags | 1u);
    };
    for (;;) {
        PrepareFrame801CA57C(s, work, 7);
        const uint16_t replay = s.Read16(work + 82u);
        s.Write32(work, 0u);
        if (replay == 1u) {
            const uint32_t index = s.Read32(0x800901C0u);
            const int32_t count = Signed32(s.Read32(0x800901BCu));
            const uint32_t row = 0x8008EEF8u + 8u * index;
            if (Signed32(index) < count &&
                Signed32(s.Read32(work + 12u)) >= Signed32(s.Read32(row))) {
                s.Write32(0x800901C0u, index + 1u);
                const int32_t tick = Signed32(s.Read32(row));
                const uint32_t pad = remapPad(s.Read32(row + 4u));
                s.Write32(work + 24u, pad);
                mapInput(pad); // A due replay row is mapped even when pad == 0.
                s.Write32(work + 16u, U(tick));
                // Native MULT/division precedes narrowing to SH/SB.
                s.Write16(work + 8u, static_cast<uint16_t>(tick / 384 + 1));
                s.Write8(work + 10u, static_cast<uint8_t>(tick % 384 / 96 + 1));
                s.Write8(work + 11u, static_cast<uint8_t>(tick % 384 % 96 + 1));
            } else {
                const uint32_t rawPad = U(s.ReadPad80035510());
                if (rawPad != 0u) Log::Printf("S2 lifecycle 80035510 direct raw=0x%04X", unsigned(rawPad & 0xFFFFu));
                s.ObserveInput80035510(rawPad, remapPad(rawPad), work);
                if (rawPad != 0u) {
                    s.Write32(work + 24u, 2048u);
                    mapInput(2048u);
                } else {
                    s.Write32(work + 24u, 0u);
                    // Unlike live idle, replay idle preserves work+32.
                }
            }
        } else {
            const uint32_t rawPad = U(s.ReadPad80035510());
            if (rawPad != 0u) Log::Printf("S2 lifecycle 80035510 direct raw=0x%04X", unsigned(rawPad & 0xFFFFu));
            const uint32_t pad = remapPad(rawPad);
            s.ObserveInput80035510(rawPad, pad, work);
            s.Write32(work + 24u, pad);
            if (pad != 0u) mapInput(pad);
            else s.Write32(work + 32u, 0u);
        }
        s.Call(0x80024FD0u, {work});
        UpdateEvents801C870C(s, work);
        bool running = true;
        const uint32_t masked = s.Read32(work + 24u) & 0x9FFu;
        // Actual LW/BEQ at 801C7270/78; not the folded 'masked != 0'.
        if ((s.Read32(work) & 1u) != 0u && masked != s.Read32(0x801CBA30u)) {
            if (s.Read16(work + 82u) == 0u) {
                const uint32_t timecode = s.Read32(work + 4u);
                // Aligned native work block: SWL(+11)/SWR(+8) each stores
                // this DWORD. Preserve both original writes in the trace.
                s.Write32(work + 8u, timecode);
                s.Write32(work + 8u, timecode);
                s.Write32(work + 16u, s.Read32(work + 12u));
            }
            const int32_t judgeResult = JudgeInput80014614(s, work);
            s.ObserveJudgeInput80014614(
                s.Read32(work + 24u), s.Read32(work + 32u), judgeResult, work);
            if (judgeResult == 0) {
                const uint32_t feedback = s.Read32(0x80094434u);
                if (feedback == 0x801C9644u) InputFeedback801C9644(s, work);
                else s.Call(feedback, {work, 1u});
            }
            const uint32_t pose = s.Read32(0x8009443Cu);
            if (pose == 0x801C9730u) SpecialPose801C9730(s, work);
            else s.Call(pose, {work});
            if (s.Read16(0x800916FAu) == 1u && (s.Read32(work + 24u) & 0x100u) != 0u) {
                s.Write32(work + 60u, 1u);
                s.Call(0x80014C5Cu, {250u});
            } else if (s.Read32(work + 24u) == 2048u) {
                running = false;
            }
        }
        const uint32_t flags = s.Read32(work);
        if ((flags & 0x200u) != 0u) {
            s.Call(0x8001A654u, {s.Read32(0x8005541Cu + 12u * U(Signed16(s.Read16(work + 78u))))});
        } else if ((flags & 0x40u) != 0u) {
            PlayCue80026EF8(s, s.Read32(0x800943ECu));
            s.CallVoid(0x8001A4A4u, {1u});
        } else if ((flags & 0x100u) != 0u) {
            running = false;
        }
        s.Write32(0x801CBA30u, masked);
        if (!running) break;
        s.Call(0x80015350u, {work, 2u * s.Read32(s.Read32(kSceneEntry) + 352u)});
        s.Call(0x80035560u, {2u});
        Present801CB170(s);
        const int32_t end = s.Call(0x8001A7F8u, {record});
        if (s.Call(0x8001A3B8u, {}) == 1) {
            s.Call(0x8001A694u, {});
            s.Call(0x80035838u, {});
            s.Write16(work + 118u, 1u);
            break;
        }
        if (end == 1) break;
    }
    s.Write16(work + 100u, 0u);
    s.Write16(work + 84u, 0u);
    s.Write16(work + 122u, 0u);
    for (int32_t remaining = 4; remaining > 0; --remaining) {
        PrepareFrame801CA57C(s, work, 7);
        s.Call(0x80035560u, {2u});
        Present801CB170(s);
    }
    s.Call(0x8001B120u, {1u});
    ResetAudio80026FA4(s);
    s.Call(0x800357D4u, {0u});
    s.Call(0x8001A694u, {});
    if (s.Read16(work + 118u) == 1u) return 2;
    if (s.Read16(work + 120u) != 1u) return 3;
    const int32_t score = s.Call(0x800166ACu, {U(scene)});
    return s.Read16(kEasyDA) == 1u || score < 4 ? 1 : 2;
}

int32_t RunScene801C74E4(Services& s, int32_t scene) {
    ConfigureText801CB244(s);
    ResetAudio80026FA4(s);
    if (static_cast<uint32_t>(s.Read16(kModeD0)) - 1u >= 2u) {
        s.Call(0x800201ACu, {kTransition, 6u, 2u, 1u});
        InitMovie801C6A3C(s, s.Read32(kSceneEntry) + 108u, 0);
        const int32_t movie = RunMovie801C6AB8(s,
            s.Read32(kSceneEntry) + 108u, kTransition, 0u);
        ResetAudio80026FA4(s);
        s.Call(0x800201ACu, {kTransition, 5u, 1u, 2u});
        if (movie == 1) {
            s.Call(0x8001EF14u, {});
            s.Write16(kExitE0, 3u);
            return -1;
        }
    }
    for (;;) {
        s.BeginGameStreamStartup();
        // The original source returns from the transition before this call.
        // Product hosts additionally wait until that returned page has had a
        // separate display opportunity, so 8001A4D0 cannot expose the first
        // gameplay page in the same compositor interval as the transition.
        s.AwaitGameEntryPresentation();
        s.Call(0x8001A4D0u, {s.Read32(kSceneEntry) + 156u, 0u});
        const int32_t result = RunGame801C6D58(s,
            s.Read32(kSceneEntry) + 156u, kTransition, scene);
        if (s.Read16(kModeD0) == 1u) {
            s.Write16(kModeD0, 0u);
            // LW in the binary; Hex-Rays folds this mutable source to zero.
            s.Write16(kEasyDA, static_cast<uint16_t>(s.Read32(0x801D2988u)));
            s.Call(0x80020110u, {kTransition, 2u, 1u, 2u});
            s.Call(0x8001EF14u, {});
            return 0;
        }
        if (s.Read16(kModeD0) == 2u) {
            s.Call(0x80015744u, {kSaveBank});
            s.Write16(kModeD0, 0u);
            s.Call(0x80020110u, {kTransition, 2u, 1u, 2u});
            s.Call(0x8001EF14u, {});
            s.Write16(kExitE0, 3u);
            s.Write16(kEasyDA, static_cast<uint16_t>(s.Read32(0x801D2988u)));
            return -1;
        }
        if (result == 1) break;
        if (s.Call(0x80026B94u, {4u, 0u}) == 2) {
            s.Call(0x80020110u, {kTransition, 2u, 1u, 2u});
            s.Call(0x8001EF14u, {});
            s.Write16(kExitE0, 2u);
            return -1;
        }
        s.Call(0x8001EF14u, {});
    }
    s.Call(0x80020110u, {kTransition, 2u, 1u, 2u});
    const int32_t score = s.Call(0x800166ACu, {U(scene)});
    const bool cool = s.Read16(0x801C368Eu) != 0u;
    const uint32_t movie = s.Read32(kSceneEntry) + (cool ? 252u : 204u);
    InitMovie801C6A3C(s, movie, 0);
    ResetAudio80026FA4(s);
    s.Call(0x800201ACu, {kTransition, 6u, 2u, 1u});
    RunMovie801C6AB8(s, movie, kTransition, cool ? 2u : 1u);
    ResetAudio80026FA4(s);
    s.Call(0x800201ACu, {kTransition, 5u, 1u, 2u});
    s.Call(0x8001EF14u, {});
    PlayCue80026EF8(s, s.Read32(0x80094410u));
    FlushAudio80026ECC(s);
    if (s.Read16(kEasyDA) == 0u) {
        s.Call(0x8001635Cu, {U(scene), cool ? 2u : 3u, U(score),
                            U(Signed16(s.Read16(0x80091816u)))});
        if (scene < 6) s.Call(0x8001628Cu, {U(scene + 1)});
        if (s.Read16(0x800916F0u) != 1u) {
            s.Call(0x80015590u, {U(scene)});
            s.Call(0x80019148u, {kSaveBank});
        }
    }
    s.Call(0x8001EF14u, {});
    const int32_t lastScene = s.Read16(kEasyDA) == 1u ? 3 : 6;
    return scene < lastScene ? scene + 1 : 0;
}

} // namespace PrStage2LifecycleDirect
