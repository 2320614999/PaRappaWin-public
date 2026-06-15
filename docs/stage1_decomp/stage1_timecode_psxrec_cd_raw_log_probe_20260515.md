# Stage1 psxrec CD raw-log coverage probe

Scope: only the `801C7560 -> 8001A3C8 -> 800364D0 -> 80037070 ->
80036AF8` lower-CD timecode source. This note does not authorize success from
status banks, final timecode bytes, STR/XA payloads, Win files, command
serials, or request metadata.

Recording:

```text
E:\game\PSgame\parappa the rapper\duckstation\bin\x64\标题画面-menu-选关选stage1-good通关-保存记忆卡（过场动画均为跳过）.psxrec
```

Probe output:

```text
docs/stage1_decomp/psxrec_cd_raw_log_probe_20260515.json
```

Command:

```powershell
python .\tools\psxrec_cd_raw_log_probe.py `
  "E:\game\PSgame\parappa the rapper\duckstation\bin\x64\标题画面-menu-选关选stage1-good通关-保存记忆卡（过场动画均为跳过）.psxrec" `
  --output docs\stage1_decomp\psxrec_cd_raw_log_probe_20260515.json
```

## Summary

The target frames have call-trace coverage for `80037070`, `80035898`, and
`80036AF8`, but this existing recording has no CD MMIO or return-capture block
coverage:

```text
frame 3905: cdLog=0 retCap=0 legacyReadLog=0 cdMmioRead=0 cdMmioWrite=0
  CdSync80037070=2 CheckCallback80035898=2 RawCdRegTransaction80036AF8=6
  after_RawCdRegTransaction80036AF8=2 return_from_CdSync80037070=2
  byte_800573D4=02 response_800882F8=2803020201036400

frame 3995: cdLog=0 retCap=0 legacyReadLog=0 cdMmioRead=0 cdMmioWrite=0
  CdSync80037070=2 CheckCallback80035898=2 RawCdRegTransaction80036AF8=4
  after_RawCdRegTransaction80036AF8=2 return_from_CdSync80037070=2
  byte_800573D4=02 response_800882F8=2804400201000000

frame 4255: cdLog=0 retCap=0 legacyReadLog=0 cdMmioRead=0 cdMmioWrite=0
  CdSync80037070=2 CheckCallback80035898=2 RawCdRegTransaction80036AF8=4
  after_RawCdRegTransaction80036AF8=2 return_from_CdSync80037070=2
  byte_800573D4=02 response_800882F8=2808660201026400

frame 4806: cdLog=0 retCap=0 legacyReadLog=0 cdMmioRead=0 cdMmioWrite=0
  CdSync80037070=2 CheckCallback80035898=2 RawCdRegTransaction80036AF8=4
  after_RawCdRegTransaction80036AF8=2 return_from_CdSync80037070=2
  byte_800573D4=02 response_800882F8=2818070201056400
```

## Recorder enhancement

The recorder has been extended for new recordings:

- DuckStation `MemoryRecorder` now writes a `CDR1` packed block with CDROM MMIO
  entries: `pc/addr/value/event_seq/access/size/offset`.
- It also writes an `RTR1` packed block for `JR $ra` returns whose target is
  `0x800371D4`, carrying `pc/target/sp/v0/event_seq`.
- `Bus::HWHandlers::CDROMRead/Write` logs byte-level accesses for
  `1F801800..1F801803`.
- `tools/模拟器内存回放器.py` now exposes `get_cd_mmio_log(frame)` and
  `get_return_captures(frame)`.
- `tools/psxrec_cd_raw_log_probe.py` consumes those new blocks when present and
  remains compatible with older recordings.

## Boundary

The existing psxrec gives useful frame/event ordering:

- `80037070` is reached twice per target frame in this window.
- `800371A4 -> 80035898 -> 800371AC` is present.
- `800371CC -> 80036AF8 -> 800371D4` is present.
- `800372E8` confirms the `80037070` return event.

It still cannot provide the builder authority needed by
`CdSyncLoopFactsInput80037070`:

- no CD MMIO read entries for `1F801800..1F801803`;
- no CD MMIO write entries for selector/ack writes;
- no `$v0` values on `800371AC`, `800371D4`, or `800372E8`;
- no FIFO byte sequence or result byte count;
- no saved/restored selector value.

Therefore this old recording remains consistency-only for status banks and call
order. A new recording made with the enhanced recorder can provide the CD MMIO
and return-register facts needed to build `CdSyncLoopFactsInput80037070`.

## v2 Enhanced-Recorder Probe

Recording:

```text
E:\game\PSgame\parappa the rapper\duckstation\标题画面-menu-选关选stage1-good通关-保存记忆卡（过场动画均为跳过）v2.psxrec
```

Probe output:

```text
docs/stage1_decomp/psxrec_cd_raw_log_probe_v2_20260515.json
```

The v2 recording proves the new `CDR1/RTR1` blocks are present and usable:

