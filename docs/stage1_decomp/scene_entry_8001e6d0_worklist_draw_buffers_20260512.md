# 8001E6D0 work-list + draw-buffer globals export (2026-05-12)

Authority: IDA Hex-Rays pseudo C + disassembly from SCUS_941.83.

## Function 0x8001e6d0 sub_8001E6D0

### Pseudo C
```c
void sub_8001E6D0()
{
  int v0; // $a1
  char *v1; // $a0
  int v2; // $v1

  v0 = 0;
  v1 = (char *)&unk_800872B0;
  v2 = 0;
  do
  {
    *(_DWORD *)((char *)&unk_8008728C + v2) = v1;
    v1 += 64;
    *(_DWORD *)((char *)&unk_80087288 + v2) = 4;
    *(_DWORD *)((char *)&unk_80087290 + v2) = 0;
    ++v0;
    v2 += 20;
  }
  while ( v0 < 2 );
  sub_8001E33C((int)&unk_80080CF8, (int)&unk_80083FC0);
}

```

### Disassembly
```asm
8001E6D0: addiu   $sp, -0x18
8001E6D4: sw      $ra, 0x10+var_s0($sp)
8001E6D8: move    $a1, $zero
8001E6DC: li      $a2, 4
8001E6E0: li      $a0, unk_800872B0
8001E6E8: move    $v1, $zero
8001E6EC: li      $at, unk_8008728C
8001E6F4: addu    $at, $v1
8001E6F8: sw      $a0, 0($at)
8001E6FC: addiu   $a0, 0x40  # '@'
8001E700: li      $at, unk_80087288
8001E708: addu    $at, $v1
8001E70C: sw      $a2, 0($at)
8001E710: li      $at, unk_80087290
8001E718: addu    $at, $v1
8001E71C: sw      $zero, 0($at)
8001E720: addiu   $a1, 1
8001E724: slti    $v0, $a1, 2
8001E728: bnez    $v0, loc_8001E6EC
8001E72C: addiu   $v1, 0x14
8001E730: li      $a0, unk_80080CF8
8001E738: jal     sub_8001E33C
8001E73C: addiu   $a1, $a0, (unk_80083FC0 - 0x80080CF8)
8001E740: lw      $ra, 0x10+var_s0($sp)
8001E744: addiu   $sp, 0x18
8001E748: jr      $ra
8001E74C: nop
```

