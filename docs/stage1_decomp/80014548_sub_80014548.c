/* Stage1 decomp bundle: Hex-Rays pseudocode export from SCUS_941.83. */
/* No manual cleanup; kept close to current IDA output. */

/* line: 0, address: 0x80014548 */ int __fastcall sub_80014548(int a1)
/* line: 1 */ {
/* line: 2 */   int result; // $v0
/* line: 3 */   int v2; // $v1
/* line: 4 */   int v3; // $v1
/* line: 5 */   int v4; // $a1
/* line: 6 */
/* line: 7, address: 0x80014554 */   result = 2;
/* line: 8, address: 0x80014548 */   if ( *(_DWORD *)(a1 + 64) )
/* line: 9 */   {
/* line: 10, address: 0x80014560 */     v2 = word_8009181C;
/* line: 11, address: 0x80014568 */     word_8009181A = word_80091816;
/* line: 12, address: 0x80014570 */     word_8009181C = word_80091816;
/* line: 13, address: 0x80014580 */     v3 = word_80091816 - v2;
/* line: 14, address: 0x80014584 */     v4 = *(__int16 *)(a1 + 78);
/* line: 15, address: 0x8001458c */     if ( v4 == 1 )
/* line: 16 */     {
/* line: 17, address: 0x800145ac */       result = 1;
/* line: 18, address: 0x800145a8 */       if ( *(__int16 *)(*(_DWORD *)(a1 + 64) + 4) < v3 )
/* line: 19, address: 0x800145a8 */         return result;
/* line: 20, address: 0x800145a8 */       goto LABEL_7;
/* line: 21 */     }
/* line: 22, address: 0x800145b8 */     if ( v4 == 2 )
/* line: 23 */     {
/* line: 24, address: 0x800145c4 */       result = 1;
/* line: 25, address: 0x800145c0 */       if ( v3 > 0 )
/* line: 26, address: 0x800145c0 */         return result;
/* line: 27 */ LABEL_7:
/* line: 28, address: 0x800145c8 */       if ( v3 >= 0 )
/* line: 29, address: 0x800145e4 */         return 2 * (word_8009181E != 1);
/* line: 30, address: 0x800145c8 */       return 0;
/* line: 31 */     }
/* line: 32, address: 0x800145f0 */     result = 1;
/* line: 33, address: 0x800145ec */     if ( v3 <= 0 )
/* line: 34 */     {
/* line: 35, address: 0x800145f8 */       result = 2;
/* line: 36, address: 0x800145f4 */       if ( v3 < 0 )
/* line: 37 */       {
/* line: 38, address: 0x80014600 */         word_8009181E = 1;
/* line: 39, address: 0x80014608 */         return 0;
/* line: 40 */       }
/* line: 41 */     }
/* line: 42 */   }
/* line: 43, address: 0x8001460c */   return result;
/* line: 44 */ }
