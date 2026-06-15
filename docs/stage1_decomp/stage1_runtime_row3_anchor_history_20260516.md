# Stage1 runtime row3 anchor/history contrast - 2026-05-16

Scope: narrow evidence for the current Stage1 row/writeback blocker. This
does not authorize renderer, marker, lifecycle clear, full-song harness input
semantic changes, synthetic row writeback, `v22` forcing, row table edits, or
`24F8C` patches.

Runtime artifact:

`artifacts/stage1_rail_probe_runtime/20260516_224538_stage1_rail_full_song_runtime/samples.jsonl`

Input-mode caveat:

`docs/stage1_decomp/stage1_full_song_input_mode_circle_bias_20260516.md`
classifies this artifact as Circle-biased by the existing full-song runtime
driver (`acceptedTickSeedControl18` is `32` for every nonzero accepted seed).
Therefore its row3 history is useful as a Win contrast, but not as PSX scorer
authority for first-beat `0x10/1`.

PSX recorder authority:

- `docs/stage1_decomp/stage1_row3_firstbeat_page_history_20260516.md`
- `docs/stage1_decomp/stage1_row3_phase1_first_second_beat_20260516.md`
- `docs/stage1_decomp/stage1_row3_accepted_input_window_20260516.md`
- `docs/stage1_decomp/stage1_writer_page_pointer_14bdc_14614_20260516.md`

## Runtime facts

Latest runtime has the direct bucket30 owner in the expected reader-page shape:
`currentPageOrdinal1Based=N` and `bucket30GameplayReaderPageOrdinal=N-1`.
The remaining mismatch is the page history contents and anchor class.

Key unique samples from the latest runtime artifact:

| query | tick96 | page56 | reader page | `ED08` | `ED36/ED38` before | req mask | union | count | anchor slot | required class | anchor class | occupied | anchor match | commit |
|---:|---:|---:|---:|---:|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| `704` | `4220` | `11` | `10` | `0x0E` | `2/0` | `0x80` | `0x20` | `0` | `8` | `4` | `2` | `1` | `0` | `-2` |
| `767` | `4593` | `12` | `11` | `0x0A` | `0/1` | `0x10` | `0x20` | `0` | `8` | `1` | `0` | `0` | `0` | `-1` |
| `830` | `4965` | `13` | `12` | `0x0E` | `2/0` | `0x20` | `0x20` | `1` | `8` | `2` | `0` | `0` | `0` | `0` |
| `893` | `5328` | `14` | `13` | `0x0A` | `0/1` | `0x40` | `0x20` | `0` | `8` | `3` | `0` | `0` | `0` | `-2` |

This confirms the contrast after the `acceptedGateOpen` fix: the reader-page
cursor is not the current suspect, but this specific artifact is driven with a
Circle-biased input mode. The decisive runtime rows feed bucket30 with `0x20`
history or an empty anchor slot where PSX recorder truth needs `0x10/1`.

## PSX contrast

PSX authority for the first row3 phase1 beat:

- frame `3858`: page11 slots `8/10/12` are `0x10/1/1`;
- frame `3861`: fresh non-replay input writes page11 slot14 as `0x10/1/1`;
- frame `3865`: descriptor `0x801CD4AC`, `ED08=0x0A`, required mask `0x10`,
  `91816/91818=9/9`, `ED36/ED38=1/1`.

PSX authority for the matching second beat:

- frame `3961`: `14BDC -> 14614` writes page12 slot10 as `0x20/2/1`;
- frame `3995`: descriptor `0x801CD4C4`, `ED08=0x0E`, required mask `0x20`,
  `91816/91818=18/18`, then `24F8C` resets `ED36=2 / ED38=0` and row writes
  `3 -> 2`.

## Conclusion

Do not patch `80024FD0`, `80014D58`, `80014D28`, `80024F8C`, coverage,
slot14, row tables, marker, renderer, lifecycle clear, or full-song harness
input semantics from this runtime shape. The next row3 validation must either
use an existing input path that can supply the PSX first-beat `0x10/1` then
second-beat `0x20/2` sequence, or stay on recorder/live facts.
