# 8003FA20 Entry Trace 2026-05-11

Scope: memory replay entry identification only. This file intentionally records caller/ra/sp/frame and does not read or use local RGB bytes.

Recording: `E:\game\PSgame\parappa the rapper\duckstation\stage1二周目_无cool_good评级通关3.psxrec`
Frames scanned: `9176`
Last frame with `8003FA20`: `9174`
Calls captured: `288842`
Full per-call CSV was intentionally not kept because replay output is only an
entry locator, not translation authority.

## Caller Counts

| ra | pc | caller_by_ra | count | first_frame | first_event_seq | first_sp |
|---|---|---|---:|---:|---:|---|
| `0x8001B5DC` | `0x8001B5D4` | `8001B590` | 181888 | 1 | 3430 | `0x801FFE80` |
| `0x8001BC00` | `0x8001BBF8` | `8001B954` | 80416 | 522 | 8949 | `0x801FFE28` |
| `0x8001BF24` | `0x8001BF1C` | `8001BEE4` | 26538 | 164 | 8916 | `0x801FFE20` |

## Boundary

Use these rows only to pick which caller needs IDA/static closure. Do not infer RGB values or packet word2 from replay data.

## S0 StageSelect Dynamic Validation Correction

- Context: DuckStation was positioned in S0 / StageSelect and hit `80020568`.
- Runtime entry chain observed only for entry/state verification:
  `8002081C -> 8001C604 -> 8001B654 -> 8001B4E0 -> 8003FA20`.
- At `8003FA20` entry, the FastSprite input pointer was `0x801FFE30`; the
  RGB bytes consumed by that invocation are therefore
  `0x801FFE30 + 0x14..0x16 == 0x801FFE44..0x801FFE46`.
- A write watchpoint on `0x801FFE44..0x801FFE46` did not fire before the next
  StageSelect submit entered `8003FA20`.
- Correction: the earlier `0x801FFEDC` write is not this FastSprite RGB tail;
  it belongs to a nearby StageSelect stack/local state region and must not be
  treated as RGB owner evidence.
- Boundary: `8001C604/B654/B4E0` remains a submit closure with no proven RGB
  writer. StageSelect RGB remains an old-stack / stack-residue lifetime gap and
  cannot use default color, template color, replay RGB, or `0x801FFEDC` to
  authorize `colorAuthoritative`.
