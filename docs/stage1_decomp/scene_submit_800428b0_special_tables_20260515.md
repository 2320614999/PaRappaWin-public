# Scene Submit `800428B0` flag4 special tables

Authority:

- New current-session IDA export:
  `docs/stage1_decomp/ida_export_800428b0_special_tables_8008eed8_8008eee4_20260515.json`.
- This evidence only covers the `800428B0` dispatch/xref shape. It does not
  prove a runtime writer or handler target for the special tables.

Facts:

- `0x20 + flag&4` branches through `dword_8008EED8[v22]`.
  The `800428B0` cursor advance remains the PSX input stride:
  `6 words / 12 halfwords`.
- `0x30 + flag&4` branches through `dword_8008EEE4[v22]`.
  The `800428B0` cursor advance remains:
  `7 words / 14 halfwords`.
- Current IDA xrefs to `8008EED8` and `8008EEE4` are only the read sites in
  `800428B0` at `80042A98` and `80042B64`.
- `8001C1E8` initializes the standard `8008EDD8/8008EDF8/...` tables visible
  in the normal dispatch path, but this export shows no assignment to
  `8008EED8` or `8008EEE4`.

Runtime boundary:

- C++ continues to fail closed for these two special paths.
- `PsxGpuPacketCommand428B0` now exposes observable gap metadata when a special
  path is encountered: `dispatchGap800428B0 =
  Flag4SpecialTableUnsupported`, `specialTableBase800428B0`, raw primitive
  packet metadata, and `handlerTargetKnown800428B0=false`.
- The emitted gap command is `valid=false`, `handler=Unsupported`, and
  `wordCount=0`, so the existing backend renderer ignores it. This does not
  authorize packet rendering, GTE output, `FLAG/IR0`, or handler return facts.

Next authority requirement:

- Do not fold `8008EED8/8008EEE4` to normal `F3NL/G3NL` handlers.
- Do not use static BSS zero, Win renderer output, TMD parser shape, ISO/file
  readability, or visual sampling as handler authority.
- If this path becomes runtime-relevant, the next evidence must be a true writer
  or live/runtime table value for `8008EED8/8008EEE4`.
