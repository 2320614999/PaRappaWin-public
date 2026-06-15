/* Stage1 decomp bundle: Hex-Rays pseudocode export from SCUS_941.83. */
/* No manual cleanup; kept close to current IDA output. */

/* line: 0, address: 0x80024e98 */ int sub_80024E98()
/* line: 1 */ {
/* line: 2 */   int v0; // $gp
/* line: 3 */
/* line: 4, address: 0x80024eb0 */   sub_80025C44(&dword_8008ECE0, 244);
/* line: 5, address: 0x80024ec8 */   dword_8008ED20 = 0;
/* line: 6, address: 0x80024ed0 */   dword_8008ED00 = 0;
/* line: 7, address: 0x80024ed8 */   g_PrStageEventStreamFlag = 1;
/* line: 8, address: 0x80024ee0 */   g_PrStageEventStreamId = 0;
/* line: 9, address: 0x80024ee8 */   *(_DWORD *)(v0 + 800) = 0;
/* line: 10, address: 0x80024eec */   sub_80025C44(&unk_801C3640, 488);
/* line: 11, address: 0x80024efc */   word_801C368E = 1;
/* line: 12, address: 0x80024f04 */   word_801C3690 = 0;
/* line: 13, address: 0x80024f0c */   word_801C3692 = 0;
/* line: 14, address: 0x80024f14 */   word_801C36AA = 1;
/* line: 15, address: 0x80024f1c */   word_801C36BA = 1;
/* line: 16, address: 0x80024f24 */   word_801C36CA = 0;
/* line: 17, address: 0x80024f2c */   word_801C36D0 = 0;
/* line: 18, address: 0x80024f34 */   word_801C36E2 = 0;
/* line: 19, address: 0x80024f40 */   dword_801C36D4 = dword_800943D0 + 4;
/* line: 20, address: 0x80024f54 */   dword_801C36D8 = dword_800943D0 + 24;
/* line: 21, address: 0x80024f5c */   word_801C36A8 = word_800916DC;
/* line: 22, address: 0x80024f64 */   word_801C3694 = word_800916DC;
/* line: 23, address: 0x80024f84 */   return sub_80024F8C(&unk_801C3640);
/* line: 24 */ }
