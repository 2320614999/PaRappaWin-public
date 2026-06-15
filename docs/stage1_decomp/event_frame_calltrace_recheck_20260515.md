# Event-Frame Call-Trace Recheck - 2026-05-15

Source recording:

- `E:\game\PSgame\parappa the rapper\duckstation\bin\x64\标题画面-menu-选关选stage1-good通关-保存记忆卡（过场动画均为跳过）.psxrec`

Tool:

- scripted scan over `tools/模拟器内存回放器.py`
- `PSXRecording.get_call_trace(frame)` for every frame `0..14317`

This is dynamic coverage evidence only. It does not authorize deriving RGB,
payload, success, handled state, or default values from sampled RAM.

## Full-Recording Target Counts

```text
frames=14318 has_wlog=True has_vram=True

0x800203D4 event4_prompt_800203D4: count=0 first=None
0x80020568 event2_stage_select_80020568: count=118 first=620
  caller 0x8001E8BC count=118

0x8001C550 event4_submit_8001C550: count=238264 first=2
  caller 0x8001F1D0 count=124032
  caller 0x80024A28 count=42493
  caller 0x8001FE44 count=16800
  caller 0x8002499C count=12140
  caller 0x80023EB0 count=4425
  caller 0x80023E98 count=4423
  caller 0x80023EC8 count=4423
  caller 0x80023EE0 count=4423

0x8001C604 event2_submit_8001C604: count=708 first=620
  caller 0x8002081C count=708

0x8001B590 builder_8001B590: count=501861 first=2
  caller 0x8001C590 count=238259
  caller 0x801CC85C count=30989
  caller 0x8001D8F8 count=23376
  caller 0x801CC7EC count=22136
  caller 0x801CC7A4 count=22135
  caller 0x8001E0A4 count=9537
  caller 0x8001C5EC count=8157
  caller 0x8001D960 count=6818

0x8001B654 builder_8001B654: count=708 first=620
  caller 0x8001C650 count=708

0x8001B25C builder_8001B25C: count=501859 first=2
  caller 0x8001B5C4 count=501859

0x8001B4E0 builder_8001B4E0: count=708 first=620
  caller 0x8001B694 count=708

0x8003FA20 fast_sprite_consumer_8003FA20: count=653358 first=2
  caller 0x8001B5D4 count=501858
  caller 0x8001BBF8 count=98163
  caller 0x8001BF1C count=26546
  caller 0x8001B634 count=24681
  caller 0x8001B904 count=1402
  caller 0x8001B6A4 count=708

0x80026314 menuhelp_80026314: count=0 first=None
0x80026B94 stageclear_80026B94: count=2 first=454
  caller 0x80015808 count=1
  caller 0x8001591C count=1
0x800167A8 status_leaf_800167A8: count=0 first=None
```

## Decision

- This recheck does not create a new event-frame RGB owner window.
- Event2 StageSelect still resolves to the already-known frame `620` chain:
  `8001E8BC -> 80020568 -> 8002081C -> 8001C604 -> 8001B654 ->
  8001B4E0 -> 8003FA20`.
- Event4 prompt entry `800203D4` is still not covered by this recording.
- `8001C550/8001B590/8001B25C/8003FA20` are high-traffic shared submit
  helpers/consumers in this recording. Re-exporting them without a new owner
  address would repeat prior work.
- `80026B94` is covered at frames `454` and `620`, but this is only a
  StageClear text consumer hit. It is not a `word_800916F6` writer fact and
  does not reopen the retired `800916F6` evidence target.

Practical follow-up: temporarily avoid the event2/event4 RGB owner loop until
there is a new recording/live hit for `800203D4` or a new static owner address.
The next productive direct-port window remains the Scene Submit / TOD COORD /
TMD primitive dispatcher coupling graph.
