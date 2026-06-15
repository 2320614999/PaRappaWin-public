# IDA export: word_800916E0 current SCUS xref matrix (2026-05-14)

Generated: 2026-05-14T12:03:06
IDA input: `SCUS_941.83`
Target: `0x800916E0` `word_800916E0`

## Key addresses
- `0x800916E0` `word_800916E0` func=`` bounds=`<no function>`
- `0x8001E750` `sub_8001E750` func=`sub_8001E750` bounds=`8001E750-8001EA00`
- `0x8001EF40` `sub_8001EF40` func=`sub_8001EF40` bounds=`8001EF40-8001F230`
- `0x80015660` `sub_80015660` func=`sub_80015660` bounds=`80015660-80015700`
- `0x80015408` `sub_80015408` func=`sub_80015408` bounds=`80015408-8001545C`
- `0x801C81EC` `sub_801C81EC` func=`sub_801C81EC` bounds=`801C81EC-801C858C`

## Direct xrefs to word_800916E0
| from | access | xref type | function | disasm |
|---|---|---|---|---|
| `0x80015424` | write | dr_W | `sub_80015408` `80015408-8001545C` | `sh      $v0, word_800916E0` |
| `0x80015684` | write | dr_W | `sub_80015660` `80015660-80015700` | `sh      $v0, word_800916E0` |
| `0x8001E8CC` | read | dr_R | `sub_8001E750` `8001E750-8001EA00` | `lh      $a0, word_800916E0 # jumptable 8001E7D8 case 10` |
| `0x8001EBCC` | read | dr_R | `sub_8001EA74` `8001EA74-8001EBF4` | `lh      $a0, word_800916E0` |

## Literal/name scan hits
| ea | function | disasm |
|---|---|---|
| `0x80015424` | `sub_80015408` `80015408-8001545C` | `sh      $v0, word_800916E0` |
| `0x80015684` | `sub_80015660` `80015660-80015700` | `sh      $v0, word_800916E0` |
| `0x8001E8CC` | `sub_8001E750` `8001E750-8001EA00` | `lh      $a0, word_800916E0 # jumptable 8001E7D8 case 10` |
| `0x8001EBCC` | `sub_8001EA74` `8001EA74-8001EBF4` | `lh      $a0, word_800916E0` |

## Contexts
### `0x80015424` write in `sub_80015408`
```asm
   80015410: sw      $fp, 0x10+var_s0($sp)
   80015414: move    $fp, $sp
   80015418: sw      $a0, 0x10+arg_0($fp)
   8001541C: lhu     $v0, 0x10+arg_0($fp)
   80015420: nop
>> 80015424: sh      $v0, word_800916E0
   8001542C: sw      $zero, dword_8006ECD4
   80015434: li      $a0, sub_8001537C
   8001543C: jal     VSyncCallback
   80015440: nop
   80015444: move    $sp, $fp
```

### `0x80015684` write in `sub_80015660`
```asm
   80015670: sw      $a0, 0x10+arg_0($fp)
   80015674: sw      $a1, 0x10+arg_4($fp)
   80015678: sw      $a2, 0x10+arg_8($fp)
   8001567C: lhu     $v0, 0x10+arg_4($fp)
   80015680: nop
>> 80015684: sh      $v0, word_800916E0
   8001568C: lw      $a0, 0x10+arg_4($fp)
   80015690: jal     sub_80015408
   80015694: nop
   80015698: lw      $v0, 0x10+arg_0($fp)
   8001569C: nop
```

### `0x8001E8CC` read in `sub_8001E750`
```asm
   8001E8B8: li      $a0, 3
   8001E8BC: jal     sub_80020568
   8001E8C0: move    $a0, $s0
   8001E8C4: j       def_8001E7D8 # jumptable 8001E7D8 default case
   8001E8C8: nop
>> 8001E8CC: lh      $a0, word_800916E0 # jumptable 8001E7D8 case 10
   8001E8D4: jal     sub_8001EF40
   8001E8D8: move    $a1, $zero
   8001E8DC: j       def_8001E7D8 # jumptable 8001E7D8 default case
   8001E8E0: nop
   8001E8E4: lw      $a1, 0x368($gp) # jumptable 8001E7D8 case 3
```

### `0x8001EBCC` read in `sub_8001EA74`
```asm
   8001EBB8: sw      $v0, 0x318($gp)
   8001EBBC: j       loc_8001EBDC
   8001EBC0: nop
   8001EBC4: beqz    $s1, loc_8001EBDC # jumptable 8001EB00 default case
   8001EBC8: nop
>> 8001EBCC: lh      $a0, word_800916E0
   8001EBD4: jal     sub_8001EF40
   8001EBD8: move    $a1, $zero
   8001EBDC: lw      $ra, 0x10+var_s8($sp)
   8001EBE0: lw      $s1, 0x10+var_s4($sp)
   8001EBE4: lw      $s0, 0x10+var_s0($sp)
```

## Pseudocode for referenced functions
### `sub_80015408` `0x80015408`
```c
// write access to const memory has been detected, the output may be wrong!
int __fastcall sub_80015408(__int16 a1)
{
  word_800916E0 = a1;
  dword_8006ECD4 = 0;
  return VSyncCallback(sub_8001537C);
}

```

