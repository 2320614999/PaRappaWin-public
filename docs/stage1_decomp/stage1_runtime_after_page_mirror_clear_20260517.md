# Stage1 runtime after page-record mirror page clear (2026-05-17)

Scope: existing integration run only. No new observer, sentinel, harness, parser, or
debug-only field was added. This file records the runtime evidence after the
page-record mirror adjacent-page clear was wired.

## Command / artifact

Command:

```powershell
python .\test_dispatcher.py --stage1-rail-full-song-runtime --stage1-runtime-retries 1
```

Artifact:

```text
artifacts/stage1_rail_probe_runtime/20260517_003901_stage1_rail_full_song_runtime/
```

Summary:

- `stop_reason=timeout`
- `full_song.final_scene=1`
- `full_song.final_stageRunning=True`
- `full_song.final_sceneExitReason=0`
- `full_song.liveClearTerminalTailGate=True`
- `full_song.liveClearTailPulse=False`
- `full_song.rowWritebackReachedQueryFrame=True`
- `full_song.rowWritebackCoverageQueryFrame=4255`
- `full_song.rowWritebackLastRowWriteMaxQuery=696`
- `full_song.rowWritebackLastRowWriteReachedQueryFrame=False`
- `full_song.rowWritebackPositiveDeltaSamples=0`
- `full_song.rowWritebackV22OneSamples=0`
- `full_song.rowWritebackRowWriteSamples=677`

Marker failures in this run are not PSX authority. They only show that the
old full-song runtime marker expectations are still not a valid accepted-cadence
source.

## Delta from previous runtime artifact

Previous post-`801C7A60` artifact:

```text
artifacts/stage1_rail_probe_runtime/20260516_224538_stage1_rail_full_song_runtime/summary.txt
```

It still reported:

- `full_song.rowWritebackPositiveDeltaSamples=6`
- final row-write state kept `accumulator91816=6`, `baseline91818=6`,
  `phaseCacheED36=1`

After clearing the adjacent natural page in the page-record mirror, the new
artifact reports:

- `full_song.rowWritebackPositiveDeltaSamples=0`
- final row-write state is `accumulator91816=0`, `baseline91818=0`,
  `phaseCacheED36=0`

Conclusion: those previous positive deltas were not valid scorer convergence.
They were compatible with stale four-page ring history leaking into the direct
`80014D58` reader/anchor path. The clear fixed that contamination, but also
exposed that the current runtime still does not produce the PSX first-beat
page history.

## Key sampled state

Representative q767 row3 sample:

- `tick96=4593`
- `currentPageOrdinal1Based=12`
- reader page `gPage=11`
- `gReq=0x10`
- `gUnion=0x20`
- `gAcc=3`
- anchor slot `8`
- required class `1`
- anchor class `0`
- anchor occupied `0`
- anchor match `0`
- `gCommit=-1`
- `gWb=0`
- clamp active
- accepted seed query `756`
- seed control `0x20`
- seed class `2`
- accepted tick `4534`
- `recordSlot24=13`
- `recordRem24=6`
- write page `11`
- `lastWriteAvailable=0`

This does not match the PSX first-beat authority window, which needs page
history for `0x10/class1` before the second-beat `0x20/class2` path can drive
`14D58 -> 14D28 -> 24F8C` to the `9/9 -> 18/18 -> v22=1` sequence.

Later samples continue to show the same shape:

- q956: `gReq=0x40`, `gUnion=0x20`, anchor empty, `gCommit=-2`;
  accepted seed is still `0x20/class2`, and the last materialized write is
  page 14 slot 9 mask `0x20`.
- q1433: `gReq=0x04`, `gUnion=0x20`, `gAcc=2`, anchor empty,
  `gCommit=-1`; accepted seed remains `0x20/class2`.

## Boundary conclusion

The page-record mirror fix removed stale page-history contamination. The
remaining blocker is not row/v22 table math, coverage/slot14, or scorer
resolver cleanup. The remaining blocker is the real accepted page-history
production/source: the current full-song runtime driver is still Circle-biased
(`0x20/class2`) and cannot be used as PSX first-beat `0x10/class1` authority.

Do not derive success from renderer markers, lifecycle clear, default values,
Win duration, F5/no-fail, or the old full-song marker set. The next valid code
target must be backed by IDA/recorder/live evidence for the accepted input/page
history source, or by an already translated runtime source that naturally
delivers `0x10/class1 -> 0x20/class2` into `80014614/80014D58`.
