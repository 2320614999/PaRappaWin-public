# TEXT formatter/link exports: 80043A14 / 800440B8 / 8004401C

Scope: static-only closure for the TEXT record follow-up. No build, no test,
no emulator, no `src` edits.

Primary raw exports written in this pass:

- `ida_export_text_formatter_link_20260510.md`
- `ida_export_text_formatter_link_20260510.json`
- `ida_export_text_formatter_link_20260510_job.json`

Inputs read before export:

- `docs/项目规则.md`
- `text_record_packet_init_exports_20260510.md`
- `text_vararg_record_gap_matrix_20260510.md`
- `record_producer_branch_20260510.md`
- `80043A14_callsite_order_20260510.md`

IDA MCP status:

- Async IDAPython export completed without timeout.
- Exported functions:
  - `80043A14..80043DF4` as `sub_80043A14`
  - `800440B8..800440D0` as `sub_800440B8`
  - `8004401C..80044058` as `sub_8004401C`
- No timeout recovery was needed; conclusions below are based on the fresh raw
  export plus existing TEXT record documents.

## Static conclusion

This pass closes the minimum static behavior for the PSX text formatter and the
link helper pair:

- `80043A14` is a PSX `FntPrint`-style append/formatter. It selects a text
  record, appends literal bytes, supports `%%`, width parsing, `%d`, `%x/%X`,
  `%c`, and `%s`, advances `record+0x28`, bounds-checks against `record+0x1C`,
  and null-terminates `record+0x24[record+0x28]`.
- `80043A14` has a two-shape vararg ABI:
  - valid first argument as slot: `a0 = slot`, `a1 = fmt`, varargs start at
    saved `a2`;
  - invalid first argument: fallback slot is `dword_8005CCE0`, `a0` is treated
    as `fmt`, varargs start at saved `a1`.
- Hex-Rays still decompiles `80043A14` incorrectly as `return -1`; use the
  disassembly-derived pseudo C below for direct-port planning.
- `800440B8(head)` writes an OT/list terminator by OR-ing low 24 bits with
  `0x00FFFFFF`, preserving the high byte of `head[0]`.
- `8004401C(head, node)` prepends `node` into the low-24-bit link chain:
  `node[0].next = head[0].next`, then `head[0].next = node & 0x00FFFFFF`,
  preserving high-byte packet metadata in both words.

This is enough to direct-translate the formatter/link mechanics, but not enough
to delete the Win fallback renderer by itself. Deleting fallback still requires
the whole record bank + formatter + flush + submit path to be authoritative.

## `80043A14` formatter evidence

### Hex-Rays status

IDA raw decompile:

```c
int sub_80043A14()
{
  return -1;
}
```

This is a decompiler failure. The function body is present and the disassembly
shows the formatter state machine.

### Minimal pseudo C from disassembly

```c
int FntPrint_80043A14(int slot_or_fmt, ...)
{
  int slot;
  const char *fmt;
  uint32_t *ap;
  TextRecord *rec;

  if (slot_or_fmt >= 0 && slot_or_fmt < dword_8005CCDC) {
    slot = slot_or_fmt;
    fmt = saved_a1;
    ap = &saved_a2;
  } else {
    slot = dword_8005CCE0;
    fmt = (const char *)slot_or_fmt;
    ap = &saved_a1;
    if (record(slot)->active_or_text_ptr == 0)
      return -1;
  }

  rec = (TextRecord *)(0x8005CB5C + slot * 0x30);
  if (rec->write_index >= rec->capacity)
    return -1;

  while (*fmt != 0) {
    if (*fmt != '%' || fmt[1] == '%') {
      append_byte_checked(rec, *fmt++);
      continue;
    }

    fmt++;
    bool zero_pad = (*fmt == '0');
    int width = parse_decimal_width(&fmt);
    if (width <= 0)
      width = 1;

    switch (*fmt) {
    case 'd':
      format_signed_decimal_from_next_arg(&ap, scratch, &len);
      break;
    case 'x':
    case 'X':
      format_hex_from_next_arg(&ap, scratch, &len, zero_pad, width);
      break;
    case 'c':
      format_low_byte_from_next_arg(&ap, scratch, &len);
      break;
    case 's':
      scratch = (char *)*ap++;
      len = strlen(scratch);
      break;
    default:
      len = 0;
      break;
    }

    append_left_padding_spaces_if_needed(rec, width, len);
    append_buffer_checked(rec, scratch, len);
    fmt++;
  }

  rec->text[rec->write_index] = 0;
  return rec->write_index;
}
```

