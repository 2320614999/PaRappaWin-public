# Rail/GTE live 8003F710 positive-depth cutover - 2026-05-13

Source:

- Live GDB result from the Rail/GTE side window, consumed in this workspace
  turn.
- Static `8003F710` instruction shape from
  `sprite_submit_20260510/ida_export_8003F1B4_gte_helpers.txt`.

## Adopted live facts

The live trace covers the ordinary positive-depth
`8003F4F4 -> 8003F710` path:

- `sxy[0..3].word = FF90FF8E / FF90FFAE / FFB0FF8E / FFB0FFAE`
- `flagAfterRtpt = 0x100`
- `flagAfterRtps = 0x100`
- `ir0 = 0x127`
- `szAfterRtpt = {0, 0x1B8, 0x1B8, 0x1B8}`
- `szAfterRtps = {0, 0x1B8, 0x1B8, 0x1B8}`
- `sz3AfterRtps = 0x1B8`
- `returnValue = 0x6E`
- the four division samples have known inputs, quotient candidate, and
  divide-overflow=false

## flagOr resolution

The side-window report had a mismatch between `FLAG snapshot = 0x100` and
`flag OR write slot = 0x1000`.

The current static instruction sequence is:

- `8003F748: cfc2 $v1, $31`
- `8003F770: cfc2 $t0, $31`
- `8003F778: or $t0, $v1`
- `8003F77C: sw $t0, 0($t2)`

Given the two live snapshots are both `0x100`, the value written by
`8003F77C` for this sample is `0x100`. The `0x1000` value is not used as
`flagOr` authority.

## Runtime boundary

`PrPsxGteDirect::PsxCall8003F710_RotTransPers4RtptRtpsGap(...)` now promotes
the live facts to known only when the current direct helper already has a
complete geometry input, `H/SZ` match the `0x1B8` positive-depth sample, and
the existing SXY candidate words match the four live SXY words. Non-matching
near-zero, negative-depth, clamp, and overflow windows remain explicit gaps
until a matching live trace exists.
