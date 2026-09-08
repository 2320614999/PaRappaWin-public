# Roadmap

Updated: 2026-09-08 (post-push documentation sync)

[README](README.md) | [Build](BUILDING.md) | [Status](STATUS.md) | [中文进度](PROGRESS.zh-CN.md) | [English Progress](PROGRESS.en-US.md)

## Work state

The current development round is paused at the maintainer's request after the
source publication. The items below are a resumption plan, not work already
started or a claim of new verification. This update changes documentation only.

## Published — 2026-09-08

Public source publication checkpoint: [c43ef46](https://github.com/2320614999/PaRappaWin-public/commit/c43ef463090d1c549143f45616bbf7d4321a42c5). Later documentation commits do
not change this source baseline.

- [x] Complete current build-source snapshot, including S0/SS0 and Windows additions.
- [x] Portable PowerShell build entrypoint and CMake project.
- [x] Separate public history, current status notes and publication inventories.
- [x] Recorded full `build.ps1` compilation and link of 153 product translation units; source/header/table inventory checked at 439 files. See [BUILDING.md](BUILDING.md). This is the existing publication record, not a new build in this update.

Published source and a successful build do not close the implementation tasks below.

## P0 — unblock and verify the same-process return loop

After an explicit decision to resume:

- [ ] Close the Stage1 resident directory lower-CD completion path.
- [ ] Verify Stage1 return, LOAD, high-score and progression readback in the same process. Do not replace this gate with an isolated save test or a new-process readback.

Acceptance: the native directory resource request completes and the directory
becomes usable without restarting the program or bypassing the pending
completion path. Verify LOAD, high-score and progression readback through that
same-process return. Record each result against the tested source commit;
reaching the load request alone does not close this gate.

## Remaining Stage1 acceptance

- [ ] Complete long-tail save/error/input/render/audio parity review against original semantics.
- [ ] Confirm native movie skipping with physical controllers; existing injected-PAD observations are not physical-device verification.

For physical-controller movie skipping, verify that Cross input reaches the
native skip path and ends video, subtitles and movie audio together. Record
save/error/input/render/audio checks as behavior-specific results rather than
inferring a pass from successful compilation.

## Later-stage implementation

- [ ] Port Stage2 and later stages. Existing scaffolding and successful compilation are not completion or gameplay acceptance.

Record working entry, gameplay and exit flows, with unresolved behavior listed
per stage. Distinguish implementation presence, buildability, playable flows
and parity verification.

## Build verification after resumption

- [ ] Record a full CMake product build before claiming that build path is verified; the current publication record only validates `build.ps1`.
- [ ] Repeat full-build validation for future source checkpoints, documenting the source commit, toolchain and exact verification scope.

## Future publications

- Publish reviewed source checkpoints without private research artifacts or private commit ancestry.
- Retain Windows-specific features rather than rolling back to an older implementation for publication.
- Keep source completeness, full-build verification and gameplay verification explicitly separate.
- Tag a stable release only after the relevant end-to-end behavior is verified.
