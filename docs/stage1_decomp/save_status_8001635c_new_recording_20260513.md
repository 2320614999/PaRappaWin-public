# Save/Status `8001635C` Live Evidence - 2026-05-13

Sources:

- Recording:
  `E:\game\PSgame\parappa the rapper\duckstation\bin\x64\标题画面-menu-选关选stage1-good通关-保存记忆卡（过场动画均为跳过）.psxrec`
- Static export:
  `docs/stage1_decomp/ida_export_multimodule_min_windows_clean_20260513.txt`
- Follow-up call-chain export:
  `docs/stage1_decomp/ida_export_save_status_8001635c_callchain_20260513.txt`
- Call-chain summary:
  `docs/stage1_decomp/save_status_8001635c_callchain_20260513.md`
- S1 overlay pseudocode:
  `docs/stage1_decomp/comod_status_coupled_pseudocode_20260511/S1_COMOD1/decompile_801c81ec.txt`

This is evidence only. It does not authorize replay payload sampling or Win-side
save-file state as status-bank authority.

## Replay Hit

`writers 0x80092F1D 6 12300 12330` reports one hit:

| Frame | PC | Write |
| --- | --- | --- |
| `12319` | `0x80016408` | `80092F1D: 0x01 -> 0x02` |

`80026314`, `800167A8`, and `800916F6` do not move in this recording.

## Static Meaning

`sub_8001635C(a1, a2, a3, a4)` computes
`idx = sub_8001615C(a1)`, then promotes the status byte only when the current
value is less than `a2`:

```c
if (*((unsigned __int8 *)&byte_80092F10 + idx + 13) < a2)
    *((_BYTE *)&byte_80092F10 + idx + 13) = a2;
```

The disassembly shows the actual store at `80016404: sb $v0, 0($v1)`;
`80016408` is the following instruction, so the replay PC is the post-store
continuation point.

`80092F1D == 80092F10 + 0x0D`, therefore this hit is slot `0` status
promotion. The observed `0x01 -> 0x02` supports `a2 == 2` for this path.

## Caller Window

Follow-up SCUS IDA export shows only two direct main-EXE callers of
`8001635C`: `800154F4` and `80015CC4`, both with `(1, 1, 1, 0)`. The live
Stage1 clear-tail hit is therefore the S1 overlay function-pointer call path,
not a new SCUS direct xref.

The frame call trace includes:

```text
0x8001635C (from 0x801C8494, sp=0x801FFF88)
```

The S1 overlay pseudocode around `PrScene1_RunMovie1_AndStage1` has the matching
clear-tail pattern:

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

## Conclusion

- The `80092F1D` hit is `8001635C` save/status bank max-promote behavior.
- It is not the `800167A8` leaf writer.
- It proves a live write in this recording, not payload contents.
- `80016404` is an internal store instruction in `8001635C`, not a new
  function/request id.
- Remaining evidence gap: the recording does not include `$ra/a0-a3`, so a
  future live watchpoint should record `ra`, `a0-a3`, `800916DA`, `800916F0`,
  and `80091816` before calling this caller fully closed.
