[summary]
success: True
mode: exec
filename: <mcp-idapython-exec-job>
persist_globals: False
result_var: result
result_var_found: True
result_type: str
result_repr: "# IDA export: text record packet init helpers 2026-05-10\n\n## 80044238 sub_80044238\n\n### Pseudocode\n```c\nint __fastcall sub_80044238(int a1)\n{\n  int result; // $v0\n\n  *(_BYTE *)(a1 + 3) = 3;\n  result = 96;\n  *(_BYTE *)(a1 + 7) = 96;\n  return result;\n}\n\n```\n\n### Disassembly\n```asm\n80044238: li      $v0, 3\n8004423C: sb      $v0, 3($a0)\n80044240: li      $v0, 0x60  # '`'\n80044244: jr      $ra\n80044248: sb      $v0, 7($a0)\n```\n\n### Xrefs to\n```text\n80043530 -> 80044238 type=17 in sub_80043438\n```\n\n## 800440D0 sub_800440D0\n\n### Pseudocode\n```c\nint __fastcall sub_800440D0(int a1, int a2)\n{\n  int result; // $v0\n\n  if ( a2 )\n    result = *(unsigned __int8 *)(a1 + 7) | 2;\n  else\n    result = *(_BYTE *)(a1 + 7) & 0xFD;\n  *(_BYTE *)(a1 + 7) = result;\n  return result;\n}\n\n```\n\n### Disassembly\n```asm\n800440D0: beqz    $a1, loc_800440E4\n800440D4: nop\n800440D8: lbu     $v0, 7($a0)\n800440DC: j       locret_800440F0\n800440E0: ori     $v0, 2\n800440E4: lbu     $v0, 7($a0)\n800440E8: nop\n800440EC: andi    $v0, 0xFD\n800440F0: jr      $ra\n800440F4: sb      $v0, 7($a0)\n```\n\n### Xrefs to\n```text\n800435B0 -> 800440D0 type=17 in sub_80043438\n```\n\n## 800441C0 sub_800441C0\n\n### Pseudocode\n```c\nint __fastcall sub_800441C0(int a1)\n{\n  int result; // $v0\n\n  *(_BYTE *)(a1 + 3) = 3;\n  result = 116;\n  *(_BYTE *)(a1 + 7) = 116;\n  return result;\n}\n\n```\n\n### Disassembly\n```asm\n800441C0: li      $v0, 3\n800441C4: sb      $v0, 3($a0)\n800441C8: li      $v0, 0x74  # 't'\n800441CC: jr      $ra\n800441D0: sb      $v0, 7($a0)\n```\n\n### Xrefs to\n```text\n80043674 -> 800441C0 type=17 in sub_80043438\n```\n\n## Caller/consumer context 80043438 sub_80043438\n\n### Pseudocode\n```c\n// write access to const memory has been detected, the output may be wrong!\nint __fastcall sub_80043438(__int16 a1, __int16 a2, int a3, __int16 a4, int a5, int a6)\n{\n  int v6; // $s2\n  __int16 v9; // $s3\n  int v11; // $s0\n  int i; // $s1\n  int result; // $v0\n  _WORD v14[8]; // [sp+18h] [-10h] BYREF\n\n  v6 = a6;\n  v9 = a3;\n  dword_8005D6E4 = 0;\n  dword_8005CB88 = a3 == 0;\n  if ( a6 >= 1025 )\n    v6 = 1024;\n  v14[2] = 256;\n  v14[3] = 256;\n  v14[0] = 0;\n  v14[1] = 0;\n  ((void (__fastcall *)(int *, _DWORD, _DWORD, _DWORD, _WORD *))sub_80045934)(\n    dword_8005CB6C,\n    0,\n    0,\n    (unsigned __int16)word_8008EB50,\n    v14);\n  if ( a5 )\n  {\n    sub_80044238(dword_8005CB5C);\n    LOBYTE(dword_8005CB5C[1]) = 0;\n    BYTE1(dword_8005CB5C[1]) = 0;\n    BYTE2(dword_8005CB5C[1]) = 0;\n    sub_800440D0(dword_8005CB5C, a5 == 2);\n  }\n  LOWORD(dword_8005CB5C[2]) = a1;\n  HIWORD(dword_8005CB5C[2]) = a2;\n  LOWORD(dword_8005CB5C[3]) = v9;\n  HIWORD(dword_8005CB5C[3]) = a4;\n  dword_8005CB78 = v6;\n  dword_8005CB84 = 0;\n  dword_8005CB80 = (int)&unk_8008A750;\n  dword_8005CB7C = (int)&unk_8008AB50;\n  MEMORY[0] = 0;\n  v11 = 0;\n  for ( i = 0; i < v6; v11 += 16 )\n  {\n    sub_800441C0(v11);\n    ++i;\n    *(_WORD *)(v11 + 14) = word_8008EB54;\n  }\n  result = 0;\n  dword_8005D6E4 = v6;\n  dword_8005CCDC = 1;\n  return result;\n}\n\n```\n\n### Disassembly calls/field accesses excerpt\n```asm\n80043530: jal     sub_80044238\n800435B0: jal     sub_800440D0\n80043674: jal     sub_800441C0\n```\n\n## Caller/consumer context 800436F0 sub_800436F0\n\n### Pseudocode\n```c\nint sub_800436F0()\n{\n  return 0;\n}\n\n```\n\n### Disassembly calls/field accesses excerpt\n```asm\n800437D0: jal     sub_800440B8\n80043824: li      $v0, 0x7E  # '~'\n80043914: jal     sub_8004401C\n80043980: lbu     $v0, 7($s3)\n80043994: jal     sub_8004401C\n800439C8: jal     sub_800450A0\n```\n"

