#include "pr_scene_entry_direct.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace PrSceneEntryDirect {

namespace {

struct SceneEntryPathTableRow {
    uint32_t pathPtr;
    SceneEntryPathRole role;
    const char* psxPath;
    const char* relativeWinPath;
};

struct SceneEntryStaticRow {
    uint32_t words[kSceneEntryMovieSegmentRowWordCount];
};

constexpr SceneEntryPathTableRow kKnownPathRows[] = {
    {0x800113A0u, SceneEntryPathRole::Common, "\\S0\\COMMON.INT;1",
     "S0/COMMON.INT"},
    {0x800117D4u, SceneEntryPathRole::Comod, "\\S0\\COMOD0.BIN;1",
     "S0/COMOD0.BIN"},
    {0x800117C0u, SceneEntryPathRole::Compo, "\\S0\\COMPO00.INT;1",
     "S0/COMPO00.INT"},
    {0x800113B4u, SceneEntryPathRole::OpeningMovie, "\\SS\\MOVIE0.STR;1",
     "SS/MOVIE0.STR"},
    {0x800117ACu, SceneEntryPathRole::StageRuntime, "\\SS\\MOVIE0T.STR;1",
     "SS/MOVIE0T.STR"},
    {0x80011798u, SceneEntryPathRole::ZCompo, "\\S0\\ZCOMPO.INT;1",
     "S0/ZCOMPO.INT"},
    {0x800113C8u, SceneEntryPathRole::PracticeYCompo,
     "\\S0\\YCOMPO.INT;1", "S0/YCOMPO.INT"},
    {0x80011784u, SceneEntryPathRole::Comod, "\\S1\\COMOD1.BIN;1",
     "S1/COMOD1.BIN"},
    {0x80011770u, SceneEntryPathRole::Compo, "\\S1\\COMPO01.INT;1",
     "S1/COMPO01.INT"},
    {0x8001175Cu, SceneEntryPathRole::OpeningMovie, "\\SS\\MOVIE1.STR;1",
     "SS/MOVIE1.STR"},
    {0x80011748u, SceneEntryPathRole::StageRuntime, "\\S1\\STAGE1.XA1;1",
     "S1/STAGE1.XA1"},
    {0x80011734u, SceneEntryPathRole::ClearMovie, "\\S1\\XMOVIE1.STR;1",
     "S1/XMOVIE1.STR"},
    {0x80011720u, SceneEntryPathRole::ZCompo, "\\S1\\ZCOMPO.INT;1",
     "S1/ZCOMPO.INT"},
    {0x8001170Cu, SceneEntryPathRole::Comod, "\\S2\\COMOD2.BIN;1",
     "S2/COMOD2.BIN"},
    {0x800116F8u, SceneEntryPathRole::Compo, "\\S2\\COMPO02.INT;1",
     "S2/COMPO02.INT"},
    {0x800116E4u, SceneEntryPathRole::OpeningMovie, "\\SS\\MOVIE2.STR;1",
     "SS/MOVIE2.STR"},
    {0x800116D0u, SceneEntryPathRole::StageRuntime, "\\S2\\STAGE2.XA1;1",
     "S2/STAGE2.XA1"},
    {0x800116BCu, SceneEntryPathRole::ClearMovie, "\\S2\\XMOVIE2.STR;1",
     "S2/XMOVIE2.STR"},
    {0x800116A8u, SceneEntryPathRole::ClearMovie, "\\S2\\YMOVIE2.STR;1",
     "S2/YMOVIE2.STR"},
    {0x80011694u, SceneEntryPathRole::ZCompo, "\\S2\\ZCOMPO.INT;1",
     "S2/ZCOMPO.INT"},
    {0x80011680u, SceneEntryPathRole::Comod, "\\S3\\COMOD3.BIN;1",
     "S3/COMOD3.BIN"},
    {0x8001166Cu, SceneEntryPathRole::Compo, "\\S3\\COMPO03.INT;1",
     "S3/COMPO03.INT"},
    {0x80011658u, SceneEntryPathRole::OpeningMovie, "\\SS\\MOVIE3.STR;1",
     "SS/MOVIE3.STR"},
    {0x80011644u, SceneEntryPathRole::StageRuntime, "\\S3\\STAGE3.XA1;1",
     "S3/STAGE3.XA1"},
    {0x80011630u, SceneEntryPathRole::ClearMovie, "\\S3\\XMOVIE3.STR;1",
     "S3/XMOVIE3.STR"},
    {0x8001161Cu, SceneEntryPathRole::ClearMovie, "\\S3\\YMOVIE3.STR;1",
     "S3/YMOVIE3.STR"},
    {0x80011608u, SceneEntryPathRole::ZCompo, "\\S3\\ZCOMPO.INT;1",
     "S3/ZCOMPO.INT"},
    {0x800115E4u, SceneEntryPathRole::Comod, "\\S5\\COMOD5.BIN;1",
     "S5/COMOD5.BIN"},
    {0x800115D0u, SceneEntryPathRole::Compo, "\\S5\\COMPO05.INT;1",
     "S5/COMPO05.INT"},
    {0x800115BCu, SceneEntryPathRole::OpeningMovie, "\\SS\\MOVIE5.STR;1",
     "SS/MOVIE5.STR"},
    {0x800115A8u, SceneEntryPathRole::StageRuntime, "\\S5\\STAGE5.XA1;1",
     "S5/STAGE5.XA1"},
    {0x80011594u, SceneEntryPathRole::ClearMovie, "\\S5\\XMOVIE5.STR;1",
     "S5/XMOVIE5.STR"},
    {0x80011580u, SceneEntryPathRole::ClearMovie, "\\S5\\YMOVIE5.STR;1",
     "S5/YMOVIE5.STR"},
    {0x8001156Cu, SceneEntryPathRole::ZCompo, "\\S5\\ZCOMPO.INT;1",
     "S5/ZCOMPO.INT"},
    {0x80011558u, SceneEntryPathRole::Comod, "\\S6\\COMOD6.BIN;1",
     "S6/COMOD6.BIN"},
    {0x80011544u, SceneEntryPathRole::Compo, "\\S6\\COMPO06.INT;1",
     "S6/COMPO06.INT"},
    {0x80011530u, SceneEntryPathRole::OpeningMovie, "\\SS\\MOVIE6.STR;1",
     "SS/MOVIE6.STR"},
    {0x8001151Cu, SceneEntryPathRole::StageRuntime, "\\S6\\STAGE6.XA1;1",
     "S6/STAGE6.XA1"},
    {0x80011508u, SceneEntryPathRole::ClearMovie, "\\S6\\XMOVIE6.STR;1",
     "S6/XMOVIE6.STR"},
    {0x800114F4u, SceneEntryPathRole::ClearMovie, "\\S6\\YMOVIE6.STR;1",
     "S6/YMOVIE6.STR"},
    {0x800114E0u, SceneEntryPathRole::ZCompo, "\\S6\\ZCOMPO.INT;1",
     "S6/ZCOMPO.INT"},
    {0x800114CCu, SceneEntryPathRole::Comod, "\\S7\\COMOD7.BIN;1",
     "S7/COMOD7.BIN"},
    {0x800114B8u, SceneEntryPathRole::Compo, "\\S7\\COMPO07.INT;1",
     "S7/COMPO07.INT"},
    {0x800114A4u, SceneEntryPathRole::OpeningMovie, "\\SS\\MOVIE7.STR;1",
     "SS/MOVIE7.STR"},
    {0x80011490u, SceneEntryPathRole::StageRuntime, "\\S7\\STAGE7.XA1;1",
     "S7/STAGE7.XA1"},
    {0x8001147Cu, SceneEntryPathRole::ZCompo, "\\S7\\ZCOMPO.INT;1",
     "S7/ZCOMPO.INT"},
};

constexpr SceneEntryStaticRow
    kStaticRowsByScene[kSceneEntrySceneCount][kSceneEntryMovieSegmentRowCount] =
        {
            {{{0x800117D4u, 0x00000000u, 0x00000000u, 0x00000000u,
               0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,
               0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u}},
             {{0x800117C0u, 0x00000000u, 0x00000000u, 0x00000000u,
               0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,
               0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u}},
             {{0x800113B4u, 0x005A0001u, 0x00000000u, 0x00000000u,
               0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,
               0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u}},
             {{0x800117ACu, 0x007F0001u, 0x00000000u, 0x00000000u,
               0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,
               0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u}},
             {{0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,
               0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,
               0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u}},
             {{0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,
               0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,
               0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u}},
             {{0x80011798u, 0x005A0001u, 0x00000000u, 0x00000000u,
               0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,
               0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u}}},
            {{{0x80011784u, 0x00000000u, 0x00000000u, 0x00000000u,
               0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,
               0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u}},
             {{0x80011770u, 0x00000000u, 0x00000000u, 0x00000000u,
               0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,
               0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u}},
             {{0x8001175Cu, 0x005A0001u, 0x00000000u, 0x00000000u,
               0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,
               0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u}},
             {{0x80011748u, 0x005A0001u, 0x00000000u, 0x00000000u,
               0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,
               0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u}},
             {{0x80011734u, 0x005A0001u, 0x00000000u, 0x00000000u,
               0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,
               0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u}},
             {{0x80011734u, 0x005A0001u, 0x00000000u, 0x00000000u,
               0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,
               0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u}},
             {{0x80011720u, 0x005A0001u, 0x00000000u, 0x00000000u,
               0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,
               0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u}}},
            {{{0x8001170Cu, 0x00000000u, 0x00000000u, 0x00000000u,
               0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,
               0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u}},
             {{0x800116F8u, 0x00000000u, 0x00000000u, 0x00000000u,
               0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,
               0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u}},
             {{0x800116E4u, 0x005A0001u, 0x00000000u, 0x00000000u,
               0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,
               0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u}},
             {{0x800116D0u, 0x005A0001u, 0x00000000u, 0x00000000u,
               0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,
               0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u}},
             {{0x800116BCu, 0x007F0001u, 0x0000004Bu, 0x00000000u,
               0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,
               0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u}},
             {{0x800116A8u, 0x007F0001u, 0x0000004Bu, 0x00000000u,
               0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,
               0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u}},
             {{0x80011694u, 0x005A0001u, 0x00000000u, 0x00000000u,
               0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,
               0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u}}},
            {{{0x80011680u, 0x00000000u, 0x00000000u, 0x00000000u,
               0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,
               0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u}},
             {{0x8001166Cu, 0x00000000u, 0x00000000u, 0x00000000u,
               0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,
               0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u}},
             {{0x80011658u, 0x005A0001u, 0x0000004Bu, 0x00000000u,
               0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,
               0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u}},
             {{0x80011644u, 0x005A0001u, 0x00000000u, 0x00000000u,
               0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,
               0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u}},
             {{0x80011630u, 0x007F0001u, 0x00000000u, 0x00000000u,
               0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,
               0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u}},
             {{0x8001161Cu, 0x007F0001u, 0x00000000u, 0x00000000u,
               0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,
               0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u}},
             {{0x80011608u, 0x005A0001u, 0x00000000u, 0x00000000u,
               0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,
               0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u}}},
            {{{0x800115E4u, 0x00000000u, 0x00000000u, 0x00000000u,
               0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,
               0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u}},
             {{0x800115D0u, 0x00000000u, 0x00000000u, 0x00000000u,
               0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,
               0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u}},
             {{0x800115BCu, 0x005A0001u, 0x00000000u, 0x00000000u,
               0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,
               0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u}},
             {{0x800115A8u, 0x005A0001u, 0x00000000u, 0x00000000u,
               0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,
               0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u}},
             {{0x80011594u, 0x005A0001u, 0x00000000u, 0x00000000u,
               0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,
               0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u}},
             {{0x80011580u, 0x005A0001u, 0x00000000u, 0x00000000u,
               0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,
               0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u}},
             {{0x8001156Cu, 0x005A0001u, 0x00000000u, 0x00000000u,
               0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,
               0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u}}},
            {{{0x80011558u, 0x00000000u, 0x00000000u, 0x00000000u,
               0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,
               0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u}},
             {{0x80011544u, 0x00000000u, 0x00000000u, 0x00000000u,
               0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,
               0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u}},
             {{0x80011530u, 0x005A0001u, 0xFFFFFA24u, 0x00000000u,
               0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,
               0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u}},
             {{0x8001151Cu, 0x005A0001u, 0x00000000u, 0x00000000u,
               0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,
               0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u}},
             {{0x80011508u, 0x005A0001u, 0x00000000u, 0x00000000u,
               0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,
               0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u}},
             {{0x800114F4u, 0x005A0001u, 0x00000000u, 0x00000000u,
               0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,
               0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u}},
             {{0x800114E0u, 0x005A0001u, 0x00000000u, 0x00000000u,
               0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,
               0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u}}},
            {{{0x800114CCu, 0x00000000u, 0x00000000u, 0x00000000u,
               0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,
               0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u}},
             {{0x800114B8u, 0x00000000u, 0x00000000u, 0x00000000u,
               0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,
               0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u}},
             {{0x800114A4u, 0x005A0001u, 0x00000096u, 0x00000000u,
               0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,
               0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u}},
             {{0x80011490u, 0x005A0001u, 0x00000000u, 0x00000000u,
               0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,
               0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u}},
             {{0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,
               0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,
               0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u}},
             {{0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,
               0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,
               0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u}},
             {{0x8001147Cu, 0x00000000u, 0x00000000u, 0x00000000u,
               0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u,
               0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u}}},
};

constexpr uint32_t kFn8001EF14 = 0x8001EF14u;
constexpr uint32_t kFn80015788 = 0x80015788u;
constexpr uint32_t kFn80015700 = 0x80015700u;
constexpr uint32_t kFn80015590 = 0x80015590u;
constexpr uint32_t kFn800161A8 = 0x800161A8u;
constexpr uint32_t kFn80017524 = 0x80017524u;
constexpr uint32_t kFn80017574 = 0x80017574u;
constexpr uint32_t kFn80018E10 = 0x80018E10u;
constexpr uint32_t kFn80018FB0 = 0x80018FB0u;
constexpr uint32_t kFn800191E4 = 0x800191E4u;
constexpr uint32_t kFn80019284 = 0x80019284u;
constexpr uint32_t kFn800193B0 = 0x800193B0u;
constexpr uint32_t kFn800193F4 = 0x800193F4u;
constexpr uint32_t kFn80019414 = 0x80019414u;
constexpr uint32_t kFn800179B4 = 0x800179B4u;
constexpr uint32_t kFn80017FC4 = 0x80017FC4u;
constexpr uint32_t kFn80016000 = 0x80016000u;
constexpr uint32_t kFn800164F8 = 0x800164F8u;
constexpr uint32_t kFn800168DC = 0x800168DCu;
constexpr uint32_t kFn80025A34 = 0x80025A34u;
constexpr uint32_t kFn8001A324 = 0x8001A324u;
constexpr uint32_t kFn800154B0 = 0x800154B0u;
constexpr uint32_t kFn80015CC4 = 0x80015CC4u;
constexpr uint32_t kFn8001E34C = 0x8001E34Cu;
constexpr uint32_t kFn80026B94 = 0x80026B94u;
constexpr uint32_t kFn80026ECC = 0x80026ECCu;
constexpr uint32_t kFn80026EF8 = 0x80026EF8u;
constexpr uint32_t kFn80026FA4 = 0x80026FA4u;
constexpr uint32_t kFn80026784 = 0x80026784u;
constexpr uint32_t kFn8002776C = 0x8002776Cu;
constexpr uint32_t kFn80035510 = 0x80035510u;
constexpr uint32_t kFn80019D7C = 0x80019D7Cu;
constexpr uint32_t kFn80025C64 = 0x80025C64u;
constexpr std::array<SceneCallbackTriplet80048D28,
                     kSceneCallbackTableSceneCount80048D28>
    kSceneCallbackTable80048D28{{
        {true, 0u, 0x80048D28u, 0x801C5B14u, 0x80048D2Cu, 0x801C4260u,
         0x80048D30u, 0x801C4DC4u},
        {true, 1u, 0x80048D34u, 0x801CA3BCu, 0x80048D38u, 0x801C7284u,
         0x80048D3Cu, 0x801C81ECu},
        {true, 2u, 0x80048D40u, 0x801C97ECu, 0x80048D44u, 0x801C657Cu,
         0x80048D48u, 0x801C74E4u},
        {true, 3u, 0x80048D4Cu, 0x801C998Cu, 0x80048D50u, 0x801C6918u,
         0x80048D54u, 0x801C7880u},
        {true, 4u, 0x80048D58u, 0x801CB348u, 0x80048D5Cu, 0x801C7D20u,
         0x80048D60u, 0x801C8C88u},
        {true, 5u, 0x80048D64u, 0x801C9310u, 0x80048D68u, 0x801C60B0u,
         0x80048D6Cu, 0x801C7030u},
        {true, 6u, 0x80048D70u, 0x801C9EA0u, 0x80048D74u, 0x801C6AC4u,
         0x80048D78u, 0x801C7A2Cu},
        {true, 7u, 0x80048D7Cu, 0x801C5780u, 0x80048D80u, 0x801C3870u,
         0x80048D84u, 0x801C4870u},
        {true, 8u, 0x80048D88u, 0x801C5BDCu, 0x80048D8Cu, 0x801C3870u,
         0x80048D90u, 0x801C4834u},
    }};
constexpr uint32_t kByte80092F10Address = 0x80092F10u;
constexpr uint32_t kUnk801C3640Address = 0x801C3640u;
constexpr uint32_t kWord800916D0Address = 0x800916D0u;
constexpr uint32_t kAsciiToPsxGlyphByteBase800491C4 = 0x800491C4u;
constexpr uint32_t kDword80049278Address = 0x80049278u;
constexpr int32_t kCase17Arg2EarlyReturn80019D7C = 3;
constexpr int32_t kCase17EarlyResult80019D7C = 5;
constexpr int32_t kCase17DoneResult80019D7C = 23;
constexpr int32_t kCase17Gp720DoneValue80019D7C = 1;
constexpr size_t kHiScoreNameBase80019284 = 4876u;
constexpr size_t kHiScoreScoreBase80019284 = 4888u;
constexpr size_t kHiScoreRowStride80019284 = 64u;
constexpr size_t kHiScoreColumnStride80019284 = 16u;
constexpr size_t kHiScoreBankScoreOffset80016000 = 12u;
constexpr uint8_t kAsciiToPsxGlyphByte800491C4[256] = {
    0x90, 0x82, 0x91, 0x82, 0x92, 0x82, 0x93, 0x82,
    0x94, 0x82, 0x95, 0x82, 0x96, 0x82, 0x97, 0x82,
    0x98, 0x82, 0x99, 0x82, 0x9A, 0x82, 0x6F, 0x81,
    0x62, 0x81, 0x70, 0x81, 0x50, 0x81, 0x40, 0x81,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x7C, 0x26, 0x27,
    0x3C, 0x3E, 0x2A, 0x2B, 0x1E, 0x2D, 0x1F, 0x2F,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
    0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
    0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
    0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
    0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x7E, 0x5F,
    0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
    0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
    0x78, 0x79, 0x7A, 0x2A, 0x7C, 0x7D, 0x7E, 0x7F,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xE8, 0x90, 0x04, 0x80,
    0x04, 0x00, 0x0E, 0x00, 0x39, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x06, 0x00, 0x03, 0x00, 0x13, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

constexpr uint8_t kHiScoreInitialTable80049278[kHiScoreTableSize80019284] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x03, 0x00,
    0x13, 0x00, 0x00, 0x00,
};

int32_t ReadS32LE(const uint8_t* p) {
    return static_cast<int32_t>(
        static_cast<uint32_t>(p[0]) |
        (static_cast<uint32_t>(p[1]) << 8) |
        (static_cast<uint32_t>(p[2]) << 16) |
        (static_cast<uint32_t>(p[3]) << 24));
}

void WriteS32LE(uint8_t* p, int32_t value) {
    const uint32_t v = static_cast<uint32_t>(value);
    p[0] = static_cast<uint8_t>(v & 0xFFu);
    p[1] = static_cast<uint8_t>((v >> 8) & 0xFFu);
    p[2] = static_cast<uint8_t>((v >> 16) & 0xFFu);
    p[3] = static_cast<uint8_t>((v >> 24) & 0xFFu);
}

bool IsKnownHiScoreCString80016000(const HiScoreBankName80016000& name) {
    return name.known && name.nulTerminatedBeforeScore;
}

bool SameHiScoreName80016000(const HiScoreBankName80016000& lhs,
                             const HiScoreBankName80016000& rhs) {
    if (!IsKnownHiScoreCString80016000(lhs) ||
        !IsKnownHiScoreCString80016000(rhs)) {
        return false;
    }
    for (size_t i = 0u; i < kHiScoreBankNameSize80016000; ++i) {
        if (lhs.bytes[i] != rhs.bytes[i]) {
            return false;
        }
        if (lhs.bytes[i] == 0u) {
            return true;
        }
    }
    return true;
}

bool SameHiScoreSlotCandidate800164F8(
    const HiScoreBankSlot800164F8& slot,
    const HiScoreBankName80016000& name,
    int32_t score) {
    return slot.valid && slot.scoreKnown && slot.score == score &&
           SameHiScoreName80016000(slot.name, name);
}

HiScoreSavePayload800164F8 ReadHiScoreSavePayload800164F8(
    const uint8_t* payload,
    size_t payloadSize) {
    HiScoreSavePayload800164F8 out{};
    out.inputSize = payloadSize;
    if (!payload ||
        payloadSize < kHiScoreSavePayloadRequiredSize800164F8) {
        out.missingInputRange = true;
        return out;
    }

    out.inputRangeKnown = true;
    out.name.known = true;
    std::memcpy(out.name.bytes,
                payload + kHiScoreSavePayloadNameOffset800164F8,
                kHiScoreBankNameSize80016000);
    for (uint8_t b : out.name.bytes) {
        if (b == 0u) {
            out.name.nulTerminatedBeforeScore = true;
            break;
        }
    }
    for (size_t row = 0u; row < kHiScoreBankRowCount800164F8; ++row) {
        const size_t scoreOffset =
            kHiScoreSavePayloadScoreBase800164F8 +
            row * kHiScoreSavePayloadScoreStride800164F8;
        out.scoreKnown[row] = true;
        out.score[row] = ReadS32LE(payload + scoreOffset);
    }
    return out;
}

HiScoreBankRow800164F8 PsxCall80016000_SortHiScoreRow(
    HiScoreBankRow800164F8 row,
    int32_t count) {
    const size_t safeCount =
        (count > 0)
            ? (std::min)(static_cast<size_t>(count),
                         kHiScoreBankSlotCount800164F8)
            : 0u;
    for (size_t i = 0u; i < safeCount; ++i) {
        if (i >= safeCount - 1u) {
            break;
        }
        for (size_t j = i + 1u; j < safeCount; ++j) {
            if (row.slots[i].scoreKnown &&
                row.slots[j].scoreKnown &&
                row.slots[i].score < row.slots[j].score) {
                std::swap(row.slots[i], row.slots[j]);
            }
        }
    }
    row.sortFunction80016000 = kFn80016000;
    row.sortArg80016000 = count;
    row.sortConvergenceGap80016000 = false;
    return row;
}

struct GlyphCopyResult80017FC4 {
    bool known = false;
    size_t writeCount = 0;
};

GlyphCopyResult80017FC4 ApplyGlyphMap80017FC4(uint8_t* dst,
                                              size_t dstSize,
                                              const char* src) {
    GlyphCopyResult80017FC4 out{};
    if (!dst || dstSize == 0u) {
        return out;
    }
    size_t i = 0u;
    if (src) {
        for (; src[i] != '\0'; ++i) {
            if (i + 1u >= dstSize) {
                return out;
            }
            const uint8_t sourceByte = static_cast<uint8_t>(src[i]);
            dst[i] = kAsciiToPsxGlyphByte800491C4[sourceByte];
        }
    }
    dst[i] = 0;
    out.known = true;
    out.writeCount = i + 1u;
    return out;
}

void EmitMainSceneRequest(MainSceneRequestTrace80015D18& trace,
                          MainSceneRequestKind80015D18 kind,
                          uint32_t psxFunction,
                          bool arg0Known = false,
                          int32_t arg0 = 0,
                          bool arg1Known = false,
                          int32_t arg1 = 0,
                          bool psxFunctionSlotKnown = false,
                          uint32_t psxFunctionSlot = 0) {
    if (trace.count >= trace.requests.size()) {
        trace.overflow = true;
        return;
    }
    MainSceneRequest80015D18& request = trace.requests[trace.count++];
    request.valid = true;
    request.kind = kind;
    request.psxFunction = psxFunction;
    request.psxFunctionSlotKnown = psxFunctionSlotKnown;
    request.psxFunctionSlot = psxFunctionSlot;
    request.arg0Known = arg0Known;
    request.arg0 = arg0;
    request.arg1Known = arg1Known;
    request.arg1 = arg1;
}

void EmitGenericSwitchRequest(GenericSwitchRequestTrace80015788& trace,
                              GenericSwitchRequestKind80015788 kind,
                              uint32_t psxFunction,
                              bool arg0Known = false,
                              int32_t arg0 = 0,
                              bool arg1Known = false,
                              int32_t arg1 = 0) {
    if (trace.count >= trace.requests.size()) {
        trace.overflow = true;
        return;
    }
    GenericSwitchRequest80015788& request = trace.requests[trace.count++];
    request.valid = true;
    request.kind = kind;
    request.psxFunction = psxFunction;
    request.arg0Known = arg0Known;
    request.arg0 = arg0;
    request.arg1Known = arg1Known;
    request.arg1 = arg1;
}

int32_t WordToS32(uint32_t word) {
    if ((word & 0x80000000u) == 0u) {
        return static_cast<int32_t>(word);
    }
    return static_cast<int32_t>(static_cast<int64_t>(word) - 0x100000000LL);
}

SceneEntryRawRow BuildRawRow(const SceneEntryStaticRow& source) {
    SceneEntryRawRow out{};
    out.known = true;
    for (uint32_t wordIndex = 0; wordIndex < kSceneEntryMovieSegmentRowWordCount;
         ++wordIndex) {
        const uint32_t word = source.words[wordIndex];
        out.words[wordIndex] = word;
        out.bytes[wordIndex * 4u + 0u] = static_cast<uint8_t>(word & 0xFFu);
        out.bytes[wordIndex * 4u + 1u] =
            static_cast<uint8_t>((word >> 8) & 0xFFu);
        out.bytes[wordIndex * 4u + 2u] =
            static_cast<uint8_t>((word >> 16) & 0xFFu);
        out.bytes[wordIndex * 4u + 3u] =
            static_cast<uint8_t>((word >> 24) & 0xFFu);
    }
    out.pathPtrKnown = true;
    out.pathPtr = out.words[0];
    out.opaque04Known = true;
    out.opaque04 = out.words[1];
    out.endBias08Known = true;
    out.endBias08 = WordToS32(out.words[2]);
    out.loadedState0CKnown = true;
    out.loadedState0C = WordToS32(out.words[3]);
    return out;
}

}  // namespace

