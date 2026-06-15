# Stage1 Field Matrix

This matrix stays intentionally small. Each item only records currently confirmed writers, readers, and the safest semantic level for the current Stage1 hot path.

## `word_80091810`
- Writers: `sub_80014614` increments; `sub_80014C5C` increments; `sub_80014C1C` clears; `sub_80014400` clears.
- Readers: `sub_80014458`, `sub_80014D58`.
- Current semantic level: current-bucket accepted contribution count, including the additive-injector path from `sub_80014C5C`.

## `word_80091812`
- Writers: `sub_80014614` increments on the record-write path; `sub_80014C1C` clears; `sub_80014400` clears.
- Readers: none confirmed inside the requested hot set after writeback; current scorer path does not read it.
- Current semantic level: record-write shadow count for the current bucket.

## `word_80091814`
- Writers: `sub_80014614` increments on the accepted-but-unrecorded split; `sub_80014C1C` clears; `sub_80014400` clears.
- Readers: `sub_80014D58`.
- Current semantic level: current-bucket split / penalty-side count consumed by both formula branches.

## `word_80091816`
- Writers: `sub_80014D58` commits and clamps; `sub_80014344` clears; `sub_800169E0` special-cases direct sync in the manual-select path.
- Readers: `sub_80024FC0`, `sub_80014C80`, `sub_80014D28`, `sub_8001448C`, `sub_80014548`.
- Current semantic level: cumulative Stage1 accumulator that later gets mirrored to the HUD.

## `word_80091818`
- Writers: `sub_8001448C` snapshots from `word_80091816`; `sub_80014C80` and `sub_80014D28` update it after reading the previous baseline.
- Readers: `sub_80014C80`, `sub_80014D28`.
- Current semantic level: shared growth baseline for COOL and non-COOL classifiers; do not split it into per-caller baselines.

## `word_8009181A`
- Writers: `sub_8001448C` snapshots from `word_80091816`; `sub_80014548` snapshots from `word_80091816` before the compare update.
- Readers: no hot-path reader confirmed in this requested set.
- Current semantic level: PSX accumulator snapshot companion; keep it as a real shared scorer global even when Win currently observes it only through diagnostics.

## `word_8009181C`
- Writers: `sub_8001448C` snapshots from `word_80091816`; `sub_80014548` writes the current accumulator after using the previous value as compare baseline.
- Readers: `sub_80014548`.
- Current semantic level: tie-break compare baseline for the final right-rank resolution helper.

## `word_8009181E`
- Writers: `sub_80014400` clears; `sub_80014548` writes `1` on the negative row3 delta path; `sub_800144B8` clears it after the bucket0 helper gate.
- Readers: `sub_80014548`, `sub_800144B8`.
- Current semantic level: tie-carry latch used by right-rank tie-break logic; not an accepted-count field.

## `word_80091820`
- Writers: `sub_80014400` clears; `sub_80014458` increments when `word_80091810 == 0`; `sub_800144B8` clears it after the bucket0 helper gate.
- Readers: `sub_80014458`, `sub_800144B8`.
- Current semantic level: no-input counter companion for the scorer/right-rank helper package.

## `word_80091822`
- Writers: `sub_80014C5C` writes; `sub_80014C1C` clears.
- Readers: `sub_80014D58`.
- Current semantic level: additive bonus term injected into the current scorer commit.

## `word_80091826`
- Writers: `sub_80014400` clears; `sub_80014C80` writes the COOL-gain latch.
- Readers: `sub_80014538`.
- Current semantic level: COOL-gain latch read by the delayed follow-up arm gate.

## `word_80091828`
- Writers: `sub_80014400` clears; `sub_80014C80` updates the COOL-gain streak.
- Readers: `sub_80014C80`.
- Current semantic level: COOL classifier streak counter paired with `word_80091826`.

## `word_8009182A`
- Writers: `sub_800143F0` writes the setup/restart gate argument.
- Readers: `sub_80014A80`; `sub_80024FD0` reads it in the GOOD->COOL commit arm.
- Current semantic level: setup/restart GOOD->COOL / pair-bonus enable latch; it shares its `801C7A60` source argument with the `word_8008ED34` setter, but is not itself the short-window latch.

