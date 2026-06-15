# StageClear/MenuHelp `800916F6` Offline Writer Windows - 2026-05-13

Source recording:

- `E:\game\PSgame\parappa the rapper\duckstation\bin\x64\标题画面-menu-选关选stage1-good通关-保存记忆卡（过场动画均为跳过）.psxrec`

Tool:

- `tools/模拟器内存回放器.py`

This is offline write-log evidence only. It does not authorize deriving
MenuHelp/StageClear producer truth from replay state.

## Tool Boundary

`PSXREC03` has write logs and call traces. The `writers` command can return
writer `frame` and `PC`, but `find_writers(...)` scans frame by frame and has no
address index. Full-range `writers 0x800916F6 2 ...` / `xref ...` can time out
on this recording.

Use narrow windows around already-known events. Also scan `0x800916F7` as a
single byte because a byte store to the high halfword byte would not be covered
by only checking the halfword start address.

## Narrow Scans

Commands:

```text
python tools\模拟器内存回放器.py <rec> writers 0x800916F6 2 430 640
python tools\模拟器内存回放器.py <rec> writers 0x800916F7 1 430 640
python tools\模拟器内存回放器.py <rec> writers 0x800916F6 2 12280 12340
python tools\模拟器内存回放器.py <rec> writers 0x800916F7 1 12280 12340
python tools\模拟器内存回放器.py <rec> writers 0x800916F6 2 13580 13620
python tools\模拟器内存回放器.py <rec> writers 0x800916F7 1 13580 13620
```

Results:

```text
430..640:      0 writers for 0x800916F6, 0 writers for 0x800916F7
12280..12340: 0 writers for 0x800916F6, 0 writers for 0x800916F7
13580..13620: 0 writers for 0x800916F6, 0 writers for 0x800916F7
```

Readbacks:

```text
frame 454:   0x800916F6 = 0
frame 620:   0x800916F6 = 0
frame 12319: 0x800916F6 = 0
frame 13609: 0x800916F6 = 0
```

## Consumer Gate

The useful `80026B94` dynamic hit is frame `620`, with call trace:

```text
8001591C -> 80026B94
```

The static consumer gate is:

```text
80026B94 a1 == 2
80026D70: lh word_800916F6
80026D84/80026D8C: append "\n\n\n~c000StageClear: " when the word is nonzero
80026D9C onward: loop over byte_80092F1D..80092F22
```

This narrows the consumer window, not the writer.

## Decision

- The checked windows do not cover a write to `800916F6`.
- This does not prove the writer is absent from the full recording, because the
  full `writers/xref` scan is still too expensive without an address index.
- No source patch is authorized for `word_800916F6`.
- The next authoritative step remains live watchpoint `write 0x800916F6`, with
  `frame/pc/ra/$ra-8/sp/a0-a3`, write-before/write-after halfword, and
  `800916D0..80091710` dumped at the hit.
- If staying offline, the next useful tool work is an indexed write-log pass or
  a recorder extension that records `ra/sp/a0-a3` with matching write events.