SceneEntryKey BuildSceneEntryKeyFromSceneIndex(uint32_t sceneIndex) {
    SceneEntryKey key{};
    key.sceneIndexKnown = true;
    key.sceneIndex = sceneIndex;
    key.baseKnown = true;
    key.base =
        kSceneEntryTableBase8005474C + sceneIndex * kSceneEntryStride8005474C;
    return key;
}

uint32_t ComputeSceneEntryRowOffset(uint32_t rowIndex) {
    return kSceneEntryFirstMovieSegmentOffset +
           rowIndex * kSceneEntryMovieSegmentRowSize;
}

SceneEntryRowRole DefaultMovieSegmentRowRole(uint32_t rowIndex) {
    switch (rowIndex) {
    case 0:
        return SceneEntryRowRole::Comod;
    case 1:
        return SceneEntryRowRole::Compo;
    case 2:
        return SceneEntryRowRole::OpeningMovie;
    case 3:
        return SceneEntryRowRole::StageRuntime;
    case 4:
        return SceneEntryRowRole::ClearMovieA;
    case 5:
        return SceneEntryRowRole::ClearMovieB;
    case 6:
        return SceneEntryRowRole::ZCompo;
    default:
        return SceneEntryRowRole::Unknown;
    }
}

SceneEntryPathIdentity IdentifySceneEntryPathPtr(uint32_t pathPtr) {
    for (const SceneEntryPathTableRow& row : kKnownPathRows) {
        if (row.pathPtr != pathPtr) {
            continue;
        }
        SceneEntryPathIdentity out{};
        out.known = true;
        out.pathPtrKnown = true;
        out.pathPtr = row.pathPtr;
        out.role = row.role;
        out.psxPath = row.psxPath;
        out.relativeWinPath = row.relativeWinPath;
        return out;
    }
    return {};
}

