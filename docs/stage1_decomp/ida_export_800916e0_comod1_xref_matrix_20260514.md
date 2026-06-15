# IDA export: word_800916E0 COMOD1 overlay xref matrix (2026-05-14)

Generated: 2026-05-14T12:04:33
IDA input: `COMOD1.BIN`
Target: `0x800916E0` ``

## Key addresses
- `0x800916E0` `` func=`` bounds=`<no function>`
- `0x801C81EC` `PrScene1_RunMovie1_AndStage1` func=`PrScene1_RunMovie1_AndStage1` bounds=`801C81EC-801C858C`
- `0x801C838C` `` func=`PrScene1_RunMovie1_AndStage1` bounds=`801C81EC-801C858C`
- `0x801C8558` `loc_801C8558` func=`PrScene1_RunMovie1_AndStage1` bounds=`801C81EC-801C858C`
- `0x80026B94` `` func=`` bounds=`<no function>`
- `0x8001EF40` `` func=`` bounds=`<no function>`

## Direct xrefs to word_800916E0
| from | access | xref type | function | disasm |
|---|---|---|---|---|
| `0x801C838C` | write | dr_W | `PrScene1_RunMovie1_AndStage1` `801C81EC-801C858C` | `sh      $v1, 0x800916E0` |
| `0x801C8558` | write | dr_W | `PrScene1_RunMovie1_AndStage1` `801C81EC-801C858C` | `sh      $v0, 0x800916E0` |

## Literal/name scan hits
| ea | function | disasm |
|---|---|---|
| `0x801C838C` | `PrScene1_RunMovie1_AndStage1` `801C81EC-801C858C` | `sh      $v1, 0x800916E0` |
| `0x801C8558` | `PrScene1_RunMovie1_AndStage1` `801C81EC-801C858C` | `sh      $v0, 0x800916E0` |

## Contexts
### `0x801C838C` write in `PrScene1_RunMovie1_AndStage1`
```asm
   801C8368: li      $a2, 1
   801C836C: li      $a3, 2
   801C8370: jal     0x80020110
   801C8374: sh      $zero, 0($s2)
   801C8378: jal     0x8001EF14
   801C837C: nop
   801C8380: lw      $a0, dword_801D3040
   801C8388: li      $v1, 3
>> 801C838C: sh      $v1, 0x800916E0
   801C8394: sh      $a0, 0x800916DA
   801C839C: j       loc_801C8564
   801C83A0: li      $v0, 0xFFFFFFFF
   801C83A4: bne     $v0, $a0, loc_801C8518
   801C83A8: move    $a0, $s3
   801C83AC: li      $a1, 2
   801C83B0: li      $a2, 1
   801C83B4: jal     0x80020110
```

### `0x801C8558` write in `PrScene1_RunMovie1_AndStage1`
```asm
   801C8538: nop
   801C853C: move    $a0, $s3
   801C8540: li      $a2, 1
   801C8544: jal     0x80020110
   801C8548: li      $a3, 2
   801C854C: jal     0x8001EF14
   801C8550: nop
   801C8554: li      $v0, 2
>> 801C8558: sh      $v0, 0x800916E0
   801C8560: li      $v0, 0xFFFFFFFF
   801C8564: lw      $ra, 0x10+var_s18($sp)
   801C8568: lw      $s5, 0x10+var_s14($sp)
   801C856C: lw      $s4, 0x10+var_s10($sp)
   801C8570: lw      $s3, 0x10+var_sC($sp)
   801C8574: lw      $s2, 0x10+var_s8($sp)
   801C8578: lw      $s1, 0x10+var_s4($sp)
   801C857C: lw      $s0, 0x10+var_s0($sp)
```