[stdout]
# IDA export: text record packet init helpers 2026-05-10

## 80044238 sub_80044238

### Pseudocode
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

### Disassembly
```asm
80044238: li      $v0, 3
8004423C: sb      $v0, 3($a0)
80044240: li      $v0, 0x60  # '`'
80044244: jr      $ra
80044248: sb      $v0, 7($a0)
```

### Xrefs to
```text
80043530 -> 80044238 type=17 in sub_80043438
```

## 800440D0 sub_800440D0

### Pseudocode
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

### Disassembly
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

### Xrefs to
```text
800435B0 -> 800440D0 type=17 in sub_80043438
```

## 800441C0 sub_800441C0

### Pseudocode
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

### Disassembly
```asm
800441C0: li      $v0, 3
800441C4: sb      $v0, 3($a0)
800441C8: li      $v0, 0x74  # 't'
800441CC: jr      $ra
800441D0: sb      $v0, 7($a0)
```

### Xrefs to
```text
80043674 -> 800441C0 type=17 in sub_80043438
```

## Caller/consumer context 80043438 sub_80043438

### Pseudocode
```c
// write access to const memory has been detected, the output may be wrong!
int __fastcall sub_80043438(__int16 a1, __int16 a2, int a3, __int16 a4, int a5, int a6)
{
  int v6; // $s2
  __int16 v9; // $s3
  int v11; // $s0
  int i; // $s1
  int result; // $v0
  _WORD v14[8]; // [sp+18h] [-10h] BYREF

  v6 = a6;
  v9 = a3;
  dword_8005D6E4 = 0;
  dword_8005CB88 = a3 == 0;
  if ( a6 >= 1025 )
    v6 = 1024;
  v14[2] = 256;
  v14[3] = 256;
  v14[0] = 0;
  v14[1] = 0;
  ((void (__fastcall *)(int *, _DWORD, _DWORD, _DWORD, _WORD *))sub_80045934)(
    dword_8005CB6C,
    0,
    0,
    (unsigned __int16)word_8008EB50,
    v14);
  if ( a5 )
  {
    sub_80044238(dword_8005CB5C);
    LOBYTE(dword_8005CB5C[1]) = 0;
    BYTE1(dword_8005CB5C[1]) = 0;
    BYTE2(dword_8005CB5C[1]) = 0;
    sub_800440D0(dword_8005CB5C, a5 == 2);
  }
  LOWORD(dword_8005CB5C[2]) = a1;
  HIWORD(dword_8005CB5C[2]) = a2;
  LOWORD(dword_8005CB5C[3]) = v9;
  HIWORD(dword_8005CB5C[3]) = a4;
  dword_8005CB78 = v6;
  dword_8005CB84 = 0;
  dword_8005CB80 = (int)&unk_8008A750;
  dword_8005CB7C = (int)&unk_8008AB50;
  MEMORY[0] = 0;
  v11 = 0;
  for ( i = 0; i < v6; v11 += 16 )
  {
    sub_800441C0(v11);
    ++i;
    *(_WORD *)(v11 + 14) = word_8008EB54;
  }
  result = 0;
  dword_8005D6E4 = v6;
  dword_8005CCDC = 1;
  return result;
}

```

### Disassembly calls/field accesses excerpt
```asm
80043530: jal     sub_80044238
800435B0: jal     sub_800440D0
80043674: jal     sub_800441C0
```

## Caller/consumer context 800436F0 sub_800436F0

### Pseudocode
```c
int sub_800436F0()
{
  return 0;
}

