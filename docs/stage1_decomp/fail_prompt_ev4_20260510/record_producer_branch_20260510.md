# record producer branch: 80043A14 / 800436F0 / 8005CB5C

Scope: static-only review for the subtitle/text record producer branch. No build,
no test, no `src` edits.

Primary exports:

- `ida_export_record_producer_80043A14_800436F0_20260510.txt`
- `ida_export_record_bank_init_800433xx_20260510.txt`
- Existing references: `ida_export_scus_80026B94_lower_helpers_current_p0_20260510.txt`,
  `ida_export_ev4_table_dispatch_followup.txt`

IDA MCP did not time out in this pass. Both new export files were written under
this directory.

## Verified relationship

`8005CB5C` is the base of a text/font record bank. The PSX stride is `0x30`:

```text
slot_addr = 8005CB5C + slot * 0x30
```

Bank setup is not owned by `80043A14` or `800436F0`:

- `FntLoad` (`80043394`) clears the full bank with `memset(8005CB5C, 0, 0x180)`
  and resets `8005CCDC` to zero. `0x180 / 0x30 = 8`, so static evidence points
  to eight records, not five.
- `80043438` allocates/initializes one record at the current `8005CCDC` slot,
  then increments `8005CCDC`.
- `SetDumpFnt` (`80043354`) selects the current dump slot in `8005CCE0` and
  stores `80043A14` into `off_8005D730`.

The producer/consumer split is:

- Producer: `80043438` is the record-bank initializer/allocator producer. It
  writes geometry, text capacity, active text-buffer pointer, glyph packet
  cursor, active flag/table pointer, and increments the slot limit.
- Producer: `80043A14` is the text append/format producer. It resolves a slot,
  appends chars or formatted `%d/%x/%s/%c` output into `record+0x24` using
  `record+0x28` as write index, enforces `record+0x1C`, and null-terminates.
- Consumer: `800436F0` is the flush/draw consumer. It resolves a slot, reads the
  text pointer and geometry from the record, emits glyph packets through
  `800440B8/8004401C`, calls `800450A0(record+0x10)`, then clears
  `record+0x28` and the first text byte.
- Producers into `80043A14`: `80026314` and the `event_id==2 &&
  word_800916F6` branch in `80026B94` enqueue visible/debug strings before
  flushing.
- Consumers of the produced text: `80026478` and `80026DD4` are callsites into
  `800436F0(-1)`, not independent record producers.

## Address-specific notes

### `80043A14`

Hex-Rays degrades this to `return -1`, but the disassembly is clear:

- If `arg0` is a valid slot (`0 <= arg0 < 8005CCDC`), it writes to that slot.
- Otherwise it falls back to `8005CCE0`.
- It requires the selected record's active pointer/table at `record+0x24` and
  available capacity at `record+0x1C`.
- Plain chars are appended by writing to `*(record+0x24 + record+0x28)`, then
  incrementing `record+0x28`.
- `%d`, `%x`, `%c`, and `%s` are expanded onto a stack scratch buffer and then
  appended through the same buffer/index path.
- It writes a final zero byte at `record+0x24 + record+0x28`.

Conclusion: `80043A14` is the record text producer, not a draw function.

### `800436F0`

Hex-Rays degrades this to `return 0`, but the disassembly shows the full
consumer:

- Slot selection mirrors `80043A14`: valid `arg0` uses that slot, otherwise
  fallback to `8005CCE0`, guarded by `record+0x24`/active state.
- It reads:
  - `record+0x08/+0x0A`: start x/y
  - `record+0x0C/+0x0E`: width/height bounds
  - `record+0x10`: list head / OT head carrier
  - `record+0x1C`: text budget
  - `record+0x20`: glyph packet cursor
  - `record+0x24`: text buffer pointer
  - `record+0x2C`: measure/rewrite mode
- It interprets spaces, tabs, newlines, and `~cXYZ` color controls.
- It writes glyph packet fields and links them through `8004401C`.
- It optionally links the record itself when `record+0x07` is set.
- It calls `800450A0(record+0x10)`, then clears `record+0x28` and the first byte
  at `record+0x24`.