### Xrefs from this function
```json
[
  {
    "from": "0x8001e6d0",
    "to": "0x8001e6d4",
    "to_name": "",
    "type": 21,
    "line": "addiu   $sp, -0x18"
  },
  {
    "from": "0x8001e6d4",
    "to": "0x8001e6d8",
    "to_name": "",
    "type": 21,
    "line": "sw      $ra, 0x10+var_s0($sp)"
  },
  {
    "from": "0x8001e6d8",
    "to": "0x8001e6dc",
    "to_name": "",
    "type": 21,
    "line": "move    $a1, $zero"
  },
  {
    "from": "0x8001e6dc",
    "to": "0x8001e6e0",
    "to_name": "",
    "type": 21,
    "line": "li      $a2, 4"
  },
  {
    "from": "0x8001e6e0",
    "to": "0x8001e6e8",
    "to_name": "",
    "type": 21,
    "line": "li      $a0, unk_800872B0"
  },
  {
    "from": "0x8001e6e0",
    "to": "0x800872b0",
    "to_name": "unk_800872B0",
    "type": 1,
    "line": "li      $a0, unk_800872B0"
  },
  {
    "from": "0x8001e6e8",
    "to": "0x8001e6ec",
    "to_name": "loc_8001E6EC",
    "type": 21,
    "line": "move    $v1, $zero"
  },
  {
    "from": "0x8001e6ec",
    "to": "0x8001e6f4",
    "to_name": "",
    "type": 21,
    "line": "li      $at, unk_8008728C"
  },
  {
    "from": "0x8001e6ec",
    "to": "0x8008728c",
    "to_name": "unk_8008728C",
    "type": 1,
    "line": "li      $at, unk_8008728C"
  },
  {
    "from": "0x8001e6f4",
    "to": "0x8001e6f8",
    "to_name": "",
    "type": 21,
    "line": "addu    $at, $v1"
  },
  {
    "from": "0x8001e6f8",
    "to": "0x8001e6fc",
    "to_name": "",
    "type": 21,
    "line": "sw      $a0, 0($at)"
  },
  {
    "from": "0x8001e6fc",
    "to": "0x8001e700",
    "to_name": "",
    "type": 21,
    "line": "addiu   $a0, 0x40  # '@'"
  },
  {
    "from": "0x8001e700",
    "to": "0x8001e708",
    "to_name": "",
    "type": 21,
    "line": "li      $at, unk_80087288"
  },
  {
    "from": "0x8001e700",
    "to": "0x80087288",
    "to_name": "unk_80087288",
    "type": 1,
    "line": "li      $at, unk_80087288"
  },
  {
    "from": "0x8001e708",
    "to": "0x8001e70c",
    "to_name": "",
    "type": 21,
    "line": "addu    $at, $v1"
  },
  {
    "from": "0x8001e70c",
    "to": "0x8001e710",
    "to_name": "",
    "type": 21,
    "line": "sw      $a2, 0($at)"
  },
  {
    "from": "0x8001e710",
    "to": "0x8001e718",
    "to_name": "",
    "type": 21,
    "line": "li      $at, unk_80087290"
  },
  {
    "from": "0x8001e710",
    "to": "0x80087290",
    "to_name": "unk_80087290",
    "type": 1,
    "line": "li      $at, unk_80087290"
  },
  {
    "from": "0x8001e718",
    "to": "0x8001e71c",
    "to_name": "",
    "type": 21,
    "line": "addu    $at, $v1"
  },
  {
    "from": "0x8001e71c",
    "to": "0x8001e720",
    "to_name": "",
    "type": 21,
    "line": "sw      $zero, 0($at)"
  },
  {
    "from": "0x8001e720",
    "to": "0x8001e724",
    "to_name": "",
    "type": 21,
    "line": "addiu   $a1, 1"
  },
  {
    "from": "0x8001e724",
    "to": "0x8001e728",
    "to_name": "",
    "type": 21,
    "line": "slti    $v0, $a1, 2"
  },
  {
    "from": "0x8001e728",
    "to": "0x8001e72c",
    "to_name": "",
    "type": 21,
    "line": "bnez    $v0, loc_8001E6EC"
  },
  {
    "from": "0x8001e728",
    "to": "0x8001e6ec",
    "to_name": "loc_8001E6EC",
    "type": 19,
    "line": "bnez    $v0, loc_8001E6EC"
  },
  {
    "from": "0x8001e72c",
    "to": "0x8001e730",
    "to_name": "",
    "type": 21,
    "line": "addiu   $v1, 0x14"
  },
  {
    "from": "0x8001e730",
    "to": "0x8001e738",
    "to_name": "",
    "type": 21,
    "line": "li      $a0, unk_80080CF8"
  },
  {
    "from": "0x8001e730",
    "to": "0x80080cf8",
    "to_name": "unk_80080CF8",
    "type": 1,
    "line": "li      $a0, unk_80080CF8"
  },
  {
    "from": "0x8001e738",
    "to": "0x8001e73c",
    "to_name": "",
    "type": 21,
    "line": "jal     sub_8001E33C"
  },
  {
    "from": "0x8001e738",
    "to": "0x8001e33c",
    "to_name": "sub_8001E33C",
    "type": 17,
    "line": "jal     sub_8001E33C"
  },
  {
    "from": "0x8001e73c",
    "to": "0x8001e740",
    "to_name": "",
    "type": 21,
    "line": "addiu   $a1, $a0, (unk_80083FC0 - 0x80080CF8)"
  },
  {
    "from": "0x8001e73c",
    "to": "0x80083fc0",
    "to_name": "unk_80083FC0",
    "type": 1,
    "line": "addiu   $a1, $a0, (unk_80083FC0 - 0x80080CF8)"
  },
  {
    "from": "0x8001e740",
    "to": "0x8001e744",
    "to_name": "",
    "type": 21,
    "line": "lw      $ra, 0x10+var_s0($sp)"
  },
  {
    "from": "0x8001e744",
    "to": "0x8001e748",
    "to_name": "",
    "type": 21,
    "line": "addiu   $sp, 0x18"
  },
  {
    "from": "0x8001e748",
    "to": "0x8001e74c",
    "to_name": "",
    "type": 21,
    "line": "jr      $ra"
  }
]
```

