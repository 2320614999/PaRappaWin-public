# IDA recheck: `80028054` coord44 control-flow window

Date: 2026-05-15

Scope: Stage1 Scene Submit / TOD `80028054` only.

Current IDA function:

- address: `0x80028054`
- name: `sub_80028054`
- size: `0x4B0`

Current Hex-Rays shape:

```c
v3 = *a1;
v4 = (__int16 *)(a1 + 1);
v5 = HIWORD(*a1) & 0xF;
v6 = (v3 >> 20) & 0xF;
v7 = HIBYTE(v3);
if (a2)
{
  v10 = (_DWORD *)a2[1];
  v9 = (char *)v10[17];
  v8 = (char *)(v10 + 1);
  *v10 = 0;
}
else
{
  a2 = (int *)&v21;
  v8 = &v22;
  v9 = &v23;
}
switch (v5)
```

The relevant raw disassembly window:

```asm
8002807C  lw      $v0, 0($s5)
80028080  addiu   $s2, $s5, 4
80028084  srl     $v1, $v0, 16
80028088  andi    $a0, $v1, 0xF
8002808C  srl     $v1, $v0, 20
80028090  andi    $s3, $v1, 0xF
80028094  bnez    $a1, loc_800280AC
80028098  srl     $s6, $v0, 24
8002809C  addiu   $a1, $sp, 0x68+var_58
800280A0  addiu   $s1, $sp, 0x68+var_48
800280A4  j       loc_800280C0
800280A8  addiu   $s0, $sp, 0x68+var_28
800280AC  lw      $v0, 4($a1)
800280B0  nop
800280B4  lw      $s0, 0x44($v0)
800280B8  addiu   $s1, $v0, 4
800280BC  sw      $zero, 0($v0)
800280C0  sltiu   $v0, $a0, 0xA
800280C4  beqz    $v0, def_800280E0
800280C8  sll     $v0, $a0, 2
800280CC  li      $at, jpt_800280E0
800280D4  addu    $at, $v0
800280D8  lw      $v0, 0($at)
800280DC  nop
800280E0  jr      $v0
800280E4  nop
```

## Control-flow facts

- `80028094` is the only branch that selects the desc/COORD-node path.
- If `$a1 == 0`, execution uses stack locals:
  - `$a1 = sp+0x10`
  - `$s1 = sp+0x20`
  - `$s0 = sp+0x40`
  - then jumps to `800280C0`.
- If `$a1 != 0`, execution goes to `800280AC`:
  - `800280AC` loads the COORD node pointer from `desc+4` into `$v0`.
  - `800280B4` loads the type1 TRS backing pointer/value from `coordNode+0x44`
    into `$s0`.
  - `800280B8` sets `$s1 = coordNode+4`, the COORD matrix destination.
  - `800280BC` clears `coordNode+0`, not `coordNode+0x44`.
  - execution then reaches the switch dispatcher at `800280C0`.

## Recorder implication

Frame `2212` full-branch can prove `800280AC` was reached 18 times. That proves
the `$a1 != 0` desc/COORD-node path was taken.

It still cannot prove `800280B4`'s loaded value:

- `800280B4` is a sequential load between branch targets.
- `800280BC` is also sequential and frame `2212` has no write-log hit there.
- frame `2212` has `read_log=0`.
- `pc_trace` is branch-target oriented and has zero `800280B4` hits.

Therefore `800280AC` hit count is a path fact only. It is not a
`coordNode+0x44` value fact.

## Valid next stops

- `80028094`: optional path diagnostics only.
- `800280AC`: can confirm the desc path was reached, but still before the
  `+0x44` load.
- `800280B4`: required live authority stop for `$v0` and `mem32[$v0+0x44]`.
- `800280BC`: optional confirmation that the same `$v0` node header is cleared;
  not a substitute for `800280B4`.

Do not use `800280AC`, `800280BC`, switch case stores, matrix stores, candidate
absolute slots, frame-boundary registers, Win renderer/TMD, or visual sampling
to authorize `coord44BackingSlotKnown80028054`.
