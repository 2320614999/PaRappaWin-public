#include "pr_stage2_native_dispatch.h"
#include "pr_stage2_graph_bootstrap_direct.h"
#include "pr_stage2_irq_direct.h"
#include "pr_stage2_callback_init_direct.h"
#include "pr_stage2_tim_direct.h"
#include <type_traits>
#include <utility>

namespace PrStage2NativeDispatch {
namespace {
namespace G = PrStage2GpuDirect;
namespace I = PrStage2GraphInitDirect;
namespace B = PrStage2GraphBootstrapDirect;
namespace T = PrStage2TimDirect;
namespace L = PrStage2LifecycleDirect;
using Arguments = std::initializer_list<uint32_t>;

template<class R, class... A, size_t... N>
R InvokeImpl(BaseServices& s, Arguments args, R (*fn)(BaseServices&, A...),
             std::index_sequence<N...>) {
    return fn(s, args.begin()[N]...);
}
template<class R, class... A>
R Invoke(BaseServices& s, Arguments args, R (*fn)(BaseServices&, A...)) {
    static_assert((std::is_same_v<A, uint32_t> && ...), "Non-word ABI needs typed transport");
    if (args.size() != sizeof...(A))
        throw std::invalid_argument("Native SCUS call argument count mismatch");
    return InvokeImpl(s, args, fn, std::index_sequence_for<A...>{});
}
}

bool TryCall(BaseServices& s, uint32_t function, Arguments args, int32_t& result) {
    // Reuse the already translated IRQ table, including its arity checks.
    if (PrStage2CallbackInitDirect::TryDispatch(s, function, args, result)) return true;
    if (PrStage2IrqDirect::TryDispatch(s, function, args, result)) return true;
    switch (function) {
    case 0x80040FA0u: result = Invoke(s,args,L::SetCamera80040FA0); break;
    case 0x80041374u: result = Invoke(s,args,L::ScaleCameraPoints80041374); break;
    case 0x80041464u: result = Invoke(s,args,L::CameraMagnitude80041464); break;
    case 0x8004152Cu: result = Invoke(s,args,L::PositiveBitLength8004152C); break;
    case 0x80041548u: result = Invoke(s,args,L::CameraSquareRoot80041548); break;
    case 0x800415D8u: result = Invoke(s,args,L::TransposeCameraMatrix800415D8); break;
    case 0x80041628u: result = Invoke(s,args,L::RollCamera80041628); break;
    case 0x800416E0u: result = Invoke(s,args,L::BuildCameraAxis800416E0); break;
    case 0x800417A4u: result = Invoke(s,args,L::ResolveCoordinateMatrix800417A4); break;
    case 0x8003A1D0u: result = Invoke(s,args,L::CameraSine8003A1D0); break;
    case 0x8003A29Cu: result = Invoke(s,args,L::CameraCosine8003A29C); break;
    case 0x80047144u: result = Invoke(s,args,G::ResetTimeout80047144); break;
    case 0x80047178u: result = Invoke(s,args,G::CheckTimeout80047178); break;
    case 0x80046840u: result = Invoke(s,args,G::StartLinkedDma80046840); break;
    case 0x80046BC4u: result = Invoke(s,args,G::DrainQueue80046BC4); break;
    case 0x800468E0u: result = Invoke(s,args,G::Enqueue800468E0); break;
    case 0x800450A0u: result = Invoke(s,args,G::DrawOrderingTable800450A0); break;
    case 0x80044B3Cu: result = Invoke(s,args,G::DrawSync80044B3C); break;
    case 0x80046FFCu: result = Invoke(s,args,G::SyncQueue80046FFC); break;
    case 0x80044FA8u: result = Invoke(s,args,G::ClearOrderingTable80044FA8); break;
    case 0x80045FC4u: result = Invoke(s,args,G::StartOtcDma80045FC4); break;
    case 0x80044BA8u: result = Invoke(s,args,G::CheckImageRect80044BA8); break;
    case 0x80044CD0u: result = Invoke(s,args,G::ClearImage80044CD0); break;
    case 0x80044E2Cu: result = Invoke(s,args,G::MoveImage80044E2C); break;
    case 0x8001B120u: result = Invoke(s,args,G::MoveFramebuffer8001B120); break;
    case 0x800460ACu: result = Invoke(s,args,G::ClearImage800460AC); break;
    case 0x8001B1B0u: result = Invoke(s,args,G::ClearFramebuffer8001B1B0); break;
    case 0x8004688Cu: result = Invoke(s,args,G::ReadGpuInfo8004688C); break;
    case 0x8004019Cu: result = Invoke(s,args,G::CurrentDrawBuffer8004019C); break;
    case 0x8003623Cu: result = Invoke(s,args,G::VideoMode8003623C); break;
    case 0x800467B4u: result = Invoke(s,args,G::WriteGpuControl800467B4); break;
    case 0x800473C0u: result = Invoke(s,args,G::FillBytes800473C0); break;
    case 0x80045EF0u: result = Invoke(s,args,G::DisplayX80045EF0); break;
    case 0x80044AA0u: result = Invoke(s,args,G::SetDisplayMask80044AA0); break;
    case 0x800452ECu: result = Invoke(s,args,G::PutDisplayEnvironment800452EC); break;
    case 0x80045C8Cu: result = Invoke(s,args,G::PackDrawAreaStart80045C8C); break;
    case 0x80045D58u: result = Invoke(s,args,G::PackDrawAreaEnd80045D58); break;
    case 0x80045E24u: result = Invoke(s,args,G::PackDrawOffset80045E24); break;
    case 0x80045C30u: result = Invoke(s,args,G::PackDrawMode80045C30); break;
    case 0x80045E6Cu: result = Invoke(s,args,G::PackTextureWindow80045E6C); break;
    case 0x8004598Cu: result = Invoke(s,args,G::BuildDrawEnvironment8004598C); break;
    case 0x80045114u: result = Invoke(s,args,G::PutDrawEnvironment80045114); break;
    case 0x800402E0u: result = Invoke(s,args,G::ApplyDrawClip800402E0); break;
    case 0x800401ACu: result = Invoke(s,args,G::ApplyFrameOffset800401AC); break;
    case 0x80040370u: result = Invoke(s,args,G::SwapBuffers80040370); break;
    case 0x800472E4u: result = Invoke(s,args,I::DetectGpu800472E4); break;
    case 0x80046EC0u: result = Invoke(s,args,I::ResetGpu80046EC0); break;
    case 0x800446A0u: result = Invoke(s,args,I::ResetGraph800446A0); break;
    case 0x8003FC14u: result = Invoke(s,args,I::InitGraphEnvironment8003FC14); break;
    case 0x800442F4u: result = Invoke(s,args,I::InitClearPacket800442F4); break;
    case 0x8003FDE4u: result = Invoke(s,args,I::InitGeometry8003FDE4); break;
    case 0x8003FB9Cu: result = Invoke(s,args,I::InitGraph8003FB9C); break;
    case 0x80040AE4u: result = Invoke(s,args,I::SetDoubleBufferOffsets80040AE4); break;
    case 0x80040B84u: result = Invoke(s,args,I::SetScreenCenter80040B84); break;
    case 0x8001C1E8u: result = Invoke(s,args,B::InitPrimitiveDispatch8001C1E8); break;
    case 0x800354C0u: result = Invoke(s,args,B::InitPad800354C0); break;
    case 0x8001C470u: result = Invoke(s,args,B::InitGraphics8001C470); break;
    case 0x80040EACu: result = Invoke(s,args,T::GetTimInfo80040EAC); break;
    case 0x8001AE7Cu: result = Invoke(s,args,T::UploadTim8001AE7C); break;
    case 0x800431E0u: result = Invoke(s,args,T::UploadClut800431E0); break;
    case 0x8001ADECu: result = Invoke(s,args,T::UploadRuntimeTim8001ADEC); break;
    case 0x80043EBCu:
        if (args.size() != 2u) throw std::invalid_argument("Native CLUT argument count mismatch");
        result = T::GetClut80043EBC(args.begin()[0], args.begin()[1]); break;
    case 0x80040F90u: case 0x80040C94u: case 0x80040C74u: case 0x800402C0u: case 0x80040D20u:
        throw std::invalid_argument("Native void entry requires CallVoid");
    default: return false;
    }
    return true;
}

bool TryCallVoid(BaseServices& s, uint32_t function, Arguments args) {
    switch (function) {
    case 0x80040D20u: Invoke(s,args,I::InitGte80040D20); return true;
    case 0x80040F90u: Invoke(s,args,G::SetWorkBase80040F90); return true;
    case 0x80040C94u: Invoke(s,args,G::SetGeomScreen80040C94); return true;
    case 0x80040C74u: Invoke(s,args,G::SetProjection80040C74); return true;
    case 0x800402C0u: Invoke(s,args,G::SetGeomOffset800402C0); return true;
    default:
        int32_t ignored;
        return TryCall(s, function, args, ignored);
    }
}

int32_t Services::Call(uint32_t function, Arguments args) {
    int32_t result;
    if (TryCall(*this, function, args, result)) return result;
    return CallExternal(function, args);
}
void Services::CallVoid(uint32_t function, Arguments args) {
    if (!TryCallVoid(*this, function, args)) CallExternalVoid(function, args);
}
}
