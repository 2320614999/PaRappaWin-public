# lower-CD sync/ready wait-loop facts producer evidence

Scope: static / IDA-only evidence for the shared `80037070` / `800372F0`
wait-loop facts producer. This note is implementation guidance for
`BuildCdSyncLoopFacts80037070(...)` and the parallel `ReadyWait800372F0`
input shape; it does not authorize Win-side CD, XA, STR, file, ISO, payload, or
replay facts.

Primary export:
`ida_export_cd_sync_waitloop_80037070_800372f0_20260513.txt`.

Prior exports used:

- `ida_export_cd_lower_80035898_800359b8_80036af8_80037070_20260512.md`
- `ida_export_cd_lower_read_callbacks_80038bc4_80038fc0_800390c8_800372f0_20260512.txt`
- `ida_export_cd_lower_producer_followup_8001a818_80038de8_800364f0_20260513.txt`

## Producer Target

The minimum closed chain is:

`800359B8 interrupt snapshot -> 80035898 CheckCallback -> 80037070/800372F0 wait loop -> 80036AF8 raw CD register pump`.

The producer must emit only explicit PSX-shaped facts:

- timeout deadline / spin facts from `80035560(-1)`, `80088310`, `80088314`;
- callback pending from `word_80055F7A`;
- saved/restored `CD_REG0 & 3` selector from `dword_800573BC`;
- raw `80036AF8` pump transactions from `CD_REG0..3` and result FIFO bytes;
- status/result banks `byte_800573D4/D5/D6` and
  `800882F8/80088300/80088308`.

## Function Facts

### `80035898 CheckCallback`

Hex-Rays still prints a false `return 0`; use disassembly:

```asm
80035898: lhu     $v0, word_80055F7A
800358A0: jr      $ra
```

`CheckCallback()` is a read of `word_80055F7A`, not a host playback state.
Current xrefs are `80037070`, `800372F0`, and `800375BC`.

### `800359B8` interrupt dispatcher

The dispatcher sets `word_80055F7A = 1` before it drains interrupt callbacks,
then clears it before returning:

```asm
80035A20: sh      $v0, word_80055F7A
...
80035B6C: sh      $zero, word_80055F7A
```

It gates on `word_80055F78`, combines I_STAT/I_MASK through
`dword_80057004`, `dword_80057008`, and `word_80055FA8`, then dispatches
callback table entries at `80055F7C`.

Producer implication: `callbackPending` must come from this interrupt snapshot
or a dynamic watchpoint equivalent. It cannot be synthesized from Win frame
progress, XA playback, file readability, or request metadata.

### `80036AF8` raw CD register pump

The pump owns the raw CD lower event facts:

- writes selector `1` to `CD_REG0` via `dword_800573BC`;
- reads interrupt code from `CD_REG3 & 7` via `dword_800573C8`;
- reads up to 8 FIFO bytes while `CD_REG0 & 0x20` is set, via
  `dword_800573BC` and `dword_800573C0`;
- pads missing bytes to zero;
- acknowledges by writing `7` through `CD_REG3` and `CD_REG2`;
- updates `byte_800573D4/D5/D6` and copies result bytes into
  `800882F8/80088300/80088308`;
- returns an event mask consumed by the wait loops.

Disassembly is the authority where Hex-Rays conflicts. Notably, interrupt case
4 writes `byte_800573D6 = 4` and mirrors that into `byte_800573D5`; ready wait
still consumes `D6` first.

Minimum producer facts for one pump result:

- initial `CD_REG3 & 7` interrupt code;
- stable `CD_REG3 & 7` observation;
- FIFO byte count and up to 8 bytes;
- prior `dword_80057108`, `dword_80057110`, and command `byte_80057119`
  where the existing event builder requires them;
- resulting PSX return mask from `80036AF8`.

### `80037070` CD sync loop

Entry sets:

```asm
8003709C: jal     sub_80035560
800370B8: addiu   $v0, 0x3C0
800370BC: sw      $v0, dword_80088310
800370CC: sw      $zero, dword_80088314
800370D4: sw      $v0, dword_80088318  # "CD_sync"
```

Loop timeout check:

- timeout if current `80035560(-1)` is after `dword_80088310`;
- timeout if pre-increment `dword_80088314` is greater than `0x3C0000`;
- timeout path prints, calls `80037A8C`, and returns `-1`.

Callback path:

```asm
800371A4: jal     CheckCallback
800371B4: lw      $v0, dword_800573BC
800371C8: andi    $s1, $v0, 3
800371CC: jal     sub_80036AF8
...
8003724C: lw      $v0, dword_800573BC
80037258: sb      $s1, 0($v0)
```