### `sub_80015660` `0x80015660`
```c
int __fastcall sub_80015660(int a1, int a2, int a3)
{
  word_800916E0 = a2;
  sub_80015408(a2);
  sub_8001AC18(&(&off_80054758)[91 * a1 + 12], a3);
  return sub_8001545C();
}

```

### `sub_8001E750` `0x8001E750`
```c
int __fastcall sub_8001E750(int a1, _DWORD *a2)
{
  int v3; // $v0
  int v5; // $a0
  int v6; // $gp
  int v7; // $gp
  int result; // $v0
  int v9; // $gp
  int v10; // $v1
  int v11; // $gp
  int v12; // $gp
  int v13; // $a0

  v3 = sub_8004019C();
  v5 = dword_8006ED50[v3];
  *(_DWORD *)(v6 + 872) = v3;
  sub_80040F90(v5);
  result = sub_80040CC8(0, 0, (int *)&unk_80087288 + 5 * *(_DWORD *)(v7 + 872));
  switch ( a1 )
  {
    case 2:
      sub_8001D74C(3, *(_DWORD *)(v9 + 872));
      return sub_80020568(a2);
    case 3:
      sub_8001D74C(3, *(_DWORD *)(v9 + 872));
      return sub_80021E60(a2);
    case 4:
      v10 = *(_DWORD *)(v9 + 908);
      if ( !v10 )
        return sub_800203D4(*a2);
      if ( v10 == 1 )
      {
        sub_8001B6C4(0, 0, 320, 240, 1074728719, 0, (char *)&unk_80087288 + 20 * *(_DWORD *)(v9 + 872));
        result = *(_DWORD *)(v11 + 908) + 1;
        *(_DWORD *)(v11 + 908) = result;
      }
      else
      {
        result = sub_8001B120(0);
        *(_DWORD *)(v12 + 908) = 0;
      }
      return result;
    case 5:
      sub_8001D74C(3, *(_DWORD *)(v9 + 872));
      return sub_80020BE4(a2);
    case 6:
      sub_8001D74C(3, *(_DWORD *)(v9 + 872));
      return sub_80021594(a2);
    case 7:
    case 8:
    case 9:
      sub_8001D74C(3, *(_DWORD *)(v9 + 872));
      return sub_80020F94(a1, a2);
    case 10:
      return sub_8001EF40(word_800916E0, 0);
    case 11:
      sub_8001D74C(3, *(_DWORD *)(v9 + 872));
      v13 = 4;
      goto LABEL_22;
    case 12:
      sub_8001D74C(3, *(_DWORD *)(v9 + 872));
      v13 = 1;
      goto LABEL_22;
    case 13:
      sub_8001D74C(3, *(_DWORD *)(v9 + 872));
      v13 = 2;
      goto LABEL_22;
    case 14:
      sub_8001D74C(3, *(_DWORD *)(v9 + 872));
      v13 = 3;
      goto LABEL_22;
    case 15:
      sub_8001D74C(3, *(_DWORD *)(v9 + 872));
      v13 = 5;
      goto LABEL_22;
    case 16:
      sub_8001D74C(3, *(_DWORD *)(v9 + 872));
      return sub_80023618(a2);
    case 17:
      sub_8001D74C(4, *(_DWORD *)(v9 + 872));
      return sub_80021910(a2);
    case 18:
      sub_8001D74C(3, *(_DWORD *)(v9 + 872));
      v13 = 6;
      goto LABEL_22;
    case 19:
      sub_8001D74C(3, *(_DWORD *)(v9 + 872));
      v13 = 7;
LABEL_22:
      result = sub_80022CBC(v13, a2);
      break;
    default:
      return result;
  }
  return result;
}

```

### `sub_8001EA74` `0x8001EA74`
```c
int __fastcall sub_8001EA74(int a1, int a2)
{
  int v3; // $v0
  int v5; // $a0
  int v6; // $gp
  int v7; // $gp
  int result; // $v0
  int v9; // $gp
  int v10; // $gp
  int v11; // $gp
  int v12; // $gp

  v3 = sub_8004019C();
  v5 = dword_8006ED50[v3];
  *(_DWORD *)(v6 + 872) = v3;
  *(_DWORD *)(v6 + 792) = 1;
  sub_80040F90(v5);
  result = sub_80040CC8(0, 0, (int *)&unk_80087288 + 5 * *(_DWORD *)(v7 + 872));
  switch ( a2 )
  {
    case 1:
      return sub_8001F524(0, 8);
    case 2:
      *(_DWORD *)(v9 + 792) = 0;
      sub_8001FCBC(8, 4);
      goto LABEL_6;
    case 3:
      sub_80022CBC(4, a1);
      sub_8001D74C(5, *(_DWORD *)(v10 + 872));
      sub_8001FC40(4, 8);
      goto LABEL_6;
    case 4:
      sub_80021E60(0);
      sub_8001D74C(5, *(_DWORD *)(v11 + 872));
      sub_8001FC40(4, 8);
LABEL_6:
      result = sub_8001FDC0(0);
      break;
    case 5:
      result = sub_80020308(word_800916DC);
      *(_DWORD *)(v12 + 792) = result;
      break;
    case 6:
      result = sub_80020248(word_800916DC);
      break;
    default:
      if ( a1 )
        result = sub_8001EF40(word_800916E0, 0);
      break;
  }
  return result;
}

```

