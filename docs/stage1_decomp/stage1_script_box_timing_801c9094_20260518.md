# Stage1 script box timing: `801C9094 -> 801C8604 -> ctx+0x10C`

Date: 2026-05-18

## Scope

This note covers the gameplay subtitle/script-box timing seam only. It does
not change Movie1 subtitle rendering, native text fallback, renderer-side
delays, or the `80023E10` packet draw implementation.

## PSX authority

- `801C9094` owns the gameplay text sink lifetime. Its frame update decrements
  `word_8008ECFA` and clears `unk_8008ECE4` plus `ctx+0x10C` when the timer
  expires.
- `PrStageRunner_ApplyScriptEvent_SetText` is called only from the direct
  `801C9094` runner path when the script event is actually applied and
  `ctx+0x4E` is non-zero.
- The later `801CBFDC` render tail still calls `80023E10(ctx+0x10C)` when
  `ctx+0x54 != 0` and `ctx+0x68 != 0`.
- `80023E10` itself does not guard the frame geometry on `ctx+0x10C`. Its
  pseudo-C draws three `8001C4EC` box-fill rects, four `8001C550` corner
  FastSprites, then calls `8001B730(28,194,0)` and the text loop
  `8001C6A0(a0,480)`.

Therefore the source of `ctx+0x10C` must be the direct
`Call801C8604ApplyScriptEventSetText` action, not every consumed event and not
the `ctx+0x54` draw gate; the frame/corners must still be emitted when
`ctx+0x10C == 0`.

## Recorder evidence

Recording:
`标题画面-menu-选关选stage1-good通关-保存记忆卡（存档重名保存失败重试成功）（过场动画均为跳过）v3.psxrec`

Narrow frame facts:

- frame `1500`: `ctx+0x54 = 1`, `ctx+0x68 = 1`, `ctx+0x10C = 0`.
- frame `1780`: call trace has `801CC984 -> 80023E10` and
  `801CC9A0 -> 8001E2E4` while `ctx+0x10C = 0`.
- frames `1780..2136`: repeated `80023E10` calls continue with
  `ctx+0x10C = 0`.
- frame `2137`: `801C9094 -> 801C895C -> 801C8604` writes
  `ctx+0x10C = 0x801C584C` and `word_8008ECFA = 0x78`.
- frame `2138`: the next `80023E10` draw has `ctx+0x10C = 0x801C584C`.

This proves the original Stage1 box/frame appears before the lyric text, and
the glyph text joins later when `801C8604` installs the text pointer.

## Code change

- `PrStageEventDirectStage1FrameResult801C9094` now carries
  `setTextEvent801C8604`, resolved from the runner action trace entry
  `Call801C8604ApplyScriptEventSetText`.
- `PrStage1OverlayScriptTextDirect` now updates the active `ctx+0x10C` mirror
  only from `setTextEvent801C8604`.
- `80023E10` scene-submit now emits rect/corner packets even when the current
  `ctx+0x10C` pointer is null; only the glyph loop remains dependent on a
  non-null text pointer.

## Boundary

This is not a renderer timing patch. It separates the PSX text-write condition
from the PSX box-draw condition: the box is authorized by the real
`ctx+0x54/0x68 -> 80023E10` call, while glyphs are authorized by
`ctx+0x10C`.
