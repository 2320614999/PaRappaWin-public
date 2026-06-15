# Stage1 compact rail static closure

日期：2026-04-12  
范围：纯静态分析，不含编译、运行或内存回放。本页只收敛 PSX Stage1 compact rail 相关的 7 个主 EXE 地址，以及当前还缺的上游/下游闭环地址。

## 结论先行

- 这 7 个地址并不是一条单线串行链，而是两条互补路径：
  - `0x80023618` 是 practice / 单行 reference shell，用来证明 row0 绝对几何、selector/value 拆分、slot-scale 共享表的消费顺序。
  - `0x80024744` 是 Stage1 steady gameplay 的 live compact painter，用来消费 COMOD1 当前 rail ctx 并真正画出 active compact rail。
- 对 Stage1 native rail 的稳妥静态 owner 顺序，应按“数据生产 -> 选择器 -> 绘制消费”理解：
  1. `0x80024FD0` 更新 Stage1 runtime，并按现有文档结论切 compact painter gate。
  2. `0x801C9094` 生产/刷新 compact ctx 字段，必要时重置两套 wobble/flip bank。
  3. `0x801CB6BC -> 0x800246A8(1)` 先把 Stage1 teacher compact portrait 的 sprite family 选成 `ON_SI`。
  4. `0x801CBFDC -> 0x80024744(ctx)` 再消费当前 compact ctx 并完成 live rail 绘制。
- 所以，`0x80023618` 应视为 Stage1 rail 的“reference shell”，`0x80024744` 才是 steady gameplay 的“active painter”。

## 函数职责表

| 地址 | 当前职责 | 直接调用/被调 | 关键共享状态 |
| --- | --- | --- | --- |
| `0x80023618` | practice / 单行 compact shell。先画固定 chrome，再画两条 portrait/highlight lane，最后扫描 18 格 body stream。它不是 steady Stage1 的 live caller，但能稳定证明 row0 几何和 selector/value 拆分。 | 内部调用 `0x80023F20`、`0x800246A8`、`0x80024600`、`0x80024418`；由 `sub_8001E750(ev=16)` 间接分发。 | `ctx+0x8C`/`+0x9E` 两条 compact portrait lane；`ctx+0x94` 的 18 格 body stream；`ctx+0x1C` page/state；`ctx+0x48` 选择态；语言表 `word_800916D8`。 |
| `0x80023F20` | wobble/scale updater A。推进 bank-A 相位计数，把结果写回共享 slot-scale 表。 | 被 `0x80023618` 和 `0x80024744` 的 lane0 路径调用。 | 写 `word_80087668[]` / `word_800876B0[]`；内部状态为 `dword_80087704[]`、`unk_800876F8[]`、`unk_800876FC[]`、`unk_80087700[]`。 |
| `0x80024114` | wobble/scale updater B。和 `0x80023F20` 输出同一组共享 slot-scale 表，但使用另一套本地 bank。 | 被 `0x80024744` 的 lane1 路径调用。 | 仍写 `word_80087668[]` / `word_800876B0[]`；内部状态为 `dword_80087944[]`、`unk_80087938[]`、`unk_8008793C[]`、`unk_80087940[]`。 |
| `0x80024418` | compact note/body sprite emitter。把传入 `(x,y,slot,type)` 转成 center-anchor sprite draw，并按 slot ordinal 读取共享缩放表。 | 被 `0x80023618` 和 `0x80024744` 调用；内部落到 `0x8001C804`。 | `dword_800540BC[type]` 图标模板表；`word_80087668[slot]` / `word_800876B0[slot]` 共享缩放表。 |
| `0x80024600` | moving portrait/highlight emitter。维护“同值持续帧数 -> hold frame”，再按 `base + 15*value + 4*hold` 出图。 | 被 `0x80023618` 调用；内部落到 `0x8001C550`。 | `gp+0x130` / `gp+0x134` 的本地 hold cache；`gp+0x31C` 当前 sprite 模板指针。 |
| `0x800246A8` | compact portrait selector。只负责按 `sel` 选 sprite family，并写入 `gp+0x31C`。 | 被 `0x80023618` 调用；也被 COMOD1 `0x801CB6BC` 调用。 | `sel=4 -> KT_SI`，`sel=0 -> PA_SI`，`sel=1 -> ON_SI`；输出到 `gp+0x31C`。 |
| `0x80024744` | live multi-row compact painter。按 row 循环消费当前 compact ctx：portrait lane0、portrait lane1、18 格 body stream，以及 teacher-side large/small threshold markers。 | 被 COMOD1 `0x801CBFDC` 调用；内部调用 `0x80023F20`、`0x80024114`、`0x80024418`、`0x8001C550`。 | `ctx+0x8A` row count；`ctx+0x8C` row0 raw；`ctx+0x94` row0 18 格 stream pointer；`ctx+0x9E` second compact lane；`ctx+122` painter gate；large/small threshold 表 `dword_800540E8[]` / `dword_80054100[]`。 |

