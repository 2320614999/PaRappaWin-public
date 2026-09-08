# Public Boundary

Updated: 2026-09-08, following the project owner's authorization to publish all
source required for the current full build.

## Included

- Complete current C++ build source and headers, including S0/SS0, Stage1,
  Windows-specific additions, and existing later-stage scaffolding.
- Required implementation tables, source tests, PowerShell and CMake build entrypoints.
- Public build/status documentation and file inventories.
- Previously published helper tools, subtitle data, mapping tables, sample
  configuration, and project-authored PR2 rail PNG textures.

The earlier restriction on S0/SS0 and compile-required orchestration source is
superseded. Unfinished code remains unfinished even though it is now public.

## Excluded from this update

- Private Git history, development notes and temporary research traces.
- Credentials, tokens, account-specific configuration and personal save cards.
- IDA/PDB databases, memory dumps, build products and runtime captures.
- Additional official/extracted game images, audio, video, disc images or archives.

The already-public `src/tim_sony.h` and `src/tim_masaya.h` contain embedded
boot-logo asset bytes and are unchanged in this update. Their pre-existing
presence does not grant redistribution or ownership rights under Apache-2.0.

## Release rule

Sync a reviewed source allowlist into the public repository's own history;
do not mirror the private worktree or push private commit ancestry here.
Build source completeness, successful compilation, and gameplay parity are
separate claims. Report unresolved behavior explicitly.
