# PaRappaWin Public Progress

[中文进度](PROGRESS.zh-CN.md) | [English Progress](PROGRESS.en-US.md) | [Status](STATUS.md) | [Boundary](PUBLIC_BOUNDARY.md)

![stage1 overlay](https://img.shields.io/badge/Stage1_overlay-96%25-brightgreen?style=flat-square)
![stage1 gameplay](https://img.shields.io/badge/Stage1_gameplay-100%25-brightgreen?style=flat-square)
![s0 direct runtime](https://img.shields.io/badge/S0_direct_runtime-70%25-yellowgreen?style=flat-square)
![build source](https://img.shields.io/badge/full_build_source-not_complete-lightgrey?style=flat-square)
![assets](https://img.shields.io/badge/assets-not_distributed-lightgrey?style=flat-square)

> Public progress and curated source snapshot for the PaRappaWin preservation
> and Windows-port effort.
>
> This repository does **not** currently contain every source file required for
> a clean public build. The remaining build/orchestration code and held-back
> stage sources will be published gradually after the migration reaches a safe
> completion boundary.

---

## Progress Snapshot

Percentages describe current private-workspace port maturity, not the amount of
source currently published in this repository.

| Area | Progress | Status | Notes |
|---|---:|---|---|
| Stage1 overlay port | 96% | Near complete | The Stage1 overlay layer is mostly migrated. Remaining work is long-tail parity, cleanup, and release packaging. |
| Stage1 core gameplay | 100% | Gameplay complete | Core Stage1 gameplay behavior is considered complete in the private workspace. |
| S0 / SS0 direct runtime | 70% | Active, not cut over | Title/menu/stage-select/practice/options/card/replay surfaces have substantial direct-runtime evidence, but card/save/replay authority, F0, visible/audio parity, and final cutover are still open. |
| Public build completeness | Not complete | Held back | The public repo intentionally omits some compile-required entrypoint, orchestration, S0/SS0, and later-stage source files. |

## Current Public Boundary

- Published: Stage1-facing source, selected Windows adaptation layer code,
  public notes, helper tools, subtitle data, `bin/config.ini`, and HD texture
  mapping tables without image assets.
- Held back: S0/SS0 implementation, Stage2 and later stages, images, binaries,
  screenshots, logs, private scratch artifacts, and private build entrypoints
  that still reference unreleased boundaries.
- Empty placeholder directories may exist where private directories exist but
  their contents are not part of the current public boundary.

## Documents

- [PROGRESS.zh-CN.md](PROGRESS.zh-CN.md) - Chinese progress display.
- [PROGRESS.en-US.md](PROGRESS.en-US.md) - English progress display.
- [STATUS.md](STATUS.md) - current public snapshot status.
- [ROADMAP.md](ROADMAP.md) - public release roadmap.
- [PUBLIC_BOUNDARY.md](PUBLIC_BOUNDARY.md) - what can and cannot be published.
- [PUBLIC_SYNC_FILELIST.txt](PUBLIC_SYNC_FILELIST.txt) - files included in this
  curated snapshot.
- [PUBLIC_WIN_LAYER_FILELIST.txt](PUBLIC_WIN_LAYER_FILELIST.txt) - Windows
  adaptation layer files currently included.

## Not Included

- Full clean-build source tree.
- Private development logs, runtime dumps, and temporary research artifacts.
- S0/SS0 source files under the current public boundary.
- Stage2 or later stage content.
- Images, screenshots, binaries, videos, PDB/IDA files, save data, or
  proprietary game assets.

## License

No open-source license has been declared yet. Until a license is added, all
rights are reserved by default.