The pseudo C intentionally keeps structure names provisional. The record layout
offsets are the evidence-backed contract; final Win field names should wait
until the whole direct-port unit is wired.

### Key disassembly evidence

Argument save area and ABI split:

```asm
80043A14: sw      $a0, -0x20+arg_0($sp)
80043A18: sw      $a1, -0x20+arg_4($sp)
80043A1C: sw      $a2, -0x20+arg_8($sp)
80043A20: sw      $a3, -0x20+arg_C($sp)
80043A28: addiu   $v0, $sp, 0x218+arg_4
80043A4C: bltz    $a0, loc_80043A6C
80043A50: sw      $v0, 0x218+var_8($sp)
80043A60: slt     $v0, $a0, $v0
80043A64: bnez    $v0, loc_80043AA4
80043A68: addiu   $v0, $sp, 0x218+arg_8
80043A6C: lw      $v0, dword_8005CCE0
80043A90: move    $s3, $a0
80043AA4: sw      $v0, 0x218+var_8($sp)
80043AA8: lw      $s3, 0x218+arg_4($sp)
```

Record address and capacity/write-index gate:

```asm
80043AB4: sll     $v0, $v1, 1
80043AB8: addu    $v0, $v1
80043ABC: sll     $v0, 4
80043AC0: li      $v1, dword_8005CB5C
80043AC8: addu    $s1, $v0, $v1
80043ACC: lw      $v1, 0x28($s1)
80043AD0: lw      $v0, 0x1C($s1)
80043AD8: slt     $v0, $v1
80043ADC: bnez    $v0, loc_80043A9C
```

Literal append and `%%` path:

```asm
80043AF4: li      $s5, 0x25  # '%'
80043B00: bne     $a1, $s5, loc_80043B1C
80043B08: addiu   $s3, 1
80043B14: bne     $a1, $s5, loc_80043B54
80043B1C: lw      $a0, 0x28($s1)
80043B24: addiu   $v0, $a0, 1
80043B2C: sw      $v0, 0x28($s1)
80043B30: sb      $a1, 0($v1)
```

Width parse and switch dispatch:

```asm
80043B54: xori    $v0, $a1, 0x30
80043B60: move    $a3, $v0
80043B64: sll     $v0, $s2, 2
80043B70: addiu   $v0, -0x30
80043B74: addu    $s2, $v0, $a1
80043B84: addiu   $v0, $a1, -0x30
80043B88: sltiu   $v0, 0xA
80043BA0: addiu   $v1, $a1, -0x58 # switch 33 cases
80043BB0: lw      $v0, jpt_80043BC0($v0)
80043BC0: jr      $v0
```

`%d` signed decimal:

```asm
80043BC8: lw      $v1, 0x218+var_8($sp)
80043BD0: addiu   $v0, $v1, 4
80043BD4: sw      $v0, 0x218+var_8($sp)
80043BD8: lw      $a0, 0($v1)
80043BE0: bgez    $a0, loc_80043BF0
80043BE8: negu    $a0, $a0
80043BEC: li      $a1, 0x2D  # '-'
80043BF4: multu   $a0, $s4
80043C18: addiu   $v0, 0x30  # '0'
80043C1C: sb      $v0, 0($s0)
```

`%x/%X` hexadecimal:

```asm
80043C48: lw      $v1, 0x218+var_8($sp)
80043C54: sw      $v0, 0x218+var_8($sp)
80043C58: lw      $a0, 0($v1)
80043C60: andi    $v0, $a0, 0xF
80043C64: lw      $v1, off_8005D6E8
80043C70: addu    $v1, $v0
80043C74: lbu     $v0, 0($v1)
80043C80: sb      $v0, 0($s0)
80043C94: beqz    $v0, def_80043BC0
80043CA0: li      $v1, 0x30  # '0'
```

`%c` and `%s`:

```asm
80043CC0: lw      $v0, 0x218+var_8($sp) # case 99
80043CCC: sw      $v1, 0x218+var_8($sp)
80043CD0: lbu     $v0, 0($v0)
80043CDC: sb      $v0, 0($s0)
80043CE0: lw      $v1, 0x218+var_8($sp) # case 115
80043CEC: sw      $v0, 0x218+var_8($sp)
80043CF0: lw      $s0, 0($v1)
80043CF4: jal     strlen
```

Common left-padding, copy, and final terminator:

```asm
80043D0C: li      $a1, 0x20  # ' '
80043D10: lw      $a0, 0x28($s1)
80043D1C: addu    $v1, $a0
80043D20: sw      $v0, 0x28($s1)
80043D24: sb      $a1, 0($v1)
80043D5C: lw      $a0, 0x28($s1)
80043D68: sw      $v0, 0x28($s1)
80043D70: lbu     $v1, 0($s0)
80043D78: sb      $v1, 0($v0)
80043DB4: lw      $v0, 0x24($s1)
80043DB8: lw      $v1, 0x28($s1)
80043DC4: sb      $zero, 0($v0)
80043DC8: lw      $v0, 0x28($s1)
```

### Formatter field/data matrix

| Item | Evidence | Direct translation status |
|---|---|---|
| slot selection | `0 <= a0 < dword_8005CCDC` uses explicit slot; otherwise fallback `dword_8005CCE0` | Directly translatable. |
| vararg save area | saved `a1/a2/a3`; `var_8` points to saved `a2` or saved `a1` depending on ABI shape | Directly translatable as explicit argument-word array. |
| record base/stride | `record = 0x8005CB5C + slot * 0x30` | Directly translatable. |
| `record+0x1C` | capacity/bounds compared against `record+0x28` | Directly translatable. |
| `record+0x24` | text buffer pointer used for writes and final NUL | Directly translatable once bank init owns the pointer. |
| `record+0x28` | write index incremented after every emitted byte and returned on success | Directly translatable. |
| `%%` | emits literal `%` through the normal append path | Directly translatable. |
| width | decimal parser over ASCII digits; default width becomes `1` | Directly translatable. |
| `%d` | consumes next 32-bit argument, handles negative sign, decimal conversion by magic divide | Directly translatable. |
| `%x/%X` | consumes next 32-bit argument, uses `off_8005D6E8` digit table, supports zero-padding when first width char was `0` | Mechanics direct; exact upper/lowercase table value should stay data-backed. |
| `%c` | consumes next argument word and uses its low byte | Directly translatable. |
| `%s` | consumes next pointer and uses `strlen` before common append | Directly translatable if the pointer is mapped into the direct-port memory/data model. |
| overflow return | jumps to `return -1` when appending would exceed `record+0x1C` | Directly translatable. |

## `800440B8` list-head initializer

### IDA pseudo C

```c
int __fastcall sub_800440B8(int *a1)
{
  int result; // $v0

  result = *a1 | 0xFFFFFF;
  *a1 = result;
  return result;
}
```

### Disassembly

```asm
800440B8: lui     $v1, 0xFF
800440BC: lw      $v0, 0($a0)
800440C0: li      $v1, 0xFFFFFF
800440C4: or      $v0, $v1
800440C8: jr      $ra
800440CC: sw      $v0, 0($a0)
```

### Field matrix

| Field | Operation | Translation status |
|---|---|---|
| `head+0x00` high byte | preserved from old word | Directly translatable. |
| `head+0x00` low 24 bits | set to `0xFFFFFF` | Directly translatable as OT/list terminator. |

Xref evidence:

```text
800437D0 -> 800440B8 in sub_800436F0
```

