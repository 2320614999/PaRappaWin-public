# 2026-05-10 event4 prompt `800203D4` RGB chain audit

IDA MCP decompile target set:

- `800203D4`
- `8001C550`
- `8001B590`
- `8001B25C`
- `8003FA20`

## Static facts

- `800203D4(ctx0)` calls `8001C550` three times:
  - title: `(56,57,dword_80050950,0)`
  - left: `(70,149,ctx0 == 0 ? dword_80050980 : dword_80050960,0)`
  - right: `(178,152,ctx0 == 1 ? dword_80050990 : dword_80050970,0)`
- `8001C550(x,y,tpl,priority)` forwards to
  `8001B590(x,y,tpl,0,0,priority,&unk_80087288 + 20 * gp[872])`.
- `8001B590` builds a stack-local sprite at `sp+0x10`, writes screen center
  offsets at local `+0x04/+0x06`, calls `8001B25C(local,tpl,0,0)`, then calls
  `8003FA20(local,work,priority)`.
- `8001B25C` writes local fields through `+0x12`:
  - `+0x00` attr
  - `+0x08/+0x0A` width/height
  - `+0x0C` tpage
  - `+0x0E/+0x0F` uv
  - `+0x10/+0x12` clut / texture page word fields
- `8003FA20` reads RGB from local `+0x16/+0x15/+0x14` and packs them into
  packet word 2.

## Conclusion

The `800203D4 -> 8001C550 -> 8001B590 -> 8001B25C -> 8003FA20` static chain
does not close the local FastSprite RGB bytes. `PromptSubmitResult800203D4`
must keep `localSpriteRgbKnown=false`, `localSpriteRgbGap=true`, and
`packetColorWordKnown=false`.

Do not close this gap with renderer white, zero initialization, template color,
or Win native fallback. A future closure needs a positive writer for local
`+0x14/+0x15/+0x16` or a dynamic verification pass that proves the PSX stack
bytes at the `8003FA20` call boundary.
