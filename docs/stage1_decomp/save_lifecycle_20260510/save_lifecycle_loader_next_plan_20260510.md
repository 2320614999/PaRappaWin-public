# Stage1 save/lifecycle/loader next plan 2026-05-10

Scope: static verification and document output only. No build, no test, no
`src` edit.

This is the P1-D follow-up for `save / lifecycle / loader feedback` from
`docs/stage1_decomp/stage1_overlay_remaining_direct_plan_20260510.md`. The
goal is not to implement another Win-side adapter. The goal is to define the
next IDA export batch and keep the direct/HAL boundary explicit before any
future cutover.

## Static inputs checked

- `docs/项目规则.md`: direct-port work must follow `导出 -> 画图 -> 翻译 -> 删除 -> 接线`; adapters may convert/forward, but must not own logic, cache state, or infer authority.
- `docs/待办事项.md` tail: Save UI still calls out missing `80017594`,
  `80017B18`, byte-exact `80017C08`, `80017A10/80017B60` retry feedback,
  and the rule that Win filesystem / `PrCard` must not be used to close PSX
  card helpers.
- `src/pr/pr_stage1_save_ui_direct.h/.cpp`: Save UI already classifies actions
  into `DirectMemory`, `HostHalBoundary`, `HelperGap`, and
  `UnsupportedHostAction`; card write/format remains an ordered action
  skeleton, not a real feedback loop.
- `src/pr/pr_stage1_lifecycle_direct.h`: `801C81EC` direct actions include
  Movie1 STR, stage runner, abort query, audio, save status, unlock next stage,
  `Bootstrap15590`, and `SaveUi19148`.
- `src/pr/pr_stage1_lifecycle_executor_direct.h/.cpp`: executor already owns
  host-block state for Movie1, ClearTailMovie, AbortPoll, Bootstrap15590, and
  SaveUi19148, and exposes live feedback entry points for record, CD, GPU, SPU,
  PadCom, callback, and completion.
- `src/pr/pr_stage1_loader_direct.*`: Bootstrap15590 loader already has an
  action skeleton and producer-step model. It distinguishes `DirectControl`,
  `LoaderMemoryDirect`, CD/GPU/SPU/PadCom/Callback HAL boundaries, and
  completion.
- `src/pr/pr_stage1_loader_cd_hal.*`,
  `src/pr/pr_stage1_loader_gpu_hal.*`,
  `src/pr/pr_stage1_loader_spu_hal.*`,
  `src/pr/pr_stage1_loader_memory_direct.*`: memory helpers are direct-owned;
  CD/GPU/SPU actions are HAL feedback seams, not places to call host APIs from
  the direct runner.

## Current boundary map

| Area | Current direct owner | Current HAL / host boundary | Remaining gap |
|---|---|---|---|
| Save UI state machine | `PrStage1SaveUiDirect::Tick19148` carries `80018FB0 -> 800185D0 -> 80019458` shaped state/actions | `80035510`, event draw/reset/end-frame, card wait/event/card ops | `80017594`, `80017B18`, card write/format retry results, lower card event helpers |
| Save payload bank | `Sub80015CC4`, `Sub800164B4`, `Sub8001628C`, `Sub8001635C` wrappers and direct-memory carrier | replay/source payload producer and eventual card write feedback | full owner of `8008EEF8 -> 80092F5C` and when `80092F10..` is known |
| Save list/name/title | bounded `80017B08`, `80017FC4`, limited `8004800C`, byte-exact bounded `80017C08` pieces | user input and card directory data | `80017B18`, full formatter behavior, list row scan feedback |
| Lifecycle host blocks | `PrStage1LifecycleExecutorDirect::State801C81EC` | STR playback, clear-tail movie, abort modal, Bootstrap15590 loader pump, SaveUi19148 block | thinner feedback wiring; no new state in scene shell |
| Bootstrap15590 loader | `PrStage1LoaderDirect::RunnerState` and producer steps | record dispatch, CD payload, TIM upload, SPU/VAB/SFX, PadCom, callback | live record/CD/GPU/SPU producers must feed typed feedback in PSX order |
| Loader memory | `PrStage1LoaderMemoryDirectState` | none, unless caller lacks record data | keep as direct memory, not host allocation |
| Loader CD/GPU/SPU | action payloads and typed feedback contracts | real file sector bytes, image upload/DMA, SPU/VAB/SFX lower result | preserve HAL boundaries and do not synthesize success |

## Next IDA export batch

### Save UI direct-memory helpers

Export these first because they decide which current `HelperGap` actions can
become direct-memory actions without touching host filesystem/card APIs:

