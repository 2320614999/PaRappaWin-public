# Stage1 script box `80023E10` priority consumer fix

## Trigger

User retest at 1920x1080 showed gameplay subtitle glyphs present but visually
under the subtitle frame, with wrong-looking color. Movie1 video subtitles were
still absent and Movie1 subtitle-frame top corners still looked wrong.

## PSX authority

IDA export for `80023E10` shows the steady gameplay script box call order:

- `8001C4EC(10,199,8,0x12,0x400F0F0F,2)`
- `8001C4EC(18,191,0x11C,0x22,0x400F0F0F,2)`
- `8001C4EC(302,199,8,0x12,0x400F0F0F,2)`
- four `8001C550(..., template, 2)` corner submits
- `8001B730(28,194,0)`
- `8001C6A0(text,480)` -> `8001B954` -> `8003FA20`, priority `0`

The frame/corners therefore carry PSX priority `2`, while glyph FastSprite
packets carry priority `0`.

## Win gap

`DrawStage1ScriptBoxCommand80023E10(...)` consumed the direct frame carrier but
overrode the PSX priority with hardcoded renderer layer `920` for the fill
rectangles and corner templates. The common FastSprite consumer maps priority
`0` glyph packets to layer `784`; the renderer queue draws higher layers later,
so the hardcoded frame layer could cover the glyphs.

## Code change

`src/pr/pr_stage_scene_submit_backend.cpp` now maps script-box rects and corners
through `Stage1FastSpritePacketLayer8003FA20(priority)` and keeps local submit
order in the PSX call order. No native text fallback, renderer default color, or
Movie1 frame workaround was added.

After retest the subtitle frame disappeared because the common renderer mapping
had no explicit `priority=2` case, so `80023E10` fell to default layer `779`.
The mapping now includes `priority=2 -> layer 781`, keeping glyph priority `0`
above the frame while leaving priority `3`/default scene packets below it.

## Remaining boundary

- Gameplay glyph color still needs retest after the frame no longer covers the
  glyphs. If it remains wrong, continue from `8001B954` RGB/source authority.
- Movie1 video subtitles remain on the separate
  `801C77C0/8001EC54 -> 8001DB00 -> 8001B730 -> 8001B954 -> 8003FA20` chain.
- Movie1 subtitle-frame top-corner texture issues remain a template/CLUT
  evidence task, not a native UI fallback task.

## Verification

Full `.\build.ps1` passed on 2026-05-18. Output:
`E:\game\PSgame\parappa the rapper\PaRappaWin.exe`.

Full `.\build.ps1` passed again after adding the explicit priority-2 renderer
mapping. Output:
`E:\game\PSgame\parappa the rapper\PaRappaWin.exe`.
