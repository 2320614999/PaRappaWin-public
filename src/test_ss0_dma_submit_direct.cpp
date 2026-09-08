#include "pr/pr_psx_dma_submit_direct.h"

#include <cstdio>

namespace {

using namespace PrPsxDmaSubmitDirect;

bool CheckAcceptedSlot(uint32_t slot, uint32_t headAddress) {
    PsxWorkListSubmitInput80040CA4 input{};
    input.workListSlot = slot;
    input.workListAddress =
        kWorkListBase80087288 + kWorkListStride80040CA4 * slot;
    input.order = 14u;
    input.headAddress = headAddress;
    input.lastAddressOffset10 = headAddress + (4u << input.order) - 4u;
    input.clearOtagRCalled = true;
    input.priorCallCount = 7u;

    const auto out = ExecuteWorkListSubmitSoftware80040CA4(input);
    return out.status == PsxWorkListSubmitStatus80040CA4::Executed &&
           out.attempted && out.sourceKnown && out.workListOffset10Read &&
           out.drawOtag800450A0Called &&
           out.gpuWaitPrepare80047144Called &&
           out.gpuWaitPoll80047178Available &&
           out.dmaQueueConsumer80046BC4Available &&
           out.immediateDispatchPath &&
           out.interruptMaskClearedBeforeCallback &&
           out.gpuReadyWaitRecorded && out.callback80046840Dispatched &&
           out.completionTupleRecorded &&
           out.interruptMaskClearedAfterCallback &&
           out.softwareStateCommitted &&
           out.translatedGpuDmaStateCommitted &&
           out.translatedGpuCompletionPublished &&
           out.translatedGpuGp1Value == kGp1DmaDirectionCpuToGp0 &&
           out.translatedGpuDma2Madr == input.lastAddressOffset10 &&
           out.translatedGpuDma2Bcr == 0u &&
           out.translatedGpuDma2Chcr == kDma2LinkedListChcr &&
           !out.hostMmioWritten &&
           !out.hostGpuSubmitted && !out.exactPsxHalParity &&
           out.otagHeadAddress == input.lastAddressOffset10 &&
           out.callbackFunction == kFn80046840_DmaLinkedList &&
           out.callbackArgument == 0u && out.dispatchQueueCapacity == 64u &&
           out.gpuWaitDeadlineOffset == 0xF0u && out.mmioWriteCount == 4u &&
           out.mmioWrites[0].address == kGp1Address &&
           out.mmioWrites[0].value == 0x04000002u &&
           out.mmioWrites[1].address == kDma2MadrAddress &&
           out.mmioWrites[1].value == input.lastAddressOffset10 &&
           out.mmioWrites[2].address == kDma2BcrAddress &&
           out.mmioWrites[2].value == 0u &&
           out.mmioWrites[3].address == kDma2ChcrAddress &&
           out.mmioWrites[3].value == 0x01000401u &&
           out.callCount == 8u;
}

}  // namespace

