#pragma once

#include <cstdint>

namespace PrStage1LoaderBootstrapPlanDirect {

struct LoaderReset25A34DirectPlan801C81EC {
    bool valid = false;
    uint32_t function = 0x80025A34u;
    uint32_t clearEndAddress = 0x80092854u;
    uint16_t clearDwordCount = 1024;
    int8_t clearDirectionDwords = -1;
    uint32_t tailFunction = 0x80025A00u;
};

struct LoaderOpenFile1A324DirectPlan801C81EC {
    bool valid = false;
    uint32_t function = 0x8001A324u;
    uint8_t loadedFlagWord = 3;
    uint8_t descriptorPresentWord = 0;
    uint8_t descriptorPayloadFirstWord = 4;
    uint8_t sizeBytesWord = 5;
    uint8_t startSectorWord = 10;
    uint8_t endSectorWord = 11;
    uint8_t sectorShift = 11;
    uint32_t probeFunction = 0x8001A2B0u;
    uint32_t probeLowerLookupFunction = 0x800381F8u;
    uint8_t probeRetryCount = 4;
    uint32_t resolveStartSectorFunction = 0x80036A78u;
    int32_t alreadyLoadedOrMissingReturn = 0;
    int32_t probeFailedReturn = -1;
    int32_t openedReturn = 0;
    bool probeReturnsOriginalPointer = true;
    bool probeZeroMeansFailure = true;
};

struct LoaderSeek1A89CDirectPlan801C81EC {
    bool valid = false;
    uint32_t function = 0x8001A89Cu;
    uint32_t resolveStartSectorFunction = 0x80036A78u;
    uint32_t seekSectorFunction = 0x80036974u;
    uint32_t syncFunction = 0x800367A4u;
    int32_t syncMode = 2;
    int32_t syncArg2 = 0;
    int32_t lbaToMsfBiasSectors = 150;
    bool addsOffsetToResolvedSector = true;
    bool seekConvertsLbaToBcdMsf = true;
    bool syncFunctionIsCdWrapperBoundary = true;
};

struct LoaderRead1A818DirectPlan801C81EC {
    bool valid = false;
    uint32_t function = 0x8001A818u;
    uint32_t readStartFunction = 0x80038FC0u;
    uint32_t resetInputFunction = 0x80035560u;
    uint32_t readSyncFunction = 0x800390C8u;
    int32_t resetInputArg = 3;
    int32_t syncArg0 = 1;
    int32_t syncArg1 = 0;
    uint8_t streamedModeFlag = 0x80;
    int32_t successSyncResult = 0;
    int32_t readStartSectorSize = 512;
    int32_t readStartTransferUnit = 32;
    int32_t readSyncTimeoutVblanks = 1200;
    int32_t readSyncPumpAfterVblanks = 60;
    uint32_t readSyncFinalFunction = 0x800364F0u;
    bool readStartFunctionIsCdHalWrapper = true;
    bool resetInputFunctionIsVblankHal = true;
    bool readSyncFunctionIsCdHalWrapper = true;
};

struct LoaderTempAlloc25B28DirectPlan801C81EC {
    bool valid = false;
    uint32_t function = 0x80025B28u;
    uint8_t alignmentBytes = 8;
    int32_t positiveRoundAdd = 7;
    int32_t negativeRoundAdd = 14;
    bool topDownFromHeapEnd = true;
    bool updatesLowWater = true;
    uint16_t lowWaterGpOffset = 320;
    uint16_t heapEndGpOffset = 900;
    uint16_t heapCursorGpOffset = 904;
};

struct LoaderStackAlloc25A70DirectPlan801C81EC {
    bool valid = false;
    uint32_t function = 0x80025A70u;
    uint32_t stackTableAddress = 0x80091858u;
    uint8_t alignmentBytes = 8;
    int32_t positiveRoundAdd = 7;
    int32_t negativeRoundAdd = 14;
    uint32_t maxRequestBytes = 0x12CFFFu;
    uint16_t maxStackDepth = 1024;
    uint16_t stackDepthGpOffset = 324;
    uint16_t heapEndGpOffset = 900;
    uint16_t heapCursorGpOffset = 904;
    bool storesOldCursorAtDepthPlusOne = true;
};

struct LoaderStackFree25AF8DirectPlan801C81EC {
    bool valid = false;
    uint32_t function = 0x80025AF8u;
    uint32_t stackTableAddress = 0x80091858u;
    uint16_t stackDepthGpOffset = 324;
    uint16_t heapCursorGpOffset = 904;
    bool clearsCurrentStackEntry = true;
    int8_t stackDepthDelta = -1;
};

struct LoaderStackFind25BFCDirectPlan801C81EC {
    bool valid = false;
    uint32_t function = 0x80025BFCu;
    uint32_t stackTableFirstEntryAddress = 0x8009185Cu;
    uint16_t stackDepthGpOffset = 324;
    uint16_t firstIndex = 1;
    int32_t missingReturn = 0;
};

struct LoaderStackSplit25BBCDirectPlan801C81EC {
    bool valid = false;
    uint32_t function = 0x80025BBCu;
    uint32_t stackTableAddress = 0x80091858u;
    uint16_t stackDepthGpOffset = 324;
    uint16_t heapEndGpOffset = 900;
    uint16_t heapCursorGpOffset = 904;
    bool storesBlockAtRequestedIndex = true;
    bool updatesDepthToRequestedIndex = true;
    int32_t successReturn = 1;
    int32_t overflowReturn = 0;
};

struct LoaderTimUpload1AE7CDirectPlan801C81EC {
    bool valid = false;
    uint32_t function = 0x8001AE7Cu;
    uint32_t timInfoFunction = 0x80040EACu;
    uint32_t uploadRectFunction = 0x80044D64u;
    uint32_t drawSyncFunction = 0x80044B3Cu;
    uint32_t lowerUploadCommandFunction = 0x800468E0u;
    uint32_t lowerDrawSyncFunction = 0x80046FFCu;
    uint8_t timInfoPayloadOffset = 4;
    uint8_t clutFlagShift = 3;
    uint8_t clutFlagMask = 1;
    int32_t drawSyncArg = 0;
    bool uploadsPixelRectFirst = true;
    bool drawsSyncAfterPixelUpload = true;
    bool uploadsClutWhenFlagSet = true;
    bool drawsSyncAfterClutUpload = true;
    bool uploadFunctionIsGpuHalBoundary = true;
    bool drawSyncFunctionIsGpuHalBoundary = true;
};

struct LoaderSpuVabDirectPlan801C81EC {
    bool valid = false;
    uint32_t closeFunction = 0x80027120u;
    uint32_t openFunction = 0x80027078u;
    uint32_t transferFunction = 0x800270D4u;
    uint32_t enableFunction = 0x800270FCu;
    uint32_t lowLevelCloseFunction = 0x8002DF80u;
    uint32_t lowLevelOpenFunction = 0x8002E3D8u;
    uint32_t lowLevelTransferFunction = 0x8002EB80u;
    uint32_t lowLevelEnableFunction = 0x8002EEFCu;
    uint32_t currentVabIdAddress = 0x800943A8u;
    int32_t closedVabSentinel = -1;
    int32_t openArg1 = -1;
    int32_t enableArg = 1;
    bool closeOnlyWhenCurrentVabOpen = true;
    bool transferOnlyWhenOpenSucceeds = true;
    bool closeWritesClosedSentinel = true;
    bool openStoresReturnedVabId = true;
    bool openSuccessIsNonNegativeVabId = true;
    bool enableArgIsBoolean = true;
};

struct LoaderRetrySfxDirectPlan801C81EC {
    bool valid = false;
    uint32_t resetFunction = 0x80026FA4u;
    uint32_t cueFunction = 0x80026EF8u;
    uint32_t flushFunction = 0x80026ECCu;
    uint32_t lowLevelResetFunction = 0x800351B8u;
    uint32_t lowLevelCueFunction = 0x80034240u;
    uint32_t lowLevelFlushFunction = 0x8002EFF4u;
    uint32_t currentVabIdAddress = 0x800943A8u;
    uint32_t currentVoiceAddress = 0x800943ACu;
    uint32_t flushBusyFlagAddress = 0x800943B4u;
    uint32_t cueTableAddress = 0x80094410u;
    uint8_t cuePitchAdd = 24;
    uint8_t cuePitchByteOffset = 2;
    uint8_t cueVolumeByteOffset = 3;
    int32_t resetArg = 0;
    bool writesCurrentVoice = true;
    bool flushOnlyWhenBusyFlagClear = true;
    bool resetIgnoresCallerArg = true;
    bool cueMutatesPitchByte = true;
    bool cueUsesCurrentVabId = true;
    bool lowerFunctionsAreSfxHalBoundaries = true;
};

struct LoaderPadStartComDirectPlan801C81EC {
    bool valid = false;
    uint32_t function = 0x80026E4Cu;
    bool callsSsInitHot = true;
    uint32_t lowerInit4096Function = 0x8002DA78u;
    uint32_t lowerInitB130Function = 0x8002B130u;
    uint32_t lowerInitA6ACFunction = 0x8002A6ACu;
    uint32_t lowerInitAA90Function = 0x8002AA90u;
    uint32_t lowerInitAB24Function = 0x8002AB24u;
    int32_t lowerInit4096Arg = 4096;
    int32_t lowerInitA6ACArg0 = 90;
    int32_t lowerInitA6ACArg1 = 90;
    uint32_t currentVabIdAddress = 0x800943A8u;
    uint32_t currentVabSlotAddress = 0x800943AAu;
    uint32_t currentVoiceAddress = 0x800943ACu;
    uint32_t flushBusyFlagAddress = 0x800943B4u;
    int32_t currentVabIdInit = 0;
    int32_t currentVabSlotInit = -1;
    int32_t currentVoiceInit = -1;
    int32_t flushBusyFlagInit = 0;
    int32_t returnValue = -1;
    bool lowerCallsAreInitHalBoundaries = true;
};

struct LoaderParse1A8F0DirectPlan801C81EC {
    bool valid = false;
    uint32_t function = 0x8001A8F0u;
    uint32_t seekFunction = 0x8001A89Cu;
    uint32_t readFunction = 0x8001A818u;
    uint32_t tempAllocFunction = 0x80025B28u;
    uint32_t payloadAllocFunction = 0x80025A70u;
    uint32_t freeFunction = 0x80025AF8u;
    uint32_t tempAllocBytes = 0x2000u;
    uint8_t headerReadBytes = 4;
    int32_t initialSeekOffset = 0;
    int32_t headerSeekOffset = 4;
    int32_t recordTypeEnd = -1;
    int32_t recordTypeRegisterBlocks = 1;
    int32_t recordTypePadSpu = 2;
    int32_t recordTypeSplitBlocks = 3;
    uint16_t unknownTypeErrorGpOffset = 744;
    uint8_t recordCountWord = 1;
    uint8_t payloadSectorCountWord = 2;
    uint8_t recordPayloadFirstWord = 4;
    uint8_t recordStrideWords = 5;
    uint8_t type2PayloadBytesWord = 4;
    uint8_t type2SpuBytesWord = 9;
    uint8_t type1AllocSectorCountWord = 2;
    uint8_t type1SeekSectorCountWord = 2;
    uint8_t type3AllocSectorCountWord = 2;
    uint8_t type3SeekSectorCountWord = 2;
    uint8_t sectorToBytesShift = 11;
    uint8_t type2SuccessFreeCount = 1;
    uint8_t type2FailureFreeCount = 2;
    uint8_t type1FreeCount = 1;
    int32_t successReturn = 1;
    int32_t failureReturn = 0;
    bool type2PadStartComGap = true;
    bool type2RetrySfxOnlyWhenMode1 = true;
    bool type3RequiresFoundStackBase = true;
    bool type3SplitIndexStartsAtFoundBase = true;
    uint32_t type1RegisterHelper = 0x8001AE7Cu;
    uint32_t type3FindHeaderHelper = 0x80025BFCu;
    uint32_t type3SplitBlockHelper = 0x80025BBCu;
    uint32_t type2ComInitFunction = 0x80027120u;
    uint32_t type2PadStartComFunction = 0x80026E4Cu;
    uint32_t type2SpuProbeFunction = 0x80027078u;
    uint32_t type2SpuLoadFunction = 0x800270D4u;
    uint32_t type2SpuEnableFunction = 0x800270FCu;
    uint32_t retryAudioResetFunction = 0x80026FA4u;
    uint32_t retrySfxFunction = 0x80026EF8u;
    uint32_t retryAudioFlushFunction = 0x80026ECCu;
    uint32_t retrySfxCueTableAddress = 0x80094410u;
    LoaderSeek1A89CDirectPlan801C81EC seekPlan1A89C;
    LoaderRead1A818DirectPlan801C81EC readPlan1A818;
    LoaderTempAlloc25B28DirectPlan801C81EC tempAllocPlan25B28;
    LoaderStackAlloc25A70DirectPlan801C81EC stackAllocPlan25A70;
    LoaderStackFree25AF8DirectPlan801C81EC stackFreePlan25AF8;
    LoaderStackFind25BFCDirectPlan801C81EC stackFindPlan25BFC;
    LoaderStackSplit25BBCDirectPlan801C81EC stackSplitPlan25BBC;
    LoaderTimUpload1AE7CDirectPlan801C81EC timUploadPlan1AE7C;
    LoaderSpuVabDirectPlan801C81EC spuVabPlan;
    LoaderRetrySfxDirectPlan801C81EC retrySfxPlan;
    LoaderPadStartComDirectPlan801C81EC padStartComPlan;
};

struct LoaderDraw1EA74DirectPlan801C81EC {
    bool valid = false;
    uint32_t function = 0x8001EA74u;
    uint32_t displayIndexFunction = 0x8004019Cu;
    uint32_t displayTableAddress = 0x8006ED50u;
    uint32_t setDrawEnvFunction = 0x80040F90u;
    uint32_t drawBaseFunction = 0x80040CC8u;
    uint32_t defaultPatternFunction = 0x8001EF40u;
    uint32_t mode1Function = 0x8001F524u;
    uint32_t mode2Function = 0x8001FCBCu;
    uint32_t mode3SetupFunction = 0x80022CBCu;
    uint32_t mode3Or4TextFunction = 0x8001D74Cu;
    uint32_t mode3Or4DrawFunction = 0x8001FC40u;
    uint32_t mode4SetupFunction = 0x80021E60u;
    uint32_t mode2To4FlushFunction = 0x8001FDC0u;
    uint32_t mode5Function = 0x80020308u;
    uint32_t mode6Function = 0x80020248u;
    uint32_t subtitleFlagAddress = 0x800916DCu;
    uint32_t patternModeAddress = 0x800916E0u;
    uint16_t currentDisplayIndexGpOffset = 872;
    uint16_t drawActiveFlagGpOffset = 792;
};

struct LoaderPattern1EF40DirectPlan801C81EC {
    bool valid = false;
    uint32_t function = 0x8001EF40u;
    uint8_t gridColumns = 16;
    uint8_t gridRows = 12;
    uint8_t tileWidth = 20;
    uint8_t tileHeight = 20;
    uint8_t advanceEveryFrames = 3;
    uint8_t bitsPerPatternWord = 32;
    uint32_t liveGridAddress = 0x80087330u;
    uint32_t liveGridLastColumnAddress = 0x8008736Cu;
    uint32_t liveGridEndAddress = 0x8008766Cu;
    uint32_t tableMode0Address = 0x80050730u;
    uint32_t tableMode1Address = 0x80050790u;
    uint32_t tableMode2Address = 0x800507F0u;
    uint32_t tableMode3Address = 0x80050850u;
    uint32_t frameLimitTableAddress = 0x800508B4u;
    uint32_t highlightTextureTableAddress = 0x800508B0u;
    uint32_t tileIndexTableAddress = 0x80050420u;
    uint32_t tileTextureTableAddress = 0x80050720u;
    uint32_t drawHighlightFunction = 0x8001C4ECu;
    uint32_t drawTileFunction = 0x8001C550u;
    uint16_t bitCursorGpOffset = 49;
    uint16_t wordCursorGpOffset = 50;
    uint16_t frameCounterGpOffset = 51;
};

struct Bootstrap15590DirectPlan801C81EC {
    bool valid = false;
    uint32_t wrapperFunction = 0x80015590u;
    uint8_t sceneId = 0;
    uint32_t rawSceneArg = 0;
    uint32_t startCurtainFunction = 0x80015408u;
    int16_t curtainMode15408 = 3;
    uint32_t curtainModeAddress15408 = 0x800916E0u;
    uint32_t curtainCallbackFlagAddress = 0x8006ECD4u;
    int32_t curtainStartFlagValue15408 = 0;
    uint32_t callbackFunction = 0x8001537Cu;
    uint32_t callbackDrawFunction1537C = 0x8001EA74u;
    int32_t callbackDrawArg0_1537C = 1;
    int32_t callbackDrawArg1_1537C = 0;
    uint32_t callbackFlushFunction1537C = 0x80026ECCu;
    int32_t callbackFlagValue1537C = 1;
    int32_t callbackReturnValue1537C = 1;
    uint32_t loaderFunction = 0x8001AC18u;
    uint32_t loaderOffsetFromSceneEntry = 0x12Cu;
    int32_t loaderMode1AC18 = 1;
    uint8_t loaderMaxAttempts1AC18 = 4;
    int32_t loaderFirstAttemptParserFlag1AC18 = 1;
    int32_t loaderRetryParserFlag1AC18 = 0;
    uint8_t loaderCopiedDescriptorFirstWord1AC18 = 4;
    uint8_t loaderCopiedDescriptorWordCount1AC18 = 6;
    uint32_t resetDiscFunction = 0x80025A34u;
    uint32_t openFileFunction = 0x8001A324u;
    uint32_t parseIntFunction = 0x8001A8F0u;
    uint32_t stopCurtainFunction = 0x8001545Cu;
    bool sceneLoaderSlotKnown = false;
    bool sceneLoaderSlotPresent = false;
    uint8_t sceneLoaderSlot = 0;
    bool sceneLoaderRecordKnown = false;
    uint8_t sceneLoaderRowIndex = 0;
    bool sceneLoaderPsxAddrKnown = false;
    uint32_t sceneLoaderPsxAddr = 0;
    bool sceneLoaderPathPtrKnown = false;
    uint32_t sceneLoaderPathPtr = 0;
    bool bootstrapZcompoLoader = false;
    LoaderReset25A34DirectPlan801C81EC resetPlan25A34;
    LoaderOpenFile1A324DirectPlan801C81EC openFilePlan1A324;
    LoaderParse1A8F0DirectPlan801C81EC parsePlan1A8F0;
    LoaderDraw1EA74DirectPlan801C81EC callbackDrawPlan1EA74;
    LoaderPattern1EF40DirectPlan801C81EC patternDrawPlan1EF40;
};

} // namespace PrStage1LoaderBootstrapPlanDirect
