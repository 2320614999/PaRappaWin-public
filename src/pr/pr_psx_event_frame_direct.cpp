#include "pr_psx_event_frame_direct.h"

#include "pr_stage1_movie_text_direct.h"

#include <array>
#include <cstring>

namespace PrPsxEventFrameDirect {
namespace {

constexpr uint32_t kBoxFillPacketPayloadWords8003EE84 = 4u;
constexpr uint32_t kBoxFillPacketTotalWords8003EE84 =
    kBoxFillPacketPayloadWords8003EE84 + 1u;
constexpr uint32_t kBoxFillPacketAdvanceBytes8003EE84 =
    kBoxFillPacketTotalWords8003EE84 * 4u;
constexpr uint32_t kEventBackdropTileTemplateSlot8001D74C = 0x8004E7D0u;
constexpr uint32_t kEventBackdropSubmitCount8001D74C = 84u;
constexpr uint32_t kStageSelectTitleBgPos80053248 = 0x80053248u;
constexpr uint32_t kStageSelectTitleBgTemplateTable80053244 = 0x80053244u;
constexpr uint32_t kStageSelectTitleTemplate80051AA0 = 0x80051AA0u;
constexpr uint32_t kStageSelectBonusOnTemplate800519F0 = 0x800519F0u;
constexpr uint32_t kStageSelectBonusOffTemplate800519E0 = 0x800519E0u;
constexpr uint32_t kStageSelectBonusEnabledTemplate80051A10 = 0x80051A10u;
constexpr uint32_t kStageSelectBonusLabelTemplate80051A00 = 0x80051A00u;
constexpr uint32_t kStageSelectTopTextPos8005308C = 0x8005308Cu;
constexpr uint32_t kStageSelectTopTextTemplateTable80053050 = 0x80053050u;
constexpr uint32_t kStageSelectPointX80053104 = 0x80053104u;
constexpr uint32_t kStageSelectPointY80053106 = 0x80053106u;
constexpr uint32_t kStageSelectSliceTemplate80051BF0 = 0x80051BF0u;
constexpr uint32_t kStageSelectSliceUOffset800531D0 = 0x800531D0u;
constexpr uint32_t kStageSelectSliceWidth800531D2 = 0x800531D2u;
constexpr uint32_t kStageSelectBoxPos8005317C = 0x8005317Cu;
constexpr uint32_t kStageSelectBoxTemplateTable800531C4 = 0x800531C4u;
constexpr uint32_t kStageSelectNamePos80053194 = 0x80053194u;
constexpr uint32_t kStageSelectNameTemplateTable80053214 = 0x80053214u;
constexpr uint32_t kStageSelectBadgePos800531AC = 0x800531ACu;
constexpr uint32_t kStageSelectBadgeATemplateTable800531F4 = 0x800531F4u;
constexpr uint32_t kStageSelectBadgeBTemplateTable800531E8 = 0x800531E8u;
constexpr uint32_t kStageSelectExitLabelPos8005300C = 0x8005300Cu;
constexpr uint32_t kStageSelectExitLabelNormalTable80053000 = 0x80053000u;
constexpr uint32_t kStageSelectExitLabelOffTable80053004 = 0x80053004u;
constexpr uint32_t kStageSelectExitLabelOnTable80053008 = 0x80053008u;
constexpr uint32_t kStageSelectExitBarPos80052FFC = 0x80052FFCu;
constexpr uint32_t kStageSelectExitBarNormalTemplate800509A0 = 0x800509A0u;
constexpr uint32_t kStageSelectExitBarOffTemplate800509B0 = 0x800509B0u;
constexpr uint32_t kStageSelectExitBarOnTemplate800509C0 = 0x800509C0u;
constexpr uint32_t kStageSelectExitFrameNormalGpOffset80020568 = 0xD4u;
constexpr uint32_t kStageSelectExitFrameOnGpOffset80020568 = 0xD0u;
constexpr uint32_t kStageSelectGpBase80020568 = 0x8006EA40u;
constexpr uint32_t kStageSelectExitFrameNormalSlot80020568 =
    kStageSelectGpBase80020568 + kStageSelectExitFrameNormalGpOffset80020568;
constexpr uint32_t kStageSelectExitFrameOnSlot80020568 =
    kStageSelectGpBase80020568 + kStageSelectExitFrameOnGpOffset80020568;
constexpr uint32_t kStageSelectExitFrameNormalTemplate80050AC0 = 0x80050AC0u;
constexpr uint32_t kStageSelectExitFrameOnTemplate80050AD0 = 0x80050AD0u;
constexpr uint32_t kSaveUiType1HeaderPos80053A0C = 0x80053A0Cu;
constexpr uint32_t kSaveUiType1HeaderTemplateTable80053A08 = 0x80053A08u;
constexpr uint32_t kSaveUiType2HeaderPos80053994 = 0x80053994u;
constexpr uint32_t kSaveUiType2HeaderTemplateTable80053990 = 0x80053990u;
constexpr uint32_t kSaveUiType3HeaderPos800539BC = 0x800539BCu;
constexpr uint32_t kSaveUiType3HeaderTemplateTable800539B8 = 0x800539B8u;
constexpr uint32_t kSaveUiType4HeaderPos80053A34 = 0x80053A34u;
constexpr uint32_t kSaveUiType4HeaderTemplateTable80053A30 = 0x80053A30u;
constexpr uint32_t kSaveUiType5HeaderPos80053A5C = 0x80053A5Cu;
constexpr uint32_t kSaveUiType5HeaderTemplateTable80053A58 = 0x80053A58u;
constexpr uint32_t kSaveUiType6HeaderPos80053A84 = 0x80053A84u;
constexpr uint32_t kSaveUiType6HeaderTemplateTable80053A80 = 0x80053A80u;
constexpr uint32_t kSaveUiType7HeaderPos800539E4 = 0x800539E4u;
constexpr uint32_t kSaveUiType7HeaderTemplateTable800539E0 = 0x800539E0u;
constexpr uint32_t kSaveUiChoice0Pos80053AB4 = 0x80053AB4u;
constexpr uint32_t kSaveUiChoice0TemplateATable80053AAC = 0x80053AACu;
constexpr uint32_t kSaveUiChoice0TemplateBTable80053AB0 = 0x80053AB0u;
constexpr uint32_t kSaveUiChoice1Pos80053B04 = 0x80053B04u;
constexpr uint32_t kSaveUiChoice1TemplateATable80053AFC = 0x80053AFCu;
constexpr uint32_t kSaveUiChoice1TemplateBTable80053B00 = 0x80053B00u;
constexpr uint32_t kSaveUiType4HeaderTemplate80052350 = 0x80052350u;
constexpr uint32_t kSaveUiType4PromptTemplateOn800526A0 = 0x800526A0u;
constexpr uint32_t kSaveUiType4PromptTemplateOff800526B0 = 0x800526B0u;
constexpr uint32_t kSaveUiChoice0TemplateA80052590 = 0x80052590u;
constexpr uint32_t kSaveUiChoice0TemplateB800525A0 = 0x800525A0u;
constexpr uint32_t kSaveUiChoice1TemplateA800526C0 = 0x800526C0u;
constexpr uint32_t kSaveUiChoice1TemplateB800526D0 = 0x800526D0u;
constexpr uint32_t kSaveUiTitleTemplate80052320 = 0x80052320u;

bool g_stage1Event4Gp38CSeedValid8006EDCC = false;
uint32_t g_stage1Event4Gp38CSeed8006EDCC = 0u;

struct StageSelectPoint80020568 {
    int16_t x;
    int16_t y;
};

struct StageSelectDotArgs80020568 {
    int16_t uOffset;
    int16_t width;
};

struct PromptSpriteTemplate800203D4 {
    uint32_t attr = 0;
    uint16_t texX = 0;
    uint16_t texY = 0;
    uint16_t w = 0;
    uint16_t h = 0;
    uint16_t clutX = 0;
    uint16_t clutY = 0;
};

struct BoxFillLocal8001B6C4 {
    uint32_t attr_00 = 0;
    int16_t x_04 = 0;
    int16_t y_06 = 0;
    int16_t w_08 = 0;
    int16_t h_0A = 0;
    uint8_t r_0C = 0;
    uint8_t g_0D = 0;
    uint8_t b_0E = 0;
};

constexpr StageSelectPoint80020568
    kStageSelectDotPos80053104[5][kStageSelectEntryCount80020568] = {
        {{ 92, 122}, {183, 106}, {271,  90},
         { 90, 199}, {184, 182}, {271, 166}},
        {{ 45, 122}, {135, 106}, {223,  90},
         { 42, 199}, {135, 182}, {223, 166}},
        {{ 92, 122}, {183, 106}, {271,  90},
         { 90, 199}, {184, 182}, {271, 166}},
        {{ 92, 122}, {183, 106}, {271,  90},
         { 90, 199}, {184, 182}, {271, 166}},
        {{ 92, 122}, {183, 106}, {271,  90},
         { 90, 199}, {184, 182}, {271, 166}},
};

constexpr StageSelectDotArgs80020568
    kStageSelectDotArgs800531D0[kStageSelectEntryCount80020568] = {
        { 0, 4},
        { 4, 8},
        {12, 8},
        {20, 8},
        {28, 8},
        {36, 8},
};

constexpr StageSelectSpriteTemplate8001B4E0
    kStageSelectDotTemplate80051BF0{
        true,
        kStageSelectSliceTemplate80051BF0,
        0x50000040u,
        606u,
        208u,
        44u,
        10u,
        1008u,
        31u};

StageSelectSpriteTemplate8001B4E0 MakeStageSelectSpriteTemplate8001B25C(
    uint32_t addr,
    uint32_t attr,
    uint16_t texX,
    uint16_t texY,
    uint16_t width,
    uint16_t height,
    uint16_t clutX,
    uint16_t clutY) {
    StageSelectSpriteTemplate8001B4E0 out{};
    out.known = true;
    out.addr = addr;
    out.attr_00 = attr;
    out.texX_04 = texX;
    out.texY_06 = texY;
    out.width_08 = width;
    out.height_0A = height;
    out.clutX_0C = clutX;
    out.clutY_0E = clutY;
    return out;
}

StageSelectSpriteTemplate8001B4E0 ResolveStageSelectTemplate8001B25C(
    uint32_t addr) {
    switch (addr) {
    case 0x8004E7D0u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 0x0332u, 0x0100u, 0x0028u, 0x0028u,
            0x0100u, 0x01EAu);
    case 0x8004E7E0u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 0x0300u, 0x0100u, 0x0014u, 0x0064u,
            0x0100u, 0x01EBu);
    case 0x8004E7F0u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 0x0305u, 0x0100u, 0x0014u, 0x0064u,
            0x0100u, 0x01ECu);
    case 0x8004E800u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 0x030Au, 0x0100u, 0x0014u, 0x0064u,
            0x0100u, 0x01EDu);
    case 0x8004E810u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 0x030Fu, 0x0100u, 0x0014u, 0x0064u,
            0x0100u, 0x01EEu);
    case 0x8004E820u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 0x0314u, 0x0100u, 0x0078u, 0x0014u,
            0x0100u, 0x01EFu);
    case 0x8004E830u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 0x0314u, 0x0114u, 0x0078u, 0x0014u,
            0x0100u, 0x01F0u);
    case 0x8004E840u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 0x0314u, 0x0128u, 0x0078u, 0x0014u,
            0x0100u, 0x01F1u);
    case 0x8004E850u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 0x0314u, 0x013Cu, 0x0078u, 0x0014u,
            0x0100u, 0x01F2u);
    case 0x8004E900u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x10000040u, 0x0380u, 0x0163u, 0x0014u, 0x0014u,
            0x0110u, 0x01EEu);
    case 0x8004E910u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x10000040u, 0x0385u, 0x0163u, 0x0014u, 0x0014u,
            0x0110u, 0x01EFu);
    case 0x8004E920u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x10000040u, 0x038Au, 0x0163u, 0x0014u, 0x0014u,
            0x0110u, 0x01F0u);
    case 0x8004E930u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x10000040u, 0x038Fu, 0x0163u, 0x0014u, 0x0014u,
            0x0110u, 0x01F1u);
    case 0x800508D0u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x40000040u, 0x03FDu, 0x0199u, 0x0008u, 0x0008u,
            0x0120u, 0x01EEu);
    case 0x800508E0u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x40000040u, 0x03FDu, 0x01A1u, 0x0008u, 0x0008u,
            0x0120u, 0x01EEu);
    case 0x800508F0u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x40000040u, 0x03FDu, 0x0189u, 0x0008u, 0x0008u,
            0x0120u, 0x01EEu);
    case 0x80050900u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x40000040u, 0x03FDu, 0x0191u, 0x0008u, 0x0008u,
            0x0120u, 0x01EEu);
    case 0x800509A0u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 320u, 0u, 48u, 17u, 960u, 2u);
    case 0x800509B0u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 320u, 0u, 48u, 17u, 960u, 3u);
    case 0x800509C0u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 320u, 0u, 48u, 17u, 960u, 4u);
    case 0x800509D0u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 320u, 256u, 28u, 11u, 960u, 5u);
    case 0x800509E0u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 320u, 256u, 28u, 11u, 960u, 6u);
    case 0x800509F0u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 320u, 256u, 28u, 11u, 960u, 7u);
    case 0x80050A00u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 327u, 256u, 32u, 8u, 960u, 5u);
    case 0x80050A10u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 327u, 256u, 32u, 8u, 960u, 6u);
    case 0x80050A20u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 327u, 256u, 32u, 8u, 960u, 7u);
    case 0x80050A30u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 335u, 256u, 32u, 8u, 960u, 5u);
    case 0x80050A40u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 335u, 256u, 32u, 8u, 960u, 6u);
    case 0x80050A50u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 335u, 256u, 32u, 8u, 960u, 7u);
    case 0x80050A60u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 343u, 256u, 28u, 12u, 960u, 5u);
    case 0x80050A70u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 343u, 256u, 28u, 12u, 960u, 6u);
    case 0x80050A80u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 343u, 256u, 28u, 12u, 960u, 7u);
    case 0x80050A90u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 350u, 256u, 32u, 10u, 960u, 5u);
    case 0x80050AA0u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 350u, 256u, 32u, 10u, 960u, 6u);
    case 0x80050AB0u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 350u, 256u, 32u, 10u, 960u, 7u);
    case 0x80050AC0u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 384u, 0u, 64u, 37u, 960u, 0u);
    case 0x80050AD0u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 384u, 0u, 64u, 37u, 960u, 1u);
    case 0x800518D0u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 576u, 120u, 68u, 72u, 1008u, 0u);
    case 0x800518E0u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 576u, 120u, 68u, 72u, 1008u, 1u);
    case 0x800518F0u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 576u, 120u, 68u, 72u, 1008u, 2u);
    case 0x80051900u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 593u, 120u, 44u, 44u, 1008u, 3u);
    case 0x80051910u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 604u, 120u, 44u, 44u, 1008u, 4u);
    case 0x80051920u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 615u, 120u, 44u, 44u, 1008u, 5u);
    case 0x80051930u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 593u, 164u, 44u, 44u, 1008u, 6u);
    case 0x80051940u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 604u, 164u, 44u, 44u, 1008u, 7u);
    case 0x80051950u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 615u, 164u, 44u, 44u, 1008u, 8u);
    case 0x80051960u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 626u, 0u, 36u, 36u, 1008u, 9u);
    case 0x80051970u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 626u, 36u, 36u, 36u, 1008u, 10u);
    case 0x80051980u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 626u, 72u, 36u, 36u, 1008u, 11u);
    case 0x80051990u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 626u, 108u, 36u, 36u, 1008u, 12u);
    case 0x800519A0u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 626u, 0u, 36u, 36u, 1008u, 13u);
    case 0x800519B0u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 626u, 36u, 36u, 36u, 1008u, 14u);
    case 0x800519C0u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 626u, 72u, 36u, 36u, 1008u, 15u);
    case 0x800519D0u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 626u, 108u, 36u, 36u, 1008u, 16u);
    case 0x800519E0u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 576u, 0u, 192u, 40u, 1008u, 17u);
    case 0x800519F0u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 576u, 40u, 192u, 40u, 1008u, 18u);
    case 0x80051A00u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x40000040u, 576u, 80u, 192u, 40u, 1008u, 19u);
    case 0x80051A10u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 576u, 0u, 192u, 40u, 1008u, 20u);
    case 0x80051A20u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 593u, 120u, 44u, 44u, 1008u, 21u);
    case 0x80051A30u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 604u, 120u, 44u, 44u, 1008u, 22u);
    case 0x80051A40u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 615u, 120u, 44u, 44u, 1008u, 23u);
    case 0x80051A50u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 593u, 164u, 44u, 44u, 1008u, 24u);
    case 0x80051A60u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 604u, 164u, 44u, 44u, 1008u, 25u);
    case 0x80051A70u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 615u, 164u, 44u, 44u, 1008u, 26u);
    case 0x80051A80u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 626u, 0u, 36u, 36u, 1008u, 27u);
    case 0x80051A90u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 626u, 36u, 36u, 36u, 1008u, 28u);
    case 0x80051AA0u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 576u, 208u, 120u, 20u, 1008u, 29u);
    case 0x80051AB0u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 448u, 256u, 96u, 13u, 1008u, 30u);
    case 0x80051AC0u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 448u, 270u, 112u, 11u, 1008u, 30u);
    case 0x80051AD0u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 472u, 256u, 100u, 14u, 1008u, 30u);
    case 0x80051AE0u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 476u, 270u, 112u, 12u, 1008u, 30u);
    case 0x80051AF0u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 448u, 281u, 112u, 14u, 1008u, 30u);
    case 0x80051B00u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 448u, 295u, 36u, 10u, 1008u, 31u);
    case 0x80051B10u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 448u, 295u, 36u, 10u, 1008u, 35u);
    case 0x80051B20u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 448u, 295u, 36u, 10u, 1008u, 36u);
    case 0x80051B30u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 468u, 295u, 40u, 10u, 1008u, 34u);
    case 0x80051B40u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 468u, 295u, 40u, 10u, 1008u, 35u);
    case 0x80051B50u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 468u, 295u, 40u, 10u, 1008u, 36u);
    case 0x80051B60u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 457u, 295u, 44u, 10u, 1008u, 34u);
    case 0x80051B70u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 457u, 295u, 44u, 10u, 1008u, 35u);
    case 0x80051B80u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 457u, 295u, 44u, 10u, 1008u, 36u);
    case 0x80051B90u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 478u, 295u, 44u, 10u, 1008u, 34u);
    case 0x80051BA0u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 478u, 295u, 44u, 10u, 1008u, 35u);
    case 0x80051BB0u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 478u, 295u, 44u, 10u, 1008u, 36u);
    case 0x80051BC0u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 489u, 295u, 36u, 10u, 1008u, 34u);
    case 0x80051BD0u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 489u, 295u, 36u, 10u, 1008u, 35u);
    case 0x80051BE0u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 489u, 295u, 36u, 10u, 1008u, 36u);
    case 0x80051BF0u:
        return kStageSelectDotTemplate80051BF0;
    case 0x80052320u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 0x02C0u, 0x0088u, 0x00ACu, 0x0054u,
            0x03F0u, 0x00D6u);
    case 0x80052330u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x51000040u, 0x02C0u, 0x0000u, 0x005Cu, 0x0088u,
            0x0300u, 0x00FDu);
    case 0x80052340u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x51000040u, 0x0300u, 0x0000u, 0x0060u, 0x0088u,
            0x0300u, 0x00FEu);
    case 0x80052350u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x51000040u, 0x0340u, 0x0000u, 0x0060u, 0x0086u,
            0x0300u, 0x00FFu);
    case 0x80052360u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 0x0200u, 0x0100u, 0x0094u, 0x0031u,
            0x03F0u, 0x0032u);
    case 0x80052370u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 0x0240u, 0x01CDu, 0x0090u, 0x0023u,
            0x03F0u, 0x0032u);
    case 0x80052380u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 0x0300u, 0x00E9u, 0x0090u, 0x000Eu,
            0x03F0u, 0x0032u);
    case 0x80052390u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 0x0225u, 0x0100u, 0x0068u, 0x0023u,
            0x03F0u, 0x0032u);
    case 0x800523A0u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 0x0221u, 0x01B7u, 0x0028u, 0x000Eu,
            0x03F0u, 0x0032u);
    case 0x800523B0u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 0x0240u, 0x011Fu, 0x0090u, 0x0035u,
            0x03F0u, 0x0032u);
    case 0x800523C0u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 0x0265u, 0x0100u, 0x0064u, 0x0031u,
            0x03F0u, 0x0032u);
    case 0x800523D0u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 0x0200u, 0x0163u, 0x008Cu, 0x0031u,
            0x03F0u, 0x0032u);
    case 0x800523E0u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 0x0221u, 0x0194u, 0x0078u, 0x0023u,
            0x03F0u, 0x0032u);
    case 0x800523F0u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 0x035Fu, 0x00C0u, 0x0048u, 0x0010u,
            0x03F0u, 0x0032u);
    case 0x80052400u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 0x0340u, 0x00C0u, 0x007Cu, 0x0028u,
            0x03F0u, 0x0032u);
    case 0x80052410u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 0x0225u, 0x0146u, 0x005Cu, 0x0012u,
            0x03F0u, 0x0032u);
    case 0x80052420u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 0x0240u, 0x0188u, 0x008Cu, 0x0020u,
            0x03F0u, 0x0032u);
    case 0x80052430u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 0x0380u, 0x0000u, 0x008Cu, 0x0023u,
            0x03F0u, 0x0032u);
    case 0x80052440u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 0x0200u, 0x0131u, 0x0094u, 0x0032u,
            0x03F0u, 0x0032u);
    case 0x80052450u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 0x0340u, 0x00E8u, 0x0094u, 0x0011u,
            0x03F0u, 0x0032u);
    case 0x80052460u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 0x0340u, 0x0094u, 0x0094u, 0x000Du,
            0x03F0u, 0x0032u);
    case 0x80052470u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 0x0322u, 0x0088u, 0x0070u, 0x0028u,
            0x03F0u, 0x0032u);
    case 0x80052480u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 0x0225u, 0x0175u, 0x0048u, 0x0012u,
            0x03F0u, 0x0032u);
    case 0x80052490u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 0x0300u, 0x00BBu, 0x0094u, 0x002Eu,
            0x03F0u, 0x0032u);
    case 0x800524A0u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 0x0380u, 0x0023u, 0x0088u, 0x0038u,
            0x03F0u, 0x0032u);
    case 0x800524B0u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 0x0200u, 0x0194u, 0x0084u, 0x0036u,
            0x03F0u, 0x0032u);
    case 0x800524C0u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 0x0225u, 0x0123u, 0x0060u, 0x0023u,
            0x03F0u, 0x0032u);
    case 0x800524D0u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 0x0380u, 0x005Bu, 0x0090u, 0x0029u,
            0x03F0u, 0x0032u);
    case 0x800524E0u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 0x0200u, 0x01CAu, 0x008Cu, 0x0028u,
            0x03F0u, 0x0032u);
    case 0x800524F0u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 0x0225u, 0x0158u, 0x005Cu, 0x000Du,
            0x03F0u, 0x0032u);
    case 0x80052500u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 0x0300u, 0x0088u, 0x0088u, 0x0033u,
            0x03F0u, 0x0032u);
    case 0x80052510u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 0x0380u, 0x0084u, 0x008Cu, 0x0036u,
            0x03F0u, 0x0032u);
    case 0x80052520u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 0x0380u, 0x00BAu, 0x0090u, 0x0032u,
            0x03F0u, 0x0032u);
    case 0x80052530u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 0x0240u, 0x0100u, 0x0094u, 0x001Fu,
            0x03F0u, 0x0032u);
    case 0x80052540u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 0x0340u, 0x0086u, 0x0090u, 0x000Eu,
            0x03F0u, 0x0032u);
    case 0x80052550u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 0x0240u, 0x01A8u, 0x0088u, 0x0025u,
            0x03F0u, 0x0032u);
    case 0x80052560u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 0x0225u, 0x0165u, 0x004Cu, 0x0010u,
            0x03F0u, 0x0032u);
    case 0x80052570u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 0x0240u, 0x0154u, 0x0090u, 0x0034u,
            0x03F0u, 0x0032u);
    case 0x80052580u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 0x0340u, 0x00A1u, 0x008Cu, 0x001Fu,
            0x03F0u, 0x0032u);
    case 0x80052590u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 0x02EEu, 0x003Fu, 0x0030u, 0x0015u,
            0x03F0u, 0x00C9u);
    case 0x800525A0u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 0x02EEu, 0x003Fu, 0x0030u, 0x0015u,
            0x03F0u, 0x00CAu);
    case 0x800525C0u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 0x0330u, 0x0000u, 0x001Cu, 0x000Eu,
            0x03F0u, 0x00CCu);
    case 0x800525D0u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 0x0330u, 0x0000u, 0x001Cu, 0x000Eu,
            0x03F0u, 0x00CDu);
    case 0x800525F0u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 0x0330u, 0x001Bu, 0x001Cu, 0x000Du,
            0x03F0u, 0x00CCu);
    case 0x80052600u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 0x0330u, 0x001Bu, 0x001Cu, 0x000Du,
            0x03F0u, 0x00CDu);
    case 0x80052620u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 0x0330u, 0x000Eu, 0x0014u, 0x000Du,
            0x03F0u, 0x00CCu);
    case 0x80052630u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 0x0330u, 0x000Eu, 0x0014u, 0x000Du,
            0x03F0u, 0x00CDu);
    case 0x80052650u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 0x0330u, 0x0028u, 0x0014u, 0x0012u,
            0x03F0u, 0x00CCu);
    case 0x80052660u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 0x0330u, 0x0028u, 0x0014u, 0x0012u,
            0x03F0u, 0x00CDu);
    case 0x80052680u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 0x0330u, 0x003Au, 0x0014u, 0x0012u,
            0x03F0u, 0x00CCu);
    case 0x80052690u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 0x0330u, 0x003Au, 0x0014u, 0x0012u,
            0x03F0u, 0x00CDu);
    case 0x800526A0u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 0x02EEu, 0x0000u, 0x0044u, 0x003Fu,
            0x03F0u, 0x00CEu);
    case 0x800526B0u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 0x02EEu, 0x0000u, 0x0044u, 0x003Fu,
            0x03F0u, 0x00CFu);
    case 0x800526C0u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 0x02EEu, 0x0054u, 0x0030u, 0x0015u,
            0x03F0u, 0x00D1u);
    case 0x800526D0u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 0x02EEu, 0x0054u, 0x0030u, 0x0015u,
            0x03F0u, 0x00D2u);
    case 0x800526F0u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 0x0370u, 0x0000u, 0x0014u, 0x000Eu,
            0x03F0u, 0x00D4u);
    case 0x80052700u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 0x0370u, 0x0000u, 0x0014u, 0x000Eu,
            0x03F0u, 0x00D5u);
    case 0x80052720u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 0x0370u, 0x001Cu, 0x001Cu, 0x000Eu,
            0x03F0u, 0x00D4u);
    case 0x80052730u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 0x0370u, 0x001Cu, 0x001Cu, 0x000Eu,
            0x03F0u, 0x00D5u);
    case 0x80052740u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 0x0370u, 0x000Eu, 0x0020u, 0x000Eu,
            0x03F0u, 0x00D3u);
    case 0x80052750u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 0x0370u, 0x000Eu, 0x0020u, 0x000Eu,
            0x03F0u, 0x00D4u);
    case 0x80052760u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 0x0370u, 0x000Eu, 0x0020u, 0x000Eu,
            0x03F0u, 0x00D5u);
    case 0x80052780u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 0x0370u, 0x0000u, 0x0014u, 0x000Eu,
            0x03F0u, 0x00D4u);
    case 0x80052790u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 0x0370u, 0x0000u, 0x0014u, 0x000Eu,
            0x03F0u, 0x00D5u);
    case 0x800527B0u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 0x0370u, 0x0000u, 0x0014u, 0x000Eu,
            0x03F0u, 0x00D4u);
    case 0x800527C0u:
        return MakeStageSelectSpriteTemplate8001B25C(
            addr, 0x50000040u, 0x0370u, 0x0000u, 0x0014u, 0x000Eu,
            0x03F0u, 0x00D5u);
    default:
        return StageSelectSpriteTemplate8001B4E0{};
    }
}

