# CD read pump `80038DE8` static export 2026-05-12

Authority: current IDA database `SCUS_941.83`, using Hex-Rays plus
disassembly checks. No dynamic replay, Win file read, STR, or XA behavior is
used as authority.

## Scope

- Primary function: `80038DE8` / `sub_80038DE8`.
- Consumers in this chain: `80038FC0` read-start and `800390C8` read-sync.
- Callback side: `80038BC4`, installed by `80038DE8` through `sub_80036528`.
- Final ready pump: `800390C8` always calls `800364F0`, which calls
  `800372F0`.

## Corrected pseudo C: `80038DE8`

Hex-Rays emits `sub_80036394(v4)` with an undefined `v4`. Disassembly shows
`80036394` is called with no meaningful argument; it returns `0`. Hex-Rays also
prints the local retry packet as `v6[0] = 0`, but disassembly shows the byte is
loaded from `dword_8005741C` and truncated to 8 bits.

```c
int sub_80038DE8(int retry)
{
    uint8_t mode_arg[8];

    sub_80036510(0);
    sub_80036528(0);

    if ((sub_80036384() & 0x10) != 0) {
        if ((sub_80035560(-1) & 0x3F) == 0)
            sub_80047F4C("CdRead: Shell open...\n");
        sub_80036678(1, 0);
        dword_8005742C = sub_80035560(-1);
        dword_80057424 = -1;
        return dword_80057424;
    }

    if (retry != 0) {
        sub_80047F4C("CdRead: retry...\n");
        sub_80036540(9, 0, 0);
        if (!sub_80036540(2, sub_800363B4(), 0)) {
            dword_80057424 = -1;
            return dword_80057424;
        }
    }

    sub_80036430();

    mode_arg[0] = (uint8_t)dword_8005741C;
    if (((mode_arg[0] != sub_80036394()) || retry != 0) &&
        !sub_80036540(0x0E, mode_arg, 0)) {
        dword_80057424 = -1;
        return dword_80057424;
    }

    dword_80057430 = sub_80036A78(sub_800363B4());
    sub_80036528(sub_80038BC4);
    dword_80057418 = dword_80057414;
    sub_80036678(6, 0);
    dword_80057424 = dword_80057410;
    dword_80057428 = sub_80035560(-1);
    return dword_80057424;
}
```

## `80038DE8` calls and direct dependencies

| Callee | Evidence | Role in this read pump |
|---|---:|---|
| `80036510(0)` | `80038DFC` | Clears/restores sync callback slot before pump setup. |
| `80036528(0)` | `80038E04` | Clears/restores ready/data callback slot before pump setup. |
| `80036384()` | `80038E0C` | Status read; bit `0x10` is shell-open/error branch. |
| `80035560(-1)` | `80038E20`, `80038E50`, `80038F90` | Vblank/tick clock source copied to read globals. |
| `80047F4C(...)` | `80038E3C`, `80038E88` | Debug logging only. |
| `80036678(1,0)` | `80038E48` | Shell-open branch CD action. |
| `80036540(9,0,0)` | `80038E98` | Retry stop/pause command before seek/read restart. |
| `800363B4()` | `80038EA0`, `80038F30` | Returns `byte_80057114`; used as current location buffer. |
| `80036540(2,loc,0)` | `80038EB0` | Retry seek/setloc command; failure makes read count `-1`. |
| `80036430()` | `80038ED8` | Lower pre-read setup boundary. |
| `80036394()` | `80038EF4` | Returns `0`; compared against low byte of `dword_8005741C`. |
| `80036540(0x0E,mode_arg,0)` | `80038F10` | Mode command; failure makes read count `-1`. |
| `80036A78(byte_80057114)` | `80038F38` | Converts current location bytes to sector/index value. |
| `80036528(80038BC4)` | `80038F54` | Installs read callback. |
| `80036678(6,0)` | `80038F74` | Starts/arms the actual read command/action. |

## Global state read/write by `80038DE8`

| Global | Access | Meaning in this chain |
|---|---|---|
| `dword_8005741C` | read at `80038EE0`; written by `80038FC0` | Read mode/flags. Low byte is sent as mode command `0x0E` argument. |
| `dword_8005742C` | write at `80038E60` | Read-start timeout base clock; also used by `800390C8` and `80038BC4`. |
| `dword_80057424` | write `-1` at `80038E68/80038EC8/80038F20`; write sector count at `80038F88`; read for return at `80038FA0` | Remaining read sectors / current read result. Negative means failed or retry needed. |
| `dword_80057430` | write at `80038F50` | Current expected sector/index derived from `byte_80057114`. |
| `dword_80057414` | read at `80038F60`; written by `80038FC0` | Destination pointer staged for callback copy. |
| `dword_80057418` | write at `80038F6C` | Current destination pointer advanced by callback. |
| `dword_80057410` | read at `80038F7C`; written by `80038FC0` | Requested sector count staged as active remaining count. |
| `dword_80057428` | write at `80038F98` | Last pump/read callback clock; used by `800390C8` retry timing. |
| `byte_80057114` | read through `800363B4()` | Current CD location argument buffer. |

