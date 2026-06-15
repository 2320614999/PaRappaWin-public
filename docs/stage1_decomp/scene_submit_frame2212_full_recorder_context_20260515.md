# Scene Submit frame 2212 full recorder context

Scope: Stage1 Scene Submit / TOD `80028054 coordNode+0x44`. This note extends the frame `2212` call-trace window with the available recorder channels. It is still not `800280B4` register/read authority.

Recording:

- `E:\game\PSgame\parappa the rapper\duckstation\bin\x64\标题画面-menu-选关选stage1-good通关-保存记忆卡（过场动画均为跳过）.psxrec`

Raw artifact:

- `artifacts/coord44_backing_recorder/coord44_frame2212_full_context_20260515.json`

Recorder channel availability at frame `2212`:

| channel | count | authority boundary |
| --- | ---: | --- |
| call trace | `4486` events | has call/return `pc/target/sp/ra/event_seq`; no GPR |
| PC trace | `4096` branch-target PCs | no event seq, no GPR |
| full branch trace | `24563` entries | branch targets only; sequential instructions such as `800280B4` are absent |
| branch results | `2048` entries | branch decisions only |
| write log | `13084` entries | confirms stores and write PCs |
| read log | `0` entries | cannot infer `lw $s0,0x44($v0)` address/value |

Frame-header registers:

`pc=8003B958`, `v0=0000006F`, `a0=800C0378`, `a1=800C0C38`, `a2=801AD170`, `sp=801FFDE0`, `ra=80042E18`.

These are frame-boundary registers only. They are not the register state at `800280B4`.

Upstream call chain:

| seq | type | pc | target | sp | ra |
| ---: | --- | --- | --- | --- | --- |
| `1893` | call | `801C82F0` | `801C7A60` | `801FFF88` | `801C82F8` |
| `1897` | call | `801C7A8C` | `801C79E4` | `801FFF50` | `801C7A94` |
| `1903` | call | `801C7A00` | `80026FA4` | `801FFF30` | `801C7A08` |
| `2085` | return | `80026FBC` | `801C7A08` | `801FFF30` | `80000000` |
| `2087` | call | `801C7A08` | `801CB6BC` | `801FFF30` | `801C7A10` |

Static match:

- `docs/stage1_decomp/801C79E4_sub_801C79E4.c` shows `sub_801C79E4` calling `MEMORY[0x80026FA4]();` followed by `PrStageRunner_InitUI();`.
- Raw COMOD1 disassembly maps `PrStageRunner_InitUI` to `801CB6BC..801CBFDC`.

So frame `2212` now closes the dynamic upstream path to:

```text
801C82F0 -> 801C7A60
  -> 801C7A8C -> 801C79E4
    -> 801C7A00 -> 80026FA4
    -> 801C7A08 -> 801CB6BC / PrStageRunner_InitUI
      -> 801CBD60 / 801CBDA8 -> 8001B000 -> 80028504 -> 80028054
```

Submit window:

| seq | type | pc | target | sp | ra |
| ---: | --- | --- | --- | --- | --- |
| `18326` | call | `801CBD44` | `8001AFD8` | `801FFEF8` | `801CBD4C` |
| `18330` | return | `8001AFF8` | `801CBD4C` | `801FFEF8` | `80000000` |
| `18333` | call | `801CBD60` | `8001B000` | `801FFEF8` | `801CBD68` |
| `18340` | call | `8001B044` | `80028504` | `801FFED8` | `8001B04C` |
| `18347..18420` | call | `80028548` | `80028054` | `801FFEA8` | `80028550` |
| `18429` | return | `80028580` | `8001B04C` | `801FFED8` | `80000000` |
| `18433` | return | `8001B07C` | `801CBD68` | `801FFEF8` | `80000000` |
| `18435` | call | `801CBD8C` | `8001AFD8` | `801FFEF8` | `801CBD94` |
| `18439` | return | `8001AFF8` | `801CBD94` | `801FFEF8` | `80000000` |
| `18442` | call | `801CBDA8` | `8001B000` | `801FFEF8` | `801CBDB0` |
| `18448` | call | `8001B044` | `80028504` | `801FFED8` | `8001B04C` |
| `18453..18520` | call | `80028548` | `80028054` | `801FFEA8` | `80028550` |
| `18532` | return | `80028580` | `8001B04C` | `801FFED8` | `80000000` |
| `18536` | return | `8001B07C` | `801CBDB0` | `801FFEF8` | `80000000` |

Write-log side effects in this frame:

| seq | pc | address | old | new | meaning |
| ---: | --- | --- | --- | --- | --- |
| `18327` | `8001AFE0` | `801D301C` | `00000000` | `8009F13C` | first `8001AFD8` seeds cursor cell |
| `18331` | `801CBD4C` | `801D301C` | `8009F13C` | `8009F140` | first callsite delay/adjacent store advances cursor cell |
| `18430` | `8001B054` | `801D301C` | `8009F140` | `8009F1C4` | `8001B000` writes back advanced cursor |
| `18328` | `8001AFEC` | `801DB2D4` | `00000000` | `00000001` | first `8001AFD8` seeds count |
| `18431` | `8001B064` | `801DB2D4` | `00000001` | `00000000` | `8001B000` decrements count |
| `18399` | `800284D0` | `801D98F0` | `00000000` | `00000080` | downstream `80028054` writes desc/header field |
| `18436` | `8001AFE0` | `801D3024` | `00000000` | `800A66F4` | second `8001AFD8` seeds cursor cell |
| `18440` | `801CBD94` | `801D3024` | `800A66F4` | `800A66F8` | second callsite delay/adjacent store advances cursor cell |
| `18533` | `8001B054` | `801D3024` | `800A66F8` | `800A677C` | `8001B000` writes back advanced cursor |
| `18437` | `8001AFEC` | `801DB338` | `00000000` | `00000001` | second `8001AFD8` seeds count |
| `18534` | `8001B064` | `801DB338` | `00000001` | `00000000` | `8001B000` decrements count |
| `18499` | `800284D0` | `801DB2D8` | `00000000` | `00000080` | downstream `80028054` writes desc/header field |

Full-branch limitation:

- `full_branch_trace` has 18 entries for `80028054` and 18 entries for return target `80028550`.
- It has `0` entries for `800280B4` and `800280BC` because these are sequential instructions, not branch targets.
- Therefore full-branch confirms entry/return cadence but cannot recover the `lw $s0,0x44($v0)` address or value.

Boundary conclusion:

- New positive fact: frame `2212` now dynamically closes `801C79E4 -> 801CB6BC -> 801CBD60/801CBDA8 -> 8001B000 -> 80028504 -> 80028054`, and the write log confirms the two cursor/count writeback sequences.
- Still missing: instruction-time `$v0` at `800280B4` and `mem32[$v0+0x44]`.
- Because read log is absent and `800280B4` is not a branch target, this recording cannot close `coord44BackingSlotKnown80028054`.
- Next valid authority remains live GDB: gate with `801CBD60` / `801CBDA8`, then break `800280B4` and capture `$v0` plus `mem32[$v0+0x44]`.
