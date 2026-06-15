# CD Raw Producer Chain Follow-up - 2026-05-13

Sources:

- `docs/stage1_decomp/ida_export_cd_raw_producer_chain_800359b8_80035898_80036af8_80037070_800372f0_20260513.txt`
- `docs/stage1_decomp/cd_raw_producer_callchain_boundary_20260513.md`
- `src/pr/pr_stage1_lower_cd_producer_direct.*`

This is evidence only. It does not authorize Win file, ISO, STR/XA,
payload, replay-sampled bytes, or request-only metadata as lower-CD truth.

## Direct Call Boundary

The current SCUS export confirms the raw callback chain:

| Target | Direct callsites / xrefs | Boundary |
| --- | --- | --- |
| `800359B8` | `800358DC:8003594C` | interrupt dispatcher; owns pending write window |
| `80035898` | `80037070:800371A4`, `800372F0:8003742C`, `800375BC:80037888` | `CheckCallback`, read-only pending fact |
| `80036AF8` | `80037070:800371CC`, `800372F0:80037454`, `800375BC:800378B0`, `80038118:80038144` | raw CD register/result-bank pump |
| `80037070` | `800364D0`, `800367A4`, `800375BC`, `80037CB0` | sync-loop consumer of `D4/882F8` |
| `800372F0` | `800364F0:800364F8` | ready-loop consumer of `D6/D5/88308/88300` |
| `800375BC` | `80036540`, `80036678`, `800367A4`, `80037CB0` | command wait-loop consumer |

Raw `jal` scans match the xref set above:

- `jal 800359B8`: 1 hit.
- `jal 80035898`: 3 hits.
- `jal 80036AF8`: 4 hits.
- `jal 80037070`: 4 hits.
- `jal 800372F0`: 1 hit.
- `jal 800375BC`: 13 hits.

## Data Authority

- `word_80055F7A` has one read at `80035898` and writes at
  `800359B8:80035A20` / `800359B8:80035B6C`.
- `byte_800573D4` is written by `80036AF8` sync cases and mirrored by
  case5 at `80036FD0`; it is then consumed by `80037070`, `800372F0`,
  and `800375BC`.
- `byte_800573D5` is written by `80036AF8` ready cases and mirrored by
  case4 at `80036F48`; it is also cleared/consumed by ready/sync wait loops.
- `byte_800573D6` is written by `80036AF8:80036F30` and consumed/cleared by
  `800372F0`.
- `800882F8`, `80088300`, and `80088308` are the response banks paired with
  `D4`, `D5`, and `D6` respectively; wait loops only consume them after the
  raw pump path has made the corresponding status bank meaningful.

The extra `80038118 -> 80036AF8` caller is an additional raw pump consumer.
It does not make `80038118` a status/result authority and does not change the
existing direct builders: `BuildCdSyncLoopFacts80037070` and
`BuildReadyWaitResult800372F0` remain consumers of raw-derived facts.

## Implementation Status

`src/pr/pr_stage1_lower_cd_producer_direct.*` already represents the important
raw-pump facts:

- case4 mirrors `D6 -> D5`;
- case5 mirrors `D5 -> D4`;
- raw callback pump sequence validation requires supplied return values to
  match each transaction's own `psxReturn`;
- explicit bank inputs are consistency checks and do not authorize sync/ready
  success when the validated raw transaction sequence did not produce the bank.

No new source change is authorized by this export alone. The remaining
implementation gap is still the runtime facts source that can provide the
ordered raw CD register evidence:

- `I_STAT/I_MASK/word_80055FA8` pending samples for `800359B8`;
- `word_80055F7A` before/set/clear facts for `80035898`;
- saved/restored `1F801800 & 3` selector facts for each wait-loop consumer;
- ordered `80036AF8` transactions with FIFO status samples, drained result
  bytes, ack writes, prior `80057108/10/19` facts, and `psxReturn`;
- timeout/spin facts for `80037070`, `800372F0`, and `800375BC`.

For the downstream `8001A818 -> 800390C8(1,0)` read-sync seam, the separate
minimum typed gap remains caller-proven final-ready facts:
`800390C8 -> 800364F0(1,a1) -> 800372F0`. These facts only allow
`BuildReadyWaitResult800372F0(...)` to model the side effect; they do not
authorize `800390C8` return mapping. Retry return authority stays on
`80038DE8(1)` post-retry `dword_80057410` facts.
