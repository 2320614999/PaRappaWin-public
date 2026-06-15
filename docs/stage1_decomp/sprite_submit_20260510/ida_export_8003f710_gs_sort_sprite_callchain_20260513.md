# 8003F710 / GsSortSprite Call-Chain Follow-up - 2026-05-13

Source:

- current-session IDA decompile for `0x8003F710`

This page is evidence only. It does not authorize a Win-side GTE approximation.

## Direct Caller

`RotTransPers4` at `0x8003F710` is called from `GsSortSprite`.

- `GsSortSprite` start: `0x8003F1B4`
- `RotTransPers4` callsite inside `GsSortSprite`: `0x8003F4F4`

Relevant chain already tracked in the project notes:

`0x80024418 -> 0x8001C804 -> 0x8001B338 -> 0x8003F1B4 -> 0x8003F710`

## What the function proves

The current decompile shows:

- two `cop2` stages: `0x280030` then `0x180001`
- `cfc2 $31` after each stage
- `swc2 $12/$13/$14` after `RTPT`
- `swc2 $14/$8` after `RTPS`
- `mfc2 $19` and a final `>> 2` return
- `*a10 = $31_before | $31_after`

## What remains unknown

Keep these false until a PSX-authoritative trace closes them:

- `sxyWordsKnown`
- `flagAfterRtptKnown`
- `flagAfterRtpsKnown`
- `ir0Known`
- `returnValueKnown`

The function shape is enough to confirm the remaining gap is not the dispatcher
or packet linkage. The remaining work is exact GTE truth, not another wrapper
layer.
