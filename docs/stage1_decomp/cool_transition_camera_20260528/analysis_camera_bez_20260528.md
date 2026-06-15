# Stage1 COOL transition camera BEZ audit 2026-05-28

## Scope

User-visible remaining issue: after GOOD->COOL succeeds, the transition into
free rap has a camera animation mismatch. Gameplay/free-rap logic is otherwise
mostly aligned.

Rules for this pass:

- Use IDA/pseudo-C/disasm authority.
- Stay in the direct-port Stage1 camera/`801CBFDC` path.
- Do not patch renderer/UI/shell, role rows, audio, rail, or harness code.

## New evidence

`docs/stage1_decomp/cool_transition_camera_20260528/ida_export_camera_bez_20260528.md`
exports SCUS `800127F0`, `800128DC`, and `80012960`.

Relevant `80012960` facts:

- `800127F0(a0, a1)` writes camera BEZ count to `$gp+0`, current segment to
  `$gp+4 = 0`, and period to `$gp+8 = a1`.
- `80012960` uses `$a0` as the tick input for `tick % period`.
- `80012960` uses `$gp+4` as the active BEZ segment index and only advances it
  on phase 0.
- `80012960` writes `80095C60..80095C74`, then calls `80040FA0`.

COMOD1 callsite disassembly for `801CBFDC` clarifies the Hex-Rays order around
the camera tick:

```asm
801CC5FC: lw       $a0, 0x104($s3)
801CC600: lui      $at, 0x801d
801CC604: sw       $zero, 0x302c($at)
801CC608: jal      0x800127f0
801CC60C: ori      $a1, $zero, 0xb
801CC610: lui      $v0, 0x801d
801CC614: lw       $v0, 0x3034($v0)
801CC618: nop
801CC61C: beqz     $v0, 0x801cc648
801CC620: addiu    $a0, $sp, 0x20
801CC624: lui      $a0, 0x801d
801CC628: lw       $a0, 0x302c($a0)
801CC62C: nop
801CC630: addiu    $v0, $a0, 1
801CC634: lui      $at, 0x801d
801CC638: sw       $v0, 0x302c($at)
801CC63C: jal      0x80012960
801CC640: nop
```

Therefore `801CBFDC` passes the pre-increment `dword_801D302C` value in `$a0`
to `80012960`, then stores the incremented value for the next frame.

## Win mismatch

`ApplyPrStageRunnerRenderGlobalsBlockG_801CBFDC` used:

```cpp
const int32_t tick =
    logicFrame ? ++g.dword_801D302C : g.dword_801D302C;
PsxCall80012960_801CBFDC(runtimeState, tick, renderSubFrame8);
```

This advances the Win camera one tick ahead of the PSX callsite. The first frame
after `800127F0` should call `80012960(0)`, while Win called `80012960(1)`.

The render-only path also used the already-advanced `dword_801D302C` as the
presentation base and sampled `tick + renderSubFrame8 / 255.0f`. There is no
fractional camera tick in the PSX `801CBFDC -> 80012960` callsite.

External Pro review of the same evidence also pointed out that `80012960` is a
stateful camera work update. It recomputes mode/velocity only when
`tick % period == 0`; Win's absolute `SampleAtTick()` recomputed from the full
tick on every call.

## Patch

The direct-port path now:

- Calls `PsxCall80012960_801CBFDC` with the pre-increment tick on logic frames.
- Increments `dword_801D302C` after capturing that tick, matching the COMOD1
  callsite.
- Uses stateful `CameraBezPlayer::Tick()` on logic frames instead of absolute
  `SampleAtTick()`.
- Reuses the last authoritative camera on render-only frames instead of
  performing non-PSX fractional interpolation.

No BEZ curve sampler, actor row, renderer, UI overlay, audio, or free-rap logic
was changed.
