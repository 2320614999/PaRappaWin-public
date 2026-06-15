# Stage1 80037070 / 80036AF8 live GDB capture plan

Scope: only `801C7560 -> 8001A3C8 -> 800364D0 -> 80037070 -> 80036AF8`.
Do not use this line to infer lower-CD success from XA ring state, STR/XA
payload, Win files, command serials, status-bank samples, or final timecode
bytes.

## Current availability

DuckStation GDB remote port was probed at `127.0.0.1:2345` and refused the
connection:

```text
ConnectionRefusedError(10061, '由于目标计算机积极拒绝，无法连接。', ...)
```

Probe output:

```text
docs/stage1_decomp/duck_gdb_cd_raw_pump_probe_20260515.json
```

No live facts were captured in this round.

## Capture script

The narrow capture helper is:

```text
tools/duck_gdb_cd_raw_pump_capture.py
```

When DuckStation GDB server is enabled and the target recording/gameplay is
running near the target path, run:

```powershell
python .\tools\duck_gdb_cd_raw_pump_capture.py `
  --output docs\stage1_decomp\duck_gdb_cd_raw_pump_capture_20260515.json `
  --max-hits 32
```

The script sets only these execution breakpoints:

```text
80037070 8003709C 800370BC 800370DC
80037124 8003719C 800371A4 800371AC 800371B4
800371C8 800371CC 800371D4
80036AF8 80036B14 80036B30 80036B88 80036BA8
80036BFC 80036C0C 80036E18 80036E40 80036E8C
80037054 80037058 80037258 8003725C 800372C4
```

It reads registers and these memory windows at each hit:

```text
1F801800[4]      CD_REG0..3
800573BC[0x10]  CD_REG0..3 pointer table
80055F78[0x34]  callback/interrupt side state
1F801070[8]     I_STAT/I_MASK
80057004[0x10]  callback dispatch side state
80057108[0x18]  lower status/prior globals
800573D4[4]     sync/ready status bytes
800882F8[8]     sync response bank
80088300[8]     ready response bank
80088308[8]     ready response bank
80088310[0x0C]  timeout/spin state
$sp+0x10[0x20]  80036AF8 interrupt local and FIFO/result byte buffer
```

The JSON now also contains:

```text
captures[*].seq
captures[*].breakpoint.address / label / pc_candidate
captures[*].stop_registers_decoded
typedFactsDraft80037070
```

`typedFactsDraft80037070` is a conservative live-facts draft. Fields remain
`known=false` until a matching breakpoint hit proves them; it is not a runtime
success producer.

## Required facts for `CdSyncLoopFactsInput80037070`

The live capture must establish:

- `timeoutKnown=true`, `timedOut=false`
- `checkCallbackKnown=true`, `callbackPending=true`
- `savedCdReg0SelectorKnown=true` at `800371C8`
- `selectorRestoredKnown=true`, `selectorRestored=true`,
  `restoredCdReg0Selector == savedCdReg0Selector` at `80037258`
- ordered `rawPumpSequenceKnown=true`, `rawPumpCount=N`
- each `rawPump[i]` corresponds to one `80036AF8` call and has matching
  `rawPumpPsxReturns[i]`
- final `80036AF8` return is `0`, proving `callbackPumpDrained=true`

`800371AC` is the `CheckCallback()` return point. `800371D4` and `80037058`
are the most important raw-pump return-value breakpoints: `$v0` there is the
live `80036AF8` return. `800372C4` is the wait-loop return epilogue. Do not
substitute any of these from `byte_800573D4` or response-bank values.

Each non-zero `80036AF8` transaction must provide:

- CD register pointer facts for `1F801800..1F801803`
- selector write `CD_REG0=1`
- initial and stable `CD_REG3 & 7` interrupt values
- `CD_REG0` FIFO status samples
- FIFO result bytes and byte count from `CD_REG1`
- ack writes `CD_REG0=1`, `CD_REG3=7`, `CD_REG2=7`
- case-1 clear writes when interrupt is `1`
- prior facts from `80057108/80057110/80057119`
- real `80036AF8` return value

## Consistency-only facts

Existing recorder facts may only be used to check the live capture:

- frame `3905` status bank has `byte_800573D4=02`
- frame `3905` response bank has `800882F8[8]=28 03 02 02 01 03 64 00`
- `80036E18/80036E40` are the observed bank writers
- `800376D0` resets `byte_800573D4`
- `byte_80057119=10`

They cannot fill `RawCdRegTransactionResult80036AF8` or
`CdSyncLoopFactsInput80037070` by themselves.
