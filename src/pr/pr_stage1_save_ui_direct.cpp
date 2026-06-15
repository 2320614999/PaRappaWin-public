#include "pr_stage1_save_ui_direct.h"

#include "logger.h"
#include "pr_game_context.h"
#include "pr_pad.h"
#include "pr_stage_payload_bank_direct.h"
#include "pr_stage1_scorer_direct.h"

#include <array>
#include <cstddef>
#include <cstring>
#include <string>

namespace {

static constexpr int32_t kStateExit80018FB0 = 23;
static constexpr int32_t kInitialState80019148 = 21;
static constexpr int32_t kInitialEvent80019148 = 11;
static constexpr int32_t kInputCross800185D0 = 64;
static constexpr int32_t kInputCircle800185D0 = 32;
static constexpr int32_t kInputTriangle800185D0 = 16;
static constexpr int32_t kInputNameMask800185D0 = 0xE0;
static constexpr int32_t kInputNameUp800185D0 = 0x1000;
static constexpr int32_t kInputNameLeft800185D0 = 0x4000;
static constexpr int32_t kInputNameRight800185D0 = 0x2000;
static constexpr int32_t kInputNameDown800185D0 = 0x8000;
static constexpr int32_t kSaveUiListRowCount800181D0 = 15;
static constexpr int32_t kSaveUiListEntrySize80019458 = 108;
static constexpr int32_t kSaveUiDirEntryCount80019458 = 15;
static constexpr int32_t kSaveUiDirEntrySize80019458 = 40;
static constexpr int32_t kSaveUiSavePayloadBytes80019458 = 4876;
static constexpr int32_t kSaveUiSaveHeaderBytes80017C08 = 512;
static constexpr int32_t kSaveUiSaveBlockBytes80017C08 = 0x2000;
static constexpr int32_t kSaveUiEncodedTitleBytes80017C08 = 64;
static constexpr int32_t kSaveUiWriteRetryCount80017A10 = 4;
static constexpr int32_t kSaveUiFormatRetryCount80017B60 = 3;
static constexpr int32_t kSaveUiWriteEventPollCount80016EB8 = 300;
static constexpr int32_t kSaveUiWriteBlocks80019458 = 1;
static constexpr int32_t kSaveUiCardBlockShift80017454 = 13;
static constexpr int32_t kSaveUiNameInputEventArg80018060 = 0x80049244;
static constexpr int32_t kSaveUiNameRawBuffer80018060 = 0x80049260;
static constexpr int32_t kSaveUiNamePreviewTarget80017FC4 = 0x8004926C;
static constexpr int32_t kSaveUiListEventArg80019458 = 0x80048E50;
static constexpr int32_t kSaveUiFilenameBuffer80019458 = 0x8007CBE8;
static constexpr int32_t kSaveUiEncodedTitleBuffer80019458 = 0x8007CC08;
static constexpr uint32_t kAddrSaveUiGpBase80019458 = 0x8006EA40u;
static constexpr uint32_t kAddrSavePayloadMirrorSrc8008EEF8 =
    PrStagePayloadBankDirect::kMirrorSrcAddress8008EEF8;
static constexpr uint32_t kAddrSaveDataBuffer80092F10 =
    PrStagePayloadBankDirect::kBaseAddress80092F10;
static constexpr uint32_t kAddrSaveStatusBackup80079008 = 0x80079008u;
static constexpr uint32_t kAddrSavePayloadProgressBase80092F1D =
    PrStagePayloadBankDirect::kStatusBaseAddress80092F1D;
static constexpr uint32_t kAddrSavePayloadCarrierIndex80092F3C =
    PrStagePayloadBankDirect::kCarrierIndexAddress80092F3C;
static constexpr uint32_t kAddrSavePayloadCarrierA380092F40 =
    PrStagePayloadBankDirect::kCarrierA3Address80092F40;
static constexpr uint32_t kAddrSavePayloadCarrierComplete80092F44 =
    PrStagePayloadBankDirect::kCarrierCompleteAddress80092F44;
static constexpr uint32_t kAddrSavePayloadCarrierSource80092F48 =
    PrStagePayloadBankDirect::kCarrierSourceAddress80092F48;
static constexpr uint32_t kAddrSavePayloadMirrorDst80092F5C =
    PrStagePayloadBankDirect::kMirrorDstAddress80092F5C;
static constexpr uint32_t kAddrTitleCharClass8006E999 = 0x8006E999u;
static constexpr uint32_t kAddrSaveUiGpSource8006EAC8 = 0x8006EAC8u;
static constexpr uint32_t kAddrSaveFilenamePrefix800101E0 = 0x800101E0u;
static constexpr uint32_t kAddrSaveTitlePrefix8006EAD8 = 0x8006EAD8u;
static constexpr uint32_t kAddrSaveFilenamePrefixPtr80019458 =
    kAddrSaveUiGpBase80019458 + 0x88u;
static constexpr uint32_t kAddrSaveTitlePrefixPtr80019458 =
    kAddrSaveUiGpBase80019458 + 0xA0u;
static constexpr uint32_t kAddrHeaderIconSource80010004 = 0x80010004u;
static constexpr uint32_t kAddrNameGlyphState800490E8 = 0x800490E8u;
static constexpr uint32_t kAddrGlyphMap800491C4 = 0x800491C4u;
static constexpr uint32_t kAddrDefaultNamePreview8006EAF0 = 0x8006EAF0u;
static constexpr uint32_t kAddrListEventArg80048E50 = 0x80048E50u;
static constexpr uint32_t kAddrDirBank8007A318 = 0x8007A318u;
static constexpr uint32_t kAddrSaveBuffers8007ABE8 = 0x8007ABE8u;
static constexpr uint32_t kAddrSavePayloadDst8007ADE8 = 0x8007ADE8u;
static constexpr uint32_t kAddrPreviousSnapshot8007CC74 = 0x8007CC74u;
static constexpr uint32_t kAddrSaveBuffersEnd8007CECC = 0x8007CECCu;
static constexpr uint32_t kFn80017E58 = 0x80017E58u;
static constexpr uint32_t kFn80017E6C = 0x80017E6Cu;
static constexpr uint32_t kFn80018060 = 0x80018060u;
static constexpr uint32_t kFn800180D8 = 0x800180D8u;
static constexpr uint32_t kFn800181D0 = 0x800181D0u;
static constexpr uint32_t kFn800185D0 = 0x800185D0u;
static constexpr uint32_t kFn80018FB0 = 0x80018FB0u;
static constexpr uint32_t kFn80019458 = 0x80019458u;
static constexpr uint32_t kFn80015700 = 0x80015700u;
static constexpr uint32_t kFn80015744 = 0x80015744u;
static constexpr uint32_t kFn80017594 = 0x80017594u;
static constexpr uint32_t kFn80035510 = 0x80035510u;
static constexpr uint32_t kFn80017B08 = 0x80017B08u;
static constexpr uint32_t kFn80017B18 = 0x80017B18u;
static constexpr uint32_t kFn80017B60 = 0x80017B60u;
static constexpr uint32_t kFn800178C8 = 0x800178C8u;
static constexpr uint32_t kFn80017354 = 0x80017354u;
static constexpr uint32_t kFn80017900 = 0x80017900u;
static constexpr uint32_t kFn80017454 = 0x80017454u;
static constexpr uint32_t kFn80016FC0 = 0x80016FC0u;
static constexpr uint32_t kFn80016E18 = 0x80016E18u;
static constexpr uint32_t kFn80016EB8 = 0x80016EB8u;
static constexpr uint32_t kFn8001707C = 0x8001707Cu;
static constexpr uint32_t kFn80017008 = 0x80017008u;
static constexpr uint32_t kFn80047EE4 = 0x80047EE4u;
static constexpr uint32_t kFn80017C08 = 0x80017C08u;
static constexpr uint32_t kFn80017A10 = 0x80017A10u;
static constexpr uint32_t kFn80017FC4 = 0x80017FC4u;
static constexpr uint32_t kFn80015CC4 = 0x80015CC4u;
static constexpr uint32_t kFn800164B4 = 0x800164B4u;
static constexpr uint32_t kFn8001635C = 0x8001635Cu;
static constexpr uint32_t kFn800169E0 = 0x800169E0u;
static constexpr uint32_t kFn8001628C = 0x8001628Cu;
static constexpr uint32_t kFn8001615C = 0x8001615Cu;
static constexpr uint32_t kFn800161A8 = 0x800161A8u;
static constexpr uint32_t kFn800161F4 = 0x800161F4u;
static constexpr uint32_t kFn800166AC = 0x800166ACu;
static constexpr uint32_t kFn800167A8 = 0x800167A8u;
static constexpr uint32_t kFn80025C44 = 0x80025C44u;
static constexpr uint32_t kFn80025C64 = 0x80025C64u;
static constexpr uint32_t kFn80025C8C = 0x80025C8Cu;
static constexpr uint32_t kFn8004800C = 0x8004800Cu;
static constexpr uint32_t kFn800488E4 = 0x800488E4u;
static constexpr uint32_t kFn8001E750 = 0x8001E750u;
static constexpr uint32_t kFn80035560 = 0x80035560u;
static constexpr uint32_t kFn8001EA00 = 0x8001EA00u;
static constexpr uint32_t kHeaderTitleOffset80017C08 = 4u;
static constexpr uint32_t kHeaderFixedDwordsOffset80017C08 = 96u;
static constexpr uint32_t kHeaderIconChunk0DstOffset80017C08 = 128u;
static constexpr uint32_t kHeaderIconChunk1DstOffset80017C08 = 256u;
static constexpr uint32_t kHeaderIconChunk2DstOffset80017C08 = 384u;
static constexpr uint32_t kHeaderIconChunk0Src80017C08 =
    kAddrHeaderIconSource80010004 + 8u * 4u;
static constexpr uint32_t kHeaderIconChunk1Src80017C08 =
    kAddrHeaderIconSource80010004 + 40u * 4u;
static constexpr uint32_t kHeaderIconChunk2Src80017C08 =
    0x800100C4u + 24u * 4u;
static constexpr int32_t kHeaderIconChunkBytes80017C08 = 128;
static constexpr int32_t kSaveUiTitleSuffixMax80019458 = 12;
static constexpr int32_t kSaveUiTitleLocalBytes80019458 = 32;
static constexpr int32_t kStage1SaveUiStageNumber80016314 = 1;
static constexpr int32_t kSavePayloadMirrorBytes8001635C =
    PrStagePayloadBankDirect::kMirrorBytes8001635C;
static constexpr uint32_t kGlyphEncodeTableBase80019458 =
    0x80049124u - 16u * 4u;

static constexpr char kNameInputCharTable800490E8[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890-!@#$&%^{()_+,.}[]\b\n";

static constexpr uint8_t kHeaderIconSource80010004[508] = {
    0x20, 0x45, 0x58, 0x45, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x90, 0x85, 0x02, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x80,
    0x00, 0xF0, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0xFF, 0x1F, 0x80,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x53, 0x6F, 0x6E, 0x79,
    0x20, 0x43, 0x6F, 0x6D, 0x70, 0x75, 0x74, 0x65, 0x72, 0x20, 0x45, 0x6E,
    0x74, 0x65, 0x72, 0x74, 0x61, 0x69, 0x6E, 0x6D, 0x65, 0x6E, 0x74, 0x20,
    0x49, 0x6E, 0x63, 0x2E, 0x20, 0x66, 0x6F, 0x72, 0x20, 0x4E, 0x6F, 0x72,
    0x74, 0x68, 0x20, 0x41, 0x6D, 0x65, 0x72, 0x69, 0x63, 0x61, 0x20, 0x61,
    0x72, 0x65, 0x61, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
};

static_assert(sizeof(kHeaderIconSource80010004) == 508,
              "80010004 static slice size must match manifest");

static constexpr uint8_t kTitleCharClass8006E999[256] = {
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x08, 0x08, 0x08,
    0x08, 0x08, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x18, 0x10, 0x10, 0x10,
    0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
    0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x10, 0x10,
    0x10, 0x10, 0x10, 0x10, 0x10, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x10, 0x10, 0x10, 0x10, 0x10,
    0x10, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x02, 0x02, 0x02, 0x02, 0x02,
    0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
    0x02, 0x02, 0x02, 0x10, 0x10, 0x10, 0x10, 0x20, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x70,
    0x10, 0x80, 0x1F, 0x00, 0x11, 0x80, 0x1F, 0x10, 0x00, 0x00, 0x00, 0x20,
    0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0B, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x10, 0x29, 0x09, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x04, 0x1C, 0x5A, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x05, 0x1D, 0x5A, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x06,
    0x1E, 0x5A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x1F, 0x5A, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x08, 0x20, 0x5A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09,
    0x21, 0x5A, 0x00,
};

static_assert(sizeof(kTitleCharClass8006E999) == 256,
              "8006E999 char-class table size must match IDA dump");

static constexpr uint8_t kSaveUiGpSource8006EAC8[32] = {
    0xE0, 0x01, 0x01, 0x80, 0x50, 0x41, 0x52, 0x41,
    0x50, 0x50, 0x00, 0x00, 0xCC, 0xEA, 0x06, 0x80,
    0x50, 0x41, 0x52, 0x41, 0x50, 0x50, 0x41, 0x00,
    0xD8, 0xEA, 0x06, 0x80, 0x25, 0x73, 0x25, 0x73,
};

static_assert(sizeof(kSaveUiGpSource8006EAC8) == 32,
              "8006EAC8 source slice must cover gp+0x88 and gp+0xA0");

static constexpr uint8_t kSaveFilenamePrefix800101E0[] = {
    0x42, 0x41, 0x53, 0x43, 0x55, 0x53, 0x2D,
    0x39, 0x34, 0x31, 0x38, 0x33, 0x00,
};

static_assert(sizeof(kSaveFilenamePrefix800101E0) == 13,
              "800101E0 filename prefix must cover BASCUS-94183");

static constexpr uint8_t kNameGlyphState800490E8[] = {
    0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xC6, 0x24, 0x03, 0x00, 0xC0, 0x1C,
    0x01, 0x00, 0xA5, 0x24, 0x4A, 0x22, 0x01, 0x08, 0x21, 0x10, 0x00, 0x00,
    0x00, 0x00, 0x83, 0x90, 0x00, 0x00, 0xA2, 0x90, 0x00, 0x00, 0x00, 0x00,
    0xF7, 0xFF, 0x62, 0x10, 0x01, 0x00, 0x84, 0x24, 0xFF, 0xFF, 0x84, 0x24,
    0x00, 0x00, 0x83, 0x90, 0x00, 0x00, 0xA2, 0x90, 0x00, 0x00, 0x00, 0x00,
    0x23, 0x10, 0x62, 0x00, 0x08, 0x00, 0xE0, 0x03, 0x00, 0x00, 0x00, 0x00,
    0xA0, 0x00, 0x0A, 0x24, 0x08, 0x00, 0x40, 0x01, 0x39, 0x00, 0x09, 0x24,
    0x00, 0x00, 0x00, 0x00, 0xA0, 0x00, 0x0A, 0x24, 0x08, 0x00, 0x40, 0x01,
    0x44, 0x00, 0x09, 0x24, 0x00, 0x00, 0x00, 0x00, 0xA0, 0x00, 0x0A, 0x24,
    0x08, 0x00, 0x40, 0x01, 0x49, 0x00, 0x09, 0x24, 0x00, 0x00, 0x00, 0x00,
    0xA0, 0x00, 0x0A, 0x24, 0x08, 0x00, 0x40, 0x01, 0x72, 0x00, 0x09, 0x24,
    0x00, 0x00, 0x00, 0x00, 0xA0, 0x00, 0x0A, 0x24, 0x08, 0x00, 0x40, 0x01,
    0x9F, 0x00, 0x09, 0x24, 0x00, 0x00, 0x00, 0x00, 0xB0, 0x00, 0x0A, 0x24,
    0x08, 0x00, 0x40, 0x01, 0x07, 0x00, 0x09, 0x24, 0x00, 0x00, 0x00, 0x00,
    0xB0, 0x00, 0x0A, 0x24, 0x08, 0x00, 0x40, 0x01, 0x08, 0x00, 0x09, 0x24,
    0x00, 0x00, 0x00, 0x00, 0xB0, 0x00, 0x0A, 0x24, 0x08, 0x00, 0x40, 0x01,
    0x09, 0x00, 0x09, 0x24, 0x00, 0x00, 0x00, 0x00, 0xB0, 0x00, 0x0A, 0x24,
    0x08, 0x00, 0x40, 0x01, 0x0A, 0x00, 0x09, 0x24, 0x00, 0x00, 0x00, 0x00,
    0xB0, 0x00, 0x0A, 0x24, 0x08, 0x00, 0x40, 0x01, 0x0B, 0x00, 0x09, 0x24,
    0x00, 0x00, 0x00, 0x00, 0xB0, 0x00, 0x0A, 0x24, 0x08, 0x00, 0x40, 0x01,
    0x0C, 0x00, 0x09, 0x24, 0x00, 0x00, 0x00, 0x00, 0xB0, 0x00, 0x0A, 0x24,
    0x08, 0x00, 0x40, 0x01, 0x14, 0x00, 0x09, 0x24, 0x00, 0x00, 0x00, 0x00,
    0xB0, 0x00, 0x0A, 0x24, 0x08, 0x00, 0x40, 0x01, 0x15, 0x00, 0x09, 0x24,
    0x00, 0x00, 0x00, 0x00, 0xB0, 0x00, 0x0A, 0x24, 0x08, 0x00, 0x40, 0x01,
    0x16, 0x00, 0x09, 0x24, 0x00, 0x00, 0x00, 0x00, 0xB0, 0x00, 0x0A, 0x24,
    0x08, 0x00, 0x40, 0x01, 0x17, 0x00, 0x09, 0x24, 0x00, 0x00, 0x00, 0x00,
    0xB0, 0x00, 0x0A, 0x24, 0x08, 0x00, 0x40, 0x01, 0x18, 0x00, 0x09, 0x24,
    0x00, 0x00, 0x00, 0x00, 0xB0, 0x00, 0x0A, 0x24, 0x08, 0x00, 0x40, 0x01,
    0x19, 0x00, 0x09, 0x24, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x04, 0x24,
    0x0C, 0x00, 0x00, 0x00, 0x08, 0x00, 0xE0, 0x03, 0x00, 0x00, 0x00, 0x00,
    0x02, 0x00, 0x04, 0x24, 0x0C, 0x00, 0x00, 0x00, 0x08, 0x00, 0xE0, 0x03,
    0x00, 0x00, 0x00, 0x00, 0xB0, 0x00, 0x0A, 0x24, 0x08, 0x00, 0x40, 0x01,
    0x32, 0x00, 0x09, 0x24, 0x00, 0x00, 0x00, 0x00, 0xB0, 0x00, 0x0A, 0x24,
    0x08, 0x00, 0x40, 0x01, 0x34, 0x00, 0x09, 0x24, 0x00, 0x00, 0x00, 0x00,
    0xB0, 0x00, 0x0A, 0x24, 0x08, 0x00, 0x40, 0x01, 0x35, 0x00, 0x09, 0x24,
    0x00, 0x00, 0x00, 0x00, 0xB0, 0x00, 0x0A, 0x24, 0x08, 0x00, 0x40, 0x01,
    0x36, 0x00, 0x09, 0x24, 0x00, 0x00, 0x00, 0x00, 0xB0, 0x00, 0x0A, 0x24,
    0x08, 0x00, 0x40, 0x01, 0x41, 0x00, 0x09, 0x24, 0x00, 0x00, 0x00, 0x00,
    0xB0, 0x00, 0x0A, 0x24, 0x08, 0x00, 0x40, 0x01, 0x42, 0x00, 0x09, 0x24,
    0x00, 0x00, 0x00, 0x00, 0xB0, 0x00, 0x0A, 0x24, 0x08, 0x00, 0x40, 0x01,
    0x43, 0x00, 0x09, 0x24, 0x00, 0x00, 0x00, 0x00, 0xB0, 0x00, 0x0A, 0x24,
    0x08, 0x00, 0x40, 0x01, 0x45, 0x00, 0x09, 0x24, 0x00, 0x00, 0x00, 0x00,
    0xB0, 0x00, 0x0A, 0x24, 0x08, 0x00, 0x40, 0x01, 0x5B, 0x00, 0x09, 0x24,
    0x00, 0x00, 0x00, 0x00, 0xC0, 0x00, 0x0A, 0x24, 0x08, 0x00, 0x40, 0x01,
    0x0A, 0x00, 0x09, 0x24, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x88, 0x30,
    0x03, 0x00, 0x02, 0x29, 0x03, 0x00, 0x40, 0x14, 0x48, 0x00, 0x07, 0x34,
    0xE6, 0x22, 0x01, 0x08, 0x21, 0x10, 0x00, 0x00, 0x07, 0x80, 0x02, 0x3C,
    0x2C, 0xEA, 0x42, 0x8C, 0x00, 0x19, 0x08, 0x00,
};

static_assert(sizeof(kNameGlyphState800490E8) == 572,
              "800490E8 static slice size must match manifest");

static constexpr uint8_t kDefaultNamePreview8006EAF0[] = {
    0x08, 0x00, 0xDD, 0x03, 0x08, 0x00, 0xDD, 0x03, 0x08, 0x00, 0x22, 0x00,
    0x08, 0x00, 0x22, 0x00, 0x08, 0x00, 0xDE, 0x03, 0x08, 0x00, 0xDE, 0x03,
    0x08, 0x00, 0x21, 0x00, 0x08, 0x00, 0x21, 0x00, 0x08, 0x00, 0xDF, 0x03,
    0x08, 0x00, 0xDF, 0x03, 0x08, 0x00, 0x20, 0x00, 0x08, 0x00, 0x20, 0x00,
    0x08, 0x00, 0xE0, 0x03, 0x08, 0x00, 0xE0, 0x03, 0x08, 0x00, 0x0E, 0x04,
    0x08, 0x00, 0x0E, 0x04, 0x08, 0x00, 0xF2, 0x07, 0x08, 0x00, 0xF2, 0x07,
    0x08, 0x00, 0x0D, 0x04, 0x08, 0x00, 0x0D, 0x04, 0x08, 0x00, 0xF3, 0x07,
    0x08, 0x00, 0xF3, 0x07, 0x08, 0x00, 0x0C, 0x04, 0x08, 0x00, 0x0C, 0x04,
    0x08, 0x00, 0xF4, 0x07, 0x08, 0x00, 0xF4, 0x07, 0x08, 0x00, 0x0B, 0x04,
    0x08, 0x00, 0x0B, 0x04, 0x08, 0x00, 0xF5, 0x07, 0x08, 0x00, 0xF5, 0x07,
    0x08, 0x00, 0x0A, 0x04, 0x08, 0x00, 0x0A, 0x04, 0x08, 0x00, 0xF6, 0x07,
    0x08, 0x00, 0xF6, 0x07, 0x08, 0x00, 0x09, 0x04, 0x08, 0x00, 0x09, 0x04,
};

static_assert(sizeof(kDefaultNamePreview8006EAF0) == 144,
              "8006EAF0 static slice size must match manifest");

static constexpr uint8_t kListEventArg80048E50[] = {
    0x2A, 0x10, 0x02, 0x02, 0xFB, 0xFF, 0x40, 0x14, 0xFF, 0xFF, 0x31, 0x26,
    0x01, 0x00, 0x31, 0x26, 0x10, 0x02, 0xA2, 0x8F, 0x00, 0x00, 0x00, 0x00,
    0x04, 0x00, 0x42, 0x30, 0x43, 0x00, 0x40, 0x10, 0x30, 0x00, 0x02, 0x34,
    0xFF, 0xFF, 0x31, 0x26, 0x00, 0x00, 0x25, 0xA2, 0xFF, 0xFF, 0x31, 0x26,
    0x02, 0x00, 0x10, 0x26, 0xDF, 0x21, 0x01, 0x08, 0x00, 0x00, 0x22, 0xA2,
    0x20, 0x02, 0xA2, 0x8F, 0xFF, 0xFF, 0x31, 0x26, 0x04, 0x00, 0x43, 0x24,
    0x20, 0x02, 0xA3, 0xAF, 0x00, 0x00, 0x42, 0x90, 0x01, 0x00, 0x10, 0x34,
    0xDF, 0x21, 0x01, 0x08, 0x00, 0x00, 0x22, 0xA2, 0x20, 0x02, 0xA2, 0x8F,
    0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x43, 0x24, 0x20, 0x02, 0xA3, 0xAF,
    0x10, 0x02, 0xA3, 0x8F, 0x00, 0x00, 0x51, 0x8C, 0x04, 0x00, 0x62, 0x30,
    0x0B, 0x00, 0x40, 0x10, 0x10, 0x00, 0x62, 0x30,
};

static_assert(sizeof(kListEventArg80048E50) == 128,
              "80048E50 static slice size must match manifest");

struct SaveUiDirectMemory {
    std::array<uint8_t, 508> headerIconSource{};
    std::array<uint8_t, 256> titleCharClass{};
    std::array<uint8_t, 32> saveUiGpSource{};
    std::array<uint8_t, 13> saveFilenamePrefix{};
    std::array<uint8_t, 572> nameGlyphState{};
    std::array<uint8_t, 144> defaultNamePreview{};
    std::array<uint8_t, 128> listEventArg{};
    std::array<uint8_t, kAddrSaveBuffers8007ABE8 - kAddrDirBank8007A318>
        dirBank{};
    std::array<uint8_t, kAddrSaveBuffersEnd8007CECC - kAddrSaveBuffers8007ABE8>
        saveBuffers{};
    PrStagePayloadBankDirect::MemoryState80092F10 payloadBank{};
};

struct SaveUi19148Runtime {
    bool active = false;
    int32_t state = kInitialState80019148;
    int32_t eventId = kInitialEvent80019148;
    int32_t gp708_lastInput = 0;
    int32_t gp716_saveOk = 0;
    int32_t gp720_result = 0;
    int32_t gp724_ioResult = 0;
    PrStage1SaveUiCardIoState80017594 cardIoState80017594{};
    int32_t gp728_blinkCounter = 0;
    int32_t gp732_mode = 0;
    int32_t gp712_overwriteScanFlag = 0;
    char saveSuffix[32]{};
    int32_t word8004925A = 0;
    int32_t word8004925C = 0;
    int32_t word8004925E = 0;
    int32_t dword80049248 = 0;
    char byte80049260[96]{};
    char encodedNamePreview[96]{};
    int32_t word8007ABE4_entryCount = 0;
    int32_t word8007ABE6_freeSlots = 0;
    int32_t word80048E62_entryCount = 0;
    int32_t word80048E64_selected = 15;
    int32_t word80048E84_dirty = 0;
    bool listRowActive[kSaveUiListRowCount800181D0]{};
    bool listRowFreeSlot[kSaveUiListRowCount800181D0]{};
    int32_t listRowBlockIndex[kSaveUiListRowCount800181D0]{};
    char listRowSuffix[kSaveUiListRowCount800181D0][32]{};
    char byte8007CBE8[128]{};
    bool byte8007CBE8Present = false;
    bool encodedTitleBufferPresent = false;
    bool saveHeaderBuilt = false;
    uint32_t savePayloadSourceAddress = kAddrSaveDataBuffer80092F10;
    bool savePayloadCopied = false;
    bool savePayloadSourceKnown = false;
    // PSX BSS global dword_800901BC is known-zero until an event table writer
    // publishes a replay mirror count.
    bool dword800901BCKnown = true;
    uint32_t dword800901BC = 0;
    bool helperGap = false;
    bool dispatcherPending = false;
    int32_t dispatcherEventId = 0;
    int32_t dispatcherResultPending = 0;
};

static SaveUi19148Runtime s_saveUi19148{};
static SaveUiDirectMemory s_saveUiMemory{};
static bool s_saveUiReplayMirrorSourceKnown = false;
static PrStage1ScorerDirectReplayBufferState s_saveUiReplayMirrorSource{};

static bool SpanContains(uint32_t start,
                         size_t size,
                         uint32_t address,
                         size_t count) {
    const uint64_t begin = start;
    const uint64_t end = begin + size;
    const uint64_t accessBegin = address;
    const uint64_t accessEnd = accessBegin + count;
    return accessBegin >= begin && accessEnd <= end && accessEnd >= accessBegin;
}

static uint8_t* DirectMemoryPtr(uint32_t address, size_t count) {
    if (SpanContains(kAddrNameGlyphState800490E8,
                     s_saveUiMemory.nameGlyphState.size(),
                     address,
                     count)) {
        return s_saveUiMemory.nameGlyphState.data() +
               (address - kAddrNameGlyphState800490E8);
    }
    if (SpanContains(kAddrDefaultNamePreview8006EAF0,
                     s_saveUiMemory.defaultNamePreview.size(),
                     address,
                     count)) {
        return s_saveUiMemory.defaultNamePreview.data() +
               (address - kAddrDefaultNamePreview8006EAF0);
    }
    if (SpanContains(kAddrListEventArg80048E50,
                     s_saveUiMemory.listEventArg.size(),
                     address,
                     count)) {
        return s_saveUiMemory.listEventArg.data() +
               (address - kAddrListEventArg80048E50);
    }
    if (SpanContains(kAddrDirBank8007A318,
                     s_saveUiMemory.dirBank.size(),
                     address,
                     count)) {
        return s_saveUiMemory.dirBank.data() + (address - kAddrDirBank8007A318);
    }
    if (SpanContains(kAddrSaveBuffers8007ABE8,
                     s_saveUiMemory.saveBuffers.size(),
                     address,
                     count)) {
        return s_saveUiMemory.saveBuffers.data() +
               (address - kAddrSaveBuffers8007ABE8);
    }
    if (uint8_t* payloadPtr =
            PrStagePayloadBankDirect::DirectMemoryPtr80092F10(
                s_saveUiMemory.payloadBank,
                address,
                count)) {
        return payloadPtr;
    }
    s_saveUiMemory.payloadBank.boundsFault = true;
    s_saveUiMemory.payloadBank.lastFaultAddress = address;
    return nullptr;
}

static const uint8_t* DirectMemoryReadPtr(uint32_t address, size_t count) {
    if (SpanContains(kAddrSaveFilenamePrefix800101E0,
                     s_saveUiMemory.saveFilenamePrefix.size(),
                     address,
                     count)) {
        return s_saveUiMemory.saveFilenamePrefix.data() +
               (address - kAddrSaveFilenamePrefix800101E0);
    }
    if (SpanContains(kAddrHeaderIconSource80010004,
                     s_saveUiMemory.headerIconSource.size(),
                     address,
                     count)) {
        return s_saveUiMemory.headerIconSource.data() +
               (address - kAddrHeaderIconSource80010004);
    }
    if (SpanContains(kAddrTitleCharClass8006E999,
                     s_saveUiMemory.titleCharClass.size(),
                     address,
                     count)) {
        return s_saveUiMemory.titleCharClass.data() +
               (address - kAddrTitleCharClass8006E999);
    }
    if (SpanContains(kAddrSaveUiGpSource8006EAC8,
                     s_saveUiMemory.saveUiGpSource.size(),
                     address,
                     count)) {
        return s_saveUiMemory.saveUiGpSource.data() +
               (address - kAddrSaveUiGpSource8006EAC8);
    }
    return DirectMemoryPtr(address, count);
}

static bool DirectReadByte(uint32_t address, uint8_t& value) {
    const uint8_t* ptr = DirectMemoryReadPtr(address, 1);
    if (!ptr) {
        return false;
    }
    value = *ptr;
    return true;
}

static bool DirectWriteByte(uint32_t address, uint8_t value) {
    uint8_t* ptr = DirectMemoryPtr(address, 1);
    if (!ptr) {
        return false;
    }
    *ptr = value;
    return true;
}

static bool DirectWriteLe32(uint32_t address, uint32_t value) {
    uint8_t* ptr = DirectMemoryPtr(address, sizeof(value));
    if (!ptr) {
        return false;
    }
    ptr[0] = static_cast<uint8_t>(value & 0xFFu);
    ptr[1] = static_cast<uint8_t>((value >> 8) & 0xFFu);
    ptr[2] = static_cast<uint8_t>((value >> 16) & 0xFFu);
    ptr[3] = static_cast<uint8_t>((value >> 24) & 0xFFu);
    return true;
}

static bool IsReplayMirrorSourceShapeKnown(
    const PrStage1ScorerDirectReplayBufferState& replay) {
    const uint32_t capacity =
        static_cast<uint32_t>(kPrStage1ScorerDirectReplayBufferCapacity);
    return replay.replayMirrorKnown8008EEF8 &&
           replay.replayMirrorProducerKnown8008EEF8 &&
           PrStage1ScorerDirectIsKnownReplayMirrorProducerFunction(
               replay.replayMirrorProducerFunction) &&
           replay.replayMirrorByteCountKnown8008EEF8 &&
           replay.replayMirrorKnownByteCount8008EEF8 >=
               kPrStage1ScorerDirectReplayMirrorByteCount &&
           replay.dword901BCPublishedCount <= capacity &&
           replay.dword901C0WriteCount <= capacity;
}

static void HydrateReplayMirrorSource() {
    if (!s_saveUiReplayMirrorSourceKnown) {
        return;
    }

    if (!IsReplayMirrorSourceShapeKnown(s_saveUiReplayMirrorSource)) {
        s_saveUiReplayMirrorSourceKnown = false;
        s_saveUiMemory.payloadBank.replayMirrorKnown8008EEF8 = false;
        s_saveUiMemory.payloadBank.replayMirrorProducerKnown8008EEF8 = false;
        s_saveUiMemory.payloadBank.replayMirrorProducerFunction = 0;
        s_saveUiMemory.payloadBank.replayMirrorByteCountKnown8008EEF8 = false;
        s_saveUiMemory.payloadBank.replayMirrorKnownByteCount8008EEF8 = 0;
        return;
    }

    const uint32_t publishedCount =
        s_saveUiReplayMirrorSource.dword901BCPublishedCount;
    s_saveUi19148.dword800901BCKnown = true;
    s_saveUi19148.dword800901BC = publishedCount;

    for (size_t i = 0; i < kPrStage1ScorerDirectReplayBufferCapacity; ++i) {
        const uint32_t recordAddr =
            kAddrSavePayloadMirrorSrc8008EEF8 +
            static_cast<uint32_t>(i * 2u * sizeof(uint32_t));
        (void)DirectWriteLe32(recordAddr,
                              s_saveUiReplayMirrorSource.dwordEEF8Tick96[i]);
        (void)DirectWriteLe32(
            recordAddr + sizeof(uint32_t),
            s_saveUiReplayMirrorSource.dwordEEFCClassMask[i]);
    }
    s_saveUiMemory.payloadBank.replayMirrorKnown8008EEF8 = true;
    s_saveUiMemory.payloadBank.replayMirrorProducerKnown8008EEF8 = true;
    s_saveUiMemory.payloadBank.replayMirrorProducerFunction =
        s_saveUiReplayMirrorSource.replayMirrorProducerFunction;
    s_saveUiMemory.payloadBank.replayMirrorByteCountKnown8008EEF8 = true;
    s_saveUiMemory.payloadBank.replayMirrorKnownByteCount8008EEF8 =
        s_saveUiReplayMirrorSource.replayMirrorKnownByteCount8008EEF8;
}

[[maybe_unused]] static bool DirectReadLe16(uint32_t address, uint16_t& value) {
    uint8_t lo = 0;
    uint8_t hi = 0;
    if (!DirectReadByte(address, lo) || !DirectReadByte(address + 1u, hi)) {
        return false;
    }
    value = static_cast<uint16_t>(lo | (static_cast<uint16_t>(hi) << 8));
    return true;
}

static bool DirectReadLe32(uint32_t address, uint32_t& value) {
    uint8_t b0 = 0;
    uint8_t b1 = 0;
    uint8_t b2 = 0;
    uint8_t b3 = 0;
    if (!DirectReadByte(address, b0) || !DirectReadByte(address + 1u, b1) ||
        !DirectReadByte(address + 2u, b2) ||
        !DirectReadByte(address + 3u, b3)) {
        return false;
    }
    value = static_cast<uint32_t>(b0) |
            (static_cast<uint32_t>(b1) << 8u) |
            (static_cast<uint32_t>(b2) << 16u) |
            (static_cast<uint32_t>(b3) << 24u);
    return true;
}

static void MarkSavePayloadWriter80092F10(uint32_t function) {
    PrStagePayloadBankDirect::MarkPayloadWriter80092F10(
        s_saveUiMemory.payloadBank,
        function);
}

static void ResetDirectMemory() {
    s_saveUiMemory = SaveUiDirectMemory{};
    std::memcpy(s_saveUiMemory.headerIconSource.data(),
                kHeaderIconSource80010004,
                sizeof(kHeaderIconSource80010004));
    std::memcpy(s_saveUiMemory.titleCharClass.data(),
                kTitleCharClass8006E999,
                sizeof(kTitleCharClass8006E999));
    std::memcpy(s_saveUiMemory.saveUiGpSource.data(),
                kSaveUiGpSource8006EAC8,
                sizeof(kSaveUiGpSource8006EAC8));
    std::memcpy(s_saveUiMemory.saveFilenamePrefix.data(),
                kSaveFilenamePrefix800101E0,
                sizeof(kSaveFilenamePrefix800101E0));
    std::memcpy(s_saveUiMemory.nameGlyphState.data(),
                kNameGlyphState800490E8,
                sizeof(kNameGlyphState800490E8));
    std::memcpy(s_saveUiMemory.defaultNamePreview.data(),
                kDefaultNamePreview8006EAF0,
                sizeof(kDefaultNamePreview8006EAF0));
    std::memcpy(s_saveUiMemory.listEventArg.data(),
                kListEventArg80048E50,
                sizeof(kListEventArg80048E50));
    HydrateReplayMirrorSource();
}

static bool ImportSaveStatusPrefix80092F10Internal(
    const PrStage1SaveStatusPrefix80092F10& seed) {
    if (!seed.known ||
        seed.helperGap ||
        !seed.statusBankKnown80092F1D ||
        seed.psxAddress != PrStage1SaveStatusPrefix80092F10::kPsxAddress ||
        seed.byteCount != PrStage1SaveStatusPrefix80092F10::kByteCount) {
        s_saveUiMemory.payloadBank.savePayloadBankKnown = false;
        s_saveUiMemory.payloadBank.statusBankKnown80092F1D = false;
        s_saveUiMemory.payloadBank.savePayloadBankLastWriterFunction = 0;
        s_saveUiMemory.payloadBank.wrote80015CC4 = false;
        s_saveUiMemory.payloadBank.wrote800164B4 = false;
        s_saveUiMemory.payloadBank.wrote8001635C = false;
        s_saveUiMemory.payloadBank.wrote8001628C = false;
        s_saveUiMemory.payloadBank.wrote800167A8 = false;
        s_saveUiMemory.payloadBank.wrote80015744 = false;
        s_saveUiMemory.payloadBank.boundsFault = true;
        s_saveUiMemory.payloadBank.lastFaultAddress = seed.psxAddress;
        s_saveUi19148.savePayloadSourceKnown = false;
        return false;
    }

    std::memcpy(s_saveUiMemory.payloadBank.savePayloadBank.data(),
                seed.bytes,
                PrStage1SaveStatusPrefix80092F10::kByteCount);
    s_saveUiMemory.payloadBank.savePayloadBankKnown = true;
    s_saveUiMemory.payloadBank.statusBankKnown80092F1D = seed.statusBankKnown80092F1D;
    s_saveUiMemory.payloadBank.savePayloadBankLastWriterFunction =
        seed.lastWriterFunction;
    s_saveUiMemory.payloadBank.wrote80015CC4 = seed.wrote80015CC4;
    s_saveUiMemory.payloadBank.wrote800164B4 = seed.wrote800164B4;
    s_saveUiMemory.payloadBank.wrote8001635C = seed.wrote8001635C;
    s_saveUiMemory.payloadBank.wrote8001628C = seed.wrote8001628C;
    s_saveUiMemory.payloadBank.wrote800167A8 = seed.wrote800167A8;
    s_saveUiMemory.payloadBank.wrote80015744 = seed.wrote80015744;
    s_saveUiMemory.payloadBank.boundsFault = seed.helperGap;
    s_saveUiMemory.payloadBank.lastFaultAddress = seed.lastFaultAddress;
    s_saveUi19148.savePayloadSourceAddress =
        PrStage1SaveStatusPrefix80092F10::kPsxAddress;
    s_saveUi19148.savePayloadSourceKnown = true;
    s_saveUi19148.helperGap = seed.helperGap;
    return true;
}

static void MarkInitialSavePayloadBssKnown80092F10() {
    s_saveUiMemory.payloadBank.savePayloadBankKnown = true;
    s_saveUiMemory.payloadBank.statusBankKnown80092F1D = true;
    s_saveUiMemory.payloadBank.boundsFault = false;
    s_saveUiMemory.payloadBank.lastFaultAddress = 0;
    s_saveUi19148.savePayloadSourceAddress = kAddrSaveDataBuffer80092F10;
    s_saveUi19148.savePayloadSourceKnown = true;
}

PrStage1SaveUi19148ActionHostBoundary ClassifyActionHostBoundary(
    PrStage1SaveUi19148ActionKind kind);

void AppendAction(PrStage1SaveUi19148TickResult& out,
                  PrStage1SaveUi19148ActionKind kind,
                  uint32_t psxFunction,
                  int32_t stateBefore,
                  int32_t stateAfter,
                  int32_t arg0 = 0,
                  int32_t arg1 = 0,
                  int32_t arg2 = 0,
                  int32_t arg3 = 0,
                  int32_t arg4 = 0) {
    if (kind == PrStage1SaveUi19148ActionKind::None) {
        return;
    }
    if (out.actions.count >=
        static_cast<uint32_t>(sizeof(out.actions.actions) /
                              sizeof(out.actions.actions[0]))) {
        out.actions.truncated = true;
        s_saveUi19148.helperGap = true;
        out.helperGap = true;
        return;
    }
    PrStage1SaveUi19148Action& action =
        out.actions.actions[out.actions.count++];
    action.kind = kind;
    action.hostBoundary = ClassifyActionHostBoundary(kind);
    action.psxFunction = psxFunction;
    action.stateBefore = stateBefore;
    action.stateAfter = stateAfter;
    action.arg0 = arg0;
    action.arg1 = arg1;
    action.arg2 = arg2;
    action.arg3 = arg3;
    action.arg4 = arg4;
}

void RewriteLastCardIoRequestState80017594(
    PrStage1SaveUi19148TickResult& out,
    const PrStage1SaveUiCardIoState80017594& state) {
    for (uint32_t i = out.actions.count; i > 0; --i) {
        PrStage1SaveUi19148Action& action = out.actions.actions[i - 1];
        if (action.kind !=
            PrStage1SaveUi19148ActionKind::Call80017594PollCardIo) {
            continue;
        }
        action.arg0 = state.dword800917E8;
        action.arg1 = state.dword800917EC;
        action.arg2 = state.dword800917F0;
        action.arg3 = state.dword800917F4;
        action.arg4 = state.gp700;
        return;
    }
}

PrStage1SaveUi19148ActionHostBoundary ClassifyActionHostBoundary(
    PrStage1SaveUi19148ActionKind kind) {
    switch (kind) {
    case PrStage1SaveUi19148ActionKind::None:
    case PrStage1SaveUi19148ActionKind::Call80017E58InitEventArg:
    case PrStage1SaveUi19148ActionKind::Call80017E6CSetEventResult:
    case PrStage1SaveUi19148ActionKind::Call80018060InitNameInput:
    case PrStage1SaveUi19148ActionKind::Call80017B08EnumerateEntries:
    case PrStage1SaveUi19148ActionKind::Call80017B18SnapshotDirectory:
    case PrStage1SaveUi19148ActionKind::Call80025C64CopySavePayload:
    case PrStage1SaveUi19148ActionKind::Call80015700BackupSaveStatusPrefix:
    case PrStage1SaveUi19148ActionKind::Call80019458ConsumePrefixState15:
    case PrStage1SaveUi19148ActionKind::Call80019458CommitResultState15:
    case PrStage1SaveUi19148ActionKind::Call80015744RestoreSaveStatusPrefix:
    case PrStage1SaveUi19148ActionKind::Call80015CC4InitSavePayload:
    case PrStage1SaveUi19148ActionKind::Call8001615CMapSaveStage:
    case PrStage1SaveUi19148ActionKind::Call800161A8MapScene:
    case PrStage1SaveUi19148ActionKind::Call800164B4LoadSavePayload:
    case PrStage1SaveUi19148ActionKind::Call8001628CEnsureSaveProgress:
    case PrStage1SaveUi19148ActionKind::Call8001635CUpdateSavePayload:
    case PrStage1SaveUi19148ActionKind::Call800169E0SyncSavedScore:
    case PrStage1SaveUi19148ActionKind::Call800181D0ListInput:
        return PrStage1SaveUi19148ActionHostBoundary::DirectMemory;
    case PrStage1SaveUi19148ActionKind::Call80025C8CPlayInputSfx:
    case PrStage1SaveUi19148ActionKind::Call80016FC0ClearSwCardEvents:
    case PrStage1SaveUi19148ActionKind::Call80016E18PollCardInfoLoad:
    case PrStage1SaveUi19148ActionKind::Call80047EE4ResetHwCardEvents:
    case PrStage1SaveUi19148ActionKind::Call80016EB8PollSwCardEvents:
    case PrStage1SaveUi19148ActionKind::Call8001707CDrainHwCardEvents:
    case PrStage1SaveUi19148ActionKind::Call80017008PollHwCardEvents:
    case PrStage1SaveUi19148ActionKind::Call80035560CardWait:
    case PrStage1SaveUi19148ActionKind::Call8001E750DrawEvent:
    case PrStage1SaveUi19148ActionKind::Call80035560ResetInput:
    case PrStage1SaveUi19148ActionKind::Call8001EA00EndFrame:
        return PrStage1SaveUi19148ActionHostBoundary::HostHalBoundary;
    case PrStage1SaveUi19148ActionKind::HelperGap:
        return PrStage1SaveUi19148ActionHostBoundary::HelperGap;
    case PrStage1SaveUi19148ActionKind::Call80017B60FormatCard:
    case PrStage1SaveUi19148ActionKind::Call80017A10WriteSaveBlock:
    case PrStage1SaveUi19148ActionKind::Call80017594PollCardIo:
    case PrStage1SaveUi19148ActionKind::Call80017900ScanCardDirectory:
    case PrStage1SaveUi19148ActionKind::Call80017454SubmitWrite:
    case PrStage1SaveUi19148ActionKind::CardHalOpenCheck:
    case PrStage1SaveUi19148ActionKind::CardHalOpenWrite:
    case PrStage1SaveUi19148ActionKind::CardHalWrite:
    case PrStage1SaveUi19148ActionKind::CardHalClose:
    case PrStage1SaveUi19148ActionKind::CardHalFormat:
    case PrStage1SaveUi19148ActionKind::CardHalInfo:
    case PrStage1SaveUi19148ActionKind::CardHalLoad:
        return PrStage1SaveUi19148ActionHostBoundary::HostHalBoundary;
    }
    return PrStage1SaveUi19148ActionHostBoundary::UnsupportedHostAction;
}

bool AppendHostActionRequest(
    PrStage1SaveUi19148HostActionRequestList& out,
    const PrStage1SaveUi19148Action& action,
    PrStage1SaveUi19148HostActionRequestKind kind,
    PrStage1SaveUi19148ActionGapReason gapReason =
        PrStage1SaveUi19148ActionGapReason::None) {
    if (kind == PrStage1SaveUi19148HostActionRequestKind::None) {
        return true;
    }
    if (out.count >=
        static_cast<uint32_t>(sizeof(out.requests) / sizeof(out.requests[0]))) {
        out.truncated = true;
        return false;
    }
    PrStage1SaveUi19148HostActionRequest& request = out.requests[out.count++];
    request.kind = kind;
    request.action = action;
    request.gapReason = gapReason;
    if (kind == PrStage1SaveUi19148HostActionRequestKind::PlayInputSfx) {
        request.sfxCue = static_cast<uint16_t>(action.arg0);
    }
    return true;
}

bool AppendLowerFeedbackRequest(
    PrStage1SaveUi19148LowerFeedbackRequestList& out,
    const PrStage1SaveUi19148Action& action,
    PrStage1SaveUi19148LowerFeedbackRequestKind kind) {
    if (kind == PrStage1SaveUi19148LowerFeedbackRequestKind::None) {
        return true;
    }
    if (out.count >=
        static_cast<uint32_t>(sizeof(out.requests) / sizeof(out.requests[0]))) {
        out.truncated = true;
        return false;
    }

    PrStage1SaveUi19148LowerFeedbackRequest& request =
        out.requests[out.count++];
    request.kind = kind;
    request.action = action;
    request.psxFunction = action.psxFunction;
    request.stateBefore = action.stateBefore;
    request.stateAfter = action.stateAfter;
    if (kind ==
        PrStage1SaveUi19148LowerFeedbackRequestKind::Write80017A10) {
        request.nameAddress = static_cast<uint32_t>(action.arg0);
        request.dataAddress = static_cast<uint32_t>(action.arg1);
        request.blockCount = action.arg2;
        request.retryCount = action.arg3;
        request.writeCloseGp696FactRequired80017A10 = true;
        request.writeCloseGp696Address80017A10 =
            static_cast<uint32_t>(0x8006ECF8u);
        request.writeFdMustMatchCloseGp69680017A10 = true;
    } else if (kind ==
               PrStage1SaveUi19148LowerFeedbackRequestKind::Format80017B60) {
        request.formatArg0 = static_cast<uint32_t>(action.arg0);
        request.formatArg1 = static_cast<uint32_t>(action.arg1);
        request.retryCount = action.arg2;
    } else if (kind ==
               PrStage1SaveUi19148LowerFeedbackRequestKind::CardIo80017594) {
        request.cardIoState.dword800917E8 = action.arg0;
        request.cardIoState.dword800917EC = action.arg1;
        request.cardIoState.dword800917F0 = action.arg2;
        request.cardIoState.dword800917F4 = action.arg3;
        request.cardIoState.gp700 = action.arg4;
    }
    return true;
}

PrStage1SaveUi19148ActionGapReason GapReasonForHostAction19148(
    const PrStage1SaveUi19148Action& action) {
    if (action.hostBoundary ==
        PrStage1SaveUi19148ActionHostBoundary::UnsupportedHostAction) {
        return PrStage1SaveUi19148ActionGapReason::UnsupportedHostAction;
    }
    if (action.hostBoundary ==
        PrStage1SaveUi19148ActionHostBoundary::HelperGap) {
        return PrStage1SaveUi19148ActionGapReason::DirectHelperGap;
    }

    switch (action.kind) {
    case PrStage1SaveUi19148ActionKind::Call80016FC0ClearSwCardEvents:
    case PrStage1SaveUi19148ActionKind::Call80016E18PollCardInfoLoad:
    case PrStage1SaveUi19148ActionKind::Call80047EE4ResetHwCardEvents:
    case PrStage1SaveUi19148ActionKind::Call80016EB8PollSwCardEvents:
    case PrStage1SaveUi19148ActionKind::Call8001707CDrainHwCardEvents:
    case PrStage1SaveUi19148ActionKind::Call80017008PollHwCardEvents:
        return PrStage1SaveUi19148ActionGapReason::CardEventHalNotPorted;

    case PrStage1SaveUi19148ActionKind::Call80017B60FormatCard:
    case PrStage1SaveUi19148ActionKind::Call80017A10WriteSaveBlock:
    case PrStage1SaveUi19148ActionKind::Call80017594PollCardIo:
    case PrStage1SaveUi19148ActionKind::Call80017900ScanCardDirectory:
    case PrStage1SaveUi19148ActionKind::Call80017454SubmitWrite:
    case PrStage1SaveUi19148ActionKind::CardHalOpenCheck:
    case PrStage1SaveUi19148ActionKind::CardHalOpenWrite:
    case PrStage1SaveUi19148ActionKind::CardHalWrite:
    case PrStage1SaveUi19148ActionKind::CardHalClose:
    case PrStage1SaveUi19148ActionKind::CardHalFormat:
    case PrStage1SaveUi19148ActionKind::CardHalInfo:
    case PrStage1SaveUi19148ActionKind::CardHalLoad:
        return PrStage1SaveUi19148ActionGapReason::
            CardFilesystemHostActionUnsupported;

    case PrStage1SaveUi19148ActionKind::HelperGap:
        return PrStage1SaveUi19148ActionGapReason::DirectHelperGap;

    default:
        break;
    }
    return PrStage1SaveUi19148ActionGapReason::None;
}

void AppendHelperGap(PrStage1SaveUi19148TickResult& out,
                     uint32_t psxFunction,
                     int32_t stateBefore,
                     int32_t stateAfter,
                     int32_t gapId,
                     int32_t arg1 = 0,
                     int32_t arg2 = 0,
                     int32_t arg3 = 0) {
    s_saveUi19148.helperGap = true;
    out.helperGap = true;
    AppendAction(out,
                 PrStage1SaveUi19148ActionKind::HelperGap,
                 psxFunction,
                 stateBefore,
                 stateAfter,
                 gapId,
                 arg1,
                 arg2,
                 arg3);
}

static PrStage1SaveUi19148TickResult MakeResult(bool running) {
    PrStage1SaveUi19148TickResult out{};
    out.active = running;
    out.psxState = s_saveUi19148.state;
    out.psxEventId = s_saveUi19148.eventId;
    out.ioResult = s_saveUi19148.gp724_ioResult;
    return out;
}

static PrStage1SaveUi19148TickResult Finish19148(
    int32_t saveResult,
    PrStage1SaveUi19148TickResult out = MakeResult(false)) {
    if (out.helperGap || out.actions.truncated) {
        out.active = true;
        out.done = false;
        out.psxState = s_saveUi19148.state;
        out.psxEventId = s_saveUi19148.eventId;
        return out;
    }
    out.active = false;
    out.done = true;
    out.saveResult = saveResult;
    out.saveSucceeded = (saveResult == 1);
    s_saveUi19148 = SaveUi19148Runtime{};
    return out;
}

static void CopySuffix(const char* src) {
    if (!src) {
        s_saveUi19148.saveSuffix[0] = '\0';
        if (!DirectWriteByte(kAddrSaveDataBuffer80092F10 + 1u, 0)) {
            s_saveUiMemory.payloadBank.savePayloadBankKnown = false;
        } else if (s_saveUiMemory.payloadBank.savePayloadBankKnown) {
            MarkSavePayloadWriter80092F10(kFn800185D0);
        }
        return;
    }
    std::strncpy(s_saveUi19148.saveSuffix,
                 src,
                 sizeof(s_saveUi19148.saveSuffix) - 1u);
    s_saveUi19148.saveSuffix[sizeof(s_saveUi19148.saveSuffix) - 1u] = '\0';
    const size_t copyBytes = std::strlen(s_saveUi19148.saveSuffix) + 1u;
    uint8_t* dst =
        DirectMemoryPtr(kAddrSaveDataBuffer80092F10 + 1u, copyBytes);
    if (!dst) {
        s_saveUiMemory.payloadBank.savePayloadBankKnown = false;
        return;
    }
    std::memcpy(dst, s_saveUi19148.saveSuffix, copyBytes);
    if (s_saveUiMemory.payloadBank.savePayloadBankKnown) {
        MarkSavePayloadWriter80092F10(kFn800185D0);
    }
}

static void CopySmallString(char* dst, uint32_t dstSize, const char* src) {
    if (!dst || dstSize == 0) {
        return;
    }
    if (!src) {
        dst[0] = '\0';
        return;
    }
    std::strncpy(dst, src, dstSize - 1u);
    dst[dstSize - 1u] = '\0';
}

static uint32_t Sub80017B08Direct() {
    return kAddrDirBank8007A318;
}

static bool Sub80025C44Direct(uint32_t dstAddress, int32_t count) {
    if (count < 0) {
        s_saveUiMemory.payloadBank.boundsFault = true;
        s_saveUiMemory.payloadBank.lastFaultAddress = dstAddress;
        return false;
    }
    uint8_t* dst = DirectMemoryPtr(dstAddress, static_cast<size_t>(count));
    if (!dst) {
        return false;
    }
    std::memset(dst, 0, static_cast<size_t>(count));
    return true;
}

static int32_t Sub800488E4Direct(uint32_t lhsAddress,
                                 uint32_t rhsAddress,
                                 int32_t count) {
    if (count <= 0) {
        return 0;
    }
    const uint8_t* lhs =
        DirectMemoryReadPtr(lhsAddress, static_cast<size_t>(count));
    const uint8_t* rhs =
        DirectMemoryReadPtr(rhsAddress, static_cast<size_t>(count));
    if (!lhs || !rhs) {
        return 0;
    }
    while (true) {
        const int32_t v4 = *lhs++;
        if (v4 != *rhs) {
            return v4 - *rhs;
        }
        --count;
        ++rhs;
        if (count <= 0) {
            return 0;
        }
    }
}

static bool Sub80025C64Direct(uint32_t srcAddress,
                              uint32_t dstAddress,
                              int32_t count) {
    if (count < 0) {
        s_saveUiMemory.payloadBank.boundsFault = true;
        s_saveUiMemory.payloadBank.lastFaultAddress = dstAddress;
        return false;
    }
    const uint8_t* src =
        DirectMemoryReadPtr(srcAddress, static_cast<size_t>(count));
    uint8_t* dst = DirectMemoryPtr(dstAddress, static_cast<size_t>(count));
    if (!src || !dst) {
        return false;
    }
    for (; count > 0; --count) {
        *dst++ = *src++;
    }
    return true;
}

static bool Sub8001615CDirect(int32_t a1,
                              int32_t& mappedIndex,
                              PrStage1SaveUi19148TickResult& out,
                              int32_t state) {
    const PrStagePayloadBankDirect::MapResult8001615C map =
        PrStagePayloadBankDirect::MapSaveStage8001615C(a1);
    if (!map.mapped) {
        AppendAction(out,
                     PrStage1SaveUi19148ActionKind::
                         Call8001615CMapSaveStage,
                     kFn8001615C,
                     state,
                     state,
                     a1,
                     static_cast<int32_t>(map.tableAddress),
                     -1);
        AppendHelperGap(out,
                        kFn8001615C,
                        state,
                        state,
                        a1,
                        static_cast<int32_t>(map.tableAddress),
                        -1);
        return false;
    }

    mappedIndex = map.mappedIndex;
    AppendAction(out,
                 PrStage1SaveUi19148ActionKind::Call8001615CMapSaveStage,
                 kFn8001615C,
                 state,
                 state,
                 a1,
                 static_cast<int32_t>(map.tableAddress),
                 mappedIndex);
    return true;
}

[[maybe_unused]] static bool Sub800161A8Direct(int32_t a1,
                                               int32_t& sceneId,
                                               PrStage1SaveUi19148TickResult& out,
                                               int32_t state) {
    const PrStagePayloadBankDirect::MapSceneResult800161A8 map =
        PrStagePayloadBankDirect::MapScene800161A8(a1);
    if (!map.mapped) {
        AppendAction(out,
                     PrStage1SaveUi19148ActionKind::Call800161A8MapScene,
                     kFn800161A8,
                     state,
                     state,
                     a1,
                     static_cast<int32_t>(map.tableAddress),
                     -1);
        AppendHelperGap(out,
                        kFn800161A8,
                        state,
                        state,
                        a1,
                        static_cast<int32_t>(map.tableAddress),
                        -1);
        return false;
    }

    sceneId = map.sceneId;
    AppendAction(out,
                 PrStage1SaveUi19148ActionKind::Call800161A8MapScene,
                 kFn800161A8,
                 state,
                 state,
                 a1,
                 static_cast<int32_t>(map.tableAddress),
                 sceneId);
    return true;
}

static void AppendStatusBankLiveGap(PrStage1SaveUi19148TickResult& out,
                                    uint32_t psxFunction,
                                    int32_t state,
                                    int32_t a1 = 0,
                                    int32_t a2 = 0) {
    AppendHelperGap(out,
                    psxFunction,
                    state,
                    state,
                    static_cast<int32_t>(kAddrSavePayloadProgressBase80092F1D),
                    a1,
                    a2);
}

static int32_t Sub8001635CDirect(int32_t a1,
                                 int32_t a2,
                                 int32_t a3,
                                 int32_t a4,
                                 PrStage1SaveUi19148TickResult& out,
                                 int32_t state) {
    AppendAction(out,
                 PrStage1SaveUi19148ActionKind::Call8001635CUpdateSavePayload,
                 kFn8001635C,
                 state,
                 state,
                 a1,
                 a2,
                 a3,
                 a4);

    int32_t mappedIndex = 0;
    Sub8001615CDirect(a1, mappedIndex, out, state);
    const PrStagePayloadBankDirect::UpdateSavePayloadResult8001635C direct =
        PrStagePayloadBankDirect::UpdateSavePayload8001635C(
            s_saveUiMemory.payloadBank,
            a1,
            a2,
            a3,
            a4,
            s_saveUi19148.dword800901BCKnown,
            s_saveUi19148.dword800901BC);

    if (direct.mapped && !direct.statusBankKnown) {
        AppendStatusBankLiveGap(out, kFn8001635C, state, a1, a2);
    }

    if (!direct.carrierSourceKnown) {
        AppendHelperGap(out,
                        kFn8001635C,
                        state,
                        state,
                        static_cast<int32_t>(
                            kAddrSavePayloadCarrierSource80092F48),
                        static_cast<int32_t>(0x800901BCu));
    }

    if (!direct.mirrorSourceKnown || !direct.mirrorCopied) {
        AppendHelperGap(out,
                        kFn80025C64,
                        state,
                        state,
                        static_cast<int32_t>(kAddrSavePayloadMirrorSrc8008EEF8),
                        static_cast<int32_t>(kAddrSavePayloadMirrorDst80092F5C),
                        kSavePayloadMirrorBytes8001635C,
                        -1);
    }

    if (!direct.allClearQueried) {
        AppendStatusBankLiveGap(out, kFn800161F4, state);
    }

    s_saveUi19148.savePayloadSourceAddress = kAddrSaveDataBuffer80092F10;
    s_saveUi19148.savePayloadSourceKnown = s_saveUiMemory.payloadBank.savePayloadBankKnown;
    return static_cast<int32_t>(direct.result);
}

static int32_t Sub80015CC4Direct(PrStage1SaveUi19148TickResult& out,
                                 int32_t state) {
    AppendAction(out,
                 PrStage1SaveUi19148ActionKind::Call80015CC4InitSavePayload,
                 kFn80015CC4,
                 state,
                 state,
                 static_cast<int32_t>(kAddrSaveDataBuffer80092F10),
                 kSaveUiSavePayloadBytes80019458);
    const PrStagePayloadBankDirect::InitSavePayloadResult80015CC4 direct =
        PrStagePayloadBankDirect::InitSavePayload80015CC4(
            s_saveUiMemory.payloadBank);
    const bool cleared = direct.ok;
    s_saveUi19148.savePayloadSourceAddress = kAddrSaveDataBuffer80092F10;
    s_saveUi19148.savePayloadSourceKnown = cleared;
    if (!cleared) {
        AppendHelperGap(out,
                        kFn80025C44,
                        state,
                        state,
                        static_cast<int32_t>(kAddrSaveDataBuffer80092F10),
                        kSaveUiSavePayloadBytes80019458);
        return 0;
    }
    const int32_t result = Sub8001635CDirect(1, 1, 1, 0, out, state);
    s_saveUi19148.savePayloadSourceKnown =
        s_saveUiMemory.payloadBank.savePayloadBankKnown;
    return result;
}

static bool Sub800164B4Direct(uint32_t srcAddress,
                              PrStage1SaveUi19148TickResult& out,
                              int32_t state) {
    AppendAction(out,
                 PrStage1SaveUi19148ActionKind::Call800164B4LoadSavePayload,
                 kFn800164B4,
                 state,
                 state,
                 static_cast<int32_t>(srcAddress),
                 static_cast<int32_t>(kAddrSaveDataBuffer80092F10),
                 kSaveUiSavePayloadBytes80019458);
    if (srcAddress == 0u) {
        AppendHelperGap(out,
                        kFn800164B4,
                        state,
                        state,
                        0,
                        static_cast<int32_t>(kAddrSaveDataBuffer80092F10),
                        kSaveUiSavePayloadBytes80019458);
        s_saveUiMemory.payloadBank.savePayloadBankKnown = false;
        s_saveUiMemory.payloadBank.statusBankKnown80092F1D = false;
        s_saveUi19148.savePayloadSourceKnown = false;
        return false;
    }
    const uint8_t* source =
        DirectMemoryReadPtr(srcAddress, kSaveUiSavePayloadBytes80019458);
    const PrStagePayloadBankDirect::LoadSavePayloadResult800164B4 direct =
        PrStagePayloadBankDirect::LoadSavePayload800164B4(
            s_saveUiMemory.payloadBank,
            srcAddress,
            source,
            source ? static_cast<size_t>(kSaveUiSavePayloadBytes80019458) : 0u,
            PrStagePayloadBankDirect::LoadSavePayloadAuthority800164B4{});
    const bool copied = direct.ok;
    s_saveUi19148.savePayloadSourceAddress = kAddrSaveDataBuffer80092F10;
    s_saveUi19148.savePayloadSourceKnown = copied;
    if (!copied) {
        AppendHelperGap(out,
                        kFn80025C64,
                        state,
                        state,
                        static_cast<int32_t>(srcAddress),
                        static_cast<int32_t>(kAddrSaveDataBuffer80092F10),
                        kSaveUiSavePayloadBytes80019458,
                        -1);
    }
    return copied;
}

static PrStage1SavePayloadProducerResult MakeProducerResult(
    const PrStage1SaveUi19148TickResult& out,
    int32_t result,
    bool ok) {
    PrStage1SavePayloadProducerResult producer{};
    producer.ok = ok;
    producer.payloadKnown = s_saveUiMemory.payloadBank.savePayloadBankKnown;
    producer.helperGap = s_saveUi19148.helperGap;
    producer.result = result;
    producer.lastFaultAddress = s_saveUiMemory.payloadBank.lastFaultAddress;
    producer.actions = out.actions;
    return producer;
}

static PrStage1SaveStatusBackupResult80015700 MakeBackupResult80015700(
    const PrStage1SaveUi19148TickResult& out,
    uint32_t psxFunction,
    uint32_t a1Address,
    int32_t result,
    bool ok,
    bool restoreKnown) {
    PrStage1SaveStatusBackupResult80015700 carrier{};
    carrier.ok = ok;
    carrier.backupKnown = s_saveUiMemory.payloadBank.saveStatusBackupKnown80079008;
    carrier.backupStatusBankKnown80092F1D =
        s_saveUiMemory.payloadBank.saveStatusBackupStatusBankKnown80092F1D;
    carrier.restoreKnown = restoreKnown;
    carrier.helperGap = s_saveUi19148.helperGap;
    carrier.result = result;
    carrier.psxFunction = psxFunction;
    carrier.a1Address = a1Address;
    carrier.lastFaultAddress = s_saveUiMemory.payloadBank.lastFaultAddress;
    carrier.actions = out.actions;
    return carrier;
}

static int32_t Sub80015700Direct(uint32_t a1Address,
                                 PrStage1SaveUi19148TickResult& out,
                                 int32_t state) {
    AppendAction(out,
                 PrStage1SaveUi19148ActionKind::
                     Call80015700BackupSaveStatusPrefix,
                 kFn80015700,
                 state,
                 state,
                 static_cast<int32_t>(a1Address),
                 static_cast<int32_t>(kAddrSaveStatusBackup80079008),
                 kSaveUiSavePayloadBytes80019458);

    if (a1Address != kAddrSaveDataBuffer80092F10) {
        s_saveUiMemory.payloadBank.lastFaultAddress = a1Address;
        AppendHelperGap(out,
                        kFn80015700,
                        state,
                        state,
                        static_cast<int32_t>(a1Address),
                        static_cast<int32_t>(kAddrSaveStatusBackup80079008),
                        kSaveUiSavePayloadBytes80019458);
        return 0;
    }

    if (!s_saveUiMemory.payloadBank.savePayloadBankKnown) {
        s_saveUiMemory.payloadBank.lastFaultAddress = kAddrSaveDataBuffer80092F10;
        AppendHelperGap(out,
                        kFn80015700,
                        state,
                        state,
                        static_cast<int32_t>(kAddrSaveDataBuffer80092F10),
                        static_cast<int32_t>(kAddrSaveStatusBackup80079008),
                        kSaveUiSavePayloadBytes80019458);
        return 0;
    }

    const bool copied = Sub80025C64Direct(a1Address,
                                          kAddrSaveStatusBackup80079008,
                                          kSaveUiSavePayloadBytes80019458);
    s_saveUiMemory.payloadBank.saveStatusBackupKnown80079008 = copied;
    s_saveUiMemory.payloadBank.saveStatusBackupStatusBankKnown80092F1D =
        copied && s_saveUiMemory.payloadBank.statusBankKnown80092F1D;
    if (!copied) {
        AppendHelperGap(out,
                        kFn80025C64,
                        state,
                        state,
                        static_cast<int32_t>(a1Address),
                        static_cast<int32_t>(kAddrSaveStatusBackup80079008),
                        kSaveUiSavePayloadBytes80019458);
        return 0;
    }
    return 1;
}

static int32_t Sub80015744Direct(uint32_t a1Address,
                                 PrStage1SaveUi19148TickResult& out,
                                 int32_t state) {
    AppendAction(out,
                 PrStage1SaveUi19148ActionKind::
                     Call80015744RestoreSaveStatusPrefix,
                 kFn80015744,
                 state,
                 state,
                 static_cast<int32_t>(kAddrSaveStatusBackup80079008),
                 static_cast<int32_t>(a1Address),
                 kSaveUiSavePayloadBytes80019458);

    if (a1Address != kAddrSaveDataBuffer80092F10) {
        s_saveUiMemory.payloadBank.lastFaultAddress = a1Address;
        AppendHelperGap(out,
                        kFn80015744,
                        state,
                        state,
                        static_cast<int32_t>(kAddrSaveStatusBackup80079008),
                        static_cast<int32_t>(a1Address),
                        kSaveUiSavePayloadBytes80019458);
        return 0;
    }

    if (!s_saveUiMemory.payloadBank.saveStatusBackupKnown80079008) {
        s_saveUiMemory.payloadBank.lastFaultAddress = kAddrSaveStatusBackup80079008;
        AppendHelperGap(out,
                        kFn80015744,
                        state,
                        state,
                        static_cast<int32_t>(kAddrSaveStatusBackup80079008),
                        static_cast<int32_t>(a1Address),
                        kSaveUiSavePayloadBytes80019458);
        return 0;
    }
    if (!s_saveUiMemory.payloadBank.saveStatusBackupStatusBankKnown80092F1D) {
        s_saveUiMemory.payloadBank.lastFaultAddress = kAddrSaveStatusBackup80079008;
        AppendHelperGap(out,
                        kFn80015744,
                        state,
                        state,
                        static_cast<int32_t>(kAddrSaveStatusBackup80079008),
                        static_cast<int32_t>(a1Address),
                        kSaveUiSavePayloadBytes80019458);
        return 0;
    }

    const bool copied = Sub80025C64Direct(kAddrSaveStatusBackup80079008,
                                          a1Address,
                                          kSaveUiSavePayloadBytes80019458);
    if (!copied) {
        s_saveUiMemory.payloadBank.savePayloadBankKnown = false;
        s_saveUiMemory.payloadBank.statusBankKnown80092F1D = false;
        s_saveUi19148.savePayloadSourceKnown = false;
        AppendHelperGap(out,
                        kFn80025C64,
                        state,
                        state,
                        static_cast<int32_t>(kAddrSaveStatusBackup80079008),
                        static_cast<int32_t>(a1Address),
                        kSaveUiSavePayloadBytes80019458);
        return 0;
    }

    s_saveUiMemory.payloadBank.savePayloadBankKnown = true;
    s_saveUiMemory.payloadBank.statusBankKnown80092F1D =
        s_saveUiMemory.payloadBank.saveStatusBackupStatusBankKnown80092F1D;
    MarkSavePayloadWriter80092F10(kFn80015744);
    s_saveUi19148.savePayloadSourceAddress = kAddrSaveDataBuffer80092F10;
    s_saveUi19148.savePayloadSourceKnown = true;
    return 1;
}

static int32_t AppendHostCStringToDirect(uint32_t dstAddress,
                                         int32_t dstOffset,
                                         const char* src) {
    if (!src) {
        src = "";
    }
    while (*src) {
        if (!DirectWriteByte(dstAddress + static_cast<uint32_t>(dstOffset),
                             static_cast<uint8_t>(*src++))) {
            return -1;
        }
        ++dstOffset;
    }
    return dstOffset;
}

static int32_t AppendDirectCStringToDirect(uint32_t dstAddress,
                                           int32_t dstOffset,
                                           uint32_t srcAddress) {
    uint32_t srcOffset = 0;
    while (true) {
        uint8_t value = 0;
        if (!DirectReadByte(srcAddress + srcOffset, value)) {
            return -1;
        }
        if (value == 0) {
            return dstOffset;
        }
        if (!DirectWriteByte(dstAddress + static_cast<uint32_t>(dstOffset),
                             value)) {
            return -1;
        }
        ++srcOffset;
        ++dstOffset;
    }
}

static int32_t AppendDecimalToDirect(uint32_t dstAddress,
                                     int32_t dstOffset,
                                     int32_t value) {
    char digits[12]{};
    int32_t count = 0;
    bool negative = value < 0;
    uint32_t remaining =
        negative ? static_cast<uint32_t>(-value) : static_cast<uint32_t>(value);
    do {
        digits[count++] = static_cast<char>('0' + (remaining % 10u));
        remaining /= 10u;
    } while (remaining != 0 && count < static_cast<int32_t>(sizeof(digits)));
    if (negative) {
        digits[count++] = '-';
    }
    while (count > 0) {
        if (!DirectWriteByte(dstAddress + static_cast<uint32_t>(dstOffset),
                             static_cast<uint8_t>(digits[--count]))) {
            return -1;
        }
        ++dstOffset;
    }
    return dstOffset;
}

static int32_t AppendByteToLocal(char* dst,
                                 uint32_t dstSize,
                                 int32_t dstOffset,
                                 uint8_t value) {
    if (!dst || dstSize == 0 || dstOffset < 0 ||
        static_cast<uint32_t>(dstOffset) >= dstSize) {
        return -1;
    }
    dst[dstOffset] = static_cast<char>(value);
    return dstOffset + 1;
}

static int32_t AppendHostCStringToLocal(char* dst,
                                        uint32_t dstSize,
                                        int32_t dstOffset,
                                        const char* src) {
    if (!src) {
        src = "";
    }
    while (*src) {
        dstOffset = AppendByteToLocal(
            dst, dstSize, dstOffset, static_cast<uint8_t>(*src++));
        if (dstOffset < 0) {
            return -1;
        }
    }
    return dstOffset;
}

static int32_t AppendDirectCStringToLocal(char* dst,
                                          uint32_t dstSize,
                                          int32_t dstOffset,
                                          uint32_t srcAddress) {
    uint32_t srcOffset = 0;
    while (true) {
        uint8_t value = 0;
        if (!DirectReadByte(srcAddress + srcOffset, value)) {
            return -1;
        }
        if (value == 0) {
            return dstOffset;
        }
        dstOffset = AppendByteToLocal(dst, dstSize, dstOffset, value);
        if (dstOffset < 0) {
            return -1;
        }
        ++srcOffset;
    }
}

static int32_t AppendDecimalToLocal(char* dst,
                                    uint32_t dstSize,
                                    int32_t dstOffset,
                                    int32_t value) {
    char digits[12]{};
    int32_t count = 0;
    bool negative = value < 0;
    uint32_t remaining =
        negative ? static_cast<uint32_t>(-value) : static_cast<uint32_t>(value);
    do {
        digits[count++] = static_cast<char>('0' + (remaining % 10u));
        remaining /= 10u;
    } while (remaining != 0 && count < static_cast<int32_t>(sizeof(digits)));
    if (negative) {
        digits[count++] = '-';
    }
    while (count > 0) {
        dstOffset = AppendByteToLocal(
            dst, dstSize, dstOffset, static_cast<uint8_t>(digits[--count]));
        if (dstOffset < 0) {
            return -1;
        }
    }
    return dstOffset;
}

static bool TerminateLocalCString(char* dst, uint32_t dstSize, int32_t offset) {
    return AppendByteToLocal(dst, dstSize, offset, 0) >= 0;
}

static int32_t Sub8004800CSaveUiSubset_s_s(
    uint32_t dstAddress,
    uint32_t firstAddress,
    const char* second) {
    int32_t offset = AppendDirectCStringToDirect(dstAddress, 0, firstAddress);
    if (offset < 0) {
        return -1;
    }
    offset = AppendHostCStringToDirect(dstAddress, offset, second);
    if (offset < 0 || !DirectWriteByte(dstAddress + static_cast<uint32_t>(offset),
                                       0)) {
        return -1;
    }
    return offset;
}

[[maybe_unused]] static int32_t Sub8004800CSaveUiSubset_s(
    uint32_t dstAddress,
    uint32_t firstAddress) {
    int32_t offset = AppendDirectCStringToDirect(dstAddress, 0, firstAddress);
    if (offset < 0 || !DirectWriteByte(dstAddress + static_cast<uint32_t>(offset),
                                       0)) {
        return -1;
    }
    return offset;
}

static int32_t Sub8004800CSaveUiSubset_s_Local(char* dst,
                                               uint32_t dstSize,
                                               uint32_t firstAddress) {
    int32_t offset = AppendDirectCStringToLocal(dst, dstSize, 0, firstAddress);
    if (offset < 0 || !TerminateLocalCString(dst, dstSize, offset)) {
        return -1;
    }
    return offset;
}

static int32_t Sub8004800CSaveUiSubset_s_quoted_s_STd_Local(
    char* dst,
    uint32_t dstSize,
    uint32_t firstAddress,
    const char* second,
    int32_t stage) {
    int32_t offset = AppendDirectCStringToLocal(dst, dstSize, 0, firstAddress);
    if (offset < 0) {
        return -1;
    }
    offset = AppendHostCStringToLocal(dst, dstSize, offset, " \"");
    if (offset < 0) {
        return -1;
    }
    offset = AppendHostCStringToLocal(dst, dstSize, offset, second);
    if (offset < 0) {
        return -1;
    }
    offset = AppendHostCStringToLocal(dst, dstSize, offset, "\" ST");
    if (offset < 0) {
        return -1;
    }
    offset = AppendDecimalToLocal(dst, dstSize, offset, stage);
    if (offset < 0 || !TerminateLocalCString(dst, dstSize, offset)) {
        return -1;
    }
    return offset;
}

[[maybe_unused]] static int32_t Sub8004800CSaveUiSubset_s_quoted_s_STd(
    uint32_t dstAddress,
    uint32_t firstAddress,
    const char* second,
    int32_t stage) {
    int32_t offset = AppendDirectCStringToDirect(dstAddress, 0, firstAddress);
    if (offset < 0) {
        return -1;
    }
    offset = AppendHostCStringToDirect(dstAddress, offset, " \"");
    if (offset < 0) {
        return -1;
    }
    offset = AppendHostCStringToDirect(dstAddress, offset, second);
    if (offset < 0) {
        return -1;
    }
    offset = AppendHostCStringToDirect(dstAddress, offset, "\" ST");
    if (offset < 0) {
        return -1;
    }
    offset = AppendDecimalToDirect(dstAddress, offset, stage);
    if (offset < 0 || !DirectWriteByte(dstAddress + static_cast<uint32_t>(offset),
                                       0)) {
        return -1;
    }
    return offset;
}

[[maybe_unused]] static int32_t Sub8004800CSaveUiSubset_bu_1d_1d(
    uint32_t dstAddress,
    int32_t first,
    int32_t second) {
    int32_t offset = AppendHostCStringToDirect(dstAddress, 0, "bu");
    if (offset < 0) {
        return -1;
    }
    offset = AppendDecimalToDirect(dstAddress, offset, first);
    if (offset < 0) {
        return -1;
    }
    offset = AppendDecimalToDirect(dstAddress, offset, second);
    if (offset < 0) {
        return -1;
    }
    offset = AppendHostCStringToDirect(dstAddress, offset, ":");
    if (offset < 0 || !DirectWriteByte(dstAddress + static_cast<uint32_t>(offset),
                                       0)) {
        return -1;
    }
    return offset;
}

static bool ResolveSaveUiGpPointer80019458(uint32_t pointerAddress,
                                           uint32_t& sourceAddress) {
    if (pointerAddress != kAddrSaveFilenamePrefixPtr80019458 &&
        pointerAddress != kAddrSaveTitlePrefixPtr80019458) {
        return false;
    }
    return DirectReadLe32(pointerAddress, sourceAddress);
}

static bool CopyDirectCStringToSmallString(char* dst,
                                           uint32_t dstSize,
                                           uint32_t srcAddress) {
    if (!dst || dstSize == 0) {
        return false;
    }
    uint32_t srcOffset = 0;
    while (srcOffset + 1u < dstSize) {
        uint8_t value = 0;
        if (!DirectReadByte(srcAddress + srcOffset, value)) {
            dst[0] = '\0';
            return false;
        }
        dst[srcOffset] = static_cast<char>(value);
        if (value == 0) {
            return true;
        }
        ++srcOffset;
    }
    dst[dstSize - 1u] = '\0';
    return true;
}

static bool FilterSaveUiTitleSuffix80019458(const char* src,
                                            char* dst,
                                            uint32_t dstSize) {
    if (!dst || dstSize <= 1u) {
        return false;
    }
    if (!src) {
        src = "";
    }

    int32_t count = 0;
    if (*src) {
        do {
            if (count >= kSaveUiTitleSuffixMax80019458 ||
                count + 1 >= static_cast<int32_t>(dstSize)) {
                break;
            }
            uint8_t charClass = 0;
            const uint8_t ch = static_cast<uint8_t>(*src);
            if (!DirectReadByte(kAddrTitleCharClass8006E999 + ch, charClass)) {
                return false;
            }
            dst[count++] = ((charClass & 7u) != 0u) ? static_cast<char>(ch) : '?';
            ++src;
        } while (*src);
    }
    dst[count] = '\0';
    return true;
}

static bool EncodeAsciiTitleToDirect(uint32_t dstAddress,
                                     const char* text,
                                     int32_t sourceBytes) {
    if (!text || sourceBytes < 0) {
        return false;
    }
    bool pastTerminator = false;
    for (int32_t i = 0; i < sourceBytes; ++i) {
        uint8_t ch = 0;
        if (!pastTerminator) {
            ch = static_cast<uint8_t>(text[i]);
            if (ch == 0) {
                pastTerminator = true;
            }
        }
        uint16_t glyph = 0;
        if (ch != 0 &&
            !DirectReadLe16(kGlyphEncodeTableBase80019458 +
                                static_cast<uint32_t>(ch) * 2u,
                            glyph)) {
            return false;
        }
        if (!DirectWriteByte(dstAddress + static_cast<uint32_t>(i * 2),
                             static_cast<uint8_t>(glyph >> 8)) ||
            !DirectWriteByte(dstAddress + static_cast<uint32_t>(i * 2 + 1),
                             static_cast<uint8_t>(glyph & 0xFFu))) {
            return false;
        }
    }
    return true;
}

static bool ProduceState15FilenameAndTitle80019458(
    PrStage1SaveUi19148TickResult& out,
    int32_t state) {
    uint32_t filenamePrefixAddress = 0;
    uint32_t titlePrefixAddress = 0;
    const bool sourcesResolved =
        ResolveSaveUiGpPointer80019458(kAddrSaveFilenamePrefixPtr80019458,
                                       filenamePrefixAddress) &&
        ResolveSaveUiGpPointer80019458(kAddrSaveTitlePrefixPtr80019458,
                                       titlePrefixAddress) &&
        filenamePrefixAddress == kAddrSaveFilenamePrefix800101E0 &&
        titlePrefixAddress == kAddrSaveTitlePrefix8006EAD8;
    if (!sourcesResolved ||
        Sub8004800CSaveUiSubset_s_s(kSaveUiFilenameBuffer80019458,
                                    filenamePrefixAddress,
                                    s_saveUi19148.saveSuffix) < 0) {
        s_saveUi19148.byte8007CBE8[0] = '\0';
        s_saveUi19148.byte8007CBE8Present = false;
        AppendHelperGap(
            out, kFn8004800C, state, state, kSaveUiFilenameBuffer80019458);
    } else {
        s_saveUi19148.byte8007CBE8Present = CopyDirectCStringToSmallString(
            s_saveUi19148.byte8007CBE8,
            sizeof(s_saveUi19148.byte8007CBE8),
            kSaveUiFilenameBuffer80019458);
    }

    char filteredSuffix[kSaveUiTitleSuffixMax80019458 + 1]{};
    char titleLocal[kSaveUiTitleLocalBytes80019458]{};
    bool titleBuilt = sourcesResolved &&
                      FilterSaveUiTitleSuffix80019458(s_saveUi19148.saveSuffix,
                                                      filteredSuffix,
                                                      sizeof(filteredSuffix));
    if (titleBuilt) {
        const int32_t titleLength =
            filteredSuffix[0] != '\0'
                ? Sub8004800CSaveUiSubset_s_quoted_s_STd_Local(
                      titleLocal,
                      sizeof(titleLocal),
                      titlePrefixAddress,
                      filteredSuffix,
                      kStage1SaveUiStageNumber80016314)
                : Sub8004800CSaveUiSubset_s_Local(
                      titleLocal, sizeof(titleLocal), titlePrefixAddress);
        titleBuilt = titleLength >= 0 &&
                     EncodeAsciiTitleToDirect(
                         kSaveUiEncodedTitleBuffer80019458,
                         titleLocal,
                         kSaveUiTitleLocalBytes80019458);
    }

    s_saveUi19148.encodedTitleBufferPresent = titleBuilt;
    if (!titleBuilt) {
        AppendHelperGap(out,
                        kFn8004800C,
                        state,
                        state,
                        kSaveUiEncodedTitleBuffer80019458);
    }
    return s_saveUi19148.byte8007CBE8Present && titleBuilt;
}

static bool CopyState15SavePayload80019458(
    PrStage1SaveUi19148TickResult& out,
    int32_t state) {
    const uint32_t srcAddress = s_saveUi19148.savePayloadSourceAddress;
    const bool sourceInPayloadBank =
        SpanContains(kAddrSaveDataBuffer80092F10,
                     s_saveUiMemory.payloadBank.savePayloadBank.size(),
                     srcAddress,
                     kSaveUiSavePayloadBytes80019458);
    s_saveUi19148.savePayloadSourceKnown =
        sourceInPayloadBank && s_saveUiMemory.payloadBank.savePayloadBankKnown;
    out.consumedBy80019458State15Known = true;
    out.consumedBy80019458State15 = s_saveUi19148.savePayloadSourceKnown;
    out.state15PrefixAddress = srcAddress;
    AppendAction(out,
                 PrStage1SaveUi19148ActionKind::
                     Call80019458ConsumePrefixState15,
                 kFn80019458,
                 state,
                 state,
                 static_cast<int32_t>(srcAddress),
                 static_cast<int32_t>(kAddrSavePayloadDst8007ADE8),
                 kSaveUiSavePayloadBytes80019458,
                 s_saveUi19148.savePayloadSourceKnown ? 1 : 0);
    AppendAction(out,
                 PrStage1SaveUi19148ActionKind::Call80025C64CopySavePayload,
                 kFn80025C64,
                 state,
                 state,
                 static_cast<int32_t>(srcAddress),
                 static_cast<int32_t>(kAddrSavePayloadDst8007ADE8),
                 kSaveUiSavePayloadBytes80019458,
                 s_saveUi19148.savePayloadSourceKnown ? 1 : 0);
    s_saveUi19148.savePayloadCopied =
        Sub80025C64Direct(srcAddress,
                          kAddrSavePayloadDst8007ADE8,
                          kSaveUiSavePayloadBytes80019458);
    out.state15CopyTo8007ADE8Known = true;
    out.state15CopyTo8007ADE8 = s_saveUi19148.savePayloadCopied;
    if (!s_saveUi19148.savePayloadCopied ||
        !s_saveUi19148.savePayloadSourceKnown) {
        AppendHelperGap(out,
                        kFn80025C64,
                        state,
                        state,
                        static_cast<int32_t>(srcAddress),
                        static_cast<int32_t>(kAddrSavePayloadDst8007ADE8),
                        kSaveUiSavePayloadBytes80019458,
                        s_saveUi19148.savePayloadCopied ? 0 : -1);
    }
    return s_saveUi19148.savePayloadCopied &&
           s_saveUi19148.savePayloadSourceKnown;
}

static void AppendSub80016FC0Action(PrStage1SaveUi19148TickResult& out,
                                    int32_t state,
                                    bool feedbackKnown) {
    AppendAction(out,
                 PrStage1SaveUi19148ActionKind::Call80016FC0ClearSwCardEvents,
                 kFn80016FC0,
                 state,
                 state,
                 static_cast<int32_t>(0x8006ECD8u),
                 4);
    if (!feedbackKnown) {
        AppendHelperGap(out, kFn80016FC0, state, state, 4);
    }
}

static void AppendSub80016EB8Action(PrStage1SaveUi19148TickResult& out,
                                    int32_t state,
                                    int32_t attempt,
                                    bool feedbackKnown) {
    AppendAction(out,
                 PrStage1SaveUi19148ActionKind::Call80016EB8PollSwCardEvents,
                 kFn80016EB8,
                 state,
                 state,
                 attempt,
                 kSaveUiWriteEventPollCount80016EB8,
                 static_cast<int32_t>(0x8006ECD8u),
                 4);
    if (!feedbackKnown) {
        AppendHelperGap(out,
                        kFn80016EB8,
                        state,
                        state,
                        kSaveUiWriteEventPollCount80016EB8,
                        attempt,
                        static_cast<int32_t>(0x8006ECD8u),
                        4);
    }
}

static PrStage1SaveUiCardIoCarrier80017594 AppendSub80017594FeedbackCarrier(
    PrStage1SaveUi19148TickResult& out,
    int32_t saveUiState,
    const PrStage1SaveUiCardIoState80017594& inputState,
    const PrStage1SaveUiCardIoFeedback80017594* feedback) {
    PrStage1SaveUiCardIoCarrier80017594 carrier{};
    carrier.stateBefore = inputState;
    carrier.stateAfter = inputState;
    carrier.result = inputState.dword800917EC;

    auto finishKnownReturn = [&]() {
        carrier.resultKnown = true;
        carrier.result = carrier.stateAfter.dword800917EC;
        carrier.actions = out.actions;
        return carrier;
    };
    auto finishUnknownState = [&]() {
        carrier.resultKnown = true;
        carrier.result = inputState.dword800917EC;
        carrier.actions = out.actions;
        return carrier;
    };
    auto applyPollSwGp700Facts = [&]() {
        if (!feedback ||
            !feedback->pollSwGp700BeforeKnown80016E18 ||
            !feedback->pollSwGp700AfterKnown80016E18 ||
            !feedback->pollSwTimedOutKnown80016E18 ||
            feedback->pollSwGp700Before80016E18 != inputState.gp700) {
            AppendHelperGap(out,
                            kFn80016E18,
                            saveUiState,
                            saveUiState,
                            inputState.dword800917E8,
                            inputState.gp700);
            carrier.helperGap = true;
            return false;
        }
        if (feedback->pollSwGp700After80016E18 != inputState.gp700 - 1) {
            AppendHelperGap(out,
                            kFn80016E18,
                            saveUiState,
                            saveUiState,
                            inputState.dword800917E8,
                            inputState.gp700);
            carrier.helperGap = true;
            return false;
        }
        const bool timedOut =
            feedback->pollSwGp700After80016E18 < 0;
        if (feedback->pollSwTimedOut80016E18 != timedOut ||
            (timedOut && feedback->pollSwResult80016E18 != 2)) {
            AppendHelperGap(out,
                            kFn80016E18,
                            saveUiState,
                            saveUiState,
                            inputState.dword800917E8,
                            inputState.gp700);
            carrier.helperGap = true;
            return false;
        }
        carrier.stateAfter.gp700 = feedback->pollSwGp700After80016E18;
        return true;
    };

    AppendAction(out,
                 PrStage1SaveUi19148ActionKind::Call80017594PollCardIo,
                 kFn80017594,
                 saveUiState,
                 saveUiState,
                 inputState.dword800917E8,
                 inputState.dword800917EC,
                 inputState.dword800917F0,
                 inputState.dword800917F4,
                 inputState.gp700);

    switch (inputState.dword800917E8) {
    case 0:
        AppendAction(out,
                     PrStage1SaveUi19148ActionKind::CardHalInfo,
                     kFn80017594,
                     saveUiState,
                     saveUiState,
                     0);
        if (!feedback || !feedback->cardInfoKnown) {
            AppendHelperGap(out, kFn80017594, saveUiState, saveUiState, 0);
            carrier.helperGap = true;
        }
        carrier.stateAfter.dword800917E8 = 1;
        carrier.stateAfter.dword800917EC = 0;
        carrier.stateAfter.gp700 = 300;
        carrier.stateAfterKnown = true;
        return finishKnownReturn();

    case 1: {
        AppendAction(out,
                     PrStage1SaveUi19148ActionKind::Call80016E18PollCardInfoLoad,
                     kFn80016E18,
                     saveUiState,
                     saveUiState,
                     inputState.dword800917E8);
        if (!feedback || !feedback->pollSwKnown80016E18) {
            AppendHelperGap(out, kFn80016E18, saveUiState, saveUiState, 1);
            carrier.helperGap = true;
            return finishUnknownState();
        }
        const int32_t poll = feedback->pollSwResult80016E18;
        if (!applyPollSwGp700Facts()) {
            return finishUnknownState();
        }
        if (poll == 0) {
            carrier.stateAfterKnown = true;
            return finishKnownReturn();
        }
        if (poll == 1) {
            carrier.stateAfter.dword800917F0 = 1;
            carrier.stateAfter.dword800917E8 =
                inputState.dword800917F4 == 1 ? 4 : 2;
            carrier.stateAfterKnown = true;
            return finishKnownReturn();
        }
        if (poll == 3) {
            carrier.stateAfter.dword800917F0 = 3;
        } else if (poll == 4) {
            carrier.stateAfter.dword800917F0 = 4;
            AppendAction(out,
                         PrStage1SaveUi19148ActionKind::Call8001707CDrainHwCardEvents,
                         kFn8001707C,
                         saveUiState,
                         saveUiState,
                         inputState.dword800917E8);
            if (!feedback->drainHwEventsKnown8001707C) {
                AppendHelperGap(out, kFn8001707C, saveUiState, saveUiState, 1);
                carrier.helperGap = true;
            }
            AppendAction(out,
                         PrStage1SaveUi19148ActionKind::Call80047EE4ResetHwCardEvents,
                         kFn80047EE4,
                         saveUiState,
                         saveUiState,
                         0);
            if (!feedback->resetHwEventsKnown80047EE4 ||
                !feedback->resetHwNewCardKnown80047EE4 ||
                !feedback->resetHwCardWriteArgsKnown80047EE4 ||
                feedback->resetHwCardWriteArg0_80047EE4 != 0 ||
                feedback->resetHwCardWriteArg1_80047EE4 != 63 ||
                feedback->resetHwCardWriteArg2_80047EE4 != 0 ||
                !feedback->resetHwCardWriteResultKnown80047EE4) {
                AppendHelperGap(out, kFn80047EE4, saveUiState, saveUiState, 4);
                carrier.helperGap = true;
            }
            AppendAction(out,
                         PrStage1SaveUi19148ActionKind::Call80017008PollHwCardEvents,
                         kFn80017008,
                         saveUiState,
                         saveUiState,
                         inputState.dword800917E8);
            if (!feedback->pollHwKnown80017008) {
                AppendHelperGap(out, kFn80017008, saveUiState, saveUiState, 1);
                carrier.helperGap = true;
            }
            carrier.stateAfter.dword800917E8 = 2;
            carrier.stateAfter.dword800917F4 = 0;
            carrier.stateAfterKnown = true;
            return finishKnownReturn();
        } else {
            carrier.stateAfter.dword800917F0 = -3;
        }
        carrier.stateAfter.dword800917E8 = 4;
        carrier.stateAfter.dword800917F4 = 0;
        carrier.stateAfterKnown = true;
        return finishKnownReturn();
    }

    case 2:
        AppendSub80016FC0Action(
            out,
            saveUiState,
            feedback && feedback->clearSwEventsKnown80016FC0);
        if (!feedback || !feedback->clearSwEventsKnown80016FC0) {
            carrier.helperGap = true;
        }
        AppendAction(out,
                     PrStage1SaveUi19148ActionKind::CardHalLoad,
                     kFn80017594,
                     saveUiState,
                     saveUiState,
                     0);
        if (!feedback || !feedback->cardLoadKnown) {
            AppendHelperGap(out, kFn80017594, saveUiState, saveUiState, 2);
            carrier.helperGap = true;
        }
        carrier.stateAfter.dword800917E8 = 3;
        carrier.stateAfter.gp700 = 300;
        carrier.stateAfterKnown = true;
        return finishKnownReturn();

    case 3: {
        AppendAction(out,
                     PrStage1SaveUi19148ActionKind::Call80016E18PollCardInfoLoad,
                     kFn80016E18,
                     saveUiState,
                     saveUiState,
                     inputState.dword800917E8);
        if (!feedback || !feedback->pollSwKnown80016E18) {
            AppendHelperGap(out, kFn80016E18, saveUiState, saveUiState, 3);
            carrier.helperGap = true;
            return finishUnknownState();
        }
        const int32_t poll = feedback->pollSwResult80016E18;
        if (!applyPollSwGp700Facts()) {
            return finishUnknownState();
        }
        if (poll == 0) {
            carrier.stateAfterKnown = true;
            return finishKnownReturn();
        }
        carrier.stateAfter.dword800917E8 = 4;
        carrier.stateAfter.dword800917F4 = 0;
        if (poll == 1) {
            carrier.stateAfter.dword800917F4 = 1;
        } else if (poll == 3) {
            carrier.stateAfter.dword800917F0 = 3;
        } else if (poll == 4) {
            carrier.stateAfter.dword800917F0 = 5;
        } else {
            carrier.stateAfter.dword800917F0 = 2;
        }
        carrier.stateAfterKnown = true;
        return finishKnownReturn();
    }

    case 4:
        carrier.stateAfter.dword800917E8 = 0;
        carrier.stateAfter.dword800917EC = inputState.dword800917F0;
        carrier.stateAfterKnown = true;
        return finishKnownReturn();

    default:
        carrier.stateAfterKnown = true;
        return finishKnownReturn();
    }
}

static bool CardIoStateEquals80017594(
    const PrStage1SaveUiCardIoState80017594& lhs,
    const PrStage1SaveUiCardIoState80017594& rhs) {
    return lhs.dword800917E8 == rhs.dword800917E8 &&
           lhs.dword800917EC == rhs.dword800917EC &&
           lhs.dword800917F0 == rhs.dword800917F0 &&
           lhs.dword800917F4 == rhs.dword800917F4 &&
           lhs.gp700 == rhs.gp700;
}

static bool TryConsumeTickCardIoFeedback80017594(
    PrStage1SaveUi19148TickResult& out,
    const PrStage1SaveUi19148LowerFeedback* lowerFeedback,
    int32_t& ioResult) {
    if (!lowerFeedback || !lowerFeedback->cardIoFeedbackKnown80017594) {
        return false;
    }
    const PrStage1SaveUiCardIoFeedback80017594& feedback =
        lowerFeedback->cardIoFeedback80017594;
    if (!feedback.stateBeforeKnown ||
        !CardIoStateEquals80017594(
            feedback.stateBefore,
            s_saveUi19148.cardIoState80017594)) {
        AppendHelperGap(out,
                        kFn80017594,
                        s_saveUi19148.state,
                        s_saveUi19148.state,
                        s_saveUi19148.cardIoState80017594.dword800917E8,
                        s_saveUi19148.cardIoState80017594.gp700);
        return true;
    }

    const PrStage1SaveUiCardIoCarrier80017594 carrier =
        AppendSub80017594FeedbackCarrier(
            out,
            s_saveUi19148.state,
            s_saveUi19148.cardIoState80017594,
            &feedback);

    out.cardIoStateBeforeKnown80017594 = true;
    out.cardIoStateBefore80017594 = carrier.stateBefore;
    if (carrier.stateAfterKnown) {
        s_saveUi19148.cardIoState80017594 = carrier.stateAfter;
        out.cardIoStateAfterKnown80017594 = true;
        out.cardIoStateAfter80017594 = carrier.stateAfter;
        RewriteLastCardIoRequestState80017594(out, carrier.stateAfter);
    }
    if (carrier.resultKnown) {
        ioResult = carrier.result;
        out.ioResultKnown = true;
    }
    return true;
}

static PrStage1SaveUiWriteFeedbackCarrier80017A10
AppendSub80017A10FeedbackCarrier(
    PrStage1SaveUi19148TickResult& out,
    int32_t state,
    uint32_t nameAddress,
    uint32_t dataAddress,
    int32_t blocks,
    const PrStage1SaveUiWriteFeedbackInput80017A10* feedback) {
    PrStage1SaveUiWriteFeedbackCarrier80017A10 carrier{};
    AppendAction(out,
                 PrStage1SaveUi19148ActionKind::Call80017A10WriteSaveBlock,
                 kFn80017A10,
                 state,
                 kStateExit80018FB0,
                 static_cast<int32_t>(nameAddress),
                 static_cast<int32_t>(dataAddress),
                 blocks,
                 kSaveUiWriteRetryCount80017A10);
    for (int32_t attempt = 0; attempt < kSaveUiWriteRetryCount80017A10;
         ++attempt) {
        carrier.attemptsUsed = attempt + 1;
        const PrStage1SaveUiWriteAttemptFeedback80017A10* attemptFeedback =
            feedback ? &feedback->attempts[attempt] : nullptr;
        const bool scanKnown =
            attemptFeedback && attemptFeedback->scanResultKnown80017900;
        const int32_t scanResult =
            scanKnown ? attemptFeedback->scanResult80017900 : 0;
        const int32_t checkExists = scanKnown && scanResult == 1 ? 0 : 1;
        AppendAction(out,
                     PrStage1SaveUi19148ActionKind::Call80017900ScanCardDirectory,
                     kFn80017900,
                     state,
                     state,
                     static_cast<int32_t>(nameAddress),
                     static_cast<int32_t>(kAddrDirBank8007A318),
                     kSaveUiDirEntryCount80019458,
                     attempt);
        if (!scanKnown) {
            AppendHelperGap(out,
                            kFn80017900,
                            state,
                            state,
                            attempt,
                            static_cast<int32_t>(nameAddress));
            carrier.helperGap = true;
        }
        AppendAction(out,
                     PrStage1SaveUi19148ActionKind::Call80017454SubmitWrite,
                     kFn80017454,
                     state,
                     state,
                     static_cast<int32_t>(nameAddress),
                     static_cast<int32_t>(dataAddress),
                     blocks,
                     (attempt << 16) | (checkExists & 0xFFFF));
        if (checkExists != 0) {
            AppendAction(out,
                         PrStage1SaveUi19148ActionKind::CardHalOpenCheck,
                         kFn80017454,
                         state,
                         state,
                         (blocks << 16) | 0x200,
                         attempt,
                         static_cast<int32_t>(nameAddress),
                         checkExists);
            if (!attemptFeedback ||
                !attemptFeedback->openCheckKnown80017454) {
                AppendHelperGap(out, kFn80017454, state, state, 0x200, attempt);
                carrier.helperGap = true;
            }
        }
        const bool openCheckComplete =
            checkExists == 0 ||
            (attemptFeedback &&
             attemptFeedback->openCheckKnown80017454 &&
             attemptFeedback->openCheckReturnKnown80017454 &&
             attemptFeedback->openCheckFdKnown80017454 &&
             attemptFeedback->openCheckReturn80017454 ==
                 attemptFeedback->openCheckFd80017454 &&
             (attemptFeedback->openCheckFd80017454 == -1 ||
              (attemptFeedback->openCheckCloseKnown80017454 &&
               attemptFeedback->openCheckCloseFd80017454 ==
                   attemptFeedback->openCheckFd80017454)));
        const bool openCheckFailed =
            checkExists != 0 &&
            openCheckComplete &&
            attemptFeedback &&
            attemptFeedback->openCheckFd80017454 == -1;
        const bool openCheckPassed =
            checkExists == 0 || (openCheckComplete && !openCheckFailed);
        if (!openCheckFailed) {
            AppendAction(out,
                         PrStage1SaveUi19148ActionKind::CardHalOpenWrite,
                         kFn80017454,
                         state,
                         state,
                         32770,
                         attempt,
                         static_cast<int32_t>(nameAddress));
            if (!attemptFeedback ||
                !attemptFeedback->openWriteKnown80017454) {
                AppendHelperGap(out,
                                kFn80017454,
                                state,
                                state,
                                32770,
                                attempt);
                carrier.helperGap = true;
            }
        }
        const bool openWriteComplete =
            attemptFeedback &&
            openCheckPassed &&
            attemptFeedback->openWriteKnown80017454 &&
            attemptFeedback->openWriteFdKnown80017454 &&
            attemptFeedback->openWriteReturnKnown80017454 &&
            attemptFeedback->openWriteReturn80017454 ==
                attemptFeedback->openWriteFd80017454;
        const bool openWriteFailed =
            openWriteComplete && attemptFeedback->openWriteFd80017454 == -1;
        if (!openCheckFailed && !openWriteFailed) {
            const bool clearSwKnown =
                attemptFeedback &&
                attemptFeedback->clearSwEventsKnown80016FC0;
            AppendSub80016FC0Action(out, state, clearSwKnown);
            if (!clearSwKnown) {
                carrier.helperGap = true;
            }
            AppendAction(out,
                         PrStage1SaveUi19148ActionKind::CardHalWrite,
                         kFn80017454,
                         state,
                         state,
                         static_cast<int32_t>(dataAddress),
                         blocks << kSaveUiCardBlockShift80017454,
                         attempt);
            if (!attemptFeedback || !attemptFeedback->writeKnown80017454) {
                AppendHelperGap(out,
                                kFn80017454,
                                state,
                                state,
                                static_cast<int32_t>(dataAddress),
                                blocks << kSaveUiCardBlockShift80017454,
                                attempt);
                carrier.helperGap = true;
            }
        }
        AppendAction(out,
                     PrStage1SaveUi19148ActionKind::Call80035560CardWait,
                     kFn80035560,
                     state,
                     state,
                     4,
                     attempt);
        const bool pollKnown =
            attemptFeedback && attemptFeedback->pollResultKnown80016EB8;
        AppendSub80016EB8Action(out, state, attempt, pollKnown);
        AppendAction(out,
                     PrStage1SaveUi19148ActionKind::CardHalClose,
                     kFn80017A10,
                     state,
                     state,
                     static_cast<int32_t>(0x8006ECF8u),
                     attempt);
        if (!attemptFeedback || !attemptFeedback->closeResultKnown) {
            AppendHelperGap(out,
                            kFn80017A10,
                            state,
                            state,
                            static_cast<int32_t>(0x8006ECF8u),
                            attempt);
            carrier.helperGap = true;
        }
        const bool submitKnown =
            attemptFeedback && attemptFeedback->submitReturnKnown80017454;
        const bool submitSucceeded =
            attemptFeedback &&
            submitKnown &&
            attemptFeedback->submitReturn80017454 == 0;
        const bool waitPollCloseComplete =
            attemptFeedback &&
            attemptFeedback->waitCallKnown80035560 &&
            attemptFeedback->waitArg80035560 == 4 &&
            attemptFeedback->pollResultKnown80016EB8 &&
            attemptFeedback->closeResultKnown &&
            attemptFeedback->closeFdKnown &&
            attemptFeedback->gp696FdCloseKnown80017A10 &&
            attemptFeedback->closeFd ==
                attemptFeedback->gp696FdClose80017A10;
        const bool submitEarlyFailed =
            submitKnown &&
            attemptFeedback->submitReturn80017454 == -1 &&
            (openCheckFailed || openWriteFailed);
        const bool writeSubmitted =
            attemptFeedback &&
            scanKnown &&
            openCheckComplete &&
            submitSucceeded &&
            openWriteComplete &&
            attemptFeedback->openWriteFd80017454 != -1 &&
            attemptFeedback->gp696FdWriteKnown80017454 &&
            attemptFeedback->gp696Fd80017454 ==
                attemptFeedback->openWriteFd80017454 &&
            attemptFeedback->clearSwEventsKnown80016FC0 &&
            attemptFeedback->writeKnown80017454 &&
            attemptFeedback->writeByteCountKnown80017454 &&
            attemptFeedback->writeByteCount80017454 ==
                (blocks << kSaveUiCardBlockShift80017454) &&
            attemptFeedback->writeReturnKnown80017454 &&
            waitPollCloseComplete &&
            attemptFeedback->gp696FdClose80017A10 ==
                attemptFeedback->gp696Fd80017454;
        const bool attemptComplete =
            (writeSubmitted || submitEarlyFailed) && waitPollCloseComplete;
        if (!attemptComplete) {
            AppendHelperGap(out,
                            kFn80017A10,
                            state,
                            kStateExit80018FB0,
                            attempt,
                            static_cast<int32_t>(nameAddress),
                            static_cast<int32_t>(dataAddress),
                            blocks);
            carrier.helperGap = true;
            break;
        }
        if (!pollKnown) {
            carrier.helperGap = true;
            break;
        }
        if (attemptFeedback->pollResult80016EB8 == 1) {
            if (!writeSubmitted) {
                AppendHelperGap(out,
                                kFn80017A10,
                                state,
                                kStateExit80018FB0,
                                attempt,
                                static_cast<int32_t>(nameAddress),
                                static_cast<int32_t>(dataAddress),
                                blocks);
                carrier.helperGap = true;
                break;
            }
            carrier.resultKnown = true;
            carrier.result = 0;
            carrier.stoppedOnSuccess = true;
            break;
        }
        if (attempt == kSaveUiWriteRetryCount80017A10 - 1) {
            carrier.resultKnown = true;
            carrier.result = -1;
        }
    }
    if (!carrier.resultKnown) {
        AppendHelperGap(out,
                        kFn80017A10,
                        state,
                        kStateExit80018FB0,
                        kSaveUiWriteRetryCount80017A10,
                        static_cast<int32_t>(nameAddress),
                        static_cast<int32_t>(dataAddress),
                        blocks);
        carrier.helperGap = true;
    }
    carrier.actions = out.actions;
    return carrier;
}

static PrStage1SaveUiFormatFeedbackCarrier80017B60
AppendSub80017B60FeedbackCarrier(
    PrStage1SaveUi19148TickResult& out,
    int32_t state,
    const PrStage1SaveUiFormatFeedbackInput80017B60* feedback) {
    PrStage1SaveUiFormatFeedbackCarrier80017B60 carrier{};
    AppendAction(out,
                 PrStage1SaveUi19148ActionKind::Call80017B60FormatCard,
                 kFn80017B60,
                 state,
                 4,
                 static_cast<int32_t>(0x8006EABCu),
                 static_cast<int32_t>(0x8006EAC0u),
                 kSaveUiFormatRetryCount80017B60);
    for (int32_t attempt = 0; attempt < kSaveUiFormatRetryCount80017B60;
         ++attempt) {
        carrier.attemptsUsed = attempt + 1;
        AppendAction(out,
                     PrStage1SaveUi19148ActionKind::Call8001707CDrainHwCardEvents,
                     kFn8001707C,
                     state,
                     state,
                     attempt,
                     static_cast<int32_t>(0x8006ECE8u),
                     4);
        const PrStage1SaveUiFormatAttemptFeedback80017B60* attemptFeedback =
            feedback ? &feedback->attempts[attempt] : nullptr;
        if (!attemptFeedback ||
            !attemptFeedback->drainHwEventsKnown8001707C) {
            AppendHelperGap(out, kFn8001707C, state, state, 4, attempt);
            carrier.helperGap = true;
        }
        AppendAction(out,
                     PrStage1SaveUi19148ActionKind::CardHalFormat,
                     kFn80017B60,
                     state,
                     state,
                     attempt,
                     static_cast<int32_t>(0x8006EABCu),
                     static_cast<int32_t>(0x8006EAC0u));
        if (!attemptFeedback || !attemptFeedback->formatKnown) {
            AppendHelperGap(out, kFn80017B60, state, state, attempt);
            carrier.helperGap = true;
        }
        AppendAction(out,
                     PrStage1SaveUi19148ActionKind::Call80017008PollHwCardEvents,
                     kFn80017008,
                     state,
                     state,
                     attempt,
                     static_cast<int32_t>(0x8006ECE8u),
                     4);
        if (!attemptFeedback ||
            !attemptFeedback->pollResultKnown80017008) {
            AppendHelperGap(out, kFn80017008, state, state, 4, attempt);
            carrier.helperGap = true;
            break;
        }
        if (attemptFeedback->pollResult80017008 == 1) {
            carrier.resultKnown = true;
            carrier.result = 1;
            carrier.stoppedOnSuccess = true;
            break;
        }
        if (attemptFeedback->pollResult80017008 == 3) {
            carrier.resultKnown = true;
            carrier.result = 3;
            carrier.stoppedOnTimeout = true;
            break;
        }
        if (attempt == kSaveUiFormatRetryCount80017B60 - 1) {
            carrier.resultKnown = true;
            carrier.result = attemptFeedback->pollResult80017008;
        }
    }
    if (!carrier.resultKnown) {
        AppendHelperGap(out,
                        kFn80017B60,
                        state,
                        4,
                        kSaveUiFormatRetryCount80017B60);
        carrier.helperGap = true;
    }
    carrier.actions = out.actions;
    return carrier;
}

static bool Sub80017C08Direct(uint32_t dstAddress, uint32_t titleAddress) {
    if (dstAddress != kAddrSaveBuffers8007ABE8 ||
        titleAddress != static_cast<uint32_t>(kSaveUiEncodedTitleBuffer80019458)) {
        s_saveUiMemory.payloadBank.boundsFault = true;
        s_saveUiMemory.payloadBank.lastFaultAddress = dstAddress;
        return false;
    }
    if (!Sub80025C44Direct(dstAddress, kSaveUiSaveHeaderBytes80017C08) ||
        !DirectWriteByte(dstAddress + 0u, 'S') ||
        !DirectWriteByte(dstAddress + 1u, 'C') ||
        !DirectWriteByte(dstAddress + 2u, 0x13u) ||
        !DirectWriteByte(dstAddress + 3u, 1u)) {
        return false;
    }

    for (int32_t i = 0; i < kSaveUiEncodedTitleBytes80017C08; ++i) {
        uint8_t value = 0;
        if (!DirectReadByte(titleAddress + static_cast<uint32_t>(i), value) ||
            !DirectWriteByte(dstAddress + kHeaderTitleOffset80017C08 +
                                 static_cast<uint32_t>(i),
                             value)) {
            return false;
        }
        if (value == 0) {
            break;
        }
        if (i == kSaveUiEncodedTitleBytes80017C08 - 1) {
            return false;
        }
    }

    static constexpr uint32_t kFixedDwords[] = {
        0x00DF0000u,
        0x00BB00DDu,
        0x00D90199u,
        0x00B200B9u,
        0x008F0092u,
        0x008C008Du,
        0x0069006Bu,
        0x1A660047u,
    };
    for (uint32_t i = 0; i < sizeof(kFixedDwords) / sizeof(kFixedDwords[0]);
         ++i) {
        if (!DirectWriteLe32(dstAddress + kHeaderFixedDwordsOffset80017C08 +
                                 i * 4u,
                             kFixedDwords[i])) {
            return false;
        }
    }

    return Sub80025C64Direct(kHeaderIconChunk0Src80017C08,
                             dstAddress + kHeaderIconChunk0DstOffset80017C08,
                             kHeaderIconChunkBytes80017C08) &&
           Sub80025C64Direct(kHeaderIconChunk1Src80017C08,
                             dstAddress + kHeaderIconChunk1DstOffset80017C08,
                             kHeaderIconChunkBytes80017C08) &&
           Sub80025C64Direct(kHeaderIconChunk2Src80017C08,
                             dstAddress + kHeaderIconChunk2DstOffset80017C08,
                             kHeaderIconChunkBytes80017C08);
}

static bool Sub80017FC4Direct(uint32_t dstAddress, uint32_t srcAddress) {
    uint32_t offset = 0;
    while (true) {
        uint8_t srcByte = 0;
        if (!DirectReadByte(srcAddress + offset, srcByte)) {
            return false;
        }
        if (srcByte == 0) {
            return DirectWriteByte(dstAddress + offset, 0);
        }

        uint8_t mappedByte = 0;
        if (!DirectReadByte(kAddrGlyphMap800491C4 + srcByte, mappedByte) ||
            !DirectWriteByte(dstAddress + offset, mappedByte)) {
            return false;
        }
        ++offset;
    }
}

static void AppendSfx80025C8C(PrStage1SaveUi19148TickResult& out,
                              int32_t stateBefore,
                              int32_t stateAfter,
                              int32_t code) {
    AppendAction(out,
                 PrStage1SaveUi19148ActionKind::Call80025C8CPlayInputSfx,
                 kFn80025C8C,
                 stateBefore,
                 stateAfter,
                 code);
}

static void AppendListInput800181D0(PrStage1SaveUi19148TickResult& out,
                                    int32_t stateBefore,
                                    int32_t stateAfter,
                                    int32_t input,
                                    int32_t selected,
                                    int32_t result) {
    AppendAction(out,
                 PrStage1SaveUi19148ActionKind::Call800181D0ListInput,
                 kFn800181D0,
                 stateBefore,
                 stateAfter,
                 input,
                 selected,
                 result,
                 s_saveUi19148.byte8007CBE8Present ? 1 : 0);
}

static void Sub80017FC4Preview(PrStage1SaveUi19148TickResult& out,
                               int32_t stateBefore,
                               int32_t stateAfter,
                               const char* source,
                               int32_t targetTag) {
    CopySmallString(s_saveUi19148.encodedNamePreview,
                    sizeof(s_saveUi19148.encodedNamePreview),
                    source ? source : "");
    const uint32_t srcAddress =
        (source == s_saveUi19148.byte80049260)
            ? static_cast<uint32_t>(kSaveUiNameRawBuffer80018060)
            : kAddrDefaultNamePreview8006EAF0;
    const bool translated =
        Sub80017FC4Direct(static_cast<uint32_t>(targetTag), srcAddress);
    if (!translated) {
        AppendHelperGap(out, kFn80017FC4, stateBefore, stateAfter, targetTag);
    }
}

static int32_t FindListRowFromHigh800181D0() {
    int32_t v4 = 14;
    while (true) {
        const int32_t v5 = (v4 >= 0) ? v4 : 15;
        v4 = v5 - 1;
        if (v5 >= 0 && v5 < kSaveUiListRowCount800181D0 &&
            s_saveUi19148.listRowActive[v5]) {
            return v5;
        }
        if (v4 < -1) {
            return 15;
        }
    }
}

static int32_t FindListRowFromLow800181D0() {
    int32_t v6 = 2;
    for (int guard = 0; guard < 16; ++guard) {
        const int32_t v7 = v6 % 16;
        v6 = v7 + 1;
        if (v7 >= 0 && v7 < kSaveUiListRowCount800181D0 &&
            s_saveUi19148.listRowActive[v7]) {
            return v7;
        }
    }
    return 15;
}

static void ClearListRows80019458() {
    s_saveUi19148.word8007ABE4_entryCount = 0;
    s_saveUi19148.word8007ABE6_freeSlots = 0;
    s_saveUi19148.word80048E62_entryCount = 0;
    s_saveUi19148.word80048E64_selected = 15;
    s_saveUi19148.word80048E84_dirty = 0;
    s_saveUi19148.byte8007CBE8Present = false;
    s_saveUi19148.byte8007CBE8[0] = '\0';
    s_saveUi19148.encodedTitleBufferPresent = false;
    for (int i = 0; i < kSaveUiListRowCount800181D0; ++i) {
        s_saveUi19148.listRowActive[i] = false;
        s_saveUi19148.listRowFreeSlot[i] = false;
        s_saveUi19148.listRowBlockIndex[i] = -1;
        s_saveUi19148.listRowSuffix[i][0] = '\0';
    }
}

static bool ApplyTypedDirectoryRows80019458() {
    ClearListRows80019458();

    constexpr bool kSaveUiTypedDirectoryRows80019458Wired = false;
    if (!kSaveUiTypedDirectoryRows80019458Wired) {
        return false;
    }
    return false;
}

static bool EnumerateDirectoryListGap80019458(
    PrStage1SaveUi19148TickResult& out,
    int32_t stateBefore,
    int32_t stateAfter,
    bool withCompare) {
    if (withCompare && s_saveUi19148.gp712_overwriteScanFlag != 0) {
        (void)Sub800488E4Direct(
            kAddrPreviousSnapshot8007CC74, kSaveUiFilenameBuffer80019458, 600);
    }
    const uint32_t dirBank = Sub80017B08Direct();
    AppendAction(out,
                 PrStage1SaveUi19148ActionKind::Call80017B08EnumerateEntries,
                 kFn80017B08,
                 stateBefore,
                 stateAfter,
                 static_cast<int32_t>(dirBank),
                 kSaveUiDirEntrySize80019458);
    AppendAction(out,
                 PrStage1SaveUi19148ActionKind::Call80017B18SnapshotDirectory,
                 kFn80017B18,
                 stateBefore,
                 stateAfter,
                 static_cast<int32_t>(dirBank),
                 static_cast<int32_t>(kFn800178C8),
                 static_cast<int32_t>(kFn80017354),
                 kSaveUiDirEntryCount80019458);
    const int32_t clearBytes =
        kSaveUiListEntrySize80019458 * kSaveUiListRowCount800181D0 + 4;
    if (!Sub80025C44Direct(dirBank, clearBytes)) {
        AppendHelperGap(out, kFn80025C44, stateBefore, stateAfter, clearBytes);
    }
    if (!ApplyTypedDirectoryRows80019458()) {
        AppendHelperGap(out, kFn80019458, stateBefore, stateAfter, 0);
        return false;
    }
    return true;
}

static bool StartDispatcherEvent(int32_t eventId,
                                 int32_t select,
                                 int32_t flag,
                                 PrGameContext& ctx,
                                 PrStage1SaveUi19148TickResult& out,
                                 int32_t stateBefore,
                                 int32_t stateAfter) {
    (void)ctx;
    AppendAction(out,
                 PrStage1SaveUi19148ActionKind::Call80017E6CSetEventResult,
                 kFn80017E6C,
                 stateBefore,
                 stateAfter,
                 eventId,
                 select,
                 flag);
    return true;
}

static void Sub80018060(PrStage1SaveUi19148TickResult& out,
                        int32_t stateBefore,
                        int32_t stateAfter) {
    AppendAction(out,
                 PrStage1SaveUi19148ActionKind::Call80017E58InitEventArg,
                 kFn80017E58,
                 stateBefore,
                 stateAfter,
                 kSaveUiNameInputEventArg80018060);
    CopySuffix("");
    s_saveUi19148.word8004925A = 0;
    s_saveUi19148.word8004925C = 0;
    s_saveUi19148.word8004925E = 0;
    s_saveUi19148.dword80049248 = 0;
    std::memset(s_saveUi19148.byte80049260,
                0,
                sizeof(s_saveUi19148.byte80049260));
    std::memset(s_saveUi19148.encodedNamePreview,
                0,
                sizeof(s_saveUi19148.encodedNamePreview));
    Sub80025C44Direct(0x8004925Au, 6);
    Sub80025C44Direct(static_cast<uint32_t>(kSaveUiNameRawBuffer80018060), 6);
    s_saveUi19148.byte8007CBE8Present = false;
    s_saveUi19148.encodedTitleBufferPresent = false;
    AppendAction(out,
                 PrStage1SaveUi19148ActionKind::Call80018060InitNameInput,
                 kFn80018060,
                 stateBefore,
                 stateAfter);
    Sub80017FC4Preview(out,
                       stateBefore,
                       stateAfter,
                       "",
                       kSaveUiNamePreviewTarget80017FC4);
}

static void Sub800180D8(int32_t state,
                        PrStage1SaveUi19148TickResult& out) {
    const int32_t stateBefore = state;
    switch (state) {
    case 2:
        s_saveUi19148.eventId = 11;
        break;
    case 5:
        s_saveUi19148.eventId = 12;
        break;
    case 7:
        s_saveUi19148.eventId = 14;
        break;
    case 8:
        s_saveUi19148.eventId = 13;
        break;
    case 10:
        s_saveUi19148.eventId = 5;
        AppendHelperGap(out, kFn800180D8, stateBefore, state, 10);
        break;
    case 11:
        s_saveUi19148.eventId = 7;
        break;
    case 12:
        s_saveUi19148.eventId = 8;
        break;
    case 13:
        s_saveUi19148.eventId = 9;
        break;
    case 18:
        s_saveUi19148.eventId = 15;
        break;
    case 22:
        s_saveUi19148.eventId = 19;
        break;
    default:
        break;
    }
    AppendAction(out,
                 PrStage1SaveUi19148ActionKind::Call80017E6CSetEventResult,
                 kFn800180D8,
                 stateBefore,
                 state,
                 s_saveUi19148.eventId);
}

static int32_t Sub800181D0(int32_t input,
                           PrStage1SaveUi19148TickResult& out,
                           int32_t stateBefore,
                           int32_t stateAfter) {
    int32_t v1 = 0;
    switch (input) {
    case kInputCross800185D0: {
        AppendSfx80025C8C(out, stateBefore, stateAfter, 0x20);
        s_saveUi19148.gp716_saveOk = 1;
        const int32_t selected = s_saveUi19148.word80048E64_selected;
        if (selected >= 0 && selected < kSaveUiListRowCount800181D0) {
            CopySmallString(s_saveUi19148.byte8007CBE8,
                            sizeof(s_saveUi19148.byte8007CBE8),
                            s_saveUi19148.listRowSuffix[selected]);
            s_saveUi19148.byte8007CBE8Present =
                s_saveUi19148.byte8007CBE8[0] != '\0';
        } else {
            s_saveUi19148.byte8007CBE8[0] = '\0';
            s_saveUi19148.byte8007CBE8Present = false;
        }
        AppendListInput800181D0(out,
                                stateBefore,
                                stateAfter,
                                input,
                                selected,
                                1);
        return 1;
    }
    case kInputNameUp800185D0:
        AppendSfx80025C8C(out, stateBefore, stateAfter, 0x1000);
        s_saveUi19148.word80048E64_selected = FindListRowFromHigh800181D0();
        AppendListInput800181D0(out,
                                stateBefore,
                                stateAfter,
                                input,
                                s_saveUi19148.word80048E64_selected,
                                0);
        return 0;
    case kInputNameLeft800185D0:
        AppendSfx80025C8C(out, stateBefore, stateAfter, 0x1000);
        v1 = 15;
        break;
    case kInputNameRight800185D0:
        AppendSfx80025C8C(out, stateBefore, stateAfter, 0x1000);
        s_saveUi19148.word80048E64_selected = FindListRowFromLow800181D0();
        AppendListInput800181D0(out,
                                stateBefore,
                                stateAfter,
                                input,
                                s_saveUi19148.word80048E64_selected,
                                0);
        return 0;
    case kInputNameDown800185D0:
        AppendSfx80025C8C(out, stateBefore, stateAfter, 0x1000);
        v1 = FindListRowFromHigh800181D0();
        break;
    default:
        AppendListInput800181D0(out,
                                stateBefore,
                                stateAfter,
                                input,
                                s_saveUi19148.word80048E64_selected,
                                0);
        return 0;
    }
    s_saveUi19148.word80048E64_selected = v1;
    AppendListInput800181D0(out,
                            stateBefore,
                            stateAfter,
                            input,
                            s_saveUi19148.word80048E64_selected,
                            0);
    return 0;
}

static int32_t HandleInput800185D0(int32_t input,
                                   int32_t state,
                                   PrGameContext& ctx,
                                   PrStage1SaveUi19148TickResult& out) {
    int32_t v4 = state;
    switch (state) {
    case 2:
        if (input == kInputCross800185D0) {
            AppendAction(out,
                         PrStage1SaveUi19148ActionKind::Call80025C8CPlayInputSfx,
                         kFn80025C8C,
                         state,
                         4,
                         0x20);
            StartDispatcherEvent(11, 1, 0, ctx, out, state, 4);
            return 4;
        }
        if (input == kInputCircle800185D0) {
            AppendAction(out,
                         PrStage1SaveUi19148ActionKind::Call80025C8CPlayInputSfx,
                         kFn80025C8C,
                         state,
                         kStateExit80018FB0,
                         0x20);
            StartDispatcherEvent(
                11, 2, 0, ctx, out, state, kStateExit80018FB0);
            return kStateExit80018FB0;
        }
        return state;

    case 5:
        if (input == kInputCross800185D0) {
            AppendAction(out,
                         PrStage1SaveUi19148ActionKind::Call80025C8CPlayInputSfx,
                         kFn80025C8C,
                         state,
                         2,
                         0x20);
            StartDispatcherEvent(12, 2, 0, ctx, out, state, 2);
            return 2;
        }
        return state;

    case 7:
        if (input == kInputCross800185D0) {
            AppendAction(out,
                         PrStage1SaveUi19148ActionKind::Call80025C8CPlayInputSfx,
                         kFn80025C8C,
                         state,
                         2,
                         0x20);
            StartDispatcherEvent(14, 2, 0, ctx, out, state, 2);
            return 2;
        }
        return state;

    case 8:
        if (input == kInputCross800185D0) {
            AppendAction(out,
                         PrStage1SaveUi19148ActionKind::Call80025C8CPlayInputSfx,
                         kFn80025C8C,
                         state,
                         14,
                         0x20);
            StartDispatcherEvent(13, 1, 1, ctx, out, state, 14);
            return 14;
        }
        if (input == kInputCircle800185D0) {
            AppendAction(out,
                         PrStage1SaveUi19148ActionKind::Call80025C8CPlayInputSfx,
                         kFn80025C8C,
                         state,
                         2,
                         0x20);
            StartDispatcherEvent(13, 2, 0, ctx, out, state, 2);
            return 2;
        }
        return state;

    case 10: {
        int32_t v12 = 0;
        int32_t v17 = 0;
        s_saveUi19148.word8004925E = 0;
        if (input == kInputNameUp800185D0) {
            AppendSfx80025C8C(out, state, state, 0x1000);
            v12 = 56;
        }
        if (input == kInputNameLeft800185D0) {
            AppendSfx80025C8C(out, state, state, 0x1000);
            v12 = 14;
        }
        if (input == kInputNameRight800185D0) {
            AppendSfx80025C8C(out, state, state, 0x1000);
            v12 = 1;
        }
        int32_t v13 = input & kInputNameMask800185D0;
        if (input == kInputNameDown800185D0) {
            AppendSfx80025C8C(out, state, state, 0x1000);
            v12 = 56;
            v13 = 0;
        }
        s_saveUi19148.word8004925A = v12;
        if (v13 != 0) {
            const uint8_t v14 =
                static_cast<uint8_t>(kNameInputCharTable800490E8[v12]);
            if (v14 == 8) {
                AppendSfx80025C8C(out, state, state, 0x40);
            } else {
                bool v15 = v14 < 32;
                if (v14 == 10) {
                    if (input == kInputCross800185D0) {
                        AppendSfx80025C8C(out, state, state, 0x20);
                        s_saveUi19148.word8004925E = 1;
                        s_saveUi19148.dword80049248 = 1;
                        v17 = 1;
                    } else {
                        v15 = true;
                        if (input == kInputCircle800185D0) {
                            AppendSfx80025C8C(out, state, state, 0x20);
                            s_saveUi19148.word8004925E = 2;
                            s_saveUi19148.dword80049248 = 2;
                            v17 = 2;
                        }
                    }
                }
                if (!v15) {
                    AppendSfx80025C8C(out, state, state, 0x100);
                    s_saveUi19148.word8004925C = 1;
                    s_saveUi19148.byte80049260[0] = '\0';
                }
            }
        }
        if (input == kInputTriangle800185D0) {
            AppendSfx80025C8C(out, state, state, 0x40);
        }
        Sub80017FC4Preview(out,
                           state,
                           state,
                           s_saveUi19148.byte80049260,
                           kSaveUiNamePreviewTarget80017FC4);
        if (v17 == 1) {
            StartDispatcherEvent(5, 1, 1, ctx, out, state, state);
            if (!EnumerateDirectoryListGap80019458(out, state, 18, false)) {
                return state;
            }
            if (s_saveUi19148.word8007ABE6_freeSlots <= 0) {
                return 7;
            }
            s_saveUi19148.gp712_overwriteScanFlag = 1;
            StartDispatcherEvent(5, 1, 1, ctx, out, state, 15);
            return 15;
        }
        if (v17 == 2) {
            StartDispatcherEvent(5, 2, 0, ctx, out, state, 2);
            return 2;
        }
        return v4;
    }

    case 11: {
        const int32_t v35 = Sub800181D0(input, out, state, state);
        if (v35 == 1) {
            v4 = 22;
            if (s_saveUi19148.byte8007CBE8Present) {
                return v4;
            }
            Sub80018060(out, state, 10);
            return 10;
        }
        if (v35 == 2) {
            StartDispatcherEvent(7, 1, 0, ctx, out, state, 2);
            return 2;
        }
        return v4;
    }

    case 18:
        if (input == kInputCross800185D0) {
            AppendAction(out,
                         PrStage1SaveUi19148ActionKind::Call80025C8CPlayInputSfx,
                         kFn80025C8C,
                         state,
                         10,
                         0x20);
            StartDispatcherEvent(15, 2, 0, ctx, out, state, 10);
            Sub80018060(out, state, 10);
            return 10;
        }
        return state;

    case 22:
        if (input == kInputCross800185D0) {
            AppendSfx80025C8C(out, state, 15, 0x20);
            StartDispatcherEvent(19, 1, 1, ctx, out, state, 15);
            CopySuffix(s_saveUi19148.byte8007CBE8Present
                           ? s_saveUi19148.byte8007CBE8
                           : "");
            return 15;
        }
        if (input == kInputCircle800185D0) {
            AppendSfx80025C8C(out, state, 11, 0x20);
            StartDispatcherEvent(19, 2, 0, ctx, out, state, 11);
            return 11;
        }
        return state;

    default:
        return v4;
    }
}

static int32_t TickState80019458(int32_t state,
                                 int32_t ioResult,
                                 PrStage1SaveUi19148TickResult& out,
                                 const PrStage1SaveUi19148LowerFeedback*
                                     lowerFeedback) {
    switch (state) {
    case 4:
        switch (ioResult) {
        case 1:
        case 4:
            return 6;
        case 3:
            return 5;
        case 5:
            return s_saveUi19148.gp732_mode ? 19 : 8;
        default:
            return state;
        }

    case 6:
        if (!EnumerateDirectoryListGap80019458(out, state, 9, false)) {
            return state;
        }
        return s_saveUi19148.word8007ABE4_entryCount > 0 ? 9 : 7;

    case 8:
        if (ioResult == 3) {
            return 5;
        }
        return state;

    case 9:
        if (!EnumerateDirectoryListGap80019458(out, state, 11, true)) {
            return state;
        }
        AppendAction(out,
                     PrStage1SaveUi19148ActionKind::Call80017E58InitEventArg,
                     kFn80017E58,
                     state,
                     11,
                     kSaveUiListEventArg80019458);
        s_saveUi19148.word80048E62_entryCount =
            s_saveUi19148.word8007ABE4_entryCount;
        s_saveUi19148.word80048E64_selected = 15;
        s_saveUi19148.word80048E84_dirty = 1;
        return 11;

    case 10:
    case 11:
        if (ioResult == 3) {
            return 4;
        }
        return state;

    case 14:
        if (ioResult == 3) {
            return 2;
        }
        (void)AppendSub80017B60FeedbackCarrier(
            out,
            state,
            lowerFeedback && lowerFeedback->formatFeedbackKnown80017B60
                ? &lowerFeedback->formatFeedback80017B60
                : nullptr);
        return 4;

    case 15: {
        if (ioResult == 3) {
            return 2;
        }
        (void)ProduceState15FilenameAndTitle80019458(out, state);
        const bool cleared =
            Sub80025C44Direct(kAddrSaveBuffers8007ABE8,
                              kSaveUiSaveBlockBytes80017C08);
        if (!cleared) {
            AppendHelperGap(out,
                            kFn80025C44,
                            state,
                            state,
                            kSaveUiSaveBlockBytes80017C08);
        }
        if (s_saveUi19148.encodedTitleBufferPresent &&
            Sub80017C08Direct(kAddrSaveBuffers8007ABE8,
                              kSaveUiEncodedTitleBuffer80019458)) {
            s_saveUi19148.saveHeaderBuilt = true;
        } else {
            s_saveUi19148.saveHeaderBuilt = false;
            AppendHelperGap(out,
                            kFn80017C08,
                            state,
                            state,
                            kSaveUiSaveHeaderBytes80017C08);
        }
        const bool payloadConsumed =
            CopyState15SavePayload80019458(out, state);
        const PrStage1SaveUiWriteFeedbackInput80017A10* writeFeedback =
            lowerFeedback && lowerFeedback->writeFeedbackKnown80017A10
                ? &lowerFeedback->writeFeedback80017A10
                : nullptr;
        const PrStage1SaveUiWriteFeedbackCarrier80017A10 writeCarrier =
            AppendSub80017A10FeedbackCarrier(
            out,
            state,
            static_cast<uint32_t>(kSaveUiFilenameBuffer80019458),
            kAddrSaveBuffers8007ABE8,
            kSaveUiWriteBlocks80019458,
            writeFeedback);
        out.saveWriteResultKnown80017A10 = writeCarrier.resultKnown;
        out.saveWriteResult80017A10 = writeCarrier.result;
        out.saveWriteSucceeded80019458 =
            payloadConsumed && writeCarrier.resultKnown &&
            writeCarrier.result >= 0;
        if (!out.saveWriteSucceeded80019458) {
            AppendHelperGap(out,
                            kFn80019458,
                            state,
                            state,
                            payloadConsumed ? 1 : 0,
                            writeCarrier.resultKnown ? 1 : 0,
                            writeCarrier.result);
            return state;
        }
        s_saveUi19148.gp716_saveOk = 1;
        s_saveUi19148.gp720_result = 1;
        out.gp716After80019458Known = true;
        out.gp716After80019458 = s_saveUi19148.gp716_saveOk;
        out.gp720After80019458Known = true;
        out.gp720After80019458 = s_saveUi19148.gp720_result;
        AppendAction(out,
                     PrStage1SaveUi19148ActionKind::
                         Call80019458CommitResultState15,
                     kFn80019458,
                     state,
                     kStateExit80018FB0,
                     s_saveUi19148.gp716_saveOk,
                     s_saveUi19148.gp720_result,
                     writeCarrier.result);
        return kStateExit80018FB0;
    }

    case 21:
        AppendAction(out,
                     PrStage1SaveUi19148ActionKind::Call80017E58InitEventArg,
                     kFn80017E58,
                     state,
                     2,
                     kInitialEvent80019148);
        return 2;

    default:
        return state;
    }
}

static bool StateConsumesInput80018FB0(int32_t state) {
    switch (state) {
    case 2:
    case 5:
    case 7:
    case 8:
    case 10:
    case 11:
    case 18:
    case 22:
        return true;
    default:
        return false;
    }
}

static int32_t BuildLocalSaveUiInputMask80035510(uint16_t localMask) {
    int32_t inputMask = 0;
    if ((localMask & (uint16_t)PrPadButton::Cross) != 0u) {
        inputMask |= kInputCross800185D0;
    }
    if ((localMask & (uint16_t)PrPadButton::Circle) != 0u) {
        inputMask |= kInputCircle800185D0;
    }
    if ((localMask & (uint16_t)PrPadButton::Triangle) != 0u) {
        inputMask |= kInputTriangle800185D0;
    }
    if ((localMask & (uint16_t)PrPadButton::Up) != 0u) {
        inputMask |= kInputNameUp800185D0;
    }
    if ((localMask & (uint16_t)PrPadButton::Left) != 0u) {
        inputMask |= kInputNameLeft800185D0;
    }
    if ((localMask & (uint16_t)PrPadButton::Right) != 0u) {
        inputMask |= kInputNameRight800185D0;
    }
    if ((localMask & (uint16_t)PrPadButton::Down) != 0u) {
        inputMask |= kInputNameDown800185D0;
    }
    return inputMask;
}

static int32_t BuildSaveUiInputMask80035510(const PrGameContext& ctx) {
    const PrPadState pad = PrPad::GetState(0);
    const uint16_t localMask =
        static_cast<uint16_t>(pad.pressed | pad.held);
    const int32_t localInputMask =
        BuildLocalSaveUiInputMask80035510(localMask);
    const int32_t debugInputMask =
        ctx.debugPadInput &
        (kInputCross800185D0 | kInputCircle800185D0 |
         kInputTriangle800185D0 | kInputNameUp800185D0 |
         kInputNameLeft800185D0 | kInputNameRight800185D0 |
         kInputNameDown800185D0);
    return localInputMask | debugInputMask;
}

static void StepBlinkAndDraw80018FB0(PrStage1SaveUi19148TickResult& out) {
    if (s_saveUi19148.state >= 15 && s_saveUi19148.state < 17) {
        s_saveUi19148.gp728_blinkCounter = 0;
    } else if (s_saveUi19148.gp728_blinkCounter != 0) {
        if (s_saveUi19148.gp728_blinkCounter < 19) {
            ++s_saveUi19148.gp728_blinkCounter;
        } else {
            s_saveUi19148.gp728_blinkCounter = 0;
        }
    } else {
        s_saveUi19148.gp728_blinkCounter = 1;
    }
    AppendAction(out,
                 PrStage1SaveUi19148ActionKind::Call8001E750DrawEvent,
                 kFn8001E750,
                 s_saveUi19148.state,
                 s_saveUi19148.state,
                 s_saveUi19148.eventId);
    AppendAction(out,
                 PrStage1SaveUi19148ActionKind::Call80035560ResetInput,
                 kFn80035560,
                 s_saveUi19148.state,
                 s_saveUi19148.state,
                 0);
    AppendAction(out,
                 PrStage1SaveUi19148ActionKind::Call8001EA00EndFrame,
                 kFn8001EA00,
                 s_saveUi19148.state,
                 s_saveUi19148.state,
                 0);
}

}  // namespace

namespace PrStage1SaveUiDirect {

void Reset19148() {
    s_saveUi19148 = SaveUi19148Runtime{};
}

bool Start19148(PrGameContext& ctx) {
    return Start19148(ctx, nullptr);
}

bool ImportSaveStatusPrefix80092F10(
    const PrStage1SaveStatusPrefix80092F10& seed) {
    return ImportSaveStatusPrefix80092F10Internal(seed);
}

bool Start19148(PrGameContext& ctx,
                const PrStage1SaveStatusPrefix80092F10* seed80092F10) {
    (void)ctx;
    if (s_saveUi19148.active) {
        return true;
    }

    s_saveUi19148 = SaveUi19148Runtime{};
    ResetDirectMemory();
    if (seed80092F10 && !seed80092F10->statusBankKnown80092F1D) {
        Log::Printf(
            "Stage1 SaveUi19148: reject 80092F10 seed with unknown status bank "
            "(known=%d addr=%08X bytes=%u statusKnown=%d)",
            seed80092F10->known ? 1 : 0,
            seed80092F10->psxAddress,
            seed80092F10->byteCount,
            seed80092F10->statusBankKnown80092F1D ? 1 : 0);
        return false;
    }
    if (seed80092F10 && !ImportSaveStatusPrefix80092F10Internal(*seed80092F10)) {
        Log::Printf(
            "Stage1 SaveUi19148: reject incomplete 80092F10 seed "
            "(known=%d addr=%08X bytes=%u)",
            seed80092F10->known ? 1 : 0,
            seed80092F10->psxAddress,
            seed80092F10->byteCount);
        return false;
    }
    s_saveUi19148.active = true;
    s_saveUi19148.state = kInitialState80019148;
    s_saveUi19148.eventId = kInitialEvent80019148;
    Log::Printf(
        "Stage1 SaveUi19148: start PSX state carrier "
        "(80018FB0 state=%d event=%d)",
        s_saveUi19148.state,
        s_saveUi19148.eventId);
    return true;
}

PrStage1SaveUi19148TickResult Tick19148(PrGameContext& ctx) {
    return Tick19148(ctx, nullptr);
}

PrStage1SaveUi19148TickResult Tick19148(
    PrGameContext& ctx,
    const PrStage1SaveUi19148LowerFeedback* lowerFeedback) {
    if (!s_saveUi19148.active) {
        return PrStage1SaveUi19148TickResult{};
    }

    PrStage1SaveUi19148TickResult out = MakeResult(true);
    if (s_saveUi19148.state == kInitialState80019148) {
        AppendAction(out,
                     PrStage1SaveUi19148ActionKind::Call80017E58InitEventArg,
                     kFn80017E58,
                     s_saveUi19148.state,
                     2,
                     s_saveUi19148.eventId);
        s_saveUi19148.dispatcherPending = false;
        s_saveUi19148.dispatcherEventId = 0;
        s_saveUi19148.dispatcherResultPending = 0;
        s_saveUi19148.state = 2;
        out.psxState = s_saveUi19148.state;
        Log::Printf("Stage1 SaveUi19148: direct initial confirm event=%d",
                    kInitialEvent80019148);
    }

    int32_t inputMask = 0;
    const bool dispatcherResultCanDriveInput =
        StateConsumesInput80018FB0(s_saveUi19148.state) &&
        s_saveUi19148.state != 10 &&
        s_saveUi19148.state != 11;
    if (StateConsumesInput80018FB0(s_saveUi19148.state) &&
        !dispatcherResultCanDriveInput &&
        s_saveUi19148.dispatcherResultPending != 0) {
        s_saveUi19148.dispatcherResultPending = 0;
    }
    if (dispatcherResultCanDriveInput &&
        s_saveUi19148.dispatcherResultPending == 1) {
        inputMask = kInputCross800185D0;
        s_saveUi19148.dispatcherResultPending = 0;
    } else if (dispatcherResultCanDriveInput &&
               s_saveUi19148.dispatcherResultPending == 2) {
        inputMask = kInputCircle800185D0;
        s_saveUi19148.dispatcherResultPending = 0;
    } else if (StateConsumesInput80018FB0(s_saveUi19148.state)) {
        inputMask = BuildSaveUiInputMask80035510(ctx);
    }
    if (inputMask == s_saveUi19148.gp708_lastInput) {
        inputMask = 0;
    } else {
        s_saveUi19148.gp708_lastInput = inputMask;
    }
    if (inputMask != 0) {
        const int32_t before = s_saveUi19148.state;
        s_saveUi19148.state =
            HandleInput800185D0(inputMask, before, ctx, out);
        out.psxState = s_saveUi19148.state;
    }

    int32_t ioResult = 0;
    const bool consumedExplicitCardIo =
        TryConsumeTickCardIoFeedback80017594(out, lowerFeedback, ioResult);
    if (!consumedExplicitCardIo &&
        !StateConsumesInput80018FB0(s_saveUi19148.state) &&
        s_saveUi19148.dispatcherResultPending != 0) {
        ioResult = s_saveUi19148.dispatcherResultPending;
        out.ioResultKnown = true;
        s_saveUi19148.dispatcherResultPending = 0;
    } else if (!consumedExplicitCardIo) {
        (void)AppendSub80017594FeedbackCarrier(
            out,
            s_saveUi19148.state,
            s_saveUi19148.cardIoState80017594,
            nullptr);
    }
    s_saveUi19148.gp724_ioResult = ioResult;
    out.ioResult = ioResult;
    const bool shouldTickState =
        ioResult != 0 || s_saveUi19148.state == 15;
    if (shouldTickState) {
        const int32_t before = s_saveUi19148.state;
        s_saveUi19148.state =
            TickState80019458(before, ioResult, out, lowerFeedback);
        out.psxState = s_saveUi19148.state;
        Sub800180D8(s_saveUi19148.state, out);
    }

    if (s_saveUi19148.state != kStateExit80018FB0) {
        StepBlinkAndDraw80018FB0(out);
        out.active = true;
        out.psxState = s_saveUi19148.state;
        out.psxEventId = s_saveUi19148.eventId;
        return out;
    }

    if (out.helperGap) {
        out.active = true;
        out.psxState = s_saveUi19148.state;
        out.psxEventId = s_saveUi19148.eventId;
        return out;
    }

    AppendAction(out,
                 PrStage1SaveUi19148ActionKind::Call80017E6CSetEventResult,
                 kFn80017E6C,
                 s_saveUi19148.state,
                 s_saveUi19148.state,
                 s_saveUi19148.eventId,
                 -1,
                 0);
    return Finish19148(s_saveUi19148.gp720_result, out);
}

bool IsActive19148() {
    return s_saveUi19148.active;
}

const char* ActionKindName19148(PrStage1SaveUi19148ActionKind kind) {
    switch (kind) {
    case PrStage1SaveUi19148ActionKind::None:
        return "None";
    case PrStage1SaveUi19148ActionKind::Call80017E58InitEventArg:
        return "Call80017E58InitEventArg";
    case PrStage1SaveUi19148ActionKind::Call80017E6CSetEventResult:
        return "Call80017E6CSetEventResult";
    case PrStage1SaveUi19148ActionKind::Call80025C8CPlayInputSfx:
        return "Call80025C8CPlayInputSfx";
    case PrStage1SaveUi19148ActionKind::Call80018060InitNameInput:
        return "Call80018060InitNameInput";
    case PrStage1SaveUi19148ActionKind::Call80017B08EnumerateEntries:
        return "Call80017B08EnumerateEntries";
    case PrStage1SaveUi19148ActionKind::Call80017B18SnapshotDirectory:
        return "Call80017B18SnapshotDirectory";
    case PrStage1SaveUi19148ActionKind::Call80017B60FormatCard:
        return "Call80017B60FormatCard";
    case PrStage1SaveUi19148ActionKind::Call80017A10WriteSaveBlock:
        return "Call80017A10WriteSaveBlock";
    case PrStage1SaveUi19148ActionKind::Call80025C64CopySavePayload:
        return "Call80025C64CopySavePayload";
    case PrStage1SaveUi19148ActionKind::Call80019458ConsumePrefixState15:
        return "Call80019458ConsumePrefixState15";
    case PrStage1SaveUi19148ActionKind::Call80019458CommitResultState15:
        return "Call80019458CommitResultState15";
    case PrStage1SaveUi19148ActionKind::Call80015700BackupSaveStatusPrefix:
        return "Call80015700BackupSaveStatusPrefix";
    case PrStage1SaveUi19148ActionKind::Call80015744RestoreSaveStatusPrefix:
        return "Call80015744RestoreSaveStatusPrefix";
    case PrStage1SaveUi19148ActionKind::Call80015CC4InitSavePayload:
        return "Call80015CC4InitSavePayload";
    case PrStage1SaveUi19148ActionKind::Call8001615CMapSaveStage:
        return "Call8001615CMapSaveStage";
    case PrStage1SaveUi19148ActionKind::Call800161A8MapScene:
        return "Call800161A8MapScene";
    case PrStage1SaveUi19148ActionKind::Call800164B4LoadSavePayload:
        return "Call800164B4LoadSavePayload";
    case PrStage1SaveUi19148ActionKind::Call8001628CEnsureSaveProgress:
        return "Call8001628CEnsureSaveProgress";
    case PrStage1SaveUi19148ActionKind::Call8001635CUpdateSavePayload:
        return "Call8001635CUpdateSavePayload";
    case PrStage1SaveUi19148ActionKind::Call800169E0SyncSavedScore:
        return "Call800169E0SyncSavedScore";
    case PrStage1SaveUi19148ActionKind::Call80017594PollCardIo:
        return "Call80017594PollCardIo";
    case PrStage1SaveUi19148ActionKind::Call80016E18PollCardInfoLoad:
        return "Call80016E18PollCardInfoLoad";
    case PrStage1SaveUi19148ActionKind::Call80047EE4ResetHwCardEvents:
        return "Call80047EE4ResetHwCardEvents";
    case PrStage1SaveUi19148ActionKind::Call80017900ScanCardDirectory:
        return "Call80017900ScanCardDirectory";
    case PrStage1SaveUi19148ActionKind::Call80017454SubmitWrite:
        return "Call80017454SubmitWrite";
    case PrStage1SaveUi19148ActionKind::Call80016FC0ClearSwCardEvents:
        return "Call80016FC0ClearSwCardEvents";
    case PrStage1SaveUi19148ActionKind::Call80016EB8PollSwCardEvents:
        return "Call80016EB8PollSwCardEvents";
    case PrStage1SaveUi19148ActionKind::Call8001707CDrainHwCardEvents:
        return "Call8001707CDrainHwCardEvents";
    case PrStage1SaveUi19148ActionKind::Call80017008PollHwCardEvents:
        return "Call80017008PollHwCardEvents";
    case PrStage1SaveUi19148ActionKind::CardHalOpenCheck:
        return "CardHalOpenCheck";
    case PrStage1SaveUi19148ActionKind::CardHalOpenWrite:
        return "CardHalOpenWrite";
    case PrStage1SaveUi19148ActionKind::CardHalWrite:
        return "CardHalWrite";
    case PrStage1SaveUi19148ActionKind::CardHalClose:
        return "CardHalClose";
    case PrStage1SaveUi19148ActionKind::CardHalFormat:
        return "CardHalFormat";
    case PrStage1SaveUi19148ActionKind::CardHalInfo:
        return "CardHalInfo";
    case PrStage1SaveUi19148ActionKind::CardHalLoad:
        return "CardHalLoad";
    case PrStage1SaveUi19148ActionKind::Call80035560CardWait:
        return "Call80035560CardWait";
    case PrStage1SaveUi19148ActionKind::Call8001E750DrawEvent:
        return "Call8001E750DrawEvent";
    case PrStage1SaveUi19148ActionKind::Call80035560ResetInput:
        return "Call80035560ResetInput";
    case PrStage1SaveUi19148ActionKind::Call8001EA00EndFrame:
        return "Call8001EA00EndFrame";
    case PrStage1SaveUi19148ActionKind::Call800181D0ListInput:
        return "Call800181D0ListInput";
    case PrStage1SaveUi19148ActionKind::HelperGap:
        return "HelperGap";
    }
    return "Unknown";
}

const char* ActionHostBoundaryName19148(
    PrStage1SaveUi19148ActionHostBoundary boundary) {
    switch (boundary) {
    case PrStage1SaveUi19148ActionHostBoundary::DirectMemory:
        return "direct-memory";
    case PrStage1SaveUi19148ActionHostBoundary::HostHalBoundary:
        return "host-hal-boundary";
    case PrStage1SaveUi19148ActionHostBoundary::HelperGap:
        return "helper-gap";
    case PrStage1SaveUi19148ActionHostBoundary::UnsupportedHostAction:
        return "unsupported-host-action";
    }
    return "unknown";
}

const char* ActionGapReasonName19148(
    PrStage1SaveUi19148ActionGapReason reason) {
    switch (reason) {
    case PrStage1SaveUi19148ActionGapReason::None:
        return "";
    case PrStage1SaveUi19148ActionGapReason::CardEventHalNotPorted:
        return "card event HAL not ported";
    case PrStage1SaveUi19148ActionGapReason::
        CardFilesystemHostActionUnsupported:
        return "card/filesystem host action intentionally unsupported";
    case PrStage1SaveUi19148ActionGapReason::DirectHelperGap:
        return "direct helper gap";
    case PrStage1SaveUi19148ActionGapReason::UnsupportedHostAction:
        return "unsupported host action";
    }
    return "unknown";
}

PrStage1SaveUi19148HostActionRequestList BuildHostActionRequests19148(
    const PrStage1SaveUi19148ActionList& actions) {
    PrStage1SaveUi19148HostActionRequestList out{};
    out.sourceActionListTruncated = actions.truncated;
    for (uint32_t i = 0; i < actions.count; ++i) {
        const PrStage1SaveUi19148Action& action = actions.actions[i];
        if (action.kind ==
            PrStage1SaveUi19148ActionKind::Call80025C8CPlayInputSfx) {
            AppendHostActionRequest(
                out,
                action,
                PrStage1SaveUi19148HostActionRequestKind::PlayInputSfx);
            continue;
        }

        const PrStage1SaveUi19148ActionGapReason gapReason =
            GapReasonForHostAction19148(action);
        if (gapReason != PrStage1SaveUi19148ActionGapReason::None) {
            AppendHostActionRequest(
                out,
                action,
                PrStage1SaveUi19148HostActionRequestKind::GapReport,
                gapReason);
        }
    }
    return out;
}

PrStage1SaveUi19148LowerFeedbackRequestList
BuildLowerFeedbackRequests19148(
    const PrStage1SaveUi19148ActionList& actions) {
    PrStage1SaveUi19148LowerFeedbackRequestList out{};
    out.sourceActionListTruncated = actions.truncated;
    for (uint32_t i = 0; i < actions.count; ++i) {
        const PrStage1SaveUi19148Action& action = actions.actions[i];
        switch (action.kind) {
        case PrStage1SaveUi19148ActionKind::Call80017B60FormatCard:
            AppendLowerFeedbackRequest(
                out,
                action,
                PrStage1SaveUi19148LowerFeedbackRequestKind::Format80017B60);
            break;
        case PrStage1SaveUi19148ActionKind::Call80017A10WriteSaveBlock:
            AppendLowerFeedbackRequest(
                out,
                action,
                PrStage1SaveUi19148LowerFeedbackRequestKind::Write80017A10);
            break;
        case PrStage1SaveUi19148ActionKind::Call80017594PollCardIo:
            AppendLowerFeedbackRequest(
                out,
                action,
                PrStage1SaveUi19148LowerFeedbackRequestKind::CardIo80017594);
            break;
        default:
            break;
        }
    }
    return out;
}

PrStage1SaveUiWriteFeedbackCarrier80017A10 BuildWriteFeedback80017A10(
    uint32_t nameAddress,
    uint32_t dataAddress,
    int32_t blocks,
    const PrStage1SaveUiWriteFeedbackInput80017A10* feedback) {
    const bool oldHelperGap = s_saveUi19148.helperGap;
    s_saveUi19148.helperGap = false;
    PrStage1SaveUi19148TickResult out = MakeResult(s_saveUi19148.active);
    PrStage1SaveUiWriteFeedbackCarrier80017A10 result =
        AppendSub80017A10FeedbackCarrier(
            out, s_saveUi19148.state, nameAddress, dataAddress, blocks, feedback);
    result.actions = out.actions;
    result.helperGap = result.helperGap || s_saveUi19148.helperGap;
    s_saveUi19148.helperGap = oldHelperGap;
    return result;
}

PrStage1SaveUiFormatFeedbackCarrier80017B60 BuildFormatFeedback80017B60(
    const PrStage1SaveUiFormatFeedbackInput80017B60* feedback) {
    const bool oldHelperGap = s_saveUi19148.helperGap;
    s_saveUi19148.helperGap = false;
    PrStage1SaveUi19148TickResult out = MakeResult(s_saveUi19148.active);
    PrStage1SaveUiFormatFeedbackCarrier80017B60 result =
        AppendSub80017B60FeedbackCarrier(out, s_saveUi19148.state, feedback);
    result.actions = out.actions;
    result.helperGap = result.helperGap || s_saveUi19148.helperGap;
    s_saveUi19148.helperGap = oldHelperGap;
    return result;
}

PrStage1SaveUiCardIoCarrier80017594 BuildCardIoFeedback80017594(
    const PrStage1SaveUiCardIoState80017594& state,
    const PrStage1SaveUiCardIoFeedback80017594* feedback) {
    const bool oldHelperGap = s_saveUi19148.helperGap;
    s_saveUi19148.helperGap = false;
    PrStage1SaveUi19148TickResult out = MakeResult(s_saveUi19148.active);
    PrStage1SaveUiCardIoCarrier80017594 result =
        AppendSub80017594FeedbackCarrier(out, s_saveUi19148.state, state, feedback);
    result.actions = out.actions;
    result.helperGap = result.helperGap || s_saveUi19148.helperGap;
    s_saveUi19148.helperGap = oldHelperGap;
    return result;
}

void SetReplayMirrorSource(
    const PrStage1ScorerDirectReplayBufferState& replay) {
    if (!IsReplayMirrorSourceShapeKnown(replay)) {
        s_saveUiReplayMirrorSourceKnown = false;
        s_saveUiMemory.payloadBank.replayMirrorKnown8008EEF8 = false;
        s_saveUiMemory.payloadBank.replayMirrorProducerKnown8008EEF8 = false;
        s_saveUiMemory.payloadBank.replayMirrorProducerFunction = 0;
        s_saveUiMemory.payloadBank.replayMirrorByteCountKnown8008EEF8 = false;
        s_saveUiMemory.payloadBank.replayMirrorKnownByteCount8008EEF8 = 0;
        return;
    }
    s_saveUiReplayMirrorSourceKnown = true;
    s_saveUiReplayMirrorSource = replay;
    HydrateReplayMirrorSource();
}

PrStage1SavePayloadProducerResult Sub80015CC4() {
    s_saveUi19148.helperGap = false;
    PrStage1SaveUi19148TickResult out = MakeResult(s_saveUi19148.active);
    const int32_t result = Sub80015CC4Direct(out, s_saveUi19148.state);
    return MakeProducerResult(out, result, s_saveUiMemory.payloadBank.savePayloadBankKnown);
}

PrStage1SaveStatusBackupResult80015700 Sub80015700(uint32_t a1Address) {
    s_saveUi19148.helperGap = false;
    s_saveUiMemory.payloadBank.boundsFault = false;
    s_saveUiMemory.payloadBank.lastFaultAddress = 0;
    PrStage1SaveUi19148TickResult out = MakeResult(s_saveUi19148.active);
    const int32_t result =
        Sub80015700Direct(a1Address, out, s_saveUi19148.state);
    return MakeBackupResult80015700(out,
                                    kFn80015700,
                                    a1Address,
                                    result,
                                    result != 0,
                                    false);
}

PrStage1SaveStatusBackupResult80015700 Sub80015744(uint32_t a1Address) {
    s_saveUi19148.helperGap = false;
    s_saveUiMemory.payloadBank.boundsFault = false;
    s_saveUiMemory.payloadBank.lastFaultAddress = 0;
    PrStage1SaveUi19148TickResult out = MakeResult(s_saveUi19148.active);
    const int32_t result =
        Sub80015744Direct(a1Address, out, s_saveUi19148.state);
    return MakeBackupResult80015700(out,
                                    kFn80015744,
                                    a1Address,
                                    result,
                                    result != 0,
                                    result != 0);
}

PrStage1SavePayloadProducerResult Sub800164B4(uint32_t srcAddress) {
    s_saveUi19148.helperGap = false;
    PrStage1SaveUi19148TickResult out = MakeResult(s_saveUi19148.active);
    const bool copied = Sub800164B4Direct(srcAddress, out, s_saveUi19148.state);
    return MakeProducerResult(out, copied ? 1 : 0, copied);
}

PrStage1SavePayloadProducerResult CommitTypedPayload800164B4(
    uint32_t srcAddress,
    const uint8_t* source,
    std::size_t sourceBytes,
    const PrStagePayloadBankDirect::LoadSavePayloadAuthority800164B4&
        authority) {
    s_saveUi19148.helperGap = false;
    PrStage1SaveUi19148TickResult out = MakeResult(s_saveUi19148.active);
    AppendAction(out,
                 PrStage1SaveUi19148ActionKind::Call800164B4LoadSavePayload,
                 kFn800164B4,
                 s_saveUi19148.state,
                 s_saveUi19148.state,
                 static_cast<int32_t>(srcAddress),
                 static_cast<int32_t>(kAddrSaveDataBuffer80092F10),
                 kSaveUiSavePayloadBytes80019458);

    const PrStagePayloadBankDirect::LoadSavePayloadResult800164B4 direct =
        PrStagePayloadBankDirect::LoadSavePayload800164B4(
            s_saveUiMemory.payloadBank,
            srcAddress,
            source,
            sourceBytes,
            authority);
    s_saveUi19148.savePayloadSourceAddress = kAddrSaveDataBuffer80092F10;
    s_saveUi19148.savePayloadSourceKnown = direct.ok;
    if (!direct.ok) {
        AppendHelperGap(out,
                        kFn800164B4,
                        s_saveUi19148.state,
                        s_saveUi19148.state,
                        static_cast<int32_t>(srcAddress),
                        static_cast<int32_t>(kAddrSaveDataBuffer80092F10),
                        kSaveUiSavePayloadBytes80019458,
                        -1);
    }
    return MakeProducerResult(out, direct.ok ? 1 : 0, direct.ok);
}

PrStage1SavePayloadProducerResult Sub8001628C(int32_t a1) {
    s_saveUi19148.helperGap = false;
    PrStage1SaveUi19148TickResult out = MakeResult(s_saveUi19148.active);
    AppendAction(out,
                 PrStage1SaveUi19148ActionKind::
                     Call8001628CEnsureSaveProgress,
                 kFn8001628C,
                 s_saveUi19148.state,
                 s_saveUi19148.state,
                 a1);

    int32_t mappedIndex = 0;
    Sub8001615CDirect(a1, mappedIndex, out, s_saveUi19148.state);

    const PrStagePayloadBankDirect::EnsureProgressResult8001628C direct =
        PrStagePayloadBankDirect::EnsureProgress8001628C(
            s_saveUiMemory.payloadBank,
            a1);
    if (direct.mapped && !direct.statusBankKnown) {
        AppendStatusBankLiveGap(out, kFn8001628C, s_saveUi19148.state, a1);
    }
    s_saveUi19148.savePayloadSourceAddress = kAddrSaveDataBuffer80092F10;
    s_saveUi19148.savePayloadSourceKnown = direct.ok && direct.payloadKnown;
    return MakeProducerResult(out, direct.result, direct.ok);
}

PrStage1SavePayloadProducerResult Sub8001635C(int32_t a1,
                                              int32_t a2,
                                              int32_t a3,
                                              int32_t a4) {
    s_saveUi19148.helperGap = false;
    PrStage1SaveUi19148TickResult out = MakeResult(s_saveUi19148.active);
    const int32_t result =
        Sub8001635CDirect(a1, a2, a3, a4, out, s_saveUi19148.state);
    return MakeProducerResult(out, result, s_saveUiMemory.payloadBank.savePayloadBankKnown);
}

PrStage1SavePayloadProducerResult SeedColdBootStatusPrefix800154F4() {
    s_saveUi19148.helperGap = false;
    ResetDirectMemory();
    MarkInitialSavePayloadBssKnown80092F10();
    PrStage1SaveUi19148TickResult out = MakeResult(s_saveUi19148.active);
    const int32_t result = Sub8001635CDirect(1, 1, 1, 0, out, s_saveUi19148.state);
    return MakeProducerResult(out, result, s_saveUiMemory.payloadBank.savePayloadBankKnown);
}

PrStageClearAllStatusQueryResult Sub800161F4() {
    s_saveUi19148.helperGap = false;
    PrStage1SaveUi19148TickResult out = MakeResult(s_saveUi19148.active);
    const PrStagePayloadBankDirect::AllStatusQueryResult800161F4 direct =
        PrStagePayloadBankDirect::QueryAllStatuses800161F4(
            s_saveUiMemory.payloadBank);
    PrStageClearAllStatusQueryResult result{};
    result.psxFunction = kFn800161F4;
    result.statusBaseAddress80092F1D = kAddrSavePayloadProgressBase80092F1D;
    result.statusBankKnown = direct.statusBankKnown;
    result.statusBytesKnown80092F1D = direct.statusBytesKnown80092F1D;
    for (uint32_t i = 0; i < 6u; ++i) {
        result.byte80092F1D[i] = direct.byte80092F1D[i];
    }
    result.result = direct.result;

    if (!direct.statusBankKnown) {
        AppendStatusBankLiveGap(out, kFn800161F4, s_saveUi19148.state);
        result.helperGap = s_saveUi19148.helperGap;
        return result;
    }

    if (!direct.ok) {
        AppendStatusBankLiveGap(out, kFn800161F4, s_saveUi19148.state);
        result.helperGap = s_saveUi19148.helperGap;
        return result;
    }

    result.ok = true;
    result.helperGap = s_saveUi19148.helperGap;
    return result;
}

PrSavedScoreSync169E0Result Sub800169E0(int32_t word800916D0,
                                        int32_t word800916E2) {
    s_saveUi19148.helperGap = false;
    PrStage1SaveUi19148TickResult out = MakeResult(s_saveUi19148.active);
    const PrStagePayloadBankDirect::SavedScoreSyncResult800169E0 direct =
        PrStagePayloadBankDirect::SyncSavedScore800169E0(
            s_saveUiMemory.payloadBank,
            word800916D0,
            word800916E2);
    PrSavedScoreSync169E0Result result{};
    result.psxFunction = kFn800169E0;
    result.word800916D0 = word800916D0;
    result.word800916E2 = word800916E2;
    result.statusBankKnown = direct.statusBankKnown;
    result.applied = direct.applied;
    result.mapped = direct.mapped;
    result.slotIndex = direct.slotIndex;
    result.ctxScoreDword = direct.ctxScoreDword;
    result.word80091816 = direct.word80091816;
    auto finish = [&]() {
        result.actions = out.actions;
        return result;
    };

    AppendAction(out,
                 PrStage1SaveUi19148ActionKind::Call800169E0SyncSavedScore,
                 kFn800169E0,
                 s_saveUi19148.state,
                 s_saveUi19148.state,
                 word800916D0,
                 word800916E2);

    if (word800916D0 != 2) {
        result.ok = direct.ok;
        result.helperGap = s_saveUi19148.helperGap;
        return finish();
    }

    int32_t mappedIndex = 0;
    Sub8001615CDirect(word800916E2, mappedIndex, out, s_saveUi19148.state);
    if (!result.mapped) {
        result.helperGap = s_saveUi19148.helperGap;
        return finish();
    }

    if (!direct.ok) {
        if (direct.scoreDwordReadAttempted) {
            AppendStatusBankLiveGap(out,
                                    kFn800169E0,
                                    s_saveUi19148.state,
                                    word800916E2,
                                    direct.slotIndex);
        } else {
            AppendStatusBankLiveGap(
                out, kFn800169E0, s_saveUi19148.state, word800916E2);
        }
        result.helperGap = s_saveUi19148.helperGap;
        return finish();
    }

    result.ok = true;
    result.helperGap = s_saveUi19148.helperGap;
    return finish();
}

PrStageClearStatusQueryResult Sub800166AC(int32_t a1) {
    s_saveUi19148.helperGap = false;
    PrStage1SaveUi19148TickResult out = MakeResult(s_saveUi19148.active);
    const PrStagePayloadBankDirect::StatusQueryResult800166AC direct =
        PrStagePayloadBankDirect::QueryStatus800166AC(
            s_saveUiMemory.payloadBank,
            a1);
    PrStageClearStatusQueryResult result{};
    result.psxFunction = kFn800166AC;
    result.sceneId = a1;
    result.statusBankKnown = direct.statusBankKnown;
    result.mapped = direct.mapped;
    result.slotIndex = direct.slotIndex;
    result.status = direct.status;

    int32_t mappedIndex = 0;
    Sub8001615CDirect(a1, mappedIndex, out, s_saveUi19148.state);
    if (!result.mapped) {
        result.helperGap = s_saveUi19148.helperGap;
        return result;
    }

    if (!direct.ok) {
        AppendStatusBankLiveGap(out, kFn800166AC, s_saveUi19148.state, a1);
        result.helperGap = s_saveUi19148.helperGap;
        return result;
    }

    result.ok = true;
    result.helperGap = s_saveUi19148.helperGap;
    return result;
}

PrStageClearStatusQueryResult Sub800167A8(int32_t a1, int32_t a2) {
    s_saveUi19148.helperGap = false;
    PrStage1SaveUi19148TickResult out = MakeResult(s_saveUi19148.active);
    const PrStagePayloadBankDirect::StatusWriteResult800167A8 direct =
        PrStagePayloadBankDirect::WriteStatus800167A8(
            s_saveUiMemory.payloadBank,
            a1,
            a2);
    PrStageClearStatusQueryResult result{};
    result.psxFunction = kFn800167A8;
    result.sceneId = a1;
    result.statusBankKnown = direct.statusBankKnown;
    result.status = direct.status;
    result.mapped = direct.mapped;
    result.slotIndex = direct.slotIndex;

    int32_t mappedIndex = 0;
    Sub8001615CDirect(a1, mappedIndex, out, s_saveUi19148.state);
    if (!result.mapped) {
        result.helperGap = s_saveUi19148.helperGap;
        return result;
    }

    if (!direct.ok) {
        AppendStatusBankLiveGap(out, kFn800167A8, s_saveUi19148.state, a1, a2);
        result.helperGap = s_saveUi19148.helperGap;
        return result;
    }

    result.ok = true;
    result.helperGap = s_saveUi19148.helperGap;
    return result;
}

PrStage1SaveStatusPrefix80092F10 GetSaveStatusPrefix80092F10() {
    const PrStagePayloadBankDirect::SaveStatusPrefixSnapshot80092F10 direct =
        PrStagePayloadBankDirect::SnapshotSaveStatusPrefix80092F10(
            s_saveUiMemory.payloadBank);
    PrStage1SaveStatusPrefix80092F10 out{};
    out.known = direct.known;
    out.statusBankKnown80092F1D = direct.statusBankKnown80092F1D;
    out.helperGap = s_saveUi19148.helperGap;
    out.psxAddress = direct.psxAddress;
    out.byteCount = direct.byteCount;
    out.lastWriterFunction = direct.lastWriterFunction;
    out.lastFaultAddress = direct.lastFaultAddress;
    out.wrote80015CC4 = direct.wrote80015CC4;
    out.wrote800164B4 = direct.wrote800164B4;
    out.wrote8001635C = direct.wrote8001635C;
    out.wrote8001628C = direct.wrote8001628C;
    out.wrote800167A8 = direct.wrote800167A8;
    out.wrote80015744 = direct.wrote80015744;
    if (out.known) {
        std::memcpy(out.bytes, direct.bytes, sizeof(out.bytes));
    }
    return out;
}

PrStageClearStatusBankSnapshot GetStageClearStatusBankSnapshot() {
    const PrStagePayloadBankDirect::StatusBankSnapshot80092F10 direct =
        PrStagePayloadBankDirect::SnapshotStatusBank80092F10(
            s_saveUiMemory.payloadBank);
    PrStageClearStatusBankSnapshot snapshot{};
    snapshot.statusBytesKnown80092F1D = direct.statusBytesKnown80092F1D;
    for (uint32_t i = 0; i < 6u; ++i) {
        snapshot.byte80092F1D[i] = direct.byte80092F1D[i];
    }
    snapshot.scoreDwordsKnown80092F24 = direct.scoreDwordsKnown80092F24;
    for (uint32_t i = 0; i < 6u; ++i) {
        snapshot.dword80092F24[i] = direct.dword80092F24[i];
    }
    snapshot.lastSavedSlotKnown80092F3C = direct.lastSavedSlotKnown80092F3C;
    snapshot.dword80092F3C = direct.dword80092F3C;
    snapshot.allClearKnown80092F44 = direct.allClearKnown80092F44;
    snapshot.dword80092F44 = direct.dword80092F44;
    return snapshot;
}

}  // namespace PrStage1SaveUiDirect
