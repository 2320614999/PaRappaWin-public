# 801C44E0 movie/text setup -> dword_801C9554 (2026-05-15)

Authority:
- `docs/stage1_decomp/ida_export_801c44e0_movie_text_setup_20260515.json`
- Current IDA session, `SCUS_941.83`

Scope:
- Only the `801C44E0` setup fact needed by `801C455C`:
  `dword_801C9554`.
- No STR playback state, host movie progress, ISO readability, vtext success,
  or renderer state is used as authority.

## Pseudocode fact

IDA decompiled `801C44E0(a1, a2)` as:

```c
sub_80026FA4(a1);
nullsub_8();
sub_80024E98();
v4 = *(s16 *)(a1 + 6);
dword_801C9554 = a2 != 1;
sub_8001A478(v4);
sub_80027288(a2);
sub_8001A4D0(a1, 1);
return sub_800274D4();
```

The exact writer is:

- `801C450C`: `xori $v0, $s0, 1`
- `801C4514`: `sltu $v0, $zero, $v0`
- `801C4518`: `sw $v0, dword_801C9554`

So `dword_801C9554` is known when the direct setup action provides a known
`a2`; the value is `a2 != 1`.

## Callsite facts

IDA xrefs show two direct callers:

- `801C4DC4 @ 801C4E34`: calls `801C44E0(sceneEntry+0x6C, 0)`, then
  `801C455C(sceneEntry+0x6C, &unk_801C3640, 0)`.
- `801C4894 @ 801C48F4`: calls `801C44E0(a1, 1)`.

For the `801C4DC4 -> 801C455C` movie/text outer-loop path, `a2=0`, therefore:

- `dword801C9554Known = true`
- `dword801C9554EqualsOne = true`

## Implementation boundary

The C++ path now treats existing `StrInit` / `ApplyPrStrPlayerInitSub801C7744`
as the `801C44E0` setup source for this one global. It records the setup value
as known, then `801C455C` consumes it through
`MovieTextOuterLoopInputSub801C455C`.

The rest of `801C44E0` remains only partially represented:

- `80024E98` has existing direct reset pieces.
- `8001A4D0` has existing lifecycle/XA action coverage.
- `80027288` and `800274D4` are not treated as fully direct-ported behavior in
  this change.

Do not infer `dword_801C9554` from vtext success, subtitle enable state,
`word_800916DC`, STR playback state, host progress, or renderer output.
