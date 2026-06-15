# Multimodule Follow-up Callchains - 2026-05-13

Sources:

- `docs/stage1_decomp/menuhelp_80026314_context_20260513/ida_export_menuhelp_801c4894_callchain_20260513.txt`
- `docs/stage1_decomp/sprite_submit_20260510/ida_export_gte_matrix_control_followup_20260513.txt`
- code-side lower-CD review in `src/pr/pr_stage1_lifecycle_executor_direct.cpp`,
  `src/pr/pr_stage1_lifecycle_host_adapter_801c81ec.cpp`,
  `src/pr/pr_stage1_lower_cd_producer_direct.cpp`, and
  `src/pr/pr_stage1_xa_cd_direct.cpp`

This pass continues direct-port evidence collection. It does not authorize
host-media success, replay samples, HUD/TEXT fallback, or candidate GTE
projection bits as truth.

## Lower-CD Apply Chain

The safe apply chain remains:

```text
801C81EC bootstrap waiting step
-> BuildBootstrap15590CdLowerProducerRequest801C81EC
-> RunBootstrap15590CdLowerFacts801C81EC
-> LowerCdRequestMetadataMatches801C81EC
-> LowerCdAttemptProvenanceMatches801C81EC
-> PrStage1XaCdDirectBuildLowerCdProducerSnapshot
-> BuildLowerCdSeamFromFacts
-> PrStage1XaCdDirectApplyLowerCdProducerSnapshot
-> RunBootstrap15590LoaderCdSeamLiveProducer801C81EC
```

Key code anchors:

| Item | Location |
| --- | --- |
| request-only host helper | `src/pr/pr_stage1_lifecycle_host_adapter_801c81ec.cpp:201` |
| helper still returns false after request-ready | `src/pr/pr_stage1_lifecycle_host_adapter_801c81ec.cpp:235` |
| request builder | `src/pr/pr_stage1_lifecycle_executor_direct.cpp:2167` |
| metadata guard | `src/pr/pr_stage1_lifecycle_executor_direct.cpp:1952`, `:2750` |
| attempt provenance guard | `src/pr/pr_stage1_lifecycle_executor_direct.cpp:2031`, `:2761` |
| apply gate | `src/pr/pr_stage1_lifecycle_executor_direct.cpp:2690` |
| ready wait builder | `src/pr/pr_stage1_lower_cd_producer_direct.cpp:930` |
| lower seam from facts | `src/pr/pr_stage1_lower_cd_producer_direct.cpp:1527` |
| XA bridge build/apply | `src/pr/pr_stage1_xa_cd_direct.cpp:751`, `:653` |

Remaining proof is not another request guard. It is a real lower-CD facts
producer for callback pending, raw `80036AF8` pump, `80037070` sync loop,
`800372F0` ready loop, and `80038FC0/80038DE8/800390C8` read start/sync facts.

## MenuHelp / StageClear

`801C4894` is now the highest-value static window after the negative
`80026314` xref/raw/jal sweeps.

| Evidence | Export line |
| --- | --- |
| `801C4894` function window | line 5 |
| caller `801C4DC4 -> 801C4894` | line 8 |
| `801C4C9C -> 80024FD0` | line 60 |
| `801C4D14 -> 80024FD0` | line 66 |
| disasm callsite `801C4C9C` | line 509 |
| disasm callsite `801C4D14` | line 539 |
| `80024FD0` reads `dword_80094438` | lines 657, 1513, 2364 |
| `80014614` reads `dword_80094430` | lines 1651, 2026, 2376 |
| watched xrefs to `80026314` | line 2397, still `(none)` |

Current conclusion: `801C4894` proves the path into `80024FD0`, but the
downstream slot is still `dword_80094438`, whose observed writer remains
`801C5B14`. This continues to describe callback/table mechanics, not a
`80026314` caller. `800167A8` stays at leaf-writer evidence until an execute
breakpoint or `80092F1D..80092F22` write watchpoint finds its live caller.

Next static step, if any, is only to interpret `801C4894` inputs and table flags
against `80024FD0:80025744..80025788`. Do not re-export `80026314` body or
repeat raw dword / direct `jal` sweeps.

## Rail / GTE

Superseded by `multimodule_followup_callchains_20260513_round2.md` and the
current P0 todo. The matrix/control-register ownership notes below are retained
as historical static evidence only; they are no longer the next proof target.
Current proof target is live trace `0x8003F4F4 -> 0x8003F710` with COP2/GTE
`SXY/FLAG/IR0/SZ/return` facts.

The GTE follow-up export moves the next proof target away from repeated
`8003F710` schedule and toward matrix/control register ownership.

| Evidence | Export line |
| --- | --- |
| `SetGeomOffset 800402C0` writes OFX/OFY control regs `$24/$25` | lines 129, 138-155 |
| `MulMatrix 80040884` uses `cop2 0x486012`, reads `$9/$10/$11`, stores `$11` | lines 376, 389-412 |
| `MulMatrix2 80040994` has the same matrix GTE op shape | lines 540, 548-571 |
| `GsSetProjection -> SetGeomScreen 80040C94` writes control reg `$26` | lines 699, 724-739 |
| `80040D6C` initializes `$29/$30/$26/$27/$28/$24/$25` | lines 743, 752-815 |
| `80041D3C` caller family reaches `MulMatrix` and `8003ABA4` | lines 1226 onward |

Current conclusion: this proves matrix/control register ownership and
same-library GTE matrix helper shape. It still does not prove exact RTPT/RTPS
division, screen saturation, `FLAG` bit mapping, prior SZ FIFO state, IR0, or
`$19 >> 2` return truth. Therefore `sxyWordsKnown`,
`flagAfterRtptKnown`, `flagAfterRtpsKnown`, `ir0Known`, and
`returnValueKnown` must remain false.