## `dword_80092F40`
- Writers: outside this requested hot set; populated from the saved/replay payload setup path.
- Readers: `sub_80016758`.
- Current semantic level: previous-grade payload field used only as a boolean `>= 2` setup/restart gate input for `801C7A60`.

## `ctx+0x56`
- Writers: not pinned in this requested hot set; current confirmed role is the cadence-owned 1-based page ordinal fed into bucket31 page maintenance.
- Readers: `sub_80024FD0` passes it to `sub_80014BDC`; `sub_80014D58` and `sub_80014A80` use it as the absolute lookback page ordinal for ring-page reads.
- Current semantic level: reader/root page-ordinal owner; distinct from the mutable writer-page helper seeded by `sub_80014BDC`.

## `word_8008ED00` (`IDA: dword_8008ED00`)
- Writers: `sub_80024E98` initializes to `0`; `sub_80024FD0` performs the runtime state transitions (`0/1/4/5/6/7/8` cases).
- Readers: `sub_80024BF4`, `sub_80024FD0`.
- Current semantic level: bucket30/bucket31 follow-up state for the right-rank special tail.

## `word_8008ED34`
- Writers: `sub_80024E98` clears via the shared state-block reset; `sub_800259C0` writes the live value supplied by the `801C7A60` setup/restart gate query.
- Readers: `sub_80024FD0`.
- Current semantic level: right-rank short-window helper latch; current confirmed use is to gate a GOOD+positive first-beat blink emit.

## `word_8008ED36`
- Writers: `sub_80024E98` clears via the shared state-block reset; `sub_80024F8C` writes `2`; `sub_80024FD0` writes/update-caches live values including the forced-GOOD path.
- Readers: `sub_80024FD0`.
- Current semantic level: right-rank two-phase resolution cache / short-window bucket result.

## `word_8008ED38`
- Writers: `sub_80024E98` clears via the shared state-block reset; `sub_80024F8C` clears; `sub_80024FD0` increments and clears it.
- Readers: `sub_80024FD0`.
- Current semantic level: right-rank short-window phase counter.

## `dword_8008ED08`
- Writers: `sub_80024FD0` refreshes it from the current descriptor row at bucket0.
- Readers: `sub_80024BF4`, `sub_80024FD0`.
- Current semantic level: current descriptor flag word that gates formula, right-rank, and special-tail branches.

## `ctx+0x30`
- Writers: `sub_80024E98` clears via whole-context reset; `sub_80024FC0` mirrors `word_80091816`; `sub_800169E0` special-cases a direct write in manual-select state.
- Readers: `sub_8001DF24`.
- Current semantic level: cumulative display-value mirror for the left-lower decimal HUD slot.

## `ctx+0x3C`
- Writers: `sub_80024E98` clears via whole-context reset; `COMOD1/PrStageRunner_Run (0x801C7A60)` writes constant `1` on the `sub_80014C5C(250)` special branch.
- Readers: none confirmed in this window.
- Current semantic level: special fixed-bonus companion latch / branch marker.

## `ctx+0x18`
- Writers: `COMOD1/PrStageRunner_Run (0x801C7A60)` writes the current steady-loop control sample before `sub_80024FD0`; `sub_80024E98` clears it via whole-context reset.
- Readers: `sub_80024FD0`, `sub_80014614`, and the same-loop `sub_80024B54(ctx+0x18)` class-token call.
- Current semantic level: literal current-frame writer control mask; low `1/2` are remapped to `4/8` in the `801C7A60` writer before downstream scorer consumption.

## `ctx+0x20`
- Writers: `COMOD1/PrStageRunner_Run (0x801C7A60)` writes `sub_80024B54(ctx+0x18)` on nonzero live/replay control dispatches and writes literal `0` on the live no-control clear path; `sub_80024E98` clears it via whole-context reset.
- Readers: `sub_80014614` and adjacent accepted-producer/descriptor carrier logic.
- Current semantic level: priority class token generated by the exact `sub_80024B54` classifier, not a one-hot validator.

## `ctx+0x76`
- Writers: `sub_801C9094` writes the stream/tail completion latch on the `unk_8008ED1C == 4` path; `sub_801C7A60` writes it on the low-level abort/tail path.
- Readers: `sub_801C7A60` reads it in the final runner-tail return gate.
- Current semantic level: COMOD1 runner exit latch; keep it separate from scorer numeric row state.

