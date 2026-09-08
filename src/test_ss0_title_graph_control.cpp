#include "pr/pr_psx_graph_owner_direct.h"
#include "pr/pr_psx_clear_image_direct.h"
#include "pr/pr_ss0_title_transform_direct.h"

#include <cstdio>

int main()
{
    PrPsxGraphOwnerDirect::PsxGraphState graph{};
    PrPsxGraphOwnerDirect::PsxInitializeGraphState8003FB9C(
        graph, 320u, 240u);
    PrPsxGraphOwnerDirect::PsxCall8003FC14_ApplyGraphModeFlags(graph, 4u);
    PrPsxGraphOwnerDirect::PsxCall80040AE4_SetDoubleBufferOffsets(
        graph, 0, 0, 0, 240);
    PrPsxGraphOwnerDirect::PsxCall80040B84_ApplyScreenCenterAndDrawOffset(
        graph);
    PrPsxGraphOwnerDirect::PsxCall80040C74_GsSetProjection(graph, 440u);

    if (!graph.mainPageWorkLists80087288Initialized ||
        !graph.tmdFastHandlerTableKnown8001C1E8 ||
        !PrPsxGraphOwnerDirect::IsExactTmdFastHandlerTable8001C1E8(graph) ||
        graph.tmdFastHandlerTableNonZeroCount8001C1E8 != 16u ||
        graph.tmdFastHandlerTable8001C1E8[2] !=
            PrPsxGraphOwnerDirect::kGsTmdFastF3NL8001C1E8 ||
        graph.tmdFastHandlerTable8001C1E8[22] !=
            PrPsxGraphOwnerDirect::kGsTmdFastTNF3_8001C1E8 ||
        graph.tmdFastHandlerTable8001C1E8[62] !=
            PrPsxGraphOwnerDirect::kGsTmdFastTNG4_8001C1E8 ||
        graph.tmdFastHandlerTable8001C1E8[0] != 0u ||
        graph.word_80096590 != 0u || graph.word_800965A0 != 4u ||
        graph.word_8008ECA8 !=
            (std::array<int16_t, 2>{{0, 0}}) ||
        graph.word_8008ECAC !=
            (std::array<int16_t, 2>{{0, 240}}) ||
        graph.word_8008EEF0 !=
            (std::array<int16_t, 2>{{0, 0}}) ||
        graph.word_8008EEF4 !=
            (std::array<int16_t, 2>{{0, 0}}) ||
        graph.word_800901C4 != 160 || graph.word_800901C6 != 120 ||
        !graph.drawOffset.setDrawEnvCalled ||
        graph.drawOffset.word_800917AA != 0 ||
        graph.drawOffset.word_800917AC != 0 ||
        graph.drawOffset.word_80091730 != 0 ||
        graph.drawOffset.word_80091732 != 0 ||
        graph.drawOffset.word_80091734 != 320 ||
        graph.drawOffset.word_80091736 != 240 ||
        graph.drawOffset.word_80091738 != 160 ||
        graph.drawOffset.word_8009173A != 120 ||
        !PrSS0TitleTransformDirect::IsExactTitleGteControl801C609C(
            graph.gte)) {
        std::printf("test_ss0_title_graph_control: FAIL\n");
        return 1;
    }

    const auto flip0 =
        PrPsxGraphOwnerDirect::PsxCall80040370_FlipGraph(graph);
    if (flip0.previousSlot != 0u || flip0.nextSlot != 1u ||
        flip0.previousFrameCounter8009658C != 1u ||
        flip0.nextFrameCounter8009658C != 2u ||
        !flip0.sub800452ECCalled || !flip0.sub80044AA0Called ||
        !flip0.sub800402E0Called || !flip0.sub800401ACCalled ||
        graph.word_80096590 != 1u || graph.dword_8009658C != 2u ||
        graph.word_80091790 != 0 || graph.word_80091792 != 0 ||
        graph.drawOffset.word_80091730 != 0 ||
        graph.drawOffset.word_80091732 != 240) {
        std::printf("test_ss0_title_graph_control: FAIL flip0\n");
        return 1;
    }

    PrPsxClearImageDirect::PsxClearImageInput80040420 clearInput{};
    clearInput.word_80096590 = graph.word_80096590;
    clearInput.word_8008ECA8 = graph.word_8008ECA8;
    clearInput.word_8008ECAC = graph.word_8008ECAC;
    clearInput.dword_800917FC = 320u;
    clearInput.dword_8009182C = 240u;
    const auto clear = PrPsxClearImageDirect::PsxCall80040420_ClearImage(
        clearInput, 0x100u, 0x101u, 0x146u);
    if (!clear.attempted || !clear.sourceKnown ||
        !clear.softwareStateCommitted || !clear.clearImage80044CD0Called ||
        clear.hostGpuClearSubmitted || clear.exactPsxGpuParity ||
        clear.slot != 1u || clear.x != 0 || clear.y != 240 ||
        clear.width != 320u || clear.height != 240u ||
        clear.r != 0u || clear.g != 1u || clear.b != 70u ||
        clear.callCount != 1u) {
        std::printf("test_ss0_title_graph_control: FAIL clear\n");
        return 1;
    }

    clearInput.word_80096590 = 2u;
    clearInput.priorCallCount = clear.callCount;
    const auto rejectedClear =
        PrPsxClearImageDirect::PsxCall80040420_ClearImage(
            clearInput, 0u, 0u, 70u);
    if (!rejectedClear.attempted || rejectedClear.sourceKnown ||
        rejectedClear.softwareStateCommitted ||
        rejectedClear.clearImage80044CD0Called ||
        rejectedClear.callCount != 1u) {
        std::printf("test_ss0_title_graph_control: FAIL clear guard\n");
        return 1;
    }

    const auto flip1 =
        PrPsxGraphOwnerDirect::PsxCall80040370_FlipGraph(graph);
    if (flip1.previousSlot != 1u || flip1.nextSlot != 0u ||
        flip1.previousFrameCounter8009658C != 2u ||
        flip1.nextFrameCounter8009658C != 3u ||
        graph.word_80096590 != 0u || graph.dword_8009658C != 3u ||
        graph.word_80091790 != 0 || graph.word_80091792 != 240 ||
        graph.drawOffset.word_80091730 != 0 ||
        graph.drawOffset.word_80091732 != 0) {
        std::printf("test_ss0_title_graph_control: FAIL flip1\n");
        return 1;
    }
    std::printf("test_ss0_title_graph_control: PASS\n");
    return 0;
}