struct TextHexDigitTableCarrier80043A14 {
    bool tableKnown = false;
    uint32_t pointerSlotAddr = kTextHexDigitTablePointerSlot8005D6E8;
    uint32_t tableAddr = 0;
    uint8_t tableBytes[kTextHexDigitTableByteCount80043A14]{};
    const char* source =
        "unresolved: 80043C64 loads off_8005D6E8 pointer slot; pointed "
        "16-byte digit table needs IDA/data export";
};

constexpr char kStageClearWordFormat8006EC14[] = "%d";
constexpr char kVisibleCompatibilityHexDigits80043A14[] =
    "0123456789abcdef";

constexpr TextHexDigitTableCarrier80043A14
    kHexDigitTable8005D6E8Carrier80043A14{
        true,
        kTextHexDigitTablePointerSlot8005D6E8,
        kTextHexDigitTableAddr8001229C,
        {'0', '1', '2', '3', '4', '5', '6', '7',
         '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'},
        "IDA export 2026-05-10: 8005D6E8 .word 8001229C; "
        "8001229C bytes 0123456789ABCDEF"};

uint32_t WorkListAddr80087288(uint32_t slot) {
    return kWorkListBase80087288 + kWorkListStride80087288 * slot;
}

bool ResolveStageSelectPositionPair80020568(uint32_t addr,
                                            int16_t* outX,
                                            int16_t* outY) {
    if (outX == nullptr || outY == nullptr) {
        return false;
    }

    auto resolveFiveEntryTable =
        [&](uint32_t base,
            uint32_t stride,
            const StageSelectPoint80020568 (&positions)[5]) -> bool {
        if (addr < base || addr >= base + 5u * stride ||
            ((addr - base) % stride) != 0u) {
            return false;
        }
        const uint32_t index = (addr - base) / stride;
        *outX = positions[index].x;
        *outY = positions[index].y;
        return true;
    };

    static constexpr StageSelectPoint80020568 kSaveUiType1HeaderPos[5] = {
        {152, 58}, {148, 55}, {144, 55}, {135, 55}, {137, 57}};
    if (resolveFiveEntryTable(kSaveUiType1HeaderPos80053A0C,
                              8u,
                              kSaveUiType1HeaderPos)) {
        return true;
    }

    static constexpr StageSelectPoint80020568 kSaveUiType2HeaderPos[5] = {
        {131, 52}, {131, 51}, {135, 51}, {139, 49}, {133, 53}};
    if (resolveFiveEntryTable(kSaveUiType2HeaderPos80053994,
                              8u,
                              kSaveUiType2HeaderPos)) {
        return true;
    }

    static constexpr StageSelectPoint80020568 kSaveUiType3HeaderPos[5] = {
        {134, 59}, {131, 68}, {142, 60}, {157, 59}, {131, 60}};
    if (resolveFiveEntryTable(kSaveUiType3HeaderPos800539BC,
                              8u,
                              kSaveUiType3HeaderPos)) {
        return true;
    }

    static constexpr StageSelectPoint80020568 kSaveUiType5HeaderPos[5] = {
        {134, 51}, {131, 53}, {135, 58}, {137, 50}, {133, 51}};
    if (resolveFiveEntryTable(kSaveUiType5HeaderPos80053A5C,
                              8u,
                              kSaveUiType5HeaderPos)) {
        return true;
    }

    static constexpr StageSelectPoint80020568 kSaveUiType6HeaderPos[5] = {
        {154, 52}, {138, 49}, {135, 57}, {135, 50}, {137, 59}};
    if (resolveFiveEntryTable(kSaveUiType6HeaderPos80053A84,
                              8u,
                              kSaveUiType6HeaderPos)) {
        return true;
    }

    static constexpr StageSelectPoint80020568 kSaveUiType7HeaderPos[5] = {
        {134, 69}, {131, 69}, {171, 66}, {133, 57}, {133, 70}};
    if (resolveFiveEntryTable(kSaveUiType7HeaderPos800539E4,
                              8u,
                              kSaveUiType7HeaderPos)) {
        return true;
    }

    if (addr >= kStageSelectTitleBgPos80053248 &&
        addr < kStageSelectTitleBgPos80053248 + 5u * 8u &&
        ((addr - kStageSelectTitleBgPos80053248) % 8u) == 0u) {
        static constexpr StageSelectPoint80020568 kTitleTextPos[5] = {
            {44, 36}, {43, 35}, {35, 37}, {35, 37}, {35, 35}};
        const uint32_t lang = (addr - kStageSelectTitleBgPos80053248) / 8u;
        *outX = kTitleTextPos[lang].x;
        *outY = kTitleTextPos[lang].y;
        return true;
    }

    if (addr >= kStageSelectTopTextPos8005308C &&
        addr < kStageSelectTopTextPos8005308C + 5u * 24u &&
        ((addr - kStageSelectTopTextPos8005308C) % 4u) == 0u) {
        static constexpr StageSelectPoint80020568 kStageBoxPos[5][6] = {
            {{ 48, 122}, {141, 106}, {229,  90},
             { 48, 199}, {141, 182}, {229, 166}},
            {{ 50, 122}, {143, 106}, {231,  90},
             { 50, 199}, {143, 182}, {231, 166}},
            {{ 47, 122}, {140, 106}, {228,  90},
             { 47, 199}, {140, 182}, {228, 166}},
            {{ 44, 122}, {137, 106}, {225,  90},
             { 44, 199}, {137, 182}, {225, 166}},
            {{ 48, 122}, {141, 106}, {229,  90},
             { 48, 199}, {141, 182}, {229, 166}},
        };
        const uint32_t rel = addr - kStageSelectTopTextPos8005308C;
        const uint32_t lang = rel / 24u;
        const uint32_t index = (rel % 24u) / 4u;
        *outX = kStageBoxPos[lang][index].x;
        *outY = kStageBoxPos[lang][index].y;
        return true;
    }

    if (addr >= kStageSelectBoxPos8005317C &&
        addr < kStageSelectBoxPos8005317C + 6u * 4u &&
        ((addr - kStageSelectBoxPos8005317C) % 4u) == 0u) {
        static constexpr StageSelectPoint80020568 kStageTopPos[6] = {
            {37, 66}, {130, 50}, {218, 34},
            {37, 143}, {130, 126}, {218, 110}};
        const uint32_t index = (addr - kStageSelectBoxPos8005317C) / 4u;
        *outX = kStageTopPos[index].x;
        *outY = kStageTopPos[index].y;
        return true;
    }

    if (addr >= kStageSelectNamePos80053194 &&
        addr < kStageSelectNamePos80053194 + 6u * 4u &&
        ((addr - kStageSelectNamePos80053194) % 4u) == 0u) {
        static constexpr StageSelectPoint80020568 kStageNamePos[6] = {
            {45, 75}, {138, 59}, {226, 43},
            {45, 152}, {138, 135}, {226, 119}};
        const uint32_t index = (addr - kStageSelectNamePos80053194) / 4u;
        *outX = kStageNamePos[index].x;
        *outY = kStageNamePos[index].y;
        return true;
    }

    if (addr >= kStageSelectBadgePos800531AC &&
        addr < kStageSelectBadgePos800531AC + 6u * 4u &&
        ((addr - kStageSelectBadgePos800531AC) % 4u) == 0u) {
        static constexpr StageSelectPoint80020568 kStageBadgePos[6] = {
            {78, 64}, {171, 48}, {259, 32},
            {78, 141}, {171, 124}, {259, 108}};
        const uint32_t index = (addr - kStageSelectBadgePos800531AC) / 4u;
        *outX = kStageBadgePos[index].x;
        *outY = kStageBadgePos[index].y;
        return true;
    }

    if (addr >= kStageSelectExitLabelPos8005300C &&
        addr < kStageSelectExitLabelPos8005300C + 5u * 16u &&
        ((addr - kStageSelectExitLabelPos8005300C) % 16u) == 0u) {
        static constexpr StageSelectPoint80020568 kExitLabelPos[5] = {
            {242, 191}, {238, 193}, {238, 193}, {241, 190}, {239, 191}};
        const uint32_t lang =
            (addr - kStageSelectExitLabelPos8005300C) / 16u;
        *outX = kExitLabelPos[lang].x;
        *outY = kExitLabelPos[lang].y;
        return true;
    }

    if (addr == kStageSelectExitBarPos80052FFC) {
        *outX = 238;
        *outY = 188;
        return true;
    }

    if (addr >= kSaveUiType4HeaderPos80053A34 &&
        addr < kSaveUiType4HeaderPos80053A34 + 5u * 8u &&
        ((addr - kSaveUiType4HeaderPos80053A34) % 8u) == 0u) {
        static constexpr StageSelectPoint80020568 kSaveUiHeaderPos[5] = {
            {185, 71}, {170, 69}, {160, 69}, {162, 71}, {167, 71}};
        const uint32_t lang =
            (addr - kSaveUiType4HeaderPos80053A34) / 8u;
        *outX = kSaveUiHeaderPos[lang].x;
        *outY = kSaveUiHeaderPos[lang].y;
        return true;
    }

    if (addr >= kSaveUiChoice0Pos80053AB4 &&
        addr < kSaveUiChoice0Pos80053AB4 + 5u * 16u &&
        ((addr - kSaveUiChoice0Pos80053AB4) % 16u) == 0u) {
        static constexpr StageSelectPoint80020568 kSaveUiChoice0Pos[5] = {
            {237, 162}, {242, 163}, {237, 163}, {241, 160}, {241, 160}};
        const uint32_t lang =
            (addr - kSaveUiChoice0Pos80053AB4) / 16u;
        *outX = kSaveUiChoice0Pos[lang].x;
        *outY = kSaveUiChoice0Pos[lang].y;
        return true;
    }

    if (addr >= kSaveUiChoice1Pos80053B04 &&
        addr < kSaveUiChoice1Pos80053B04 + 5u * 16u &&
        ((addr - kSaveUiChoice1Pos80053B04) % 16u) == 0u) {
        static constexpr StageSelectPoint80020568 kSaveUiChoice1Pos[5] = {
            {241, 185}, {234, 185}, {236, 185}, {241, 185}, {241, 185}};
        const uint32_t lang =
            (addr - kSaveUiChoice1Pos80053B04) / 16u;
        *outX = kSaveUiChoice1Pos[lang].x;
        *outY = kSaveUiChoice1Pos[lang].y;
        return true;
    }

    return false;
}