## `ctx+0x78`
- Writers: `sub_801C9094` writes the event/tail dispatch result family after consuming `unk_8008ED1C`.
- Readers: `sub_801C7A60` reads it in the final runner-tail gate and passes it to the tail helper path.
- Current semantic level: COMOD1 runner tail gate/result latch; not an accepted-count or right-rank row field.

## `ctx+0x4E`
- Writers: `sub_80024E98` initializes it; `sub_80024FD0` performs the runtime row transitions including the forced write-back to GOOD.
- Readers: `sub_80024FD0`, `sub_8001DF24`, `PrStageRunner_ApplyScriptEvent`.
- Current semantic level: `rightRankActiveRow`, the authoritative 4-row ladder selector.

## `ctx+0x50`
- Writers: `sub_80024E98` initializes it to `0`; `sub_80024FD0` toggles it between the base path and special substate path.
- Readers: `sub_80024FD0`, `sub_80014D58`, `sub_80014614`.
- Current semantic level: special substate index for the descriptor/scorer tail; not just a plain bool.

## `ctx+0x58`
- Writers: `sub_80024E98` clears via whole-context reset; `sub_80024FD0` writes the live row id.
- Readers: `sub_8001DF24`.
- Current semantic level: `rightRankBlinkTargetRow`.

## `ctx+0x5A`
- Writers: `sub_80024E98` clears via whole-context reset; `sub_80024F8C` clears; `sub_80024FD0` sets and clears it during bucket30 resolution.
- Readers: `sub_8001DF24`.
- Current semantic level: `rightRankBlinkEnabled`.

## `ctx+0x5C`
- Writers: `sub_80024E98` clears via whole-context reset; `sub_80024FD0` writes the change-triggered visible bit.
- Readers: `sub_8001E2E4`.
- Current semantic level: `topLessonPairChangeVisible`.

## `ctx+0x5E`
- Writers: `sub_80024E98` clears via whole-context reset; `sub_80024FD0` writes the current lesson variant id.
- Readers: `sub_8001DB9C` via `sub_8001E2E4`.
- Current semantic level: `topLessonPairLessonId`.

## `ctx+0x6A`
- Writers: `sub_80024E98` initializes it to `1` after the whole-context reset; no COMOD1 writer is confirmed in the current scorer host hot set.
- Readers: `sub_800144B8` gates the row3 no-input return path and the row0/row3 tie-carry return path through `ctx+0x6A != 0`.
- Current semantic level: bucket0 `sub_800144B8` consumer permit carried as Stage1 bucket context; do not treat it as a top-level bucket0 result or a COMOD1 overlay output.

## `ctx+0x18E`
- Writers: `sub_80024FD0` writes the transition animation/resource selector on right-rank row-write paths.
- Readers: `sub_801C9094` reads it only inside the `ctx&0x200` frame-update branch to select the DAT/VDF resource pair.
- Current semantic level: transition animation selector that also feeds the COMOD1 resource-pair request; it is not a score formula input.

## `ctx+0xE0`
- Writers: `sub_801C9094` writes the DAT resource handle/index in the `ctx&0x200` branch for `ctx+0x18E` values `1..4`.
- Readers: outside the current scorer hot set after COMOD1 frame update.
- Current semantic level: COMOD1 DAT resource slot paired with `ctx+0xEC`; model as resource request state, not scorer numeric state.

## `ctx+0xEC`
- Writers: `sub_801C9094` writes the VDF resource handle/index in the `ctx&0x200` branch for `ctx+0x18E` values `1..4`.
- Readers: outside the current scorer hot set after COMOD1 frame update.
- Current semantic level: COMOD1 VDF resource slot paired with `ctx+0xE0`; model as resource request state, not scorer numeric state.

## `dword_801CCBB8`
- Writers: `sub_801C7A60` mirrors the accepted/control mask during the Stage1 host loop.
- Readers: outside the current scorer hot set.
- Current semantic level: COMOD1 runner-side mirror of current accepted/control state; do not fold it into formula counters.

## `unk_8008ED1C`
- Writers: outside this requested hot set.
- Readers: `sub_801C9094` consumes and clears it, then writes the corresponding `ctx+0x78` and sometimes `ctx+0x76` tail state.
- Current semantic level: event/tail dispatch latch for COMOD1 frame update; it is a host/tail gate, not bucket30 scorer state.