SceneEntryRawRow GetSceneEntryStaticRawRow(uint32_t sceneIndex,
                                           uint32_t rowIndex) {
    if (sceneIndex >= kSceneEntrySceneCount ||
        rowIndex >= kSceneEntryMovieSegmentRowCount) {
        return {};
    }
    return BuildRawRow(kStaticRowsByScene[sceneIndex][rowIndex]);
}

SceneEntryRowIdentity IdentifySceneEntryRow(const SceneEntryKey& key,
                                            uint32_t rowIndex) {
    SceneEntryRowIdentity out{};
    if (rowIndex >= kSceneEntryMovieSegmentRowCount) {
        return out;
    }

    out.known = key.sceneIndexKnown || key.baseKnown;
    out.entry = key;
    out.rowIndexKnown = true;
    out.rowIndex = rowIndex;
    out.rowOffsetKnown = true;
    out.rowOffset = ComputeSceneEntryRowOffset(rowIndex);
    out.rowAddrKnown = key.baseKnown;
    if (out.rowAddrKnown) {
        out.rowAddr = key.base + out.rowOffset;
    }
    out.role = DefaultMovieSegmentRowRole(rowIndex);

    if (key.sceneIndexKnown && key.sceneIndex < kSceneEntrySceneCount) {
        out.raw = GetSceneEntryStaticRawRow(key.sceneIndex, rowIndex);
        const uint32_t pathPtr = out.raw.pathPtr;
        out.path = IdentifySceneEntryPathPtr(pathPtr);
        if (!out.path.pathPtrKnown) {
            out.path.pathPtrKnown = true;
            out.path.pathPtr = pathPtr;
        }
    }
    return out;
}