int main() {
    PsxMoveImageInput80044E2C moveInput{};
    moveInput.sourceX = 0;
    moveInput.sourceY = 240;
    moveInput.width = 320;
    moveInput.height = 240;
    moveInput.destinationX = 0u;
    moveInput.destinationY = 0u;
    moveInput.interruptMaskKnown = true;
    moveInput.interruptMask = 0u;
    const auto move = ExecuteMoveImageSoftware80044E2C(moveInput);
    if (move.status !=
            PsxMoveImageStatus80044E2C::SoftwareTransactionRecorded ||
        !move.attempted || !move.moveImageLog80044BA8Called ||
        !move.rectangleValidated ||
        !move.packetTemplateLoaded8005D7DC ||
        !move.packetGlobalsWritten || !move.dispatch800468E0Called ||
        !move.gpuWaitPrepare80047144Called ||
        !move.gpuReadyWaitRecorded ||
        !move.interruptMaskClearedBeforeCallback ||
        !move.callback80046840Dispatched ||
        !move.completionTupleRecorded ||
        !move.interruptMaskClearedAfterCallback ||
        !move.softwarePacketStateCommitted ||
        !move.directDmaTransactionRecorded || move.hostMmioWritten ||
        move.hostGpuSubmitted || move.exactPsxHalParity ||
        !move.translatedGpuDmaStateCommitted ||
        !move.translatedGpuCompletionPublished ||
        move.translatedGpuGp1Value != kGp1DmaDirectionCpuToGp0 ||
        move.translatedGpuDma2Madr != kMoveImagePacketAddress8005D7DC ||
        move.translatedGpuDma2Bcr != 0u ||
        move.translatedGpuDma2Chcr != kDma2LinkedListChcr ||
        !move.gpuCompletion.known ||
        !move.gpuCompletion.waitPrepare80047144Called ||
        !move.gpuCompletion.gp1ReadyBitPoll800468E0Recorded ||
        !move.gpuCompletion.timeoutHelper80047178Available ||
        move.gpuCompletion.timeoutHelper80047178Called ||
        !move.gpuCompletion.readyAccepted ||
        !move.gpuCompletion.softwareCompletionStateCommitted ||
        !move.gpuCompletion.translatedGpuStateKnown ||
        !move.gpuCompletion.translatedGpuReadyObserved ||
        !move.gpuCompletion.translatedGpuCompletionPublished ||
        !move.gpuCompletion.returnValueKnown ||
        move.gpuCompletion.waitingForGpuReady ||
        move.gpuCompletion.hostMmioWritten ||
        move.gpuCompletion.hostGpuSubmitted ||
        move.gpuCompletion.psxGpuMmioAuthority ||
        move.gpuCompletion.hardwareCompletionTimingAuthority ||
        move.rectValue != 0x00F00000u ||
        move.destinationValue != 0u ||
        move.sizeValue != 0x00F00140u ||
        move.packetWords[0] != 0x04FFFFFFu ||
        move.packetWords[1] != 0x80000000u ||
        move.packetWords[2] != move.rectValue ||
        move.packetWords[3] != move.destinationValue ||
        move.packetWords[4] != move.sizeValue ||
        move.callbackFunction != 0x80046840u ||
        move.dispatchWidth != 20u || move.dispatchMode != 0u ||
        move.gpuWaitDeadlineOffset != 240u ||
        move.dispatcherActive8005D73C != 1u ||
        move.savedInterruptMask8005D840 != 0u ||
        move.mmioWriteCount != 4u ||
        move.mmioWrites[0].address != kGp1Address ||
        move.mmioWrites[0].value != 0x04000002u ||
        move.mmioWrites[1].address != kDma2MadrAddress ||
        move.mmioWrites[1].value != 0x8005D7DCu ||
        move.mmioWrites[2].address != kDma2BcrAddress ||
        move.mmioWrites[2].value != 0u ||
        move.mmioWrites[3].address != kDma2ChcrAddress ||
        move.mmioWrites[3].value != 0x01000401u ||
        move.returnValue != 0) {
        std::printf("test_ss0_dma_submit_direct: FAIL move image\n");
        return 1;
    }
    moveInput.width = 0;
    if (ExecuteMoveImageSoftware80044E2C(moveInput).status !=
        PsxMoveImageStatus80044E2C::InvalidRectangle) {
        std::printf("test_ss0_dma_submit_direct: FAIL move rect guard\n");
        return 1;
    }
    moveInput.width = 320;
    moveInput.interruptMaskKnown = false;
    if (ExecuteMoveImageSoftware80044E2C(moveInput).status !=
        PsxMoveImageStatus80044E2C::InterruptMaskUnknown) {
        std::printf("test_ss0_dma_submit_direct: FAIL move mask guard\n");
        return 1;
    }

    PsxGpuCompletionInput800468E0 completionInput{};
    completionInput.callbackFunctionKnown = true;
    completionInput.callbackFunction = kFn80046840_DmaLinkedList;
    completionInput.callbackArgumentKnown = true;
    completionInput.gp1ReadyBitKnown = true;
    completionInput.gp1ReadyBitSet = false;
    completionInput.interruptMaskKnown = true;
    completionInput.completionTupleKnown = true;
    const auto waiting = ExecuteGpuCompletionSoftware800468E0(
        completionInput);
    if (!waiting.waitPrepare80047144Called ||
        !waiting.gp1ReadyBitPoll800468E0Recorded ||
        !waiting.timeoutHelper80047178Available ||
        waiting.timeoutHelper80047178Called ||
        !waiting.waitingForGpuReady || !waiting.known ||
        waiting.softwareCompletionStateCommitted || waiting.returnValueKnown ||
        waiting.returnValue != -1) {
        std::printf("test_ss0_dma_submit_direct: FAIL gpu ready wait\n");
        return 1;
    }

    if (!CheckAcceptedSlot(0u, 0x80088288u) ||
        !CheckAcceptedSlot(1u, 0x80098288u)) {
        std::printf("test_ss0_dma_submit_direct: FAIL accepted\n");
        return 1;
    }

    PsxWorkListSubmitInput80040CA4 input{};
    input.workListSlot = 2u;
    input.workListAddress = kWorkListBase80087288 + 2u * 0x14u;
    input.order = 14u;
    input.headAddress = 0x80088288u;
    input.lastAddressOffset10 = 0x80098284u;
    input.clearOtagRCalled = true;
    if (ExecuteWorkListSubmitSoftware80040CA4(input).status !=
        PsxWorkListSubmitStatus80040CA4::InvalidWorkListSlot) {
        std::printf("test_ss0_dma_submit_direct: FAIL slot guard\n");
        return 1;
    }

    input.workListSlot = 0u;
    input.workListAddress = kWorkListBase80087288 + 4u;
    if (ExecuteWorkListSubmitSoftware80040CA4(input).status !=
        PsxWorkListSubmitStatus80040CA4::InvalidWorkListAddress) {
        std::printf("test_ss0_dma_submit_direct: FAIL address guard\n");
        return 1;
    }

    input.workListAddress = kWorkListBase80087288;
    input.lastAddressOffset10 = 0x80098280u;
    if (ExecuteWorkListSubmitSoftware80040CA4(input).status !=
        PsxWorkListSubmitStatus80040CA4::InvalidOtagHead) {
        std::printf("test_ss0_dma_submit_direct: FAIL head guard\n");
        return 1;
    }

    input.lastAddressOffset10 = 0x80098284u;
    input.clearOtagRCalled = false;
    const auto unknown = ExecuteWorkListSubmitSoftware80040CA4(input);
    if (unknown.status !=
            PsxWorkListSubmitStatus80040CA4::UnknownWorkListShape ||
        unknown.sourceKnown || unknown.softwareStateCommitted ||
        unknown.mmioWriteCount != 0u) {
        std::printf("test_ss0_dma_submit_direct: FAIL shape guard\n");
        return 1;
    }

    std::printf("test_ss0_dma_submit_direct: PASS\n");
    return 0;
}
