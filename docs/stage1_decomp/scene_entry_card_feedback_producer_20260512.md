# 800179B4 card-read feedback producer wiring

Scope: static-only整理 for `800179B4 -> 800173A8/80016EB8` feeding
`80019D7C case17` and then `80019414`. Authority is IDA Hex-Rays / assembly
and current direct-code field contracts. No memory replay, `PrCard`, Win save
payload, or host file availability may authorize PSX success.

## 1. PSX success condition and payload lifetime

IDA re-check:

```c
// 800179B4(name, blockBuffer, blocks)
sub_800173A8(*(gp + 128), *(gp + 124), name, blockBuffer, blocks);
int ev = sub_80016EB8();
close(*(gp + 696));
return -(ev != 1);
```

```c
// 800173A8(port, slot, name, blockBuffer, blocks)
sprintf(path, "bu%1d%1d:%s", port, slot, name);
fd = open(path, 0x8001);
if (fd == -1) {
  close(-1);
  return -1;
}
*(gp + 696) = fd;
sub_80016FC0();
read(fd, blockBuffer, blocks << 13);
return 0;
```

```c
// 80016EB8()
for (i = 300; i > 0; --i) {
  if (TestEvent(*(gp + 664)) == 1) return 1;
  if (TestEvent(*(gp + 668)) == 1) return 2;
  if (TestEvent(*(gp + 672)) == 1) return 3;
  if (TestEvent(*(gp + 676)) == 1) return 4;
  sub_80035560(0);
}
return 2;
```

Success is therefore exactly `80016EB8() == 1`, which makes `800179B4`
return `0`. Any other event code or timeout makes it return `-1`. The immediate
`800173A8` open/read return is not used by `800179B4` after the call; it only
affects whether `gp+696` and `read()` were issued internally.

For `80019D7C case17`, IDA shows this payload lifetime:

1. `sub_80025C44(byte_8007ABE8, 0x2000)` clears the full card block buffer
   every row iteration.
2. If `row+0x68 != 0`, `strcpy(byte_8007CBE8, row)` builds the BIOS filename.
3. `sub_800179B4(byte_8007CBE8, byte_8007ABE8, 1)` attempts one 8192-byte
   block read.
4. Only when that call returns `>= 0`, PSX reads `dword_8007AE14` from inside
   the loaded block and calls `sub_800164F8(byte_8007ADE8)`.
5. `byte_8007ADE8 == byte_8007ABE8 + 0x200`, so the high-score merge payload
   is the save payload area after the card header/icon area.

Payload bytes are valid for direct high-score merging only in the row where
`80016EB8 == 1`. A producer must not pass payload bytes for disabled rows,
unknown rows, failed event rows, or rows whose success is inferred from Win file
state.

## 2. Current carrier fields and gaps

Current direct carrier already has the minimum deterministic memory side:

- `Case17Feedback80019D7C.word8007ABE4Known/word8007ABE4`: gates whether the
  row loop runs.
- `Case17CardRow80019D7C.rowEnabledKnown/rowEnabled`: mirrors `row+0x68`.
- `Case17CardRow80019D7C.eventResult80016EB8Known/eventResult80016EB8`: the
  authoritative success source.
- `Case17CardRow80019D7C.readResultKnown/readSucceeded`: weaker fallback shape;
  should be secondary and should not override a known event result.
- `Case17CardRow80019D7C.rowMetadata8007AE14Known/rowMetadata8007AE14`: the
  post-read metadata copied into `row+0x6A`.
- `Case17CardRow80019D7C.payload/payloadSize`: the bytes passed to
  `800164F8(byte_8007ADE8)` after success.
- `PsxCall80019D7C_Case17HiScoreBankCarrier`: clears the bank with `800168DC`,
  merges successful rows through `800164F8`, sorts through `80016000`, writes
  `gp720=1`, and returns `23`.

Remaining gaps:

- There is no producer that fills `Case17Feedback80019D7C` from the actual
  `800179B4 -> 800173A8/80016EB8` lower card sequence.
- There is no row-level trace for `800173A8` path construction/open/read/close
  yet: `gp+128`, `gp+124`, `"bu%1d%1d:%s"`, fd at `gp+696`, `blocks << 13`.
- There is no explicit `80016FC0` clear-event acknowledgement attached to the
  read attempt.
- `PrCard::Payload()`, `PrCard::PayloadConst()`, `PrCard::LoadFromMemCardEntry`,
  and `PrEvent` memcard confirm flow exist in Win code, but they are not PSX
  feedback producers for this seam. They operate at a higher Win UI/save layer
  and cannot authorize `80016EB8 == 1`.
- SaveUi direct has card-HAL action/feedback shapes for write/format/load
  helpers, including `pollResult80016EB8`, but it is scoped to
  `80017A10/80017B60/80017594`; it is only a pattern for the future read
  producer, not a reusable producer for `800179B4`.

## 3. Minimal producer / adapter write range

Add a narrow producer outside `pr_scene_entry_direct.*`. The pure direct file
should keep only PSX memory transforms; the producer may live beside the
executor or in a small scene-entry/card feedback adapter.

Minimum producer input facts per candidate row:

- row index `0..14`.
- row enabled flag from `unk_8007A590 + row*0x6C + 0x68`.
- row name copied to `byte_8007CBE8` before `800179B4`.
- card selector values loaded from `gp+128` and `gp+124`.
- `800173A8` action trace: built path, open attempted, fd if known, read target
  `byte_8007ABE8`, block count `1`, byte count `0x2000`, close fd.
- `80016FC0` clear-event step known/unknown.
- `80016EB8` event code if known.
- full read buffer bytes `byte_8007ABE8[0..0x1FFF]` only if the read event code
  is known and equals `1`.

