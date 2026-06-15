# PaRappaWin Public

This repository is the public, curated snapshot for the PaRappaWin
preservation and Windows-port effort.

It is intentionally not a raw mirror of the private development worktree. The
current public snapshot is limited to Stage1-facing code plus the reusable
Windows adaptation layer that is safe to share at this point.

## Current Public Position

- The active implementation work is still in a private development repository.
- This repository currently publishes the Stage1 public surface and the Windows
  adaptation layer.
- S0/SS0 work, Stage2 and later stages, images, binaries, screenshots, logs,
  and private scratch artifacts are intentionally excluded.
- Empty placeholder directories may be present where a private directory exists
  but its contents are not part of the current public boundary.

## What Belongs Here

- Stage1 source files selected for public release.
- Stage1 public notes and decompilation references.
- Stage1 helper tools selected for public release.
- Windows platform and media adaptation code selected for public release.
- `bin/config.ini`.
- Stage1 subtitle data.
- Stage1 HD texture mapping tables, without image assets.
- Public roadmap and boundary documents.

## What Does Not Belong Here

- Private worktree dumps.
- Reverse-engineering scratch logs.
- Temporary capture artifacts.
- Images, screenshots, binaries, videos, PDB/IDA files, or save data.
- S0/SS0 implementation or verification artifacts.
- Stage2 or later stage content.
- Current private entrypoint and full scene orchestration files while they still
  reference held-back boundaries.
- Internal prompts, session logs, or tool output.

## Status Documents

- [STATUS.md](STATUS.md) - current public status.
- [ROADMAP.md](ROADMAP.md) - release-facing roadmap.
- [PUBLIC_BOUNDARY.md](PUBLIC_BOUNDARY.md) - rules for what can be published.
- [PUBLIC_SYNC_FILELIST.txt](PUBLIC_SYNC_FILELIST.txt) - files included in
  this curated snapshot.

## License

No open-source license has been declared yet. Until a license is added, all
rights are reserved by default.
