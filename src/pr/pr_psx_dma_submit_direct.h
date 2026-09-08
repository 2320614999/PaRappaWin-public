#pragma once

#include <array>
#include <cstdint>

namespace PrPsxDmaSubmitDirect {

constexpr uint32_t kFn80040CA4_SubmitWorkList = 0x80040CA4u;
constexpr uint32_t kFn80044E2C_MoveImage = 0x80044E2Cu;
constexpr uint32_t kFn800450A0_DrawOtag = 0x800450A0u;
constexpr uint32_t kFn800468E0_DmaDispatch = 0x800468E0u;
constexpr uint32_t kFn80046840_DmaLinkedList = 0x80046840u;
constexpr uint32_t kFn80047144_GpuWaitPrepare = 0x80047144u;
constexpr uint32_t kFn80047178_GpuWaitPoll = 0x80047178u;
constexpr uint32_t kFn80046BC4_DmaQueueConsumer = 0x80046BC4u;

constexpr uint32_t kWorkListBase80087288 = 0x80087288u;
constexpr uint32_t kWorkListStride80040CA4 = 0x14u;
constexpr uint32_t kWorkListCount80040CA4 = 2u;
constexpr uint32_t kWorkListLastAddressOffset80040CA4 = 0x10u;
constexpr uint32_t kDispatchQueueCapacity800468E0 = 64u;
constexpr uint32_t kGpuWaitDeadlineOffset80047144 = 0xF0u;
constexpr uint32_t kMoveImagePacketAddress8005D7DC = 0x8005D7DCu;
constexpr uint32_t kMoveImageRectAddress8005D7E4 = 0x8005D7E4u;
constexpr uint32_t kMoveImageDestAddress8005D7E8 = 0x8005D7E8u;
constexpr uint32_t kMoveImageSizeAddress8005D7EC = 0x8005D7ECu;
constexpr uint32_t kMoveImagePacketLink8005D7DC = 0x04FFFFFFu;
constexpr uint32_t kMoveImageCommand8005D7E0 = 0x80000000u;
constexpr uint32_t kMoveImagePacketWordCount80044E2C = 5u;
constexpr uint32_t kMoveImageDispatchWidth800468E0 = 20u;

constexpr uint32_t kGp1Address = 0x1F801814u;
constexpr uint32_t kDma2MadrAddress = 0x1F8010A0u;
constexpr uint32_t kDma2BcrAddress = 0x1F8010A4u;
constexpr uint32_t kDma2ChcrAddress = 0x1F8010A8u;
constexpr uint32_t kGp1DmaDirectionCpuToGp0 = 0x04000002u;
constexpr uint32_t kDma2LinkedListChcr = 0x01000401u;

struct PsxDmaMmioWrite80046840 {
    uint32_t address = 0;
    uint32_t value = 0;
};

struct PsxGpuCompletionInput800468E0 {
    bool callbackFunctionKnown = false;
    uint32_t callbackFunction = 0u;
    bool callbackArgumentKnown = false;
    uint32_t callbackArgument = 0u;
    bool gp1ReadyBitKnown = false;
    bool gp1ReadyBitSet = false;
    bool interruptMaskKnown = false;
    uint16_t interruptMask = 0u;
    bool completionTupleKnown = false;
};

struct PsxGpuCompletionResult800468E0 {
    bool known = false;
    bool waitPrepare80047144Called = false;
    bool gp1ReadyBitPoll800468E0Recorded = false;
    bool timeoutHelper80047178Available = false;
    bool timeoutHelper80047178Called = false;
    bool readyAccepted = false;
    bool waitingForGpuReady = false;
    bool interruptMaskClearedBeforeCallback = false;
    bool dispatcherActive8005D73C = false;
    bool callback80046840Dispatched = false;
    bool completionTupleRecorded = false;
    bool interruptMaskClearedAfterCallback = false;
    bool softwareCompletionStateCommitted = false;
    // Translated PSX runtime state.  This is the software owner of the
    // original GP1/DMA2 transaction on Windows; it deliberately does not
    // claim that host MMIO or hardware timing was exercised.
    bool translatedGpuStateKnown = false;
    bool translatedGpuReadyObserved = false;
    bool translatedGpuCompletionPublished = false;
    bool hostMmioWritten = false;
    bool hostGpuSubmitted = false;
    bool psxGpuMmioAuthority = false;
    bool hardwareCompletionTimingAuthority = false;
    bool replayValueAuthority = false;
    bool oldWinS0Authority = false;
    bool stage2PlusAuthority = false;
    bool comod2Authority = false;
    uint32_t callbackFunction = 0u;
    uint32_t callbackArgument = 0u;
    uint32_t gpuWaitDeadlineOffset = kGpuWaitDeadlineOffset80047144;
    uint16_t previousInterruptMask = 0u;
    uint16_t savedInterruptMask8005D840 = 0u;
    bool returnValueKnown = false;
    int32_t returnValue = -1;
};

enum class PsxMoveImageStatus80044E2C : uint8_t {
    NotRun = 0,
    InvalidRectangle,
    InterruptMaskUnknown,
    SoftwareTransactionRecorded,
};

struct PsxMoveImageInput80044E2C {
    int16_t sourceX = 0;
    int16_t sourceY = 0;
    int16_t width = 0;
    int16_t height = 0;
    uint16_t destinationX = 0u;
    uint16_t destinationY = 0u;
    bool interruptMaskKnown = false;
    uint16_t interruptMask = 0u;
};

struct PsxMoveImageResult80044E2C {
    PsxMoveImageStatus80044E2C status =
        PsxMoveImageStatus80044E2C::NotRun;
    bool attempted = false;
    bool moveImageLog80044BA8Called = false;
    bool rectangleValidated = false;
    bool packetTemplateLoaded8005D7DC = false;
    bool packetGlobalsWritten = false;
    bool dispatch800468E0Called = false;
    bool gpuWaitPrepare80047144Called = false;
    bool gpuReadyWaitRecorded = false;
    bool interruptMaskClearedBeforeCallback = false;
    bool callback80046840Dispatched = false;
    bool completionTupleRecorded = false;
    bool interruptMaskClearedAfterCallback = false;
    bool softwarePacketStateCommitted = false;
    bool directDmaTransactionRecorded = false;
    bool translatedGpuDmaStateCommitted = false;
    bool translatedGpuCompletionPublished = false;
    uint32_t translatedGpuGp1Value = 0u;
    uint32_t translatedGpuDma2Madr = 0u;
    uint32_t translatedGpuDma2Bcr = 0u;
    uint32_t translatedGpuDma2Chcr = 0u;
    bool hostMmioWritten = false;
    bool hostGpuSubmitted = false;
    bool exactPsxHalParity = false;
    PsxGpuCompletionResult800468E0 gpuCompletion{};
    uint32_t sourceFunction = kFn80044E2C_MoveImage;
    uint32_t packetAddress = kMoveImagePacketAddress8005D7DC;
    uint32_t rectAddress = kMoveImageRectAddress8005D7E4;
    uint32_t destinationAddress = kMoveImageDestAddress8005D7E8;
    uint32_t sizeAddress = kMoveImageSizeAddress8005D7EC;
    uint32_t rectValue = 0u;
    uint32_t destinationValue = 0u;
    uint32_t sizeValue = 0u;
    uint32_t callbackFunction = kFn80046840_DmaLinkedList;
    uint32_t callbackArgument = 0u;
    uint32_t dispatchWidth = kMoveImageDispatchWidth800468E0;
    uint32_t dispatchMode = 0u;
    uint32_t gpuWaitDeadlineOffset = kGpuWaitDeadlineOffset80047144;
    uint16_t previousInterruptMask = 0u;
    uint32_t dispatcherActive8005D73C = 0u;
    uint16_t savedInterruptMask8005D840 = 0u;
    int32_t returnValue = -1;
    std::array<uint32_t, kMoveImagePacketWordCount80044E2C> packetWords{};
    uint32_t mmioWriteCount = 0u;
    std::array<PsxDmaMmioWrite80046840, 4> mmioWrites{};
};

enum class PsxWorkListSubmitStatus80040CA4 : uint8_t {
    NotRun = 0,
    InvalidWorkListSlot,
    InvalidWorkListAddress,
    UnknownWorkListShape,
    InvalidOtagHead,
    Executed,
};

struct PsxWorkListSubmitInput80040CA4 {
    uint32_t workListAddress = 0;
    uint32_t workListSlot = 0;
    uint32_t order = 0;
    uint32_t headAddress = 0;
    uint32_t lastAddressOffset10 = 0;
    bool clearOtagRCalled = false;
    uint32_t priorCallCount = 0;
    // 80040CA4 accepts a pointer to any PSX work-list record.  The original
    // direct adapter historically pinned this check to the main-page table
    // (80087288), which made the title caller 801C689C impossible to model.
    // Keep the main-page defaults for existing callers, while allowing the
    // title packet table (801C9574 + lane*0x14) to use the same exact lower
    // DMA/OT transaction.
    uint32_t expectedWorkListBaseAddress = kWorkListBase80087288;
    uint32_t expectedWorkListStride = kWorkListStride80040CA4;
    uint32_t expectedWorkListCount = kWorkListCount80040CA4;
};

struct PsxWorkListSubmitResult80040CA4 {
    PsxWorkListSubmitStatus80040CA4 status =
        PsxWorkListSubmitStatus80040CA4::NotRun;
    bool attempted = false;
    bool sourceKnown = false;
    bool workListOffset10Read = false;
    bool drawOtag800450A0Called = false;
    bool gpuWaitPrepare80047144Called = false;
    bool gpuWaitPoll80047178Available = false;
    bool dmaQueueConsumer80046BC4Available = false;
    bool immediateDispatchPath = false;
    bool interruptMaskClearedBeforeCallback = false;
    bool gpuReadyWaitRecorded = false;
    bool callback80046840Dispatched = false;
    bool completionTupleRecorded = false;
    bool interruptMaskClearedAfterCallback = false;
    bool softwareStateCommitted = false;
    bool translatedGpuDmaStateCommitted = false;
    bool translatedGpuCompletionPublished = false;
    uint32_t translatedGpuGp1Value = 0u;
    uint32_t translatedGpuDma2Madr = 0u;
    uint32_t translatedGpuDma2Bcr = 0u;
    uint32_t translatedGpuDma2Chcr = 0u;
    bool hostMmioWritten = false;
    bool hostGpuSubmitted = false;
    bool exactPsxHalParity = false;
    uint32_t workListAddress = 0;
    uint32_t expectedWorkListAddress = 0;
    uint32_t workListSlot = 0;
    uint32_t otagHeadAddress = 0;
    uint32_t callbackFunction = 0;
    uint32_t callbackArgument = 0;
    uint32_t dispatchQueueCapacity = kDispatchQueueCapacity800468E0;
    uint32_t gpuWaitDeadlineOffset = kGpuWaitDeadlineOffset80047144;
    uint32_t mmioWriteCount = 0;
    uint32_t callCount = 0;
    std::array<PsxDmaMmioWrite80046840, 4> mmioWrites{};
};

PsxWorkListSubmitResult80040CA4 ExecuteWorkListSubmitSoftware80040CA4(
    const PsxWorkListSubmitInput80040CA4& input);
PsxMoveImageResult80044E2C ExecuteMoveImageSoftware80044E2C(
    const PsxMoveImageInput80044E2C& input);
PsxGpuCompletionResult800468E0 ExecuteGpuCompletionSoftware800468E0(
    const PsxGpuCompletionInput800468E0& input);

}  // namespace PrPsxDmaSubmitDirect
