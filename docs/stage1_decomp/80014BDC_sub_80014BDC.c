/* Stage1 decomp bundle: Hex-Rays pseudocode export from SCUS_941.83. */
/* No manual cleanup; kept close to current IDA output. */

/* line: 0, address: 0x80014bdc */ void __fastcall sub_80014BDC(char a1)
/* line: 1 */ {
/* line: 2 */   int v1; // $gp
/* line: 3 */   _BYTE *v2; // $a0
/* line: 4 */
/* line: 5, address: 0x80014bf8 */   v2 = (char *)&unk_80092910 + 384 * (a1 & 3);
/* line: 6, address: 0x80014c00 */   *(_DWORD *)(v1 + 28) = v2;
/* line: 7, address: 0x80014c04 */   sub_80025C44(v2, 384);
/* line: 8 */ }
