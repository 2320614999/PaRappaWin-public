# Stage1 v3 GOOD clear-tail stream2 terminal facts (2026-05-17)

## Source

- Recording:
  `E:\game\PSgame\parappa the rapper\duckstation\标题画面-menu-选关选stage1-good通关-保存记忆卡（存档重名保存失败重试成功）（过场动画均为跳过）v3.psxrec`
- Tool: `tools/模拟器内存回放器.py`
- Recording summary observed by the tool: 14386 frames, v3 compressed, write log present.

## Addresses

- `g_PrStageEventStreamDone`: `0x8008ED28` dword
- `g_PrStageEventStreamId`: `0x8008ED2C` word
- `g_PrStageEventStreamFlag`: `0x8008ED2E` word
- `dword_801D3048`: `0x801D3048` dword
- `ctx flags`: `0x801C3640` dword
- `ctx tick0C`: `0x801C364C` dword
- `rightRankActiveRow`: `0x801C368E` word
- stream2 descriptor cursor: `0x801D2D84` dword

## Recorder facts

`g_PrStageEventStreamId` switches into stream2 and then clears:

```text
changes 0x8008ED2C 2 10090 10720
Frame 10099: 0x0000 -> 0x0002
Frame 10625: 0x0002 -> 0x0000

writers 0x8008ED2C 2 10090 10630
Frame 10099 PC=0x801C96CC 0x0000 -> 0x0002
Frame 10625 PC=0x801CA058 0x0002 -> 0x0000
```

The base stream flag is cleared when stream2 is armed, then restored later by
the reset path:

```text
changes 0x8008ED2E 2 10090 10720
Frame 10099: 0x0001 -> 0x0000
Frame 10711: 0x0000 -> 0x0001

writers 0x8008ED2E 2 10095 10102
Frame 10099 PC=0x801C9694 0x0001 -> 0x0000

writers 0x8008ED2E 2 10708 10712
Frame 10711 PC=0x80024EE0 0x0000 -> 0x0001
```

The stream2 base frame is captured at the arm point:

```text
changes 0x801D3048 4 10090 10105
Frame 10099: 0x00000000 -> 0x00006007
```

The stream2 descriptor cursor consumes six events and resets:

```text
changes 0x801D2D84 4 10090 10630
Frame 10099: 0 -> 1
Frame 10197: 1 -> 2
Frame 10231: 2 -> 3
Frame 10361: 3 -> 4
Frame 10393: 4 -> 5
Frame 10623: 5 -> 6
Frame 10625: 6 -> 0

writers 0x801D2D84 4 10090 10630
Frame 10099 PC=0x801C9F44 0 -> 1
Frame 10197 PC=0x801C9F44 1 -> 2
Frame 10231 PC=0x801C9F44 2 -> 3
Frame 10361 PC=0x801C9F44 3 -> 4
Frame 10393 PC=0x801C9F44 4 -> 5
Frame 10623 PC=0x801C9F44 5 -> 6
Frame 10625 PC=0x801CA050 6 -> 0
```

The terminal completion signal is `ctx flags |= 0x100`, not
`g_PrStageEventStreamDone`:

```text
changes 0x8008ED28 4 10090 10720
0 found

writers 0x801C3640 4 10620 10630
Frame 10621 PC=0x801C7EDC 0x0000000E -> 0x00000000
Frame 10625 PC=0x801CA00C 0x00000000 -> 0x00000100
```

## Current code boundary

- `src/pr/pr_stage_event_direct.cpp` treats streams `2..5` as terminal streams
  in `PrStageEventDirectStage1IsTerminalStream(...)`.
- Active terminal stream completion emits `flag100BlocksWaitPulse` /
  `flag100SourceStream`, while the separate
  `gPrStageEventStreamDone/eventStreamDonePending` path is only used by the
  flag2000 stream family.
- The stream start path now immediately attempts the due id-stream consume in
  the same frame after `StartStage1EventStreamDispatch(...)`. This matches
  frame `10099`, where stream2 is selected and cursor `0 -> 1` in the same
  recorder frame.
