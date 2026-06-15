# Fail rail compact reset analysis 2026-05-25

## Scope

This file accompanies `ida_export_fail_rail_compact_reset_20260525.md`.
It records the local IDA evidence gathered after commit `347a64da`, which
adds a direct one-frame `801C9094` flag40 early-return reset pulse into the
Stage1 runtime-slot compact producer.

## Export Notes

- Current attached IDA input is the base executable
  `E:\game\PS模拟器\parappa the rapper\SCUS_941.83`.
- Base functions exported successfully: `80024308`, `80024390`, `80024744`.
- Overlay functions `801C8D78` and `801C9094` are not present as functions in
  this attached base IDB; use the already committed COMOD1 exports:
  `docs/stage1_decomp/801C9094_sub_801C9094.c` and
  `docs/stage1_decomp/comod_status_coupled_pseudocode_20260511/S1_COMOD1/decompile_801c9094.txt`.

## Findings

- `80024308` loops 36 entries and resets the `800876F8/800876FC/80087700/80087704`
  bank to `0/2048/2048/0`, while also writing shared scale words
  `80087668` and `800876B0` to `4096`.
- `80024390` loops 36 entries and resets the parallel
  `80087938/8008793C/80087940/80087944` bank to `0/2048/2048/0`, with the
  same shared `80087668/800876B0 = 4096` writes.
- `80024744(a1)` draws compact rail rows only when `*(s16*)(a1+0x8A) > 0`.
  If the producer state that feeds `ctx+0x8A` is cleared to zero before the
  submit path, `80024744` should return before emitting compact rail commands.
- COMOD1 `801C9094` flag40 early-return pseudo-C still provides the overlay
  authority: it clears `ctx+0x8A/+0x8C/+0x8E/+0x90` and
  `ctx+0x9E/+0xA0/+0xA2` state, restores the body pointers to
  `unk_801CFA58/unk_801CFA6C`, calls `80024390` and `80024308`, and returns
  early.

## Current Patch Boundary

- `347a64da` now carries the `returnEarlyAfterFlag40Reset` fact into runtime
  slots and calls `ResetStage1RuntimeCompactProducer801C9094(...)` before the
  pre-event steady compact producer, then emits both `80024308/80024390` reset
  pulses.
- If user testing still shows rail HUD during the fail animation after
  `347a64da`, the next direct-port seam is no longer stale `801C9094`
  compactProducer state. The next seam is likely retained `80024744` sprite
  commands / work-list preservation across `801CBFDC`, or a separate owner
  still submitting compact rail commands after `ctx+0x8A` has been cleared.

## Patch Status

No additional runtime patch in this export pass.
