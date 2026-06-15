# Scene Submit frame 2212 upstream overlay callchain

Scope: Stage1 Scene Submit / TOD `80028054 coordNode+0x44`. This file records the narrow upstream overlay chain that leads into the frame `2212` `801CBD60/801CBDA8` submit pair. It is a live-GDB gate map, not `coordNode+0x44` backing authority.

Inputs:

- Recorder context: `docs/stage1_decomp/scene_submit_frame2212_full_recorder_context_20260515.md`
- Raw context artifact: `artifacts/coord44_backing_recorder/coord44_frame2212_full_context_20260515.json`
- Overlay runner decompile: `tools/auto_port/output/stage1_overlay_closure_20260508_codex/decompile_801c81ec.txt`
- Overlay run-loop export: `docs/stage1_decomp/fail_prompt_ev4_20260510/ida_export_comod1_801C7A60_801C81EC_p0_20260510.txt`
- Init wrapper decompile: `docs/stage1_decomp/801C79E4_sub_801C79E4.c`
- Raw overlay: `E:\game\PSgame\parappa the rapper\S1\COMOD1.BIN`, loaded at `0x801C3870`

## Dynamic Chain

Recorder frame `2212` shows:

| seq | callsite | target | sp | ra | static role |
| ---: | --- | --- | --- | --- | --- |
| `1893` | `801C82F0` | `801C7A60` | `801FFF88` | `801C82F8` | `PrScene1_RunMovie1_AndStage1 -> PrStageRunner_Run` |
| `1897` | `801C7A8C` | `801C79E4` | `801FFF50` | `801C7A94` | `PrStageRunner_Run -> PrStageRunner_Init` |
| `1903` | `801C7A00` | `80026FA4` | `801FFF30` | `801C7A08` | `PrStageRunner_Init` reset/pre-init call |
| `2087` | `801C7A08` | `801CB6BC` | `801FFF30` | `801C7A10` | `PrStageRunner_Init -> PrStageRunner_InitUI` |
| `18333` | `801CBD60` | `8001B000` | `801FFEF8` | `801CBD68` | first init TOD submit |
| `18442` | `801CBDA8` | `8001B000` | `801FFEF8` | `801CBDB0` | second init TOD submit |

The event sequence is not globally monotonic across all recorder event streams, so the table must be read by call nesting/order in `call_trace`, not by sorting every subsystem by `event_seq`.

## Static Match

### `801C81EC PrScene1_RunMovie1_AndStage1`

Existing decompile has the run-loop call:

```c
MEMORY[0x8001A4D0](MEMORY[0x8006EDB8] + 156, 0);
v3 = PrStageRunner_Run(MEMORY[0x8006EDB8] + 156, (int *)0x801C3640, a1);
```

Raw disassembly around the recorder hit:

```asm
801C82CC: lui      $a0, 0x8007
801C82D0: lw       $a0, -0x1248($a0)   ; MEMORY[0x8006EDB8]
801C82D4: move     $a1, $zero
801C82D8: jal      0x8001a4d0
801C82DC: addiu    $a0, $a0, 0x9c
801C82E0: move     $a1, $s3            ; 0x801C3640
801C82E4: lui      $a0, 0x8007
801C82E8: lw       $a0, -0x1248($a0)   ; MEMORY[0x8006EDB8]
801C82EC: move     $a2, $s1            ; stage / scene arg
801C82F0: jal      0x801c7a60
801C82F4: addiu    $a0, $a0, 0x9c      ; MEMORY[0x8006EDB8] + 156
```

So the recorder call `801C82F0 -> 801C7A60` is the stage runner entry with:

- `a0 = MEMORY[0x8006EDB8] + 156`
- `a1 = 0x801C3640`
- `a2 = s1` from `801C81EC` input

### `801C7A60 PrStageRunner_Run`

Existing export names `801C7A60..801C81EC` as `PrStageRunner_Run` and shows:

```c
PrStageRunner_Init(a1, (int)a2);
...
PrStageRunner_RenderFrame(a2, 7);
...
```

Raw disassembly entry:

```asm
801C7A60: addiu    $sp, $sp, -0x38
801C7A68: move     $s5, $a0
801C7A70: move     $s1, $a1
801C7A78: move     $s6, $a2
801C7A8C: jal      0x801c79e4
801C7A90: sw       $s0, 0x18($sp)
```

So the recorder call `801C7A8C -> 801C79E4` is the run-loop prologue calling the init wrapper before the frame loop.

### `801C79E4 PrStageRunner_Init`

Existing decompile:

```c
int __fastcall sub_801C79E4(int a1, int a2)
{
  dword_801D303C = 0;
  MEMORY[0x80026FA4]();
  PrStageRunner_InitUI();
  MEMORY[0x8001A478](*(__int16 *)(a1 + 6));
  MEMORY[0x8001A654](*(__int16 *)(a1 + 4));
  MEMORY[0x80014344]();
  MEMORY[0x80024E98]();
  MEMORY[0x80024FC0](a2);
  return PrStageRunner_ResetState();
}
```

Raw disassembly:

```asm
801C79F8: lui      $at, 0x801d
801C79FC: sw       $zero, 0x303c($at)
801C7A00: jal      0x80026fa4
801C7A04: move     $s1, $a1
801C7A08: jal      0x801cb6bc
801C7A0C: nop
801C7A10: lh       $a0, 6($s0)
801C7A14: jal      0x8001a478
...
```

The recorder return `80026FBC -> 801C7A08` followed by `801C7A08 -> 801CB6BC` matches the static order exactly.

### `801CB6BC PrStageRunner_InitUI`

The downstream callsite map remains:

| callsite | call | meaning |
| --- | --- | --- |
| `801CBD44` | `8001AFD8(MEMORY[80091888], &801D301C, &801DB2D4)` | seed first cursor/count |
| `801CBD60` | `8001B000(0, &801DB2D4, &801D301C, &801D98F0)` | first init TOD submit |
| `801CBD8C` | `8001AFD8(MEMORY[800918A0], &801D3024, &801DB338)` | seed second cursor/count |
| `801CBDA8` | `8001B000(0, &801DB338, &801D3024, &801DB2D8)` | second init TOD submit |

## Live-GDB Gate

The current minimum live-GDB gate remains:

```text
break exec 0x801CBD60
break exec 0x801CBDA8
continue
on hit:
  break exec 0x800280B4
  continue
  capture $pc,$ra,$sp,$a0,$a1,$a2,$v0,$s2,$s3,$s5,$s6
  capture mem32[$a1], mem32[$a1+4], mem32[$v0], mem32[$v0+0x44]
```

Optional diagnostic gate:

```text
break exec 0x801C7A08
```

Use `801C7A08` only when confirming that the same live run entered `PrStageRunner_InitUI`. It is not the narrowest authority for `coordNode+0x44`.

## Boundary Conclusion

- This closes the overlay upstream call chain for frame `2212` and ties the recorder events to static COMOD1 pseudocode/raw disassembly.
- The chain explains when the `801CBD60/801CBDA8` submit pair is reached, but it does not provide the `800280B4` instruction-time COORD pointer.
- Do not derive `coord44BackingSlotKnown80028054` from `801C81EC`, `801C7A60`, `801C79E4`, runner state, cursor/count writeback, desc header stores, Win renderer, TMD state, candidate slots, or frame-boundary registers.
- The remaining authority gap is still only: live `$v0` at `800280B4` and `mem32[$v0+0x44]`.
