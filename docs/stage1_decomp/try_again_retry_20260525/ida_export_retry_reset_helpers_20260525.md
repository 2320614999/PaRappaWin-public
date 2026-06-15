# Stage1 Try Again retry reset helper exports - 2026-05-25

## Scope

- Branch: `codex/stage1-gpt55-pro-handoff-20260524`
- Runtime baseline: `70c1bf89`
- Purpose: local IDA evidence requested by the web Pro audit after the
  Try Again YES retry still appeared to jump forward to the old fail point.
- Authority: Hex-Rays pseudo-C and disassembly from the local IDA session.

## `80014C1C` / `sub_80014C1C`

Pseudo-C:

```c
int sub_80014C1C()
{
  int result; // $v0

  result = dword_80091808;
  word_80091810 = 0;
  word_80091812 = 0;
  word_80091814 = 0;
  word_80091822 = 0;
  dword_80091808 = 0;
  dword_8009180C = result;
  return result;
}
```

Disassembly and xrefs:

```text
bounds 80014C1C-80014C5C
80014C1C: lw      $v0, dword_80091808
80014C24: sh      $zero, word_80091810
80014C2C: sh      $zero, word_80091812
80014C34: sh      $zero, word_80091814
80014C3C: sh      $zero, word_80091822
80014C44: sw      $zero, dword_80091808
80014C4C: sw      $v0, dword_8009180C
80014C54: jr      $ra
80014C58: nop
xrefs_to:
800143C0->80014C1C type=17 caller=sub_80014344
80025760->80014C1C type=17 caller=sub_80024FD0
8002598C->80014C1C type=17 caller=sub_80024FD0
```

## `8001448C` / `sub_8001448C`

Pseudo-C:

```c
int sub_8001448C()
{
  int result; // $v0

  result = (unsigned __int16)word_80091816;
  word_80091818 = word_80091816;
  word_8009181A = word_80091816;
  word_8009181C = word_80091816;
  return result;
}
```

Disassembly and xrefs:

```text
bounds 8001448C-800144B8
8001448C: lhu     $v0, word_80091816
80014494: nop
80014498: sh      $v0, word_80091818
800144A0: sh      $v0, word_8009181A
800144A8: sh      $v0, word_8009181C
800144B0: jr      $ra
800144B4: nop
xrefs_to:
800143C8->8001448C type=17 caller=sub_80014344
80014440->8001448C type=17 caller=sub_80014400
80024FA8->8001448C type=17 caller=sub_80024F8C
```

## `80024F8C` / `sub_80024F8C`

Pseudo-C:

```c
int __fastcall sub_80024F8C(int a1)
{
  word_8008ED36 = 2;
  word_8008ED38 = 0;
  *(_WORD *)(a1 + 90) = 0;
  return sub_8001448C();
}
```

Disassembly and xrefs:

```text
bounds 80024F8C-80024FC0
80024F8C: addiu   $sp, -0x18
80024F90: li      $v0, 2
80024F94: sw      $ra, 0x10+var_s0($sp)
80024F98: sh      $v0, word_8008ED36
80024FA0: sh      $zero, word_8008ED38
80024FA8: jal     sub_8001448C
80024FAC: sh      $zero, 0x5A($a0)
80024FB0: lw      $ra, 0x10+var_s0($sp)
80024FB4: addiu   $sp, 0x18
80024FB8: jr      $ra
80024FBC: nop
xrefs_to:
80024F6C->80024F8C type=17 caller=sub_80024E98
80025544->80024F8C type=17 caller=sub_80024FD0
80025994->80024F8C type=17 caller=sub_80024FD0
```

## `80024FC0` / `sub_80024FC0`

Pseudo-C:

```c
int __fastcall sub_80024FC0(int a1)
{
  int result; // $v0

  result = word_80091816;
  *(_DWORD *)(a1 + 48) = word_80091816;
  return result;
}
```

Disassembly and xrefs:

```text
bounds 80024FC0-80024FD0
80024FC0: lh      $v0, word_80091816
80024FC8: jr      $ra
80024FCC: sw      $v0, 0x30($a0)
xrefs_to:
800258C0->80024FC0 type=17 caller=sub_80024FD0
801C4934->80024FC0 type=17 caller=sub_801C4894
```
