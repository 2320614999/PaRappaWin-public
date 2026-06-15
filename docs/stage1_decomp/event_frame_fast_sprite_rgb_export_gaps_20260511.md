# EventFrame / FastSprite RGB 证据矩阵（2026-05-11）

范围：本文件保留详细导出项、静态证据、缺口和风险。待办文档只保留未完成动作。内存回放只作为入口定位与现象复核依据，不能作为 RGB 实现权威；RGB 权威必须来自 PSX 代码路径、调用参数、模板字段和 writer closure 的静态证据。

当前导出件：`docs/stage1_decomp/ida_export_event_frame_rgb_boundaries_20260511.txt` 已落地，覆盖 event4 同帧边界、`8001E750 -> 800203D4`、`800203D4` 三次 `8001C550`、`8001C550 -> 8001B590`、event2 `80020568 -> 8001C604`、`8001C604` caller 集、`sub_80023618`、`8001B590/B25C`、`8001B654/B4E0` 与 `8003FA20`。

补充导出件：`docs/stage1_decomp/ida_export_8001b590_xref_family_rgb_owner_20260511.txt` 已落地，覆盖 `8001B590` 上游 xref 族和所有直接 `jal 8001B590` callsite 窗口。

直接 caller 族导出件：`docs/stage1_decomp/ida_export_direct_gssortfastsprite_callers_rgb_owner_20260511.txt` 已落地，覆盖 `GsSortFastSprite` 的全部直接 caller、相关 helper 和 `8003FA20` 消费窗口。

builder/global 补充导出件：
`docs/stage1_decomp/ida_export_fast_sprite_builder_helpers_rgb_tail_20260511.txt`
与
`docs/stage1_decomp/ida_export_fast_sprite_global_object_init_20260511.txt`
已落地，覆盖 `8001B428/8001BE34` helper 字段写入、
`8007CED0` 全局 sprite 对象窗口、`8001BC48/8001BC78` 初始化 helper、
以及 `8007CED0+0x14..+0x16` 的 xref 查询。

stack/template 补充导出件：
`docs/stage1_decomp/ida_export_fast_sprite_stack_template_owner_20260511.txt`
与 `docs/stage1_decomp/ida_export_fast_sprite_template_xrefs_20260511.txt`
已落地，覆盖直接 caller 调用前窗口、`8004E690/80050950/80052DA0`
模板族 0x20/0x24 疑点、`8001B5F4/8001BEE4` 上游 xref，以及模板
`+0x14/+0x15/+0x16/+0x20/+0x24/+0x30` xref 扫描。

upstream stack 补充导出件：
`docs/stage1_decomp/ida_export_fast_sprite_upstream_stack_owner_20260511.txt`
已落地，覆盖 `8001C754/8001C7A8 -> 8001B5F4` 上游 xref、
`8001DF24 -> 8001BEE4` 六个 callsite、`8001DE08/8004800C/8001B590`
前序窗口、`gp+0x368`/`unk_80087288` work-list xref，以及
`8004E6D0..8004E750` 模板族 tail xrefs。

sourceKind 补充导出件：
`docs/stage1_decomp/ida_export_event_frame_sourcekind_owner_20260511.txt`
已落地，覆盖 `800203D4` prompt 三次 `8001C550` submit、
`80020568` 的 `8002081C -> 8001C604` StageSelect slot-loop callsite、
`sub_80023618` 的 `80023C34/54/74/94 -> 8001C604` case16 对照路线、
`8001C604/8001B654/8001B4E0` wrapper/consumer 窗口，以及
`80050950/60/70/80/90`、`80051BF0`、`80052DA0` 模板族 tail xrefs。

stack-tail map 补充导出件：
`docs/stage1_decomp/ida_export_event_frame_stack_tail_map_20260511.txt`
已落地，覆盖 event4/event2/case16 的 `8003FA20` RGB tail 反算、相关
wrapper/submit 窗口、`8003FA20` `+0x14..+0x16` 读取窗口，以及
`8001C550/8001C604/8001B590/8001B654/8003FA20` xref 对照。

old-stack overlap 补充导出件：
`docs/stage1_decomp/ida_export_event_frame_old_stack_overlap_20260511.txt`
已落地，覆盖 event4 `80026B94 -> 8001E750 -> 800203D4`、
event2 `800207B4 -> 8001C5A8` 到 `8002081C -> 8001C604`、
case16 helper 窗口、`off_80054564` 数据窗口，以及 stack/call summary。

event4 callback 补充导出件：
`docs/stage1_decomp/ida_export_event4_callback_80025e6c_20260511.txt`
已落地，覆盖 `off_80054564` 的 event4 table 解析、`80025E6C`
per-frame callback、`800267C8` init、`80025F0C` input handler、
`80026B94` callback callsite 和 `8001E750` case4 prompt callsite。

event-frame RGB follow-up 导出件：
`docs/stage1_decomp/ida_export_event_frame_rgb_followup_20260511.txt`
已落地，覆盖 event4 `80025E6C -> 80026EF8/80026ECC -> 80034240`
SFX/audio 下游、`8009441C` cue payload xrefs、以及 event2
`800205C4-800207B8` 前序 submit 窗口和 `800205C4-80020824`
store/call scan。

