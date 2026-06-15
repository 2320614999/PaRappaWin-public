# Stage1 retry face VRAM/TIM upload export - 2026-05-26

Purpose: follow up after user confirmed `c146ac64` did not fix the Try Again
YES retry transition frozen-frame face expression. This export narrows whether
the remaining issue is stale dynamic `F_*` TIM pixels in the Stage1 VRAM/atlas
carrier versus a stale runtime follow-up fact.

## Summary

- `8001ADEC` is the dynamic TIM upload helper used by `801CBFDC` for
  `ctx&0x8000` short-list TIM uploads. It always uploads the TIM image through
  `80044D64`; CLUT upload is conditional only on its second argument and the
  TIM CLUT flag.
- `8001AE7C` is the static INT TIM-block upload helper used by `8001A8F0`
  while loading resources. It uploads TIM image through `80044D64`, waits with
  `80044B3C(0)`, then uploads CLUT through `80044D64` and waits again when the
  TIM has a CLUT.
- `80044D64` is a `LoadImage` wrapper that dispatches GPU DMA through
  `800468E0(sub_800462C4, rect, 8, pixels)`.
- `8001AC18` is the INT container load retry wrapper. It calls `8001A8F0`;
  for TIM blocks, `8001A8F0` calls `8001AE7C` once per TIM entry.

These facts support the current remaining suspect: dynamic `F_*` uploads during
failed gameplay overwrite the fixed face/hand VRAM slots used by PA.TMD, and a
Try Again retry runner init must either restore the resource VRAM baseline or
replay the initial frame-0 `801C9094/801CBFDC` `F_*` uploads before the retry
transition preview samples PA.TMD.

## `8001ADEC`

```c
/* line: 0, address: 0x8001adec */ int __fastcall sub_8001ADEC(int a1, int a2)
/* line: 1 */ {
/* line: 2 */   int result; // $v0
/* line: 3 */   _WORD v4[4]; // [sp+10h] [-28h] BYREF
/* line: 4 */   unsigned int v5; // [sp+18h] [-20h] BYREF
/* line: 5 */   __int16 v6; // [sp+1Ch] [-1Ch]
/* line: 6 */   __int16 v7; // [sp+1Eh] [-1Ah]
/* line: 7 */   __int16 v8; // [sp+20h] [-18h]
/* line: 8 */   __int16 v9; // [sp+22h] [-16h]
/* line: 9 */   int v10; // [sp+24h] [-14h]
/* line: 10 */   __int16 v11; // [sp+28h] [-10h]
/* line: 11 */   __int16 v12; // [sp+2Ah] [-Eh]
/* line: 12 */   int v13; // [sp+30h] [-8h]
/* line: 13 */
/* line: 14, address: 0x8001ae00 */   GsGetTimInfo(a1 + 4, &v5);
/* line: 15, address: 0x8001ae20 */   v4[0] = v6;
/* line: 16, address: 0x8001ae24 */   v4[1] = v7;
/* line: 17, address: 0x8001ae28 */   v4[2] = v8;
/* line: 18, address: 0x8001ae30 */   v4[3] = v9;
/* line: 19, address: 0x8001ae2c */   result = sub_80044D64((int)v4, v10);
/* line: 20, address: 0x8001ae34 */   if ( a2 )
/* line: 21 */   {
/* line: 22, address: 0x8001ae48 */     result = (v5 >> 3) & 1;
/* line: 23, address: 0x8001ae4c */     if ( result )
/* line: 24, address: 0x8001ae60 */       return sub_800431E0(v13, v11, v12);
/* line: 25 */   }
/* line: 26, address: 0x8001ae74 */   return result;
/* line: 27 */ }
```

## `8001AE7C`

```c
/* line: 0, address: 0x8001ae7c */ int __fastcall sub_8001AE7C(int a1)
/* line: 1 */ {
/* line: 2 */   int result; // $v0
/* line: 3 */   __int16 v2; // [sp+10h] [-28h] BYREF
/* line: 4 */   __int16 v3; // [sp+12h] [-26h]
/* line: 5 */   __int16 v4; // [sp+14h] [-24h]
/* line: 6 */   __int16 v5; // [sp+16h] [-22h]
/* line: 7 */   unsigned int v6; // [sp+18h] [-20h] BYREF
/* line: 8 */   __int16 v7; // [sp+1Ch] [-1Ch]
/* line: 9 */   __int16 v8; // [sp+1Eh] [-1Ah]
/* line: 10 */   __int16 v9; // [sp+20h] [-18h]
/* line: 11 */   __int16 v10; // [sp+22h] [-16h]
/* line: 12 */   int v11; // [sp+24h] [-14h]
/* line: 13 */   __int16 v12; // [sp+28h] [-10h]
/* line: 14 */   __int16 v13; // [sp+2Ah] [-Eh]
/* line: 15 */   __int16 v14; // [sp+2Ch] [-Ch]
/* line: 16 */   __int16 v15; // [sp+2Eh] [-Ah]
/* line: 17 */   int v16; // [sp+30h] [-8h]
/* line: 18 */
/* line: 19, address: 0x8001ae88 */   GsGetTimInfo(a1 + 4, &v6);
/* line: 20, address: 0x8001aea8 */   v2 = v7;
/* line: 21, address: 0x8001aeac */   v3 = v8;
/* line: 22, address: 0x8001aeb0 */   v4 = v9;
/* line: 23, address: 0x8001aeb8 */   v5 = v10;
/* line: 24, address: 0x8001aeb4 */   sub_80044D64(&v2, v11);
/* line: 25, address: 0x8001aebc */   sub_80044B3C(0);
/* line: 26, address: 0x8001aed0 */   result = (v6 >> 3) & 1;
/* line: 27, address: 0x8001aed4 */   if ( result )
/* line: 28 */   {
/* line: 29, address: 0x8001aef0 */     v2 = v12;
/* line: 30, address: 0x8001aef4 */     v3 = v13;
/* line: 31, address: 0x8001aef8 */     v4 = v14;
/* line: 32, address: 0x8001af00 */     v5 = v15;
/* line: 33, address: 0x8001aefc */     sub_80044D64(&v2, v16);
/* line: 34, address: 0x8001af04 */     return sub_80044B3C(0);
/* line: 35 */   }
/* line: 36, address: 0x8001af14 */   return result;
/* line: 37 */ }
```

