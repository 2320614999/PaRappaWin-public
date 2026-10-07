#include "pr/pr_stage2_soft_float_direct.h"
#include <iostream>
#include <vector>

namespace S = PrStage2SoftFloatDirect;
namespace L = PrStage2LifecycleDirect;

// 只允许原错误路径写全局并投递 BIOS 事件；任何未声明的读取或设备调用立即失败。
struct Memory final : L::Services {
    [[noreturn]] static void Unexpected() { throw std::logic_error("Unexpected soft-float service"); }
    uint8_t Read8(uint32_t) override { Unexpected(); }
    uint16_t Read16(uint32_t) override { Unexpected(); }
    uint32_t Read32(uint32_t) override { Unexpected(); }
    void Write8(uint32_t, uint8_t) override { Unexpected(); }
    void Write16(uint32_t, uint16_t) override { Unexpected(); }
    void Write32(uint32_t address, uint32_t value) override {
        if (address != 0x800555B8u && address != 0x800555BCu) Unexpected();
        std::cout << "write " << address << " 4 " << value << '\n';
    }
    int32_t Call(uint32_t fn, S::Arguments args) override {
        if (fn != 0x80048980u || args.size() != 2u) Unexpected();
        std::cout << "call " << fn;
        for (uint32_t v : args) std::cout << ' ' << v;
        std::cout << '\n';
        return -73;
    }
    L::Words64 Call64(uint32_t, S::Arguments) override { Unexpected(); }
    L::Vector32 NormalizeVector8003A3DC(L::Vector32) override { Unexpected(); }
    int32_t SetCdLocation800367A4(uint32_t) override { Unexpected(); }
    int32_t LoadImage80044D64(L::ImageRect, uint32_t) override { Unexpected(); }
    PrPsxGteDirect::MatrixRegisters& MatrixGte() override { Unexpected(); }
    [[noreturn]] void Exit(uint32_t, S::Arguments) override { Unexpected(); }
    [[noreturn]] void Break(uint32_t, uint32_t) override { Unexpected(); }
};

// 单进程逐例运行，输出完整副作用和双字返回；不接受预期结果或函数成功回执。
int main() {
    try {
        size_t index = 0, count;
        uint32_t fn;
        while (std::cin >> fn >> count) {
            std::vector<uint32_t> args(count);
            for (auto& v : args) if (!(std::cin >> v)) return 2;
            std::cout << "case " << index++ << '\n';
            Memory memory;
            const auto invoke = [&](S::Arguments values) {
                L::Words64 result;
                int32_t scalar;
                if (S::TryCall64(memory, fn, values, result))
                    std::cout << "return64 " << result.low << ' ' << result.high << '\n';
                else if (S::TryCall(memory, fn, values, scalar))
                    std::cout << "return " << scalar << '\n';
                else std::cout << "unbound\n";
            };
            try {
                switch (count) {
                case 0: invoke({}); break;
                case 1: invoke({args[0]}); break;
                case 2: invoke({args[0],args[1]}); break;
                case 3: invoke({args[0],args[1],args[2]}); break;
                case 4: invoke({args[0],args[1],args[2],args[3]}); break;
                default: return 3;
                }
            } catch (const std::domain_error&) {
                if (fn != 0x8002902Cu || args != std::vector<uint32_t>{0x80000000u}) throw;
                std::cout << "source-nonreturn INT_MIN\n";
            } catch (const std::invalid_argument&) { std::cout << "invalid-arity\n"; }
        }
        return 0;
    } catch (const std::exception& e) { std::cerr << e.what(); return 1; }
}
