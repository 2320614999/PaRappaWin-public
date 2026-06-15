# Stage1 `801C7A60` accepted-tail mask-edge recheck

Scope: only the accepted producer tail in `801C7A60`; no renderer, lifecycle,
full-song harness input semantics, row table, or `80014614` page write changes.

## Source

- Existing COMOD1 IDA export:
  `docs/stage1_decomp/fail_prompt_ev4_20260510/ida_export_comod1_801C7A60_801C81EC_p0_20260510.txt`
- Current UI IDB did not map `0x801C7A60`; the useful authority remains the
  saved COMOD1 export above and `docs/stage1_decomp/801C7A60_sub_801C7A60.c`.
- A headless COMOD1 re-export attempt timed out at the client boundary before
  writing a new file, so this note records the already available IDA export
  lines rather than treating the timeout as new authority.

## Relevant COMOD1 facts

- `801C7EDC` calls `80035510(1)` on the non-replay live input path.
- `801C7F1C/801C7F20` branches on the normalized input value and writes
  `ctx+0x18`.
- `801C7F24 -> 801C7F34` calls `80024B54(ctx+0x18)` and writes `ctx+0x20`.
- `801C7F38/801C7F40` sets `ctx[0] bit0`.
- `801C7F44` only clears `ctx+0x20` on the no-input path.
- `801C7F48 -> 801C7F50` runs `80024FD0` then `801C9094`.
- `801C7F68 -> 801C7F80` gates the accepted tail with:
  `(*ctx & 1) && ((ctx+0x18) & 0x9FF) != dword_801CCBB8`.
- `801C7F98 -> 801C7FB4` backs up `ctx+0x08/0x10` only after the mask-edge
  gate passes and only in non-replay mode.
- `801C7FB8` calls `80014614(ctx)`, followed by optional `80094434(ctx,1)` and
  unconditional `8009443C(ctx)` inside that same mask-edge block.
- `801C80BC` writes `dword_801CCBB8 = ((ctx+0x18) & 0x9FF)` after the block.

## Direct-port implication

`acceptedGateOpen` remains a useful observable for `(*ctx & 1) && mask != 0`,
but it is not the call authority for `80014614/8009443C`.

The direct call authority is `acceptedMaskChanged`.

This means `PrStageRunnerDirectResolvePostFrame7A60(...)` must call the
accepted producer tail only when the current accepted mask edge differs from
`dword_801CCBB8`, matching COMOD1, rather than on every gate-open frame.

