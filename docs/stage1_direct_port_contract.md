# Stage1 Direct-Port Contract

日期：2026-04-13  
范围：把当前已收口的 PSX Stage1 compact rail 规格整理成 Win 侧 direct-port 合同。  
目标：后续 Win 代码只消费这份输入/输出合同，不再从零散 artifact 或旧兼容壳里临时摘规则。

## 1. Source Inputs

Win 侧 direct-port 应只依赖两类 PSX source：

- stream1 authored event table
  - 当前导出见 [stage1_stream1_event_table_export_v2_20260412.json](../artifacts/stage1_stream1_event_table_export_v2_20260412.json)
- compact row table
  - 当前导出见 [psx_stage1_compact_row_table_20260412.json](../artifacts/psx_stage1_compact_row_table_20260412.json)

不要把 overlay 运行期结果硬编码回 Win。

## 2. Parser Contract

最小 parser 输出应分成三层：

1. actor state timeline
   - teacher/student
   - `selector`
   - `raw value`
   - `tick96`
   - hidden/materialize/handoff

2. row-local note/body layout
   - `selector -> laneA payload`
   - `slot`
   - `type code`
   - `template ptr`

3. chrome runtime state
   - teacher raw -> large/small light count
   - slot ordinal -> wobble bank output

当前对应产物：

- [psx_stage1_direct_compact_parser_20260413.json](../artifacts/psx_stage1_direct_compact_parser_20260413.json)
- [psx_stage1_compact_note_symbol_model_20260413.json](../artifacts/psx_stage1_compact_note_symbol_model_20260413.json)
- [psx_stage1_compact_chrome_model_20260413.json](../artifacts/psx_stage1_compact_chrome_model_20260413.json)
- [psx_stage1_compact_wobble_sequences_20260413.json](../artifacts/psx_stage1_compact_wobble_sequences_20260413.json)

## 3. Event Grammar

当前 direct-port 必须保留这些 stream1 语义：

- teacher bootstrap:
  - `flags04 = 0x800` 或 `0x20800`
- student bootstrap:
  - `flags04 = 0x100000`
- gameplay row:
  - `flags04 = 0x30000`
- bootstrap -> next gameplay:
  - `+48 tick`
  - `same byte1D`

当前证据链已钉死：

- `0x800 -> next 0x30000`: `24/24`
- `0x100000 -> next 0x30000`: `31/31`
- `0x20800` 仍满足这条规则

## 4. Row Model

当前 Stage1 active compact 主链只使用：

- `rowMode == 1`
- `laneA`
- `selector 0..35`

当前不要把下面这些混回主线：

- `rowMode == 2`
- `laneB`
- `laneA[0] == -2`

row family 的已知边界：

- short rows:
  - `laneA_first_neg1 = 9`
- full combo rows:
  - 无 `-1`
  - 结束依赖下一条 bootstrap 抢占 ownership

## 5. Symbol / Template Contract

`type code -> symbol/template`：

- `1 -> triangle`
- `2 -> circle`
- `3 -> cross`
- `4 -> square`
- `5/6 -> left prompt`
- `7/8 -> right prompt`

当前 active Stage1 rows 只观测到：

- `1/2/3/4/5/7`

当前未观测到但模板表已存在：

- `6/8`

## 6. Chrome Contract

teacher-side bar/light 不吃 portrait hold。

公式：

- `teacherProgressX = raw > 0 ? 15*raw + 31 : 0`

阈值：

- large:
  - `[56, 116, 176, 236]`
- small:
  - `[30, 45, 75, 90, 105, 135, 150, 165, 195, 210, 225, 255, 270, 285]`

row0 样本：

- `raw=1 -> large 0 / small 2`
- `raw=2 -> large 1 / small 2`
- `raw=6 -> large 2 / small 5`
- `raw=9 -> large 2 / small 8`

## 7. Wobble Contract

两套 updater bank 的 exact sequence 已收口：

