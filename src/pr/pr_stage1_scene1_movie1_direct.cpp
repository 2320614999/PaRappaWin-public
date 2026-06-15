#include "pr_stage1_scene1_movie1_direct.h"

#include "logger.h"
#include "pr_psx_graph_owner_direct.h"
#include "pr_stage1_movie_text_direct.h"
#include "str_player.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>

namespace PrStage1Scene1Movie1Direct {
namespace {

static constexpr int kMovie1PsxScreenW = 320;
static constexpr int kMovie1PsxScreenH = 240;
static constexpr int kMovie1PsxTilePx = 20;
static constexpr int kMovie1PsxMetaTilePx = 40;
static constexpr int kStage1IntroTransitionRows = kMovie1PsxScreenH / kMovie1PsxTilePx;
static constexpr int kStage1IntroTransitionCols = kMovie1PsxScreenW / kMovie1PsxTilePx;
static constexpr int kSub8001FEB4NoVideoGridRows = kMovie1PsxScreenH / kMovie1PsxMetaTilePx;
static constexpr int kSub8001FEB4NoVideoGridCols = kMovie1PsxScreenW / kMovie1PsxMetaTilePx;
static constexpr int kSub8001FEB4TilesPerMetaTile = 4;
static constexpr uint32_t kMovie1StrSkipInputMaskSub80035510 = 0x0100u;

struct Movie1PsxRect {
    int16_t x = 0;
    int16_t y = 0;
    int16_t w = 0;
    int16_t h = 0;
};

static constexpr Movie1PsxRect kSub8001CE30Mode5VideoRect = {
    35, 48, 256, 144,
};

static constexpr Movie1PsxRect kSub8001C864Mode5VideoRect = {
    35, 25, 256, 144,
};

static constexpr Movie1PsxRect kSub8001C864Mode5SubtitleBoxRect = {
    24, 184, 272, 32,
};

PrPsxGraphOwnerDirect::PsxGraphState MakeMovie1GraphOwner8003FB9C() {
    PrPsxGraphOwnerDirect::PsxGraphState graph{};
    PrPsxGraphOwnerDirect::PsxInitializeGraphState8003FB9C(graph, 320u, 240u);
    graph.word_800965A0 = 4u;
    PrPsxGraphOwnerDirect::PsxCall80040B84_ApplyScreenCenterAndDrawOffset(
        graph);
    return graph;
}

static constexpr uint8_t kStage1IntroTransitionLifeByRowCol
    [kStage1IntroTransitionRows][kStage1IntroTransitionCols] = {
    { 76, 59, 60, 60, 60, 61, 61, 61, 62, 62, 62, 63, 63, 63, 64, 64 },
    { 76, 59, 45, 45, 45, 46, 46, 46, 47, 47, 47, 48, 48, 48, 49, 64 },
    { 76, 59, 44, 33, 33, 33, 34, 34, 34, 35, 35, 35, 36, 36, 49, 65 },
    { 75, 58, 44, 32, 23, 24, 24, 24, 25, 25, 25, 26, 26, 36, 49, 65 },
    { 75, 58, 44, 32, 23, 17, 17, 17, 18, 18, 18, 19, 26, 37, 50, 65 },
    { 75, 58, 43, 32, 23, 16, 13, 13, 13, 14, 14, 19, 27, 37, 50, 66 },
    { 74, 57, 43, 31, 22, 16, 16, 15, 15, 15, 14, 19, 27, 37, 50, 66 },
    { 74, 57, 43, 31, 22, 22, 21, 21, 21, 20, 20, 20, 27, 38, 51, 66 },
    { 74, 57, 42, 31, 30, 30, 30, 29, 29, 29, 28, 28, 28, 38, 51, 67 },
    { 73, 56, 42, 42, 41, 41, 41, 40, 40, 40, 39, 39, 39, 38, 51, 67 },
    { 73, 56, 56, 55, 55, 55, 54, 54, 54, 53, 53, 53, 52, 52, 52, 67 },
    { 73, 72, 72, 72, 71, 71, 71, 70, 70, 70, 69, 69, 69, 68, 68, 68 },
};

static constexpr uint32_t kSub800201ACLoopFrames = 191u;
static constexpr uint32_t kSub800201ACTailFrames = 4u;
static constexpr uint32_t kSub80020308SubtitleHoldFrames = 15u;
static constexpr uint32_t kSub80020308SubtitleCompleteFrames = 31u;
static constexpr uint32_t kSub80020308NoSubtitleCompleteFrames = 16u;
static constexpr uint32_t kSub80020248SubtitleCompleteFrames = 31u;
static constexpr uint32_t kSub80020248NoSubtitleCompleteFrames = 16u;
static constexpr uint32_t kSkipOutroLeadInFrames = 4;
static constexpr uint16_t kStage1IntroTransitionLayer = 900u;
static constexpr uint32_t kSub800201ACWork801C3640 = 0x801C3640u;
static constexpr uint32_t kSub800201ACPostMovieMode = 5u;
static constexpr uint32_t kSub800201ACPostMoviePreFfd4Arg = 1u;
static constexpr uint32_t kSub800201ACPostMoviePostFfd4Arg = 2u;
static constexpr uint32_t kMovie1PsxWorkBufferBase80087288 = 0x80087288u;
static constexpr uint32_t kMovie1PsxLanguageWord800916D8 = 0x800916D8u;
static constexpr uint32_t kMovie1PsxGpBase8006EA40 = 0x8006EA40u;

struct Movie1PsxTemplateCommand {
    int16_t x = 0;
    int16_t y = 0;
    uint32_t tplAddr = 0;
    uint16_t layer = kMovie1PsxDefaultLayer;
    Movie1PsxTemplateSubmitHelper submitHelper =
        Movie1PsxTemplateSubmitHelper::Sub8001B590;
};

struct Movie1PsxTemplateRepeatCommand {
    int16_t x = 0;
    int16_t y = 0;
    int16_t stepX = 0;
    int16_t stepY = 0;
    uint8_t count = 0;
    uint32_t tplAddr = 0;
    uint16_t layer = kMovie1PsxDefaultLayer;
    Movie1PsxTemplateSubmitHelper submitHelper =
        Movie1PsxTemplateSubmitHelper::Sub8001B590;
};

struct Movie1TransitionTileMaskPair {
    uint8_t row = 0;
    uint8_t col = 0;
};

static constexpr uint32_t kOff80050720TransitionTemplatePtrs[4] = {
    0x800503E0u, 0x800503F0u, 0x80050400u, 0x80050410u,
};

static constexpr uint8_t kDword80050420TransitionTileIndex
    [kStage1IntroTransitionRows][kStage1IntroTransitionCols] = {
    { 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1 },
    { 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3 },
    { 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1 },
    { 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3 },
    { 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1 },
    { 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3 },
    { 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1 },
    { 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3 },
    { 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1 },
    { 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3 },
    { 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1 },
    { 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3 },
};

// PS-X EXE static data: dword_8004EB80/dword_8004EB84, file offset 0x3F380.
static constexpr Movie1TransitionTileMaskPair kDword8004EB80ClearOrderPairs
    [kMovie1TransitionTileMaskCellCount] = {
    { 0, 0 }, { 0, 1 }, { 0, 2 }, { 0, 3 }, { 0, 4 }, { 0, 5 }, { 0, 6 }, { 0, 7 },
    { 0, 8 }, { 0, 9 }, { 0, 10 }, { 0, 11 }, { 0, 12 }, { 0, 13 }, { 0, 14 }, { 0, 15 },
    { 1, 15 }, { 2, 15 }, { 3, 15 }, { 4, 15 }, { 5, 15 }, { 6, 15 }, { 7, 15 }, { 8, 15 },
    { 9, 15 }, { 10, 15 }, { 11, 15 }, { 11, 14 }, { 11, 13 }, { 11, 12 }, { 11, 11 }, { 11, 10 },
    { 11, 9 }, { 11, 8 }, { 11, 7 }, { 11, 6 }, { 11, 5 }, { 11, 4 }, { 11, 3 }, { 11, 2 },
    { 11, 1 }, { 11, 0 }, { 10, 0 }, { 9, 0 }, { 8, 0 }, { 7, 0 }, { 6, 0 }, { 5, 0 },
    { 4, 0 }, { 3, 0 }, { 2, 0 }, { 1, 0 }, { 1, 1 }, { 1, 2 }, { 1, 3 }, { 1, 4 },
    { 1, 5 }, { 1, 6 }, { 1, 7 }, { 1, 8 }, { 1, 9 }, { 1, 10 }, { 1, 11 }, { 1, 12 },
    { 1, 13 }, { 1, 14 }, { 2, 14 }, { 3, 14 }, { 4, 14 }, { 5, 14 }, { 6, 14 }, { 7, 14 },
    { 8, 14 }, { 9, 14 }, { 10, 14 }, { 10, 13 }, { 10, 12 }, { 10, 11 }, { 10, 10 }, { 10, 9 },
    { 10, 8 }, { 10, 7 }, { 10, 6 }, { 10, 5 }, { 10, 4 }, { 10, 3 }, { 10, 2 }, { 10, 1 },
    { 9, 1 }, { 8, 1 }, { 7, 1 }, { 6, 1 }, { 5, 1 }, { 4, 1 }, { 3, 1 }, { 2, 1 },
    { 2, 2 }, { 2, 3 }, { 2, 4 }, { 2, 5 }, { 2, 6 }, { 2, 7 }, { 2, 8 }, { 2, 9 },
    { 2, 10 }, { 2, 11 }, { 2, 12 }, { 2, 13 }, { 3, 13 }, { 4, 13 }, { 5, 13 }, { 6, 13 },
    { 7, 13 }, { 8, 13 }, { 9, 13 }, { 9, 12 }, { 9, 11 }, { 9, 10 }, { 9, 9 }, { 9, 8 },
    { 9, 7 }, { 9, 6 }, { 9, 5 }, { 9, 4 }, { 9, 3 }, { 9, 2 }, { 8, 2 }, { 7, 2 },
    { 6, 2 }, { 5, 2 }, { 4, 2 }, { 3, 2 }, { 3, 3 }, { 3, 4 }, { 3, 5 }, { 3, 6 },
    { 3, 7 }, { 3, 8 }, { 3, 9 }, { 3, 10 }, { 3, 11 }, { 3, 12 }, { 4, 12 }, { 5, 12 },
    { 6, 12 }, { 7, 12 }, { 8, 12 }, { 8, 11 }, { 8, 10 }, { 8, 9 }, { 8, 8 }, { 8, 7 },
    { 8, 6 }, { 8, 5 }, { 8, 4 }, { 8, 3 }, { 7, 3 }, { 6, 3 }, { 5, 3 }, { 4, 3 },
    { 4, 4 }, { 4, 5 }, { 4, 6 }, { 4, 7 }, { 4, 8 }, { 4, 9 }, { 4, 10 }, { 4, 11 },
    { 5, 11 }, { 6, 11 }, { 7, 11 }, { 7, 10 }, { 7, 9 }, { 7, 8 }, { 7, 7 }, { 7, 6 },
    { 7, 5 }, { 7, 4 }, { 6, 4 }, { 5, 4 }, { 5, 5 }, { 5, 6 }, { 5, 7 }, { 5, 8 },
    { 5, 9 }, { 5, 10 }, { 6, 10 }, { 6, 9 }, { 6, 8 }, { 6, 7 }, { 6, 6 }, { 6, 5 },
};

// PS-X EXE static data: dword_8004F180/dword_8004F184, file offset 0x3F980.
static constexpr Movie1TransitionTileMaskPair kDword8004F180Pattern4OrderPairs
    [kMovie1TransitionTileMaskCellCount] = {
    { 0, 0 }, { 1, 0 }, { 2, 0 }, { 3, 0 }, { 4, 0 }, { 5, 0 }, { 6, 0 }, { 7, 0 },
    { 8, 0 }, { 9, 0 }, { 10, 0 }, { 11, 0 }, { 11, 1 }, { 11, 2 }, { 11, 3 }, { 11, 4 },
    { 11, 5 }, { 11, 6 }, { 11, 7 }, { 11, 8 }, { 11, 9 }, { 11, 10 }, { 11, 11 }, { 11, 12 },
    { 11, 13 }, { 11, 14 }, { 11, 15 }, { 10, 15 }, { 9, 15 }, { 8, 15 }, { 7, 15 }, { 6, 15 },
    { 5, 15 }, { 4, 15 }, { 3, 15 }, { 2, 15 }, { 1, 15 }, { 0, 15 }, { 0, 14 }, { 0, 13 },
    { 0, 12 }, { 0, 11 }, { 0, 10 }, { 0, 9 }, { 0, 8 }, { 0, 7 }, { 0, 6 }, { 0, 5 },
    { 0, 4 }, { 0, 3 }, { 0, 2 }, { 0, 1 }, { 1, 1 }, { 2, 1 }, { 3, 1 }, { 4, 1 },
    { 5, 1 }, { 6, 1 }, { 7, 1 }, { 8, 1 }, { 9, 1 }, { 10, 1 }, { 10, 2 }, { 10, 3 },
    { 10, 4 }, { 10, 5 }, { 10, 6 }, { 10, 7 }, { 10, 8 }, { 10, 9 }, { 10, 10 }, { 10, 11 },
    { 10, 12 }, { 10, 13 }, { 10, 14 }, { 9, 14 }, { 8, 14 }, { 7, 14 }, { 6, 14 }, { 5, 14 },
    { 4, 14 }, { 3, 14 }, { 2, 14 }, { 1, 14 }, { 1, 13 }, { 1, 12 }, { 1, 11 }, { 1, 10 },
    { 1, 9 }, { 1, 8 }, { 1, 7 }, { 1, 6 }, { 1, 5 }, { 1, 4 }, { 1, 3 }, { 1, 2 },
    { 2, 2 }, { 3, 2 }, { 4, 2 }, { 5, 2 }, { 6, 2 }, { 7, 2 }, { 8, 2 }, { 9, 2 },
    { 9, 3 }, { 9, 4 }, { 9, 5 }, { 9, 6 }, { 9, 7 }, { 9, 8 }, { 9, 9 }, { 9, 10 },
    { 9, 11 }, { 9, 12 }, { 9, 13 }, { 8, 13 }, { 7, 13 }, { 6, 13 }, { 5, 13 }, { 4, 13 },
    { 3, 13 }, { 2, 13 }, { 2, 12 }, { 2, 11 }, { 2, 10 }, { 2, 9 }, { 2, 8 }, { 2, 7 },
    { 2, 6 }, { 2, 5 }, { 2, 4 }, { 2, 3 }, { 3, 3 }, { 4, 3 }, { 5, 3 }, { 6, 3 },
    { 7, 3 }, { 8, 3 }, { 8, 4 }, { 8, 5 }, { 8, 6 }, { 8, 7 }, { 8, 8 }, { 8, 9 },
    { 8, 10 }, { 8, 11 }, { 8, 12 }, { 7, 12 }, { 6, 12 }, { 5, 12 }, { 4, 12 }, { 3, 12 },
    { 3, 11 }, { 3, 10 }, { 3, 9 }, { 3, 8 }, { 3, 7 }, { 3, 6 }, { 3, 5 }, { 3, 4 },
    { 4, 4 }, { 5, 4 }, { 6, 4 }, { 7, 4 }, { 7, 5 }, { 7, 6 }, { 7, 7 }, { 7, 8 },
    { 7, 9 }, { 7, 10 }, { 7, 11 }, { 6, 11 }, { 5, 11 }, { 4, 11 }, { 4, 10 }, { 4, 9 },
    { 4, 8 }, { 4, 7 }, { 4, 6 }, { 4, 5 }, { 5, 5 }, { 6, 5 }, { 6, 6 }, { 6, 7 },
    { 6, 8 }, { 6, 9 }, { 6, 10 }, { 5, 10 }, { 5, 9 }, { 5, 8 }, { 5, 7 }, { 5, 6 },
};

static constexpr Movie1PsxTemplateRepeatCommand kSub8001CE30Mode5OuterStrip[] = {
    { 40,   0, 40,  0, 7, 0x8004E900u },
    { 20,   0, 40,  0, 7, 0x8004E910u },
    { 40, 220, 40,  0, 7, 0x8004E920u },
    { 20, 220, 40,  0, 7, 0x8004E930u },
    { 40,  20, 40,  0, 7, 0x8004E920u },
    { 20,  20, 40,  0, 7, 0x8004E930u },
    { 40, 200, 40,  0, 7, 0x8004E900u },
    { 20, 200, 40,  0, 7, 0x8004E910u },
};

static constexpr Movie1PsxTemplateRepeatCommand kSub8001CE30Mode5SideStrip[] = {
    {  0,  20,  0, 40, 6, 0x8004E920u },
    {  0,   0,  0, 40, 6, 0x8004E900u },
    {300,  20,  0, 40, 6, 0x8004E930u },
    {300,   0,  0, 40, 6, 0x8004E910u },
};

static constexpr uint32_t kSub8001CE30Mode5LowerRowTpl[12] = {
    0x8004EA60u, 0x8004EA70u, 0x8004EA80u, 0x8004EA90u,
    0x8004EAA0u, 0x8004EAB0u, 0x8004EAC0u, 0x8004EAD0u,
    0x8004EAE0u, 0x8004EAF0u, 0x8004EB00u, 0x8004EB10u,
};

static constexpr Movie1PsxTemplateCommand kSub8001CE30Mode5TopRow[] = {
    {280, 40, 0x8004E980u },
    { 20, 40, 0x8004EA50u },
    {160, 40, 0x8004E9F0u },
    {140, 40, 0x8004E9E0u },
    {180, 40, 0x8004EA00u },
    {120, 40, 0x8004E9D0u },
    {200, 40, 0x8004EA10u },
    {100, 40, 0x8004E9C0u },
    {240, 40, 0x8004EA30u },
    { 80, 40, 0x8004E9B0u },
    {220, 40, 0x8004EA20u },
    { 60, 40, 0x8004E9A0u },
    {260, 40, 0x8004EA40u },
    { 40, 40, 0x8004E990u },
};

static constexpr Movie1PsxTemplateCommand kSub8001C864Mode5FixedBegin[] = {
    {280, 200, 0x8004E940u },
    { 20, 200, 0x8004E950u },
    {280, 180, 0x8004E960u },
    { 20, 180, 0x8004E970u },
};

static constexpr Movie1PsxTemplateRepeatCommand kSub8001C864Mode5OuterStrip[] = {
    { 40,   0, 40,  0, 7, 0x8004E900u },
    { 20,   0, 40,  0, 7, 0x8004E910u },
    { 40, 220, 40,  0, 7, 0x8004E920u },
    { 20, 220, 40,  0, 7, 0x8004E930u },
};

static constexpr Movie1PsxTemplateRepeatCommand kSub8001C864Mode5SubtitleStrip[] = {
    {  0,  20,  0, 40, 6, 0x8004E920u },
    {  0,   0,  0, 40, 6, 0x8004E900u },
    {300,  20,  0, 40, 6, 0x8004E930u },
    {300,   0,  0, 40, 6, 0x8004E910u },
    { 40, 200, 40,  0, 6, 0x8004E860u },
    { 60, 200, 40,  0, 6, 0x8004E870u },
    { 40, 180, 40,  0, 6, 0x8004E880u },
    { 60, 180, 40,  0, 6, 0x8004E890u },
};

static constexpr Movie1PsxTemplateCommand kSub8001C864Mode5FixedTail[] = {
    { 40, 160, 0x8004E8E0u },
    {160, 160, 0x8004E8F0u },
    { 40,  20, 0x8004E8C0u },
    {160,  20, 0x8004E8D0u },
    { 20,  20, 0x8004E8A0u },
    {280,  20, 0x8004E8B0u },
};

static constexpr Movie1PsxTemplateCommand kSub8001F230Mode5FixedBegin[] = {
    { 20,  20, 0x80050380u, kMovie1PsxDefaultLayer, Movie1PsxTemplateSubmitHelper::Sub8001C550 },
    {280,  20, 0x80050390u, kMovie1PsxDefaultLayer, Movie1PsxTemplateSubmitHelper::Sub8001C550 },
    { 40,  20, 0x800503A0u, kMovie1PsxDefaultLayer, Movie1PsxTemplateSubmitHelper::Sub8001C550 },
    {160,  20, 0x800503B0u, kMovie1PsxDefaultLayer, Movie1PsxTemplateSubmitHelper::Sub8001C550 },
    { 40, 160, 0x800503C0u, kMovie1PsxDefaultLayer, Movie1PsxTemplateSubmitHelper::Sub8001C550 },
    {160, 160, 0x800503D0u, kMovie1PsxDefaultLayer, Movie1PsxTemplateSubmitHelper::Sub8001C550 },
};

static constexpr Movie1PsxTemplateRepeatCommand kSub8001F230Mode5OuterStrip[] = {
    { 40,   0, 40,  0, 7, 0x800503E0u, kMovie1PsxDefaultLayer, Movie1PsxTemplateSubmitHelper::Sub8001C550 },
    { 20,   0, 40,  0, 7, 0x800503F0u, kMovie1PsxDefaultLayer, Movie1PsxTemplateSubmitHelper::Sub8001C550 },
    { 40, 220, 40,  0, 7, 0x80050400u, kMovie1PsxDefaultLayer, Movie1PsxTemplateSubmitHelper::Sub8001C550 },
    { 20, 220, 40,  0, 7, 0x80050410u, kMovie1PsxDefaultLayer, Movie1PsxTemplateSubmitHelper::Sub8001C550 },
};

static constexpr Movie1PsxTemplateRepeatCommand kSub8001F230Mode5InnerStrip[] = {
    {  0,  20,  0, 40, 6, 0x80050400u, kMovie1PsxDefaultLayer, Movie1PsxTemplateSubmitHelper::Sub8001C550 },
    {  0,   0,  0, 40, 6, 0x800503E0u, kMovie1PsxDefaultLayer, Movie1PsxTemplateSubmitHelper::Sub8001C550 },
    {300,  20,  0, 40, 6, 0x80050410u, kMovie1PsxDefaultLayer, Movie1PsxTemplateSubmitHelper::Sub8001C550 },
    {300,   0,  0, 40, 6, 0x800503F0u, kMovie1PsxDefaultLayer, Movie1PsxTemplateSubmitHelper::Sub8001C550 },
    { 40, 200, 40,  0, 6, 0x800503E0u, kMovie1PsxDefaultLayer, Movie1PsxTemplateSubmitHelper::Sub8001C550 },
    { 60, 200, 40,  0, 6, 0x800503F0u, kMovie1PsxDefaultLayer, Movie1PsxTemplateSubmitHelper::Sub8001C550 },
    { 40, 180, 40,  0, 6, 0x80050400u, kMovie1PsxDefaultLayer, Movie1PsxTemplateSubmitHelper::Sub8001C550 },
    { 60, 180, 40,  0, 6, 0x80050410u, kMovie1PsxDefaultLayer, Movie1PsxTemplateSubmitHelper::Sub8001C550 },
};

static constexpr Movie1PsxTemplateCommand kSub8001F230Mode5FixedTail[] = {
    {280, 200, 0x800503E0u, kMovie1PsxDefaultLayer, Movie1PsxTemplateSubmitHelper::Sub8001C550 },
    { 20, 200, 0x800503F0u, kMovie1PsxDefaultLayer, Movie1PsxTemplateSubmitHelper::Sub8001C550 },
    {280, 180, 0x80050400u, kMovie1PsxDefaultLayer, Movie1PsxTemplateSubmitHelper::Sub8001C550 },
    { 20, 180, 0x80050410u, kMovie1PsxDefaultLayer, Movie1PsxTemplateSubmitHelper::Sub8001C550 },
};

static uint32_t GetStage1IntroTransitionTplAddr(int cellX, int cellY) {
    const uint8_t tileIndex = kDword80050420TransitionTileIndex[cellY][cellX] & 3u;
    return kOff80050720TransitionTemplatePtrs[tileIndex];
}

static bool IsStage1IntroTransitionCellAlive(int frameIndex, int cellX, int cellY) {
    return frameIndex < (int)kStage1IntroTransitionLifeByRowCol[cellY][cellX];
}

static const Movie1PsxRect& SelectMovie1VideoRectPsx(bool subtitleEnabled) {
    return subtitleEnabled
        ? kSub8001C864Mode5VideoRect
        : kSub8001CE30Mode5VideoRect;
}

static Movie1PsxTemplateCommand BuildSub8001FEB4NoVideoTileCommand(int col,
                                                                   int row,
                                                                   int tile) {
    const int localX = (tile & 1) * kMovie1PsxTilePx;
    const int localY = ((tile >> 1) & 1) * kMovie1PsxTilePx;
    Movie1PsxTemplateCommand command{};
    command.x = (int16_t)(kMovie1PsxMetaTilePx * col + localX);
    command.y = (int16_t)(kMovie1PsxMetaTilePx * row + localY);
    command.tplAddr = kOff80050720TransitionTemplatePtrs[tile];
    command.submitHelper = Movie1PsxTemplateSubmitHelper::Sub8001C550;
    return command;
}

static bool AppendMovie1TemplateDrawCommand(Movie1DrawPlan& plan,
                                            const Movie1PsxTemplateCommand& command,
                                            int& order) {
    if (plan.templates.size() >= kMovie1DrawPlanMaxTemplates) {
        return false;
    }

    plan.templates.push_back(Movie1TemplateDrawCommand{});
    plan.templateCount = (uint32_t)plan.templates.size();
    Movie1TemplateDrawCommand& out = plan.templates.back();
    out.x = (float)command.x;
    out.y = (float)command.y;
    out.tplAddr = command.tplAddr;
    out.desc = ResolveMovie1PsxTemplateDesc(command.tplAddr);
    out.submitHelper = command.submitHelper;
    out.alpha = 1.0f;
    out.layer = (int)command.layer;
    // D3D11Renderer treats order 0 as auto-order; PSX call order 0 is explicit.
    out.order = ++order;
    return true;
}

template <std::size_t N>
static bool AppendMovie1TemplateDrawCommands(Movie1DrawPlan& plan,
                                             const Movie1PsxTemplateCommand (&commands)[N],
                                             int& order) {
    for (const Movie1PsxTemplateCommand& command : commands) {
        if (!AppendMovie1TemplateDrawCommand(plan, command, order)) {
            return false;
        }
    }
    return true;
}

template <std::size_t N>
static bool AppendMovie1TemplateRepeatCommandGroup(
    Movie1DrawPlan& plan,
    const Movie1PsxTemplateRepeatCommand (&commands)[N],
    int& order) {
    uint8_t maxCount = 0;
    for (const Movie1PsxTemplateRepeatCommand& command : commands) {
        maxCount = (std::max)(maxCount, command.count);
    }
    for (uint8_t i = 0; i < maxCount; ++i) {
        for (const Movie1PsxTemplateRepeatCommand& command : commands) {
            if (i >= command.count) {
                continue;
            }
            Movie1PsxTemplateCommand emitted{};
            emitted.x = (int16_t)(command.x + command.stepX * (int)i);
            emitted.y = (int16_t)(command.y + command.stepY * (int)i);
            emitted.tplAddr = command.tplAddr;
            emitted.layer = command.layer;
            emitted.submitHelper = command.submitHelper;
            if (!AppendMovie1TemplateDrawCommand(plan, emitted, order)) {
                return false;
            }
        }
    }
    return true;
}

static bool AppendSub8001CE30NoSubtitleFramePlan(Movie1DrawPlan& plan) {
    int order = 0;
    if (!AppendMovie1TemplateRepeatCommandGroup(
            plan, kSub8001CE30Mode5OuterStrip, order)) {
        return false;
    }
    if (!AppendMovie1TemplateRepeatCommandGroup(
            plan, kSub8001CE30Mode5SideStrip, order)) {
        return false;
    }
    for (std::size_t i = 0; i < 12; ++i) {
        const Movie1PsxTemplateCommand command{
            (int16_t)(40 + 20 * (int)i), 180, kSub8001CE30Mode5LowerRowTpl[i] };
        if (!AppendMovie1TemplateDrawCommand(plan, command, order)) {
            return false;
        }
    }
    if (!AppendMovie1TemplateDrawCommands(plan, kSub8001CE30Mode5TopRow, order)) {
        return false;
    }
    return true;
}

static bool AppendSub8001C864SubtitleFramePlan(Movie1DrawPlan& plan) {
    int order = 0;
    return AppendMovie1TemplateDrawCommands(
               plan, kSub8001C864Mode5FixedBegin, order) &&
           AppendMovie1TemplateRepeatCommandGroup(
               plan, kSub8001C864Mode5OuterStrip, order) &&
           AppendMovie1TemplateRepeatCommandGroup(
               plan, kSub8001C864Mode5SubtitleStrip, order) &&
           AppendMovie1TemplateDrawCommands(
               plan, kSub8001C864Mode5FixedTail, order);
}

static bool AppendSub8001F230OutroNoSubboxFramePlan(Movie1DrawPlan& plan) {
    int order = 0;
    return AppendMovie1TemplateDrawCommands(
               plan, kSub8001F230Mode5FixedBegin, order) &&
           AppendMovie1TemplateRepeatCommandGroup(
               plan, kSub8001F230Mode5OuterStrip, order) &&
           AppendMovie1TemplateRepeatCommandGroup(
               plan, kSub8001F230Mode5InnerStrip, order) &&
           AppendMovie1TemplateDrawCommands(
               plan, kSub8001F230Mode5FixedTail, order);
}

static bool AppendSub8001FEB4NoVideoLayoutPlan(Movie1DrawPlan& plan) {
    int order = 0;
    for (int col = 0; col < kSub8001FEB4NoVideoGridCols; ++col) {
        for (int row = 0; row < kSub8001FEB4NoVideoGridRows; ++row) {
            for (int tile = 0; tile < kSub8001FEB4TilesPerMetaTile; ++tile) {
                const Movie1PsxTemplateCommand command =
                    BuildSub8001FEB4NoVideoTileCommand(col, row, tile);
                if (!AppendMovie1TemplateDrawCommand(plan, command, order)) {
                    return false;
                }
            }
        }
    }
    return true;
}

static Movie1PsxDrawHelper ResolveMovie1PsxDrawHelper(const Movie1FrameState& frame) {
    if (frame.useFinalNoVideoLayout) {
        return Movie1PsxDrawHelper::Sub8001FEB4_FinalNoVideoFrame;
    }
    if (frame.useEndingNoSubBoxLayout) {
        return Movie1PsxDrawHelper::Sub8001F230_OutroNoSubboxFrame;
    }
    if (frame.useSubtitleLayout) {
        return Movie1PsxDrawHelper::Sub8001C864_SubtitleFrame;
    }
    return Movie1PsxDrawHelper::Sub8001CE30_NoSubtitleFrame;
}

static void ApplyMovie1PsxDrawHelperLayout(Movie1FrameState& frame,
                                           Movie1PsxDrawHelper helper) {
    frame.useFinalNoVideoLayout =
        helper == Movie1PsxDrawHelper::Sub8001FEB4_FinalNoVideoFrame;
    frame.useEndingNoSubBoxLayout =
        helper == Movie1PsxDrawHelper::Sub8001F230_OutroNoSubboxFrame;
    frame.useSubtitleLayout =
        helper == Movie1PsxDrawHelper::Sub8001C864_SubtitleFrame;
    frame.drawSubBox = frame.useSubtitleLayout;
    frame.psxDrawHelper = helper;
}

static bool AppendMovie1PsxDrawHelperPlan(Movie1DrawPlan& plan,
                                          Movie1PsxDrawHelper helper) {
    switch (helper) {
        case Movie1PsxDrawHelper::Sub8001FEB4_FinalNoVideoFrame:
            return AppendSub8001FEB4NoVideoLayoutPlan(plan);
        case Movie1PsxDrawHelper::Sub8001F230_OutroNoSubboxFrame:
            return AppendSub8001F230OutroNoSubboxFramePlan(plan);
        case Movie1PsxDrawHelper::Sub8001C864_SubtitleFrame:
            return AppendSub8001C864SubtitleFramePlan(plan);
        case Movie1PsxDrawHelper::Sub800201AC_TransitionFrame:
            // sub_800201AC frames are emitted through TransitionSub800201ACStep raw actions.
            return true;
        case Movie1PsxDrawHelper::Sub8001CE30_NoSubtitleFrame:
        default:
            return AppendSub8001CE30NoSubtitleFramePlan(plan);
    }
}

struct Movie1PsxStaticTemplateEntry {
    uint32_t addr = 0;
    Movie1PsxTemplateDesc tpl{};
};

static constexpr Movie1PsxStaticTemplateEntry kMovie1PsxTemplates[] = {
    {0x8004E6B0u, {0x50000040u, 0x0380u, 0x01C2u, 0x0040u, 0x0023u, 0x0130u, 0x01F3u, true}},
    {0x8004E6C0u, {0x50000040u, 0x0380u, 0x019Fu, 0x0050u, 0x0023u, 0x0130u, 0x01F4u, true}},
    {0x8004E6D0u, {0x50000040u, 0x03C0u, 0x01EFu, 0x0070u, 0x0010u, 0x0120u, 0x01F4u, true}},
    {0x8004E6E0u, {0x50000040u, 0x03F4u, 0x01EDu, 0x0020u, 0x000Cu, 0x0130u, 0x01E0u, true}},
    {0x8004E6F0u, {0x50000040u, 0x03E0u, 0x0181u, 0x0024u, 0x000Cu, 0x0130u, 0x01E1u, true}},
    {0x8004E700u, {0x50000040u, 0x03E9u, 0x0181u, 0x0018u, 0x000Cu, 0x0130u, 0x01E2u, true}},
    {0x8004E710u, {0x50000040u, 0x03EFu, 0x0181u, 0x002Cu, 0x000Cu, 0x0130u, 0x01E3u, true}},
    {0x8004E720u, {0x50000040u, 0x03EEu, 0x01A9u, 0x0018u, 0x0018u, 0x0120u, 0x01EAu, true}},
    {0x8004E730u, {0x50000040u, 0x03E8u, 0x01A9u, 0x0018u, 0x0018u, 0x0120u, 0x01EBu, true}},
    {0x8004E740u, {0x50000040u, 0x03D7u, 0x0181u, 0x0024u, 0x000Cu, 0x0130u, 0x01EDu, true}},
    {0x8004E750u, {0x50000040u, 0x03C0u, 0x0181u, 0x0009u, 0x000Cu, 0x0130u, 0x01ECu, true}},
    {0x8004E760u, {0x50000040u, 0x03C0u, 0x018Du, 0x005Cu, 0x001Cu, 0x0130u, 0x01E5u, true}},
    {0x8004E770u, {0x50000040u, 0x03D7u, 0x018Du, 0x0014u, 0x001Cu, 0x0130u, 0x01E6u, true}},
    {0x8004E780u, {0x50000040u, 0x03DCu, 0x018Du, 0x0014u, 0x001Cu, 0x0130u, 0x01E7u, true}},
    {0x8004E790u, {0x50000040u, 0x03E1u, 0x018Du, 0x0014u, 0x001Cu, 0x0130u, 0x01E8u, true}},
    {0x8004E7A0u, {0x50000040u, 0x03E6u, 0x018Du, 0x0014u, 0x001Cu, 0x0130u, 0x01E9u, true}},
    {0x8004E7B0u, {0x50000040u, 0x03EBu, 0x018Du, 0x0014u, 0x001Cu, 0x0130u, 0x01EAu, true}},
    {0x8004E7C0u, {0x50000040u, 0x03F0u, 0x018Du, 0x0014u, 0x001Cu, 0x0130u, 0x01EBu, true}},
    {0x8004E7D0u, {0x50000040u, 0x0332u, 0x0100u, 0x0028u, 0x0028u, 0x0100u, 0x01EAu, true}},
    {0x8004E7E0u, {0x50000040u, 0x0300u, 0x0100u, 0x0014u, 0x0064u, 0x0100u, 0x01EBu, true}},
    {0x8004E7F0u, {0x50000040u, 0x0305u, 0x0100u, 0x0014u, 0x0064u, 0x0100u, 0x01ECu, true}},
    {0x8004E800u, {0x50000040u, 0x030Au, 0x0100u, 0x0014u, 0x0064u, 0x0100u, 0x01EDu, true}},
    {0x8004E810u, {0x50000040u, 0x030Fu, 0x0100u, 0x0014u, 0x0064u, 0x0100u, 0x01EEu, true}},
    {0x8004E820u, {0x50000040u, 0x0314u, 0x0100u, 0x0078u, 0x0014u, 0x0100u, 0x01EFu, true}},
    {0x8004E830u, {0x50000040u, 0x0314u, 0x0114u, 0x0078u, 0x0014u, 0x0100u, 0x01F0u, true}},
    {0x8004E840u, {0x50000040u, 0x0314u, 0x0128u, 0x0078u, 0x0014u, 0x0100u, 0x01F1u, true}},
    {0x8004E850u, {0x50000040u, 0x0314u, 0x013Cu, 0x0078u, 0x0014u, 0x0100u, 0x01F2u, true}},
    {0x8004E860u, {0x10000040u, 0x0380u, 0x0163u, 0x0014u, 0x0014u, 0x0110u, 0x01E4u, true}},
    {0x8004E870u, {0x10000040u, 0x0385u, 0x0163u, 0x0014u, 0x0014u, 0x0110u, 0x01E5u, true}},
    {0x8004E880u, {0x10000040u, 0x038Au, 0x0163u, 0x0014u, 0x0014u, 0x0110u, 0x01E6u, true}},
    {0x8004E890u, {0x10000040u, 0x038Fu, 0x0163u, 0x0014u, 0x0014u, 0x0110u, 0x01E7u, true}},
    {0x8004E8A0u, {0x10000040u, 0x03A8u, 0x014Bu, 0x0014u, 0x00A0u, 0x0110u, 0x01E8u, true}},
    {0x8004E8B0u, {0x10000040u, 0x03ADu, 0x014Bu, 0x0014u, 0x00A0u, 0x0110u, 0x01E9u, true}},
    {0x8004E8C0u, {0x10000040u, 0x0380u, 0x014Bu, 0x0078u, 0x000Cu, 0x0110u, 0x01EAu, true}},
    {0x8004E8D0u, {0x10000040u, 0x0380u, 0x0157u, 0x0078u, 0x000Cu, 0x0110u, 0x01EBu, true}},
    {0x8004E8E0u, {0x10000040u, 0x0380u, 0x0177u, 0x0078u, 0x0014u, 0x0110u, 0x01ECu, true}},
    {0x8004E8F0u, {0x10000040u, 0x0380u, 0x018Bu, 0x0078u, 0x0014u, 0x0110u, 0x01EDu, true}},
    {0x8004E900u, {0x10000040u, 0x0380u, 0x0163u, 0x0014u, 0x0014u, 0x0110u, 0x01EEu, true}},
    {0x8004E910u, {0x10000040u, 0x0385u, 0x0163u, 0x0014u, 0x0014u, 0x0110u, 0x01EFu, true}},
    {0x8004E920u, {0x10000040u, 0x038Au, 0x0163u, 0x0014u, 0x0014u, 0x0110u, 0x01F0u, true}},
    {0x8004E930u, {0x10000040u, 0x038Fu, 0x0163u, 0x0014u, 0x0014u, 0x0110u, 0x01F1u, true}},
    {0x8004E940u, {0x00000040u, 0x03B2u, 0x014Bu, 0x0014u, 0x0014u, 0x0110u, 0x01F2u, true}},
    {0x8004E950u, {0x00000040u, 0x03B2u, 0x015Fu, 0x0014u, 0x0014u, 0x0110u, 0x01F3u, true}},
    {0x8004E960u, {0x00000040u, 0x03B2u, 0x0173u, 0x0014u, 0x0014u, 0x0110u, 0x01F4u, true}},
    {0x8004E970u, {0x00000040u, 0x03B2u, 0x0187u, 0x0014u, 0x0014u, 0x0110u, 0x01F5u, true}},
    {0x8004E980u, {0x10000040u, 0x039Eu, 0x014Bu, 0x0014u, 0x00A0u, 0x0110u, 0x01E0u, true}},
    {0x8004E990u, {0x10000040u, 0x03B2u, 0x019Bu, 0x0014u, 0x0014u, 0x0110u, 0x01E1u, true}},
    {0x8004E9A0u, {0x10000040u, 0x03B2u, 0x01AFu, 0x0014u, 0x0014u, 0x0110u, 0x01E2u, true}},
    {0x8004E9B0u, {0x10000040u, 0x03B2u, 0x01C3u, 0x0014u, 0x0014u, 0x0110u, 0x01E1u, true}},
    {0x8004E9C0u, {0x10000040u, 0x03B2u, 0x01D7u, 0x0014u, 0x0014u, 0x0110u, 0x01E2u, true}},
    {0x8004E9D0u, {0x10000040u, 0x03B7u, 0x014Bu, 0x0014u, 0x0014u, 0x0110u, 0x01E1u, true}},
    {0x8004E9E0u, {0x10000040u, 0x03B7u, 0x015Fu, 0x0014u, 0x0014u, 0x0110u, 0x01E2u, true}},
    {0x8004E9F0u, {0x10000040u, 0x03B7u, 0x0173u, 0x0014u, 0x0014u, 0x0110u, 0x01E1u, true}},
    {0x8004EA00u, {0x10000040u, 0x03B7u, 0x0187u, 0x0014u, 0x0014u, 0x0110u, 0x01E2u, true}},
    {0x8004EA10u, {0x10000040u, 0x03B7u, 0x019Bu, 0x0014u, 0x0014u, 0x0110u, 0x01E1u, true}},
    {0x8004EA20u, {0x10000040u, 0x03B7u, 0x01AFu, 0x0014u, 0x0014u, 0x0110u, 0x01E2u, true}},
    {0x8004EA30u, {0x10000040u, 0x03B7u, 0x01C3u, 0x0014u, 0x0014u, 0x0110u, 0x01E1u, true}},
    {0x8004EA40u, {0x10000040u, 0x03B7u, 0x01D7u, 0x0014u, 0x0014u, 0x0110u, 0x01E2u, true}},
    {0x8004EA50u, {0x10000040u, 0x03A3u, 0x014Bu, 0x0014u, 0x00A0u, 0x0110u, 0x01E3u, true}},
    {0x8004EA60u, {0x10000040u, 0x0394u, 0x0163u, 0x0014u, 0x0014u, 0x0110u, 0x01F0u, true}},
    {0x8004EA70u, {0x10000040u, 0x0399u, 0x0163u, 0x0014u, 0x0014u, 0x0110u, 0x01F1u, true}},
    {0x8004EA80u, {0x10000040u, 0x0394u, 0x019Fu, 0x0014u, 0x0014u, 0x0110u, 0x01F0u, true}},
    {0x8004EA90u, {0x10000040u, 0x0399u, 0x019Fu, 0x0014u, 0x0014u, 0x0110u, 0x01F1u, true}},
    {0x8004EAA0u, {0x50000040u, 0x0394u, 0x01B3u, 0x0014u, 0x0014u, 0x0110u, 0x01F0u, true}},
    {0x8004EAB0u, {0x50000040u, 0x0399u, 0x01B3u, 0x0014u, 0x0014u, 0x0110u, 0x01F1u, true}},
    {0x8004EAC0u, {0x50000040u, 0x0394u, 0x01C7u, 0x0014u, 0x0014u, 0x0110u, 0x01F0u, true}},
    {0x8004EAD0u, {0x50000040u, 0x0399u, 0x01C7u, 0x0014u, 0x0014u, 0x0110u, 0x01F1u, true}},
    {0x8004EAE0u, {0x50000040u, 0x0394u, 0x01DBu, 0x0014u, 0x0014u, 0x0110u, 0x01F0u, true}},
    {0x8004EAF0u, {0x50000040u, 0x0399u, 0x01DBu, 0x0014u, 0x0014u, 0x0110u, 0x01F1u, true}},
    {0x8004EB00u, {0x50000040u, 0x039Eu, 0x01EBu, 0x0014u, 0x0014u, 0x0110u, 0x01F0u, true}},
    {0x8004EB10u, {0x50000040u, 0x03A3u, 0x01EBu, 0x0014u, 0x0014u, 0x0110u, 0x01F1u, true}},
    {0x80050380u, {0x10000040u, 0x03A8u, 0x014Bu, 0x0014u, 0x00A0u, 0x0110u, 0x01E8u, true}},
    {0x80050390u, {0x10000040u, 0x03ADu, 0x014Bu, 0x0014u, 0x00A0u, 0x0110u, 0x01E9u, true}},
    {0x800503A0u, {0x10000040u, 0x0380u, 0x014Bu, 0x0078u, 0x000Cu, 0x0110u, 0x01EAu, true}},
    {0x800503B0u, {0x10000040u, 0x0380u, 0x0157u, 0x0078u, 0x000Cu, 0x0110u, 0x01EBu, true}},
    {0x800503C0u, {0x10000040u, 0x0380u, 0x0177u, 0x0078u, 0x0014u, 0x0110u, 0x01ECu, true}},
    {0x800503D0u, {0x10000040u, 0x0380u, 0x018Bu, 0x0078u, 0x0014u, 0x0110u, 0x01EDu, true}},
    {0x800503E0u, {0x10000040u, 0x0380u, 0x0163u, 0x0014u, 0x0014u, 0x0110u, 0x01EEu, true}},
    {0x800503F0u, {0x10000040u, 0x0385u, 0x0163u, 0x0014u, 0x0014u, 0x0110u, 0x01EFu, true}},
    {0x80050400u, {0x10000040u, 0x038Au, 0x0163u, 0x0014u, 0x0014u, 0x0110u, 0x01F0u, true}},
    {0x80050410u, {0x10000040u, 0x038Fu, 0x0163u, 0x0014u, 0x0014u, 0x0110u, 0x01F1u, true}},
    {0x800509A0u, {0x50000040u, 0x0140u, 0x0000u, 0x0030u, 0x0011u, 0x03C0u, 0x0002u, true}},
    {0x800509D0u, {0x50000040u, 0x0140u, 0x0100u, 0x001Cu, 0x000Bu, 0x03C0u, 0x0005u, true}},
    {0x80050A00u, {0x50000040u, 0x0147u, 0x0100u, 0x0020u, 0x0008u, 0x03C0u, 0x0005u, true}},
    {0x80050A30u, {0x50000040u, 0x014Fu, 0x0100u, 0x0020u, 0x0008u, 0x03C0u, 0x0005u, true}},
    {0x80050A60u, {0x50000040u, 0x0157u, 0x0100u, 0x001Cu, 0x000Cu, 0x03C0u, 0x0005u, true}},
    {0x80050A90u, {0x50000040u, 0x015Eu, 0x0100u, 0x0020u, 0x000Au, 0x03C0u, 0x0005u, true}},
    {0x80050EF0u, {0x50000040u, 0x014Cu, 0x0000u, 0x0044u, 0x002Du, 0x03C0u, 0x0008u, true}},
    {0x80050F20u, {0x50000040u, 0x0140u, 0x010Bu, 0x003Cu, 0x000Cu, 0x03C0u, 0x000Bu, true}},
    {0x80051010u, {0x50000040u, 0x0190u, 0x0000u, 0x0058u, 0x0044u, 0x03C0u, 0x0000u, true}},
    {0x80051020u, {0x50000040u, 0x0190u, 0x0000u, 0x0058u, 0x0044u, 0x03C0u, 0x0001u, true}},
    {0x80051030u, {0x50000040u, 0x015Du, 0x0000u, 0x0054u, 0x002Au, 0x03C0u, 0x000Eu, true}},
    {0x80051040u, {0x50000040u, 0x015Du, 0x0000u, 0x0054u, 0x002Au, 0x03C0u, 0x000Fu, true}},
    {0x80051050u, {0x50000040u, 0x015Du, 0x0000u, 0x0054u, 0x002Au, 0x03C0u, 0x0010u, true}},
    {0x80051060u, {0x50000040u, 0x0140u, 0x0123u, 0x003Cu, 0x000Bu, 0x03C0u, 0x0011u, true}},
    {0x80051070u, {0x50000040u, 0x0140u, 0x0123u, 0x003Cu, 0x000Bu, 0x03C0u, 0x0012u, true}},
    {0x80051080u, {0x50000040u, 0x0140u, 0x0123u, 0x003Cu, 0x000Bu, 0x03C0u, 0x0013u, true}},
    {0x80051090u, {0x50000040u, 0x0161u, 0x01C6u, 0x0040u, 0x000Au, 0x03C0u, 0x0011u, true}},
    {0x800510A0u, {0x50000040u, 0x0161u, 0x01C6u, 0x0040u, 0x000Au, 0x03C0u, 0x0012u, true}},
    {0x800510B0u, {0x50000040u, 0x0161u, 0x01C6u, 0x0040u, 0x000Au, 0x03C0u, 0x0013u, true}},
    {0x800510C0u, {0x50000040u, 0x015Eu, 0x0123u, 0x003Cu, 0x000Bu, 0x03C0u, 0x0011u, true}},
    {0x800510D0u, {0x50000040u, 0x015Eu, 0x0123u, 0x003Cu, 0x000Bu, 0x03C0u, 0x0012u, true}},
    {0x800510E0u, {0x50000040u, 0x015Eu, 0x0123u, 0x003Cu, 0x000Bu, 0x03C0u, 0x0013u, true}},
    {0x800510F0u, {0x50000040u, 0x016Du, 0x0123u, 0x0038u, 0x000Cu, 0x03C0u, 0x0011u, true}},
    {0x80051100u, {0x50000040u, 0x016Du, 0x0123u, 0x0038u, 0x000Cu, 0x03C0u, 0x0012u, true}},
    {0x80051110u, {0x50000040u, 0x016Du, 0x0123u, 0x0038u, 0x000Cu, 0x03C0u, 0x0013u, true}},
    {0x80051120u, {0x50000040u, 0x0171u, 0x01C5u, 0x0038u, 0x001Au, 0x03C0u, 0x0011u, true}},
    {0x80051150u, {0x50000040u, 0x01A6u, 0x0000u, 0x0064u, 0x003Du, 0x03C0u, 0x0000u, true}},
    {0x80051160u, {0x50000040u, 0x01A6u, 0x0000u, 0x0064u, 0x003Du, 0x03C0u, 0x0001u, true}},
    {0x80051170u, {0x50000040u, 0x0140u, 0x002Du, 0x0044u, 0x0016u, 0x03C0u, 0x0014u, true}},
    {0x800511A0u, {0x50000040u, 0x0140u, 0x0139u, 0x0034u, 0x000Du, 0x03C0u, 0x0017u, true}},
    {0x800511D0u, {0x50000040u, 0x014Du, 0x0139u, 0x0034u, 0x000Du, 0x03C0u, 0x0017u, true}},
    {0x80051200u, {0x50000040u, 0x015Au, 0x0139u, 0x0034u, 0x000Du, 0x03C0u, 0x0017u, true}},
    {0x80051230u, {0x50000040u, 0x0167u, 0x0139u, 0x0034u, 0x000Du, 0x03C0u, 0x0017u, true}},
    {0x80051260u, {0x50000040u, 0x0140u, 0x0146u, 0x0034u, 0x000Du, 0x03C0u, 0x0017u, true}},
    {0x80051290u, {0x50000040u, 0x0180u, 0x0044u, 0x0058u, 0x0043u, 0x03C0u, 0x0000u, true}},
    {0x800512A0u, {0x50000040u, 0x0180u, 0x0044u, 0x0058u, 0x0043u, 0x03C0u, 0x0001u, true}},
    {0x800512B0u, {0x50000040u, 0x0140u, 0x0043u, 0x0044u, 0x0016u, 0x03C0u, 0x001Au, true}},
    {0x800512E0u, {0x50000040u, 0x0140u, 0x0153u, 0x0030u, 0x000Cu, 0x03C0u, 0x001Du, true}},
    {0x80051310u, {0x50000040u, 0x014Cu, 0x0153u, 0x0030u, 0x000Cu, 0x03C0u, 0x001Du, true}},
    {0x80051340u, {0x50000040u, 0x0158u, 0x0153u, 0x0030u, 0x000Cu, 0x03C0u, 0x001Du, true}},
    {0x80051370u, {0x50000040u, 0x0164u, 0x0153u, 0x0030u, 0x000Cu, 0x03C0u, 0x001Du, true}},
    {0x800513A0u, {0x50000040u, 0x0170u, 0x0153u, 0x0030u, 0x000Fu, 0x03C0u, 0x001Du, true}},
    {0x800513D0u, {0x50000040u, 0x0151u, 0x002Du, 0x0048u, 0x0037u, 0x03C0u, 0x0020u, true}},
    {0x80051400u, {0x50000040u, 0x0140u, 0x015Fu, 0x002Cu, 0x0019u, 0x03C0u, 0x0023u, true}},
    {0x80051430u, {0x50000040u, 0x014Bu, 0x015Fu, 0x002Cu, 0x0022u, 0x03C0u, 0x0023u, true}},
    {0x80051460u, {0x50000040u, 0x0156u, 0x015Fu, 0x0028u, 0x001Au, 0x03C0u, 0x0023u, true}},
    {0x80051490u, {0x50000040u, 0x0160u, 0x015Fu, 0x0028u, 0x001Au, 0x03C0u, 0x0023u, true}},
    {0x800514C0u, {0x50000040u, 0x0170u, 0x0162u, 0x002Cu, 0x001Au, 0x03C0u, 0x0023u, true}},
    {0x800514F0u, {0x50000040u, 0x0196u, 0x0044u, 0x0044u, 0x0049u, 0x03C0u, 0x0000u, true}},
    {0x80051500u, {0x50000040u, 0x0196u, 0x0044u, 0x0044u, 0x0049u, 0x03C0u, 0x0001u, true}},
    {0x80051510u, {0x50000040u, 0x0163u, 0x002Du, 0x0040u, 0x002Bu, 0x03C0u, 0x0026u, true}},
    {0x80051540u, {0x50000040u, 0x0140u, 0x0178u, 0x002Cu, 0x001Cu, 0x03C0u, 0x0029u, true}},
    {0x80051570u, {0x50000040u, 0x014Bu, 0x0181u, 0x002Cu, 0x001Bu, 0x03C0u, 0x0029u, true}},
    {0x800515A0u, {0x50000040u, 0x0156u, 0x0179u, 0x002Cu, 0x001Cu, 0x03C0u, 0x0029u, true}},
    {0x800515D0u, {0x50000040u, 0x0161u, 0x0179u, 0x002Cu, 0x001Cu, 0x03C0u, 0x0029u, true}},
    {0x80051600u, {0x50000040u, 0x0170u, 0x017Cu, 0x002Cu, 0x001Au, 0x03C0u, 0x0029u, true}},
    {0x80051630u, {0x50000040u, 0x01A7u, 0x0044u, 0x0044u, 0x0042u, 0x03C0u, 0x0000u, true}},
    {0x80051640u, {0x50000040u, 0x01A7u, 0x0044u, 0x0044u, 0x0042u, 0x03C0u, 0x0001u, true}},
    {0x80051650u, {0x50000040u, 0x0140u, 0x0064u, 0x0044u, 0x002Bu, 0x03C0u, 0x002Cu, true}},
    {0x80051680u, {0x50000040u, 0x0140u, 0x0194u, 0x002Cu, 0x000Du, 0x03C0u, 0x002Fu, true}},
    {0x800516B0u, {0x50000040u, 0x014Bu, 0x019Cu, 0x0030u, 0x0018u, 0x03C0u, 0x002Fu, true}},
    {0x800516E0u, {0x50000040u, 0x0157u, 0x0195u, 0x0030u, 0x001Bu, 0x03C0u, 0x002Fu, true}},
    {0x80051710u, {0x50000040u, 0x0163u, 0x0195u, 0x002Cu, 0x000Eu, 0x03C0u, 0x002Fu, true}},
    {0x80051740u, {0x50000040u, 0x0170u, 0x0196u, 0x0028u, 0x0018u, 0x03C0u, 0x002Fu, true}},
    {0x80051770u, {0x50000040u, 0x0180u, 0x008Du, 0x0040u, 0x0042u, 0x03C0u, 0x0000u, true}},
    {0x80051780u, {0x50000040u, 0x0180u, 0x008Du, 0x0040u, 0x0042u, 0x03C0u, 0x0001u, true}},
    {0x80051790u, {0x50000040u, 0x0151u, 0x0064u, 0x0048u, 0x0037u, 0x03C0u, 0x0032u, true}},
    {0x800517C0u, {0x50000040u, 0x0140u, 0x01A1u, 0x0024u, 0x0013u, 0x03C0u, 0x0035u, true}},
    {0x800517F0u, {0x50000040u, 0x014Bu, 0x01B4u, 0x0028u, 0x0017u, 0x03C0u, 0x0035u, true}},
    {0x80051820u, {0x50000040u, 0x0157u, 0x01B0u, 0x0028u, 0x0016u, 0x03C0u, 0x0035u, true}},
    {0x80051850u, {0x50000040u, 0x0163u, 0x01A3u, 0x0028u, 0x0015u, 0x03C0u, 0x0035u, true}},
    {0x80051880u, {0x50000040u, 0x0170u, 0x01AEu, 0x0028u, 0x0017u, 0x03C0u, 0x0035u, true}},
    {0x800518B0u, {0x50000040u, 0x0190u, 0x008Du, 0x0048u, 0x0048u, 0x03C0u, 0x0000u, true}},
    {0x800518C0u, {0x50000040u, 0x0190u, 0x008Du, 0x0048u, 0x0048u, 0x03C0u, 0x0001u, true}},
    {0x800508D0u, {0x40000040u, 0x03FDu, 0x0199u, 0x0008u, 0x0008u, 0x0120u, 0x01EEu, true}},
    {0x800508E0u, {0x40000040u, 0x03FDu, 0x01A1u, 0x0008u, 0x0008u, 0x0120u, 0x01EEu, true}},
    {0x800508F0u, {0x40000040u, 0x03FDu, 0x0189u, 0x0008u, 0x0008u, 0x0120u, 0x01EEu, true}},
    {0x80050900u, {0x40000040u, 0x03FDu, 0x0191u, 0x0008u, 0x0008u, 0x0120u, 0x01EEu, true}},
    {0x80052320u, {0x50000040u, 0x02C0u, 0x0088u, 0x00ACu, 0x0054u, 0x03F0u, 0x00D6u, true}},
    {0x80052350u, {0x51000040u, 0x0340u, 0x0000u, 0x0060u, 0x0086u, 0x0300u, 0x00FFu, true}},
    {0x800523A0u, {0x50000040u, 0x0221u, 0x01B7u, 0x0028u, 0x000Eu, 0x03F0u, 0x0032u, true}},
    {0x80052410u, {0x50000040u, 0x0225u, 0x0146u, 0x005Cu, 0x0012u, 0x03F0u, 0x0032u, true}},
    {0x80052480u, {0x50000040u, 0x0225u, 0x0175u, 0x0048u, 0x0012u, 0x03F0u, 0x0032u, true}},
    {0x800524F0u, {0x50000040u, 0x0225u, 0x0158u, 0x005Cu, 0x000Du, 0x03F0u, 0x0032u, true}},
    {0x80052560u, {0x50000040u, 0x0225u, 0x0165u, 0x004Cu, 0x0010u, 0x03F0u, 0x0032u, true}},
    {0x80052590u, {0x50000040u, 0x02EEu, 0x003Fu, 0x0030u, 0x0015u, 0x03F0u, 0x00C9u, true}},
    {0x800525A0u, {0x50000040u, 0x02EEu, 0x003Fu, 0x0030u, 0x0015u, 0x03F0u, 0x00CAu, true}},
    {0x800526A0u, {0x50000040u, 0x02EEu, 0x0000u, 0x0044u, 0x003Fu, 0x03F0u, 0x00CEu, true}},
    {0x800526B0u, {0x50000040u, 0x02EEu, 0x0000u, 0x0044u, 0x003Fu, 0x03F0u, 0x00CFu, true}},
    {0x800526C0u, {0x50000040u, 0x02EEu, 0x0054u, 0x0030u, 0x0015u, 0x03F0u, 0x00D1u, true}},
    {0x800526D0u, {0x50000040u, 0x02EEu, 0x0054u, 0x0030u, 0x0015u, 0x03F0u, 0x00D2u, true}},
    {0x800526F0u, {0x50000040u, 0x0370u, 0x0000u, 0x0014u, 0x000Eu, 0x03F0u, 0x00D4u, true}},
    {0x80052700u, {0x50000040u, 0x0370u, 0x0000u, 0x0014u, 0x000Eu, 0x03F0u, 0x00D5u, true}},
    {0x80052740u, {0x50000040u, 0x0370u, 0x000Eu, 0x0020u, 0x000Eu, 0x03F0u, 0x00D3u, true}},
    {0x80052750u, {0x50000040u, 0x0370u, 0x000Eu, 0x0020u, 0x000Eu, 0x03F0u, 0x00D4u, true}},
};

struct Movie1PsxStaticPositionPairEntry {
    uint32_t addr = 0;
    int16_t x = 0;
    int16_t y = 0;
};

static constexpr Movie1PsxStaticPositionPairEntry kMovie1PsxC5A8PositionPairs[] = {
    {0x80052FFCu, 238, 188},
    {0x8005300Cu, 242, 191},
    {0x8005301Cu, 238, 193},
    {0x8005302Cu, 238, 193},
    {0x8005303Cu, 241, 190},
    {0x8005304Cu, 239, 191},
    {0x80053560u,  41,  64},
    {0x800535BCu, 124,  81},
    {0x800535CCu, 123,  81},
    {0x800535DCu, 121,  82},
    {0x800535ECu, 126,  80},
    {0x800535FCu, 128,  71},
    {0x8005362Cu, 216,  47},
    {0x8005363Cu, 216,  71},
    {0x8005364Cu, 218,  52},
    {0x8005365Cu, 218,  52},
    {0x8005366Cu, 218,  52},
    {0x8005367Cu, 218,  52},
    {0x8005368Cu, 218,  52},
    {0x8005369Cu, 218,  76},
    {0x800536ACu, 220,  76},
    {0x800536BCu, 218,  76},
    {0x800536CCu, 218,  76},
    {0x800536DCu, 219,  73},
    {0x8005371Cu,  41, 127},
    {0x8005372Cu,  42, 126},
    {0x8005373Cu,  43, 123},
    {0x8005374Cu,  42, 127},
    {0x8005375Cu,  40, 127},
    {0x8005376Cu,  99, 144},
    {0x8005377Cu, 100, 144},
    {0x8005378Cu, 101, 145},
    {0x8005379Cu, 101, 145},
    {0x800537ACu, 101, 145},
    {0x800537BCu, 161, 152},
    {0x800537CCu, 161, 146},
    {0x800537DCu, 162, 149},
    {0x800537ECu, 162, 152},
    {0x800537FCu, 163, 148},
    {0x8005380Cu, 226, 136},
    {0x8005381Cu, 224, 134},
    {0x8005382Cu, 225, 132},
    {0x8005383Cu, 224, 134},
    {0x8005384Cu, 225, 132},
    {0x80053A34u, 185,  71},
    {0x80053A3Cu, 170,  69},
    {0x80053A44u, 160,  69},
    {0x80053A4Cu, 162,  71},
    {0x80053A54u, 167,  71},
    {0x80053AB4u, 237, 162},
    {0x80053AC4u, 242, 163},
    {0x80053AD4u, 237, 163},
    {0x80053AE4u, 241, 160},
    {0x80053AF4u, 241, 160},
    {0x80053B04u, 241, 185},
    {0x80053B14u, 234, 185},
    {0x80053B24u, 236, 185},
    {0x80053B34u, 241, 185},
    {0x80053B44u, 241, 185},
};

struct Movie1PsxStaticPointerEntry {
    uint32_t addr = 0;
    uint32_t value = 0;
};

static constexpr Movie1PsxStaticPointerEntry kMovie1PsxC5A8TemplatePtrs[] = {
    {0x80053000u, 0x800509D0u},
    {0x80053010u, 0x80050A30u},
    {0x80053020u, 0x80050A00u},
    {0x80053030u, 0x80050A60u},
    {0x80053040u, 0x80050A90u},
    {0x800535B0u, 0x80051060u},
    {0x800535C0u, 0x800510C0u},
    {0x800535D0u, 0x80051090u},
    {0x800535E0u, 0x800510F0u},
    {0x800535F0u, 0x80051120u},
    {0x80053640u, 0x800511A0u},
    {0x80053650u, 0x80051200u},
    {0x80053660u, 0x800511D0u},
    {0x80053670u, 0x80051230u},
    {0x80053680u, 0x80051260u},
    {0x80053690u, 0x800512E0u},
    {0x800536A0u, 0x80051340u},
    {0x800536B0u, 0x80051310u},
    {0x800536C0u, 0x80051370u},
    {0x800536D0u, 0x800513A0u},
    {0x80053710u, 0x80051400u},
    {0x80053720u, 0x80051460u},
    {0x80053730u, 0x80051430u},
    {0x80053740u, 0x80051490u},
    {0x80053750u, 0x800514C0u},
    {0x80053760u, 0x80051540u},
    {0x80053770u, 0x800515A0u},
    {0x80053780u, 0x80051570u},
    {0x80053790u, 0x800515D0u},
    {0x800537A0u, 0x80051600u},
    {0x800537B0u, 0x80051680u},
    {0x800537C0u, 0x800516E0u},
    {0x800537D0u, 0x800516B0u},
    {0x800537E0u, 0x80051710u},
    {0x800537F0u, 0x80051740u},
    {0x80053800u, 0x800517C0u},
    {0x80053810u, 0x80051820u},
    {0x80053820u, 0x800517F0u},
    {0x80053830u, 0x80051850u},
    {0x80053840u, 0x80051880u},
    {0x80053A30u, 0x800523A0u},
    {0x80053A38u, 0x80052480u},
    {0x80053A40u, 0x80052410u},
    {0x80053A48u, 0x800524F0u},
    {0x80053A50u, 0x80052560u},
    {0x80053AACu, 0x800525C0u},
    {0x80053AB0u, 0x800525D0u},
    {0x80053ABCu, 0x80052620u},
    {0x80053AC0u, 0x80052630u},
    {0x80053ACCu, 0x800525F0u},
    {0x80053AD0u, 0x80052600u},
    {0x80053ADCu, 0x80052650u},
    {0x80053AE0u, 0x80052660u},
    {0x80053AECu, 0x80052680u},
    {0x80053AF0u, 0x80052690u},
    {0x80053AFCu, 0x800526F0u},
    {0x80053B00u, 0x80052700u},
    {0x80053B1Cu, 0x80052720u},
    {0x80053B20u, 0x80052730u},
    {0x80053B0Cu, 0x80052740u},
    {0x80053B10u, 0x80052750u},
    {0x80053B2Cu, 0x80052780u},
    {0x80053B30u, 0x80052790u},
    {0x80053B3Cu, 0x800527B0u},
    {0x80053B40u, 0x800527C0u},
};

static constexpr Movie1PsxStaticPointerEntry kMovie1PsxGpTemplatePtrs[] = {
    {0x8006EB1Cu, 0x800509C0u},
    {0x8006EB20u, 0x80051010u},
    {0x8006EB24u, 0x80051020u},
    {0x8006EB28u, 0x80051150u},
    {0x8006EB2Cu, 0x80051160u},
    {0x8006EB30u, 0x80051290u},
    {0x8006EB34u, 0x800512A0u},
    {0x8006EB38u, 0x800514F0u},
    {0x8006EB3Cu, 0x80051500u},
    {0x8006EB40u, 0x80051630u},
    {0x8006EB44u, 0x80051640u},
    {0x8006EB48u, 0x80051770u},
    {0x8006EB4Cu, 0x80051780u},
    {0x8006EB50u, 0x800518B0u},
    {0x8006EB54u, 0x800518C0u},
};

static bool ResolveMovie1PsxStaticPositionPair(uint32_t addr,
                                               int16_t& x,
                                               int16_t& y) {
    for (const Movie1PsxStaticPositionPairEntry& entry :
         kMovie1PsxC5A8PositionPairs) {
        if (entry.addr == addr) {
            x = entry.x;
            y = entry.y;
            return true;
        }
    }
    return false;
}

static bool ResolveMovie1PsxStaticTemplatePointer(uint32_t addr,
                                                  uint32_t& value) {
    for (const Movie1PsxStaticPointerEntry& entry :
         kMovie1PsxC5A8TemplatePtrs) {
        if (entry.addr == addr) {
            value = entry.value;
            return true;
        }
    }
    return false;
}

static bool ResolveMovie1PsxGpTemplatePointer(uint32_t gpOffset,
                                              uint32_t& value) {
    const uint32_t addr = kMovie1PsxGpBase8006EA40 + gpOffset;
    for (const Movie1PsxStaticPointerEntry& entry : kMovie1PsxGpTemplatePtrs) {
        if (entry.addr == addr) {
            value = entry.value;
            return true;
        }
    }
    return false;
}

static PrPsxFastSpriteSubmitDirect::FastSpriteSubmitSourceKind8003FA20
ResolveRawDrawFastSpriteSourceKind8003FA20(
    const Movie1PsxRawDrawAction* action) {
    if (action == nullptr) {
        return PrPsxFastSpriteSubmitDirect::
            FastSpriteSubmitSourceKind8003FA20::Movie1RawDraw;
    }

    switch (action->sourceFunction) {
    case 0x8001B744u:
    case 0x8001BCA0u:
        return PrPsxFastSpriteSubmitDirect::
            FastSpriteSubmitSourceKind8003FA20::Stage1MovieText;
    case 0x8001B954u:
        return PrPsxFastSpriteSubmitDirect::
            FastSpriteSubmitSourceKind8003FA20::Stage1MovieText;
    case 0x8001DB9Cu:
    case 0x8001DF24u:
        return PrPsxFastSpriteSubmitDirect::
            FastSpriteSubmitSourceKind8003FA20::Stage1Hud;
    default:
        return PrPsxFastSpriteSubmitDirect::
            FastSpriteSubmitSourceKind8003FA20::Movie1RawDraw;
    }
}

static void AttachRawDrawB25CMetadata(Movie1PsxRawDrawAction& action);

static bool ResolveMovie1PsxGpTemplateAction(Movie1PsxRawDrawAction& action) {
    if (action.templateGpOffset == 0u) {
        return action.templateAddr != 0u;
    }
    uint32_t resolvedTemplateAddr = 0;
    if (!ResolveMovie1PsxGpTemplatePointer(
            action.templateGpOffset,
            resolvedTemplateAddr)) {
        return false;
    }
    action.templateGpBaseAddr = kMovie1PsxGpBase8006EA40;
    action.templateAddr = resolvedTemplateAddr;
    AttachRawDrawB25CMetadata(action);
    return true;
}

static bool IsResolvedTransitionTileTemplateAction(
    const Movie1PsxRawDrawAction& action) {
    return action.helper == Movie1PsxRawDrawHelper::Call8001C550 &&
           action.templatePtrTableAddr == 0x80050720u &&
           action.templateLanguageStride == 4u &&
           action.positionLanguageStride == 0u &&
           action.templateAddr != 0u;
}

static uint16_t ComputeSub80043DF4ClutWordFromTemplateWords(uint16_t word4,
                                                            uint16_t word6,
                                                            uint16_t width,
                                                            uint16_t glyphIndex) {
    const uint16_t v8 = (uint16_t)(4u * word4 + glyphIndex * width);
    const uint16_t texHelperA3 = (uint16_t)((v8 & 0xFF00u) >> 2);
    const uint16_t texHelperA4 = (uint16_t)(word6 & 0xFF00u);
    // 8001B25C calls 80043DF4(0, 1, a3, a4); this is a tpage word,
    // not a direct OR of the helper arguments.
    return (uint16_t)(
        0x20u |
        ((texHelperA4 & 0x0100u) >> 4) |
        ((texHelperA3 & 0x03FFu) >> 6) |
        (4u * (texHelperA4 & 0x0200u)));
}

static Movie1PsxRawDrawB25CMetadata BuildRawDrawB25CMetadata(
    uint32_t tplAddr,
    const Movie1PsxTemplateDesc& desc,
    uint16_t glyphIndex,
    bool addGlyphIndexToFinalWord) {
    return BuildRawDrawTileRectFieldsSub8001B25C(
        tplAddr,
        desc,
        glyphIndex,
        addGlyphIndexToFinalWord);
}

static void AttachRawDrawC5A8Metadata(Movie1PsxRawDrawAction& action) {
    if (action.helper != Movie1PsxRawDrawHelper::Call8001C5A8) {
        return;
    }
    action.c5a8.valid = true;
    action.c5a8.positionPairAddr = action.positionAddr;
    action.c5a8.workBaseAddr = action.workBaseAddr;
    action.c5a8.bufferIndex = action.bufferIndex;
    action.c5a8.ot = (uint16_t)action.ot;
    action.c5a8.workEntryAddr =
        action.workBaseAddr + action.bufferIndex * action.c5a8.workStride;
}

static void AttachRawDrawB25CMetadata(Movie1PsxRawDrawAction& action) {
    if (action.templateAddr == 0u) {
        return;
    }
    const Movie1PsxTemplateDesc desc =
        ResolveMovie1PsxTemplateDesc(action.templateAddr);
    action.b25c = BuildRawDrawB25CMetadata(action.templateAddr, desc, 0u, false);
}

static void AttachRawDrawB590Metadata(Movie1PsxRawDrawAction& action) {
    if (action.helper != Movie1PsxRawDrawHelper::Call8001B590) {
        return;
    }
    action.b590 =
        BuildRawDrawSubmitActionSub8001B590(
            action.sourceFunction,
            action.x,
            action.y,
            action.templateAddr,
            action.b25c.glyphIndex,
            action.b25c.addGlyphIndexToFinalWord,
            (uint16_t)action.ot,
            action.workBaseAddr + action.bufferIndex * 20u,
            action.bufferIndex,
            action.workBaseAddr)
            .b590;
}

static void AttachRawDrawBEE4Metadata(Movie1PsxRawDrawAction& action) {
    if (action.helper != Movie1PsxRawDrawHelper::Call8001BEE4) {
        return;
    }
    PrPsxFastSpriteSubmitDirect::FastSpriteStackScratchTail8003FA20
        stackTail{};
    if (action.bee4.stackResidue.valid) {
        stackTail = action.bee4.stackResidue.stackTail;
    }
    const int32_t enabled = action.bee4.valid
        ? action.bee4.enabled
        : action.be34.enabled;
    action.bee4 =
        BuildRawDrawSubmitActionSub8001BEE4(
            action.sourceFunction,
            action.x,
            action.y,
            action.templateAddr,
            enabled,
            action.workBaseAddr + action.bufferIndex * 20u,
            action.bufferIndex,
            action.workBaseAddr,
            stackTail)
            .bee4;
}

static bool IsFastSpriteRgbGapStatus(Movie1PsxRawDrawResolutionStatus status);
static void AttachFastSpriteRgbGapClassification(
    Movie1PsxRawDrawAction& action,
    Movie1PsxRawDrawB590Metadata& b590,
    Movie1PsxRawDrawResolutionStatus& status);
static void AttachFastSpriteRgbGapClassification(
    Movie1PsxRawDrawAction& action,
    Movie1PsxRawDrawBEE4Metadata& bee4,
    Movie1PsxRawDrawResolutionStatus& status);

static bool ResolveRawDrawC5A8ToB590ForTypedSubmit(
    const Movie1PsxRawDrawAction& sourceAction,
    uint8_t languageIndex,
    Movie1PsxRawDrawAction& outAction,
    Movie1PsxRawDrawB590Metadata& outB590,
    Movie1PsxRawDrawResolutionStatus& status) {
    Movie1PsxRawDrawAction action = sourceAction;
    AttachRawDrawC5A8Metadata(action);
    if (action.helper != Movie1PsxRawDrawHelper::Call8001C5A8) {
        outAction = action;
        status = Movie1PsxRawDrawResolutionStatus::GapUnknownHelper;
        return false;
    }

    action.languageIndex = languageIndex;
    action.languageIndexKnown = true;
    action.c5a8.languageIndex = languageIndex;
    action.c5a8.languageIndexKnown = true;

    const uint32_t resolvedPositionAddr =
        action.positionAddr +
        action.positionLanguageStride * (uint32_t)languageIndex;
    int16_t x = 0;
    int16_t y = 0;
    if (!ResolveMovie1PsxStaticPositionPair(resolvedPositionAddr, x, y)) {
        action.c5a8.resolvedPositionPairAddr = resolvedPositionAddr;
        outAction = action;
        status = Movie1PsxRawDrawResolutionStatus::GapPositionPair;
        return false;
    }
    action.x = x;
    action.y = y;
    action.positionAddr = resolvedPositionAddr;
    action.c5a8.resolvedPositionPairAddr = resolvedPositionAddr;
    action.c5a8.resolvedX = x;
    action.c5a8.resolvedY = y;
    action.c5a8.positionPairResolved = true;

    uint32_t resolvedTemplateAddr = action.templateAddr;
    uint32_t resolvedTemplatePtrAddr = 0;
    if (action.templatePtrTableAddr != 0u) {
        resolvedTemplatePtrAddr =
            action.templatePtrTableAddr +
            action.templateLanguageStride * (uint32_t)languageIndex;
        if (!ResolveMovie1PsxStaticTemplatePointer(
                resolvedTemplatePtrAddr,
                resolvedTemplateAddr)) {
            action.c5a8.templatePtrAddr = resolvedTemplatePtrAddr;
            outAction = action;
            status = Movie1PsxRawDrawResolutionStatus::GapTemplatePointer;
            return false;
        }
        action.c5a8.templatePtrAddr = resolvedTemplatePtrAddr;
        action.c5a8.templatePointerResolved = true;
    }
    action.templateAddr = resolvedTemplateAddr;
    action.c5a8.resolvedTemplateAddr = resolvedTemplateAddr;
    if (resolvedTemplateAddr == 0u) {
        outAction = action;
        status = Movie1PsxRawDrawResolutionStatus::GapTemplatePointer;
        return false;
    }

    const Movie1PsxTemplateDesc desc =
        ResolveMovie1PsxTemplateDesc(resolvedTemplateAddr);
    action.b25c =
        BuildRawDrawB25CMetadata(
            resolvedTemplateAddr,
            desc,
            0u,
            false);
    if (!desc.valid || !action.b25c.valid) {
        outAction = action;
        status = Movie1PsxRawDrawResolutionStatus::GapTemplateDesc;
        return false;
    }

    outAction =
        BuildRawDrawSubmitActionSub8001B590(
            action.sourceFunction,
            x,
            y,
            resolvedTemplateAddr,
            0u,
            false,
            (uint16_t)action.ot,
            action.workBaseAddr + action.bufferIndex * 20u,
            action.bufferIndex,
            action.workBaseAddr);
    outAction.sourceFunction = action.sourceFunction;
    outAction.condition = action.condition;
    outAction.positionAddr = resolvedPositionAddr;
    outAction.languageSourceAddr = action.languageSourceAddr;
    outAction.languageIndex = languageIndex;
    outAction.languageIndexKnown = true;
    outAction.c5a8 = action.c5a8;
    outB590 = outAction.b590;
    outB590.submitImplemented =
        outB590.gsSortFastSprite.localSpriteKnown ||
        (outB590.gsSortFastSprite.localSpritePrefixKnown &&
         PrPsxFastSpriteSubmitDirect::HasKnownRgbTail8003FA20(
             outB590.gsSortFastSprite.rawLocalBytes));
    outAction.b590 = outB590;
    status = outB590.submitImplemented
        ? Movie1PsxRawDrawResolutionStatus::ResolvedTypedSubmit
        : Movie1PsxRawDrawResolutionStatus::GapFastSpriteStackRgb;
    AttachFastSpriteRgbGapClassification(outAction, outB590, status);
    return true;
}

static bool IsRawDrawActionFullyResolvedForTemplateCommand(
    const Movie1PsxRawDrawAction& action,
    Movie1PsxRawDrawResolutionStatus& status) {
    if (action.condition == Movie1PsxRawDrawCondition::ActiveTileMaskCell) {
        status = Movie1PsxRawDrawResolutionStatus::GapActiveTileMask;
        return false;
    }
    if (action.condition != Movie1PsxRawDrawCondition::Always) {
        status = Movie1PsxRawDrawResolutionStatus::GapCondition;
        return false;
    }
    if ((action.templatePtrTableAddr != 0u ||
         action.templateLanguageStride != 0u ||
         action.positionLanguageStride != 0u) &&
        !IsResolvedTransitionTileTemplateAction(action)) {
        status = Movie1PsxRawDrawResolutionStatus::GapLanguageOrPointerTable;
        return false;
    }
    if (action.templateGpOffset != 0u) {
        Movie1PsxRawDrawAction resolvedAction = action;
        if (!ResolveMovie1PsxGpTemplateAction(resolvedAction)) {
            status = Movie1PsxRawDrawResolutionStatus::GapTemplatePointer;
            return false;
        }
        const Movie1PsxTemplateDesc desc =
            ResolveMovie1PsxTemplateDesc(resolvedAction.templateAddr);
        if (!desc.valid) {
            status = Movie1PsxRawDrawResolutionStatus::GapTemplateDesc;
            return false;
        }
        status = Movie1PsxRawDrawResolutionStatus::ResolvedTemplateCommand;
        return true;
    }
    if (action.templateAddr == 0u) {
        status = Movie1PsxRawDrawResolutionStatus::GapTemplatePointer;
        return false;
    }
    const Movie1PsxTemplateDesc desc =
        ResolveMovie1PsxTemplateDesc(action.templateAddr);
    if (!desc.valid) {
        status = Movie1PsxRawDrawResolutionStatus::GapTemplateDesc;
        return false;
    }
    switch (action.helper) {
        case Movie1PsxRawDrawHelper::Call8001C550:
            status = Movie1PsxRawDrawResolutionStatus::ResolvedTemplateCommand;
            return true;
        case Movie1PsxRawDrawHelper::Call8001B590:
            status = Movie1PsxRawDrawResolutionStatus::GapGsSortFastSpriteSubmit;
            return false;
        case Movie1PsxRawDrawHelper::Call8001BEE4:
            status = Movie1PsxRawDrawResolutionStatus::GapGsSortFastSpriteSubmit;
            return false;
        case Movie1PsxRawDrawHelper::Call8001C5A8:
            status = (action.positionAddr != 0u)
                ? Movie1PsxRawDrawResolutionStatus::GapSub8001B590Submit
                : Movie1PsxRawDrawResolutionStatus::GapTemplatePointer;
            return false;
        case Movie1PsxRawDrawHelper::None:
        default:
            status = Movie1PsxRawDrawResolutionStatus::GapUnknownHelper;
            return false;
    }
}

static bool TryBuildRawDrawB590ForTypedSubmit(
    const Movie1PsxRawDrawAction& sourceAction,
    Movie1PsxRawDrawAction& outAction,
    Movie1PsxRawDrawB590Metadata& outB590,
    Movie1PsxRawDrawResolutionStatus& status,
    uint8_t languageIndex) {
    Movie1PsxRawDrawAction action = sourceAction;
    AttachRawDrawC5A8Metadata(action);
    AttachRawDrawB25CMetadata(action);
    AttachRawDrawB590Metadata(action);

    if (action.condition == Movie1PsxRawDrawCondition::ActiveTileMaskCell) {
        status = Movie1PsxRawDrawResolutionStatus::GapActiveTileMask;
        outAction = action;
        return false;
    }
    if (action.condition != Movie1PsxRawDrawCondition::Always) {
        status = Movie1PsxRawDrawResolutionStatus::GapCondition;
        outAction = action;
        return false;
    }

    if (action.helper == Movie1PsxRawDrawHelper::Call8001C5A8) {
        return ResolveRawDrawC5A8ToB590ForTypedSubmit(
            action,
            languageIndex,
            outAction,
            outB590,
            status);
    }

    if (action.templatePtrTableAddr != 0u ||
        action.templateLanguageStride != 0u ||
        action.positionLanguageStride != 0u) {
        status = Movie1PsxRawDrawResolutionStatus::GapLanguageOrPointerTable;
        outAction = action;
        return false;
    }
    if (action.templateGpOffset != 0u) {
        if (!ResolveMovie1PsxGpTemplateAction(action)) {
            status = Movie1PsxRawDrawResolutionStatus::GapTemplatePointer;
            outAction = action;
            return false;
        }
    }
    if (action.templateAddr == 0u) {
        status = Movie1PsxRawDrawResolutionStatus::GapTemplatePointer;
        outAction = action;
        return false;
    }

    const Movie1PsxTemplateDesc desc =
        ResolveMovie1PsxTemplateDesc(action.templateAddr);
    if (!desc.valid || !action.b25c.valid) {
        status = Movie1PsxRawDrawResolutionStatus::GapTemplateDesc;
        outAction = action;
        return false;
    }

    if (action.b590.valid) {
        outB590 = action.b590;
        outB590.submitImplemented =
            outB590.gsSortFastSprite.localSpriteKnown ||
            (outB590.gsSortFastSprite.localSpritePrefixKnown &&
             PrPsxFastSpriteSubmitDirect::HasKnownRgbTail8003FA20(
                 outB590.gsSortFastSprite.rawLocalBytes));
        action.b590 = outB590;
        status = outB590.submitImplemented
            ? Movie1PsxRawDrawResolutionStatus::ResolvedTypedSubmit
            : Movie1PsxRawDrawResolutionStatus::GapFastSpriteStackRgb;
        AttachFastSpriteRgbGapClassification(action, outB590, status);
        outAction = action;
        return true;
    }

    status = action.helper == Movie1PsxRawDrawHelper::Call8001C550
        ? Movie1PsxRawDrawResolutionStatus::ResolvedTemplateCommand
        : Movie1PsxRawDrawResolutionStatus::GapUnknownHelper;
    outAction = action;
    return false;
}

static bool TryBuildRawDrawBEE4ForTypedSubmit(
    const Movie1PsxRawDrawAction& sourceAction,
    Movie1PsxRawDrawAction& outAction,
    Movie1PsxRawDrawBEE4Metadata& outBEE4,
    Movie1PsxRawDrawResolutionStatus& status) {
    Movie1PsxRawDrawAction action = sourceAction;
    if (action.helper != Movie1PsxRawDrawHelper::Call8001BEE4) {
        status = Movie1PsxRawDrawResolutionStatus::GapUnknownHelper;
        outAction = action;
        return false;
    }

    AttachRawDrawBEE4Metadata(action);

    if (action.condition == Movie1PsxRawDrawCondition::ActiveTileMaskCell) {
        status = Movie1PsxRawDrawResolutionStatus::GapActiveTileMask;
        outAction = action;
        return false;
    }
    if (action.condition != Movie1PsxRawDrawCondition::Always) {
        status = Movie1PsxRawDrawResolutionStatus::GapCondition;
        outAction = action;
        return false;
    }
    if (action.templatePtrTableAddr != 0u ||
        action.templateLanguageStride != 0u ||
        action.positionLanguageStride != 0u) {
        status = Movie1PsxRawDrawResolutionStatus::GapLanguageOrPointerTable;
        outAction = action;
        return false;
    }
    if (action.templateGpOffset != 0u) {
        if (!ResolveMovie1PsxGpTemplateAction(action)) {
            status = Movie1PsxRawDrawResolutionStatus::GapTemplatePointer;
            outAction = action;
            return false;
        }
        AttachRawDrawBEE4Metadata(action);
    }
    if (action.templateAddr == 0u) {
        status = Movie1PsxRawDrawResolutionStatus::GapTemplatePointer;
        outAction = action;
        return false;
    }

    const Movie1PsxTemplateDesc desc =
        ResolveMovie1PsxTemplateDesc(action.templateAddr);
    if (!desc.valid || !action.be34.valid || !action.bee4.valid) {
        status = Movie1PsxRawDrawResolutionStatus::GapTemplateDesc;
        outAction = action;
        return false;
    }

    outBEE4 = action.bee4;
    outBEE4.submitImplemented =
        outBEE4.gsSortFastSprite.localSpriteKnown ||
        (outBEE4.gsSortFastSprite.localSpritePrefixKnown &&
         PrPsxFastSpriteSubmitDirect::HasKnownRgbTail8003FA20(
             outBEE4.gsSortFastSprite.rawLocalBytes));
    action.bee4 = outBEE4;
    status = outBEE4.submitImplemented
        ? Movie1PsxRawDrawResolutionStatus::ResolvedTypedSubmit
        : Movie1PsxRawDrawResolutionStatus::GapFastSpriteStackRgb;
    AttachFastSpriteRgbGapClassification(action, outBEE4, status);
    outAction = action;
    return true;
}

static bool TryBuildRawDrawForTypedSubmit(
    const Movie1PsxRawDrawAction& sourceAction,
    Movie1PsxRawDrawAction& outAction,
    Movie1PsxRawDrawB590Metadata& outB590,
    Movie1PsxRawDrawBEE4Metadata& outBEE4,
    Movie1PsxRawDrawResolutionStatus& status,
    uint8_t languageIndex) {
    if (sourceAction.helper == Movie1PsxRawDrawHelper::Call8001BEE4) {
        return TryBuildRawDrawBEE4ForTypedSubmit(
            sourceAction,
            outAction,
            outBEE4,
            status);
    }
    return TryBuildRawDrawB590ForTypedSubmit(
        sourceAction,
        outAction,
        outB590,
        status,
        languageIndex);
}

static bool AppendRawDrawActionRecord(Movie1DrawPlan& plan,
                                      const Movie1PsxRawDrawAction& action) {
    if (plan.rawDrawActions.size() >= kMovie1DrawPlanMaxRawActions) {
        return false;
    }
    plan.rawDrawActions.push_back(action);
    plan.rawDrawActionCount = (uint32_t)plan.rawDrawActions.size();
    return true;
}

enum class ContextRawDrawEvalResult : uint8_t {
    CannotEvaluate = 0,
    Execute,
    Skip,
};

static bool TryReadMovie1TransitionCtxWord801C3640(
    const Movie1TransitionCtxWords801C3640& ctxWords,
    const Movie1PsxRawDrawAction& action,
    uint32_t& outWord) {
    if (!ctxWords.known || action.contextPtr != ctxWords.base) {
        return false;
    }
    if (action.contextOffset == 0u) {
        outWord = ctxWords.word0_flags801C3640;
        return true;
    }
    if (action.contextOffset == 4u) {
        outWord = ctxWords.word1_timecode801C3644;
        return true;
    }
    return false;
}

static int Movie1RawDrawLayerFromOtBucket(uint32_t otBucket) {
    switch (otBucket) {
    case 0:
        return 784;
    case 1:
        return 782;
    case 2:
        return 781;
    case 3:
        return 780;
    default:
        return 779;
    }
}

static int Movie1RendererOrderFromPsxCallOrder(uint32_t psxCallOrder) {
    return (int)psxCallOrder + 1;
}

static ContextRawDrawEvalResult EvaluateMovie1TransitionCtxRawDrawCondition(
    const Movie1TransitionCtxWords801C3640& ctxWords,
    const Movie1PsxRawDrawAction& action) {
    switch (action.condition) {
        case Movie1PsxRawDrawCondition::Always:
        case Movie1PsxRawDrawCondition::ActiveTileMaskCell:
            return ContextRawDrawEvalResult::Execute;
        case Movie1PsxRawDrawCondition::ContextWordEquals:
        case Movie1PsxRawDrawCondition::ContextWordNotEquals:
        case Movie1PsxRawDrawCondition::ContextWordDefault:
            break;
        default:
            return ContextRawDrawEvalResult::CannotEvaluate;
    }

    uint32_t actualWord = 0;
    if (!TryReadMovie1TransitionCtxWord801C3640(ctxWords, action, actualWord)) {
        return ContextRawDrawEvalResult::CannotEvaluate;
    }

    const uint32_t expectedWord = (uint32_t)action.contextValue;
    if (action.condition == Movie1PsxRawDrawCondition::ContextWordEquals) {
        return actualWord == expectedWord
            ? ContextRawDrawEvalResult::Execute
            : ContextRawDrawEvalResult::Skip;
    }
    if (action.condition == Movie1PsxRawDrawCondition::ContextWordNotEquals) {
        return actualWord != expectedWord
            ? ContextRawDrawEvalResult::Execute
            : ContextRawDrawEvalResult::Skip;
    }

    if (action.contextOffset == 4u) {
        return (actualWord != 1u && actualWord != 2u)
            ? ContextRawDrawEvalResult::Execute
            : ContextRawDrawEvalResult::Skip;
    }
    return ContextRawDrawEvalResult::CannotEvaluate;
}

static bool IsFastSpriteRgbGapStatus(Movie1PsxRawDrawResolutionStatus status) {
    return status == Movie1PsxRawDrawResolutionStatus::GapFastSpriteStackRgb ||
           status == Movie1PsxRawDrawResolutionStatus::GapRawDrawFastSpriteStackRgb ||
           status == Movie1PsxRawDrawResolutionStatus::GapTransitionRawActionFastSpriteStackRgb ||
           status == Movie1PsxRawDrawResolutionStatus::GapTextGlyphFastSpriteRgb;
}

static bool IsTransitionRawActionSource(uint32_t sourceFunction) {
    switch (sourceFunction) {
        case 0x8001D74Cu:
        case 0x8001EA74u:
        case 0x8001F524u:
        case 0x8001F698u:
        case 0x8001FCBCu:
        case 0x8001FC40u:
        case 0x8001FDC0u:
        case 0x8001FFD4u:
        case 0x80020090u:
        case 0x800201ACu:
        case 0x80020248u:
        case 0x80020308u:
        case 0x80021E60u:
        case 0x80022CBCu:
            return true;
        default:
            return false;
    }
}

static Movie1PsxRawDrawResolutionStatus ClassifyFastSpriteRgbGapStatus(
    const Movie1PsxRawDrawAction& action) {
    if (action.sourceFunction == 0x8001B954u ||
        action.sourceFunction == 0x8001B744u ||
        action.sourceFunction == 0x8001BCA0u) {
        return Movie1PsxRawDrawResolutionStatus::GapTextGlyphFastSpriteRgb;
    }
    if (IsTransitionRawActionSource(action.sourceFunction)) {
        return Movie1PsxRawDrawResolutionStatus::GapTransitionRawActionFastSpriteStackRgb;
    }
    return Movie1PsxRawDrawResolutionStatus::GapRawDrawFastSpriteStackRgb;
}

static Movie1PsxFastSpriteLocalRgbGapReason ClassifyFastSpriteRgbGapReason(
    const Movie1PsxRawDrawAction& action) {
    const Movie1PsxRawDrawResolutionStatus status =
        ClassifyFastSpriteRgbGapStatus(action);
    if (status == Movie1PsxRawDrawResolutionStatus::GapTextGlyphFastSpriteRgb) {
        return Movie1PsxFastSpriteLocalRgbGapReason::
            TextGlyphLocalRgbUnresolved8001B954;
    }
    if (status ==
        Movie1PsxRawDrawResolutionStatus::GapTransitionRawActionFastSpriteStackRgb) {
        return Movie1PsxFastSpriteLocalRgbGapReason::
            TransitionRawActionStackLocalTailUnresolved8001B590;
    }
    return Movie1PsxFastSpriteLocalRgbGapReason::
        RawDrawStackLocalTailUnresolved8001B590;
}

static void AttachFastSpriteRgbGapClassification(
    Movie1PsxRawDrawAction& action,
    Movie1PsxRawDrawB590Metadata& b590,
    Movie1PsxRawDrawResolutionStatus& status) {
    if (status != Movie1PsxRawDrawResolutionStatus::GapFastSpriteStackRgb) {
        return;
    }
    status = ClassifyFastSpriteRgbGapStatus(action);
    b590.gsSortFastSprite.localSpriteRgbGapReason =
        ClassifyFastSpriteRgbGapReason(action);
    action.b590 = b590;
}

static void AttachFastSpriteRgbGapClassification(
    Movie1PsxRawDrawAction& action,
    Movie1PsxRawDrawBEE4Metadata& bee4,
    Movie1PsxRawDrawResolutionStatus& status) {
    if (status != Movie1PsxRawDrawResolutionStatus::GapFastSpriteStackRgb) {
        return;
    }
    status = ClassifyFastSpriteRgbGapStatus(action);
    bee4.gsSortFastSprite.localSpriteRgbGapReason =
        Movie1PsxFastSpriteLocalRgbGapReason::
            StackLocalTailUnresolved8001BEE4;
    action.bee4 = bee4;
}

static bool AppendResolvedRawDrawActionToPlan(
    Movie1DrawPlan& plan,
    const Movie1PsxRawDrawAction& action) {
    if (plan.templates.size() >= kMovie1DrawPlanMaxTemplates) {
        return false;
    }
    uint32_t templateAddr = action.templateAddr;
    if (templateAddr == 0u && action.templateGpOffset != 0u) {
        (void)ResolveMovie1PsxGpTemplatePointer(
            action.templateGpOffset,
            templateAddr);
    }
    plan.templates.push_back(Movie1TemplateDrawCommand{});
    plan.templateCount = (uint32_t)plan.templates.size();
    Movie1TemplateDrawCommand& out = plan.templates.back();
    out.x = (float)action.x;
    out.y = (float)action.y;
    out.tplAddr = templateAddr;
    out.desc = ResolveMovie1PsxTemplateDesc(templateAddr);
    out.submitHelper = action.helper == Movie1PsxRawDrawHelper::Call8001C550
        ? Movie1PsxTemplateSubmitHelper::Sub8001C550
        : Movie1PsxTemplateSubmitHelper::Sub8001B590;
    out.alpha = 1.0f;
    out.layer = Movie1RawDrawLayerFromOtBucket(action.ot);
    out.order = Movie1RendererOrderFromPsxCallOrder(action.psxCallOrder);
    return true;
}

static void ClearMovieFrame30(Movie1RuntimeState& state) {
    state.currentMovieFrame30 = 0;
}

static void ClearDrawPlanFailure(Movie1RuntimeState& state) {
    state.lastDrawPlanBuildFailureKnown = false;
    state.lastDrawPlanBuildFailureReason =
        Movie1DrawPlanBuildFailureReason::None;
}

static void MarkDrawPlanFailure(
    Movie1RuntimeState& state,
    Movie1DrawPlan& plan,
    Movie1DrawPlanBuildFailureReason reason) {
    plan.buildFailureKnown = true;
    plan.buildFailureReason = reason;
    state.lastDrawPlanBuildFailureKnown = true;
    state.lastDrawPlanBuildFailureReason = reason;
}

static void AppendMovie1HostAction(Movie1HostActionList& actions,
                                   Movie1HostActionKind kind,
                                   uint32_t subFrame = 0,
                                   uint8_t cue9441CIndex = 0) {
    if (actions.count >= 16u) {
        return;
    }
    Movie1HostAction& action = actions.actions[actions.count++];
    action.kind = kind;
    action.subFrame = subFrame;
    action.cue9441CIndex = cue9441CIndex;
}

static void AppendTransitionSub800201ACAction(
    TransitionSub800201ACStep& step,
    TransitionSub800201ACAction action) {
    if (step.actionCount >= (uint32_t)(sizeof(step.actions) / sizeof(step.actions[0]))) {
        return;
    }
    step.actions[step.actionCount++] = action;
}

static void AppendTransitionTileMaskMutation(
    TransitionSub800201ACStep& step,
    TransitionTileMaskMutationKind kind,
    uint32_t startGp196,
    uint32_t count,
    uint32_t pattern,
    uint32_t value,
    uint32_t layer) {
    if (step.tileMutationCount >=
        (uint32_t)(sizeof(step.tileMutations) / sizeof(step.tileMutations[0]))) {
        return;
    }
    TransitionTileMaskMutation& mutation =
        step.tileMutations[step.tileMutationCount++];
    mutation.kind = kind;
    mutation.startGp196 = startGp196;
    mutation.count = count;
    mutation.pattern = pattern;
    mutation.value = value;
    mutation.layer = layer;
}

static void CopyTransitionTileMaskMutationsToRuntime(
    Movie1TransitionTileMaskRuntimeState& runtime,
    const TransitionSub800201ACStep& step) {
    runtime.lastMutationCount = step.tileMutationCount;
    for (uint32_t i = 0; i < runtime.lastMutationCount; ++i) {
        runtime.lastMutations[i] = step.tileMutations[i];
    }
}

static void InitTransitionTileMaskRuntime(
    Movie1TransitionTileMaskRuntimeState& runtime,
    bool active) {
    runtime.ownerValid = true;
    runtime.cellsKnown = true;
    for (uint32_t i = 0; i < kMovie1TransitionTileMaskCellCount; ++i) {
        runtime.cells[i] = active ? 1u : 0u;
    }
    ++runtime.mutationSerial;
}

static void MarkTransitionTileMaskRuntimeTableGap(
    Movie1TransitionTileMaskRuntimeState& runtime) {
    runtime.ownerValid = true;
    runtime.cellsKnown = false;
    ++runtime.mutationSerial;
}

static void WriteTransitionTileMaskRuntimeCell(
    Movie1TransitionTileMaskRuntimeState& runtime,
    const Movie1TransitionTileMaskPair& pair,
    uint8_t value) {
    runtime.cells[(uint32_t)pair.row * kMovie1TransitionTileMaskCols + pair.col] =
        value;
}

static void ApplyDword8004EB80ClearOrderMutation(
    Movie1TransitionTileMaskRuntimeState& runtime,
    const TransitionTileMaskMutation& mutation) {
    if (!runtime.ownerValid || !runtime.cellsKnown) {
        MarkTransitionTileMaskRuntimeTableGap(runtime);
        return;
    }
    for (uint32_t i = 0; i < mutation.count; ++i) {
        const uint32_t gp196 = mutation.startGp196 + i;
        if (gp196 < kMovie1TransitionTileMaskCellCount) {
            WriteTransitionTileMaskRuntimeCell(
                runtime,
                kDword8004EB80ClearOrderPairs[gp196],
                0u);
        }
    }
    runtime.ownerValid = true;
    runtime.cellsKnown = true;
    ++runtime.mutationSerial;
}

static bool ApplySub8001F698Pattern4Mutation(
    Movie1TransitionTileMaskRuntimeState& runtime,
    const TransitionTileMaskMutation& mutation) {
    if (mutation.pattern != 4u) {
        return false;
    }
    if (!runtime.ownerValid || !runtime.cellsKnown) {
        MarkTransitionTileMaskRuntimeTableGap(runtime);
        return true;
    }
    for (uint32_t i = 0; i < mutation.count; ++i) {
        const uint32_t gp196 = mutation.startGp196 + i;
        if (gp196 < kMovie1TransitionTileMaskCellCount) {
            const uint32_t tableIndex =
                kMovie1TransitionTileMaskCellCount - 1u - gp196;
            WriteTransitionTileMaskRuntimeCell(
                runtime,
                kDword8004F180Pattern4OrderPairs[tableIndex],
                (uint8_t)(mutation.value ? 1u : 0u));
        }
    }
    runtime.ownerValid = true;
    runtime.cellsKnown = true;
    ++runtime.mutationSerial;
    return true;
}

static void ApplyTransitionTileMaskMutationsToRuntime(
    Movie1TransitionTileMaskRuntimeState& runtime,
    const TransitionSub800201ACStep& step) {
    if (step.tileMutationCount == 0u) {
        return;
    }
    CopyTransitionTileMaskMutationsToRuntime(runtime, step);
    for (uint32_t i = 0; i < step.tileMutationCount; ++i) {
        const TransitionTileMaskMutation& mutation = step.tileMutations[i];
        switch (mutation.kind) {
            case TransitionTileMaskMutationKind::InitAllClearBySub8001FFD4:
                InitTransitionTileMaskRuntime(runtime, false);
                break;
            case TransitionTileMaskMutationKind::InitAllSetBySub8001FFD4:
                InitTransitionTileMaskRuntime(runtime, true);
                break;
            case TransitionTileMaskMutationKind::ClearByDword8004EB80Order:
                ApplyDword8004EB80ClearOrderMutation(runtime, mutation);
                break;
            case TransitionTileMaskMutationKind::SetBySub8001F698Pattern4:
            case TransitionTileMaskMutationKind::ClearBySub8001F698Pattern4:
                if (!ApplySub8001F698Pattern4Mutation(runtime, mutation)) {
                    MarkTransitionTileMaskRuntimeTableGap(runtime);
                }
                break;
            case TransitionTileMaskMutationKind::DrawActiveMask:
                runtime.ownerValid = true;
                break;
            case TransitionTileMaskMutationKind::None:
            default:
                break;
        }
    }
}

static bool TryAttachActiveTileMaskCell(
    Movie1PsxRawDrawAction& action,
    const Movie1TransitionTileMaskRuntimeState* runtime) {
    if (action.condition != Movie1PsxRawDrawCondition::ActiveTileMaskCell ||
        runtime == nullptr ||
        !runtime->ownerValid ||
        !runtime->cellsKnown ||
        action.activeTileMaskIndex >= kMovie1TransitionTileMaskCellCount) {
        return false;
    }
    action.activeTileMaskKnown = true;
    action.activeTileMaskActive =
        runtime->cells[action.activeTileMaskIndex] != 0u;
    if (action.activeTileMaskActive) {
        action.condition = Movie1PsxRawDrawCondition::Always;
    }
    return action.activeTileMaskActive;
}

static void AppendMovie1PsxRawDrawAction(
    TransitionSub800201ACStep& step,
    const Movie1PsxRawDrawAction& action) {
    if (step.psxDrawActions.size() >= kMovie1DrawPlanMaxRawActions) {
        return;
    }
    Movie1PsxRawDrawAction out = action;
    out.psxCallOrder = (uint32_t)step.psxDrawActions.size();
    step.psxDrawActions.push_back(out);
    step.psxDrawActionCount = (uint32_t)step.psxDrawActions.size();
}

static void AppendRawCall8001C550(TransitionSub800201ACStep& step,
                                  uint32_t sourceFunction,
                                  int16_t x,
                                  int16_t y,
                                  uint32_t templateAddr,
                                  uint32_t ot,
                                  uint32_t templateGpOffset = 0u,
                                  Movie1PsxRawDrawCondition condition =
                                      Movie1PsxRawDrawCondition::Always,
                                  uint32_t contextPtr = 0u,
                                  uint32_t contextOffset = 0u,
                                  int32_t contextValue = 0) {
    Movie1PsxRawDrawAction action{};
    action.helper = Movie1PsxRawDrawHelper::Call8001C550;
    action.condition = condition;
    action.sourceFunction = sourceFunction;
    action.x = x;
    action.y = y;
    action.templateAddr = templateAddr;
    action.templateGpOffset = templateGpOffset;
    if (templateGpOffset != 0u) {
        action.templateGpBaseAddr = kMovie1PsxGpBase8006EA40;
    }
    action.ot = ot;
    action.bufferIndex = step.nextGp872;
    action.workBaseAddr = kMovie1PsxWorkBufferBase80087288;
    action.contextPtr = contextPtr;
    action.contextOffset = contextOffset;
    action.contextValue = contextValue;
    AttachRawDrawB25CMetadata(action);
    AppendMovie1PsxRawDrawAction(step, action);
}

static void AppendRawCall8001C5A8(TransitionSub800201ACStep& step,
                                  uint32_t sourceFunction,
                                  uint32_t positionAddr,
                                  uint32_t templateAddr,
                                  uint32_t ot,
                                  uint32_t positionLanguageStride = 0u,
                                  uint32_t templatePtrTableAddr = 0u,
                                  uint32_t templateLanguageStride = 0u,
                                  Movie1PsxRawDrawCondition condition =
                                      Movie1PsxRawDrawCondition::Always,
                                  uint32_t contextPtr = 0u,
                                  uint32_t contextOffset = 0u,
                                  int32_t contextValue = 0) {
    Movie1PsxRawDrawAction action =
        BuildRawDrawWrapperActionSub8001C5A8(
            sourceFunction,
            positionAddr,
            templateAddr,
            ot,
            step.nextGp872,
            kMovie1PsxWorkBufferBase80087288);
    action.condition = condition;
    action.positionLanguageStride = positionLanguageStride;
    action.templatePtrTableAddr = templatePtrTableAddr;
    action.templateLanguageStride = templateLanguageStride;
    action.contextPtr = contextPtr;
    action.contextOffset = contextOffset;
    action.contextValue = contextValue;
    action.languageSourceAddr = kMovie1PsxLanguageWord800916D8;
    AttachRawDrawB25CMetadata(action);
    AppendMovie1PsxRawDrawAction(step, action);
}

static void AppendRawCall8001B590(TransitionSub800201ACStep& step,
                                  uint32_t sourceFunction,
                                  int16_t x,
                                  int16_t y,
                                  uint32_t templateAddr,
                                  uint32_t ot,
                                  uint32_t bufferIndex) {
    Movie1PsxRawDrawAction action{};
    action.helper = Movie1PsxRawDrawHelper::Call8001B590;
    action.sourceFunction = sourceFunction;
    action.x = x;
    action.y = y;
    action.templateAddr = templateAddr;
    action.ot = ot;
    action.bufferIndex = bufferIndex;
    action.workBaseAddr = kMovie1PsxWorkBufferBase80087288;
    AttachRawDrawB25CMetadata(action);
    AttachRawDrawB590Metadata(action);
    AppendMovie1PsxRawDrawAction(step, action);
}

static void AppendActiveTileMaskC550RawDrawActions(
    TransitionSub800201ACStep& step,
    uint32_t sourceFunction,
    uint32_t ot) {
    for (int row = 0; row < kStage1IntroTransitionRows; ++row) {
        for (int col = 0; col < kStage1IntroTransitionCols; ++col) {
            Movie1PsxRawDrawAction action{};
            action.helper = Movie1PsxRawDrawHelper::Call8001C550;
            action.condition = Movie1PsxRawDrawCondition::ActiveTileMaskCell;
            action.sourceFunction = sourceFunction;
            action.x = (int16_t)(20 * col);
            action.y = (int16_t)(20 * row);
            action.templatePtrTableAddr = 0x80050720u;
            action.templateLanguageStride = 4u;
            action.ot = ot;
            action.bufferIndex = step.nextGp872;
            action.workBaseAddr = kMovie1PsxWorkBufferBase80087288;
            action.tableIndex =
                kDword80050420TransitionTileIndex[row][col] & 3u;
            action.templateAddr =
                kOff80050720TransitionTemplatePtrs[action.tableIndex];
            action.activeTileMaskRow = (uint16_t)row;
            action.activeTileMaskCol = (uint16_t)col;
            action.activeTileMaskIndex =
                (uint32_t)row * kMovie1TransitionTileMaskCols + (uint32_t)col;
            AttachRawDrawB25CMetadata(action);
            AppendMovie1PsxRawDrawAction(step, action);
        }
    }
}

static void AppendSub8001EA74CommonActions(TransitionSub800201ACStep& step) {
    AppendTransitionSub800201ACAction(
        step,
        TransitionSub800201ACAction::Sub8001EA74_SetGp872AndGp792);
    AppendTransitionSub800201ACAction(
        step,
        TransitionSub800201ACAction::Sub80040F90_SetDrawBuffer);
    AppendTransitionSub800201ACAction(
        step,
        TransitionSub800201ACAction::Sub80040CC8_BindBufferState);
}

static void AppendSub8001EBF4Actions(TransitionSub800201ACStep& step) {
    AppendTransitionSub800201ACAction(
        step,
        TransitionSub800201ACAction::Sub8001EBF4_Sub80040370);
    if (step.nextGp792 != 0u) {
        AppendTransitionSub800201ACAction(
            step,
            TransitionSub800201ACAction::Sub80040420_ClearWhenGp792NonZero);
    }
    AppendTransitionSub800201ACAction(
        step,
        TransitionSub800201ACAction::Sub80040CA4_PresentGp872Buffer);
}

static bool TransitionStepHasAction(
    const TransitionSub800201ACStep& step,
    TransitionSub800201ACAction action) {
    for (uint32_t i = 0; i < step.actionCount; ++i) {
        if (step.actions[i] == action) {
            return true;
        }
    }
    return false;
}

static void ApplySub80040370Flip(Movie1RuntimeState& state) {
    (void)PrPsxGraphOwnerDirect::PsxCall80040370_FlipGraph(
        state.graphOwner8003FB9C);
}

static uint32_t AdvanceGp196BySub8001FCBCOrSub8001FC40(uint32_t gp196,
                                                       uint32_t count) {
    return gp196 + count;
}

static uint32_t AdvanceGp196BySub8001F524(uint32_t gp196, uint32_t count) {
    if (gp196 >= 194u) {
        return gp196;
    }
    return gp196 + count;
}

static void AppendSub8001FFD4InitMaskMutation(TransitionSub800201ACStep& step,
                                              uint32_t arg) {
    AppendTransitionTileMaskMutation(
        step,
        arg == 1u
            ? TransitionTileMaskMutationKind::InitAllClearBySub8001FFD4
            : TransitionTileMaskMutationKind::InitAllSetBySub8001FFD4,
        0u,
        kMovie1TransitionTileMaskCellCount,
        0u,
        arg == 1u ? 0u : 1u,
        0u);
}

static void AppendSub8001F524DirectEffects(TransitionSub800201ACStep& step,
                                           uint16_t layer,
                                           uint32_t count) {
    AppendTransitionSub800201ACAction(
        step,
        TransitionSub800201ACAction::Sub8001F524_Mode1Clear8AndDrawMask);
    AppendTransitionSub800201ACAction(
        step,
        TransitionSub800201ACAction::Sub8001F524_ClearMaskOrderDword8004EB80);
    AppendTransitionTileMaskMutation(
        step,
        TransitionTileMaskMutationKind::ClearByDword8004EB80Order,
        step.gp196,
        count,
        10u,
        0u,
        layer);
    AppendTransitionSub800201ACAction(
        step,
        TransitionSub800201ACAction::Sub8001F524_DrawActiveMask);
    AppendTransitionTileMaskMutation(
        step,
        TransitionTileMaskMutationKind::DrawActiveMask,
        step.gp196,
        16u * 12u,
        0u,
        0u,
        layer);
    AppendActiveTileMaskC550RawDrawActions(step, 0x8001F524u, layer);
}

static void AppendSub8001FCBCDirectEffects(TransitionSub800201ACStep& step,
                                           uint32_t count,
                                           uint32_t pattern) {
    AppendTransitionSub800201ACAction(
        step,
        TransitionSub800201ACAction::Sub8001FCBC_Mode2Advance8);
    AppendTransitionSub800201ACAction(
        step,
        TransitionSub800201ACAction::Sub8001F698_Pattern4SetMask);
    AppendTransitionTileMaskMutation(
        step,
        TransitionTileMaskMutationKind::SetBySub8001F698Pattern4,
        step.gp196,
        count,
        pattern,
        1u,
        0u);
}

static void AppendSub8001FC40DirectEffects(TransitionSub800201ACStep& step,
                                           uint32_t count,
                                           uint32_t pattern) {
    AppendTransitionSub800201ACAction(
        step,
        TransitionSub800201ACAction::Sub8001FC40_Mode3Or4Advance8);
    AppendTransitionSub800201ACAction(
        step,
        TransitionSub800201ACAction::Sub8001F698_Pattern4ClearMask);
    AppendTransitionTileMaskMutation(
        step,
        TransitionTileMaskMutationKind::ClearBySub8001F698Pattern4,
        step.gp196,
        count,
        pattern,
        0u,
        0u);
}

static void AppendSub80022CBCType4RawDrawActions(TransitionSub800201ACStep& step,
                                                 uint32_t contextPtr) {
    static constexpr uint32_t kSource = 0x80022CBCu;
    AppendTransitionSub800201ACAction(
        step,
        TransitionSub800201ACAction::Sub80022CBC_Mode3DirectDraw);
    AppendRawCall8001C550(step, kSource, 28, 56, 0x80052350u, 1u);
    AppendRawCall8001C5A8(
        step,
        kSource,
        0x80053A34u,
        0u,
        1u,
        8u,
        0x80053A30u,
        8u);
    AppendRawCall8001C550(
        step,
        kSource,
        224,
        149,
        0x800526A0u,
        1u,
        0u,
        Movie1PsxRawDrawCondition::ContextWordEquals,
        contextPtr,
        0u,
        1);
    AppendRawCall8001C550(
        step,
        kSource,
        224,
        149,
        0x800526B0u,
        1u,
        0u,
        Movie1PsxRawDrawCondition::ContextWordNotEquals,
        contextPtr,
        0u,
        1);

    AppendRawCall8001C5A8(
        step,
        kSource,
        0x80053AB4u,
        0u,
        1u,
        16u,
        0x80053AB0u,
        16u,
        Movie1PsxRawDrawCondition::ContextWordEquals,
        contextPtr,
        4u,
        1);
    AppendRawCall8001C5A8(
        step,
        kSource,
        0x80053B04u,
        0u,
        1u,
        16u,
        0x80053AFCu,
        16u,
        Movie1PsxRawDrawCondition::ContextWordEquals,
        contextPtr,
        4u,
        1);
    AppendRawCall8001C550(
        step,
        kSource,
        234,
        159,
        0x800525A0u,
        1u,
        0u,
        Movie1PsxRawDrawCondition::ContextWordEquals,
        contextPtr,
        4u,
        1);
    AppendRawCall8001C550(
        step,
        kSource,
        234,
        182,
        0x800526C0u,
        1u,
        0u,
        Movie1PsxRawDrawCondition::ContextWordEquals,
        contextPtr,
        4u,
        1);

    AppendRawCall8001C5A8(
        step,
        kSource,
        0x80053AB4u,
        0u,
        1u,
        16u,
        0x80053AACu,
        16u,
        Movie1PsxRawDrawCondition::ContextWordEquals,
        contextPtr,
        4u,
        2);
    AppendRawCall8001C5A8(
        step,
        kSource,
        0x80053B04u,
        0u,
        1u,
        16u,
        0x80053B00u,
        16u,
        Movie1PsxRawDrawCondition::ContextWordEquals,
        contextPtr,
        4u,
        2);
    AppendRawCall8001C550(
        step,
        kSource,
        234,
        159,
        0x80052590u,
        1u,
        0u,
        Movie1PsxRawDrawCondition::ContextWordEquals,
        contextPtr,
        4u,
        2);
    AppendRawCall8001C550(
        step,
        kSource,
        234,
        182,
        0x800526D0u,
        1u,
        0u,
        Movie1PsxRawDrawCondition::ContextWordEquals,
        contextPtr,
        4u,
        2);

    AppendRawCall8001C5A8(
        step,
        kSource,
        0x80053AB4u,
        0u,
        1u,
        16u,
        0x80053AACu,
        16u,
        Movie1PsxRawDrawCondition::ContextWordDefault,
        contextPtr,
        4u,
        0);
    AppendRawCall8001C5A8(
        step,
        kSource,
        0x80053B04u,
        0u,
        1u,
        16u,
        0x80053AFCu,
        16u,
        Movie1PsxRawDrawCondition::ContextWordDefault,
        contextPtr,
        4u,
        0);
    AppendRawCall8001C550(
        step,
        kSource,
        234,
        159,
        0x80052590u,
        1u,
        0u,
        Movie1PsxRawDrawCondition::ContextWordDefault,
        contextPtr,
        4u,
        0);
    AppendRawCall8001C550(
        step,
        kSource,
        234,
        182,
        0x800526C0u,
        1u,
        0u,
        Movie1PsxRawDrawCondition::ContextWordDefault,
        contextPtr,
        4u,
        0);
    AppendRawCall8001C550(step, kSource, 121, 36, 0x80052320u, 2u);
}

static void AppendSub80021E60Ctx0RawDrawActions(TransitionSub800201ACStep& step) {
    static constexpr uint32_t kSource = 0x80021E60u;
    AppendTransitionSub800201ACAction(
        step,
        TransitionSub800201ACAction::Sub80021E60_Mode4DirectDraw);
    AppendRawCall8001C550(step, kSource, 28, 36, 0u, 1u, 224u);
    AppendRawCall8001C550(step, kSource, 110, 56, 0u, 1u, 232u);
    AppendRawCall8001C550(step, kSource, 207, 34, 0u, 1u, 240u);
    AppendRawCall8001C550(step, kSource, 26, 106, 0u, 1u, 248u);
    AppendRawCall8001C550(step, kSource, 94, 122, 0u, 1u, 256u);
    AppendRawCall8001C550(step, kSource, 162, 122, 0u, 1u, 264u);
    AppendRawCall8001C550(step, kSource, 226, 106, 0u, 1u, 272u);
    AppendRawCall8001C550(step, kSource, 231, 179, 0u, 1u, 212u);
    AppendRawCall8001C5A8(step, kSource, 0x80053560u, 0x80050F20u, 1u);
    AppendRawCall8001C5A8(step, kSource, 0x800535BCu, 0u, 1u, 16u, 0x800535B0u, 16u);
    AppendRawCall8001C5A8(step, kSource, 0x8005364Cu, 0u, 1u, 16u, 0x80053640u, 16u);
    AppendRawCall8001C5A8(step, kSource, 0x8005369Cu, 0u, 1u, 16u, 0x80053690u, 16u);
    AppendRawCall8001C5A8(step, kSource, 0x8005371Cu, 0u, 1u, 16u, 0x80053710u, 16u);
    AppendRawCall8001C5A8(step, kSource, 0x8005376Cu, 0u, 1u, 16u, 0x80053760u, 16u);
    AppendRawCall8001C5A8(step, kSource, 0x800537BCu, 0u, 1u, 16u, 0x800537B0u, 16u);
    AppendRawCall8001C5A8(step, kSource, 0x8005380Cu, 0u, 1u, 16u, 0x80053800u, 16u);
    AppendRawCall8001C5A8(step, kSource, 0x8005300Cu, 0u, 1u, 16u, 0x80053000u, 16u);
    AppendRawCall8001C550(step, kSource, 37, 47, 0x80050EF0u, 1u);
    AppendRawCall8001C550(step, kSource, 118, 65, 0x80051030u, 1u);
    AppendRawCall8001C5A8(step, kSource, 0x8005362Cu, 0x80051170u, 1u);
    AppendRawCall8001C5A8(step, kSource, 0x8005363Cu, 0x800512B0u, 1u);
    AppendRawCall8001C550(step, kSource, 32, 115, 0x800513D0u, 1u);
    AppendRawCall8001C550(step, kSource, 95, 136, 0x80051510u, 1u);
    AppendRawCall8001C550(step, kSource, 160, 136, 0x80051650u, 1u);
    AppendRawCall8001C550(step, kSource, 216, 115, 0x80051790u, 1u);
    AppendRawCall8001C5A8(step, kSource, 0x80052FFCu, 0x800509A0u, 1u);
}

static void AppendSub8001D74CRawDrawActions(TransitionSub800201ACStep& step,
                                            uint32_t ot,
                                            uint32_t bufferIndex) {
    static constexpr uint32_t kSource = 0x8001D74Cu;
    AppendTransitionSub800201ACAction(
        step,
        TransitionSub800201ACAction::Sub8001D74C_Mode3Or4DirectDraw);
    AppendRawCall8001B590(step, kSource, 20, 20, 0x8004E7E0u, ot, bufferIndex);
    AppendRawCall8001B590(step, kSource, 20, 120, 0x8004E7F0u, ot, bufferIndex);
    AppendRawCall8001B590(step, kSource, 280, 20, 0x8004E800u, ot, bufferIndex);
    AppendRawCall8001B590(step, kSource, 280, 120, 0x8004E810u, ot, bufferIndex);
    AppendRawCall8001B590(step, kSource, 40, 20, 0x8004E820u, ot, bufferIndex);
    AppendRawCall8001B590(step, kSource, 160, 20, 0x8004E830u, ot, bufferIndex);
    AppendRawCall8001B590(step, kSource, 40, 200, 0x8004E840u, ot, bufferIndex);
    AppendRawCall8001B590(step, kSource, 160, 200, 0x8004E850u, ot, bufferIndex);
    for (int outer = 0; outer < 6; ++outer) {
        const int16_t x = (int16_t)(40 + 40 * outer);
        for (int inner = 0; inner < 4; ++inner) {
            const int16_t y = (int16_t)(40 + 40 * inner);
            AppendRawCall8001B590(step, kSource, x, y, 0x8004E7D0u, ot, bufferIndex);
        }
    }
    for (int i = 0; i < 7; ++i) {
        const int16_t x0 = (int16_t)(40 + 40 * i);
        const int16_t x1 = (int16_t)(20 + 40 * i);
        AppendRawCall8001B590(step, kSource, x0, 0, 0x8004E900u, ot, bufferIndex);
        AppendRawCall8001B590(step, kSource, x1, 0, 0x8004E910u, ot, bufferIndex);
        AppendRawCall8001B590(step, kSource, x0, 220, 0x8004E920u, ot, bufferIndex);
        AppendRawCall8001B590(step, kSource, x1, 220, 0x8004E930u, ot, bufferIndex);
    }
    for (int i = 0; i < 6; ++i) {
        const int16_t y0 = (int16_t)(40 * i);
        const int16_t y1 = (int16_t)(20 + 40 * i);
        AppendRawCall8001B590(step, kSource, 0, y0, 0x8004E900u, ot, bufferIndex);
        AppendRawCall8001B590(step, kSource, 300, y0, 0x8004E910u, ot, bufferIndex);
        AppendRawCall8001B590(step, kSource, 0, y1, 0x8004E920u, ot, bufferIndex);
        AppendRawCall8001B590(step, kSource, 300, y1, 0x8004E930u, ot, bufferIndex);
    }
}

static void AppendSub8001FDC0RawDrawActions(TransitionSub800201ACStep& step,
                                            uint32_t ot) {
    static constexpr uint32_t kSource = 0x8001FDC0u;
    AppendTransitionSub800201ACAction(
        step,
        TransitionSub800201ACAction::Sub8001FDC0_DrawMask);
    AppendActiveTileMaskC550RawDrawActions(step, kSource, ot);
}

static void ClearTransitionDrawStep(Movie1RuntimeState& state) {
    state.transitionDrawStepValid = false;
    state.transitionDrawStep = TransitionSub800201ACStep{};
    state.transitionDrawMask = Movie1TransitionTileMaskRuntimeState{};
}

static void LatchTransitionDrawStep(
    Movie1RuntimeState& state,
    const TransitionSub800201ACStep& step) {
    state.transitionDrawStepValid = true;
    state.transitionDrawStep = step;
    state.transitionDrawMask = state.transitionTileMask;
    state.transitionDrawStep.activeTileMask = state.transitionDrawMask;
}

static void MarkMovieCompleted(Movie1RuntimeState& state) {
    state.strPlayed = true;
    state.strStarted = false;
    state.outroActive = false;
    state.outroGp196 = 0;
    state.outroSourceFrame30 = 0;
    state.outroTailFrames = 0;
    state.outroTailCompletePending = false;
    state.playAndWaitCompletionPending = true;
    state.transitionGp792 = 0;
    state.transitionGp872 = 0;
    state.transitionSub800201ACActive = false;
    state.transitionSub800201ACCompletesMovie = true;
    state.transitionSub800201ACCompleted = false;
    state.transitionSub800201ACPhase = TransitionSub800201ACPhase::Idle;
    state.transitionSub800201ACA1 = 0;
    state.transitionSub800201ACA2 = 0;
    state.transitionSub800201ACA3 = 0;
    state.transitionSub800201ACA4 = 0;
    state.transitionSub80020090TailIndex = 0;
    state.transitionSub80020110Variant = false;
    state.transitionCtxWords801C3640 = Movie1TransitionCtxWords801C3640{};
    state.transitionTileMask = Movie1TransitionTileMaskRuntimeState{};
    ClearTransitionDrawStep(state);
    state.skipPreludeFrames = 0;
    state.skipPreludeSourceFrame30 = 0;
    ClearMovieFrame30(state);
}

static void MarkMovieStrBlockCompletedBeforePostTransition(
    Movie1RuntimeState& state,
    uint32_t sourceFrame30) {
    state.strPlayed = true;
    state.strStarted = false;
    state.outroActive = false;
    state.outroDrainStrAudio = false;
    state.outroCue0Played = false;
    state.outroCue1Played = false;
    state.outroGp196 = 192u;
    state.outroSourceFrame30 = sourceFrame30;
    state.outroTailFrames = 0;
    state.outroTailCompletePending = false;
    state.playAndWaitCompletionPending = true;
    state.transitionGp792 = 0;
    state.transitionGp872 = 0;
    state.transitionSub800201ACActive = false;
    state.transitionSub800201ACCompletesMovie = false;
    state.transitionSub800201ACCompleted = false;
    state.transitionSub800201ACPhase = TransitionSub800201ACPhase::Idle;
    state.transitionSub800201ACA1 = 0;
    state.transitionSub800201ACA2 = 0;
    state.transitionSub800201ACA3 = 0;
    state.transitionSub800201ACA4 = 0;
    state.transitionSub80020090TailIndex = 0;
    state.transitionSub80020110Variant = false;
    state.transitionTileMask = Movie1TransitionTileMaskRuntimeState{};
    ClearTransitionDrawStep(state);
    state.skipPreludeFrames = 0;
    state.skipPreludeSourceFrame30 = 0;
    state.currentMovieFrame30 = 0;
}

}  // namespace

Movie1PsxTemplateDesc ResolveMovie1PsxTemplateDesc(uint32_t tplAddr) {
    for (const Movie1PsxStaticTemplateEntry& entry : kMovie1PsxTemplates) {
        if (entry.addr == tplAddr) {
            return entry.tpl;
        }
    }
    return Movie1PsxTemplateDesc{};
}

Movie1PsxRawDrawB25CMetadata BuildRawDrawTileRectFieldsSub8001B25C(
    uint32_t templateAddr,
    const Movie1PsxTemplateDesc& desc,
    uint16_t glyphIndex,
    bool addGlyphIndexToFinalWord) {
    Movie1PsxRawDrawB25CMetadata out{};
    out.valid = desc.valid;
    out.templateAddr = templateAddr;

    // sub_8001B25C: copy the template fields, then build the UV/CLUT words.
    out.copiedAttr = desc.attr;
    out.width = desc.w;
    out.height = desc.h;
    out.sourceWord4 = desc.texX;
    out.sourceWord6 = desc.texY;
    out.sourceWord12 = desc.clutX;
    out.sourceWord14 = desc.clutY;

    out.glyphIndex = glyphIndex;
    out.addGlyphIndexToFinalWord = addGlyphIndexToFinalWord;
    const uint16_t v8 = (uint16_t)(4u * desc.texX + glyphIndex * desc.w);
    out.computedUvWord = v8;
    out.computedClutWord =
        ComputeSub80043DF4ClutWordFromTemplateWords(
            desc.texX,
            desc.texY,
            desc.w,
            glyphIndex);
    out.computedUByte = (uint8_t)v8;
    out.computedVByte = (uint8_t)desc.texY;
    out.finalWord = addGlyphIndexToFinalWord
        ? (uint16_t)(desc.clutY + glyphIndex)
        : desc.clutY;
    return out;
}

Movie1PsxRawDrawBE34Metadata BuildRawDrawTileRectFieldsSub8001BE34(
    uint32_t templateAddr,
    const Movie1PsxTemplateDesc& desc,
    int32_t enabled) {
    Movie1PsxRawDrawBE34Metadata out{};
    out.valid = desc.valid;
    out.templateAddr = templateAddr;
    out.copiedAttr = desc.attr;
    out.sourceWord4 = desc.texX;
    out.sourceWord6 = desc.texY;
    out.width = desc.w;
    out.height = desc.h;
    out.sourceWord12 = desc.clutX;
    out.sourceWord14 = desc.clutY;
    out.enabled = enabled;
    const uint16_t v8 = (uint16_t)(4u * desc.texX);
    out.computedUvWord = v8;
    out.computedClutWord =
        ComputeSub80043DF4ClutWordFromTemplateWords(
            desc.texX,
            desc.texY,
            desc.w,
            0u);
    out.computedUByte = (uint8_t)v8;
    out.computedVByte = (uint8_t)desc.texY;
    out.forcesEnabledClutY = enabled != 0;
    out.finalWord = out.forcesEnabledClutY ? 0x01E4u : desc.clutY;
    return out;
}

Movie1PsxRawDrawAction BuildRawDrawWrapperActionSub8001C5A8(
    uint32_t sourceFunction,
    uint32_t positionAddr,
    uint32_t templateAddr,
    uint32_t ot,
    uint32_t bufferIndex,
    uint32_t workBaseAddr) {
    Movie1PsxRawDrawAction action{};
    action.helper = Movie1PsxRawDrawHelper::Call8001C5A8;
    action.sourceFunction = sourceFunction;
    action.positionAddr = positionAddr;
    action.templateAddr = templateAddr;
    action.ot = ot;
    action.bufferIndex = bufferIndex;
    action.workBaseAddr = workBaseAddr;
    AttachRawDrawC5A8Metadata(action);
    return action;
}

Movie1PsxRawDrawAction BuildRawDrawSubmitActionSub8001B590(
    uint32_t sourceFunction,
    int16_t screenX,
    int16_t screenY,
    uint32_t templateAddr,
    uint16_t glyphIndex,
    bool addGlyphIndexToFinalWord,
    uint16_t priority,
    uint32_t otPacketAddr,
    uint32_t bufferIndex,
    uint32_t workBaseAddr) {
    Movie1PsxRawDrawAction action{};
    action.helper = Movie1PsxRawDrawHelper::Call8001B590;
    action.sourceFunction = sourceFunction;
    action.x = screenX;
    action.y = screenY;
    action.templateAddr = templateAddr;
    action.ot = priority;
    action.bufferIndex = bufferIndex;
    action.workBaseAddr = workBaseAddr;

    const Movie1PsxTemplateDesc desc =
        ResolveMovie1PsxTemplateDesc(templateAddr);
    action.b25c =
        BuildRawDrawB25CMetadata(
            templateAddr,
            desc,
            glyphIndex,
            addGlyphIndexToFinalWord);

    Movie1PsxRawDrawB590Metadata& b590 = action.b590;
    b590.valid = true;
    b590.screenX = screenX;
    b590.screenY = screenY;
    b590.spriteX = (int16_t)(screenX - kMovie1PsxScreenW / 2);
    b590.spriteY = (int16_t)(screenY - kMovie1PsxScreenH / 2);
    b590.templateAddr = templateAddr;
    b590.glyphIndex = glyphIndex;
    b590.addGlyphIndexToFinalWord = addGlyphIndexToFinalWord;
    b590.priority = priority;
    b590.otPacketAddr = otPacketAddr;
    b590.workBaseAddr = workBaseAddr;
    b590.bufferIndex = bufferIndex;
    b590.b25c = action.b25c;
    b590.gsSortFastSprite.valid = true;
    b590.gsSortFastSprite.otStructAddr = otPacketAddr;
    b590.gsSortFastSprite.priority = priority;
    b590.gsSortFastSprite.localSprite =
        BuildRawDrawFastSpriteLocalSub8001B590(b590);
    PrPsxFastSpriteSubmitDirect::FastSpriteRawLocalBytes8003FA20
        stackSeed{};
    stackSeed.provenance =
        PrPsxFastSpriteSubmitDirect::
            FastSpriteRawLocalProvenance8003FA20::PsxStackScratchResidue;
    stackSeed.note =
        "8001B590/8001B25C write local prefix only; RGB tail requires PSX "
        "stack scratch bytes";
    b590.gsSortFastSprite.rawLocalBytes =
        PrPsxFastSpriteSubmitDirect::
            BuildRawLocalBytesFromKnownFields8003FA20(
                b590.gsSortFastSprite.localSprite,
                stackSeed,
                0x8001B590u,
                0x8001B5D4u,
                stackSeed.note);
    b590.gsSortFastSprite.localSpritePrefixKnown = true;
    b590.gsSortFastSprite.localSpriteRgbKnown = false;
    b590.gsSortFastSprite.localSpriteRgbGapReason =
        Movie1PsxFastSpriteLocalRgbGapReason::
            RawDrawStackLocalTailUnresolved8001B590;
    b590.gsSortFastSprite.localSpriteKnown = false;
    return action;
}

Movie1PsxRawDrawAction BuildRawDrawSubmitActionSub8001BEE4(
    uint32_t sourceFunction,
    int16_t screenX,
    int16_t screenY,
    uint32_t templateAddr,
    int32_t enabled,
    uint32_t otPacketAddr,
    uint32_t bufferIndex,
    uint32_t workBaseAddr,
    const PrPsxFastSpriteSubmitDirect::FastSpriteStackScratchTail8003FA20&
        stackTail) {
    Movie1PsxRawDrawAction action{};
    action.helper = Movie1PsxRawDrawHelper::Call8001BEE4;
    action.sourceFunction = sourceFunction;
    action.x = screenX;
    action.y = screenY;
    action.templateAddr = templateAddr;
    action.ot = 3u;
    action.bufferIndex = bufferIndex;
    action.workBaseAddr = workBaseAddr;

    const Movie1PsxTemplateDesc desc =
        ResolveMovie1PsxTemplateDesc(templateAddr);
    action.be34 =
        BuildRawDrawTileRectFieldsSub8001BE34(templateAddr, desc, enabled);

    Movie1PsxRawDrawBEE4Metadata& bee4 = action.bee4;
    bee4.valid = true;
    bee4.screenX = screenX;
    bee4.screenY = screenY;
    bee4.spriteX = (int16_t)(screenX - kMovie1PsxScreenW / 2);
    bee4.spriteY = (int16_t)(screenY - kMovie1PsxScreenH / 2);
    bee4.templateAddr = templateAddr;
    bee4.enabled = enabled;
    bee4.priority = 3u;
    bee4.otPacketAddr = otPacketAddr;
    bee4.workBaseAddr = workBaseAddr;
    bee4.bufferIndex = bufferIndex;
    bee4.be34 = action.be34;
    bee4.stackResidue.valid = stackTail.active;
    bee4.stackResidue.stackTail = stackTail;
    bee4.gsSortFastSprite.valid = true;
    bee4.gsSortFastSprite.callerFunction = 0x8001BEE4u;
    bee4.gsSortFastSprite.otStructAddr = otPacketAddr;
    bee4.gsSortFastSprite.priority = 3u;
    bee4.gsSortFastSprite.localSprite =
        BuildRawDrawFastSpriteLocalSub8001BEE4(bee4);
    PrPsxFastSpriteSubmitDirect::FastSpriteRawLocalBytes8003FA20
        stackSeed =
            PrPsxFastSpriteSubmitDirect::
                ApplyStackScratchTailToRawLocalBytes8003FA20(
                    {},
                    stackTail);
    if (!stackSeed.active) {
        stackSeed.provenance =
            PrPsxFastSpriteSubmitDirect::
                FastSpriteRawLocalProvenance8003FA20::
                    PsxStackScratchResidue;
    }
    stackSeed.note =
        "8001BEE4/8001BE34 write local prefix only; RGB tail requires "
        "PSX stack scratch bytes";
    bee4.gsSortFastSprite.rawLocalBytes =
        PrPsxFastSpriteSubmitDirect::
            BuildRawLocalBytesFromKnownFields8003FA20(
                bee4.gsSortFastSprite.localSprite,
                stackSeed,
                0x8001BEE4u,
                0x8001BF1Cu,
                stackSeed.note);
    bee4.gsSortFastSprite.localSpritePrefixKnown = true;
    bee4.gsSortFastSprite.localSpriteRgbKnown =
        PrPsxFastSpriteSubmitDirect::HasKnownRgbTail8003FA20(
            bee4.gsSortFastSprite.rawLocalBytes);
    bee4.gsSortFastSprite.localSpriteRgbGapReason =
        bee4.gsSortFastSprite.localSpriteRgbKnown
            ? Movie1PsxFastSpriteLocalRgbGapReason::None
            : Movie1PsxFastSpriteLocalRgbGapReason::
                  StackLocalTailUnresolved8001BEE4;
    bee4.gsSortFastSprite.localSpriteKnown =
        bee4.gsSortFastSprite.localSpriteRgbKnown;
    bee4.submitImplemented = bee4.gsSortFastSprite.localSpriteKnown;
    return action;
}

PrPsxFastSpriteSubmitDirect::FastSpriteLocalFields8003FA20
BuildRawDrawFastSpriteLocalSub8001B590(
    const Movie1PsxRawDrawB590Metadata& b590) {
    PrPsxFastSpriteSubmitDirect::FastSpriteLocalFields8003FA20 out{};
    out.attr_00 = b590.b25c.copiedAttr;
    out.x_04 = b590.spriteX;
    out.y_06 = static_cast<uint16_t>(b590.spriteY);
    out.width_08 = b590.b25c.width;
    out.height_0A = b590.b25c.height;
    out.tpage_0C = b590.b25c.computedClutWord;
    out.u_0E = b590.b25c.computedUByte;
    out.v_0F = b590.b25c.computedVByte;
    out.clutX_10 = static_cast<int16_t>(b590.b25c.sourceWord12);
    out.clutY_12 = static_cast<int16_t>(b590.b25c.finalWord);
    return out;
}

PrPsxFastSpriteSubmitDirect::FastSpriteLocalFields8003FA20
BuildRawDrawFastSpriteLocalSub8001BEE4(
    const Movie1PsxRawDrawBEE4Metadata& bee4) {
    PrPsxFastSpriteSubmitDirect::FastSpriteLocalFields8003FA20 out{};
    out.attr_00 = bee4.be34.copiedAttr;
    out.x_04 = bee4.spriteX;
    out.y_06 = static_cast<uint16_t>(bee4.spriteY);
    out.width_08 = bee4.be34.width;
    out.height_0A = bee4.be34.height;
    out.tpage_0C = bee4.be34.computedClutWord;
    out.u_0E = bee4.be34.computedUByte;
    out.v_0F = bee4.be34.computedVByte;
    out.clutX_10 = static_cast<int16_t>(bee4.be34.sourceWord12);
    out.clutY_12 = static_cast<int16_t>(bee4.be34.finalWord);
    return out;
}

Movie1RawDrawFastSpriteRuntimeBridgeInput
BuildRawDrawFastSpriteRuntimeBridgeInputSub8001B590(
    const Movie1PsxRawDrawB590Metadata& b590,
    const PrPsxFastSpriteSubmitDirect::RuntimeState8003FA20& runtime,
    const Movie1PsxRawDrawAction* action) {
    Movie1RawDrawFastSpriteRuntimeBridgeInput out{};
    out.valid = b590.valid;
    out.b590 = b590;
    out.runtime = runtime;
    out.provenance.active = true;
    out.provenance.sourceKind =
        ResolveRawDrawFastSpriteSourceKind8003FA20(action);
    out.provenance.sourceFunction = b590.sourceFunction;
    out.provenance.helper = static_cast<uint32_t>(
        Movie1PsxRawDrawHelper::Call8001B590);
    out.provenance.priority = b590.priority;
    if (action != nullptr) {
        out.provenance.helper = static_cast<uint32_t>(action->helper);
        out.provenance.psxCallOrder = action->psxCallOrder;
    }
    return out;
}

static Movie1RawDrawFastSpriteRuntimeBridgeResult
ResolveRawDrawFastSpriteRuntimeBridgeSub8001B590(
    const Movie1RawDrawFastSpriteRuntimeBridgeInput& input,
    bool apply) {
    Movie1RawDrawFastSpriteRuntimeBridgeResult out{};
    out.valid = input.valid && input.b590.valid;
    out.bridgeInput = input;
    out.runtimeAfter = input.runtime;
    if (!out.valid) {
        return out;
    }

    out.submitInput =
        PrPsxFastSpriteSubmitDirect::BuildInputFromRuntime8003FA20(
            input.runtime,
            BuildRawDrawFastSpriteLocalSub8001B590(input.b590),
            input.b590.gsSortFastSprite.rawLocalBytes,
            input.b590.priority);
    out.submitInput.provenance = input.provenance;
    out.applied = apply;
    const bool localSpriteRgbKnown =
        PrPsxFastSpriteSubmitDirect::HasKnownRgbTail8003FA20(
            input.b590.gsSortFastSprite.rawLocalBytes);
    if (localSpriteRgbKnown) {
        out.submitResult =
            PrPsxFastSpriteSubmitDirect::PsxCall8003FA20_GsSortFastSprite(
                out.submitInput);
        out.runtimeUpdate = apply
            ? PrPsxFastSpriteSubmitDirect::ApplyRuntimeUpdate8003FA20(
                  out.runtimeAfter,
                  out.submitResult)
            : PrPsxFastSpriteSubmitDirect::PredictRuntimeUpdate8003FA20(
                  input.runtime,
                  out.submitResult);
        return out;
    }

    out.partial = true;
    out.partialSubmitResult =
        PrPsxFastSpriteSubmitDirect::PredictGsSortFastSpritePartial8003FA20(
            out.submitInput,
            false);
    out.runtimeUpdate = apply
        ? PrPsxFastSpriteSubmitDirect::ApplyRuntimeUpdate8003FA20(
              out.runtimeAfter,
              out.partialSubmitResult)
        : PrPsxFastSpriteSubmitDirect::PredictRuntimeUpdate8003FA20(
              input.runtime,
              out.partialSubmitResult);
    return out;
}

Movie1RawDrawFastSpriteRuntimeBridgeResult
PredictRawDrawFastSpriteRuntimeBridgeSub8001B590(
    const Movie1RawDrawFastSpriteRuntimeBridgeInput& input) {
    return ResolveRawDrawFastSpriteRuntimeBridgeSub8001B590(input, false);
}

Movie1RawDrawFastSpriteRuntimeBridgeResult
ApplyRawDrawFastSpriteRuntimeBridgeSub8001B590(
    const Movie1RawDrawFastSpriteRuntimeBridgeInput& input) {
    return ResolveRawDrawFastSpriteRuntimeBridgeSub8001B590(input, true);
}

Movie1RawDrawFastSpriteRuntimeBridgeInput
BuildRawDrawFastSpriteRuntimeBridgeInputSub8001BEE4(
    const Movie1PsxRawDrawBEE4Metadata& bee4,
    const PrPsxFastSpriteSubmitDirect::RuntimeState8003FA20& runtime,
    const Movie1PsxRawDrawAction* action) {
    Movie1RawDrawFastSpriteRuntimeBridgeInput out{};
    out.valid = bee4.valid;
    out.bee4 = bee4;
    out.runtime = runtime;
    out.provenance.active = true;
    out.provenance.sourceKind =
        ResolveRawDrawFastSpriteSourceKind8003FA20(action);
    out.provenance.sourceFunction = bee4.sourceFunction;
    out.provenance.helper = static_cast<uint32_t>(
        Movie1PsxRawDrawHelper::Call8001BEE4);
    out.provenance.priority = bee4.priority;
    if (action != nullptr) {
        out.provenance.helper = static_cast<uint32_t>(action->helper);
        out.provenance.psxCallOrder = action->psxCallOrder;
    }
    return out;
}

static Movie1RawDrawFastSpriteRuntimeBridgeResult
ResolveRawDrawFastSpriteRuntimeBridgeSub8001BEE4(
    const Movie1RawDrawFastSpriteRuntimeBridgeInput& input,
    bool apply) {
    Movie1RawDrawFastSpriteRuntimeBridgeResult out{};
    out.valid = input.valid && input.bee4.valid;
    out.bridgeInput = input;
    out.runtimeAfter = input.runtime;
    if (!out.valid) {
        return out;
    }

    out.submitInput =
        PrPsxFastSpriteSubmitDirect::BuildInputFromRuntime8003FA20(
            input.runtime,
            BuildRawDrawFastSpriteLocalSub8001BEE4(input.bee4),
            input.bee4.gsSortFastSprite.rawLocalBytes,
            input.bee4.priority);
    out.submitInput.provenance = input.provenance;
    const bool localSpriteRgbKnown =
        PrPsxFastSpriteSubmitDirect::HasKnownRgbTail8003FA20(
            input.bee4.gsSortFastSprite.rawLocalBytes);
    out.applied = apply;
    if (localSpriteRgbKnown) {
        out.submitResult =
            PrPsxFastSpriteSubmitDirect::PsxCall8003FA20_GsSortFastSprite(
                out.submitInput);
        out.runtimeUpdate = apply
            ? PrPsxFastSpriteSubmitDirect::ApplyRuntimeUpdate8003FA20(
                  out.runtimeAfter,
                  out.submitResult)
            : PrPsxFastSpriteSubmitDirect::PredictRuntimeUpdate8003FA20(
                  input.runtime,
                  out.submitResult);
        return out;
    }

    out.partial = true;
    out.partialSubmitResult =
        PrPsxFastSpriteSubmitDirect::PredictGsSortFastSpritePartial8003FA20(
            out.submitInput,
            false);
    out.runtimeUpdate = apply
        ? PrPsxFastSpriteSubmitDirect::ApplyRuntimeUpdate8003FA20(
              out.runtimeAfter,
              out.partialSubmitResult)
        : PrPsxFastSpriteSubmitDirect::PredictRuntimeUpdate8003FA20(
              input.runtime,
              out.partialSubmitResult);
    return out;
}

Movie1RawDrawFastSpriteRuntimeBridgeResult
PredictRawDrawFastSpriteRuntimeBridgeSub8001BEE4(
    const Movie1RawDrawFastSpriteRuntimeBridgeInput& input) {
    return ResolveRawDrawFastSpriteRuntimeBridgeSub8001BEE4(input, false);
}

Movie1RawDrawFastSpriteRuntimeBridgeResult
ApplyRawDrawFastSpriteRuntimeBridgeSub8001BEE4(
    const Movie1RawDrawFastSpriteRuntimeBridgeInput& input) {
    return ResolveRawDrawFastSpriteRuntimeBridgeSub8001BEE4(input, true);
}

Movie1RawDrawTypedSubmitResult
PredictRawDrawTypedSubmit(
    const Movie1PsxRawDrawAction& action,
    const PrPsxFastSpriteSubmitDirect::RuntimeState8003FA20& runtime,
    uint8_t languageIndex) {
    Movie1RawDrawTypedSubmitResult out{};
    Movie1PsxRawDrawResolutionStatus status =
        Movie1PsxRawDrawResolutionStatus::RawOnly;
    Movie1PsxRawDrawAction typedAction{};
    Movie1PsxRawDrawB590Metadata b590{};
    Movie1PsxRawDrawBEE4Metadata bee4{};
    if (!TryBuildRawDrawForTypedSubmit(
            action,
            typedAction,
            b590,
            bee4,
            status,
            languageIndex)) {
        out.status = status;
        out.action = typedAction;
        return out;
    }

    out.valid = true;
    out.status = status;
    out.action = typedAction;
    out.b590 = b590;
    out.bee4 = bee4;
    const bool canSubmit =
        status == Movie1PsxRawDrawResolutionStatus::ResolvedTypedSubmit;
    const bool canPartialFastSprite = IsFastSpriteRgbGapStatus(status);
    if (!canSubmit && !canPartialFastSprite) {
        return out;
    }
    if (typedAction.helper == Movie1PsxRawDrawHelper::Call8001BEE4) {
        out.fastSprite =
            PredictRawDrawFastSpriteRuntimeBridgeSub8001BEE4(
                BuildRawDrawFastSpriteRuntimeBridgeInputSub8001BEE4(
                    bee4,
                    runtime,
                    &typedAction));
    } else {
        out.fastSprite =
            PredictRawDrawFastSpriteRuntimeBridgeSub8001B590(
                BuildRawDrawFastSpriteRuntimeBridgeInputSub8001B590(
                    b590,
                    runtime,
                    &typedAction));
    }
    out.partial = out.fastSprite.partial;
    out.submitted =
        out.fastSprite.valid && !out.fastSprite.partial &&
        !out.fastSprite.submitResult.skipped;
    out.skipped =
        out.fastSprite.valid &&
        (out.fastSprite.partial
             ? out.fastSprite.partialSubmitResult.skipped
             : out.fastSprite.submitResult.skipped);
    return out;
}

Movie1RawDrawTypedSubmitResult
ApplyRawDrawTypedSubmit(
    const Movie1PsxRawDrawAction& action,
    PrPsxFastSpriteSubmitDirect::RuntimeState8003FA20& runtime,
    uint8_t languageIndex) {
    Movie1RawDrawTypedSubmitResult out{};
    Movie1PsxRawDrawResolutionStatus status =
        Movie1PsxRawDrawResolutionStatus::RawOnly;
    Movie1PsxRawDrawAction typedAction{};
    Movie1PsxRawDrawB590Metadata b590{};
    Movie1PsxRawDrawBEE4Metadata bee4{};
    if (!TryBuildRawDrawForTypedSubmit(
            action,
            typedAction,
            b590,
            bee4,
            status,
            languageIndex)) {
        out.status = status;
        out.action = typedAction;
        return out;
    }

    out.valid = true;
    out.applied = true;
    out.status = status;
    out.action = typedAction;
    out.b590 = b590;
    out.bee4 = bee4;
    const bool canSubmit =
        status == Movie1PsxRawDrawResolutionStatus::ResolvedTypedSubmit;
    const bool canPartialFastSprite = IsFastSpriteRgbGapStatus(status);
    if (!canSubmit && !canPartialFastSprite) {
        return out;
    }
    if (typedAction.helper == Movie1PsxRawDrawHelper::Call8001BEE4) {
        out.fastSprite =
            ApplyRawDrawFastSpriteRuntimeBridgeSub8001BEE4(
                BuildRawDrawFastSpriteRuntimeBridgeInputSub8001BEE4(
                    bee4,
                    runtime,
                    &typedAction));
    } else {
        out.fastSprite =
            ApplyRawDrawFastSpriteRuntimeBridgeSub8001B590(
                BuildRawDrawFastSpriteRuntimeBridgeInputSub8001B590(
                    b590,
                    runtime,
                    &typedAction));
    }
    runtime = out.fastSprite.runtimeAfter;
    out.partial = out.fastSprite.partial;
    out.submitted =
        out.fastSprite.valid && !out.fastSprite.partial &&
        !out.fastSprite.submitResult.skipped;
    out.skipped =
        out.fastSprite.valid &&
        (out.fastSprite.partial
             ? out.fastSprite.partialSubmitResult.skipped
             : out.fastSprite.submitResult.skipped);
    return out;
}

SubtitleFrameWindow ScanSubtitleFrameWindow(
    const PrMovieSubtitles::MovieSubtitleTrack& track) {
    return SelectMovieTextDescSub80024C84(track);
}

SubtitleFrameWindow SelectMovieTextDescSub80024C84(
    const PrMovieSubtitles::MovieSubtitleTrack& track) {
    SubtitleFrameWindow out{};
    out.descOffset = track.descOffset;
    if (!track.loaded || track.lines.empty()) {
        return out;
    }

    // PSX-confirmed MOVIE1 order:
    // first screen has no frame/text, then the subtitle frame appears, then
    // the first subtitle text appears.
    uint32_t firstTextFrame = UINT32_MAX;
    uint32_t lastTextEndFrame = 0;
    for (const PrMovieSubtitles::MovieSubtitleLine& line : track.lines) {
        bool any = false;
        for (int i = 0; i < 5; ++i) {
            if (!line.texts[i].empty()) {
                any = true;
                break;
            }
        }
        if (!any) {
            continue;
        }
        if (line.frame30 < firstTextFrame) {
            firstTextFrame = line.frame30;
        }
        const uint32_t lineEnd = line.frame30 + (uint32_t)line.duration;
        if (lineEnd > lastTextEndFrame) {
            lastTextEndFrame = lineEnd;
        }
    }
    if (firstTextFrame == UINT32_MAX || lastTextEndFrame <= firstTextFrame) {
        return out;
    }

    out.valid = true;
    out.firstTextFrame = firstTextFrame;
    out.startFrame = (firstTextFrame > 0) ? (firstTextFrame - 1) : 0;
    out.endFrame = lastTextEndFrame;
    return out;
}

const char* ResolveMovieTextSub80024CF8(
    const PrMovieSubtitles::MovieSubtitleTrack& track,
    uint32_t frame30,
    int lang) {
    if (!track.loaded || track.lines.empty()) {
        return nullptr;
    }
    if (lang < 0 || lang >= 5) {
        lang = 0;
    }

    for (const PrMovieSubtitles::MovieSubtitleLine& line : track.lines) {
        const uint32_t end = line.frame30 + (uint32_t)line.duration;
        if (frame30 < line.frame30 || frame30 >= end) {
            continue;
        }
        if (!line.texts[lang].empty()) {
            return line.texts[lang].c_str();
        }
        if (!line.texts[0].empty()) {
            return line.texts[0].c_str();
        }
        for (int i = 0; i < 5; ++i) {
            if (!line.texts[i].empty()) {
                return line.texts[i].c_str();
            }
        }
    }
    return nullptr;
}

bool IsMovieTextFrameVisibleSub80024CF8(uint32_t movieFrame30,
                                        const SubtitleFrameWindow& window,
                                        bool textVisible) {
    return (window.valid && movieFrame30 >= window.startFrame) || textVisible;
}

Movie1PsxPhase ResolveMovie1PsxPhaseSub801C8270(
    const Movie1RuntimeState& state,
    bool stageLoopActive) {
    if (state.outroActive || state.transitionSub800201ACActive ||
        state.outroTailFrames > 0u ||
        state.outroTailCompletePending || state.skipPreludeFrames > 0u) {
        return Movie1PsxPhase::MovieOutroSub80020308;
    }
    if (state.strStarted && !state.strPlayed) {
        return Movie1PsxPhase::MovieStrSub801C77C0;
    }
    if (stageLoopActive) {
        return Movie1PsxPhase::StageLoopSub801C7A60;
    }
    return Movie1PsxPhase::Idle;
}

TransitionSub800201ACStep ResolveTransitionSub800201ACStep(
    uint32_t gp196,
    uint32_t tailFramesRemaining,
    uint32_t mode) {
    TransitionSub800201ACStep out{};
    out.phase = (mode > 0u &&
                 mode <= (uint32_t)TransitionSub800201ACPhase::Complete)
        ? (TransitionSub800201ACPhase)mode
        : (gp196 < kSub800201ACLoopFrames
            ? TransitionSub800201ACPhase::LoopSub8001EA74
            : (tailFramesRemaining > 0u
                ? TransitionSub800201ACPhase::TailSub80020090
                : TransitionSub800201ACPhase::Complete));
    out.gp196 = gp196;
    out.nextGp196 = gp196;
    out.gp792 = 0;
    out.nextGp792 = 0;
    out.gp872 = 0;
    out.nextGp872 = 0;
    out.tailFramesRemaining = tailFramesRemaining;
    out.tailIteration = (tailFramesRemaining <= kSub800201ACTailFrames)
        ? (kSub800201ACTailFrames - tailFramesRemaining)
        : 0u;

    switch (out.phase) {
        case TransitionSub800201ACPhase::InitialSub8001FFD4:
            AppendTransitionSub800201ACAction(
                out,
                TransitionSub800201ACAction::Sub8001FFD4_ResetGp196AndInitMask);
            AppendSub8001FFD4InitMaskMutation(
                out,
                kSub800201ACPostMoviePreFfd4Arg);
            out.nextGp196 = 0;
            break;
        case TransitionSub800201ACPhase::LoopSub8001EA74:
            out = ResolveTransitionSub8001EA74Step(
                0u,
                (uint32_t)TransitionSub8001EA74Mode::Mode1_Sub8001F524,
                gp196,
                0u,
                0u,
                false,
                0u);
            AppendTransitionSub800201ACAction(
                out,
                TransitionSub800201ACAction::Sub80035560_WaitGpu2);
            AppendSub8001EBF4Actions(out);
            AppendTransitionSub800201ACAction(
                out,
                TransitionSub800201ACAction::Sub8001F518_CheckGp196Lt191);
            out.phase = TransitionSub800201ACPhase::LoopSub8001EA74;
            out.tailFramesRemaining = tailFramesRemaining;
            out.tailIteration = (tailFramesRemaining <= kSub800201ACTailFrames)
                ? (kSub800201ACTailFrames - tailFramesRemaining)
                : 0u;
            break;
        case TransitionSub800201ACPhase::EndSub8001FFD4:
            AppendTransitionSub800201ACAction(
                out,
                TransitionSub800201ACAction::Sub8001FFD4_ResetGp196AndInitMask);
            AppendSub8001FFD4InitMaskMutation(
                out,
                kSub800201ACPostMoviePreFfd4Arg);
            AppendTransitionSub800201ACAction(
                out,
                TransitionSub800201ACAction::SetGp196To190);
            out.nextGp196 = kSub800201ACLoopFrames - 1u;
            break;
        case TransitionSub800201ACPhase::TailSub80020090:
            out = ResolveTransitionSub8001EA74Step(
                0u,
                (uint32_t)TransitionSub8001EA74Mode::Mode2_Sub8001FCBC_Sub8001FDC0,
                gp196,
                0u,
                0u,
                false,
                0u);
            out.tailFrame = true;
            AppendTransitionSub800201ACAction(
                out,
                TransitionSub800201ACAction::Sub80035560_WaitGpu2);
            AppendSub8001EBF4Actions(out);
            AppendTransitionSub800201ACAction(
                out,
                TransitionSub800201ACAction::Sub80020090_TailFrame);
            out.phase = TransitionSub800201ACPhase::TailSub80020090;
            out.tailFramesRemaining = tailFramesRemaining;
            out.tailIteration = (tailFramesRemaining <= kSub800201ACTailFrames)
                ? (kSub800201ACTailFrames - tailFramesRemaining)
                : 0u;
            break;
        case TransitionSub800201ACPhase::Complete:
            out.complete = true;
            break;
        case TransitionSub800201ACPhase::Idle:
        default:
            out.complete = gp196 >= kSub800201ACLoopFrames &&
                           tailFramesRemaining == 0u;
            break;
    }
    return out;
}

TransitionSub800201ACStep ResolveTransitionSub8001EA74Step(
    uint32_t a1,
    uint32_t mode,
    uint32_t gp196,
    uint32_t gp792,
    uint32_t gp872,
    bool word800916DC,
    uint32_t word80096590) {
    TransitionSub800201ACStep out{};
    out.drawFrame = true;
    out.phase = TransitionSub800201ACPhase::LoopSub8001EA74;
    out.mode = mode >= 1u && mode <= 6u
        ? (TransitionSub8001EA74Mode)mode
        : TransitionSub8001EA74Mode::Default;
    out.a1 = a1;
    out.a2 = mode;
    out.gp196 = gp196;
    out.nextGp196 = gp196;
    out.gp792 = gp792;
    out.nextGp792 = 1u;
    out.gp872 = gp872;
    out.nextGp872 = word80096590;

    AppendSub8001EA74CommonActions(out);

    switch (out.mode) {
        case TransitionSub8001EA74Mode::Mode1_Sub8001F524:
            AppendSub8001F524DirectEffects(out, 0u, 8u);
            out.nextGp196 = AdvanceGp196BySub8001F524(gp196, 8u);
            out.psxResult = 0u;
            out.drawHelper = Movie1PsxDrawHelper::Sub800201AC_TransitionFrame;
            break;
        case TransitionSub8001EA74Mode::Mode2_Sub8001FCBC_Sub8001FDC0:
            out.nextGp792 = 0u;
            AppendSub8001FCBCDirectEffects(out, 8u, 4u);
            out.nextGp196 = AdvanceGp196BySub8001FCBCOrSub8001FC40(gp196, 8u);
            AppendSub8001FDC0RawDrawActions(out, 0u);
            AppendTransitionTileMaskMutation(
                out,
                TransitionTileMaskMutationKind::DrawActiveMask,
                out.nextGp196,
                16u * 12u,
                0u,
                0u,
                0u);
            out.psxResult = 0u;
            out.drawHelper = Movie1PsxDrawHelper::Sub800201AC_TransitionFrame;
            break;
        case TransitionSub8001EA74Mode::Mode3_Sub80022CBC_Sub8001D74C_Sub8001FC40_Sub8001FDC0:
            AppendSub80022CBCType4RawDrawActions(out, a1);
            AppendSub8001D74CRawDrawActions(out, 5u, out.nextGp872);
            AppendSub8001FC40DirectEffects(out, 8u, 4u);
            out.nextGp196 = AdvanceGp196BySub8001FCBCOrSub8001FC40(gp196, 8u);
            AppendSub8001FDC0RawDrawActions(out, 0u);
            AppendTransitionTileMaskMutation(
                out,
                TransitionTileMaskMutationKind::DrawActiveMask,
                out.nextGp196,
                16u * 12u,
                0u,
                0u,
                0u);
            out.psxResult = 0u;
            out.drawHelper = Movie1PsxDrawHelper::Sub800201AC_TransitionFrame;
            break;
        case TransitionSub8001EA74Mode::Mode4_Sub80021E60_Sub8001D74C_Sub8001FC40_Sub8001FDC0:
            AppendSub80021E60Ctx0RawDrawActions(out);
            AppendSub8001D74CRawDrawActions(out, 5u, out.nextGp872);
            AppendSub8001FC40DirectEffects(out, 8u, 4u);
            out.nextGp196 = AdvanceGp196BySub8001FCBCOrSub8001FC40(gp196, 8u);
            AppendSub8001FDC0RawDrawActions(out, 0u);
            AppendTransitionTileMaskMutation(
                out,
                TransitionTileMaskMutationKind::DrawActiveMask,
                out.nextGp196,
                16u * 12u,
                0u,
                0u,
                0u);
            out.psxResult = 0u;
            out.drawHelper = Movie1PsxDrawHelper::Sub800201AC_TransitionFrame;
            break;
        case TransitionSub8001EA74Mode::Mode5_Sub80020308: {
            AppendTransitionSub800201ACAction(
                out,
                TransitionSub800201ACAction::Sub80020308_Mode5OutroStep);
            uint32_t nextGp196 = gp196;
            const OutroSub80020308Step step =
                AdvanceOutroSub80020308(word800916DC, nextGp196);
            out.nextGp196 = step.nextGp196;
            out.nextGp792 = (uint32_t)step.psxResult;
            out.psxResult = (uint32_t)step.psxResult;
            out.drawHelper = step.drawHelper;
            out.complete = step.complete;
            if (step.playCue0) {
                AppendTransitionSub800201ACAction(
                    out,
                    TransitionSub800201ACAction::Sub800271E4_PlayCue0);
            }
            if (step.playCue1) {
                AppendTransitionSub800201ACAction(
                    out,
                    TransitionSub800201ACAction::Sub800271E4_PlayCue1);
            }
            AppendTransitionSub800201ACAction(
                out,
                step.drawHelper == Movie1PsxDrawHelper::Sub8001FEB4_FinalNoVideoFrame
                    ? TransitionSub800201ACAction::Sub8001FEB4_FinalNoVideoFrame
                    : TransitionSub800201ACAction::Sub8001F230_OutroNoSubboxFrame);
            break;
        }
        case TransitionSub8001EA74Mode::Mode6_Sub80020248: {
            AppendTransitionSub800201ACAction(
                out,
                TransitionSub800201ACAction::Sub80020248_Mode6OutroStep);
            uint32_t nextGp196 = gp196;
            const OutroSub80020308Step step =
                AdvanceOutroSub80020248(word800916DC, nextGp196);
            if (step.playCue1) {
                AppendTransitionSub800201ACAction(
                    out,
                    TransitionSub800201ACAction::Sub800271E4_PlayCue1);
            }
            if (step.playCue0) {
                AppendTransitionSub800201ACAction(
                    out,
                    TransitionSub800201ACAction::Sub800271E4_PlayCue0);
            }
            out.nextGp196 = step.nextGp196;
            out.psxResult = (uint32_t)step.psxResult;
            out.drawHelper = step.drawHelper;
            out.complete = step.complete;
            AppendTransitionSub800201ACAction(
                out,
                step.drawHelper == Movie1PsxDrawHelper::Sub8001C864_SubtitleFrame
                    ? TransitionSub800201ACAction::Sub8001C864_Mode6SubtitleFrame
                    : (step.drawHelper == Movie1PsxDrawHelper::Sub8001F230_OutroNoSubboxFrame
                        ? TransitionSub800201ACAction::Sub8001F230_OutroNoSubboxFrame
                        : TransitionSub800201ACAction::Sub8001CE30_Mode6NoSubtitleFrame));
            break;
        }
        case TransitionSub8001EA74Mode::Default:
        default:
            if (a1 != 0u) {
                AppendTransitionSub800201ACAction(
                    out,
                    TransitionSub800201ACAction::Sub8001EF40_DefaultScrollMask);
            }
            out.psxResult = 1u;
            break;
    }
    return out;
}

static TransitionSub800201ACStep ResolveTransitionSub800201ACStepWithArgs(
    uint32_t a1,
    uint32_t modeA2,
    uint32_t preFfd4ArgA3,
    uint32_t postFfd4ArgA4,
    uint32_t gp196,
    uint32_t gp792,
    uint32_t gp872,
    uint32_t tailFramesRemaining,
    uint32_t phaseMode,
    uint32_t tailIteration,
    const Movie1TransitionCtxWords801C3640& ctxWords,
    bool variant80020110,
    bool word800916DC,
    uint32_t word80096590) {
    TransitionSub800201ACStep out{};
    out.phase = (phaseMode > 0u &&
                 phaseMode <= (uint32_t)TransitionSub800201ACPhase::Complete)
        ? (TransitionSub800201ACPhase)phaseMode
        : (gp196 < kSub800201ACLoopFrames
            ? TransitionSub800201ACPhase::LoopSub8001EA74
            : (tailFramesRemaining > 0u
                ? TransitionSub800201ACPhase::TailSub80020090
                : TransitionSub800201ACPhase::Complete));
    out.a1 = a1;
    out.a2 = modeA2;
    out.a3 = preFfd4ArgA3;
    out.a4 = postFfd4ArgA4;
    out.gp196 = gp196;
    out.nextGp196 = gp196;
    out.gp792 = gp792;
    out.nextGp792 = gp792;
    out.gp872 = gp872;
    out.nextGp872 = gp872;
    out.tailFramesRemaining = tailFramesRemaining;
    out.tailIteration = tailIteration;
    out.ctxWords801C3640 = ctxWords;

    switch (out.phase) {
        case TransitionSub800201ACPhase::InitialSub8001FFD4:
            AppendTransitionSub800201ACAction(
                out,
                TransitionSub800201ACAction::Sub8001FFD4_ResetGp196AndInitMask);
            AppendSub8001FFD4InitMaskMutation(out, preFfd4ArgA3);
            out.nextGp196 = 0;
            break;
        case TransitionSub800201ACPhase::LoopSub8001EA74:
            out = ResolveTransitionSub8001EA74Step(
                a1,
                modeA2,
                gp196,
                gp792,
                gp872,
                word800916DC,
                word80096590);
            out.ctxWords801C3640 = ctxWords;
            out.a3 = preFfd4ArgA3;
            out.a4 = postFfd4ArgA4;
            if (variant80020110) {
                AppendTransitionSub800201ACAction(
                    out,
                    TransitionSub800201ACAction::
                        Sub80027194_Cue8006EC18AndFlush);
            }
            AppendTransitionSub800201ACAction(
                out,
                TransitionSub800201ACAction::Sub80035560_WaitGpu2);
            AppendSub8001EBF4Actions(out);
            AppendTransitionSub800201ACAction(
                out,
                TransitionSub800201ACAction::Sub8001F518_CheckGp196Lt191);
            out.phase = TransitionSub800201ACPhase::LoopSub8001EA74;
            out.tailFramesRemaining = tailFramesRemaining;
            out.tailIteration = tailIteration;
            break;
        case TransitionSub800201ACPhase::EndSub8001FFD4:
            AppendTransitionSub800201ACAction(
                out,
                TransitionSub800201ACAction::Sub8001FFD4_ResetGp196AndInitMask);
            AppendSub8001FFD4InitMaskMutation(out, postFfd4ArgA4);
            if (variant80020110) {
                out.nextGp196 = 0;
            } else {
                AppendTransitionSub800201ACAction(
                    out,
                    TransitionSub800201ACAction::SetGp196To190);
                out.nextGp196 = kSub800201ACLoopFrames - 1u;
            }
            break;
        case TransitionSub800201ACPhase::TailSub80020090:
            out = ResolveTransitionSub8001EA74Step(
                a1,
                modeA2,
                gp196,
                gp792,
                gp872,
                word800916DC,
                word80096590);
            out.ctxWords801C3640 = ctxWords;
            out.a3 = preFfd4ArgA3;
            out.a4 = postFfd4ArgA4;
            out.tailFrame = true;
            if (variant80020110) {
                AppendTransitionSub800201ACAction(
                    out,
                    TransitionSub800201ACAction::
                        Sub80027194_Cue8006EC18AndFlush);
            }
            AppendTransitionSub800201ACAction(
                out,
                TransitionSub800201ACAction::Sub80035560_WaitGpu2);
            AppendSub8001EBF4Actions(out);
            AppendTransitionSub800201ACAction(
                out,
                variant80020110
                    ? TransitionSub800201ACAction::Sub80020008_TailFrame
                    : TransitionSub800201ACAction::Sub80020090_TailFrame);
            out.phase = TransitionSub800201ACPhase::TailSub80020090;
            out.tailFramesRemaining = tailFramesRemaining;
            out.tailIteration = tailIteration;
            break;
        case TransitionSub800201ACPhase::Complete:
            out.complete = true;
            break;
        case TransitionSub800201ACPhase::Idle:
        default:
            out.complete = gp196 >= kSub800201ACLoopFrames &&
                           tailFramesRemaining == 0u;
            break;
    }
    return out;
}

TransitionSub800201ACStep ResolveTransitionSub800201ACStep(
    const Movie1RuntimeState& state,
    bool word800916DC) {
    const uint32_t word80096590 =
        PrPsxGraphOwnerDirect::PsxCall8004019C_GetDrawBuffer(
            state.graphOwner8003FB9C);
    if (state.transitionSub800201ACActive) {
        TransitionSub800201ACStep step = ResolveTransitionSub800201ACStepWithArgs(
            state.transitionSub800201ACA1,
            state.transitionSub800201ACA2,
            state.transitionSub800201ACA3,
            state.transitionSub800201ACA4,
            state.outroGp196,
            state.transitionGp792,
            state.transitionGp872,
            state.outroTailFrames,
            (uint32_t)state.transitionSub800201ACPhase,
            state.transitionSub80020090TailIndex,
            state.transitionCtxWords801C3640,
            state.transitionSub80020110Variant,
            word800916DC,
            word80096590);
        step.activeTileMask = state.transitionTileMask;
        return step;
    }
    TransitionSub800201ACStep step = ResolveTransitionSub800201ACStep(
        state.outroGp196,
        state.outroTailFrames,
        state.outroTailCompletePending
            ? (uint32_t)TransitionSub800201ACPhase::Complete
            : (uint32_t)TransitionSub800201ACPhase::Idle);
    step.activeTileMask = state.transitionTileMask;
    return step;
}

TransitionSub800201ACStep ResolveTransitionSub800201ACStep(
    const Movie1RuntimeState& state) {
    return ResolveTransitionSub800201ACStep(state, false);
}

Movie1RuntimeState::Movie1RuntimeState()
    : graphOwner8003FB9C(MakeMovie1GraphOwner8003FB9C()) {}

void ResetRuntime(Movie1RuntimeState& state) {
    state = Movie1RuntimeState{};
}

void ClearCurrentMovieFrame30(Movie1RuntimeState& state) {
    ClearMovieFrame30(state);
}

void SetSubtitleWindow(Movie1RuntimeState& state,
                       const SubtitleFrameWindow& window) {
    state.subtitleWindow = window;
}

void SetTransitionCtxWords801C3640(
    Movie1RuntimeState& state,
    const Movie1TransitionCtxWords801C3640& ctxWords) {
    state.transitionCtxWords801C3640 = ctxWords;
}

void SetRawDrawFastSpriteRuntime8003FA20(
    Movie1RuntimeState& state,
    const PrPsxFastSpriteSubmitDirect::RuntimeState8003FA20& runtime) {
    state.rawDrawFastSpriteRuntimeOwnerValid = true;
    state.rawDrawFastSpriteRuntime = runtime;
}

void ClearRawDrawFastSpriteRuntime8003FA20(Movie1RuntimeState& state) {
    state.rawDrawFastSpriteRuntimeOwnerValid = false;
    state.rawDrawFastSpriteRuntime =
        PrPsxFastSpriteSubmitDirect::RuntimeState8003FA20{};
}

bool SyncRawDrawFastSpriteRuntimeOwner8003FA20(Movie1RuntimeState& state) {
    // Replay truth: stage1二周目_无cool_good评级通关3.psxrec frames 9041/9168.
    // 8001EA74 selects 8004019C(), then 80040F90(dword_8006ED50[slot])
    // before the first 8003FA20. Do not use the frame-end dword_800901C8.
    // word_800965A0 == 4 in the same replay keeps 800401AC offsets at 0/0.
    PrPsxFastSpriteSubmitDirect::RuntimeState8003FA20 runtime{};
    const PrPsxGraphOwnerDirect::PsxFastSpriteRuntimeBuildResult8003FA20
        result = PrPsxGraphOwnerDirect::
            BuildMovie1SmallOtRuntimeState8003FA20FromGraphOwner(
            state.graphOwner8003FB9C,
            runtime);
    if (!result.built) {
        ClearRawDrawFastSpriteRuntime8003FA20(state);
        return false;
    }

    SetRawDrawFastSpriteRuntime8003FA20(state, runtime);
    return true;
}

static bool AppendRuntimePacketWrites8003FA20(
    PrPsxFastSpriteSubmitDirect::RuntimeState8003FA20& dst,
    const PrPsxFastSpriteSubmitDirect::RuntimeState8003FA20& src) {
    bool appendedAny = false;
    for (const PrPsxFastSpriteSubmitDirect::RuntimePacketWrite8003FA20& write :
         src.packetWrites) {
        if (!write.valid) {
            continue;
        }
        bool copied = false;
        for (PrPsxFastSpriteSubmitDirect::RuntimePacketWrite8003FA20& slot :
             dst.packetWrites) {
            if (slot.valid) {
                continue;
            }
            slot = write;
            slot.valid = true;
            copied = true;
            appendedAny = true;
            break;
        }
        if (!copied) {
            return appendedAny;
        }
    }
    return appendedAny;
}

static uint32_t CountRuntimePacketWrites8003FA20(
    const PrPsxFastSpriteSubmitDirect::RuntimeState8003FA20& runtime) {
    uint32_t count = 0;
    for (const PrPsxFastSpriteSubmitDirect::RuntimePacketWrite8003FA20& write :
         runtime.packetWrites) {
        if (write.valid) {
            ++count;
        }
    }
    return count;
}

static bool IsTailCommonLyricsTextAddrSub801C77C0(uint32_t psxAddr) {
    return psxAddr >= 0x801C5000u && psxAddr < 0x801C5200u;
}

bool PrepareMovie1DrawRuntimeSub801C77C0(
    Movie1RuntimeState& state,
    PrStage1MovieTextDirect::Movie1TextWindowTickResult& textWindowTick) {
    const bool rawRuntimeOwnerKnown =
        SyncRawDrawFastSpriteRuntimeOwner8003FA20(state);
    const bool textDisplayRequested =
        textWindowTick.displayActionSub8001EC54Ctx7.requested;
    if (textDisplayRequested) {
        PrPsxFastSpriteSubmitDirect::RuntimeState8003FA20 textRuntime{};
        const PrPsxGraphOwnerDirect::PsxFastSpriteRuntimeBuildResult8003FA20
            textBuild =
                PrPsxGraphOwnerDirect::
                    BuildMovie1TextMainRuntimeState8003FA20FromGraphOwner(
                        state.graphOwner8003FB9C,
                        textRuntime);
        const uint32_t rawPacketCountBefore =
            CountRuntimePacketWrites8003FA20(state.rawDrawFastSpriteRuntime);
        const PrStage1MovieTextDirect::
            Movie1TextFastSpriteSequenceApplyResultSub8001B954 apply =
                PrStage1MovieTextDirect::
                    ApplyMovieTextWindowSubmitSub801C77C0(
            textWindowTick,
            state.graphOwner8003FB9C,
            textBuild.built ? &textRuntime : nullptr);
        const uint32_t textPacketCount =
            CountRuntimePacketWrites8003FA20(textRuntime);
        if (textBuild.built) {
            if (rawRuntimeOwnerKnown) {
                (void)AppendRuntimePacketWrites8003FA20(
                    state.rawDrawFastSpriteRuntime,
                    textRuntime);
            } else {
                SetRawDrawFastSpriteRuntime8003FA20(state, textRuntime);
            }
        }
        const PrStage1MovieTextDirect::Movie1TextFastSpriteSequenceSub8001B954&
            sequence = textWindowTick.displayActionSub8001EC54Ctx7
                           .textFastSpriteSequenceSub8001B954;
        static uint32_t s_tailTextTraceCount = 0;
        if (s_tailTextTraceCount < 24u &&
            IsTailCommonLyricsTextAddrSub801C77C0(sequence.textPsxAddr)) {
            ++s_tailTextTraceCount;
            Log::Printf(
                "[TailNativeTextApply] text=0x%08X requested=%d helper=%u "
                "textBuild=%d rawBuild=%d seqValid=%d strlen=%u "
                "glyphSubmit=%u applied=%u textPackets=%u rawBefore=%u "
                "rawAfter=%u firstGap=%u currentSlotKnown=%d currentSlot=%u",
                sequence.textPsxAddr,
                textDisplayRequested ? 1 : 0,
                static_cast<unsigned>(
                    textWindowTick.displayActionSub8001EC54Ctx7.helper),
                textBuild.built ? 1 : 0,
                rawRuntimeOwnerKnown ? 1 : 0,
                sequence.valid ? 1 : 0,
                static_cast<unsigned>(sequence.strlenBytes),
                static_cast<unsigned>(sequence.glyphSubmitResultCount),
                static_cast<unsigned>(apply.appliedSubmitCount),
                textPacketCount,
                rawPacketCountBefore,
                CountRuntimePacketWrites8003FA20(
                    state.rawDrawFastSpriteRuntime),
                static_cast<unsigned>(sequence.firstGap),
                sequence.currentWork.gp872SlotKnown ? 1 : 0,
                static_cast<unsigned>(sequence.currentWork.gp872Slot));
        }
        return rawRuntimeOwnerKnown || textBuild.built;
    }

    (void)PrStage1MovieTextDirect::ApplyMovieTextWindowSubmitSub801C77C0(
        textWindowTick,
        state.graphOwner8003FB9C,
        nullptr);
    return rawRuntimeOwnerKnown;
}

bool TryGetDrawBufferWord80096590(const Movie1RuntimeState& state,
                                  uint8_t& out) {
    out = static_cast<uint8_t>(
        PrPsxGraphOwnerDirect::PsxCall8004019C_GetDrawBuffer(
            state.graphOwner8003FB9C));
    return out <= 1u;
}

void ApplyTransitionStateMovieBoundaryPrelude(Movie1RuntimeState& state,
                                               uint16_t transitionState916D0) {
    if (transitionState916D0 != 1u && transitionState916D0 != 2u) {
        return;
    }
    state.strPlayed = true;
    state.strStarted = false;
}

void BeginOutro(Movie1RuntimeState& state,
                uint32_t movieFrame30,
                bool drainStrAudio) {
    if (state.outroActive) {
        return;
    }
    state.outroActive = true;
    state.outroDrainStrAudio = drainStrAudio;
    state.outroCue0Played = false;
    state.outroCue1Played = false;
    state.outroGp196 = 0;
    state.outroSourceFrame30 = movieFrame30;
    state.outroTailFrames = 0;
    state.outroTailCompletePending = false;
    state.transitionGp792 = 0;
    state.transitionGp872 = 0;
    state.transitionSub800201ACActive = false;
    state.transitionSub800201ACCompletesMovie = true;
    state.transitionSub800201ACCompleted = false;
    state.transitionSub800201ACPhase = TransitionSub800201ACPhase::Idle;
    state.transitionSub80020090TailIndex = 0;
    state.transitionSub80020110Variant = false;
    state.transitionTileMask = Movie1TransitionTileMaskRuntimeState{};
    ClearTransitionDrawStep(state);
    state.skipPreludeFrames = 0;
    state.skipPreludeSourceFrame30 = 0;
}

void BeginSkipPrelude(Movie1RuntimeState& state, uint32_t movieFrame30) {
    if (state.outroActive) {
        return;
    }
    state.skipPreludeFrames = kSkipOutroLeadInFrames;
    state.skipPreludeSourceFrame30 = movieFrame30;
}

bool TickSkipPrelude(Movie1RuntimeState& state) {
    if (state.outroActive || state.skipPreludeFrames == 0) {
        return false;
    }
    --state.skipPreludeFrames;
    if (state.skipPreludeFrames == 0) {
        const uint32_t sourceFrame = state.skipPreludeSourceFrame30;
        state.skipPreludeSourceFrame30 = 0;
        BeginOutro(state, sourceFrame, false);
    }
    return true;
}

bool IsSkipPreludeActive(const Movie1RuntimeState& state) {
    return !state.outroActive && state.skipPreludeFrames > 0;
}

Movie1DrawableStateQueryResult QueryDrawableState(
    const Movie1RuntimeState& state) {
    Movie1DrawableStateQueryResult result{};
    result.strFrameActive = state.strStarted;
    result.outroActive = state.outroActive;
    result.outroTailActive =
        state.transitionSub800201ACActive ||
        state.outroTailFrames > 0u ||
        state.outroTailCompletePending;
    result.skipPreludeActive = IsSkipPreludeActive(state);
    result.drawableActive =
        result.strFrameActive ||
        result.outroActive ||
        result.outroTailActive ||
        result.skipPreludeActive;
    return result;
}

Movie1HostStrPollPlan BuildHostStrPollPlan(
    const Movie1RuntimeState& state,
    const Movie1HostFeedback& host) {
    Movie1HostStrPollPlan plan{};
    plan.strPlayerReady = host.strPlayerReady;
    plan.preludeActive = IsSkipPreludeActive(state);
    plan.tailActive =
        state.outroTailCompletePending || state.outroTailFrames > 0u;
    plan.outroDrainStrAudio =
        !state.outroActive || state.outroDrainStrAudio;
    plan.shouldUpdateStr =
        host.strPlayerReady &&
        state.strStarted &&
        !state.strPlayed &&
        !plan.preludeActive &&
        !plan.tailActive &&
        plan.outroDrainStrAudio;
    const bool psxSkipRequested =
        host.inputMaskSub80035510Known &&
        host.inputMaskSub80035510 == kMovie1StrSkipInputMaskSub80035510;
    plan.skipAllowed =
        plan.shouldUpdateStr &&
        !state.outroActive &&
        (host.debugF1StrSkipRequested || psxSkipRequested);
    return plan;
}

Movie1HostActionFeedbackResolution ApplyHostActionFeedback(
    Movie1RuntimeState& state,
    const Movie1AdvanceResult& advance,
    const Movie1HostActionFeedback& feedback) {
    Movie1HostActionFeedbackResolution out{};
    out.advanceResult = advance;
    if (feedback.kind != Movie1HostActionFeedbackKind::PlayMovie1Str ||
        !feedback.playAttempted) {
        return out;
    }

    if (!feedback.playSucceeded) {
        state.strStarted = false;
        out.advanceResult = Movie1AdvanceResult{};
        return out;
    }

    for (uint32_t i = feedback.actionIndex + 1u;
         i < advance.hostActions.count &&
         out.followupHostActions.count <
             static_cast<uint32_t>(
                 sizeof(out.followupHostActions.actions) /
                 sizeof(out.followupHostActions.actions[0]));
         ++i) {
        out.followupHostActions.actions[out.followupHostActions.count++] =
            advance.hostActions.actions[i];
    }
    return out;
}

OutroSub80020308Step ResolveOutroSub80020308Step(
    const Movie1RuntimeState& state,
    bool word800916DC) {
    return ResolveOutroSub80020308Step(word800916DC, state.outroGp196);
}

OutroSub80020308Step AdvanceOutroSub80020308(Movie1RuntimeState& state,
                                             bool word800916DC) {
    OutroSub80020308Step out = AdvanceOutroSub80020308(word800916DC, state.outroGp196);
    if (out.complete) {
        state.outroActive = false;
        state.outroDrainStrAudio = false;
        state.outroCue0Played = false;
        state.outroCue1Played = false;
        state.outroTailFrames = 0;
        state.outroTailCompletePending = false;
        state.transitionGp792 = 0;
        state.transitionGp872 = 0;
        state.transitionSub800201ACActive = false;
        state.transitionSub800201ACCompletesMovie = true;
        state.transitionSub800201ACCompleted = false;
        state.transitionSub800201ACPhase = TransitionSub800201ACPhase::Idle;
        state.transitionSub80020090TailIndex = 0;
        state.transitionSub80020110Variant = false;
        state.transitionTileMask = Movie1TransitionTileMaskRuntimeState{};
    }
    return out;
}

OutroSub80020308Step ResolveOutroSub80020308Step(bool word800916DC,
                                                 uint32_t gp196) {
    OutroSub80020308Step out{};
    out.playCue0 = gp196 == 0u;
    if (word800916DC) {
        out.playCue1 = gp196 == kSub80020308SubtitleHoldFrames;
        out.keepVideoFrame = gp196 < kSub80020308SubtitleHoldFrames;
        out.noVideoFrame = gp196 >= kSub80020308SubtitleHoldFrames;
        out.complete = gp196 >= kSub80020308SubtitleCompleteFrames;
        out.psxResult = (gp196 >= kSub80020308SubtitleHoldFrames) ? 1 : 0;
        out.nextGp196 = out.complete ? 192u : gp196;
        out.drawHelper = out.noVideoFrame
            ? Movie1PsxDrawHelper::Sub8001FEB4_FinalNoVideoFrame
            : Movie1PsxDrawHelper::Sub8001F230_OutroNoSubboxFrame;
    } else {
        out.noVideoFrame = true;
        out.complete = gp196 >= kSub80020308NoSubtitleCompleteFrames;
        out.psxResult = 0;
        out.nextGp196 = out.complete ? 192u : gp196;
        out.drawHelper = Movie1PsxDrawHelper::Sub8001FEB4_FinalNoVideoFrame;
    }
    return out;
}

OutroSub80020308Step AdvanceOutroSub80020308(bool word800916DC,
                                             uint32_t& gp196) {
    OutroSub80020308Step out = ResolveOutroSub80020308Step(word800916DC, gp196);
    if (!out.complete) {
        ++gp196;
        out.nextGp196 = gp196;
        out.complete = word800916DC
            ? (gp196 >= kSub80020308SubtitleCompleteFrames)
            : (gp196 >= kSub80020308NoSubtitleCompleteFrames);
        if (out.complete) {
            gp196 = 192u;
            out.nextGp196 = 192u;
        }
    } else {
        gp196 = 192u;
        out.nextGp196 = 192u;
    }
    return out;
}

OutroSub80020308Step ResolveOutroSub80020248Step(bool word800916DC,
                                                 uint32_t gp196) {
    OutroSub80020308Step out{};
    out.playCue1 = gp196 == 0u;
    out.psxResult = 192;
    if (word800916DC) {
        out.playCue0 = gp196 == kSub80020248SubtitleCompleteFrames / 2u;
        out.keepVideoFrame = true;
        out.noVideoFrame = false;
        out.complete = gp196 >= kSub80020248SubtitleCompleteFrames;
        out.nextGp196 = out.complete ? 192u : gp196;
        out.drawHelper = gp196 >= kSub80020248SubtitleCompleteFrames / 2u
            ? Movie1PsxDrawHelper::Sub8001C864_SubtitleFrame
            : Movie1PsxDrawHelper::Sub8001F230_OutroNoSubboxFrame;
    } else {
        out.keepVideoFrame = true;
        out.noVideoFrame = false;
        out.complete = gp196 >= kSub80020248NoSubtitleCompleteFrames;
        out.nextGp196 = out.complete ? 192u : gp196;
        out.drawHelper = Movie1PsxDrawHelper::Sub8001CE30_NoSubtitleFrame;
    }
    return out;
}

OutroSub80020308Step AdvanceOutroSub80020248(bool word800916DC,
                                             uint32_t& gp196) {
    OutroSub80020308Step out = ResolveOutroSub80020248Step(word800916DC, gp196);
    if (!out.complete) {
        ++gp196;
        out.nextGp196 = word800916DC
            ? (gp196 < kSub80020248SubtitleCompleteFrames ? gp196 : 192u)
            : (gp196 < kSub80020248NoSubtitleCompleteFrames ? gp196 : 192u);
        out.complete = out.nextGp196 == 192u;
        if (out.complete) {
            gp196 = 192u;
        }
    } else {
        gp196 = 192u;
        out.nextGp196 = 192u;
    }
    return out;
}

void BeginTransitionSub800201ACTail(Movie1RuntimeState& state,
                                    uint32_t sourceFrame30) {
    state.outroActive = false;
    state.outroDrainStrAudio = false;
    state.outroCue0Played = false;
    state.outroCue1Played = false;
    state.outroGp196 = kSub800201ACLoopFrames;
    state.outroSourceFrame30 = sourceFrame30;
    state.outroTailFrames = kSub800201ACTailFrames;
    state.outroTailCompletePending = false;
    state.transitionGp792 = 0;
    state.transitionGp872 = 0;
    state.transitionSub800201ACActive = true;
    state.transitionSub800201ACCompletesMovie = true;
    state.transitionSub800201ACCompleted = false;
    state.transitionSub800201ACPhase = TransitionSub800201ACPhase::EndSub8001FFD4;
    state.transitionSub800201ACA1 = kSub800201ACWork801C3640;
    state.transitionSub800201ACA2 = kSub800201ACPostMovieMode;
    state.transitionSub800201ACA3 = kSub800201ACPostMoviePreFfd4Arg;
    state.transitionSub800201ACA4 = kSub800201ACPostMoviePostFfd4Arg;
    state.transitionSub80020090TailIndex = 0;
    state.transitionSub80020110Variant = false;
    state.transitionTileMask = Movie1TransitionTileMaskRuntimeState{};
    ClearTransitionDrawStep(state);
}

void BeginTransitionSub800201AC(Movie1RuntimeState& state,
                                uint32_t a1,
                                uint32_t modeA2,
                                uint32_t preFfd4ArgA3,
                                uint32_t postFfd4ArgA4,
                                uint32_t sourceFrame30,
                                bool completesMovie) {
    state.outroActive = false;
    state.outroDrainStrAudio = false;
    state.outroCue0Played = false;
    state.outroCue1Played = false;
    state.outroGp196 = 0;
    state.outroSourceFrame30 = sourceFrame30;
    state.outroTailFrames = 0;
    state.outroTailCompletePending = false;
    state.playAndWaitCompletionPending = false;
    state.transitionGp792 = 0;
    state.transitionGp872 = 0;
    state.transitionSub800201ACActive = true;
    state.transitionSub800201ACCompletesMovie = completesMovie;
    state.transitionSub800201ACCompleted = false;
    state.transitionSub800201ACPhase =
        TransitionSub800201ACPhase::InitialSub8001FFD4;
    state.transitionSub800201ACA1 = a1;
    state.transitionSub800201ACA2 = modeA2;
    state.transitionSub800201ACA3 = preFfd4ArgA3;
    state.transitionSub800201ACA4 = postFfd4ArgA4;
    state.transitionSub80020090TailIndex = 0;
    state.transitionSub80020110Variant = false;
    state.transitionTileMask = Movie1TransitionTileMaskRuntimeState{};
    ClearTransitionDrawStep(state);
    state.skipPreludeFrames = 0;
    state.skipPreludeSourceFrame30 = 0;
}

void BeginTransitionSub80020110(Movie1RuntimeState& state,
                                uint32_t a1,
                                uint32_t modeA2,
                                uint32_t preFfd4ArgA3,
                                uint32_t postFfd4ArgA4,
                                uint32_t sourceFrame30,
                                bool completesMovie) {
    BeginTransitionSub800201AC(
        state,
        a1,
        modeA2,
        preFfd4ArgA3,
        postFfd4ArgA4,
        sourceFrame30,
        completesMovie);
    state.transitionSub80020110Variant = true;
}

bool ConsumeTransitionSub800201ACCompleted(Movie1RuntimeState& state) {
    if (!state.transitionSub800201ACCompleted) {
        return false;
    }
    state.transitionSub800201ACCompleted = false;
    ClearMovieFrame30(state);
    ClearTransitionDrawStep(state);
    return true;
}

bool IsPlayAndWaitCompletionPending(const Movie1RuntimeState& state) {
    return state.playAndWaitCompletionPending;
}

bool IsTransitionSub80020110Active(const Movie1RuntimeState& state,
                                   uint32_t modeA2,
                                   uint32_t preFfd4ArgA3,
                                   uint32_t postFfd4ArgA4) {
    return state.transitionSub800201ACActive &&
           state.transitionSub80020110Variant &&
           state.transitionSub800201ACA2 == modeA2 &&
           state.transitionSub800201ACA3 == preFfd4ArgA3 &&
           state.transitionSub800201ACA4 == postFfd4ArgA4;
}

bool ApplySub80027194CueCadence(Movie1RuntimeState& state) {
    const bool triggerCue = state.transitionSub80027194Counter >= 2u;
    if (triggerCue) {
        state.transitionSub80027194Counter = 0;
    }
    ++state.transitionSub80027194Counter;
    return triggerCue;
}

void ClearPlayAndWaitCompletionPending(Movie1RuntimeState& state) {
    state.playAndWaitCompletionPending = false;
    state.currentMovieFrame30 = 0;
    state.skipPreludeFrames = 0;
    state.skipPreludeSourceFrame30 = 0;
    ClearTransitionDrawStep(state);
}

Movie1AdvanceResult AdvanceRuntimePure(
    Movie1RuntimeState& state,
    const Movie1HostFeedback& host,
    const PrMovieSubtitles::MovieSubtitleTrack& subtitleTrack) {
    Movie1AdvanceResult result{};
    const bool freezeSubbox = host.freezeSubbox;

    if (TickSkipPrelude(state)) {
        const uint32_t frame30 = state.outroActive ? state.outroSourceFrame30 : state.skipPreludeSourceFrame30;
        state.currentMovieFrame30 = frame30;
        if (state.outroActive && state.outroGp196 == 0u && !state.outroCue0Played) {
            AppendMovie1HostAction(
                result.hostActions,
                Movie1HostActionKind::PlayMovie1Cue9441C,
                0u,
                0u);
            state.outroCue0Played = true;
        }
        result.handledFrame = true;
        return result;
    }

    if (state.transitionSub800201ACActive) {
        const TransitionSub800201ACStep transitionStep =
            ResolveTransitionSub800201ACStep(state, host.subtitleEnabled);
        if (transitionStep.phase == TransitionSub800201ACPhase::Complete) {
            if (state.transitionSub800201ACCompletesMovie) {
                AppendMovie1HostAction(result.hostActions, Movie1HostActionKind::StopStr);
                MarkMovieCompleted(state);
                result.completedToStage1 = true;
                result.resetStageRenderRuntime = true;
                result.resetTextRuntimes = true;
            } else {
                state.transitionSub800201ACActive = false;
                state.transitionSub800201ACPhase =
                    TransitionSub800201ACPhase::Idle;
                state.transitionSub800201ACA1 = 0;
                state.transitionSub800201ACA2 = 0;
                state.transitionSub800201ACA3 = 0;
                state.transitionSub800201ACA4 = 0;
                state.transitionSub80020090TailIndex = 0;
                state.transitionSub80020110Variant = false;
                state.transitionGp792 = 0;
                state.transitionGp872 = 0;
                state.outroGp196 = 0;
                state.outroTailFrames = 0;
                state.outroTailCompletePending = false;
                state.transitionTileMask = Movie1TransitionTileMaskRuntimeState{};
                ClearTransitionDrawStep(state);
                state.transitionSub800201ACCompleted = true;
                result.handledFrame = true;
            }
            return result;
        }

        state.currentMovieFrame30 = state.outroSourceFrame30;
        ApplyTransitionTileMaskMutationsToRuntime(
            state.transitionTileMask,
            transitionStep);
        LatchTransitionDrawStep(state, transitionStep);
        if (TransitionStepHasAction(
                transitionStep,
                TransitionSub800201ACAction::Sub8001EBF4_Sub80040370)) {
            ApplySub80040370Flip(state);
        }
        if (TransitionStepHasAction(
                transitionStep,
                TransitionSub800201ACAction::
                    Sub80027194_Cue8006EC18AndFlush) &&
            ApplySub80027194CueCadence(state)) {
            AppendMovie1HostAction(
                result.hostActions,
                Movie1HostActionKind::PlayMovieTransitionCue8006EC18);
        }
        if (TransitionStepHasAction(
                transitionStep,
                TransitionSub800201ACAction::Sub800271E4_PlayCue0)) {
            AppendMovie1HostAction(
                result.hostActions,
                Movie1HostActionKind::PlayMovie1Cue9441C,
                0u,
                0u);
        }
        if (TransitionStepHasAction(
                transitionStep,
                TransitionSub800201ACAction::Sub800271E4_PlayCue1)) {
            AppendMovie1HostAction(
                result.hostActions,
                Movie1HostActionKind::PlayMovie1Cue9441C,
                0u,
                1u);
        }
        if (transitionStep.phase ==
            TransitionSub800201ACPhase::InitialSub8001FFD4) {
            state.outroGp196 = transitionStep.nextGp196;
            state.outroTailFrames = 0;
            state.outroTailCompletePending = false;
            state.transitionGp792 = transitionStep.nextGp792;
            state.transitionGp872 = transitionStep.nextGp872;
            state.transitionSub800201ACPhase =
                TransitionSub800201ACPhase::LoopSub8001EA74;
            state.transitionSub80020090TailIndex = 0;
            result.handledFrame = true;
            return result;
        }
        if (transitionStep.phase ==
            TransitionSub800201ACPhase::LoopSub8001EA74) {
            state.outroGp196 = transitionStep.nextGp196;
            state.transitionGp792 = transitionStep.nextGp792;
            state.transitionGp872 = transitionStep.nextGp872;
            if (transitionStep.complete ||
                state.outroGp196 >= kSub800201ACLoopFrames) {
                state.transitionSub800201ACPhase =
                    TransitionSub800201ACPhase::EndSub8001FFD4;
            }
            result.handledFrame = true;
            return result;
        }
        if (transitionStep.phase == TransitionSub800201ACPhase::EndSub8001FFD4) {
            state.outroGp196 = transitionStep.nextGp196;
            state.outroTailFrames = kSub800201ACTailFrames;
            state.outroTailCompletePending = false;
            state.transitionGp792 = transitionStep.nextGp792;
            state.transitionGp872 = transitionStep.nextGp872;
            state.transitionSub800201ACPhase =
                TransitionSub800201ACPhase::TailSub80020090;
            state.transitionSub80020090TailIndex = 0;
            result.handledFrame = true;
            return result;
        }
        if (transitionStep.phase == TransitionSub800201ACPhase::TailSub80020090) {
            state.outroGp196 = transitionStep.nextGp196;
            state.transitionGp792 = transitionStep.nextGp792;
            state.transitionGp872 = transitionStep.nextGp872;
            if (state.outroTailFrames > 0u) {
                --state.outroTailFrames;
                ++state.transitionSub80020090TailIndex;
            }
            if (state.outroTailFrames == 0u) {
                state.outroTailCompletePending = true;
                state.transitionSub800201ACPhase =
                    TransitionSub800201ACPhase::Complete;
            }
            result.handledFrame = true;
            return result;
        }
        result.handledFrame = true;
        return result;
    }

    if (state.outroTailCompletePending) {
        AppendMovie1HostAction(result.hostActions, Movie1HostActionKind::StopStr);
        MarkMovieCompleted(state);
        result.completedToStage1 = true;
        result.resetStageRenderRuntime = true;
        result.resetTextRuntimes = true;
        return result;
    }

    if (state.outroTailFrames > 0u) {
        state.currentMovieFrame30 = state.outroSourceFrame30;
        --state.outroTailFrames;
        if (state.outroTailFrames == 0u) {
            state.outroTailCompletePending = true;
        }
        result.handledFrame = true;
        return result;
    }

    if (state.outroActive) {
        const bool subtitleEnabled = host.subtitleEnabled;
        if (state.outroGp196 == 0u && !state.outroCue0Played) {
            AppendMovie1HostAction(
                result.hostActions,
                Movie1HostActionKind::PlayMovie1Cue9441C,
                0u,
                0u);
            state.outroCue0Played = true;
        }
        if (subtitleEnabled &&
            state.outroGp196 == kSub80020308SubtitleHoldFrames &&
            !state.outroCue1Played) {
            AppendMovie1HostAction(
                result.hostActions,
                Movie1HostActionKind::PlayMovie1Cue9441C,
                0u,
                1u);
            state.outroCue1Played = true;
        }
        const OutroSub80020308Step step = ResolveOutroSub80020308Step(state, subtitleEnabled);
        state.currentMovieFrame30 = state.outroSourceFrame30;
        const uint32_t sourceFrame30 = state.outroSourceFrame30;
        const OutroSub80020308Step advanced = AdvanceOutroSub80020308(state, subtitleEnabled);
        if (advanced.complete) {
            MarkMovieStrBlockCompletedBeforePostTransition(
                state,
                sourceFrame30);
            result.completedToStage1 = true;
            result.resetStageRenderRuntime = true;
            result.resetTextRuntimes = true;
        }
        result.handledFrame = true;
        return result;
    }

    if (host.debugStage1DirectBootRequested && !state.strPlayed) {
        AppendMovie1HostAction(result.hostActions, Movie1HostActionKind::StopStr);
        MarkMovieCompleted(state);
        result.completedToStage1 = true;
        result.resetStageRenderRuntime = true;
        AppendMovie1HostAction(
            result.hostActions,
            Movie1HostActionKind::LogDebugDirectBootSkip);
        return result;
    }

    if (state.strPlayed) {
        return result;
    }

    if (!host.strPlayerReady) {
        return result;
    }

    if (!state.strStarted) {
        AppendMovie1HostAction(result.hostActions, Movie1HostActionKind::StopStr);
        if (host.movie1StrExists) {
            AppendMovie1HostAction(
                result.hostActions,
                Movie1HostActionKind::PlayMovie1Str);
            state.strStarted = true;
            result.handledFrame = true;
            return result;
        } else {
            return result;
        }
    }

    if (!state.strStarted) {
        return result;
    }

    if (freezeSubbox && host.subtitleEnabled) {
        const uint32_t freezeFrame30 = host.movieFrame30;
        const bool inSubboxOnlyWindow =
            IsMovieTextFrameVisibleSub80024CF8(freezeFrame30,
                                               state.subtitleWindow,
                                               false) &&
            freezeFrame30 < state.subtitleWindow.firstTextFrame;
        if (inSubboxOnlyWindow) {
            state.currentMovieFrame30 = freezeFrame30;
            result.handledFrame = true;
            return result;
        }
    }

    const StrPlayerResult playerResult = host.lastStrUpdateResult;

    const uint32_t movieFrame30 = host.movieFrame30;
    const char* movieText =
        ResolveMovieTextSub80024CF8(
            subtitleTrack,
            movieFrame30,
            (int)host.languageIndex);
    const bool subtitleTextVisible =
        (host.subtitleEnabled && movieText && movieText[0] != '\0');
    const bool subtitleFrameVisible =
        host.subtitleEnabled &&
        IsMovieTextFrameVisibleSub80024CF8(movieFrame30,
                                           state.subtitleWindow,
                                           subtitleTextVisible);
    state.currentMovieFrame30 = movieFrame30;
    if (freezeSubbox && subtitleFrameVisible) {
        result.handledFrame = true;
        return result;
    }

    if (playerResult == StrPlayerResult::Skipped) {
        AppendMovie1HostAction(result.hostActions, Movie1HostActionKind::PauseStr);
        MarkMovieStrBlockCompletedBeforePostTransition(
            state,
            movieFrame30);
        result.completedToStage1 = true;
        result.resetStageRenderRuntime = true;
        result.resetTextRuntimes = true;
        result.handledFrame = true;
        return result;
    }
    if (host.strVideoFinished) {
        MarkMovieStrBlockCompletedBeforePostTransition(state, movieFrame30);
        result.completedToStage1 = true;
        result.resetStageRenderRuntime = true;
        result.resetTextRuntimes = true;
        result.handledFrame = true;
        return result;
    }
    if (playerResult == StrPlayerResult::Playing) {
        result.handledFrame = true;
        return result;
    }
    if (playerResult == StrPlayerResult::Finished) {
        MarkMovieStrBlockCompletedBeforePostTransition(state, movieFrame30);
        result.completedToStage1 = true;
        result.resetStageRenderRuntime = true;
        result.resetTextRuntimes = true;
        result.handledFrame = true;
        return result;
    }
    return result;
}

void CalcMovie1VideoRect(bool subtitleEnabled,
                         float vx,
                         float vy,
                         float vs,
                         float& outX,
                         float& outY,
                         float& outW,
                         float& outH) {
    const Movie1PsxRect& rect = SelectMovie1VideoRectPsx(subtitleEnabled);
    outX = vx + (float)rect.x * vs;
    outY = vy + (float)rect.y * vs;
    outW = (float)rect.w * vs;
    outH = (float)rect.h * vs;
}

void CalcSubtitleBoxRect(float vx,
                         float vy,
                         float vs,
                         float& outX,
                         float& outY,
                         float& outW,
                         float& outH) {
    outX = vx + (float)kSub8001C864Mode5SubtitleBoxRect.x * vs;
    outY = vy + (float)kSub8001C864Mode5SubtitleBoxRect.y * vs;
    outW = (float)kSub8001C864Mode5SubtitleBoxRect.w * vs;
    outH = (float)kSub8001C864Mode5SubtitleBoxRect.h * vs;
}

Movie1FrameState BuildFrameState(bool subtitleEnabled,
                                  float vx,
                                  float vy,
                                  float vs,
                                  bool subtitleFrameVisible,
                                  bool endingNoSubBoxLayout,
                                  bool finalNoVideoLayout) {
    Movie1FrameState state{};
    state.vx = vx;
    state.vy = vy;
    state.vs = vs;
    CalcMovie1VideoRect(subtitleEnabled, vx, vy, vs, state.videoX, state.videoY, state.videoW, state.videoH);
    CalcSubtitleBoxRect(vx, vy, vs, state.subBoxX, state.subBoxY, state.subBoxW, state.subBoxH);
    state.drawSubBox = subtitleEnabled && subtitleFrameVisible && !endingNoSubBoxLayout && !finalNoVideoLayout;
    state.useSubtitleLayout = state.drawSubBox;
    state.useEndingNoSubBoxLayout = endingNoSubBoxLayout;
    state.useFinalNoVideoLayout = finalNoVideoLayout;
    state.psxDrawHelper = ResolveMovie1PsxDrawHelper(state);
    return state;
}

bool BuildDrawPlanFromFrameState(const Movie1FrameState& frame,
                                 Movie1DrawPlan& outPlan,
                                 bool suppressFrameTemplates) {
    outPlan = Movie1DrawPlan{};
    outPlan.frame = frame;

    if (!frame.useFinalNoVideoLayout && frame.videoW > 0.0f && frame.videoH > 0.0f) {
        outPlan.drawVideo = true;
        outPlan.video.x = frame.videoX;
        outPlan.video.y = frame.videoY;
        outPlan.video.w = frame.videoW;
        outPlan.video.h = frame.videoH;
    }

    if (!suppressFrameTemplates &&
        !AppendMovie1PsxDrawHelperPlan(outPlan, frame.psxDrawHelper)) {
        outPlan = Movie1DrawPlan{};
        return false;
    }

    return true;
}

bool BuildRuntimeDrawPlan(Movie1RuntimeState& state,
                          const Movie1HostFeedback& host,
                          const PrMovieSubtitles::MovieSubtitleTrack& subtitleTrack,
                          float vx,
                          float vy,
                          float vs,
                          Movie1DrawPlan& outPlan,
                          bool suppressFrameTemplates) {
    outPlan = Movie1DrawPlan{};
    const bool subtitleEnabled = host.subtitleEnabled;
    const bool freezeSubbox = host.freezeSubbox;
    const bool skipPreludeActive = IsSkipPreludeActive(state);
    const bool outroTailActive =
        state.transitionSub800201ACActive ||
        state.outroTailFrames > 0u ||
        state.outroTailCompletePending;
    if (!state.strStarted && !state.outroActive && !skipPreludeActive &&
        !outroTailActive) {
        ClearMovieFrame30(state);
        MarkDrawPlanFailure(
            state,
            outPlan,
            Movie1DrawPlanBuildFailureReason::RuntimeInactive);
        return false;
    }
    if (!host.strPlayerReady && !state.outroActive && !outroTailActive &&
        !skipPreludeActive) {
        ClearMovieFrame30(state);
        MarkDrawPlanFailure(
            state,
            outPlan,
            Movie1DrawPlanBuildFailureReason::StrPlayerNotReady);
        return false;
    }
    if (host.strVideoFinished && !state.outroActive && !outroTailActive &&
        !skipPreludeActive) {
        ClearMovieFrame30(state);
        MarkDrawPlanFailure(
            state,
            outPlan,
            Movie1DrawPlanBuildFailureReason::StrVideoFinishedBeforeOutro);
        return false;
    }

    const uint32_t movieFrame30 =
        (state.outroActive || outroTailActive) ? state.outroSourceFrame30 :
        (skipPreludeActive ? state.skipPreludeSourceFrame30 : host.movieFrame30);
    const char* movieText =
        ResolveMovieTextSub80024CF8(subtitleTrack, movieFrame30, (int)host.languageIndex);
    const bool subtitleTextVisibleRaw =
        (subtitleEnabled && movieText && movieText[0] != '\0') &&
        !state.outroActive &&
        !outroTailActive;
    const bool subtitleFrameVisible =
        subtitleEnabled &&
        !state.outroActive &&
        !outroTailActive &&
        IsMovieTextFrameVisibleSub80024CF8(movieFrame30,
                                           state.subtitleWindow,
                                           subtitleTextVisibleRaw);
    bool endingNoSubBoxLayout = false;
    bool finalNoVideoLayout = false;
    bool transitionStepKnown = false;
    TransitionSub800201ACStep transitionStepForPlan{};
    const Movie1TransitionTileMaskRuntimeState* transitionMaskForPlan =
        nullptr;
    bool transitionDrawHelperKnown = false;
    Movie1PsxDrawHelper transitionDrawHelper =
        Movie1PsxDrawHelper::Sub8001CE30_NoSubtitleFrame;
    if (state.outroActive || outroTailActive) {
        if (state.transitionDrawStepValid) {
            transitionStepForPlan = state.transitionDrawStep;
            transitionStepKnown = true;
            transitionMaskForPlan = &state.transitionDrawMask;
        } else if (state.transitionSub800201ACActive) {
            transitionStepForPlan =
                ResolveTransitionSub800201ACStep(state, subtitleEnabled);
            transitionStepKnown = true;
            transitionMaskForPlan = &state.transitionTileMask;
        }
        if (transitionStepKnown) {
            if (transitionStepForPlan.drawFrame ||
                transitionStepForPlan.tailFrame) {
                transitionDrawHelperKnown = true;
                transitionDrawHelper = transitionStepForPlan.drawHelper;
            }
        }
        const OutroSub80020308Step outroRenderStep = outroTailActive
            ? ResolveOutroSub80020308Step(subtitleEnabled, state.outroGp196)
            : ResolveOutroSub80020308Step(state, subtitleEnabled);
        endingNoSubBoxLayout = outroRenderStep.keepVideoFrame;
        finalNoVideoLayout = outroRenderStep.noVideoFrame;
        movieText = nullptr;
    }

    state.currentMovieFrame30 = movieFrame30;
    Movie1FrameState movieFrame =
        BuildFrameState(subtitleEnabled,
                        vx,
                        vy,
                        vs,
                        subtitleFrameVisible,
                        endingNoSubBoxLayout,
                        finalNoVideoLayout);
    if (transitionDrawHelperKnown) {
        ApplyMovie1PsxDrawHelperLayout(movieFrame, transitionDrawHelper);
    }
    const bool keepFrozenMovieVideoForNoSubboxTransition =
        movieFrame30 != 0u &&
        movieFrame.psxDrawHelper ==
            Movie1PsxDrawHelper::Sub8001F230_OutroNoSubboxFrame &&
        !movieFrame.useFinalNoVideoLayout;
    const bool suppressThisFrameTemplates =
        suppressFrameTemplates &&
        !state.outroActive &&
        !outroTailActive &&
        !transitionDrawHelperKnown;
    if (!BuildDrawPlanFromFrameState(
            movieFrame,
            outPlan,
            suppressThisFrameTemplates)) {
        MarkDrawPlanFailure(
            state,
            outPlan,
            Movie1DrawPlanBuildFailureReason::HelperPlanFull);
        return false;
    }
    if ((state.outroActive ||
         outroTailActive ||
         transitionStepKnown ||
         skipPreludeActive) &&
        !keepFrozenMovieVideoForNoSubboxTransition) {
        outPlan.drawVideo = false;
    }
    ClearDrawPlanFailure(state);
    if (transitionStepKnown) {
        (void)AppendResolvedRawDrawActionsToPlan(
            transitionStepForPlan,
            outPlan,
            host.languageIndex,
            &state,
            transitionMaskForPlan);
    }
    return true;
}

bool BuildIntroTransitionDrawPlan(int frameIndex,
                                  float vx,
                                  float vy,
                                  float vs,
                                  Movie1DrawPlan& outPlan) {
    outPlan = Movie1DrawPlan{};
    outPlan.frame.vx = vx;
    outPlan.frame.vy = vy;
    outPlan.frame.vs = vs;

    int order = 0;
    for (int row = 0; row < kStage1IntroTransitionRows; ++row) {
        for (int col = 0; col < kStage1IntroTransitionCols; ++col) {
            if (!IsStage1IntroTransitionCellAlive(frameIndex, col, row)) {
                continue;
            }
            Movie1PsxTemplateCommand command{};
            command.x = (int16_t)(kMovie1PsxTilePx * col);
            command.y = (int16_t)(kMovie1PsxTilePx * row);
            command.tplAddr = GetStage1IntroTransitionTplAddr(col, row);
            command.layer = kStage1IntroTransitionLayer;
            if (!AppendMovie1TemplateDrawCommand(outPlan, command, order)) {
                outPlan = Movie1DrawPlan{};
                return false;
            }
        }
    }
    return true;
}

void InitTransitionTileMaskRuntimeSub8001FFD4(
    Movie1TransitionTileMaskRuntimeState& runtime,
    uint32_t arg) {
    InitTransitionTileMaskRuntime(runtime, arg != 1u);
    runtime.lastMutationCount = 1u;
    runtime.lastMutations[0] = TransitionTileMaskMutation{};
    runtime.lastMutations[0].kind =
        arg == 1u
            ? TransitionTileMaskMutationKind::InitAllClearBySub8001FFD4
            : TransitionTileMaskMutationKind::InitAllSetBySub8001FFD4;
    runtime.lastMutations[0].count = kMovie1TransitionTileMaskCellCount;
}

static bool AdvanceTransitionTileMaskBySub8001F698Pattern4(
    Movie1TransitionTileMaskRuntimeState& runtime,
    uint32_t& gp196,
    uint32_t pattern,
    uint32_t count,
    bool active) {
    TransitionTileMaskMutation mutation{};
    mutation.kind = active
        ? TransitionTileMaskMutationKind::SetBySub8001F698Pattern4
        : TransitionTileMaskMutationKind::ClearBySub8001F698Pattern4;
    mutation.startGp196 = gp196;
    mutation.count = count;
    mutation.pattern = pattern;
    mutation.value = active ? 1u : 0u;
    runtime.lastMutationCount = 1u;
    runtime.lastMutations[0] = mutation;

    const bool known = ApplySub8001F698Pattern4Mutation(runtime, mutation);
    if (!known) {
        MarkTransitionTileMaskRuntimeTableGap(runtime);
    }
    gp196 = AdvanceGp196BySub8001FCBCOrSub8001FC40(gp196, count);
    return known;
}

bool AdvanceTransitionTileMaskSub8001FC40(
    Movie1TransitionTileMaskRuntimeState& runtime,
    uint32_t& gp196,
    uint32_t pattern,
    uint32_t count) {
    return AdvanceTransitionTileMaskBySub8001F698Pattern4(
        runtime,
        gp196,
        pattern,
        count,
        false);
}

bool AdvanceTransitionTileMaskSub8001FCBC(
    Movie1TransitionTileMaskRuntimeState& runtime,
    uint32_t& gp196,
    uint32_t pattern,
    uint32_t count) {
    return AdvanceTransitionTileMaskBySub8001F698Pattern4(
        runtime,
        gp196,
        pattern,
        count,
        true);
}

bool BuildTransitionTileMaskDrawPlanSub8001FDC0(
    const Movie1TransitionTileMaskRuntimeState& runtime,
    uint16_t priority,
    float vx,
    float vy,
    float vs,
    Movie1DrawPlan& outPlan) {
    outPlan = Movie1DrawPlan{};
    outPlan.frame.vx = vx;
    outPlan.frame.vy = vy;
    outPlan.frame.vs = vs;
    if (!runtime.ownerValid || !runtime.cellsKnown) {
        return false;
    }

    int order = 0;
    for (uint32_t row = 0; row < kMovie1TransitionTileMaskRows; ++row) {
        for (uint32_t col = 0; col < kMovie1TransitionTileMaskCols; ++col) {
            const uint32_t cellIndex =
                row * kMovie1TransitionTileMaskCols + col;
            if (runtime.cells[cellIndex] == 0u) {
                continue;
            }
            Movie1PsxTemplateCommand command{};
            command.x = static_cast<int16_t>(kMovie1PsxTilePx * col);
            command.y = static_cast<int16_t>(kMovie1PsxTilePx * row);
            command.tplAddr =
                GetStage1IntroTransitionTplAddr((int)col, (int)row);
            command.layer = (uint16_t)Movie1RawDrawLayerFromOtBucket(priority);
            command.submitHelper = Movie1PsxTemplateSubmitHelper::Sub8001C550;
            if (!AppendMovie1TemplateDrawCommand(outPlan, command, order)) {
                outPlan = Movie1DrawPlan{};
                return false;
            }
        }
    }
    return true;
}

bool AppendResolvedRawDrawActionsToPlan(const TransitionSub800201ACStep& step,
                                        Movie1DrawPlan& plan,
                                        uint8_t languageIndex,
                                        Movie1RuntimeState* rawDrawRuntimeOwner,
                                        const Movie1TransitionTileMaskRuntimeState* activeTileMask) {
    bool appendedAnyTemplate = false;
    for (uint32_t i = 0; i < step.psxDrawActionCount; ++i) {
        Movie1PsxRawDrawAction action = step.psxDrawActions[i];
        AttachRawDrawC5A8Metadata(action);
        AttachRawDrawB25CMetadata(action);
        AttachRawDrawB590Metadata(action);
        AttachRawDrawBEE4Metadata(action);

        if (action.condition == Movie1PsxRawDrawCondition::ActiveTileMaskCell) {
            const Movie1TransitionTileMaskRuntimeState* maskRuntime =
                activeTileMask != nullptr ? activeTileMask : &step.activeTileMask;
            if (!TryAttachActiveTileMaskCell(action, maskRuntime)) {
                if (!action.activeTileMaskKnown) {
                    action.resolutionStatus =
                        Movie1PsxRawDrawResolutionStatus::GapActiveTileMask;
                    ++plan.rawOnlyActionCount;
                    if (!AppendRawDrawActionRecord(plan, action)) {
                        return appendedAnyTemplate;
                    }
                }
                continue;
            }
        }

        const ContextRawDrawEvalResult ctxEval =
            EvaluateMovie1TransitionCtxRawDrawCondition(
                step.ctxWords801C3640,
                action);
        if (ctxEval == ContextRawDrawEvalResult::Skip) {
            continue;
        }
        if (ctxEval == ContextRawDrawEvalResult::Execute &&
            (action.condition == Movie1PsxRawDrawCondition::ContextWordEquals ||
             action.condition == Movie1PsxRawDrawCondition::ContextWordNotEquals ||
             action.condition == Movie1PsxRawDrawCondition::ContextWordDefault)) {
            action.condition = Movie1PsxRawDrawCondition::Always;
        }

        Movie1PsxRawDrawResolutionStatus status =
            Movie1PsxRawDrawResolutionStatus::RawOnly;
        (void)ResolveMovie1PsxGpTemplateAction(action);
        const bool resolvedTemplate =
            IsRawDrawActionFullyResolvedForTemplateCommand(action, status);
        Movie1PsxRawDrawAction typedAction{};
        Movie1PsxRawDrawB590Metadata typedB590{};
        Movie1PsxRawDrawBEE4Metadata typedBEE4{};
        const bool resolvedTypedSubmit =
            !resolvedTemplate &&
            TryBuildRawDrawForTypedSubmit(
                action,
                typedAction,
                typedB590,
                typedBEE4,
                status,
                languageIndex);
        if (resolvedTypedSubmit) {
            action = typedAction;
            action.b590 = typedB590;
            action.bee4 = typedBEE4;
            const bool canRuntimeSubmit =
                status == Movie1PsxRawDrawResolutionStatus::ResolvedTypedSubmit ||
                (action.helper == Movie1PsxRawDrawHelper::Call8001BEE4 &&
                 IsFastSpriteRgbGapStatus(status));
            if (canRuntimeSubmit &&
                rawDrawRuntimeOwner != nullptr &&
                rawDrawRuntimeOwner->rawDrawFastSpriteRuntimeOwnerValid) {
                const Movie1RawDrawTypedSubmitResult submit =
                    ApplyRawDrawTypedSubmit(
                        action,
                        rawDrawRuntimeOwner->rawDrawFastSpriteRuntime,
                        languageIndex);
                if (submit.valid) {
                    action = submit.action;
                    action.b590 = submit.b590;
                    action.bee4 = submit.bee4;
                    status = submit.status;
                } else {
                    status = submit.status;
                }
            } else {
                if (status ==
                    Movie1PsxRawDrawResolutionStatus::ResolvedTypedSubmit) {
                    status =
                        Movie1PsxRawDrawResolutionStatus::GapGsSortFastSpriteSubmit;
                }
            }
        }
        action.resolutionStatus = status;

        if (resolvedTemplate) {
            if (!AppendResolvedRawDrawActionToPlan(plan, action)) {
                action.resolutionStatus =
                    Movie1PsxRawDrawResolutionStatus::GapPlanFull;
                ++plan.rawOnlyActionCount;
                if (!AppendRawDrawActionRecord(plan, action)) {
                    return appendedAnyTemplate;
                }
                continue;
            }
            ++plan.rawResolvedTemplateCount;
            appendedAnyTemplate = true;
        } else if (resolvedTypedSubmit) {
            if (action.resolutionStatus ==
                Movie1PsxRawDrawResolutionStatus::ResolvedTypedSubmit) {
                ++plan.rawResolvedTypedSubmitCount;
                ++plan.rawAppliedTypedSubmitCount;
            } else if (action.resolutionStatus ==
                           Movie1PsxRawDrawResolutionStatus::GapGsSortFastSpriteSubmit ||
                       IsFastSpriteRgbGapStatus(action.resolutionStatus)) {
                ++plan.rawTypedSubmitOwnerGapCount;
                if (IsFastSpriteRgbGapStatus(action.resolutionStatus)) {
                    ++plan.rawTypedSubmitRgbGapCount;
                }
                if (action.resolutionStatus ==
                    Movie1PsxRawDrawResolutionStatus::GapRawDrawFastSpriteStackRgb) {
                    ++plan.rawDrawFastSpriteRgbGapCount;
                } else if (action.resolutionStatus ==
                           Movie1PsxRawDrawResolutionStatus::
                               GapTransitionRawActionFastSpriteStackRgb) {
                    ++plan.transitionRawActionFastSpriteRgbGapCount;
                } else if (action.resolutionStatus ==
                           Movie1PsxRawDrawResolutionStatus::GapTextGlyphFastSpriteRgb) {
                    ++plan.textGlyphFastSpriteRgbGapCount;
                }
                ++plan.rawOnlyActionCount;
            } else {
                ++plan.rawOnlyActionCount;
            }
        } else {
            ++plan.rawOnlyActionCount;
        }

        if (!AppendRawDrawActionRecord(plan, action)) {
            return appendedAnyTemplate;
        }
    }
    return appendedAnyTemplate;
}

}  // namespace PrStage1Scene1Movie1Direct