Minimum write into existing carrier:

```text
Case17Feedback80019D7C.word8007ABE4Known
Case17Feedback80019D7C.word8007ABE4
Case17Feedback80019D7C.cardRows[i].rowEnabledKnown
Case17Feedback80019D7C.cardRows[i].rowEnabled
Case17Feedback80019D7C.cardRows[i].eventResult80016EB8Known
Case17Feedback80019D7C.cardRows[i].eventResult80016EB8
Case17Feedback80019D7C.cardRows[i].readResultKnown
Case17Feedback80019D7C.cardRows[i].readSucceeded
Case17Feedback80019D7C.cardRows[i].rowMetadata8007AE14Known
Case17Feedback80019D7C.cardRows[i].rowMetadata8007AE14
Case17Feedback80019D7C.cardRows[i].payload
Case17Feedback80019D7C.cardRows[i].payloadSize
```

Write rules:

- If row is disabled, only set row enable facts. Do not set read success or
  payload.
- If row is enabled but `80016EB8` is unknown, leave success unknown and keep
  payload null.
- If `80016EB8 != 1`, set the event result and failed read result, but keep
  payload null and metadata unknown.
- If `80016EB8 == 1`, set read success, expose payload as
  `byte_8007ABE8 + 0x200` with size at least the `800164F8` required range, and
  copy metadata from `byte_8007ABE8 + 0x22C`.
- The adapter may own a stable 0x2000-byte row buffer copy so the pointer stays
  valid through the direct call. It must not point directly at transient local
  stack storage.

Recommended trace-only additions for a later code pass:

- Add a `CardReadAttempt800179B4` or equivalent nested carrier with
  `function800179B4`, `function800173A8`, `function80016FC0`,
  `function80016EB8`, `functionClose`, path args, target buffer identity, block
  count, byte count, event result, and close target.
- Keep that trace adjacent to producer/adapter state. Do not push BIOS/file I/O
  implementation into `pr_scene_entry_direct.*`.

Forbidden shortcuts:

- `PrCard::Payload()` as payload source without a matching `80016EB8 == 1`
  feedback record.
- Win memcard directory entry availability as read success.
- Replay-sampled save bytes as implementation truth.
- Defaulting missing rows to success, or defaulting `eventResult80016EB8` to 1.
- Filling `Call80019414Feedback80015788` before `case17` bank and
  `80019284` output are both known.

## 4. When executor / 80019414 can connect

Current executor behavior should stay on the default no-completed-feedback path
until the producer exists:

- The four-argument `RunGenericSwitch80015788(..., completedCall19414)` and
  `ExecuteMainSceneSwitchTrace80015D18(..., completedCall19414)` paths exist.
- The default caller still passes no completed `80019414` feedback.
- `PsxCall80019414_HiScoreEntry80015788` then reports missing `gp720` /
  missing `80019284` result and executor returns `DirectGap`.
- This is correct because no real card-read feedback producer exists.

Executor can connect only after this full chain is available:

1. Card feedback producer fills `Case17Feedback80019D7C` from
   `800179B4/800173A8/80016EB8` facts.
2. `PsxCall80019D7C_Case17HiScoreBankCarrier(a2, feedback)` produces a known
   `Case17Result80019D7C`, including `gp720=1` and the high-score bank.
3. A save/status prefix producer supplies `byte_80092F10[0..4875]`.
4. `PsxBuild80019284InputMemoryFromStatusAndBank80015788(prefix, bank)` builds
   full `a1Memory` and static `dword_80049278` initial table memory.
5. `PsxCall80019284_BuildHiScoreRecords80015788(...)` returns the real
   `0x80049278` token/table result.
6. A thin host arg adapter owns stable event-6 table storage and fills
   `Call80019414Feedback80015788`:
   `gp720Known=true`, `gp720=1`, `call80019284ResultKnown=true`,
   `call80019284Result=0x80049278`, `call80019284HostArgPtr=<stable table>`.

Only then should `ExecuteGenericSwitchTrace80015788` pass non-empty
`Call80019414Feedback80015788` to `PsxCall80019414_HiScoreEntry80015788`.
Until then, `DirectGap` is the correct boundary.

## 5. 2026-05-12 carrier code note

Implemented the first narrow carrier in
`src/pr/pr_scene_entry_card_feedback_direct.*`. It accepts typed per-row
`800179B4/800173A8/80016FC0/read/80016EB8/close` facts and reports explicit
row gaps for missing row status, card selectors, path/open fd, target buffer,
block/byte count, clear-event, read submission, event result, close fd, and
block bytes.

Important guard: the builder does not expose `eventResult80016EB8 == 1`,
`readSucceeded`, metadata, or payload to `Case17Feedback80019D7C` unless the
full HAL read shape is known. This avoids triggering the existing direct
case17 merge path from a typed event alone when read/close/slot/status facts are
missing. Failed non-1 event results may still be carried as failures because
they cannot synthesize success.

## 6. 2026-05-13 xref/call-chain note

`scene_entry_cardread_case17_callchain_20260513.md` records the current IDA
session evidence and C++ xref table for the existing typed chain:

`BuildCase17CardReadHalFeedbackFromSaveCardHal800179B4`
-> `BuildFeedback80019414FromCase17CardReadFacts`
-> `PsxCall80019D7C_Case17HiScoreBankCarrier`
-> `BuildFeedback80019414FromStatusPrefixAndCase17Bank`
-> `BuildCompletedCall80019414InputFromFeedbackAdapterResult80019414`
-> four-argument `RunGenericSwitch80015788` / `ExecuteMainSceneSwitchTrace80015D18`.

No additional adapter function is needed here. The remaining gap is the real
`800179B4` typed facts producer.
