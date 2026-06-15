# Stage1 `801C7560` timecode source window

Scope: Stage1 full-song row/writeback only. This note does not authorize
lower-CD, Rail/GTE, case17, RGB, MenuHelp/StageClear, boot, renderer,
lifecycle, terminal, or no-fail changes.

## Static source

Current IDA UI session could not provide a clean `801C7560` function boundary:
`docs/stage1_decomp/ida_export_801c7560_timecode_source_20260515.txt`
shows the address folded into a bad `sub_801C74E4` raw-byte range. That export
is retained as negative evidence for the current IDB state, not as Hex-Rays
authority.

Offline disassembly of
`E:\game\PSgame\parappa the rapper\S1\COMOD1.BIN` with overlay base
`0x801C3870` gives the useful narrow window:

- `801C7C94/801C7C98`: loads `a0 = 0x801C7560`.
- `801C7CA0`: calls `800357D4(a0)` after writing `ctx+0x0C` from
  `sceneEntry+356`.
- `801C7560`: calls `80026ECC`, `8001A3C8`, `8001A280`, then
  `8001A7A4(*(sceneEntry+0xC4))`.
- `801C75A4..801C7634`: compares `dword_801D303C` with the `8001A7A4`
  return; if the read value advanced, computes the rounded XA-derived tick and
  writes `0x801C364C` at `801C7634`; otherwise adds `sceneEntry+352` to the
  prior `0x801C364C`.
- `801C76B4`: writes `dword_801D303C = current 8001A7A4 return`.

This matches the existing direct helper shape:
`PrStageRunnerDirectUpdateTimecode801C7560(...)` consumes
`sceneEntry+348/+352/+356`, previous `dword_801D303C`, and an
`8001A7A4`-equivalent read value.

## Recorder facts

Recording:
`E:\game\PSgame\parappa the rapper\duckstation\bin\x64\标题画面-menu-选关选stage1-good通关-保存记忆卡（过场动画均为跳过）.psxrec`

The recording has `read_log_enabled=False`, so `trace_origin` can identify the
writer PC for `0x801C364C` but cannot recover the write operands. It reports
`from ?` for the source chain.

| frame | `0x801C364C` | writer | `ctx+0x38` | `ED08` |
|---:|---:|---|---:|---:|
| 3905 | `0x1249` / 4681 | `0x801C7634` | 13 | `0x000E` |
| 3995 | `0x1353` / 4947 | `0x801C7634` | 13 | `0x000E` |
| 4255 | `0x1650` / 5712 | `0x801C7634` | 15 | `0x000E` |
| 4806 | `0x1CA5` / 7333 | `0x801C7634` | 20 | `0x000A` |

The `ctx+0x38` writer is `0x80025108`; the `ED08` writer is `0x80025194`.
For the 3995/4255 target windows those are earlier bucket-boundary writes,
not same-frame writes.

## Win observable added

`src/main.cpp` now prints the existing `RunnerTimecode801C7560Runtime` fields
in `stage1ovl handoff` and `stage1ovl handoffhistory`:

- `timecodeKnown`
- `tcBaseK`, `tcBase75`
- `tcPrevXa`, `tcXa`, `tcFallback`
- `tc348`, `tc352`, `tc356`
- `tcProduct`, `tcRound`, `tcCbRet`

`test_dispatcher.py` parses these into row/writeback summaries and prints
`full_song.rowWritebackLastTimecode=...`.

This is observer-only. It does not change the direct runtime, does not
synthesize success, and does not derive `8001A7A4` from STR/ISO/file payload.

## Direct source wiring

`src/pr/pr_stage1_scorer_host.cpp` no longer treats
`Xa1Player::GetCdClockSectorIndex()` as the `8001A7A4` authority for
`801C7560`.

The runtime source is now:

```text
ctx.stage1XaCdDirect byte_800493F4
-> PrStage1XaCdDirectProbeStreamClockProducer800493F4
-> BuildStreamClockProducerCarrier800493F4
-> 80036A78(byte_800493F4)
-> clockLba - sceneEntry+0xC4 row3 timeBase
-> PrStageRunnerDirectUpdateTimecode801C7560 xaSectorReadValueA7A4
```

`src/pr/pr_scenes.cpp` builds the extra `sceneEntry+0xC4` input from Stage1
scene loader row 3 (`sceneEntry+0x9C`, `STAGE1.XA1`) after the existing
`801C4780/8001A324` scan has produced `row.timeBaseA1Plus40`.

If either the row3 timeBase or the `byte_800493F4` carrier is missing,
`ResolveStage1TimecodeXaSector801C7560(...)` returns false and the direct
`801C7560` helper keeps the previous `dword_801D303C` read value, which routes
through the existing fallback path. This leaves the gap observable; it does not
authorize success from STR playback, ISO/file readability, payload bytes,
renderer state, lifecycle gates, terminal state, or no-fail.

