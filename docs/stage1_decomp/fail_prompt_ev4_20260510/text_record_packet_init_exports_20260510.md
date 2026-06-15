# TEXT record packet init exports: 80044238 / 800440D0 / 800441C0

Scope: static-only IDA evidence整理 for closing the text record
`+0x04..+0x07` gap and glyph packet init gap. No build, no test, no emulator,
no `src` edits.

Primary raw export written in this pass:

- `ida_export_text_record_packet_init_20260510.md`

Inputs read before export:

- `docs/项目规则.md`
- `text_vararg_record_gap_matrix_20260510.md`
- `record_producer_branch_20260510.md`
- Existing exports:
  - `ida_export_record_bank_init_800433xx_20260510.txt`
  - `ida_export_record_producer_80043A14_800436F0_20260510.txt`

IDA MCP status:

- `decompile_function(0x80044238)`, `decompile_function(0x800440D0)`, and
  `decompile_function(0x800441C0)` completed without timeout.
- An async IDAPython export also completed and wrote
  `ida_export_text_record_packet_init_20260510.md`.
- No timeout recovery was needed; no assumption is based on a missing export.

## Static conclusion

The focused exports close the narrow static ownership gap for text record
`+0x04..+0x07` and glyph packet init:

- `80044238(record)` initializes the record header/control bytes at
  `record+0x03 = 0x03` and `record+0x07 = 0x60`.
- `80043438` then explicitly clears `record+0x04`, `record+0x05`, and
  `record+0x06` when `mode != 0`.
- `800440D0(record, mode == 2)` only toggles bit `0x02` in `record+0x07`;
  it does not touch `record+0x04..+0x06`.
- `800441C0(packet)` initializes each glyph packet header/control bytes at
  `packet+0x03 = 0x03` and `packet+0x07 = 0x74`.
- `80043438` writes `packet+0x0E = word_8008EB54` immediately after each
  `800441C0(packet)` call.
- `800436F0` remains the glyph packet consumer: it writes per-glyph RGB/XY/UV,
  links nodes through `8004401C`, optionally links the record itself when
  `record+0x07 != 0`, calls `800450A0(record+0x10)`, then clears the text.

This is translatable as direct-port helper behavior. It is not evidence for
replacing the PSX text path with a Win text renderer.

## Function evidence

### `80044238(record)`

Pseudo C from IDA:

```c
int __fastcall sub_80044238(int a1)
{
  int result; // $v0

  *(_BYTE *)(a1 + 3) = 3;
  result = 96;
  *(_BYTE *)(a1 + 7) = 96;
  return result;
}
```

Disassembly:

```asm
80044238: li      $v0, 3
8004423C: sb      $v0, 3($a0)
80044240: li      $v0, 0x60
80044244: jr      $ra
80044248: sb      $v0, 7($a0)
```

Xref:

```text
80043530 -> 80044238 in sub_80043438
```

Field matrix:

| Field | Access | Value | Evidence | Translation status |
|---|---:|---:|---|---|
| `record+0x03` | write byte | `0x03` | `sb $v0, 3($a0)` after `li $v0, 3` | Directly translatable. |
| `record+0x04` | none | n/a | No access in this helper. | Owned by later `80043438` zero write when `mode != 0`. |
| `record+0x05` | none | n/a | No access in this helper. | Owned by later `80043438` zero write when `mode != 0`. |
| `record+0x06` | none | n/a | No access in this helper. | Owned by later `80043438` zero write when `mode != 0`. |
| `record+0x07` | write byte | `0x60` | `sb $v0, 7($a0)` after `li $v0, 0x60` | Directly translatable; base flag/control byte before `800440D0`. |

### `800440D0(record, enabled)`

Pseudo C from IDA:

```c
int __fastcall sub_800440D0(int a1, int a2)
{
  int result; // $v0

  if ( a2 )
    result = *(unsigned __int8 *)(a1 + 7) | 2;
  else
    result = *(_BYTE *)(a1 + 7) & 0xFD;
  *(_BYTE *)(a1 + 7) = result;
  return result;
}
```

Disassembly:

```asm
800440D0: beqz    $a1, loc_800440E4
800440D4: nop
800440D8: lbu     $v0, 7($a0)
800440DC: j       locret_800440F0
800440E0: ori     $v0, 2
800440E4: lbu     $v0, 7($a0)
800440E8: nop
800440EC: andi    $v0, 0xFD
800440F0: jr      $ra
800440F4: sb      $v0, 7($a0)
```

Xref:

```text
800435B0 -> 800440D0 in sub_80043438
```

Field matrix:

