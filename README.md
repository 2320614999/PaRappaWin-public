<p align="center">
  <img src=".github/assets/hero.svg" width="1280" alt="PaRappaWin — A classic rhythm. A native future. Native Windows port and preservation project.">
</p>

<p align="center">
  <strong>English</strong> · <a href="README.zh-CN.md">简体中文</a><br>
  <a href="#watch-it-run">Watch the demo</a> · <a href="BUILDING.md">Build</a> · <a href="STATUS.md">Status</a> · <a href="ROADMAP.md">Roadmap</a> · <a href="CONTRIBUTING.md">Contribute</a>
</p>

**PaRappaWin** brings *PaRappa the Rapper* (PSX) into a native Windows runtime through a port of the original program's behavior—not an embedded emulator. This repository publishes the current full-build source snapshot, including Windows-specific additions.

**经典节奏，原生新生。** 面向 Windows 的原生移植与保存项目；[中文首页](README.zh-CN.md) · [中文进度](PROGRESS.zh-CN.md)。

<table>
<tr>
<td width="33%" valign="top"><strong>01 / NATIVE</strong><br><br>Windows entrypoint, rendering, media, audio and input in the port's own runtime.</td>
<td width="33%" valign="top"><strong>02 / PRESERVE</strong><br><br>Original behavior is the reference. Publishing source is a milestone, not a claim of perfect parity.</td>
<td width="33%" valign="top"><strong>03 / BUILD</strong><br><br>Current product source and both build entrypoints are public. Bring your own lawful game data to run it.</td>
</tr>
</table>

## Watch it run

<p align="center">
  <a href="https://www.youtube.com/watch?v=jLRAnNM8XWU">
    <img src="https://img.youtube.com/vi/jLRAnNM8XWU/maxresdefault.jpg" width="880" alt="Watch the existing Stage 1 Chinese localization gameplay demo on YouTube">
  </a><br>
  <strong><a href="https://www.youtube.com/watch?v=jLRAnNM8XWU">▶ Stage 1 · Chinese localization demo</a></strong><br>
  <sub>Native PaRappaWin Windows footage—not emulator footage. An existing demonstration, not a new acceptance test for the current HEAD.</sub>
</p>

## The checkpoint, at a glance

**Source snapshot: 2026-10-07 · Development checkpoint `610705a9`**

| 646 source / header / table files | 213 product C++ translation units | 2 build entrypoints |
| :---: | :---: | :---: |
| Current publication inventory | Includes Stage2, modern presentation and UI/HUD | `build.ps1` + `CMakeLists.txt` |

| Area | What the current record supports |
| :--- | :--- |
| **Stage1** | Playable implementation with modern feedback, HUD, subtitles and texture support. |
| **Stage2** | A manual first clear and earlier real save, LOAD, second-playthrough and Replay records; a playable native runtime is now included. |
| **Shared presentation** | Common toggles, automatic aspect modes, wide-sky/transition fixes and restored press glow in both stages. |
| **Remaining work** | Complete Stage3–6 ports, ending-video timing stability and unverified behavior branches; intermittent UI/model flicker is deferred. |

The feedback fix passed a full development build, actual GPU readback and bounded PAD probes in both stages. Historical full-clear records are not a complete regression of this snapshot. See [BUILDING.md](BUILDING.md) for public-build results and [STATUS.md](STATUS.md) for evidence scope.

## Build on Windows

Use 64-bit Windows, Visual Studio 2022 C++ x64 build tools, a Windows 10/11 SDK, and PowerShell 5.1 or later. From the repository root:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File build.ps1
```

The default is a **full rebuild**; do not use `-Fast` for verification. Close this checkout's game before building and do not run it or tests concurrently with compilation.

**Output:** `build/product/PaRappaWin.exe`, copied to `bin/PaRappaWin.exe`.

Compilation does not require a disc image or private research workspace. Running the executable requires game data from your own lawful copy; it is not a self-contained game distribution. Toolchain overrides and the CMake alternative are in [BUILDING.md](BUILDING.md).

## Explore the project

| Start here | Go deeper |
| :--- | :--- |
| [中文进度](PROGRESS.zh-CN.md) · [English progress](PROGRESS.en-US.md) | [Status & validation scope](STATUS.md) · [Roadmap](ROADMAP.md) |
| [Full-build guide](BUILDING.md) · [Contribution guide](CONTRIBUTING.md) | [Source inventory](PUBLIC_SYNC_FILELIST.txt) · [Windows-layer inventory](PUBLIC_WIN_LAYER_FILELIST.txt) |
| [Report a reproducible issue](https://github.com/2320614999/PaRappaWin-public/issues/new/choose) | [Publication boundary](PUBLIC_BOUNDARY.md) · [Third-party notes](THIRD_PARTY.md) |

## Frequently asked

<details>
<summary><strong>Is this an emulator?</strong></summary>

No. The target is a native Windows port of the original program's behavior, not an embedded PSX emulator. The current snapshot still has documented parity gaps.

</details>

<details>
<summary><strong>Can I compile it? Can I play the entire game?</strong></summary>

Build results are recorded in [BUILDING.md](BUILDING.md). Stage1 and Stage2 have playable implementations and bounded acceptance records; complete Stage3–6 ports remain unfinished. A full build is not an all-tests or full-game pass.

</details>

<details>
<summary><strong>Does this repository include the game data?</strong></summary>

It is not a complete game-data distribution. Supply your own lawful runtime data. The public boundary retains previously published subtitles, mappings, project-authored PR2 rail textures and pre-existing embedded boot-logo byte headers; this refresh adds no extracted game media. Those third-party bytes are not licensed as project-owned artwork. Read [PUBLIC_BOUNDARY.md](PUBLIC_BOUNDARY.md), [THIRD_PARTY.md](THIRD_PARTY.md) and [NOTICE](NOTICE).

</details>

<details>
<summary><strong>What is the most useful next implementation step?</strong></summary>

Cover more ending-video timing, continuous XA/BGM handoff and remaining save/return branches, then continue later-stage ports. New-process LOAD and same-process menu return remain separate acceptance claims. See [ROADMAP.md](ROADMAP.md).

</details>

<details>
<summary><strong>How can I help?</strong></summary>

Reproducible bug reports, documentation corrections and narrowly scoped patches are useful. English and Chinese are welcome. Include the commit and what you actually tested, redact personal data, and never attach game dumps or personal save cards. Start with [CONTRIBUTING.md](CONTRIBUTING.md). Submission templates do not promise a review timeline.

</details>

---

**License & publication boundary.** Original project-owned materials use [Apache-2.0](LICENSE). This does not grant rights to third-party game data, names, trademarks, audio, video or artwork. The public repository has separate history; private research, credentials, configuration and personal cards remain outside this publication. [NOTICE](NOTICE) · [THIRD_PARTY.md](THIRD_PARTY.md) · [PUBLIC_BOUNDARY.md](PUBLIC_BOUNDARY.md).

<sub>The header is original decorative project artwork, not a game screenshot or an official game cover. [Presentation assets](.github/assets/README.md)</sub>
