# Scene Submit `800280B4` live GDB gap

Scope:

- Stage1 Scene Submit / TOD `8001B000 -> 80028504 -> 80028054`.
- Narrow question: real `coordNode+0x44` backing source for type1 commands.
- This note does not touch RGB, Rail/GTE, lower-CD, case17, MenuHelp/StageClear,
  boot seam, renderer, or presentation fallback.

Static anchor:

- `800280AC`: `lw $v0, 4($a1)` loads the COORD node pointer from
  `desc+4`.
- `800280B4`: `lw $s0, 0x44($v0)` reads the real `coordNode+0x44`
  backing value.
- `800280B8`: `addiu $s1, $v0, 4` targets the COORD matrix area.
- `800280BC`: `sw $zero, 0($v0)` clears the COORD header, not `+0x44`.

Recorder follow-up:

- Recording:
  `E:\game\PSgame\parappa the rapper\duckstation\bin\x64\标题画面-menu-选关选stage1-good通关-保存记忆卡（过场动画均为跳过）.psxrec`.
- Previous candidate-slot window:
  `docs/stage1_decomp/scene_submit_coord44_backing_recorder_window_20260515.md`.
- A narrow call-trace scan over frames `0..1599` found outer Scene0 tick
  coverage, but no command execution into the type1 interpreter:

| target | hits in frames `0..1599` |
| --- | ---: |
| `801C5EF0` | 90 |
| `801C5E60` | 90 |
| `8001B000` | 90 |
| `80028504` | 0 |
| `80028054` | 0 |
| `801C609C` | 0 |

- Representative samples are even frames starting at frame `2`, e.g.
  `801C5EF0 -> 8001B000` with `sp=0x801FFEA0/0x801FFE80` depending on
  call nesting, but no matching `80028504` or `80028054` call in the same
  scanned range.
- Frame `803` and `13599` call traces are dominated by lower-CD / scene
  transition work and still do not provide `800280B4` register context.

Follow-up full target scan:

- A faster full-recording call-trace target scan was added after the narrow
  `0..1599` check:
  `artifacts/coord44_backing_recorder/coord44_calltrace_target_scan_0_14318_20260515.json`.
- Summary:

| target | hits | first observed frame |
| --- | ---: | ---: |
| `801C609C` | 0 | none |
| `801C5EF0` | 90 | 2 |
| `801C5E60` | 90 | 2 |
| `8001B000` | 9078 | 2 |
| `80028504` | 138 | 2212 |
| `80028054` | 202 | 2212 |

- Detailed frame `2212` call-chain sample:
  `docs/stage1_decomp/scene_submit_80028054_calltrace_hits_20260515.md`.
- This corrects the narrow-window statement: the current PSXREC03 does cover
  `80028504 -> 80028054`, first at frame `2212`.
- It still does not provide instruction-time `$v0` at `800280B4` or
  `mem32[$v0+0x44]`; call-trace only carries `pc/target/sp/ra/event_seq`.

Live GDB attempt:

- DuckStation GDB MCP connection to `127.0.0.1:2345` was attempted.
- Result: connection refused (`WinError 10061`), so no live register snapshot
  was available in this run.
- A second connection attempt after the full target scan also returned
  `WinError 10061`.

Boundary conclusion:

- The current recording now proves that both the outer tick and the
  `80028504 -> 80028054` command interpreter are covered. The first useful
  reproduction frame is `2212`.
- The recording still does not contain the `800280B4` instruction register
  state needed to read `coordNode+0x44`.
- The five candidate absolute slots remain non-authoritative. Their values
  cannot be promoted to `coord44BackingSlotKnown80028054`.
- The next authoritative evidence must be a live `800280B4` hit, or a new
  recording that contains equivalent register context for the same instruction.

Minimal live GDB capture:

1. Start DuckStation with GDB server enabled on `127.0.0.1:2345`.
2. Reproduce to frame `2212` or the same menu/TOD path.
3. Break on `0x800280B4`.
4. Capture:
   - `$pc,$ra,$sp,$a0,$a1,$a2,$v0,$s2,$s3,$s5,$s6`
   - `mem32[$a1]`, `mem32[$a1+4]`
   - `mem32[$v0]`, `mem32[$v0+0x44]`
   - `mem32[$s5]`, and the command payload range
     `mem32[$s2 .. $s5 + 4*$s6]`
5. If `mem32[$v0+0x44]` is a valid pointer, add a write watchpoint on that
   backing address to find its real writer/lifetime.

Authorization rule:

- Only the `800280B4` hit with `v0` and `mem32[v0+0x44]` authorizes
  `coord44BackingSlotKnown80028054`.
- Do not infer this field from Win-side COORD state, default identity TRS,
  renderer output, TMD data, ISO/file readability, or sampled candidate-slot
  values.