SceneEntryRowIdentity IdentifySceneEntryRowByOffset(const SceneEntryKey& key,
                                                    uint32_t rowOffset) {
    if (rowOffset < kSceneEntryFirstMovieSegmentOffset) {
        return {};
    }
    const uint32_t delta = rowOffset - kSceneEntryFirstMovieSegmentOffset;
    if ((delta % kSceneEntryMovieSegmentRowSize) != 0u) {
        return {};
    }
    return IdentifySceneEntryRow(key, delta / kSceneEntryMovieSegmentRowSize);
}

SceneCallbackTriplet80048D28 GetSceneCallbackTriplet80048D28(
    uint32_t sceneIndex) {
    if (sceneIndex >= kSceneCallbackTable80048D28.size()) {
        return SceneCallbackTriplet80048D28{};
    }
    return kSceneCallbackTable80048D28[sceneIndex];
}

MainSceneState80015D18 InitMainSceneState80015D18() {
    MainSceneState80015D18 out{};
    out.word800916D4Known = true;
    out.word800916D4 = 0;
    out.word800916DEKnown = true;
    out.word800916DE = 0;
    out.currentSceneV0Known = true;
    out.currentSceneV0 = 0;
    out.previousSceneV1Known = true;
    out.previousSceneV1 = 0;
    return out;
}

