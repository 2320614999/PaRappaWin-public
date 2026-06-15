# 8003F710 GTE projection formula source - 2026-05-10

范围：GTE-formula-source。只用本地源码、现有 IDA 导出和 docs 证据，为 `8003F710 RotTransPers4` 查找同类 `RTPT/RTPS/GsSortSprite/TMD` 投影公式来源；不改 `src`，不编译、不测试、不上网。

结论：本地证据足够把 `8003F710` 的“可翻译公式壳”推进到 `pr_psx_gte_direct`：矩阵乘法、`H/SZ` 透视除法、`OFX/OFY` 偏移、SXY packed word、RTPT+RTPS 调度、FLAG OR、`SZ3 >> 2` 返回字段关系都能写成 direct helper 的结构。但不能把输出标成 GTE truth：GTE division 精确商、screen saturation、FLAG bit、SZ FIFO prior state、IR0/depth cue 和异常深度行为仍需要动态真值或更完整的 GTE 位级表验证。

## 本地证据清单

### `8003F710 RotTransPers4`

来源：

- `docs/stage1_decomp/sprite_submit_20260510/ida_export_8003F710_rottranspers4_followup.txt`
- `docs/stage1_decomp/sprite_submit_20260510/ida_export_8003F1B4_gte_helpers.txt`
- `docs/stage1_decomp/sprite_submit_20260510/ida_export_gte_register_owner_scan.txt`

证据：

- `8003F710-8003F724` 连续 `lwc2 $0..$5` 载入前三个顶点。
- `8003F72C cop2 0x280030` 是 RTPT 调度点。
- `8003F73C/8003F740/8003F744 swc2 $12/$13/$14` 把 RTPT 后三个 SXY word 写给 caller。
- `8003F748 cfc2 $v1,$31` 读取 RTPT 后 FLAG。
- `8003F74C/8003F750` 载入第四个顶点，`8003F758 cop2 0x180001` 是 RTPS 调度点。
- `8003F768 swc2 $14` 写第四个 SXY word，`8003F76C swc2 $8` 写 IR0/side output。
- `8003F770 cfc2 $t0,$31` 读取 RTPS 后 FLAG，`8003F778/8003F77C` 将两次 FLAG OR 后写回 caller 指针。
- `8003F774 mfc2 $v0,$19`，delay slot `8003F784 sra $v0,2` 形成返回值。按 GTE data reg 使用习惯，这是最终 `SZ3 >> 2` 字段关系，但 exact SZ truth 仍未闭合。

### `8003F1B4 GsSortSprite`

来源：

- `docs/stage1_decomp/sprite_submit_20260510/ida_export_8003F1B4_gte_helpers.txt`
- `docs/stage1_decomp/sprite_submit_20260510/ida_export_8003F1B4_deps.txt`
- `docs/stage1_decomp/sprite_submit_20260510/rail_gte_next_cutover_plan_20260510.md`

证据：

- transform path 在 `8003F430 ReadGeomScreen` 后把 local sprite x/y 和 `GeomScreen` 写入 translation，经 `TransMatrix`、`sub_8003F6B0`、`SetTransMatrix` 再调用 `RotTransPers4`。
- 四个 local vertex 是 `(-mx,-my,0)`、`(w-mx,-my,0)`、`(-mx,h-my,0)`、`(w-mx,h-my,0)`。
- `RotTransPers4(v48,v49,v50,v51,&v52,&v53,&v54,&v55,v56,v57,...)` 后，packet transform words 直接消费 `v52/v53/v54/v55`：`v8[2]=v52`、`v8[4]=v53`、`v8[6]=v54`、`v8[8]=v55`。
- 这证明 `8003F710` 的 SXY packed word 是 `8003F1B4` transform packet 的 geometry authority；不能用 Win renderer 的 float 坐标替代。

### TMD / `GsTMDfast*` 同类 GTE 路径

来源：

- `docs/stage1_decomp/sprite_submit_20260510/ida_export_gte_register_owner_scan.txt`
- `src/pr/pr_tmd.h`
- `src/pr/pr_stage_scene_submit_direct.cpp`

证据：

- register scan 显示大量 TMD fast 函数执行同类 GTE pipeline：`GsTMDfastNF3/F3NL/NF4/F4NL/NG3/G3NL/NG4/G4NL/TNF3/TF3NL/TF3L/TNF4/TF4NL/TNG3/TG3NL/TNG4/TG4NL` 都有 `cop2 0x280030` RTPT；四点 primitive 还出现 `cop2 0x180001` RTPS。
- 同一批 TMD fast 函数反复读取 `cfc2 $v0,$31`，证明 FLAG 不是 `8003F710` 独有副产物，而是 GTE helper 通用可见状态。
- TMD fast path 还调用 `cop2 0x1400006` 后 `mfc2 $v0,$24`，以及 `cop2 0x158002D` / `0x168002E` 后 `mfc2 $t6,$7`，对应 NCLIP/AVSZ 类后续几何筛选和 OT 深度行为。这能证明 SXY/SZ/FLAG 是一个 GTE pipeline，不是单独的 host projection。
- `src/pr/pr_tmd.h` 明确 Stage1 scene TMD 使用 `sub_800428B0/GsTMDfast*` 的 PSX primitive subset。
- `src/pr/pr_stage_scene_submit_direct.cpp` 已有 `PsxPackSxy428B0()`：`uint16(x) | uint16(y) << 16`，以及 `PsxPacketSxyWordIndex428B0()` / `PsxBuildGsTmdFastPacket428B0()` 把 projected SXY 写入不同 primitive packet word。这个是本地同类 SXY packet 形状证据。

