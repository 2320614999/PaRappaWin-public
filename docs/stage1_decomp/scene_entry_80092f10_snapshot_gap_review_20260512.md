# 80092F10 typed snapshot gap review

Scope: task B static-only review. This pass only checks current
`PrStage1SaveUiDirect` code, existing docs, and saved IDA exports for whether
the current `byte_80092F10[0..4875]` typed snapshot fully covers the
`80015744` restore path and the `80019148/800185D0/80019458` pointer-writer
facts. No `src` edit, no build, no runtime replay.

Authority: IDA Hex-Rays/assembly exports already present under
`tools/auto_port/output` and `docs/stage1_decomp`, plus read-only inspection of
`src/pr/pr_stage1_save_ui_direct.*`.

## 1. PSX facts being reviewed

`80019414(&byte_80092F10)` eventually calls `80019284(a1)` only after
`800191E4(a1, 3)` sets `gp+720 == 1`. For this path `a1` is
`0x80092F10`, so the first 4876 bytes must be the real save/status prefix left
by the prefix producer graph.

The relevant indirect writers are:

```c
// 80015744
sub_80025C64(&unk_80079008, a1, 4876);
```

When `a1 == &byte_80092F10`, this is a whole-prefix restore from the
`80015700` backup buffer.

```c
// 80019148
sub_80018FB0(a1, sub_800185D0, sub_80019458, 21, 11);
return *(gp + 716);
```

When `a1 == &byte_80092F10`, save UI passes the same prefix pointer through the
input callback and state-machine callback.

```c
// 800185D0, name/suffix writer examples
strcpy(a3 + 1, *(gp + 148));
strcpy(a3 + 1, selectedSuffix);
```

`800185D0` writes through the `a3` pointer. For the Stage1 save flow this means
`byte_80092F10 + 1` is a real pointer-write field, not a separate Win-side
string.

```c
// 80019458 state 15
sub_80025C64(a3, byte_8007ADE8, 4876);
if (sub_80017A10(&byte_8007CBE8, byte_8007ABE8, 1) < 0)
  return 2;
*(gp + 716) = 1;
*(gp + 720) = 1;
return 23;
```

`80019458` does not copy back into `byte_80092F10` in state 15; it consumes the
current `a3` prefix and writes it to the card payload buffer before submitting
the card write.

## 2. Current `PrStage1SaveUiDirect` snapshot shape

Current public carrier:

```c++
struct PrStage1SaveStatusPrefix80092F10 {
    bool known;
    bool statusBankKnown80092F1D;
    bool helperGap;
    uint32_t psxAddress;
    uint32_t byteCount;
    uint32_t lastWriterFunction;
    uint32_t lastFaultAddress;
    bool wrote80015CC4;
    bool wrote800164B4;
    bool wrote8001635C;
    bool wrote8001628C;
    bool wrote800167A8;
    bool wrote80015744;
    uint8_t bytes[4876];
};
```

Current internal backing state:

```c++
SaveUiDirectMemory::savePayloadBank[4876]
SaveUiDirectMemory::savePayloadBankKnown
SaveUiDirectMemory::statusBankKnown80092F1D
SaveUiDirectMemory::savePayloadBankLastWriterFunction
SaveUiDirectMemory::wrote80015CC4
SaveUiDirectMemory::wrote800164B4
SaveUiDirectMemory::wrote8001635C
SaveUiDirectMemory::wrote8001628C
SaveUiDirectMemory::wrote800167A8
SaveUiDirectMemory::wrote80015744
```

`GetSaveStatusPrefix80092F10()` copies the internal `savePayloadBank` to the
public snapshot only when `savePayloadBankKnown` is true. It also exposes all
writer flags verbatim.

## 3. Fields that are truly written today

These snapshot facts are currently backed by actual direct writes:

- `bytes[4876]`: real when `known == true`; it is copied from
  `SaveUiDirectMemory::savePayloadBank`.
- `known`: real current completeness bit from `savePayloadBankKnown`.
- `statusBankKnown80092F1D`: real subrange-known bit for
  `byte_80092F10 + 0x0D`.
- `lastWriterFunction`: real only for writers that call
  `MarkSavePayloadWriter80092F10`.
- `lastFaultAddress`: real direct-memory fault address when bounds checks fail.
- `wrote80015CC4`: real. `Sub80015CC4Direct` clears 4876 bytes and then calls
  `Sub8001635CDirect(1,1,1,0)`.
- `wrote800164B4`: real. `Sub800164B4Direct(src)` copies 4876 bytes from a
  direct-memory source into `0x80092F10`.
- `wrote8001635C`: real when `Sub8001635CDirect(...)` completes all writes:
  status max update, score at `0x14 + idx*4`, slot index, replay mirror,
  source, and all-clear latch.
- `wrote8001628C`: real. `Sub8001628CDirect(stage)` conditionally unlocks one
  status byte if it is zero.
- `wrote800167A8`: real. `Sub800167A8(stage, mode)` writes one status byte
  through `byte_80092F1D`.

Important nuance: `Sub8001635CDirect` depends on the replay mirror source and
`dword800901BC` being known. If either is unknown, it keeps the prefix marked
incomplete and does not mark `wrote8001635C`.

## 4. Fields that are only reserved today

These fields exist in the carrier but are not currently set by any direct
writer:

- `wrote80015744`: reserved only. `MarkSavePayloadWriter80092F10()` has no
  `kFn80015744` case, and `PrStage1SaveUiDirect` has no direct
  `Sub80015700/Sub80015744` backup/restore implementation.
