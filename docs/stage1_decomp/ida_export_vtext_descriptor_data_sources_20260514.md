[summary]
success: True
mode: script
filename: E:\game\PSgame\parappa the rapper\win\temp\ida_export_vtext_descriptor_data_sources_20260514.py
persist_globals: False
result_type: NoneType
result_repr: None

[stdout]
# IDA export: vtext descriptor data sources (2026-05-14)

Scope: narrow xref/data dump for `dword_800943CC`, `word_800916D8`, and known Stage1 vtext descriptors.

## Global xrefs

### `dword_800943CC` `800943CC`
- current data value: `<not loaded>`
- `801C4594 -> 800943CC` type=3 caller=`801C455C sub_801C455C` insn=`lw      $v0, dword_800943CC`
- `801C5B60 -> 800943CC` type=2 caller=`801C5B14 sub_801C5B14` insn=`sw      $v0, dword_800943CC`

### `word_800916D8` `800916D8`
- current data value: `<not loaded>`
- `80020584 -> 800916D8` type=3 caller=`80020568 sub_80020568` insn=`lh      $fp, word_800916D8`
- `80020A54 -> 800916D8` type=3 caller=`80020A3C sub_80020A3C` insn=`lh      $s0, word_800916D8`
- `80020BFC -> 800916D8` type=3 caller=`80020BE4 sub_80020BE4` insn=`lh      $s5, word_800916D8`
- `80020FA0 -> 800916D8` type=3 caller=`80020F94 sub_80020F94` insn=`lh      $t0, word_800916D8`
- `800215CC -> 800916D8` type=3 caller=`80021594 sub_80021594` insn=`lh      $t0, word_800916D8`
- `8002192C -> 800916D8` type=3 caller=`80021910 sub_80021910` insn=`lh      $s0, word_800916D8`
- `80021E68 -> 800916D8` type=3 caller=`80021E60 sub_80021E60` insn=`lh      $s0, word_800916D8`
- `80022CD0 -> 800916D8` type=3 caller=`80022CBC sub_80022CBC` insn=`lh      $s0, word_800916D8`
- `80023640 -> 800916D8` type=3 caller=`80023618 sub_80023618` insn=`lh      $s2, word_800916D8`
- `80024CA8 -> 800916D8` type=3 caller=`80024C84 sub_80024C84` insn=`lh      $v0, word_800916D8`
- `80024DE8 -> 800916D8` type=3 caller=`80024CF8 sub_80024CF8` insn=`lh      $v0, word_800916D8`
- `801C4654 -> 800916D8` type=3 caller=`801C455C sub_801C455C` insn=`lhu     $v0, word_800916D8`

## Known Stage1 vtext descriptors

| label | desc | desc[0] | desc[1] | desc[2] | desc[3] | desc[4] | desc[5] entries | desc[6] count | loaded |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---|
| `dword_800943CC_mode0` | `801C6BF8` | `801C69C8` | `801C6A08` | `801C6A48` | `801C6A88` | `801C6AC8` | `801C6B08` | `0000000F` | yes |
| `dword_800943CC_mode1` | `801C6C14` | `00000000` | `00090012` | `000B0014` | `000C0015` | `000D0016` | `000E0017` | `000F0018` | yes |
| `dword_800943CC_mode2` | `801C6C30` | `00100019` | `0011001A` | `00000000` | `0000001D` | `00000000` | `00000018` | `0000001E` | yes |
| `dword_800943CC_mode3` | `801C6C4C` | `00000000` | `00000030` | `0000001C` | `00000000` | `00000048` | `0000001D` | `00000000` | yes |
| `mode0_movie1` | `801CE080` | `00000000` | `00000000` | `00000000` | `00000000` | `00000000` | `00000000` | `00000000` | yes |
| `mode1_short` | `801CE09C` | `00000000` | `00000000` | `00000000` | `00000000` | `00000000` | `00000000` | `00000000` | yes |
| `mode2_short` | `801CE0B8` | `00000000` | `00000000` | `00000000` | `00000000` | `00000000` | `00000000` | `00000000` | yes |

