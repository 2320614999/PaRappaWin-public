# 80092F10 prefix producer for 80019414 / 80019284

Scope: static-only整理 for `byte_80092F10[0..4875]`, the 4876-byte
save/status prefix consumed by `80015788 -> 80019414(&byte_80092F10)`.
No build, no runtime replay, no `src` edits. Authority is IDA Hex-Rays /
assembly from current IDB `E:\game\PS模拟器\parappa the rapper\SCUS_941.83`
plus read-only inspection of current Win/direct code.

## 1. Static call chain

Scene-entry high-score path:

```c
// 80015788
v4 = sub_80019414(&byte_80092F10);
if (v4)
  sub_80026B94(6, v4);

// 80019414
sub_800191E4(a1, 3);
if (*(gp + 720) == 1)
  return sub_80019284(a1);
return 0;

// 800191E4(a1, mode)
sub_80018FB0(a1, sub_80018E10, sub_80019D7C, 20, 3);
```

For this path, `a1 == &byte_80092F10`. `80019284(a1)` consumes:

```text
a1 + 0x0000..0x130B  byte_80092F10 save/status prefix, 4876 bytes
a1 + 0x130C..        adjacent high-score bank at 0x8009421C
```

The `800191E4(mode=3)` tick callback reaches `80019D7C case 17`, which does
not refresh the prefix. It refreshes only the high-score bank:

```c
// 80019D7C case 17
sub_800168DC();
if (word_8007ABE4) {
  for (row = 0; row < 15; ++row) {
    sub_80025C44(byte_8007ABE8, 0x2000);
    if (row_enabled && sub_800179B4(&byte_8007CBE8, byte_8007ABE8, 1) >= 0) {
      row_meta = dword_8007AE14;
      sub_800164F8(byte_8007ADE8);
    }
  }
}
*(gp + 720) = 1;
return 23;
```

So the prefix must already be carried from the save/status producer graph before
`80019414` calls `80019284`.

The broader producer graph that feeds this prefix is:

```text
main 80015D18
  -> 80015788 generic scene/menu dispatcher
  -> stage overlay clear producers
       COMOD1/2/3/5/6/7 call 8001635C(...)
       COMOD1/2/3/5/6 also call 8001628C(stage+1)
       if word_800916F0 != 1: 80015590(stage) + 80019148(&byte_80092F10)
  -> if scene0 return v2 > 0 and word_800916D0 != 1:
       80015CC4()
```

Memory-card menu paths also pass the same pointer:

```text
80015788 result 2 -> 80015700(&byte_80092F10)
                   -> 800193F4(&byte_80092F10)
                   -> 800191E4(a1, 2)

80015788 result 6 -> 800193B0(&byte_80092F10)
                   -> 800191E4(a1, 1)
```

`80019D7C case 16` is the load-state producer for the prefix:

```c
sub_800179B4(&byte_8007CBE8, byte_8007ABE8, 1);
sub_800164B4(byte_8007ADE8);
*(gp + 720) = 1;
return 23;
```

`80015700/80015744` form a backup/restore pair:

```c
sub_80015700(a1) { return sub_80025C64(a1, &unk_80079008, 4876); }
sub_80015744(a1) { return sub_80025C64(&unk_80079008, a1, 4876); }
```

When their `a1` is `&byte_80092F10`, `80015744` is also a whole-prefix writer.

## 2. Functions writing byte_80092F10[0..4875]

IDA direct xref scan for `0x80092F10..0x8009421B` found these explicit
address users relevant to the prefix:

```text
80015CC4 -> byte_80092F10
8001628C -> byte_80092F10
8001635C -> byte_80092F10
800164B4 -> byte_80092F10
800167A8 -> byte_80092F1D
800169E0 -> dword_80092F24 (read)
80019D7C -> indirect via 800164B4 in case 16
80019148/80019458/800185D0 -> indirect via a1/a3 pointer when a1 is 80092F10
80015744 -> indirect whole-prefix restore when a1 is 80092F10
```

Confirmed writers:

- `80015CC4`: clears the entire prefix and immediately seeds it.

```c
sub_80025C44(&byte_80092F10, 4876);
return sub_8001635C(1, 1, 1, 0);
```

