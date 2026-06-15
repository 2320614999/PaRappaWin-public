# Stage1 glyph CLUT STP / FastSprite alpha - 2026-05-18

## Scope

User retest still showed gameplay subtitle glyphs tinted yellow/green after
the `8001B954` glyph tpage/CLUT and raw-texture path were connected.

This follow-up stays inside the PSX FastSprite packet consumer. It does not
set a Win text color and does not restore native subtitle fallback.

## Evidence

- Gameplay script text uses `80023E10 -> 8001C6A0 -> 8001B954 -> 8003FA20`.
- The common glyph packet word2 remains `0x6701B5CC`; command bit 0 means raw
  texture, so `CC B5 01` is packet RGB data but not a renderer tint.
- The glyph texture demand is still `tpage=0x003E` normalized to atlas key
  `0x001E`, CLUT CBA `0x7810`.
- Read-only scan of `S1/COMPO01.INT` entry `EU1_256.TIM`:
  - image `(896,256,64,75)`, tpage key `0x001E`
  - CLUT `(256,480,16,1)`, CBA `0x7810`
  - row values are grayscale and have no STP bit set:
    `7FFF 7FFF 77BD 6F7B 6739 5EF7 56B5 4E73 4631 39CE 318C 294A 2108 18C6 0000 0800`

## Code decision

`SubmitFastSpritePacket8003FA20(...)` no longer treats the primitive
semi-transparent bit alone as enough to alpha-blend every textured glyph
pixel. The Stage1 VRAM atlas now exposes whether a known CLUT row contains
STP bits:

- known CLUT row with no STP bits: draw opaque even when packet word2 has the
  primitive semi-transparent bit;
- known CLUT row with STP bits: keep the existing ABR blend path;
- unknown CLUT row: preserve the previous conservative behavior instead of
  authorizing a new opaque result.

For `8001B954` gameplay glyphs this makes the `0x7810` grayscale CLUT render
as texture color rather than half-blending with the caption box / scene
background.

## Boundary

This is not a hard-coded subtitle color. The color still comes from the PSX
glyph TIM and CLUT row. The change only corrects the FastSprite texture
semi-transparency gate at the renderer adapter boundary.
