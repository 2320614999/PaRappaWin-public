#include "pr_stage1_loader_direct.h"

namespace PrStage1LoaderDirect {

namespace {

ActionBoundary BoundaryForKind(ActionKind kind) {
    switch (kind) {
    case ActionKind::OpenFile1A324:
    case ActionKind::Seek1A89C:
    case ActionKind::ReadHeader1A818:
    case ActionKind::ReadPayload1A818:
        return ActionBoundary::CdHalBoundary;
    case ActionKind::TempAlloc25B28:
    case ActionKind::Reset25A34:
    case ActionKind::StackAlloc25A70:
    case ActionKind::StackFind25BFC:
    case ActionKind::StackSplit25BBC:
    case ActionKind::StackFree25AF8:
        return ActionBoundary::LoaderMemoryDirect;
    case ActionKind::TimGetInfo40EAC:
        return ActionBoundary::DirectControl;
    case ActionKind::TimLoadImage44D64:
    case ActionKind::TimDrawSync44B3C:
        return ActionBoundary::GpuHalBoundary;
    case ActionKind::VabClose27120:
    case ActionKind::VabOpen27078:
    case ActionKind::VabTransfer270D4:
    case ActionKind::VabEnable270FC:
    case ActionKind::RetrySfxReset26FA4:
    case ActionKind::RetrySfxCue26EF8:
    case ActionKind::RetrySfxFlush26ECC:
        return ActionBoundary::SpuHalBoundary;
    case ActionKind::PadStartCom26E4C:
        return ActionBoundary::PadComHalBoundary;
    case ActionKind::Callback1537C:
    case ActionKind::CallbackDraw1EA74:
    case ActionKind::CallbackFlush26ECC:
        return ActionBoundary::CallbackHalBoundary;
    case ActionKind::Complete15590:
        return ActionBoundary::Completion;
    default:
        return ActionBoundary::DirectControl;
    }
}

bool BlocksForFeedback(ActionKind kind) {
    switch (kind) {
    case ActionKind::TempAlloc25B28:
    case ActionKind::Reset25A34:
    case ActionKind::StackAlloc25A70:
    case ActionKind::StackFind25BFC:
    case ActionKind::StackSplit25BBC:
    case ActionKind::StackFree25AF8:
    case ActionKind::OpenFile1A324:
    case ActionKind::Seek1A89C:
    case ActionKind::ReadHeader1A818:
    case ActionKind::ReadPayload1A818:
    case ActionKind::TimLoadImage44D64:
    case ActionKind::TimDrawSync44B3C:
    case ActionKind::VabClose27120:
    case ActionKind::PadStartCom26E4C:
    case ActionKind::VabOpen27078:
    case ActionKind::VabTransfer270D4:
    case ActionKind::VabEnable270FC:
    case ActionKind::RetrySfxReset26FA4:
    case ActionKind::RetrySfxCue26EF8:
    case ActionKind::RetrySfxFlush26ECC:
    case ActionKind::Callback1537C:
    case ActionKind::CallbackDraw1EA74:
    case ActionKind::CallbackFlush26ECC:
    case ActionKind::DispatchRecord1A8F0:
        return true;
    default:
        return false;
    }
}

bool IsRecordPayloadBranch(LoaderRecordType type) {
    return type == LoaderRecordType::Type1Tim ||
           type == LoaderRecordType::Type2Vab ||
           type == LoaderRecordType::Type3Split;
}

bool IsParserTerminalAction(ActionKind kind) {
    return kind == ActionKind::ParserSuccess1A8F0 ||
           kind == ActionKind::ParserFailure1A8F0;
}

std::size_t FindBranchBlockEnd(const std::vector<Action>& actions,
                               std::size_t begin) {
    std::size_t cursor = begin;
    while (cursor < actions.size() && actions[cursor].branchTemplate) {
        ++cursor;
    }
    return cursor;
}

bool HasLaterMatchingBranchTemplate(const std::vector<Action>& actions,
                                    std::size_t begin,
                                    std::size_t end,
                                    LoaderRecordType type) {
    for (std::size_t i = begin; i < end && i < actions.size(); ++i) {
        if (actions[i].branchTemplate && actions[i].recordType == type) {
            return true;
        }
    }
    return false;
}

void AttachRecordLayout(RecordData& data,
                        const Bootstrap15590Plan& plan) {
    const auto& parse = plan.parsePlan1A8F0;
    data.recordCountWord = parse.recordCountWord;
    data.sectorCountWord = parse.payloadSectorCountWord;
    data.payloadFirstWord = parse.recordPayloadFirstWord;
    data.payloadBytesWord = parse.type2PayloadBytesWord;
    data.spuBytesWord = parse.type2SpuBytesWord;
    data.recordStrideWords = parse.recordStrideWords;
}

void AttachCurrentRecordData(const RunnerState& state, Action& action) {
    if (state.currentRecordDataKnown) {
        action.recordData = state.currentRecordData;
        action.recordDataResolved = true;
        action.recordIndex = state.currentRecordData.recordIndex;
    }
    if (state.currentResolvedPayloadKnown) {
        action.resolvedPayload = state.currentResolvedPayload;
        action.payloadResolved = state.currentResolvedPayload.valid;
    }
}

void FillResolvedPayloadFromRecord(ResolvedPayload& payload,
                                   const RecordData& record) {
    if (!record.valid) {
        return;
    }
    payload.valid = true;
    payload.recordIndex = record.recordIndex;
    if (record.recordTypeKnown) {
        payload.recordType = record.recordType;
    }
    if (!payload.psxAddressKnown && record.payloadPsxAddressKnown) {
        payload.psxAddress = record.payloadPsxAddress;
        payload.psxAddressKnown = true;
    }
    if (!payload.sizeBytesKnown && record.payloadBytesKnown) {
        payload.sizeBytes = record.payloadBytes;
        payload.sizeBytesKnown = true;
    }
    if (!payload.sectorCountKnown && record.sectorCountKnown) {
        payload.sectorCount = record.sectorCount;
        payload.sectorCountKnown = true;
    }
    payload.liveBytesPresent =
        payload.liveBytesPresent || record.livePayloadBytesKnown;
    if (record.livePayloadBytesKnown && !payload.liveBytesSizeKnown &&
        record.payloadBytesKnown) {
        payload.liveBytesSize = record.payloadBytes;
        payload.liveBytesSizeKnown = true;
    }
}

void NormalizeResolvedPayload(ResolvedPayload& payload,
                              const Action& action) {
    if (payload.recordIndex == 0 && action.recordIndex != 0) {
        payload.recordIndex = action.recordIndex;
    }
    if (payload.recordType == LoaderRecordType::Unknown &&
        action.recordType != LoaderRecordType::Unknown) {
        payload.recordType = action.recordType;
    }
    if (action.recordDataResolved) {
        FillResolvedPayloadFromRecord(payload, action.recordData);
    }
    if (payload.psxAddressKnown || payload.sizeBytesKnown ||
        payload.sectorCountKnown || payload.liveBytesPresent) {
        payload.valid = true;
    }
}

bool MergeResolvedPayload(ResolvedPayload& dst,
                          const ResolvedPayload& src) {
    if (!src.valid && !src.psxAddressKnown && !src.sizeBytesKnown &&
        !src.sectorCountKnown && !src.liveBytesPresent) {
        return false;
    }
    dst.valid = true;
    if (dst.recordIndex == 0 && src.recordIndex != 0) {
        dst.recordIndex = src.recordIndex;
    }
    if (dst.recordType == LoaderRecordType::Unknown &&
        src.recordType != LoaderRecordType::Unknown) {
        dst.recordType = src.recordType;
    }
    if (src.psxAddressKnown) {
        dst.psxAddress = src.psxAddress;
        dst.psxAddressKnown = true;
    }
    if (src.sizeBytesKnown) {
        dst.sizeBytes = src.sizeBytes;
        dst.sizeBytesKnown = true;
    }
    if (src.sectorCountKnown) {
        dst.sectorCount = src.sectorCount;
        dst.sectorCountKnown = true;
    }
    if (src.liveBytesPresent) {
        dst.liveBytesPresent = true;
        if (src.liveBytesData != nullptr) {
            dst.liveBytesData = src.liveBytesData;
        }
    }
    if (src.liveBytesSizeKnown) {
        dst.liveBytesSize = src.liveBytesSize;
        dst.liveBytesSizeKnown = true;
    }
    return true;
}

CdSeamResult BuildNormalizedCdSeamResultStreamClock800493F4(
    const CdSeamResult& result) {
    CdSeamResult out = result;
    if (out.streamClock800493F4Known ||
        out.gapMissingStreamClock800493F4Producer) {
        return out;
    }

    PrMovieSegmentDirect::StreamClockProducerFeedback800493F4 feedback{};
    out.streamClock800493F4 =
        PrMovieSegmentDirect::BuildStreamClockProducerCarrier800493F4(
            feedback);
    out.streamClock800493F4Known = out.streamClock800493F4.clockKnown;
    out.gapMissingStreamClock800493F4Producer =
        out.streamClock800493F4.gapMissingByte800493F4ClockProducer;
    return out;
}

ResolvedPayload BaseResolvedPayloadForAction(const RunnerState& state,
                                             const Action& action) {
    ResolvedPayload payload{};
    if (state.currentResolvedPayloadKnown) {
        payload = state.currentResolvedPayload;
    } else if (action.payloadResolved) {
        payload = action.resolvedPayload;
    }
    NormalizeResolvedPayload(payload, action);
    return payload;
}

bool ResolveCdSeekLbaForAction(const Action& action, int32_t& out) {
    if (action.cd.lba != 0) {
        out = action.cd.lba;
        return true;
    }
    if (action.recordDataResolved && action.recordData.startSectorKnown &&
        action.recordData.startSector <= static_cast<uint32_t>(0x7FFFFFFF)) {
        out = static_cast<int32_t>(action.recordData.startSector);
        return true;
    }
    return false;
}

bool ResolveCdReadStartDstForAction(const Action& action, uint32_t& out) {
    if (action.cd.dstPtr != 0u) {
        out = action.cd.dstPtr;
        return true;
    }
    if (action.payloadResolved && action.resolvedPayload.psxAddressKnown) {
        out = action.resolvedPayload.psxAddress;
        return true;
    }
    if (action.recordDataResolved &&
        action.recordData.payloadPsxAddressKnown) {
        out = action.recordData.payloadPsxAddress;
        return true;
    }
    return false;
}

bool ResolveCdReadStartSectorCountForAction(const Action& action,
                                            int32_t& out) {
    if (action.cd.sectorCount != 0) {
        out = action.cd.sectorCount;
        return true;
    }
    if (action.payloadResolved && action.resolvedPayload.sectorCountKnown &&
        action.resolvedPayload.sectorCount <=
            static_cast<uint32_t>(0x7FFFFFFF)) {
        out = static_cast<int32_t>(action.resolvedPayload.sectorCount);
        return true;
    }
    if (action.recordDataResolved && action.recordData.sectorCountKnown &&
        action.recordData.sectorCount <= static_cast<uint32_t>(0x7FFFFFFF)) {
        out = static_cast<int32_t>(action.recordData.sectorCount);
        return true;
    }
    return false;
}

PrStage1LoaderCdHal::LowerActionRequestMetadata
BuildCdLowerActionRequestMetadataForAction(const Action& action) {
    PrStage1LoaderCdHal::LowerActionRequestMetadata out{};
    if (!HasCdSeamPayload(action)) {
        return out;
    }

    out.known = true;
    out.actionKind = action.cd.actionKind;
    out.callerFunction = action.cd.callerFunction;
    out.directHelperFunction = action.cd.directHelperFunction;
    out.lowerFunction = action.cd.lowerFunction;
    out.finalFunction = action.cd.finalFunction;

    switch (action.cd.actionKind) {
    case PrStage1LoaderCdHal::ActionKind::SeekSync800367A4: {
        out.seekRequestKnown = true;
        out.seekMsfTargetPtrKnown = action.cd.msfTargetPtr != 0u;
        out.seekMsfTargetPtr = action.cd.msfTargetPtr;
        int32_t lba = 0;
        if (ResolveCdSeekLbaForAction(action, lba)) {
            out.seekLbaKnown = true;
            out.seekLba = lba;
            out.seekMsfTargetKnown = true;
            out.seekMsfTarget =
                PrStage1LoaderCdHal::LbaToBcdMsf80036974(lba);
        }
        out.seekArg0 = action.cd.arg0;
        out.seekArg1 = static_cast<int32_t>(action.cd.msfTargetPtr);
        out.seekArg2 = action.cd.arg2;
        break;
    }
    case PrStage1LoaderCdHal::ActionKind::ReadStart80038FC0: {
        out.readStartRequestKnown = true;
        uint32_t dst = 0;
        if (ResolveCdReadStartDstForAction(action, dst)) {
            out.readStartDstPtrKnown = true;
            out.readStartDstPtr = dst;
            out.readStartArg1 = static_cast<int32_t>(dst);
        }
        int32_t sectorCount = 0;
        if (ResolveCdReadStartSectorCountForAction(action, sectorCount)) {
            out.readStartSectorCountKnown = true;
            out.readStartSectorCount = sectorCount;
            out.readStartArg0 = sectorCount;
        }
        out.readStartModeFlagKnown = true;
        out.readStartModeFlag = action.cd.modeFlag;
        out.readSyncRequestKnown = action.cd.finalFunction != 0u;
        out.readSyncFunction = action.cd.finalFunction;
        out.readSyncArg0 = action.cd.arg1;
        out.readSyncArg1 = action.cd.arg2;
        break;
    }
    case PrStage1LoaderCdHal::ActionKind::ReadSync800390C8:
        out.readSyncRequestKnown = true;
        out.readSyncFunction = action.cd.lowerFunction;
        out.readSyncArg0 = action.cd.arg0;
        out.readSyncArg1 = action.cd.arg1;
        break;
    default:
        break;
    }
    return out;
}

CdSeamResult AttachCdLowerActionRequestMetadata(const CdSeamResult& cd,
                                                const Action& action) {
    CdSeamResult out = cd;
    const PrStage1LoaderCdHal::LowerActionRequestMetadata lowerRequest =
        BuildCdLowerActionRequestMetadataForAction(action);
    if (lowerRequest.known) {
        out.lowerRequest = lowerRequest;
    }
    return out;
}

SeamResultCarrier BuildSeamResultCarrier(
    const RunnerState& state,
    const Action& action,
    const ActionFeedback& feedback) {
    SeamResultCarrier out{};
    out.psxOrder = action.psxOrder;
    out.kind = action.kind;
    out.boundary = action.boundary;
    out.recordIndex = action.recordIndex;
    out.recordType = action.recordType;
    if (state.currentResolvedPayloadKnown) {
        out.resolvedPayloadKnown = true;
        out.resolvedPayload = state.currentResolvedPayload;
    }

    if (feedback.hasCdResult) {
        out.present = true;
        out.cd = BuildNormalizedCdSeamResultStreamClock800493F4(
            AttachCdLowerActionRequestMetadata(feedback.cdResult, action));
    }
    if (feedback.hasMemoryResult) {
        out.present = true;
        out.memory = feedback.memoryResult;
    }
    if (feedback.hasGpuResult) {
        out.present = true;
        out.gpu = feedback.gpuResult;
    }
    if (feedback.hasSpuResult) {
        out.present = true;
        out.spu = feedback.spuResult;
    }
    if (feedback.hasCallbackResult) {
        out.present = true;
        out.callback = feedback.callbackResult;
    }
    return out;
}

void StoreResolvedPayload(RunnerState& state,
                          ResolvedPayload payload,
                          const Action& action) {
    NormalizeResolvedPayload(payload, action);
    if (!payload.valid) {
        return;
    }
    if (state.currentResolvedPayloadKnown) {
        ResolvedPayload merged = state.currentResolvedPayload;
        MergeResolvedPayload(merged, payload);
        payload = merged;
    }
    state.currentResolvedPayloadKnown = true;
    state.currentResolvedPayload = payload;
    state.resolvedPayloads.push_back(payload);
}

void StoreCdResolvedPayload(RunnerState& state,
                            const Action& action,
                            const CdSeamResult& result) {
    if (!result.dstPtrKnown && !result.sectorCountKnown) {
        return;
    }
    ResolvedPayload payload = BaseResolvedPayloadForAction(state, action);
    if (result.dstPtrKnown) {
        payload.psxAddress = result.dstPtr;
        payload.psxAddressKnown = true;
    }
    if (result.sectorCountKnown) {
        if (result.sectorCount >= 0) {
            payload.sectorCount = static_cast<uint32_t>(result.sectorCount);
            payload.sectorCountKnown = true;
        }
        if (!payload.sizeBytesKnown && result.sectorCount > 0) {
            payload.sizeBytes =
                static_cast<uint32_t>(result.sectorCount) *
                static_cast<uint32_t>(
                    PrStage1LoaderCdHal::kReadStart80038FC0SectorSize);
            payload.sizeBytesKnown = true;
        }
    }
    if (result.livePayloadBytesKnown) {
        payload.liveBytesPresent = true;
        payload.liveBytesData = result.livePayloadData;
        payload.liveBytesSize = result.livePayloadSize;
        payload.liveBytesSizeKnown = true;
        if (!payload.sizeBytesKnown) {
            payload.sizeBytes =
                static_cast<uint32_t>(result.livePayloadSize);
            payload.sizeBytesKnown = true;
        }
    }
    StoreResolvedPayload(state, payload, action);
}

void StoreMemoryResolvedPayload(RunnerState& state,
                                const Action& action,
                                const MemorySeamResult& result) {
    if (!result.allocKnown || !result.alloc.success) {
        return;
    }
    ResolvedPayload payload = BaseResolvedPayloadForAction(state, action);
    payload.valid = true;
    payload.psxAddress = result.alloc.psxAddress;
    payload.sizeBytes = result.alloc.alignedBytes;
    payload.psxAddressKnown = true;
    payload.sizeBytesKnown = true;
    StoreResolvedPayload(state, payload, action);
}

void StoreGpuResolvedPayload(RunnerState& state,
                             const Action& action,
                             const GpuSeamResult& result) {
    const bool parsed =
        (result.timParseKnown &&
         result.timParse.status == PrStage1LoaderGpuHal::TimParseStatus::Ok) ||
        (result.actionListKnown &&
         result.actionList.timParse.status ==
             PrStage1LoaderGpuHal::TimParseStatus::Ok);
    if (!parsed) {
        return;
    }
    ResolvedPayload payload = BaseResolvedPayloadForAction(state, action);
    payload.liveBytesPresent = true;
    StoreResolvedPayload(state, payload, action);
}

void StoreFeedbackCarriers(RunnerState& state,
                           const Action& action,
                           const ActionFeedback& feedback) {
    if (feedback.hasResolvedPayload) {
        StoreResolvedPayload(state, feedback.resolvedPayload, action);
    }
    if (feedback.hasCdResult) {
        StoreCdResolvedPayload(state, action, feedback.cdResult);
    }
    if (feedback.hasMemoryResult) {
        StoreMemoryResolvedPayload(state, action, feedback.memoryResult);
    }
    if (feedback.hasGpuResult) {
        StoreGpuResolvedPayload(state, action, feedback.gpuResult);
    }

    const SeamResultCarrier carrier =
        BuildSeamResultCarrier(state, action, feedback);
    if (carrier.present) {
        state.seamResults.push_back(carrier);
    }
}

void AttachDirectSeamMapping(Action& action) {
    switch (action.kind) {
    case ActionKind::Reset25A34:
        action.memory.directFunction =
            kPrStage1LoaderMemoryDirectFn80025A34;
        break;
    case ActionKind::OpenFile1A324:
        action.cd.actionKind = PrStage1LoaderCdHal::ActionKind::Lookup800381F8;
        action.cd.callerFunction = PrStage1LoaderCdHal::kFn8001A2B0;
        action.cd.lowerFunction = PrStage1LoaderCdHal::kFn800381F8;
        break;
    case ActionKind::Seek1A89C:
        action.cd.actionKind =
            PrStage1LoaderCdHal::ActionKind::SeekSync800367A4;
        action.cd.callerFunction = PrStage1LoaderCdHal::kFn8001A89C;
        action.cd.directHelperFunction = PrStage1LoaderCdHal::kFn80036974;
        action.cd.lowerFunction = PrStage1LoaderCdHal::kFn800367A4;
        action.cd.arg0 = PrStage1LoaderCdHal::kSeek8001A89CSyncMode;
        action.cd.arg2 = PrStage1LoaderCdHal::kSeek8001A89CSyncArg2;
        action.cd.recordDataRequired = true;
        break;
    case ActionKind::ReadHeader1A818:
    case ActionKind::ReadPayload1A818:
        action.cd.actionKind =
            PrStage1LoaderCdHal::ActionKind::ReadStart80038FC0;
        action.cd.callerFunction = PrStage1LoaderCdHal::kFn8001A818;
        action.cd.lowerFunction = PrStage1LoaderCdHal::kFn80038FC0;
        action.cd.finalFunction = PrStage1LoaderCdHal::kFn800390C8;
        action.cd.modeFlagFromParserFlag = true;
        action.cd.modeFlag =
            PrStage1LoaderCdHal::BuildReadModeFlag8001A818(
                action.parserFlag);
        action.cd.arg0 = PrStage1LoaderCdHal::kRead8001A818WaitArg;
        action.cd.arg1 = PrStage1LoaderCdHal::kRead8001A818SyncArg0;
        action.cd.arg2 = PrStage1LoaderCdHal::kRead8001A818SyncArg1;
        action.cd.recordDataRequired =
            action.kind == ActionKind::ReadPayload1A818;
        break;
    case ActionKind::TempAlloc25B28:
        action.memory.directFunction =
            kPrStage1LoaderMemoryDirectFn80025B28;
        break;
    case ActionKind::StackAlloc25A70:
        action.memory.directFunction =
            kPrStage1LoaderMemoryDirectFn80025A70;
        action.memory.recordDataRequired = true;
        break;
    case ActionKind::StackFind25BFC:
        action.memory.directFunction =
            kPrStage1LoaderMemoryDirectFn80025BFC;
        action.memory.addressFromRecordPayloadWord = true;
        action.memory.recordDataRequired = true;
        break;
    case ActionKind::StackSplit25BBC:
        action.memory.directFunction =
            kPrStage1LoaderMemoryDirectFn80025BBC;
        action.memory.stackIndexFromFindFeedback = true;
        action.memory.recordDataRequired = true;
        break;
    case ActionKind::StackFree25AF8:
        action.memory.directFunction =
            kPrStage1LoaderMemoryDirectFn80025AF8;
        break;
    case ActionKind::TimUpload1AE7C:
        action.gpu.directFunction = PrStage1LoaderGpuHal::kFn8001AE7C;
        action.gpu.timPayloadFromRecord = true;
        action.gpu.parsedTimInfoRequired = true;
        action.gpu.recordDataRequired = true;
        break;
    case ActionKind::TimGetInfo40EAC:
        action.gpu.actionKind = PrStage1LoaderGpuHal::GpuActionKind::GsGetTimInfo;
        action.gpu.directFunction = PrStage1LoaderGpuHal::kFn8001AE7C;
        action.gpu.psxFunction = PrStage1LoaderGpuHal::kFnGsGetTimInfo80040EAC;
        action.gpu.timInfoInputOffset =
            PrStage1LoaderGpuHal::kTimInfoPayloadOffset8001AE7C;
        action.gpu.timPayloadFromRecord = true;
        action.gpu.recordDataRequired = true;
        break;
    case ActionKind::TimLoadImage44D64:
        action.gpu.actionKind = PrStage1LoaderGpuHal::GpuActionKind::LoadImage;
        action.gpu.directFunction = PrStage1LoaderGpuHal::kFn8001AE7C;
        action.gpu.psxFunction = PrStage1LoaderGpuHal::kFnLoadImage80044D64;
        action.gpu.lowerFunction =
            PrStage1LoaderGpuHal::kFnLowerLoadImage800468E0;
        action.gpu.parsedTimInfoRequired = true;
        action.gpu.recordDataRequired = true;
        break;
    case ActionKind::TimDrawSync44B3C:
        action.gpu.actionKind = PrStage1LoaderGpuHal::GpuActionKind::DrawSync;
        action.gpu.directFunction = PrStage1LoaderGpuHal::kFn8001AE7C;
        action.gpu.psxFunction = PrStage1LoaderGpuHal::kFnDrawSync80044B3C;
        action.gpu.lowerFunction =
            PrStage1LoaderGpuHal::kFnLowerDrawSync80046FFC;
        action.gpu.drawSyncMode =
            PrStage1LoaderGpuHal::kDrawSyncMode8001AE7C;
        action.gpu.parsedTimInfoRequired = true;
        action.gpu.recordDataRequired = true;
        break;
    case ActionKind::VabClose27120:
        action.spu.actionKind =
            PrStage1LoaderSpuHal::ActionKind::DirectWrapper80027120;
        action.spu.wrapperFunction = PrStage1LoaderSpuHal::kFn80027120;
        action.spu.lowerFunction = PrStage1LoaderSpuHal::kFnSsVabClose8002DF80;
        action.spu.lowerResultFeedbackRequired = true;
        break;
    case ActionKind::VabOpen27078:
        action.spu.actionKind =
            PrStage1LoaderSpuHal::ActionKind::DirectWrapper80027078;
        action.spu.wrapperFunction = PrStage1LoaderSpuHal::kFn80027078;
        action.spu.lowerFunction = PrStage1LoaderSpuHal::kFn8002E3D8;
        action.spu.recordDataRequired = true;
        action.spu.lowerResultFeedbackRequired = true;
        break;
    case ActionKind::VabTransfer270D4:
        action.spu.actionKind =
            PrStage1LoaderSpuHal::ActionKind::DirectWrapper800270D4;
        action.spu.wrapperFunction = PrStage1LoaderSpuHal::kFn800270D4;
        action.spu.lowerFunction = PrStage1LoaderSpuHal::kFn8002EB80;
        action.spu.recordDataRequired = true;
        action.spu.lowerResultFeedbackRequired = true;
        break;
    case ActionKind::VabEnable270FC:
        action.spu.actionKind =
            PrStage1LoaderSpuHal::ActionKind::DirectWrapper800270FC;
        action.spu.wrapperFunction = PrStage1LoaderSpuHal::kFn800270FC;
        action.spu.lowerFunction = PrStage1LoaderSpuHal::kFn8002EEFC;
        action.spu.lowerResultFeedbackRequired = true;
        break;
    case ActionKind::RetrySfxReset26FA4:
        action.spu.actionKind =
            PrStage1LoaderSpuHal::ActionKind::DirectWrapper80026FA4;
        action.spu.wrapperFunction = PrStage1LoaderSpuHal::kFn80026FA4;
        action.spu.lowerFunction = PrStage1LoaderSpuHal::kFn800351B8;
        action.spu.lowerResultFeedbackRequired = true;
        break;
    case ActionKind::RetrySfxCue26EF8:
        action.spu.actionKind =
            PrStage1LoaderSpuHal::ActionKind::DirectWrapper80026EF8;
        action.spu.wrapperFunction = PrStage1LoaderSpuHal::kFn80026EF8;
        action.spu.lowerFunction = PrStage1LoaderSpuHal::kFn80034240;
        action.spu.cuePointer = PrStage1LoaderSpuHal::kDword80094410;
        action.spu.cueFromRetryTable = true;
        action.spu.lowerResultFeedbackRequired = true;
        break;
    case ActionKind::RetrySfxFlush26ECC:
        action.spu.actionKind =
            PrStage1LoaderSpuHal::ActionKind::DirectWrapper80026ECC;
        action.spu.wrapperFunction = PrStage1LoaderSpuHal::kFn80026ECC;
        action.spu.lowerFunction = PrStage1LoaderSpuHal::kFn8002EFF4;
        action.spu.lowerResultFeedbackRequired = true;
        break;
    case ActionKind::Callback1537C:
        action.callback.kind = CallbackFeedbackKind::Callback1537C;
        action.callback.callbackFunction = action.psxFunction;
        break;
    case ActionKind::CallbackDraw1EA74:
        action.callback.kind = CallbackFeedbackKind::CallbackDraw1EA74;
        action.callback.drawFunction = action.psxFunction;
        break;
    case ActionKind::CallbackFlush26ECC:
        action.callback.kind = CallbackFeedbackKind::CallbackFlush26ECC;
        action.callback.flushFunction = action.psxFunction;
        break;
    default:
        break;
    }
}

Action MakeAction(ActionKind kind,
                  uint32_t& order,
                  uint32_t psxFunction,
                  uint8_t attemptIndex,
                  int32_t parserFlag,
                  int32_t loaderMode,
                  LoaderRecordType recordType = LoaderRecordType::Unknown) {
    Action out{};
    out.kind = kind;
    out.boundary = BoundaryForKind(kind);
    out.psxOrder = order++;
    out.psxFunction = psxFunction;
    out.attemptIndex = attemptIndex;
    out.recordType = recordType;
    out.parserFlag = parserFlag;
    out.loaderMode = loaderMode;
    out.blocksForFeedback = BlocksForFeedback(kind);
    out.halBoundary = IsHalBoundary(out.boundary);
    out.completesRunner = kind == ActionKind::Complete15590;
    AttachDirectSeamMapping(out);
    return out;
}

void Push(std::vector<Action>& actions,
          ActionKind kind,
          uint32_t& order,
          uint32_t psxFunction,
          uint8_t attemptIndex,
          int32_t parserFlag,
          int32_t loaderMode,
          LoaderRecordType recordType = LoaderRecordType::Unknown) {
    actions.push_back(MakeAction(kind,
                                 order,
                                 psxFunction,
                                 attemptIndex,
                                 parserFlag,
                                 loaderMode,
                                 recordType));
}

void PushBranchTemplate(std::vector<Action>& actions,
                        ActionKind kind,
                        uint32_t& order,
                        uint32_t psxFunction,
                        uint8_t attemptIndex,
                        int32_t parserFlag,
                        int32_t loaderMode,
                        LoaderRecordType recordType) {
    Push(actions,
         kind,
         order,
         psxFunction,
         attemptIndex,
         parserFlag,
         loaderMode,
         recordType);
    actions.back().branchTemplate = true;
    actions.back().requiresRecordData = true;
    actions.back().payloadResolved = false;
    actions.back().blocksForFeedback = true;
}

void PushType1TimTemplate(std::vector<Action>& actions,
                          const Bootstrap15590Plan& plan,
                          uint32_t& order,
                          uint8_t attemptIndex,
                          int32_t parserFlag) {
    const auto& parse = plan.parsePlan1A8F0;
    PushBranchTemplate(actions,
                       ActionKind::StackAlloc25A70,
                       order,
                       parse.payloadAllocFunction,
                       attemptIndex,
                       parserFlag,
                       plan.loaderMode1AC18,
                       LoaderRecordType::Type1Tim);
    actions.back().sectorCountWord = parse.type1AllocSectorCountWord;
    actions.back().memory.sectorCountWord = parse.type1AllocSectorCountWord;
    actions.back().memory.sectorToBytesShift = parse.sectorToBytesShift;
    actions.back().memory.sizeFromSectorCountWord = true;
    PushBranchTemplate(actions,
                       ActionKind::Seek1A89C,
                       order,
                       parse.seekFunction,
                       attemptIndex,
                       parserFlag,
                       plan.loaderMode1AC18,
                       LoaderRecordType::Type1Tim);
    actions.back().cd.sectorCountWord = parse.type1SeekSectorCountWord;
    PushBranchTemplate(actions,
                       ActionKind::ReadPayload1A818,
                       order,
                       parse.readFunction,
                       attemptIndex,
                       parserFlag,
                       plan.loaderMode1AC18,
                       LoaderRecordType::Type1Tim);
    actions.back().cd.sectorCountWord = parse.type1SeekSectorCountWord;
    actions.back().cd.dstPtrWord = parse.recordPayloadFirstWord;
    PushBranchTemplate(actions,
                       ActionKind::TimUpload1AE7C,
                       order,
                       parse.type1RegisterHelper,
                       attemptIndex,
                       parserFlag,
                       plan.loaderMode1AC18,
                       LoaderRecordType::Type1Tim);
    actions.back().gpu.timPayloadWord = parse.recordPayloadFirstWord;
    PushBranchTemplate(actions,
                       ActionKind::TimGetInfo40EAC,
                       order,
                       parse.timUploadPlan1AE7C.timInfoFunction,
                       attemptIndex,
                       parserFlag,
                       plan.loaderMode1AC18,
                       LoaderRecordType::Type1Tim);
    actions.back().lowerFunction =
        parse.timUploadPlan1AE7C.timInfoFunction;
    actions.back().gpu.timPayloadWord = parse.recordPayloadFirstWord;
    PushBranchTemplate(actions,
                       ActionKind::TimLoadImage44D64,
                       order,
                       parse.timUploadPlan1AE7C.uploadRectFunction,
                       attemptIndex,
                       parserFlag,
                       plan.loaderMode1AC18,
                       LoaderRecordType::Type1Tim);
    actions.back().lowerFunction =
        parse.timUploadPlan1AE7C.lowerUploadCommandFunction;
    actions.back().gpu.lowerFunction = actions.back().lowerFunction;
    actions.back().gpu.timPayloadWord = parse.recordPayloadFirstWord;
    PushBranchTemplate(actions,
                       ActionKind::TimDrawSync44B3C,
                       order,
                       parse.timUploadPlan1AE7C.drawSyncFunction,
                       attemptIndex,
                       parserFlag,
                       plan.loaderMode1AC18,
                       LoaderRecordType::Type1Tim);
    actions.back().lowerFunction =
        parse.timUploadPlan1AE7C.lowerDrawSyncFunction;
    actions.back().gpu.lowerFunction = actions.back().lowerFunction;
    actions.back().gpu.timPayloadWord = parse.recordPayloadFirstWord;
    PushBranchTemplate(actions,
                       ActionKind::StackFree25AF8,
                       order,
                       parse.freeFunction,
                       attemptIndex,
                       parserFlag,
                       plan.loaderMode1AC18,
                       LoaderRecordType::Type1Tim);
    actions.back().freeCount = parse.type1FreeCount;
    actions.back().memory.freeCount = parse.type1FreeCount;
}

void PushType2VabTemplate(std::vector<Action>& actions,
                          const Bootstrap15590Plan& plan,
                          uint32_t& order,
                          uint8_t attemptIndex,
                          int32_t parserFlag) {
    const auto& parse = plan.parsePlan1A8F0;
    PushBranchTemplate(actions,
                       ActionKind::StackAlloc25A70,
                       order,
                       parse.payloadAllocFunction,
                       attemptIndex,
                       parserFlag,
                       plan.loaderMode1AC18,
                       LoaderRecordType::Type2Vab);
    actions.back().payloadBytesWord = parse.type2PayloadBytesWord;
    actions.back().spuBytesWord = parse.type2SpuBytesWord;
    actions.back().memory.sizeBytesWord = parse.type2PayloadBytesWord;
    actions.back().memory.recordDataRequired = true;
    PushBranchTemplate(actions,
                       ActionKind::Seek1A89C,
                       order,
                       parse.seekFunction,
                       attemptIndex,
                       parserFlag,
                       plan.loaderMode1AC18,
                       LoaderRecordType::Type2Vab);
    actions.back().cd.sectorCountWord = parse.type2PayloadBytesWord;
    PushBranchTemplate(actions,
                       ActionKind::ReadPayload1A818,
                       order,
                       parse.readFunction,
                       attemptIndex,
                       parserFlag,
                       plan.loaderMode1AC18,
                       LoaderRecordType::Type2Vab);
    actions.back().cd.sectorCountWord = parse.type2PayloadBytesWord;
    actions.back().cd.dstPtrWord = parse.recordPayloadFirstWord;
    PushBranchTemplate(actions,
                       ActionKind::VabClose27120,
                       order,
                       parse.type2ComInitFunction,
                       attemptIndex,
                       parserFlag,
                       plan.loaderMode1AC18,
                       LoaderRecordType::Type2Vab);
    actions.back().lowerFunction = parse.spuVabPlan.lowLevelCloseFunction;
    actions.back().spu.lowerFunction = actions.back().lowerFunction;
    PushBranchTemplate(actions,
                       ActionKind::PadStartCom26E4C,
                       order,
                       parse.type2PadStartComFunction,
                       attemptIndex,
                       parserFlag,
                       plan.loaderMode1AC18,
                       LoaderRecordType::Type2Vab);
    PushBranchTemplate(actions,
                       ActionKind::VabOpen27078,
                       order,
                       parse.type2SpuProbeFunction,
                       attemptIndex,
                       parserFlag,
                       plan.loaderMode1AC18,
                       LoaderRecordType::Type2Vab);
    actions.back().lowerFunction = parse.spuVabPlan.lowLevelOpenFunction;
    actions.back().spu.lowerFunction = actions.back().lowerFunction;
    actions.back().spu.vhPtrWord = parse.recordPayloadFirstWord;
    actions.back().spu.spuBytesWord = parse.type2SpuBytesWord;
    PushBranchTemplate(actions,
                       ActionKind::VabTransfer270D4,
                       order,
                       parse.type2SpuLoadFunction,
                       attemptIndex,
                       parserFlag,
                       plan.loaderMode1AC18,
                       LoaderRecordType::Type2Vab);
    actions.back().lowerFunction = parse.spuVabPlan.lowLevelTransferFunction;
    actions.back().spu.lowerFunction = actions.back().lowerFunction;
    actions.back().spu.vbPtrWord = parse.type2SpuBytesWord;
    actions.back().spu.spuBytesWord = parse.type2SpuBytesWord;
    PushBranchTemplate(actions,
                       ActionKind::VabEnable270FC,
                       order,
                       parse.type2SpuEnableFunction,
                       attemptIndex,
                       parserFlag,
                       plan.loaderMode1AC18,
                       LoaderRecordType::Type2Vab);
    actions.back().lowerFunction = parse.spuVabPlan.lowLevelEnableFunction;
    actions.back().spu.lowerFunction = actions.back().lowerFunction;
    PushBranchTemplate(actions,
                       ActionKind::RetrySfxReset26FA4,
                       order,
                       parse.retryAudioResetFunction,
                       attemptIndex,
                       parserFlag,
                       plan.loaderMode1AC18,
                       LoaderRecordType::Type2Vab);
    actions.back().lowerFunction = parse.retrySfxPlan.lowLevelResetFunction;
    actions.back().spu.lowerFunction = actions.back().lowerFunction;
    PushBranchTemplate(actions,
                       ActionKind::RetrySfxCue26EF8,
                       order,
                       parse.retrySfxFunction,
                       attemptIndex,
                       parserFlag,
                       plan.loaderMode1AC18,
                       LoaderRecordType::Type2Vab);
    actions.back().lowerFunction = parse.retrySfxPlan.lowLevelCueFunction;
    actions.back().spu.lowerFunction = actions.back().lowerFunction;
    actions.back().spu.cuePointer = parse.retrySfxCueTableAddress;
    PushBranchTemplate(actions,
                       ActionKind::RetrySfxFlush26ECC,
                       order,
                       parse.retryAudioFlushFunction,
                       attemptIndex,
                       parserFlag,
                       plan.loaderMode1AC18,
                       LoaderRecordType::Type2Vab);
    actions.back().lowerFunction = parse.retrySfxPlan.lowLevelFlushFunction;
    actions.back().spu.lowerFunction = actions.back().lowerFunction;
    PushBranchTemplate(actions,
                       ActionKind::StackFree25AF8,
                       order,
                       parse.freeFunction,
                       attemptIndex,
                       parserFlag,
                       plan.loaderMode1AC18,
                       LoaderRecordType::Type2Vab);
    actions.back().freeCount = parse.type2SuccessFreeCount;
    actions.back().memory.freeCount = parse.type2SuccessFreeCount;
}

void PushType3SplitTemplate(std::vector<Action>& actions,
                            const Bootstrap15590Plan& plan,
                            uint32_t& order,
                            uint8_t attemptIndex,
                            int32_t parserFlag) {
    const auto& parse = plan.parsePlan1A8F0;
    PushBranchTemplate(actions,
                       ActionKind::StackAlloc25A70,
                       order,
                       parse.payloadAllocFunction,
                       attemptIndex,
                       parserFlag,
                       plan.loaderMode1AC18,
                       LoaderRecordType::Type3Split);
    actions.back().sectorCountWord = parse.type3AllocSectorCountWord;
    actions.back().memory.sectorCountWord = parse.type3AllocSectorCountWord;
    actions.back().memory.sectorToBytesShift = parse.sectorToBytesShift;
    actions.back().memory.sizeFromSectorCountWord = true;
    PushBranchTemplate(actions,
                       ActionKind::Seek1A89C,
                       order,
                       parse.seekFunction,
                       attemptIndex,
                       parserFlag,
                       plan.loaderMode1AC18,
                       LoaderRecordType::Type3Split);
    actions.back().cd.sectorCountWord = parse.type3SeekSectorCountWord;
    PushBranchTemplate(actions,
                       ActionKind::ReadPayload1A818,
                       order,
                       parse.readFunction,
                       attemptIndex,
                       parserFlag,
                       plan.loaderMode1AC18,
                       LoaderRecordType::Type3Split);
    actions.back().cd.sectorCountWord = parse.type3SeekSectorCountWord;
    actions.back().cd.dstPtrWord = parse.recordPayloadFirstWord;
    PushBranchTemplate(actions,
                       ActionKind::StackFind25BFC,
                       order,
                       parse.type3FindHeaderHelper,
                       attemptIndex,
                       parserFlag,
                       plan.loaderMode1AC18,
                       LoaderRecordType::Type3Split);
    actions.back().memory.psxAddressWord = parse.recordPayloadFirstWord;
    PushBranchTemplate(actions,
                       ActionKind::StackSplit25BBC,
                       order,
                       parse.type3SplitBlockHelper,
                       attemptIndex,
                       parserFlag,
                       plan.loaderMode1AC18,
                       LoaderRecordType::Type3Split);
    actions.back().memory.psxAddressWord = parse.recordPayloadFirstWord;
    actions.back().memory.sectorCountWord = parse.type3AllocSectorCountWord;
    actions.back().memory.sectorToBytesShift = parse.sectorToBytesShift;
    actions.back().memory.sizeFromSectorCountWord = true;
}

void PushParseSkeleton(std::vector<Action>& actions,
                       const Bootstrap15590Plan& plan,
                       uint32_t& order,
                       uint8_t attemptIndex,
                       int32_t parserFlag) {
    const auto& parse = plan.parsePlan1A8F0;
    Push(actions,
         ActionKind::BeginParse1A8F0,
         order,
         parse.function,
         attemptIndex,
         parserFlag,
         plan.loaderMode1AC18);
    Push(actions,
         ActionKind::Seek1A89C,
         order,
         parse.seekFunction,
         attemptIndex,
         parserFlag,
         plan.loaderMode1AC18);
    Push(actions,
         ActionKind::TempAlloc25B28,
         order,
         parse.tempAllocFunction,
         attemptIndex,
         parserFlag,
         plan.loaderMode1AC18);
    actions.back().byteCount = parse.tempAllocBytes;
    actions.back().memory.sizeBytes = static_cast<int32_t>(parse.tempAllocBytes);
    actions.back().payloadResolved = true;
    Push(actions,
         ActionKind::ReadHeader1A818,
         order,
         parse.readFunction,
         attemptIndex,
         parserFlag,
         plan.loaderMode1AC18);
    actions.back().byteCount = parse.headerReadBytes;
    actions.back().cd.sectorCount = parse.headerReadBytes;
    actions.back().payloadResolved = true;
    Push(actions,
         ActionKind::DispatchRecord1A8F0,
         order,
         parse.function,
         attemptIndex,
         parserFlag,
         plan.loaderMode1AC18);

    PushBranchTemplate(actions,
                       ActionKind::RecordEnd1A8F0,
                       order,
                       parse.function,
                       attemptIndex,
                       parserFlag,
                       plan.loaderMode1AC18,
                       LoaderRecordType::End);
    PushType1TimTemplate(actions, plan, order, attemptIndex, parserFlag);
    PushType2VabTemplate(actions, plan, order, attemptIndex, parserFlag);
    PushType3SplitTemplate(actions, plan, order, attemptIndex, parserFlag);
    PushBranchTemplate(actions,
                       ActionKind::UnknownRecord1A8F0,
                       order,
                       parse.function,
                       attemptIndex,
                       parserFlag,
                       plan.loaderMode1AC18,
                       LoaderRecordType::Unknown);
    Push(actions,
         ActionKind::ParserSuccess1A8F0,
         order,
         parse.function,
         attemptIndex,
         parserFlag,
         plan.loaderMode1AC18);
    Push(actions,
         ActionKind::ParserFailure1A8F0,
         order,
         parse.function,
         attemptIndex,
         parserFlag,
         plan.loaderMode1AC18);
}

} // namespace

void Reset(RunnerState& state) {
    state = RunnerState{};
}

bool Begin(RunnerState& state, const Bootstrap15590Plan& plan) {
    Reset(state);
    if (!plan.valid || !plan.parsePlan1A8F0.valid) {
        state.status = RunnerStatus::Failed;
        return false;
    }

    state.plan = plan;
    state.planKnown = true;
    state.actions = BuildActionSkeleton(plan);
    state.nextActionIndex = 0;
    state.status = state.actions.empty() ? RunnerStatus::Failed
                                         : RunnerStatus::Ready;
    return state.status == RunnerStatus::Ready;
}

std::vector<Action> BuildActionSkeleton(const Bootstrap15590Plan& plan) {
    std::vector<Action> actions;
    if (!plan.valid || !plan.parsePlan1A8F0.valid) {
        return actions;
    }

    actions.reserve(8u + static_cast<std::size_t>(plan.loaderMaxAttempts1AC18) *
                              40u);

    uint32_t order = 0;
    Push(actions,
         ActionKind::BeginBootstrap15590,
         order,
         plan.wrapperFunction,
         0,
         plan.loaderFirstAttemptParserFlag1AC18,
         plan.loaderMode1AC18);
    Push(actions,
         ActionKind::StartCurtain15408,
         order,
         plan.startCurtainFunction,
         0,
         plan.loaderFirstAttemptParserFlag1AC18,
         plan.loaderMode1AC18);
    Push(actions,
         ActionKind::Callback1537C,
         order,
         plan.callbackFunction,
         0,
         plan.loaderFirstAttemptParserFlag1AC18,
         plan.loaderMode1AC18);
    actions.back().callback.callbackFunction = plan.callbackFunction;
    actions.back().callback.drawFunction = plan.callbackDrawFunction1537C;
    actions.back().callback.flushFunction = plan.callbackFlushFunction1537C;
    actions.back().callback.drawArg0 = plan.callbackDrawArg0_1537C;
    actions.back().callback.drawArg1 = plan.callbackDrawArg1_1537C;
    actions.back().callback.flagValue = plan.callbackFlagValue1537C;
    actions.back().callback.returnValue = plan.callbackReturnValue1537C;
    Push(actions,
         ActionKind::CallbackDraw1EA74,
         order,
         plan.callbackDrawFunction1537C,
         0,
         plan.loaderFirstAttemptParserFlag1AC18,
         plan.loaderMode1AC18);
    actions.back().callback.callbackFunction = plan.callbackFunction;
    actions.back().callback.drawFunction = plan.callbackDrawFunction1537C;
    actions.back().callback.drawArg0 = plan.callbackDrawArg0_1537C;
    actions.back().callback.drawArg1 = plan.callbackDrawArg1_1537C;
    Push(actions,
         ActionKind::CallbackFlush26ECC,
         order,
         plan.callbackFlushFunction1537C,
         0,
         plan.loaderFirstAttemptParserFlag1AC18,
         plan.loaderMode1AC18);
    actions.back().callback.callbackFunction = plan.callbackFunction;
    actions.back().callback.flushFunction = plan.callbackFlushFunction1537C;
    Push(actions,
         ActionKind::BeginLoader1AC18,
         order,
         plan.loaderFunction,
         0,
         plan.loaderFirstAttemptParserFlag1AC18,
         plan.loaderMode1AC18);

    for (uint8_t i = 0; i < plan.loaderMaxAttempts1AC18; ++i) {
        const int32_t parserFlag =
            (i == 0) ? plan.loaderFirstAttemptParserFlag1AC18
                     : plan.loaderRetryParserFlag1AC18;
        Push(actions,
             ActionKind::Reset25A34,
             order,
             plan.resetDiscFunction,
             i,
             parserFlag,
             plan.loaderMode1AC18);
        Push(actions,
             ActionKind::OpenFile1A324,
             order,
             plan.openFileFunction,
             i,
             parserFlag,
             plan.loaderMode1AC18);
        actions.back().cd.callerFunction = plan.openFilePlan1A324.probeFunction;
        actions.back().cd.lowerFunction =
            plan.openFilePlan1A324.probeLowerLookupFunction;
        actions.back().cd.descriptorPresentWord =
            plan.openFilePlan1A324.descriptorPresentWord;
        actions.back().cd.descriptorPayloadFirstWord =
            plan.openFilePlan1A324.descriptorPayloadFirstWord;
        actions.back().cd.sizeBytesWord =
            plan.openFilePlan1A324.sizeBytesWord;
        actions.back().cd.startSectorWord =
            plan.openFilePlan1A324.startSectorWord;
        actions.back().cd.endSectorWord =
            plan.openFilePlan1A324.endSectorWord;
        actions.back().cd.sectorShift = plan.openFilePlan1A324.sectorShift;
        if (plan.sceneLoaderPsxAddrKnown) {
            actions.back().cd.cdlFilePtr =
                plan.sceneLoaderPsxAddr +
                static_cast<uint32_t>(
                    plan.openFilePlan1A324.descriptorPayloadFirstWord) *
                    static_cast<uint32_t>(sizeof(uint32_t));
        }
        if (plan.sceneLoaderPathPtrKnown) {
            actions.back().cd.pathPtr = plan.sceneLoaderPathPtr;
        }
        actions.back().payloadResolved = false;
        Push(actions,
             ActionKind::CopyDescriptorWords1AC18,
             order,
             plan.loaderFunction,
             i,
             parserFlag,
             plan.loaderMode1AC18);
        actions.back().descriptorFirstWord =
            plan.loaderCopiedDescriptorFirstWord1AC18;
        actions.back().descriptorWordCount =
            plan.loaderCopiedDescriptorWordCount1AC18;

        PushParseSkeleton(actions, plan, order, i, parserFlag);

        if (i + 1u < plan.loaderMaxAttempts1AC18) {
            Push(actions,
                 ActionKind::RetryNext1AC18,
                 order,
                 plan.loaderFunction,
                 i,
                 parserFlag,
                 plan.loaderMode1AC18);
        }
    }

    Push(actions,
         ActionKind::StopCurtain1545C,
         order,
         plan.stopCurtainFunction,
         0,
         plan.loaderRetryParserFlag1AC18,
         plan.loaderMode1AC18);
    Push(actions,
         ActionKind::Complete15590,
         order,
         plan.wrapperFunction,
         0,
         plan.loaderRetryParserFlag1AC18,
         plan.loaderMode1AC18);
    actions.back().boundary = ActionBoundary::Completion;

    return actions;
}

bool HasPendingAction(const RunnerState& state) {
    return state.nextActionIndex < state.actions.size();
}

bool PopNextAction(RunnerState& state, Action& out) {
    if (state.waitingForFeedback || !HasPendingAction(state)) {
        return false;
    }

    if (state.status == RunnerStatus::Ready) {
        state.status = RunnerStatus::Running;
    }

    if (state.recordLoopPendingDispatch) {
        out = state.recordLoopDispatchAction;
        out.recordIndex = state.nextRecordIndex;
        out.recordData = RecordData{};
        out.recordDataResolved = false;
        out.resolvedPayload = ResolvedPayload{};
        out.payloadResolved = false;
        state.recordLoopPendingDispatch = false;
        state.currentRecordTypeKnown = false;
        state.currentRecordType = LoaderRecordType::Unknown;
        state.currentRecordDataKnown = false;
        state.currentRecordData = RecordData{};
        state.currentResolvedPayloadKnown = false;
        state.currentResolvedPayload = ResolvedPayload{};
        if (out.blocksForFeedback) {
            state.waitingForFeedback = true;
            state.feedbackAction = out;
        }
        return true;
    }

    while (HasPendingAction(state)) {
        const Action& candidate = state.actions[state.nextActionIndex];
        if (state.parserTailSelectionKnown) {
            if (IsParserTerminalAction(candidate.kind)) {
                if (candidate.kind != state.parserTailSelection) {
                    ++state.nextActionIndex;
                    continue;
                }
            } else {
                state.parserTailSelectionKnown = false;
                state.parserTailSelection = ActionKind::None;
            }
        }
        if (!candidate.branchTemplate) {
            break;
        }
        if (!state.currentRecordTypeKnown) {
            return false;
        }
        if (candidate.recordType != state.currentRecordType) {
            ++state.nextActionIndex;
            continue;
        }
        break;
    }
    if (!HasPendingAction(state)) {
        return false;
    }

    out = state.actions[state.nextActionIndex++];
    if (out.branchTemplate) {
        AttachCurrentRecordData(state, out);
        const std::size_t branchEnd =
            state.recordLoopActive ? state.recordBranchEndIndex
                                   : FindBranchBlockEnd(state.actions,
                                                        state.nextActionIndex);
        const bool lastMatching =
            !HasLaterMatchingBranchTemplate(state.actions,
                                            state.nextActionIndex,
                                            branchEnd,
                                            out.recordType);
        if (state.recordLoopActive && lastMatching) {
            if (IsRecordPayloadBranch(out.recordType)) {
                state.recordLoopPendingDispatch = true;
                state.nextActionIndex = state.recordBranchEndIndex;
                state.nextRecordIndex = out.recordIndex + 1u;
            } else {
                state.recordLoopActive = false;
                state.recordLoopPendingDispatch = false;
                state.nextActionIndex = state.recordAfterBranchIndex;
            }
        }
    }
    if (out.kind == ActionKind::DispatchRecord1A8F0) {
        if (!state.recordLoopActive) {
            state.recordLoopActive = true;
            state.recordLoopDispatchAction = out;
            state.recordBranchBeginIndex = state.nextActionIndex;
            state.recordBranchEndIndex =
                FindBranchBlockEnd(state.actions, state.recordBranchBeginIndex);
            state.recordAfterBranchIndex = state.recordBranchEndIndex;
            state.nextRecordIndex = 0;
        }
        state.currentRecordTypeKnown = false;
        state.currentRecordType = LoaderRecordType::Unknown;
        state.currentRecordDataKnown = false;
        state.currentRecordData = RecordData{};
        state.currentResolvedPayloadKnown = false;
        state.currentResolvedPayload = ResolvedPayload{};
        state.parserTailSelectionKnown = false;
        state.parserTailSelection = ActionKind::None;
    }
    if (out.blocksForFeedback) {
        state.waitingForFeedback = true;
        state.feedbackAction = out;
    }
    if (out.completesRunner) {
        state.status = RunnerStatus::Completed;
        state.completionKnown = true;
        state.completionResult = state.plan.parsePlan1A8F0.successReturn;
    }
    return true;
}

bool ApplyFeedback(RunnerState& state,
                   const Action& action,
                   const ActionFeedback& feedback) {
    if (!state.waitingForFeedback || !feedback.completed ||
        feedback.psxOrder != action.psxOrder ||
        feedback.kind != action.kind ||
        state.feedbackAction.psxOrder != action.psxOrder ||
        state.feedbackAction.kind != action.kind) {
        return false;
    }

    state.waitingForFeedback = false;
    state.feedbackAction = Action{};
    StoreFeedbackCarriers(state, action, feedback);

    const int32_t feedbackResult =
        feedback.hasCallbackResult && feedback.callbackResult.resultKnown
            ? feedback.callbackResult.result
            : feedback.result;

    if (!feedback.success) {
        state.status = RunnerStatus::Failed;
        state.completionKnown = true;
        state.completionResult = feedbackResult;
        return true;
    }

    if (action.kind == ActionKind::DispatchRecord1A8F0 &&
        (feedback.hasRecordData || feedback.hasRecordType)) {
        RecordData data{};
        if (feedback.hasRecordData) {
            data = feedback.recordData;
        }
        data.valid = true;
        data.recordIndex = action.recordIndex;
        AttachRecordLayout(data, state.plan);
        if (feedback.hasRecordType) {
            data.recordTypeKnown = true;
            data.recordType = feedback.recordType;
        }
        if (data.recordTypeKnown) {
            state.currentRecordTypeKnown = true;
            state.currentRecordType = data.recordType;
        }
        state.currentRecordDataKnown = data.recordTypeKnown;
        state.currentRecordData = data;
        state.records.push_back(data);
        if (state.recordLoopActive) {
            if (data.recordType == LoaderRecordType::End ||
                data.recordType == LoaderRecordType::Unknown) {
                state.recordLoopActive = false;
                state.recordLoopPendingDispatch = false;
                state.nextActionIndex = state.recordAfterBranchIndex;
                state.parserTailSelectionKnown = true;
                state.parserTailSelection =
                    data.recordType == LoaderRecordType::End
                        ? ActionKind::ParserSuccess1A8F0
                        : ActionKind::ParserFailure1A8F0;
            } else {
                state.nextActionIndex = state.recordBranchBeginIndex;
            }
        }
    }

    if (action.kind == ActionKind::Complete15590) {
        state.status = RunnerStatus::Completed;
        state.completionKnown = true;
        state.completionResult = feedbackResult;
    }
    return true;
}

static bool IsCallbackProducerStepKind(ProducerStepKind stepKind) {
    return stepKind == ProducerStepKind::CallbackBoundary ||
           stepKind == ProducerStepKind::Callback1537C ||
           stepKind == ProducerStepKind::CallbackDraw1EA74 ||
           stepKind == ProducerStepKind::CallbackFlush26ECC;
}

bool IsTypedActionFeedbackCompleteForProducerStep(
    const ProducerStep& step,
    const TypedActionFeedback& feedback) {
    if (!feedback.valid) {
        return false;
    }
    if (feedback.category != step.category ||
        feedback.stepKind != step.stepKind) {
        return false;
    }
    if (IsCallbackProducerStepKind(step.stepKind) &&
        feedback.callbackKind != step.callbackKind) {
        return false;
    }
    if (feedback.feedback.psxOrder != step.psxOrder ||
        feedback.feedback.kind != step.actionKind) {
        return false;
    }
    return IsActionFeedbackCompleteForProducerStep(
        step,
        feedback.feedback,
        feedback.resultPresent);
}

bool ApplyTypedFeedback(RunnerState& state,
                        const TypedActionFeedback& feedback,
                        TypedFeedbackApplyResult* out) {
    TypedFeedbackApplyResult result{};
    (void)DescribeWaitingProducerStep(state, result.waitingStep);
    result.waitingStepValid = result.waitingStep.valid;

    if (!result.waitingStep.valid) {
        result.status = TypedFeedbackApplyStatus::NoWaitingStep;
        if (out) {
            *out = result;
        }
        return false;
    }

    if (!feedback.valid) {
        result.status = TypedFeedbackApplyStatus::IncompleteFeedback;
        if (out) {
            *out = result;
        }
        return false;
    }

    if (feedback.category != result.waitingStep.category ||
        feedback.stepKind != result.waitingStep.stepKind) {
        result.status = TypedFeedbackApplyStatus::IncompleteFeedback;
        if (out) {
            *out = result;
        }
        return false;
    }
    if (IsCallbackProducerStepKind(result.waitingStep.stepKind) &&
        feedback.callbackKind != result.waitingStep.callbackKind) {
        result.status = TypedFeedbackApplyStatus::IncompleteFeedback;
        if (out) {
            *out = result;
        }
        return false;
    }

    if (result.waitingStep.helperGap) {
        result.status = TypedFeedbackApplyStatus::HelperGap;
        if (out) {
            *out = result;
        }
        return false;
    }

    result.feedbackComplete =
        IsTypedActionFeedbackCompleteForProducerStep(result.waitingStep,
                                                     feedback);
    if (!result.feedbackComplete) {
        result.status = TypedFeedbackApplyStatus::IncompleteFeedback;
        if (out) {
            *out = result;
        }
        return false;
    }

    result.applied =
        ApplyFeedback(state, result.waitingStep.action, feedback.feedback);
    result.status = result.applied ? TypedFeedbackApplyStatus::Applied
                                   : TypedFeedbackApplyStatus::ApplyRejected;
    if (out) {
        *out = result;
    }
    return result.applied;
}

bool IsTerminalStatus(RunnerStatus status) {
    return status == RunnerStatus::Completed || status == RunnerStatus::Failed;
}

bool IsHalBoundary(ActionBoundary boundary) {
    switch (boundary) {
    case ActionBoundary::CdHalBoundary:
    case ActionBoundary::GpuHalBoundary:
    case ActionBoundary::SpuHalBoundary:
    case ActionBoundary::PadComHalBoundary:
    case ActionBoundary::CallbackHalBoundary:
        return true;
    default:
        return false;
    }
}

bool ActionNeedsFeedback(const Action& action) {
    return action.blocksForFeedback || action.requiresRecordData ||
           action.cd.recordDataRequired || action.memory.recordDataRequired ||
           action.gpu.recordDataRequired || action.spu.recordDataRequired;
}

bool ActionNeedsRecordData(const Action& action) {
    return action.requiresRecordData || action.cd.recordDataRequired ||
           action.memory.recordDataRequired || action.gpu.recordDataRequired ||
           action.spu.recordDataRequired;
}

bool ActionNeedsResolvedPayload(const Action& action) {
    return action.kind == ActionKind::ReadPayload1A818 ||
           action.gpu.timPayloadFromRecord ||
           action.gpu.parsedTimInfoRequired ||
           action.spu.vhPtrWord != 0u || action.spu.vbPtrWord != 0u;
}

bool ActionNeedsCdFeedback(const Action& action) {
    return action.boundary == ActionBoundary::CdHalBoundary &&
           HasCdSeamPayload(action);
}

bool ActionNeedsMemoryFeedback(const Action& action) {
    return action.boundary == ActionBoundary::LoaderMemoryDirect &&
           HasMemorySeamPayload(action);
}

bool ActionNeedsGpuFeedback(const Action& action) {
    return action.boundary == ActionBoundary::GpuHalBoundary &&
           HasGpuSeamPayload(action);
}

bool ActionNeedsSpuFeedback(const Action& action) {
    return action.boundary == ActionBoundary::SpuHalBoundary &&
           HasSpuSeamPayload(action);
}

ActionProducerRequirements GetActionProducerRequirements(
    const Action& action) {
    ActionProducerRequirements out{};
    out.waitsForFeedback = action.blocksForFeedback;
    out.needsRecordData = ActionNeedsRecordData(action);
    out.needsResolvedPayload = ActionNeedsResolvedPayload(action);
    out.needsCdFeedback = ActionNeedsCdFeedback(action);
    out.needsMemoryFeedback = ActionNeedsMemoryFeedback(action);
    out.needsGpuFeedback = ActionNeedsGpuFeedback(action);
    out.needsSpuFeedback = ActionNeedsSpuFeedback(action);
    out.needsPadComFeedback =
        action.boundary == ActionBoundary::PadComHalBoundary;
    out.needsCallbackFeedback =
        action.boundary == ActionBoundary::CallbackHalBoundary;
    out.completesRunner = action.completesRunner;
    return out;
}

bool IsWaitingForFeedback(const RunnerState& state) {
    return state.waitingForFeedback;
}

bool PeekWaitingAction(const RunnerState& state, Action& out) {
    if (!state.waitingForFeedback) {
        out = Action{};
        return false;
    }
    out = state.feedbackAction;
    return true;
}

ActionFeedback BuildFeedbackSkeletonForAction(const Action& action) {
    ActionFeedback out{};
    out.psxOrder = action.psxOrder;
    out.kind = action.kind;
    return out;
}

ProducerCategory ClassifyProducerStepKind(ProducerStepKind stepKind);
bool IsExternalProducerCategory(ProducerCategory category);
bool IsKnownProducerHelperGap(ProducerCategory category);

bool DescribeWaitingProducerStep(const RunnerState& state,
                                 ProducerStep& out) {
    Action action{};
    if (!PeekWaitingAction(state, action)) {
        out = ProducerStep{};
        return false;
    }

    out = ProducerStep{};
    out.valid = true;
    out.action = action;
    out.requirements = GetActionProducerRequirements(action);
    out.feedbackSkeleton = BuildFeedbackSkeletonForAction(action);
    out.actionKind = action.kind;
    out.boundary = action.boundary;
    out.psxOrder = action.psxOrder;
    out.psxFunction = action.psxFunction;
    out.lowerFunction = action.lowerFunction;
    out.recordIndex = action.recordIndex;
    out.recordType = action.recordType;
    out.cdActionKind = action.cd.actionKind;
    out.gpuActionKind = action.gpu.actionKind;
    out.spuActionKind = action.spu.actionKind;
    out.callbackKind = action.callback.kind;

    switch (action.boundary) {
    case ActionBoundary::CdHalBoundary:
        out.directFunction = action.cd.directHelperFunction;
        if (out.directFunction == 0u) {
            out.directFunction = action.cd.callerFunction;
        }
        out.lowerFunction = action.cd.lowerFunction;
        break;
    case ActionBoundary::LoaderMemoryDirect:
        out.directFunction = action.memory.directFunction;
        break;
    case ActionBoundary::GpuHalBoundary:
        out.directFunction = action.gpu.directFunction;
        out.lowerFunction = action.gpu.lowerFunction;
        break;
    case ActionBoundary::SpuHalBoundary:
        out.directFunction = action.spu.wrapperFunction;
        out.lowerFunction = action.spu.lowerFunction;
        break;
    case ActionBoundary::CallbackHalBoundary:
        out.directFunction = action.psxFunction;
        break;
    default:
        out.directFunction = action.gpu.directFunction;
        if (out.directFunction == 0u) {
            out.directFunction = action.memory.directFunction;
        }
        if (out.directFunction == 0u) {
            out.directFunction = action.spu.wrapperFunction;
        }
        break;
    }

    ProducerFeedbackFields fields{};
    switch (action.kind) {
    case ActionKind::DispatchRecord1A8F0:
        out.stepKind = ProducerStepKind::RecordDispatch1A8F0;
        fields.recordType = true;
        fields.recordData = false;
        break;
    case ActionKind::ReadPayload1A818:
        out.stepKind = ProducerStepKind::CdReadPayload1A818;
        fields.cdResult = true;
        break;
    case ActionKind::TimUpload1AE7C:
        out.stepKind = ProducerStepKind::TimUpload1AE7C;
        fields.resolvedPayload = true;
        fields.gpuResult = true;
        break;
    case ActionKind::TimGetInfo40EAC:
    case ActionKind::TimLoadImage44D64:
    case ActionKind::TimDrawSync44B3C:
        out.stepKind = ProducerStepKind::TimGpuBoundary;
        fields.gpuResult = true;
        break;
    case ActionKind::VabClose27120:
    case ActionKind::VabOpen27078:
    case ActionKind::VabTransfer270D4:
    case ActionKind::VabEnable270FC:
    case ActionKind::RetrySfxReset26FA4:
    case ActionKind::RetrySfxCue26EF8:
    case ActionKind::RetrySfxFlush26ECC:
        out.stepKind = ProducerStepKind::VabSfxBoundary;
        fields.spuResult = true;
        break;
    case ActionKind::TempAlloc25B28:
    case ActionKind::Reset25A34:
    case ActionKind::StackAlloc25A70:
    case ActionKind::StackFind25BFC:
    case ActionKind::StackSplit25BBC:
    case ActionKind::StackFree25AF8:
        out.stepKind = ProducerStepKind::MemoryBoundary;
        fields.memoryResult = true;
        break;
    case ActionKind::Callback1537C:
        out.stepKind = ProducerStepKind::Callback1537C;
        fields.callbackResult = true;
        break;
    case ActionKind::CallbackDraw1EA74:
        out.stepKind = ProducerStepKind::CallbackDraw1EA74;
        fields.callbackResult = true;
        break;
    case ActionKind::CallbackFlush26ECC:
        out.stepKind = ProducerStepKind::CallbackFlush26ECC;
        fields.callbackResult = true;
        break;
    case ActionKind::Complete15590:
        out.stepKind = ProducerStepKind::Completion;
        fields.result = true;
        break;
    default:
        switch (action.boundary) {
        case ActionBoundary::CdHalBoundary:
            out.stepKind = ProducerStepKind::CdBoundary;
            fields.cdResult = HasCdSeamPayload(action);
            break;
        case ActionBoundary::PadComHalBoundary:
            out.stepKind = ProducerStepKind::PadComBoundary;
            fields.result = true;
            break;
        case ActionBoundary::CallbackHalBoundary:
            out.stepKind = ProducerStepKind::CallbackBoundary;
            fields.callbackResult = true;
            break;
        case ActionBoundary::DirectControl:
            out.stepKind = ProducerStepKind::DirectControl;
            break;
        default:
            out.stepKind = ProducerStepKind::DirectControl;
            break;
        }
        break;
    }

    if (out.requirements.needsResolvedPayload &&
        (fields.cdResult || fields.gpuResult || fields.spuResult)) {
        fields.resolvedPayload = true;
    }
    out.requiredFields = fields;
    out.category = ClassifyProducerStepKind(out.stepKind);
    out.externalProducerRequired = IsExternalProducerCategory(out.category);
    out.helperGap = IsKnownProducerHelperGap(out.category);
    out.incompleteFeedback = out.helperGap;
    out.canApplyFeedback = !out.helperGap;
    return true;
}

bool HasCdSeamPayload(const Action& action) {
    return action.cd.actionKind != PrStage1LoaderCdHal::ActionKind::None;
}

ProducerCategory ClassifyProducerStepKind(ProducerStepKind stepKind) {
    switch (stepKind) {
    case ProducerStepKind::RecordDispatch1A8F0:
        return ProducerCategory::Record;
    case ProducerStepKind::CdReadPayload1A818:
    case ProducerStepKind::CdBoundary:
        return ProducerCategory::Cd;
    case ProducerStepKind::MemoryBoundary:
        return ProducerCategory::Memory;
    case ProducerStepKind::TimUpload1AE7C:
    case ProducerStepKind::TimGpuBoundary:
        return ProducerCategory::Gpu;
    case ProducerStepKind::VabSfxBoundary:
        return ProducerCategory::Spu;
    case ProducerStepKind::PadComBoundary:
        return ProducerCategory::PadCom;
    case ProducerStepKind::CallbackBoundary:
    case ProducerStepKind::Callback1537C:
    case ProducerStepKind::CallbackDraw1EA74:
    case ProducerStepKind::CallbackFlush26ECC:
        return ProducerCategory::Callback;
    case ProducerStepKind::Completion:
        return ProducerCategory::Completion;
    case ProducerStepKind::DirectControl:
        return ProducerCategory::DirectControl;
    case ProducerStepKind::None:
        return ProducerCategory::None;
    default:
        return ProducerCategory::Unknown;
    }
}

bool IsExternalProducerCategory(ProducerCategory category) {
    switch (category) {
    case ProducerCategory::Record:
    case ProducerCategory::Cd:
    case ProducerCategory::Memory:
    case ProducerCategory::Gpu:
    case ProducerCategory::Spu:
    case ProducerCategory::PadCom:
    case ProducerCategory::Callback:
    case ProducerCategory::Completion:
        return true;
    default:
        return false;
    }
}

bool IsKnownProducerHelperGap(ProducerCategory category) {
    return category == ProducerCategory::DirectControl ||
           category == ProducerCategory::Unknown;
}

bool ProducerStepKindIsCd(const ProducerStep& step) {
    return step.stepKind == ProducerStepKind::CdReadPayload1A818 ||
           step.stepKind == ProducerStepKind::CdBoundary;
}

bool CdFeedbackKindCompletesProducerStep(const ProducerStep& step,
                                         PrStage1LoaderCdHal::ActionKind kind) {
    if (kind == step.cdActionKind) {
        return true;
    }
    return step.cdActionKind ==
               PrStage1LoaderCdHal::ActionKind::ReadStart80038FC0 &&
           kind == PrStage1LoaderCdHal::ActionKind::ReadSync800390C8 &&
           step.action.cd.finalFunction == PrStage1LoaderCdHal::kFn800390C8;
}

bool RequiredFeedbackFieldsPresentForProducerStep(
    const ProducerFeedbackFields& fields,
    const ActionFeedback& feedback,
    bool resultPresent) {
    if (!feedback.completed) {
        return false;
    }
    if (!feedback.success) {
        return true;
    }
    if (fields.result && !resultPresent) {
        return false;
    }
    if (fields.recordType && !feedback.hasRecordType) {
        return false;
    }
    if (fields.recordData && !feedback.hasRecordData) {
        return false;
    }
    if (fields.resolvedPayload && !feedback.hasResolvedPayload) {
        return false;
    }
    if (fields.cdResult && !feedback.hasCdResult) {
        return false;
    }
    if (fields.memoryResult && !feedback.hasMemoryResult) {
        return false;
    }
    if (fields.gpuResult && !feedback.hasGpuResult) {
        return false;
    }
    if (fields.spuResult && !feedback.hasSpuResult) {
        return false;
    }
    if (fields.callbackResult && !feedback.hasCallbackResult) {
        return false;
    }
    return true;
}

bool IsActionFeedbackCompleteForProducerStep(
    const ProducerStep& step,
    const ActionFeedback& feedback,
    bool resultPresent) {
    if (!step.valid) {
        return false;
    }
    if (feedback.psxOrder != step.psxOrder ||
        feedback.kind != step.actionKind) {
        return false;
    }
    return RequiredFeedbackFieldsPresentForProducerStep(
        step.requiredFields,
        feedback,
        resultPresent);
}

bool CdSeamResultHasRequiredPayloadForProducerStep(
    const ProducerStep& step,
    const CdSeamResult& cd) {
    if (!step.valid || !ProducerStepKindIsCd(step) ||
        !step.requiredFields.cdResult || !cd.present) {
        return false;
    }
    if (!cd.feedback.handled ||
        !CdFeedbackKindCompletesProducerStep(step, cd.feedback.kind)) {
        return false;
    }
    if (step.requiredFields.resolvedPayload &&
        (!cd.dstPtrKnown || !cd.sectorCountKnown)) {
        return false;
    }
    return true;
}

bool BuildCdActionFeedbackForProducerStep(
    const ProducerStep& step,
    const CdSeamResult& cd,
    bool success,
    ActionFeedback& out) {
    if (!step.valid || !ProducerStepKindIsCd(step) || !cd.present) {
        out = ActionFeedback{};
        return false;
    }

    out = step.feedbackSkeleton;
    out.completed = true;
    out.success = success;
    out.hasCdResult = true;
    out.cdResult =
        cd.lowerRequest.known && cd.feedback.kind != step.cdActionKind
            ? cd
            : AttachCdLowerActionRequestMetadata(cd, step.action);
    if (step.requiredFields.resolvedPayload &&
        (step.action.resolvedPayload.valid || out.cdResult.dstPtrKnown ||
         out.cdResult.sectorCountKnown)) {
        ResolvedPayload payload = step.action.resolvedPayload;
        payload.recordIndex = step.recordIndex;
        payload.recordType = step.recordType;
        if (out.cdResult.dstPtrKnown) {
            payload.psxAddress = out.cdResult.dstPtr;
            payload.psxAddressKnown = true;
        }
        if (out.cdResult.sectorCountKnown && out.cdResult.sectorCount >= 0) {
            payload.sectorCount =
                static_cast<uint32_t>(out.cdResult.sectorCount);
            payload.sectorCountKnown = true;
            if (!payload.sizeBytesKnown && out.cdResult.sectorCount > 0) {
                payload.sizeBytes =
                    static_cast<uint32_t>(out.cdResult.sectorCount) *
                    static_cast<uint32_t>(
                        PrStage1LoaderCdHal::kReadStart80038FC0SectorSize);
                payload.sizeBytesKnown = true;
            }
        }
        if (out.cdResult.livePayloadBytesKnown) {
            payload.liveBytesPresent = true;
            payload.liveBytesData = out.cdResult.livePayloadData;
            payload.liveBytesSize = out.cdResult.livePayloadSize;
            payload.liveBytesSizeKnown = true;
            if (!payload.sizeBytesKnown) {
                payload.sizeBytes =
                    static_cast<uint32_t>(out.cdResult.livePayloadSize);
                payload.sizeBytesKnown = true;
            }
        }
        payload.valid = payload.psxAddressKnown || payload.sizeBytesKnown ||
                        payload.sectorCountKnown ||
                        payload.liveBytesPresent;
        if (payload.valid) {
            out.hasResolvedPayload = true;
            out.resolvedPayload = payload;
        }
    }
    return true;
}

bool HasMemorySeamPayload(const Action& action) {
    return action.memory.directFunction != 0u;
}

bool HasGpuSeamPayload(const Action& action) {
    return action.gpu.directFunction != 0u ||
           action.gpu.actionKind != PrStage1LoaderGpuHal::GpuActionKind::None;
}

bool HasSpuSeamPayload(const Action& action) {
    return action.spu.actionKind != PrStage1LoaderSpuHal::ActionKind::None;
}

bool TryGetCurrentRecordData(const RunnerState& state, RecordData& out) {
    if (!state.currentRecordDataKnown) {
        out = RecordData{};
        return false;
    }
    out = state.currentRecordData;
    return true;
}

bool TryGetCurrentResolvedPayload(const RunnerState& state,
                                  ResolvedPayload& out) {
    if (!state.currentResolvedPayloadKnown) {
        out = ResolvedPayload{};
        return false;
    }
    out = state.currentResolvedPayload;
    return true;
}

bool TryGetResolvedPayloadForRecord(const RunnerState& state,
                                    uint16_t recordIndex,
                                    ResolvedPayload& out) {
    for (std::size_t i = state.resolvedPayloads.size(); i > 0u; --i) {
        const ResolvedPayload& candidate = state.resolvedPayloads[i - 1u];
        if (candidate.recordIndex == recordIndex) {
            out = candidate;
            return true;
        }
    }
    out = ResolvedPayload{};
    return false;
}

static bool ReadPayloadDwordLE(const ResolvedPayload& payload,
                               uint32_t wordIndex,
                               uint32_t& out) {
    if (!payload.liveBytesPresent || payload.liveBytesData == nullptr ||
        !payload.liveBytesSizeKnown) {
        return false;
    }
    const std::size_t offset =
        static_cast<std::size_t>(wordIndex) * sizeof(uint32_t);
    if (offset + sizeof(uint32_t) > payload.liveBytesSize) {
        return false;
    }
    const uint8_t* p = payload.liveBytesData + offset;
    out = static_cast<uint32_t>(p[0]) |
          (static_cast<uint32_t>(p[1]) << 8) |
          (static_cast<uint32_t>(p[2]) << 16) |
          (static_cast<uint32_t>(p[3]) << 24);
    return true;
}

static LoaderRecordType DecodeRecordType(int32_t value) {
    switch (value) {
    case -1:
        return LoaderRecordType::End;
    case 1:
        return LoaderRecordType::Type1Tim;
    case 2:
        return LoaderRecordType::Type2Vab;
    case 3:
        return LoaderRecordType::Type3Split;
    default:
        return LoaderRecordType::Unknown;
    }
}

static bool TryGetDescriptorPayload(const RunnerState& state,
                                    ResolvedPayload& out) {
    if (state.currentResolvedPayloadKnown &&
        state.currentResolvedPayload.liveBytesPresent &&
        state.currentResolvedPayload.liveBytesData != nullptr &&
        state.currentResolvedPayload.recordType == LoaderRecordType::Unknown) {
        out = state.currentResolvedPayload;
        return true;
    }
    for (std::size_t i = state.resolvedPayloads.size(); i > 0u; --i) {
        const ResolvedPayload& candidate = state.resolvedPayloads[i - 1u];
        if (candidate.liveBytesPresent &&
            candidate.liveBytesData != nullptr &&
            candidate.recordType == LoaderRecordType::Unknown) {
            out = candidate;
            return true;
        }
    }
    out = ResolvedPayload{};
    return false;
}

bool TryBuildRecordDataFromDescriptorPayload(const RunnerState& state,
                                             uint16_t recordIndex,
                                             RecordData& out) {
    out = RecordData{};
    if (!state.planKnown || !state.plan.parsePlan1A8F0.valid) {
        return false;
    }

    ResolvedPayload payload{};
    if (!TryGetDescriptorPayload(state, payload)) {
        return false;
    }

    uint32_t typeWord = 0;
    if (!ReadPayloadDwordLE(payload, 0u, typeWord)) {
        return false;
    }

    out.valid = true;
    out.recordIndex = recordIndex;
    out.recordTypeKnown = true;
    out.recordType = DecodeRecordType(static_cast<int32_t>(typeWord));
    AttachRecordLayout(out, state.plan);

    uint32_t recordCount = 0;
    if (ReadPayloadDwordLE(payload, out.recordCountWord, recordCount)) {
        out.recordCount = static_cast<uint16_t>(
            recordCount > 0xFFFFu ? 0xFFFFu : recordCount);
        out.recordCountKnown = true;
    }

    uint32_t sectorCount = 0;
    if (ReadPayloadDwordLE(payload, out.sectorCountWord, sectorCount)) {
        out.sectorCount = sectorCount;
        out.sectorCountKnown = true;
    }

    out.startSector =
        static_cast<uint32_t>(state.plan.parsePlan1A8F0.headerSeekOffset);
    out.startSectorKnown = true;

    uint32_t payloadBytes = 0;
    if (ReadPayloadDwordLE(payload, out.payloadBytesWord, payloadBytes)) {
        out.payloadBytes = payloadBytes;
        out.payloadBytesKnown = payloadBytes != 0u;
    }

    uint32_t spuBytes = 0;
    if (ReadPayloadDwordLE(payload, out.spuBytesWord, spuBytes)) {
        out.spuBytes = spuBytes;
        out.spuBytesKnown = spuBytes != 0u;
    }
    return true;
}

bool TryGetLastSeamResult(const RunnerState& state, SeamResultCarrier& out) {
    if (state.seamResults.empty()) {
        out = SeamResultCarrier{};
        return false;
    }
    out = state.seamResults.back();
    return true;
}

bool TryGetLastSeamResultForRecord(const RunnerState& state,
                                   uint16_t recordIndex,
                                   SeamResultCarrier& out) {
    for (std::size_t i = state.seamResults.size(); i > 0u; --i) {
        const SeamResultCarrier& candidate = state.seamResults[i - 1u];
        if (candidate.recordIndex == recordIndex) {
            out = candidate;
            return true;
        }
    }
    out = SeamResultCarrier{};
    return false;
}

static bool ResolveMemorySizeBytesForAction(const Action& action,
                                            int32_t& out) {
    const MemorySeamPayload& memory = action.memory;
    if (memory.sizeFromSectorCountWord) {
        if (!action.recordDataResolved ||
            !action.recordData.sectorCountKnown) {
            return false;
        }
        return PrStage1LoaderMemoryDirectSectorCountToBytes(
            action.recordData.sectorCount,
            memory.sectorToBytesShift,
            out);
    }
    if (memory.sizeBytesWord != 0u) {
        if (!action.recordDataResolved ||
            !action.recordData.payloadBytesKnown) {
            return false;
        }
        if (action.recordData.payloadBytes > (uint32_t)0x7FFFFFFF) {
            return false;
        }
        out = (int32_t)action.recordData.payloadBytes;
        return true;
    }
    out = memory.sizeBytes;
    return true;
}

static bool ResolveMemoryPsxAddressForAction(const Action& action,
                                             uint32_t& out) {
    const MemorySeamPayload& memory = action.memory;
    if (memory.addressFromRecordPayloadWord) {
        if (action.payloadResolved && action.resolvedPayload.psxAddressKnown) {
            out = action.resolvedPayload.psxAddress;
            return true;
        }
        if (action.recordDataResolved &&
            action.recordData.payloadPsxAddressKnown) {
            out = action.recordData.payloadPsxAddress;
            return true;
        }
        return false;
    }
    out = memory.psxAddress;
    return true;
}

static bool ResolveMemoryStackIndexForAction(const RunnerState& state,
                                             const Action& action,
                                             int32_t& out) {
    if (!action.memory.stackIndexFromFindFeedback) {
        out = action.memory.stackIndex;
        return true;
    }

    SeamResultCarrier carrier{};
    if (!TryGetLastSeamResultForRecord(state, action.recordIndex, carrier) ||
        !carrier.memory.findKnown) {
        return false;
    }
    if (carrier.memory.findStackIndex <= 0) {
        return false;
    }
    out = carrier.memory.findStackIndex;
    return true;
}

bool BuildMemorySeamResultForProducerStep(
    const RunnerState& state,
    PrStage1LoaderMemoryDirectState& memoryState,
    const ProducerStep& step,
    MemorySeamResult& out) {
    out = MemorySeamResult{};
    if (!step.valid || step.stepKind != ProducerStepKind::MemoryBoundary) {
        return false;
    }

    const Action& action = step.action;
    out.present = true;
    out.directFunction = action.memory.directFunction;

    switch (action.kind) {
    case ActionKind::Reset25A34:
        PrStage1LoaderMemoryDirectReset(memoryState);
        out.resetKnown = true;
        out.resetApplied = true;
        out.tailFunction = kPrStage1LoaderMemoryDirectFn80025A00;
        return true;
    case ActionKind::TempAlloc25B28: {
        int32_t sizeBytes = 0;
        if (!ResolveMemorySizeBytesForAction(action, sizeBytes)) {
            out = MemorySeamResult{};
            return false;
        }
        out.alloc =
            PrStage1LoaderMemoryDirectApply80025B28(memoryState, sizeBytes);
        out.allocKnown = true;
        return true;
    }
    case ActionKind::StackAlloc25A70: {
        int32_t sizeBytes = 0;
        if (!ResolveMemorySizeBytesForAction(action, sizeBytes)) {
            out = MemorySeamResult{};
            return false;
        }
        out.alloc =
            PrStage1LoaderMemoryDirectApply80025A70(memoryState, sizeBytes);
        out.allocKnown = true;
        return true;
    }
    case ActionKind::StackFree25AF8: {
        const uint8_t freeCount = action.memory.freeCount != 0u
                                      ? action.memory.freeCount
                                      : 1u;
        for (uint8_t i = 0u; i < freeCount; ++i) {
            out.freeResult =
                PrStage1LoaderMemoryDirectApply80025AF8(memoryState);
            out.freeKnown = true;
            if (!out.freeResult.popped) {
                return true;
            }
        }
        return true;
    }
    case ActionKind::StackFind25BFC: {
        uint32_t psxAddress = 0;
        if (!ResolveMemoryPsxAddressForAction(action, psxAddress)) {
            out = MemorySeamResult{};
            return false;
        }
        out.findStackIndex =
            PrStage1LoaderMemoryDirectApply80025BFC(memoryState, psxAddress);
        out.findKnown = true;
        return true;
    }
    case ActionKind::StackSplit25BBC: {
        uint32_t psxAddress = 0;
        int32_t stackIndex = 0;
        int32_t sizeBytes = 0;
        if (!ResolveMemoryPsxAddressForAction(action, psxAddress) ||
            !ResolveMemoryStackIndexForAction(state, action, stackIndex) ||
            !ResolveMemorySizeBytesForAction(action, sizeBytes)) {
            out = MemorySeamResult{};
            return false;
        }
        out.split = PrStage1LoaderMemoryDirectApply80025BBC(
            memoryState,
            psxAddress,
            stackIndex,
            sizeBytes);
        out.splitKnown = true;
        return true;
    }
    default:
        out = MemorySeamResult{};
        return false;
    }
}

bool MemorySeamResultSucceededForAction(const Action& action,
                                        const MemorySeamResult& result) {
    switch (action.kind) {
    case ActionKind::Reset25A34:
        return result.resetKnown && result.resetApplied;
    case ActionKind::TempAlloc25B28:
    case ActionKind::StackAlloc25A70:
        return result.allocKnown && result.alloc.success;
    case ActionKind::StackFree25AF8:
        return result.freeKnown && result.freeResult.popped;
    case ActionKind::StackFind25BFC:
        return result.findKnown;
    case ActionKind::StackSplit25BBC:
        return result.splitKnown && result.split.success;
    default:
        return false;
    }
}

CdSeamResult NormalizeCdSeamResultStreamClock800493F4(
    const CdSeamResult& result) {
    return BuildNormalizedCdSeamResultStreamClock800493F4(result);
}

CdSeamResult AttachStreamClockProducerCarrierToCdSeamResult800493F4(
    const CdSeamResult& cd,
    const PrMovieSegmentDirect::StreamClockProducerCarrier800493F4& carrier) {
    CdSeamResult out = cd;
    out.streamClock800493F4 = carrier;
    out.streamClock800493F4Known = carrier.clockKnown;
    out.gapMissingStreamClock800493F4Producer =
        carrier.gapMissingByte800493F4ClockProducer;
    return out;
}

} // namespace PrStage1LoaderDirect
