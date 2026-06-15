# Stage1 script box packet fill/corner - 2026-05-18

## Scope

Gameplay script box from `80023E10`, after the user reported wrong text color
and visible color differences at the four corners.

## PSX source path

Static decompile for `80023E10` shows the frame is not a renderer-native box:

- `80023E48 -> 8001C4EC(10,199,8,0x12,0x400F0F0F,2)`
- `80023E64 -> 8001C4EC(18,191,0x11C,0x22,0x400F0F0F,2)`
- `80023E80 -> 8001C4EC(302,199,8,0x12,0x400F0F0F,2)`
- `80023E98 -> 8001C550(10,191,dword_80050900,2)`
- `80023EAC -> 8001C550(10,217,dword_800508F0,2)`
- `80023EC0 -> 8001C550(302,191,dword_800508E0,2)`
- `80023ED4 -> 8001C550(302,217,dword_800508D0,2)`
- `80023EE8 -> 8001B730(28,194,0)`
- `80023EF4 -> 8001C6A0(a1,480)`

`8001C4EC` tails through `8001B6C4 -> 8003EE84` and writes a five-word
box-fill packet. The previous Win-side path drew these rects directly in the
renderer, after FastSprite packets, with a hard-coded alpha. That split the
rect body from the PSX ordering table path used by corners and glyphs.

Recorder evidence from v3 frame 1780 confirms the corner path goes through
`8001C550 -> 8001B590 -> 8003FA20`. The four corner packet word2 writes at
`8003FB10` were `0x671D7B1C`; the consumed `8001B590` local RGB tail was
`1C 7B 1D`, last written by `801CA644` into the stack-local window before
`8003FA20`.

## Code decision

- `PsxCall80023E10_801CBFDC` now applies the rect calls as explicit
  `8003EE84` packet writes before corners and glyphs.
- The old renderer-side script-box rect draw is disabled; visible rects now
  come from `BuildStage1BoxFillPacketCommands8003EE84` over the same
  packet mirror as the other scene-submit primitives.
- The corner templates now stay in the packet path through
  `8001C550 / 8001B590 / 8003FA20`; no white-tinted direct template fallback
  is used for gameplay script-box corners.
- `Stage1ScriptBoxCornerStackScratch8001B590` is accepted by the scene-submit
  command builder as authoritative enough to render this source-specific
  packet. Before this follow-up, the corner packets existed but were classified
  as audit-only and dropped by the backend.
- The four `80023E10` corner templates register their packet tpage/CLUT demand
  through the Stage1 VRAM atlas before TIM scanning. This keeps the source as
  PSX template metadata rather than the old Win template renderer fallback.
- Follow-up after the user's 2026-05-18 retest: the registration and runtime
  path were still using a mistranslated `80043DF4` result from
  `8001B25C`. The old local helper ORed the pre-shifted UV helper with
  `texY & 0xFF00`; PSX calls `80043DF4(0,1,a3,a4)`, which returns the GPU
  tpage word. The four script-box corner templates now resolve to the PSX
  tpage bit layout instead of tpage zero.
- FastSprite submission now reads the PSX semi-transparent bit from packet
  word2 and ABR from word1. ABR0 uses alpha blend at `0.5`, ABR1/3 use
  additive, ABR2 uses subtractive.
- FastSprite submission also respects the PSX raw-texture bit from packet
  word2. When command code bit 0 is set, RGB bytes remain packet facts but are
  not used as a Win tint. This is required for glyph packets such as
  `0x6701B5CC`; the `CC B5 01` bytes are not a semantic yellow text color in
  that raw-texture mode.

## Boundary

This is not a generic color override. The corner RGB tail is accepted only for
the observed `80023E10 -> 8001C550 -> 8001B590 -> 8003FA20` path. If gameplay
text color is still wrong after this build, the next target is the
source-specific glyph packet producer (`8001B954` or HUD `8001BEE4`) rather
than Win renderer tint or native text fallback.

## Follow-up facts

Recorder v3 frame 1780 shows the script-box sequence:

- `8255`: `801CC984 -> 80023E10`
- `8261/8319/8358`: rect calls to `8001C4EC`
- `8397/8460/8503/8547`: corner calls to `8001C550`
- `8447/8490/8534/8577`: corner packet word2 writes at `8003FB10`
- `8596 -> 8599`: `8001C6A0 -> 8001B954`

Across gameplay frames, `8001B954 / 8001BBF8` packet word2 commonly appears
as `0x6701B5CC`. The high byte `0x67` carries the raw-texture and
semi-transparent command bits; the low RGB bytes should not tint the glyph
texture in raw-texture mode.
