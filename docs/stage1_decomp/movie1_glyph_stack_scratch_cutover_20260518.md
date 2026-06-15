# Movie1 / Stage1 glyph FastSprite stack-scratch cutover, 2026-05-18

Scope: only the `8001B954 -> 8003FA20` glyph submit path. This does not
authorize generic FastSprite stack residue, renderer default colors, native text
fallback, or `8001B590/8001BEE4` raw-draw color.

Evidence reused:

- Current IDA `8001B954` pseudo-C builds the local FastSprite object at
  `sp+0x10` and calls `GsSortFastSprite(&v24, work, 0)` at `8001BBF8`.
- Current IDA `8003FA20` reads RGB from `a0+0x14..0x16` and writes packet
  word2.
- Existing replay evidence
  `movie1_fast_sprite_rgb_20260510/stack_residue_rgb_replay_20260510.md`
  shows the `8001B954_glyph_stack` callsite is stable for 80416 hits, with
  bytes `[204,181,1]` and last writer `8001B26C`.
- Existing static audit maps `8001B26C` to the `8001B25C` prologue
  saved-register spill. It is PSX stack-scratch behavior, not an explicit glyph
  RGB field write.

Implementation boundary:

- `PrPsxFastSpriteSubmitDirect` now has a source-specific provenance value for
  the exact `8001B954 / 8001BBF8 / 8001B26C` stack-scratch window.
- `HasKnownRgbTail8003FA20(...)` treats that specific provenance as renderable.
- Plain `PsxStackScratchResidue`, replay reconstructed residue, and
  `StaticGlobalObject` remain non-authoritative.