## `80044D64`

```c
/* line: 0, address: 0x80044d64 */ int __fastcall sub_80044D64(int a1, int a2)
/* line: 1 */ {
/* line: 2, address: 0x80044d84 */   sub_80044BA8("LoadImage", a1);
/* line: 3, address: 0x80044dc0 */   return sub_800468E0(sub_800462C4, a1, 8, a2);
/* line: 4 */ }
```

## `800431E0`

```c
/* line: 0, address: 0x800431e0 */ int __fastcall sub_800431E0(int a1, int a2, int a3)
/* line: 1 */ {
/* line: 2 */   _WORD v6[4]; // [sp+10h] [-8h] BYREF
/* line: 3 */
/* line: 4, address: 0x80043204 */   v6[2] = 16;
/* line: 5, address: 0x80043210 */   v6[0] = a2;
/* line: 6, address: 0x80043214 */   v6[1] = a3;
/* line: 7, address: 0x8004321c */   v6[3] = 1;
/* line: 8, address: 0x80043218 */   sub_80044D64((int)v6, a1);
/* line: 9, address: 0x80043240 */   return (unsigned __int16)sub_80043EBC(a2, a3);
/* line: 10 */ }
```

## `8001AC18`

```c
/* line: 0, address: 0x8001ac18 */ int __fastcall sub_8001AC18(_DWORD *a1, int a2)
/* line: 1 */ {
/* line: 2 */   int v4; // $s3
/* line: 3 */   int v5; // $s1
/* line: 4 */   int i; // $s0
/* line: 5 */   int v7; // $v1
/* line: 6 */   int v8; // $a0
/* line: 7 */   int v9; // $a1
/* line: 8 */   int v10; // $v1
/* line: 9 */   int result; // $v0
/* line: 10 */   _DWORD v12[6]; // [sp+10h] [-18h] BYREF
/* line: 11 */
/* line: 12, address: 0x8001ac30 */   v4 = 1;
/* line: 13, address: 0x8001ac38 */   v5 = 0;
/* line: 14, address: 0x8001ac40 */   for ( i = 0; i < 4; ++i )
/* line: 15 */   {
/* line: 16, address: 0x8001ac50 */     if ( i > 0 )
/* line: 17, address: 0x8001ac58 */       v4 = 0;
/* line: 18, address: 0x8001ac5c */     sub_80025A34();
/* line: 19, address: 0x8001ac6c */     if ( sub_8001A324(a1) >= 0 )
/* line: 20 */     {
/* line: 21, address: 0x8001ac78 */       v7 = a1[5];
/* line: 22, address: 0x8001ac7c */       v8 = a1[6];
/* line: 23, address: 0x8001ac80 */       v9 = a1[7];
/* line: 24, address: 0x8001ac84 */       v12[0] = a1[4];
/* line: 25, address: 0x8001ac88 */       v12[1] = v7;
/* line: 26, address: 0x8001ac8c */       v12[2] = v8;
/* line: 27, address: 0x8001ac90 */       v12[3] = v9;
/* line: 28, address: 0x8001ac98 */       v10 = a1[9];
/* line: 29, address: 0x8001ac9c */       v12[4] = a1[8];
/* line: 30, address: 0x8001aca0 */       v12[5] = v10;
/* line: 31, address: 0x8001acac */       result = ((int (__fastcall *)(_DWORD *, int, int))sub_8001A8F0)(v12, v4, a2);
/* line: 32, address: 0x8001acb4 */       v5 = result;
/* line: 33, address: 0x8001acb8 */       if ( result == 1 )
/* line: 34, address: 0x8001acb8 */         break;
/* line: 35 */     }
/* line: 36, address: 0x8001accc */     result = v5;
/* line: 37 */   }
/* line: 38, address: 0x8001acf0 */   return result;
/* line: 39 */ }
```