- `800164B4(src)`: overwrites the entire prefix from a loaded save payload.

```c
return sub_80025C64(src, byte_80092F10, 4876);
```

- `8001635C(stage, status, prev, score)`: writes metadata and mirrors the
  replay payload into the prefix.

```c
idx = sub_8001615C(stage);
dword_80092F40 = prev ? prev : 1;
if (byte_80092F10[idx + 0x0D] < status)
  byte_80092F10[idx + 0x0D] = status;
*(s32 *)(byte_80092F10 + 0x14 + idx * 4) = score;
dword_80092F3C = idx;
dword_80092F48 = dword_800901BC;
sub_80025C64(byte_8008EEF8, byte_80092F5C, 4800);
*(s32 *)(byte_80092F10 + 0x34) = sub_800161F4(&byte_80092F1D);
```

Assembly check for the score store uses `idx << 2` plus `0x14`; the score bank
offset is therefore `0x14 + idx * 4`, not `0x05 + idx`.

- `8001628C(stage)`: unlocks one status byte if it is still zero.

```c
idx = sub_8001615C(stage);
if (!byte_80092F10[idx + 0x0D])
  byte_80092F10[idx + 0x0D] = 1;
```

- `800167A8(stage, mode)`: writes one status byte through `byte_80092F1D`.

```c
byte_80092F1D[sub_8001615C(stage)] = (mode == 1) ? 2 : 1;
```

- `80015744(a1)`: whole-prefix restore when called with `a1=&byte_80092F10`.

```c
sub_80025C64(&unk_80079008, a1, 4876);
```

- `80019148(&byte_80092F10) -> 800185D0/80019458`: save UI uses the prefix
  pointer as `saveBuf`. `800185D0` writes the suffix/name field at `a3+1`.
  `80019458 state 15` then writes the save block by copying that `a3` buffer:

```c
// 80019148
sub_80018FB0(a1, sub_800185D0, sub_80019458, 21, 11);

// 800185D0 examples
strcpy(a3 + 1, ...);

// 80019458 state 15
sub_80025C64(a3, byte_8007ADE8, 4876);
if (sub_80017A10(&byte_8007CBE8, byte_8007ABE8, 1) >= 0) {
  *(gp + 716) = 1;
  *(gp + 720) = 1;
  return 23;
}
```

Confirmed non-writer / reader-only for this prefix:

- `800169E0(ctx)` reads `dword_80092F24[sub_8001615C(word_800916E2)]` when
  `word_800916D0 == 2`; it writes `ctx+48` and `word_80091816`, not the prefix.
- `800166AC(stage)` reads `byte_80092F1D[sub_8001615C(stage)]`.
- `8001681C()` restores `dword_800901BC` and the replay mirror from
  `dword_80092F48/byte_80092F5C`; it reads this prefix/mirror, it does not
  produce new prefix bytes.
- `80015700(a1)` copies the prefix out to `unk_80079008`; it is a backup
  writer, not a `byte_80092F10` writer.
- `80019414` itself does not write the prefix; it delegates to
  `800191E4(a1,3)` and then conditionally calls `80019284(a1)`.

## 3. Current Win/direct carrier status

Current Win/direct has partial pieces, but no completed typed carrier suitable
for `80019414 -> 80019284` yet.

- `src/pr/pr_scene_entry_direct.*` already has the consumer-side shape:
  `Call80019284InputMemory80015788` and
  `PsxBuild80019284InputMemoryFromStatusAndBank80015788(prefix, bank)`.
  This helper requires `byte_80092F10[0..4875]` as input; it is not the
  prefix producer.
- `src/pr/pr_scene_entry_direct.*` also has `HiScoreBankCarrier800164F8` and
  `PsxCall80019D7C_Case17HiScoreBankCarrier(...)` for the adjacent
  high-score bank. That closes the `a1+4876` suffix, not the 4876-byte prefix.
- `src/pr/pr_stage1_save_ui_direct.*` contains an internal
  `SaveUiDirectMemory::savePayloadBank[4876]`, plus direct wrappers for
  `Sub80015CC4`, `Sub800164B4`, `Sub8001628C`, `Sub8001635C`, `Sub800167A8`,
  and `Sub800169E0`. Its public producer result reports `payloadKnown` and
  actions, but it does not expose a typed immutable
  `byte_80092F10[0..4875]` snapshot with producer provenance for scene-entry.
