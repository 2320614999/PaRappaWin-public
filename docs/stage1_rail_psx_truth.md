# Stage1 rail PSX truth

日期：2026-04-12  
范围：只收敛 Stage1 compact rail 的 PSX 真值，不讨论当前 Win 侧修法。  
目标：把 first note / first overlap 的 authority、字段偏移、writer PC、以及 active owner 链压成一页，后续 Win 侧只按这份真值实现。

后续真正切回 Win 实现时的输入/输出合同已独立整理到：

- [stage1_direct_port_contract.md](./stage1_direct_port_contract.md)
- [psx_stage1_direct_port_contract_bundle_20260413.json](../artifacts/psx_stage1_direct_port_contract_bundle_20260413.json)

## 证据来源

- 静态 owner / consumer 责任页：
  - [stage1_compact_rail_static.md](./stage1_compact_rail_static.md)
- 主 EXE compact rail 核心伪 C：
  - [current_idb_stage1_rail_core.json](../tools/auto_port/output/stage1_rail_extract_20260412_1455/current_idb_stage1_rail_core.json)
- COMOD1 overlay 批量伪 C：
  - [batch_dump_manifest_20260412_151840.json](../tools/auto_port/output/stage1_rail_overlay_dump_20260412_1545/batch_dump_manifest_20260412_151840.json)
  - [decompile_801c9094.txt](../tools/auto_port/output/stage1_rail_overlay_dump_20260412_1545/decompile_801c9094.txt)
  - [decompile_801cb6bc.txt](../tools/auto_port/output/stage1_rail_overlay_dump_20260412_1545/decompile_801cb6bc.txt)
  - [decompile_801cbfdc.txt](../tools/auto_port/output/stage1_rail_overlay_dump_20260412_1545/decompile_801cbfdc.txt)
- first-note 动态窗口：
  - [psx_stage1_first_note_truth_v2_20260412.json](../artifacts/psx_stage1_first_note_truth_v2_20260412.json)
- second-overlap 交叉窗口：
  - [psx_stage1_second_overlap_truth_20260412.json](../artifacts/psx_stage1_second_overlap_truth_20260412.json)
- later-cycle / stream meta：
  - [psx_stage1_later_cycle_truth_20260412.json](../artifacts/psx_stage1_later_cycle_truth_20260412.json)
  - [psx_stage1_stream_meta_full_20260412.json](../artifacts/psx_stage1_stream_meta_full_20260412.json)
  - [psx_stage1_event_stream_meta_20260412.json](../artifacts/psx_stage1_event_stream_meta_20260412.json)
  - [psx_stage1_event_records_20260412.json](../artifacts/psx_stage1_event_records_20260412.json)
- first-phase stream1 records：
  - [psx_stage1_first_phase_stream_meta_full_20260412.json](../artifacts/psx_stage1_first_phase_stream_meta_full_20260412.json)
  - [psx_stage1_stream1_records_block_20260412.json](../artifacts/psx_stage1_stream1_records_block_20260412.json)
- 早先窗口导出里把第二条 descriptor 指针窗口误记成了 `studentPtrA6@0x801C36E6`。
  - 本页已按回放写日志修正为 `studentDescPtrA4@0x801C36E4`。

## Reproduction

first-note / first-overlap 真值窗口可直接重跑：

```powershell
python .\tools\模拟器内存回放器.py `
  ..\duckstation\stage1二周目_无cool_good评级通关3.psxrec `
  export_windows 1025 1105 artifacts\psx_stage1_first_note_truth_v2_20260412.json `
  --window ctx@0x801C3640:0x148 `
  --window teacherIdx8C@0x801C36CC:2 `
  --window teacherDescPtr94@0x801C36D4:4 `
  --window studentIdx9E@0x801C36DE:2 `
  --window studentDescPtrA4@0x801C36E4:4 `
  --window teacherBodyPtr98@0x801C36D8:4 `
  --window studentBodyPtrA8@0x801C36E8:4 `
  --window desc0@0x801CFA58:0x2C `
  --window desc1@0x801CFA84:0x2C `
  --window desc2@0x801CFAB0:0x2C `
  --window desc3@0x801CFADC:0x2C `
  --format json --step 1
```

second overlap crosscheck 窗口：

```powershell
python .\tools\模拟器内存回放器.py `
  ..\duckstation\stage1二周目_无cool_good评级通关3.psxrec `
  export_windows 1157 1164 artifacts\psx_stage1_second_overlap_truth_20260412.json `
  --window ctx@0x801C3640:0x148 `
  --window teacherIdx8C@0x801C36CC:2 `
  --window teacherEnable90@0x801C36D0:2 `
  --window teacherDescPtr94@0x801C36D4:4 `
  --window studentIdx9E@0x801C36DE:2 `
  --window studentEnableA2@0x801C36E2:2 `
  --window studentDescPtrA4@0x801C36E4:4 `
  --window desc1@0x801CFA84:0x2C `
  --window desc2@0x801CFAB0:0x2C `
  --format json --step 1
```

## Active Owner 链

Stage1 steady gameplay 的 compact rail 按下面这条链看：

1. `0x801C7A60`
   - Scene1 steady owner。
2. `0x80024FD0`
   - Stage1 runtime dispatcher / gate owner。
3. `0x801C9094`
   - compact rail producer；写 `ctx` 里的 teacher/student authority 字段。
4. `0x801CB6BC -> 0x800246A8(1)`
   - 把 teacher compact portrait family 选成 `ON_SI`。
5. `0x801CBFDC -> 0x80024744(ctx)`
   - compact rail live painter。

静态侧细节看 [stage1_compact_rail_static.md](./stage1_compact_rail_static.md)。本页只补动态 authority。

## Truth Core

### `ctx` 字段真值

以 `ctx = 0x801C3640` 为准，first-note 窗口里已钉住这些字段：

| 偏移 | 类型 | 当前语义 | first-note 窗口观察 |
| --- | --- | --- | --- |
| `+0x0C` | `u32` | `tick96` | `1025=2640`，`1083=2809`，`1091=2835`，`1099=2858` |
| `+0x38` | `u32` | `descriptorCursor` | `1025=7`，`1083..1164=8` |
| `+0x40` | `u32` | `currentDescPtr` | `1025=0x801CD434`，`1083..1164=0x801CD44C` |
| `+0x44` | `u32` | `lookaheadDescPtr` | 当前窗口里和 `currentDescPtr` 同步 |
| `+0x7A` | `u16` | `gate7A` | `1025..1164` 内恒为 `1` |
| `+0x8A` | `s16` | compact row count | `1025..1105` 内恒为 `1` |
| `+0x8C` | `s16` | teacher compact cursor | `1025` 为 `1`，之后每 `8` 帧加 `1`，`1099` 变回 `-1` |
| `+0x90` | `u16` | teacher enable | `1025..1091` 为 `1`，`1099` 变 `0` |
| `+0x94` | `u32` | teacher descriptor ptr | `1025` 起就是 `0x801CFA84` |
| `+0x98` | `u32` | teacher body ptr | `1025..1105` 内恒为 `0x801CFA6C` |
| `+0x9E` | `s16` | student compact cursor | `1025..1083` 为 `-1`，`1091` 变 `1`，`1099` 变 `2` |
| `+0xA2` | `u16` | student enable | `1025..1083` 为 `0`，`1091` 起为 `1` |
| `+0xA4` | `u32` | student descriptor ptr | `1025..1083` 为 `0x801CFA58`，`1091` 切到 `0x801CFA84` |
| `+0xA8` | `u32` | student body ptr | `1025..1105` 内恒为 `0x801CFA6C` |

仍未正式命名、但已观测到的 companion halfword：

- `+0x8E`
- `+0xA0`

它们在 first bridge 期间也会变，但当前证据还不足以正式赋名；实现前先不要硬编码语义。

## Descriptor Ring

`0x801CFA58` 起至少有 4 个连续 `0x2C` 字节 descriptor：

| 地址 | 本页简称 | 备注 |
| --- | --- | --- |
| `0x801CFA58` | `desc0` | first note 前 student 停在这里 |
| `0x801CFA84` | `desc1` | teacher 从 first note 开始就指向这里；`1091` student 也切到这里 |
| `0x801CFAB0` | `desc2` | 已导出，但 first-note 窗口里未被 teacher/student 指到 |
| `0x801CFADC` | `desc3` | 已导出，但 first-note 窗口里未被 teacher/student 指到 |

first-note 窗口里，teacher/student 都不是直接指向 `0x801CFAB0 / 0x801CFADC`；因此早先把这两个地址口头叫成 “rowA / rowB” 容易误导。当前更稳妥的叫法是 `desc0..desc3`。

### Producer Reset 基线

`0x801C9094` 的 reset 路径已经被 overlay 伪 C 钉住：

- `teacherIdx(+0x8C)` / `teacherNext(+0x8E)` 先被清成 `-1`
- `teacherDescPtr(+0x94)` 复位到 `0x801CFA58`
- `teacherBodyPtr(+0x98)` 复位到 `0x801CFA6C`
- `teacherEnable(+0x90)` 复位到 `0`
- `studentIdx(+0x9E)` / `studentNext(+0xA0)` 先被清成 `-1`
- `studentDescPtr(+0xA4)` 复位到 `0x801CFA58`
- `studentBodyPtr(+0xA8)` 复位到 `0x801CFA6C`
- `studentEnable(+0xA2)` 复位到 `0`
- 同时调用 `0x80024390()` 和 `0x80024308()` 重置两套 compact bank

这解释了为什么 first note 前，两条 lane 都会从 `desc0/body0` 起步；真正把 teacher 提前切到 `desc1` 的，不是 reset，而是 `1025` 帧的 prewarm writer。

### First-Note / First-Overlap 真值

first note 到 first overlap 的 authority 变化如下：

| 帧 | teacher `+0x8C` | student `+0x9E` | teacher desc `+0x94` | student desc `+0xA4` | 备注 |
| --- | --- | --- | --- | --- | --- |
| `1025` | `1` | `-1` | `0x801CFA84` | `0x801CFA58` | teacher 先起跑；student 仍隐藏 |
| `1035` | `2` | `-1` | same | same | teacher 仅自己推进 |
| `1043` | `3` | `-1` | same | same | teacher 仅自己推进 |
| `1051` | `4` | `-1` | same | same | teacher 仅自己推进 |
| `1059` | `5` | `-1` | same | same | teacher 仅自己推进 |
| `1067` | `6` | `-1` | same | same | teacher 仅自己推进 |
| `1075` | `7` | `-1` | same | same | teacher 仅自己推进 |
| `1083` | `8` | `-1` | same | same | teacher tail |
| `1091` | `9` | `1` | `0x801CFA84` | `0x801CFA84` | first overlap；student 首次 materialize |
| `1099` | `-1` | `2` | `0x801CFA84` | `0x801CFA84` | teacher hidden；student 接手继续走 |

直接结论：

- 第一个音符不是 student 先跑，teacher 才是 authority。
- student 不是从 first note 一开始就显示；它直到 `1091` 才 materialize。
- teacher 并不是从末尾起跑；`1025` 时它已经是 `cursor=1`。
- first bridge 的真实顺序是 `teacher 1..9 -> student 1..2...`，不是两个头像同时或错位各占两小节。

### Second-Overlap Crosscheck

`1157..1164` 这一窗是更强的 handoff 验证：

| 帧 | teacher | student | teacher enable | student enable | teacher ptr | student ptr | 备注 |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `1157..1164` | `1` | `9` | `1` | `1` | `0x801CFAB0` | `0x801CFA84` | teacher 已切到 `desc2`，student 仍留在 `desc1` |

这条交叉窗口很重要，因为它说明：

- teacher/student 不是一直共用同一 descriptor；
- 后续 handoff 的真实模式是 “teacher = next lane / student = current lane”；
- 如果 Win 侧只是把 first bridge 画对，但 second overlap 没过，仍然不算对齐。

## Writer PC 责任表

first-note 窗口里，authority 写入已通过回放写日志钉死：

| 帧 | PC | 写入 | 责任 |
| --- | --- | --- | --- |
| `1025` | `0x801C99C4` | `0x801C36CC: -1 -> 0` | teacher prewarm |
| `1025` | `0x801C99EC` | `0x801C36D4: 0x801CFA58 -> 0x801CFA84` | teacher descriptor 切到 `desc1` |
| `1025` | `0x801C9CA0` | `0x801C36CC: 0 -> 1` | teacher seed / first visible |
| `1035..1091` | `0x801C933C` | `0x801C36CC: n -> n+1` | teacher steady step |
| `1091` | `0x801C9CC4` | `0x801C36DE: -1 -> 0` | student prewarm |
| `1091` | `0x801C9CE0` | `0x801C36E4: 0x801CFA58 -> 0x801CFA84` | student descriptor 切到 `desc1` |
| `1091` | `0x801C9E70` | `0x801C36DE: 0 -> 1` | student seed / first materialize |
| `1099` | `0x801C9334` | `0x801C36CC: 9 -> -1` | teacher hide |
| `1099` | `0x801C947C` | `0x801C36DE: 1 -> 2` | student steady step |

补充：

- `1099` 的 call trace 还能看到 `0x801C9334 -> 0x80024308`，说明 teacher hide 同时会触发 compact bank reset helper。
- 上面这些 writer 全部都发生在 `0x801C9094` 这条 producer 帧更新路径内。

## Alignment Anchors

Win 侧自动对齐时，建议只把下面这些当 anchor，不要混成 PSX 真值本体：

- coarse marker：
  - [summary.txt](../artifacts/stage1_rail_probe_runtime/20260412_144737_stage1_rail_overlap_runtime/summary.txt)
- exact capture sidecar：
  - [q442_stage1ovl_rail.txt](../artifacts/stage1_rail_probe_runtime/20260412_090914_stage1_rail_first_bridge_exact_capture/q442_stage1ovl_rail.txt)
  - [q448_stage1ovl_rail.txt](../artifacts/stage1_rail_probe_runtime/20260412_090914_stage1_rail_first_bridge_exact_capture/q448_stage1ovl_rail.txt)
  - [q454_stage1ovl_rail.txt](../artifacts/stage1_rail_probe_runtime/20260412_090914_stage1_rail_first_bridge_exact_capture/q454_stage1ovl_rail.txt)

这些文件只适合做 Win consumer/render 对齐入口，不是 PSX authority。

## 30/60 与步进节奏

在这个 first-note 窗口里，compact cursor 的 authority 步进是固定的 `8` 帧一档：

- `1025 -> 1035 -> 1043 -> 1051 -> ... -> 1091 -> 1099`

因此：

- Win 侧如果看到头像“一格一格走”，不能直接改 authority 节奏。
- 真正该做的是把 render smoothing 和 authority 拆开：
  - authority 仍按 PSX 逻辑帧推进；
  - 视觉上的连续滚动只能是 render-only 插值，不能把 consumer 提前切到下一格。

## 当前能直接约束 Win 对齐的规则

- teacher portrait 必须先于 student materialize。
- student 在 `1091` 前不能提前显示。
- first note 期间 teacher/student 的 descriptor handoff 是共享 `desc1`，不是各自独占不同 slot。
- `ctx+0xA4` 是 student descriptor 的真实 32 位字段；不要再按 `+0xA6` 读。
- 现在 producer 伪 C 已经到位；后续仍然不要补任何“first bridge 视觉壳”，而是直接按 `0x801C9094` + replay 真值改 authority / consumer。

