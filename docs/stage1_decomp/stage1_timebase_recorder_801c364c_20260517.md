# Stage1 timebase recorder facts: `801C7560 -> 801C364C`

录制：

- `E:\game\PSgame\parappa the rapper\duckstation\标题画面-menu-选关选stage1-good通关-保存记忆卡（存档重名保存失败重试成功）（过场动画均为跳过）v3.psxrec`
- `PSXREC03`，14386 frames，VRAM/write-log present。

## Call-chain hit

`call_trace 3000` 与 `call_trace 6000` 均命中：

```text
80035EAC
  -> 801C7560
     -> 80026ECC
     -> 8001A3C8
        -> 800364D0
           -> 80037070
     -> 8001A280
        -> 80036678
     -> 8001A7A4
```

这说明录制帧 3000/6000 覆盖 Stage1 主循环时间基准路径，
不是菜单或表现层采样。

## Writer hit

`writers 0x801C364C 4 2950 3050`：

```text
Frame 2950  PC=0x801C7634  0x00000DDA -> 0x00000DDC
Frame 2999  PC=0x801C7634  0x00000E69 -> 0x00000E6D
Frame 3000  PC=0x801C7634  0x00000E6D -> 0x00000E70
...
PC ranking:
  0x801C7634: 101 writes
```

`801C364C` 的写入 owner 在该窗口内唯一为 `801C7634`，属于
`801C7560` 内部写回路径。

## Value samples

```text
Frame 3000:
  0x801C3640 = 0x00000000
  0x801C3644 = 0x3103000A
  0x801C364C = 0x00000E70 (3696)

Frame 6000:
  0x801C3640 = 0x00008001
  0x801C3644 = 0x2B030021
  0x801C364C = 0x000030EA (12522)

Frame 9000:
  0x801C3640 = 0x00008011
  0x801C3644 = 0x27030038
  0x801C364C = 0x00005366 (21350)
```

Delta:

- frame 3000 -> 6000: `12522 - 3696 = 8826`, about `2.942` ticks per recorded frame.
- frame 6000 -> 9000: `21350 - 12522 = 8828`, about `2.943` ticks per recorded frame.
- DuckStation recording frame here is 60Hz-visible cadence; this equals about
  `5.886` `801C364C` ticks per 30Hz logic frame.

## Boundary conclusion

The current Win direct runtime log after the `801C7560` cutover showed direct
ticks such as:

```text
query=1436 tick96=8513 tcRound=8417
query=1592 tick96=9433 tcRound=9337
```

Those values are in the same order and cadence as the PSX recorder facts above.
Therefore the observed half-speed / stutter should not currently be treated as
pollution of the direct Stage1 main timebase, full-song summary, row-writeback,
or scorer authority.

Keep visual/audio cadence issues as later integration observation unless a new
PSX recorder/live fact shows `801C364C` diverging at the same path.
