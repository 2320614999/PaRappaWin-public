# 800179B4 -> 800173A8/80016EB8 card read seam for 80019D7C case17

Scope: static IDA pseudocode / assembly semantics only. Dynamic replay can identify entry/verification points, but is not an implementation source for this seam.

## 1. IDA semantics

### `800179B4 sub_800179B4(name, blockBuffer, blocks)`

Hex-Rays:

```c
int __fastcall sub_800179B4(int name, int blockBuffer, int blocks)
{
  sub_800173A8(*(int *)(gp + 128), *(int *)(gp + 124),
               name, blockBuffer, blocks);
  int ev = sub_80016EB8();
  close(*(int *)(gp + 696));
  return -(ev != 1);
}
```

Necessary assembly details:

```asm
800179B4  move  v0,a0          ; save name
800179B8  move  a3,a1          ; a3 = blockBuffer
800179BC  lw    a0,0x80(gp)    ; port-ish card selector
800179C0  lw    a1,0x7C(gp)    ; slot/channel-ish card selector
800179C8  sw    a2,0x10(sp)    ; 5th arg = blocks
800179CC  move  a2,v0          ; a2 = name
800179D4  jal   sub_800173A8
800179DC  jal   sub_80016EB8   ; poll card events
800179E4  lw    a0,0x2B8(gp)   ; fd saved by 800173A8
800179E8  jal   close
800179EC  move  s0,v0          ; delay slot: preserve 80016EB8 result
800179F0  xori  s0,1
800179F4  sltu  s0,zero,s0
800179F8  negu  v0,s0          ; 0 if event==1, -1 otherwise
```

Return contract: `0` means `80016EB8()` returned event code `1`; `-1` means any other event/timeout result. `800179B4` ignores `800173A8`'s immediate `open/read` return and gates success only through the later event poll.

### `800173A8 sub_800173A8(port, slot, name, blockBuffer, blocks)`

Hex-Rays:

```c
int __fastcall sub_800173A8(int port, int slot,
                            const char *name, int blockBuffer, int blocks)
{
  char path[64];
  sprintf(path, "bu%1d%1d:%s", port, slot, name);
  int fd = open(path, 0x8001);
  if (fd == -1) {
    close(-1);
    return -1;
  }

  *(int *)(gp + 696) = fd;
  sub_80016FC0();               // drain/clear software card events
  read(fd, blockBuffer, blocks << 13);
  return 0;
}
```

Necessary assembly details:

```asm
800173E0  jal   sub_8004800C    ; sprintf(path,"bu%1d%1d:%s",port,slot,name)
800173EC  li    a1,0x8001
800173F0  jal   open
800173F4  sll   s1,s0,13        ; bytes = blocks * 8192
80017400  beq   s0,-1,fail
80017408  sw    s0,0x2B8(gp)    ; gp+696 = fd
8001740C  jal   sub_80016FC0    ; clear pending events before read
80017414  move  a0,s0           ; fd
80017418  move  a1,s2           ; blockBuffer
8001741C  jal   read
80017420  move  a2,s1           ; bytes
80017428  move  v0,zero         ; immediate return 0
fail:
8001742C  jal   close           ; close(-1), as original code does
80017430  li    a0,-1
80017434  li    v0,-1
```

### `80016EB8 sub_80016EB8()`

Hex-Rays:

```c
int sub_80016EB8(void)
{
  for (int i = 300; i > 0; --i) {
    if (TestEvent(*(int *)(gp + 664)) == 1) return 1;
    if (TestEvent(*(int *)(gp + 668)) == 1) return 2;
    if (TestEvent(*(int *)(gp + 672)) == 1) return 3;
    if (TestEvent(*(int *)(gp + 676)) == 1) return 4;
    sub_80035560(0);
  }
  return 2;
}
```

Necessary assembly facts:

```asm
80016EC0  li    s1,300
80016ED0  lw    a0,0x298(gp) ; event0 -> return 1
80016EE4  lw    a0,0x29C(gp) ; event1 -> return 2
80016EF8  lw    a0,0x2A0(gp) ; event2 -> return 3
80016F0C  lw    a0,0x2A4(gp) ; event3 -> return 4
80016F20  jal   sub_80035560 ; wait/yield between polls
80016F28  addiu s1,-1
80016F30  li    v0,2         ; timeout fallback
```

`80016FC0` is the companion clear/drain helper: it calls `TestEvent` once on the same four event handles at `gp+664/668/672/676` and returns the last call's result. `800173A8` invokes it before issuing `read`.

## 2. Parameter / return relationships