## Pseudocode for referenced functions
### `PrScene1_RunMovie1_AndStage1` `0x801C81EC`
```c
int __fastcall PrScene1_RunMovie1_AndStage1(int a1)
{
  __int16 v2; // $v0
  int v3; // $v0
  int result; // $v0
  int v5; // $s5
  unsigned int v6; // $s2
  int v7; // $s4
  int v8; // $s0
  BOOL v9; // $v0
  bool v10; // dc

  PrStrPlayer_ConfigMovieViewport();
  MEMORY[0x80026FA4]();
  if ( (unsigned int)MEMORY[0x800916D0] - 1 < 2 )
  {
    while ( 1 )
    {
LABEL_5:
      MEMORY[0x8001A4D0](MEMORY[0x8006EDB8] + 156, 0);
      v3 = PrStageRunner_Run(MEMORY[0x8006EDB8] + 156, (int *)0x801C3640, a1);
      if ( MEMORY[0x800916D0] == 1 )
      {
        MEMORY[0x800916D0] = 0;
        MEMORY[0x800916DA] = dword_801D3040;
        MEMORY[0x80020110](-2145634752, 2, 1, 2);
        MEMORY[0x8001EF14]();
        return 0;
      }
      if ( MEMORY[0x800916D0] == 2 )
      {
        MEMORY[0x80015744](-2146881776);
        MEMORY[0x800916D0] = 0;
        MEMORY[0x80020110](-2145634752, 2, 1, 2);
        MEMORY[0x8001EF14]();
        MEMORY[0x800916E0] = 3;
        MEMORY[0x800916DA] = dword_801D3040;
        return -1;
      }
      if ( v3 == 1 )
        break;
      if ( MEMORY[0x80026B94](4, 0) == 2 )
      {
        MEMORY[0x80020110](-2145634752, 2, 1, 2);
        MEMORY[0x8001EF14]();
        v2 = 2;
        goto LABEL_26;
      }
      MEMORY[0x8001EF14]();
    }
    MEMORY[0x80020110](-2145634752, 2, 1, 2);
    v5 = MEMORY[0x800166AC](a1);
    if ( MEMORY[0x801C368E] )
    {
      v6 = 2;
      v7 = 2;
      v8 = MEMORY[0x8006EDB8] + 252;
    }
    else
    {
      v6 = 1;
      v7 = 3;
      v8 = MEMORY[0x8006EDB8] + 204;
    }
    PrStrPlayer_Init(v8, 0);
    MEMORY[0x80026FA4]();
    MEMORY[0x800201AC](-2145634752, 6, 2, 1);
    PrStrPlayer_PlayAndWait(v8, -2145634752, v6);
    MEMORY[0x80026FA4]();
    MEMORY[0x800201AC](-2145634752, 5, 1, 2);
    MEMORY[0x8001EF14]();
    MEMORY[0x80026EF8](MEMORY[0x80094410]);
    MEMORY[0x80026ECC]();
    if ( !MEMORY[0x800916DA] )
    {
      MEMORY[0x8001635C](a1, v7, v5, MEMORY[0x80091816]);
      if ( a1 < 6 )
        MEMORY[0x8001628C](a1 + 1);
      if ( MEMORY[0x800916F0] != 1 )
      {
        MEMORY[0x80015590](a1);
        MEMORY[0x80019148](-2146881776);
      }
    }
    MEMORY[0x8001EF14]();
    if ( MEMORY[0x800916DA] == 1 )
      v9 = a1 < 3;
    else
      v9 = a1 < 6;
    v10 = !v9;
    result = 0;
    if ( !v10 )
      return a1 + 1;
  }
  else
  {
    MEMORY[0x800201AC](-2145634752, 6, 2, 1);
    PrStrPlayer_Init(MEMORY[0x8006EDB8] + 108, 0);
    if ( PrStrPlayer_PlayAndWait(MEMORY[0x8006EDB8] + 108, -2145634752, 0) != 1 )
    {
      MEMORY[0x80026FA4]();
      MEMORY[0x800201AC](-2145634752, 5, 1, 2);
      goto LABEL_5;
    }
    MEMORY[0x80026FA4]();
    MEMORY[0x800201AC](-2145634752, 5, 1, 2);
    MEMORY[0x8001EF14]();
    v2 = 3;
LABEL_26:
    MEMORY[0x800916E0] = v2;
    return -1;
  }
  return result;
}

```

