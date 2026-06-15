# Stage1 text glyph VRAM requirement (`8001B954 -> 8003FA20`)

## Conclusion

The `8001B954` glyph FastSprite path now has RGB authority and packet authority, but the renderer can still drop the packet if the Stage1 VRAM atlas never loads the glyph TIM page.  The required atlas demand is sourceable from the PSX packet fields:

- raw tpage from `8001B954 -> 80043DF4(0,1,a3,a4)`: `0x003E`
- atlas-normalized tpage key: `0x001E`
- glyph CLUT: `clutX=256`, `clutY=480`, CBA `0x7810`

This is a GPU/VRAM HAL facts-source registration.  It does not use native text fallback, a Win-side texture name, renderer tint, or payload bytes as text authority.

## Evidence

- `src/pr/pr_stage1_movie_text_direct.cpp` keeps the direct glyph tpage formula in `ResolveGlyphTpageSub80043DF4_Mode0_4bppSub8001B954(...)`.
- `src/pr/pr_stage1_movie_text_direct.h` defines `kMovie1TextGlyphFastSpriteClutXSub8001B954 = 256`.
- `Movie1TextDisplayActionSub8001EC54::textDrawScaleSub8001DB00` is initialized to `480`, and script-box `8001B954` also feeds the same glyph scale path.
- A local read-only TIM header scan of `E:\game\PSgame\parappa the rapper\S1\COMPO01.INT` found the matching 4-bit page at offset `0x3330`:
  - CLUT `(256,480,16,1)`, CBA `0x7810`
  - image `(896,256,64,75)`
  - TIM-derived tpage `0x001E`

## Code Connection

`LoadStage1Resources(...)` now registers the `8001B954` glyph tpage/CLUT demand before scanning Stage1 TIM resources.  Existing `PsxVramAtlas::LoadTim(...)` then loads the matching TIM page through the same PSX VRAM atlas path used by other direct packets.
