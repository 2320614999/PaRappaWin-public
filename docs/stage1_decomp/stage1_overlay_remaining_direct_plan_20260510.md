# Stage1 overlay remaining direct-port plan 2026-05-10

本文件只做静态核实和下一批模块规划。本轮未编译、未测试、未修改 `src`。

> 2026-05-11 修正：本文件的 P0-A 旧口径把 movie/text
> `801C455C -> 8001EC54/8001ED3C` 与 `8005CB5C/800436F0` text-record
> 链绑定在一起。后续 IDA xref 已证明 `801C4DC4/801C455C` 没有
> `80027FAC/80043394/80043438/80043A14/800436F0` 直接调用；movie/text
> 路径应改为继续直译 `8001EC54 -> 8001DB00/8001B954` 与
> `8001ED3C -> 80040CA4`。`8005CB5C/800436F0` 只保留在
> SCUS text-system/event-frame lane。

## 约束

- 执行 `docs/项目规则.md` 的 direct-port 路线：`导出 -> 画图 -> 翻译 -> 删除 -> 接线`。
- 不做症状补丁，不在 renderer、`pr_scenes.cpp`、`pr_ui_overlay.cpp` 里用 `if` 分支补表现。
- `adapter` 只允许做类型转换、平台 HAL 调用和一行 direct 入口转发；不能持有新状态、推导 authority、缓存结果。
- `SCUS_941.83` / overlay 只能作为离线导出来源，不能成为运行时依赖。
- 删除动作必须等对应 direct unit 的 producer/consumer/packet 边界闭合后再做；不能为了“清壳”提前删除平台 HAL。

## 静态核实摘要

- `docs/项目规则.md` 已把当前优先级定为“Stage1 overlay 全部翻完”，并明确后续才恢复 Win 渲染适配层。
- `docs/memory.md` 末尾显示 2026-05-10 已新增 movie/text outer-loop、movie segment、event4 frame carrier、text flush carrier、compact rail RGB carrier、GTE direct 边界。
- `docs/待办事项.md` 末尾仍列出未关闭项：movie segment producer、text record bank producer、event4 prompt sprite packet/OT、compact rail RGB/GTE、save UI lower helpers、loader/lifecycle HAL feedback。
- `src/pr/pr_stage1_movie_text_outer_loop_direct.*` 已有 `801C455C` runtime，但仍保留 `modeDescGap`、`sub801C448CGap` 以及 loose segment input facts。
- `src/pr/pr_stage1_movie_segment_direct.*` 只提供 48-byte segment record helper，尚未承载 `801C4780` 七行扫描和真实 table/CD lookup producer。
- `src/pr/pr_psx_event_frame_direct.*` 已有 `8001E750/8001EA00/800436F0/80043394/80043438/80043A14` carrier，但 `TextRecordBridgeCarrier800436F0` 默认仍带 `textRecordBankGap/textRecordSlotGap/textRecordGeometryGap/textRecordPacketCursorGap/glyphPacketGap`。
- `src/pr/pr_stage1_fail_prompt_direct.*` 仍把 `8001E750` trace 标成 `DirectPromptDrawWrapperGap`，把 `800436F0` trace 标成 `TextFlushHelperGap`；`BuildDrawCommands_800203D4` 仍是 prompt sprite 表层命令 carrier，不是完整 packet/OT direct submit。
- `src/pr/pr_stage1_compact_rail_80024744_direct.*` 已显式记录 `8001C550/8001C804` local RGB carrier，但默认 `known=false`、`gap=UnresolvedStackLocalTail/StaticChainHasNoRgbWrite`，`submitPacketGap=true` 仍保留。
- `src/pr/pr_psx_gte_direct.*` 当前只定义 GTE state/schedule/gap output，`RtptRtps4Output8003F710::known` 仍不能表示真实 projected XY、IR0、SZ FIFO、FLAG。
- `src/pr/pr_stage1_save_ui_direct.*` 仍大量发出 `HelperGap`；`pr_scenes.cpp` 仍通过 save UI / bootstrap / fail prompt / movie text adapter 消费 direct action。
- `src/pr/pr_stage_scene_submit_direct.cpp` 已持有 owned runtime 和 graph owner；`src/pr/pr_stage_scene_submit_backend.cpp` 仍是资源上传、D3D 绘制和 renderer 适配层，不应拿来关闭 PSX logic gap。

## 优先级总表

