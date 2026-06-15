# Scene Submit 800428B0 / GsTMDfast Packet Side Effects - 2026-05-15

Authority:

- New IDA current-session export:
  `docs/stage1_decomp/ida_export_800428b0_gstmdfast_packet_sideeffects_20260515.json`.
- Prior dispatch export:
  `docs/stage1_decomp/ida_export_800428b0_dispatch_table_recheck_20260515.txt`.
- Prior four-point family export:
  `docs/stage1_decomp/sprite_submit_20260510/ida_export_gstmdfast_family_8003bd9c_8003bf04_8003c91c_8003ca9c_8003d58c_8003d72c_8003e26c_8003e428_20260513.txt`.

This is an evidence / graph artifact. It does not authorize renderer
fallbacks, Win-side projection, replay-sampled packets, or setting GTE fields
known without a PSX-authoritative facts source.

## Confirmed Current-Session Targets

| Function | IDA status | Key facts |
| --- | --- | --- |
| `800428B0` | function found | callers `8001B084`, `801C5E60`; no GTE ops; dispatch key is `primitive.mode & 0xFD`; dispatches through `jalr $v0`; writes `dword_800901C8` from the handler return |
| `8001C1E8` | function found | initializes `8008EDD8..8008EED4` handler table slots; static BSS table bytes are zero in the IDB |
| `8003BD9C` | function found, `GsTMDfastNF4` | four-point GTE/packet writer |
| `8003BF04` | function found, `GsTMDfastF4NL` | four-point GTE/packet writer |
| `8003C91C` | function found, `GsTMDfastNG4` | four-point GTE/packet writer |
| `8003CA9C` | function found, `GsTMDfastG4NL` | four-point GTE/packet writer |
| `8003D58C` | function found, `GsTMDfastTNF4` | four-point GTE/packet writer |
| `8003D72C` | function found, `GsTMDfastTF4NL` | four-point GTE/packet writer |
| `8003E26C` | function found, `GsTMDfastTNG4` | four-point GTE/packet writer |
| `8003E428` | function found, `GsTMDfastTG4NL` | four-point GTE/packet writer |

## 800428B0 Dispatch / Cursor Graph

`800428B0` dispatch key is `(*((byte*)prim + 3) & 0xFD)`. Cursor advance below
is the TMD primitive input cursor advance, not GPU packet word count.

| Key | Handler table source | Handler | GPU packet words | Primitive cursor advance |
| --- | --- | --- | --- | --- |
| `0x20` | `8008EDD8[slot 2]` | `F3NL` | 5 | 4 words / 16 bytes |
| `0x21` | `8008EDF0[slot 6]` | `NF3` | 5 | 4 words / 16 bytes |
| `0x24` | `8008EE18[slot 18]` | `TF3NL` | 8 | 6 words / 24 bytes |
| `0x25` | `8008EE30[slot 22]` | `TNF3` | 8 | 7 words / 28 bytes |
| `0x28` | `8008EE58[slot 34]` | `F4NL` | 6 | 5 words / 20 bytes |
| `0x29` | `8008EE70[slot 38]` | `NF4` | 6 | 4 words / 16 bytes |
| `0x2C` | `8008EE98[slot 50]` | `TF4NL` | 10 | 8 words / 32 bytes |
| `0x2D` | `8008EEB0[slot 54]` | `TNF4` | 10 | 8 words / 32 bytes |
| `0x30` | `8008EDF8[slot 10]` | `G3NL` | 7 | 5 words / 20 bytes |
| `0x31` | `8008EE10[slot 14]` | `NG3` | 7 | 6 words / 24 bytes |
| `0x34` | `8008EE38[slot 26]` | `TG3NL` | 8 | 7 words / 28 bytes |
| `0x35` | `8008EE50[slot 30]` | `TNG3` | 10 | 9 words / 36 bytes |
| `0x38` | `8008EE78[slot 42]` | `G4NL` | 9 | 6 words / 24 bytes |
| `0x39` | `8008EE90[slot 46]` | `NG4` | 9 | 7 words / 28 bytes |
| `0x3C` | `8008EEB8[slot 58]` | `TG4NL` | 10 | 9 words / 36 bytes |
| `0x3D` | `8008EED0[slot 62]` | `TNG4` | 13 | 11 words / 44 bytes |

Special paths remain unsupported in current C++:

| Key | Branch | Handler table source | Primitive cursor advance |
| --- | --- | --- | --- |
| `0x20` | `flag & 4` | `8008EED8[...]` | 6 words / 24 bytes |
| `0x30` | `flag & 4` | `8008EEE4[...]` | 7 words / 28 bytes |

Current C++ covers all 16 standard keys above through
`PsxResolveGsTmdFastHandler800428B0(...)`. It intentionally rejects the two
`flag & 4` special paths instead of folding them into a standard handler.