- bank-A:
  - `linear(0..5) -> rsin(6..21) -> rest(22..23)`
- bank-B:
  - `linear(0..4) -> rsin(5..21) -> rest(22..23)`

reset 初值：

- `counter=0`
- `sin_phase=0`
- `linear_acc=2048`
- `linear_vel=2048`

Win 侧不要用宿主 `sin()` 近似替代这套 phase machine。

## 8. Geometry Contract

几何真值必须保留 PSX `320x240` 语义。

当前公式：

- note center:
  - `x = 32 + 15*slot`
  - `y = 24 + 20*row`
- teacher portrait top-left:
  - `x = 15*raw + 26 + 4*hold`
  - `y = 18 + 20*row`
- student portrait top-left:
  - `x = 15*raw + 26 + 4*hold`
  - `y = 16 + 20*row`
- large marker center:
  - `x = threshold_x + 6`
  - `y = row_large_top_left_y + 6`
- small marker center:
  - `x = threshold_x + 4`
  - `y = row_small_top_left_y + 4`

row-local Y：

- large rows = `[18, 38]`
- small rows = `[22, 41]`

## 9. Multires Rule

Win 侧允许做分辨率适配，但只允许这一种方式：

- 保持所有 PSX 坐标公式不变
- 把 `320x240` 坐标统一做 viewport scaling

不允许：

- 为了高分辨率重写 PSX 几何公式
- 为了填满屏幕去改 marker / portrait / note 相对位置
- 为了避免像素误差再加一层“分辨率特判补丁”

## 10. Draw-Plan Contract

当前最终 direct-port 输入见：

- [psx_stage1_direct_compact_port_bundle_20260413.json](../artifacts/psx_stage1_direct_compact_port_bundle_20260413.json)
- [psx_stage1_direct_compact_draw_plan_20260413.json](../artifacts/psx_stage1_direct_compact_draw_plan_20260413.json)
- [psx_stage1_direct_port_contract_bundle_20260413.json](../artifacts/psx_stage1_direct_port_contract_bundle_20260413.json)

最小 draw item 字段应包含：

- `kind`
- `selector`
- `tick96`
- `raw_value`
- `type_code`
- `template_ptr_hex`
- `anchor_mode`
- `x_psx/y_psx`
- `x_norm/y_norm`
- `scale_x/scale_y`
- `scale_source`
- `ot_bucket`
- `sort_bucket`
- `submission_stage`
- `submit_seq`
- `visible_order_front_to_back`
- `source_ref`

当前 contract 对 `scale_x/scale_y` 的边界是：

- portrait / marker：
  - 已可直接写成 `4096 / 4096`
- note/body：
  - 当前不伪造 draw-time scale 数值
  - 先通过 `scale_source = shared_slot_scale_table(slot_ordinal)` 明确回指：
    - `word_80087668`
    - `word_800876B0`
  - 等后续把 slot-scale 的动态 lifecycle 完整接到 draw-plan，再补成数值

render order 需要同时保留两套语义：

- call sequence:
  - `teacher_portrait -> student_portrait -> marker -> note`
- visible OT order:
  - `portrait(0) -> note(1) -> marker(3)`

## 11. Cutover Rule

当切回 Win 实现时，顺序应是：

1. 解析 source table / compact rows
2. 生成 actor timeline
3. 生成 note/chrome/wobble/geometry state
4. 生成 draw-plan
5. 渲染层只消费 draw-plan

不要反过来从旧 Win consumer 里回填 authority。

## 12. Machine-Readable Entry

如果 Win 侧实现需要一个统一入口，优先直接读取：

- [psx_stage1_direct_port_contract_bundle_20260413.json](../artifacts/psx_stage1_direct_port_contract_bundle_20260413.json)

它已经把当前 contract 的核心字段合并成：

- source inputs
- event grammar
- row model
- symbol contract
- geometry contract
- chrome contract
- wobble contract
- draw-plan contract
- cutover rule