## 静态调用顺序

### 1. reference shell

`0x80023618` 内部顺序非常稳定：

1. 先画固定 chrome / 文本。
2. 若 `ctx+140 >= 0`：
   - `0x80023F20(ctx+140)`
   - `0x800246A8(4)`
   - `0x80024600(0, 16, 93, 3, ctx+140)`
3. 若 `ctx+158 >= 0`：
   - `0x80023F20(ctx+158)`
   - `0x800246A8(0)`
   - `0x80024600(0, 16, 93, 3, ctx+158)`
4. 再扫描 `ctx+148` 指向的 18 格 body stream，并对 `1..8` 调 `0x80024418`。

这条路径给 Stage1 提供了 3 个稳定 reference fact：

- row0 note 几何是绝对屏幕坐标，不需要再加额外 root Y。
- portrait/highlight 的语义是 `selector (0x800246A8) + value (0x80024600)` 两段式。
- `0x80023F20` 会先写共享 `word_80087668/B0`，后续 `0x80024418` 才按 slot 读走。

### 2. Stage1 producer / consumer closure

当前仓库已有的 docs 和 Ghidra dump 足够把 Stage1 steady compact rail 压成下面这条 owner 链：

1. `0x80024FD0`
   - Stage1 主 runtime dispatcher。
   - 现有文档已把 compact painter gate 的 steady writer 闭到它。
2. `0x801C9094`
   - 生产 compact painter 当前要消费的 ctx 字段。
   - 已确认会写 `ctx+0x8C`、`ctx+0x94`、`ctx+0x9E`、`ctx+0xA4` 等，并在路径切换时调用 `0x80024390` / `0x80024308` 重置两套 wobble bank。
   - 当前静态新结论：
     - active gameplay event 先走 `g_PrStageEventStreamFlag` descriptor，direct compact bit-test (`flags04 & 0x800` / `flags04 & 0x100000`) 都发生在这条主 stream path 上。
     - `LABEL_163` 之后的 `g_PrStageEventStreamId` 分支目前只证明是 branch/follow-up `ApplyScriptEvent + SetText`，没有同拍 direct compact bootstrap 证据。
3. `0x801CB6BC -> 0x800246A8(1)`
   - 提前把 Stage1 teacher compact portrait 的 sprite family 选成 `ON_SI`。
   - 这一步解释了为什么 Stage1 不能继续套 practice 的 `KT_SI`。
4. `0x801CBFDC -> 0x80024744(ctx)`
   - render wrapper 最终把当前 compact ctx 交给 live painter。
5. `0x80024744`
   - lane0 走 `0x80023F20`
   - lane1 走 `0x80024114`
   - note/body 走 `0x80024418`
   - moving portrait / threshold marker 走 `0x8001C550`

注意：

- `0x80024744` 里的 teacher threshold marker 比较用的是 raw compact 值阈值，不吃 portrait hold。
- `0x80023F20` 和 `0x80024114` 虽然 bank 分离，但最终都回写同一份 `word_80087668/B0`；谁后写，谁覆盖该 slot 的最终 scale。
- `0x80023618` 的 row0 参考壳证明了“同一份共享 scale 表被后续 `0x80024418` 按 slot ordinal 消费”的模式，Win 侧现在也按这个顺序对齐。

