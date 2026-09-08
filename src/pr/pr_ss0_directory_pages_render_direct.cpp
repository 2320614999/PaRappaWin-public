#include "pr_ss0_directory_pages_render_direct.h"

#include "pr_psx_graph_owner_direct.h"
#include "pr_psx_gs_sprite_submit_direct.h"
#include "pr_psx_text_glyph_metrics_direct.h"

#include <cstring>

namespace PrSS0DirectoryPagesRenderDirect {
namespace {

struct StaticTemplateFamily8001B25C {
    uint32_t firstAddress = 0;
    uint32_t attr = 0;
    uint16_t texX = 0;
    uint16_t texY = 0;
    uint16_t width = 0;
    uint16_t height = 0;
    uint16_t clutX = 0;
    uint16_t firstClutY = 0;
    uint8_t stateCount = 0;
};

struct StaticLanguageSpriteFamily80021E60 {
    StaticTemplateFamily8001B25C templates{};
    int16_t x = 0;
    int16_t y = 0;
};

struct StaticCardGridTemplate80020F94 {
    uint32_t address = 0;
    uint32_t attr = 0;
    uint16_t texX = 0;
    uint16_t texY = 0;
    uint16_t width = 0;
    uint16_t height = 0;
    uint16_t clutX = 0;
    uint16_t clutY = 0;
};

struct PositionedCardGridTemplate80020F94 {
    uint32_t address = 0;
    int16_t x = 0;
    int16_t y = 0;
};

struct StaticCardGridLanguageRecord80020F94 {
    int32_t eventId = 0;
    int32_t language = 0;
    PositionedCardGridTemplate80020F94 text[5]{};
};

constexpr uint32_t kMainDirectoryRawTextureAttr80021E60 = 0x50000040u;

constexpr StaticTemplateFamily8001B25C kMainDirectoryFixedFamilies[] = {
    {0x80050AC0u, kMainDirectoryRawTextureAttr80021E60, 384u, 0u, 64u, 37u, 960u, 0u, 2u},
    {0x80050EF0u, kMainDirectoryRawTextureAttr80021E60, 332u, 0u, 68u, 45u, 960u, 8u, 3u},
    {0x80051010u, kMainDirectoryRawTextureAttr80021E60, 400u, 0u, 88u, 68u, 960u, 0u, 2u},
    {0x80051030u, kMainDirectoryRawTextureAttr80021E60, 349u, 0u, 84u, 42u, 960u, 14u, 3u},
    {0x80051150u, kMainDirectoryRawTextureAttr80021E60, 422u, 0u, 100u, 61u, 960u, 0u, 2u},
    {0x80051170u, kMainDirectoryRawTextureAttr80021E60, 320u, 45u, 68u, 22u, 960u, 20u, 3u},
    {0x80051290u, kMainDirectoryRawTextureAttr80021E60, 384u, 68u, 88u, 67u, 960u, 0u, 2u},
    {0x800512B0u, kMainDirectoryRawTextureAttr80021E60, 320u, 67u, 68u, 22u, 960u, 26u, 3u},
    {0x800513D0u, kMainDirectoryRawTextureAttr80021E60, 337u, 45u, 72u, 55u, 960u, 32u, 3u},
    {0x800514F0u, kMainDirectoryRawTextureAttr80021E60, 406u, 68u, 68u, 73u, 960u, 0u, 2u},
    {0x80051510u, kMainDirectoryRawTextureAttr80021E60, 355u, 45u, 64u, 43u, 960u, 38u, 3u},
    {0x80051630u, kMainDirectoryRawTextureAttr80021E60, 423u, 68u, 68u, 66u, 960u, 0u, 2u},
    {0x80051650u, kMainDirectoryRawTextureAttr80021E60, 320u, 100u, 68u, 43u, 960u, 44u, 3u},
    {0x80051770u, kMainDirectoryRawTextureAttr80021E60, 384u, 141u, 64u, 66u, 960u, 0u, 2u},
    {0x80051790u, kMainDirectoryRawTextureAttr80021E60, 337u, 100u, 72u, 55u, 960u, 50u, 3u},
    {0x800518B0u, kMainDirectoryRawTextureAttr80021E60, 400u, 141u, 72u, 72u, 960u, 0u, 2u},
    {0x800509A0u, kMainDirectoryRawTextureAttr80021E60, 320u, 0u, 48u, 17u, 960u, 2u, 3u},
};

constexpr StaticTemplateFamily8001B25C kOptionsFixedFamilies80021910[] = {
    {0x80050AE0u, kMainDirectoryRawTextureAttr80021E60, 482u, 0u, 72u, 49u, 976u, 0u, 1u},
    {0x80051170u, kMainDirectoryRawTextureAttr80021E60, 320u, 45u, 68u, 22u, 960u, 20u, 3u},
    {0x800512B0u, kMainDirectoryRawTextureAttr80021E60, 320u, 67u, 68u, 22u, 960u, 26u, 3u},
    {0x80051290u, kMainDirectoryRawTextureAttr80021E60, 384u, 68u, 88u, 67u, 960u, 0u, 2u},
    {0x80050D30u, kMainDirectoryRawTextureAttr80021E60, 448u, 0u, 136u, 82u, 960u, 0u, 2u},
    {0x80050E10u, kMainDirectoryRawTextureAttr80021E60, 448u, 82u, 136u, 82u, 960u, 0u, 2u},
    {0x80050AC0u, kMainDirectoryRawTextureAttr80021E60, 384u, 0u, 64u, 37u, 960u, 0u, 2u},
    {0x800509A0u, kMainDirectoryRawTextureAttr80021E60, 320u, 0u, 48u, 17u, 960u, 2u, 3u},
};

constexpr StaticLanguageSpriteFamily80021E60 kMainExitText[] = {
    {{0x800509D0u, kMainDirectoryRawTextureAttr80021E60, 320u, 256u, 28u, 11u, 960u, 5u, 3u}, 242, 191},
    {{0x80050A30u, kMainDirectoryRawTextureAttr80021E60, 335u, 256u, 32u, 8u, 960u, 5u, 3u}, 238, 193},
    {{0x80050A00u, kMainDirectoryRawTextureAttr80021E60, 327u, 256u, 32u, 8u, 960u, 5u, 3u}, 238, 193},
    {{0x80050A60u, kMainDirectoryRawTextureAttr80021E60, 343u, 256u, 28u, 12u, 960u, 5u, 3u}, 241, 190},
    {{0x80050A90u, kMainDirectoryRawTextureAttr80021E60, 350u, 256u, 32u, 10u, 960u, 5u, 3u}, 239, 191},
};

constexpr StaticLanguageSpriteFamily80021E60 kMainLanguageText[] = {
    {{0x80050F20u, kMainDirectoryRawTextureAttr80021E60, 320u, 267u, 60u, 12u, 960u, 11u, 3u}, 41, 64},
    {{0x80050F80u, kMainDirectoryRawTextureAttr80021E60, 335u, 279u, 60u, 11u, 960u, 11u, 3u}, 41, 64},
    {{0x80050F50u, kMainDirectoryRawTextureAttr80021E60, 320u, 279u, 60u, 12u, 960u, 11u, 3u}, 39, 63},
    {{0x80050FB0u, kMainDirectoryRawTextureAttr80021E60, 350u, 279u, 56u, 12u, 960u, 11u, 3u}, 43, 63},
    {{0x80050FE0u, kMainDirectoryRawTextureAttr80021E60, 364u, 279u, 60u, 11u, 960u, 11u, 3u}, 40, 64},
};

constexpr StaticLanguageSpriteFamily80021E60 kMainHiScoreText[] = {
    {{0x80051060u, kMainDirectoryRawTextureAttr80021E60, 320u, 291u, 60u, 11u, 960u, 17u, 3u}, 124, 81},
    {{0x800510C0u, kMainDirectoryRawTextureAttr80021E60, 350u, 291u, 60u, 11u, 960u, 17u, 3u}, 123, 81},
    {{0x80051090u, kMainDirectoryRawTextureAttr80021E60, 353u, 454u, 64u, 10u, 960u, 17u, 3u}, 121, 82},
    {{0x800510F0u, kMainDirectoryRawTextureAttr80021E60, 365u, 291u, 56u, 12u, 960u, 17u, 3u}, 126, 80},
    {{0x80051120u, kMainDirectoryRawTextureAttr80021E60, 369u, 453u, 56u, 26u, 960u, 17u, 3u}, 128, 71},
};

constexpr StaticLanguageSpriteFamily80021E60 kMainNormalText[] = {
    {{0x800511A0u, kMainDirectoryRawTextureAttr80021E60, 320u, 313u, 52u, 13u, 960u, 23u, 3u}, 218, 52},
    {{0x80051200u, kMainDirectoryRawTextureAttr80021E60, 346u, 313u, 52u, 13u, 960u, 23u, 3u}, 218, 52},
    {{0x800511D0u, kMainDirectoryRawTextureAttr80021E60, 333u, 313u, 52u, 13u, 960u, 23u, 3u}, 218, 52},
    {{0x80051230u, kMainDirectoryRawTextureAttr80021E60, 359u, 313u, 52u, 13u, 960u, 23u, 3u}, 218, 52},
    {{0x80051260u, kMainDirectoryRawTextureAttr80021E60, 320u, 326u, 52u, 13u, 960u, 23u, 3u}, 218, 52},
};

constexpr StaticLanguageSpriteFamily80021E60 kMainEasyText[] = {
    {{0x800512E0u, kMainDirectoryRawTextureAttr80021E60, 320u, 339u, 48u, 12u, 960u, 29u, 3u}, 218, 76},
    {{0x80051340u, kMainDirectoryRawTextureAttr80021E60, 344u, 339u, 48u, 12u, 960u, 29u, 3u}, 220, 76},
    {{0x80051310u, kMainDirectoryRawTextureAttr80021E60, 332u, 339u, 48u, 12u, 960u, 29u, 3u}, 218, 76},
    {{0x80051370u, kMainDirectoryRawTextureAttr80021E60, 356u, 339u, 48u, 12u, 960u, 29u, 3u}, 218, 76},
    {{0x800513A0u, kMainDirectoryRawTextureAttr80021E60, 368u, 339u, 48u, 15u, 960u, 29u, 3u}, 219, 73},
};

constexpr StaticLanguageSpriteFamily80021E60 kMainPracticeText[] = {
    {{0x80051400u, kMainDirectoryRawTextureAttr80021E60, 320u, 351u, 44u, 25u, 960u, 35u, 3u}, 41, 127},
    {{0x80051460u, kMainDirectoryRawTextureAttr80021E60, 342u, 351u, 40u, 26u, 960u, 35u, 3u}, 42, 126},
    {{0x80051430u, kMainDirectoryRawTextureAttr80021E60, 331u, 351u, 44u, 34u, 960u, 35u, 3u}, 43, 123},
    {{0x80051490u, kMainDirectoryRawTextureAttr80021E60, 352u, 351u, 40u, 26u, 960u, 35u, 3u}, 42, 127},
    {{0x800514C0u, kMainDirectoryRawTextureAttr80021E60, 368u, 354u, 44u, 26u, 960u, 35u, 3u}, 40, 127},
};

constexpr StaticLanguageSpriteFamily80021E60 kMainStageSelectText[] = {
    {{0x80051540u, kMainDirectoryRawTextureAttr80021E60, 320u, 376u, 44u, 28u, 960u, 41u, 3u}, 99, 144},
    {{0x800515A0u, kMainDirectoryRawTextureAttr80021E60, 342u, 377u, 44u, 28u, 960u, 41u, 3u}, 100, 144},
    {{0x80051570u, kMainDirectoryRawTextureAttr80021E60, 331u, 385u, 44u, 27u, 960u, 41u, 3u}, 101, 145},
    {{0x800515D0u, kMainDirectoryRawTextureAttr80021E60, 353u, 377u, 44u, 28u, 960u, 41u, 3u}, 101, 145},
    {{0x80051600u, kMainDirectoryRawTextureAttr80021E60, 368u, 380u, 44u, 26u, 960u, 41u, 3u}, 101, 145},
};

constexpr StaticLanguageSpriteFamily80021E60 kMainReplayText[] = {
    {{0x80051680u, kMainDirectoryRawTextureAttr80021E60, 320u, 404u, 44u, 13u, 960u, 47u, 3u}, 161, 152},
    {{0x800516E0u, kMainDirectoryRawTextureAttr80021E60, 343u, 405u, 48u, 27u, 960u, 47u, 3u}, 161, 146},
    {{0x800516B0u, kMainDirectoryRawTextureAttr80021E60, 331u, 412u, 48u, 24u, 960u, 47u, 3u}, 162, 149},
    {{0x80051710u, kMainDirectoryRawTextureAttr80021E60, 355u, 405u, 44u, 14u, 960u, 47u, 3u}, 162, 152},
    {{0x80051740u, kMainDirectoryRawTextureAttr80021E60, 368u, 406u, 40u, 24u, 960u, 47u, 3u}, 163, 148},
};

constexpr StaticLanguageSpriteFamily80021E60 kMainLoadText[] = {
    {{0x800517C0u, kMainDirectoryRawTextureAttr80021E60, 320u, 417u, 36u, 19u, 960u, 53u, 3u}, 226, 136},
    {{0x80051820u, kMainDirectoryRawTextureAttr80021E60, 343u, 432u, 40u, 22u, 960u, 53u, 3u}, 224, 134},
    {{0x800517F0u, kMainDirectoryRawTextureAttr80021E60, 331u, 436u, 40u, 23u, 960u, 53u, 3u}, 225, 132},
    {{0x80051850u, kMainDirectoryRawTextureAttr80021E60, 355u, 419u, 40u, 21u, 960u, 53u, 3u}, 224, 134},
    {{0x80051880u, kMainDirectoryRawTextureAttr80021E60, 368u, 430u, 40u, 23u, 960u, 53u, 3u}, 225, 132},
};

// SCUS_941.83 descriptor and VTEXT tables consumed by 80020F94.
constexpr StaticCardGridTemplate80020F94 kCardGridTemplates80020F94[] = {
    {0x800509A0u, 0x50000040u, 320u, 0u, 48u, 17u, 960u, 2u},
    {0x800509B0u, 0x50000040u, 320u, 0u, 48u, 17u, 960u, 3u},
    {0x800509C0u, 0x50000040u, 320u, 0u, 48u, 17u, 960u, 4u},
    {0x800509D0u, 0x50000040u, 320u, 256u, 28u, 11u, 960u, 5u},
    {0x800509E0u, 0x50000040u, 320u, 256u, 28u, 11u, 960u, 6u},
    {0x800509F0u, 0x50000040u, 320u, 256u, 28u, 11u, 960u, 7u},
    {0x80050A00u, 0x50000040u, 327u, 256u, 32u, 8u, 960u, 5u},
    {0x80050A10u, 0x50000040u, 327u, 256u, 32u, 8u, 960u, 6u},
    {0x80050A20u, 0x50000040u, 327u, 256u, 32u, 8u, 960u, 7u},
    {0x80050A30u, 0x50000040u, 335u, 256u, 32u, 8u, 960u, 5u},
    {0x80050A40u, 0x50000040u, 335u, 256u, 32u, 8u, 960u, 6u},
    {0x80050A50u, 0x50000040u, 335u, 256u, 32u, 8u, 960u, 7u},
    {0x80050A60u, 0x50000040u, 343u, 256u, 28u, 12u, 960u, 5u},
    {0x80050A70u, 0x50000040u, 343u, 256u, 28u, 12u, 960u, 6u},
    {0x80050A80u, 0x50000040u, 343u, 256u, 28u, 12u, 960u, 7u},
    {0x80050A90u, 0x50000040u, 350u, 256u, 32u, 10u, 960u, 5u},
    {0x80050AA0u, 0x50000040u, 350u, 256u, 32u, 10u, 960u, 6u},
    {0x80050AB0u, 0x50000040u, 350u, 256u, 32u, 10u, 960u, 7u},
    {0x80050AC0u, 0x50000040u, 384u, 0u, 64u, 37u, 960u, 0u},
    {0x80050AD0u, 0x50000040u, 384u, 0u, 64u, 37u, 960u, 1u},
    {0x80052240u, 0x50000040u, 640u, 0u, 76u, 20u, 1008u, 40u},
    {0x80052250u, 0x50000040u, 640u, 0u, 76u, 20u, 1008u, 41u},
    {0x80052260u, 0x50000040u, 640u, 0u, 76u, 20u, 1008u, 42u},
    {0x80052270u, 0x50000040u, 448u, 400u, 48u, 17u, 1008u, 50u},
    {0x80052280u, 0x50000040u, 457u, 431u, 80u, 12u, 1008u, 50u},
    {0x80052290u, 0x50000040u, 460u, 400u, 76u, 15u, 1008u, 50u},
    {0x800522A0u, 0x50000040u, 493u, 399u, 60u, 15u, 1008u, 50u},
    {0x800522B0u, 0x50000040u, 477u, 431u, 76u, 15u, 1008u, 50u},
    {0x800522C0u, 0x50000040u, 659u, 0u, 172u, 42u, 1008u, 46u},
    {0x800522D0u, 0x50000040u, 448u, 417u, 36u, 7u, 1008u, 50u},
    {0x800522E0u, 0x50000040u, 471u, 417u, 72u, 13u, 1008u, 50u},
    {0x800522F0u, 0x50000040u, 457u, 417u, 56u, 14u, 1008u, 50u},
    {0x80052300u, 0x50000040u, 496u, 431u, 48u, 15u, 1008u, 50u},
    {0x80052310u, 0x50000040u, 489u, 417u, 52u, 14u, 1008u, 50u},
    {0x800527D0u, 0x50000040u, 448u, 310u, 48u, 17u, 1008u, 50u},
    {0x800527E0u, 0x50000040u, 476u, 310u, 72u, 15u, 1008u, 50u},
    {0x800527F0u, 0x50000040u, 460u, 310u, 64u, 18u, 1008u, 50u},
    {0x80052800u, 0x50000040u, 494u, 310u, 72u, 15u, 1008u, 50u},
    {0x80052810u, 0x50000040u, 486u, 368u, 72u, 15u, 1008u, 50u},
    {0x80052820u, 0x50000040u, 448u, 327u, 32u, 12u, 1008u, 50u},
    {0x80052830u, 0x50000040u, 466u, 328u, 48u, 14u, 1008u, 50u},
    {0x80052840u, 0x50000040u, 456u, 328u, 36u, 11u, 1008u, 50u},
    {0x80052850u, 0x50000040u, 478u, 328u, 52u, 15u, 1008u, 50u},
    {0x80052860u, 0x50000040u, 491u, 328u, 44u, 14u, 1008u, 50u},
    {0x80052870u, 0x50000040u, 640u, 152u, 84u, 30u, 1008u, 44u},
    {0x80052880u, 0x50000040u, 448u, 351u, 72u, 17u, 1008u, 50u},
    {0x80052890u, 0x50000040u, 486u, 357u, 76u, 11u, 1008u, 50u},
    {0x800528A0u, 0x50000040u, 466u, 357u, 80u, 11u, 1008u, 50u},
    {0x800528B0u, 0x50000040u, 448u, 368u, 76u, 15u, 1008u, 50u},
    {0x800528C0u, 0x50000040u, 467u, 368u, 76u, 16u, 1008u, 50u},
    {0x800528D0u, 0x50000040u, 448u, 385u, 48u, 15u, 1008u, 50u},
    {0x800528E0u, 0x50000040u, 479u, 385u, 76u, 8u, 1008u, 50u},
    {0x800528F0u, 0x50000040u, 460u, 385u, 76u, 14u, 1008u, 50u},
    {0x80052900u, 0x50000040u, 479u, 393u, 56u, 15u, 1008u, 50u},
    {0x80052910u, 0x50000040u, 498u, 385u, 48u, 14u, 1008u, 50u},
    {0x80052920u, 0x50000040u, 448u, 424u, 16u, 11u, 1008u, 50u},
    {0x80052930u, 0x50000040u, 448u, 445u, 28u, 11u, 1008u, 50u},
    {0x80052940u, 0x50000040u, 448u, 435u, 24u, 10u, 1008u, 50u},
    {0x80052950u, 0x50000040u, 448u, 481u, 44u, 11u, 1008u, 50u},
    {0x80052960u, 0x50000040u, 448u, 456u, 52u, 14u, 1008u, 50u},
    {0x80052970u, 0x50000040u, 448u, 470u, 28u, 11u, 1008u, 50u},
    {0x80052980u, 0x50000040u, 661u, 152u, 16u, 16u, 1008u, 43u},
    {0x80052990u, 0x50000040u, 448u, 339u, 32u, 12u, 1008u, 50u},
    {0x800529A0u, 0x50000040u, 466u, 343u, 120u, 14u, 1008u, 50u},
    {0x800529B0u, 0x50000040u, 456u, 340u, 40u, 11u, 1008u, 50u},
    {0x800529C0u, 0x50000040u, 496u, 342u, 20u, 15u, 1008u, 50u},
    {0x800529D0u, 0x50000040u, 501u, 345u, 32u, 12u, 1008u, 50u},
};

constexpr StaticCardGridLanguageRecord80020F94
    kCardGridLanguageRecords80020F94[] = {
        {7, 0, {{0x80052990u, 154, 44}, {0x80052980u, 189, 44}, {0x80052270u, 53, 42}, {0x800522D0u, 230, 48}, {0x80052920u, 208, 45}}},
        {7, 1, {{0x800529B0u, 139, 45}, {0x80052980u, 178, 43}, {0x80052290u, 40, 43}, {0x800522F0u, 221, 45}, {0x80052940u, 195, 47}}},
        {7, 2, {{0x800529A0u, 140, 39}, {0x80052980u, 260, 37}, {0x80052280u, 39, 45}, {0x800522E0u, 186, 52}, {0x80052930u, 156, 54}}},
        {7, 3, {{0x800529C0u, 145, 44}, {0x80052980u, 252, 44}, {0x800522A0u, 49, 43}, {0x80052300u, 169, 44}, {0x80052960u, 221, 45}}},
        {7, 4, {{0x800529D0u, 143, 44}, {0x80052980u, 175, 44}, {0x800522B0u, 42, 43}, {0x80052310u, 223, 45}, {0x80052970u, 191, 48}}},
        {8, 0, {{0x80052990u, 154, 44}, {0x80052980u, 189, 44}, {0x800527D0u, 53, 42}, {0x80052820u, 230, 44}, {0x80052920u, 208, 45}}},
        {8, 1, {{0x800529B0u, 150, 45}, {0x80052980u, 188, 43}, {0x800527F0u, 47, 41}, {0x80052840u, 232, 45}, {0x80052940u, 205, 47}}},
        {8, 2, {{0x800529A0u, 140, 39}, {0x80052980u, 260, 37}, {0x800527E0u, 43, 43}, {0x80052830u, 199, 51}, {0x80052930u, 169, 54}}},
        {8, 3, {{0x800529C0u, 143, 44}, {0x80052980u, 255, 44}, {0x80052800u, 43, 43}, {0x80052850u, 167, 44}, {0x80052960u, 224, 45}}},
        {8, 4, {{0x800529D0u, 146, 44}, {0x80052980u, 178, 44}, {0x80052810u, 43, 43}, {0x80052860u, 225, 45}, {0x80052970u, 194, 48}}},
        {9, 0, {{0x80052990u, 147, 44}, {0x80052980u, 182, 44}, {0x80052880u, 42, 42}, {0x800528D0u, 222, 44}, {0x80052920u, 201, 45}}},
        {9, 1, {{0x800529B0u, 158, 38}, {0x80052980u, 196, 37}, {0x800528A0u, 39, 45}, {0x800528F0u, 170, 51}, {0x80052950u, 213, 38}}},
        {9, 2, {{0x800529A0u, 140, 39}, {0x80052980u, 260, 37}, {0x80052890u, 41, 45}, {0x800528E0u, 184, 54}, {0x80052930u, 154, 54}}},
        {9, 3, {{0x800529C0u, 140, 44}, {0x80052980u, 256, 44}, {0x800528B0u, 40, 43}, {0x80052900u, 164, 44}, {0x80052960u, 225, 45}}},
        {9, 4, {{0x800529D0u, 145, 44}, {0x80052980u, 177, 44}, {0x800528C0u, 41, 42}, {0x80052910u, 224, 45}, {0x80052970u, 193, 48}}},
};

constexpr PositionedCardGridTemplate80020F94
    kCardGridExitLabels80020F94[5][3] = {
        {{0x800509D0u, 242, 191}, {0x800509E0u, 242, 191}, {0x800509F0u, 242, 191}},
        {{0x80050A30u, 238, 193}, {0x80050A40u, 238, 193}, {0x80050A50u, 238, 193}},
        {{0x80050A00u, 238, 193}, {0x80050A10u, 238, 193}, {0x80050A20u, 238, 193}},
        {{0x80050A60u, 241, 190}, {0x80050A70u, 241, 190}, {0x80050A80u, 241, 190}},
        {{0x80050A90u, 239, 191}, {0x80050AA0u, 239, 191}, {0x80050AB0u, 239, 191}},
    };

constexpr StaticLanguageSpriteFamily80021E60
    kOptionsTitleLanguageLabel80021910 = {
        {0x80050F20u, kMainDirectoryRawTextureAttr80021E60, 320u, 267u,
         60u, 12u, 960u, 11u, 3u},
        41,
        64,
};

constexpr StaticLanguageSpriteFamily80021E60 kOptionsSubtitleOn80021910[] = {
    {{0x80050AF0u, kMainDirectoryRawTextureAttr80021E60, 384u, 356u, 48u, 19u, 960u, 23u, 3u}, 220, 49},
    {{0x80050B50u, kMainDirectoryRawTextureAttr80021E60, 396u, 356u, 48u, 18u, 960u, 23u, 3u}, 219, 50},
    {{0x80050B20u, kMainDirectoryRawTextureAttr80021E60, 408u, 356u, 52u, 18u, 960u, 23u, 3u}, 218, 50},
    {{0x80050B80u, kMainDirectoryRawTextureAttr80021E60, 421u, 356u, 52u, 18u, 960u, 23u, 3u}, 218, 50},
    {{0x80050BB0u, kMainDirectoryRawTextureAttr80021E60, 434u, 356u, 48u, 20u, 960u, 23u, 3u}, 220, 48},
};

constexpr StaticLanguageSpriteFamily80021E60 kOptionsSubtitleOff80021910[] = {
    {{0x80050BE0u, kMainDirectoryRawTextureAttr80021E60, 384u, 376u, 48u, 20u, 960u, 29u, 3u}, 220, 72},
    {{0x80050C40u, kMainDirectoryRawTextureAttr80021E60, 396u, 376u, 48u, 18u, 960u, 29u, 3u}, 219, 73},
    {{0x80050C10u, kMainDirectoryRawTextureAttr80021E60, 408u, 376u, 52u, 18u, 960u, 29u, 3u}, 218, 73},
    {{0x80050C70u, kMainDirectoryRawTextureAttr80021E60, 421u, 376u, 52u, 18u, 960u, 29u, 3u}, 218, 73},
    {{0x80050CA0u, kMainDirectoryRawTextureAttr80021E60, 434u, 376u, 48u, 21u, 960u, 29u, 3u}, 220, 71},
};

constexpr StaticLanguageSpriteFamily80021E60 kOptionsLanguageA80021910[] = {
    {{0x80050D00u, kMainDirectoryRawTextureAttr80021E60, 384u, 256u, 40u, 26u, 960u, 35u, 3u}, 42, 127},
    {{0x80050D80u, kMainDirectoryRawTextureAttr80021E60, 384u, 282u, 44u, 18u, 960u, 23u, 3u}, 89, 146},
    {{0x80050DE0u, kMainDirectoryRawTextureAttr80021E60, 384u, 300u, 44u, 16u, 960u, 41u, 3u}, 138, 153},
    {{0x80050E60u, kMainDirectoryRawTextureAttr80021E60, 384u, 316u, 44u, 14u, 960u, 53u, 3u}, 187, 150},
    {{0x80050EC0u, kMainDirectoryRawTextureAttr80021E60, 384u, 330u, 44u, 26u, 960u, 47u, 3u}, 235, 127},
};

constexpr StaticLanguageSpriteFamily80021E60 kOptionsLanguageB80021910[] = {
    {{0x80050CD0u, kMainDirectoryRawTextureAttr80021E60, 482u, 49u, 60u, 52u, 976u, 1u, 3u}, 33, 115},
    {{0x80050D50u, kMainDirectoryRawTextureAttr80021E60, 482u, 101u, 56u, 44u, 976u, 4u, 3u}, 81, 133},
    {{0x80050DB0u, kMainDirectoryRawTextureAttr80021E60, 482u, 145u, 52u, 37u, 976u, 7u, 3u}, 134, 142},
    {{0x80050E30u, kMainDirectoryRawTextureAttr80021E60, 497u, 49u, 56u, 45u, 976u, 10u, 3u}, 183, 133},
    {{0x80050E90u, kMainDirectoryRawTextureAttr80021E60, 496u, 101u, 60u, 52u, 976u, 13u, 3u}, 228, 115},
};

struct StaticStageSelectPosition80020568 {
    int16_t x = 0;
    int16_t y = 0;
};

struct StaticStageSelectTemplate80020568 {
    uint32_t address = 0;
    uint32_t attr = 0;
    uint16_t texX = 0;
    uint16_t texY = 0;
    uint16_t width = 0;
    uint16_t height = 0;
    uint16_t clutX = 0;
    uint16_t clutY = 0;
};

constexpr StaticStageSelectTemplate80020568
    kStageSelectTemplates80020568[] = {
        {0x800509A0u, 0x50000040u, 320u, 0u, 48u, 17u, 960u, 2u},
        {0x800509B0u, 0x50000040u, 320u, 0u, 48u, 17u, 960u, 3u},
        {0x800509C0u, 0x50000040u, 320u, 0u, 48u, 17u, 960u, 4u},
        {0x800509D0u, 0x50000040u, 320u, 256u, 28u, 11u, 960u, 5u},
        {0x800509E0u, 0x50000040u, 320u, 256u, 28u, 11u, 960u, 6u},
        {0x800509F0u, 0x50000040u, 320u, 256u, 28u, 11u, 960u, 7u},
        {0x80050A00u, 0x50000040u, 327u, 256u, 32u, 8u, 960u, 5u},
        {0x80050A10u, 0x50000040u, 327u, 256u, 32u, 8u, 960u, 6u},
        {0x80050A20u, 0x50000040u, 327u, 256u, 32u, 8u, 960u, 7u},
        {0x80050A30u, 0x50000040u, 335u, 256u, 32u, 8u, 960u, 5u},
        {0x80050A40u, 0x50000040u, 335u, 256u, 32u, 8u, 960u, 6u},
        {0x80050A50u, 0x50000040u, 335u, 256u, 32u, 8u, 960u, 7u},
        {0x80050A60u, 0x50000040u, 343u, 256u, 28u, 12u, 960u, 5u},
        {0x80050A70u, 0x50000040u, 343u, 256u, 28u, 12u, 960u, 6u},
        {0x80050A80u, 0x50000040u, 343u, 256u, 28u, 12u, 960u, 7u},
        {0x80050A90u, 0x50000040u, 350u, 256u, 32u, 10u, 960u, 5u},
        {0x80050AA0u, 0x50000040u, 350u, 256u, 32u, 10u, 960u, 6u},
        {0x80050AB0u, 0x50000040u, 350u, 256u, 32u, 10u, 960u, 7u},
        {0x80050AC0u, 0x50000040u, 384u, 0u, 64u, 37u, 960u, 0u},
        {0x80050AD0u, 0x50000040u, 384u, 0u, 64u, 37u, 960u, 1u},
        {0x800518D0u, 0x50000040u, 576u, 120u, 68u, 72u, 1008u, 0u},
        {0x800518E0u, 0x50000040u, 576u, 120u, 68u, 72u, 1008u, 1u},
        {0x800518F0u, 0x50000040u, 576u, 120u, 68u, 72u, 1008u, 2u},
        {0x80051900u, 0x50000040u, 593u, 120u, 44u, 44u, 1008u, 3u},
        {0x80051910u, 0x50000040u, 604u, 120u, 44u, 44u, 1008u, 4u},
        {0x80051920u, 0x50000040u, 615u, 120u, 44u, 44u, 1008u, 5u},
        {0x80051930u, 0x50000040u, 593u, 164u, 44u, 44u, 1008u, 6u},
        {0x80051940u, 0x50000040u, 604u, 164u, 44u, 44u, 1008u, 7u},
        {0x80051950u, 0x50000040u, 615u, 164u, 44u, 44u, 1008u, 8u},
        {0x80051960u, 0x50000040u, 626u, 0u, 36u, 36u, 1008u, 9u},
        {0x80051970u, 0x50000040u, 626u, 36u, 36u, 36u, 1008u, 10u},
        {0x80051980u, 0x50000040u, 626u, 72u, 36u, 36u, 1008u, 11u},
        {0x80051990u, 0x50000040u, 626u, 108u, 36u, 36u, 1008u, 12u},
        {0x800519A0u, 0x50000040u, 626u, 0u, 36u, 36u, 1008u, 13u},
        {0x800519B0u, 0x50000040u, 626u, 36u, 36u, 36u, 1008u, 14u},
        {0x800519C0u, 0x50000040u, 626u, 72u, 36u, 36u, 1008u, 15u},
        {0x800519D0u, 0x50000040u, 626u, 108u, 36u, 36u, 1008u, 16u},
        {0x800519E0u, 0x50000040u, 576u, 0u, 192u, 40u, 1008u, 17u},
        {0x800519F0u, 0x50000040u, 576u, 40u, 192u, 40u, 1008u, 18u},
        {0x80051A00u, 0x40000040u, 576u, 80u, 192u, 40u, 1008u, 19u},
        {0x80051A10u, 0x50000040u, 576u, 0u, 192u, 40u, 1008u, 20u},
        {0x80051A20u, 0x50000040u, 593u, 120u, 44u, 44u, 1008u, 21u},
        {0x80051A30u, 0x50000040u, 604u, 120u, 44u, 44u, 1008u, 22u},
        {0x80051A40u, 0x50000040u, 615u, 120u, 44u, 44u, 1008u, 23u},
        {0x80051A50u, 0x50000040u, 593u, 164u, 44u, 44u, 1008u, 24u},
        {0x80051A60u, 0x50000040u, 604u, 164u, 44u, 44u, 1008u, 25u},
        {0x80051A70u, 0x50000040u, 615u, 164u, 44u, 44u, 1008u, 26u},
        {0x80051A80u, 0x50000040u, 626u, 0u, 36u, 36u, 1008u, 27u},
        {0x80051A90u, 0x50000040u, 626u, 36u, 36u, 36u, 1008u, 28u},
        {0x80051AA0u, 0x50000040u, 576u, 208u, 120u, 20u, 1008u, 29u},
        {0x80051AB0u, 0x50000040u, 448u, 256u, 96u, 13u, 1008u, 30u},
        {0x80051AC0u, 0x50000040u, 448u, 270u, 112u, 11u, 1008u, 30u},
        {0x80051AD0u, 0x50000040u, 472u, 256u, 100u, 14u, 1008u, 30u},
        {0x80051AE0u, 0x50000040u, 476u, 270u, 112u, 12u, 1008u, 30u},
        {0x80051AF0u, 0x50000040u, 448u, 281u, 112u, 14u, 1008u, 30u},
        {0x80051B00u, 0x50000040u, 448u, 295u, 36u, 10u, 1008u, 31u},
        {0x80051B10u, 0x50000040u, 448u, 295u, 36u, 10u, 1008u, 35u},
        {0x80051B20u, 0x50000040u, 448u, 295u, 36u, 10u, 1008u, 36u},
        {0x80051B30u, 0x50000040u, 468u, 295u, 40u, 10u, 1008u, 34u},
        {0x80051B40u, 0x50000040u, 468u, 295u, 40u, 10u, 1008u, 35u},
        {0x80051B50u, 0x50000040u, 468u, 295u, 40u, 10u, 1008u, 36u},
        {0x80051B60u, 0x50000040u, 457u, 295u, 44u, 10u, 1008u, 34u},
        {0x80051B70u, 0x50000040u, 457u, 295u, 44u, 10u, 1008u, 35u},
        {0x80051B80u, 0x50000040u, 457u, 295u, 44u, 10u, 1008u, 36u},
        {0x80051B90u, 0x50000040u, 478u, 295u, 44u, 10u, 1008u, 34u},
        {0x80051BA0u, 0x50000040u, 478u, 295u, 44u, 10u, 1008u, 35u},
        {0x80051BB0u, 0x50000040u, 478u, 295u, 44u, 10u, 1008u, 36u},
        {0x80051BC0u, 0x50000040u, 489u, 295u, 36u, 10u, 1008u, 34u},
        {0x80051BD0u, 0x50000040u, 489u, 295u, 36u, 10u, 1008u, 35u},
        {0x80051BE0u, 0x50000040u, 489u, 295u, 36u, 10u, 1008u, 36u},
        {0x80051BF0u, 0x50000040u, 606u, 208u, 44u, 10u, 1008u, 31u},
};

constexpr uint32_t kStageSelectTitleTextTemplates80053244[5] = {
    0x80051AB0u, 0x80051AD0u, 0x80051AC0u, 0x80051AE0u, 0x80051AF0u,
};
constexpr StaticStageSelectPosition80020568
    kStageSelectTitleTextPositions80053248[5] = {
        {44, 36}, {43, 35}, {35, 37}, {35, 37}, {35, 35},
};
constexpr uint32_t kStageSelectTopTextTemplates80053050[5][3] = {
    {0x80051B20u, 0x80051B00u, 0x80051B10u},
    {0x80051B80u, 0x80051B60u, 0x80051B70u},
    {0x80051B50u, 0x80051B30u, 0x80051B40u},
    {0x80051BB0u, 0x80051B90u, 0x80051BA0u},
    {0x80051BE0u, 0x80051BC0u, 0x80051BD0u},
};
constexpr StaticStageSelectPosition80020568
    kStageSelectTopTextPositions8005308C[5][6] = {
        {{48, 122}, {141, 106}, {229, 90}, {48, 199}, {141, 182}, {229, 166}},
        {{50, 122}, {143, 106}, {231, 90}, {50, 199}, {143, 182}, {231, 166}},
        {{47, 122}, {140, 106}, {228, 90}, {47, 199}, {140, 182}, {228, 166}},
        {{44, 122}, {137, 106}, {225, 90}, {44, 199}, {137, 182}, {225, 166}},
        {{48, 122}, {141, 106}, {229, 90}, {48, 199}, {141, 182}, {229, 166}},
};
constexpr uint32_t kStageSelectPanelTemplates800531C4[3] = {
    0x800518F0u, 0x800518D0u, 0x800518E0u,
};
constexpr StaticStageSelectPosition80020568
    kStageSelectPanelPositions8005317C[6] = {
        {37, 66}, {130, 50}, {218, 34}, {37, 143}, {130, 126}, {218, 110},
};
constexpr uint32_t kStageSelectNameTemplates80053214[6][2] = {
    {0x80051900u, 0x80051A20u},
    {0x80051910u, 0x80051A30u},
    {0x80051920u, 0x80051A40u},
    {0x80051930u, 0x80051A50u},
    {0x80051940u, 0x80051A60u},
    {0x80051950u, 0x80051A70u},
};
constexpr StaticStageSelectPosition80020568
    kStageSelectNamePositions80053194[6] = {
        {45, 75}, {138, 59}, {226, 43}, {45, 152}, {138, 135}, {226, 119},
};
constexpr uint32_t kStageSelectBadgeATemplates800531F4[8] = {
    0x80051A90u, 0x800519B0u, 0x80051970u, 0x800519B0u,
    0x80051980u, 0x800519C0u, 0x80051990u, 0x800519D0u,
};
constexpr uint32_t kStageSelectBadgeBTemplates800531E8[3] = {
    0x80051A80u, 0x80051960u, 0x800519A0u,
};
constexpr StaticStageSelectPosition80020568
    kStageSelectBadgePositions800531AC[6] = {
        {78, 64}, {171, 48}, {259, 32}, {78, 141}, {171, 124}, {259, 108},
};
constexpr uint32_t kStageSelectExitTextTemplates80053000[5][3] = {
    {0x800509D0u, 0x800509E0u, 0x800509F0u},
    {0x80050A30u, 0x80050A40u, 0x80050A50u},
    {0x80050A00u, 0x80050A10u, 0x80050A20u},
    {0x80050A60u, 0x80050A70u, 0x80050A80u},
    {0x80050A90u, 0x80050AA0u, 0x80050AB0u},
};
constexpr StaticStageSelectPosition80020568
    kStageSelectExitTextPositions8005300C[5] = {
        {242, 191}, {238, 193}, {238, 193}, {241, 190}, {239, 191},
};

static_assert(sizeof(kStageSelectTemplates80020568) /
                      sizeof(kStageSelectTemplates80020568[0]) ==
                  71u,
              "80020568 must own every referenced SCUS descriptor");

constexpr PageDrawRouteSpec kPageRoutes[] = {
    {DirectoryPage::StageSelectEv2,
     2,
     kFn80020568,
     3,
     "event=2 stage select page",
     "drawn through 8001E750 after 8001D74C(3); event loop owns input"},
    {DirectoryPage::MainDirectoryEv3,
     3,
     kFn80021E60,
     3,
     "event=3 main directory page",
     "drawn through 8001E750 after 8001D74C(3); ctx state words drive selection"},
    {DirectoryPage::CardSaveEv7,
     7,
     kFn80020F94,
     3,
     "event=7 save card grid page",
     "draw-only id from card loop; arg block owns rows, cols, selected, exit blink"},
    {DirectoryPage::CardLoadEv8,
     8,
     kFn80020F94,
     3,
     "event=8 load card grid page",
     "draw-only id from card loop; item count is provided by card loop"},
    {DirectoryPage::CardReplayEv9,
     9,
     kFn80020F94,
     3,
     "event=9 replay card grid page",
     "draw-only id from card loop; replay has German footer template special case"},
    {DirectoryPage::PracticeEv16,
     16,
     kFn80023618,
     3,
     "event=16 practice page",
     "draw-only id from practice self-loop; not a dispatcher case"},
    {DirectoryPage::OptionsEv17,
     17,
     kFn80021910,
     4,
     "event=17 options/language page",
     "drawn through 8001E750 after 8001D74C(4); timeout belongs to event loop"},
};

constexpr FixedAnchorSpec kFixedAnchors[] = {
    {DirectoryPage::MainDirectoryEv3,
     "top language panel",
     28,
     36,
     0x80051010u,
     0x80051020u,
     0,
     kTableMainLanguage80053554,
     "ctx+0x10 index 0: default 0; Cross writes 1"},
    {DirectoryPage::MainDirectoryEv3,
     "top hi-score panel",
     110,
     56,
     0x80051150u,
     0x80051160u,
     0,
     kTableMainHiScore800535B0,
     "ctx+0x14 index 1: default -1; Cross writes 0"},
    {DirectoryPage::MainDirectoryEv3,
     "top normal/easy panel",
     207,
     34,
     0x80051290u,
     0x800512A0u,
     0,
     kTableMainNormal80053640,
     "ctx+0x18 index 2 selects normal/easy side text"},
    {DirectoryPage::MainDirectoryEv3,
     "bottom practice panel",
     26,
     106,
     0x800514F0u,
     0x80051500u,
     0,
     kTableMainPractice80053710,
     "ctx+0x1C index 3: Square writes 0"},
    {DirectoryPage::MainDirectoryEv3,
     "bottom stage select panel",
     94,
     122,
     0x80051630u,
     0x80051640u,
     0,
     kTableMainStageSelect80053760,
     "ctx+0x1C index 3: Cross writes 1"},
    {DirectoryPage::MainDirectoryEv3,
     "bottom replay panel",
     162,
     122,
     0x80051770u,
     0x80051780u,
     0,
     kTableMainReplay800537B0,
     "ctx+0x1C index 3: Circle writes 2"},
    {DirectoryPage::MainDirectoryEv3,
     "bottom load panel",
     226,
     106,
     0x800518B0u,
     0x800518C0u,
     0,
     kTableMainLoad80053800,
     "ctx+0x1C index 3: Triangle writes 3"},
    {DirectoryPage::MainDirectoryEv3,
     "play hardcoded background",
     37,
     47,
     0x80050EF0u,
     0x80050F00u,
     0x80050F10u,
     0,
     "hardcoded layer before language text"},
    {DirectoryPage::MainDirectoryEv3,
     "hi-score hardcoded hint",
     118,
     65,
     0x80051030u,
     0x80051040u,
     0x80051050u,
     0,
     "hardcoded hint layer"},
    {DirectoryPage::MainDirectoryEv3,
     "normal deco sprite",
     216,
     47,
     0x80051170u,
     0x80051180u,
     0x80051190u,
     0x8005362Cu,
     "sprite struct at 8005362C supplies base xy"},
    {DirectoryPage::MainDirectoryEv3,
     "easy deco sprite",
     216,
     71,
     0x800512B0u,
     0x800512C0u,
     0x800512D0u,
     0x8005363Cu,
     "sprite struct at 8005363C supplies base xy"},
    {DirectoryPage::MainDirectoryEv3,
     "practice icon",
     32,
     115,
     0x800513D0u,
     0x800513E0u,
     0x800513F0u,
     kTableMainPractice80053710,
     "bottom icon anchor"},
    {DirectoryPage::MainDirectoryEv3,
     "stage select icon",
     95,
     136,
     0x80051510u,
     0x80051520u,
     0x80051530u,
     kTableMainStageSelect80053760,
     "bottom icon anchor"},
    {DirectoryPage::MainDirectoryEv3,
     "replay icon",
     160,
     136,
     0x80051650u,
     0x80051660u,
     0x80051670u,
     kTableMainReplay800537B0,
     "bottom icon anchor"},
    {DirectoryPage::MainDirectoryEv3,
     "load icon",
     216,
     115,
     0x80051790u,
     0x800517A0u,
     0x800517B0u,
     kTableMainLoad80053800,
     "bottom icon anchor"},
    {DirectoryPage::StageSelectEv2,
     "stage select title",
     32,
     33,
     0,
     0,
     0,
     0,
     "title is emitted through the language text helper"},
    {DirectoryPage::StageSelectEv2,
     "shared exit",
     231,
     179,
     0x800509A0u,
     0x800509B0u,
     0x800509C0u,
     kTableSharedExitText80053000,
     "ctx+0x0 blink and ctx+0x4 confirm flag"},
    {DirectoryPage::CardGridFamily,
     "card title",
     37,
     36,
     0,
     0,
     0,
     kTableCardTitle80053DF4,
     "shared by save/load/replay card grid"},
    {DirectoryPage::CardGridFamily,
     "card prompt",
     123,
     30,
     0,
     0,
     0,
     kTableCardPrompt80053DA4,
     "shared by save/load/replay card grid"},
    {DirectoryPage::CardGridFamily,
     "card grid shared exit",
     231,
     179,
     0x800509A0u,
     0x800509B0u,
     0x800509C0u,
     kTableSharedExitText80053000,
     "arg+0 and arg+4 drive blink/done; selected==rows*cols selects exit"},
    {DirectoryPage::OptionsEv17,
     "options title panel",
     36,
     45,
     0x80050AE0u,
     0,
     0,
     0,
     "not the main directory top-left panel"},
    {DirectoryPage::OptionsEv17,
     "options top-right panel",
     207,
     34,
     0x80051290u,
     0x800512A0u,
     0,
     0,
     "cursor 0 blinks side state"},
    {DirectoryPage::OptionsEv17,
     "subtitle left plate",
     26,
     106,
     0x80050D30u,
     0x80050D40u,
     0,
     kTableOptionsSubtitleOn80053850,
     "cursor 1 language selection blinks side plate"},
    {DirectoryPage::OptionsEv17,
     "subtitle right plate",
     162,
     106,
     0x80050E10u,
     0x80050E20u,
     0,
     kTableOptionsSubtitleOff800538A0,
     "cursor 1 language selection blinks side plate"},
    {DirectoryPage::OptionsEv17,
     "shared exit",
     231,
     179,
     0x800509A0u,
     0x800509B0u,
     0x800509C0u,
     kTableSharedExitText80053000,
     "cursor 2 and done flag choose exit sprite/text"},
    {DirectoryPage::PracticeEv16,
     "practice exit prompt",
     231,
     179,
     0x800509B0u,
     0x800509C0u,
     0,
     kTableSharedExitText80053000,
     "ctx+0x48 selects yes/no prompt and ctx+0x4C blinks"},
};

constexpr LanguageTableSpec kLanguageTables[] = {
    {DirectoryPage::MainDirectoryEv3,
     DirectoryTableKind::MainLanguage,
     kTableMainLanguage80053554,
     16,
     5,
     "{u32 tpl0,u32 tpl1,u32 tpl2,s16 x,s16 y}",
     "word_800916D8 selects language; ctx state selects tpl"},
    {DirectoryPage::MainDirectoryEv3,
     DirectoryTableKind::MainHiScore,
     kTableMainHiScore800535B0,
     16,
     5,
     "{u32 tpl0,u32 tpl1,u32 tpl2,s16 x,s16 y}",
     "word_800916D8 selects language; ctx state selects tpl"},
    {DirectoryPage::MainDirectoryEv3,
     DirectoryTableKind::SharedExitText,
     kTableSharedExitText80053000,
     16,
     5,
     "{u32 tpl0,u32 tpl1,u32 tpl2,s16 x,s16 y}",
     "shared EXIT text, no local replacement text"},
    {DirectoryPage::MainDirectoryEv3,
     DirectoryTableKind::MainNormal,
     kTableMainNormal80053640,
     16,
     5,
     "{u32 tpl0,u32 tpl1,u32 tpl2,s16 x,s16 y}",
     "top right difficulty text"},
    {DirectoryPage::MainDirectoryEv3,
     DirectoryTableKind::MainEasy,
     kTableMainEasy80053690,
     16,
     5,
     "{u32 tpl0,u32 tpl1,u32 tpl2,s16 x,s16 y}",
     "top right difficulty text"},
    {DirectoryPage::MainDirectoryEv3,
     DirectoryTableKind::MainPractice,
     kTableMainPractice80053710,
     16,
     5,
     "{u32 tpl0,u32 tpl1,u32 tpl2,s16 x,s16 y}",
     "bottom button text"},
    {DirectoryPage::MainDirectoryEv3,
     DirectoryTableKind::MainStageSelect,
     kTableMainStageSelect80053760,
     16,
     5,
     "{u32 tpl0,u32 tpl1,u32 tpl2,s16 x,s16 y}",
     "bottom button text"},
    {DirectoryPage::MainDirectoryEv3,
     DirectoryTableKind::MainReplay,
     kTableMainReplay800537B0,
     16,
     5,
     "{u32 tpl0,u32 tpl1,u32 tpl2,s16 x,s16 y}",
     "bottom button text"},
    {DirectoryPage::MainDirectoryEv3,
     DirectoryTableKind::MainLoad,
     kTableMainLoad80053800,
     16,
     5,
     "{u32 tpl0,u32 tpl1,u32 tpl2,s16 x,s16 y}",
     "bottom button text"},
    {DirectoryPage::StageSelectEv2,
     DirectoryTableKind::StageSelectPoints,
     kTableStageSelectPoints80053104,
     4,
     30,
     "{s16 x,s16 y}; five languages times six points",
     "idx_base = 12 * word_800916D8; each point is two s16 values"},
    {DirectoryPage::StageSelectEv2,
     DirectoryTableKind::StageSelectSlices,
     kTableStageSelectSlices800531D0,
     4,
     6,
     "{u16 uOffsetPx,u16 widthPx}",
     "stage slice template base is 80051BF0"},
    {DirectoryPage::StageSelectEv2,
     DirectoryTableKind::SharedExitText,
     kTableSharedExitText80053000,
     16,
     5,
     "{u32 tpl0,u32 tpl1,u32 tpl2,s16 x,s16 y}",
     "shared EXIT text"},
    {DirectoryPage::CardGridFamily,
     DirectoryTableKind::CardPrompt,
     kTableCardPrompt80053DA4,
     16,
     5,
     "{u32 tpl0,u32 tpl1,u32 tpl2,s16 x,s16 y}",
     "shared card prompt text"},
    {DirectoryPage::CardGridFamily,
     DirectoryTableKind::CardTitle,
     kTableCardTitle80053DF4,
     16,
     5,
     "{u32 tpl0,u32 tpl1,u32 tpl2,s16 x,s16 y}",
     "shared card title text"},
    {DirectoryPage::CardGridFamily,
     DirectoryTableKind::CardFooter,
     kTableCardFooter80053E44,
     16,
     5,
     "{u32 tpl0,u32 tpl1,u32 tpl2,s16 x,s16 y}",
     "replay German uses template 80052950 special case"},
    {DirectoryPage::CardGridFamily,
     DirectoryTableKind::SharedExitText,
     kTableSharedExitText80053000,
     16,
     5,
     "{u32 tpl0,u32 tpl1,u32 tpl2,s16 x,s16 y}",
     "shared EXIT text"},
    {DirectoryPage::CardSaveEv7,
     DirectoryTableKind::CardSaveTextA,
     kTableCardSaveTextA80053E94,
     8,
     5,
     "{u32 tpl,s16 x,s16 y}",
     "save-specific third text table"},
    {DirectoryPage::CardSaveEv7,
     DirectoryTableKind::CardSaveTextB,
     kTableCardSaveTextB80053EBC,
     8,
     5,
     "{u32 tpl,s16 x,s16 y}",
     "save-specific fourth text table"},
    {DirectoryPage::CardLoadEv8,
     DirectoryTableKind::CardLoadTextA,
     kTableCardLoadTextA80053EE4,
     8,
     5,
     "{u32 tpl,s16 x,s16 y}",
     "load-specific third text table"},
    {DirectoryPage::CardLoadEv8,
     DirectoryTableKind::CardLoadTextB,
     kTableCardLoadTextB80053F0C,
     8,
     5,
     "{u32 tpl,s16 x,s16 y}",
     "load-specific fourth text table"},
    {DirectoryPage::CardReplayEv9,
     DirectoryTableKind::CardReplayTextA,
     kTableCardReplayTextA80053F34,
     8,
     5,
     "{u32 tpl,s16 x,s16 y}",
     "replay-specific third text table"},
    {DirectoryPage::CardReplayEv9,
     DirectoryTableKind::CardReplayTextB,
     kTableCardReplayTextB80053F5C,
     8,
     5,
     "{u32 tpl,s16 x,s16 y}",
     "replay-specific fourth text table"},
    {DirectoryPage::OptionsEv17,
     DirectoryTableKind::OptionsSubtitleOn,
     kTableOptionsSubtitleOn80053850,
     16,
     5,
     "{u32 tpl0,u32 tpl1,u32 tpl2,s16 x,s16 y}",
     "cursor 0: current side tpl2, other side tpl1"},
    {DirectoryPage::OptionsEv17,
     DirectoryTableKind::OptionsSubtitleOff,
     kTableOptionsSubtitleOff800538A0,
     16,
     5,
     "{u32 tpl0,u32 tpl1,u32 tpl2,s16 x,s16 y}",
     "cursor 0: current side tpl2, other side tpl1"},
    {DirectoryPage::OptionsEv17,
     DirectoryTableKind::OptionsLanguageA,
     kTableOptionsLanguageA800538F0,
     16,
     5,
     "{u32 tpl0,u32 tpl1,u32 tpl2,s16 x,s16 y}",
     "cursor 1: selected language tpl2, other languages tpl1"},
    {DirectoryPage::OptionsEv17,
     DirectoryTableKind::OptionsLanguageB,
     kTableOptionsLanguageB80053940,
     16,
     5,
     "{u32 tpl0,u32 tpl1,u32 tpl2,s16 x,s16 y}",
     "alternate language list plate coordinates"},
    {DirectoryPage::OptionsEv17,
     DirectoryTableKind::SharedExitText,
     kTableSharedExitText80053000,
     16,
     5,
     "{u32 tpl0,u32 tpl1,u32 tpl2,s16 x,s16 y}",
     "cursor 2 and done flag choose exit text"},
    {DirectoryPage::PracticeEv16,
     DirectoryTableKind::PracticeTitle,
     kTablePracticeTitle80053D48,
     8,
     5,
     "{u32 tpl,s16 x,s16 y}",
     "base practice header by language"},
    {DirectoryPage::PracticeEv16,
     DirectoryTableKind::PracticeSubtitle,
     kTablePracticeSubtitle80053BB8,
     8,
     5,
     "{u32 tpl,s16 x,s16 y}",
     "base practice subtitle/header by language"},
    {DirectoryPage::PracticeEv16,
     DirectoryTableKind::PracticeOverlay,
     kTablePracticeOverlayA80053C08,
     8,
     45,
     "{u32 tpl,s16 x,s16 y}; overlay table family",
     "ctx flags bit 0x400000 and ctx+0x1C select overlay"},
    {DirectoryPage::PracticeEv16,
     DirectoryTableKind::SharedExitText,
     kTableSharedExitText80053000,
     16,
     5,
     "{u32 tpl0,u32 tpl1,u32 tpl2,s16 x,s16 y}",
     "ctx+0x48 selects exit ask state"},
    {DirectoryPage::PracticeEv16,
     DirectoryTableKind::PracticeIconTemplate,
     kTablePracticeIconTemplate800540BC,
     4,
     9,
     "{u32 templatePtr}",
     "sequence stream icon code to template pointer"},
};

constexpr StagePointSpec kStagePoints[] = {
    {0, 0, 92, 122},
    {0, 1, 183, 106},
    {0, 2, 271, 90},
    {0, 3, 90, 199},
    {0, 4, 184, 182},
    {0, 5, 271, 166},
    {1, 0, 45, 122},
    {1, 1, 135, 106},
    {1, 2, 223, 90},
    {1, 3, 42, 199},
    {1, 4, 135, 182},
    {1, 5, 223, 166},
    {2, 0, 92, 122},
    {2, 1, 183, 106},
    {2, 2, 271, 90},
    {2, 3, 90, 199},
    {2, 4, 184, 182},
    {2, 5, 271, 166},
    {3, 0, 92, 122},
    {3, 1, 183, 106},
    {3, 2, 271, 90},
    {3, 3, 90, 199},
    {3, 4, 184, 182},
    {3, 5, 271, 166},
    {4, 0, 92, 122},
    {4, 1, 183, 106},
    {4, 2, 271, 90},
    {4, 3, 90, 199},
    {4, 4, 184, 182},
    {4, 5, 271, 166},
};

constexpr StageSliceSpec kStageSlices[] = {
    {0, 0, 4},
    {1, 4, 8},
    {2, 12, 8},
    {3, 20, 8},
    {4, 28, 8},
    {5, 36, 8},
};

constexpr CardGridSpec kCardGrid = {
    51,
    74,
    73,
    21,
    52,
    78,
    256,
    482,
    "rows=arg+12, cols=arg+14, selected=arg+20; exit is selected==rows*cols",
};

constexpr PracticeIconTemplateSpec kPracticeIcons[] = {
    {0, 0, "null"},
    {1, 0x8005405Cu, "GUI_SANK"},
    {2, 0x8005403Cu, "GUI_MARU"},
    {3, 0x8005404Cu, "GUI_PEKE"},
    {4, 0x8005406Cu, "GUI_SIKA"},
    {5, 0x8005401Cu, "GUI_L"},
    {6, 0x8005401Cu, "GUI_L"},
    {7, 0x8005402Cu, "GUI_R"},
    {8, 0x8005402Cu, "GUI_R"},
};

constexpr DirectoryGp0ReplayBaselineSpec kDirectoryGp0ReplayBaselines[] = {
    {DirectoryPage::MainDirectoryEv3,
     3,
     8126,
     "S0Directory",
     "main_directory_first_draw_ev3",
     438,
     1913,
     "db6b1699466fa3b9fe3d19904ea7d8bf3c8392f9464035724fffe7cff9db7c6a",
     "77e4093fcb336a80b39bd602e0879e302de95feafb53b2964af8d73d132033d4"},
    {DirectoryPage::MainDirectoryEv3,
     3,
     8230,
     "S0Directory",
     "main_directory_stage_select_action_tail",
     240,
     1052,
     "abd267e0bf489f97856b39ac796188062830c35e9b8930ca41f3e2df48ef6f40",
     "a0f82383851f73f36b2d326ffbabbf4dec3487a36ac155c344d9aaecb1e93a6b"},
    {DirectoryPage::StageSelectEv2,
     2,
     8291,
     "S0Directory",
     "stage_select_first_draw_ev2",
     240,
     1052,
     "32f1439dfcc73a2ab04037b85b499802d0b11ac24994638f9354d1856972e045",
     "a6a469c908e524f1e7f8cdd51c88d4d6c947f41a037b3897fd3bc3a7c91b9e37"},
    {DirectoryPage::StageSelectEv2,
     2,
     8339,
     "S0Directory",
     "stage_select_cursor8_after_down",
     270,
     1187,
     "fac3d432442be78925901b9f381bd4a5c8b01cfcec4e2e0c9a79d2dfebb17f19",
     "d9915a1c11c3d74b14fe1d8a22dffee264d45b6d83291ac369c0897b30daa274"},
    {DirectoryPage::StageSelectEv2,
     2,
     8468,
     "S0Directory",
     "stage_select_cancel_input_tail",
     270,
     1187,
     "a109d11ae4257fa20867e0cae02826f2303a1c31ac310d236dafb497b449c158",
     "d28a3d06bc06af4e380feab1a0696ed13d3f3ce5dcd76acbaf9119ad465bafdd"},
    {DirectoryPage::CardReplayEv9,
     9,
     8840,
     "S0Directory",
     "replay_card_grid_first_nonempty_draw",
     256,
     1130,
     "a90b952674344f609ca14b49646a8247bf1277607f2900506a8c30d7799b826c",
     "9cefafdcb134bfe4996e2745e52c170176723679b3ce2d3f98620d21b3d137b0"},
    {DirectoryPage::CardLoadEv8,
     8,
     9358,
     "S0Directory",
     "load_card_grid_first_nonempty_draw",
     256,
     1130,
     "a4c86fac811d9ec9efbe618f619143f42eb2efd11e973426e6aa5977a716aca7",
     "78eb30a29a606a5b31dc9eeff2a0a2e9ec21d36d3b44e3baa254dbe346f6a8a3"},
    {DirectoryPage::PracticeEv16,
     16,
     9976,
     "S0Directory",
     "practice_entry_first_nonempty_draw",
     250,
     1103,
     "0187ae701aa20d73477c6caddcfcac93de8b96f2e8e0111d87f5af5314ffda7b",
     "0d878be523d70eb415c19d5228487904e15aa3c1e079df918b5463d68637b0b1"},
    {DirectoryPage::PracticeEv16,
     16,
     11046,
     "S0Directory",
     "practice_runtime_draw",
     258,
     1139,
     "3781a73f6a678a49a364dcf65eed825121c658a40d90d7466c9837ea30807cee",
     "45a96bb842b028df2a3ef1555c33bc3c9378b16ed836aad866327942300f3053"},
    {DirectoryPage::MainDirectoryEv3,
     3,
     11200,
     "S0Directory",
     "main_directory_after_practice",
     434,
     1899,
     "3b0985e22cde992065d50ccb10f460862b76401c6996deec18fae739a1f02f9b",
     "aaa48644ce86659eee35e2cabd3941a29270d47108dec5baf4c3338411f5bd7f"},
    {DirectoryPage::CardLoadEv8,
     8,
     11524,
     "S0Directory",
     "main_directory_card_io_banner_ev8_first_trace",
     240,
     1052,
     "108cc6d2f3c34b1621d5d28386f1edd7dfe3452583c12ea7f70195e5d496dd33",
     "bf1efecb612d28c96d07931d6f876a02c2a1749e035e50ea9913e5a2c77aba7f"},
    {DirectoryPage::CardLoadEv8,
     8,
     11525,
     "S0Directory",
     "main_directory_card_io_banner_ev8_first_full_draw",
     296,
     1298,
     "5376f62d0a59d4f3f0cc02f0dea5609646fb4953cc5bb1ab51e66e23617c3305",
     "6fcfa5a00effa03fef5b3ae20c0198ded8fb8fd6ac640b86fcac28c2791e3232"},
    {DirectoryPage::CardLoadEv8,
     8,
     11584,
     "S0Directory",
     "main_directory_card_io_banner_ev8_first_run_last",
     296,
     1298,
     "80b5fdec5cdae55ce89ded582b602fb28dbb836cd9153ae4e3fbb77c607cb775",
     "fe9e8d5b9df2cf51ee8a914fcca6dec15754c8f6aec0992d526b468c5394ef09"},
    {DirectoryPage::CardLoadEv8,
     8,
     11662,
     "S0Directory",
     "main_directory_card_io_banner_ev8_second_run_first_nonempty",
     294,
     1295,
     "3fd7df54bb4a25d60cb9d05780000f370744b23fedefb816cf7a9d23f041033f",
     "b180ac7f71a0c7142650e62cf7052eeebd9918f17db9398a473724a5f657ea6a"},
    {DirectoryPage::MainDirectoryEv3,
     3,
     11742,
     "S0Directory",
     "main_directory_card_io_banner_ev3_transition_full",
     294,
     1295,
     "fd84a01714bcbe2ced4105e4d3dac87f234fd28a85ee32d5c3e40d4f9da01bbf",
     "377d42e26b1fdc03d023cbb6a969a495ce24e0430f4ae67925cf11d0d2d6f519"},
    {DirectoryPage::MainDirectoryEv3,
     3,
     12388,
     "S0Directory",
     "main_directory_card_io_banner_ev3_flash_first_nonempty",
     294,
     1295,
     "8cc27c79b31c7d636ace9272c3c35f2fac4dd03ab5257c758bd3aa28335ba503",
     "07b840f2dffa632775e39a1e7ef3a99590850d38d77723082bee6d67a6a95169"},
    {DirectoryPage::MainDirectoryEv3,
     3,
     12407,
     "S0Directory",
     "main_directory_card_io_banner_ev3_flash_last",
     294,
     1295,
     "a369950a740d9b5158f43119b86c1d15af83e4649085ea99a9c44332509befa9",
     "6e562af1d10d5ee555022e32f0f0c554e9ccf264a02b2c4a0df105d259d907f7"},
    {DirectoryPage::MainDirectoryEv3,
     3,
     13408,
     "S0Directory",
     "main_directory_exit_highlight",
     240,
     1052,
     "1d2a7d3f2f08290d3208e7d7e2bc2c19f8ff6c98255ee99dda24e181cd8479c5",
     "52327a7479d6c06a25fc869dda0b739873edb51e4e27e7361da0acad39f6aeca"},
    {DirectoryPage::MainDirectoryEv3,
     3,
     13663,
     "S0Directory",
     "main_directory_exit_action_tail",
     240,
     1052,
     "fc978ef104194746a6de6ea429df1891e99d07e66ab03a52c5783ddaf691e33c",
     "a70a265f4f8c0bb01990f655083188502a7ee2414c5efb7f8ff84b2e38089667"},
    {DirectoryPage::OptionsEv17,
     17,
     12774,
     "S0Options",
     "options_ev17_first_draw",
     240,
     1052,
     "a13a32228e4b9228defc57a2e6966e40c2e4581afd46969df94ea2e7c9328fbc",
     "c584dd740d36efa2601b6ebd32cd1917be70e55a2e05f59c4a6e6ea3e2fbd3ca"},
    {DirectoryPage::OptionsEv17,
     17,
     12839,
     "S0Options",
     "options_language_first_change",
     232,
     1016,
     "2337b25aaf09a1930941fdc5ed4d94b6e81ec05b9be49cf1d1f62f25f2896ad0",
     "5f4725d0bc622ffd0587e5b8d529927b5bae1e811d085e091c4337dfaec417cd"},
    {DirectoryPage::OptionsEv17,
     17,
     13023,
     "S0Options",
     "options_language_return_english",
     232,
     1016,
     "e62fdea65971556dc1e7cd17f3af8c1f3b2bd941e2ddc31bc26a2fb273d41ffe",
     "faa45447c2431321dd243796ef362694ef4ccc8ba460bbd6a1e87a6bdf2d69da"},
    {DirectoryPage::OptionsEv17,
     17,
     13121,
     "S0Options",
     "options_subtitle_off",
     232,
     1016,
     "90671c6fdeb469fca2452ece0466f918c7b822600de332ff4b46895bda433032",
     "f09a2118bd58a33cabbdc47309cfb6e7d76fd6ae76b684b4caab223dc874f721"},
    {DirectoryPage::OptionsEv17,
     17,
     13151,
     "S0Options",
     "options_subtitle_on",
     232,
     1016,
     "c4959f97a67137b85261a0748d9c8dec389bfb41e68201e8ca69736d35762c06",
     "9932a6911f9b49f3203ee2f1fa209e89435485b8c1a4f5fa524eb73156705386"},
    {DirectoryPage::OptionsEv17,
     17,
     13251,
     "S0Options",
     "options_exit_input",
     232,
     1016,
     "6147c4a94d982ecf6ccc27a4d16aa04d0e8cca7d0ef19a880e623671080e9126",
     "8da8faf8563fec85126aba654a55ba30b87b125f659b64897da95e955b1fccef"},
    {DirectoryPage::OptionsEv17,
     17,
     13312,
     "S0Options",
     "options_tail_return",
     232,
     1016,
     "f3887d589f7817e915d51132037925d5e29731451e595d17c4cf89fde0770c31",
     "cc77c9e36150312bef09f614de37e971b21365cdc265a463de00d8aa9a996da4"},
    {DirectoryPage::CardSaveEv7,
     7,
     11933,
     "Stage1Save",
     "save_card_grid_first_nonempty_draw",
     264,
     1166,
     "4b67f5a20c203354e774a3a50fdaf10ca92b1627ffe854cbce55df98e5e7ba75",
     "2b99d8338bc58ef5afb35371b9ba793e71be4efcc4d5744b6a4e137ccca1b46b"},
    {DirectoryPage::CardSaveEv7,
     7,
     12051,
     "Stage1Save",
     "save_card_grid_list_input_draw",
     264,
     1166,
     "4b67f5a20c203354e774a3a50fdaf10ca92b1627ffe854cbce55df98e5e7ba75",
     "2b99d8338bc58ef5afb35371b9ba793e71be4efcc4d5744b6a4e137ccca1b46b"},
    {DirectoryPage::CardSaveEv7,
     7,
     12158,
     "Stage1Save",
     "save_card_grid_select_input_draw",
     264,
     1166,
     "1da48de1d5c2becc56305bb8545b486ecf5b39f685432aab73a2c426459fb745",
     "201d5c6d1bb68672ebc265aae4b343f28e4d549bc0065806a5a7b92bde149793"},
};

static_assert(sizeof(kDirectoryGp0ReplayBaselines) /
                  sizeof(kDirectoryGp0ReplayBaselines[0]) ==
              static_cast<uint32_t>(kDirectoryGp0ReplayBaselineCount),
              "directory page GP0 replay baseline count must stay PSX-sized");

bool Append(DirectoryRenderPlan& plan, const DirectoryRenderAction& action)
{
    if (plan.count >= sizeof(plan.actions) / sizeof(plan.actions[0])) {
        plan.truncated = true;
        return false;
    }
    plan.actions[plan.count++] = action;
    return true;
}

void AppendAction(DirectoryRenderPlan& plan,
                  DirectoryRenderActionKind kind,
                  uint32_t psxFunction,
                  uint32_t tableAddress = 0,
                  uint32_t templateAddress = 0,
                  int16_t x = 0,
                  int16_t y = 0,
                  int32_t arg0 = 0,
                  int32_t arg1 = 0,
                  int32_t arg2 = 0,
                  int32_t arg3 = 0,
                  bool conditional = false)
{
    DirectoryRenderAction action{};
    action.kind = kind;
    action.page = plan.page;
    action.psxFunction = psxFunction;
    action.tableAddress = tableAddress;
    action.templateAddress = templateAddress;
    action.eventId = plan.eventId;
    action.x = x;
    action.y = y;
    action.args[0] = arg0;
    action.args[1] = arg1;
    action.args[2] = arg2;
    action.args[3] = arg3;
    action.conditional = conditional;
    (void)Append(plan, action);
}

void AppendPlan(DirectoryRenderPlan& dst, const DirectoryRenderPlan& src)
{
    for (uint32_t i = 0; i < src.count; ++i) {
        (void)Append(dst, src.actions[i]);
    }
    dst.truncated = dst.truncated || src.truncated;
    dst.blockedByGap = dst.blockedByGap || src.blockedByGap;
}

DirectoryRenderPlan MakePlan(const char* name, DirectoryPage page,
                             int32_t eventId)
{
    DirectoryRenderPlan plan{};
    plan.name = name;
    plan.runtimeCutoverAllowed = RuntimeCutoverAllowed();
    plan.page = page;
    plan.eventId = eventId;
    return plan;
}

bool PageMatches(DirectoryPage specPage, DirectoryPage page)
{
    if (specPage == page) {
        return true;
    }
    return specPage == DirectoryPage::CardGridFamily &&
           (page == DirectoryPage::CardSaveEv7 ||
            page == DirectoryPage::CardLoadEv8 ||
            page == DirectoryPage::CardReplayEv9);
}

void AppendRoute(DirectoryRenderPlan& plan)
{
    const PageDrawRouteSpec* route = FindPageDrawRouteSpec(plan.eventId);
    if (route == nullptr) {
        AppendAction(plan, DirectoryRenderActionKind::Gap, 0);
        return;
    }

    AppendAction(plan,
                 DirectoryRenderActionKind::GatePageDrawRoute,
                 route->drawFunction,
                 0,
                 0,
                 0,
                 0,
                 route->eventId,
                 route->workSlot,
                 1,
                 1,
                 true);
    if (route->workSlot >= 0) {
        AppendAction(plan,
                     DirectoryRenderActionKind::BeginDrawWork8001D74C,
                     kFn8001D74C,
                     0,
                     0,
                     0,
                     0,
                     route->workSlot);
    }

    AppendAction(plan,
                 DirectoryRenderActionKind::DrawPageFunction,
                 route->drawFunction,
                 0,
                 0,
                 0,
                 0,
                 route->eventId);
}

void AppendFixedAnchors(DirectoryRenderPlan& plan)
{
    for (uint32_t i = 0; i < KnownFixedAnchorSpecCount(); ++i) {
        const FixedAnchorSpec& spec = KnownFixedAnchorSpecAt(i);
        if (!PageMatches(spec.page, plan.page)) {
            continue;
        }

        AppendAction(plan,
                     DirectoryRenderActionKind::FixedSprite,
                     plan.eventId == 16 ? kFn80023618 : 0,
                     spec.tableAddress,
                     spec.template0,
                     spec.x,
                     spec.y,
                     static_cast<int32_t>(spec.template1),
                     static_cast<int32_t>(spec.template2));
    }
}

void AppendLanguageTables(DirectoryRenderPlan& plan)
{
    for (uint32_t i = 0; i < KnownLanguageTableSpecCount(); ++i) {
        const LanguageTableSpec& spec = KnownLanguageTableSpecAt(i);
        if (!PageMatches(spec.page, plan.page)) {
            continue;
        }

        AppendAction(plan,
                     DirectoryRenderActionKind::LanguageTextTable,
                     0,
                     spec.baseAddress,
                     0,
                     0,
                     0,
                     static_cast<int32_t>(spec.strideBytes),
                     static_cast<int32_t>(spec.entryCount),
                     static_cast<int32_t>(spec.kind));
    }
}

void AppendSharedExit(DirectoryRenderPlan& plan)
{
    AppendAction(plan,
                 DirectoryRenderActionKind::SharedExit,
                 kFn8001C550,
                 kTableSharedExitText80053000,
                 0,
                 231,
                 179,
                 plan.eventId);
    AppendAction(plan,
                 DirectoryRenderActionKind::SharedExitIconTemplate,
                 kFn8001C550,
                 kGpSharedExitIconSlotOn8006EB10,
                 kSpriteSharedExitIconOn80050AD0,
                 231,
                 179,
                 kSharedExitIconGpSlotOn,
                 1,
                 static_cast<int32_t>(kGpBase8006EA40),
                 plan.eventId,
                 true);
    AppendAction(plan,
                 DirectoryRenderActionKind::SharedExitIconTemplate,
                 kFn8001C550,
                 kGpSharedExitIconSlotOff8006EB14,
                 kSpriteSharedExitIconOff80050AC0,
                 231,
                 179,
                 kSharedExitIconGpSlotOff,
                 0,
                 static_cast<int32_t>(kGpBase8006EA40),
                 plan.eventId,
                 true);
}

void AppendDirectoryGp0ReplayBaselines(DirectoryRenderPlan& plan)
{
    for (uint32_t i = 0; i < sizeof(kDirectoryGp0ReplayBaselines) /
                                 sizeof(kDirectoryGp0ReplayBaselines[0]);
         ++i) {
        const DirectoryGp0ReplayBaselineSpec& spec =
            kDirectoryGp0ReplayBaselines[i];
        if (spec.page != plan.page || spec.eventId != plan.eventId) {
            continue;
        }

        AppendAction(plan,
                     DirectoryRenderActionKind::DirectoryGp0ReplayBaseline,
                     spec.eventId == 17 ? kFn80021910 :
                         (spec.eventId == 16 ? kFn80023618 :
                          (spec.eventId == 7 || spec.eventId == 8 ||
                           spec.eventId == 9 ? kFn80020F94 :
                           (spec.eventId == 2 ? kFn80020568 : kFn80021E60))),
                     0,
                     0,
                     0,
                     0,
                     spec.frame,
                     static_cast<int32_t>(spec.packetCount),
                     static_cast<int32_t>(spec.flatWordCount),
                     static_cast<int32_t>(i),
                     true);
    }
}

MainDirectorySpriteTemplate8001B25C ResolveTemplate8001B25C(
    const StaticTemplateFamily8001B25C& family,
    uint32_t state) {
    MainDirectorySpriteTemplate8001B25C out{};
    if (family.firstAddress == 0u || state >= family.stateCount) {
        return out;
    }
    out.known = true;
    out.psxAddress = family.firstAddress + state * 0x10u;
    out.attr = family.attr;
    out.texX = family.texX;
    out.texY = family.texY;
    out.width = family.width;
    out.height = family.height;
    out.clutX = family.clutX;
    out.clutY = static_cast<uint16_t>(family.firstClutY + state);
    return out;
}

const StaticTemplateFamily8001B25C* FindFixedFamily8001B25C(
    uint32_t firstAddress) {
    for (const auto& family : kMainDirectoryFixedFamilies) {
        if (family.firstAddress == firstAddress) {
            return &family;
        }
    }
    return nullptr;
}

bool AppendMainDirectorySprite80021E60(
    MainDirectoryDrawList80021E60& out,
    int16_t x,
    int16_t y,
    const StaticTemplateFamily8001B25C& family,
    uint32_t state,
    uint16_t priority) {
    if (out.count >= kMainDirectorySpriteCapacity80021E60) {
        out.truncated = true;
        return false;
    }
    MainDirectorySpriteCommand80021E60 command{};
    command.sprite = ResolveTemplate8001B25C(family, state);
    if (!command.sprite.known) {
        return false;
    }
    command.known = true;
    command.x = x;
    command.y = y;
    command.priority = priority;
    command.callOrder = out.count;
    out.commands[out.count++] = command;
    out.rawTextureOnly = out.rawTextureOnly &&
                         (command.sprite.attr & 0x40u) != 0u;
    return true;
}

bool AppendMainDirectoryFixed80021E60(
    MainDirectoryDrawList80021E60& out,
    int16_t x,
    int16_t y,
    uint32_t firstAddress,
    uint32_t state,
    uint16_t priority) {
    const auto* family = FindFixedFamily8001B25C(firstAddress);
    return family != nullptr &&
           AppendMainDirectorySprite80021E60(
               out, x, y, *family, state, priority);
}

bool AppendMainDirectoryLanguage80021E60(
    MainDirectoryDrawList80021E60& out,
    const StaticLanguageSpriteFamily80021E60 (&families)[5],
    int32_t language,
    uint32_t state,
    uint16_t priority) {
    if (language < 0 || language >= 5) {
        return false;
    }
    const auto& family = families[language];
    return AppendMainDirectorySprite80021E60(
        out, family.x, family.y, family.templates, state, priority);
}

bool IsKnownMainDirectoryState80021E60(
    const MainDirectoryState80021E60& state) {
    // 80021E60 uses language as an unchecked table index on both the null-ctx
    // and context-present paths. Cursor is compared only with rows 0..4 and
    // blink is compared only with 1, so other known values select the
    // original all-unselected / blink-off defaults instead of rejecting the
    // complete page.
    return state.language >= 0 && state.language < 5;
}

const StaticTemplateFamily8001B25C* FindOptionsFixedFamily80021910(
    uint32_t firstAddress) {
    for (const auto& family : kOptionsFixedFamilies80021910) {
        if (family.firstAddress == firstAddress) {
            return &family;
        }
    }
    return nullptr;
}

bool AppendOptionsSprite80021910(
    OptionsDrawList80021910& out,
    int16_t x,
    int16_t y,
    const StaticTemplateFamily8001B25C& family,
    uint32_t state,
    uint16_t priority) {
    if (out.count >= kOptionsSpriteCapacity80021910) {
        out.truncated = true;
        return false;
    }
    OptionsSpriteCommand80021910 command{};
    command.sprite = ResolveTemplate8001B25C(family, state);
    if (!command.sprite.known) {
        return false;
    }
    command.known = true;
    command.x = x;
    command.y = y;
    command.priority = priority;
    command.callOrder = out.count;
    out.commands[out.count++] = command;
    out.rawTextureOnly = out.rawTextureOnly &&
                         (command.sprite.attr & 0x40u) != 0u;
    return true;
}

bool AppendOptionsFixed80021910(OptionsDrawList80021910& out,
                                int16_t x,
                                int16_t y,
                                uint32_t firstAddress,
                                uint32_t state,
                                uint16_t priority) {
    const auto* family = FindOptionsFixedFamily80021910(firstAddress);
    return family != nullptr &&
           AppendOptionsSprite80021910(
               out, x, y, *family, state, priority);
}

bool AppendOptionsTableEntry80021910(
    OptionsDrawList80021910& out,
    const StaticLanguageSpriteFamily80021E60 (&families)[5],
    int32_t index,
    uint32_t state,
    uint16_t priority) {
    if (index < 0 || index >= 5) {
        return false;
    }
    const auto& family = families[index];
    return AppendOptionsSprite80021910(
        out, family.x, family.y, family.templates, state, priority);
}

bool IsKnownOptionsState80021910(const OptionsState80021910& state) {
    // 80021910 uses language as an unchecked table index. Every context
    // field is instead consumed only by equality/nonzero predicates, and the
    // third item value at ctx+0x14 is not read by the renderer at all.
    return state.language >= 0 && state.language < 5;
}

const StaticCardGridTemplate80020F94* FindCardGridTemplate80020F94(
    uint32_t address) {
    for (const auto& source : kCardGridTemplates80020F94) {
        if (source.address == address) {
            return &source;
        }
    }
    return nullptr;
}

MainDirectorySpriteTemplate8001B25C ResolveCardGridTemplate80020F94(
    uint32_t address) {
    const auto* source = FindCardGridTemplate80020F94(address);
    if (source == nullptr) {
        return {};
    }
    MainDirectorySpriteTemplate8001B25C out{};
    out.known = true;
    out.psxAddress = source->address;
    out.attr = source->attr;
    out.texX = source->texX;
    out.texY = source->texY;
    out.width = source->width;
    out.height = source->height;
    out.clutX = source->clutX;
    out.clutY = source->clutY;
    return out;
}

const StaticCardGridLanguageRecord80020F94*
FindCardGridLanguageRecord80020F94(int32_t eventId, int32_t language) {
    for (const auto& record : kCardGridLanguageRecords80020F94) {
        if (record.eventId == eventId && record.language == language) {
            return &record;
        }
    }
    return nullptr;
}

bool AppendCardGridTemplate80020F94(
    CardGridDrawList80020F94& out,
    CardGridSpriteRole80020F94 role,
    uint32_t psxFunction,
    uint32_t sourceCallOrder,
    int16_t x,
    int16_t y,
    uint32_t templateAddress) {
    if (out.count >= out.commands.size()) {
        out.truncated = true;
        return false;
    }
    CardGridSpriteCommand80020F94 command{};
    command.sprite = ResolveCardGridTemplate80020F94(templateAddress);
    if (!command.sprite.known || command.sprite.width == 0u ||
        command.sprite.height == 0u) {
        return false;
    }
    command.known = true;
    command.role = role;
    command.psxFunction = psxFunction;
    command.sourceCallOrder = sourceCallOrder;
    command.x = x;
    command.y = y;
    command.priority = 0u;
    command.callOrder = out.count;
    out.commands[out.count++] = command;
    out.rawTextureOnly = out.rawTextureOnly &&
                         (command.sprite.attr & 0x40u) != 0u;
    return true;
}

int16_t ReadS16LE80020F94(const uint8_t* bytes) {
    const uint16_t value = static_cast<uint16_t>(bytes[0]) |
        (static_cast<uint16_t>(bytes[1]) << 8u);
    return static_cast<int16_t>(value);
}

bool AppendCardGridGlyph8001B744(CardGridDrawList80020F94& out,
                                 uint8_t glyphCode,
                                 uint32_t sourceCallOrder,
                                 int16_t originX,
                                 int16_t originY,
                                 bool dim,
                                 int32_t& penX) {
    PrPsxTextGlyphMetricsDirect::GlyphMetricRaw8004945C raw{};
    if (!PrPsxTextGlyphMetricsDirect::TryCopyGlyphMetricRaw8004945C(
            glyphCode, raw)) {
        return false;
    }
    const int16_t textureX = ReadS16LE80020F94(raw.data());
    const int16_t textureY = ReadS16LE80020F94(raw.data() + 2u);
    const uint16_t width = raw[4u];
    const uint16_t height = raw[5u];
    const int16_t xOffset = ReadS16LE80020F94(raw.data() + 6u);
    const int32_t penBefore = penX;
    penX += static_cast<int32_t>(width);
    if (textureY < 0) {
        ++out.glyphNoPacketCount;
        return true;
    }
    if (width == 0u || height == 0u ||
        out.count >= out.commands.size()) {
        out.truncated = out.count >= out.commands.size();
        return false;
    }

    const int32_t roundedUBase = textureX <= 0
        ? static_cast<int32_t>(textureX) + 3584
        : static_cast<int32_t>(textureX) + 3583;
    const uint16_t uvWord = static_cast<uint16_t>(roundedUBase);
    const uint16_t helperA3 =
        static_cast<uint16_t>((uvWord & 0xFF00u) >> 2u);
    const uint16_t helperA4 = static_cast<uint16_t>(
        static_cast<uint16_t>(static_cast<int32_t>(textureY) + 256) &
        0xFF00u);

    CardGridSpriteCommand80020F94 command{};
    command.known = true;
    command.role = CardGridSpriteRole80020F94::SlotGlyph;
    command.psxFunction = kFn8001C668;
    command.sourceCallOrder = sourceCallOrder;
    command.textureCoordinatesResolved = true;
    command.x = static_cast<int16_t>(
        static_cast<int32_t>(originX) + penBefore + xOffset);
    command.y = static_cast<int16_t>(
        static_cast<int32_t>(originY) +
        (glyphCode >= 192u && glyphCode <= 222u ? -3 : 0));
    command.priority = 0u;
    command.callOrder = out.count;
    command.sprite.known = true;
    command.sprite.psxAddress = kFn8001B744;
    command.sprite.attr = kMainDirectoryRawTextureAttr80021E60;
    command.sprite.width = width;
    command.sprite.height = height;
    command.sprite.clutX = 256u;
    command.sprite.clutY = static_cast<uint16_t>(482u + (dim ? 1u : 0u));
    command.tpage = static_cast<uint16_t>(
        0x20u | ((helperA4 & 0x0100u) >> 4u) |
        ((helperA3 & 0x03FFu) >> 6u) | (4u * (helperA4 & 0x0200u)));
    command.u = static_cast<uint8_t>(uvWord);
    command.v = static_cast<uint8_t>(textureY);
    command.glyphCode = glyphCode;
    out.commands[out.count++] = command;
    ++out.glyphSpriteCount;
    out.rawTextureOnly = out.rawTextureOnly &&
                         (command.sprite.attr & 0x40u) != 0u;
    return true;
}

bool IsKnownCardGridState80020F94(const CardGridState80020F94& state) {
    // 80020F94 consumes ctx+0/+4/+8 and each enabled halfword only as
    // zero/nonzero predicates. The remaining fields still own table, loop,
    // or backing-array indexes and retain their proven host bounds.
    if (!state.requestBound ||
        state.argAddress != kCardGridArgAddress80048E50 ||
        state.eventId < 7 || state.eventId > 9 ||
        state.language < 0 || state.language >= 5 ||
        state.rows <= 0 || state.columns <= 0) {
        return false;
    }
    const int32_t slotCount =
        static_cast<int32_t>(state.rows) * state.columns;
    if (slotCount <= 0 ||
        slotCount > static_cast<int32_t>(kCardGridItemCapacity80020F94) ||
        state.itemCount < 0 || state.itemCount > slotCount ||
        (state.selected != slotCount &&
         (state.selected < 0 || state.selected >= state.itemCount))) {
        return false;
    }
    for (int32_t i = 0; i < state.itemCount; ++i) {
        if (std::memchr(state.slotText[static_cast<std::size_t>(i)].data(),
                        '\0',
                        kCardGridSlotTextCapacity80020F94) == nullptr) {
            return false;
        }
    }
    return FindCardGridLanguageRecord80020F94(
               state.eventId, state.language) != nullptr;
}

MainDirectorySpriteTemplate8001B25C ResolveStageSelectTemplate80020568(
    uint32_t address) {
    for (const auto& source : kStageSelectTemplates80020568) {
        if (source.address != address) {
            continue;
        }
        MainDirectorySpriteTemplate8001B25C out{};
        out.known = true;
        out.psxAddress = source.address;
        out.attr = source.attr;
        out.texX = source.texX;
        out.texY = source.texY;
        out.width = source.width;
        out.height = source.height;
        out.clutX = source.clutX;
        out.clutY = source.clutY;
        return out;
    }
    return {};
}

bool AppendStageSelectSprite80020568(StageSelectDrawList80020568& out,
                                     int16_t x,
                                     int16_t y,
                                     uint32_t templateAddress,
                                     uint16_t priority) {
    if (out.count >= kStageSelectSpriteCapacity80020568) {
        out.truncated = true;
        return false;
    }
    StageSelectSpriteCommand80020568 command{};
    command.sprite = ResolveStageSelectTemplate80020568(templateAddress);
    if (!command.sprite.known) {
        return false;
    }
    command.known = true;
    command.x = x;
    command.y = y;
    command.priority = priority;
    command.callOrder = out.count;
    out.commands[out.count++] = command;
    out.rawTextureOnly = out.rawTextureOnly &&
                         (command.sprite.attr & 0x40u) != 0u;
    return true;
}

uint16_t StageSelectResolvedTpage8001B654(uint16_t texX,
                                         uint16_t texY,
                                         int16_t uOffset) {
    const uint16_t uvWord = static_cast<uint16_t>(
        4u * texX + static_cast<uint16_t>(uOffset));
    const uint16_t helperA3 = static_cast<uint16_t>(
        (uvWord & 0xFF00u) >> 2u);
    const uint16_t helperA4 = static_cast<uint16_t>(texY & 0xFF00u);
    return static_cast<uint16_t>(
        0x20u |
        ((helperA4 & 0x0100u) >> 4u) |
        ((helperA3 & 0x03FFu) >> 6u) |
        (4u * (helperA4 & 0x0200u)));
}

bool AppendStageSelectSlice8001C604(StageSelectDrawList80020568& out,
                                    int16_t x,
                                    int16_t y,
                                    int16_t uOffset,
                                    int16_t width,
                                    int32_t state,
                                    uint16_t priority) {
    if (out.count >= kStageSelectSpriteCapacity80020568) {
        out.truncated = true;
        return false;
    }
    if (uOffset < 0 || width <= 0 || state < 0 || state > 2) {
        return false;
    }

    StageSelectSpriteCommand80020568 command{};
    command.sprite = ResolveStageSelectTemplate80020568(0x80051BF0u);
    if (!command.sprite.known) {
        return false;
    }
    const uint16_t uvWord = static_cast<uint16_t>(
        4u * command.sprite.texX + static_cast<uint16_t>(uOffset));
    command.known = true;
    command.textureCoordinatesResolved = true;
    command.x = x;
    command.y = y;
    command.priority = priority;
    command.callOrder = out.count;
    command.sprite.width = static_cast<uint16_t>(width);
    command.sprite.clutY = static_cast<uint16_t>(
        static_cast<uint32_t>(command.sprite.clutY) +
        static_cast<uint32_t>(state));
    command.tpage = StageSelectResolvedTpage8001B654(
        command.sprite.texX, command.sprite.texY, uOffset);
    command.u = static_cast<uint8_t>(uvWord);
    command.v = static_cast<uint8_t>(command.sprite.texY);
    out.commands[out.count++] = command;
    out.rawTextureOnly = out.rawTextureOnly &&
                         (command.sprite.attr & 0x40u) != 0u;
    return true;
}

struct PracticeLanguageRecord80023618 {
    uint32_t templateAddress;
    int16_t x;
    int16_t y;
};

constexpr PracticeLanguageRecord80023618 kPracticeCaption80053B90[5] = {
    {0x80052A20u, 39, 49}, {0x80052B80u, 40, 46},
    {0x80052AD0u, 33, 50}, {0x80052C30u, 37, 49},
    {0x80052CE0u, 36, 46},
};
constexpr PracticeLanguageRecord80023618 kPracticeSubtitle80053BB8[5] = {
    {0x80052A30u, 136, 40}, {0x80052B90u, 139, 40},
    {0x80052AE0u, 128, 40}, {0x80052C40u, 137, 40},
    {0x80052CF0u, 142, 40},
};
constexpr PracticeLanguageRecord80023618 kPracticeCase8B80053BE0[5] = {
    {0x80052A40u, 135, 39}, {0x80052BA0u, 137, 40},
    {0x80052AF0u, 125, 42}, {0x80052C50u, 123, 41},
    {0x80052D00u, 122, 40},
};
constexpr PracticeLanguageRecord80023618 kPracticeCase0A80053C08[5] = {
    {0x80052A50u, 130, 135}, {0x80052BB0u, 128, 134},
    {0x80052B00u, 130, 134}, {0x80052C60u, 130, 134},
    {0x80052D10u, 111, 133},
};
constexpr PracticeLanguageRecord80023618 kPracticeCase5A80053C30[5] = {
    {0x80052A60u, 120, 129}, {0x80052BC0u, 105, 137},
    {0x80052B10u, 103, 129}, {0x80052C70u, 118, 127},
    {0x80052D20u, 105, 127},
};
constexpr PracticeLanguageRecord80023618 kPracticeCase4A80053C58[5] = {
    {0x80052A70u, 110, 137}, {0x80052BD0u, 113, 127},
    {0x80052B20u, 105, 128}, {0x80052C80u, 106, 136},
    {0x80052D30u, 121, 136},
};
constexpr PracticeLanguageRecord80023618 kPracticeCase7A80053C80[5] = {
    {0x80052A80u, 104, 118}, {0x80052BE0u, 104, 118},
    {0x80052B30u, 115, 119}, {0x80052C90u, 104, 117},
    {0x80052D40u, 103, 117},
};
constexpr PracticeLanguageRecord80023618 kPracticeCase1A80053CA8[5] = {
    {0x80052A90u, 139, 143}, {0x80052BF0u, 131, 143},
    {0x80052B40u, 134, 145}, {0x80052CA0u, 131, 144},
    {0x80052D50u, 132, 144},
};
constexpr PracticeLanguageRecord80023618 kPracticeCase2A80053CD0[5] = {
    {0x80052AA0u, 139, 143}, {0x80052C00u, 129, 143},
    {0x80052B50u, 141, 144}, {0x80052CB0u, 137, 144},
    {0x80052D60u, 137, 144},
};
constexpr PracticeLanguageRecord80023618 kPracticeCase6A80053CF8[5] = {
    {0x80052AB0u, 127, 129}, {0x80052C10u, 105, 137},
    {0x80052B60u, 119, 128}, {0x80052CC0u, 115, 135},
    {0x80052D70u, 103, 136},
};
constexpr PracticeLanguageRecord80023618 kPracticeCase12B80053D20[5] = {
    {0x80052AC0u, 113, 125}, {0x80052C20u, 113, 125},
    {0x80052B70u, 121, 126}, {0x80052CD0u, 119, 126},
    {0x80052D80u, 121, 126},
};
constexpr PracticeLanguageRecord80023618 kPracticeTitle80053D48[5] = {
    {0x80052A00u, 204, 55}, {0x80052A00u, 206, 55},
    {0x80052A00u, 257, 55}, {0x80052A00u, 249, 55},
    {0x80052A00u, 244, 55},
};
constexpr PracticeLanguageRecord80023618 kPracticeCase8A80053D70[5] = {
    {0x80052A10u, 194, 55}, {0x80052A10u, 206, 55},
    {0x80052A10u, 264, 55}, {0x80052A10u, 242, 57},
    {0x80052A10u, 236, 57},
};

struct PracticeExitRecord80023618 {
    uint32_t templateOff;
    uint32_t templateOn;
    int16_t x;
    int16_t y;
};

constexpr PracticeExitRecord80023618 kPracticeExit80053000[5] = {
    {0x800509E0u, 0x800509F0u, 242, 191},
    {0x80050A40u, 0x80050A50u, 238, 193},
    {0x80050A10u, 0x80050A20u, 238, 193},
    {0x80050A70u, 0x80050A80u, 241, 190},
    {0x80050AA0u, 0x80050AB0u, 239, 191},
};

struct StaticPracticeLeadingTemplate80023618 {
    uint32_t attr;
    uint16_t texX;
    uint16_t texY;
    uint16_t width;
    uint16_t height;
    uint16_t clutX;
    uint16_t clutY;
};

constexpr uint32_t kPracticeLeadingTemplateBase80023618 = 0x80052A00u;
constexpr StaticPracticeLeadingTemplate80023618
    kPracticeLeadingTemplates80023618[] = {
        {0x50000040u, 0x01F6u, 0x0014u, 16u, 13u, 0x01C0u, 0x010Fu},
        {0x50000040u, 0x01FAu, 0x0014u, 16u, 13u, 0x01C0u, 0x0110u},
        {0x50000040u, 0x022Du, 0x0000u, 64u, 12u, 0x0200u, 0x0100u},
        {0x50000040u, 0x0240u, 0x0000u, 132u, 31u, 0x0240u, 0x0100u},
        {0x50000040u, 0x0240u, 0x001Fu, 136u, 28u, 0x0240u, 0x0100u},
        {0x50000040u, 0x0240u, 0x003Bu, 52u, 16u, 0x0240u, 0x0100u},
        {0x50000040u, 0x0240u, 0x004Bu, 72u, 29u, 0x0240u, 0x0100u},
        {0x50000040u, 0x0240u, 0x0068u, 92u, 13u, 0x0240u, 0x0100u},
        {0x50000040u, 0x0240u, 0x0075u, 104u, 50u, 0x0240u, 0x0100u},
        {0x50000040u, 0x0240u, 0x00A7u, 32u, 16u, 0x0240u, 0x0100u},
        {0x50000040u, 0x0240u, 0x00B7u, 28u, 12u, 0x0240u, 0x0100u},
        {0x50000040u, 0x0240u, 0x00C3u, 56u, 29u, 0x0240u, 0x0100u},
        {0x50000040u, 0x0240u, 0x00E0u, 84u, 13u, 0x0240u, 0x0100u},
        {0x50000040u, 0x022Du, 0x001Bu, 76u, 11u, 0x0200u, 0x0100u},
        {0x50000040u, 0x0280u, 0x0075u, 148u, 31u, 0x0240u, 0x0100u},
        {0x50000040u, 0x0280u, 0x0094u, 156u, 28u, 0x0240u, 0x0100u},
        {0x50000040u, 0x0280u, 0x00B0u, 52u, 15u, 0x0240u, 0x0100u},
        {0x50000040u, 0x0280u, 0x00BFu, 104u, 29u, 0x0240u, 0x0100u},
        {0x50000040u, 0x0280u, 0x00DCu, 100u, 29u, 0x0240u, 0x0100u},
        {0x50000040u, 0x02A7u, 0x0075u, 80u, 48u, 0x0240u, 0x0100u},
        {0x50000040u, 0x02A7u, 0x00A5u, 40u, 14u, 0x0240u, 0x0100u},
        {0x50000040u, 0x02B1u, 0x00A5u, 28u, 12u, 0x0240u, 0x0100u},
        {0x50000040u, 0x02A5u, 0x00B3u, 72u, 28u, 0x0240u, 0x0100u},
        {0x50000040u, 0x02A5u, 0x00CFu, 68u, 15u, 0x0240u, 0x0100u},
        {0x50000040u, 0x022Du, 0x000Cu, 60u, 15u, 0x0200u, 0x0100u},
        {0x50000040u, 0x0280u, 0x0000u, 132u, 31u, 0x0240u, 0x0100u},
        {0x50000040u, 0x0280u, 0x001Fu, 128u, 28u, 0x0240u, 0x0100u},
        {0x50000040u, 0x0280u, 0x003Bu, 56u, 15u, 0x0240u, 0x0100u},
        {0x50000040u, 0x0280u, 0x004Au, 100u, 13u, 0x0240u, 0x0100u},
        {0x50000040u, 0x0280u, 0x0057u, 84u, 30u, 0x0240u, 0x0100u},
        {0x50000040u, 0x02A1u, 0x0000u, 104u, 50u, 0x0240u, 0x0100u},
        {0x50000040u, 0x02A1u, 0x0032u, 48u, 16u, 0x0240u, 0x0100u},
        {0x50000040u, 0x02ADu, 0x0032u, 52u, 16u, 0x0240u, 0x0100u},
        {0x50000040u, 0x02A1u, 0x0042u, 100u, 11u, 0x0240u, 0x0100u},
        {0x50000040u, 0x02A1u, 0x004Du, 84u, 13u, 0x0240u, 0x0100u},
        {0x50000040u, 0x022Du, 0x0026u, 68u, 12u, 0x0200u, 0x0100u},
        {0x50000040u, 0x02C0u, 0x0000u, 132u, 31u, 0x0240u, 0x0100u},
        {0x50000040u, 0x02C0u, 0x001Fu, 160u, 28u, 0x0240u, 0x0100u},
        {0x50000040u, 0x02E8u, 0x001Fu, 52u, 15u, 0x0240u, 0x0100u},
        {0x50000040u, 0x02E1u, 0x0000u, 76u, 31u, 0x0240u, 0x0100u},
        {0x50000040u, 0x02DAu, 0x003Bu, 100u, 15u, 0x0240u, 0x0100u},
        {0x50000040u, 0x02C0u, 0x003Bu, 104u, 49u, 0x0240u, 0x0100u},
        {0x50000040u, 0x02DAu, 0x004Au, 48u, 12u, 0x0240u, 0x0100u},
        {0x50000040u, 0x02E6u, 0x004Au, 36u, 12u, 0x0240u, 0x0100u},
        {0x50000040u, 0x02DAu, 0x0056u, 80u, 13u, 0x0240u, 0x0100u},
        {0x50000040u, 0x02EEu, 0x0056u, 72u, 15u, 0x0240u, 0x0100u},
        {0x50000040u, 0x022Du, 0x0032u, 72u, 15u, 0x0200u, 0x0100u},
        {0x50000040u, 0x02C0u, 0x006Cu, 120u, 31u, 0x0240u, 0x0100u},
        {0x50000040u, 0x02C0u, 0x008Bu, 160u, 28u, 0x0240u, 0x0100u},
        {0x50000040u, 0x02DEu, 0x006Cu, 88u, 20u, 0x0240u, 0x0100u},
        {0x50000040u, 0x02C0u, 0x00A7u, 100u, 28u, 0x0240u, 0x0100u},
        {0x50000040u, 0x02E8u, 0x008Bu, 68u, 12u, 0x0240u, 0x0100u},
        {0x50000040u, 0x02C0u, 0x00C3u, 104u, 50u, 0x0240u, 0x0100u},
        {0x50000040u, 0x02D9u, 0x00A7u, 48u, 15u, 0x0240u, 0x0100u},
        {0x50000040u, 0x02E5u, 0x00A7u, 36u, 12u, 0x0240u, 0x0100u},
        {0x50000040u, 0x02DAu, 0x00C3u, 104u, 15u, 0x0240u, 0x0100u},
        {0x50000040u, 0x02DAu, 0x00D2u, 68u, 12u, 0x0240u, 0x0100u},
        {0x50000040u, 0x01C0u, 0x0000u, 132u, 60u, 0x01C0u, 0x0100u},
        {0x50000040u, 0x01E1u, 0x0020u, 7u, 8u, 0x01C0u, 0x0101u},
        {0x50000040u, 0x01EFu, 0x0000u, 28u, 12u, 0x01C0u, 0x0104u},
        {0x50000040u, 0x01F6u, 0x0000u, 24u, 10u, 0x01C0u, 0x0105u},
        {0x50000040u, 0x01EEu, 0x0014u, 16u, 20u, 0x01C0u, 0x0106u},
        {0x50000040u, 0x01F2u, 0x0014u, 16u, 20u, 0x01C0u, 0x0107u},
        {0x50000040u, 0x01E8u, 0x000Cu, 28u, 8u, 0x01C0u, 0x0109u},
        {0x50000040u, 0x01EFu, 0x000Cu, 28u, 8u, 0x01C0u, 0x010Au},
        {0x50000040u, 0x01F6u, 0x000Cu, 28u, 8u, 0x01C0u, 0x010Bu},
        {0x50000040u, 0x01E1u, 0x0014u, 52u, 12u, 0x01C0u, 0x010Cu},
};

static_assert(sizeof(kPracticeLeadingTemplates80023618) /
                      sizeof(kPracticeLeadingTemplates80023618[0]) ==
                  67u,
              "80023618 leading descriptor range must stay SCUS-sized");

MainDirectorySpriteTemplate8001B25C
ResolvePracticeLeadingTemplate80023618(uint32_t address) {
    if (address < kPracticeLeadingTemplateBase80023618 ||
        ((address - kPracticeLeadingTemplateBase80023618) & 0x0Fu) != 0u) {
        return {};
    }
    const uint32_t index =
        (address - kPracticeLeadingTemplateBase80023618) / 0x10u;
    if (index >= sizeof(kPracticeLeadingTemplates80023618) /
                     sizeof(kPracticeLeadingTemplates80023618[0])) {
        return {};
    }
    const auto& source = kPracticeLeadingTemplates80023618[index];
    MainDirectorySpriteTemplate8001B25C out{};
    out.known = true;
    out.psxAddress = address;
    out.attr = source.attr;
    out.texX = source.texX;
    out.texY = source.texY;
    out.width = source.width;
    out.height = source.height;
    out.clutX = source.clutX;
    out.clutY = source.clutY;
    return out;
}

constexpr int32_t kPracticeGuideLargeThresholds80053B48[4] = {
    63, 119, 175, 231,
};
constexpr int32_t kPracticeGuideSmallThresholds80053B58[14] = {
    39, 53, 81, 95, 109, 137, 151,
    165, 193, 207, 221, 249, 263, 277,
};

const PracticeLanguageRecord80023618* ResolvePracticeLanguageRecord80023618(
    uint32_t tableBase,
    uint32_t language) {
    if (language >= 5u) {
        return nullptr;
    }
    switch (tableBase) {
    case 0x80053B90u: return &kPracticeCaption80053B90[language];
    case 0x80053BB8u: return &kPracticeSubtitle80053BB8[language];
    case 0x80053BE0u: return &kPracticeCase8B80053BE0[language];
    case 0x80053C08u: return &kPracticeCase0A80053C08[language];
    case 0x80053C30u: return &kPracticeCase5A80053C30[language];
    case 0x80053C58u: return &kPracticeCase4A80053C58[language];
    case 0x80053C80u: return &kPracticeCase7A80053C80[language];
    case 0x80053CA8u: return &kPracticeCase1A80053CA8[language];
    case 0x80053CD0u: return &kPracticeCase2A80053CD0[language];
    case 0x80053CF8u: return &kPracticeCase6A80053CF8[language];
    case 0x80053D20u: return &kPracticeCase12B80053D20[language];
    case 0x80053D48u: return &kPracticeTitle80053D48[language];
    case 0x80053D70u: return &kPracticeCase8A80053D70[language];
    default: break;
    }
    return nullptr;
}

uint32_t ResolvePracticeIconTemplate80024418(int32_t code) {
    switch (code) {
    case 1: return 0x8005405Cu;
    case 2: return 0x8005403Cu;
    case 3: return 0x8005404Cu;
    case 4: return 0x8005406Cu;
    case 5:
    case 6: return 0x8005401Cu;
    case 7:
    case 8: return 0x8005402Cu;
    default: break;
    }
    return 0u;
}

uint32_t ResolvePracticePortraitTemplate800246A8(int32_t mode) {
    switch (mode) {
    case 1: return 0x80053FFCu;
    case 2: return 0x80053FECu;
    case 3: return 0x80053FBCu;
    case 4: return 0x80053FCCu;
    case 5: return 0x80053FACu;
    case 7: return 0x80053FDCu;
    case 0:
    case 6:
    case 8:
    default: return 0x8005400Cu;
    }
}

struct PracticePortraitDescriptor80024600 {
    uint32_t attr;
    uint16_t texX;
    uint16_t texY;
    uint16_t width;
    uint16_t height;
    uint16_t clutX;
    uint16_t clutY;
};

bool ResolvePracticePortraitDescriptor80024600(
    uint32_t templateAddress,
    PracticePortraitDescriptor80024600& out) {
    switch (templateAddress) {
    case 0x80053FACu:
        out = {0x50000040u, 0x03F5u, 0x018Du, 16u, 16u,
               0x0120u, 0x01E4u};
        return true;
    case 0x80053FBCu:
        out = {0x50000040u, 0x03F9u, 0x018Du, 16u, 16u,
               0x0120u, 0x01E5u};
        return true;
    case 0x80053FCCu:
        out = {0x50000040u, 0x03F5u, 0x019Du, 16u, 16u,
               0x0120u, 0x01EFu};
        return true;
    case 0x80053FDCu:
        out = {0x50000040u, 0x03F9u, 0x019Du, 16u, 16u,
               0x0120u, 0x01F0u};
        return true;
    case 0x80053FECu:
        out = {0x50000040u, 0x03F5u, 0x01ADu, 16u, 16u,
               0x0120u, 0x01F1u};
        return true;
    case 0x80053FFCu:
        out = {0x50000040u, 0x03F9u, 0x01ADu, 16u, 16u,
               0x0120u, 0x01F2u};
        return true;
    case 0x8005400Cu:
        out = {0x50000040u, 0x03F5u, 0x01BDu, 16u, 16u,
               0x0120u, 0x01F3u};
        return true;
    default: break;
    }
    return false;
}

struct PracticeGuideDescriptor80023518 {
    uint32_t attr;
    uint16_t texX;
    uint16_t texY;
    uint16_t width;
    uint16_t height;
    uint16_t clutX;
    uint16_t clutY;
};

bool ResolvePracticeGuideDescriptor80023518(
    uint32_t templateAddress,
    PracticeGuideDescriptor80023518& out) {
    switch (templateAddress) {
    case 0x80050910u:
        out = {0x50000040u, 0x03FDu, 0x0181u, 8u, 8u,
               0x0120u, 0x01E0u};
        return true;
    case 0x80050920u:
        out = {0x50000040u, 0x03FDu, 0x0181u, 8u, 8u,
               0x0120u, 0x01E1u};
        return true;
    case 0x80050930u:
        out = {0x50000040u, 0x03FAu, 0x0181u, 12u, 12u,
               0x0120u, 0x01E2u};
        return true;
    case 0x80050940u:
        out = {0x50000040u, 0x03FAu, 0x0181u, 12u, 12u,
               0x0120u, 0x01E3u};
        return true;
    default: break;
    }
    return false;
}

struct PracticeIconDescriptor80024418 {
    uint16_t texX;
    uint16_t texY;
    uint16_t width;
    uint16_t height;
    uint16_t clutX;
    uint16_t clutY;
};

bool ResolvePracticeIconDescriptor80024418(
    int32_t code,
    PracticeIconDescriptor80024418& out) {
    switch (code) {
    case 1: out = {0x03F8u, 0x01DDu, 16u, 16u, 0x0120u, 0x01ECu}; return true;
    case 2: out = {0x03F8u, 0x01CDu, 16u, 16u, 0x0120u, 0x01E7u}; return true;
    case 3: out = {0x03FCu, 0x01CDu, 16u, 16u, 0x0120u, 0x01E8u}; return true;
    case 4: out = {0x03FCu, 0x01DDu, 16u, 16u, 0x0120u, 0x01EDu}; return true;
    case 5:
    case 6: out = {0x03F4u, 0x01CDu, 16u, 16u, 0x0120u, 0x01E6u}; return true;
    case 7:
    case 8: out = {0x03F4u, 0x01DDu, 16u, 16u, 0x0120u, 0x01E9u}; return true;
    default: break;
    }
    return false;
}

int16_t ResolvePracticeRsinStep25680023F20(int32_t phase) {
    static constexpr int16_t kRsin[32] = {
        0, 1567, 2896, 3784, 4096, 3784, 2896, 1567,
        0, -1567, -2896, -3784, -4096, -3784, -2896, -1567,
        0, 1567, 2896, 3784, 4096, 3784, 2896, 1567,
        0, -1567, -2896, -3784, -4096, -3784, -2896, -1567,
    };
    int32_t wrapped = phase % 8192;
    if (wrapped < 0) {
        wrapped += 8192;
    }
    return kRsin[(static_cast<uint32_t>(wrapped) / 256u) & 31u];
}

bool ResolvePracticeTraceStaticOperands80023618(
    PracticeDrawCallTrace80023618& trace,
    uint32_t language) {
    uint32_t rec44Template = 0u;
    for (uint32_t index = 0u; index < trace.count; ++index) {
        auto& call = trace.calls[index];
        switch (call.kind) {
        case PracticeDrawCallKind80023618::FastSprite8001C5A8: {
            if (call.positionSourceAddress == 0x80052FFCu &&
                !call.templatePointerIndirect) {
                call.positionResolved = true;
                call.resolvedX = 238;
                call.resolvedY = 188;
                call.templateResolved = true;
                call.resolvedTemplateAddress = call.templateSourceAddress;
                break;
            }
            const uint32_t exitRecord = 0x80053000u + language * 16u;
            if (call.positionSourceAddress == exitRecord + 12u) {
                const auto& record = kPracticeExit80053000[language];
                const bool selected = call.templateSourceAddress ==
                                      exitRecord + 8u;
                if (!selected && call.templateSourceAddress !=
                                     exitRecord + 4u) {
                    return false;
                }
                call.positionResolved = true;
                call.resolvedX = record.x;
                call.resolvedY = record.y;
                call.templateResolved = true;
                call.resolvedTemplateAddress = selected
                    ? record.templateOn
                    : record.templateOff;
                break;
            }
            const uint32_t tableBase =
                call.templateSourceAddress - language * 8u;
            const auto* record = ResolvePracticeLanguageRecord80023618(
                tableBase, language);
            if (record == nullptr || call.positionSourceAddress !=
                                     call.templateSourceAddress + 4u) {
                return false;
            }
            call.positionResolved = true;
            call.resolvedX = record->x;
            call.resolvedY = record->y;
            call.templateResolved = true;
            call.resolvedTemplateAddress = record->templateAddress;
            break;
        }
        case PracticeDrawCallKind80023618::Sprite8001C550: {
            const uint32_t captionRecord = 0x80053B90u + language * 8u;
            if (call.templatePointerIndirect &&
                call.templateSourceAddress == captionRecord) {
                const auto* record = ResolvePracticeLanguageRecord80023618(
                    0x80053B90u, language);
                if (record == nullptr || call.positionSourceAddress !=
                                         captionRecord + 4u) {
                    return false;
                }
                call.positionResolved = true;
                call.resolvedX = record->x;
                call.resolvedY = record->y;
                call.templateResolved = true;
                call.resolvedTemplateAddress = record->templateAddress;
                break;
            }
            call.positionResolved = true;
            call.resolvedX = static_cast<int16_t>(call.args[0]);
            call.resolvedY = static_cast<int16_t>(call.args[1]);
            call.templateResolved = true;
            if (call.templateSourceAddress ==
                kGpSharedExitIconSlotOn8006EB10) {
                call.resolvedTemplateAddress =
                    kSpriteSharedExitIconOn80050AD0;
            } else if (call.templateSourceAddress ==
                       kGpSharedExitIconSlotOff8006EB14) {
                call.resolvedTemplateAddress =
                    kSpriteSharedExitIconOff80050AC0;
            } else if (!call.templatePointerIndirect) {
                call.resolvedTemplateAddress = call.templateSourceAddress;
            } else {
                return false;
            }
            break;
        }
        case PracticeDrawCallKind80023618::Rec44Mode800246A8:
            rec44Template = ResolvePracticePortraitTemplate800246A8(
                call.args[0]);
            break;
        case PracticeDrawCallKind80023618::Rec44Draw80024600:
            if (rec44Template == 0u) {
                return false;
            }
            call.templateResolved = true;
            call.resolvedTemplateAddress = rec44Template;
            call.dynamicPositionRequired = call.args[4] > 0;
            break;
        case PracticeDrawCallKind80023618::Icon80024418:
            call.positionResolved = true;
            call.resolvedX = static_cast<int16_t>(call.args[0]);
            call.resolvedY = static_cast<int16_t>(call.args[1]);
            call.resolvedTemplateAddress =
                ResolvePracticeIconTemplate80024418(call.args[3]);
            call.templateResolved = call.resolvedTemplateAddress != 0u;
            if (!call.templateResolved) {
                return false;
            }
            {
                PracticeIconState80024418 iconState{};
                iconState.argumentsKnown = true;
                iconState.centerX = static_cast<int16_t>(call.args[0]);
                iconState.centerY = static_cast<int16_t>(call.args[1]);
                iconState.slotOrdinal = static_cast<int16_t>(call.args[2]);
                iconState.type = static_cast<int16_t>(call.args[3]);
                const auto icon = BuildPracticeIconSubmit80024418(iconState);
                if (!icon.argumentsAccepted || !icon.staticPrefixResolved ||
                    !icon.templateResolved) {
                    return false;
                }
                call.dynamicScaleRequired = true;
                call.scaleXSourceAddress = icon.scaleXSourceAddress;
                call.scaleYSourceAddress = icon.scaleYSourceAddress;
                call.helperStaticPrefixResolved = true;
            }
            break;
        case PracticeDrawCallKind80023618::Slice8001C604:
            call.positionResolved = true;
            call.resolvedX = static_cast<int16_t>(call.args[0]);
            call.resolvedY = static_cast<int16_t>(call.args[1]);
            call.templateResolved =
                call.templateSourceAddress == 0x80052DA0u;
            call.resolvedTemplateAddress = call.templateSourceAddress;
            if (!call.templateResolved) {
                return false;
            }
            break;
        case PracticeDrawCallKind80023618::None:
            return false;
        case PracticeDrawCallKind80023618::WobbleUpdate80023F20:
            break;
        case PracticeDrawCallKind80023618::Guide80023518: {
            PracticeGuideState80023518 guideState{};
            guideState.progressKnown = true;
            guideState.progress = call.args[0];
            const auto guide = BuildPracticeGuideDrawList80023518(guideState);
            if (!guide.accepted) {
                return false;
            }
            call.helperExpansionResolved = true;
            call.helperExpandedCallCount = guide.count;
            break;
        }
        }
    }
    return true;
}

bool IsKnownStageSelectState80020568(
    const StageSelectState80020568& state) {
    // 80020568 consumes ctx+0 and ctx+4 only as zero/nonzero predicates.
    // Cursor/status values still form renderer table or scratch-array indexes,
    // so keep their independently proven producer bounds.
    if (state.language < 0 || state.language >= 5 ||
        state.cursor < 1 || state.cursor > 8) {
        return false;
    }
    for (uint32_t index = 0; index < 6u; ++index) {
        if (state.rawStatus80092F1DTo23[index] > 3u) {
            return false;
        }
    }
    return true;
}

DirectoryRenderPlan BuildUnknownPlan(int32_t eventId)
{
    DirectoryRenderPlan plan =
        MakePlan("DirectoryPageDraw_UnknownEvent",
                 DirectoryPage::Unknown,
                 eventId);
    AppendAction(plan, DirectoryRenderActionKind::Gap, 0);
    return plan;
}

} // namespace

bool RuntimeCutoverAllowed()
{
    return false;
}

uint32_t KnownPageDrawRouteSpecCount()
{
    return sizeof(kPageRoutes) / sizeof(kPageRoutes[0]);
}

const PageDrawRouteSpec& KnownPageDrawRouteSpecAt(uint32_t index)
{
    if (index >= KnownPageDrawRouteSpecCount()) {
        index = KnownPageDrawRouteSpecCount() - 1u;
    }
    return kPageRoutes[index];
}

const PageDrawRouteSpec* FindPageDrawRouteSpec(int32_t eventId)
{
    for (uint32_t i = 0; i < KnownPageDrawRouteSpecCount(); ++i) {
        if (kPageRoutes[i].eventId == eventId) {
            return &kPageRoutes[i];
        }
    }
    return nullptr;
}

uint32_t KnownFixedAnchorSpecCount()
{
    return sizeof(kFixedAnchors) / sizeof(kFixedAnchors[0]);
}

const FixedAnchorSpec& KnownFixedAnchorSpecAt(uint32_t index)
{
    if (index >= KnownFixedAnchorSpecCount()) {
        index = KnownFixedAnchorSpecCount() - 1u;
    }
    return kFixedAnchors[index];
}

uint32_t KnownLanguageTableSpecCount()
{
    return sizeof(kLanguageTables) / sizeof(kLanguageTables[0]);
}

const LanguageTableSpec& KnownLanguageTableSpecAt(uint32_t index)
{
    if (index >= KnownLanguageTableSpecCount()) {
        index = KnownLanguageTableSpecCount() - 1u;
    }
    return kLanguageTables[index];
}

uint32_t KnownStagePointSpecCount()
{
    return sizeof(kStagePoints) / sizeof(kStagePoints[0]);
}

const StagePointSpec& KnownStagePointSpecAt(uint32_t index)
{
    if (index >= KnownStagePointSpecCount()) {
        index = KnownStagePointSpecCount() - 1u;
    }
    return kStagePoints[index];
}

uint32_t KnownStageSliceSpecCount()
{
    return sizeof(kStageSlices) / sizeof(kStageSlices[0]);
}

const StageSliceSpec& KnownStageSliceSpecAt(uint32_t index)
{
    if (index >= KnownStageSliceSpecCount()) {
        index = KnownStageSliceSpecCount() - 1u;
    }
    return kStageSlices[index];
}

CardGridSpec KnownCardGridSpec()
{
    return kCardGrid;
}

uint32_t KnownPracticeIconTemplateSpecCount()
{
    return sizeof(kPracticeIcons) / sizeof(kPracticeIcons[0]);
}

const PracticeIconTemplateSpec& KnownPracticeIconTemplateSpecAt(
    uint32_t index)
{
    if (index >= KnownPracticeIconTemplateSpecCount()) {
        index = KnownPracticeIconTemplateSpecCount() - 1u;
    }
    return kPracticeIcons[index];
}

uint32_t KnownDirectoryGp0ReplayBaselineSpecCount()
{
    return sizeof(kDirectoryGp0ReplayBaselines) /
           sizeof(kDirectoryGp0ReplayBaselines[0]);
}

const DirectoryGp0ReplayBaselineSpec& KnownDirectoryGp0ReplayBaselineSpecAt(
    uint32_t index)
{
    if (index >= KnownDirectoryGp0ReplayBaselineSpecCount()) {
        index = KnownDirectoryGp0ReplayBaselineSpecCount() - 1u;
    }
    return kDirectoryGp0ReplayBaselines[index];
}

DirectoryRenderPlan BuildMainDirectory80021E60Plan(uint32_t ctxAddress,
                                                   bool ctxKnown)
{
    DirectoryRenderPlan plan =
        MakePlan("MainDirectory80021E60",
                 DirectoryPage::MainDirectoryEv3,
                 3);
    plan.blockedByGap = true;
    const bool ctxSourceKnown = ctxKnown && ctxAddress != 0u;
    const uint32_t stateBase = ctxSourceKnown ? ctxAddress + 0x10u : 0u;

    AppendRoute(plan);
    AppendFixedAnchors(plan);
    AppendLanguageTables(plan);
    AppendAction(plan,
                 DirectoryRenderActionKind::GateMainDirectoryStateSource80021E60,
                 kFn80021E60,
                 stateBase,
                 0,
                 0,
                 0,
                 static_cast<int32_t>(ctxAddress),
                 0x10,
                 4,
                 ctxSourceKnown ? 1 : 0,
                 true);
    AppendAction(plan,
                 DirectoryRenderActionKind::StateField,
                 kFn80021E60,
                 stateBase,
                 0,
                 0,
                 0,
                 0x10,
                 4,
                 -1,
                 0);
    AppendSharedExit(plan);
    AppendDirectoryGp0ReplayBaselines(plan);
    AppendAction(plan, DirectoryRenderActionKind::Gap, kFn80021E60);
    return plan;
}

MainDirectoryDrawList80021E60 BuildMainDirectoryDrawList80021E60(
    const MainDirectoryState80021E60& state) {
    MainDirectoryDrawList80021E60 out{};
    out.rawTextureOnly = true;
    out.sourceKnown = IsKnownMainDirectoryState80021E60(state);
    if (!out.sourceKnown) {
        return out;
    }

    const uint32_t blinkState = state.blinkOnOff == 1 ? 1u : 0u;
    bool ok = true;
    if (!state.contextPresent) {
        ok = AppendMainDirectoryFixed80021E60(out, 28, 36, 0x80051010u, 0u, 1u) && ok;
        ok = AppendMainDirectoryFixed80021E60(out, 110, 56, 0x80051150u, 0u, 1u) && ok;
        ok = AppendMainDirectoryFixed80021E60(out, 207, 34, 0x80051290u, 0u, 1u) && ok;
        ok = AppendMainDirectoryFixed80021E60(out, 26, 106, 0x800514F0u, 0u, 1u) && ok;
        ok = AppendMainDirectoryFixed80021E60(out, 94, 122, 0x80051630u, 0u, 1u) && ok;
        ok = AppendMainDirectoryFixed80021E60(out, 162, 122, 0x80051770u, 0u, 1u) && ok;
        ok = AppendMainDirectoryFixed80021E60(out, 226, 106, 0x800518B0u, 0u, 1u) && ok;
        ok = AppendMainDirectoryFixed80021E60(out, 231, 179, 0x80050AC0u, 0u, 1u) && ok;
        ok = AppendMainDirectoryLanguage80021E60(out, kMainLanguageText, 0, 0u, 1u) && ok;
        ok = AppendMainDirectoryLanguage80021E60(out, kMainHiScoreText, state.language, 0u, 1u) && ok;
        ok = AppendMainDirectoryLanguage80021E60(out, kMainNormalText, state.language, 0u, 1u) && ok;
        ok = AppendMainDirectoryLanguage80021E60(out, kMainEasyText, state.language, 0u, 1u) && ok;
        ok = AppendMainDirectoryLanguage80021E60(out, kMainPracticeText, state.language, 0u, 1u) && ok;
        ok = AppendMainDirectoryLanguage80021E60(out, kMainStageSelectText, state.language, 0u, 1u) && ok;
        ok = AppendMainDirectoryLanguage80021E60(out, kMainReplayText, state.language, 0u, 1u) && ok;
        ok = AppendMainDirectoryLanguage80021E60(out, kMainLoadText, state.language, 0u, 1u) && ok;
        ok = AppendMainDirectoryLanguage80021E60(out, kMainExitText, state.language, 0u, 1u) && ok;
        ok = AppendMainDirectoryFixed80021E60(out, 37, 47, 0x80050EF0u, 0u, 1u) && ok;
        ok = AppendMainDirectoryFixed80021E60(out, 118, 65, 0x80051030u, 0u, 1u) && ok;
        ok = AppendMainDirectoryFixed80021E60(out, 216, 47, 0x80051170u, 0u, 1u) && ok;
        ok = AppendMainDirectoryFixed80021E60(out, 216, 71, 0x800512B0u, 0u, 1u) && ok;
        ok = AppendMainDirectoryFixed80021E60(out, 32, 115, 0x800513D0u, 0u, 1u) && ok;
        ok = AppendMainDirectoryFixed80021E60(out, 95, 136, 0x80051510u, 0u, 1u) && ok;
        ok = AppendMainDirectoryFixed80021E60(out, 160, 136, 0x80051650u, 0u, 1u) && ok;
        ok = AppendMainDirectoryFixed80021E60(out, 216, 115, 0x80051790u, 0u, 1u) && ok;
        ok = AppendMainDirectoryFixed80021E60(out, 238, 188, 0x800509A0u, 0u, 1u) && ok;
    } else {
        const bool selected0 = state.cursor == 0;
        const int16_t item0 = static_cast<int16_t>(state.itemValue[0]);
        const uint32_t state0 = selected0
            ? (item0 != 0 ? 2u : 1u)
            : 0u;
        ok = AppendMainDirectoryLanguage80021E60(out, kMainLanguageText, 0, state0, 0u) && ok;
        ok = AppendMainDirectoryFixed80021E60(out, 37, 47, 0x80050EF0u, state0, 0u) && ok;
        ok = AppendMainDirectoryFixed80021E60(out, 28, 36, 0x80051010u, selected0 ? blinkState : 0u, 1u) && ok;

        const bool selected1 = state.cursor == 1;
        const int16_t item1 = static_cast<int16_t>(state.itemValue[1]);
        if (selected1) {
            const uint32_t textState = item1 == -1 ? 1u : 2u;
            ok = AppendMainDirectoryLanguage80021E60(out, kMainHiScoreText, state.language, textState, 0u) && ok;
            ok = AppendMainDirectoryFixed80021E60(out, 118, 65, 0x80051030u, textState, 0u) && ok;
            if (item1 != -1) {
                out.cardIoOverlay80020A3CRequired = true;
                out.blockedByCardIoOverlay80020A3C = true;
                out.cardIoOverlayInsertIndexKnown = true;
                out.cardIoOverlayInsertIndex = out.count;
            }
            ok = AppendMainDirectoryFixed80021E60(out, 110, 56, 0x80051150u, blinkState, 0u) && ok;
        } else {
            ok = AppendMainDirectoryFixed80021E60(out, 110, 56, 0x80051150u, 0u, 0u) && ok;
            ok = AppendMainDirectoryLanguage80021E60(out, kMainHiScoreText, state.language, 0u, 0u) && ok;
            ok = AppendMainDirectoryFixed80021E60(out, 118, 65, 0x80051030u, 0u, 0u) && ok;
        }

        const bool selected2 = state.cursor == 2;
        if (selected2) {
            const bool easy =
                static_cast<int16_t>(state.itemValue[2]) != 0;
            ok = AppendMainDirectoryLanguage80021E60(out, kMainNormalText, state.language, easy ? 1u : 2u, 0u) && ok;
            ok = AppendMainDirectoryLanguage80021E60(out, kMainEasyText, state.language, easy ? 2u : 1u, 0u) && ok;
            ok = AppendMainDirectoryFixed80021E60(out, 216, 47, 0x80051170u, easy ? 1u : 2u, 0u) && ok;
            ok = AppendMainDirectoryFixed80021E60(out, 216, 71, 0x800512B0u, easy ? 2u : 1u, 0u) && ok;
            ok = AppendMainDirectoryFixed80021E60(out, 207, 34, 0x80051290u, blinkState, 0u) && ok;
        } else {
            ok = AppendMainDirectoryFixed80021E60(out, 207, 34, 0x80051290u, 0u, 0u) && ok;
            ok = AppendMainDirectoryLanguage80021E60(out, kMainNormalText, state.language, 0u, 0u) && ok;
            ok = AppendMainDirectoryLanguage80021E60(out, kMainEasyText, state.language, 0u, 0u) && ok;
            ok = AppendMainDirectoryFixed80021E60(out, 216, 47, 0x80051170u, 0u, 0u) && ok;
            ok = AppendMainDirectoryFixed80021E60(out, 216, 71, 0x800512B0u, 0u, 0u) && ok;
        }

        const bool selected3 = state.cursor == 3;
        if (selected3) {
            const int16_t choice =
                static_cast<int16_t>(state.itemValue[3]);
            const auto appendSelectedChoice = [&](uint32_t selectedChoice) {
                const uint32_t practiceState = selectedChoice == 0u ? 2u : 1u;
                const uint32_t stageState = selectedChoice == 1u ? 2u : 1u;
                const uint32_t replayState = selectedChoice == 2u ? 2u : 1u;
                const uint32_t loadState = selectedChoice == 3u ? 2u : 1u;
                ok = AppendMainDirectoryLanguage80021E60(out, kMainPracticeText, state.language, practiceState, 0u) && ok;
                ok = AppendMainDirectoryLanguage80021E60(out, kMainStageSelectText, state.language, stageState, 0u) && ok;
                ok = AppendMainDirectoryLanguage80021E60(out, kMainReplayText, state.language, replayState, 0u) && ok;
                ok = AppendMainDirectoryLanguage80021E60(out, kMainLoadText, state.language, loadState, 0u) && ok;
                ok = AppendMainDirectoryFixed80021E60(out, 32, 115, 0x800513D0u, practiceState, 0u) && ok;
                ok = AppendMainDirectoryFixed80021E60(out, 95, 136, 0x80051510u, stageState, 0u) && ok;
                ok = AppendMainDirectoryFixed80021E60(out, 160, 136, 0x80051650u, replayState, 0u) && ok;
                ok = AppendMainDirectoryFixed80021E60(out, 216, 115, 0x80051790u, loadState, 0u) && ok;
            };
            if (choice == 0 || choice == 1) {
                appendSelectedChoice(static_cast<uint32_t>(choice));
            } else if (choice == 2) {
                appendSelectedChoice(2u);
                out.choice2ReplayThenLoadFallthrough = true;
                appendSelectedChoice(3u);
            } else if (choice == 3) {
                appendSelectedChoice(3u);
            } else {
                appendSelectedChoice(4u);
            }
            ok = AppendMainDirectoryFixed80021E60(out, 26, 106, 0x800514F0u, blinkState, 0u) && ok;
            ok = AppendMainDirectoryFixed80021E60(out, 94, 122, 0x80051630u, blinkState, 0u) && ok;
            ok = AppendMainDirectoryFixed80021E60(out, 162, 122, 0x80051770u, blinkState, 0u) && ok;
            ok = AppendMainDirectoryFixed80021E60(out, 226, 106, 0x800518B0u, blinkState, 0u) && ok;
        } else {
            ok = AppendMainDirectoryFixed80021E60(out, 26, 106, 0x800514F0u, 0u, 0u) && ok;
            ok = AppendMainDirectoryFixed80021E60(out, 94, 122, 0x80051630u, 0u, 0u) && ok;
            ok = AppendMainDirectoryFixed80021E60(out, 162, 122, 0x80051770u, 0u, 0u) && ok;
            ok = AppendMainDirectoryFixed80021E60(out, 226, 106, 0x800518B0u, 0u, 0u) && ok;
            ok = AppendMainDirectoryLanguage80021E60(out, kMainPracticeText, state.language, 0u, 0u) && ok;
            ok = AppendMainDirectoryLanguage80021E60(out, kMainStageSelectText, state.language, 0u, 0u) && ok;
            ok = AppendMainDirectoryLanguage80021E60(out, kMainReplayText, state.language, 0u, 0u) && ok;
            ok = AppendMainDirectoryLanguage80021E60(out, kMainLoadText, state.language, 0u, 0u) && ok;
            ok = AppendMainDirectoryFixed80021E60(out, 32, 115, 0x800513D0u, 0u, 0u) && ok;
            ok = AppendMainDirectoryFixed80021E60(out, 95, 136, 0x80051510u, 0u, 0u) && ok;
            ok = AppendMainDirectoryFixed80021E60(out, 160, 136, 0x80051650u, 0u, 0u) && ok;
            ok = AppendMainDirectoryFixed80021E60(out, 216, 115, 0x80051790u, 0u, 0u) && ok;
        }

        const bool selected4 = state.cursor == 4;
        const uint32_t exitState = selected4
            ? (state.exitConfirmed ? 2u : 1u)
            : 0u;
        ok = AppendMainDirectoryFixed80021E60(out, 231, 179, 0x80050AC0u, selected4 ? blinkState : 0u, 0u) && ok;
        ok = AppendMainDirectoryLanguage80021E60(out, kMainExitText, state.language, exitState, 0u) && ok;
        ok = AppendMainDirectoryFixed80021E60(out, 238, 188, 0x800509A0u, exitState, 0u) && ok;
    }

    const uint32_t expectedCount =
        kMainDirectoryBaseSpriteCount80021E60 +
        (out.choice2ReplayThenLoadFallthrough
             ? kMainDirectoryChoice2FallthroughSpriteCount80021E60
             : 0u);
    out.accepted = ok && !out.truncated && out.rawTextureOnly &&
                   out.count == expectedCount;
    out.complete = out.accepted &&
                   !out.blockedByCardIoOverlay80020A3C;
    return out;
}

OptionsDrawList80021910 BuildOptionsDrawList80021910(
    const OptionsState80021910& state) {
    OptionsDrawList80021910 out{};
    out.rawTextureOnly = true;
    out.sourceKnown = IsKnownOptionsState80021910(state);
    if (!out.sourceKnown) {
        return out;
    }

    const uint32_t blinkState = state.blinkOnOff == 1 ? 1u : 0u;
    bool ok = true;

    ok = AppendOptionsSprite80021910(
             out,
             kOptionsTitleLanguageLabel80021910.x,
             kOptionsTitleLanguageLabel80021910.y,
             kOptionsTitleLanguageLabel80021910.templates,
             2u,
             1u) && ok;
    ok = AppendOptionsFixed80021910(
             out, 36, 45, 0x80050AE0u, 0u, 1u) && ok;

    // 80021910 does not render ctx+0x0C as a conventional ON/OFF value.
    // Only cursor 0 with ctx+0x0C == 0 enters the alternating pair: ctx+0
    // chooses {ON=1,OFF=2} or {ON=2,OFF=1}.  A non-zero ctx+0x0C uses the
    // base state for both choices, even while cursor 0 is selected.
    const bool subtitleBlinkActive =
        state.cursor == 0 && state.opt0Value == 0;
    const uint32_t subtitleOnState = subtitleBlinkActive
        ? (blinkState != 0u ? 1u : 2u)
        : 0u;
    const uint32_t subtitleOffState = subtitleBlinkActive
        ? (blinkState != 0u ? 2u : 1u)
        : 0u;
    ok = AppendOptionsTableEntry80021910(
             out, kOptionsSubtitleOn80021910, state.language,
             subtitleOnState, 0u) && ok;
    ok = AppendOptionsTableEntry80021910(
             out, kOptionsSubtitleOff80021910, state.language,
             subtitleOffState, 0u) && ok;
    ok = AppendOptionsFixed80021910(
             out, 216, 47, 0x80051170u, subtitleOnState, 0u) && ok;
    ok = AppendOptionsFixed80021910(
             out, 216, 71, 0x800512B0u, subtitleOffState, 0u) && ok;
    ok = AppendOptionsFixed80021910(
             out, 207, 34, 0x80051290u,
             subtitleBlinkActive ? blinkState : 0u, 0u) && ok;

    const bool languageRowSelected = state.cursor == 1;
    for (int32_t index = 0; index < 5; ++index) {
        const uint32_t languageState = languageRowSelected
            ? (index == state.opt1Value ? 2u : 1u)
            : 0u;
        ok = AppendOptionsTableEntry80021910(
                 out, kOptionsLanguageA80021910, index,
                 languageState, 0u) && ok;
        ok = AppendOptionsTableEntry80021910(
                 out, kOptionsLanguageB80021910, index,
                 languageState, 0u) && ok;
    }
    ok = AppendOptionsFixed80021910(
             out, 26, 106, 0x80050D30u,
             languageRowSelected ? blinkState : 0u, 0u) && ok;
    ok = AppendOptionsFixed80021910(
             out, 162, 106, 0x80050E10u,
             languageRowSelected ? blinkState : 0u, 0u) && ok;

    const bool exitRowSelected = state.cursor == 2;
    const uint32_t exitState = exitRowSelected
        ? (state.doneFlag != 0 ? 2u : 1u)
        : 0u;
    ok = AppendOptionsFixed80021910(
             out, 231, 179, 0x80050AC0u,
             exitRowSelected ? blinkState : 0u, 0u) && ok;
    ok = AppendOptionsTableEntry80021910(
             out, kMainExitText, state.language, exitState, 0u) && ok;
    ok = AppendOptionsFixed80021910(
             out, 238, 188, 0x800509A0u, exitState, 0u) && ok;

    out.accepted = ok && !out.truncated && out.rawTextureOnly &&
                   out.count == kOptionsSpriteCapacity80021910;
    out.complete = out.accepted;
    return out;
}

StageSelectDrawList80020568 BuildStageSelectDrawList80020568(
    const StageSelectState80020568& state) {
    StageSelectDrawList80020568 out{};
    out.sourceKnown = IsKnownStageSelectState80020568(state);
    if (!out.sourceKnown) {
        return out;
    }
    out.rawTextureOnly = true;

    const uint32_t language = static_cast<uint32_t>(state.language);
    bool ok = AppendStageSelectSprite80020568(
        out,
        kStageSelectTitleTextPositions80053248[language].x,
        kStageSelectTitleTextPositions80053248[language].y,
        kStageSelectTitleTextTemplates80053244[language],
        0u);
    ok = AppendStageSelectSprite80020568(
             out, 32, 33, 0x80051AA0u, 0u) && ok;

    int32_t enabledState[6]{};
    int32_t rawStatus[6]{};
    int32_t localDotState[6]{};
    int32_t localSliceState[6]{};
    int32_t localNameState[6]{};
    for (uint32_t index = 0; index < 6u; ++index) {
        rawStatus[index] = state.rawStatus80092F1DTo23[index];
        if (rawStatus[index] != 0) {
            enabledState[index] = 1;
            localSliceState[index] = 0;
            localNameState[index] = 0;
        } else {
            enabledState[index] = 0;
            localSliceState[index] = 2;
            localNameState[index] = 1;
        }
        localDotState[index] = 0;
    }

    if (state.cursor == 7) {
        out.bonusOverlayPresent = true;
        ok = AppendStageSelectSprite80020568(
                 out,
                 state.doneFlag != 0 ? 64 : 63,
                 state.doneFlag != 0 ? 100 : 98,
                 state.doneFlag != 0 ? 0x800519F0u : 0x800519E0u,
                 0u) && ok;
        ok = AppendStageSelectSprite80020568(
                 out, 66, 103, 0x80051A00u, 0u) && ok;
    } else {
        if (state.rawStatus80092F1DTo23[6] != 0u) {
            out.bonusOverlayPresent = true;
            ok = AppendStageSelectSprite80020568(
                     out, 63, 98, 0x80051A10u, 0u) && ok;
            ok = AppendStageSelectSprite80020568(
                     out, 66, 103, 0x80051A00u, 0u) && ok;
        }
        if (state.cursor < 7) {
            const uint32_t selected =
                static_cast<uint32_t>(state.cursor - 1);
            localSliceState[selected] = 1;
            enabledState[selected] = 2;
            localDotState[selected] = 1;
        }
    }

    for (uint32_t index = 0; index < 6u; ++index) {
        const auto& topPosition =
            kStageSelectTopTextPositions8005308C[language][index];
        ok = AppendStageSelectSprite80020568(
                 out,
                 topPosition.x,
                 topPosition.y,
                 kStageSelectTopTextTemplates80053050
                     [language][enabledState[index]],
                 0u) && ok;

        const auto& dotPoint = kStagePoints[language * 6u + index];
        const auto& slice = kStageSlices[index];
        ok = AppendStageSelectSlice8001C604(
                 out,
                 dotPoint.x,
                 dotPoint.y,
                 static_cast<int16_t>(slice.uOffsetPx),
                 static_cast<int16_t>(slice.widthPx),
                 localSliceState[index],
                 0u) && ok;

        const auto& panelPosition =
            kStageSelectPanelPositions8005317C[index];
        ok = AppendStageSelectSprite80020568(
                 out,
                 panelPosition.x,
                 panelPosition.y,
                 kStageSelectPanelTemplates800531C4[enabledState[index]],
                 0u) && ok;

        const auto& namePosition =
            kStageSelectNamePositions80053194[index];
        ok = AppendStageSelectSprite80020568(
                 out,
                 namePosition.x,
                 namePosition.y,
                 kStageSelectNameTemplates80053214
                     [index][localNameState[index]],
                 0u) && ok;

        const auto& badgePosition =
            kStageSelectBadgePositions800531AC[index];
        const uint32_t badgeAState = static_cast<uint32_t>(
            2 * rawStatus[index] + localDotState[index]);
        ok = AppendStageSelectSprite80020568(
                 out,
                 badgePosition.x,
                 badgePosition.y,
                 kStageSelectBadgeATemplates800531F4[badgeAState],
                 0u) && ok;
        ok = AppendStageSelectSprite80020568(
                 out,
                 badgePosition.x,
                 badgePosition.y,
                 kStageSelectBadgeBTemplates800531E8[enabledState[index]],
                 0u) && ok;
    }

    uint32_t exitState = 0u;
    if (state.cursor == 8) {
        exitState = state.doneFlag != 0 ? 2u : 1u;
    }
    const auto& exitPosition =
        kStageSelectExitTextPositions8005300C[language];
    ok = AppendStageSelectSprite80020568(
             out,
             exitPosition.x,
             exitPosition.y,
             kStageSelectExitTextTemplates80053000[language][exitState],
             0u) && ok;
    ok = AppendStageSelectSprite80020568(
             out,
             238,
             188,
             0x800509A0u + exitState * 0x10u,
             0u) && ok;
    ok = AppendStageSelectSprite80020568(
             out,
             231,
             179,
             state.cursor == 8 && state.blinkOnOff != 0
                 ? 0x80050AD0u
                 : 0x80050AC0u,
             1u) && ok;

    const uint32_t expectedCount =
        kStageSelectBaseSpriteCount80020568 +
        (out.bonusOverlayPresent ? 2u : 0u);
    out.accepted = ok && !out.truncated && out.rawTextureOnly &&
                   out.count == expectedCount;
    out.complete = out.accepted;
    return out;
}

CardGridDrawList80020F94 BuildCardGridDrawList80020F94(
    const CardGridState80020F94& state) {
    CardGridDrawList80020F94 out{};
    out.rawTextureOnly = true;
    out.sourceKnown = IsKnownCardGridState80020F94(state);
    if (!out.sourceKnown) {
        return out;
    }

    const auto* language = FindCardGridLanguageRecord80020F94(
        state.eventId, state.language);
    if (language == nullptr) {
        return out;
    }

    bool ok = true;
    uint32_t sourceCallOrder = 0u;
    for (const auto& text : language->text) {
        ok = AppendCardGridTemplate80020F94(
                 out,
                 CardGridSpriteRole80020F94::LanguageText,
                 kFn8001C5A8,
                 sourceCallOrder++,
                 text.x,
                 text.y,
                 text.address) && ok;
    }

    if (state.cardIoFlag != 0) {
        out.cardIoOverlay80020A3CRequired = true;
        out.blockedByCardIoOverlay80020A3C = true;
        out.cardIoOverlayInsertIndexKnown = true;
        out.cardIoOverlayInsertIndex = out.count;
        out.cardIoMessageType = state.eventId == 7 ? 2 : 0;
        ++sourceCallOrder;
    }

    ok = AppendCardGridTemplate80020F94(
             out,
             CardGridSpriteRole80020F94::FixedPanel,
             kFn8001C550,
             sourceCallOrder++,
             37,
             36,
             0x80052870u) && ok;
    ok = AppendCardGridTemplate80020F94(
             out,
             CardGridSpriteRole80020F94::FixedPanel,
             kFn8001C550,
             sourceCallOrder++,
             123,
             30,
             0x800522C0u) && ok;

    for (int32_t index = 0; index < state.itemCount; ++index) {
        const int32_t row = index / state.columns;
        const int32_t column = index % state.columns;
        const int16_t markerX = static_cast<int16_t>(51 + 74 * column);
        const int16_t markerY = static_cast<int16_t>(73 + 21 * row);
        const int16_t textX = static_cast<int16_t>(52 + 74 * column);
        const int16_t textY = static_cast<int16_t>(78 + 21 * row);
        const uint32_t markerState = index == state.selected
            ? 1u
            : (state.enabled[static_cast<std::size_t>(index)] == 0
                   ? 2u
                   : 0u);
        const auto& text = state.slotText[static_cast<std::size_t>(index)];
        const uint32_t textCallOrder = sourceCallOrder++;
        int32_t penX = 0;
        for (std::size_t offset = 0u; offset < text.size(); ++offset) {
            const uint8_t glyphCode =
                static_cast<uint8_t>(text[offset]);
            if (glyphCode == 0u) {
                break;
            }
            ok = AppendCardGridGlyph8001B744(
                     out,
                     glyphCode,
                     textCallOrder,
                     textX,
                     textY,
                     markerState != 1u,
                     penX) && ok;
        }
        ok = AppendCardGridTemplate80020F94(
                 out,
                 CardGridSpriteRole80020F94::SlotMarker,
                 kFn8001C550,
                 sourceCallOrder++,
                 markerX,
                 markerY,
                 0x80052240u + markerState * 0x10u) && ok;
        ++out.markerCount;
    }

    const int32_t slotCount =
        static_cast<int32_t>(state.rows) * state.columns;
    const bool exitSelected = state.selected == slotCount;
    const uint32_t exitState = exitSelected
        ? (state.exitBlinkState != 0 ? 2u : 1u)
        : 0u;
    ok = AppendCardGridTemplate80020F94(
             out,
             CardGridSpriteRole80020F94::ExitFrame,
             kFn8001C550,
             sourceCallOrder++,
             231,
             179,
             exitSelected && state.exitFrameState != 0
                 ? 0x80050AD0u
                 : 0x80050AC0u) && ok;
    const auto& exitLabel =
        kCardGridExitLabels80020F94[state.language][exitState];
    ok = AppendCardGridTemplate80020F94(
             out,
             CardGridSpriteRole80020F94::ExitLabel,
             kFn8001C5A8,
             sourceCallOrder++,
             exitLabel.x,
             exitLabel.y,
             exitLabel.address) && ok;
    ok = AppendCardGridTemplate80020F94(
             out,
             CardGridSpriteRole80020F94::ExitBar,
             kFn8001C5A8,
             sourceCallOrder++,
             238,
             188,
             0x800509A0u + exitState * 0x10u) && ok;

    const uint32_t expectedCount =
        10u + static_cast<uint32_t>(state.itemCount) +
        out.glyphSpriteCount;
    out.accepted = ok && !out.truncated && out.rawTextureOnly &&
                   out.markerCount == static_cast<uint32_t>(state.itemCount) &&
                   out.count == expectedCount;
    out.complete = out.accepted && !out.cardIoOverlay80020A3CRequired;
    out.runtimeSubmitAllowed = out.complete;
    return out;
}

PracticeGuideDrawList80023518 BuildPracticeGuideDrawList80023518(
    const PracticeGuideState80023518& state) {
    PracticeGuideDrawList80023518 out{};
    out.sourceKnown = state.progressKnown;
    if (!out.sourceKnown) {
        return out;
    }

    const int64_t scaled = state.progress <= 0
        ? 0
        : static_cast<int64_t>(state.progress) * 15 + 31;
    if (scaled > 0x7FFFFFFFLL) {
        return out;
    }
    out.arithmeticSupported = true;
    out.scaledProgress = static_cast<int32_t>(scaled);

    const auto append = [&](int32_t threshold,
                            int16_t y,
                            uint32_t belowTemplate,
                            uint32_t reachedTemplate) {
        if (out.count >= kPracticeGuideSpriteCount80023518) {
            out.truncated = true;
            return false;
        }
        PracticeGuideSprite80023518 sprite{};
        sprite.psxFunction = kFn8001C550;
        sprite.threshold = threshold;
        sprite.x = static_cast<int16_t>(threshold);
        sprite.y = y;
        sprite.thresholdReached = out.scaledProgress >= threshold;
        sprite.templateAddress = sprite.thresholdReached
            ? reachedTemplate
            : belowTemplate;
        PracticeGuideDescriptor80023518 descriptor{};
        sprite.staticDescriptorKnown =
            ResolvePracticeGuideDescriptor80023518(
                sprite.templateAddress, descriptor);
        if (!sprite.staticDescriptorKnown) {
            return false;
        }
        sprite.attr = descriptor.attr;
        sprite.texX = descriptor.texX;
        sprite.texY = descriptor.texY;
        sprite.width = descriptor.width;
        sprite.height = descriptor.height;
        sprite.clutX = descriptor.clutX;
        sprite.clutY = descriptor.clutY;
        sprite.priority = 2u;
        sprite.callOrder = out.count;
        out.sprites[out.count++] = sprite;
        return true;
    };

    bool ok = true;
    for (int32_t threshold : kPracticeGuideLargeThresholds80053B48) {
        ok = append(threshold, 93, 0x80050930u, 0x80050940u) && ok;
    }
    for (int32_t threshold : kPracticeGuideSmallThresholds80053B58) {
        ok = append(threshold, 97, 0x80050910u, 0x80050920u) && ok;
    }
    out.staticTemplateSourcesResolved = ok;
    out.accepted = ok && !out.truncated &&
                   out.count == kPracticeGuideSpriteCount80023518;
    out.complete = out.accepted;
    out.runtimeSubmitAllowed = out.complete;
    return out;
}

PracticeIconSubmit80024418 BuildPracticeIconSubmit80024418(
    const PracticeIconState80024418& state) {
    PracticeIconSubmit80024418 out{};
    out.argumentsKnown = state.argumentsKnown;
    out.scaleWordsKnown = state.scaleWordsKnown;
    out.localRgbKnown = state.localRgbKnown;
    out.localR = state.localR;
    out.localG = state.localG;
    out.localB = state.localB;
    if (!out.argumentsKnown) {
        return out;
    }
    if (state.slotOrdinal < 0 || state.slotOrdinal >= 36 ||
        state.type < 1 || state.type > 8) {
        return out;
    }
    const int32_t localX = static_cast<int32_t>(state.centerX) - 160;
    const int32_t localY = static_cast<int32_t>(state.centerY) - 120;
    if (localX < -32768 || localX > 32767 ||
        localY < -32768 || localY > 32767) {
        return out;
    }
    out.argumentsAccepted = true;
    out.templateTableBase = kTablePracticeIconTemplate800540BC;
    out.templateSourceAddress = out.templateTableBase +
        static_cast<uint32_t>(state.type) * 4u;
    out.templateAddress = ResolvePracticeIconTemplate80024418(state.type);
    PracticeIconDescriptor80024418 descriptor{};
    out.templateResolved = out.templateAddress != 0u &&
        ResolvePracticeIconDescriptor80024418(state.type, descriptor);
    if (!out.templateResolved) {
        return out;
    }
    out.scaleXSourceAddress = 0x80087668u +
        static_cast<uint32_t>(state.slotOrdinal) * 2u;
    out.scaleYSourceAddress = 0x800876B0u +
        static_cast<uint32_t>(state.slotOrdinal) * 2u;
    out.localAttr = 0x50000040u;
    out.centerX = state.centerX;
    out.centerY = state.centerY;
    out.localX = static_cast<int16_t>(localX);
    out.localY = static_cast<int16_t>(localY);
    out.texX = descriptor.texX;
    out.texY = descriptor.texY;
    out.width = descriptor.width;
    out.height = descriptor.height;
    out.clutX = descriptor.clutX;
    out.clutY = descriptor.clutY;
    out.pivotX = static_cast<int16_t>(descriptor.width / 2u);
    out.pivotY = static_cast<int16_t>(descriptor.height / 2u);
    out.priority = 1u;
    out.staticPrefixResolved = true;
    out.sourceKnown = out.scaleWordsKnown;
    if (!out.sourceKnown) {
        return out;
    }
    out.scaleX = state.scaleX;
    out.scaleY = state.scaleY;
    out.accepted = true;
    out.complete = true;
    out.runtimeSubmitAllowed = out.localRgbKnown;
    return out;
}

void ResetPracticeWobbleBank80024308(
    PracticeWobbleBank80023F20& bank) {
    for (uint32_t index = 0u;
         index < kPracticeWobbleSlotCount80024308;
         ++index) {
        bank.scaleX[index] = 4096;
        bank.scaleY[index] = 4096;
        bank.slots[index].counter = 0;
        bank.slots[index].phase = 0;
        bank.slots[index].linearAcc = 2048;
        bank.slots[index].linearVel = 2048;
    }
    bank.initialized = true;
}

bool UpdatePracticeWobbleBank80023F20(
    PracticeWobbleBank80023F20& bank,
    int32_t count) {
    if (!bank.initialized) {
        return false;
    }
    if (count <= 0) {
        return true;
    }
    if (count > static_cast<int32_t>(kPracticeWobbleSlotCount80024308)) {
        return false;
    }
    for (int32_t index = 0; index < count; ++index) {
        auto& slot = bank.slots[index];
        if (slot.counter >= 24) {
            continue;
        }
        if (slot.counter >= 6) {
            if (slot.counter >= 22) {
                bank.scaleX[index] = 4096;
                bank.scaleY[index] = 4096;
            } else {
                if (slot.phase >= 8193) {
                    slot.phase = 0;
                }
                bank.scaleX[index] =
                    ResolvePracticeRsinStep25680023F20(slot.phase);
                bank.scaleY[index] = 4096;
                slot.phase += 256;
            }
        } else {
            bank.scaleX[index] =
                static_cast<int16_t>(slot.linearAcc + 4096);
            bank.scaleY[index] =
                static_cast<int16_t>(slot.linearAcc + 4096);
            slot.linearAcc += slot.linearVel;
            if (slot.linearAcc >= 4096) {
                slot.linearVel = -1024;
            }
        }
        ++slot.counter;
    }
    return true;
}

bool ReadPracticeWobbleScale80024418(
    const PracticeWobbleBank80023F20& bank,
    int32_t slotOrdinal,
    int16_t& scaleX,
    int16_t& scaleY) {
    if (!bank.initialized || slotOrdinal < 0 ||
        slotOrdinal >= static_cast<int32_t>(
                           kPracticeWobbleSlotCount80024308)) {
        return false;
    }
    scaleX = bank.scaleX[slotOrdinal];
    scaleY = bank.scaleY[slotOrdinal];
    return true;
}

PracticeIconSubmit80024418 BuildPracticeIconSubmitFromWobbleBank80024418(
    const PracticeWobbleBank80023F20& bank,
    int16_t centerX,
    int16_t centerY,
    int16_t slotOrdinal,
    int16_t type) {
    PracticeIconState80024418 state{};
    state.argumentsKnown = true;
    state.centerX = centerX;
    state.centerY = centerY;
    state.slotOrdinal = slotOrdinal;
    state.type = type;
    state.scaleWordsKnown = ReadPracticeWobbleScale80024418(
        bank, slotOrdinal, state.scaleX, state.scaleY);
    return BuildPracticeIconSubmit80024418(state);
}

PracticeIconSubmit80024418
BuildPracticeIconSubmitFromWobbleBankAndRgb80024418(
    const PracticeWobbleBank80023F20& bank,
    int16_t centerX,
    int16_t centerY,
    int16_t slotOrdinal,
    int16_t type,
    uint8_t localR,
    uint8_t localG,
    uint8_t localB) {
    PracticeIconState80024418 state{};
    state.argumentsKnown = true;
    state.centerX = centerX;
    state.centerY = centerY;
    state.slotOrdinal = slotOrdinal;
    state.type = type;
    state.scaleWordsKnown = ReadPracticeWobbleScale80024418(
        bank, slotOrdinal, state.scaleX, state.scaleY);
    state.localRgbKnown = true;
    state.localR = localR;
    state.localG = localG;
    state.localB = localB;
    return BuildPracticeIconSubmit80024418(state);
}

PracticeIconDirectRenderPayload80024418
BuildPracticeIconDirectRenderPayload80024418(
    const PracticeIconSubmit80024418& submit,
    const PrPsxGraphOwnerDirect::PsxGraphState* graph) {
    PracticeIconDirectRenderPayload80024418 out{};
    out.sourceKnown = submit.argumentsKnown && submit.argumentsAccepted &&
        submit.scaleWordsKnown && submit.sourceKnown &&
        submit.staticPrefixResolved && submit.templateResolved &&
        submit.localRgbKnown && submit.accepted && submit.complete &&
        submit.runtimeSubmitAllowed && !submit.hostRendererUsed &&
        !submit.replaySourceUsed && submit.localAttr == 0x50000040u &&
        submit.width == 16u && submit.height == 16u &&
        submit.pivotX == 8 && submit.pivotY == 8 &&
        submit.priority == 1u;
    if (!out.sourceKnown || graph == nullptr) {
        return out;
    }

    const auto& drawOffset = graph->drawOffset;
    const auto& gte = graph->gte;
    const int expectedDrawOffsetY =
        120 + ((graph->word_80096590 & 1u) != 0u ? 240 : 0);
    out.graphControlKnown =
        graph->word_800965A0 == 4u &&
        graph->word_800928D4 == 320 && graph->word_800928D6 == 240 &&
        drawOffset.setDrawEnvCalled &&
        drawOffset.word_800917AA == 0 &&
        drawOffset.word_800917AC == 0 &&
        drawOffset.word_80091738 == 160 &&
        drawOffset.word_8009173A == expectedDrawOffsetY &&
        gte.geomScreenKnown && gte.geomScreen == 440u &&
        gte.geomOffsetKnown &&
        gte.geomOffsetX == 0 && gte.geomOffsetY == 0 &&
        gte.depthCueKnown && gte.depthCueA == -4194 &&
        gte.depthCueB == 0x01400000 &&
        gte.zScaleFactorKnown && gte.zScaleFactor3 == 341 &&
        gte.zScaleFactor4 == 256;
    if (!out.graphControlKnown) {
        return out;
    }

    // 80040AE4/800401AC keep the two PSX draw pages at Y=0 and Y=240.
    // 80024418 consumes the currently selected page, so the packet's XY
    // words carry 120 or 360 as the draw-environment centre.  Both are exact
    // source states and are retained for host-page normalization below.
    out.drawOffsetX = graph->drawOffset.word_80091738;
    out.drawOffsetY = graph->drawOffset.word_8009173A;

    const uint16_t uvWord = static_cast<uint16_t>(4u * submit.texX);
    const uint16_t helperA3 = static_cast<uint16_t>(
        (uvWord & 0xFF00u) >> 2u);
    const uint16_t helperA4 = static_cast<uint16_t>(submit.texY & 0xFF00u);
    const uint16_t tpage = static_cast<uint16_t>(
        0x20u |
        ((helperA4 & 0x0100u) >> 4u) |
        ((helperA3 & 0x03FFu) >> 6u) |
        (4u * (helperA4 & 0x0200u)));

    PrPsxGsSpriteSubmitDirect::GsSortSpriteInput8003F1B4 input{};
    input.sprite.attr_00 = submit.localAttr;
    input.sprite.x_04 = submit.localX;
    input.sprite.y_06 = submit.localY;
    input.sprite.width_08 = submit.width;
    input.sprite.height_0A = submit.height;
    input.sprite.tpage_0C = tpage;
    input.sprite.u_0E = static_cast<uint8_t>(uvWord);
    input.sprite.v_0F = static_cast<uint8_t>(submit.texY);
    input.sprite.clutX_10 = static_cast<int16_t>(submit.clutX);
    input.sprite.clutY_12 = static_cast<int16_t>(submit.clutY);
    input.sprite.r_14 = submit.localR;
    input.sprite.g_15 = submit.localG;
    input.sprite.b_16 = submit.localB;
    input.sprite.mx_18 = submit.pivotX;
    input.sprite.my_1A = submit.pivotY;
    input.sprite.scaleX_1C = submit.scaleX;
    input.sprite.scaleY_1E = submit.scaleY;
    input.sprite.rot_20 = 0;
    input.drawOffsets.word_800917AA = drawOffset.word_800917AA;
    input.drawOffsets.word_800917AC = drawOffset.word_800917AC;
    input.gte = gte;
    input.priority = submit.priority;

    const auto result =
        PrPsxGsSpriteSubmitDirect::PsxCall8003F1B4_GsSortSprite(input);
    out.gsSortSpriteEvaluated = true;
    if (result.skipped || !result.priorityKnown ||
        result.priority != submit.priority || !result.packet.written ||
        result.packetPath ==
            PrPsxGsSpriteSubmitDirect::
                GsSortSpritePacketPath8003F1B4::None) {
        return out;
    }

    out.attr = submit.localAttr;
    out.packetWordCount = result.packet.totalWordCount;
    out.priority = result.priority;
    out.width = submit.width;
    out.height = submit.height;
    const auto applyDrawOffset = [](uint32_t word,
                                    int16_t xOffset,
                                    int16_t yOffset,
                                    int16_t& x,
                                    int16_t& y) {
        const uint16_t rawX = static_cast<uint16_t>(word & 0xFFFFu);
        const uint16_t rawY = static_cast<uint16_t>(word >> 16);
        x = static_cast<int16_t>(
            rawX + static_cast<uint16_t>(xOffset));
        y = static_cast<int16_t>(
            rawY + static_cast<uint16_t>(yOffset));
    };

    if (result.packetPath ==
        PrPsxGsSpriteSubmitDirect::GsSortSpritePacketPath8003F1B4::Fast) {
        out.fastPacketPath = true;
        if (!result.packet.fieldsKnown ||
            result.packet.totalWordCount !=
                PrPsxGsSpriteSubmitDirect::
                    kGsSortSpritePacketFastTotalWords8003F1B4) {
            return out;
        }
        const uint32_t color = result.packet.word2_colorOrXy0;
        out.r = static_cast<uint8_t>(color);
        out.g = static_cast<uint8_t>(color >> 8);
        out.b = static_cast<uint8_t>(color >> 16);
        applyDrawOffset(result.packet.word3_xyOrUv0,
                        drawOffset.word_80091738,
                        drawOffset.word_8009173A,
                        out.x[0], out.y[0]);
        const uint32_t uvClut = result.packet.word4_uvOrXy1;
        out.u[0] = static_cast<uint8_t>(uvClut);
        out.v[0] = static_cast<uint8_t>(uvClut >> 8);
        out.clut = static_cast<uint16_t>(uvClut >> 16);
        out.tpage = static_cast<uint16_t>(
            result.packet.word1_drawModeOrColor & 0x01FFu);
    } else {
        out.transformPacketPath = true;
        if (!result.packet.transformNonGeometryFieldsKnown ||
            result.packet.totalWordCount !=
                PrPsxGsSpriteSubmitDirect::
                    kGsSortSpritePacketTransformTotalWords8003F1B4) {
            return out;
        }

        PrPsxGteDirect::Rtpt3ExactInput280030 firstThree{};
        firstThree.matrixKnown =
            result.packet.transformPrelude.matrixValuesKnown;
        firstThree.matrix = result.packet.transformPrelude.matrix;
        firstThree.controlKnown = true;
        firstThree.control = gte;
        firstThree.verticesKnown = true;
        for (std::size_t index = 0; index < firstThree.vertices.size();
             ++index) {
            firstThree.vertices[index] =
                result.packet.transformPrelude.rotTransPers4.vertices[index];
        }
        PrPsxGteDirect::Rtpt3ExactInput280030 fourth{};
        fourth.matrixKnown = firstThree.matrixKnown;
        fourth.matrix = firstThree.matrix;
        fourth.controlKnown = true;
        fourth.control = gte;
        fourth.verticesKnown = true;
        fourth.vertices.fill(
            result.packet.transformPrelude.rotTransPers4.vertices[3]);
        const auto firstThreeProjected =
            PrPsxGteDirect::ExecuteRtpt3Exact280030(firstThree);
        const auto fourthProjected =
            PrPsxGteDirect::ExecuteRtpt3Exact280030(fourth);
        if (!firstThreeProjected.known || !fourthProjected.known) {
            return out;
        }

        const uint32_t color = result.packet.word1_drawModeOrColor;
        out.r = static_cast<uint8_t>(color);
        out.g = static_cast<uint8_t>(color >> 8);
        out.b = static_cast<uint8_t>(color >> 16);
        const std::array<uint32_t, 4> xyWords{{
            firstThreeProjected.sxy[0].word,
            firstThreeProjected.sxy[1].word,
            firstThreeProjected.sxy[2].word,
            fourthProjected.sxy[0].word,
        }};
        const std::array<uint32_t, 4> uvWords{{
            result.packet.word3_xyOrUv0,
            result.packet.word5_whOrUv1,
            result.packet.word7_uv2,
            result.packet.word9_uv3,
        }};
        for (std::size_t index = 0; index < xyWords.size(); ++index) {
            applyDrawOffset(xyWords[index],
                            drawOffset.word_80091738,
                            drawOffset.word_8009173A,
                            out.x[index], out.y[index]);
            out.u[index] = static_cast<uint8_t>(uvWords[index]);
            out.v[index] = static_cast<uint8_t>(uvWords[index] >> 8);
        }
        out.clut = static_cast<uint16_t>(uvWords[0] >> 16);
        out.tpage = static_cast<uint16_t>((uvWords[1] >> 16) & 0x01FFu);
        out.transformGeometryExactRtpt = true;
    }

    out.drawEnvOffsetApplied = true;
    out.renderPayloadKnown = true;
    return out;
}

void ResetPracticePortraitBank80024600(
    PracticePortraitBank80024600& bank) {
    bank.repeatFrame = 0;
    bank.lastValue = 0;
    bank.templateKnown = false;
    bank.templateAddress = 0u;
    bank.initialized = true;
}

bool SelectPracticePortraitTemplate800246A8(
    PracticePortraitBank80024600& bank,
    int32_t mode) {
    if (!bank.initialized) {
        return false;
    }
    bank.templateAddress = ResolvePracticePortraitTemplate800246A8(mode);
    bank.templateKnown = true;
    return true;
}

PracticePortraitSubmit80024600 UpdatePracticePortraitBank80024600(
    PracticePortraitBank80024600& bank,
    const PracticePortraitState80024600& state) {
    PracticePortraitSubmit80024600 out{};
    out.argumentsKnown = state.argumentsKnown;
    out.value = state.value;
    out.templateKnown = bank.templateKnown;
    out.templateAddress = bank.templateAddress;
    out.argumentsAccepted = state.argumentsKnown && state.row == 0 &&
        state.baseX == 16 && state.baseY == 93 &&
        state.maxRepeatFrame == 3 && state.value >= -1 &&
        state.value <= 18;
    PracticePortraitDescriptor80024600 descriptor{};
    out.staticDescriptorKnown = bank.templateKnown &&
        ResolvePracticePortraitDescriptor80024600(
            bank.templateAddress, descriptor);
    out.sourceKnown = bank.initialized && out.argumentsAccepted &&
        bank.templateKnown && out.staticDescriptorKnown;
    if (!out.sourceKnown) {
        return out;
    }

    out.attr = descriptor.attr;
    out.texX = descriptor.texX;
    out.texY = descriptor.texY;
    out.width = descriptor.width;
    out.height = descriptor.height;
    out.clutX = descriptor.clutX;
    out.clutY = descriptor.clutY;

    if (bank.lastValue != state.value) {
        bank.lastValue = state.value;
        bank.repeatFrame = 0;
    } else {
        ++bank.repeatFrame;
    }
    if (bank.repeatFrame > state.maxRepeatFrame) {
        bank.repeatFrame = state.maxRepeatFrame;
    }

    out.repeatFrame = bank.repeatFrame;
    out.accepted = true;
    out.complete = true;
    out.runtimeSubmitAllowed = true;
    if (state.value <= 0) {
        return out;
    }
    out.x = static_cast<int16_t>(state.baseX + 15 * state.value +
                                 4 * bank.repeatFrame);
    out.y = static_cast<int16_t>(state.baseY + 20 * state.row);
    out.drawRequested = true;
    return out;
}

PracticeSliceDrawList8001C604 BuildPracticeSliceDrawList8001C604(
    const PracticeSliceProducerState8001C604& state) {
    PracticeSliceDrawList8001C604 out{};
    out.eventDispatchKnown = state.eventDispatchKnown;
    out.eventIdAccepted = state.eventDispatchKnown && state.eventId == 16u;
    out.precedingPortraitDrawProducerKnown =
        state.precedingPortraitDrawProducerKnown;
    out.staticWriterSemanticsKnown = out.eventIdAccepted &&
        out.precedingPortraitDrawProducerKnown;
    out.localRgbKnown = out.staticWriterSemanticsKnown;
    out.sourceKnown = out.localRgbKnown;
    if (!out.sourceKnown) {
        return out;
    }

    constexpr int16_t kX[kPracticeSliceSpriteCount8001C604] = {
        65, 122, 178, 234,
    };
    constexpr uint32_t kTemplateAddress = 0x80052DA0u;
    constexpr uint32_t kAttr = 0x50000040u;
    constexpr uint16_t kTexX = 0x01E1u;
    constexpr uint16_t kTexY = 0x0020u;
    constexpr uint16_t kWidth = 7u;
    constexpr uint16_t kHeight = 8u;
    constexpr uint16_t kClutX = 0x01C0u;
    constexpr uint16_t kClutY = 0x0101u;
    for (uint32_t index = 0u;
         index < kPracticeSliceSpriteCount8001C604;
         ++index) {
        auto& sprite = out.sprites[out.count++];
        sprite.known = true;
        sprite.staticDescriptorKnown = true;
        sprite.textureCoordinatesResolved = true;
        sprite.localRgbKnown = true;
        sprite.templateAddress = kTemplateAddress;
        sprite.attr = kAttr;
        sprite.texX = kTexX;
        sprite.texY = kTexY;
        sprite.width = kWidth;
        sprite.height = kHeight;
        sprite.clutX = kClutX;
        sprite.clutY = kClutY;
        sprite.x = kX[index];
        sprite.y = 81;
        sprite.uOffset = static_cast<int16_t>(index * kWidth);
        sprite.tpage = StageSelectResolvedTpage8001B654(
            sprite.texX, sprite.texY, sprite.uOffset);
        sprite.u = static_cast<uint8_t>(
            4u * sprite.texX + static_cast<uint16_t>(sprite.uOffset));
        sprite.v = static_cast<uint8_t>(sprite.texY);
        sprite.localR = state.eventId;
        sprite.localG = 0u;
        sprite.localB = 0u;
        sprite.priority = 0u;
        sprite.callOrder = index;
        out.rawTextureOnly = out.rawTextureOnly &&
            (sprite.attr & 0x40u) != 0u;
    }

    out.accepted = out.count == kPracticeSliceSpriteCount8001C604 &&
        !out.truncated && out.rawTextureOnly;
    out.complete = out.accepted;
    out.runtimeSubmitAllowed = out.complete;
    return out;
}

PracticePostSliceFixedDrawList80023618
BuildPracticePostSliceFixedDrawList80023618(
    const PracticePostSliceFixedProducerState80023618& state) {
    PracticePostSliceFixedDrawList80023618 out{};
    out.eventDispatchKnown = state.eventDispatchKnown;
    out.eventIdAccepted = state.eventDispatchKnown && state.eventId == 16u;
    out.precedingSliceDrawPublished = state.precedingSliceDrawPublished;
    out.staticDescriptorSourcesKnown = true;
    out.sourceKnown = out.eventIdAccepted &&
        out.precedingSliceDrawPublished &&
        out.staticDescriptorSourcesKnown;
    if (!out.sourceKnown) {
        return out;
    }

    struct StaticSpriteSpec {
        int16_t x;
        int16_t y;
        uint32_t templateAddress;
        uint32_t attr;
        uint16_t texX;
        uint16_t texY;
        uint16_t width;
        uint16_t height;
        uint16_t clutX;
        uint16_t clutY;
    };
    static constexpr StaticSpriteSpec kSpecs[
        kPracticePostSliceFixedSpriteCount80023618] = {
        {32, 34, 0x800529E0u, 0x50000040u,
         0x0200u, 0x0034u, 80u, 44u, 0x01C0u, 0x010Du},
        {112, 31, 0x800529F0u, 0x50000040u,
         0x0200u, 0x0000u, 180u, 52u, 0x01C0u, 0x010Eu},
        {211, 113, 0x80052E30u, 0x51000040u,
         0x01C0u, 0x003Cu, 84u, 64u, 0x01C0u, 0x01FEu},
        {26, 119, 0x80052E40u, 0x51000040u,
         0x01C0u, 0x007Cu, 84u, 86u, 0x01C0u, 0x01FFu},
    };

    for (uint32_t index = 0u;
         index < kPracticePostSliceFixedSpriteCount80023618;
         ++index) {
        const auto& spec = kSpecs[index];
        auto& command = out.sprites[out.count++];
        command.known = true;
        command.x = spec.x;
        command.y = spec.y;
        command.priority = 0u;
        command.callOrder = index;
        command.sprite.known = true;
        command.sprite.psxAddress = spec.templateAddress;
        command.sprite.attr = spec.attr;
        command.sprite.texX = spec.texX;
        command.sprite.texY = spec.texY;
        command.sprite.width = spec.width;
        command.sprite.height = spec.height;
        command.sprite.clutX = spec.clutX;
        command.sprite.clutY = spec.clutY;
        out.rawTextureOnly = out.rawTextureOnly &&
            (command.sprite.attr & 0x40u) != 0u;
    }

    out.accepted = out.count ==
        kPracticePostSliceFixedSpriteCount80023618 &&
        !out.truncated && out.rawTextureOnly;
    out.complete = out.accepted;
    out.runtimeSubmitAllowed = out.complete;
    return out;
}

MainDirectorySpriteTemplate8001B25C
ResolvePracticeCaptionTemplate80023618(uint32_t address) {
    struct StaticCaptionTemplate {
        uint32_t address;
        uint32_t attr;
        uint16_t texX;
        uint16_t texY;
        uint16_t width;
        uint16_t height;
        uint16_t clutX;
        uint16_t clutY;
    };
    static constexpr StaticCaptionTemplate kTemplates[5] = {
        {0x80052A20u, 0x50000040u, 0x022Du, 0x0000u,
         64u, 12u, 0x0200u, 0x0100u},
        {0x80052B80u, 0x50000040u, 0x022Du, 0x000Cu,
         60u, 15u, 0x0200u, 0x0100u},
        {0x80052AD0u, 0x50000040u, 0x022Du, 0x001Bu,
         76u, 11u, 0x0200u, 0x0100u},
        {0x80052C30u, 0x50000040u, 0x022Du, 0x0026u,
         68u, 12u, 0x0200u, 0x0100u},
        {0x80052CE0u, 0x50000040u, 0x022Du, 0x0032u,
         72u, 15u, 0x0200u, 0x0100u},
    };
    for (const auto& source : kTemplates) {
        if (source.address != address) {
            continue;
        }
        MainDirectorySpriteTemplate8001B25C out{};
        out.known = true;
        out.psxAddress = source.address;
        out.attr = source.attr;
        out.texX = source.texX;
        out.texY = source.texY;
        out.width = source.width;
        out.height = source.height;
        out.clutX = source.clutX;
        out.clutY = source.clutY;
        return out;
    }
    return {};
}

PracticePostSliceTailDrawList80023618
BuildPracticePostSliceTailDrawList80023618(
    const PracticePostSliceTailProducerState80023618& state) {
    PracticePostSliceTailDrawList80023618 out{};
    out.eventDispatchKnown = state.eventDispatchKnown;
    out.eventIdAccepted = state.eventDispatchKnown && state.eventId == 16u;
    out.precedingSliceDrawPublished = state.precedingSliceDrawPublished;
    out.languageAccepted = state.languageKnown &&
        state.language >= 0 && state.language < 5;
    // 80023D38/40 and 80023D64/6C consume the original halfword/dword only
    // as zero-versus-nonzero predicates.
    out.exitSelectionAccepted = state.exitSelectionKnown;
    out.exitBlinkAccepted = state.exitBlinkKnown;
    out.staticWriterSemanticsKnown = true;
    out.staticDescriptorSourcesKnown = true;
    out.sourceKnown = out.eventIdAccepted &&
        out.precedingSliceDrawPublished && out.languageAccepted &&
        out.exitSelectionAccepted && out.exitBlinkAccepted &&
        out.staticWriterSemanticsKnown && out.staticDescriptorSourcesKnown;
    if (!out.sourceKnown) {
        return out;
    }

    const uint32_t language = static_cast<uint32_t>(state.language);
    PracticePostSliceFixedProducerState80023618 fixedState{};
    fixedState.eventDispatchKnown = state.eventDispatchKnown;
    fixedState.eventId = state.eventId;
    fixedState.precedingSliceDrawPublished =
        state.precedingSliceDrawPublished;
    const auto fixed = BuildPracticePostSliceFixedDrawList80023618(fixedState);
    if (!fixed.sourceKnown || !fixed.accepted || !fixed.complete ||
        !fixed.runtimeSubmitAllowed || fixed.truncated ||
        fixed.count != kPracticePostSliceFixedSpriteCount80023618) {
        return out;
    }

    PracticePostSliceTailDrawList80023618 candidate = out;
    const auto append = [&](uint32_t psxFunction,
                            int16_t x,
                            int16_t y,
                            const MainDirectorySpriteTemplate8001B25C& sprite) {
        if (!sprite.known ||
            candidate.count >= kPracticePostSliceTailSpriteCount80023618) {
            candidate.truncated =
                candidate.count >= kPracticePostSliceTailSpriteCount80023618;
            return false;
        }
        auto& command = candidate.sprites[candidate.count];
        command.known = true;
        command.psxFunction = psxFunction;
        command.x = x;
        command.y = y;
        command.priority = 0u;
        command.callOrder = candidate.count;
        command.sprite = sprite;
        ++candidate.count;
        candidate.rawTextureOnly = candidate.rawTextureOnly &&
            (sprite.attr & 0x40u) != 0u;
        return true;
    };

    const auto& captionRecord = kPracticeCaption80053B90[language];
    if (!append(kFn8001C550,
                captionRecord.x,
                captionRecord.y,
                ResolvePracticeCaptionTemplate80023618(
                    captionRecord.templateAddress))) {
        return out;
    }
    for (uint32_t index = 0u; index < fixed.count; ++index) {
        const auto& command = fixed.sprites[index];
        if (!append(command.psxFunction,
                    command.x,
                    command.y,
                    command.sprite)) {
            return out;
        }
    }

    const uint32_t exitIconAddress = state.exitBlink4C != 0
        ? kSpriteSharedExitIconOn80050AD0
        : kSpriteSharedExitIconOff80050AC0;
    if (!append(kFn8001C550,
                231,
                179,
                ResolveStageSelectTemplate80020568(exitIconAddress))) {
        return out;
    }

    const auto& exitRecord = kPracticeExit80053000[language];
    const uint32_t exitTextAddress = state.exitSelection48 != 0
        ? exitRecord.templateOn
        : exitRecord.templateOff;
    if (!append(kFn8001C5A8,
                exitRecord.x,
                exitRecord.y,
                ResolveStageSelectTemplate80020568(exitTextAddress))) {
        return out;
    }

    const uint32_t exitBarAddress = state.exitSelection48 != 0
        ? 0x800509C0u
        : 0x800509B0u;
    if (!append(kFn8001C5A8,
                238,
                188,
                ResolveStageSelectTemplate80020568(exitBarAddress))) {
        return out;
    }

    candidate.accepted =
        candidate.count == kPracticePostSliceTailSpriteCount80023618 &&
        !candidate.truncated && candidate.rawTextureOnly;
    candidate.complete = candidate.accepted;
    candidate.runtimeSubmitAllowed = candidate.complete;
    return candidate;
}

PracticePostSliceTailDrawList80023618
BuildPracticeAlwaysVisibleTailDrawList80023618(
    const PracticePostSliceTailProducerState80023618& state) {
    // 80023618 emits this caption/fixed-character/EXIT tail after the optional
    // lane and icon loops on every frame.  Reuse the exact eight-record
    // descriptor builder while keeping the observed slice publication as
    // metadata; the always-visible authority is explicit and does not claim
    // that an absent Rec44 slice was submitted.
    PracticePostSliceTailProducerState80023618 descriptorState = state;
    descriptorState.precedingSliceDrawPublished = true;
    auto out = BuildPracticePostSliceTailDrawList80023618(descriptorState);
    out.precedingSliceDrawPublished = state.precedingSliceDrawPublished;
    out.alwaysVisibleFixedTail = out.sourceKnown && out.accepted && out.complete;
    return out;
}

PracticeDrawCallTrace80023618 BuildPracticeDrawCallTrace80023618(
    const PracticeState80023618& state) {
    PracticeDrawCallTrace80023618 out{};
    out.sourceKnown = state.contextPresent && state.iconBytesKnown &&
                      state.language >= 0 && state.language < 5 &&
                      state.laneA8C >= -1 && state.laneA8C <= 18 &&
                      state.laneB9E >= -1 && state.laneB9E <= 18;
    if (!out.sourceKnown) {
        return out;
    }

    const uint32_t language = static_cast<uint32_t>(state.language);
    bool ok = true;
    const auto append = [&](PracticeDrawCallKind80023618 kind,
                            uint32_t function,
                            uint32_t positionSource,
                            uint32_t templateSource,
                            bool templateIndirect,
                            int32_t arg0 = 0,
                            int32_t arg1 = 0,
                            int32_t arg2 = 0,
                            int32_t arg3 = 0,
                            int32_t arg4 = 0,
                            int32_t arg5 = 0,
                            int32_t arg6 = 0,
                            int32_t arg7 = 0) {
        if (out.count >= kPracticeDrawCallCapacity80023618) {
            out.truncated = true;
            return false;
        }
        PracticeDrawCall80023618 call{};
        call.kind = kind;
        call.psxFunction = function;
        call.positionSourceAddress = positionSource;
        call.templateSourceAddress = templateSource;
        call.templatePointerIndirect = templateIndirect;
        call.args[0] = arg0;
        call.args[1] = arg1;
        call.args[2] = arg2;
        call.args[3] = arg3;
        call.args[4] = arg4;
        call.args[5] = arg5;
        call.args[6] = arg6;
        call.args[7] = arg7;
        call.callOrder = out.count;
        out.calls[out.count++] = call;
        return true;
    };
    const auto appendFastTable = [&](uint32_t tableBase) {
        const uint32_t record = tableBase + language * 8u;
        return append(PracticeDrawCallKind80023618::FastSprite8001C5A8,
                      kFn8001C5A8,
                      record + 4u,
                      record,
                      true,
                      static_cast<int32_t>(record + 4u),
                      static_cast<int32_t>(record),
                      0);
    };
    const auto appendFixed = [&](int16_t x,
                                 int16_t y,
                                 uint32_t templateAddress,
                                 uint16_t priority = 0u) {
        return append(PracticeDrawCallKind80023618::Sprite8001C550,
                      kFn8001C550,
                      0u,
                      templateAddress,
                      false,
                      x,
                      y,
                      static_cast<int32_t>(templateAddress),
                      priority);
    };
    const auto appendCommonHeader = [&]() {
        bool headerOk = appendFastTable(0x80053D48u);
        headerOk = appendFastTable(0x80053BB8u) && headerOk;
        return headerOk;
    };

    if ((state.flags00 & 0x00400000u) != 0u) {
        switch (state.overlayState1C) {
        case 0:
            ok = appendFastTable(0x80053C08u) && ok;
            ok = appendFixed(54, 196, 0x80052E10u) && ok;
            ok = appendFixed(101, 114, 0x80052D90u) && ok;
            ok = appendCommonHeader() && ok;
            break;
        case 1:
        case 2:
            ok = appendFastTable(state.overlayState1C == 1
                                     ? 0x80053CA8u
                                     : 0x80053CD0u) && ok;
            ok = appendFastTable(0x80053D20u) && ok;
            ok = appendFixed(240, 161, 0x80052DB0u) && ok;
            ok = appendFixed(225, 134, 0x80052DD0u) && ok;
            ok = appendFixed(266, 134, 0x80052DE0u) && ok;
            ok = appendFixed(54, 196, 0x80052E00u) && ok;
            ok = appendFixed(42, 180, 0x80052E20u) && ok;
            ok = appendFixed(101, 114, 0x80052D90u) && ok;
            ok = appendCommonHeader() && ok;
            break;
        case 3:
            ok = appendCommonHeader() && ok;
            break;
        case 4:
        case 5:
        case 6: {
            const uint32_t table = state.overlayState1C == 4
                ? 0x80053C58u
                : (state.overlayState1C == 5
                       ? 0x80053C30u
                       : 0x80053CF8u);
            ok = appendFastTable(table) && ok;
            ok = appendFixed(240, 161, 0x80052DC0u) && ok;
            ok = appendFixed(54, 196, 0x80052DF0u) && ok;
            ok = appendFixed(101, 114, 0x80052D90u) && ok;
            ok = appendCommonHeader() && ok;
            break;
        }
        case 7:
            ok = appendFastTable(0x80053C80u) && ok;
            ok = appendFixed(101, 114, 0x80052D90u) && ok;
            ok = appendCommonHeader() && ok;
            break;
        case 8:
            ok = appendFastTable(0x80053D70u) && ok;
            ok = appendFastTable(0x80053BE0u) && ok;
            ok = appendFixed(240, 161, 0x80052DC0u) && ok;
            ok = appendFixed(54, 196, 0x80052DF0u) && ok;
            break;
        default:
            break;
        }
    } else {
        ok = appendCommonHeader() && ok;
    }

    const auto appendRec44Lane = [&](int16_t value, int32_t mode) {
        if (value < 0) {
            return true;
        }
        bool laneOk = append(
            PracticeDrawCallKind80023618::WobbleUpdate80023F20,
            kFn80023F20, 0u, 0u, false, value);
        laneOk = append(PracticeDrawCallKind80023618::Rec44Mode800246A8,
                        kFn800246A8, 0u, 0u, false, mode) && laneOk;
        laneOk = append(PracticeDrawCallKind80023618::Rec44Draw80024600,
                        kFn80024600, 0u, 0u, false,
                        0, 16, 93, 3, value) && laneOk;
        return laneOk;
    };
    ok = appendRec44Lane(state.laneA8C, 4) && ok;
    ok = appendRec44Lane(state.laneB9E, 0) && ok;
    ok = append(PracticeDrawCallKind80023618::Guide80023518,
                kFn80023518, 0u, 0u, false, -1) && ok;

    for (uint32_t index = 0; index < 18u; ++index) {
        const int32_t code = static_cast<int8_t>(state.iconCodes[index]);
        if (code < 1 || code > 8) {
            continue;
        }
        ok = append(PracticeDrawCallKind80023618::Icon80024418,
                    kFn80024418,
                    0u,
                    kTablePracticeIconTemplate800540BC,
                    true,
                    41 + static_cast<int32_t>(14u * index),
                    99,
                    static_cast<int32_t>(index),
                    code) && ok;
    }

    constexpr int16_t kSliceX[4] = {65, 122, 178, 234};
    for (uint32_t index = 0; index < 4u; ++index) {
        ok = append(PracticeDrawCallKind80023618::Slice8001C604,
                    kFn8001C604,
                    0u,
                    0x80052DA0u,
                    false,
                    kSliceX[index],
                    81,
                    static_cast<int32_t>(0x80052DA0u),
                    static_cast<int32_t>(index * 7u),
                    7,
                    0,
                    0) && ok;
    }

    const uint32_t captionRecord = 0x80053B90u + language * 8u;
    ok = append(PracticeDrawCallKind80023618::Sprite8001C550,
                kFn8001C550,
                captionRecord + 4u,
                captionRecord,
                true,
                static_cast<int32_t>(captionRecord + 4u),
                static_cast<int32_t>(captionRecord + 6u),
                static_cast<int32_t>(captionRecord),
                0) && ok;
    ok = appendFixed(32, 34, 0x800529E0u) && ok;
    ok = appendFixed(112, 31, 0x800529F0u) && ok;
    ok = appendFixed(211, 113, 0x80052E30u) && ok;
    ok = appendFixed(26, 119, 0x80052E40u) && ok;

    const uint32_t exitFrameSource = state.exitBlink4C != 0
        ? kGpSharedExitIconSlotOn8006EB10
        : kGpSharedExitIconSlotOff8006EB14;
    ok = append(PracticeDrawCallKind80023618::Sprite8001C550,
                kFn8001C550,
                0u,
                exitFrameSource,
                true,
                231,
                179,
                static_cast<int32_t>(exitFrameSource),
                0) && ok;

    const uint32_t exitRecord = 0x80053000u + language * 16u;
    const uint32_t exitTemplateSource = exitRecord +
        (state.exitSelection48 != 0 ? 8u : 4u);
    ok = append(PracticeDrawCallKind80023618::FastSprite8001C5A8,
                kFn8001C5A8,
                exitRecord + 12u,
                exitTemplateSource,
                true,
                static_cast<int32_t>(exitRecord + 12u),
                static_cast<int32_t>(exitTemplateSource),
                0) && ok;
    const uint32_t exitBarTemplate = state.exitSelection48 != 0
        ? 0x800509C0u
        : 0x800509B0u;
    ok = append(PracticeDrawCallKind80023618::FastSprite8001C5A8,
                kFn8001C5A8,
                0x80052FFCu,
                exitBarTemplate,
                false,
                static_cast<int32_t>(0x80052FFCu),
                static_cast<int32_t>(exitBarTemplate),
                0) && ok;

    out.staticOperandSourcesResolved =
        ResolvePracticeTraceStaticOperands80023618(out, language);
    out.accepted = ok && !out.truncated &&
                   out.staticOperandSourcesResolved;
    out.complete = out.accepted;
    return out;
}

PracticeLeadingOverlayDrawList80023618
BuildPracticeLeadingOverlayDrawList80023618(
    const PracticeLeadingOverlayProducerState80023618& state) {
    PracticeLeadingOverlayDrawList80023618 out{};
    out.eventDispatchKnown = state.eventDispatchKnown;
    out.eventIdAccepted = state.eventDispatchKnown && state.eventId == 16u;
    out.languageAccepted = state.languageKnown &&
        state.language >= 0 && state.language < 5;
    out.flagsAccepted = state.flagsKnown;
    out.overlayGateActive = state.flagsKnown &&
        (state.flags00 & 0x00400000u) != 0u;
    out.overlayStateAccepted = !out.overlayGateActive ||
        state.overlayStateKnown;
    out.staticWriterSemanticsKnown = true;
    out.staticDescriptorSourcesKnown = true;
    out.sourceKnown = out.eventIdAccepted && out.languageAccepted &&
        out.flagsAccepted && out.overlayStateAccepted &&
        out.staticWriterSemanticsKnown && out.staticDescriptorSourcesKnown;
    if (!out.sourceKnown) {
        return out;
    }

    PracticeState80023618 traceState{};
    traceState.language = state.language;
    traceState.flags00 = state.flags00;
    traceState.overlayState1C = state.overlayState1C;
    const auto trace = BuildPracticeDrawCallTrace80023618(traceState);
    if (!trace.sourceKnown || !trace.accepted || !trace.complete ||
        trace.truncated || !trace.staticOperandSourcesResolved) {
        return out;
    }

    static constexpr uint32_t kOverlaySpriteCounts[9] = {
        5u, 10u, 10u, 2u, 6u, 6u, 6u, 4u, 4u,
    };
    uint32_t expectedCount = 2u;
    if (out.overlayGateActive) {
        expectedCount = state.overlayState1C >= 0 &&
                state.overlayState1C <= 8
            ? kOverlaySpriteCounts[state.overlayState1C]
            : 0u;
    }
    PracticeLeadingOverlayDrawList80023618 candidate = out;
    for (uint32_t index = 0u; index < expectedCount; ++index) {
        if (index >= trace.count ||
            candidate.count >=
                kPracticeLeadingOverlaySpriteCapacity80023618) {
            candidate.truncated = candidate.count >=
                kPracticeLeadingOverlaySpriteCapacity80023618;
            return out;
        }
        const auto& call = trace.calls[index];
        if ((call.kind != PracticeDrawCallKind80023618::FastSprite8001C5A8 &&
             call.kind != PracticeDrawCallKind80023618::Sprite8001C550) ||
            (call.psxFunction != kFn8001C5A8 &&
             call.psxFunction != kFn8001C550) ||
            !call.positionResolved || !call.templateResolved ||
            call.dynamicPositionRequired || call.dynamicScaleRequired) {
            return out;
        }
        const auto sprite = ResolvePracticeLeadingTemplate80023618(
            call.resolvedTemplateAddress);
        if (!sprite.known) {
            return out;
        }
        auto& command = candidate.sprites[candidate.count];
        command.known = true;
        command.psxFunction = call.psxFunction;
        command.x = call.resolvedX;
        command.y = call.resolvedY;
        command.priority = 0u;
        command.callOrder = candidate.count;
        command.sprite = sprite;
        ++candidate.count;
        candidate.rawTextureOnly = candidate.rawTextureOnly &&
            (sprite.attr & 0x40u) != 0u;
    }

    if (expectedCount < trace.count) {
        const auto nextKind = trace.calls[expectedCount].kind;
        if (nextKind != PracticeDrawCallKind80023618::WobbleUpdate80023F20 &&
            nextKind != PracticeDrawCallKind80023618::Guide80023518) {
            return out;
        }
    }
    candidate.accepted = candidate.count == expectedCount &&
        !candidate.truncated && candidate.rawTextureOnly;
    candidate.complete = candidate.accepted;
    candidate.runtimeSubmitAllowed = candidate.complete;
    return candidate;
}

DirectoryRenderPlan BuildStageSelect80020568Plan()
{
    DirectoryRenderPlan plan =
        MakePlan("StageSelect80020568",
                 DirectoryPage::StageSelectEv2,
                 2);
    plan.blockedByGap = true;

    AppendRoute(plan);
    AppendFixedAnchors(plan);
    AppendLanguageTables(plan);
    AppendAction(plan,
                 DirectoryRenderActionKind::GateStageSelectDrawSource80020568,
                 kFn80020568,
                 kStageSelectCtx80087B78,
                 kGlobalWord800916D8,
                 0,
                 0,
                 0x04,
                 0x08,
                 0x0E,
                 0x1A,
                 true);
    for (uint32_t i = 0; i < KnownStagePointSpecCount(); ++i) {
        const StagePointSpec& point = KnownStagePointSpecAt(i);
        AppendAction(plan,
                     DirectoryRenderActionKind::StagePointTable,
                     kFn80020568,
                     kTableStageSelectPoints80053104,
                     0,
                     point.x,
                     point.y,
                     point.language,
                     point.pointIndex);
    }
    for (uint32_t i = 0; i < KnownStageSliceSpecCount(); ++i) {
        const StageSliceSpec& slice = KnownStageSliceSpecAt(i);
        AppendAction(plan,
                     DirectoryRenderActionKind::StageSliceTable,
                     kFn80020568,
                     kTableStageSelectSlices800531D0,
                     0x80051BF0u,
                     0,
                     0,
                     slice.index,
                     slice.uOffsetPx,
                     slice.widthPx);
    }
    AppendAction(plan,
                 DirectoryRenderActionKind::StageStatusState,
                 kFn80020568,
                 kStageSelectCtx80087B78 + 0x0Eu,
                 kGlobalWord800916D8,
                 0,
                 0,
                 0,
                 3,
                 2);
    AppendSharedExit(plan);
    AppendDirectoryGp0ReplayBaselines(plan);
    AppendAction(plan, DirectoryRenderActionKind::Gap, kFn80020568);
    return plan;
}

DirectoryRenderPlan BuildCardGrid80020F94Plan(int32_t eventId,
                                              uint32_t argAddress,
                                              bool argKnown)
{
    const DirectoryPage page = DirectoryPageFromEventId(eventId);
    if (page != DirectoryPage::CardSaveEv7 &&
        page != DirectoryPage::CardLoadEv8 &&
        page != DirectoryPage::CardReplayEv9) {
        return BuildUnknownPlan(eventId);
    }

    DirectoryRenderPlan plan =
        MakePlan("CardGrid80020F94", page, eventId);
    plan.blockedByGap = true;

    const CardGridSpec grid = KnownCardGridSpec();
    const bool argSourceKnown = argKnown && argAddress != 0u;
    const uint32_t rowsSource = argSourceKnown ? argAddress + 12u : 0u;
    const uint32_t colsSource = argSourceKnown ? argAddress + 14u : 0u;
    const uint32_t selectedSource = argSourceKnown ? argAddress + 20u : 0u;
    const uint32_t exitBlinkSource = argSourceKnown ? argAddress + 4u : 0u;
    AppendRoute(plan);
    AppendFixedAnchors(plan);
    AppendLanguageTables(plan);
    AppendAction(plan,
                 DirectoryRenderActionKind::GateCardGridArgSource80020F94,
                 kFn80020F94,
                 rowsSource,
                 colsSource,
                 0,
                 0,
                 static_cast<int32_t>(argAddress),
                 static_cast<int32_t>(selectedSource),
                 static_cast<int32_t>(exitBlinkSource),
                 argSourceKnown ? 1 : 0,
                 true);
    AppendAction(plan,
                 DirectoryRenderActionKind::CardGridGeometry,
                 kFn80020F94,
                 0,
                 0,
                 grid.startX,
                 grid.startY,
                 grid.stepX,
                 grid.stepY,
                 grid.slotTextX,
                 grid.slotTextY);
    AppendAction(plan,
                 DirectoryRenderActionKind::CardGridGeometry,
                 kFn80020F94,
                 0,
                 0,
                 0,
                 0,
                 grid.glyphClutX,
                 grid.glyphClutYBase,
                 0,
                 1);
    AppendAction(plan,
                 DirectoryRenderActionKind::CardGridExitFromArg,
                 kFn80020F94,
                 0,
                 0,
                 231,
                 179,
                 12,
                 14,
                 20,
                 4);
    AppendSharedExit(plan);
    AppendDirectoryGp0ReplayBaselines(plan);
    AppendAction(plan, DirectoryRenderActionKind::Gap, kFn80020F94);
    return plan;
}

DirectoryRenderPlan BuildOptions80021910Plan()
{
    DirectoryRenderPlan plan =
        MakePlan("OptionsLanguage80021910",
                 DirectoryPage::OptionsEv17,
                 17);
    plan.blockedByGap = true;

    AppendRoute(plan);
    AppendFixedAnchors(plan);
    AppendLanguageTables(plan);
    AppendAction(plan,
                 DirectoryRenderActionKind::GateOptionsGlobalSource80021910,
                 kFn80021910,
                 kGlobalWord800916D8,
                 kGlobalWord800916DC,
                 0,
                 0,
                 0,
                 4,
                 0,
                 1,
                 true);
    AppendAction(plan,
                 DirectoryRenderActionKind::OptionsSubtitleToggle,
                 kFn80021910,
                 kTableOptionsSubtitleOn80053850,
                 0,
                 26,
                 106,
                 0,
                 1);
    AppendAction(plan,
                 DirectoryRenderActionKind::OptionsLanguageList,
                 kFn80021910,
                 kTableOptionsLanguageA800538F0,
                 0,
                 42,
                 127,
                 5,
                 16);
    AppendAction(plan,
                 DirectoryRenderActionKind::OptionsLanguageList,
                 kFn80021910,
                 kTableOptionsLanguageB80053940,
                 0,
                 33,
                 115,
                 5,
                 16);
    AppendSharedExit(plan);
    AppendDirectoryGp0ReplayBaselines(plan);
    AppendAction(plan, DirectoryRenderActionKind::Gap, kFn80021910);
    return plan;
}

DirectoryRenderPlan BuildPracticeDraw80023618Plan(uint32_t ctxAddress,
                                                  bool ctxKnown)
{
    DirectoryRenderPlan plan =
        MakePlan("PracticeDraw80023618",
                 DirectoryPage::PracticeEv16,
                 16);
    plan.blockedByGap = true;
    const bool ctxSourceKnown = ctxKnown && ctxAddress != 0u;
    const uint32_t exitBlinkSource = ctxSourceKnown ? ctxAddress + 0x48u : 0u;
    const uint32_t exitConfirmSource = ctxSourceKnown ? ctxAddress + 0x4Cu : 0u;

    AppendRoute(plan);
    AppendFixedAnchors(plan);
    AppendLanguageTables(plan);
    AppendAction(plan,
                 DirectoryRenderActionKind::PracticeOverlay,
                 kFn80023618,
                 kTablePracticeOverlayA80053C08,
                 0,
                 0,
                 0,
                 0x400000,
                 0x1C);
    AppendAction(plan,
                 DirectoryRenderActionKind::PracticeHeader,
                 kFn80023618,
                 kTablePracticeTitle80053D48,
                 0,
                 0,
                 0,
                 static_cast<int32_t>(kTablePracticeSubtitle80053BB8));
    AppendAction(plan,
                 DirectoryRenderActionKind::PracticeRec44Layer,
                 kFn80023F20,
                 0,
                 0,
                 0,
                 0,
                 0x8C,
                 0x9E);
    AppendAction(plan,
                 DirectoryRenderActionKind::PracticeRec44Layer,
                 kFn800246A8,
                 0,
                 0,
                 0,
                 0,
                 0x8C,
                 0x9E);
    AppendAction(plan,
                 DirectoryRenderActionKind::PracticeRec44Layer,
                 kFn80024600,
                 0,
                 0,
                 0,
                 0,
                 0x8C,
                 0x9E);
    for (uint32_t i = 0; i < KnownPracticeIconTemplateSpecCount(); ++i) {
        const PracticeIconTemplateSpec& icon =
            KnownPracticeIconTemplateSpecAt(i);
        AppendAction(plan,
                     DirectoryRenderActionKind::PracticeIconRow,
                     kFn80024418,
                     kTablePracticeIconTemplate800540BC,
                     icon.templateAddress,
                     0,
                     0,
                     icon.code,
                     0x94,
                     18);
    }
    AppendAction(plan,
                 DirectoryRenderActionKind::GatePracticeExitSource80023618,
                 kFn80023618,
                 exitBlinkSource,
                 exitConfirmSource,
                 0,
                 0,
                 static_cast<int32_t>(ctxAddress),
                 0x48,
                 0x4C,
                 ctxSourceKnown ? 1 : 0,
                 true);
    AppendAction(plan,
                 DirectoryRenderActionKind::PracticeExitPrompt,
                 kFn80023618,
                 kTableSharedExitText80053000,
                 0,
                 231,
                 179,
                 0x48,
                 0x4C);
    AppendDirectoryGp0ReplayBaselines(plan);
    AppendAction(plan, DirectoryRenderActionKind::Gap, kFn80023618);
    return plan;
}

DirectoryRenderPlan BuildDirectoryPageDrawPlan(int32_t eventId)
{
    switch (eventId) {
    case 2:
        return BuildStageSelect80020568Plan();
    case 3:
        return BuildMainDirectory80021E60Plan();
    case 7:
    case 8:
    case 9:
        return BuildCardGrid80020F94Plan(eventId);
    case 16:
        return BuildPracticeDraw80023618Plan();
    case 17:
        return BuildOptions80021910Plan();
    }
    return BuildUnknownPlan(eventId);
}

DirectoryPage DirectoryPageFromEventId(int32_t eventId)
{
    switch (eventId) {
    case 2:
        return DirectoryPage::StageSelectEv2;
    case 3:
        return DirectoryPage::MainDirectoryEv3;
    case 7:
        return DirectoryPage::CardSaveEv7;
    case 8:
        return DirectoryPage::CardLoadEv8;
    case 9:
        return DirectoryPage::CardReplayEv9;
    case 16:
        return DirectoryPage::PracticeEv16;
    case 17:
        return DirectoryPage::OptionsEv17;
    }
    return DirectoryPage::Unknown;
}

const char* DirectoryPageName(DirectoryPage page)
{
    switch (page) {
    case DirectoryPage::Unknown:
        return "Unknown";
    case DirectoryPage::StageSelectEv2:
        return "StageSelectEv2";
    case DirectoryPage::MainDirectoryEv3:
        return "MainDirectoryEv3";
    case DirectoryPage::CardGridFamily:
        return "CardGridFamily";
    case DirectoryPage::CardSaveEv7:
        return "CardSaveEv7";
    case DirectoryPage::CardLoadEv8:
        return "CardLoadEv8";
    case DirectoryPage::CardReplayEv9:
        return "CardReplayEv9";
    case DirectoryPage::PracticeEv16:
        return "PracticeEv16";
    case DirectoryPage::OptionsEv17:
        return "OptionsEv17";
    }
    return "Unknown";
}

const char* DirectoryTableKindName(DirectoryTableKind kind)
{
    switch (kind) {
    case DirectoryTableKind::Unknown:
        return "Unknown";
    case DirectoryTableKind::SharedExitText:
        return "SharedExitText";
    case DirectoryTableKind::StageSelectPoints:
        return "StageSelectPoints";
    case DirectoryTableKind::StageSelectSlices:
        return "StageSelectSlices";
    case DirectoryTableKind::MainLanguage:
        return "MainLanguage";
    case DirectoryTableKind::MainHiScore:
        return "MainHiScore";
    case DirectoryTableKind::MainNormal:
        return "MainNormal";
    case DirectoryTableKind::MainEasy:
        return "MainEasy";
    case DirectoryTableKind::MainPractice:
        return "MainPractice";
    case DirectoryTableKind::MainStageSelect:
        return "MainStageSelect";
    case DirectoryTableKind::MainReplay:
        return "MainReplay";
    case DirectoryTableKind::MainLoad:
        return "MainLoad";
    case DirectoryTableKind::CardPrompt:
        return "CardPrompt";
    case DirectoryTableKind::CardTitle:
        return "CardTitle";
    case DirectoryTableKind::CardFooter:
        return "CardFooter";
    case DirectoryTableKind::CardSaveTextA:
        return "CardSaveTextA";
    case DirectoryTableKind::CardSaveTextB:
        return "CardSaveTextB";
    case DirectoryTableKind::CardLoadTextA:
        return "CardLoadTextA";
    case DirectoryTableKind::CardLoadTextB:
        return "CardLoadTextB";
    case DirectoryTableKind::CardReplayTextA:
        return "CardReplayTextA";
    case DirectoryTableKind::CardReplayTextB:
        return "CardReplayTextB";
    case DirectoryTableKind::OptionsSubtitleOn:
        return "OptionsSubtitleOn";
    case DirectoryTableKind::OptionsSubtitleOff:
        return "OptionsSubtitleOff";
    case DirectoryTableKind::OptionsLanguageA:
        return "OptionsLanguageA";
    case DirectoryTableKind::OptionsLanguageB:
        return "OptionsLanguageB";
    case DirectoryTableKind::PracticeTitle:
        return "PracticeTitle";
    case DirectoryTableKind::PracticeSubtitle:
        return "PracticeSubtitle";
    case DirectoryTableKind::PracticeOverlay:
        return "PracticeOverlay";
    case DirectoryTableKind::PracticeIconTemplate:
        return "PracticeIconTemplate";
    }
    return "Unknown";
}

const char* DirectoryRenderActionKindName(DirectoryRenderActionKind kind)
{
    switch (kind) {
    case DirectoryRenderActionKind::None:
        return "None";
    case DirectoryRenderActionKind::GatePageDrawRoute:
        return "GatePageDrawRoute";
    case DirectoryRenderActionKind::BeginDrawWork8001D74C:
        return "BeginDrawWork8001D74C";
    case DirectoryRenderActionKind::DrawPageFunction:
        return "DrawPageFunction";
    case DirectoryRenderActionKind::FixedSprite:
        return "FixedSprite";
    case DirectoryRenderActionKind::LanguageTextTable:
        return "LanguageTextTable";
    case DirectoryRenderActionKind::StagePointTable:
        return "StagePointTable";
    case DirectoryRenderActionKind::StageSliceTable:
        return "StageSliceTable";
    case DirectoryRenderActionKind::GateStageSelectDrawSource80020568:
        return "GateStageSelectDrawSource80020568";
    case DirectoryRenderActionKind::StageStatusState:
        return "StageStatusState";
    case DirectoryRenderActionKind::GateCardGridArgSource80020F94:
        return "GateCardGridArgSource80020F94";
    case DirectoryRenderActionKind::CardGridGeometry:
        return "CardGridGeometry";
    case DirectoryRenderActionKind::CardGridExitFromArg:
        return "CardGridExitFromArg";
    case DirectoryRenderActionKind::GateOptionsGlobalSource80021910:
        return "GateOptionsGlobalSource80021910";
    case DirectoryRenderActionKind::OptionsLanguageList:
        return "OptionsLanguageList";
    case DirectoryRenderActionKind::OptionsSubtitleToggle:
        return "OptionsSubtitleToggle";
    case DirectoryRenderActionKind::PracticeHeader:
        return "PracticeHeader";
    case DirectoryRenderActionKind::PracticeOverlay:
        return "PracticeOverlay";
    case DirectoryRenderActionKind::PracticeRec44Layer:
        return "PracticeRec44Layer";
    case DirectoryRenderActionKind::PracticeIconRow:
        return "PracticeIconRow";
    case DirectoryRenderActionKind::GatePracticeExitSource80023618:
        return "GatePracticeExitSource80023618";
    case DirectoryRenderActionKind::PracticeExitPrompt:
        return "PracticeExitPrompt";
    case DirectoryRenderActionKind::SharedExit:
        return "SharedExit";
    case DirectoryRenderActionKind::SharedExitIconTemplate:
        return "SharedExitIconTemplate";
    case DirectoryRenderActionKind::GateMainDirectoryStateSource80021E60:
        return "GateMainDirectoryStateSource80021E60";
    case DirectoryRenderActionKind::StateField:
        return "StateField";
    case DirectoryRenderActionKind::DirectoryGp0ReplayBaseline:
        return "DirectoryGp0ReplayBaseline";
    case DirectoryRenderActionKind::Gap:
        return "Gap";
    }
    return "Unknown";
}

} // namespace PrSS0DirectoryPagesRenderDirect
