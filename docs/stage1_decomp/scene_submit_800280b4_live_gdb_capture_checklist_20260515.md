# Scene Submit `800280B4` live GDB capture checklist

Date: 2026-05-15

Scope: Stage1 Scene Submit / TOD `80028054` `coordNode+0x44` authority only.

## Breakpoint order

| Order | Breakpoint | Purpose | Hit rule |
|---:|---|---|---|
| 1 | `0x801CBD60` | mark first submit group | subsequent path should enter `8001B000 -> 80028504 -> 80028054`; expected group size is 9 `80028054` calls |
| 2 | `0x801CBDA8` | mark second submit group | subsequent path should enter `8001B000 -> 80028504 -> 80028054`; expected group size is 9 `80028054` calls |
| 3 | `0x800280B4` | capture `coordNode+0x44` authority | sample only after one of the submit gates; require `pc == 800280B4` |

`801CBD60/801CBDA8` are grouping gates only. They are not authority for
`coordNode+0x44`.

When stopped at the `jal` callsite, remember that delay-slot argument setup has
not necessarily been reflected as a stable callee-entry snapshot. For authority,
use `800280B4`, not the gate-site argument registers.

## Required capture fields

At each valid `800280B4` hit, capture:

| Category | Fields | Purpose |
|---|---|---|
| Registers | `$pc,$ra,$sp,$a0,$a1,$a2,$v0,$s2,$s3,$s5,$s6` | bind current submit/call, desc, TOD cursor, and command window |
| Desc memory | `mem32[$a1]`, `mem32[$a1+4]` | verify `desc+4` points to current COORD node |
| COORD node memory | `mem32[$v0]`, `mem32[$v0+0x44]` | `mem32[$v0+0x44]` is the target authority value |
| TOD command | `mem32[$s5]`, plus command payload window if available | map the hit back to one of the 18 frame2212 per-call rows |

Valid authorization requires:

- `pc == 0x800280B4`,
- `$v0 != 0`,
- `$v0 == mem32[$a1+4]`,
- `mem32[$v0+0x44]` was read successfully at that stop.

## Non-authority

These do not authorize `coord44BackingSlotKnown80028054`:

- `801CBD60/801CBDA8` gate hits alone,
- `$s0` before `800280B4` executes,
- `800280BC` clearing `coordNode+0`,
- `80028054` call entry without instruction-time `$v0`,
- frame-boundary registers,
- call-trace / branch-trace / write-log alone,
- five candidate absolute slots,
- prologue stack-save writes,
- case0 desc/header stores,
- case4 matrix/translation stores,
- Win COORD state, renderer, TMD, visual sampling, ISO/file readability, or
  default identity TRS.