## `8001A8F0` TIM branch excerpt

The full function handles TIM/VAB/MEM/EOF block types. The relevant TIM block
branch is:

```c
/* line: 37, address: 0x8001a97c */       v9 = *v6;
/* line: 38, address: 0x8001a984 */       if ( *v6 == 1 )
/* line: 39, address: 0x8001a984 */         break;
...
/* line: 129, address: 0x8001a990 */     v10 = sub_80025A70(v6[2] << 11);
/* line: 130, address: 0x8001a998 */     v11 = v6;
/* line: 131, address: 0x8001a9a0 */     if ( !v10 )
/* line: 132, address: 0x8001a9a8 */       exit(1);
/* line: 133, address: 0x8001a9c4 */     v12 = v10;
/* line: 134, address: 0x8001a9c0 */     if ( !sub_8001A818(v10, v6[2], a2) )
/* line: 135, address: 0x8001a9c0 */       goto LABEL_31;
/* line: 136, address: 0x8001a9d4 */     v13 = 0;
/* line: 137, address: 0x8001a9d0 */     if ( (int)v6[1] > 0 )
/* line: 138 */     {
/* line: 139, address: 0x8001a9d8 */       v14 = v6 + 4;
/* line: 140, address: 0x8001a9fc */       do
/* line: 141 */       {
/* line: 142, address: 0x8001a9dc */         sub_8001AE7C(v12);
/* line: 143, address: 0x8001a9e4 */         v15 = *v14;
/* line: 144, address: 0x8001a9e8 */         v14 += 5;
/* line: 145, address: 0x8001a9ec */         ++v13;
/* line: 146, address: 0x8001aa00 */         v12 += v15;
/* line: 147 */       }
/* line: 148, address: 0x8001a9fc */       while ( v13 < v11[1] );
/* line: 149 */     }
/* line: 150, address: 0x8001aa04 */     sub_80025AF8();
/* line: 151, address: 0x8001aa0c */     v16 = v11[2];
```

## `800462C4` / `800468E0` upload dispatch excerpt

`80044D64` passes the RECT-like argument and image pointer to this path:

```c
/* line: 54, address: 0x800463d4 */   if ( (MEMORY[0x1F801814] & 0x4000000) != 0 )
/* line: 55 */   {
/* line: 56 */ LABEL_15:
/* line: 57, address: 0x80046410 */     MEMORY[0x1F801814] = 0x4000000;
/* line: 58, address: 0x80046444 */     MEMORY[0x1F801810] = -1610612736;
/* line: 59, address: 0x80046458 */     MEMORY[0x1F801810] = *(_DWORD *)a1;
/* line: 60, address: 0x80046468 */     v16 = v13 - 1;
/* line: 61, address: 0x80046474 */     for ( MEMORY[0x1F801810] = *(_DWORD *)(a1 + 4); v16 != -1; MEMORY[0x1F801810] = v17 )
/* line: 62 */     {
/* line: 63, address: 0x80046480 */       v17 = *a2++;
/* line: 64, address: 0x80046490 */       --v16;
/* line: 65 */     }
/* line: 66, address: 0x800464a0 */     if ( v14 )
/* line: 67 */     {
/* line: 68, address: 0x800464b4 */       MEMORY[0x1F801814] = 67108866;
/* line: 69, address: 0x800464c4 */       MEMORY[0x1F8010A0] = a2;
/* line: 70, address: 0x800464d8 */       MEMORY[0x1F8010A4] = (v14 << 16) | 0x10;
/* line: 71, address: 0x800464e8 */       MEMORY[0x1F8010A8] = 16777729;
/* line: 72 */     }
/* line: 73, address: 0x800464ec */     return 0;
```

```c
/* line: 0, address: 0x800468e0 */ // write access to const memory has been detected, the output may be wrong!
/* line: 1 */ int __fastcall sub_800468E0(void (__fastcall *a1)(int, int), int a2, int a3, int a4)
/* line: 2 */ {
/* line: 3 */   int v7; // $v0
/* line: 4 */
/* line: 5, address: 0x80046904 */   sub_80047144();
/* line: 6, address: 0x8004694c */   v7 = SetIntrMask(0);
/* line: 7, address: 0x80046960 */   dword_8005D73C = 1;
/* line: 8, address: 0x8004696c */   dword_8005D840 = v7;
/* line: 9, address: 0x800469e4 */   while ( (MEMORY[0x1F801814] & 0x4000000) == 0 )
/* line: 10, address: 0x800469d4 */     ;
/* line: 11, address: 0x800469f0 */   a1(a2, a4);
/* line: 12, address: 0x80046a08 */   dword_8005D828 = (int)a1;
/* line: 13, address: 0x80046a0c */   dword_8005D82C = a2;
/* line: 14, address: 0x80046a14 */   dword_8005D830 = a4;
/* line: 15, address: 0x80046a1c */   SetIntrMask(0);
/* line: 16, address: 0x80046bbc */   return 0;
/* line: 17 */ }
```