| 优先级 | 并行 lane | 模块 | 为什么排这里 | 不能做什么 |
|---|---|---|---|---|
| P0-A | Lane 1 | movie/text + segment + text record producer | 直接阻塞 `801C455C` outer-loop、`80024CF8` 字幕 advance、`800436F0` flush gap 和 movie/text 壳删除 | 不能用 Win STR 播放状态、frame counter、默认 segment 字段补洞 |
| P0-B | Lane 2 | fail prompt / event4 frame / prompt sprite packet | 直接阻塞 event4 旧 dispatcher 删除和 fail prompt native draw fallback 删除 | 不能把 `BuildDrawCommands_800203D4` 当完整 direct submit |
| P0-C | Lane 3 | rail / GTE / sprite submit owner | 直接阻塞 compact rail typed submit、Movie1 glyph typed submit 和 GTE transform packet | 不能用默认 RGB、renderer fallback、Win 投影关闭 gap |
| P1-D | Lane 4 | save / lifecycle / loader feedback | 大模块且剩余 helper 多，但边界多为 HAL/action，适合与 P0 并行导出和画图 | 不能接 Win filesystem/PrCard 作为 PSX helper 实现 |
| P1-E | Lane 5 | scene submit / renderer 适配层 | direct-owned runtime 已迁入，下一步应等 P0 raw output 稳定后恢复现代渲染适配 | 不能在 renderer 里选择 branch 或修 PSX 状态 |

建议下一批先并行推进 P0-A/P0-B/P0-C 的“导出+画图”，Lane 4 做 save/lifecycle 的 helper 清单导出，Lane 5 只记录 renderer adapter contract，不抢先改 renderer。

## P0-A: movie/text + segment + text record producer

### 导出

- 补齐 `801C4780` 七行 segment scan 的 standalone 伪 C / disasm 证据，和 `8001A324` 对七个 0x30 row 的 mutation 顺序。
- 补齐 `8001A2B0` / CD lookup 对 `CdlFILE.pos/size` 的 producer 边界，至少明确哪些字段是 direct record、哪些是 CD HAL feedback。
- 补齐 `801C455C` 的 mode desc 表来源，尤其 `mode == 3` 的 `dword_800943CC + 28*mode` 字段。
- 补齐 text record producer 的 `80044238/800440D0`，并把 `80043394/80043438/80043A14/800436F0` 的 record field 写入关系整理成同一张图。
- 对 `80026314` 和 `80026B94(event=2 StageClear)` 的 `80043A14` callsite 做 vararg/format 数据流导出，禁止把 `&8006EC14`、`byte_80092F1D[i]` 硬编码成 host string。

### 画图

- 图 1：`801C4780 -> 8001A324 -> MovieSegmentRecord48[7] -> 801C4DC4(sceneEntry+0x6C) -> 801C4350 -> 8001A7A4/8001A7F8`。
- 图 2：`801C455C -> 80024C84 -> 80024CF8 -> 8001EC54/8001ED3C -> 800436F0`。
- 图 3：`80043394/FntLoad -> 80043438/record alloc -> 80043A14/append -> 800436F0/flush -> 800450A0/DrawOTag`。
- 图 4：`gp+872 work slot -> 80087288 + 20*slot -> work+0x10 list head -> text/glyph packet cursor`。

### 翻译

- 扩展 `PrStage1MovieSegmentDirect`：增加七行 record bank、`801C4780` scan/mutate、scene entry 选择、CD lookup feedback carrier。
- 扩展 `PrStage1MovieTextOuterLoopDirect`：把 loose `segmentTimeBaseA1Plus40/segmentEndA1Plus44/segmentEndBiasA1Plus8` input 收窄为 `MovieSegmentRecord48` 或 segment runtime snapshot。
- 扩展 `PrPsxEventFrameDirect`：让 `80043438` 真正写 8-slot record bank 的 geometry/text buffer/glyph cursor，`80043A14` 消费真实 record slot。
- 保留 `8001EC54/8001ED3C` bridge，直到 record producer 能完全替代 `textRecord*Gap`。

### 删除

- record bank producer 闭合前，不删 `TextRecordBridgeCarrier800436F0`。
- segment row producer 闭合前，不删 `BuildStage1Movie1HostFeedback(...)` 的 segment/clock pass-through。
- `TickStage1MovieTextDirectAdapter(...)` 只能在 `801C455C` outer-loop 能直接消费 segment、clock、work slot、text record producer 后压成一行调用。
- `Begin/End...801C77C0`、散装 movie/text state 拼装必须等 `801C455C` 成为唯一 authority 后删除。

### 接线