bool ResolveStageSelectTemplatePointer80020568(uint32_t addr,
                                               uint32_t* outTemplate) {
    if (outTemplate == nullptr) {
        return false;
    }

    auto resolveFiveEntryTable =
        [&](uint32_t base,
            uint32_t stride,
            const uint32_t (&templates)[5]) -> bool {
        if (addr < base || addr >= base + 5u * stride ||
            ((addr - base) % stride) != 0u) {
            return false;
        }
        const uint32_t index = (addr - base) / stride;
        *outTemplate = templates[index];
        return true;
    };

    static constexpr uint32_t kSaveUiType1HeaderTpl[5] = {
        0x80052390u, 0x80052470u, 0x80052400u,
        0x800524E0u, 0x80052550u};
    if (resolveFiveEntryTable(kSaveUiType1HeaderTemplateTable80053A08,
                              8u,
                              kSaveUiType1HeaderTpl)) {
        return true;
    }

    static constexpr uint32_t kSaveUiType2HeaderTpl[5] = {
        0x80052360u, 0x80052440u, 0x800523D0u,
        0x800524B0u, 0x80052520u};
    if (resolveFiveEntryTable(kSaveUiType2HeaderTemplateTable80053990,
                              8u,
                              kSaveUiType2HeaderTpl)) {
        return true;
    }

    static constexpr uint32_t kSaveUiType3HeaderTpl[5] = {
        0x80052370u, 0x80052450u, 0x800523E0u,
        0x800524C0u, 0x80052530u};
    if (resolveFiveEntryTable(kSaveUiType3HeaderTemplateTable800539B8,
                              8u,
                              kSaveUiType3HeaderTpl)) {
        return true;
    }

    static constexpr uint32_t kSaveUiType5HeaderTpl[5] = {
        0x800523B0u, 0x80052490u, 0x80052420u,
        0x80052500u, 0x80052570u};
    if (resolveFiveEntryTable(kSaveUiType5HeaderTemplateTable80053A58,
                              8u,
                              kSaveUiType5HeaderTpl)) {
        return true;
    }

    static constexpr uint32_t kSaveUiType6HeaderTpl[5] = {
        0x800523C0u, 0x800524A0u, 0x80052430u,
        0x80052510u, 0x80052580u};
    if (resolveFiveEntryTable(kSaveUiType6HeaderTemplateTable80053A80,
                              8u,
                              kSaveUiType6HeaderTpl)) {
        return true;
    }

    static constexpr uint32_t kSaveUiType7HeaderTpl[5] = {
        0x80052380u, 0x80052460u, 0x800523F0u,
        0x800524D0u, 0x80052540u};
    if (resolveFiveEntryTable(kSaveUiType7HeaderTemplateTable800539E0,
                              8u,
                              kSaveUiType7HeaderTpl)) {
        return true;
    }

    if (addr >= kStageSelectTitleBgTemplateTable80053244 &&
        addr < kStageSelectTitleBgTemplateTable80053244 + 5u * 8u &&
        ((addr - kStageSelectTitleBgTemplateTable80053244) % 8u) == 0u) {
        static constexpr uint32_t kTitleTextTpl[5] = {
            0x80051AB0u, 0x80051AD0u, 0x80051AC0u,
            0x80051AE0u, 0x80051AF0u};
        const uint32_t lang =
            (addr - kStageSelectTitleBgTemplateTable80053244) / 8u;
        *outTemplate = kTitleTextTpl[lang];
        return true;
    }

    if (addr >= kStageSelectTopTextTemplateTable80053050 &&
        addr < kStageSelectTopTextTemplateTable80053050 + 15u * 4u &&
        ((addr - kStageSelectTopTextTemplateTable80053050) % 4u) == 0u) {
        static constexpr uint32_t kStageBoxTpl[15] = {
            0x80051B20u, 0x80051B00u, 0x80051B10u,
            0x80051B80u, 0x80051B60u, 0x80051B70u,
            0x80051B50u, 0x80051B30u, 0x80051B40u,
            0x80051BB0u, 0x80051B90u, 0x80051BA0u,
            0x80051BE0u, 0x80051BC0u, 0x80051BD0u};
        const uint32_t index =
            (addr - kStageSelectTopTextTemplateTable80053050) / 4u;
        *outTemplate = kStageBoxTpl[index];
        return true;
    }

    if (addr >= kStageSelectBoxTemplateTable800531C4 &&
        addr < kStageSelectBoxTemplateTable800531C4 + 3u * 4u &&
        ((addr - kStageSelectBoxTemplateTable800531C4) % 4u) == 0u) {
        static constexpr uint32_t kStageTopTpl[3] = {
            0x800518F0u, 0x800518D0u, 0x800518E0u};
        const uint32_t index =
            (addr - kStageSelectBoxTemplateTable800531C4) / 4u;
        *outTemplate = kStageTopTpl[index];
        return true;
    }

    if (addr >= kStageSelectNameTemplateTable80053214 &&
        addr < kStageSelectNameTemplateTable80053214 + 12u * 4u &&
        ((addr - kStageSelectNameTemplateTable80053214) % 4u) == 0u) {
        static constexpr uint32_t kStageNameTpl[12] = {
            0x80051900u, 0x80051A20u, 0x80051910u, 0x80051A30u,
            0x80051920u, 0x80051A40u, 0x80051930u, 0x80051A50u,
            0x80051940u, 0x80051A60u, 0x80051950u, 0x80051A70u};
        const uint32_t index =
            (addr - kStageSelectNameTemplateTable80053214) / 4u;
        *outTemplate = kStageNameTpl[index];
        return true;
    }

    if (addr >= kStageSelectBadgeATemplateTable800531F4 &&
        addr < kStageSelectBadgeATemplateTable800531F4 + 8u * 4u &&
        ((addr - kStageSelectBadgeATemplateTable800531F4) % 4u) == 0u) {
        static constexpr uint32_t kStageBadgeATpl[8] = {
            0x80051A90u, 0x800519B0u, 0x80051970u, 0x800519B0u,
            0x80051980u, 0x800519C0u, 0x80051990u, 0x800519D0u};
        const uint32_t index =
            (addr - kStageSelectBadgeATemplateTable800531F4) / 4u;
        *outTemplate = kStageBadgeATpl[index];
        return true;
    }

    if (addr >= kStageSelectBadgeBTemplateTable800531E8 &&
        addr < kStageSelectBadgeBTemplateTable800531E8 + 3u * 4u &&
        ((addr - kStageSelectBadgeBTemplateTable800531E8) % 4u) == 0u) {
        static constexpr uint32_t kStageBadgeBTpl[3] = {
            0x80051A80u, 0x80051960u, 0x800519A0u};
        const uint32_t index =
            (addr - kStageSelectBadgeBTemplateTable800531E8) / 4u;
        *outTemplate = kStageBadgeBTpl[index];
        return true;
    }

    if (addr >= kStageSelectExitLabelNormalTable80053000 &&
        addr < kStageSelectExitLabelNormalTable80053000 + 5u * 16u &&
        ((addr - kStageSelectExitLabelNormalTable80053000) % 16u) == 0u) {
        static constexpr uint32_t kExitLabelNormalTpl[5] = {
            0x800509D0u, 0x80050A30u, 0x80050A00u,
            0x80050A60u, 0x80050A90u};
        const uint32_t lang =
            (addr - kStageSelectExitLabelNormalTable80053000) / 16u;
        *outTemplate = kExitLabelNormalTpl[lang];
        return true;
    }

    if (addr >= kStageSelectExitLabelOffTable80053004 &&
        addr < kStageSelectExitLabelOffTable80053004 + 5u * 16u &&
        ((addr - kStageSelectExitLabelOffTable80053004) % 16u) == 0u) {
        static constexpr uint32_t kExitLabelOffTpl[5] = {
            0x800509E0u, 0x80050A40u, 0x80050A10u,
            0x80050A70u, 0x80050AA0u};
        const uint32_t lang =
            (addr - kStageSelectExitLabelOffTable80053004) / 16u;
        *outTemplate = kExitLabelOffTpl[lang];
        return true;
    }

    if (addr >= kStageSelectExitLabelOnTable80053008 &&
        addr < kStageSelectExitLabelOnTable80053008 + 5u * 16u &&
        ((addr - kStageSelectExitLabelOnTable80053008) % 16u) == 0u) {
        static constexpr uint32_t kExitLabelOnTpl[5] = {
            0x800509F0u, 0x80050A50u, 0x80050A20u,
            0x80050A80u, 0x80050AB0u};
        const uint32_t lang =
            (addr - kStageSelectExitLabelOnTable80053008) / 16u;
        *outTemplate = kExitLabelOnTpl[lang];
        return true;
    }

    if (addr >= kSaveUiType4HeaderTemplateTable80053A30 &&
        addr < kSaveUiType4HeaderTemplateTable80053A30 + 5u * 8u &&
        ((addr - kSaveUiType4HeaderTemplateTable80053A30) % 8u) == 0u) {
        static constexpr uint32_t kSaveUiHeaderTpl[5] = {
            0x800523A0u, 0x80052480u, 0x80052410u,
            0x800524F0u, 0x80052560u};
        const uint32_t lang =
            (addr - kSaveUiType4HeaderTemplateTable80053A30) / 8u;
        *outTemplate = kSaveUiHeaderTpl[lang];
        return true;
    }

    if (addr >= kSaveUiChoice0TemplateATable80053AAC &&
        addr < kSaveUiChoice0TemplateATable80053AAC + 5u * 16u &&
        ((addr - kSaveUiChoice0TemplateATable80053AAC) % 16u) == 0u) {
        static constexpr uint32_t kSaveUiChoice0TplA[5] = {
            0x800525C0u, 0x80052620u, 0x800525F0u,
            0x80052650u, 0x80052680u};
        const uint32_t lang =
            (addr - kSaveUiChoice0TemplateATable80053AAC) / 16u;
        *outTemplate = kSaveUiChoice0TplA[lang];
        return true;
    }

    if (addr >= kSaveUiChoice0TemplateBTable80053AB0 &&
        addr < kSaveUiChoice0TemplateBTable80053AB0 + 5u * 16u &&
        ((addr - kSaveUiChoice0TemplateBTable80053AB0) % 16u) == 0u) {
        static constexpr uint32_t kSaveUiChoice0TplB[5] = {
            0x800525D0u, 0x80052630u, 0x80052600u,
            0x80052660u, 0x80052690u};
        const uint32_t lang =
            (addr - kSaveUiChoice0TemplateBTable80053AB0) / 16u;
        *outTemplate = kSaveUiChoice0TplB[lang];
        return true;
    }

    if (addr >= kSaveUiChoice1TemplateATable80053AFC &&
        addr < kSaveUiChoice1TemplateATable80053AFC + 5u * 16u &&
        ((addr - kSaveUiChoice1TemplateATable80053AFC) % 16u) == 0u) {
        static constexpr uint32_t kSaveUiChoice1TplA[5] = {
            0x800526F0u, 0x80052750u, 0x80052720u,
            0x80052780u, 0x800527B0u};
        const uint32_t lang =
            (addr - kSaveUiChoice1TemplateATable80053AFC) / 16u;
        *outTemplate = kSaveUiChoice1TplA[lang];
        return true;
    }

    if (addr >= kSaveUiChoice1TemplateBTable80053B00 &&
        addr < kSaveUiChoice1TemplateBTable80053B00 + 5u * 16u &&
        ((addr - kSaveUiChoice1TemplateBTable80053B00) % 16u) == 0u) {
        static constexpr uint32_t kSaveUiChoice1TplB[5] = {
            0x80052700u, 0x80052760u, 0x80052730u,
            0x80052790u, 0x800527C0u};
        const uint32_t lang =
            (addr - kSaveUiChoice1TemplateBTable80053B00) / 16u;
        *outTemplate = kSaveUiChoice1TplB[lang];
        return true;
    }

    return false;
}

bool ResolveStageSelectGpTemplateSlot80020568(uint32_t gpOffset,
                                              uint32_t* outSlotAddr,
                                              uint32_t* outTemplate) {
    if (outSlotAddr == nullptr || outTemplate == nullptr) {
        return false;
    }

    switch (gpOffset) {
    case kStageSelectExitFrameOnGpOffset80020568:
        *outSlotAddr = kStageSelectExitFrameOnSlot80020568;
        *outTemplate = kStageSelectExitFrameOnTemplate80050AD0;
        return true;
    case kStageSelectExitFrameNormalGpOffset80020568:
        *outSlotAddr = kStageSelectExitFrameNormalSlot80020568;
        *outTemplate = kStageSelectExitFrameNormalTemplate80050AC0;
        return true;
    default:
        return false;
    }
}

uint32_t MakePacketLinkTag8003EF5C(uint32_t oldOtSlotValue,
                                   uint32_t payloadWordCount) {
    return (oldOtSlotValue & kPsxPacketAddrMask8003EF5C) |
           ((payloadWordCount & 0xFFu) << 24);
}

uint32_t ResolveOtSlotAddr8003EF5C(
    const PrPsxGraphOwnerDirect::PsxGraphWorkList80040CC8& work,
    uint16_t priority) {
    return work.headAddr_04 + static_cast<uint32_t>(priority) * 4u -
           work.x_08 * 4u;
}

uint32_t ResolveClearedOtSlotValue8003EF5C(uint32_t slotAddr,
                                           uint32_t workHeadAddr) {
    return slotAddr == workHeadAddr ? kPsxPacketAddrMask8003EF5C
                                    : ((slotAddr - 4u) &
                                       kPsxPacketAddrMask8003EF5C);
}

uint32_t ResolveWorkOtSlotValue8003EF5C(
    const PrPsxGraphOwnerDirect::PsxGraphWorkList80040CC8& work,
    uint32_t slotAddr) {
    if (work.otSlotMirrorKnown) {
        for (const PrPsxFastSpriteSubmitDirect::RuntimeOtSlotValue8003FA20&
                 slot : work.otSlotMirror) {
            if (slot.valid && slot.addr == slotAddr) {
                return slot.value;
            }
        }
    }
    return ResolveClearedOtSlotValue8003EF5C(slotAddr, work.headAddr_04);
}

bool SetWorkOtSlotValue8003EF5C(
    PrPsxGraphOwnerDirect::PsxGraphWorkList80040CC8& work,
    uint32_t slotAddr,
    uint32_t value) {
    work.otSlotMirrorKnown = true;
    for (PrPsxFastSpriteSubmitDirect::RuntimeOtSlotValue8003FA20& slot :
         work.otSlotMirror) {
        if (slot.valid && slot.addr == slotAddr) {
            slot.value = value;
            return true;
        }
    }
    for (PrPsxFastSpriteSubmitDirect::RuntimeOtSlotValue8003FA20& slot :
         work.otSlotMirror) {
        if (!slot.valid) {
            slot.valid = true;
            slot.addr = slotAddr;
            slot.value = value;
            return true;
        }
    }
    return false;
}

bool AppendBoxFillPacketWrite8003EE84(
    PrPsxGraphOwnerDirect::PsxGraphWorkList80040CC8& work,
    uint32_t packetAddr,
    uint32_t word0LinkTag,
    uint32_t word1DrawMode,
    uint32_t word2ColorCode,
    uint32_t word3Xy,
    uint32_t word4Wh,
    uint16_t priority) {
    work.packetWriteMirrorKnown = true;
    PrPsxFastSpriteSubmitDirect::RuntimePacketWrite8003FA20 write{};
    write.valid = true;
    write.addr = packetAddr;
    write.wordCount = kBoxFillPacketTotalWords8003EE84;
    write.words = {{word0LinkTag,
                    word1DrawMode,
                    word2ColorCode,
                    word3Xy,
                    word4Wh,
                    0u}};
    write.wordKnown = {{true, true, true, true, true, false}};
    write.provenance.active = true;
    write.provenance.sourceFunction = kFn8003EE84_GsSortBoxFill;
    write.provenance.helper = kFn8003EF5C_LinkPacket;
    write.provenance.priority = priority;
    for (PrPsxFastSpriteSubmitDirect::RuntimePacketWrite8003FA20& slot :
         work.packetWriteMirror) {
        if (!slot.valid) {
            slot = write;
            return true;
        }
    }
    return false;
}

BoxFillPacketCommand8003EE84 BuildBoxFillPacketCommand8003EE84(
    const PrPsxFastSpriteSubmitDirect::RuntimePacketWrite8003FA20& write,
    uint32_t packetMirrorOrder) {
    BoxFillPacketCommand8003EE84 out{};
    out.valid = write.valid && write.wordCount == kBoxFillPacketTotalWords8003EE84 &&
                write.provenance.sourceFunction == kFn8003EE84_GsSortBoxFill;
    if (!out.valid) {
        return out;
    }

    out.packetAddr = write.addr;
    out.psxCallOrder =
        packetMirrorOrder != 0u ? packetMirrorOrder
                                : write.provenance.psxCallOrder;
    out.priority = write.provenance.priority;
    out.wordsKnown = true;
    for (uint32_t i = 0; i < kBoxFillPacketTotalWords8003EE84; ++i) {
        out.wordsKnown = out.wordsKnown && write.wordKnown[i];
    }
    out.word0LinkTag = write.words[0];
    out.word1DrawMode = write.words[1];
    out.word2ColorCode = write.words[2];
    out.word3Xy = write.words[3];
    out.word4Wh = write.words[4];
    if (out.wordsKnown) {
        out.r = static_cast<uint8_t>(out.word2ColorCode & 0xFFu);
        out.g = static_cast<uint8_t>((out.word2ColorCode >> 8) & 0xFFu);
        out.b = static_cast<uint8_t>((out.word2ColorCode >> 16) & 0xFFu);
        out.commandCode =
            static_cast<uint8_t>((out.word2ColorCode >> 24) & 0xFFu);
        out.x = static_cast<int16_t>(out.word3Xy & 0xFFFFu);
        out.y = static_cast<int16_t>((out.word3Xy >> 16) & 0xFFFFu);
        out.w = static_cast<uint16_t>(out.word4Wh & 0xFFFFu);
        out.h = static_cast<uint16_t>((out.word4Wh >> 16) & 0xFFFFu);
    }
    return out;
}