## Current conclusion

The build after explicit clock-source wiring passed, but runtime verification
shows the clock carrier still does not reach `801C7560` in the main path.

Artifact:
`artifacts/stage1_rail_probe_runtime/20260515_205221_stage1_rail_full_song_runtime/summary.txt`

Key facts:

- `max_query_frame=4806`, so the run covered the target PSX window.
- `full_song.rowWritebackLastTimecode` reports:
  `known=True baseK=False xaK=False timeBaseGap=False clockGap=True
  prevXa=0 xa=0 fallback=True tc348=10560 tc352=3 tc356=96 product=0
  round=0 cbRet=288`.
- Therefore Stage1 row3 `sceneEntry+0xC4` / `timeBaseA1Plus40` is available,
  but `ctx.stage1XaCdDirect` has no accepted `byte_800493F4` carrier at the
  scorer call site.
- The next source window is not `Xa1Player`, renderer, lifecycle, terminal, or
  no-fail. It is the runtime path that should execute
  `8001A3C8/8001A210 -> byte_800493F4` into `ctx.stage1XaCdDirect` before
  `801C7560`.

## Current direct-port update

`RunStage1DirectFrame7A60(...)` now executes the existing direct
`PrStage1XaCdDirectApplySub8001A3C8ClockPollFromLowerState(...)` before
`ResolveStage1TimecodeXaSector801C7560(...)`.

This matches the static `801C7560` call order: the clock poll runs before the
`8001A7A4` read-value query. The update does not add a new carrier or infer
success; it only lets the existing `8001A3C8` direct path consume already
available lower-CD facts and, when accepted, write `byte_800493F4` through the
existing `ApplyStreamClockFeedback800493F4(...)` authority.

The handoff logs now also expose the narrow clock path:

- `tcPollA3C8`, `tcPollRet`, `tcPollAccept`, `tcPollGapD0`, `tcPollGapA4`
- `xaCdF4K`, `xaCdF4M/S/F`, `xaCdF4Producer`
- `xaCdClockK`, `xaCdClockLba`, `xaCdClockGap`
- `xaCdA210Dispatch`, `xaCdSync37070Gap`, `xaCdAsync37070Gap`
- `xaCdEvtSerial`, `xaCdEvtDispatched`, `xaCdRet36AF8K/Ret36AF8`
- `xaCdCb570F8K/Cb570F8`

Next runtime check after the next build should read
`full_song.rowWritebackLastTimecode` directly:

- `pollA3C8=1`, `pollAccept=1`, `xaCdF4K=True`, `xaCdClockK=True` means the
  `8001A3C8 -> byte_800493F4 -> 8001A7A4` source reached `801C7560`.
- `pollA3C8=1` with `pollGapD0` or `pollGapA4` means the gap is still lower-CD
  feedback availability, not the timecode source calculation.
- `xaCdF4K=True` with `xaCdClockK=False` means the remaining gap is the
  `80036A78` MSF-to-LBA carrier.

Additional recorder authority for the target PSX frames is recorded in:

`docs/stage1_decomp/stage1_timecode_800493f4_target_frames_20260515.md`

At frames `3905/3995/4255/4806`, the PSX values are respectively
`28:03:01`, `28:04:39`, `28:08:65`, and `28:18:06` for
`byte_800493F4/F5/F6`; writer windows show `8001A454` writing F5 at second
rollovers and `8001A45C` writing F6 on the target frames.

The same target frames also show `801C757C -> 8001A3C8`, then
`8001A3D4 -> 800364D0` and `8001A41C -> 800363A4`. Therefore the next Win
runtime check must require explicit `tcPollGapD0=false` and
`tcPollGapA4=false` before accepting the clock. The PSX status banks are
listed in the target-frame evidence file.

## CD command authority follow-up

New narrow export:

`docs/stage1_decomp/ida_export_timecode_cd_command_window_8001a3b8_8001a3c8_8001a750_80036678_800375bc_20260515.json`

Summary:

- `8001A3B8` is read-only for `dword_80049428`; it does not call a CD command
  wrapper and cannot produce `byte_80057119`.
- `8001A3C8` is the `byte_800493F4/F5/F6` writer, but it consumes
  `800364D0` and `800363A4`; it is not the `byte_80057119` producer.
- `8001A750 -> 80036678(1,0) -> 800375BC(1,0,0,1)` only proves command `1`
  side effects.
- The current `byte_80057119=0x10` target-frame fact still needs its real
  pre-`8001A3C8` writer. The next window is either a static
  `800375BC(0x10, ...)` caller or a live writer hit on `0x80057119`.

Detailed note:

`docs/stage1_decomp/stage1_timecode_cd_command_authority_20260515.md`
