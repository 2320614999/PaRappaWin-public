# Stage1 `8001A3C8` CD command authority window

Scope:

- Target chain:
  `801C7560 -> 8001A3C8/8001A210 -> byte_800493F4 -> 8001A7A4`.
- This note only closes the narrow CD-command authority question for the
  current timecode gap. It does not authorize lower-CD success, STR/XA
  playback success, payload-derived facts, renderer changes, lifecycle
  terminal/no-fail changes, or fallback from final clock bytes.

IDA export:

`docs/stage1_decomp/ida_export_timecode_cd_command_window_8001a3b8_8001a3c8_8001a750_80036678_800375bc_20260515.json`

## Narrow findings

- `8001A3B8` is `GsGetWorkBase`: it only loads `dword_80049428` and returns.
  It is not a CD command producer and does not write `byte_80057119`.
- `8001A3C8` calls `800364D0(1, sp+0x10)`, then calls `800363A4()`.
  On the accepted path it writes `dword_80049428 = 0` and copies
  `sp+0x10..0x12` to `byte_800493F4/F5/F6`.
- `8001A750` is a separate status poll. It calls `800364D0(1, status)` and
  only calls `80036678(1,0)` when `status[0] & 0x20` is clear. That path only
  proves the existing `80036678(1,0) -> 800375BC(1,0,0,1)` side effect; it does
  not prove `byte_80057119 = 0x10`.
- `80036678` is a wrapper around `800375BC`; it forwards its `a1` as the final
  command argument to `800375BC(a1, a2, 0, 1)`.
- In this exported window, the direct writer xrefs to `byte_80057119` are:
  `800375BC:8003775C sb $s3, byte_80057119` and
  `80037CB0:80037CE0 sb $zero, byte_80057119`. `800363A4`,
  `80036AF8`, `80037070`, `800372F0`, and `80037E9C` only read it.

## Boundary

The PSX target frames show `byte_80057119 = 0x10`, but this export does not
identify the `800375BC(0x10, ...)` caller responsible for that value. Therefore
Win must not set `byte_80057119 = 0x10` from:

- `IssueCdCommand` / `lastCdCommand` / command serials;
- `8001A3B8`, because it is read-only for `dword_80049428`;
- `8001A750`, because the closed side effect there is command `1`, not `0x10`;
- STR/XA playback, payload bytes, ISO/file readability, or final
  `byte_800493F4/F5/F6` values.

## Next action

The next evidence window is the real pre-`8001A3C8` writer for
`byte_80057119 = 0x10`: either find the static caller that reaches
`800375BC(0x10, ...)` before the Stage1 scorer poll, or capture a live
writer hit on `0x80057119` with `$pc/$ra` and then return to IDA for that
caller. Until that exists, `tcPollGapA4` must remain a real gap.