## Notes

- `summary.txt`、截图和肉眼反馈只能当索引，不能当真值。
- first-note / second-overlap 这两份 `export_windows` JSON 才是 authoritative raw truth。
- `0x800493EC:0x20` 这组 XA/timing 窗口值得后续补到规范里，用来直接解 `start_lba` 和 `now+150`，避免再靠 Win anchor 猜节拍。
- 如果后面继续扩窗，优先补：
  - `1018..1087` 的 first-note lead 窗
  - `1088..1105` 的 first-overlap bridge 窗
  - `1350..1460` 的 later-cycle 窗
- 后续若要继续补 writer 细节，优先用 `writers`、`wlog`、`conditional_watch` 和 `trace_value_origin`，不要把只看帧末结果的断点当成 first-writer 真值。

## 2026-04-12 Direct-Port Cut

- Win 侧 direct-port 的最小切线已经收敛：
  - teacher live lane 只接受 `PSX selector -> formal authority`；
  - 不再允许 `legacy latest/next live-seed` 回填 teacher；
  - `Scene1NativeRailConsumerSnapshot.slot0/slot1` 只吃 live teacher/live student compact lane。
- 这条切线的实现目标不是“把当前 consumer 补平”，而是先切掉最主要的 Win 侧 authority 污染源：
  - `TryResolveStage1RailCursorTeacherRec44SourceResolver()` 的 legacy fallback
  - `ApplyStage1RailCursorTeacherAuthority()` 的 local transition fallback
  - `ResolveStage1Scene1NativeRailConsumerSnapshot()` 里 slot0/slot1 对 local preview/icon probe 的主源仲裁
- 新导出的 writer dump 在
  - [stage1_rail_writer_dump_20260412_1630](../tools/auto_port/output/stage1_rail_writer_dump_20260412_1630)
  - 其中 manifest 证明 `0x801C9334/933C/947C/99C4/...` 这些函数内 writer 地址会被批量导伪 C 工具折叠回 `0x801C9094`
  - 所以微 writer 的责任边界继续以 replay `writers` 为准，不以这些重复导出的 `decompile_*.txt` 为准。

## 2026-04-12 Direct-Port Render Cut

- `0x80024744` 的主头像语义继续只认 live compact lane：
  - raw cursor 来自 `ctx+0x8C / +0x9E`
  - 本地推进是 `hold` 累进
  - additive/shadow 不是主头像 authority
- 因此 Win 侧已经把 `renderPortraitSample` 从主头像 source 上摘掉：
  - `renderPortraitSample*` 继续保留给 host 侧 shadow/additive 路径
  - `ApplyStage1Scene1NativeRailConsumerPortraitsToStripState()` 的主头像改回只吃 `livePortraitCursorRaw + local hold`
  - `ResolveStage1Scene1NativeRailConsumerSnapshot()` 里 `preferRenderPortraitSampleOnRenderOnly` 已显式关掉
- 这条切线只收 render authority，不碰 producer cadence / first bridge writer 真值。
- 串行验证：
  - `powershell -ExecutionPolicy Bypass -File .\\build.ps1`
  - `python .\\test_dispatcher.py --stage1-rail-overlap-runtime`
    - [20260412_164314_stage1_rail_overlap_runtime/summary.txt](../artifacts/stage1_rail_probe_runtime/20260412_164314_stage1_rail_overlap_runtime/summary.txt)

## 2026-04-12 First-Note Rail Visibility Cut

- 这轮 direct-port 继续收掉了两条把 first note onset 压后的 Win 可见门：
  - `ShouldSuppressStage1Scene1NativeRailCurrentLaneBodyBeforeIconPhase()`
    - 现在只在 teacher 仍停留在 pre-visible placeholder 时 suppress
    - 一旦 live teacher 已 materialize，且 live teacher 的 `primaryStreamIdentity` 与当前要发布的 `bodyStreamA` 是同一份 visible payload，就不再压掉 first note
  - `RenderStage1GameplayScene1NativeRail()`
    - rail visible/latch 不再绑定 `displayGate.lessonSelector`
    - `lessonSelector` 来自 numeric `topLessonPairLessonId`，不是 PSX compact rail authority
- 直接证据：
  - 改前 capture：
    - [20260412_170232_first_overlap_after_onset_body_unsuppress](../artifacts/stage1_win_teacher_rail_capture/20260412_170232_first_overlap_after_onset_body_unsuppress)
    - 首个 teacher portrait draw 在 `q197`
    - 但首个 rail visible / body 要到 `q387`
  - 改后 capture：
    - [20260412_171442_first_overlap_after_lesson_gate_cut](../artifacts/stage1_win_teacher_rail_capture/20260412_171442_first_overlap_after_lesson_gate_cut)
    - `q196`：
      - `slot0=1`
      - `currentPayloadGridIndex=2`
      - `currentPayloadGridType=1`
      - `nativeRailVisible=1`
      - `nativeRailRenderedBodyCount=1`
      - `nativeRailTeacherPortraitDraw=1`
    - `q444 -> q453 -> q462`：
      - `8/-1 -> 9/1 -> -1/2`
      - first bridge 顺序保持不变
- 串行验证：
  - `powershell -ExecutionPolicy Bypass -File .\\build.ps1`
  - `python .\\test_dispatcher.py --stage1-rail-overlap-runtime`
    - [20260412_171433_stage1_rail_overlap_runtime/summary.txt](../artifacts/stage1_rail_probe_runtime/20260412_171433_stage1_rail_overlap_runtime/summary.txt)

## 2026-04-12 Same-Source Preview Reentry Cut

- 新证据先把 PSX 侧责任边界再次钉死了：
  - `0x10000` stream1 事件链只会落到 `ctx+0xDC/+0xE8`，没有静态证据表明它会直接 materialize `ctx+0x8C/+0x9E`
  - compact rail consumer `0x80023618 / 0x80024744` 也没有“同一 future source family 用更晚 startDueFrame 重开”的 PSX 对应壳
  - 这部分静态结论来自 Linnaeus 的复核，和前面 replay writer truth 一致：Win 的 `sourceDueFrame=2640, startDueFrame=443 -> 509` 是 host preview 链自己的重锚，不是 PSX authority

- Win 侧这轮只收三条直接命中的链，不再碰 event scan：
  - `src/pr/pr_scenes.cpp`
    - `PopulateStage1RailCursorLocalTeacherPreviewProbeMirror()`
      - 当 `resolverImplKind=FutureNextLookaheadSequentialPayloadSymbol`
      - 且仍处在同一 `sourceDueFrame` 的可见 tail（`cursor>=8`）
      - 不允许 next-lookahead 用 later same-source family 直接换掉当前 preview payload
    - `ResolveStage1RailTeacherPortraitPreviewWindowKey()` / `UpdateStage1RailTeacherPortraitCadenceGateMirror()`
      - 在 visible future preview tail 窗口内，teacher preview cadence 改为按 `actor.startDueFrame` 计步
      - 这样同一 `sourceDueFrame=2640` 即使 preview gate phase 在 host 侧抖动，也不会把 teacher 从 `9` 重启回 `1`
    - `TryUpdateStage1RailCursorTeacherFormalAuthority()`
      - 当 student 正处于 `TeacherCurrentTailOverlap`
      - 且 `studentCurrentTeacherPreviewProbe.startDueFrame > localTeacherPreviewProbe.startDueFrame`
      - formal authority 又想退回 `OnPairCandidate` 的 zero-body teacher
      - 直接 suppress 这条 fallback，避免 `latest(0/0)` 在 teacher hide 之后马上把 compact slot0 污染回来

- 关键前后对比：
  - 改前：
    - [20260412_175850_current_lane_body_borrow_cut_v3_no_student_future_promote](../artifacts/stage1_win_teacher_rail_capture/20260412_175850_current_lane_body_borrow_cut_v3_no_student_future_promote)
    - `q504: teacher=8, student=-1`
    - `q512: teacher=9, student=1, localPreview.startDueFrame=509`
    - `q520: teacher=2, student=2`
    - 结论：同一 `2640` source 在 bridge 内被 later-family 重开，teacher/student 双双被拖乱
  - 中间收链后：
    - [20260412_183210_same_source_preview_tail_carry_v4_actor_base_hold](../artifacts/stage1_win_teacher_rail_capture/20260412_183210_same_source_preview_tail_carry_v4_actor_base_hold)
    - `q512: teacher=9, student=1`
    - 但 `q520` 仍会掉进 `latest 0/0` fallback
  - 当前：
    - [20260412_185958_same_source_preview_tail_carry_v8_current_state_onpair_suppress](../artifacts/stage1_win_teacher_rail_capture/20260412_185958_same_source_preview_tail_carry_v8_current_state_onpair_suppress)
    - [20260412_190411_same_source_preview_tail_carry_v9_overlap_then_hide](../artifacts/stage1_win_teacher_rail_capture/20260412_190411_same_source_preview_tail_carry_v9_overlap_then_hide)
    - `q508/q513: teacher=9, student=1`
    - `q516/q521 之后: teacher=-1, student>=3`
    - `teacherSource` 保持 `preview`，不再回到 `latest 0/0`

- runtime raw probe 现在也已经能直接看到主污染链被切掉：
  - [20260412_190657_stage1_rail_overlap_runtime/samples.jsonl](../artifacts/stage1_rail_probe_runtime/20260412_190657_stage1_rail_overlap_runtime/samples.jsonl)
  - 关键样本：
    - `q502/q505: teacher=8, student=-1`
    - `q508/q511/q514: teacher=9, student=1`
    - `q517/q520+: teacher=-1, student=3/4/...`
  - 这说明：
    - `443 -> 509` same-source preview reentry 已被切掉
    - `latest 0/0` fallback 也不再抢占 slot0
  - 当前 runtime summary 仍显示 marker miss：
    - [20260412_190657_stage1_rail_overlap_runtime/summary.txt](../artifacts/stage1_rail_probe_runtime/20260412_190657_stage1_rail_overlap_runtime/summary.txt)
  - 原因不是又回到旧坏链，而是 probe 采样窗口没有正好命中 `hidden/2` 这一格；raw `samples.jsonl` 里已经能看到 `8/-1 -> 9/1 -> hidden/3+`

- 当前剩余问题：
  - first bridge 的 authority 主链已经从“same-source preview reentry + latest fallback”继续收到了正确半拍：
    - [20260412_191824_same_source_preview_tail_carry_v10_no_mismatched_halfcadence](../artifacts/stage1_win_teacher_rail_capture/20260412_191824_same_source_preview_tail_carry_v10_no_mismatched_halfcadence)
    - `q509: teacher=9, student=1`
    - `q517: teacher=-1, student=2`
    - 说明剩余 release 偏移不在 teacher hide，而在 student cadence 起点被错误 backdate
  - 根因与修正：
    - `BuildStage1RailCursorStudentLiveTimingDecision()`
    - `continueTeacherTailOverlapTiming` 分支里的 `hiddenLaterSameSourceCarryVisibleContinuation`
    - 之前会在 later same-source hidden probe 已经切成别的 `record/stream` 时，仍把 student cadence 从 `507` 回拨到 `499`
    - 现在只有 hidden probe 仍和当前 student seed 是同一 `recordIndex/secondaryRecordIndex/visible body payload` 时，才允许这条 half-cadence backdate
    - 因此当前 release 已回到 `9/1 -> hidden/2`
  - runtime harness 也已经同步回归：
    - [20260412_192434_stage1_rail_overlap_runtime/summary.txt](../artifacts/stage1_rail_probe_runtime/20260412_192434_stage1_rail_overlap_runtime/summary.txt)
    - [20260412_192434_stage1_rail_overlap_runtime/summary.json](../artifacts/stage1_rail_probe_runtime/20260412_192434_stage1_rail_overlap_runtime/summary.json)
    - 关键 marker：
      - `q501: 8/-1`
      - `q510: 9/1`
      - `q516: hidden/2`
      - `q564: hidden/8`
      - `q573: 1/9`
      - `q582: 2/hidden`
  - 注意：
    - 这条 runtime harness 还顺手修了旧基线 marker 条件
    - first overlap 不再硬要求 `teacherSource=current-lane`
  - overlap runtime 的 `max_query_frame` 也已扩到 `650`，否则 second overlap 不会被窗口捕获

## 2026-04-12 Later-Cycle Ring Truth

- 这轮不再只看 first note / first overlap，而是把 `1150..1500` 的 later-cycle 真值整窗导出：
  - [psx_stage1_later_cycle_truth_20260412.json](../artifacts/psx_stage1_later_cycle_truth_20260412.json)
- 这份窗口把 compact lane 的 later-cycle 模式彻底钉成了一个稳定 ring：

| 帧 | teacher `+0x8C` | student `+0x9E` | teacher desc `+0x94` | student desc `+0xA4` | 备注 |
| --- | --- | --- | --- | --- | --- |
| `1157` | `1` | `9` | `0x801CFAB0` | `0x801CFA84` | second overlap；teacher 已切到 `desc2` |
| `1165` | `2` | `-1` | `0x801CFAB0` | `0x801CFA84` | student hide；teacher 接手 `desc2` |
| `1215` | `8` | `-1` | `0x801CFAB0` | `0x801CFA84` | teacher tail |
| `1221` | `9` | `1` | `0x801CFAB0` | `0x801CFAB0` | student materialize 到同一 `desc2` |
| `1229` | `-1` | `2` | `0x801CFAB0` | `0x801CFAB0` | teacher hide；student 接手 |
| `1287` | `1` | `9` | `0x801CFADC` | `0x801CFAB0` | 下一轮 overlap；teacher 切到 `desc3` |
| `1351` | `9` | `1` | `0x801CFADC` | `0x801CFADC` | bridge 进入 `desc3` |
| `1417` | `1` | `9` | `0x801CFB08` | `0x801CFADC` | 再下一轮 overlap；teacher 切到 `desc4` |
| `1483` | `9` | `1` | `0x801CFB08` | `0x801CFB08` | bridge 进入 `desc4` |

- 直接结论：
  - later-cycle 不是 first bridge 的特例；compact ring 会继续按 `desc1 -> desc2 -> desc3 -> desc4 -> ...` 推进。
  - 稳定模式始终是：
    - `teacher = next lane`
    - `student = current lane`
  - overlap 帧上，teacher 会先切到下一条 descriptor，student 仍停留在上一条 descriptor；
    到 `teacher=9 / student=1` bridge 帧，student 才切到 teacher 当前那条 descriptor。
  - compact lane 的显示节拍依旧是：
    - `teacher 1..9 -> hidden`
    - `student hidden -> 1..9 -> hidden`
    - 没有“各自占两个小节”的 later-cycle 证据。

### Later-Cycle Writer PCs

- later-cycle 的 writer PC 与 first-note / first-overlap 完全同构：

