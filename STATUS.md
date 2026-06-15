# Public Status

Last updated: 2026-06-15

## Summary

The project is active. This repository now contains a curated Stage1 plus
Windows adaptation layer public snapshot rather than a full private worktree
mirror.

The private repository still contains active implementation work, validation
tools, research notes, generated logs, temporary artifacts, and unfinished
state. Those materials are not published here unless they pass the public
boundary.

## Current State

- Active development continues in the private repository.
- The public repository contains Stage1-facing source, Windows/media adaptation
  code, docs, tools, subtitle data, config, and HD texture mapping tables.
- S0/SS0 and Stage2+ content are held back.
- Image and binary assets are held back.
- The private entrypoint/build orchestration is still held back where it
  references S0/SS0 or Stage2+.

## Release Readiness Checklist

- [x] Public README describes the project accurately.
- [ ] Source license is chosen and committed.
- [x] Private logs and temporary artifacts are excluded from this snapshot.
- [x] Image and binary assets are excluded from this snapshot.
- [x] S0/SS0 and Stage2+ paths are excluded from this snapshot.
- [x] Windows adaptation layer files selected for public release.
- [ ] Build instructions are reproducible from a clean public checkout.
- [ ] A full public source license decision is made.
- [ ] Later-stage public release boundaries are reviewed.

## Near-Term Public Updates

The next public updates should focus on:

- what is implemented;
- what is still incomplete;
- what can be built or tested by outside users;
- which parts are intentionally not shipped yet.