event4 `80034240` stack-chain 校正导出件：
`docs/stage1_decomp/ida_export_event4_stack_chain_80034240_20260511.txt`
已落地，用完整 prologue 校验 `80034240` callee-save stack store 与
Prompt FastSprite RGB tail 是否同址。

recursive stack-store 补充导出件：
`docs/stage1_decomp/ida_export_event_frame_recursive_stack_store_hits_20260511.txt`
已落地，修正为仅跟踪 direct `jal`，覆盖 event4/event2 目标 RGB tail
的递归 stack store 命中扫描。

event4 input/flush 候选补充导出件：
`docs/stage1_decomp/ida_export_event4_input_flush_stack_candidate_20260511.txt`
已落地，覆盖 `80026B94` input handler 顺序、
`80025F0C -> 80025C8C -> 80026ECC -> 8002EFF4 -> 80032B00`
和 `80032B34` prologue store 命中窗口。

event4 `$s1` provenance 补充导出件：
`docs/stage1_decomp/ida_export_event4_s1_provenance_20260511.txt`
已落地，覆盖 `off_80054564` 表、`80032B34` 写入的 `$s1` 值、
little-endian 字节影响，以及候选链中各函数对 `$s1` 的读写。

event2 caller old-stack 补充导出件：
`docs/stage1_decomp/ida_export_event2_stage_select_caller_old_stack_20260511.txt`
已落地，覆盖 `80020568` 的唯一 xref `8001E8BC -> 80020568`、
`8001E750` case2 pre-call 窗口，以及 caller 内 stack store 摘要。

event2 `8001D74C` old-stack 复核导出件：
`docs/stage1_decomp/ida_export_event2_d74c_old_stack_20260511.txt`
已落地，覆盖 `8001E750 case2 -> 8001D74C(3, gp+0x368) -> 80020568`
紧邻路径、`8001D74C` 的 stack stores/calls、`8001B590/8001B25C`
窗口与 `8003FA20` consumer 窗口。

## event4 Prompt 证据矩阵

| 项 | 已落地导出证据 | 当前结论 | 仍存缺口/风险 |
| --- | --- | --- | --- |
| 同帧边界 | `80026B94-80026E2C` 与重点窗口 `80026D40-80026DDC`。 | `v4[2]` per-frame callback 后调用 `sub_8001E750(a1, v4[4])`，随后才进入 stage clear text、`sub_80035560(0)`、`sub_8001EA00(a1)`、`sub_800436F0(-1)`。 | 同帧提交边界已固定，但 RGB owner 仍不在该边界本身。 |
| prologue 到 `800203D4` | `8001E750-8001E870` 伪 C/汇编。 | `a1 == 4` 且 `gp+0x38C == 0` 时进入 `sub_800203D4(*a2)`；其它分支走 fade/clear 路线。 | event4 入口已闭合到 prompt submit，不能再把入口缺口当阻塞；阻塞已后移到 fast-sprite RGB owner。 |
| 三次 `8001C550` | `800203D4-80020488`。 | 三次 prompt submit 分别使用 `(56,57,dword_80050950)`、`(70,149,dword_80050960/80050980)`、`(178,152,dword_80050990/80050970)`，`a3` 均为 `0`。 | 模板指针已明确，但模板/stack RGB 来源仍未证明；不能用回放色值反推。 |
| `8001C550` 参数传递 | `8001C550-8001C5A8`。 | wrapper 把坐标、模板、zero 参数和当前 OT buffer `unk_80087288 + 20 * gp+0x368` 转交 `8001B590`。 | wrapper 不生成 RGB owner；后续应追 `8001B590` 上游 xref 族是否存在 stack-local RGB tail 约定。 |
| writer clean 窗口 | `8001B590`、`8001B25C`、`8003FA20`。 | `8001B590` 写 local x/y 后调用 `8001B25C` 和 `GsSortFastSprite`；`8001B25C` 写 `+0x0..+0x12` 的 texture/size/UV 字段；`GsSortFastSprite` 从 `a1+0x14..0x16` 读 RGB 并写 packet word2。 | 当前 closure 没有写 `+0x14..+0x16`；RGB owner 不在本 closure 内，必须追 `8001B590` 其它上游调用族和 stack lifetime。 |

## event2 StageSelect 证据矩阵

| 项 | 已落地导出证据 | 当前结论 | 仍存缺口/风险 |
| --- | --- | --- | --- |
| `80020568` callsite | `80020568` 函数与 `0x8002081C -> 8001C604` callsite。 | StageSelect event2 会经 `sub_8001C604` 进入 `sub_8001B654` fast-sprite path；callsite 前在 stack 上传递模板/offset/状态参数。 | event2 单点参数不能推广到所有 `8001C604` caller；仍需确认 shared helper 的 RGB owner。 |
| `8001C604` caller 集 | xrefs/caller 表含 `8002081C/80023C34/80023C54/80023C74/80023C94`。 | `8002081C` 来自 event2 StageSelect；`80023C34/54/74/94` 来自 `sub_80023618` case 路线。 | 多 caller 共用 submit helper，后续实现必须按 caller/sourceKind 区分，不能只按 helper 名打开 RGB authority。 |
| `sub_80023618` case 对照 | 函数级伪 C 与 `80023C00-80023CB0` callsites。 | `case16` 路线里四次 `8001C604` 使用同一 `dword_80052DA0` 族模板和不同 x/UV offset 参数。 | case 语义与 event2 不同，不能把某个入口的 RGB 行为泛化到所有 case。 |
| closure writer | `8001B654`、`8001B4E0`、`8003FA20`。 | `8001B654` 与 `8001B4E0` 路线同样只构造 texture/size/UV 等字段，`GsSortFastSprite` 仍消费 `a1+0x14..0x16` RGB。 | 当前 closure 仍是 RGB consumer path，不是 RGB owner。 |