| Field | Access | Value/operation | Evidence | Translation status |
|---|---:|---|---|---|
| `record+0x04` | none | n/a | No access in function. | No owner role here. |
| `record+0x05` | none | n/a | No access in function. | No owner role here. |
| `record+0x06` | none | n/a | No access in function. | No owner role here. |
| `record+0x07` | read byte | current flag/control byte | `lbu $v0, 7($a0)` | Directly translatable. |
| `record+0x07` | write byte | `old | 0x02` when `enabled != 0`; `old & 0xFD` otherwise | `ori $v0, 2` / `andi $v0, 0xFD` / `sb $v0, 7($a0)` | Directly translatable. |

Note: IDA names the first argument as `a1`, but the disassembly is unambiguous:
`$a0` is the record pointer and `$a1` is the boolean branch input.

### `800441C0(packet)`

Pseudo C from IDA:

```c
int __fastcall sub_800441C0(int a1)
{
  int result; // $v0

  *(_BYTE *)(a1 + 3) = 3;
  result = 116;
  *(_BYTE *)(a1 + 7) = 116;
  return result;
}
```

Disassembly:

```asm
800441C0: li      $v0, 3
800441C4: sb      $v0, 3($a0)
800441C8: li      $v0, 0x74
800441CC: jr      $ra
800441D0: sb      $v0, 7($a0)
```

Xref:

```text
80043674 -> 800441C0 in sub_80043438
```

Packet init matrix:

| Packet field | Access | Value | Evidence | Translation status |
|---|---:|---:|---|---|
| `packet+0x03` | write byte | `0x03` | `sb $v0, 3($a0)` after `li $v0, 3` | Directly translatable. |
| `packet+0x04` | none in init | n/a | No access in `800441C0`; later runtime RGB write in `800436F0`. | Consumer-owned per glyph. |
| `packet+0x05` | none in init | n/a | No access in `800441C0`; later runtime RGB write in `800436F0`. | Consumer-owned per glyph. |
| `packet+0x06` | none in init | n/a | No access in `800441C0`; later runtime RGB write in `800436F0`. | Consumer-owned per glyph. |
| `packet+0x07` | write byte | `0x74` | `sb $v0, 7($a0)` after `li $v0, 0x74` | Directly translatable. |
| `packet+0x0E` | not in helper; written by caller | `word_8008EB54` | `8004367C lhu word_8008EB54`; `80043688 sh $v0, 0xE($s0)` | Caller-owned in `80043438`. |

## Caller sequence in `80043438`

The relevant caller evidence from `ida_export_record_bank_init_800433xx_20260510.txt`
shows the exact record index math and ordering. For slot address:

```text
slot = dword_8005CCDC
record = 8005CB5C + slot * 0x30
```

Mode branch sequence:

```asm
80043510: beqz    $s6, loc_800435B8
80043530: jal     sub_80044238
80043534: addu    $a0, $s0
80043554: sb      $zero, 4($v0)
80043574: sb      $zero, 5($v0)
80043594: sb      $zero, 6($v0)
80043580: xori    $a1, $s6, 2
800435A0: sltiu   $a1, 1
800435B0: jal     sub_800440D0
800435B4: addu    $a0, $s0
```

Record `+0x04..+0x07` combined matrix:

| Field | Producer/write order | Effective static value after `80043438` mode branch | Consumer | Translation status |
|---|---|---|---|---|
| `record+0x04` | `FntLoad` bank clear; `80043438` writes zero when `mode != 0` | `0x00` for initialized mode records in this branch | Not used by `800436F0` glyph RGB path; RGB starts from local `0x80` vars | Directly translatable as explicit zero in `80043438`. Dynamic validation only needed if later code claims these bytes have cross-frame semantics. |
| `record+0x05` | Same as `+0x04` | `0x00` | Same as `+0x04` | Directly translatable. |
| `record+0x06` | Same as `+0x04` | `0x00` | Same as `+0x04` | Directly translatable. |
| `record+0x07` | `80044238` writes `0x60`; `800440D0` sets/clears bit `0x02` from `mode == 2` | `0x62` when `mode == 2`; `0x60` otherwise, for this branch | `800436F0` tests nonzero and optionally links `record` through `8004401C` | Directly translatable for static behavior; dynamic verification still useful before naming this as a final-carrier-link flag rather than a broader primitive header/control byte. |

Glyph packet loop:

```asm
80043660: lw      $s0, dword_8005CB7C($v0)
8004366C: blez    $s2, loc_80043698
80043674: jal     sub_800441C0
80043678: move    $a0, $s0
8004367C: lhu     $v0, word_8008EB54
80043688: sh      $v0, 0xE($s0)
80043690: bnez    $v0, loc_80043674
80043694: addiu   $s0, 0x10
```

Glyph packet caller matrix:

| Field | Producer/write order | Effective static value | Translation status |
|---|---|---|---|
| packet base pointer | `80043438` stores `record+0x20 = 8008AB50 + dword_8005D6E4 * 0x10` | First packet for this record | Directly translatable. |
| packet stride | Loop increments `$s0 += 0x10` | `0x10` bytes per glyph packet | Directly translatable. |
| packet `+0x03` | `800441C0` | `0x03` | Directly translatable. |
| packet `+0x07` | `800441C0` | `0x74` | Directly translatable. |
| packet `+0x0E` | `80043438` after `800441C0` | `word_8008EB54` | Directly translatable once `word_8008EB54` is carried from `FntLoad`. |

## Consumer evidence in `800436F0`

`800436F0` Hex-Rays still degrades to `return 0`, so use disassembly for this
part. Existing and new exports agree on this flow:

```asm
80043798: addiu   $a2, $s3, 0x10
800437D0: jal     sub_800440B8
800438DC: sb      $a0, 0xC($s2)
800438E0: sb      $v0, 0xD($s2)
800438E4: sh      $s1, 8($s2)
800438E8: sh      $s5, 0xA($s2)
800438F4: sb      $a2, 4($s2)
80043900: sb      $a2, 5($s2)
8004390C: sb      $a2, 6($s2)
80043914: jal     sub_8004401C
80043918: addiu   $s2, 0x10
80043980: lbu     $v0, 7($s3)
80043988: beqz    $v0, loc_800439C4
80043994: jal     sub_8004401C
80043998: move    $a1, $s3
800439C8: jal     sub_800450A0
800439D8: sw      $zero, 0x28($s3)
800439DC: sb      $zero, 0($v1)
```

List/link matrix:

| Carrier | Init/link owner | Evidence | Translation status |
|---|---|---|---|
| `record+0x10` list head | Initialized by `800440B8(record+0x10)` at flush start | `80043798 addiu $a2, $s3, 0x10`; `800437D0 jal sub_800440B8` | Needs `800440B8` export before fully naming structure, but the callsite and address are translatable. |
| Glyph packet node link | `800436F0` calls `8004401C(record+0x10, packet, ...)` after writing packet fields | `80043914 jal sub_8004401C` with `$a1 = $s2` before delay-slot increment | Needs `8004401C` export for exact primitive/link semantics; callsite is translatable. |
| Optional record self/final carrier link | `800436F0` tests `record+0x07`; if nonzero, calls `8004401C(record+0x10, record, ...)` | `80043980 lbu $v0, 7($s3)`; `80043988 beqz`; `80043994 jal sub_8004401C`; `80043998 move $a1, $s3` | Static condition is translatable. Final name/semantics should remain provisional until `8004401C` is exported or dynamically confirmed. |
| Submit boundary | `800450A0(record+0x10)` | `800439C4 lw $a0, var_30`; `800439C8 jal sub_800450A0` | Callsite translatable; full submit behavior needs existing/next submit export if not already covered elsewhere. |

## What is now safe to translate

- `80044238` as a two-byte record header/control initializer:
  `record[3] = 0x03; record[7] = 0x60`.
- `800440D0` as a bit toggle on `record[7]` bit `0x02`, using the PSX argument
  `mode == 2` computed by `80043438`.
- `80043438` writes to `record[4]`, `record[5]`, and `record[6]` as explicit
  zeroes in the `mode != 0` branch after `80044238`.
- `800441C0` as a two-byte glyph packet initializer:
  `packet[3] = 0x03; packet[7] = 0x74`.
- The `80043438` glyph packet preinit loop with `0x10` stride and
  `packet+0x0E = word_8008EB54`.
- `800436F0` consuming `record+0x07` as a nonzero optional link condition.

## Still needs export or dynamic validation

- `800440B8` and `8004401C` should still be exported before assigning final
  structure names to the list head and primitive link fields. The callsites are
  clear, but their internal link/header mutation is not covered by this export.
- `800450A0` remains the submit boundary. This pass confirms where it is called;
  it does not redefine DrawOTag/submit semantics.
- The final semantic name for `record+0x07` should remain provisional. Static
  evidence proves it is a nonzero-tested header/control byte with bit `0x02`
  mode toggle, not only a boolean. Dynamic validation can confirm whether the
  `0x60/0x62` values ever matter beyond enabling the final carrier link.
- This pass does not close the `80043A14` MIPS vararg formatter path. The `%s`
  and `%d` producer gap remains exactly as described in
  `text_vararg_record_gap_matrix_20260510.md`.

## Direct-port boundary note

The correct route remains PSX text record bank direct-port:

```text
FntLoad / 80043438
  -> record bank + packet preinit
80043A14
  -> PSX text append / formatter producer
800436F0
  -> glyph packet writer + link/submit consumer
```

Do not replace this with a Win text renderer. A Win renderer can only remain a
temporary observation/fallback seam until the PSX producer/consumer chain is
ported and verified.
