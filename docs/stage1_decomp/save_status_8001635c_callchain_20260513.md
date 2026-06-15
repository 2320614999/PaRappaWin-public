# Save/Status `8001635C` Call-Chain - 2026-05-13

Sources:

- Current SCUS IDA export:
  `docs/stage1_decomp/ida_export_save_status_8001635c_callchain_20260513.txt`
- New recording evidence:
  `docs/stage1_decomp/save_status_8001635c_new_recording_20260513.md`
- S1 overlay pseudocode:
  `docs/stage1_decomp/comod_status_coupled_pseudocode_20260511/S1_COMOD1/decompile_801c81ec.txt`

This is evidence only. It does not authorize replay payload sampling, Win save
state, or any `800167A8` substitute path.

## SCUS Xrefs

Current SCUS IDA has only two direct xrefs to `sub_8001635C`:

| Caller | Callsite | Arguments | Meaning |
| --- | --- | --- | --- |
| `sub_800154F4` | `80015550` | `(1, 1, 1, 0)` | boot/init seed after `SetMem`, callback reset, pad start |
| `sub_80015CC4` | `80015CF8` | `(1, 1, 1, 0)` | clears `80092F10` then reseeds slot 1 |

The live Stage1 clear-tail hit is not one of these SCUS direct callsites. It
comes through the S1 overlay function pointer call path already visible in the
COMOD1 decompile.

## `80016404` Store

`80016404` is not a separate function or request id. It is the conditional
status max-promote store inside `sub_8001635C`:

```text
800163D0: lbu     $v0, 0($v1)          ; current status
800163D4: lw      $v1, 0x18+arg_4($fp) ; a2/status
800163DC: slt     $v0, $v1             ; current < a2
800163E0: beqz    $v0, loc_80016408
800163FC: lbu     $v0, 0x18+arg_4($fp)
80016404: sb      $v0, 0($v1)
```

The recording reports `PC=80016408` because it observes the instruction after
the store. The written address `80092F1D` is `80092F10 + sub_8001615C(a1) + 0x0D`
for slot `0`.

## S1 Clear-Tail Caller

The S1 COMOD1 clear-tail pseudocode calls the SCUS function pointer as:

```c
if (!MEMORY[0x800916DA]) {
    MEMORY[0x8001635C](a1, v7, v5, MEMORY[0x80091816]);
    if (a1 < 6)
        MEMORY[0x8001628C](a1 + 1);
    if (MEMORY[0x800916F0] != 1) {
        MEMORY[0x80015590](a1);
        MEMORY[0x80019148](-2146881776);
    }
}
```

The replay call trace already narrowed the live caller to:

```text
0x8001635C (from 0x801C8494, sp=0x801FFF88)
```

The static call shape is therefore:

```text
S1 PrScene1_RunMovie1_AndStage1
  -> 801C8494 function-pointer call MEMORY[0x8001635C](a1, v7, v5, word_80091816)
  -> SCUS sub_8001635C
  -> 80016404 conditional sb into 80092F10 + idx + 0x0D
```

## Adjacent Xrefs

- `byte_80092F1D` direct xrefs in SCUS remain:
  `800166AC` read, `800167A8` leaf write, and `80026B94` read.
- The new recording hit is not the `800167A8` leaf writer. It is the
  `8001635C` max-promote store through the `byte_80092F10 + idx + 0x0D` base.
- `word_800916F6` still has no writer in this closure.
- `80026314` and `800167A8` live callers remain unclosed for
  MenuHelp/StageClear TEXT.

## Remaining Live Gap

This window closes the static function identity and call shape, but not the
full live register snapshot. The next live watchpoint should still record
`ra`, `a0-a3`, `800916DA`, `800916F0`, and `80091816` at `exec 8001635C` or
`write 80092F1D..80092F22`.
