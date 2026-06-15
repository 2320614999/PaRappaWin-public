# Stage1 FastSprite draw-env offset: 80040B84 -> 800401AC

## Trigger

User visual check at 1920x1080 showed gameplay lyrics visible but at the wrong
screen position/color, while Movie1 subtitles were still absent. This pass only
closes the PSX coordinate/offset seam; if color remains wrong after retest, it
is a separate RGB/source-authority issue.

## Boundary

- Movie1 STR pixels are still a Win HAL video surface
  (`StrPlayer::RenderToRect(...)`). That is the platform video backend, not the
  subtitle/text authority.
- Movie1 UI/template draw is intended to come from the PSX draw plan.
- Movie1 subtitles must come from the PSX FastSprite chain:
  `801C77C0/8001EC54 -> 8001DB00 -> 8001B730(24,184,0) ->
  8001B954 -> 8003FA20`.
- No native text fallback is used as authority for Stage1 Movie1 text.

## Evidence

- IDA export `ida_export_boot_8001c470_callees_80040ae4_80040b84_8001b1b0_800446a0_800354c0_8001c1e8_20260513.txt`
  shows `80040B84` setting `word_800901C4 = dword_800917FC / 2` and
  `word_800901C6 = dword_8009182C / 2`, then calling `800401AC`.
- `800401AC` has the `word_800965A0 != 0` branch used by Movie1 graph owner:
  packet offset globals `word_800917AA/AC` stay zero, while draw-env offsets
  `word_80091738/3A` receive the screen-center based offset.
- `8001B954` writes FastSprite packet-local glyph coordinates and relies on the
  graph/draw environment to place them on the PSX screen.

## Code Change

- `PrPsxGraphOwnerDirect::PsxCall80040B84_ApplyScreenCenterAndDrawOffset(...)`
  now models the PSX screen-center setup before `800401AC`.
- FastSprite runtime and scene-submit packet commands carry the known
  `800401AC` draw-env offset.
- `SubmitFastSpritePacket8003FA20(...)` applies that offset when mapping PSX
  FastSprite packet coordinates to the Win viewport.
- Movie1 graph owner initialization now calls the `80040B84 -> 800401AC` helper.

## Verification

- Full `.\build.ps1` passed.
- Output:
  `E:\game\PSgame\parappa the rapper\PaRappaWin.exe`.
