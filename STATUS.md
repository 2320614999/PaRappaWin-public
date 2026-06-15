# Public Status

Last updated: 2026-06-15

## Summary

This repository is a public progress and curated-source snapshot for
PaRappaWin. It is not yet the complete clean-build source tree.

The private workspace remains the source of truth for active implementation,
validation, reverse-engineering notes, generated logs, and unfinished state.
Only materials that pass the public boundary are published here.

## Current Progress

| Area | Progress | Public status |
|---|---:|---|
| Stage1 overlay port | 96% | Selected source and notes published. |
| Stage1 core gameplay | 100% | Gameplay complete in the private workspace. |
| S0 / SS0 direct runtime | 70% | Active private work; source held back. |
| Full public build source | Not complete | Some compile-required source is intentionally omitted for now. |

S0 / SS0 is estimated from current direct-runtime evidence, implemented
scaffolding, live probes, and remaining cutover blockers. It is not being
presented as a finished migration.

## Current Public Contents

- Stage1-facing source selected for public release.
- Reusable Windows platform/media/decoder/PSX-adapter code selected for public
  release.
- Public docs, selected helper tools, Stage1 subtitle data, `bin/config.ini`,
  and Stage1 HD texture mapping tables.
- Empty placeholders for private directories whose contents are not currently
  publishable.

## Explicit Limits

- This repository does not contain all source files required for a clean public
  build.
- Private entrypoint/build orchestration remains held back where it references
  S0/SS0 or Stage2+ boundaries.
- S0/SS0 implementation and verification artifacts remain private for now.
- Stage2 and later stage content remains private for now.
- Images, binaries, screenshots, logs, save data, PDB/IDA files, and proprietary
  assets are not distributed here.

After the migration reaches the completion boundary, additional compile-required
source will be reviewed and pushed in curated batches.

## Release Readiness Checklist

- [x] Public README describes the project accurately.
- [x] Stage1 progress is stated separately from public-source completeness.
- [x] S0/SS0 progress is stated with current blockers and no cutover claim.
- [x] Private logs and temporary artifacts are excluded from this snapshot.
- [x] Image and binary assets are excluded from this snapshot.
- [x] S0/SS0 and Stage2+ source paths are excluded from this snapshot.
- [x] Windows adaptation layer files selected for public release.
- [ ] Build instructions are reproducible from a clean public checkout.
- [ ] Source license is chosen and committed.
- [ ] Later-stage public release boundaries are reviewed.
