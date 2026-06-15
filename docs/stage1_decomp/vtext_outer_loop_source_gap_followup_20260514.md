# Stage1 vtext outer-loop source-gap follow-up (2026-05-14)

Authority:
- `docs/stage1_decomp/ida_export_vtext_outer_loop_source_gaps_8001a750_801c9554_8001a3b8_20260514.md`
- Current active IDA database: `SCUS_941.83`

Scope:
- Only `801C455C` movie/common-lyrics outer-loop gaps that remain after
  `80024C84/80024CF8` descriptor and `word_800916D8` closure.
- No Win fallback, ISO readability, payload bytes, or render text can authorize
  these facts.

## `sub8001A750Result`

`8001A750` is not a host-timer or arbitrary warmup flag. It calls:

- `800364D0(1, statusBuffer)`
- if return is not `2`: returns `0`
- if return is `2` and `statusBuffer[0] & 0x20`: returns `1`
- if return is `2` and bit `0x20` is clear: calls `80036678(1,0)` and returns
  `0`

Therefore `MovieTextOuterLoopInputSub801C455C::sub8001A750Result` needs real
`800364D0/80036678` status facts before it can be treated as known. The current
default `0` must remain only a gap/default, not proof that PSX polled false.

## `dword801C9554EqualsOne`

`dword_801C9554` has a single writer in this export:

- `801C44E0:801C4518 sw v0, dword_801C9554`

`801C455C` only reads it:

- `801C46B8`
- `801C46F8`

So `dword801C9554EqualsOne` must be sourced from the `801C44E0` setup path or a
runtime fact for that global. It is only a display/flush gate around
`8001EC54(a2,7)` and `8001ED3C`; it is not a descriptor-source fact and cannot
be inferred from vtext success.

## `GsGetWorkBase`

`8001A3B8` is `GsGetWorkBase` and simply returns `dword_80049428`.

The paired updater `8001A3C8` is called from `801C448C` and updates
`dword_80049428` from `800364D0(1, statusBuffer)`:

- return `5` with `statusBuffer[0] & 0x10`: `dword_80049428 = 1`
- return `2` and `800363A4() != 13`: `dword_80049428 = 0` and writes
  `byte_800493F4..F6`

So `gsGetWorkBaseResult` should stay gated by explicit known facts. It cannot
be filled from host movie progress or successful vtext advance.

## `MEMORY[0x164]`

The export confirms `801C455C` copies `MEMORY[0x164]` into `ctx+0x0C`, but no
static data xref exists for literal `0x164` in the current narrow export.
Existing movie-segment docs narrow this to the scene-entry cache:
`*(dword_8006ED98 + 0x20 + 0x164)`, with `801C4260` writing the current
scene-entry cache and `sceneEntry+0x164 = *(s16+8)+*(s16+10)`.

So this is not a render-state or Win movie-state value. The next code-facing
step is to consume the scene-entry row/cache source, not to default the copied
value to zero.

## Next Implementation Boundary

The next safe runtime/source change is not another vtext descriptor carrier.
It should either:

- consume real `800364D0/80036678/8001A3C8` facts already produced elsewhere, or
- add explicit known/gap flags so `801C455C` actions do not present unknown
  defaults as applied PSX facts.

Until then, `sub8001A750Result`, `dword801C9554EqualsOne`,
`gsGetWorkBaseResult`, and `memory164` remain non-authoritative inputs.

For the next evidence export, prefer `801C4780`: existing notes indicate
`801C455C` consumes `sceneEntry+0x6C`, while the movie-segment direct boundary
should materialize the seven `sceneEntry+0x0C+i*0x30` rows, run
`801C4780 -> 8001A324`, and select row index `2` for `801C455C`. This is more
useful than re-exporting `8001A750` or `8001A3B8`, whose boundaries are now
clear.