| 地址 | later-cycle writer | 帧样本 | 责任 |
| --- | --- | --- | --- |
| `0x801C36CC` | `0x801C99C4` | `1157/1287/1417` | teacher prewarm `-1 -> 0` |
| `0x801C36CC` | `0x801C9CA0` | `1157/1287/1417` | teacher seed `0 -> 1` |
| `0x801C36CC` | `0x801C933C` | `1165..1491` | teacher steady step |
| `0x801C36CC` | `0x801C9334` | `1229/1361/1491` | teacher hide `9 -> -1` |
| `0x801C36DE` | `0x801C9CC4` | `1221/1351/1483` | student prewarm `-1 -> 0` |
| `0x801C36DE` | `0x801C9E70` | `1221/1351/1483` | student seed `0 -> 1` |
| `0x801C36DE` | `0x801C947C` | `1157/1229/1351/1491...` | student steady step |
| `0x801C36DE` | `0x801C9474` | `1165/1295/1427` | student hide `9 -> -1` |
| `0x801C36D4` | `0x801C99EC` | `1157/1287/1417` | teacher descriptor 切到 next lane |
| `0x801C36E4` | `0x801C9CE0` | `1221/1351/1483` | student descriptor 切到 teacher 当前 lane |

- 这说明：
  - later-cycle 不是另一套隐藏分支；
  - `0x801C99C4/99EC/9CA0` 仍是 teacher 这条 lane 的 prewarm/seed/descriptor writer；
  - `0x801C9CC4/9CE0/9E70` 仍是 student 这条 lane 的 prewarm/seed/descriptor writer；
  - `0x801C9334/933C/9474/947C` 仍是 steady step/hide writer。

### Producer Descriptor Stream Cadence

- 这轮把 producer 级的 `descriptorCursor/current/lookahead` 也补齐了：
  - `ctx+0x38 @ 0x801C3678`
  - `ctx+0x40 @ 0x801C3680`
  - `ctx+0x44 @ 0x801C3684`
- later-cycle 变化如下：

| 帧 | `descriptorCursor` | `currentDescPtr` | `lookaheadDescPtr` | writer |
| --- | --- | --- | --- | --- |
| `1039` | `7` | `0x801CD434` | `0x801CD44C` | `0x80025104` 先切 lookahead |
| `1043` | `8` | `0x801CD44C` | `0x801CD44C` | `0x80025108/0x8002515C` 切 current |
| `1169` | `8` | `0x801CD44C` | `0x801CD464` | `0x80025104` |
| `1173` | `9` | `0x801CD464` | `0x801CD464` | `0x80025108/0x8002515C` |
| `1299` | `9` | `0x801CD464` | `0x801CD47C` | `0x80025104` |
| `1303` | `10` | `0x801CD47C` | `0x801CD47C` | `0x80025108/0x8002515C` |
| `1431` | `10` | `0x801CD47C` | `0x801CD494` | `0x80025104` |
| `1435` | `11` | `0x801CD494` | `0x801CD494` | `0x80025108/0x8002515C` |

- 直接结论：
  - `lookaheadDescPtr` 会比 `currentDescPtr` 提前 `4` 帧翻页；
  - `descriptorCursor/currentDescPtr` 自身则按 `130` 帧一格推进；
  - 这些 writer 全都落在主 EXE `0x80024FD0` 内：
    - `0x80025104` = lookahead pointer write
    - `0x80025108` = descriptor cursor write
    - `0x8002515C` = current descriptor pointer write
  - 所以后续如果 Win 要直接抠 parser，不需要猜“什么时候翻到下一条 row”；
    可以按 `sub_80024FD0` 这三个 writer 的关系落实现。

### Companion Halfword Recheck

- 早先只说 `ctx+0x8E / +0xA0` 还未赋名，这轮 later-cycle 重查得到一条更稳的负证据：
  - 在 `1025..1500` 的 first-note、second overlap、later-cycle 窗口里：
    - `ctx+0x8E`
    - `ctx+0xA0`
    都保持 `-1`，没有命中任何变化。
- 结论：
  - 当前不要把这两个 halfword 提升成 “next cursor / next descriptor ordinal” 之类语义。
  - 至少在 active Stage1 compact rail 当前已抓到的主窗口里，它们不是决定 later-cycle ring 的关键字段。

### Stream1 Event Grammar

- 这轮继续把 `1000..1170` 的 first-phase 主 stream1 记录和 compact lane 真值对到了一起：
  - [psx_stage1_first_phase_stream_meta_full_20260412.json](../artifacts/psx_stage1_first_phase_stream_meta_full_20260412.json)
  - [psx_stage1_stream1_records_block_20260412.json](../artifacts/psx_stage1_stream1_records_block_20260412.json)
- 关键 cursor / record 序列如下：

| 帧 | stream1 cursor | `flags04` | `byte1D` | `byte1E` | teacher | student | teacher desc | student desc |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| `1000` | `16` | `0x20000` | `0` | `0` | `-1` | `-1` | `desc0` | `desc0` |
| `1009` | `17` | `0x800` | `1` | `0` | `-1` | `-1` | `desc0` | `desc0` |
| `1025` | `18` | `0x30000` | `1` | `0` | `1` | `-1` | `desc1` | `desc0` |
| `1043` | `19` | `0x0` | `1` | `0` | `3` | `-1` | `desc1` | `desc0` |
| `1075` | `20` | `0x100000` | `1` | `0` | `7` | `-1` | `desc1` | `desc0` |
| `1091` | `21` | `0x30000` | `1` | `0` | `9` | `1` | `desc1` | `desc1` |
| `1107` | `22` | `0x20000` | `1` | `0` | `-1` | `3` | `desc1` | `desc1` |
| `1141` | `23` | `0x800` | `2` | `0` | `-1` | `7` | `desc1` | `desc1` |
| `1157` | `24` | `0x30000` | `2` | `0` | `1` | `9` | `desc2` | `desc1` |

- 直接结论：
  - `0x800` record 是 teacher next-lane bootstrap 的点火 record；它会把下一轮的 descriptor selector (`byte1D`) 喂进 `0x801C99EC` 这一簇 writer。
  - `0x100000` record 是 student catch-up phase 的点火 record；真正的 `student -1 -> 0 -> 1` materialize 则落在后续 `0x30000` overlap record 的窗口里。
  - `0x30000` record 是 compact overlap / handoff 的关键 gameplay record：
    - `1025` 让 teacher 首次在 `desc1` materialize
    - `1091` 形成 `teacher=9 / student=1`
    - `1157` 形成 `teacher=1(desc2) / student=9(desc1)`
  - later-cycle 并不是另一套隐藏 stream；它只是 stream1 gameplay records 在 descriptor ring 上继续重复。
  - 同一批 record 里的 HUD bytes 也有稳定模式：
    - `event+0x17(slot0)` 在主 gameplay 记录上长期稳定为 `3`
    - `event+0x18..0x1B(slot1 by mode)` 才是按词组/模式切换的 overlay slot
    - `event+0x1C(slot2)` 在当前 compact rail 主窗口里长期为 `0`
  - 所以后续如果 Win 要切 Stage1 rail 上层 HUD/word shell，`slot0` 应先按稳定常驻层理解，而不是拿来当 compact rail authority。
  - `byte1E(body selector)` 在当前 Stage1 `stream1` 整条事件表里也是全程 `0`：
    - 不只是 `1000..1170` 的 first-phase 窗口
    - 对 `temp/stage1_stream1_events.json` 的全表扫描结果也是 `nonzero_count = 0`
  - 与之对应，`body ptr/cache` 在 active rail 主窗口里基本是常量链：
    - `teacherBodyPtr98 = 0x801CFA6C`
    - `studentBodyPtrA8 = 0x801CFA6C`
    - `dword_801D3050` 只在 `1025` 被 `0x801C9A2C` 初始化到 `0x801CFA6C` 一次，之后 `1000..1600` 内无再次改写
  - 当前直接结论：
    - `byte1D` 是 Stage1 rail 当前真正活跃的 compact selector 轴
    - `byte1E` 虽然结构上仍应保留为 body selector，但在当前 Stage1 stream1 主 rail 流里没有表现出动态切换作用
  - 同时，`byte1E` 的结构性 reader 这轮也被静态命中了，不再需要猜 packed-read：
    - `PrStageRunner_FrameUpdate (0x801C9094)`
      - `0x801C99EC : lbu event+0x1E`
      - `0x801C9A64 : lbu event+0x1E`
    - 说明 body selector 路径在代码上是真实存在的，只是当前 Stage1 stream1 主 rail 流把它固定在 `0`。

### `byte16 -> ctx+0xFC` Pulse Mapping

- 这轮把 `stream1` 整表里的 `byte16==1` event 和回放里的 `ctx+0xFC` 高窗逐一对齐了：
  - [psx_stage1_byte16_ctxfc_alignment_20260412.json](../artifacts/psx_stage1_byte16_ctxfc_alignment_20260412.json)
  - [psx_stage1_stream1_records_full_20260412.json](../artifacts/psx_stage1_stream1_records_full_20260412.json)
  - [psx_stage1_ctx_fc_track_20260412.json](../artifacts/psx_stage1_ctx_fc_track_20260412.json)
- 当前已能下硬结论：
  - `byte16==1` 的 event 一共 `33` 条
  - `ctx+0xFC == 1` 的高窗也一共 `33` 段
  - 两者可以按顺序 `33/33` 无歧义一一对应，不是松散相关
- 对齐口径：
  - 事件侧用 `event.frame`
  - 回放侧不能直接拿录制帧比较，而应拿 `ctxTick96(+0x0C)` 对齐
  - 定义 `Δtick = ctxFC 上升沿时的 ctxTick96 - event.frame`
  - 这条边界对 Stage1 compact rail 主体也成立：
    - `stage1_stream1_event_table_export_v2_20260412.json` 里的 `event.frame`
      和 replay truth json 里的录制 `frame` 不是同一时间基准
    - replay truth 的 `frame` 是 `export_windows` 的录制帧轴
    - `event.frame` 更接近 script / `tick96` 域
    - 当前没有证据支持一个全局固定的 `event.frame <-> replay frame` 直接换算公式
    - 所以后续 direct-PSX parser/validator 也应先经 `ctx+0x0C (tick96)` 建桥，再谈和 replay 的帧对位
- 统计结果：
  - `ctxFC` 从未早于 `byte16==1` event 拉高
  - `Δtick` 全部落在 `0..6`
  - 中位数 `+2`，均值约 `+2.7`
  - 换到录制帧域，`ctxFC` 上升沿只会发生在“同帧或下一帧”：
    - `lagF=0` 共 `13` 次
    - `lagF=1` 共 `20` 次
- 持续时长也呈现稳定分档：
  - `flags04=0x000000` 的主组几乎固定 `16` 帧高窗，仅 `idx31` 缩到 `14`
  - `flags04=0x030000` 的主组基本是 `34/32` 交替，仅 `idx240` 缩到 `30`
  - 边界两条长窗：
    - `idx6(frame=960, flags04=0x010000)` -> `32` 帧
    - `idx278(frame=24000, flags04=0x020000)` -> `64` 帧
- 代表样本：
  - `idx19 frame2784 flags=0x000000` -> `rise=1075 fall=1091 tickWin=2785..2830 Δtick=+1 durF=16`
  - `idx125 frame10560 flags=0x030000` -> `rise=3717 fall=3751 tickWin=10560..10656 Δtick=+0 durF=34`
  - `idx278 frame24000 flags=0x020000` -> `rise=8287 fall=8351 tickWin=24006..24192 Δtick=+6 durF=64`
- 直接结论：
  - `byte16` 已经不是“可能相关”的旁证，而是 PSX 里稳定对应一条 `ctx+0xFC` 脉冲触发链
  - 它更像 center-variant / mascot side selector 的 event 脉冲
  - 它不是 Stage1 compact rail 的 teacher/student authority，也不是 later-cycle compact lane 的主驱动
  - 新的静态链也支持这个边界：
    - [decompile_801c895c.txt](../temp/ida_stage1_ctxfc_chain_20260412/decompile_801c895c.txt)
    - [decompile_801c9094.txt](../temp/ida_stage1_ctxfc_chain_20260412/decompile_801c9094.txt)
    - [COMOD1_ctxFC.json](../temp/ghidra_reports/COMOD1_ctxFC.json)
    - [FUN_801cbfdc_ctxFC_dump.json](../temp/ghidra_reports/FUN_801cbfdc_ctxFC_dump.json)
    - [FUN_801cb190_full_dump.json](../temp/ghidra_reports/FUN_801cb190_full_dump.json)
    - `0x801C895C` 在 `0x801C8DD8` 写 `ctx+0xFC`
    - `0x801CBFDC` 在 `0x801CC5DC` 读 `ctx+0xFC`，并在 `0x801CC5F0` 写到 `dword_801D3014`
    - `0x801CBFDC` 后段先 `jal 0x80024744(ctx)`，再 `jal 0x801CB190`
    - `0x80024744` 读的是 compact rail row/raw/body-stream/marker 槽位，不消费 `ctx+0xFC`
    - 真正使用 selector bank 的是 `0x801CB190`：`0x801CB44C/0x801CB48C` 读 `dword_801D3014`，并走 `0x801D98D0 + 0x10*selector`
  - 因而 `byte16 -> ctx+0xFC` 这条链应继续归到 center mascot / TAMA variant，而不是 compact rail 本体

### `streamId` Branch Exclusion

- 这轮把 `streamId` 假设也正式排除了：
  - `1148..1492` 导出的 [psx_stage1_stream_meta_full_20260412.json](../artifacts/psx_stage1_stream_meta_full_20260412.json) 里，只有 stream1 descriptor cursor 在推进；
    slots `2..8` 的 cursor 都保持 `0`。
  - 同一窗口里 `g_PrStageEventStreamId @ 0x8008ED2C` 没有任何帧末变化，也没有命中写日志。
- `0x801C9094` 的静态控制流也支持这个结论：
  - active gameplay record 先走 `g_PrStageEventStreamFlag` 路径取 `v2`
  - direct compact bit-test：
    - `v2[1] & 0x800`
    - `v2[1] & 0x100000`
    都发生在 `LABEL_163` 之前
  - `LABEL_163` 之后才会单独取 `g_PrStageEventStreamId` 的 descriptor；
    这条支路当前只看到了 branch/follow-up `ApplyScriptEvent + SetText`，没有再回到 compact bootstrap bit-test。
- 当前已确认的 `streamId` 赋值入口只覆盖 branch / follow-up 流：
  - `(*ctx & 0x40) != 0` -> `streamId = 4/5`
  - `unk_8008ED1C == 1` -> `streamId = 2/3/4`
  - `(*ctx & 0x2000) != 0` -> `streamId = 6/8`
  - `(*ctx & 0x4000) != 0` -> `streamId = 7`
- 直接结论：
  - active Stage1 compact rail 的 direct bootstrap / overlap truth，当前只在 `stream1 gameplay record` 上有证据；
  - `streamId 2..8` 不能再当 later-cycle active compact rail 的主 authority 来源。

### `byte1F` vs `textId`

