# Fast/local sprite RGB owner matrix - 2026-05-10

Scope: P0-C3 static owner trace for `8001C550/8001B590/8001B25C/8001C804/8001B338`.

Rules applied:

- Static only. No build, no tests, no simulator, no memory replay.
- Use existing IDA/export artifacts and source grep only.
- RGB ownership means a proven write to local sprite hex offsets `+0x14/+0x15/+0x16` before submit reads them.
- If no write is proven, status stays `unknown/gap`; do not default to `0x80`, `0`, white, renderer tint, or template-derived RGB.

## Executive conclusion

No chain in the requested static scope proves an owner write for local sprite RGB bytes `+0x14/+0x15/+0x16`.

The strongest evidence is negative:

- `8003FA20 GsSortFastSprite` reads local fast sprite `+0x16/+0x15/+0x14` as B/G/R at `8003FAB0/8003FAB4/8003FAB8`.
- `8003F1B4 GsSortSprite` reads local GsSprite `+0x16/+0x15/+0x14` in both fast and transform paths.
- `8001B25C` writes local fast sprite offsets `+0x0E/+0x0F/+0x10/+0x12`, not hex `+0x14/+0x15/+0x16`.
- `8001B338` writes local GsSprite offsets `+0x0E/+0x0F/+0x10/+0x12/+0x18/+0x1A/+0x1C/+0x1E`, not hex `+0x14/+0x15/+0x16`.
- `8001B590`, `8001C550`, and `8001C804` are wrappers/stack local callers and do not show RGB writes.
- `80024418` initializes a stack sprite prefix from `dword_8005405C` plus local X/Y, then calls `8001C804`; it does not prove RGB writes.

Therefore deletion/cutover gate remains blocked for packet-complete RGB. The next connection step may keep metadata/gap carriers, but must not set `localFastSpriteKnown`, `localGsSpriteKnown`, or packet color known from guessed values.

## Owner matrix

| Chain | Caller | Wrapper | Builder | Submit | RGB write to local `+0x14/+0x15/+0x16` proven? | Evidence | Delete/connect gate |
|---|---|---|---|---|---|---|---|
| Compact rail portrait/marker fast sprite | `80024744` | `8001C550 -> 8001B590` | `8001B25C` | `8003FA20` | No, gap | `80024744` calls `8001C550` at `80024850/80024908/8002499C/80024A28`; `8001C550` only forwards to `8001B590`; `8001B590` creates a stack local, sets X/Y, calls `8001B25C`, then `GsSortFastSprite`; `8001B25C` writes UV/CLUT fields only; `8003FA20` reads RGB. | Do not connect complete packet RGB. Keep `localFastSpriteRgb8001C550` unknown and `StaticChainHasNoRgbWrite` until an authoritative owner is found. |
| Event4 prompt fast sprite | `800203D4` | `8001C550 -> 8001B590` | `8001B25C` | `8003FA20` | No, gap | `event4_prompt_sprite_packet_plan_20260510.md` records `800203D4 -> 8001C550 -> 8001B590 -> 8001B25C -> 8003FA20`; `pr_psx_event_frame_direct.cpp` builds the prefix only and explicitly marks `localSpriteRgbKnown=false`, `localSpriteRgbGap=true`, and `packetColorWordKnown=false`. | Event4 can route through submit metadata, but deletion of UI/fallback or complete packet color is blocked until RGB owner is proven. |
| Movie/raw fast sprite bridge | movie/raw callers via `8001C550` or direct `8001B590` | `8001C550` or direct `8001B590` | `8001B25C` | `8003FA20` | No, gap | `pr_stage1_scene1_movie1_direct.cpp` reconstructs `8001B25C` fields and sets `localSpriteRgbKnown=false`, `localSpriteKnown=false`; `BuildRawDrawFastSpriteLocalSub8001B590()` only fills attr, XY, WH, tpage, U/V, CLUT. | Existing movie bridge remains a reference for prefix/runtime, not proof of RGB ownership. Do not reuse it to close rail/event4 RGB. |
| Movie1 text glyph | `8001DB00` callsite `8001DB30` | `8001B954` | local glyph builder in `8001B954` | `8003FA20` | No, gap | `8001EC54(ctx,7) -> 8001DB00(ctx+0x10C, work) -> 8001B954(text,480,work)` proves the work/OT owner, but current `8001B954` evidence only proves glyph metric, position, UV, CLUT and tpage fields. It does not prove stack-local RGB writes. | Keep `LocalFastSpriteRgbMissing`; code carries `Movie1TextGlyphRgbGapEvidenceSub8001B954` and explicitly marks `dword_8007CED0` non-applicable to this stack local. |
| Compact rail body note GsSprite | `80024744 -> 80024418` | `8001C804` | `8001B338` | `8003F1B4` | No, gap | `80024744` calls `80024418`; `80024418` stack-initializes attr and X/Y then calls `8001C804`; `8001C804` calls `8001B338` then `GsSortSprite`; `8001B338` writes template, UV/CLUT, scale, and midpoint fields only; `8003F1B4` reads RGB from local `+0x14/+0x15/+0x16`. | Do not connect complete body-note packet RGB. Keep `localGsSpriteRgb8001C804` unknown and `StaticChainHasNoRgbWrite` until owner is found. |

