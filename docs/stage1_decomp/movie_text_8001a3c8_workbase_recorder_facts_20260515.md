# 8001A3C8 / 8001A3B8 workbase recorder facts (2026-05-15)

Scope:
- Target seam: `801C455C` movie/text outer-loop `SnapshotWorkBaseSub8001A3C8`
  followed by `CheckGsGetWorkBase(8001A3B8)`.
- Recording:
  `E:\game\PSgame\parappa the rapper\duckstation\bin\x64\标题画面-menu-选关选stage1-good通关-保存记忆卡（过场动画均为跳过）.psxrec`
- This recording has call trace and write log enabled, but read log disabled.

Boundary:
- This is new recorder evidence for the shared `8001A3C8 -> 8001A3B8`
  workbase mechanism.
- It is not enough to cut over `801C455C`, because this recording skipped
  movies and has no `801C455C` call-trace hit.
- Do not infer workbase success from STR playback, host movie progress,
  renderer state, ISO/file readability, payload bytes, or default zeroes.

## Coverage

Targeted call-trace scan over frames `0..14317`:

```text
801C455C: 0 frames
8001A3C8: 9686 frames
8001A3B8: 5254 frames
800364D0: 9831 frames
800363A4: 9686 frames
```

Representative early frame:

```text
frame 1338:
  801C752C -> 8001A3C8, sp=801FFF38
  8001A3D4 -> 800364D0, sp=801FFF10
  8001A41C -> 800363A4, sp=801FFF10
  801C7978 -> 8001A3B8, sp=801FFF58
```

Representative tail frame:

```text
frame 14205:
  801C6824 -> 8001A3C8, sp=801FFF38
  8001A3D4 -> 800364D0, sp=801FFF10
  8001A41C -> 800363A4, sp=801FFF10
  801C6C70 -> 8001A3B8, sp=801FFF58
```

## Facts

`8001A3C8` uses its local status buffer at `sp+0x10`, so in the frames above
the stack status buffer is `0x801FFF20`.

Frame `1338` after the poll:

```text
status[0..3] @ 801FFF20 = E8 6E 22 00
dword_80049428 = 0
byte_800493F4 = 0x58
byte_800493F5 = 0x26
byte_800493F6 = 0x20
byte_80057119 = 0x10
byte_800573D4 = 0x02
```

Write log:

```text
8001A44C writes byte_800493F4: 00 -> 58
8001A454 writes byte_800493F5: 00 -> 26
8001A45C writes byte_800493F6: 00 -> 20
800376D0 writes byte_800573D4: 02 -> 00
80036E18 writes byte_800573D4: 00 -> 02
```

Frame `1340`:

```text
status[0..3] @ 801FFF20 = E8 6E 22 00
dword_80049428 = 0
byte_800493F4 = 0x58
byte_800493F5 = 0x26
byte_800493F6 = 0x25
byte_80057119 = 0x10
byte_800573D4 = 0x02
8001A45C writes byte_800493F6: 20 -> 25
```

Frame `14205`:

```text
status[0..3] @ 801FFF20 = E8 6E 22 00
dword_80049428 = 0
byte_800493F4 = 0x53
byte_800493F5 = 0x35
byte_800493F6 = 0x71
byte_80057119 = 0x10
byte_800573D4 = 0x02
```

Write log:

```text
8001A44C writes byte_800493F4: 30 -> 53
8001A454 writes byte_800493F5: 43 -> 35
8001A45C writes byte_800493F6: 43 -> 71
800376D0 writes byte_800573D4: 02 -> 00
80036E18 writes byte_800573D4: 00 -> 02
```

Frame `14317`:

```text
status[0..3] @ 801FFF20 = E8 6E 22 00
dword_80049428 = 0
byte_800493F4 = 0x53
byte_800493F5 = 0x39
byte_800493F6 = 0x52
byte_80057119 = 0x10
byte_800573D4 = 0x02
8001A45C writes byte_800493F6: 47 -> 52
```

## Conclusion

The recorder confirms the shared `8001A3C8` normal path:

- `800364D0(1, sp+0x10)` provides status bytes.
- `800363A4()` is called in the `8001A3C8` path.
- `8001A3C8` writes `byte_800493F4..F6` from status bytes.
- `8001A3B8` is called in the same frame and reads the workbase global path.
- `dword_80049428` remains `0` in these sampled frames; same-value writes are
  not visible as write-log changes.

The remaining `801C455C` runtime gap is not static control flow. It is the
absence of exact same-tick lower/CD feedback for the `801C455C` path in the Win
runtime, and the current PSX recording does not cover `801C455C` because movies
were skipped.

Next evidence options:
- Capture a non-skipped movie recording that hits `801C455C`, then collect
  `801C455C -> 801C448C -> 8001A3C8 -> 800364D0 -> 800363A4` and
  `801C455C -> 8001A3B8` in the same frame.
- Or use live GDB on the `801C455C` movie path to capture `$pc/$ra/sp`,
  `sp+0x10` status bytes, `byte_80057119`, `byte_800493F4..F6`, and
  `dword_80049428`.