## sourceKind owner 补充结论

| 项 | 已落地导出证据 | 当前结论 | 仍存缺口/风险 |
| --- | --- | --- | --- |
| event4 Prompt | `ida_export_event_frame_sourcekind_owner_20260511.txt` 中 `800203D4` 与 `80050950/60/70/80/90` xrefs。 | 三次 submit 均可归档为 `Stage1EventFramePrompt`：title 使用 `(56,57,dword_80050950)`；左/中选项按 `ctx0` 选择 `80050960/80050980`，坐标 `(70,149)`；右选项按 `ctx0` 选择 `80050990/80050970`，坐标 `(178,152)`；`a3` 均为 `0`。 | sourceKind 与模板已闭合，但 `8001C550 -> 8001B590 -> 8001B25C -> 8003FA20` 仍没有 `+0x14..+0x16` RGB writer 正证据。 |
| event2 StageSelect | 同导出中 `80020568` 的 `8002081C -> 8001C604` callsite 与 `80051BF0/80053104/800531D0` 数据窗口。 | `8002081C` 只属于 event2 StageSelect slot-loop：模板为 `dword_80051BF0`，坐标来自 `byte_80053104/word_80053106`，UV/offset/状态参数来自 `byte_800531D0/word_800531D2` 与本地 slot 状态；不能按 `8001C604` helper 全局放行。 | sourceKind/模板来源已闭合到具体 caller，但 `8001C604 -> 8001B654 -> 8001B4E0 -> 8003FA20` 仍没有 RGB tail writer。 |
| `sub_80023618` case16 对照 | 同导出中 `80023C34/54/74/94 -> 8001C604` callsite 与 `80052DA0` 数据窗口。 | case16 是独立对照 sourceKind：四次固定调用 `(65,81)/(122,81)/(178,81)/(234,81)`，共用 `dword_80052DA0`，`a3` 依次 `0/7/0xE/0x15`，stack 参数固定为 `7,0,0`。 | 它只能证明 `8001C604` 有多 sourceKind caller，不能把 case16 的任何行为推广给 event2。 |
| 模板 tail xrefs | 同导出中 `80050950`、`80051BF0`、`80052DA0` 的 `+0x14/+0x15/+0x16/+0x24` xrefs。 | prompt、event2 StageSelect、case16 模板族 tail xrefs 均为负；`+0x20/+0x30` 是后续 16-byte 模板入口，不是同一 FastSprite 记录的 RGB 扩展字段。 | 模板路线仍不提供 RGB owner；不能从模板窗口外延、默认白、zero-init 或回放采样打开 packet word2 authority。 |

## stack-tail map 补充结论

| 项 | 已落地导出证据 | 当前结论 | 仍存缺口/风险 |
| --- | --- | --- | --- |
| event4 Prompt tail map | `ida_export_event_frame_stack_tail_map_20260511.txt` 的 stack notes 与 `800203D4/8001C550/8001B590` 窗口。 | `8001B590` local RGB tail 是 child `sp+0x24..0x26`，即 `8001B590` entry `sp-0x24..-0x22`；穿过 `8001C550` 与 `800203D4` 后，反算到 `800203D4` entry `sp-0x64..-0x62`。第一条 prompt submit 前 `800203D4` 内没有前序 callee 能正常写这个区间。 | 这说明 RGB tail 不在 `800203D4/8001C550/8001B590` 的普通 caller-local 字段内；但 old-stack/residue 仍不是实现 authority，除非继续静态闭合实际写入/调用顺序。 |
| event2 StageSelect tail map | 同导出中的 `80020568` slot-loop、`8001C604`、`8001B654` 窗口。 | `8001B654` local RGB tail 是 child `sp+0x2C..0x2E`，即 `8001B654` entry `sp-0x24..-0x22`；穿过 `8001C604` 与 `80020568` 后，反算到 `80020568` entry `sp-0xE4..-0xE2`，不在 `80020568` frame 的 `v27` 或 outgoing arg 正常字段内。 | StageSelect slot-loop 前的 `8001C5A8` 只证明同帧前序调用存在，不证明它写入同一物理 tail；需要进一步按 stack depth/前序 callee 逐项闭合。 |
| case16 tail map | 同导出中的 `sub_80023618` case16 四个 `8001C604` callsite。 | case16 经 `8001B654` 同型 tail，反算到 `sub_80023618` entry `sp-0x8C..-0x8A`，不在 `sub_80023618` 的 local `v11/v12` 或 outgoing arg 正常字段内。 | case16 仍只能作为多 caller 对照；不能把它的 old-stack 区间推广为 event2 RGB owner。 |
| `8003FA20` consumer | 同导出中的 `consumer_8003FA20_rgb_read` 窗口。 | 汇编再次确认 packet word2 使用 `lbu 0x16/0x15/0x14($t3)`，也就是 FastSprite input 的 RGB tail；它只消费，不写入。 | gate 仍必须等待静态 producer/sourceKind closure；不能用 renderer 或 replay 替代。 |