- `name`: `800179B4.a0`, forwarded to `800173A8.a2`, used as `%s` in `"bu%1d%1d:%s"`. In `case17`, it is copied from the selected `unk_8007A590` row into `byte_8007CBE8` before the read.
- `blockBuffer`: `800179B4.a1`, forwarded to `800173A8.a3`, passed to `read(fd, blockBuffer, blocks << 13)`. In `case17`, this is `byte_8007ABE8`, the full 8192-byte card block buffer.
- `blocks`: `800179B4.a2`, stored as the 5th arg to `800173A8`, converted to bytes by `blocks << 13`. In `case17`, `blocks == 1`.
- `gp+128` / `gp+124`: loaded by `800179B4` as the first two args to `800173A8`, then formatted into `bu%1d%1d:`. They are card port/slot-like selectors for the BIOS path.
- `gp+696`: written by `800173A8` with the opened file descriptor and later read by `800179B4` for `close(fd)`.
- `dword_8007AE14`: located inside the read buffer: `byte_8007ABE8 + 0x22C`, equivalently `byte_8007ADE8 + 0x2C`. `case17` copies it into the active row's halfword field at `row + 0x6A` after a successful read. This is row metadata taken from the loaded save block, not a success flag.

## 3. `80019D7C case17` high-score refresh flow

Static flow from `8001A0C4..8001A198`:

```c
case 17:
  if (a2 == 3) {
    return 5;
  }

  sub_800168DC();               // clear high-score bank carrier
  if (word_8007ABE4) {
    *(int *)(gp + 736) = 0;      // gp+0x2E0
    *(int *)(gp + 740) = 0;      // gp+0x2E4
    VSyncCallback(sub_80017F38);

    row = &unk_8007A590;
    for (i = 0; i < 15; ++i, row += 0x6C) {
      sub_80025C44(byte_8007ABE8, 0x2000);
      if (*(int16_t *)(row + 0x68) != 0) {
        strcpy(byte_8007CBE8, row);
        if (sub_800179B4(byte_8007CBE8, byte_8007ABE8, 1) >= 0) {
          *(uint16_t *)(row + 0x6A) = dword_8007AE14;
          sub_800164F8(byte_8007ADE8);
        }
      }
    }

    VSyncCallback(0);
  }

  *(int *)(gp + 720) = 1;        // gp+0x2D0
  return 23;
```

Key assembly anchors:

```asm
8001A118  li    a0,byte_8007ABE8
8001A120  jal   sub_80025C44
8001A124  li    a1,0x2000       ; clear full block buffer
8001A128  lh    v0,0x68(s0)     ; row valid flag
8001A138  li    a0,byte_8007CBE8
8001A140  jal   strcpy
8001A144  move  a1,s0           ; copy full row name into byte_8007CBE8
8001A148  li    a0,byte_8007CBE8
8001A150  li    a1,byte_8007ABE8
8001A158  jal   sub_800179B4
8001A15C  li    a2,1
8001A160  bltz  v0,skip_merge
8001A164  move  a0,s2           ; delay slot: s2 = byte_8007ADE8
8001A168  lw    v0,(dword_8007AE14-byte_8007ADE8)(s2)
8001A16C  jal   sub_800164F8
8001A170  sh    v0,0x6A(s0)     ; delay slot: row metadata from loaded save
```

Why `byte_8007ADE8` is passed to `800164F8`: `byte_8007ABE8` is the start of the full card block. The first `0x200` bytes are the card header/icon area. `byte_8007ADE8 == byte_8007ABE8 + 0x200` is the actual save payload. `800164F8(payload)` reads suffix/name at `payload + 1` and six stage scores at `payload + 0x14 + 4*i`, merges them into the high-score bank at `0x8009421C/0x80094228`, and calls `80016000(rowBase, 4)` to sort each row when a non-duplicate candidate is inserted.

## 4. Thin host feedback boundary

Acceptable thin host feedback:

- For `800179B4`, expose only explicit card-read feedback matching the PSX poll result: name, target block buffer identity, block count, event result (`80016EB8` code), and optionally fd/open/read action trace. Success is `event == 1 -> return 0`; otherwise return `-1`.
- For `case17`, direct carrier may describe the deterministic PSX-side dataflow after read success: `row.name -> byte_8007CBE8`, `read byte_8007ABE8`, payload view `byte_8007ADE8`, `dword_8007AE14 -> row+0x6A`, and `800164F8(payload)` merge into the high-score bank.
- `800168DC` and `800164F8` can remain direct carriers because they are memory transforms over PSX global state and save payload bytes.

Must remain card HAL side-effect:

- BIOS path construction/open/read/close: `"bu%1d%1d:%s"`, `open`, `read`, `close`.
- `gp+124/gp+128` card selector semantics beyond forwarding into the BIOS path.
- Event drain/poll behavior: `80016FC0`, `80016EB8`, the four event handles at `gp+664..676`, timeout/yield behavior, and exact return code source.
- Any claim that a row read succeeded without explicit `800179B4/80016EB8` feedback.

Implementation caution: do not fill this seam from `PrCard::Payload()` or replay-sampled save bytes. If Win needs to model `case17`, host input should be a narrow, explicit card-read feedback record per row; the payload may then be carried directly only after that feedback says the PSX read succeeded.
