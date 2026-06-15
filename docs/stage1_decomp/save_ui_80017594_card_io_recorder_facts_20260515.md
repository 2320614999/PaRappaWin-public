# SaveUi 80017594 card-I/O recorder facts - 2026-05-15

Source recording:

`E:\game\PSgame\parappa the rapper\duckstation\bin\x64\标题画面-menu-选关选stage1-good通关-保存记忆卡（过场动画均为跳过）.psxrec`

Format facts:

```text
PSXREC03
frames=14318
write_log_enabled=True
pc_trace_enabled=True
call_trace_enabled=True
record_time=2026-05-13T09:10:13 -> 2026-05-13T09:14:12
```

## Call Coverage

Full-frame `call_trace` scan for the SaveUi/card-I/O window:

| target | count | first frame / callsite | last frame / callsite |
|---|---:|---|---|
| `80019148` | 1 | `frame 12584`, `801C84D4 -> 80019148` | same |
| `80018FB0` | 1 | `frame 12715`, `800191B4 -> 80018FB0` | same |
| `80017594` | 609 | `frame 12715`, `8001904C -> 80017594` | `frame 13386`, `8001904C -> 80017594` |
| `80016E18` | 338 | `frame 12716`, `80017604 -> 80016E18` | `frame 13344`, `80017604 -> 80016E18` |
| `80016FC0` | 2 | `frame 12717`, `800176FC -> 80016FC0` | `frame 13394`, `800174EC -> 80016FC0` |
| `80035510` | 609 | `frame 12715`, `8001900C -> 80035510` | `frame 13345`, `8001900C -> 80035510` |
| `800185D0` | 21 | `frame 12789`, `80019040 -> 800185D0` | `frame 13345`, `80019040 -> 800185D0` |
| `80019458` | 135 | `frame 12790`, `80019064 -> 80019458` | `frame 13386`, `80019064 -> 80019458` |
| `8001707C` | 0 | not covered | not covered |
| `80017008` | 0 | not covered | not covered |
| `80047EE4` | 0 | not covered | not covered |

The observed save path covers the normal card-info/card-load success loop. It
does not cover the `80016E18() == 4` new-card/reset branch, so
`8001707C/80047EE4/80017008` remain explicit gaps for that branch.

## State And Counter Addresses

The recorder confirms the `80017594` timeout counter is written at
`0x8006ECFC`, not at the earlier guessed `0x8009185C`.

| PSX field | address | writer evidence |
|---|---:|---|
| `dword_800917E8` | `800917E8` | `800175EC`, `80017674`, `80017684`, `80017718`, `80017748`, `800177E0` |
| `dword_800917EC` | `800917EC` | `800175F8`, `800177E8` |
| `dword_800917F0` | `800917F0` | `80017664` |
| `dword_800917F4` | `800917F4` | `80017794` |
| `gp+700` timeout | `8006ECFC` | `800175FC`, `80017720`, `80016E90` |
| SwCARD event handles | `8006ECD8..8006ECE4` | `80017120/13C/158/174` |
| HwCARD event handles | `8006ECE8..8006ECF4` | `80017190/1AC/1C8/1D4` |
| `gp+696` fd slot | `8006ECF8` | `800174EC` in later read/close path |

## First Normal Success Cycle

