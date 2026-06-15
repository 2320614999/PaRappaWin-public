# `800375BC` CD command static export 2026-05-12

Authority: IDA Hex-Rays pseudo-C from current SCUS database.

## `800375BC` / `sub_800375BC`

Role: low-level CD command sender used by seek/read lower code.

Key pseudo-C facts:

```c
int sub_800375BC(unsigned __int8 a1, char *a2, int a3, int a4)
{
  if (dword_8005733C[a1] && !a2)
    return -2;

  sub_80037070(0);

  if (a1 == 2) {
    for (i = 0; i < 4; ++i)
      byte_80057114[i] = a2[i];
  }

  byte_800573D4 = 0;
  if (dword_8005723C[a1])
    byte_800573D5 = 0;

  CD_REG0 = 0;
  for (i = 0; i < dword_8005733C[a1]; ++i)
    CD_REG2 = a2[i];

  byte_80057119 = a1;
  CD_REG1 = a1;

  if (!a4) {
    dword_80088310 = sub_80035560(-1) + 960;
    dword_80088314 = 0;
    dword_80088318 = "CD_cw";
    while (1) {
      if (dword_80088310 >= sub_80035560(-1) &&
          dword_80088314++ <= 3932160) {
        timedOut = 0;
      } else {
        sub_80047F4C("CD timeout: ");
        printf("%s:(%s) Sync=%s, Ready=%s\n",
               dword_80088318, "CdlSync", "NoIntr", "NoIntr");
        sub_80037A8C();
        timedOut = -1;
      }

      if (timedOut)
        return -1;

      if (CheckCallback()) {
        prior = CD_REG0 & 3;
        while (sub_80036AF8())
          ;
        CD_REG0 = prior;
      }
    }
  }

  return 0;
}
```

## Direct-port boundary

`800375BC` cannot be replaced by "command was requested":

- it first drains `80037070(0)`;
- it copies Setloc args to `byte_80057114` for command `2`;
- it writes command arguments and command code to CD registers;
- it updates `byte_80057119`;
- unless `a4` skips waiting, it runs the timeout loop and repeatedly pumps
  `CheckCallback -> 80036AF8`.

Current `PrStage1XaCdDirectApplySub800375BCCommand(...)` only records command
facts plus a shallow optional callback pass. It is not yet a complete
translation of this function.

## Implementation consequence

`PrStage1LowerCdProducerDirect` may consume an explicit translated
`800375BC` result, but it must not synthesize that result from request metadata,
Win file availability, STR/XA state, or payload presence.

`PrStage1LowerCdProducerDirect` now models the wait-loop portion as an explicit
transaction. For `a4 == 0`, the carrier requires:

- a clock fact for the `now + 960` timeout deadline;
- explicit timeout/non-timeout status;
- explicit `CheckCallback` pending status;
- when pending, saved `CD_REG0 & 3`, drained `80036AF8` pump, final pump return
  `0`, and selector restore;
- an explicit final wait-loop return. Timeout can produce `-1`; normal `0`
  must still be supplied by a closed lower-event path.

This keeps `800375BC` from becoming a synthetic success gate.

## Command tables

Additional IDA export:
`docs/stage1_decomp/ida_export_800375bc_command_tables_compact_temp3_20260512.json`.

`dword_8005733C` is the command parameter-count table. Static evidence only
proves 32 entries before the CD register pointer area starts at `0x800573BC`,
so direct code must treat command `>= 32` as unknown instead of reading past the
table:

```text
index:  0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
count:  0 0 3 0 0 0 0 0 0 0 0  0  0  2  1  0  0  0  1  0  1  0  0  0  0  0  0  0  0  0  0  0
```

`dword_8005723C` is the attribute table read by `800375BC` before command issue.
Nonzero entries clear `byte_800573D5`. Its full semantic name remains unproven;
use a neutral field such as `resetReadyByte573D5`.

Known command facts used by the current lower-CD chain:

- `2` (`CdlSetloc`) has 3 CD-register parameter bytes and additionally mirrors
  `a2[0..3]` to `byte_80057114`.
- `13` has 2 parameter bytes.
- `14` has 1 parameter byte.
- `27` has 0 parameter bytes and a nonzero `dword_8005723C` attribute.
- Commands outside `0..31` are a direct-port gap for this table.