## Data 0x80087288 unk_80087288

- bytes[0:32]: `10 43 00 10 00 40 10 00 00 00 00 1D 80 02 3C 7C C6 42 24 10 00 A2 AF 1D 80 02 3C 38 95 42 24 14`

```json
[
  {
    "from": "0x8001c528",
    "from_func": "0x8001c4ec",
    "from_func_name": "sub_8001C4EC",
    "line": "li      $v1, unk_80087288"
  },
  {
    "from": "0x8001c584",
    "from_func": "0x8001c550",
    "from_func_name": "sub_8001C550",
    "line": "li      $v1, unk_80087288"
  },
  {
    "from": "0x8001c5d8",
    "from_func": "0x8001c5a8",
    "from_func_name": "sub_8001C5A8",
    "line": "li      $v1, unk_80087288"
  },
  {
    "from": "0x8001c63c",
    "from_func": "0x8001c604",
    "from_func_name": "sub_8001C604",
    "line": "li      $v1, unk_80087288"
  },
  {
    "from": "0x8001c680",
    "from_func": "0x8001c668",
    "from_func_name": "sub_8001C668",
    "line": "li      $v0, unk_80087288"
  },
  {
    "from": "0x8001c6c0",
    "from_func": "0x8001c6a0",
    "from_func_name": "sub_8001C6A0",
    "line": "li      $v0, unk_80087288"
  },
  {
    "from": "0x8001c6fc",
    "from_func": "0x8001c6e0",
    "from_func_name": "sub_8001C6E0",
    "line": "li      $v0, unk_80087288"
  },
  {
    "from": "0x8001c734",
    "from_func": "0x8001c71c",
    "from_func_name": "sub_8001C71C",
    "line": "li      $v0, unk_80087288"
  },
  {
    "from": "0x8001c784",
    "from_func": "0x8001c754",
    "from_func_name": "sub_8001C754",
    "line": "li      $v1, unk_80087288"
  },
  {
    "from": "0x8001c7d8",
    "from_func": "0x8001c7a8",
    "from_func_name": "sub_8001C7A8",
    "line": "li      $v1, unk_80087288"
  },
  {
    "from": "0x8001c840",
    "from_func": "0x8001c804",
    "from_func_name": "sub_8001C804",
    "line": "li      $v0, unk_80087288"
  },
  {
    "from": "0x8001c890",
    "from_func": "0x8001c864",
    "from_func_name": "sub_8001C864",
    "line": "li      $v0, unk_80087288"
  },
  {
    "from": "0x8001ca94",
    "from_func": "0x8001c864",
    "from_func_name": "sub_8001C864",
    "line": "li      $s3, unk_80087288"
  },
  {
    "from": "0x8001ccbc",
    "from_func": "0x8001c864",
    "from_func_name": "sub_8001C864",
    "line": "li      $s0, unk_80087288"
  },
  {
    "from": "0x8001ce48",
    "from_func": "0x8001ce30",
    "from_func_name": "sub_8001CE30",
    "line": "li      $s3, unk_80087288"
  },
  {
    "from": "0x8001d054",
    "from_func": "0x8001ce30",
    "from_func_name": "sub_8001CE30",
    "line": "li      $s6, unk_80087288"
  },
  {
    "from": "0x8001d180",
    "from_func": "0x8001ce30",
    "from_func_name": "sub_8001CE30",
    "line": "li      $s0, unk_80087288"
  },
  {
    "from": "0x8001d788",
    "from_func": "0x8001d74c",
    "from_func_name": "sub_8001D74C",
    "line": "li      $s0, unk_80087288"
  },
  {
    "from": "0x8001d8e4",
    "from_func": "0x8001d74c",
    "from_func_name": "sub_8001D74C",
    "line": "li      $s6, unk_80087288"
  },
  {
    "from": "0x8001da04",
    "from_func": "0x8001d74c",
    "from_func_name": "sub_8001D74C",
    "line": "li      $v1, unk_80087288"
  },
  {
    "from": "0x8001dc00",
    "from_func": "0x8001db9c",
    "from_func_name": "sub_8001DB9C",
    "line": "li      $v0, unk_80087288"
  },
  {
    "from": "0x8001dc54",
    "from_func": "0x8001db9c",
    "from_func_name": "sub_8001DB9C",
    "line": "li      $v0, unk_80087288"
  },
  {
    "from": "0x8001dca8",
    "from_func": "0x8001db9c",
    "from_func_name": "sub_8001DB9C",
    "line": "li      $v0, unk_80087288"
  },
  {
    "from": "0x8001dcfc",
    "from_func": "0x8001db9c",
    "from_func_name": "sub_8001DB9C",
    "line": "li      $v0, unk_80087288"
  },
  {
    "from": "0x8001dd50",
    "from_func": "0x8001db9c",
    "from_func_name": "sub_8001DB9C",
    "line": "li      $v0, unk_80087288"
  },
  {
    "from": "0x8001dda4",
    "from_func": "0x8001db9c",
    "from_func_name": "sub_8001DB9C",
    "line": "li      $v0, unk_80087288"
  },
  {
    "from": "0x8001def0",
    "from_func": "0x8001de08",
    "from_func_name": "sub_8001DE08",
    "line": "li      $v1, unk_80087288"
  },
  {
    "from": "0x8001e018",
    "from_func": "0x8001df24",
    "from_func_name": "sub_8001DF24",
    "line": "li      $v1, unk_80087288"
  },
  {
    "from": "0x8001e098",
    "from_func": "0x8001df24",
    "from_func_name": "sub_8001DF24",
    "line": "li      $v1, unk_80087288"
  },
  {
    "from": "0x8001e104",
    "from_func": "0x8001df24",
    "from_func_name": "sub_8001DF24",
    "line": "li      $v1, unk_80087288"
  },
  {
    "from": "0x8001e170",
    "from_func": "0x8001df24",
    "from_func_name": "sub_8001DF24",
    "line": "li      $s0, unk_80087288"
  },
  {
    "from": "0x8001e388",
    "from_func": "0x8001e374",
    "from_func_name": "sub_8001E374",
    "line": "li      $a2, unk_80087288"
  },
  {
    "from": "0x8001e3c4",
    "from_func": "0x8001e3b0",
    "from_func_name": "sub_8001E3B0",
    "line": "li      $a0, unk_80087288"
  },
  {
    "from": "0x8001e4a0",
    "from_func": "0x8001e408",
    "from_func_name": "sub_8001E408",
    "line": "li      $s0, unk_80087288"
  },
  {
    "from": "0x8001e578",
    "from_func": "0x8001e54c",
    "from_func_name": "sub_8001E54C",
    "line": "li      $v0, unk_80087288"
  },
  {
    "from": "0x8001e63c",
    "from_func": "0x8001e5a4",
    "from_func_name": "sub_8001E5A4",
    "line": "li      $s0, unk_80087288"
  },
  {
    "from": "0x8001e700",
    "from_func": "0x8001e6d0",
    "from_func_name": "sub_8001E6D0",
    "line": "li      $at, unk_80087288"
  },
  {
    "from": "0x8001e7a4",
    "from_func": "0x8001e750",
    "from_func_name": "sub_8001E750",
    "line": "li      $v0, unk_80087288"
  },
  {
    "from": "0x8001e824",
    "from_func": "0x8001e750",
    "from_func_name": "sub_8001E750",
    "line": "li      $v1, unk_80087288"
  },
  {
    "from": "0x8001ea50",
    "from_func": "0x8001ea00",
    "from_func_name": "sub_8001EA00",
    "line": "li      $v0, unk_80087288"
  },
  {
    "from": "0x8001ead0",
    "from_func": "0x8001ea74",
    "from_func_name": "sub_8001EA74",
    "line": "li      $v0, unk_80087288"
  },
  {
    "from": "0x8001ec34",
    "from_func": "0x8001ebf4",
    "from_func_name": "sub_8001EBF4",
    "line": "li      $v0, unk_80087288"
  },
  {
    "from": "0x8001eca0",
    "from_func": "0x8001ec54",
    "from_func_name": "sub_8001EC54",
    "line": "li      $s1, unk_80087288"
  },
  {
    "from": "0x8001ed54",
    "from_func": "0x8001ed3c",
    "from_func_name": "sub_8001ED3C",
    "line": "li      $v0, unk_80087288"
  },
  {
    "from": "0x8001ee00",
    "from_func": "0x8001edbc",
    "from_func_name": "sub_8001EDBC",
    "line": "li      $s1, unk_80087288"
  },
  {
    "from": "0x8001ee8c",
    "from_func": "0x8001ee74",
    "from_func_name": "sub_8001EE74",
    "line": "li      $v0, unk_80087288"
  }
]
```