## Function-level write/read matrix

| Function | Role | Proven writes relevant to local sprite | Proven reads relevant to local sprite | RGB owner status |
|---|---|---|---|---|
| `8001C550` | Fast sprite wrapper | No local sprite field writes. It forwards args to `8001B590(a1, a2, template, 0, 0, priority, work)` using current `80087288 + 20 * drawBuffer`. | None. | Not owner. |
| `8001B590` | Fast sprite stack local wrapper | Stack local X/Y only, then calls `8001B25C`. No hex `+0x14/+0x15/+0x16` write in disasm. | Passes stack local to `8003FA20`. | Not proven owner. |
| `8001B25C` | Fast sprite template builder | Writes attr `+0x00`, WH `+0x08/+0x0A`, tpage `+0x0C`, U/V `+0x0E/+0x0F`, CLUT `+0x10/+0x12`. | Reads template fields `+0x00/+0x04/+0x06/+0x08/+0x0A/+0x0C/+0x0E`. | Not RGB owner; no hex `+0x14/+0x15/+0x16` write proven. |
| `8003FA20` | Fast sprite submit | Writes packet words and OT/allocator, not local sprite RGB. | Reads local `+0x16`, `+0x15`, `+0x14` at `8003FAB0/8003FAB4/8003FAB8` and packs them into packet word at `8003FB0C`. | RGB consumer only. |
| `8001C804` | GsSprite wrapper | No RGB writes. Calls `8001B338`, then `8003F1B4` with priority `1`. | Passes same caller-provided local sprite to submit. | Not owner. |
| `8001B338` | GsSprite template builder | Writes WH `+0x08/+0x0A`, tpage `+0x0C`, U/V `+0x0E/+0x0F`, CLUT `+0x10/+0x12`, midpoint `+0x18/+0x1A`, scale `+0x1C/+0x1E`. | Reads template fields and scale args. | Not RGB owner; no hex `+0x14/+0x15/+0x16` write proven. |
| `8003F1B4` | GsSprite submit | Writes packet words and OT/allocator, not local sprite RGB. | Reads local `+0x16/+0x15/+0x14` in fast path and transform path. | RGB consumer only. |
| `80024418` | Rail body-note stack local caller | Initializes stack local attr from `dword_8005405C`, X/Y from note args, then calls `8001C804`. No RGB write proven in current export. | Passes stack local to `8001C804`. | Not proven owner. |
| `8001B954` | Text glyph loop stack local caller | Builds glyph local FastSprite geometry from `byte_8004945C` metrics and text origin. No proven write to local `+0x14/+0x15/+0x16`. | Calls `8003FA20` for drawable glyphs whose metric word at `+2` is non-negative. | Work/OT owner known through `gp+872`; RGB owner still not proven. |
| `8001B744` | Same-family glyph comparator | Current stack write graph also lacks proven writes to local `+0x14/+0x15/+0x16`. | Calls the same FastSprite submit family. | Comparator only; cannot close `8001B954` RGB. |
| `8001BCA0` | Global FastSprite path | Uses global object `dword_8007CED0` rather than a caller stack local. | Calls `8003FA20(&dword_8007CED0, work, 0)`. | Its global-object RGB evidence is scoped to `8001BCA0` only and must not be projected to `8001B954/8001B744`. |