- This matches the v3 GOOD path: stream2 completion should be observed as a
  stream2 terminal `flag100` pulse, not as `g_PrStageEventStreamDone`.

## COMOD1 terminal trigger event

Static parse of `E:\game\PSgame\parappa the rapper\S1\COMOD1.BIN` confirms
stream1 has exactly one `flags04 & 0x80` event:

```text
stream1 idx=280 addr=801D27E8 frame=0x6000 flags=00000080
```

The terminal stream selection in the recorder happens with
`dword_801D3048 = 0x6007`, not `0x6000`:

```text
Frame 10099: dword_801D3048 0x00000000 -> 0x00006007
```

IDA `801C9094` explains the gap: the `flags04 & 0x80` event sets
`unk_8008ED1C = 1` and `ctx+0x54 = 1`; the next `801C9094` call consumes that
latch, clears `g_PrStageEventStreamFlag`, selects stream2/3/4 from
`ctx+0x4E`, and resets `dword_801D3048` to the current `ctx+0x0C`.

The Win direct runtime was tightened to preserve this latch boundary:

```text
stream1 terminal event -> clearTerminalTailLatchPending801C9094
next 801C9094 Advance -> clear gPrStageEventStreamFlag, select stream2/3/4,
                         reset dword801D3048 from current scriptFrame
```

This keeps the clear-tail source as the COMOD1 `0x80` event while avoiding
same-frame synthetic stream start.

The trigger predicate was also tightened to the PSX cursor boundary: the
terminal pulse is only accepted when `g_PrStageEventStreamFlag == 1` and
stream1's current descriptor cursor points at the due `flags04 & 0x80` event.
It no longer scans the whole stream1 table for any already-past terminal event.

2026-05-17 follow-up: `PrStageEventDirectStage1Advance(...)` now keeps the
event body (`eventFrame / flags04 / byte29 / byte30`) when it advances a
flag/id cursor, then feeds that just-consumed event back into
`PrStageRunnerDirectFrameUpdate801C9094(...)`. This preserves the PSX
`801C9094` same-frame side effects such as `ctx+0x54=1`, `unk_8008ED1C=1`,
and id-stream apply-event actions even though the direct runtime cursor has
already advanced before the runner sub-translation is called.

2026-05-17 follow-up 2: lifecycle-facing `flag100BlocksWaitPulse` now also
consumes the runner-direct result itself. When
`PrStageRunnerDirectFrameUpdate801C9094(...)` returns `setCtxFlag0100` with
`resolvedEventStreamId` in terminal streams `2..5`,
`PrStageEventDirectStage1RunFrameUpdate801C9094(...)` writes that stream id to
`flag100SourceStream`. This pins the lifecycle input to the direct
`ctx flags |= 0x100` projection rather than relying only on the event-runtime
active-dispatch end approximation.

2026-05-17 follow-up 3: runtime-slot projection no longer calls
`RunStage1EventStreamFrameUpdate9094(...)` a second time for the same query.
`PrimeStage1RuntimeSlotPlayer(...)` now advances slots from the already-updated
`s_stage1EventStreamRuntime9094`, so the terminal stream state produced by the
main `801C7A60 -> 801C9094` pass is not re-applied before lifecycle consumes it.

## Conclusion

The v3 GOOD clear-tail authority is:

1. frame `10099`: `801C96CC` selects stream2, `801C9694` clears the base stream
   flag, and `dword_801D3048` becomes `0x6007`;
2. frames `10099..10623`: `801C9F44` advances stream2 cursor `0 -> 6`;
3. frame `10625`: `801CA00C` writes `ctx flags |= 0x100`,
   `801CA050` resets stream2 cursor, and `801CA058` clears stream id;
4. `g_PrStageEventStreamDone` remains unchanged in this window.

No scorer row, marker, renderer, lifecycle, Win duration, or full-song driver
artifact is allowed to replace this stream2 terminal authority.