## 关键共享状态表

| 状态/表 | 写入者 | 消费者 | 当前语义 |
| --- | --- | --- | --- |
| `ctx+0x8A` | `0x801C9094` | `0x80024744` | compact row count。 |
| `ctx+0x8C` / `ctx+0x94` | `0x801C9094` | `0x80024744`；`0x80023618` 用同形 row0 reference | lane0 raw compact 值 + row0 18-slot body stream pointer。 |
| `ctx+0x9E` / `ctx+0xA4` | `0x801C9094` | `0x80024744`；`0x80023618` 用 practice 对应 lane | lane1 raw compact 值 + 对应 stream pointer。 |
| `word_80087668[]` / `word_800876B0[]` | `0x80023F20`、`0x80024114` | `0x80024418` | 共享 slot-scale / wobble 输出表。 |
| `dword_80087704[]` + `unk_800876F8/FC/700[]` | `0x80023F20` | `0x80023F20` 自己；间接影响 `0x80024418` | bank-A 相位/速度/计数。 |
| `dword_80087944[]` + `unk_80087938/3C/40[]` | `0x80024114` | `0x80024114` 自己；间接影响 `0x80024418` | bank-B 相位/速度/计数。 |
| `gp+0x130/+0x134` | `0x80024600` | `0x80024600` | 单行 portrait/highlight 的 hold cache。 |
| `gp+0x138/+0x13C` | `0x80024744` lane1 本地逻辑 | `0x80024744` | live multi-row 第二条 portrait lane 的 hold cache。 |
| `gp+0x31C` | `0x800246A8` / COMOD1 `0x801CB6BC` | `0x80024600` / `0x80024744` portrait draw | 当前 compact portrait sprite family。 |
| `dword_800540BC[]` | 静态模板表 | `0x80024418` | note/body 图标模板。 |
| `dword_800540E8[]` / `dword_80054100[]` | 静态阈值表 | `0x80024744` | teacher-side large/small marker 阈值。 |

## 还缺的上游 / 下游函数地址

### 必补

- `PrStageRunner_ApplyScriptEvent`
  - `0x801C9094` 的 direct compact bit-test 已经够清楚了，下一步真正该补的是主 gameplay record 的字段语义：
    - `flags04`
    - `byte1D / 0x1E / 0x1F / 0x20`
  - 这一步会直接决定“哪条主 stream1 record 点燃 teacher/student compact state machine”。
- `0x801CB6BC`
  - 这是 `0x800246A8(1)` 的 Stage1 上游 owner，直接决定 teacher compact portrait 为什么是 `ON_SI`。
- `0x801CBFDC`
  - live render wrapper；它是 `0x80024744` 的直接 COMOD1 caller。
- `0x80024308`
  - bank-A reset helper，能把 `0x80023F20` 的初始状态一次性讲清。
- `0x80024390`
  - bank-B reset helper，和 `0x80024114` 成对。

### 次轮建议补齐

- `0x801C7A60`
  - Scene1 steady gameplay 主循环 owner；用于把 `0x80024FD0`、`0x801C9094`、render path 的时序关系一次性压实。
- `0x80024FD0`
  - 当前 `docs/stage1_decomp` 已有，但如果要把 compact painter gate 和 Stage1 runtime 彻底合并进一页分析，仍值得一起并读。
- `0x8001C550`
  - moving portrait / threshold marker 的最终 sprite submitter。
- `0x8001C804`
  - compact body note 的最终 sprite submitter。
- `0x801C858C`
  - `0x801C9094` 的上游 stream dispatch helper；如果要继续追“是谁决定下一组 compact row/stream”的来源，这个地址很关键。

### 当前 `docs/stage1_decomp` 的缺口

- 相关缓存里目前只有 `0x80024FD0`。
- 本页 7 个核心地址以及上面的 `0x801C9094 / 0x801CB6BC / 0x801CBFDC / 0x80024308 / 0x80024390` 仍未进入该 decomp bundle。