注：上表统一使用函数 entry `sp` 表示。若按函数 prologue 后的 current
`sp` 表示，event4 tail 在 `800203D4` 当前帧为 `sp-0x4C..-0x4A`，
event2/case16 tail 在各自 caller 当前帧为 `sp-0x4C..-0x4A`；两种写法是
同一物理地址，不能混读为两个不同候选区间。

## old-stack overlap 补充结论

| 项 | 已落地导出证据 | 当前结论 | 仍存缺口/风险 |
| --- | --- | --- | --- |
| event4 table | `ida_export_event_frame_old_stack_overlap_20260511.txt` 与 `ida_export_event4_callback_80025e6c_20260511.txt`。 | `off_80054564` 以 `80054564` 为 table 起点：`v4[0]=800267C8` init，`v4[1]=80025F0C` input，`v4[2]=80025E6C` per-frame callback，`v4[3]=0`，`v4[4]=8006ED74` context。 | table/callback source 已闭合，但 RGB tail owner 仍未闭合。 |
| event4 loop overlap | 同导出中 `80026D50` 与 `80026D5C` 窗口。 | `80026B94` 每帧若 `sub_80026744()` 返回非 0，则先调用 `v4[1]` input handler；随后每帧 `jalr v4[2]`，再 `80026D5C -> 8001E750`。该窗口本身没有栈 store 命中目标 tail，只确认 callback/submit 顺序。 | input handler 路线的 callee-save 同址候选已闭合为非 owner。 |
| `80025E6C` callback | `ida_export_event4_callback_80025e6c_20260511.txt`。 | `80025E6C` 只读取/更新 `gp+0x32C` 和 `gp+0x328`，按计数调用 `80026EF8(dword_8009441C[/+6])` 与 `80026ECC()`；本体只有 `ra` 栈保存，未写目标 RGB tail。 | 不能把 `80025E6C` 本体当 owner；下一步应导出 `80026EF8/80026ECC` 及其 callees 的 frame/store map。 |
| `80026EF8/80026ECC` 下游 | `ida_export_event_frame_rgb_followup_20260511.txt` 与 `ida_export_event4_stack_chain_80034240_20260511.txt`。 | `80026EF8` 读 cue payload `a0+0/+1/+3`，写 `a0+2`、调用 `80034240(...)`，并写 `word_800943AC`；`80026ECC` 只读 `dword_800943B4` 并在为 0 时调用 `8002EFF4()`。`80034240` 确有 callee-save stack store：`sw $s1, sp+0x14`，但按 `80026B94` current `sp=S` 反算落在 `S-0x64..S-0x61`；Prompt RGB tail 因 `8001E750` 实际 frame `-0x30`，经 `800203D4/8001C550/8001B590` 落在 `S-0x94..S-0x92`，不是同一物理地址。 | event4 callback SFX/audio 下游可作为 RGB owner 路线降级；不能把 callee-save residue、`8009441C` cue payload 或 SPU 状态当 packet `word2` authority。 |
| event4 input/flush callee-save 候选 | `ida_export_event_frame_recursive_stack_store_hits_20260511.txt`、`ida_export_event4_input_flush_stack_candidate_20260511.txt` 与 `ida_export_event4_s1_provenance_20260511.txt`。 | 修正后的 direct-`jal` 扫描发现：当 `80026B94` 的 `v4[1]=80025F0C` input handler 收到 `a1==64` 或 `a1==32` 时，会调用 `80025C8C(32/64)`，再经 `80026ECC -> 8002EFF4 -> 80032B00`；`80032B34: sw $s1, 0x20+var_s4($sp)` 映射到 `80026B94` current `S-0x94..S-0x91`。`$s1` 已闭合为 `80026B94` case4 设置的 `off_80054564 = 0x80054564`；little-endian 保存字节为 `64 45 05 80`，若被当作 RGB tail 会是 `64 45 05`。 | 这是 event4 table 指针的 callee-save residue，不是 PSX 语义 RGB writer；该候选降级，不能打开 Prompt packet `word2` gate。 |
| event2 same-tail overlap | `ida_export_event_frame_old_stack_overlap_20260511.txt` 中 `800207B4 -> 8001C5A8` 与 `8002081C -> 8001C604` 窗口。 | 前序 `800207B4 -> 8001C5A8 -> 8001B590` 的 B590 RGB tail 与后续 StageSelect `8002081C -> 8001C604 -> 8001B654` tail 映射到 `80020568` current `sp-0x4C..-0x4A` 同一物理区间；按 `80020568` entry / `8001E750` callsite 表示则是 `sp-0xE4..-0xE2`。 | 这只证明 same-tail overlap；`8001C5A8/8001B590/8001B25C` 仍未写 `+0x14..+0x16`，不能作为 RGB owner 正证据。 |
| event2 inter-call stores | 同导出中 `80020784-80020824` stack/call summary。 | `800207B4` 到 `8002081C` 之间仅有 `sp+0x10/+0x18/+0x1C` outgoing arg stores 和 local address setup，没有直接写 current `sp-0x4C..-0x4A` 的 store/copy。 | 若要继续追 event2，只能追前序 `8001C5A8 -> 8001B590` 的 old-stack producer，而不是把 overlap 当常量。 |
| event2 prior submit window | `ida_export_event_frame_rgb_followup_20260511.txt` 与 `ida_export_event_frame_recursive_stack_store_hits_20260511.txt`。 | 前序窗口包含 `800205C4 -> 8001C5A8`、`800205DC/800206A4/800206BC/800206E8/80020700 -> 8001C550`、`800207B4 -> 8001C5A8`；修正后的 direct-`jal` 递归扫描确认这些 direct submit roots 到 `8001B590/8001B25C/8003FA20` 均未命中 `80020568` current `sp-0x4C..-0x4A`。 | event2 仍只能证明前序 submit/call 顺序；每个 submit 的 local FastSprite RGB tail owner 未闭合，不能打开 StageSelect packet `word2` gate。 |
| event2 caller pre-call | `ida_export_event2_stage_select_caller_old_stack_20260511.txt`。 | `80020568` 只有一个 xref：`8001E8BC -> 80020568`，位于 `8001E750` case2。case2 直接顺序为 `8001E8B4 -> 8001D74C(3, gp+0x368)`，随后 `8001E8BC -> 80020568(a2)`；caller 本体仅见 prologue 保存和其它 case 的 outgoing/global stores，没有直接写 StageSelect tail 的栈 store。 | event2 不应再重复扫描 `80020568` 内部 submit roots；下一步若继续追，只能用正确 frame math 复核紧邻 `8001D74C` 是否可能留下同一物理 old-stack residue。 |
| event2 `8001D74C` tight follow-up | `ida_export_event2_d74c_old_stack_20260511.txt`。 | 校正导出显示 `8001E750` frame `0x30`、`8001D74C` frame `0x48`、`80020568` frame `0x98`；StageSelect tail 在 `80020568` current 为 `sp-0x4C..-0x4A`，在 `80020568` entry / `8001E750` callsite 为 `sp-0xE4..-0xE2`。`8001D74C` 只有 callee-save 与 `8001B590` outgoing arg stores，并经 84 个 `8001B590` backdrop submit 进入已知 consumer-only closure。 | `8001D74C -> 8001B590` 只能作为 backdrop/allocator 顺序来源，不能作为 StageSelect RGB owner；不再沿 B590/B25C/FA20 submit helper 重复扫描。 |
| S0 StageSelect replay correction | `fast_sprite_8003fa20_entry_trace_20260511.md` 的动态验证更新。 | 当前命中链为 `8002081C -> 8001C604 -> 8001B654 -> 8001B4E0 -> 8003FA20`，`8003FA20` 实际输入指针为 `0x801FFE30`，本次 RGB tail 是 `0x801FFE44..0x801FFE46`。对该 tail 设置 write watchpoint 后，下一次 StageSelect submit 仍直接进入 `8003FA20`，未先命中 RGB 写入。 | 早先的 `0x801FFEDC` 写入不是这次 FastSprite RGB tail，而是同栈附近 StageSelect 局部状态/数组写入；不能把它当 RGB owner，也不能用回放解码色值 `0/0/1` 授权 `colorAuthoritative`。 |

