# text264/text268 event-stream asymmetry (2026-05-12)

Scope: explain why `dword_8008ECE0` / `word_8008ECF8` / `ctx+0x108`
still have no non-zero producer while sibling `dword_8008ECE4` /
`word_8008ECFA` / `ctx+0x10C` does. Authority is the current SCUS IDA
pseudocode for `80024C84`, `80024CF8`, and `80024E98`, plus the existing
instruction window in `ida_export_text264_stageclear_sources_20260511.txt`.

## Static facts

- `80024E98` resets the shared block by calling `80025C44(&dword_8008ECE0,
  244)`, then initializes stage-event globals. This is a reset/clear path, not
  a producer for `dword_8008ECE0`.
- `80024C84(desc)` clears `word_8008ECFA`, `dword_8008ECE4`, `gp+0x37C`, and
  `gp+0x320`. If `desc` is non-null, it maps descriptor fields into the event
  stream globals:
  - `gp+0x36C` / `v1[219]` = `desc[6]` entry count.
  - `gp+0x370` / `v1[220]` = `desc[5]` entry table.
  - `gp+0x374` / `v1[221]` = `desc[word_800916D8]` selected text table.
  - `gp+0x37C` / `v1[223]` = cursor reset to zero.
- `80024CF8(ctx)` first decrements the primary timer `word_8008ECF8`; when it
  reaches zero, it clears `dword_8008ECE0` and `ctx+0x108`. There is no branch
  in this function that writes a non-zero `dword_8008ECE0`.
- `80024CF8(ctx)` then decrements secondary timer `word_8008ECFA`; when it
  reaches zero, it clears `dword_8008ECE4` and `ctx+0x10C`.
- `80024CF8(ctx)` advances the event stream only through the secondary path:
  it compares `gp+0x37C < gp+0x36C`, reads the current 16-byte row from
  `gp+0x370 + 16 * cursor`, compares row timecode against `ctx+4/6/7`,
  increments `gp+0x37C`, reads text index at `row+6+2*word_800916D8`, then if
  `textIndex > 0` writes:
  - `dword_8008ECE4 = *(gp+0x374 + 4 * textIndex)`;
  - `word_8008ECFA = row[2]`;
  - `ctx+0x10C = dword_8008ECE4`.

## Boundary

- Current evidence supports `text268` as the live event-stream text channel in
  this closure.
- Current evidence does not support any non-zero `text264` producer in
  `80024C84/80024CF8/80024E98`, nor in the already exported exact xrefs.
- Do not chase `8001B954` / `8003FA20` glyph or FastSprite consumers for this
  producer; they are downstream consumers and cannot create the missing
  `dword_8008ECE0` producer authority.
- Next useful static step, if this channel must be closed further, is to
  identify every caller/descriptor passed to `80024C84` and prove whether
  primary text is intentionally unused for Stage1 or produced by a different
  overlay path.
