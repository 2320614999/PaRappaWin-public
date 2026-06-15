# Stage1 Movie1 entry / skip audit handoff

Date: 2026-05-26

Branch: `codex/stage1-gpt55-pro-handoff-20260524`

Current pushed HEAD visible to ChatGPT web: `19a71e61`

Local candidate patch status: not committed, full `.\build.ps1` passed.

## Scope

Only audit these two Stage1 Movie1 issues:

1. Extra visible state between entering Stage1 and the story movie beginning. If the user presses skip during this state, the game can black-screen/hang.
2. During story movie playback, pressing skip freezes the video but then shows an instant black frame before disappearing. Original PSX behavior is freeze then disappear directly.

Do not touch Try Again, fail branch, rail, audio restart, face-expression, renderer/UI/shell fallback, sentinel/matcher/observer/harness code.

## Important correction to previous Pro audit

Previous Pro response cited an older `801C81EC` export and treated the initial Movie1 pre call as:

```text
800201AC(..., 6, 2, 1)
```

The fresh local IDA export in this folder proves the current authoritative COMOD1 entry function `801C4DC4` does:

```text
sub_800201AC(&unk_801C3640, 6, 1, 2);
sub_801C44E0(108, 0);
sub_801C455C(108, &unk_801C3640, 0);
sub_8001B120(1);
word_800916D2 = 1;
sub_800201AC(&unk_801C3640, 5, 1, 2);
```

The full export is:

```text
docs/stage1_decomp/movie1_entry_skip_audit_20260526/ida_export_movie1_entry_skip_audit_20260526.md
```

It includes pseudo-C, xrefs, calls, and full disasm for:

```text
801C4DC4
801C44E0
801C455C
800201AC
8001EA74
80020248
80020308
8001F230
8001FEB4
8001C864
8001CE30
80035510
80040CA4
80040370
```

## Local candidate patch already applied

The local candidate patch only aligns the Initial Movie1 direct lifecycle pre transition to `800201AC(6,1,2)`.

Files changed:

```text
src/pr/pr_stage1_lifecycle_direct.cpp
src/pr/pr_scenes.cpp
src/pr/pr_stage1_lifecycle_executor_direct.cpp
src/pr/pr_stage1_lifecycle_host_adapter_801c81ec.cpp
docs/memory.md
docs/待办事项.md
docs/stage1_decomp/801c81ec_lifecycle_callsite_table_20260514.md
```

Behavioral intent:

```text
Initial Movie1 pre:  800201AC(..., 6, 1, 2)
Initial Movie1 post: 800201AC(..., 5, 1, 2)
Clear-tail movie path remains unchanged.
```

Local full build result:

```text
.\build.ps1
[BUILD] === SUCCESS ===
Output: E:\game\PSgame\parappa the rapper\PaRappaWin.exe
```

This candidate needs human visual test for issue 1.

## Remaining question for Pro

Audit the fresh IDA export plus pushed branch code and decide whether issue 2 has a safe direct-port patch:

```text
Movie1 skip: video freezes, then Win shows an instant black frame before disappearing.
Original: video freezes, then disappears directly.
```

Known Win suspect, not authority:

```text
src/pr/pr_stage1_scene1_movie1_direct.cpp

MarkMovieStrBlockCompletedBeforePostTransition(...)
BuildRuntimeDrawPlan(...)
QueryDrawableState(...)
StrPlayerResult::Skipped path
outroSourceFrame30 / playAndWaitCompletionPending
```

Please compare those Win paths against `801C455C`, `80020308`, and the called helpers in the attached export. If there is a proven mismatch, output one `git apply --index` compatible unified diff. If not, output `no safe patch` and list the missing exact evidence.
