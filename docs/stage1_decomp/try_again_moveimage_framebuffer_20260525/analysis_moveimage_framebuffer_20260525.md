# Try Again MoveImage framebuffer analysis 2026-05-25

## Scope

This file accompanies `ida_export_moveimage_framebuffer_20260525.md`. It records that the missing local IDA export has been produced for the MoveImage framebuffer line. Runtime code is unchanged.

## Functions Exported

- `800468E0`
- `80046840`
- `80044E2C`
- `8001B120`
- `8004019C`
- `80040F90`
- `80040CC8`
- `80040370`

## Review Target

Use the export to close whether `8005D7DC/E4/E8/EC` constructs a GPU framebuffer copy packet, which page is the source, which page is the destination, and where Win can carry the previous PSX framebuffer page without re-running Stage1 gameplay submit from overlay.

## First-Pass Findings

- `8001B120(a1)` is confirmed as the page-to-page `MoveImage` caller. It reads the current draw-buffer slot through `8004019C`, builds a `RECT` with `x=0`, `w=320`, `h=240`, and selects `rect.y` as `0` or `240`. Destination is always `x=0`, with `destY=0` or `0xF0`.
- `80044E2C` writes the GPU packet words:
  - `8005D7E4 = source xy`
  - `8005D7E8 = dest xy`
  - `8005D7EC = width/height`
  - dispatch size `20` bytes from packet base `8005D7DC`
- `80046840` is the low-level DMA submit callback. It writes GPU/DMA registers `0x1F801814`, `0x1F8010A0`, `0x1F8010A4`, and `0x1F8010A8`; `0x1F8010A0` receives the packet pointer passed in `a0`.
- `800468E0` is the DMA dispatcher. It either calls the submit callback immediately after waiting for GPU readiness, or queues callback/function arguments and command words through the `80094448/4C/50/54` ring.
- `80040CC8` only resets/clears work-list packet mirrors through `80044FA8`; this export still does not support treating it as a framebuffer clear.

## Win Mismatch Boundary

- Existing Win `PsxCall80044E2C_MoveImage` is still incomplete as a direct-port implementation: it does not carry source rect, destination page, or framebuffer pixels, and effectively preserves only dispatch shape.
- The export now proves the PSX side is a real GPU DMA packet path, not a high-level redraw call. A safe Win patch still needs a direct framebuffer/page carrier; deleting `DrawGameplaySubmitFrozenRuntimeBaseOnly(ctx)` without that carrier remains unsafe because it removes the current temporary source-frame stand-in.

## Patch Status

No runtime patch in this export pass.
