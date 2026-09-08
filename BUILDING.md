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

## Publication verification — 2026-09-08

- Full build.ps1 compilation and linking: PASS (153 product translation units).
- Toolchain: MSVC 14.44.35207 x64, Windows SDK 10.0.19041.0.
- All 439 source/header/table files match checkpoint e1275d39 byte for byte.
- Missing or untracked quoted-include dependencies: zero.
- No game or runtime probe was run during this build.
- No CMake build or new gameplay-parity claim is made by this verification.
- The source snapshot retains inherited whitespace warnings; publication did
  not reformat or otherwise change the verified game implementation.
