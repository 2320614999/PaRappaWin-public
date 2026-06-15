# Scene Submit frame 2212 COMOD1 callsites

Scope: Stage1 Scene Submit / TOD `8001B000 -> 80028504 -> 80028054`, only to narrow the live `800280B4` reproduction window. This is not `coordNode+0x44` backing authority.

Inputs:

- Recorder call-trace note: `docs/stage1_decomp/scene_submit_80028054_calltrace_hits_20260515.md`
- Current IDA UI export: `docs/stage1_decomp/ida_export_scene_submit_frame2212_callsite_801cbd60_801cbda8_20260515.txt`
- COMOD1 overlay pseudocode: `tools/auto_port/output/stage1_rail_overlay_dump_20260412_1545/decompile_801cb6bc.txt`
- Raw overlay: `E:\game\PSgame\parappa the rapper\S1\COMOD1.BIN`, loaded at `0x801C3870`

Current-IDB boundary:

- The current IDA UI session has no function bounds at `801CBD60` / `801CBDA8`; its export only sees fall-through xrefs around those addresses.
- Therefore the overlay ownership below comes from the existing COMOD1 overlay decompile plus raw `COMOD1.BIN` disassembly, not from current-IDB function naming.

Function ownership:

| range | name | source |
| --- | --- | --- |
| `801CB6BC..801CBFDC` | `PrStageRunner_InitUI` / `sub_801CB6BC` | COMOD1 overlay decompile and raw callsite window |
| `801CBFDC..801CC9E8` | `PrStageRunner_RenderFrame` | auto-port near-function map |
| `801CB190..801CB67C` | `PrStageRunner_RenderUI` | auto-port near-function map |

`801CBD60` and `801CBDA8` are both inside `PrStageRunner_InitUI`, not the per-frame render function.

Callsite parameters:

| callsite | pseudocode line | `8001B000` args | meaning |
| --- | ---: | --- | --- |
| `801CBD60` | `160` | `a0=0`, `a1=&unk_801DB2D4`, `a2=&unk_801D301C`, `a3=&unk_801D98F0` | first init TOD cursor/count pair; desc / COORD target is `801D98F0` |
| `801CBDA8` | `163` | `a0=0`, `a1=&unk_801DB338`, `a2=&unk_801D3024`, `a3=&unk_801DB2D8` | second init TOD cursor/count pair; desc / COORD target is `801DB2D8` |

Relevant raw disassembly:

```asm
801CBD1C: lui      $s1, 0x801d
801CBD20: addiu    $s1, $s1, 0x301c
801CBD24: move     $a1, $s1
801CBD28: lui      $s0, 0x801e
801CBD2C: addiu    $s0, $s0, -0x4d2c
801CBD30: move     $a2, $s0
801CBD34: lui      $a0, 0x8009
801CBD38: lw       $a0, 0x1888($a0)
801CBD3C: lui      $at, 0x801d
801CBD40: sw       $zero, 0x3020($at)
801CBD44: jal      0x8001afd8
801CBD48: move     $s2, $zero
801CBD4C: move     $a1, $s0
801CBD50: lui      $a0, 0x801d
801CBD54: lw       $a0, 0x3020($a0)
801CBD58: lui      $a3, 0x801e
801CBD5C: addiu    $a3, $a3, -0x6710
801CBD60: jal      0x8001b000
801CBD64: move     $a2, $s1

801CBD68: lui      $s1, 0x801d
801CBD6C: addiu    $s1, $s1, 0x3024
801CBD70: move     $a1, $s1
801CBD74: lui      $s0, 0x801e
801CBD78: addiu    $s0, $s0, -0x4cc8
801CBD7C: lui      $a0, 0x8009
801CBD80: lw       $a0, 0x18a0($a0)
801CBD84: lui      $at, 0x801d
801CBD88: sw       $zero, 0x3028($at)
801CBD8C: jal      0x8001afd8
801CBD90: move     $a2, $s0
801CBD94: move     $a1, $s0
801CBD98: lui      $a0, 0x801d
801CBD9C: lw       $a0, 0x3028($a0)
801CBDA0: lui      $a3, 0x801e
801CBDA4: addiu    $a3, $a3, -0x4d28
801CBDA8: jal      0x8001b000
801CBDAC: move     $a2, $s1
```

Downstream semantics already exported from SCUS:

```c
int sub_8001B000(int a1, int a2, int *a3, int a4)
{
  int v6 = *a3;
  if (!*a3 || *(int *)a2 <= 0)
    return -1;
  int v8 = sub_80028504(a1, v6, a4, 0);
  *a3 = v8;
  if (v6 != v8)
    --*(_DWORD *)a2;
  return *(_DWORD *)a2;
}
```

Frame `2212` recorder split:

| chain | event seq | call chain | count |
| --- | ---: | --- | ---: |
| first init TOD submit | `18333..18420` | `801CBD60 -> 8001B000 -> 80028504 -> 80028054` | 9 `80028054` calls |
| second init TOD submit | `18442..18520` | `801CBDA8 -> 8001B000 -> 80028504 -> 80028054` | 9 `80028054` calls |

Both outer calls have `sp=0x801FFEF8`, so the two submit chains are sibling calls in `PrStageRunner_InitUI`, not nested submit calls.

Live-GDB implication:

- Breakpoints can be narrowed to `801CBD60` and `801CBDA8` first, then enable/continue to `800280B4`.
- At `800280B4`, `800280AC` has already loaded the COORD node from `desc+4`; the live authority remains `mem32[$v0+0x44]`.
- Because `800280B4` is the load itself, `$s0` before stepping is stale and must not be treated as the loaded backing value.
- Capture minimum: `$pc,$ra,$sp,$a0,$a1,$a2,$v0,$s2,$s3,$s5,$s6`, `mem32[$a1]`, `mem32[$a1+4]`, `mem32[$v0]`, and `mem32[$v0+0x44]`.

Boundary conclusion:

- This closes the frame `2212` callsite ownership and argument table for the two observed `8001B000` chains.
- It still does not authorize `coord44BackingSlotKnown80028054`, because neither current recorder call-trace nor the current-IDB export contains instruction-time `$v0` or `mem32[$v0+0x44]`.
- Next valid product remains a live `0x800280B4` register/memory hit, now preferably gated by `801CBD60` / `801CBDA8` first.
