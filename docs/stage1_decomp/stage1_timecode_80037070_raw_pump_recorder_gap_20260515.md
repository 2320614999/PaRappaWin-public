# Stage1 80037070 / 80036AF8 raw pump recorder gap

Scope: only `801C7560 -> 8001A3C8 -> 800364D0 -> 80037070 -> 80036AF8`
for the Stage1 timecode source. This note does not authorize lower-CD success
from XA ring state, STR/XA payload, Win files, command serials, or final
`byte_800493F4..F6`.

Recording:

```text
E:\game\PSgame\parappa the rapper\duckstation\bin\x64\标题画面-menu-选关选stage1-good通关-保存记忆卡（过场动画均为跳过）.psxrec
```

## Frame 3905 call chain

Recorder `call_trace 3905` shows the exact target chain:

```text
800359B8 -> 80035EAC -> 801C7560
  801C757C -> 8001A3C8
    8001A3D4 -> 800364D0
      800364D8 -> 80037070
        800371A4 -> 80035898
        800371CC -> 80036AF8
    8001A41C -> 800363A4

800375BC command wait path in the same frame also calls:
  80037688 -> 80037070
    800371A4 -> 80035898
    800371CC -> 80036AF8
```

## Recorder writer facts

Frame `3905` and the narrow window `3904..3905` provide these concrete writes:

```text
writers 0x800573D4 1 3904 3905
  frame 3904 PC=800376D0 02 -> 00
  frame 3904 PC=80036E18 00 -> 02
  frame 3905 PC=800376D0 02 -> 00
  frame 3905 PC=80036E18 00 -> 02

writers 0x800882FA 1 3904 3905
  frame 3904 PC=80036E40 00 -> 01
  frame 3905 PC=80036E40 01 -> 02

writers 0x800882FD 1 3905 3905
  frame 3905 PC=80036E40 02 -> 03

writers 0x80088310 4 3905 3905
  frame 3905 PC=800370C4 00002327 -> 00002328

writers 0x80088314 4 3905 3905
  frame 3905 PC=800370D4 00000001 -> 00000000
  frame 3905 PC=80037114 00000000 -> 00000001
  frame 3905 PC=800370D4 00000001 -> 00000000
  frame 3905 PC=80037114 00000000 -> 00000001

writers 0x80056F30 8 3905 3905
  frame 3905 PC=80036BAC 02 -> 28
  frame 3905 PC=80036BAC 28 -> 22
```

Frame `3905` reads:

```text
0x80055F7A[2] = 0000
0x80057119[1] = 10
0x800573D4[1] = 02
0x800882F8[4] = 02020328
0x800882FC[4] = 00640301
0x80088310[4] = 00002328
0x80088314[4] = 00000001
```

`func_writes 0x80036AF8 0x80037200 3905 3905` ranks the target function
window writes as:

```text
45 total writes
0x80088314: 4 writes
0x80056F38: 4 writes
0x80056F28: 4 writes
...
0x800573D4: 1 write
0x800882FA: 1 write
0x800882FD: 1 write
```

`xref 0x800573D4 3898 3906` and `xref 0x800882FA 3898 3906` show:

```text
0x800573D4:
  800376D0: 9 writes, values [0-0], frames [3898-3906]
  80036E18: 9 writes, values [2-2], frames [3898-3906]

0x800882FA:
  80036E40: 9 writes, values [0-115], frames [3898-3906]
```

## IDA narrow export summary

`80037070` pseudo-C around the target facts:

```text
800370BC: dword_80088310 = sub_80035560(-1) + 960
800370CC: dword_80088314 = 0
80037114: ++dword_80088314 spin counter path
800371A4: if (CheckCallback())
800371C8: saved selector = CD_REG0 & 3
800371CC: while (sub_80036AF8()) ;
80037258: CD_REG0 = saved selector
```

`80036AF8` pseudo-C around the observed bank writes:

```text
80036B14: CD_REG0 = 1
80036B30: interrupt = CD_REG3 & 7
80036BA8: local result byte = CD_REG1
80036BFC: CD_REG3 = 7
80036C0C: CD_REG2 = 7
case 2:
  80036E5C: byte_800573D4 = 2 or 5
  80036E64..80036E88: copy local result bytes to 800882F8
  return 2
case 3 no sync mask:
  80036E14: byte_800573D4 = 2
  80036E18..80036E3C: copy local result bytes to 800882F8
  return 2
```

`800375BC` pseudo-C explains the same-frame `800376D0` reset:

```text
80037688: sub_80037070(0)
800376CC: byte_800573D4 = 0
8003775C: byte_80057119 = command
80037764: CD_REG1 = command
```

## Boundary conclusion

The current recording is sufficient to prove:

- target-frame call order into `8001A3C8 -> 800364D0 -> 80037070`;
- `80037070` executes `CheckCallback()` then calls `80036AF8`;
- `80036AF8` is the bank writer for `byte_800573D4` and `800882F8`;
- `800375BC` resets `byte_800573D4` before issuing command `0x10`.

The current recording is not sufficient to construct a valid
`CdSyncLoopFactsInput80037070`, because it still lacks the instruction-level
CD register facts required by the builder:

- `savedCdReg0SelectorKnown`;
- `selectorRestoredKnown / selectorRestored / restoredCdReg0Selector`;
- ordered `rawPumpSequenceKnown / rawPumpCount`;
- each raw transaction's interrupt value, FIFO status samples, FIFO bytes,
  ack writes, and `psxReturn`;
- `callbackPumpDrained` proven by final `80036AF8` return `0`.

Therefore the next source step is live GDB on the `80037070 -> 80036AF8`
target-frame path, or a recorder enhancement that logs CD_REG/FIFO reads and
`80036AF8` returns. Do not bridge the existing status-bank samples into
`cdSyncFeedback80037070` directly.