- `pr_scenes.cpp` 只传平台事实：pad mask、language、current draw slot、CD/STR HAL feedback。
- movie/text direct 输出 text record bridge 或 record append actions；event-frame direct 消费这些 actions。
- 不允许由 scene 壳根据 Win STR `strPlayed`、播放结束、frame counter 推导 `8001A7A4/8001A7F8`。

## P0-B: fail prompt / event4 frame / prompt sprite packet

### 导出

- 补齐 `800203D4 -> 8001C550 -> 8001B590 -> 8001B25C -> 8003FA20` 的 prompt sprite packet/OT submit 证据，尤其 local FastSprite RGB tail owner。
- 继续核对 `80026B94(event=4)` 完整 frame loop：`80026744 -> 8001E750 -> 80035560 -> 8001EA00 -> 800436F0`。
- 补齐 `800468E0` 的 interrupt-mask scheduling 和 GPU/DMA HAL completion 语义；当前 `80046840` 寄存器 carrier 不是完整调度器。
- 整理 `gp+0x38C` 三个 event4 draw branch 的状态流：prompt `800203D4`、backdrop `8001B6C4`、reset `8001B120`。

### 画图

- 图 1：`801C81EC -> 801C7A60 fail/abort -> 80026B94(4,0) -> event4 modal`。
- 图 2：`event4 state gp+0x38C -> 8001E750 branch -> packet/OT work list -> 8001EA00 flip -> 80040CA4/800450A0/800468E0`。
- 图 3：`80026744` input edge -> `80025F0C` selection -> `80025C8C` cue -> 60-frame tail。
- 图 4：`800436F0(-1)` 和 P0-A text record producer 的共享边界，避免 Lane 1/Lane 2 双写 record bank。

### 翻译

- `PrStage1FailPromptDirect::BuildDrawCommands_800203D4` 只能作为临时 draw command carrier；下一步应翻成 prompt sprite packet/OT direct action。
- `PrPsxEventFrameDirect::PsxCall8001E750_Event4` 应保存 prompt sprite submit result，和 backdrop/reset branch 一样有 packet/OT evidence。
- `PrStage1FailPromptDirect` 继续只输出 dispatcher frame actions；平台 cue/vblank 仍走 thin HAL adapter。

### 删除

- `pr_ui_overlay.cpp::RenderEvent4PromptNative(...)` 不能在 `800203D4` packet/OT 未闭合前删除或重写成新 fallback。
- `RenderVerticalMenu` 的 event4 fallback 只能在 native prompt sprite direct submit 可渲染后删除。
- `pr_event.cpp` event4 只有在 `80026B94` frame loop、text flush、prompt sprite submit、DMA/HAL boundary 全部闭合后删除。

### 接线

- `pr_scenes.cpp` 的 fail prompt adapter 只消费 action list：pad state、cue action、vblank HAL、text flush/graph flip carrier。
- `800436F0(-1)` record bank 由 Lane 1 提供；Lane 2 只调用/消费，不另建本地 text record mirror。
- `800468E0` 接线必须保持 HAL 边界：direct 记录调度语义，Win 侧只执行 GPU/DMA 等价提交。

## P0-C: rail / GTE / sprite submit owner

### 导出

- 继续追 `8001C550/8001C804` local sprite `+0x14/+0x15/+0x16` RGB owner；当前静态链只证明 submit 读取 RGB，没有证明调用端写入。
- 补齐 `8003F700/8003F710` 依赖的 GTE register/FLAG/division/SXY/SZ FIFO 语义。
- 对 `8003F6B0/8003F6E0/80040C74/800402C0/80040D6C/801CB6BC` 到 `8003F710` 的 control/matrix 写入链画完整。
- 如果 RGB owner 在当前调用链不存在，继续向更外层 stack frame 或 template construction owner 查，不允许用默认色结束。

### 画图

- 图 1：`801CBFDC -> 80024744 compact rail -> 8001C550/8001C804 -> 8003FA20/8003F1B4 -> OT`。
- 图 2：`PrPsxGraphOwnerDirect -> 8004019C/work list/packet allocator -> compact rail typed submit runtime`。
- 图 3：`SetGeomScreen/Offset/DepthCue/ZSF -> RotMatrix/TransMatrix -> RTPT/RTPS -> SXY/SZ/FLAG -> sprite transform packet`。
- 图 4：Movie1 glyph `8001B954 -> 8001C550` 与 compact rail `8001C550` 共用 FastSprite backend 的 RGB/owner 分叉。

### 翻译

