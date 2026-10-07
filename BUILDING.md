# Building PaRappaWin on Windows

## Requirements

- 64-bit Windows.
- Visual Studio 2022 C++ x64 build tools and a Windows 10/11 SDK.
- Windows PowerShell 5.1 or later.

Compilation does not require a disc image, extracted game archive, private
research directory, or downloaded prebuilt object files. Running the game
requires user-supplied game data from a lawful copy.

## Recommended full build

From this repository:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File build.ps1
```

The script discovers Visual Studio via vswhere, or uses the developer
environment's VCToolsInstallDir, WindowsSdkDir and WindowsSDKVersion.
Nonstandard installations can use -MsvcPath, -WindowsSdkPath and
-WindowsSdkVersion. MsvcPath names the versioned VC/Tools/MSVC directory,
not the Visual Studio installation root.

Output is build/product/PaRappaWin.exe, copied to bin/PaRappaWin.exe.
The default performs a full rebuild. Do not use -Fast for verification.
Do not run tests or the game concurrently with compilation. Close this
checkout's executable before rebuilding; the public script does not kill
other checkouts' game or compiler processes.

## CMake alternative

With CMake installed and a Visual Studio x64 toolchain:

```powershell
cmake -S . -B build/cmake -G "Visual Studio 17 2022" -A x64
cmake --build build/cmake --config RelWithDebInfo --target PaRappaWin --clean-first
```

The executable is placed in bin. The product target is separate from research
tests, some of which require private diagnostic inputs. Publication validation
uses build.ps1; a CMake command being documented is not a claim it was run.

## Source and runtime boundaries

All current product translation units and their headers/tables are included.
The public build scripts differ from the private checkpoint only in machine
discovery, output locations and process/cleanup isolation; game source remains
the checkpoint source. No compiler output or private history is distributed.
Existing embedded boot-logo byte headers predate this snapshot and are
unchanged; their data is not licensed as project-owned artwork.

Successful compilation does not prove gameplay parity. In particular, the
Stage1-to-directory resource completion remains unresolved. See STATUS.md.

## Publication verification — 2026-10-07

- Full `build.ps1` compilation and linking: PASS (213 product translation units).
- Toolchain: MSVC 14.44.35207 x64, Windows SDK 10.0.19041.0.
- The public checkout contains 646 current source/header/table files, including
  the Stage2 native runtime, shared presentation settings and UI/HUD sources.
- Public output was kept inside this checkout: `build/product/PaRappaWin.exe`
  and `bin/PaRappaWin.exe`; their SHA-256 is
  `ABD33D2263E28E3E6CA4E426FBE99A8ABDC97828D95A669C956CE555E4973FE3`.
- The full source build included the UI/HUD translation units and completed
  without a missing quoted-include dependency.
- The development checkout separately recorded 5,040 animation samples, actual
  D3D11 glow readback and bounded 60-second Stage1/Stage2 PAD probes. Those
  results are summarized in [STATUS.md](STATUS.md); they are not rerun here.
- CMake product compilation and the complete source-test suite were not rerun
  for this publication. A successful build does not claim full gameplay parity.
- No game data, personal cards, private logs, research dumps or compiled
  binaries are included in the public repository.
