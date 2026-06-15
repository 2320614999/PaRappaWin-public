# CD lower seek/read/sync static export 2026-05-12

Authority: IDA Hex-Rays pseudo-C from current SCUS database. Dynamic replay is
not used as implementation authority here.

## `800367A4` / `sub_800367A4`

Role: seek/sync wrapper used by `8001A89C`.

Key pseudo-C facts:

```c
v6 = 3;
v7 = &dword_80057078[a1];
while (1) {
  dword_800570F8 = 0;
  if (!a2 || !*v7 || !sub_800375BC(2, a2, a3, 0)) {
    dword_800570F8 = 0;
    if (!sub_800375BC(a1, a2, a3, 0)) {
      v9 = 0;
      break;
    }
  }
  if (--v6 == -1) {
    dword_800570F8 = 0;
    v9 = -1;
    break;
  }
}
return !v9 && sub_80037070(0, a3) == 2;
```

Implementation consequence: seek success is not request existence. It depends on
`800375BC` command execution and `80037070(0, a3) == 2`.

## `80038FC0` / `sub_80038FC0`

Role: read-start setup used by `8001A818`.

Key pseudo-C facts:

```c
dword_80057420 = 512;
dword_8005741C = 32;
dword_80057414 = a2;
dword_80057410 = a1;
dword_80057434 = sub_80036510(0);
dword_80057438 = sub_80036528(0);
dword_8005742C = sub_80035560(-1);
if ((sub_80036384() & 0xE0) != 0)
  sub_800367A4(9u, 0, 0);
return sub_80038DE8(0) > 0;
```

Implementation consequence: read-start success is the return of
`80038DE8(0)>0`, not payload availability or Win file readability.

## `800390C8` / `sub_800390C8`

Role: read sync poll used by `8001A818`.

Key pseudo-C facts:

```c
v0 = -1;
if (sub_80035560(-1) <= 1200) {
  if (sub_80035560(-1) > 60)
    sub_80038DE8(1);
  v0 = 0;
}
sub_800364F0();
return v0;
```

Implementation consequence: sync returns `0` while inside timeout and `-1` once
timed out; it always calls `800364F0`.

## `800364F0` / `sub_800364F0`

```c
return sub_800372F0();
```

## Direct dependencies requiring separate closure

- `800375BC`: CD command write/wait loop; it calls `80037070(0)`, writes CD
  command/argument registers, and pumps `80036AF8` while callback is pending.
- `80038DE8`: read pump/retry body; sets read globals and installs callbacks.
- `800372F0`: ready wait loop; pumps `80036AF8` via `CheckCallback`.
- `80037070`, `80036AF8`, `80035898`, `800359B8`: existing partial direct
  translations must remain the lower feedback source, not Win playback state.

## `80036AF8` raw CD register transaction carrier

`PrStage1LowerCdProducerDirect` now has a typed carrier for the raw facts
needed by `80036AF8`: initial/stable `CD_REG3` interrupt, `CD_REG0 & 0x20`
FIFO drain, result byte count/bytes, the `CD_REG0=1 / CD_REG3=7 / CD_REG2=7`
ack sequence, the interrupt-case-1 `CD_REG0=0 / CD_REG3=0` clear, and the
prior `dword_80057108` / `dword_80057110` / `byte_80057119` facts.

This carrier is intentionally fact-only. If any register transaction fact is
missing, the lower producer stays `incomplete`; it must not derive
`psxReturn`, status, or response bytes from STR/XA ring state, payload
availability, or request metadata.

## Boundary rule

The lower producer must output a handled `CdSeamResult` only after explicit lower
feedback exists:

- `feedback.kind` matches `SeekSync800367A4`, `ReadStart80038FC0`, or
  `ReadSync800390C8`.
- `feedback.handled == true`.
- `feedback.success` and `feedback.psxReturn` come from the translated lower CD
  function result.
- read payload facts (`dstPtr`, `sectorCount`, live bytes) may be attached, but
  they do not authorize success.