- `src/pr/pr_stage_status_bank_direct.*` emits typed direct-memory requests
  for `Call8001635C`, `Call8001628C`, `Call80015CC4`, and `Call800169E0`.
  It is a request/trace carrier; it does not own or expose the actual prefix
  bytes.
- `src/pr/pr_event.cpp` still contains Win shell/mirror paths using
  `PrCard::Payload()` (`SyncStageSelectStatusToPayload`,
  `ApplyStageSelectSavePayloadWriteback1635C`,
  `ReloadStageSelectAuthorityFromPayload164B4`,
  `CaptureStageSelectPayloadBackup15700`,
  `RestoreStageSelectPayloadBackup15744`). These are not a typed PSX direct
  carrier for `80019414`.

Conclusion: the direct consumer exists, and the high-score suffix carrier
exists, but the authoritative prefix producer is still only partially
represented inside save/status modules and shell payload mirrors. There is no
single current Win/direct API that can be passed to
`PsxBuild80019284InputMemoryFromStatusAndBank80015788(...)` as the proven
`byte_80092F10[0..4875]` source.

## 4. Minimal wiring position

The minimal safe seam is a narrow save/status-prefix producer, not executor
fallback logic.

Required future wiring:

1. Let the save/status direct owner expose an immutable typed snapshot:

```text
SaveStatusPrefix80092F10
  bytes[4876]
  known
  producer facts:
    80015CC4 clear/seed
    800164B4 whole load
    8001635C metadata + replay mirror copy
    8001628C unlock
    800167A8 status write
    80015744 restore
    80019148/800185D0/80019458 save UI pointer writes
```

2. A thin scene-entry feedback builder consumes:

```text
SaveStatusPrefix80092F10
HiScoreBankCarrier800164F8 from 80019D7C case17
static dword_80049278 initial table
```

3. The builder calls existing pure helpers:

```c++
input = PsxBuild80019284InputMemoryFromStatusAndBank80015788(prefix, bank);
records = PsxCall80019284_BuildHiScoreRecords80015788(
    0x80092F10,
    true,
    input.a1Memory,
    sizeof(input.a1Memory),
    input.initialTableMemory,
    sizeof(input.initialTableMemory));
```

4. Only after that, fill `Call80019414Feedback80015788` with:

```text
gp720Known=true
gp720=case17.gp720
call80019284ResultKnown=true
call80019284Result=0x80049278
call80019284HostArgPtr=<stable host table storage>
```

Then the existing executor `Call80019414` site may consume this completed
feedback.

## 5. Forbidden shortcuts

Do not use these as the `byte_80092F10[0..4875]` authority:

- `PrCard::Payload()` directly. It is a Win card-file payload mirror and is
  only 4876 bytes; it also bypasses PSX producer ordering and cannot supply the
  adjacent high-score suffix.
- `pr_event.cpp` stage-select status mirrors or debug unlock helpers. They are
  shell authority and cover only selected fields.
- Replay/memory sampling bytes. Replay may locate the entry or validate a
  finished translation, but it cannot authorize the implementation contents.
- A default zero buffer. `80019284` must receive whatever the current PSX
  prefix producer chain left in memory.
- Synthetic `gp720=1`, fake nonzero `80019414` result, or a fabricated
  `dword_80049278` pointer.
- Formatting the high-score UI directly in Win. `80019284` must remain the
  table builder, and renderer/UI should consume the translated table.

## 6. Open static follow-up

- `PrStage1SaveUiDirect` already contains the closest in-memory owner of the
  prefix, but its public carrier is not yet typed enough for scene-entry.
- Before code wiring, recheck the current `Sub8001635CDirect` score offset
  against PSX assembly: PSX writes score at `byte_80092F10 + 0x14 + idx*4`.
- `80019148/80019458` are pointer-based writers and should be treated as part
  of the prefix producer graph, even though direct xref scans of
  `byte_80092F10` do not show their writes.
