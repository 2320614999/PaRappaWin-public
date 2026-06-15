# ChatGPT Web Pro Review Result - 2026-05-29

Conversation URL:

```text
https://chatgpt.com/c/6a187834-65f8-83aa-bdce-c9675d763209
```

Branch reviewed:

```text
codex/stage1-blackblock-pro-handoff-20260529
commit 5d75342d
```

## Decision

```text
no safe patch
```

Pro's decision matches the local evidence boundary:

- The reverted GTE/SXY saturation attempt did not remove the black block.
- Current Win submit membership is:
  `ON, PA, FLOOR, TAMA, TOUROU, AUX_7FF8, KAGE`.
- That membership matches the current `801CB190` pseudo-C for the
  `UIList_Block` branch plus final `KAGE` submit.
- Existing `.psxrec` files never enter `UIList_Block != 0`, so they cannot
  authorize a same-path fix.

## Recommended Next Action

Next safe action is live PSX/GDB capture, with IDA export only as a decoder aid.
Do not make a renderer/sort/range-membership patch from the current evidence.

## Minimum Facts To Capture

### 1. Prove Whether Real PSX Reaches The Same Path

Capture several raised-camera GOOD->COOL frames from PSX with these globals:

```text
0x801DB340 = g_PrStageRunner_UIList_Block
0x801DB33C = g_PrStageRunner_UIRender_Block
0x801DB344 = g_PrStageRunner_UIRender_Block2
0x801D3034 = dword_801D3034
0x801D302C = dword_801D302C
0x8006EDA8 = current draw/work slot
```

If `0x801DB340` never becomes nonzero on real PSX, the forced Win path is not
patch authority.

### 2. Break At `801CB190` And Every `8001B084` Call

For each `8001B084` call, record:

```text
$ra
$a0
$a1
$a2
$a3
5 words at $a2 before the call
5 words at $a2 after the call
*(u32*)0x8006EDA8
```

Decode as:

```text
desc start = $a0
desc count = $a1
worklist pointer/base = $a2
depth / OT shift argument = $a3
current slot = *(u32*)0x8006EDA8
```

Specifically confirm presence/absence and order for:

```text
0x801D98F0 ON
0x801DB2D8 PA
0x801D8038 FLOOR
0x801D98D0 TAMA
0x801D7F68 TOUROU
0x801D7FF8, 0x801D8008, 0x801D8018 AUX range
0x801D8028 KAGE
```

Also confirm whether these are absent on the same path:

```text
0x801D7F78 TENNJOU
0x801D7F88 KABE_R
0x801D7F98 KABE_L
0x801D7FA8 KABE_B
0x801D7FB8 KAMON_L
0x801D7FC8 KAMON_R
```

### 3. Break At `800428B0`

Prioritize relevant descs, especially `KAGE` at `0x801D8028`.

For every primitive accepted or skipped, capture:

```text
raw entry registers
desc memory, at least 16 bytes
desc attr
coord node pointer
object/TMD token
model/object/primitive index
raw TMD packet offset
primitive mode, flag, stride, quad/tri
selected GsTMDfast handler/table slot/target
input vertices and combined matrix
RTPT projected SXY/SZ values
NCLIP/MAC0 result
AVSZ/OTZ result
GTE FLAG
packet address, word count, and full GPU packet words
```

The current Win gap is exactly in this area: projection rejection, NCLIP,
OTZ/OT bucket, packet construction, and worklist insertion.

### 4. Capture OT / Link Behavior

Record:

```text
OT slot address
old OT head
packet link word
new OT head
packet address
final linked-list traversal order
```

Prioritize worklists:

```text
0x801D3244 path used by TOUROU/AUX in the UIList_Block branch
0x801D526C path used by KAGE
any other $a2 observed from 0x8001B084
```

### 5. Capture Same-Frame Camera / Projection Facts

Break or capture around:

```text
0x80040FA0 / GsSetRefView2
0x80040C74 / projection setup
matrix at/around 0x80092880 after view setup
```

Record:

```text
camera pos
camera target
twist
projection distance
screen center
final 3x4 view matrix used before 0x800428B0
```

### 6. IDA Export If Needed

Use IDA export as decoder support, not as permission for a visual patch.
Minimum exports:

```text
0x8001B084 pseudo-C + assembly
0x800428B0 pseudo-C + assembly
reachable GsTMDfast handlers
dispatch table around 0x8008EDD8
flag-4 / special tables around 0x8008EED8 and 0x8008EEE4
any OT-link helper reached from 0x800428B0
```

The export should identify register/stack argument layout and packet/OT write
sites.
