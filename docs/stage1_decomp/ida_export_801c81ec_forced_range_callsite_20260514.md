# IDA export: forced/range 801C81EC lifecycle callsite (2026-05-14)

Generated: 2026-05-14T11:43:37
Range: `801C81EC-801C858C`
Function before: `None`; add_func attempted result: `True`; err: ``; after: `(2149351916, 2149352844)`

## Pseudocode
```c
void sub_801C81EC()
{
  __asm
  {
    sd      $ra, -0x1204($ra)
    sd      $ra, -0x13A9($ra)
  }
  JUMPOUT(0x801C81FC);
}

```

## Key call refs inside range
- <none>

## Status/data refs inside range
- <none>

## Key call contexts
## Full range disassembly
```asm
801C81EC: nop
801C81F0: nop
801C81F4: sd      $ra, -0x1204($ra)
801C81F8: sd      $ra, -0x13A9($ra)
801C8200: nop
801C8204: sd      $ra, -0x578($ra)
801C8208: nop
801C820C: nop
801C8210: nop
801C8214: sd      $ra, -0x1209($ra)
801C8218: sd      $ra, -0x1394($ra)
801C821C: mtlo    $zero
801C8220: nop
801C8224: sd      $ra, -0x578($ra)
801C8228: nop
801C822C: nop
801C8230: nop
801C8234: sd      $ra, -0x1203($ra)
801C8238: sd      $ra, -0x137F($ra)
801C823C: sd      $ra, -0x5C($ra)
801C8240: nop
801C8244: sd      $ra, -0x578($ra)
801C8248: nop
801C824C: nop
801C8250: nop
801C8254: sd      $ra, -0x11F2($ra)
801C8258: sd      $ra, -0x1369($ra)
801C825C: sd      $ra, -0x195($ra)
801C8260: nop
801C8264: sd      $ra, -0x578($ra)
801C8268: nop
801C826C: nop
801C8270: nop
801C8274: sd      $ra, -0x11D5($ra)
801C8278: sd      $ra, -0x1353($ra)
801C827C: sd      $ra, -0x2D6($ra)
801C8280: nop
801C8284: sd      $ra, -0x578($ra)
801C8288: nop
801C828C: nop
801C8290: nop
801C8294: sd      $ra, -0x11AF($ra)
801C8298: sd      $ra, -0x133D($ra)
801C829C: sd      $ra, -0x417($ra)
801C82A0: nop
801C82A4: sd      $ra, -0x578($ra)
801C82A8: nop
801C82AC: nop
801C82B0: nop
801C82B4: sd      $ra, -0x1182($ra)
801C82B8: sd      $ra, -0x1327($ra)
801C82BC: sd      $ra, -0x54B($ra)
801C82C0: nop
801C82C4: sd      $ra, -0x578($ra)
801C82C8: nop
801C82CC: nop
801C82D0: nop
801C82D4: sd      $ra, -0x1150($ra)
801C82D8: sd      $ra, -0x1312($ra)
801C82DC: sd      $ra, -0x673($ra)
801C82E0: nop
801C82E4: sd      $ra, -0x578($ra)
801C82E8: nop
801C82EC: nop
801C82F0: nop
801C82F4: sd      $ra, -0x1117($ra)
801C82F8: sd      $ra, -0x12FD($ra)
801C82FC: sd      $ra, -0x78F($ra)
801C8300: nop
801C8304: sd      $ra, -0x578($ra)
801C8308: nop
801C830C: nop
801C8310: nop
801C8314: sd      $ra, -0x10D7($ra)
801C8318: sd      $ra, -0x12E9($ra)
801C831C: sd      $ra, -0x8A0($ra)
801C8320: nop
801C8324: sd      $ra, -0x578($ra)
801C8328: nop
801C832C: nop
801C8330: nop
801C8334: sd      $ra, -0x1092($ra)
801C8338: sd      $ra, -0x12D5($ra)
801C833C: sd      $ra, -0x9A6($ra)
801C8340: nop
801C8344: sd      $ra, -0x578($ra)
801C8348: nop
801C834C: nop
801C8350: nop
801C8354: sd      $ra, -0x1046($ra)
801C8358: sd      $ra, -0x12C1($ra)
801C835C: sd      $ra, -0xAA1($ra)
801C8360: nop
801C8364: sd      $ra, -0x578($ra)
801C8368: nop
801C836C: nop
801C8370: nop
801C8374: sd      $ra, -0xFF5($ra)
801C8378: sd      $ra, -0x12AE($ra)
801C837C: sd      $ra, -0xB90($ra)
801C8380: nop
801C8384: sd      $ra, -0x578($ra)
801C8388: nop
801C838C: nop
801C8390: nop
801C8394: sd      $ra, -0xF9D($ra)
801C8398: sd      $ra, -0x129C($ra)
801C839C: sd      $ra, -0xC75($ra)
801C83A0: nop
801C83A4: sd      $ra, -0x578($ra)
801C83A8: nop
801C83AC: nop
801C83B0: nop
801C83B4: sd      $ra, -0xF40($ra)
801C83B8: sd      $ra, -0x1289($ra)
801C83BC: sd      $ra, -0xD4F($ra)
801C83C0: nop
801C83C4: sd      $ra, -0x578($ra)
801C83C8: nop
801C83CC: nop
801C83D0: nop
801C83D4: sd      $ra, -0xEDD($ra)
801C83D8: sd      $ra, -0x1277($ra)
801C83DC: sd      $ra, -0xE1F($ra)
801C83E0: nop
801C83E4: sd      $ra, -0x578($ra)
801C83E8: nop
801C83EC: nop
801C83F0: nop
801C83F4: sd      $ra, -0xE75($ra)
801C83F8: sd      $ra, -0x1266($ra)
801C83FC: sd      $ra, -0xEE4($ra)
801C8400: nop
801C8404: sd      $ra, -0x578($ra)
801C8408: nop
801C840C: nop
801C8410: nop
801C8414: sd      $ra, -0xE07($ra)
801C8418: sd      $ra, -0x1254($ra)
801C841C: sd      $ra, -0xF9F($ra)
801C8420: nop
801C8424: sd      $ra, -0x578($ra)
801C8428: nop
801C842C: nop
801C8430: nop
801C8434: sd      $ra, -0xD93($ra)
801C8438: sd      $ra, -0x1244($ra)
801C843C: sd      $ra, -0x104F($ra)
801C8440: nop
801C8444: sd      $ra, -0x578($ra)
801C8448: nop
801C844C: nop
801C8450: nop
801C8454: sd      $ra, -0xD1A($ra)
801C8458: sd      $ra, -0x1233($ra)
801C845C: sd      $ra, -0x10F6($ra)
801C8460: nop
801C8464: sd      $ra, -0x578($ra)
801C8468: nop
801C846C: nop
801C8470: nop
801C8474: sd      $ra, -0xC9C($ra)
801C8478: sd      $ra, -0x1223($ra)
801C847C: sd      $ra, -0x1193($ra)
801C8480: nop
801C8484: sd      $ra, -0x578($ra)
801C8488: nop
801C848C: nop
801C8490: nop
801C8494: sd      $ra, -0xC18($ra)
801C8498: sd      $ra, -0x1213($ra)
801C849C: sd      $ra, -0x1226($ra)
801C84A0: nop
801C84A4: sd      $ra, -0x578($ra)
801C84A8: nop
801C84AC: nop
801C84B0: nop
801C84B4: sd      $ra, -0xB8F($ra)
801C84B8: sd      $ra, -0x1204($ra)
801C84BC: sd      $ra, -0x12AF($ra)
801C84C0: nop
801C84C4: sd      $ra, -0x578($ra)
801C84C8: nop
801C84CC: nop
801C84D0: nop
801C84D4: sd      $ra, -0xB01($ra)
801C84D8: sd      $ra, -0x11F5($ra)
801C84DC: sd      $ra, -0x132F($ra)
801C84E0: nop
801C84E4: sd      $ra, -0x578($ra)
801C84E8: nop
801C84EC: nop
801C84F0: nop
801C84F4: sd      $ra, -0xA6E($ra)
801C84F8: sd      $ra, -0x11E6($ra)
801C84FC: sd      $ra, -0x13A6($ra)
801C8500: nop
801C8504: sd      $ra, -0x578($ra)
801C8508: nop
801C850C: nop
801C8510: nop
801C8514: sd      $ra, -0x9D6($ra)
801C8518: sd      $ra, -0x11D8($ra)
801C851C: sd      $ra, -0x1414($ra)
801C8520: nop
801C8524: sd      $ra, -0x578($ra)
801C8528: nop
801C852C: nop
801C8530: nop
801C8534: sd      $ra, -0x939($ra)
801C8538: sd      $ra, -0x11CA($ra)
801C853C: sd      $ra, -0x1478($ra)
801C8540: nop
801C8544: sd      $ra, -0x578($ra)
801C8548: nop
801C854C: nop
801C8550: nop
801C8554: sd      $ra, -0x897($ra)
801C8558: sd      $ra, -0x11BC($ra)
801C855C: sd      $ra, -0x14D3($ra)
801C8560: nop
801C8564: sd      $ra, -0x578($ra)
801C8568: nop
801C856C: nop
801C8570: nop
801C8574: sd      $ra, -0x7F0($ra)
801C8578: sd      $ra, -0x11AF($ra)
801C857C: sd      $ra, -0x1526($ra)
801C8580: nop
801C8584: sd      $ra, -0x578($ra)
801C8588: nop
```
