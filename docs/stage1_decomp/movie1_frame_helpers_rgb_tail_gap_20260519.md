# Movie1 frame helpers RGB tail gap 2026-05-19

Scope: narrow IDA check for Movie1 / clear-tail visible frame helpers
`8001F230`, `8001FEB4`, `8001C864`, and `8001CE30`.

## IDA evidence

- `8001F230(a1)` decompiles to repeated `8001C550(x,y,template,a1)`
  calls using `dword_80050380..80050410`. The call windows only show
  callee-save spills and argument setup; no local FastSprite RGB tail writer
  was found in the helper before the `8001C550` calls.
- `8001FEB4(a1)` likewise emits the no-video 8 x 6 frame through repeated
  `8001C550(x,y,dword_800503E0/3F0/400/410,a1)` calls. Its stack writes are
  loop locals / callee-save / outgoing argument setup, not a semantic RGB
  source.
- `8001C864(a1)` emits subtitle-frame layout through direct
  `8001B590(...)` calls. Each call sets:
  - `sp+0x10 = 0`
  - `sp+0x14 = a1`
  - `sp+0x18 = &unk_80087288 + 20 * gp+0x368`
  These are the 5th-7th call arguments, not local bytes `+0x14..+0x16` of
  the `8003FA20` FastSprite object.
- `8001CE30(a1)` follows the same `8001B590(...)` calling pattern for the
  no-subtitle frame layout and shows the same outgoing-argument stores.

## Boundary

- This evidence does not authorize treating generic `8001B590` stack residue
  as renderable color.
- Existing narrow gates such as glyph `8001B954` and subtitle-box corner
  `80023E10 -> 8001C550 -> 8001B590` remain separate, source-specific
  provenances.
- Movie frame helpers that use `8001C550` may continue through their existing
  template draw route. Direct `8001B590` frame helpers must keep the RGB gap
  until a real PSX source or live/runtime fact provides the tail bytes.

## Next use

If Movie1 / clear-tail frame tiles still disappear, do not open
`StaticLocalFields` or generic `PsxStackScratchResidue` for these helpers.
Instead, capture the exact `8003FA20` entry local object bytes for the target
callsite or find a concrete upstream writer in IDA.
