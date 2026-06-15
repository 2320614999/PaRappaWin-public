# Event2 StageSelect `8001B4E0` Frame 620 Follow-Up - 2026-05-13

Source recording:

- `E:\game\PSgame\parappa the rapper\duckstation\bin\x64\标题画面-menu-选关选stage1-good通关-保存记忆卡（过场动画均为跳过）.psxrec`

IDA export:

- `docs/stage1_decomp/ida_export_event2_stage_select_8001b4e0_frame620_20260513.txt`

This is a static follow-up to
`docs/stage1_decomp/event_frame_psxrec_calltrace_20260513.md`. It uses the
frame `620` call-trace chain only to choose the IDA evidence window. It does
not use sampled RGB bytes as implementation authority.

## Frame 620 Chain

The recording covers the event2 StageSelect path:

```text
8001E8BC -> 80020568
8002081C -> 8001C604
8001C650 -> 8001B654
8001B694 -> 8001B4E0
8001B6A4 -> 8003FA20
```

The current IDA export confirms the same static call chain. `80020568` has the
single function-start xref `8001E8BC -> 80020568`; `8001C604` has the event2
callsite `8002081C`; `8001B654` calls `8001B4E0` at `8001B694` and
`GsSortFastSprite` at `8001B6A4`.

## `8001B4E0` Local Writes

`8001B4E0` writes the local FastSprite prefix only:

```text
8001B50C: sw  ..., 0($s2)
8001B508: sh  ..., 8($s2)
8001B518: sh  ..., 0xA($s2)
8001B540: sh  ..., 0xC($s2)
8001B544: sb  ..., 0xE($s2)
8001B550: sb  ..., 0xF($s2)
8001B55C: sh  ..., 0x10($s2)
8001B56C: sh  ..., 0x12($s2)
```

There is still no store to local `+0x14..+0x16`. Hex-Rays decimal `+14/+15/+16`
in this function correspond to `+0x0E/+0x0F/+0x10`, not the RGB tail.

`8003FA20` remains the consumer:

```text
8003FAB0: lbu $a2, 0x16($t3)
8003FAB4: lbu $t0, 0x15($t3)
8003FAB8: lbu $t1, 0x14($t3)
```

The later `8003FB54: sw ..., 0x14($a3)` is a packet write after consumption,
not a write to the FastSprite local input.

## Stack Tail Map

The existing stack-tail map still applies:

```text
8001B654 local base: child_sp+0x18
8003FA20 RGB tail read: child_sp+0x2C..0x2E
8001B654 entry map: entry_sp-0x24..entry_sp-0x22
8001C604 entry map: entry_sp-0x4C..entry_sp-0x4A
80020568 entry map: entry_sp-0xE4..entry_sp-0xE2
```

This proves the current closure is a consumer path, not an RGB owner path.
Old-stack overlap remains possible, but overlap is not semantic authority.

## Decision

- No source patch is authorized from this window.
- `RenderStageSelect(...)` fallback cannot be deleted from this evidence alone.
- `8001B4E0`, `8001B654`, and `8001C604` must stay classified as local
  prefix/submit builders until a true writer for `+0x14..+0x16` is proven.
- Next event2 evidence, if this line is reopened, should focus only on the
  tight old-stack window around `80020568` `800207B4 -> 8002081C` and the
  same physical stack tail. Do not rescan generic `8001B590/B25C/FA20`
  helpers unless a new producer entry is identified.
