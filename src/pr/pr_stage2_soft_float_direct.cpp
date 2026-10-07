#include "pr_stage2_soft_float_direct.h"

namespace PrStage2SoftFloatDirect {
namespace {
// 私有栈上的双字只在本机按值传递；原 800289C8/80029130 分别为回绕加法/取负。
uint64_t Join(Words64 value) { return (uint64_t(value.high) << 32) | value.low; }
Words64 Split(uint64_t value) { return {uint32_t(value), uint32_t(value >> 32)}; }
int32_t Signed(uint32_t value) {
    return value <= 0x7FFFFFFFu ? int32_t(value) : int32_t(int64_t(value) - 0x100000000LL);
}
bool Zero(Words64 value) { return (value.high & 0x7FFFFFFFu) == 0u && value.low == 0u; }
int32_t Exponent(Words64 value) { return int32_t((value.high >> 20) & 0x7FFu); }
uint64_t Mantissa(Words64 value) { return (Join(value) & 0xFFFFFFFFFFFFFull) | 0x10000000000000ull; }

// 80028A78 是逐位算术右移；80028E64 是逻辑右移，超宽位移仍保留原循环结果。
uint64_t ShiftRight(uint64_t value, int32_t count, bool arithmetic) {
    if (count <= 0) return value;
    const bool negative = arithmetic && (value >> 63) != 0;
    if (count >= 64) return negative ? ~uint64_t(0) : 0u;
    const uint64_t result = value >> count;
    return negative ? result | (~uint64_t(0) << (64 - count)) : result;
}
void RequireArity(Arguments args, size_t count) {
    if (args.size() != count) throw std::invalid_argument("S2 software float argument count");
}
}

// 原 800291A4 先发布错误码和运算编号，再投递 BIOS 事件，忽略事件返回值。
int32_t ReportError800291A4(Memory& s, uint32_t error, uint32_t operation) {
    s.Write32(0x800555B8u, error);
    s.Write32(0x800555BCu, operation);
    if (error == 0x21u || error == 0x22u)
        s.CallVoid(0x80048980u, {0xF4000002u, error == 0x21u ? 0x301u : 0x302u});
    return 0;
}

// 原加法：有符号扩展尾数保留九位，先对齐相加，再按最低保留位选择 255/256 舍入。
Words64 Add8002864C(Memory& s, Words64 lhs, Words64 rhs) {
    if (Zero(lhs)) return rhs;
    if (Zero(rhs)) return lhs;
    int32_t exponent = Exponent(lhs);
    const int32_t rhsExponent = Exponent(rhs);
    if (exponent > rhsExponent + 54) return lhs;
    if (rhsExponent > exponent + 54) return rhs;
    uint64_t left = Mantissa(lhs), right = Mantissa(rhs);
    if (lhs.high & 0x80000000u) left = 0u - left;
    if (rhs.high & 0x80000000u) right = 0u - right;
    left <<= 9;
    right <<= 9;
    if (rhsExponent < exponent) right = ShiftRight(right, exponent - rhsExponent, true);
    else {
        left = ShiftRight(left, rhsExponent - exponent, true);
        exponent = rhsExponent;
    }
    uint64_t sum = left + right;
    uint32_t sign = 0;
    if (sum >> 63) { sum = 0u - sum; sign = 0x80000000u; }
    else if (sum == 0) return {0u, 0u};
    while ((sum & 0xE000000000000000ull) == 0u) { sum <<= 1; --exponent; }
    if (sum & 0x4000000000000000ull) { sum = ShiftRight(sum, 1, true); ++exponent; }
    sum += (sum & 0x200u) ? 0x100u : 0xFFu;
    if (sum & 0x4000000000000000ull) { sum = ShiftRight(sum, 1, true); ++exponent; }
    Words64 result = Split(ShiftRight(sum, 9, true));
    result.high &= 0xFFEFFFFFu;
    if (exponent >= 0x7FF) {
        ReportError800291A4(s, 0x22u, 0xBu);
        return {0u, sign | 0x7FF00000u};
    }
    // 原代码没有指数下溢钳位，保留负指数移位后的位模式。
    result.high |= sign | (uint32_t(exponent) << 20);
    return result;
}

// 原除法按 54 个商位长除，包含非 IEEE 的除零结果及下溢时 MIPS SLLV 的低五位规则。
Words64 Divide80028B40(Memory& s, Words64 lhs, Words64 rhs) {
    int32_t exponent = Exponent(lhs) - Exponent(rhs) + 0x3FE;
    const uint32_t sign = (lhs.high ^ rhs.high) & 0x80000000u;
    if (Zero(rhs)) return {0xFFFFFFFFu, sign ? 0xFFFFFFFFu : 0x7FFFFFFFu};
    if (Zero(lhs)) return {0u, sign};
    uint64_t numerator = Mantissa(lhs), denominator = Mantissa(rhs);
    if (numerator < denominator) { numerator <<= 1; --exponent; }
    uint64_t quotient = 0;
    for (uint64_t bit = 0x20000000000000ull; bit != 0; bit >>= 1) {
        if (numerator >= denominator) { quotient |= bit; numerator -= denominator; }
        numerator <<= 1;
    }
    int32_t shift;
    if (exponent >= 0) { ++quotient; ++exponent; shift = 1; }
    else {
        shift = -exponent;
        quotient += uint32_t(1u << (uint32_t(shift) & 31u));
        exponent = 0;
        ++shift;
    }
    Words64 result = Split(ShiftRight(quotient, shift, false));
    result.high &= 0xFFEFFFFFu;
    if (exponent >= 0x7FF) {
        ReportError800291A4(s, 0x22u, 0xFu);
        return {0u, sign | 0x7FF00000u};
    }
    result.high |= sign | (uint32_t(exponent) << 20);
    return result;
}

// 原转整数保留负零返回 0x80000000 的行为；溢出需投递事件后返回带符号饱和值。
int32_t ToSigned80028F2C(Memory& s, Words64 value) {
    if (Zero(value)) return Signed(value.high & 0x80000000u);
    const int32_t exponent = Exponent(value), shift = exponent - 0x41D;
    if (shift > 0) {
        ReportError800291A4(s, 0x22u, 0x11u);
        return Signed((value.high & 0x80000000u) ? 0x80000000u : 0x7FFFFFFFu);
    }
    const uint32_t high = uint32_t((Mantissa(value) << 10) >> 32);
    if (uint32_t(exponent - 0x3FE) >= 32u || high == 0u) return 0;
    const uint32_t result = high >> (uint32_t(-shift) & 31u);
    return Signed((value.high & 0x80000000u) ? 0u - result : result);
}

// 原整数正规化采用有符号比较；INT_MIN 取负仍为自身，原循环最终停在零且永不返回。
Words64 FromSigned8002902C(uint32_t value) {
    if (value == 0u) return {0u, 0u};
    if (value == 0x80000000u)
        throw std::domain_error("Original 8002902C does not terminate for INT_MIN");
    const uint32_t sign = value & 0x80000000u;
    if (sign) value = 0u - value;
    int32_t exponent = 0x41D;
    while (value <= 0xFFFFFFu) { value <<= 4; exponent -= 4; }
    while (value <= 0x3FFFFFFFu) { value <<= 1; --exponent; }
    Words64 result = Split((uint64_t(value) << 32) >> 10);
    result.high = (result.high & 0xFFEFFFFFu) | sign | (uint32_t(exponent) << 20);
    return result;
}

// 双字分派严格区分原参数个数，避免将 v1 当作可丢弃的标量结果。
bool TryCall64(Memory& s, uint32_t fn, Arguments args, Words64& result) {
    const auto a = args.begin();
    switch (fn) {
    case 0x8002902Cu: RequireArity(args, 1); result = FromSigned8002902C(a[0]); break;
    case 0x8002864Cu: RequireArity(args, 4); result = Add8002864C(s, {a[0],a[1]}, {a[2],a[3]}); break;
    case 0x80028B40u: RequireArity(args, 4); result = Divide80028B40(s, {a[0],a[1]}, {a[2],a[3]}); break;
    default: return false;
    }
    return true;
}

// 标量分派只接收原整数转换和错误发布入口。
bool TryCall(Memory& s, uint32_t fn, Arguments args, int32_t& result) {
    const auto a = args.begin();
    switch (fn) {
    case 0x80028F2Cu: RequireArity(args, 2); result = ToSigned80028F2C(s, {a[0],a[1]}); break;
    case 0x800291A4u: RequireArity(args, 2); result = ReportError800291A4(s, a[0],a[1]); break;
    default: return false;
    }
    return true;
}
}