## Evidence map

| Evidence file/source | Evidence used |
|---|---|
| `docs/stage1_decomp/movie1_fast_sprite_rgb_20260510/disasm_8001B590_stack.txt` | `8001B590` stack flow calls `8001B25C` then `GsSortFastSprite`; `8003FA20` reads `0x16/0x15/0x14` as RGB. |
| `docs/stage1_decomp/movie1_fast_sprite_rgb_20260510/ida_export_8001B590_caller_stack_matrix.txt` | Full disasm/pseudocode for `8001B25C`, `8001B338`, `8001B590`, `8001C550`, `8001C804`, `80024418`, and `8003FA20`; confirms builder write offsets and wrapper calls. |
| `docs/stage1_decomp/sprite_submit_20260510/ida_export_8001BC48_8001C804_8003F1B4.txt` | `8001C804 -> 8001B338 -> 8003F1B4`; `8003F1B4` local RGB reads; `80024744` calls to `8001C550` and `80024418`. |
| `docs/stage1_decomp/sprite_submit_20260510/rail_gte_next_cutover_plan_20260510.md` | Existing P0 gate: `8001C550/8001C804` RGB owner not found; source intentionally marks `StaticChainHasNoRgbWrite`. |
| `docs/stage1_decomp/fail_prompt_ev4_20260510/event4_prompt_sprite_packet_plan_20260510.md` | Event4 prompt chain and explicit rule that RGB tail stays unresolved; no default host RGB/renderer tint. |
| `src/pr/pr_stage1_compact_rail_80024744_direct.cpp/.h` | Rail source keeps RGB as external carriers and only writes local `r_14/g_15/b_16` if carrier is known. Unknown carrier becomes `StaticChainHasNoRgbWrite`. |
| `src/pr/pr_psx_event_frame_direct.cpp/.h` | Event4 prompt direct path builds local fast sprite prefix and marks RGB/packet color unknown. |
| `src/pr/pr_stage1_scene1_movie1_direct.cpp/.h` | Movie/raw fast sprite bridge reconstructs `8001B25C` prefix and keeps local RGB unknown. |

## 2026-05-10 Task C recheck

- No new static evidence in this folder proves an `8001B590` stack-local RGB tail producer.
- `801C448C/801C4350` only proves movie timecode writes into the `801C3640` context (`ctx+4/+6/+7`) and `dword_801C954C`; it does not produce fast-sprite RGB.
- `801C455C` / movie outer-loop exports show `sub_800201AC(&unk_801C3640, ...)` and play-loop wiring, but do not add a new `801C3640` word0/word1 owner beyond the existing context carrier.
- Code-side gap classification is now split so the old broad `GapFastSpriteStackRgb` does not mix:
- `GapRawDrawFastSpriteStackRgb`: generic `8001B590 -> 8001B25C -> 8003FA20` stack-local RGB tail missing.
- `GapTransitionRawActionFastSpriteStackRgb`: transition raw actions such as `8001FDC0/8001D74C/80022CBC/80021E60` still reach the same RGB-tail gap.
- `GapTextGlyphFastSpriteRgb`: reserved for text glyph submit paths (`8001B954/8001B744/8001BCA0`) so glyph RGB evidence stays separate from Movie1 raw draw.
- 2026-05-10 text glyph narrowing: Stage1's confirmed text path is `8001DB00 -> 8001B954`; `8001BCA0/dword_8007CED0` is a separate global-object path, not a stack-local RGB source for `8001B954`.
- No fallback was added, no default RGB was assigned, and `8003FA20` remains blocked unless the local sprite is packet-complete.

## Next-round connection gate

Allowed next:

- Keep wrapper/builder/submit metadata and runtime owner wiring.
- Carry explicit `rgbUnknown` / `StaticChainHasNoRgbWrite` / packet color gap fields.
- If dynamic work is later allowed, read PSX local sprite bytes immediately before `8003FA20` and `8003F1B4` submit for the relevant call sites.

Blocked until owner proof:

- Setting `localFastSpriteKnown` or `localGsSpriteKnown` true for these chains.
- Marking `packetColorWordKnown` true.
- Deleting fallback/render paths that still depend on complete color packet parity.
- Replacing unknown RGB with fixed defaults, template bytes, host tint, or renderer defaults.