- `80019458` deliberately has no `wrote*` prefix-writer field after review.
  `CopyState15SavePayload80019458` only copies
  `a3/0x80092F10 -> byte_8007ADE8`; it does not mutate the prefix.

Related gap:

- The code has no direct-memory representation of `unk_80079008[4876]`, so
  `80015700(a1)` backup and `80015744(a1)` restore cannot be expressed inside
  `PrStage1SaveUiDirect` yet.
- The lifecycle host adapter still handles
  `RestoreTransitionPayload15744` through `PrEvent::RestoreStageSelectPayloadBackup15744()`.
  That path copies `PrCard::Payload()` backup bytes and reloads shell
  authority. It is not a typed PSX direct restore into
  `SaveUiDirectMemory::savePayloadBank`.
- `Start19148()` currently calls `ResetDirectMemory()`. That is correct for an
  isolated save-UI carrier, but it means a later `80019148(&byte_80092F10)`
  cutover must explicitly preserve/import the current prefix before entering
  the UI; otherwise the pointer-writer path starts from a fresh direct-memory
  state rather than the producer graph's existing `byte_80092F10`.

## 5. Coverage verdict

Current `PrStage1SaveUiDirect::GetSaveStatusPrefix80092F10()` is a useful
typed snapshot carrier, but it does not yet completely cover the full PSX
writer provenance for `80019414 -> 80019284`.

Covered:

- direct clear/seed: `80015CC4`
- direct load-overwrite: `800164B4`
- direct stage progress/score/replay mirror update: `8001635C`
- direct status unlock: `8001628C`
- direct status write: `800167A8`
- current 4876-byte snapshot bytes when all known bits remain true

Not covered:

- `80015744(&byte_80092F10)` whole-prefix restore from `unk_80079008`
- `80015700(&byte_80092F10)` backup source that makes the later restore valid
- `80019148(&byte_80092F10)` importing an already-produced prefix into the
  save-UI direct memory before the state machine begins
- `800185D0` writes to `a3 + 1` as direct writes into the prefix bank
- `80019458` state 15 provenance as a consumer of the prefix and successful
  save writer (`gp+716/gp+720`), not as a prefix mutator

Therefore the current snapshot can feed `80019284` only for paths whose prefix
was produced by the already-covered direct helper calls. It must not be treated
as a fully provenance-complete authority for paths involving restore or the
save-UI pointer writer until the gaps above are implemented.

## 6. Minimal next implementation position

Do not implement this in `pr_scene_entry_direct.*` and do not use
`PrCard::Payload()` as the authority. The minimal correct location is the
save/lifecycle direct seam that already owns `PrStage1SaveUiDirect` and the
`801C81EC` lifecycle host adapter.

Minimum code changes for a later code pass:

1. Add direct backup storage to `PrStage1SaveUiDirect`:

```text
backup80079008[4876]
backup80079008Known
```

2. Add direct wrappers:

```text
Sub80015700_BackupSaveStatusPrefix(a1Address)
Sub80015744_RestoreSaveStatusPrefix(a1Address)
```

For the Stage1 path, only the `a1 == 0x80092F10` case should write
`savePayloadBank`; unknown pointer cases should return a helper gap, not fall
back to `PrCard`.

3. Route `ActionKind801C81EC::RestoreTransitionPayload15744` in
`pr_stage1_lifecycle_host_adapter_801c81ec.cpp` to the new
`PrStage1SaveUiDirect::Sub80015744...` wrapper instead of
`PrEvent::RestoreStageSelectPayloadBackup15744()` once the matching
`80015700` backup producer is also wired.

4. Add a narrow import path for `80019148(&byte_80092F10)`:

```text
Start19148(ctx, optional SaveStatusPrefix80092F10 seed)
```

The seed must come from the current direct prefix snapshot. It must be copied
into `savePayloadBank` before the `80018FB0/800185D0/80019458` state machine
runs. This keeps `a3 + 1` writes as direct writes into the same PSX memory bank.

5. Track `800185D0` suffix writes as prefix writes:

```text
DirectWriteCString(0x80092F10 + 1, suffix)
Mark provenance: wrote80019148PointerInput or wrote800185D0
```

The existing `saveSuffix` host field may remain a convenience mirror for UI
logic, but the authoritative bytes for `80019284` must be the direct memory
bank.

6. Do not add `wrote80019458` as a prefix writer. If provenance is needed,
add separate consumer/save-write fields in a later code pass:

```text
consumedBy80019458State15
saveWriteSucceeded80019458
gp716After80019458
gp720After80019458
```

This avoids implying that `80019458` mutates `byte_80092F10` when IDA shows it
copies `a3` out to `byte_8007ADE8`.

Only after these gaps are closed should the scene-entry feedback builder treat
`GetSaveStatusPrefix80092F10()` as complete enough to combine with
`Case17Result80019D7C.bank` and call
`PsxBuild80019284InputMemoryFromStatusAndBank80015788(...)`.

## 7. Forbidden shortcuts

- Do not set `wrote80015744` from `PrEvent::RestoreStageSelectPayloadBackup15744()`.
- Do not read `PrCard::Payload()` directly to seed the scene-entry prefix.
- Do not default missing prefix bytes to zero for `80019284`.
- Do not treat `80019458` as a real prefix writer.
- Do not connect executor `Call80019414` until prefix snapshot, case17 bank,
  `80019284` table result, and stable event-6 host arg storage are all known.