```

### Disassembly calls/field accesses excerpt
```asm
800437D0: jal     sub_800440B8
80043824: li      $v0, 0x7E  # '~'
80043914: jal     sub_8004401C
80043980: lbu     $v0, 7($s3)
80043994: jal     sub_8004401C
800439C8: jal     sub_800450A0
```



[console]
# IDA export: text record packet init helpers 2026-05-10

## 80044238 sub_80044238

### Pseudocode
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

### Disassembly
```asm
80044238: li      $v0, 3
8004423C: sb      $v0, 3($a0)
80044240: li      $v0, 0x60  # '`'
80044244: jr      $ra
80044248: sb      $v0, 7($a0)
```

### Xrefs to
```text
80043530 -> 80044238 type=17 in sub_80043438
```

## 800440D0 sub_800440D0

### Pseudocode
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

### Disassembly
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

### Xrefs to
```text
800435B0 -> 800440D0 type=17 in sub_80043438
```

## 800441C0 sub_800441C0

### Pseudocode
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

### Disassembly
```asm
800441C0: li      $v0, 3
800441C4: sb      $v0, 3($a0)
800441C8: li      $v0, 0x74  # 't'
800441CC: jr      $ra
800441D0: sb      $v0, 7($a0)
```

### Xrefs to
```text
80043674 -> 800441C0 type=17 in sub_80043438
```

## Caller/consumer context 80043438 sub_80043438

### Pseudocode
```c
// write access to const memory has been detected, the output may be wrong!
int __fastcall sub_80043438(__int16 a1, __int16 a2, int a3, __int16 a4, int a5, int a6)
{
  int v6; // $s2
  __int16 v9; // $s3
  int v11; // $s0
  int i; // $s1
  int result; // $v0
  _WORD v14[8]; // [sp+18h] [-10h] BYREF

  v6 = a6;
  v9 = a3;
  dword_8005D6E4 = 0;
  dword_8005CB88 = a3 == 0;
  if ( a6 >= 1025 )
    v6 = 1024;
  v14[2] = 256;
  v14[3] = 256;
  v14[0] = 0;
  v14[1] = 0;
  ((void (__fastcall *)(int *, _DWORD, _DWORD, _DWORD, _WORD *))sub_80045934)(
    dword_8005CB6C,
    0,
    0,
    (unsigned __int16)word_8008EB50,
    v14);
  if ( a5 )
  {
    sub_80044238(dword_8005CB5C);
    LOBYTE(dword_8005CB5C[1]) = 0;
    BYTE1(dword_8005CB5C[1]) = 0;
    BYTE2(dword_8005CB5C[1]) = 0;
    sub_800440D0(dword_8005CB5C, a5 == 2);
  }
  LOWORD(dword_8005CB5C[2]) = a1;
  HIWORD(dword_8005CB5C[2]) = a2;
  LOWORD(dword_8005CB5C[3]) = v9;
  HIWORD(dword_8005CB5C[3]) = a4;
  dword_8005CB78 = v6;
  dword_8005CB84 = 0;
  dword_8005CB80 = (int)&unk_8008A750;
  dword_8005CB7C = (int)&unk_8008AB50;
  MEMORY[0] = 0;
  v11 = 0;
  for ( i = 0; i < v6; v11 += 16 )
  {
    sub_800441C0(v11);
    ++i;
    *(_WORD *)(v11 + 14) = word_8008EB54;
  }
  result = 0;
  dword_8005D6E4 = v6;
  dword_8005CCDC = 1;
  return result;
}

```

### Disassembly calls/field accesses excerpt
```asm
80043530: jal     sub_80044238
800435B0: jal     sub_800440D0
80043674: jal     sub_800441C0
```

## Caller/consumer context 800436F0 sub_800436F0

### Pseudocode
```c
int sub_800436F0()
{
  return 0;
}

