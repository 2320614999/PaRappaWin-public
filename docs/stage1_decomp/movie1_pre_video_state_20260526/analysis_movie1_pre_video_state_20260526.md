# Movie1 pre-video state audit

Date: 2026-05-26

Scope: Stage1 entry state before `MOVIE1.STR` playback starts. This audit stays
inside the direct lifecycle / IDA evidence path and does not use renderer, UI, or
shell symptom patches.

## Web Pro result

The web Pro pass returned no safe patch from the previously pushed evidence. It
specifically rejected hiding the pre-transition or ignoring early skip input
without closing the PSX `800201AC` / STR player boundary, and requested fresh
local IDA evidence for the Stage1 movie entry and lower event helpers.

## New local IDA evidence

Exports in this directory:

- `ida_export_scus_movie1_pre_video_state_20260526.json`
- `ida_export_comod1_movie1_pre_video_state_20260526.json`
- `comod1_batch/decompile_801c81ec.txt`
- `comod1_batch/decompile_801c77c0.txt`
- `comod1_batch/decompile_801c750c.txt`

Current COMOD1 `PrScene1_RunMovie1_AndStage1(801C81EC)` shows the initial
Movie1 path as:

```c
MEMORY[0x800201AC](-2145634752, 6, 2, 1);
PrStrPlayer_Init(MEMORY[0x8006EDB8] + 108, 0);
if (PrStrPlayer_PlayAndWait(MEMORY[0x8006EDB8] + 108, -2145634752, 0) != 1) {
  MEMORY[0x80026FA4]();
  MEMORY[0x800201AC](-2145634752, 5, 1, 2);
  goto LABEL_5;
}
```

## Concrete mismatch

Win `EmitInitialMovie1Request` was still emitting
`800201AC(&801C3640, 6, 1, 2)`, while
`IsMovie1TransitionAction801C81EC` already validates the initial pre-movie action
as `mode=6, pre=2, post=1`.

That means the producer and validator disagreed on the exact direct lifecycle
action. This is a wiring mismatch, not a visual workaround.

## Rejected partial patch attempt

`EmitInitialMovie1Request` was temporarily changed to emit
`800201AC(&801C3640, 6, 2, 1)` for the initial Movie1 pre-transition. Full build
passed, but user visual testing confirmed that entering Stage1 immediately
became a persistent black screen.

Follow-up inspection showed why: only the producer had been changed. The
InitialMovie1 receivers still accepted only `6,1,2`:

- `IsInitialMovie1Transition201ACAction801C81EC`
- `TickStage1InitialMovie1Transition201AC801C81EC`
- `IsInitialMovie1PreTransition201ACPending801C81EC`

The next patch aligns producer, validator, tick, and pending predicate together
to `800201AC(&801C3640, 6, 2, 1)`. Clear-tail movie handling remains on its own
`ClearTailMovie` branch.

No renderer/UI/shell changes were made.
