You are an external reverse-engineering/code-review collaborator. Work only
from the material in this prompt and the pushed branch I specify.

Environment limits:
- You cannot access my local files, IDA session, emulator, debugger, terminal,
  screenshots outside the branch, or build output unless pasted here.
- You cannot modify files, run tests, build, clone private repos unless the
  branch is accessible to you, or inspect live PSX memory.
- Treat the Win runtime captures as reproduction evidence, not PSX authority.
- Treat the pasted pseudo-C and PSX recordings as the strongest available
  authority, but note that current `.psxrec` files do not cover the same
  `UIList_Block=1` path.

Goal:
Review the Stage1 GOOD->COOL raised-camera black-block issue and decide whether
there is a safe direct-port patch now.

Branch:
`codex/stage1-blackblock-pro-handoff-20260529`

Primary handoff file:
`docs/stage1_decomp/good_to_cool_black_block_20260529/README.md`

Constraints:
- Do not propose hiding KAGE, adding KABE/TENNJOU to the UIList branch, or
  changing renderer sort/layer unless backed by PSX pseudo-C/live facts.
- Do not treat the forced GOOD->COOL Win run as authority for PSX behavior.
- If a patch is justified, output a `git apply --index` compatible unified diff
  and cite the exact evidence line/section that justifies it.
- If no safe patch is justified, output `no safe patch`, then list the minimum
  next facts to capture from PSX or IDA.

Observed local facts:
- Full build after reverting a failed GTE/SXY saturation attempt succeeded.
- Current source has no runtime-code diff after the revert.
- Black block remains in post-revert screenshots.
- Read-only external sampler sees the black-block path submitting:
  `ON, PA, FLOOR, TAMA, TOUROU, AUX_7FF8, KAGE`.
- That submit membership matches the current `801CB190` pseudo-C for
  `g_PrStageRunner_UIList_Block`.
- Existing PSX recordings under `..\duckstation` have `UIList_Block nonzero=0`,
  so they do not cover the same path.

Question:
Given the evidence, is the next safe action a code patch, an IDA export, or a
live PSX/GDB capture? Be concrete. If you recommend a capture, specify exact
addresses/functions and what values/packets should be recorded.
