# Stage1 rail/GTE next cutover plan - 2026-05-10

> Superseded 2026-05-13: this file is an old static plan. Current Rail/GTE
> proof is live-trace only: `0x8003F4F4 -> 0x8003F710`, with COP2/GTE
> `SXY/FLAG/IR0/SZ/return` facts. Do not use the "next round should first
> implement the minimal GTE helper" statement below as current authorization.

范围：P0-C 静态核实。按 `docs/项目规则.md`，本轮只读源码/IDA 导出并输出文档；不编译、不测试、不改 `src`。

## 结论

当前 `80024744 compact rail -> 8001C550/8001C804 -> 8003FA20/8003F1B4` 直译链已经接到新的 submit runtime，但还不能做最终 cutover。两个 P0 缺口仍会让 rail sprite metadata 停在 gap 状态：

1. `8003F710 RotTransPers4` 只在 Win 侧记录 RTPT/RTPS 调度，没有真实 GTE 投影结果，导致 `8003F1B4` transform packet 的几何字段仍不能 authoritative。
2. `8001C550/8001C804` 的 local sprite RGB owner 还没有找到。源码已把这条链标成 `StaticChainHasNoRgbWrite`，不能用猜测常量去填 `r/g/b`。

因此下一轮应先补 `8003F710` 最小 GTE helper，再补 RGB owner 证据。两者都闭合后，才把 rail submit 从 metadata/gap 推到完整 packet cutover。

## 已核实源码状态

### `src/pr/pr_psx_gte_direct.*`

- `RtptRtps4Input8003F710` 已具备 matrix/control/4 顶点输入字段，`RtptRtps4Output8003F710` 已预留 SXY、SZ FIFO、FLAG、IR0、return、division trace。
- `PsxCall8003F710_RotTransPers4RtptRtpsGap()` 当前只做：
  - 标记 `inputsCompleteForGeometry`；
  - 标记 RTPT/RTPS schedule；
  - 把 `geomScreen` 写进 division trace 的 `h`；
  - 没有计算 `sxy[]`、`szAfterRtpt`、`szAfterRtps`、`flagAfterRtpt`、`flagAfterRtps`、`ir0`、`returnValue`。

影响：这个文件现在是结构占位，不是 GTE 行为实现。

### `src/pr/pr_psx_gs_sprite_submit_direct.*`

- `8003F1B4` transform path 已经把四个局部顶点构造成：
  - `(-mx, -my, 0)`
  - `(width - mx, -my, 0)`
  - `(-mx, height - my, 0)`
  - `(width - mx, height - my, 0)`
- `BuildTransformPrelude8003F1B4()` 已覆盖 matrix 准备顺序：`RotMatrix` 或 identity globals、可选 `ScaleMatrix`、`ReadGeomScreen`、`TransMatrix(x, y, geomScreen)`、`SetRotMatrix`、`SetTransMatrix`、再调用 `8003F710` gap helper。
- transform packet 当前只确认非几何字段：color/code、UV/CLUT/tpage 等；`word2/4/6/8` 应由 `8003F710` 的 SXY 输出填入，但现在仍受 `transformFieldsGap` 阻塞。

影响：`8003F1B4` 的 fast path 可以较完整地写包；transform path 已接近闭合，但缺 `8003F710` 几何核心。

### `src/pr/pr_stage1_compact_rail_80024744_direct.*`

- `80024744` 已按 Stage1 compact rail 直译链输出 portrait、marker、body note 三类 sprite command。
- portrait/marker 走 `PsxCall8001C550_80024744()`，再进入 fast sprite submit metadata。
- body note 走 `PsxCall80024418_80024744()`，再进入 `8001C804 -> 8003F1B4` metadata。
- `PsxCompactRailInput80024744` 已接收 `submitRuntime8003FA20` 和 `submitRuntime8003F1B4`，来源是 graph owner 的 draw buffer、packet allocator、OT work、draw offset、GTE control。
- RGB carrier 仍是外部输入字段：`localFastSpriteRgb8001C550` / `localGsSpriteRgb8001C804`。当前 `PsxCall80024744_801CBFDC()` 没有填这两个字段。
- `ApplyStaticNoRgbWriteEvidence80024744()` 会在 RGB unknown 时把 gap 标为 `StaticChainHasNoRgbWrite`，并记录 wrapper/local builder/submit 函数。

影响：rail 侧没有硬猜 RGB，这是正确的；但只要 RGB unknown，`localFastSpriteKnown` / `localGsSpriteKnown` 就为 false，后端 submit metadata 不会进入完整包写入。

