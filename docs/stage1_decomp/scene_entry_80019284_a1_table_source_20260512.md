# 80019284(a1) a1 memory source and dword_80049278 initial table

Scope: static-only整理 for closing high-score record tail carry. Authority is
IDA pseudocode/disassembly plus current IDB data bytes. No build, no runtime
replay, no source-code edits.

## 1. Caller/context

IDA caller chain:

```c
// sub_80015788
v4 = sub_80019414(&byte_80092F10);
if (v4)
  sub_80026B94(6, v4);

// sub_80019414
sub_800191E4(a1, 3);
if (*(gp + 720) == 1)
  return sub_80019284(a1);
return 0;
```

`80019284` therefore receives `a1 = &byte_80092F10`.

Address arithmetic:

```text
byte_80092F10 + 4876 == 0x80092F10 + 0x130C == 0x8009421C
```

Conclusion: `80019284(a1)` intentionally reads past the 4876-byte save/status
payload into the adjacent high-score bank at `0x8009421C`. It is not a separate
replay/staging buffer, and it is not safe to model with `PrCard::Payload()`
alone.

Relevant producers/owners around `byte_80092F10`:

- `80015CC4`: `sub_80025C44(&byte_80092F10, 4876)` then
  `8001635C(1,1,1,0)`.
- `800164B4(src)`: `sub_80025C64(src, byte_80092F10, 4876)`.
- `8001635C`: writes save/status metadata inside `byte_80092F10` and copies
  `byte_8008EEF8 -> byte_80092F5C` for 4800 bytes.

Relevant high-score bank producers:

- `80019D7C case17`: calls `800168DC()` first, then for each successful card
  read calls `800164F8(byte_8007ADE8)`.
- `800168DC`: clears the high-score bank by setting each slot score to `0` and
  each slot first name byte to NUL.
- `800164F8`: merges one save payload into `0x8009421C` rows and calls
  `80016000(rowBase, 4)`.

## 2. a1+4876..5243 layout and bank relationship

`80019284` loop from IDA:

```c
v3 = 4876;
for (row = 0; row < 6; ++row) {
  v7 = row << 6;
  v8 = (char *)(a1 + v3);
  for (col = 0; col < 3; ++col) {
    score = *(s32 *)(a1 + v7 + 4888);
    name = v8;
    ...
    v8 += 16;
    v7 += 16;
  }
  v3 += 64;
}
```

Typed layout:

```text
a1 + 4876 + row*64 + col*16 == 0x8009421C + row*64 + col*16
  +0x00: char name[12]
  +0x0C: s32 score

row count: 6
visible cols read by 80019284: 3
physical slots in bank row: 4
row stride: 64
slot stride: 16
```

The requested source range is:

```text
a1+4876..5243
== 0x8009421C..0x8009438B
== six rows, visible slots 0..2, including the last visible score dword.
```

Relationship to `800164F8/80016000`:

- `800164F8(payload)` writes a candidate into physical slot 3
  (`rowBase+0x30`) when it is not a duplicate of visible slots 0..2.
- `80016000(rowBase, 4)` sorts all 4 physical slots by signed score descending.
- `80019284` reads only visible slots 0..2 after that convergence. Slot 3 is
  retained in memory but not displayed by this builder.

## 3. dword_80049278 initial table owner

`80019284` writes into `dword_80049278` and returns that address:

```c
v5 = 16 * cell + 20;
if (score > 0) {
  sub_8004800C(tmp, "%4d %-3.3s", score, name);
  sub_80017FC4((char *)dword_80049278 + v5, tmp);
} else {
  *((u8 *)dword_80049278 + v5) = 0;
}
return dword_80049278;
```

Record table shape used by `80019284`:

```text
base: 0x80049278
header: 20 bytes
records: 18 * 16 bytes
required bytes for tail-carry-accurate input: 308 bytes
range: 0x80049278..0x800493AB
```

Current IDB initial bytes for that full 308-byte range:

```text
0x80049278 + 0x00..0x0B: 12 zero bytes
0x80049278 + 0x0C..0x13: 06 00 03 00 13 00 00 00
0x80049278 + 0x14..0x133: zero bytes
```

IDA xref evidence:

- Xrefs to `0x80049278`: only `80019284` data use.
- Xrefs to `0x8004928C` (`base+20`, first record): none.
- Xrefs to `0x80049284` (`base+0x0C`, header field): `80019284` only.
- No static xref found to a separate clear/init/template producer for
  `0x80049278..0x800493AB`.

Important adjacent-data clarification:

- `0x800491C4..0x800492C3` is also the static byte table used by `80017FC4`
  as `0x800491C4 + sourceByte`.
- `0x80049278` lies inside that broader static byte region, but `80019284`
  treats `0x80049278` as an output table base and overwrites from `base+20`.
- Existing save-UI helpers around `dword_80049244` (`80018060`, `800180D8`,
  `800185D0`) initialize/use `0x80049244` and `0x8004926C`, but the examined
  pseudocode does not initialize/clear the `0x80049278` high-score table range.

Conclusion: the initial table memory source is the ROM/current-IDB static data
image at `0x80049278..0x800493AB`. No separate initializer/clear/template
producer has been proven. Host code should carry this static initial table
verbatim until a real producer is found.

## 4. Thin Win-side wiring recommendation

Do not pass `PrCard::Payload()` directly to
`PsxCall80019284_BuildHiScoreRecords80015788`: it is only 4876 bytes and stops
exactly before the bank read range.

Minimal faithful carrier:

```text
fullA1Memory[0x130C + 0x180]
  [0x0000..0x130B] = byte_80092F10 save/status payload
  [0x130C..0x148B] = high-score bank 0x8009421C, 6 rows * 4 slots * 16 bytes

initialTableMemory[308]
  = static bytes from 0x80049278..0x800493AB
```

Data sources:

- The `byte_80092F10` prefix must come from the direct save/status carrier
  (`80015CC4/800164B4/8001635C` path), not from a truncated card payload if the
  call is after the case17 refresh.
- The `0x8009421C` suffix must come from the direct high-score bank carrier
  after `800168DC -> 800164F8 -> 80016000` has run for successful card reads.
- The `dword_80049278` initial table must be supplied as the static IDB table
  bytes above, not C++ zero-init, replay-captured output, or ad-hoc formatting.

The existing builder signature already has the right shape:

```c++
PsxCall80019284_BuildHiScoreRecords80015788(
    a1,
    a1Known,
    fullA1Memory.data(),
    fullA1Memory.size(),
    initialTableMemory.data(),
    initialTableMemory.size());
```

This lets `80019284` preserve both tail cases:

- Positive-score path: `80017FC4` writes mapped glyph bytes plus a terminator;
  untouched bytes after the terminator remain from `initialTableMemory`.
- Non-positive-score path: only `record[0] = 0`; `record[1..15]` remain from
  `initialTableMemory`.

## 5. Still unknown / not proven

- No separate runtime initializer for `dword_80049278` was found, but this is a
  negative xref result, not proof that no runtime write can exist through an
  computed pointer.
- Exact ownership/name of the `0x80049278` static table header fields
  `06 00 03 00 13 00 00 00` is not identified beyond "static IDB bytes consumed
  as the initial output table".
- The full high-score bank correctness still depends on faithful card-read
  feedback into `80019D7C case17`; memory replay remains unsuitable as an
  implementation source.
- Upstream name writer constraints for the 12-byte bank name C string are still
  not fully proven here; `800164F8/80016000/80016D7C` only prove
  `strcpy/strcmp` semantics and score-at-`+0x0C` layout.
