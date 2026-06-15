# 80019414 feedback wiring minimal path

Scope: static-only整理 for `80015788 -> 80019414(&byte_80092F10)` feedback
wiring. Authority is IDA pseudocode/assembly plus current Win direct carrier
code. Memory replay is not an implementation source. No build/test and no
`src` edits in this pass.

## 1. Current Win code path

### Direct helper contract

Current direct helper inputs live in
`src/pr/pr_scene_entry_direct.h`:

- `Call80019414Feedback80015788` carries the external feedback needed by the
  pure `80019414` carrier:
  - `gp720Known/gp720`
  - `call80019284ResultKnown/call80019284Result`
  - `call80019284HostArgPtr`
  - plus `800191E4` state-machine feedback fields:
    `sub80026784Result`, `gp716AfterStateMachine`, `wordA1Plus44`
- `Call80019284InputMemory80015788` carries the completed input memory for
  `80019284`:
  - `a1Memory[0x130C + 0x180]`
  - `initialTableMemory[308]`
  - `missingStatusPrefix/missingBankSlot`
- `Case17Feedback80019D7C` is only row-level card feedback:
  row enabled, `80016EB8` result/read success, row metadata, payload pointer.

Current implementation behavior:

- `PsxCall80019414_HiScoreEntry80015788(a1, a1Known, feedback)` first runs the
  `800191E4(a1,3)` carrier, then gates only on `feedback.gp720`. If `gp720 != 1`
  it returns known `0`; if `gp720 == 1`, it requires
  `feedback.call80019284ResultKnown` and returns that token. It also forwards
  `feedback.call80019284HostArgPtr` as `event6HostArgPtr`.
- `PsxBuild80019284InputMemoryFromStatusAndBank80015788(statusPrefix, bank)`
  is already the right pure helper for constructing `80019284` input memory:
  prefix `byte_80092F10[0..4875]` plus high-score bank suffix
  `0x8009421C[0..383]`, with static `dword_80049278` initial table bytes.
- `PsxCall80019D7C_Case17HiScoreBankCarrier(a2, feedback)` is already the
  deterministic direct carrier for `case17`: it clears the bank, merges only
  rows whose card-read feedback says the PSX read succeeded, writes `gp720=1`,
  and returns `23`.

### Executor behavior

Current executor path is intentionally not wired to a real producer yet:

- `RunGenericSwitch80015788(...)` is called from `pr_main.cpp` when the main
  scene loop hits a pending generic switch.
- `StepGenericSwitch80015788` emits `Call80019414` when event `3` returns `1`.
- `ExecuteGenericSwitchTrace80015788` handles `Call80019414` by constructing an
  empty `Call80019414Feedback80015788 callFeedback{}` and calling
  `PsxCall80019414_HiScoreEntry80015788(...)`.
- Because the feedback is empty, `gp720Known == false`; the direct carrier
  returns `missingGp720`, executor logs the direct gap, returns `DirectGap`,
  resets the current `80015788` carrier, and does not fake `ev=6`.
- `pendingEvent6ArgToken/pendingEvent6ArgPtr` already exists. If a future
  `80019414` call returns a nonzero token, executor stores `direct.result` as
  `pendingEvent6ArgToken` and `direct.event6HostArgPtr` as
  `pendingEvent6ArgPtr`. When starting `ev=6`, the token must match and the ptr
  must be non-null, otherwise the start is rejected.

Conclusion: current code has the event-6 arg channel but no producer for
`gp720`, `call80019284Result`, or a stable `hostArgPtr`. That is correct for
now; it avoids the old fake `call19414Result = 1` behavior.

## 2. Where to produce complete `a1Memory` and `initialTableMemory`

IDA authority:

```c
// 80015788
pr = sub_80019414(&byte_80092F10);
if (pr)
  sub_80026B94(6, pr);

// 80019414
sub_800191E4(a1, 3);
if (*(gp + 720) == 1)
  return sub_80019284(a1);
return 0;
```

`a1` is therefore `&byte_80092F10`. `80019284` reads:

```text
a1 + 0x0000..0x130B  save/status prefix, 4876 bytes
a1 + 0x130C..0x148B  high-score bank visible/physical rows, 384 bytes
```

The Win minimal direct wiring should produce the full input in this order:

1. Direct save/status producer provides `byte_80092F10[0..4875]`.
2. `80019D7C case17` direct carrier produces `HiScoreBankCarrier800164F8`.
3. `PsxBuild80019284InputMemoryFromStatusAndBank80015788(prefix, bank)` builds
   full `a1Memory` and supplies the static IDB `dword_80049278` initial table.
4. `PsxCall80019284_BuildHiScoreRecords80015788(...)` consumes that memory and
   returns the PSX token `0x80049278`.
5. A thin host arg adapter may expose the resulting table bytes to Win `ev=6`,
   but that adapter must be outside the pure direct core and must own stable
   storage for the pointer lifetime.

Layering recommendation:

- Pure memory construction remains in `pr_scene_entry_direct.*` via existing
  helpers. This layer must not call Win UI, `PrCard`, replay capture, or
  dispatcher APIs.