| Address | Why export now | Expected boundary |
|---|---|---|
| `80017594` | Save UI polls I/O/event result before `80019458` advances state; currently helper-gapped in `Tick19148`. | Direct control + event/card feedback input, not Win inference |
| `80017B18` | Directory/list row materializer after `80017B08`; current list enumeration still carries gap evidence. | Direct memory over `dirBank/listRows`; card directory data remains feedback |
| `800180D8` | state-to-event mapper; current implementation still emits a gap when state mapping is incomplete. | Direct control |
| `800181D0` | list input branch for Cross/Circle/Up/Down and selected row. | Direct control over list row state |
| `80017FC4` | name preview/glyph expansion is mostly direct, but keep export for byte-exact glyph table and failure cases. | Direct memory |
| `8004800C` | Save UI-only formatter subset is present; full formatter flags/width/precision/specifier behavior is not. | Direct memory formatter; no host `snprintf` |
| `80048878` / `800488E4` | overlap-safe copy and string helper neighbors used by formatter/list/title paths. | Direct memory |
| `80025C44` / `80025C64` | clear/copy helpers are already partly direct, but need exact dependency notes for state=15 payload/header path. | Direct memory |

### Save card/event HAL helpers

Export these as one card-event feedback unit. They should not be translated into
host filesystem calls inside `PrStage1SaveUiDirect`; direct should emit ordered
actions and consume exact feedback.

| Address | Why export now | Expected boundary |
|---|---|---|
| `80017A10` | 4-attempt save-block write retry skeleton exists; result mapping is still a terminal helper gap. | Card HAL feedback loop |
| `80017B60` | 3-attempt card format skeleton exists; lower drain/poll helpers still gapped. | Card HAL feedback loop |
| `80017900` | card directory scan before write/overwrite decisions. | Card HAL feedback -> direct list memory |
| `80017454` | submit write/open/write/close wrapper. | Card HAL boundary |
| `80016FC0` | clear software card events; currently action + helper gap. | Card event HAL |
| `80016EB8` | poll software card events with 300-count bound. | Card event HAL |
| `8001707C` | drain hardware card events during format. | Card event HAL |
| `80017008` | poll hardware card events during format. | Card event HAL |
| `80016E18` / `80047EE4` | lower event/card queue neighbors named by previous TODOs. | Card event HAL / direct queue metadata |

### Save payload producer helpers

The public wrappers already exist, but the next export should close the source
owner chain so the `payloadKnown/helperGap` flags are not guessed.

| Address | Why export now | Expected boundary |
|---|---|---|
| `80015CC4` | initializes the save payload bank and calls the stage payload update path. | Direct memory |
| `800164B4` | loads/copies save payload from a source address. | Direct memory, source must be known |
| `8001635C` | updates payload and mirrors `0x8008EEF8 -> 0x80092F5C`-style data. | Direct memory with explicit source owner |
| `8001628C` | unlock/progress helper used by lifecycle and save payload. | Direct memory / save-state authority |
| owner of `byte_8008EEF8` / `dword_800901BC` | required before `8001635C` can be considered authoritative. | Direct producer, not replay fallback |

### Lifecycle host-block helpers

These are not all new functions to translate; they are the next graph-export
targets so the executor can stay thin and scene shell feedback can be limited
to platform facts.

| Address / unit | Why export now | Expected boundary |
|---|---|---|
| `801C81EC` host-block flow around `Bootstrap15590` and `SaveUi19148` | confirm ordering between pending actions, blocked actions, deferred scene result, and save start deferral. | Direct lifecycle state; host only returns feedback |
| `801C7A60` result path into clear-tail/save/status | ensure `SaveStatus1635C`, `UnlockNextStage1628C`, clear-tail, and abort paths do not double-own scene result. | Direct lifecycle actions + host movie/modal feedback |
| `800166AC` / `8001670C` / `80016758` | status/progress inputs already affect setup/restart and clear-tail decisions. | Direct status query; payload unknown stays unknown |
| `80026B94` event result contract as consumed by abort query | executor should consume result, not re-run event4 logic. | Host modal feedback boundary |
| `800201AC` / `80020110` transition actions | already carried as direct transition payloads; verify callback/delay ordering against blocked host actions. | Direct transition action + platform draw/audio HAL |

### Bootstrap15590 loader feedback helpers

The loader action skeleton is already shaped. The next export batch should
target the helpers needed to turn waiting steps into typed feedback without
adding logic to `PrStage1LifecycleExecutorDirect`.

| Address | Why export now | Expected boundary |
|---|---|---|
| `8001A2B0` + `800381F8` | `OpenFile1A324` needs CD lookup feedback and `CdlFILE.pos/size` ownership. | CD HAL lookup feedback |
| `8001A324` | descriptor loaded flag, payload words, sector fields, and re-entry guard. | Direct control + CD lookup feedback |
| `8001A8F0` record parser | dispatches record type 1/2/3 and selects memory/CD/GPU/SPU lanes. | Direct parser |
| `8001A89C` / `80036974` / `800367A4` | seek/Msf conversion/sync wrapper. | CD HAL feedback |
| `8001A818` / `80038FC0` / `800390C8` / `800364F0` | payload read start/sync/final result. | CD payload HAL feedback |
| `80025B28`, `80025A70`, `80025AF8`, `80025BFC`, `80025BBC` | loader heap/stack allocation, find, split, free. | Loader memory direct |
| `8001AE7C` / `80040EAC` / `80044D64` / `80044B3C` | TIM info parse and image upload/draw sync order. | Direct TIM parse + GPU HAL upload/sync |
| `800468E0` / `80046FFC` | lower GPU/DMA scheduling and draw sync completion. | GPU/DMA HAL |
| `80027120`, `80027078`, `800270D4`, `800270FC` | VAB close/open/transfer/enable and global current VAB state. | SPU HAL lower result feedback |
| `80026E4C` PadStartCom | audio reset contract is locally handled for known case; export for full init sequence and return semantics. | PadCom HAL |
| `80026FA4`, `80026EF8`, `80026ECC` | retry SFX reset/cue/flush during loader retry/audio path. | SPU/SFX HAL lower result feedback |