`800436F0` callsite context:

```asm
80043798: addiu   $a2, $s3, 0x10
800437D0: jal     sub_800440B8
```

## `8004401C` link helper

### IDA pseudo C

```c
unsigned int __fastcall sub_8004401C(unsigned int *a1, unsigned int *a2)
{
  unsigned int result; // $v0

  *a2 = *a2 & 0xFF000000 | *a1 & 0xFFFFFF;
  result = *a1 & 0xFF000000 | (unsigned int)a2 & 0xFFFFFF;
  *a1 = result;
  return result;
}
```

### Disassembly

```asm
8004401C: li      $a2, 0xFFFFFF
80044024: lui     $a3, 0xFF00
80044028: lw      $v1, 0($a1)
8004402C: lw      $v0, 0($a0)
80044030: and     $v1, $a3
80044034: and     $v0, $a2
80044038: or      $v1, $v0
8004403C: sw      $v1, 0($a1)
80044040: lw      $v0, 0($a0)
80044044: and     $a1, $a2
80044048: and     $v0, $a3
8004404C: or      $v0, $a1
80044050: jr      $ra
80044054: sw      $v0, 0($a0)
```

### Field matrix

| Field | Operation | Translation status |
|---|---|---|
| `node+0x00` high byte | preserved from old node word | Directly translatable. |
| `node+0x00` low 24 bits | receives old `head+0x00` low 24 bits | Directly translatable. |
| `head+0x00` high byte | preserved from old head word | Directly translatable. |
| `head+0x00` low 24 bits | receives `(uintptr_t)node & 0x00FFFFFF` | Directly translatable, but the Win carrier must explicitly model the PSX 24-bit pointer/tag encoding. |

Xref evidence:

```text
80040184 -> 8004401C in sub_80040060
80043914 -> 8004401C in sub_800436F0
80043994 -> 8004401C in sub_800436F0
```

`800436F0` glyph and optional record-link context from existing export:

```asm
80043914: jal     sub_8004401C
80043918: addiu   $s2, 0x10
80043980: lbu     $v0, 7($s3)
80043988: beqz    $v0, loc_800439C4
80043994: jal     sub_8004401C
80043998: move    $a1, $s3
```

## What can be direct translated now

- `80043A14` slot/fallback selection and two-shape vararg ABI.
- `80043A14` record append into `record+0x24` with `record+0x28` write index
  and `record+0x1C` capacity checks.
- Literal byte append, `%%`, width parsing, `%d`, `%x/%X`, `%c`, `%s`, common
  left-padding, buffer copy, final NUL write, and `-1` overflow return.
- Current known vararg callsites:
  - `80026394`: `"%s:"` consumes group-title pointer from `a1`.
  - `80026DAC`: `&8006EC14` consumes `byte_80092F1D[i]` as at least the first
    `%d` argument.
- `800440B8` low-24 terminator write on `record+0x10`.
- `8004401C` low-24 prepend link for glyph packets and optional final record
  carrier.
- `800436F0` flush side-effect carrier can now explicitly expose the proven
  callsites and record mutation boundary:
  - `800437D0 -> 800440B8(record+0x10)` list-head terminator init.
  - `80043914 -> 8004401C(record+0x10, glyphPacket)` per-glyph prepend link.
  - `80043980/80043988/80043994 -> 8004401C(record+0x10, record)` optional
    final record carrier, gated only by nonzero `record+0x07`.
  - `800439C8 -> 800450A0(record+0x10)` submit boundary.
  - `800439D8` clears `record+0x28`; `800439DC` clears first byte at
    `record+0x24`.

## Submit backend follow-up: `800450A0 / 800468E0 / 80046840`

Existing export `ida_current_decompile_event4_tail_followup_20260510.md`
proves the narrow DrawOTag front:

```c
int __fastcall sub_800450A0(int a1)
{
  return sub_800468E0(sub_80046840, a1, 0, 0);
}
```

The same export proves `80040CA4(work)` feeds it from the work-list tail/head
field:

```c
return sub_800450A0(*(_DWORD *)(a1 + 16));
```

`ida_export_event4_helper_closure_20260510.md` closes the next carrier layer:

```c
sub_80047144();
oldMask = SetIntrMask(0);
dword_8005D73C = 1;
dword_8005D840 = oldMask;
while ((MEMORY[0x1F801814] & 0x04000000) == 0) ;
callback(arg1, arg3);
dword_8005D828 = callback;
dword_8005D82C = arg1;
dword_8005D830 = arg3;
SetIntrMask(0);
return 0;
```

For the `800450A0` caller shape this means:

- `800450A0(head)` calls `800468E0(80046840, head, 0, 0)`.
- `800468E0` waits on GP1 status bit `0x04000000`, masks interrupts, calls the
  callback with `(head, 0)`, then records callback/head/arg3 in
  `8005D828/8005D82C/8005D830`.
- `80046840(head)` is the DMA2 linked-list HAL carrier:
  `GP1(0x1F801814)=0x04000002`, `DMA2_MADR(0x1F8010A0)=head`,
  `DMA2_BCR(0x1F8010A4)=0`, `DMA2_CHCR(0x1F8010A8)=0x01000401`.

Direct-port status:

- `PrPsxEventFrameDirect` can now expose `DrawOtagResult800450A0`,
  `DmaDispatchResult800468E0`, and `DmaLinkedListSubmit80046840` as the thin
  PSX-shaped carrier for TEXT submit and event-frame work-list submit.
- This closes the software call/argument/register-write evidence, but the real
  DMA/GPU timing and interrupt delivery remain a Win HAL boundary. Do not
  execute GPU/DMA from this carrier and do not delete renderer fallback solely
  because this submit carrier is present.

## Still needs dynamic truth or later closure

- `record+0x07` final semantic name remains provisional. Static evidence proves
  it is initialized to `0x60/0x62` and consumed as a nonzero optional final
  carrier-link condition; dynamic truth is still needed before naming it as
  only a boolean or assuming the numeric values have no GPU-side meaning.
- `off_8005D6E8` is closed only to the pointer-slot level in current exports:
  `80043C64` loads the table pointer from `0x8005D6E8`, then indexes one byte
  by the low nibble. The pointed 16 table bytes are not present in the current
  export set, so direct-port code must report `tableKnown=false` and must not
  treat the compatibility digit string as PSX evidence.
- The `off_8005D6E8` carrier should stay separate from `%s` pointer mapping:
  it is a formatter-internal hex-table source gap at `80043C64`, not a caller
  string-data gap. To close it, export the pointer value loaded from
  `0x8005D6E8` plus the pointed 16 bytes from the active SCUS IDB/data image.
- `%s` pointers and `&8006EC14` must be valid in the direct-port memory/data
  model. Static formatter mechanics are closed; live pointed data still needs
  runtime/data ownership. Current evidence only proves `80026394` passes
  `*(group+0x04)` as the `%s` argument; it does not export a PSX
  pointer-to-host-string map for that group-title pointer. To close it, export
  the group/title string table or a precise pointer map for the `80026314`
  menu-help source data.
- `800450A0(record+0x10)` is now closed to the thin
  `800468E0(80046840, head,0,0)` and `80046840` DMA2 register-write carrier.
  Real GPU/DMA completion, interrupt timing, and platform execution remain a
  HAL gap.
- `sub_80040060` also calls `8004401C`; this pass records the xref but does not
  classify that separate caller's packet semantics.

## Direct-port boundary

Minimum direct-port unit for TEXT should remain:

```text
FntLoad / 80043438
  -> record bank + packet preinit
80043A14
  -> PSX formatter/text append producer
800436F0
  -> glyph packet writer and color/control consumer
800440B8 / 8004401C
  -> low-24 OT/list init and prepend link
800450A0
  -> submit boundary
```

Do not replace this path with a Win text renderer as authority. A Win renderer
can only remain a bridge/observation seam until the PSX record bank, formatter,
flush/link, and submit path are cut over and verified.
