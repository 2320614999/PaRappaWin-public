# Stage1 `byte_80057119=0x10` Writer Evidence

Scope: `801C7560 -> 8001A3C8/8001A210 -> byte_800493F4 -> 8001A7A4`.

## Recorder hit

Recording:

`E:\game\PSgame\parappa the rapper\duckstation\bin\x64\标题画面-menu-选关选stage1-good通关-保存记忆卡（过场动画均为跳过）.psxrec`

`0x80057119` value transitions before the target status frames:

| frame | value |
| ---: | ---: |
| 1335 | `0x01` |
| 1336 | `0x10` |
| 1337 | `0x10` |

Narrow writer query:

`python tools/模拟器内存回放器.py <recording> writers 0x80057119 1 1328 1338`

Result:

| frame | PC | address | old | new |
| ---: | ---: | ---: | ---: | ---: |
| 1336 | `0x80037764` | `0x80057119` | `0x01` | `0x10` |

Filtered frame-1336 write-log context:

| index | PC | address | write |
| ---: | ---: | ---: | --- |
| 530 | `0x800376D0` | `0x800573D4` | `0x02 -> 0x00` |
| 531 | `0x80037764` | `0x80057119` | `0x01 -> 0x10` |

Frame-1336 call trace around the writer:

| event | call |
| ---: | --- |
| 1892 | `801C7898 -> 8001A280` |
| 1895 | `8001A298 -> 80036678` |
| 1906 | `80036748 -> 800375BC` |
| 1919 | `80037688 -> 80037070` |

## IDA evidence

Narrow export:

`docs/stage1_decomp/ida_export_stage1_timecode_byte57119_writer_20260515.json`

Static chain:

- `8001A280` checks `dword_80049428`; if non-zero it returns without issuing the command.
- If `dword_80049428 == 0`, `8001A280` executes:
  - `8001A294: li $a0, 0x10`
  - `8001A298: jal sub_80036678`
  - `8001A29C: move $a1, $zero`
- `80036678` preserves the command in `$s3`, then reaches:
  - `8003673C: andi $a0, $s3, 0xFF`
  - `80036748: jal sub_800375BC`
  - `8003674C: li $a3, 1`
- `800375BC` writes the command:
  - `8003775C: sb $s3, byte_80057119`
  - frame writer log reports the mirrored write at `80037764` to `0x80057119`.

## Authority boundary

This closes the target-frame command source as:

`801C7898 -> 8001A280 -> 80036678(0x10, 0) -> 800375BC(0x10, 0, 0, 1) -> byte_80057119=0x10`

The authority is the recorder writer hit plus IDA call-chain proof. Do not derive this command from `lastCdCommand`, STR/XA playback, Win file/ISO readability, payload bytes, or final `byte_800493F4/F5/F6` values.

Runtime still needs to ensure the Stage1 main path executes the existing direct `8001A280/80036678/800375BC` equivalent before the `8001A3C8` poll; the command source itself is no longer an open evidence gap.