```

### Disassembly calls/field accesses excerpt
```asm
800437D0: jal     sub_800440B8
80043824: li      $v0, 0x7E  # '~'
80043914: jal     sub_8004401C
80043980: lbu     $v0, 7($s3)
80043994: jal     sub_8004401C
800439C8: jal     sub_800450A0
```



[result]
"# IDA export: text record packet init helpers 2026-05-10\n\n## 80044238 sub_80044238\n\n### Pseudocode\n```c\nint __fastcall sub_80044238(int a1)\n{\n  int result; // $v0\n\n  *(_BYTE *)(a1 + 3) = 3;\n  result = 96;\n  *(_BYTE *)(a1 + 7) = 96;\n  return result;\n}\n\n```\n\n### Disassembly\n```asm\n80044238: li      $v0, 3\n8004423C: sb      $v0, 3($a0)\n80044240: li      $v0, 0x60  # '`'\n80044244: jr      $ra\n80044248: sb      $v0, 7($a0)\n```\n\n### Xrefs to\n```text\n80043530 -> 80044238 type=17 in sub_80043438\n```\n\n## 800440D0 sub_800440D0\n\n### Pseudocode\n```c\nint __fastcall sub_800440D0(int a1, int a2)\n{\n  int result; // $v0\n\n  if ( a2 )\n    result = *(unsigned __int8 *)(a1 + 7) | 2;\n  else\n    result = *(_BYTE *)(a1 + 7) & 0xFD;\n  *(_BYTE *)(a1 + 7) = result;\n  return result;\n}\n\n```\n\n### Disassembly\n```asm\n800440D0: beqz    $a1, loc_800440E4\n800440D4: nop\n800440D8: lbu     $v0, 7($a0)\n800440DC: j       locret_800440F0\n800440E0: ori     $v0, 2\n800440E4: lbu     $v0, 7($a0)\n800440E8: nop\n800440EC: andi    $v0, 0xFD\n800440F0: jr      $ra\n800440F4: sb      $v0, 7($a0)\n```\n\n### Xrefs to\n```text\n800435B0 -> 800440D0 type=17 in sub_80043438\n```\n\n## 800441C0 sub_800441C0\n\n### Pseudocode\n```c\nint __fastcall sub_800441C0(int a1)\n{\n  int result; // $v0\n\n  *(_BYTE *)(a1 + 3) = 3;\n  result = 116;\n  *(_BYTE *)(a1 + 7) = 116;\n  return result;\n}\n\n```\n\n### Disassembly\n```asm\n800441C0: li      $v0, 3\n800441C4: sb      $v0, 3($a0)\n800441C8: li      $v0, 0x74  # 't'\n800441CC: jr      $ra\n800441D0: sb      $v0, 7($a0)\n```\n\n### Xrefs to\n```text\n80043674 -> 800441C0 type=17 in sub_80043438\n```\n\n## Caller/consumer context 80043438 sub_80043438\n\n### Pseudocode\n```c\n// write access to const memory has been detected, the output may be wrong!\nint __fastcall sub_80043438(__int16 a1, __int16 a2, int a3, __int16 a4, int a5, int a6)\n{\n  int v6; // $s2\n  __int16 v9; // $s3\n  int v11; // $s0\n  int i; // $s1\n  int result; // $v0\n  _WORD v14[8]; // [sp+18h] [-10h] BYREF\n\n  v6 = a6;\n  v9 = a3;\n  dword_8005D6E4 = 0;\n  dword_8005CB88 = a3 == 0;\n  if ( a6 >= 1025 )\n    v6 = 1024;\n  v14[2] = 256;\n  v14[3] = 256;\n  v14[0] = 0;\n  v14[1] = 0;\n  ((void (__fastcall *)(int *, _DWORD, _DWORD, _DWORD, _WORD *))sub_80045934)(\n    dword_8005CB6C,\n    0,\n    0,\n    (unsigned __int16)word_8008EB50,\n    v14);\n  if ( a5 )\n  {\n    sub_80044238(dword_8005CB5C);\n    LOBYTE(dword_8005CB5C[1]) = 0;\n    BYTE1(dword_8005CB5C[1]) = 0;\n    BYTE2(dword_8005CB5C[1]) = 0;\n    sub_800440D0(dword_8005CB5C, a5 == 2);\n  }\n  LOWORD(dword_8005CB5C[2]) = a1;\n  HIWORD(dword_8005CB5C[2]) = a2;\n  LOWORD(dword_8005CB5C[3]) = v9;\n  HIWORD(dword_8005CB5C[3]) = a4;\n  dword_8005CB78 = v6;\n  dword_8005CB84 = 0;\n  dword_8005CB80 = (int)&unk_8008A750;\n  dword_8005CB7C = (int)&unk_8008AB50;\n  MEMORY[0] = 0;\n  v11 = 0;\n  for ( i = 0; i < v6; v11 += 16 )\n  {\n    sub_800441C0(v11);\n    ++i;\n    *(_WORD *)(v11 + 14) = word_8008EB54;\n  }\n  result = 0;\n  dword_8005D6E4 = v6;\n  dword_8005CCDC = 1;\n  return result;\n}\n\n```\n\n### Disassembly calls/field accesses excerpt\n```asm\n80043530: jal     sub_80044238\n800435B0: jal     sub_800440D0\n80043674: jal     sub_800441C0\n```\n\n## Caller/consumer context 800436F0 sub_800436F0\n\n### Pseudocode\n```c\nint sub_800436F0()\n{\n  return 0;\n}\n\n```\n\n### Disassembly calls/field accesses excerpt\n```asm\n800437D0: jal     sub_800440B8\n80043824: li      $v0, 0x7E  # '~'\n80043914: jal     sub_8004401C\n80043980: lbu     $v0, 7($s3)\n80043994: jal     sub_8004401C\n800439C8: jal     sub_800450A0\n```\n"