## 本页使用的现成证据

- `docs/memory.md`
  - `9185..9193`：`0x80023618` portrait/highlight 调用链
  - `9425..9440`：单行 reference shell 的 row0 几何与共享 scale 表
  - `20488..20500`：Stage1 `ON_SI` vs practice `KT_SI`
  - `25547..25576`：`0x801C9094 -> 0x801CBFDC -> 0x80024744` 的 active compact field map
  - `25693..25696`：`0x80023618` 的后写覆盖语义
  - `26105..26129`：当前 accepted 的 active compact authority 结论
- `src/pr/pr_ui_overlay.cpp`
  - `1840..1878`：row0 几何和 center-anchor 结论
  - `1953..1957`：共享 flip table 的覆盖顺序
  - `3501..3502`：`0x80024418` 的 center-anchor X
- `src/pr/pr_scenes.cpp`
  - `25477..25480`：slot0/slot1 对齐 `0x80023618`
  - `25604..25606`：Stage1 slot0 portrait family = `ON_SI`
- `temp/ghidra_reports/FUN_801c7a60_dump.json`
  - `4408..4422`：`0x80024FD0 -> 0x801C9094`
  - `2392..2394`：render path earlier helper -> `0x801CB6BC`
  - `3244..3252` 与 `3508..3516`：steady render path -> `0x801CBFDC`
- `temp/ghidra_reports/FUN_801cbfdc_full_dump.json`
  - `2218..2226`：`0x801CB6BC -> 0x800246A8(1)`
  - `8686..8694`：`0x801CBFDC -> 0x80024744`
- `temp/ghidra_reports/FUN_801c9094_writes_dump.json`
  - `1228..1302`：`0x801C9094` 写 `ctx+0x8C/+0x94/+0x9E/+0xA4` 并调 `0x80024390/0x80024308`

## 2026-04-12 Active Painter Decompile Refresh

- 这轮重新导出了主 EXE / overlay 关键伪 C：
  - [batch_dump_manifest_20260412_193915.json](../tools/auto_port/output/stage1_overlay_extract_20260412_2015/batch_dump_manifest_20260412_193915.json)
  - [decompile_801c858c.txt](../tools/auto_port/output/stage1_overlay_extract_20260412_2015/decompile_801c858c.txt)
  - [decompile_801c9094.txt](../tools/auto_port/output/stage1_overlay_extract_20260412_2015/decompile_801c9094.txt)
  - [decompile_801cb6bc.txt](../tools/auto_port/output/stage1_overlay_extract_20260412_2015/decompile_801cb6bc.txt)
  - 主 EXE 当前 IDB 重新看的函数：
    - `0x80024744`
    - `0x80024418`
    - `0x80024600`
    - `0x800246A8`
    - `0x80023F20`
    - `0x80024114`
    - `0x80024308`
    - `0x80024390`
    - `0x8001C550`

### `0x80024744` 的精确职责边界