## Callback side: `80038BC4`

`80038DE8` installs `80038BC4` as callback. Its key side effects are:

```c
if (callback_status != 1)
    dword_80057424 = -1;

if (callback_status == 1 && dword_80057424 > 0) {
    if (dword_80057420 == 0x200) {
        sub_8003690C(local_sector_header, 3);
        if (sub_80036A78(local_sector_header) != dword_80057430)
            dword_80057424 = -1;
    }

    sub_8003690C(dword_80057418, dword_80057420);
    dword_80057418 += dword_80057420 << 2;
    --dword_80057424;
    ++dword_80057430;
}

dword_80057428 = sub_80035560(-1);
if (dword_80057424 < 0)
    sub_80038DE8(1);
if (sub_80035560(-1) > dword_8005742C + 1200)
    dword_80057424 = -1;

if (dword_80057424 == 0 || timeout) {
    sub_80036510(dword_80057434);
    sub_80036528(dword_80057438);
    sub_80036540(9, 0, 0);
    if (dword_8005740C)
        dword_8005740C(dword_80057424 == 0 ? 2 : 5, saved_callback_arg);
}
```

Important callback globals:

- Reads `dword_80057420` as transfer unit/sector size.
- Advances `dword_80057418` by `dword_80057420 << 2`.
- Decrements `dword_80057424`; zero means completion.
- Increments `dword_80057430`.
- Restores callback slots from `dword_80057434` and `dword_80057438`.
- Optionally calls `dword_8005740C` with status `2` on completion or `5` on
  non-zero/non-complete exit.

## Consumer: `80038FC0`

Corrected from disassembly, because Hex-Rays drops the third argument:

```c
bool sub_80038FC0(int sector_count, int dst, int mode)
{
    dword_8005741C = mode;

    switch (mode & 0x30) {
    case 0x00: dword_80057420 = 0x200; break;
    case 0x20: dword_80057420 = 0x249; break;
    default:   dword_80057420 = 0x246; break;
    }

    dword_8005741C |= 0x20;
    dword_80057414 = dst;
    dword_80057410 = sector_count;
    dword_80057434 = sub_80036510(0);
    dword_80057438 = sub_80036528(0);
    dword_8005742C = sub_80035560(-1);

    if ((sub_80036384() & 0xE0) != 0)
        sub_800367A4(9, 0, 0);

    return sub_80038DE8(0) > 0;
}
```

Consumption rule: `80038FC0` is just read-start setup plus one
`80038DE8(0)` call. Its success is exactly `readPumpPsxReturn > 0`; payload
availability is not success authority.

## Consumer: `800390C8`

Corrected from disassembly, because Hex-Rays drops `a0/a1`:

```c
int sub_800390C8(int wait, void *ready_result)
{
    int result;

    do {
        if (sub_80035560(-1) > dword_8005742C + 1200) {
            result = -1;
        } else if (dword_80057424 < 0) {
            sub_80038DE8(1);
            result = dword_80057410;
        } else if (sub_80035560(-1) > dword_80057428 + 60) {
            sub_80038DE8(1);
            result = dword_80057410;
        } else {
            result = dword_80057424;
        }
    } while (!wait && result > 0);

    sub_800364F0(1, ready_result);
    return result;
}
```

Consumption rule: `800390C8` polls the shared globals written by
`80038FC0/80038DE8/80038BC4`. It may call `80038DE8(1)` for retry/pump after
negative remaining count or after `dword_80057428 + 60`. It returns:

- `-1` after `dword_8005742C + 1200` timeout.
- `dword_80057410` immediately after retry pump paths.
- `dword_80057424` on ordinary poll, including `0` completion.

With `wait == 0`, it loops while the result is positive. With `wait != 0`, it
performs one poll and returns that result. The known caller `8001A818` uses
`800390C8(1, 0)` in a loop until the return is no longer positive.

## Typed input/result fields for `PrStage1LowerCdProducerDirect`

Required input facts for direct translation of read start:

