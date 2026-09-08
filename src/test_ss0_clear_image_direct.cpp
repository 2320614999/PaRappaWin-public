#include "pr/pr_psx_clear_image_direct.h"

#include <cstdio>

int main() {
    PrPsxClearImageDirect::PsxClearImageInput80040420 input{};
    input.word_80096590 = 1u;
    input.word_8008ECA8 = {{0, 0}};
    input.word_8008ECAC = {{0, 240}};
    input.dword_800917FC = 0xABCD0140u;
    input.dword_8009182C = 0x123400F0u;

    const auto clear = PrPsxClearImageDirect::PsxCall80040420_ClearImage(
        input, 0x100u, 0x101u, 0x146u);
    if (!clear.attempted || !clear.sourceKnown ||
        !clear.softwareStateCommitted || !clear.clearImage80044CD0Called ||
        clear.hostGpuClearSubmitted || clear.exactPsxGpuParity ||
        clear.slot != 1u || clear.x != 0 || clear.y != 240 ||
        clear.width != 320u || clear.height != 240u ||
        clear.r != 0u || clear.g != 1u || clear.b != 70u ||
        clear.callCount != 1u) {
        std::printf("test_ss0_clear_image_direct: FAIL clear\n");
        return 1;
    }

    input.word_80096590 = 2u;
    input.priorCallCount = clear.callCount;
    const auto rejected = PrPsxClearImageDirect::PsxCall80040420_ClearImage(
        input, 0u, 0u, 70u);
    if (!rejected.attempted || rejected.sourceKnown ||
        rejected.softwareStateCommitted || rejected.clearImage80044CD0Called ||
        rejected.callCount != 1u) {
        std::printf("test_ss0_clear_image_direct: FAIL guard\n");
        return 1;
    }

    std::printf("test_ss0_clear_image_direct: PASS\n");
    return 0;
}
