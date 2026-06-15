# Stage1 Try Again retry analysis - 2026-05-25

## Coverage matrix

| Seam | Status | Result |
| --- | --- | --- |
| `801C81EC` Try Again YES branch | match | PSX calls `8001EF14()` and loops back to `LABEL_5`; it does not run the abort/exit `80020110(...,2,1,2)` branch unless event4 returns `2`. |
| `8001EF14` body | match | Local IDA shows the narrow reset: `gp[51]=0`, `gp[49]=0`, `gp[50]=0`, then `8001EEAC(0)`. |
| Retry stage entry | mismatch in host split | PSX retry re-enters the blocking stage runner through `8001A4D0(sceneEntry+0x9C,0)` and `PrStageRunner_Run(...)`. Win's split runner can retain the previous formal lifecycle snapshot before the new `runner.Update(...)`. |
| Event4 active event-frame state | mismatch | `ConsumeDispatcherResult()` set dispatcher state to `Idle`, but left `s_dispEventId == 4`; `GetActiveEventFrameState8001E750()` could still return event4 packets from that stale id. |
| Retry intro transition | mismatch | The Movie1 path marks `s_stage1IntroTransitionFrame = -1` after the post-movie `800201AC(5,1,2)` transition; Try Again YES re-enters the stage loop without rearming that fresh runner-entry transition carrier. |
| Retry `8001A4D0` stream start | mismatch | `801C81EC` re-enters `8001A4D0(sceneEntry+0x9C,0)` for every non-abort retry. Win kept `s_xaStarted` / `stage1XaCdDirect.streamStarted` from the failed run, so the retry stage-record tick could continue the old XA/timecode stream instead of starting from the scene-entry record. This must be gated by the terminal nonzero result, not every Win split stage-record tick. |

## Patch rationale

- Clear the completed event id/frame state in `ConsumeDispatcherResult()`, and gate `PrEvent::GetActiveEventFrameState8001E750()` by dispatcher `Running` state so stale event4 packets are not rendered after the synchronous `80026B94(4,0)` result has been consumed.
- On a fresh Stage1 runner entry, clear the Win-only split formal lifecycle/drain carrier before checking terminal cleanup, then mark the intro transition preroll pending. This models PSX re-entering the blocking `LABEL_5` runner call instead of carrying a stale terminal snapshot from the previous failed call.
- Treat terminal `801C7A60` nonzero return as a one-shot retry restart fact. The next `StageRecordTick1A4D0` consumes that fact; if the previous Stage1 XA/CD stream carrier is still alive, stop host XA and reset `PrStage1XaCdDirectState` before the existing direct `8001A4D0` start path runs again. Normal per-frame split `StageRecordTick1A4D0` calls do not reset the stream.
- This is not a renderer/UI symptom patch: the changes are tied to the direct event dispatcher lifecycle and the Stage1 `801C81EC -> 8001A4D0 -> 801C7A60` retry entry seam.

## Expected manual checks

1. Fail Stage1, choose Try Again YES.
2. Confirm the restart shows the character-grid transition and frozen gameplay frame again.
3. Confirm Try Again does not keep reappearing after the new gameplay attempt begins.
4. Confirm gameplay/timecode starts from the beginning of the retry instead of jumping forward to the prior failure point.
5. Confirm the earlier Try Again black-flash fix does not regress.
