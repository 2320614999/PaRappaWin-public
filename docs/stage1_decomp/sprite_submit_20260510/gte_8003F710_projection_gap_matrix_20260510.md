# 8003F710 projection gap matrix - 2026-05-10

范围：GTE-C2-static。只整理 `8003F710` packed SXY / SZ FIFO / FLAG / IR0 / return 证据，不改 `src`，不编译、不测试、不跑模拟器。

结论：`8003F710` 的 PSX 调度和 caller 写包位置已足够清楚，但投影数值语义仍未闭合。下一轮可以实现“最小 GTE projection helper”的输入、调度、部分 SZ/division trace；`SXY packed word`、`FLAG bits`、`IR0`、`returnValue known` 仍必须保持 gap，直到补齐 IDA/动态真值或权威 GTE 公式验证。不能用 Win float 投影坐标替代。

## 已确认静态边界

证据来源：

- `gte_8003F710_direct_notes_20260510.md`：当前 helper 明确只记录 RTPT/RTPS carrier，`known=false`，不冒充 SXY/FLAG/IR0/return。
- `rail_gte_next_cutover_plan_20260510.md`：`8003F1B4` transform path 已把四个 local vertex 交给 `8003F710`，packet 的 `word2/4/6/8` 等待 SXY 输出。
- `ida_export_8003F710_rottranspers4_followup.txt` / `ida_export_8003F1B4_gte_helpers.txt`：`8003F710` exact region 与 `8003F1B4` xref。
- `ida_export_gte_register_owner_scan.txt`：GTE register owner scan 中确认相关 control/data register 读写点。
- `src/pr/pr_psx_gte_direct.*`：当前 Win direct helper 的 gap state 和 trace 字段。
- `src/pr/pr_psx_gs_sprite_submit_direct.*`：`8003F1B4` transform prelude、vertex 构造、packet gap 消费点。

`8003F710` 静态调度顺序：

1. `8003F710-8003F724`：`lwc2 $0..$5` 载入前三个 vertex。
2. `8003F72C`：执行 `cop2 0x280030`，按导出命名为 RTPT。
3. `8003F73C/740/744`：`swc2 $12/$13/$14` 写前三个 SXY 输出。
4. `8003F748`：`cfc2 $v1, $31` 读取 RTPT 后 FLAG。
5. `8003F74C/750`：`lwc2 $0/$1` 载入第四个 vertex。
6. `8003F758`：执行 `cop2 0x180001`，按导出命名为 RTPS。
7. `8003F768`：`swc2 $14` 写第四个 SXY 输出。
8. `8003F76C`：`swc2 $8` 写 depth/IR0 side output。
9. `8003F770-77C`：读取第二次 `$31`，与第一次 FLAG OR 后写 caller flag pointer。
10. `8003F774/784`：`mfc2 $v0, $19`，delay slot `sra $v0, 2` 形成返回值。

`8003F1B4` 消费关系：

- `RotTransPers4(v48, v49, v50, v51, &v52, &v53, &v54, &v55, v56, v57, ...)` 后，transform packet 写入 `v8[2]=v52`、`v8[4]=v53`、`v8[6]=v54`、`v8[8]=v55`。
- Win 当前 `BuildTransformPrelude8003F1B4()` 已准备 matrix、`ReadGeomScreen`、translation、`SetRotMatrix`、`SetTransMatrix` 和四个 vertex。
- Win 当前仍设置 `transformFieldsGap = true`，没有把 `rtOutput.sxy[]` 接到 packet。

## 投影字段矩阵