- 这轮继续把 stream1 raw record 的 `+0x1F/+0x20` 和事件表导出里的 `textId` 做了逐项比对：
  - [comod1_event_tail_nonstack_scan_20260412.json](../artifacts/comod1_event_tail_nonstack_scan_20260412.json)
  - [comod1_event_build_findings_20260412.json](../artifacts/comod1_event_build_findings_20260412.json)
  - [comod1_stream_table_xrefs_20260412.json](../artifacts/comod1_stream_table_xrefs_20260412.json)
  - [psx_stage1_byte1f_byte20_classes_20260412.json](../artifacts/psx_stage1_byte1f_byte20_classes_20260412.json)
  - [psx_stage1_byte1f_authored_context_20260412.json](../artifacts/psx_stage1_byte1f_authored_context_20260412.json)
  - [psx_stage1_byte1f_nonzero_scope_20260412.json](../artifacts/psx_stage1_byte1f_nonzero_scope_20260412.json)
  - [psx_stage1_byte1f_nonzero_flags_20260412.json](../artifacts/psx_stage1_byte1f_nonzero_flags_20260412.json)
  - [psx_stage1_byte1f_ordinal_proof_20260412.json](../artifacts/psx_stage1_byte1f_ordinal_proof_20260412.json)
  - [psx_stage1_byte1f_sequence_20260412.json](../artifacts/psx_stage1_byte1f_sequence_20260412.json)
  - [psx_stage1_byte1f_segments_20260412.json](../artifacts/psx_stage1_byte1f_segments_20260412.json)
  - [psx_stage1_byte1f_nonzero_steps_20260412.json](../artifacts/psx_stage1_byte1f_nonzero_steps_20260412.json)
  - [psx_stage1_byte1f_transition_windows_20260412.json](../artifacts/psx_stage1_byte1f_transition_windows_20260412.json)
  - [psx_stage1_stream1_records_large_20260412.json](../artifacts/psx_stage1_stream1_records_large_20260412.json)
  - [stage1_stream1_events.json](../temp/stage1_stream1_events.json)
  - [stage1_stream1_event_table_export_v2_20260412.json](../artifacts/stage1_stream1_event_table_export_v2_20260412.json)
- 已确认的稳定事实：
  - `tools/stage1_event_table_export.py` 这轮已补齐原始 tail byte 导出；`stream1` v2 源表现在会直接带出 `byte16..byte20`，不用再手工把 raw replay 和 event-table export 拼起来。
  - `FUN_801c8604` 静态上只读 `event+0x20` 作为 `textId`。
  - 这轮用 headless IDA 对 COMOD1 当前已解码代码做了两轮 `displ=0x1F/0x20` 扫描：
    - 按已识别函数扫描
    - 按所有已解码 code heads 扫描
  - 命中的 event-byte read 仍然只有：
    - `PrStageRunner_ApplyScriptEvent_SetText (0x801C8604) : lbu event+0x20`
  - 当前没有扫到任何 `event+0x1F` 的已解码读点。
  - 这轮又补了一次“过滤掉栈槽噪音”的 non-stack tail scan：
    - `PrStageRunner_ApplyScriptEvent_SetText (0x801C8604)` 只读 `event+0x20`
    - `PrStageRunner_ApplyScriptEvent (0x801C895C)` 只读 `event+0x1C`
    - `PrStageRunner_FrameUpdate (0x801C9094)` 只读 `event+0x1D/+0x1E`
    - 额外出现的 `PrStageRunner_Run (0x801C7A60)` 对 `$s1+0x20` 的写入是 ctx/local field 更新，不是 event tail 读取
  - `0x801C9094` 当前导出的控制流也支持“没有下游 event-tail copy”这一点：
    - `v57 = (*stream.ptr + 36*cursor)` 之后
    - 只有 `PrStageRunner_ApplyScriptEvent(a1, v57)` 和 `PrStageRunner_ApplyScriptEvent_SetText(a1, v57)`
    - 没有看到中间把 `0x1C..0x20` tail 单独复制到新缓冲再消费的窗口
  - 到目前为止，filtered non-stack scan 仍然没有任何显式 `event+0x1F` 访问。
  - first-phase 早段里，`+0x1F` 和 `+0x20/textId` 确实会重合：
    - `idx18 frame2688`: `byte1F=5`, `byte20=5`, `textId=5` -> `Kick`
    - `idx24 frame3072`: `byte1F=7`, `byte20=7`, `textId=7` -> `Punch`
    - `idx36 frame3840`: `byte1F=11`, `byte20=11`, `textId=11` -> `Block`
  - 但从 `idx43 frame4224` 开始，两者就脱钩了：
    - `idx43`: `flags04=0x30000`, `byte1D=5`, `byte1F=14`, `byte20=0`, `textId=0`
    - `idx46`: `flags04=0x30000`, `byte1D=5`, `byte1F=15`, `byte20=14`, `textId=14`
    - `idx49`: `flags04=0x30000`, `byte1D=6`, `byte1F=16`, `byte20=15`, `textId=15`
    - `idx52`: `flags04=0x30000`, `byte1D=6`, `byte1F=17`, `byte20=16`, `textId=16`
  - 直接结论：
    - `+0x20` 仍应保留为唯一稳定的 displayed `textId` 字段。
  - `+0x1F` 不能再与 `textId` 合并；这轮已经能把它压成 authored token ordinal，而不是只说“像 lookahead”：
    - 对 stream1 全表逐行验证后，`byte20` 在每一条 `byte20 != 0` 的 row 上都零误差等于 visible-text ordinal
    - 同样地，`byte1F` 在每一条 `byte1F != 0` 的 row 上都零误差等于：
      - `visible_text_count + hidden_token_count - display_only_count`
      - 以上三个计数都按当前 row 累计
    - 当前 proof 统计是：
      - `visible_rows = 75`
      - `hidden_token_rows = 5`
      - `display_only_rows = 1`
      - `byte1F_nonzero_rows = 79`
      - 两条公式的 mismatch 都是 `0`
    - 因而更稳的语义已经是：
      - `byte20` = displayed text ordinal
      - `byte1F` = authored token ordinal
      - 两者的当前偏移量 = `hidden_token_count - display_only_count`
  - 当前在已导出的 `0x801C895C / 0x801C9094` 伪 C、现有 grep、以及这轮 headless IDA 的函数级/全 code-head 扫描里，都还没有找到 `+0x1F` 的明确静态消费者，所以暂时只能保留原值，不给硬语义。
  - 当前更强的 runtime 边界判断是：
    - `0x801C9094` 只把 event 指针直接传给 `0x801C895C` 和 `0x801C8604`
    - `0x801C895C` 不读 `+0x1F`
    - `0x801C8604` 只读 `+0x20`
    - filtered non-stack scan 也没有别的 `+0x1F` 访问
    - 因而在当前 Stage1 overlay runtime 范围内，`byte1F` 很可能不是 direct-consumed runtime field，而更像 authored token / queue ordinal
  - 行级分类也已经收口：
    - `aligned(byte1F==byte20!=0)`：`13` 条
    - `hidden_token(byte1F!=0, byte20==0)`：`5` 条
    - `display_only(byte1F==0, byte20!=0)`：`1` 条
    - `offset_visible(byte1F!=0, byte20!=0, byte1F!=byte20)`：`61` 条
    - `both_zero`：其余 `201` 条
  - 其中关键例外是：
    - `hidden_token` 只有 `idx43/61/95/113/207`
    - `display_only` 只有 `idx204(frame18288, flags=0x00000000, byte20=61)`
  - `hidden_token` 行在 authored source table 里不是“只有 `byte1F` 异常”的裸行，而是稳定的段首 seed 行：
    - 5 条 `hidden_token` 都是 `0x30000`
    - `byte17` 稳定回到 `3`
    - `byte19/1A/1B` 会恢复到一组 HUD/resource seed：
      - 常见 `8/13/14`
      - `idx113` 为 `9/13/14`
    - 同时 `byte20/textId = 0`
  - 对应地，紧跟在这些 `hidden_token` 后面的第一条 visible `0x30000` record 会把 `byte19/1A/1B` 清回 `0/0/0`，并开始发出真实可见文本：
    - `idx43 -> idx46 : 14/0 -> 15/14`
    - `idx61 -> idx64 : 21/0 -> 22/20`
    - `idx95 -> idx98 : 33/0 -> 34/31`
    - `idx113 -> idx116 : 40/0 -> 41/37`
    - `idx207 -> idx213 : 65/0 -> 66/62`
  - 唯一的 `display_only idx204` 也已被源表钉成 authored pure-text row：
    - `flags04 = 0x00000000`
    - `byte17..byte1F` 全是 `0`
    - 只有 `byte20 = 0x3D`
    - 没有任何 HUD slot / selector pair seed
  - 这说明当前看到的 `hidden_token / display_only` 不是 runtime 临时吃字或补字，更像 authored 脚本里显式存在的：
    - segment seed / warm-up row
    - pure-text bridge row
  - 这也解释了为什么 `max(byte1F)-max(byte20) == 4`：
    - authored token 侧多出 `5` 条 hidden-token
    - 但又有 `1` 条 display-only row 把一格 visible text 补回
    - 净差值正好是 `4`
  - 这条偏移变化也已经能按累计计数解释，不必再只用“分段 delta”描述：
    - `idx43` 后：`hidden=1, display_only=0`，所以 `byte1F-byte20 = 1`
    - `idx61` 后：`hidden=2, display_only=0`，所以 `byte1F-byte20 = 2`
    - `idx95` 后：`hidden=3, display_only=0`，所以 `byte1F-byte20 = 3`
    - `idx113` 后：`hidden=4, display_only=0`，所以 `byte1F-byte20 = 4`
    - `idx204` 纯文本桥接后：`hidden=4, display_only=1`，理论偏移短暂回到 `3`
    - `idx207` 新 hidden-token 后：`hidden=5, display_only=1`，偏移回到 `4`
  - `PrEventTable_Build (0x801C8660)` 这轮也补看过了：
    - `decompile_801c8660.txt` 现在已经导出，行为比之前更清楚：
      - 它循环 `53` 次，构造的是一张独立的 `8B` runtime table
      - 写入目标是 `0x8008EEF8/0x8008EEFC + 8*i`
      - 输入来源是 `0x801D2E2C + 4*i` 的另一张短描述表
      - 每项只做：
        - `24 * source_halfword0`
        - `0x80024BC0(source_halfword1)`
    - 所以它不是在复制或加工 `36B` script event body；当前也没有给 `byte1F` 提供新的消费窗口
  - `PrStageRunner_EventStream_Reset (0x801C858C)` 这轮也被 decompile 钉住了：
    - 只按 `streamId` 选中 `0x801D2D64 + 12*streamId`
    - 清该 entry 的第三个 dword
    - 把 `ctx+0x0C` 写到 `dword_801D3048`
    - 不读取、不复制任何 `36B` event body
  - late-phase 仍保留“分段式”观察值，但现在应把它降级为 authored ordinal 的表现形式，而不是主定义：
    - `idx43..58` 这一小段里，`0x30000` 主 gameplay record 确实表现成 one-step pipeline：
      - `current.byte20 == current.textId`
      - `current.byte20 == previous_0x30000.byte1F`
      - `current.byte1F == current.byte20 + 1`
    - 样本：
      - `idx46`: `byte1F=15`, `byte20=14`, `textId=14`, 上一条 `0x30000(idx43)` 的 `byte1F=14`
      - `idx49`: `byte1F=16`, `byte20=15`, `textId=15`, 上一条 `0x30000(idx46)` 的 `byte1F=15`
      - `idx52`: `byte1F=17`, `byte20=16`, `textId=16`, 上一条 `0x30000(idx49)` 的 `byte1F=16`
      - `idx55`: `byte1F=18`, `byte20=17`, `textId=17`, 上一条 `0x30000(idx52)` 的 `byte1F=17`
      - `idx58`: `byte1F=19`, `byte20=18`, `textId=18`, 上一条 `0x30000(idx55)` 的 `byte1F=18`
    - 但更后段会重新分段并改变偏移：
      - `idx61`: `byte1F=21`, `byte20=0`
      - `idx64`: `byte1F=22`, `byte20=20`
      - `idx67`: `byte1F=24`, `byte20=22`
      - `idx71`: `byte1F=25`, `byte20=23`
      - `idx98`: `byte1F=34`, `byte20=31`
    - 这说明 late-phase 的“lookahead 感”其实来自 authored token ordinal 与 visible-text ordinal 之间的累计偏移，而不是必须假设一个 runtime queue consumer。
    - `idx43` 自身仍可视为第一段 pipeline 的 warm-up：`byte1F=14`, `byte20=0`, `textId=0`
  - 这轮再往前压了一层分段边界：
    - `delta=0` 段：`idx12..39`，覆盖 `byte1D=0..4`
    - `reset(byte20=0)`：`idx43`
    - `delta=1` 段：`idx46..58`，覆盖 `byte1D=5..7`
    - `reset(byte20=0)`：`idx61`
    - `delta=2` 主段：`idx64..92`，其中 `idx66/68` 是插入式 reset，稳定覆盖 `byte1D=8..13`
    - `reset(byte20=0)`：`idx95`
    - `delta=3` 段：`idx98..110`，覆盖 `byte1D=14..16`
    - `reset(byte20=0)`：`idx113`
    - `delta=4` 段：从 `idx116` 开始成为主模式，之后大部分 `0x30000` 记录都维持 `byte1F = byte20 + 4`，中间穿插新的 reset record
  - 当前更稳的压缩规则：
    - `byte1F` 不是只活跃在 `0x30000`
    - 全表 `byte1F != 0` 的 flags 分布是：
      - `0x00030000 x68`
      - `0x00010000 x3`
      - `0x00031000 x3`
      - `0x00000000 x2`
      - `0x00000800 x2`
      - `0x00020000 x1`
    - 也就是说，`0x30000` 是主承载环，但不是唯一承载者
    - 早段非 `0x30000` 样本也是真实存在的：
      - `idx8 frame1152 flags=0x00010000 -> byte1F=1, byte20=1`
      - `idx10 frame1536 flags=0x00010000 -> byte1F=2, byte20=2`
      - `idx14 frame2304 flags=0x00010000 -> byte1F=4, byte20=4`
      - `idx40 frame4080 flags=0x00000000 -> byte1F=13, byte20=13`
      - `idx60 frame5328 flags=0x00000800 -> byte1F=20, byte20=19`
      - `idx279 frame24192 flags=0x00020000 -> byte1F=79, byte20=75`
    - visible text 侧本身也已经压实成一条连续序列：
      - 全表 `byte20 != 0` 的去重顺序正好是 `1..75`
      - 说明 `byte20/textId` 仍然是完整的 displayed text ordinal，只是它的承载 row 不总与 `byte1F` 同行
    - 在后半段最规整的部分，`byte1F` 主要沿 `0x30000` gameplay ring 按段推进
    - 每段开头常出现 `byte20=0` 的 reset/warm-up record
    - 段内再维持固定 `byte1F-byte20` 偏移，且偏移会随段向后抬高：`0 -> 1 -> 2 -> 3 -> 4`
    - 如果只看全部 `byte1F != 0` record，它本身又近似是一条单调递增 token 序列：
      - 段内大多数相邻非零 record 都是 `byte1F + 1`
      - 跨段时常见 `byte1F + 2`
      - 所以 `byte1F` 更像独立的 queue/token ordinal；`byte20/textId` 只是按段从它派生/对齐出来的显示侧值
    - 当前步进统计：
      - `byte1F_step` 直方图：`+1 x59`, `+2 x8`
      - `byte20_step` 则会在 reset/warm-up 处出现大幅负跳和下一条的大幅正跳
    - 结合分段表，一个更强但仍属推断的解释是：
      - `byte1F` 更接近“全局 authored token ordinal”
      - `byte20/textId` 是在显示层跳过若干隐藏 warm-up token 后得到的可见文本 id
      - `delta` 从 `0` 抬到 `4`，正对应当前已知会抬升显示偏移的 4 个早中段 warm-up 边界（`idx43/61/95/113`）
  - 这轮还补了一条更窄的段首门条件：
    - reset 不是随机插入，但“总是紧跟 `0x800` 切段”这个说法只对早中段的窄义 token-reset 成立
    - 按广义 reset（`flags04==0x30000 && byte20==0`）扫全表，共 `46` 条：
      - 只有 `4` 条严格命中“`0x800` 切段后的下一条 `0x30000` record”：
        - `idx42(frame4176, 0x800, byte1D=5) -> idx43(frame4224, 0x30000 reset)`
        - `idx60(frame5328, 0x800, byte1D=8) -> idx61(frame5376, 0x30000 reset)`
        - `idx94(frame8016, 0x800, byte1D=14) -> idx95(frame8064, 0x30000 reset)`
        - `idx112(frame9168, 0x800, byte1D=17) -> idx113(frame9216, 0x30000 reset)`
      - 其余 `42` 条都是例外，说明广义 reset 不能再概括成“紧跟 `0x800` 切段”
    - 若只看窄义 token-reset（再加 `byte1F!=0`），则共有 `5` 条：
      - 上述 `4` 条成立
      - 但 `idx207(frame18432, byte1D=28, byte1F=65, byte20=0)` 是明确例外
    - 因而更稳的记法是：
      - 早中段的段首 token-reset 确实和一部分 `0x800 -> next 0x30000` teacher/bootstrap 节拍同步
      - 但 full raw table 的 reset 总体并不总是严格落在这种切段后
  - 这条“切段后紧接 seed”关系现在也已经能在 authored source table 上直接看到窗口：
    - `idx42(frame4176, flags=0x800, byte1D=5) -> idx43(frame4224, flags=0x30000, hidden-token)`
    - `idx60(frame5328, flags=0x800, byte1D=8) -> idx61(frame5376, flags=0x30000, hidden-token)`
    - `idx94(frame8016, flags=0x800, byte1D=14) -> idx95(frame8064, flags=0x30000, hidden-token)`
    - `idx112(frame9168, flags=0x800, byte1D=17) -> idx113(frame9216, flags=0x30000, hidden-token)`
    - 晚段例外是 `idx206(frame18384, flags=0x20800, byte1D=28) -> idx207(frame18432, flags=0x30000, hidden-token)`
  - headless COMOD1 xref 也把 event source 的静态边界压实了：
    - `0x801D2D64(stream table)` 只有 `PrStageRunner_EventStream_Reset(0x801C858C)` 和 `PrStageRunner_FrameUpdate(0x801C9094)` 的 direct dref
    - `0x801D0088(stream1 events start)` 没有 direct xref
    - `0x801CE8D4(stream desc table)` 只被 `PrStageRunner_ApplyScriptEvent(0x801C895C)` 和 `PrStageRunner_FrameUpdate(0x801C9094)` 读
    - 这说明当前 Stage1 event body 确实是通过 `stream entry -> events_ptr` 间接取用的纯静态 authored 表，不存在“代码直接写死某条 event blob”的新旁路

