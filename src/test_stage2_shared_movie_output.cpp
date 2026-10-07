# Public Status

Last updated: 2026-10-07

[README](README.md) | [中文进度](PROGRESS.zh-CN.md) | [English Progress](PROGRESS.en-US.md) | [Build](BUILDING.md) | [Roadmap](ROADMAP.md)

## Source checkpoint

Source checkpoint: `610705a9` (development provenance; private Git ancestry is not included).
This source update follows the September publication and retains the public repository's bilingual landing pages and independent history.

The snapshot includes 646 C++ source/header/table files and 213 product translation units, including UI/HUD, Stage2 native runtime, and shared presentation settings. Both build entrypoints are updated. Public scripts retain portable toolchain discovery and output paths inside this checkout.

## Runtime progress

| Area | Verified progress | Remaining limits |
| --- | --- | --- |
| Stage1 | Playable implementation and modern note feedback, HUD, subtitle and texture support are present. | Full original-behavior parity and every same-process return/save/error branch are not claimed. |
| Stage2 first playthrough | The maintainer completed a manual playthrough. Earlier isolated PAD play also reached GOOD and wrote 110 actual replay inputs to a test card. | The save test skipped the ending movie; it does not prove all ending-video timing is stable. |
| LOAD and second playthrough | An earlier new-process LOAD read the real card and entered the second playthrough; actual PAD play reached 96 points/GOOD and the ending transition. | A second save was not established by that run. |
| Replay | Card-recorded inputs played to completion, returned naturally to the resident menu, and restored progression without changing the card. | This does not validate every Stage1 return/error branch. |
| Modern presentation | Stage1/Stage2 share enhancement toggles, auto/4:3/stretch aspect modes, 60fps presentation, note feedback, score HUD, creative prompts, HD subtitles and matched texture replacements. | Source support is not a guarantee of all-scene performance or complete replacement-art coverage. |
| Widescreen and feedback | Sky extension, retained-frame edge trails, transition/video viewports and both stages' visible press glow are fixed in this snapshot. | Intermittent UI/model flicker remains deferred. |
| Stage3–6 | Existing scaffolding is published. | Complete native playable ports remain unfinished. |

Windows replaces the PSX main executable at runtime: `SCUS_941.83` is not required when the native data package and the remaining required user-supplied resources are available. Stage2 uses `S2/S2_NATIVE_DATA.BIN` and its original stage archives; those files are not distributed here.

The Stage2 ending-video VLC crash received a DMA stop-boundary fix and one complete outro decode was observed. The overall historical transition probe did not pass every acceptance condition; all timing variants remain unproven. Stage2 entry also stops retained Stage1 XA/BGM; this publication does not add a continuous save-page-to-Stage2 audio acceptance run.

## Validation for this snapshot

- Development build: full Windows `build.ps1`, including UI/HUD, passed on 2026-10-07.
- Shared impact/fade/single-turn animation: 5,040 samples passed. Actual D3D11 readback verified visible light, rapid fade, disabled glow and parity between sprite and native-triangle rendering.
- The same development binary completed separate 60-second Stage1 and Stage2 PAD-input probes without `SCUS_941.83`; both exited normally and preserved configuration/cards. These are bounded feedback tests, not new full playthroughs.
- Public checkout full build: pending in this publication work session; the result will be recorded before push.
- CMake product compilation and the complete source-test suite were not rerun for this publication.

Historical clear/save/LOAD/Replay observations belong to the tested earlier binaries. They are not presented as a complete regression run of this snapshot. Private evidence logs and user cards are not published.

See [BUILDING.md](BUILDING.md), [PUBLIC_BOUNDARY.md](PUBLIC_BOUNDARY.md) and [THIRD_PARTY.md](THIRD_PARTY.md). No completion percentage or full-game completion claim is assigned.
