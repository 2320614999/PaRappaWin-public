/* Stage1 decomp bundle: Hex-Rays pseudocode export from SCUS_941.83. */
/* No manual cleanup; kept close to current IDA output. */

/* line: 0, address: 0x80014c80 */ int sub_80014C80()
/* line: 1 */ {
/* line: 2 */   int v0; // $v1
/* line: 3 */   int result; // $v0
/* line: 4 */
/* line: 5, address: 0x80014c88 */   v0 = word_80091818;
/* line: 6, address: 0x80014c90 */   word_80091818 = word_80091816;
/* line: 7, address: 0x80014ca0 */   if ( word_80091816 - v0 >= 66 )
/* line: 8 */   {
/* line: 9, address: 0x80014cf0 */     result = 0;
/* line: 10, address: 0x80014cec */     if ( word_80091828 < 2 )
/* line: 11 */     {
/* line: 12, address: 0x80014cf4 */       word_80091826 = 0;
/* line: 13, address: 0x80014cfc */       word_80091828 = 0;
/* line: 14, address: 0x80014d04 */       return result;
/* line: 15 */     }
/* line: 16 */   }
/* line: 17 */   else
/* line: 18 */   {
/* line: 19, address: 0x80014cb8 */     ++word_80091828;
/* line: 20, address: 0x80014cd0 */     result = 1;
/* line: 21, address: 0x80014ccc */     if ( word_80091828 < 2 )
/* line: 22, address: 0x80014ccc */       return result;
/* line: 23 */   }
/* line: 24, address: 0x80014d10 */   word_80091826 = 1;
/* line: 25, address: 0x80014d20 */   return 1;
/* line: 26 */ }
