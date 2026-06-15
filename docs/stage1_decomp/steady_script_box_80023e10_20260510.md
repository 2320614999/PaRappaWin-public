# 80023E10 steady script box owner

IDA MCP current database: `SCUS_941.83`.

```c
int __fastcall sub_80023E10(unsigned __int8 *a1)
{
  sub_8001C4EC(10, 199, 8u, 0x12u, 0x400F0F0F, 2);
  sub_8001C4EC(18, 191, 0x11Cu, 0x22u, 0x400F0F0F, 2);
  sub_8001C4EC(302, 199, 8u, 0x12u, 0x400F0F0F, 2);
  sub_8001C550(10, 191, (int)dword_80050900, 2u);
  sub_8001C550(10, 217, (int)dword_800508F0, 2u);
  sub_8001C550(302, 191, (int)dword_800508E0, 2u);
  sub_8001C550(302, 217, (int)dword_800508D0, 2u);
  sub_8001B730(28, 194, 0);
  return sub_8001C6A0(a1, 480);
}
```

Helper facts:

- `8001C4EC(x,y,w,h,attr,priority)` tail-calls `8001B6C4(..., &unk_80087288 + 20 * gp872Slot)`.
- `8001C550(x,y,tpl,priority)` tail-calls `8001B590(x,y,tpl,0,0,priority,&unk_80087288 + 20 * gp872Slot)`.
- `8001C6A0(text,scale)` tail-calls `8001B954(text,scale,&unk_80087288 + 20 * gp872Slot)`.

Current Win cutover:

- `801CBFDC` direct runtime emits `PsxScriptBoxCommand80023E10{textHandle10C,maxGlyphs=480}`.
- `PrStageSceneSubmitBackend` consumes `BuildStage1ScriptBoxCommands80023E10(...)` and uses `PrStage1ScriptBoxDirect::BuildScriptBoxFrameSub80023E10(...)` for the rect/corner/origin/text submit intent.
- `PrStage1LiveHud` no longer rebuilds or draws the steady script box from overlay text snapshots.
- Steady script text glyph submit remains a gap until `8001B954 -> 8003FA20` RGB/packet owner is closed.

Carrier state after this pass:

- `ScriptBoxTextSub8001C6A0` now records the direct call chain
  `8001C6A0 -> 8001B954 -> 8003FA20` and the incoming `textHandle10C`.
- The helper evidence proves the work-list base form
  `0x80087288 + 20 * gp872Slot`, but the current `80023E10` carrier does not
  carry the concrete `gp872Slot` value, so `workListSlotKnown=false` and
  `firstGap=MissingWorkListSlotCarrier`.
- The existing `8001B954` stack-local audit still leaves local FastSprite RGB
  and full packet words unresolved, so
  `glyphLocalFastSpriteRgbKnown=false`, `fullFastSpritePacketKnown=false`, and
  `glyphPacketGap=true`.
- No renderer/HUD/pr_scenes fallback is introduced here; the frame/corner
  geometry remains the only drawable output from this carrier until the text
  submit owner is closed.
