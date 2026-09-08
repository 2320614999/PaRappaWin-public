# Public Status

Last updated: 2026-09-08

Source checkpoint: `e1275d39` (private development reference, for provenance;
its Git ancestry is not included in this repository).

## Source availability

All 439 current source/header/table files from the working source snapshot are
published, together with both build entrypoints. The product build contains
153 C++ translation units. Windows-specific features remain in the source.
This is a reviewed snapshot in the public repository's separate history,
not a publication of the private Git history or research workspace.

## Runtime status

- S0/SS0 and Stage1 implementations are present; parity work is not complete.
- Stage1 movie Cross-skip was observed through the native PAD logic ending
  video, subtitles and movie audio together. Physical-controller verification
  is still outstanding; the observation used injected PAD input.
- Returning from Stage1 reaches the resident directory resource-load request,
  but the lower CD completion feedback still stalls. The same-process
  directory / LOAD / high-score / progression loop is not closed.
- Earlier isolated save/readback observations do not prove that remaining
  same-process loop works.
- Stage2 and later scaffolding is included where it exists; those stages are
  not claimed as completed ports.

No new completion percentages are assigned to this snapshot. The older
progress pages are historical estimates, not proof of current end-to-end closure.

## Validation

The public PowerShell full build passed on 2026-09-08 with MSVC 14.44.35207
and Windows SDK 10.0.19041.0 (all 153 product translation units and link).
All 439 source/header/table files match the source checkpoint byte for byte;
no quoted-include dependency is absent from the published source tree.

See [BUILDING.md](BUILDING.md) for the full-build procedure and publication
validation record. Source completeness is not equivalent to runtime parity.
Private captures, logs, user cards and additional game assets are not included.
