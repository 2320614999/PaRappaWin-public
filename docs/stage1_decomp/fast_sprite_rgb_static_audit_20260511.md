# FastSprite RGB Tail Static Audit 2026-05-11

Scope: event-frame visible FastSprite blockers for event4 Prompt and event2
StageSelect. Authority is existing IDA MCP / Hex-Rays and disassembly exports;
memory replay may only identify which caller enters `8003FA20`, not define RGB
semantics.

## Consumer

`8003FA20 GsSortFastSprite` consumes the local object RGB tail:

- `a0 + 0x14` -> R
- `a0 + 0x15` -> G
- `a0 + 0x16` -> B

Evidence:

- `tools/auto_port/output/movie_fast_sprite_adjacent_rgb_20260509/decompile_8003fa20.c`
- `tools/auto_port/output/movie_fast_sprite_adjacent_rgb_20260509/disasm_8003fa20.asm`
- `tools/auto_port/output/movie_fast_sprite_adjacent_rgb_20260509/summary.md`

## `8001B590 -> 8001B25C -> 8003FA20`

Static facts:

- `8001B590` builds the FastSprite local object at stack base `sp + 0x10`.
- `8001B590` writes only object `+0x04/+0x06` for screen-centered X/Y.
- `8001B25C` writes object `+0x00`, `+0x08`, `+0x0A`, `+0x0C`,
  `+0x0E`, `+0x0F`, `+0x10`, and `+0x12`.
- No static writer in this closure writes object `+0x14..+0x16`.
- Submit callsite is `8001B5D4`; return address after `8003FA20` is
  `8001B5DC`.

Evidence:

- `tools/auto_port/output/movie_fast_sprite_adjacent_rgb_20260509/decompile_8001b590.c`
- `tools/auto_port/output/movie_fast_sprite_adjacent_rgb_20260509/disasm_8001b590.asm`
- `tools/auto_port/output/movie_fast_sprite_adjacent_rgb_20260509/decompile_8001b25c.c`
- `tools/auto_port/output/movie_fast_sprite_adjacent_rgb_20260509/disasm_8001b25c.asm`
- `docs/stage1_decomp/movie1_fast_sprite_rgb_20260510/ida_export_8001B590_caller_stack_matrix.txt`

## `8001B654 -> 8001B4E0 -> 8003FA20`

Static facts:

- `8001B654` builds the FastSprite local object at stack base `sp + 0x18`.
- `8001B654` writes only object `+0x04/+0x06` for screen-centered X/Y, plus
  an extra stack argument outside the object.
- `8001B4E0` writes object `+0x00`, `+0x08`, `+0x0A`, `+0x0C`,
  `+0x0E`, `+0x0F`, `+0x10`, and `+0x12`.
- No static writer in this closure writes object `+0x14..+0x16`.
- Submit callsite is `8001B6A4`; return address after `8003FA20` is
  `8001B6AC`.

Evidence:

- `tools/auto_port/output/movie_fast_sprite_adjacent_rgb_20260509/decompile_8001b654.c`
- `tools/auto_port/output/movie_fast_sprite_adjacent_rgb_20260509/disasm_8001b654.asm`
- `tools/auto_port/output/movie_fast_sprite_adjacent_rgb_20260509/decompile_8001b4e0.c`
- `tools/auto_port/output/movie_fast_sprite_adjacent_rgb_20260509/disasm_8001b4e0.asm`

## Implementation Boundary

Current direct-port code must keep event-frame StageSelect/Prompt FastSprite
packets as partial/audit-only until a static RGB owner is proven.

Replay entry-only sampling for the available Stage1 recording is stored in
`fast_sprite_8003fa20_entry_trace_20260511.md`. It confirms active
`8003FA20` callers by `ra/pc/frame` only and intentionally does not read or
use `a0+0x14..0x16`.

Allowed:

- Use replay call trace to confirm which `8003FA20` callers are active in a
  recording.
- Use static IDA pseudocode/disassembly to find a real writer for
  `local + 0x14..+0x16`.
- Keep packet geometry/texture/OT side effects mirrored with unknown word2.
- Require `colorAuthoritative` before a FastSprite packet can become
  `renderPayloadKnown` or reach the backend draw gate.
- `StaticLocalFields` and `StaticGlobalObject` are the only current
  authoritative color provenance kinds; stack/replay residue remain audit-only.

Forbidden:

- Do not fill RGB from replay sampled bytes.
- Do not use default white, template color, renderer color, or C++ zero-init.
- Do not draw event-frame FastSprite packets whose `word2` is not statically
  known.
- Do not treat PSX stack residue or replay reconstructed residue as
  `HasKnownRgbTail8003FA20`.
- Do not treat a raw packet `wordKnown[2]` bit alone as proof that the color is
  authoritative.