## 8001C1E8 Table Boundary

The new export confirms `8001C1E8` writes the handler globals, but the static
IDB bytes at `8008EDD8..8008EEE4` read as zero because they are BSS/runtime
initialized. Therefore:

- use `8001C1E8` writes and function xrefs as table authority;
- do not treat static zero table bytes as runtime table values;
- do not derive handler entries from renderer output or TMD parser convenience
  order.

## Four-Point Packet Write Shape

The current-session narrow export confirms each four-point function has GTE
ops and packet/OT memory writes. The packet cursor register differs by family:
some write through `$a2`, some through `$a3`; this remains explicit when
porting handler side effects.

| Function | Packet cursor | Packet write offsets seen | OT link write |
| --- | --- | --- | --- |
| `8003BD9C` `NF4` | `$a2` | `0x04,0x08,0x0C,0x10,0x14`, then word0 at `0x00` | `sw ...,0($t6)` |
| `8003BF04` `F4NL` | `$a3` | `0x04,0x08,0x0C,0x10,0x14`, then word0 at `0x00` | `sw ...,0($t6)` |
| `8003C91C` `NG4` | `$a2` | `0x04,0x08,0x0C,0x10,0x14,0x18,0x1C,0x20`, then word0 at `0x00` | `sw ...,0($t6)` |
| `8003CA9C` `G4NL` | `$a3` | `0x04,0x08,0x0C,0x10,0x14,0x18,0x1C,0x20`, then word0 at `0x00` | `sw ...,0($t6)` |
| `8003D58C` `TNF4` | `$a2` | `0x04,0x08,0x0C,0x10,0x14,0x18,0x1C,0x20,0x24`, then word0 at `0x00` | `sw ...,0($t6)` |
| `8003D72C` `TF4NL` | `$a3` | `0x04,0x08,0x0C,0x10,0x14,0x18,0x1C,0x20,0x24`, then word0 at `0x00` | `sw ...,0($t6)` |
| `8003E26C` `TNG4` | `$a2` | `0x04,0x08,0x0C,0x10,0x14,0x18,0x1C,0x20,0x24,0x28,0x2C,0x30`, then word0 at `0x00` | `sw ...,0($t6)` |
| `8003E428` `TG4NL` | `$a3` | `0x04,0x08,0x0C,0x10,0x14,0x18,0x1C,0x20,0x24`, then word0 at `0x00` | `sw ...,0($t6)` |

Shared shape remains:

1. Load three vertices with `lwc2 $0..$5`.
2. Run `cop2 0x280030` (`RTPT`).
3. Read `cfc2 $31` and reject on negative `FLAG`.
4. Run fourth-vertex path with `cop2 0x180001` (`RTPS`).
5. Read `cfc2 $31` and reject on negative `FLAG`.
6. Run `cop2 0x168002E`, read `$7`, compute OT bucket, link packet word0 and
   OT slot, then return the advanced packet cursor.

## Runtime Boundary

Closed by this window:

- `800428B0` dispatch/cursor-advance graph is explicit enough to avoid treating
  `PsxBuildGsTmdFastPacket428B0(...)` as a free-form C++ packet layout.
- Runtime `PsxGpuPacketCommand428B0` now carries `primitiveCode`,
  `primitiveFlag`, `handlerTableSlot8001C1E8`,
  `handlerTableEntryAddress8001C1E8`, and
  `primitiveCursorAdvanceHalfwords800428B0` from the IDA dispatch facts.
- Four-point handler packet write offsets and OT-link shape are visible for the
  eight already-exported `GsTMDfast*` functions.
- Static BSS table bytes are confirmed not to be authority for initialized
  table values.

Still not closed:

- `sxyWordsKnown`
- `flagAfterRtptKnown`
- `flagAfterRtpsKnown`
- `flagOrKnown`
- `ir0Known`
- `returnValueKnown`
- exact screen-division / saturation behavior
- the two `0x20/0x30 + flag&4` special handler paths

Runtime review boundary:

- `BuildPsxTmdModelView428B0(...)`, `BuildPrimitiveGroups8004274C(...)`,
  `PsxProjectVertexRtpt428B0(...)`, `PsxNclipMac0FromSxy428B0(...)`, and
  `PsxAvszOtZ428B0(...)` are still adapters/replicas, not PSX-authoritative
  GTE truth.
- `DrawStage1GpuPacketCommand428B0(...)` remains the Win renderer adapter.
- `PsxCall8001B084_428B0(...)` has a misleading bool return if future code
  starts consuming it; current callers ignore that return.

Next code-safe cut: do not change renderer behavior. Either export/translate
the two special `flag&4` paths, or replace the C++ GTE/OTZ replicas with a
real facts source before setting any GTE/register known fields.
