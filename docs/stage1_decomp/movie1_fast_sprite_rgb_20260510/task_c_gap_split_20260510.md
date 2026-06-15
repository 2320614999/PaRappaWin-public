# Task C Movie1 raw RGB / transition gap split - 2026-05-10

Scope: static recheck only. No build, no tests, no simulator, no fallback.

## Recheck result

- `8001B590/8001B25C` still only prove the local FastSprite prefix: attr, X/Y, W/H, tpage, U/V, CLUT.
- `8003FA20` still reads local bytes `+0x14/+0x15/+0x16` as RGB, but no current export in this folder proves a producer write before that read.
- `801C448C/801C4350` add movie timecode context evidence, not RGB evidence.
- `801C455C` and its caller exports confirm `801C3640` is passed through Movie1 transition/play-loop calls, but do not close a new transition ctx owner gap for raw action RGB.

## Code action

- Split the broad RGB gap carrier into raw draw, transition raw action, and text glyph categories.
- Added per-category counters to Movie1 draw-plan summaries.
- Kept the typed submit gate unchanged: `8003FA20` is only called for packet-complete local sprite input.

## Text glyph recheck

- Stage1 text glyph mainline remains `8001EC54(ctx,7) -> 8001DB00(ctx+0x10C, work) -> 8001B954(text,480,work) -> 8003FA20`.
- The work/OT owner for that path is known through the current `gp+872` slot (`0x80087288 + 20 * slot`), so this is no longer an owner-missing gap.
- The unresolved part is narrower: `8001B954` stack-local FastSprite bytes `+0x14/+0x15/+0x16` are read by `8003FA20` as RGB, but no `8001B954`/caller export proves writes to those stack bytes.
- `8001B744` is the closest same-family glyph comparator and also does not prove stack-local RGB writes.
- `8001BCA0` uses global object `dword_8007CED0`; that white/global-object evidence only applies to the global FastSprite path and is explicitly not portable to the `8001B954` / `8001B744` stack-local glyph path.
- Code now carries `Movie1TextGlyphRgbGapEvidenceSub8001B954` on glyph commands/sequence, pinned to `8001DB00 -> 8001B954 -> 8003FA20`, with `dword8007CED0AppliesToThisStackLocal=false`.

## Still blocked

- Do not use default RGB, zero-initialized stack bytes, template bytes, renderer tint, or gameplay/scene submit fallback.
- Next evidence step is still an authoritative producer for local FastSprite RGB bytes `+0x14/+0x15/+0x16`, or a PSX-grounded decision that these bytes are intentionally uninitialized for the callsite.
