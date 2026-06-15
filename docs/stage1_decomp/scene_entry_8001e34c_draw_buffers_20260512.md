# 8001E34C / 8001E33C scene loop draw-buffer globals export (2026-05-12)

## Function 0x8001e34c sub_8001E34C

### Pseudocode
```c
int sub_8001E34C()
{
  return sub_8001E33C(&unk_80080CF8, &unk_80083FC0);
}

```

### Disassembly
```asm
8001E34C: addiu   $sp, -0x18
8001E350: sw      $ra, 0x10+var_s0($sp)
8001E354: li      $a0, unk_80080CF8
8001E35C: jal     sub_8001E33C
8001E360: addiu   $a1, $a0, (unk_80083FC0 - 0x80080CF8)
8001E364: lw      $ra, 0x10+var_s0($sp)
8001E368: addiu   $sp, 0x18
8001E36C: jr      $ra
8001E370: nop
```

### XrefsTo
```json
[
  {
    "from": "0x80015fd4",
    "type": 17,
    "func": "0x80015d18",
    "func_name": "sub_80015D18",
    "line": "jal     sub_8001E34C"
  }
]
```

## Function 0x8001e33c sub_8001E33C

### Pseudocode
```c
void __fastcall sub_8001E33C(int a1, int a2)
{
  int v2; // $gp

  *(_DWORD *)(v2 + 784) = a1;
  *(_DWORD *)(v2 + 788) = a2;
}

```

### Disassembly
```asm
8001E33C: sw      $a0, 0x310($gp)
8001E340: sw      $a1, 0x314($gp)
8001E344: jr      $ra
8001E348: nop
```

### XrefsTo
```json
[
  {
    "from": "0x8001e35c",
    "type": 17,
    "func": "0x8001e34c",
    "func_name": "sub_8001E34C",
    "line": "jal     sub_8001E33C"
  },
  {
    "from": "0x8001e738",
    "type": 17,
    "func": "0x8001e6d0",
    "func_name": "sub_8001E6D0",
    "line": "jal     sub_8001E33C"
  },
  {
    "from": "0x801c60c8",
    "type": 17,
    "func": "0x801c609c",
    "func_name": "sub_801C609C",
    "line": "jal     sub_8001E33C"
  }
]
```

## Target globals

| addr | name | seg | bytes[0:32] | xrefs |
|---|---|---|---|---|
| 0x80080cf8 | unk_80080CF8 | 3 | FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF | 0x8001e354 sub_8001E34C; 0x8001e730 sub_8001E6D0 |
| 0x80083fc0 | unk_80083FC0 | 3 | FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF | 0x8001e360 sub_8001E34C; 0x8001e73c sub_8001E6D0 |

## Direct-port facts

- `8001E34C` is a wrapper for `8001E33C(&unk_80080CF8, &unk_80083FC0)`.
- `8001E33C(a1,a2)` writes `*(gp+784)=a1` and `*(gp+788)=a2`.
- `80015D18` calls `8001E34C` once at loop tail after scene result state updates.
- This export proves only draw-buffer global pointer assignment. It does not authorize renderer submit, OT flush, or scene-specific visual fixes.
