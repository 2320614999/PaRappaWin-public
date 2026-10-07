# Roadmap

Updated: 2026-10-07

[README](README.md) | [Build](BUILDING.md) | [Status](STATUS.md) | [中文进度](PROGRESS.zh-CN.md) | [English Progress](PROGRESS.en-US.md)

## Published progress

- [x] Current complete product source, including S0/SS0, Stage1, playable Stage2 and Windows enhancements.
- [x] Stage2 manual first clear and earlier real save / new-process LOAD / second-playthrough / Replay observations, with limits documented in STATUS.md.
- [x] Shared enhancement settings and automatic aspect adaptation; wide sky and transition/video viewport fixes.
- [x] Visible Stage1/Stage2 press glow with rapid nonlinear shrink/fade and one flip.
- [x] Separate public history, bilingual project pages and reviewed source inventories.

## Runtime acceptance still open

- [ ] Cover more Stage2 ending-video start/stop timing, including repeated full outros and saving afterward.
- [ ] Verify retained XA/BGM teardown through a continuous Stage1 save-page-to-Stage2 handoff.
- [ ] Revisit intermittent UI/model flicker once reliable reproduction is available; avoid speculative model-priority changes.
- [ ] Complete Stage1 same-process return / LOAD / high-score / progression and long-tail save/error acceptance. Stage2 Replay return and a new-process card read do not establish all Stage1 branches.
- [ ] Verify physical-controller behavior separately from injected PAD tests.

## Later stages and build coverage

- [ ] Complete Stage3–6 native playable ports with entry/gameplay/exit evidence per stage.
- [ ] Record a full CMake product compilation before claiming that build path is verified.
- [ ] Broaden behavior-specific parity and performance validation; published test source does not mean the whole suite passed.

Continue publishing reviewed source checkpoints while preserving Windows enhancements and the asset boundary. Tag a stable release only after its relevant end-to-end behavior is verified.
