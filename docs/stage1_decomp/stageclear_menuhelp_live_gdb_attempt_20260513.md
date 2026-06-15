# StageClear/MenuHelp Live GDB Attempt - 2026-05-13

Sources:

- `docs/stage1_decomp/stageclear_menuhelp_psxrec_live_coverage_20260513.md`
- DuckStation GDB MCP connect attempt to `127.0.0.1:2345`
- Local process / TCP listener checks

This is evidence only. No live writer hit was captured in this attempt.

## Attempt

Tried to connect to DuckStation GDB:

```text
host: 127.0.0.1
port: 2345
result: connection refused
```

Local checks:

- no DuckStation process was visible in the current process list
- no TCP listener was visible on local port `2345`

Repeated check in the current turn produced the same result:

```text
Get-Process *DuckStation*: no process
Get-NetTCPConnection -LocalPort 2345: no listener
duck_gdb_connect 127.0.0.1:2345: WinError 10061 connection refused
```

Therefore no `write 0x800916F6` live hit can be claimed from this run.

## Next Live Setup

When DuckStation is running with its GDB server enabled, use this order:

1. `write 0x800916F6`
2. `exec 0x80026B94`
3. `exec 0x80026314`
4. `exec 0x800167A8`
5. `write 0x80092F1D..0x80092F22`

For the primary `write 0x800916F6` hit, record:

- `pc`, `ra`, `sp`, `gp`
- `a0-a3`, `v0-v1`, `t0-t9`, `s0-s7`
- old/new halfword at `0x800916F6`
- `0x800916D0..0x80091710`
- `0x80092F10..0x80092F60`
- `sp-0x80..sp+0x80`

If the writer PC is in an overlay page, also record the overlay identity and a
small disassembly window around the writer PC before returning to IDA.
