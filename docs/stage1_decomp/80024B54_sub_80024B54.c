/* Stage1 decomp bundle: Hex-Rays pseudocode export from SCUS_941.83. */
/* No manual cleanup; kept close to current IDA output. */

/* line: 0, address: 0x80024b54 */ int __fastcall sub_80024B54(char a1)
/* line: 1 */ {
/* line: 2 */   int result; // $v0
/* line: 3 */
/* line: 4, address: 0x80024b5c */   result = 1;
/* line: 5 */   if ( (a1 & 0x10) == 0 )
/* line: 6 */   {
/* line: 7, address: 0x80024b68 */     result = 2;
/* line: 8 */     if ( (a1 & 0x20) == 0 )
/* line: 9 */     {
/* line: 10, address: 0x80024b74 */       result = 3;
/* line: 11 */       if ( (a1 & 0x40) == 0 )
/* line: 12 */       {
/* line: 13, address: 0x80024b80 */         result = 4;
/* line: 14 */         if ( (a1 & 0x80) == 0 )
/* line: 15 */         {
/* line: 16, address: 0x80024b8c */           result = 5;
/* line: 17 */           if ( (a1 & 4) == 0 )
/* line: 18 */           {
/* line: 19, address: 0x80024b98 */             result = 5;
/* line: 20 */             if ( (a1 & 1) == 0 )
/* line: 21 */             {
/* line: 22, address: 0x80024ba4 */               result = 7;
/* line: 23 */               if ( (a1 & 8) == 0 )
/* line: 24 */                 return (a1 & 2) != 0 ? 7 : 0;
/* line: 25 */             }
/* line: 26 */           }
/* line: 27 */         }
/* line: 28 */       }
/* line: 29 */     }
/* line: 30 */   }
/* line: 31, address: 0x80024bb8 */   return result;
/* line: 32 */ }
