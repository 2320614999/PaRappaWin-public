# PaRappaWin Port Progress

Updated: 2026-09-08

[README](README.md) | [中文](PROGRESS.zh-CN.md) | [Build](BUILDING.md)

The complete current build-source set is public, including the Windows
entrypoint and enhancements, S0/SS0, Stage1, existing later-stage scaffolding,
and build scripts. The public repository retains its separate history.

Source completeness is not port completion:

- Stage1 native Cross-skip was observed ending movie video, subtitles and audio
  through injected PAD input. Physical-controller verification remains open.
- Stage1's resident directory return starts the native resource request but
  still stalls waiting for lower CD completion feedback.
- Same-process directory / LOAD / high-score / progression readback is not closed.
- Stage2 and later stages are not completed ports.
- Windows-specific optimizations and additions are retained.

Use a full rebuild, not incremental verification. Users must supply their own
lawfully held runtime game data. No additional game media, personal saves,
private configuration or research captures are published in this update.

See [Status](STATUS.md) and [Public Boundary](PUBLIC_BOUNDARY.md). This snapshot
does not restate the old progress percentages as proof of end-to-end parity.
