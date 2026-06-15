# 8001C470 boot callee export summary (2026-05-13)

## Source

- New IDA export:
  `docs/stage1_decomp/ida_export_boot_8001c470_callees_80040ae4_80040b84_8001b1b0_800446a0_800354c0_8001c1e8_20260513.txt`
- Trigger:
  `800154F4 -> 8001ED94 -> 8001C470` exposed these untranslated callees in
  the boot seam.

## Callee facts

| Function | Direct caller in this seam | Main PSX side effects / calls |
| - | - | - |
| `80040AE4(a1,a2,a3,a4)` | `8001C470:8001C4B4` | Writes clip/global words at `8008ECA8/AA/AC/AE`; writes `8008EEF0/F2/F4/F6` either zero or input args depending on `word_800965A0`; calls `800402E0` and `800401AC`. |
| `80040B84()` | `8001C470:8001C4BC` | Writes `word_800901C4=dword_800917FC/2`, `word_800901C6=dword_8009182C/2`; calls `800401AC`; sets `80095D04=10`, `80095D00=0`, `800928A4=0x3FFF`. |
| `8001B1B0(r,g,b)` | `8001C470:8001C4D4` | Builds stack RECT `(0,0,320,480)` and calls `80044CD0(rect,r,g,b)`. Also called from `801C689C`. |
| `800446A0(mode)` | `8001C470:8001C480` | ResetGraph-like path: may call `printf`, `800473C0`, `ResetCallback`, `GPU_cw`, `80046EC0`; writes `8005D734/735/738/73A` and clears related GPU env buffers. |
| `800354C0(a1)` | `8001C470:8001C488` | Writes `80091830=a1`, `800882F0=-1`; calls `ResetCallback`, `PAD_init2(0x20000001,&800882F0)`, `ChangeClearPAD(0)`. |
| `8001C1E8()` | `8001C470:8001C478` | Initializes `8008EDE0..8008EED4` TMD fast function table and zero fields; no external calls. |

## Boundary conclusion

- `8001C470` is no longer just an opaque call-order gap, but it still cannot
  be marked translated or cut over: several callees perform platform/HAL-facing
  setup (`ResetCallback`, `GPU_cw`, `PAD_init2`, `ChangeClearPAD`) and graph/TMD
  table writes that need a coherent boot graph/pad/GPU initialization unit.
- The next direct-port unit should not create more loose carriers. It should
  translate one coupled subgraph with explicit side-effect ownership:
  - graph/TMD table setup: `8001C1E8`, `80040AE4`, `80040B84`, `8001B1B0`;
  - platform reset/HAL-facing setup: `800446A0`, `800354C0`.
- Do not wire `800154F4` or `8001C470` into runtime while this split remains
  open.