- `PrPsxGteDirect` 新增最小 RTPT/RTPS helper，先实现 PSX division、saturation、FLAG、SXY/SZ FIFO，再让 `pr_psx_gs_sprite_submit_direct` 消费真实 output。
- `PrPsxGsSpriteSubmitDirect` 只有在 transform fields 全部来自 GTE direct 后才能清 `transformFieldsGap`。
- `PrStage1CompactRail80024744Direct` 只在找到权威 RGB owner 后，把 `localFastSpriteRgbKnown/localGsSpriteRgbKnown` 置 true 并打开 typed submit。

### 删除

- 不删除 `submitPacketGap`，直到 packet words、RGB、work/OT owner、allocator advance 都由 direct runtime 证明。
- 不删 renderer fallback 或改 draw path 来掩盖 rail sprite 缺字段。
- 不把 Movie1 glyph submit 和 compact rail submit 混成一个“默认白色”路径。

### 接线

- compact rail 和 Movie1 glyph 都应消费 `PrPsxGraphOwnerDirect` 或 direct-owned scene submit runtime 的 work/OT owner。
- GTE direct 输出 PSX 整数投影和 packet fields；renderer adapter 只做现代坐标/纹理显示转换。
- `pr_stage_scene_submit_backend.cpp` 不能参与 branch 选择或字段推导。

## P1-D: save / lifecycle / loader feedback

### 导出

- save UI 优先补：`80017594`、`80017B18`、`80017C08`、`80017A10`、`80017B60`、`8004800C`。
- payload producer 继续补：`80015CC4/800164B4/8001635C/8001628C` 的 full owner，确保 `80092F10..+4876` bank 何时 known。
- loader/lifecycle 继续补：INT/header record producer、CD live payload feedback、TIM/GPU upload feedback、SPU/VAB/SFX feedback。
- `80035510` 只做 pad exact mask/de-dupe adapter，不要在 save UI 内重写输入策略。

### 画图

- 图 1：`80018FB0 -> 800185D0 -> 80019458` state machine，以及每个 state 调用的 helper。
- 图 2：`80019458(state=15) -> 8004800C/title -> 80017C08/header/icon -> 80025C64/payload copy -> 80017A10/card write retry`。
- 图 3：`15590 -> 1AC18 -> 1A8F0` loader runner，分出 memory/CD/GPU/SPU 四个 HAL feedback lane。
- 图 4：`PrStage1LifecycleExecutorDirect` 的 host block：Movie1、ClearTailMovie、AbortPoll、Bootstrap15590、SaveUi19148。

### 翻译

- `PrStage1SaveUiDirect` 继续把 state/action 翻成 PSX-shaped action list；card open/write/format 仍输出 HAL action，不接 Win filesystem。
- `PrStage1LoaderDirect` / `PrStage1LoaderProducerAdapter` 只接真实 feedback，不在 executor 里实现资源查找、D3D upload、SPU playback。
- `PrStage1LifecycleExecutorDirect` 持续把 host block 变薄：状态机归 direct，平台副作用保留 adapter。

### 删除

- `Start/TickStage1SaveUi19148Block801C81EC` 不能删到没有 card HAL feedback 为止。
- `Start/TickStage1Bootstrap15590Block801C81EC` 不能删到 loader action pump 能消费 live record/CD/GPU/SPU feedback 为止。
- `ApplyStage1LifecycleAction801C81EC` 的 audio/transition/resource/save 分支是平台 adapter；目标是变薄，不是提前删除。

### 接线

- scene 壳只把 host/HAL result 回填给 lifecycle executor。
- save result 必须来自 direct action/HAL feedback，不由 Win UI 或 filesystem 推断。
- loader payload bytes 必须沿 `CD payload provider -> resolved payload -> TIM/SPU feedback` 进入 direct runner。

## P1-E: scene submit / renderer 适配层

### 导出

- 不优先新增 PSX logic 导出；先整理 direct output contract：PSX int16 坐标、OT/packet words、TMD primitive groups、texture/tpage/clut、GTE projected fields。
- 如果发现 renderer 需要的字段在 direct output 中缺失，回到对应 P0 模块导出 PSX owner，而不是在 renderer 补默认。

### 画图

- 图 1：`PrStageSceneSubmitDirect owned runtime -> PrStageSceneSubmitBackend -> D3D11Renderer`。
- 图 2：`PSX integer output -> Win adapter int16->float / viewport / perspective correction / texture atlas`。
- 图 3：`resource upload HAL` 和 `render draw HAL` 分界；资源加载不是 PSX logic authority。

### 翻译

