# 80019148 seeded start minimal boundary

Scope: static-only review for the `80019148(&byte_80092F10)` save-UI entry.
This document uses IDA Hex-Rays for `80019148`, `800185D0`, and `80019458`,
plus read-only inspection of `PrStage1SaveUiDirect`. No `src` edit, no build,
no runtime replay.

## PSX facts

`80019148(a1)` does not allocate or clear `a1`. It prepares the save UI and
passes the exact caller pointer into `80018FB0`:

```c
sub_80020110(0, 3, 2, 1);
sub_80025C64(sub_80026784(), byte_8007CC50, 36);
*(gp + 716) = 0;
*(gp + 732) = 0;
sub_80017524();
sub_80018FB0(a1, sub_800185D0, sub_80019458, 21, 11);
sub_80018F70();
sub_80017574();
return *(gp + 716);
```

For the Stage1 caller path, `a1 == &byte_80092F10`. Therefore the save UI must
start from the already-produced `byte_80092F10[0..4875]` prefix, not from a
fresh buffer.

`800185D0(input, state, a3)` is the input callback. In the naming flow it writes
through the `a3` pointer:

```c
// state 10, confirm generated suffix
strcpy(a3 + 1, *(gp + 148));

// state 22, confirm selected existing suffix
strcpy(a3 + 1, selectedSuffix);
```

These writes are real writes to `byte_80092F10 + 1` when `a3` is the Stage1
save prefix. A host `saveSuffix` string can only be a UI mirror; it is not the
authoritative memory for later `80019284`.

`80019458(state, ioResult, a3)` is the state-machine/IO callback. It reads
`a3 + 1` in directory-list states and, in state 15, consumes the full prefix:

```c
sub_8004800C(byte_8007CBE8, "%s%s", *(gp + 136), a3 + 1);
...
sub_80025C64(a3, byte_8007ADE8, 4876);
if (sub_80017A10(byte_8007CBE8, byte_8007ABE8, 1) < 0)
  return 2;
*(gp + 716) = 1;
*(gp + 720) = 1;
return 23;
```

So `80019458` is not a prefix writer. It is a consumer of the current `a3`
prefix and a producer of save-write result state.

## Current gap

`PrStage1SaveUiDirect::Start19148(ctx)` currently does:

```c++
s_saveUi19148 = SaveUi19148Runtime{};
ResetDirectMemory();
s_saveUi19148.active = true;
s_saveUi19148.state = 21;
s_saveUi19148.eventId = 11;
```

`ResetDirectMemory()` resets `SaveUiDirectMemory`, including
`savePayloadBank[4876]`, `savePayloadBankKnown`, writer flags, and fault state.
That is safe for an isolated save-UI carrier, but it breaks the PSX pointer
contract for `80019148(&byte_80092F10)`: the state machine starts with an empty
direct memory instead of the prefix produced by `80015CC4/800164B4/8001635C/
8001628C/800167A8/80015744`.

The current suffix path is also only a mirror:

```c++
CopySuffix(...)
```

copies into `s_saveUi19148.saveSuffix`, while IDA shows the authoritative write
is `strcpy(a3 + 1, ...)`. `CopyState15SavePayload80019458()` later expects
`savePayloadSourceAddress == 0x80092F10` and requires
`savePayloadBankKnown`; after `ResetDirectMemory()` this can only be true if a
real seed was imported or a direct writer ran in the same memory instance.

Therefore `Start19148()` needs a seeded entry form before it can be used as the
true `80019148(&byte_80092F10)` implementation for scene-entry feedback.

## Minimal code boundary

Keep the implementation inside `PrStage1SaveUiDirect`. Do not implement this in
`pr_scene_entry_direct.*` and do not read card payloads from `PrCard`.

Minimum later code pass:

1. Add a seeded start overload:

```text
Start19148(ctx, const PrStage1SaveStatusPrefix80092F10* seed)
```

or an equivalent request object. The no-seed overload may remain for callers
that intentionally start an isolated carrier.

2. Preserve the current `ResetDirectMemory()` call, but import the seed
immediately after reset and before setting the state machine active:

```text
s_saveUi19148 = {};
ResetDirectMemory();
if (seed && seed->known && seed->psxAddress == 0x80092F10 &&
    seed->byteCount == 4876) {
    copy seed->bytes into savePayloadBank;
    savePayloadBankKnown = true;
    statusBankKnown80092F1D = seed->statusBankKnown80092F1D;
    copy/merge provenance flags and last fault/writer fields;
    savePayloadSourceAddress = 0x80092F10;
    savePayloadSourceKnown = true;
} else {
    return false or mark helper gap;
}
s_saveUi19148.active = true;
s_saveUi19148.state = 21;
s_saveUi19148.eventId = 11;
```

The seed source must be `PrStage1SaveUiDirect::GetSaveStatusPrefix80092F10()`
from the direct prefix producer graph. If that snapshot is unknown or incomplete,
the save UI must stay a gap rather than fabricate bytes.

3. Add a direct-memory suffix writer for the two `800185D0` write sites:

```text
WriteSuffixToA3Plus1_800185D0(suffix):
  DirectWriteCString(0x80092F10 + 1, suffix, within 4876-byte bank)
  update savePayloadBankKnown according to bounds/write success
  mark provenance as 800185D0 pointer write
  optionally mirror to s_saveUi19148.saveSuffix for UI convenience
```

`CopySuffix()` should not remain the only write for these states. The host mirror
may stay, but the authoritative bytes must live in `savePayloadBank`.

4. Keep `80019458` state 15 as a consumer:

```text
CopyState15SavePayload80019458:
  source = 0x80092F10
  require savePayloadBankKnown
  copy 4876 bytes to byte_8007ADE8
  only after real write feedback succeeds, set gp716/gp720/result facts
```

If provenance fields are needed, add consumer/result fields such as
`consumedBy80019458State15`, `saveWriteSucceeded80019458`,
`gp716After80019458`, and `gp720After80019458`. Do not add a
`wrote80019458` prefix-writer flag.

5. Scene-entry feedback may consume the result only after the seeded save UI can
produce a complete `PrStage1SaveStatusPrefix80092F10` and the case17 high-score
bank is also known. The executor should receive a completed
`Call80019414Feedback80015788`; it should not own the seed/import logic.

## Forbidden shortcuts

- Do not seed from `PrCard::Payload()` or any Win save-file payload mirror.
- Do not use emulator replay values as the seed bytes.
- Do not zero-fill a missing `byte_80092F10` prefix.
- Do not bypass `ResetDirectMemory()` by leaving stale direct memory alive; seed
  import must be explicit and typed.
- Do not treat `CopySuffix()` as authoritative unless it also writes
  `0x80092F10 + 1` in direct memory.
- Do not mark `80019458` as a prefix writer.
- Do not connect `80019414` executor on the assumption that seeded start exists;
  keep `DirectGap` until the seed import and `800185D0` pointer writes are real.
