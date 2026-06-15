# Stage1 Memory Probe

External, detachable Stage1 debug sidecar.

Goals:

- Do not link into `PaRappaWin.exe`.
- Do not add runtime shell code, observers, sentinels, or render patches.
- Coordinate with the existing DebugServer on `127.0.0.1:19790`.
- Read optional whitelisted memory fields with `ReadProcessMemory`.

The probe samples DebugServer commands and optional memory-profile fields into
JSONL. This keeps visual/runtime evidence outside the game code while still
letting samples align to the existing debug server frame/status output.

Read-only tools:

- `stage1_mem_probe.py`
- `stage1_scene_submit_probe.py`
- `analyze_good_to_cool_capture.py`

Reproduction-only tool:

- `stage1_force_good_to_cool.py` writes a small set of PDB-verified runtime
  fields in a running process to force a GOOD->COOL transition. Use it only to
  reproduce/capture a visual state when manual play is unavailable. Do not
  treat a forced run as PSX authority.

Example:

```powershell
python tools\stage1_mem_probe\stage1_mem_probe.py --once
python tools\stage1_mem_probe\stage1_mem_probe.py --count 300 --interval 0.05 --out artifacts\stage1_mem_probe\rail.jsonl
python tools\stage1_mem_probe\stage1_mem_probe.py --profile tools\stage1_mem_probe\profiles\stage1_rail.example.json --count 120
python tools\stage1_mem_probe\stage1_scene_submit_probe.py --count 40 --interval 0.1 --out artifacts\stage1_mem_probe\scene_submit.jsonl
```

Profile fields are opt-in. Keep disabled example fields in source control; put
machine/build-specific RVA profiles under `artifacts/` or another scratch path
unless they are intentionally reusable.