void LatchStage1Event4MoveImageBoxFill8001B120(
    EventFrameState8001E750& state,
    const PrPsxGraphOwnerDirect::PsxGraphPageWorkList8001E374& pageWork) {
    if (!pageWork.work.packetWriteMirrorKnown) {
        return;
    }

    uint32_t packetMirrorOrder = 1u;
    for (const PrPsxFastSpriteSubmitDirect::RuntimePacketWrite8003FA20& write :
         pageWork.work.packetWriteMirror) {
        const BoxFillPacketCommand8003EE84 command =
            BuildBoxFillPacketCommand8003EE84(write, packetMirrorOrder++);
        if (!command.valid || !command.wordsKnown) {
            continue;
        }
        EventFrameMoveImageBoxFill8001B120& copy =
            state.stage1Event4MoveImageBoxFill8001B120;
        copy.valid = true;
        copy.r = command.r;
        copy.g = command.g;
        copy.b = command.b;
        copy.commandCode = command.commandCode;
        copy.x = 0;
        copy.y = 0;
        copy.w = 320;
        copy.h = 240;
    }
}

uint32_t Pack16(int16_t low, int16_t high) {
    return static_cast<uint32_t>(static_cast<uint16_t>(low)) |
           (static_cast<uint32_t>(static_cast<uint16_t>(high)) << 16);
}

uint16_t ComputeSub80043DF4Word(uint16_t texX,
                                uint16_t texY,
                                uint16_t width,
                                uint16_t glyphIndex) {
    const uint16_t v8 =
        static_cast<uint16_t>(4u * texX + glyphIndex * width);
    return static_cast<uint16_t>(((v8 & 0xFF00u) >> 2) |
                                 (texY & 0xFF00u));
}

uint16_t ComputeSub80043DF4WordFromOffset(uint16_t texX,
                                          uint16_t texY,
                                          int16_t uOffset) {
    const uint16_t v8 =
        static_cast<uint16_t>(4u * texX +
                              static_cast<uint16_t>(uOffset));
    return static_cast<uint16_t>(((v8 & 0xFF00u) >> 2) |
                                 (texY & 0xFF00u));
}

uint32_t MakeBoxFillDrawModeWord8003EE84(uint32_t attr) {
    return ((attr >> 17) & 0x180u) | ((attr >> 23) & 0x60u) |
           0xE1000200u;
}

uint32_t MakeBoxFillColorCodeWord8003EE84(uint32_t attr,
                                          uint8_t r,
                                          uint8_t g,
                                          uint8_t b) {
    const uint8_t code =
        static_cast<uint8_t>(((attr >> 29) & 0x02u) | 0x60u);
    return static_cast<uint32_t>(r) |
           (static_cast<uint32_t>(g) << 8) |
           (static_cast<uint32_t>(b) << 16) |
           (static_cast<uint32_t>(code) << 24);
}

void PsxCall8003EE84_GsSortBoxFill(
    PrPsxGraphOwnerDirect::PsxGraphState& graph,
    const BoxFillLocal8001B6C4& box,
    PrPsxGraphOwnerDirect::PsxGraphWorkList80040CC8& work,
    uint16_t priority) {
    if (static_cast<int32_t>(box.attr_00) < 0) {
        return;
    }

    const uint32_t otSlotAddr = ResolveOtSlotAddr8003EF5C(work, priority);
    const uint32_t oldOtSlotValue =
        ResolveWorkOtSlotValue8003EF5C(work, otSlotAddr);
    const uint32_t packetAddr = graph.dword_800901C8;
    const uint32_t word0LinkTag =
        MakePacketLinkTag8003EF5C(oldOtSlotValue,
                                  kBoxFillPacketPayloadWords8003EE84);
    const uint32_t word1DrawMode =
        MakeBoxFillDrawModeWord8003EE84(box.attr_00);
    const uint32_t word2ColorCode =
        MakeBoxFillColorCodeWord8003EE84(box.attr_00,
                                         box.r_0C,
                                         box.g_0D,
                                         box.b_0E);
    const uint32_t word3Xy =
        Pack16(static_cast<int16_t>(box.x_04 + graph.drawOffset.word_800917AA),
               static_cast<int16_t>(box.y_06 + graph.drawOffset.word_800917AC));
    const uint32_t word4Wh = Pack16(box.w_08, box.h_0A);
    (void)AppendBoxFillPacketWrite8003EE84(work,
                                           packetAddr,
                                           word0LinkTag,
                                           word1DrawMode,
                                           word2ColorCode,
                                           word3Xy,
                                           word4Wh,
                                           priority);
    (void)SetWorkOtSlotValue8003EF5C(
        work,
        otSlotAddr,
        packetAddr & kPsxPacketAddrMask8003EF5C);
    graph.dword_800901C8 = packetAddr + kBoxFillPacketAdvanceBytes8003EE84;
}

void PsxCall8001B6C4_BoxFill(
    PrPsxGraphOwnerDirect::PsxGraphState& graph,
    int16_t x,
    int16_t y,
    int16_t w,
    int16_t h,
    uint32_t attr,
    uint16_t priority,
    PrPsxGraphOwnerDirect::PsxGraphWorkList80040CC8& work) {
    BoxFillLocal8001B6C4 local{};
    local.x_04 = static_cast<int16_t>(x - 160);
    local.y_06 = static_cast<int16_t>(y - 120);
    local.w_08 = w;
    local.h_0A = h;
    local.attr_00 = attr & 0xFF000000u;
    local.r_0C = static_cast<uint8_t>((attr >> 16) & 0xFFu);
    local.g_0D = static_cast<uint8_t>((attr >> 8) & 0xFFu);
    local.b_0E = static_cast<uint8_t>(attr & 0xFFu);
    PsxCall8003EE84_GsSortBoxFill(graph,
                                  local,
                                  work,
                                  priority);
}

void PsxCall80046840_DmaLinkedList(uint32_t otHead) {
    (void)otHead;
}

void PsxCall800468E0_DmaDispatch(
    uint32_t callbackFunction,
    uint32_t arg1,
    int32_t arg2,
    int32_t arg3) {
    (void)arg2;
    (void)arg3;
    if (callbackFunction == kFn80046840_DmaLinkedList) {
        PsxCall80046840_DmaLinkedList(arg1);
    }
}

void PsxCall800450A0_DrawOtag(uint32_t otHead) {
    PsxCall800468E0_DmaDispatch(kFn80046840_DmaLinkedList,
                                otHead,
                                0,
                                0);
}

uint32_t PsxCall800440B8_TextInitListHead(
    uint32_t headAddr,
    uint32_t oldHeadWord) {
    (void)headAddr;
    return oldHeadWord | kPsxPacketAddrMask8003EF5C;
}

void PsxCall8004401C_TextLinkNode(
    uint32_t headAddr,
    uint32_t& headWord,
    uint32_t nodeAddr,
    uint32_t& nodeWord) {
    (void)headAddr;
    const uint32_t newNodeWord =
        (nodeWord & 0xFF000000u) |
        (headWord & kPsxPacketAddrMask8003EF5C);
    const uint32_t newHeadWord =
        (headWord & 0xFF000000u) |
        (nodeAddr & kPsxPacketAddrMask8003EF5C);
    nodeWord = newNodeWord;
    headWord = newHeadWord;
}

void PsxCall80044238_TextRecordHeaderInit(
    EventFrameState8001E750::TextFlushRecord800436F0& record) {
    record.control_03 = 0x03u;
    record.control_07 = 0x60u;
}

void PsxCall800440D0_TextRecordModeControl(
    EventFrameState8001E750::TextFlushRecord800436F0& record,
    bool enabled) {
    record.control_07 =
        enabled ? static_cast<uint8_t>(record.control_07 | 0x02u)
                : static_cast<uint8_t>(record.control_07 & 0xFDu);
}

void PsxCall800441C0_TextGlyphPacketInit(
    uint32_t packetAddr,
    bool clutKnown,
    uint16_t clutWord) {
    (void)packetAddr;
    (void)clutKnown;
    (void)clutWord;
}

int32_t GlyphIndex800436F0(uint8_t ch) {
    if (ch >= static_cast<uint8_t>('a') &&
        ch <= static_cast<uint8_t>('z')) {
        return static_cast<int32_t>(ch) - 0x40;
    }
    return static_cast<int32_t>(ch) - 0x20;
}

void PsxCall80044E2C_MoveImage(const Rect8001B120& rect,
                               uint16_t destX,
                               uint16_t destY) {
    if (rect.w == 0) {
        return;
    }
    if (rect.h == 0) {
        return;
    }

    (void)destX;
    (void)destY;
    PsxCall800468E0_DmaDispatch(kFn80046840_DmaLinkedList,
                                kMoveImagePacketAddr8005D7DC,
                                20,
                                0);
}

void PsxCall8001B120_ResetMoveImage(
    PrPsxGraphOwnerDirect::PsxGraphState& graph,
    int32_t arg0) {
    const uint32_t slot8004019C =
        PrPsxGraphOwnerDirect::PsxCall8004019C_GetDrawBuffer(graph);
    Rect8001B120 rect{};
    rect.x = 0;
    rect.w = 320;
    rect.h = 240;
    uint16_t destY = 0;
    if (arg0 != 0) {
        rect.y = slot8004019C != 0u ? 0 : 240;
        destY = slot8004019C != 0u ? 0x00F0u : 0x0000u;
    } else {
        rect.y = slot8004019C != 0u ? 240 : 0;
        destY = slot8004019C == 0u ? 0x00F0u : 0x0000u;
    }
    PsxCall80044E2C_MoveImage(rect, 0, destY);
}

PromptSpriteTemplate800203D4 MakePromptTemplate800203D4(
    uint32_t attr,
    uint16_t texX,
    uint16_t texY,
    uint16_t w,
    uint16_t h,
    uint16_t clutX,
    uint16_t clutY) {
    PromptSpriteTemplate800203D4 out{};
    out.attr = attr;
    out.texX = texX;
    out.texY = texY;
    out.w = w;
    out.h = h;
    out.clutX = clutX;
    out.clutY = clutY;
    return out;
}

PrPsxFastSpriteSubmitDirect::FastSpriteLocalFields8003FA20
BuildPromptFastSpriteLocal8001B590(int16_t screenX,
                                   int16_t screenY,
                                   const PromptSpriteTemplate800203D4& tpl) {
    PrPsxFastSpriteSubmitDirect::FastSpriteLocalFields8003FA20 out{};
    out.attr_00 = tpl.attr;
    out.x_04 = static_cast<int16_t>(screenX - 160);
    out.y_06 = static_cast<uint16_t>(
        static_cast<int16_t>(screenY - 120));
    out.width_08 = tpl.w;
    out.height_0A = tpl.h;
    out.tpage_0C = ComputeSub80043DF4Word(tpl.texX, tpl.texY, tpl.w, 0);
    out.u_0E = static_cast<uint8_t>(4u * tpl.texX);
    out.v_0F = static_cast<uint8_t>(tpl.texY);
    out.clutX_10 = static_cast<int16_t>(tpl.clutX);
    out.clutY_12 = static_cast<int16_t>(tpl.clutY);
    return out;
}

PrPsxFastSpriteSubmitDirect::RuntimeState8003FA20
BuildFrameFastSpriteRuntime8003FA20(
    const PrPsxGraphOwnerDirect::PsxGraphState& graph,
    const PrPsxGraphOwnerDirect::PsxGraphPageWorkList8001E374& pageWork) {
    PrPsxFastSpriteSubmitDirect::RuntimeState8003FA20 out{};
    PrPsxGraphOwnerDirect::BuildRuntimeState8003FA20FromPageWork(
        graph,
        pageWork,
        out);
    return out;
}

PrPsxFastSpriteSubmitDirect::RuntimeState8003FA20
BuildEvent4FastSpriteRuntime800203D4(
    const PrPsxGraphOwnerDirect::PsxGraphState& graph,
    const PrPsxGraphOwnerDirect::PsxGraphPageWorkList8001E374& pageWork) {
    return BuildFrameFastSpriteRuntime8003FA20(graph, pageWork);
}

void TagEventFrameFastSpriteInput8003FA20(
    PrPsxFastSpriteSubmitDirect::GsSortFastSpriteInput8003FA20& input,
    PrPsxFastSpriteSubmitDirect::FastSpriteSubmitSourceKind8003FA20
        sourceKind,
    uint32_t sourceFunction,
    uint32_t callsite,
    uint32_t helper) {
    input.provenance.active = true;
    input.provenance.sourceKind = sourceKind;
    input.provenance.sourceFunction = sourceFunction;
    input.provenance.callsite = callsite;
    input.provenance.helper = helper;
}

void AppendPromptSprite800203D4(
    PrPsxFastSpriteSubmitDirect::RuntimeState8003FA20& runtime,
    int16_t screenX,
    int16_t screenY,
    const PromptSpriteTemplate800203D4& tpl,
    uint16_t priority) {
    const PrPsxFastSpriteSubmitDirect::FastSpriteLocalFields8003FA20 local =
        BuildPromptFastSpriteLocal8001B590(screenX, screenY, tpl);
    PrPsxFastSpriteSubmitDirect::GsSortFastSpriteInput8003FA20 submitInput =
        PrPsxFastSpriteSubmitDirect::BuildInputFromRuntime8003FA20(
            runtime,
            local,
            priority);
    TagEventFrameFastSpriteInput8003FA20(
        submitInput,
        PrPsxFastSpriteSubmitDirect::FastSpriteSubmitSourceKind8003FA20::
            Stage1EventFramePrompt,
        0x8001B590u,
        0x8001B5D4u,
        0x800203D4u);
    const PrPsxFastSpriteSubmitDirect::GsSortFastSpritePartialResult8003FA20
        partialSubmit =
        PrPsxFastSpriteSubmitDirect::PredictGsSortFastSpritePartial8003FA20(
            submitInput,
            false);
    PrPsxFastSpriteSubmitDirect::ApplyRuntimeUpdate8003FA20(runtime,
                                                           partialSubmit);
}

void PsxCall800203D4_PromptSprites(
    PrPsxGraphOwnerDirect::PsxGraphState& graph,
    uint32_t workListSlot,
    PrPsxGraphOwnerDirect::PsxGraphPageWorkList8001E374& pageWork,
    int32_t ctx0) {
    PrPsxFastSpriteSubmitDirect::RuntimeState8003FA20 runtime =
        BuildEvent4FastSpriteRuntime800203D4(graph, pageWork);

    static constexpr uint16_t kPriority800203D4 = 0u;
    const PromptSpriteTemplate800203D4 kPromptTitle =
        MakePromptTemplate800203D4(
            0x50000040u, 0x03C0u, 0x01CBu, 0x00D0u,
            0x0024u, 0x0130u, 0x01EEu);
    const PromptSpriteTemplate800203D4 kLeftDefault =
        MakePromptTemplate800203D4(
            0x50000040u, 0x03C0u, 0x01A9u, 0x0054u,
            0x0022u, 0x0130u, 0x01EFu);
    const PromptSpriteTemplate800203D4 kRightDefault =
        MakePromptTemplate800203D4(
            0x50000040u, 0x03D5u, 0x01A9u, 0x004Cu,
            0x0022u, 0x0130u, 0x01F0u);
    const PromptSpriteTemplate800203D4 kLeftSelected =
        MakePromptTemplate800203D4(
            0x50000040u, 0x03C0u, 0x01A9u, 0x0054u,
            0x0022u, 0x0130u, 0x01F1u);
    const PromptSpriteTemplate800203D4 kRightSelected =
        MakePromptTemplate800203D4(
            0x50000040u, 0x03D5u, 0x01A9u, 0x004Cu,
            0x0022u, 0x0130u, 0x01F2u);

    AppendPromptSprite800203D4(runtime,
                               56,
                               57,
                               kPromptTitle,
                               kPriority800203D4);
    AppendPromptSprite800203D4(runtime,
                               70,
                               149,
                               ctx0 == 0 ? kLeftSelected : kLeftDefault,
                               kPriority800203D4);
    AppendPromptSprite800203D4(runtime,
                               178,
                               152,
                               ctx0 == 1 ? kRightSelected : kRightDefault,
                               kPriority800203D4);

    PrPsxGraphOwnerDirect::CommitRuntimeState8003FA20ToMainPageWork(
        graph,
        static_cast<uint8_t>(workListSlot),
        runtime);
}

void ApplySubmitWorkList80040CA4(
    uint32_t workListAddr,
    uint32_t workListSlot,
    const PrPsxGraphOwnerDirect::PsxGraphWorkList80040CC8& work) {
    (void)workListAddr;
    (void)workListSlot;
    PsxCall800450A0_DrawOtag(work.lastAddr_10);
}

uint32_t TextFormatterArgWord80043A14(
    const TextFormatterArg80043A14* args,
    uint32_t argCount,
    uint32_t& argIndex) {
    if (args == nullptr || argIndex >= argCount) {
        return 0;
    }
    const TextFormatterArg80043A14& arg = args[argIndex++];
    return arg.word;
}

const char* TextFormatterArgString80043A14(
    const TextFormatterArg80043A14* args,
    uint32_t argCount,
    uint32_t& argIndex) {
    if (args == nullptr || argIndex >= argCount) {
        return "";
    }
    const TextFormatterArg80043A14& arg = args[argIndex++];
    (void)arg.psxPointerKnown;
    (void)arg.psxPointer;
    (void)arg.stringPointerMapKnown;
    (void)arg.sourceCallsite;
    (void)arg.source;
    if (arg.kind == TextFormatterArgKind80043A14::String) {
        return arg.string != nullptr ? arg.string : "";
    }
    return arg.string != nullptr ? arg.string : "";
}

bool AppendTextByte80043A14(
    EventFrameState8001E750::TextFlushRecord800436F0& record,
    char ch) {
    const uint32_t capacity =
        record.budget_1C < 0 ? 0u : static_cast<uint32_t>(record.budget_1C);
    const uint32_t writeIndex = record.word_28;
    if (writeIndex >= capacity) {
        return false;
    }
    record.textStorage_24[writeIndex] = ch;
    record.word_28 = writeIndex + 1u;
    return true;
}

void FormatSignedDecimal80043A14(int32_t value,
                                 char* scratch,
                                 uint32_t& start,
                                 uint32_t& length) {
    char* cursor = scratch + 64;
    uint32_t magnitude = static_cast<uint32_t>(value);
    bool negative = false;
    if (value < 0) {
        negative = true;
        magnitude = static_cast<uint32_t>(0u - magnitude);
    }
    do {
        *--cursor = static_cast<char>('0' + (magnitude % 10u));
        magnitude /= 10u;
    } while (magnitude != 0u);
    if (negative) {
        *--cursor = '-';
    }
    start = static_cast<uint32_t>(cursor - scratch);
    length = static_cast<uint32_t>((scratch + 64) - cursor);
}

void FormatHex80043A14(uint32_t value,
                       const TextHexDigitTableCarrier80043A14& tableCarrier,
                       bool zeroPad,
                       uint32_t width,
                       char* scratch,
                       uint32_t& start,
                       uint32_t& length) {
    char* cursor = scratch + 64;
    do {
        const uint32_t digit = value & 0x0Fu;
        *--cursor = tableCarrier.tableKnown
            ? static_cast<char>(tableCarrier.tableBytes[digit])
            : kVisibleCompatibilityHexDigits80043A14[digit];
        value >>= 4;
    } while (value != 0u);
    length = static_cast<uint32_t>((scratch + 64) - cursor);
    if (zeroPad) {
        while (length < width) {
            *--cursor = '0';
            ++length;
        }
    }
    start = static_cast<uint32_t>(cursor - scratch);
}