- `0x80024744(ctx)` 的 row-loop 这次已经能按伪 C 写死：
  - `ctx+0x8A` 决定 row count。
  - 每轮 row 先读：
    - `ctx+0x8C + 2*row` 作为 teacher raw compact value
    - `ctx+0x9E + 2*row` 作为 student raw compact value
    - `ctx+0x94 + 4*row` 作为 18-slot body stream pointer
  - teacher lane：
    - 先 `sub_80023F20(raw + rowOffset18)`
    - 再按 `gp+304/308` 维护 hold
    - 若 `raw > 0`，用 `gp+796` 当前 compact portrait family 调 `sub_8001C550(...)`
  - student lane：
    - 先 `sub_80024114(raw + rowOffset18)`
    - 再按 `gp+312/316` 维护 hold
    - 若 `raw >= 0`，固定用 `dword_8005400C` 调 `sub_8001C550(...)`
  - body/note：
    - 从 `*(ctx+148 + 4*row)` 指向的 18-slot 流里逐格读 `s8 type`
    - `type in 1..8` 时调 `sub_80024418({x,y,slot,type})`
  - teacher-side bar chrome：
    - 只看 teacher raw compact 值，不吃 portrait hold
    - 先算 `teacherProgressX = raw > 0 ? 15*raw + 31 : 0`
    - large marker：
      - 阈值表 `dword_800540E8[4]`
      - lit/unlit 模板分别是 `dword_800540AC / dword_8005409C`
    - small marker：
      - 阈值表 `dword_80054100[14]`
      - lit/unlit 模板分别是 `dword_8005408C / dword_8005407C`
      - 仅当 `row > 0` 时跳过前 2 个小阈值（`2 * (row > 0)`）
    - marker 全都走 `sub_8001C550(..., ot=3)`
  - 2026-04-13 通过 IDA 直接导出的阈值表已经落成 artifact：
    - [psx_stage1_compact_chrome_tables_20260413.json](../artifacts/psx_stage1_compact_chrome_tables_20260413.json)
  - 当前 row0 的 teacher-side threshold 实值是：
    - large = `[56, 116, 176, 236]`
    - small = `[30, 45, 75, 90, 105, 135, 150, 165, 195, 210, 225, 255, 270, 285]`
  - 因为 `teacherProgressX = raw > 0 ? 15*raw + 31 : 0`，所以 row0 下的点亮结果已能直接写死成：
    - `raw=0 -> large 0 / small 0`
    - `raw=1 -> large 0 / small 2`
    - `raw=2 -> large 1 / small 2`
    - `raw=3 -> large 1 / small 3`
    - `raw=4 -> large 1 / small 4`
    - `raw=5 -> large 1 / small 5`
    - `raw=6 -> large 2 / small 5`
    - `raw=7 -> large 2 / small 6`
    - `raw=8 -> large 2 / small 7`
    - `raw=9 -> large 2 / small 8`
  - 这层规则也已经被接进离线 chrome 模型：
    - [tools/stage1_compact_chrome_model.py](../tools/stage1_compact_chrome_model.py)
    - [psx_stage1_compact_chrome_model_20260413.json](../artifacts/psx_stage1_compact_chrome_model_20260413.json)
  - 所以后续 Win 侧缺失的 large/small bar light，不需要再靠视觉猜：
    - 直接按 `teacher raw compact value -> teacherProgressX -> threshold compare` 落地

### `flip/wobble` 不是独立第三套 authority

- `0x80023F20` 和 `0x80024114` 的新伪 C 说明：
  - 两者都只负责推进各自 bank 的本地 phase/counter；
  - 最终统一回写：
    - `word_80087668[]`
    - `word_800876B0[]`
  - `0x80024418` 再按 `slot ordinal` 读取这两张共享表。
- 所以当前 Stage1 compact row 的“note flip / wobble”责任边界是：
  - updater：
    - `0x80023F20`（teacher-side bank-A）
    - `0x80024114`（student-side bank-B）
  - consumer：
    - `0x80024418`
  - 不是另外某条 COMOD1 私有 render 壳。
- 2026-04-13 这层又被补成 exact-sequence artifact：
  - [psx_stage1_compact_wobble_sequences_20260413.json](../artifacts/psx_stage1_compact_wobble_sequences_20260413.json)
  - reset 初值两边都一样：
    - `counter=0`
    - `sin_phase=0`
    - `linear_acc=2048`
    - `linear_vel=2048`
  - bank-A (`0x80023F20`) 的 24-step 节奏：
    - `step 0..5` = `linear`
    - `step 6..21` = `rsin`
    - `step 22..23` = `rest(4096,4096)`
  - bank-B (`0x80024114`) 的 24-step 节奏：
    - `step 0..4` = `linear`
    - `step 5..21` = `rsin`
    - `step 22..23` = `rest(4096,4096)`
  - 关键 early scales 已可直接写死：
    - A：`6144, 8192, 7168, 6144, 5120, 4096`
    - B：`6144, 8192, 7168, 6144, 5120`
  - `rsin` 采样也直接从 PSX 表导出：
    - `0 -> 0`
    - `256 -> 1567`
    - `512 -> 2896`
    - `768 -> 3784`
    - `1024 -> 4096`
    - `2048 -> 0`
    - `3072 -> -4096`
    - `4096 -> 0`
  - 所以后续 Win 侧的 note wobble/flip 对齐，至少应按这条 exact phase machine 实现，而不是拿宿主侧 `sin()` 近似或凭视觉调参。

