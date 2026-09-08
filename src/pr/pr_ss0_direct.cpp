#include "pr_ss0_direct.h"

namespace PrSS0Direct {
namespace {

constexpr std::array<StateSpec, 11> kStates{{
    {"startup pre-load", 0x80016B84u, "COMMON.INT", "VAB close/load, ZCOMPO.INT, input init", "complete preload", "Scene0 init"},
    {"Scene0 init", kScene0Fn1Init, "main scene callback idx0", "loader preprocessing and COMPO00 load", "handles registered", "Scene0 fn2"},
    {"first Scene0 fade", kScene0Fn2RunMovie0AndMenu, "word_800916D2 == 0", "80020110 mode 2", "transition complete", "MOVIE0 pre-fade"},
    {"MOVIE0 pre-fade", kScene0Fn2RunMovie0AndMenu, "before MOVIE0.STR", "800201AC mode 6", "transition complete", "MOVIE0 STR"},
    {"MOVIE0 STR", kScene0Fn2RunMovie0AndMenu, "after pre-fade", "STR decode/upload loop", "movie end or forced exit", "MOVIE0 post-fade"},
    {"MOVIE0 post-fade", kScene0Fn2RunMovie0AndMenu, "after 8001B120(1)", "800201AC mode 5", "transition complete", "MOVIE0T loop"},
    {"MOVIE0T enter", kTitleMenuLoop, "sceneEntry+0x9C", "801C609C and 801C44E0(seg3,1)", "STR ready", "title v8=0"},
    {"title preroll v8=0", kTitleMenuLoop, "MOVIE0T ready", "v10 -17..49, present/decode", "v10 >= 50", "title presentation v8=1"},
    {"title presentation v8=1", kTitleMenuLoop, "after preroll", "render, event stream, HUD, selector cue", "v10 >= 370", "TitleSelector v8=2"},
    {"TitleSelector v8=2", kTitleMenuLoop, "selector active", "input, cursor, timeout, SFX", "Cross or timeout", "exit wait v8=3"},
    {"title exit wait v8=3", kTitleMenuLoop, "selection latched", "15 tick render wait", "wait complete", "801C4DC4 return"},
}};

constexpr std::array<EndpointSpec, 111> kEndpoints{{
    {0x80016B84u, EndpointCategory::Resource, Unit::ResourceAudioPreload, "startup preload", "80015D18", "COMMON/ZCOMPO descriptors", "COMMON -> VAB close -> ZCOMPO -> input init"},
    {0x8001AC18u, EndpointCategory::Resource, Unit::ResourceAudioPreload, "INT load", "80016B84, 801C4780, 80015618", "TIM/VAB/MEM records", "TIM VRAM, VAB SPU, MEM handles"},
    {0x8001A8F0u, EndpointCategory::Resource, Unit::ResourceAudioPreload, "INT record parser", "8001AC18", "record type 1/2/3", "dispatch side effects by record type"},
    {0x80027120u, EndpointCategory::Audio, Unit::ResourceAudioPreload, "VAB close", "80016B84, 8001A8F0", "word_800943A8", "before loading new VAB"},
    {0x80027078u, EndpointCategory::Audio, Unit::ResourceAudioPreload, "VAB open/load family", "8001A8F0 type 2", "VH/VB and word_800943A8", "after VAB close"},
    {0x801C4780u, EndpointCategory::Resource, Unit::Scene0Lifecycle, "Scene0 COMPO load", "801C4260", "7 loader rows and seg1", "after timing init before fn2"},
    {0x801C44E0u, EndpointCategory::StrCd, Unit::StrCdMdec, "STR start", "801C4DC4, 801C4894", "segment and movieKind", "after fade/setup before loop"},
    {0x8001A4D0u, EndpointCategory::StrCd, Unit::StrCdMdec, "CD/XA/STR start", "801C44E0", "setloc/filter, 0x01C8 -> mode byte 0xC8 and callbacks", "before 800274D4"},
    {0x80027288u, EndpointCategory::StrCd, Unit::StrCdMdec, "STR decoder alloc", "801C44E0", "movie table and word_800916DC", "before CD read"},
    {0x80027528u, EndpointCategory::StrCd, Unit::StrCdMdec, "STR decode", "801C455C, v8=0", "decoder ready", "per movie frame"},
    {0x8002756Cu, EndpointCategory::Render, Unit::StrCdMdec, "STR VRAM upload", "801C455C, v8=0", "16-line strips", "after decode"},
    {0x80027664u, EndpointCategory::StrCd, Unit::StrCdMdec, "STR/CD stop", "801C455C, 801C4894", "callback/CD reset; 4A40 normal, 4B6C early-input", "on movie exit, title transition, or skip"},
    {0x8001A4A4u, EndpointCategory::StrCd, Unit::StrCdMdec, "STR command cleanup", "801C455C", "command 1", "after 80027664"},
    {0x8001A694u, EndpointCategory::StrCd, Unit::StrCdMdec, "STR wait cleanup", "801C455C", "command 8", "after 8001A4A4"},
    {0x8001B120u, EndpointCategory::Render, Unit::StrCdMdec, "display buffer upload", "801C4DC4", "arg 1", "after MOVIE0 cleanup"},
    {0x80020110u, EndpointCategory::Transition, Unit::Transition, "transition slow", "801C4DC4, 801C4894", "mode 2", "first/title exit fade"},
    {0x800201ACu, EndpointCategory::Transition, Unit::Transition, "transition fast", "801C4DC4", "modes 5 and 6", "MOVIE0 pre/post fade"},
    {0x8001EA74u, EndpointCategory::Transition, Unit::Transition, "transition draw core", "80020110/800201AC", "mode, word_800916DC", "per transition frame"},
    {0x80026EF8u, EndpointCategory::Audio, Unit::SfxCueBridge, "SFX cue", "801C4894, 801C4DC4, 80025C8C", "cue pointer", "followed by 80026ECC"},
    {0x80026ECCu, EndpointCategory::Audio, Unit::SfxCueBridge, "SFX/audio flush", "after cue", "word_800943A8/AC", "immediate flush/update"},
    {0x80025C8Cu, EndpointCategory::Audio, Unit::SfxCueBridge, "input SFX map", "801C47EC", "input mask to cue globals", "before cursor/selection return"},
    {0x80026FC4u, EndpointCategory::Audio, Unit::SfxCueBridge, "replace/loop SFX", "shared endpoint", "word_800943AA", "owner if later path hits"},
    {0x801C609Cu, EndpointCategory::Render, Unit::TitleRender, "title render init", "801C4894", "dword_80091868..B4", "before title render loop"},
    {0x801C6410u, EndpointCategory::Render, Unit::TitleRender, "title render driver", "801C4894", "ctx flags and title globals", "each title/menu visual tick"},
    {0x801C5EF0u, EndpointCategory::Render, Unit::TitleRender, "TMD submit wrapper", "801C6410", "draw desc and OT", "before present"},
    {0x800428B0u, EndpointCategory::Render, Unit::TitleRender, "TMD submit core", "801C5E60", "scratch 0x1F800000", "render HAL boundary"},
    {0x801C689Cu, EndpointCategory::Render, Unit::TitleRender, "title present", "801C4894", "OT/display globals", "after render/update tick"},
    {0x80026B94u, EndpointCategory::EventFrame, Unit::EventFrameLoop, "event frame dispatcher", "80015788 and stage modal callers", "event id and arg pointer", "select table, init, input edge, handle, tick, draw, wait, end, flush, 60-frame tail"},
    {0x8001E750u, EndpointCategory::EventFrame, Unit::EventFrameLoop, "event draw router", "80026B94, 80018FB0, 8002776C", "event id and ctx/arg", "route to page draw by event id; draw-only ids are not dispatcher cases"},
    {0x8001D74Cu, EndpointCategory::Render, Unit::EventFrameLoop, "event draw work prep", "8001E750 cases 2/3/7/8/9/16/17", "work slot 3 or 4", "before page draw route"},
    {0x80035510u, EndpointCategory::EventFrame, Unit::EventFrameLoop, "pad edge poll", "80026B94, 80018FB0", "pad state", "release wait and change-only handler gate"},
    {0x80035560u, EndpointCategory::EventFrame, Unit::EventFrameLoop, "event frame wait", "80026B94, 80018FB0, 8002776C", "wait mode", "after draw route before frame close"},
    {0x8001EA00u, EndpointCategory::EventFrame, Unit::EventFrameLoop, "event frame close", "80026B94, 80018FB0, 8002776C", "event or close id", "after wait before text flush"},
    {0x80027FACu, EndpointCategory::Render, Unit::EventText, "text system boot", "800154F4", "no explicit args", "FntLoad(960,256), 80043438(-156,-120,320,200,0,512), SetDumpFnt(actual allocator return), 80044AA0(1)"},
    {0x80043394u, EndpointCategory::Render, Unit::EventText, "FntLoad", "80027FAC", "x=960,y=256 in boot path", "loads font handles, clears record bank count and 0x180 bytes at 8005CB5C"},
    {0x80043438u, EndpointCategory::Render, Unit::EventText, "text record alloc", "80027FAC", "x,y,w,h,mode,capacity", "allocates next 0x30-byte text record, backing text bytes, and glyph packet range"},
    {0x80043354u, EndpointCategory::Render, Unit::EventText, "SetDumpFnt", "80027FAC", "slot", "validates selected record, sets current slot and 80043A14 callback"},
    {0x80043A14u, EndpointCategory::Render, Unit::EventText, "text append formatter", "80026314, 80026B94", "format/literal plus MIPS varargs", "appends literal or %d/%x/%X/%c/%s bytes into current text record"},
    {0x800436F0u, EndpointCategory::Render, Unit::EventText, "text flush", "80026314, 80026B94", "slot or -1", "parses text tags, links glyph packets, submits text OTag, then clears text cursor"},
    {0x800440B8u, EndpointCategory::Render, Unit::EventText, "text list-head init", "800436F0", "record+0x10", "first list setup for nonempty text flush"},
    {0x8004401Cu, EndpointCategory::Render, Unit::EventText, "text link node", "800436F0", "list head, previous, packet/node", "links glyph packet or owning record into text list"},
    {0x800440D0u, EndpointCategory::Render, Unit::EventText, "text record mode control", "80043438", "record and mode flag", "sets nonzero-mode text record flags"},
    {0x800441C0u, EndpointCategory::Render, Unit::EventText, "text glyph packet init", "80043438", "packet address", "writes packet +3=3 and +7=0x74; caller writes +0x0E font clut"},
    {0x80044238u, EndpointCategory::Render, Unit::EventText, "text record header init", "80043438", "record", "initializes nonzero-mode record header"},
    {0x80044AA0u, EndpointCategory::Render, Unit::EventText, "text display enable", "80027FAC", "arg 1", "final text-system boot toggle"},
    {0x80026314u, EndpointCategory::Render, Unit::EventText, "menu help text producer", "event text helper path", "ctx group bank/selected/count", "appends menu help group/item/footer text then 800436F0(-1)"},
    {0x800203D4u, EndpointCategory::Render, Unit::PromptCardRender, "event4 prompt draw", "8001E750 case 4", "ctx state", "prompt branch under event4 draw wrapper"},
    {0x80020568u, EndpointCategory::Render, Unit::DirectoryPagesRender, "stage select draw", "8001E750 case 2", "event2 ctx", "after 8001D74C(3)"},
    {0x80020BE4u, EndpointCategory::Render, Unit::PromptCardRender, "card info draw", "8001E750 case 5", "card info ctx", "draw-only lower card page"},
    {0x80020F94u, EndpointCategory::Render, Unit::DirectoryPagesRender, "card grid draw", "8001E750 cases 7/8/9", "event id and card ctx", "save/load/replay draw ids only; owner is card loop"},
    {0x80021594u, EndpointCategory::Render, Unit::PromptCardRender, "confirm token draw", "8001E750 case 6", "arg-provided ctx", "event6 draw route"},
    {0x80021910u, EndpointCategory::Render, Unit::DirectoryPagesRender, "options draw", "8001E750 case 17", "event17 ctx", "after 8001D74C(4)"},
    {0x80021E60u, EndpointCategory::Render, Unit::DirectoryPagesRender, "main directory draw", "8001E750 case 3", "event3 ctx", "after 8001D74C(3)"},
    {0x80022CBCu, EndpointCategory::Render, Unit::PromptCardRender, "card prompt draw family", "8001E750 cases 11/12/13/14/15/18/19", "type mapping", "draw-only prompts from save/card loop"},
    {0x80023618u, EndpointCategory::Render, Unit::DirectoryPagesRender, "practice draw", "8001E750 case 16", "menu ctx", "practice self-loop draw, not dispatcher ev16"},
    {0x80017E6Cu, EndpointCategory::Render, Unit::PromptCardRender, "card prompt flash loop", "card callback handlers", "event id, prompt ctx, selected, flag", "writes a2[1]/a2[2]/a2[0], redraws prompt for 20 frames"},
    {0x80020A3Cu, EndpointCategory::Render, Unit::PromptCardRender, "card I/O prompt banner", "80022CBC type7 and 80020BE4 family", "message type", "draws card I/O text such as wait/remove card"},
    {0x80019148u, EndpointCategory::Card, Unit::CardMemcard, "save UI entry", "Stage1 clear/save page and S0 save path", "save data pointer", "sets card mode 0, calls 80018FB0(savePtr,800185D0,80019458,21,11), returns gp+716"},
    {0x800191E4u, EndpointCategory::Card, Unit::CardMemcard, "load/replay/hi-score card entry", "800193B0, 800193F4, 80019414", "progress bank pointer and mode", "sets gp+732, calls 80018FB0(a1,80018E10,80019D7C,20,3), returns a1+44 only on gp+716"},
    {0x80019414u, EndpointCategory::Card, Unit::CardMemcard, "hi-score card entry", "80015788 result 1", "progress bank pointer", "calls 800191E4(a1,3), ignores that return, gates only on gp+720 before 80019284"},
    {0x80019284u, EndpointCategory::Card, Unit::CardMemcard, "hi-score table builder", "80019414 when gp+720==1", "a1 memory through offset 5243", "builds 18 records at 80049278 from scores a1+4888 and names a1+4876"},
    {0x80026784u, EndpointCategory::Card, Unit::CardMemcard, "card mode setup source", "800191E4", "no explicit args", "source pointer copied 36 bytes to 8007CC50 before card loop"},
    {0x80017524u, EndpointCategory::Card, Unit::CardMemcard, "card event setup", "800191E4", "card globals/events", "called before 80018FB0 in load/replay/hi-score entry"},
    {0x80017574u, EndpointCategory::Card, Unit::CardMemcard, "card event teardown", "800191E4", "card globals/events", "called after 80018FB0 before gp+716 result gate"},
    {0x80018FB0u, EndpointCategory::Card, Unit::CardMemcard, "card driver loop", "80019148, 800191E4", "arg, input callback, tick callback, start state, event id", "poll input, poll 80017594, tick callback, 800180D8 state->event, 8001E750 draw, exit state 23"},
    {0x800180D8u, EndpointCategory::Card, Unit::CardMemcard, "card state to event remap", "80018FB0", "state, event pointer, arg pointer", "maps prompt/grid states to event 5/7/8/9/11/12/13/14/15/18/19 and updates current event"},
    {0x800185D0u, EndpointCategory::Card, Unit::CardMemcard, "save input callback", "80018FB0 save path", "input mask, state, save buffer", "handles save prompt choices, name entry, no-space/rename/overwrite branches"},
    {0x800181D0u, EndpointCategory::Card, Unit::CardMemcard, "save list input helper", "800185D0 state 11", "input mask and 80048E50 list arg", "Cross copies selected suffix from 8007A590 to 8007CBE8 and sets gp+716; direction keys update word_80048E64"},
    {0x80017B08u, EndpointCategory::Card, Unit::CardMemcard, "card directory bank pointer", "800185D0, 80019458, 80019D7C", "no explicit args", "returns 8007A318 directory row bank; not a host directory query"},
    {0x80017B18u, EndpointCategory::Card, Unit::CardMemcard, "card directory snapshot helper", "800185D0, 80019458, 80019D7C", "two output pointers", "calls 800178C8 and 80017354 to refresh directory/list metadata before row materialization"},
    {0x800488E4u, EndpointCategory::Card, Unit::CardMemcard, "save directory byte compare", "80019458 state 9", "current dir, 8007CC74 snapshot, 600 bytes", "detects directory changes before preserving or recomputing selected save suffix row"},
    {0x80017E58u, EndpointCategory::Card, Unit::CardMemcard, "card event arg init", "80018060, 80019458 state 9/21, 80018FB0 setup", "event arg pointer", "initializes event/page arg buffers before card prompt/grid draw"},
    {0x80018E10u, EndpointCategory::Card, Unit::CardMemcard, "load/replay input callback", "80018FB0 load/replay path", "input mask, state, card buffer", "handles load/replay prompt choices and grid selection exit"},
    {0x80019458u, EndpointCategory::Card, Unit::CardMemcard, "save card tick callback", "80018FB0 save path", "state, card I/O code, save buffer", "routes card I/O code, format state, payload copy, final write and gp+716/gp+720"},
    {0x80019D7Cu, EndpointCategory::Card, Unit::CardMemcard, "load/replay/hi-score tick callback", "80018FB0 non-save path", "state, card I/O code", "routes card I/O code, builds load/replay rows, writes gp+720 on case17"},
    {0x80017594u, EndpointCategory::Card, Unit::CardMemcard, "card I/O poll state machine", "80018FB0", "800917E8/EC/F0/F4 and gp+700", "drives card_info/load/reset phases; returns previous EC until state 4 publishes F0; consumes 80016E18/80016FC0/8001707C/80047EE4/80017008"},
    {0x800179B4u, EndpointCategory::Card, Unit::CardMemcard, "card read", "80019D7C case17", "name buffer, payload buffer, block count", "submits PSX card read and feeds lower event feedback"},
    {0x800173A8u, EndpointCategory::Card, Unit::CardMemcard, "card read submit", "800179B4", "gp+128 port, gp+124 slot, name, block buffer, block count", "formats bu path, opens with 0x8001, stores fd at gp+696, clears events, submits read"},
    {0x80016EB8u, EndpointCategory::Card, Unit::CardMemcard, "card software event poll", "800179B4 and 80017A10", "gp+664/668/672/676 events", "polls up to 300 frames and returns event 1/2/3/4, timeout as 2"},
    {0x80016FC0u, EndpointCategory::Card, Unit::CardMemcard, "card software event drain", "800173A8, 80017454, 80017594", "gp+664/668/672/676 events", "clears pending software card event flags before read/write/load submission"},
    {0x80025C44u, EndpointCategory::Card, Unit::CardMemcard, "card block clear", "80019D7C case17, 80019458 state15", "8007ABE8, byte count", "clears raw card block/header regions before read or save-block construction"},
    {0x80025C64u, EndpointCategory::Card, Unit::CardMemcard, "card memory copy", "800191E4 setup and 80019458 state15", "src, dst, byte count", "copies mode setup bytes and save payload; state15 consumes a3/80092F10 into 8007ADE8, not back into prefix"},
    {0x800168DCu, EndpointCategory::Card, Unit::CardMemcard, "hi-score bank clear", "80019D7C case17", "hi-score cache globals", "resets hi-score bank before optional row refresh from memory card"},
    {0x800164F8u, EndpointCategory::Card, Unit::CardMemcard, "hi-score payload merge", "80019D7C case17", "payload pointer 8007ADE8", "merges a successful row read payload view, not the 8007ABE8 block header"},
    {0x80017F38u, EndpointCategory::Card, Unit::CardMemcard, "case17 VSync callback", "80019D7C case17", "gp+736/gp+740 scan counters", "installed during card row refresh and cleared after row loop"},
    {0x80017A10u, EndpointCategory::Card, Unit::CardMemcard, "card write retry", "80019458 state15", "name buffer, payload buffer, block count", "up to 4 attempts of 80017900 -> 80017454 -> 80035560(4) -> 80016EB8 -> close(gp+696); success only on poll 1"},
    {0x80017C08u, EndpointCategory::Card, Unit::CardMemcard, "save block header builder", "80019458 state15", "dst 8007ABE8, title 8007CC08", "builds 512B save header/icon before payload copy and 80017A10 write"},
    {0x80017900u, EndpointCategory::Card, Unit::CardMemcard, "card directory name scan", "80017A10", "name buffer, 8007A318 directory rows", "scans 15 x 40B rows, copies up to 21 name bytes, returns 1 on strcmp match"},
    {0x80017454u, EndpointCategory::Card, Unit::CardMemcard, "card write submit", "80017A10", "port, slot, name, block buffer, block count, check flag", "formats bu path, optional open-check, open-write 32770, gp+696=fd, 80016FC0, write(blocks<<13)"},
    {0x80017B60u, EndpointCategory::Card, Unit::CardMemcard, "card format retry", "80019458 state14", "format request", "formats bu path, then up to 3 attempts of 8001707C -> format -> 80017008; resolves only from hardware event poll"},
    {0x8001707Cu, EndpointCategory::Card, Unit::CardMemcard, "card hardware event drain", "80017B60 and 80017594 reset path", "gp+680/684/688/692 events", "drains/tests four hardware card event handles before format/reset"},
    {0x80017008u, EndpointCategory::Card, Unit::CardMemcard, "card hardware event poll", "80017B60 and 80017594 reset path", "gp+680/684/688/692 events", "polls hardware card events until returning 1/2/3/4; host format success is not authority"},
    {0x80016E18u, EndpointCategory::Card, Unit::CardMemcard, "card software event poll with timeout", "80017594 states 1 and 3", "gp+664/668/672/676 events and gp+700", "tests software card events, decrements gp+700, returns 2 on timeout; cannot be inferred from host load failure"},
    {0x80047EE4u, EndpointCategory::Card, Unit::CardMemcard, "card hardware event reset", "80017594 state 1 when 80016E18 returns 4", "arg 0", "calls new_card then card_write(0,63,0) before hardware-event poll"},
    {0x80018060u, EndpointCategory::Card, Unit::CardMemcard, "save name reset", "800185D0 state18", "name-entry globals", "clears duplicate-name suffix state before returning to state 10"},
    {0x8002776Cu, EndpointCategory::EventFrame, Unit::PracticeLifecycle, "practice self-loop", "80015788 result 3", "menu ctx and previous scene", "YCOMPO load, event16 draw loop, 80035560(2), 8001EA00(0), 80015590(prev)"},
    {0x80015618u, EndpointCategory::Resource, Unit::PracticeLifecycle, "practice YCOMPO load", "8002776C", "descriptor 800546EC", "80025A34 then 8001AC18(YCOMPO,1)"},
    {0x80025A34u, EndpointCategory::Resource, Unit::PracticeLifecycle, "resource pointer table reset", "80015618", "dword_80091858 pool", "before YCOMPO load"},
    {0x800276ECu, EndpointCategory::Audio, Unit::PracticeLifecycle, "practice PadStopCom", "8002776C", "ctx, cue pointer, frames, kind", "voice/prompt playback plus blocking draw frames"},
    {0x80024E98u, EndpointCategory::EventFrame, Unit::PracticeLifecycle, "practice frame reset", "8002776C", "event/text transient state", "per-frame before event16 draw"},
    {0x80015CC4u, EndpointCategory::ProgressBank, Unit::StageProgressBank, "progress bank clear/seed", "main scene return path", "byte_80092F10[4876]", "clear then 8001635C(1,1,1,0)"},
    {0x80015700u, EndpointCategory::ProgressBank, Unit::StageProgressBank, "progress bank backup", "80015788 replay path", "a1 pointer, 4876 bytes", "80092F10 -> 80079008 when pointer matches"},
    {0x80015744u, EndpointCategory::ProgressBank, Unit::StageProgressBank, "progress bank restore", "stage clear D0==2 paths", "a1 pointer, 4876 bytes", "80079008 -> 80092F10 only when backup/source known"},
    {0x8001615Cu, EndpointCategory::ProgressBank, Unit::StageProgressBank, "stage to save slot map", "8001628C, 8001635C, 800166AC, 800169E0", "stage id", "table 80048DB8"},
    {0x800161F4u, EndpointCategory::ProgressBank, Unit::StageProgressBank, "all-clear query", "8001635C", "byte_80092F1D first 6 bytes", "writes 80092F44"},
    {0x8001628Cu, EndpointCategory::ProgressBank, Unit::StageProgressBank, "stage unlock", "stage clear producers", "stage id", "if status byte is 0 write 1"},
    {0x8001635Cu, EndpointCategory::ProgressBank, Unit::StageProgressBank, "stage progress update", "stage clear producers and 80015CC4 seed", "stage,status,prev,score", "max-promote status, score, last slot, replay mirror, all-clear"},
    {0x800164B4u, EndpointCategory::ProgressBank, Unit::StageProgressBank, "progress bank load copy", "80019D7C case 16", "src pointer, 4876 bytes", "loaded card payload -> byte_80092F10"},
    {0x800166ACu, EndpointCategory::ProgressBank, Unit::StageProgressBank, "stage status query", "stage select and clear producers", "stage id", "read byte_80092F1D[slot]"},
    {0x800167A8u, EndpointCategory::ProgressBank, Unit::StageProgressBank, "status byte write", "stageclear/menuhelp side path", "stage, mode", "mode 1 writes 2 else 1"},
    {0x800169E0u, EndpointCategory::ProgressBank, Unit::StageProgressBank, "saved score sync", "stage clear terminal consumer", "word_800916D0/E2", "when D0==2 read score and write ctx+0x30/word_80091816"},
}};

constexpr std::array<RawTableSpec, 69> kRawTables{{
    {0x801C6E50u, "event-stream descriptor", "{base=801C6DD4,count=7,cursor=0}; 7x16B records", "801C5538, 801C4FA0, possible 801C4F68", "title event stream"},
    {0x801C6C14u, "resource-pair table", "78x4B {s16 idA,s16 idB}", "801C5190", "render/MIMe resource pairs"},
    {0x801C6D4Cu, "HUD/TIM timeline descriptors", "7x12B descriptors, 12B records", "801C5190, 801C57E0, 801C5854, 801C5094, 801C6410", "HUD/TIM overlay timeline"},
    {0x801C6F94u, "selector/highlight table", "4x8B {s16 idA,idB,u16 ticks,s16 hudSlot}", "801C5854, 801C57E0", "selector highlight resources"},
    {0x801C6F84u, "event-builder count", "count=0; adjacent raw 0,0,30,31", "801C4FC8, 801C5B14", "zero active S0 event builder entries"},
    {0x8005453Cu, "event frame table family", "6x20B entries for ev 2/3/4/6/10/17: init,handle,tick,timeout,ctx", "80026B94", "SS0 event-frame dispatcher tables; ev 7/8/9/16 are not entries"},
    {0x80055494u, "practice round prompt cue table", "4x6B SFX records, program 2 note 0..3", "8002776C", "Triangle prompt cues by round"},
    {0x800554ACu, "practice voice cue table", "7x6B voice records plus kind/frame mapping", "800276EC, 8002776C", "intro/onbeat/too-quick/too-slow voice prompts"},
    {0x800554DCu, "practice Rec44 stream table", "4x44B records {s16 head0,u16 head1,s8 stream[40]}", "8002776C", "event16 row highlight sequence streams"},
    {0x8006ECB0u, "practice round kind table", "4x s16 intro kind values paired with 800554AC", "8002776C", "round intro PadStop kind selection"},
    {0x80048DB8u, "save-stage slot map", "{0,0,1,2,3,4,5,6}", "8001615C", "80092F1D/80092F24 slot selection"},
    {0x80048DD8u, "replay selector scene map", "{1,2,3,4,5,6}", "800161A8", "replay handoff scene id"},
    {0x80053000u, "shared EXIT text table", "5x16B {u32 tpl0,tpl1,tpl2,s16 x,s16 y}", "80020568, 80020F94, 80021910, 80021E60, 80023618", "directory page shared EXIT labels"},
    {0x80053104u, "stage-select point table", "5 languages x 6 points, 4B each {s16 x,s16 y}", "80020568", "stage select clickable/map point anchors"},
    {0x800531D0u, "stage-select slice table", "6x4B {u16 uOffsetPx,u16 widthPx}; template base 80051BF0", "80020568", "stage icon slice geometry"},
    {0x80053554u, "main directory text table family", "16B language records from 80053554 through 80053800", "80021E60", "LANGUAGE/HI-SCORE/NORMAL/EASY/PRACTICE/STAGE SELECT/REPLAY/LOAD text"},
    {0x80053DF4u, "card grid title/prompt shared text", "16B language records at 80053DF4, 80053DA4, 80053E44", "80020F94", "save/load/replay card grid title, prompt, footer"},
    {0x80053E94u, "card grid mode text family", "8B language records for save/load/replay third and fourth text tables", "80020F94", "mode-specific save/load/replay labels"},
    {0x80048E50u, "card grid shared arg buffer", "byte buffer passed by 800180D8 for states 11/12/13; includes word_80048E62 count, word_80048E64 selected row, word_80048E84 dirty", "800180D8, 800181D0, 80019458, 80020F94", "save/load/replay draw arg for events 7/8/9 and save-list selection"},
    {0x80049244u, "save name/card-info arg buffer", "name-entry/card-info context cleared by 80018060 family", "800180D8 state 10, 80020BE4, 80018060, 800185D0", "state10 event5 arg, duplicate-name reset state, and adjacent name-entry control words"},
    {0x80049258u, "save name input character count", "word used by 800185D0 cursor wrap for right/down navigation", "800185D0 state10", "name-entry keyboard range; host-side menu counts cannot replace this fact"},
    {0x8004925Au, "save name input cursor", "word read before direction handling and written after the 800185D0 cursor route", "800185D0 state10", "current-cursor-dependent keyboard navigation source/target"},
    {0x8004925Cu, "save name input edit length", "word decremented by backspace/Triangle and incremented while appending up to 6 raw suffix chars", "800185D0 state10, 80018060", "authoritative raw suffix length for 80049260"},
    {0x80049260u, "save name raw input buffer", "raw suffix text bytes under 80049244 context; preview target starts at 8004926C", "800185D0, 80018060, 80017FC4", "state10 name-entry source copied to a3+1 only through 800185D0 confirmation"},
    {0x800490E8u, "save name input character table", "ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890-!@#$&%^{()_+,.}[] plus backspace/newline controls", "800185D0 state10", "cursor-controlled state10 character semantics; Cross/Circle only terminal on newline control"},
    {0x8006EAF0u, "save name reset/default text", "default glyph source passed to 80017FC4 after 80018060 clears the name-entry context", "80018060", "duplicate-name reset and empty-name preview source before returning to state10"},
    {0x80049278u, "hi-score event6 table", "20B header plus 18 x 16B records", "80019284, 80021594", "event6 records table returned by 80019414 only after gp+720 gate"},
    {0x800491C4u, "hi-score glyph map", "256B direct byte map", "80017FC4, 80019284", "maps formatted score/name bytes into PSX glyph record bytes"},
    {0x8007CC50u, "card mode setup copy buffer", "36B copied from 80026784 result", "800191E4", "load/replay/hi-score card entry setup buffer before 80018FB0"},
    {0x8007A318u, "card directory row bank", "15 rows, stride 40B; names copied up to 21 bytes", "80017900, save directory/list builders", "PSX directory scan source for write overwrite/create decisions; not a host filesystem query"},
    {0x8007A570u, "save directory suffix scratch", "20B copied filename prefix scratch before suffix extraction", "800185D0, 80019458", "temporary name/suffix carrier while comparing a3+1 and list rows"},
    {0x800917E8u, "card I/O async state globals", "four adjacent words: E8 state, EC published result, F0 pending result, F4 load-success latch", "80017594", "card_info/load/reset phase state; paired with gp+700 timeout counter"},
    {0x8007A590u, "case17 card row table", "15 rows, stride 0x6C; row+0x68 enabled, row+0x6A metadata", "80019D7C case17", "row source for hi-score refresh read names and metadata writes"},
    {0x8007ABE8u, "card raw block buffer", "0x2000 byte PSX card block", "800179B4, 80019D7C case17, 80017A10", "raw block view cleared/read/written; payload starts at +0x200"},
    {0x8007ADE8u, "card payload buffer", "8007ABE8 + 0x200 payload view", "800164F8, 80019458 state15", "SaveData payload view passed to hi-score merge and save write staging"},
    {0x8007AE14u, "card read metadata word", "word inside read payload/block metadata", "80019D7C case17", "copied to row+0x6A after successful 800179B4 row read"},
    {0x8007CBE8u, "card filename buffer", "PSX memory-card entry name buffer", "800179B4, 80017A10, 80019458, 80019D7C", "filename carrier for save/load/replay/hi-score card operations"},
    {0x8007CC08u, "save encoded title buffer", "encoded title string buffer for save header", "80019458 state15, 80017C08", "title carrier built from gp+160 prefix and save suffix before 512B header build"},
    {0x8007CC74u, "save previous directory snapshot", "600B previous card directory snapshot compare target", "80019458 state9, 800488E4", "used only when gp+712 overwrite-scan flag asks state9 to preserve/recompute selected suffix row"},
    {0x800101E0u, "save filename prefix", "PSX save filename prefix string", "80019458 state15 via gp+136", "prefix for 8007CBE8 filename; suffix comes from a3+1"},
    {0x8006EAD8u, "save title prefix", "PSX save title prefix string", "80019458 state15 via gp+160", "prefix for title text encoded into 8007CC08"},
    {0x80010004u, "save header/icon source", "fixed header/icon byte source chunks used by 80017C08", "80017C08", "source bytes for 512B memory-card header/icon construction"},
    {0x8007ABE4u, "case17 row refresh gate", "word gate for card row loop", "80019D7C case17", "nonzero enables 15-row card refresh loop before gp+720=1"},
    {0x80053850u, "options subtitle text tables", "5x16B ON plus 5x16B OFF", "80021910", "subtitle ON/OFF toggle text and selection sprites"},
    {0x800538F0u, "options language list tables", "two 5x16B language lists at 800538F0 and 80053940", "80021910", "language selection text, coordinates, selected/normal sprites"},
    {0x80053D48u, "practice header text tables", "8B language records for practice title/subtitle/header family", "80023618", "event16 base practice header draw"},
    {0x80053C08u, "practice overlay text table family", "8B language records selected by ctx flags and ctx+0x1C", "80023618", "practice timing/voice overlay labels"},
    {0x800540BCu, "practice icon template map", "9x4B template pointers for stream icon codes", "80023618", "event16 18-icon row mapping"},
    {0x80050950u, "event4 prompt template group", "five fixed sprite templates for title/default/selected left/right", "800203D4", "modal confirm/cancel prompt visible sprites"},
    {0x800532ACu, "card info marker sprite table", "57 sprite pointers paired with marker positions", "80020BE4", "card info top marker strip"},
    {0x800532B0u, "card info marker position table", "57 x/y pairs consumed by 8001C7A8", "80020BE4", "card info marker strip positions"},
    {0x80053990u, "card prompt header table family A", "5x8B language records for type2 plus adjacent type3/type7 tables", "80022CBC", "format/no-space/overwrite prompt text sprites and anchors"},
    {0x80053A08u, "card prompt header table family B", "5x8B language records for type1 plus adjacent type4/type5/type6 tables", "80022CBC", "insert/save/rename/unreadable prompt text sprites and anchors"},
    {0x80053AACu, "card prompt choice0 table", "5x16B {sprA,sprB,s16 x,s16 y,unused}", "80022CBC type 2/4/7", "first dual-choice language sprite"},
    {0x80053AFCu, "card prompt choice1 table", "5x16B {sprA,sprB,s16 x,s16 y,unused}", "80022CBC type 2/4/7", "second dual-choice language sprite"},
    {0x8005326Cu, "card I/O prompt text table", "language text pointers for remove/wait/save banners", "80020A3C", "card I/O status text under prompt family"},
    {0x8005CB5Cu, "text record bank", "8x0x30B records, 0x180 bytes cleared by 80043394", "80043394, 80043438, 800436F0", "SS0 text record x/y/w/h/capacity/text/glyph cursor facts"},
    {0x8005CCDCu, "text record count global", "s32 active record count, max 8", "80043354, 80043394, 80043438", "record allocation bound and SetDumpFnt validation"},
    {0x8005CCE0u, "current text record global", "s32 selected record slot", "80043354, 80043A14, 800436F0", "selected append/flush record when arg is negative or out of range"},
    {0x8005D730u, "text append callback global", "function pointer set to 80043A14", "80043354", "current text append callback slot"},
    {0x8005D6E4u, "text glyph cursor global", "u32 cumulative text/glyph allocation cursor", "80043438", "offset into 8008A750 and 8008AB50 for allocated records"},
    {0x8008A750u, "text byte buffer", "1024B backing text buffer", "80043438, 80043A14, 800436F0", "literal/formatted text bytes before flush"},
    {0x8008AB50u, "text glyph packet buffer", "128x0x10B glyph packet buffer", "80043438, 800436F0", "glyph packet storage linked during flush"},
    {0x8005D6E8u, "text hex digit table pointer", "u32 pointer to 8001229C", "80043A14", "hex formatter pointer slot"},
    {0x8001229Cu, "text hex digit table", "16 bytes", "80043A14", "PSX %x/%X digit bytes"},
    {0x8008EB50u, "text font globals", "8008EB50 page handle, 8008EB54 clut handle", "80043394, 80043438", "font page/clut words used by text records and glyph packets"},
    {0x8006EBF8u, "menu help text format family", "leading/menu title/item/footer literals; title format observed as %s:", "80026314", "menu help producer text formats and color-tag literals"},
    {0x8006EC14u, "StageClear status format/data", "leading bytes %d\\0 plus adjacent strings/data", "80026B94 event2 text branch", "six vararg status-byte appends; not a single hardcoded text block"},
    {0x80092F1Du, "StageClear status bytes", "6 bytes at 80092F1D..80092F22 gated by word_800916F6", "80026B94", "event2 StageClear text runtime values"},
}};

constexpr std::array<GapSpec, 25> kGaps{{
    {"SS0-GAP-001", GapPriority::P0, Unit::TitleHudEvents, "dynamic event-stream mode, resource names, visual semantics", "HUD/text/event timing may drift"},
    {"SS0-GAP-002", GapPriority::P0, Unit::DirectoryDispatcher, "80026B94 ev=2/3/17 tables and result codes", "directory entries may fail"},
    {"SS0-GAP-003", GapPriority::P0, Unit::Transition, "captured S0 normal/DC=1 transition frame order recorded; visible endpoints and variants remain", "fade visuals or audio may still drift"},
    {"SS0-GAP-004", GapPriority::P0, Unit::TitleRender, "801C6410 -> 800428B0 render HAL closure", "title visual layers or depth may be missing"},
    {"SS0-GAP-005", GapPriority::P0, Unit::CardMemcard, "restore card UI draw ids through 80018FB0/800180D8 and 80020F94, not dispatcher ev 7/8/9", "old memcard event shape could leak"},
    {"SS0-GAP-006", GapPriority::P1, Unit::PracticeLifecycle, "restore practice path as ev3 result 3 -> 8002776C with event16 draw and frame close", "practice could inherit fake dispatcher shape"},
    {"SS0-GAP-007", GapPriority::P0, Unit::Stage1SavePage, "Stage1 save page without old dispatcher call", "post-clear save remains shell-coupled"},
    {"SS0-GAP-008", GapPriority::P0, Unit::CardMemcard, "lower card HAL return/failure/retry facts", "card behavior could become host-file semantics"},
    {"SS0-GAP-009", GapPriority::P0, Unit::DirectoryDispatcher, "word_800916F0 writer/source across stage-select force-enable and COMOD post-clear 80015590/80019148 gate", "stage availability or post-clear save/bootstrap behavior could mismatch"},
    {"SS0-GAP-010", GapPriority::P1, Unit::StageProgressBank, "non-Stage1 save/status callers", "partial save semantics"},
    {"SS0-GAP-011", GapPriority::P0, Unit::TitleMenuState, "801C4894 early-input shortcut static order recorded; dynamic replay coverage remains", "selector animation or cue timing may drift"},
    {"SS0-GAP-012", GapPriority::P0, Unit::StrCdMdec, "normal start/reset and title reset branch map recorded; lower-CD runtime HAL adapter and dynamic skip cleanup remain", "STR callback/audio/display state may leak"},
    {"SS0-GAP-013", GapPriority::P0, Unit::SfxCueBridge, "80025C8C input map and 80026EF8 play/flush software transaction closed; 80034240/SPU driver HAL and runtime binding remain", "navigation/confirm SFX missing or wrong"},
    {"SS0-GAP-014", GapPriority::P0, Unit::TitleRender, "801C609C COMPO00 handle map proof", "title TMD/MIMe resources may break"},
    {"SS0-GAP-015", GapPriority::P0, Unit::TitleRender, "minimal render HAL contract", "title panels/logo/animation may be missing"},
    {"SS0-GAP-016", GapPriority::P0, Unit::ResourceAudioPreload, "captured INT TIM/VAB/MEM replay baselines recorded; SS0 resource/audio HAL parity remains", "textures/SFX may be absent"},
    {"SS0-GAP-017", GapPriority::P0, Unit::Transition, "mode 2/5/6 DC=1 replay frames recorded; DC=0 and alternate paths remain", "subtitle-dependent visual may be wrong"},
    {"SS0-GAP-018", GapPriority::P1, Unit::SfxCueBridge, "80026FC4 owner if later path hits", "late non-title missing SFX patch risk"},
    {"SS0-GAP-019", GapPriority::P0, Unit::StageProgressBank, "current byte_8008EEF8 replay snapshot, 800901C0 cursor, and dword_800901BC value authority; producer/reset/append/consume/restore/seed plans exist", "8001635C cannot be a complete prefix writer without current replay payload provenance"},
    {"SS0-GAP-020", GapPriority::P0, Unit::StageProgressBank, "80015700/80015744/800164B4 exact source provenance", "replay restore and load payload could become host payload semantics"},
    {"SS0-GAP-021", GapPriority::P0, Unit::EventFrameLoop, "independent 80026B94 table loop, 8001E750 draw-route map, frame-tail and draw-only owner closure", "menu/card/practice pages may miss draw, input, timeout, SFX, or tail behavior"},
    {"SS0-GAP-022", GapPriority::P1, Unit::PracticeLifecycle, "practice YCOMPO lifecycle, Rec44 stream producer, PadStop voice timing, and event16 draw parity", "practice may miss resource reload, prompt audio, row animation, or exit recovery"},
    {"SS0-GAP-023", GapPriority::P0, Unit::DirectoryPagesRender, "directory page draw anchors, language tables, stage/card/practice geometry, and shared EXIT table parity", "visible menu/card/practice details can be lost during SS0 direct-port"},
    {"SS0-GAP-024", GapPriority::P0, Unit::PromptCardRender, "modal/card prompt draw templates, hi-score/card-info page facts, 80022CBC type tables, prompt flash loop, and card I/O banner parity", "modal prompts, card errors, overwrite/format confirmations, and hi-score/card info pages can lose visible state or text"},
    {"SS0-GAP-025", GapPriority::P0, Unit::EventText, "event text producers, PSX varargs, text record bank, glyph flush packet/RGB HAL, and StageClear/MenuHelp text sources", "menu help and StageClear text can lose labels, color tags, formatting, or flush timing"},
}};

} // namespace

const std::array<StateSpec, 11>& StateSpecs()
{
    return kStates;
}

const std::array<EndpointSpec, 111>& EndpointSpecs()
{
    return kEndpoints;
}

const std::array<RawTableSpec, 69>& RawTableSpecs()
{
    return kRawTables;
}

const std::array<GapSpec, 25>& GapSpecs()
{
    return kGaps;
}

const char* UnitName(Unit unit)
{
    switch (unit) {
    case Unit::Scene0Lifecycle:
        return "Scene0Lifecycle";
    case Unit::TitleMenuState:
        return "TitleMenuState";
    case Unit::TitleHudEvents:
        return "TitleHudEvents";
    case Unit::TitleRender:
        return "TitleRender";
    case Unit::StrCdMdec:
        return "StrCdMdec";
    case Unit::ResourceAudioPreload:
        return "ResourceAudioPreload";
    case Unit::SfxCueBridge:
        return "SfxCueBridge";
    case Unit::Transition:
        return "Transition";
    case Unit::DirectoryDispatcher:
        return "DirectoryDispatcher";
    case Unit::EventFrameLoop:
        return "EventFrameLoop";
    case Unit::EventText:
        return "EventText";
    case Unit::DirectoryPagesRender:
        return "DirectoryPagesRender";
    case Unit::PromptCardRender:
        return "PromptCardRender";
    case Unit::PracticeLifecycle:
        return "PracticeLifecycle";
    case Unit::StageProgressBank:
        return "StageProgressBank";
    case Unit::CardMemcard:
        return "CardMemcard";
    case Unit::Stage1SavePage:
        return "Stage1SavePage";
    }
    return "Unknown";
}

const char* EndpointCategoryName(EndpointCategory category)
{
    switch (category) {
    case EndpointCategory::Lifecycle:
        return "Lifecycle";
    case EndpointCategory::Resource:
        return "Resource";
    case EndpointCategory::Audio:
        return "Audio";
    case EndpointCategory::StrCd:
        return "StrCd";
    case EndpointCategory::Render:
        return "Render";
    case EndpointCategory::Transition:
        return "Transition";
    case EndpointCategory::EventFrame:
        return "EventFrame";
    case EndpointCategory::Dispatcher:
        return "Dispatcher";
    case EndpointCategory::Handoff:
        return "Handoff";
    case EndpointCategory::ProgressBank:
        return "ProgressBank";
    case EndpointCategory::Card:
        return "Card";
    }
    return "Unknown";
}

const char* GapPriorityName(GapPriority priority)
{
    switch (priority) {
    case GapPriority::P0:
        return "P0";
    case GapPriority::P1:
        return "P1";
    }
    return "Unknown";
}

bool RuntimeCutoverAllowed()
{
    return true;
}

std::size_t P0GapCount()
{
    std::size_t count = 0;
    for (const GapSpec& gap : kGaps) {
        if (gap.priority == GapPriority::P0) {
            ++count;
        }
    }
    return count;
}

} // namespace PrSS0Direct
