# CD Raw Producer Callchain Boundary - 2026-05-13

Sources:

- `docs/stage1_decomp/ida_export_cd_sync_waitloop_80037070_800372f0_20260513.txt`
- `docs/stage1_decomp/cd_sync_waitloop_facts_producer_80037070_800372f0_20260513.md`
- `docs/stage1_decomp/ida_export_cd_raw_producer_chain_800359b8_80035898_80036af8_80037070_800372f0_20260513.txt`
- `docs/stage1_decomp/cd_raw_producer_chain_followup_20260513.md`
- current-session IDA decompile for `80036AF8` and `800359B8`

This page is evidence only. It does not authorize Win file, ISO, STR/XA,
payload, or replay-sampled facts as lower-CD truth.

## Minimum Chain

`800359B8 -> 80035898 -> 80037070/800372F0/800375BC -> 80036AF8`

| Function | Role | Static evidence | Boundary |
| --- | --- | --- | --- |
| `800359B8` | interrupt dispatcher / pending flag owner | writes `word_80055F7A` at `80035A20`, clears it at `80035B6C` | owns callback-pending timing, not CD return data |
| `80035898` | `CheckCallback` | disasm is `lhu $v0, word_80055F7A; jr $ra`; called by `80037070`, `800372F0`, `800375BC` | read-only pending fact |
| `80037070` | CD sync loop consumer | calls `CheckCallback` at `800371A4`, calls `80036AF8` at `800371CC` | consumes `D4/882F8`, does not own raw facts |
| `800372F0` | CD ready loop consumer | calls `CheckCallback` at `8003742C`, calls `80036AF8` at `80037454` | consumes `D6/D5/88308/88300`, does not own raw facts |
| `800375BC` | command wait loop consumer | calls `80036AF8` at `800378B0` after callback check path | second consumer; not a replacement for request metadata |
| `80036AF8` | raw CD register pump | xrefs from `80037070`, `800372F0`, `800375BC`, `80038118` | only current static owner of raw register/result-bank facts |

Follow-up raw `jal` scan confirms the same direct caller set:
`80037070:800371CC`, `800372F0:80037454`,
`800375BC:800378B0`, and `80038118:80038144`.
`80038118` is an additional raw pump consumer, not a new status/result
authority.

## `80036AF8` Facts

IDA pseudocode and disassembly agree on the useful shape:

- writes selector `1` to `1F801800`;
- reads interrupt code from `1F801803 & 7`;
- drains up to 8 FIFO bytes from `1F801801` while `1F801800 & 0x20`;
- pads missing FIFO bytes with zero;
- acknowledges through `1F801803` and `1F801802`;
- updates `dword_80057108`, `dword_8005710C`, and sometimes `dword_80057110`;
- dispatches status/result banks:
  - `byte_800573D4` / `800882F8` for sync status;
  - `byte_800573D5` / `80088300` for ready fallback status;
  - `byte_800573D6` / `80088308` for ready primary status.

Important branch details:

- case 4 writes `byte_800573D6` and mirrors the value into `byte_800573D5`
  in the current disassembly window (`80036F30`, `80036F48`);
- case 5 writes `byte_800573D5` and mirrors that value into `byte_800573D4`
  in the current disassembly window (`80036FB8`, `80036FD0`);
- these raw writes are the source facts that later builders must validate or
  consume; callers must not synthesize the same banks from request metadata.

Correction note:

- This supersedes earlier notes that interpreted case 4 as clearing
  `byte_800573D5` and case 5 as clearing `byte_800573D4`. The raw disassembly
  stores the loaded status value into the adjacent bank, so the direct
  translation must mirror `D6 -> D5` and `D5 -> D4` for these two cases.

## Consumer Boundary

`80037070` and `800372F0` both follow the same pattern:

1. call `80035898`;
2. if pending, save `CD_REG0 & 3`;
3. call `80036AF8` until its return is drained;
4. restore the selector;
5. consume only the relevant status/result bank.

Therefore the existing typed builders are correctly placed as consumers:

- `BuildCdSyncLoopFacts80037070` consumes raw-derived `D4/882F8`;
- `BuildReadyWaitResult800372F0` consumes raw-derived `D6/D5/88308/88300`;
- `BuildReadSyncSeam800390C8` remains downstream and must not become a raw
  side-channel.

## Remaining Gap

The remaining gap is a runtime facts source that observes PSX-shaped interrupt
and CD-register state and feeds the existing typed builders.

There is no safe source-level change from this evidence alone. A valid next
implementation step must produce explicit facts equivalent to:

- `word_80055F7A` callback pending from `800359B8/80035898`;
- saved/restored selector from `1F801800 & 3`;
- complete ordered `80036AF8` raw pump transactions;
- result bank bytes written by `80036AF8`;
- timeout/spin facts from the wait loops.

Hard rejects remain:

- no Win filesystem / ISO readability as `handled`, `success`, or PSX return;
- no STR/XA playback state as callback pending or ready status;
- no payload or replay bytes as CD register FIFO facts;
- no request metadata as raw CD return source.