- A future narrow feedback builder, probably in the executor/host-adapter side
  or a small adjacent scene-entry feedback module, should assemble
  `Call80019414Feedback80015788` from typed direct facts:
  `case17.gp720`, `80019284.result`, and a stable host arg pointer.
- Executor should only consume the completed feedback. It should not invent
  `a1Memory`, read `PrCard::Payload()` directly, or default the token/ptr.

## 3. `80019D7C case17`: gap vs carrier

Before the real card-read feedback producer is connected, these must stay gap:

- Per-row read success. Success must come from the `800179B4 -> 80016EB8`
  feedback path, specifically event result `1`. Win file availability,
  `PrCard::Payload()`, or replay bytes cannot assert success.
- Per-row payload bytes. The `payload` passed to `800164F8` is valid only after
  the PSX card read for that row succeeded.
- `rowEnabledKnown` if the producer has not modeled the `unk_8007A590`
  row-enable flag.
- `rowMetadata8007AE14`, because it is copied from the loaded block after a
  successful read.
- BIOS/card HAL details: `bu%1d%1d:%s`, `open/read/close`, `gp+124/gp+128`,
  event handles, timeout/yield behavior.

These can be carried forward by direct memory carrier:

- `a2 == 3 -> result 5` early return.
- `800168DC` bank clear semantics.
- If `word_8007ABE4` is known false: no row reads are attempted, but `gp720=1`
  and `result=23` are still deterministic after the clear path.
- If a row has explicit PSX read-success feedback and payload bytes from that
  feedback: `800164F8(payload)` merge and `80016000(base,4)` sort can be
  carried directly.
- Final `gp720=1` and `result=23` for the non-early case are deterministic
  after the case body, but using them to drive `80019414 -> 80019284` only
  becomes useful when the bank content is also known enough to build
  `a1Memory`.

Important distinction: `gp720=1` alone is not enough to close `80019414`.
It only authorizes the `80019284(a1)` call. The event-6 token/arg still needs a
known `80019284` result and a stable host representation of `dword_80049278`.

## 4. File-level implementation suggestion

Do not mix these into direct core:

- `PrCard::Payload()` as direct `a1Memory`.
- Replay-captured high-score rows or replay output table bytes.
- Win UI fallback table formatting.
- Default nonzero `call19414Result`.
- C++ zero-init as a substitute for the IDB `dword_80049278` initial table.

Suggested file responsibilities:

- `src/pr/pr_scene_entry_direct.*`
  - Keep pure PSX carriers only:
    `PsxCall80019D7C_Case17HiScoreBankCarrier`,
    `PsxBuild80019284InputMemoryFromStatusAndBank80015788`,
    `PsxCall80019284_BuildHiScoreRecords80015788`,
    `PsxCall80019414_HiScoreEntry80015788`.
  - Do not add Win UI, card file IO, dispatcher, or fallback formatting here.

- `src/pr/pr_scene_entry_executor_direct.cpp`
  - Remain a consumer of completed feedback.
  - Keep `DirectGap` behavior until a real feedback producer exists.
  - When connecting later, only pass a completed `Call80019414Feedback80015788`
    into the existing call site; do not synthesize missing fields.

- Future narrow adapter module, if needed
  - Own stable host storage for event-6 arg table bytes.
  - Convert `Call80019284Result80015788.tableBytes` into whatever `ev=6` needs
    on the Win side.
  - Produce `call80019284HostArgPtr` with lifetime at least until `ev=6`
    dispatcher completion.
  - Keep source provenance fields so gaps remain visible when card read
    feedback or status prefix is missing.

- Card HAL producer, future work
  - Produce row-level `Case17Feedback80019D7C` from the real PSX-equivalent
    card-read seam.
  - The accepted payload bytes enter direct memory carrier only after that
    feedback says `80016EB8 == 1`.

## 5. Should this pass connect executor?

No. This pass should not connect executor beyond the already-present helper
channel.

Reason:

- Executor currently has no real producer for `gp720`, full `a1Memory`,
  `initialTableMemory` consumption result, or stable `hostArgPtr`.
- Connecting now would force one of the forbidden shortcuts:
  `PrCard::Payload()`, replay values, Win UI fallback, or a fake token.
- The safe current behavior is to keep `Call80019414` as `DirectGap` when
  feedback is absent, while preserving the helper contracts and event-6 token
  channel for the later producer.

Minimum next implementation step, when code edits are allowed:

1. Add a narrow feedback builder that receives direct save/status prefix plus
   `Case17Result80019D7C.bank`.
2. Build `Call80019284InputMemory80015788`.
3. Run `PsxCall80019284_BuildHiScoreRecords80015788`.
4. Fill `Call80019414Feedback80015788` with:
   `gp720Known=true`, `gp720=case17.gp720`,
   `call80019284ResultKnown=true`, `call80019284Result=0x80049278`,
   and a stable `call80019284HostArgPtr`.
5. Only then pass that feedback to executor's existing `Call80019414` site.