## IDA 静态证据

### `8003F710 RotTransPers4`

导出文件：

- `docs/stage1_decomp/sprite_submit_20260510/ida_export_8003F710_rottranspers4_followup.txt`
- `docs/stage1_decomp/sprite_submit_20260510/ida_export_8003F1B4_gte_helpers.txt`
- `docs/stage1_decomp/sprite_submit_20260510/ida_export_gte_register_owner_scan.txt`

已确认控制流：

1. `8003F710-8003F724`：把前三个顶点载入 GTE data regs `$0..$5`。
2. `8003F72C`：执行 `cop2 0x280030`，即 RTPT。
3. `8003F73C/740/744`：输出 `$12/$13/$14`，即前三个投影 SXY。
4. `8003F748`：读取 FLAG 到 `$v1`。
5. `8003F74C/750`：把第四个顶点载入 `$0/$1`。
6. `8003F758`：执行 `cop2 0x180001`，即 RTPS。
7. `8003F768`：输出第四个 SXY `$14`。
8. `8003F76C`：输出 `$8`，作为 IR0/depth side output。
9. `8003F770-77C`：读取第二次 FLAG，与 RTPT FLAG OR 后写到 caller-provided flag pointer。
10. `8003F774/784`：读取 `$19` 并 `>> 2` 返回，按 GTE data reg 语义应为最终 `SZ3 >> 2`。

`8003F1B4` 只有一个 xref 调用 `8003F710`，地址是 `8003F4F4`。这说明 P0-C 的 GTE helper 最小范围可以先只服务 `GsSortSprite` transform sprite，不必一次吃完整 TMD GTE 栈。

### `8003F1B4 GsSortSprite`

静态导出显示：

- fast path 用 `0x64000000 | b<<16 | g<<8 | r` 写 sprite code/color，并直接写 XY/UV/WH。
- transform path 在调用 `RotTransPers4` 后，把 `v52/v53/v54/v55` 写进 packet 的 SXY 字段；非几何 UV/tpage/color 字段仍由 local sprite 字段构造。
- `8003F1B4` 依赖 `ReadGeomScreen`、`SetRotMatrix`、`SetTransMatrix`，并由 `SetGeomScreen` / `SetGeomOffset` 生命周期提供控制寄存器。

对应 Win 现状：`RuntimeState8003F1B4.gte` 已从 graph owner 传入，但 `8003F710` 没产出真实几何，导致 transform packet 仍不能关闭 gap。

### `8001C804 -> 8001B338 -> 8003F1B4`

导出文件：

- `docs/stage1_decomp/sprite_submit_20260510/ida_export_8001BC48_8001C804_8003F1B4.txt`
- `docs/stage1_decomp/sprite_submit_20260510/ida_export_8003F1B4_deps.txt`

已确认：

- `8001C804` 只是 wrapper：先 `sub_8001B338(a1, template, scaleX, scaleY)`，再 `GsSortSprite(a1, &unk_80087288[20 * drawBuffer], 1)`。
- `8001B338` 写 local GsSprite 的宽高、tpage、UV、CLUT、scale、mx/my 等字段。
- `8001B338` 导出范围里没有看到对 `a1+0x14/+0x15/+0x16` 的 RGB 写入。
- `80024418` 构造 note 的 local stack sprite 后调用 `8001C804`，其当前导出也没有解释 RGB owner。

结论：`8001C804` 这一侧还缺 RGB 来源证据，不能把 note color 当作已知。

### `80024744 -> 8001C550 / 80024418`

已确认：

- teacher/student portrait 和大小 marker 走 `sub_8001C550(...)`。
- body note 走 `sub_80024418(v27)`，再进入 `sub_8001C804(...)`。
- `80024744` 自身没有对 submit local sprite RGB 的显式写入。

结论：`8001C550` 的 owner 还需要单独导出 `8001C550`、它的 wrapper `8001B590`、local builder `8001B25C`，再做 offset write matrix。当前已有导出不足以判定 fast sprite 的 RGB。

## P0-C 下一步切换顺序

### 1. 先闭合 `8003F710` 最小 GTE helper

目标：让 `8003F1B4` transform path 能写出真实 SXY/SZ/FLAG/return metadata。

静态实现前必须补齐的最小规格：

