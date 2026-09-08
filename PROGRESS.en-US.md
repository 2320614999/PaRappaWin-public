# PaRappaWin Port Progress

Updated: 2026-09-08 (post-push documentation sync)

[README](README.md) | [中文](PROGRESS.zh-CN.md) | [Build](BUILDING.md) | [Status](STATUS.md) | [Roadmap](ROADMAP.md)

## Current state

**The complete current build-source set is public and a successful full public build is recorded. This development round is paused at the maintainer's request, pending an explicit decision to resume.**

The public source publication checkpoint is `c43ef463090d1c549143f45616bbf7d4321a42c5` ([c43ef46](https://github.com/2320614999/PaRappaWin-public/commit/c43ef463090d1c549143f45616bbf7d4321a42c5)), confirmed on remote `main` before this documentation update. This identifies the source publication, not the HEAD of later documentation commits.

Windows-specific optimizations and additions are retained; publication did not roll the game implementation back to an older version. This follow-up changes documentation only, not runtime source or build scripts, and does not rebuild or run the game.

## Completed publication work

- The complete current build-source set is public, including the Windows entrypoint and enhancements, S0/SS0, Stage1, existing later-stage scaffolding, headers, tables and build scripts.
- The existing publication verification records 439 source/header/table files matching the development checkpoint byte for byte, with no missing or untracked quoted-include dependencies.
- The 2026-09-08 `build.ps1` full compilation and link are recorded as passing: 153 product C++ translation units, MSVC 14.44.35207 x64 and Windows SDK 10.0.19041.0. See [BUILDING.md](BUILDING.md).
- The public repository retains its separate history, without private Git ancestry or the research workspace. Completed publication does not mean a completed game port.

## Runtime progress and open boundaries

| Area | Existing record | Still open |
| --- | --- | --- |
| S0 / SS0 / Stage1 | Implementations are included in the current source snapshot. | Original-behavior parity is not complete. |
| Stage1 movie skip | Native Cross-skip was observed ending video, subtitles and movie audio together through injected PAD input. | Physical-controller verification remains outstanding. |
| Stage1 directory return | The native directory resource request starts. | Lower CD completion feedback still stalls; same-process return, LOAD, high-score and progression readback are not closed. |
| Save and readback | Earlier isolated save/readback observations exist. | They do not prove the same-process return/readback loop; save, error handling, input, rendering and audio still have unverified branches. |
| Stage2 and later | Existing scaffolding is included in the public source. | These are not completed ports; compiling the scaffolding does not establish playable stages. |

Source completeness, a successful full build, limited runtime observations and end-to-end port acceptance are separate claims. This update assigns no new completion percentages and does not use historical percentages as proof of current closure.

## Priorities after resumption

First repair the lower CD completion path for Stage1's directory return, then verify return, LOAD, high-score and progression readback in the same process. A new-process readback is not a substitute. Continue the remaining parity review and physical-controller verification afterward. See [ROADMAP.md](ROADMAP.md). No new implementation round starts automatically while work is paused.

## Verification and publication boundaries

The build and runtime results above refer to existing records, not new Windows execution in this documentation follow-up. The recorded publication build did not run the game or a runtime probe and did not claim a CMake build.

| Check | Recorded state | Scope |
| --- | --- | --- |
| Source and dependencies | Existing publication check | 439 source/header/table files match; zero missing or untracked quoted-include dependencies. |
| PowerShell full compile and link | Existing pass | 153 product translation units; not a gameplay or all-tests pass. |
| CMake build | Not run in publication validation | A project and instructions exist; that build path is not claimed as verified. |
| Game/runtime probes during publication build | Not run | Compilation cannot substitute for runtime acceptance. |
| This documentation update | No new execution results | Runtime observation limits and unresolved work remain in place. |

Use a full rebuild, not incremental verification. Users must supply their own lawfully held runtime game data. No additional game media, personal saves, private configuration or research captures are published in this update.

See [STATUS.md](STATUS.md) and [PUBLIC_BOUNDARY.md](PUBLIC_BOUNDARY.md). Boundaries for previously published assets remain unchanged; see [THIRD_PARTY.md](THIRD_PARTY.md) and [NOTICE](NOTICE).