| frame | calls | state snapshot after frame | write facts |
|---:|---|---|---|
| 12584 | `801C84D4 -> 80019148` | `E8=0 EC=0 F0=0 F4=0 gp700=0` | SaveUi19148 entered from COMOD1 clear-tail path. |
| 12715 | `800191B4 -> 80018FB0`, `8001904C -> 80017594` | `E8=1 EC=0 F0=0 F4=0 gp700=300` | `800175EC: E8 0->1`; `800175FC: gp700 0->300`; event handles initialized by `80017120..800171D4`. |
| 12716 | `8001904C -> 80017594`, `80017604 -> 80016E18` | `E8=2 EC=0 F0=1 F4=0 gp700=299` | `80016E90: gp700 300->299`; `80017664: F0 0->1`; `80017684: E8 1->2`. This is `80016E18() == 1`. |
| 12717 | `8001904C -> 80017594`, `800176FC -> 80016FC0` | `E8=3 EC=0 F0=1 F4=0 gp700=300` | `80017718: E8 2->3`; `80017720: gp700 299->300`. This is the `card_load(0)` submit path after clearing SwCARD events. |
| 12718 | `80017728 -> 80016E18` | `E8=3 EC=0 F0=1 F4=0 gp700=299` | `80016E90: gp700 300->299`; no state transition, so poll result is `0`. |
| 12719 | `80017728 -> 80016E18` | `E8=3 EC=0 F0=1 F4=0 gp700=298` | `80016E90: gp700 299->298`; no state transition, so poll result is `0`. |
| 12720 | `80017728 -> 80016E18` | `E8=3 EC=0 F0=1 F4=0 gp700=297` | `80016E90: gp700 298->297`; no state transition, so poll result is `0`. |
| 12789 | `80017728 -> 80016E18` | `E8=4 EC=0 F0=1 F4=1 gp700=228` | `80016E90: gp700 229->228`; `80017748: E8 3->4`; `80017794: F4 0->1`. This is card-load poll success `80016E18() == 1`. |
| 12790 | `8001904C -> 80017594`, `80019064 -> 80019458` | `E8=0 EC=1 F0=1 F4=1 gp700=228` | `800177E0: E8 4->0`; `800177E8: EC 0->1`. This is the first nonzero `80017594` return consumed by `80019458`. |

## Later Repeated Success Cycle

After the first load succeeds, the SaveUi loop repeatedly resets state `0 -> 1`
and then resolves via a shorter poll path:

| frame | state snapshot after frame | write facts |
|---:|---|---|
| 12791 | `E8=1 EC=0 F0=1 F4=1 gp700=300` | `800175EC: E8 0->1`; `800175F8: EC 1->0`; `800175FC: gp700 228->300`. |
| 12792 | `E8=4 EC=0 F0=1 F4=1 gp700=299` | `80016E90: gp700 300->299`; `80017674: E8 1->4`. |
| 12793 | `E8=0 EC=1 F0=1 F4=1 gp700=299` | `800177E0: E8 4->0`; `800177E8: EC 0->1`; `80019458` consumes the nonzero result. |
| 13344 | `E8=4 EC=0 F0=1 F4=1 gp700=298` | Late repeated `80016E18` poll with `80017674: E8 1->4`. |
| 13386 | `E8=0 EC=1 F0=1 F4=1 gp700=298` | Late repeated final return: `800177E0: E8 4->0`; `800177E8: EC 0->1`; `80019458` consumes it. |

## Direct-Port Boundary

These facts can seed/validate a real `CardIoHostFacts80017594` producer for the
observed card-info/card-load success path:

- `cardInfoKnown=true`, `cardInfoArg=0` for the state `0` path.
- `pollSwKnown80016E18=true` with:
  - state `1`: first observed `pollSwResult80016E18=1`,
    `gp700 300->299`;
  - state `3`: repeated `pollSwResult80016E18=0`,
    `gp700` decrements each call;
  - state `3` success: `pollSwResult80016E18=1`,
    `gp700 229->228`, then `F4=1`;
  - later state `1` shortcut: `pollSwResult80016E18=1`,
    `gp700 300->299`, then `E8=4`.
- `clearSwEventsKnown80016FC0=true` for the `E8=2 -> E8=3` load submit path.
- `cardLoadKnown=true`, `cardLoadArg=0` for the same state `2` path.
- `80035510` is called from the `80018FB0` tick path on the same frames as
  `80017594`; `800185D0` first appears on frame `12789`, immediately before
  the first `80019458` consumer frame. These are call-order facts only; they do
  not authorize any input or card-I/O success by themselves.

These facts do not authorize success from `PrCard`, Win save files, host I/O
success, payload bytes, ISO/STR/XA readability, renderer state, or default
values. They are PSX recorder facts for the observed path. The unobserved
new-card/reset branch still requires explicit `8001707C/80047EE4/80017008`
facts before it can be closed.