MainSceneStepResult80015D18
BeginMainSceneLoopIteration80015D18(const MainSceneState80015D18& in) {
    MainSceneStepResult80015D18 out{};
    out.state = in;

    if (!in.word800916DEKnown || in.word800916DE != 1u) {
        return out;
    }

    out.state.word800916DEKnown = true;
    out.state.word800916DE = 0;
    if (in.currentSceneV0Known && in.currentSceneV0 == 0) {
        out.state.previousSceneV1Known = true;
        out.state.previousSceneV1 = 0;
    }

    EmitMainSceneRequest(
        out.trace,
        MainSceneRequestKind80015D18::Call8001EF14,
        kFn8001EF14);

    EmitMainSceneRequest(
        out.trace,
        MainSceneRequestKind80015D18::Call80015788,
        kFn80015788,
        out.state.previousSceneV1Known,
        out.state.previousSceneV1);
    out.waitingFor80015788Result = true;
    return out;
}

MainSceneState80015D18
ApplyMainSceneSwitchResult80015D18(const MainSceneState80015D18& in,
                                   int32_t switchResult) {
    MainSceneState80015D18 out = in;
    out.currentSceneV0Known = true;
    out.currentSceneV0 = switchResult;
    out.previousSceneV1Known = true;
    out.previousSceneV1 = switchResult;
    return out;
}

MainSceneStepResult80015D18
PrepareMainSceneCallbacks80015D18(const MainSceneState80015D18& in) {
    MainSceneStepResult80015D18 out{};
    out.state = in;
    if (!in.currentSceneV0Known) {
        return out;
    }
    if (in.currentSceneV0 < 0 ||
        in.currentSceneV0 >=
            static_cast<int32_t>(kSceneCallbackTableSceneCount80048D28)) {
        out.invalidCurrentSceneIndex = true;
        return out;
    }

    const SceneEntryKey key =
        BuildSceneEntryKeyFromSceneIndex(static_cast<uint32_t>(in.currentSceneV0));
    const int32_t loaderRowAddr =
        static_cast<int32_t>(key.base + kSceneEntryFirstMovieSegmentOffset);
    const int32_t sceneEntryAddr = static_cast<int32_t>(key.base);
    const SceneCallbackTriplet80048D28 callbacks =
        GetSceneCallbackTriplet80048D28(
            static_cast<uint32_t>(in.currentSceneV0));

    EmitMainSceneRequest(
        out.trace,
        MainSceneRequestKind80015D18::Call80025A34,
        kFn80025A34);

    out.state.word800916E2Known = true;
    out.state.word800916E2 = static_cast<uint16_t>(in.currentSceneV0);

    EmitMainSceneRequest(
        out.trace,
        MainSceneRequestKind80015D18::Call8001A324,
        kFn8001A324,
        true,
        loaderRowAddr);
    EmitMainSceneRequest(
        out.trace,
        MainSceneRequestKind80015D18::Call800154B0,
        kFn800154B0,
        true,
        loaderRowAddr,
        true,
        0);
    EmitMainSceneRequest(
        out.trace,
        MainSceneRequestKind80015D18::Call80025A34,
        kFn80025A34);
    EmitMainSceneRequest(
        out.trace,
        MainSceneRequestKind80015D18::CallSceneFn0,
        callbacks.fn0,
        false,
        0,
        false,
        0,
        true,
        callbacks.fn0Slot);
    EmitMainSceneRequest(
        out.trace,
        MainSceneRequestKind80015D18::CallSceneFn1,
        callbacks.fn1,
        true,
        sceneEntryAddr,
        true,
        in.currentSceneV0,
        true,
        callbacks.fn1Slot);
    EmitMainSceneRequest(
        out.trace,
        MainSceneRequestKind80015D18::CallSceneFn2,
        callbacks.fn2,
        true,
        in.currentSceneV0,
        false,
        0,
        true,
        callbacks.fn2Slot);
    out.waitingForSceneFn2Result = true;
    return out;
}

MainSceneStepResult80015D18
CompleteMainSceneLoopIteration80015D18(const MainSceneState80015D18& in,
                                       int32_t sceneFn2Result) {
    MainSceneStepResult80015D18 out{};
    out.state = in;

    if (in.word800916D0Known && in.word800916D0 == 1u) {
        out.state.word800916EEKnown = true;
        out.state.word800916EE = sceneFn2Result;
    }

    if (in.currentSceneV0Known && in.currentSceneV0 == 0 &&
        sceneFn2Result > 0) {
        if (!in.word800916D0Known) {
            out.missingWord800916D0For15CC4Gate = true;
        } else if (in.word800916D0 != 1u) {
            EmitMainSceneRequest(
                out.trace,
                MainSceneRequestKind80015D18::Call80015CC4,
                kFn80015CC4);
        }
    }

    if (sceneFn2Result >= 0) {
        out.state.currentSceneV0Known = true;
        out.state.currentSceneV0 = sceneFn2Result;
        out.state.word800916D4Known = true;
        out.state.word800916D4 = static_cast<uint16_t>(sceneFn2Result);
    } else {
        out.state.word800916DEKnown = true;
        out.state.word800916DE = 1;
    }

    EmitMainSceneRequest(
        out.trace,
        MainSceneRequestKind80015D18::Call8001E34C,
        kFn8001E34C);
    return out;
}

Scene0TitleSelectorResult801C4DC4
ResolveScene0TitleSelectorResult801C4DC4(int selectorResult,
                                         int scene,
                                         int16_t lastWord800916EE) {
    Scene0TitleSelectorResult801C4DC4 out{};
    out.known = true;
    out.valid = true;

    switch (selectorResult) {
    case 1:
        out.word800916D0 = 0;
        out.fn2ReturnScene = -1;
        return out;

    case 3: {
        out.word800916D0 = 1;
        int randomScene = std::rand() % 6 + 1;
        while (lastWord800916EE >= 1 && lastWord800916EE <= 6 &&
               randomScene == lastWord800916EE) {
            randomScene = std::rand() % 6 + 1;
        }

        out.fn2ReturnScene = randomScene;
        out.randomScene = randomScene;
        out.word800916EEWrite = true;
        out.word800916EE = static_cast<int16_t>(randomScene);
        return out;
    }

    default:
        out.word800916D0 = 0;
        out.fn2ReturnScene = scene + 1;
        return out;
    }
}

GenericSwitchState80015788 InitGenericSwitchState80015788(int32_t prevScene) {
    GenericSwitchState80015788 out{};
    out.phase = GenericSwitchPhase80015788::Start;
    out.prevSceneA1Known = true;
    out.prevSceneA1 = prevScene;
    return out;
}