bool AppendBoundedText80026314(char* dst,
                               uint32_t capacity,
                               uint32_t& cursor,
                               const char* text) {
    if (dst == nullptr || capacity == 0u || text == nullptr) {
        return false;
    }
    bool complete = true;
    for (const char* src = text; *src != '\0'; ++src) {
        if (cursor + 1u >= capacity) {
            complete = false;
            break;
        }
        dst[cursor++] = *src;
    }
    dst[cursor < capacity ? cursor : capacity - 1u] = '\0';
    return complete;
}

const char* MenuHelpGroupMarker80026314(bool selected) {
    return selected ? "~c888 >>>" : "~c000    ";
}

const char* MenuHelpItemMarker80026314(bool selected) {
    return selected ? "~c888*" : "~c000 ";
}

}  // namespace

void PsxCall80040CA4_SubmitWorkList(
    uint32_t workListAddr,
    uint32_t workListSlot,
    const PrPsxGraphOwnerDirect::PsxGraphWorkList80040CC8& work) {
    ApplySubmitWorkList80040CA4(workListAddr,
                                workListSlot,
                                work);
}

void ResetEventFrameState8003FB9C(EventFrameState8001E750& state,
                                  uint16_t width,
                                  uint16_t height) {
    state = EventFrameState8001E750{};
    PrPsxGraphOwnerDirect::PsxInitializeGraphState8003FB9C(state.graph,
                                                          width,
                                                          height);
    for (uint32_t index = 0; index < kTextRecordCount800436F0; ++index) {
        state.textFlush.records[index].recordAddr =
            kTextRecordBase8005CB5C +
            kTextRecordStride800436F0 * index;
    }
    state.initialized = true;
}

void ResetStage1Event4Gp38CSeed8006EDCC() {
    g_stage1Event4Gp38CSeedValid8006EDCC = false;
    g_stage1Event4Gp38CSeed8006EDCC = 0u;
}

void SeedStage1Event4Gp38C8006EDCC(uint32_t value) {
    g_stage1Event4Gp38CSeedValid8006EDCC = true;
    g_stage1Event4Gp38CSeed8006EDCC = value;
}

bool ConsumeStage1Event4Gp38CSeed8006EDCC(uint32_t& outValue) {
    if (!g_stage1Event4Gp38CSeedValid8006EDCC) {
        outValue = 0u;
        return false;
    }

    outValue = g_stage1Event4Gp38CSeed8006EDCC;
    ResetStage1Event4Gp38CSeed8006EDCC();
    return true;
}

void PsxCall80043394_FntLoad(
    EventFrameState8001E750& state,
    int16_t x,
    int16_t y) {
    (void)x;
    (void)y;
    if (!state.initialized) {
        ResetEventFrameState8003FB9C(state, 320u, 240u);
    }

    EventFrameState8001E750::TextFlushState800436F0& text =
        state.textFlush;
    text = EventFrameState8001E750::TextFlushState800436F0{};
    text.bankKnown = true;
    text.slotLimit8005CCDC = 0;
    text.currentSlot8005CCE0 = 0;
    text.currentSlotActiveKnown = false;
    text.currentSlotActive8005CB80 = false;
    text.fontLoadKnown = true;
    text.fontPageWord8008EB50Known = false;
    text.fontClutWord8008EB54Known = false;
    for (uint32_t index = 0; index < kTextRecordCount800436F0; ++index) {
        text.records[index].recordAddr =
            kTextRecordBase8005CB5C +
            kTextRecordStride800436F0 * index;
    }
}

void PsxCall80043354_SetDumpFnt(
    EventFrameState8001E750& state,
    int32_t slot) {
    if (!state.initialized) {
        ResetEventFrameState8003FB9C(state, 320u, 240u);
    }

    if (slot < 0 || slot > state.textFlush.slotLimit8005CCDC) {
        return;
    }

    state.textFlush.currentSlot8005CCE0 = slot;
    state.textFlush.textAppendCallback8005D730Known = true;
    state.textFlush.textAppendCallback8005D730 = kFn80043A14_TextAppend;
}

void PsxCall80043438_TextRecordAlloc(
    EventFrameState8001E750& state,
    int16_t x,
    int16_t y,
    int16_t w,
    int16_t h,
    int32_t mode,
    int32_t capacity) {
    if (!state.initialized) {
        ResetEventFrameState8003FB9C(state, 320u, 240u);
    }

    if (!state.textFlush.bankKnown) {
        return;
    }

    const int32_t slot = state.textFlush.slotLimit8005CCDC;
    if (slot < 0 || slot >= static_cast<int32_t>(kTextRecordCount800436F0)) {
        return;
    }

    if (state.textFlush.slotLimit8005CCDC == 0) {
        state.textFlush.glyphCursorDword8005D6E4 = 0;
    }

    const uint32_t glyphCursor = state.textFlush.glyphCursorDword8005D6E4;
    int32_t clampedCapacity = capacity;
    if (clampedCapacity < 0) {
        clampedCapacity = 0;
    }
    if (glyphCursor >= kTextRecordMaxTextCapacity80043438) {
        clampedCapacity = 0;
    } else if (glyphCursor + static_cast<uint32_t>(clampedCapacity) >
        kTextRecordMaxTextCapacity80043438) {
        clampedCapacity =
            static_cast<int32_t>(kTextRecordMaxTextCapacity80043438 -
                                 glyphCursor);
    }

    EventFrameState8001E750::TextFlushRecord800436F0& record =
        state.textFlush.records[slot];
    record.x_08 = x;
    record.y_0A = y;
    record.w_0C = w;
    record.h_0E = h;
    record.budget_1C = clampedCapacity;
    record.packetCursor_20 =
        kTextRecordGlyphBufferBase8008AB50 +
        glyphCursor * kTextGlyphPacketStride800436F0;
    record.textAddr_24 = kTextRecordTextBufferBase8008A750 + glyphCursor;
    record.text_24 = record.textStorage_24;
    record.textStorage_24[0] = '\0';
    record.word_28 = 0;
    record.measureMode_2C = w == 0 ? 1 : 0;
    if (mode != 0) {
        PsxCall80044238_TextRecordHeaderInit(record);
        record.r_04 = 0;
        record.g_05 = 0;
        record.b_06 = 0;
        PsxCall800440D0_TextRecordModeControl(record, mode == 2);
    }

    for (int32_t i = 0; i < clampedCapacity; ++i) {
        const uint32_t packetAddr =
            record.packetCursor_20 +
            static_cast<uint32_t>(i) * kTextGlyphPacketStride800436F0;
        PsxCall800441C0_TextGlyphPacketInit(
            packetAddr,
            state.textFlush.fontClutWord8008EB54Known,
            static_cast<uint16_t>(state.textFlush.fontClutWord8008EB54));
    }

    state.textFlush.glyphCursorDword8005D6E4 =
        glyphCursor + static_cast<uint32_t>(clampedCapacity);
    state.textFlush.slotLimit8005CCDC = slot + 1;
    state.textFlush.currentSlotActiveKnown = true;
    state.textFlush.currentSlotActive8005CB80 = record.text_24 != nullptr;
}

void PsxCall80043A14_TextAppend(
    EventFrameState8001E750& state,
    int32_t arg0,
    const char* text) {
    PsxCall80043A14_TextAppend(state, arg0, text, nullptr, 0);
}

void PsxCall80043A14_TextAppend(
    EventFrameState8001E750& state,
    int32_t arg0,
    const char* text,
    const TextFormatterArg80043A14* args,
    uint32_t argCount) {
    if (!state.initialized) {
        ResetEventFrameState8003FB9C(state, 320u, 240u);
    }

    int32_t selectedSlot = arg0;
    if (arg0 < 0 || arg0 >= state.textFlush.slotLimit8005CCDC) {
        selectedSlot = state.textFlush.currentSlot8005CCE0;
        if (state.textFlush.currentSlotActiveKnown &&
            !state.textFlush.currentSlotActive8005CB80) {
            return;
        }
    }
    if (selectedSlot < 0 ||
        selectedSlot >= static_cast<int32_t>(kTextRecordCount800436F0)) {
        return;
    }

    EventFrameState8001E750::TextFlushRecord800436F0& record =
        state.textFlush.records[selectedSlot];
    if (record.text_24 == nullptr) {
        return;
    }
    if (text == nullptr) {
        return;
    }

    const uint32_t capacity =
        record.budget_1C < 0 ? 0u : static_cast<uint32_t>(record.budget_1C);
    if (record.word_28 >= capacity) {
        return;
    }

    uint32_t argIndex = 0;
    const char* fmt = text;
    while (*fmt != '\0') {
        if (*fmt != '%' || fmt[1] == '%') {
            if (*fmt == '%' && fmt[1] == '%') {
                ++fmt;
            }
            if (!AppendTextByte80043A14(record, *fmt++)) {
                return;
            }
            continue;
        }

        ++fmt;
        const bool zeroPad = *fmt == '0';
        uint32_t width = 0;
        while (*fmt >= '0' && *fmt <= '9') {
            width = width * 10u +
                    static_cast<uint32_t>(*fmt - '0');
            ++fmt;
        }
        if (width == 0u) {
            width = 1u;
        }

        char scratch[64]{};
        const char* formatted = scratch;
        uint32_t formattedStart = 0;
        uint32_t formattedLen = 0;
        switch (*fmt) {
            case 'd':
                FormatSignedDecimal80043A14(
                    static_cast<int32_t>(
                        TextFormatterArgWord80043A14(args,
                                                      argCount,
                                                      argIndex)),
                    scratch,
                    formattedStart,
                    formattedLen);
                formatted = scratch + formattedStart;
                break;
            case 'x':
            case 'X':
                FormatHex80043A14(
                    TextFormatterArgWord80043A14(args, argCount, argIndex),
                    kHexDigitTable8005D6E8Carrier80043A14,
                    zeroPad,
                    width,
                    scratch,
                    formattedStart,
                    formattedLen);
                formatted = scratch + formattedStart;
                break;
            case 'c':
                scratch[0] = static_cast<char>(
                    TextFormatterArgWord80043A14(args, argCount, argIndex) &
                    0xFFu);
                formattedLen = 1;
                break;
            case 's':
                formatted = TextFormatterArgString80043A14(
                    args,
                    argCount,
                    argIndex);
                formattedLen =
                    static_cast<uint32_t>(std::strlen(formatted));
                break;
            default:
                formattedLen = 0;
                break;
        }

        while (formattedLen < width) {
            if (!AppendTextByte80043A14(record, ' ')) {
                return;
            }
            --width;
        }
        for (uint32_t i = 0; i < formattedLen; ++i) {
            if (!AppendTextByte80043A14(record, formatted[i])) {
                return;
            }
        }

        if (*fmt != '\0') {
            ++fmt;
        }
    }

    record.textStorage_24[record.word_28] = '\0';
    record.text_24 = record.textStorage_24;
}

void PsxCall80043A14_TextAppendSlotFmt(
    EventFrameState8001E750& state,
    int32_t slot,
    const char* fmt,
    const TextFormatterArg80043A14* args,
    uint32_t argCount) {
    PsxCall80043A14_TextAppend(state, slot, fmt, args, argCount);
}

void PsxCall80043A14_TextAppendCurrentFmt(
    EventFrameState8001E750& state,
    const char* fmt,
    const TextFormatterArg80043A14* args,
    uint32_t argCount) {
    PsxCall80043A14_TextAppend(state, -1, fmt, args, argCount);
}

MenuHelpItemStackText80026424 PsxBuild80026314_MenuHelpItemStackText(
    uint32_t groupIndex,
    uint32_t itemIndex,
    uint32_t groupAddr,
    bool selected,
    const MenuHelpItemInput80026314& input) {
    MenuHelpItemStackText80026424 out{};
    out.called = true;
    out.groupIndex = groupIndex;
    out.itemIndex = itemIndex;
    out.groupAddr = groupAddr;
    out.itemPointerSlotAddrKnown = groupAddr != 0u;
    if (out.itemPointerSlotAddrKnown) {
        out.itemPointerSlotAddr =
            groupAddr + kMenuHelpGroupItemLabelPointerBaseOffset80026314 +
            itemIndex * sizeof(uint32_t);
    }
    out.selected = selected;
    out.marker = MenuHelpItemMarker80026314(selected);
    out.labelKnown = input.labelKnown && input.label != nullptr;
    out.labelAddr = input.labelAddr;
    out.label = input.label;
    out.labelPointerMapGap = !out.labelKnown;
    out.labelPointerMapSource =
        out.labelPointerMapGap
            ? "unresolved: 800263D4 loads item text pointer from s2+8; "
              "PSX pointer->host string map not exported"
            : "caller supplied exported host string for 800263D4 item text";

    if (!out.labelKnown) {
        out.stackBufferProducerGap = true;
        return out;
    }

    uint32_t cursor = 0;
    bool complete = AppendBoundedText80026314(
        out.stackText,
        kMenuHelpStackTextCapacity80026314,
        cursor,
        out.marker);
    complete = AppendBoundedText80026314(
        out.stackText,
        kMenuHelpStackTextCapacity80026314,
        cursor,
        input.label) &&
               complete;

    const uint32_t labelLen = static_cast<uint32_t>(std::strlen(input.label));
    for (uint32_t i = labelLen; i < 6u; ++i) {
        if (cursor + 1u >= kMenuHelpStackTextCapacity80026314) {
            complete = false;
            break;
        }
        out.stackText[cursor++] = ' ';
        out.stackText[cursor] = '\0';
    }

    out.stackBufferKnown = true;
    out.stackBufferProducerGap = false;
    out.stackBufferTruncatedGap = !complete;
    return out;
}

void PsxProduce80026314_MenuHelpText(
    EventFrameState8001E750& state,
    const MenuHelpTextInput80026314& input) {
    (void)PsxCall80043A14_TextAppendCurrentFmt(
        state,
        "\n\n\n\n",
        nullptr,
        0);

    const uint32_t groupLimit =
        input.groupCount < kMenuHelpMaxGroups80026314
            ? input.groupCount
            : kMenuHelpMaxGroups80026314;
    for (uint32_t groupIndex = 0; groupIndex < groupLimit; ++groupIndex) {
        MenuHelpGroupAppend80026314 groupOut{};
        groupOut.called = true;
        groupOut.groupIndex = groupIndex;
        groupOut.selected = groupIndex == input.selectedGroupIndex;
        groupOut.markerLiteral = MenuHelpGroupMarker80026314(groupOut.selected);
        (void)PsxCall80043A14_TextAppendCurrentFmt(
            state,
            groupOut.markerLiteral,
            nullptr,
            0);

        if (input.groups == nullptr) {
            (void)PsxCall80043A14_TextAppendCurrentFmt(
                state,
                "\n",
                nullptr,
                0);
            continue;
        }

        const MenuHelpGroupInput80026314& groupIn = input.groups[groupIndex];
        groupOut.groupAddr = groupIn.groupAddr;
        groupOut.titlePointerSlotAddrKnown = groupIn.groupAddr != 0u;
        if (groupOut.titlePointerSlotAddrKnown) {
            groupOut.titlePointerSlotAddr =
                groupIn.groupAddr +
                kMenuHelpGroupTitlePointerOffset80026314;
        }
        groupOut.titleKnown = groupIn.titleKnown && groupIn.title != nullptr;
        groupOut.titleAddr = groupIn.titleAddr;
        groupOut.titlePointerMapAddr = groupIn.titleAddr;
        groupOut.titlePointerMapGap = !groupOut.titleKnown;
        groupOut.titlePointerMapSource =
            groupOut.titlePointerMapGap
                ? "unresolved: 80026394 passes *(group+0x04) to %s; "
                  "PSX pointer->host string map needs IDA/data export"
                : "caller supplied exported host string for 80026394 group title";
        groupOut.titleArg.kind = TextFormatterArgKind80043A14::String;
        groupOut.titleArg.string = groupIn.title;
        groupOut.titleArg.psxPointerKnown = groupIn.titleAddr != 0u;
        groupOut.titleArg.psxPointer = groupIn.titleAddr;
        groupOut.titleArg.stringPointerMapKnown = groupOut.titleKnown;
        groupOut.titleArg.sourceCallsite =
            kCallsite80026394_MenuHelpGroupTitle;
        groupOut.titleArg.source = groupOut.titlePointerMapSource;
        if (groupOut.titleKnown) {
            (void)PsxCall80043A14_TextAppendCurrentFmt(
                state,
                "%s:",
                &groupOut.titleArg,
                1);
        }

        groupOut.itemCount = groupIn.itemCount;
        groupOut.selectedItemIndex = groupIn.selectedItemIndex;
        groupOut.itemSourceGap =
            groupIn.items == nullptr && groupIn.itemCount != 0u;
        groupOut.itemOutputTruncatedGap =
            groupIn.itemCount > kMenuHelpMaxItemsPerGroup80026314;
        const uint32_t itemLimit =
            groupIn.itemCount < kMenuHelpMaxItemsPerGroup80026314
                ? groupIn.itemCount
                : kMenuHelpMaxItemsPerGroup80026314;
        for (uint32_t itemIndex = 0; itemIndex < itemLimit; ++itemIndex) {
            MenuHelpItemStackText80026424& itemOut =
                groupOut.itemAppends[groupOut.itemAppendCount++];
            if (groupIn.items == nullptr) {
                itemOut.called = true;
                itemOut.groupIndex = groupIndex;
                itemOut.itemIndex = itemIndex;
                itemOut.stackBufferProducerGap = true;
                continue;
            }
            itemOut = PsxBuild80026314_MenuHelpItemStackText(
                groupIndex,
                itemIndex,
                groupIn.groupAddr,
                itemIndex == groupIn.selectedItemIndex,
                groupIn.items[itemIndex]);
            if (itemOut.stackBufferKnown && !itemOut.stackBufferTruncatedGap) {
                (void)PsxCall80043A14_TextAppendCurrentFmt(
                    state,
                    itemOut.stackText,
                    nullptr,
                    0);
            }
        }

        (void)PsxCall80043A14_TextAppendCurrentFmt(
            state,
            "\n",
            nullptr,
            0);
    }

    (void)PsxCall80043A14_TextAppendCurrentFmt(
        state,
        "\n\n~c222      O: OK   X: CANCEL~c888\n",
        nullptr,
        0);
    (void)PsxCall800436F0_TextFlush(state, -1);
}

void PsxProduce80026B94_Event2StageClearText(
    EventFrameState8001E750& state,
    const StageClearTextInput80026B94& input) {
    if (input.eventId != 2) {
        return;
    }
    if (!input.word800916F6Known) {
        return;
    }
    if (input.word_800916F6 == 0u) {
        return;
    }

    (void)PsxCall80043A14_TextAppendCurrentFmt(
        state,
        "\n\n\n~c000StageClear: ",
        nullptr,
        0);

    const char* format8006EC14 =
        input.format8006EC14 != nullptr
            ? input.format8006EC14
            : kStageClearWordFormat8006EC14;
    if (!input.statusBytesKnown80092F1D) {
        return;
    }

    for (uint32_t i = 0; i < kStageClearStatusWordAppendCount80026DAC; ++i) {
        TextFormatterArg80043A14 arg{};
        arg.kind = TextFormatterArgKind80043A14::Word;
        arg.word = input.byte_80092F1D[i];
        (void)PsxCall80043A14_TextAppendCurrentFmt(
            state,
            format8006EC14,
            &arg,
            1);
    }
}

