# TEXT vararg / record gap matrix: 80043A14 -> 800436F0

Scope: static-only整理。No build, no test, no emulator, no `src` edits.

Inputs read:

- `80043A14_callsite_order_20260510.md`
- `record_producer_branch_20260510.md`
- `ida_export_record_producer_80043A14_800436F0_20260510.txt`
- `src/pr/pr_psx_event_frame_direct.{h,cpp}`
- `src/pr/pr_stage1_fail_prompt_direct.{h,cpp}`

## Static conclusion

`80043A14` is the text append/format producer. The current Win carrier
`PsxCall80043A14_TextAppend(state, arg0, const char* text)` can only represent
literal or caller-prebuilt strings. It cannot represent the PSX MIPS vararg
save-area path used by `%s`, `%d`, `%x`, `%X`, and `%c`.

`800436F0` is the record flush/glyph consumer. It reads the real record bank,
parses `~cXYZ`, emits glyph packets, links through `8004401C`, calls
`800450A0(record+0x10)`, then clears `record+0x28` and the first text byte.
It is not a text producer.

Therefore:

- Do not replace vararg callsites with constant strings.
- Do not route these strings through a Win text renderer as authority.
- Do not delete UI fallback until the PSX record bank, formatter, color fields,
  glyph packet initialization, and flush/link path are closed.

## Callsite matrix

| Callsite | Producer/consumer | Effective PSX input | Literal/non-vararg carrier can accept? | Needs PSX formatter? | Record `+0x04..+0x07` owner | Glyph packet init owner | Delete UI fallback threshold |
|---|---|---|---|---|---|---|---|
| `80026348` | `80043A14` producer | `"\n\n\n\n"` | Yes | No | Not owned here; selected record must already be initialized by `FntLoad/80043438` and any color/link helpers. | Not owned here; flush emits packets later. | Only after append writes real record slot and `800436F0` flush path is authoritative. |
| `80026380` | `80043A14` producer | `"~c000    "` or `"~c888 >>>"` | Yes | No | Not owned here; `~cXYZ` affects consumer-local color in `800436F0`, not record byte ownership. | Not owned here; `800436F0` consumes color control and writes packet RGB. | Same as above; this call alone is not enough to remove fallback. |
| `80026394` | `80043A14` producer | `"%s:"`, `a1 = *(group+0x04)` | No | Yes, `%s` from PSX vararg save area | Not owned here. It depends on selected record state initialized before append. | Not owned here. | Do not delete fallback until a PSX-compatible `80043A14` vararg entry can consume `a1` without pre-collapsing to a hardcoded/Win-rendered string. |
| `80026424` | `80043A14` producer | Stack buffer built by `strcpy/strcat/strlen` in `80026314` | Yes, after caller stack-buffer construction is direct-ported | No inside `80043A14`; caller still owns stack string construction | Not owned here. | Not owned here. | Requires preserving `80026314` local stack-buffer producer and real record append/flush. |
| `80026448` | `80043A14` producer | `"\n"` | Yes | No | Not owned here. | Not owned here. | Same append/flush threshold. |
| `80026470` | `80043A14` producer | `"\n\n~c222      O: OK   X: CANCEL~c888\n"` | Yes | No | Not owned here; color is interpreted by `800436F0`. | Not owned here. | Same append/flush threshold. |
| `80026478` | `800436F0` consumer | `a0 = -1` | No; flush only | No formatter; needs record consumer | Reads `+0x07` for optional final carrier link; does not prove producer semantics. | Emits glyph packets and links them, but packet base/init must come from `80043438/800441C0`. | Requires full `800436F0 -> 800440B8/8004401C -> 800450A0` carrier with record clear side effects. |
| `80026D8C` | `80043A14` producer | `"\n\n\n~c000StageClear: "` | Yes | No | Not owned here. | Not owned here. | Same append/flush threshold, plus StageClear branch guard must remain `event_id == 2 && word_800916F6 != 0`. |
| `80026DAC` | `80043A14` producer | `a0 = &8006EC14`, `a1 = byte_80092F1D[i]`, repeated 6 times | No | Yes, at least leading `%d`; `8006EC14` is format/data, not a safe literal text block | Not owned here. | Not owned here. | Do not delete fallback until PSX formatter can consume `byte_80092F1D[i]` at runtime and `8006EC14` table semantics are decoded/export-backed. |
| `80026DD4` | `800436F0` consumer | `a0 = -1` after draw/wait/end-frame | No; flush only | No formatter; needs record consumer | Reads `+0x07` if final carrier link is enabled. | Emits/link glyph packets from current record text. | Requires same full flush carrier, including clear of `record+0x28` and first text byte. |

## Minimum next export / translation range

### Vararg producer closure

Minimum function/data range:

- `80043A14`: translate the actual formatter path, not only `const char*`
  append. Required cases from current callsites are `%s` and `%d`; the exported
  switch also covers `%x/%X/%c`, width, zero-padding, and `%%`, so the safer
  direct-port unit is the full formatter subgraph inside `80043A14`.
- `80026314`: translate as a caller producer when connecting menu/help text,
  because `80026424` depends on the PSX stack-buffer construction before the
  literal append.
- `80026B94` event-2 tail branch: preserve the exact guarded order
  `80043A14(StageClear literal) -> six 80043A14(&8006EC14, byte_80092F1D[i]) ->
  800436F0(-1)`.
- Data: `8006EC14` and `byte_80092F1D` must stay runtime/data-backed. Static
  bytes prove the leading `"%d\0"` and adjacent labels, not a constant printable
  replacement.

Translation requirement:

- Add a PSX-compatible formatter entry that can receive the MIPS argument save
  area / explicit argument words. Do not pre-collapse `80026394` or `80026DAC`
  into constant strings as the authority.

### Record and color-owner closure

Minimum function range:

- `80043394/FntLoad`: bank clear and slot-limit reset.
- `80043438`: record allocator/initializer. Already proven owner for geometry,
  capacity, text buffer pointer, glyph packet cursor, `record+0x2C`, slot limit,
  and the loop that pre-initializes glyph packets.
- `80044238`: focused export still needed. It is called from the `80043438`
  `mode != 0` branch before explicit zeroing of `record+0x04..+0x06`.
- `800440D0`: focused export still needed. It is called from the same branch
  with `a1 = (mode == 2)` and is the current candidate owner for
  `record+0x07` final-carrier-link semantics.

Current owner classification for `record+0x04..+0x07`:

| Field | Current static owner | Current gap |
|---|---|---|
| `record+0x04` | `80043438` visibly zeros it in `mode != 0`; `800436F0` does not use it for initial glyph color in current Win carrier. | Need `80044238` export to know whether a prior helper initializes packet/header fields before the zero writes. |
| `record+0x05` | Same as `+0x04`. | Same as `+0x04`. |
| `record+0x06` | Same as `+0x04`. | Same as `+0x04`. |
| `record+0x07` | `800440D0(record, mode == 2)` is the likely owner; `800436F0` consumes it for optional final carrier link. | Need `800440D0` export before treating Win `finalCarrierLinkEnabled_07` as complete. |

### Glyph packet init closure

Minimum function range:

- `800441C0`: focused export required. `80043438` calls it once per glyph packet
  slot, then writes `word_8008EB54` into packet `+0x0E`. This is the packet
  initialization owner, not `800436F0`.
- `800436F0`: consumer writes per-glyph runtime fields `+0x04/+0x05/+0x06`
  RGB, `+0x08/+0x0A` XY, `+0x0C/+0x0D` UV, then links through `8004401C`.
- `800440B8`: list-head initialization owner for `record+0x10`.
- `8004401C`: link owner for glyph nodes and optional final record carrier.
- `800450A0`: DrawOTag/submit boundary after glyph list construction.

Current owner classification for glyph packet bytes:

| Packet field | Init/update owner |
|---|---|
| packet base/cursor | `80043438`, from `record+0x20 = 8008AB50 + glyphCursor * 0x10`. |
| packet initial words | `800441C0`, then `80043438` writes packet `+0x0E = word_8008EB54`. |
| packet `+0x04..+0x06` RGB | `800436F0`, from local color state initialized to `0x80/0x80/0x80` and updated by `~cXYZ`. |
| packet `+0x08/+0x0A` XY | `800436F0`, from record geometry and text layout. |
| packet `+0x0C/+0x0D` UV | `800436F0`, from glyph index math. |
| packet link word | `8004401C`, called by `800436F0`. |

## UI fallback deletion gate

The fallback/bridge UI can be deleted only when all of these are true:

- `80043394/80043438` are authoritative for the 8-slot `0x30` record bank,
  including capacity, text pointer, packet cursor, `record+0x2C`, and slot
  selection via `8005CCDC/8005CCE0`.
- `80043A14` has a PSX formatter-capable path that can consume the argument
  words for `80026394("%s:", groupTitle)` and
  `80026DAC(&8006EC14, byte_80092F1D[i])`.
- `80026314` stack-buffer construction is preserved for the `80026424` append.
- `8006EC14` and `byte_80092F1D` remain data/runtime-backed, not collapsed to a
  hardcoded StageClear string.
- `80044238/800440D0` are exported enough to assign `record+0x04..+0x07`
  ownership without guessing.
- `800441C0` is exported enough to initialize glyph packets before
  `800436F0` mutates per-glyph runtime fields.
- `800436F0 -> 800440B8/8004401C -> 800450A0` is the actual flush path and
  clears `record+0x28` plus `*record+0x24` after flush.

Until that gate is met, keep fallback as a bridge/observation seam only. Do not
promote fallback output to text authority.