```c++
struct ReadPumpInput80038DE8 {
    bool retry;
    bool statusKnown;
    uint32_t cdStatus;              // result of 80036384
    bool clockKnown;
    int32_t clockNow;               // result of 80035560(-1)
    bool modeKnown;
    uint32_t modeWord8005741C;
    CommandWrapperResult80036540 retryPauseResult;   // 80036540(9, 0, 0)
    CommandWrapperResult80036540 retrySetlocResult;  // 80036540(2, byte_80057114, 0)
    CommandWrapperResult80036540 modeCommandResult;  // 80036540(0x0E, mode_arg, 0)
    bool startReadResultKnown;
    CommandWrapperResult80036678 startReadResult;    // 80036678(6, 0)
    bool locSectorKnown;
    int32_t locSector80057430;      // 80036A78(byte_80057114)
};

struct ReadPumpResult80038DE8 {
    bool produced;
    bool incomplete;
    int32_t psxReturn;              // final dword_80057424
    bool shellOpen;
    bool retryIssued;
    bool modeCommandIssued;
    uint8_t modeCommandArg0;
    bool callbackInstalled80038BC4;
    bool globalsKnown;
    int32_t remaining80057424;
    int32_t startClock8005742C;
    int32_t lastPumpClock80057428;
    int32_t activeDst80057418;
    int32_t activeSectorCount80057410;
    int32_t expectedSector80057430;
};
```

`ReadStartInput80038FC0` should carry or derive:

- Existing `LowerActionRequestMetadata request`.
- `sectorCount` / `dstPtr` / `mode` from the PSX call/action, not Win file
  state.
- `readPumpResultKnown`.
- `readPumpPsxReturn` from `ReadPumpResult80038DE8.psxReturn`.
- Optional `ReadPumpResult80038DE8` payload for debug/provenance.

`ReadStart` output rule:

```c++
feedback.psxReturn = readPumpPsxReturn > 0 ? 1 : 0;
feedback.success = feedback.psxReturn != 0;
```

`ReadSyncInput800390C8` needs more than the current `vblankClockKnown` and
`finalReadyResultKnown` if it is to be a direct translation:

```c++
struct ReadSyncInput800390C8 {
    LowerActionRequestMetadata request;
    bool wait;
    bool clockKnown;
    int32_t clockNow;
    bool startClockKnown;
    int32_t startClock8005742C;
    bool lastPumpClockKnown;
    int32_t lastPumpClock80057428;
    bool remainingKnown;
    int32_t remaining80057424;
    bool activeSectorCountKnown;
    int32_t activeSectorCount80057410;
    bool retryPumpResultKnown;      // required when remaining < 0 or clock > last+60
    ReadPumpResult80038DE8 retryPump;
    bool finalReadyResultKnown;
    int32_t finalReadyPsxReturn;    // 800364F0/800372F0 result
};
```

`ReadSync` output rule:

- If `clockNow > startClock8005742C + 1200`, PSX return is `-1`.
- Else if `remaining80057424 < 0` or `clockNow > lastPumpClock80057428 + 60`,
  require explicit `80038DE8(1)` result and return `activeSectorCount80057410`.
- Else return `remaining80057424`.
- Always require/carry `800364F0` / `800372F0` final ready result as a separate
  side-effect dependency, but do not substitute it for the `800390C8` return.

## Direct-port boundary

For `PrStage1LowerCdProducerDirect`, the CD lower producer should be driven by
explicit PSX lower action facts:

- `80038FC0` success is `80038DE8(0) > 0`.
- `800390C8` success/completion is the PSX return path above, not host payload
  readiness.
- Payload bytes may be attached only as data copied by a separate explicit
  read/callback fact; they must not authorize read-start or read-sync success.
- Do not infer any of these fields from Win file existence, STR, or XA state.

## Direct carrier progress

`PrStage1LowerCdProducerDirect` now models `80036540/80036678` as wrapper-level
typed carriers. They preserve the hidden `byte_80057108 & 0x10` command-1
restart branch, the optional `CdlSetloc` gate from `dword_80057078[cmd]`, the
four-attempt loop, repeated `dword_800570F8=0` clears, and the final
`80036540`/`80036678` `skipWait` difference.

`80038BC4` is also represented as a direct callback carrier. Its
`copiedSectorPayload` field only means the PSX callback path requires a sector
copy; it is not proof that Win payload bytes are available or valid.

`80038FC0` setup is now also represented explicitly. The setup carrier records
`mode & 0x30` to `dword_80057420`, `dword_8005741C = mode | 0x20`, staged dst
and sector count, saved `80036510/80036528` callbacks, start clock, and the
optional `800367A4(9,0,0)` side-effect when `(80036384() & 0xE0) != 0`. The
read-start seam requires both setup and `80038DE8(0)` pump results; setup alone
does not authorize read success.

`80038DE8` now carries the remaining mandatory side-effects from the pseudo C:
the entry `80036510(0)` / `80036528(0)` callback clears, the shell-open
`80036678(1,0)` command, and the `80036430()` pre-read setup boundary. These are
explicit facts; the direct carrier stays incomplete if a side-effect is absent.

`80038BC4` retry handling now consumes the explicit `80038DE8(1)` result and
copies its post-pump shared read globals back into the callback state when they
are known. A retry record by itself is not enough to advance
`dword_80057424/57418/57430`.
