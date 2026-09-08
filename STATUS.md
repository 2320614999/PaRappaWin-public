# Public Status

Last updated: 2026-09-08 (post-push documentation sync)

[README](README.md) | [中文进度](PROGRESS.zh-CN.md) | [English Progress](PROGRESS.en-US.md) | [Build](BUILDING.md) | [Roadmap](ROADMAP.md)

## Checkpoints and work state

- Source checkpoint: `e1275d39` (private development reference, for provenance; its Git ancestry is not included in this repository).
- Public source publication: `c43ef463090d1c549143f45616bbf7d4321a42c5` ([c43ef46](https://github.com/2320614999/PaRappaWin-public/commit/c43ef463090d1c549143f45616bbf7d4321a42c5)), confirmed on remote `main` before this documentation follow-up.
- Current work state: this development round is paused at the maintainer's request. Implementation resumes only after an explicit decision to continue.
- This follow-up is documentation-only: no runtime source or build-script changes, no rollback of the published implementation, and no new build or gameplay verification.

These identifiers describe the source/publication baseline, not the HEAD of subsequent documentation commits. An unresolved task remains unresolved while work is paused.

## Source availability

All 439 current source/header/table files from the working source snapshot are
published, together with both build entrypoints. The product build contains
153 C++ translation units. Windows-specific features remain in the source.
This is a reviewed snapshot in the public repository's separate history,
not a publication of the private Git history or research workspace.

The [publication inventory](PUBLIC_SYNC_FILELIST.txt) and
[Windows layer inventory](PUBLIC_WIN_LAYER_FILELIST.txt) describe the published
file scope. These inventories are not gameplay-completion checklists.

## Runtime status

- S0/SS0 and Stage1 implementations are present; parity work is not complete.
- Stage1 movie Cross-skip was observed through the native PAD logic ending
  video, subtitles and movie audio together. Physical-controller verification
  is still outstanding; the observation used injected PAD input.
- Returning from Stage1 reaches the resident directory resource-load request,
  but the lower CD completion feedback still stalls. The same-process
  directory / LOAD / high-score / progression loop is not closed.
- Earlier isolated save/readback observations do not prove that remaining
  same-process loop works. Outstanding save/error/input/render/audio parity
  branches remain open.
- Stage2 and later scaffolding is included where it exists; those stages are
  not claimed as completed ports.

No new completion percentages are assigned. Historical progress estimates are
not proof of current end-to-end closure. The first implementation gate after
resumption is the directory completion path, followed by same-process readback;
see [ROADMAP.md](ROADMAP.md).

## Validation already recorded

The public PowerShell full build passed on 2026-09-08 with MSVC 14.44.35207
and Windows SDK 10.0.19041.0 (all 153 product translation units and link),
according to the existing publication validation record. That record also
reports all 439 source/header/table files matching the source checkpoint byte
for byte, with no missing or untracked quoted-include dependency.

| Check | Recorded state | Scope |
| --- | --- | --- |
| Source and dependencies | Existing publication check | 439 source/header/table files match; zero missing or untracked quoted-include dependencies. |
| PowerShell full compile and link | Existing pass | 153 product translation units; not a gameplay or all-tests pass. |
| CMake build | Not run in publication validation | A project and instructions exist; that build path is not claimed as verified. |
| Game/runtime probes during publication build | Not run | Compilation cannot substitute for runtime acceptance. |
| This documentation update | No new execution results | Runtime observation limits and unresolved work remain in place. |

Source tests being published does not mean all tests were run. Some research
test targets require private diagnostic inputs and are separate from the
product target, as described in [BUILDING.md](BUILDING.md).

These results were not rerun as part of this documentation sync. The publication
build did not run the game or a runtime probe, and no CMake execution or new
gameplay-parity result is claimed. Full-build success is not runtime acceptance.

See [BUILDING.md](BUILDING.md) for the full-build procedure and publication
validation record. Private captures, logs, user cards and additional game assets
are not included. The existing asset and licensing boundaries remain as stated
in [PUBLIC_BOUNDARY.md](PUBLIC_BOUNDARY.md), [THIRD_PARTY.md](THIRD_PARTY.md)
and [NOTICE](NOTICE).