Call800191E4Result80015788
PsxCall800191E4_MemcardStateMachine80015788(
    int32_t a1,
    bool a1Known,
    int32_t mode,
    bool modeKnown,
    const Call80019414Feedback80015788& feedback) {
    Call800191E4Result80015788 out{};
    out.sourceFunction = kFn800191E4;
    out.arg0Known = a1Known;
    out.arg0 = a1;
    out.modeKnown = modeKnown;
    out.mode = mode;

    out.called80026784 = true;
    out.sub80026784ResultKnown = feedback.sub80026784ResultKnown;
    out.sub80026784Result = feedback.sub80026784Result;
    out.copied36BytesTo8007CC50 = feedback.sub80026784ResultKnown;
    out.wroteGp716Zero = true;
    out.wroteGp732Mode = modeKnown;
    out.called80017524 = true;
    out.called80018FB0 = true;
    out.callback80018E10 = kFn80018E10;
    out.callback80019D7C = kFn80019D7C;
    out.stateMachineArg3 = 20;
    out.stateMachineArg4 = 3;
    out.called80017574 = true;

    if (!feedback.gp716AfterStateMachineKnown) {
        out.missingGp716AfterStateMachine = true;
        return out;
    }

    out.gp716AfterStateMachineKnown = true;
    out.gp716AfterStateMachine = feedback.gp716AfterStateMachine;
    if (feedback.gp716AfterStateMachine != 1) {
        out.resultKnown = true;
        out.result = -1;
        return out;
    }

    if (!feedback.wordA1Plus44Known) {
        out.missingA1Plus44 = true;
        return out;
    }

    out.wordA1Plus44Known = true;
    out.wordA1Plus44 = feedback.wordA1Plus44;
    out.resultKnown = true;
    out.result = feedback.wordA1Plus44;
    return out;
}

Call80019284Result80015788
PsxCall80019284_BuildHiScoreRecords80015788(
    int32_t a1,
    bool a1Known,
    const uint8_t* a1Memory,
    size_t a1MemorySize,
    const uint8_t* initialTableMemory,
    size_t initialTableMemorySize) {
    Call80019284Result80015788 out{};
    out.sourceFunction = kFn80019284;
    out.arg0Known = a1Known;
    out.arg0 = a1;
    out.inputSize = a1MemorySize;
    out.initialTableSize = initialTableMemorySize;

    if (!a1Memory ||
        a1MemorySize < kHiScoreInputRequiredSize80019284) {
        out.missingInputRange = true;
        return out;
    }

    out.inputRangeKnown = true;
    out.resultKnown = true;
    out.result = static_cast<int32_t>(kDword80049278Address);
    out.tableAsciiKnown = true;
    if (initialTableMemory &&
        initialTableMemorySize >= kHiScoreTableSize80019284) {
        out.initialTableKnown = true;
        std::memcpy(
            out.tableBytes,
            initialTableMemory,
            kHiScoreTableSize80019284);
    }

    size_t cellIndex = 0u;
    for (size_t row = 0u; row < kHiScoreRowCount80019284; ++row) {
        for (size_t col = 0u; col < kHiScoreColumnCount80019284; ++col) {
            HiScoreCell80019284& cell = out.cells[cellIndex];
            const size_t scoreOffset =
                kHiScoreScoreBase80019284 +
                row * kHiScoreRowStride80019284 +
                col * kHiScoreColumnStride80019284;
            const size_t nameOffset =
                kHiScoreNameBase80019284 +
                row * kHiScoreRowStride80019284 +
                col * kHiScoreColumnStride80019284;
            const size_t destOffset =
                kHiScoreTableHeaderSize80019284 +
                cellIndex * kHiScoreRecordStride80019284;

            cell.scoreKnown = true;
            cell.score = ReadS32LE(a1Memory + scoreOffset);
            cell.nameKnown = true;
            std::memcpy(cell.name, a1Memory + nameOffset, 3u);
            cell.name[3] = '\0';
            cell.destinationOffset = static_cast<uint32_t>(destOffset);

            if (cell.score > 0) {
                std::snprintf(
                    cell.formattedAscii,
                    sizeof(cell.formattedAscii),
                    "%4d %-3.3s",
                    cell.score,
                    cell.name);
                cell.formattedAsciiKnown = true;
                cell.requested80017FC4Copy = true;
                const GlyphCopyResult80017FC4 glyph =
                    ApplyGlyphMap80017FC4(
                        cell.glyphBytes,
                        kHiScoreRecordStride80019284,
                        cell.formattedAscii);
                cell.glyphBytesKnown = glyph.known;
                cell.glyphByteWriteCount = glyph.writeCount;
                if (cell.glyphBytesKnown) {
                    std::memcpy(
                        out.tableBytes + destOffset,
                        cell.glyphBytes,
                        cell.glyphByteWriteCount);
                    if (cell.glyphByteWriteCount <
                            kHiScoreRecordStride80019284 &&
                        !out.initialTableKnown) {
                        out.glyphRecordTailCarry80017FC4 = true;
                    }
                } else {
                    out.glyphCopyOverflow80017FC4 = true;
                }
            } else {
                out.tableBytes[destOffset] = 0;
                // 80019284 only writes the first byte for empty records; the
                // rest remains prior dword_80049278 contents on PSX.
                cell.glyphBytes[0] = 0;
                if (!out.initialTableKnown) {
                    out.nonPositiveRecordTailCarry80019284 = true;
                }
            }

            ++cellIndex;
        }
    }

    out.tablePsxGlyphBytesKnown =
        !out.glyphCopyOverflow80017FC4 &&
        !out.nonPositiveRecordTailCarry80019284 &&
        !out.glyphRecordTailCarry80017FC4;
    return out;
}

Call80019284InputMemory80015788
PsxBuild80019284InputMemoryFromStatusAndBank80015788(
    const uint8_t* statusPrefix80092F10,
    size_t statusPrefixSize,
    const HiScoreBankCarrier800164F8& bank) {
    Call80019284InputMemory80015788 out{};
    out.statusPrefixSize = statusPrefixSize;
    std::memcpy(
        out.initialTableMemory,
        kHiScoreInitialTable80049278,
        kHiScoreTableSize80019284);

    if (!statusPrefix80092F10 ||
        statusPrefixSize < kHiScoreStatusPrefixSize80019284) {
        out.missingStatusPrefix = true;
        return out;
    }

    std::memcpy(
        out.a1Memory,
        statusPrefix80092F10,
        kHiScoreStatusPrefixSize80019284);

    bool bankKnown = true;
    for (size_t row = 0u; row < kHiScoreBankRowCount800164F8; ++row) {
        for (size_t slot = 0u; slot < kHiScoreBankSlotCount800164F8;
             ++slot) {
            const HiScoreBankSlot800164F8& bankSlot =
                bank.rows[row].slots[slot];
            if (!bankSlot.valid || !bankSlot.scoreKnown ||
                !bankSlot.name.known) {
                bankKnown = false;
                continue;
            }

            const size_t offset =
                kHiScoreStatusPrefixSize80019284 +
                row * kHiScoreBankRowStride80016000 +
                slot * kHiScoreBankSlotStride80016000;
            std::memcpy(
                out.a1Memory + offset,
                bankSlot.name.bytes,
                kHiScoreBankNameSize80016000);
            WriteS32LE(
                out.a1Memory + offset + kHiScoreBankScoreOffset80016000,
                bankSlot.score);
        }
    }

    out.missingBankSlot = !bankKnown;
    out.inputMemoryKnown = bankKnown;
    return out;
}

Call80019414Result80015788
PsxCall80019414_HiScoreEntry80015788(
    int32_t a1,
    bool a1Known,
    const Call80019414Feedback80015788& feedback) {
    Call80019414Result80015788 out{};
    out.sourceFunction = kFn80019414;
    out.arg0Known = a1Known;
    out.arg0 = a1;

    out.call800191E4 =
        PsxCall800191E4_MemcardStateMachine80015788(
            a1,
            a1Known,
            3,
            true,
            feedback);

    if (!feedback.gp720Known) {
        out.missingGp720 = true;
        return out;
    }

    out.gp720Known = true;
    out.gp720 = feedback.gp720;
    if (feedback.gp720 != 1) {
        out.resultKnown = true;
        out.result = 0;
        return out;
    }

    out.called80019284 = true;
    out.call80019284Function = kFn80019284;
    if (!feedback.call80019284ResultKnown) {
        out.missing80019284Result = true;
        return out;
    }

    out.resultKnown = true;
    out.result = feedback.call80019284Result;
    out.event6HostArgPtr = feedback.call80019284HostArgPtr;
    return out;
}

