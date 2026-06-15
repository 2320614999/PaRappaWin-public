/* Stage1 decomp bundle: Hex-Rays pseudocode export from SCUS_941.83. */
/* No manual cleanup; kept close to current IDA output. */

/* line: 0, address: 0x80024bf4 */ BOOL __fastcall sub_80024BF4(int a1)
/* line: 1 */ {
/* line: 2 */   BOOL result; // $v0
/* line: 3 */
/* line: 4, address: 0x80024c00 */   if ( dword_8008ED00 == 1 )
/* line: 5 */   {
/* line: 6, address: 0x80024c18 */     result = 0;
/* line: 7, address: 0x80024c14 */     if ( dword_8008ED14 )
/* line: 8 */     {
/* line: 9, address: 0x80024c34 */       result = 1;
/* line: 10, address: 0x80024c30 */       if ( dword_8008ED0C < *(_DWORD *)(a1 + 12) )
/* line: 11 */       {
/* line: 12, address: 0x80024c38 */         dword_8008ED14 = 0;
/* line: 13, address: 0x80024c44 */         return 0;
/* line: 14 */       }
/* line: 15 */     }
/* line: 16 */   }
/* line: 17 */   else
/* line: 18 */   {
/* line: 19, address: 0x80024c58 */     result = 1;
/* line: 20, address: 0x80024c54 */     if ( !g_PrStageEventStreamId )
/* line: 21, address: 0x80024c74 */       return dword_8008ED14 && dword_8008ED00 == 5;
/* line: 22 */   }
/* line: 23, address: 0x80024c7c */   return result;
/* line: 24 */ }