void PsxCall80027FAC_TextSystemBoot(EventFrameState8001E750& state) {
    if (!state.initialized) {
        ResetEventFrameState8003FB9C(state, 320u, 240u);
    }

    PsxCall80043394_FntLoad(state, 960, 256);
    PsxCall80043438_TextRecordAlloc(state, -156, -120, 320, 200, 0, 512);
    PsxCall80043354_SetDumpFnt(state, 0);
}

void BeginDrawWrapper8001E750(
    EventFrameState8001E750& state,
    int32_t eventId) {
    (void)eventId;
    if (!state.initialized) {
        ResetEventFrameState8003FB9C(state, 320u, 240u);
    }

    const uint32_t slot8004019C =
        PrPsxGraphOwnerDirect::PsxCall8004019C_GetDrawBuffer(state.graph) &
        1u;
    state.gp368WorkSlot = slot8004019C;
    const uint32_t packetAllocator8006ED50 =
        state.graph.dword_8006ED50[slot8004019C];
    PrPsxGraphOwnerDirect::PsxCall80040F90_SetPacketAllocator(
        state.graph,
        packetAllocator8006ED50);
    (void)PrPsxGraphOwnerDirect::PsxCall8001E374_ClearMainPageWork(
        state.graph,
        static_cast<uint8_t>(slot8004019C));
    EventFrameState8001E750::FastSpritePageRuntime8003FA20& pageRuntime =
        state.fastSpritePageRuntime8003FA20;
    pageRuntime.valid = true;
    pageRuntime.dirty = false;
    pageRuntime.slot8004019C = static_cast<uint8_t>(slot8004019C);
    pageRuntime.workListAddr = WorkListAddr80087288(slot8004019C & 1u);
    pageRuntime.runtime =
        BuildFrameFastSpriteRuntime8003FA20(
            state.graph,
            state.graph.mainPageWorkLists80087288[slot8004019C & 1u]);
}

void AppendEventBackdropSubmit8001D74C(
    uint32_t& submitCount,
    uint16_t priority,
    int16_t x,
    int16_t y,
    uint32_t templatePointerSlot,
    PrPsxFastSpriteSubmitDirect::RuntimeState8003FA20& runtime) {
    if (submitCount >= kEventBackdropSubmitCount8001D74C) {
        return;
    }

    ++submitCount;
    const StageSelectSpriteTemplate8001B4E0 tpl =
        ResolveStageSelectTemplate8001B25C(templatePointerSlot);
    if (tpl.known) {
        PrPsxFastSpriteSubmitDirect::FastSpriteLocalFields8003FA20 local{};
        local.attr_00 = tpl.attr_00;
        local.x_04 = static_cast<int16_t>(x - 160);
        local.y_06 =
            static_cast<uint16_t>(static_cast<int16_t>(y - 120));
        local.width_08 = tpl.width_08;
        local.height_0A = tpl.height_0A;
        local.tpage_0C = ComputeSub80043DF4Word(tpl.texX_04,
                                                tpl.texY_06,
                                                tpl.width_08,
                                                0);
        local.u_0E =
            static_cast<uint8_t>(static_cast<uint16_t>(4u * tpl.texX_04));
        local.v_0F = static_cast<uint8_t>(tpl.texY_06);
        local.clutX_10 = static_cast<int16_t>(tpl.clutX_0C);
        local.clutY_12 = static_cast<int16_t>(tpl.clutY_0E);
        PrPsxFastSpriteSubmitDirect::GsSortFastSpriteInput8003FA20
            submitInput =
            PrPsxFastSpriteSubmitDirect::BuildInputFromRuntime8003FA20(
                runtime,
                local,
                priority);
        TagEventFrameFastSpriteInput8003FA20(
            submitInput,
            PrPsxFastSpriteSubmitDirect::FastSpriteSubmitSourceKind8003FA20::
                Stage1EventFrameBackdrop,
            0x8001B590u,
            0x8001B5D4u,
            0x8001D74Cu);
        const PrPsxFastSpriteSubmitDirect::GsSortFastSpritePartialResult8003FA20
            partialSubmit =
            PrPsxFastSpriteSubmitDirect::PredictGsSortFastSpritePartial8003FA20(
                submitInput,
                false);
        PrPsxFastSpriteSubmitDirect::ApplyRuntimeUpdate8003FA20(
            runtime,
            partialSubmit);
    }
}

void PsxCall8001D74C_EventBackdrop(
    uint16_t priority,
    PrPsxFastSpriteSubmitDirect::RuntimeState8003FA20& runtime) {
    uint32_t submitCount = 0;

    AppendEventBackdropSubmit8001D74C(
        submitCount, priority, 20, 20, 0x8004E7E0u, runtime);
    AppendEventBackdropSubmit8001D74C(
        submitCount, priority, 20, 120, 0x8004E7F0u, runtime);
    AppendEventBackdropSubmit8001D74C(
        submitCount, priority, 280, 20, 0x8004E800u, runtime);
    AppendEventBackdropSubmit8001D74C(
        submitCount, priority, 280, 120, 0x8004E810u, runtime);
    AppendEventBackdropSubmit8001D74C(
        submitCount, priority, 40, 20, 0x8004E820u, runtime);
    AppendEventBackdropSubmit8001D74C(
        submitCount, priority, 160, 20, 0x8004E830u, runtime);
    AppendEventBackdropSubmit8001D74C(
        submitCount, priority, 40, 200, 0x8004E840u, runtime);
    AppendEventBackdropSubmit8001D74C(
        submitCount, priority, 160, 200, 0x8004E850u, runtime);

    for (int32_t x = 40; x < 280; x += 40) {
        for (int32_t y = 40; y < 200; y += 40) {
            AppendEventBackdropSubmit8001D74C(
                submitCount,
                priority,
                static_cast<int16_t>(x),
                static_cast<int16_t>(y),
                kEventBackdropTileTemplateSlot8001D74C,
                runtime);
        }
    }

    for (int32_t x = 40; x < 320; x += 40) {
        AppendEventBackdropSubmit8001D74C(submitCount, priority,
                                         static_cast<int16_t>(x), 0,
                                         0x8004E900u, runtime);
        AppendEventBackdropSubmit8001D74C(submitCount, priority,
                                         static_cast<int16_t>(x + 20), 0,
                                         0x8004E910u, runtime);
        AppendEventBackdropSubmit8001D74C(submitCount, priority,
                                         static_cast<int16_t>(x), 220,
                                         0x8004E920u, runtime);
        AppendEventBackdropSubmit8001D74C(submitCount, priority,
                                         static_cast<int16_t>(x + 20), 220,
                                         0x8004E930u, runtime);
    }

    for (int32_t y = 0; y < 240; y += 40) {
        AppendEventBackdropSubmit8001D74C(submitCount, priority, 0,
                                         static_cast<int16_t>(y),
                                         0x8004E900u, runtime);
        AppendEventBackdropSubmit8001D74C(submitCount, priority, 300,
                                         static_cast<int16_t>(y),
                                         0x8004E910u, runtime);
        AppendEventBackdropSubmit8001D74C(submitCount, priority, 0,
                                         static_cast<int16_t>(y + 20),
                                         0x8004E920u, runtime);
        AppendEventBackdropSubmit8001D74C(submitCount, priority, 300,
                                         static_cast<int16_t>(y + 20),
                                         0x8004E930u,
                                         runtime);
    }
}

bool BuildStageSelectLocal8001B25C(
    int16_t screenX,
    int16_t screenY,
    const StageSelectSpriteTemplate8001B4E0& tpl,
    int32_t repeatArg,
    int32_t clutYOffsetFlagArg,
    PrPsxFastSpriteSubmitDirect::FastSpriteLocalFields8003FA20& outLocal) {
    outLocal = PrPsxFastSpriteSubmitDirect::FastSpriteLocalFields8003FA20{};
    if (!tpl.known) {
        return false;
    }
    outLocal.attr_00 = tpl.attr_00;
    outLocal.x_04 = static_cast<int16_t>(screenX - 160);
    outLocal.y_06 =
            static_cast<uint16_t>(static_cast<int16_t>(screenY - 120));
    outLocal.width_08 = tpl.width_08;
    outLocal.height_0A = tpl.height_0A;
    outLocal.tpage_0C = ComputeSub80043DF4Word(tpl.texX_04,
                                               tpl.texY_06,
                                               tpl.width_08,
                                               static_cast<uint16_t>(
                                                   repeatArg));
    outLocal.u_0E =
        static_cast<uint8_t>(static_cast<uint16_t>(
            4u * tpl.texX_04 +
            static_cast<uint16_t>(repeatArg) * tpl.width_08));
    outLocal.v_0F = static_cast<uint8_t>(tpl.texY_06);
    outLocal.clutX_10 = static_cast<int16_t>(tpl.clutX_0C);
    outLocal.clutY_12 =
            clutYOffsetFlagArg != 0
                ? static_cast<int16_t>(
                      static_cast<int32_t>(tpl.clutY_0E) + repeatArg)
                : static_cast<int16_t>(tpl.clutY_0E);
    return true;
}

void MakeStageSelectSubmit8001B590(
    int16_t screenX,
    int16_t screenY,
    uint32_t templateAddr,
    int32_t repeatArg,
    int32_t clutYOffsetFlagArg,
    uint16_t priority,
    bool workListKnown,
    uint32_t workListAddr,
    PrPsxFastSpriteSubmitDirect::RuntimeState8003FA20* runtime) {
    (void)workListAddr;
    PrPsxFastSpriteSubmitDirect::FastSpriteLocalFields8003FA20 local{};
    const bool localKnown =
        BuildStageSelectLocal8001B25C(
            screenX,
            screenY,
            ResolveStageSelectTemplate8001B25C(templateAddr),
            repeatArg,
            clutYOffsetFlagArg,
            local);
    if (localKnown && workListKnown && runtime != nullptr) {
        PrPsxFastSpriteSubmitDirect::GsSortFastSpriteInput8003FA20
            submitInput =
                PrPsxFastSpriteSubmitDirect::BuildInputFromRuntime8003FA20(
                    *runtime,
                    local,
                    priority);
        TagEventFrameFastSpriteInput8003FA20(
            submitInput,
            PrPsxFastSpriteSubmitDirect::FastSpriteSubmitSourceKind8003FA20::
                Stage1EventFrameStageSelect,
            0x8001B590u,
            0x8001B5D4u,
            0x80020568u);
        const PrPsxFastSpriteSubmitDirect::GsSortFastSpritePartialResult8003FA20
            partialSubmit =
                PrPsxFastSpriteSubmitDirect::PredictGsSortFastSpritePartial8003FA20(
                    submitInput,
                    false);
        PrPsxFastSpriteSubmitDirect::ApplyRuntimeUpdate8003FA20(
            *runtime,
            partialSubmit);
    }
}

void MakeStageSelectSubmit8001C550(int16_t x,
                                   int16_t y,
                                   uint32_t templatePtr,
                                   uint16_t priority,
                                   uint32_t gp368WorkSlot,
                                   PrPsxFastSpriteSubmitDirect::RuntimeState8003FA20* runtime) {
    const bool workListKnown = true;
    const uint32_t workListAddr = WorkListAddr80087288(gp368WorkSlot & 1u);
    uint32_t templateForDownstream = templatePtr;
    uint32_t templateSourceSlotAddr = 0;
    uint32_t templateFromGp = 0;
    if (ResolveStageSelectGpTemplateSlot80020568(templatePtr,
                                                 &templateSourceSlotAddr,
                                                 &templateFromGp)) {
        (void)templateSourceSlotAddr;
        templateForDownstream = templateFromGp;
    }
    MakeStageSelectSubmit8001B590(x,
                                  y,
                                  templateForDownstream,
                                  0,
                                  0,
                                  priority,
                                  workListKnown,
                                  workListAddr,
                                  runtime);
}

void MakeStageSelectSubmit8001C5A8(
    uint32_t positionPairAddr,
    uint32_t templatePtrOrTableAddr,
    uint32_t selectedIndex,
    uint16_t priority,
    uint32_t gp368WorkSlot,
    PrPsxFastSpriteSubmitDirect::RuntimeState8003FA20* runtime) {
    (void)selectedIndex;
    const bool workListKnown = true;
    const uint32_t workListAddr = WorkListAddr80087288(gp368WorkSlot & 1u);
    int16_t x = 0;
    int16_t y = 0;
    const bool resolvedPairKnown =
        ResolveStageSelectPositionPair80020568(positionPairAddr, &x, &y);
    uint32_t templatePtrOrSlot = 0;
    bool templateKnown =
        ResolveStageSelectTemplatePointer80020568(templatePtrOrTableAddr,
                                                  &templatePtrOrSlot);
    if (!templateKnown) {
        const StageSelectSpriteTemplate8001B4E0 directTpl =
            ResolveStageSelectTemplate8001B25C(templatePtrOrTableAddr);
        if (directTpl.known) {
            templatePtrOrSlot = templatePtrOrTableAddr;
            templateKnown = true;
        }
    }
    if (resolvedPairKnown && templateKnown) {
        MakeStageSelectSubmit8001B590(x,
                                      y,
                                      templatePtrOrSlot,
                                      0,
                                      0,
                                      priority,
                                      workListKnown,
                                      workListAddr,
                                      runtime);
    }
}

int32_t ClampLanguageIndex80022CBC(int32_t languageIndex) {
    if (languageIndex < 0) {
        return 0;
    }
    if (languageIndex > 4) {
        return 4;
    }
    return languageIndex;
}

void MakeSaveUiSubmit80022CBC(int16_t x,
                              int16_t y,
                              uint32_t templateAddr,
                              uint16_t priority,
                              uint32_t gp368WorkSlot,
                              PrPsxFastSpriteSubmitDirect::RuntimeState8003FA20* runtime) {
    if (runtime == nullptr) {
        return;
    }

    PrPsxFastSpriteSubmitDirect::FastSpriteLocalFields8003FA20 local{};
    const bool localKnown =
        BuildStageSelectLocal8001B25C(
            x,
            y,
            ResolveStageSelectTemplate8001B25C(templateAddr),
            0,
            0,
            local);
    if (!localKnown) {
        return;
    }

    PrPsxFastSpriteSubmitDirect::GsSortFastSpriteInput8003FA20 submitInput =
        PrPsxFastSpriteSubmitDirect::BuildInputFromRuntime8003FA20(
            *runtime,
            local,
            priority);
    TagEventFrameFastSpriteInput8003FA20(
        submitInput,
        PrPsxFastSpriteSubmitDirect::FastSpriteSubmitSourceKind8003FA20::
            Stage1EventFrameSaveUi,
        kFn8001B590_FastSpriteSubmit,
        0x8001B5D4u,
        0x80022CBCu);
    const PrPsxFastSpriteSubmitDirect::GsSortFastSpritePartialResult8003FA20
        partialSubmit =
        PrPsxFastSpriteSubmitDirect::PredictGsSortFastSpritePartial8003FA20(
            submitInput,
            false);
    PrPsxFastSpriteSubmitDirect::ApplyRuntimeUpdate8003FA20(
        *runtime,
        partialSubmit);
}

void MakeSaveUiSubmit8001C5A8Table(
    uint32_t positionBaseAddr,
    uint32_t templateBaseAddr,
    uint32_t stride,
    int32_t languageIndex,
    uint16_t priority,
    uint32_t gp368WorkSlot,
    PrPsxFastSpriteSubmitDirect::RuntimeState8003FA20* runtime) {
    (void)gp368WorkSlot;
    if (runtime == nullptr) {
        return;
    }
    const uint32_t lang =
        static_cast<uint32_t>(ClampLanguageIndex80022CBC(languageIndex));
    int16_t x = 0;
    int16_t y = 0;
    uint32_t templateAddr = 0;
    if (!ResolveStageSelectPositionPair80020568(positionBaseAddr + stride * lang,
                                                &x,
                                                &y) ||
        !ResolveStageSelectTemplatePointer80020568(templateBaseAddr + stride * lang,
                                                   &templateAddr)) {
        return;
    }
    MakeSaveUiSubmit80022CBC(x, y, templateAddr, priority, gp368WorkSlot, runtime);
}

void PsxCall80022CBC_SingleButton(
    uint32_t headerPositionBaseAddr,
    uint32_t headerTemplateBaseAddr,
    int32_t contextWord0,
    int32_t contextWord1,
    int32_t languageIndex,
    uint32_t gp368WorkSlot,
    PrPsxFastSpriteSubmitDirect::RuntimeState8003FA20* runtime) {
    MakeSaveUiSubmit80022CBC(
        36,
        54,
        0x80052330u,
        1,
        gp368WorkSlot,
        runtime);
    MakeSaveUiSubmit8001C5A8Table(headerPositionBaseAddr,
                                  headerTemplateBaseAddr,
                                  8u,
                                  languageIndex,
                                  1,
                                  gp368WorkSlot,
                                  runtime);
    MakeSaveUiSubmit80022CBC(
        231,
        179,
        contextWord0 == 1 ? kStageSelectExitFrameOnTemplate80050AD0
                          : kStageSelectExitFrameNormalTemplate80050AC0,
        1,
        gp368WorkSlot,
        runtime);
    MakeSaveUiSubmit8001C5A8Table(
        kStageSelectExitLabelPos8005300C,
        contextWord1 != 0 ? kStageSelectExitLabelOnTable80053008
                          : kStageSelectExitLabelOffTable80053004,
        16u,
        languageIndex,
        0,
        gp368WorkSlot,
        runtime);
    MakeSaveUiSubmit80022CBC(
        238,
        188,
        contextWord1 != 0 ? kStageSelectExitBarOnTemplate800509C0
                          : kStageSelectExitBarOffTemplate800509B0,
        0,
        gp368WorkSlot,
        runtime);
    MakeSaveUiSubmit80022CBC(
        121,
        36,
        kSaveUiTitleTemplate80052320,
        2,
        gp368WorkSlot,
        runtime);
}

