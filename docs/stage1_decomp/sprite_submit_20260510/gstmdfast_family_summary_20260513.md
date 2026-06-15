# GsTMDfast Four-Point Family Summary - 2026-05-13

Source:

- `ida_export_gstmdfast_family_8003bd9c_8003bf04_8003c91c_8003ca9c_8003d58c_8003d72c_8003e26c_8003e428_20260513.txt`

This is a Rail/GTE direct-port evidence window only. It proves callsites and
packet/GTE instruction shape for the four-point TMD fast paths. It does not
authorize renderer projection, Win TMD state, or replay sampling as truth.

## Dispatcher Call Table

All eight targets are called from the same dispatcher, `8001C1E8`.

| Function | Name | Dispatcher callsite |
| --- | --- | --- |
| `8003BD9C` | `GsTMDfastNF4` | `8001C348 -> 8003BD9C` |
| `8003BF04` | `GsTMDfastF4NL` | `8001C268 -> 8003BF04` |
| `8003C91C` | `GsTMDfastNG4` | `8001C368 -> 8003C91C` |
| `8003CA9C` | `GsTMDfastG4NL` | `8001C358 -> 8003CA9C` |
| `8003D58C` | `GsTMDfastTNF4` | `8001C388 -> 8003D58C` |
| `8003D72C` | `GsTMDfastTF4NL` | `8001C378 -> 8003D72C` |
| `8003E26C` | `GsTMDfastTNG4` | `8001C3A8 -> 8003E26C` |
| `8003E428` | `GsTMDfastTG4NL` | `8001C398 -> 8003E428` |

## Shared GTE Shape

The eight functions share the same four-point pipeline shape:

- Load three vertices with `lwc2 $0..$5`, then run `cop2 0x280030` (`RTPT`).
- Read `cfc2 $31` and reject when FLAG is negative.
- Load the fourth vertex, run `cop2 0x180001` (`RTPS`), and reject on
  negative FLAG.
- Emit screen-space packet words through `swc2 $12/$13/$14` and, for the
  fourth point, `swc2 $14` after `RTPS`.
- Run `cop2 0x168002E`, read `$7`, and use it with the ordering-table base and
  shift to link the packet.
- Use `dword_80095C48` in the primitive code / color word construction.

The primitive packet tag families observed here are:

- flat untextured quads: `0x05` tag (`NF4`, `F4NL`)
- gouraud untextured quads: `0x08` tag (`NG4`, `G4NL`)
- textured flat/gouraud quads: `0x09` tag (`TNF4`, `TF4NL`, `TG4NL`)
- textured gouraud normal path: `0x0C` tag (`TNG4`)

## Boundary

This closes the static callsite and packet-shape evidence for the four-point
TMD fast family. It still does not close:

- exact `FLAG` bit mapping after `RTPT` or `RTPS`
- screen-division / saturation details behind SXY values
- SZ FIFO and depth semantics beyond the observed `$7` ordering-table path
- `IR0`
- the `$19 >> 2` return truth used by `8003F710`

Therefore `sxyWordsKnown`, `flagAfterRtptKnown`, `flagAfterRtpsKnown`,
`ir0Known`, and `returnValueKnown` must remain false until a PSX-authoritative
GTE translation or trace closes those fields.