HiScoreBankCarrier800164F8
PsxCall800168DC_ClearHiScoreBank80019D7C() {
    HiScoreBankCarrier800164F8 out{};
    out.psxAddress = kByte80092F10Address;
    out.clearedBy800168DC = true;
    for (HiScoreBankRow800164F8& row : out.rows) {
        for (HiScoreBankSlot800164F8& slot : row.slots) {
            slot.valid = true;
            slot.scoreKnown = true;
            slot.score = 0;
            slot.name.known = true;
            slot.name.nulTerminatedBeforeScore = true;
            slot.name.bytes[0] = 0;
        }
    }
    return out;
}

Call800164F8Result80019D7C
PsxCall800164F8_MergeSavePayloadHiScoreBank80019D7C(
    const HiScoreBankCarrier800164F8& bank,
    const uint8_t* payload,
    size_t payloadSize) {
    Call800164F8Result80019D7C out{};
    out.sourceFunction = kFn800164F8;
    out.beforeBank = bank;
    out.afterBank = bank;
    out.payload = ReadHiScoreSavePayload800164F8(payload, payloadSize);

    if (!out.payload.inputRangeKnown) {
        return out;
    }

    for (size_t row = 0u; row < kHiScoreBankRowCount800164F8; ++row) {
        HiScoreMergeRow800164F8& rowResult = out.rows[row];
        rowResult.candidateKnown =
            IsKnownHiScoreCString80016000(out.payload.name) &&
            out.payload.scoreKnown[row];
        if (!rowResult.candidateKnown) {
            continue;
        }

        const int32_t score = out.payload.score[row];
        const HiScoreBankName80016000& name = out.payload.name;
        const HiScoreBankRow800164F8& beforeRow = out.beforeBank.rows[row];
        for (size_t slot = 0u; slot < kHiScoreBankVisibleSlotCount800164F8;
             ++slot) {
            if (SameHiScoreSlotCandidate800164F8(
                    beforeRow.slots[slot],
                    name,
                    score)) {
                rowResult.duplicateInVisibleSlots = true;
                break;
            }
        }

        if (rowResult.duplicateInVisibleSlots) {
            continue;
        }

        HiScoreBankRow800164F8& afterRow = out.afterBank.rows[row];
        HiScoreBankSlot800164F8& scratch =
            afterRow.slots[kHiScoreBankScratchSlotIndex800164F8];
        scratch.valid = true;
        scratch.scoreKnown = true;
        scratch.score = score;
        scratch.name = name;
        afterRow.wroteScratchSlotBefore80016000 = true;
        afterRow.sortFunction80016000 = kFn80016000;
        afterRow.sortArg80016000 = 4;
        afterRow.sortConvergenceGap80016000 = false;
        afterRow = PsxCall80016000_SortHiScoreRow(afterRow, 4);

        rowResult.wroteScratchSlot = true;
        rowResult.sortFunction80016000 = kFn80016000;
        rowResult.sortArg80016000 = 4;
        rowResult.sortConvergenceGap80016000 = false;
        rowResult.sortedBy80016000 = true;
    }

    return out;
}

void ApplyCase17HiScoreRow80019D7C(Case17Result80019D7C& out, size_t row,
    const Case17CardRow80019D7C& inRow) {
    if (row >= out.cardRows.size()) return;
    auto& outRow = out.cardRows[row];
    outRow.clearedBlockBuffer80025C44 = true;
    outRow.rowEnabledKnown = inRow.rowEnabledKnown;
    outRow.rowEnabled = inRow.rowEnabled;
    if (!inRow.rowEnabledKnown || !inRow.rowEnabled) return;
    outRow.copiedRowNameTo8007CBE8 = true;
    outRow.cardReadFunction800179B4 = kFn800179B4;
    outRow.readResultKnown = inRow.readResultKnown || inRow.eventResult80016EB8Known;
    outRow.eventResult80016EB8Known = inRow.eventResult80016EB8Known;
    outRow.eventResult80016EB8 = inRow.eventResult80016EB8;
    const bool succeeded = inRow.eventResult80016EB8Known
        ? inRow.eventResult80016EB8 == 1 : inRow.readSucceeded;
    if (!outRow.readResultKnown || !succeeded) return;
    if (!inRow.rowMetadata8007AE14Known || !inRow.payloadPointerKnown ||
        !inRow.payloadPassedTo800164F8 || !inRow.payload ||
        inRow.payloadSize < kHiScoreSavePayloadRequiredSize800164F8) return;
    outRow.readSucceeded = true;
    outRow.rowMetadata8007AE14Known = true;
    outRow.rowMetadata8007AE14 = inRow.rowMetadata8007AE14;
    outRow.wroteRowMetadata = true;
    outRow.readBufferKnown = inRow.readBufferKnown;
    outRow.readBuffer = inRow.readBuffer;
    outRow.readLengthKnown = inRow.readLengthKnown;
    outRow.readLength = inRow.readLength;
    outRow.payloadPointerKnown = inRow.payloadPointerKnown;
    outRow.payloadPointer = inRow.payloadPointer;
    outRow.payloadPassedTo800164F8 = inRow.payloadPassedTo800164F8;
    outRow.mergeCalled = true;
    outRow.merge = PsxCall800164F8_MergeSavePayloadHiScoreBank80019D7C(
        out.bank, inRow.payload, inRow.payloadSize);
    out.bank = outRow.merge.afterBank;
}

Case17Result80019D7C
PsxCall80019D7C_Case17HiScoreBankCarrier(
    int32_t a2,
    bool a2Known,
    const Case17Feedback80019D7C& feedback) {
    Case17Result80019D7C out{};
    out.sourceFunction = kFn80019D7C;
    out.arg2Known = a2Known;
    out.arg2 = a2;

    if (!a2Known) {
        out.missingArg2ForCase17Gate = true;
        return out;
    }

    if (a2 == kCase17Arg2EarlyReturn80019D7C) {
        out.returnedEarlyForArg2Equals3 = true;
        out.resultKnown = true;
        out.result = kCase17EarlyResult80019D7C;
        return out;
    }

    out.called800168DC = true;
    out.clearFunction800168DC = kFn800168DC;
    out.bank = PsxCall800168DC_ClearHiScoreBank80019D7C();

    if (!feedback.word8007ABE4Known) {
        out.missingWord8007ABE4 = true;
    } else if (feedback.word8007ABE4 > 0) {
        out.calledVSyncCallback80017F38 = true;
        for (size_t row = 0u; row < kHiScoreCardRowCount80019D7C; ++row) {
            ApplyCase17HiScoreRow80019D7C(out, row, feedback.cardRows[row]);
        }
        out.clearedVSyncCallback = true;
    }

    out.gp720Written = true;
    out.gp720 = kCase17Gp720DoneValue80019D7C;
    out.resultKnown = true;
    out.result = kCase17DoneResult80019D7C;
    return out;
}