### `stream1` Row Grammar

- 这轮把 `stream1` 的 row grammar 也单独压成了 parser 级摘要：
  - [psx_stage1_stream1_flag_grammar_20260412.json](../artifacts/psx_stage1_stream1_flag_grammar_20260412.json)
  - [psx_stage1_stream1_row_grammar_20260412.json](../artifacts/psx_stage1_stream1_row_grammar_20260412.json)
- 当前最稳的 `flags04` 角色如下：
  - `0x00000800`
    - direct teacher bootstrap / descriptor-cut row
    - 静态上命中 `0x801C9094` 的 `flags & 0x800` 分支，直接写 teacher lane/raw/body cache
    - source-table 上通常是 zero-tail timing row；但 `idx60/112` 是明确例外，也会携带 visible text/token
  - `0x00100000`
    - direct student catch-up/bootstrap row
    - 静态上命中 `0x801C9094` 的 `flags & 0x100000` 分支，从 teacher cache 接 student lane
    - source-table 上 tail byte 基本全零，更像 producer timing row，不是 text/token carrier
  - `0x00030000`
    - main gameplay ring row
    - 是 `byte1F`、`byte20`、hidden-token warm-up、以及 shell pair/hud 更新的主承载环
    - 既包含普通 visible text row，也包含 `idx43/61/95/113/207` 这类 hidden-token seed row，还包含大量 zero spacer row
  - `0x00031000`
    - `0x00030000 + 0x1000` 的稀有 late-row 变体
    - 静态上会先进入 `0x1000` reset block，再继续走后续 per-row 逻辑
    - source-table 上都是 visible text/token row，且带非零 seed bytes；当前更像 phrase/setup + clear/reset modifier
  - `0x00020800`
    - `0x00020000 + 0x800` 的 late combined row
    - 静态上仍会走 teacher bootstrap，因为 `0x800` 已置位
    - source-table tail bytes几乎全零；更像 timing/control 组合行，同时给 shell 的 mode-pair 更新留出入口
  - `0x00021000`
    - `0x00020000 + 0x1000` 的单个 late control row
    - 静态上只可安全记成 shell mode-pair row + clear/reset modifier
  - `0x00010000`
    - early shell pair/text row，走 `ev+0x14/0x15`
    - 没有 direct compact bootstrap bit
    - 主要集中在 intro/prelude；`byte17=3`，可见文本只落在 `idx8/10/14`
  - `0x00020000`
    - shell mode-pair row，走 `ev+0x10..0x13`
    - 通常不是 compact producer row
    - 绝大多数无 text/token，唯一明确 closing-text 样本是 `idx279`
  - `0x00000000`
    - neutral spacer / pure-text bridge row
    - 没有 direct compact bootstrap bits
    - 其中 `idx204` 是唯一 display-only pure-text bridge；late rows 还会出现 `byte19=8/9` 的壳层 marker
  - `0x00000080`
    - terminal branch trigger row
    - 静态上会置 `dword_8008ED1C`
  - `0x000F1020`
    - frame0 setup composite，仅 startup 使用
- 直接结论：
  - `stream1` 不是“所有 row 都同一语法”的单环，而是：
    - compact producer timing rows (`0x800 / 0x100000`)
    - shell pair rows (`0x10000 / 0x20000`)
    - main gameplay rows (`0x30000`)
    - late clear/reset variants (`0x31000 / 0x21000 / 0x20800`)
    - pure spacers / branch markers (`0x0 / 0x80 / frame0 composite`)
  - 后续做 direct-PSX parser 时，`flags04` 至少应拆成这一层 grammar，而不是只保留 raw bitfield 注释。

### `byte1D` Selector Families

- 在 row grammar 基础上，这轮又把 `byte1D` 分组压成了有限几个 authored 段模板：
  - [psx_stage1_byte1d_group_templates_20260412.json](../artifacts/psx_stage1_byte1d_group_templates_20260412.json)
  - [psx_stage1_byte1d_template_families_20260412.json](../artifacts/psx_stage1_byte1d_template_families_20260412.json)
- 当前最稳的 family 划分：
  - `selector 0`
    - intro / prelude setup block
  - `selectors 1,2,3,5,6,7,10,11,12,13,14,15,16`
    - canonical 6-row teaching segment：
      - `0x800 -> 0x30000 -> 0x0 -> 0x100000 -> 0x30000 -> 0x20000`
  - `selector 4`
    - canonical 段，但在 shell close 前多一条 `0x00000000` spacer
  - `selectors 8,17`
    - bootstrap-text-plus-hidden-seed family：
      - teacher bootstrap 自己就携带 visible text/token
      - 随后紧接一条 hidden-token `0x30000`
      - 再到 visible `0x30000`
      - 不带最后的 `0x20000` shell close
  - `selectors 9,18`
    - phrase-setup-reset family：
      - `0x31000` 开场
      - 后接几条 `0x30000` phrase row
      - 以 `0x20000` shell close 收尾
  - `selectors 19,20,21,23,24,25`
    - extended combo-loop family：
      - bootstrap
      - 多条 gameplay row / spacer
      - 显式 student bootstrap
      - 两条 visible gameplay close
      - 最后 `0x20000` shell close
  - `selector 22`
    - extended combo-loop family 的加长版，close 前多一轮 spacer/visible
  - `selector 26`
    - combo-loop no-shell-close 变体
  - `selector 27`
    - long late-phrase bridge：
      - `0x31000` setup
      - 多条 `0x30000` phrase row
      - `0x00000000` pure-text bridge
      - `0x20000` shell close
  - `selectors 28,29,30,31,32,33`
    - late combo-repeat family：
      - `0x20800` late combined bootstrap
      - hidden-token / neutral bridge rows
      - `0x100000` student bootstrap
      - 两条 visible gameplay close
      - `0x20000` shell close
  - `selector 34`
    - late combo-repeat extended 变体，以额外 visible `0x30000` 收尾，不走 `0x20000` close
  - `selector 35`
    - finale tail：
      - `0x21000`
      - 若干 shell row
      - closing text `idx279`
      - `0x80` branch trigger `idx280`
- 直接结论：
  - `byte1D` 现在已经不只是“compact desc selector”的单字段注释，而是 authored 段模板的核心 selector。
  - direct-PSX parser 至少需要同时建模：
    - `flags04` row grammar
    - `byte1D` selector family
    - 以及 `0x801C9094` 对 `byte1D/byte1E` 的 compact table 索引行为
  - `byte1D` 对应的 compact row table 布局也已经有了直接支撑：
    - [psx_stage1_desc_row_layout_20260412.json](../artifacts/psx_stage1_desc_row_layout_20260412.json)
    - [psx_stage1_compact_row_invariance_20260412.json](../artifacts/psx_stage1_compact_row_invariance_20260412.json)
    - [psx_stage1_compact_row_table_20260412.json](../artifacts/psx_stage1_compact_row_table_20260412.json)
    - `0x801C9094` 的指针运算是：
      - `row_base = 0x801CFA54 + 44 * byte1D`
      - `laneA_ptr = 0x801CFA58 + 44 * byte1D`
      - `laneB_ptr = 0x801CFA6C + 44 * byte1E`
    - replay 抓到的 `desc0..desc4` 窗口虽然也是 `44B`，但它们从 `laneA_ptr` 开始，不是从 `row_base` 开始：
      - 每个窗口实际包含 `laneA[20] + laneB[20] + next_row_header[4]`
    - 当前最稳的 row layout 仍可先记成：
      - `int16 head0`
      - `int16 head1`
      - `uint8 laneA[20]`
      - `uint8 laneB[20]`
    - 但 `desc0..desc4` payload 窗口现在反而支持的是旧结论，而不是前一轮误记的 `head1=selector`：
      - `selector-like` 的 `1/2/3/4` 出现在 `laneA` 里：
        - `desc1 laneA = [0,0,1,0,0,0,0,0,0,-1,...]`
        - `desc2 laneA = [0,0,2,0,0,0,0,0,0,-1,...]`
        - `desc3 laneA = [0,0,3,0,0,0,0,0,0,-1,...]`
        - `desc4 laneA = [0,0,4,0,0,0,0,0,0,-1,...]`
      - 而窗口尾部带出的 next-row header 一直是 `head0=1, head1=0`
    - 直接结论：
      - `desc0..4` 这份 replay artifact 不能再被用来支持 `head1 = selector ordinal`
      - 当前更强的证据反而是：row header 和 payload 语义必须分开；visible lane/type 更接近 payload 字节，不是 `head1`
    - 同时，这些 sampled compact rows 在主窗口里还是强 invariant：
      - `desc0..4` 的 payload 窗口在 `501` 个采样帧里各自只有 `1` 份唯一内容
      - 每个窗口尾部带出的 next-row header 也都只有 `1` 组唯一值：`(1,0)`
      - `teacherDescCache304C` 只在 `0x801CFA84 -> 0x801CFAB0 -> 0x801CFADC -> 0x801CFB08` 这些 payload row 地址间切换
      - `teacherBodyCache3050` 在该窗口里则始终固定 `0x801CFA6C`
    - 因而在 Stage1 主 rail 窗口内，`byte1D` 选中的这组 compact rows 更像“overlay-sourced static row table”，不是逐帧重写的动态 body cache。
    - `head0/head1` 的静态用途也因此更清楚了：
      - `0x801C9094` 在 `flags & 0x800` 分支里先做：
        - `phaseCount = head0`
        - `cursor = head1`
      - 然后在同一帧的 teacher update block 里，若 `phaseCount == 1` 且 `laneA[cursor] != -1`，会立刻 `cursor++`
      - 所以在当前样本里看到的 `head0=1, head1=0`，会自然导向“首个可见 onset 落在 cursor=1”
      - 这也解释了为什么 PSX overlap/onset 观察值经常表现成：
        - payload row 明明以 `[...,1,...]` 为第一个有意义的 lane token
        - 但 header 本身并没有 `head1=1`
    - 这轮再把 `0x801CFA54` 的静态 row table 整体拉直之后，Stage1 stream1 当前实际用到的 compact-row execution boundary 也清楚了：
      - selector `0..35` 的静态 row 全部满足：
        - `head0=1`
        - `head1=0`
        - `laneB[20]` 全零
        - 不存在任何 `-2` byte
      - 直接结论：
        - 在 Stage1 stream1 主 rail 里，`count=2` / `laneB` / `-2 sentinel` 不是活跃主链，而是 `0x801C9094` 这套通用 compact interpreter 为别的 overlay/分支预留的 generic path
        - 这也和 replay truth 一致：
          - overlap/handoff 由 teacher lane machine + student lane machine 的双 actor 协作给出
          - 不是 teacher 自己切到 `phaseCount=2`
      - row family 从静态 payload 上也能再细一层：
        - empty spacer / bridge rows：`0, 9, 18, 27, 35`
        - short one-shot rows：`1..8, 10..17`
          - `laneA_first_neg1 = 9`
          - 典型形态：`[0,0,n,0,0,0,0,0,0,-1,...]`
        - full combo rows：`19..26, 28..34`
          - `laneA[20]` 内没有 `-1`
          - 它们承载的是多 token 的 authored combo/repeat pattern，而不是前半段那种单 note onset row
          - 直接实现含义：
            - 这类 row 在 Stage1 stream1 里不能靠 payload sentinel 自收尾
            - 它们的 ownership 结束更像由下一条 bootstrap row (`0x800` / `0x100000`) 抢占，而不是 row 内部 hit `-1`
      - 这意味着 direct-PSX parser 的实现顺序也应该收紧：
        - 先按 `head0=1/head1=0 + laneA` 复刻 Stage1 stream1 的真实执行模型
        - 不要在实现主线时先引入 `count=2` 或 `-2` 的 generic path
        - 那些路径除非后续 replay/别的 overlay 证明活跃，否则只应保留为“已知存在但当前未启用”的静态边界
    - 伪 C 这轮也暴露了两个需要警惕的误译点：
      - student bootstrap 块里出现的 `a1[41] = 0; a1[42] = 0;`
        - 不能按字面相信
        - 因为后面的 student update block 紧接着就把 `a1[41] / a1[42]` 当有效 row 指针解引用
        - 也与 replay 里 `ctx+0xA4` 持续指向 `0x801CFA84/AB0/ADC/...` 的事实冲突
      - reset 块里第二次出现的 `*((_WORD *)a1 + 69) = 0`
        - 更像 Hex-Rays 在交错 halfword store 上做错了别名折叠
        - 不能把它当成“student 也共享 teacher count 的显式 reset 证据”
      - 因而这部分实现仍应坚持：
        - 字段 ownership 以 replay truth 为准
        - 伪 C 只用来给出 control-flow / pointer-arithmetic 的边界，不直接拿错译 store 当 authority