## Writer pseudocode

### `801C5B14`

```c
int sub_801C5B14()
{
  dword_800943C0 = (int)&unk_801C6F88;
  dword_800943C4 = (int)&unk_801C6F68;
  dword_800943C8 = 1;
  dword_800943CC = (int)&off_801C6BF8;
  dword_800943D0 = (int)&unk_801C6DA4;
  dword_800943D8 = (int)&unk_801C6EF0;
  dword_800943E0 = (int)&unk_801C6F18;
  dword_800943E4 = (int)&dword_801C6F84;
  dword_800943FC = (int)&unk_801C6E88;
  dword_80094400 = (int)&unk_801C6E7C;
  dword_80094414 = (int)&unk_801C6EB4;
  dword_80094418 = (int)&unk_801C6EBC;
  dword_8009441C = (int)&unk_801C6EC4;
  dword_80094420 = (int)&unk_801C6ED0;
  dword_80094424 = (int)&unk_801C6ED8;
  dword_80094428 = (int)&unk_801C6EE0;
  dword_8009442C = (int)&unk_801C6EE8;
  dword_80094404 = (int)&unk_801C6E94;
  dword_80094408 = (int)&unk_801C6E9C;
  dword_8009440C = (int)&unk_801C6EA4;
  dword_80094410 = (int)&unk_801C6EAC;
  dword_800943EC = (int)&unk_801C6E5C;
  dword_800943F0 = (int)&unk_801C6E64;
  dword_800943F4 = (int)&unk_801C6E6C;
  dword_800943F8 = (int)&unk_801C6E74;
  dword_80094430 = (int)&nullsub_9;
  dword_80094434 = (int)&nullsub_11;
  dword_800943D4 = dword_801C6DD0;
  dword_800943DC = dword_801C6F14;
  dword_800943E8 = dword_801C6F84;
  dword_80094438 = (int (__fastcall *)(_DWORD))&nullsub_10;
  dword_8009443C = (int)&nullsub_12;
  dword_80094440 = (int)&sub_801C4FC8;
  return 0;
}

```


[console]
# IDA export: vtext descriptor data sources (2026-05-14)

Scope: narrow xref/data dump for `dword_800943CC`, `word_800916D8`, and known Stage1 vtext descriptors.

## Global xrefs

### `dword_800943CC` `800943CC`
- current data value: `<not loaded>`
- `801C4594 -> 800943CC` type=3 caller=`801C455C sub_801C455C` insn=`lw      $v0, dword_800943CC`
- `801C5B60 -> 800943CC` type=2 caller=`801C5B14 sub_801C5B14` insn=`sw      $v0, dword_800943CC`

### `word_800916D8` `800916D8`
- current data value: `<not loaded>`
- `80020584 -> 800916D8` type=3 caller=`80020568 sub_80020568` insn=`lh      $fp, word_800916D8`
- `80020A54 -> 800916D8` type=3 caller=`80020A3C sub_80020A3C` insn=`lh      $s0, word_800916D8`
- `80020BFC -> 800916D8` type=3 caller=`80020BE4 sub_80020BE4` insn=`lh      $s5, word_800916D8`
- `80020FA0 -> 800916D8` type=3 caller=`80020F94 sub_80020F94` insn=`lh      $t0, word_800916D8`
- `800215CC -> 800916D8` type=3 caller=`80021594 sub_80021594` insn=`lh      $t0, word_800916D8`
- `8002192C -> 800916D8` type=3 caller=`80021910 sub_80021910` insn=`lh      $s0, word_800916D8`
- `80021E68 -> 800916D8` type=3 caller=`80021E60 sub_80021E60` insn=`lh      $s0, word_800916D8`
- `80022CD0 -> 800916D8` type=3 caller=`80022CBC sub_80022CBC` insn=`lh      $s0, word_800916D8`
- `80023640 -> 800916D8` type=3 caller=`80023618 sub_80023618` insn=`lh      $s2, word_800916D8`
- `80024CA8 -> 800916D8` type=3 caller=`80024C84 sub_80024C84` insn=`lh      $v0, word_800916D8`
- `80024DE8 -> 800916D8` type=3 caller=`80024CF8 sub_80024CF8` insn=`lh      $v0, word_800916D8`
- `801C4654 -> 800916D8` type=3 caller=`801C455C sub_801C455C` insn=`lhu     $v0, word_800916D8`

