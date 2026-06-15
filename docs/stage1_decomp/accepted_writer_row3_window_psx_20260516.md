## Stage1 accepted writer: PSX row3 late-window truth

Recording:
- `E:\game\PSgame\parappa the rapper\duckstation\stage1二周目_无cool_good评级通关3.psxrec`

New artifact:
- `artifacts/psx_row3_writer_window_3368_3384.json`

Window:
- `frame 3368..3384`
- `ctx base = 0x801C3640`
- `ctx+0x18 = 0x801C3658`
- `ctx+0x20 = 0x801C3660`
- `ctx+0x10 = 0x801C3650`
- `row4E = 0x801C368E`
- `901C0 = 0x800901C0`
- `901BC = 0x800901BC`

Facts:
- `frame 3368..3384`
  - `ctx+0x18 == 0`
  - `ctx+0x20 == 0`
  - `ctx+0x10 == 9490`
  - `901C0 == 19`
  - `901BC == 19`
- `row4E` stays `3` until `frame 3374`, then changes `3 -> 2` at `frame 3375`.

Writer hits around the same family (`frame 3300..3400`):
- `0x801C3658`
  - `frame 3319  PC=0x801C7F24  0 -> 0x8`
  - `frame 3327  PC=0x801C7F44  0x8 -> 0`
  - `frame 3335  PC=0x801C7F24  0 -> 0x8`
  - `frame 3341  PC=0x801C7F44  0x8 -> 0`
  - `frame 3353  PC=0x801C7F24  0 -> 0x8`
  - `frame 3361  PC=0x801C7F44  0x8 -> 0`
- `0x801C3660`
  - `frame 3319  PC=0x801C7F38  0 -> 0x7`
  - `frame 3327  PC=0x801C7F48  0x7 -> 0`
  - `frame 3335  PC=0x801C7F38  0 -> 0x7`
  - `frame 3341  PC=0x801C7F48  0x7 -> 0`
  - `frame 3353  PC=0x801C7F38  0 -> 0x7`
  - `frame 3361  PC=0x801C7F48  0x7 -> 0`

Conclusion:
- PSX row3 late window does **not** carry live `ctx+0x18/+0x20 = 0x20/2`.
- The `0x8/7` family is written earlier by `FUN_801c7a60` writer beats, then cleared before the `3375 row3 -> row2` late window.
- Therefore current Win `page24 slot8/11 = 32/2` cannot be explained by the late consumer window itself.
- The next cut should move earlier to the accepted writer beat that feeds the later `page24` history, not keep chasing `14D58` late-window branch math.

Derived slot math (`halfWindow34 = 8`):
- `frame 3319`
  - `tick10 = 9389`
  - `phase384 = (9389 + 8) % 384 = 181`
  - `slot24 = 181 / 24 = 7`
  - `rem24 = 13`
  - `13 <= 16`, so this beat is recordable
  - matches PSX `page24 slot7 -> 0x8/7`
- `frame 3335`
  - `tick10 = 9436`
  - `phase384 = (9436 + 8) % 384 = 228`
  - `slot24 = 228 / 24 = 9`
  - `rem24 = 12`
  - `12 <= 16`, so this beat is recordable
  - matches PSX `page24 slot9 -> 0x8/7`
- `frame 3353`
  - `tick10 = 9490`
  - `phase384 = (9490 + 8) % 384 = 282`
  - `slot24 = 282 / 24 = 11`
  - `rem24 = 18`
  - `18 > 16`, so this beat is **not** recordable
  - explains why PSX `page24 slot11` stays empty even though `ctx+0x18/+0x20` still shows `0x8/7`

Win contrast from current runtime:
- `query 1592`
  - `acceptedTick96 = 9402`
  - `phase384 = (9402 + 8) % 384 = 194`
  - `slot24 = 194 / 24 = 8`
  - `rem24 = 2`
  - this creates the current bogus `page24 slot8 = 32/2`
- So the current Win seed tick lands exactly between the two legal PSX writer ticks
  (`9389 -> slot7`, `9436 -> slot9`) and produces a non-PSX middle slot.
