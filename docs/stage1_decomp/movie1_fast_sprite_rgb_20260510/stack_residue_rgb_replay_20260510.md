# `8003FA20` RGB stack-residue evidence, 2026-05-10

## Replay input

- Recording:
  `E:\game\PSgame\parappa the rapper\duckstation\stage1二周目_无cool_good评级通关3.psxrec`
- Tool:
  `tools/模拟器内存回放器.py`
- Output:
  `temp/psxrec_8003fa20_rgb_20260510/summary.json`
  and `temp/psxrec_8003fa20_rgb_20260510/samples.json`

The recording has `call_trace` and `write_log`, but no `read_log`.
The sampled RGB bytes are reconstructed at the `8003FA20` call `event_seq`
from frame-start bytes plus writes before the call.

## Covered callsites

- `8001B590_stack`: `8001B5D4 -> 8003FA20`, return `8001B5DC`
- `8001BEE4_stack`: `8001BF1C -> 8003FA20`, return `8001BF24`
- `8001B954_glyph_stack`: `8001BBF8 -> 8003FA20`, return `8001BC00`

This recording did not cover `8001B654`, `8001B744`, or `8001BCA0`.

## Replay facts

- Full summary counts:
  - `8001B590_stack`: 181888
  - `8001BEE4_stack`: 26538
  - `8001B954_glyph_stack`: 80416
- `8001B954_glyph_stack` is stable in this recording:
  all 80416 hits have RGB `[204,181,1]`.
- The 360-row sample window shows `8001B954_glyph_stack`
  `a0+0x14..0x16` last writer as `0x8001B26C`.
- The same sample window shows `8001B590_stack` first-frame samples have no
  last writer before the `8003FA20` call.
- `8001BEE4_stack` samples have last writers `0x80048040` or `0x8001B274`,
  and full-summary RGB values are not stable.

## Static attribution

- `8001B25C` pseudocode writes the FastSprite-like local prefix only through
  `a1+0x12`: attr, width/height, tpage, uv, and clut. It does not write
  `a1+0x14..0x16`.
- `8001B26C` is not a pseudocode field write. The disassembly maps it to the
  `8001B25C` prologue saved-register spill `sw $s4,...($sp)`.
- `8001B954` pseudocode builds its local glyph object and calls
  `GsSortFastSprite(&v24, v33, 0)`, but it has no pseudocode assignment to the
  local bytes consumed by `8003FA20` as `+0x14..0x16`.
- `8001BEE4` calls `8001BE34` then `GsSortFastSprite(v6, a5, 3)`.
  `8001BE34` mirrors the same prefix-only behavior and does not write
  RGB tail bytes.
- `801CA634` (`PrDrawGradientRect`) is a Gouraud rectangle submitter with its
  own stack-local color fields.
- `801CAC34` (`PrStageRunner_DrawBackground`) calls `PrDrawGradientRect` and
  then repeatedly calls `8001B590`; this explains `801CA654` as an upstream
  stack-residue source for later `8001B590` local bytes, not as a FastSprite
  RGB owner.
- `8004800C` is the formatter. Its stack writes can seed later reused stack
  slots, but it is not an `8001BEE4` FastSprite RGB field initializer.

## Implementation consequence

`8003FA20` really consumes `a0+0x14..0x16` for packet word2. For these callers,
the consumed bytes are PSX stack residue, not semantic local fields. Therefore:

- Do not add default RGB in the renderer.
- Do not add an explicit `[204,181,1]` constant to `8001B954`.
- Do not treat `8001B590`, `8001BEE4`, or `8001B954` as full packet authority
  until the direct runtime can represent the stack scratch/residue feeding
  those local object tails.
- The next implementation unit should be a PSX stack scratch carrier attached
  to the direct submit path, or a wider caller-chain direct port that naturally
  preserves the same stack bytes.