- 这里不做“PSX 逻辑翻译”；它是 direct 后的渲染适配层恢复。
- `PrStageSceneSubmitDirect` 继续保持 PSX-style state/output。
- `PrStageSceneSubmitBackend` 只消费 direct output 并做 Win 渲染所需转换。

### 删除

- backend 中 debug dump/helper 可以等 direct current-state API 稳定后迁走。
- 不删资源/TIM/TMD/VRAM/D3D upload HAL。
- 不在 P0 gap 未闭合前删除 fallback draw path，否则只会把未直译字段伪装成渲染问题。

### 接线

- P0-A/P0-B/P0-C 闭合后，再恢复/压薄 Win 渲染适配：int16->float、viewport scale、texture atlas、透视/采样修正。
- adapter 输入必须是 direct output；禁止 adapter 读取 scene/scorer/runner 旧壳状态重新推导。

## 下一批并行安排

### Batch 1: 导出 + 画图

| Lane | 输出物 | 完成条件 |
|---|---|---|
| Lane 1 movie/text | `801C4780/8001A324/8001A2B0` segment producer 图；`80043394/80043438/80043A14/800436F0` record producer 图 | 能说明每个 `801C4350` input 和每个 `800436F0` record field 的 owner |
| Lane 2 event4 | `800203D4 -> 8001C550 -> 8003FA20` prompt packet 图；`80026B94(4)` full frame 图 | 能说明 event4 prompt branch、text flush、DMA/HAL 的删除门槛 |
| Lane 3 rail/GTE | GTE control/matrix/RTPT/RTPS 图；RGB owner search report | 能说明 `8003F710` 数值 helper缺什么，以及 RGB 是否有真实 owner |
| Lane 4 save/lifecycle | save helper dependency graph；loader feedback graph | 能说明哪些是 direct memory、哪些是 card/CD/GPU/SPU HAL |
| Lane 5 renderer adapter | direct output contract | 能说明 renderer 只消费什么，不拥有什么 |

### Batch 2: 翻译

| Lane | 目标文件 | 翻译边界 |
|---|---|---|
| Lane 1 movie/text | `pr_stage1_movie_segment_direct.*`、`pr_stage1_movie_text_outer_loop_direct.*`、`pr_psx_event_frame_direct.*` | segment row bank + text record bank producer |
| Lane 2 event4 | `pr_stage1_fail_prompt_direct.*`、`pr_psx_event_frame_direct.*`、shared sprite submit direct | prompt sprite packet/OT + full frame carrier |
| Lane 3 rail/GTE | `pr_psx_gte_direct.*`、`pr_psx_gs_sprite_submit_direct.*`、`pr_stage1_compact_rail_80024744_direct.*` | RTPT/RTPS numeric output + RGB-known submit |
| Lane 4 save/lifecycle | `pr_stage1_save_ui_direct.*`、`pr_stage1_loader_*`、`pr_stage1_lifecycle_executor_direct.*` | direct action lists + real HAL feedback seams |
| Lane 5 renderer adapter | `pr_stage_scene_submit_backend.*` only after P0 output stable | adapter-only conversion, no PSX branch logic |

### Batch 3: 删除 + 接线

| 删除候选 | 删除前门槛 | 接线形态 |
|---|---|---|
| `TickStage1MovieTextDirectAdapter(...)` 散装字段拼装 | `801C455C` consumes direct segment/text record/work facts | scene 一行调用 outer-loop direct |
| `RenderEvent4PromptNative(...)` / event4 vertical fallback | `800203D4` direct packet/OT submit can feed renderer | UI 只消费 direct packet draw output |
| compact rail `submitPacketGap` / local RGB gap logs | RGB owner and GTE transform fields are known | rail direct calls shared submit helper |
| save UI action gap logging | lower helpers produce direct actions or HAL actions with exact result mapping | scene 只转发 HAL feedback |
| bootstrap opaque host block | loader direct pump consumes record/CD/GPU/SPU feedback | lifecycle executor owns state machine |
| scene submit backend ownership helpers | direct current-state API is complete | backend only draw/resource HAL |

## Stop rules

- 如果某 lane 只能提出 renderer/壳侧 `if` 补丁，立即停止，回到 PSX 函数导出和 dependency 图。
- 如果某字段没有 owner，不得用 C++ 零初始化、默认白色、Win frame、Win STR 状态、renderer fallback 关闭 gap。
- 如果 adapter 开始持有状态或推导 branch，说明边界切错，应扩大 direct-port unit，而不是继续接线。
- 如果两个 lane 共享同一 producer，例如 text record bank，必须指定唯一 owner；另一个 lane 只能消费。
