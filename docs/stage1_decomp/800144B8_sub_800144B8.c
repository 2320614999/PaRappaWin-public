/* line: 0, address: 0x800144b8 */ BOOL __fastcall sub_800144B8(int a1)
/* line: 1 */ {
/* line: 2 */   BOOL v1; // $a1
/* line: 3 */
/* line: 4, address: 0x800144c0 */   if ( *(_WORD *)(a1 + 78) == 3 )
/* line: 5 */   {
/* line: 6, address: 0x800144dc */     if ( word_80091820 > 0 && *(_WORD *)(a1 + 106) )
/* line: 7 */     {
/* line: 8, address: 0x800144f0 */       v1 = 1;
/* line: 9, address: 0x800144ec */       goto LABEL_9;
/* line: 10 */     }
/* line: 11 */   }
/* line: 12, address: 0x800144b8 */   else if ( *(_WORD *)(a1 + 78) )
/* line: 13 */   {
/* line: 14, address: 0x8001451c */     v1 = 0;
/* line: 15, address: 0x8001451c */     goto LABEL_9;
/* line: 16 */   }
/* line: 17, address: 0x8001450c */   v1 = 0;
/* line: 18, address: 0x80014508 */   if ( word_8009181E == 1 )
/* line: 19, address: 0x80014518 */     v1 = *(__int16 *)(a1 + 106) != 0;
/* line: 20 */ LABEL_9:
/* line: 21, address: 0x80014520 */   word_8009181E = 0;
/* line: 22, address: 0x80014528 */   word_80091820 = 0;
/* line: 23, address: 0x80014530 */   return v1;
/* line: 24 */ }