The producer must save `CD_REG0 & 3`, pump `80036AF8` until the final return is
0, and prove the selector restore. If `CheckCallback()==0`, the direct result
is `syncResult=0`; this no-pending result must not be mixed with a raw event.

After the pump, `80037070` consumes sync status from `byte_800573D4`. Status
2 or 5 copies `800882F8` to `a1` when non-null and returns the consumed status;
otherwise `a0 == 0` loops and `a0 != 0` returns 0.

### `800372F0` CD ready loop

The shape is the same loop with label `"CD_ready"` and the same timeout gates:

```asm
80037320: jal     sub_80035560
80037340: addiu   $v0, 0x3C0
80037344: sw      $v0, dword_80088310
80037354: sw      $zero, dword_80088314
8003735C: sw      $v0, dword_80088318  # "CD_ready"
```

Callback pump / selector restore matches `80037070`:

```asm
8003742C: jal     CheckCallback
8003743C: lw      $v0, dword_800573BC
80037450: andi    $s1, $v0, 3
80037454: jal     sub_80036AF8
...
800374D4: lw      $v0, dword_800573BC
800374E0: sb      $s1, 0($v0)
```

Ready result priority is:

1. If `byte_800573D6 != 0`, clear `D6`, optionally copy `80088308` to `a1`,
   and return the consumed `D6`.
2. Else if `byte_800573D5 != 0`, clear `D5`, optionally copy `80088300` to
   `a1`, and return the consumed `D5`.
3. Else `a0 == 0` loops and `a0 != 0` returns 0.

For the `800390C8` final-ready side effect, only the caller-proven
`800364F0(1,a1) -> 800372F0(1,a1)` route should populate
`ReadyWaitInput800372F0.a0WaitModeKnown/a1OutputBufferPtrKnown`.

### `800375BC` command wait loop

`800375BC` first drains `80037070(0)`, then runs a command wait loop with the
same `CheckCallback -> 80036AF8` pump shape. This is useful as a second
consumer of the shared facts producer, but it is not a replacement for
`8001A818/8001A89C` request metadata and does not authorize request success.

## Field Mapping

For `CdSyncLoopFactsInput80037070`:

- `timeoutKnown/timedOut`: from `80035560(-1)`, `80088310`, `80088314`,
  deadline `+0x3C0`, and spin limit `0x3C0000`.
- `checkCallbackKnown/callbackPending`: from `CheckCallback()` reading
  `word_80055F7A`.
- `savedCdReg0SelectorKnown/savedCdReg0Selector`: from `*dword_800573BC & 3`
  immediately before the pump.
- `rawPumpSequenceKnown/rawPumpCount/rawPump/rawPumpPsxReturns`: every
  `80036AF8` transaction in order.
- `callbackPumpDrained`: true only when the final `80036AF8` return is `0`.
- `selectorRestoredKnown/selectorRestored/restoredCdReg0Selector`: from the
  post-drain write back to `*dword_800573BC`.

For `ReadyWaitInput800372F0`:

- `a0WaitModeKnown/a0WaitMode`: caller-proven argument, e.g.
  `800364F0(1,a1)` for read-sync final ready.
- `a1OutputBufferPtrKnown/a1OutputBufferPtrNonNull`: caller-proven `a1`.
- `timeoutCheckKnown/timedOut`: same deadline/spin facts as `80037070`.
- `callbackCheckKnown/callbackPending`: same `word_80055F7A` source.
- `callbackPumpDrainedKnown/callbackPumpDrained`: same pump drain proof.
- `rawCallbackTransactionSequenceKnown/count/transactions/returns`: the
  complete `80036AF8` pump sequence.
- `callbackMask`: the OR/observed return masks from the pump sequence; dispatch
  bits are `4` for ready callback and `2` for sync callback.
- `readyCallbackRegistered/syncCallbackRegistered`: presence of
  `dword_800570FC` / `dword_800570F8`, respectively.
- `byte800573D6/response88308`: ready-priority status and buffer.
- `byte800573D5/response88300`: fallback ready status and buffer.

## Remaining Implementation Gap

This export closes the static ownership map, but it does not yet provide a
runtime facts source. The next step is to wait for or capture PSX-shaped
runtime facts for the interrupt/CD-register fields listed above; only after
those facts exist should code feed the existing builders. It must reject
incomplete facts rather than falling back to Win media state.

Hard prohibitions:

- no `PrCd` / Win filesystem / ISO readability as `handled` or `success`;
- no STR/XA playback state as callback pending, timeout, or ready status;
- no payload presence or replay bytes as CD register FIFO facts;
- no request-only metadata as PSX return source.