## FastSprite RGB Authority Gate 风险

| 风险项 | 当前判断 | 风险 | 下一步要求 |
| --- | --- | --- | --- |
| `colorAuthoritative` 裸 gate | EventFrame fast-sprite draw gate 不应只依赖 `colorAuthoritative`。 | 只靠布尔 gate 会把来源不同的颜色路径混成同一权威通道，容易让内存回放值或临时静态对象绕过 PSX 证据链。 | 后续 gate 必须绑定 `sourceKind`，按 PSX event/frame/template/submit 来源区分权威等级。 |
| `StaticGlobalObject` | `StaticGlobalObject` 已降级为候选标签，不能作为裸权威通行证。 | 静态全局对象只能说明入口或对象来源，不等于 RGB 字段由 PSX fast-sprite 路径权威写入。 | `StaticGlobalObject` 不能单独打开 RGB authoritative path；必须有具体 producer/sourceKind closure。 |
| 内存回放 | 内存回放只用于入口定位。 | 若把回放 RGB 当实现权威，会绕开 caller 参数、模板字段和 writer closure 的静态验证，导致实现与 PSX 代码不一致。 | 所有 RGB 实现依据必须回到 IDA 导出的 callsite、参数传递、模板字段和 writer clean 窗口。 |

## `8001B590` xref 族结论

| 组 | caller | 当前结论 | 下一步 |
| --- | --- | --- | --- |
| wrapper | `8001C550` / `8001C5A8` | 只把坐标、desc、priority、work list 转交 `8001B590`，无 RGB writer。 | 不在 wrapper 上补色。 |
| background / tile batch | `8001C864` / `8001CE30` / `8001D74C` | 大量固定 tile/border submit 都经 `8001B590 -> 8001B25C -> 8003FA20`，未发现写 local `+0x14..+0x16`。 | 作为负证据，不推广默认色。 |
| prompt/menu batch | `8001DB9C` / `8001DE08` / `8001DF24` / `8001E408` / `8001E5A4` / `8001EDBC` | 同属 `8001B590` submit 族，当前无显式 RGB writer。 | 继续查直接 `GsSortFastSprite` caller 族作对照。 |

