# Movie1 / clear-tail frame helpers: `8001F230`, `8001FEB4`, `8001C864`, `8001CE30`

Date: 2026-05-19

Scope: Stage1 Movie1 / clear-tail movie UI and transition draw path.

## IDA facts

- `8001F230`
  - Draws the outro/no-subtitle-box frame layout through repeated `sub_8001C550(...)` calls.
  - Uses templates in the `80050380..80050410` range.
  - Emits fixed corner/edge tiles and repeated strips; this path is not the normal subtitle-frame `8001B590` path.

- `8001FEB4`
  - Draws the no-video grid frame through nested 8 x 6 meta-tile loops.
  - Each meta-tile emits four `sub_8001C550(...)` calls using `800503E0`, `800503F0`, `80050400`, and `80050410`.

- `8001C864`
  - Draws subtitle-frame layout through `sub_8001B590(...)`.
  - Uses templates around `8004E860..8004E970`.
  - This path also participates in the `8001EC54 -> 8001DB00 -> 8001B730 -> 8001B954` subtitle/text chain.

- `8001CE30`
  - Draws no-subtitle frame layout through `sub_8001B590(...)`.
  - Uses templates around `8004E900..8004EB10`.

## Runtime implication

`8001F230` and `8001FEB4` use the raw `8001C550` submit path. Their raw action `ot` field is a PSX ordering-table bucket, not a modern renderer layer.

The Stage1 packet submit backend already maps the same `8001C550`/FastSprite OT buckets as:

- bucket `0` -> layer `784`
- bucket `1` -> layer `782`
- bucket `2` -> layer `781`
- bucket `3` -> layer `780`

Movie1 raw draw actions must use that same mapping before entering the renderer queue. Using literal layers `0/1/2/3` can place transition/frame tiles behind video or clear-tail frame content, matching the observed "transition has sound but no visible tiles" failure.

## Current code action

`src/pr/pr_stage1_scene1_movie1_direct.cpp` now maps `Movie1PsxRawDrawAction::ot` through the Stage1 packet backend OT bucket order when resolving raw `8001C550` actions into `Movie1TemplateDrawCommand::layer`.

This is a direct-port ordering fix, not a Win-side transition implementation.