### Next Static Target

- 复用 stream meta + event record 补证后，下一静态落点不再是 `streamId` 分支本身：
  - `streamId 2..8` 当前已经收口成 branch/follow-up 流
  - later-cycle active compact rail 仍应压在 `stream1` gameplay record 上
- 下一优先级应改成：
  - 不再继续重复扫 overlay 下游 `event+0x1F` 读取点；这一侧当前已经足够说明“runtime 范围内未 direct-consume”
  - 转而往上游补 authored/export 侧语义：
    - `PrStageRunner_ApplyScriptEvent` 对主 record `flags04` 的上游语义
    - `stream1` record 内 `byte1D / byte1E / byte1F / byte20` 的 authored role
    - `EventTable_Build / stream source table` 是否会在 overlay 外给 `byte1F` 赋更明确的 token 语义
  - 同时继续保留 `0x801C9094` 窄切片和这些 record bytes 的对应关系：
    - `0x801C99EC`
    - `0x801C9334`
    - `0x801C9CE0`
    - `0x801C947C`
  - 特别是：
    - `byte1F` 是否根本没有 overlay runtime consumer，而只是 authored token ordinal
    - 为什么它在 early-phase 与 `textId` 重合、却在 late-phase 变成分段 reset 的 queue/lookahead token
    - `byte16 -> ctx+0xFC` 这条线本轮已完成动态 + 静态收口，后续只需在实现时按既有 desc/resource 对位消费
- `0x801C895C` 这轮新导出的伪 C 反而补了一条排除结论：
  - 它主要在按 `unk_801CE8D4` 和 event bytes 写 HUD/resource slot、late-row/TOD 相关资源句柄；
  - 当前新 dump 没有出现对 compact lane 主字段的直接写入：
    - `ctx+0x8C`
    - `ctx+0x94`
    - `ctx+0x9E`
    - `ctx+0xA4`
  - 所以对 Stage1 compact rail 本身，`0x801C895C` 更像并行的 HUD/resource producer，不是 compact lane writer hub。
- 另外，这轮新导出的 `0x801C858C` 只支持把它定义成 stream reset/helper：
  - 会按 `a2` 选 `unk_801D2D64 + 12*a2`
  - 清该项的第三个 dword
  - 同时把 `ctx->tick96` 写到 `dword_801D3048`
  - 目前没有证据支持把它提升成 later-cycle compact lane writer

## 2026-04-12 Narrow Slice Semantics

- 这轮把 `0x801C9094` 里最关键的 4 个窄切片从“writer 名字”进一步收成了最小语义块。
- 注意：
  - `0x801C99EC / 0x801C9CE0` 更像 bootstrap cluster 的中段地址，不是单一一条 store
  - `0x801C9334 / 0x801C947C` 更像 hide/steady block 的 label，不是唯一写 cursor 的那条 PC
  - 但用它们做 later-cycle rail 的职责命名仍然是稳的

### `0x801C99EC` = teacher next-lane bootstrap / descriptor cut

- 上游门条件：
  - 当前 event record 有效
  - 且 `event.flags04 & 0x800 != 0`
- 关键读：
  - `event+0x1D`
    - descriptor selector byte
  - `event+0x1E`
    - body selector byte
  - `unk_801CFA54 + 44*sel`
    - teacher desc head / initial cursor
- 关键写：
  - `ctx+0x8A = descHead`
  - `ctx+0x8E = -1`
  - `ctx+0x90 = 1`
  - `ctx+0x8C = initialCursor`
  - `ctx+0x94 = 0x801CFA58 + 44*(event[0x1D])`
  - `ctx+0x98 = teacher body ptr`
  - 同时更新：
    - `dword_801D304C`
    - `dword_801D3050`
- later-cycle 对应：
  - `1157`: teacher 切到 `desc2`
  - `1287`: teacher 切到 `desc3`
  - `1417`: teacher 切到 `desc4`

### `0x801C9334` = teacher hide/reset tail

- 这段是 teacher lane phase tail 的收尾块，不是普通 steady step。
- 它前面的 teacher state machine 会分两种模式：
  - `ctx+0x8A == 1`
  - `ctx+0x8A == 2`
- 两条路径都会先尝试沿：
  - `ctx+0x94[ctx+0x8C]`
  - 或 phase2 的 `ctx+0x98[ctx+0x8E]`
 继续推进。
- 真正进入这个 hide tail 时，关键效果是：
  - `ctx+0x8E = -1`
  - `ctx+0x90 = 0`
  - `jal 0x80024308`
    - reset bank-A
- later-cycle 对应：
  - `1229 / 1361 / 1491`
  - 也就是 `teacher 9 -> hidden`

### `0x801C9CE0` = student catch-up bootstrap

- 上游门条件：
  - 当前 event record 有效
  - 且 `event.flags04 & 0x100000 != 0`
- 这段不是自己重算一条新 lane，而是直接接 teacher 当前 lane cache：
  - `ctx+0xA4 = dword_801D304C`
  - `ctx+0xA8 = dword_801D3050`
- 关键写：
  - `ctx+0xA2 = 1`
  - `ctx+0x9E = 0`
  - `ctx+0xA0 = -1`
  - `*ctx |= 0x100000`
  - `ctx+0xA4 = current teacher desc cache`
  - `ctx+0xA8 = current teacher body cache`
- 如果当前 selector 对应的 descriptor head `< 0`，还会：
  - `jal 0x80024390`
    - reset bank-B
- later-cycle 对应：
  - `1221`: student 切到 `desc2`
  - `1351`: student 切到 `desc3`
  - `1483`: student 切到 `desc4`

### `0x801C947C` = student steady/hide block

- `0x801C947C` 本身是 join label，真正的 cursor 写入散在它前面的几个 PC：
  - `0x801C93AC`
  - `0x801C93BC`
  - `0x801C9408`
  - `0x801C9464`
  - `0x801C946C`
- 但从 later-cycle 语义看，把这整块叫成 student steady/hide block 是对的：
  - mode1：
    - 沿 `ctx+0xA4[ctx+0x9E]` 推进
    - step store 在 `0x801C93AC`
  - mode2：
    - 先过渡到：
      - `ctx+0x9E = -1`
      - `ctx+0xA0 = 2`
      - `ctx+0xA2 = 2`
    - 再沿 `ctx+0xA8[ctx+0xA0]` 推进
    - step store 在 `0x801C9464`
  - tail 结束时：
    - `ctx+0xA0 = -1`
    - `ctx+0xA2 = 0`
    - `jal 0x80024390`
- later-cycle 对应：
  - overlap 上的 `student=9`
  - 以及 teacher hide 后的 `student=2..8`

### Lane Payload Sentinel Semantics (`-1` / `-2`)

- 这套 lane state machine 在 `0x801C9280..0x801C9474`（帧首 upkeep）和
  `0x801C9AB4..0x801C9E68`（本轮 event bootstrap 后立刻消费）各出现一次；
  两处对 payload sentinel 的语义是一致的。
- teacher / `ctx+0x94` (`laneA_ptr`)：
  - `-1` 是 live terminator。
    - `rowMode == 1`：
      - `0x801C9B60/0x801C9B64` 命中后落到 `0x801C9B74..0x801C9C98`
      - 写回：
        - `ctx+0x8C = -1`
        - `ctx+0x90 = 0`
      - 然后 `jal 0x80024308`
    - `rowMode == 2`：
      - `0x801C9BBC/0x801C9BC0` 命中时不会立即 hide teacher
      - 它先把 laneA phase 收尾成：
        - `ctx+0x8C = -1`
        - `ctx+0x8E = 2`
        - `ctx+0x90 = 2`
      - 之后改由 `ctx+0x98` (`laneB_ptr`) 继续跑；直到 `0x801C9C18/0x801C9C1C`
        命中 `-1` 才会：
        - `ctx+0x8E = -1`
        - `ctx+0x90 = 0`
        - `jal 0x80024308`
  - `-2` 只在 bootstrap 后检查 `laneA[0]`：
    - `0x801C9ABC..0x801C9AD4`
    - 触发效果不是清 teacher，而是强制清 student：
      - `ctx+0x9E = -1`
      - `ctx+0xA0 = -1`
      - `ctx+0xA2 = 0`
    - 这里不会直接调 reset helper。
- student / `ctx+0xA8` (`laneB_ptr`)：
  - 没有对称的 `-2` sentinel 检查。
  - student update block 只把
    - `cursor >= 19`
    - 或 payload byte `== -1`
    当成 generic lane-end。
  - `rowMode == 1` 的尾部在 `0x801C9D90..0x801C9E68`：
    - `ctx+0x9E = -1`
    - `ctx+0xA2 = 0`
    - `jal 0x80024390`
  - `rowMode == 2` 时：
    - `laneA` 结束：`0x801C9DEC..0x801C9E10`
      - `ctx+0x9E = -1`
      - `ctx+0xA0 = 2`
      - `ctx+0xA2 = 2`
    - `laneB` 结束：`0x801C9E48..0x801C9E68`
      - `ctx+0xA0 = -1`
      - `ctx+0xA2 = 0`
      - `jal 0x80024390`
- 当前 COMOD1 `0x801CFA54` 的 `36 x 44B` row 扫描也支持这条不对称结论：
  - 观测到的 `laneA` 样本在 index `9` 用 `-1` 收尾
  - 没有观测到 `laneA[0] == -2`
  - 也没有观测到任何 `laneB` `-1/-2` payload 字节

### Teacher / Student Row Handoff Rule

- 新的 replay 汇总见：
  - [psx_stage1_row_handoff_truth_20260412.json](../artifacts/psx_stage1_row_handoff_truth_20260412.json)
- 当前 captured windows 已经足够把 `teacher bootstrap` 和 `student bootstrap` 的 row ownership 关系压成两条明确规则：
  - teacher `idx==1`：
    - 会把 `ctx+0x94` 推进到新的 descriptor row
    - 而这同一时刻 student 仍可能停在上一轮 row 的尾段
    - captured samples：
      - `1157: teacherPtr=desc2, studentPtr=desc1`
      - `1287: teacherPtr=desc3, studentPtr=desc2`
      - `1417: teacherPtr=desc4, studentPtr=desc3`
  - student `idx==1`：
    - 在当前所有 captured windows 里，都直接拿“当前 teacher row pointer”
    - 它不是 `byte1D-1` 规则，也不是 replay 里独立存在的第三条 row cache
    - captured samples：
      - `1091: teacherPtr=desc1, studentPtr=desc1`
      - `1221: teacherPtr=desc2, studentPtr=desc2`
      - `1351: teacherPtr=desc3, studentPtr=desc3`
      - `1483: teacherPtr=desc4, studentPtr=desc4`
- 直接结论：
  - later-cycle 的 overlap 语义应记成：
    - teacher bootstrap 先把 teacher 切到新 row
    - student 继续跑上一轮 row 的尾段直到 hidden
    - 下一次 student bootstrap 再把 student 接到“当前 teacher row”
  - 这比“student bootstrap 取 `byte1D-1`”更符合 replay truth
  - 也进一步说明 `0x801C9CE0` 一带的伪 C 里 `a1[41]=0; a1[42]=0;` 不能按字面解释

### Implementation Consequence

- 这四段收口后，current Stage1 compact rail 的 producer 语义已经足够指导后续直抠：
  - teacher lane 不是“自己猜下一条 desc”
    - 而是 `0x801C99EC` bootstrap + `0x801C9334` hide tail
  - student lane 不是“body preview 借壳 materialize”
    - 而是 `0x801C9CE0` catch-up + `0x801C947C` steady/hide
- 所以接下来要继续抠的重点不再是 consumer/render，而是：
  - `0x801C9094` 这两套 lane state machine 如何被 event bytes 点燃

### Current Implementable Model (Stage1 stream1 only)

- 以当前 replay + static row table 证据，Stage1 stream1 的 compact interpreter 已可先按下面这条窄模型实现：
  1. teacher bootstrap (`flags04 & 0x800`)
     - `row = compactRows[byte1D]`
     - `teacher.rowMode = row.head0`
     - `teacher.cursorA = row.head1`
     - `teacher.cursorB = -1`
     - `teacher.state = 1`
     - `teacher.ptrA = row.laneA`
     - `teacher.ptrB = bodyRows[byte1E]`
     - 对当前 Stage1 stream1，又可直接收窄成：
       - `head0/head1` 恒为 `1/0`
       - `byte1E` 恒为 `0`
       - 所以等效于：
         - `teacher.rowMode = 1`
         - `teacher.cursorA = 0`
         - `teacher.ptrA = laneA(byte1D)`
         - `teacher.ptrB = laneB(0)`，但当前不会进入 `rowMode==2`
     - bootstrap 同帧马上跑一次 teacher update：
       - 若 `laneA[cursorA] != -1`，`cursorA++`
       - 所以 first visible onset 落在 `cursorA=1`
  2. teacher upkeep
     - `rowMode==1` 时，只沿 `ptrA` 推 `cursorA`
     - short rows (`1..8,10..17`) 会在 `idx9` 命中 `-1` 并自收尾
     - full combo rows (`19..26,28..34`) 没有 `-1`
       - 所以它们不是靠 payload sentinel 结束
       - 而是被下一条 bootstrap row 抢占 ownership
  3. student bootstrap (`flags04 & 0x100000`)
     - 不要按当前伪 C 的 `a1[41]=0; a1[42]=0;` 实现
     - 当前 replay truth 更稳的规则是：
       - `student.rowMode = teacher.rowMode`
       - `student.cursorA = 0`
       - `student.cursorB = -1`
       - `student.state = 1`
       - `student.ptrA = current teacher.ptrA`
       - `student.ptrB = current teacher.ptrB`
     - bootstrap 同帧马上跑一次 student update：
       - 若 `ptrA[cursorA] != -1`，`cursorA++`
       - 所以 student visible onset 同样落在 `cursorA=1`
  4. student upkeep
     - `rowMode==1` 时，只沿 `ptrA` 推 `cursorA`
     - 命中 `-1` 时 hidden + `0x80024390`
     - replay 上表现成：
       - teacher bootstrap 先切到新 row
       - student 跑完上一轮 row 尾段后 hidden
       - 下一次 student bootstrap 再接到“当前 teacher row”
