# Stage1 8001A3C8 -> 800364D0 lower feedback observable

Scope: only the Stage1 timecode chain
`801C7560 -> 8001A3C8 -> 800364D0/800363A4 -> byte_800493F4..F6`.
This note does not authorize lower-CD success from XA/STR payload, Win files,
command metadata, or final timecode bytes.

## Existing runtime gap

The latest full-song runtime reached `max_query_frame=4806`, but
`full_song.rowWritebackLastTimecode` still reported:

```text
pollA3C8=True pollRet=0 pollAccept=False pollGapD0=True pollGapA4=False
cmdA280=True cmdA280Issued=False cmdA280Gap=True cmdA280Base=0
xaCdF4K=False xaCdClockGap=True
```

This means the same-frame `8001A3C8` call and `8001A280` observable are in the
main path, but `800364D0` has no explicit lower feedback in
`ctx.stage1XaCdDirect` for that tick.

## Recorder facts

The target-frame recorder facts remain valid:

- target frames: `3905/3995/4255/4806`
- `801C757C -> 8001A3C8`
- `8001A3D4 -> 800364D0`
- `8001A41C -> 800363A4`
- `byte_80057119=0x10`
- `byte_800573D4=0x02`
- `800882F8[0..7]`:
  - frame `3905`: `28 03 02 02 01 03 64 00`
  - frame `3995`: `28 04 40 02 01 00 00 00`
  - frame `4255`: `28 08 66 02 01 02 64 00`
  - frame `4806`: `28 18 07 02 01 05 64 00`

Additional recorder writer facts around frame `3905`:

- `writers 0x800573D4 1 3898 3906` shows paired writes from
  `800376D0` and `80036E18` each frame, ending with `byte_800573D4=0x02`.
- `writers 0x800882F9 1 3898 3906` shows `80036E40` updates the second
  response byte, including frame `3903` `0x02 -> 0x03`.
- `writers 0x800882FA 1 3898 3906` shows `80036E40` updates the third
  response byte each frame, including frame `3905` `0x01 -> 0x02`.

These facts are status-bank observations. They do not by themselves close the
ordered `80037070 -> 80036AF8` wait-loop semantics or raw CD_REG transaction
sequence.

## Ring/pump exclusion

Static review confirms the XA ring path is not a lower feedback source:

- `PumpStage1XaCdDirectRingPackets()` consumes accepted XA ring packets and
  calls `PrStage1XaCdDirectApplySub80039670Packet(...)`; it does not carry
  `80036AF8` raw CD register transactions or `80037070` sync results.
- `PrStage1XaCdDirectApplySub80039670Packet(...)` reports ring packet state
  such as `consumed/accepted/frameReady/statusCode/writeIndex`, not
  lower-CD wait-loop facts.
- `PrStage1XaCdDirectApplySub80039240PumpCallback()` and
  `PrStage1XaCdDirectApplySub80039318DmaCallback()` only record callback/DMA
  ring state.

Therefore do not extend the XA ring result to synthesize
`CdSyncLowerFeedback80037070`.

## Valid next source

The valid carrier is already present:

```text
PrStage1XaCdDirectLowerCdSnapshotBridgeInput
  -> PrStage1XaCdDirectBuildLowerCdProducerSnapshot(...)
  -> PrStage1XaCdDirectApplyLowerCdProducerSnapshot(...)
  -> state.cdLowerFeedback80036AF8
  -> PrStage1XaCdDirectApplySub800364D0CdSyncFromLowerState(...)
```

The next code step must populate the bridge input from explicit lower facts:

- preferred: `cdSyncLoopFacts80037070Known + cdSyncLoopFacts80037070`
- acceptable: explicit `cdSyncFeedback80037070Known + cdSyncFeedback80037070`
  only if it comes from a real lower facts source

Do not infer any of these from command serials, XA ring status, STR/XA payload,
Win file readability, recorder sampling values, or final `byte_800493F4..F6`.