## Data 0x8008728c unk_8008728C

- bytes[0:32]: `00 40 10 00 00 00 00 1D 80 02 3C 7C C6 42 24 10 00 A2 AF 1D 80 02 3C 38 95 42 24 14 00 A2 AF E4`

```json
[
  {
    "from": "0x8001e6ec",
    "from_func": "0x8001e6d0",
    "from_func_name": "sub_8001E6D0",
    "line": "li      $at, unk_8008728C"
  }
]
```

## Data 0x80087290 unk_80087290

- bytes[0:32]: `00 00 00 1D 80 02 3C 7C C6 42 24 10 00 A2 AF 1D 80 02 3C 38 95 42 24 14 00 A2 AF E4 00 65 8E F0`

```json
[
  {
    "from": "0x8001e710",
    "from_func": "0x8001e6d0",
    "from_func_name": "sub_8001E6D0",
    "line": "li      $at, unk_80087290"
  }
]
```

## Data 0x800872b0 unk_800872B0

- bytes[0:32]: `00 66 8E 1D 80 07 3C 6C C6 E7 24 14 50 00 0C 01 00 04 34 01 00 02 34 1D 80 01 3C 00 B6 22 AC 00`

```json
[
  {
    "from": "0x8001e6e0",
    "from_func": "0x8001e6d0",
    "from_func_name": "sub_8001E6D0",
    "line": "li      $a0, unk_800872B0"
  }
]
```

