# IDA export: Save/card HAL lower facts functions

source=current IDA database via `decompile_function`
input_file=current SCUS_941.83 IDB session
functions=80017A10,80017900,80017454,80016FC0,80016EB8,80017B60,8001707C,80017008,80017594,80016E18,80047EE4

Note: the first headless for-file batch export hit the 120s client timeout and did not leave the requested txt artifact. This file records the current-session Hex-Rays facts used for the next direct-port unit. Re-run a smaller disassembly/xref batch before implementation if exact instruction windows are needed.

## 80017A10 sub_80017A10

Hex-Rays summary:

```c
int __fastcall sub_80017A10(int a1, int a2, int a3)
{
  int attempts = 4;
  while (1) {
    int scan = sub_80017900(a1);
    sub_80017454(*(gp + 128), *(gp + 124), a1, a2, a3, scan != 1);
    sub_80035560(4);
    int poll = sub_80016EB8();
    close(*(gp + 696));
    --attempts;
    if (poll == 1) break;
    if (attempts <= 0) return -1;
  }
  return 0;
}
```

Direct-port boundary:

- Write lower feedback must be shaped as four attempts.
- Each attempt depends on `80017900` scan, `80017454` submit/write, `80035560(4)`, `80016EB8` poll, and `close(gp+696)`.
- `PrCard::SaveToMemCardEntry` success cannot be split into these facts.

## 80017900 sub_80017900

Hex-Rays summary:

```c
int __fastcall sub_80017900(int a1)
{
  int index = 0;
  char *row = &unk_8007A318;
  do {
    char name[32];
    int n = 0;
    while (n < 21 && row[n]) {
      name[n] = row[n];
      ++n;
    }
    name[n] = 0;
    ++index;
    if (!strcmp(a1, name)) return 1;
    row += 40;
  } while (index < 15);
  return 0;
}
```

Direct-port boundary:

- This is a 15-row directory scan against `unk_8007A318`, not a host filesystem query.
- The result feeds `80017A10` as `scan != 1` into `80017454` arg6.

## 80017454 sub_80017454

Hex-Rays summary:

```c
int __fastcall sub_80017454(int a1, int a2, const char *a3,
                            int a4, int a5, int a6)
{
  char path[64];
  sub_8004800C(path, "bu%1d%1d:%s", a1, a2, a3);
  if (a6 == 1) {
    int checkFd = open(path, (a5 << 16) | 0x200);
    if (checkFd == -1) return -1;
    close(checkFd);
  }
  int fd = open(path, 32770);
  if (fd == -1) return -1;
  *(gp + 696) = fd;
  sub_80016FC0();
  write(fd, a4, a5 << 13);
  return 0;
}
```

Direct-port boundary:

- The PSX path format is `bu%1d%1d:%s`.
- Open-check, open-write, `gp+696`, `80016FC0`, and write submission are separate facts.
- The later poll result comes from `80016EB8`; `80017454` itself returns submit/open status only.

## 80016FC0 sub_80016FC0

Hex-Rays summary:

```c
int sub_80016FC0()
{
  TestEvent(*(gp + 664));
  TestEvent(*(gp + 668));
  TestEvent(*(gp + 672));
  return TestEvent(*(gp + 676));
}
```

Direct-port boundary:

- This clears/tests four software card event handles.
- A coarse host write success cannot prove these four event observations.

## 80016EB8 sub_80016EB8

Hex-Rays summary:

```c
int sub_80016EB8()
{
  for (int i = 300; i > 0; --i) {
    if (TestEvent(*(gp + 664)) == 1) return 1;
    if (TestEvent(*(gp + 668)) == 1) return 2;
    if (TestEvent(*(gp + 672)) == 1) return 3;
    if (TestEvent(*(gp + 676)) == 1) return 4;
    sub_80035560(0);
  }
  return 2;
}
```

Direct-port boundary:

- `80017A10` write success is specifically `80016EB8() == 1`.
- Timeout/default return is `2`.
- This must be fed by typed event facts, not by Win save return.

## 80017B60 sub_80017B60

Hex-Rays summary:

```c
int sub_80017B60()
{
  int attempts = 3;
  char path[64];
  sub_8004800C(path, "bu%1d%1d:", *(gp + 128), *(gp + 124));
  while (1) {
    sub_8001707C();
    format(path);
    int poll = sub_80017008();
    if (poll == 1) return 1;
    --attempts;
    if (poll == 3) break;
    if (attempts <= 0) return result;
  }
  return 3;
}
```

Direct-port boundary:

- Format lower feedback is three attempts of `8001707C -> format -> 80017008`.
- `PrCard` currently has no public PSX-shaped format action or hardware event poll facts.

## 8001707C sub_8001707C

Hex-Rays summary:

```c
int sub_8001707C()
{
  TestEvent(*(gp + 680));
  TestEvent(*(gp + 684));
  TestEvent(*(gp + 688));
  return TestEvent(*(gp + 692));
}
```