- 输入：matrix 3x4、geom screen `H`、geom offset `OFX/OFY`、四个 `VertexS16`。
- 顺序：前三点按 RTPT 更新 FIFO，第四点按 RTPS 更新 FIFO。
- 输出：
  - `sxy[0..2]` 来自 RTPT 后 `$12/$13/$14`；
  - `sxy[3]` 来自 RTPS 后 `$14`；
  - `szAfterRtpt` 和 `szAfterRtps`；
  - `flagAfterRtpt`、`flagAfterRtps`、`flagOr`；
  - `ir0`，即 `$8`；
  - `returnValue = SZ3 >> 2`。
- 除法：记录 `H / SZ` 的 GTE division trace，处理近零、overflow、saturation、FLAG bit。
- 坐标：使用 PSX fixed-point 规则，输出 packed SXY word；不要用 float 渲染坐标替代 GTE 整数结果。

验收口径：`PrPsxGteDirect::RtptRtps4Output8003F710.known == true` 且 `PrPsxGsSpriteSubmitDirect::GsSortSpritePacket8003F1B4.transformFieldsGap == false` 的设计前提成立。实际代码和验证不在本轮做。

### 2. 再把 `8003F1B4` transform packet 接上 GTE 输出

目标：关闭 `8003F1B4` 的 transform geometry gap。

应填字段：

- `word2_colorOrXy0 = sxy[0]`
- `word4_uvOrXy1 = sxy[1]`
- `word6_xy2 = sxy[2]`
- `word8_xy3 = sxy[3]`

应保留字段：

- `word1_drawModeOrColor` 仍由 transform color/code 构造。
- `word3/5/7/9` 仍由 UV/CLUT/tpage 构造。
- `sub_8003EF5C` OT link/allocator 逻辑不应和 GTE helper 混在一起改。

风险：如果 `SetGeomScreen` / `SetGeomOffset` 的 runtime owner 不完整，`8003F710` 即使实现也会缺 control input。源码现状已从 graph owner 传 `gte`，但下一轮仍要先核对 `gte_init`、`SetGeomScreen`、`SetGeomOffset` 对 `graph.gte` 的初始化/更新顺序。

### 3. 并行静态搜索 `8001C550/8001C804` RGB owner

目标：找到 local sprite `+0x14/+0x15/+0x16` 的真实来源，或者明确证明该路径依赖 PSX 未初始化/默认值。

优先导出/搜索清单：

- `8001C550`
- `8001B590`
- `8001B25C`
- `8001C804`
- `8001B338`
- `80024418`
- 所有调用 `8001C550` 的 wrapper，重点看调用前是否写 caller-local RGB。
- 所有写入 local sprite offsets `0x14/0x15/0x16` 的 `sb/sh/sw/swl/swr`。

判断规则：

- 如果 owner 是模板字段：把 `PsxCompactRailLocalSpriteRgbCarrier80024744::Source::Template` 接到模板解析。
- 如果 owner 是调用参数：把 `Source::CallArgument` 接到 `80024744` 调用点。
- 如果 owner 是固定常量或全局表：记录具体地址/常量，再接 `FixedConstant` 或 `GlobalTable`。
- 如果静态仍无 owner：下一轮应切到 PSX 内存回放读取调用 `8001C550/8001C804` 前的 local sprite bytes `+0x14..+0x16`，但那属于动态验证，不在本轮执行。

禁止事项：

- 不要把 RGB 默认填 `0x80/0x80/0x80` 或 `0/0/0` 当作修复。
- 不要绕过 carrier 直接让 `localFastSpriteKnown/localGsSpriteKnown` 为 true。
- 不要为了画面颜色临时改 Win renderer。

## 推荐 cutover 检查点

1. `P0-C1`: 文档确认后，先实现 `8003F710` helper。只改 `pr_psx_gte_direct.*` 和最小调用消费，不碰 rail 行为。
2. `P0-C2`: 让 `8003F1B4` transform packet 从 GTE helper 拿四个 SXY，并把 transform geometry gap 降到 false。
3. `P0-C3`: 导出并闭合 `8001C550/8001B590/8001B25C` RGB owner；同步复核 `8001C804/8001B338` 是否确实无 RGB 写入。
4. `P0-C4`: 把 rail `8001C550/8001C804` carrier 接入真实 RGB owner 后，再允许 compact rail submit metadata 进入完整 packet 输出。
5. `P0-C5`: 完成上述静态闭合后，再按项目规则做编译/回归验证。本文件不执行这一步。

## 本轮未做

- 未编译。
- 未运行测试。
- 未启动模拟器或内存回放。
- 未修改 `src`。
