# Public Status

Last updated: 2026-06-15

## Summary

The project is active, but the public source release is not ready to publish as
a raw mirror of the private working repository.

The current private repository contains active implementation work, validation
tools, research notes, generated logs, temporary artifacts, and unfinished
state. Publishing that repository directly would expose noisy or unsafe
material and would not represent a clean public release.

## Current State

- Active development continues in the private repository.
- A separate public-status repository is being prepared as the public-facing
  source of truth.
- The first public source release should be a curated snapshot, not a dump of
  the private worktree.

## Release Readiness Checklist

- [ ] Public README describes the project accurately.
- [ ] Source license is chosen and committed.
- [ ] Private logs and temporary artifacts are excluded.
- [ ] Asset and binary redistribution boundaries are reviewed.
- [ ] Build instructions are reproducible from a clean checkout.
- [ ] Current source snapshot passes the release validation suite.
- [ ] GitHub repository visibility and release notes are prepared.

## Near-Term Public Updates

The next public updates should focus on:

- what is implemented;
- what is still incomplete;
- what can be built or tested by outside users;
- which parts are intentionally not shipped yet.