## 直接 `GsSortFastSprite` caller 族结论

| caller | 导出状态 | 当前结论 | 仍存缺口/风险 |
| --- | --- | --- | --- |
| `8001B590` | 已导出函数窗口与 `8001B5D4` callsite。 | 仍是 `sub_8001B25C` helper 构造字段后提交；本函数只写 x/y 与转交 OT/priority。 | event4 prompt 的模板/stack RGB owner 仍不在 `8001B590` wrapper 本体。 |
| `8001B5F4` | 已导出函数窗口与 `8001B634` callsite；已补导 `8001B428`。 | 通过 `sub_8001B428` 构造 local sprite 后提交；`8001B428` 只写 `+0x00/+0x08/+0x0A/+0x0C/+0x0E/+0x0F/+0x10/+0x12`，没有写 RGB tail。 | 负证据；不能把 helper 构造字段当 `+0x14..+0x16` RGB owner。 |
| `8001B654` | 已导出函数窗口与 `8001B6A4` callsite。 | 通过 `sub_8001B4E0` 构造 local sprite 后提交；`sub_8001B4E0` 只写 texture/UV/size 侧字段，未写 `GsSortFastSprite` 消费的 `a1+0x14..+0x16`。 | event2 StageSelect 仍缺 RGB owner，不能按 `8001B4E0` helper 全局放行。 |
| `8001B744` | 已导出函数窗口与 `8001B904` callsite。 | glyph/text 路径在 caller 本地写 texture/UV 侧字段后提交；未在 callsite 窗口看到 `a1+0x14..+0x16` RGB writer。 | 属于文本 glyph 路线，当前只能作为负证据对照，不能反推 event2/event4。 |
| `8001B954` | 已导出函数窗口与 `8001BBF8` callsite。 | glyph loop 同样只看到 texture/UV 侧字段写入后提交，未闭合 `a1+0x14..+0x16` RGB owner。 | P1 glyph packet owner 仍未闭合，需继续追 stack tail 或共享初始化来源。 |
| `8001BCA0` | 已导出函数窗口与 `8001BDEC` callsite；已补导 `8001BC48/8001BC78`、`8007CED0` 数据窗口和 tail xrefs。 | 全局 sprite `dword_8007CED0` 路线只证明 `+0x00/+0x04/+0x06/+0x08/+0x0A/+0x0C/+0x0E/+0x0F/+0x10/+0x12` 侧字段写入；`8007CED0+0x14/+0x15/+0x16` 无 xref。 | 不能作为 `StaticGlobalObject` 裸 gate；全局对象 tail 仍不是 event2/event4 RGB owner 正证据。 |
| `8001BEE4` | 已导出函数窗口与 `8001BF1C` callsite；已补导 `8001BE34`。 | 通过 `sub_8001BE34` 构造 local sprite 后提交；`8001BE34` 只写到 `+0x12`，没有写 RGB tail。 | 负证据；不能把 helper 构造字段当 `+0x14..+0x16` RGB owner。 |
| `8003FA20` | 已导出消费窗口。 | `GsSortFastSprite` 从 `a1+0x14/+0x15/+0x16` 读 RGB 并写 packet word2；它是 consumer，不是 owner。 | 所有实现 gate 仍必须追到 caller/helper 的 writer closure。 |

## builder / global 补充结论

| 项 | 已落地导出证据 | 当前结论 | 仍存缺口/风险 |
| --- | --- | --- | --- |
| `8001B428` | `ida_export_fast_sprite_builder_helpers_rgb_tail_20260511.txt`。 | 与 `8001B25C/8001B4E0` 同型，只构造 prefix/texture/UV/size 字段到 `+0x12`；Hex-Rays 的 `a1+14/15/16` 是十进制 `+0x0E/+0x0F/+0x10`。 | 不是 RGB tail owner。 |
| `8001BE34` | `ida_export_fast_sprite_builder_helpers_rgb_tail_20260511.txt`。 | 与 `8001B428` 同型，只构造到 `+0x12`；没有 `+0x14..+0x16` writer。 | 不是 RGB tail owner。 |
| `8001BC48/8001BC78` | `ida_export_fast_sprite_global_object_init_20260511.txt`。 | `8001BC48` 从 `dword_8004E690` 初始化 `8007CED0+0x00`，并写 `+0x10/+0x12`；`8001BC78` 写 graph/global 参数和 `+0x08/+0x0A`。 | 仍没有 `8007CED0+0x14..+0x16` writer。 |
| `8007CED0` global object | `ida_export_fast_sprite_global_object_init_20260511.txt`。 | IDA 数据窗口显示静态 bytes 为 `0xFF`，但 `8007CEE4/CEE5/CEE6` 没有 xref；这只能说明 tail 未在已知静态 writer 中闭合。 | 不能把未写 tail 或 IDB 初始字节当 event-frame packet word2 authority。 |

## stack/template 补充结论

