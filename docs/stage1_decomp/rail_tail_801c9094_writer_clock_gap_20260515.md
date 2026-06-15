# rail-tail `801C9094` writer / clock gap facts (2026-05-15)

Scope: Stage1 rail-tail stream4 compact producer only. Do not use this file as
authority for RGB, lower-CD, case17, SaveUi, MenuHelp/StageClear, boot seam, or
renderer behavior.

## Runtime status

- Full build passed after the latest C++ edits. Output:
  `E:\game\PSgame\parappa the rapper\PaRappaWin.exe`.
- Latest rail-tail runtime still fails:
  `python .\test_dispatcher.py --stage1-rail-tail-runtime --stage1-runtime-retries 1`
  with summary:
  `artifacts\stage1_rail_probe_runtime\20260515_093026_stage1_rail_tail_runtime\summary.txt`.
- The failure moved from "no compact event reaches the producer" to a clock
  alignment gap. Win now consumes flag-stream compact events, but consumes
  `0x801D0B38` too early, so teacher row0 has already reset before the expected
  tail marker window.

## Win observable facts

From the latest summary window:

- `q1182/script7023`: `stream=1 row=4`, compact source `0x801D0B38`,
  `sourceFrame=1155`, record `11`, teacher cursor `-1`.
- `q1188/script7059`: `event801C9094.flag.valid=1`,
  `index=79 due=7056 psx=0x801D0BA4`; compact source `0x801D0BA4`,
  `studentCursor9E=1`, `studentStateA2=1`, teacher still `-1`.
- `q1221`: `event801C9094.flag.valid=1`,
  `index=82 due=7248 psx=0x801D0C10`; compact source `0x801D0C10`,
  teacher cursor `1`, then advances through `4/7`.

## PSX recorder facts

Recorder:
`E:\game\PSgame\parappa the rapper\duckstation\bin\x64\标题画面-menu-选关选stage1-good通关-保存记忆卡（过场动画均为跳过）.psxrec`

Writer window:

- `0x801C36CC`:
  - frame `3507`, PC `0x801C933C`, `0x0004 -> 0x0005`
  - frame `3515`, PC `0x801C933C`, `0x0005 -> 0x0006`
  - frame `3523`, PC `0x801C933C`, `0x0006 -> 0x0007`
  - frame `3531`, PC `0x801C933C`, `0x0007 -> 0x0008`
  - frame `3537`, PC `0x801C933C`, `0x0008 -> 0x0009`
  - frame `3547`, PC `0x801C9334`, `0x0009 -> 0xFFFF`
- `0x801C36DE`:
  - frame `3537`, PC `0x801C9CC4`, `0xFFFF -> 0`
  - frame `3537`, PC `0x801C9E70`, `0 -> 1`
  - frame `3547`, PC `0x801C947C`, `1 -> 2`
  - frame `3555`, PC `0x801C947C`, `2 -> 3`
  - frame `3563`, PC `0x801C947C`, `3 -> 4`

Event-stream globals in the same window:

- `0x8008ED2E == 1`
- `0x8008ED2C == 0`
- No observed writer/change to those two addresses in the checked
  `3300..3570` range.

`ctx+0x0C` values:

- frame `3531`: `3581`
- frame `3537`: `3600`
- frame `3547`: `3628`

## IDA export

Authoritative narrow export:
`docs/stage1_decomp/ida_export_rail_tail_compact_writers_801c9334_801c933c_801c947c_801c9cc4_801c9e70_20260515.txt`

The export confirms all writer PCs belong to function `0x801C9094`
(`PrStageRunner_FrameUpdate`, end `0x801CA0E0`):

- `801C933C/801C9334`: teacher row0 steady tick/reset writes `ctx+0x8C`.
- `801C9CC4/801C9E70/801C947C`: flag-stream compact/student path writes
  `ctx+0x9E`.
- Caller xref includes `801C7F50`.

## Code hookup

The clock authority for the steady compact tick is now explicit:

- PSX `80024FD0` writes `ctx+0x00 |= 8` only when
  `(dword_8008ECF4 & 1) == 0`; on an 8-bucket boundary it writes `| 0xC`.
- The Win direct scorer host now carries this as
  `Stage1BucketCadenceRuntime::ctxFlagTickAdvanceKnown801C9094 /
  ctxFlagTickAdvance801C9094`, derived from the direct `80024FD0` bucket-change
  path, not from renderer, selectedRow, marker, or previous-frame cursor state.
- `BuildStage1RuntimeSlotsFollowUpFacts801CBFDC(...)` passes the fact into
  `PrStage1RuntimeSlotsDirectAdvanceFromHostFacts801C9094(...)`.
- `Psx801C9094RunCompactProducer(...)` still applies compact reset,
  teacher-bootstrap, and student-catchup events from overlay event facts, but
  teacher/student steady cursor advance now requires the explicit bit8 fact.
  Missing facts therefore leave a gap instead of authorizing per-frame success.
- The compact producer `sourceFrame` observable now records the same
  `ctx+0x0C` / script-frame source used by `801C9094` event due checks and
  `801C858C -> dword_801D3048` reset, not the Win query/apply frame. This
  keeps the remaining `0x801D0B38` timing investigation on PSX clock facts.

## Next evidence window

Do not add renderer, selectedRow, marker, previous-frame, or synthetic cursor
logic. The next window is only:

1. Continue integration validation from the passing rail-tail runtime artifact:
   `artifacts\stage1_rail_probe_runtime\20260515_101704_stage1_rail_tail_runtime\summary.txt`.
2. If a later visual/runtime failure appears, compare the captured
   `ctxFlagTickAdvanceKnown801C9094/ctxFlagTickAdvance801C9094`,
   `compactProducer801C9094.sourceFrame`, and `sourceEventPsxAddr` against the
   PSX writer clock facts before changing code.
3. Patch only after any remaining mismatch is tied back to IDA/control-flow
   evidence.

## 2026-05-15 validation

- `git diff --check` passed with only existing LF/CRLF warnings.
- Full `.\build.ps1` passed, output
  `E:\game\PSgame\parappa the rapper\PaRappaWin.exe`.
- `python .\test_dispatcher.py --stage1-rail-tail-runtime
  --stage1-runtime-retries 1` passed. Markers:
  `tail_teacher8_student_hidden`, `tail_teacher9_student1`,
  `tail_teacher_hidden_student2`.
- Passing samples expose the direct facts path:
  `q=1184 script=7035 bitK=1 bit=1 cpAddr=0x801D0B38 cpSrc=6866
  teacher=8 student=-1`,
  `q=1190 script=7070 bitK=1 bit=0 cpAddr=0x801D0BA4 cpSrc=7059
  teacher=-1 student=1`,
  `q=1193 script=7089 bitK=1 bit=0 cpAddr=0x801D0BA4 cpSrc=7059
  teacher=-1 student=2`.
