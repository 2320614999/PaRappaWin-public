# Scene Submit `coordNode+0x44` recorder window

Authority:

- Recording:
  `E:\game\PSgame\parappa the rapper\duckstation\bin\x64\标题画面-menu-选关选stage1-good通关-保存记忆卡（过场动画均为跳过）.psxrec`.
- Metadata: PSXREC03, `write_log_enabled=True`, `pc_trace_enabled=True`,
  `call_trace_enabled=True`, `branch_result_enabled=True`, `frames=14318`.
- Static setup reference:
  `docs/stage1_decomp/ida_export_scene_submit_coord44_owner_recheck_20260515.txt`.

Recorder commands:

```powershell
python tools/模拟器内存回放器.py <rec> export_windows 0 14318 artifacts\coord44_backing_recorder\coord44_slots_0_14318_step1.json --window cb64c@0x801CB64C:4 --window cb5e4@0x801CB5E4:4 --window cd7b0@0x801CD7B0:4 --window cc700@0x801CC700:4 --window cd800@0x801CD800:4 --step 1
python tools/模拟器内存回放器.py <rec> writers <slot> 4 790 810
python tools/模拟器内存回放器.py <rec> writers <slot> 4 13590 13605
python tools/模拟器内存回放器.py <rec> call_trace 803
python tools/模拟器内存回放器.py <rec> call_trace 804
```

Window values:

| slot | value timeline |
| --- | --- |
| `0x801CB64C` | frame `0..802`: `0x00000000`; frame `803..13598`: `0x00C23021`; frame `13599..14317`: `0x3C01801D` |
| `0x801CB5E4` | frame `0..802`: `0x00000000`; frame `803..13598`: `0x0C006C21`; frame `13599..14317`: `0x2455FEA8` |
| `0x801CD7B0` | frame `0..803`: `0x00000000`; frame `804..13600`: `0x00000015`; frame `13601..14317`: `0x000000DB` |
| `0x801CC700` | frame `0..802`: `0x00000000`; frame `803..13599`: `0x00002021`; frame `13600..14317`: `0x000000F1` |
| `0x801CD800` | frame `0..803`: `0x00000000`; frame `804..13600`: `0x00010000`; frame `13601..14317`: `0x00000030` |

Writer / call-trace facts:

- `writers <slot> 4 790 810` returned `0 found` for all five slots.
- `writers <slot> 4 13590 13605` returned `0 found` for all five slots.
- `call_trace` for frames `803`, `804`, `805`, `13599`, `13600`, and
  `13601` contains no target calls to:
  `801C609C`, `8004049C`, `8001AF1C`, `8001B000`, `80028504`, or `80028054`.
- `wlog 803/804` contains thousands of writes, but not writes to these five
  slots; the frame-level value changes therefore are not CPU-store writer facts
  for `coordNode+0x44`.

Boundary conclusion:

- This recorder window disproves treating the five candidate absolute slots as
  simple always-zero backing cells in the covered recording.
- The observed values are frame/window memory contents around overlay load and
  later scene transition, not proven `80028054` type1 TRS backing state and not
  proven writes from `801C609C/8004049C/8001AF1C/8001B000/80028504/80028054`.
- Therefore this evidence does not authorize `coord44BackingSlotKnown80028054`
  for runtime cutover.

Next authority requirement:

- The next useful evidence must hit `800280B4` live or recorder-side with
  register context: `v0` as the COORD node pointer, `*(v0+0x44)` as the actual
  backing pointer/value, and the caller chain into `80028054`.
- If using recorder first, find frames with `80028054` call-trace coverage and
  then read `v0+0x44` from the same frame/window. If recorder cannot provide
  register context for the hit, use live GDB `exec 0x800280B4`.