- 这条模型刻意不先实现：
  - `rowMode==2`
  - `laneB` active payload
  - `laneA[0] == -2`
- 原因不是它们在引擎里不存在，而是当前 Stage1 stream1 证据明确显示它们不在活跃主链上。
- 这轮还把这套窄模型固化成了离线汇总工具：
  - [tools/stage1_compact_row_model.py](../tools/stage1_compact_row_model.py)
  - [tools/stage1_compact_model_validator.py](../tools/stage1_compact_model_validator.py)
  - [tools/stage1_compact_tick96_simulator.py](../tools/stage1_compact_tick96_simulator.py)
  - 默认产物：
    - [psx_stage1_compact_model_summary_20260412.json](../artifacts/psx_stage1_compact_model_summary_20260412.json)
    - [psx_stage1_compact_model_validation_20260412.json](../artifacts/psx_stage1_compact_model_validation_20260412.json)
    - [psx_stage1_compact_tick96_simulation_20260413.json](../artifacts/psx_stage1_compact_tick96_simulation_20260413.json)
  - 它会把三类证据合并到一页：
    - static row table 分类
    - stream1 的 teacher/student bootstrap rows
    - replay truth 的 handoff/cadence 摘要
  - validator 当前会自动检查：
    - row table 是否仍是 `head0=1/head1=0 + laneB all-zero + no -2`
    - teacher/student handoff 是否仍满足“teacher 先切新 row / student 接当前 teacher row”
    - `tick96 bridge` 是否仍落在当前已知小差值集合
    - bootstrap row 到紧随其后的 gameplay `0x30000` row 是否仍是 `+48 tick + same byte1D`
    - short cadence 的主峰是否仍是 `8`
  - `tick96 simulator` 当前则把“event-driven +24 tick nominal schedule”直接对到 replay truth：
    - teacher captured sequences 的误差全集是 `0..7 tick`
    - student captured sequences 的误差全集也是 `0..7 tick`
    - 这说明 direct parser 的下一层可以安全先在 `tick96` 域采用：
      - bootstrap 从 `event.frame` 起锚
      - visible cadence 先用 `+24 tick` nominal step
      - replay frame 只作为展示/验收域，而不是主仿真域
  - 当前输出也再次确认：
    - teacher short cadence 的主峰是 `8`
    - student short cadence 的主峰也是 `8`
    - `10` 只出现在少数首段/边界窗口
    - bootstrap sequence 摘要也已经直接导出：
      - teacher:
        - `1025`: `[10,8,8,8,8,8,8,8,8]`
        - `1157`: `[8,8,8,8,8,8,10,6,8]`
        - `1287`: `[8,8,8,10,8,8,8,6,10]`
        - `1417`: `[10,8,8,8,8,8,8,8,8]`
      - student:
        - `1221`: `[8,8,10,8,8,8,8,8,8]`
        - `1351`: `[10,8,8,8,8,8,8,8,10]`
    - 直接实现含义仍不变：
      - common cadence = `8`
      - `6/10` 只应视为 bootstrap / handoff 边界上的局部 edge case
    - bootstrap 的 `tick96 bridge` 也已经直接对上：
      - teacher:
        - `1025 -> ctxTick96 2640 -> event.frame 2640 (Δ=0)`
        - `1157 -> 3027 -> 3024 (Δ=3)`
        - `1287 -> 3409 -> 3408 (Δ=1)`
        - `1417 -> 3792 -> 3792 (Δ=0)`
      - student:
        - `1091 -> 2835 -> 2832 (Δ=3)`
        - `1221 -> 3217 -> 3216 (Δ=1)`
        - `1351 -> 3600 -> 3600 (Δ=0)`
        - `1483 -> 3987 -> 3984 (Δ=3)`
    - source-table 侧的 bootstrap follow-up 规则也已压实：
      - `0x800 -> next 0x30000`：
        - `Δtick = +48`
        - `byte1D` 不变
        - 当前 `24/24` 全命中
      - `0x100000 -> next 0x30000`：
        - `Δtick = +48`
        - `byte1D` 不变
        - 当前 `31/31` 全命中
      - `0x20800` 作为 late teacher-bootstrap 变体，也仍满足这条 `+48 + same byte1D`
      - `0x21000` 不是 bootstrap row，不参与这条规则
    - 所以后续若要把 source-table bootstrap 和 replay truth 自动对位，已经可以先在 `tick96` 域做窄匹配，不需要发明 replay-frame 直换公式
  - 2026-04-13 进一步把这套规则落成了 source-table 直解析器：
    - [tools/stage1_direct_compact_parser.py](../tools/stage1_direct_compact_parser.py)
    - 默认产物：
      - [psx_stage1_direct_compact_parser_20260413.json](../artifacts/psx_stage1_direct_compact_parser_20260413.json)
    - 当前它直接从：
      - `stage1_stream1_event_table_export_v2_20260412.json`
      - `psx_stage1_compact_row_table_20260412.json`
      - 生出 teacher/student compact state timeline
    - 当前直接解析结果已经压实：
      - teacher bootstrap count = `31`
      - student bootstrap count = `31`
      - bootstrap follow-up `62/62` 全命中：
        - `Δtick = +48`
        - `same selector = true`
      - teacher timeline = `31` 条 sequence / `310` 个 state changes
      - student timeline = `30` 条 sequence / `300` 个 state changes
      - teacher bootstrap flags 分布：
        - `0x800 x24`
        - `0x20800 x7`
      - student bootstrap flags 分布：
        - `0x100000 x30`
      - row kind 分布：
        - teacher `short x16 / full_combo x15`
        - student `short x16 / full_combo x14`
      - 和 replay truth 对比后的误差全集仍保持：
        - teacher `0..7 tick`
        - student `0..7 tick`
    - 这意味着接下来做 Win 侧 direct-port 时，已经不需要再靠中间 summary 手工拼：
      - source-table -> parser -> actor timeline
      - 这条最小闭环已经具备可复跑产物
  - 2026-04-13 还把 teacher-side chrome/light 也接回了这条闭环：
    - [psx_stage1_compact_chrome_tables_20260413.json](../artifacts/psx_stage1_compact_chrome_tables_20260413.json)
    - [tools/stage1_compact_chrome_model.py](../tools/stage1_compact_chrome_model.py)
    - [psx_stage1_compact_chrome_model_20260413.json](../artifacts/psx_stage1_compact_chrome_model_20260413.json)
    - 当前直接导出的 PSX 阈值表：
      - large = `[56, 116, 176, 236]`
      - small = `[30, 45, 75, 90, 105, 135, 150, 165, 195, 210, 225, 255, 270, 285]`
    - 对 row0 teacher lane，raw -> light count 已经可复跑：
      - `raw=1 -> large 0 / small 2`
      - `raw=2 -> large 1 / small 2`
      - `raw=6 -> large 2 / small 5`
      - `raw=9 -> large 2 / small 8`
    - 也就是缺失的 bar light 后续不该再按视觉补丁修：
      - 应直接接到 `teacher raw compact value` 的 PSX 阈值比较
  - note/body + wobble chrome 这轮也开始从 PSX source 侧闭环：
    - note template 表：
      - [psx_stage1_note_template_table_20260413.json](../artifacts/psx_stage1_note_template_table_20260413.json)
    - note/body symbol 摘要：
      - [tools/stage1_compact_note_symbol_model.py](../tools/stage1_compact_note_symbol_model.py)
      - [psx_stage1_compact_note_symbol_model_20260413.json](../artifacts/psx_stage1_compact_note_symbol_model_20260413.json)
    - wobble exact sequence：
      - [psx_stage1_compact_wobble_sequences_20260413.json](../artifacts/psx_stage1_compact_wobble_sequences_20260413.json)
    - 当前已钉死的 direct-port 规则：
      - `type 1..4` = `triangle/circle/cross/square`
      - `type 5/6` 复用 `left prompt`
      - `type 7/8` 复用 `right prompt`
      - 当前 active Stage1 rows 只观测到 `1/2/3/4/5/7`
      - bank-A 的 phase machine = `linear(0..5) -> rsin(6..21) -> rest(22..23)`
      - bank-B 的 phase machine = `linear(0..4) -> rsin(5..21) -> rest(22..23)`
    - 这意味着 Stage1 rail 的 note/body/flip 也已经不再只是“视觉现象”：
      - 它们现在已有 source-table / template-table / updater-bank 三层可复跑规格
  - 2026-04-13 还把这些分层 artifact 合并成了 direct-port bundle：
    - [tools/stage1_direct_compact_port_bundle.py](../tools/stage1_direct_compact_port_bundle.py)
    - [psx_stage1_direct_compact_port_bundle_20260413.json](../artifacts/psx_stage1_direct_compact_port_bundle_20260413.json)
    - 当前 bundle 已经按 selector 统一打包：
      - `note_layout`
      - `teacher_sequence`
      - `student_sequence`
      - `raw_value -> chrome count`
      - `bankA/bankB wobble sequence`
    - 摘要值：
      - selector count = `36`
      - teacher sequence count = `31`
      - student sequence count = `30`
      - active note type histogram = `{1:8, 2:9, 3:6, 4:8, 5:12, 7:10}`
      - current rows unused type codes = `[6, 8]`
    - 这份 bundle 已经足够作为后续 Win 侧 Stage1 rail direct-port 的规格输入，而不是继续靠散落的证据文件手工拼接
  - 用户额外强调的多分辨率约束，这轮也已经提前收进 geometry spec：
    - [psx_stage1_compact_geometry_model_20260413.json](../artifacts/psx_stage1_compact_geometry_model_20260413.json)
    - [psx_stage1_direct_compact_port_bundle_20260413.json](../artifacts/psx_stage1_direct_compact_port_bundle_20260413.json)
    - 当前 bundle 同时携带：
      - PSX `320x240` 像素坐标
      - 归一化坐标 `x/320`, `y/240`
    - 规范里已明确：
      - Win 侧高分辨率适配只能做统一 viewport scaling
      - 不允许为了适配分辨率修改任何 PSX 几何公式
  - 2026-04-13 这条 bundle 又继续长成了 draw-plan：
    - [tools/stage1_direct_compact_draw_plan.py](../tools/stage1_direct_compact_draw_plan.py)
    - [psx_stage1_direct_compact_draw_plan_20260413.json](../artifacts/psx_stage1_direct_compact_draw_plan_20260413.json)
    - 当前 draw-plan 已经统一输出：
      - selector-local row0 draw templates
      - captured tick96 draw snapshots
      - render order / OT semantics
    - 当前 render-order 规则写死为：
      - call sequence = `teacher_portrait -> student_portrait -> marker -> note`
      - visible OT order = `portrait(0) -> note(1) -> marker(3)`
    - 当前 captured tick plan 数量 = `520`
    - 也就是 Stage1 rail 已经不只是“direct-port 规格包”：
      - 现在还多了一份直接可消费的 PSX draw-plan
  - 2026-04-13 晚些时候，Win 侧已经开始接第一刀 direct-port：
    - 代码入口改在 [pr_scenes.cpp](../src/pr/pr_scenes.cpp)
    - 当前 `ResolveStage1Scene1NativeRailConsumerSnapshot()` 已不再从旧的 rail cursor / gameplay rail authority 回填 snapshot
    - 新路径直接用：
      - `stream1 authored events`
      - `0x801CFA54 + 44*selector` compact row table
    - 当前 producer 规则落地为：
      - teacher bootstrap = `0x800 / 0x20800`
      - student bootstrap = `0x100000`
      - 当前 active direct-port 仍把 `event.frame` 喂给 compact actor 的 `queryTick96` 时基做比较
      - `raw = 1 + floor((queryTick96 - startFrame) / 24)`，上限按 `kStage1RailPortraitCursorCount - 1`
      - teacher hide = `same-selector student bootstrap + 24`
      - student hide = `next teacher bootstrap with larger selector + 24`
      - `visible_selector = teacher.active ? teacher.selector : student.selector`
      - direct-port row-kind 补充：
        - `laneA_first_neg1 == 9` 的 `short` row：继续按 row 内 `-1` / handoff 求 hide
        - `laneA_first_neg1 == null && laneA 有可见 payload` 的 `full_combo` row：当前只补 `tail hide = startFrame + 17*24`
        - raw 仍继续按 portrait cursor 上限推进，不在 runtime 里额外钳死到 `9`；这样不会把后面本来应该越过旧 bar 上限的窗口再卡住
      - render-only 所需的 `renderPortraitSample / shadowPortrait*` 也已开始由 direct-port producer 自产：
        - 当前直接取 `queryTick96 + 3` 的 next logic sample
        - 不再强依赖旧 rail cursor 的 next sample 镜像
      - 2026-04-13 下午尝试把这里改成 `queryFrame30/+1`，但 overlap runtime 直接整条 rail 不可见，说明真实 source-table 查询时基不是裸 `queryFrame30`
      - 当前结论：
        - 不能再在 `queryFrame30` 和 `runner.GetTick96()` 之间继续拍脑袋二选一
        - 必须先补齐“Stage1 compact authored frame 在 Win runtime 里的真实桥接时基”证据，再继续改 late-gap bug
      - portrait anchor 现在也开始直接吃 producer `rollFrame`：
        - live portrait 用 `livePortraitRollFrame`
        - render-only 且 next sample 同 raw 时，用 `renderPortraitSampleRollFrame`
        - UI 本地 `portraitHold/renderPortraitHold` 已不再参与 portrait X 计算
      - 这一步的目的就是把 Win 本地 hold 重建从主头像位置里摘掉，减少“小头像走走停停”的宿主污染
      - 同轮还把 current/next payload overlay 重新接回 snapshot：
        - 当前 direct rule 是：
          - `current = first visible body slot >= activeRaw`
          - `next = first visible body slot after current`
        - body/payload overlap 会在 UI consumer 里先 suppress，再画 payload overlay
      - 2026-04-13 进一步收紧 overlap/body ownership：
        - visible body row 不再走 `student current + teacher next` 的双 row 壳
        - 当前 direct producer 改成 single-selector、teacher-first：
          - `teacher.raw > 0` 时，visible body row 直接取 `teacher.selector`
          - 只有 teacher 不可见时，才回退到 `student.selector`
        - `current/next` 现在都只从同一条 visible body row 内推导：
          - 不再跨 row 伪造 `next = teacher row head`
          - overlap 的 `9/1` 窗口里，visible body row 与 `current/next` 仍归 teacher row
          - `hidden/2` 后再整体切到 student row
        - `accepted` 在当前 cut 里继续保持 hidden，不重新引入旧的 endpoint handoff 壳
        - `teacherOwnedVisualPhase` 也已改成 single visible row owner 语义，不再等同“只要 rail 上有任何 actor/body 就 true”
      - 2026-04-13 随后又把 consumer 侧 note/body wobble 改回 PSX shared-table 语义：
        - `teacherFlip` / `studentFlip` 仍各自推进本地 bank
        - 但 `TryResolveStage1Scene1NativeRailSymbolAnimScale()` 不再按 `symbol.teacherOwned` 直接选 bank
        - 现在按 slot ordinal 合成最终 shared scale：
          - teacher bank 先写
          - student bank 后写；若两边同 slot 同时 active，则 student 覆盖 teacher
        - 这样 later-cycle overlap 里的 student after-write flip 不会再被 teacher-owned visible row 吃掉
        - 同轮也补齐了 slot1/student 的 compact phase lifecycle：
          - `PrStage1Scene1NativeRailConsumerSnapshot` 现在也发布 `studentCompactPhaseAvailable/currentEnable/phaseKey/cadenceStartFrameUsed`
          - UI anim state 也按 `slot1CompactPhaseKey` reset `studentFlip/studentNext*`
        - 这一刀的目的就是修“学生第一次滚动能翻，第二次及之后不翻”的 direct-port 残留
      - 同轮还修掉了 teacher marker chrome 的 UI 清零错误：
        - `BuildStage1GameplayStripFromScene1ChartBody()` 之前先把 `teacherLargeMarkersOn/teacherSmallMarkersOn` 写进 `outStrip`
        - 但紧接着 `BuildStage1GameplayStripFromScene1NativeConsumerSnapshot()` 又整结构清零，导致 active native marker chrome 读到全 `false`
        - 现在 marker bool 在 native consumer build 之后再写回 `outStrip`
        - fresh overlap probe 已直接证明这条 producer->consumer 链恢复：
          - `q=505`: `teacherSmallMarkersOn=[1,1,0,...]`
          - `q=514`: `teacherLargeMarkersOn=[1,0,0,0]`, `teacherSmallMarkersOn=[1,1,1,0,...]`
      - 随后又把 active Scene1 native strip 上的 endpoint payload 渲染彻底停掉：
        - `BuildStage1GameplayStripFromScene1NativeConsumerSnapshot()` 不再把 `rail.payloadSymbols` 复制成 `endpointPayloadSymbols`
        - 这意味着 `current/next` 仍保留在 snapshot/debug 层，但不再作为第二条 Win 渲染路线盖在 body row 上
        - active 画面现在只渲染：
          - single visible body row
          - portraits
          - marker chrome
        - 与这条切线一起删掉的 dead helper：
          - body/payload overlap suppress
          - payload-only current detector
          - endpoint payload append helper
        - 这一步的目的就是把“同一条 PSX row 再走一遍 endpoint overlay”的 Win scaffolding 从 active strip 上彻底摘掉
      - 2026-04-13 随后又把 producer 侧对应壳一起收掉：
        - `BuildStage1Scene1NativeRailRenderSymbols()` 不再生成 `payloadSymbols`
        - `activationContent` 现在只看 `bodySymbolCount > 0`
        - `teacherOwnedPayloadOnlyCurrent` 在当前 direct-port cut 下固定回到 `false`
        - 这样 active native rail 不会再因为旧 endpoint payload 壳而提前 visible / 提前开 portrait gate
      - 同轮尾部又做了一次 producer/consumer 清理：
        - `BuildStage1Scene1NativeRailRenderSymbols()` 现在直接吃 `teacherOwnedVisibleRow`
        - 不再先把 `bodySymbols[].teacherOwned` 写成假值后再回填
        - `pr_ui_overlay.cpp` 里旧的
          - `AppendStage1GameplayStripNativeBodyFromStream`
          - `HasStage1GameplayStripVisibleBodyPayloadInStream`
          已删除
        - 这属于 direct-port 切线周围的死代码清理，不改既定可见语义
      - 2026-04-13 继续补回 active note 强调：
        - endpoint 壳摘掉后，active strip 上的 bead/glow/token emphasis 仍由 `bodySymbols[].active` 驱动
        - 现在 `BuildStage1Scene1NativeRailRenderSymbols()` 会把 `currentVisibleBodyIndex` 对应的 body symbol 标成 `active=true`
        - 也就是当前 visible row 上真正的“current note”重新回到 body row 本体上，而不是依赖已经移除的 endpoint/current 路线
      - 同轮又把两个 active native render-only 残留收掉了：
        - `GetStage1GameplayStripScene1NativeTokenCenterY()` 不再保留 `teacherOwned` 的 fallback Y 偏移
        - active native note/body token center 现在直接固定回 PSX row0 `y=24`
        - `CanInterpolateStage1NativeRailSymbol()` 也不再把 `phraseStart/phraseEnd` 当作 interpolation identity 条件
        - 因为 active native sprite path 本身不消费 phrase 装饰，所以这条 metadata 不应继续无意义地打断 render-only 连续性
      - `RenderStage1GameplayScene1NativeRail()` 入口也已不再读取旧 `GetStage1RailCursorSnapshot()` 做 shadow/debug：
        - render debug 和 shadow cursor 现在直接来自 native snapshot 自身的 `renderPortraitSample*`
    - 这一刀只替换 snapshot producer，不动 [pr_ui_overlay.cpp](../src/pr/pr_ui_overlay.cpp) 的现有 render consumer
    - 因而当前切换边界是：
      - authority / row selection / portrait raw cursor 来自 direct-PSX producer
      - portrait hold / render-only interpolation 仍暂时留在 UI consumer
    - 同轮已做串行全量编译，产物仍为：
      - `E:\game\PSgame\parappa the rapper\PaRappaWin.exe`
    - 这一步没有跑自动测试；后续主要靠用户视觉回归继续收 Stage1 rail 剩余偏差
  - 2026-04-13 又把 `stage1railprobe` / `test_dispatcher.py` 的 full-song gate 切到了 native direct-port 口径：
    - `stage1railprobe` 现在额外输出：
      - `nativeRailConsumer.slot0CompactValueRaw/PhaseAvailable/CurrentEnable`
      - `nativeRailConsumer.slot1CompactValueRaw/PhaseAvailable/CurrentEnable`
      - `nativeRailConsumer.slot0PortraitVisible/slot1PortraitVisible`
      - `nativeRailConsumer.slot0EffectiveCursor/slot1EffectiveCursor`
    - `test_dispatcher.py::_parse_stage1_rail_probe()` 现在保留：
      - `cursorTeacher/cursorStudent`
      - `nativeTeacher/nativeStudent`
    - 并把聚合 `teacher/student` 默认切到 native slot raw；只有 native 字段缺失时才回退旧 cursor
    - 这样 Stage1 rail runtime gate 现在验证的是 active direct-port rail，而不是旧 cursor shell
  - 这轮 no-fail 全段验证不再使用会让用户误判成“失败重进”的 aggregate wrapper，而是主代理串行跑 5 个 delegated segment：
    - overlap:
      - [20260413_100651_stage1_rail_overlap_runtime/summary.txt](../artifacts/stage1_rail_probe_runtime/20260413_100651_stage1_rail_overlap_runtime/summary.txt)
    - later-cycle:
      - [20260413_100542_stage1_rail_later_cycle_runtime/summary.txt](../artifacts/stage1_rail_probe_runtime/20260413_100542_stage1_rail_later_cycle_runtime/summary.txt)
    - repeat:
      - [20260413_100923_stage1_rail_repeat_runtime/summary.txt](../artifacts/stage1_rail_probe_runtime/20260413_100923_stage1_rail_repeat_runtime/summary.txt)
    - late-gap:
      - [20260413_101053_stage1_rail_late_gap_runtime/summary.txt](../artifacts/stage1_rail_probe_runtime/20260413_101053_stage1_rail_late_gap_runtime/summary.txt)
    - tail:
      - [20260413_101224_stage1_rail_tail_runtime/summary.txt](../artifacts/stage1_rail_probe_runtime/20260413_101224_stage1_rail_tail_runtime/summary.txt)
  - 当前结论：
    - `debugStage1NoFail=1` 下，完整 Stage1 rail/full-chart 的五段轻量 runtime gate 均已通过
    - 用户看到的“full-flow 前半段失败后反复关游戏重进”并不等价于 rail 行为失败：
      - 先前一部分是 aggregate wrapper 的 segment restart 设计
      - 另一部分是 later-cycle summary 仍在读旧 cursor teacher/student，而不是 active native direct-port slot raw
  - 2026-04-13 随后又把 Stage1 rail gap/hide 和 long-row portrait cursor 收到了更接近 PSX 的 direct-port 语义：
    - PSX replay + 现有 compact row/static truth 共同说明，段落空档不是“等下一个 bootstrap 才隐藏”，而是当前 compact lane 自己命中 row tail `-1/end` 后就退场：
      - teacher/student hide frame 现在按 `ReadStage1RailCursorRec44(...).streamA` 的 sentinel 计算 lane tail，再与 bootstrap handoff `+24 tick` 取较早者
      - 这一步修掉了“段落间没有音符时旧 rail 仍挂在屏幕上”的 direct-port 偏差
    - 同时，PSX 内存回放已直接证实 `teacherIdx8C/studentIdx9E` 会超过 `9`：
      - `0x801C36CC` 在录制 `stage1二周目_无cool_good评级通关3.psxrec` 中至少出现到 `15`
      - `0x801C36DE` 至少出现到 `16`
      - 因而 Win 侧 `ResolveStage1Scene1DirectCompactRawValueAtQuery()` 早先的 `1..9` clamp 是错误的短-row 假设
      - 现在 direct-port raw cursor 已改为按 `kStage1RailPortraitCursorCount-1`（当前为 `18`）上限推进，不再卡在第三个 bar
    - full-stage harness 也已从“只停在 rail tail”升级为“完整走完 Scene1 clear/exit path”：
      - `debugStage1NoFail=1` 只 suppress fail，不会让无输入 AWFUL run 自然 clear
      - 所以 `--stage1-rail-full-song-runtime` 现在会先跑过 authoritative rail tail，再用现成 debug hook `f5StageClear` 驱动 Scene1 clear 分支，验证完整 lifecycle 退出
      - 当前通过结果：
        - [20260413_132233_stage1_rail_full_song_runtime/summary.txt](../artifacts/stage1_rail_probe_runtime/20260413_132233_stage1_rail_full_song_runtime/summary.txt)
        - `stopReason=stage_clear_terminal`
        - `maxQuery=1294`
  - 2026-04-13 随后把 Stage1 fail threshold / fail ending 的权威验证也锁到了 formal lifecycle：
    - 先前的 `--fail` 只是看 smoke/meter 一类表象，不足以证明 Stage1 fail gate 与 PSX 对齐
    - 这轮改成直接验证 Scene1 formal lifecycle runtime：
      - 权威 fail 轴 = `rightRankActiveRow == 3`
      - 权威 fail hold = `kStage1FormalLifecycleAwfulFailHoldFrames == 60`
      - `debugStage1NoFail=1` 会 suppress fail gate；fail runtime 必须显式关掉 no-fail
      - Scene1 fail path 仍是 `Scene1::Fn2` fail 分支 `return 0` 回 Scene0
    - `PrStage1FormalLifecycleSnapshot` 已补齐：
      - `rightRankActiveRow`
      - `awfulHoldFrames`
      - `awfulHoldFramesRequired`
    - 同轮新增了 Scene1 terminal fail latch：
      - `s_stage1TerminalFormalLifecycleSnapshot`
      - clear / init 时 reset
      - `Scene1::Fn2` fail 分支在 `resetStage1LoopRuntime(); return 0;` 之前先 latch
      - 导出接口：`PrScn1::GetTerminalFormalLifecycleSnapshot(...)`
    - 新增调试命令：
      - `stage1ovl lifecycle`
      - 同时输出 live snapshot 与 terminal latched snapshot：
        - `valid/query`
        - `clearGate/failGate`
        - `rightRankActiveRow`
        - `awfulHoldFrames`
        - `awfulHoldFramesRequired`
    - 之所以不能只靠 `stage1ovl handoffhistory`：
      - handoff history sample 发生在 `PrMain::Run(...)` 返回之后
      - Stage1 fail 会在同帧内退出 Scene1
      - 所以 history 常只能抓到最后一帧 `awfulHoldFrames=59, failGate=0`
      - 真正的 `hold=60/failGate=1` 需要 terminal latch 才能稳定保留
    - `test_dispatcher.py --fail` 现在的断言已经切到 formal lifecycle：
      - gameplay 中轮询 `stage1ovl handoff`
      - Scene0 返回后读取 `stage1ovl lifecycle`
      - 强制要求：
        - terminal snapshot `valid=1`
        - `failGate=1`
        - `clearGate=0`
        - `rightRankActiveRow=3`
        - `awfulHoldFramesRequired=60`
        - `awfulHoldFrames=60`
        - 最后一条 pre-exit handoff history sample 仍停在 AWFUL row，且 `awfulHoldFrames=59`
    - 当前权威结果：
      - `python .\\test_dispatcher.py --fail`
      - 通过；关键输出：
        - `Scene1::Fn2 stage failed`
        - `stage1-fail-runtime: firstFailQuery≈761..763 awfulHoldFrames=60 historyScene0Return=1`
  - 2026-04-13 随后补上了真正单 session 的 no-fail full-song rail validator：
    - 入口：
      - `python .\\test_dispatcher.py --stage1-rail-full-song-runtime`
    - 与旧 `--stage1-rail-full-flow-runtime` 的区别：
      - `full-flow` 仍是 delegated segmented wrapper
      - `full-song` 才是一次启动、一次进关、不中断跑完整条 Stage1 rail 的单 session 验证
    - 这段是较早的历史阶段记录，后续已被“继续走到 `stage_clear_terminal` / `scene2_after_stage_clear`”的新 full-song harness 取代
    - 当时 `full-song` 的临时停止条件不是 `Scene2 clear`，而是 **rail authoritative tail complete**
      - harness 会持续采样直到 `queryFrame > 1290`
      - 然后立即结束 capture 并主动杀进程
      - 不再等待 Scene clear / Scene2 切换
    - 这条边界是故意收出来的，不是妥协：
      - `stage1NoFail=1` 只 suppress fail gate，不会替坏局面补一个 clear route
      - 第一次错误实现里，我把 no-fail full-song 写成“等 Scene2 clear”
      - 结果 Stage1 在 `rightRankActiveRow=3` 下继续无限跑，`awfulHoldFrames` 累到 `2000+`
      - 最终在 `queryFrame≈2880` 进入未定义后段并触发：
        - `UNHANDLED EXCEPTION code=0xC0000005`
      - 这也是为什么曾经先收了一版“rail tail 即停”的 no-fail harness；但当前 active harness 已在保住稳定性的前提下继续验证到 clear terminal
    - 当前通过结果：
      - [summary.txt](../artifacts/stage1_rail_probe_runtime/20260413_111050_stage1_rail_full_song_runtime/summary.txt)
      - `maxQuery=1294`
      - `stopReason=rail_tail_complete`
      - overlap / later-cycle / repeat / late-gap / tail 全部 marker 都在同一次 uninterrupted run 中命中
    - 为避免 cleanup 反向污染结果：
      - `full-song` harness 现在先杀游戏，再清 `stage1NoFail`
      - 不再出现“capture 已完成，但 cleanup 又把 no-fail 关掉，随后补出 terminal fail”这类假阴性
    - 同轮 `Scene1::Fn2` clear 分支也改成了与 fail 对称的 terminal lifecycle latch：
      - clear 不再 reset terminal snapshot
      - 现在会先 `LatchStage1TerminalFormalLifecycleSnapshot(...)`
      - 虽然当前 `full-song` 不靠它停表，但后续若要做真实 clear 路径验证，终态证据链已对称
