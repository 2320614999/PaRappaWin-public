# SaveUi19148 80017A10 write recorder facts

Source recording:

`E:\game\PSgame\parappa the rapper\duckstation\bin\x64\标题画面-menu-选关选stage1-good通关-保存记忆卡（过场动画均为跳过）.psxrec`

Recorder metadata:

- Format: `PSXREC03`, write log enabled, pc trace enabled, call trace enabled.
- Frames: `14318`.
- Record time: `2026-05-13T09:10:13` to `2026-05-13T09:14:12`.

## Call chain facts

The recording covers the SaveUi19148 write path, including the final
`80017A10` save-block write helper.

| Frame | Fact |
|---:|---|
| 12584 | `801C84D4 -> 80019148`, `ra=801C84DC`, `sp=801FFF88` |
| 12715 | `800191B4 -> 80018FB0`, `ra=800191BC`, `sp=801FFF68` |
| 12790..13386 | `80019064 -> 80019458` repeated while SaveUi state advances |
| 13386 | `80019D24 -> 80017A10`, `ra=80019D2C`, `sp=801FFE00` |
| 13386 | `80017A44 -> 80017900`, `ra=80017A4C`, `sp=801FFDC8` |
| 13386 | `80017A68 -> 80017454`, `ra=80017A70`, `sp=801FFDC8` |
| 13394 | `800174EC -> 80016FC0`, `ra=800174F4`, `sp=801FFD60` |
| 13394 | `80017A70 -> 80035560`, `ra=80017A78`, `sp=801FFDC8` |
| 13395 | `80017A78 -> 80016EB8`, `ra=80017A80`, `sp=801FFDC8` |
| 13395..13523 | `80016EB8` polls software card events and calls `80035560` |
| 13524 | `80017A84 -> 80048A90`, `ra=80017A8C`, `sp=801FFDC8` |
| 13524 | `80019078 -> 800180D8`, then `800190E4 -> 8001E750` |
| 13525 | `80019114 -> 80017E6C`, setting the SaveUi event result |

The important boundary is that `80017A10` success is still authorized by the
PSX event-poll path: `80016EB8` reaches the branch that returns to
`80017A84`, and `80019458` commits the result in the same frame. This is not a
Win save-file or `PrCard` authority.

## Write log and state facts

| Frame | Fact |
|---:|---|
| 13386 | save name at `0x8007CBE8` is `BASCUS-94183AA`; save buffer starts at `0x8007ABE8`, first word `0x01134353` |
| 13394 | `800174EC` writes `0x8006ECF8` (`gp+696`) from `0` to `2` |
| 13394 | BIOS/card write path records buffer `0x8007ABE8` and byte count `0x2000` in the write-log stack/control facts |
| 13395 | `80016EB8` begins polling while `gp+696 == 2` |
| 13523 | before success commit: `gp+696=2`, `gp+716=1`, `gp+720=0`, `gp+724=1` |
| 13524 | `80019D3C` writes `0x8006ED10` (`gp+720`) from `0` to `1` |
| 13524 | after commit: `gp+696=2`, `gp+716=1`, `gp+720=1`, `gp+724=1` |

## Direct-port boundary

This recording can be used as evidence that the terminal save path covers a
single successful `80017A10` attempt:

- `scanResult80017900` is reached before `80017454`.
- `80017454` reaches the write-submit path and records `gp+696 == 2`.
- `80016FC0`, `80035560(4)`, and `80016EB8` are in the observed attempt.
- Success is indicated by returning past `80016EB8` to `80017A84` and the
  subsequent `80019458` commit of `gp+720=1`.

This is evidence for a future real `CardWriteHostFacts80017A10` producer.
It does not authorize deriving write success from `PrCard`, a Win save file,
payload bytes, ISO/STR/XA readability, or request metadata.