void PsxCall80022CBC_DualChoice(
    int16_t headerX,
    int16_t headerY,
    uint32_t headerTemplateAddr,
    uint32_t headerPositionBaseAddr,
    uint32_t headerTemplateBaseAddr,
    int32_t contextWord0,
    int32_t contextWord1,
    int32_t languageIndex,
    uint32_t gp368WorkSlot,
    PrPsxFastSpriteSubmitDirect::RuntimeState8003FA20* runtime) {
    MakeSaveUiSubmit80022CBC(
        headerX,
        headerY,
        headerTemplateAddr,
        1,
        gp368WorkSlot,
        runtime);
    MakeSaveUiSubmit8001C5A8Table(
        headerPositionBaseAddr,
        headerTemplateBaseAddr,
        8u,
        languageIndex,
        1,
        gp368WorkSlot,
        runtime);

    MakeSaveUiSubmit80022CBC(
        224,
        149,
        contextWord0 == 1 ? kSaveUiType4PromptTemplateOn800526A0
                          : kSaveUiType4PromptTemplateOff800526B0,
        1,
        gp368WorkSlot,
        runtime);

    if (contextWord1 == 1) {
        MakeSaveUiSubmit8001C5A8Table(kSaveUiChoice0Pos80053AB4,
                                      kSaveUiChoice0TemplateBTable80053AB0,
                                      16u,
                                      languageIndex,
                                      1,
                                      gp368WorkSlot,
                                      runtime);
        MakeSaveUiSubmit8001C5A8Table(kSaveUiChoice1Pos80053B04,
                                      kSaveUiChoice1TemplateATable80053AFC,
                                      16u,
                                      languageIndex,
                                      1,
                                      gp368WorkSlot,
                                      runtime);
        MakeSaveUiSubmit80022CBC(
            234,
            159,
            kSaveUiChoice0TemplateB800525A0,
            1,
            gp368WorkSlot,
            runtime);
        MakeSaveUiSubmit80022CBC(
            234,
            182,
            kSaveUiChoice1TemplateA800526C0,
            1,
            gp368WorkSlot,
            runtime);
    } else if (contextWord1 == 2) {
        MakeSaveUiSubmit8001C5A8Table(kSaveUiChoice0Pos80053AB4,
                                      kSaveUiChoice0TemplateATable80053AAC,
                                      16u,
                                      languageIndex,
                                      1,
                                      gp368WorkSlot,
                                      runtime);
        MakeSaveUiSubmit8001C5A8Table(kSaveUiChoice1Pos80053B04,
                                      kSaveUiChoice1TemplateBTable80053B00,
                                      16u,
                                      languageIndex,
                                      1,
                                      gp368WorkSlot,
                                      runtime);
        MakeSaveUiSubmit80022CBC(
            234,
            159,
            kSaveUiChoice0TemplateA80052590,
            1,
            gp368WorkSlot,
            runtime);
        MakeSaveUiSubmit80022CBC(
            234,
            182,
            kSaveUiChoice1TemplateB800526D0,
            1,
            gp368WorkSlot,
            runtime);
    } else {
        MakeSaveUiSubmit8001C5A8Table(kSaveUiChoice0Pos80053AB4,
                                      kSaveUiChoice0TemplateATable80053AAC,
                                      16u,
                                      languageIndex,
                                      1,
                                      gp368WorkSlot,
                                      runtime);
        MakeSaveUiSubmit8001C5A8Table(kSaveUiChoice1Pos80053B04,
                                      kSaveUiChoice1TemplateATable80053AFC,
                                      16u,
                                      languageIndex,
                                      1,
                                      gp368WorkSlot,
                                      runtime);
        MakeSaveUiSubmit80022CBC(
            234,
            159,
            kSaveUiChoice0TemplateA80052590,
            1,
            gp368WorkSlot,
            runtime);
        MakeSaveUiSubmit80022CBC(
            234,
            182,
            kSaveUiChoice1TemplateA800526C0,
            1,
            gp368WorkSlot,
            runtime);
    }

    MakeSaveUiSubmit80022CBC(
        121,
        36,
        kSaveUiTitleTemplate80052320,
        2,
        gp368WorkSlot,
        runtime);
}

PrStage1MovieTextDirect::Movie1TextCurrentGp872WorkCarrier
BuildSaveUiCardIoTextWorkCarrier80020A3C(uint32_t gp368WorkSlot) {
    PrStage1MovieTextDirect::Movie1TextCurrentGp872WorkCarrier out{};
    out.usesCurrentGp872DrawBuffer = true;
    out.workBasePsxAddr =
        PrStage1MovieTextDirect::kMovie1TextOtBufferBasePsxAddr;
    out.workStrideBytes =
        PrStage1MovieTextDirect::kMovie1TextOtBufferStrideBytes;
    out.gp872SlotKnown = true;
    out.gp872Slot = static_cast<uint8_t>(gp368WorkSlot & 1u);
    out.workAddrKnown = true;
    out.workAddr =
        WorkListAddr80087288(static_cast<uint32_t>(out.gp872Slot));
    out.workLastAddrOffset = 0x10u;
    out.drawOtagAddrKnown = true;
    out.drawOtagAddr = out.workAddr + out.workLastAddrOffset;
    return out;
}

const char* ResolveSaveUiCardIoPromptText80020A3C(int32_t msgType,
                                                  int32_t languageIndex) {
    static constexpr const char* kRemoveCardText8005326C[5] = {
        "Don't remove memory card.",
        "Memory Card nicht entfernen.",
        "Ne pas enlever la carte m\xE9moire.",
        "Non rimuovere la scheda di memoria.",
        "No extraigas la tarjeta de memoria.",
    };
    static constexpr const char* kPleaseWaitText80053280[5] = {
        "Please wait a minute.",
        "Bitte eine Minute warten.",
        "Attends un moment, s'il te pla\xEEt.",
        "Aspetta un attimo.",
        "Espera un momento",
    };

    const int32_t lang = ClampLanguageIndex80022CBC(languageIndex);
    if (msgType == 1) {
        return kPleaseWaitText80053280[lang];
    }
    if (msgType == 2 && lang == 0) {
        return "Now saving.";
    }
    return kRemoveCardText8005326C[lang];
}

PrStage1MovieTextDirect::Movie1TextDrawCommand
BuildSaveUiCardIoTextCommand8001C6E0(const char* text) {
    PrStage1MovieTextDirect::Movie1TextDrawCommand command{};
    command.kind =
        PrStage1MovieTextDirect::Movie1TextDrawCommandKind::
            SubmitTextFastSpriteSequenceSub8001B954;
    command.psxFunctionAddr =
        PrStage1MovieTextDirect::kMovie1TextGlyphLoopFunctionSub8001B954;
    command.fastSpriteSourceKind =
        PrPsxFastSpriteSubmitDirect::FastSpriteSubmitSourceKind8003FA20::
            Stage1EventFrameSaveUi;
    command.usesCurrentGp872DrawBuffer = true;
    command.otBufferBasePsxAddr =
        PrStage1MovieTextDirect::kMovie1TextOtBufferBasePsxAddr;
    command.otBufferStrideBytes =
        PrStage1MovieTextDirect::kMovie1TextOtBufferStrideBytes;
    command.x = 30;
    command.y = 121;
    command.z = 0;
    command.scale = 480;
    command.textPtr = text;
    command.centeredLineWidth =
        PrStage1MovieTextDirect::kMovie1TextGlyphCenteredBodyWidthSub8001B954;
    return command;
}

bool PsxCall80020A3C_SaveUiCardIoPrompt(
    EventFrameState8001E750& state,
    int32_t msgType,
    int32_t languageIndex,
    EventFrameState8001E750::FastSpritePageRuntime8003FA20& pageRuntime) {
    const char* text =
        ResolveSaveUiCardIoPromptText80020A3C(msgType, languageIndex);
    if (text == nullptr || text[0] == '\0') {
        return true;
    }

    const PrStage1MovieTextDirect::Movie1TextCurrentGp872WorkCarrier
        currentWork =
            BuildSaveUiCardIoTextWorkCarrier80020A3C(state.gp368WorkSlot);
    const PrStage1MovieTextDirect::Movie1TextDrawCommand textCommand =
        BuildSaveUiCardIoTextCommand8001C6E0(text);
    const PrStage1MovieTextDirect::Movie1TextFastSpriteSequenceSub8001B954
        textSequence =
            PrStage1MovieTextDirect::BuildTextFastSpriteSequenceSub8001B954(
                textCommand,
                PrStage1MovieTextDirect::Movie1TextGlyphMetricTablesSub8001B954{},
                currentWork);
    if (!textSequence.valid || textSequence.lineCount == 0u ||
        !textSequence.lines[0].widthKnown) {
        return true;
    }

    const PrStage1MovieTextDirect::Movie1TextFastSpriteSequenceApplyResultSub8001B954
        textApply =
            PrStage1MovieTextDirect::ApplyTextFastSpriteSequenceSub8001B954(
                textSequence,
                pageRuntime.runtime);
    if (!textApply.valid || textApply.appliedSubmitCount == 0u) {
        return true;
    }

    const PrPsxGraphOwnerDirect::PsxFastSpriteRuntimeCommitResult8003FA20
        textCommit =
            PrPsxGraphOwnerDirect::CommitRuntimeState8003FA20ToMainPageWork(
                state.graph,
                pageRuntime.slot8004019C,
                pageRuntime.runtime);
    if (!(textCommit.committed && textCommit.allocatorUpdated &&
          textCommit.otSlotMirrorUpdated &&
          textCommit.packetWriteMirrorUpdated)) {
        return true;
    }

    PrPsxGraphOwnerDirect::PsxGraphPageWorkList8001E374& pageWork =
        state.graph.mainPageWorkLists80087288[pageRuntime.slot8004019C & 1u];
    const int16_t widthPlus8 =
        static_cast<int16_t>(textSequence.lines[0].widthPx + 8u);
    const int16_t left =
        static_cast<int16_t>((320 - static_cast<int32_t>(widthPlus8)) / 2);
    const int16_t right = static_cast<int16_t>(left + widthPlus8);

    PsxCall8001B6C4_BoxFill(
        state.graph, left, 123, 8, 9, 0x400F0F0Fu, 0, pageWork.work);
    PsxCall8001B6C4_BoxFill(
        state.graph,
        static_cast<int16_t>(left + 8),
        115,
        static_cast<int16_t>(widthPlus8 - 8),
        25,
        0x400F0F0Fu,
        0,
        pageWork.work);
    PsxCall8001B6C4_BoxFill(
        state.graph, right, 123, 8, 9, 0x400F0F0Fu, 0, pageWork.work);

    pageRuntime.runtime =
        BuildFrameFastSpriteRuntime8003FA20(state.graph, pageWork);
    MakeSaveUiSubmit80022CBC(
        left, 115, 0x80050900u, 0, state.gp368WorkSlot, &pageRuntime.runtime);
    MakeSaveUiSubmit80022CBC(
        left, 132, 0x800508F0u, 0, state.gp368WorkSlot, &pageRuntime.runtime);
    MakeSaveUiSubmit80022CBC(
        right, 115, 0x800508E0u, 0, state.gp368WorkSlot, &pageRuntime.runtime);
    MakeSaveUiSubmit80022CBC(
        right, 132, 0x800508D0u, 0, state.gp368WorkSlot, &pageRuntime.runtime);
    return false;
}

bool BuildStageSelectLocal8001B4E0(
    int16_t screenX,
    int16_t screenY,
    const StageSelectSpriteTemplate8001B4E0& tpl,
    int16_t uOffset,
    int16_t width,
    int32_t state,
    PrPsxFastSpriteSubmitDirect::FastSpriteLocalFields8003FA20& outLocal) {
    outLocal = PrPsxFastSpriteSubmitDirect::FastSpriteLocalFields8003FA20{};
    if (!tpl.known) {
        return false;
    }
    outLocal.attr_00 = tpl.attr_00;
    outLocal.x_04 = static_cast<int16_t>(screenX - 160);
    outLocal.y_06 =
            static_cast<uint16_t>(static_cast<int16_t>(screenY - 120));
    outLocal.width_08 = static_cast<uint16_t>(width);
    outLocal.height_0A = tpl.height_0A;
    outLocal.tpage_0C =
            ComputeSub80043DF4WordFromOffset(tpl.texX_04,
                                              tpl.texY_06,
                                              uOffset);
    outLocal.u_0E =
            static_cast<uint8_t>(static_cast<uint16_t>(4u * tpl.texX_04 +
                                                       static_cast<uint16_t>(
                                                           uOffset)));
    outLocal.v_0F = static_cast<uint8_t>(tpl.texY_06);
    outLocal.clutX_10 = static_cast<int16_t>(tpl.clutX_0C);
    outLocal.clutY_12 =
            static_cast<int16_t>(static_cast<int32_t>(tpl.clutY_0E) + state);
    return true;
}

void MakeStageSelectSliceSubmit8001B654(
    int16_t screenX,
    int16_t screenY,
    uint32_t templateAddr,
    int16_t uOffset,
    int16_t width,
    int32_t state,
    uint16_t priority,
    bool workListKnown,
    uint32_t workListAddr,
    PrPsxFastSpriteSubmitDirect::RuntimeState8003FA20* runtime) {
    (void)templateAddr;
    (void)workListAddr;
    PrPsxFastSpriteSubmitDirect::FastSpriteLocalFields8003FA20 local{};
    const bool localKnown =
        BuildStageSelectLocal8001B4E0(screenX,
                                      screenY,
                                      kStageSelectDotTemplate80051BF0,
                                      uOffset,
                                      width,
                                      state,
                                      local);
    if (localKnown && workListKnown && runtime != nullptr) {
        PrPsxFastSpriteSubmitDirect::GsSortFastSpriteInput8003FA20
            submitInput =
                PrPsxFastSpriteSubmitDirect::BuildInputFromRuntime8003FA20(
                    *runtime,
                    local,
                    priority);
        TagEventFrameFastSpriteInput8003FA20(
            submitInput,
            PrPsxFastSpriteSubmitDirect::FastSpriteSubmitSourceKind8003FA20::
                Stage1EventFrameStageSelect,
            0x8001B654u,
            0x8001B6A4u,
            0x8001C604u);
        const PrPsxFastSpriteSubmitDirect::GsSortFastSpritePartialResult8003FA20
            partialSubmit =
                PrPsxFastSpriteSubmitDirect::PredictGsSortFastSpritePartial8003FA20(
                    submitInput,
                    false);
        PrPsxFastSpriteSubmitDirect::ApplyRuntimeUpdate8003FA20(
            *runtime,
            partialSubmit);
    }
}

void MakeStageSelectSubmit8001C604(int16_t a0_screenX,
                                   int16_t a1_screenY,
                                   uint32_t a2_templateAddr,
                                   int16_t a3_uOffset,
                                   int16_t stackArg10_width,
                                   int32_t stackArg14_state,
                                   uint16_t stackArg18_priority,
                                   uint32_t gp368WorkSlot,
                                   PrPsxFastSpriteSubmitDirect::RuntimeState8003FA20* runtime) {
    const bool workListKnown = true;
    const uint32_t stackArg1C_workListAddr =
        WorkListAddr80087288(gp368WorkSlot & 1u);
    MakeStageSelectSliceSubmit8001B654(a0_screenX,
                                       a1_screenY,
                                       a2_templateAddr,
                                       a3_uOffset,
                                       stackArg10_width,
                                       stackArg14_state,
                                       stackArg18_priority,
                                       workListKnown,
                                       stackArg1C_workListAddr,
                                       runtime);
}

void PsxCall80020568_StageSelectDraw(
    const StageSelectDrawInput80020568& input,
    uint32_t gp368WorkSlot,
    PrPsxFastSpriteSubmitDirect::RuntimeState8003FA20* runtime) {
    int16_t lang = input.languageIndex_800916D8;
    if (lang < 0) {
        lang = 0;
    } else if (lang > 4) {
        lang = 4;
    }
    const uint32_t langU = static_cast<uint32_t>(lang);

    MakeStageSelectSubmit8001C5A8(
        kStageSelectTitleBgPos80053248 + 8u * langU,
        kStageSelectTitleBgTemplateTable80053244 + 8u * langU,
        0,
        0,
        gp368WorkSlot,
        runtime);
    MakeStageSelectSubmit8001C550(32,
                                  33,
                                  kStageSelectTitleTemplate80051AA0,
                                  0,
                                  gp368WorkSlot,
                                  runtime);

    int32_t enabledState[kStageSelectEntryCount80020568]{};
    int32_t rawStatus[kStageSelectEntryCount80020568]{};
    int32_t localDotState[kStageSelectEntryCount80020568]{};
    int32_t localSliceState[kStageSelectEntryCount80020568]{};
    int32_t localNameState[kStageSelectEntryCount80020568]{};

    for (uint32_t i = 0; i < kStageSelectEntryCount80020568; ++i) {
        rawStatus[i] = input.status_0E[i];
        if (rawStatus[i] != 0) {
            enabledState[i] = 1;
            localSliceState[i] = 0;
            localNameState[i] = 0;
        } else {
            enabledState[i] = 0;
            localSliceState[i] = 2;
            localNameState[i] = 1;
        }
        localDotState[i] = 0;
    }
    if (input.cursor_08 == 7) {
        if (input.selected_04 != 0) {
            MakeStageSelectSubmit8001C550(64,
                                          100,
                                          kStageSelectBonusOnTemplate800519F0,
                                          0,
                                          gp368WorkSlot,
                                          runtime);
        } else {
            MakeStageSelectSubmit8001C550(63,
                                          98,
                                          kStageSelectBonusOffTemplate800519E0,
                                          0,
                                          gp368WorkSlot,
                                          runtime);
        }
        MakeStageSelectSubmit8001C550(
            66,
            103,
            kStageSelectBonusLabelTemplate80051A00,
            0,
            gp368WorkSlot,
            runtime);
    } else {
        if (input.bonusStatus_1A != 0) {
            MakeStageSelectSubmit8001C550(
                63,
                98,
                kStageSelectBonusEnabledTemplate80051A10,
                0,
                gp368WorkSlot,
                runtime);
            MakeStageSelectSubmit8001C550(
                66,
                103,
                kStageSelectBonusLabelTemplate80051A00,
                0,
                gp368WorkSlot,
                runtime);
        }
        if (input.cursor_08 < 7) {
            const int32_t selectedIndex = input.cursor_08 - 1;
            if (selectedIndex >= 0 &&
                selectedIndex <
                    static_cast<int32_t>(kStageSelectEntryCount80020568)) {
                localSliceState[selectedIndex] = 1;
                enabledState[selectedIndex] = 2;
                localDotState[selectedIndex] = 1;
            }
        }
    }

    for (uint32_t i = 0; i < kStageSelectEntryCount80020568; ++i) {
        MakeStageSelectSubmit8001C5A8(
            kStageSelectTopTextPos8005308C + 24u * langU + 4u * i,
            kStageSelectTopTextTemplateTable80053050 +
                (3u * langU + static_cast<uint32_t>(enabledState[i])) * 4u,
            static_cast<uint32_t>(enabledState[i]),
            0,
            gp368WorkSlot,
            runtime);
        const StageSelectPoint80020568& slicePoint =
            kStageSelectDotPos80053104[langU][i];
        const StageSelectDotArgs80020568& sliceArgs =
            kStageSelectDotArgs800531D0[i];
        MakeStageSelectSubmit8001C604(slicePoint.x,
                                      slicePoint.y,
                                      kStageSelectSliceTemplate80051BF0,
                                      sliceArgs.uOffset,
                                      sliceArgs.width,
                                      localSliceState[i],
                                      0,
                                      gp368WorkSlot,
                                      runtime);
        MakeStageSelectSubmit8001C5A8(
            kStageSelectBoxPos8005317C + 4u * i,
            kStageSelectBoxTemplateTable800531C4 +
                static_cast<uint32_t>(enabledState[i]) * 4u,
            static_cast<uint32_t>(enabledState[i]),
            0,
            gp368WorkSlot,
            runtime);
        MakeStageSelectSubmit8001C5A8(
            kStageSelectNamePos80053194 + 4u * i,
            kStageSelectNameTemplateTable80053214 +
                (2u * i + static_cast<uint32_t>(localNameState[i])) * 4u,
            static_cast<uint32_t>(localNameState[i]),
            0,
            gp368WorkSlot,
            runtime);
        MakeStageSelectSubmit8001C5A8(
            kStageSelectBadgePos800531AC + 4u * i,
            kStageSelectBadgeATemplateTable800531F4 +
                (2u * static_cast<uint32_t>(rawStatus[i]) +
                 static_cast<uint32_t>(localDotState[i])) *
                    4u,
            2u * static_cast<uint32_t>(rawStatus[i]) +
                static_cast<uint32_t>(localDotState[i]),
            0,
            gp368WorkSlot,
            runtime);
        MakeStageSelectSubmit8001C5A8(
            kStageSelectBadgePos800531AC + 4u * i,
            kStageSelectBadgeBTemplateTable800531E8 +
                static_cast<uint32_t>(enabledState[i]) * 4u,
            static_cast<uint32_t>(enabledState[i]),
            0,
            gp368WorkSlot,
            runtime);
    }

    if (input.cursor_08 != 8) {
        MakeStageSelectSubmit8001C5A8(
            kStageSelectExitLabelPos8005300C + 16u * langU,
            kStageSelectExitLabelNormalTable80053000 + 16u * langU,
            0,
            0,
            gp368WorkSlot,
            runtime);
        MakeStageSelectSubmit8001C5A8(
            kStageSelectExitBarPos80052FFC,
            kStageSelectExitBarNormalTemplate800509A0,
            0,
            0,
            gp368WorkSlot,
            runtime);
        MakeStageSelectSubmit8001C550(
            231,
            179,
            kStageSelectExitFrameNormalGpOffset80020568,
            1,
            gp368WorkSlot,
            runtime);
        return;
    }

    if (input.selected_04 != 0) {
        MakeStageSelectSubmit8001C5A8(
            kStageSelectExitLabelPos8005300C + 16u * langU,
            kStageSelectExitLabelOnTable80053008 + 16u * langU,
            0,
            0,
            gp368WorkSlot,
            runtime);
        MakeStageSelectSubmit8001C5A8(
            kStageSelectExitBarPos80052FFC,
            kStageSelectExitBarOnTemplate800509C0,
            0,
            0,
            gp368WorkSlot,
            runtime);
    } else {
        MakeStageSelectSubmit8001C5A8(
            kStageSelectExitLabelPos8005300C + 16u * langU,
            kStageSelectExitLabelOffTable80053004 + 16u * langU,
            0,
            0,
            gp368WorkSlot,
            runtime);
        MakeStageSelectSubmit8001C5A8(
            kStageSelectExitBarPos80052FFC,
            kStageSelectExitBarOffTemplate800509B0,
            0,
            0,
            gp368WorkSlot,
            runtime);
    }
    MakeStageSelectSubmit8001C550(
        231,
        179,
        input.blinkFlag_00 != 0
            ? kStageSelectExitFrameOnGpOffset80020568
            : kStageSelectExitFrameNormalGpOffset80020568,
        1,
        gp368WorkSlot,
        runtime);
}

