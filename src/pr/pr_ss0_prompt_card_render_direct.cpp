#include "pr_ss0_prompt_card_render_direct.h"

namespace PrSS0PromptCardRenderDirect {
namespace {

constexpr PromptCardRouteSpec kRoutes[] = {
    {PromptCardPage::Event4Prompt,
     4,
     0,
     kFn800203D4,
     "event=4 modal prompt draw",
     "8001E750 event4 state 0 calls 800203D4(ctx0); wrapper/backdrop remain event-frame facts"},
    {PromptCardPage::CardInfoEv5,
     5,
     0,
     kFn80020BE4,
     "event=5 card info/practice info draw",
     "draw-only lower card page; marker strip and string buffer are arg-owned"},
    {PromptCardPage::HiScoreEv6,
     6,
     0,
     kFn80021594,
     "event=6 hi-score confirmation draw",
     "80026B94 event6 uses arg as ctx; page draw owns table anchors only"},
    {PromptCardPage::PromptType4,
     11,
     4,
     kFn80022CBC,
     "event=11 save confirm prompt",
     "card loop prompt id; type 4 dual-choice layout"},
    {PromptCardPage::PromptType1,
     12,
     1,
     kFn80022CBC,
     "event=12 insert card prompt",
     "card loop prompt id; type 1 single-button layout"},
    {PromptCardPage::PromptType2,
     13,
     2,
     kFn80022CBC,
     "event=13 format card prompt",
     "card loop prompt id; type 2 dual-choice layout"},
    {PromptCardPage::PromptType3,
     14,
     3,
     kFn80022CBC,
     "event=14 no space prompt",
     "card loop prompt id; type 3 single-button layout"},
    {PromptCardPage::PromptType5,
     15,
     5,
     kFn80022CBC,
     "event=15 rename save prompt",
     "card loop prompt id; type 5 single-button layout"},
    {PromptCardPage::PromptType6,
     18,
     6,
     kFn80022CBC,
     "event=18 unreadable card prompt",
     "card loop prompt id; type 6 single-button layout"},
    {PromptCardPage::PromptType7,
     19,
     7,
     kFn80022CBC,
     "event=19 overwrite prompt",
     "card loop prompt id; type 7 dual-choice layout plus optional I/O banner"},
};

constexpr PromptCardAnchorSpec kAnchors[] = {
    {PromptCardPage::Event4Prompt,
     "prompt title",
     56,
     57,
     0x80050950u,
     0,
     0,
     "always drawn"},
    {PromptCardPage::Event4Prompt,
     "left choice default/selected",
     70,
     149,
     0x80050960u,
     0x80050980u,
     0,
     "ctx0==0 selects template1; otherwise template0"},
    {PromptCardPage::Event4Prompt,
     "right choice default/selected",
     178,
     152,
     0x80050970u,
     0x80050990u,
     0,
     "ctx0==1 selects template1; otherwise template0"},
    {PromptCardPage::CardInfoEv5,
     "page title",
     36,
     28,
     0,
     0,
     0,
     "title text/table fact still pending"},
    {PromptCardPage::CardInfoEv5,
     "panel/string origin",
     117,
     64,
     0,
     0,
     0,
     "8001B730 then 8001C668(arg+0x28)"},
    {PromptCardPage::CardInfoEv5,
     "lower marker A",
     223,
     160,
     0,
     0,
     0,
     "fixed lower page anchor"},
    {PromptCardPage::CardInfoEv5,
     "lower marker B",
     232,
     169,
     0,
     0,
     0,
     "fixed lower page anchor"},
    {PromptCardPage::CardInfoEv5,
     "lower marker C",
     232,
     188,
     0,
     0,
     0,
     "fixed lower page anchor"},
    {PromptCardPage::HiScoreEv6,
     "title",
     37,
     31,
     0,
     0,
     0,
     "hi-score title table/template facts pending"},
    {PromptCardPage::HiScoreEv6,
     "column 0",
     84,
     56,
     0,
     0,
     0,
     "top column/header anchor"},
    {PromptCardPage::HiScoreEv6,
     "column 1",
     163,
     56,
     0,
     0,
     0,
     "top column/header anchor"},
    {PromptCardPage::HiScoreEv6,
     "column 2",
     242,
     56,
     0,
     0,
     0,
     "top column/header anchor"},
    {PromptCardPage::HiScoreEv6,
     "shared exit",
     231,
     179,
     kSpriteSharedExitIconOff80050AC0,
     kSpriteSharedExitIconOn80050AD0,
     kTableSharedExitText80053000,
     "EXIT anchor shared with other directory pages"},
    {PromptCardPage::PromptType1,
     "single prompt body",
     36,
     54,
     0x80052330u,
     0,
     kTablePromptType1Header80053A08,
     "single-button prompt frame"},
    {PromptCardPage::PromptType2,
     "dual prompt body",
     34,
     54,
     0x80052340u,
     0,
     kTablePromptType2Header80053990,
     "dual-choice prompt frame"},
    {PromptCardPage::PromptType3,
     "single prompt body",
     36,
     54,
     0x80052330u,
     0,
     kTablePromptType3Header800539B8,
     "single-button prompt frame"},
    {PromptCardPage::PromptType4,
     "dual prompt body",
     28,
     56,
     0x80052350u,
     0,
     kTablePromptType4Header80053A30,
     "dual-choice prompt frame"},
    {PromptCardPage::PromptType5,
     "single prompt body",
     36,
     54,
     0x80052330u,
     0,
     kTablePromptType5Header80053A58,
     "single-button prompt frame"},
    {PromptCardPage::PromptType6,
     "single prompt body",
     36,
     54,
     0x80052330u,
     0,
     kTablePromptType6Header80053A80,
     "single-button prompt frame"},
    {PromptCardPage::PromptType7,
     "dual prompt body",
     28,
     56,
     0x80052350u,
     0,
     kTablePromptType7Header800539E0,
     "dual-choice prompt frame"},
    {PromptCardPage::PromptType2,
     "dual prompt select bar",
     224,
     149,
     0x800526B0u,
     0x800526A0u,
     0,
     "a2[0]==1 selects template1"},
    {PromptCardPage::PromptType4,
     "dual prompt select bar",
     224,
     149,
     0x800526B0u,
     0x800526A0u,
     0,
     "a2[0]==1 selects template1"},
    {PromptCardPage::PromptType7,
     "dual prompt select bar",
     224,
     149,
     0x800526B0u,
     0x800526A0u,
     0,
     "a2[0]==1 selects template1"},
    {PromptCardPage::PromptType2,
     "choice 0 fixed sprite",
     234,
     159,
     0x80052590u,
     0x800525A0u,
     kTablePromptChoice0_80053AAC,
     "a2[1]==1 selects template1"},
    {PromptCardPage::PromptType4,
     "choice 0 fixed sprite",
     234,
     159,
     0x80052590u,
     0x800525A0u,
     kTablePromptChoice0_80053AAC,
     "a2[1]==1 selects template1"},
    {PromptCardPage::PromptType7,
     "choice 0 fixed sprite",
     234,
     159,
     0x80052590u,
     0x800525A0u,
     kTablePromptChoice0_80053AAC,
     "a2[1]==1 selects template1"},
    {PromptCardPage::PromptType2,
     "choice 1 fixed sprite",
     234,
     182,
     0x800526C0u,
     0x800526D0u,
     kTablePromptChoice1_80053AFC,
     "a2[1]==2 selects template1"},
    {PromptCardPage::PromptType4,
     "choice 1 fixed sprite",
     234,
     182,
     0x800526C0u,
     0x800526D0u,
     kTablePromptChoice1_80053AFC,
     "a2[1]==2 selects template1"},
    {PromptCardPage::PromptType7,
     "choice 1 fixed sprite",
     234,
     182,
     0x800526C0u,
     0x800526D0u,
     kTablePromptChoice1_80053AFC,
     "a2[1]==2 selects template1"},
    {PromptCardPage::PromptType1,
     "prompt family title",
     121,
     36,
     0x80052320u,
     0,
     0,
     "tail title for 80022CBC"},
    {PromptCardPage::PromptType2,
     "prompt family title",
     121,
     36,
     0x80052320u,
     0,
     0,
     "tail title for 80022CBC"},
    {PromptCardPage::PromptType3,
     "prompt family title",
     121,
     36,
     0x80052320u,
     0,
     0,
     "tail title for 80022CBC"},
    {PromptCardPage::PromptType4,
     "prompt family title",
     121,
     36,
     0x80052320u,
     0,
     0,
     "tail title for 80022CBC"},
    {PromptCardPage::PromptType5,
     "prompt family title",
     121,
     36,
     0x80052320u,
     0,
     0,
     "tail title for 80022CBC"},
    {PromptCardPage::PromptType6,
     "prompt family title",
     121,
     36,
     0x80052320u,
     0,
     0,
     "tail title for 80022CBC"},
    {PromptCardPage::PromptType7,
     "prompt family title",
     121,
     36,
     0x80052320u,
     0,
     0,
     "tail title for 80022CBC"},
};

constexpr PromptHeaderTableSpec kHeaderTables[] = {
    {PromptCardPage::PromptType1, 1, kTablePromptType1Header80053A08, 8, 5, "insert memory card"},
    {PromptCardPage::PromptType2, 2, kTablePromptType2Header80053990, 8, 5, "format memory card"},
    {PromptCardPage::PromptType3, 3, kTablePromptType3Header800539B8, 8, 5, "vacant block insufficient"},
    {PromptCardPage::PromptType4, 4, kTablePromptType4Header80053A30, 8, 5, "save confirm"},
    {PromptCardPage::PromptType5, 5, kTablePromptType5Header80053A58, 8, 5, "name already exists"},
    {PromptCardPage::PromptType6, 6, kTablePromptType6Header80053A80, 8, 5, "unformatted/unreadable card"},
    {PromptCardPage::PromptType7, 7, kTablePromptType7Header800539E0, 8, 5, "overwrite confirm"},
};

constexpr PromptHeaderEntrySpec kHeaderEntries[] = {
    {1, 0, 0x80052390u, 152, 58},
    {1, 1, 0x80052470u, 0, 0},
    {1, 2, 0x80052400u, 0, 0},
    {1, 3, 0x800524E0u, 0, 0},
    {1, 4, 0x80052550u, 0, 0},
    {2, 0, 0x80052360u, 131, 52},
    {2, 1, 0x80052440u, 0, 0},
    {2, 2, 0x800523D0u, 0, 0},
    {2, 3, 0x800524B0u, 0, 0},
    {2, 4, 0x80052520u, 0, 0},
    {3, 0, 0x80052370u, 134, 59},
    {3, 1, 0x80052450u, 0, 0},
    {3, 2, 0x800523E0u, 0, 0},
    {3, 3, 0x800524C0u, 0, 0},
    {3, 4, 0x80052530u, 0, 0},
    {4, 0, 0x800523A0u, 185, 71},
    {4, 1, 0x80052480u, 0, 0},
    {4, 2, 0x80052410u, 0, 0},
    {4, 3, 0x800524F0u, 0, 0},
    {4, 4, 0x80052560u, 0, 0},
    {5, 0, 0x800523B0u, 134, 51},
    {5, 1, 0x80052490u, 0, 0},
    {5, 2, 0x80052420u, 0, 0},
    {5, 3, 0x80052500u, 0, 0},
    {5, 4, 0x80052570u, 0, 0},
    {6, 0, 0x800523C0u, 154, 52},
    {6, 1, 0x800524A0u, 0, 0},
    {6, 2, 0x80052430u, 0, 0},
    {6, 3, 0x80052510u, 0, 0},
    {6, 4, 0x80052580u, 0, 0},
    {7, 0, 0x80052380u, 134, 69},
    {7, 1, 0x80052460u, 0, 0},
    {7, 2, 0x800523F0u, 0, 0},
    {7, 3, 0x800524D0u, 0, 0},
    {7, 4, 0x80052540u, 0, 0},
};

constexpr PromptChoiceEntrySpec kChoiceEntries[] = {
    {0, 0, 0x800525C0u, 0x800525D0u, 237, 162},
    {0, 1, 0x80052620u, 0x80052630u, 242, 163},
    {0, 2, 0x800525F0u, 0x80052600u, 237, 163},
    {0, 3, 0x80052650u, 0x80052660u, 241, 160},
    {0, 4, 0x80052680u, 0x80052690u, 241, 160},
    {1, 0, 0x800526F0u, 0x80052700u, 241, 185},
    {1, 1, 0x80052750u, 0x80052760u, 234, 185},
    {1, 2, 0x80052720u, 0x80052730u, 236, 185},
    {1, 3, 0x80052780u, 0x80052790u, 241, 185},
    {1, 4, 0x800527B0u, 0x800527C0u, 241, 185},
};

constexpr CardInfoMarkerEntrySpec kCardInfoMarkerEntries[] = {
    {0, 0x80051D10u, 31, 96},
    {1, 0x80051D50u, 49, 96},
    {2, 0x80051D70u, 68, 96},
    {3, 0x80051DA0u, 85, 96},
    {4, 0x80051DC0u, 103, 96},
    {5, 0x80051DF0u, 121, 96},
    {6, 0x80051E10u, 140, 96},
    {7, 0x80051E20u, 158, 96},
    {8, 0x80051E30u, 175, 96},
    {9, 0x80051E40u, 193, 96},
    {10, 0x80051E50u, 210, 96},
    {11, 0x80051E60u, 229, 96},
    {12, 0x80051E70u, 248, 96},
    {13, 0x80051EB0u, 265, 96},
    {14, 0x80051EC0u, 32, 116},
    {15, 0x80051ED0u, 49, 116},
    {16, 0x80051F10u, 68, 116},
    {17, 0x80051F30u, 86, 116},
    {18, 0x80051F40u, 104, 116},
    {19, 0x80051F60u, 122, 116},
    {20, 0x80051F70u, 140, 116},
    {21, 0x80051F80u, 158, 116},
    {22, 0x80051F90u, 175, 116},
    {23, 0x80051FA0u, 194, 116},
    {24, 0x80051FB0u, 212, 116},
    {25, 0x80051FD0u, 230, 116},
    {26, 0x80051C70u, 248, 116},
    {27, 0x80051C80u, 266, 116},
    {28, 0x80051C90u, 32, 137},
    {29, 0x80051CA0u, 50, 137},
    {30, 0x80051CB0u, 68, 137},
    {31, 0x80051CC0u, 86, 137},
    {32, 0x80051CD0u, 103, 137},
    {33, 0x80051CE0u, 122, 137},
    {34, 0x80051CF0u, 140, 137},
    {35, 0x80051C60u, 158, 137},
    {36, 0x80051E90u, 175, 137},
    {37, 0x80051DE0u, 194, 137},
    {38, 0x80051D40u, 210, 137},
    {39, 0x80051F50u, 230, 137},
    {40, 0x80051DB0u, 248, 136},
    {41, 0x80051D20u, 265, 135},
    {42, 0x80051F00u, 31, 161},
    {43, 0x80051D80u, 51, 161},
    {44, 0x80051DD0u, 71, 161},
    {45, 0x80051D30u, 91, 161},
    {46, 0x80051E00u, 110, 161},
    {47, 0x80051D60u, 130, 161},
    {48, 0x80051D00u, 151, 161},
    {49, 0x80051EF0u, 31, 185},
    {50, 0x80051D90u, 51, 185},
    {51, 0x80051EE0u, 72, 185},
    {52, 0x80051F20u, 91, 185},
    {53, 0x80051EA0u, 111, 185},
    {54, 0x80051E80u, 130, 185},
    {55, 0x80051FC0u, 193, 185},
    {56, 0x00000000u, 0, 0},
};

static_assert(sizeof(kCardInfoMarkerEntries) /
                  sizeof(kCardInfoMarkerEntries[0]) ==
              kCardInfoMarkerCount80020BE4,
              "80020BE4 marker entry count must stay PSX-sized");

constexpr CardIoTextEntrySpec kCardIoTextEntries[] = {
    {0, 0, kTableCardIoRemoveText8005326C, 0x80011130u, "remove-card English"},
    {0, 1, kTableCardIoRemoveText8005326C, 0x80011110u, "remove-card German"},
    {0, 2, kTableCardIoRemoveText8005326C, 0x800110ECu, "remove-card French"},
    {0, 3, kTableCardIoRemoveText8005326C, 0x800110C8u, "remove-card Italian"},
    {0, 4, kTableCardIoRemoveText8005326C, 0x800110A4u, "remove-card Spanish"},
    {1, 0, kTableCardIoPleaseWaitText80053280, 0x800111B4u, "please-wait English"},
    {1, 1, kTableCardIoPleaseWaitText80053280, 0x80011198u, "please-wait German"},
    {1, 2, kTableCardIoPleaseWaitText80053280, 0x80011174u, "please-wait French"},
    {1, 3, kTableCardIoPleaseWaitText80053280, 0x80011160u, "please-wait Italian"},
    {1, 4, kTableCardIoPleaseWaitText80053280, 0x8001114Cu, "please-wait Spanish"},
    {2, 0, 0, kTextCardIoNowSaving800111CC, "now-saving English literal"},
    {2, 1, kTableCardIoRemoveText8005326C, 0x80011110u, "save/remove-card German"},
    {2, 2, kTableCardIoRemoveText8005326C, 0x800110ECu, "save/remove-card French"},
    {2, 3, kTableCardIoRemoveText8005326C, 0x800110C8u, "save/remove-card Italian"},
    {2, 4, kTableCardIoRemoveText8005326C, 0x800110A4u, "save/remove-card Spanish"},
};

constexpr CardInfoLowerTextEntrySpec kCardInfoLowerTextEntries[] = {
    {0, 0, 0x8005349Cu, 0x80052010u, 0x80052020u, 0x80052030u, 234, 171},
    {0, 1, 0x800534ACu, 0x80052070u, 0x80052080u, 0x80052090u, 240, 172},
    {0, 2, 0x800534BCu, 0x80052040u, 0x80052050u, 0x80052060u, 234, 172},
    {0, 3, 0x800534CCu, 0x800520A0u, 0x800520B0u, 0x800520C0u, 234, 173},
    {0, 4, 0x800534DCu, 0x800520D0u, 0x800520E0u, 0x800520F0u, 238, 172},
    {1, 0, 0x800534ECu, 0x80052150u, 0x80052160u, 0x80052170u, 232, 191},
    {1, 1, 0x800534FCu, 0x800521B0u, 0x800521C0u, 0x800521D0u, 232, 193},
    {1, 2, 0x8005350Cu, 0x80052180u, 0x80052190u, 0x800521A0u, 232, 192},
    {1, 3, 0x8005351Cu, 0x800521E0u, 0x800521F0u, 0x80052200u, 232, 191},
    {1, 4, 0x8005352Cu, 0x80052210u, 0x80052220u, 0x80052230u, 233, 192},
};

static_assert(sizeof(kCardInfoLowerTextEntries) /
                  sizeof(kCardInfoLowerTextEntries[0]) ==
              static_cast<uint32_t>(kCardIoLanguageCount80020A3C * 2),
              "80020BE4 lower text records must stay PSX-sized");

constexpr HiScoreLanguageTitleEntrySpec kHiScoreLanguageTitleEntries[] = {
    {0, 0x80052E60u, 52, 34},
    {1, 0x80052E80u, 49, 34},
    {2, 0x80052E70u, 42, 34},
    {3, 0x80052E90u, 61, 34},
    {4, 0x80052EA0u, 40, 34},
};

constexpr HiScoreExitLabelEntrySpec kHiScoreExitLabelEntries[] = {
    {0, 0x800509E0u, 0x800509F0u, 242, 191},
    {1, 0x80050A40u, 0x80050A50u, 238, 193},
    {2, 0x80050A10u, 0x80050A20u, 238, 193},
    {3, 0x80050A70u, 0x80050A80u, 241, 190},
    {4, 0x80050AA0u, 0x80050AB0u, 239, 191},
};

constexpr HiScoreFixedSpriteSpec kHiScoreFixedSprites[] = {
    {0, 0, kSpriteHiScoreTitlePanel80052E50, 37, 31},
    {1, 0, kSpriteHiScoreColumn0_80052EB0, 84, 56},
    {1, 1, kSpriteHiScoreColumn1_80052EC0, 163, 56},
    {1, 2, kSpriteHiScoreColumn2_80052ED0, 242, 56},
    {2, 0, kSpriteHiScoreRow0_80052F10, 37, 69},
    {2, 1, 0x80052F20u, 37, 87},
    {2, 2, 0x80052F30u, 37, 105},
    {2, 3, 0x80052F40u, 37, 123},
    {2, 4, 0x80052F50u, 37, 141},
    {2, 5, 0x80052F60u, 37, 159},
};

constexpr HiScoreGp0ReplayBaselineSpec kHiScoreGp0ReplayBaselines[] = {
    {12408,
     "hiscore_first_draw_event6",
     294,
     1295,
     "ba43bcc481311e1fa3d7a0d3a009aa4b9c00ec4d22ed95ee62b2157076bf5eaa",
     "868cdd0499a9836c003c1d494aeb7e842648288f64204ea2c69af734269716f1"},
    {12497,
     "hiscore_exit_input_event6",
     380,
     1682,
     "bb09aed56b8b334d6896e449558f5f43b0a0abfd1692354298282cbd7d570053",
     "25bac2e5b927c7955356b7a3e233cf71d2530d93a34f097f8df97bb813b1e8da"},
    {12558,
     "hiscore_tail_return_main_menu",
     380,
     1682,
     "9ec0f2ab96c2ca531c253885c07522829b15d6d09b9333d28eb9c0b7db77c1d1",
     "cae9c70a2b611882eaf97f9d884dd6c2da65c6902e19b559623621798f4ce26d"},
};

constexpr PromptCardGp0ReplayBaselineSpec kPromptCardGp0ReplayBaselines[] = {
    {PromptCardPage::PromptType4,
     11,
     4,
     11878,
     "Stage1SavePrompt",
     "save_confirm_prompt_type4_first_draw",
     202,
     887,
     "c89f9c84c82b35a7604957cb99be0a4d7fb23778ae0f3aa12e8050d696bfc02a",
     "22d71c36ed93ac285c0f6bc7d1f7d05874920c4523f0172188c969194846c32f"},
    {PromptCardPage::PromptType4,
     11,
     4,
     11879,
     "Stage1SavePrompt",
     "save_confirm_prompt_type4_adjacent_draw",
     202,
     887,
     "4d873ad65091df29baf1bf58884dbdebcc111c9426e9cd1943d8a2650c7dc7af",
     "4e58764d264062c5ea8f846943db16f83cd725a5ed313865cc5cb1171da54eec"},
    {PromptCardPage::PromptType4,
     11,
     4,
     11903,
     "Stage1SavePromptFlash",
     "save_confirm_prompt_type4_flash_first_draw",
     202,
     887,
     "07aad70bfd3d46165af05e97298a55e08b106dba12e7b9c937af65079e69090f",
     "be3e65057e5892706ff9b4664f17cb7a9e4f4667015d20b33867ecf474bb3f5f"},
    {PromptCardPage::PromptType4,
     11,
     4,
     11904,
     "Stage1SavePromptFlash",
     "save_confirm_prompt_type4_flash_adjacent_draw",
     202,
     887,
     "3d4318bc321a76704c661e9517c7f1f0ea0854552dee6ccb325819a4f6b67da5",
     "796a16d12bc48cfc1777958eecb4b40f2eee741b9a386f33792814c09e98c155"},
    {PromptCardPage::PromptType4,
     11,
     4,
     11922,
     "Stage1SavePromptFlash",
     "save_confirm_prompt_type4_flash_last_draw",
     202,
     887,
     "8861d98c000d42fa1d599febe161b18b714ee60eb405646f26ccf7fdb562d4b0",
     "76174a48faab7b89159e274a5803e87c7406697557107ff6a4219ddccb8d06a3"},
    {PromptCardPage::CardInfoEv5,
     5,
     0,
     12180,
     "Stage1SavePrompt",
     "card_info_ev5_first_real_draw",
     312,
     1382,
     "7bb16cce522c7bebf656b34497e06dfd45c497aa325a94aada4765f12c6e60fd",
     "4d349719baa202be36c983892420187846871e8565f2360cc670b2ec4a3c12c8"},
    {PromptCardPage::PromptType5,
     15,
     5,
     12525,
     "Stage1SavePrompt",
     "rename_prompt_type5_first_nonempty_draw",
     198,
     869,
     "077ca76c5a316e7f9458f40e3586b71f3c65aedd4cc0f70a70832b78b40f80c0",
     "a5d89c5cd1af7e90790ce6557b125000119492b13cf3e73221e07e32668fe46b"},
    {PromptCardPage::PromptType5,
     15,
     5,
     12651,
     "Stage1SavePromptFlash",
     "rename_prompt_type5_flash_first_draw",
     198,
     869,
     "950019c30c74ae5ee084617e5e22864d051de02f4262cc777c22f712e78f36b7",
     "3559ef4d1ab7324c7618191ae34e63ce03cb90a435deacb456a2a8be99000436"},
    {PromptCardPage::PromptType5,
     15,
     5,
     12652,
     "Stage1SavePromptFlash",
     "rename_prompt_type5_flash_adjacent_draw",
     198,
     869,
     "d1741eed86a44963fc718601bbde6c243058467e07ec3de89eef10f8112f1f29",
     "359fc3fe1be85f33819388012a97b5bd69dc9ccabebd95756aa4e10d443d56ff"},
    {PromptCardPage::PromptType5,
     15,
     5,
     12670,
     "Stage1SavePromptFlash",
     "rename_prompt_type5_flash_last_draw",
     198,
     869,
     "d1741eed86a44963fc718601bbde6c243058467e07ec3de89eef10f8112f1f29",
     "359fc3fe1be85f33819388012a97b5bd69dc9ccabebd95756aa4e10d443d56ff"},
    {PromptCardPage::CardInfoEv5,
     5,
     0,
     12672,
     "Stage1SavePrompt",
     "card_info_ev5_after_rename_return",
     312,
     1382,
     "f6ad89d34da7a91f5258034b5bd1520314f0e021d68554986445c37f95206d78",
     "848d2786903ba390712685fe2d617a7f14eae7ee8acbd6cf87acf9c3893aa2ee"},
    {PromptCardPage::CardInfoEv5,
     5,
     0,
     13370,
     "Stage1SaveCardIoBanner",
     "card_info_ev5_io_banner_first_draw",
     360,
     1592,
     "86a5edff946f5f8cb36cb1be2ec681246efe7fc2a812cc6335dd9ab60e20e9d5",
     "1d7ee7c7c9bd5729ea3c2c1b6cd8067b1df2be4c9f5a959b97528567d460ae24"},
    {PromptCardPage::CardInfoEv5,
     5,
     0,
     13371,
     "Stage1SaveCardIoBanner",
     "card_info_ev5_io_banner_adjacent_draw",
     360,
     1592,
     "a4615bef70ba9afe0078a9487168a4da2158cd594d3d0679547b888f91e1537e",
     "2da8c9677eb5b3bc9931bf8907238efb3ec18b5d8655a0a58d02e3edeaef68da"},
    {PromptCardPage::CardInfoEv5,
     5,
     0,
     13386,
     "Stage1SaveCardIoBanner",
     "card_info_ev5_io_banner_last_pre_success",
     360,
     1592,
     "86a5edff946f5f8cb36cb1be2ec681246efe7fc2a812cc6335dd9ab60e20e9d5",
     "1d7ee7c7c9bd5729ea3c2c1b6cd8067b1df2be4c9f5a959b97528567d460ae24"},
    {PromptCardPage::CardInfoEv5,
     5,
     0,
     13525,
     "Stage1SaveFlash",
     "card_info_ev5_success_flash_first_draw",
     360,
     1592,
     "4b5499836c7b7ddec73f6fb182e01045279747179e458f533ae22ba40fd39a67",
     "6ab0dc39ef491704e29f2085f97c81147a723902f073f591ef27130a00a99a34"},
    {PromptCardPage::CardInfoEv5,
     5,
     0,
     13526,
     "Stage1SaveFlash",
     "card_info_ev5_success_flash_second_draw",
     324,
     1436,
     "4c0d50be432434300d68228c743a33db1f0024cbeebfe89a1d44383de3a32cf8",
     "0022c27e949334c4b8ff0e77e437a589886fd579f70de53e93d141f530963dfa"},
};

static_assert(sizeof(kHiScoreLanguageTitleEntries) /
                  sizeof(kHiScoreLanguageTitleEntries[0]) ==
              static_cast<uint32_t>(kCardIoLanguageCount80020A3C),
              "80021594 language title records must stay PSX-sized");
static_assert(sizeof(kHiScoreExitLabelEntries) /
                  sizeof(kHiScoreExitLabelEntries[0]) ==
              static_cast<uint32_t>(kCardIoLanguageCount80020A3C),
              "80021594 exit label records must stay PSX-sized");
static_assert(sizeof(kHiScoreFixedSprites) / sizeof(kHiScoreFixedSprites[0]) ==
              static_cast<uint32_t>(1 + kHiScoreMaxCols80021594 +
                                    kHiScoreMaxRows80021594),
              "80021594 fixed sprite records must stay PSX-sized");
static_assert(sizeof(kHiScoreGp0ReplayBaselines) /
                  sizeof(kHiScoreGp0ReplayBaselines[0]) ==
              static_cast<uint32_t>(kHiScoreGp0ReplayBaselineCount),
              "80021594 GP0 replay baseline count must stay PSX-sized");
static_assert(sizeof(kPromptCardGp0ReplayBaselines) /
                  sizeof(kPromptCardGp0ReplayBaselines[0]) ==
              static_cast<uint32_t>(kPromptCardGp0ReplayBaselineCount),
              "prompt/card GP0 replay baseline count must stay PSX-sized");

bool Append(PromptCardPlan& plan, const PromptCardAction& action)
{
    if (plan.count >= sizeof(plan.actions) / sizeof(plan.actions[0])) {
        plan.truncated = true;
        return false;
    }
    plan.actions[plan.count++] = action;
    return true;
}

void AppendAction(PromptCardPlan& plan,
                  PromptCardActionKind kind,
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
    PromptCardAction action{};
    action.kind = kind;
    action.page = plan.page;
    action.psxFunction = psxFunction;
    action.tableAddress = tableAddress;
    action.templateAddress = templateAddress;
    action.eventId = plan.eventId;
    action.promptType = plan.promptType;
    action.x = x;
    action.y = y;
    action.args[0] = arg0;
    action.args[1] = arg1;
    action.args[2] = arg2;
    action.args[3] = arg3;
    action.conditional = conditional;
    (void)Append(plan, action);
}

void AppendPlan(PromptCardPlan& dst, const PromptCardPlan& src)
{
    for (uint32_t i = 0; i < src.count; ++i) {
        (void)Append(dst, src.actions[i]);
    }
    dst.truncated = dst.truncated || src.truncated;
    dst.blockedByGap = dst.blockedByGap || src.blockedByGap;
}

PromptCardPlan MakePlan(const char* name,
                        PromptCardPage page,
                        int32_t eventId,
                        int32_t promptType)
{
    PromptCardPlan plan{};
    plan.name = name;
    plan.runtimeCutoverAllowed = RuntimeCutoverAllowed();
    plan.page = page;
    plan.eventId = eventId;
    plan.promptType = promptType;
    return plan;
}

bool PageMatches(PromptCardPage specPage, PromptCardPage page)
{
    return specPage == page;
}

void AppendRoute(PromptCardPlan& plan)
{
    const PromptCardRouteSpec* route =
        plan.promptType > 0 ? FindPromptCardRouteSpecByType(plan.promptType)
                            : FindPromptCardRouteSpecByEvent(plan.eventId);
    if (route == nullptr) {
        AppendAction(plan, PromptCardActionKind::Gap, 0);
        return;
    }
    AppendAction(plan,
                 PromptCardActionKind::DrawRoute,
                 route->drawFunction,
                 0,
                 0,
                 0,
                 0,
                 route->eventId,
                 route->promptType);
}

void AppendAnchors(PromptCardPlan& plan)
{
    for (uint32_t i = 0; i < KnownPromptCardAnchorSpecCount(); ++i) {
        const PromptCardAnchorSpec& spec = KnownPromptCardAnchorSpecAt(i);
        if (!PageMatches(spec.page, plan.page)) {
            continue;
        }
        AppendAction(plan,
                     PromptCardActionKind::FixedSprite,
                     plan.promptType > 0 ? kFn80022CBC : plan.eventId == 4 ? kFn800203D4 : 0,
                     spec.tableAddress,
                     spec.template0,
                     spec.x,
                     spec.y,
                     static_cast<int32_t>(spec.template1));
    }
}

void AppendHeaderTables(PromptCardPlan& plan)
{
    for (uint32_t i = 0; i < KnownPromptHeaderTableSpecCount(); ++i) {
        const PromptHeaderTableSpec& spec = KnownPromptHeaderTableSpecAt(i);
        if (spec.promptType != plan.promptType) {
            continue;
        }
        AppendAction(plan,
                     PromptCardActionKind::LanguageTable,
                     kFn80022CBC,
                     spec.tableAddress,
                     0,
                     0,
                     0,
                     static_cast<int32_t>(spec.strideBytes),
                     static_cast<int32_t>(spec.entryCount));
    }
}

bool CardIoTextEntryMatches(const CardIoTextEntrySpec& spec,
                            int32_t msgType,
                            bool msgKnown,
                            uint8_t language,
                            bool languageKnown)
{
    if (msgKnown && spec.msgType != msgType) {
        return false;
    }
    if (languageKnown && spec.language != language) {
        return false;
    }
    return true;
}

void AppendCardIoBannerActions(PromptCardPlan& plan,
                               int32_t msgType,
                               uint8_t language,
                               bool msgSourceKnown,
                               bool languageKnown)
{
    const bool msgKnown =
        msgSourceKnown && msgType >= 0 && msgType <= 2;
    const bool langKnown =
        languageKnown && language < kCardIoLanguageCount80020A3C;

    AppendAction(plan,
                 PromptCardActionKind::GateCardIoMsgSource80020A3C,
                 kFn80020A3C,
                 msgKnown ? static_cast<uint32_t>(msgType) : 0u,
                 langKnown ? static_cast<uint32_t>(language) : 0u,
                 kCardIoBannerTextX80020A3C,
                 kCardIoBannerTextY80020A3C,
                 msgKnown ? msgType : -1,
                 langKnown ? language : -1,
                 msgKnown ? 1 : 0,
                 langKnown ? 1 : 0,
                 true);

    for (uint32_t i = 0; i < KnownCardIoTextEntrySpecCount(); ++i) {
        const CardIoTextEntrySpec& spec = KnownCardIoTextEntrySpecAt(i);
        if (!CardIoTextEntryMatches(spec,
                                    msgType,
                                    msgKnown,
                                    language,
                                    langKnown)) {
            continue;
        }
        AppendAction(plan,
                     PromptCardActionKind::CardIoBannerText,
                     kFn8001C6E0,
                     spec.tableAddress,
                     spec.textAddress,
                     kCardIoBannerTextX80020A3C,
                     kCardIoBannerTextY80020A3C,
                     spec.msgType,
                     spec.language,
                     kCardIoBannerTextOt80020A3C,
                     msgKnown && langKnown ? 1 : 0,
                     true);
    }

    AppendAction(plan,
                 PromptCardActionKind::CardIoBannerLayout,
                 kFn80020A3C,
                 kFn8001C6E0,
                 0,
                 -1,
                 -1,
                 kCardIoBannerScreenWidth80020A3C,
                 kCardIoBannerPadding80020A3C,
                 kCardIoBannerTextOt80020A3C,
                 1,
                 true);
    AppendAction(plan,
                 PromptCardActionKind::CardIoBannerBoxFill,
                 kFn8001C4EC,
                 0,
                 0,
                 -1,
                 123,
                 0,
                 8,
                 9,
                 kCardIoBannerFillColor80020A3C,
                 true);
    AppendAction(plan,
                 PromptCardActionKind::CardIoBannerBoxFill,
                 kFn8001C4EC,
                 0,
                 0,
                 -1,
                 115,
                 1,
                 -1,
                 25,
                 kCardIoBannerFillColor80020A3C,
                 true);
    AppendAction(plan,
                 PromptCardActionKind::CardIoBannerBoxFill,
                 kFn8001C4EC,
                 0,
                 0,
                 -1,
                 123,
                 2,
                 8,
                 9,
                 kCardIoBannerFillColor80020A3C,
                 true);
    AppendAction(plan,
                 PromptCardActionKind::CardIoBannerCornerSprite,
                 kFn8001C550,
                 0,
                 kSpriteCardIoCornerTopLeft80050900,
                 -1,
                 115,
                 0,
                 0,
                 0,
                 0,
                 true);
    AppendAction(plan,
                 PromptCardActionKind::CardIoBannerCornerSprite,
                 kFn8001C550,
                 0,
                 kSpriteCardIoCornerBottomLeft800508F0,
                 -1,
                 132,
                 1,
                 0,
                 0,
                 0,
                 true);
    AppendAction(plan,
                 PromptCardActionKind::CardIoBannerCornerSprite,
                 kFn8001C550,
                 0,
                 kSpriteCardIoCornerTopRight800508E0,
                 -1,
                 115,
                 2,
                 1,
                 0,
                 0,
                 true);
    AppendAction(plan,
                 PromptCardActionKind::CardIoBannerCornerSprite,
                 kFn8001C550,
                 0,
                 kSpriteCardIoCornerBottomRight800508D0,
                 -1,
                 132,
                 3,
                 1,
                 0,
                 0,
                 true);
}

uint32_t CardInfoLowerTemplateForVariant(const CardInfoLowerTextEntrySpec& spec,
                                         int32_t variant)
{
    if (variant == 1) {
        return spec.selectedTemplate;
    }
    if (variant == 2) {
        return spec.forcedTemplate;
    }
    return spec.defaultTemplate;
}

void AppendCardInfoLowerBranchAction(PromptCardPlan& plan,
                                     int32_t branch,
                                     int32_t topTextVariant,
                                     int32_t bottomTextVariant,
                                     int32_t topGpSlot)
{
    AppendAction(plan,
                 PromptCardActionKind::CardInfoLowerBranch,
                 kFn80020BE4,
                 kTableCardInfoLowerTopText8005349C,
                 kTableCardInfoLowerBottomText800534EC,
                 223,
                 160,
                 branch,
                 topTextVariant,
                 bottomTextVariant,
                 topGpSlot,
                 true);
}

void AppendCardInfoLowerSpriteAction(PromptCardPlan& plan,
                                     int32_t branch,
                                     int32_t role,
                                     uint32_t tableAddress,
                                     uint32_t templateAddress,
                                     int16_t x,
                                     int16_t y)
{
    AppendAction(plan,
                 PromptCardActionKind::CardInfoLowerFixedSprite,
                 kFn8001C550,
                 tableAddress,
                 templateAddress,
                 x,
                 y,
                 branch,
                 role,
                 0,
                 0,
                 true);
}

void AppendCardInfoLowerActions(PromptCardPlan& plan,
                                uint32_t argAddress,
                                bool argSourceKnown,
                                int32_t selectedMarker,
                                int32_t lowerMode,
                                int32_t topFlag,
                                bool topFlagKnown,
                                bool lowerModeKnown)
{
    const uint32_t lowerModeSource =
        argSourceKnown ? argAddress + kCardInfoLowerModeOffset80020BE4 : 0u;
    const uint32_t topFlagSource =
        argSourceKnown ? argAddress + kCardInfoLowerTopFlagOffset80020BE4 : 0u;
    const bool selectedKnown =
        selectedMarker >= 0 && selectedMarker < kCardInfoMarkerCount80020BE4;
    const int32_t knownMask = (selectedKnown ? 1 : 0) |
                              (lowerModeKnown ? 2 : 0) |
                              (topFlagKnown ? 4 : 0);

    AppendAction(plan,
                 PromptCardActionKind::GateCardInfoLowerState80020BE4,
                 kFn80020BE4,
                 lowerModeSource,
                 topFlagSource,
                 0,
                 0,
                 selectedKnown ? selectedMarker : -1,
                 lowerModeKnown ? lowerMode : -1,
                 topFlagKnown ? topFlag : -1,
                 knownMask,
                 true);

    AppendCardInfoLowerBranchAction(
        plan, 0, 0, 0, kCardInfoLowerGpTemplateSlotDefault80020BE4);
    AppendCardInfoLowerBranchAction(
        plan, 1, 2, 1, kCardInfoLowerGpTemplateSlotActive80020BE4);
    AppendCardInfoLowerBranchAction(
        plan, 2, 1, 2, kCardInfoLowerGpTemplateSlotActive80020BE4);
    AppendCardInfoLowerBranchAction(plan, 3, 1, 1, -1);

    for (uint32_t i = 0; i < KnownCardInfoLowerTextEntrySpecCount(); ++i) {
        const CardInfoLowerTextEntrySpec& spec =
            KnownCardInfoLowerTextEntrySpecAt(i);
        for (int32_t variant = 0; variant < 3; ++variant) {
            AppendAction(plan,
                         PromptCardActionKind::CardInfoLowerLanguageText,
                         kFn8001C5A8,
                         spec.recordAddress,
                         CardInfoLowerTemplateForVariant(spec, variant),
                         spec.x,
                         spec.y,
                         spec.group,
                         spec.language,
                         variant,
                         0,
                         true);
        }
    }

    AppendCardInfoLowerSpriteAction(plan,
                                    0,
                                    0,
                                    kCardInfoLowerGpTemplateSlotDefault80020BE4,
                                    0,
                                    223,
                                    160);
    AppendCardInfoLowerSpriteAction(plan,
                                    0,
                                    1,
                                    0,
                                    kSpriteCardInfoLowerMidDefault80051FE0,
                                    232,
                                    169);
    AppendCardInfoLowerSpriteAction(plan,
                                    0,
                                    2,
                                    0,
                                    kSpriteCardInfoLowerFinalDefault80052120,
                                    232,
                                    188);
    AppendCardInfoLowerSpriteAction(plan,
                                    1,
                                    0,
                                    kCardInfoLowerGpTemplateSlotActive80020BE4,
                                    0,
                                    223,
                                    160);
    AppendCardInfoLowerSpriteAction(plan,
                                    1,
                                    1,
                                    0,
                                    kSpriteCardInfoLowerMidMode1_80052000,
                                    232,
                                    169);
    AppendCardInfoLowerSpriteAction(plan,
                                    1,
                                    2,
                                    0,
                                    kSpriteCardInfoLowerFinalMode1_80052130,
                                    232,
                                    188);
    AppendCardInfoLowerSpriteAction(plan,
                                    2,
                                    0,
                                    kCardInfoLowerGpTemplateSlotActive80020BE4,
                                    0,
                                    223,
                                    160);
    AppendCardInfoLowerSpriteAction(plan,
                                    2,
                                    1,
                                    0,
                                    kSpriteCardInfoLowerMidSelected80051FF0,
                                    232,
                                    169);
    AppendCardInfoLowerSpriteAction(plan,
                                    2,
                                    2,
                                    0,
                                    kSpriteCardInfoLowerFinalMode2_80052140,
                                    232,
                                    188);
    AppendCardInfoLowerSpriteAction(plan,
                                    3,
                                    0,
                                    kCardInfoLowerGpTemplateSlotDefault80020BE4,
                                    kCardInfoLowerGpTemplateSlotActive80020BE4,
                                    223,
                                    160);
    AppendCardInfoLowerSpriteAction(plan,
                                    3,
                                    1,
                                    0,
                                    kSpriteCardInfoLowerMidSelected80051FF0,
                                    232,
                                    169);
    AppendCardInfoLowerSpriteAction(plan,
                                    3,
                                    2,
                                    0,
                                    kSpriteCardInfoLowerFinalMode1_80052130,
                                    232,
                                    188);
}

void AppendHiScoreActions(PromptCardPlan& plan,
                          uint32_t argAddress,
                          bool argSourceKnown,
                          int32_t rows,
                          int32_t cols,
                          int32_t exitIconState,
                          int32_t exitLabelState,
                          bool exitIconStateKnown,
                          bool exitLabelStateKnown)
{
    const uint32_t rowsSource =
        argSourceKnown ? argAddress + kHiScoreRowsOffset80021594 : 0u;
    const uint32_t colsSource =
        argSourceKnown ? argAddress + kHiScoreColsOffset80021594 : 0u;
    const uint32_t exitIconSource =
        argSourceKnown ? argAddress + kHiScoreExitIconStateOffset80021594 : 0u;
    const uint32_t exitLabelSource =
        argSourceKnown ? argAddress + kHiScoreExitLabelStateOffset80021594 : 0u;
    const bool rowsKnown = rows >= 0 && rows <= kHiScoreMaxRows80021594;
    const bool colsKnown = cols >= 0 && cols <= kHiScoreMaxCols80021594;
    const int32_t knownMask = (rowsKnown ? 1 : 0) |
                              (colsKnown ? 2 : 0) |
                              (exitIconStateKnown ? 4 : 0) |
                              (exitLabelStateKnown ? 8 : 0);

    AppendAction(plan,
                 PromptCardActionKind::GateHiScoreArgSource80021594,
                 kFn80021594,
                 rowsSource,
                 colsSource,
                 0,
                 0,
                 rowsKnown ? rows : -1,
                 colsKnown ? cols : -1,
                 static_cast<int32_t>(argAddress),
                 knownMask,
                 true);
    AppendAction(plan,
                 PromptCardActionKind::HiScoreCellGrid,
                 kFn80021594,
                 argSourceKnown ? argAddress + kHiScoreCellBaseOffset80021594
                                : 0u,
                 kTableHiScoreCellStatusSprites8005353C,
                 kHiScoreCellIconX80021594,
                 kHiScoreCellIconY80021594,
                 kHiScoreMaxRows80021594,
                 kHiScoreMaxCols80021594,
                 kHiScoreCellStride80021594,
                 kHiScoreCellColStep80021594,
                 true);

    for (int32_t row = 0; row < kHiScoreMaxRows80021594; ++row) {
        for (int32_t col = 0; col < kHiScoreMaxCols80021594; ++col) {
            const int32_t index = row * kHiScoreMaxCols80021594 + col;
            const int32_t cellOffset =
                kHiScoreCellBaseOffset80021594 +
                index * kHiScoreCellStride80021594;
            const uint32_t cellSource =
                argSourceKnown ? argAddress + static_cast<uint32_t>(cellOffset)
                               : 0u;
            AppendAction(plan,
                         PromptCardActionKind::HiScoreCellTextEntry,
                         kFn8001B730,
                         cellSource,
                         kFn8001C668,
                         static_cast<int16_t>(kHiScoreCellTextX80021594 +
                                              col * kHiScoreCellColStep80021594),
                         static_cast<int16_t>(kHiScoreCellTextY80021594 +
                                              row * kHiScoreCellRowStep80021594),
                         row,
                         col,
                         index,
                         1,
                         true);
            AppendAction(plan,
                         PromptCardActionKind::HiScoreCellEntry,
                         kFn80021594,
                         cellSource,
                         kSpriteHiScoreCellNonEmpty80052EE0,
                         static_cast<int16_t>(kHiScoreCellIconX80021594 +
                                              col * kHiScoreCellColStep80021594),
                         static_cast<int16_t>(kHiScoreCellIconY80021594 +
                                              row * kHiScoreCellRowStep80021594),
                         row,
                         col,
                         index,
                         static_cast<int32_t>(kSpriteHiScoreCellEmpty80052F00),
                         true);
        }
    }

    for (uint32_t i = 0; i < KnownHiScoreLanguageTitleEntrySpecCount(); ++i) {
        const HiScoreLanguageTitleEntrySpec& spec =
            KnownHiScoreLanguageTitleEntrySpecAt(i);
        AppendAction(plan,
                     PromptCardActionKind::HiScoreLanguageTitle,
                     kFn8001C5A8,
                     kTableHiScoreTitlePos80053F88 +
                         static_cast<uint32_t>(spec.language) * 8u,
                     spec.templateAddress,
                     spec.x,
                     spec.y,
                     spec.language,
                     0,
                     0,
                     0,
                     true);
    }

    for (uint32_t i = 0; i < KnownHiScoreFixedSpriteSpecCount(); ++i) {
        const HiScoreFixedSpriteSpec& spec = KnownHiScoreFixedSpriteSpecAt(i);
        AppendAction(plan,
                     PromptCardActionKind::HiScoreFixedSprite,
                     kFn8001C550,
                     0,
                     spec.templateAddress,
                     spec.x,
                     spec.y,
                     spec.group,
                     spec.index,
                     0,
                     0,
                     true);
    }

    AppendAction(plan,
                 PromptCardActionKind::GateHiScoreExitState80021594,
                 kFn80021594,
                 exitIconSource,
                 exitLabelSource,
                 231,
                 179,
                 exitIconStateKnown ? exitIconState : -1,
                 exitLabelStateKnown ? exitLabelState : -1,
                 knownMask,
                 0,
                 true);
    AppendAction(plan,
                 PromptCardActionKind::HiScoreFixedSprite,
                 kFn8001C550,
                 kGpSharedExitIconSlotOn8006EB10,
                 kSpriteSharedExitIconOn80050AD0,
                 231,
                 179,
                 kHiScoreExitIconGpSlotOn80021594,
                 1,
                 static_cast<int32_t>(kGpBase8006EA40),
                 0,
                 true);
    AppendAction(plan,
                 PromptCardActionKind::HiScoreFixedSprite,
                 kFn8001C550,
                 kGpSharedExitIconSlotOff8006EB14,
                 kSpriteSharedExitIconOff80050AC0,
                 231,
                 179,
                 kHiScoreExitIconGpSlotOff80021594,
                 0,
                 static_cast<int32_t>(kGpBase8006EA40),
                 0,
                 true);

    for (uint32_t i = 0; i < KnownHiScoreExitLabelEntrySpecCount(); ++i) {
        const HiScoreExitLabelEntrySpec& spec =
            KnownHiScoreExitLabelEntrySpecAt(i);
        AppendAction(plan,
                     PromptCardActionKind::HiScoreExitLabel,
                     kFn8001C5A8,
                     kTableSharedExitLabelPos8005300C +
                         static_cast<uint32_t>(spec.language) * 16u,
                     spec.offTemplate,
                     spec.x,
                     spec.y,
                     spec.language,
                     static_cast<int32_t>(spec.onTemplate),
                     0,
                     0,
                     true);
    }
    AppendAction(plan,
                 PromptCardActionKind::HiScoreExitBar,
                 kFn8001C5A8,
                 kTableSharedExitBarPos80052FFC,
                 kSpriteHiScoreExitBarOff800509B0,
                 238,
                 188,
                 0,
                 kHiScoreExitLabelStateOffset80021594,
                 0,
                 0,
                 true);
    AppendAction(plan,
                 PromptCardActionKind::HiScoreExitBar,
                 kFn8001C5A8,
                 kTableSharedExitBarPos80052FFC,
                 kSpriteHiScoreExitBarOn800509C0,
                 238,
                 188,
                 1,
                 kHiScoreExitLabelStateOffset80021594,
                 0,
                 0,
                 true);

    for (uint32_t i = 0; i < KnownHiScoreGp0ReplayBaselineSpecCount(); ++i) {
        const HiScoreGp0ReplayBaselineSpec& spec =
            KnownHiScoreGp0ReplayBaselineSpecAt(i);
        AppendAction(plan,
                     PromptCardActionKind::HiScoreGp0ReplayBaseline,
                     kFn80021594,
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

void AppendPromptCardGp0ReplayBaselines(PromptCardPlan& plan)
{
    for (uint32_t i = 0; i < KnownPromptCardGp0ReplayBaselineSpecCount(); ++i) {
        const PromptCardGp0ReplayBaselineSpec& spec =
            KnownPromptCardGp0ReplayBaselineSpecAt(i);
        if (!PageMatches(spec.page, plan.page) ||
            spec.eventId != plan.eventId ||
            spec.promptType != plan.promptType) {
            continue;
        }
        const uint32_t drawFunction =
            spec.promptType > 0 ? kFn80022CBC :
            spec.eventId == 5 ? kFn80020BE4 :
            spec.eventId == 4 ? kFn800203D4 : 0u;
        AppendAction(plan,
                     PromptCardActionKind::PromptCardGp0ReplayBaseline,
                     drawFunction,
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

PromptCardPlan BuildUnknownPlan(int32_t eventId)
{
    PromptCardPlan plan =
        MakePlan("PromptCardRender_UnknownEvent",
                 PromptCardPage::Unknown,
                 eventId,
                 0);
    AppendAction(plan, PromptCardActionKind::Gap, 0);
    return plan;
}

} // namespace

bool RuntimeCutoverAllowed()
{
    return false;
}

uint32_t KnownPromptCardRouteSpecCount()
{
    return sizeof(kRoutes) / sizeof(kRoutes[0]);
}

const PromptCardRouteSpec& KnownPromptCardRouteSpecAt(uint32_t index)
{
    if (index >= KnownPromptCardRouteSpecCount()) {
        index = KnownPromptCardRouteSpecCount() - 1u;
    }
    return kRoutes[index];
}

const PromptCardRouteSpec* FindPromptCardRouteSpecByEvent(int32_t eventId)
{
    for (uint32_t i = 0; i < KnownPromptCardRouteSpecCount(); ++i) {
        if (kRoutes[i].eventId == eventId) {
            return &kRoutes[i];
        }
    }
    return nullptr;
}

const PromptCardRouteSpec* FindPromptCardRouteSpecByType(int32_t type)
{
    for (uint32_t i = 0; i < KnownPromptCardRouteSpecCount(); ++i) {
        if (kRoutes[i].promptType == type) {
            return &kRoutes[i];
        }
    }
    return nullptr;
}

uint32_t KnownPromptCardAnchorSpecCount()
{
    return sizeof(kAnchors) / sizeof(kAnchors[0]);
}

const PromptCardAnchorSpec& KnownPromptCardAnchorSpecAt(uint32_t index)
{
    if (index >= KnownPromptCardAnchorSpecCount()) {
        index = KnownPromptCardAnchorSpecCount() - 1u;
    }
    return kAnchors[index];
}

uint32_t KnownPromptHeaderTableSpecCount()
{
    return sizeof(kHeaderTables) / sizeof(kHeaderTables[0]);
}

const PromptHeaderTableSpec& KnownPromptHeaderTableSpecAt(uint32_t index)
{
    if (index >= KnownPromptHeaderTableSpecCount()) {
        index = KnownPromptHeaderTableSpecCount() - 1u;
    }
    return kHeaderTables[index];
}

uint32_t KnownPromptHeaderEntrySpecCount()
{
    return sizeof(kHeaderEntries) / sizeof(kHeaderEntries[0]);
}

const PromptHeaderEntrySpec& KnownPromptHeaderEntrySpecAt(uint32_t index)
{
    if (index >= KnownPromptHeaderEntrySpecCount()) {
        index = KnownPromptHeaderEntrySpecCount() - 1u;
    }
    return kHeaderEntries[index];
}

uint32_t KnownPromptChoiceEntrySpecCount()
{
    return sizeof(kChoiceEntries) / sizeof(kChoiceEntries[0]);
}

const PromptChoiceEntrySpec& KnownPromptChoiceEntrySpecAt(uint32_t index)
{
    if (index >= KnownPromptChoiceEntrySpecCount()) {
        index = KnownPromptChoiceEntrySpecCount() - 1u;
    }
    return kChoiceEntries[index];
}

uint32_t KnownCardInfoMarkerEntrySpecCount()
{
    return sizeof(kCardInfoMarkerEntries) / sizeof(kCardInfoMarkerEntries[0]);
}

const CardInfoMarkerEntrySpec& KnownCardInfoMarkerEntrySpecAt(uint32_t index)
{
    if (index >= KnownCardInfoMarkerEntrySpecCount()) {
        index = KnownCardInfoMarkerEntrySpecCount() - 1u;
    }
    return kCardInfoMarkerEntries[index];
}

uint32_t KnownCardIoTextEntrySpecCount()
{
    return sizeof(kCardIoTextEntries) / sizeof(kCardIoTextEntries[0]);
}

const CardIoTextEntrySpec& KnownCardIoTextEntrySpecAt(uint32_t index)
{
    if (index >= KnownCardIoTextEntrySpecCount()) {
        index = KnownCardIoTextEntrySpecCount() - 1u;
    }
    return kCardIoTextEntries[index];
}

uint32_t KnownCardInfoLowerTextEntrySpecCount()
{
    return sizeof(kCardInfoLowerTextEntries) /
           sizeof(kCardInfoLowerTextEntries[0]);
}

const CardInfoLowerTextEntrySpec& KnownCardInfoLowerTextEntrySpecAt(uint32_t index)
{
    if (index >= KnownCardInfoLowerTextEntrySpecCount()) {
        index = KnownCardInfoLowerTextEntrySpecCount() - 1u;
    }
    return kCardInfoLowerTextEntries[index];
}

uint32_t KnownHiScoreLanguageTitleEntrySpecCount()
{
    return sizeof(kHiScoreLanguageTitleEntries) /
           sizeof(kHiScoreLanguageTitleEntries[0]);
}

const HiScoreLanguageTitleEntrySpec& KnownHiScoreLanguageTitleEntrySpecAt(uint32_t index)
{
    if (index >= KnownHiScoreLanguageTitleEntrySpecCount()) {
        index = KnownHiScoreLanguageTitleEntrySpecCount() - 1u;
    }
    return kHiScoreLanguageTitleEntries[index];
}

uint32_t KnownHiScoreExitLabelEntrySpecCount()
{
    return sizeof(kHiScoreExitLabelEntries) /
           sizeof(kHiScoreExitLabelEntries[0]);
}

const HiScoreExitLabelEntrySpec& KnownHiScoreExitLabelEntrySpecAt(uint32_t index)
{
    if (index >= KnownHiScoreExitLabelEntrySpecCount()) {
        index = KnownHiScoreExitLabelEntrySpecCount() - 1u;
    }
    return kHiScoreExitLabelEntries[index];
}

uint32_t KnownHiScoreFixedSpriteSpecCount()
{
    return sizeof(kHiScoreFixedSprites) / sizeof(kHiScoreFixedSprites[0]);
}

const HiScoreFixedSpriteSpec& KnownHiScoreFixedSpriteSpecAt(uint32_t index)
{
    if (index >= KnownHiScoreFixedSpriteSpecCount()) {
        index = KnownHiScoreFixedSpriteSpecCount() - 1u;
    }
    return kHiScoreFixedSprites[index];
}

uint32_t KnownHiScoreGp0ReplayBaselineSpecCount()
{
    return sizeof(kHiScoreGp0ReplayBaselines) /
           sizeof(kHiScoreGp0ReplayBaselines[0]);
}

const HiScoreGp0ReplayBaselineSpec& KnownHiScoreGp0ReplayBaselineSpecAt(uint32_t index)
{
    if (index >= KnownHiScoreGp0ReplayBaselineSpecCount()) {
        index = KnownHiScoreGp0ReplayBaselineSpecCount() - 1u;
    }
    return kHiScoreGp0ReplayBaselines[index];
}

uint32_t KnownPromptCardGp0ReplayBaselineSpecCount()
{
    return sizeof(kPromptCardGp0ReplayBaselines) /
           sizeof(kPromptCardGp0ReplayBaselines[0]);
}

const PromptCardGp0ReplayBaselineSpec& KnownPromptCardGp0ReplayBaselineSpecAt(
    uint32_t index)
{
    if (index >= KnownPromptCardGp0ReplayBaselineSpecCount()) {
        index = KnownPromptCardGp0ReplayBaselineSpecCount() - 1u;
    }
    return kPromptCardGp0ReplayBaselines[index];
}

PromptCardPlan BuildEvent4Prompt800203D4Plan(int32_t ctx0,
                                             bool choiceSourceKnown)
{
    PromptCardPlan plan =
        MakePlan("Event4Prompt800203D4",
                 PromptCardPage::Event4Prompt,
                 4,
                 0);
    plan.blockedByGap = true;
    const bool validChoice = ctx0 >= -1 && ctx0 <= 1;
    const bool choiceKnown = choiceSourceKnown && validChoice;

    AppendRoute(plan);
    AppendAnchors(plan);
    AppendAction(plan,
                 PromptCardActionKind::GateEvent4ChoiceSource800203D4,
                 kFn800203D4,
                 0,
                 0,
                 0,
                 0,
                 choiceKnown ? ctx0 : -1,
                 choiceKnown ? 1 : 0,
                 0,
                 1,
                 true);
    AppendAction(plan,
                 PromptCardActionKind::Event4ChoiceState,
                 kFn800203D4,
                 0,
                 0,
                 0,
                 0,
                 choiceKnown ? ctx0 : -1,
                 0,
                 1);
    AppendAction(plan,
                 PromptCardActionKind::Event4PromptFastSpriteChain8001C550,
                 kFn8001C550,
                 kWorkListBase80087288,
                 kFn8001B590,
                 0,
                 0,
                 kEvent4PromptSpriteCount800203D4,
                 static_cast<int32_t>(kWorkListStride80087288),
                 0,
                 0,
                 true);
    AppendAction(plan,
                 PromptCardActionKind::Event4PromptLocalSprite8001B590,
                 kFn8001B590,
                 kWorkListBase80087288,
                 kFn8001B25C,
                 0,
                 0,
                 -160,
                 -120,
                 0,
                 0,
                 true);
    AppendAction(plan,
                 PromptCardActionKind::Event4PromptTemplateCopy8001B25C,
                 kFn8001B25C,
                 0,
                 kFn8003FA20,
                 0,
                 0,
                 kFastSpriteTemplateLastWrittenOffset8001B25C,
                 kFastSpriteLocalRgbR8003FA20,
                 kFastSpriteLocalRgbG8003FA20,
                 kFastSpriteLocalRgbB8003FA20,
                 true);
    AppendAction(plan,
                 PromptCardActionKind::GateEvent4PromptRgbTail8003FA20,
                 kFn8003FA20,
                 0,
                 0,
                 0,
                 0,
                 kFastSpriteLocalRgbR8003FA20,
                 kFastSpriteLocalRgbG8003FA20,
                 kFastSpriteLocalRgbB8003FA20,
                 0,
                 true);
    AppendAction(plan,
                 PromptCardActionKind::Event4PromptPacketIntent8003FA20,
                 kFn8003FA20,
                 kWorkListBase80087288,
                 0,
                 0,
                 0,
                 kFastSpritePacketWords8003FA20,
                 kFastSpritePacketAdvance8003FA20,
                 0,
                 0,
                 true);
    AppendAction(plan,
                 PromptCardActionKind::Gap,
                 kFn8003FA20,
                 0,
                 0,
                 0,
                 0,
                 kFastSpriteLocalRgbR8003FA20,
                 kFastSpriteLocalRgbG8003FA20,
                 kFastSpriteLocalRgbB8003FA20);
    return plan;
}

PromptCardPlan BuildCardInfo80020BE4Plan(uint32_t argAddress,
                                         bool argKnown,
                                         int32_t selectedMarker,
                                         int32_t lowerMode,
                                         int32_t topFlag,
                                         bool topFlagKnown,
                                         bool lowerModeKnown)
{
    PromptCardPlan plan =
        MakePlan("CardInfo80020BE4",
                 PromptCardPage::CardInfoEv5,
                 5,
                 0);
    plan.blockedByGap = true;
    const bool argSourceKnown = argKnown && argAddress != 0u;
    const uint32_t markerSource =
        argSourceKnown ? argAddress + kCardInfoMarkerSelectedOffset80020BE4 : 0u;
    const uint32_t stringSource =
        argSourceKnown ? argAddress + kCardInfoStringOffset80020BE4 : 0u;

    AppendRoute(plan);
    AppendAnchors(plan);
    AppendAction(plan,
                 PromptCardActionKind::GateCardInfoArgSource80020BE4,
                 kFn80020BE4,
                 markerSource,
                 stringSource,
                 0,
                 0,
                 static_cast<int32_t>(argAddress),
                 kCardInfoMarkerSelectedOffset80020BE4,
                 kCardInfoStringOffset80020BE4,
                 argSourceKnown ? 1 : 0,
                 true);
    AppendAction(plan,
                 PromptCardActionKind::CardInfoMarkerStrip,
                 kFn8001C7A8,
                 kTableCardInfoMarkerPositions800532B0,
                 kTableCardInfoMarkerSprites800532AC,
                 0,
                 0,
                 kCardInfoMarkerCount80020BE4,
                 kCardInfoMarkerSelectedOffset80020BE4,
                 argSourceKnown ? 1 : 0);
    for (uint32_t i = 0; i < KnownCardInfoMarkerEntrySpecCount(); ++i) {
        const CardInfoMarkerEntrySpec& spec =
            KnownCardInfoMarkerEntrySpecAt(i);
        const uint32_t positionAddress =
            kTableCardInfoMarkerPositions800532B0 +
            static_cast<uint32_t>(spec.index) *
                kCardInfoMarkerStride80020BE4;
        AppendAction(plan,
                     PromptCardActionKind::CardInfoMarkerEntry,
                     kFn8001C7A8,
                     positionAddress,
                     spec.spriteTemplate,
                     spec.x,
                     spec.y,
                     spec.index,
                     kCardInfoMarkerSelectedOffset80020BE4,
                     argSourceKnown ? 1 : 0,
                     0,
                     true);
    }
    AppendAction(plan,
                 PromptCardActionKind::CardInfoStringBuffer,
                 kFn8001C668,
                 stringSource,
                 0,
                 117,
                 64,
                 kCardInfoStringOffset80020BE4,
                 argSourceKnown ? 1 : 0);
    AppendCardInfoLowerActions(plan,
                               argAddress,
                               argSourceKnown,
                               selectedMarker,
                               lowerMode,
                               topFlag,
                               topFlagKnown,
                               lowerModeKnown);
    AppendPromptCardGp0ReplayBaselines(plan);
    AppendAction(plan, PromptCardActionKind::Gap, kFn80020BE4);
    return plan;
}

PromptCardPlan BuildHiScore80021594Plan(uint32_t argAddress,
                                        bool argKnown,
                                        int32_t rows,
                                        int32_t cols,
                                        int32_t exitIconState,
                                        int32_t exitLabelState,
                                        bool exitIconStateKnown,
                                        bool exitLabelStateKnown)
{
    PromptCardPlan plan =
        MakePlan("HiScore80021594",
                 PromptCardPage::HiScoreEv6,
                 6,
                 0);
    plan.blockedByGap = true;

    AppendRoute(plan);
    AppendAnchors(plan);
    AppendHiScoreActions(plan,
                         argAddress,
                         argKnown && argAddress != 0u,
                         rows,
                         cols,
                         exitIconState,
                         exitLabelState,
                         exitIconStateKnown,
                         exitLabelStateKnown);
    AppendAction(plan,
                 PromptCardActionKind::HiScoreRows,
                 kFn80021594,
                 0,
                 0,
                 37,
                 69,
                 6,
                 18,
                 3);
    AppendAction(plan,
                 PromptCardActionKind::SharedExit,
                 kFn80021594,
                 kTableSharedExitText80053000,
                 0,
                 231,
                 179);
    AppendAction(plan, PromptCardActionKind::Gap, kFn80021594);
    return plan;
}

PromptCardPlan BuildCardIoBanner80020A3CPlan(int32_t msgType,
                                             uint8_t language,
                                             bool msgSourceKnown,
                                             bool languageKnown)
{
    PromptCardPlan plan =
        MakePlan("CardIoBanner80020A3C",
                 PromptCardPage::PromptType7,
                 19,
                 7);
    plan.blockedByGap = true;

    AppendCardIoBannerActions(plan,
                              msgType,
                              language,
                              msgSourceKnown,
                              languageKnown);
    AppendAction(plan, PromptCardActionKind::Gap, kFn80020A3C);
    return plan;
}

PromptCardPlan BuildPromptFamily80022CBCPlan(int32_t promptType)
{
    const PromptCardRouteSpec* route = FindPromptCardRouteSpecByType(promptType);
    if (route == nullptr) {
        PromptCardPlan plan =
            MakePlan("PromptFamily80022CBC_UnknownType",
                     PromptCardPage::Unknown,
                     0,
                     promptType);
        AppendAction(plan, PromptCardActionKind::Gap, kFn80022CBC);
        return plan;
    }

    PromptCardPlan plan =
        MakePlan("PromptFamily80022CBC",
                 route->page,
                 route->eventId,
                 route->promptType);
    plan.blockedByGap = true;

    AppendRoute(plan);
    AppendAction(plan,
                 PromptCardActionKind::GatePromptTypeRoute80022CBC,
                 kFn80022CBC,
                 0,
                 0,
                 0,
                 0,
                 route->eventId,
                 route->promptType,
                 1,
                 1,
                 true);
    AppendAnchors(plan);
    AppendHeaderTables(plan);

    if (promptType == 2 || promptType == 4 || promptType == 7) {
        AppendAction(plan,
                     PromptCardActionKind::PromptDualChoice,
                     kFn80022CBC,
                     kTablePromptChoice0_80053AAC,
                     0,
                     224,
                     149,
                     0,
                     1,
                     2);
        for (uint32_t i = 0; i < KnownPromptChoiceEntrySpecCount(); ++i) {
            const PromptChoiceEntrySpec& spec =
                KnownPromptChoiceEntrySpecAt(i);
            AppendAction(plan,
                         PromptCardActionKind::PromptChoiceState,
                         kFn80022CBC,
                         spec.group == 0 ? kTablePromptChoice0_80053AAC
                                         : kTablePromptChoice1_80053AFC,
                         spec.templateA,
                         spec.x,
                         spec.y,
                         spec.group,
                         spec.language,
                         static_cast<int32_t>(spec.templateB));
        }
        if (promptType == 7) {
            AppendAction(plan,
                         PromptCardActionKind::CardIoPrompt,
                         kFn80020A3C,
                         kTableCardIoRemoveText8005326C,
                         0,
                         0,
                         0,
                         2,
                         0,
                         0,
                         0,
                         true);
            AppendCardIoBannerActions(plan, 2, 0, true, false);
        }
    } else {
        AppendAction(plan,
                     PromptCardActionKind::PromptSingleButton,
                     kFn80022CBC,
                     kTableSharedExitText80053000,
                     0,
                     231,
                     179,
                     0,
                     1);
    }

    AppendPromptCardGp0ReplayBaselines(plan);
    AppendAction(plan, PromptCardActionKind::Gap, kFn80022CBC);
    return plan;
}

PromptCardPlan BuildPromptFlash80017E6CPlan(int32_t eventId,
                                            int32_t selected,
                                            int32_t flag)
{
    const PromptCardRouteSpec* route = FindPromptCardRouteSpecByEvent(eventId);
    if (route == nullptr || route->promptType <= 0) {
        return BuildUnknownPlan(eventId);
    }

    PromptCardPlan plan =
        MakePlan("PromptFlash80017E6C",
                 route->page,
                 eventId,
                 route->promptType);
    plan.blockedByGap = true;

    // 80017E6C writes ctx+4 only for nonnegative selected values. It always
    // writes the full flag dword to ctx+8; downstream renderers own the value
    // predicates rather than this call boundary.
    const bool selectedWritesCtx4 = selected >= 0;
    constexpr bool flagWritesCtx8 = true;
    AppendAction(plan,
                 PromptCardActionKind::GatePromptFlashSource80017E6C,
                 kFn80017E6C,
                 0,
                 0,
                 0,
                 0,
                 selected,
                 flag,
                 selectedWritesCtx4 ? 1 : 0,
                 flagWritesCtx8 ? 1 : 0,
                 true);
    AppendAction(plan,
                 PromptCardActionKind::PromptFlash20Frames,
                 kFn80017E6C,
                 0,
                 0,
                 0,
                 0,
                 selected,
                 flag,
                 20,
                 eventId);
    AppendPlan(plan, BuildPromptFamily80022CBCPlan(route->promptType));
    return plan;
}

PromptCardPlan BuildPromptCardRenderPlanForEvent(int32_t eventId)
{
    switch (eventId) {
    case 4:
        return BuildEvent4Prompt800203D4Plan();
    case 5:
        return BuildCardInfo80020BE4Plan();
    case 6:
        return BuildHiScore80021594Plan();
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
    case 18:
    case 19: {
        const PromptCardRouteSpec* route =
            FindPromptCardRouteSpecByEvent(eventId);
        if (route != nullptr) {
            return BuildPromptFamily80022CBCPlan(route->promptType);
        }
        break;
    }
    }
    return BuildUnknownPlan(eventId);
}

PromptCardPage PromptCardPageFromEventId(int32_t eventId)
{
    const PromptCardRouteSpec* route = FindPromptCardRouteSpecByEvent(eventId);
    return route != nullptr ? route->page : PromptCardPage::Unknown;
}

PromptCardPage PromptCardPageFromType(int32_t promptType)
{
    const PromptCardRouteSpec* route =
        FindPromptCardRouteSpecByType(promptType);
    return route != nullptr ? route->page : PromptCardPage::Unknown;
}

const char* PromptCardPageName(PromptCardPage page)
{
    switch (page) {
    case PromptCardPage::Unknown:
        return "Unknown";
    case PromptCardPage::Event4Prompt:
        return "Event4Prompt";
    case PromptCardPage::CardInfoEv5:
        return "CardInfoEv5";
    case PromptCardPage::HiScoreEv6:
        return "HiScoreEv6";
    case PromptCardPage::PromptType1:
        return "PromptType1";
    case PromptCardPage::PromptType2:
        return "PromptType2";
    case PromptCardPage::PromptType3:
        return "PromptType3";
    case PromptCardPage::PromptType4:
        return "PromptType4";
    case PromptCardPage::PromptType5:
        return "PromptType5";
    case PromptCardPage::PromptType6:
        return "PromptType6";
    case PromptCardPage::PromptType7:
        return "PromptType7";
    }
    return "Unknown";
}

const char* PromptCardTableKindName(PromptCardTableKind kind)
{
    switch (kind) {
    case PromptCardTableKind::Unknown:
        return "Unknown";
    case PromptCardTableKind::Event4PromptTemplates:
        return "Event4PromptTemplates";
    case PromptCardTableKind::CardInfoMarkerSprites:
        return "CardInfoMarkerSprites";
    case PromptCardTableKind::CardInfoMarkerPositions:
        return "CardInfoMarkerPositions";
    case PromptCardTableKind::HiScoreLayout:
        return "HiScoreLayout";
    case PromptCardTableKind::SharedExitText:
        return "SharedExitText";
    case PromptCardTableKind::PromptHeaderType1:
        return "PromptHeaderType1";
    case PromptCardTableKind::PromptHeaderType2:
        return "PromptHeaderType2";
    case PromptCardTableKind::PromptHeaderType3:
        return "PromptHeaderType3";
    case PromptCardTableKind::PromptHeaderType4:
        return "PromptHeaderType4";
    case PromptCardTableKind::PromptHeaderType5:
        return "PromptHeaderType5";
    case PromptCardTableKind::PromptHeaderType6:
        return "PromptHeaderType6";
    case PromptCardTableKind::PromptHeaderType7:
        return "PromptHeaderType7";
    case PromptCardTableKind::PromptChoice0:
        return "PromptChoice0";
    case PromptCardTableKind::PromptChoice1:
        return "PromptChoice1";
    case PromptCardTableKind::CardIoText:
        return "CardIoText";
    }
    return "Unknown";
}

const char* PromptCardActionKindName(PromptCardActionKind kind)
{
    switch (kind) {
    case PromptCardActionKind::None:
        return "None";
    case PromptCardActionKind::DrawRoute:
        return "DrawRoute";
    case PromptCardActionKind::FixedSprite:
        return "FixedSprite";
    case PromptCardActionKind::LanguageTable:
        return "LanguageTable";
    case PromptCardActionKind::SharedExit:
        return "SharedExit";
    case PromptCardActionKind::GateEvent4ChoiceSource800203D4:
        return "GateEvent4ChoiceSource800203D4";
    case PromptCardActionKind::Event4ChoiceState:
        return "Event4ChoiceState";
    case PromptCardActionKind::Event4PromptFastSpriteChain8001C550:
        return "Event4PromptFastSpriteChain8001C550";
    case PromptCardActionKind::Event4PromptLocalSprite8001B590:
        return "Event4PromptLocalSprite8001B590";
    case PromptCardActionKind::Event4PromptTemplateCopy8001B25C:
        return "Event4PromptTemplateCopy8001B25C";
    case PromptCardActionKind::GateEvent4PromptRgbTail8003FA20:
        return "GateEvent4PromptRgbTail8003FA20";
    case PromptCardActionKind::Event4PromptPacketIntent8003FA20:
        return "Event4PromptPacketIntent8003FA20";
    case PromptCardActionKind::GateCardInfoArgSource80020BE4:
        return "GateCardInfoArgSource80020BE4";
    case PromptCardActionKind::CardInfoMarkerStrip:
        return "CardInfoMarkerStrip";
    case PromptCardActionKind::CardInfoMarkerEntry:
        return "CardInfoMarkerEntry";
    case PromptCardActionKind::CardInfoStringBuffer:
        return "CardInfoStringBuffer";
    case PromptCardActionKind::GateCardInfoLowerState80020BE4:
        return "GateCardInfoLowerState80020BE4";
    case PromptCardActionKind::CardInfoLowerBranch:
        return "CardInfoLowerBranch";
    case PromptCardActionKind::CardInfoLowerLanguageText:
        return "CardInfoLowerLanguageText";
    case PromptCardActionKind::CardInfoLowerFixedSprite:
        return "CardInfoLowerFixedSprite";
    case PromptCardActionKind::GateHiScoreArgSource80021594:
        return "GateHiScoreArgSource80021594";
    case PromptCardActionKind::HiScoreRows:
        return "HiScoreRows";
    case PromptCardActionKind::HiScoreCellGrid:
        return "HiScoreCellGrid";
    case PromptCardActionKind::HiScoreCellTextEntry:
        return "HiScoreCellTextEntry";
    case PromptCardActionKind::HiScoreCellEntry:
        return "HiScoreCellEntry";
    case PromptCardActionKind::HiScoreLanguageTitle:
        return "HiScoreLanguageTitle";
    case PromptCardActionKind::HiScoreFixedSprite:
        return "HiScoreFixedSprite";
    case PromptCardActionKind::GateHiScoreExitState80021594:
        return "GateHiScoreExitState80021594";
    case PromptCardActionKind::HiScoreExitLabel:
        return "HiScoreExitLabel";
    case PromptCardActionKind::HiScoreExitBar:
        return "HiScoreExitBar";
    case PromptCardActionKind::HiScoreGp0ReplayBaseline:
        return "HiScoreGp0ReplayBaseline";
    case PromptCardActionKind::PromptCardGp0ReplayBaseline:
        return "PromptCardGp0ReplayBaseline";
    case PromptCardActionKind::GatePromptTypeRoute80022CBC:
        return "GatePromptTypeRoute80022CBC";
    case PromptCardActionKind::PromptHeader:
        return "PromptHeader";
    case PromptCardActionKind::PromptSingleButton:
        return "PromptSingleButton";
    case PromptCardActionKind::PromptDualChoice:
        return "PromptDualChoice";
    case PromptCardActionKind::PromptChoiceState:
        return "PromptChoiceState";
    case PromptCardActionKind::GatePromptFlashSource80017E6C:
        return "GatePromptFlashSource80017E6C";
    case PromptCardActionKind::PromptFlash20Frames:
        return "PromptFlash20Frames";
    case PromptCardActionKind::CardIoPrompt:
        return "CardIoPrompt";
    case PromptCardActionKind::GateCardIoMsgSource80020A3C:
        return "GateCardIoMsgSource80020A3C";
    case PromptCardActionKind::CardIoBannerText:
        return "CardIoBannerText";
    case PromptCardActionKind::CardIoBannerLayout:
        return "CardIoBannerLayout";
    case PromptCardActionKind::CardIoBannerBoxFill:
        return "CardIoBannerBoxFill";
    case PromptCardActionKind::CardIoBannerCornerSprite:
        return "CardIoBannerCornerSprite";
    case PromptCardActionKind::Gap:
        return "Gap";
    }
    return "Unknown";
}

} // namespace PrSS0PromptCardRenderDirect
