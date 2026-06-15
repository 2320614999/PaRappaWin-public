# Scene Entry Card-Read Case17 Call Chain - 2026-05-13

Evidence source:

- Current IDA session decompile excerpts for `800179B4`, `800173A8`,
  `80019D7C`, `80019414`, and `80019284`.
- A headless `SCUS_941.83.i64` xref export was attempted to
  `ida_export_scene_entry_cardread_case17_callchain_20260513.txt`, but the MCP
  call timed out before the file was produced. The current-session decompile
  calls returned successfully and are the source for this table.

## PSX Call Chain

| Step | PSX evidence | Port-side carrier |
| --- | --- | --- |
| `80019D7C` case 17 | At `8001A0D8` it calls `800168DC`; if `word_8007ABE4` is nonzero it clears `gp+720/gp+724`, installs `VSyncCallback(80017F38)`, iterates 15 rows, clears `byte_8007ABE8[0x2000]`, copies the row name to `byte_8007CBE8`, calls `800179B4(byte_8007CBE8, byte_8007ABE8, 1)`, copies `dword_8007AE14` to row metadata, then calls `800164F8(byte_8007ADE8)`. It finally clears the callback, writes `gp+720 = 1`, and returns `23`. | `PrSceneEntryCardFeedbackDirect::BuildCase17Feedback80019D7C(...)` and `PrSceneEntryDirect::PsxCall80019D7C_Case17HiScoreBankCarrier(...)`. |
| `800179B4` | Calls `800173A8(gp+128, gp+124, a1, a2, a3)`, then `80016EB8()`, closes `gp+696`, and returns `0` only when the event result is `1`; otherwise it returns `-1`. | `PrStage1SaveCardHalDirect::BuildCardReadHalResult800179B4(...)`. |
| `800173A8` | Builds `bu%1d%1d:%s`, opens with mode `32769`, writes fd to `gp+696`, calls `80016FC0()`, submits `read(fd, a4, a5 << 13)`, and returns `0`. On open failure it closes `-1` and returns `-1`. | `CardReadAttemptFeedback800179B4` path/open/read/clear-events fields. |
| `80019414` | Calls `800191E4(a1, 3)`, returns `0` unless `gp+720 == 1`, and then returns `80019284(a1)`. | `PrSceneEntryDirect::PsxCall80019414_HiScoreEntry80015788(...)`. |
| `80019284` | Builds 18 high-score record glyph rows from the status prefix at `a1 + 4876`, writes through `dword_80049278`, and returns `dword_80049278`. | `PrSceneEntryFeedbackAdapterDirect::BuildFeedback80019414FromStatusPrefixAndCase17Bank(...)`. |

## Existing C++ Chain

| Function | Role | Boundary |
| --- | --- | --- |
| `BuildCase17CardReadHalFeedbackFromSaveCardHal800179B4(...)` | Converts already-authoritative `CardReadFeedback800179B4 + CardReadHalBuildResult800179B4` into case17 row facts. | Does not read Win card data and does not infer success from payload presence. |
| `BuildFeedback80019414FromCase17CardReadFacts(...)` | Runs `BuildCase17Feedback80019D7C(...)`, then `PsxCall80019D7C_Case17HiScoreBankCarrier(...)`, then the `80019414` feedback adapter. | Requires complete typed case17 card-read facts. |
| `BuildCompletedCall80019414InputFromFeedbackAdapterResult80019414(...)` | Converts a completed adapter result into executor-owned stable table storage input. | Rejects missing `gp720`, missing `80019284` result, wrong table address, or wrong table byte count. |
| `RunGenericSwitch80015788(..., completedCall19414)` and `ExecuteMainSceneSwitchTrace80015D18(..., completedCall19414)` | Existing four-argument execution path can consume completed feedback. | The default running path still passes `nullptr` until real `800179B4` typed facts exist. |

## Current Boundary

The missing piece is not a new request/provenance carrier and not another
`BuildFeedback80019414FromCase17CardReadFacts(...)` implementation. The missing
piece is a real producer for the PSX-shaped `800179B4/800173A8/80016FC0/read/
80016EB8/close` facts. Until that exists, `RunGenericSwitch80015788` must keep
using the default no-completed-feedback path and letting `Call80019414` report a
direct gap.

Forbidden shortcuts remain unchanged:

- Do not infer row success from `PrCard::Payload()`, Win save files, payload
  bytes, replay samples, or host I/O success.
- Do not fill `gp720`, `80019284` result, or event-6 table pointer before
  `80019D7C` and `80019284` have both produced typed facts.
- Do not connect the four-argument runtime path with partial `800179B4` facts.

## Minimal Live / Recorder Facts Needed

Current host facts are not enough: they only carry row enabled/name/card
selector/read-buffer bytes. A real producer needs at least:

- `80019414 -> 80019D7C -> 800179B4` provenance, non-early-return
  `arg2Known/arg2`, and `word_8007ABE4`.
- For all 15 attempts: row enabled/status, row name `byte_8007CBE8`, and
  `gp+128/gp+124` card selector.
- `800173A8` path/open facts: built `bu%1d%1d:%s` path, open flags `0x8001`,
  open attempted, fd, `gp+696` fd write, and open-fail `close(-1)`.
- Read submission facts: target buffer `0x8007ABE8`, block count `1`, byte
  count `0x2000`, read fd/buffer/count/return.
- `80016FC0` facts: call observed, four event handles, four `TestEvent`
  results.
- `80016EB8` facts: call observed, event handles, PSX return, timeout/hit event
  index, poll iteration count, and `80035560` wait count.
- Close facts: `close(gp+696)`, close fd known, and success-path close fd must
  match the `800173A8` fd / `gp+696` value.
- Payload authority: only if `80016EB8 == 1` and all required facts above are
  complete may `0x8007ABE8[0x2000]` become the row payload source.