void PsxCall8001E750_Event4(
    EventFrameState8001E750& state,
    int32_t promptCtx0) {
    BeginDrawWrapper8001E750(state, 4);
    state.stage1Event4PrepareUnderlayValid8001B120 = false;

    if (state.gp38CEvent4State == 0u) {
        const uint32_t slot = state.gp368WorkSlot & 1u;
        PsxCall800203D4_PromptSprites(
            state.graph,
            slot,
            state.graph.mainPageWorkLists80087288[slot],
            promptCtx0);
    } else if (state.gp38CEvent4State == 1u) {
        const uint32_t slot = state.gp368WorkSlot & 1u;
        PrPsxGraphOwnerDirect::PsxGraphPageWorkList8001E374& pageWork =
            state.graph.mainPageWorkLists80087288[slot];
        PsxCall8001B6C4_BoxFill(state.graph,
                                0,
                                0,
                                320,
                                240,
                                0x400F0F0Fu,
                                0,
                                pageWork.work);
        LatchStage1Event4MoveImageBoxFill8001B120(state, pageWork);
        state.stage1Event4PrepareUnderlayValid8001B120 = true;
        ++state.gp38CEvent4State;
    } else {
        PsxCall8001B120_ResetMoveImage(state.graph, 0);
        state.stage1Event4MoveImageUnderlayValid8001B120 = true;
        state.gp38CEvent4State = 0u;
    }
}

bool PsxCall8001E750_SaveUiEventFrame(
    EventFrameState8001E750& state,
    int32_t eventId,
    int32_t contextWord0,
    int32_t contextWord1,
    int32_t contextWord2,
    int32_t languageIndex) {
    const int32_t type = [eventId]() -> int32_t {
        switch (eventId) {
        case 12:
            return 1;
        case 13:
            return 2;
        case 14:
            return 3;
        case 11:
            return 4;
        case 15:
            return 5;
        case 18:
            return 6;
        case 19:
            return 7;
        default:
            return 0;
        }
    }();
    if (type == 0) {
        return true;
    }

    BeginDrawWrapper8001E750(state, eventId);
    EventFrameState8001E750::FastSpritePageRuntime8003FA20& pageRuntime =
        state.fastSpritePageRuntime8003FA20;
    PsxCall8001D74C_EventBackdrop(3, pageRuntime.runtime);
    bool frameCloseBlocked = false;
    switch (type) {
    case 1:
        PsxCall80022CBC_SingleButton(kSaveUiType1HeaderPos80053A0C,
                                     kSaveUiType1HeaderTemplateTable80053A08,
                                     contextWord0,
                                     contextWord1,
                                     languageIndex,
                                     state.gp368WorkSlot,
                                     &pageRuntime.runtime);
        break;
    case 2:
        PsxCall80022CBC_DualChoice(34,
                                   54,
                                   0x80052340u,
                                   kSaveUiType2HeaderPos80053994,
                                   kSaveUiType2HeaderTemplateTable80053990,
                                   contextWord0,
                                   contextWord1,
                                   languageIndex,
                                   state.gp368WorkSlot,
                                   &pageRuntime.runtime);
        break;
    case 3:
        PsxCall80022CBC_SingleButton(kSaveUiType3HeaderPos800539BC,
                                     kSaveUiType3HeaderTemplateTable800539B8,
                                     contextWord0,
                                     contextWord1,
                                     languageIndex,
                                     state.gp368WorkSlot,
                                     &pageRuntime.runtime);
        break;
    case 4:
        PsxCall80022CBC_DualChoice(28,
                                   56,
                                   kSaveUiType4HeaderTemplate80052350,
                                   kSaveUiType4HeaderPos80053A34,
                                   kSaveUiType4HeaderTemplateTable80053A30,
                                   contextWord0,
                                   contextWord1,
                                   languageIndex,
                                   state.gp368WorkSlot,
                                   &pageRuntime.runtime);
        break;
    case 5:
        PsxCall80022CBC_SingleButton(kSaveUiType5HeaderPos80053A5C,
                                     kSaveUiType5HeaderTemplateTable80053A58,
                                     contextWord0,
                                     contextWord1,
                                     languageIndex,
                                     state.gp368WorkSlot,
                                     &pageRuntime.runtime);
        break;
    case 6:
        PsxCall80022CBC_SingleButton(kSaveUiType6HeaderPos80053A84,
                                     kSaveUiType6HeaderTemplateTable80053A80,
                                     contextWord0,
                                     contextWord1,
                                     languageIndex,
                                     state.gp368WorkSlot,
                                     &pageRuntime.runtime);
        break;
    case 7:
        PsxCall80022CBC_DualChoice(28,
                                   56,
                                   kSaveUiType4HeaderTemplate80052350,
                                   kSaveUiType7HeaderPos800539E4,
                                   kSaveUiType7HeaderTemplateTable800539E0,
                                   contextWord0,
                                   contextWord1,
                                   languageIndex,
                                   state.gp368WorkSlot,
                                   &pageRuntime.runtime);
        if (contextWord2 != 0) {
            frameCloseBlocked =
                PsxCall80020A3C_SaveUiCardIoPrompt(state,
                                                   2,
                                                   languageIndex,
                                                   pageRuntime);
        }
        break;
    default:
        return true;
    }
    if (frameCloseBlocked) {
        return true;
    }
    pageRuntime.dirty = true;
    const PrPsxGraphOwnerDirect::PsxFastSpriteRuntimeCommitResult8003FA20
        eventFrameFastSpriteRuntimeCommit8003FA20 =
        PrPsxGraphOwnerDirect::CommitRuntimeState8003FA20ToMainPageWork(
            state.graph,
            pageRuntime.slot8004019C,
            pageRuntime.runtime);
    return !(eventFrameFastSpriteRuntimeCommit8003FA20.committed &&
             eventFrameFastSpriteRuntimeCommit8003FA20.allocatorUpdated &&
             eventFrameFastSpriteRuntimeCommit8003FA20.otSlotMirrorUpdated &&
             eventFrameFastSpriteRuntimeCommit8003FA20.packetWriteMirrorUpdated);
}

bool PsxCall8001E750_Event2StageSelectFrameCloseBlocked(
    EventFrameState8001E750& state,
    const StageSelectDrawInput80020568* input) {
    BeginDrawWrapper8001E750(state, 2);
    EventFrameState8001E750::FastSpritePageRuntime8003FA20& pageRuntime =
        state.fastSpritePageRuntime8003FA20;
    PsxCall8001D74C_EventBackdrop(3, pageRuntime.runtime);
    if (input != nullptr) {
        (void)PsxCall80020568_StageSelectDraw(*input,
                                              state.gp368WorkSlot,
                                              &pageRuntime.runtime);
    }
    pageRuntime.dirty = true;
    const PrPsxGraphOwnerDirect::PsxFastSpriteRuntimeCommitResult8003FA20
        event2FastSpriteRuntimeCommit8003FA20 =
        PrPsxGraphOwnerDirect::CommitRuntimeState8003FA20ToMainPageWork(
            state.graph,
            pageRuntime.slot8004019C,
            pageRuntime.runtime);
    const bool graphPageCommitGap =
        !(event2FastSpriteRuntimeCommit8003FA20.committed &&
          event2FastSpriteRuntimeCommit8003FA20.allocatorUpdated &&
          event2FastSpriteRuntimeCommit8003FA20.otSlotMirrorUpdated &&
          event2FastSpriteRuntimeCommit8003FA20.packetWriteMirrorUpdated);
    return graphPageCommitGap;
}

void PsxCall80035560_WaitFrame(EventFrameState8001E750& state,
                               int32_t arg0) {
    if (!state.initialized) {
        ResetEventFrameState8003FB9C(state, 320u, 240u);
    }
    EventFrameState8001E750::WaitFrameState80035560& wait =
        state.waitFrame80035560;
    ++wait.requestCount;
    wait.lastArg = arg0;
    wait.pendingVblanks += arg0 <= 0 ? 1 : arg0;
    wait.hostVblankHalPending = true;
}

void PsxConsume80035560_WaitFrameHostVblank(
    EventFrameState8001E750& state,
    int32_t consumedVblanks) {
    if (!state.initialized) {
        ResetEventFrameState8003FB9C(state, 320u, 240u);
    }
    EventFrameState8001E750::WaitFrameState80035560& wait =
        state.waitFrame80035560;
    if (!wait.hostVblankHalPending) {
        return;
    }
    if (consumedVblanks <= 0) {
        consumedVblanks = 1;
    }
    wait.pendingVblanks -= consumedVblanks;
    if (wait.pendingVblanks <= 0) {
        wait.pendingVblanks = 0;
        wait.hostVblankHalPending = false;
    }
}

void PsxCall8001EA00_EndFrame(EventFrameState8001E750& state,
                              int32_t eventId) {
    (void)eventId;
    if (!state.initialized) {
        ResetEventFrameState8003FB9C(state, 320u, 240u);
    }

    const uint32_t submitSlotFromGp368BeforeFlip = state.gp368WorkSlot;
    (void)PrPsxGraphOwnerDirect::PsxCall80040370_FlipGraph(state.graph);
    const uint32_t submitSlot = submitSlotFromGp368BeforeFlip & 1u;
    PsxCall80040CA4_SubmitWorkList(
        WorkListAddr80087288(submitSlot),
        submitSlot,
        state.graph.mainPageWorkLists80087288[submitSlot].work);
}

void PsxCall800436F0_TextFlush(
    EventFrameState8001E750& state,
    int32_t arg0) {
    if (!state.initialized) {
        ResetEventFrameState8003FB9C(state, 320u, 240u);
    }

    if (!state.textFlush.bankKnown) {
        return;
    }

    int32_t selectedSlot = arg0;
    if (arg0 < 0 || arg0 >= state.textFlush.slotLimit8005CCDC) {
        selectedSlot = state.textFlush.currentSlot8005CCE0;
        if (state.textFlush.currentSlotActiveKnown &&
            !state.textFlush.currentSlotActive8005CB80) {
            return;
        }
    }
    if (selectedSlot < 0 ||
        selectedSlot >= static_cast<int32_t>(kTextRecordCount800436F0)) {
        return;
    }

    EventFrameState8001E750::TextFlushRecord800436F0& record =
        state.textFlush.records[selectedSlot];
    if (record.text_24 == nullptr || record.text_24[0] == '\0') {
        return;
    }

    const uint32_t listHeadAddr = record.recordAddr + 0x10u;
    record.listHead_10 =
        PsxCall800440B8_TextInitListHead(listHeadAddr, record.listHead_10);

    int16_t x = record.x_08;
    int16_t y = record.y_0A;
    int32_t maxObservedX = 0;
    int32_t budget = record.budget_1C;
    uint32_t packetCursor = record.packetCursor_20;
    uint8_t r = 0x80u;
    uint8_t g = 0x80u;
    uint8_t b = 0x80u;
    const int16_t rightBound =
        static_cast<int16_t>(record.x_08 + record.w_0C);
    const int16_t bottomBound =
        static_cast<int16_t>(record.y_0A + record.h_0E);

    const char* source = record.text_24;
    uint32_t glyphPacketCount = 0;
    while (source[0] != '\0' && budget != 0) {
        bool newline = false;
        const uint8_t ch = static_cast<uint8_t>(source[0]);
        if (ch == static_cast<uint8_t>(' ')) {
            x = static_cast<int16_t>(x + 8);
        } else if (ch == 9u) {
            x = static_cast<int16_t>(x + 0x20);
        } else if (ch == 10u) {
            newline = true;
        } else if (ch == static_cast<uint8_t>('~')) {
            ++source;
            if (source[0] == 'c') {
                ++source;
                r = static_cast<uint8_t>(
                    (static_cast<uint8_t>(source[0]) - 0x30u) << 4);
                ++source;
                g = static_cast<uint8_t>(
                    (static_cast<uint8_t>(source[0]) - 0x30u) << 4);
                ++source;
                b = static_cast<uint8_t>(
                    (static_cast<uint8_t>(source[0]) - 0x30u) << 4);
            }
        } else if (glyphPacketCount < kTextMaxGlyphPackets800436F0) {
            ++glyphPacketCount;
            uint32_t packetWord0 = 0;
            const int32_t glyphIndex = GlyphIndex800436F0(ch);
            (void)glyphIndex;
            (void)r;
            (void)g;
            (void)b;
            PsxCall8004401C_TextLinkNode(listHeadAddr,
                                         record.listHead_10,
                                         packetCursor,
                                         packetWord0);
            packetCursor += kTextGlyphPacketStride800436F0;
            x = static_cast<int16_t>(x + 8);
        }

        if (x >= rightBound) {
            if (record.measureMode_2C == 0) {
                newline = true;
            }
        }
        if (newline) {
            if (maxObservedX < x) {
                maxObservedX = x;
            }
            y = static_cast<int16_t>(y + 8);
            x = record.x_08;
            if (y >= bottomBound) {
                break;
            }
        }

        ++source;
        --budget;
    }

    (void)budget;
    (void)packetCursor;

    if (record.control_07 != 0u) {
        PsxCall8004401C_TextLinkNode(listHeadAddr,
                                     record.listHead_10,
                                     record.recordAddr,
                                     record.word0);
    }
    if (record.measureMode_2C != 0) {
        record.w_0C = static_cast<int16_t>(maxObservedX - record.x_08);
        record.h_0E = static_cast<int16_t>(y - record.y_0A + 8);
    }

    PsxCall800450A0_DrawOtag(listHeadAddr);
    record.word_28 = 0;
    record.textStorage_24[0] = '\0';
}

void PsxCall80026B94_EventFrameTail(
    EventFrameState8001E750& state,
    int32_t eventId,
    int32_t event4PromptCtx0,
    const StageSelectDrawInput80020568* event2StageSelectInput,
    const StageClearTextInput80026B94* event2StageClearInput) {
    bool frameCloseBlocked = false;

    if (eventId == 4) {
        (void)PsxCall8001E750_Event4(state, event4PromptCtx0);
    } else if (eventId == 2) {
        frameCloseBlocked =
            PsxCall8001E750_Event2StageSelectFrameCloseBlocked(
                state,
                event2StageSelectInput);
    } else {
        frameCloseBlocked = true;
    }

    if (event2StageClearInput != nullptr) {
        (void)PsxProduce80026B94_Event2StageClearText(
            state,
            *event2StageClearInput);
    }
    if (frameCloseBlocked) {
        return;
    }

    PsxCall80035560_WaitFrame(state, 0);
    PsxCall8001EA00_EndFrame(state, eventId);
    PsxCall800436F0_TextFlush(state, -1);
}

void BuildEventFrameBoxFillPacketCommands8003EE84(
    const EventFrameState8001E750& state,
    BoxFillPacketCommandConsumer8003EE84 consumeCommand,
    void* commandUserData) {
    if (consumeCommand == nullptr || !state.initialized ||
        !state.graph.mainPageWorkLists80087288Initialized) {
        return;
    }

    const uint8_t pageIndex = static_cast<uint8_t>(state.gp368WorkSlot & 1u);
    const PrPsxGraphOwnerDirect::PsxGraphPageWorkList8001E374& pageWork =
        state.graph.mainPageWorkLists80087288[pageIndex];
    if (!pageWork.work.packetWriteMirrorKnown) {
        return;
    }

    std::array<const PrPsxFastSpriteSubmitDirect::RuntimePacketWrite8003FA20*,
               PrPsxFastSpriteSubmitDirect::
                   kGsSortFastSpriteRuntimePacketWriteCapacity8003FA20>
        validWrites{};
    uint32_t validCount = 0;
    for (const PrPsxFastSpriteSubmitDirect::RuntimePacketWrite8003FA20& write :
         pageWork.work.packetWriteMirror) {
        if (!write.valid) {
            continue;
        }
        if (validCount < validWrites.size()) {
            validWrites[validCount++] = &write;
        }
    }

    uint32_t packetMirrorOrder = 1u;
    for (uint32_t i = validCount; i > 0; --i) {
        const BoxFillPacketCommand8003EE84 command =
            BuildBoxFillPacketCommand8003EE84(*validWrites[i - 1],
                                              packetMirrorOrder++);
        if (command.valid) {
            consumeCommand(command, commandUserData);
        }
    }
}

}  // namespace PrPsxEventFrameDirect
