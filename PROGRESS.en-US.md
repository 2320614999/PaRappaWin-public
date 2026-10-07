# PaRappaWin Port Progress

Updated: 2026-10-07

[README](README.md) | [中文](PROGRESS.zh-CN.md) | [Build](BUILDING.md) | [Status](STATUS.md) | [Roadmap](ROADMAP.md)

The public source now follows development checkpoint `610705a9`: 646 source/header/table files, 213 product translation units, and both build entrypoints. It includes the playable Stage2 runtime and shared Stage1/Stage2 presentation enhancements.

- Stage1 remains playable with modern note feedback, HUD, subtitle and texture support.
- Stage2 has a maintainer-confirmed manual first clear, plus earlier real save, new-process LOAD, second-playthrough and Replay acceptance records. Replay returned naturally to the menu and preserved the card.
- Presentation settings are shared: master toggle, automatic/4:3/stretch aspect modes, 60fps presentation and modern HUD/rail/subtitle/texture options.
- Wide sky coverage, retained-frame edge trails and transition/video viewport scaling were corrected. The latest fix restores a visible flash over each pressed note, scales its halo with the impact, and preserves the rapid nonlinear shrink/fade followed by one complete flip.
- The native Windows runtime can run without `SCUS_941.83`, using the native data package and user-supplied stage resources.

These milestones come from different tested builds. The 2026-10-07 feedback change passed a full development build, 5,040 animation samples, actual GPU readback checks and separate 60-second Stage1/Stage2 PAD probes; this is not a repeat of every full-clear/save test. See [STATUS.md](STATUS.md) for the public-build result and evidence scope.

Remaining work includes complete Stage3–6 ports, ending-video timing stability, deferred intermittent UI/model flicker, and unverified save/error/controller/parity branches. The XA/BGM handoff has an explicit stop fix but no new continuous handoff acceptance in this publication.

The repository retains independent public history and the existing publication boundary. Runtime media, native binary data packages, private diagnostics and personal cards are not included. Bring your own lawful game data; compiling the source does not prove complete gameplay parity.