| 字段 | 已有证据 | 当前源码状态 | 缺口 | 下一轮最小实现/验证口径 |
|---|---|---|---|---|
| SXY packed word `$12/$13/$14` | `8003F73C/740/744` 写 RTPT 后三个 SXY；`8003F768` 写 RTPS 后 `$14`。`8003F1B4` 将这四个 word 作为 transform packet XY 字段。 | `RtptRtps4Output8003F710::sxy[4].known=false`；`8003F1B4` 不消费 SXY。 | 缺 PSX GTE RTPT/RTPS 的 fixed-point projection 公式：`SX = OFX + H * IR1 / SZ3`、`SY = OFY + H * IR2 / SZ3` 的精确移位、舍入、符号、screen clamp、packed endian/order、near-zero/overflow 处理都未闭合。还缺 `$12/$13/$14` 对 FIFO SXY0/SXY1/SXY2 的精确更新顺序验证。 | 可以先实现 `PackSxy(sx, sy) = uint16(sx) | uint16(sy)<<16` 作为字段形状，但 `known` 只能在公式/FLAG/clamp 被验证后置 true。必须用 IDA helper/TMD GTE 既有实现或动态断点拿至少一组 `input matrix/control/vertex -> $12/$13/$14` 真值。 |
| SZ FIFO | `8003F710` 本身未显式读 `$16..$19`，但 `mfc2 $19` 说明最终返回依赖 `SZ3`。RTPT/RTPS 指令按 GTE 语义更新 depth FIFO。 | `GteSzFifo` 有 `elementKnown`；当前 helper 只用 matrix trace 填 `szAfterRtpt[1..3]` 与 `szAfterRtps[0..3]`，整体 `known=false`。 | 缺 RTPT 前旧 `SZ0` 初态；缺 PSX 对 `MAC3/IR3 -> SZ` 的 exact clamp/shift/FLAG 规则；缺 RTPT 后 FIFO 和 RTPS 后 FIFO 的动态真值。当前源码 `ClampUnsigned16(mac[2])` 只是保守 trace，不是权威 GTE SZ。 | 最小实现应显式输入可选 prior SZ FIFO；在没有 prior SZ0 时继续 `szFifoPriorStateGap=true`。下一轮先只把可计算的 `SZ1/SZ2/SZ3` trace 暴露，不把 `szAfterRtpt.known`/`szAfterRtps.known` 置 true，除非补到 PSX SZ FIFO 真值。 |
| FLAG bits `$31` | `8003F748` 读取 RTPT 后 FLAG，`8003F770` 读取 RTPS 后 FLAG，`8003F778/77C` OR 后写 caller flag pointer。register scan 也确认 `$31` 是大量 GTE helper 的 flag/status 读取点。 | `flagAfterRtptKnown=false`、`flagAfterRtpsKnown=false`、`flagOrKnown=false`；gap state `flagBitMappingGap=true`。 | 缺各 bit 的 exact mapping 和触发条件：MAC overflow、IR saturation、SZ saturation、divide overflow、SX/SY screen saturation、near-plane 等。只知道 OR 关系，不知道 bit 生成规则。 | 下一轮字段清单应先保留 `flagAfterRtpt`、`flagAfterRtps`、`flagOr = flagAfterRtpt | flagAfterRtps` 的结构。实现前必须补 GTE FLAG bit table 或动态读取 `$31` 对比；不能用“有 overflow 就随便置 bit”的近似。 |
| IR0 / depth side output `$8` | `8003F76C` 在 RTPS 后 `swc2 $8` 写到 caller 参数；direct notes 标为 IR0/depth side output。 | `ir0Known=false`，`ir0DepthKnown=false`；gap state `ir0DepthCueGap=true`。 | 缺 `$8` 在该 RTPS 指令后的真实语义：可能是 IR0/depth cue side output，但当前导出没有 depth cue 指令参数、DQA/DQB/ZSF 参与关系，也没有 caller 是否使用 `v56/v57` 的后续证据。 | 下一轮可以保留 `ir0` 字段和写出路径，但不能声明 known。需导出/验证 caller 对 `v56/v57` 的使用，或动态断点在 `8003F76C` 抓 `$8`，再决定是否接 depth cue 公式。 |
| return value `$19 >> 2` | `8003F774` `mfc2 $v0,$19`，`8003F784` `sra $v0,2`；按 GTE data reg 语义 `$19` 是最终 SZ3。 | `sz3AfterRtpsKnown=false`、`returnValueKnown=false`；源码只填 `sz3AfterRtps` 值但不置 known，不返回给 `8003F1B4` 使用。 | 缺 `$19` 与最终 RTPS `SZ3` 的 verified mapping、signed/unsigned shift 口径、SZ clamp/overflow 对返回值影响。`sra` 是算术右移，若 `$19` 理论为非负 16-bit，效果等于 `>>2`，但仍要以真值确认。 | 下一轮可实现公式壳：`returnValue = int32(SZ3) >> 2`。但 `returnValueKnown` 只能在 SZ3 exact 规则和 `$19` 真值确认后置 true。 |

## 输入/公式最小清单

下一轮可以直接落到 `pr_psx_gte_direct.*` 的最小字段，但默认仍以 gap 输出：

- 输入：`Matrix3x4` 的 rotation `$0..$4`、translation `$5..$7`，`GteControlState.geomScreen`，`GteControlState.geomOffsetX/Y`，四个 `VertexS16`，可选 prior `SZ0..SZ3`。
- 线性变换 trace：`MAC1/MAC2/MAC3 = TR + R * V`，保留当前 `mac/ir/depth` trace，但标注它不是完整 GTE saturation 实现。
- 深度输入：每个 vertex 的 `SZ` candidate，明确区分 `candidateKnown` 和 `gteSzKnown`。
- 除法输入：`H` 和每个 candidate `SZ`，记录 `inputsKnown=true`；quotient、overflow、flagBits 默认 unknown。
- SXY 结构：四个 packed word 输出槽，字段形状为 `low16=sx`、`high16=sy`，但 `known=false`，直到 fixed-point 除法和 clamp 验证闭合。
- FIFO 结构：RTPT 后至少有三点 depth candidate；RTPS 后有四点 candidate；没有 prior FIFO 时整体 `known=false`。
- FLAG 结构：`flagAfterRtpt`、`flagAfterRtps`、`flagOr` 三段字段，默认 unknown。
- IR0 结构：保留 RTPS 后 `$8` 输出槽，默认 unknown。
- return 结构：保留 `rawSz3Reg19` 和 `returnValue = rawSz3Reg19 >> 2` 的字段关系，默认 unknown。

## 仍需 IDA/动态验证

必须补证据后才能关闭对应 gap：

- IDA：导出或整理 PSX SDK/GTE helper 中 RTPT/RTPS、perspective divide、screen clamp、FLAG bit 的权威公式；优先在现有 TMD GTE 函数里找同类 `$12/$13/$14/$19/$31` 消费规则。
- IDA：确认 `$8` 在 `8003F710` 之后的 caller 使用范围，判断它是否实际影响 `8003F1B4` / rail packet。
- 动态：在 `8003F710` 设置断点，抓同一帧的 matrix control regs `$0..$7/$24/$25/$26`、四个 input vertex、`$12/$13/$14/$19/$31/$8` 输出。
- 动态：至少覆盖普通正深度、近零/负深度、screen clamp/overflow 三类样本；否则 FLAG 和 saturation 不能置 known。

## 禁止结论

- 不得把 Win float 投影、renderer 坐标或现代透视校正坐标写回 `8003F710` 的 SXY。
- 不得仅凭 `8003F1B4` packet 需要四个 XY，就让 `transformFieldsGap=false`。
- 不得把当前 `ClampUnsigned16(mac[2])` 当成已验证的 PSX SZ FIFO。
- 不得把 `returnValue = sz3 >> 2` 标成 known，除非 `$19` / SZ3 / saturation 已被真值确认。
