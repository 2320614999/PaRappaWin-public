# text record runtime review 2026-05-10

Scope: static-only review of current `PrPsxEventFrameDirect` text record producer/flush code against the exported `80043394`, `80043438`, `80043A14`, `800436F0`, and `80043354` evidence. No build, no test, no `src` edits.

Primary local evidence:

- `src/pr/pr_psx_event_frame_direct.cpp`
- `src/pr/pr_psx_event_frame_direct.h`
- `ida_export_record_bank_init_800433xx_20260510.txt`
- `ida_export_record_producer_80043A14_800436F0_20260510.txt`
- `record_producer_branch_20260510.md`
- `80043A14_callsite_order_20260510.md`

## Static conclusion

Current record-bank shape is mostly on the right route: 8 records, `0x30` stride, `80043394` clearing `0x180`, `80043438` producing record slots, `80043A14` appending to `record+0x24` through `record+0x28`, and `800436F0` flushing then clearing `record+0x28` plus first text byte.

The obvious static deviation is not initial RGB. `800436F0` initializes glyph RGB from stack locals fixed to `0x80`, then only changes them through `~cXYZ`; current flush code matches that. `record+0x04..0x06` should not be used as the flush initial RGB source based on the current disasm.

The clearest record-field mismatch is `80043438`'s `record+0x2C`: PSX writes `record+0x2C = (a2 == 0)` before flush later reads it as the measure/rewrite flag. Current code always sets `measureMode_2C = 0`, so zero-width/measure-mode records will not match PSX.

## Address checks

### `800436F0` initial RGB

Evidence:

- `800436F4: li $a2, 0x80`
- `800436F8/800436FC/80043730` store that same `0x80` into the three color locals.
- Glyph packet writes at `800438EC..8004390C` copy those locals into packet `+04/+05/+06`.
- `~cXYZ` at `80043844..80043880` overwrites the locals from text control bytes.

Current code:

- Starts `r/g/b` as `0x80`.
- Writes packet `r_04/g_05/b_06` from those locals.
- Updates them on `~cXYZ`.

Conclusion: no obvious deviation here. Keep initial RGB fixed `0x80`; do not derive it from `record+04..06`.

### `record+04..07`

Evidence:

- `80043438` with nonzero `a5` calls `80044238(record)`, zeroes `record+04/+05/+06`, then calls `800440D0(record, a5 == 2)`.
- `800436F0` does not read `record+04/+05/+06` for glyph RGB.
- `800436F0` reads `record+07` at `80043980` and, if nonzero, links the record itself via `8004401C(record+0x10, record)`.

Current code:

- Has fields `r_04/g_05/b_06/finalCarrierLinkEnabled_07`.
- Zeroes `+04..06` when `mode != 0`.
- Uses `mode == 2` as `+07`.
- Flush consumes `+07` only for final carrier link.

Conclusion: current `+04..06` not being used as initial color is correct. The `+07 = (mode == 2)` assumption is plausible from the local branch, but still depends on `800440D0` internals; keep it marked evidence-dependent until that helper is exported.

### `80043354` slot <= limit

Evidence:

- `8004335C` loads `8005CCDC`.
- `80043368: slt $v0, $a0` with loaded limit in `$v0` means `limit < slot`.
- `8004336C bnez` returns when `slot > limit`.
- Therefore accepted domain is `slot >= 0 && slot <= 8005CCDC`.

Current code:

- Returns for `slot < 0 || slot > state.textFlush.slotLimit8005CCDC`.

Conclusion: current inclusive `slot <= limit` behavior matches the disasm. Do not change it to `< limit` unless later evidence proves `8005CCDC` has been renamed/advanced differently at the call boundary.

### `80043438` record fields

Evidence-backed field writes:

```text
0x08 = a0 x
0x0A = a1 y
+0x0C = a2 w
+0x0E = a3 h
+0x1C = clamped capacity
+0x20 = 8008AB50 + glyphCursor * 0x10
+0x24 = 8008A750 + glyphCursor
+0x28 = 0
+0x2C = (a2 == 0)
```

Current code matches `+08/+0A/+0C/+0E/+1C/+20/+24/+28` well enough for the current static pass.

Deviation:

- `record.measureMode_2C = 0` is hardcoded.
- PSX writes `dword_8005CB88(slot) = (a2 == 0)`, i.e. `record+0x2C = (w == 0)`.

Direct correction candidate:

- In `PsxCall80043438_TextRecordAlloc`, set `record.measureMode_2C = (w == 0) ? 1 : 0`.
- Preserve the current flush behavior that rewrites `record+0x0C/+0x0E` only when `measureMode_2C != 0`.

### `80043A14` producer

Evidence:

- Valid `arg0` uses that slot; invalid/negative falls back to `8005CCE0`.
- It appends into `record+0x24 + record+0x28`, increments `record+0x28`, checks `record+0x1C`, and null-terminates.
- It handles `%d`, `%x/%X`, `%c`, and `%s` using stack scratch and vararg reads.

Current code:

- Implements slot fallback, capacity check, append, null termination, and write-index update.
- Does not implement PSX vararg formatting; the API only accepts `const char* text`.

Direct correction candidate:

- Keep the existing literal append path, but do not claim full `80043A14` authority until a PSX-compatible vararg/input wrapper exists.
- For callsites with `"%s:"` or `8006EC14` formats, either port the caller-side prepared string exactly or add typed vararg support; do not collapse these to hardcoded visible text.

## Directly fixable items

1. Set `record.measureMode_2C` from `w == 0` in `PsxCall80043438_TextRecordAlloc`.
2. Keep `800436F0` initial RGB fixed to `0x80`; explicitly avoid using `record+04..06` as initial color.
3. Keep `80043354` accepting `slot == slotLimit8005CCDC`; do not "fix" it to strict `<`.
4. Keep `record+07` as a final-carrier-link flag only, not part of initial color.
5. Treat current `PsxCall80043A14_TextAppend` as literal-append partial coverage until vararg callsite handling is added.

## IDA/export gaps

1. Export `800440D0` and `80044238` to prove exact `record+00/+04/+05/+06/+07` helper semantics. Current `+07 = (mode == 2)` is inferred from `80043438` call arguments and `800436F0` consumption, not from helper internals.
2. Export/annotate `80045934`, `800441C0`, `800440B8`, `8004401C`, and `800450A0` as one small closure if packet/list ownership must become authoritative.
3. Export concrete `80043438` callsites beyond `80027FAC` if nonzero/zero width records exist outside the currently reviewed boot path.
4. Export or map `80043A14` vararg callsite inputs for `"%s:"`, `8006EC14`, and any `%d/%x/%c` users before replacing caller text generation.
5. Confirm whether negative `capacity` can ever reach `80043438`; current code clamps it to zero, while the disasm only clamps overflow against the shared `0x400` glyph/text pool.