| 项 | 已落地导出证据 | 当前结论 | 仍存缺口/风险 |
| --- | --- | --- | --- |
| `8001B5F4` stack lifetime | `ida_export_fast_sprite_stack_template_owner_20260511.txt`。 | local base 是 `sp+0x10`；helper 前只写 `+0x04/+0x06`，`8001B428` 只写到 `+0x12`；submit 前只重装 `a0/a1/a2`。 | 函数内不是 RGB owner；若要继续追，只能追 caller 入口前旧栈/上游来源。 |
| `8001BEE4` stack lifetime | `ida_export_fast_sprite_stack_template_owner_20260511.txt`。 | local base 是 `sp+0x10`；helper 前只写 `+0x04/+0x06`，`8001BE34` 只写到 `+0x12`；submit 前只重装 `a0/a1/a2`。 | 函数内不是 RGB owner；若要继续追，只能追 caller 入口前旧栈/上游来源。 |
| `8001B744/8001B954` glyph/text stack | `ida_export_fast_sprite_stack_template_owner_20260511.txt`。 | 两条 glyph/text 路径的 local base 都是 `sp+0x10`；`8003FA20` 读取的 RGB 是 `sp+0x24..0x26`；调用前窗口只看到写到 `sp+0x22`。 | 只能作为负面对照，不能推广到 event2/event4，也不能推出默认色。 |
| prompt 模板族 `80050950/60/70/80/90` | `ida_export_fast_sprite_template_xrefs_20260511.txt`。 | 各模板 `+0x14/+0x15/+0x16` 与 `+0x24` 无 xref；`+0x20/+0x30` xref 指向同族后续 16-byte 模板起点。 | 没有扩展 RGB tail 正证据；不能从 0x20/0x24 外延补 packet word2。 |
| event2 case16 模板族 `80052DA0` | `ida_export_fast_sprite_template_xrefs_20260511.txt`。 | `+0x14/+0x15/+0x16` 与 `+0x24` 无 xref；`+0x20/+0x30` 是后续模板入口。 | 不能作为 `8001C604 -> 8001B654` RGB owner。 |

## upstream stack 补充结论

| 项 | 已落地导出证据 | 当前结论 | 仍存缺口/风险 |
| --- | --- | --- | --- |
| `8001C754 -> 8001B5F4` | `ida_export_fast_sprite_upstream_stack_owner_20260511.txt`。 | `8001C754` 本身无上游 xref；它只转发参数并构造 `unk_80087288 + 20 * gp+0x368` work-list。 | 作为 `8001B5F4` 上游 RGB owner 路线可降级；没有 caller 入口前 writer 可追。 |
| `8001C7A8 -> 8001B5F4` | 同上。 | 只有 `80020BE4` 一个上游 xref；`8001B5F4` 的 RGB tail `child_sp+0x24..0x26` 换算到 `8001C7A8` 当前 `sp` 为 `sp-0x24..-0x22`，不在 wrapper frame 内。 | 仍需闭合 `80020BE4` 是否确定写入该反算旧栈区间；旧栈残留本身不是 authority。 |
| `8001DF24 -> 8001BEE4` 六个 callsite | 同上。 | `8001BEE4` 的 RGB tail `child_sp+0x24..0x26` 换算到 `8001DF24` 当前 `sp` 为 `sp-0x1C..-0x1A`，不在 `8001DF24` frame 内；六个 callsite 只重建 x/y/template/enable/work 参数。 | 本层未证明 RGB owner；若继续追，只能追前序 callee 或更上层是否确定写入同一物理旧栈区间。 |
| `8001DE08` 前序 helper | 同上。 | `8001DE08` frame `0x28`，内部只写 `ra`、`gp+0xB8`、`sub_8001B590` outgoing args，并调用 `8001B590`；没有确定写入后续 `8001BEE4` tail 的 store/copy。 | 只能作为负证据；不能把它的 `8001B590` 提交色推广给后续 `8001BEE4`。 |
| `8004800C("%d")` | 同上。 | `8004800C` 作为格式化输出写调用者提供的 `8001DF24` `sp+0x20` 字符串 buffer；其自身大 frame 内大量局部写入不等于写入 `8001BEE4` tail。 | 需要避免把可变参数函数的内部栈污染误判为 RGB owner；除非证明越界/目标指针覆盖同一 tail，否则不是 authority。 |
| digit loop `8001B590` | 同上。 | `8001DF24` 中 digit loop 的 `8001B590` tail 换算到本层为 `sp-0x14..-0x12`，与后续 `8001BEE4` tail `sp-0x1C..-0x1A` 不同址。 | 不能作为下一次 `8001BEE4` RGB owner。 |
| `gp+0x368` / `unk_80087288` | 同上。 | `gp+0x368` 写者和 `unk_80087288` xrefs 表明这是当前 draw work-list/OT slot；`80087288+0x14/+0x20/+0x40/+0x50/+0x64` 等未见 RGB-tail owner xref。 | work-list 指针池不是 FastSprite local RGB scratch；不能打开 packet word2 authority。 |
| `8004E6D0..8004E750` 模板族 | 同上。 | 各 16-byte 模板记录的 `+0x14/+0x15/+0x16` 与 `+0x24` 仍无 xref；`+0x20/+0x30` 继续是后续模板入口。 | 不构成 `8001BEE4` 或 digit loop 的 RGB tail owner。 |

## 下一步 IDA 导出清单

