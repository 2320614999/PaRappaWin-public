# Multimodule Minimal Evidence Windows Summary - 2026-05-13

Source export:

- `docs/stage1_decomp/ida_export_multimodule_min_windows_clean_20260513.txt`

This pass deliberately keeps the windows narrow. It does not authorize Win-side
fallbacks, replay samples, renderer guesses, or candidate GTE projection bits as
truth sources.

## Lower-CD Apply Gate

Code-side evidence shows the bootstrap lower-CD apply gate is
`RunBootstrap15590CdLowerFacts801C81EC`, not
`TryPumpBootstrap15590CdLowerExternalPlan`. The host helper can reach
`RequestReady` and still return false; the real gate requires pending request,
non-lookup action, request metadata, attempt provenance, and ready state before
the XA bridge build/apply seam.

| Evidence | Location |
| --- | --- |
| request builder | `src/pr/pr_stage1_lifecycle_executor_direct.cpp:2167` |
| runtime gate | `src/pr/pr_stage1_lifecycle_executor_direct.cpp:2690` |
| metadata guard | `src/pr/pr_stage1_lifecycle_executor_direct.cpp:1952`, `:2750` |
| attempt provenance guard | `src/pr/pr_stage1_lifecycle_executor_direct.cpp:2031`, `:2761` |
| XA bridge build/apply | `src/pr/pr_stage1_lifecycle_executor_direct.cpp:2789`, `:2796` |
| host helper boundary | `src/pr/pr_stage1_lifecycle_host_adapter_801c81ec.cpp:201`, `:268` |

Current direct-port window: lower-CD static exports are stopped. Keep
`BuildBootstrap15590CdLowerProducerRequest801C81EC`,
`LowerCdRequestMetadataMatches801C81EC`,
`LowerCdAttemptProvenanceMatches801C81EC`, and
`BuildReadyWaitResult800372F0` as existing guards only; the next evidence must
be live/runtime facts, then a narrow IDA return to a new PC, `$ra-8`, or
unknown callback writer.

## MenuHelp / StageClear

The clean IDA export covers the minimum callable/data windows for the current
MenuHelp and StageClear bank gap.

| Target | Export evidence | Current conclusion |
| --- | --- | --- |
| `80026314` | function window at export line 1980; no xrefs to function start at line 1982 | text builder candidate only; caller/table source still open |
| `80026B94` | function window at line 2157; `80026D70 -> 800916F6` at line 2179; `80026D9C -> 80092F1D` at line 2183 | reader path confirmed; does not prove producer |
| `800167A8` | function window at line 707; `800167D4 -> 80092F1D` at line 713 | leaf writer to status byte bank; caller/source still open |
| `801C5B14` | `80048D28 -> 801C5B14` at line 3174; writes `80094430` and `80094440` at lines 3229/3240 | callback slot setup; not TEXT producer |
| `801C4FC8` | `801C5D10 -> 801C4FC8` at line 3086; writes `8008EEF8` / `800901BC` at lines 3090/3095 | buffer/count writer path, not enough for MenuHelp text authority |
| `word_800916F6` | data xref section line 3353; only `80026D70 -> 800916F6` at line 3354 | still read-only in this window |
| `80092F1D` | data xref section line 3477; refs from `800166AC`, `800167A8`, `80026B94` | status bank relation visible, but ownership not closed |

Next direct-port window: wait for live watch/break evidence around
`write 0x800916F6`, `exec 80026314`, `exec 80026B94`, `exec 800167A8`, or
`write 80092F1D..22`, then narrow-export the hit. Do not fill HUD/TEXT from
fallback presentation data.

## Rail / GTE

The Rail/GTE export confirms the library instruction schedule and callsites, but
not the bit-exact semantic facts needed to raise known flags.

| Target | Export evidence | Current conclusion |
| --- | --- | --- |
| `8003F710` `RotTransPers4` | function window at line 2973; caller `8003F4F4 -> 8003F710` at line 2976; `swc2 $12/$13/$14/$8` at lines 3019-3031 | RTPT/RTPS output schedule visible; not enough for SXY/FLAG/IR0 truth |
| `8003B9C8` `GsTMDfastF3NL` | function window at line 2634; caller `8001C1E8 -> 8003B9C8` at line 2637; output stores at lines 2780-2783 | same-library SXY/FLAG pipeline reference |
| `8003BD9C` `GsTMDfastNF4` | function window at line 2792; caller `8001C348 -> 8003BD9C` at line 2795; output stores at lines 2936-2948 | four-point RTPT+RTPS contrast reference |

Still blocked: `sxyWordsKnown`, `flagAfterRtptKnown`, `flagAfterRtpsKnown`,
`ir0Known`, and `returnValueKnown` must remain conservative until the PSX GTE
register semantics are proved from a direct source rather than candidate math or
Win renderer behavior.
