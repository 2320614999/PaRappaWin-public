#include "pr_psx_dma_submit_direct.h"

#include <cstdint>

namespace PrPsxDmaSubmitDirect {

PsxGpuCompletionResult800468E0 ExecuteGpuCompletionSoftware800468E0(
    const PsxGpuCompletionInput800468E0& input) {
    PsxGpuCompletionResult800468E0 out{};
    out.callbackFunction = input.callbackFunction;
    out.callbackArgument = input.callbackArgument;
    out.previousInterruptMask = input.interruptMask;
    out.savedInterruptMask8005D840 = input.interruptMask;
    if (!input.callbackFunctionKnown ||
        input.callbackFunction != kFn80046840_DmaLinkedList ||
        !input.callbackArgumentKnown || !input.gp1ReadyBitKnown ||
        !input.interruptMaskKnown || !input.completionTupleKnown) {
        return out;
    }

    out.waitPrepare80047144Called = true;
    out.gp1ReadyBitPoll800468E0Recorded = true;
    out.timeoutHelper80047178Available = true;
    if (!input.gp1ReadyBitSet) {
        out.waitingForGpuReady = true;
        out.known = true;
        return out;
    }

    out.readyAccepted = true;
    out.interruptMaskClearedBeforeCallback = true;
    out.dispatcherActive8005D73C = true;
    out.callback80046840Dispatched = true;
    out.completionTupleRecorded = true;
    out.interruptMaskClearedAfterCallback = true;
    out.softwareCompletionStateCommitted = true;
    out.translatedGpuStateKnown = true;
    out.translatedGpuReadyObserved = true;
    out.translatedGpuCompletionPublished = true;
    out.returnValueKnown = true;
    out.returnValue = 0;
    out.known = true;
    return out;
}

PsxMoveImageResult80044E2C ExecuteMoveImageSoftware80044E2C(
    const PsxMoveImageInput80044E2C& input) {
    PsxMoveImageResult80044E2C out{};
    out.attempted = true;
    out.moveImageLog80044BA8Called = true;
    if (input.width <= 0 || input.height <= 0) {
        out.status = PsxMoveImageStatus80044E2C::InvalidRectangle;
        return out;
    }
    if (!input.interruptMaskKnown) {
        out.status = PsxMoveImageStatus80044E2C::InterruptMaskUnknown;
        return out;
    }

    out.rectangleValidated = true;
    out.rectValue =
        static_cast<uint32_t>(static_cast<uint16_t>(input.sourceX)) |
        (static_cast<uint32_t>(static_cast<uint16_t>(input.sourceY)) << 16u);
    out.destinationValue =
        static_cast<uint32_t>(input.destinationX) |
        (static_cast<uint32_t>(input.destinationY) << 16u);
    out.sizeValue =
        static_cast<uint32_t>(static_cast<uint16_t>(input.width)) |
        (static_cast<uint32_t>(static_cast<uint16_t>(input.height)) << 16u);
    out.packetWords = {{
        kMoveImagePacketLink8005D7DC,
        kMoveImageCommand8005D7E0,
        out.rectValue,
        out.destinationValue,
        out.sizeValue,
    }};
    out.packetTemplateLoaded8005D7DC = true;
    out.packetGlobalsWritten = true;

    // Current SCUS 80044E2C dispatches the 20-byte packet through 800468E0.
    // Record the exact software-observable wait/interrupt/callback sequence
    // without writing PSX addresses on the Windows host.
    out.dispatch800468E0Called = true;
    out.gpuWaitPrepare80047144Called = true;
    out.gpuReadyWaitRecorded = true;
    out.previousInterruptMask = input.interruptMask;
    out.interruptMaskClearedBeforeCallback = true;
    out.dispatcherActive8005D73C = 1u;
    out.savedInterruptMask8005D840 = input.interruptMask;
    out.callback80046840Dispatched = true;
    out.completionTupleRecorded = true;
    out.interruptMaskClearedAfterCallback = true;
    out.mmioWrites = {{
        {kGp1Address, kGp1DmaDirectionCpuToGp0},
        {kDma2MadrAddress, kMoveImagePacketAddress8005D7DC},
        {kDma2BcrAddress, 0u},
        {kDma2ChcrAddress, kDma2LinkedListChcr},
    }};
    out.mmioWriteCount = static_cast<uint32_t>(out.mmioWrites.size());
    // Commit the translated register image in the same order as the PSX
    // caller.  The values are consumed by SS0's VRAM owner; no Windows host
    // MMIO is touched and the hardware-authority fields remain false.
    out.translatedGpuGp1Value = out.mmioWrites[0].value;
    out.translatedGpuDma2Madr = out.mmioWrites[1].value;
    out.translatedGpuDma2Bcr = out.mmioWrites[2].value;
    out.translatedGpuDma2Chcr = out.mmioWrites[3].value;
    PsxGpuCompletionInput800468E0 completionInput{};
    completionInput.callbackFunctionKnown = true;
    completionInput.callbackFunction = kFn80046840_DmaLinkedList;
    completionInput.callbackArgumentKnown = true;
    completionInput.callbackArgument = 0u;
    completionInput.gp1ReadyBitKnown = true;
    completionInput.gp1ReadyBitSet = true;
    completionInput.interruptMaskKnown = input.interruptMaskKnown;
    completionInput.interruptMask = input.interruptMask;
    completionInput.completionTupleKnown = true;
    out.gpuCompletion = ExecuteGpuCompletionSoftware800468E0(
        completionInput);
    if (!out.gpuCompletion.known ||
        !out.gpuCompletion.softwareCompletionStateCommitted ||
        out.gpuCompletion.waitingForGpuReady ||
        out.gpuCompletion.hostMmioWritten ||
        out.gpuCompletion.hostGpuSubmitted ||
        out.gpuCompletion.psxGpuMmioAuthority ||
        out.gpuCompletion.hardwareCompletionTimingAuthority) {
        return out;
    }
    out.softwarePacketStateCommitted = true;
    out.directDmaTransactionRecorded = true;
    out.translatedGpuDmaStateCommitted = true;
    out.translatedGpuCompletionPublished =
        out.gpuCompletion.translatedGpuCompletionPublished;
    out.returnValue = 0;
    out.status =
        PsxMoveImageStatus80044E2C::SoftwareTransactionRecorded;
    return out;
}

PsxWorkListSubmitResult80040CA4 ExecuteWorkListSubmitSoftware80040CA4(
    const PsxWorkListSubmitInput80040CA4& input) {
    PsxWorkListSubmitResult80040CA4 out{};
    out.attempted = true;
    out.workListAddress = input.workListAddress;
    out.workListSlot = input.workListSlot;
    out.otagHeadAddress = input.lastAddressOffset10;
    out.callbackFunction = kFn80046840_DmaLinkedList;
    out.callbackArgument = 0u;
    out.callCount = input.priorCallCount;

    if (input.expectedWorkListStride == 0u ||
        input.expectedWorkListCount == 0u ||
        input.workListSlot >= input.expectedWorkListCount) {
        out.status = PsxWorkListSubmitStatus80040CA4::InvalidWorkListSlot;
        return out;
    }
    const uint64_t expectedAddress =
        static_cast<uint64_t>(input.expectedWorkListBaseAddress) +
        static_cast<uint64_t>(input.expectedWorkListStride) *
            input.workListSlot;
    if (expectedAddress > UINT32_MAX) {
        out.status = PsxWorkListSubmitStatus80040CA4::InvalidWorkListAddress;
        return out;
    }
    out.expectedWorkListAddress = static_cast<uint32_t>(expectedAddress);
    if (input.workListAddress != out.expectedWorkListAddress) {
        out.status = PsxWorkListSubmitStatus80040CA4::InvalidWorkListAddress;
        return out;
    }
    if (!input.clearOtagRCalled || input.order >= 31u ||
        input.headAddress == 0u) {
        out.status = PsxWorkListSubmitStatus80040CA4::UnknownWorkListShape;
        return out;
    }

    const uint64_t expectedLast =
        static_cast<uint64_t>(input.headAddress) +
        (static_cast<uint64_t>(4u) << input.order) - 4u;
    if (expectedLast > UINT32_MAX || input.lastAddressOffset10 == 0u ||
        input.lastAddressOffset10 != static_cast<uint32_t>(expectedLast)) {
        out.status = PsxWorkListSubmitStatus80040CA4::InvalidOtagHead;
        return out;
    }

    // For 800450A0's zero-width call, current IDA reaches 800468E0's
    // immediate callback path when the translated dispatcher is idle. Record
    // the PSX wait/interrupt/MMIO transaction without writing host MMIO.
    out.sourceKnown = true;
    out.workListOffset10Read = true;
    out.drawOtag800450A0Called = true;
    out.gpuWaitPrepare80047144Called = true;
    out.gpuWaitPoll80047178Available = true;
    out.dmaQueueConsumer80046BC4Available = true;
    out.immediateDispatchPath = true;
    out.interruptMaskClearedBeforeCallback = true;
    out.gpuReadyWaitRecorded = true;
    out.callback80046840Dispatched = true;
    out.completionTupleRecorded = true;
    out.interruptMaskClearedAfterCallback = true;
    out.mmioWrites = {{
        {kGp1Address, kGp1DmaDirectionCpuToGp0},
        {kDma2MadrAddress, input.lastAddressOffset10},
        {kDma2BcrAddress, 0u},
        {kDma2ChcrAddress, kDma2LinkedListChcr},
    }};
    out.mmioWriteCount = static_cast<uint32_t>(out.mmioWrites.size());
    out.translatedGpuGp1Value = out.mmioWrites[0].value;
    out.translatedGpuDma2Madr = out.mmioWrites[1].value;
    out.translatedGpuDma2Bcr = out.mmioWrites[2].value;
    out.translatedGpuDma2Chcr = out.mmioWrites[3].value;
    out.translatedGpuDmaStateCommitted = true;
    out.translatedGpuCompletionPublished = true;
    out.softwareStateCommitted = true;
    out.callCount = input.priorCallCount + 1u;
    out.status = PsxWorkListSubmitStatus80040CA4::Executed;
    return out;
}

}  // namespace PrPsxDmaSubmitDirect