```text
frame 3905: cdLog=36 retCap=2
  801C757C -> 8001A3C8 -> 800364D0 -> 80037070
  wait-loop 800371CC -> 80036AF8 -> 800371D4 captured v0=0
  second wait-loop 800371CC -> 80036AF8 -> 800371D4 captured v0=0
  interrupt-service 80038144 -> 80036AF8 has fifo=2812440201000000 ack=3
  byte_800493F4..F6 after poll = 28 12 43
  byte_80057119=10 byte_800573D4=02 response_800882F8=2812440201000000

frame 3995: cdLog=36 retCap=2
  wait-loop raw returns v0=0 / v0=0
  interrupt-service fifo=2814070201016400 ack=3
  byte_800493F4..F6 after poll = 28 14 06
  byte_80057119=10 byte_800573D4=02 response_800882F8=2814070201016400

frame 4255: cdLog=51 retCap=2
  wait-loop raw returns v0=0 / v0=0
  interrupt-service fifo=2818330201076400 ack=3
  extra interrupt-service fifo=22 ack=3
  byte_800493F4..F6 after poll = 28 18 32
  byte_80057119=10 byte_800573D4=02 response_800882F8=2818330201076400

frame 4806: cdLog=36 retCap=2
  wait-loop raw returns v0=0 / v0=0
  interrupt-service fifo=2827490201026400 ack=3
  byte_800493F4..F6 after poll = 28 27 48
  byte_80057119=10 byte_800573D4=02 response_800882F8=2827490201026400
```

Authority boundary:

- The `8001A3C8` normal target path is visible in each target frame:
  `801C757C -> 8001A3C8`, `8001A3D4 -> 800364D0`,
  `800364D8 -> 80037070`, and `8001A41C -> 800363A4`.
- The target path should be represented at the Win seam as explicit lower
  feedback only: `800364D0(1, sp+0x10)` returns `2`, copies response bytes into
  the caller status buffer, `800363A4()` returns `byte_80057119=0x10`, and
  `8001A3C8` writes `byte_800493F4..F6` from those status bytes, keeps
  `dword_80049428=0`, and returns `1`.
- v2 is still not the final `CdSyncLoopFactsInput80037070` authority for the
  FIFO-producing interrupt-service calls because it was recorded before the
  extra return capture targets. It has no `RTR1` capture for
  `80038144 -> 80036AF8 -> 8003814C`, and no `RTR1` capture for the
  `80037070` epilogue `pc=800372E8`.
- The recorder has now been rebuilt to also capture `target=0x8003814C` and
  `pc=0x800372E8`. The next recording should be the single remaining recorder
  pass for return-value authority; no CD hardware implementation translation is
  needed.

## v3 Return-Authority Closure

Recording:

```text
E:\game\PSgame\parappa the rapper\duckstation\标题画面-menu-选关选stage1-good通关-保存记忆卡（存档重名保存失败重试成功）（过场动画均为跳过）v3.psxrec
```

Probe output:

```text
docs/stage1_decomp/psxrec_cd_raw_log_probe_v3_20260516.json
```

The rebuilt recorder captured both missing return-authority windows:

```text
frame 3905:
  800372E8 -> 800364E0 v0=2
  800372E8 -> 80037690 v0=2
  80038144 -> 80036AF8 -> 8003814C fifo=2812420201066400 v0=2
  no-interrupt drain -> 8003814C v0=0

frame 3995:
  800372E8 -> 800364E0 v0=2
  800372E8 -> 80037690 v0=2
  80038144 -> 80036AF8 -> 8003814C fifo=2814050201076400 v0=2
  no-interrupt drain -> 8003814C v0=0
  async interrupt fifo=22 v0=4, then no-interrupt drain v0=0

frame 4255:
  800372E8 -> 800364E0 v0=2
  800372E8 -> 80037690 v0=2
  80038144 -> 80036AF8 -> 8003814C fifo=2818310201056400 v0=2
  no-interrupt drain -> 8003814C v0=0

frame 4806:
  800372E8 -> 800364E0 v0=2
  800372E8 -> 80037690 v0=2
  80038144 -> 80036AF8 -> 8003814C fifo=2827470201000000 v0=2
  no-interrupt drain -> 8003814C v0=0
  async interrupt fifo=22 v0=4, then no-interrupt drain v0=0
```

The status bytes consumed by `8001A3C8` are not the frame-end
`response_800882F8` bank. `80037070` copies the caller output buffer at
`800372AC` before the later interrupt-service call updates `800882F8`:

```text
frame 3905: 800372AC writes statusBuf = 2812410201056400; 8001A3C8 writes F4/F5/F6 = 28/12/41
frame 3995: 800372AC writes statusBuf = 2814030201056400; 8001A3C8 writes F4/F5/F6 = 28/14/03
frame 4255: 800372AC writes statusBuf = 2818290201036400; 8001A3C8 writes F4/F5/F6 = 28/18/29
frame 4806: 800372AC writes statusBuf = 2827450201066400; 8001A3C8 writes F4/F5/F6 = 28/27/45
```

Final seam facts for the target path:

- `800364D0(1, statusBuf)` returns `2`.
- It copies eight explicit status bytes to the caller buffer.
- `800363A4()` returns `0x10`.
- `8001A3C8` keeps `dword_80049428=0`, writes `byte_800493F4..F6` from
  `statusBuf[0..2]`, and returns `1`.
- The Win side should provide these fields as explicit HAL/direct seam facts.
  It must not translate CD hardware internals and must not use frame-end
  status banks, STR/XA progress, payload, ISO readability, Win files, command
  serials, or final timecode bytes as return authority.
