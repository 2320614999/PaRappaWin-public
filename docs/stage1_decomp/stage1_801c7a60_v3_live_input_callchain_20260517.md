# Stage1 `801C7A60` v3 live-input call-chain boundary (2026-05-17)

Scope: only the Stage1 `801C7A60` live accepted-input path around the v3
row3 first-beat window. This note does not authorize changes to scorer math,
row/v22 tables, replay seed tables, full-song harness input semantics,
renderer, lifecycle clear, marker logic, or synthetic success.

Recording:

```text
E:\game\PSgame\parappa the rapper\duckstation\标题画面-menu-选关选stage1-good通关-保存记忆卡（存档重名保存失败重试成功）（过场动画均为跳过）v3.psxrec
```

Primary facts already captured in
`docs/stage1_decomp/stage1_v3_accepted_page_history_source_20260517.md` and
`docs/stage1_decomp/stage1_v3_ctx52_replay_append_boundary_20260517.md`:

- frame `5399`: `801C7F24` writes `ctx+0x18 = 0x10`, `801C7F38`
  writes `ctx+0x20 = 1`, and `ctx+0x52 == 0`;
- frames `5429/5449/5479`: the same writer PCs write
  `ctx+0x18 = 0x20`, `ctx+0x20 = 2`, with `ctx+0x52 == 0`;
- `80014614` appends the same non-replay accepted sequence into the replay log
  (`800901BC/800901C0` advance to `21/22/23/24`);
- frame `5513`: `80014D58/80024F8C` then advance the score/latch state.

## Static call order

Authority source:

- `docs/stage1_decomp/801C7A60_sub_801C7A60.c`
- `docs/stage1_decomp/stage1_801c7a60_mask_edge_tail_recheck_20260516.md`
- saved COMOD1 export
  `docs/stage1_decomp/fail_prompt_ev4_20260510/ida_export_comod1_801C7A60_801C81EC_p0_20260510.txt`

| Order | PSX site | Meaning in this window |
|---:|---|---|
| 1 | `801C7EDC -> 80035510(1)` | Poll live pad/control value because `ctx+0x52 == 0` non-replay path is active. |
| 2 | `801C7F1C/801C7F20` | Normalize low bits `1 -> 4`, `2 -> 8`, otherwise preserve the returned mask. |
| 3 | `801C7F24` | Write normalized control mask to `ctx+0x18`; v3 frame `5399` proves the first row3 accepted value is `0x10`. |
| 4 | `801C7F24 -> 80024B54(ctx+0x18)` | Resolve class token from the just-written control mask. |
| 5 | `801C7F38` | Write class token to `ctx+0x20` and set `ctx[0] bit0`; v3 frame `5399` writes class `1`. |
| 6 | `801C7F48 -> 80024FD0(ctx)` | Run the bucket owner after the same-frame input fields are populated. |
| 7 | `801C7F50 -> 801C9094(ctx)` | Run event-stream frame update after bucket owner. |
| 8 | `801C7F68..801C7FB8` | Accepted tail gate is the mask edge: `(*ctx & 1) && ((ctx+0x18) & 0x9FF) != dword_801CCBB8`; non-replay backs up `ctx+0x08/0x10` before calling `80014614(ctx)`. |
| 9 | `80014614` | Append the non-replay accepted record into page/replay history; v3 appends `10755/0x10`, then `10841/0x20`, `10900/0x20`, `10989/0x20`. |
| 10 | `801C80BC` | Update `dword_801CCBB8` to the current masked input edge. |

## Current Win boundary

The current direct host already mirrors the required call order:

```text
CaptureStage1SteadyInput7A60
  -> UpdateStage1AcceptedProducerCarrierRuntime
  -> RunStage1ScorerDispatcher24FD0
  -> RunStage1FrameUpdate9094
  -> RunStage1AcceptedProducer14614
```

The current mismatch is not a fresh `801C8660` replay-table seed and not a
known `80014614/80014D58/80024F8C` direct-core deviation. The v3 call-chain
boundary says the missing runtime authority is the live `80035510(1)` source
sequence that must present `0x10/class1` first, then `0x20/class2`, on the
same non-replay `801C7A60` path.

## Recorder trace follow-up

After `zstandard` became available for the current Python, the same v3
recording was rechecked with the recorder Python API rather than the truncated
CLI `call_trace` printout. The full call trace confirms the static order above
in the row3 window:

| Frame | Full call-trace hits |
|---:|---|
| `5399` | `idx55: 80035EF4 -> 801C7560`; `idx705: 801C7EDC -> 80035510`; `idx709: 801C7F28 -> 80024B54`; `idx711: 801C7F48 -> 80024FD0`; `idx715: 801C7F50 -> 801C9094`; `idx727: 801C7FB8 -> 80014614`; `idx750: 800149B4 -> 80024B54`. |
| `5429` | same chain at `idx755/759/761/763/771`, with the later `80014614 -> 80024B54` call at `idx794`. |
| `5449` | same chain at `idx695/699/701/703/711`, with the later `80014614 -> 80024B54` call at `idx734`. |
| `5479` | same chain at `idx728/732/734/736/744`, with the later `80014614 -> 80024B54` call at `idx767`. |
| `5513` | `801C7EDC -> 80035510`, `801C7F48 -> 80024FD0`, and `801C7F50 -> 801C9094`; no same-frame accepted `80014614` call in this filtered target set. |

The writer/xref follow-up also confirms the field owners in `5390..5520`:

- `0x801C3658` (`ctx+0x18`): `801C7F24` writes `0 -> 0x10/0x20`;
  `801C7F44` clears it back to `0`;
- `0x801C3660` (`ctx+0x20`): `801C7F38` writes `0 -> 1/2`;
  `801C7F48` clears it back to `0`;
- `0x801C3650` (`ctx+0x10`): `801C7FB8` writes
  `10755/10841/10900/10989`;
- `0x801CCBB8`: `801C80C4` mirrors the masked edge `0x10/0x20` and its
  later clear;
- `0x800901BC` and `0x800901C0`: `80014994` / `8001498C` advance the replay
  write counters from `20` to `24`;
- frame `5513`: `8001528C` writes `80091816`, `80014D44` writes
  `80091818`, and `80025434` writes `8008ED36`.

The same recorder frame input values match that dispatch without requiring a
new scorer-side authority:

| Frame | Recorder input | `80035510` returned-shape value used by `801C7A60` |
|---:|---:|---:|
| `5398..5400` | `0x1000` | `0x10`, class `1` after `80024B54`. |
| `5405` | `0x0000` | no accepted input; `ctx+0x18/+0x20` clear. |
| `5428..5430` | `0x2000` | `0x20`, class `2` after `80024B54`. |
| `5449` | `0x2000` | `0x20`, class `2`. |
| `5479` | `0x2000` | `0x20`, class `2`. |
| `5513` | `0x0000` | no accepted input in this filtered window. |

`src/pr/pr_psx_pad_direct.cpp` already has both needed normalization surfaces:
debug/recorder-style high bits `0x1000/0x2000` become returned-shape
`0x10/0x20`, while local `PrPadButton::Triangle/Circle` already occupy
`0x10/0x20`. Therefore this evidence does not justify a scorer or replay
patch; it says the runtime path must receive the correct live input sequence.

## Conclusion

Do not keep editing scorer/replay/row tables for this line. The next executable
change, if any, must connect the real Stage1 runtime input source so the
existing `801C7A60 -> 80014614` direct path receives the PSX-shaped
non-replay live sequence. If that source cannot be recovered from current
runtime state, the valid next evidence is recorder/live context for
`80035510(1)` return source in the target window, not another scorer patch.
