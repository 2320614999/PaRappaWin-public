# Event-Frame PSXREC03 Call-Trace Coverage - 2026-05-13

Source recording:

- `E:\game\PSgame\parappa the rapper\duckstation\bin\x64\标题画面-menu-选关选stage1-good通关-保存记忆卡（过场动画均为跳过）.psxrec`

Tool:

- `tools/模拟器内存回放器.py`

This is dynamic coverage evidence only. It does not authorize deriving RGB,
payload, success, or handled state from sampled RAM.

## Important Correction

The CLI `bp <addr>` command only checks the frame-end PC. A `bp` miss is not
proof that the function did not execute inside the frame. Use `pc_trace`,
`call_trace`, or a scripted scan over `get_call_trace(...)` for intra-frame
call coverage.

Earlier `bp 0x80020568` / `bp 0x8001C604` misses are therefore weak negative
evidence. The call trace in this recording does cover the event2 StageSelect
chain.

## Target Scan

Full-frame scripted `call_trace` target scan over `0..14317`:

```text
0x80020568: first frame 620, then 620..629...
0x800203D4: no call_trace hit
0x8001D74C: first frame 454
0x8001C604: first frame 620, then 620..629...
0x8001B654: first frame 620, then 620..629...
0x8001B4E0: first frame 620, then 620..629...
0x8001B590: first frame 2
0x8001B25C: first frame 2
0x8003FA20: first frame 2
0x80026314: no call_trace hit
0x80026B94: first frame 454, also frame 620
0x800167A8: no call_trace hit
```

## Frame 620 Event2 Chain

Relevant call counts from frame `620`:

```text
8001E8B4 -> 8001D74C  count=1
8001E8BC -> 80020568  count=1

8002081C -> 8001C604  count=6
8001C650 -> 8001B654  count=6
8001B694 -> 8001B4E0  count=6
8001B6A4 -> 8003FA20  count=6

80026B94 caller: 8001591C -> 80026B94 count=1
```

The same frame also has the expected shared/backdrop submit traffic:

```text
8001D74C -> 8001B590 / 8001B25C / 8003FA20 backdrop path
8001B5D4 -> 8003FA20 count=84 at sp=0x801FFE98
8001B5D4 -> 8003FA20 count=35 at sp=0x801FFE20
```

Frame-end `info 620` reports `PC=0x80035724`, which explains why frame-end
`bp` cannot be used as a negative execution proof for this chain.

## Decision

- The latest recording covers event2 StageSelect:
  `8001E750 -> 8001D74C -> 80020568 -> 8001C604 -> 8001B654 -> 8001B4E0 -> 8003FA20`.
- The latest recording still does not cover event4 prompt `800203D4`.
- The remaining event-frame blocker is unchanged: the `8003FA20` local
  `+0x14..+0x16` RGB owner/writer is still missing. The frame 620 call trace
  gives the exact dynamic callsite window for follow-up, but the sampled local
  RGB bytes are not implementation authority.
- Next evidence window: use frame 620 callsites to re-check the
  `80020568 -> 8001C604 -> 8001B654 -> 8001B4E0` static/local stack path,
  especially the `8001B4E0` local tail consumed by `8003FA20`.
