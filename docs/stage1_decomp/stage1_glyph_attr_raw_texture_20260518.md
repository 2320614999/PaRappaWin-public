# Stage1 glyph FastSprite attr / raw-texture bit - 2026-05-18

## Scope

Gameplay subtitle main color was still yellow/brown after the border and
subtitle frame path were corrected. This pass stays inside the direct
`8001B954 -> 8003FA20` packet source. It does not set a Win text color and
does not restore native subtitle rendering.

## Evidence

- IDA export `ida_export_glyph_b954_owner_followup_20260511.txt` shows
  `8001B954` initializes the local FastSprite object with
  `v24 = 1342177344`, which is `0x50000040`.
- `8003FA20` builds packet word2 with:
  `(attr >> 5) & 0x02000000 | (attr << 18) & 0x01000000 | 0x64000000 | rgb`.
- With the PSX attr `0x50000040`, the glyph packet opcode is `0x67`.
  Opcode bit `0x01` is the raw-texture bit, so the packet RGB tail
  `CC B5 01` is not a texture tint.
- The previous direct constant `0x50000000` instead produced opcode `0x66`,
  disabling raw-texture and causing white glyph texels to be multiplied by
  `CC B5 01`.

## Code decision

`kMovie1TextGlyphFastSpriteAttrSub8001B954` now uses `0x50000040`, matching the
IDA value from `8001B954`. The renderer adapter continues to derive raw-texture
behavior from the packet opcode; no subtitle color is hard-coded.

