# Stage1 writer page pointer 14BDC -> 14614 facts - 2026-05-16

Scope: narrow PSX recorder facts for the Stage1 accepted page-record writer
page owner. This only closes the `14BDC` current-page-pointer authority used by
`14614`; it does not authorize renderer, marker, lifecycle, `v22`, row-table,
input-timing, or harness patches.

Recording:

`E:\game\PSgame\parappa the rapper\duckstation\标题画面-menu-选关选stage1-good通关-保存记忆卡（过场动画均为跳过）.psxrec`

## Commands

```text
python .\tools\模拟器内存回放器.py "<recording>" read 3961 0x8006EA5C 4
python .\tools\模拟器内存回放器.py "<recording>" read 3959 0x8006EA5C 4
python .\tools\模拟器内存回放器.py "<recording>" read 3865 0x8006EA5C 4
python .\tools\模拟器内存回放器.py "<recording>" read 3995 0x8006EA5C 4
python .\tools\模拟器内存回放器.py "<recording>" changes 0x8006EA5C 4 3800 4000
python .\tools\模拟器内存回放器.py "<recording>" writers 0x8006EA5C 4 3600 4000
python .\tools\模拟器内存回放器.py "<recording>" writers 0x80092988 4 3900 3970
```

`0x8006EA5C` is `gp+0x1C` for this hot path (`gp=0x8006EA40` in the
recorder frame context).

## Facts

| Frame | Address | Value | Meaning |
|---:|---|---:|---|
| `3865` | `0x8006EA5C[4]` | `0x80092D90` | current writer page pointer before the later page12 target |
| `3877` | `0x8006EA5C[4]` | `0x80092910` | `14BDC` has selected page12 ring base |
| `3959` | `0x8006EA5C[4]` | `0x80092910` | pointer remains page12 before accepted write |
| `3961` | `0x8006EA5C[4]` | `0x80092910` | `14614` recorded split writes through page12 pointer |
| `3995` | `0x8006EA5C[4]` | `0x80092910` | row3 resolution still sees page12 history |

Writer hits for the pointer:

```text
Frame 3617 PC=0x80014C04 0x80092A90 -> 0x80092C10
Frame 3747 PC=0x80014C04 0x80092C10 -> 0x80092D90
Frame 3877 PC=0x80014C04 0x80092D90 -> 0x80092910
```

At the decisive accepted input frame:

```text
Frame 3961 0x80092988[4] PC=0x80024B54 0 -> 0x20
```

Frame `3877` context:

```text
ctx+0x0C = 4599
ctx+0x38 = 12
dword_8008ECE8 = 375
gp+0x1C = 0x80092910
```

## Conclusion

The PSX path is:

```text
80014BDC / 80014C04 writes gp+0x1C = 0x80092910
later 80014614 reads gp+0x1C
80024B54 writes page12 slot10 at 0x80092988
```

So the remaining Win blocker is not writer-page routing. The next code target
stays on accepted input timing/source so a fresh non-replay `0x20` reaches
`14614` around `ctx+0x0C=4846`; do not fix this by changing page routing,
`v22`, row tables, renderer, or lifecycle.
