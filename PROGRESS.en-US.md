# PaRappaWin Port Progress

[README](README.md) | [中文](PROGRESS.zh-CN.md) | [Public Boundary](PUBLIC_BOUNDARY.md)

![Stage1 overlay](https://img.shields.io/badge/Stage1_overlay-96%25-brightgreen?style=flat-square)
![Stage1 gameplay](https://img.shields.io/badge/Stage1_gameplay-100%25-brightgreen?style=flat-square)
![S0 direct runtime](https://img.shields.io/badge/S0_direct_runtime-70%25-yellowgreen?style=flat-square)
![Full build source](https://img.shields.io/badge/full_build_source-not_complete-lightgrey?style=flat-square)

> Public progress display and curated source snapshot for the private
> PaRappaWin port workspace.
>
> This repository does **not** currently include every source file required for
> a clean public build. Full entrypoint/build orchestration, S0/SS0 work, and
> later-stage source are still held in the private workspace. They will be
> reviewed and pushed gradually after the migration reaches a safe completion
> boundary.

## Progress Snapshot

Percentages describe current private-workspace port maturity, not the amount of
source currently published in this repository.

| Module | Progress | Status | Notes |
|---|---:|---|---|
| Stage1 overlay port | 96% | Near complete | The Stage1 overlay layer is mostly migrated. Remaining work is long-tail parity, cleanup, public boundary review, and packaging. |
| Stage1 core gameplay | 100% | Complete | Core gameplay behavior is considered complete in the private workspace under the current criteria. |
| S0 / SS0 direct runtime | 70% | Active, not cut over | Title, main menu, stage-select, practice, options, card/replay, and related paths have substantial direct-runtime evidence and implementation slices. Card/save/replay authority, `word_800916F0`, visible/audio parity, and final runtime cutover remain open. |
| Standalone public build | Not complete | Intentionally held back | This is a curated snapshot, not a full private-worktree mirror. Some compile entrypoints and scene orchestration sources are not public yet. |

## Published Now

- Stage1 source, docs, and helper tools selected for public release.
- Windows low-level adaptation code: platform, media, decoder, and PSX-adapter
  source selected for public release.
- `bin/config.ini`.
- Stage1 subtitle data.
- Stage1 HD texture mapping tables, without image assets.
- Public boundary, roadmap, and sync file lists.

## Not Published Yet

- The complete set of source files required for a clean build.
- S0/SS0 implementation, verification tools, and private evidence.
- Stage2 and later stage content.
- Images, screenshots, binaries, videos, PDB/IDA files, logs, save data, and
  proprietary game assets.
- Private entrypoint, build orchestration, and full scene scheduling code while
  they still reference unreleased boundaries.

## Publication Plan

1. Continue closing Stage1 long-tail parity and release-boundary work.
2. Keep S0/SS0 private until card/save/replay, F0, visible/audio parity, and
   cutover blockers are closed.
3. After the migration reaches the completion boundary, publish
   compile-required source in curated, reviewable batches instead of mirroring
   the private workspace directly.
