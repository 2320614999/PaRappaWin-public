# lower-CD live GDB final-ready facts 2026-05-13

Scope: Stage1 lower-CD only. No RGB, Rail/GTE, MenuHelp, or StageClear facts
are used here.

Tool/source: DuckStation GDB server at `127.0.0.1:2345`, live process
`duckstation-qt-x64-ReleaseLTCG.exe`. These are live runtime facts, not
Win file state, ISO/STR/XA state, replay sampled bytes, or request-only
metadata.

## Captured path

- `800375BC` command path was seen first with `a0=25`.
- `800375BC -> 80037070(0,0)` was then captured through `ra=80037690`.
- Inside `80037070`, `80036AF8` was called and captured as an ordered raw pump.
- `800390C8` was hit later, then the final-ready side effect was captured as:
  `800390C8 -> 800364F0(1,0) -> 800372F0(1,0)`.

## Sync pump facts

- `80036AF8` entry: `PC=80036AF8`.
- `80036B34`: initial/stable interrupt byte was `3`.
- `80036D20`: switch source still `3`; `s1=0`.
- `80037058`: `psxReturn=2`.
- Live globals after the pump:
  - `word_80055F78=1`
  - `word_80055F7A=1` during the callback pump
  - `word_80055FA8=0x000D`
  - `byte_800573D4=2`
  - `800882F8[0..7]=02 00 00 00 00 00 00 00`
  - `dword_80088310=0x3C0`
  - `dword_80088314=0xB8` at the sampled point

## Final-ready facts

- `800364F0` entry:
  - `PC=800364F0`
  - `ra=800390F0`
  - `a0=1`
  - `a1=0`
- `800372F0` entry:
  - `PC=800372F0`
  - `ra=80036500`
  - `a0=1`
  - `a1=0`
- `800372F0` before `CheckCallback`:
  - `word_80055F78=1`
  - `word_80055F7A=0`
  - `word_80055FA8=0x000D`
- `800372F0` no-callback path at `800374E4`:
  - `CheckCallback` returned `0`
  - `byte_800573D4=3`
  - `byte_800573D5=1`
  - `byte_800573D6=0`
  - `80088300[0]=0x22`
  - `80088308[0]=0x00`
  - `dword_80088310=0x419`
  - `dword_80088314=1`
- `800372F0` return at `8003758C`:
  - `v0=1`
  - consumed `byte_800573D5`
  - no output buffer copy because `a1=0`

## Code consequence

`ReadyWaitInput800372F0` already carried explicit live status-bank fields.
The builder now accepts those explicit `byte800573D6/byte800573D5` and
`response88308/response88300` facts when no current raw callback pump produced
ready-bank facts. Raw pump facts still win when present, and explicit fields
remain consistency-checked against raw facts.

Verification target:
`test_stage1_lower_cd_facts` covers both captured facts:

- `80037070` callback-pending raw sync pump: interrupt `3`, return `2`,
  `D4=2`, `882F8[0]=2`.
- `800390C8 -> 800364F0(1,0) -> 800372F0(1,0)` final-ready no-callback path:
  `D6=0`, `D5=1`, `a1=0`, return `1`.
