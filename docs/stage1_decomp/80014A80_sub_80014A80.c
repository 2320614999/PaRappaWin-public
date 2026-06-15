/* Stage1 decomp bundle: Hex-Rays pseudocode export from SCUS_941.83. */
/* No manual cleanup; kept close to current IDA output. */

/* line: 0, address: 0x80014a80 */ int __fastcall sub_80014A80(int a1, int a2, int a3)
/* line: 1 */ {
/* line: 2 */   int v3; // $t4
/* line: 3 */   int result; // $v0
/* line: 4 */   int v5; // $t0
/* line: 5 */   int v6; // $t1
/* line: 6 */   int v7; // $a3
/* line: 7 */   int v8; // $v1
/* line: 8 */   int v9; // $t2
/* line: 9 */   int v10; // $a0
/* line: 10 */   int v11; // $t3
/* line: 11 */   int v12; // $v1
/* line: 12 */   int v13; // $v0
/* line: 13 */
/* line: 14, address: 0x80014a90 */   v3 = 0;
/* line: 15, address: 0x80014a8c */   if ( !word_8009182A )
/* line: 16, address: 0x80014a94 */     return 0;
/* line: 17, address: 0x80014a9c */   v5 = 0;
/* line: 18, address: 0x80014aa0 */   v6 = 0;
/* line: 19, address: 0x80014aa4 */   v7 = 0;
/* line: 20, address: 0x80014aa8 */   v8 = 0;
/* line: 21, address: 0x80014ab8 */   v9 = a2;
/* line: 22, address: 0x80014ab4 */   if ( a2 < a3 )
/* line: 23 */   {
/* line: 24, address: 0x80014ac8 */     v10 = 12 * a2 + a1;
/* line: 25, address: 0x80014b2c */     do
/* line: 26 */     {
/* line: 27, address: 0x80014acc */       if ( *(_WORD *)(v10 + 6) )
/* line: 28 */       {
/* line: 29, address: 0x80014adc */         if ( *(_WORD *)(v10 + 18) )
/* line: 30 */         {
/* line: 31, address: 0x80014aec */           v8 |= 8u;
/* line: 32, address: 0x80014af4 */           ++v7;
/* line: 33 */         }
/* line: 34 */         else
/* line: 35 */         {
/* line: 36, address: 0x80014af8 */           v8 |= 4u;
/* line: 37, address: 0x80014b00 */           ++v6;
/* line: 38 */         }
/* line: 39 */       }
/* line: 40, address: 0x80014b04 */       else if ( *(_WORD *)(v10 + 18) )
/* line: 41 */       {
/* line: 42, address: 0x80014b14 */         v8 |= 2u;
/* line: 43, address: 0x80014b1c */         ++v5;
/* line: 44 */       }
/* line: 45 */       else
/* line: 46 */       {
/* line: 47, address: 0x80014b20 */         ++v3;
/* line: 48 */       }
/* line: 49, address: 0x80014b24 */       v9 += 2;
/* line: 50, address: 0x80014b30 */       v10 += 24;
/* line: 51 */     }
/* line: 52, address: 0x80014b2c */     while ( v9 < a3 );
/* line: 53 */   }
/* line: 54, address: 0x80014b38 */   if ( v8 == 14 )
/* line: 55 */   {
/* line: 56, address: 0x80014b68 */     v11 = 15 * v5 + 6 * v6 + 9 * v7;
/* line: 57, address: 0x80014b64 */     if ( !a2 )
/* line: 58 */     {
/* line: 59, address: 0x80014b70 */       result = 15 * v5 + 6 * v6 + 9 * v7;
/* line: 60, address: 0x80014b6c */       if ( v3 <= 0 )
/* line: 61, address: 0x80014b6c */         return result;
/* line: 62, address: 0x80014b78 */       v11 += 18;
/* line: 63 */     }
/* line: 64, address: 0x80014b74 */     return v11;
/* line: 65 */   }
/* line: 66, address: 0x80014b7c */   if ( v8 == 12 )
/* line: 67 */   {
/* line: 68, address: 0x80014b8c */     v12 = 9 * v7;
/* line: 69 */ LABEL_23:
/* line: 70, address: 0x80014bbc */     v13 = 6 * v6;
/* line: 71, address: 0x80014bc4 */     return v12 + v13;
/* line: 72 */   }
/* line: 73, address: 0x80014b90 */   if ( v8 != 10 )
/* line: 74 */   {
/* line: 75, address: 0x80014bb0 */     result = 0;
/* line: 76, address: 0x80014bac */     if ( v8 != 6 )
/* line: 77, address: 0x80014bac */       return result;
/* line: 78, address: 0x80014bb8 */     v12 = 15 * v5;
/* line: 79, address: 0x80014bb8 */     goto LABEL_23;
/* line: 80 */   }
/* line: 81, address: 0x80014b9c */   v12 = 9 * v7;
/* line: 82, address: 0x80014ba4 */   v13 = 15 * v5;
/* line: 83, address: 0x80014bd4 */   return v12 + v13;
/* line: 84 */ }
