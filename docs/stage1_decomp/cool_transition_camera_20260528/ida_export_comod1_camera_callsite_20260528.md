# COMOD1 camera callsite focused disasm 2026-05-28

Source: `E:\game\PSgame\parappa the rapper\S1\COMOD1.BIN`, mapped at
`0x801C3870` from the existing COMOD1 IDA segment notes. This file records the
focused callsite bytes around `801CBFDC -> 800127F0/80012960`; the broader
Hex-Rays pseudo-C for `801CBFDC` is already present in
`docs/stage1_decomp/tmp_20260521_camera_recheck/decompile_801cbfdc.txt`.

## Focused 801CBFDC camera block

```asm
801CC5E0: andi     $v1, $v1, 0x400
801CC5E4: lui      $at, 0x801e
801CC5E8: sw       $v0, -0x4cc4($at)
801CC5EC: lui      $at, 0x801d
801CC5F0: sw       $a0, 0x3014($at)
801CC5F4: beqz     $v1, 0x801cc610
801CC5F8: nop
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
801CC644: addiu    $a0, $sp, 0x20
801CC648: addiu    $a1, $sp, 0x30
```

Important ordering:

- `801CC628` loads the pre-increment `dword_801D302C` into `$a0`.
- `801CC630..801CC638` increments and stores `dword_801D302C`.
- `801CC63C` calls `80012960` with the old tick still in `$a0`.

This means the first camera tick after `800127F0` is `80012960(0)`, not
`80012960(1)`.