## Known Stage1 vtext descriptors

| label | desc | desc[0] | desc[1] | desc[2] | desc[3] | desc[4] | desc[5] entries | desc[6] count | loaded |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---|
| `dword_800943CC_mode0` | `801C6BF8` | `801C69C8` | `801C6A08` | `801C6A48` | `801C6A88` | `801C6AC8` | `801C6B08` | `0000000F` | yes |
| `dword_800943CC_mode1` | `801C6C14` | `00000000` | `00090012` | `000B0014` | `000C0015` | `000D0016` | `000E0017` | `000F0018` | yes |
| `dword_800943CC_mode2` | `801C6C30` | `00100019` | `0011001A` | `00000000` | `0000001D` | `00000000` | `00000018` | `0000001E` | yes |
| `dword_800943CC_mode3` | `801C6C4C` | `00000000` | `00000030` | `0000001C` | `00000000` | `00000048` | `0000001D` | `00000000` | yes |
| `mode0_movie1` | `801CE080` | `00000000` | `00000000` | `00000000` | `00000000` | `00000000` | `00000000` | `00000000` | yes |
| `mode1_short` | `801CE09C` | `00000000` | `00000000` | `00000000` | `00000000` | `00000000` | `00000000` | `00000000` | yes |
| `mode2_short` | `801CE0B8` | `00000000` | `00000000` | `00000000` | `00000000` | `00000000` | `00000000` | `00000000` | yes |

## Writer pseudocode

### `801C5B14`

```c
int sub_801C5B14()
{
  dword_800943C0 = (int)&unk_801C6F88;
  dword_800943C4 = (int)&unk_801C6F68;
  dword_800943C8 = 1;
  dword_800943CC = (int)&off_801C6BF8;
  dword_800943D0 = (int)&unk_801C6DA4;
  dword_800943D8 = (int)&unk_801C6EF0;
  dword_800943E0 = (int)&unk_801C6F18;
  dword_800943E4 = (int)&dword_801C6F84;
  dword_800943FC = (int)&unk_801C6E88;
  dword_80094400 = (int)&unk_801C6E7C;
  dword_80094414 = (int)&unk_801C6EB4;
  dword_80094418 = (int)&unk_801C6EBC;
  dword_8009441C = (int)&unk_801C6EC4;
  dword_80094420 = (int)&unk_801C6ED0;
  dword_80094424 = (int)&unk_801C6ED8;
  dword_80094428 = (int)&unk_801C6EE0;
  dword_8009442C = (int)&unk_801C6EE8;
  dword_80094404 = (int)&unk_801C6E94;
  dword_80094408 = (int)&unk_801C6E9C;
  dword_8009440C = (int)&unk_801C6EA4;
  dword_80094410 = (int)&unk_801C6EAC;
  dword_800943EC = (int)&unk_801C6E5C;
  dword_800943F0 = (int)&unk_801C6E64;
  dword_800943F4 = (int)&unk_801C6E6C;
  dword_800943F8 = (int)&unk_801C6E74;
  dword_80094430 = (int)&nullsub_9;
  dword_80094434 = (int)&nullsub_11;
  dword_800943D4 = dword_801C6DD0;
  dword_800943DC = dword_801C6F14;
  dword_800943E8 = dword_801C6F84;
  dword_80094438 = (int (__fastcall *)(_DWORD))&nullsub_10;
  dword_8009443C = (int)&nullsub_12;
  dword_80094440 = (int)&sub_801C4FC8;
  return 0;
}

```


[result]
null