Conclusion: `800436F0` is a consumer/flush side-effect function; it must not be
treated as no-op just because Hex-Rays failed.

### `80026478`

`80026478` is inside `sub_80026314`. That function builds menu/help text via
multiple `80043A14` calls and then calls `800436F0(-1)`.

Conclusion: `80026314` is a higher-level text producer sequence; the exact
address `80026478` is only the consumer flush callsite.

### `80026DD4`

`80026DD4` is inside `80026B94` frame-tail dispatch. For event 2 with
`word_800916F6 != 0`, the code writes:

- `80043A14("\n\n\n~c000StageClear: ")`
- six repeated `80043A14(&8006EC14)` calls using `byte_80092F1D` as the second
  argument

Then every iteration runs:

```text
8001E750(event, handler_arg)
80035560(0)
8001EA00(event)
800436F0(-1)
```

Conclusion: `80026DD4` is also only the frame-tail flush consumer callsite.

## Record layout currently supported by evidence

```text
+0x00 unknown/list packet header word used by final link
+0x04 r byte, initialized/used by font helpers
+0x05 g byte, initialized/used by font helpers
+0x06 b byte, initialized/used by font helpers
+0x07 final-carrier-link flag consumed by 800436F0
+0x08 x
+0x0A y
+0x0C w / rewritten measured width when +0x2C != 0
+0x0E h / rewritten measured height when +0x2C != 0
+0x10 list head / DrawOTag head carrier
+0x1C text capacity / per-flush budget
+0x20 glyph packet cursor
+0x24 text buffer pointer
+0x28 text write index, cleared by 800436F0 after flush
+0x2C measure/rewrite mode
```

## Unknowns still open

- `record+0x00` exact packet/header word semantics. It is used for the optional
  final `8004401C(record+0x10, head, record, record+0x00)` link.
- `record+0x04/+0x05/+0x06/+0x07` producer semantics beyond the visible zeroing
  and `800436F0` consumption. The helper pair `80044238/800440D0` still needs a
  focused export if these flags/colors are to become authoritative.
- `record+0x10` list-head lifecycle across `800440B8`, `8004401C`, and
  `800450A0` is only partially translated here; packet ownership should remain
  explicit until that subgraph is fully closed.
- `8005CB6C`, `8005CB78`, `8005CB7C`, `8005CB80`, `8005CB84`, `8005CB88`,
  `8005CCDC`, `8005CCE0`, and `8005D6E4` need final Win field names. Current
  static meaning is clear enough for wiring, but naming should wait until the
  helper closure is done.
- `8006EC14` is a format/data table used by the StageClear debug path; its exact
  vararg behavior through `80043A14` should not be collapsed into hardcoded text.
- `byte_80092F1D` supplies the second argument for the six StageClear producer
  calls; its runtime value is not established by this static pass.

## Next main-thread wiring

1. Add a real text record bank state seeded from the PSX `FntLoad/80043438`
   contract, with `0x30` stride and eight slots unless later evidence proves a
   narrower active subset. Do not keep the bank as a bridge-only gap.
2. Port `80043438` before treating `80043A14` as authoritative: it owns the
   record geometry, text capacity, text buffer pointer, glyph packet cursor, and
   slot limit (`8005CCDC`).
3. Add a direct wrapper for `80043A14` that writes into that bank. Callers such
   as `80026314` and the StageClear branch should enqueue strings into the bank,
   not bypass it through `8001EC54/8001ED3C` fast-sprite bridge data.
4. Keep `800436F0(-1)` as the consumer flush. It should read the selected record
   from the real bank, emit/link glyph packets, call the `800450A0` path, then
   clear `record+0x28` and `*record+0x24`.
5. Treat `80026478` and `80026DD4` as flush callsites only. The producer work
   happens earlier in their caller bodies through `80043A14`.
6. Do not delete existing fallback/bridge logs until the real `8005CB5C` bank
   producer closes `textRecordBankGap`, `textRecordSlotGap`,
   `textRecordGeometryGap`, `textRecordPacketCursorGap`, and `glyphPacketGap`.

