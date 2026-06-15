# Event-frame `8003EE84` box-fill order gap - 2026-05-14

## Scope

Narrow evidence window for whether event-frame `8003EE84` box-fill packets can
join the renderer sortable queue with event-frame `8003FA20` fast-sprite
packets.

## IDA evidence

### `8001E750`

Hex-Rays current session:

```c
case 4:
  v10 = *(_DWORD *)(v9 + 908);
  if ( !v10 )
    return sub_800203D4(*a2);
  if ( v10 == 1 )
  {
    sub_8001B6C4(0, 0, 320, 240, 1074728719, 0,
                 (char *)&unk_80087288 + 20 * *(_DWORD *)(v9 + 872));
    result = *(_DWORD *)(v11 + 908) + 1;
    *(_DWORD *)(v11 + 908) = result;
  }
  else
  {
    result = sub_8001B120(0);
    *(_DWORD *)(v12 + 908) = 0;
  }
  return result;
```

Interpretation: event4 backdrop `8001B6C4` is one branch of `8001E750`, while
prompt fast sprites come from `800203D4` in a different branch. This does not
by itself prove a shared cross-type packet order.

### `8001B6C4`

Hex-Rays current session:

```c
v9 = a1 - 160;
v10 = a2 - 120;
v11 = a3;
v12 = a4;
v8 = a5 & 0xFF000000;
v13 = BYTE2(a5);
v14 = BYTE1(a5);
v15 = a5;
return GsSortBoxFill(&v8, a7, a6);
```

Interpretation: `a6` is the PSX priority and `a7` is the work-list pointer.
The function does not provide a per-call sequence number.

### `8003EE84`

Hex-Rays current session:

```c
if ( *(int *)a1 >= 0 )
{
  v5 = dword_800901C8;
  *(uint32_t *)(dword_800901C8 + 4) =
      (v4 >> 17) & 0x180 | (v4 >> 23) & 0x60 | 0xE1000200;
  *(uint8_t *)(v5 + 8) = *(uint8_t *)(a1 + 12);
  *(uint8_t *)(v5 + 9) = *(uint8_t *)(a1 + 13);
  *(uint8_t *)(v5 + 11) = (v4 >> 29) & 2 | 0x60;
  *(uint8_t *)(v5 + 10) = *(uint8_t *)(a1 + 14);
  *(uint16_t *)(v5 + 12) = *(uint16_t *)(a1 + 4) + word_800917AA;
  *(uint16_t *)(v5 + 14) = *(uint16_t *)(a1 + 6) + word_800917AC;
  *(uint16_t *)(v5 + 16) = *(uint16_t *)(a1 + 8);
  *(uint16_t *)(v5 + 18) = *(uint16_t *)(a1 + 10);
  dword_800901C8 = sub_8003EF5C(v5, a2, a3, 4u);
}
```

Interpretation: the PSX function writes a 5-word packet and links it with
`sub_8003EF5C(v5, work, priority, 4)`. It proves priority/OT insertion, not a
Win-side sortable queue order shared with fast sprites.

### `8003EF5C`

Hex-Rays current session:

```c
v6 = a3 - *(uint32_t *)(a2 + 8);
if ( v6 < 0 )
  printf("ps_sort_sprite,bg: z resolution overflow\n");
v8 = 4 * v6 + *(uint32_t *)(a2 + 4);
result = a1 + 4 * a4 + 4;
*(uint32_t *)a1 = *(uint32_t *)v8;
*(uint8_t *)(a1 + 3) = a4;
*(uint32_t *)v8 = a1;
*(uint8_t *)(v8 + 3) = 0;
return result;
```

Interpretation: `8003EF5C` links the packet into the OT slot and returns the
next allocator address. It does not expose a linear draw order suitable for the
renderer queue. Cross-type order must come from an OT traversal consumer or a
same-domain runtime packet-write order fact.

## Current Win-source facts

- `AppendBoxFillPacketWrite8003EE84(...)` writes
  `write.provenance.priority = priority`.
- It does not write `write.provenance.psxCallOrder`.
- Before the 2026-05-14 follow-up fix,
  `BuildEventFrameBoxFillPacketCommands8003EE84(...)` filtered only box-fill
  writes, then assigned `packetMirrorOrder` over that filtered subset.
- Event-frame fast-sprite writes go through
  `ApplyRuntimeUpdate8003FA20(...) -> AppendRuntimePacketWrite8003FA20(...)`,
  storing `GsSortFastSpriteSubmitProvenance8003FA20`.
- `TagEventFrameFastSpriteInput8003FA20(...)` fills source kind / function /
  callsite / helper, but does not itself assign a global same-domain
  `psxCallOrder`.
- `AppendRuntimePacketWrite8003FA20(...)` preserves first-free append order in
  `runtime.packetWrites`, but that order is currently only implicit in the
  array position and is not exposed as a provenance field for both fast-sprite
  and box-fill writes.

Therefore the old `BoxFillPacketCommand8003EE84.psxCallOrder` was a
box-fill-subset order, not a same-domain global packet order across event-frame
box-fill and fast-sprite packets.

## Source-window table

| Path | Write storage | Priority fact | Order fact status |
| --- | --- | --- | --- |
| event-frame `8003FA20` fast sprite | `RuntimeState8003FA20.packetWrites` through `AppendRuntimePacketWrite8003FA20(...)` | `provenance.priority` | command builder uses whole-mirror reverse traversal order |
| event-frame `8003EE84` box fill | same `RuntimeState8003FA20.packetWrites` through `AppendBoxFillPacketWrite8003EE84(...)` | `provenance.priority` | command builder now uses whole-mirror reverse traversal order, not a filtered subset |
| PSX OT link `8003EF5C` | writes OT link slot | priority argument `a3` | no linear order; traversal required |

## 2026-05-14 follow-up resolution

The safe minimum runtime fix is not to synthesize a new order. Instead:

- keep using the existing `pageWork.work.packetWriteMirror` array as the
  same-domain packet mirror;
- traverse the whole mirror in reverse, matching the existing fast-sprite
  command builder's OT-head insertion order;
- increment `packetMirrorOrder` for every valid packet, not just for box-fill
  packets;
- only then filter to `8003EE84` when building the box-fill command.

`BuildStage1BoxFillPacketCommands8003EE84(...)` and
`BuildEventFrameBoxFillPacketCommands8003EE84(...)` now follow that rule. This
keeps `8003EE84` solid-rect queue order in the same order domain as
`8003FA20` fast sprites without using renderer order, visual results, or a
box-fill-only subset counter.
