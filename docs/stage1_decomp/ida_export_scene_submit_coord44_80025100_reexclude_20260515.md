# IDA recheck: `80025100` is not `80028054` coord44 owner

Date: 2026-05-15

Scope: Stage1 Scene Submit / TOD `coordNode+0x44` authority only.

## Current IDA function

- function: `sub_80024FD0`
- range: `0x80024FD0..0x800259C0`
- size: `0x9F0`

`0x80025100` belongs to `sub_80024FD0`, not `sub_80028054`.

## Local value flow

Current IDA disassembly around the candidate store:

```asm
800250D4  lw      $v0, (dword_800943C8 - 0x800943C8)($a0)
800250E0  slt     $v0, $a1, $v0
800250E4  beqz    $v0, loc_80025104
800250E8  sll     $v0, $a1, 1
800250EC  addu    $v0, $a1
800250F0  lw      $v1, dword_800943C4
800250F8  sll     $v0, 3
800250FC  addu    $v1, $v0
80025100  sw      $v1, 0x44($s1)
80025104  sw      $t0, 0x38($s1)
80025120  lw      $v1, 0x38($s1)
80025158  sw      $a0, 0x40($s1)
80025168  lh      $v1, 0x50($s1)
```

Hex-Rays expresses the same flow as:

```c
if (v7 < dword_800943C8)
  *(_DWORD *)(a1 + 68) = dword_800943C4 + 24 * v7;
*(_DWORD *)(a1 + 56) = v6;
```

So the `+0x44` field here is `a1 + 68` / `$s1 + 0x44` in the
`sub_80024FD0` runner/scorer context. The stored value is a row/descriptor
pointer derived from `dword_800943C4 + 24 * index`.

## Xrefs

Current code xrefs to `sub_80024FD0`:

- `801C4C9C` in `sub_801C4894`: `jal sub_80024FD0`
- `801C4D14` in `sub_801C4894`: `jal sub_80024FD0`

Current code xref to the exact `80025100` instruction is only the intra-function
fallthrough from `800250FC`.

## Classification

`80025100` remains excluded as a Stage1 COORD `+0x44` owner:

- different function and structure: `sub_80024FD0` runner/scorer context, not
  `sub_80028054` COORD node.
- different base register: `$s1` is the `sub_80024FD0` context pointer, not
  `$v0 = mem32[desc+4]` from `80028054`.
- different value source: `dword_800943C4 + 24 * index`, not a type1 TRS
  backing pointer/value.
- adjacent fields `+0x38`, `+0x40`, and `+0x50` are row/lookahead/current
  descriptor and selector state in the same runner/scorer context.

The actual `80028054` COORD backing read remains:

```asm
800280AC  lw      $v0, 4($a1)
800280B4  lw      $s0, 0x44($v0)
800280BC  sw      $zero, 0($v0)
```

This document does not authorize `coord44BackingSlotKnown80028054`; it only
re-confirms that `80025100` must not be used as the owner/writer for that field.
