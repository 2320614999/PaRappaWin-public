# PaRappaWin

<p align="center">
  <a href="https://www.youtube.com/watch?v=jLRAnNM8XWU">
    <img
      src="https://img.youtube.com/vi/jLRAnNM8XWU/maxresdefault.jpg"
      width="720"
      alt="PaRappaWin Stage 1 Chinese localization gameplay demo"
    >
  </a>
</p>

<p align="center">
  <strong>Stage 1 Chinese localization gameplay demo</strong><br>
  Native PaRappaWin Windows runtime footage — not emulator
</p>

[中文进度](PROGRESS.zh-CN.md) | [English Progress](PROGRESS.en-US.md) | [Build](BUILDING.md) | [Boundary](PUBLIC_BOUNDARY.md)

![build source](https://img.shields.io/badge/full_build_source-published-brightgreen?style=flat-square)
![port status](https://img.shields.io/badge/port-in_progress-yellow?style=flat-square)

A native Windows port and preservation project, not an embedded emulator.

## Source snapshot — 2026-09-08

The public repository now includes the complete current build-source set:
Windows entrypoint, platform/media/render/audio code, S0/SS0 implementation,
Stage1 implementation, existing later-stage scaffolding, headers, tables,
source tests, `build.ps1`, and `CMakeLists.txt`.

Source availability does **not** mean that every scene is fully ported or that
all behavioral parity issues are resolved. Stage1-to-directory loading remains
blocked in the current snapshot. Stage2 and later stages are not complete ports.
See [Status](STATUS.md).

## Build

Install Visual Studio 2022 C++ x64 build tools and a Windows 10/11 SDK, then run:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File build.ps1
```

The default is a **full rebuild**. Do not use `-Fast` for verification.
Output: `build/product/PaRappaWin.exe` and `bin/PaRappaWin.exe`.
Toolchain overrides and CMake instructions are in [BUILDING.md](BUILDING.md).
A compiled executable is not a self-contained game distribution: runtime game
data must come from the user's own lawful copy.

## Publication boundary

This update publishes build source, not the private repository's Git history,
research logs, IDA databases, memory dumps, personal configuration, save cards,
or additional extracted game assets. Existing public subtitles, mapping files,
and project-authored PR2 rail textures are retained. The previously published
embedded boot-logo byte headers are unchanged; they are asset data, not covered
by a claim of project ownership. See [Third-party notes](THIRD_PARTY.md).

## Documents and license

[Public boundary](PUBLIC_BOUNDARY.md), [status](STATUS.md), [roadmap](ROADMAP.md),
[publication inventory](PUBLIC_SYNC_FILELIST.txt), and
[Windows layer inventory](PUBLIC_WIN_LAYER_FILELIST.txt).

Original project-owned materials use [Apache-2.0](LICENSE). That license does
not grant rights to third-party game data, trademarks, audio, video, or artwork.
See [NOTICE](NOTICE) and [THIRD_PARTY.md](THIRD_PARTY.md).
