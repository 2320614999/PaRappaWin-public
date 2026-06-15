# Movie1 glyph FastSprite RGB stack-local closure - 2026-05-10

Scope: static/IDA evidence for whether `8001B954` / `8001B744` write the
local FastSprite RGB bytes `+0x14/+0x15/+0x16` before calling `8003FA20`.
No build, no test, no simulator, no `src` edits.

## Evidence files

- `docs/stage1_decomp/movie1_fast_sprite_rgb_20260510/ida_export_glyph_rgb_stack_audit_20260510.txt`
  - Current IDB: `E:\game\PS模拟器\parappa the rapper\SCUS_941.83`.
  - `8001B744` xref: `8001C688 -> 8001B744`.
  - `8001B954` xrefs: `8001C6C8/8001C704/8001DB30 -> 8001B954`.
  - `8003FA20` xrefs include `8001B904` from `8001B744` and `8001BBF8`
    from `8001B954`.
  - `8003FA20` consumes RGB with:
    - `8003FAB0: lbu $a2, 0x16($t3)`
    - `8003FAB4: lbu $t0, 0x15($t3)`
    - `8003FAB8: lbu $t1, 0x14($t3)`
- `docs/stage1_decomp/movie1_fast_sprite_rgb_20260510/ida_export_movie_text_common_lyrics_closure_20260510.md`
  - Confirms Stage1/Movie1 text path:
    `8001EC54(ctx,7) -> 8001DB00(ctx+0x10C, work) -> 8001B954(text,480,work)`.
  - `8001B954` pseudo C calls `GsSortFastSprite(&v24, v33, 0)`.
- `docs/stage1_decomp/movie1_fast_sprite_rgb_20260510/ida_export_fast_sprite_rgb_probe.txt`
  - Existing probe shows `8001B744`, `8001B954`, `8001BCA0`,
    `8001B590/8001B25C/8001C550`, and `8003FA20` in the same FastSprite
    family.
- `docs/stage1_decomp/movie1_fast_sprite_rgb_20260510/fast_sprite_rgb_owner_matrix_20260510.md`
  - Existing owner matrix already separates raw draw, transition raw action,
    and text glyph RGB gaps.
- `docs/stage1_decomp/sprite_submit_20260510/ida_export_8001BC48_8001C804_8003F1B4.txt`
  - Global FastSprite comparator: `8001BC48` writes `dword_8007CED0`;
    `8001BCA0` later calls `GsSortFastSprite(&dword_8007CED0, work, 0)`.

## Stack-local audit

Both glyph functions build the FastSprite local at `sp+0x10`:

- `8001B744`: Hex-Rays local `v19 [sp+10h]`, callsite `8001B904`.
- `8001B954`: Hex-Rays local `v24 [sp+10h]`, callsite `8001BBF8`.

Therefore the bytes read by `8003FA20` as local `+0x14/+0x15/+0x16` map to
caller stack bytes `sp+0x24/sp+0x25/sp+0x26`.

The fresh audit lists all stack stores in both functions before the
`GsSortFastSprite` calls:

- `8001B744` writes local/object fields through `sp+0x22`:
  `8001B8FC` (`sh ... var_24`), `8001B900` (`sb ... var_22`), and
  `8001B908` (`sb ... var_21`); then calls `8003FA20` at `8001B904`.
- `8001B954` writes local/object fields through `sp+0x22`:
  `8001BBF0` (`sh ... var_3C`), `8001BBF4` (`sb ... var_3A`), and
  `8001BBFC` (`sb ... var_39`); then calls `8003FA20` at `8001BBF8`.

No `sb`, `sh`, or `sw` to `sp+0x24`, `sp+0x25`, or `sp+0x26` is present in
either stack-store list before the submit call.

## `8001BCA0` / `8001BC48` boundary

`8001BC48` / `8001BCA0` do not close the stack-local glyph RGB gap:

- `8001BC48` writes the global object at `dword_8007CED0`
  (`8001BC68: sw $v0, dword_8007CED0`).
- `8001BCA0` passes the global object to `8003FA20`
  (`8001BDD8: addiu $a0, ..., dword_8007CED0`; call `8001BDEC`).
- This is not the `sp+0x10` local object used by `8001B744` / `8001B954`.

So `dword_8007CED0` evidence is scoped to the global-object path only. It
must not be projected onto the two glyph stack locals.

## Conclusion

The static write chain does not close.

`8003FA20` definitely reads local `+0x14/+0x15/+0x16` as RGB, and
`8001B744` / `8001B954` definitely call it with stack-local FastSprite
objects. But the current IDA stack-store audit shows no write to the
corresponding caller stack bytes `sp+0x24/sp+0x25/sp+0x26` before the calls.

Keep `GapTextGlyphFastSpriteRgb` / `LocalFastSpriteRgbMissing` for
`8001B954` and the same-family `8001B744` comparator. Do not use default RGB,
zero-initialized C++ locals, template bytes, renderer tint, or
`dword_8007CED0` to mark packet color as known.

## Still missing

- A positive producer write to `8001B954` / `8001B744` local
  `+0x14/+0x15/+0x16`, if one exists outside the currently audited static
  chain.
- Or a PSX-grounded decision that these bytes are intentionally uninitialized
  at these callsites. That would require dynamic/local-stack capture at
  `8001BBF8` / `8001B904` or equivalent runtime evidence; it was not run in
  this no-test/no-simulator task.