## Data 0x80080cf8 unk_80080CF8

- bytes[0:32]: `FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF`

```json
[
  {
    "from": "0x8001e354",
    "from_func": "0x8001e34c",
    "from_func_name": "sub_8001E34C",
    "line": "li      $a0, unk_80080CF8"
  },
  {
    "from": "0x8001e730",
    "from_func": "0x8001e6d0",
    "from_func_name": "sub_8001E6D0",
    "line": "li      $a0, unk_80080CF8"
  }
]
```

## Data 0x80083fc0 unk_80083FC0

- bytes[0:32]: `FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF`

```json
[
  {
    "from": "0x8001e360",
    "from_func": "0x8001e34c",
    "from_func_name": "sub_8001E34C",
    "line": "addiu   $a1, $a0, (unk_80083FC0 - 0x80080CF8)"
  },
  {
    "from": "0x8001e73c",
    "from_func": "0x8001e6d0",
    "from_func_name": "sub_8001E6D0",
    "line": "addiu   $a1, $a0, (unk_80083FC0 - 0x80080CF8)"
  }
]
```

## Translation boundary

- `8001E6D0` initializes two 20-byte work-list descriptors at `80087288/8C/90` using storage starting at `800872B0`, stride 64.
- It then calls `8001E33C(&unk_80080CF8, &unk_80083FC0)`; this is the same draw-buffer global setter used by `8001E34C`.
- Do not treat this as renderer flush/submit; it only mutates PSX global pointer state plus the work-list seed state.
