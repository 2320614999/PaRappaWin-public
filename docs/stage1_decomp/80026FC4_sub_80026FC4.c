/* IDA Hex-Rays pseudocode export from SCUS_941.83, 2026-05-12. */
/* line: 0, address: 0x80026fc4 */ void __fastcall sub_80026FC4(char *a1)
/* line: 1 */ {
/* line: 2 */   int v2; // $a1
/* line: 3 */   char v3; // $a2
/* line: 4 */ 
/* line: 5, address: 0x80026fdc */   if ( a1 )
/* line: 6 */   {
/* line: 7, address: 0x80027008 */     sub_800345E4(
/* line: 8 */       word_800943AA,
/* line: 9 */       word_800943A8,
/* line: 10 */       (unsigned __int8)byte_800943AE,
/* line: 11 */       (unsigned __int8)byte_800943AF,
/* line: 12 */       (unsigned __int8)byte_800943B0);
/* line: 13, address: 0x80027010 */     v2 = (unsigned __int8)*a1;
/* line: 14, address: 0x80027018 */     v3 = a1[1];
/* line: 15, address: 0x80027020 */     a1[2] = v3 + 24;
/* line: 16, address: 0x80027050 */     word_800943AA = sub_80034240(
/* line: 17 */                       word_800943A8,
/* line: 18 */                       v2,
/* line: 19 */                       v3,
/* line: 20 */                       (unsigned __int8)(v3 + 24),
/* line: 21 */                       0,
/* line: 22 */                       (unsigned __int8)a1[3],
/* line: 23 */                       (unsigned __int8)a1[3]);
/* line: 24, address: 0x80027058 */     sub_80025C64(a1, &byte_800943AE, 6);
/* line: 25 */   }
/* line: 26 */ }