## Graphs to produce before code changes

1. Save UI state graph:
   `80018FB0 -> 800185D0 -> 80019458`, with each state edge annotated as
   direct memory, card/event HAL, draw/vblank HAL, or helper gap.
2. Save write/format graph:
   `80019458(state=15) -> 8004800C/title -> 80017C08/header/icon ->
   80025C64/payload copy -> 80017A10(card write retry)` and
   `state=14 -> 80017B60(format retry)`.
3. Save payload owner graph:
   `80015CC4/800164B4/8001635C/8001628C -> 80092F10 payload bank`, including
   the owner of `8008EEF8` and replay mirror source.
4. Lifecycle host-block graph:
   `801C81EC actions -> executor host blocks -> feedback -> deferred scene
   result`, covering Movie1, ClearTailMovie, AbortPoll, Bootstrap15590,
   SaveUi19148.
5. Loader feedback graph:
   `80015590 -> 8001AC18 -> 8001A8F0` split into Record, CD, Memory, GPU, SPU,
   PadCom, Callback, Completion producer steps.

## Direct/HAL rules for the next implementation window

- Save UI direct may mutate PSX-shaped memory and emit ordered actions. It must
  not call Win filesystem, `PrCard`, host save metadata, or infer save success
  from host UI state.
- Card helpers must become a feedback seam: direct emits
  open/check/write/close/format/poll actions, host returns exact PSX-like result
  fields.
- `80035510` remains a pad exact-mask/de-dupe adapter. Do not invent input
  policy inside `PrStage1SaveUiDirect`.
- Loader memory stays direct-owned. Do not replace `80025B28/25A70/25AF8/25BFC/25BBC`
  with host allocation.
- CD payload bytes must flow as:
  `CD lookup/read HAL -> CdSeamResult/CdPayloadLiveInput -> resolved payload ->
  TIM/SPU feedback`, not as a direct runner file read.
- GPU upload remains a HAL boundary. Direct may parse TIM and order
  `LoadImage/DrawSync` actions; D3D/VRAM upload belongs to the host side.
- SPU/VAB/SFX remains a HAL boundary. Direct may update PSX globals when lower
  results are supplied; it must not synthesize successful lower calls.
- Lifecycle executor should get thinner, not smarter. If it needs to branch on
  resource state, the boundary is wrong; export a larger direct unit or add a
  typed feedback field.

## Proposed cutover order after exports

1. Close Save UI pure direct-memory helpers:
   `80017B18`, `800180D8`, `800181D0`, full `8004800C` where needed, and any
   remaining bounded `80017C08/80025C64/80025C44` evidence.
2. Add a Save Card feedback carrier:
   one typed result structure for `80017A10/80017B60` lower actions, carrying
   open/check/write/close/format/poll results in action order.
3. Close Save Payload owner chain:
   make `80015CC4/800164B4/8001635C/8001628C` report known/unknown solely from
   PSX source ownership, not replay fallback.
4. Extend loader feedback graph only through typed producer inputs:
   record dispatch first, then CD payload, then TIM/GPU, then SPU/VAB/SFX.
5. Thin lifecycle host blocks:
   scene shell should only start host work and return feedback. It should not
   interpret save result, loader completion, payload readiness, or card retry
   branches.

## Stop rules

- If a missing value is a card/CD/GPU/SPU lower result, stop at a HAL feedback
  carrier. Do not fill it with a host success default.
- If a Save UI helper is still unknown, keep `HelperGap`; do not relabel it as
  direct because current Win behavior happens to work.
- If a loader producer step cannot build complete typed feedback, leave
  `externalProducerRequired` or `helperGap` visible.
- If closing a lifecycle branch requires scene-shell state or new `if` logic in
  an adapter, expand the direct graph instead of patching the shell.

## Static conclusion

The next useful P1-D work is export-first, not adapter-first. Save UI needs a
clean split between direct memory helpers and card/event HAL feedback. Lifecycle
already has the correct host-block shape, so it should only receive typed
feedback. Bootstrap15590 loader already has the runner and producer-step
skeleton; the next work is to feed record/CD/GPU/SPU results through those typed
seams without moving platform side effects into direct code.
