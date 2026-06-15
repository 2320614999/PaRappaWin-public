# Stage1 `byte_800493F4` target-frame recorder facts

Scope:

- Target chain:
  `801C7560 -> 8001A3C8/8001A210 -> byte_800493F4 -> 8001A7A4`.
- Recording:
  `E:\game\PSgame\parappa the rapper\duckstation\bin\x64\标题画面-menu-选关选stage1-good通关-保存记忆卡（过场动画均为跳过）.psxrec`
- This is recorder authority for the PSX clock bytes at the existing
  full-song row/writeback target frames. It is not a Win-side success
  inference and does not derive values from STR, ISO, payload, renderer,
  lifecycle, terminal, or no-fail state.

## Target-frame values

All four frames have:

- `dword_80049428 = 0`
- `byte_80057119 = 0x10`
- `byte_800573D4 = 0x02`

| frame | `byte_800493F4` | `byte_800493F5` | `byte_800493F6` |
|---:|---:|---:|---:|
| 3905 | `0x28` | `0x03` | `0x01` |
| 3995 | `0x28` | `0x04` | `0x39` |
| 4255 | `0x28` | `0x08` | `0x65` |
| 4806 | `0x28` | `0x18` | `0x06` |

## Writer windows

Target windows show the same direct writer family:

```text
window 3898..3905
  frame 3904: 8001A454 writes 800493F5 02 -> 03
              8001A45C writes 800493F6 73 -> 00
  frame 3905: 8001A45C writes 800493F6 00 -> 01

window 3990..3995
  frame 3990: 8001A45C writes 800493F6 31 -> 32
  frame 3991: 8001A45C writes 800493F6 32 -> 34
  frame 3992: 8001A45C writes 800493F6 34 -> 35
  frame 3993: 8001A45C writes 800493F6 35 -> 36
  frame 3994: 8001A45C writes 800493F6 36 -> 38
  frame 3995: 8001A45C writes 800493F6 38 -> 39

window 4248..4255
  frame 4248: 8001A45C writes 800493F6 55 -> 56
  frame 4249: 8001A45C writes 800493F6 56 -> 57
  frame 4250: 8001A45C writes 800493F6 57 -> 58
  frame 4251: 8001A45C writes 800493F6 58 -> 60
  frame 4252: 8001A45C writes 800493F6 60 -> 61
  frame 4253: 8001A45C writes 800493F6 61 -> 62
  frame 4254: 8001A45C writes 800493F6 62 -> 63
  frame 4255: 8001A45C writes 800493F6 63 -> 65

window 4798..4806
  frame 4798: 8001A45C writes 800493F6 69 -> 71
  frame 4799: 8001A45C writes 800493F6 71 -> 72
  frame 4800: 8001A45C writes 800493F6 72 -> 73
  frame 4801: 8001A45C writes 800493F6 73 -> 74
  frame 4802: 8001A454 writes 800493F5 17 -> 18
              8001A45C writes 800493F6 74 -> 01
  frame 4803: 8001A45C writes 800493F6 01 -> 02
  frame 4804: 8001A45C writes 800493F6 02 -> 03
  frame 4805: 8001A45C writes 800493F6 03 -> 04
  frame 4806: 8001A45C writes 800493F6 04 -> 06
```

## Integration reading

The next Win runtime comparison should expect the handoff summary to expose:

- `tcPollA3C8=1`
- `tcPollAccept=1`
- `xaCdF4K=True`
- `xaCdF4=28:03:01` near frame 3905,
  `28:04:39` near 3995,
  `28:08:65` near 4255,
  or `28:18:06` near 4806, depending on the sample.
- `xaCdClockK=True` after the existing `80036A78` carrier accepts the MSF
  value.

If `tcPollA3C8=1` but `tcPollAccept=False`, the remaining gap is the
`800364D0/800363A4` lower-CD feedback visible in `tcPollGapD0/tcPollGapA4`.
If `xaCdF4K=True` but `xaCdClockK=False`, the remaining gap is the
MSF-to-LBA carrier rather than the `8001A3C8` writer.

## Call trace and status banks

The target frames also show the expected direct call order into the poll:

```text
frame 3905
  801C757C -> 8001A3C8  sp=80056F28 ra=801C7584
  8001A3D4 -> 800364D0  sp=80056F00 ra=8001A3DC
  8001A41C -> 800363A4  sp=80056F00 ra=8001A424

frame 3995
  801C757C -> 8001A3C8  sp=80056F28 ra=801C7584
  8001A3D4 -> 800364D0  sp=80056F00 ra=8001A3DC
  8001A41C -> 800363A4  sp=80056F00 ra=8001A424

frame 4255
  801C757C -> 8001A3C8  sp=80056F28 ra=801C7584
  8001A3D4 -> 800364D0  sp=80056F00 ra=8001A3DC
  8001A41C -> 800363A4  sp=80056F00 ra=8001A424

frame 4806
  801C757C -> 8001A3C8  sp=80056F28 ra=801C7584
  8001A3D4 -> 800364D0  sp=80056F00 ra=8001A3DC
  8001A41C -> 800363A4  sp=80056F00 ra=8001A424
  801C8104 -> 8001A3B8  sp=801FFF50 ra=801C810C
```

Status banks at the same frames:

| frame | `800882F8[0..7]` | `80088300[0..7]` | `57119` | `573D4` | `573D5` | `573D6` |
|---:|---|---|---:|---:|---:|---:|
| 3905 | `28 03 02 02 01 03 64 00` | `22 00 00 00 00 00 00 00` | `10` | `02` | `01` | `00` |
| 3995 | `28 04 40 02 01 00 00 00` | `22 00 00 00 00 00 00 00` | `10` | `02` | `01` | `00` |
| 4255 | `28 08 66 02 01 02 64 00` | `22 00 00 00 00 00 00 00` | `10` | `02` | `01` | `00` |
| 4806 | `28 18 07 02 01 05 64 00` | `22 00 00 00 00 00 00 00` | `10` | `02` | `01` | `00` |

This narrows the remaining Win check: `tcPollA3C8` must have explicit
`800364D0` sync facts and explicit `800363A4` ready facts before it can accept
`byte_800493F4`. The PSX call trace proves the same frame calls both helpers;
Win must not infer the poll result from the final clock bytes alone.
