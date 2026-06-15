# Stage1 fail terminal cleanup gate analysis 2026-05-25

## Scope

This file accompanies `ida_export_fail_terminal_cleanup_gate_20260525.md`. It records that the missing local IDA export has been produced for the fail terminal cleanup gate line. Runtime code is unchanged.

## Functions Exported

- `8001A7F8`
- `8001A3B8`
- `80015350`
- `8001A4A4`
- `80026EF8`
- `80026FA4`
- `800357D4`
- `8001A694`

## Review Target

Use the export to close the return facts for `8001A7F8(a1)` and `8001A3B8()`, then map those facts into Win `continuationGate1A7F8Known` / `frameExit1A3B8Known` without guessing a runner/XA/audio freeze.

## First-Pass Findings

- `8001A7F8(a1)` is now function-body evidence, not just a callsite. Hex-Rays prints `*(a1+0x2C) + *(a1+8) <= 0`, but the disassembly loads `dword_80049404` before `slt`; treat the precise condition as a comparison between `*(a1+0x2C) + *(a1+8)` and `dword_80049404`, with the current decompile collapsing that global to zero.
- `8001A3B8()` is named `GsGetWorkBase`. Hex-Rays collapses it to `return 0`, but the disassembly clearly returns `dword_80049428`. This is the return fact Win currently lacks when it passes `frameExit1A3B8Known=false`.
- `80015350` decrements `word_80091824` by the incoming `$a1` value while `word_80091824 > 0`; it is a small countdown helper, not a broad runner/audio driver by itself.
- `8001A4A4(a1)` forwards to `8001A478(a1 != 1 ? 0x7F : 0)`. The fail terminal callsite `8001A4A4(1)` therefore calls `8001A478(0)`.
- `80026EF8(ptr)` reads `ptr[0]`, `ptr[1]`, sets `ptr[2] = ptr[1] + 24`, calls `80034240(word_800943A8, ptr[0], ptr[1], ptr[2], 0, ptr[3], ptr[3])`, and stores the result in `word_800943AC`.
- `80026FA4()` is a direct `800351B8(0)` wrapper.
- `800357D4(a1)` is `VSyncCallback`; it calls the callback from `off_80057000 + 0x14` as `callback(0, a1)`.
- `8001A694()` waits until `800367A4(8, 0, 80049414)` succeeds, then calls `80036510(0)`.

## Win Mismatch Boundary

- The earlier gap is now narrower: Win has action-trace slots for `8001A7F8` and `8001A3B8`, but the source facts should be mapped from `a1+0x08`, `a1+0x2C`, `dword_80049404`, and `dword_80049428`, not guessed from visual fail timing.
- This export still does not justify a generic `runner.Update` freeze, XA poll freeze, or host audio mute. Any runtime patch should first wire these return facts into `ResolveStage1RunnerMainLoopTail7A60` and only then re-evaluate the retry/cleanup boundary.

## Patch Status

No runtime patch in this export pass.