1. 已完成：`ida_export_event_frame_rgb_boundaries_20260511.txt` 覆盖原清单 1-9。
2. 已完成：`ida_export_8001b590_xref_family_rgb_owner_20260511.txt` 覆盖 `8001B590` 上游 xref 族；当前结论仍是无显式 RGB writer。
3. 已完成：`ida_export_direct_gssortfastsprite_callers_rgb_owner_20260511.txt` 覆盖 `8001B5F4/8001B654/8001B744/8001B954/8001BCA0/8001BEE4` 全函数与 callsite。
4. 已完成：`ida_export_fast_sprite_builder_helpers_rgb_tail_20260511.txt` 覆盖 `8001B428/8001BE34` helper 字段写入；当前结论仍是无 RGB tail writer。
5. 已完成：`ida_export_fast_sprite_global_object_init_20260511.txt` 覆盖 `8001BC48/8001BC78` 与 `8007CED0` tail xrefs；当前结论仍是无 `+0x14..+0x16` writer。
6. 已完成：`ida_export_fast_sprite_stack_template_owner_20260511.txt` 覆盖 `8001B5F4/8001BEE4/8001B744/8001B954` 调用前窗口；当前结论仍是无函数内 RGB tail writer。
7. 已完成：`ida_export_fast_sprite_template_xrefs_20260511.txt` 覆盖 `8004E690/80050950/80052DA0` 模板族 0x20/0x24 疑点与 xrefs；当前结论仍是无模板扩展 RGB tail 正证据。
8. 已完成：`ida_export_fast_sprite_upstream_stack_owner_20260511.txt` 覆盖 `8001C754/8001C7A8/8001DF24` 上游 stack owner 路线、`8001DE08/8004800C` 前序窗口、`gp+0x368/unk_80087288` 和 `8004E6D0..8004E750` 模板族；当前仍无静态 RGB owner 正证据。
9. 已完成：`ida_export_event_frame_sourcekind_owner_20260511.txt` 闭合 event2 `80020568 -> 8001C604` StageSelect caller、`sub_80023618` case16 对照 caller、event4 prompt 三次 `8001C550` submit 和相关模板族；sourceKind/模板来源已可按具体 caller 区分，不能按 helper 全局放行。
10. 已完成：`ida_export_event_frame_stack_tail_map_20260511.txt` 反算 event4/event2/case16 的 RGB tail 均落在上游 entry-sp 以下的 old-stack 区间，而非当前函数普通 local/outgoing 字段；当前仍只构成负证据和下一步范围收窄。
11. 已完成：`ida_export_event_frame_old_stack_overlap_20260511.txt` 导出 event4 `80026B94 -> 8001E750 -> 800203D4` 与 event2 `800207B4 -> 8001C5A8` 到 `8002081C -> 8001C604` old-stack overlap scan；event2 只证明 same-tail overlap，仍无 RGB writer。
12. 已完成：`ida_export_event4_callback_80025e6c_20260511.txt` 闭合 event4 table `off_80054564`：`v4[2]=80025E6C`，`v4[4]=8006ED74`；`80025E6C` 本体无 RGB tail writer。
13. 已完成：`ida_export_event_frame_rgb_followup_20260511.txt` 与 `ida_export_event4_stack_chain_80034240_20260511.txt` 导出 event4 `80026EF8/80026ECC/80034240` 与 event2 `800205C4-800207B8` prior-submit 窗口；event4 `80034240` callee-save store 命中 `S-0x64`，Prompt RGB tail 是 `S-0x94`，不是同址；event2 prior window 仍无目标 RGB tail writer。
14. 已完成：`ida_export_event_frame_recursive_stack_store_hits_20260511.txt` 以 direct `jal` 修正递归 stack-store 扫描；event2 prior submit roots 未命中目标 tail，event4 input/flush 路线发现 `80032B34` 同址 callee-save 候选。
15. 已完成：`ida_export_event4_input_flush_stack_candidate_20260511.txt` 闭合候选路径条件和顺序：`80026B94` 的 input handler `v4[1]=80025F0C` 仅在 `sub_80026744()` 返回输入且 `a1==32/64` 时经 `80025C8C -> 80026ECC -> 8002EFF4 -> 80032B00` 触发；`80032B34` 写入位于 prompt submit 前的同一帧循环内。
16. 已完成：`ida_export_event4_s1_provenance_20260511.txt` 闭合 `80032B00` 候选的 `$s1` 来源；它是 `80026B94` event4 table 指针 `off_80054564` 的 callee-save residue，不是语义 RGB writer。
17. 已完成：`ida_export_event2_stage_select_caller_old_stack_20260511.txt` 确认 `80020568` 唯一 caller 为 `8001E750` case2 的 `8001E8BC`，直接前序是 `8001D74C(3, gp+0x368)`；caller 本体未见直接写 StageSelect tail 的栈 store。
18. 已完成：`ida_export_event2_d74c_old_stack_20260511.txt` 复核 `8001E750 case2 -> 8001D74C -> 80020568` 紧邻路径；`8001D74C` 只提供 backdrop partial submit 顺序，未闭合 StageSelect RGB owner。
19. 未完成：event2/event4 仍需闭合 `8003FA20` packet word2 的 RGB owner；当前 sourceKind、模板、helper、global object、stack/upstream/stack-tail/old-stack-overlap/follow-up/D74C 导出均不能授权 `colorAuthoritative` 裸 gate、默认色、zero-init、回放采样或 renderer 补色。