### `type(1..8)` -> template / symbol 的静态闭环

- `0x80024418` 的模板表也已经由 IDA 直接导出：
  - [psx_stage1_note_template_table_20260413.json](../artifacts/psx_stage1_note_template_table_20260413.json)
- 当前 `dword_800540BC[type]` 的直接映射是：
  - `1 -> 0x8005405C -> triangle / GUI_SANK`
  - `2 -> 0x8005403C -> circle / GUI_MARU`
  - `3 -> 0x8005404C -> cross / GUI_PEKE`
  - `4 -> 0x8005406C -> square / GUI_SIKA`
  - `5/6 -> 0x8005401C -> left prompt / GUI_L`
  - `7/8 -> 0x8005402C -> right prompt / GUI_R`
- 当前 active Stage1 compact row table 的 note/body symbol 摘要也已落成：
  - [tools/stage1_compact_note_symbol_model.py](../tools/stage1_compact_note_symbol_model.py)
  - [psx_stage1_compact_note_symbol_model_20260413.json](../artifacts/psx_stage1_compact_note_symbol_model_20260413.json)
- 它确认了两条直接影响 Win 对齐的事实：
  - 当前 row table 里实际出现的 type 只有 `1/2/3/4/5/7`
  - `6/8` 只在模板表里作为 `5/7` 的复用 code 存在，当前 active Stage1 rows 未观测到
- per-selector 例子：
  - `selector=1 (short)`：只在 `slot2` 放 `triangle`
  - `selector=19 (full_combo)`：在 `slot2` 放 `triangle`，在 `slot6` 放 `circle`
- 这意味着后续 Stage1 rail 的 body stream direct-port 可以直接按：
  - `row selector -> laneA slot payload -> type code -> template ptr`
  - 不需要再靠 Win 侧 cacheKey 或视觉猜测反推图标类别

### 几何层与多分辨率约束

- 2026-04-13 又把 active compact painter 的几何层单独落成了 artifact：
  - [psx_stage1_compact_geometry_tables_20260413.json](../artifacts/psx_stage1_compact_geometry_tables_20260413.json)
  - [psx_stage1_portrait_templates_20260413.json](../artifacts/psx_stage1_portrait_templates_20260413.json)
  - [tools/stage1_compact_geometry_model.py](../tools/stage1_compact_geometry_model.py)
  - [psx_stage1_compact_geometry_model_20260413.json](../artifacts/psx_stage1_compact_geometry_model_20260413.json)
- 当前 active painter 的最小几何规则已经可直接写死：
  - note center：
    - `x = 32 + 15*slot`
    - `y = 24 + 20*row`
  - teacher portrait top-left：
    - `x = 15*raw + 26 + 4*hold`
    - `y = 18 + 20*row`
  - student portrait top-left：
    - `x = 15*raw + 26 + 4*hold`
    - `y = 16 + 20*row`
  - large marker center：
    - `x = threshold_x + 6`
    - `y = row_large_top_left_y + 6`
  - small marker center：
    - `x = threshold_x + 4`
    - `y = row_small_top_left_y + 4`
- 从主 EXE 直接导出的 row-local Y：
  - large marker rows = `[18, 38]`
  - small marker rows = `[22, 41]`
  - 这也说明 row1 small marker center 是 `45`，不是简单照 row0 `+20` 套公式
- 直接几何样本：
  - row0 note0 center = `(32, 24)`
  - row1 note0 center = `(32, 44)`
  - row0 teacher portrait `raw=1 hold=0` center = `(49, 26)`
  - row0 student portrait `raw=1 hold=0` center = `(49, 24)`
  - row0 first large marker center = `(62, 24)`
  - row1 first visible small marker center = `(79, 45)`（对应 threshold index `2`）