GenericSwitchStepResult80015788
StepGenericSwitch80015788(const GenericSwitchState80015788& in,
                          const GenericSwitchFeedback80015788& feedback) {
    GenericSwitchStepResult80015788 out{};
    out.state = in;

    if (feedback.word800916D0Written) {
        out.state.word800916D0Known = true;
        out.state.word800916D0 = feedback.word800916D0;
    }

    switch (in.phase) {
    case GenericSwitchPhase80015788::Start:
        EmitGenericSwitchRequest(
            out.trace,
            GenericSwitchRequestKind80015788::Call80026FA4,
            kFn80026FA4,
            in.prevSceneA1Known,
            in.prevSceneA1);
        EmitGenericSwitchRequest(
            out.trace,
            GenericSwitchRequestKind80015788::Call80026EF8,
            kFn80026EF8);
        EmitGenericSwitchRequest(
            out.trace,
            GenericSwitchRequestKind80015788::Call80026ECC,
            kFn80026ECC);
        EmitGenericSwitchRequest(
            out.trace,
            GenericSwitchRequestKind80015788::Call80015590,
            kFn80015590,
            in.prevSceneA1Known,
            in.prevSceneA1);
        out.state.phase = GenericSwitchPhase80015788::WaitInput35510;
        return out;

    case GenericSwitchPhase80015788::WaitInput35510:
        if (!feedback.poll35510Known) {
            EmitGenericSwitchRequest(
                out.trace,
                GenericSwitchRequestKind80015788::Call80035510,
                kFn80035510,
                true,
                1);
            out.waitingForFeedback = true;
            return out;
        }
        if (feedback.poll35510Result != 0) {
            out.waitingForFeedback = true;
            return out;
        }
        EmitGenericSwitchRequest(
            out.trace,
            GenericSwitchRequestKind80015788::Call80026B94,
            kFn80026B94,
            true,
            3,
            true,
            static_cast<int32_t>(kWord800916D0Address));
        out.state.phase = GenericSwitchPhase80015788::WaitEvent3;
        out.waitingForFeedback = true;
        return out;

    case GenericSwitchPhase80015788::WaitEvent3:
        if (!feedback.event26B94ResultKnown) {
            out.waitingForFeedback = true;
            return out;
        }
        out.state.lastEventResultV2Known = true;
        out.state.lastEventResultV2 = feedback.event26B94Result;
        switch (feedback.event26B94Result) {
        case 1:
            EmitGenericSwitchRequest(
                out.trace,
                GenericSwitchRequestKind80015788::Call80019414,
                kFn80019414,
                true,
                static_cast<int32_t>(kByte80092F10Address));
            out.state.phase = GenericSwitchPhase80015788::WaitCall19414;
            out.waitingForFeedback = true;
            return out;
        case 2:
            EmitGenericSwitchRequest(
                out.trace,
                GenericSwitchRequestKind80015788::Call80015700,
                kFn80015700,
                true,
                static_cast<int32_t>(kByte80092F10Address));
            EmitGenericSwitchRequest(
                out.trace,
                GenericSwitchRequestKind80015788::Call800193F4,
                kFn800193F4,
                true,
                static_cast<int32_t>(kByte80092F10Address));
            out.state.phase = GenericSwitchPhase80015788::WaitCall193F4;
            out.waitingForFeedback = true;
            return out;
        case 3:
            EmitGenericSwitchRequest(
                out.trace,
                GenericSwitchRequestKind80015788::Call8002776C,
                kFn8002776C,
                true,
                static_cast<int32_t>(kUnk801C3640Address),
                in.prevSceneA1Known,
                in.prevSceneA1);
            out.state.phase = GenericSwitchPhase80015788::WaitPractice2776C;
            return out;
        case 4:
            EmitGenericSwitchRequest(
                out.trace,
                GenericSwitchRequestKind80015788::Call80026B94,
                kFn80026B94,
                true,
                2,
                true,
                0);
            out.state.phase = GenericSwitchPhase80015788::WaitEvent2;
            out.waitingForFeedback = true;
            return out;
        case 6:
            EmitGenericSwitchRequest(
                out.trace,
                GenericSwitchRequestKind80015788::Call800193B0,
                kFn800193B0,
                true,
                static_cast<int32_t>(kByte80092F10Address));
            out.state.phase = GenericSwitchPhase80015788::WaitSave193B0;
            return out;
        case 7:
            out.state.returnValueKnown = true;
            out.state.returnValue = 0;
            out.state.phase = GenericSwitchPhase80015788::Done;
            out.done = true;
            return out;
        case 8:
            EmitGenericSwitchRequest(
                out.trace,
                GenericSwitchRequestKind80015788::Call80026B94,
                kFn80026B94,
                true,
                17,
                true,
                static_cast<int32_t>(kWord800916D0Address));
            out.state.phase = GenericSwitchPhase80015788::WaitEvent17;
            out.waitingForFeedback = true;
            return out;
        default:
            out.state.phase = GenericSwitchPhase80015788::WaitInput35510;
            return out;
        }

    case GenericSwitchPhase80015788::WaitCall19414:
        if (!feedback.call19414ResultKnown) {
            out.waitingForFeedback = true;
            return out;
        }
        if (feedback.call19414Result != 0) {
            EmitGenericSwitchRequest(
                out.trace,
                GenericSwitchRequestKind80015788::Call80026B94,
                kFn80026B94,
                true,
                6,
                true,
                feedback.call19414Result);
            out.state.phase = GenericSwitchPhase80015788::WaitEvent6;
            out.waitingForFeedback = true;
            return out;
        }
        out.state.phase = GenericSwitchPhase80015788::WaitInput35510;
        return out;

    case GenericSwitchPhase80015788::WaitCall193F4:
        if (!feedback.call193F4ResultKnown) {
            out.waitingForFeedback = true;
            return out;
        }
        if (feedback.call193F4Result >= 0) {
            out.state.replaySlotV3Known = true;
            out.state.replaySlotV3 = feedback.call193F4Result;
            out.state.word800916D0Known = true;
            out.state.word800916D0 = 2;
            EmitGenericSwitchRequest(
                out.trace,
                GenericSwitchRequestKind80015788::Call800161A8,
                kFn800161A8,
                true,
                feedback.call193F4Result);
            out.state.phase = GenericSwitchPhase80015788::WaitCall161A8;
            out.waitingForFeedback = true;
            return out;
        }
        out.state.phase = GenericSwitchPhase80015788::WaitInput35510;
        return out;

    case GenericSwitchPhase80015788::WaitCall161A8:
        if (!feedback.call161A8ResultKnown) {
            out.waitingForFeedback = true;
            return out;
        }
        out.state.returnValueKnown = true;
        out.state.returnValue = feedback.call161A8Result;
        out.state.phase = GenericSwitchPhase80015788::Done;
        out.done = true;
        return out;

    case GenericSwitchPhase80015788::WaitEvent2:
        if (!feedback.event26B94ResultKnown) {
            out.waitingForFeedback = true;
            return out;
        }
        if (feedback.event26B94Result == 1) {
            if (!feedback.event26B94OutArgKnown) {
                out.waitingForFeedback = true;
                return out;
            }
            out.state.returnValueKnown = true;
            out.state.returnValue = feedback.event26B94OutArg;
            out.state.phase = GenericSwitchPhase80015788::Done;
            out.done = true;
            return out;
        }
        out.state.phase = GenericSwitchPhase80015788::WaitInput35510;
        return out;

    case GenericSwitchPhase80015788::WaitPractice2776C:
        if (!feedback.call2776CDoneKnown) {
            out.waitingForFeedback = true;
            return out;
        }
        out.state.phase = GenericSwitchPhase80015788::WaitInput35510;
        return out;

    case GenericSwitchPhase80015788::WaitEvent6:
        if (!feedback.event26B94ResultKnown) {
            out.waitingForFeedback = true;
            return out;
        }
        out.state.phase = GenericSwitchPhase80015788::WaitInput35510;
        return out;

    case GenericSwitchPhase80015788::WaitSave193B0:
        if (!feedback.call193B0DoneKnown) {
            out.waitingForFeedback = true;
            return out;
        }
        out.state.phase = GenericSwitchPhase80015788::WaitInput35510;
        return out;

    case GenericSwitchPhase80015788::WaitEvent17:
        if (!feedback.event26B94ResultKnown) {
            out.waitingForFeedback = true;
            return out;
        }
        out.state.phase = GenericSwitchPhase80015788::WaitInput35510;
        return out;

    case GenericSwitchPhase80015788::Done:
        out.done = true;
        return out;
    }

    return out;
}

}  // namespace PrSceneEntryDirect