### 现有 Win TMD renderer 只能作形状参考

来源：

- `src/pr/pr_tmd_renderer.cpp`

证据：

- `ProjectVertex()` 注释写明 PSX GTE perspective 形状：`sx = x * h / z + 160`，`sy = y * h / z + 120`。
- 但该文件是 Scene0/title TMD renderer，使用 float、near clamp `cz < 10.0f`、现代 draw batch 和 viewScale；它只能证明“公式形状”与历史 renderer 意图，不能作为 `8003F710` 的 SXY authority。
- `8003F710` 的 authority 必须来自 PSX GTE instruction/register behavior 或动态真值，不来自 Win float renderer 坐标。

## 可以翻译进 `pr_psx_gte_direct` 的公式

以下可以作为 direct helper 的可计算结构落地，但输出 known 状态要分层：

```text
输入：
  rotation matrix: GTE control regs $0..$4
  translation:     GTE control regs $5..$7
  H:               GTE control reg $26 / ReadGeomScreen
  OFX/OFY:         GTE control regs $24/$25 / SetGeomOffset
  vertices:        four SVECTOR-compatible local vertices

对每个 vertex：
  MAC1 = TRX + ((R11*x + R12*y + R13*z) >> 12)
  MAC2 = TRY + ((R21*x + R22*y + R23*z) >> 12)
  MAC3 = TRZ + ((R31*x + R32*y + R33*z) >> 12)

  candidateSZ = clamp_unsigned_16(MAC3)
  quotient    = gte_divide(H, candidateSZ)
  SX          = OFX + project(MAC1, quotient)
  SY          = OFY + project(MAC2, quotient)
  SXY         = uint16(SX) | (uint16(SY) << 16)
```

调度结构也可以翻译：

- vertex 0..2 先执行 RTPT，输出 `sxy[0] = reg12`、`sxy[1] = reg13`、`sxy[2] = reg14`。
- vertex 3 再执行 RTPS，输出 `sxy[3] = reg14`。
- `flagAfterRtpt` 和 `flagAfterRtps` 分开保存，`flagOr = flagAfterRtpt | flagAfterRtps`。
- `rawSz3Reg19` 保留为最终 RTPS 后 `$19`，`returnValueFormula = int32(rawSz3Reg19) >> 2`。
- `ir0Reg8AfterRtps` 保留为 `$8` side output，不要先解释成已知 depth cue。

可以确定的 packet 边界：

- `8003F1B4` transform packet 的 `word2/word4/word6/word8` 应分别来自 `sxy[0..3]`。
- `PackSxy(sx, sy)` 的 word 形状可以复用 TMD path 的本地证据：low16 = x，high16 = y。
- `pr_psx_gte_direct` 只负责 PSX integer/GTE fields；Win render adapter 后续可把 int16 坐标转 float，但不能反向成为 authority。

## 仍需动态真值 / 不可提前 known

这些不能仅凭本轮本地静态证据关闭：

- GTE division exact quotient：需要验证 `H/SZ` 查表/规格化/rounding、near-zero、overflow 和 `SZ <= 0` 行为。
- SX/SY saturation：`PsxSxyFromRtpt428B0()` 目前只是 `int16` clamp 近似，不等于已验证 GTE screen saturation。
- FLAG bit mapping：已经证明 `$31` 被 RTPT/RTPS/TMD fast path读取，但每个 bit 的触发条件仍缺表或真值。
- SZ FIFO：RTPT 前旧 `SZ0` 和 RTPT/RTPS 后 `SZ0..SZ3` 精确滚动、clamp、overflow 仍缺动态样本。
- IR0 / `$8`：`8003F76C` 写出 `$8`，但本地静态证据未证明 caller 是否消费、也未证明 depth cue 公式。
- return known：`$19 >> 2` 字段关系可记录，但必须等 `$19/SZ3` exact 行为验证后才能 `returnValueKnown=true`。
- TMD fast code path：本地 `PsxProjectVertexRtpt428B0()` 是 Stage1 direct 层的整数近似，不是完整 PSX GTE emulator；可作为公式来源和 packet shape 参考，不能把它的 clamp/divide 当作 8003F710 的最终 truth。

## 建议实现边界

可以在下一轮代码中把 helper 拆成三层：

1. `transformTraceKnown`：矩阵乘法、candidate depth、`H/SZ` 输入、SXY formula candidate 可计算。
2. `packetShapeKnown`：SXY word packing、RTPT/RTPS 输出槽、FLAG OR、`$19 >> 2` 字段关系可记录。
3. `gteTruthKnown`：只有动态真值或完整 GTE 位级规则验证后，才允许 `sxy[].known`、`szAfterRtpt.known`、`szAfterRtps.known`、`flag*Known`、`returnValueKnown` 置 true。

禁止线：

- 不得用 `src/pr/pr_tmd_renderer.cpp` 的 float 投影坐标作为 `8003F710` SXY authority。
- 不得因为 `src/pr/pr_stage_scene_submit_direct.cpp` 已有 `PsxProjectVertexRtpt428B0()` 就把 `8003F710` 的 FLAG/SZ/IR0/return 标 known。
- 不得让 `8003F1B4.transformFieldsGap=false`，除非四个 SXY word 来自 `pr_psx_gte_direct` 的 verified GTE output。

## 本轮未做

- 未修改 `src`。
- 未编译。
- 未测试。
- 未启动模拟器或内存回放。
- 未联网。