- 这套 geometry model 还会同时输出：
  - PSX 原始像素坐标
  - `320x240` 归一化坐标
- 后续 Win 侧多分辨率适配的规则也在 artifact 里写死了：
  - 保持 PSX 几何不变
  - 只通过统一 viewport scaling 把 `320x240` 坐标映射到目标分辨率
  - 不允许为了高分辨率去改任何 PSX 坐标语义

### Draw-plan / 图层顺序闭环

- 2026-04-13 又把 Stage1 compact rail 的 draw-plan 单独落成了：
  - [tools/stage1_direct_compact_draw_plan.py](../tools/stage1_direct_compact_draw_plan.py)
  - [psx_stage1_direct_compact_draw_plan_20260413.json](../artifacts/psx_stage1_direct_compact_draw_plan_20260413.json)
- 当前把两套顺序明确分开写死：
  - call sequence / submission order：
    - `teacher_portrait`
    - `student_portrait`
    - `marker`
    - `note`
  - visible order / OT semantics：
    - portrait = `ot 0`，最前
    - note = `ot 1`，中间
    - marker = `ot 3`，最后
- 这正好对上已有静态边界：
  - `0x8001C550 -> GsSortFastSprite(..., ot=0)`：portrait
  - `0x80024418 -> 0x8001C804 -> GsSortSprite(..., pri=1)`：note/body
  - `0x8001C550(..., ot=3)`：large/small marker
- draw-plan artifact 里每个 draw item 现在都显式带：
  - `submission_stage`
  - `submit_seq`
  - `visible_order_front_to_back`
  - `ot_bucket`
  - `sort_bucket`
  - `anchor_mode`
  - `template_ptr_hex`
  - `source_ref`
  - PSX / normalized geometry
- 当前 `scale` 字段边界也已明确：
  - portrait / marker = 固定 `4096`
  - note/body 暂不伪造数值，先只通过
    - `scale_source.kind = shared_slot_scale_table`
    - `slot_ordinal`
    - `word_80087668 / word_800876B0`
    回指 PSX 共享缩放表
- 所以后续 Win 侧若出现“调用顺序看起来对了但遮挡错了”，不应再回头怀疑 authority；直接按这份 draw-plan 的 OT 语义对齐即可。

### Reset Helpers 的真实作用

- `0x80024308` / `0x80024390` 的新伪 C 已经钉住：
  - 两者都会把 36 个 slot 的：
    - phase / delta / counter
    - `word_80087668[] / word_800876B0[]`
    统一复位到初值。
  - 因此 `0x801C9094` 里调用这两个 helper 的意义，不是“切个 UI 开关”，而是显式重置 compact wobble/scale bank。
  - 其中 `teacherBodyStream94[0] == -2` 这类 sentinel 会先把 student 的 `9E/A0/A2` 归位，再走 `0x80024390` 的 student wobble bank reset pulse；`0x80024390` 本身仍只是重置 wobble/scale bank。

### Overlay 侧最小 owner 补充

- `0x801CB6BC`
  - init 期间明确调 `0x800246A8(1)`，把 Stage1 teacher compact portrait family 选成 `ON_SI`。
- `0x801C858C`
  - 这轮新 dump 只证明它会按 `a2` 选 `unk_801D2D64 + 12*a2` 并清其第三个 dword，同时把 `ctx->tick96` 写到 `dword_801D3048`；
  - 当前还不能把它提升成 later-cycle compact writer，本轮只保留为上游 helper。

### Win-only gate 清理

- 2026-05-22 复核 `801C9094` / `80024744` 后，Win 侧不再用
  `compactProducer801C9094.active` 作为 teacher tick 或 live painter valid
  条件。
- 对齐边界：
  - `801C9094` compact tick 只看 `ctx&8` 与 lane state；
  - `80024744` 可见输入只看 painter gate 与 `ctx+0x8A` row count；
  - `producer.active` 只保留为 Win 调试/来源状态，不参与 PSX 没有的运行门。
