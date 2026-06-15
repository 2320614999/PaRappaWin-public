# Scene Submit 800428B0 Dispatch Recheck - 2026-05-15

Authority:

- Current IDA database export:
  `ida_export_800428b0_dispatch_table_recheck_20260515.txt`
- Code-side tightening:
  `src/pr/pr_stage_scene_submit_direct.cpp`

This note does not authorize renderer-side behavior, visual sampling, or Win
TMD defaults as PSX packet authority.

## Current IDA Facts

`800428B0` has two callers in the current IDA session:

```text
8001B0E4 -> 800428B0  func=sub_8001B084
801C5EC8 -> 800428B0  func=sub_801C5E60
```

The `8001B084` caller loop remains:

```text
sub_80041A68(desc+4, stack)
sub_80040544(stack)
sub_800428B0(desc, otag, 14-depth, 0x1F800000)
```

The current main IDB does not contain a function at `801CAC34`; that address
still needs the overlay/current-COMOD IDB path for standalone disassembly/xref.

## 800428B0 Dispatch Shape

The re-export confirms the top-level descriptor decode writes these globals:

```text
800901CC = attr & 7
8008ECB4 = (attr >> 3) & 3
8008ECB8 = (attr >> 5) & 1
8008ECB0 = (attr >> 6) & 1
801C3638 = (attr >> 9) & 7
80095C48 = (attr >> 30) & 1
```

The primitive code dispatch is keyed by `(*((byte*)prim + 3) & 0xFD)` and
uses `jpt_80042A5C` at `0x80012224`.

First 16 table entries from the current IDA session:

```text
0x80012224 -> 0x80042A64
0x80012228 -> 0x80042DB8
0x8001222C -> 0x80043040
0x80012230 -> 0x80043040
0x80012234 -> 0x80042AEC
0x80012238 -> 0x80042E94
0x8001223C -> 0x80043040
0x80012240 -> 0x80043040
0x80012244 -> 0x80042C60
0x80012248 -> 0x80042DE4
0x8001224C -> 0x80043040
0x80012250 -> 0x80043040
0x80012254 -> 0x80042CC8
0x80012258 -> 0x80042F88
0x8001225C -> 0x80043040
0x80012260 -> 0x80043040
```

The export also confirms handler table globals used by those dispatch arms,
including:

```text
8008EDD8 / 8008EED8
8008EDF0 / 8008EE10 / 8008EE18 / 8008EE30 / 8008EE38
8008EE50 / 8008EE58 / 8008EE70 / 8008EE78 / 8008EE90
8008EE98 / 8008EEB0 / 8008EEB8 / 8008EED0 / 8008EEE4
```

## Code Boundary Tightening

`BuildPrimitiveGroups8004274C(...)` previously treated missing raw packet facts
as a valid continuation. The first pass tightened this to require:

```text
rawPacketKnown == true
rawPacketByteSize != 0
current.rawPrimitiveIndex == previous.rawPrimitiveIndex + 1
current.rawPacketOffset == previous.rawPacketOffset + previous.rawPacketByteSize
```

The group stride is now `0` when the first primitive lacks raw packet facts, so
`PsxCall800428B0_DrawDescEntry(...)` will skip that group instead of granting a
PSX primitive stream without source facts.

The second pass adds `TmdObject::rawPrimitivePackets` as a raw packet sidecar.
`TmdParser::Parse(...)` now records every raw primitive packet cursor, including
unsupported packets that are not converted into `TmdPrimitive`. Parsed packets
store their parsed primitive index so `8004274C/800428B0` grouping can walk the
raw packet stream instead of the flattened parsed vector.

`BuildPrimitiveGroups8004274C(...)` now forms groups only from parsed packets
whose raw packet index, raw byte offset, and parsed primitive index are
contiguous. Unsupported/unparsed raw packets terminate the current group and are
left as explicit gaps. This preserves raw TMD stream metadata as the authority
for group formation, does not add renderer logic, and does not infer packet
success from visible output.

## Remaining Gaps

- `801CAC34` standalone disassembly/xref must come from the overlay/COMOD IDB;
  the current main IDB reports no function at that address.
- `800428B0` packet side effects are still only partially translated; backend
  and renderer deletion is not authorized by this recheck.