Direct-port boundary:

- This drains/tests four hardware card event handles.
- It is not equivalent to host-side memcard image creation.

## 80017008 sub_80017008

Hex-Rays summary:

```c
int sub_80017008()
{
  do {
    if (TestEvent(*(gp + 680)) == 1) return 1;
    if (TestEvent(*(gp + 684)) == 1) return 2;
    if (TestEvent(*(gp + 688)) == 1) return 3;
    if (TestEvent(*(gp + 692)) == 1) return 4;
  } while (true);
}
```

Direct-port boundary:

- Format completion/status must come from hardware event observations.
- Host filesystem success does not provide this fact.

## 80017594 sub_80017594

Hex-Rays summary:

```c
int sub_80017594()
{
  switch (dword_800917E8) {
  case 0:
    card_info(0);
    dword_800917E8 = 1;
    dword_800917EC = 0;
    *(gp + 700) = 300;
    return dword_800917EC;
  case 1: {
    int r = sub_80016E18();
    if (!r) return dword_800917EC;
    if (r == 1) {
      dword_800917F0 = 1;
      dword_800917E8 = (dword_800917F4 == 1) ? 4 : 2;
      return dword_800917EC;
    }
    if (r == 3) {
      dword_800917F0 = 3;
      dword_800917E8 = 4;
      dword_800917F4 = 0;
      return dword_800917EC;
    }
    if (r == 4) {
      dword_800917F0 = 4;
      sub_8001707C();
      sub_80047EE4(0);
      sub_80017008();
      dword_800917E8 = 2;
      dword_800917F4 = 0;
      return dword_800917EC;
    }
    dword_800917F0 = -3;
    dword_800917E8 = 4;
    dword_800917F4 = 0;
    return dword_800917EC;
  }
  case 2:
    sub_80016FC0();
    card_load(0);
    dword_800917E8 = 3;
    *(gp + 700) = 300;
    return dword_800917EC;
  case 3: {
    int r = sub_80016E18();
    if (!r) return dword_800917EC;
    dword_800917E8 = 4;
    dword_800917F4 = 0;
    if (r == 1) {
      dword_800917F4 = 1;
      return dword_800917EC;
    }
    if (r == 3) {
      dword_800917F0 = 3;
      return dword_800917EC;
    }
    if (r == 4) {
      dword_800917F0 = 5;
      return dword_800917EC;
    }
    dword_800917F0 = 2;
    return dword_800917EC;
  }
  case 4:
    dword_800917E8 = 0;
    dword_800917EC = dword_800917F0;
    return dword_800917EC;
  default:
    return dword_800917EC;
  }
}
```

Direct-port boundary:

- `80017594` is a state machine over `800917E8/EC/F0/F4` plus `gp+700`.
- It consumes `80016E18`, `80016FC0`, `8001707C`, `80047EE4`, and `80017008`.
- Current host card APIs may prove coarse actions such as card-info or load were attempted, but not the PSX event/poll/reset facts needed to complete this state machine.

## 80016E18 sub_80016E18

Hex-Rays summary:

```c
int sub_80016E18()
{
  int result = 0;
  if (TestEvent(*(gp + 664)) == 1) result = 1;
  if (TestEvent(*(gp + 668)) == 1) result = 2;
  if (TestEvent(*(gp + 672)) == 1) result = 3;
  if (TestEvent(*(gp + 676)) == 1) result = 4;
  int remaining = *(gp + 700) - 1;
  *(gp + 700) = remaining;
  if (remaining < 0) return 2;
  return result;
}
```

Direct-port boundary:

- `gp+700` is the timeout counter.
- Return `2` can mean timeout; it cannot be inferred from host-side card load failure without the counter/event facts.

## 80047EE4 sub_80047EE4

Hex-Rays summary:

```c
int __fastcall sub_80047EE4(int a1)
{
  new_card();
  return card_write(a1, 63, 0);
}
```

Direct-port boundary:

- `80017594` uses this only in the `80016E18() == 4` path.
- It still needs surrounding hardware event drain/poll facts from `8001707C/80017008`.

## Next implementation unit

Implementing this unit should update `PrStage1SaveCardHalDirect` rather than the lifecycle host adapter:

- Add explicit typed facts/results for `80017900`, `80017454`, `80016FC0`, `80016EB8`, `8001707C`, `80017008`, `80016E18`, and `80047EE4` as needed.
- Keep `BuildSaveUiWriteLowerFeedbackFromProducerInput80017A10(...)`, `BuildSaveUiFormatLowerFeedbackFromHostFacts80017B60(...)`, and `BuildSaveUiCardIoLowerFeedbackFromHostFacts80017594(...)` as pure typed-facts projections.
- Do not derive any required fact from `PrCard`, Win filesystem success, payload bytes, replay sampling, `gp720`, or `call19414Result`.
